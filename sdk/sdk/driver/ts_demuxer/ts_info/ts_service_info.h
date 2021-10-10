#ifndef __TS_SERVICE_INFO_H_C6RVJNGN_F3IR_E66I_ZC42_AO804QX2B4IW__
#define __TS_SERVICE_INFO_H_C6RVJNGN_F3IR_E66I_ZC42_AO804QX2B4IW__

#ifdef __cplusplus
extern "C" {
#endif


#include "ts.h"
#include "ite_ts_demuxer.h"
#include "ts_channel_info.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
typedef enum _TS_SRVC_CTL_CMD_TAG
{
    TS_SRVC_CTL_UNKNOW              = 0,
    TS_SRVC_CTL_SET_TSD_CTRL_STATUS,
    TS_SRVC_CTL_SKIP_PSI_INFO,
    TS_SRVC_CTL_SET_PID_STAT_CB_INFO,
    TS_SRVC_CTL_SET_CHANNEL_INFO,
    TS_SRVC_CTL_RESET_PAT_STATUS,
    TS_SRVC_CTL_RESET_SDT_STATUS,
    TS_SRVC_CTL_GET_SRVC_CNT_IN_CHNL,
    TS_SRVC_CTL_GET_TOTAL_SRVC,
    TS_SRVC_CTL_DEL_ALL_SRVC_INFO,
    TS_SRVC_CTL_MERGE_SRVC_INFO,
    TS_SRVC_CTL_SET_CTL_STATUS,
    
}TS_SRVC_CTL_CMD;

typedef enum _TS_SRVC_AUD_TYPE_TAG
{
    TS_SRVC_AUD_UNKNOW  = 0,
    TS_SRVC_AUD_MP3     = 0xA0000EB3,
    TS_SRVC_AUD_AC3     = 0xA0000AC3,
    TS_SRVC_AUD_AAC     = 0xA0000AAC,
    
} TS_SRVC_AUD_TYPE;

typedef enum _TS_SRVC_MERGE_TYPE_TAG
{
    TS_SRVC_MERGE_DEFAULT = 0, // SDT and PMT all match
    TS_SRVC_MERGE_BASE_PMT,    // reserve, no implement
    TS_SRVC_MERGE_BASE_SDT,    // reserve, no implement
    TS_SRVC_MERGE_2_HANDLE,    // for dual demod
    
} TS_SRVC_MERGE_TYPE;
//=============================================================================
//				  Macro Definition
//=============================================================================

//=============================================================================
//				  Structure Definition
//=============================================================================
/**
 * Ts PID analysis data for ts_parser callback
 **/
typedef struct TS_PID_ANAL_DATA_TAG
{
    uint32_t    service_order_num;//serviceIdx;
    uint32_t    pid;
    bool        bVideo;
    
}TS_PID_ANAL_DATA;

// callback function 
typedef TSD_ERR (*PID_STATISTICS_CALLBACK)(TS_PID_ANAL_DATA *newData, void  *extraData);

typedef struct TS_SRVC_PID_STAT_CB_INFO_TAG
{
    void                        *pStatInfo;
    PID_STATISTICS_CALLBACK     pfStat_CB;
    
}TS_SRVC_PID_STAT_CB_INFO;

/**
 * ts parser service info
 **/
typedef struct TS_SERVICE_INFO_TAG
{
    struct TS_SERVICE_INFO_TAG  *next, *prev;
    uint32_t                    order_num;
    
    bool                bHidden;
    uint32_t            version_number;
    uint32_t            programNumber;    
    uint32_t            videoPID;
    uint32_t            videoType;
    uint32_t            audioPID[SERVICE_MAX_AUDIO_COUNT];
    TS_SRVC_AUD_TYPE    audioType[SERVICE_MAX_AUDIO_COUNT];
    uint32_t            subtitlePID[SERVICE_MAX_SUBTITLE_COUNT];
    
    TSD_SRVC_USER_INFO  userInfo;

    // channel info
    TS_CHNL_INFO        tsChnlInfo;

    // PMT info
    uint32_t            pmt_pid;
} TS_SERVICE_INFO;

typedef struct TS_SRVC_HANDLE_TAG
{
    TS_SERVICE_INFO     *pStartSrvcInfo;
    TS_SERVICE_INFO     *pCurSrvcInfo;
    
    bool                bReady_PAT; // for feedback to ts demuxer
    bool                bReady_PMT; // for feedback to ts demuxer
    bool                bReady_SDT; // for feedback to ts demuxer
    
    uint32_t            totalSrvc;

}TS_SRVC_HANDLE;

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
tsSrvc_CreateHandle(
    TS_SRVC_HANDLE  **pHSrvc,
    void            *extraData);
    
    
TSD_ERR
tsSrvc_DestroyHandle(
    TS_SRVC_HANDLE  **pHSrvc);


TSD_ERR
tsSrvc_AddService(
    TS_SRVC_HANDLE  *pHSrvc,
    TS_SERVICE_INFO *pInsSrvcInfo,
    uint32_t        *curSrvcIdx,
    void            *extraData);


TSD_ERR
tsSrvc_GetServiceInfo(
    TS_SRVC_HANDLE      *pHSrvc,
    uint32_t            orderIdx,
    TS_SERVICE_INFO     *pSrvc_info_start,
    TS_SERVICE_INFO     **pSrvc_info,
    void                *extraData);
    

TSD_ERR
tsSrvc_Control(
    TS_SRVC_HANDLE      *pHSrvc,
    TS_SRVC_CTL_CMD     cmd,
    uint32_t            *value,
    void                *extraData);

/////////////////////////////////////////////
// psi callback for collecting data
/////////////////////////////////////////////    
void
tsSrvc_PatCallBack(
    TS_DEMUX*       ptDemux,
    PSI_PAT_INFO*   ptPatInfo);


void
tsSrvc_PmtCallBack(
    TS_DEMUX*       ptDemux,
    PSI_PMT_INFO*   ptPmtInfo);
    
    
void
tsSrvc_SdtCallBack(
    TS_DEMUX*       ptDemux,
    PSI_SDT_INFO*   ptSdtInfo);

    
#ifdef __cplusplus
}
#endif

#endif
