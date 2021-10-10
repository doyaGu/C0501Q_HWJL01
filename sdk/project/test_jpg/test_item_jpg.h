#ifndef __test_items_jpg_H_hzsk6U8p_7lrP_ROqP_Bb4K_PDEsjZBufTjz__
#define __test_items_jpg_H_hzsk6U8p_7lrP_ROqP_Bb4K_PDEsjZBufTjz__

#ifdef __cplusplus
extern "C" {
#endif


#include "io_api.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
typedef enum _JPEG_ENC_IN_TYPE_TAG
{
    JPEG_ENC_IN_UNKNOW     = 0,
    JPEG_ENC_IN_FILE,
    JPEG_ENC_IN_MEM,
    JPEG_ENC_IN_RGB,
    JPEG_ENC_IN_YUV,
    
}JPEG_ENC_IN_TYPE;
//=============================================================================
//				  Macro Definition
//=============================================================================

//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct _JPEG_ENC_PARAM_TAG
{
    int                 bWithIsp;
    int                 outWidth;
    int                 outHeight;
    
    JPEG_ENC_IN_TYPE    jEncSrcType;
    
    // in
    BASE_STRM_INFO      input;
        // raw data
    unsigned char       *rgbAddr;
    uint32_t            rgbLength;
    uint32_t            rgb_w;
    uint32_t            rgb_h;
    uint32_t            rgb_pitch;
    
    // out
    BASE_STRM_INFO      output;
    
}JPEG_ENC_PARAM;
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
test_jpeg_dec_withIsp(
    unsigned char   *jpegStream,
    unsigned int    streamLength,
    char            *filename);
   
void
test_jpeg_enc(
   uint32_t        in_w,
   uint32_t        in_h,
   uint8_t         *pAddr_y,
   uint8_t         *pAddr_u,
   uint8_t         *pAddr_v,
   bool            bYuv422,
   char            *encName);


void
test_jpeg_enc_clip(
	uint32_t		start_x,
	uint32_t		start_y,
	uint32_t		in_w,
	uint32_t		in_h,
	uint32_t		pitch_Y,
	uint8_t 		*pAddr_y,
	uint8_t 		*pAddr_u,
	uint8_t 		*pAddr_v,
	char			*encName);


void
test_jpeg_dec_to_yuv(
    unsigned char   *jpegStream,
    unsigned int    streamLength,
    char            *filename);

void
test_jpeg_dec_enc(
   char            *decName,
   char            *encName);

    

#ifdef __cplusplus
}
#endif

#endif
