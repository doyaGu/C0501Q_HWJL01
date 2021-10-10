#ifndef __HM1375_H__
#define __HM1375_H__

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

uint16_t HM1375_InWidth;
uint16_t HM1375_InHeight;
uint16_t HM1375_InFrameRate;

typedef struct HM1375CaptureModuleDriverStruct *HM1375CaptureModuleDriver;
CaptureModuleDriver HM1375CaptureModuleDriver_Create();
static void HM1375CaptureModuleDriver_Destory(CaptureModuleDriver base);
void HM1375Initialize(void);
void HM1375Terminate(void);
void HM1375OutputPinTriState(unsigned char flag);
void HM1375GetProperty(CAP_GET_PROPERTY *pGetProperty);
void HM1375PowerDown(unsigned char enable);
void HM1375ForCaptureDriverSetting(CAP_CONTEXT *Capctxt );
unsigned char HM1375IsSignalStable(void);

#ifdef __cplusplus
}
#endif

#endif