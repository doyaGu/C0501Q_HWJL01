#ifndef __test_items_H_lKsRz0pw_fH58_wZEP_7rJE_jvdQ3tNkHqMV__
#define __test_items_H_lKsRz0pw_fH58_wZEP_7rJE_jvdQ3tNkHqMV__

#ifdef __cplusplus
extern "C" {
#endif

#include "test_item_isp.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
typedef enum JPEG_ENC_COLOR_SPACE_TAG
{
    JPEG_ENC_UNKNOW         = 0,
    JPEG_ENC_YUV_420,
    JPEG_ENC_YUV_422,

}JPEG_ENC_COLOR_SPACE;
//=============================================================================
//				  Macro Definition
//=============================================================================

//=============================================================================
//				  Structure Definition
//=============================================================================

//=============================================================================
//				  Global Data Definition
//=============================================================================

//=============================================================================
//				  Private Function Definition
//=============================================================================

//=============================================================================
//				  Public Function Definition
//=============================================================================
void
item_lcd(
    void);

void
item_isp_color_trans(
    void                *inInfo,
    DATA_COLOR_TYPE     colorType,
    int                 rawWidth,
    int                 rawHeight);

void
item_jpeg_decoder(
    void  *inInfo);

void
item_jpeg_isp_dec_fileName(
    char  *path);

void
item_jpeg_yuv_fileName(
    char    *path);

void
item_jpeg_encoder(
    void                 *inInfo,
    int                  rawWidth,
    int                  rawHeight,
    JPEG_ENC_COLOR_SPACE colorSpace,
    char                 *encName);

void
item_jpeg_codec(
    char            *decName,
    char            *encName);

void
item_play_video(
    void                *inInfo,
    void                *uiInfo_00,
    void                *uiInfo_01,
    int                 rawWidth,
    int                 rawHeight);



#ifdef __cplusplus
}
#endif

#endif
