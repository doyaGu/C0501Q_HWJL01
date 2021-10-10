#ifndef __CAP_HW_H__
#define __CAP_HW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "capture/capture_types.h"
#include "capture/mmp_capture.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define B_CAP_INPUT_INTERLEVING    (1 << 9)
#define B_CAP_SYNC_MODE_BT_656     (1 << 1)
#define B_CAP_ONFLY_MODE           (1 << 3)

#define B_CAP_ENABLE_INT           (1 << 15)
#define M_CAP_ENABLE_INT           0x8000

/* color bar */
#define B_CAP_COLOR_BAR_ENABLE     (1 << 15)
#define B_CAP_COLOR_BAR_VSPOL      (1 << 13)
#define B_CAP_COLOR_BAR_HSPOL      (1 << 12)

/* input data format */
#define CAP_IN_DATA_FORMAT_YUV422  (1 << 12)

#define CAP_IN_YUV444              0
#define CAP_IN_YUV422              1
#define CAP_PROGRESSIVE            0
#define CAP_INTERLEVING            1
#define CAP_BT_601                 0
#define CAP_BT_656                 1

#define CAP_ONE_CHANNEL_MODE       0
#define CAP_TWO_CHANNEL_MODE       1
#define CAP_CHANNEL_MODE           (1 << 13)
#define CAP_IN_FORMAT_MASK         3

#define CAP_PRELOADNUM_MASK        3

#define CAP_ERRLD                  (0x7 << 8)
#define CAP_ONCEFULL               (0x1 << 3)
#define CAP_HALTFORFIRE            (0x1 << 2)
#define CAP_SIZEERR                (0x1 << 1)
#define CAP_ENCSFUN                (0x1 << 15)

/* Capture interrupt mode */
#define CAP_INT_MODE_ERR           0
#define CAP_INT_MODE_ERR_FRAMEEND  1
#define CAP_INT_MODE_MULTI_CHANNEL 2
#define CAP_INT_MODE_MASK          3

#define CAP_BIT_SCALEWEIGHT        0x00FF           // 0000 0000 1111 1111

#define CAP_SHT_SCALEWEIGHT_H      8
#define CAP_SHT_SCALEWEIGHT_L      0

#define CAP_BIT_RGB_TO_YUV         0x07FF           // 0000 0111 1111 1111
#define CAP_BIT_RGB_TO_YUV_CONST   0x03FF           // 0000 0011 1111 1111

#define MMP_CAP_CLOCK_REG_62       0x0062
#define MMP_CAP_CLOCK_REG_64       0x0064
#define MMP_CAP_EN_DIV_CAPCLK      0x0800           // [11]
#define MMP_CAP_RESET              0x0008           // [ 3]
#define MMP_CAP_REG_RESET          0x0004           // [ 2]
#define MMP_CAP_EN_M17CLK          0x0001           // [ 0] memory clock in Capture

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
Cap_Fire(
    void);

void
Cap_UnFire(
    void);

void
Cap_TurnOnClock_Reg(
    MMP_BOOL flag);

MMP_RESULT
Cap_Set_Input_Pin_Mux_Reg(
    CAP_INPUT_MUX_INFO *pininfo);

void
Cap_Set_Output_Pin_Mux_Reg(
    CAP_OUTPUT_PIN_SELECT *pininfo);

MMP_RESULT
Cap_Set_Output_Clk_Mode_Reg(
    MMP_UINT16 value);

void
Cap_Set_Color_Format_Reg(
    CAP_YUV_INFO *pYUVinfo);

MMP_RESULT
Cap_Set_IO_Mode_Reg(
    CAP_IO_MODE_INFO *io_config);

void
Cap_Set_Input_Data_Info_Reg(
    CAP_INPUT_INFO *pIninfo);

void
Cap_Set_HorScale_Width_Reg(
    CAP_OUTPUT_INFO *pOutInfo);

void
Cap_Set_Skip_Pattern_Reg(
    MMP_UINT16 pattern,
    MMP_UINT16 period);

MMP_BOOL
IsCapFire(
    void);

MMP_RESULT
Cap_WaitEngineIdle(
    void);

MMP_UINT16 Cap_Get_Lane_status(
    CAP_LANE_STATUS lanenum);

MMP_UINT16
Cap_Get_Hsync_Polarity(
    void);

MMP_UINT16
Cap_Get_Vsync_Polarity(
    void);

MMP_UINT16
Cap_Get_Hsync_Polarity_Status(
    void);

MMP_UINT16
Cap_Get_Vsync_Polarity_Status(
    void);

void
Cap_Engine_Reset(
    void);

void
Cap_Engine_Register_Reset(
    void);

void
Cap_Set_Enable_Interrupt(
    MMP_BOOL flag);

void
Cap_Set_Interrupt_Mode(
    MMP_UINT8 Intr_Mode);

void
Cap_Set_Color_Bar(
    CAP_COLOR_BAR_CONFIG color_info);

MMP_RESULT
Cap_Get_BT601_SyncCnt(
    void);

MMP_RESULT
Cap_Get_Detected_Region(
    void);

MMP_UINT16
Cap_Get_Sync_Status(
    void);

MMP_UINT16
Cap_Get_Error_Status(
    void);

void Cap_Dump_Reg(
    void);

MMP_UINT16
Cap_Clean_Intr(
    void);

void
Cap_Set_EnableFrameRate_Reg(
    void);

void Cap_Set_Enable_Reg(
    CAP_ENFUN_INFO *pFunEn);

MMP_RESULT
Cap_Set_ISP_HandShaking(
    CAP_ISP_HANDSHAKING_MODE mode, MMP_UINT8 preloadnum);

MMP_RESULT
Cap_Set_Error_Handleing(
    MMP_UINT16 errDetectEn);

MMP_RESULT
Cap_Set_Wait_Error_Reset(
    void);

void
Cap_Set_FrameBase_0_Reg(
    MMP_UINT32 dst_Yaddr, MMP_UINT32 dst_UVaddr);

void
Cap_Set_FrameBase_1_Reg(
    MMP_UINT32 dst_Yaddr, MMP_UINT32 dst_UVaddr);

void
Cap_Set_FrameBase_2_Reg(
    MMP_UINT32 dst_Yaddr, MMP_UINT32 dst_UVaddr);

void
Cap_Set_FrameBase_3_Reg(
    MMP_UINT32 dst_Yaddr, MMP_UINT32 dst_UVaddr);

void
Cap_Set_FrameBase_4_Reg(
    MMP_UINT32 dst_Yaddr, MMP_UINT32 dst_UVaddr);

void
Cap_Set_Buffer_addr_Reg (
    MMP_UINT32 *pAddr, MMP_UINT32 addrOffset);

void
CAP_SetScaleParam_Reg(
    const CAP_SCALE_CTRL *pScaleFun);

void
    Cap_SetIntScaleMatrixH_Reg(
    MMP_UINT16 WeightMatX[][CAP_SCALE_TAP]);

//=============================================================================
/**
 * RGB to YUV transfer matrix.
 */
//=============================================================================
void
Cap_SetRGBtoYUVMatrix_Reg(
    const CAP_RGB_TO_YUV *pRGBtoYUV);

//=============================================================================
/**
 * Set color correction matrix and constant.
 */
//=============================================================================
void
Cap_SetCCMatrix_Reg(
    const CAP_COLOR_CORRECTION *pColorCorrect);

//=============================================================================
/**
 * Audio/Video/Mute Counter control function
 */
//=============================================================================
void
AVSync_CounterCtrl(
    AV_SYNC_COUNTER_CTRL mode, MMP_UINT16 divider);

void
AVSync_CounterReset(
    AV_SYNC_COUNTER_CTRL mode);

MMP_UINT32
AVSync_CounterLatch(
    AV_SYNC_COUNTER_CTRL mode);

MMP_UINT32
AVSync_CounterRead(
    AV_SYNC_COUNTER_CTRL mode);

MMP_BOOL
AVSync_MuteDetect(
    void);

void
Cap_Set_Output_Driving_Strength_Reg(
    MMP_UINT16 driving);

void
Cap_EnableClock(
    void);

void
Cap_DisableClock(
    void);

void
Cap_Reset(
    void);

void
Cap_Reg_Reset(
    void);

#ifdef __cplusplus
}
#endif

#endif
