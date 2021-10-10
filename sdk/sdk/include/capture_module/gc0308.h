#ifndef __GC0308_H__
#define __GC0308_H__

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

typedef enum CAP_FRAMERATE_TAG
{
    CAP_FRAMERATE_UNKNOW = 0,
    CAP_FRAMERATE_10HZ,
} CAP_FRAMERATE;

uint16_t GC0308_InWidth;
uint16_t GC0308_InHeight;
uint16_t GC0308_InFrameRate;

void Set_GC0308_Tri_State_Enable();

void Set_GC0308_Tri_State_Disable();

void GC0308Initial();

MMP_BOOL GC0308_IsStable();

void GC0308_PowerDown(
    MMP_BOOL enable);

//X10LightDriver_t1.h
typedef struct GC0308CaptureModuleDriverStruct *GC0308CaptureModuleDriver;
CaptureModuleDriver GC0308CaptureModuleDriver_Create();
static void GC0308CaptureModuleDriver_Destory(CaptureModuleDriver base);
void GC0308Initialize(void);
void GC0308Terminate(void);
void GC0308OutputPinTriState(unsigned char flag);
void GC0308GetProperty(CAP_GET_PROPERTY *pGetProperty);
void GC0308PowerDown(unsigned char enable);
void GC0308ForCaptureDriverSetting(CAP_CONTEXT *Capctxt );
unsigned char GC0308IsSignalStable(void);
//end of X10LightDriver_t1.h

#ifdef __cplusplus
}
#endif

#endif