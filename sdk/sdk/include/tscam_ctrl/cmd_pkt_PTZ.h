#ifndef __cmd_packet_PTZ_H_sI3eNnYa_RE1Z_A6GV_Xbai_56XGxP2SBbbQ__
#define __cmd_packet_PTZ_H_sI3eNnYa_RE1Z_A6GV_Xbai_56XGxP2SBbbQ__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmd_data_type.h"
//=============================================================================
//                Constant Definition
//=============================================================================
/**
 * PTZ service
 **/
#define PTZ_SERVICE_TAG_MASK                0xFF00
#define PTZ_SERVICE_CMD_MASK                0x00FF
#define PTZ_INPUT_SERVICE_TAG               0x0500
#define PTZ_OUTPUT_SERVICE_TAG              0x8500
// cmd
#define CMD_GetConfigurationsInput                                   0x0500
#define CMD_GetConfigurationsOutput                                  0x8500
#define CMD_PTZ_GetStatusInput                                       0x0501
#define CMD_PTZ_GetStatusOutput                                      0x8501
#define CMD_GetPresetsInput                                          0x0502
#define CMD_GetPresetsOutput                                         0x8502
#define CMD_GotoPresetInput                                          0x0580
#define CMD_GotoPresetOutput                                         0x8580
#define CMD_RemovePresetInput                                        0x0581
#define CMD_RemovePresetOutput                                       0x8581
#define CMD_SetPresetInput                                           0x0582
#define CMD_SetPresetOutput                                          0x8582
#define CMD_AbsoluteMoveInput                                        0x0583
#define CMD_AbsoluteMoveOutput                                       0x8583
#define CMD_RelativeMoveInput                                        0x0584
#define CMD_RelativeMoveOutput                                       0x8584
#define CMD_ContinuousMoveInput                                      0x0585
#define CMD_ContinuousMoveOutput                                     0x8585
#define CMD_SetHomePositionInput                                     0x0586
#define CMD_SetHomePositionOutput                                    0x8586
#define CMD_GotoHomePositionInput                                    0x0587
#define CMD_GotoHomePositionOutput                                   0x8587
#define CMD_PTZ_StopInput                                            0x0588
#define CMD_PTZ_StopOutput                                           0x8588
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * GetConfigurations Input context
 **/
typedef struct _byte_align4 GetConfigurationsInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetConfigurationsInput_INFO;

/**
 * GetConfigurations Output context
 **/
typedef struct _byte_align4 PTZ_CONFIGURATION_T
{
    CMD_STRING_T    name;
    uint8_t         use_count;
    CMD_STRING_T    token;
    int16_t         default_pan_speed;
    int16_t         default_tilt_speed;
    int16_t         default_zoom_speed;
    uint32_t        default_timeout;
    uint16_t        pan_limit_min;
    uint16_t        pan_limit_max;
    uint16_t        tilt_limit_min;
    uint16_t        tilt_limit_max;
    uint16_t        zoom_limit_min;
    uint16_t        zoom_limit_max;
    uint8_t         ptz_control_direction_eflip_mode;
    uint8_t         ptz_control_direction_reverse_mode;

}PTZ_CONFIGURATION;

typedef struct _byte_align4 GetConfigurationsOutput_INFO_T
{
    uint16_t            cmd_code;
    uint8_t             return_code;
    CMD_STRING_T        user_name;
    CMD_STRING_T        password;
    uint8_t             ptz_configuration_list_size;
    PTZ_CONFIGURATION   *pPtz_configuration;

}GetConfigurationsOutput_INFO;

/**
 * PTZ_GetStatus Input context
 **/
typedef struct _byte_align4 PTZ_GetStatusInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    token;

}PTZ_GetStatusInput_INFO;

/**
 * PTZ_GetStatus Output context
 **/
typedef struct _byte_align4 PTZ_GetStatusOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    int16_t         pan_position;
    int16_t         tilt_position;
    int16_t         zoom_position;
    uint8_t         pan_tilt_move_status;
    uint8_t         zoom_move_status;
    CMD_STRING_T    error_msg;
    uint8_t         utc_hour;
    uint8_t         utc_minute;
    uint8_t         utc_second;
    uint16_t        utc_year;
    uint8_t         utc_month;
    uint8_t         utc_day;

}PTZ_GetStatusOutput_INFO;

/**
 * GetPresets Input context
 **/
typedef struct _byte_align4 GetPresetsInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    token;

}GetPresetsInput_INFO;

/**
 * GetPresets Output context
 **/
typedef struct _byte_align4 PTZ_PRESET_DATA_T
{
    CMD_STRING_T    name;
    CMD_STRING_T    token;
    int16_t         pan_position;
    int16_t         tilt_position;
    int16_t         zoom_position;

}PTZ_PRESET_DATA;

typedef struct _byte_align4 GetPresetsOutput_INFO_T
{
    uint16_t            cmd_code;
    uint8_t             return_code;
    CMD_STRING_T        user_name;
    CMD_STRING_T        password;
    uint8_t             preset_list_size;
    PTZ_PRESET_DATA     *pPtz_preset_data;

}GetPresetsOutput_INFO;

/**
 * GotoPreset Input context
 **/
typedef struct _byte_align4 GotoPresetInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    token;
    CMD_STRING_T    preset_token;
    int16_t         pan_speed;
    int16_t         tilt_speed;
    int16_t         zoom_speed;

}GotoPresetInput_INFO;

/**
 * GotoPreset Output context
 **/
typedef struct _byte_align4 GotoPresetOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GotoPresetOutput_INFO;

/**
 * RemovePreset Input context
 **/
typedef struct _byte_align4 RemovePresetInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    token;
    CMD_STRING_T    preset_token;

}RemovePresetInput_INFO;

/**
 * RemovePreset Output context
 **/
typedef struct _byte_align4 RemovePresetOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}RemovePresetOutput_INFO;

/**
 * SetPreset Input context
 **/
typedef struct _byte_align4 SetPresetInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    token;
    CMD_STRING_T    preset_name;
    CMD_STRING_T    preset_token;

}SetPresetInput_INFO;

/**
 * SetPreset Output context
 **/
typedef struct _byte_align4 SetPresetOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    preset_token;

}SetPresetOutput_INFO;

/**
 * AbsoluteMove Input context
 **/
typedef struct _byte_align4 AbsoluteMoveInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    token;
    int16_t         pan_position;
    int16_t         tilt_position;
    int16_t         zoom_position;
    int16_t         pan_speed;
    int16_t         tilt_speed;
    int16_t         zoom_speed;

}AbsoluteMoveInput_INFO;

/**
 * AbsoluteMove Output context
 **/
typedef struct _byte_align4 AbsoluteMoveOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}AbsoluteMoveOutput_INFO;

/**
 * RelativeMove Input context
 **/
typedef struct _byte_align4 RelativeMoveInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    token;
    int16_t         pan_translation;
    int16_t         tilt_translation;
    int16_t         zoom_translation;
    int16_t         pan_speed;
    int16_t         tilt_speed;
    int16_t         zoom_speed;

}RelativeMoveInput_INFO;

/**
 * RelativeMove Output context
 **/
typedef struct _byte_align4 RelativeMoveOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}RelativeMoveOutput_INFO;

/**
 * ContinuousMove Input context
 **/
typedef struct _byte_align4 ContinuousMoveInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    token;
    int16_t         pan_velocity;
    int16_t         tilt_velocity;
    int16_t         zoom_velocity;
    uint32_t        timeout;

}ContinuousMoveInput_INFO;

/**
 * ContinuousMove Output context
 **/
typedef struct _byte_align4 ContinuousMoveOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}ContinuousMoveOutput_INFO;

/**
 * SetHomePosition Input context
 **/
typedef struct _byte_align4 SetHomePositionInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    token;

}SetHomePositionInput_INFO;

/**
 * SetHomePosition Output context
 **/
typedef struct _byte_align4 SetHomePositionOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetHomePositionOutput_INFO;

/**
 * GotoHomePosition Input context
 **/
typedef struct _byte_align4 GotoHomePositionInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    token;
    int16_t         pan_speed;
    int16_t         tilt_speed;
    int16_t         zoom_speed;

}GotoHomePositionInput_INFO;

/**
 * GotoHomePosition Output context
 **/
typedef struct _byte_align4 GotoHomePositionOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GotoHomePositionOutput_INFO;

/**
 * PTZ_Stop Input context
 **/
typedef struct _byte_align4 PTZ_StopInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    token;
    uint8_t         pan_tilt_zoom;

}PTZ_StopInput_INFO;

/**
 * PTZ_Stop Output context
 **/
typedef struct _byte_align4 PTZ_StopOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}PTZ_StopOutput_INFO;
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
