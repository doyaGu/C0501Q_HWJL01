#ifndef __ADV7180_H__
#define __ADV7180_H__

#include "ite/itp.h"
#include "ite/ith_defs.h"
#include "ite/ith.h"

#include "encoder/encoder_types.h"
#include "capture_module.h" //Benson

#ifdef __cplusplus
extern "C" {
#endif

/*
 *
 */
//#define	MMP_BOOL	unsigned char

#ifndef MMP_TRUE
    #define MMP_TRUE  1
#endif
#ifndef MMP_FALSE
    #define MMP_FALSE 0
#endif

#define COMPOSITE_DEV //Benson

typedef enum _ADV7180_INPUT_MODE
{
    ADV7180_INPUT_CVBS   = 0,
    ADV7180_INPUT_SVIDEO = 1,
    ADV7180_INPUT_YPBPR  = 2,
} ADV7180_INPUT_MODE;

typedef enum _ADV7180_INPUT_STANDARD
{
    ADV7180_NTSM_M_J          = 0x0,
    ADV7180_NTSC_4_43         = 0x1,
    ADV7180_PAL_M             = 0x2,
    ADV7180_PAL_60            = 0x3,
    ADV7180_PAL_B_G_H_I_D     = 0x4,
    ADV7180_SECAM             = 0x5,
    ADV7180_PAL_COMBINATION_N = 0x6,
    ADV7180_SECAM_525         = 0x7,
} ADV7180_INPUT_STANDARD;

typedef enum CAP_FRAMERATE_TAG
{
    CAP_FRAMERATE_UNKNOW = 0,
    CAP_FRAMERATE_25HZ,
    CAP_FRAMERATE_50HZ,
    CAP_FRAMERATE_30HZ,
    CAP_FRAMERATE_60HZ,
    CAP_FRAMERATE_29_97HZ,
    CAP_FRAMERATE_59_94HZ,
    CAP_FRAMERATE_23_97HZ,
    CAP_FRAMERATE_24HZ,
    CAP_FRAMERATE_56HZ,
    CAP_FRAMERATE_70HZ,
    CAP_FRAMERATE_72HZ,
    CAP_FRAMERATE_75HZ,
    CAP_FRAMERATE_85HZ,
    CAP_FRAMERATE_VESA_30HZ,
    CAP_FRAMERATE_VESA_60HZ
} CAP_FRAMERATE;

uint16_t ADV7180_InWidth;
uint16_t ADV7180_InHeight;
uint16_t ADV7180_InFrameRate;

void Set_ADV7180_Tri_State_Enable();

void Set_ADV7180_Tri_State_Disable();

ADV7180_INPUT_STANDARD Get_Auto_Detection_Result();

void ADV7180Initial(ADV7180_INPUT_MODE mode);

void ADV7180_Input_Mode(ADV7180_INPUT_MODE mode);

MMP_BOOL ADV7180_IsStable();

void ADV7180_PowerDown(
    MMP_BOOL enable);

//X10LightDriver_t1.h
typedef struct ADV7180CaptureModuleDriverStruct *ADV7180CaptureModuleDriver;
CaptureModuleDriver ADV7180CaptureModuleDriver_Create();
static void ADV7180CaptureModuleDriver_Destory(CaptureModuleDriver base);
void ADV7180Initialize(void);
void ADV7180Terminate(void);
void ADV7180OutputPinTriState(unsigned char flag);
void ADV7180GetProperty(CAP_GET_PROPERTY *pGetProperty);
void ADV7180PowerDown(unsigned char enable);
void ADV7180ForCaptureDriverSetting(CAP_CONTEXT *Capctxt );
unsigned char ADV7180IsSignalStable(void);
//end of X10LightDriver_t1.h

#ifdef __cplusplus
}
#endif

#endif