#ifndef __cmd_packet_image_H_5pIEyjxw_he6w_Vgy3_2gea_yWvGUAjPdOas__
#define __cmd_packet_image_H_5pIEyjxw_he6w_Vgy3_2gea_yWvGUAjPdOas__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmd_data_type.h"
//=============================================================================
//                Constant Definition
//=============================================================================
/**
 * image service
 **/
#define IMG_SERVICE_TAG_MASK                0xFF00
#define IMG_SERVICE_CMD_MASK                0x00FF
#define IMG_SERVICE_INPUT_TAG               0x0300
#define IMG_SERVICE_OUTPUT_TAG              0x8300
// cmd
#define CMD_GetImagingSettingsInput                                 0x0300
#define CMD_GetImagingSettingsOutput                                0x8300
#define CMD_IMG_GetStatusInput                                      0x0301
#define CMD_IMG_GetStatusOutput                                     0x8301
#define CMD_SetImagingSettingsInput                                 0x0380
#define CMD_SetImagingSettingsOutput                                0x8380
#define CMD_MoveInput                                               0x0381
#define CMD_MoveOutput                                              0x8381
#define CMD_IMG_StopInput                                           0x0382
#define CMD_IMG_StopOutput                                          0x8382
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * GetImagingSettings Input context
 **/
typedef struct _byte_align4 GetImagingSettingsInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    video_source_token;

}GetImagingSettingsInput_INFO;

/**
 * GetImagingSettings Output context
 **/
typedef struct _byte_align4 GetImagingSettingsOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         backlight_compensation_mode;
    float           backlight_compensation_level;
    float           brightness;
    float           color_saturation;
    float           contrast;
    uint8_t         exposure_mode;
    uint8_t         exposure_priority;
    uint16_t        exposure_window_bottom;
    uint16_t        exposure_window_top;
    uint16_t        exposure_window_right;
    uint16_t        exposure_window_left;
    uint32_t        min_exposuretime;
    uint32_t        max_exposuretime;
    float           exposure_mingain;
    float           exposure_maxgain;
    float           exposure_miniris;
    float           exposure_maxiris;
    uint32_t        exposure_time;
    float           exposure_gain;
    float           exposure_iris;
    uint8_t         auto_focus_mode;
    float           focus_default_speed;
    float           focus_near_limit;
    float           focus_far_limit;
    uint8_t         ir_cut_filter_mode;
    float           sharpness;
    uint8_t         wide_dynamic_range_mode;
    float           wide_dynamic_range_level;
    uint8_t         white_balance_mode;
    float           white_balance_cr_gain;
    float           white_balance_cb_gain;
    uint8_t         image_stabilization_mode;
    float           image_stabilization_level;

}GetImagingSettingsOutput_INFO;

/**
 * IMG_GetStatus Input context
 **/
typedef struct _byte_align4 IMG_GetStatusInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    video_source_token;

}IMG_GetStatusInput_INFO;

/**
 * IMG_GetStatus Output context
 **/
typedef struct _byte_align4 IMG_GetStatusOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint32_t        position;
    uint8_t         move_status;
    CMD_STRING_T    error_msg;

}IMG_GetStatusOutput_INFO;

/**
 * SetImagingSettings Input context
 **/
typedef struct _byte_align4 SetImagingSettingsInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    video_source_token;
    uint8_t         backlight_compensation_mode;
    float           backlight_compensation_level;
    float           brightness;
    float           color_saturation;
    float           contrast;
    uint8_t         exposure_mode;
    uint8_t         exposure_priority;
    uint16_t        exposure_window_bottom;
    uint16_t        exposure_window_top;
    uint16_t        exposure_window_right;
    uint16_t        exposure_window_left;
    uint32_t        min_exposure_time;
    uint32_t        max_exposure_time;
    float           exposure_min_gain;
    float           exposure_max_gain;
    float           exposure_min_iris;
    float           exposure_max_iris;
    uint32_t        exposure_time;
    float           exposure_gain;
    float           exposure_iris;
    uint8_t         auto_focus_mode;
    float           focus_default_speed;
    float           focus_nearlimit;
    float           focus_farlimit;
    uint8_t         ir_cut_filter_mode;
    float           sharpness;
    uint8_t         wide_dynamicrange_mode;
    float           wide_dynamicrange_level;
    uint8_t         white_balance_mode;
    float           white_balance_cr_gain;
    float           whitebalance_cb_gain;
    uint8_t         image_stabilization_mode;
    float           image_stabilization_level;
    uint8_t         force_persistence;

}SetImagingSettingsInput_INFO;

/**
 * SetImagingSettings Output context
 **/
typedef struct _byte_align4 SetImagingSettingsOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetImagingSettingsOutput_INFO;

/**
 * Move Input context
 **/
typedef struct _byte_align4 MoveInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    video_source_token;
    float           absolute_focus_position;
    float           absolute_focus_speed;
    float           relative_focus_distance;
    float           relative_focus_speed;
    float           continuous_focus_speed;

}MoveInput_INFO;

/**
 * Move Output context
 **/
typedef struct _byte_align4 MoveOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}MoveOutput_INFO;

/**
 * IMG_Stop Input context
 **/
typedef struct _byte_align4 IMG_StopInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    video_source_token;

}IMG_StopInput_INFO;

/**
 * IMG_Stop Output context
 **/
typedef struct _byte_align4 IMG_StopOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}IMG_StopOutput_INFO;
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
