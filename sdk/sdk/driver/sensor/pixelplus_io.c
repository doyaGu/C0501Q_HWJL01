
#include "iic/mmp_iic.h"
#include "iic/iic.h"
#include "encoder/encoder_types.h"
#include "sensor/pixelplus_io.h"

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
static uint8_t   PIXELPLUS_IICADDR = 0x64 >> 1;

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
uint8_t
PixelPlus_ReadI2C_Byte(
    uint16_t RegAddr)
{
    int result;
    uint8_t  dbuf[256];
    uint8_t*  pdbuf = dbuf;
    uint8_t value;

    *pdbuf++ = (uint8_t)(RegAddr&0x00ff);

    mmpIicLockModule(IIC_PORT_0);

    result = IIC_SendData(PIXELPLUS_IICADDR, dbuf, 1, W_STOP);
    if(result == 0)
    {
        result = IIC_ReceiveData(PIXELPLUS_IICADDR, pdbuf, 1);
        if(result != 0)
            printf("PIXELPLUS_ReadI2C_8Bit receive data at address 0x%02x error!\n", RegAddr);
    }
    else
        printf("PIXELPLUS_ReadI2C_8Bit send address 0x%02x error!\n", RegAddr);

    mmpIicReleaseModule(IIC_PORT_0);

    value = (dbuf[1] & 0xFF);

    return value;

}

int
PixelPlus_WriteI2C_Byte(
    uint16_t RegAddr,
    uint8_t data)
{
    int result;
    uint8_t  dbuf[256];
    uint8_t*  pdbuf = dbuf;

    *pdbuf++ = (uint8_t)(RegAddr&0x00ff);
    *pdbuf = (uint8_t)(data);

    mmpIicLockModule(IIC_PORT_0);
    if(0 != (result = IIC_SendData(PIXELPLUS_IICADDR, dbuf, 2, W_STOP)))
    {
        printf("PIXELPLUS_WriteI2C_8Bit I2c Write Error, reg=%02x val=%02x\n", RegAddr, data);
        mmpIicGenStop(IIC_PORT_0);
    }

    mmpIicReleaseModule(IIC_PORT_0);
    return result;

}


