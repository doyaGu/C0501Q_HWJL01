#ifndef __MMP_SENSOR_H__
#define __MMP_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "encoder/encoder_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#if defined(WIN32)

#if defined(SENSOR_EXPORTS)
#define SENSOR_API __declspec(dllexport)
#else
#define SENSOR_API __declspec(dllimport)
#endif

#else
#define SENSOR_API extern
#endif //#if defined(WIN32)

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum MMP_SENSOR_FLICK_MODE_TAG
{
    SENSOR_FLICK_MODE_AUTO,
    SENSOR_FLICK_MODE_50HZ,
    SENSOR_FLICK_MODE_60HZ,
    SENSOR_FLICK_MODE_OFF
}MMP_SENSOR_FLICK_MODE;

//=============================================================================
//                              Structure Definition
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


//=============================================================================
//                Private Function Definition
//=============================================================================


//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
* Device Initialize
*/
//=============================================================================
SENSOR_API void
mmpSensorInitialize(
    void);

void
mmpSensorSetEffectDefault(
    void);

void
mmpSensorImageMirror(
    MMP_BOOL bEnHorMirror,
    MMP_BOOL bEnVerMirror);

void
mmpSensorSetBrightness(
    MMP_UINT8 value);

void
mmpSensorGetBrightness(
    MMP_UINT8 *value);

void
mmpSensorSetContrast(
    MMP_UINT8 value);

void
mmpSensorGetContrast(
    MMP_UINT8 *value);

void
mmpSensorSetSaturation(
    MMP_UINT8 value);

void
mmpSensorGetSaturation(
    MMP_UINT8 *value);

void
mmpSensorSetSharpness(
    MMP_UINT8 value);

void
mmpSensorGetSharpness(
    MMP_UINT8 *value);

void
mmpSensorSetFlickMode(
    MMP_SENSOR_FLICK_MODE mode);

MMP_SENSOR_FLICK_MODE
mmpSensorGetFlickMode(
    void);
#ifdef __cplusplus
}
#endif

#endif
