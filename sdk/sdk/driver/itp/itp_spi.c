/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL SPI functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include "itp_cfg.h"
#include "ssp/mmp_spi.h"

typedef struct _ITPSpi
{
    SPI_PORT	port;
    int			initCount;
}ITPSpi;

//#ifdef CFG_NOR_SHARE_GPIO_X
//static const unsigned int shareGpioPin[3]={ CFG_GPIO_SHARE_SPI_FOR_NOR,
//											CFG_GPIO_SHARE_SPI_FOR_SLAVE,
//											CFG_GPIO_SHARE_SPI_FOR_NAND};
//#else

#ifdef CFG_NOR_SHARE_SPI_NAND
static const unsigned int shareGpioPin[3] = {19, 0, 24};
#else
static const unsigned int shareGpioPin[3] = {50, 51, 0};
#endif

//#endif

static int Spi_Open(const char* name, int flags, int mode, void* info)
{
    
    ITPSpi* spiObj = (ITPSpi*)info;
	
	if ( spiObj )
	{
		mmpSpiInitialize(spiObj->port, SPI_OP_MASTR, CPO_0_CPH_0, SPI_CLK_20M);
		spiObj->initCount++;
		return (int)0;
	}
    else
    {
	    errno = ENOENT;
	    return -1;
    }
}

static int SpiClose(int file, void* info)
{
 
    ITPSpi* spiObj = (ITPSpi*)info;

	if ( spiObj )
	{
		mmpSpiTerminate(spiObj->port);
		spiObj->initCount--;
		return 0;
	}
	else
	{
	    errno = ENOENT;
	    return -1;
	}
}

static int SpiRead(int file, char *ptr, int len, void* info)
{
    
    ITPSpi*     spiObj  = (ITPSpi*)info;
	ITPSpiInfo* spiInfo = (ITPSpiInfo*)ptr;
		
	if ( spiObj )
	{
		int readIndex = 0;
		int readCount = 0;
		
		for ( readIndex = 0; readIndex < len; readIndex++ )
		{
			ITPSpiInfo* spiInfo = (ITPSpiInfo*)(ptr + (sizeof(ITPSpiInfo) * readIndex));
			
			if (   spiInfo
			    && spiInfo->cmdBuffer
	            && spiInfo->dataBuffer )
	        {
	        	int32_t spiResult = 0xFFFFFFFF;
	        	
	        	switch(spiInfo->readWriteFunc)
	        	{
	        	case ITP_SPI_PIO_READ:					
					spiResult = mmpSpiPioRead(
                                    spiObj->port,
                                    spiInfo->cmdBuffer,
                                    spiInfo->cmdBufferSize,
                                    spiInfo->dataBuffer,
                                    spiInfo->dataBufferSize,
                                    spiInfo->dataLength);					
	        		break;

	        	case ITP_SPI_DMA_READ:
	        		spiResult = mmpSpiDmaRead(
                                    spiObj->port,
                                    spiInfo->cmdBuffer,
                                    spiInfo->cmdBufferSize,
                                    spiInfo->dataBuffer,
                                    spiInfo->dataBufferSize,
                                    spiInfo->dataLength);
	        		break;

	        	default:
	        		break;
	        	}

				if ( spiResult == true )
		       	{
		       		// Success
		       		readCount += spiInfo->dataBufferSize;
		       	}
				else
					printf("itp spi read fail\n");
	        }
		}

		return readCount;
	}
	else
	{
		return 0;
	}
}

static int SpiWrite(int file, char *ptr, int len, void* info)
{
    
	ITPSpi*     spiObj  = (ITPSpi*)info;
	ITPSpiInfo* spiInfo = (ITPSpiInfo*)ptr;

	if ( spiObj )
	{
		int writeIndex = 0;
		int writeCount = 0;
		
		for ( writeIndex = 0; writeIndex < len; writeIndex++ )
		{
			ITPSpiInfo* spiInfo = (ITPSpiInfo*)(ptr + (sizeof(ITPSpiInfo) * writeIndex));
			
			if (   spiInfo
			    && spiInfo->cmdBuffer
	            && spiInfo->dataBuffer )
	        {
	        	int32_t spiResult = 0xFFFFFFFF;
	        	
	        	switch(spiInfo->readWriteFunc)
	        	{
	        	case ITP_SPI_DMA_WRITE:
	        		spiResult = mmpSpiDmaWrite(
		            	spiObj->port, 
		            	spiInfo->cmdBuffer, 
		            	spiInfo->cmdBufferSize,
		            	spiInfo->dataBuffer, 
		            	spiInfo->dataBufferSize,
		            	spiInfo->dataLength);
	        		break;

	        	case ITP_SPI_PIO_WRITE:
	        		spiResult = mmpSpiPioWrite(
	        			spiObj->port, 
		            	spiInfo->cmdBuffer, 
		            	spiInfo->cmdBufferSize,
		            	spiInfo->dataBuffer, 
		            	spiInfo->dataBufferSize,
		            	spiInfo->dataLength);					
	        		break;

	        	default:
	        		break;
	        	}

				if ( spiResult == true )
		       	{
		       		// Success
		       		writeCount += spiInfo->dataBufferSize;
		       	}
				else
					printf("itp spi write fail\n");
	        }
		}

		return writeCount;
	}
	else
	{
		return 0;
	}
}

static int SpiIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:	        
        // Init in open()        
        #if defined(CFG_NOR_SHARE_GPIO) || defined(CFG_NOR_SHARE_SPI_NAND)
        mmpSpiInitShareGpioPin(shareGpioPin);
        #endif
        break;
        
    default:
        errno = (ITP_DEVICE_SPI << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

static ITPSpi ItpSpi0 = {SPI_0, 0};
const ITPDevice itpDeviceSpi0 =
{
    ":spi0",
    Spi_Open,
    SpiClose,
    SpiRead,
    SpiWrite,
    itpLseekDefault,
    SpiIoctl,
    &ItpSpi0
};

static ITPSpi ItpSpi1 = {SPI_1, 0};
const ITPDevice itpDeviceSpi1 =
{
    ":spi1",
    Spi_Open,
    SpiClose,
    SpiRead,
    SpiWrite,
    itpLseekDefault,
    SpiIoctl,
    &ItpSpi1
};

