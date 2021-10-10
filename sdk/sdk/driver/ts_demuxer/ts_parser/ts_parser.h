#ifndef __TS_PARSER_H_O0V6H70M_Z3PC_0UKI_COV4_HFSSBW2B5JPD__
#define __TS_PARSER_H_O0V6H70M_Z3PC_0UKI_COV4_HFSSBW2B5JPD__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "ts_demuxer_defs.h"
#include "ts_epg_info.h"
#include "ts_service_info.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
#define MAX_FILE_SERVICE_COUNT             64
#define MAX_ANALYSIS_PID_COUNT             256

/**
 * ts parser control cmd
 **/
typedef enum _TSP_CTRL_CMD_TAG
{
    TSP_CTRL_UNKNOW     = 0,
    TSP_CTRL_RESET,
    TSP_CTRL_ATTACH_PID_STATISTICS_CB,
    TSP_CTRL_ATTACH_EPG_HANDLE,
    TSP_CTRL_SET_TSD_CTL_STATUS,
    TSP_CTRL_ATTACH_EIT_CB,
    TSP_CTRL_REGEN_EIT_HANDLE,
    TSP_CTRL_CAL_PACKET_SIZE,
    TSP_CTRL_ENABLE_PID,
    TSP_CTRL_DISABLE_PID,
    TSP_CTRL_SET_CHNL_INFO,
    
}TSP_CTRL_CMD;
//=============================================================================
//				  Macro Definition
//=============================================================================


//=============================================================================
//				  Structure Definition
//=============================================================================
/**
 * ts parser PID data statistic
 **/
typedef struct PID_DATA_STAT_TAG
{
    uint32_t    fileServiceIndex;
    uint32_t    bVideo;
    uint32_t    pid;
    uint32_t    pidDataCount;
} PID_DATA_STAT;

/**
 * base unit for strung component
 **/
typedef struct _TUNNEL_INFO_TAG
{
    //pthread_mutex_t     mutex;
    void                *handle;
}TUNNEL_INFO;

/**
 * ts parser tunnel info for database
 **/
typedef struct _TSP_TUNNEL_INFO_TAG
{
    // for ts service info
    TUNNEL_INFO     srvc_tunnel_info;
    
    // for ts epg info
    TUNNEL_INFO     epg_tunnel_info;

    // for pes output info
    bool            bOnPesOut; // enable pes output
    TSD_BUF_INFO    pesOutBuf_a; // pes audio packet    
    TSD_BUF_INFO    pesOutBuf_v; // pes video packet
    TSD_BUF_INFO    pesOutBuf_s; // pes subtitle packet      
    TSD_BUF_INFO    pesOutBuf_t; // pes teletext packet

}TSP_TUNNEL_INFO;

typedef struct _TSP_PES_SAMPLE_INFO_TAG
{
    TSD_SAMPLE_TYPE     sampleType;
    uint8_t             *pSampleAddr;
    uint32_t            sampleSize;

}TSP_PES_SAMPLE_INFO;

typedef struct TSP_FILE_INFO_TAG
{
    // TS_SERVICE_INFO     ptFileService[MAX_FILE_SERVICE_COUNT];
    TS_SERVICE_INFO     *pCurSrvcInfo;
    PID_DATA_STAT       pPidStatistics[MAX_ANALYSIS_PID_COUNT];
    uint32_t            pesPidCount;
    uint32_t            serviceCount;
    int32_t             activeServiceIndex;
    uint32_t            fileSize;
    uint32_t            totalMs;  // total ms 
} TSP_PARSER_INFO;


typedef struct _TSP_HANDLE_TAG
{
    uint32_t            bRingBuf;
    uint8_t             *dataBuf;
    uint32_t            dataSize;

    // link channel info handle 
    TS_CHNL_INFO        tsChnlInfo;
    
    TSP_PARSER_INFO     *pTspPrsInfo;

    // pes output

}TSP_HANDLE;


//=============================================================================
//				  Global Data Definition
//=============================================================================


//=============================================================================
//				  Private Function Definition
//=============================================================================


//=============================================================================
//				  Public Function Definition
//=============================================================================
TSD_ERR
tsp_CreateHandle(
    TSP_HANDLE          **pHTsp,
    bool                bCollectEit,
    TSP_TUNNEL_INFO     *pTspTunnelInfo,
    void                *extraData);
    

TSD_ERR
tsp_DestroyHandle(
    TSP_HANDLE  **pHTsd);
    

TSD_ERR
tsp_ParseStream(
    TSP_HANDLE      *pHTsp,
    void            *extraData);


TSD_ERR
tsp_GetNextSample(
    TSP_HANDLE          *pHTsp,
    TSP_PES_SAMPLE_INFO *pPesSample,
    void                *extraData);


TSD_ERR
tsp_Control(
    TSP_HANDLE      *pHTsp,
    TSP_CTRL_CMD    cmd,
    uint32_t        *value,
    void            *extraData);
    
#ifdef __cplusplus
}
#endif

#endif
