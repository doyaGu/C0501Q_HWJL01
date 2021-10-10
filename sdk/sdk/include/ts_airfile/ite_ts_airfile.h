#ifndef __ITE_TS_AIRFILE_H_N0QAYFVE_OONM_VATA_5I1V_XTPEUQCV4D2K__
#define __ITE_TS_AIRFILE_H_N0QAYFVE_OONM_VATA_5I1V_XTPEUQCV4D2K__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "ts_airfile_err.h"
//=============================================================================
//                Constant Definition
//=============================================================================
#if (CFG_DEMOD_SUPPORT_COUNT > 0)
    #if (CFG_DEMOD_SUPPORT_COUNT > 3)
        #define MAX_DEMOD_SUPPORT_CNT         4
    #else
        #define MAX_DEMOD_SUPPORT_CNT         CFG_DEMOD_SUPPORT_COUNT // 4
    #endif
#else
    #define MAX_DEMOD_SUPPORT_CNT         1
#endif

/**
 * ts airfile demod type
 **/
typedef enum _TSAF_DEMOD_TYPE_T
{
    TSAF_DEMOD_UNKNOW      = 0,
    TSAF_DEMOD_OMEGA,
    TSAF_DEMOD_IT9135,
    TSAF_DEMOD_IT9137,
    TSAF_DEMOD_IT9137_USB,

}TSAF_DEMOD_TYPE;

/**
 * ts air file handle stat
 **/
typedef enum _TSAF_STATE_TAG
{
    TSAF_STATE_UNKNOW     = 0,
    TSAF_STATE_IDLE,
    TSAF_STATE_READY,
    TSAF_STATE_SCANNING,
    TSAF_STATE_READING,
    TSAF_STATE_WRITING,

}TSAF_STATE;

typedef enum _TSAF_INSRC_TYPE_TAG
{
    TSAF_INSRC_UNKNOW       =  0,
    TSAF_INSRC_NETTV,
    TSAF_INSRC_GATEWAY,
    TSAF_INSRC_SPLIT_GATEWAY,
    TSAF_INSRC_AGGRE_GATEWAY,

}TSAF_INSRC_TYPE;

typedef enum _TSAF_SCAN_TYPE_TAG
{
    TSAF_SCAN_UNKNOW    = 0,
    TSAF_SCAN_AUTO,
    TSAF_SCAN_MANUAL,

}TSAF_SCAN_TYPE;

/**
 *  country id for frequency definitions
 **/
typedef enum _TSAF_COUNTRY_ID_TAG
{
    TSAF_COUNTRY_UNKNOW          = 0,
    TSAF_COUNTRY_AUSTRALIA,
    TSAF_COUNTRY_AUSTRIA,
    TSAF_COUNTRY_CHINA,
    TSAF_COUNTRY_FRANCE,
    TSAF_COUNTRY_GERMANY,
    TSAF_COUNTRY_GREECE,
    TSAF_COUNTRY_HUNGARY,
    TSAF_COUNTRY_ITALY,
    TSAF_COUNTRY_NETHERLANDS,
    TSAF_COUNTRY_POLAND,
    TSAF_COUNTRY_PORTUGAL,
    TSAF_COUNTRY_RUSSIAN,
    TSAF_COUNTRY_SPAIN,
    TSAF_COUNTRY_TAIWAN,
    TSAF_COUNTRY_UK,
    TSAF_COUNTRY_DENMARK,
    TSAF_COUNTRY_SWEDEN,
    TSAF_COUNTRY_FINLAND,

    TSAF_COUNTRY_CNT,

}TSAF_COUNTRY_ID;

/**
 *  control cmd
 **/
typedef enum _TSAF_CTRL_CMD_TAG
{
    TSAF_CTRL_UNKNOW             = 0,
    TSAF_CTRL_GET_CMD_STREAM     = 1,
    
}_TSAF_CTRL_CMD; 
//=============================================================================
//                Macro Definition
//=============================================================================


//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct _TSAF_ARG_TAG
{
    uint32_t    type;
    
    union{
        struct {
            uint8_t     *pStream_buf;
            uint32_t    stream_length;
        }get_cmd_stream;
    }args;
    
}TSAF_ARG;


typedef struct _TSAF_COUNTRY_INFO_TAG
{
    uint32_t             countryId;
    char                 country_name[32];
}TSAF_COUNTRY_INFO;

typedef struct _TSAF_SETUP_INFO_TAG
{
    pthread_mutex_t     mutex;

    TSAF_INSRC_TYPE     insrcType;
    
    // to integrate with aggregation module
    uint32_t            tsi_index;

}TSAF_SETUP_INFO;

typedef struct _TSAF_SCAN_PARAM_TAG
{
    TSAF_SCAN_TYPE      scan_type;
    TSAF_COUNTRY_ID     country_id;

    uint32_t            start_freq;
    uint32_t            bandwidth;
    uint32_t            end_freq;
    uint32_t            times_cnt;

}TSAF_SCAN_PARAM;

typedef struct _TSAF_HANDLE_TAG
{
    uint32_t        demodId;

    TSAF_STATE      state;

    int                 ts_country_cnt;
    TSAF_COUNTRY_INFO   ts_country_table[TSAF_COUNTRY_CNT-1];

    void            *privData;

}TSAF_HANDLE;

//=============================================================================
//                Global Data Definition
//=============================================================================


//=============================================================================
//                Private Function Definition
//=============================================================================


//=============================================================================
//                Public Function Definition
//=============================================================================
TSAF_ERR
tsaf_CreateHandle(
    TSAF_HANDLE         **pHTsaf,
    TSAF_SETUP_INFO     *pSetupInfo,
    void                *extraData);


TSAF_ERR
tsaf_DestroyHandle(
    TSAF_HANDLE     **pHTsaf);


TSAF_ERR
tsaf_Init_Ts(
    TSAF_HANDLE     **pHTsaf,
    void            *extraData);


TSAF_ERR
tsaf_deInit_Ts(
    TSAF_HANDLE     **pHTsaf,
    void            *extraData);


TSAF_ERR
tsaf_Open(
    TSAF_HANDLE     **pHTsaf,
    uint32_t        service_index,
    void            *extraData);


TSAF_ERR
tsaf_Close(
    TSAF_HANDLE     **pHTsaf,
    void            *extraData);


TSAF_ERR
tsaf_Read(
    TSAF_HANDLE     **pHTsaf,
    uint8_t         *pDstBuf,
    int             length,
    uint32_t        *realLength,
    void            *extraData);


TSAF_ERR
tsaf_Write(
    TSAF_HANDLE     **pHTsaf,
    void            *extraData);


TSAF_ERR
tsaf_Scan_Freq(
    TSAF_HANDLE     **pHTsaf,
    TSAF_SCAN_PARAM *pScanParam,
    void            *extraData);


TSAF_ERR
tsaf_Control(
    TSAF_HANDLE     **pHTsaf,
    uint32_t        cmd,
    uint32_t        *value,
    void            *extraData);


#ifdef __cplusplus
}
#endif

#endif

