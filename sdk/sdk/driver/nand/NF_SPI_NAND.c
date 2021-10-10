#include <unistd.h>	//for usleep()
#include <stdio.h>	//for printf()
#include <string.h>	//for strcpy()


#include "ite/ith.h"
#include "ite/itp.h"	//for itpVmemAlloc()
#include "inc/configs.h"
#include "inc/NF_SPI_NAND.h"



#define    ENABLE_NEW_READ_PART
////////////////////////////////////////////////////////////
// Compile Option
////////////////////////////////////////////////////////////

 
static uint8_t  *g_pNFdataBuf          = MMP_NULL;
static uint8_t  *g_pNFspareBuf         = MMP_NULL;
static uint8_t  *g_pNFeccBuf           = MMP_NULL;

static uint8_t  *gNfDataBuf            = MMP_NULL;

#define    SNF_PAGE_SIZE          CFG_NAND_PAGE_SIZE
//#define    SNF_TOTAL_SPARE_SIZE   128
//#define    SNF_SPARE_SIZE         24
////////////////////////////////////////////////////////////
// Compile Option
////////////////////////////////////////////////////////////
//#define    SHOW_RWE_MSG
//#define    ENABLE_ERASE_ALL_BLOCKS



/********************************************************************
 * enum/type define
 ********************************************************************/
#define    SPINF_INVALID_REVBLK_SIZE		 (0xFFFFFFFF)

typedef uint8_t (*BLOCK_CHECK_METHOD) (uint32_t pba);
 
enum {
	LL_OK,
	LL_ERASED,
	LL_ERROR
};

typedef enum{
    NAND_MAKER_TOSHIBA = 0, // 0
    NAND_MAKER_SAMSUNG,     // 1
    NAND_MAKER_HYNIX,       // 2
    NAND_MAKER_NUMONYX,     // 3
    NAND_MAKER_POWERFLASH,  // 4
    NAND_MAKER_GPOWER,      // 5
    NAND_MAKER_WINBOND,     // 6
    NAND_MAKER_XTX,     	// 7
    NAND_UNKNOWN_MAKER
}NAND_MAKER;



typedef enum{
    NAND_TOSHIBA_METHOD_01 = 0, // Check byte 0 in data & 1st bytes in spare of 1st & 2nd page
    NAND_TOSHIBA_METHOD_02,     // Check byte 0 in data & 1st bytes in spare of last page
    NAND_TOSHIBA_METHOD_03,     // Check byte 0 in data & 1st bytes in spare of 1st page & last page
    NAND_SAMSUNG_METHOD_01,     // Check 1st bytes in spare of 1st & 2nd page
    NAND_SAMSUNG_METHOD_02,     // Check 1st bytes in spare of last page
    NAND_HYNIX_METHOD_01,       // Check 1st bytes in spare of 1st & 2nd page
    NAND_HYNIX_METHOD_02,       // Check 1st bytes in spare of last page & (last-2) page
    NAND_NUMONYX_METHOD_01,     // Check 1st & 6th byte in spare of 1st page
    NAND_NUMONYX_METHOD_02,     // Check 1st byte in spare of last page
    NAND_POWERFLASH_METHOD_01,
    NAND_GPOWER_METHOD_01,      // Check 1st byte in spare of first page
    NAND_UNKNOWN_METHOD
}NAND_BLOCK_CHECK_METHOD;

typedef struct 
{
	unsigned long wear; 		/* spare area 32-bit Wear Leveling Counter */
	unsigned char dummy[8];		/* 8 bytes allocated for any structure below */
    unsigned long ecc;			/* space for ECC for lower layer calculation */
	unsigned long goodCode;
	unsigned long revBlkNum;
    unsigned char dummy32[104];		/* other 128-24 = 104 dummy byte */
} SPINF_ST_SPARE;

/********************************************************************
 * global variable
 ********************************************************************/
static SPI_NF_INFO g_NfAttrib; 
static unsigned long  g_ReservedBlockNum       = SPINF_INVALID_REVBLK_SIZE;

static const MMP_UINT32 WRITE_SYMBOL  = 0x46544C57;
/********************************************************************
 * private function
 ********************************************************************/

 

// Check 1st bytes in spare of last page
// The initial invalid block(s) status is defined by the 1st byte in the spare area.
// Samsung makes sure that the last page of every initial invalid block has non-FFh data at the column address of 2,048.
static uint8_t _IsBadBlockGPowerMethod01(uint32_t pba);

uint8_t _checkEcc(void)
{
    uint8_t status;

    LOG_INFO "[LEAVE]SpiNf_Read: \n" LOG_END
    return (0);
}

static uint8_t _checkOriBadBlock(uint32_t pba)
{
   	uint8_t	rst = LL_ERROR;
   	uint8_t	spiRst;
    uint8_t Byte=0;

    switch(g_NfAttrib.BBM_type)
    {
    	case 1:	//for GigaDevice BBM_TYPE_GD1
    		//printf("BBM type1\n");
    		spiRst = SpiNf_ReadOneByte(pba, 0, g_NfAttrib.PageSize, &Byte);
    		if( Byte == 0xFF )	rst = LL_OK;
    		break;
    		
    	case 2://for BBM_TYPE_MXIC1
    		//printf("BBM type2\n");
    		spiRst = SpiNf_ReadOneByte(pba, 0, g_NfAttrib.PageSize, &Byte);
    		if( Byte == 0xFF )
    		{    		
    			spiRst = SpiNf_ReadOneByte(pba, 1, g_NfAttrib.PageSize, &Byte);
    			if( Byte == 0xFF )	rst = LL_OK;
    		}
    		break;
    		
    	case 3://for BBM_TYPE_WINBOND1
    		//printf("BBM type3\n");
    		spiRst = SpiNf_ReadOneByte(pba, 0, 0, &Byte);
    		if( Byte == 0xFF )
    		{
    		    //check the 0th byte of spare area
    		    //however, the function "SpiNf_ReadOneByte()" has exchange the data to 16th byte of spare area
    			spiRst = SpiNf_ReadOneByte(pba, 0, g_NfAttrib.PageSize+16, &Byte);
    			if( Byte == 0xFF )	rst = LL_OK;
    		}
    		break;
    			
    	case 4://for BBM_TYPE_XTX1
    		//printf("BBM type4\n");
    		spiRst = SpiNf_ReadOneByte(pba, 0, 0, &Byte);
    		if( Byte == 0xFF )
    		{
    		    //check the 0th byte of spare area
    		    //however, the function "SpiNf_ReadOneByte()" has exchange the data to 0th byte of spare area
    		    //rst = LL_OK;
    			spiRst = SpiNf_ReadOneByte(pba, 0, g_NfAttrib.PageSize+0, &Byte);
    			if( Byte == 0xFF )	rst = LL_OK;
    		}
    		break;
    			
    	default:
    		spiRst = SpiNf_ReadOneByte(pba, 0, g_NfAttrib.PageSize, &Byte);
    		if( Byte == 0xFF )	rst = LL_OK;
    		break;
    }
    
    #ifdef SHOW_INFO_MSG
    printf("_chkbadBlk=%x,%x\n",pba,rst);
    #endif
    
    return rst;
}

/********************************************************************
 * public function
 ********************************************************************/
void SpiNf_GetAttribute(uint8_t *NfType, uint8_t *EccStatus)
{
    LOG_INFO  "[ENTER]SpiNf_GetAttribute: \n"  LOG_END

	*NfType = MMP_TRUE;
	*EccStatus = MMP_TRUE;

    LOG_INFO "[LEAVE]SpiNf_GetAttribute: \n" LOG_END
}

void SpiNf_Getwear(uint32_t blk, uint8_t *dest)
{
	  //NOTHING TO DO
    LOG_INFO  "[ENTER]SpiNf_Getwear: \n"  LOG_END
    LOG_INFO "[LEAVE]SpiNf_Getwear: \n" LOG_END
}

uint8_t SpiNf_Initialize(uint32_t* pNumOfBlocks, uint8_t* pPagePerBlock, uint32_t* pPageSize)
{
	SPI_NF_INFO dev;
	
	#ifdef SHOW_INFO_MSG
	printf("spiNf init:1\n");
	#endif
	  
	memset(&g_NfAttrib, 0, sizeof(SPI_NF_INFO));
	memset(&dev, 0, sizeof(SPI_NF_INFO));

	spiNf_Initial(&dev);
	
	printf("    NfDev.Init=%x\n",dev.Init);
	printf("    NfDev.TotalBlk=%x\n",dev.TotalBlk);
	printf("    NfDev.PageInBlk=%x\n",dev.PageInBlk);
	printf("    NfDev.PageSize=%x\n",dev.PageSize);
    printf("    NfDev.SprSize=%x\n",dev.FtlSprSize);

	if(!dev.Init)	return 1;

	memcpy(&g_NfAttrib, &dev, sizeof(SPI_NF_INFO));

    *pNumOfBlocks = g_NfAttrib.TotalBlk;
    *pPagePerBlock = g_NfAttrib.PageInBlk;
    *pPageSize = g_NfAttrib.PageSize;	
    
    /* malloc read/write buffer */
    if(gNfDataBuf==MMP_NULL)
    {
        gNfDataBuf = (unsigned char*)malloc(g_NfAttrib.PageSize+g_NfAttrib.SprSize+32);
    }
    
    #ifdef ENABLE_ERASE_ALL_BLOCKS
    if(1)
    {
        uint32_t i;
		for(i=0; i<0x80; i++)	SpiNf_Erase(i);
		for(i=0; i<g_NfAttrib.TotalBlk; i++)	if(SpiNf_IsBadBlockForBoot(i)==LL_OK)	SpiNf_Erase(i);
    
    	printf("Erase All Blocks OK!!!\n");
    	while(1);
	}
    #endif		
    
    #ifdef SHOW_INFO_MSG
    printf("spiNf init:4\n");
    #endif
    
	return LL_OK;
}

uint8_t SpiNf_Erase(uint32_t pba)
{
	uint8_t  status;
	
	LOG_INFO  "[ENTER]SpiNf_Erase: \n"  LOG_END	
	
	status = spiNf_BlockErase(pba);
		
	LOG_INFO "[LEAVE]SpiNf_Erase: \n" LOG_END	
	
    #ifdef SHOW_RWE_MSG
	printf("Ers[%x,%x]\n",pba,status);
    #endif
    
	if(status)    return (LL_ERROR);
	else          return (LL_OK);
}

uint8_t SpiNf_Write(uint32_t pba, uint32_t ppo, uint8_t* pDtBuf, uint8_t* pSprBuf)
{
	uint8_t  status;
		
    LOG_INFO  "[ENTER]SpiNf_Write: \n"  LOG_END
    
    #ifdef SHOW_RWE_MSG
	printf("W1[%x,%x][%x,%x]\n",pba,ppo,pDtBuf,pSprBuf);
    #endif

    memcpy(&gNfDataBuf[0], pDtBuf, g_NfAttrib.PageSize);
    memcpy(&gNfDataBuf[g_NfAttrib.PageSize], pSprBuf, 16);//g_NfAttrib.FtlSprSize);
    memcpy(&gNfDataBuf[g_NfAttrib.PageSize+16], &WRITE_SYMBOL, 4);
    memcpy(&gNfDataBuf[g_NfAttrib.PageSize+20], &g_ReservedBlockNum, 4);
    memset(&gNfDataBuf[g_NfAttrib.PageSize+24], 0xFF, g_NfAttrib.SprSize-g_NfAttrib.FtlSprSize);
    
    status = spiNf_PageProgram(pba,ppo,gNfDataBuf);
		
    LOG_INFO "[LEAVE]SpiNf_Write: \n" LOG_END		
    
	if(status)    return (LL_ERROR);
	else          return (LL_OK);
}

uint8_t SpiNf_WriteDouble(uint32_t pba, uint32_t ppo, uint8_t* pBuf0, uint8_t* pBuf1)
{
	uint8_t  status;
		
    LOG_INFO  "[ENTER]SpiNf_WriteDouble: \n"  LOG_END
    
    #ifdef SHOW_RWE_MSG
	printf("W2[%x,%x][%x,%x]\n",pba,ppo,pBuf0,pBuf1);
	//if( (pba>0) && (pba<5) && !ppo )	showData(pBuf0,1024);
	//if( (pba>0) && (pba<5) && !ppo )	showData(pBuf1,1024+24);
    #endif
    
    memcpy(&gNfDataBuf[0], pBuf0, g_NfAttrib.PageSize/2);
    memcpy(&gNfDataBuf[g_NfAttrib.PageSize/2], pBuf1, (g_NfAttrib.PageSize/2)+16);//g_NfAttrib.FtlSprSize);
    memcpy(&gNfDataBuf[g_NfAttrib.PageSize+16], &WRITE_SYMBOL, 4);
    memcpy(&gNfDataBuf[g_NfAttrib.PageSize+20], &g_ReservedBlockNum, 4);
    memset(&gNfDataBuf[g_NfAttrib.PageSize+24], 0xFF, g_NfAttrib.SprSize-g_NfAttrib.FtlSprSize);
    
    status = spiNf_PageProgram(pba,ppo,gNfDataBuf);  
		
    LOG_INFO "[LEAVE]SpiNf_WriteDouble: \n" LOG_END
    
	if(status)    return (LL_ERROR);
	else          return (LL_OK);
}

uint8_t SpiNf_LBA_Write(uint32_t pba, uint32_t ppo, uint8_t* pDtBuf, uint8_t* pSprBuf)
{
	uint8_t  status;
    
    LOG_INFO  "[ENTER]SpiNf_LBA_Write: \n"  LOG_END
    
    memcpy(&gNfDataBuf[0], pDtBuf, g_NfAttrib.PageSize);
    memcpy(&gNfDataBuf[g_NfAttrib.PageSize], pSprBuf, g_NfAttrib.FtlSprSize);
    
    status = spiNf_PageProgram(pba,ppo,gNfDataBuf);    
		
    LOG_INFO "[LEAVE]SpiNf_LBA_Write: \n" LOG_END
    
	if(status)    return (LL_ERROR);
	else          return (LL_OK);
}

uint8_t SpiNf_Read(uint32_t pba, uint32_t ppo, uint8_t* pBuf)
{
	uint8_t  status,rst=LL_ERASED;
	uint32_t i;
		
    LOG_INFO  "[ENTER]SpiNf_Read: \n"  LOG_END
    
    #ifdef SHOW_RWE_MSG
	printf("R1[%x,%x]\n",pba,ppo);
    #endif
    
    status = spiNf_PageRead(pba,ppo,gNfDataBuf);
    
    memcpy(pBuf, &gNfDataBuf[0], g_NfAttrib.PageSize+g_NfAttrib.FtlSprSize);
    
    if(g_ReservedBlockNum==SPINF_INVALID_REVBLK_SIZE)
    {
    	SPINF_ST_SPARE* pSpr = (SPINF_ST_SPARE*)&gNfDataBuf[g_NfAttrib.PageSize];
    	if (pSpr->goodCode == WRITE_SYMBOL)	g_ReservedBlockNum = pSpr->revBlkNum;
	}
		
	for(i=0;i<g_NfAttrib.FtlSprSize;i++)	if(gNfDataBuf[g_NfAttrib.PageSize+i]!=0xFF)	rst = LL_ERROR;
		
    LOG_INFO "[LEAVE]SpiNf_Read: \n" LOG_END		
    
	if(status)    return (LL_ERROR);
	else          
	{
		if(rst==LL_ERASED)	return (LL_ERASED);
		else				return (LL_OK);
	}
}

uint8_t SpiNf_LBA_Read(uint32_t pba, uint32_t ppo, uint8_t* pBuf)
{
	uint8_t  status;
		
    LOG_INFO  "[ENTER]SpiNf_LBA_Read: \n"  LOG_END
    
    #ifdef SHOW_RWE_MSG
	printf("R4[%x,%x]\n",pba,ppo);
    #endif

    status = spiNf_PageRead(pba,ppo,gNfDataBuf);    
    
    memcpy(pBuf, &gNfDataBuf[0], g_NfAttrib.PageSize+g_NfAttrib.FtlSprSize);
		
    LOG_INFO "[LEAVE]SpiNf_LBA_Read: \n" LOG_END
    
	if(status)    return (LL_ERROR);
	else          return (LL_OK);
}

uint8_t SpiNf_ReadOneByte(uint32_t pba, uint32_t ppo, uint8_t offset, uint8_t* pByte)
{
	uint8_t  status;
		
    LOG_INFO  "[ENTER]SpiNf_ReadOneByte: \n"  LOG_END
#ifdef ENABLE_NEW_READ_PART
    status = spiNf_ByteRead(pba, ppo, gNfDataBuf, g_NfAttrib.PageSize, g_NfAttrib.FtlSprSize);
    if(!status)	*pByte = gNfDataBuf[offset];
#else
    status = spiNf_PageRead(pba,ppo,gNfDataBuf);
    
    if(!status)	*pByte = gNfDataBuf[g_NfAttrib.PageSize+offset];
#endif
    
    #ifdef SHOW_RWE_MSG
	printf("R3[%x,%x,%x]=%02X, r=%x\n",pba,ppo,offset,*pByte,status);
    #endif    

    LOG_INFO "[LEAVE]SpiNf_Write: \n" LOG_END		
    
	if(status)    return (LL_ERROR);
	else          return (LL_OK);
}

uint8_t SpiNf_ReadPart(uint32_t pba, uint32_t ppo, uint8_t* pBuf, uint8_t index)
{
	uint8_t  status,rst=LL_ERASED;
    uint32_t *dBuf32;
	uint32_t i;
		
    LOG_INFO  "[ENTER]SpiNf_ReadPart: \n"  LOG_END
		
    #ifdef SHOW_RWE_MSG
	printf("R2[%x,%x,%x]\n",pba,ppo,index);
    #endif
    
    dBuf32 = (uint32_t*)gNfDataBuf;
#ifdef ENABLE_NEW_READ_PART
    switch(index)
    {
        case LL_RP_1STHALF:
            status = spiNf_ByteRead(pba, ppo, &gNfDataBuf[0], 0, g_NfAttrib.PageSize/2);
            for(i=0; i<(g_NfAttrib.PageSize/8); i++)
            {
                if(dBuf32[i]!=0xFFFFFFFF)
                {
                	rst = LL_ERROR;  
                	break;
                }
            }
            if(rst == LL_ERASED)
            {
                status = spiNf_ByteRead(pba, ppo, &gNfDataBuf[g_NfAttrib.PageSize], g_NfAttrib.PageSize, g_NfAttrib.FtlSprSize);
                for(i=(g_NfAttrib.PageSize/4); i<(g_NfAttrib.PageSize+g_NfAttrib.FtlSprSize)/4; i++)
                {
                    if(dBuf32[i]!=0xFFFFFFFF)	rst = LL_ERROR;
                }                
            }
            break;

        case LL_RP_2NDHALF:
            status = spiNf_ByteRead(pba, ppo, &gNfDataBuf[g_NfAttrib.PageSize/2], g_NfAttrib.PageSize/2, g_NfAttrib.PageSize/2);
            for(i=(g_NfAttrib.PageSize/8); i<(g_NfAttrib.PageSize/4); i++)
            {
                if(dBuf32[i]!=0xFFFFFFFF)
                {
                	rst = LL_ERROR;  
                	break;
                }
            }
            if(rst == LL_ERASED)
            {
                status = spiNf_ByteRead(pba, ppo, &gNfDataBuf[g_NfAttrib.PageSize], g_NfAttrib.PageSize, g_NfAttrib.FtlSprSize);
                for(i=(g_NfAttrib.PageSize/4); i<(g_NfAttrib.PageSize+g_NfAttrib.FtlSprSize)/4; i++)
                {
                    if(dBuf32[i]!=0xFFFFFFFF)	rst = LL_ERROR;
                }                
            }
            break;

        case LL_RP_DATA:
            status = spiNf_PageRead(pba,ppo,gNfDataBuf);            
            for(i=0;i<(g_NfAttrib.PageSize+g_NfAttrib.FtlSprSize)/4;i++)
            {
                if(dBuf32[i]!=0xFFFFFFFF)
                {
                	rst = LL_ERROR;  
                	break;
                }
            }        
            break;
            
        case LL_RP_SPARE:
            status = spiNf_ByteRead(pba, ppo, &gNfDataBuf[g_NfAttrib.PageSize], g_NfAttrib.PageSize, g_NfAttrib.FtlSprSize);
            for(i=(g_NfAttrib.PageSize/4); i<(g_NfAttrib.PageSize+g_NfAttrib.FtlSprSize)/4; i++)
            {
                if(dBuf32[i]!=0xFFFFFFFF)	rst = LL_ERROR;
            }            
            break; 

        default:
            //error read part index, do return error.
            printf("SPI NAND SpiNf_ReadPart() error, incorrect index(%x)\n",index);
            return 111;
            break;       
        
    }
#else

    status = spiNf_PageRead(pba,ppo,gNfDataBuf);
    
    if(index == LL_RP_DATA)
    {
        for(i=0;i<(g_NfAttrib.PageSize+g_NfAttrib.FtlSprSize)/4;i++)	if(dBuf32[i]!=0xFFFFFFFF)	rst = LL_ERROR;
        //printf("	dBuf32[%x]=%08x\n",(g_NfAttrib.PageSize+16),dBuf32[(g_NfAttrib.PageSize+16)/4]);
    }
    else
    {
        rst = LL_ERROR;
    }
#endif
    
    if(!status)
    {
    	switch(index)
    	{
            case LL_RP_1STHALF:
                #ifdef SHOW_RWE_MSG
                printf("R2.1[%x,%x,%x]\n", pBuf, &gNfDataBuf[0], g_NfAttrib.PageSize/2);
                #endif
                memcpy(pBuf, &gNfDataBuf[0], g_NfAttrib.PageSize/2);
                break;
            case LL_RP_2NDHALF:
                #ifdef SHOW_RWE_MSG
                printf("R2.2[%x,%x,%x]\n", pBuf, &gNfDataBuf[g_NfAttrib.PageSize/2], g_NfAttrib.PageSize/2);
                #endif
            	memcpy(pBuf, &gNfDataBuf[g_NfAttrib.PageSize/2], g_NfAttrib.PageSize/2);
            	break;
            case LL_RP_DATA:
                #ifdef SHOW_RWE_MSG
                printf("R2.3[%x,%x,%x]\n", pBuf, &gNfDataBuf[0], g_NfAttrib.PageSize);
                #endif
            	memcpy(pBuf, &gNfDataBuf[0], g_NfAttrib.PageSize);
            	break;
            case LL_RP_SPARE:
                #ifdef SHOW_RWE_MSG
                printf("R2.4[%x,%x,%x]\n", pBuf, &gNfDataBuf[g_NfAttrib.PageSize], g_NfAttrib.FtlSprSize);
                #endif
            	memcpy(pBuf, &gNfDataBuf[g_NfAttrib.PageSize], g_NfAttrib.FtlSprSize);
            	break;
            default:
                printf("[SPI NAND] error: unknown index=%x\n", index);
                break;
    	}
    }
    
    #ifdef SHOW_RWE_MSG_X
    if( (pba>0) && (pba<5) && (!ppo) && (index==1) )	showData(pBuf,1024);
    #endif
		
    LOG_INFO "[LEAVE]SpiNf_ReadPart: \n" LOG_END		

    #ifdef SHOW_RWE_MSG
    printf("R2.f, R=[%x,%x,%x]\n", status, rst, LL_ERASED);//0 2 1
    #endif

	if(status)    return (LL_ERROR);
	else          
	{
		if(rst==LL_ERASED)	return (LL_ERASED);
		else				return (LL_OK);
	}
}

uint8_t SpiNf_IsBadBlock(uint32_t pba)
{
	uint8_t  blockStatus = LL_ERROR;
	
    LOG_INFO "[ENTER]SpiNf_IsBadBlock: pba(%d), realAddress = 0x%08x\n", pba, ((pba * g_NfAttrib.PageInBlk)* SNF_PAGE_SIZE) LOG_END
    
    // ----------------------------------------
    // 0. PRECONDITION
    if ( pba >= g_NfAttrib.TotalBlk )
    {
        printf("\tLB_HWECC_IsBadBlock: ERROR! pba(%u) >= g_NfAttrib.numOfBlocks(%u)\n", pba, g_NfAttrib.TotalBlk);
        return LL_ERROR;
    }    

    // Reserve boot section
    if ( (pba >= 0) && (pba < g_ReservedBlockNum) )	
    {
    	printf("IBB[%x] is BAD block for reserved block,(%x,%x)\n", pba, g_ReservedBlockNum);
    	return LL_ERROR;
    }
    
    // ----------------------------------------
    // 2. Read first page
    {
    	MMP_UINT8	status;
        SPINF_ST_SPARE* pSpare = MMP_NULL;
        
        status = spiNf_PageRead(pba,0,gNfDataBuf);
        
        pSpare = (SPINF_ST_SPARE*)&gNfDataBuf[g_NfAttrib.PageSize];
        
        if (pSpare->goodCode == WRITE_SYMBOL)	blockStatus = LL_OK;
        else									blockStatus = _checkOriBadBlock(pba); 
    }
    
    //#ifdef SHOW_RWE_MSG
	printf("IBB[%x]=%x\n",pba,blockStatus);
    //#endif
    
    LOG_INFO "[LEAVE]SpiNf_IsBadBlock: \n" LOG_END    
    return blockStatus;
}

/*
 return:: LL_OK: GOOD block;  LL_ERROR: BAD block;
*/
uint8_t SpiNf_IsBadBlockForBoot(uint32_t pba)
{
	uint8_t  blockStatus = LL_ERROR;
	
    LOG_INFO "[ENTER]SpiNf_IsBadBlockForBoot: pba(%d), realAddress = 0x%08x\n", pba, ((pba * g_NfAttrib.PageInBlk)* SNF_PAGE_SIZE) LOG_END

    // ----------------------------------------
    // 0. PRECONDITION
    if ( pba >= g_NfAttrib.TotalBlk )
    {
        printf("\tSpiNf_IsBadBlockForBoot: ERROR! pba(%u) >= g_NfAttrib.numOfBlocks(%u)\n", pba, g_NfAttrib.TotalBlk);
        return LL_ERROR;
    }    

    // ----------------------------------------
    // 2. Read first page
    {
    	MMP_UINT8	status;
        SPINF_ST_SPARE* pSpare = MMP_NULL;
        
        status = spiNf_PageRead(pba,0,gNfDataBuf);       
        
        pSpare = (SPINF_ST_SPARE*)&gNfDataBuf[g_NfAttrib.PageSize];
        
        
        if (pSpare->goodCode == WRITE_SYMBOL)	blockStatus = LL_OK;
        else									blockStatus = _checkOriBadBlock(pba); 
        
        if (blockStatus != LL_OK)
            printf("    Spr->code = %08x,%08x\n",pSpare->goodCode,WRITE_SYMBOL);        
    }
    
    #ifdef SHOW_RWE_MSG
	printf("IBB fb[%x]=%x\n",pba,blockStatus);
	#else
	if (blockStatus != LL_OK)   printf("    IBB fb[%x]=%x\n",pba,blockStatus);
    #endif
    
    LOG_INFO "[LEAVE]SpiNf_IsBadBlockForBoot: \n" LOG_END    
    return blockStatus;
}

uint8_t SpiNf_SetReservedBlocks(uint32_t revBlks)
{
	uint8_t  rst = LL_ERROR;
	
	LOG_INFO  "[ENTER]SpiNf_SetReservedBlocks: \n"  LOG_END
	printf("SpiNf_SetReservedBlocks::%x\n",revBlks);
	
	if ( ithGetDeviceId() == 0x9850 )
	{
		//only IT9850 support SPI nand boot(i.e. support Reserved Block)
		if(g_ReservedBlockNum==SPINF_INVALID_REVBLK_SIZE)
		{
			g_ReservedBlockNum = revBlks;
			rst = LL_OK;
		}
		else
		{
			if(g_ReservedBlockNum==revBlks)	rst = LL_OK;	
			else	printf("SPINF error: Nf_revBlkSize: ori=%x, tgt=%x\n",g_ReservedBlockNum,revBlks);	
		}	
	}
	else
	{
		//IT9070, and others are not support Reserved Block function...
		//if revBlks!=0, then return fail.
		g_ReservedBlockNum = 0;
		if(!revBlks)	rst = LL_OK;
		else	printf("This chip[%04x] can not support reserved block of SPI nand\n",ithGetDeviceId());			
	}
	
    LOG_INFO "[LEAVE]SpiNf_SetReservedBlocks: \n" LOG_END	
	return rst;
}

void SpiNf_MarkAsBadBlk(uint32_t pba)
{
    uint8_t dBuf[4096+512] = {0};
    uint8_t rbBuf[4096+512] = {0};
    uint8_t res;
    uint32_t i;
    
    printf("    ### mark as bad, b=%x (%x,%x)###\n",pba,g_NfAttrib.PageSize,g_NfAttrib.FtlSprSize);
    
    //res = SpiNf_Write(pba, 0, dBuf, sBuf);
    res = spiNf_PageProgram(pba,0,dBuf);
    
    res = spiNf_PageRead(pba,0,rbBuf);
    
    //for(i=0; i<(g_NfAttrib.PageSize+g_NfAttrib.FtlSprSize); i++)
    for(i=0; i<(32); i++)
    {
        printf("%02x ",rbBuf[i]);
        if( (i&0x0F)==0x0F )    printf("\n");
        if( (i&0x3FF)==0x3FF )    printf("\n");
    }
    printf("\n");
}

void SpiNf_SetSpiCsCtrl(uint32_t csCtrl)
{
	spiNf_SetSpiCsCtrl(csCtrl);
}

