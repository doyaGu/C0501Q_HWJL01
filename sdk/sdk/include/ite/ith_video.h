/*
 * Copyright (c) 2014 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL Video functions.
 *
 * @author I-Chun Lai
 * @version 1.0
 */

#ifndef _ITH_VIDEO_H_
#define _ITH_VIDEO_H_

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

#include "ite/itp.h"
#include "ith_vregs.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define BLANK_VIDEO_FRAME_INDEX ((uint32_t)-1)

typedef enum VIDEO_CODEC_TYPE_TAG
{
    VID_CODEC_AVC = 0,
    VID_CODEC_MP4
} VIDEO_CODEC_TYPE;

typedef enum VIDEO_QUERY_TYPE_TAG
{
    VIDEO_STATUS_FRAME_BUF,
    VIDEO_STATUS_T_FIELD_BUF,
    VIDEO_STATUS_B_FIELD_BUF,
    VIDEO_STATUS_FRAME_BUF_ERROR,
    VIDEO_STATUS_T_FIELD_BUF_ERROR,
    VIDEO_STATUS_B_FIELD_BUF_ERROR,
    VIDEO_ADDRESS_FRAME_BUF_Y,
    VIDEO_ADDRESS_FRAME_BUF_U,
    VIDEO_ADDRESS_FRAME_BUF_V,
    VIDEO_ADDRESS_BLANK_BUF_Y,
    VIDEO_ADDRESS_BLANK_BUF_U,
    VIDEO_ADDRESS_BLANK_BUF_V,
    VIDEO_SEMAPHORE,
    VIDEO_FRAME_BUFFER_COUNT,
    VIDEO_FRAME_BUFFER_Y_PITCH,
    VIDEO_FRAME_BUFFER_UV_PITCH,
    VIDEO_MAX_VIDEO_WIDTH,
    VIDEO_MAX_VIDEO_HEIGHT,
    VIDEO_CMD_DATA_BUFFER_MAX_SIZE
} VIDEO_QUERY_TYPE;

/**
 * Error codes.
 */
typedef enum VIDEO_ERROR_CODE_TAG
{
    VIDEO_ERROR_SUCCESS = 0,
    ERROR_VIDEO_MEM_REQUEST,
    ERROR_VIDEO_MEM_RELEASE,
    ERROR_VIDEO_DECODE_HALT,
    ERROR_VIDEO_DECODE_BUFFER_INDEX,
    ERROR_VIDEO_DECODE_BUFFER_BUSY,
    ERROR_VIDEO_DECODE_BUFFER_ERROR,
    ERROR_VIDEO_SEMAPHORE_CREATE,
    ERROR_VIDEO_INVALID_ARG,
    ERROR_VIDEO_INIT_CONFLICT,
    ERROR_VIDEO_NOT_INITED,
} VIDEO_ERROR_CODE;

typedef enum VIDEO_HARDWARE_STATUS_TAG
{
    VIDEO_HARDWARE_STATUS_IDLE = 0,
    VIDEO_HARDWARE_STATUS_BUSY = 1,
    VIDEO_HARDWARE_STATUS_HALT = 2,
    VIDEO_HARDWARE_STATUS_ERROR = 3
} VIDEO_HARDWARE_STATUS;

typedef enum
{
    NORMAL_MODE,
    RMI_MODE,
    VIDEO_DECODE_MODE_COUNT
} VIDEO_DECODE_MODE;

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct AVC_DECODER_TAG
{
    VIDEO_CODEC_TYPE vidCodecType;

    uint8_t   *ppCmdDataBufAddr[3];
    uint32_t  cmdDataBufSize;
    uint32_t  cmdDataBufSelect;
    uint32_t  decodeBufSelect;
    uint32_t  cmdDataBufWrSel;
    uint32_t  cmdDataBufMaxSize;

    // picture_display_extension
    //uint32_t  picture_structure;

    uint32_t  framePitchY;
    uint32_t  framePitchUV;
    uint32_t  frameWidth;
    uint32_t  frameHeight;
    uint32_t  clipStartX;
    uint32_t  clipStartY;
    uint32_t  clipWidth;
    uint32_t  clipHeight;
    uint32_t  frameBufCount;
    //uint32_t  switchHardwareScan; // 0: disable; !0: enable

    uint32_t  currDisplayFrameBufIndex;
    uint32_t  prevDisplayFrameBufIndex;
    uint16_t  enableKeepLastField;
    uint16_t  isProgressive;

    // h.264
    uint32_t  colDataBufAdr[32];
    uint32_t  colDataBufCount;
    uint32_t  lastDisplayFrameBufIndex;
    uint32_t  aspect_ratio_information;
    uint32_t  active_format;

    uint8_t   *pAVCDecoderBuf;
    uint8_t   *pFrameBufAddr[18][3];
    VIDEO_DECODE_MODE mode;
} AVC_DECODER;

typedef struct VIDEO_DECODER_DESC_TAG
{
    uint32_t            video_max_width;
    uint32_t            video_max_height;
    VIDEO_DECODE_MODE   video_decode_mode;
} VIDEO_DECODER_DESC;

//=============================================================================
//                              Function Declaration
//=============================================================================

/*
 * Initialize hardware video decoder
 */
VIDEO_ERROR_CODE ithVideoInit(VIDEO_DECODER_DESC* p_video_desc);

VIDEO_ERROR_CODE ithVideoExit(void);

VIDEO_ERROR_CODE ithVideoOpen(AVC_DECODER* pInfo);

VIDEO_ERROR_CODE ithVideoFire(AVC_DECODER* pInfo);

VIDEO_ERROR_CODE ithVideoWait(AVC_DECODER* pInfo, uint32_t timeout);

VIDEO_ERROR_CODE ithVideoClose(AVC_DECODER* pInfo);

VIDEO_ERROR_CODE ithVideoReset(AVC_DECODER* pInfo);

uint32_t ithVideoQuery(VIDEO_QUERY_TYPE queryType, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif
