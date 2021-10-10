/*
 * Copyright (c) 2013 ITE Technology Corp. All Rights Reserved.
 */
/** @file xcpu_io.c
 *
 * @version 0.1
 */

#include "xcpu_io.h"
#include "bus.h"
#include "itx.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MEM_ADDRESS_HI      0x206
#define PREREAD_ADDRESS_LO  0x208
#define PREREAD_ADDRESS_HI  0x20A
#define PREREAD_LENGTH      0x20C
#define PREREAD_FIRE        0x20E
#define REGISTER_ADDR_MASK  0x7FFF //[15]=0 host read mmio register  
#define MEMORY_ADDR_MASK    0x8000 //[15]=1 host read memory  
#define MAX_PREREAD_COUNT   0xF000

//=============================================================================
//                              Macro Definition
//=============================================================================

#define ENDIAN_AUTO         (0)
#define ENDIAN_LITTLE       (1)
#define ENDIAN_BIG          (2)

#define ENDIAN              ENDIAN_AUTO // 0: auto, 1: little, 2: big

#define MAX_RETRY_TIME      (3)

#define EndianSwap32(x) \
        (((x) & 0x000000FF) << 24) | \
        (((x) & 0x0000FF00) << 8) | \
        (((x) & 0x00FF0000) >> 8) | \
        (((x) & 0xFF000000) >> 24)

#define EndianSwap16(x) \
        (((x) & (MMP_UINT16) 0x00FF) << 8) | \
        (((x) & (MMP_UINT16) 0xFF00) >> 8)
//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================
#if (ITX_BUS_TYPE == ITX_BUS_I2C)
extern MMP_UINT8  gITXs_IIC_ID;
#endif

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static MMP_INLINE MMP_UINT16
_UINT16LE_To_UINT16BE(
    MMP_UINT16 int16le)
{
    return ((int16le & 0x00FF) << 8)
         + ((int16le & 0xFF00) >> 8);
}

static MMP_INLINE MMP_UINT16
_UINT16BE_To_UINT16LE(
    MMP_UINT16 int16be)
{
    return ((int16be & 0x00FF) << 8)
         + ((int16be & 0xFF00) >> 8);
}

static MMP_INLINE MMP_UINT32
_UINT32LE_To_UINT32BE(
    MMP_UINT32 int32le)
{
    return ((int32le & 0x000000FFL) << 24)
         + ((int32le & 0x0000FF00L) <<  8)
         + ((int32le & 0x00FF0000L) >>  8)
         + ((int32le & 0xFF000000L) >> 24);
}

static MMP_INLINE MMP_UINT32
_UINT32BE_To_UINT32LE(
    MMP_UINT32 int32be)
{
    return ((int32be & 0x000000FFL) << 24)
         + ((int32be & 0x0000FF00L) <<  8)
         + ((int32be & 0x00FF0000L) >>  8)
         + ((int32be & 0xFF000000L) >> 24);
}

#if (ENDIAN == ENDIAN_AUTO)
MMP_BOOL
_IsBigEndian(
    void);
#endif

#if (ITX_BUS_TYPE == ITX_BUS_I2C)
static MMP_RESULT
_xCpuIO_I2CReadRegister(
    MMP_UINT8   slaveAddr,
    MMP_UINT16  startAddr,
    MMP_UINT8*  pdata)
{
    MMP_RESULT  result = MMP_RESULT_SUCCESS;
    MMP_UINT16  ite_addr = 0;
    
    ite_addr = (startAddr >> 1) & REGISTER_ADDR_MASK;       
    result = busI2CRead(slaveAddr, ite_addr, (MMP_UINT8 *)pdata, sizeof(MMP_UINT16));
    if (result != MMP_RESULT_SUCCESS)
    {
        //printf("%s()#%d Error. Error=0x%08x\n", __FUNCTION__, __LINE__, result);
        return result;
    }  
        
#if (ENDIAN == ENDIAN_AUTO)
    if (_IsBigEndian())
#endif
#if (ENDIAN == ENDIAN_AUTO || ENDIAN == ENDIAN_BIG)
    {
        *((MMP_UINT16*)pdata) = _UINT16LE_To_UINT16BE(*((MMP_UINT16*)pdata));
    }
#endif
    return result;
}

static MMP_RESULT
_xCpuIO_I2CWriteRegister(
    MMP_UINT8   slaveAddr,
    MMP_UINT16  startAddr,
    MMP_UINT16  data)
{
    MMP_RESULT result = MMP_RESULT_SUCCESS;
    MMP_UINT16  ite_addr = 0;
    
    ite_addr = (startAddr >> 1) & REGISTER_ADDR_MASK;        

#if (ENDIAN == ENDIAN_AUTO)
    if (_IsBigEndian())
#endif
#if (ENDIAN == ENDIAN_AUTO || ENDIAN == ENDIAN_BIG)
    {
        data = _UINT16BE_To_UINT16LE(data);
    }
#endif

    result = busI2CWrite(slaveAddr, ite_addr, (MMP_UINT8 *)&data, sizeof(MMP_UINT16));
    if (result != MMP_RESULT_SUCCESS)
    {
        //printf("%s()#%d Error. Error=0x%08x\n", __FUNCTION__, __LINE__, result);
        return result;
    }     
    
    return result;
}

static MMP_RESULT
_xCpuIO_I2CReadMemory(
    MMP_UINT8   slaveAddr,
    MMP_UINT32  startAddr,
    MMP_UINT8*  pdata,
    MMP_UINT32  sizeinbyte)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT    data;
    MMP_UINT    remainlen = sizeinbyte, preSize;
    MMP_UINT32  srcAddr = startAddr;
    MMP_UINT8*  ptrdata = pdata;
	MMP_UINT16  ite_addr = 0;
	    
	while (remainlen)
	{
		preSize = (remainlen > MAX_PREREAD_COUNT)
				? MAX_PREREAD_COUNT
				: remainlen;

        //pre-read lo address 
        data = (srcAddr & 0x0000FFFF);   
        result = _xCpuIO_I2CWriteRegister(slaveAddr, PREREAD_ADDRESS_LO, (MMP_UINT16)data);        
        if (result != MMP_RESULT_SUCCESS)
        {
            //printf("%s()#%d Error. Error=0x%08x\n", __FUNCTION__, __LINE__, result);
            return result;
        } 
    
        //pre-read hi address 
        data = ((srcAddr & 0xFFFF0000) >> 16);    
        result = _xCpuIO_I2CWriteRegister(slaveAddr, PREREAD_ADDRESS_HI, (MMP_UINT16)data);
        if (result != MMP_RESULT_SUCCESS)
        {
            //printf("%s()#%d Error. Error=0x%08x\n", __FUNCTION__, __LINE__, result);
            return result;
        } 

        //pre-read length					            
    	data = (preSize / 2) - 1;
    	result = _xCpuIO_I2CWriteRegister(slaveAddr, PREREAD_LENGTH, (MMP_UINT16)data);
        if (result != MMP_RESULT_SUCCESS)
    {
            //printf("%s()#%d Error. Error=0x%08x\n", __FUNCTION__, __LINE__, result);
            return result;
        }     
        
        //pre-read fire
        data = 0x8007;
    	result = _xCpuIO_I2CWriteRegister(slaveAddr, PREREAD_FIRE, (MMP_UINT16)data);
        if (result != MMP_RESULT_SUCCESS)
        {
            //printf("%s()#%d Error. Error=0x%08x\n", __FUNCTION__, __LINE__, result);
            return result;
        } 
                        
    	ite_addr = ((0x0 >> 1) | MEMORY_ADDR_MASK);    	    	
    	result = busI2CRead(slaveAddr, ite_addr, ptrdata, preSize); 
        if (result != MMP_RESULT_SUCCESS)
        {
            //printf("%s()#%d Error. Error=0x%08x\n", __FUNCTION__, __LINE__, result);
            return result;
        } 
        
		remainlen -= preSize;
		srcAddr += preSize;
		ptrdata += preSize;   
    }    
        
    return result;
}

static MMP_RESULT
_xCpuIO_I2CWriteMemory(
    MMP_UINT8   slaveAddr,
    MMP_UINT32  startAddr,
    MMP_UINT8*  pdata,
    MMP_UINT32  sizeinbyte)
{
    MMP_RESULT  result = MMP_RESULT_SUCCESS;
    MMP_UINT32  ite_addr = 0, remainlen, writelen;
    MMP_UINT16  membase_hi = 0, membase_lo = 0;
    MMP_UINT8*  ptrdata;
    
    membase_hi = (MMP_UINT16)((startAddr & 0xFFFF0000) >> 16);    
    membase_lo = (MMP_UINT16)(startAddr & 0x0000FFFF);    
    
    writelen  = (sizeinbyte > (0x10000 - (MMP_UINT32)membase_lo)) ? (0x10000 - (MMP_UINT32)membase_lo) : sizeinbyte;
    remainlen = sizeinbyte;
    ptrdata = (MMP_UINT8 *)pdata;

    while(remainlen > 0)
    {
        result = _xCpuIO_I2CWriteRegister(gITXs_IIC_ID, MEM_ADDRESS_HI, membase_hi);
        if (result != MMP_RESULT_SUCCESS)
        {
            //printf("%s()#%d Error. Error=0x%08x\n", __FUNCTION__, __LINE__, result);
            return result;
        } 
                        
        ite_addr = (membase_lo >> 1) | MEMORY_ADDR_MASK;   
        result = busI2CWrite(slaveAddr, ite_addr, ptrdata, writelen);
        if (result != MMP_RESULT_SUCCESS)
        {
            //printf("%s()#%d Error. Error=0x%08x\n", __FUNCTION__, __LINE__, result);
            return result;
        } 
            
        membase_hi++;
        membase_lo = 0;
        ptrdata += writelen;
        remainlen -= writelen;
        writelen = ((remainlen > 0x10000) ? 0x10000 : remainlen);
    }
    
    return result;
}
#endif

#if (ITX_BUS_TYPE == ITX_BUS_SPI)
static MMP_RESULT
_xCpuIO_SPIReadRegister(
    MMP_UINT16  startAddr,
    MMP_UINT8*  pdata)
{
    MMP_RESULT  result = MMP_SUCCESS;
	MMP_UINT8   txBuf[3];
	MMP_UINT16  ite_addr = 0;
	
	ite_addr = (startAddr >> 1) & REGISTER_ADDR_MASK;
	
	txBuf[0] = 0x1;
    txBuf[1] = (MMP_UINT8)((ite_addr & 0x00FF));
    txBuf[2] = (MMP_UINT8)((ite_addr & 0xFF00) >> 8);

    result = busSPIRead(txBuf, 3, pdata, sizeof(MMP_UINT16));
    if (result != MMP_RESULT_SUCCESS)
    {
        //printf("%s (SPI Read) Error. Error=0x%08x\n", __FUNCTION__, result);
        return result;
    } 
    
#if (ENDIAN == ENDIAN_AUTO)
    if (_IsBigEndian())
#endif
#if (ENDIAN == ENDIAN_AUTO || ENDIAN == ENDIAN_BIG)
    {
        *((MMP_UINT16*)pdata) = _UINT16LE_To_UINT16BE(*((MMP_UINT16*)pdata));
    }
#endif
    return result;
}

static MMP_RESULT
_xCpuIO_SPIWriteRegister(
    MMP_UINT16  startAddr,
    MMP_UINT16  data)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT8   txBuf[3];
    MMP_UINT16  ite_addr = 0;
    
    ite_addr = (startAddr >> 1) & REGISTER_ADDR_MASK;

    txBuf[0] = 0x0;
    txBuf[1] = (MMP_UINT8)((ite_addr & 0x00FF));
    txBuf[2] = (MMP_UINT8)((ite_addr & 0xFF00) >> 8);

#if (ENDIAN == ENDIAN_AUTO)
    if (_IsBigEndian())
#endif
#if (ENDIAN == ENDIAN_AUTO || ENDIAN == ENDIAN_BIG)
    {
        data = _UINT16BE_To_UINT16LE(data);
    }
#endif

    result = busSPIWrite(txBuf, 3, (MMP_UINT8*)&data, sizeof(MMP_UINT16));
    if (result != MMP_RESULT_SUCCESS)
    {
        //printf("%s (SPI Write) Error. Error=0x%08x\n", __FUNCTION__, result);
        return result;
    } 
    
    return result;
}

static MMP_RESULT
_xCpuIO_SPIReadMemory(
    MMP_UINT32  startAddr,
    MMP_UINT8*  pdata,
    MMP_UINT32  size)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT16  data;
    MMP_UINT    sizeInByte = size, preSize;
    MMP_UINT32  srcAddr = startAddr;
    MMP_UINT8*  pdest = pdata;
	MMP_UINT8   txBuf[3];
	MMP_UINT16  ite_addr = 0;
	  
	while (sizeInByte)
	{
		preSize = (sizeInByte > MAX_PREREAD_COUNT)
				? MAX_PREREAD_COUNT
				: sizeInByte;
						    	
        //pre-read lo address 
        data = srcAddr & 0x0000FFFF;
        result = _xCpuIO_SPIWriteRegister(PREREAD_ADDRESS_LO, data);
        if (result != MMP_RESULT_SUCCESS)
        {
            //printf("%s (SPI Write) Error. Error=0x%08x\n", __FUNCTION__, result);
            return result;
        } 
    
        //pre-read hi address 
        data = (srcAddr & 0xFFFF0000) >> 16;
        result = _xCpuIO_SPIWriteRegister(PREREAD_ADDRESS_HI, data);
        if (result != MMP_RESULT_SUCCESS)
        {
            //printf("%s (SPI Write) Error. Error=0x%08x\n", __FUNCTION__, result);
            return result;
        } 
            
        //pre-read length					            
    	data = (preSize / 2) - 1;
    	result = _xCpuIO_SPIWriteRegister(PREREAD_LENGTH, data);
        if (result != MMP_RESULT_SUCCESS)
        {
            //printf("%s (SPI Write) Error. Error=0x%08x\n", __FUNCTION__, result);
            return result;
        }     
        
        //pre-read fire
        data = 0x8007;
    	result = _xCpuIO_SPIWriteRegister(PREREAD_FIRE, data);
        if (result != MMP_RESULT_SUCCESS)
        {
            //printf("%s (SPI Write) Error. Error=0x%08x\n", __FUNCTION__, result);
            return result;
        } 
            	
    	ite_addr = ((0x0 >> 1) | MEMORY_ADDR_MASK);    	
    	txBuf[0] = 0x1;
        txBuf[1] = (MMP_UINT8)((ite_addr & 0x00FF));
        txBuf[2] = (MMP_UINT8)((ite_addr & 0xFF00) >> 8);    
    	result = busSPIRead(txBuf, 3, pdest, preSize);
        if (result != MMP_RESULT_SUCCESS)
        {
            //printf("%s (SPI Read) Error. Error=0x%08x\n", __FUNCTION__, result);
            return result;
        } 
            	
		sizeInByte -= preSize;
		srcAddr += preSize;
		pdest += preSize;   
	}  
    return result;
}

static MMP_RESULT
_xCpuIO_SPIWriteMemory(
    MMP_UINT32  startAddr,
    MMP_UINT8*  pdata,
    MMP_UINT32  size)
{

    MMP_RESULT  result = MMP_RESULT_SUCCESS;
    MMP_UINT8   txBuf[3];
    MMP_UINT32  ite_addr = 0, remainlen, writelen;
    MMP_UINT16  membase_hi = 0, membase_lo = 0;
    MMP_UINT8*  ptrdata;
    
    membase_hi = (MMP_UINT16)((startAddr & 0xFFFF0000) >> 16);    
    membase_lo = (MMP_UINT16)(startAddr & 0x0000FFFF);    
    
    writelen  = (size > (0x10000 - (MMP_UINT32)membase_lo)) ? (0x10000 - (MMP_UINT32)membase_lo) : size;
    remainlen = size;
    ptrdata = (MMP_UINT8 *)pdata;

    while(remainlen > 0)
    {
        result = _xCpuIO_SPIWriteRegister(MEM_ADDRESS_HI, membase_hi);
        if (result != MMP_RESULT_SUCCESS)
        {
            //printf("%s (SPI Write) Error. Error=0x%08x\n", __FUNCTION__, result);
            return result;
        } 
                            
        ite_addr = ((membase_lo >> 1) | MEMORY_ADDR_MASK);
        txBuf[0] = 0x0;
        txBuf[1] = (MMP_UINT8)((ite_addr & 0x00FF));
        txBuf[2] = (MMP_UINT8)((ite_addr & 0xFF00) >> 8);
    	result = busSPIWrite(txBuf, 3, ptrdata, writelen);       
        if (result != MMP_RESULT_SUCCESS)
        {
            //printf("%s (SPI Write) Error. Error=0x%08x\n", __FUNCTION__, result);
            return result;
        } 
            
        membase_hi++;
        membase_lo = 0;
        ptrdata += writelen;
        remainlen -= writelen;
        writelen = ((remainlen > 0x10000) ? 0x10000 : remainlen);
    }

    return result;
}

#endif

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Read slave device register.
 *
 * @param addr  The register address
 * @return the 16-bits data.
 */
//=============================================================================
MMP_UINT16
xCpuIO_ReadRegister(
    MMP_UINT16 addr)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT    retryTime = 0;    
    MMP_UINT16  data = 0;
    
    if (addr & 0x1)
    {
        //printf("xCpuIO_ReadRegister addr(0x%x) error!\n", addr);
        return 0;        
    }   
    
#if (ITX_BUS_TYPE == ITX_BUS_SPI)  
    busSPIEnter();    
    while (retryTime++ < MAX_RETRY_TIME)
    {
        result = _xCpuIO_SPIReadRegister(addr, (MMP_UINT8*)&data);
        if (result == MMP_SUCCESS)
            break;
    }    
    busSPILeave();    
    
#elif (ITX_BUS_TYPE == ITX_BUS_I2C)

    busI2CEnter();    
    while (retryTime++ < MAX_RETRY_TIME)
    {
        result = _xCpuIO_I2CReadRegister(gITXs_IIC_ID, addr, (MMP_UINT8*)&data);   
        if (result == MMP_SUCCESS)
            break;
    }
    busI2CLeave();   
     
#endif

    return data;
}

//=============================================================================
/**
 * Write slave device register.
 *
 * @param addr  The register address
 * @param data  The data to be written to the register
 */
//=============================================================================
void
xCpuIO_WriteRegister(
    MMP_UINT16 addr,
    MMP_UINT16 data)
{
    MMP_RESULT  result = MMP_SUCCESS;
    MMP_UINT    retryTime = 0;

    if (addr & 0x1)
    {
        //printf("xCpuIO_ReadRegister addr(0x%x) error!\n", addr);   
        return;   
    }

#if (ITX_BUS_TYPE == ITX_BUS_SPI)

    busSPIEnter();
    while (retryTime++ < MAX_RETRY_TIME)
    {
        result = _xCpuIO_SPIWriteRegister(addr, data);
        if (result == MMP_SUCCESS)
            break;
    }
    busSPILeave();
    
#elif (ITX_BUS_TYPE == ITX_BUS_I2C)

    busI2CEnter();
    while (retryTime++ < MAX_RETRY_TIME)
    {
        result = _xCpuIO_I2CWriteRegister(gITXs_IIC_ID, addr, data);
        if (result == MMP_SUCCESS)
            break;
    }
    busI2CLeave();   
    
#endif
}

void
xCpuIO_ReadMemory(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
    MMP_RESULT  result = MMP_SUCCESS;
    
    if (sizeInByte & 0x1)
    {
        //printf("xCpuIO_ReadMemory size(0x%x) error!\n", sizeInByte);
        return;        
    }
    
#if (ITX_BUS_TYPE == ITX_BUS_SPI)

    busSPIEnter();
    result = _xCpuIO_SPIReadMemory(srcAddress, (MMP_UINT8*)destAddress, sizeInByte);               
    busSPILeave();

#elif (ITX_BUS_TYPE == ITX_BUS_I2C)

    busI2CEnter();
    result = _xCpuIO_I2CReadMemory(gITXs_IIC_ID, srcAddress, (MMP_UINT8*)destAddress, sizeInByte);               
    busI2CLeave();
    
#endif
}

void
xCpuIO_WriteMemory(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{    
    MMP_RESULT  result = MMP_SUCCESS;
    
#if (ITX_BUS_TYPE == ITX_BUS_SPI)

    busSPIEnter();    
    result = _xCpuIO_SPIWriteMemory(destAddress, (MMP_UINT8*)srcAddress, sizeInByte);
    busSPILeave();
    
#elif (ITX_BUS_TYPE == ITX_BUS_I2C)

    busI2CEnter();
    result = _xCpuIO_I2CWriteMemory(gITXs_IIC_ID, destAddress, (MMP_UINT8*)srcAddress, sizeInByte);
    busI2CLeave();
        
#endif
}

void
xCpuIO_ReadMemoryUInt16(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
    xCpuIO_ReadMemory(destAddress, srcAddress, sizeInByte);

#if (ENDIAN == ENDIAN_AUTO)
    if (!_IsBigEndian())
#endif
#if (ENDIAN == ENDIAN_AUTO || ENDIAN == ENDIAN_LITTLE)
    {
        MMP_UINT    i;
        MMP_UINT16* ptr;

        ptr = (MMP_UINT16*)destAddress;
        for (i = 0; i < sizeInByte / sizeof(MMP_UINT16); ++i)
        {
            *(ptr + i) = _UINT16BE_To_UINT16LE(*(ptr + i));
        }
    }
#endif
}

void
xCpuIO_WriteMemoryUInt16(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
#if (ENDIAN == ENDIAN_AUTO || ENDIAN == ENDIAN_LITTLE)
    MMP_UINT    i;
    MMP_UINT16* ptr;
#endif
#if (ENDIAN == ENDIAN_AUTO)
    MMP_BOOL    isLittleEndian = !_IsBigEndian();

    if (isLittleEndian)
#endif
#if (ENDIAN == ENDIAN_AUTO || ENDIAN == ENDIAN_LITTLE)
    {
        ptr = (MMP_UINT16*)srcAddress;
        for (i = 0; i < sizeInByte / sizeof(MMP_UINT16); ++i)
        {
            *(ptr + i) = _UINT16LE_To_UINT16BE(*(ptr + i));
        }
    }
#endif

    xCpuIO_WriteMemory(destAddress, srcAddress, sizeInByte);

#if (ENDIAN == ENDIAN_AUTO)
    if (isLittleEndian)
#endif
#if (ENDIAN == ENDIAN_AUTO || ENDIAN == ENDIAN_LITTLE)
    {
        ptr = (MMP_UINT16*)srcAddress;
        for (i = 0; i < sizeInByte / sizeof(MMP_UINT16); ++i)
        {
            *(ptr + i) = _UINT16BE_To_UINT16LE(*(ptr + i));
        }
    }
#endif
}

void
xCpuIO_ReadMemoryUInt32(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
    xCpuIO_ReadMemory(destAddress, srcAddress, sizeInByte);

#if (ENDIAN == ENDIAN_AUTO)
    if (!_IsBigEndian())
#endif
#if (ENDIAN == ENDIAN_AUTO || ENDIAN == ENDIAN_LITTLE)
    {
        MMP_UINT    i;
        MMP_UINT32* ptr;

        ptr = (MMP_UINT32*)destAddress;
        for (i = 0; i < sizeInByte / sizeof(MMP_UINT32); ++i)
        {
            *(ptr + i) = _UINT32BE_To_UINT32LE(*(ptr + i));
        }
    }
#endif
}

void
xCpuIO_WriteMemoryUInt32(
    MMP_UINT32 destAddress,
    MMP_UINT32 srcAddress,
    MMP_UINT32 sizeInByte)
{
#if (ENDIAN == ENDIAN_AUTO || ENDIAN == ENDIAN_LITTLE)
    MMP_UINT    i;
    MMP_UINT32* ptr;
#endif
#if (ENDIAN == ENDIAN_AUTO)
    MMP_BOOL    isLittleEndian = !_IsBigEndian();

    if (isLittleEndian)
#endif
#if (ENDIAN == ENDIAN_AUTO || ENDIAN == ENDIAN_LITTLE)
    {
        ptr = (MMP_UINT32*)srcAddress;
        for (i = 0; i < sizeInByte / sizeof(MMP_UINT32); ++i)
        {
            *(ptr + i) = _UINT32LE_To_UINT32BE(*(ptr + i));
        }
    }
#endif

    xCpuIO_WriteMemory(
        (MMP_UINT32)destAddress,
        (MMP_UINT32)srcAddress,
        sizeInByte);

#if (ENDIAN == ENDIAN_AUTO)
    if (isLittleEndian)
#endif
#if (ENDIAN == ENDIAN_AUTO || ENDIAN == ENDIAN_LITTLE)
    {
        ptr = (MMP_UINT32*)srcAddress;
        for (i = 0; i < sizeInByte / sizeof(MMP_UINT32); ++i)
        {
            *(ptr + i) = _UINT32BE_To_UINT32LE(*(ptr + i));
        }
    }
#endif
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

#if (ENDIAN == ENDIAN_AUTO)
MMP_BOOL
_IsBigEndian(
    void)
{
    union
    {
        MMP_UINT32 k;
        MMP_UINT8  c[4];
    } u;

    u.k = 0xFF000000;

    if (0xFF == u.c[0])
    {
        return MMP_TRUE;
    }
    return MMP_FALSE;
}
#endif
