#ifndef __ite_jpg_H_zKfqvZ2P_tbXg_IQcC_FryT_GeCb2KZmdsYN__
#define __ite_jpg_H_zKfqvZ2P_tbXg_IQcC_FryT_GeCb2KZmdsYN__

#ifdef __cplusplus
extern "C" {
#endif

#include "jpg_stream.h"
#include "jpg_debug.h"

/** @defgroup ite_jpg JPG
 *  @{
 */

//=============================================================================
//                  Constant Definition
//=============================================================================
typedef void*   HJPG;

/**
 * jpg H/W support type
 **/
typedef enum JPG_CODEC_TYPE_TAG
{
    JPG_CODEC_UNKNOW     = 0,
    JPG_CODEC_DEC_JPG,
    JPG_CODEC_ENC_JPG,
    JPG_CODEC_DEC_MJPG,
    JPG_CODEC_ENC_MJPG,
    JPG_CODEC_DEC_JPG_CMD,
    JPG_CODEC_DEC_JPROG, // jpg progressive only primary

}JPG_CODEC_TYPE;

typedef enum JPG_STATUS_TAG
{
    JPG_STATUS_IDLE = 0x11,
    JPG_STATUS_BUSY = 0xBB,
    JPG_STATUS_FAIL = 0xFF,

}JPG_STATUS;

/**
 * Jpg decode type (Primary/ small Thumbnail/ lagre Thumbnail)
 **/
typedef enum JPG_DEC_TYPE_TAG
{
    JPG_DEC_PRIMARY      = 0,
    JPG_DEC_SMALL_THUMB,
    JPG_DEC_LARGE_THUMB,
    JPG_DEC_TYPE_COUNT

} JPG_DEC_TYPE;

/**
 * color component definition
 */
typedef enum COLOR_COMP_TYPE_TAG
{
    COLOR_COMP_NONE         = 0,
    COLOR_COMP_HUE          = (0x1 << 1),   // range is 0 ~ 359, and default = 0.
    COLOR_COMP_CONTRAST     = (0x1 << 2),   // range is -64 ~ 63, and default = 0.
    COLOR_COMP_SATURATION   = (0x1 << 3),   // range is 0 ~ 255, and default = 128.
    COLOR_COMP_BRIGHTNESS   = (0x1 << 4),   // range is -64 ~ 63, and default = 0,
    COLOR_COMP_ALL          = 0xFFFFFFFF,

}COLOR_COMP_TYPE;

/**
 *  JPG Color space
 */
typedef enum JPG_COLOR_SPACE_TAG
{
    JPG_COLOR_SPACE_UNKNOW      = 0,
    JPG_COLOR_SPACE_YUV444,   // plant mode
    JPG_COLOR_SPACE_YUV422,   // plant mode
    JPG_COLOR_SPACE_YUV420,   // plant mode
    JPG_COLOR_SPACE_YUV411,   // plant mode
    JPG_COLOR_SPACE_YUV422R,  // plant mode
    JPG_COLOR_SPACE_RGB565,   // packet mode
    JPG_COLOR_SPACE_ARGB8888, // packet mode
    JPG_COLOR_SPACE_ARGB4444, // packet mode

} JPG_COLOR_SPACE;

/**
 *  JPG rotation angle
 */
typedef enum JPG_ROT_TYPE_TAG
{
    JPG_ROT_TYPE_0     = 0,
    JPG_ROT_TYPE_90,
    JPG_ROT_TYPE_180,
    JPG_ROT_TYPE_270,

}JPG_ROT_TYPE;

typedef enum JPG_DISP_MODE_TAG
{
    JPG_DISP_UNKNOW     = 0,
    JPG_DISP_CENTER,
    JPG_DISP_FIT,
    JPG_DISP_CUT,
    JPG_DISP_CUT_PART,

}JPG_DISP_MODE;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct JPG_RECT_TAG
{
    int         x, y;
    uint32_t    w, h;
}JPG_RECT;

/**
 * Decode Color Effect
 */
typedef struct JPG_COLOR_CTRL_TAG
{
    bool               bColorCtl;  // Enable color control or not.
    COLOR_COMP_TYPE    ctrlFlag;   // flag for setting type

    int                hue;
    int                contrast;
    int                saturation;
    int                brightness;

}JPG_COLOR_CTRL;

/**
 * jpg output info (display setting)
 */
typedef struct JPG_DISP_INFO_TAG
{
    // Source information (jpeg source)
    int               srcX;
    int               srcY;
    uint32_t          srcW;
    uint32_t          srcH;

    // Destination information
    int               dstX;
    int               dstY;
    uint32_t          dstW;
    uint32_t          dstH;

    // base
    JPG_COLOR_SPACE   outColorSpace;
    //JPG_COLOR_SPACE   inColorSpace;

    // Some special process of decoding.
    JPG_ROT_TYPE      rotType;
    bool              bCMYK;
    uint32_t          orientation;  // from exif

    JPG_COLOR_CTRL    colorCtrl;

}JPG_DISP_INFO;


/**
 * jpg initial parameters whe create handle
 **/
typedef struct JPG_INIT_PARAM_TAG
{
    JPG_CODEC_TYPE      codecType;

    // base info
    JPG_COLOR_SPACE     outColorSpace;
    uint32_t            width;  // deocding => output width, encoding => input and output width (1:1 scaling)
    uint32_t            height; // deocding => output hgitht, encoding => input and output hgitht (1:1 scaling)
    JPG_DEC_TYPE        decType; // thumbnail or primary
    JPG_ROT_TYPE        rotType;
    JPG_COLOR_CTRL      colorCtl; // for Hue, Contrast, Saturation, Brightness.
    JPG_DISP_MODE       dispMode;

    uint32_t            encQuality; // for encode jpg

    // special case setting
    bool                bNeedSleep;        // for content switch
    bool                bExifParsing;
    bool                bExifOrientation;  // enable rotating with orientation of exif
    uint32_t            keepPercentage;    // reserved percentage of jpeg

}JPG_INIT_PARAM;

/**
 * buffer info
 **/
typedef struct JPG_BUF_INFO_TAG
{
    uint8_t         *pBufAddr;
    uint32_t        bufLength;
    uint32_t        pitch;

}JPG_BUF_INFO;

/**
 * jpg user info
 **/
typedef struct JPG_USER_INFO_TAG
{
    JPG_STATUS      status;
    JPG_RECT        jpgRect;

    uint32_t        real_width;
    uint32_t        real_height;
    uint32_t        slice_num;

    uint32_t        imgHeight;
    uint32_t        imgWidth;

    uint32_t        comp1Pitch;
    uint32_t        comp23Pitch;

    JPG_COLOR_SPACE     colorFormate;

    // exif

}JPG_USER_INFO;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
#if !(JPG_CHECK_CALLER)

    /**
    * Create the handle of ITE_jpg.
    *
    * @param set iteJpg handle to contact the whole Jpeg driver.
    * @param set initial parameter to iteJpg handle.
    * @param void parameter for extra use.
    */
    JPG_ERR
    iteJpg_CreateHandle(
        HJPG            **pHJpeg,
        JPG_INIT_PARAM  *pInitParam,
        void            *extraData);


    /**
     * Destroy the handle of ITE_jpg.
     *
     * @param destroy iteJpg handle.
     * @param void parameter for extra use.
     */        
    JPG_ERR
    iteJpg_DestroyHandle(
        HJPG            **pHJpeg,
        void            *extraData);


    /**
    * Set Input/Output stream info to  iteJpg handle.
    *
    * @param set iteJpg handle.
    * @param set Input stream to iteJpg handle.
    * @param set Output stream to iteJpg handle.
    * @param void parameter for extra use.
    */    
    JPG_ERR
    iteJpg_SetStreamInfo(
        HJPG            *pHJpeg,
        JPG_STREAM_INFO *pInStreamInfo,
        JPG_STREAM_INFO *pOutStreamInfo,
        void            *extraData);


    /**
    * iteJpg decoder parser.
    *
    * @param set iteJpg handle.
    * @param get the entropy buffer to decode jpg bitstream.
    * @param void parameter for extra use.
    */       
    JPG_ERR
    iteJpg_Parsing(
        HJPG            *pHJpeg,
        JPG_BUF_INFO    *pEntropyBufInfo,
        void            *extraData);

    /**
    * According to the Jpeg codecType , initial bit stream buffer to prepare Jpeg encode/decode.
    *
    * @param set iteJpg handle.
    * @param void parameter for extra use.
    */ 
    JPG_ERR
    iteJpg_Setup(
        HJPG            *pHJpeg,
        void            *extraData);

    /**
    * According to the Jpeg codecType , fire Jpeg engine to encode/decode jpeg file and return the encode size.
    *
    * @param set iteJpg handle.
    * @param set the encode/decode entropy buffer to Jpeg engine.
    * @param return the jpeg encoder size.
    * @param void parameter for extra use.
    */ 
    JPG_ERR
    iteJpg_Process(
        HJPG            *pHJpeg,
        JPG_BUF_INFO    *pStreamBufInfo,
        uint32_t        *pJpgSize,
        void            *extraData);

    /**
    * calculate Jpeg resolution to fit the real case.
    *
    * @param set iteJpg handle.
    * @param set input width  to fit the user display requirement.
    * @param set input height to fit the user display requirement.
    * @param void parameter for extra use.
    */ 
    JPG_ERR
    iteJpg_SetBaseOutInfo(
        HJPG            *pHJpeg,
        uint32_t        *pWidth,
        uint32_t        *pHeight,
        void            *extraData);

    /**
    * Reset Jpeg input setting.
    *
    * @param set iteJpg handle.
    * @param void parameter for extra use.
    */ 
    JPG_ERR
    iteJpg_Reset(
        HJPG            *pHJpeg,
        void            *extraData);

    /**
    * Wait Jpeg engine to idle.
    *
    * @param set iteJpg handle.
    * @param void parameter for extra use.
    */ 
    JPG_ERR
    iteJpg_WaitIdle(
        HJPG            *pHJpeg,
        void            *extraData);
#else
    // for debug
    JPG_ERR iteJpg_CreateHandle_dbg(HJPG **pHJpeg, JPG_INIT_PARAM *pInitParam, void *extraData JPG_EXTRA_INFO);
    JPG_ERR iteJpg_DestroyHandle_dbg(HJPG **pHJpeg, void *extraData JPG_EXTRA_INFO);
    JPG_ERR iteJpg_SetStreamInfo_dbg(HJPG *pHJpeg, JPG_STREAM_INFO *pInStreamInfo, JPG_STREAM_INFO *pOutStreamInfo, void *extraData JPG_EXTRA_INFO);
    JPG_ERR iteJpg_Parsing_dbg(HJPG *pHJpeg, JPG_BUF_INFO *pEntropyBufInfo, void *extraData JPG_EXTRA_INFO);
    JPG_ERR iteJpg_Setup_dbg(HJPG *pHJpeg, void *extraData JPG_EXTRA_INFO);
    JPG_ERR iteJpg_Process_dbg(HJPG *pHJpeg, JPG_BUF_INFO *pStreamBufInfo, uint32_t *pJpgSize, void *extraData JPG_EXTRA_INFO);
    JPG_ERR iteJpg_SetBaseOutInfo_dbg(HJPG *pHJpeg, uint32_t *pWidth, uint32_t *pHeight, void *extraData JPG_EXTRA_INFO);
    JPG_ERR iteJpg_Reset_dbg(HJPG *pHJpeg, void *extraData JPG_EXTRA_INFO);
    JPG_ERR iteJpg_WaitIdle_dbg(HJPG *pHJpeg, void *extraData JPG_EXTRA_INFO);

    #define iteJpg_CreateHandle(pHJpeg, pInitParam, extraData)                      iteJpg_CreateHandle_dbg(pHJpeg, pInitParam, extraData jpg_extra_param)
    #define iteJpg_DestroyHandle(pHJpeg, extraData)                                 iteJpg_DestroyHandle_dbg(pHJpeg, extraData jpg_extra_param)
    #define iteJpg_SetStreamInfo(pHJpeg, pInStreamInfo, pOutStreamInfo, extraData)  iteJpg_SetStreamInfo_dbg(pHJpeg, pInStreamInfo, pOutStreamInfo, extraData jpg_extra_param)
    #define iteJpg_Parsing(pHJpeg, pEntropyBufInfo, extraData)                      iteJpg_Parsing_dbg(pHJpeg, pEntropyBufInfo, extraData jpg_extra_param)
    #define iteJpg_Setup(pHJpeg, extraData)                                         iteJpg_Setup_dbg(pHJpeg, extraData jpg_extra_param)
    #define iteJpg_Process(pHJpeg, pStreamBufInfo, pJpgSize, extraData)             iteJpg_Process_dbg(pHJpeg, pStreamBufInfo, pJpgSize, extraData jpg_extra_param)
    #define iteJpg_SetBaseOutInfo(pHJpeg, pWidth, pHeight, extraData)               iteJpg_SetBaseOutInfo_dbg(pHJpeg, pWidth, pHeight, extraData jpg_extra_param)
    #define iteJpg_Reset(pHJpeg, extraData)                                         iteJpg_Reset_dbg(pHJpeg, extraData jpg_extra_param)
    #define iteJpg_WaitIdle(pHJpeg, extraData)                                      iteJpg_WaitIdle_dbg(pHJpeg, extraData jpg_extra_param)
#endif

/**
* Get the Jpeg status to understand Jpeg encode/decode success or not.
*
* @param set iteJpg handle.
* @param get the jpeg user info.
* @param void parameter for extra use.
*/ 
JPG_ERR
iteJpg_GetStatus(
    HJPG            *pHJpeg,
    JPG_USER_INFO   *pJpgUserInfo,
    void            *extraData);

/** @} */ // end of ite_jpg

#ifdef __cplusplus
}
#endif

#endif
