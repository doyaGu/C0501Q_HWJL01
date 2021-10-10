#ifndef __aggre_ctrl_H_4cHmADgm_3QVx_KZJm_29EU_wH8Fb82DNCyt__
#define __aggre_ctrl_H_4cHmADgm_3QVx_KZJm_29EU_wH8Fb82DNCyt__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>
#include "ts_aggre_err.h"
//=============================================================================
//                Constant Definition
//=============================================================================

#if (CFG_DEMOD_SUPPORT_COUNT > 0)
    #if (CFG_DEMOD_SUPPORT_COUNT > 3)
        #define TSA_MAX_PORT_NUM         4
    #else
        #define TSA_MAX_PORT_NUM         CFG_DEMOD_SUPPORT_COUNT // 4
    #endif
#else
    #define TSA_MAX_PORT_NUM         1
#endif

//=============================================================================
//                Macro Definition
//=============================================================================
/**
 * ts aggregation id
 **/
typedef enum TS_AGGR_ID_T
{
    TS_AGGR_ID_NONE             = 0,
    TS_AGGR_ID_ENDEAVOUR,

}TS_AGGR_ID;

/**
 * ts aggregation mode
 **/
typedef enum TSA_MODE_T
{
    TSA_MODE_NONE         = 0,
    TSA_MODE_SYNC_BYTE    = 1,
    TSA_MODE_TAG          = 2,
    TSA_MODE_PID_REMAP    = 3,
}TSA_MODE;

/**
 * ts aggregation tag length
 **/
typedef enum TSA_TAG_LEN_T
{
    TSA_TAG_LEN_0      = 0,
    TSA_TAG_LEN_4      = 4,
    TSA_TAG_LEN_16     = 16,
    TSA_TAG_LEN_20     = 20,
}TSA_TAG_LEN;

/**
 * ts aggregation bus type
 **/
typedef enum TSA_BUS_TYPE_T
{
    TSA_BUS_UNKNOW  = 0,
    TSA_BUS_I2C,
    TSA_BUS_USB,
}TSA_BUS_TYPE;

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * ts aggregation arguments
 **/
typedef struct TSA_AGR_T
{
#define TSA_ARG_TYPE_SET_PORT           0xA1
#define TSA_ARG_TYPE_SET_MODE           0xA2

    uint32_t        type;

    union{

        struct{
            uint32_t        port_index;
        }set_port;

        struct{
            uint32_t        port_index;
            TSA_MODE        aggre_mode;

            // reg[tag_value3] << 24 | reg[tag_value2] << 16 | reg[tag_value1] << 8 | reg[tag_value0]
            uint32_t        tag_value;
        }set_mode;
    }arg;

}TSA_AGR;

/**
 * ts aggregation setup info when create handle
 **/
typedef struct TSA_SETUP_INFO_T
{
    uint32_t        tsa_idx;
    TS_AGGR_ID      aggre_id;
    //uint32_t        total_demod_port;
    TSA_TAG_LEN     tsa_tag_len;

}TSA_SETUP_INFO;


/**
 * ts aggregation init parameters
 **/
typedef struct TSA_INIT_PARAM_T
{
    TSA_BUS_TYPE    bus_type;
    uint32_t        aggre_i2c_addr;
    TSA_MODE        tsa_mode;

    uint32_t        total_demod_port;

    // reg[tag_value3] << 24 | reg[tag_value2] << 16 | reg[tag_value1] << 8 | reg[tag_value0]
    uint32_t        tag_value[TSA_MAX_PORT_NUM];

    uint32_t        demod_i2c_addr[TSA_MAX_PORT_NUM];
    uint32_t        linked_aggre_port_idx[TSA_MAX_PORT_NUM];

}TSA_INIT_PARAM;

/**
 * ts aggregation ctrl handle
 **/
typedef struct TSA_HANDLE_T
{
    TS_AGGR_ID      tsa_type;

    uint32_t        tsa_index;

    uint32_t        tag_length;

}TSA_HANDLE;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
TSA_ERR
tsa_CreateHandle(
    TSA_HANDLE      **ppHTsa,
    TSA_SETUP_INFO  *pSetup_info,
    void            *extraData);


TSA_ERR
tsa_DestroyHandle(
    TSA_HANDLE     **ppHTsa,
    void           *extraData);


TSA_ERR
tsa_Init(
    TSA_HANDLE      *pHTsa,
    TSA_INIT_PARAM  *pInit_param,
    void            *extraData);


TSA_ERR
tsa_Deinit(
    TSA_HANDLE     *pHTsa,
    TSA_AGR        *pTsaArg,
    void           *extraData);


TSA_ERR
tsa_Enable_Port(
    TSA_HANDLE     *pHTsa,
    TSA_AGR        *pTsaArg,
    void           *extraData);


TSA_ERR
tsa_Disable_Port(
    TSA_HANDLE     *pHTsa,
    TSA_AGR        *pTsaArg,
    void           *extraData);


TSA_ERR
tsa_Set_Aggre_Mode(
    TSA_HANDLE     *pHTsa,
    TSA_AGR        *pTsaArg,
    void           *extraData);


#ifdef __cplusplus
}
#endif

#endif
