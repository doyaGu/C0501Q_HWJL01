/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL NOR functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <time.h>
#include "itp_cfg.h"
#include "nor/mmp_nor.h"
#include "ssp/mmp_spi.h"

#define NOR_SPI_PORT  SPI_0
#define NOR_SPI_CSN   SPI_CSN_0

typedef struct _ITPNor
{
    int initCount;
    int seekPos;
}ITPNor;

extern void iteFatNorFlush(void);
static unsigned long norBlockSizeInBytes;

static int _NorOpen(const char* name, int flags, int mode, void* info)
{
    ITPNor* norObj = (ITPNor*)info;

    if ( norObj )
   	{
        norObj->seekPos = 0;
   	    return 0;
   	}
   	else
   	{
   		errno = ENOENT;
		return -1;
   	}
}

static int _NorClose(int file, void* info)
{
    ITPNor* norObj = (ITPNor*)info;

    if ( norObj )
   	{
#if 0
   		if ( NorTerminate() == 0 )
   		{
   			// Success
   			norObj->initCount--;
   			return 0;
   		}
   		else
   		{
   			errno = ENOENT;
   			return -1;
   		}
#endif
		norObj->initCount--;
   		return 0;
   	}
   	else
   	{
   		errno = ENOENT;
		return -1;
   	}
}

static int _NorRead(int file, char *ptr, int len, void* info)
{
	ITPNor* norObj = (ITPNor*)info;

    if ( norObj )
    {
    	uint32_t norResult  = 0;
    	int32_t  norSize    = (int32_t)NorGetCapacity(NOR_SPI_PORT, NOR_SPI_CSN);
    	int	     readLength = len * norBlockSizeInBytes;

    	if ( norObj->seekPos >= norSize )
    	{
    		return 0;
    	}

    	if ( (norObj->seekPos + (len * norBlockSizeInBytes) ) >= norSize )
    	{
    		readLength = (norObj->seekPos + len * norBlockSizeInBytes) - norSize;
    	}

    #ifdef CFG_NOR_SHARE_GPIO	
        mmpSpiSetControlMode(SPI_CONTROL_NOR);
    #endif

    	norResult = NorRead(NOR_SPI_PORT, NOR_SPI_CSN, norObj->seekPos, ptr, readLength);		

    #ifdef CFG_NOR_SHARE_GPIO
        mmpSpiResetControl();
    #endif
    	
    	if ( norResult == 0 )
    	{
    		// Success
    		norObj->seekPos += readLength;
    		return readLength / norBlockSizeInBytes;
    	}
    	else
    	{
    		errno = EIO;
    		return -1;
    	}
    }
    else
	{
		errno = EBADF;
		return -1;
	}
}

static int _NorWrite(int file, char *ptr, int len, void* info)
{
    ITPNor* norObj = (ITPNor*)info;

    if ( norObj )
    {
    	uint32_t norResult  = 0;
    	int32_t  norSize    = (int32_t)NorGetCapacity(NOR_SPI_PORT, NOR_SPI_CSN);
    	int	     writeLength = len * norBlockSizeInBytes;

    	if ( norObj->seekPos >= norSize )
    	{
    		return 0;
    	}

    	if ( (norObj->seekPos + (len * norBlockSizeInBytes) ) >= norSize )
    	{
    		writeLength = (norObj->seekPos + len * norBlockSizeInBytes) - norSize;
    	}
    	
    #ifdef CFG_NOR_SHARE_GPIO
        mmpSpiSetControlMode(SPI_CONTROL_NOR);
    #endif
    	
    	norResult = NorWrite(NOR_SPI_PORT, NOR_SPI_CSN, norObj->seekPos, ptr, writeLength);
	
    #ifdef CFG_NOR_SHARE_GPIO
        mmpSpiResetControl();
    #endif

    	if ( norResult == 0 )
    	{
    		// Success
    		norObj->seekPos += writeLength;
    		return writeLength / norBlockSizeInBytes;
    	}
    	else
    	{
    		errno = EIO;
    		return -1;
    	}
    }
    else
	{
		errno = EBADF;
		return -1;
	}
}

static int _NorLseek(int file, int ptr, int dir, void* info)
{
    ITPNor* norObj = (ITPNor*)info;

    if ( norObj )
    {
    	switch(dir)
	    {
	    case SEEK_SET:
	        norObj->seekPos = ptr * norBlockSizeInBytes;
	        break;
	        
	    case SEEK_CUR:
	        norObj->seekPos += ptr * norBlockSizeInBytes;
	        break;
	        
	    case SEEK_END:
	    	{
	    		uint32_t norSize = NorGetCapacity(NOR_SPI_PORT, NOR_SPI_CSN);

	    		norObj->seekPos = norSize;
	    	}
	        break;

	     default:
	     	return -1;
	     	break;
	    }

	    return norObj->seekPos / norBlockSizeInBytes;
    }
    else
    {
    	return -1;
    }
}

#if defined(CFG_FS_FAT) && CFG_NOR_CACHE_FLUSH_INTERVAL > 0

static struct timeval norLastTime;

static void NorFlushHandler(void)
{
    struct timeval currTime;
    
    gettimeofday(&currTime, NULL);
    
    if (itpTimevalDiff(&norLastTime, &currTime) >= CFG_NOR_CACHE_FLUSH_INTERVAL * 1000)
    {
        LOG_DBG "Flush NOR cache!\n" LOG_END
        iteFatNorFlush();
        gettimeofday(&norLastTime, NULL);
    }
}
#endif // defined(CFG_FS_FAT) && CFG_NOR_CACHE_FLUSH_INTERVAL > 0

static void NorInit(void)
{
#if defined(CFG_FS_FAT) && CFG_NOR_CACHE_FLUSH_INTERVAL > 0
    itpRegisterIdleHandler(NorFlushHandler);
    gettimeofday(&norLastTime, NULL);
#endif // defined(CFG_FS_FAT) && CFG_NOR_CACHE_FLUSH_INTERVAL > 0

    NorInitial(NOR_SPI_PORT, NOR_SPI_CSN);	
    norBlockSizeInBytes = NorGetAttitude(NOR_SPI_PORT, NOR_SPI_CSN, NOR_ATTITUDE_PAGE_SIZE) * NorGetAttitude(NOR_SPI_PORT, NOR_SPI_CSN, NOR_ATTITUDE_PAGE_PER_SECTOR) * NorGetAttitude(NOR_SPI_PORT, NOR_SPI_CSN, NOR_ATTITUDE_SECTOR_PER_BLOCK);
}

static int _NorIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_FLUSH:
    #ifdef CFG_FS_FAT
        iteFatNorFlush();
    #endif
        break;
        
    case ITP_IOCTL_INIT:
        NorInit();
        break;

    case ITP_IOCTL_GET_BLOCK_SIZE:
        *(unsigned long*)ptr = norBlockSizeInBytes;
        break;

    case ITP_IOCTL_GET_GAP_SIZE:
        *(unsigned long*)ptr = 0;
        break;

    default:
        errno = (ITP_DEVICE_NOR << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

static ITPNor ItpNor = {0, 0};
const ITPDevice itpDeviceNor =
{
    ":nor",
    _NorOpen,
    _NorClose,
    _NorRead,
    _NorWrite,
    _NorLseek,
    _NorIoctl,
    &ItpNor
};
