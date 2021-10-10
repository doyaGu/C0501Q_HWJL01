/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file video_decoder.h
 * Used as video stream decode header.
 *
 * @author Vincent Lee
 * @version 0.1
 */

#ifndef _VIDEO_DECODER_H_
#define _VIDEO_DECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

#include "core_config.h"
#include "mmp_mpg2.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define MPG2_START_CODE                             0x000001

// ISO/IEC 13818-2 section 6.2.1 table 6-1
// start_code
#define MPG2_PICTURE_START_CODE                     0x100

#define MPG2_SLICE_START_CODE_FIRST                 0x101
#define MPG2_SLICE_START_CODE_LAST                  0x1AF

#define MPG2_USER_DATA_START_CODE                   0x1B2
#define MPG2_SEQUENCE_HEADER_CODE                   0x1B3
#define MPG2_SEQUENCE_ERROR_CODE                    0x1B4
#define MPG2_EXTENSION_START_CODE                   0x1B5
#define MPG2_SEQUENCE_END_CODE                      0x1B7
#define MPG2_GROUP_START_CODE                       0x1B8

// ISO/IEC 13818-2 section 6.3.9 table 6-12
// picture_coding_type
#define MPG2_I_FRAME                                1
#define MPG2_P_FRAME                                2
#define MPG2_B_FRAME                                3
#define MPG2_D_FRAME                                4

// ISO/IEC 13818-2 section 6.3.10 table 6-14
// picture_structure
#define MPG2_TOP_FIELD                              1
#define MPG2_BOTTOM_FIELD                           2
#define MPG2_FRAME_PICTURE                          3

// ISO/IEC 13818-2 section 6.3.1 table 6-2
// extension_start_code_identifier codes
#define MPG2_SEQUENCE_EXTENSION_ID                  1
#define MPG2_SEQUENCE_DISPLAY_EXTENSION_ID          2
#define MPG2_QUANT_MATRIX_EXTENSION_ID              3
#define MPG2_COPYRIGHT_EXTENSION_ID                 4
#define MPG2_SEQUENCE_SCALABLE_EXTENSION_ID         5
#define MPG2_PICTURE_DISPLAY_EXTENSION_ID           7
#define MPG2_PICTURE_CODING_EXTENSION_ID            8
#define MPG2_PICTURE_SPATIAL_SCALABLE_EXTENSION_ID  9
#define MPG2_PICTURE_TEMPORAL_SCALABLE_EXTENSION_ID 10

// video decoder return state
typedef enum VIDEO_RETURN_ERR_TAG
{
    VIDEO_RETURN_NORMAL                 = (1 << 0),
    VIDEO_RETURN_END_OF_PACKET          = (1 << 1),
    VIDEO_RETURN_RESOLUTION_NOT_SUPPORT = (1 << 2),
    VIDEO_RETURN_SKIP_DECODE            = (1 << 3),
    VIDEO_RETURN_UNEXPECTED_START_CODE  = (1 << 4),
    VIDEO_RETURN_WAIT_FIRE              = (1 << 5),
    VIDEO_RETURN_OUT_REF_FRAME_RANGE    = (1 << 6),
    VIDEO_RETURN_TOOL_NOT_SUPPORT       = (1 << 7),
    VIDEO_RETURN_GET_DEC_HEADER         = (1 << 8),
    VIDEO_RETURN_SKIP_DISPLAY_ONCE      = (1 << 9),
    VIDEO_RETURN_FILL_BLANK             = (1 << 10),
    VIDEO_RETURN_SKIP_PACKET        =  (1 << 11),

    VIDEO_RETURN_UNKNOW                 = (1 << 31),
} VIDEO_RETURN_ERR;

typedef enum VIDEO_FLIP_TYPE_TAG
{
    VIDEO_FRAME = 0,
    BLANK_FRAME,
    LAST_VIDEO_FRAME,
} VIDEO_FLIP_TYPE;

typedef enum CUT_SOURCE_VIDEO_TAG
{
    CUTVIDEO_NONE = 0,
    CUTVIDEO_ZOOMIN,
    CUTVIDEO_HEIGHT_4_3_TO_16_9, // modify height to fit 16:9
    CUTVIDEO_HEIGHT_4_3_TO_14_9, // modify height to fit 14:9
    CUTVIDEO_WIDTH_16_9_TO_4_3,  // modify width to fit 4:3
    CUTVIDEO_WIDTH_16_9_TO_14_9, // modify width to fit 14:9
    CUTVIDEO_HEIGHT_4_3_TO_14_9_WIDTH_14_9_TO_4_3,
    CUTVIDEO_HEIGHT_16_9_TO_14_9_WIDTH_14_9_TO_4_3,
    CUTVIDEO_WIDTH_16_9_TO_4_3_HEIGHT_4_3_TO_16_9
} CUT_SOURCE_VIDEO;

typedef enum VIDEO_DISPLAY_TYPE_TAG
{
    DISPLAY_FULLSCREEN = 0,
    DISPLAY_4_3_AT_16_9,
    DISPLAY_16_9_AT_4_3,
    DISPLAY_14_9_AT_4_3,
    DISPLAY_14_9_AT_16_9,
    DISPLAY_24_1_AT_4_3,  // 2.4:1 -> 4:3
    DISPLAY_24_1_AT_16_9  // 2.4:1 -> 16:9
} VIDEO_DISPLAY_TYPE;

typedef enum VIDEO_FRAME_DROP_STATE_TAG
{
    VIDEO_NO_DROP = 0,
    VIDEO_DROP_B_FRAME,
    VIDEO_DROP_P_FRAME,
    VIDEO_DROP_HALF_I_FRAME,
    VIDEO_DROP_3_4_I_FRAME,
    VIDEO_DROP_7_8_I_FRAME
} VIDEO_FRAME_DROP_STATE;

typedef enum VIDEO_FRAME_SLOW_MOTION_STATE_TAG
{
    VIDEO_NO_SLOW_FRAME = 0,
    VIDEO_SLOW_2X_FRAME,
    VIDEO_SLOW_4X_FRAME,
    VIDEO_SLOW_8X_FRAME
} VIDEO_FRAME_SLOW_MOTION_STATE;

typedef enum VIDEO_DECODE_EX_MODE_TAG
{
    VIDEO = 0,
    SPLASH,
    SPLASH_LAST_FRAME,
    BACKGROUND,
    IMAGE,
    MPLAYER,
    PLAYBACK
} VIDEO_DECODE_EX_MODE;

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

void
Video_Decoder_Reset(
    MMP_BOOL bFullyReset);

void
Video_Decoder_Init(
    void);

void
Video_Decoder_SetDropState(
    VIDEO_FRAME_DROP_STATE dropState);

void
Video_Decoder_SetSlowMotionState(
    VIDEO_FRAME_SLOW_MOTION_STATE slowmotionState);

void
Video_Decoder_DisplayFirstI(
    void);

//void Video_Decoder_SetPlaybackMode(
//    MMP_BOOL bPlayback);

MMP_UINT32
Video_Decoder_Next(
    void *ptVideoSample);

MMP_UINT32
Video_Decoder_Reload(
    void);

void
Video_Decoder_Display(
    void);

void
Video_Decoder_DisplayJumpFrame(
    void);

void
Video_Decoder_DisplaySlowFrame(
    void);

void
Video_Decoder_Flip(
    VIDEO_FLIP_TYPE videoFlipType);

void
Video_Decoder_Wait(
    void);

void
Video_Decoder_Fire(
    void);

void
Video_Decoder_End(
    void);

void
Video_Decoder_Ex_Display(
    VIDEO_DECODE_EX_MODE exMode);

void
Video_Decoder_Ex_Fire(
    void);

void
Video_Decoder_Ex_SetVideoWindow(
    void);

void
Video_Decoder_Ex_Enter(
    VIDEO_DECODE_EX_MODE exMode);

void
Video_Decoder_Ex_Leave(
    void);

void
Video_Decoder_FocusParse(
    MMP_BOOL bEnable);

void
Video_Decoder_SetAudioLagOffset(
    MMP_UINT32 offset);

void
Video_Decoder_ReSync(
    MMP_BOOL bEnable);

MMP_BOOL
Video_Decoder_IsReSync(
    void);

MMP_BOOL
Video_Decoder_IsReceiveFirstI(
    void);

void
Video_Decoder_ResetDisplayIndex(
    void);

void
Video_SetPauseStcTime(
    void);

void
Video_SetResumeStcTime(
    void);

#ifdef __cplusplus
}
#endif

#endif