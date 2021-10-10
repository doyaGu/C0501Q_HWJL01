#ifndef __CAP_HW_H__
#define __CAP_HW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "capture_s/capture_types.h"

//=============================================================================
//                Constant Definition
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

void
CaptureControllerHardware_Run(
    void);

void
CaptureControllerHardware_Stop(
    void);

void
CaptureControllerHardware_SetColorFormat(
    MMP_UINT8 YUV422Format);

void
CaptureControllerHardware_SetInterleaveMode(
    CAP_INPUT_VIDEO_FORMAT Interleave);

void
CaptureControllerHardware_SetInputDataInfo(
    CAP_INPUT_INFO *pIninfo);

void
CaptureControllerHardware_Reset(
    void);

void
CaptureControllerHardware_DumpAllRegister(
    void);

void
CaptureControllerHardware_UseTripleBuffer(
    MMP_BOOL flag);

void
CaptureControllerHardware_SetDataEnable(
    MMP_BOOL EnDEMode);

void 
CaptureControllerHardware_SetInputProtocol(
    CAP_INPUT_PROTOCOL_INFO pInputProtocol);

void
CaptureControllerHardware_SetBufferAddress (
    MMP_UINT32 *pAddr);

void
Cap_EnableClock(
    void);

void
Cap_DisableClock(
    void);

MMP_UINT16
CaptureControllerHardwareGetBufferIndex(
    void);

#ifdef __cplusplus
}
#endif

#endif