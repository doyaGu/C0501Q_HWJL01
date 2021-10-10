/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL XD functions.
 *
 * @author Jim Tan
 * @version 1.0
 *
 */
#include <errno.h>
#include "ite/itp.h"
#include "itp_cfg.h"
#include "ite/ite_xd.h"

/********************
 * MACRO defination *
 ********************/
#define ITP_XD_PAGE_SIZE		(512) 
#define ITP_XD_PAGES_OF_BLOCK	(32)
#define ITP_XD_BLOCK_SIZE		(16384)
#define ITP_XD_LOGICAL_BLK_NUM	(1000)

/********************
 * global variable *
 ********************/
/*
typedef struct NF_INFO_TAG
{
	uint32_t   Init;
	uint32_t   CurrSector;
	uint32_t   NumOfBlk;
}ITE_XD_INFO;
*/

static ITE_XD_INFO	itpXdInfo;

/********************
 * private function *
 ********************/


/********************
 * public function *
 ********************/
static int XdOpen(const char* name, int flags, int mode, void* info)
{
    // TODO: IMPLEMENT

    //to get XD attribution

    //errno = ENOENT;
    return 0;
}

static int XdClose(int file, void* info)
{
    //errno = ENOENT;
    //errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
    return 0;
}

static int XdRead(int file, char *ptr, int len, void* info)
{
    // TODO: IMPLEMENT
    int  DoneLen = 0;
	uint32_t  blk = 0;
	uint32_t  pg = 0;
    char *databuf;
	char *tmpbuf;
	char *srcbuf=ptr;	
    
    if( !ptr || !len )
    {
    	errno = (ITP_DEVICE_XD << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//ptr or len error
    	LOG_ERR "ptr or len error: \n" LOG_END
    	goto end;
    }
    
    if(!itpXdInfo.NumOfBlk)
    {
    	errno = (ITP_DEVICE_XD << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//XD initial fail
    	LOG_ERR "XD block initial error: \n" LOG_END
    	goto end;
    }

    if( !iteXdRead( itpXdInfo.CurrSector, len, ptr ) )
    {
    	errno = (ITP_DEVICE_XD << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//XD read fail
    	LOG_ERR "XD read error: \n" LOG_END
    	goto end;
    }
end:
	/*
	if(databuf)
	{
		free(databuf);
		databuf = NULL;
	}	
	*/
    return DoneLen;
}

static int XdWrite(int file, char *ptr, int len, void* info)
{
    int  DoneLen = 0;
    char *DataBuf;
	char *tmpDataBuf;
	char *tmpSrcBuf = ptr;
    char *SprBuf= NULL;
    uint32_t blk,pg;
    
    // TODO: IMPLEMENT
    
    if( !ptr || !len )
    {
    	errno = (ITP_DEVICE_XD << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//ptr or len error
    	LOG_ERR "ptr or len error: \n" LOG_END
    	goto end;
    }
    
    if(!itpXdInfo.NumOfBlk)
    {
    	errno = (ITP_DEVICE_XD << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//XD initial fail
    	LOG_ERR "XD block initial error: \n" LOG_END
    	goto end;
    }

    if( !iteXdWrite( itpXdInfo.CurrSector, len, ptr ) )
    {
    	errno = (ITP_DEVICE_XD << ITP_DEVICE_ERRNO_BIT) | __LINE__;	//XD read fail
    	LOG_ERR "XD write error: \n" LOG_END
    	goto end;
    }
end:
	/*
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
	*/
    return DoneLen;
}

static int XdLseek(int file, int ptr, int dir, void* info)
{
    switch(ptr)
    {
    default:
    case SEEK_SET:
        itpXdInfo.CurrSector = dir;
        break;
    case SEEK_CUR:
        itpXdInfo.CurrSector += dir;
        break;
    case SEEK_END:
        break;
    }
    return itpXdInfo.CurrSector;
}

static int XdIoctl(int file, unsigned long request, void* ptr, void* info)
{
	int res = -1;
	
    switch (request)
    {
	    case ITP_IOCTL_INIT:
	    	//printf("ITP_IOCTL_INIT[01](%x,%x)\n",&itpXdInfo,itpXdInfo.NumOfBlk);
	        if( iteXdInitial()==true )
	        {
	        	int pagesize,TotalSecNum;
	        	
	        	iteXdGetCapacity( &TotalSecNum, &pagesize);
	        	itpXdInfo.NumOfBlk = (TotalSecNum / ITP_XD_PAGES_OF_BLOCK) / ITP_XD_LOGICAL_BLK_NUM;
				itpXdInfo.TotalSecNum = TotalSecNum;
	        	//printf("ITP_IOCTL_INIT[02](%x,%x)(%x,%x)\n",&itpXdInfo, itpXdInfo.NumOfBlk, itpXdInfo.BootRomSize, CFG_UPGRADE_IMAGE_POS);
	        	res = 0;
	        }
	        break;
	
	    case ITP_IOCTL_GET_BLOCK_SIZE:
	        if(itpXdInfo.NumOfBlk)
	        {
	        	*(unsigned long*)ptr = ITP_XD_PAGE_SIZE;
	        	res = 0;
	        }
	        else
	        {
	        	*(unsigned long*)ptr = 0;
	        }
	        break;

	    default:
	        errno = (ITP_DEVICE_XD << ITP_DEVICE_ERRNO_BIT) | 1;
	        break;
    }
    return res;
}

const ITPDevice itpDeviceXd =
{
    ":xd",
    XdOpen,
    XdClose,
    XdRead,
    XdWrite,
    XdLseek,
    XdIoctl,
    NULL
};
