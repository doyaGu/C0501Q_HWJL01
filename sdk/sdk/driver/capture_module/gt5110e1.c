#include <unistd.h>
#include "iic/mmp_iic.h"
#include "capture_module/gt5110e1.h"
#include "capture_module.h" 

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
    uint8_t  addr;
    uint16_t value;
} REGPAIR;

typedef struct GT5110E1CaptureModuleDriverStruct
{
    CaptureModuleDriverStruct base;
} GT5110E1CaptureModuleDriverStruct;

//=============================================================================
//                Global Data Definition
//=============================================================================
static uint8_t  GT5110E1_IICADDR   = 0x62 >> 1; 
static MMP_BOOL gGT5110E1InitDone  = MMP_FALSE;
static char EEPROM[8] = {0};


/* 32-Lead LFCSP , page 108*/
static REGPAIR  INPUT_REG[]      =
{
#if 0
	//gt5110e1 initial setting
	{0xfc , 0x16},
	{0xfe , 0x80},
	{0xfe , 0x00},
	{0xfc , 0x16},
	{0xf1 , 0x01},
	{0xf0 , 0x07},
	{0xfa , 0x00},
	{0x24 , 0x3f},
	{0x46 , 0x02},
	{0x09 , 0x00},
	{0x0a , 0x04},
	{0x0b , 0x00},
	{0x0c , 0x00},
	{0x0d , 0x01},
	{0x0e , 0xe8},
	{0x0f , 0x02},
	{0x10 , 0x88},
	{0x17 , 0x14},
	{0x19 , 0x05},
	{0x1F , 0xC0},
	{0x1E , 0x15},
	{0x20 , 0x00},
	{0x21 , 0x48},
	{0x22 , 0xDA},
	{0x23 , 0x41},
	{0x24 , 0x16},
	{0x33 , 0x20},
	{0x34 , 0x20},
	{0x35 , 0xFF},
	{0x36 , 0xFF},
	{0x41 , 0x00},
	{0x42 , 0xFE},
	{0x4F , 0x00},
	{0x70 , 0x40},
	{0x76 , 0x8A},
	{0xB0 , 0x00},
	{0xBC , 0x00},
	{0xBD , 0x00},
	{0xBE , 0x00},
	{0x4D , 0x03},
	{0xfe , 0x00},
	{0x4b , 0xCA},
	{0x50 , 0x01},
	{0x51 , 0x00},
	{0x52 , 0x00},
	{0x53 , 0x00},
	{0x54 , 0x00},
	{0x55 , 0x00},
	{0x56 , 0xf0},
	{0x57 , 0x01},
	{0x58 , 0x40},
	{0x59 , 0x22},
	{0x5a , 0x03},
	{0x5b , 0x00},
	{0x5c , 0x00},
	{0x5d , 0x00},
	{0x5e , 0x00},
	{0x5f , 0x00},
	{0x60 , 0x00},
	{0x61 , 0x00},
	{0x62 , 0x00},
	{0x03 , 0x01}, //EEPROM[7] =0x1
	{0x04 , 0xA0}, //EEPROM[8] = 0x1
	{0x44 , 0x02},
	{0x11 , 0x2A},
	{0x40 , 0xBf},
	{0x41 , 0x02},
    {0x70 , 0x80}, //{0x70 , 0x75},
	{0x7A , 0xFF},
	{0x7B , 0x70},
	{0x7C , 0xFF},
	{0xD3 , 0xFF}, //{0xD3 , 0x80},  
    {0x63 , 0x25}, //{0x63 , 0x00},  
    {0x64 , 0x3E}, //{0x64 , 0x0B},  
    {0x65 , 0x54}, //{0x65 , 0x19},  
	{0x66 , 0x69}, //{0x66 , 0x2B},  
    {0x67 , 0x7C}, //{0x67 , 0x37},  
	{0x68 , 0x8E}, //{0x68 , 0x48},  
	{0x69 , 0xA0}, //{0x69 , 0x51},  
	{0x6A , 0xB1}, //{0x6A , 0x5E},  
    {0x6B , 0xC1}, //{0x6B , 0x97},  
	{0x6C , 0xD1}, //{0x6C , 0xB2},  
	{0x6D , 0xE0}, //{0x6D , 0xC9},  
	{0x6E , 0xF0}, //{0x6E , 0xDF},  
	{0x6F , 0xFF},
#else
 {0xfc , 0x16},
 // Soft reset
  {0xfe , 0x80},
  {0xfe , 0x00},
  {0xfc , 0x16},
  {0xf1 , 0x01},//0x11},
                            
  // ?e¢DX-P¡Âa¡Ó¡Ó¡Li
  {0xf0 , 0x07},
  // 1/4 MCLK
  {0xfa , 0x00},
  
  // ?e¢DXPINAX¢XE¡Âa?O
  {0x24 , 0x3f},
  // |P¡LB?H¡M1¡P¢DcE¡Ó¡Ó¡Li
  {0x46 , 0x02},

  
  // Windowsing?O|! {640X480}
	
	
	
	{0x05 , 0x00},
  {0x06 , 0x6A},
  {0x07 , 0x00},
  {0x08 , 0x0C},
	
	
	
  {0x09 , 0x00},
  {0x0a , 0x00},
  {0x0b , 0x00},
  {0x0c , 0x00},
  {0x0d , 0x01},
  {0x0e , 0xe8},
  {0x0f , 0x02},
  {0x10 , 0x88},
	
	
	
	
/*-----------------------------------------------*/
  {0x17 , 0x14},
  {0x19 , 0x05},
  {0x1F , 0xC0},
  {0x1E , 0x15},
  {0x20 , 0x00},
  {0x21 , 0x48},
  {0x22 , 0xDA},
  {0x23 , 0x41},
  {0x24 , 0x16},
  {0x33 , 0x20},
  {0x34 , 0x20},
  {0x35 , 0xFF},
  {0x36 , 0xFF},
  {0x41 , 0x00},
  {0x42 , 0xFE},
  {0x4F , 0x00},//0x00},
  {0x70 , 0x40},
  {0x76 , 0x8A},
  {0xB0 , 0x00},
  {0xBC , 0x00},
  {0xBD , 0x00},
  {0xBE , 0x00},
  {0x4D , 0x03},

	{0xfe , 0x01},  // page 1
	{0x10 , 0x00},
//	{0x13 , 0x7F},//0x79}, AE target value
	{0x13 , 0x70},//0x79}, AE target value
	
	{0x2b , 0x01},
	{0x2c , 0xf4},
	{0x2d , 0x03},
	{0x2e , 0xe8},
	{0x2f , 0x05},
	{0x30 , 0xd0},
	{0x31 , 0x0b},
	{0x32 , 0xb8},
	
	{0x33 , 0x20},
	{0x34 , 0x04},
	
	
  {0xfe , 0x00},  // page 0
  

  
  // Crop out Window mode
  {0x50 , 0x01},
  
  // Subsample output
  {0x51 , 0x00},
  {0x52 , 0x00},
  {0x53 , 0x00},
  {0x54 , 0x00},
  {0x55 , 0x00},
  {0x56 , 0xf0},
  {0x57 , 0x01},
  {0x58 , 0x40},
  
  {0x59 , 0x22},
  {0x5a , 0x03},
  {0x5b , 0x00},
  {0x5c , 0x00},
  {0x5d , 0x00},
  {0x5e , 0x00},
  {0x5f , 0x11},
  {0x60 , 0x11},
  {0x61 , 0x11},
  {0x62 , 0x11},	  
  
  // ext_time
  {0x03 , 0x03},
  {0x04 , 0xAC},
   
  // Output format {YCbYCr}
  {0x44 , 0x02},
  
  // sh_delay
  {0x11 , 0x2A},
  
  // ISP Related setting
  {0x40 , 0x6F},


  {0x41 , 0x40},
  {0x4F , 0x01},
	{0x42 , 0x00},
  {0x70 , 0x3F},
//  {0xD3 , 0x80},
  {0xD3 , 0x80},
	{0x7C , 0xFF},
	{0x81 , 0x30},
	{0x82 , 0xFF},
	{0x83 , 0x00},
#endif
};

static CAP_TIMINFO_TABLE GT5110E1_TABLE [] = {
    //Index, HActive, VActive,  Rate,            FrameRate,                 Hpor,   Vpor,  HStar,     HEnd,   VStar1,   VEnd1,  VStar2,   VEnd2,
    {0,     320,    240,     3000,   CAP_FRAMERATE_30HZ,       0,      0,      0,        0,        0,      0,   0,       0   }, //240i30     // Benson
};      

//=============================================================================
//                Private Function Definition
//=============================================================================
static uint8_t _GT5110E1_ReadI2c_Byte(uint8_t RegAddr)
{
    uint8_t    data;
    MMP_RESULT flag;

    //mmpIicLockModule(IIC_PORT_0);
    if (0 != (flag = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, GT5110E1_IICADDR, &RegAddr, 1, &data, sizeof(data))))
    {
        printf("GT5110E1 I2C Read error, reg = %02x\n", RegAddr);
        mmpIicGenStop(IIC_PORT_0);
    }
    //mmpIicReleaseModule(IIC_PORT_0);
    return data;
}

static MMP_RESULT _GT5110E1_WriteI2c_Byte(uint8_t RegAddr, uint8_t data)
{
    MMP_RESULT flag;

    //mmpIicLockModule(IIC_PORT_0);
    if (0 != (flag = mmpIicSendData(IIC_PORT_0, IIC_MASTER_MODE, GT5110E1_IICADDR, RegAddr, &data, sizeof(data))))
    {
        printf("Gt5110e1 I2c write error, reg = %02x val =%02x\n", RegAddr, data);
        mmpIicGenStop(IIC_PORT_0);
    }
    //mmpIicReleaseModule(IIC_PORT_0);
    return flag;
}

static MMP_RESULT _GT5110E1_WriteI2c_ByteMask(uint8_t RegAddr, uint8_t data, uint8_t mask)
{
    uint8_t    value;
    MMP_RESULT flag;

    //mmpIicLockModule(IIC_PORT_0);
    if (0 != (flag = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, GT5110E1_IICADDR, &RegAddr, 1, &value, 1)))
    {
        printf("GT5110E1 I2C Read error, reg = %02x\n", RegAddr);
        mmpIicGenStop(IIC_PORT_0);
    }

    value = ((value & ~mask) | (data & mask));

    if (0 != (flag = mmpIicSendData(IIC_PORT_0, IIC_MASTER_MODE, GT5110E1_IICADDR, RegAddr, &value, 1)))
    {
        printf("GT5110E1 I2c write error, reg = %02x val =%02x\n", RegAddr, value);
		mmpIicGenStop(IIC_PORT_0);
    }
    //mmpIicReleaseModule(IIC_PORT_0);
    return flag;
}


static void _Set_GT5110E1_Input_Setting(void)
{
    uint16_t i;

    for (i = 0; i < (sizeof(INPUT_REG) / sizeof(REGPAIR)); i++)
    {
        _GT5110E1_WriteI2c_Byte(INPUT_REG[i].addr, INPUT_REG[i].value);
    }
}

//=============================================================================
//                Public Function Definition
//=============================================================================

void GT5110E1Initialize(void)
{
	MMP_BOOL    bSensorID;	
    uint16_t data;
    printf("gt5110e1 Initial\n");

    gGT5110E1InitDone = MMP_FALSE;
	
    usleep(1000* 10);
    _Set_GT5110E1_Input_Setting();

    gGT5110E1InitDone = MMP_TRUE;
}

void GT5110E1Terminate(void)
{
    printf("GT5110E1 Terminate\n");
}

void GT5110E1OutputPinTriState(unsigned char flag)
{
}

unsigned char GT5110E1IsSignalStable(void)
{
	return true;
}

void GT5110E1GetProperty(CAP_GET_PROPERTY * pGetProperty)
{
    uint16_t i;
    pGetProperty->GetTopFieldPolarity = MMP_FALSE;
    pGetProperty->GetHeight = 240;
    pGetProperty->GetWidth  = 320;
    pGetProperty->Rate = 3000;
    pGetProperty->GetModuleIsInterlace = 0;

    for (i = 0; i < (sizeof(GT5110E1_TABLE) / sizeof(CAP_TIMINFO_TABLE)); ++i)
    {
       if ((pGetProperty->GetWidth == GT5110E1_TABLE[i].HActive) && (pGetProperty->GetHeight == GT5110E1_TABLE[i].VActive) && pGetProperty->Rate == GT5110E1_TABLE[i].Rate)
        {
            pGetProperty->HPolarity       = GT5110E1_TABLE[i].HPolarity; 
            pGetProperty->VPolarity       = GT5110E1_TABLE[i].VPolarity;
            pGetProperty->FrameRate       = GT5110E1_TABLE[i].FrameRate; 
            pGetProperty->matchResolution = MMP_TRUE;

            pGetProperty->HStar  = GT5110E1_TABLE[i].HStar;
            pGetProperty->HEnd   = GT5110E1_TABLE[i].HEnd;
            pGetProperty->VStar1 = GT5110E1_TABLE[i].VStar1;
            pGetProperty->VEnd1  = GT5110E1_TABLE[i].VEnd1;
            pGetProperty->VStar2 = GT5110E1_TABLE[i].VStar2; 
            pGetProperty->VEnd2  = GT5110E1_TABLE[i].VEnd2; 
        }
    }
}

void GT5110E1PowerDown(unsigned char enable)
{
}

void GT5110E1ForCaptureDriverSetting(CAP_CONTEXT *Capctxt )
{
    /* Input Format default Setting */
    Capctxt->Interleave   = Progressive;

    /* Input Data Format Setting */
    // 8bit bus
    Capctxt->YUV422Format  = CAP_IN_YUV422_YUYV;
    Capctxt->EnDEMode             = MMP_TRUE;
    Capctxt->input_protocol       = BT_601;
    
    Capctxt->input_video_source.HSYNCDE =   MMP_TRUE;
}

static void GT5110E1CaptureModuleDriver_Destory(CaptureModuleDriver base)
{
    GT5110E1CaptureModuleDriver self = (GT5110E1CaptureModuleDriver)base;
    if(self)
        free(self);
}

static CaptureModuleDriverInterfaceStruct interface =
{
    GT5110E1Initialize,
    GT5110E1Terminate,
    GT5110E1OutputPinTriState,
    GT5110E1IsSignalStable,
    GT5110E1GetProperty,
    GT5110E1PowerDown,
    GT5110E1ForCaptureDriverSetting,
    GT5110E1CaptureModuleDriver_Destory
};

CaptureModuleDriver GT5110E1CaptureModuleDriver_Create()
{
    GT5110E1CaptureModuleDriver self = calloc(1, sizeof(GT5110E1CaptureModuleDriverStruct));
    self->base.vtable = &interface;
    self->base.type = "GT5110E1";
    return (GT5110E1CaptureModuleDriver)self;
}
