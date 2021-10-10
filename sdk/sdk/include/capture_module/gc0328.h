#ifndef __GC0328_H__
#define __GC0328_H__

#include "ite/itp.h"
#include "ite/ith_defs.h"
#include "ite/ith.h"

#include "encoder/encoder_types.h"
#include "capture_module.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MMP_TRUE
    #define MMP_TRUE  1
#endif
#ifndef MMP_FALSE
    #define MMP_FALSE 0
#endif

typedef enum CAP_FRAMERATE_TAG
{
    CAP_FRAMERATE_UNKNOW = 0,
    CAP_FRAMERATE_30HZ,
} CAP_FRAMERATE;

uint16_t GC0328_InWidth;
uint16_t GC0328_InHeight;
uint16_t GC0328_InFrameRate;

void Set_GC0328_Tri_State_Enable();
void Set_GC0328_Tri_State_Disable();

MMP_BOOL GC0328_IsStable();
void GC0328_PowerDown(
    MMP_BOOL enable);

typedef struct GC0328CaptureModuleDriverStruct *GC0328CaptureModuleDriver;
CaptureModuleDriver GC0328CaptureModuleDriver_Create();
static void GC0328CaptureModuleDriver_Destory(CaptureModuleDriver base);
void GC0328Initialize(void);
void GC0328Terminate(void);
void GC0328OutputPinTriState(unsigned char flag);
void GC0328GetProperty(CAP_GET_PROPERTY *pGetProperty);
void GC0328PowerDown(unsigned char enable);
void GC0328ForCaptureDriverSetting(CAP_CONTEXT *Capctxt );
unsigned char GC0328IsSignalStable(void);

#ifdef __cplusplus
}
#endif

#endif