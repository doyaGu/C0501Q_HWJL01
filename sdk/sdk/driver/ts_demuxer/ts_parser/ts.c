/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file ts.c
 * Used to provide transport stream operations
 * @author I-Chun Lai
 * @version 0.1
 */


#include "ts.h"
//#include "dtv_mem.h"
//#include "iso8859.h"
//#include "demod_control.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define INVALID_VERSION_NUMBER  ((uint32)(-1))

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static TS_DEMUX*
_TS_CreateTsDemux(
    void);

static void
_TS_DestroyTsDemux(
    TS_DEMUX* ptTsDemux);

static TS_PID*
_TS_CreateTsPid(
    MMP_UINT32  PID,
    TS_PID_TYPE pidType,
    MMP_BOOL    bPidValid);

static void
_TS_DestroyTsPid(
    TS_DEMUX*   ptDemux,
    TS_PID*     ptTsPid);

static MMP_INLINE void
_TS_CleanPatPid(
    TS_DEMUX*   ptDemux,
    TS_PID*     ptPatPid);

static MMP_INLINE void
_TS_CleanPmtPid(
    TS_DEMUX*   ptDemux,
    TS_PID*     ptPmtPid);

static void
_TS_PatCallBack(
    TS_DEMUX*       ptDemux,
    PSI_PAT_INFO*   ptPatInfo);

static void
_TS_PmtCallBack(
    TS_DEMUX*       ptDemux,
    PSI_PMT_INFO*   ptPmtInfo);

static void
_TS_PesCallBack(
    TS_DEMUX*   ptDemux,
    PES_INFO*   ptPesInfo);

static void
_TS_NewSubtable(
    TS_DEMUX*       ptDemux,
    PSI_DECODER*    ptDecoder,
    MMP_UINT32      table_id,
    MMP_UINT32      table_id_extension);

static void
_TS_NitCallBack(
    TS_DEMUX*       ptDemux,
    PSI_NIT_INFO*   ptNitInfo);

static void
_TS_SdtCallBack(
    TS_DEMUX*       ptDemux,
    PSI_SDT_INFO*   ptSdtInfo);

static MMP_BOOL
_TS_EitSectionFilterCallBack(
    TS_DEMUX*       ptDemux,
    PSI_SECTION*    ptSection);

static void
_TS_EitPresentFollowingCallBack(
    TS_DEMUX*       ptDemux,
    PSI_EIT_INFO*   ptEitInfo);

static void
_TS_EitScheduleCallBack(
    TS_DEMUX*       ptDemux,
    PSI_EIT_INFO*   ptEitInfo);

static void
_TS_TdtCallBack(
    TS_DEMUX*       ptDemux,
    PSI_MJDBCD_TIME tUtcTime);

static void
_TS_TotCallBack(
    TS_DEMUX*       ptDemux,
    PSI_TOT_INFO*   ptTotInfo);

static MMP_INLINE MMP_UINT
_TS_GetPid(
    MMP_UINT8*  pData);

static void
_TS_CreateTmpActualNIT(
    TS_DEMUX*       ptDemux,
    PSI_NIT_INFO*   ptNitInfo);

static void
_TS_DestroyTmpActualNIT(
    TS_DEMUX*       ptDemux);

#ifdef ENABLE_DSM_CC
static void
_TS_DsmCcCallBack(
    TS_DEMUX*       ptDemux,
    DSM_CC_INFO*    ptDsmCcInfo);
#endif

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * TS decoder initialing invokes creating TS_DEMUX structure and hanging
 * particular table handlers (callback functions return after decoding).
 *
 * @param   bWaitNit    Whether if the TS_DEMUX need to wait NIT ready or not.
 * @param   bInScan     Whether if the TS_DEMUX in scan stage.
 * @param   bCollectEit Wether collect EIT table.
 * @return  TS_DEMUX*   structure pointer to store pid data.
 */
//=============================================================================
TS_DEMUX*
TS_Init(
    MMP_BOOL bWaitNit,
    MMP_BOOL bInScan,
    MMP_BOOL bCollectEit)
{
    TS_DEMUX*   ptDemux     = MMP_NULL;
    TS_PID**    pptPid      = MMP_NULL;

    ptDemux = _TS_CreateTsDemux();

    if (MMP_NULL == ptDemux)
        return MMP_NULL;

    ptDemux->bWaitNitReady = bWaitNit;
    ptDemux->bReceivePat = !bInScan;
    ptDemux->bReceiveSdt = !bInScan;
    ptDemux->bEnableEsPID = MMP_FALSE;

    if (bInScan)
        ptDemux->transport_stream_id = INVALID_TRANSPORT_STREAM_ID;

    // Init PAT handler
    pptPid  = &ptDemux->ptTsPid[PAT_PID];
    *pptPid = _TS_CreateTsPid(PAT_PID, TS_PID_PSI, MMP_TRUE);
    (*pptPid)->ptPsi->ptPsiDecoder =
        psiTablePAT_AttachDecoder((PSI_PAT_CALLBACK)_TS_PatCallBack,
                                  ptDemux);

#ifdef ENABLE_ISDB_T_ONE_SEG
    pptPid  = &ptDemux->ptTsPid[0x1FC8];
    *pptPid = _TS_CreateTsPid(0x1FC8, TS_PID_PSI, MMP_TRUE);
    (*pptPid)->ptPsi->table.PMT.program_number = 0x580;
    (*pptPid)->ptPsi->table.PMT.totalEsPidCount = 0;
    (*pptPid)->ptPsi->table.PMT.pmtPid = 0x1FC8;
    (*pptPid)->ptPsi->table.PMT.version_number = INVALID_VERSION_NUMBER;
    (*pptPid)->ptPsi->table.PMT.pEsPid = MMP_NULL;
    (*pptPid)->ptPsi->table.PMT.ptNextPmt = MMP_NULL;
    (*pptPid)->ptPsi->ptPsiDecoder =
        psiTablePMT_AttachDecoder(0x580,
                                  (PSI_PMT_CALLBACK)_TS_PmtCallBack,
                                  ptDemux);
    DemodCtrl_UpdatePidTable(0, 0x1FC8, PID_PMT_INDEX);
#endif

    // Init NIT handler
//    if (ptDemux->bWaitNitReady)
    {
        pptPid  = &ptDemux->ptTsPid[NIT_PID];
        *pptPid = _TS_CreateTsPid(NIT_PID, TS_PID_PSI, MMP_TRUE);
        (*pptPid)->ptPsi->ptPsiDecoder =
            psiTableDemux_AttachDemux((PSI_DEMUX_CALLBACK)_TS_NewSubtable,
                                      ptDemux);
    }
    // Init SDT handler
    pptPid  = &ptDemux->ptTsPid[SDT_PID];
    *pptPid = _TS_CreateTsPid(SDT_PID, TS_PID_PSI, MMP_TRUE);
    (*pptPid)->ptPsi->ptPsiDecoder =
        psiTableDemux_AttachDemux((PSI_DEMUX_CALLBACK)_TS_NewSubtable,
                                  ptDemux);

    // Init EIT handler
    //pptPid  = &ptDemux->ptTsPid[EIT_PID];
    //*pptPid = _TS_CreateTsPid(EIT_PID, MMP_TRUE, MMP_TRUE);
    //(*pptPid)->ptPsi->ptPsiDecoder =
    //    psiTableDemux_AttachDemux((PSI_DEMUX_CALLBACK)_TS_NewSubtable,
    //                              ptDemux);
    if (bCollectEit)
        TS_CreateEitHandle(ptDemux);

    // Init TDT handler
    pptPid  = &ptDemux->ptTsPid[TDT_TOT_PID];
    *pptPid = _TS_CreateTsPid(TDT_TOT_PID, TS_PID_PSI, MMP_TRUE);
    (*pptPid)->ptPsi->ptPsiDecoder =
        psiTableDemux_AttachDemux((PSI_DEMUX_CALLBACK)_TS_NewSubtable,
                                  ptDemux);

    return ptDemux;
}

//=============================================================================
/**
 * TS decoder terminating invokes destroying TS_DEMUX structure.
 *
 * @param   ptDemux     Obtain TS_DEMUX* structure pointer to destroy it.
 * @return  none.
 */
//=============================================================================
void
TS_Terminate(
    TS_DEMUX*   ptDemux)
{
    if (MMP_NULL != ptDemux)
    {
        _TS_DestroyTsDemux(ptDemux);
    }
}

//=============================================================================
/**
 * Release the whole resource allocated by EIT decoder.
 *
 * @param   ptDemux     The running demux decoder pointer.
 * @return  none.
 */
//=============================================================================
void
TS_ReCreateEitHandle(
    TS_DEMUX*   ptDemux)
{
    TS_PID** pptPid = MMP_NULL;

    if (TS_DestroyEitHandle(ptDemux))
        TS_CreateEitHandle(ptDemux);
}

void
TS_CreateEitHandle(
    TS_DEMUX*   ptDemux)
{
    TS_PID** pptPid = MMP_NULL;

    if (MMP_NULL == ptDemux->ptTsPid[EIT_PID])
    {
        // Init EIT handler
        pptPid = &ptDemux->ptTsPid[EIT_PID];
        *pptPid = _TS_CreateTsPid(EIT_PID, TS_PID_PSI, MMP_TRUE);
        (*pptPid)->ptPsi->ptPsiDecoder =
            psiTableDemux_AttachDemux((PSI_DEMUX_CALLBACK)_TS_NewSubtable, ptDemux);
    }
}

MMP_BOOL
TS_DestroyEitHandle(
    TS_DEMUX*   ptDemux)
{
    MMP_BOOL result = MMP_FALSE;

    if (MMP_NULL != ptDemux->ptTsPid[EIT_PID])
    {
        _TS_DestroyTsPid(ptDemux, ptDemux->ptTsPid[EIT_PID]);
        result = MMP_TRUE;
    }

    return result;
}

//=============================================================================
/**
 * TS decoder decoding invokes dispatching transport stream packet data to
 * appropriate decoder (psi and pes decoders depend on pid).
 *
 * @param   ptDemux     Get particular table handler by pid in TS_DEMUX
 *                      structure.
 * @param   pData       Transport stream packet data input. Suppose it should
 *                      be 188 bytes and the first byte is 0x47 (sync byte).
 * @return  MMP_BOOL to indicate error or not.
 */
//=============================================================================
MMP_BOOL
TS_Decode(
    TS_DEMUX*   ptDemux,
    MMP_UINT8*  pData)
{
    TS_PID*     ptTsPid = MMP_NULL;

    if ((MMP_NULL != ptDemux) && (MMP_NULL != pData))
    {
        ptTsPid = ptDemux->ptTsPid[_TS_GetPid(pData)];

        if (MMP_NULL != ptTsPid)
        {
            if (MMP_TRUE == ptTsPid->bValid
             && MMP_NULL != ptTsPid->ptPsi
             && MMP_NULL != ptTsPid->ptPsi->ptPsiDecoder)
            {
                psiPacket_DecodePacket(ptTsPid->ptPsi->ptPsiDecoder,
                                       pData);
            }
            else if (MMP_TRUE == ptTsPid->bValid
                  && MMP_NULL != ptTsPid->ptPes
                  && MMP_NULL != ptTsPid->ptPes->ptPesDecoder)
            {
                pesPacket_DecodePacket(ptTsPid->ptPes->ptPesDecoder, pData);
            }
#ifdef ENABLE_DSM_CC
            else if (MMP_TRUE == ptTsPid->bValid
                  && MMP_NULL != ptTsPid->ptDsmCc
                  && MMP_NULL != ptTsPid->ptDsmCc->ptPsiDecoder)
            {
                psiPacket_DecodePacket(ptTsPid->ptDsmCc->ptPsiDecoder,
                                       pData);
            }
#endif
        }
    }
    return MMP_TRUE;
}

//=============================================================================
/**
 * Insert the valid transport_stream_id for the TS_DEMUX system.
 *
 * @param   ptTsDemux           TS_DEMUX structure that created pid data will be insert
 *                              into.
 * @param   transport_stream_id The valid transport_stream_id for the TS_DEMUX
 *                              system.
 * @param   original_network_id The valid originla_network_id for the TS_DEMUX
 * @return  MMP_BOOL to indicate error or not.
 */
//=============================================================================
MMP_BOOL
TS_SetValidTransportStream(
    TS_DEMUX*           ptTsDemux,
    MMP_UINT32          transport_stream_id,
    MMP_UINT32          original_network_id)
{
    if (MMP_NULL == ptTsDemux || 0 == transport_stream_id)
        return MMP_FALSE;

    ptTsDemux->bReceiveSdt = MMP_FALSE;
    ptTsDemux->transport_stream_id = transport_stream_id;
    ptTsDemux->original_network_id = original_network_id;
    return MMP_TRUE;
}

//=============================================================================
/**
 * Usually use in known pid of elementary stream to insert into TS_DEMUX
 * structure, hang handler for callback.
 *
 * @param   ptTsDemux   TS_DEMUX structure that created pid data will be insert
 *                      into.
 * @param   PID         Specified pid value.
 * @param   bPCR_PID    Whether the PID is PCR_PID or not.
 * @return  MMP_BOOL to indicate error or not.
 */
//=============================================================================
MMP_BOOL
TS_InsertEsPid(
    TS_DEMUX*       ptTsDemux,
    uint            channelId,
    PES_PID_TYPE    pesIndex,
    PID_INFO*       ptPidInfo,
    uint8*          pRiscBaseAddr)
{
    TS_PID**    pptPid  = MMP_NULL;
    uint32      pid;

    if( !(channelId < MAX_CHANNEL_COUNT
     || pesIndex < MAX_PID_COUNT_PER_CHANNEL
     || MMP_NULL == ptPidInfo
     || MMP_NULL == ptTsDemux) )
        return MMP_FALSE;

    // Init PES handler
    pid = ptPidInfo->pid;
    pptPid  = &ptTsDemux->ptTsPid[pid];

    if (*pptPid)
        return MMP_FALSE;

    *pptPid = _TS_CreateTsPid(pid, TS_PID_PES, MMP_FALSE);

    if (MMP_NULL == *pptPid)
        return MMP_FALSE;

    (*pptPid)->ptPes->ptPesDecoder =
       pesPacket_AttachDecoder(channelId,
                               pesIndex,
                               ptPidInfo,
                               pRiscBaseAddr,
                               (PES_CALLBACK)_TS_PesCallBack,
                               ptTsDemux);

    return MMP_TRUE;
}

//=============================================================================
/**
 * Use to set enable or disable for specified pid.
 *
 * @param   ptTsDemux       Get pid data in TS_DEMUX structure.
 * @param   pid             Specified pid value.
 * @param   bEnable         Enable or disable to be set.
 * @return  none.
 */
//=============================================================================
void
TS_SetPid(
    TS_DEMUX*  ptTsDemux,
    MMP_UINT32 pid,
    MMP_BOOL   bEnable)
{
    if (ptTsDemux->ptTsPid[pid] != MMP_NULL)
        ptTsDemux->ptTsPid[pid]->bValid = bEnable;
}

#ifdef ENABLE_DSM_CC
//=============================================================================
/**
 * Usually use in known pid of elementary stream to insert into TS_DEMUX
 * structure, hang handler for callback.
 *
 * @param   ptTsDemux   TS_DEMUX structure that created pid data will be insert
 *                      into.
 * @param   PID         Specified pid value.
 * @return  MMP_BOOL to indicate error or not.
 */
//=============================================================================
MMP_BOOL
TS_InsertDsmCcPid(
    TS_DEMUX*   ptTsDemux,
    MMP_UINT32  PID)
{
    TS_PID**    pptPid  = MMP_NULL;

    if (MMP_NULL == ptTsDemux)
        return MMP_FALSE;

    // Init DSM_CC handler
    pptPid  = &ptTsDemux->ptTsPid[PID];

    if (*pptPid)
        return MMP_FALSE;

    *pptPid = _TS_CreateTsPid(PID, TS_PID_DSM_CC, MMP_TRUE);
    if (MMP_NULL == *pptPid)
        return MMP_FALSE;
    (*pptPid)->ptDsmCc->ptPsiDecoder =
        DSM_CC_AttachDecoder(
            (DSM_CC_CALLBACK)_TS_DsmCcCallBack,
            ptTsDemux);
    return MMP_TRUE;
}


//=============================================================================
/**
 * Usually destroy certain DsmCc Pid of TS_DEMUX
 *
 * @param   ptTsDemux   TS_DEMUX structure that created pid data will be insert
 *                      into.
 * @param   PID         Specified pid value.
 * @return  none.
 */
//=============================================================================
void
TS_DestroyDsmCcPid(
    TS_DEMUX*   ptTsDemux,
    MMP_UINT32  PID)
{
    if (ptTsDemux->ptTsPid[PID] && ptTsDemux->ptTsPid[PID]->ptDsmCc)
        _TS_DestroyTsPid(ptTsDemux, ptTsDemux->ptTsPid[PID]);
}
#endif

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Create the TS_DEMUX structure.
 *
 * @param none
 * @return TS_DEMUX* structure pointer to store pid data.
 */
//=============================================================================
static TS_DEMUX*
_TS_CreateTsDemux(
    void)
{
    TS_DEMUX* ptDemux = MMP_NULL;

    ptDemux = (TS_DEMUX*)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(TS_DEMUX));
    if (MMP_NULL == ptDemux)
        return MMP_NULL;

    // TS Demux initialization
    PalMemset(ptDemux, 0x0, sizeof(TS_DEMUX));

    return ptDemux;
}

//=============================================================================
/**
 * Destroy the TS_DEMUX structure.
 *
 * @param ptTsDemux     Obtain TS_DEMUX* structure pointer to destroy it.
 * @return none
 */
//=============================================================================
static void
_TS_DestroyTsDemux(
    TS_DEMUX*   ptTsDemux)
{
    MMP_UINT32 i = 0;
    if (MMP_NULL != ptTsDemux)
    {
        // Destroy PAT handler
        if (MMP_NULL != ptTsDemux->ptTsPid[PAT_PID])
            _TS_DestroyTsPid(ptTsDemux, ptTsDemux->ptTsPid[PAT_PID]);

        // Destroy NIT handler
        if (MMP_NULL != ptTsDemux->ptTsPid[NIT_PID])
            _TS_DestroyTsPid(ptTsDemux, ptTsDemux->ptTsPid[NIT_PID]);

        // Destroy SDT handler
        if (MMP_NULL != ptTsDemux->ptTsPid[SDT_PID])
            _TS_DestroyTsPid(ptTsDemux, ptTsDemux->ptTsPid[SDT_PID]);

        // Destroy EIT handler
        //if (MMP_NULL != ptTsDemux->ptTsPid[EIT_PID])
        //    _TS_DestroyTsPid(ptTsDemux, ptTsDemux->ptTsPid[EIT_PID]);
        TS_DestroyEitHandle(ptTsDemux);

        // Destroy TDT handler
        if (MMP_NULL != ptTsDemux->ptTsPid[TDT_TOT_PID])
            _TS_DestroyTsPid(ptTsDemux, ptTsDemux->ptTsPid[TDT_TOT_PID]);

        // Destroy all other PIDs which are existed.
        for (i = 0; i < MAX_PID_NUMBER; i++)
        {
            if (ptTsDemux->ptTsPid[i])
                _TS_DestroyTsPid(ptTsDemux, ptTsDemux->ptTsPid[i]);
        }

        if (ptTsDemux->ptTmpActualNit)
            _TS_DestroyTmpActualNIT(ptTsDemux);

        PalHeapFree(PAL_HEAP_DEFAULT, ptTsDemux);
    }
}

//=============================================================================
/**
 * Create a new TS_PID structure that will be used to store decoder
 * information for each serviceable pid.
 *
 * @param PID           Specified pid value.
 * @param pidType       pid Type of the certain PID
 * @param bPidValid     Set this value MMP_TRUE the specified pid will be
 *                      decoded, otherwise not.
 * @return TS_PID* structure pointer to store decoder information.
 */
//=============================================================================
static TS_PID*
_TS_CreateTsPid(
    MMP_UINT32  PID,
    TS_PID_TYPE pidType,
    MMP_BOOL    bPidValid)
{
    TS_PID* ptTsPid = MMP_NULL;

    ptTsPid = (TS_PID*)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(TS_PID));
    if (MMP_NULL == ptTsPid)
        return MMP_NULL;

    // TS PID initialization
    PalMemset(ptTsPid, 0x0, sizeof(TS_PID));
    ptTsPid->PID = PID;

    if (TS_PID_PSI == pidType)
    {
        ptTsPid->ptPsi = (TS_PSI*)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(TS_PSI));
        if (MMP_NULL == ptTsPid->ptPsi)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, ptTsPid);
            return MMP_NULL;
        }

        // TS PSI initialization
        PalMemset(ptTsPid->ptPsi, 0x0, sizeof(TS_PSI));
        ptTsPid->ptPsi->version_number = INVALID_VERSION_NUMBER;

        if (NIT_PID == PID)
        {
            ptTsPid->ptPsi->table.NIT.actual_version_number = INVALID_VERSION_NUMBER;
            ptTsPid->ptPsi->table.NIT.other_version_number = INVALID_VERSION_NUMBER;
        }
    }
    else if (TS_PID_PES == pidType)
    {
        ptTsPid->ptPes = (TS_PES*)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(TS_PES));
        if (MMP_NULL == ptTsPid->ptPes)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, ptTsPid);
            return MMP_NULL;
        }

        // TS PES initialization
        PalMemset(ptTsPid->ptPes, 0x0, sizeof(TS_PES));
    }
#ifdef ENABLE_DSM_CC
    else if (TS_PID_DSM_CC == pidType)
    {
        ptTsPid->ptDsmCc = (TS_DSM_CC*)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(TS_DSM_CC));
        if (MMP_NULL == ptTsPid->ptDsmCc)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, ptTsPid);
            return MMP_NULL;
        }

        // TS PES initialization
        PalMemset(ptTsPid->ptDsmCc, 0x0, sizeof(TS_DSM_CC));
    }
#endif
    else
    {
        // Invalid Input, throw out error.
        tsp_msg(TSP_MSG_TYPE_ERR, "ts.c(%d), Invalid TS PID type assign: %u\n", __LINE__, pidType);
        PalAssert(0);
    }
    ptTsPid->bValid = bPidValid;

    return ptTsPid;
}

//=============================================================================
/**
 * Destroy the TS_PID entry.
 *
 * @param ptTsPid   Pointer to the TS_PID entry to be destroyed.
 * @return none
 */
//=============================================================================
static void
_TS_DestroyTsPid(
    TS_DEMUX*   ptDemux,
    TS_PID*     ptTsPid)
{
    if (MMP_NULL != ptTsPid)
    {
        MMP_UINT pid = ptTsPid->PID;

        if (MMP_NULL != ptTsPid->ptPsi)
        {
            switch (pid)
            {
            case PAT_PID:
                _TS_CleanPatPid(ptDemux, ptTsPid);
                break;
            case NIT_PID:
            case SDT_PID:
            case EIT_PID:
            case TDT_TOT_PID:
                if (MMP_NULL != ptTsPid->ptPsi->ptPsiDecoder)
                {
                    psiTableDemux_DetachDemux(ptTsPid->ptPsi->ptPsiDecoder);
                }
                break;
            default:    // PMT
                _TS_CleanPmtPid(ptDemux, ptTsPid);
                break;
            }

            PalHeapFree(PAL_HEAP_DEFAULT, ptTsPid->ptPsi);
        }

        if (MMP_NULL != ptTsPid->ptPes)
        {
            if (MMP_NULL != ptTsPid->ptPes->ptPesDecoder)
            {
                pesPacket_DetachDecoder(ptTsPid->ptPes->ptPesDecoder);
            }
            PalHeapFree(PAL_HEAP_DEFAULT, ptTsPid->ptPes);
        }

#ifdef ENABLE_DSM_CC
        if (MMP_NULL != ptTsPid->ptDsmCc)
        {
            if (MMP_NULL != ptTsPid->ptDsmCc->ptPsiDecoder)
            {
                DSM_CC_DetachDecoder(ptTsPid->ptDsmCc->ptPsiDecoder);
            }
            PalHeapFree(PAL_HEAP_DEFAULT, ptTsPid->ptDsmCc);
        }
#endif

        PalHeapFree(PAL_HEAP_DEFAULT, ptTsPid);
        ptDemux->ptTsPid[pid] = MMP_NULL;
    }
}

static MMP_INLINE void
_TS_CleanPatPid(
    TS_DEMUX*   ptDemux,
    TS_PID*     ptPatPid)
{
    if (MMP_NULL != ptPatPid->ptPsi->ptPsiDecoder)
        psiTablePAT_DetachDecoder(ptPatPid->ptPsi->ptPsiDecoder);

    // Clean all the PMTs belong to the PAT
    if (MMP_NULL != ptPatPid->ptPsi->table.PAT.pPmtPid)
    {
        MMP_UINT pmtIndex = 0;
        MMP_UINT totalPmtPidCount = ptPatPid->ptPsi->table.PAT.totalPmtPidCount;
        for (pmtIndex = 0; pmtIndex < totalPmtPidCount; pmtIndex++)
        {
            MMP_UINT32 pmtPid = ptPatPid->ptPsi->table.PAT.pPmtPid[pmtIndex];

            if (ptDemux->ptTsPid[pmtPid])
                _TS_DestroyTsPid(ptDemux, ptDemux->ptTsPid[pmtPid]);
        }

        PalHeapFree(PAL_HEAP_DEFAULT, ptPatPid->ptPsi->table.PAT.pPmtPid);
    }
}

static MMP_INLINE void
_TS_CleanPmtPid(
    TS_DEMUX*   ptDemux,
    TS_PID*     ptPmtPid)
{
    struct PMT_TBL_SECTION* ptFirstPmt = &ptPmtPid->ptPsi->table.PMT;
    struct PMT_TBL_SECTION* ptCurrentPmt = ptFirstPmt;
    struct PMT_TBL_SECTION* ptNextPmt = MMP_NULL;
    if (MMP_NULL != ptPmtPid->ptPsi->ptPsiDecoder)
        psiTablePMT_DetachDecoder(ptPmtPid->ptPsi->ptPsiDecoder);

    // Clean all the elementary streams belong to this PMT
    while (ptCurrentPmt)
    {
        ptNextPmt = ptCurrentPmt->ptNextPmt;
        if (MMP_NULL != ptCurrentPmt->pEsPid)
        {
            if (ptCurrentPmt->totalEsPidCount > 0)
            {
                MMP_UINT esIndex = 0;
                MMP_UINT totalEsPidCount = ptCurrentPmt->totalEsPidCount;
                for (esIndex = 0; esIndex < totalEsPidCount; esIndex++)
                {
                    MMP_UINT32 esPid = ptCurrentPmt->pEsPid[esIndex];
                    _TS_DestroyTsPid(ptDemux,
                                     ptDemux->ptTsPid[esPid]);
                }
            }
            PalHeapFree(PAL_HEAP_DEFAULT, ptCurrentPmt->pEsPid);
        }
        if (ptCurrentPmt != ptFirstPmt)
            PalHeapFree(PAL_HEAP_DEFAULT, ptCurrentPmt);
        ptCurrentPmt = ptNextPmt;
    }
}

// attach to pat decoder, call back this funtion after pat decoding
// to receive pat info
static void
_TS_PatCallBack(
    TS_DEMUX*       ptDemux,
    PSI_PAT_INFO*   ptPatInfo)
{
    TS_PID*             ptPatPid            = MMP_NULL;
    PSI_PAT_PROGRAM*    ptCurrentProgram    = MMP_NULL;
    MMP_UINT i = 0;

    if (MMP_NULL == ptDemux
     || MMP_NULL == ptPatInfo
     || MMP_TRUE == ptDemux->bWaitNitReady
     || MMP_FALSE == ptDemux->bReceiveSdt)
    {
        if (ptDemux)
        {
            // announce upper layer that the bitstream do receive a PAT
            if (MMP_FALSE == ptDemux->bReceivePat)
            {
                if (MMP_NULL != ptPatInfo)
                {
                    // call _TSDEMUX_PatCallBack()
                    // to set ptDemux->bReceivePat = MMP_TRUE
                    if (ptDemux->pfPatCallBack)
                        ptDemux->pfPatCallBack((void*)ptDemux, ptPatInfo);

                    psiTablePAT_DestroyTable(ptPatInfo);
                }
            }
            else
            {
                // have received PAT, ignore it
                if (MMP_NULL != ptPatInfo)
                    psiTablePAT_DestroyTable(ptPatInfo);
            }
        }
        else
        {
            // demux doesn't init
            if (MMP_NULL != ptPatInfo)
                psiTablePAT_DestroyTable(ptPatInfo);
        }
        return;
    }
    ptPatPid = ptDemux->ptTsPid[PAT_PID];

    // check if PAT is not applied (current_next_indicator == 0)
    // or if PAT content is not updated (the same version_number)
    if (INVALID_VERSION_NUMBER != ptPatPid->ptPsi->version_number
     && (0 == ptPatInfo->current_next_indicator || ptPatInfo->version_number == ptPatPid->ptPsi->version_number)
     && ptDemux->transport_stream_id == ptPatInfo->transport_stream_id)
    {
        psiTablePAT_DestroyTable(ptPatInfo);
        return;
    }

    // SDT must ready before PAT to valid transport_stream_id
    if (ptDemux->bReceiveSdt)
    {
        if (INVALID_TRANSPORT_STREAM_ID == ptDemux->transport_stream_id)
        {
            // no SDT, maybe wait for SDT but timeout
            // apply PAT transport_stream_id
            ptDemux->transport_stream_id = ptPatInfo->transport_stream_id;
        }
        else
        {
            // PAT transport_stream_id is different from SDT, destroy PAT
            if (ptPatInfo->transport_stream_id != ptDemux->transport_stream_id)
            {
                psiTablePAT_DestroyTable(ptPatInfo);
                return;
            }
        }
    }
    else
    {
        // SDT is not ready
        psiTablePAT_DestroyTable(ptPatInfo);
        return;
    }

    // clean old PMT TS_PID entity that doesn't match updated PAT
    if (MMP_NULL != ptPatPid->ptPsi->table.PAT.pPmtPid)
    {
        for (i = 0; i < ptPatPid->ptPsi->table.PAT.totalPmtPidCount; i++)
        {
            MMP_BOOL    bKeep       = MMP_FALSE;
            MMP_UINT32  oldPmtPid   = ptPatPid->ptPsi->table.PAT.pPmtPid[i];
            TS_PID*     ptOldPmtPid = ptDemux->ptTsPid[oldPmtPid];
            struct PMT_TBL_SECTION* ptExaminePmt = MMP_NULL;

            // Prevent the PID is one of the static PSI PID
            if (oldPmtPid == PAT_PID || oldPmtPid == NIT_PID
             || oldPmtPid == SDT_PID || oldPmtPid == EIT_PID
             || oldPmtPid == TDT_TOT_PID)
            {
                continue;
            }

            if (ptOldPmtPid)
            {
                for (ptCurrentProgram = ptPatInfo->ptFirstProgram;
                     (bKeep == MMP_FALSE) && (ptCurrentProgram != MMP_NULL);
                     ptCurrentProgram = ptCurrentProgram->ptNextProgram)
                {
                    // program number 0 means NIT PID, skip it
                    if (ptCurrentProgram->program_number == 0)
                        continue;

                    // check if PMT has existed
                    ptExaminePmt = &ptOldPmtPid->ptPsi->table.PMT;
                    for (; ptExaminePmt != MMP_NULL; ptExaminePmt = ptExaminePmt->ptNextPmt)
                    {
                        if (ptCurrentProgram->program_map_PID == ptOldPmtPid->PID
                         && ptCurrentProgram->program_number  == ptExaminePmt->program_number)
                        {
                            // keep this old TS PID (PMT) data, no need to create again
                            bKeep = MMP_TRUE;
                            break;
                        }
                    }
                }

                if (!bKeep)
                {
                    // can not find any match PMT
                    // clean the old PMT and the elementary stream belongs to the PMT
                    _TS_DestroyTsPid(ptDemux, ptOldPmtPid);
                    break;
                }
            }
        }
    }

    // always free old PAT table and create a new one
    if( ptPatPid->ptPsi->table.PAT.pPmtPid )
        PalHeapFree(PAL_HEAP_DEFAULT, ptPatPid->ptPsi->table.PAT.pPmtPid);
    ptPatPid->ptPsi->table.PAT.pPmtPid = MMP_NULL;
    ptPatPid->ptPsi->table.PAT.totalPmtPidCount = 0;

    //if (MMP_NULL == ptPatPid->ptPsi->table.PAT.pPmtPid)
    {
        MMP_INT pmtPidSize = sizeof(MMP_UINT32) * ptPatInfo->totalProgramCount;

        ptPatPid->ptPsi->table.PAT.pPmtPid = PalHeapAlloc(PAL_HEAP_DEFAULT, pmtPidSize);
        if (MMP_NULL == ptPatPid->ptPsi->table.PAT.pPmtPid)
        {
            psiTablePAT_DestroyTable(ptPatInfo);
            return;
        }

        PalMemset(ptPatPid->ptPsi->table.PAT.pPmtPid, 0x0, pmtPidSize);
    }

    // update PAT table and PMT TS_PID entity
    for (ptCurrentProgram = ptPatInfo->ptFirstProgram;
         (ptPatPid->ptPsi->table.PAT.totalPmtPidCount < ptPatInfo->totalProgramCount)
      && (ptCurrentProgram != MMP_NULL);
         ptCurrentProgram = ptCurrentProgram->ptNextProgram)
    {
        if (0 != ptCurrentProgram->program_number)
        {
            MMP_UINT32  pmtPid              = ptCurrentProgram->program_map_PID;
            MMP_UINT32  pmtProgramNumber    = ptCurrentProgram->program_number;
            TS_PID**    pptPmtPid           = &ptDemux->ptTsPid[pmtPid];
            struct PMT_TBL_SECTION* ptExaminePmt        = MMP_NULL;
            struct PMT_TBL_SECTION* ptLastExaminePmt    = MMP_NULL;
            struct PMT_TBL_SECTION* ptAttachPmt         = MMP_NULL;

            if (MMP_NULL == *pptPmtPid)
            {
                // create a new PMT data identified by PID
                *pptPmtPid = _TS_CreateTsPid(pmtPid, TS_PID_PSI, MMP_TRUE);
                (*pptPmtPid)->ptPsi->ptPsiDecoder =
                    psiTablePMT_AttachDecoder(pmtProgramNumber,
                                              (PSI_PMT_CALLBACK)_TS_PmtCallBack,
                                              ptDemux);
                (*pptPmtPid)->ptPsi->table.PMT.program_number = pmtProgramNumber;
                (*pptPmtPid)->ptPsi->table.PMT.totalEsPidCount = 0;
                (*pptPmtPid)->ptPsi->table.PMT.pmtPid = pmtPid;
                (*pptPmtPid)->ptPsi->table.PMT.version_number = INVALID_VERSION_NUMBER;
                (*pptPmtPid)->ptPsi->table.PMT.pEsPid = MMP_NULL;
                (*pptPmtPid)->ptPsi->table.PMT.ptNextPmt = MMP_NULL;
            }
            else
            {
                // in this statement, found the same PMT PID in PAT
                // check if it has a new program number
                // if yes, add into PMT link list
                
                // solution for the case of more service define in the same PMT PID
                // if a new program number with existed PMT PID come in,
                // add this PMT data in link list
                if ((*pptPmtPid)->ptPsi)
                {
                    for (ptExaminePmt = &((*pptPmtPid)->ptPsi->table.PMT);
                         ptExaminePmt != MMP_NULL;
                         ptLastExaminePmt = ptExaminePmt, ptExaminePmt = ptExaminePmt->ptNextPmt)
                    {
                        if (ptExaminePmt->program_number == pmtProgramNumber)
                            break;
                    }
                }

                // can not found the case of "ptExaminePmt->program_number == pmtProgramNumber"
                // that means this is a existed PMT with a new program number, add it
                if (MMP_NULL == ptExaminePmt && (*pptPmtPid)->ptPsi)
                {
                    ptAttachPmt = (struct PMT_TBL_SECTION*) PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(struct PMT_TBL_SECTION));
                    ptAttachPmt->program_number = pmtProgramNumber;
                    ptAttachPmt->pmtPid = pmtPid;
                    ptAttachPmt->totalEsPidCount = 0;
                    ptAttachPmt->version_number = INVALID_VERSION_NUMBER;
                    ptAttachPmt->pEsPid = MMP_NULL;
                    ptAttachPmt->ptNextPmt = MMP_NULL;
                    ptLastExaminePmt->ptNextPmt = ptAttachPmt;
                }
            }

            // update PAT table
            ptPatPid->ptPsi->table.PAT.pPmtPid[ptPatPid->ptPsi->table.PAT.totalPmtPidCount++]
                = pmtPid;
        }
    }

    // call _TSDEMUX_PatCallBack()
    if (ptDemux->pfPatCallBack)
        ptDemux->pfPatCallBack((void*)ptDemux, ptPatInfo);

    ptPatPid->ptPsi->version_number = ptPatInfo->version_number;
    psiTablePAT_DestroyTable(ptPatInfo);
}

static void
_TS_PesCallBack(
    TS_DEMUX*   ptDemux,
    PES_INFO*   ptPesInfo)
{
    if (MMP_NULL == ptDemux || MMP_NULL == ptPesInfo)
    {
        if (MMP_NULL != ptPesInfo)
            pesPacket_DestroyPesData(ptPesInfo);
        return;
    }

    if (MMP_NULL == ptDemux->pfPesCallBack)
    {
        pesPacket_DestroyPesData(ptPesInfo);
    }
    else
    {
        ptDemux->pfPesCallBack((void*)ptDemux, ptPesInfo);
        ptPesInfo->pData =
        ptPesInfo->pPayloadStartAddress =
        ptPesInfo->pPayloadEndAddress = MMP_NULL;
    }
}

static void
_TS_PmtCallBack(
    TS_DEMUX*       ptDemux,
    PSI_PMT_INFO*   ptPmtInfo)
{
    TS_PID* ptPatPid = MMP_NULL;
    TS_PID* ptPmtPid = MMP_NULL;
    PSI_PMT_ES_INFO* ptCurrentEsInfo = MMP_NULL;
    PSI_DESCRIPTOR* ptDescriptor = MMP_NULL;
    MMP_UINT i = 0;
    MMP_UINT32 audioAc3ChkNum = 0;
    MMP_UINT32 format_identifier = 0;
    MMP_UINT32 currentPmtPid = 0;
    TS_PID* ptCurrentPmtPid = MMP_NULL;
    struct PMT_TBL_SECTION* ptCurrentPmt = MMP_NULL;

    ptPatPid = ptDemux->ptTsPid[PAT_PID];
#ifdef ENABLE_ISDB_T_ONE_SEG
    if (MMP_FALSE == ptDemux->bReceiveSdt
     || MMP_NULL == ptDemux || MMP_NULL == ptPmtInfo || MMP_NULL == ptPatPid)
#else
    if (MMP_NULL == ptDemux || MMP_NULL == ptPmtInfo || MMP_NULL == ptPatPid)
#endif
    {
        if (MMP_NULL != ptPmtInfo)
            psiTablePMT_DestroyTable(ptPmtInfo);
        return;
    }

#ifdef ENABLE_ISDB_T_ONE_SEG
    ptCurrentPmtPid = ptDemux->ptTsPid[0x1FC8];
    ptCurrentPmt = &ptCurrentPmtPid->ptPsi->table.PMT;
#else
    // First find this PMT declared in PAT
    for (i = 0; i < ptPatPid->ptPsi->table.PAT.totalPmtPidCount; i++)
    {
        currentPmtPid = ptPatPid->ptPsi->table.PAT.pPmtPid[i];
        if (currentPmtPid == PAT_PID || currentPmtPid == NIT_PID
         || currentPmtPid == SDT_PID || currentPmtPid == EIT_PID
         || currentPmtPid == TDT_TOT_PID)
        {
            continue;
        }

        // check if it matches the program number in PMT PID
        ptCurrentPmtPid = ptDemux->ptTsPid[currentPmtPid];
        if (MMP_NULL != ptCurrentPmtPid && ptCurrentPmtPid->ptPsi)
        {
            for (ptCurrentPmt = &ptCurrentPmtPid->ptPsi->table.PMT;
                 ptCurrentPmt != MMP_NULL;
                 ptCurrentPmt = ptCurrentPmt->ptNextPmt)
            {
                if (ptPmtInfo->program_number)
                {
                    if (ptCurrentPmt->program_number == ptPmtInfo->program_number)
                    {
                        ptPmtPid = ptCurrentPmtPid;
                        break;
                    }
                }
            }
            if (ptPmtPid)
                break;
        }
    }

    if (MMP_NULL == ptPmtPid)
    {
        // unreferenced program (broken stream)
        psiTablePMT_DestroyTable(ptPmtInfo);
        return;
    }
#endif

    if ((INVALID_VERSION_NUMBER != ptCurrentPmt->version_number)
     && ((0 == ptPmtInfo->current_next_indicator)
     ||  (ptPmtInfo->version_number == ptCurrentPmt->version_number)))
    {
        psiTablePMT_DestroyTable(ptPmtInfo);
        return;
    }

    // ToDo: Clean all the old elementary streams belongs to the PMT?

    if (ptPmtInfo->totalEsCount > 0)
    {
        if (ptPmtInfo->totalEsCount != ptCurrentPmt->totalEsPidCount)
        {
            if (ptCurrentPmt->pEsPid)
            {
                PalHeapFree(PAL_HEAP_DEFAULT, ptCurrentPmt->pEsPid);
                ptCurrentPmt->pEsPid = MMP_NULL;
            }
        }

        if (MMP_NULL == ptCurrentPmt->pEsPid)
        {
            ptCurrentPmt->pEsPid = PalHeapAlloc(PAL_HEAP_DEFAULT,
                sizeof(MMP_UINT32) * ptPmtInfo->totalEsCount);
            if (MMP_NULL == ptCurrentPmt->pEsPid)
            {
                psiTablePMT_DestroyTable(ptPmtInfo);
                return;
            }
        }

        // reset count and go on to re-assign ES PID into PMT
        ptCurrentPmt->totalEsPidCount = 0;
    }

    for (ptCurrentEsInfo = ptPmtInfo->ptFirstEsInfo;
         (ptCurrentPmt->totalEsPidCount < ptPmtInfo->totalEsCount)
         && (ptCurrentEsInfo != MMP_NULL);
         ptCurrentEsInfo = ptCurrentEsInfo->ptNexEsInfo)
    {
        MMP_UINT32  esPid       = ptCurrentEsInfo->elementary_PID;
        TS_PID**    pptEsPid    = &ptDemux->ptTsPid[esPid];
        MMP_UINT32  stream_type = ptCurrentEsInfo->stream_type;

        audioAc3ChkNum = 0;
        if (*pptEsPid && (*pptEsPid)->ptPsi)
        {
            _TS_DestroyTsPid(ptDemux, *pptEsPid);
            *pptEsPid = MMP_NULL;
        }

        if (MMP_NULL == *pptEsPid)
        {
            *pptEsPid = _TS_CreateTsPid(esPid, TS_PID_PES, ptDemux->bEnableEsPID);
        }

        if (stream_type == ISO_IEC_11172_2_VIDEO   // mpeg1 video
         || stream_type == ISO_IEC_13818_2_VIDEO   // mpeg2 video
         || stream_type == ISO_IEC_14496_10_VIDEO) // avc (h.264) video
        {
            if( ptDemux->bOnPesOut == true  )
            {
                if ((*pptEsPid)->ptPes
                 && (*pptEsPid)->ptPes->ptPesDecoder)
                {
                    _TS_DestroyTsPid(ptDemux, *pptEsPid);
                    *pptEsPid = _TS_CreateTsPid(esPid, TS_PID_PES, ptDemux->bEnableEsPID);
                }

                if (MMP_NULL == (*pptEsPid)->ptPes->ptPesDecoder)
                {
                    (*pptEsPid)->ptPes->ptPesDecoder =
                       pesPacket_AttachDecoder(0, // channelId,
                                               PES_PID_VIDEO,
                                               &ptDemux->pidInfo_v,
                                               ptDemux->pidInfo_v.pOutPesBuffer,
                                               (PES_CALLBACK)_TS_PesCallBack,
                                               ptDemux);                        
                }
            }
        }
        else if (stream_type == ISO_IEC_11172_3_AUDIO   // mpeg1 audio
              || stream_type == ISO_IEC_13818_3_AUDIO   // mpeg2 audio
              || stream_type == ISO_IEC_13818_7_AUDIO   // aac adts
              || stream_type == ISO_IEC_14496_3_AUDIO ) // aac latm
        {
            if( ptDemux->bOnPesOut == true )
            {
                if ((*pptEsPid)->ptPes
                 && (*pptEsPid)->ptPes->ptPesDecoder)
                {
                    _TS_DestroyTsPid(ptDemux, *pptEsPid);
                    *pptEsPid = _TS_CreateTsPid(esPid, TS_PID_PES, ptDemux->bEnableEsPID);
                }

                if (MMP_NULL == (*pptEsPid)->ptPes->ptPesDecoder)
                {
                    (*pptEsPid)->ptPes->ptPesDecoder =
                       pesPacket_AttachDecoder(0, // channelId,
                                               PES_PID_AUDIO,
                                               &ptDemux->pidInfo_a,
                                               ptDemux->pidInfo_a.pOutPesBuffer,
                                               (PES_CALLBACK)_TS_PesCallBack,
                                               ptDemux);                         
                }
            }
        }
        else
        {
            ptDescriptor = ptCurrentEsInfo->ptFirstDescriptor;
            while (ptDescriptor)
            {
                if (0x59 == ptDescriptor->descriptor_tag   // subtitling_descriptor
                 && ISO_IEC_13818_1_PES == stream_type)    // mpeg2 private data
                {
                    if( ptDemux->bOnPesOut == true )
                    {
                        if ((*pptEsPid)->ptPes
                         && (*pptEsPid)->ptPes->ptPesDecoder)
                        {
                            _TS_DestroyTsPid(ptDemux, *pptEsPid);
                            *pptEsPid = _TS_CreateTsPid(esPid, TS_PID_PES, ptDemux->bEnableEsPID);
                        }

                        if (MMP_NULL == (*pptEsPid)->ptPes->ptPesDecoder)
                        {
                            (*pptEsPid)->ptPes->ptPesDecoder =
                               pesPacket_AttachDecoder(0, // channelId,
                                                       PES_PID_SUBTITLE,
                                                       &ptDemux->pidInfo_s,
                                                       ptDemux->pidInfo_s.pOutPesBuffer,
                                                       (PES_CALLBACK)_TS_PesCallBack,
                                                       ptDemux); 
                        }
                    }
                    break;
                }
                else if (0x56 == ptDescriptor->descriptor_tag   // teletext_descriptor
                      && ISO_IEC_13818_1_PES == stream_type)    // mpeg2 private data
                {
                    if( ptDemux->bOnPesOut == true )
                    {
                        if ((*pptEsPid)->ptPes
                         && (*pptEsPid)->ptPes->ptPesDecoder)
                        {
                            _TS_DestroyTsPid(ptDemux, *pptEsPid);
                            *pptEsPid = _TS_CreateTsPid(esPid, TS_PID_PES, ptDemux->bEnableEsPID);
                        }

                        if (MMP_NULL == (*pptEsPid)->ptPes->ptPesDecoder)
                        {
                            (*pptEsPid)->ptPes->ptPesDecoder =
                               pesPacket_AttachDecoder(0, // channelId,
                                                       PES_PID_TELETEXT,
                                                       &ptDemux->pidInfo_t,
                                                       ptDemux->pidInfo_t.pOutPesBuffer,
                                                       (PES_CALLBACK)_TS_PesCallBack,
                                                       ptDemux);
                        }
                    }
                    break;
                }
                else if (0x6A == ptDescriptor->descriptor_tag  //AC3_descriptor
                      || 0x7A == ptDescriptor->descriptor_tag) // enhanced_AC3_descriptor
                {
                    audioAc3ChkNum++;
                }
                else if (0x50 == ptDescriptor->descriptor_tag) // component_descriptor
                {
                    if (0x04 == (*ptDescriptor->pPayload & 0x0F)) // reserved for AC3
                        audioAc3ChkNum++;
                }
                else if (0x5 == ptDescriptor->descriptor_tag // registration_descriptor
                      && ptDescriptor->descriptor_length >= 4)
                {
                    if( !strncmp(ptDescriptor->pPayload, "AC-3", strlen("AC-3")) )
                        audioAc3ChkNum++;
                }

                ptDescriptor = ptDescriptor->ptNextDescriptor;
            }

            if (audioAc3ChkNum && ptDemux->bOnPesOut == true)
            {
                if ((*pptEsPid)->ptPes
                 && (*pptEsPid)->ptPes->ptPesDecoder)
                {
                    _TS_DestroyTsPid(ptDemux, *pptEsPid);
                    *pptEsPid = _TS_CreateTsPid(esPid, TS_PID_PES, ptDemux->bEnableEsPID);
                }

                if (MMP_NULL == (*pptEsPid)->ptPes->ptPesDecoder)
                {
                    (*pptEsPid)->ptPes->ptPesDecoder =
                       pesPacket_AttachDecoder(0, // channelId,
                                               PES_PID_AUDIO,
                                               &ptDemux->pidInfo_a,
                                               ptDemux->pidInfo_a.pOutPesBuffer,
                                               (PES_CALLBACK)_TS_PesCallBack,
                                               ptDemux); 
                    audioAc3ChkNum = 0;
                }
            }
        }
        //(*pptEsPid)->ptPes->stream_type = stream_type;
        ptCurrentPmt->pEsPid[ptCurrentPmt->totalEsPidCount++]
            = esPid;
    }
    ptCurrentPmt->version_number = ptPmtInfo->version_number;
    ptPmtInfo->PID = currentPmtPid;

    if (ptDemux->pfPmtCallBack)
        ptDemux->pfPmtCallBack((void*)ptDemux, ptPmtInfo);

    psiTablePMT_DestroyTable(ptPmtInfo);
}

static void
_TS_NewSubtable(
    TS_DEMUX*       ptDemux,
    PSI_DECODER*    ptDecoder,
    MMP_UINT32      table_id,
    MMP_UINT32      table_id_extension)
{
    if (NIT_ACTUAL_TABLE_ID == table_id)
    {
        psiTableNIT_AttachDecoder(
            ptDecoder,
            table_id,
            table_id_extension,
            (PSI_NIT_CALLBACK)_TS_NitCallBack,
            ptDemux);
    }
    else if (NIT_OTHER_TABLE_ID == table_id)
    {
        psiTableNIT_AttachDecoder(
            ptDecoder,
            table_id,
            table_id_extension,
            (PSI_NIT_CALLBACK)_TS_NitCallBack,
            ptDemux);
    }
    else if (SDT_ACTUAL_TABLE_ID == table_id)
    {
        psiTableSDT_AttachDecoder(
            ptDecoder,
            table_id,
            table_id_extension,
            (PSI_SDT_CALLBACK)_TS_SdtCallBack,
            ptDemux);
    }
    else if (EIT_ACTUAL_PRESENT_FOLLOWING_EVENT_TABLE_ID == table_id)
    {
        psiTableEIT_AttachDecoder(
            ptDecoder,
            table_id,
            table_id_extension,
            (PSI_EIT_CALLBACK)_TS_EitPresentFollowingCallBack,
            ptDemux,
            (PSI_SECTION_FILTER_CALLBACK)_TS_EitSectionFilterCallBack);
    }
    else if ((EIT_ACTUAL_SCHEDULE_EVENT_MIN_TABLE_ID <= table_id) && (table_id <= EIT_ACTIAL_SCHEDULE_EVENT_MAX_TABLE_ID))
    {
        psiTableEIT_AttachDecoder(
            ptDecoder,
            table_id,
            table_id_extension,
            (PSI_EIT_CALLBACK)_TS_EitScheduleCallBack,
            ptDemux,
            (PSI_SECTION_FILTER_CALLBACK)_TS_EitSectionFilterCallBack);
    }
    else if (TDT_TABLE_ID == table_id)
    {
        psiTableTDT_AttachDecoder(
            ptDecoder,
            table_id,
            table_id_extension,
            (PSI_TDT_CALLBACK)_TS_TdtCallBack,
            ptDemux);
    }
    else if (TOT_TABLE_ID == table_id)
    {
        psiTableTOT_AttachDecoder(
            ptDecoder,
            table_id,
            table_id_extension,
            (PSI_TOT_CALLBACK)_TS_TotCallBack,
            ptDemux);
    }
}

// attach to NIT decoder, call back this funtion after NIT decoding
// to receive NIT info
static void
_TS_NitCallBack(
    TS_DEMUX*       ptDemux,
    PSI_NIT_INFO*   ptNitInfo)
{
    TS_PID*                     ptNitPid = MMP_NULL;
    struct NIT_TBL_SECTION*     ptNitSection = MMP_NULL;

    if (MMP_NULL == ptDemux || MMP_NULL == ptNitInfo)
    {
        if (MMP_NULL != ptNitInfo)
            psiTableNIT_DestroyTable(ptNitInfo);
        return;
    }

    ptNitPid = ptDemux->ptTsPid[NIT_PID];
    ptNitSection = &ptNitPid->ptPsi->table.NIT;

    if (INVALID_TRANSPORT_STREAM_ID == ptDemux->transport_stream_id)
    {
        // The SDT is not received yet.
        tsp_msg(1, "NIT Received Before SDT\n");
        // save actual network NIT info in ptDemux->ptTmpActualNit temporarily
        // then call back _TSDEMUX_NitCallBack() after receiving SDT (in _TS_SdtCallBack())
        if (NIT_ACTUAL_TABLE_ID == ptNitInfo->table_id) // actual netowrk
            _TS_CreateTmpActualNIT(ptDemux, ptNitInfo);

        psiTableNIT_DestroyTable(ptNitInfo);
        return;
    }
    else
    {
        if (ptDemux->bReceiveSdt)
        {
            if (NIT_ACTUAL_TABLE_ID == ptNitInfo->table_id) // actual netowrk
            {
                if (INVALID_VERSION_NUMBER != ptNitSection->actual_version_number
                 && (0 == ptNitInfo->current_next_indicator || ptNitInfo->version_number == ptNitSection->actual_version_number))
                {
                    // no need to update NIT, destroy NIT info and return
                    psiTableNIT_DestroyTable(ptNitInfo);
                    return;
                }
                else
                {
                    ptNitSection->actual_version_number = ptNitInfo->version_number;
                    ptDemux->bWaitNitReady = MMP_FALSE; // no more need to wait for NIT
                }
            }
            else // other network
            {
                if (INVALID_VERSION_NUMBER != ptNitSection->other_version_number
                 && (0 == ptNitInfo->current_next_indicator || ptNitInfo->version_number == ptNitSection->other_version_number))
                {
                    // no need to update NIT, destroy NIT info and return
                    psiTableNIT_DestroyTable(ptNitInfo);
                    return;
                }
                else
                    ptNitSection->other_version_number = ptNitInfo->version_number;
            }
        }
    }

    if (ptDemux->bReceiveSdt)
    {
        if (ptDemux->pfNitCallBack)
            ptDemux->pfNitCallBack((void*)ptDemux, ptNitInfo);
    }

    psiTableNIT_DestroyTable(ptNitInfo);
}

static void
_TS_SdtCallBack(
    TS_DEMUX*       ptDemux,
    PSI_SDT_INFO*   ptSdtInfo)
{
    TS_PID*             ptSdtPid    = MMP_NULL;
    PSI_SDT_SERVICE*    ptService   = MMP_NULL;

    if (MMP_NULL == ptDemux || MMP_NULL == ptSdtInfo)
    {
        if (MMP_NULL != ptSdtInfo)
            psiTableSDT_DestroyTable(ptSdtInfo);
        return;
    }

    ptSdtPid = ptDemux->ptTsPid[SDT_PID];
//    if (INVALID_VERSION_NUMBER != ptSdtPid->ptPsi->version_number
//     && (0 == ptSdtInfo->current_next_indicator || ptSdtInfo->version_number == ptSdtPid->ptPsi->version_number)
//     && ptSdtInfo->transport_stream_id == ptDemux->transport_stream_id)
//    {
//        psiTableSDT_DestroyTable(ptSdtInfo);
//        return;
//    }

    ptSdtPid->ptPsi->version_number = ptSdtInfo->version_number;
    if (INVALID_TRANSPORT_STREAM_ID == ptDemux->transport_stream_id)
        ptDemux->transport_stream_id = ptSdtInfo->transport_stream_id;

    // [20101019] vincent: not install service with private temporary use NID and ONID
//    if (ptDemux->ptTmpActualNit == MMP_NULL
//     || (ptDemux->ptTmpActualNit->network_id & PRIVATE_TEMPORARY_USE_NID) != PRIVATE_TEMPORARY_USE_NID)
    {
        if (ptDemux->pfSdtCallBack)
            ptDemux->pfSdtCallBack((void*)ptDemux, ptSdtInfo);
    }

    psiTableSDT_DestroyTable(ptSdtInfo);

    // notify temp keeped NIT to upper layer.
    if (ptDemux->ptTmpActualNit)
    {
        tsp_msg(TSP_MSG_TYPE_ERR, "Notify Temp Actual NIT to upper layer\n");
        if (ptDemux->pfNitCallBack)
            ptDemux->pfNitCallBack((void*)ptDemux, ptDemux->ptTmpActualNit);
        ptDemux->bWaitNitReady = MMP_FALSE;
        _TS_DestroyTmpActualNIT(ptDemux);
    }
}

static MMP_BOOL
_TS_EitSectionFilterCallBack(
    TS_DEMUX*       ptDemux,
    PSI_SECTION*    ptSection)
{
    MMP_BOOL result = MMP_FALSE;
    if (ptDemux->pfEitSectionFilterCallBack)
        result = ptDemux->pfEitSectionFilterCallBack(ptDemux, ptSection);
    return result;
}

static void
_TS_EitPresentFollowingCallBack(
    TS_DEMUX*       ptDemux,
    PSI_EIT_INFO*   ptEitInfo)
{
    if (MMP_NULL == ptDemux || MMP_NULL == ptEitInfo)
    {
        if (MMP_NULL != ptEitInfo)
            psiTableEIT_DestroyTable(ptEitInfo);
        return;
    }

    // current_next_indicator is 0 that means
    // this table should not be activated
    // until current_next_indicator is 1 next time
    if (0 == ptEitInfo->current_next_indicator)
    {
        psiTableEIT_DestroyTable(ptEitInfo);
        return;
    }

    if (ptDemux->pfEitPfCallBack)
        ptDemux->pfEitPfCallBack((void*)ptDemux, ptEitInfo);

    psiTableEIT_DestroyTable(ptEitInfo);
}

static void
_TS_EitScheduleCallBack(
    TS_DEMUX*       ptDemux,
    PSI_EIT_INFO*   ptEitInfo)
{
    if (MMP_NULL == ptDemux || MMP_NULL == ptEitInfo)
    {
        if (MMP_NULL != ptEitInfo)
            psiTableEIT_DestroyTable(ptEitInfo);
        return;
    }

    // current_next_indicator is 0 that means
    // this table should not be activated
    // until current_next_indicator is 1 next time
    if (0 == ptEitInfo->current_next_indicator)
    {
        psiTableEIT_DestroyTable(ptEitInfo);
        return;
    }

    if (ptDemux->pfEitSchCallBack)
        ptDemux->pfEitSchCallBack((void*)ptDemux, ptEitInfo);

    psiTableEIT_DestroyTable(ptEitInfo);
}

static void
_TS_TdtCallBack(
    TS_DEMUX*       ptDemux,
    PSI_MJDBCD_TIME tUtcTime)
{
    if (ptDemux->pfTdtCallBack)
        ptDemux->pfTdtCallBack((void*)ptDemux, tUtcTime);
}

static void
_TS_TotCallBack(
    TS_DEMUX*       ptDemux,
    PSI_TOT_INFO*   ptTotInfo)
{
    if (MMP_NULL == ptDemux || MMP_NULL == ptTotInfo)
    {
        if (MMP_NULL != ptTotInfo)
            psiTableTOT_DestroyTable(ptTotInfo);
        return;
    }

    if (ptDemux->pfTotCallBack)
        ptDemux->pfTotCallBack((void*)ptDemux, ptTotInfo);

    psiTableTOT_DestroyTable(ptTotInfo);
}

#ifdef ENABLE_DSM_CC
static void
_TS_DsmCcCallBack(
    TS_DEMUX*       ptDemux,
    DSM_CC_INFO*    ptDsmCcInfo)
{
    MMP_UINT8* pCurrentAddr = MMP_NULL;
    MMP_UINT32 payloadSize = 0;

    if (MMP_NULL == ptDemux || MMP_NULL == ptDsmCcInfo)
    {
        if (MMP_NULL != ptDsmCcInfo)
            DSM_CC_DestroyInfo(ptDsmCcInfo);
        return;
    }

    if (ptDemux->pfDsmCcCallBack)
        ptDemux->pfDsmCcCallBack((void*)ptDemux, ptDsmCcInfo);
    DSM_CC_DestroyInfo(ptDsmCcInfo);
}
#endif

static MMP_INLINE MMP_UINT
_TS_GetPid(
    MMP_UINT8*  pData)
{
    return ((MMP_UINT)(pData[1] & 0x1f) << 8) | pData[2];
}

static void
_TS_CreateTmpActualNIT(
    TS_DEMUX*       ptDemux,
    PSI_NIT_INFO*   ptNitInfo)
{
    PSI_NIT_INFO*               ptTmpNit = MMP_NULL;
    PSI_NIT_TRANSPORT_STREAM*   ptTransportStream = MMP_NULL;
    PSI_NIT_TRANSPORT_STREAM*   ptNewTransportStream = MMP_NULL;
    PSI_NIT_TRANSPORT_STREAM*   ptTmpLastTransportStream = MMP_NULL;
    PSI_DESCRIPTOR*             ptDescriptor = MMP_NULL;
    PSI_DESCRIPTOR*             ptNewDescriptor = MMP_NULL;
    PSI_DESCRIPTOR*             ptTmpLastDescriptor = MMP_NULL;

    PalAssert(ptDemux);

    if (ptDemux->ptTmpActualNit)
        _TS_DestroyTmpActualNIT(ptDemux);

    ptTmpNit = (PSI_NIT_INFO*)  PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(PSI_NIT_INFO));
    PalMemcpy(ptTmpNit, ptNitInfo, sizeof(PSI_NIT_INFO));

#if defined (SUPPORT_OTA) && defined (ENABLE_DSM_CC)
    for (ptDescriptor = ptNitInfo->ptFirstDescriptor;
         ptDescriptor;
         ptDescriptor = ptDescriptor->ptNextDescriptor)
    {
        ptNewDescriptor = (PSI_DESCRIPTOR*) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                         sizeof(PSI_DESCRIPTOR));
        ptNewDescriptor->descriptor_tag = ptDescriptor->descriptor_tag;
        ptNewDescriptor->descriptor_length = ptDescriptor->descriptor_length;
        ptNewDescriptor->pPayload = MMP_NULL;
        ptNewDescriptor->ptDecodedContent = MMP_NULL;
        ptNewDescriptor->ptNextDescriptor = MMP_NULL;

        if (ptDescriptor->pPayload && ptDescriptor->descriptor_length)
        {
            ptNewDescriptor->pPayload = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                                  ptDescriptor->descriptor_length);
            PalMemcpy(ptNewDescriptor->pPayload, ptDescriptor->pPayload, ptDescriptor->descriptor_length);
        }

        if (ptTmpLastDescriptor)
            ptTmpLastDescriptor->ptNextDescriptor = ptNewDescriptor;
        else
            ptNitInfo->ptFirstDescriptor = ptNewDescriptor;
        ptTmpLastDescriptor = ptNewDescriptor;
    }
#endif

    ptTmpNit->ptFirstTransportStream = MMP_NULL;
    ptTmpLastTransportStream = MMP_NULL;

    for (ptTransportStream = ptNitInfo->ptFirstTransportStream;
         ptTransportStream;
         ptTransportStream = ptTransportStream->ptNextTransportStream)
    {
        ptNewTransportStream = (PSI_NIT_TRANSPORT_STREAM*) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                                        sizeof(PSI_NIT_TRANSPORT_STREAM));
        ptNewTransportStream->original_network_id = ptTransportStream->original_network_id;
        ptNewTransportStream->transport_stream_id = ptTransportStream->transport_stream_id;
        ptNewTransportStream->ptNextTransportStream = MMP_NULL;
        ptNewTransportStream->ptFirstDescriptor = MMP_NULL;
        ptTmpLastDescriptor = MMP_NULL;

        for (ptDescriptor = ptTransportStream->ptFirstDescriptor;
             ptDescriptor;
             ptDescriptor = ptDescriptor->ptNextDescriptor)
        {
            ptNewDescriptor = (PSI_DESCRIPTOR*) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                             sizeof(PSI_DESCRIPTOR));
            ptNewDescriptor->descriptor_tag = ptDescriptor->descriptor_tag;
            ptNewDescriptor->descriptor_length = ptDescriptor->descriptor_length;
            ptNewDescriptor->pPayload = MMP_NULL;
            ptNewDescriptor->ptDecodedContent = MMP_NULL;
            ptNewDescriptor->ptNextDescriptor = MMP_NULL;

            if (ptDescriptor->pPayload && ptDescriptor->descriptor_length)
            {
                ptNewDescriptor->pPayload = (MMP_UINT8*) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                                      ptDescriptor->descriptor_length);
                PalMemcpy(ptNewDescriptor->pPayload, ptDescriptor->pPayload, ptDescriptor->descriptor_length);
            }

            if (ptTmpLastDescriptor)
                ptTmpLastDescriptor->ptNextDescriptor = ptNewDescriptor;
            else
                ptNewTransportStream->ptFirstDescriptor = ptNewDescriptor;
            ptTmpLastDescriptor = ptNewDescriptor;
        }

        if (ptTmpLastTransportStream)
            ptTmpLastTransportStream->ptNextTransportStream = ptNewTransportStream;
        else
            ptTmpNit->ptFirstTransportStream = ptNewTransportStream;

        ptTmpLastTransportStream = ptNewTransportStream;
    }

    ptDemux->ptTmpActualNit = ptTmpNit;
}

static void
_TS_DestroyTmpActualNIT(
    TS_DEMUX*       ptDemux)
{
    PSI_NIT_INFO*               ptTmpNit = ptDemux->ptTmpActualNit;
    PSI_NIT_TRANSPORT_STREAM*   ptTransportStream = MMP_NULL;
    PSI_NIT_TRANSPORT_STREAM*   ptFreeTransportStream = MMP_NULL;
    PSI_DESCRIPTOR*             ptDescriptor = MMP_NULL;
    PSI_DESCRIPTOR*             ptFreeDescriptor = MMP_NULL;

#if defined (SUPPORT_OTA) && defined (ENABLE_DSM_CC)
    ptDescriptor = ptTmpNit->ptFirstDescriptor;
    while (ptDescriptor)
    {
        if (ptDescriptor->pPayload)
            PalHeapFree(PAL_HEAP_DEFAULT, ptDescriptor->pPayload);
        ptFreeDescriptor = ptDescriptor;
        ptDescriptor = ptDescriptor->ptNextDescriptor;
        PalHeapFree(PAL_HEAP_DEFAULT, ptFreeDescriptor);
    }
#endif

    ptTransportStream = ptTmpNit->ptFirstTransportStream;
    while (ptTransportStream)
    {
        ptDescriptor = ptTransportStream->ptFirstDescriptor;
        while (ptDescriptor)
        {
            if (ptDescriptor->pPayload)
                PalHeapFree(PAL_HEAP_DEFAULT, ptDescriptor->pPayload);
            ptFreeDescriptor = ptDescriptor;
            ptDescriptor = ptDescriptor->ptNextDescriptor;
            PalHeapFree(PAL_HEAP_DEFAULT, ptFreeDescriptor);
        }

        ptFreeTransportStream = ptTransportStream;
        ptTransportStream = ptTransportStream->ptNextTransportStream;
        PalHeapFree(PAL_HEAP_DEFAULT, ptFreeTransportStream);
    }
    PalHeapFree(PAL_HEAP_DEFAULT, ptTmpNit);
    ptDemux->ptTmpActualNit = MMP_NULL;
}
