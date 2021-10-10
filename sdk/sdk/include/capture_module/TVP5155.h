#ifndef __TVP5155_H__
#define __TVP5155_H__

#include "ite/itp.h"
#include "ite/ith_defs.h"
#include "ite/ith.h"

//#include "m2d/m2d_types.h"
#include "encoder/encoder_types.h"
#include "capture_module.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 *
 */
#ifndef MMP_BOOL
    #define MMP_BOOL  unsigned char
#endif
#ifndef MMP_TRUE
    #define MMP_TRUE  1
#endif
#ifndef MMP_FALSE
    #define MMP_FALSE 0
#endif

#define COMPOSITE_DEV //Benson

#if 1
typedef struct CaptureModuleDriverTVP5155Struct *TVP5155CaptureModuleDriver;
CaptureModuleDriver TVP5155CaptureModule_Create();

/* Functions just needed by the spy */
//void LightDriverSpy_Reset(void);
//int LightDriverSpy_GetState(int id);
//int LightDriverSpy_GetLastId(void);
//int LightDriverSpy_GetLastState(void);
//void LightDriverSpy_AddSpiesToController(void);

//doesn`t use it.
//void TVP5155CaptureModuleDriver_InstallInterface(void);
//void TVP5155CaptureModule_Destroy(CaptureModuleDriver);
//void TVP5155CaptureModule_TurnOn(CaptureModuleDriver);
//void TVP5155CaptureModule_TurnOff(CaptureModuleDriver);
//end of doesn`t use it.

enum {
    LIGHT_ID_UNKNOWN = -1, LIGHT_STATE_UNKNOWN = -1,
    LIGHT_OFF        = 0, LIGHT_ON =1
};
#endif

#ifdef __cplusplus
}
#endif

#endif