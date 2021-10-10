#ifndef __cmd_packet_dev_mgt_H_93kEDQFU_7IbQ_EdUQ_1hPo_obMnD8auN9P2__
#define __cmd_packet_dev_mgt_H_93kEDQFU_7IbQ_EdUQ_1hPo_obMnD8auN9P2__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmd_data_type.h"
//=============================================================================
//                Constant Definition
//=============================================================================

/**
 * device management service
 **/
#define DEV_MGT_SERVICE_TAG_MASK            0xFF00
#define DEV_MGT_SERVICE_CMD_MASK            0x00FF
#define DEV_MGT_INPUT_SERVICE_TAG           0x0100
#define DEV_MGT_OUTPUT_SERVICE_TAG          0x8100
// cmd
#define CMD_GetCapabilitiesInput                                    0x0100
#define CMD_GetCapabilitiesOutput                                   0x8100
#define CMD_GetDeviceInformationInput                               0x0101
#define CMD_GetDeviceInformationOutput                              0x8101
#define CMD_GetHostnameInput                                        0x0102
#define CMD_GetHostnameOutput                                       0x8102
#define CMD_GetSystemDateAndTimeInput                               0x0103
#define CMD_GetSystemDateAndTimeOutput                              0x8103
#define CMD_GetSystemLogInput                                       0x0104
#define CMD_GetSystemLogOutput                                      0x8104
#define CMD_GetOSDInformationInput                                  0x0105
#define CMD_GetOSDInformationOutput                                 0x8105
#define CMD_SystemRebootInput                                       0x0180
#define CMD_SystemRebootOutput                                      0x8180
#define CMD_SetSystemFactoryDefaultInput                            0x0181
#define CMD_SetSystemFactoryDefaultOutput                           0x8181
#define CMD_SetHostnameInput                                        0x0182
#define CMD_SetHostnameOutput                                       0x8182
#define CMD_SetSystemDateAndTimeInput                               0x0183
#define CMD_SetSystemDateAndTimeOutput                              0x8183
#define CMD_SetOSDInformationInput                                  0x0185
#define CMD_SetOSDInformationOutput                                 0x8185
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * GetCapabilities Input context
 **/
typedef struct _byte_align4 GetCapabilitiesInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetCapabilitiesInput_INFO;

/**
 * GetCapabilities Output context
 **/
typedef struct _byte_align4 GetCapabilitiesOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint32_t        supported_features;

}GetCapabilitiesOutput_INFO;

/**
 * GetDeviceInformation Input context
 **/
typedef struct _byte_align4 GetDeviceInformationInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetDeviceInformationInput_INFO;

/**
 * GetDeviceInformation Output context
 **/
typedef struct _byte_align4 GetDeviceInformationOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    manufacturer;
    CMD_STRING_T    model;
    CMD_STRING_T    firmware_version;
    CMD_STRING_T    serial_number;
    CMD_STRING_T    hardware_id;

}GetDeviceInformationOutput_INFO;

/**
 * GetHostname Input context
 **/
typedef struct _byte_align4 GetHostnameInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetHostnameInput_INFO;

/**
 * GetHostname Output context
 **/
typedef struct _byte_align4 GetHostnameOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    host_name;

}GetHostnameOutput_INFO;

/**
 * GetSystemDateAndTime Input context
 **/
typedef struct _byte_align4 GetSystemDateAndTimeInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetSystemDateAndTimeInput_INFO;

/**
 * GetSystemDateAndTime Output context
 **/
typedef struct _byte_align4 GetSystemDateAndTimeOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         country_code[3];
    uint8_t         country_region_id;
    uint8_t         daylightsavings;
    uint8_t         timezone;
    uint8_t         utc_hour;
    uint8_t         utc_minute;
    uint8_t         utc_second;
    uint16_t        utc_year;
    uint8_t         utc_month;
    uint8_t         utc_day;

}GetSystemDateAndTimeOutput_INFO;

/**
 * GetSystemLog Input context
 **/
typedef struct _byte_align4 GetSystemLogInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         log_type;

}GetSystemLogInput_INFO;

/**
 * GetSystemLog Output context
 **/
typedef struct _byte_align4 GetSystemLogOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    system_log;

}GetSystemLogOutput_INFO;

/**
 * GetOSDInformation Input context
 **/
typedef struct _byte_align4 GetOSDInformationInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetOSDInformationInput_INFO;

/**
 * GetOSDInformation Output context
 **/
typedef struct _byte_align4 GetOSDInformationOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         osd_date_enable;
    uint8_t         osd_date_position;
    uint8_t         osd_date_format;
    uint8_t         osd_time_enable;
    uint8_t         osd_time_position;
    uint8_t         osd_time_format;
    uint8_t         osd_logo_enable;
    uint8_t         osd_logo_position;
    uint8_t         osd_logo_option;
    uint8_t         osd_detail_info_enable;
    uint8_t         osd_detail_info_position;
    uint8_t         osd_detail_info_option;
    uint8_t         osd_text_enable;
    uint8_t         osd_text_position;
    CMD_STRING_T    osd_text;

}GetOSDInformationOutput_INFO;

/**
 * SystemReboot Input context
 **/
typedef struct _byte_align4 SystemRebootInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         reboot_type;

}SystemRebootInput_INFO;

/**
 * SystemReboot Output context
 **/
typedef struct _byte_align4 SystemRebootOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    reboot_message;

}SystemRebootOutput_INFO;

/**
 * SetSystemFactoryDefault Input context
 **/
typedef struct _byte_align4 SetSystemFactoryDefaultInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         factory_default;

}SetSystemFactoryDefaultInput_INFO;

/**
 * SetSystemFactoryDefault Output context
 **/
typedef struct _byte_align4 SetSystemFactoryDefaultOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetSystemFactoryDefaultOutput_INFO;

/**
 * SetHostname Input context
 **/
typedef struct _byte_align4 SetHostnameInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    host_name;

}SetHostnameInput_INFO;

/**
 * SetHostname Output context
 **/
typedef struct _byte_align4 SetHostnameOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetHostnameOutput_INFO;

/**
 * SetSystemDateAndTime Input context
 **/
typedef struct _byte_align4 SetSystemDateAndTimeInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         country_code[3];
    uint8_t         country_region_id;
    uint8_t         daylightsavings;
    uint8_t         timezone;
    uint8_t         utc_hour;
    uint8_t         utc_minute;
    uint8_t         utc_second;
    uint16_t        utc_year;
    uint8_t         utc_month;
    uint8_t         utc_day;

}SetSystemDateAndTimeInput_INFO;

/**
 * SetSystemDateAndTime Output context
 **/
typedef struct _byte_align4 SetSystemDateAndTimeOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetSystemDateAndTimeOutput_INFO;

/**
 * SetOSDInformation Input context
 **/
typedef struct _byte_align4 SetOSDInformationInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         osd_date_enable;
    uint8_t         osd_date_position;
    uint8_t         osd_date_format;
    uint8_t         osd_time_enable;
    uint8_t         osd_time_position;
    uint8_t         osd_time_format;
    uint8_t         osd_logo_enable;
    uint8_t         osd_logo_position;
    uint8_t         osd_logo_option;
    uint8_t         osd_detail_info_enable;
    uint8_t         osd_detail_info_position;
    uint8_t         osd_detail_info_option;
    uint8_t         osd_text_enable;
    uint8_t         osd_text_position;
    CMD_STRING_T    osd_text;

}SetOSDInformationInput_INFO;

/**
 * SetOSDInformation Output context
 **/
typedef struct _byte_align4 SetOSDInformationOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetOSDInformationOutput_INFO;
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
