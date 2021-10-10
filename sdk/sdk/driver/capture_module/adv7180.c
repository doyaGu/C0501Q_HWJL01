#include <unistd.h>
#include "iic/mmp_iic.h"
#include "capture_module/adv7180.h"
#include "capture_module.h" //Benson add

//=============================================================================
//                Constant Definition
//=============================================================================
static uint8_t ADV7180_IICADDR = 0x40 >> 1; //0x42 >> 1;

#define POWER_MANAGEMENT 0x0F
#define REG_STATUS1      0x10
#define REG_IDENT        0x11
#define REG_STATUS2      0x12
#define REG_STATUS3      0x13

#define RESET_MASK       (1 << 7)
#define TRI_STATE_ENABLE (1 << 7)

//#define AUTO_DETECT_INPUT

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

typedef enum _INPUT_TYPE
{
    _NTSM_M_J = 0,
    _NTSC_4_43,
    _PAL_M,
    _PAL_60,
    _PAL_B_G_H_I_D,
    _SECAM,
    _PAL_COMBINATION_N,
    _SECAM_525
} INPUT_TYPE;

typedef struct ADV7180CaptureModuleDriverStruct
{
    CaptureModuleDriverStruct base;
} ADV7180CaptureModuleDriverStruct;

//=============================================================================
//                Global Data Definition
//=============================================================================
static uint16_t gtADV7180CurMode  = 0xFF;
static uint16_t gtADV7180PreMode  = 0xFF;
static uint16_t gtADV7180CurDev   = 0xFF;
static MMP_BOOL gtADV7180InitDone = MMP_FALSE;

/* 32-Lead LFCSP , page 108*/
static REGPAIR  CVBS_INPUT[]      =
{
    {0x00, 0x04}, //AIN3
    {0x04, 0x54},
    {0x17, 0x41},
    {0x31, 0x02},
    {0x3D, 0xA2},
    {0x3E, 0x6A},
    {0x3F, 0xA0},
    {0x58, 0x01},
    {0x0E, 0x80},
    {0x55, 0x81},
    {0x0E, 0x00},

    //Autodetect enable PAL_B/NTSC/N443
    {0x07, 0x23},  // Benson

#if 0
    //    // Figure35 and Figure 40 for BT601 NTSC and PAL
    {0x31, 0x1A},
    {0x32, 0x81},
    {0x33, 0x84},
    {0x34, 0x00},
    {0x35, 0x00},
    {0x36, 0x7D},
    {0x37, 0xA1},
    //NTSC
    {0xE5, 0x41},
    {0xE6, 0x84},
    {0xE7, 0x06},
    //PAL
    {0xE8, 0x41},
    {0xE9, 0x84},
    {0xEA, 0x06},
#endif
};

static REGPAIR  SVIDEO_INPUT [] =
{
    {0x00, 0x06}, //AIN1 AIN2
    {0x04, 0x54},
    {0x31, 0x02},
    {0x3D, 0xA2},
    {0x3E, 0x6A},
    {0x3F, 0xA0},
    {0x58, 0x05},
    {0x0E, 0x80},
    {0x55, 0x81},
    {0x0E, 0x00},

    //Autodetect enable PAL_B/NTSC/N443
    {0x07, 0x23},

    // Figure35 and Figure 40 for BT601 NTSC and PAL
    {0x31, 0x1A},
    {0x32, 0x81},
    {0x33, 0x84},
    {0x34, 0x00},
    {0x35, 0x00},
    {0x36, 0x7D},
    {0x37, 0xA1},
    //NTSC
    {0xE5, 0x41},
    {0xE6, 0x84},
    {0xE7, 0x06},
    //PAL
    {0xE8, 0x41},
    {0xE9, 0x84},
    {0xEA, 0x06},
};

static REGPAIR  YPrPb_INPUT [] =
{
    {0x00, 0x09},
    {0x31, 0x02},
    {0x3D, 0xA2},
    {0x3E, 0x6A},
    {0x3F, 0xA0},
    {0x58, 0x01},
    {0x0E, 0x80},
    {0x55, 0x81},
    {0x0E, 0x00},

    //Autodetect enable PAL_B/NTSC/N443
    {0x07, 0x23},

    // Figure35 and Figure 40 for BT601 NTSC and PAL
    {0x31, 0x1A},
    {0x32, 0x81},
    {0x33, 0x84},
    {0x34, 0x00},
    {0x35, 0x00},
    {0x36, 0x7D},
    {0x37, 0xA1},
    //NTSC
    {0xE5, 0x41},
    {0xE6, 0x84},
    {0xE7, 0x06},
    //PAL
    {0xE8, 0x41},
    {0xE9, 0x84},
    {0xEA, 0x06},
};

//ADV7180 Table
static CAP_TIMINFO_TABLE ADV7180_TABLE [] = {
    //Index, HActive, VActive,  Rate,            FrameRate,                 Hpor,   Vpor,  HStar,     HEnd,   VStar1,   VEnd1,  VStar2,   VEnd2,
    {0,     720,    487,        2997,   CAP_FRAMERATE_29_97HZ,            0,      0, 238 + 32,   1677 + 32,     22 - 7,   261 - 7,   285 - 7,   524 - 7   }, //480i60     // Benson
    {1,     720,    576,        2500,   CAP_FRAMERATE_25HZ,               0,      0, 264 + 18,   1703 + 18,     23 - 4,   310 - 4,   336 - 4,   623 - 4   }, //576i50
};

//=============================================================================
//                Private Function Definition
//=============================================================================
static uint8_t _ADV7180_ReadI2c_Byte(uint8_t RegAddr)
{
    uint8_t    data;
    MMP_RESULT flag;

    //mmpIicLockModule(IIC_PORT_0);
    if (0 != (flag = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, ADV7180_IICADDR, &RegAddr, 1, &data, sizeof(data))))
    {
        printf("ADV7180 I2C Read error, reg = %02x\n", RegAddr);
        //mmpIicGenStop(IIC_PORT_0);
    }
    //mmpIicReleaseModule(IIC_PORT_0);
    return data;
}

static MMP_RESULT _ADV7180_WriteI2c_Byte(uint8_t RegAddr, uint8_t data)
{
    MMP_RESULT flag;

    //mmpIicLockModule(IIC_PORT_0);
    if (0 != (flag = mmpIicSendData(IIC_PORT_0, IIC_MASTER_MODE, ADV7180_IICADDR, RegAddr, &data, sizeof(data))))
    {
        printf("ADV7180 I2c write error, reg = %02x val =%02x\n", RegAddr, data);
        mmpIicGenStop(IIC_PORT_0);
    }
    //mmpIicReleaseModule(IIC_PORT_0);
    return flag;
}

static MMP_RESULT _ADV7180_WriteI2c_ByteMask(uint8_t RegAddr, uint8_t data, uint8_t mask)
{
    uint8_t    value;
    MMP_RESULT flag;

    //mmpIicLockModule(IIC_PORT_0);

    if (0 != (flag = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, ADV7180_IICADDR, &RegAddr, 1, &value, 1)))
    {
        printf("ADV7180 I2C Read error, reg = %02x\n", RegAddr);
        mmpIicGenStop(IIC_PORT_0);
    }

    value = ((value & ~mask) | (data & mask));

    if (0 != (flag = mmpIicSendData(IIC_PORT_0, IIC_MASTER_MODE, ADV7180_IICADDR, RegAddr, &value, 1)))
    {
        if(RegAddr!= 0x0F)
            printf("ADV7180 I2c write error, reg = %02x val =%02x\n", RegAddr, value);
        mmpIicGenStop(IIC_PORT_0);
    }
    //mmpIicReleaseModule(IIC_PORT_0);
    return flag;
}

static void _ADV7180_SWReset()
{
    _ADV7180_WriteI2c_ByteMask(POWER_MANAGEMENT, RESET_MASK, RESET_MASK);
}

static void _Set_ADV7180_Input_CVBS(void)
{
    uint16_t i;

    for (i = 0; i < (sizeof(CVBS_INPUT) / sizeof(REGPAIR)); i++)
    {
        _ADV7180_WriteI2c_Byte(CVBS_INPUT[i].addr, CVBS_INPUT[i].value);
    }
    _ADV7180_WriteI2c_ByteMask(0x4, 0x80, 0x80);
}

static void _Set_ADV7180_Input_SVIDEO(void)
{
    uint16_t i;

    for (i = 0; i < (sizeof(SVIDEO_INPUT) / sizeof(REGPAIR)); i++)
        _ADV7180_WriteI2c_Byte(SVIDEO_INPUT[i].addr, SVIDEO_INPUT[i].value);
}

static void _Set_ADV7180_Input_YPrPb(void)
{
    uint16_t i;

    for (i = 0; i < (sizeof(YPrPb_INPUT) / sizeof(REGPAIR)); i++)
        _ADV7180_WriteI2c_Byte(YPrPb_INPUT[i].addr, YPrPb_INPUT[i].value);
}

//=============================================================================
//                Public Function Definition
//=============================================================================
void Set_ADV7180_Tri_State_Enable()
{
    //LLC pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x1D, 0x80, 0x80);

    //TIM_OE pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x04, 0x00, 0x08);

    //SFL Pin Disable (DE)
    _ADV7180_WriteI2c_ByteMask(0x04, 0x00, 0x02);

    //HS, VS, FIELD Data pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x03, 0x40, 0x40);
}

void Set_ADV7180_Tri_State_Disable()
{
    //LLC pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x1D, 0x00, 0x80);

    //TIM_OE pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x04, 0x08, 0x08);

    //SFL Pin Disable (DE)
    _ADV7180_WriteI2c_ByteMask(0x04, 0x00, 0x02);

    //HS, VS, FIELD Data pin Tri-State
    _ADV7180_WriteI2c_ByteMask(0x03, 0x00, 0x40);
}

ADV7180_INPUT_STANDARD Get_Auto_Detection_Result()
{
    uint8_t result;

    result = _ADV7180_ReadI2c_Byte(REG_STATUS1);
    result = (result & 0x70) >> 4;

    switch (result)
    {
    case ADV7180_NTSM_M_J:
        ADV7180_InWidth     = 720;
        ADV7180_InHeight    = 487; //480;
        ADV7180_InFrameRate = 2997;
        gtADV7180CurMode    = ADV7180_NTSM_M_J;
        printf("NTSM M/J\n");
        break;

    case ADV7180_NTSC_4_43:
        ADV7180_InWidth     = 720;
        ADV7180_InHeight    = 480;
        ADV7180_InFrameRate = 2997;
        gtADV7180CurMode    = ADV7180_NTSC_4_43;
        printf("NTSC 4.43\n");
        break;

    //case ADV7180_PAL_M:
    //  ADV7180_InWidth = 720;
    //  ADV7180_InHeight = 480;
    //  ADV7180_InFrameRate = 2997;
    //  gtADV7180CurMode = ADV7180_PAL_M;
    //  printf("PAL_M\n");
    //  break;
    //case ADV7180_PAL_60:
    //  ADV7180_InWidth = 720;
    //  ADV7180_InHeight = 480;
    //  ADV7180_InFrameRate = 3000;
    //  gtADV7180CurMode = ADV7180_PAL_60;
    //  printf("PAL_60\n");
    //  break;
    case ADV7180_PAL_B_G_H_I_D:
        ADV7180_InWidth     = 720;
        ADV7180_InHeight    = 576;
        ADV7180_InFrameRate = 2500;
        gtADV7180CurMode    = ADV7180_PAL_B_G_H_I_D;
        printf("0 PAL B/G/H/I/D\n");
        break;

    //case ADV7180_SECAM:
    //  ADV7180_InWidth = 720;
    //  ADV7180_InHeight = 576;
    //  ADV7180_InFrameRate = 2500;
    //  gtADV7180CurMode = ADV7180_SECAM;
    //  printf("SECAM\n");
    //  break;
    //case ADV7180_PAL_COMBINATION_N:
    //  ADV7180_InWidth = 720;
    //  ADV7180_InHeight = 576;
    //  ADV7180_InFrameRate = 2500;
    //  gtADV7180CurMode = ADV7180_PAL_COMBINATION_N;
    //  printf("PAL Combination N\n");
    //  break;
    //case ADV7180_SECAM_525:
    //  ADV7180_InWidth = 720;
    //  ADV7180_InHeight = 480;
    //  ADV7180_InFrameRate = 2997;
    //  gtADV7180CurMode = ADV7180_SECAM_525;
    //  printf("SECAM 525\n");
    //  break;
    default:
        printf("Can not recognize\n");
        break;
    }
    return result;
}

uint16_t _ADV7180_InputSelection()
{
#ifdef AUTO_DETECT_INPUT
    uint16_t Value;

    Value = _ADV7180_ReadI2c_Byte(REG_STATUS1);
    if ((Value & 0x05) != 0x05)
    {
        Value = _ADV7180_ReadI2c_Byte(0x00);
        if ((Value & 0x0F) == 0x06)
        {
            ADV7180_Input_Mode(ADV7180_INPUT_CVBS);
            return ADV7180_INPUT_CVBS;
        }
        else
        {
            ADV7180_Input_Mode(ADV7180_INPUT_SVIDEO);
            return ADV7180_INPUT_SVIDEO;
        }
    }
    else
    {
        Value = _ADV7180_ReadI2c_Byte(0x00);
        if ((Value & 0x0F) == 0x06)
            return ADV7180_INPUT_SVIDEO;
        else
            return ADV7180_INPUT_CVBS;
    }
#else
    uint16_t Value;
    Value = _ADV7180_ReadI2c_Byte(0x00);
    if ((Value & 0x0F) == 0x06)
        return ADV7180_INPUT_SVIDEO;
    else
        return ADV7180_INPUT_CVBS;
#endif
}

void ADV7180Initial(ADV7180_INPUT_MODE mode)
{
    ADV7180_PowerDown(MMP_FALSE);
    gtADV7180InitDone = MMP_FALSE;
    _ADV7180_SWReset();

    gtADV7180CurMode  = 0xFF;
    gtADV7180PreMode  = 0xFF;

    usleep(1000 * 10);

    ADV7180_Input_Mode(mode);
    gtADV7180CurDev   = mode;
    gtADV7180InitDone = MMP_TRUE;
}

void ADV7180_Input_Mode(ADV7180_INPUT_MODE mode)
{
    if (mode == ADV7180_INPUT_CVBS)
        _Set_ADV7180_Input_CVBS();
    else if (mode == ADV7180_INPUT_SVIDEO)
        _Set_ADV7180_Input_SVIDEO();
    else if (mode == ADV7180_INPUT_YPBPR)
        _Set_ADV7180_Input_YPrPb();

    //Y Range 16 - 235, UV Range 16 - 240
    _ADV7180_WriteI2c_ByteMask(0x04, 0x00, 0x01);

    //_ADV7180_WriteI2c_ByteMask(0x04, 0x01, 0x02);  // Benson test
    //Lock status set by horizontal lock and subcarrier lock
    //_ADV7180_WriteI2c_ByteMask(0x51, 0x80, 0x80);

    //drive strength
    _ADV7180_WriteI2c_Byte(0xF4, 0x04);
}

MMP_BOOL ADV7180_IsStable()
{
    uint16_t Value;
    uint16_t IsStable;

    if (!gtADV7180InitDone)
        return MMP_FALSE;

    Value = _ADV7180_ReadI2c_Byte(REG_STATUS1);
    if (((Value & 0x85) == 0x05) || ((Value & 0x85) == 0x81)) //Color Burst or No Color Burst
    {
        Get_Auto_Detection_Result();

        if (gtADV7180CurMode != gtADV7180PreMode)
        {
            printf("--------ADV7180 Resolution = \n");
            if (gtADV7180CurMode == ADV7180_NTSM_M_J)
                printf("NTSM_M_J\n");
            else if (gtADV7180CurMode == ADV7180_NTSC_4_43)
                printf( "NTSC_4_43\n");
            else if (gtADV7180CurMode == ADV7180_PAL_M)
                printf("PAL_M\n");
            else if (gtADV7180CurMode == ADV7180_PAL_60)
                printf("PAL_60\n");
            else if (gtADV7180CurMode == ADV7180_PAL_B_G_H_I_D)
                printf("PAL_B_G_H_I_D\n");
            else if (gtADV7180CurMode == ADV7180_SECAM)
                printf("SECAM\n");
            else if (gtADV7180CurMode == ADV7180_PAL_COMBINATION_N)
                printf("PAL_COMBINATION_N\n");
            else if (gtADV7180CurMode == ADV7180_SECAM_525)
                printf("SECAM_525\n");
            else
                printf("Unknow Format\n");

            if (gtADV7180CurDev == ADV7180_INPUT_CVBS)
                printf("----CVBS ---------\n");
            else if (gtADV7180CurDev == ADV7180_INPUT_SVIDEO)
                printf("----S-Video ------\n");

            gtADV7180PreMode = gtADV7180CurMode;
        }
        IsStable = MMP_TRUE;
    }
    else
    {
        gtADV7180CurDev = _ADV7180_InputSelection();
        IsStable        = MMP_FALSE;
    }

    return IsStable;
}

void ADV7180_PowerDown(
    MMP_BOOL enable)
{
    //When PDBP is set to 1, setting the PWRDWN bit switches the ADV7180 to a chip-wide power-down mode.
    _ADV7180_WriteI2c_ByteMask(POWER_MANAGEMENT, 0x04, 0x04);

    if (enable)
    {
        gtADV7180InitDone = MMP_FALSE;
        _ADV7180_WriteI2c_ByteMask(POWER_MANAGEMENT, 0x20, 0x20);
    }
    else
        _ADV7180_WriteI2c_ByteMask(POWER_MANAGEMENT, 0x00, 0x20);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

//X10LightDriver_t1.c
void ADV7180Initialize(void)
{
    printf("ADV7180Initialize here\n");
    ADV7180Initial(ADV7180_INPUT_CVBS);
}

void ADV7180Terminate(void)
{
    printf("ADV7180Terminate\n");
}

void ADV7180OutputPinTriState(unsigned char flag)
{
    if (flag == MMP_TRUE)
        Set_ADV7180_Tri_State_Enable();
    else
        Set_ADV7180_Tri_State_Disable();
}

unsigned char ADV7180IsSignalStable(void)
{
    MMP_BOOL isStable;
    isStable = ADV7180_IsStable();
    return isStable;
}

void ADV7180GetProperty(CAP_GET_PROPERTY * pGetProperty)
{
    uint16_t i;

    Get_Auto_Detection_Result();
    pGetProperty->GetTopFieldPolarity = MMP_TRUE;
    pGetProperty->GetHeight = ADV7180_InHeight;
    pGetProperty->GetWidth  = ADV7180_InWidth;
    pGetProperty->Rate = ADV7180_InFrameRate;
    pGetProperty->GetModuleIsInterlace = 1;

    for (i = 0; i < (sizeof(ADV7180_TABLE) / sizeof(CAP_TIMINFO_TABLE)); ++i)
    {
       if ((pGetProperty->GetWidth == ADV7180_TABLE[i].HActive) && (pGetProperty->GetHeight == ADV7180_TABLE[i].VActive) && pGetProperty->Rate == ADV7180_TABLE[i].Rate)
        {
            pGetProperty->HPolarity       = ADV7180_TABLE[i].HPolarity; 
            pGetProperty->VPolarity       = ADV7180_TABLE[i].VPolarity;
            pGetProperty->FrameRate       = ADV7180_TABLE[i].FrameRate; 
            pGetProperty->matchResolution = MMP_TRUE;

            pGetProperty->HStar  = ADV7180_TABLE[i].HStar;
            pGetProperty->HEnd   = ADV7180_TABLE[i].HEnd;
            pGetProperty->VStar1 = ADV7180_TABLE[i].VStar1;
            pGetProperty->VEnd1  = ADV7180_TABLE[i].VEnd1;
            pGetProperty->VStar2 = ADV7180_TABLE[i].VStar2; 
            pGetProperty->VEnd2  = ADV7180_TABLE[i].VEnd2; 
        }
    }
}

void ADV7180PowerDown(unsigned char enable)
{
    ADV7180_PowerDown(enable);
}

void ADV7180ForCaptureDriverSetting(CAP_CONTEXT *Capctxt )
{
    /* Input Format default Setting */
    Capctxt->Interleave   = Interleaving;

    /* Input Data Format Setting */
    // 8bit bus
    Capctxt->YUV422Format  = CAP_IN_YUV422_UYVY;
    Capctxt->EnDEMode             = MMP_FALSE;
    Capctxt->input_protocol       = BT_656;
}

static void ADV7180CaptureModuleDriver_Destory(CaptureModuleDriver base)
{
    ADV7180CaptureModuleDriver self = (ADV7180CaptureModuleDriver)base;
    if(self)
        free(self);
}

static CaptureModuleDriverInterfaceStruct interface =
{
    ADV7180Initialize,
    ADV7180Terminate,
    ADV7180OutputPinTriState,
    ADV7180IsSignalStable,
    ADV7180GetProperty,
    ADV7180PowerDown,
    ADV7180ForCaptureDriverSetting,
    ADV7180CaptureModuleDriver_Destory
};

CaptureModuleDriver ADV7180CaptureModuleDriver_Create()
{
    ADV7180CaptureModuleDriver self = calloc(1, sizeof(ADV7180CaptureModuleDriverStruct));
    self->base.vtable = &interface;
    self->base.type = "ADV7180";
    return (ADV7180CaptureModuleDriver)self;
}
//end of X10LightDriver_t1.c
