

#include <stdint.h>
#include <stdbool.h>

#include "ts_extract.h"
#include "ts_split.h"
#include "ts_packet_analysis.h"
#include "ts_packet_demux.h"
//=============================================================================
//                Constant Definition
//=============================================================================
#define TSEXT_VALID_SYNC_BYTE               (0x47)
#define TSEXT_TS_PACKET_SIZE                (188)

#define CA_DESCRIPTOR                       (0x9)

/**
 *
 **/
typedef enum _TSEXT_STATUS_TAG
{
    TSEXT_STATUS_IDLE = 0x11,
    TSEXT_STATUS_BUSY = 0xBB,
    TSEXT_STATUS_FAIL = 0xFF,

}TSEXT_STATUS;

/**
 *
 **/
typedef enum TSEXT_STREAM_STATE_T
{
    TSEXT_STREAM_STATE_SEARCH_PKT_START  = 0x00,
    TSEXT_STREAM_STATE_LESS_PKT_SIZE     = 0x22,
}TSEXT_STREAM_STATE;
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * ts extract device info
 **/
typedef struct TSEXT_DEV_T
{
    TSEXT_HANDLE            hTsExt;

    TSEXT_PKT_SPLIT_LEVEL   pkt_split_level;

    // ts split handle
    TSS_HANDLE              *pHTss;

    TSEXT_STATUS            status;

    // packet parsing
    uint32_t                act_pkt_size;   // action packet size from argge module
    TSEXT_STREAM_STATE      stream_state;
    uint32_t                collectedByte;
    uint8_t                 *pIncompletePktCache;

    // pid info
    TSEXT_PKT_ANAL_INFO     pkt_anal_info[TSEXT_MAX_PKT_ANALYZER_NUM];

    // for less memory
    bool                    bSpareMem;

}TSEXT_DEV;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
static uint32_t
_tsext_Tspa_pat_recv(
    TSPA_USER_MBOX_ARG  *pUser_mbox_arg,
    void                *extraData)
{
    uint32_t        result = 0;

    do{
        TSEXT_PKT_ANAL_INFO     *pCur_pkt_anal_info = 0;
        TS_PACKET_ANAL          *pHTspa = 0;
        PSI_PAT_INFO            *pPat_Info = 0;
        uint32_t                totalSrvcCnt = 0;
        uint32_t                i = 0;

        if( !pUser_mbox_arg )
        {
            tsext_msg_ex(TSEXT_MSG_ERR, "err, Null arg !!");
            break;
        }

        pCur_pkt_anal_info = (TSEXT_PKT_ANAL_INFO*)pUser_mbox_arg->arg.pat.pTunnelInfo;
        pHTspa             = (TS_PACKET_ANAL*)pCur_pkt_anal_info->pHTspa;
        pPat_Info          = (PSI_PAT_INFO*)pUser_mbox_arg->arg.pat.pPat_info;

        pHTspa->bReceivePat = true;

        if( !pCur_pkt_anal_info || !pHTspa || !pPat_Info ||
            pPat_Info->version_number == pCur_pkt_anal_info->pat_version_number )
            break;

        if( !pHTspa->bWaitNitReady || pHTspa->bReceiveSdt )
        {
            PSI_PAT_PROGRAM  *pCurProgram = 0;

            for( pCurProgram = pPat_Info->pFirstProgram;
                 pCurProgram != 0;
                 pCurProgram = pCurProgram->pNextProgram )
            {
                if( pCurProgram->program_number )   totalSrvcCnt++;
            }
        }

        pCur_pkt_anal_info->pat_version_number = pPat_Info->version_number;
        pCur_pkt_anal_info->total_service      = totalSrvcCnt;
        pCur_pkt_anal_info->bPat_ready         = true;

        if( pCur_pkt_anal_info->pSrvc_pid_info )
            free(pCur_pkt_anal_info->pSrvc_pid_info);

        pCur_pkt_anal_info->pSrvc_pid_info =
                tsext_malloc(pCur_pkt_anal_info->total_service * sizeof(TSEXT_SRVC_PID_INFO));

        if( pCur_pkt_anal_info->pSrvc_pid_info )
            memset(pCur_pkt_anal_info->pSrvc_pid_info, 0x0, pCur_pkt_anal_info->total_service * sizeof(TSEXT_SRVC_PID_INFO));

        for(i = 0; i < pCur_pkt_anal_info->total_service; i++)
            pCur_pkt_anal_info->pSrvc_pid_info[i].version_number = -1;

        tsext_msg(1, "    PAT get total service %d \n", totalSrvcCnt);

    }while(0);

    return result;
}

static uint32_t
_tsext_Tspa_pmt_recv(
    TSPA_USER_MBOX_ARG  *pUser_mbox_arg,
    void                *extraData)
{
    uint32_t                result = 0;
    // static USER_PID_INFO    userPidInfo[100] = {0};
    static uint32_t         cur_index = 0;

    do{
        TSEXT_PKT_ANAL_INFO     *pCur_pkt_anal_info = 0;
        TS_PACKET_ANAL          *pHTspa = 0;
        PSI_PMT_INFO            *pPmt_Info = 0;
        PSI_DESCR               *ptDescriptor = 0;
        TSEXT_SRVC_PID_INFO     *pCur_srvc_pid_info = 0;
        uint32_t                j = 0, i = 0, audioIdx = 0;

        if( !pUser_mbox_arg )
        {
            tsext_msg_ex(TSEXT_MSG_ERR, "err, Null arg !!");
            break;
        }

        pCur_pkt_anal_info = (TSEXT_PKT_ANAL_INFO*)pUser_mbox_arg->arg.pmt.pTunnelInfo;
        pHTspa             = (TS_PACKET_ANAL*)pCur_pkt_anal_info->pHTspa;
        pPmt_Info          = (PSI_PMT_INFO*)pUser_mbox_arg->arg.pmt.pPmt_info;

        if( !pCur_pkt_anal_info || !pHTspa || !pPmt_Info ||
            pCur_pkt_anal_info->used_srvc_pid_info_idx >= pCur_pkt_anal_info->total_service )
            break;

        // analysis PMT
        if( pPmt_Info->pid != PAT_PID && pPmt_Info->pid != NIT_PID
         && pPmt_Info->pid != SDT_PID && pPmt_Info->pid != EIT_PID
         && pPmt_Info->pid != TDT_TOT_PID )
        {
            PSI_PMT_ES_INFO         *pCurEsInfo = 0;
            PSI_DESCR               *pDescriptor = 0;
            bool                    bLoop_break = false;

            for(j = 0; j < pCur_pkt_anal_info->used_srvc_pid_info_idx; j++)
            {
                if( pCur_pkt_anal_info->pSrvc_pid_info &&
                    pCur_pkt_anal_info->pSrvc_pid_info[j].programNumber == pPmt_Info->program_number )
                {
                    bLoop_break = true;
                    break;
                }
            }

            if( bLoop_break == true &&
                pPmt_Info->version_number == pCur_pkt_anal_info->pSrvc_pid_info[j].version_number )
                break;

            // skip CA system
            for(ptDescriptor = pPmt_Info->ptFirstDescriptor;
                ptDescriptor;
                ptDescriptor = ptDescriptor->ptNextDescriptor )
            {
                if( CA_DESCRIPTOR == ptDescriptor->descriptor_tag )
                    return result;
            }

            pCur_srvc_pid_info = &pCur_pkt_anal_info->pSrvc_pid_info[pCur_pkt_anal_info->used_srvc_pid_info_idx];

            memset(pCur_srvc_pid_info, 0x0, sizeof(TSEXT_SRVC_PID_INFO));

            for(pCurEsInfo = pPmt_Info->ptFirstEsInfo;
                pCurEsInfo != 0;
                pCurEsInfo = pCurEsInfo->ptNexEsInfo )
            {
                uint32_t    esPid = pCurEsInfo->elementary_PID;
                uint32_t    audioAc3ChkNum = 0;

                switch( pCurEsInfo->stream_type )
                {
                case ITE_CCHDTV_DATA_CHANNEL:
                case ITE_DTV_CAM_DEV_INFO:
                    // ccHDTv
                    // To Do:
                    break;

                case ISO_IEC_11172_2_VIDEO:   // mpeg1 video
                case ISO_IEC_13818_2_VIDEO:   // mpeg2 video
                case ISO_IEC_14496_10_VIDEO:  // avc (h.264) video
                    pCur_srvc_pid_info->videoPID  = esPid;
                    pCur_srvc_pid_info->videoType = pCurEsInfo->stream_type;
                    break;

                case ISO_IEC_11172_3_AUDIO:  // mpeg1 audio
                case ISO_IEC_13818_3_AUDIO:  // mpeg2 audio
                    if( audioIdx+1 < TSEXT_SRVC_MAX_AUD_COUNT )
                    {
                        pCur_srvc_pid_info->audioPID[audioIdx] = esPid;
                        audioIdx++;
                    }
                    break;

                case ISO_IEC_13818_7_AUDIO:
                case ISO_IEC_14496_3_AUDIO:
                    // aac latm,adts audio
                    if( audioIdx+1 < TSEXT_SRVC_MAX_AUD_COUNT )
                    {
                        pCur_srvc_pid_info->audioPID[audioIdx] = (uint16_t)pCurEsInfo->elementary_PID;
                        audioIdx++;
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
                                uint32_t        format_identifier = 0;
                                format_identifier = *((uint32_t*) pDescriptor->pPayload);
                                if( format_identifier == 0x41432D33 )
                                    audioAc3ChkNum++;
                            }
                            break;
                        case 0x59:
                            if( 0x06 == pCurEsInfo->stream_type &&
                                pDescriptor->pPayload )
                            {
                                // skip subtitle
                            }
                            break;
                        }

                        pDescriptor = pDescriptor->ptNextDescriptor;
                    }

                    if( audioAc3ChkNum )
                    {
                        if( audioIdx+1 < TSEXT_SRVC_MAX_AUD_COUNT )
                        {
                            pCur_srvc_pid_info->audioPID[audioIdx] = esPid;
                            audioIdx++;
                        }
                        audioAc3ChkNum = 0;
                    }
                    break;
                }
            }

            pCur_srvc_pid_info->audioCount    = audioIdx;
            pCur_srvc_pid_info->pmt_pid       = pPmt_Info->pid;
            pCur_srvc_pid_info->programNumber = pPmt_Info->program_number;
        }

        pCur_srvc_pid_info->version_number = pPmt_Info->version_number;

        tsext_msg(1, "  pmtPid= 0x%x, program_num=0x%x\n",
                    pCur_srvc_pid_info->pmt_pid,
                    pCur_srvc_pid_info->programNumber);
        tsext_msg(1, "    vid= 0x%x\n", pCur_srvc_pid_info->videoPID);
        for(j = 0; j < pCur_srvc_pid_info->audioCount; j++)
            tsext_msg(1, "    aid= 0x%x\n", pCur_srvc_pid_info->audioPID[j]);

        pCur_pkt_anal_info->used_srvc_pid_info_idx++;

        // ------------------------------
        // statistics the weighting of this service
        for(j = 0; j < pCur_pkt_anal_info->used_srvc_pid_info_idx; j++)
        {
            TSPA_PID                *pPid_handler = 0;

            pCur_srvc_pid_info = &pCur_pkt_anal_info->pSrvc_pid_info[j];
            pCur_srvc_pid_info->proportion = 0;
            if( pCur_srvc_pid_info->videoPID )
            {
                pPid_handler = pHTspa->pTsPid_handler[pCur_srvc_pid_info->videoPID];
                pCur_srvc_pid_info->proportion = pPid_handler->proportion;
            }

            for(i = 0; i < pCur_srvc_pid_info->audioCount; i++)
            {
                if( pCur_srvc_pid_info->audioPID[i] )
                {
                    pPid_handler = pHTspa->pTsPid_handler[pCur_srvc_pid_info->audioPID[i]];
                    pCur_srvc_pid_info->proportion += pPid_handler->proportion;
                }
            }
        }

        //---------------------------------------
        if( pCur_pkt_anal_info->used_srvc_pid_info_idx == pCur_pkt_anal_info->total_service )
            pCur_pkt_anal_info->bPmt_ready = true;

    }while(0);

    return result;
}

static uint32_t
_tsext_Tspa_sdt_recv(
    TSPA_USER_MBOX_ARG  *pUser_mbox_arg,
    void                *extraData)
{
    uint32_t        result = 0;

    do{
        TSEXT_PKT_ANAL_INFO     *pCur_pkt_anal_info = 0;
        TS_PACKET_ANAL          *pHTspa = 0;
        PSI_SDT_INFO            *pSdt_Info = 0;
        PSI_SDT_SERVICE         *pCurService = 0;
        PSI_DESCR               *pDescriptor = 0;
        uint8_t                 *pCurAddr = 0;
        static uint32_t         cur_version_number = -1;

        if( !pUser_mbox_arg )
        {
            tsext_msg_ex(TSEXT_MSG_ERR, "err, Null arg !!");
            break;
        }

        pCur_pkt_anal_info = (TSEXT_PKT_ANAL_INFO*)pUser_mbox_arg->arg.sdt.pTunnelInfo;
        pHTspa             = (TS_PACKET_ANAL*)pCur_pkt_anal_info->pHTspa;
        pSdt_Info          = (PSI_SDT_INFO*)pUser_mbox_arg->arg.sdt.pSdt_info;

        if( !pCur_pkt_anal_info || !pHTspa || !pSdt_Info ||
            pSdt_Info->version_number == pCur_pkt_anal_info->sdt_version_number )
            break;

        for( pCurService = pSdt_Info->ptFirstService;
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

                    uint32_t    nameSize = 0;
                    uint8_t     service_type = 0xFF;

                    pCurAddr = pDescriptor->pPayload;

                    service_type = pCurAddr[0];

                    pCurAddr += (2 + pCurAddr[1]);
                    nameSize = pCurAddr[0];
                    pCurAddr += 1;

                    if( (service_type == DIGITAL_TELEVISION_SERVICE ||
                         service_type == DIGITAL_RADIO_SOUND_SERVICE ||
                         service_type == ADVANCED_SD_DIGITAL_TELEVISION_SERVICE ||
                         service_type == ADVANCED_HD_DIGITAL_TELEVISION_SERVICE) &&
                         nameSize > 0 &&
                         nameSize < 256 )
                    {
                        // _show_srvc_txt(pCurAddr, nameSize);
                        tsext_msg(1, "   sdt_program_num= 0x%x\n", pCurService->service_id);
                        pCur_pkt_anal_info->sdt_version_number = pSdt_Info->version_number;
                    }
                }
                pDescriptor = pDescriptor->ptNextDescriptor;
            }
        }

        if( pHTspa->bReceiveSdt == false )
            pHTspa->bReceiveSdt = true;

        pCur_pkt_anal_info->bSdt_ready = true;

    }while(0);

    return result;
}

static uint32_t
_tsext_pkt_analyzer_create(
    TSEXT_DEV       *pTsExtDev,
    TSS_MBOX_ARG    *pMbox_arg,
    void            *extraData)
{
    uint32_t                tspa_idx = 0;
    TSEXT_PKT_ANAL_INFO     *pCur_pkt_anal_info = 0;
    TS_PACKET_ANAL          *pHTspa = 0;
    TSPA_SETUP_PARAM        setupParam = {0};

    setupParam.bInScan      = true;
    setupParam.bWaitNit     = false;

    tspa_idx = pMbox_arg->arg.user.index;

    pCur_pkt_anal_info = &pTsExtDev->pkt_anal_info[tspa_idx];

    // create tspa handle
    tspa_CreateHandle((TS_PACKET_ANAL**)(&pCur_pkt_anal_info->pHTspa), &setupParam, 0);

    pCur_pkt_anal_info->used_srvc_pid_info_idx = 0;
    pCur_pkt_anal_info->bPat_ready = false;
    pCur_pkt_anal_info->bSdt_ready = false;
    pCur_pkt_anal_info->bPmt_ready = false;
    pCur_pkt_anal_info->pat_version_number = -1;
    pCur_pkt_anal_info->sdt_version_number = -1;

    pHTspa = (TS_PACKET_ANAL*)pCur_pkt_anal_info->pHTspa;

    pHTspa->pat_user_mbox.func                                   = _tsext_Tspa_pat_recv;
    pHTspa->pat_user_mbox.tspa_user_mbox_arg.type                = TSPA_USER_MBOX_TYPE_PAT;
    pHTspa->pat_user_mbox.tspa_user_mbox_arg.arg.pat.pTunnelInfo = (void*)pCur_pkt_anal_info; //pHTspa;

    pHTspa->pmt_user_mbox.func                                   = _tsext_Tspa_pmt_recv;
    pHTspa->pmt_user_mbox.tspa_user_mbox_arg.type                = TSPA_USER_MBOX_TYPE_PMT;
    pHTspa->pmt_user_mbox.tspa_user_mbox_arg.arg.pmt.pTunnelInfo = (void*)pCur_pkt_anal_info; //pHTspa;

    pHTspa->sdt_user_mbox.func                                   = _tsext_Tspa_sdt_recv;
    pHTspa->sdt_user_mbox.tspa_user_mbox_arg.type                = TSPA_USER_MBOX_TYPE_SDT;
    pHTspa->sdt_user_mbox.tspa_user_mbox_arg.arg.sdt.pTunnelInfo = (void*)pCur_pkt_anal_info; //pHTspa;

    return 0;
}

static uint32_t
_tsext_pkt_analyzer_destroy(
    TSEXT_DEV       *pTsExtDev,
    TSS_MBOX_ARG    *pMbox_arg,
    void            *extraData)
{
    uint32_t                tspa_idx = 0;
    TSEXT_HANDLE            *pHTsExt = &pTsExtDev->hTsExt;
    TSEXT_PKT_ANAL_INFO     *pCur_pkt_anal_info = 0;

    tspa_idx = pMbox_arg->arg.user.index;

    pCur_pkt_anal_info = &pTsExtDev->pkt_anal_info[tspa_idx];

    tspa_DestroyHandle((TS_PACKET_ANAL**)(&pCur_pkt_anal_info->pHTspa), 0);

    pCur_pkt_anal_info->proc_mode = TSEXT_PKT_PROC_IDLE;

    //---------------------------------------------
    // scan ready and callback to high level application program
    if( pHTsExt->scan_state_recv.func )
    {
        TSE_USER_ARG     *pTse_user_arg = &pHTsExt->scan_state_recv.tse_user_arg;

        pTse_user_arg->arg.scan.port_index = tspa_idx;
        pHTsExt->scan_state_recv.func(pTse_user_arg, 0);
    }

    return 0;
}

static uint32_t
_tsext_pkt_analyzer_proc(
    TSEXT_DEV       *pTsExtDev,
    TSS_MBOX_ARG    *pMbox_arg,
    void            *extraData)
{
    uint32_t                tspa_idx = 0;
    TSEXT_PKT_ANAL_INFO     *pCur_pkt_anal_info = 0;
    uint8_t                 *pPacketBuf = 0;
    TS_PACKET_ANAL          *pHTspa = 0;

    do{
        tspa_idx = pMbox_arg->arg.user.index;

        pCur_pkt_anal_info = &pTsExtDev->pkt_anal_info[tspa_idx];
        pHTspa             = (TS_PACKET_ANAL*)pCur_pkt_anal_info->pHTspa;

        pPacketBuf = pMbox_arg->arg.user.pBuf_addr;

        if( pCur_pkt_anal_info->pHTspa )
            tspa_Analysis(pHTspa, pPacketBuf, 0);

        if( pCur_pkt_anal_info->bPat_ready &&
            pCur_pkt_anal_info->bSdt_ready &&
            pCur_pkt_anal_info->bPmt_ready )
        {
            TSEXT_HANDLE    *pHTsExt = &pTsExtDev->hTsExt;

#if (TSPA_ENABLE_PES_DECODE)
            if( pHTspa->bGetPesVideo == false )
                break;
#endif
            pHTsExt->pPkt_analyzer_info[tspa_idx] = pCur_pkt_anal_info;

            pMbox_arg->arg.user.feedback_cmd = TSS_CMD_USER_OPR_DESTROY;
        }
    }while(0);

    return 0;
}


static uint32_t
_tsext_pkt_demux_get_user_info(
    TSPD_USER_MBOX_ARG  *pUser_mbox_arg,
    void                *extraData)
{
    if( pUser_mbox_arg )
    {
        uint32_t              i = 0, j = 0;
        TSEXT_PKT_ANAL_INFO   *pCur_pkt_anal_info = (TSEXT_PKT_ANAL_INFO*)pUser_mbox_arg->arg.set_pid.pTunnelInfo;
        TSEXT_SRVC_PID_INFO   *pSrvc_pid_info = pCur_pkt_anal_info->pSrvc_pid_info;
        TSPD_SRVC_ATTR        *pSrvc_attr = (TSPD_SRVC_ATTR*)pUser_mbox_arg->arg.set_pid.pSrvc_attr;
        uint32_t              total_service = pUser_mbox_arg->arg.set_pid.total_service;
        bool                  bDef_sort = true;

        if( pCur_pkt_anal_info->get_info.func )
        {
            TSE_USER_ARG     *pTse_arg = &pCur_pkt_anal_info->get_info.tse_user_arg;

            // get resolution from ap
            for(i = 0; i < total_service; i++)
            {
                pTse_arg->type                     = TSE_USER_ARG_TYPE_GET_VIDEO_INFO;
                pTse_arg->arg.v_info.port_index    = pCur_pkt_anal_info->port_index;
                pTse_arg->arg.v_info.v_pid         = pSrvc_pid_info[i].videoPID;
                pTse_arg->arg.v_info.width         = 0;
                pTse_arg->arg.v_info.height        = 0;
                pTse_arg->arg.v_info.srvc_buf_size = 0;
                pCur_pkt_anal_info->get_info.func(pTse_arg, extraData);

                pSrvc_pid_info[i].ring_buf_size = pTse_arg->arg.v_info.srvc_buf_size;

                pSrvc_pid_info[i].width  = pTse_arg->arg.v_info.width;
                pSrvc_pid_info[i].height = pTse_arg->arg.v_info.height;

                if( !pSrvc_pid_info[i].width || !pSrvc_pid_info[i].height )
                {
                    bDef_sort = true;
                    break;
                }
                else
                    bDef_sort = false;
            }
        }

        // ------------------------
        // sort (decreasing)
        if( bDef_sort == true )
        {
            for(i = 0; i < total_service; i++)
                for(j = i; j < total_service; j++)
                {
                    if( pSrvc_pid_info[j].proportion > pSrvc_pid_info[i].proportion )
                    {
                        TSEXT_SRVC_PID_INFO   tmp_srvc_pid_info = {0};

                        memcpy(&tmp_srvc_pid_info, &pSrvc_pid_info[j], sizeof(TSEXT_SRVC_PID_INFO));
                        memcpy(&pSrvc_pid_info[j], &pSrvc_pid_info[i], sizeof(TSEXT_SRVC_PID_INFO));
                        memcpy(&pSrvc_pid_info[i], &tmp_srvc_pid_info, sizeof(TSEXT_SRVC_PID_INFO));
                    }
                }

            //for(i = 0; i < total_service; i++)
            //    printf(" srvc_proportion = %8d\n", pSrvc_pid_info[order[i]].proportion);
        }
        else
        {
            // sort with service resolution
            for(i = 0; i < total_service; i++)
                for(j = i; j < total_service; j++)
                {
                    if( (pSrvc_pid_info[j].width * pSrvc_pid_info[j].height) >
                        (pSrvc_pid_info[i].width * pSrvc_pid_info[i].height) )
                    {
                        TSEXT_SRVC_PID_INFO   tmp_srvc_pid_info = {0};

                        memcpy(&tmp_srvc_pid_info, &pSrvc_pid_info[j], sizeof(TSEXT_SRVC_PID_INFO));
                        memcpy(&pSrvc_pid_info[j], &pSrvc_pid_info[i], sizeof(TSEXT_SRVC_PID_INFO));
                        memcpy(&pSrvc_pid_info[i], &tmp_srvc_pid_info, sizeof(TSEXT_SRVC_PID_INFO));
                    }
                }

            for(i = 0; i < total_service; i++)
                printf(" srvc %d-th: vid=0x%x, %dx%d, buf_size=%d\n",
                    i, pSrvc_pid_info[i].videoPID, pSrvc_pid_info[i].width, pSrvc_pid_info[i].height, pSrvc_pid_info[i].ring_buf_size);
        }

        // assign service buffer size
        for(i = 0; i < total_service; i++)
            pSrvc_attr[i].srvc_cache_buf_size = pSrvc_pid_info[i].ring_buf_size;
    }

    return 0;
}

static uint32_t
_tsext_pkt_demux_set_srvc_pid(
    TSPD_USER_MBOX_ARG  *pUser_mbox_arg,
    void                *extraData)
{
    if( pUser_mbox_arg )
    {
        TSEXT_PKT_ANAL_INFO   *pCur_pkt_anal_info = (TSEXT_PKT_ANAL_INFO*)pUser_mbox_arg->arg.set_pid.pTunnelInfo;
        TSEXT_SRVC_PID_INFO   *pSrvc_pid_info = pCur_pkt_anal_info->pSrvc_pid_info;
        TSPD_SRVC_ATTR        *pSrvc_attr = (TSPD_SRVC_ATTR*)pUser_mbox_arg->arg.set_pid.pSrvc_attr;
        uint32_t              total_service = pUser_mbox_arg->arg.set_pid.total_service;
        uint32_t              i = 0, j = 0;

        //-------------------------------
        // set PID info
        for(i = 0; i < total_service; i++)
        {
            pSrvc_attr[i].pat_pid     = PAT_PID;
            pSrvc_attr[i].sdt_pid     = SDT_PID;
            pSrvc_attr[i].nit_pid     = NIT_PID;
            pSrvc_attr[i].tdt_tot_pid = TDT_TOT_PID;
            pSrvc_attr[i].pmt_pid     = pSrvc_pid_info[i].pmt_pid;
            pSrvc_attr[i].video_pid   = pSrvc_pid_info[i].videoPID;

            for(j = 0; j < TSPD_SRVC_MAX_AUD_NUM; j++)
                pSrvc_attr[i].audio_pid[j] = pSrvc_pid_info[i].audioPID[j];

            // return ring_buf_info for gateway
            pSrvc_pid_info[i].ring_buf_size = (pSrvc_pid_info[i].ring_buf_size) ? pSrvc_pid_info[i].ring_buf_size : pSrvc_attr[i].srvc_cache_buf_size;
            pSrvc_pid_info[i].pRing_buf     = pSrvc_attr[i].pSrvc_cache_buf;
            if( pSrvc_pid_info[i].ring_buf_size != pSrvc_attr[i].srvc_cache_buf_size )
            {
                tsext_msg_ex(TSEXT_MSG_ERR, " Warning, srvc_cache_buf_size not match (%d, %d) !!",
                            pSrvc_pid_info[i].ring_buf_size, pSrvc_attr[i].srvc_cache_buf_size);
            }
        }
    }

    return 0;
}

static uint32_t
_tsext_pkt_demux_create(
    TSEXT_DEV       *pTsExtDev,
    TSS_MBOX_ARG    *pMbox_arg,
    void            *extraData)
{
    uint32_t                dest_idx = 0;
    TSEXT_PKT_ANAL_INFO     *pCur_pkt_anal_info = 0;
    TSPD_HANLDE             *pHTspd = 0;
    TSPD_SETUP_PARAM        setupParam = {0};

    dest_idx = pMbox_arg->arg.user.index;

    pCur_pkt_anal_info = &pTsExtDev->pkt_anal_info[dest_idx];

    // create tspa handle
    setupParam.total_service = pCur_pkt_anal_info->total_service;
    switch( pTsExtDev->pkt_split_level )
    {
        case TSEXT_PKT_SPLIT_DEMOD:     setupParam.pkt_demux_level = TSPD_DEMUX_LEVEL_DEMOD;    break;
        case TSEXT_PKT_SPLIT_SERVICE:   setupParam.pkt_demux_level = TSPD_DEMUX_LEVEL_SERVICE;  break;
    }

    setupParam.bSpareMem                                           = pTsExtDev->bSpareMem;

    setupParam.get_user_info.func                                  = _tsext_pkt_demux_get_user_info;
    setupParam.get_user_info.user_mbox_arg.type                    = TSPD_USER_MBOX_TYPE_GET_USER_INFO;
    setupParam.get_user_info.user_mbox_arg.arg.set_pid.pTunnelInfo = (void*)pCur_pkt_anal_info;

    setupParam.set_srvc_pid.func                                  = _tsext_pkt_demux_set_srvc_pid;
    setupParam.set_srvc_pid.user_mbox_arg.type                    = TSPD_USER_MBOX_TYPE_SET_PID;
    setupParam.set_srvc_pid.user_mbox_arg.arg.set_pid.pTunnelInfo = (void*)pCur_pkt_anal_info;

    tspd_CreateHandle((TSPD_HANLDE**)(&pCur_pkt_anal_info->pHTspd), &setupParam, 0);

    // pHTspd = (TSPD_HANLDE*)pCur_pkt_anal_info->pHTspd;

    return 0;
}

static uint32_t
_tsext_pkt_demux_destroy(
    TSEXT_DEV       *pTsExtDev,
    TSS_MBOX_ARG    *pMbox_arg,
    void            *extraData)
{
    uint32_t                dest_idx = 0;
    TSEXT_PKT_ANAL_INFO     *pCur_pkt_anal_info = 0;

    dest_idx = pMbox_arg->arg.user.index;

    pCur_pkt_anal_info = &pTsExtDev->pkt_anal_info[dest_idx];

    tspd_DestroyHandle((TSPD_HANLDE**)(&pCur_pkt_anal_info->pHTspd), 0);

    pCur_pkt_anal_info->proc_mode = TSEXT_PKT_PROC_IDLE;

    return 0;
}

static uint32_t
_tsext_pkt_demux_proc(
    TSEXT_DEV       *pTsExtDev,
    TSS_MBOX_ARG    *pMbox_arg,
    void            *extraData)
{
    uint32_t                dest_idx = 0;
    TSEXT_PKT_ANAL_INFO     *pCur_pkt_anal_info = 0;
    TSPD_BUF_INFO           buf_info = {0};

    dest_idx = pMbox_arg->arg.user.index;

    pCur_pkt_anal_info = &pTsExtDev->pkt_anal_info[dest_idx];

    buf_info.pBufAddr  = pMbox_arg->arg.user.pBuf_addr;
    buf_info.bufLength = pMbox_arg->arg.user.buf_size;

    tspd_Pkt_Demux((TSPD_HANLDE*)pCur_pkt_anal_info->pHTspd, &buf_info, extraData);

    return 0;
}

static uint32_t
tsExt_Tss_user_cb_create(
    TSS_MBOX_ARG    *pMbox_arg,
    void            *extraData)
{
    TSEXT_DEV       *pTsExtDev = (TSEXT_DEV*)pMbox_arg->arg.user.pTunnelInfo;
    uint32_t        anal_idx = pMbox_arg->arg.user.index;

    if( pMbox_arg && pTsExtDev &&
        anal_idx < TSEXT_MAX_PKT_ANALYZER_NUM )
    {
        TSEXT_PKT_ANAL_INFO     *pCur_pkt_anal_info = &pTsExtDev->pkt_anal_info[anal_idx];

        if( pCur_pkt_anal_info->bRe_init == true )
        {
            pTsExtDev->pkt_anal_info[anal_idx].port_index = anal_idx;
            pTsExtDev->pkt_anal_info[anal_idx].get_info   = pTsExtDev->hTsExt.get_info;

            switch( pCur_pkt_anal_info->proc_mode )
            {
                case TSEXT_PKT_PROC_ANALYZE:  _tsext_pkt_analyzer_create(pTsExtDev, pMbox_arg, extraData);  break;
                case TSEXT_PKT_PROC_SPLIT:    _tsext_pkt_demux_create(pTsExtDev, pMbox_arg, extraData);     break;

                default:
                case TSEXT_PKT_PROC_IDLE:       break;
            }
            pCur_pkt_anal_info->bRe_init = false;
        }
    }

    return 0;
}

static uint32_t
tsExt_Tss_user_cb_destroy(
    TSS_MBOX_ARG    *pMbox_arg,
    void            *extraData)
{
    TSEXT_DEV       *pTsExtDev = (TSEXT_DEV*)pMbox_arg->arg.user.pTunnelInfo;
    uint32_t        anal_idx = pMbox_arg->arg.user.index;

    if( pMbox_arg && pTsExtDev &&
        anal_idx < TSEXT_MAX_PKT_ANALYZER_NUM )
    {
        TSEXT_PKT_ANAL_INFO     *pCur_pkt_anal_info = &pTsExtDev->pkt_anal_info[anal_idx];
        TSE_USER_MBOX           get_info = {0};

        pTsExtDev->pkt_anal_info[anal_idx].get_info = get_info;

        switch( pCur_pkt_anal_info->proc_mode )
        {
            case TSEXT_PKT_PROC_ANALYZE:  _tsext_pkt_analyzer_destroy(pTsExtDev, pMbox_arg, extraData);  break;
            case TSEXT_PKT_PROC_SPLIT:    _tsext_pkt_demux_destroy(pTsExtDev, pMbox_arg, extraData);     break;

            default:
            case TSEXT_PKT_PROC_IDLE:       break;
        }
    }

    return 0;
}

static uint32_t
tsExt_Tss_user_cb_proc(
    TSS_MBOX_ARG    *pMbox_arg,
    void            *extraData)
{
    TSEXT_DEV       *pTsExtDev = (TSEXT_DEV*)pMbox_arg->arg.user.pTunnelInfo;
    uint32_t        anal_idx = pMbox_arg->arg.user.index;

    if( pMbox_arg && pTsExtDev &&
        anal_idx < TSEXT_MAX_PKT_ANALYZER_NUM )
    {
        TSEXT_PKT_ANAL_INFO     *pCur_pkt_anal_info = &pTsExtDev->pkt_anal_info[anal_idx];

        switch( pCur_pkt_anal_info->proc_mode )
        {
            case TSEXT_PKT_PROC_ANALYZE:  _tsext_pkt_analyzer_proc(pTsExtDev, pMbox_arg, extraData);  break;
            case TSEXT_PKT_PROC_SPLIT:    _tsext_pkt_demux_proc(pTsExtDev, pMbox_arg, extraData);     break;

            default:
            case TSEXT_PKT_PROC_IDLE:       break;
        }
    }

    return 0;
}
//=============================================================================
//                Public Function Definition
//=============================================================================
TSE_ERR
tsExt_CreateHandle(
    TSEXT_HANDLE        **pHTsExt,
    TSEXT_INIT_PARAM    *pInitParam,
    void                *extraData)
{
    TSE_ERR         result = TSEXT_ERR_OK;
    TSEXT_DEV       *pTsExtDev = 0;

    // _trace_enter(TSE_MSG_TRACE_TSEXT, "0x%x, 0x%x, 0x%x\n", pHTsExt, pInitParam, extraData);

    do{
        uint32_t            i = 0;
        TSS_INIT_PARAM      tssInitParam = {0};
        TSS_USER_OPR        tssUserOpr = {0};

        if( *pHTsExt != 0 )
        {
            tsext_msg_ex(TSEXT_MSG_ERR, " error, Exist tse handle !!");
            result = TSEXT_ERR_INVALID_PARAMETER;
            break;
        }

        if( !pInitParam )
        {
            tsext_msg_ex(TSEXT_MSG_ERR, " error, Need pre-set info !!");
            result = TSEXT_ERR_INVALID_PARAMETER;
            break;
        }

        pTsExtDev = tsext_malloc(sizeof(TSEXT_DEV));
        if( !pTsExtDev )
        {
            tsext_msg_ex(TSEXT_MSG_ERR, " error, allocate fail !!");
            result = TSEXT_ERR_ALLOCATE_FAIL;
            break;
        }

        memset(pTsExtDev, 0x0, sizeof(TSEXT_DEV));

        //-------------------------------
        // init paraments
        if( pInitParam->act_pkt_size < TSEXT_TS_PACKET_SIZE )
        {
            tsext_msg_ex(TSEXT_MSG_ERR, " error, wrong packet size !!");
            result = TSEXT_ERR_INVALID_PARAMETER;
            break;
        }
        pTsExtDev->act_pkt_size    = pInitParam->act_pkt_size;
        pTsExtDev->pkt_split_level = pInitParam->pkt_split_level;
        pTsExtDev->bSpareMem       = pInitParam->bSpareMem;

        pTsExtDev->pIncompletePktCache = tse_malloc(pTsExtDev->act_pkt_size);
        if( !pTsExtDev->pIncompletePktCache )
        {
            tsext_msg_ex(TSEXT_MSG_ERR, " error, allocate fail !!");
            result = TSEXT_ERR_ALLOCATE_FAIL;
            break;
        }

        for(i = 0; i < TSEXT_MAX_PKT_ANALYZER_NUM; i++)
        {
            pTsExtDev->pkt_anal_info[i].bRe_init           = true;
            pTsExtDev->pkt_anal_info[i].pat_version_number = -1;
            pTsExtDev->pkt_anal_info[i].sdt_version_number = -1;
        }

        //---------------------------
        // create ts split handle
        tssInitParam.bBy_Pass_Tss = pInitParam->bBy_Pass_Tss;
        tss_CreateHandle(&pTsExtDev->pHTss, &tssInitParam, extraData);
        if( !pTsExtDev->pHTss )
        {
            tsext_msg_ex(TSEXT_MSG_ERR, " error, create Tss hanlde fail !!");
            result = TSEXT_ERR_ALLOCATE_FAIL;
            break;
        }

        //---------------------------------
        // attach ts split packet user callback function
        tssUserOpr.user_create.func                              = tsExt_Tss_user_cb_create;
        tssUserOpr.user_create.tss_mbox_arg.type                 = TSS_MBOX_TYPE_USER_CB;
        tssUserOpr.user_create.tss_mbox_arg.arg.user.pTunnelInfo = pTsExtDev;

        tssUserOpr.user_destroy.func                              = tsExt_Tss_user_cb_destroy;
        tssUserOpr.user_destroy.tss_mbox_arg.type                 = TSS_MBOX_TYPE_USER_CB;
        tssUserOpr.user_destroy.tss_mbox_arg.arg.user.pTunnelInfo = pTsExtDev;

        tssUserOpr.user_proc.func                              = tsExt_Tss_user_cb_proc;
        tssUserOpr.user_proc.tss_mbox_arg.type                 = TSS_MBOX_TYPE_USER_CB;
        tssUserOpr.user_proc.tss_mbox_arg.arg.user.pTunnelInfo = pTsExtDev;

        for(i = 0; i < TSEXT_MAX_PKT_ANALYZER_NUM; i++)
        {
            tssUserOpr.port_index = i;
            tss_Attach_User_Operator(pTsExtDev->pHTss, &tssUserOpr, extraData);
        }

        //---------------------------------------
        (*pHTsExt) = &pTsExtDev->hTsExt;

    }while(0);

    if( result != TSEXT_ERR_OK )
    {
        if( pTsExtDev )
        {
            TSEXT_HANDLE        *pTmpHTsExt = 0;

            pTsExtDev->status = TSEXT_STATUS_FAIL;
            pTmpHTsExt = &pTsExtDev->hTsExt;
            tsExt_DestroyHandle(&pTmpHTsExt, 0);
        }
        tsext_msg_ex(TSEXT_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    // _trace_leave(TSE_MSG_TRACE_TSEXT);
    return result;
}

TSE_ERR
tsExt_DestroyHandle(
    TSEXT_HANDLE      **pHTsExt,
    void              *extraData)
{
    TSE_ERR             result = TSEXT_ERR_OK;
    TSEXT_DEV           *pTsExtDev = 0;
    // pthread_mutex_t     tss_mutex = 0;

    // _trace_enter(TSE_MSG_TRACE_TSEXT, "0x%x\n", pTsExtDev);

    /**
     * Ap layer need to check all threads, which assess this handle, in STOP state.
     * Or system maybe crash.
     **/

    _verify_handle(pHTsExt, result);
    _verify_handle((*pHTsExt), result);

    pTsExtDev = DOWN_CAST(TSEXT_DEV, (*pHTsExt), hTsExt);

    // _mutex_lock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);

    // _disable_irq();
    if( pTsExtDev )
    {
        uint32_t    i;

        if( pTsExtDev->pIncompletePktCache )
            free(pTsExtDev->pIncompletePktCache);

        for(i = 0; i < TSEXT_MAX_PKT_ANALYZER_NUM; i++)
        {
            TSS_MBOX_ARG    mbox_arg = {0};

            mbox_arg.type                  = TSS_MBOX_TYPE_USER_CB;
            mbox_arg.arg.user.index        = i;
            mbox_arg.arg.user.feedback_cmd = TSS_CMD_USER_OPR_DESTROY;
            mbox_arg.arg.user.pTunnelInfo  = (void*)pTsExtDev;

            tsExt_Tss_user_cb_destroy(&mbox_arg, extraData);

            if( pTsExtDev->pkt_anal_info[i].pSrvc_pid_info )
                free(pTsExtDev->pkt_anal_info[i].pSrvc_pid_info);
        }

        tss_DestroyHandle(&pTsExtDev->pHTss, extraData);

        // tss_mutex = pTsExtDev->tss_mutex;
        free(pTsExtDev);
        *pHTsExt = 0;
    }

    // _mutex_unlock(TSE_MSG_TRACE_TSEXT, tss_mutex);
    // _mutex_deinit(TSE_MSG_TRACE_TSEXT, tss_mutex);

    // _enable_irq();
    // _trace_leave(TSE_MSG_TRACE_TSEXT);
    return result;
}

TSE_ERR
tsExt_Add_Passport_Info(
    TSEXT_HANDLE            *pHTsExt,
    TSEXT_PASSPORT_INFO     *pPassportInfo,
    void                    *extraData)
{
    TSE_ERR         result = TSEXT_ERR_OK;
    TSEXT_DEV       *pTsExtDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSEXT, "0x%x, 0x%x, 0x%x\n", pHTsExt, pPassportInfo, extraData);
    _verify_handle(pHTsExt, result);
    _verify_handle(pPassportInfo, result);

    pTsExtDev = DOWN_CAST(TSEXT_DEV, pHTsExt, hTsExt);

    // _mutex_lock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);

    if( pTsExtDev && pTsExtDev->status != TSEXT_STATUS_FAIL )
    {
        TSS_PASSPORT_INFO   passportInfo = {0};

        passportInfo.tag_value = pPassportInfo->tag_value;
        passportInfo.tag_len   = pPassportInfo->tag_len;

        tss_Add_Passport_Info(pTsExtDev->pHTss, &passportInfo, extraData);
    }

    if( result != TSEXT_ERR_OK )
    {
        pTsExtDev->status = TSEXT_STATUS_FAIL;
        tsext_msg_ex(TSEXT_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    // _mutex_unlock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);
    _trace_leave(TSE_MSG_TRACE_TSEXT);
    return result;
}


TSE_ERR
tsExt_Add_Cmd_Pkt_Info(
    TSEXT_HANDLE        *pHTsExt,
    uint32_t            port_index,
    TSEXT_CMD_PKT_ATTR  *pCmd_pkt_attr,
    void                *extraData)
{
    TSE_ERR         result = TSEXT_ERR_OK;
    TSEXT_DEV       *pTsExtDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSEXT, "0x%x, 0x%x, 0x%x\n", pHTsExt, pCmd_pkt_attr, extraData);
    _verify_handle(pHTsExt, result);
    _verify_handle(pCmd_pkt_attr, result);

    pTsExtDev = DOWN_CAST(TSEXT_DEV, pHTsExt, hTsExt);

    // _mutex_lock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);

    if( pTsExtDev && pTsExtDev->status != TSEXT_STATUS_FAIL )
    {
        do{
            TSPD_CMD_PKT_ATTR   tspd_cmd_pkt_attr = {0};

            if( port_index >= TSEXT_MAX_PKT_ANALYZER_NUM )
            {
                tsext_msg_ex(TSEXT_MSG_ERR, "wrong index (max=%d) !", TSEXT_MAX_PKT_ANALYZER_NUM);
                break;
            }

            tspd_cmd_pkt_attr.cmd_pkt_pid = pCmd_pkt_attr->cmd_pkt_pid;

            tspd_Attach_Cmd_Pkt_Info(pTsExtDev->pkt_anal_info[port_index].pHTspd, &tspd_cmd_pkt_attr, extraData);
        }while(0);
    }

    if( result != TSEXT_ERR_OK )
    {
        pTsExtDev->status = TSEXT_STATUS_FAIL;
        tsext_msg_ex(TSEXT_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    // _mutex_unlock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);
    _trace_leave(TSE_MSG_TRACE_TSEXT);
    return result;
}


TSE_ERR
tsExt_Extract(
    TSEXT_HANDLE    *pHTsExt,
    TSEXT_BUF_INFO  *pBuf_info,
    void            *extraData)
{
    TSE_ERR         result = TSEXT_ERR_OK;
    TSEXT_DEV       *pTsExtDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSEXT, "0x%x, 0x%x, 0x%x\n", pHTsExt, pBuf_info, extraData);
    _verify_handle(pHTsExt, result);
    _verify_handle(pBuf_info, result);

    pTsExtDev = DOWN_CAST(TSEXT_DEV, pHTsExt, hTsExt);

    // _mutex_lock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);

    if( pTsExtDev && pTsExtDev->status != TSEXT_STATUS_FAIL )
    {
        uint8_t     *pData = pBuf_info->pBufAddr;
        uint32_t    remainSize = pBuf_info->bufLength;

        while( remainSize > 0 )
        {
            switch( pTsExtDev->stream_state )
            {
                case TSEXT_STREAM_STATE_SEARCH_PKT_START:
                    if( (*pData) == TSEXT_VALID_SYNC_BYTE )
                    {
                        if( remainSize >= pTsExtDev->act_pkt_size )
                        {
                            TSS_BUF_INFO    data_info = {0};

                            data_info.pBufAddr  = pData;
                            data_info.bufLength = pTsExtDev->act_pkt_size;
                            tss_Split(pTsExtDev->pHTss, &data_info, extraData);

                            pData       += pTsExtDev->act_pkt_size;
                            remainSize  -= pTsExtDev->act_pkt_size;
                            break;
                        }
                        else
                        {
                            pTsExtDev->stream_state = TSEXT_STREAM_STATE_LESS_PKT_SIZE;
                        }
                    }
                    else
                    {
                        ++pData;
                        --remainSize;
                    }
                    break;

                case TSEXT_STREAM_STATE_LESS_PKT_SIZE:
                    if( pTsExtDev->collectedByte > 0 &&
                        remainSize >= (int)(pTsExtDev->act_pkt_size - pTsExtDev->collectedByte) )
                    {
                        TSS_BUF_INFO    data_info = {0};

                        memcpy(&pTsExtDev->pIncompletePktCache[pTsExtDev->collectedByte],
                               pData, (pTsExtDev->act_pkt_size - pTsExtDev->collectedByte));

                        data_info.pBufAddr  = pTsExtDev->pIncompletePktCache;
                        data_info.bufLength = pTsExtDev->act_pkt_size;
                        tss_Split(pTsExtDev->pHTss, &data_info, extraData);

                        pData       += (pTsExtDev->act_pkt_size - pTsExtDev->collectedByte);
                        remainSize  -= (pTsExtDev->act_pkt_size - pTsExtDev->collectedByte);

                        pTsExtDev->collectedByte = 0;
                        pTsExtDev->stream_state  = TSEXT_STREAM_STATE_SEARCH_PKT_START;
                        break;
                    }
                    else
                    {
                        memcpy(&pTsExtDev->pIncompletePktCache[pTsExtDev->collectedByte],
                               pData, remainSize);

                        pTsExtDev->collectedByte += remainSize;
                        remainSize = 0;
                    }
                    break;
            }
        }
    }

    if( result != TSEXT_ERR_OK )
    {
        pTsExtDev->status = TSEXT_STATUS_FAIL;
        tsext_msg_ex(TSEXT_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    // _mutex_unlock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);
    _trace_leave(TSE_MSG_TRACE_TSEXT);
    return result;
}

TSE_ERR
tsExt_Set_Pkt_Proc_Mode(
    TSEXT_HANDLE            *pHTsExt,
    uint32_t                port_idx,
    TSEXT_PKT_PROC_MODE     mode,
    void                    *extraData)
{
    TSE_ERR         result = TSEXT_ERR_OK;
    TSEXT_DEV       *pTsExtDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSEXT, "0x%x, 0x%x\n", pHTsExt, extraData);
    _verify_handle(pHTsExt, result);

    pTsExtDev = DOWN_CAST(TSEXT_DEV, pHTsExt, hTsExt);

    // _mutex_lock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);

    if( pTsExtDev && pTsExtDev->status != TSEXT_STATUS_FAIL )
    {
        do{
            uint32_t        i = 0;
            TSS_USER_OPR    tssUserOpr = {0};
            TSS_MBOX_ARG    mbox_arg = {0};

            if( port_idx >= TSEXT_MAX_PKT_ANALYZER_NUM )
            {
                tsext_msg_ex(TSEXT_MSG_ERR, "wrong index (max=%d) !", TSEXT_MAX_PKT_ANALYZER_NUM);
                break;
            }

            // the same proc_mode
            if( pTsExtDev->pkt_anal_info[port_idx].proc_mode == mode )
                break;

            mbox_arg.type                  = TSS_MBOX_TYPE_USER_CB;
            mbox_arg.arg.user.index        = port_idx;
            mbox_arg.arg.user.feedback_cmd = TSS_CMD_USER_OPR_DESTROY;
            mbox_arg.arg.user.pTunnelInfo  = (void*)pTsExtDev;

            tsExt_Tss_user_cb_destroy(&mbox_arg, extraData);

            pTsExtDev->pkt_anal_info[port_idx].proc_mode = mode;
            pTsExtDev->pkt_anal_info[port_idx].bRe_init  = true;

            //---------------------------------
            // attach ts split packet user callback function
            tssUserOpr.user_create.func                              = tsExt_Tss_user_cb_create;
            tssUserOpr.user_create.tss_mbox_arg.type                 = TSS_MBOX_TYPE_USER_CB;
            tssUserOpr.user_create.tss_mbox_arg.arg.user.pTunnelInfo = pTsExtDev;

            tssUserOpr.user_destroy.func                              = tsExt_Tss_user_cb_destroy;
            tssUserOpr.user_destroy.tss_mbox_arg.type                 = TSS_MBOX_TYPE_USER_CB;
            tssUserOpr.user_destroy.tss_mbox_arg.arg.user.pTunnelInfo = pTsExtDev;

            tssUserOpr.user_proc.func                              = tsExt_Tss_user_cb_proc;
            tssUserOpr.user_proc.tss_mbox_arg.type                 = TSS_MBOX_TYPE_USER_CB;
            tssUserOpr.user_proc.tss_mbox_arg.arg.user.pTunnelInfo = pTsExtDev;

            tssUserOpr.port_index = port_idx;
            tss_Attach_User_Operator(pTsExtDev->pHTss, &tssUserOpr, extraData);
        }while(0);
    }

    if( result != TSEXT_ERR_OK )
    {
        pTsExtDev->status = TSEXT_STATUS_FAIL;
        tsext_msg_ex(TSEXT_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    // _mutex_unlock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);
    _trace_leave(TSE_MSG_TRACE_TSEXT);
    return result;
}

TSE_ERR
tsExt_Set_Service_Recv(
    TSEXT_HANDLE    *pHTsExt,
    uint32_t        port_idx,
    uint32_t        service_idx,
    bool            bReceive,
    void            *extraData)
{
    TSE_ERR         result = TSEXT_ERR_OK;
    TSEXT_DEV       *pTsExtDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSEXT, "0x%x, 0x%x\n", pHTsExt, extraData);
    _verify_handle(pHTsExt, result);

    pTsExtDev = DOWN_CAST(TSEXT_DEV, pHTsExt, hTsExt);

    // _mutex_lock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);

    if( pTsExtDev && pTsExtDev->status != TSEXT_STATUS_FAIL )
    {
        do{
            TSEXT_PKT_ANAL_INFO     *pAct_pkt_analyzer_info = 0;

            if( port_idx >= TSEXT_MAX_PKT_ANALYZER_NUM )
            {
                tsext_msg_ex(TSEXT_MSG_ERR, "wrong port index (%d, max=%d) !",
                                port_idx, TSEXT_MAX_PKT_ANALYZER_NUM);
                break;
            }

            pAct_pkt_analyzer_info = &pTsExtDev->pkt_anal_info[port_idx];

            if( service_idx >= pAct_pkt_analyzer_info->total_service )
            {
                tsext_msg_ex(TSEXT_MSG_ERR, "wrong service index (%d, max=%d) !",
                                service_idx, pAct_pkt_analyzer_info->total_service);
                break;
            }

            if( bReceive )      tspd_Srvc_Start_Cache(pAct_pkt_analyzer_info->pHTspd, service_idx, 0);
            else                tspd_Srvc_Stop_Cache(pAct_pkt_analyzer_info->pHTspd, service_idx, 0);

        }while(0);
    }

    if( result != TSEXT_ERR_OK )
    {
        pTsExtDev->status = TSEXT_STATUS_FAIL;
        tsext_msg_ex(TSEXT_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    // _mutex_unlock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);
    _trace_leave(TSE_MSG_TRACE_TSEXT);
    return result;
}

TSE_ERR
tsExt_Get_Info(
    TSEXT_HANDLE        *pHTsExt,
    TSEXT_USER_INFO     *pUser_info,
    void                *extraData)
{
    TSE_ERR         result = TSEXT_ERR_OK;
    TSEXT_DEV       *pTsExtDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSEXT, "0x%x, 0x%x\n", pHTsExt, extraData);
    _verify_handle(pHTsExt, result);
    _verify_handle(pUser_info, result);

    pTsExtDev = DOWN_CAST(TSEXT_DEV, pHTsExt, hTsExt);

    // _mutex_lock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);

    if( pTsExtDev && pTsExtDev->status != TSEXT_STATUS_FAIL )
    {

    }

    if( result != TSEXT_ERR_OK )
    {
        pTsExtDev->status = TSEXT_STATUS_FAIL;
        tsext_msg_ex(TSEXT_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    // _mutex_unlock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);
    _trace_leave(TSE_MSG_TRACE_TSEXT);
    return result;
}

TSE_ERR
tsExt_Get_Sample(
    TSEXT_HANDLE        *pHTsExt,
    TSEXT_SAMPLE_INFO   *pSample_info,
    void                *extraData)
{
    TSE_ERR         result = TSEXT_ERR_OK;
    TSEXT_DEV       *pTsExtDev = 0;

    _trace_enter(TSE_MSG_TRACE_TSEXT, "0x%x, 0x%x\n", pHTsExt, extraData);
    _verify_handle(pHTsExt, result);
    _verify_handle(pSample_info, result);

    pTsExtDev = DOWN_CAST(TSEXT_DEV, pHTsExt, hTsExt);

    // _mutex_lock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);

    if( pTsExtDev && pTsExtDev->status != TSEXT_STATUS_FAIL )
    {
        if( pSample_info->port_idx < TSEXT_MAX_PKT_ANALYZER_NUM )
        {
            TSEXT_PKT_ANAL_INFO     *pAct_pkt_analyzer_info = 0;
            TSPD_SAMPLE_INFO        tspd_sample_info = {0};

            pAct_pkt_analyzer_info = &pTsExtDev->pkt_anal_info[pSample_info->port_idx];

            if( pAct_pkt_analyzer_info->proc_mode == TSEXT_PKT_PROC_SPLIT &&
                pAct_pkt_analyzer_info->pHTspd )
            {
                tspd_sample_info.service_idx  = pSample_info->service_idx;
                tspd_sample_info.customer_idx = pSample_info->customer_idx;
                tspd_sample_info.bufLength    = pSample_info->bufLength;

                switch( tspd_sample_info.service_idx )
                {
                    case TSEXT_CMD_SRVC_INDEX:
                        tspd_Get_Cmd_Pkt_Stream((TSPD_HANLDE*)pAct_pkt_analyzer_info->pHTspd, &tspd_sample_info, extraData);
                        break;

                    default:
                        tspd_Get_Srvc_Stream((TSPD_HANLDE*)pAct_pkt_analyzer_info->pHTspd, &tspd_sample_info, extraData);
                        break;
                }

                pSample_info->bufLength = tspd_sample_info.bufLength;
                pSample_info->pBufAddr  = tspd_sample_info.pBufAddr;
            }
            else
            {
                pSample_info->bufLength = 0;
                pSample_info->pBufAddr  = 0;
            }
        }
    }

    if( result != TSEXT_ERR_OK )
    {
        pTsExtDev->status = TSEXT_STATUS_FAIL;
        tsext_msg_ex(TSEXT_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    // _mutex_unlock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);
    _trace_leave(TSE_MSG_TRACE_TSEXT);
    return result;
}

// TSE_ERR
// tsExt_tamplete(
//     TSEXT_HANDLE   *pHTsExt,
//     void           *extraData)
// {
//     TSE_ERR         result = TSEXT_ERR_OK;
//     TSEXT_DEV       *pTsExtDev = 0;
//
//     _trace_enter(TSE_MSG_TRACE_TSEXT, "0x%x, 0x%x\n", pHTsExt, extraData);
//     _verify_handle(pHTsExt, result);
//
//     pTsExtDev = DOWN_CAST(TSEXT_DEV, pHTsExt, hTsExt);
//
//     // _mutex_lock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);
//
//     if( pTsExtDev && pTsExtDev->status != TSEXT_STATUS_FAIL )
//     {
//
//     }
//
//     if( result != TSEXT_ERR_OK )
//     {
//         pTsExtDev->status = TSEXT_STATUS_FAIL;
//         tsext_msg_ex(TSEXT_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
//     }
//
//     // _mutex_unlock(TSE_MSG_TRACE_TSEXT, pTsExtDev->tss_mutex);
//     _trace_leave(TSE_MSG_TRACE_TSEXT);
//     return result;
// }

