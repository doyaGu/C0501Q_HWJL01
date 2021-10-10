#ifndef __TS_CHANNEL_INFO_H_STERFYSG_XZWB_OPKJ_8SDO_GY5FUPUE8XXU__
#define __TS_CHANNEL_INFO_H_STERFYSG_XZWB_OPKJ_8SDO_GY5FUPUE8XXU__

#ifdef __cplusplus
extern "C" {
#endif


#include "ite_ts_demuxer.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
#define INFO_MAX_TRANSPORT_STREAM_ID_ENTRY  (4)
#define INFO_MAX_NIT_NETWORK_ENTRY          (8)
#define INFO_MAX_FREQUENCY_ENTRY            (32)

#define INVALID_ONID_NUMBER                 (0xFFFFFFFF)
#define INVALID_CHANNEL_INDEX               (0xFFFFFFFF)
#define INVALID_VERSION_NUMBER              (0xFFFFFFFF)


/**
 * channel info control cmd
 **/
typedef enum _TS_CHNL_CTL_CMD_TAG
{
    TS_CHNL_CTL_UNKNOW              = 0,
    TS_CHNL_CTL_SAVE_INFO,
    TS_CHNL_CTL_DEL_ALL_CHNL_INFO,    
    TS_CHNL_CTL_MERGE_CHNL_INFO,
    
}TS_CHNL_CTL_CMD;

// modify for new scan
typedef enum TS_CHNL_SDT_STATE_TAG
{
    TS_CHNL_SDT_SCANNING   = 0, 
    TS_CHNL_SDT_RECEIEVED,
    TS_CHNL_SDT_NO_DATA,

}TS_CHNL_SDT_STATE;

typedef enum TS_CHNL_MERGE_TYPE_TAG
{
    TS_CHNL_MERGE_DEFAULT = 0, // SDT and PMT all match
    TS_CHNL_MERGE_2_HANDLE,    // for dual demod
    
} TS_CHNL_MERGE_TYPE;

//=============================================================================
//				  Macro Definition
//=============================================================================


//=============================================================================
//				  Structure Definition
//=============================================================================
/**
 * network around location could be changed when moving
 * use hit bit to maintain this network list
 * delete the entries that have not hit
 **/
typedef struct TS_CHNL_NIT_NETOWRK_ENTRY_TAG
{
    /*
     * MSB 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 LSB
     *     ~~                          ~~~~~~~~~
     *     hit bit                     version_number
     **/
    uint16_t    nitTblInfo;
    uint16_t    network_id;
}TS_CHNL_NIT_NETWORK_ENTRY;

typedef struct TS_CHNL_NIT_INFO_TAG
{
    TS_CHNL_NIT_NETWORK_ENTRY  ptNetworkList[INFO_MAX_NIT_NETWORK_ENTRY];
    uint32_t                  networkCnt;
    uint32_t                  networkIdx;

    /**
     * TRANSPORT STREAM ID LIST FORMAT
     * MSB 31 --------------------- 16, 15 ------------------ 0 LSB
     *        ~~~~~~~~~~~~~~~~~~~~~~       ~~~~~~~~~~~~~~~~~~
     *                     bandwidth       transport_stream_id
     **/
    uint32_t                  ptTransportStreamIdList[INFO_MAX_TRANSPORT_STREAM_ID_ENTRY];
    uint32_t                  transportStreamCnt;

    /**
     * FREQUENCY LIST ENTRY FORMAT
     *              transport_stream_id entry index(4 bits)
     *              ~~~~24
     * MSB 31 ---------------------- 24, 23 ----------- 0 LSB
     *        ~~~~28
     *        network entry id index        frequency
     **/
    uint32_t                  ptFreqList[INFO_MAX_FREQUENCY_ENTRY];

    uint32_t                  freqCnt;
    uint32_t                  freqIdx;
}TS_CHNL_NIT_INFO;

typedef struct TS_CHNL_INFO_TAG
{
    struct TS_CHNL_INFO_TAG  *next, *prev;
    uint32_t                 order_num;
    
    bool            bHidden;
    
    TSD_CHNL_USER_INFO  userInfo;    
    
    // uint32_t        original_network_id;
    // uint32_t        transport_stream_id;

    uint32_t        patVersion;  // when change channel, it can decide update info or not.
    uint32_t        sdtVersion;  // when change channel, it can decide update info or not.
    
    // uint32_t        tvPmtCnt;
    // uint32_t        rdPmtCnt;
    // uint32_t        tvSdtCnt;
    // uint32_t        rdSdtCnt;
    
    // uint32_t        tvServiceCnt; // modify for new scan
    // uint32_t        rdServiceCnt; // modify for new scan
    // TS_CHNL_SDT_STATE sdtState;   // modify for new scan

    // TS_CHNL_NIT_INFO   tNitInfo;
    // bool            bNitReady;

    // uint16_t        pPmtPidTbl[64];

}TS_CHNL_INFO;


typedef struct TS_CHNL_HANDLE_TAG
{
    TS_CHNL_INFO        *pStartChnlInfo;
    TS_CHNL_INFO        *pCurChnlInfo;
    
    uint32_t            totalChnls;
    
}TS_CHNL_HANDLE;

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
tsChnl_CreateHandle(
    TS_CHNL_HANDLE  **pHChnl,
    void            *extraData);


TSD_ERR
tsChnl_DestroyHandle(
    TS_CHNL_HANDLE  **pHChnl);


TSD_ERR
tsChnl_GetChannelInfo(
    TS_CHNL_HANDLE      *pHChnl,
    uint32_t            orderIdx,
    TS_CHNL_INFO        *pChnl_info_start,
    TS_CHNL_INFO        **pChnl_info,
    void                *extraData);


TSD_ERR
tsChnl_AddChannel(
    TS_CHNL_HANDLE  *pHChnl,
    TS_CHNL_INFO    *pInsChnlInfo,
    uint32_t        *curChnlIdx,
    void            *extraData);


TSD_ERR
tsChnl_Control(
    TS_CHNL_HANDLE  *pHChnl,
    TS_CHNL_CTL_CMD cmd,
    uint32_t        *value,
    void            *extraData);


#ifdef __cplusplus
}
#endif

#endif
