#ifndef __MMP_VP_H_2XXFCCN9_G1EL_C42M_SEM9_S29A3L5FADVX__
#define __MMP_VP_H_2XXFCCN9_G1EL_C42M_SEM9_S29A3L5FADVX__

#ifdef __cplusplus
extern "C" {
#endif

//#include "mmp_types.h"
#include "vp/vp_types.h"
#include "vp/vp_error.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#if defined(WIN32)

    #if defined(VP_EXPORTS)
        #define VP_API __declspec(dllexport)
    #else
        #define VP_API __declspec(dllimport)
    #endif

#else
    #define VP_API     extern
#endif  //#if defined(WIN32)

//=============================================================================
//                              Constant Definition
//=============================================================================

/**
 *  JPEG/MPEG engine fire mode
 */
typedef enum TRIGGER_MODE_TAG
{
    TRIGGER_MODE_HW,
    TRIGGER_MODE_COMMAND
} TRIGGER_MODE;

/**
 * @remark When application enable or disable these capability during preview,
 * it must call mmpVPUpdate() to active it.
 *
 * @see mmpVPEnable mmpVPDisable
 */
typedef enum MMP_VP_CAPS_TAG
{
    //Enable/disable deinterlace
    MMP_VP_DEINTERLACE,

    //Enable/disable low leve ledge deinterlace
    MMP_VP_LOWLEVELEDGE,

    //Enable/disable frame function
    MMP_VP_FRAME_FUNCTION_0,

    //Enable/disable remap address
    MMP_VP_REMAP_ADDRESS,

    //Enable/disable Interrupt
    MMP_VP_INTERRUPT,

    //Top Field Deinter
    MMP_VP_DEINTER_FIELD_TOP,

    //Bottom Field Deinter
    MMP_VP_DEINTER_FIELD_BOTTOM,

    //Scene Change
    MMP_VP_SCENECHANGE
} MMP_VP_CAPS;

// Input format
typedef enum MMP_VP_INFORMAT_TAG
{
    MMP_VP_IN_NV12,
    MMP_VP_IN_NV21
} MMP_VP_INFORMAT;

// Output format
typedef enum MMP_VP_OUTFORMAT_TAG
{
    //ISP Output Format
    MMP_VP_OUT_YUV422  = MMP_PIXEL_FORMAT_YUV422,
    MMP_VP_OUT_YUV420  = MMP_PIXEL_FORMAT_YV12,
    MMP_VP_OUT_YUV444  = 0x0101,   // avoid mapping fail
    MMP_VP_OUT_YUV422R = 0x0102,   // avoid mapping fail
    MMP_VP_OUT_NV12    = 0x0103,   // avoid mapping fail
    MMP_VP_OUT_NV21    = 0x0104    // avoid mapping fail
} MMP_VP_OUTFORMAT;

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct MMP_VP_SINGLE_SHARE_TAG
{
    //Signal Process Input Parameter
    MMP_UINT32       In_AddrY;
    MMP_UINT32       In_AddrUV;
    MMP_UINT32       In_AddrYp;
    MMP_UINT16       In_Width;
    MMP_UINT16       In_Height;
    MMP_UINT16       In_PitchY;
    MMP_UINT16       In_PitchUV;
    MMP_VP_INFORMAT  In_Format;

    //Signal Process Output Parameter
    MMP_UINT32       Out_AddrY;
    MMP_UINT32       Out_AddrU;
    MMP_UINT32       Out_AddrV;
    MMP_UINT16       Out_Width;
    MMP_UINT16       Out_Height;
    MMP_UINT16       Out_PitchY;
    MMP_UINT16       Out_PitchUV;
    MMP_VP_OUTFORMAT Out_Format;
} MMP_VP_SINGLE_SHARE;

typedef struct MMP_VP_SEQUENCE_SHARE_TAG
{
    //Input Parameter
    MMP_UINT32       In_AddrY[5];
    MMP_UINT32       In_AddrUV[5];
    MMP_UINT16       In_Width;
    MMP_UINT16       In_Height;
    MMP_UINT16       In_PitchY;
    MMP_UINT16       In_PitchUV;
    MMP_VP_INFORMAT  In_Format;
    MMP_UINT16       In_BufferNum;

    //Output Parameter
    MMP_UINT32       Out_AddrY[5];
    MMP_UINT32       Out_AddrU[5];
    MMP_UINT32       Out_AddrV[5];
    MMP_UINT16       Out_Width;
    MMP_UINT16       Out_Height;
    MMP_UINT16       Out_PitchY;
    MMP_UINT16       Out_PitchUV;
    MMP_VP_OUTFORMAT Out_Format;
    MMP_UINT16       Out_BufferNum;

    //for sequence process
    MMP_BOOL         EnCapOnflyMode;
    MMP_BOOL         EnOnflyInFieldMode;
} MMP_VP_SEQUENCE_SHARE;

typedef struct MMP_VP_REMAP_ADDR_TAG
{
    MMP_UINT16 YAddr_03;
    MMP_UINT16 YAddr_04;
    MMP_UINT16 YAddr_05;
    MMP_UINT16 YAddr_06;
    MMP_UINT16 YAddr_07;
    MMP_UINT16 YAddr_08;
    MMP_UINT16 YAddr_09;
    MMP_UINT16 YAddr_10;
    MMP_UINT16 YAddr_11;
    MMP_UINT16 YAddr_12;
    MMP_UINT16 YAddr_13;
    MMP_UINT16 YAddr_14;
    MMP_UINT16 YAddr_15;
    MMP_UINT16 YAddr_16;
    MMP_UINT16 YAddr_17;
    MMP_UINT16 YAddr_18;
    MMP_UINT16 YAddr_19;
    MMP_UINT16 YAddr_20;
    MMP_UINT16 YAddr_21;
    MMP_UINT16 YAddr_22;
    MMP_UINT16 YAddr_23;
    MMP_UINT16 YAddr_24;
    MMP_UINT16 YAddr_25;
    MMP_UINT16 YAddr_26;
    MMP_UINT16 YAddr_27;
    MMP_UINT16 YAddr_28;
    MMP_UINT16 YAddr_29;
    MMP_UINT16 YAddr_30;
    MMP_UINT16 YAddr_31;

    MMP_UINT16 UVAddr_03;
    MMP_UINT16 UVAddr_04;
    MMP_UINT16 UVAddr_05;
    MMP_UINT16 UVAddr_06;
    MMP_UINT16 UVAddr_07;
    MMP_UINT16 UVAddr_08;
    MMP_UINT16 UVAddr_09;
    MMP_UINT16 UVAddr_10;
    MMP_UINT16 UVAddr_11;
    MMP_UINT16 UVAddr_12;
    MMP_UINT16 UVAddr_13;
    MMP_UINT16 UVAddr_14;
    MMP_UINT16 UVAddr_15;
    MMP_UINT16 UVAddr_16;
    MMP_UINT16 UVAddr_17;
    MMP_UINT16 UVAddr_18;
    MMP_UINT16 UVAddr_19;
    MMP_UINT16 UVAddr_20;
    MMP_UINT16 UVAddr_21;
    MMP_UINT16 UVAddr_22;
    MMP_UINT16 UVAddr_23;
    MMP_UINT16 UVAddr_24;
    MMP_UINT16 UVAddr_25;
    MMP_UINT16 UVAddr_26;
    MMP_UINT16 UVAddr_27;
    MMP_UINT16 UVAddr_28;
    MMP_UINT16 UVAddr_29;
    MMP_UINT16 UVAddr_30;
    MMP_UINT16 UVAddr_31;
} MMP_VP_REMAP_ADDR;

typedef struct MMP_VP_COLOR_CTRL_TAG
{
    MMP_INT32 brightness;
    MMP_FLOAT contrast;
    MMP_INT32 hue;
    MMP_FLOAT saturation;
} MMP_VP_COLOR_CTRL;

typedef struct MMP_VP_MOTION_PARAM_TAG
{
    MMP_UINT16 MDThreshold_High;
    MMP_UINT16 MDThreshold_Low;
    MMP_BOOL   EnLPFWeight;
    MMP_BOOL   EnLPFStaticPixel;
    MMP_BOOL   DisableMV_A;
    MMP_BOOL   DisableMV_B;
    MMP_BOOL   DisableMV_C;
    MMP_BOOL   DisableMV_D;
    MMP_BOOL   DisableMV_E;
    MMP_BOOL   DisableMV_F;
    MMP_BOOL   DisableMV_G;
} MMP_VP_MOTION_PARAM;

typedef void (*VP_handler)(void *arg);

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
 * ISP initialization.
 *
 * @return VP_SUCCESS if succeed, error codes of VP_ERR_ERROR otherwise.
 *
 * @remark Application must call this API first when it want to use ISP API.
 *
 * @see MMP_VP_MODE
 */
//=============================================================================
VP_API MMP_RESULT
mmpVPInitialize(
    void);

//=============================================================================
/**
 * ISP terminate.
 *
 * @return VP_SUCCESS if succeed, error codes of VP_ERR_ERROR otherwise.
 *
 * @remark Application must call this API when leaving ISP module.
 */
//=============================================================================
VP_API MMP_RESULT
mmpVPTerminate(
    void);

//=============================================================================
/**
 * ISP context reset.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 *
 * @remark Application must call this API first when it want to use ISP API.
 */
//=============================================================================
VP_API MMP_RESULT
mmpVPContextReset(
    void);

//=============================================================================
/**
 * Enable ISP capability.
 *
 * @param  cap  Specifies a symbolic constant indicating a ISP capability.
 * @return VP_SUCCESS if succeed, error codes of VP_ERR_ERROR otherwise.
 *
 * @see MMP_VP_ATTRIBUTE mmpVPDisable
 */
//=============================================================================
VP_API MMP_RESULT
mmpVPEnable(
    MMP_VP_CAPS cap);

//=============================================================================
/**
 * Disable ISP capability.
 *
 * @param  cap  Specifies a symbolic constant indicating a ISP capability.
 * @return VP_SUCCESS if succeed, error codes of VP_ERR_ERROR otherwise.
 *
 * @see MMP_VP_ATTRIBUTE mmpVPEnable
 */
//=============================================================================
VP_API MMP_RESULT
mmpVPDisable(
    MMP_VP_CAPS cap);

//=============================================================================
/**
 * Query ISP capability.
 *
 * @param cap       Specifies a symbolic constant indicating a ISP capability.
 * @return MMP_TRUE if function enabled, MMP_FALSE if function disable.
 */
//=============================================================================
VP_API MMP_BOOL
mmpVPQuery(
    MMP_VP_CAPS cap);

//=============================================================================
/**
 * Set Remap Address Parameter
 * @param MMP_VP_REMAP_ADDR
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpVPSetRemapAddr(
    MMP_VP_REMAP_ADDR *data);

//=============================================================================
/**
 * ISP Motion Detection Process
 *
 * @param data  MMP_VP_SINGLE_SHARE
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpVPMotionProcess(
    const MMP_VP_SINGLE_SHARE *data);

//=============================================================================
/**
 * ISP Sequence Process.
 * @param data MMP_VP_SEQUENCE_SHARE
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
VP_API MMP_RESULT
mmpVPSingleProcess(
    const MMP_VP_SINGLE_SHARE *data);

//=============================================================================
/**
 * Set Sequence Output Parameter
 * @param MMP_VP_SEQUENCE_SHARE
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
VP_API MMP_RESULT
mmpVPSequenceProcess(
    const MMP_VP_SEQUENCE_SHARE *data);

//=============================================================================
/**
 * Set Sequence Output Parameter
 * @param MMP_VP_SEQUENCE_SHARE
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
VP_API MMP_RESULT
mmpVPSetSequenceOutputInfo(
    const MMP_VP_SEQUENCE_SHARE *data);

//=============================================================================
/**
 * Set frame function background image information & color key.  (For Direct
 * Assign VRAM address. Ex.2D input)
 *
 * @param baseAddr      base address of the background image buffer.
 * @param startX        x position of the background image.
 * @param startY        y position of the background image.
 * @param width         width of the background image.
 * @param height        height of the background image.
 * @param colorKeyR     color key for R channel.
 * @param colorKeyG     color key for G channel.
 * @param colorKeyB     color key for B channel.
 * @param constantAlpha constant Alpha Value.
 * @param format        format of the picture & color key. only support RGB 888, RGB565
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 *
 * @see mmpVPEnable() mmpVPDisable()
 */
//=============================================================================
VP_API MMP_RESULT
mmpVPSetFrameFunction(
    void             *vramAddr,
    MMP_UINT         startX,
    MMP_UINT         startY,
    MMP_UINT         width,
    MMP_UINT         height,
    MMP_UINT         pitch,
    MMP_UINT         colorKeyR,
    MMP_UINT         colorKeyG,
    MMP_UINT         colorKeyB,
    MMP_UINT         constantAlpha,
    MMP_PIXEL_FORMAT format);

//=============================================================================
/**
 * Wait ISP Engine Idle
 * @return VP_SUCCESS if succeed, error codes of VP_ERR_ERROR otherwise.
 */
//=============================================================================
VP_API MMP_RESULT
mmpVPWaitEngineIdle(
    void);

//=============================================================================
/**
 * Is ISP Engine Idle
 */
//=============================================================================
MMP_BOOL
mmpVPIsEngineIdle(
    void);

//=============================================================================
/**
 * Wait ISP Interrupt Idle
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
VP_API MMP_RESULT
mmpVPWaitInterruptIdle(
    void);

//=============================================================================
/**
 * Clear ISP Interrupt
 */
//=============================================================================
VP_API void
mmpVPClearInterrupt(
    void);

//=============================================================================
/**
 * ISP Write Buffer Index
 * @return index number
 */
//=============================================================================
VP_API MMP_UINT16
mmpVPReturnWrBufIndex(
    void);

//=============================================================================
/**
 * ISP Register IRQ
 */
//=============================================================================
VP_API void
mmpVPRegisterIRQ(
    VP_handler isphandler);

//=============================================================================
/**
 * Disable ISP Interrupt.
 */
//=============================================================================
VP_API MMP_RESULT
mmpVPDisableInterrupt(
    void);

//=============================================================================
/**
 * mmpVPResetEngine
 */
//=============================================================================
VP_API MMP_RESULT
mmpVPResetEngine(
    void);

//=============================================================================
/**
 * Set color control value.
 */
//=============================================================================
VP_API void
mmpVPSetColorCtrl(
    const MMP_VP_COLOR_CTRL *data);

//=============================================================================
/**
 * Get color control value.
 */
//=============================================================================
VP_API void
mmpVPGetColorCtrl(
    MMP_VP_COLOR_CTRL *data);

//=============================================================================
/**
 * Update Color Matrix.
 */
//=============================================================================
VP_API void
mmpVPOnflyUpdateColorMatrix(
    void);

//=============================================================================
/**
 * Set color control value.
 */
//=============================================================================
VP_API MMP_UINT16
mmpVPSceneChgTotalDiff(
    void);

//=============================================================================
/**
 * Set Motion Parameter
 * @param MMP_VP_MOTION_PARAM
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
VP_API MMP_RESULT
mmpVPSetMotionParameter(
    const MMP_VP_MOTION_PARAM *data);

//=============================================================================
/**
 * VP Power Up.
 */
//=============================================================================
VP_API void
mmpVPPowerUp(
    void);

//=============================================================================
/**
 * VP Power Down
 */
//=============================================================================
VP_API void
mmpVPPowerDown(
    void);
#ifdef __cplusplus
}
#endif

#endif