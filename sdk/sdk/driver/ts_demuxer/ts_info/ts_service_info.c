
#include "list_template.h"
#include "ts_service_info.h"
#include "ts_demuxer_defs.h"
#include "psi_descriptor_kit.h"

//=============================================================================
//				  Constant Definition
//=============================================================================
// descriptor tag value
// H.222 table 2-39 - program and program element descriptors
#define REGISTRATION_DESCRIPTOR             (0x5)
#define CA_DESCRIPTOR                       (0x9)
#define ISO_639_LANGUAGE_DESCRIPTOR         (0xA)


#define INVALID_SERVICE_INDEX               (0xFFFF)

#define INVALID_TELETEXT_INDEX              (0xFF)
#define INVALID_SUBTITLE_INDEX              (0xFF)
#define INVALID_SERVICE_NUMBER              (0xFFFF)

typedef enum _SRVC_STATUS_TAG
{
    SRVC_STATUS_IDLE = 0x11,
    SRVC_STATUS_BUSY = 0xBB,
    SRVC_STATUS_FAIL = 0xFF, 

}SRVC_STATUS;

typedef enum _SRVC_FIND_CMP_TYPE_TAG
{
    SRVC_FIND_CMP_OREDER_NUM, 
    SRVC_FIND_CMP_PROGRAM_NUM,
    SRVC_FIND_CMP_CUSTOMIZE_0, 
    
}SRVC_FIND_CMP_TYPE;

//=============================================================================
//				  Macro Definition
//=============================================================================
#if _MSC_VER 
    #include "ts_txt_conv.h"
    #include "locale.h"
    #define _show_srvc_txt(srvc_txt, srvc_txt_length) \
                do{ wchar_t tmp_txt[512] = {0}; \
                    tsTxt_ConvToUtf16(tmp_txt, srvc_txt, srvc_txt_length, 0); \
                    setlocale(LC_ALL, "cht");   \
                    wprintf(L"- %s\n", tmp_txt); \
                }while(0)
#elif 1
    #include "ts_txt_conv.h"
    #include "ts_demuxer/ts_debug.h"
    #define _show_srvc_txt(srvc_txt, srvc_txt_length) \
                do{ char utf8_txt[256] = {0};   \
                    uint16_t utf16_txt[256] = {0}; \
                    unsigned int bytes_used = 0;    \
                    tsTxt_ConvToUtf16(utf16_txt, srvc_txt, srvc_txt_length, 0); \
                    utf16le_to_utf8(utf16_txt, utf8_txt, 256, &bytes_used); \
                    printf("%s", utf8_txt);    \
                }while(0)
#else
    #define _show_srvc_txt(srvc_txt, srvc_txt_length)
#endif

#define ENABLE_WARNING_MSG      0

//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct _srvc_cmp_data_tag
{
    uint32_t    frequency;
    uint32_t    program_number;
    
}srvc_cmp_data;

/**
 * psi table parsing info for database
 **/
typedef struct _TS_SRVC_PRS_INFO_TAG
{
    uint32_t            cur_version_number;
    uint32_t            srvc_cnt;
    TS_SERVICE_INFO     *pFirstSrvc;
    TS_SERVICE_INFO     *pCurSrvc;

}TS_SRVC_PRS_INFO;


typedef struct _TS_SRVC_DB_TAG
{
    TS_SRVC_HANDLE      hSrvc;
    
    pthread_mutex_t     psi_collect_mutex;
    
    SRVC_STATUS         srvcStatus;

    PID_STATISTICS_CALLBACK  Pid_Stat_cb;
    void                     *pTspStatInfo;
    
    bool                bSkip;    // lock/unlok or other applications
    TS_CHNL_INFO        curChnlInfo;

    TS_SRVC_PRS_INFO    act_srvc_info;    // action service info (total services)
    //uint32_t            srvc_order_cnt;   // for order number ?? 

    // PAT info
    uint32_t            pat_total_srvc; // total PMTs in PAT
    uint32_t            pat_version_number;

    // PMT info
    TS_SRVC_PRS_INFO    pmt_srvc_info;  // pmt info in one channel 
    
    // SDT info
    TS_SRVC_PRS_INFO    sdt_srvc_info;  // pmt info in one channel 
    
    // ts file case 
    TSD_CTRL_STATUS     tsdCtrlStatus;

}TS_SRVC_DB;

//=============================================================================
//				  Global Data Definition
//=============================================================================

//=============================================================================
//				  Private Function Definition
//=============================================================================
DEFINE_LIST_TEMPLATE(TS_SERVICE_INFO);

static int
_Search_Func(
    int             cmpMode,
    TS_SERVICE_INFO *pCur, 
    void            *pattern)
{
    int             rst = 0;
    srvc_cmp_data   *pCmpData = 0;
    
    switch( cmpMode )
    {
        case SRVC_FIND_CMP_OREDER_NUM:
            rst = (pCur->order_num == (uint32_t)pattern);
            break;
        case SRVC_FIND_CMP_PROGRAM_NUM:
            pCmpData = (srvc_cmp_data*)pattern;
            
            rst = ((pCur->tsChnlInfo.userInfo.frequency == pCmpData->frequency) && 
                   (pCur->programNumber == pCmpData->program_number ));
            break;
        case SRVC_FIND_CMP_CUSTOMIZE_0:
            rst = ((pCur->userInfo.serviceName[0] == 0) && (pCur->programNumber == (uint32_t)pattern));
            break;
    }
    return rst;
}



static uint32_t
_Add_to_Service_List(
    TS_SRVC_DB          *pSrvcDb,
    TS_SRVC_PRS_INFO    *pSrvcPrsInfo,
    TS_SERVICE_INFO     *pCurTsSrvcInfo,
    bool                bCheckExist)
{
    uint32_t            result = 0;
    TS_SERVICE_INFO     *pNewTsSrvcInfo = tsd_malloc(sizeof(TS_SERVICE_INFO));
    
    // needt mutex
    // To Do:

    do{
        if( !pNewTsSrvcInfo )   break;

        //memset(pNewTsSrvcInfo, 0x0, sizeof(TS_SERVICE_INFO));
        memcpy(pNewTsSrvcInfo, pCurTsSrvcInfo, sizeof(TS_SERVICE_INFO));
        pNewTsSrvcInfo->order_num = pSrvcPrsInfo->srvc_cnt;

        if( !pSrvcPrsInfo->pFirstSrvc )
        {
            pNewTsSrvcInfo->version_number = pSrvcPrsInfo->cur_version_number;
            pSrvcPrsInfo->pFirstSrvc = pSrvcPrsInfo->pCurSrvc = pNewTsSrvcInfo;
            LIST_INIT(pSrvcPrsInfo->pCurSrvc);
            pSrvcPrsInfo->srvc_cnt++;
        }
        else
        {
            TS_SERVICE_INFO     *pTmpTsSrvcInfo = 0;
            
            if( bCheckExist )
            {
                srvc_cmp_data   cmpData = {0};

                cmpData.frequency      = pCurTsSrvcInfo->tsChnlInfo.userInfo.frequency;
                cmpData.program_number = pCurTsSrvcInfo->programNumber;
                pTmpTsSrvcInfo = LIST_FIND(TS_SERVICE_INFO, 
                                           _Search_Func, SRVC_FIND_CMP_PROGRAM_NUM, 
                                           pSrvcPrsInfo->pCurSrvc, 
                                           &cmpData);
            }

            if( !pTmpTsSrvcInfo )
            {
                pNewTsSrvcInfo->version_number = pSrvcPrsInfo->cur_version_number;
                pSrvcPrsInfo->pCurSrvc = LIST_ADD(TS_SERVICE_INFO, pSrvcPrsInfo->pCurSrvc, pNewTsSrvcInfo);
                pSrvcPrsInfo->srvc_cnt++;
            }
            else
            {
                uint32_t         srvc_order_cnt = pTmpTsSrvcInfo->order_num; 
                TS_SERVICE_INFO  *pNext, *pPrev;

                // Idealy, we need to del old and new last node, 
                // but AP layer (user_info) save the pointer, How to sync ????

            #if 0
                // del exist service info
                pSrvcPrsInfo->pCurSrvc = LIST_DEL(TS_SERVICE_INFO, pTmpTsSrvcInfo);
                free(pTmpTsSrvcInfo);
                
                // update service info
                pNewTsSrvcInfo->order_num = srvc_order_cnt;
                pSrvcPrsInfo->pCurSrvc = LIST_ADD(TS_SERVICE_INFO, pSrvcPrsInfo->pCurSrvc, pNewTsSrvcInfo);
            #else
                pNext = pTmpTsSrvcInfo->next;
                pPrev = pTmpTsSrvcInfo->prev;
                memcpy(pTmpTsSrvcInfo, pNewTsSrvcInfo, sizeof(TS_SERVICE_INFO));

                pTmpTsSrvcInfo->version_number = pSrvcPrsInfo->cur_version_number;
                pTmpTsSrvcInfo->next      = pNext;
                pTmpTsSrvcInfo->prev      = pPrev;
                pTmpTsSrvcInfo->order_num = srvc_order_cnt;
                free(pNewTsSrvcInfo);
            #endif
            }
        }
    }while(0);

    return result;
}

static void
_Del_Service_List(
    TS_SRVC_DB          *pSrvcDb,
    TS_SRVC_PRS_INFO    *pSrvcPrsInfo)
{
    TS_SERVICE_INFO  *tmp_srvc_info = 0;
    uint32_t         i;

    // needt mutex
    // To Do:

    do{
        if( !pSrvcPrsInfo )     break;

        if( pSrvcPrsInfo->pFirstSrvc && pSrvcPrsInfo->pCurSrvc )      
        {
            for(i = 0; i < pSrvcPrsInfo->srvc_cnt; i++)
            {
                tmp_srvc_info = LIST_FIND(TS_SERVICE_INFO, _Search_Func, 
                                          SRVC_FIND_CMP_OREDER_NUM, 
                                          pSrvcPrsInfo->pCurSrvc, i);
                if( tmp_srvc_info )  
                {
                    pSrvcPrsInfo->pCurSrvc = LIST_DEL(TS_SERVICE_INFO, tmp_srvc_info);
                    free(tmp_srvc_info);
                }
            }
        }

        memset(pSrvcPrsInfo, 0x0, sizeof(TS_SRVC_PRS_INFO));
        pSrvcPrsInfo->cur_version_number = (uint32_t)-1;
        
    }while(0);

    return;
}

static uint32_t
_Merge_Service_List(
    TS_SRVC_DB          *pSrvcDb_master,
    TS_SRVC_DB          *pSrvcDb_slave,
    TS_SRVC_MERGE_TYPE  type)
{
    uint32_t            result = 0;
    srvc_cmp_data       cmpData = {0};
    TS_SERVICE_INFO     *pTmp_Sdt_srvc = 0;
    TS_SERVICE_INFO     *pTmp_Pmt_srvc = 0;
    TS_SERVICE_INFO     *pCur_Sdt_srvc = 0;
    TS_SERVICE_INFO     *pCur_Pmt_srvc = 0;
    uint32_t            i;

    do{
        switch( type )
        {
            default:
            case TS_SRVC_MERGE_DEFAULT:
                pCur_Sdt_srvc = pSrvcDb_master->sdt_srvc_info.pFirstSrvc;
                pCur_Pmt_srvc = pSrvcDb_master->pmt_srvc_info.pFirstSrvc;

                // compare sdt_service_list and pmt_service_list
                for(i = 0; i < pSrvcDb_master->sdt_srvc_info.srvc_cnt; i++)
                {                    
                    pTmp_Sdt_srvc = LIST_FIND(TS_SERVICE_INFO, _Search_Func, 
                                              SRVC_FIND_CMP_OREDER_NUM, 
                                              pCur_Sdt_srvc, i);
                      
                    if( !pTmp_Sdt_srvc )    continue;
                    else                    pCur_Sdt_srvc = pTmp_Sdt_srvc;
                        
                    cmpData.frequency      = pCur_Sdt_srvc->tsChnlInfo.userInfo.frequency;
                    cmpData.program_number = pCur_Sdt_srvc->programNumber;

                    pTmp_Pmt_srvc = LIST_FIND(TS_SERVICE_INFO, _Search_Func, 
                                              SRVC_FIND_CMP_PROGRAM_NUM, 
                                              pCur_Pmt_srvc, 
                                              &cmpData);
                    if( pTmp_Pmt_srvc )
                    {
                        TS_SERVICE_INFO     *pValid_Pmt_srvc = 0;
                        
                        pValid_Pmt_srvc = pTmp_Pmt_srvc;
                        pCur_Pmt_srvc = LIST_DEL(TS_SERVICE_INFO, pValid_Pmt_srvc);
                        
                        memcpy(pValid_Pmt_srvc->userInfo.serviceName, 
                               pCur_Sdt_srvc->userInfo.serviceName,
                               pCur_Sdt_srvc->userInfo.nameSize);
                        pValid_Pmt_srvc->userInfo.nameSize = pCur_Sdt_srvc->userInfo.nameSize;

                        _Add_to_Service_List(pSrvcDb_master, &pSrvcDb_master->act_srvc_info, pValid_Pmt_srvc, true);
                        printf("\tprogram_num: pmt(0x%x), sdt(0x%x), ", 
                                pValid_Pmt_srvc->programNumber,
                                pCur_Sdt_srvc->programNumber);
                        _show_srvc_txt(pValid_Pmt_srvc->userInfo.serviceName, pValid_Pmt_srvc->userInfo.nameSize);
                        printf("\n");
                    }

                    pTmp_Sdt_srvc = pCur_Sdt_srvc;
                    pCur_Sdt_srvc = LIST_DEL(TS_SERVICE_INFO, pTmp_Sdt_srvc);
                    free(pTmp_Sdt_srvc);
                }

                memset(&pSrvcDb_master->sdt_srvc_info, 0x0, sizeof(TS_SRVC_PRS_INFO));
                pSrvcDb_master->sdt_srvc_info.cur_version_number = (uint32_t)-1;

                // del pmt_service_list
                pSrvcDb_master->pmt_srvc_info.pCurSrvc = pCur_Pmt_srvc;
                _Del_Service_List(pSrvcDb_master, &pSrvcDb_master->pmt_srvc_info);
                
                srvc_msg(1, "\n\n\tGet total action services %d \n\n", pSrvcDb_master->act_srvc_info.srvc_cnt);
                break;
                
            //case TS_SRVC_MERGE_BASE_PMT:
            //    break;
                
            //case TS_SRVC_MERGE_BASE_SDT:
            //    break;

            case TS_SRVC_MERGE_2_HANDLE:
                do{
                    TS_SRVC_PRS_INFO    *pAct_srvc_info_master = &pSrvcDb_master->act_srvc_info;
                    TS_SRVC_PRS_INFO    *pAct_srvc_info_slave = &pSrvcDb_slave->act_srvc_info;
                    TS_SERVICE_INFO     *pCur_Master_srvc = 0;
                    TS_SERVICE_INFO     *pCur_Slave_srvc = 0;

                    if( !pAct_srvc_info_slave->srvc_cnt )       break;

                    pCur_Master_srvc = pAct_srvc_info_master->pFirstSrvc;
                    pCur_Slave_srvc = pAct_srvc_info_slave->pFirstSrvc;
                    for(i = 0; i < pAct_srvc_info_slave->srvc_cnt; i++)
                    {
                        pCur_Slave_srvc = LIST_FIND(TS_SERVICE_INFO, _Search_Func, 
                                                    SRVC_FIND_CMP_OREDER_NUM, 
                                                    pCur_Slave_srvc, i);

                        if( !pCur_Slave_srvc )    
                        {
                            pCur_Slave_srvc = pAct_srvc_info_slave->pFirstSrvc;
                            continue; 
                        }

                        cmpData.frequency      = pCur_Slave_srvc->tsChnlInfo.userInfo.frequency;
                        cmpData.program_number = pCur_Slave_srvc->programNumber;
                        
                        pCur_Master_srvc = LIST_FIND(TS_SERVICE_INFO, _Search_Func, 
                                                     SRVC_FIND_CMP_PROGRAM_NUM, 
                                                     pCur_Master_srvc, 
                                                     &cmpData);
                        if( !pCur_Master_srvc )
                        {
                            // add slave_srvc to master_info
                            _Add_to_Service_List(pSrvcDb_master, &pSrvcDb_master->act_srvc_info, pCur_Slave_srvc, true);
                            pCur_Master_srvc = pAct_srvc_info_master->pFirstSrvc;
                        }                                              
                    }
                }while(0);
                break;
        }

        pSrvcDb_master->hSrvc.pStartSrvcInfo = pSrvcDb_master->act_srvc_info.pFirstSrvc;
        pSrvcDb_master->hSrvc.pCurSrvcInfo   = pSrvcDb_master->act_srvc_info.pCurSrvc;
        pSrvcDb_master->hSrvc.totalSrvc      = pSrvcDb_master->act_srvc_info.srvc_cnt; 
    }while(0);

    return result;
}

/////////////////////////////////////////////
// psi callback for collecting data
/////////////////////////////////////////////   
void
tsSrvc_PatCallBack(
    TS_DEMUX*       ptDemux,
    PSI_PAT_INFO*   ptPatInfo)
{
    TS_SRVC_DB      *pSrvcDb = DOWN_CAST(TS_SRVC_DB, ptDemux->pHTsSrvc, hSrvc);
    bool            bReady_PAT = false;
    uint32_t        bandwidth = (uint32_t)-1;
    uint32_t        frequency = (uint32_t)-1;
    uint32_t        totalSrvcCnt = 0;

    _trace_enter("\n");

    if( ptDemux->pHTsSrvc && pSrvcDb && pSrvcDb->bSkip == true ) 
    {
        srvc_msg_ex(ENABLE_WARNING_MSG, " ts service info skip PAT data !!");
        _trace_leave();
        return;
    }

    do{
        if( !ptDemux->bReceivePat )
        {
            // send RECEIVE_PAT to AP layer, if need
            // To Do:
            //_TSDEMUX_EventNotify(MMP_TRUE, NOTIFY_RECEIVE_PAT, MMP_NULL);

            bReady_PAT = true;
            ptDemux->bReceivePat = 1;
        }

        if( ptDemux->pHTsSrvc && pSrvcDb &&
            ptPatInfo->version_number == pSrvcDb->pat_version_number )
            break;

        // call infoMgr_UpdatePat() after receiving SDT or NIT
        // for updating PID table
        if( !ptDemux->bWaitNitReady || ptDemux->bReceiveSdt )
        {
            //infoMgr_UpdatePat((void*) ptPatInfo);
            PSI_PAT_PROGRAM  *pCurProgram = 0;
            
            for( pCurProgram = ptPatInfo->ptFirstProgram;
                 pCurProgram != 0;
                 pCurProgram = pCurProgram->ptNextProgram )
            {
                if( pCurProgram->program_number )   totalSrvcCnt++;
            }
        }

        if( ptDemux->pHTsSrvc && pSrvcDb )
        {
            //if( pSrvcDb->curChnlInfo.userInfo.frequency )
            //{
            //    frequency = pSrvcDb->curChnlInfo.userInfo.frequency;
            //    bandwidth = pSrvcDb->curChnlInfo.userInfo.bandwidth;
            //}
            pSrvcDb->hSrvc.bReady_PAT = bReady_PAT;
            pSrvcDb->pat_total_srvc   = totalSrvcCnt;

            pSrvcDb->pat_version_number = ptPatInfo->version_number;

            srvc_msg(1, "    PAT get total service %d \n", pSrvcDb->pat_total_srvc);
            srvc_msg(1, "\t updata pat (%d, %d)\n", 
                    pSrvcDb->curChnlInfo.userInfo.frequency, 
                    pSrvcDb->curChnlInfo.userInfo.bandwidth);        
        }
    }while(0);

    _trace_leave();
    return;
}

void
tsSrvc_PmtCallBack(
    TS_DEMUX*       ptDemux,
    PSI_PMT_INFO*   ptPmtInfo)
{
    TS_SRVC_DB          *pSrvcDb = DOWN_CAST(TS_SRVC_DB, ptDemux->pHTsSrvc, hSrvc);
    bool                bReady_PMT = false;
    bool                bUpdate_pmt_info = false;
    TS_SERVICE_INFO     tsSrvcInfo = {0};

    _trace_enter("\n");
    
    if( ptDemux->pHTsSrvc && pSrvcDb && pSrvcDb->bSkip == true ) 
    {
        srvc_msg_ex(ENABLE_WARNING_MSG, " ts service info skip PMT data !!");
        _trace_leave();
        return;
    }
     
    do{
        PSI_DESCRIPTOR      *ptDescriptor = 0;
        TS_SERVICE_INFO     *pNewTsSrvcInfo = &tsSrvcInfo;
        
        if( ptDemux->pHTsSrvc && pSrvcDb && 
            pSrvcDb->tsdCtrlStatus == TSD_CTRL_NORMAL_MODE )
            break;

        // analysis PMT 
        if( ptPmtInfo->PID != PAT_PID && ptPmtInfo->PID != NIT_PID
         && ptPmtInfo->PID != SDT_PID && ptPmtInfo->PID != EIT_PID
         && ptPmtInfo->PID != TDT_TOT_PID )
        {
            PSI_PMT_ES_INFO *pCurEsInfo = 0;
            PSI_DESCRIPTOR  *pDescriptor = 0;
            uint32_t        audioIdx = 0;
            uint32_t        subtitleIdx = 0;
            uint32_t        audioAc3ChkNum = 0;
            uint32_t        format_identifier = 0;
        
            // check version_number
            if( ptDemux->pHTsSrvc && pSrvcDb )
            {
                TS_SERVICE_INFO     *pTmpTsSrvcInfo = 0;
                srvc_cmp_data       CmpData = {0};

                CmpData.frequency      = pSrvcDb->curChnlInfo.userInfo.frequency;
                CmpData.program_number = ptPmtInfo->program_number;
                pTmpTsSrvcInfo = LIST_FIND(TS_SERVICE_INFO, 
                                           _Search_Func, SRVC_FIND_CMP_PROGRAM_NUM, 
                                           pSrvcDb->pmt_srvc_info.pCurSrvc,
                                           &CmpData);
                if( pTmpTsSrvcInfo && 
                    ptPmtInfo->version_number == pTmpTsSrvcInfo->version_number )
                    break;
            }

            // need to update
            bUpdate_pmt_info = true;
            
            // skip CA system
            for( ptDescriptor = ptPmtInfo->ptFirstDescriptor;
                 ptDescriptor;
                 ptDescriptor = ptDescriptor->ptNextDescriptor )
            {
                if (CA_DESCRIPTOR == ptDescriptor->descriptor_tag)
                    return;
            } 

            memset(pNewTsSrvcInfo, 0x0, sizeof(TS_SERVICE_INFO));
            
            for( pCurEsInfo = ptPmtInfo->ptFirstEsInfo;
                 pCurEsInfo != 0;
                 pCurEsInfo = pCurEsInfo->ptNexEsInfo )
            {
                uint32_t    esPid = pCurEsInfo->elementary_PID;
                
                audioAc3ChkNum = 0;
                
                switch( pCurEsInfo->stream_type ) 
                {
                    case ISO_IEC_11172_2_VIDEO:   // mpeg1 video
                    case ISO_IEC_13818_2_VIDEO:   // mpeg2 video
                    case ISO_IEC_14496_10_VIDEO:  // avc (h.264) video
                        pNewTsSrvcInfo->videoPID  = esPid;
                        pNewTsSrvcInfo->videoType = pCurEsInfo->stream_type;
                        pNewTsSrvcInfo->userInfo.bTV = true;
            
                        // update pid analysis data to ts parser
                        if( ptDemux->pHTsSrvc && pSrvcDb && pSrvcDb->Pid_Stat_cb )
                        {
                            TS_PID_ANAL_DATA    pidData = {0};
                            
                            pidData.service_order_num = pNewTsSrvcInfo->order_num;
                            pidData.pid               = pNewTsSrvcInfo->videoPID;
                            pidData.bVideo            = pNewTsSrvcInfo->userInfo.bTV;
                            
                            pSrvcDb->Pid_Stat_cb(&pidData, pSrvcDb->pTspStatInfo);
                        }
                        break;
                        
                    case ISO_IEC_11172_3_AUDIO:  // mpeg1 audio
                    case ISO_IEC_13818_3_AUDIO:  // mpeg2 audio
                        if( audioIdx+1 < SERVICE_MAX_AUDIO_COUNT )
                        {
                            pNewTsSrvcInfo->audioPID[audioIdx] = esPid;
                            pNewTsSrvcInfo->audioType[audioIdx] = TS_SRVC_AUD_MP3;
                            audioIdx++;
            
                            // update pid analysis data to ts parser
                            if( ptDemux->pHTsSrvc && pSrvcDb && pSrvcDb->Pid_Stat_cb )
                            {
                                TS_PID_ANAL_DATA    pidData = {0};
                                
                                pidData.service_order_num = pNewTsSrvcInfo->order_num;
                                pidData.pid               = esPid;
                                pidData.bVideo            = false;
                                
                                pSrvcDb->Pid_Stat_cb(&pidData, pSrvcDb->pTspStatInfo);
                            }
                        }
                        break;
                        
                    case ISO_IEC_13818_7_AUDIO:
                    case ISO_IEC_14496_3_AUDIO:
                        // aac latm,adts audio
                        if( audioIdx+1 < SERVICE_MAX_AUDIO_COUNT )
                        {
                            pNewTsSrvcInfo->audioPID[audioIdx] = (uint16_t) pCurEsInfo->elementary_PID;
                            pNewTsSrvcInfo->audioType[audioIdx] = TS_SRVC_AUD_AAC;
                            audioIdx++;
                            
                            // update pid analysis data to ts parser
                            if( ptDemux->pHTsSrvc && pSrvcDb && pSrvcDb->Pid_Stat_cb )
                            {
                                TS_PID_ANAL_DATA    pidData = {0};
                                
                                pidData.service_order_num = pNewTsSrvcInfo->order_num;
                                pidData.pid               = esPid;
                                pidData.bVideo            = false;
                                
                                pSrvcDb->Pid_Stat_cb(&pidData, pSrvcDb->pTspStatInfo);
                            }
                        }
                        break;
                        
                    default:
                        pDescriptor = pCurEsInfo->ptFirstDescriptor;
                        while( pDescriptor )
                        {
                            switch( pDescriptor->descriptor_tag )
                            {
                                case 0x6A:  // AC3_descriptor
                                case 0x7A:  // enhanced_AC3_descriptor
                                    audioAc3ChkNum++;
                                    break;
                                case 0x50:  // component_descriptor
                                    if (0x04 == (*pDescriptor->pPayload & 0x0F)) // reserved for AC3
                                        audioAc3ChkNum++;
                                    break;
                                case 0x5: // registration_descriptor
                                    if( pDescriptor->descriptor_length >= 4 )
                                    {
                                        if( !strncmp(pDescriptor->pPayload, "AC-3", strlen("AC-3")) )
                                            audioAc3ChkNum++;
                                    }
                                    break;
                                case 0x59:
                                    if( 0x06 == pCurEsInfo->stream_type && 
                                        pDescriptor->pPayload )
                                    {
                                        uint32_t    descriptorLen = pDescriptor->descriptor_length;
                                        uint32_t    payloadPosition = 0;
            
                                        while( descriptorLen >= 8 )
                                        {
                                            if( subtitleIdx < SERVICE_MAX_SUBTITLE_COUNT )
                                            {
                                                // Composition Page id
                                                pNewTsSrvcInfo->subtitlePID[subtitleIdx] |= (pDescriptor->pPayload[payloadPosition + 4] << 24);
                                                pNewTsSrvcInfo->subtitlePID[subtitleIdx] |= (pDescriptor->pPayload[payloadPosition + 5] << 16);
            
                                                // Subtitle PID
                                                pNewTsSrvcInfo->subtitlePID[subtitleIdx] |= pCurEsInfo->elementary_PID;
            
                                                descriptorLen -= 8;
                                                payloadPosition += 8;
                                                ++subtitleIdx;
                                            }
                                            else
                                                break;
                                        }
                                    }
                                    break;
                            }
            
                            pDescriptor = pDescriptor->ptNextDescriptor;
                        }
            
                        if( audioAc3ChkNum )
                        {
                            if( audioIdx+1 < SERVICE_MAX_AUDIO_COUNT )
                            {
                                pNewTsSrvcInfo->audioPID[audioIdx]  = esPid;
                                pNewTsSrvcInfo->audioType[audioIdx] = TS_SRVC_AUD_AC3;
                                audioIdx++;
            
                                // update pid analysis data to ts parser
                                if( ptDemux->pHTsSrvc && pSrvcDb && pSrvcDb->Pid_Stat_cb )
                                {
                                    TS_PID_ANAL_DATA    pidData = {0};
                                    
                                    pidData.service_order_num = pNewTsSrvcInfo->order_num;
                                    pidData.pid               = esPid;
                                    pidData.bVideo            = false;
                                    
                                    pSrvcDb->Pid_Stat_cb(&pidData, pSrvcDb->pTspStatInfo);
                                }                                        
                            }
                            audioAc3ChkNum = 0;
                        }
                        break;
                } 
            }

            pNewTsSrvcInfo->userInfo.audioCount = audioIdx;
            pNewTsSrvcInfo->userInfo.subtitleCount = subtitleIdx;
            pNewTsSrvcInfo->userInfo.actSubtitleIdx = 0xFFFF;
            
            pNewTsSrvcInfo->programNumber = ptPmtInfo->program_number;
            pNewTsSrvcInfo->pmt_pid       = ptPmtInfo->PID;
        }
        
        // update to ts service database
        if( ptDemux->pHTsSrvc && pSrvcDb && bUpdate_pmt_info == true )
        {
            if( pSrvcDb->curChnlInfo.userInfo.frequency && 
                pSrvcDb->curChnlInfo.userInfo.bandwidth )
            {
                pNewTsSrvcInfo->tsChnlInfo.userInfo.frequency = pSrvcDb->curChnlInfo.userInfo.frequency;
                pNewTsSrvcInfo->tsChnlInfo.userInfo.bandwidth = pSrvcDb->curChnlInfo.userInfo.bandwidth;
            }
            else
                srvc_msg_ex(SRVC_MSG_TYPE_ERR, " loss channel info in Service info Database !!");

            // save new version number
            pSrvcDb->pmt_srvc_info.cur_version_number = ptPmtInfo->version_number;

            _Add_to_Service_List(pSrvcDb, &pSrvcDb->pmt_srvc_info, pNewTsSrvcInfo, true);

            srvc_msg(1, "   pmtPid= 0x%x, program_num=0x%x, freq=%d\n",
                        pNewTsSrvcInfo->pmt_pid, pNewTsSrvcInfo->programNumber,
                        pNewTsSrvcInfo->tsChnlInfo.userInfo.frequency);

            if( pSrvcDb->pmt_srvc_info.srvc_cnt >= pSrvcDb->pat_total_srvc )
            {
                pSrvcDb->hSrvc.bReady_PMT = true;
                srvc_msg(1, "   hSrvc.bReady_PMT = %d\n",  pSrvcDb->hSrvc.bReady_PMT);
            }
        }
        
    }while(0);

    _trace_leave();
    return;
}



void
tsSrvc_SdtCallBack(
    TS_DEMUX*       ptDemux,
    PSI_SDT_INFO*   ptSdtInfo)
{
    TS_SRVC_DB          *pSrvcDb = DOWN_CAST(TS_SRVC_DB, ptDemux->pHTsSrvc, hSrvc);
    bool                bReady_SDT = false;
    TS_SERVICE_INFO     tsSrvcInfo = {0};

    _trace_enter("\n");

    if( ptDemux->pHTsSrvc && pSrvcDb && pSrvcDb->bSkip == true )  
    {
        srvc_msg_ex(ENABLE_WARNING_MSG, " ts service info skip SDT data !!");
        _trace_leave();
        return;
    }

    do{
        TS_SERVICE_INFO     *pNewTsSrvcInfo = &tsSrvcInfo;
        PSI_SDT_SERVICE     *pCurService = 0;
        PSI_DESCRIPTOR      *pDescriptor = 0;
        uint8_t             *pCurAddr = 0;
        uint32_t            i = 0;

        if( ptDemux->pHTsSrvc && pSrvcDb && 
            pSrvcDb->tsdCtrlStatus == TSD_CTRL_NORMAL_MODE &&
            // check version_number
            ptSdtInfo->version_number == pSrvcDb->sdt_srvc_info.cur_version_number  )
            break;

        for( pCurService = ptSdtInfo->ptFirstService;
             pCurService;
             pCurService = pCurService->ptNextService )
        {
            pDescriptor = pCurService->ptFirstDescriptor;
            while( pDescriptor )
            {
                // 0x48 = service description tag
                if( 0x48 == pDescriptor->descriptor_tag )
                {
                    // service type coding
                    // (ETSI EN 300 468 V1.12.1 section 6.2.33)
                    #define DIGITAL_TELEVISION_SERVICE                  0X01
                    #define DIGITAL_RADIO_SOUND_SERVICE                 0X02
                    #define ADVANCED_SD_DIGITAL_TELEVISION_SERVICE      0X16
                    #define ADVANCED_HD_DIGITAL_TELEVISION_SERVICE      0X19

                    uint8_t     service_type = 0xFF;
                    
                    pCurAddr = pDescriptor->pPayload;

                    service_type = pCurAddr[0];
                    
                    pCurAddr += (2 + pCurAddr[1]);
                    pNewTsSrvcInfo->userInfo.nameSize = pCurAddr[0];
                    pCurAddr += 1;

                    if( (service_type == DIGITAL_TELEVISION_SERVICE || 
                         service_type == DIGITAL_RADIO_SOUND_SERVICE ||
                         service_type == ADVANCED_SD_DIGITAL_TELEVISION_SERVICE ||
                         service_type == ADVANCED_HD_DIGITAL_TELEVISION_SERVICE) &&
                        pNewTsSrvcInfo->userInfo.nameSize > 0 && 
                        pNewTsSrvcInfo->userInfo.nameSize < TSD_MAX_SRVC_NAME_SIZE )
                    {
                        memcpy(pNewTsSrvcInfo->userInfo.serviceName,
                               pCurAddr,
                               pNewTsSrvcInfo->userInfo.nameSize);

                        // update to ts service database
                        if( ptDemux->pHTsSrvc && pSrvcDb )
                        {
                            pNewTsSrvcInfo->programNumber                 = pCurService->service_id;
                            pNewTsSrvcInfo->tsChnlInfo.userInfo.frequency = pSrvcDb->curChnlInfo.userInfo.frequency;
                            pNewTsSrvcInfo->tsChnlInfo.userInfo.bandwidth = pSrvcDb->curChnlInfo.userInfo.bandwidth;

                            pSrvcDb->sdt_srvc_info.cur_version_number = ptSdtInfo->version_number;

                            srvc_msg(1, "   sdt_program_num= 0x%x, freq=%d\n", 
                                     pNewTsSrvcInfo->programNumber,
                                     pNewTsSrvcInfo->tsChnlInfo.userInfo.frequency);

                            _Add_to_Service_List(pSrvcDb, &pSrvcDb->sdt_srvc_info, pNewTsSrvcInfo, true);
                        }
                    }
                }
                pDescriptor = pDescriptor->ptNextDescriptor;
            } 
        }

        if( !ptDemux->bReceiveSdt )         ptDemux->bReceiveSdt = 1;

        // update flag to scan_process
        if( ptDemux->pHTsSrvc && pSrvcDb )  pSrvcDb->hSrvc.bReady_SDT = true;

    }while(0);
    
    _trace_leave();
    return;
}
//=============================================================================
//				  Public Function Definition
//=============================================================================
TSD_ERR
tsSrvc_CreateHandle(
    TS_SRVC_HANDLE  **pHSrvc,
    void            *extraData)
{
    TSD_ERR         result = SRVC_ERR_OK;
    TS_SRVC_DB      *pSrvcDb = 0; 

    _trace_enter("\n");
    
    do{
        if( *pHSrvc != 0 )
        {
            srvc_msg_ex(SRVC_MSG_TYPE_ERR, " error, Exist service DB handle !!");
            result = SRVC_ERR_INVALID_PARAMETER;
            break;
        }

        // ------------------------
        // craete servic database handle
        pSrvcDb = tsd_malloc(sizeof(TS_SRVC_DB));
        if( !pSrvcDb )
        {
            srvc_msg_ex(SRVC_MSG_TYPE_ERR, " error, allocate fail !!");
            result = SRVC_ERR_ALLOCATE_FAIL;
            break;
        }

        memset((void*)pSrvcDb, 0x0, sizeof(TS_SRVC_DB));
        pSrvcDb->srvcStatus = SRVC_STATUS_IDLE;
        
        //--------------------------------
        // initial variable
        pSrvcDb->pat_version_number               = (uint32_t)-1;
        pSrvcDb->pmt_srvc_info.cur_version_number = (uint32_t)-1;
        pSrvcDb->sdt_srvc_info.cur_version_number = (uint32_t)-1;
        
        // create mutex
        _mutex_init(TSD_MSG_TYPE_TRACE_SRVC, pSrvcDb->psi_collect_mutex);
        
        // if not error
        (*pHSrvc) = &pSrvcDb->hSrvc;
        
    }while(0);
    
    if( result != SRVC_ERR_OK )
    {
        pSrvcDb->srvcStatus = SRVC_STATUS_FAIL;
        srvc_msg_ex(SRVC_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    
    _trace_leave();
    return result;
}

TSD_ERR
tsSrvc_DestroyHandle(
    TS_SRVC_HANDLE  **pHSrvc)
{
    TSD_ERR         result = SRVC_ERR_OK;
    TS_SRVC_DB      *pSrvcDb = DOWN_CAST(TS_SRVC_DB, (*pHSrvc), hSrvc);
    pthread_mutex_t psi_collect_mutex = 0;

    _trace_enter("0x%x\n", pHSrvc);    
    
    _verify_handle((*pHSrvc), result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_SRVC, pSrvcDb->psi_collect_mutex);

    if( pSrvcDb )
    {
        _Del_Service_List(pSrvcDb, &pSrvcDb->act_srvc_info);
        _Del_Service_List(pSrvcDb, &pSrvcDb->pmt_srvc_info);
        _Del_Service_List(pSrvcDb, &pSrvcDb->sdt_srvc_info);

        *pHSrvc = 0;

        psi_collect_mutex = pSrvcDb->psi_collect_mutex;

        // free handle
        free(pSrvcDb);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_SRVC, psi_collect_mutex);
    // de-init mutex
    _mutex_deinit(TSD_MSG_TYPE_TRACE_SRVC, psi_collect_mutex);
    _trace_leave();
    return result;
}

TSD_ERR
tsSrvc_AddService(
    TS_SRVC_HANDLE  *pHSrvc,
    TS_SERVICE_INFO *pInsSrvcInfo,
    uint32_t        *curSrvcIdx,
    void            *extraData)
{
    TSD_ERR         result = SRVC_ERR_OK;
    TS_SRVC_DB      *pSrvcDb = DOWN_CAST(TS_SRVC_DB, pHSrvc, hSrvc);

    _trace_enter("0x%x, 0x%x, 0x%x, 0x%x\n", pHSrvc, pInsSrvcInfo, curSrvcIdx, extraData);

    _verify_handle(pHSrvc, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_SRVC, pSrvcDb->psi_collect_mutex);
    
    if( pSrvcDb && pInsSrvcInfo && pSrvcDb->srvcStatus != SRVC_STATUS_FAIL )
    {
        do{
            _Add_to_Service_List(pSrvcDb, &pSrvcDb->act_srvc_info, pInsSrvcInfo, true);
            
            pSrvcDb->hSrvc.pStartSrvcInfo = pSrvcDb->act_srvc_info.pFirstSrvc;
            pSrvcDb->hSrvc.pCurSrvcInfo   = pSrvcDb->act_srvc_info.pCurSrvc;
            pSrvcDb->hSrvc.totalSrvc      = pSrvcDb->act_srvc_info.srvc_cnt;

            if( curSrvcIdx )    (*curSrvcIdx) = pSrvcDb->act_srvc_info.srvc_cnt;
            
        }while(0);
        
    }
    
    if( result != SRVC_ERR_OK )
    {
        pSrvcDb->srvcStatus = SRVC_STATUS_FAIL;
        srvc_msg_ex(SRVC_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_SRVC, pSrvcDb->psi_collect_mutex);
    _trace_leave();
    return result;

}

TSD_ERR
tsSrvc_GetServiceInfo(
    TS_SRVC_HANDLE      *pHSrvc,
    uint32_t            orderIdx,
    TS_SERVICE_INFO     *pSrvc_info_start,
    TS_SERVICE_INFO     **pSrvc_info,
    void                *extraData)
{
    TSD_ERR         result = SRVC_ERR_OK;
    TS_SRVC_DB      *pSrvcDb = DOWN_CAST(TS_SRVC_DB, pHSrvc, hSrvc);

    _trace_enter("0x%x, %d, 0x%x, 0x%x, 0x%x\n", pHSrvc, orderIdx, pSrvc_info_start, pSrvc_info, extraData);

    _verify_handle(pHSrvc, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_SRVC, pSrvcDb->psi_collect_mutex);

    if( pSrvcDb && pSrvcDb->srvcStatus != SRVC_STATUS_FAIL )
    {        
        // feedback psi table status        
        if( !pSrvc_info_start )
            *pSrvc_info = LIST_FIND(TS_SERVICE_INFO, _Search_Func, 
                                    SRVC_FIND_CMP_OREDER_NUM, pSrvcDb->hSrvc.pCurSrvcInfo, orderIdx);
        else
            *pSrvc_info = LIST_FIND(TS_SERVICE_INFO, _Search_Func, 
                                    SRVC_FIND_CMP_OREDER_NUM, pSrvc_info_start, orderIdx);

        #if 0
        {
            // dump
            FILE *fout = 0;
            if( fout = fopen("./serviceInfo.txt", "w") )
            {
                uint32_t         i;
                TS_SERVICE_INFO  *tmp_srvc_info = 0;
                for(i = 0; i < g_info.serviceCnt; i++)
                {
                    tmp_srvc_info = LIST_FIND(TS_SERVICE_INFO, _Search_Func, 
                                              SRVC_FIND_CMP_OREDER_NUM, g_info.pCurSrvcInfo, i);

                    fprintf(fout, "%02d-th: prgNum = %d, vPid= 0x%x, freq=%d, bw= %d\n", 
                        i, tmp_srvc_info->programNumber, 
                        tmp_srvc_info->videoPID, 
                        tmp_srvc_info->tsChnlInfo.frequency, 
                        tmp_srvc_info->tsChnlInfo.bandwidth);
                }
                fclose(fout);
            }
            else
                _err("open fail !!");
        }
        #endif
    }
    
    if( result != SRVC_ERR_OK )
    {
        pSrvcDb->srvcStatus = SRVC_STATUS_FAIL;
        srvc_msg_ex(SRVC_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSD_MSG_TYPE_TRACE_SRVC, pSrvcDb->psi_collect_mutex);
    _trace_leave();
    return result;
}


TSD_ERR
tsSrvc_Control(
    TS_SRVC_HANDLE      *pHSrvc,
    TS_SRVC_CTL_CMD     cmd,
    uint32_t            *value,
    void                *extraData)
{
    TSD_ERR         result = SRVC_ERR_OK;
    TS_SRVC_DB      *pSrvcDb = DOWN_CAST(TS_SRVC_DB, pHSrvc, hSrvc);

    _trace_enter("0x%x, %d, 0x%x, 0x%x\n", pHSrvc, cmd, value, extraData);

    _verify_handle(pHSrvc, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_SRVC, pSrvcDb->psi_collect_mutex);
    
    if( pSrvcDb && pSrvcDb->srvcStatus != SRVC_STATUS_FAIL )
    {
        switch( cmd )
        {                
            case TS_SRVC_CTL_SKIP_PSI_INFO:
                pSrvcDb->bSkip = (value) ? true : false;
                break;
            
            case TS_SRVC_CTL_SET_PID_STAT_CB_INFO:
                if( extraData )     
                {
                    TS_SRVC_PID_STAT_CB_INFO    *pCbInfo = (TS_SRVC_PID_STAT_CB_INFO*)extraData;

                    pSrvcDb->pTspStatInfo = pCbInfo->pStatInfo;
                    pSrvcDb->Pid_Stat_cb  = pCbInfo->pfStat_CB;
                }
                break;
                
            case TS_SRVC_CTL_SET_CHANNEL_INFO:
                if( extraData )     memcpy(&pSrvcDb->curChnlInfo, extraData, sizeof(TS_CHNL_INFO));
                break;

            case TS_SRVC_CTL_GET_SRVC_CNT_IN_CHNL:
                if( extraData )
                {
                    TS_CHNL_INFO        *pCurTsChnlInfo = (TS_CHNL_INFO*)extraData;
                    TS_SERVICE_INFO     *pCurSrvc = 0;
                    TS_SERVICE_INFO     *pTmpSrvc = 0;
                    uint32_t            srvc_cnt = 0;
                    uint32_t            i;

                    pCurSrvc = pSrvcDb->act_srvc_info.pFirstSrvc;
                    for(i = 0; i < pSrvcDb->act_srvc_info.srvc_cnt; i++)
                    {
                        pTmpSrvc = LIST_FIND(TS_SERVICE_INFO, _Search_Func, 
                                             SRVC_FIND_CMP_OREDER_NUM, 
                                             pCurSrvc, i);
                        if( pTmpSrvc && 
                            pTmpSrvc->tsChnlInfo.userInfo.frequency == pCurTsChnlInfo->userInfo.frequency)  
                        {
                            srvc_cnt++;
                        }
                    }
                    
                    if( value )     *value = srvc_cnt;
                }
                break;

            case TS_SRVC_CTL_GET_TOTAL_SRVC:
                if( value )     *value = pSrvcDb->act_srvc_info.srvc_cnt;
                break;

            case TS_SRVC_CTL_DEL_ALL_SRVC_INFO:
                _Del_Service_List(pSrvcDb, &pSrvcDb->act_srvc_info);
                _Del_Service_List(pSrvcDb, &pSrvcDb->pmt_srvc_info);
                _Del_Service_List(pSrvcDb, &pSrvcDb->sdt_srvc_info);

                //--------------------------------
                // initial variable
                memset(&pSrvcDb->hSrvc, 0x0, sizeof(TS_SRVC_HANDLE));
                
                pSrvcDb->pat_total_srvc     = 0;
                pSrvcDb->pat_version_number = (uint32_t)-1;
                
                memset(&pSrvcDb->curChnlInfo, 0x0, sizeof(TS_CHNL_INFO));
                break;

            case TS_SRVC_CTL_MERGE_SRVC_INFO:
                _Merge_Service_List(pSrvcDb, (TS_SRVC_DB*)extraData, (TS_SRVC_MERGE_TYPE)value);
                break;
                
            case TS_SRVC_CTL_RESET_PAT_STATUS:
                pSrvcDb->hSrvc.bReady_PAT = false;
                pSrvcDb->pat_total_srvc   = 0;
                pSrvcDb->pat_version_number               = (uint32_t)-1;
                pSrvcDb->hSrvc.bReady_PMT = false;
                pSrvcDb->pmt_srvc_info.cur_version_number = (uint32_t)-1;
                break;
                
            case TS_SRVC_CTL_RESET_SDT_STATUS:
                pSrvcDb->hSrvc.bReady_SDT = false;
                pSrvcDb->sdt_srvc_info.cur_version_number = (uint32_t)-1;
                break;

            case TS_SRVC_CTL_SET_CTL_STATUS:
                pSrvcDb->tsdCtrlStatus = (TSD_CTRL_STATUS)value;
                break;
                
            default:
                result = SRVC_ERR_NO_IMPLEMENT;
                break;
        }
    }
    
    if( result != SRVC_ERR_OK &&
        result != SRVC_ERR_NO_IMPLEMENT )
    {
        pSrvcDb->srvcStatus = SRVC_STATUS_FAIL;
        srvc_msg_ex(SRVC_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    } 
    
    _mutex_unlock(TSD_MSG_TYPE_TRACE_SRVC, pSrvcDb->psi_collect_mutex);
    _trace_leave();
    return result;
}
