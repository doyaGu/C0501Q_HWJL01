#include "iic/mmp_iic.h"
#include "ite/ith.h"
#include "iic/iic.h"

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
static unsigned char   FM2018_IICADDR = 0xC0 >> 1;

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
unsigned short 
FM2018_MemReadI2C_16Bit(
    unsigned short   RegAddr)
{
    uint32_t result = 0;
    unsigned char  dbuf[256];
    unsigned char*  pdbuf = dbuf;
    unsigned long  value;

    *pdbuf++ = (unsigned char)0xFC;
    *pdbuf++ = (unsigned char)0xF3;
    *pdbuf++ = (unsigned char)0x37;
    *pdbuf++ = (unsigned char)((RegAddr&0xff00)>>8);
    *pdbuf++ = (unsigned char)(RegAddr&0x00ff);
        
    mmpIicLockModule(IIC_PORT_0);    
    
    result = IIC_SendData(FM2018_IICADDR, dbuf, 5, W_STOP); 
    if(result == 0)
        result = IIC_ReceiveData(FM2018_IICADDR, pdbuf, 2);

    value = ((dbuf[5] & 0xFF) << 8) | (dbuf[6] & 0xFF);  
        
    mmpIicReleaseModule(IIC_PORT_0);
    
    return value;
}

uint32_t 
FM2018_MemWriteI2C_16Bit(
    unsigned short  RegAddr,
    unsigned short  data)
{

    uint32_t result;
    unsigned char  dbuf[256];
    unsigned char*  pdbuf = dbuf;

    *pdbuf++ = (unsigned char)0xFC;
    *pdbuf++ = (unsigned char)0xF3;
    *pdbuf++ = (unsigned char)0x3B;
    *pdbuf++ = (unsigned char)((RegAddr&0xff00)>>8);
    *pdbuf++ = (unsigned char)(RegAddr&0x00ff);
    *pdbuf++ = (unsigned char)((data&0xff00)>>8);
    *pdbuf++ = (unsigned char)(data&0x00ff);

    mmpIicLockModule(IIC_PORT_0);        
    if(0 != (result = IIC_SendData(FM2018_IICADDR, dbuf, 7, W_STOP)))
    {
        printf("FM2018_MemWriteI2C_16Bit I2c Write Error, reg=%04x val=%04x\n", RegAddr, data);
        mmpIicGenStop(IIC_PORT_0);
    }       
    mmpIicReleaseModule(IIC_PORT_0);    
    
    return result;
}

unsigned short  
FM2018_RegReadI2C_16Bit(
    unsigned short  RegAddr)
{
    uint32_t result;
    unsigned char  dbuf[256];
    unsigned char*  pdbuf = dbuf;
    unsigned long value;

    *pdbuf++ = (unsigned char)0xFC;
    *pdbuf++ = (unsigned char)0xF3;
    *pdbuf++ = (unsigned char)0x60;
    *pdbuf++ = (unsigned char)(RegAddr&0x00ff);
        
    mmpIicLockModule(IIC_PORT_0);    
    
    result = IIC_SendData(FM2018_IICADDR, dbuf, 4, W_STOP); 
    if(result == 0)
        result = IIC_ReceiveData(FM2018_IICADDR, pdbuf, 2);

    value = ((dbuf[4] & 0xFF) << 8) | (dbuf[5] & 0xFF);  
        
    mmpIicReleaseModule(IIC_PORT_0);
    
    return value;
}

uint32_t 
FM2018_RegWriteI2C_16Bit(
    unsigned short  RegAddr,
    unsigned short  data)
{

    uint32_t result;
    unsigned char  dbuf[256];
    unsigned char*  pdbuf = dbuf;
    
    *pdbuf++ = (unsigned char)0xFC;
    *pdbuf++ = (unsigned char)0xF3;
    *pdbuf++ = (unsigned char)0x6A;
    *pdbuf++ = (unsigned char)(RegAddr&0x00ff);
    *pdbuf++ = (unsigned char)((data&0xff00)>>8);
    *pdbuf++ = (unsigned char)(data&0x00ff);

    mmpIicLockModule(IIC_PORT_0);        
    if(0 != (result = IIC_SendData(FM2018_IICADDR, dbuf, 6, W_STOP)))
    {
        printf("FM2018_RegWriteI2C_16Bit I2c Write Error, reg=%04x val=%04x\n", RegAddr, data);
        mmpIicGenStop(IIC_PORT_0);
    }       
    mmpIicReleaseModule(IIC_PORT_0);    
    
    return result;
}