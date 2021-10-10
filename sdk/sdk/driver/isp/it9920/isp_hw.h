#ifndef __ISP_HW_H_HL88MIOL_N62I_1CVD_9BT8_WHZ2ZK9PMMDS__
#define __ISP_HW_H_HL88MIOL_N62I_1CVD_9BT8_WHZ2ZK9PMMDS__

#ifdef __cplusplus
extern "C" {
#endif

#include "isp_types.h"

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
ISP_LogReg(
    const MMP_ISP_CORE ispCore);

//=============================================================================
/**
 * Driver Fire ISP Engine.
 */
//=============================================================================
void
ISP_DriverFire_Reg(
    const ISP_OUTPUT_INFO *pOutInfo,
    const MMP_ISP_CORE    ispCore);

//=============================================================================
/**
 * Driver Update ISP Parameter.
 */
//=============================================================================
void
ISP_UpdateFire_Reg(
    const ISP_OUTPUT_INFO *pOutInfo,
    const MMP_ISP_CORE     ispCore);

//=============================================================================
/**
 * Driver Write Raw Slice Buffer ISP Engine.
 */
//=============================================================================
void
ISP_WirteRawSliceFire_Reg(
    const ISP_OUTPUT_INFO *pOutInfo,
    const MMP_ISP_CORE    ispCore);

//=============================================================================
/**
 * Set ISP Engine Mode and Output Format.
 */
//=============================================================================
void
ISP_SetEngineModeOutParameter_Reg(
    const ISP_ENGINE_MODE_CTRL *pEngineMode,
    const ISP_OUTPUT_INFO      *pOutInfo,
    const MMP_ISP_CORE         ispCore);

//=============================================================================
/**
 * VP1 API
 */
//=============================================================================
void
ISP_SetPVPreviewMode_Reg(
    ISP_DEVICE ptDev);

//=============================================================================
/**
 * VP1 API
 */
//=============================================================================
void
ISP_SetPVOutParameter_Reg(
    const ISP_PV_OUTPUT_INFO *pOutInfo,
    const MMP_ISP_CORE       ispCore);

//=============================================================================
/**
 * Set ISP input buffer relate parameters.
 */
//=============================================================================
void
ISP_SetInputBuf_Reg(
    ISP_DEVICE           ptDev,
    const ISP_INPUT_INFO *pInInfo);

//=============================================================================
/**
 * Set ISP input buffer address relate parameters
 */
//=============================================================================
void
ISP_SetInputAddr_Reg(
    ISP_DEVICE           ptDev,
    const ISP_INPUT_INFO *pInInfo);

//=============================================================================
/**
 * Set Deinterlace Parameter.
 */
//=============================================================================
void
ISP_SetDeInterlaceInputParam_Reg(
    ISP_DEVICE                 ptDev,
    const ISP_DEINTERLACE_CTRL *pDeInterlace,
    const ISP_INPUT_INFO       *pInInfo);

//=============================================================================
/**
 * Set 3D-Deinterlace parameters.
 */
//=============================================================================
void
ISP_Set3DDeInterlaceParm_Reg(
    ISP_DEVICE ptDev);

//=============================================================================
/**
 * Set 2D-Deinterlace parameters.
 */
//=============================================================================
void
ISP_Set2DDeInterlaceParam_Reg(
    ISP_DEVICE ptDev);

//=============================================================================
/**
 * Set Subtitle 0.
 */
//=============================================================================
void
ISP_SetSubTitle0_Reg(
    const ISP_SUBTITLE_CTRL *pSubTitle);

//=============================================================================
/**
 * Set Subtitle 1
 */
//=============================================================================
void
ISP_SetSubTitle1_Reg(
    const ISP_SUBTITLE_CTRL *pSubTitle);

//=============================================================================
/**
 * YUV to RGB transfer matrix.
 */
//=============================================================================
void
ISP_SetYUVtoRGBMatrix_Reg(
    const ISP_YUV_TO_RGB *pYUVtoRGB,
    const MMP_ISP_CORE    ispCore);

//=============================================================================
/**
 * Set color correction matrix and constant
 */
//=============================================================================
void
ISP_SetCCMatrix_Reg(
    const ISP_COLOR_CORRECTION *pColorCorrect,
    const MMP_ISP_CORE         ispCore);

//=============================================================================
/*
 * Set Pre-Scale Register
 */
//=============================================================================
void
ISP_SetPreScaleParam_Reg(
    const ISP_PRESCALE_CTRL *pPreScaleFun,
    const MMP_ISP_CORE      ispCore);

//=============================================================================
/**
 * Set Pre-Scale Weight.
 */
//=============================================================================
void
ISP_SetPreScaleMatrix_Reg(
    const ISP_PRESCALE_CTRL *pPreScaleFun);

void
    ISP_SetIntPreScaleMatrix_Reg(
    MMP_UINT8          WeightMatX[][ISP_SCALE_TAP],
    const MMP_ISP_CORE ispCore);

//=============================================================================
/*
 * Set Scale Factor
 */
//=============================================================================
void
ISP_SetScaleParam_Reg(
    const ISP_SCALE_CTRL *pScaleFun,
    const MMP_ISP_CORE   ispCore);

//=============================================================================
/*
 * VP1 API
 */
//=============================================================================
void
ISP_SetPVScaleParam_Reg(
    const ISP_SCALE_CTRL *pScaleFun,
    const MMP_ISP_CORE   ispCore);

//=============================================================================
/**
 * Set Scale Horizontal Weight.
 */
//=============================================================================
void
ISP_SetScaleMatrixH_Reg(
    const ISP_SCALE_CTRL *pScaleFun);

void
    ISP_SetIntScaleMatrixH_Reg(
    MMP_UINT8          WeightMatX[][ISP_SCALE_TAP],
    const MMP_ISP_CORE ispCore);

//=============================================================================
/**
 * Set Scale Vertical Weight.
 */
//=============================================================================
void
ISP_SetScaleMatrixV_Reg(
    const ISP_SCALE_CTRL *pScaleFun);

void
    ISP_SetIntScaleMatrixV_Reg(
    MMP_UINT8          WeightMatY[][ISP_SCALE_TAP],
    const MMP_ISP_CORE ispCore);

//=============================================================================
/**
 * VP1 API
 */
//=============================================================================
void
    ISP_SetIntPVScaleMatrixH_Reg(
    MMP_UINT8 WeightMatX[][ISP_SCALE_TAP],
    const     MMP_ISP_CORE ispCore);

//=============================================================================
/**
 * VP1 API
 */
//=============================================================================
void
    ISP_SetIntPVScaleMatrixV_Reg(
    MMP_UINT8 WeightMatY[][ISP_SCALE_TAP],
    const     MMP_ISP_CORE ispCore);

//=============================================================================
/**
 * FrmFun RGB to YUV transfer matrix.
 */
//=============================================================================
void
ISP_SetFrmMatrix_Reg(
    const ISP_RGB_TO_YUV *pMatrix,
    const MMP_ISP_CORE   ispCore);

//=============================================================================
/**
 * Set Frame Function 0
 */
//=============================================================================
void
ISP_SetFrameFun0_Reg(
    const ISP_FRMFUN_CTRL *pFrameFun,
    const MMP_ISP_CORE    ispCore);

//=============================================================================
/**
 * Set Frame Function 1
 */
//=============================================================================
void
ISP_SetFrameFun1_Reg(
    const ISP_FRMFUN_CTRL *pFrameFun,
    const MMP_ISP_CORE    ispCore);

//=============================================================================
/**
 * RGB to YUV transfer matrix.
 */
//=============================================================================
void
ISP_SetRGBtoYUVMatrix_Reg(
    const ISP_RGB_TO_YUV *pRGBtoYUV,
    const MMP_ISP_CORE   ispCore);

//=============================================================================
/**
 * Set clip0 function.
 */
//=============================================================================
void
ISP_SetClip0Fun_Reg(
    const ISP_CLIP_FN_CTRL *pClipFun,
    const MMP_ISP_CORE     ispCore);

//=============================================================================
/**
 * Set clip1 function.
 */
//=============================================================================
void
ISP_SetClip1Fun_Reg(
    ISP_DEVICE             ptDev,
    const ISP_CLIP_FN_CTRL *pClipFun);

//=============================================================================
/**
 * Set clip2 function.
 */
//=============================================================================
void
ISP_SetClip2Fun_Reg(
    ISP_DEVICE             ptDev,
    const ISP_CLIP_FN_CTRL *pClipFun);

//=============================================================================
/**
 * Set Output Information
 */
//=============================================================================
void
ISP_SetOutAddressBufInfo_Reg(
    const ISP_OUTPUT_INFO *pOutInfo,
    const MMP_ISP_CORE    ispCore);

//=============================================================================
/**
 * VP1 API
 */
//=============================================================================
void
ISP_SetPVOutAddressBufInfo_Reg(
    const ISP_PV_OUTPUT_INFO *pOutInfo,
    const MMP_ISP_CORE       ispCore);

//=============================================================================
/**
 * Wait ISP engine idle!  //for JPG module use
 */
//=============================================================================
ISP_RESULT
ISP_WaitEngineIdle(
    ISP_DEVICE ptDev);

//=============================================================================
/**
 * Wait ISP change idle!
 */
//=============================================================================
ISP_RESULT
ISP_WaitISPChangeIdle(
    void);

//=============================================================================
/**
 * Wait ISP engine slice buffer empty!  //for JPG module use
 */
//=============================================================================
ISP_RESULT
ISP_WaitRawSliceBufferEmpty(
    void);

//=============================================================================
/**
 * Wait LCD Frame Buffer Idle in Queue Fire Mode
 */
//=============================================================================
ISP_RESULT
ISP_WaitQueueFireIdle(
    void);

//=============================================================================
/**
 * Set LCD OnFly register
 */
//=============================================================================
void
ISP_EnableLCD_OnFly(
    void);

void
ISP_DisableLCD_OnFly(
    void);

//=============================================================================
/**
 * ISP engine clock related.
 */
//=============================================================================
void
ISP_PowerUp(
    void);

void
ISP_PowerDown(
    void);

#ifdef __cplusplus
}
#endif

#endif