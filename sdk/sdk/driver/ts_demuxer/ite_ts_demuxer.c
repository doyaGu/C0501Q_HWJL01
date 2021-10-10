
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ts_parser.h"
#include "ts_channel_info.h"
#include "global_freq_info.h"
#include "ts_epg_info.h"
#include "demod_ctrl.h"
#include "ts_demuxer_defs.h"
#include "ite_ts_demuxer.h"
#include "ts_txt_conv.h"

#if !(ENABLE_SW_SIMULATION)
    #include "ite/itp.h"
    #include "mmp_tsi.h"
    #include "mmp_iic.h"
#endif
//=============================================================================
//                Constant Definition
//=============================================================================
typedef enum _TSD_STATUS_TAG
{
    TSD_STATUS_IDLE = 0x11,
    TSD_STATUS_BUSY = 0xBB,
    TSD_STATUS_FAIL = 0xFF,

}TSD_STATUS;

/**
 * action table to sort
 **/
typedef enum _TSD_ACT_SORT_TABLE_TAG
{
    TSD_ACT_SORT_NONE       = 0,
    TSD_ACT_SORT_USER_SRVC_INFO,
    TSD_ACT_SORT_USER_CHNL_INFO,

}TSD_ACT_SORT_TABLE;
//=============================================================================
//                Macro Definition
//=============================================================================
#if _MSC_VER
    #include "locale.h"
    #define _show_utf16_txt(utf16_txt) \
                do{ setlocale(LC_ALL, "cht");   \
                    wprintf(L"%s\n", utf16_txt); \
                }while(0)
#elif 0
    #include "ts_demuxer/ts_debug.h"
    #define _show_utf16_txt(utf16_txt)      \
                do{ char utf8_txt[256] = {0};   \
                    unsigned int bytes_used = 0;    \
                    utf16le_to_utf8(utf16_txt, utf8_txt, 256, &bytes_used); \
                    printf("%s", utf8_txt);    \
                }while(0)
#else
    #define _show_utf16_txt(utf16_txt)
#endif

#if 0
    #include "ts_demuxer/ts_debug.h"
    #define _show_dvb_txt(dvb_txt, dvb_txt_length) \
                do{ char utf8_txt[256] = {0};   \
                    uint16_t utf16_txt[256] = {0}; \
                    unsigned int bytes_used = 0;    \
                    tsTxt_ConvToUtf16(utf16_txt, dvb_txt, dvb_txt_length, 0); \
                    utf16le_to_utf8(utf16_txt, utf8_txt, 256, &bytes_used); \
                    printf("%s\n", utf8_txt);    \
                }while(0)
#else
    #define _show_dvb_txt(dvb_txt, dvb_txt_length)
#endif
//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct _ITE_TSD_DEV_TAG
{
    TSD_HANDLE          hTsd;       // communicate to user

    pthread_mutex_t     tsd_data_mutex;

    TSD_STATUS          tsdStatus;  // globle status
    TSD_OUT_MODE        tsdOutMod;  // output filtered ts stream or PES packets
    bool                bStopScanProc;
    bool                bSkipEpgParsing;

    TSD_TS_RX_INFO      tsRecevier;

    // demode handle
    DEMOD_CTRL_HANDLE   *pHDemodCtrl;
    bool                bDemodSuspend;

    // ts parser handle
    TSP_HANDLE          *pHTsp;
    bool                bOnPesOutput;
    TSD_BUF_INFO        pesOutBuf_a; // pes audio packet
    TSD_BUF_INFO        pesOutBuf_v; // pes video packet
    TSD_BUF_INFO        pesOutBuf_s; // pes subtitle packet
    TSD_BUF_INFO        pesOutBuf_t; // teletext pid info

    // service info handle
    TS_SRVC_HANDLE      *pHTsSrvc;

    // sound track
    bool                bSoundFullOut; // no filter sound

    // epg info
    TS_EPG_HANDLE       *pHEpg;
    bool                bResetEitPsi;

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

}ITE_TSD_DEV;

//=============================================================================
//                Global Data Definition
//=============================================================================
uint32_t  tsdMsgOnFlag = (0x1);// | TSD_MSG_TYPE_TRACE_TSD | TSD_MSG_TYPE_TRACE_EPG); // | TSD_MSG_TYPE_TRACE_SRVC);

#if (CFG_DEMOD_SUPPORT_COUNT > 0)
    #if (CFG_DEMOD_SUPPORT_COUNT > 3)
        #define TSD_MAX_DEMOD_CNT         4
    #else
        #define TSD_MAX_DEMOD_CNT         CFG_DEMOD_SUPPORT_COUNT // 4
    #endif
#else
    #define TSD_MAX_DEMOD_CNT         1
#endif

static DEMOD_ATTR g_demod_attr[TSD_MAX_DEMOD_CNT] =
{
#define TSD_DEMOD_ATTR_SET(demod_chip,demod_index,demod_bus_type,demod_i2c_addr,linked_tsi_index,privData)\
                {demod_index, demod_bus_type, demod_chip, demod_i2c_addr, privData},
#include "ts_demuxer_attr_set.h"
};

//=============================================================================
//                Private Function Definition
//=============================================================================
static int
_sort_freq(
    const void *parg0,
    const void *parg1)
{
    TSD_SRVC_USER_INFO  *pSrvcUserInfo_0 = *(TSD_SRVC_USER_INFO**)parg0;
    TSD_SRVC_USER_INFO  *pSrvcUserInfo_1 = *(TSD_SRVC_USER_INFO**)parg1;
    TS_SERVICE_INFO     *pSrvcInfo_0 = DOWN_CAST(TS_SERVICE_INFO, pSrvcUserInfo_0, userInfo);
    TS_SERVICE_INFO     *pSrvcInfo_1 = DOWN_CAST(TS_SERVICE_INFO, pSrvcUserInfo_1, userInfo);

    if( pSrvcInfo_0->tsChnlInfo.userInfo.frequency != pSrvcInfo_1->tsChnlInfo.userInfo.frequency )
        return ((int)(pSrvcInfo_0->tsChnlInfo.userInfo.frequency - pSrvcInfo_1->tsChnlInfo.userInfo.frequency));
    else
        return ((int)(pSrvcInfo_0->programNumber - pSrvcInfo_1->programNumber));
}

static int
_sort_name(
    const void *parg0,
    const void *parg1)
{
    TSD_SRVC_USER_INFO  *pSrvcUserInfo_0 = *(TSD_SRVC_USER_INFO**)parg0;
    TSD_SRVC_USER_INFO  *pSrvcUserInfo_1 = *(TSD_SRVC_USER_INFO**)parg1;

    return ((int)strncmp(pSrvcUserInfo_0->serviceName, pSrvcUserInfo_1->serviceName, TSD_MAX_SRVC_NAME_SIZE));
}

static void
_Sort_Func(
    ITE_TSD_DEV         *pTsdDev,
    TSD_INFO_SORT_TYPE  sortType)
{
    switch( sortType )
    {
        // ------- sort frequency
        case TSD_INFO_SORT_FREQ:
            switch( pTsdDev->actSortTbl )
            {
                case TSD_ACT_SORT_USER_SRVC_INFO:
                    qsort((void*)(pTsdDev->pSrvcUserInfo),
                          pTsdDev->hTsd.totalSrvc,
                          sizeof(TSD_SRVC_USER_INFO*),
                          _sort_freq);
                    break;
            }
            break;

        // -------- sort name
        case TSD_INFO_SORT_SRVC_NAME:
            switch( pTsdDev->actSortTbl )
            {
                case TSD_ACT_SORT_USER_SRVC_INFO:
                    qsort((void*)(pTsdDev->pSrvcUserInfo),
                          pTsdDev->hTsd.totalSrvc,
                          sizeof(TSD_SRVC_USER_INFO*),
                          _sort_name);
                    break;
            }
            break;
    }
}

static int
_EPG_PF_Compare(
    MBT_NODE    *node_curr,
    MBT_NODE    *node_pattern,
    void        *param)
{
    int             result = 0;
    EIT_MBT_NODE    *pEit_node_a = mbt_get_struct(EIT_MBT_NODE, mbt_node, node_curr);
    EIT_MBT_NODE    *pEit_node_b = mbt_get_struct(EIT_MBT_NODE, mbt_node, node_pattern);
    PSI_MJDBCD_TIME endTime = {0};

    endTime = psiTime_Add(pEit_node_a->start_time, pEit_node_a->duration);
    if( PSI_MJDBCD_TIME_LESS(pEit_node_a->start_time, pEit_node_b->start_time) &&
        PSI_MJDBCD_TIME_LESS(endTime, pEit_node_b->start_time) )
        result = 1;
    else if( PSI_MJDBCD_TIME_LARGER(pEit_node_a->start_time, pEit_node_b->start_time) )
        result = -1;


#if 0
    {
        PSI_YMDHMS_TIME     YmdHmsTime = {0};
        PSI_MJDBCD_TIME     end_Time = {0};
        char                ascii_txt[256] = {0};

        end_Time = psiTime_Add(pEit_node_a->start_time, pEit_node_a->duration);

        YmdHmsTime = psiTime_MjdBcdToYmdHms(pEit_node_b->start_time);
        sprintf(ascii_txt, "%04d-%02d-%02d %02d:%02d:%02d",
                YmdHmsTime.year, YmdHmsTime.month, YmdHmsTime.day,
                YmdHmsTime.hour, YmdHmsTime.minute, YmdHmsTime.second);
        printf("result= %d, cTime = %s, ", result, ascii_txt);

        YmdHmsTime = psiTime_MjdBcdToYmdHms(pEit_node_a->start_time);
        sprintf(ascii_txt, "%04d-%02d-%02d %02d:%02d:%02d",
                YmdHmsTime.year, YmdHmsTime.month, YmdHmsTime.day,
                YmdHmsTime.hour, YmdHmsTime.minute, YmdHmsTime.second);
        printf("sTime = %s, ", ascii_txt);
        YmdHmsTime = psiTime_MjdBcdToYmdHms(end_Time);
        sprintf(ascii_txt, "%04d-%02d-%02d %02d:%02d:%02d",
                YmdHmsTime.year, YmdHmsTime.month, YmdHmsTime.day,
                YmdHmsTime.hour, YmdHmsTime.minute, YmdHmsTime.second);
        printf("eTime = %s\n", ascii_txt);
    }
#endif
    return result;
}

static int
_Epg_Schedule_Export(
    MBT_VISIT_INFO  *visitInfo,
    MBT_NODE        *pMbt_c_node)
{
#define UTF16_TXT_BUF_MAX_SIZE      512
    TSD_INFO_REPO       *pTsdInfoRepo = 0;
    TS_CHAR_CODE        charCode;
    EIT_MBT_NODE        *pEit_node = mbt_get_struct(EIT_MBT_NODE, mbt_node, pMbt_c_node);
    uint16_t            utf16_txt[UTF16_TXT_BUF_MAX_SIZE] = {0};
    char                ascii_txt[32] = {0};
    PSI_YMDHMS_TIME     YmdHmsTime = {0};
    PSI_MJDBCD_TIME     endTime = {0};
    uint8_t             rating = 0;
    uint32_t            i, tmp_length = 0;

    _trace_enter("");

    pTsdInfoRepo = (TSD_INFO_REPO*)visitInfo->privData;
    charCode     = (TS_CHAR_CODE)visitInfo->privData_1;

    // start time/date
    pTsdInfoRepo->repoType = TSD_REPO_EPG_EVENT_START_TIME_UTF16;
    YmdHmsTime = psiTime_MjdBcdToYmdHms(pEit_node->start_time);

    snprintf(ascii_txt, 32, "%04d-%02d-%02d %02d:%02d:%02d - ",
            YmdHmsTime.year, YmdHmsTime.month, YmdHmsTime.day,
            YmdHmsTime.hour, YmdHmsTime.minute, YmdHmsTime.second);
    tmp_length = strlen(ascii_txt);
    for(i = 0; i < tmp_length; i++)
        utf16_txt[i] = ascii_txt[i];
    utf16_txt[i] = 0;

    pTsdInfoRepo->tsd_repo_export(pTsdInfoRepo, (uint8_t*)utf16_txt, (tmp_length+1)*sizeof(uint16_t));
    _show_utf16_txt(utf16_txt);

    // end time/date
    pTsdInfoRepo->repoType = TSD_REPO_EPG_EVENT_END_TIME_UTF16;
    endTime = psiTime_Add(pEit_node->start_time, pEit_node->duration);

    YmdHmsTime = psiTime_MjdBcdToYmdHms(endTime);
    sprintf(ascii_txt, "%04d-%02d-%02d %02d:%02d:%02d - ",
            YmdHmsTime.year, YmdHmsTime.month, YmdHmsTime.day,
            YmdHmsTime.hour, YmdHmsTime.minute, YmdHmsTime.second);
    tmp_length = strlen(ascii_txt);
    for(i = 0; i < tmp_length; i++)
        utf16_txt[i] = ascii_txt[i];
    utf16_txt[i] = 0;

    pTsdInfoRepo->tsd_repo_export(pTsdInfoRepo, (uint8_t*)utf16_txt, (tmp_length+1)*sizeof(uint16_t));
    _show_utf16_txt(utf16_txt);

    // event name
    pTsdInfoRepo->repoType = TSD_REPO_EPG_EVENT_NAME_UTF16;
    if( pEit_node->userInfo.nameSize && pEit_node->userInfo.pEventName )
    {
        tmp_length = tsTxt_ConvToUtf16(0, pEit_node->userInfo.pEventName, pEit_node->userInfo.nameSize, charCode);
        if( tmp_length < UTF16_TXT_BUF_MAX_SIZE )
        {
            tsTxt_ConvToUtf16(utf16_txt, pEit_node->userInfo.pEventName, pEit_node->userInfo.nameSize, charCode);
            utf16_txt[tmp_length] = 0;
            pTsdInfoRepo->tsd_repo_export(pTsdInfoRepo, (uint8_t*)utf16_txt, (1+tmp_length)*sizeof(uint16_t));
            _show_utf16_txt(utf16_txt);
        }
        else
        {
            tsd_msg_ex(TSD_MSG_TYPE_ERR, " worng, event name length > %d !!", UTF16_TXT_BUF_MAX_SIZE);
            pTsdInfoRepo->tsd_repo_export(pTsdInfoRepo, (uint8_t*)utf16_txt, 0);
        }
    }

    // description
    pTsdInfoRepo->repoType = TSD_REPO_EPG_EVENT_DESCRIPTION_UTF16;
    if( pEit_node->userInfo.textSize && pEit_node->userInfo.pText )
    {
        tmp_length =tsTxt_ConvToUtf16(0, pEit_node->userInfo.pText, pEit_node->userInfo.textSize, charCode);
        if( tmp_length < UTF16_TXT_BUF_MAX_SIZE )
        {
            tsTxt_ConvToUtf16(utf16_txt, pEit_node->userInfo.pText, pEit_node->userInfo.textSize, charCode);
            utf16_txt[tmp_length] = 0;
            pTsdInfoRepo->tsd_repo_export(pTsdInfoRepo, (uint8_t*)utf16_txt, (1+tmp_length)*sizeof(uint16_t));
            _show_utf16_txt(utf16_txt);
        }
        else
        {
            tsd_msg_ex(TSD_MSG_TYPE_ERR, " worng, description length > %d !!", UTF16_TXT_BUF_MAX_SIZE);
            pTsdInfoRepo->tsd_repo_export(pTsdInfoRepo, (uint8_t*)utf16_txt, 0);
        }
    }

    // rating
    pTsdInfoRepo->repoType = TSD_REPO_EPG_EVENT_RATING;
    rating = (uint8_t)pEit_node->userInfo.rating;
    pTsdInfoRepo->tsd_repo_export(pTsdInfoRepo, (uint8_t*)&rating, sizeof(uint8_t));
    printf("rating: %d\n", rating);

    // end msg
    pTsdInfoRepo->repoType = TSD_REPO_UNKNOW_INFO;
    pTsdInfoRepo->tsd_repo_export(pTsdInfoRepo, (uint8_t*)0, 0);

    printf("\n");
    return 0;
}


static TSD_ERR
_Gen_User_Info(
    ITE_TSD_DEV     *pTsdDev)
{
    TSD_ERR         result = TSD_ERR_OK;
    TS_SRVC_HANDLE  *pHTsSrvc = pTsdDev->pHTsSrvc;
    TS_CHNL_HANDLE  *pHTsChnl = pTsdDev->pHTsChnl;

    _trace_enter("0x%x", pTsdDev);

    do{
        TS_SERVICE_INFO *tmpSrvcInfo = 0;
        TS_SERVICE_INFO *curSrvcInfo = 0;
        TS_CHNL_INFO    *tmpChnlInfo = 0;
        TS_CHNL_INFO    *curChnlInfo = 0;
        uint32_t        totalSrvc = 0;
        uint32_t        i;

        if( pTsdDev->pSrvcUserInfo )    { free(pTsdDev->pSrvcUserInfo); pTsdDev->pSrvcUserInfo = NULL; }
        if( pTsdDev->pChnlUserInfo )    { free(pTsdDev->pChnlUserInfo); pTsdDev->pChnlUserInfo = NULL; }

#if 0 // for demo
        // service user info
        tsSrvc_Control(pHTsSrvc, TS_SRVC_CTL_GET_TOTAL_SRVC, &totalSrvc, 0);
        //pTsdDev->hTsd.totalSrvc = totalSrvc;

        pTsdDev->pSrvcUserInfo = tsd_malloc(totalSrvc*sizeof(TSD_SRVC_USER_INFO*));
        if( !pTsdDev->pSrvcUserInfo )
        {
            tsd_msg_ex(TSD_MSG_TYPE_ERR, " error, allocate fail !!");
            result = TSD_ERR_ALLOCATE_FAIL;
            break;
        }

        {
            uint16_t txt_utf16[256] = {0};
            uint16_t mlb_tw_utf16[6] = {0x6C11, 0x8996, 0x9AD8, 0x756B, 0x8CEA, 0x53F0};

            memset(pTsdDev->pSrvcUserInfo, 0, totalSrvc*sizeof(TSD_SRVC_USER_INFO*));
            curSrvcInfo = pHTsSrvc->pStartSrvcInfo;
            pTsdDev->hTsd.totalSrvc = 0;
            for(i = 0; i < totalSrvc; i++)
            {
                tsSrvc_GetServiceInfo(pHTsSrvc, i, curSrvcInfo, &tmpSrvcInfo, 0);


                tsTxt_ConvToUtf16(txt_utf16,
                                  tmpSrvcInfo->userInfo.serviceName,
                                  tmpSrvcInfo->userInfo.nameSize,
                                  TS_ISO_IEC_6937);
                if( tmpSrvcInfo->userInfo.bTV == false ||
                    tmpSrvcInfo->tsChnlInfo.userInfo.frequency == 569000 ||
                    memcmp(txt_utf16, mlb_tw_utf16, 6*sizeof(uint16_t)) == 0 )
                    continue;

                pTsdDev->pSrvcUserInfo[pTsdDev->hTsd.totalSrvc] = &tmpSrvcInfo->userInfo;
                pTsdDev->hTsd.totalSrvc++;

                curSrvcInfo = tmpSrvcInfo;
            }
        }

#else
        // service user info
        tsSrvc_Control(pHTsSrvc, TS_SRVC_CTL_GET_TOTAL_SRVC, &totalSrvc, 0);
        pTsdDev->hTsd.totalSrvc = totalSrvc;

        pTsdDev->pSrvcUserInfo = tsd_malloc(pTsdDev->hTsd.totalSrvc*sizeof(TSD_SRVC_USER_INFO*));
        if( !pTsdDev->pSrvcUserInfo )
        {
            tsd_msg_ex(TSD_MSG_TYPE_ERR, " error, (%d) allocate fail !!", pTsdDev->hTsd.totalSrvc);
            result = TSD_ERR_ALLOCATE_FAIL;
            break;
        }

        memset(pTsdDev->pSrvcUserInfo, 0, pTsdDev->hTsd.totalSrvc*sizeof(TSD_SRVC_USER_INFO*));
        curSrvcInfo = pHTsSrvc->pStartSrvcInfo;
        for(i = 0; i < pTsdDev->hTsd.totalSrvc; i++)
        {
            tsSrvc_GetServiceInfo(pHTsSrvc, i, curSrvcInfo, &tmpSrvcInfo, 0);
            pTsdDev->pSrvcUserInfo[i] = &tmpSrvcInfo->userInfo;
            curSrvcInfo = tmpSrvcInfo;
        }
#endif

        // channel user info
        pTsdDev->pChnlUserInfo = tsd_malloc(pHTsChnl->totalChnls*sizeof(TSD_CHNL_USER_INFO*));
        if( !pTsdDev->pChnlUserInfo )
        {
            tsd_msg_ex(TSD_MSG_TYPE_ERR, " error, allocate fail !!");
            result = TSD_ERR_ALLOCATE_FAIL;
            break;
        }

        curChnlInfo = pHTsChnl->pStartChnlInfo;
        pTsdDev->hTsd.totalChnl = pHTsChnl->totalChnls;

        memset(pTsdDev->pChnlUserInfo, 0, pTsdDev->hTsd.totalChnl*sizeof(TSD_CHNL_USER_INFO*));
        for(i = 0; i < pTsdDev->hTsd.totalChnl; i++)
        {
            tsChnl_GetChannelInfo(pHTsChnl, i, curChnlInfo, &tmpChnlInfo, 0);
            pTsdDev->pChnlUserInfo[i] = &tmpChnlInfo->userInfo;
            curChnlInfo = tmpChnlInfo;
        }

    }while(0);

    if( result != TSD_ERR_OK )
    {
        //pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        if( pTsdDev->pSrvcUserInfo )
        {
            free(pTsdDev->pSrvcUserInfo);
            pTsdDev->pSrvcUserInfo = 0;
        }
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _trace_leave();
    return result;
}

static TS_COUNTRY_INFO*
_Country_To_Freq(
    TSD_COUNTRY_ID  countryId,
    TS_CHAR_CODE    *charCode)
{
    TS_COUNTRY_CODE     tsCountryId = TS_COUNTRY_UNKNOW;
    TS_CHAR_CODE        char_code = TS_ISO_IEC_6937;

    _trace_enter("%d, 0x%x\n", countryId, charCode);

    switch( countryId )
    {
        case TSD_COUNTRY_AUSTRALIA:    tsCountryId = TS_COUNTRY_AU;                                 break;
        case TSD_COUNTRY_AUSTRIA:      tsCountryId = TS_COUNTRY_AT;                                 break;
        case TSD_COUNTRY_CHINA:        tsCountryId = TS_COUNTRY_CN;                                 break;
        case TSD_COUNTRY_FRANCE:       tsCountryId = TS_COUNTRY_FR;  char_code = TS_ISO_IEC_8859_9; break;
        case TSD_COUNTRY_GERMANY:      tsCountryId = TS_COUNTRY_DE;  char_code = TS_ISO_IEC_8859_9; break;
        case TSD_COUNTRY_GREECE:       tsCountryId = TS_COUNTRY_GR;                                 break;
        case TSD_COUNTRY_HUNGARY:      tsCountryId = TS_COUNTRY_HU;                                 break;
        case TSD_COUNTRY_ITALY:        tsCountryId = TS_COUNTRY_IT;  char_code = TS_ISO_IEC_8859_9; break;
        case TSD_COUNTRY_NETHERLANDS:  tsCountryId = TS_COUNTRY_NL;                                 break;
        case TSD_COUNTRY_POLAND:       tsCountryId = TS_COUNTRY_PL;                                 break;
        case TSD_COUNTRY_PORTUGAL:     tsCountryId = TS_COUNTRY_PT;                                 break;
        case TSD_COUNTRY_RUSSIAN:      tsCountryId = TS_COUNTRY_RU;                                 break;
        case TSD_COUNTRY_SPAIN:        tsCountryId = TS_COUNTRY_ES;                                 break;
        case TSD_COUNTRY_TAIWAN:       tsCountryId = TS_COUNTRY_TW;                                 break;
        case TSD_COUNTRY_UK:           tsCountryId = TS_COUNTRY_UK;                                 break;
        case TSD_COUNTRY_DENMARK:      tsCountryId = TS_COUNTRY_DK;                                 break;
        case TSD_COUNTRY_SWEDEN:       tsCountryId = TS_COUNTRY_SE;                                 break;
        case TSD_COUNTRY_FINLAND:      tsCountryId = TS_COUNTRY_FI;                                 break;
        default:                       break;
    }

    if( charCode )      *charCode = char_code;

    _trace_leave();
    return tsFreq_Get_CountryInfo(tsCountryId);
}

static void*
_Channel_Scan_Proc(
    ITE_TSD_DEV     *pTsdDev)
{
#define MAX_SEARCH_TIME_OUT_CNT     3500
#define PARSING_WAIT_TIME           2000

    TSD_SCAN_PARAM  *pTsdScanParam = &pTsdDev->hTsd.tsdScanParam;
    uint32_t        result = 0;
    bool            bChannelLock = false;
    uint32_t        pat_sdt_TimeOutCnt = 0;
    uint32_t        nit_TimeOutCnt = 0;
    bool            bNotifySdtEvent = false; // modify for new scan
    uint32_t        cbTimer = 0;
    bool            bPatReady = false;
    bool            bPmtReady = false;
    bool            bNitReady = false;
    bool            bSdtReady = false;
    bool            bSkipEpgDefault = false;

    _trace_enter("\n");

    for(;;)
    {
        switch( pTsdDev->scanState )
        {
            case TSD_SCAN_IN_SCANNING:
                // Start Channel Scan. Once lock, start the TS parsing.
                do{
                    uint32_t        st_chnlLock = 0;
                    bool            bFirst_enter = true;
                    TS_CHNL_INFO    currTsChnlInfo = {0};
                    TSP_TUNNEL_INFO tspTunnelInfo = {0};
                    struct timeval startT = {0};

                    // If the channel acquisition step is success.
                    pTsdDev->pHDemodCtrl->frequency = pTsdScanParam->scanFrequency;
                    pTsdDev->pHDemodCtrl->bandwith  = pTsdScanParam->bandwidth;
                    result = DemodCtrl_AcquireChannel(pTsdDev->pHDemodCtrl);
                    if( result )
                    {
                        tsd_msg_ex(TSD_MSG_TYPE_ERR, "acquire channel return error: 0x%x\n", result);
                        break;
                    }

                    tsd_msg(1, "scan freq: %d, bw: %d MHz\n", pTsdScanParam->scanFrequency, pTsdScanParam->bandwidth/1000);
                    tsd_get_clock(&startT);
                    bChannelLock = DemodCtrl_IsChannelLock(pTsdDev->pHDemodCtrl);

                    tsd_msg(1, "\t\tlock channel period: %d ms, bChannelLock = %d\n",
                            tsd_get_duration(&startT), bChannelLock);

                    #if (ENABLE_SW_SIMULATION)
                        bChannelLock = true;
                    #endif

                    if( bChannelLock == false )     break;

                    pat_sdt_TimeOutCnt = nit_TimeOutCnt = 0;

                    // -----------------------------------
                    // release garbage PES in ts_parser
                    // To Do: TS_ReleasePesResource()

                    currTsChnlInfo.userInfo.bandwidth = pTsdScanParam->bandwidth;
                    currTsChnlInfo.userInfo.frequency = pTsdScanParam->scanFrequency;
                    // currTsChnlInfo.original_network_id = INVALID_ONID_NUMBER;
                    // currTsChnlInfo.transport_stream_id = 0;
                    // currTsChnlInfo.sdtVersion = INVALID_VERSION_NUMBER;

                    bNitReady = bSdtReady = bPatReady = bPmtReady = false;
                    bNotifySdtEvent = false; // modify for new scan

                    if( pTsdDev->pHTsp )
                        tsp_DestroyHandle(&pTsdDev->pHTsp);

                    tspTunnelInfo.srvc_tunnel_info.handle = (void*)pTsdDev->pHTsSrvc;
                    tspTunnelInfo.bOnPesOut = false;
                    tsp_CreateHandle(&pTsdDev->pHTsp, false, &tspTunnelInfo, 0);

                    tsSrvc_Control(pTsdDev->pHTsSrvc, TS_SRVC_CTL_RESET_PAT_STATUS, 0, 0);
                    tsSrvc_Control(pTsdDev->pHTsSrvc, TS_SRVC_CTL_RESET_SDT_STATUS, 0, 0);

                    // sync status between ts parser and ts service/channel info
                    tsp_Control(pTsdDev->pHTsp, TSP_CTRL_SET_TSD_CTL_STATUS, (uint32_t*)TSD_CTRL_WAIT_VIDEO_AUDIO, 0);
                    tsp_Control(pTsdDev->pHTsp, TSP_CTRL_SET_CHNL_INFO, 0, &currTsChnlInfo);

                    //-------------------------------------
                    // reset demode PID filter and enable tsi H/W
                    DemodCtrl_ResetPidTable(pTsdDev->pHDemodCtrl, false);

                    #if !(ENABLE_SW_SIMULATION)
                        switch( pTsdDev->tsRecevier.tsrx_type )
                        {
                            case TSD_TSRX_TYPE_TSI:
                                mmpTsiEnable(pTsdDev->tsiId);
                                break;

                            case TSD_TSRX_TYPE_USB:
                                if( pTsdDev->tsRecevier.ts_rx_turn_on )
                                {
                                    pTsdDev->tsRecevier.ts_rx_turn_on(&pTsdDev->tsRecevier, 0);
                                }
                                break;

                            case TSD_TSRX_TYPE_CUSTMOR:
                            default:        break;
                        }
                    #endif

                    //-------------------------------------------
                    // start psi parsing
                    pTsdDev->scanState = TSD_SCAN_CHANNEL_PARSE;
                    bSkipEpgDefault = pTsdDev->bSkipEpgParsing;
                    pTsdDev->bSkipEpgParsing = true;

                    tsd_get_clock(&startT);
                    while( bNitReady == false || bSdtReady == false ||
                           bPatReady == false || bPmtReady == false )
                    {
                        // Time out is ((MAX_SEARCH_TIME_OUT_CNT * PARSING_WAIT_TIME)/1000) ms for PAT and SDT.
                        if( bPatReady == false || bPmtReady == false ||
                            bSdtReady == false )
                        {
                            if( bSdtReady && bNotifySdtEvent == false )
                            {
                                // updata RECEIEVE_SDT_STATE to AP, if need.
                                // To Do:
                                bNotifySdtEvent = true;
                            }

                            // extend the PAT/SDT waiting time to improve the scan channel success rate.
                            if( pat_sdt_TimeOutCnt++ < MAX_SEARCH_TIME_OUT_CNT )
                            {
                                uint32_t    tsi_buf_size = 0;
                                uint8_t     *tsi_buf_addr = 0;

                                // recevie ts data
                                switch( pTsdDev->tsRecevier.tsrx_type )
                                {
                                    case TSD_TSRX_TYPE_TSI:
                                        #if (ENABLE_SW_SIMULATION)
                                        {
                                            extern uint32_t     ts_scan_file_size;
                                            extern FILE         *f_ts_scan;

                                            #define TSI_BUF_SIZE    (200<<10)//204800
                                            static uint32_t     tsi_file_buf_size = 0;
                                            static uint8_t      tsi_file_buf_addr[TSI_BUF_SIZE] = {0};

                                            if( ts_scan_file_size == 0 )    goto channel_parse_end;
                                            tsi_file_buf_size = fread(tsi_file_buf_addr, 1, TSI_BUF_SIZE, f_ts_scan);
                                            ts_scan_file_size -= tsi_buf_size;

                                            tsi_buf_addr = tsi_file_buf_addr;
                                            tsi_buf_size = tsi_file_buf_size;
                                        }
                                        #else
                                        {
                                            uint32_t    tsi_rst = 0;

                                            tsi_rst = mmpTsiReceive(pTsdDev->tsiId, &tsi_buf_addr, &tsi_buf_size);
                                            if( tsi_rst )   tsd_msg_ex(TSD_MSG_TYPE_ERR, " tsi_rst= 0x%x\n", tsi_rst);

                                            if( bFirst_enter == true )
                                            {
                                                // drop garbage
                                                tsi_buf_size = 0;
                                                while( tsi_buf_size == 0 )
                                                {
                                                    //printf(" tsi_0_Status=0x%04x, tsi_1_Status=0x%04x\r", ithReadRegH(0x101e), ithReadRegH(0x109e));
                                                    mmpTsiReceive(pTsdDev->tsiId, &tsi_buf_addr, &tsi_buf_size);
                                                    usleep(2500);
                                                }
                                                bFirst_enter = false;
                                            }
                                            else
                                                usleep(PARSING_WAIT_TIME);  // for contaxt switch to psi parsing
                                        }
                                        #endif
                                        break;

                                    case TSD_TSRX_TYPE_USB:
                                        if( pTsdDev->tsRecevier.ts_rx_get_data )
                                        {
                                            struct timeval usbTimeOutT = {0};
                                            tsd_get_clock(&usbTimeOutT);

                                            do{
                                                pTsdDev->tsRecevier.ts_rx_get_data(&pTsdDev->tsRecevier, &tsi_buf_addr, &tsi_buf_size, 0);
                                                if( !tsi_buf_size )     usleep(PARSING_WAIT_TIME);

                                                if( tsd_get_duration(&usbTimeOutT) > 3000 )
                                                {
                                                    tsd_msg(1, " usb get data time out !!!!!\n");
                                                    break;
                                                }
                                            }while(tsi_buf_size == 0);
                                        }
                                        break;

                                    case TSD_TSRX_TYPE_CUSTMOR:
                                    default:        break;
                                }

                                // need to attach pTspPrsInfo to ts_service handle for callback
                                tsp_Control(pTsdDev->pHTsp, TSP_CTRL_ATTACH_PID_STATISTICS_CB, 0, 0);

                                tsSrvc_Control(pTsdDev->pHTsSrvc, TS_SRVC_CTL_SKIP_PSI_INFO, (uint32_t*)false, 0);

                                pTsdDev->pHTsp->bRingBuf = false;
                                pTsdDev->pHTsp->dataBuf  = tsi_buf_addr;
                                pTsdDev->pHTsp->dataSize = tsi_buf_size;
                                tsp_ParseStream(pTsdDev->pHTsp, 0);

                                // update feedback
                                bPatReady = pTsdDev->pHTsSrvc->bReady_PAT;
                                bPmtReady = pTsdDev->pHTsSrvc->bReady_PMT;
                                bSdtReady = pTsdDev->pHTsSrvc->bReady_SDT;
                                continue;
                            }
                            else
                            {
                                // the frequency is accidentally locked.
                                if( bPatReady == false && bSdtReady == false )
                                {
                                    tsd_msg(1, "\t can't find PAT and SDT !!\n");
                                    goto channel_parse_end;
                                }
                                else
                                {
                                    // skip SDT and modify for new scan
                                    if( false == bSdtReady )
                                    {
                                        // updata NO_SDT_STATE to AP, if need
                                        // To Do:
                                        bNotifySdtEvent = true;
                                    }

                                    if( false == bPmtReady )
                                    {
                                        // updata NO_PMT_STATE or PMT_CNT_NOT_MATCH with PAT to AP, if need
                                        // To Do:
                                    }

                                    bSdtReady = true;   // ask ts_parser skip SDT
                                    bPmtReady = true;   // ask ts_parser skip PMT
                                }
                            }
                        }

                        //----------------------------------
                        // LCN or MFN require NIT
                        // To Do: infoMgr_GetNitUpdate()
                        // Now, we don't implement NIT, so skip it.
                        if( bPatReady == true && bPmtReady == true &&
                            bSdtReady == true )
                            break;
                    }

                    tsd_msg(1, "\ttotal wait time PAT/SDT: %u ms\n", tsd_get_duration(&startT));

                    //----------------------------------
                    // LCN or MFN require NIT
                    // To Do: infoMgr_GetNitUpdate()

                    //----------------------------------
                    // NIT (LCN/MFN) handle
                    //pTsdDev->bNitReady = pTsdDev->bSdtReady = false;

                    // ------------------------------------
                    // merge service list bretween PMT/SDT
                    tsSrvc_Control(pTsdDev->pHTsSrvc, TS_SRVC_CTL_MERGE_SRVC_INFO, (uint32_t*)TS_SRVC_MERGE_DEFAULT, 0);

                    //--------------------------------------
                    // add channel info to database
                    tsSrvc_Control(pTsdDev->pHTsSrvc, TS_SRVC_CTL_GET_SRVC_CNT_IN_CHNL,
                                   &currTsChnlInfo.userInfo.totalSrvcCnt, &currTsChnlInfo);
                    //currTsChnlInfo.bPatReady = pTsdDev->pHTsSrvc->bReady_PAT;
                    //currTsChnlInfo.bNitReady = bNitReady;
                    //currTsChnlInfo.bSdtReady = pTsdDev->pHTsSrvc->bReady_SDT;

                    tsChnl_AddChannel(pTsdDev->pHTsChnl, &currTsChnlInfo, &pTsdDev->hTsd.actChnlIdx, 0);
                    tsd_msg(1, "\tchannel idx: %d, srvCnt = %d\n", pTsdDev->hTsd.actChnlIdx, currTsChnlInfo.userInfo.totalSrvcCnt);

channel_parse_end:
                    tsd_msg(1, "\tsleep done Freq: %d\n", pTsdScanParam->scanFrequency);

                    #if !(ENABLE_SW_SIMULATION)
                        switch( pTsdDev->tsRecevier.tsrx_type )
                        {
                            case TSD_TSRX_TYPE_TSI:
                                mmpTsiDisable(pTsdDev->tsiId);
                                break;

                            case TSD_TSRX_TYPE_USB:
                                if( pTsdDev->tsRecevier.ts_rx_turn_off )
                                {
                                    pTsdDev->tsRecevier.ts_rx_turn_off(&pTsdDev->tsRecevier, 0);
                                }
                                break;

                            case TSD_TSRX_TYPE_CUSTMOR:
                            default:        break;
                        }
                    #endif
                }while(0);

                // demod can't lock channel
                if( false == bChannelLock )     pTsdDev->scanState = TSD_SCAN_ZERO;
                break;

            case TSD_SCAN_CHANNEL_PARSE:
                // do service info sort (1st. by channel order and 2-nd. service info order)
                pTsdDev->scanState = TSD_SCAN_ZERO;
                break;

            case TSD_SCAN_ZERO:
            default:
                goto end;
        }
    }

end:

    // sync status between ts parser and ts service info
    if( pTsdDev->pHTsp )
        tsp_Control(pTsdDev->pHTsp, TSP_CTRL_SET_TSD_CTL_STATUS, (uint32_t*)TSD_CTRL_NORMAL_MODE, 0);

    if( pTsdDev->pHTsp )
        tsp_DestroyHandle(&pTsdDev->pHTsp);

    pTsdDev->scanState = TSD_SCAN_ZERO;

    pTsdDev->hTsd.actChnlIdx = (uint32_t)(-1);
    pTsdDev->hTsd.actSrvcIdx = (uint32_t)(-1);
    pTsdDev->hTsd.actFreq    = 0;

    pTsdDev->bSkipEpgParsing = bSkipEpgDefault;

    // update INVALID_CHANNEL_INDEX to AP
    // infoMgr_SetActiveChannel(INVALID_CHANNEL_INDEX);

    //-------------------------------------------
    // communication with AP layer
    if( pTsdScanParam->pfCallBack )
    {
        TSD_SCAN_RST_INFO   scanInfo = {0};
        uint32_t            totalSrvc = 0;
        TS_SERVICE_INFO     *pCurSrvcInfo = 0;
        TS_SERVICE_INFO     *pTmpSrvcInfo = 0;
        uint32_t            i;

        scanInfo.scanState    = pTsdDev->scanState;
        scanInfo.bChannelLock = bChannelLock;

        if( bChannelLock == true )
        {
            tsSrvc_Control(pTsdDev->pHTsSrvc, TS_SRVC_CTL_GET_TOTAL_SRVC, &totalSrvc, 0);

            pCurSrvcInfo = pTsdDev->pHTsSrvc->pStartSrvcInfo;
            for(i = 0; i < totalSrvc; i++)
            {
                tsSrvc_GetServiceInfo(pTsdDev->pHTsSrvc, i, pCurSrvcInfo, &pTmpSrvcInfo, 0);
                if( pTmpSrvcInfo->tsChnlInfo.userInfo.frequency == pTsdScanParam->scanFrequency )
                {
                    uint32_t    tmp_length = 0;
                    tmp_length = tsTxt_ConvToUtf16(0, pTmpSrvcInfo->userInfo.serviceName,
                                            pTmpSrvcInfo->userInfo.nameSize, pTsdDev->charCode);
                    tsTxt_ConvToUtf16(scanInfo.srvc_name, pTmpSrvcInfo->userInfo.serviceName,
                                      pTmpSrvcInfo->userInfo.nameSize, pTsdDev->charCode);
                    scanInfo.srvc_name[tmp_length] = 0;
                    scanInfo.srvc_name_length = tmp_length;
                    _show_utf16_txt(scanInfo.srvc_name);

                    (*pTsdScanParam->pfCallBack)(&scanInfo, 0);
                    if( scanInfo.bStopScanProc == true )
                        pTsdDev->bStopScanProc = true;
                }

                pCurSrvcInfo = pTmpSrvcInfo;
            }
        }
        else
        {
            (*pTsdScanParam->pfCallBack)(&scanInfo, 0);
            if( scanInfo.bStopScanProc == true )
                pTsdDev->bStopScanProc = true;
        }
    }

    _trace_leave();
    return 0;

}

//=============================================================================
//                Public Function Definition
//=============================================================================
TSD_ERR
tsd_CreateHandle(
    TSD_HANDLE          **pHTsd,
    TSD_PRE_SET_INFO    *pSetInfo,
    void                *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = 0;

    _trace_enter("0x%x, 0x%x, 0x%x\n", pHTsd, pSetInfo, extraData);

    do{
        uint32_t            rst = 0;
        DEMOD_TYPE_ID       demod_type = DEMOD_ID_UNKNOW;
        DEMOD_SETUP_INFO    demdo_setup_info = {0};

        if( *pHTsd != 0 )
        {
            tsd_msg_ex(TSD_MSG_TYPE_ERR, " error, Exist tsd handle !!");
            result = TSD_ERR_INVALID_PARAMETER;
            break;
        }

        if( !pSetInfo )
        {
            tsd_msg_ex(TSD_MSG_TYPE_ERR, " error, Need pre-set info !!");
            result = TSD_ERR_INVALID_PARAMETER;
            break;
        }

        // ------------------------
        // craete dev info
        pTsdDev = tsd_malloc(sizeof(ITE_TSD_DEV));
        if( !pTsdDev )
        {
            tsd_msg_ex(TSD_MSG_TYPE_ERR, " error, allocate fail !!");
            result = TSD_ERR_ALLOCATE_FAIL;
            break;
        }

        memset((void*)pTsdDev, 0x0, sizeof(ITE_TSD_DEV));
        pTsdDev->tsdStatus = TSD_STATUS_IDLE;
        pTsdDev->tsdOutMod = pSetInfo->tsdOutMode;
        pTsdDev->charCode  = TS_ISO_IEC_6937;
        pTsdDev->hTsd.actChnlIdx = (uint32_t)(-1);
        pTsdDev->hTsd.actSrvcIdx = (uint32_t)(-1);

        // -----------------------------
        // create handle
        tsChnl_CreateHandle(&pTsdDev->pHTsChnl, 0);
        tsSrvc_CreateHandle(&pTsdDev->pHTsSrvc, 0);
        tsEpg_CreateHandle(&pTsdDev->pHEpg, 0);

        //--------------------------------
        // initial variable
        pTsdDev->bOnPesOutput = pSetInfo->bOnPesOutput;
        pTsdDev->pesOutBuf_a = pSetInfo->pesOutBuf_a;
        pTsdDev->pesOutBuf_v = pSetInfo->pesOutBuf_v;
        pTsdDev->pesOutBuf_s = pSetInfo->pesOutBuf_s;
        pTsdDev->pesOutBuf_t = pSetInfo->pesOutBuf_t;

        // ----------------------------
        // tsi initial
        #if defined(CFG_BOARD_ITE_9079_EVB)
        pTsdDev->hTsd.act_tsi_id = pTsdDev->tsiId = 1;
        #else
        pTsdDev->hTsd.act_tsi_id = pTsdDev->tsiId = pSetInfo->tsi_id;
        #endif
        pTsdDev->tsRecevier = pSetInfo->tsRecevier;

        #if !(ENABLE_SW_SIMULATION)
            switch( pTsdDev->tsRecevier.tsrx_type )
            {
                case TSD_TSRX_TYPE_TSI:
                    mmpTsiInitialize(pTsdDev->tsiId);
                    break;

                case TSD_TSRX_TYPE_USB:
                    if( pTsdDev->tsRecevier.ts_rx_init )
                    {
                        pTsdDev->tsRecevier.privData.usb_demod_info.demod_index = (-1);
                        pTsdDev->tsRecevier.ts_rx_init(&pTsdDev->tsRecevier, 0);
                    }
                    break;

                case TSD_TSRX_TYPE_CUSTMOR:
                default:        break;
            }

            rst = (IIC_PORT_0, IIC_MASTER_MODE, 0, 0, 100 * 1000);
            if( rst )   tsd_msg_ex(TSD_MSG_TYPE_ERR, "mmpIicInitialize() err 0x%x !", rst);

            mmpIicLockModule(IIC_PORT_0);
            rst = mmpIicSetClockRate(IIC_PORT_0, 200* 1024);
            if( rst )   tsd_msg(1, "curr iic clock %d !\n", rst);
        #endif

        //----------------------------------
        // demod init

        {
            DEMOD_ATTR          demod_attr = {0};
            demod_attr.demod_idx = pSetInfo->demod_id;
            switch( pSetInfo->tsdDemodType )
            {
                case TSD_DEMOD_OMEGA:       demod_attr.demod_type = DEMOD_ID_OMEGA;    break;
                case TSD_DEMOD_IT9135:      demod_attr.demod_type = DEMOD_ID_IT9135;   break;
                case TSD_DEMOD_IT9137:      demod_attr.demod_type = DEMOD_ID_IT9137;   break;
                case TSD_DEMOD_IT9137_USB:  demod_attr.demod_type = DEMOD_ID_IT9137_USB;   break;
                default:               demod_attr.demod_type = DEMOD_ID_UNKNOW;   break;
            }

            demod_attr.bus_type       = g_demod_attr[demod_attr.demod_idx].bus_type;
            demod_attr.demod_i2c_addr = g_demod_attr[demod_attr.demod_idx].demod_i2c_addr;

            pTsdDev->demod_id = demod_attr.demod_idx;
            DemodCtrl_CreateHandle(&pTsdDev->pHDemodCtrl, &demod_attr);
            pTsdDev->hTsd.act_demod_id  = pTsdDev->demod_id;
            pTsdDev->pHDemodCtrl->tsiId = pTsdDev->tsiId;

            demdo_setup_info.supportType  = 2;
            demdo_setup_info.architecture = 2;
            DemodCtrl_Init(pTsdDev->pHDemodCtrl, &demdo_setup_info);
        }

        #if !(ENABLE_SW_SIMULATION)
            rst = mmpIicSetClockRate(IIC_PORT_0, 100* 1024);
            if( rst )   tsd_msg(1, "curr iic clock %d !\n", rst);
            mmpIicReleaseModule(IIC_PORT_0);
        #endif

        // create mutex
        _mutex_init(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

        // if not error
        (*pHTsd) = &pTsdDev->hTsd;

    }while(0);

    if( result != TSD_ERR_OK )
    {
        pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _trace_leave();
    return result;
}

TSD_ERR
tsd_DestroyHandle(
    TSD_HANDLE  **pHTsd)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = DOWN_CAST(ITE_TSD_DEV, (*pHTsd), hTsd);
    pthread_mutex_t tsd_data_mutex = 0;

    _trace_enter("0x%x\n", pHTsd);

    _verify_handle((*pHTsd), result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

    /**
     * avoid broke destroy process in pre-empty multi-task environment
     * (released_handle be used on other function)
     */
    _disable_irq();

    if( pTsdDev )
    {
        // -----------------------------
        // destroy handle
        tsp_DestroyHandle(&pTsdDev->pHTsp);
        tsSrvc_DestroyHandle(&pTsdDev->pHTsSrvc);
        tsChnl_DestroyHandle(&pTsdDev->pHTsChnl);
        tsEpg_DestroyHandle(&pTsdDev->pHEpg);

        // ------------------------------
        // demod terminate
        //DemodCtrl_Terminate();

        // ------------------------------
        // tsi terminate
        #if !(ENABLE_SW_SIMULATION)
            switch( pTsdDev->tsRecevier.tsrx_type )
            {
                case TSD_TSRX_TYPE_TSI:
                    mmpTsiDisable(pTsdDev->tsiId);
                    mmpTsiTerminate(pTsdDev->tsiId);
                    break;

                case TSD_TSRX_TYPE_USB:
                    if( pTsdDev->tsRecevier.ts_rx_turn_off )
                    {
                        pTsdDev->tsRecevier.ts_rx_turn_off(&pTsdDev->tsRecevier, 0);
                    }
                    if( pTsdDev->tsRecevier.ts_rx_deinit )
                    {
                        pTsdDev->tsRecevier.ts_rx_deinit(&pTsdDev->tsRecevier, 0);
                    }
                    break;

                case TSD_TSRX_TYPE_CUSTMOR:
                default:        break;
            }
            mmpIicTerminate(IIC_PORT_0);
        #endif

        // free userInfo array
        if( pTsdDev->pSrvcUserInfo )    { free(pTsdDev->pSrvcUserInfo); pTsdDev->pSrvcUserInfo = NULL; }
        if( pTsdDev->pChnlUserInfo )    { free(pTsdDev->pChnlUserInfo); pTsdDev->pChnlUserInfo = NULL; }

        *pHTsd = 0; // notice AP that handle has be destroyed

        tsd_data_mutex = pTsdDev->tsd_data_mutex;

        // destroy dev info
        free(pTsdDev);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, tsd_data_mutex);
    _mutex_deinit(TSD_MSG_TYPE_TRACE_TSD, tsd_data_mutex);

    _enable_irq();
    _trace_leave();
    return result;
}

TSD_ERR
tsd_Scan_Channel(
    TSD_HANDLE          *pHTsd,
    TSD_FREQ_SCAN_MODE  scanMode,
    TSD_SCAN_PARAM      *scanParm,
    void                *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = DOWN_CAST(ITE_TSD_DEV, pHTsd, hTsd);

    _trace_enter("0x%x, %d, 0x%x, 0x%x\n", pHTsd, scanMode, scanParm, extraData);

    _verify_handle(pHTsd, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

    if( pTsdDev && pTsdDev->tsdStatus != TSD_STATUS_FAIL )
    {
        int                 i;
        uint32_t            freqIdx = 0;
        TS_COUNTRY_INFO     *tsCountryInfo = 0;

        struct timeval startT = {0};

        pTsdDev->scanState = TSD_SCAN_IN_SCANNING;

#if (ENABLE_SW_SIMULATION)
        if( !extraData )
        {
            // clear info database
            tsSrvc_Control(pTsdDev->pHTsSrvc, TS_SRVC_CTL_DEL_ALL_SRVC_INFO, 0, 0);
            tsSrvc_Control(pTsdDev->pHTsSrvc, TS_SRVC_CTL_SKIP_PSI_INFO, (uint32_t*)false, 0);

            tsChnl_Control(pTsdDev->pHTsChnl, TS_CHNL_CTL_DEL_ALL_CHNL_INFO, 0, 0);
        }
#else
        // clear info database
        tsSrvc_Control(pTsdDev->pHTsSrvc, TS_SRVC_CTL_DEL_ALL_SRVC_INFO, 0, 0);
        tsSrvc_Control(pTsdDev->pHTsSrvc, TS_SRVC_CTL_SKIP_PSI_INFO, (uint32_t*)false, 0);

        tsChnl_Control(pTsdDev->pHTsChnl, TS_CHNL_CTL_DEL_ALL_CHNL_INFO, 0, 0);
#endif

        // do scan_freq
        switch( scanMode )
        {
            case TSD_FREQ_SCAN_AUTO:
                tsd_get_clock(&startT);

                pTsdDev->bStopScanProc = false;
                tsCountryInfo = _Country_To_Freq(scanParm->countryId, &pTsdDev->charCode);
                i = 0;
                pTsdDev->hTsd.scan_cnt = 0;
                if( !scanParm )     break;

                memcpy(&pTsdDev->hTsd.tsdScanParam, scanParm, sizeof(TSD_SCAN_PARAM));
                while( pTsdDev->bStopScanProc == false &&
                       tsCountryInfo && tsCountryInfo->freq_band_DB[i] )
                {
                    uint32_t    totalCount = 0;

                    pTsdDev->hTsd.tsdScanParam.bandwidth = tsCountryInfo->freq_band_DB[i]->bandwidth;
                    totalCount = tsCountryInfo->freq_band_DB[i]->totalCount;

                    for(freqIdx = 0; freqIdx < totalCount; freqIdx++)
                    {
                        pTsdDev->scanState = TSD_SCAN_IN_SCANNING;
                        if( tsCountryInfo->freq_band_DB[i]->pSpecificFreq )
                        {
                            pTsdDev->hTsd.tsdScanParam.scanFrequency =
                                tsCountryInfo->freq_band_DB[i]->pSpecificFreq[freqIdx];
                        }
                        else
                        {
                            pTsdDev->hTsd.tsdScanParam.scanFrequency =
                                tsCountryInfo->freq_band_DB[i]->startFreq + (freqIdx * pTsdDev->hTsd.tsdScanParam.bandwidth);
                        }

                        _Channel_Scan_Proc(pTsdDev);
                        pTsdDev->hTsd.scan_cnt++;
                        if( pTsdDev->bStopScanProc == true )
                            break;
                    }
                    i++;
                }
                printf("\n\n\tauto scan duration: %u\n\n", tsd_get_duration(&startT));
                break;

            case TSD_FREQ_SCAN_MANUAL:
                memcpy(&pTsdDev->hTsd.tsdScanParam, scanParm, sizeof(TSD_SCAN_PARAM));
                _Channel_Scan_Proc(pTsdDev);
                break;

            default:
                result = TSD_ERR_NO_IMPLEMENT;
                break;
        }

        // create userInfo array
        result = _Gen_User_Info(pTsdDev);

    }

    if( result != TSD_ERR_OK )
    {
        //pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);
    _trace_leave();
    return result;
}

TSD_ERR
tsd_Get_Sample(
    TSD_HANDLE       *pHTsd,
    TSD_SAMPLE_TYPE  requestType,
    TSD_SAMPLE_INFO  *pSampleInfo,
    void             *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = DOWN_CAST(ITE_TSD_DEV, pHTsd, hTsd);

    _trace_enter("0x%x, 0x%x, 0x%x\n", pHTsd, pSampleInfo, extraData);

    _verify_handle(pHTsd, result);
    _verify_handle(pSampleInfo, result);

    if( pSampleInfo->disableMutex != TSD_DISABLE_MUTEX )
        _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

    if( pTsdDev && pTsdDev->tsdStatus != TSD_STATUS_FAIL )
    {
        uint8_t     *pSampleBuf = 0;
        uint32_t    sampleSize = 0;
        uint32_t    sampleInfoIdx = 0;

        if( !pTsdDev->pHTsp )
        {
            TSP_TUNNEL_INFO tspTunnelInfo = {0};

            tspTunnelInfo.srvc_tunnel_info.handle = (void*)pTsdDev->pHTsSrvc;
            tspTunnelInfo.epg_tunnel_info.handle  = (void*)pTsdDev->pHEpg;

            tspTunnelInfo.bOnPesOut = pTsdDev->bOnPesOutput;
            if( pTsdDev->bOnPesOutput == true )
            {
                tspTunnelInfo.pesOutBuf_a = pTsdDev->pesOutBuf_a;
                tspTunnelInfo.pesOutBuf_v = pTsdDev->pesOutBuf_v;
                tspTunnelInfo.pesOutBuf_s = pTsdDev->pesOutBuf_s;
                tspTunnelInfo.pesOutBuf_t = pTsdDev->pesOutBuf_t;
            }

            tsp_CreateHandle(&pTsdDev->pHTsp, true, &tspTunnelInfo, 0);
            tsp_Control(pTsdDev->pHTsp, TSP_CTRL_ATTACH_EIT_CB, 0, 0);
        }

        // skip parsing PAT/PMT/SDT
        if( pSampleInfo->disableMutex == TSD_DISABLE_MUTEX &&
            (pTsdDev->pHTsSrvc->bReady_PAT == false ||
             pTsdDev->pHTsSrvc->bReady_PMT == false ||
             pTsdDev->pHTsSrvc->bReady_SDT == false) )
            // for gaetway usb demod scan channel case
            tsSrvc_Control(pTsdDev->pHTsSrvc, TS_SRVC_CTL_SKIP_PSI_INFO, (uint32_t*)false, 0);
        else
            tsSrvc_Control(pTsdDev->pHTsSrvc, TS_SRVC_CTL_SKIP_PSI_INFO, (uint32_t*)true, 0);

        switch( pTsdDev->tsRecevier.tsrx_type )
        {
            case TSD_TSRX_TYPE_TSI:
                //------------------------------------
                // get data from tsi buf (PULL mode)
                #if (ENABLE_SW_SIMULATION)
                    {
                        extern uint32_t     ts_scan_file_size;
                        extern FILE         *f_ts_scan;

                        //#define TSI_BUF_SIZE    204800
                        static uint32_t     tsi_buf_size = 0;
                        static uint8_t      tsi_buf_addr[TSI_BUF_SIZE] = {0};
                        static bool         bEnd = false;

                        tsi_buf_size = fread(tsi_buf_addr, 1, TSI_BUF_SIZE, f_ts_scan);
                        if( tsi_buf_size == 0 && bEnd == false )
                        {
                            ts_scan_file_size = ftell(f_ts_scan);
                            fseek(f_ts_scan, 0, SEEK_SET);
                            tsi_buf_size = fread(tsi_buf_addr, 1, TSI_BUF_SIZE, f_ts_scan);
                            //bEnd = true;
                        }
                        ts_scan_file_size -= tsi_buf_size;

                        pSampleBuf = tsi_buf_addr;
                        sampleSize = tsi_buf_size;
                    }
                #else
                    mmpTsiReceive(pTsdDev->tsiId, &pSampleBuf, &sampleSize);
                #endif
                break;

            case TSD_TSRX_TYPE_USB:
                //------------------------------------
                // usb demod
                if( pTsdDev->tsRecevier.ts_rx_get_data &&
                    pSampleInfo->bUsbDefaultAccess == false )
                {
                    pTsdDev->tsRecevier.privData.usb_demod_info.demod_index = (-1);
                    pTsdDev->tsRecevier.ts_rx_get_data(&pTsdDev->tsRecevier, &pSampleBuf, &sampleSize, 0);
                }
                else
                {
                    // usb demod write data to buffer (PUSH mode)
                    void    *pCallback_func = pSampleInfo->pfCallback;

                    pSampleBuf = pSampleInfo->sampleAddr;
                    sampleSize = pSampleInfo->sampleSize;
                    DemodCtrl_ReadDataStream(pTsdDev->pHDemodCtrl, pSampleBuf, sampleSize,
                                             pCallback_func, pSampleInfo->pCtrlParam);
                }
                break;

            case TSD_TSRX_TYPE_CUSTMOR:
            default:        break;
        }

        if( pTsdDev->bSkipEpgParsing == false && pSampleBuf && sampleSize )
        {
            pTsdDev->pHTsp->bRingBuf = false;
            pTsdDev->pHTsp->dataBuf  = pSampleBuf;
            pTsdDev->pHTsp->dataSize = sampleSize;

            tsp_Control(pTsdDev->pHTsp, TSP_CTRL_SET_TSD_CTL_STATUS, (uint32_t*)TSD_CTRL_WAIT_VIDEO_AUDIO, 0);

            if( pTsdDev->bResetEitPsi == true )
            {
                tsp_Control(pTsdDev->pHTsp, TSP_CTRL_REGEN_EIT_HANDLE, 0, 0);
                pTsdDev->bResetEitPsi = false;
            }

            tsp_ParseStream(pTsdDev->pHTsp, 0);
        }

        switch( pTsdDev->tsdOutMod )
        {
            case TSD_OUT_TS_BY_PASS:
                pSampleInfo->type = TSD_SAMPLE_TS;
                if( pSampleInfo->bShareBuf )
                {
                    // just return internal buf pointer
                    pSampleInfo->sampleAddr = pSampleBuf;
                    pSampleInfo->sampleSize = sampleSize;
                }
                else
                {
                    // need to malloc buffer to save
                    if( pSampleInfo->sampleAddr = tsd_malloc(sampleSize) )
                    {
                        memcpy(pSampleInfo->sampleAddr, pSampleBuf, sampleSize);
                        pSampleInfo->sampleSize = sampleSize;
                    }
                    else
                        tsd_msg_ex(TSD_MSG_TYPE_ERR, "err, malloc fail !!");
                }
                break;

            case TSD_OUT_PES_DATA:
                // get data from sample buf
                sampleInfoIdx = 0;
                if( requestType & TSD_SAMPLE_VIDEO )
                {
                    TSP_PES_SAMPLE_INFO     pesSample = {0};

                    pSampleInfo[sampleInfoIdx].type = pesSample.sampleType = TSD_SAMPLE_VIDEO;
                    tsp_GetNextSample(pTsdDev->pHTsp, &pesSample, 0);
                    pSampleInfo[sampleInfoIdx].sampleAddr = pesSample.pSampleAddr;
                    pSampleInfo[sampleInfoIdx].sampleSize = pesSample.sampleSize;
                    sampleInfoIdx++;
                }

                if( requestType & TSD_SAMPLE_AUDIO )
                {
                    TSP_PES_SAMPLE_INFO     pesSample = {0};

                    pSampleInfo[sampleInfoIdx].type = pesSample.sampleType = TSD_SAMPLE_AUDIO;
                    tsp_GetNextSample(pTsdDev->pHTsp, &pesSample, 0);
                    pSampleInfo[sampleInfoIdx].sampleAddr = pesSample.pSampleAddr;
                    pSampleInfo[sampleInfoIdx].sampleSize = pesSample.sampleSize;
                    sampleInfoIdx++;
                }

                if( requestType & TSD_SAMPLE_SUBTITLE )
                {
                    TSP_PES_SAMPLE_INFO     pesSample = {0};

                    pSampleInfo[sampleInfoIdx].type = pesSample.sampleType = TSD_SAMPLE_SUBTITLE;
                    tsp_GetNextSample(pTsdDev->pHTsp, &pesSample, 0);
                    pSampleInfo[sampleInfoIdx].sampleAddr = pesSample.pSampleAddr;
                    pSampleInfo[sampleInfoIdx].sampleSize = pesSample.sampleSize;
                    sampleInfoIdx++;
                }

                if( requestType & TSD_SAMPLE_TELETEXT )
                {
                    TSP_PES_SAMPLE_INFO     pesSample = {0};

                    pSampleInfo[sampleInfoIdx].type = pesSample.sampleType = TSD_SAMPLE_TELETEXT;
                    tsp_GetNextSample(pTsdDev->pHTsp, &pesSample, 0);
                    pSampleInfo[sampleInfoIdx].sampleAddr = pesSample.pSampleAddr;
                    pSampleInfo[sampleInfoIdx].sampleSize = pesSample.sampleSize;
                    sampleInfoIdx++;
                }
                break;
        }
    }

    if( result != TSD_ERR_OK )
    {
        pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    if( pSampleInfo->disableMutex != TSD_DISABLE_MUTEX )
        _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);
    _trace_leave();
    return result;
}

TSD_ERR
tsd_Merge_Service_Info(
    TSD_HANDLE          *pHTsd_master,
    TSD_HANDLE          *pHTsd_slave,
    void                *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev_master = DOWN_CAST(ITE_TSD_DEV, pHTsd_master, hTsd);
    ITE_TSD_DEV     *pTsdDev_slave = DOWN_CAST(ITE_TSD_DEV, pHTsd_slave, hTsd);

    _trace_enter("0x%x, 0x%x, 0x%x\n", pHTsd_master, pHTsd_slave, extraData);

    _verify_handle(pHTsd_master, result);
    _verify_handle(pHTsd_slave, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev_master->tsd_data_mutex);
    _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev_slave->tsd_data_mutex);

    if( pTsdDev_master && pTsdDev_master->tsdStatus != TSD_STATUS_FAIL ||
        pTsdDev_slave && pTsdDev_slave->tsdStatus != TSD_STATUS_FAIL )
    {
        tsChnl_Control(pTsdDev_master->pHTsChnl,
                       TS_CHNL_CTL_MERGE_CHNL_INFO,
                       (uint32_t*)TS_CHNL_MERGE_2_HANDLE,
                       pTsdDev_slave->pHTsChnl);

        tsSrvc_Control(pTsdDev_master->pHTsSrvc,
                       TS_SRVC_CTL_MERGE_SRVC_INFO,
                       (uint32_t*)TS_SRVC_MERGE_2_HANDLE,
                       pTsdDev_slave->pHTsSrvc);

        // gen use info
        _Gen_User_Info(pTsdDev_master);
    }

    if( result != TSD_ERR_OK )
    {
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev_master->tsd_data_mutex);
    _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev_slave->tsd_data_mutex);
    _trace_leave();
    return result;
}


TSD_ERR
tsd_Get_ChannelInfo(
    TSD_HANDLE          *pHTsd,
    uint32_t            index,
    TSD_CHNL_USER_INFO  **pChnlUserInfo,
    void                *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = DOWN_CAST(ITE_TSD_DEV, pHTsd, hTsd);

    _trace_enter("0x%x, %d, 0x%x, 0x%x\n", pHTsd, index, pChnlUserInfo, extraData);

    _verify_handle(pHTsd, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

    if( pTsdDev && pTsdDev->tsdStatus != TSD_STATUS_FAIL )
    {
        do{
            if( index >= pTsdDev->hTsd.totalChnl )
            {
                tsd_msg_ex(TSD_MSG_TYPE_ERR, " channel index (%d) out range, max=%d !", index, pTsdDev->hTsd.totalChnl - 1);
                break;
            }

            if( pChnlUserInfo && pTsdDev->pChnlUserInfo )
            {
                (*pChnlUserInfo) = pTsdDev->pChnlUserInfo[index];
            }
        }while(0);

        // need to handle (pTsdDev->pChnlUserInfo == NULL) ????
    }

    if( result != TSD_ERR_OK )
    {
        pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);
    _trace_leave();
    return result;
}

TSD_ERR
tsd_Get_ServiceInfo(
    TSD_HANDLE          *pHTsd,
    uint32_t            index,
    TSD_SRVC_USER_INFO  **pSrvcUserInfo,
    void                *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = DOWN_CAST(ITE_TSD_DEV, pHTsd, hTsd);

    _trace_enter("0x%x, %d, 0x%x, 0x%x\n", pHTsd, index, pSrvcUserInfo, extraData);

    _verify_handle(pHTsd, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

    if( pTsdDev && pTsdDev->tsdStatus != TSD_STATUS_FAIL )
    {
        do{
            if( index >= pTsdDev->hTsd.totalSrvc )
            {
                tsd_msg_ex(TSD_MSG_TYPE_ERR, " Service index (%d) out range, max=%d !", index, pTsdDev->hTsd.totalSrvc - 1);
                break;
            }

            if( pSrvcUserInfo && pTsdDev->pSrvcUserInfo )
            {
                (*pSrvcUserInfo) = pTsdDev->pSrvcUserInfo[index];
            }
        }while(0);

        // need to handle (pTsdDev->pSrvcUserInfo == NULL) ????
    }

    if( result != TSD_ERR_OK )
    {
        pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);
    _trace_leave();
    return result;
}

TSD_ERR
tsd_Get_CountryName(
    TSD_HANDLE          *pHTsd,
    TSD_COUNTRY_ID      countryId,
    char                **name,
    void                *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = DOWN_CAST(ITE_TSD_DEV, pHTsd, hTsd);

    //_trace_enter("0x%x, %d, 0x%x, 0x%x\n", pHTsd, countryId, name, extraData);

    _verify_handle(pHTsd, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

    if( pTsdDev && name && pTsdDev->tsdStatus != TSD_STATUS_FAIL )
    {
        TS_COUNTRY_INFO     *pTsCountryInfo = 0;
        TS_CHAR_CODE        charCode;

        *name = 0;

        pTsCountryInfo = _Country_To_Freq(countryId, &charCode);
        if(  pTsCountryInfo )
            *name = pTsCountryInfo->name;
    }

    if( result != TSD_ERR_OK )
    {
        pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);
    //_trace_leave();
    return result;
}

TSD_ERR
tsd_Get_Service_Schedule(
    TSD_HANDLE          *pHTsd,
    uint32_t            serviceIdx,
    TSD_INFO_REPO       *pTsdInfoRepo,
    void                *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = DOWN_CAST(ITE_TSD_DEV, pHTsd, hTsd);

    _trace_enter("0x%x, %d, 0x%x, 0x%x\n", pHTsd, serviceIdx, pTsdInfoRepo, extraData);

    _verify_handle(pHTsd, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

    if( pTsdDev && pTsdDev->tsdStatus != TSD_STATUS_FAIL )
    {
        if( pTsdInfoRepo && pTsdDev->pHEpg )
        {
            do{
                TSD_SRVC_USER_INFO  *pTmpSrvcUserInfo = 0;
                TS_SERVICE_INFO     *pSrvcInfo = 0;
                TS_1_SRVC_EIT       *pOne_srvc_eits = 0;

                if( serviceIdx >= pTsdDev->hTsd.totalSrvc )
                {
                    tsd_msg_ex(TSD_MSG_TYPE_ERR, " Service index (%d) out range, max=%d !", serviceIdx, pTsdDev->hTsd.totalSrvc - 1);
                    break;
                }

                pTmpSrvcUserInfo = pTsdDev->pSrvcUserInfo[serviceIdx];
                pSrvcInfo = DOWN_CAST(TS_SERVICE_INFO, pTmpSrvcUserInfo, userInfo);

                // get epg data
                tsEpg_Get_Service_Schedule(pTsdDev->pHEpg,
                                           pTsdDev->hTsd.actFreq,
                                           pSrvcInfo->programNumber,
                                           &pOne_srvc_eits, 0);

                //if( pTsdInfoRepo )
                {
                    int32_t             repoRst = 0;
                    EPG_1_DAY_SCHEDULE  *pEpg_1_day = 0;
                    TSD_INFO_REPO       tsdInfoRepo = {0};

                    memcpy(&tsdInfoRepo, pTsdInfoRepo, sizeof(TSD_INFO_REPO));

                    tsdInfoRepo.repoType = TSD_REPO_EPG_SCHEDULE_INFO;
                    // tsd_repo_open
                    if( tsdInfoRepo.tsd_repo_open )
                        repoRst = tsdInfoRepo.tsd_repo_open(&tsdInfoRepo, 0);

                    // tsd_repo_export
                    if( pOne_srvc_eits && tsdInfoRepo.tsd_repo_export )
                    {
                        int                 i;
                        MBT_VISIT_INFO      visitInfo = {0};

                        for(i = 0; i < EPG_MAX_SCHEDULE_DAY; i++)
                        {
                            if( !pOne_srvc_eits->epg_1_day[i].pEitMbtRoot )
                                continue;

                            // in-order
                            visitInfo.visitType = MBT_VISIT_INORDER;
                            visitInfo.act_func  = _Epg_Schedule_Export;
                            visitInfo.privData  = (void*)&tsdInfoRepo;
                            visitInfo.privData_1 = (void*)pTsdDev->charCode;
                            mbt_visit_node(EIT_MBT_NODE, mbt_node,
                                    pOne_srvc_eits->epg_1_day[i].pEitMbtRoot, &visitInfo);
                        }
                    }

                    // tsd_repo_close
                    if( tsdInfoRepo.tsd_repo_close )
                        repoRst = tsdInfoRepo.tsd_repo_close(&tsdInfoRepo, 0);
                }
            }while(0);
        }

        // need to handle (pTsdDev->pSrvcUserInfo == NULL) ????
    }

    if( result != TSD_ERR_OK )
    {
        pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);
    _trace_leave();
    return result;
}

TSD_ERR
tsd_Get_Service_PF(
    TSD_HANDLE          *pHTsd,
    uint32_t            serviceIdx,
    TSD_INFO_REPO       *pTsdInfoRepo,
    void                *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = DOWN_CAST(ITE_TSD_DEV, pHTsd, hTsd);

    _trace_enter("0x%x, %d, 0x%x, 0x%x\n", pHTsd, serviceIdx, pTsdInfoRepo, extraData);

    _verify_handle(pHTsd, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

    if( pTsdDev && pTsdDev->tsdStatus != TSD_STATUS_FAIL )
    {
        if( pTsdInfoRepo && pTsdDev->pHEpg )
        {
            do{
                TSD_SRVC_USER_INFO  *pTmpSrvcUserInfo = 0;
                TS_SERVICE_INFO     *pSrvcInfo = 0;
                TS_1_SRVC_EIT       *pOne_srvc_eits = 0;
                int32_t             repoRst = 0;
                TSD_INFO_REPO       tsdInfoRepo = {0};
                MBT_VISIT_INFO      visitInfo = {0};
                PSI_MJDBCD_TIME     presentEndTime = {0};
                int                 j;

                if( serviceIdx >= pTsdDev->hTsd.totalSrvc )
                {
                    tsd_msg_ex(TSD_MSG_TYPE_ERR, " Service index (%d) out range, max=%d !", serviceIdx, pTsdDev->hTsd.totalSrvc - 1);
                    break;
                }

                pTmpSrvcUserInfo = pTsdDev->pSrvcUserInfo[serviceIdx];
                pSrvcInfo = DOWN_CAST(TS_SERVICE_INFO, pTmpSrvcUserInfo, userInfo);

                // get epg data
                tsEpg_Get_Service_Schedule(pTsdDev->pHEpg,
                                           pTsdDev->hTsd.actFreq,
                                           pSrvcInfo->programNumber,
                                           &pOne_srvc_eits, 0);

                memcpy(&tsdInfoRepo, pTsdInfoRepo, sizeof(TSD_INFO_REPO));

                // tsd_repo_open
                if( tsdInfoRepo.tsd_repo_open )
                    repoRst = tsdInfoRepo.tsd_repo_open(&tsdInfoRepo, 0);

                // get current time from TDT/TOT
                //pTsdDev->pHEpg->curUtcTime;

                // tsd_repo_export present/following event
                if( pOne_srvc_eits && tsdInfoRepo.tsd_repo_export )
                {
                    int           i;
                    EIT_MBT_NODE  *pPF_eit_node = 0;
                    EIT_MBT_NODE  pattern_eit_node = {0};

                    for(j = 0; j < 2; j++)
                    {
                        tsdInfoRepo.repoType = (j) ? TSD_REPO_EPG_FOLLOWING_INFO : TSD_REPO_EPG_PRESENT_INFO;
                        tsdInfoRepo.tsd_repo_export(&tsdInfoRepo, 0, 0);

                        // set pattern info with current time and find it in epg_database
                        switch( tsdInfoRepo.repoType )
                        {
                            case TSD_REPO_EPG_PRESENT_INFO:
                                pattern_eit_node.start_time = pTsdDev->pHEpg->curUtcTime;
                                break;

                            case TSD_REPO_EPG_FOLLOWING_INFO:
                                pattern_eit_node.start_time = presentEndTime;
                                pattern_eit_node.start_time.low24++;
                                break;
                        }

                        for(i = 0; i < EPG_MAX_SCHEDULE_DAY; i++)
                        {
                            if( !pOne_srvc_eits->epg_1_day[i].pEitMbtRoot )
                                continue;

                            mbt_find_node(EIT_MBT_NODE, mbt_node,
                                    pOne_srvc_eits->epg_1_day[i].pEitMbtRoot,
                                    &pattern_eit_node, &pPF_eit_node, _EPG_PF_Compare, 0);

                            // get present or following eit_node
                            if( pPF_eit_node )
                            {
                                visitInfo.privData  = (void*)&tsdInfoRepo;
                                visitInfo.privData_1 = (void*)pTsdDev->charCode;

                                _Epg_Schedule_Export(&visitInfo, &pPF_eit_node->mbt_node);
                                presentEndTime = psiTime_Add(pPF_eit_node->start_time, pPF_eit_node->duration);
                                break;
                            }
                            else
                            {
                                tsd_msg_ex(TSD_MSG_TYPE_ERR, "Can't find present event\n");
                                j = 2; // no present event, no following event
                            }
                        }
                    }
                }

                // tsd_repo_close
                if( tsdInfoRepo.tsd_repo_close )
                    repoRst = tsdInfoRepo.tsd_repo_close(&tsdInfoRepo, 0);
            }while(0);
        }

        // need to handle (pTsdDev->pSrvcUserInfo == NULL) ????
    }

    if( result != TSD_ERR_OK )
    {
        pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);
    _trace_leave();
    return result;
}


TSD_ERR
tsd_Change_Channel(
    TSD_HANDLE      *pHTsd,
    uint32_t        index,
    void            *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = DOWN_CAST(ITE_TSD_DEV, pHTsd, hTsd);

    _trace_enter("0x%x, %d, 0x%x\n", pHTsd, index, extraData);

    if(0 && pTsdDev && pTsdDev->tsdStatus != TSD_STATUS_FAIL )
    {
        do{
            TSD_CHNL_USER_INFO  *tmpChnlUserInfo = 0;

            if( index >= pTsdDev->hTsd.totalChnl )
            {
                tsd_msg_ex(TSD_MSG_TYPE_ERR, " channel index (%d) out range, max=%d !", index, pTsdDev->hTsd.totalChnl - 1);
                break;
            }

            tmpChnlUserInfo = pTsdDev->pChnlUserInfo[index];

            if( pTsdDev->hTsd.totalChnl > index &&
                tmpChnlUserInfo->frequency != pTsdDev->hTsd.actFreq )
            {
                uint32_t        result = 0;

                DemodCtrl_ResetPidTable(pTsdDev->pHDemodCtrl, true);

                //// update PID filter, video/audio/subtitle
                //DemodCtrl_UpdatePidTable(pTsdDev->demod_id, pSrvcInfo->videoPID, PID_VIDEO_INDEX);
                //pSrvcInfo->userInfo.actAudioIdx = 0;
                //DemodCtrl_UpdatePidTable(pTsdDev->demod_id, pSrvcInfo->audioPID[pSrvcInfo->userInfo.actAudioIdx], PID_AUDIO_INDEX);
                //pSrvcInfo->userInfo.actSubtitleIdx = (uint16_t)-1;

                // Need to keep PMT PID
                // To Do:

                // re-Acquire Channel
                pTsdDev->pHDemodCtrl->frequency = tmpChnlUserInfo->frequency;
                pTsdDev->pHDemodCtrl->bandwith  = tmpChnlUserInfo->bandwidth;
                result = DemodCtrl_AcquireChannel(pTsdDev->pHDemodCtrl);
                if( result )
                {
                    tsd_msg_ex(TSD_MSG_TYPE_ERR, "acquire channel return err: 0x%x (demodId=%d, freq=%d, bw=%d)\n",
                                                result, pTsdDev->demod_id, tmpChnlUserInfo->frequency, tmpChnlUserInfo->bandwidth);
                    result = TSD_ERR_DEMOD_FAIL;
                }
                else
                {
                    pTsdDev->hTsd.actChnlIdx   = index;
                    pTsdDev->hTsd.actFreq      = tmpChnlUserInfo->frequency;
                    pTsdDev->hTsd.actBandwidth = tmpChnlUserInfo->bandwidth;
                }
            }
        }while(0);
    }

    if( result != TSD_ERR_OK )
    {
        pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _trace_leave();
    return result;
}

TSD_ERR
tsd_Change_Service(
    TSD_HANDLE      *pHTsd,
    uint32_t        index,
    void            *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = DOWN_CAST(ITE_TSD_DEV, pHTsd, hTsd);

    _trace_enter("0x%x, %d, 0x%x\n", pHTsd, index, extraData);

    _verify_handle(pHTsd, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

    if( pTsdDev && pTsdDev->tsdStatus != TSD_STATUS_FAIL )
    {
        TSD_SRVC_USER_INFO  *tmpSrvcUserInfo = 0;
        TS_SERVICE_INFO     *pCurSrvcInfo = 0;
        TS_SERVICE_INFO     *pNextSrvcInfo = 0;

        do{
            bool        bChannelLock = false;
            uint32_t    tsi_buf_size = 0;
            uint8_t     *tsi_buf_addr = 0;

            if( index >= pTsdDev->hTsd.totalSrvc )
            {
                tsd_msg_ex(TSD_MSG_TYPE_ERR, " service index (%d) out range, max=%d !", index, pTsdDev->hTsd.totalSrvc - 1);
                break;
            }

            if( pTsdDev->hTsd.actSrvcIdx != (uint32_t)(-1) &&
                pTsdDev->hTsd.actSrvcIdx >= pTsdDev->hTsd.totalSrvc )
                tsd_msg_ex(TSD_MSG_TYPE_ERR, " Wrong Action service idx %d, max=%d !", pTsdDev->hTsd.actSrvcIdx, pTsdDev->hTsd.totalSrvc - 1);

            #if !(ENABLE_SW_SIMULATION)
                switch( pTsdDev->tsRecevier.tsrx_type )
                {
                    case TSD_TSRX_TYPE_TSI:
                        mmpTsiDisable(pTsdDev->tsiId);
                        // drop garbage
                        // mmpTsiReceive(pTsdDev->tsiId, &tsi_buf_addr, &tsi_buf_size);
                        break;

                    case TSD_TSRX_TYPE_USB:
                        if( pTsdDev->tsRecevier.ts_rx_turn_off )
                        {
                            pTsdDev->tsRecevier.ts_rx_turn_off(&pTsdDev->tsRecevier, 0);
                        }
                        break;

                    case TSD_TSRX_TYPE_CUSTMOR:
                    default:        break;
                }
            #endif

            tmpSrvcUserInfo = pTsdDev->pSrvcUserInfo[index];
            pNextSrvcInfo = DOWN_CAST(TS_SERVICE_INFO, tmpSrvcUserInfo, userInfo);

            // change channel or not
            tsd_msg(1, "useFreq: %d, action: %d, in %s()[#%d]\n",
                pNextSrvcInfo->tsChnlInfo.userInfo.frequency,
                pTsdDev->hTsd.actFreq, __FUNCTION__, __LINE__);
           _show_dvb_txt(pNextSrvcInfo->userInfo.serviceName, pNextSrvcInfo->userInfo.nameSize);

            pTsdDev->pesOutBuf_a.pid = pTsdDev->pesOutBuf_v.pid
                                     = pTsdDev->pesOutBuf_s.pid
                                     = 0x0;

            if( pNextSrvcInfo->tsChnlInfo.userInfo.frequency != pTsdDev->hTsd.actFreq )
            {
                uint32_t        result = 0;

                // re-Acquire Channel
                pTsdDev->pHDemodCtrl->frequency = pNextSrvcInfo->tsChnlInfo.userInfo.frequency;
                pTsdDev->pHDemodCtrl->bandwith  = pNextSrvcInfo->tsChnlInfo.userInfo.bandwidth;
                result = DemodCtrl_AcquireChannel(pTsdDev->pHDemodCtrl);
                if( result )
                {
                    tsd_msg_ex(TSD_MSG_TYPE_ERR, "acquire channel return err: 0x%x (demodId=%d, freq=%d, bw=%d)\n",
                                                result, pTsdDev->demod_id,
                                                pNextSrvcInfo->tsChnlInfo.userInfo.frequency,
                                                pNextSrvcInfo->tsChnlInfo.userInfo.bandwidth);
                    result = TSD_ERR_DEMOD_FAIL;
                    break;
                }

                bChannelLock = DemodCtrl_IsChannelLock(pTsdDev->pHDemodCtrl);

                DemodCtrl_ResetPidTable(pTsdDev->pHDemodCtrl, true);

                // update PID filter, video/audio/subtitle
                tsd_msg(1, "  videoPID= 0x%x, bChannelLock= %d \n", pNextSrvcInfo->videoPID, bChannelLock);
                DemodCtrl_UpdatePidTable(pTsdDev->pHDemodCtrl, pNextSrvcInfo->videoPID, PID_VIDEO_INDEX);
                pTsdDev->pesOutBuf_v.pid = pNextSrvcInfo->videoPID;

                pNextSrvcInfo->userInfo.actAudioIdx = 0;
                if( pTsdDev->bSoundFullOut == true )
                {
                    // full sound track output
                    int         j;

                    for(j = 0; j < pNextSrvcInfo->userInfo.audioCount; j++)
                    {
                        if( (PID_AUDIO_INDEX - j) < PID_AUDIO_RESERVE_0 )
                        {
                            tsd_msg_ex(TSD_MSG_TYPE_ERR, " total sound tracks(%d) out support counte(%d) !",
                                            pNextSrvcInfo->userInfo.audioCount, (PID_AUDIO_INDEX - PID_AUDIO_RESERVE_0 + 1));
                            break;
                        }
                        DemodCtrl_UpdatePidTable(pTsdDev->pHDemodCtrl, pNextSrvcInfo->audioPID[j], PID_AUDIO_INDEX - j);
                    }
                }
                else
                    DemodCtrl_UpdatePidTable(pTsdDev->pHDemodCtrl, pNextSrvcInfo->audioPID[pNextSrvcInfo->userInfo.actAudioIdx], PID_AUDIO_INDEX);

                pTsdDev->pesOutBuf_a.pid = pNextSrvcInfo->audioPID[pNextSrvcInfo->userInfo.actAudioIdx];

                pNextSrvcInfo->userInfo.actSubtitleIdx = (uint16_t)-1;

                pTsdDev->hTsd.actFreq      = pNextSrvcInfo->tsChnlInfo.userInfo.frequency;
                pTsdDev->hTsd.actBandwidth = pNextSrvcInfo->tsChnlInfo.userInfo.bandwidth;

                if( pTsdDev->pHTsp )    tsp_DestroyHandle(&pTsdDev->pHTsp);
            }
            else
            {
                if( (int)pTsdDev->hTsd.actSrvcIdx > 0 )
                {
                    uint16_t    actAudioIdx = 0;

                    tmpSrvcUserInfo = pTsdDev->pSrvcUserInfo[pTsdDev->hTsd.actSrvcIdx];
                    pCurSrvcInfo = DOWN_CAST(TS_SERVICE_INFO, tmpSrvcUserInfo, userInfo);
                    // Turn off old video PID.
                    if( pCurSrvcInfo->videoPID )
                        tsp_Control(pTsdDev->pHTsp, TSP_CTRL_DISABLE_PID, (uint32_t*)pCurSrvcInfo->videoPID, 0);

                    // Turn off old audio PID.
                    actAudioIdx = (pCurSrvcInfo->userInfo.actAudioIdx < SERVICE_MAX_AUDIO_COUNT) ?
                                                        pCurSrvcInfo->userInfo.actAudioIdx: 0;
                    if( pCurSrvcInfo->audioPID[pCurSrvcInfo->userInfo.actAudioIdx] )
                        tsp_Control(pTsdDev->pHTsp, TSP_CTRL_DISABLE_PID, (uint32_t*)pCurSrvcInfo->audioPID[pCurSrvcInfo->userInfo.actAudioIdx], 0);
                }

                // update PID filter, video/audio/subtitle
                DemodCtrl_UpdatePidTable(pTsdDev->pHDemodCtrl, pNextSrvcInfo->videoPID, PID_VIDEO_INDEX);
                pTsdDev->pesOutBuf_v.pid = pNextSrvcInfo->videoPID;

                pNextSrvcInfo->userInfo.actAudioIdx = 0;
                if( pTsdDev->bSoundFullOut == true )
                {
                    // full sound track output
                    int         j;

                    for(j = 0; j < pNextSrvcInfo->userInfo.audioCount; j++)
                    {
                        if( (PID_AUDIO_INDEX - j) < PID_AUDIO_RESERVE_0 )
                        {
                            tsd_msg_ex(TSD_MSG_TYPE_ERR, " total sound tracks(%d) out support counte(%d) !",
                                            pNextSrvcInfo->userInfo.audioCount, (PID_AUDIO_INDEX - PID_AUDIO_RESERVE_0 + 1));
                            break;
                        }
                        DemodCtrl_UpdatePidTable(pTsdDev->pHDemodCtrl, pNextSrvcInfo->audioPID[j], PID_AUDIO_INDEX - j);
                    }
                }
                else
                    DemodCtrl_UpdatePidTable(pTsdDev->pHDemodCtrl, pNextSrvcInfo->audioPID[pNextSrvcInfo->userInfo.actAudioIdx], PID_AUDIO_INDEX);

                pTsdDev->pesOutBuf_a.pid = pNextSrvcInfo->audioPID[pNextSrvcInfo->userInfo.actAudioIdx];

                pNextSrvcInfo->userInfo.actSubtitleIdx = (uint16_t)-1;
                //DemodCtrl_UpdatePidTable(pTsdDev->pHDemodCtrl, 0x0, PID_SUBTITLE_INDEX);

                // Turn on new video PID.
                if( pNextSrvcInfo->videoPID )
                    tsp_Control(pTsdDev->pHTsp, TSP_CTRL_ENABLE_PID, (uint32_t*)pNextSrvcInfo->videoPID, 0);

                // Turn on new audio PID.
                if( pNextSrvcInfo->audioPID[pNextSrvcInfo->userInfo.actAudioIdx] )
                    tsp_Control(pTsdDev->pHTsp, TSP_CTRL_ENABLE_PID, (uint32_t*)pNextSrvcInfo->audioPID[pNextSrvcInfo->userInfo.actAudioIdx], 0);
            }

            // Need to keep PMT PID
            DemodCtrl_UpdatePidTable(pTsdDev->pHDemodCtrl, pNextSrvcInfo->pmt_pid, PID_PMT_INDEX);

            //----------------
            // EPG
            tsEpg_Change_Service(pTsdDev->pHEpg, pTsdDev->hTsd.actFreq, pNextSrvcInfo->programNumber, 0);
            if( pTsdDev->hTsd.actSrvcIdx != index )     pTsdDev->bResetEitPsi = true;

            pTsdDev->hTsd.actSrvcIdx = index;

            #if !(ENABLE_SW_SIMULATION)
                switch( pTsdDev->tsRecevier.tsrx_type )
                {
                    case TSD_TSRX_TYPE_TSI:
                        mmpTsiEnable(pTsdDev->tsiId);
                        break;

                    case TSD_TSRX_TYPE_USB:
                        if( pTsdDev->tsRecevier.ts_rx_turn_on )
                        {
                            pTsdDev->tsRecevier.ts_rx_turn_on(&pTsdDev->tsRecevier, 0);
                        }
                        break;

                    case TSD_TSRX_TYPE_CUSTMOR:
                    default:        break;
                }
            #endif
        }while(0);
    }

    if( result != TSD_ERR_OK )
    {
        pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);
    _trace_leave();
    return result;
}

TSD_ERR
tsd_Conv_Text(
    TSD_HANDLE          *pHTsd,
    TSD_TXT_CONV        *txtConv,
    void                *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = DOWN_CAST(ITE_TSD_DEV, pHTsd, hTsd);

    _trace_enter("0x%x, 0x%x, 0x%x\n", pHTsd, txtConv, extraData);

    _verify_handle(pHTsd, result);
    //_mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

    // if EIT not ready (just get partial sections), How to do ?????
    // To Do:

    if( pTsdDev && txtConv && pTsdDev->tsdStatus != TSD_STATUS_FAIL )
    {
        uint32_t    outLength = 0;

        outLength = tsTxt_ConvToUtf16(0, txtConv->dvbTxt, txtConv->dvbTxtLength, pTsdDev->charCode);

        if( (txtConv->utf16TxtLength - 1) > outLength )
        {
            tsTxt_ConvToUtf16(txtConv->utf16Txt, txtConv->dvbTxt, txtConv->dvbTxtLength, pTsdDev->charCode);
            txtConv->utf16Txt[outLength] = 0;

            txtConv->utf16TxtLength = outLength; // update real output length
        }
        else
            txtConv->utf16Txt[0] = 0;
    }

    if( result != TSD_ERR_OK )
    {
        pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    //_mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);
    _trace_leave();
    return result;
}

TSD_ERR
tsd_Change_Subtitle(
    TSD_HANDLE      *pHTsd,
    uint32_t        index,
    void            *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = DOWN_CAST(ITE_TSD_DEV, pHTsd, hTsd);

    _trace_enter("0x%x, %d, 0x%x\n", pHTsd, index, extraData);

    _verify_handle(pHTsd, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

    if( pTsdDev && pTsdDev->tsdStatus != TSD_STATUS_FAIL )
    {
        do{
            TSD_SRVC_USER_INFO  *pCurSrvcUserInfo = 0;
            TS_SERVICE_INFO     *pSrvcInfo = 0;

            //if( index >= pTsdDev->hTsd.totalSubtitle )
            //{
            //    tsd_msg_ex(TSD_MSG_TYPE_ERR, " subtitle index (%d) out range, max=%d !", index, pTsdDev->hTsd.totalSubtitle - 1);
            //    break;
            //}

            pCurSrvcUserInfo = pTsdDev->pSrvcUserInfo[pTsdDev->hTsd.actSrvcIdx];
            pSrvcInfo = DOWN_CAST(TS_SERVICE_INFO, pCurSrvcUserInfo, userInfo);

            if( index < pCurSrvcUserInfo->subtitleCount )
            {
                pSrvcInfo->userInfo.actSubtitleIdx = index;
                DemodCtrl_UpdatePidTable(pTsdDev->pHDemodCtrl, pSrvcInfo->subtitlePID[pSrvcInfo->userInfo.actSubtitleIdx], PID_SUBTITLE_INDEX);
                pTsdDev->pesOutBuf_s.pid = pSrvcInfo->subtitlePID[pSrvcInfo->userInfo.actSubtitleIdx];
            }
            else
            {
                // unknow subtitle index => disable subtitle
                pSrvcInfo->userInfo.actSubtitleIdx = (uint16_t)-1;
                DemodCtrl_UpdatePidTable(pTsdDev->pHDemodCtrl, 0x0, PID_SUBTITLE_INDEX);
                pTsdDev->pesOutBuf_s.pid = 0;
            }
        }while(0);
    }

    if( result != TSD_ERR_OK )
    {
        pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);
    _trace_leave();
    return result;
}

TSD_ERR
tsd_Change_Teletext(
    TSD_HANDLE      *pHTsd,
    void            *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = DOWN_CAST(ITE_TSD_DEV, pHTsd, hTsd);

    _trace_enter("0x%x, 0x%x\n", pHTsd, extraData);

    _verify_handle(pHTsd, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

    if( pTsdDev && pTsdDev->tsdStatus != TSD_STATUS_FAIL )
    {
        // get the ringht PID (Teletext) from database
        // and update Pid table for stream filter
    }

    if( result != TSD_ERR_OK )
    {
        pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);
    _trace_leave();
    return result;
}

TSD_ERR
tsd_Change_SoundTrack(
    TSD_HANDLE      *pHTsd,
    uint32_t        index,
    void            *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = DOWN_CAST(ITE_TSD_DEV, pHTsd, hTsd);

    _trace_enter("0x%x, %d, 0x%x\n", pHTsd, index, extraData);

    _verify_handle(pHTsd, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

    if( pTsdDev && pTsdDev->tsdStatus != TSD_STATUS_FAIL )
    {
        TSD_SRVC_USER_INFO  *pCurSrvcUserInfo = 0;
        TS_SERVICE_INFO     *pSrvcInfo = 0;

        do{
            if( pTsdDev->hTsd.actSrvcIdx >= pTsdDev->hTsd.totalSrvc )
            {
                tsd_msg_ex(TSD_MSG_TYPE_ERR, " action service index (%d) out range, max=%d !",
                                pTsdDev->hTsd.actSrvcIdx, pTsdDev->hTsd.totalSrvc - 1);
                break;
            }

            pCurSrvcUserInfo = pTsdDev->pSrvcUserInfo[pTsdDev->hTsd.actSrvcIdx];
            pSrvcInfo = DOWN_CAST(TS_SERVICE_INFO, pCurSrvcUserInfo, userInfo);

            if( index < pCurSrvcUserInfo->audioCount )
            {
                pSrvcInfo->userInfo.actAudioIdx = index;
            }
            else
            {
                // unknow audio index => recover default "0"
                pSrvcInfo->userInfo.actAudioIdx = 0;
            }

            pTsdDev->bSoundFullOut = (index == (uint32_t)(-1)) ? true : false;

            if( pTsdDev->bSoundFullOut == true )
            {
                // full sound track output
                int         j;

                for(j = 0; j < pCurSrvcUserInfo->audioCount; j++)
                {
                    if( (PID_AUDIO_INDEX - j) < PID_AUDIO_RESERVE_0 )
                    {
                        tsd_msg_ex(TSD_MSG_TYPE_ERR, " total sound tracks(%d) out support counte(%d) !",
                                        pCurSrvcUserInfo->audioCount, (PID_AUDIO_INDEX - PID_AUDIO_RESERVE_0 + 1));
                        break;
                    }
                    DemodCtrl_UpdatePidTable(pTsdDev->pHDemodCtrl, pSrvcInfo->audioPID[j], PID_AUDIO_INDEX - j);
                }
            }
            else
                DemodCtrl_UpdatePidTable(pTsdDev->pHDemodCtrl, pSrvcInfo->audioPID[pSrvcInfo->userInfo.actAudioIdx], PID_AUDIO_INDEX);

            pTsdDev->pesOutBuf_a.pid = pSrvcInfo->audioPID[pSrvcInfo->userInfo.actAudioIdx];
        }while(0);
    }

    if( result != TSD_ERR_OK )
    {
        pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);
    _trace_leave();
    return result;
}

TSD_ERR
tsd_Get_PidInfo(
    TSD_HANDLE              *pHTsd,
    uint32_t                srvcIdx,
    TSD_SERVICE_PID_INFO    *pServicePidInfo)
{
    TSD_ERR             result = TSD_ERR_OK;
    ITE_TSD_DEV         *pTsdDev = DOWN_CAST(ITE_TSD_DEV, pHTsd, hTsd);

    _trace_enter("0x%x, %d, 0x%x\n", pHTsd, srvcIdx, pServicePidInfo);

    _verify_handle(pHTsd, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

    if( pTsdDev && pTsdDev->tsdStatus != TSD_STATUS_FAIL )
    {
        do{
            TSD_SRVC_USER_INFO  *tmpSrvcUserInfo = 0;
            TS_SERVICE_INFO     *pSrvcInfo = 0;

            if( srvcIdx >= pTsdDev->hTsd.totalSrvc )
            {
                tsd_msg_ex(TSD_MSG_TYPE_ERR, " service index (%d) out range, max=%d !", srvcIdx, pTsdDev->hTsd.totalSrvc - 1);
                //result = TSD_ERR_INVALID_PARAMETER;
                break;
            }

            tmpSrvcUserInfo = pTsdDev->pSrvcUserInfo[srvcIdx];
            pSrvcInfo = DOWN_CAST(TS_SERVICE_INFO, tmpSrvcUserInfo, userInfo);
            if( pServicePidInfo )
            {
                pServicePidInfo->videoPID  = pSrvcInfo->videoPID;
                pServicePidInfo->videoType = pSrvcInfo->videoType;
                memcpy(pServicePidInfo->audioPID, pSrvcInfo->audioPID, sizeof(pServicePidInfo->audioPID));
                memcpy(pServicePidInfo->audioType, pSrvcInfo->audioType, sizeof(pServicePidInfo->audioType));
                memcpy(pServicePidInfo->subtitlePID, pSrvcInfo->subtitlePID, sizeof(pServicePidInfo->subtitlePID));
            }
        }while(0);
    }

    if( result != TSD_ERR_OK )
    {
        pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);
    _trace_leave();
    return result;
}


TSD_ERR
tsd_Control(
    TSD_HANDLE      *pHTsd,
    TSD_CTRL_CMD    cmd,
    uint32_t        *value,
    void            *extraData)
{
    TSD_ERR         result = TSD_ERR_OK;
    ITE_TSD_DEV     *pTsdDev = DOWN_CAST(ITE_TSD_DEV, pHTsd, hTsd);

    _trace_enter("0x%x, %d, %d, 0x%x\n", pHTsd, cmd, value, extraData);

    _verify_handle(pHTsd, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);

    if( pTsdDev && pTsdDev->tsdStatus != TSD_STATUS_FAIL )
    {
        switch( cmd )
        {
            //case TSD_CTRL_RECOVER_SRVC_INFO:
            //    _Gen_User_Info(pTsdDev);
            //    break;

            case TSD_CTRL_DEMOD_RESET:
                DemodCtrl_Terminate(pTsdDev->pHDemodCtrl);
                break;

            case TSD_CTRL_RESET_TS_DB:
                if( pTsdDev->pSrvcUserInfo )    { free(pTsdDev->pSrvcUserInfo); pTsdDev->pSrvcUserInfo = NULL; }
                if( pTsdDev->pChnlUserInfo )    { free(pTsdDev->pChnlUserInfo); pTsdDev->pChnlUserInfo = NULL; }

                // clear info database
                tsSrvc_Control(pTsdDev->pHTsSrvc, TS_SRVC_CTL_DEL_ALL_SRVC_INFO, 0, 0);
                tsChnl_Control(pTsdDev->pHTsChnl, TS_CHNL_CTL_DEL_ALL_CHNL_INFO, 0, 0);

                // reset parameters
                pTsdDev->hTsd.totalChnl = 0;
                pTsdDev->hTsd.totalSrvc = 0;
                pTsdDev->hTsd.actChnlIdx = (uint32_t)(-1);
                pTsdDev->hTsd.actSrvcIdx = (uint32_t)(-1);
                pTsdDev->hTsd.actFreq    = 0;
                break;

            case TSD_CTRL_DEMOD_SUSPEND_FIRE:
                #if !(ENABLE_SW_SIMULATION)
                DemodCtrl_SetPowerDown(pTsdDev->pHDemodCtrl, (uint32_t*)(!!(value)));
                #endif
                break;

            case TSD_CTRL_SET_DEMOD_SUSPEND_MODE:
                #if !(ENABLE_SW_SIMULATION)
                if( (pTsdDev->bDemodSuspend == false && !value) ||
                    (pTsdDev->bDemodSuspend == true && value) )
                    break;

                if( value )
                {
                    // enter suspend
                    pTsdDev->bDemodSuspend = true;
                    DemodCtrl_SetSuspend(pTsdDev->pHDemodCtrl, pTsdDev->bDemodSuspend);
                    switch( pTsdDev->tsRecevier.tsrx_type )
                    {
                        case TSD_TSRX_TYPE_TSI:
                            mmpTsiDisable(pTsdDev->tsiId);
                            //mmpTsiTerminate(pTsdDev->tsiId); // need to do ????
                            break;

                        case TSD_TSRX_TYPE_USB:
                            if( pTsdDev->tsRecevier.ts_rx_turn_off )
                            {
                                pTsdDev->tsRecevier.ts_rx_turn_off(&pTsdDev->tsRecevier, 0);
                            }
                            //if( pTsdDev->tsRecevier.ts_rx_deinit )
                            //{
                            //    pTsdDev->tsRecevier.ts_rx_deinit(&pTsdDev->tsRecevier, 0);
                            //}
                            break;
                        default:        break;
                    }
                    usleep(10000); // need to do ?????
                }
                else
                {
                    // wake up
                    pTsdDev->bDemodSuspend = false;
                    DemodCtrl_SetSuspend(pTsdDev->pHDemodCtrl, pTsdDev->bDemodSuspend);
                    switch( pTsdDev->tsRecevier.tsrx_type )
                    {
                        case TSD_TSRX_TYPE_TSI:
                            // mmpTsiInitialize(pTsdDev->tsiId); // need to do ?????
                            mmpTsiEnable(pTsdDev->tsiId);
                            break;

                        case TSD_TSRX_TYPE_USB:
                            //if( pTsdDev->tsRecevier.ts_rx_init )
                            //{
                            //    pTsdDev->tsRecevier.ts_rx_init(&pTsdDev->tsRecevier, 0);
                            //}
                            if( pTsdDev->tsRecevier.ts_rx_turn_on )
                            {
                                pTsdDev->tsRecevier.ts_rx_turn_on(&pTsdDev->tsRecevier, 0);
                            }
                            break;
                        default:        break;
                    }

                    usleep(10000);  // need to do ?????
                }
                #endif
                break;

            case TSD_CTRL_SKIP_EPG_PARSING:
                pTsdDev->bSkipEpgParsing = (value)? true : false;
                tsEpg_Control(pTsdDev->pHEpg, TS_EPG_CTL_SKIP_EPG, (uint32_t*)((value)? true : false), 0);
                break;

            case TSD_CTRL_RESET_ACT_INFO:
                pTsdDev->hTsd.actChnlIdx = (uint32_t)(-1);
                pTsdDev->hTsd.actSrvcIdx = (uint32_t)(-1);
                // pTsdDev->hTsd.actFreq    = 0;
                break;

            case TSD_CTRL_SET_DEMOD_STATUS:
                {
                    DEMOD_ENG_STATUS    status = 0;
                    switch( (TSD_DEMOD_STATUS)value )
                    {
                        case TSD_DEMOD_STATUS_IDLE:     status = DEMOD_ENG_STATUS_IDLE;    break;
                        case TSD_DEMOD_STATUS_RUNNING:  status = DEMOD_ENG_STATUS_RUNNING;  break;
                        default:                        status = DEMOD_ENG_STATUS_UNKNOW;  break;
                    }
                    DemodCtrl_Set_Engine_Status(pTsdDev->pHDemodCtrl, status);
                }
                break;

            case TSD_CTRL_SORT_SRVC_INFO:
                // sort userInfo array
                pTsdDev->actSortTbl = TSD_ACT_SORT_USER_SRVC_INFO;
                if( (TSD_INFO_SORT_TYPE)value != TSD_INFO_SORT_NONE )
                    _Sort_Func(pTsdDev, (TSD_INFO_SORT_TYPE)(value));
                break;

            case TSD_CTRL_ENABLE_TSI:
                #if !(ENABLE_SW_SIMULATION)
                switch( pTsdDev->tsRecevier.tsrx_type )
                {
                    case TSD_TSRX_TYPE_TSI:
                        mmpTsiEnable(pTsdDev->tsiId);
                        break;

                    case TSD_TSRX_TYPE_USB:
                        if( pTsdDev->tsRecevier.ts_rx_turn_on )
                        {
                            pTsdDev->tsRecevier.ts_rx_turn_on(&pTsdDev->tsRecevier, 0);
                        }
                        break;

                    case TSD_TSRX_TYPE_CUSTMOR:
                    default:        break;
                }
                #endif
                break;

            case TSD_CTRL_DISABLE_TSI:
                #if !(ENABLE_SW_SIMULATION)
                switch( pTsdDev->tsRecevier.tsrx_type )
                {
                    case TSD_TSRX_TYPE_TSI:
                        mmpTsiDisable(pTsdDev->tsiId);
                        break;

                    case TSD_TSRX_TYPE_USB:
                        if( pTsdDev->tsRecevier.ts_rx_turn_off )
                        {
                            pTsdDev->tsRecevier.ts_rx_turn_off(&pTsdDev->tsRecevier, 0);
                        }
                        break;

                    case TSD_TSRX_TYPE_CUSTMOR:
                    default:        break;
                }
                #endif
                break;

            case TSD_CTRL_GET_CURR_UTC_TIME:
                if( extraData && pTsdDev->pHEpg )
                {
                    PSI_YMDHMS_TIME     YmdHmsTime = {0};
                    TSD_UTC_TIME        *pUtcTime = (TSD_UTC_TIME*)extraData;

                    if( !pTsdDev->pHEpg )
                    {
                        memset(extraData, 0x00, sizeof(TSD_UTC_TIME));
                        break;
                    }

                    YmdHmsTime = psiTime_MjdBcdToYmdHms(pTsdDev->pHEpg->curUtcTime);

                    pUtcTime->year   = YmdHmsTime.year;
                    pUtcTime->month  = YmdHmsTime.month;
                    pUtcTime->day    = YmdHmsTime.day;
                    pUtcTime->hour   = YmdHmsTime.hour;
                    pUtcTime->minute = YmdHmsTime.minute;
                    pUtcTime->second = YmdHmsTime.second;
                }
                break;

            case TSD_CTRL_GET_SIGNAL_STATUS:
                #if !(ENABLE_SW_SIMULATION)
                do
                {
                    DEMOD_SIGNAL_STATISTIC  tDemodStatistic = {0};
                    uint32_t                i2c_clock_rate = 0;

                    if( !value )    break;

                    *value = 0;
                    //i2c_clock_rate = mmpIicGetClockRate();
                    //mmpIicSetClockRate(IIC_PORT_0, 200* 1024);
                    if( !DemodCtrl_GetSignalStatus(pTsdDev->pHDemodCtrl, &tDemodStatistic) )
                        break;

                    if( !tDemodStatistic.bMpg2Lock )    break;

                    *value = tDemodStatistic.signalQuality;

                    //mmpIicSetClockRate(IIC_PORT_0, i2c_clock_rate);
                }while(0);
                #endif
                break;

            case TSD_CTRL_GET_COUNTRY_FREQ_CNT:
                if( value )
                {
                    int                 i = 0;
                    TS_COUNTRY_INFO     *tsCountryInfo = 0;

                    tsCountryInfo = _Country_To_Freq((TSD_COUNTRY_ID)extraData, 0);
                    pTsdDev->hTsd.scan_total_times = 0;

                    while( tsCountryInfo && tsCountryInfo->freq_band_DB[i] )
                    {
                        pTsdDev->hTsd.scan_total_times += tsCountryInfo->freq_band_DB[i]->totalCount;
                        i++;
                    }

                    *value = (uint32_t)pTsdDev->hTsd.scan_total_times;
                }
                break;

            case TSD_CTRL_IMPORT_INFO:
                // check condition
                if( !extraData )
                {
                    tsd_msg_ex(1, "\n\tNo any TSD_INFO_REPO info !!");
                }
                else
                {
                    TSD_INFO_REPO       tsdInfoRepo = {0};

                    memcpy(&tsdInfoRepo, extraData, sizeof(TSD_INFO_REPO));

                    // action
                    if( !tsdInfoRepo.bSkipImport )
                    {
                        int32_t             repoRst = 0;
                        TS_CHNL_INFO        tmp_chnl_info = {0};
                        TS_SERVICE_INFO     tmp_srvc_info = {0};
                        TSD_INFO_REPO       *pTsdInfoReop = &tsdInfoRepo;

                        // clear info database
                        tsSrvc_Control(pTsdDev->pHTsSrvc, TS_SRVC_CTL_DEL_ALL_SRVC_INFO, 0, 0);
                        tsChnl_Control(pTsdDev->pHTsChnl, TS_CHNL_CTL_DEL_ALL_CHNL_INFO, 0, 0);

                        pTsdInfoReop->repoType = TSD_REPO_UNKNOW_INFO;
                        if( pTsdInfoReop->tsd_repo_open )
                            repoRst = pTsdInfoReop->tsd_repo_open(pTsdInfoReop, 0);

                        if( pTsdInfoReop->tsd_repo_import )
                        {
                            int     totalCnt = 0;
                            int     i;

                            // tsd_repo_import channel info
                            totalCnt = pTsdInfoReop->totalChnls;
                            pTsdInfoReop->repoType = TSD_REPO_CHANNEL_INFO;

                            i = 0;
                            while( i < totalCnt )
                            {
                                repoRst = pTsdInfoReop->tsd_repo_import(pTsdInfoReop, (uint8_t*)&tmp_chnl_info, sizeof(TS_CHNL_INFO));
                                if( repoRst != sizeof(TS_CHNL_INFO) )  break;
                                tsChnl_AddChannel(pTsdDev->pHTsChnl, &tmp_chnl_info, 0, 0);

                                i++;
                            }

                            // tsd_repo_import service info
                            totalCnt = pTsdInfoReop->totalSrvc;
                            pTsdInfoReop->repoType = TSD_REPO_SERVICE_INFO;

                            i = 0;
                            while( i < totalCnt )
                            {
                                repoRst = pTsdInfoReop->tsd_repo_import(pTsdInfoReop, (uint8_t*)&tmp_srvc_info, sizeof(TS_SERVICE_INFO));
                                if( repoRst != sizeof(TS_SERVICE_INFO) )  break;
                                tsSrvc_AddService(pTsdDev->pHTsSrvc, &tmp_srvc_info, 0, 0);

                                i++;
                            }
                        }

                        pTsdInfoReop->repoType = TSD_REPO_UNKNOW_INFO;
                        if( pTsdInfoReop->tsd_repo_close )
                            repoRst = pTsdInfoReop->tsd_repo_close(pTsdInfoReop, 0);

                        // create userInfo array
                        _Gen_User_Info(pTsdDev);

                    }
                }
                break;

            case TSD_CTRL_EXPORT_INFO:
                // check condition
                if( !extraData )
                {
                    tsd_msg_ex(1, "\n\tNo any TSD_INFO_REPO info !!");
                }
                else
                {
                    TSD_INFO_REPO       tsdInfoRepo = {0};

                    memcpy(&tsdInfoRepo, extraData, sizeof(TSD_INFO_REPO));

                    // action
                    if( !tsdInfoRepo.bSkipExport )
                    {
                        int32_t             repoRst = 0;
                        TS_CHNL_INFO        *pTmp_chnl_info = 0;
                        TS_CHNL_INFO        *pCur_chnl_info = 0;
                        TS_SERVICE_INFO     *pTmp_srvc_info = 0;
                        TS_SERVICE_INFO     *pCur_srvc_info = 0;
                        TSD_INFO_REPO       *pTsdInfoReop = &tsdInfoRepo;

                        pTsdInfoReop->repoType = TSD_REPO_UNKNOW_INFO;
                        pTsdInfoReop->totalChnls = pTsdDev->pHTsChnl->totalChnls;
                        pTsdInfoReop->totalSrvc  = pTsdDev->pHTsSrvc->totalSrvc;

                        if( pTsdInfoReop->tsd_repo_open )
                            repoRst = pTsdInfoReop->tsd_repo_open(pTsdInfoReop, 0);

                        if( pTsdInfoReop->tsd_repo_export )
                        {
                            int     totalCnt = 0;
                            int     i;

                            // tsd_repo_export channel info
                            pTsdInfoReop->repoType = TSD_REPO_CHANNEL_INFO;
                            pCur_chnl_info = pTsdDev->pHTsChnl->pStartChnlInfo;

                            totalCnt = pTsdDev->pHTsChnl->totalChnls;
                            i = 0;
                            while( i < totalCnt )
                            {
                                tsChnl_GetChannelInfo(pTsdDev->pHTsChnl, i, pCur_chnl_info, &pTmp_chnl_info, 0);
                                repoRst = pTsdInfoReop->tsd_repo_export(pTsdInfoReop, (uint8_t*)pTmp_chnl_info, sizeof(TS_CHNL_INFO));
                                if( repoRst != sizeof(TS_CHNL_INFO) )  break;

                                pCur_chnl_info = pTmp_chnl_info;
                                i++;
                            }

                            // tsd_repo_export service info
                            pTsdInfoReop->repoType = TSD_REPO_SERVICE_INFO;
                            pCur_srvc_info = pTsdDev->pHTsSrvc->pStartSrvcInfo;

                            totalCnt = pTsdDev->pHTsSrvc->totalSrvc;
                            i = 0;
                            while( i < totalCnt )
                            {
                                tsSrvc_GetServiceInfo(pTsdDev->pHTsSrvc, i, pCur_srvc_info, &pTmp_srvc_info, 0);
                                repoRst = pTsdInfoReop->tsd_repo_export(pTsdInfoReop, (uint8_t*)pTmp_srvc_info, sizeof(TS_SERVICE_INFO));
                                if( repoRst != sizeof(TS_SERVICE_INFO) )  break;

                                pCur_srvc_info = pTmp_srvc_info;
                                i++;
                            }
                        }

                        pTsdInfoReop->repoType = TSD_REPO_UNKNOW_INFO;
                        if( pTsdInfoReop->tsd_repo_close )
                            repoRst = pTsdInfoReop->tsd_repo_close(pTsdInfoReop, 0);
                    }
                }
                break;

            default:
                result = TSD_ERR_NO_IMPLEMENT;
                break;
        }
    }

    if( result != TSD_ERR_OK &&
        result != TSD_ERR_NO_IMPLEMENT )
    {
        pTsdDev->tsdStatus = TSD_STATUS_FAIL;
        tsd_msg_ex(TSD_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_TSD, pTsdDev->tsd_data_mutex);
    _trace_leave();
    return result;
}

