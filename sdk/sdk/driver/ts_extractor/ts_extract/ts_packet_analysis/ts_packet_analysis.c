
#include "ts_packet_analysis_defs.h"
#include "ts_packet_analysis.h"

//=============================================================================
//                Constant Definition
//=============================================================================
typedef enum TSPA_PID_TYPE_T
{
    TSPA_PID_PSI = 0,
    TSPA_PID_PES,
} TSPA_PID_TYPE;

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
#if !defined(TSPA_LOCAL_MACRO_DISABLE)
uint32_t  tspaMsgOnFlag = 0x1;
#endif
//=============================================================================
//                Private Function Definition
//=============================================================================
static void _tspa_destroy_pid_handler(TS_PACKET_ANAL *pTspaDev, TSPA_PID *pTsPid_handler);


static inline void
_tspa_clear_pat_handler(
    TS_PACKET_ANAL  *pTspaDev,
    TSPA_PID        *pPid_handler)
{
    if( pPid_handler->pPsi_handler->pPsiPktDecoder )
    {
        PSI_TABLE_MBOX  psiTableMbox = {0};

        psiTableMbox.psi_table_mbox_arg.psi_table_id = PSI_TABLE_PAT;
        psi_pkt_destroy_decoder(&pPid_handler->pPsi_handler->pPsiPktDecoder, &psiTableMbox, 0);
    }

    // Clean all the PMTs belong to the PAT
    if( pPid_handler->pPsi_handler->table.pat.pPmtPid )
    {
        uint32_t    pmtIdx = 0;
        uint32_t    totalCnt = pPid_handler->pPsi_handler->table.pat.totalPmtPidCount;
        for(pmtIdx = 0; pmtIdx < totalCnt; pmtIdx++)
        {
            uint32_t    pmt_pid = pPid_handler->pPsi_handler->table.pat.pPmtPid[pmtIdx];
            if( pTspaDev->pTsPid_handler[pmt_pid] )
                _tspa_destroy_pid_handler(pTspaDev, pTspaDev->pTsPid_handler[pmt_pid]);
        }

        free(pPid_handler->pPsi_handler->table.pat.pPmtPid);
    }

    return;
}

static inline void
_tspa_clear_pmt_handler(
    TS_PACKET_ANAL  *pTspaDev,
    TSPA_PID        *pPid_handler)
{
    TSPA_PMT_SECT   *pHeadPmt = &pPid_handler->pPsi_handler->table.pmt;
    TSPA_PMT_SECT   *pCurPmt = pHeadPmt;
    TSPA_PMT_SECT   *pNextPmt = 0;

    if( pPid_handler->pPsi_handler->pPsiPktDecoder )
    {
        PSI_TABLE_MBOX  psiTableMbox = {0};

        psiTableMbox.psi_table_mbox_arg.psi_table_id = PSI_TABLE_PMT;
        psi_pkt_destroy_decoder(&pPid_handler->pPsi_handler->pPsiPktDecoder, &psiTableMbox, 0);
    }

    // Clean all the elementary streams belong to this PMT
    while( pCurPmt )
    {
        pNextPmt = pCurPmt->pNextPmt;
        if( pCurPmt->pEsPid )
        {
            if( pCurPmt->totalEsPidCount > 0 )
            {
                uint32_t    esIdx = 0;
                uint32_t    totalCnt = pCurPmt->totalEsPidCount;
                for(esIdx = 0; esIdx < totalCnt; esIdx++)
                {
                    uint32_t    esPid = pCurPmt->pEsPid[esIdx];

                    _tspa_destroy_pid_handler(pTspaDev, pTspaDev->pTsPid_handler[esPid]);
                }
            }
            free(pCurPmt->pEsPid);
        }

        if( pCurPmt != pHeadPmt )
            free(pCurPmt);

        pCurPmt = pNextPmt;
    }
    return;
}

static TSPA_PID*
_tspa_create_pid_handler(
    uint32_t        pid,
    TSPA_PID_TYPE   pidType,
    bool            bPidValid)
{
    TSPA_PID    *pCur_pid_handler = 0;

    do{
        bool        bSkip = false;

        pCur_pid_handler = (TSPA_PID*)tspa_malloc(sizeof(TSPA_PID));
        if( !pCur_pid_handler )
        {
            tspa_msg_ex(TSPA_MSG_ERR, " error, allocate fail !!");
            break;
        }

        // TS PID initialization
        memset(pCur_pid_handler, 0x0, sizeof(TSPA_PID));
        pCur_pid_handler->pid = pid;

        switch( pidType )
        {
            case TSPA_PID_PSI:
                pCur_pid_handler->pPsi_handler = (TSPA_PSI*)tspa_malloc(sizeof(TSPA_PSI));
                if( pCur_pid_handler->pPsi_handler == NULL )
                {
                    free(pCur_pid_handler);
                    pCur_pid_handler = 0;
                    bSkip = true;
                }
                else
                {
                    // TS PSI initialization
                    memset(pCur_pid_handler->pPsi_handler, 0x0, sizeof(TSPA_PSI));
                    pCur_pid_handler->pPsi_handler->version_number = INVALID_VERSION_NUMBER;
                }
                break;

            case TSPA_PID_PES:
                pCur_pid_handler->pPes_handler = (TSPA_PES*)tspa_malloc(sizeof(TSPA_PES));
                if( pCur_pid_handler->pPes_handler == NULL )
                {
                    free(pCur_pid_handler);
                    pCur_pid_handler = 0;
                    bSkip = true;
                }
                else
                {
                    // TS PES initialization
                    memset(pCur_pid_handler->pPes_handler, 0x0, sizeof(TSPA_PES));
                }
                break;

            default:
                tspa_msg_ex(TSPA_MSG_ERR, "Invalid PID type (0x%x)\n", pidType);
                bSkip = true;
                break;
        }

        if( bSkip == true )         break;

        pCur_pid_handler->bValid = bPidValid;
    }while(0);

    return pCur_pid_handler;
}

static void
_tspa_destroy_pid_handler(
    TS_PACKET_ANAL  *pTspaDev,
    TSPA_PID        *pTsPid_handler)
{
    uint32_t    pid = 0;
    do{
        if( !pTsPid_handler )       break;

        pid = pTsPid_handler->pid;

        if( pTsPid_handler->pPsi_handler )
        {
            switch( pid )
            {
                default:        _tspa_clear_pmt_handler(pTspaDev, pTsPid_handler);    break; // PMT

                case PAT_PID:   _tspa_clear_pat_handler(pTspaDev, pTsPid_handler);    break;

                case SDT_PID:
                    if( pTsPid_handler->pPsi_handler->pPsiPktDecoder )
                    {
                        PSI_TABLE_MBOX  psiTableMbox = {0};

                        psiTableMbox.psi_table_mbox_arg.psi_table_id = PSI_TABLE_SDT;
                        psi_pkt_destroy_decoder(&pTsPid_handler->pPsi_handler->pPsiPktDecoder, &psiTableMbox, 0);
                    }
                    break;
            }

            free(pTsPid_handler->pPsi_handler);
        }

        if( pTsPid_handler->pPes_handler )
        {
        #if (TSPA_ENABLE_PES_DECODE)
            // not ready !!!!!
            if( pTsPid_handler->pPes_handler->pPesPktDecoder )
            {
                PES_STREAM_MBOX         pesStreamMbox = {0};

                switch( pTsPid_handler->pPes_handler->pes_stream_id )
                {
                    case PES_STREAM_VIDEO:
                        pesStreamMbox.pes_stream_mbox_arg.pes_stream_id     = PES_STREAM_VIDEO;
                        pesStreamMbox.pes_stream_mbox_arg.arg.v.pTunnelInfo = (void*)pTspaDev;
                        break;

                    case PES_STREAM_AUDIO:
                        pesStreamMbox.pes_stream_mbox_arg.pes_stream_id = PES_STREAM_AUDIO;
                        pesStreamMbox.pes_stream_mbox_arg.arg.a.pTunnelInfo = (void*)pTspaDev;
                        break;

                    case PES_STREAM_TELETEXT:
                        pesStreamMbox.pes_stream_mbox_arg.pes_stream_id = PES_STREAM_TELETEXT;
                        pesStreamMbox.pes_stream_mbox_arg.arg.t.pTunnelInfo = (void*)pTspaDev;
                        break;

                    case PES_STREAM_SUBTITLE:
                        pesStreamMbox.pes_stream_mbox_arg.pes_stream_id = PES_STREAM_SUBTITLE;
                        pesStreamMbox.pes_stream_mbox_arg.arg.s.pTunnelInfo = (void*)pTspaDev;
                        break;
                }

                pes_pkt_destroy_decoder(&pTsPid_handler->pPes_handler->pPesPktDecoder, &pesStreamMbox, 0);
            }
        #endif
            free(pTsPid_handler->pPes_handler);
        }

        free(pTsPid_handler);
        pTspaDev->pTsPid_handler[pid] = 0;
    }while(0);



    return;
}

static uint32_t
_tspa_pes_info_recv(
    PES_STREAM_MBOX_ARG  *pPes_mbox_arg,
    void                 *extraData)
{
    uint32_t        result = 0;
    TS_PACKET_ANAL  *pTspaDev = 0;
    PES_INFO        *pPes_info = 0;

    switch( pPes_mbox_arg->pes_stream_id )
    {
        case PES_STREAM_VIDEO:
            pTspaDev = (TS_PACKET_ANAL*)pPes_mbox_arg->arg.v.pTunnelInfo;
            pPes_info = pPes_mbox_arg->arg.v.pPes_info;

            pTspaDev->bGetPesVideo = pPes_mbox_arg->arg.v.bGetResolution;
            break;

        case PES_STREAM_AUDIO:
            pTspaDev = (TS_PACKET_ANAL*)pPes_mbox_arg->arg.a.pTunnelInfo;
            pPes_info = pPes_mbox_arg->arg.a.pPes_info;
            break;

        case PES_STREAM_TELETEXT:
            pTspaDev = (TS_PACKET_ANAL*)pPes_mbox_arg->arg.t.pTunnelInfo;
            pPes_info = pPes_mbox_arg->arg.t.pPes_info;
            break;

        case PES_STREAM_SUBTITLE:
            pTspaDev = (TS_PACKET_ANAL*)pPes_mbox_arg->arg.s.pTunnelInfo;
            pPes_info = pPes_mbox_arg->arg.s.pPes_info;
            break;
    }

    return result;
}

static uint32_t
_tspa_pmt_info_recv(
    PSI_TABLE_MBOX_ARG  *pPsi_table_mbox_arg,
    void                *extraData)
{
    uint32_t        result = 0;
    TS_PACKET_ANAL  *pTspaDev = (TS_PACKET_ANAL*)pPsi_table_mbox_arg->argv.pmt.pTunnelInfo;
    PSI_PMT_INFO    *pPsi_Pmt_Info = (PSI_PMT_INFO*)pPsi_table_mbox_arg->argv.pmt.pPsi_Pmt_Info;

    do{
        TSPA_PID            *pPat_pid_handler = 0;
        TSPA_PID            *pPmt_pid_handler = 0;
        TSPA_PID            *pCurPmt_pid_handler = 0;
        PSI_PMT_ES_INFO     *pCurEsInfo = 0;
        PSI_DESCR           *pDescriptor = 0;
        TSPA_PMT_SECT       *pCurPmtSectInfo = 0;
        uint32_t            i;
        uint32_t            curPmtPid = 0;

        pPat_pid_handler = pTspaDev->pTsPid_handler[PAT_PID];

        if( !pTspaDev || !pPsi_Pmt_Info || !pPat_pid_handler )
        {
            if( pPsi_Pmt_Info )
            {
                PSI_TABLE_MBOX  psiTableMbox = {0};

                psiTableMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PMT;
                psiTableMbox.psi_table_mbox_arg.argv.pmt.pPsi_Pmt_Info = pPsi_Pmt_Info;
                psi_pkt_clear_table(&psiTableMbox, extraData);
            }
            break;
        }

        //---------------------------------
        // First find this PMT declared in PAT
        for(i = 0; i < pPat_pid_handler->pPsi_handler->table.pat.totalPmtPidCount; i++)
        {
            bool        bLoop_break = false;

            curPmtPid = pPat_pid_handler->pPsi_handler->table.pat.pPmtPid[i];
            switch( curPmtPid )
            {
                case TDT_TOT_PID:   break;
                case EIT_PID:       break;
                case NIT_PID:       break;
                case SDT_PID:       break;
                case PAT_PID:       break;

                default:
                    // check if it matches the program number in PMT PID
                    pCurPmt_pid_handler = pTspaDev->pTsPid_handler[curPmtPid];
                    if( pCurPmt_pid_handler && pCurPmt_pid_handler->pPsi_handler )
                    {
                        for(pCurPmtSectInfo = &pCurPmt_pid_handler->pPsi_handler->table.pmt;
                            pCurPmtSectInfo;
                            pCurPmtSectInfo = pCurPmtSectInfo->pNextPmt)
                        {
                            if( pPsi_Pmt_Info->program_number &&
                                pCurPmtSectInfo->program_number == pPsi_Pmt_Info->program_number )
                            {
                                pPmt_pid_handler = pCurPmt_pid_handler;
                                break;
                            }
                        }

                        if( pPmt_pid_handler )      bLoop_break = true;
                    }
                    break;
            }

            if( bLoop_break == true )       break;
        }

        if( !pPmt_pid_handler )
        {
            // unreferenced program (broken stream)
            PSI_TABLE_MBOX  psiTableMbox = {0};

            psiTableMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PMT;
            psiTableMbox.psi_table_mbox_arg.argv.pmt.pPsi_Pmt_Info = pPsi_Pmt_Info;
            psi_pkt_clear_table(&psiTableMbox, extraData);
            break;
        }

        if( INVALID_VERSION_NUMBER != pCurPmtSectInfo->version_number &&
            (!pPsi_Pmt_Info->current_next_indicator ||
             pPsi_Pmt_Info->version_number == pCurPmtSectInfo->version_number) )
        {
            PSI_TABLE_MBOX  psiTableMbox = {0};

            psiTableMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PMT;
            psiTableMbox.psi_table_mbox_arg.argv.pmt.pPsi_Pmt_Info = pPsi_Pmt_Info;
            psi_pkt_clear_table(&psiTableMbox, extraData);
            break;
        }

        //---------------------------------
        // ToDo: Clean all the old elementary streams belongs to the PMT?

        if( pPsi_Pmt_Info->totalEsCount > 0 )
        {
            if( pPsi_Pmt_Info->totalEsCount != pCurPmtSectInfo->totalEsPidCount &&
                pCurPmtSectInfo->pEsPid )
            {
                free(pCurPmtSectInfo->pEsPid);
                pCurPmtSectInfo->pEsPid = 0;
            }

            if( !pCurPmtSectInfo->pEsPid )
            {
                pCurPmtSectInfo->pEsPid = tspa_malloc(sizeof(uint32_t) * pPsi_Pmt_Info->totalEsCount);
                if( !pCurPmtSectInfo->pEsPid )
                {
                     PSI_TABLE_MBOX  psiTableMbox = {0};

                    psiTableMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PMT;
                    psiTableMbox.psi_table_mbox_arg.argv.pmt.pPsi_Pmt_Info = pPsi_Pmt_Info;
                    psi_pkt_clear_table(&psiTableMbox, extraData);
                    break;
                }
            }

            // reset count and go on to re-assign ES PID into PMT
            pCurPmtSectInfo->totalEsPidCount = 0;
        }

        for(pCurEsInfo = pPsi_Pmt_Info->ptFirstEsInfo;
            (pCurEsInfo &&
             pCurPmtSectInfo->totalEsPidCount < pPsi_Pmt_Info->totalEsCount);
             pCurEsInfo = pCurEsInfo->ptNexEsInfo)
        {
            uint32_t    audioAc3ChkNum = 0;
            uint32_t    esPid = pCurEsInfo->elementary_PID;
            TSPA_PID    **ppEs_pid_handler = &pTspaDev->pTsPid_handler[esPid];
            uint32_t    stream_type = pCurEsInfo->stream_type;

            if( *ppEs_pid_handler && (*ppEs_pid_handler)->pPsi_handler )
            {
                _tspa_destroy_pid_handler(pTspaDev, (*ppEs_pid_handler));
                *ppEs_pid_handler = 0;
            }

            if( !(*ppEs_pid_handler) )
            {
                (*ppEs_pid_handler) = _tspa_create_pid_handler(esPid, TSPA_PID_PES, pTspaDev->bEnableEsPID);
                if( !(*ppEs_pid_handler) )      continue;
            }

#if (TSPA_ENABLE_PES_DECODE)
            switch( stream_type )
            {
                case ISO_IEC_11172_2_VIDEO:   // mpeg1 video
                case ISO_IEC_13818_2_VIDEO:   // mpeg2 video
                case ISO_IEC_14496_10_VIDEO:  // avc (h.264) video
                    if( pTspaDev->bEnableEsPID == true )
                    {
                        pTspaDev->video_stream_type = stream_type;

                        if( (*ppEs_pid_handler)->pPes_handler &&
                            (*ppEs_pid_handler)->pPes_handler->pPesPktDecoder )
                        {
                            _tspa_destroy_pid_handler(pTspaDev, (*ppEs_pid_handler));
                            *ppEs_pid_handler = _tspa_create_pid_handler(esPid, TSPA_PID_PES, pTspaDev->bEnableEsPID);
                        }

                        if( !(*ppEs_pid_handler)->pPes_handler->pPesPktDecoder )
                        {
                            PES_STREAM_MBOX         pesStreamMbox = {0};

                            pesStreamMbox.pes_stream_mbox_arg.pes_stream_id           = PES_STREAM_VIDEO;
                            pesStreamMbox.pes_stream_mbox_arg.arg.v.esPID             = esPid;
                            pesStreamMbox.pes_stream_mbox_arg.arg.v.pTunnelInfo       = (void*)pTspaDev;
                            pesStreamMbox.pes_stream_mbox_arg.arg.v.video_stream_type = stream_type;
                            pesStreamMbox.func                                        = _tspa_pes_info_recv;
                            pes_pkt_create_decoder(&((*ppEs_pid_handler)->pPes_handler->pPesPktDecoder), &pesStreamMbox, 0);
                        }
                    }
                    break;

                case ISO_IEC_11172_3_AUDIO:   // mpeg1 audio
                case ISO_IEC_13818_3_AUDIO:   // mpeg2 audio
                case ISO_IEC_13818_7_AUDIO:   // aac adts
                case ISO_IEC_14496_3_AUDIO:   // aac latm
                    if( pTspaDev->bEnableEsPID == true )
                    {
                        if( (*ppEs_pid_handler)->pPes_handler &&
                            (*ppEs_pid_handler)->pPes_handler->pPesPktDecoder )
                        {
                            _tspa_destroy_pid_handler(pTspaDev, (*ppEs_pid_handler));
                            *ppEs_pid_handler = _tspa_create_pid_handler(esPid, TSPA_PID_PES, pTspaDev->bEnableEsPID);
                        }

                        if( !(*ppEs_pid_handler)->pPes_handler->pPesPktDecoder )
                        {
                            PES_STREAM_MBOX         pesStreamMbox = {0};

                            pesStreamMbox.pes_stream_mbox_arg.pes_stream_id     = PES_STREAM_AUDIO;
                            pesStreamMbox.pes_stream_mbox_arg.arg.a.esPID       = esPid;
                            pesStreamMbox.pes_stream_mbox_arg.arg.a.pTunnelInfo = (void*)pTspaDev;
                            pes_pkt_create_decoder(&((*ppEs_pid_handler)->pPes_handler->pPesPktDecoder), &pesStreamMbox, 0);
                        }
                    }
                    break;

                default:
                    pDescriptor = pCurEsInfo->ptFirstDescriptor;
                    while( pDescriptor )
                    {
                        if( 0x59 == pDescriptor->descriptor_tag   // subtitling_descriptor
                         && ISO_IEC_13818_1_PES == stream_type )    // mpeg2 private data
                        {
                            if( pTspaDev->bEnableEsPID == true )
                            {
                                if( (*ppEs_pid_handler)->pPes_handler &&
                                    (*ppEs_pid_handler)->pPes_handler->pPesPktDecoder )
                                {
                                    _tspa_destroy_pid_handler(pTspaDev, (*ppEs_pid_handler));
                                    *ppEs_pid_handler = _tspa_create_pid_handler(esPid, TSPA_PID_PES, pTspaDev->bEnableEsPID);
                                }

                                if( !(*ppEs_pid_handler)->pPes_handler->pPesPktDecoder )
                                {
                                    PES_STREAM_MBOX         pesStreamMbox = {0};

                                    pesStreamMbox.pes_stream_mbox_arg.pes_stream_id     = PES_STREAM_SUBTITLE;
                                    pesStreamMbox.pes_stream_mbox_arg.arg.s.esPID       = esPid;
                                    pesStreamMbox.pes_stream_mbox_arg.arg.s.pTunnelInfo = (void*)pTspaDev;
                                    pes_pkt_create_decoder(&((*ppEs_pid_handler)->pPes_handler->pPesPktDecoder), &pesStreamMbox, 0);
                                }
                            }
                            break;
                        }
                        else if (0x56 == pDescriptor->descriptor_tag   // teletext_descriptor
                              && ISO_IEC_13818_1_PES == stream_type)    // mpeg2 private data
                        {
                            if( pTspaDev->bEnableEsPID == true )
                            {
                                if( (*ppEs_pid_handler)->pPes_handler &&
                                    (*ppEs_pid_handler)->pPes_handler->pPesPktDecoder )
                                {
                                    _tspa_destroy_pid_handler(pTspaDev, (*ppEs_pid_handler));
                                    *ppEs_pid_handler = _tspa_create_pid_handler(esPid, TSPA_PID_PES, pTspaDev->bEnableEsPID);
                                }

                                if( !(*ppEs_pid_handler)->pPes_handler->pPesPktDecoder )
                                {
                                    PES_STREAM_MBOX         pesStreamMbox = {0};

                                    pesStreamMbox.pes_stream_mbox_arg.pes_stream_id     = PES_STREAM_TELETEXT;
                                    pesStreamMbox.pes_stream_mbox_arg.arg.t.esPID       = esPid;
                                    pesStreamMbox.pes_stream_mbox_arg.arg.t.pTunnelInfo = (void*)pTspaDev;
                                    pes_pkt_create_decoder(&((*ppEs_pid_handler)->pPes_handler->pPesPktDecoder), &pesStreamMbox, 0);
                                }
                            }
                            break;
                        }
                        else if ( 0x6A == pDescriptor->descriptor_tag   //AC3_descriptor
                               || 0x7A == pDescriptor->descriptor_tag ) // enhanced_AC3_descriptor
                        {
                            audioAc3ChkNum++;
                        }
                        else if ( 0x50 == pDescriptor->descriptor_tag ) // component_descriptor
                        {
                            if( 0x04 == (*pDescriptor->pPayload & 0x0F) ) // reserved for AC3
                                audioAc3ChkNum++;
                        }
                        else if ( 0x5 == pDescriptor->descriptor_tag    // registration_descriptor
                               && pDescriptor->descriptor_length >= 4 )
                        {
                            uint32_t    format_identifier = 0;

                            if( !strncmp(pDescriptor->pPayload, "AC-3", strlen("AC-3")) )
                                audioAc3ChkNum++;
                        }

                        pDescriptor = pDescriptor->ptNextDescriptor;
                    }

                    if( audioAc3ChkNum && pTspaDev->bEnableEsPID == true )
                    {
                        if( (*ppEs_pid_handler)->pPes_handler &&
                            (*ppEs_pid_handler)->pPes_handler->pPesPktDecoder )
                        {
                            _tspa_destroy_pid_handler(pTspaDev, (*ppEs_pid_handler));
                            *ppEs_pid_handler = _tspa_create_pid_handler(esPid, TSPA_PID_PES, pTspaDev->bEnableEsPID);
                        }

                        if( !(*ppEs_pid_handler)->pPes_handler->pPesPktDecoder )
                        {
                            PES_STREAM_MBOX         pesStreamMbox = {0};

                            pesStreamMbox.pes_stream_mbox_arg.pes_stream_id     = PES_STREAM_AUDIO;
                            pesStreamMbox.pes_stream_mbox_arg.arg.a.esPID       = esPid;
                            pesStreamMbox.pes_stream_mbox_arg.arg.a.pTunnelInfo = (void*)pTspaDev;
                            pes_pkt_create_decoder(&((*ppEs_pid_handler)->pPes_handler->pPesPktDecoder), &pesStreamMbox, 0);

                            audioAc3ChkNum = 0;
                        }
                    }
                    break;
            }
#endif

            pCurPmtSectInfo->pEsPid[pCurPmtSectInfo->totalEsPidCount++] = esPid;
        }

        pCurPmtSectInfo->version_number = pPsi_Pmt_Info->version_number;
        pPsi_Pmt_Info->pid = curPmtPid;

        //------------------------------------
        // callback with user mbox
        if( pTspaDev->pmt_user_mbox.func )
        {
            pTspaDev->pmt_user_mbox.tspa_user_mbox_arg.arg.pmt.pPmt_info = pPsi_Pmt_Info;
            pTspaDev->pmt_user_mbox.func(&pTspaDev->pmt_user_mbox.tspa_user_mbox_arg, extraData);
        }

        {
            PSI_TABLE_MBOX  psiTableMbox = {0};

            psiTableMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PMT;
            psiTableMbox.psi_table_mbox_arg.argv.pmt.pPsi_Pmt_Info = pPsi_Pmt_Info;
            psi_pkt_clear_table(&psiTableMbox, extraData);
        }
    }while(0);

    return result;
}

static uint32_t
_tspa_pat_info_recv(
    PSI_TABLE_MBOX_ARG  *pPsi_table_mbox_arg,
    void                *extraData)
{
    uint32_t        result = 0;
    TS_PACKET_ANAL  *pTspaDev = (TS_PACKET_ANAL*)pPsi_table_mbox_arg->argv.pat.pTunnelInfo;
    PSI_PAT_INFO    *pPsi_Pat_Info = (PSI_PAT_INFO*)pPsi_table_mbox_arg->argv.pat.pPsi_Pat_Info;

    do{
        TSPA_PID            *pPat_pid_handler = 0;
        PSI_PAT_PROGRAM     *ptCurProgram = 0;
        uint32_t            i;

        if( !pPsi_table_mbox_arg )      break;

        if( !pTspaDev || !pPsi_Pat_Info ||
            pTspaDev->bWaitNitReady == true ||
            pTspaDev->bReceiveSdt == false )
        {
            if( pTspaDev && pPsi_Pat_Info &&
                pTspaDev->bReceivePat == false )
            {
                // call _TSDEMUX_PatCallBack()
                // to set pTspaDev->bReceivePat = MMP_TRUE
                if( pTspaDev->pat_user_mbox.func )
                {
                    pTspaDev->pat_user_mbox.tspa_user_mbox_arg.arg.pat.pPat_info = pPsi_Pat_Info;
                    pTspaDev->pat_user_mbox.func(&pTspaDev->pat_user_mbox.tspa_user_mbox_arg, 0);
                }
            }

            if( pPsi_Pat_Info )
            {
                PSI_TABLE_MBOX  psiTableMbox = {0};

                psiTableMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PAT;
                psiTableMbox.psi_table_mbox_arg.argv.pat.pPsi_Pat_Info = pPsi_Pat_Info;
                psi_pkt_clear_table(&psiTableMbox, extraData);
            }

            break;
        }

        pPat_pid_handler = pTspaDev->pTsPid_handler[PAT_PID];

        //------------------------------------
        // check if PAT is not applied (current_next_indicator == 0)
        // or if PAT content is not updated (the same version_number)
        if( (pPat_pid_handler->pPsi_handler->version_number != INVALID_VERSION_NUMBER) &&
            (pTspaDev->transport_stream_id == pPsi_Pat_Info->transport_stream_id) &&
            (pPsi_Pat_Info->current_next_indicator == 0 ||
             pPsi_Pat_Info->version_number == pPat_pid_handler->pPsi_handler->version_number) )
        {
            PSI_TABLE_MBOX  psiTableMbox = {0};

            psiTableMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PAT;
            psiTableMbox.psi_table_mbox_arg.argv.pat.pPsi_Pat_Info = pPsi_Pat_Info;
            psi_pkt_clear_table(&psiTableMbox, extraData);
            break;
        }

        //------------------------------------
        // SDT must ready before PAT to valid transport_stream_id
        if( (pTspaDev->bReceiveSdt == false) )
        {
            // SDT is not ready
            PSI_TABLE_MBOX  psiTableMbox = {0};

            psiTableMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PAT;
            psiTableMbox.psi_table_mbox_arg.argv.pat.pPsi_Pat_Info = pPsi_Pat_Info;
            psi_pkt_clear_table(&psiTableMbox, extraData);
            break;
        }

        if( pTspaDev->transport_stream_id == INVALID_TRANSPORT_STREAM_ID )
        {
            // no SDT, maybe wait for SDT but timeout
            // apply PAT transport_stream_id
            pTspaDev->transport_stream_id = pPsi_Pat_Info->transport_stream_id;
        }
        else if( pPsi_Pat_Info->transport_stream_id != pTspaDev->transport_stream_id )
        {
            // PAT transport_stream_id is different from SDT, destroy PAT
            PSI_TABLE_MBOX  psiTableMbox = {0};

            psiTableMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PAT;
            psiTableMbox.psi_table_mbox_arg.argv.pat.pPsi_Pat_Info = pPsi_Pat_Info;
            psi_pkt_clear_table(&psiTableMbox, extraData);
            break;
        }

        //------------------------------------
        // clean old PMT TS_PID entity that doesn't match updated PAT
        if( pPat_pid_handler->pPsi_handler->table.pat.pPmtPid )
        {
            bool        bLoop_break = false;
            uint32_t    totalPmtPidCount = pPat_pid_handler->pPsi_handler->table.pat.totalPmtPidCount;
            for(i = 0; i < totalPmtPidCount; i++)
            {
                uint32_t        oldPmtPid = pPat_pid_handler->pPsi_handler->table.pat.pPmtPid[i];
                TSPA_PID        *pOld_pmt_pid_handler = pTspaDev->pTsPid_handler[oldPmtPid];
                TSPA_PMT_SECT   *ptExaminePmt = 0;
                bool            bKeep = false;

                bLoop_break = false;
                switch( oldPmtPid )
                {
                    case TDT_TOT_PID:   break;
                    case EIT_PID:       break;
                    case NIT_PID:       break;
                    case SDT_PID:       break;
                    case PAT_PID:       break;

                    default:
                        if( pOld_pmt_pid_handler )
                        {
                            for(ptCurProgram = pPsi_Pat_Info->pFirstProgram;
                                (bKeep == false) && (ptCurProgram);
                                ptCurProgram = ptCurProgram->pNextProgram)
                            {
                                // program number 0 means NIT PID, skip it
                                if( ptCurProgram->program_number == 0 )
                                    continue;

                                // check if PMT has existed
                                ptExaminePmt = &pOld_pmt_pid_handler->pPsi_handler->table.pmt;
                                for(; ptExaminePmt; ptExaminePmt = ptExaminePmt->pNextPmt)
                                {
                                    if( ptCurProgram->program_map_PID == pOld_pmt_pid_handler->pid &&
                                        ptCurProgram->program_number == ptExaminePmt->program_number)
                                    {
                                        // keep this old TS PID (PMT) data, no need to create again
                                        bKeep = true;
                                        break;
                                    }
                                }
                            }

                            if( bKeep == false )
                            {
                                // can not find any match PMT
                                // clean the old PMT and the elementary stream belongs to the PMT
                                _tspa_destroy_pid_handler(pTspaDev, pOld_pmt_pid_handler);
                                bLoop_break = true;
                            }
                        }
                        break;
                }

                if( bLoop_break == true )    break;
            }

            // always free old PAT table and create a new one
            free(pPat_pid_handler->pPsi_handler->table.pat.pPmtPid);
        }

        pPat_pid_handler->pPsi_handler->table.pat.pPmtPid = 0;
        pPat_pid_handler->pPsi_handler->table.pat.totalPmtPidCount = 0;

        {
            int pmtPidSize = sizeof(uint32_t) * pPsi_Pat_Info->totalProgramCount;

            pPat_pid_handler->pPsi_handler->table.pat.pPmtPid = tspa_malloc(pmtPidSize);
            if( !pPat_pid_handler->pPsi_handler->table.pat.pPmtPid )
            {
                PSI_TABLE_MBOX  psiTableMbox = {0};

                psiTableMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PAT;
                psiTableMbox.psi_table_mbox_arg.argv.pat.pPsi_Pat_Info = pPsi_Pat_Info;
                psi_pkt_clear_table(&psiTableMbox, extraData);
                break;
            }

            memset(pPat_pid_handler->pPsi_handler->table.pat.pPmtPid, 0x0, pmtPidSize);
        }
        //------------------------------------
        // update PAT table and PMT TS_PID entity
        for(ptCurProgram = pPsi_Pat_Info->pFirstProgram;
            (ptCurProgram &&
             pPat_pid_handler->pPsi_handler->table.pat.totalPmtPidCount < pPsi_Pat_Info->totalProgramCount);
            ptCurProgram = ptCurProgram->pNextProgram)
        {
            if( ptCurProgram->program_number )
            {
                uint32_t  pmtPid              = ptCurProgram->program_map_PID;
                uint32_t  pmtProgramNumber    = ptCurProgram->program_number;
                TSPA_PID  **ppPmt_Pid_handler = &pTspaDev->pTsPid_handler[pmtPid];

                if( !(*ppPmt_Pid_handler) )
                {
                    PSI_TABLE_MBOX  psiTableMbox = {0};

                    // create a new PMT data identified by PID
                    (*ppPmt_Pid_handler) = _tspa_create_pid_handler(pmtPid, TSPA_PID_PSI, true);

                    psiTableMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PMT;
                    psiTableMbox.psi_table_mbox_arg.argv.pmt.pTunnelInfo   = (void*)pTspaDev;
                    psiTableMbox.psi_table_mbox_arg.argv.pmt.pmtProgramNum = pmtProgramNumber;
                    psiTableMbox.func                                      = _tspa_pmt_info_recv;
                    psi_pkt_create_decoder(&((*ppPmt_Pid_handler)->pPsi_handler->pPsiPktDecoder), &psiTableMbox, 0);

                    (*ppPmt_Pid_handler)->pPsi_handler->table.pmt.program_number  = pmtProgramNumber;
                    (*ppPmt_Pid_handler)->pPsi_handler->table.pmt.pmtPid          = pmtPid;
                    (*ppPmt_Pid_handler)->pPsi_handler->table.pmt.version_number  = INVALID_VERSION_NUMBER;
                }
                else
                {
                    TSPA_PMT_SECT   *ptExaminePmt     = 0;
                    TSPA_PMT_SECT   *ptLastExaminePmt = 0;
                    TSPA_PMT_SECT   *ptAttachPmt      = 0;

                    // in this statement, found the same PMT PID in PAT
                    // check if it has a new program number
                    // if yes, add into PMT link list

                    // solution for the case of more service define in the same PMT PID
                    // if a new program number with existed PMT PID come in,
                    // add this PMT data in link list
                    if( (*ppPmt_Pid_handler)->pPsi_handler )
                    {
                        for(ptExaminePmt = &((*ppPmt_Pid_handler)->pPsi_handler->table.pmt);
                            ptExaminePmt;
                            ptLastExaminePmt = ptExaminePmt, ptExaminePmt = ptExaminePmt->pNextPmt)
                        {
                            if (ptExaminePmt->program_number == pmtProgramNumber)
                                break;
                        }
                    }

                    // can not found the case of "ptExaminePmt->program_number == pmtProgramNumber"
                    // that means this is a existed PMT with a new program number, add it
                    if( !ptExaminePmt && (*ppPmt_Pid_handler)->pPsi_handler )
                    {
                        ptAttachPmt = tspa_malloc(sizeof(TSPA_PMT_SECT));
                        if( !ptAttachPmt )      break;

                        ptAttachPmt->program_number  = pmtProgramNumber;
                        ptAttachPmt->pmtPid          = pmtPid;
                        ptAttachPmt->totalEsPidCount = 0;
                        ptAttachPmt->version_number  = INVALID_VERSION_NUMBER;
                        ptAttachPmt->pEsPid          = 0;
                        ptAttachPmt->pNextPmt        = 0;

                        ptLastExaminePmt->pNextPmt = ptAttachPmt;
                    }
                }

                // update PAT table
                pPat_pid_handler->pPsi_handler->table.pat.pPmtPid[pPat_pid_handler->pPsi_handler->table.pat.totalPmtPidCount++]
                    = pmtPid;
            }
        }

        //------------------------------------
        // callback with user mbox
        if( pTspaDev->pat_user_mbox.func )
        {
            pTspaDev->pat_user_mbox.tspa_user_mbox_arg.arg.pat.pPat_info = pPsi_Pat_Info;
            pTspaDev->pat_user_mbox.func(&pTspaDev->pat_user_mbox.tspa_user_mbox_arg, extraData);
        }
        else
            pTspaDev->bReceivePat = true;

        pPat_pid_handler->pPsi_handler->version_number = pPsi_Pat_Info->version_number;
        {
            PSI_TABLE_MBOX  psiTableMbox = {0};

            psiTableMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PAT;
            psiTableMbox.psi_table_mbox_arg.argv.pat.pPsi_Pat_Info = pPsi_Pat_Info;
            psi_pkt_clear_table(&psiTableMbox, extraData);
        }
    }while(0);

    return result;
}

static uint32_t
_tspa_sdt_info_recv(
    PSI_TABLE_MBOX_ARG  *pPsi_table_mbox_arg,
    void                *extraData)
{
    uint32_t        result = 0;
    TS_PACKET_ANAL  *pTspaDev = (TS_PACKET_ANAL*)pPsi_table_mbox_arg->argv.sdt.pTunnelInfo;
    PSI_SDT_INFO    *pPsi_Sdt_Info = (PSI_SDT_INFO*)pPsi_table_mbox_arg->argv.sdt.pPsi_Sdt_Info;

    do{
        TSPA_PID            *pSdt_pid_handler = 0;
        PSI_SDT_SERVICE     *pService = 0;

        if( !pPsi_table_mbox_arg )      break;

        if( !pTspaDev || !pPsi_Sdt_Info )
        {
            if( pPsi_Sdt_Info )
            {
                PSI_TABLE_MBOX  psiTableMbox = {0};

                psiTableMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_SDT;
                psiTableMbox.psi_table_mbox_arg.argv.sdt.pPsi_Sdt_Info = pPsi_Sdt_Info;
                psi_pkt_clear_table(&psiTableMbox, extraData);
            }
            break;
        }

        pSdt_pid_handler = pTspaDev->pTsPid_handler[SDT_PID];

        pSdt_pid_handler->pPsi_handler->version_number = pPsi_Sdt_Info->version_number;
        if( pTspaDev->transport_stream_id == INVALID_TRANSPORT_STREAM_ID )
            pTspaDev->transport_stream_id = pPsi_Sdt_Info->transport_stream_id;

        //------------------------------------
        // callback with user mbox
        if( pTspaDev->sdt_user_mbox.func )
        {
            pTspaDev->sdt_user_mbox.tspa_user_mbox_arg.arg.sdt.pSdt_info = pPsi_Sdt_Info;
            pTspaDev->sdt_user_mbox.func(&pTspaDev->sdt_user_mbox.tspa_user_mbox_arg, extraData);
        }
        else
            pTspaDev->bReceiveSdt = true;

        //------------------------------------
        // clear sdt table, keep sdt table at user layer
        {
            PSI_TABLE_MBOX  psiTableMbox = {0};

            psiTableMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_SDT;
            psiTableMbox.psi_table_mbox_arg.argv.sdt.pPsi_Sdt_Info = pPsi_Sdt_Info;
            psi_pkt_clear_table(&psiTableMbox, extraData);
        }

#if 0 // not ready
        //------------------------------------
        // notify temp keeped NIT to upper layer.
        if( pTspaDev->ptTmpActualNit )
        {
            tspa_msg(1, "Notify Temp Actual NIT to upper layer\n");
            if( pTspaDev->nit_user_mbox.func )
            {
                TSPA_USER_MBOX_ARG     user_mbox_arg = {0};

                user_mbox_arg.type              = TSPS_USER_MBOX_TYPE_NIT;
                user_mbox_arg.arg.nit.pNit_info = pTspaDev->ptTmpActualNit;
                // it maybe need to change to pHTspa for callback,To Do:
                user_mbox_arg.arg.sdt.pHTspa    = (void*)pTspaDev;
                pTspaDev->sdt_user_mbox.func(&user_mbox_arg, extraData);
            }
            pTspaDev->bWaitNitReady = false;
            _TS_DestroyTmpActualNIT(pTspaDev);
        }
#endif
    }while(0);

    return result;
}
//=============================================================================
//                Public Function Definition
//=============================================================================
TSE_ERR
tspa_CreateHandle(
    TS_PACKET_ANAL      **ppTspaDev,
    TSPA_SETUP_PARAM    *pSetupParam,
    void                *extraData)
{
    TSE_ERR         result = TSPA_ERR_OK;
    TS_PACKET_ANAL  *pTspaDev = 0;

    _verify_handle(ppTspaDev, result);

    do{
        TSPA_PID        **ppCurPid_handler = 0;
        PSI_TABLE_MBOX  psiTableMbox = {0};

        if( *ppTspaDev != 0 )
        {
            tspa_msg_ex(TSPA_MSG_ERR, " error, Exist tspa handle !!");
            result = TSPA_ERR_INVALID_PARAMETER;
            break;
        }

        if( !pSetupParam )
        {
            tspa_msg_ex(TSPA_MSG_ERR, " error, Need pre-set info !!");
            result = TSPA_ERR_INVALID_PARAMETER;
            break;
        }

        pTspaDev = tspa_malloc(sizeof(TS_PACKET_ANAL));
        if( !pTspaDev )
        {
            tspa_msg_ex(TSPA_MSG_ERR, " error, allocate fail !!");
            result = TSPA_ERR_ALLOCATE_FAIL;
            break;
        }

        memset(pTspaDev, 0x0, sizeof(TS_PACKET_ANAL));

        //--------------------------------
        // init paraments
        pTspaDev->bReceivePat   = !pSetupParam->bInScan;
        pTspaDev->bReceiveSdt   = !pSetupParam->bInScan;
        pTspaDev->bWaitNitReady = pSetupParam->bWaitNit;
        // PES decode not ready(To Do), so disable PES check
#if (TSPA_ENABLE_PES_DECODE)
        pTspaDev->bEnableEsPID  = pSetupParam->bEnableEsPID;
#else
        pTspaDev->bEnableEsPID  = false;
#endif

        if( pSetupParam->bInScan )
            pTspaDev->transport_stream_id = INVALID_TRANSPORT_STREAM_ID;

#if (TSPA_ENABLE_PES_DECODE)
        //--------------------------------
        // register all pes table decoder
        pes_pkt_register_all_decoder();
#endif

        //--------------------------------
        // register all psi table decoder
        psi_pkt_register_all_decoder();

#if (CONFIG_PSI_TABLE_OPR_PAT_DECODER_DESC)
        //--------------------------------
        // Init PAT handler
        ppCurPid_handler = &pTspaDev->pTsPid_handler[PAT_PID];
        (*ppCurPid_handler) = _tspa_create_pid_handler(PAT_PID, TSPA_PID_PSI, true);

        psiTableMbox.psi_table_mbox_arg.psi_table_id         = PSI_TABLE_PAT;
        psiTableMbox.psi_table_mbox_arg.argv.pat.pTunnelInfo = (void*)pTspaDev;
        psiTableMbox.func                                    = _tspa_pat_info_recv;
        psi_pkt_create_decoder(&((*ppCurPid_handler)->pPsi_handler->pPsiPktDecoder), &psiTableMbox, 0);
#endif

#if (CONFIG_PSI_TABLE_OPR_SDT_DECODER_DESC)
        //--------------------------------
        // Init SDT handler
        ppCurPid_handler = &pTspaDev->pTsPid_handler[SDT_PID];
        (*ppCurPid_handler) = _tspa_create_pid_handler(SDT_PID, TSPA_PID_PSI, true);

        psiTableMbox.psi_table_mbox_arg.psi_table_id         = PSI_TABLE_SDT;
        psiTableMbox.psi_table_mbox_arg.argv.pat.pTunnelInfo = (void*)pTspaDev;
        psiTableMbox.func                                    = _tspa_sdt_info_recv;
        psi_pkt_create_decoder(&((*ppCurPid_handler)->pPsi_handler->pPsiPktDecoder), &psiTableMbox, 0);
#endif

        (*ppTspaDev) = pTspaDev;
    }while(0);

    if( result != TSPA_ERR_OK )
    {
        tspa_msg_ex(TSPA_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    return result;
}

TSE_ERR
tspa_DestroyHandle(
    TS_PACKET_ANAL      **ppTspaDev,
    void                *extraData)
{
    TSE_ERR     result = TSPA_ERR_OK;

    _trace_enter(TSE_MSG_TRACE_TSAP, "0x%x\n", ppTspaDev);

    _verify_handle(ppTspaDev, result);
    _verify_handle((*ppTspaDev), result);

    do{
        uint32_t        i;
        TS_PACKET_ANAL  *pTspaDev = (*ppTspaDev);

        // Destroy PAT handler
        if( pTspaDev->pTsPid_handler[PAT_PID] )
            _tspa_destroy_pid_handler(pTspaDev, pTspaDev->pTsPid_handler[PAT_PID]);

        // Destroy SDT handler
        if( pTspaDev->pTsPid_handler[SDT_PID] )
            _tspa_destroy_pid_handler(pTspaDev, pTspaDev->pTsPid_handler[SDT_PID]);

        // Destroy all other PIDs which are existed.
        for(i = 0; i < TSPA_MAX_PID_NUM; i++)
        {
            if( pTspaDev->pTsPid_handler[i] )
                _tspa_destroy_pid_handler(pTspaDev, pTspaDev->pTsPid_handler[i]);
        }

        free(pTspaDev);
        (*ppTspaDev) = 0;
    }while(0);

    _trace_leave(TSE_MSG_TRACE_TSAP);
    return result;
}

TSE_ERR
tspa_Analysis(
    TS_PACKET_ANAL      *pTspaDev,
    uint8_t             *pPacketBuf,
    void                *extraData)
{
#define _GET_PID(pData)     ((uint32_t)(((pData[1] & 0x1f) << 8) | pData[2]))

    TSE_ERR     result = TSPA_ERR_OK;

    _trace_enter(TSE_MSG_TRACE_TSAP, "0x%x, 0x%x\n", pTspaDev, extraData);

    _verify_handle(pTspaDev, result);
    _verify_handle(pPacketBuf, result);

    // _mutex_lock(TSE_MSG_TRACE_TSAP, pTseDev->tse_mutex);

    do{
        TSPA_PID    *pCur_pid_handler = 0;

        //-----------------------------------------
        //printf(" curr Pid = %d\n", _GET_PID(pPacketBuf));
        uint32_t    validated_pid = SDT_PID; //PAT_PID; //SDT_PID;
        uint32_t    cur_pid = _GET_PID(pPacketBuf);
        if( 0)//_GET_PID(pPacketBuf) == validated_pid )
        {
            static uint32_t     cnt = 0;
            printf(" get pid = 0x%x, %d-th\n", _GET_PID(pPacketBuf), ++cnt);
        }
        //-----------------------------------------
        pCur_pid_handler = pTspaDev->pTsPid_handler[_GET_PID(pPacketBuf)];
        if( pCur_pid_handler )
        {
            pCur_pid_handler->proportion++;
            if( pCur_pid_handler->bValid == false )
                break;

            if( pCur_pid_handler->pPsi_handler &&
                pCur_pid_handler->pPsi_handler->pPsiPktDecoder )
            {
                psi_pkt_decode(pCur_pid_handler->pPsi_handler->pPsiPktDecoder, pPacketBuf);
            }
            else if( pCur_pid_handler->pPes_handler &&
                     pCur_pid_handler->pPes_handler->pPesPktDecoder )
            {
            #if (TSPA_ENABLE_PES_DECODE)
                pes_pkt_decode(pCur_pid_handler->pPes_handler->pPesPktDecoder, pPacketBuf);
            #endif
            }
        }
    }while(0);

    if( result != TSPA_ERR_OK )
    {
        tspa_msg_ex(TSPA_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    // _mutex_unlock(TSE_MSG_TRACE_TSAP, pTseDev->tse_mutex);
    _trace_leave(TSE_MSG_TRACE_TSAP);
    return result;
}


// TSE_ERR
// tspa_template(
//     TS_PACKET_ANAL      *pTspaDev,
//     void                *extraData)
// {
//     TSE_ERR     result = TSPA_ERR_OK;
//
//     _trace_enter(TSE_MSG_TRACE_TSAP, "0x%x, 0x%x\n", pTspaDev, extraData);
//     _verify_handle(pTspaDev, result);
//
//     // _mutex_lock(TSE_MSG_TRACE_TSAP, pTseDev->tse_mutex);
//
//     do{
//     }while(0);
//
//     if( result != TSPA_ERR_OK )
//     {
//         tspa_msg_ex(TSPA_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
//     }
//     // _mutex_unlock(TSE_MSG_TRACE_TSAP, pTseDev->tse_mutex);
//     _trace_leave(TSE_MSG_TRACE_TSAP);
//     return result;
// }





