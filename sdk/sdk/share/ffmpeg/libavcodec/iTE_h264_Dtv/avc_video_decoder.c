/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @file avc_video_decoder.c
 *
 * @author
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core_config.h"
#include "ite/ith_video.h" //#include "mmp_video.h"
#include "isp/mmp_isp.h"
#include "video_decoder.h"
#include "avc_video_decoder.h"
#include "bitstream_kit.h"
#include "queue_mgr.h"
#include <math.h>
#include "avc.h"

/*
 * include files are needed for hook to ffmpeg decoder
 */
#include "libavcodec/avcodec.h"
#include "libavcodec/h264.h"
#include "libavutil/opt.h"
#include "fc_external.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#if !defined (CFG_CHIP_PKG_IT9852)
#define ENABLE_H264_3_CMD_DATA_BUF
#define MULTI_SLICE
#else
#define ENABLE_2_VIDEO_FRAME_BUF
#endif


//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct AVC_FRAME_BUFFER_CONTROL_TAG
{
    MMP_UINT8  decodeBufSelect;
#ifdef SKIP_BROKEN_VIDEO_FRAME
    MMP_UINT8  fireBufSelect;
#endif

    MMP_UINT32 displayOrderQueue;

    MMP_UINT32 accumTime;
    MMP_UINT32 timeStamp[8];
    MMP_UINT32 frame_rate_code;
    MMP_BOOL   bUpdateTimeStamp;
    MMP_UINT8  DecodingQueue[8];
    MMP_INT32  FramePocQueue[8];
    MMP_UINT8  structure[8];

    MMP_UINT32 preVideoTimeStamp;
} AVC_FRAME_BUFFER_CONTROL;

typedef struct AVC_FRAME_PTS_INTERVAL_TAG
{
    MMP_UINT32 baseInterval;
    MMP_UINT32 remainder;
} AVC_FRAME_PTS_INTERVAL;

typedef struct DISPLAY_INFO_TAG
{
    MMP_UINT32 aspect_ratio_information;
    MMP_UINT32 active_format;
    MMP_UINT32 horizontal_size;
    MMP_UINT32 vertical_size;
} DISPLAY_INFO;

//=============================================================================
//                              Global Data Definition
//=============================================================================3
AVC_DECODER                   *gptAVCDecoder       = MMP_NULL; // for avc driver
AVC_VIDEO_DECODER             *gptAVCVideoDecoder  = MMP_NULL; // for one video packet parsing
AVC_FRAME_BUFFER_CONTROL      gtAVCFrameBufControl = {0};      // for rule of decoded buffer reference and selection

// AVC
MMP_BOOL                      gbStartDecode            = MMP_FALSE;
MMP_BOOL                      gbPreAVCIDR              = MMP_FALSE;
MMP_BOOL                      gbFrameEnd               = MMP_FALSE;
MMP_BOOL                      gbFrameStart             = MMP_TRUE;
static MMP_UINT32             curSliceFirstMB          = 9999;
static MMP_UINT32             aspect_ratio_information = 0;
static MMP_UINT32             video_width              = 0;
static MMP_UINT32             video_height             = 0;
static VIDEO_FRAME_DROP_STATE gDropState               = VIDEO_NO_DROP;
static VIDEO_FRAME_DROP_STATE gPreDropState            = VIDEO_NO_DROP;
static MMP_UINT32             gIframeDropCountDown     = 1;
static MMP_BOOL               gbDisplayFirstI          = MMP_FALSE;
static MMP_BOOL               gbWaitFirstI             = MMP_FALSE;
static MMP_BOOL               gbCompletePic            = MMP_FALSE;
static SliceType              gPrevSliceType           = B_SLICE;
static MMP_BOOL               gbFirstDecFrame          = MMP_TRUE;
static MMP_BOOL               gbIsFrameOrLastPic       = MMP_FALSE;
static MMP_UINT32             gPreStcTime;
static MMP_UINT8              gVideoFlipPerSec;
static MMP_BOOL               gbDropFrame              = MMP_FALSE;
static MMP_BOOL               gbIsPauseStcSet          = MMP_FALSE;
static MMP_UINT32             gPauseStcTime            = 0;
static MMP_UINT32             gPauseStcTimeOffset      = 0;

// ISO/IEC 13818-2 table 6-4 - frame_rate_value
// ms unit, for example, 41.667 ms frame interval for 24 fps
static AVC_FRAME_PTS_INTERVAL gFramePTS_Interval[12] =
{
    {  0,   0}, // 0: forbidden
    { 41, 667}, // 1: 23.976 fps
    { 41, 667}, // 2: 24 fps
    { 40,   0}, // 3: 25 fps
    { 33, 333}, // 4: 29.970 fps
    { 33, 333}, // 5: 30 fps
    { 20,   0}, // 6: 50 fps
    { 16, 667}, // 7: 60 fps
    { 16, 667}, // 8: 60 fps
    { 66, 667}, // 9: 14.985 fps
    { 66, 667}, //10: 15 fps
    {100,   0} //11: 10 fps
};

static MMP_INT32              gInitFramePoc = 0;
static MMP_UINT32             gWrapFramePoc = 0;
static MMP_INT32              gMaxFramePoc  = 0;
static MMP_BOOL               gbFirstFrame  = MMP_TRUE;
static MMP_UINT32             gPTSRdIndex   = 0;
static MMP_UINT32             gPTSWrIndex   = 0;

static MMP_BOOL               gbFocusParse  = MMP_FALSE;

extern ImageParameters        *img;

static MMP_UINT32             gAudioPauseStartTime;

static DISPLAY_INFO           Disp_Info[8];

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static void
_VIDEO_DECODER_ReleaseValue(
    void);

static void
_VIDEO_DECODER_ResetValue(
    MMP_BOOL bFullReset);

static void
_VIDEO_DECODER_SetFrameBufferReference(
    AVCodecContext *avctx);

static void
_VIDEO_DECODER_SetDispInfo(
    MMP_UINT32 index);

#ifdef FF_START_CODE
static MMP_BOOL
ff_next_start_code(
    void);
#endif

static MMP_BOOL
next_start_code(
    void);

#if 0
static MMP_BOOL
_is_hd_video_format(
    MMP_UINT32 horizontal_size,
    MMP_UINT32 frame_rate_code);
#endif

MMP_INLINE static void
_prepare_nal_unit_data(
    MMP_UINT8 *pData,
    MMP_UINT  size);

static int decode_nal_units(H264Context *h, const MMP_UINT8 *buf, int buf_size);
int AVC_Video_Decoder_DecodeExtradata(H264Context *h, const MMP_UINT8 *buf, int size);
unsigned int PalGetClock(void);
unsigned long PalGetDuration(unsigned int clock);

extern uint8_t *
ithVideoGetBufAddr(
    void);

//=============================================================================
//                              Public Function Definition
//=============================================================================
void
AVC_Video_Decoder_Release()
{
    _VIDEO_DECODER_ReleaseValue();
    //mmpVideoDecodeQuery(VIDEO_ADDRESS_FRAME_BUF_Y, 0, &videoBufAddr);
    //if (videoBufAddr==0)
    //{
    // temp solution
    //mmpMpg2DecodeWait(100);
    ithVideoWait(gptAVCDecoder, 100);
    ithVideoHostReset();
    ithVideoReset(gptAVCDecoder);
    //mmpMpg2DecodeReset(gptMpg2Decoder);
}

void
AVC_Video_Decoder_Init(
    AVCodecContext *avctx)
{
    int     i     = 0, j = 0;
    uint8_t *pbuf = NULL;

    // 1. prepare avc decoder
    if (MMP_NULL == gptAVCDecoder)
        gptAVCDecoder = (AVC_DECODER *)calloc(sizeof(char), sizeof(AVC_DECODER)); // for mmp_video engine
    else
        return _VIDEO_DECODER_ResetValue(MMP_FALSE);

    gptAVCDecoder->vidCodecType      = VID_CODEC_AVC;
    gptAVCDecoder->framePitchY       = ithVideoQuery(VIDEO_FRAME_BUFFER_Y_PITCH, 0);
    gptAVCDecoder->framePitchUV      = ithVideoQuery(VIDEO_FRAME_BUFFER_UV_PITCH, 0);
    gptAVCDecoder->frameWidth        = ithVideoQuery(VIDEO_MAX_VIDEO_WIDTH, 0);
    gptAVCDecoder->frameHeight       = ithVideoQuery(VIDEO_MAX_VIDEO_HEIGHT, 0);
    gptAVCDecoder->clipStartX        = 0;
    gptAVCDecoder->clipStartY        = 0;
    gptAVCDecoder->clipWidth         = ithVideoQuery(VIDEO_MAX_VIDEO_WIDTH, 0);
    gptAVCDecoder->clipHeight        = ithVideoQuery(VIDEO_MAX_VIDEO_HEIGHT, 0);
    gptAVCDecoder->frameBufCount     = ithVideoQuery(VIDEO_FRAME_BUFFER_COUNT, 0);
    gptAVCDecoder->cmdDataBufSelect  = 1;
    gptAVCDecoder->cmdDataBufWrSel   = 1;
    gptAVCDecoder->colDataBufCount   = 6;
    gptAVCDecoder->cmdDataBufSize    = 0;
    gptAVCDecoder->cmdDataBufMaxSize = ithVideoQuery(VIDEO_CMD_DATA_BUFFER_MAX_SIZE, 0);
    avc_CreateParameterSetList();

    ithVideoHostReset();

    // frame buffer        : 13056         (in multi-channel (1088+544)*3 = 4896)
    // command data buffer    : 250 * 3    (in multi-channel 100*3)
    // TC buffer        : 4
    // VLD buffer        : 64
    // DB buffer        : 255
    // Colocate buffer        : 2176
    pbuf = ithVideoGetBufAddr(); //(uint8_t*) memalign(64, CFG_VIDEO_BUFFER_PITCH * (13056+750+4+64+255+2176));
    if (pbuf)
    {
        gptAVCDecoder->pAVCDecoderBuf = pbuf;
        av_log(avctx, AV_LOG_INFO, "Video decoder get buffer %p\n", pbuf);
    }
    else
        av_log(avctx, AV_LOG_ERROR, "Video decoder get buffer failed! Not enough Memory\n");

    // 1.1 call avc decode init to get buffers
    ithVideoOpen(gptAVCDecoder);

    // 2. prepare video decoder
    if (MMP_NULL == gptAVCVideoDecoder)
        gptAVCVideoDecoder = (AVC_VIDEO_DECODER *)malloc(sizeof(AVC_VIDEO_DECODER)); // for middle ward

    _VIDEO_DECODER_ResetValue(MMP_TRUE);
    gptAVCVideoDecoder->bIsNalFormat = MMP_FALSE;

#if 0
    printf("===H.264 decode buffer===\n");
    malloc_stats();
    printf("=========================\n");
#endif
}

MMP_UINT32
AVC_Video_Decoder_Next(
    void *ptVideoSample)
{
    // setup infomation
    MMP_UINT32   return_state = 0;
    VIDEO_SAMPLE *ptSample    = (VIDEO_SAMPLE *) ptVideoSample;

    // receive video sample from ts parser
    if (ptSample->pVideoStart)
    {
        gptAVCVideoDecoder->pPacketStartAddr = ptSample->pVideoStart;
        gptAVCVideoDecoder->pPacketEndAddr   = ptSample->pVideoEnd;
        gptAVCVideoDecoder->pBufferStartAddr = ptSample->pRingStart;
        gptAVCVideoDecoder->pBufferEndAddr   = ptSample->pRingEnd;
        gptAVCVideoDecoder->timeStamp        = ptSample->timeStamp;

        if (ptSample->bTimeStamp && VIDEO_NO_DROP == gDropState)
            gtAVCFrameBufControl.bUpdateTimeStamp = MMP_TRUE;
    }
    else
    {
        return_state |= VIDEO_RETURN_END_OF_PACKET;
        goto return_state;
    }

    gptAVCVideoDecoder->remainByte = (MMP_INT32)(gptAVCVideoDecoder->pPacketEndAddr - gptAVCVideoDecoder->pPacketStartAddr);
    if (gptAVCVideoDecoder->remainByte < 0 && gptAVCVideoDecoder->pBufferEndAddr > 0)
        gptAVCVideoDecoder->remainByte = (MMP_INT32)(gptAVCVideoDecoder->pBufferEndAddr - gptAVCVideoDecoder->pPacketStartAddr +
                                                     gptAVCVideoDecoder->pPacketEndAddr - gptAVCVideoDecoder->pBufferStartAddr);

    // for video segment decode
    gptAVCVideoDecoder->bStartSegment = ptSample->bStartSegment;
    gptAVCVideoDecoder->bEndSegment   = ptSample->bEndSegment;
    gptAVCVideoDecoder->pSegStartAddr = ptSample->pData;

    if (MMP_FALSE == gbStartDecode) // TODO
    {
        if (MMP_TRUE == gbPreAVCIDR)
            gbStartDecode = MMP_TRUE;
        else // if (MMP_FALSE == gbPreAVCIDR)
        {
            gbPreAVCIDR = ptSample->bAVCIDR;
        }
    }

    if (gptAVCVideoDecoder->remainByte <= 3 && gptAVCVideoDecoder->bEndSegment)
    {
        return_state |= VIDEO_RETURN_END_OF_PACKET;
    }

return_state:
    return return_state;
}

MMP_UINT32
AVC_Video_Decoder_Reload(
    AVCodecContext *avctx)
{
    // parsing NAL-unit
    MMP_UINT32             return_state       = 0;
    MMP_UINT8              *pPacketStartAddr  = gptAVCVideoDecoder->pPacketStartAddr;
    MMP_UINT8              *pPacketEndAddr    = gptAVCVideoDecoder->pPacketEndAddr;
    MMP_UINT8              *pBufferStartAddr  = gptAVCVideoDecoder->pBufferStartAddr;
    MMP_UINT8              *pBufferEndAddr    = gptAVCVideoDecoder->pBufferEndAddr;

    AVC_ERROR_CODE         ret                = AVC_ERROR_SUCCESS;
    MMP_BOOL               bFind              = MMP_FALSE;
    MMP_UINT8              *pNalStream        = MMP_NULL;
    MMP_UINT32             nalSize            = 0;
    AVC_NAL_HEADER         nalHeader          = {0};
    MMP_UINT8              *pPreStartCodeAddr = MMP_NULL;
    MMP_UINT32             nextSliceFirstMB   = 0;
    BITSTREAM              tBitStream         = {0};
    MMP_BOOL               bDropFrame         = MMP_FALSE;
    VIDEO_FRAME_DROP_STATE curDropState       = VIDEO_NO_DROP;
    MMP_BOOL               bRecovery_Point    = MMP_FALSE;
    MMP_UINT32             decodeBufIdx       = 0;
    int                    frame_size         = pPacketEndAddr - pPacketStartAddr;
    static int             max_i_size         = 0, max_p_size = 0;

    gptAVCVideoDecoder->remainByte = (MMP_INT32)(pPacketEndAddr - pPacketStartAddr);
    if (gptAVCVideoDecoder->remainByte < 0 && pBufferEndAddr > 0)
        gptAVCVideoDecoder->remainByte = (MMP_INT32)(pBufferEndAddr - pPacketStartAddr +
                                                     pPacketEndAddr - pBufferStartAddr);

    // for video segment decode
    if (gptAVCVideoDecoder->bStartSegment)
    {
        gptAVCVideoDecoder->pSegEndAddr   = gptAVCVideoDecoder->pPacketStartAddr;
        gptAVCVideoDecoder->pSegStartAddr = gptAVCVideoDecoder->pPacketStartAddr;
        gptAVCVideoDecoder->bStartSegment = MMP_FALSE;
    }

    if (!gptAVCVideoDecoder->bEndSegment && gptAVCVideoDecoder->remainByte < 2048)
    {
        av_log(avctx, AV_LOG_ERROR, "not enough data to decode\n");
    }

    if (gptAVCVideoDecoder->bIsNalFormat)
    {
        // RTP nal case
        tBitStream.remainByte       = gptAVCVideoDecoder->remainByte;
        tBitStream.pBufferStartAddr = gptAVCVideoDecoder->pBufferStartAddr;
        tBitStream.pBufferEndAddr   = gptAVCVideoDecoder->pBufferEndAddr;
        BitStreamKit_Init(&tBitStream, gptAVCVideoDecoder->pPacketStartAddr);
    }
    else
    {
        // annex-b case in ITU-T Rec. H.264 (03/2005)
        // search 0x000001
        if (gptAVCVideoDecoder->remainByte > 3)
            bFind = next_start_code();
        else
            return_state |= VIDEO_RETURN_SKIP_DECODE;

        // return code = TRUE means find start code
        //               FALSE means end segment
        if (!bFind)
            return_state |= VIDEO_RETURN_SKIP_DECODE;
    }

    while (return_state == 0 && gptAVCVideoDecoder->remainByte > 0)
    {
        if (gptAVCVideoDecoder->bIsNalFormat)
        {
            MMP_UINT32 LenSize = 0;

            // RTP nal case
            if (nalSize > 0)
                BitStreamKit_SkipBits(&tBitStream, 8 * nalSize);
            if (tBitStream.remainByte <= 0)
            {
                return_state |= VIDEO_RETURN_SKIP_DECODE;
                break;
            }

            // work-around for lengthSizeMinusOne Wrong
            LenSize = gptAVCVideoDecoder->NALUnitLength;

            if (gptAVCVideoDecoder->remainByte == (1 + (BitStreamKit_ShowBits(&tBitStream, 0, 8 * 1))))
                LenSize = 1;
            if (gptAVCVideoDecoder->remainByte == (2 + (BitStreamKit_ShowBits(&tBitStream, 0, 8 * 2))))
                LenSize = 2;
            if (gptAVCVideoDecoder->remainByte == (3 + (BitStreamKit_ShowBits(&tBitStream, 0, 8 * 3))))
                LenSize = 3;
            if (gptAVCVideoDecoder->remainByte == (4 + (BitStreamKit_ShowBits(&tBitStream, 0, 8 * 4))))
                LenSize = 4;

            nalSize    = BitStreamKit_GetBits(&tBitStream, 8 * LenSize);
            pNalStream = tBitStream.pStartAddress;

            // temp solution
            if (nalSize > gptAVCVideoDecoder->remainByte)
                nalSize = gptAVCVideoDecoder->remainByte;

            gptAVCVideoDecoder->pPacketStartAddr = tBitStream.pStartAddress + nalSize;
            if (gptAVCVideoDecoder->pPacketStartAddr >= pBufferEndAddr)
            {
                gptAVCVideoDecoder->pPacketStartAddr = pBufferStartAddr + (gptAVCVideoDecoder->pPacketStartAddr - pBufferEndAddr);
            }
        }
        else
        {
            // annex-b case in ITU-T Rec. H.264 (03/2005)
            pPreStartCodeAddr                    = gptAVCVideoDecoder->pPacketStartAddr;

            gptAVCVideoDecoder->pPacketStartAddr = (MMP_UINT8 *)gptAVCVideoDecoder->pPacketStartAddr + 3;

            if (gptAVCVideoDecoder->pPacketStartAddr >= pBufferEndAddr)
            {
                av_log(avctx, AV_LOG_ERROR, "Error Start 0 %x %x\n", gptAVCVideoDecoder->pPacketStartAddr, pBufferEndAddr);
                gptAVCVideoDecoder->pPacketStartAddr = pBufferStartAddr + (gptAVCVideoDecoder->pPacketStartAddr - pBufferEndAddr);
            }

#ifdef FF_START_CODE
            bFind      = ff_next_start_code();
#else
            bFind      = next_start_code();
#endif

            pNalStream = (MMP_UINT8 *)pPreStartCodeAddr + 3;
            if (pNalStream >= pBufferEndAddr)
            {
                av_log(avctx, AV_LOG_ERROR, "Error Start 1 %x %x\n", pNalStream, pBufferEndAddr);
                pNalStream = pBufferStartAddr + (pNalStream - pBufferEndAddr);
            }

            if (gptAVCVideoDecoder->pPacketStartAddr > pPreStartCodeAddr)
                nalSize = (MMP_UINT32)(gptAVCVideoDecoder->pPacketStartAddr - pPreStartCodeAddr);
            else
                nalSize = (MMP_UINT32)(pBufferEndAddr - pPreStartCodeAddr +
                                       gptAVCVideoDecoder->pPacketStartAddr - pBufferStartAddr);

            // 0x000001 (3 bytes) and Nal_Header (1 byte)
            if (nalSize < 4 || !(pPreStartCodeAddr[3] == 0x67 || pPreStartCodeAddr[3] == 0x68 || pPreStartCodeAddr[3] == 0x65 || pPreStartCodeAddr[3] == 0x41 || pPreStartCodeAddr[3] == 0x61 || pPreStartCodeAddr[3] == 0x06 ))
            {
                return_state |= VIDEO_RETURN_SKIP_PACKET;
                break;
            }
            else
            {
                nalSize -= 4;
            }
        }

        avc_Nal_Get_Header(&nalHeader, pNalStream);

        if (nalHeader.forbidden_zero_bit != 0)
        {
            // forbidden_zero_bit must be "0".
            av_log(avctx, AV_LOG_ERROR, "Nal-Uint parsing error !! \n");
        }

        pNalStream = (MMP_UINT8 *)pNalStream + 1;

        if (pNalStream >= pBufferEndAddr)
        {
            av_log(avctx, AV_LOG_ERROR, "Error Start 2 %x %x\n", pNalStream, pBufferEndAddr);
            pNalStream = pBufferStartAddr + (pNalStream - pBufferEndAddr);
        }

        //printf(" Header %d\n", nalHeader.nal_unit_type);
        switch (nalHeader.nal_unit_type)
        {
        case NAL_UNIT_TYPE_SEQ_PARAM_SET:
            av_log(avctx, AV_LOG_DEBUG, " nal_unit_type: NAL_UNIT_TYPE_SEQ_PARAM_SET\n");
            if (nalSize <= MAX_NAL_SIZE)
                ret = avc_Seq_Parameter(pNalStream, nalSize);
            else
                av_log(avctx, AV_LOG_ERROR, "! Nal size (%d) > %d\n", nalSize, MAX_NAL_SIZE);

            if (ret != AVC_ERROR_SUCCESS)
            {
                //Error control
                av_log(avctx, AV_LOG_ERROR, "parse SPS fail (0x%x)!!", ret);
            }
            break;

        case NAL_UNIT_TYPE_PIC_PARAM_SET:
            av_log(avctx, AV_LOG_DEBUG, " nal_unit_type: NAL_UNIT_TYPE_PIC_PARAM_SET\n");
            if (nalSize <= MAX_NAL_SIZE)
                ret = avc_Pic_Parameter(pNalStream, nalSize);
            else
                av_log(avctx, AV_LOG_ERROR, "! Nal size (%d) > %d\n", nalSize, MAX_NAL_SIZE);

            if (ret != AVC_ERROR_SUCCESS)
            {
                //Error control
                av_log(avctx, AV_LOG_ERROR, "parse PPS fail (0x%x)!!", ret);
            }
            break;

        case NAL_UNIT_TYPE_SEI:
            av_log(avctx, AV_LOG_DEBUG, " nal_unit_type: NAL_UNIT_TYPE_SEI\n");
            if (nalSize <= MAX_NAL_SIZE)
                ret = avc_SEI(pNalStream, nalSize, &bRecovery_Point);
            else
                av_log(avctx, AV_LOG_ERROR, "! Nal size (%d) > %d\n", nalSize, MAX_NAL_SIZE);

            if (bRecovery_Point == MMP_TRUE)
                gbStartDecode = MMP_TRUE;

            if (ret != AVC_ERROR_SUCCESS)
            {
                //Error control
                av_log(avctx, AV_LOG_ERROR, "parse SEI fail (0x%x)!!", ret);
            }
            break;

        case NAL_UNIT_TYPE_IDR_PICTURE:
            av_log(avctx, AV_LOG_DEBUG, " nal_unit_type: NAL_UNIT_TYPE_IDR_PICTURE\n");

        case NAL_UNIT_TYPE_NON_IDR_PICTURE:
            av_log(avctx, AV_LOG_DEBUG, " nal_unit_type: NAL_UNIT_TYPE_NON_IDR_PICTURE\n");
            if ((NAL_UNIT_TYPE_IDR_PICTURE == nalHeader.nal_unit_type) && (gbStartDecode == 0))
            {
                av_log(NULL, AV_LOG_ERROR, "IDR coming\n");
                gbStartDecode = MMP_TRUE;
            }

            if (MMP_FALSE == gbStartDecode || nalSize < 5)
            {
                return_state |= VIDEO_RETURN_SKIP_DECODE;
                break;
            }

#if !defined(MULTI_SLICE)
            gbFrameStart                  = MMP_TRUE;
            gbFrameEnd                    = MMP_TRUE;
            gptAVCDecoder->cmdDataBufSize = 0;
#endif

#if defined(MULTI_SLICE)
            avc_Get_First_MB_In_Slice(&nextSliceFirstMB, pNalStream, nalSize);
            av_log(avctx, AV_LOG_DEBUG, "Slice firstMB %d\n", nextSliceFirstMB);
            //printf("Slice firstMB %d cur %d\n", nextSliceFirstMB, curSliceFirstMB);
            if (nextSliceFirstMB <= curSliceFirstMB && curSliceFirstMB != 9999)
            {
                if (gbDropFrame)
                {
                    if (NAL_UNIT_TYPE_IDR_PICTURE == nalHeader.nal_unit_type)
                        avc_set_skip_dec(MMP_FALSE);
                    else
                        avc_set_skip_dec(MMP_TRUE);

                    return_state |= VIDEO_RETURN_SKIP_DECODE;
                    avc_release_decoding_buf();
                }
                else
                {
                    gptAVCDecoder->cmdDataBufWrSel = (gptAVCDecoder->cmdDataBufWrSel + 1) % 3;
                    avc_set_skip_dec(MMP_FALSE);

                    if (gbFirstDecFrame && gbIsFrameOrLastPic)
                        gbFirstDecFrame = MMP_FALSE;
                }

                if (gbDisplayFirstI)
                {
                    if (gbWaitFirstI && gbCompletePic)
                        gbWaitFirstI = MMP_FALSE;
                }

                gbFrameStart = MMP_TRUE;
                gbFrameEnd   = MMP_TRUE;

                if (!gbDropFrame)
                    avc_frame_end();

                gbDropFrame                   = MMP_FALSE;

                gptAVCDecoder->cmdDataBufSize = 0;
            }
            else
            {
                //set next slice pointer
                //set last slice
                if (MMP_FALSE == gbFrameStart)
                    avc_multi_slice_wr(nextSliceFirstMB);
            }

            curSliceFirstMB = nextSliceFirstMB;
#endif
            ret             = avc_slice(&nalHeader, pNalStream, nalSize);

            if (ret == ERROR_RESOLUTION)
            {
                av_log(avctx, AV_LOG_ERROR, "resolution not support (width = %d, height = %d), frame_rate_code: %d\n",
                       gptAVCDecoder->frameWidth,
                       gptAVCDecoder->frameHeight,
                       gptAVCVideoDecoder->frame_rate_code);

#if 0
                if (_is_hd_video_format(gptAVCDecoder->frameWidth, gptAVCVideoDecoder->frame_rate_code))
                    if (gptAVCVideoDecoder->bHDVideoFormat == MMP_FALSE)
                    {
                        gptAVCVideoDecoder->bHDVideoFormat = MMP_TRUE;
                    }
#endif
                gbStartDecode                 = MMP_FALSE;
                gbPreAVCIDR                   = MMP_FALSE;
                gbFrameStart                  = MMP_TRUE;
                curSliceFirstMB               = 9999;
                gptAVCDecoder->cmdDataBufSize = 0;

                avc_release_decoding_buf();

                return_state |= VIDEO_RETURN_RESOLUTION_NOT_SUPPORT;
                break;
            }
#if 0
            else if (MMP_TRUE == gptAVCVideoDecoder->bHDVideoFormat)
            {
                av_log(avctx, AV_LOG_ERROR, "!!! HD Video Format \n");

                gptAVCVideoDecoder->bHDVideoFormat = MMP_FALSE;
            }
#endif
            // ----------------------------
            // drop frame for forward/backward
            if (AVC_ERROR_SUCCESS == ret)
            {
                SliceType sliceType = B_SLICE;
                MMP_BOOL  bFrame    = MMP_FALSE;
                MMP_BOOL  bLastPic  = MMP_FALSE;

                if (gbFrameStart)
                {
                    if (get_dec_pic_info(&sliceType, &bFrame, &bLastPic))
                    {
                        if ((gDropState != gPreDropState) && bLastPic)
                        {
                            curDropState = gPreDropState;
                        }
                        else
                            curDropState = gDropState;

                        switch (curDropState)
                        {
                        case VIDEO_NO_DROP: //normal case
                            break;
                        case VIDEO_DROP_B_FRAME:
                            if (bFrame || MMP_FALSE == bLastPic)
                            {
                                if (B_SLICE == sliceType)
                                    bDropFrame = MMP_TRUE;
                            }
                            else
                            {
                                if (bLastPic && B_SLICE == gPrevSliceType)
                                    bDropFrame = MMP_TRUE;
                            }
                            break;

                        case VIDEO_DROP_P_FRAME:
                            if (bFrame || MMP_FALSE == bLastPic)
                            {
                                if (sliceType != I_SLICE && sliceType != SI_SLICE)
                                    bDropFrame = MMP_TRUE;
                            }
                            else
                            {
                                if (bLastPic && (gPrevSliceType != I_SLICE && gPrevSliceType != SI_SLICE))
                                    bDropFrame = MMP_TRUE;
                            }

                            if (gbDisplayFirstI)
                            {
                                if (MMP_FALSE == gbWaitFirstI)
                                    bDropFrame = MMP_TRUE;
                                if (bLastPic || bFrame)
                                    gbCompletePic = MMP_TRUE;
                            }

                            if (bDropFrame)
                                gbStartDecode = MMP_FALSE;
                            break;

                        case VIDEO_DROP_HALF_I_FRAME:
                        case VIDEO_DROP_3_4_I_FRAME:
                        case VIDEO_DROP_7_8_I_FRAME:
                            if (MMP_TRUE == bLastPic)
                                gbStartDecode = MMP_FALSE;

                            if (bFrame)
                            {
                                if (I_SLICE == sliceType || SI_SLICE == sliceType)
                                {
                                    if (gIframeDropCountDown)
                                        --gIframeDropCountDown;
                                    else
                                    {
                                        bDropFrame           = MMP_TRUE;
                                        gIframeDropCountDown = 2 ^ (curDropState - VIDEO_DROP_P_FRAME) - 1;
                                    }
                                }
                                else
                                    bDropFrame = MMP_TRUE;
                            }
                            else
                            {
                                if (MMP_FALSE == bLastPic)
                                {
                                    if ((sliceType != I_SLICE && sliceType != SI_SLICE)
                                        || gIframeDropCountDown)
                                    {
                                        bDropFrame = MMP_TRUE;
                                    }
                                }
                                else
                                {
                                    if (gIframeDropCountDown)
                                    {
                                        if (I_SLICE == gPrevSliceType || SI_SLICE == gPrevSliceType)
                                            --gIframeDropCountDown;
                                        bDropFrame = MMP_TRUE;
                                    }
                                    else
                                    {
                                        if (gPrevSliceType != I_SLICE && gPrevSliceType != SI_SLICE)
                                            bDropFrame = MMP_TRUE;
                                        else
                                            gIframeDropCountDown = 2 ^ (curDropState - VIDEO_DROP_P_FRAME) - 1;
                                    }
                                }
                            }
                            break;
                        }
                        gPrevSliceType = sliceType;
                    }
                    else
                    {
                        gPrevSliceType = B_SLICE;
                        bDropFrame     = MMP_TRUE;
                    }
                    gbDropFrame = bDropFrame;
                    if (bFrame || bLastPic)
                        gbIsFrameOrLastPic = MMP_TRUE;
                    else
                        gbIsFrameOrLastPic = MMP_FALSE;

                    gPreDropState = gDropState;
                }

                if (gbFrameStart && MMP_FALSE == bDropFrame)
                {
                    _VIDEO_DECODER_SetFrameBufferReference(avctx);
                }
            }
            else
            {
                gbFrameStart = MMP_TRUE;
                switch (ret)
                {
                case ERROR_TEMP_USE:
                    return_state |= VIDEO_RETURN_END_OF_PACKET;
                    break;

                case ERROR_OUT_SUPPORT_REF_RANGE:
                    return_state |= VIDEO_RETURN_OUT_REF_FRAME_RANGE;
                    break;

                default:
                    return_state |= VIDEO_RETURN_SKIP_DECODE;
                    break;
                }
                curSliceFirstMB               = 9999;
                gptAVCDecoder->cmdDataBufSize = 0;

                avc_release_decoding_buf();
                break;
            }

            gbFrameStart = MMP_FALSE;

            if (gbFrameEnd)
                return_state |= VIDEO_RETURN_NORMAL;
            else
                return_state |= VIDEO_RETURN_WAIT_FIRE;

            gbFrameEnd = MMP_FALSE;

#if !defined(MULTI_SLICE)
            avc_frame_end();

#ifdef ENABLE_H264_3_CMD_DATA_BUF
            gptAVCDecoder->cmdDataBufWrSel = (gptAVCDecoder->cmdDataBufWrSel + 1) % 3;
#else
            gptAVCDecoder->cmdDataBufWrSel = !gptAVCDecoder->cmdDataBufWrSel;
#endif
            
#endif
            break;

        case NAL_UNIT_TYPE_ACCESS_UNIT_DELIMITER:
            av_log(avctx, AV_LOG_DEBUG, " nal_unit_type: NAL_UNIT_TYPE_ACCESS_UNIT_DELIMITER\n");
            break;

        default:
            av_log(avctx, AV_LOG_DEBUG, " nal_unit_type: %d\n", nalHeader.nal_unit_type);
            return_state |= VIDEO_RETURN_SKIP_DECODE;
            break;
        }

        if (gptAVCVideoDecoder->bIsNalFormat)
        {
            gptAVCVideoDecoder->remainByte = gptAVCVideoDecoder->remainByte - nalSize - 4; //gptAVCVideoDecoder->NALUnitLength;
        }
    }

free_resource:
    //gptAVCVideoDecoder->pSegEndAddr = (MMP_UINT8 *) ((MMP_UINT32) gptAVCVideoDecoder->pPacketStartAddr);
    if (return_state == VIDEO_RETURN_RESOLUTION_NOT_SUPPORT || gptAVCVideoDecoder->remainByte < 0)
        gptAVCVideoDecoder->pSegEndAddr = (MMP_UINT8 *) ((MMP_UINT32) gptAVCVideoDecoder->pPacketEndAddr);
    else
        gptAVCVideoDecoder->pSegEndAddr = (MMP_UINT8 *) ((MMP_UINT32) gptAVCVideoDecoder->pPacketStartAddr);

    gptAVCVideoDecoder->pSegStartAddr = (MMP_UINT8 *) gptAVCVideoDecoder->pPacketStartAddr;

    if (gptAVCVideoDecoder->remainByte <= 3 && gptAVCVideoDecoder->bEndSegment)
    {
        return_state |= VIDEO_RETURN_END_OF_PACKET;
    }

    if (ERROR_TEMP_USE == ret)
        AVC_Video_Decoder_ReSync(MMP_TRUE);

    return return_state;
}

void
AVC_Video_Decoder_Wait(
    void)
{
    if (ithVideoWait(gptAVCDecoder, 100) != 0)
    {
        printf("wait avc idle timeout....1\n");
        ithVideoHostReset();
        printf("wait avc idle timeout....2\n");
        ithVideoReset(gptAVCDecoder);
        printf("wait avc idle timeout....3\n");
    }

#ifdef SKIP_BROKEN_VIDEO_FRAME
    if (gtAVCFrameBufControl.fireBufSelect != 0xF
        && mmpVideoDecodeQuery(VIDEO_STATUS_FRAME_BUF_ERROR, gtAVCFrameBufControl.fireBufSelect, MMP_NULL) != 0)
    {
        dbg_msg(DBG_MSG_TYPE_ERROR, "====================\nDecode Error\n====================\n");
        _VIDEO_DECODER_ResetValue(MMP_FALSE);
    }
#endif
}

void
AVC_Video_Decoder_Flip(
    VIDEO_FLIP_TYPE videoFlipType)
{
#ifdef ENABLE_ONFLY
    //MMP_UINT16 ispStatus = 0;
    MMP_UINT16 flipQCount = 0;
#endif

    switch (videoFlipType)
    {
    case VIDEO_FRAME:
        gptAVCDecoder->enableKeepLastField      = MMP_FALSE;
        gptAVCDecoder->lastDisplayFrameBufIndex = gptAVCDecoder->currDisplayFrameBufIndex;
        break;

    case LAST_VIDEO_FRAME:
        if (gptAVCDecoder->currDisplayFrameBufIndex < gptAVCDecoder->frameBufCount)
        {
            gptAVCDecoder->enableKeepLastField = MMP_TRUE;
            break;
        }

    case BLANK_FRAME:
    default:
        // set to display blank frame, for example, change channel or signal weak
        gptAVCDecoder->currDisplayFrameBufIndex     =
            gptAVCDecoder->prevDisplayFrameBufIndex = gptAVCDecoder->frameBufCount;
        break;
    }

#ifdef ENABLE_ONFLY
    //HOST_ReadRegister( ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT16*)&ispStatus );
    //  if (!(ispStatus & 0x0008))
    HOST_ReadRegister(0x412, (MMP_UINT16 *)&flipQCount);
    if (!(flipQCount & 0x000F))
    {
        if (Video_Flip_IsEnabled() == MMP_FALSE)
            mmpVideoDecodeDisplay(gptAVCDecoder);
        else
            Video_Flip_DoFlip(MPS_VIDEO_H264_CODEC, gptAVCDecoder);
    }
#else
    //[Evan:mark 'cause not implement]mmpVideoDecodeDisplay(gptAVCDecoder);
#endif
}

void
AVC_Video_Decoder_Display(
    AVCodecContext *avctx,
    AVFrame        *data,
    int            *data_size)
{
    AVFrame    *pict                    = data;

    MMP_UINT32 displayOrderQueue        = gtAVCFrameBufControl.displayOrderQueue;

#if 0//|| defined(ENABLE_7_FRAME_BUFFER)
    MMP_UINT32 currDisplayFrameBufIndex = (displayOrderQueue & 0x000000FF);
    MMP_UINT32 prevDisplayFrameBufIndex = (displayOrderQueue & 0x0000FF00) >> 8;
#else

    #if defined(MULTI_SLICE)
    MMP_UINT32 currDisplayFrameBufIndex = (displayOrderQueue & 0x00FF0000) >> 16;
    MMP_UINT32 prevDisplayFrameBufIndex = (displayOrderQueue & 0xFF000000) >> 24;
    #else
    #if defined(ENABLE_2_VIDEO_FRAME_BUF)
    MMP_UINT32 currDisplayFrameBufIndex = (displayOrderQueue & 0x0000FF);
    MMP_UINT32 prevDisplayFrameBufIndex = (displayOrderQueue & 0x00FF00) >> 8;
    #else
    MMP_UINT32 currDisplayFrameBufIndex = (displayOrderQueue & 0x0000FF00) >> 8;
    MMP_UINT32 prevDisplayFrameBufIndex = (displayOrderQueue & 0x00FF0000) >> 16;
    #endif
    #endif
#endif
    MMP_BOOL   bDeinterlace             = 0;       //[Evan]mmpIspQuery(MMP_ISP_DEINTERLACE);
    MMP_UINT32 videoTimeStamp;
    MMP_UINT32 rdIndex;

    *data_size = 0;

    if (img->structure == TOP_FIELD)
        return;

    if (gptAVCDecoder == MMP_NULL)
        return;

    // AVFrame.pkt_pts = 1 / frame_rate * 90000
    // frame_rate = 1 / time_base
    if (currDisplayFrameBufIndex != 0xFF)
        videoTimeStamp = gtAVCFrameBufControl.timeStamp[currDisplayFrameBufIndex];

    rdIndex = currDisplayFrameBufIndex;

    //printf("Disp %d\n", currDisplayFrameBufIndex);
    //[2012/05/03, Evan:hook for display order]
    if (currDisplayFrameBufIndex != 0xFF)
    {
        MMP_UINT32 width        = gptAVCDecoder->frameWidth;
        MMP_UINT32 height       = gptAVCDecoder->frameHeight;
        MMP_UINT32 framePitchY  = gptAVCDecoder->framePitchY;
        MMP_UINT32 framePitchUV = gptAVCDecoder->framePitchUV;
        MMP_UINT8  *pFrameBuf   = gptAVCDecoder->pAVCDecoderBuf;

        pict->width                             = width;
        pict->height                            = height;

        pict->data[0]                           = ithVideoQuery(VIDEO_ADDRESS_FRAME_BUF_Y, rdIndex);
        pict->data[1]                           = ithVideoQuery(VIDEO_ADDRESS_FRAME_BUF_U, rdIndex);
        pict->data[2]                           = ithVideoQuery(VIDEO_ADDRESS_FRAME_BUF_V, rdIndex);
        pict->data[3]                           = rdIndex;
        gptAVCDecoder->lastDisplayFrameBufIndex = rdIndex;

        pict->linesize[0]                       = framePitchY;
        pict->linesize[1]                       = framePitchUV;

        // tmp, need to modify! make no sense now.
        pict->pts                               = videoTimeStamp;

        *data_size                              = sizeof(AVFrame);

        //*(MMP_UINT32 *)pict->opaque             = rdIndex;
        gptAVCDecoder->currDisplayFrameBufIndex = currDisplayFrameBufIndex;
        gptAVCDecoder->prevDisplayFrameBufIndex = prevDisplayFrameBufIndex;
        av_log(avctx, AV_LOG_DEBUG, "display frame buf %d\n", rdIndex);
    }
}

void
AVC_Video_Decoder_Fire(
    void)
{
    if ((gbFirstDecFrame || (gptAVCDecoder->cmdDataBufWrSel != gptAVCDecoder->cmdDataBufSelect))
        && gptAVCDecoder->cmdDataBufSize)
    {
        gptAVCDecoder->decodeBufSelect = gtAVCFrameBufControl.decodeBufSelect;

        //if(gExMode != MPLAYER)
        //    ChkSlice();

        ithVideoFire(gptAVCDecoder);
#ifdef SKIP_BROKEN_VIDEO_FRAME
        gtAVCFrameBufControl.fireBufSelect = gptAVCDecoder->decodeBufSelect;
#endif
        // change other command data buffer
        //gptAVCDecoder->cmdDataBufSelect ^= 1;
#ifdef ENABLE_H264_3_CMD_DATA_BUF
        gptAVCDecoder->cmdDataBufSelect = (gptAVCDecoder->cmdDataBufSelect + 1) % 3;
#else
        gptAVCDecoder->cmdDataBufSelect = !gptAVCDecoder->cmdDataBufSelect;
#endif
    }
#ifdef SKIP_BROKEN_VIDEO_FRAME
    else
    {
        gtAVCFrameBufControl.fireBufSelect = 0xF;
    }
#endif
}

void
AVC_Video_Decoder_End(
    void)
{
    if (!gptAVCDecoder)
        return;

    printf("[%s] deinit\n", __FUNCTION__);

    // reset engine and free memory in avc driver
    ithVideoClose(gptAVCDecoder);

    gptAVCDecoder->pAVCDecoderBuf = MMP_NULL;

    AVC_Video_Decoder_Release();
    avc_ReleaseAllResource();

    // free memory in video decoder
    if (gptAVCDecoder)
        free(gptAVCDecoder);
    gptAVCDecoder = MMP_NULL;

    if (gptAVCVideoDecoder)
        free(gptAVCVideoDecoder);
    gptAVCVideoDecoder = MMP_NULL;
}

void
AVC_Video_Decoder_ReSync(
    MMP_BOOL bEnable)
{
    if ((gptAVCVideoDecoder->bReSync = bEnable) == MMP_TRUE)
    {
        usleep(1000);
    }
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
static void
_VIDEO_DECODER_ReleaseValue(
    void)
{
    MMP_UINT32 i;

    gptAVCDecoder->framePitchY  = ithVideoQuery(VIDEO_FRAME_BUFFER_Y_PITCH, 0);
    gptAVCDecoder->framePitchUV = ithVideoQuery(VIDEO_FRAME_BUFFER_UV_PITCH, 0);
    gptAVCDecoder->frameWidth   = ithVideoQuery(VIDEO_MAX_VIDEO_WIDTH, 0);
    gptAVCDecoder->frameHeight  = ithVideoQuery(VIDEO_MAX_VIDEO_HEIGHT, 0);

    gptAVCDecoder->cmdDataBufSize          = 0;

    gtAVCFrameBufControl.decodeBufSelect   = 0;
    gtAVCFrameBufControl.displayOrderQueue = 0xFFFFFFFF;

    for (i = 0; i < 8; i++)
    {
        gtAVCFrameBufControl.DecodingQueue[i] = 0;
        gtAVCFrameBufControl.FramePocQueue[i] = 0x7FFFFFFF;
    }

    gtAVCFrameBufControl.accumTime         = 0;
    gtAVCFrameBufControl.preVideoTimeStamp = 0;

    memset(gtAVCFrameBufControl.timeStamp, 0, sizeof(gtAVCFrameBufControl.timeStamp));

    gptAVCVideoDecoder->frame_rate_code    = 0;
    gptAVCVideoDecoder->bAVsyncReady       = MMP_FALSE;
    gptAVCVideoDecoder->currAfd            = 0xFF;
    //[Evan]gptAVCVideoDecoder->currVideoRatio           = infoMgr_GetVideoRatio();
    gptAVCVideoDecoder->bUpdateAspectRatio = MMP_FALSE;
    gptAVCVideoDecoder->active_format      = 0;
    gptAVCDecoder->active_format           = 0;

    gPauseStcTime                          = 0;
    gPauseStcTimeOffset                    = 0;
    gPTSRdIndex                            = 0;
    gPTSWrIndex                            = 0;

    avc_ResetBuffer();

    avc_ReleaseParameterSetList();
    //gptAVCDecoder->currDisplayFrameBufIndex      = 8;
    gptAVCVideoDecoder->aspect_ratio_information = 0x1;


    //avc_ResetBuffer();

    gbStartDecode                      = MMP_FALSE;
    gbPreAVCIDR                        = MMP_FALSE;
    gbFrameEnd                         = MMP_FALSE;
    gbIsFrameOrLastPic                 = MMP_FALSE;
    gbDisplayFirstI                    = MMP_FALSE;
    gbWaitFirstI                       = MMP_FALSE;
    gbIsPauseStcSet                    = MMP_FALSE;

    gbFirstFrame                       = MMP_TRUE;
    gbFirstDecFrame                    = MMP_TRUE;
    gbFrameStart                       = MMP_TRUE;
    gbDropFrame                        = MMP_TRUE;

    gPrevSliceType                     = B_SLICE;
    curSliceFirstMB                    = 9999;

    gptAVCDecoder->cmdDataBufSelect    = 1;
    gptAVCDecoder->cmdDataBufWrSel     = 1;

    gInitFramePoc                      = 0;
    gWrapFramePoc                      = 0;
    gMaxFramePoc                       = 0;
    // gCurUpdateBufIndex = 0xFF;

    gptAVCVideoDecoder->gPreTimeStamp  = 0;

#ifdef SKIP_BROKEN_VIDEO_FRAME
    gtAVCFrameBufControl.fireBufSelect = 0xF;
#endif

    gptAVCVideoDecoder->bHDVideoFormat = MMP_FALSE;
}

static void
_VIDEO_DECODER_ResetValue(
    MMP_BOOL bFullReset)
{
    MMP_UINT32 i;

    if (bFullReset == MMP_TRUE)
    {
        gptAVCDecoder->framePitchY  = ithVideoQuery(VIDEO_FRAME_BUFFER_Y_PITCH, 0);
        gptAVCDecoder->framePitchUV = ithVideoQuery(VIDEO_FRAME_BUFFER_UV_PITCH, 0);
        gptAVCDecoder->frameWidth   = ithVideoQuery(VIDEO_MAX_VIDEO_WIDTH, 0);
        gptAVCDecoder->frameHeight  = ithVideoQuery(VIDEO_MAX_VIDEO_HEIGHT, 0);
    }

    gptAVCDecoder->cmdDataBufSize          = 0;

    gtAVCFrameBufControl.decodeBufSelect   = 0;
    gtAVCFrameBufControl.displayOrderQueue = 0xFFFFFFFF;

    for (i = 0; i < 8; i++)
    {
        gtAVCFrameBufControl.DecodingQueue[i] = 0;
        gtAVCFrameBufControl.FramePocQueue[i] = 0x7FFFFFFF;
    }

    gtAVCFrameBufControl.accumTime         = 0;
    gtAVCFrameBufControl.preVideoTimeStamp = 0;

    memset(gtAVCFrameBufControl.timeStamp, 0, sizeof(gtAVCFrameBufControl.timeStamp));

    gptAVCVideoDecoder->frame_rate_code    = 0;
    gptAVCVideoDecoder->bAVsyncReady       = MMP_FALSE;
    gptAVCVideoDecoder->currAfd            = 0xFF;
    //[Evan]gptAVCVideoDecoder->currVideoRatio           = infoMgr_GetVideoRatio();
    gptAVCVideoDecoder->bUpdateAspectRatio = MMP_FALSE;
    gptAVCVideoDecoder->active_format      = 0;
    gptAVCDecoder->active_format           = 0;

    gPauseStcTime                          = 0;
    gPauseStcTimeOffset                    = 0;
    gPTSRdIndex                            = 0;
    gPTSWrIndex                            = 0;

    avc_ResetBuffer();

    //AVC
    if (bFullReset == MMP_TRUE)
    {
        avc_ResetParameterSetList();
        //gptAVCDecoder->currDisplayFrameBufIndex      = 8;
        gptAVCVideoDecoder->aspect_ratio_information = 0x1;

        // shouldn't reset aspect_ratio_information = 0
        //aspect_ratio_information = 0x1;
    }
    else
    {
        avc_ResetFrmGapCheck();
    }

    //avc_ResetBuffer();

    gbStartDecode                      = MMP_FALSE;
    gbPreAVCIDR                        = MMP_FALSE;
    gbFrameEnd                         = MMP_FALSE;
    gbIsFrameOrLastPic                 = MMP_FALSE;
    gbDisplayFirstI                    = MMP_FALSE;
    gbWaitFirstI                       = MMP_FALSE;
    gbIsPauseStcSet                    = MMP_FALSE;

    gbFirstFrame                       = MMP_TRUE;
    gbFirstDecFrame                    = MMP_TRUE;
    gbFrameStart                       = MMP_TRUE;
    gbDropFrame                        = MMP_TRUE;

    gPrevSliceType                     = B_SLICE;
    curSliceFirstMB                    = 9999;

    gptAVCDecoder->cmdDataBufSelect    = 1;
    gptAVCDecoder->cmdDataBufWrSel     = 1;

    gInitFramePoc                      = 0;
    gWrapFramePoc                      = 0;
    gMaxFramePoc                       = 0;
    // gCurUpdateBufIndex = 0xFF;

    gptAVCVideoDecoder->gPreTimeStamp  = 0;

#ifdef SKIP_BROKEN_VIDEO_FRAME
    gtAVCFrameBufControl.fireBufSelect = 0xF;
#endif

    gptAVCVideoDecoder->bHDVideoFormat = MMP_FALSE;
}

static void
_VIDEO_DECODER_SetFrameBufferReference(
    AVCodecContext *avctx)
{
    MMP_UINT8         decodeBufSelect          = 0;
    MMP_UINT32        displayOrderQueue        = gtAVCFrameBufControl.displayOrderQueue;

    MMP_UINT32        timeStamp                = 0;
    MMP_UINT32        lastDisplayFrameBufIndex = displayOrderQueue & 0x000000FF;
    MMP_UINT32        frame_rate_code          = gtAVCFrameBufControl.frame_rate_code;

    MMP_UINT32        accumTime                = gtAVCFrameBufControl.accumTime;
    MMP_INT32         curMinFramePoc           = 0x7FFFFFFF;
    MMP_UINT32        curMinFramePocIndex      = 0xFF;
    MMP_UINT32        curUpdateBufIndex        = 0xFF;
    MMP_UINT32        curFramePoc              = 0;
    MMP_UINT32        clearflipbuf             = 0;
    static MMP_UINT32 preFramePoc              = 0x7FFFFFFF;
    static MMP_INT32  prePoc                   = 0x7FFFFFFF;

    decodeBufSelect                        = img->decoding_buf_idx;

    curUpdateBufIndex                      = decodeBufSelect;
    displayOrderQueue                      = (displayOrderQueue << 8) | curUpdateBufIndex;
    gtAVCFrameBufControl.decodeBufSelect   = decodeBufSelect;
    gtAVCFrameBufControl.displayOrderQueue = displayOrderQueue;

#if defined(MULTI_SLICE)
    clearflipbuf                           = (displayOrderQueue & 0xFF000000) >> 24;
#else
  #if defined(ENABLE_2_VIDEO_FRAME_BUF)
    clearflipbuf                           = (displayOrderQueue & 0xFF00) >> 8;
  #else
    clearflipbuf                           = (displayOrderQueue & 0xFF0000) >> 16;
  #endif
#endif
    if (clearflipbuf != 0xFF)
        avc_release_flip_buf(clearflipbuf);

    av_log(NULL, AV_LOG_DEBUG, "T %d %d: idx %d poc %d cur_poc %d D %d Update %d %08x\n", img->type, img->nal_reference_idc,
           curMinFramePocIndex, curMinFramePoc, curFramePoc, decodeBufSelect, curUpdateBufIndex, displayOrderQueue);
}

#ifdef FF_START_CODE
static MMP_BOOL
ff_next_start_code(
    void)
{
    MMP_UINT8  *pCurrentAddr   = gptAVCVideoDecoder->pPacketStartAddr;
    MMP_UINT8  *pPacketEndAddr = gptAVCVideoDecoder->pPacketEndAddr;
    MMP_UINT8  *pBufferStart   = gptAVCVideoDecoder->pBufferStartAddr;
    MMP_UINT8  *pBufferEnd     = gptAVCVideoDecoder->pBufferEndAddr;
    //MMP_UINT32  remainByte = gptAVCVideoDecoder->remainByte;
    MMP_INT32  remainByte;
    MMP_UINT32 i               = 0;
    MMP_BOOL   result          = MMP_FALSE;
    MMP_UINT32 data            = 0;
    MMP_UINT32 backNum         = 0;
    MMP_UINT32 InitOffset      = 0;
    MMP_UINT32 *pCurrentAddrD  = 0; // 4-bytes alignment
    MMP_UINT32 s_result;
    MMP_UINT32 pBackAddr       = 0;
    MMP_BOOL   bInitFail       = MMP_TRUE;

    if ((MMP_UINT32)pPacketEndAddr >= (MMP_UINT32)pCurrentAddr)
        remainByte = (MMP_INT32)(pPacketEndAddr - pCurrentAddr);
    else
        remainByte = (MMP_INT32)(pBufferEnd - pCurrentAddr +
                                 pPacketEndAddr - pBufferStart);

    if (remainByte < 4)
    {
        //printf("remainByte %d %d %d\n", remainByte, gptAVCVideoDecoder->bEndSegment, ((MMP_UINT32)pCurrentAddr) & 0x3);
        if (gptAVCVideoDecoder->bEndSegment)
            goto end;
        else
            goto getnxt;
    }

init:
    bInitFail  = MMP_FALSE;
    InitOffset = ((MMP_UINT32)pCurrentAddr) & 0x3;

    for (i = 0; i < 4; i++)
    {
        if (i < InitOffset)
            data = (data << 8) | 0xFF;
        else
            data = (data << 8) | *(pCurrentAddr + i - InitOffset);
    }
    remainByte   -= (4 - InitOffset);
    pCurrentAddrD = (MMP_UINT32 *) ((MMP_UINT32)pCurrentAddr - InitOffset);
    pCurrentAddrD++;
    if ((MMP_UINT32)pCurrentAddrD == (MMP_UINT32)pBufferEnd)
    {
        pCurrentAddrD = (MMP_UINT32 *) pBufferStart;
    }

    mtspr(SPR_PATMASK, 0xFFFFFF00);
    mtspr(SPR_PATPAT,  0x00000100);

    if (remainByte < 4)
    {
        mtspr(SPR_PATDATA, data);
        s_result = mfspr(SPR_PATDATA);
        if (s_result == 1)
        {
            backNum = 4;
            result  = MMP_TRUE;
            goto end;
        }
    }
    else
    {
        while (1)
        {
restart:
            mtspr(SPR_PATDATA, data);
            s_result = mfspr(SPR_PATDATA);

            if (s_result == 0)
            {
                if (remainByte < 4)
                    break;

                data = *(pCurrentAddrD++);
                if ((MMP_UINT32)pCurrentAddrD == (MMP_UINT32)pBufferEnd)
                {
                    pCurrentAddrD = (MMP_UINT32 *) pBufferStart;
                }
                remainByte -= 4;
            }
            else
            {
                switch (s_result)
                {
                case 1:
                    backNum = 4;
                    break;

                case 2:
                    backNum = 5;
                    break;

                case 3:
                    backNum = 6;
                    break;

                //case 4:
                default:
                    backNum = 7;
                    break;
                }
                result = MMP_TRUE;
                goto end;
            }
        }
    }

getnxt:
    if (!gptAVCVideoDecoder->bEndSegment)
    {
        VIDEO_SAMPLE *ptVideoSample = MMP_NULL;
        MMP_UINT32   segmentSize;

        if (GetNxtVidSample((void **)&ptVideoSample, gExMode))
        {
            //gptAVCVideoDecoder->pPacketStartAddr = ptVideoSample->pVideoStart;
            gptAVCVideoDecoder->pPacketEndAddr = ptVideoSample->pVideoEnd;
            gptAVCVideoDecoder->bStartSegment  = ptVideoSample->bStartSegment;
            gptAVCVideoDecoder->bEndSegment    = ptVideoSample->bEndSegment;

            if ((MMP_UINT32) ptVideoSample->pVideoStart <= (MMP_UINT32) ptVideoSample->pVideoEnd)
                segmentSize = (MMP_UINT32) (ptVideoSample->pVideoEnd - ptVideoSample->pVideoStart);
            else
                segmentSize = (MMP_UINT32)(ptVideoSample->pRingEnd - ptVideoSample->pVideoStart +
                                           ptVideoSample->pVideoEnd - ptVideoSample->pRingStart);

            remainByte += segmentSize;

            if (remainByte >= 4)
            {
                if (bInitFail)
                    goto init;
                data = *(pCurrentAddrD++);
                if ((MMP_UINT32)pCurrentAddrD == (MMP_UINT32)pBufferEnd)
                {
                    pCurrentAddrD = (MMP_UINT32 *) pBufferStart;
                }
                remainByte -= 4;
                goto restart;
            }
        }
        else
        {
            gptAVCVideoDecoder->bEndSegment = MMP_TRUE;
            goto end;
        }
    }

    // process remainByte < 3 case
    pCurrentAddr = (MMP_UINT8 *)pCurrentAddrD;
    for (i = 0; i < 4; i++)
    {
        if (i < remainByte)
            data = (data << 8) | *(pCurrentAddr + i);
        else
            data = (data << 8) | 0xFF;
    }
    mtspr(SPR_PATDATA, data);
    s_result = mfspr(SPR_PATDATA);

    if (s_result == 2)
    {
        result  = MMP_TRUE;
        backNum = 1;
    }
    else if (s_result == 3)
    {
        result  = MMP_TRUE;
        backNum = 2;
    }
    else if (s_result == 4)
    {
        result  = MMP_TRUE;
        backNum = 3;
    }

end:
    if (MMP_TRUE == result)
    {
        pBackAddr = (MMP_UINT32) pCurrentAddrD - backNum;
        if (pBackAddr < (MMP_UINT32) pBufferStart)
        {
            gptAVCVideoDecoder->pPacketStartAddr = (MMP_UINT8 *) ((MMP_UINT32)pBufferEnd - (backNum - ((MMP_UINT32)pCurrentAddrD - (MMP_UINT32)pBufferStart)));
        }
        else
        {
            gptAVCVideoDecoder->pPacketStartAddr = (MMP_UINT8 *) pBackAddr;
        }
        remainByte                    += backNum;
        gptAVCVideoDecoder->remainByte = remainByte;
    }
    else
    {
        gptAVCVideoDecoder->pPacketStartAddr = gptAVCVideoDecoder->pPacketEndAddr;
        gptAVCVideoDecoder->remainByte       = 0;
    }

    return result;
}
#endif

#if 1 // modified by wlHsu

static MMP_BOOL
next_start_code(
    void)
{
    #if 1
    MMP_UINT8  *pCurrentAddr   = gptAVCVideoDecoder->pPacketStartAddr;
    MMP_UINT8  *pPacketEndAddr = gptAVCVideoDecoder->pPacketEndAddr;
    MMP_UINT8  *pBufferStart   = gptAVCVideoDecoder->pBufferStartAddr;
    MMP_UINT8  *pBufferEnd     = gptAVCVideoDecoder->pBufferEndAddr;
    MMP_UINT32 remainByte      = gptAVCVideoDecoder->remainByte;
    MMP_UINT32 stage           = 0;
    MMP_BOOL   result          = MMP_FALSE;
    MMP_UINT32 clock           = PalGetClock();
    MMP_UINT32 totalSize       = 0;
    MMP_UINT32 value           = 0;
    MMP_INT32  backNum         = 0;
    MMP_INT32  sizeToEnd       = 0;
    MMP_INT32  loopCount       = 0;
    MMP_INT32  remainSizeToEnd = 0;
    MMP_INT32  loopIndex       = 0;
    MMP_UINT32 value0, value1, value2, value3;

    remainByte = _getRemainSizeInRingBuf(
        pBufferStart,
        pPacketEndAddr,
        pBufferEnd,
        pCurrentAddr);
    totalSize = remainByte;

    //printf("start0 ptr: 0x%X\n",pCurrentAddr);

find_start_code:
    while ((((MMP_UINT32)pCurrentAddr) & 0x3))
    {
        switch (stage)
        {
        case 0:
            if (*pCurrentAddr == 0x00)
            {
                stage = 1;
            }
            break;

        case 1:
            if (*pCurrentAddr == 0x00)
            {
                stage = 2;
            }
            else
            {
                stage = 0;
            }
            break;

        case 2:
            if (*pCurrentAddr == 0x01)
            {
                pCurrentAddr -= 2;
                if (pCurrentAddr < pBufferStart)
                {
                    //printf("avc_video_decoder.c(%d) o_cur: 0x%X, n_cur: 0x%X, b_start: 0x%X, b_end: 0x%X\n",
                    //       __LINE__,
                    //       pCurrentAddr, pBufferEnd - (pBufferStart - pCurrentAddr),
                    //       pBufferStart, pBufferEnd);
                    pCurrentAddr = pBufferEnd - (pBufferStart - pCurrentAddr);
                }
                remainByte += 2;
                result      = MMP_TRUE;
                goto end;
            }
            else if (*pCurrentAddr == 0x00)
            {
                stage = 2;
            }
            else
            {
                stage = 0;
            }
            break;

        default:
            break;
        }

        pCurrentAddr = _getNextAddrInRingBuf(pCurrentAddr, pBufferStart, pBufferEnd, 1);
        remainByte--;
    }
    // Make address is double word alignment

    if (!remainByte)
    {
        if (!gptAVCVideoDecoder->bEndSegment)
        {
            gptAVCVideoDecoder->bEndSegment = MMP_TRUE;
            goto end;
        }
    }

    sizeToEnd = (MMP_UINT32) (pBufferEnd - pCurrentAddr);
    if (remainByte < sizeToEnd)
    {
        sizeToEnd = remainByte;
    }
    loopCount       = (sizeToEnd >> 4);
    remainSizeToEnd = sizeToEnd & 0xF;
    for (loopIndex = 0; loopIndex < loopCount; loopIndex++, pCurrentAddr += 16)
    {
        value0 = *((MMP_UINT32 *) pCurrentAddr);
        value1 = *((MMP_UINT32 *) (pCurrentAddr + 4));
        value2 = *((MMP_UINT32 *) (pCurrentAddr + 8));
        value3 = *((MMP_UINT32 *) (pCurrentAddr + 12));

        switch (stage)
        {
        case 0:
value0_stage0:
            if ((value0 & 0xFE000000) && (value0 & 0x00FFFFFF) != 0x00010000)
            {
                break;
            }
            else
            {
                if ((value0 & 0xFFFFFF00) == 0x01000000)
                {
                    pCurrentAddr += 1;
                    result        = MMP_TRUE;
                    goto end;
                }
                else if ((value0 & 0x00FFFFFF) == 0x00010000)
                {
                    result = MMP_TRUE;
                    goto end;
                }
                else if ((value0 & 0xFFFF0000) == 0x00000000)
                {
                    stage = 2;
                }
                else if ((value0 & 0xFF000000) == 0x00000000)
                {
                    stage = 1;
                }
            }
            break;

        case 1:
            if ((value0 & 0x0000FFFF) == 0x00000100)
            {
                pCurrentAddr -= 1;
                result        = MMP_TRUE;
                goto end;
            }
            else if ((value0 & 0x00FFFFFF) == 0x00010000)
            {
                result = MMP_TRUE;
                goto end;
            }
            else if (((value0 & 0xFFFFFF00) == 0x01000000))
            {
                pCurrentAddr += 1;
                result        = MMP_TRUE;
                goto end;
            }
            else if ((value0 & 0xFFFF0000) == 0x00000000)
            {
                stage = 2;
            }
            else if ((value0 & 0xFF000000) == 0x00000000)
            {
                stage = 1;
            }
            else
            {
                stage = 0;
            }
            break;

        case 2:
            if ((value0 & 0x000000FF) == 0x00000001)
            {
                pCurrentAddr -= 2;
                result        = MMP_TRUE;
                goto end;
            }
            else if ((value0 & 0x0000FFFF) == 0x00000100)
            {
                pCurrentAddr -= 1;
                result        = MMP_TRUE;
                goto end;
            }
            else
            {
                stage = 0;
                goto value0_stage0;
            }
            break;

        default:
            break;
        }

        switch (stage)
        {
        case 0:
value1_stage0:
            if ((value1 & 0xFE000000) && (value1 & 0x00FFFFFF) != 0x00010000)
            {
                break;
            }
            else
            {
                if ((value1 & 0xFFFFFF00) == 0x01000000)
                {
                    pCurrentAddr += 5;
                    result        = MMP_TRUE;
                    goto end;
                }
                else if ((value1 & 0x00FFFFFF) == 0x00010000)
                {
                    pCurrentAddr += 4;
                    result        = MMP_TRUE;
                    goto end;
                }
                else if ((value1 & 0xFFFF0000) == 0x00000000)
                {
                    stage = 2;
                }
                else if ((value1 & 0xFF000000) == 0x00000000)
                {
                    stage = 1;
                }
            }
            break;

        case 1:
            if ((value1 & 0x0000FFFF) == 0x00000100)
            {
                pCurrentAddr += 3;
                result        = MMP_TRUE;
                goto end;
            }
            else if ((value1 & 0x00FFFFFF) == 0x00010000)
            {
                pCurrentAddr += 4;
                result        = MMP_TRUE;
                goto end;
            }
            else if (((value1 & 0xFFFFFF00) == 0x01000000))
            {
                pCurrentAddr += 5;
                result        = MMP_TRUE;
                goto end;
            }
            else if ((value1 & 0xFFFF0000) == 0x00000000)
            {
                stage = 2;
            }
            else if ((value1 & 0xFF000000) == 0x00000000)
            {
                stage = 1;
            }
            else
            {
                stage = 0;
            }
            break;

        case 2:
            if ((value1 & 0x000000FF) == 0x00000001)
            {
                pCurrentAddr += 2;
                result        = MMP_TRUE;
                goto end;
            }
            else if ((value1 & 0x0000FFFF) == 0x00000100)
            {
                pCurrentAddr += 3;
                result        = MMP_TRUE;
                goto end;
            }
            else
            {
                stage = 0;
                goto value1_stage0;
            }
            break;

        default:
            break;
        }

        switch (stage)
        {
        case 0:
value2_stage0:
            if ((value2 & 0xFE000000) && (value2 & 0x00FFFFFF) != 0x00010000)
            {
                break;
            }
            else
            {
                if ((value2 & 0xFFFFFF00) == 0x01000000)
                {
                    pCurrentAddr += 9;
                    result        = MMP_TRUE;
                    goto end;
                }
                else if ((value2 & 0x00FFFFFF) == 0x00010000)
                {
                    pCurrentAddr += 8;
                    result        = MMP_TRUE;
                    goto end;
                }
                else if ((value2 & 0xFFFF0000) == 0x00000000)
                {
                    stage = 2;
                }
                else if ((value2 & 0xFF000000) == 0x00000000)
                {
                    stage = 1;
                }
            }
            break;

        case 1:
            if ((value2 & 0x0000FFFF) == 0x00000100)
            {
                pCurrentAddr += 7;
                result        = MMP_TRUE;
                goto end;
            }
            else if ((value2 & 0x00FFFFFF) == 0x00010000)
            {
                pCurrentAddr += 8;
                result        = MMP_TRUE;
                goto end;
            }
            else if (((value2 & 0xFFFFFF00) == 0x01000000))
            {
                pCurrentAddr += 9;
                result        = MMP_TRUE;
                goto end;
            }
            else if ((value2 & 0xFFFF0000) == 0x00000000)
            {
                stage = 2;
            }
            else if ((value2 & 0xFF000000) == 0x00000000)
            {
                stage = 1;
            }
            else
            {
                stage = 0;
            }
            break;

        case 2:
            if ((value2 & 0x000000FF) == 0x00000001)
            {
                pCurrentAddr += 6;
                result        = MMP_TRUE;
                goto end;
            }
            else if ((value2 & 0x0000FFFF) == 0x00000100)
            {
                pCurrentAddr += 7;
                result        = MMP_TRUE;
                goto end;
            }
            else
            {
                stage = 0;
                goto value2_stage0;
            }
            break;

        default:
            break;
        }

        switch (stage)
        {
        case 0:
value3_stage0:
            if ((value3 & 0xFE000000) && (value3 & 0x00FFFFFF) != 0x00010000)
            {
                break;
            }
            else
            {
                if ((value3 & 0xFFFFFF00) == 0x01000000)
                {
                    pCurrentAddr += 13;
                    result        = MMP_TRUE;
                    goto end;
                }
                else if ((value3 & 0x00FFFFFF) == 0x00010000)
                {
                    pCurrentAddr += 12;
                    result        = MMP_TRUE;
                    goto end;
                }
                else if ((value3 & 0xFFFF0000) == 0x00000000)
                {
                    stage = 2;
                }
                else if ((value3 & 0xFF000000) == 0x00000000)
                {
                    stage = 1;
                }
            }
            break;

        case 1:
            if ((value3 & 0x0000FFFF) == 0x00000100)
            {
                pCurrentAddr += 11;
                result        = MMP_TRUE;
                goto end;
            }
            else if ((value3 & 0x00FFFFFF) == 0x00010000)
            {
                pCurrentAddr += 12;
                result        = MMP_TRUE;
                goto end;
            }
            else if (((value3 & 0xFFFFFF00) == 0x01000000)) //[2012/06/28, Evan]
            {
                pCurrentAddr += 13;
                result        = MMP_TRUE;
                goto end;
            }
            else if ((value3 & 0xFFFF0000) == 0x00000000)
            {
                stage = 2;
            }
            else if ((value3 & 0xFF000000) == 0x00000000)
            {
                stage = 1;
            }
            else
            {
                stage = 0;
            }
            break;

        case 2:
            if ((value3 & 0x000000FF) == 0x00000001)
            {
                pCurrentAddr += 10;
                result        = MMP_TRUE;
                goto end;
            }
            else if ((value3 & 0x0000FFFF) == 0x00000100)
            {
                pCurrentAddr += 11;
                result        = MMP_TRUE;
                goto end;
            }
            else
            {
                stage = 0;
                goto value3_stage0;
            }
            break;

        default:
            break;
        }
    }

    // Less than 16 byte, use old state machine to take care such case
    while (remainSizeToEnd)
    {
        switch (stage)
        {
        case 0:
            if (*pCurrentAddr == 0x00)
            {
                stage = 1;
            }
            break;

        case 1:
            if (*pCurrentAddr == 0x00)
            {
                stage = 2;
            }
            else
            {
                stage = 0;
            }
            break;

        case 2:
            if (*pCurrentAddr == 0x01)
            {
                pCurrentAddr -= 2;
                if (pCurrentAddr < pBufferStart)
                {
                    //printf("avc_video_decoder.c(%d) o_cur: 0x%X, n_cur: 0x%X, b_start: 0x%X, b_end: 0x%X\n",
                    //       __LINE__,
                    //       pCurrentAddr, pBufferEnd - (pBufferStart - pCurrentAddr),
                    //       pBufferStart, pBufferEnd);
                    pCurrentAddr = pBufferEnd - (pBufferStart - pCurrentAddr);
                }
                result = MMP_TRUE;
                goto end;
            }
            else if (*pCurrentAddr == 0x00)
            {
                stage = 2;
            }
            else
            {
                stage = 0;
            }
            break;

        default:
            break;
        }

        pCurrentAddr = _getNextAddrInRingBuf(pCurrentAddr, pBufferStart, pBufferEnd, 1);
        remainSizeToEnd--;
    }

    remainByte -= sizeToEnd;
    if (remainByte)
    {
        pCurrentAddr = pBufferStart;
        goto find_start_code;
    }
    else
    {
        if (!gptAVCVideoDecoder->bEndSegment)
        {
            gptAVCVideoDecoder->bEndSegment = MMP_TRUE;
            goto end;
        }
    }

end:
    gptAVCVideoDecoder->pPacketStartAddr = pCurrentAddr;
    if (gptAVCVideoDecoder->pPacketStartAddr > gptAVCVideoDecoder->pPacketEndAddr)
    {
        gptAVCVideoDecoder->remainByte = (MMP_UINT32) (pBufferEnd - gptAVCVideoDecoder->pPacketStartAddr) + (MMP_UINT32) (gptAVCVideoDecoder->pPacketEndAddr - pBufferStart);
    }
    else
    {
        gptAVCVideoDecoder->remainByte = (MMP_UINT32) (gptAVCVideoDecoder->pPacketEndAddr - gptAVCVideoDecoder->pPacketStartAddr);
    }

    if (PalGetDuration(clock) > 5)
    {
        //printf("time: %u, total: %u, runSize: %u, remain: %u, result: %u\n", PalGetDuration(clock), totalSize, totalSize - remainByte, remainByte, result);
    }
    return result;
    #else
    MMP_UINT8  *pCurrentAddr   = gptAVCVideoDecoder->pPacketStartAddr;
    MMP_UINT8  *pPacketEndAddr = gptAVCVideoDecoder->pPacketEndAddr;
    MMP_UINT8  *pBufferStart   = gptAVCVideoDecoder->pBufferStartAddr;
    MMP_UINT8  *pBufferEnd     = gptAVCVideoDecoder->pBufferEndAddr;
    MMP_UINT32 remainByte      = gptAVCVideoDecoder->remainByte;
    MMP_UINT32 stage           = 0;
    MMP_UINT32 i               = 0;
    MMP_UINT32 byteDiff        = 0;
    MMP_BOOL   result          = MMP_FALSE;
    MMP_UINT32 clock           = PalGetClock();
    MMP_UINT32 totalSize       = 0;
    MMP_UINT8  value           = 0;

    remainByte = _getRemainSizeInRingBuf(
        pBufferStart,
        pPacketEndAddr,
        pBufferEnd,
        pCurrentAddr);
    totalSize = remainByte;
    while (remainByte > 0)
    {
        switch (stage)
        {
        case 0:
            if (*pCurrentAddr == 0x00)
                stage = 1;
            break;

        case 1:
            if (*pCurrentAddr == 0x00)
                stage = 2;
            else
                stage = 0;

            break;

        case 2:
            if (*pCurrentAddr == 0x01)
            {
                result = MMP_TRUE;
                goto end;
            }
            else if (*pCurrentAddr == 0x00)
                stage = 2;
            else
                stage = 0;
            break;

        default:
            break;
        }

        pCurrentAddr = _getNextAddrInRingBuf(pCurrentAddr, pBufferStart, pBufferEnd, 1);
        remainByte--;

        if (!gptAVCVideoDecoder->bEndSegment && remainByte == 0)
        {
            VIDEO_SAMPLE *ptVideoSample = MMP_NULL;
            MMP_UINT32   segmentSize;

            //printf("Query New Sample\n");
            if (GetNxtVidSample((void **)&ptVideoSample, gExMode))
            {
                //gptAVCVideoDecoder->pPacketStartAddr = ptVideoSample->pVideoStart;
                gptAVCVideoDecoder->pPacketEndAddr = ptVideoSample->pVideoEnd;
                gptAVCVideoDecoder->bStartSegment  = ptVideoSample->bStartSegment;
                gptAVCVideoDecoder->bEndSegment    = ptVideoSample->bEndSegment;

                // the size of the new packet
                segmentSize                        = _getRemainSizeInRingBuf(
                    ptVideoSample->pRingStart,
                    ptVideoSample->pVideoEnd,
                    ptVideoSample->pRingEnd,
                    ptVideoSample->pVideoStart);
                totalSize  += segmentSize;
                remainByte += segmentSize;
            }
            else
            {
                gptAVCVideoDecoder->bEndSegment = MMP_TRUE;
                //printf("Query Error\n");
                goto end;
            }
        }
    }

end:
    //printf("0X000001 %d %d %x\n", result, remainByte, pCurrentAddr);

    if (result)
    {
        pCurrentAddr = _getPrevAddrInRingBuf(
            pCurrentAddr,
            pBufferStart,
            pBufferEnd,
            2);
        remainByte += 2;
    }

    gptAVCVideoDecoder->pPacketStartAddr = pCurrentAddr;
    gptAVCVideoDecoder->remainByte       = (MMP_INT32) remainByte;
    //if (PalGetDuration(clock) > 5)
    //{
    //    printf("size: %u, %u, result: %u\n", totalSize, PalGetDuration(clock), result);
    //}
    //if (result)
    //{
    //    printf("ptr: 0x%X, 0x%02X%02X%02X, remain: %d, runSize: %u\n", gptAVCVideoDecoder->pPacketStartAddr, gptAVCVideoDecoder->pPacketStartAddr[0], gptAVCVideoDecoder->pPacketStartAddr[1], gptAVCVideoDecoder->pPacketStartAddr[2], gptAVCVideoDecoder->remainByte, totalSize - remainByte);
    //}
    return result;
    #endif
}

#else

static MMP_BOOL
next_start_code(
    void)
{
    MMP_UINT8  *pCurrentAddr   = gptAVCVideoDecoder->pPacketStartAddr;
    MMP_UINT8  *pPacketEndAddr = gptAVCVideoDecoder->pPacketEndAddr;
    MMP_UINT8  *pBufferStart   = gptAVCVideoDecoder->pBufferStartAddr;
    MMP_UINT8  *pBufferEnd     = gptAVCVideoDecoder->pBufferEndAddr;
    MMP_UINT32 remainByte      = gptAVCVideoDecoder->remainByte;
    MMP_UINT32 stage           = 0;
    MMP_UINT32 i               = 0;
    MMP_UINT32 byteDiff        = 0;
    MMP_BOOL   result          = MMP_FALSE;

    if ((MMP_UINT32)pPacketEndAddr >= (MMP_UINT32)pCurrentAddr)
        remainByte = (MMP_UINT32)(pPacketEndAddr - pCurrentAddr);
    else
        remainByte = (MMP_UINT32)(pBufferEnd - pCurrentAddr +
                                  pPacketEndAddr - pBufferStart);

    while (remainByte > 0)
    {
        switch (stage)
        {
        case 0:
            if (pCurrentAddr[i] == 0x00)
                stage = 1;
            break;

        case 1:
            if (pCurrentAddr[i] == 0x00)
                stage = 2;
            else
                stage = 0;
            break;

        case 2:
            if (pCurrentAddr[i] == 0x01)
            {
                result = MMP_TRUE;
                goto end;
            }
            else if (pCurrentAddr[i] == 0x00)
                stage = 2;
            else
                stage = 0;
            break;

        default:
            break;
        }

        if (&pCurrentAddr[i] == (pBufferEnd - 1))
        {
            pCurrentAddr = pBufferStart;
            i            = 0;
        }
        else
            ++i;
        remainByte--;

        if (!gptAVCVideoDecoder->bEndSegment && remainByte == 0)
        {
            VIDEO_SAMPLE *ptVideoSample = MMP_NULL;
            MMP_UINT32   segmentSize;

            //printf("Query New Sample\n");
            if (GetNxtVidSample((void **)&ptVideoSample, gExMode))
            {
                //gptAVCVideoDecoder->pPacketStartAddr = ptVideoSample->pVideoStart;
                gptAVCVideoDecoder->pPacketEndAddr = ptVideoSample->pVideoEnd;
                gptAVCVideoDecoder->bStartSegment  = ptVideoSample->bStartSegment;
                gptAVCVideoDecoder->bEndSegment    = ptVideoSample->bEndSegment;

                if ((MMP_UINT32) ptVideoSample->pVideoStart <= (MMP_UINT32) ptVideoSample->pVideoEnd)
                    segmentSize = (MMP_UINT32) (ptVideoSample->pVideoEnd - ptVideoSample->pVideoStart);
                else
                    segmentSize = (MMP_UINT32)(ptVideoSample->pRingEnd - ptVideoSample->pVideoStart +
                                               ptVideoSample->pVideoEnd - ptVideoSample->pRingStart);

                //printf("SegmentSize %d %x %x %x %x\n", segmentSize, ptVideoSample->pVideoStart, ptVideoSample->pVideoEnd, pBufferStart, pBufferEnd);
                remainByte += segmentSize;
            }
            else
            {
                gptAVCVideoDecoder->bEndSegment = MMP_TRUE;
                //printf("Query Error\n");
                goto end;
            }
        }
    }

end:
    //printf("0X000001 %d %d %x\n", result, remainByte, pCurrentAddr);

    if (result)
    {
        byteDiff = (MMP_UINT32) (&pCurrentAddr[i] - pBufferStart);
        if (byteDiff < 2)
            pCurrentAddr = pBufferEnd - (2 - byteDiff);
        else
            pCurrentAddr = (&pCurrentAddr[i] - 2);

        remainByte += 3;
    }
    else
    {
        result       = MMP_FALSE;
        pCurrentAddr = &pCurrentAddr[i];
    }

    gptAVCVideoDecoder->pPacketStartAddr = pCurrentAddr;
    gptAVCVideoDecoder->remainByte       = (MMP_INT32) remainByte;

    return result;
}
#endif

#if 0
static MMP_BOOL
_is_hd_video_format(
    MMP_UINT32 horizontal_size,
    MMP_UINT32 frame_rate_code)
{
    if (horizontal_size == 1920 ||
        horizontal_size == 1440 ||
        horizontal_size == 1280 ||
        horizontal_size == 960 ||
        ((frame_rate_code == 6 || frame_rate_code == 7 || frame_rate_code == 8) &&
         (horizontal_size == 720 || horizontal_size == 704 || horizontal_size == 640)))
    {
        return MMP_TRUE;
    }
    else
    {
        return MMP_FALSE;
    }
}
#endif

MMP_INLINE static void
_prepare_nal_unit_data(
    MMP_UINT8 *pData,
    MMP_UINT  size)
{
    gptAVCVideoDecoder->pPacketStartAddr         =
        gptAVCVideoDecoder->pSegStartAddr        =
            gptAVCVideoDecoder->pBufferStartAddr = pData;
    gptAVCVideoDecoder->pPacketEndAddr           =
        gptAVCVideoDecoder->pSegEndAddr          =
            gptAVCVideoDecoder->pBufferEndAddr   = pData + size;
    gptAVCVideoDecoder->timeStamp                = 0;
    gptAVCVideoDecoder->remainByte               = size;
    gptAVCVideoDecoder->bStartSegment            = MMP_TRUE;
    gptAVCVideoDecoder->bEndSegment              = MMP_TRUE;
}

static int decode_nal_units(H264Context *h, const MMP_UINT8 *buf, int buf_size)
{
    //MpegEncContext *const s     = &h->s;
    AVCodecContext *const avctx = h->avctx;

    gptAVCVideoDecoder->pPacketStartAddr = buf;
    gptAVCVideoDecoder->pPacketEndAddr   = buf + buf_size;
    gptAVCVideoDecoder->pBufferStartAddr = gptAVCVideoDecoder->pPacketStartAddr;
    gptAVCVideoDecoder->pBufferEndAddr   = gptAVCVideoDecoder->pPacketEndAddr;
    gptAVCVideoDecoder->bStartSegment    = MMP_TRUE;
    gptAVCVideoDecoder->bEndSegment      = MMP_TRUE;
    gptAVCVideoDecoder->pSegStartAddr    = gptAVCVideoDecoder->pPacketStartAddr;
    gptAVCVideoDecoder->pSegEndAddr      = gptAVCVideoDecoder->pPacketEndAddr;
    return AVC_Video_Decoder_Reload(avctx);
}

int AVC_Video_Decoder_DecodeExtradata(H264Context *h, const MMP_UINT8 *buf, int size)
{
    AVCodecContext *avctx = h->avctx;

    if (!buf || size <= 0)
        return -1;

    if (buf[0] == 1)
    {
        int                 i, cnt, nalsize;
        const unsigned char *p = buf;

        h->is_avc                        = 1;
        gptAVCVideoDecoder->bIsNalFormat = MMP_TRUE;

        if (size < 7)
        {
            av_log(avctx, AV_LOG_ERROR, "avcC too short\n");
            return -1;
        }
        /* sps and pps in the avcC always have length coded with 2 bytes,
           so put a fake nal_length_size = 2 while parsing them */
        h->nal_length_size                = 2;
        gptAVCVideoDecoder->NALUnitLength = 2;
        // Decode sps from avcC
        cnt                               = *(p + 5) & 0x1f; // Number of sps
        p                                += 6;
        for (i = 0; i < cnt; i++)
        {
            nalsize = AV_RB16(p) + 2;
            if (nalsize > size - (p - buf))
                return -1;
            if (decode_nal_units(h, p, nalsize) < 0)
            {
                av_log(avctx, AV_LOG_ERROR, "Decoding sps %d from avcC failed\n", i);
                return -1;
            }
            p += nalsize;
        }
        // Decode pps from avcC
        cnt = *(p++); // Number of pps
        for (i = 0; i < cnt; i++)
        {
            nalsize = AV_RB16(p) + 2;
            if (nalsize > size - (p - buf))
                return -1;
            if (decode_nal_units(h, p, nalsize) < 0)
            {
                av_log(avctx, AV_LOG_ERROR, "Decoding pps %d from avcC failed\n", i);
                return -1;
            }
            p += nalsize;
        }
        // Now store right nal length size, that will be use to parse all other nals
        h->nal_length_size                = (buf[4] & 0x03) + 1;
        gptAVCVideoDecoder->NALUnitLength = h->nal_length_size;
    }
    else
    {
        h->is_avc                        = 0;
        gptAVCVideoDecoder->bIsNalFormat = 0;
        if (decode_nal_units(h, buf, size) < 0)
            return -1;
    }
    return 0;
}

unsigned int PalGetClock(void)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0)
        printf("gettimeofday failed!\n");
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

unsigned long PalGetDuration(unsigned int clock)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0)
        printf("gettimeofday failed!\n");
    return (unsigned int)(tv.tv_sec * 1000 + tv.tv_usec / 1000) - clock;
}

int iTE_h264_Dtv_decode_init(AVCodecContext *avctx)
{
    H264Context           *h = avctx->priv_data;
    //MpegEncContext *const s  = &h->s;

    //s->avctx = avctx;
    h->avctx = avctx;
#ifdef DEBUG
    //av_log_set_level(AV_LOG_DEBUG);
#endif

    av_log(avctx, AV_LOG_INFO, "init\n");

    AVC_Video_Decoder_Init(avctx);

    if (avctx->extradata_size > 0 && avctx->extradata &&
        AVC_Video_Decoder_DecodeExtradata(h, avctx->extradata, avctx->extradata_size))
        return -1;

    return 0;
}

int iTE_h264_Dtv_decode_end(AVCodecContext *avctx)
{
    av_log(avctx, AV_LOG_INFO, "decode end\n");
    AVC_Video_Decoder_End();
    return 0;
}

static void print_video_return_state(
    AVCodecContext *avctx,
    MMP_UINT32     state)
{
    av_log(avctx, AV_LOG_DEBUG, "-- video state --\n");
    if (state & VIDEO_RETURN_NORMAL)
        av_log(avctx, AV_LOG_DEBUG, "VIDEO_RETURN_NORMAL\n");
    if (state & VIDEO_RETURN_END_OF_PACKET)
        av_log(avctx, AV_LOG_DEBUG, "VIDEO_RETURN_END_OF_PACKET\n");
    if (state & VIDEO_RETURN_RESOLUTION_NOT_SUPPORT)
        av_log(avctx, AV_LOG_DEBUG, "VIDEO_RETURN_RESOLUTION_NOT_SUPPORT\n");
    if (state & VIDEO_RETURN_SKIP_DECODE)
        av_log(avctx, AV_LOG_DEBUG, "VIDEO_RETURN_SKIP_DECODE\n");
    if (state & VIDEO_RETURN_UNEXPECTED_START_CODE)
        av_log(avctx, AV_LOG_DEBUG, "VIDEO_RETURN_UNEXPECTED_START_CODE\n");
    if (state & VIDEO_RETURN_WAIT_FIRE)
        av_log(avctx, AV_LOG_DEBUG, "VIDEO_RETURN_WAIT_FIRE\n");
    if (state & VIDEO_RETURN_OUT_REF_FRAME_RANGE)
        av_log(avctx, AV_LOG_DEBUG, "VIDEO_RETURN_OUT_REF_FRAME_RANGE\n");
    if (state & VIDEO_RETURN_TOOL_NOT_SUPPORT)
        av_log(avctx, AV_LOG_DEBUG, "VIDEO_RETURN_TOOL_NOT_SUPPORT\n");
    if (state & VIDEO_RETURN_GET_DEC_HEADER)
        av_log(avctx, AV_LOG_DEBUG, "VIDEO_RETURN_GET_DEC_HEADER\n");
    if (state & VIDEO_RETURN_SKIP_DISPLAY_ONCE)
        av_log(avctx, AV_LOG_DEBUG, "VIDEO_RETURN_SKIP_DISPLAY_ONCE\n");
    if (state & VIDEO_RETURN_FILL_BLANK)
        av_log(avctx, AV_LOG_DEBUG, "VIDEO_RETURN_FILL_BLANK\n");
}

int iTE_h264_Dtv_decode_frame(AVCodecContext *avctx, void *data, int *data_size, AVPacket *avpkt)
{
    const MMP_UINT8 *buf           = avpkt->data;
    int             buf_size       = avpkt->size;
    H264Context     *h             = avctx->priv_data;
    //MpegEncContext  *s             = &h->s;
    AVFrame         *pict          = data;

    MMP_UINT32      return_state   = 0;
    VIDEO_SAMPLE    *ptVideoSample = NULL;

    if (h->is_avc && buf_size >= 9 && buf[0] == 1 && buf[2] == 0 && (buf[4] & 0xFC) == 0xFC && (buf[5] & 0x1F) && buf[8] == 0x67)
    {
        int       cnt = buf[5] & 0x1f;
        MMP_UINT8 *p  = buf + 6;
        while (cnt--)
        {
            int nalsize = AV_RB16(p) + 2;
            if (nalsize > buf_size - (p - buf) || p[2] != 0x67)
                goto not_extra;
            p += nalsize;
        }
        cnt = *(p++);
        if (!cnt)
            goto not_extra;
        while (cnt--)
        {
            int nalsize = AV_RB16(p) + 2;
            if (nalsize > buf_size - (p - buf) || p[2] != 0x68)
                goto not_extra;
            p += nalsize;
        }

        return AVC_Video_Decoder_DecodeExtradata(h, buf, buf_size);
    }

not_extra:
    gptAVCVideoDecoder->pPacketStartAddr = avpkt->data;
    gptAVCVideoDecoder->pPacketEndAddr   = avpkt->data + avpkt->size;
    gptAVCVideoDecoder->timeStamp        = 0; // avpkt->pts;
    gptAVCVideoDecoder->pBufferStartAddr = gptAVCVideoDecoder->pPacketStartAddr;
    gptAVCVideoDecoder->pBufferEndAddr   = gptAVCVideoDecoder->pPacketEndAddr;
    gptAVCVideoDecoder->bStartSegment    = MMP_TRUE;
    gptAVCVideoDecoder->bEndSegment      = MMP_TRUE;
    gptAVCVideoDecoder->pSegStartAddr    = gptAVCVideoDecoder->pPacketStartAddr;
    gptAVCVideoDecoder->pSegEndAddr      = gptAVCVideoDecoder->pPacketEndAddr;
    av_log(avctx, AV_LOG_DEBUG, "recv packet size %d 0x%02X %02X %02X %02X %02X %02X %02X\n", avpkt->size,
           avpkt->data[0], avpkt->data[1], avpkt->data[2], avpkt->data[3], avpkt->data[4], avpkt->data[5], avpkt->data[6]);
    while (!(return_state & VIDEO_RETURN_END_OF_PACKET) )
    {
        return_state = AVC_Video_Decoder_Reload(avctx);
        print_video_return_state(avctx, return_state);
        if ((return_state & VIDEO_RETURN_RESOLUTION_NOT_SUPPORT) ||
            (return_state & VIDEO_RETURN_TOOL_NOT_SUPPORT) ||
            (return_state & VIDEO_RETURN_OUT_REF_FRAME_RANGE))
        {
            MMP_BOOL bReturnErr = MMP_TRUE;
            av_log(avctx, AV_LOG_ERROR, "error, return_state = %u\n", return_state);
            break;
        }

        if ( (return_state & VIDEO_RETURN_SKIP_DECODE) ||
             (return_state & VIDEO_RETURN_WAIT_FIRE) )
        {
            //printf("mmV status = 0x%x\n", return_state);
            continue;
        }
        if (return_state & VIDEO_RETURN_SKIP_PACKET)
        {
            av_log(avctx, AV_LOG_ERROR, "return_state \"VIDEO_RETURN_SKIP_PACKET\"\n");
            break;
        }

        // video decoder wait for engine idle
        AVC_Video_Decoder_Wait();

#define TEST_FIREHW_BITRATE 0
#if TEST_FIREHW_BITRATE
        {
            int64_t        st, ct, firedist = 0;
            static int64_t lt3 = 0;
            st       = av_gettime();
            firedist = (st - lt3) / 1000;
            if (firedist > 40)
                printf("##################################################### fire distance %lld\n", firedist);
#endif
        // video decoder fire to decode current reload frame
        if (gbDisplayFirstI)
        {
            if (gbWaitFirstI)
            {
                AVC_Video_Decoder_Fire();
                gbWaitFirstI = MMP_FALSE;
            }
            else
            {
                AVC_Video_Decoder_Flip(LAST_VIDEO_FRAME);
            }
        }
        else
        {
            AVC_Video_Decoder_Fire();
            // video decoder display by isp flip
            AVC_Video_Decoder_Display(avctx, data, data_size);
        }
#if TEST_FIREHW_BITRATE
        lt3 = ct = av_gettime();
    }
#endif
    }

    return (avpkt->size);
}

void iTE_h264_Dtv_decode_flush(AVCodecContext *avctx)
{
    av_log(avctx, AV_LOG_INFO, "flush dpb\n");
    flush_dpb();
}

static const AVProfile profiles[] = {
    { FF_PROFILE_H264_BASELINE,             "Baseline"                              },
    { FF_PROFILE_H264_CONSTRAINED_BASELINE, "Constrained Baseline"                  },
    { FF_PROFILE_H264_MAIN,                 "Main"                                  },
    { FF_PROFILE_H264_EXTENDED,             "Extended"                              },
    { FF_PROFILE_H264_HIGH,                 "High"                                  },
    { FF_PROFILE_H264_HIGH_10,              "High 10"                               },
    { FF_PROFILE_H264_HIGH_10_INTRA,        "High 10 Intra"                         },
    { FF_PROFILE_H264_HIGH_422,             "High 4:2:2"                            },
    { FF_PROFILE_H264_HIGH_422_INTRA,       "High 4:2:2 Intra"                      },
    { FF_PROFILE_H264_HIGH_444,             "High 4:4:4"                            },
    { FF_PROFILE_H264_HIGH_444_PREDICTIVE,  "High 4:4:4 Predictive"                 },
    { FF_PROFILE_H264_HIGH_444_INTRA,       "High 4:4:4 Intra"                      },
    { FF_PROFILE_H264_CAVLC_444,            "CAVLC 4:4:4"                           },
    { FF_PROFILE_UNKNOWN },
};

static const AVOption h264_options[] = {
#if defined(WIN32)
    {"is_avc",          "is avc",          offsetof(H264Context, is_avc),          FF_OPT_TYPE_INT, { 0}, 0, 1, 0},
    {"nal_length_size", "nal_length_size", offsetof(H264Context, nal_length_size), FF_OPT_TYPE_INT, { 0}, 0, 4, 0},
#else
    {"is_avc",          "is avc",          offsetof(H264Context, is_avc),          FF_OPT_TYPE_INT, {.dbl = 0}, 0, 1, 0},
    {"nal_length_size", "nal_length_size", offsetof(H264Context, nal_length_size), FF_OPT_TYPE_INT, {.dbl = 0}, 0, 4, 0},
#endif
    {NULL}
};

static const AVClass h264_class = {
    "H264 Decoder",
    av_default_item_name,
    h264_options,
    LIBAVUTIL_VERSION_INT,
};

#if CONFIG_ITE_H264_DTV_DECODER
    #if defined(_MSC_VER)
AVCodec ff_iTE_h264_Dtv_decoder = {
    "iTE_h264_Dtv",
    AVMEDIA_TYPE_VIDEO,
    CODEC_ID_H264,
    sizeof(H264Context),
    iTE_h264_Dtv_decode_init,
    NULL,
    iTE_h264_Dtv_decode_end,
    iTE_h264_Dtv_decode_frame,
    /*CODEC_CAP_DRAW_HORIZ_BAND |*/ CODEC_CAP_DR1 | CODEC_CAP_DELAY |
    CODEC_CAP_SLICE_THREADS | CODEC_CAP_FRAME_THREADS,
    NULL,
    iTE_h264_Dtv_decode_flush /*flush_dpb*/,
    NULL,
    NULL,
    "H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10",
    NULL,
    NULL,
    NULL,
    0,
    &h264_class/*,
                  NULL_IF_CONFIG_SMALL(profiles),
                  ONLY_IF_THREADS_ENABLED(decode_init_thread_copy),
                  ONLY_IF_THREADS_ENABLED(decode_update_thread_context),*/
};
    #else // !defined (_MSC_VER)
AVCodec ff_iTE_h264_Dtv_decoder = {
    .name           = "iTE_h264_Dtv",
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = CODEC_ID_H264,
    .priv_data_size = sizeof(H264Context),
    .init           = iTE_h264_Dtv_decode_init,
    .close          = iTE_h264_Dtv_decode_end,
    .decode         = iTE_h264_Dtv_decode_frame,
    .capabilities   = /*CODEC_CAP_DRAW_HORIZ_BAND |*/ CODEC_CAP_DR1 | CODEC_CAP_DELAY |
                      CODEC_CAP_SLICE_THREADS | CODEC_CAP_FRAME_THREADS,
    .flush          = iTE_h264_Dtv_decode_flush,
    .long_name      = "H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10",
    .priv_class     = &h264_class,
};
    #endif // _MSC_VER
#endif     // CONFIG_ITE_H264_DTV_DECODER