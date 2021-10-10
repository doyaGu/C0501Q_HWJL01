/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file ts.c
 * Used to provide transport stream operations
 * @author I-Chun Lai
 * @version 0.1
 */

#include "def.h"
#include "string.h"
#include "ts.h"
#include "queue_mgr.h"
#include "mmio.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define PAT_PID     (0x00)

#define INVALID_VERSION_NUMBER  ((uint32)(-1))

typedef enum MPEG2_STREAM_TYPE_TAG {
    ISO_IEC_RESERVED                = 0x00,
    ISO_IEC_11172_2_VIDEO           = 0x01,
    ISO_IEC_13818_2_VIDEO           = 0x02,
    ISO_IEC_11172_3_AUDIO           = 0x03,
    ISO_IEC_13818_3_AUDIO           = 0x04,
    ISO_IEC_13818_1_PRIVATE_SECTION = 0x05,
    ISO_IEC_13818_1_PES             = 0x06,
    ISO_IEC_13522_MHEG              = 0x07,
    ANNEX_A_DSM_CC                  = 0x08,
    ITU_T_REC_H_222_1               = 0x09,
    ISO_IEC_13818_6_TYPE_A          = 0x0A,
    ISO_IEC_13818_6_TYPE_B          = 0x0B,
    ISO_IEC_13818_6_TYPE_C          = 0x0C,
    ISO_IEC_13818_6_TYPE_D          = 0x0D,
    ISO_IEC_13818_1_AUXILIARY       = 0x0E,
    ISO_IEC_13818_7_AUDIO           = 0x0F,
    ISO_IEC_14496_2_VISUAL          = 0x10,
    ISO_IEC_14496_3_AUDIO           = 0x11,
    ISO_IEC_14496_10_VIDEO          = 0x1B,
    ISO_IEC_13818_1_RESERVED        = 0x1C,
    USER_PRIVATE                    = 0x80
} MPEG2_STREAM_TYPE;

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

static TS_DEMUX gtTsDemux[MAX_CHANNEL_COUNT];
static TS_PID   gtPid[TOTAL_PID_COUNT];
static TS_PES   gtPes[TOTAL_PES_COUNT];

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static TS_DEMUX*
_TS_CreateTsDemux(
    uint            channelId,    
    CHANNEL_INFO*   ptChanInfo);

static void
_TS_DestroyTsDemux(
    TS_DEMUX*   ptTsDemux);

MMP_BOOL
_TS_Decode188(
    TS_DEMUX*   ptDemux,
    uint8*      pData);

static TS_PID*
_TS_CreateTsPid(
    uint32      PID,
    MMP_BOOL    bPsiValid);

static void
_TS_DestroyTsPid(
    TS_DEMUX*   ptDemux,
    TS_PID*     ptTsPid);

static void
_TS_PesCallBack(
    TS_DEMUX*   ptDemux,
    PES_INFO*   ptPesInfo);

static MMP_INLINE uint
_TS_GetPid(
    uint8*      pData);

static MMP_BOOL
_TS_IsAnyPesBufFull(
    uint        channelId);

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * TS decoder initialing invokes creating TS_DEMUX structure and hanging
 * particular table handlers (callback functions return after decoding).
 *
 * @param   none.
 * @return  TS_DEMUX* structure pointer to store pid data.
 */
//=============================================================================
MMP_BOOL
TS_Init(
    uint            channelId,    
    CHANNEL_INFO*   ptChanInfo)
{
    TS_DEMUX*   ptDemux     = MMP_NULL;

    ptDemux = _TS_CreateTsDemux(channelId, ptChanInfo);

    if (MMP_NULL == ptDemux)
        return MMP_FALSE;

    return MMP_TRUE;
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
    uint        channelId)
{
    TS_DEMUX* ptDemux = MMP_NULL;

    if (channelId < MAX_CHANNEL_COUNT)
    {
        ptDemux = (TS_DEMUX*)&gtTsDemux[channelId];

        _TS_DestroyTsDemux(ptDemux);
    }
}

uint32
TS_Decode(
    uint        channelId,
    uint8*      pData,
    uint32      size)
{
    TS_DEMUX*   ptDemux = MMP_NULL;
    uint32      result = 0;
    uint32      remainSize = size;
    uint32      forceStop = MMP_FALSE;    

    if (channelId < MAX_CHANNEL_COUNT)
        ptDemux = (TS_DEMUX*)&gtTsDemux[channelId];
    else
        return 0;

    while (remainSize > 0 && (!forceStop))
    {
        switch (ptDemux->demuxState)
        {
        case TS_DEMUX_STATE_SEARCH_PACKET_START:
            if (*pData == VALID_SYNC_BYTE)
            {
                if (remainSize >= TS_PACKET_SIZE)
                {
                    switch (ptDemux->bufCtrlState)
                    {
                    case TS_BUF_CTRL_STATE_CHECK_BUFFER_EMPTY:
                        {
                            MMP_BOOL bBlockMode;
                            dc_invalidate();
                            bBlockMode = GET32(ptDemux->ptChanInfo->bBlockMode);
                            if (bBlockMode && _TS_IsAnyPesBufFull(channelId))
                            {
                                forceStop = MMP_TRUE;
                                continue;
                            }
                            else
                                ptDemux->bufCtrlState = TS_BUF_CTRL_STATE_DO_DECODE;
                        }
                        break;

                    case TS_BUF_CTRL_STATE_DO_DECODE:
                        break;
                    }
                    _TS_Decode188(ptDemux, pData);
                    pData       += TS_PACKET_SIZE;
                    remainSize  -= TS_PACKET_SIZE;
                    break;
                }
                else
                {
                    ptDemux->demuxState = TS_DEMUX_STATE_LESS_THAN_188;
                }
            }
            else
            {
                ++pData;
                --remainSize;
            }
            break;

        case TS_DEMUX_STATE_LESS_THAN_188:
            if (ptDemux->collectedByte > 0 &&
                remainSize >= (int32)(TS_PACKET_SIZE - ptDemux->collectedByte))
            {
                switch (ptDemux->bufCtrlState)
                {
                case TS_BUF_CTRL_STATE_CHECK_BUFFER_EMPTY:
                    {
                        MMP_BOOL bBlockMode;
                        dc_invalidate();
                        bBlockMode = GET32(ptDemux->ptChanInfo->bBlockMode);
                        if (bBlockMode && _TS_IsAnyPesBufFull(channelId))
                        {
                            forceStop = MMP_TRUE;
                            continue;
                        }
                        else
                            ptDemux->bufCtrlState = TS_BUF_CTRL_STATE_DO_DECODE;
                    }
                    break;

                case TS_BUF_CTRL_STATE_DO_DECODE:
                    break;
                }
                PalMemcpy(&ptDemux->incompleteTsPacket[ptDemux->collectedByte],
                    pData,
                    TS_PACKET_SIZE - ptDemux->collectedByte);
                _TS_Decode188(ptDemux, ptDemux->incompleteTsPacket);
                pData       += (TS_PACKET_SIZE - ptDemux->collectedByte);
                remainSize  -= (TS_PACKET_SIZE - ptDemux->collectedByte);

                ptDemux->collectedByte = 0;
                ptDemux->demuxState = TS_DEMUX_STATE_SEARCH_PACKET_START;
                break;
            }
            else
            {
                PalMemcpy(&ptDemux->incompleteTsPacket[ptDemux->collectedByte],
                    pData,
                    remainSize);
                ptDemux->collectedByte += remainSize;
                remainSize = 0;
            }
            break;
        }
    }

    result = size - remainSize;
    return result;
}

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
TS_InsertEsPid(
    uint        channelId,
    uint        pesIndex,
    PID_INFO*   ptPidInfo,
    uint8*      pRiscBaseAddr)
{
    TS_DEMUX*   ptTsDemux   = MMP_NULL;
    TS_PID**    pptPid  = MMP_NULL;
    uint32      pid;

    if (channelId <  MAX_CHANNEL_COUNT
     && pesIndex  <  MAX_PID_COUNT_PER_CHANNEL
     && MMP_NULL  != ptPidInfo)
        ptTsDemux = (TS_DEMUX*)&gtTsDemux[channelId];
    else
        return MMP_FALSE;

    // Init PES handler
    pid = GET32(ptPidInfo->pid);
    pptPid  = &ptTsDemux->ptTsPid[pid];
    *pptPid = _TS_CreateTsPid(pid, MMP_FALSE);

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
    uint            channelId,    
    CHANNEL_INFO*   ptChanInfo)
{
    TS_DEMUX* ptDemux = MMP_NULL;

    if (channelId < MAX_CHANNEL_COUNT)
    {
        ptDemux = (TS_DEMUX*)&gtTsDemux[channelId];

        // TS Demux initialization
        PalMemset(ptDemux, 0x0, sizeof(TS_DEMUX));
        ptDemux->ptChanInfo = ptChanInfo;
    }

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
    if (MMP_NULL != ptTsDemux)
    {
        // Destroy PAT handler
        if (MMP_NULL != ptTsDemux->ptTsPid[PAT_PID])
            _TS_DestroyTsPid(ptTsDemux, ptTsDemux->ptTsPid[PAT_PID]);
    }
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
_TS_Decode188(
    TS_DEMUX*   ptDemux,
    uint8*      pData)
{
    TS_PID*     ptTsPid = MMP_NULL;
    uint        pid = 0;

    if ((MMP_NULL != ptDemux) && (MMP_NULL != pData))
    {
        if (pData[0] != VALID_SYNC_BYTE)
        {
            return MMP_FALSE;
        }

        // Pid filter+
        pid = _TS_GetPid(pData);

        ptTsPid = ptDemux->ptTsPid[pid];
        if (MMP_NULL != ptTsPid)
        {
            if (MMP_TRUE == ptTsPid->bValid
             && MMP_NULL != ptTsPid->ptPes
             && MMP_NULL != ptTsPid->ptPes->ptPesDecoder)
            {
                pesPacket_DecodePacket(ptTsPid->ptPes->ptPesDecoder,
                                       pData);
            }
        }
    }
    return MMP_TRUE;
}

//=============================================================================
/**
 * Create a new TS_PID structure that will be used to store decoder
 * information for each serviceable pid.
 *
 * @param PID           Specified pid value.
 * @param bPsiValid     MMP_TRUE means this pid belongs to psi type, otherwise
 *                      belongs to pes type.
 * @param bPidValid     Set this value MMP_TRUE the specified pid will be
 *                      decoded, otherwise not.
 * @return TS_PID* structure pointer to store decoder information.
 */
//=============================================================================
static TS_PID*
_TS_CreateTsPid(
    uint32      PID,
    MMP_BOOL    bPsiValid)
{
    TS_PID*     ptTsPid = MMP_NULL;
    uint        i;

    if (bPsiValid)
        return MMP_NULL;

    for (i = 0; i < TOTAL_PID_COUNT; ++i)
    {
        if (!gtPid[i].bValid)
        {
            ptTsPid = &gtPid[i];
            break;
        }
    }
    if (MMP_NULL == ptTsPid)
        return MMP_NULL;

    // TS PID initialization
    PalMemset(ptTsPid, 0x0, sizeof(TS_PID));
    ptTsPid->PID = PID;

    {
        for (i = 0; i < TOTAL_PES_COUNT; ++i)
        {
            if (!gtPes[i].bValid)
            {
                ptTsPid->ptPes = &gtPes[i];
                break;
            }
        }

        if (MMP_NULL == ptTsPid->ptPes)
            return MMP_NULL;

        // TS PES initialization
        PalMemset(ptTsPid->ptPes, 0x0, sizeof(TS_PES));
        // Not finish yet!!!
        ptTsPid->bValid = MMP_TRUE;
        ptTsPid->ptPes->bValid = MMP_TRUE;
    }
    ptTsPid->bValid = MMP_TRUE;

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
        uint pid = ptTsPid->PID;

        if (MMP_NULL != ptTsPid->ptPes)
        {
            if (MMP_NULL != ptTsPid->ptPes->ptPesDecoder)
            {
                pesPacket_DetachDecoder(ptTsPid->ptPes->ptPesDecoder);
            }

            ptTsPid->ptPes->bValid = MMP_FALSE;
        }

        ptTsPid->bValid = MMP_FALSE;
        ptDemux->ptTsPid[pid] = MMP_NULL;
    }
}

static void
_TS_PesCallBack(
    TS_DEMUX*   ptDemux,
    PES_INFO*   ptPesInfo)
{
    if (MMP_NULL == ptDemux || MMP_NULL == ptPesInfo)
    {
        if (MMP_NULL != ptPesInfo)
            pesPacket_DestroyPes(ptPesInfo);
        return;
    }

    ptDemux->bufCtrlState = TS_BUF_CTRL_STATE_CHECK_BUFFER_EMPTY;
}

static MMP_INLINE uint
_TS_GetPid(
    uint8*      pData)
{
    return ((uint)(pData[1] & 0x1f) << 8) | pData[2];
}

static MMP_BOOL
_TS_IsAnyPesBufFull(
    uint        channelId)
{
    MMP_BOOL result = MMP_FALSE;
    uint     i;

    for (i = 0; i < MAX_PID_COUNT_PER_CHANNEL; ++i)
    {
        MMP_UINT32 count = 0;

        if ((MMP_SUCCESS == queueMgr_GetAvailableCount(channelId, i, &count)) && (1 >= count))
        {
            result = MMP_TRUE;
            break;
        }
    }

    return result;
}
