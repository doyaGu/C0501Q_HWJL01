/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL NAND functions.
 *
 * @author Jim Tan
 * @version 1.0
 *
 *
 *
 */
#include <errno.h>
#include <malloc.h>
#include "itp_cfg.h"
#include "ite/ite_nand.h"
#include "nand/mmp_nand.h"

#if defined(CFG_NAND_RESERVED_SIZE) && (CFG_NAND_RESERVED_SIZE!=0)
    #define ENABLE_CHECK_RESERVED_AREA
    
    #ifndef CFG_UPGRADE_IMAGE_POS
    #define CFG_UPGRADE_IMAGE_POS 0
    #endif
    
    #if defined(CFG_NET_ETHERNET_MAC_ADDR_POS)
        #if (CFG_NET_ETHERNET_MAC_ADDR_POS>=CFG_UPGRADE_IMAGE_POS)
        #error "ERROR: CFG_NET_ETHERNET_MAC_ADDR_POS can't larger than CFG_UPGRADE_IMAGE_POS"
        #endif
    #endif
    
    #if (CFG_UPGRADE_IMAGE_POS>=CFG_NAND_RESERVED_SIZE)
    #error "ERROR: CFG_UPGRADE_IMAGE_POS can't larger than CFG_NAND_RESERVED_SIZE"
    #endif
#endif
/***************************************************************************

****************************************************************************/
#ifdef	CFG_SPI_NAND
#include "ssp/mmp_spi.h"

#ifdef	CFG_SPI_NAND_USE_SPI0
#define NAND_SPI_PORT	SPI_0
#endif

#ifdef	CFG_SPI_NAND_USE_SPI1
#define NAND_SPI_PORT	SPI_1
#endif

#ifndef NAND_SPI_PORT
    #error "ERROR: itp_nand.c MUST define the SPI NAND bus(SPI0 or SPI1)"
#endif

#endif
/********************
 * global variable *
 ********************/
static ITE_NF_INFO	itpNfInfo;
static uint8_t GoodBlkCodeMark[4]={'F','I', 'N','E'};

static int gCurrentMode = 0;
static int gFsMaxSec = 0;
static int gFsSecSize = 0;
static int gBlkGap = 4;
/********************
 * private function *
 ********************/
static int FlushBootRom()
{
    char *BlkBuf=NULL;
    int  res=true;
	uint32_t  i,j;
	uint32_t  BakBlkNum;
    
    if(!itpNfInfo.BootRomSize)
    {
    	errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | 1;	//nand initial fail
    	LOG_ERR "nand block initial error: \n" LOG_END
    	goto end;
    }
    
    //allocate block buffer
    BlkBuf = malloc( itpNfInfo.PageInBlk * (itpNfInfo.PageSize + itpNfInfo.SprSize) );
    if(!BlkBuf)
    {
    	errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | 1;	//nand initial fail
    	LOG_ERR "nand block initial error: \n" LOG_END
    	goto end;
    }
    
    //calculate block number of boot ROM
	BakBlkNum = 0;
    
    for(i=0; i<BakBlkNum; i++)
    {
    	char *tmpbuf=BlkBuf;
    	
    	//read a block data from bak area(include spare data)
    	for(j=0; j<itpNfInfo.PageInBlk;j++)
    	{
    		if( iteNfRead(i, j, tmpbuf)!=true )				
    		{
    			errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | 1;
    			LOG_ERR "itp nand read block error: \n" LOG_END
    			res = -1;
    			goto end;
    		}
    		tmpbuf+=(itpNfInfo.PageSize + itpNfInfo.SprSize);
    	}
    	
    	//write to boot area a block size
    	tmpbuf=BlkBuf;
    	for(j=0;j<itpNfInfo.PageInBlk;j++)
    	{
    		if(iteNfWrite(i, j, tmpbuf, tmpbuf+itpNfInfo.SprSize)!=true )
    		{
    			errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | 1;	//nand initial fail
    			LOG_ERR "itp nand write block error: \n" LOG_END
    			res = -1;
    			goto end;
    		}
    		tmpbuf+=(itpNfInfo.PageSize + itpNfInfo.SprSize);
    	}    	
    }
     
end:
	return 	res;
}

/********************
 * public function *
 ********************/
static int NandOpen(const char* name, int flags, int mode, void* info)
{
    int blkSize = 0;
    
    LOG_DBG "NandOpen: blk=%x, page=%x\n",itpNfInfo.PageInBlk,itpNfInfo.PageSize LOG_END 
    
    if(mode == ITP_NAND_FTL_MODE)
    {
    	mmpNandGetCapacity(&gFsMaxSec, &gFsSecSize);
        gBlkGap = 0;
    }
    else
    {        
        gBlkGap = 4;
    }
    gCurrentMode = mode;
    
    itpNfInfo.CurBlk = 0;
    blkSize = itpNfInfo.PageInBlk*itpNfInfo.PageSize;
    if(!blkSize)
    {
        LOG_ERR "NAND Block size = 0!!\n" LOG_END
        return -1;
    }

#ifdef ENABLE_CHECK_RESERVED_AREA
    //to get reserved area BL/MAC/img position
    #ifdef CFG_NET_ETHERNET_MAC_ADDR_NAND
    if( CFG_NET_ETHERNET_MAC_ADDR_POS % blkSize )
    {
        LOG_ERR "CFG_NET_ETHERNET_MAC_ADDR_POS is not at the block boundary, (%x,%x)\n",CFG_NET_ETHERNET_MAC_ADDR_POS, blkSize LOG_END
        return -1;
    }

    itpNfInfo.blEndBlk = CFG_NET_ETHERNET_MAC_ADDR_POS/blkSize;
    #else
    itpNfInfo.blEndBlk = CFG_UPGRADE_IMAGE_POS/blkSize;
    #endif
    if( CFG_UPGRADE_IMAGE_POS % blkSize )
    {
        LOG_ERR "CFG_UPGRADE_IMAGE_POS is not at the block boundary, (%x,%x)\n",CFG_UPGRADE_IMAGE_POS, blkSize LOG_END
        return -1;
    }
    
    if( CFG_NAND_RESERVED_SIZE % blkSize )
    {
        LOG_ERR "CFG_NAND_RESERVED_SIZE is not at the block boundary, (%x,%x)\n",CFG_NAND_RESERVED_SIZE, blkSize LOG_END
        return -1;
    } 
    
    itpNfInfo.bootImgStartBlk = CFG_UPGRADE_IMAGE_POS/blkSize;
    itpNfInfo.bootImgEndBlk = CFG_NAND_RESERVED_SIZE/blkSize;
    LOG_DBG "reserved area, A1=0x%x, A2=0x%x, A3=0x%x(blks)\n",itpNfInfo.blEndBlk,itpNfInfo.bootImgStartBlk,itpNfInfo.bootImgEndBlk LOG_END
    
    #ifdef CFG_NET_ETHERNET_MAC_ADDR_NAND
    if(!itpNfInfo.blEndBlk)
        LOG_INFO "MacPos=0x%x, ImgPos=0x%x, bSize=0x%x\n",CFG_NET_ETHERNET_MAC_ADDR_POS,CFG_UPGRADE_IMAGE_POS,blkSize LOG_END
    #else
    if(!itpNfInfo.blEndBlk)
        LOG_INFO "MacPos=0x%x, ImgPos=0x%x, bSize=0x%x\n",0,CFG_UPGRADE_IMAGE_POS,blkSize LOG_END
    #endif
    
    return 0;
#else
    //return fail if NAND has NO reserved size(Because it does NOT make sense)
    LOG_WARN "It's forbidden for NAND itp driver without reserved size\n" LOG_END
    return -1;
#endif

}

static int NandClose(int file, void* info)
{
    LOG_DBG "NandClose::%x, %x\n",itpNfInfo.PageInBlk,itpNfInfo.PageSize LOG_END   
    
    /* clear these parameters of LBA mode  */
   	gFsMaxSec = 0;
    gFsSecSize = 0;
    
    //errno = ENOENT;
    //errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
    return 0;
}

static int NandRead(int file, char *ptr, int len, void* info)
{
    int  DoneLen = 0;
	uint32_t  blk = 0;
	uint32_t  pg = 0;
	uint32_t  doLen = 0;
    char *databuf;
	char *tmpbuf;
	char *srcbuf=ptr;
    int blkSize;
    uint32_t areaBoundary;

	//printf("itpNfRd::buf=%x, b=%x, l=%x!!\n",ptr,itpNfInfo.CurBlk,len);
	
    blkSize = itpNfInfo.PageInBlk*itpNfInfo.PageSize;
    
    if(gCurrentMode == ITP_NAND_FTL_MODE)
    {
    	unsigned long sector,sCnt;
    	
    	sector = itpNfInfo.CurBlk*(blkSize/gFsSecSize);
    	sCnt = len*(blkSize/gFsSecSize);
    	//printf("itpmmpRd:%x,%x,%x\n",sector,sCnt,ptr);
    	
    	if( mmpNandReadSector(sector, sCnt, ptr) == 0 )
    	{
    		DoneLen = len;
    		itpNfInfo.CurBlk += DoneLen;
    	}
    	goto end;
    }    

    //check baundary of bootloader/Mac address/boot image
    //check current block index "tpNfInfo.CurBlk" for suring BL/Mac/image area
    //if "tpNfInfo.CurBlk" or "tpNfInfo.CurBlk+len" is over the boundary, then return fail
    
    if(blkSize)
    {
#ifdef ENABLE_CHECK_RESERVED_AREA    
        if(itpNfInfo.CurBlk<itpNfInfo.blEndBlk)  
            areaBoundary = itpNfInfo.blEndBlk;
        else if(itpNfInfo.CurBlk<itpNfInfo.bootImgStartBlk)
            areaBoundary = itpNfInfo.bootImgStartBlk;
        else 
            areaBoundary = itpNfInfo.bootImgEndBlk;
#else
        areaBoundary = itpNfInfo.CurBlk+len;
#endif           
        if( (itpNfInfo.CurBlk+(uint32_t)len) > (uint32_t)areaBoundary )
        {
            LOG_ERR "itpNfRead has NO enough space for reading, curB=%x, len=%x, boundary=%x\n",itpNfInfo.CurBlk,len,areaBoundary LOG_END
            printf( "itpNfRead has NO enough space for reading, curB=%x, len=%x, boundary=%x\n",itpNfInfo.CurBlk,len,areaBoundary );
            
#ifdef ENABLE_CHECK_RESERVED_AREA  
           if(areaBoundary==itpNfInfo.blEndBlk)
               printf("OUT of bootloader area!!\n");
           else if(areaBoundary==itpNfInfo.bootImgStartBlk)
               printf("OUT of MAC-address area!!\n");
           else if(areaBoundary==itpNfInfo.bootImgEndBlk)
               printf("OUT of boot-image area!!\n");
           else
               printf("UNKNOW condition::%x,%x\n",areaBoundary,itpNfInfo.bootImgEndBlk);
#endif   
            goto end;
        }    
    }
    else
    {
        LOG_ERR "itpNfRead Error, block size = 0!!\n" LOG_END
        goto end; 
    }

    if( !ptr || !len )
    {
    	errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//ptr or len error
    	LOG_ERR "ptr or len error: \n" LOG_END
    	goto end;
    }
    
    if(!itpNfInfo.NumOfBlk)
    {
    	errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//nand initial fail
    	LOG_ERR "nand block initial error: \n" LOG_END
    	goto end;
    }

    databuf = (char *)malloc(itpNfInfo.PageInBlk*itpNfInfo.PageSize);
    if(databuf==NULL)
    {
    	errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//out of memory:
        LOG_ERR "out of memory:\n" LOG_END
    	goto end;  	
    }
	
   	for(blk=itpNfInfo.CurBlk; blk<areaBoundary; blk++)
   	{
#ifdef ENABLE_CHECK_RESERVED_AREA
        if( blk >= (uint32_t)areaBoundary )
        {
            LOG_ERR "itpNfRead impossible condition: out of boundary:blk=%d, Boundary=%d\n", blk, areaBoundary LOG_END
            printf( "itpNfRead impossible condition: out of boundary:blk=%d, Boundary=%d\n", blk, areaBoundary );
            //printf("ERROR:: NO enough space for nand writing data, %x, %x, %x, %x\n",blk,itpNfInfo.blEndBlk,itpNfInfo.bootImgStartBlk,itpNfInfo.bootImgEndBlk);
           
            itpNfInfo.CurBlk = blk;   //set current block index
            goto end;
        }  
#endif        
        //check if "blk" a bad block
        if( iteNfIsBadBlockForBoot(blk) != true )
        {
            //true means "GOOD BLOCK"; !true means "BAD BLOCK"
            continue;
        }

		tmpbuf = databuf;
   		for( pg=0; pg<itpNfInfo.PageInBlk; pg++)
   		{ 
   			if( iteNfReadPart(blk, pg, tmpbuf, LL_RP_DATA)!=true )
   			{
   				errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//read fail
   				return -1;
   			}
   			tmpbuf += itpNfInfo.PageSize;
   		}

        if(blk)
        {
            memcpy(srcbuf, databuf+4, itpNfInfo.PageInBlk*itpNfInfo.PageSize-4);
            srcbuf+=(itpNfInfo.PageInBlk*itpNfInfo.PageSize)-4;
        }
        else
        {
            memcpy(srcbuf, databuf, itpNfInfo.PageInBlk*itpNfInfo.PageSize);  		
            srcbuf+=(itpNfInfo.PageInBlk*itpNfInfo.PageSize);
        }

        DoneLen++;
        if(DoneLen>len)
        {
            LOG_ERR "itpNfRead impossible condition: DoneLen:%d, len:%d\n", DoneLen, len LOG_END
            itpNfInfo.CurBlk = blk+1; 
            goto end;
        }
        
        if(DoneLen==len)    break;
   	}

   	//printf("itpNfRd:cBlk=%x, b=%x\n",itpNfInfo.CurBlk,blk);
   	itpNfInfo.CurBlk = blk+1; 
    
end:
	if(databuf)
	{
		free(databuf);
		databuf = NULL;
	}	
    return DoneLen;
}

static int NandWrite(int file, char *ptr, int len, void* info)
{
    int  DoneLen = 0;
    char *DataBuf;
	char *tmpDataBuf;
	char *tmpSrcBuf = ptr;
    char *SprBuf= NULL;
    uint32_t blk,pg;
    int blkSize;
    uint32_t areaBoundary;
    
    //printf("itpNfWt::buf=%x, b=%x, l=%x!!\n", ptr, itpNfInfo.CurBlk, len);
    if( !ptr || !len )
    {
    	errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//ptr or len error
    	LOG_ERR "ptr or len error: \n" LOG_END
    	goto end;
    }
    
    if(!itpNfInfo.NumOfBlk)
    {
    	errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//nand initial fail
    	LOG_ERR "nand block initial error: \n" LOG_END
    	goto end;
    }

    blkSize = itpNfInfo.PageInBlk*itpNfInfo.PageSize;
    
    if(blkSize) areaBoundary = CFG_NAND_RESERVED_SIZE/blkSize;  //(unit:block)
    else
    {        
        printf("NfErr::block size = 0!!\n");
        LOG_ERR "block size = 0:\n" LOG_END
	    goto end;  	
    }
        
    if(gCurrentMode == ITP_NAND_FTL_MODE)
    {
    	unsigned long sector,sCnt;
    	
    	sector = itpNfInfo.CurBlk*(blkSize/gFsSecSize);
    	sCnt = len*(blkSize/gFsSecSize);
    	//printf("itpmmpWt:%x,%x,%x\n",sector,sCnt,ptr);
    	if( mmpNandWriteSector(sector, sCnt, ptr) == 0 )
    	{
    		DoneLen = len;
    		itpNfInfo.CurBlk += len;
    	}
    	goto end;
    }
        
#ifdef ENABLE_CHECK_RESERVED_AREA
        //check baundary of bootloader/Mac address/boot image
    //check current block index "tpNfInfo.CurBlk" for suring area of BL/Mac/image
    //if "tpNfInfo.CurBlk" is over the boundary, then return fail
    if(itpNfInfo.CurBlk<itpNfInfo.blEndBlk)  
        areaBoundary = itpNfInfo.blEndBlk;
    else if(itpNfInfo.CurBlk<itpNfInfo.bootImgStartBlk)
        areaBoundary = itpNfInfo.bootImgStartBlk;
    else 
        areaBoundary = itpNfInfo.bootImgEndBlk;
#endif        
        
    if( (itpNfInfo.CurBlk + (uint32_t)len) > areaBoundary )
    {
        LOG_ERR "write over the boundary, curB=%x, len=%x, boundary=%x\n",itpNfInfo.CurBlk,len,areaBoundary LOG_END
        printf( "write over the boundary, curB=%x, len=%x, boundary=%x\n",itpNfInfo.CurBlk,len,areaBoundary );
        goto end;
    }

    DataBuf = (char *)malloc(itpNfInfo.PageInBlk*itpNfInfo.PageSize);
    if(DataBuf==NULL)
    {
    	errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//out of memory:
        LOG_ERR "out of memory:\n" LOG_END
    	goto end;  	
    }

    if(!itpNfInfo.SprSize)
    {
    	errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//nand initial fail
        LOG_ERR "nand spare initial error: \n" LOG_END
    	goto end;
    }
    
    SprBuf = (char *)malloc(itpNfInfo.SprSize);
    if(SprBuf==NULL)
    {
    	errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//out of memory:
        LOG_ERR "out of memory:\n" LOG_END
    	goto end;  	
    }

    memset((uint8_t*)SprBuf, 0xFF, itpNfInfo.SprSize);

   	for(blk=itpNfInfo.CurBlk; blk<areaBoundary; blk++)
   	{
   	    int allNfPgWtHasPass;
   	    
        if( blk >= areaBoundary )
        {
           LOG_ERR "itpNfWrite out of boundary: blk:%d, Boundary:%d\n", blk, areaBoundary LOG_END
           printf("ERROR:: NO enough space for nand writing data, %x, %x, %x, %x\n",blk,itpNfInfo.blEndBlk,itpNfInfo.bootImgStartBlk,itpNfInfo.bootImgEndBlk);
#ifdef ENABLE_CHECK_RESERVED_AREA           
           if(areaBoundary==itpNfInfo.blEndBlk)
               printf("OUT of bootloader area!!\n");
           else if(areaBoundary==itpNfInfo.bootImgStartBlk)
               printf("OUT of MAC-address area!!\n");
           else if(areaBoundary==itpNfInfo.bootImgEndBlk)
               printf("OUT of boot-image area!!\n");
           else
               printf("UNKNOW condition::%x,%x\n",areaBoundary,itpNfInfo.bootImgEndBlk);
#endif           
           itpNfInfo.CurBlk = blk;  //set current block index
           goto end;
        }        
        
        //check if "blk" a bad block
        if( iteNfIsBadBlockForBoot(blk) != true )
        {
            //true means "GOOD BLOCK"; !true means "BAD BLOCK"
            continue;
        }        
        
   		//erase current block
   		if( iteNfErase(blk)!=true )
   		{
   			errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//erase fail
   			LOG_ERR "erase block fail: %d\n", blk LOG_END

#if 1
   			printf("NfErsErr:: need to mark bad block(blk=0x%x)\n",blk);
   			//mark as bad block
   			{
   			    char db[4096] = {0};    //max page size = 4096
   			    char sb[512] = {0};     //max spare size = 512
   			    iteNfWrite(blk, 0, db, sb);
   			}   			
   			continue;
#else
   			//DoneLen = 0;
   			itpNfInfo.CurBlk = blk;
   			goto end;
#endif
   		}

        if(blk)
        {
            memcpy(DataBuf, GoodBlkCodeMark, 4);
            memcpy(DataBuf+4, tmpSrcBuf, itpNfInfo.PageInBlk*itpNfInfo.PageSize - 4);
        }
        else
        {
            memcpy(DataBuf, tmpSrcBuf, itpNfInfo.PageInBlk*itpNfInfo.PageSize);
        }
        tmpDataBuf = DataBuf;
   		
        allNfPgWtHasPass = 1;
    	for( pg=0; pg<itpNfInfo.PageInBlk; pg++)
    	{
    		//SetSprBuff(blk, pg, SprBuf);
    		if( iteNfWrite(blk, pg, tmpDataBuf, SprBuf)!=true )
    		{
   				errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//erase fail
   				LOG_ERR "write fail:[%d,%d]\n", blk, pg LOG_END
   				
                //mark as bad block
                {
                    char db[4096] = {0};    //max page size = 4096
                    char sb[512] = {0};     //max spare size = 512
                    iteNfWrite(blk, 0, db, sb);
                }
                tmpDataBuf -= itpNfInfo.PageSize * pg;
                LOG_ERR "buffer address return to 0x%x:(ori=0x%x)\n", tmpDataBuf, (uint32_t)tmpDataBuf - (itpNfInfo.PageSize*pg) LOG_END
                allNfPgWtHasPass = 0;            
                break;
   				//DoneLen = 0;
   				//itpNfInfo.CurBlk = blk;
   				//goto end;
    		}
			tmpDataBuf+=itpNfInfo.PageSize;
    	}
    	if(allNfPgWtHasPass)   DoneLen++;
    	else                   continue;
    	
        if(DoneLen>len)
        {
            LOG_ERR "itpNfWrite impossible condition: DoneLen:%d, len:%d\n", DoneLen, len LOG_END
            itpNfInfo.CurBlk = blk+1;
            goto end;
        }
        
        if(DoneLen==len)
        {
            break;
        }

        if(blk)	tmpSrcBuf += itpNfInfo.PageInBlk*itpNfInfo.PageSize - 4;
        else	tmpSrcBuf += itpNfInfo.PageInBlk*itpNfInfo.PageSize;		
    }
    //itpNfInfo.CurBlk += DoneLen;    
    printf("itpNfWt:cBlk=%x, b=%x, new-cb=%x\n",itpNfInfo.CurBlk,blk,blk+1);
    itpNfInfo.CurBlk = blk+1; 

end:
	if(SprBuf)
	{
		free(SprBuf);
		SprBuf = NULL;
	}		
	if(DataBuf)
	{
		free(DataBuf);
		DataBuf = NULL;
	}	
    return DoneLen;
}

static int NandLseek(int file, int ptr, int dir, void* info)
{
	int DefBlk = CFG_NAND_RESERVED_SIZE / (itpNfInfo.PageInBlk*itpNfInfo.PageSize);
	
    switch(dir)
    {
    default:
    case SEEK_SET:
    	if(gCurrentMode == ITP_NAND_FTL_MODE)
    	{
    		//int DefBlk = CFG_NAND_RESERVED_SIZE / (itpNfInfo.PageInBlk*itpNfInfo.PageSize);
    		if(DefBlk>ptr)
    		{
    			printf("ERROR: current seek position is abnormal, ori pos=%x, rev pos=%x\n",ptr,DefBlk);
    		}
    		else
    		{
    			itpNfInfo.CurBlk = ptr - DefBlk;
    		}
    	}
    	else
    	{
    		itpNfInfo.CurBlk = ptr; //raw data mode(without FTL)
    	}	
        #ifdef ENABLE_CHECK_RESERVED_AREA
        {
           if(itpNfInfo.CurBlk<itpNfInfo.blEndBlk)
               LOG_INFO "itpNfSeek to bootloader area!!(%x,%x)\n",itpNfInfo.CurBlk,itpNfInfo.blEndBlk LOG_END
           else if(itpNfInfo.CurBlk<itpNfInfo.bootImgStartBlk)
               LOG_INFO "itpNfSeek to MAC address area!!(%x,%x)\n",itpNfInfo.CurBlk,itpNfInfo.bootImgStartBlk LOG_END
           else if(itpNfInfo.CurBlk<itpNfInfo.bootImgEndBlk)
               LOG_INFO "itpNfSeek to IMAGE area!!(%x,%x)\n",itpNfInfo.CurBlk,itpNfInfo.bootImgEndBlk LOG_END
           else
               LOG_INFO "itpNfSeek to UNKNOW area::%x, %x\n",itpNfInfo.CurBlk,itpNfInfo.blEndBlk,itpNfInfo.bootImgStartBlk,itpNfInfo.bootImgEndBlk LOG_END
        }
        #endif
        break;
    case SEEK_CUR:
        itpNfInfo.CurBlk += ptr;
        break;
    case SEEK_END:
        break;
    }
    if(gCurrentMode == ITP_NAND_FTL_MODE)
    	return itpNfInfo.CurBlk + DefBlk;
    else
    	return itpNfInfo.CurBlk;
}

static int NandIoctl(int file, unsigned long request, void* ptr, void* info)
{
	int res = -1;
	
    switch (request)
    {
	    case ITP_IOCTL_INIT:
	    	#ifdef	CFG_SPI_NAND
	    	mmpSpiInitialize(NAND_SPI_PORT, SPI_OP_MASTR, CPO_0_CPH_0, SPI_CLK_20M);
	    	#endif

	    	itpNfInfo.BootRomSize = (uint32_t)CFG_NAND_RESERVED_SIZE;
	    	#ifdef	CFG_NOR_SHARE_SPI_NAND
	    	itpNfInfo.enSpiCsCtrl = 1;
	    	#else
	    	itpNfInfo.enSpiCsCtrl = 0;
	    	#endif
	    	
	    	#ifdef	CFG_SPI_NAND_ADDR_HAS_DUMMY_BYTE
	    	LOG_INFO "set AHDB=1\n" LOG_END
	    	itpNfInfo.addrHasDummyByte = 1;
	    	#else
	    	LOG_INFO "set AHDB=0\n" LOG_END
	    	itpNfInfo.addrHasDummyByte = 0;
	    	#endif
	    	
	    	printf("ITP_IOCTL_INIT[01](%x,%x)(%x,%x)\n",&itpNfInfo,itpNfInfo.NumOfBlk,itpNfInfo.BootRomSize,itpNfInfo.enSpiCsCtrl);
	        if( iteNfInitialize(&itpNfInfo)==true )
	        {
	        	printf("ITP_IOCTL_INIT[02](%x,%x)(%x)\n",&itpNfInfo, itpNfInfo.NumOfBlk, itpNfInfo.BootRomSize);
	        	res = 0;
	        }
	        #ifdef	CFG_NOR_SHARE_SPI_NAND
	        mmpSpiSetControlMode(SPI_CONTROL_NOR);
	        mmpSpiResetControl();
	        #endif
	        break;
	
	    case ITP_IOCTL_GET_BLOCK_SIZE:
	        if(itpNfInfo.NumOfBlk)
	        {
        		*(unsigned long*)ptr = (itpNfInfo.PageInBlk)*(itpNfInfo.PageSize) - gBlkGap;
	        	res = 0;
	        }
	        break;
	        
	    case ITP_IOCTL_FLUSH:
	        // TODO: IMPLEMENT
	        if(FlushBootRom()==true)
	        {
	        	res = 0;
	        }
	        break;

	    case ITP_IOCTL_SCAN:
	    	{
	    		uint32_t *blk=(uint32_t *)ptr;
	        	if(iteNfIsBadBlockForBoot(*blk)==true )
	        	{
	        		//*(unsigned long*)ptr = itpNfInfo.PageInBlk * itpNfInfo.PageSize;
	        		res = 0;
	        	}
	        }
	        break;

        case ITP_IOCTL_GET_GAP_SIZE:
            {
               	*(unsigned long*)ptr = gBlkGap;
                res = 0;
            }
            break;

        case ITP_IOCTL_NAND_CHECK_MAC_AREA:
#ifdef CFG_NET_ETHERNET_MAC_ADDR_NAND
            //check MAC address area(need 1 good block)
            {
                int isNoGoodBlock = 1;
                int i;
                
                for(i=itpNfInfo.blEndBlk; i<itpNfInfo.bootImgStartBlk; i++)
                {
                    //check bad block
                    if( iteNfIsBadBlockForBoot(i) == true )
                    {
                        //got one good block
                        isNoGoodBlock = 0;
                        break;
                    }
                }
                if(isNoGoodBlock)
                {
                    LOG_ERR "This NAND has too many bad blocks in MAC-address area\n" LOG_END
                    ithDelay(10000);
                }
                else
                    res = 0;
            }
#endif
            break;

	    default:
	        errno = (ITP_DEVICE_NAND << ITP_DEVICE_ERRNO_BIT) | 1;
	        break;
    }
    return res;
}

const ITPDevice itpDeviceNand =
{
    ":nand",
    NandOpen,
    NandClose,
    NandRead,
    NandWrite,
    NandLseek,
    NandIoctl,
    NULL
};
