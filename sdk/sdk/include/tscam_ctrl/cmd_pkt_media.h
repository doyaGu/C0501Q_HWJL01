#ifndef __cmd_packet_media_H_hBeFCCmB_uYoI_HqJl_FglM_EjogyuMW4bqg__
#define __cmd_packet_media_H_hBeFCCmB_uYoI_HqJl_FglM_EjogyuMW4bqg__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmd_data_type.h"
//=============================================================================
//                Constant Definition
//=============================================================================
/**
 * media service
 **/
#define MEDIA_SERVICE_TAG_MASK              0xFF00
#define MEDIA_SERVICE_CMD_MASK              0x00FF
#define MEDIA_INPUT_SERVICE_TAG             0x0400
#define MEDIA_OUTPUT_SERVICE_TAG            0x8400
// cmd
#define CMD_GetProfilesInput                                         0x0400
#define CMD_GetProfilesOutput                                        0x8400
#define CMD_GetVideoSourcesInput                                     0x0401
#define CMD_GetVideoSourcesOutput                                    0x8401
#define CMD_GetVideoSourceConfigurationsInput                        0x0402
#define CMD_GetVideoSourceConfigurationsOutput                       0x8402
#define CMD_GetGuaranteedNumberOfVideoEncoderInstancesInput          0x0403
#define CMD_GetGuaranteedNumberOfVideoEncoderInstancesOutput         0x8403
#define CMD_GetVideoEncoderConfigurationsInput                       0x0404
#define CMD_GetVideoEncoderConfigurationsOutput                      0x8404
#define CMD_GetAudioSourcesInput                                     0x0405
#define CMD_GetAudioSourcesOutput                                    0x8405
#define CMD_GetAudioSourceConfigurationsInput                        0x0406
#define CMD_GetAudioSourceConfigurationsOutput                       0x8406
#define CMD_GetAudioEncoderConfigurationsInput                       0x0407
#define CMD_GetAudioEncoderConfigurationsOutput                      0x8407
#define CMD_SetSynchronizationPointInput                             0x0480
#define CMD_SetSynchronizationPointOutput                            0x8480
#define CMD_SetVideoSourceConfigurationInput                         0x0482
#define CMD_SetVideoSourceConfigurationOutput                        0x8482
#define CMD_SetVideoEncoderConfigurationInput                        0x0484
#define CMD_SetVideoEncoderConfigurationOutput                       0x8484
#define CMD_SetAudioSourceConfigurationInput                         0x0486
#define CMD_SetAudioSourceConfigurationOutput                        0x8486
#define CMD_SetAudioEncoderConfigurationInput                        0x0487
#define CMD_SetAudioEncoderConfigurationOutput                       0x8487

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * GetProfiles Input context
 **/
typedef struct _byte_align4 GetProfilesInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetProfilesInput_INFO;

/**
 * GetProfiles Output context
 **/
typedef struct _byte_align4 PROFILES_DATA_T
{
    CMD_STRING_T    name;
    CMD_STRING_T    token;
    CMD_STRING_T    video_source_configuration_name;
    CMD_STRING_T    video_source_configruation_token;
    CMD_STRING_T    video_source_configruation_sourcetoken;
    uint8_t         video_source_configuration_usecount;
    uint16_t        video_source_configuration_bounds_x;
    uint16_t        video_source_configuration_bounds_y;
    uint16_t        video_source_configuration_bounds_width;
    uint16_t        video_source_configuration_bounds_height;
    uint8_t         video_source_configuration_rotate_mode;
    uint16_t        video_source_configuration_rotate_degree;
    uint8_t         video_source_configuration_mirror_mode;
    CMD_STRING_T    audio_source_configuration_name;
    CMD_STRING_T    audio_source_configuration_token;
    CMD_STRING_T    audio_source_configuration_sourcetoken;
    uint8_t         audio_source_configuration_usecount;
    CMD_STRING_T    video_encoder_configuration_name;
    CMD_STRING_T    video_encoder_configuration_token;
    uint8_t         video_encoder_configuration_usecount;
    uint8_t         video_encoder_configuration_encoding_mode;
    uint16_t        video_encoder_configuration_resolution_width;
    uint16_t        video_encoder_configuration_resolution_height;
    uint8_t         video_encoder_configuration_quality;
    uint8_t         video_encoder_configuration_ratecontrol_frameratelimit;
    uint8_t         video_encoder_configuration_ratecontrol_encodinginterval;
    uint16_t        video_encoder_configuration_ratecontrol_bitratelimit;
    uint8_t         video_encoder_configuration_ratecontroltype;
    uint8_t         video_encoder_configuration_govlength;
    uint8_t         video_encoder_configuration_profile;
    CMD_STRING_T    audio_encoder_configuration_name;
    CMD_STRING_T    audio_encoder_configuration_token;
    uint8_t         audio_encoder_configuration_usecount;
    uint8_t         audio_encoder_configuration_encoding;
    uint16_t        audio_encoder_configuration_bitrate;
    uint16_t        audio_encoder_configuration_samplerate;
    CMD_STRING_T    audio_output_configuration_name;
    CMD_STRING_T    audio_output_configuration_token;
    CMD_STRING_T    audio_output_configuration_outputtoken;
    uint8_t         audio_output_configuration_usecount;
    uint8_t         audio_output_configuration_sendprimacy;
    uint8_t         audio_output_configuration_outputlevel;
    CMD_STRING_T    audio_decoder_name;
    CMD_STRING_T    audio_decoder_token;
    uint8_t         audio_decoder_usecount;

}PROFILES_DATA;

typedef struct _byte_align4 GetProfilesOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         profiles_list_size;
    PROFILES_DATA   *pProfiles_data;

}GetProfilesOutput_INFO;

/**
 * GetVideoSources Input context
 **/
typedef struct _byte_align4 GetVideoSourcesInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetVideoSourcesInput_INFO;

/**
 * GetVideoSources Output context
 **/
typedef struct _byte_align4 VIDEO_SOURCES_DATA_T
{
    CMD_STRING_T    token;
    uint8_t         frame_rate;
    uint16_t        resolution_width;
    uint16_t        resolution_height;

}VIDEO_SOURCES_DATA;

typedef struct _byte_align4 GetVideoSourcesOutput_INFO_T
{
    uint16_t                cmd_code;
    uint8_t                 return_code;
    CMD_STRING_T            user_name;
    CMD_STRING_T            password;
    uint8_t                 video_sources_list_size;
    VIDEO_SOURCES_DATA      *pVideo_sources_dat;

}GetVideoSourcesOutput_INFO;

/**
 * GetVideoSourceConfigurations Input context
 **/
typedef struct _byte_align4 GetVideoSourceConfigurationsInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetVideoSourceConfigurationsInput_INFO;

/**
 * GetVideoSourceConfigurations Output context
 **/
typedef struct _byte_align4 VIDEO_CONFIGURATIONS_T
{
    CMD_STRING_T    name;
    CMD_STRING_T    token;
    CMD_STRING_T    source_token;
    uint8_t         use_count;
    uint16_t        bounds_x;
    uint16_t        bounds_y;
    uint16_t        bounds_width;
    uint16_t        bounds_height;
    uint8_t         rotate_mode;
    uint16_t        rotate_degree;
    uint8_t         mirror_mode;

}VIDEO_CONFIGURATIONS;

typedef struct _byte_align4 GetVideoSourceConfigurationsOutput_INFO_T
{
    uint16_t                cmd_code;
    uint8_t                 return_code;
    CMD_STRING_T            user_name;
    CMD_STRING_T            password;
    uint8_t                 configurations_list_size;
    VIDEO_CONFIGURATIONS    *pVideo_configurations;

}GetVideoSourceConfigurationsOutput_INFO;

/**
 * GetGuaranteedNumberOfVideoEncoderInstances Input context
 **/
typedef struct _byte_align4 GetGuaranteedNumberOfVideoEncoderInstancesInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    configuration_token;

}GetGuaranteedNumberOfVideoEncoderInstancesInput_INFO;

/**
 * GetGuaranteedNumberOfVideoEncoderInstances Output context
 **/
typedef struct _byte_align4 GetGuaranteedNumberOfVideoEncoderInstancesOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         total_number;
    uint8_t         jpeg_number;
    uint8_t         h264_number;
    uint8_t         mpeg4_number;
    uint8_t         mpeg2_number;

}GetGuaranteedNumberOfVideoEncoderInstancesOutput_INFO;

/**
 * GetVideoEncoderConfigurations Input context
 **/
typedef struct _byte_align4 GetVideoEncoderConfigurationsInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetVideoEncoderConfigurationsInput_INFO;

/**
 * GetVideoEncoderConfigurations Output context
 **/
typedef struct _byte_align4 VIDEO_ENC_CONFIGURATION_DATA_T
{
    CMD_STRING_T    name;
    CMD_STRING_T    token;
    uint8_t         use_count;
    uint8_t         encoding;
    uint16_t        width;
    uint16_t        height;
    uint8_t         quality;
    uint8_t         frame_rate_limit;
    uint8_t         encoding_interval;
    uint16_t        bitrate_limit;
    uint8_t         rate_control_type;
    uint8_t         gov_length;
    uint8_t         profile;

}VIDEO_ENC_CONFIGURATION_DATA;

typedef struct _byte_align4 GetVideoEncoderConfigurationsOutput_INFO_T
{
    uint16_t                        cmd_code;
    uint8_t                         return_code;
    CMD_STRING_T                    user_name;
    CMD_STRING_T                    password;
    uint8_t                         configuration_list_size;
    VIDEO_ENC_CONFIGURATION_DATA    *pVideo_enc_configuration_data;

}GetVideoEncoderConfigurationsOutput_INFO;

/**
 * GetAudioSources Input context
 **/
typedef struct _byte_align4 GetAudioSourcesInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetAudioSourcesInput_INFO;

/**
 * GetAudioSources Output context
 **/
typedef struct _byte_align4 AUDIO_SOURCES_DATA_T
{
    CMD_STRING_T    audio_sources_token;
    uint8_t         channels;

}AUDIO_SOURCES_DATA;

typedef struct _byte_align4 GetAudioSourcesOutput_INFO_T
{
    uint16_t            cmd_code;
    uint8_t             return_code;
    CMD_STRING_T        user_name;
    CMD_STRING_T        password;
    uint8_t             audio_sources_list_size;
    AUDIO_SOURCES_DATA  *pAudio_sources_data;

}GetAudioSourcesOutput_INFO;

/**
 * GetAudioSourceConfigurations Input context
 **/
typedef struct _byte_align4 GetAudioSourceConfigurationsInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetAudioSourceConfigurationsInput_INFO;

/**
 * GetAudioSourceConfigurations Output context
 **/
typedef struct _byte_align4 AUDIO_CONFIGURATIONS_T
{
    CMD_STRING_T    name;
    CMD_STRING_T    token;
    CMD_STRING_T    source_token;
    uint8_t         use_count;

}AUDIO_CONFIGURATIONS;

typedef struct _byte_align4 GetAudioSourceConfigurationsOutput_INFO_T
{
    uint16_t                cmd_code;
    uint8_t                 return_code;
    CMD_STRING_T            user_name;
    CMD_STRING_T            password;
    uint8_t                 configurations_list_size;
    AUDIO_CONFIGURATIONS    *pAudio_configurations;

}GetAudioSourceConfigurationsOutput_INFO;

/**
 * GetAudioEncoderConfigurations Input context
 **/
typedef struct _byte_align4 GetAudioEncoderConfigurationsInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetAudioEncoderConfigurationsInput_INFO;

/**
 * GetAudioEncoderConfigurations Output context
 **/
typedef struct _byte_align4 AUDIO_ENC_CONFIGURATIONS_T
{
    CMD_STRING_T    token;
    CMD_STRING_T    name;
    uint8_t         use_count;
    uint8_t         encoding;
    uint16_t        bitrate;
    uint16_t        sample_rate;

}AUDIO_ENC_CONFIGURATIONS;

typedef struct _byte_align4 GetAudioEncoderConfigurationsOutput_INFO_T
{
    uint16_t                    cmd_code;
    uint8_t                     return_code;
    CMD_STRING_T                user_name;
    CMD_STRING_T                password;
    uint8_t                     configurations_list_size;
    AUDIO_ENC_CONFIGURATIONS    *pAudio_enc_configurations;

}GetAudioEncoderConfigurationsOutput_INFO;

/**
 * SetSynchronizationPoint Input context
 **/
typedef struct _byte_align4 SetSynchronizationPointInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    profile_token;

}SetSynchronizationPointInput_INFO;

/**
 * SetSynchronizationPoint Output context
 **/
typedef struct _byte_align4 SetSynchronizationPointOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetSynchronizationPointOutput_INFO;

/**
 * SetVideoSourceConfiguration Input context
 **/
typedef struct _byte_align4 SetVideoSourceConfigurationInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    name;
    CMD_STRING_T    token;
    CMD_STRING_T    source_token;
    uint8_t         use_count;
    uint16_t        bounds_x;
    uint16_t        bounds_y;
    uint16_t        bounds_width;
    uint16_t        bounds_height;
    uint8_t         rotate_mode;
    uint16_t        rotate_degree;
    uint8_t         mirror_mode;
    uint8_t         force_persistence;

}SetVideoSourceConfigurationInput_INFO;

/**
 * SetVideoSourceConfiguration Output context
 **/
typedef struct _byte_align4 SetVideoSourceConfigurationOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetVideoSourceConfigurationOutput_INFO;

/**
 * SetVideoEncoderConfiguration Input context
 **/
typedef struct _byte_align4 SetVideoEncoderConfigurationInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    name;
    CMD_STRING_T    token;
    uint8_t         use_count;
    uint8_t         encoding;
    uint16_t        resolution_width;
    uint16_t        resolution_height;
    uint8_t         quality;
    uint8_t         frame_rate_limit;
    uint8_t         encoding_interval;
    uint16_t        bitrate_limit;
    uint8_t         rate_control_type;
    uint8_t         gov_length;
    uint8_t         profile;
    uint8_t         force_persistence;

}SetVideoEncoderConfigurationInput_INFO;

/**
 * SetVideoEncoderConfiguration Output context
 **/
typedef struct _byte_align4 SetVideoEncoderConfigurationOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetVideoEncoderConfigurationOutput_INFO;

/**
 * SetAudioSourceConfiguration Input context
 **/
typedef struct _byte_align4 SetAudioSourceConfigurationInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    name;
    CMD_STRING_T    token;
    CMD_STRING_T    source_token;
    uint8_t         use_count;
    uint8_t         force_persistence;

}SetAudioSourceConfigurationInput_INFO;

/**
 * SetAudioSourceConfiguration Output context
 **/
typedef struct _byte_align4 SetAudioSourceConfigurationOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetAudioSourceConfigurationOutput_INFO;

/**
 * SetAudioEncoderConfiguration Input context
 **/
typedef struct _byte_align4 SetAudioEncoderConfigurationInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    name;
    CMD_STRING_T    token;
    uint8_t         use_count;
    uint8_t         encoding;
    uint16_t        bitrate;
    uint16_t        sample_rate;
    uint8_t         force_persistence;

}SetAudioEncoderConfigurationInput_INFO;

/**
 * SetAudioEncoderConfiguration Output context
 **/
typedef struct _byte_align4 SetAudioEncoderConfigurationOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetAudioEncoderConfigurationOutput_INFO;

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
