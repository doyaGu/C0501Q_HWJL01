#ifndef __cmd_packet_ccHDtv_H_BBbW34YV_p85o_jSAP_DZLy_AOKrH9WgefbD__
#define __cmd_packet_ccHDtv_H_BBbW34YV_p85o_jSAP_DZLy_AOKrH9WgefbD__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmd_data_type.h"
//=============================================================================
//                Constant Definition
//=============================================================================
/**
 * ccHDtv service
 **/
#define CCHDTV_SERVICE_TAG_MASK             0xFF00
#define CCHDTV_SERVICE_CMD_MASK             0x00FF
#define CCHDTV_INPUT_SERVICE_TAG            0x0000
#define CCHDTV_OUTPUT_SERVICE_TAG           0x8000

// cmd
#define CMD_GetTxDeviceAddressIDInput                               0x0000
#define CMD_GetTxDeviceAddressIDOutput                              0x8000
#define CMD_GetTransmissionParameterCapabilitiesInput               0x0001
#define CMD_GetTransmissionParameterCapabilitiesOutput              0x8001
#define CMD_GetTransmissionParametersInput                          0x0002
#define CMD_GetTransmissionParametersOutput                         0x8002
#define CMD_GetHwRegisterValuesInput                                0x0003
#define CMD_GetHwRegisterValuesOutput                               0x8003
#define CMD_GetAdvanceOptionsInput                                  0x0004
#define CMD_GetAdvanceOptionsOutput                                 0x8004
#define CMD_GetSiPsiTableInput                                      0x0010
#define CMD_GetSiPsiTableOutput                                     0x8010
#define CMD_GetNitLocationInput                                     0x0011
#define CMD_GetNitLocationOutput                                    0x8011
#define CMD_GetSdtServiceInput                                      0x0012
#define CMD_GetSdtServiceOutput                                     0x8012
#define CMD_SetTxDeviceAddressIDInput                               0x0080
#define CMD_SetTxDeviceAddressIDOutput                              0x8080
#define CMD_SetCalibrationTableInput                                0x0081
#define CMD_SetCalibrationTableOutput                               0x8081
#define CMD_SetTransmissionParametersInput                          0x0082
#define CMD_SetTransmissionParametersOutput                         0x8082
#define CMD_SetHwRegisterValuesInput                                0x0083
#define CMD_SetHwRegisterValuesOutput                               0x8083
#define CMD_SetAdvaneOptionsInput                                   0x0084
#define CMD_SetAdvaneOptionsOutput                                  0x8084
#define CMD_SetSiPsiTableInput                                      0x0090
#define CMD_SetSiPsiTableOutput                                     0x8090
#define CMD_SetNitLocationInput                                     0x0091
#define CMD_SetNitLocationOutput                                    0x8091
#define CMD_SetSdtServiceInput                                      0x0092
#define CMD_SetSdtServiceOutput                                     0x8092
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * GetTxDeviceAddressID Input context
 **/
typedef struct _byte_align4 GetTxDeviceAddressIDInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetTxDeviceAddressIDInput_INFO;

/**
 * GetTxDeviceAddressID Output context
 **/
typedef struct _byte_align4 GetTxDeviceAddressIDOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint16_t        tx_dev_addr_id;

}GetTxDeviceAddressIDOutput_INFO;

/**
 * GetTransmissionParameterCapabilities Input context
 **/
typedef struct _byte_align4 GetTransmissionParameterCapabilitiesInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetTransmissionParameterCapabilitiesInput_INFO;

/**
 * GetTransmissionParameterCapabilities Output context
 **/
typedef struct _byte_align4 GetTransmissionParameterCapabilitiesOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         bandwidth_options;
    uint32_t        freq_min;
    uint32_t        freq_max;
    uint8_t         constellation_options;
    uint8_t         fft_options;
    uint8_t         code_rate_options;
    uint8_t         guard_interval;
    uint8_t         rf_attenuation_min;
    uint8_t         rf_attenuation_max;

}GetTransmissionParameterCapabilitiesOutput_INFO;

/**
 * GetTransmissionParameters Input context
 **/
typedef struct _byte_align4 GetTransmissionParametersInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetTransmissionParametersInput_INFO;

/**
 * GetTransmissionParameters Output context
 **/
typedef struct _byte_align4 GetTransmissionParametersOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         bandwidth;
    uint32_t        frequency;
    uint8_t         constellation;
    uint8_t         fft;
    uint8_t         code_rate;
    uint8_t         guard_interval;
    uint8_t         attenuation;

}GetTransmissionParametersOutput_INFO;

/**
 * GetHwRegisterValues Input context
 **/
typedef struct _byte_align4 GetHwRegisterValuesInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         processor;
    uint32_t        register_address;
    uint8_t         register_value_list_size;

}GetHwRegisterValuesInput_INFO;

/**
 * GetHwRegisterValues Output context
 **/
typedef struct _byte_align4 GetHwRegisterValuesOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         register_value_list_size;
    uint8_t         *pRegister_value_list;

}GetHwRegisterValuesOutput_INFO;

/**
 * GetAdvanceOptions Input context
 **/
typedef struct _byte_align4 GetAdvanceOptionsInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetAdvanceOptionsInput_INFO;

/**
 * GetAdvanceOptions Output context
 **/
typedef struct _byte_align4 GetAdvanceOptionsOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint16_t        pts_pcr_latency;
    uint16_t        rx_latency_recover_time_interval;
    uint8_t         rx_latency_recover_frame_skip_number;
    uint8_t         input_frame_rate_over_flow_number;
    uint16_t        frame_encode_frame_data_rate_over_flow_size;

}GetAdvanceOptionsOutput_INFO;

/**
 * GetSiPsiTable Input context
 **/
typedef struct _byte_align4 GetSiPsiTableInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetSiPsiTableInput_INFO;

/**
 * GetSiPsiTable Output context
 **/
typedef struct _byte_align4 GetSiPsiTableOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint16_t        onid;
    uint16_t        nid;
    uint16_t        tsid;
    uint8_t         network_name[32];

}GetSiPsiTableOutput_INFO;

/**
 * GetNitLocation Input context
 **/
typedef struct _byte_align4 GetNitLocationInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetNitLocationInput_INFO;

/**
 * GetNitLocation Output context
 **/
typedef struct _byte_align4 GetNitLocationOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint16_t        latitude;
    uint16_t        longitude;
    uint16_t        extent_latitude;
    uint16_t        extent_longitude;

}GetNitLocationOutput_INFO;

/**
 * GetSdtService Input context
 **/
typedef struct _byte_align4 GetSdtServiceInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetSdtServiceInput_INFO;

/**
 * GetSdtService Output context
 **/
typedef struct _byte_align4 GetSdtServiceOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint16_t        service_id;
    uint8_t         enable;
    uint16_t        lcn;
    CMD_STRING_T    service_name;
    CMD_STRING_T    provider;

}GetSdtServiceOutput_INFO;

/**
 * SetTxDeviceAddressID Input context
 **/
typedef struct _byte_align4 SetTxDeviceAddressIDInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         id_type;
    uint16_t        new_device_address_id;

}SetTxDeviceAddressIDInput_INFO;

/**
 * SetTxDeviceAddressID Output context
 **/
typedef struct _byte_align4 SetTxDeviceAddressIDOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetTxDeviceAddressIDOutput_INFO;

/**
 * SetCalibrationTable Input context
 **/
typedef struct _byte_align4 SetCalibrationTableInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         access_option;
    uint8_t         table_type;
    CMD_STRING_T    table_data;

}SetCalibrationTableInput_INFO;

/**
 * SetCalibrationTable Output context
 **/
typedef struct _byte_align4 SetCalibrationTableOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetCalibrationTableOutput_INFO;

/**
 * SetTransmissionParameters Input context
 **/
typedef struct _byte_align4 SetTransmissionParametersInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         bandwidth;
    uint32_t        frequency;
    uint8_t         constellation;
    uint8_t         fft;
    uint8_t         code_rate;
    uint8_t         guard_interval;
    uint8_t         rf_attenuation;
    uint8_t         reserved_1[11];

}SetTransmissionParametersInput_INFO;

/**
 * SetTransmissionParameters Output context
 **/
typedef struct _byte_align4 SetTransmissionParametersOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetTransmissionParametersOutput_INFO;

/**
 * SetHwRegisterValues Input context
 **/
typedef struct _byte_align4 SetHwRegisterValuesInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         processor;
    uint32_t        register_address;
    uint8_t         register_value_list_size;
    uint8_t         *pRegister_value_list;

}SetHwRegisterValuesInput_INFO;

/**
 * SetHwRegisterValues Output context
 **/
typedef struct _byte_align4 SetHwRegisterValuesOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetHwRegisterValuesOutput_INFO;

/**
 * SetAdvaneOptions Input context
 **/
typedef struct _byte_align4 SetAdvaneOptionsInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint16_t        pts_pcr_latency;
    uint16_t        rx_latency_recover_time_interval;
    uint8_t         rx_latency_recover_frame_skip_number;
    uint8_t         input_frame_rate_over_flow_number;
    uint16_t        frame_encode_frame_data_rate_over_flow_size;

}SetAdvaneOptionsInput_INFO;

/**
 * SetAdvaneOptions Output context
 **/
typedef struct _byte_align4 SetAdvaneOptionsOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetAdvaneOptionsOutput_INFO;

/**
 * SetSiPsiTable Input context
 **/
typedef struct _byte_align4 SetSiPsiTableInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint16_t        onid;
    uint16_t        nid;
    uint16_t        tsid;
    uint8_t         network_name[32];
    uint8_t         reserved_1[16];

}SetSiPsiTableInput_INFO;

/**
 * SetSiPsiTable Output context
 **/
typedef struct _byte_align4 SetSiPsiTableOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetSiPsiTableOutput_INFO;

/**
 * SetNitLocation Input context
 **/
typedef struct _byte_align4 SetNitLocationInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint16_t        latitude;
    uint16_t        longitude;
    uint16_t        extent_latitude;
    uint16_t        extent_longitude;

}SetNitLocationInput_INFO;

/**
 * SetNitLocation Output context
 **/
typedef struct _byte_align4 SetNitLocationOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetNitLocationOutput_INFO;

/**
 * SetSdtService Input context
 **/
typedef struct _byte_align4 SetSdtServiceInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint16_t        service_id;
    uint8_t         enable;
    uint16_t        lcn;
    CMD_STRING_T    service_name;
    CMD_STRING_T    provider;

}SetSdtServiceInput_INFO;

/**
 * SetSdtService Output context
 **/
typedef struct _byte_align4 SetSdtServiceOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetSdtServiceOutput_INFO;
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
