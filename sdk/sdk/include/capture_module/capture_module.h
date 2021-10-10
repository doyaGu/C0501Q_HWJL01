#ifndef __CAPTURE_MODULE_H__
#define __CAPTURE_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "capture_s/capture_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================


//=============================================================================
//                              Constant Definition
//=============================================================================


//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct CAP_GET_PROPERTY_TAG
{
    MMP_UINT16  GetTopFieldPolarity;
    MMP_UINT16  GetHeight;
    MMP_UINT16  GetWidth;
    MMP_UINT16  Rate;
    MMP_UINT16  GetModuleIsInterlace;
    MMP_UINT16  FrameRate;
    MMP_UINT16  matchResolution;
    MMP_UINT16  HPolarity : 1;
    MMP_UINT16  VPolarity : 1;
    MMP_UINT16  HStar;
    MMP_UINT16  HEnd;
    MMP_UINT16  VStar1;
    MMP_UINT16  VEnd1;
    MMP_UINT16  VStar2;
    MMP_UINT16  VEnd2;
} CAP_GET_PROPERTY;

typedef struct CAP_TIMINFO_TABLE_TAG
{
    MMP_UINT16 Index;
    MMP_UINT16 HActive;
    MMP_UINT16 VActive;
    MMP_UINT16 Rate;
    MMP_UINT16 FrameRate;
    MMP_UINT16 HPolarity : 1;
    MMP_UINT16 VPolarity : 1;
    MMP_UINT16 HStar;
    MMP_UINT16 HEnd;
    MMP_UINT16 VStar1;
    MMP_UINT16 VEnd1;
    MMP_UINT16 VStar2;
    MMP_UINT16 VEnd2;
} CAP_TIMINFO_TABLE;

typedef enum CAPTURE_MODULE_GET_PROPERTY_TAG
{
    CAPTURE_MODULE_PROPRRTY_HEIGHT,
    CAPTURE_MODULE_PROPERTY_WIDTH,
    CAPTURE_MODULE_PROPERTY_FRAMERATE,
    CAPTURE_MODULE_PROPERTY_IS_INTERLACE,
} CAPTURE_MODULE_GET_PROPERTY;

//Benson add for function pointer.
//LightDriver_t1.h
typedef struct CaptureModuleDriverStruct * CaptureModuleDriver;

typedef struct CaptureModuleDriverInterfaceStruct * CaptureModuleDriverInterface;


typedef struct CaptureModuleDriverStruct
{
    CaptureModuleDriverInterface vtable;
    const char * type;
} CaptureModuleDriverStruct;
//end of LightDriver_t1.h


//LightDriverPrivate_t2.h
typedef struct CaptureModuleDriverInterfaceStruct
{
    void (*Init)(void);
    void (*Terminate)(void);
    void (*OutputPinTriState)(unsigned char flag);
    unsigned char (*IsSignalStable)(void);
    void(*GetProperty)(CAP_GET_PROPERTY *CapGetProperty);
    void (*PowerDown)(unsigned char enable);
    void (*ForCaptureDriverSetting)(CAP_CONTEXT *Capctxt);
    void (*Destroy)(CaptureModuleDriver);
} CaptureModuleDriverInterfaceStruct;
//end of LightDriverPrivate_t2.h

//LightDriver_t3.h
CaptureModuleDriver CaptureModuleDriver_GetDevice(unsigned char* moduleName);
unsigned char CaptureModuleDriver_IsSignalStable(CaptureModuleDriver);
void CaptureModuleDriver_Init(CaptureModuleDriver);
void CaptureModuleDriver_Destroy(CaptureModuleDriver);
void CaptureModuleDriver_DeInit(CaptureModuleDriver);
void CaptureModuleDriver_GetProperty(CaptureModuleDriver ,CAP_GET_PROPERTY *CapGetProperty);
void CaptureModuleDriver_PowerDown(CaptureModuleDriver , unsigned char enable);
void CaptureModuleDriver_OutputPinTriState(CaptureModuleDriver , unsigned char flag);
void CaptureModuleDriver_ForCaptureDriverSetting(CaptureModuleDriver , CAP_CONTEXT *Capctxt);
//end of LightDriver_t3.h




//=============================================================================
//                Macro Definition
//=============================================================================


//=============================================================================
//                Structure Definition
//=============================================================================


//=============================================================================
//                Global Data Definition
//=============================================================================


//=============================================================================
//                Private Function Definition
//=============================================================================


//=============================================================================
//                Public Function Definition
//=============================================================================

    
#ifdef __cplusplus
}
#endif

#endif


