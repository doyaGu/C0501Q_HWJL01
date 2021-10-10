#ifndef __TS_EPG_INFO_H_3QA6F9Z2_9WEN_XY5D_S122_996ET4UM25M8__
#define __TS_EPG_INFO_H_3QA6F9Z2_9WEN_XY5D_S122_996ET4UM25M8__

#ifdef __cplusplus
extern "C" {
#endif

#include "ts.h"
#include "mbt.h"
#include "ite_ts_demuxer.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define EPG_MAX_SCHEDULE_TBL    3 // defined by spec, recevice max schedule tables (one table include n days)
#define EPG_MAX_SCHEDULE_DAY    3 // for database, save schedule of N days, N = EPG_MAX_SCHEDULE_DAY

/**
 * EIT control cmd
 **/
typedef enum _TS_EPG_CTL_CMD_TAG
{
    TS_EPG_CTL_UNKNOW              = 0,
    TS_EPG_CTL_SKIP_EPG,            // skip all
    TS_EPG_CTL_SKIP_SCHEDULE,       // only skip schedule
    TS_EPG_CTL_UPDATE_TDT,          // force update TDT
    TS_EPG_CTL_PREFER_EVENT_LANG_B,
    TS_EPG_CTL_PREFER_EVENT_LANG_T,
    
}TS_EPG_CTL_CMD;


/**
 * EIT tpye 
 **/
typedef enum _TS_EIT_TYPE_TAG
{
    TS_EIT_UNKNOW           = 0,
    TS_EIT_PRESENT_FOLLOW,
    TS_EIT_SCHEDULE,
    
}TS_EIT_TYPE;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
// /**
//  * Modified Julian Day and binary-coded decimal
//  **/
// typedef struct PSI_MJDBCD_TIME_TAG
// {
//     uint32_t     BCD24bits;
//     uint32_t     MJD16bits;
// } PSI_MJDBCD_TIME;

// /**
//  * one EIT entry
//  **/
// typedef struct EIT_ENTRY_TAG
// {
//     // uint64_t            start_time;
//     PSI_MJDBCD_TIME    start_time;
//     uint32_t            duration;
// 
//     TSD_EPG_USER_INFO   userInfo;
// 
// } EIT_ENTRY;

typedef struct ISO_639_LANG_TAG
{
    char    charCode[4];
}ISO_639_LANG;

typedef struct ISO_3166_COUNTRY_CODE_TAG
{
    char    charCode[4];
}ISO_3166_COUNTRY_CODE;

/**
 * a node in modified binary tree
 **/
typedef struct EIT_MBT_NODE_TAG
{
    MBT_NODE            mbt_node;
    
    // uint64_t            start_time; // hight_32 bits: MJD, low_32 bits: BCD 
    PSI_MJDBCD_TIME     start_time;
    uint32_t            duration;

    TSD_EPG_USER_INFO   userInfo;    

}EIT_MBT_NODE;

/**
 * total EITs in one day
 **/
typedef struct _EPG_1_DAY_SCHEDULE_TAG
{    
    uint32_t        mjd_time;
    uint32_t        total_eits;   
    
    // eit mbt root node
    EIT_MBT_NODE    *pEitMbtRoot;
    
}EPG_1_DAY_SCHEDULE;

/**
 * total EITs in a service
 **/
typedef struct TS_1_SRVC_EIT_TAG
{
    struct TS_1_SRVC_EIT_TAG   *next, *prev;
    
    // for verify
    uint32_t            frequency; 
    uint32_t            service_id;  // == programNumber 
    uint32_t            version_num; // update or not

    uint32_t            total_eits;
    
    // for present/following
    uint8_t             version_num_p_f;
    uint8_t             rx_section_p_f;
    // EIT_MBT_NODE        *pEitMbtRoot_p_f; // recode present/following ??
    
    // for schedule
    uint8_t             version_num_sch[EPG_MAX_SCHEDULE_TBL];
    uint32_t            rx_section_sch[EPG_MAX_SCHEDULE_TBL][8];    

    // epg database by day
    EPG_1_DAY_SCHEDULE  epg_1_day[EPG_MAX_SCHEDULE_DAY];

}TS_1_SRVC_EIT;

typedef struct TS_EPG_HANDLE_TAG
{    
    PSI_MJDBCD_TIME     curUtcTime;

    bool            bRefreshed_TDT; // to notice AP that TDT is updated
    bool            bRefreshed_TOT; // to notice AP that TOT is updated
    
    ISO_639_LANG    preferEventLangB;
    ISO_639_LANG    preferEventLangT;
    
    uint32_t        totalSrvc;

}TS_EPG_HANDLE;

//=============================================================================
//                  Global Data Definition
//=============================================================================


//=============================================================================
//                  Private Function Definition
//=============================================================================
bool
tsEpg_Eit_SectionFilter_CallBack(
    TS_DEMUX        *ptDemux,
    PSI_SECTION     *ptSection);
    
    
void
tsEpg_Eit_P_F_CallBack(
    TS_DEMUX        *ptDemux,
    PSI_EIT_INFO    *ptEitInfo);


void
tsEpg_Eit_Schedule_CallBack(
    TS_DEMUX        *ptDemux,
    PSI_EIT_INFO    *ptEitInfo);    


void
tsEpg_Tdt_CallBack(
    TS_DEMUX        *ptDemux,
    PSI_MJDBCD_TIME tUtcTime);
    

void
tsEpg_Tot_CallBack(
    TS_DEMUX        *ptDemux,
    PSI_TOT_INFO    *ptTotInfo);    
//=============================================================================
//                  Public Function Definition
//=============================================================================
TSD_ERR
tsEpg_CreateHandle(
    TS_EPG_HANDLE   **pHEpg,
    void            *extraData);
    
    
TSD_ERR
tsEpg_DestroyHandle(
    TS_EPG_HANDLE   **pHEpg);
    

TSD_ERR
tsEpg_Change_Service(
    TS_EPG_HANDLE      *pHEpg, 
    uint32_t           frequency,
    uint32_t           service_id,
    void               *extraData);


TSD_ERR
tsEpg_Get_Service_Schedule(
    TS_EPG_HANDLE      *pHEpg, 
    uint32_t           frequency,
    uint32_t           service_id,
    TS_1_SRVC_EIT      **pOne_srvc_eits,
    void               *extraData);


TSD_ERR
tsEpg_Control(
    TS_EPG_HANDLE      *pHEpg,
    TS_EPG_CTL_CMD     cmd,
    uint32_t            *value,
    void                *extraData);


    
#ifdef __cplusplus
}
#endif

#endif
