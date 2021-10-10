#ifndef __jpg_config_H_StehhLHK_1Lo9_1pD8_AwcE_TqmdJqv4L1sJ__
#define __jpg_config_H_StehhLHK_1Lo9_1pD8_AwcE_TqmdJqv4L1sJ__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================

#define ENABLE_MOTION_JPG       1//0  //Benson
#define ENABLE_JPG_ENC          1

#define CONFIG_JPG_CODEC_DESC_DECODER_DESC         1
#define CONFIG_JPG_CODEC_DESC_DEC_JPG_CMD_DESC     1

#if (ENABLE_JPG_ENC)
    #define CONFIG_JPG_CODEC_DESC_ENCODER_DESC    1
#else
    #define CONFIG_JPG_CODEC_DESC_ENCODER_DESC    0
#endif

#if (ENABLE_MOTION_JPG)
    #define CONFIG_JPG_CODEC_DESC_DEC_MJPG_DESC    1

    #if (ENABLE_JPG_ENC)
        #define CONFIG_JPG_CODEC_DESC_ENC_MJPG_DESC    1
    #else
        #define CONFIG_JPG_CODEC_DESC_ENC_MJPG_DESC    0
    #endif
#else
    #define CONFIG_JPG_CODEC_DESC_DEC_MJPG_DESC    0
    #define CONFIG_JPG_CODEC_DESC_ENC_MJPG_DESC    0
#endif

//#if defined(CFG_DOORBELL_INDOOR) || defined(CFG_DOORBELL_ADMIN) || defined(CFG_CAPTURE_MODULE_ENABLE)
//#define TILING_MODE_OFF
//#endif
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

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
