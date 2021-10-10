#ifndef __jpg_codec_H_6qzgpmlF_jw4Q_nRjM_0Gcy_TzQTSR7BMtP5__
#define __jpg_codec_H_6qzgpmlF_jw4Q_nRjM_0Gcy_TzQTSR7BMtP5__

#ifdef __cplusplus
extern "C" {
#endif


#include "jpg_types.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum JCODEC_CMD_TAG
{
    JCODEC_CMD_UNKNOW      = 0,
    JCODEC_CMD_WAIT_IDLE,

}JCODEC_CMD;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
struct JPG_CODEC_HANDLE_TAG;
/**
 * jpg codec descriptor
 **/
typedef struct JPG_CODEC_DESC_TAG
{
    char              *codecName;

    struct JPG_CODEC_DESC_TAG  *next;
    JPG_CODEC_TYPE       id;

    JPG_ERR (*init)(struct JPG_CODEC_HANDLE_TAG *pHJCodec, void *extraData);
    JPG_ERR (*deInit)(struct JPG_CODEC_HANDLE_TAG *pHJCodec, void *extraData);
    JPG_ERR (*setup)(struct JPG_CODEC_HANDLE_TAG *pHJCodec, void *extraData);
    JPG_ERR (*fire)(struct JPG_CODEC_HANDLE_TAG *pHJCodec, void *extraData);
    JPG_ERR (*control)(struct JPG_CODEC_HANDLE_TAG *pHJCodec, uint32_t cmd, uint32_t *value, void *extraData);

}JPG_CODEC_DESC;

/**
 * Jpg codec handle
 **/
typedef struct JPG_CODEC_HANDLE_TAG
{
    JPG_CODEC_DESC      jCodecDesc;

    bool                bInitialed;

    JPG_STREAM_HANDLE   *pHInJStream;
    JPG_STREAM_HANDLE   *pHOutJStream;

    uint8_t             *pSysBsBuf;
    uint32_t            sysBsBufSize;
    uint32_t            sysValidBsBufSize;

    // decode info
    void                *pHJComm;

    // for mjpg encode
    bool                bSkipPreSetting;

    // H/W
    uint32_t            ctrlFlag;
    bool                bLastSection;
    JPG_HW_CTRL         jHwCtrl;
    JPG_HW_BS_CTRL      jHwBsCtrl;
    JPG_LINE_BUF_INFO   jLineBufInfo;
    JPG_FRM_SIZE_INFO   jFrmSizeInfo;
    JPG_FRM_COMP        jFrmCompInfo;
    JPG_SHARE_DATA      jShare2Isp;

    JPG_MULTI_SECT_INFO jMultiSectInfo;

    JPG_YUV_TO_RGB      *yuv2RgbMatrix;

    void                *privData;
}JPG_CODEC_HANDLE;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
