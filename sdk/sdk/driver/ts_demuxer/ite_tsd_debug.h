#pragma once
#ifndef __ite_ts_debug_h_idE66D4318_C3C7_4485_9BC1EC5DF81537CB__
#define __ite_ts_debug_h_idE66D4318_C3C7_4485_9BC1EC5DF81537CB__

#include "ts_parser.h"
#include "ts_service_info.h"
#include "ts_channel_info.h"
#include "global_freq_info.h"
#include "ts_epg_info.h"
#include "ts_txt_conv.h"

#include "ts_demuxer_defs.h"

//=============================================================================
//				  Structure Definition
//=============================================================================
//-----------------------------------
// tsd internal 
typedef struct _ITE_TSD_DEV_DBG_TAG
{
    TSD_HANDLE          hTsd;       // communicate to user

    uint32_t            tsdStatus;  // globle status
    TSD_OUT_MODE        tsdOutMod;  // output filtered ts stream or PES packets

    // demode handle
    DEMOD_CTRL_HANDLE   *pHDemodCtrl;

    // ts parser handle
    TSP_HANDLE          *pHTsp;

    // service info handle
    TS_SRVC_HANDLE      *pHTsSrvc;

    // channel info handle
    TS_CHNL_HANDLE      *pHTsChnl;

    // for freq scan
    TSD_SCAN_STATE      scanState;
    
    // for txt conver
    TS_CHAR_CODE        charCode;

    // for setting tsi
    uint32_t            tsiId;

    // for setting demod
    uint32_t            demod_id;

    // for sort
    TSD_ACT_SORT_TABLE  actSortTbl;
    TSD_SRVC_USER_INFO  **pSrvcUserInfo; // pointer array
    TSD_CHNL_USER_INFO  **pChnlUserInfo; // pointer array

}ITE_TSD_DEV_DBG;

//-----------------------------------
// ts service internal 
typedef struct _TS_SRVC_DB_DBG_TAG
{
    TS_SRVC_HANDLE      hSrvc;
    
    uint32_t            srvcStatus;

}TS_SRVC_DB_DBG;

//-----------------------------------
// ts chanbel internal
typedef struct _TS_CHNL_DB_DBG_TAG
{
    TS_CHNL_HANDLE      hChnl;
    
    uint32_t            chnlStatus;
    
    uint32_t            totalChnls;
    uint32_t            order_num;

}TS_CHNL_DB_DBG;

//-----------------------------------
// ts epg internal

//=============================================================================
//				  Macro Definition
//=============================================================================




#endif // header
