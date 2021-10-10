#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "iic/mmp_iic.h"
#include "ite/ith.h"


//=============================================================================
//                Constant Definition
//============================================================================= 

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct _REGPAIR
{
    uint16_t  addr;
    uint16_t value;
} REGPAIR;


//=============================================================================
//                Global Data Definition
//=============================================================================
static uint8_t  TC358778_IICADDR   = 0x0E; 

static REGPAIR  INPUT_REG[]      =
{
    ///TC358778 initial    
    {0x0002,0x0001},
    {0x0002,0x0000},
    {0x0016,0x1068},
    {0x0018,0x0603},
    {0x0018,0x0613},
    {0x0006,0x03FA},
    {0x0140,0x0000},
    {0x0142,0x0000},
    {0x0144,0x0000},
    {0x0146,0x0000},
    {0x0148,0x0000},
    {0x014A,0x0000},
    {0x014C,0x0000},
    {0x014E,0x0000},
    {0x0150,0x0000},
    {0x0152,0x0000},
    {0x0100,0x0002},
    {0x0102,0x0000},
    {0x0104,0x0002},
    {0x0106,0x0000},
    {0x0108,0x0002},
    {0x010A,0x0000},
    {0x010C,0x0002},
    {0x010E,0x0000},
    {0x0110,0x0002},
    {0x0112,0x0000},
    {0x0210,0x1644},
    {0x0212,0x0000},
    {0x0214,0x0002},
    {0x0216,0x0000},
    {0x0218,0x2002},
    {0x021A,0x0000},
    {0x0220,0x0602},
    {0x0222,0x0000},
    {0x0224,0x4268},
    {0x0226,0x0000},
    {0x022C,0x0000},
    {0x022E,0x0000},
    {0x0230,0x0005},
    {0x0232,0x0000},
    {0x0234,0x001F},
    {0x0236,0x0000},
    {0x0238,0x0001},
    {0x023A,0x0000},
    {0x023C,0x0001},
    {0x023E,0x0002},
    {0x0204,0x0001},
    {0x0206,0x0000},
    {0x0620,0x0001},
    {0x0622,0x0018},
    {0x0624,0x0017},
    {0x0626,0x0258},
    {0x0628,0x020D},
    {0x062A,0x01ED},
    {0x062C,0x0C00},
    {0x0518,0x0001},
    {0x051A,0x0000},
    {0x0602,0x1005},
    {0x0604,0x0000},
    {0x0610,0x0001},
    {0x0600,0x0001},
    {0x0500,0x0086},
    {0x0502,0xA300},
    {0x0500,0x8000},
    {0x0502,0xC300},
    {0x0008,0x0037},
    {0x0050,0x003E},
    {0x0032,0x0000},
    {0x0004,0x0240},
};

static uint8_t DEALY_REG[]      =
{
    0x10,
    0x00,
    0x00,
    0x10,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x10,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x10,
};


//=============================================================================
//                Private Function Definition
//=============================================================================
static uint16_t _TC358778_ReadI2c_Byte(uint16_t RegAddr)
{
    uint16_t flag;
    uint8_t  dbuf[256];
    uint8_t*  pdbuf = dbuf;
    uint16_t value;

	*pdbuf++ = (uint8_t)((RegAddr&0xff00)>>8);
    *pdbuf++ = (uint8_t)(RegAddr&0x00ff);
          
    if (0 != (flag = mmpIicReceiveDataEx(IIC_PORT_0, IIC_MASTER_MODE, TC358778_IICADDR, dbuf, 2, pdbuf, 2)))
    {
        printf("TC358778_IICADDR I2C Read error, reg = %02x\n", RegAddr);
        mmpIicGenStop(IIC_PORT_0);
    }

	value =(uint16_t)(dbuf[2] << 8 | dbuf[3]);    
    return value;
}

static uint16_t _TC358778_WriteI2c_Byte(uint16_t RegAddr, uint16_t data)
{
    uint16_t flag;
    uint8_t   dbuf[256];
    uint8_t*  pdbuf = dbuf;

	*pdbuf++ = (uint8_t)((RegAddr&0xff00)>>8);
    *pdbuf++ = (uint8_t)(RegAddr&0x00ff);
    *pdbuf++ = (uint8_t)((data&0xff00)>>8);
	*pdbuf = (uint8_t)(data&0x00ff);

    if (0 != (flag = mmpIicSendDataEx(IIC_PORT_0, IIC_MASTER_MODE, TC358778_IICADDR, dbuf, 4)))
    {
        printf("TC358778_IICADDR I2c write error, reg = %02x val =%02x\n", RegAddr, data);
        mmpIicGenStop(IIC_PORT_0);
    }
    return flag;
}

static void _TC358778SetRegister(void)
{
    uint16_t i;

    for (i = 0; i < (sizeof(INPUT_REG) / sizeof(REGPAIR)); i++)
    {
        _TC358778_WriteI2c_Byte(INPUT_REG[i].addr, INPUT_REG[i].value);
        if(DEALY_REG[i])
        {
            //printf("i=%d\n",i);
            usleep(1000);
        }
    }
}

//=============================================================================
//                Public Function Definition
//=============================================================================
void TC358778Initialize(void)
{
    uint16_t data;
    printf("TC358778 Initial\n");
	
    usleep(1000* 10);	
    _TC358778SetRegister();
}

void itpRGBToMIPIInit(void)
{
	TC358778Initialize();
}

