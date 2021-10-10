#ifndef __cmd_packet_dev_io_H_ir1Agf9R_lbrL_zj0k_oBbS_ndZSMt88iShq__
#define __cmd_packet_dev_io_H_ir1Agf9R_lbrL_zj0k_oBbS_ndZSMt88iShq__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmd_data_type.h"
//=============================================================================
//                Constant Definition
//=============================================================================
/**
 * device I/O service
 **/
#define DEV_IO_SERVICE_TAG_MASK             0xFF00
#define DEV_IO_SERVICE_CMD_MASK             0x00FF
#define DEV_IO_INPUT_SERVICE_TAG            0x0200
#define DEV_IO_OUTPUT_SERVICE_TAG           0x8200
// cmd
#define CMD_GetDigitalInputsInput                                  0x0200
#define CMD_GetDigitalInputsOutput                                 0x8200
#define CMD_GetRelayOutputsInput                                   0x0201
#define CMD_GetRelayOutputsOutput                                  0x8201
#define CMD_SetRelayOutputStateInput                               0x0280
#define CMD_SetRelayOutputStateOutput                              0x8280
#define CMD_SetRelayOutputSettingsInput                            0x0281
#define CMD_SetRelayOutputSettingsOutput                           0x8281
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * GetDigitalInputs Input context
 **/
typedef struct _byte_align4 GetDigitalInputsInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetDigitalInputsInput_INFO;

/**
 * GetDigitalInputs Output context
 **/
typedef struct _byte_align4 GetDigitalInputsOutput_INFO_T
{
    uint16_t            cmd_code;
    uint8_t             return_code;
    CMD_STRING_T        user_name;
    CMD_STRING_T        password;
    uint8_t             digitalinputs_list_size;
    CMD_STRING_LIST_T   token_list;

}GetDigitalInputsOutput_INFO;

/**
 * GetRelayOutputs Input context
 **/
typedef struct _byte_align4 GetRelayOutputsInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetRelayOutputsInput_INFO;

/**
 * GetRelayOutputs Output context
 **/
typedef struct _byte_align4 RELAY_OUTPUTS_DATA_T
{
    CMD_STRING_T         token_list;
    uint8_t              mode;
    uint32_t             delay_time;
    uint8_t              idle_state;

}RELAY_OUTPUTS_DATA;

typedef struct _byte_align4 GetRelayOutputsOutput_INFO_T
{
    uint16_t             cmd_code;
    uint8_t              return_code;
    CMD_STRING_T         user_name;
    CMD_STRING_T         password;
    uint8_t              relayoutputs_list_size;
    RELAY_OUTPUTS_DATA   *pRelayoutputs_data;

}GetRelayOutputsOutput_INFO;

/**
 * SetRelayOutputState Input context
 **/
typedef struct _byte_align4 SetRelayOutputStateInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    relay_output_token;
    uint8_t         logical_state;

}SetRelayOutputStateInput_INFO;

/**
 * SetRelayOutputState Output context
 **/
typedef struct _byte_align4 SetRelayOutputStateOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetRelayOutputStateOutput_INFO;

/**
 * SetRelayOutputSettings Input context
 **/
typedef struct _byte_align4 SetRelayOutputSettingsInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    token;
    uint8_t         mode;
    uint32_t        delay_time;
    uint8_t         idle_state;

}SetRelayOutputSettingsInput_INFO;

/**
 * SetRelayOutputSettings Output context
 **/
typedef struct _byte_align4 SetRelayOutputSettingsOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}SetRelayOutputSettingsOutput_INFO;
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
