/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL I2C functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include "itp_cfg.h"
#include "iic/mmp_iic.h"

typedef struct _ITPI2c
{
    int initCount;
}ITPI2c;

#define IIC_DELAY	20

static int I2cOpen(const char* name, int flags, int mode, void* info)
{
    IIC_PORT iic_port = (IIC_PORT)info;

	return (int)0;
}

static int I2cClose(int file, void* info)
{
    IIC_PORT iic_port = (IIC_PORT)info;

	return 0;
}

static int I2cRead(int file, char *ptr, int len, void* info)
{
    IIC_PORT iic_port = (IIC_PORT)info;

    int readIndex = 0;
	int readCount = 0;

	mmpIicLockModule(iic_port);

	for ( readIndex = 0; readIndex < len; readIndex++ )
	{
		ITPI2cInfo* i2cInfo   = (ITPI2cInfo*)(ptr + (sizeof(ITPI2cInfo) * readIndex));
		uint32_t    i2cResult = 0;

		if (   i2cInfo
		    && i2cInfo->cmdBuffer
		    && i2cInfo->dataBuffer )
	    {
	    	i2cResult = mmpIicReceiveDataEx(
				iic_port,
	    		IIC_MASTER_MODE,
	    		i2cInfo->slaveAddress,
	    		i2cInfo->cmdBuffer,
	    		i2cInfo->cmdBufferSize,
	    		i2cInfo->dataBuffer,
	    		i2cInfo->dataBufferSize);
	    		
	    	i2cInfo->errCode = i2cResult;
	    	if ( i2cResult == 0 )
	    	{
	    		// Success
	    		readCount += i2cInfo->dataBufferSize;
	    	}
	    }
	}

	mmpIicReleaseModule(iic_port);

	return readCount;
}

static int I2cWrite(int file, char *ptr, int len, void* info)
{
	IIC_PORT iic_port = (IIC_PORT)info;

    int writeIndex = 0;
	int writeCount = 0;

	mmpIicLockModule(iic_port);

	for ( writeIndex = 0; writeIndex < len; writeIndex++ )
	{
		ITPI2cInfo* i2cInfo   = (ITPI2cInfo*)(ptr + (sizeof(ITPI2cInfo) * writeIndex));
		uint32_t    i2cResult = 0;

		if (   i2cInfo
		    && i2cInfo->cmdBuffer )
	    {
	    	i2cResult = mmpIicSendDataEx(
				iic_port,
	    		IIC_MASTER_MODE,
	    		i2cInfo->slaveAddress,
	    		i2cInfo->cmdBuffer,
	    		i2cInfo->cmdBufferSize);
	    	
	    	i2cInfo->errCode = i2cResult;
	    	if ( i2cResult == 0 )
	    	{
	    		// Success
	    		writeCount += i2cInfo->cmdBufferSize;
	    	}
	    }
	}

	mmpIicReleaseModule(iic_port);

	return writeCount;
}

static int I2cIoctl(int file, unsigned long request, void* ptr, void* info)
{
	IIC_PORT iic_port = (IIC_PORT)info;
	IIC_OP_MODE iic_mode = (IIC_OP_MODE)ptr;
	
    switch (request)
    {
    case ITP_IOCTL_INIT:
		mmpIicInitialize(iic_port, iic_mode, 0, 0, 400 * 1000, 0);
        break;

    case ITP_IOCTL_EXIT:
        mmpIicTerminate(iic_port);
        break;

    case ITP_IOCTL_RESET:
        mmpIicTerminate(iic_port);
        mmpIicInitialize(iic_port, iic_mode, 0, 0, 400 * 1000, 0);
        //mmpIicSetClockRate(iic_port, 100 * 1024);
        break;

    default:
		if(iic_port == IIC_PORT_0)
        	errno = (ITP_DEVICE_I2C0 << ITP_DEVICE_ERRNO_BIT) | 1;
		else
			errno = (ITP_DEVICE_I2C1 << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceI2c0 =
{
    ":i2c0",
    I2cOpen,
    I2cClose,
    I2cRead,
    I2cWrite,
    itpLseekDefault,
    I2cIoctl,
    (void*)IIC_PORT_0
};

const ITPDevice itpDeviceI2c1 =
{
    ":i2c1",
    I2cOpen,
    I2cClose,
    I2cRead,
    I2cWrite,
    itpLseekDefault,
    I2cIoctl,
    (void*)IIC_PORT_1
};

const ITPDevice itpDeviceI2c2 =
{
    ":i2c2",
    I2cOpen,
    I2cClose,
    I2cRead,
    I2cWrite,
    itpLseekDefault,
    I2cIoctl,
    (void*)IIC_PORT_2
};
