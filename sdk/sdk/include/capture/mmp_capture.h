#ifndef __MMP_CAPTURE_H__
#define __MMP_CAPTURE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "encoder/encoder_types.h"
#include "ite/ith.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#if defined(WIN32)
    #if defined(ISP_EXPORTS)
        #define CAP_API __declspec(dllexport)
    #else
        #define CAP_API __declspec(dllimport)
    #endif
#else
    #define CAP_API     extern
#endif //#if defined(WIN32)

//=============================================================================
//                              Constant Definition
//=============================================================================
//#if defined (RESOLUTION_1080P)
//    #define CAP_MEM_BUF_PITCH    1920
//    #define CAP_INPUT_MAX_HEIGHT 1080
//#elif defined (RESOLUTION_720P)
    #define CAP_MEM_BUF_PITCH    1280
    #define CAP_INPUT_MAX_HEIGHT 720
//#else
//    #define CAP_MEM_BUF_PITCH    720
//    #define CAP_INPUT_MAX_HEIGHT 480
//#endif

#define CAP_MEM_BUF_HEIGHT       (CAP_INPUT_MAX_HEIGHT * 3 + (CAP_INPUT_MAX_HEIGHT >> 1) * 3) //CAP_MEM_MAX_HEIGHT*3 + (CAP_MEM_MAX_HEIGHT / 2)*3
#define CAP_HDMI_INPUT_VESA      14
/**
 *  Device Select
 */
typedef enum MMP_CAP_DEVICE_ID_TAG
{
    MMP_CAP_UNKNOW_DEVICE = 0,
    MMP_CAP_DEV_ADV7180,
    MMP_CAP_DEV_CAT9883,
    MMP_CAP_DEV_HDMIRX,
    MMP_CAP_DEV_SENSOR,
} MMP_CAP_DEVICE_ID;

typedef enum MMP_CAP_SKIP_MODE_TAG
{
    MMP_CAPTURE_NO_DROP = 0,
    MMP_CAPTURE_SKIP_BY_TWO,
    MMP_CAPTURE_SKIP_BY_THREE,
    MMP_CAPTURE_SKIP_30FPS_TO_25FPS,
    MMP_CAPTURE_SKIP_60FPS_TO_25FPS,
} MMP_CAP_SKIP_MODE;

typedef enum MMP_CAP_FUN_FLAG_TAG
{
    //Enable/Disable Interrupt
    MMP_CAP_INTERRUPT = 0,

    //Enable/Disable Capture Onfly Mode
    MMP_CAP_ONFLY_MODE,
} MMP_CAP_FUN_FLAG;

typedef enum MMP_CAP_LANE_STATUS_TAG
{
    MMP_CAP_LANE0_STATUS,
    MMP_CAP_LANE1_STATUS,
    MMP_CAP_LANE2_STATUS,
    MMP_CAP_LANE3_STATUS
} MMP_CAP_LANE_STATUS;

typedef enum MMP_CAP_ERROR_CODE_TAG
{
    MMP_HSYNC_ERR = 0x1,        //0x01
    MMP_VSYNC_ERR,              //0x02
    MMP_SIZE_ERROR_ERR,         //0x03
    MMP_OVERFLOW_ERR,           //0x04
    MMP_ABNORMAL_TERMINATE_ERR, //0x05
    MMP_TIMEOUT_ERR,            //0x06
    MMP_SCDTO_ERR,              //0x07
    MMP_BT656_SYNC_ERR,         //0x08
    MMP_BT656_PREAMBLE_ERR,     //0x09
    MMP_MEM_LAST_ERR,           //0x10
    MMP_FRAME_SYNC_POINT_ERR,   //0x11
    MMP_FRAME_SYNC_ACTIVE_ERR,  //0x12
    MMP_FIRE_ISP_ERR,           //0x13
    MMP_INTERNAL_DATAPTH_ERR    //0x14
} MMP_CAP_ERROR_CODE;

typedef enum MMP_CAP_FRAMERATE_TAG
{
    MMP_CAP_FRAMERATE_UNKNOW = 0,
    MMP_CAP_FRAMERATE_25HZ,
    MMP_CAP_FRAMERATE_50HZ,
    MMP_CAP_FRAMERATE_30HZ,
    MMP_CAP_FRAMERATE_60HZ,
    MMP_CAP_FRAMERATE_29_97HZ,
    MMP_CAP_FRAMERATE_59_94HZ,
    MMP_CAP_FRAMERATE_23_97HZ,
    MMP_CAP_FRAMERATE_24HZ,
    MMP_CAP_FRAMERATE_56HZ,
    MMP_CAP_FRAMERATE_70HZ,
    MMP_CAP_FRAMERATE_72HZ,
    MMP_CAP_FRAMERATE_75HZ,
    MMP_CAP_FRAMERATE_85HZ,
    MMP_CAP_FRAMERATE_VESA_30HZ,
    MMP_CAP_FRAMERATE_VESA_60HZ
} MMP_CAP_FRAMERATE;

typedef enum MMP_CAP_VIDEO_SOURCE_TAG
{
    MMP_CAP_VIDEO_SOURCE_UNKNOW = 0,
    MMP_CAP_VIDEO_SOURCE_CVBS,
    MMP_CAP_VIDEO_SOURCE_SVIDEO,
    MMP_CAP_VIDEO_SOURCE_YPBPR,
    MMP_CAP_VIDEO_SOURCE_VGA,
    MMP_CAP_VIDEO_SOURCE_HDMI,
    MMP_CAP_VIDEO_SOURCE_SENSOR
} MMP_CAP_VIDEO_SOURCE;

typedef enum MMP_CAP_INPUT_INFO_TAG
{
    MMP_CAP_INPUT_INFO_640X480_60P   = 0,
    MMP_CAP_INPUT_INFO_720X480_59I   = 1,
    MMP_CAP_INPUT_INFO_720X480_59P   = 2,
    MMP_CAP_INPUT_INFO_720X480_60I   = 3,
    MMP_CAP_INPUT_INFO_720X480_60P   = 4,
    MMP_CAP_INPUT_INFO_720X576_50I   = 5,
    MMP_CAP_INPUT_INFO_720X576_50P   = 6,
    MMP_CAP_INPUT_INFO_1280X720_50P  = 7,
    MMP_CAP_INPUT_INFO_1280X720_59P  = 8,
    MMP_CAP_INPUT_INFO_1280X720_60P  = 9,
    MMP_CAP_INPUT_INFO_1920X1080_23P = 10,
    MMP_CAP_INPUT_INFO_1920X1080_24P = 11,
    MMP_CAP_INPUT_INFO_1920X1080_25P = 12,
    MMP_CAP_INPUT_INFO_1920X1080_29P = 13,
    MMP_CAP_INPUT_INFO_1920X1080_30P = 14,
    MMP_CAP_INPUT_INFO_1920X1080_50I = 15,
    MMP_CAP_INPUT_INFO_1920X1080_50P = 16,
    MMP_CAP_INPUT_INFO_1920X1080_59I = 17,
    MMP_CAP_INPUT_INFO_1920X1080_59P = 18,
    MMP_CAP_INPUT_INFO_1920X1080_60I = 19,
    MMP_CAP_INPUT_INFO_1920X1080_60P = 20,
    MMP_CAP_INPUT_INFO_800X600_60P   = 21,
    MMP_CAP_INPUT_INFO_1024X768_60P  = 22,
    MMP_CAP_INPUT_INFO_1280X768_60P  = 23,
    MMP_CAP_INPUT_INFO_1280X800_60P  = 24,
    MMP_CAP_INPUT_INFO_1280X960_60P  = 25,
    MMP_CAP_INPUT_INFO_1280X1024_60P = 26,
    MMP_CAP_INPUT_INFO_1360X768_60P  = 27,
    MMP_CAP_INPUT_INFO_1366X768_60P  = 28,
    MMP_CAP_INPUT_INFO_1440X900_60P  = 29,
    MMP_CAP_INPUT_INFO_1400X1050_60P = 30,
    MMP_CAP_INPUT_INFO_1440X1050_60P = 31,
    MMP_CAP_INPUT_INFO_1600X900_60P  = 32,
    MMP_CAP_INPUT_INFO_1600X1200_60P = 33,
    MMP_CAP_INPUT_INFO_1680X1050_60P = 34,
    MMP_CAP_INPUT_INFO_ALL           = 35,
    MMP_CAP_INPUT_INFO_NUM           = 36,
    MMP_CAP_INPUT_INFO_UNKNOWN       = 37,
    MMP_CAP_INPUT_INFO_CAMERA        = 38,
} MMP_CAP_INPUT_INFO;

typedef MMP_UINT32 AV_SYNC_COUNTER_CTRL;

#define AUDIO_COUNTER_CLEAR 0x0001
#define VIDEO_COUNTER_CLEAR 0x0002
#define AUDIO_COUNTER_LATCH 0x0004
#define VIDEO_COUNTER_LATCH 0x0004
#define MUTE_COUNTER_LATCH  0x0008
#define MUTE_COUNTER_CLEAR  0x0010
#define AUDIO_COUNTER_SEL   0x0020
#define VIDEO_COUNTER_SEL   0x0040
#define MUTE_COUNTER_SEL    0x0080
#define I2S_SOURCE_SEL      0x0100
#define SPDIF_SOURCE_SEL    0x0200

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct MMP_CAP_COLOR_CTRL_TAG
{
    MMP_INT32 brightness;
    MMP_FLOAT contrast;
    MMP_INT32 hue;
    MMP_FLOAT saturation;
} MMP_CAP_COLOR_CTRL;

typedef struct MMP_CAP_SHARE_TAG
{
    //Input Parameter
    MMP_UINT16        Width;
    MMP_UINT16        Height;
    MMP_BOOL          HSyncPol;
    MMP_BOOL          VSyncPol;
    MMP_BOOL          IsInterlaced;
    MMP_CAP_FRAMERATE FrameRate;
    MMP_UINT16        OutWidth;
    MMP_UINT16        OutHeight;
    MMP_UINT32        OutAddrY[3];
    MMP_UINT32        OutAddrUV[3];
    MMP_UINT32        OutAddrOffset;
    MMP_BOOL          bMatchResolution;
} MMP_CAP_SHARE;

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
 * Cap Select Device.
 */
//=============================================================================
CAP_API MMP_RESULT
mmpCapSetCaptureDevice(
    MMP_CAP_DEVICE_ID id);

//=============================================================================
/**
 * Cap Get Device.
 */
//=============================================================================
CAP_API MMP_CAP_DEVICE_ID
mmpCapGetCaptureDevice(
    void);

//=============================================================================
/**
 * Cap Auto Detect Device.
 */
//=============================================================================
MMP_CAP_DEVICE_ID
mmpCapAutoDetectDevice(
    void);

//=============================================================================
/**
 * Cap Device Tri-State.
 */
//=============================================================================
void
mmpCapDeviceAllDeviceTriState(
    void);

//=============================================================================
/**
 * Cap Device initialization.
 */
//=============================================================================
CAP_API MMP_RESULT
mmpCapDeviceInitialize(
    void);

//=============================================================================
/**
 * Cap Device terminate.
 */
//=============================================================================
void
mmpCapDeviceTerminate(
    void);

//=============================================================================
/**
 * Cap Device Signal State.
 */
//=============================================================================
CAP_API MMP_BOOL
mmpCapDeviceIsSignalStable(
    void);

//=============================================================================
/**
 * Cap Get Device Signal State.
 */
//=============================================================================
CAP_API MMP_BOOL
mmpCapGetDeviceIsSignalStable(
    MMP_CAP_DEVICE_ID id);

//=============================================================================
/**
 * mmpCapGetDeviceInfo
 */
//=============================================================================
CAP_API void
mmpCapGetDeviceInfo(
    MMP_CAP_SHARE   *data);

//=============================================================================
/**
 * Device reboot.
 */
//=============================================================================
CAP_API MMP_RESULT
mmpCapSetDeviceReboot(
    MMP_BOOL flag);

CAP_API MMP_BOOL
mmpCapGetDeviceReboot(
    void);

//=============================================================================
/**
 * Capture Resolution.
 */
//=============================================================================
CAP_API MMP_UINT16
mmpCapGetResolutionIndex(
    MMP_CAP_DEVICE_ID id);

//=============================================================================
/**
 * Capture VESA Timing Check.
 */
//=============================================================================
CAP_API MMP_BOOL
mmpCapVESATimingCheck(
    MMP_UINT16 width,
    MMP_UINT16 height,
    MMP_CAP_INPUT_INFO *info);

//=============================================================================
/**
 * Cap context initialization.
 */
//=============================================================================
CAP_API MMP_RESULT
mmpCapInitialize(
    void);

//=============================================================================
/**
 * Cap terminate.
 */
//=============================================================================
CAP_API MMP_RESULT
mmpCapTerminate(
    void);

//=============================================================================
/**
 * mmpCapIsFire
 */
//=============================================================================
CAP_API MMP_BOOL
mmpCapIsFire(
    void);

//=============================================================================
/**
 * mmpCapGetEngineErrorStatus
 */
//=============================================================================
CAP_API MMP_UINT16
mmpCapGetEngineErrorStatus(
    MMP_CAP_LANE_STATUS lanenum);

//=============================================================================
/**
 * mmpCapOnflyResetAllEngine
 */
//=============================================================================
CAP_API MMP_RESULT
mmpCapOnflyResetAllEngine(
    void);

//=============================================================================
/**
 * mmpCapResetEngine
 */
//=============================================================================
MMP_RESULT
mmpCapResetEngine(
    void);

//=============================================================================
/**
 * mmpCapProcess
 */
//=============================================================================
CAP_API MMP_RESULT
mmpCapParameterSetting(
    const MMP_CAP_SHARE *data);

//=============================================================================
/**
 * mmpCapFire
 */
//=============================================================================
CAP_API void
mmpCapFire(
    void);

//=============================================================================
/**
 * mmpCapMemoryInitialize
 */
//=============================================================================
CAP_API MMP_RESULT
mmpCapMemoryInitialize(
    void);

//=============================================================================
/**
 * mmpCapEnableInterrupt
 */
//=============================================================================
CAP_API void
mmpCapRegisterIRQ(
    ITHIntrHandler caphandler);

//=============================================================================
/**
 * mmpCapDisableIRQ
 */
//=============================================================================
CAP_API void
mmpCapDisableIRQ(
    void);

//=============================================================================
/**
 * mmpCapEnableInterrupt
 */
//=============================================================================
CAP_API void
mmpCapEnableInterrupt(
    MMP_BOOL flag);

//=============================================================================
/**
 * mmpCapClearInterrupt
 */
//=============================================================================
CAP_API MMP_UINT16
mmpCapClearInterrupt(
    void);

//=============================================================================
/**
 * Cap Write Buffer Index
 * @return index number
 */
//=============================================================================
CAP_API MMP_UINT16
mmpCapReturnWrBufIndex(
    void);

//=============================================================================
/**
 * mmpCapGetInputFrameRate
 */
//=============================================================================
CAP_API MMP_CAP_FRAMERATE
mmpCapGetInputFrameRate(
    void);

//=============================================================================
/**
 * mmpCapSetPolarity
 */
//=============================================================================
CAP_API void
mmpCapSetPolarity (
    MMP_UINT16 HPolarity,
    MMP_UINT16 VPolarity);

//=============================================================================
/**
 * mmpAVSyncCounterInit
 */
//=============================================================================
CAP_API void
mmpAVSyncCounterCtrl(
    AV_SYNC_COUNTER_CTRL mode, MMP_UINT16 divider);

//=============================================================================
/**
 * mmpAVSyncCounterReset
 */
//=============================================================================
CAP_API void
mmpAVSyncCounterReset(
    AV_SYNC_COUNTER_CTRL mode);

//=============================================================================
/**
 * mmpAVSyncCounterLatch
 */
//=============================================================================
CAP_API MMP_UINT32
mmpAVSyncCounterLatch(
    AV_SYNC_COUNTER_CTRL cntSel);

//=============================================================================
/**
 * mmpAVSyncCounterRead
 */
//=============================================================================
CAP_API MMP_UINT32
mmpAVSyncCounterRead(
    AV_SYNC_COUNTER_CTRL mode);

//=============================================================================
/**
 * mmpAVSyncMuteDetect
 */
//=============================================================================
CAP_API MMP_BOOL
mmpAVSyncMuteDetect(
    void);

//=============================================================================
/**
 * mmpCapSetSkipPattern
 */
//=============================================================================
CAP_API MMP_RESULT
mmpCapSetSkipMode(
    MMP_CAP_SKIP_MODE mode);

//=============================================================================
/**
 * mmpCapIsOnflyMode
 */
//=============================================================================
CAP_API MMP_BOOL
mmpCapIsOnflyMode(
    void);

//=============================================================================
/**
 * mmpCapTurnOnClock
 */
//=============================================================================
CAP_API MMP_BOOL
mmpCapTurnOnClock(
    MMP_BOOL flag);

//=============================================================================
/**
 * mmpCapGetOutputFrameRate
 */
//
CAP_API MMP_CAP_FRAMERATE
mmpCapGetOutputFrameRate(
    MMP_UINT32 *FrameRated);

//=============================================================================
/**
 * mmpCapGetInputSrcInfo
 */
//=============================================================================
CAP_API MMP_CAP_INPUT_INFO
mmpCapGetInputSrcInfo(
    void);

//=============================================================================
/**
 * Set color control value.
 */
//=============================================================================
CAP_API void
mmpCapSetColorCtrl(
    const MMP_CAP_COLOR_CTRL     *data);

//=============================================================================
/**
 * Get color control value.
 */
//=============================================================================
CAP_API void
mmpCapGetColorCtrl(
    MMP_CAP_COLOR_CTRL     *data);

//=============================================================================
/**
 * Update Color Matrix.
 */
//=============================================================================
CAP_API void
mmpCapUpdateColorMatrix(
    void);

//=============================================================================
/**
 * Capture Power Up.
 */
//=============================================================================
void
mmpCapPowerUp(
    void);

//=============================================================================
/**
 * Capture Power Down.
 */
//=============================================================================
void
mmpCapPowerDown(
    void);
#ifdef __cplusplus
}
#endif

#endif