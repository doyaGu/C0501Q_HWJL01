/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file pes_packet.c
 * Used to process PES packet decode issue
 * @author Steven Hsiao
 * @version 0.1
 */

#include "def.h"
#include "string.h"
#include "pes_packet.h"
#include "queue_mgr.h"
#include "ts.h"
#include "mmio.h"

// #include "stdafx.h"
//
// #ifdef _DEBUG
//     static void dbg_printf (const char * format,...)
//     {
//     #define MAX_DBG_MSG_LEN (1024)
//         char buf[MAX_DBG_MSG_LEN];
//         va_list ap;
//
//         va_start(ap, format);
//
//         _vsnprintf(buf, sizeof(buf), format, ap);
//         OutputDebugString(buf);
//
//         va_end(ap);
//     }
//     #define DBG dbg_printf
// #else
//     static void dbg_printf (const char * format,...) {}
//     #define DBG  1?((void)(NULL)):dbg_printf
// #endif

//=============================================================================
//                              Constant Definition
//=============================================================================

#define VALID_SYNC_BYTE                 (0x47)
#define TS_PACKET_BYTE_NUMBER           (188)
#define CONTINUITY_COUNTER_MASK         (0xF)

typedef enum ADAPTATION_FIELD_CONTROL_TAG
{
    PAYLOAD_EXIST           = 0x01,
    ADAPTATION_FIELD_EXIST  = 0x02
} ADAPTATION_FIELD_CONTROL;

// The offset is from the end of adaptation field length
#define PCR_FLAG_OFFSET                 (3)

// The size of PCR is 33 bits. It's impossible to save in a uint32,
// therefore, we save bit 32 to HIGH and bit 0 ~ 31 to LOW.
#define PCR_HIGH_OFFSET                 (8)
#define PCR_LOW_OFFSET                  (9)

#define PES_START_CODE_PREFIX           (0x000001)

// See H222.0 p35 Table 2-18 to get further information
#define PROGRAM_STREAM_MAP_ID           (0xBC)
#define PADDING_STREAM_ID               (0xBE)
#define PRIVATE_STREAM_2_ID             (0xBF)
#define ECM_ID                          (0xF0)
#define EMM_ID                          (0xF1)
#define PROGRAM_STREAM_DIRECTORY_ID     (0xFF)
#define DSMCC_STREAM_ID                 (0xF2)
#define ITU_H222_1_TYPE_E_STREAM_ID     (0xF8)

#define PTS_FIELD_ONLY                  (0x2)
#define PTS_DTS_FIELD_EXIST             (0x3)
#define PES_PTS_DTS_SECTION_SIZE        (5)

#define PES_SEGMENT_SIZE                (40960)
#define PES_PRIOR_HEADER_DATA_LENGTH    (9)

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

PES_DECODER gtPesDecoder[TOTAL_PES_COUNT];

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static PES_INFO*
_PES_CreatePesInfo(
    PES_DECODER*    ptDecoder,
    uint32          elementary_ID);

static MMP_BOOL
_PES_CollectSegment(
    PES_DECODER*    ptDecoder,
    uint32          size,
    uint8*          pDataSource);

static void
_PES_GatherPesSegment(
    int32           availableByte,
    uint8*          pPayloadAddress,
    PES_DECODER*    ptDecoder);

static void
_PES_ParsePesPriorHeader(
    PES_DECODER*    ptDecoder);

static void
_PES_ParsePes(
    PES_DECODER*    ptDecoder);

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Attach the PES decoder on the TS parser system.
 *
 * @param elementary_PID    The owner of the PES decoder
 * @param bPcrPid           Is the PID is a PCR_PID
 * @param pfCallback        A callback function is called after the PES packet
 *                          decoding complete.
 * @param pCallbackData     The datagram of the callback function
 * @return                  PEI_DECODER* to handle the PES packet data
 *                          collection and decode of incoming TS packets.
 */
//=============================================================================
PES_DECODER*
pesPacket_AttachDecoder(
    uint                channelId,
    uint                pesIndex,
    PID_INFO*           ptPidInfo,
    uint8*              pRiscBaseAddr,
    PES_CALLBACK        pfCallback,
    void*               pCallbackData)
{
    PES_DECODER*        ptDecoder   = MMP_NULL;
    QUEUE_DATA_BLOCK*   ptQueue;
    uint32              i;

    ptQueue = queueMgr_CreateQueue(channelId, pesIndex, ptPidInfo, pRiscBaseAddr);
    if (MMP_NULL == ptQueue)
        return MMP_NULL;

    for (i = 0; i < TOTAL_PES_COUNT; ++i)
    {
        if (0 == gtPesDecoder[i].elementary_PID)
        {
            ptDecoder = &gtPesDecoder[i];
            break;
        }
    }

    if (ptDecoder)
    {
        PalMemset(ptDecoder, 0x0, sizeof(PES_DECODER));
        ptDecoder->ptQueue          = ptQueue;
        ptDecoder->queueSampleSize  = ptQueue->pOutPesSampleSize;
        ptDecoder->bFirstPacket     = MMP_TRUE;
        ptDecoder->elementary_PID   = ptQueue->pid;
        ptDecoder->pfCallback       = pfCallback;
        ptDecoder->pCallbackData    = pCallbackData;
    }
    return ptDecoder;
}

//=============================================================================
/**
 * Detach the PES decoder from the TS parser system.
 *
 * @param ptDecoder The decoder to decode a incoming TS packet.
 * @return none
 */
//=============================================================================
void
pesPacket_DetachDecoder(
    PES_DECODER*    ptDecoder)
{
    if (MMP_NULL == ptDecoder)
        return;

    if (ptDecoder->ptBuildingPes)
    {
        pesPacket_DestroyPes(ptDecoder->ptBuildingPes);
        ptDecoder->ptBuildingPes = MMP_NULL;
    }

    //_PES_DestorySegment(ptDecoder);
    queueMgr_DestroyQueue(ptDecoder->ptQueue);

    ptDecoder->elementary_PID = 0;
}

//=============================================================================
/**
 * Inject a TS packet into a Pes decoder for content decoding.
 *
 * @param ptDecoder         The decoder to decode a incoming TS packet.
 * @param pPacketAddress    The pointer is pointing to the start of a 188 bytes
 *                          TS packet.
 * @return none
 */
//=============================================================================
void
pesPacket_DecodePacket(
    PES_DECODER*    ptDecoder,
    uint8*          pPacketAddress)
{
    uint32      sync_byte = 0;
    uint32      payload_unit_start_indicator = 0;
    uint32      adaptation_field_control = 0;
    uint32      continuity_counter = 0;
    uint32      PID = 0;
    uint32      adaptation_field_length = 0;
    uint32      PCR_flag = 0;

    int32       availableByte = 0;

    uint8*      pPayloadAddress = MMP_NULL;
    uint8*      pAdapationFieldAddress = MMP_NULL;
    uint8*      pDataAddress = MMP_NULL;
    int32       counterGap = 0;
    uint32      errorCount = 0;

    sync_byte = pPacketAddress[0];
    // transport_error_indicator
    if (MMP_FALSE == ptDecoder->bErrorIndicator)
    {
        ptDecoder->bErrorIndicator =
            ((pPacketAddress[1] & 0x80) ? MMP_TRUE : MMP_FALSE);
        if (ptDecoder->bErrorIndicator)
            errorCount++;
    }

    payload_unit_start_indicator =
        (pPacketAddress[1] & 0x40) >> 6;

    PID =
        (((uint32)pPacketAddress[1] & 0x1F) << 8) | (uint32) pPacketAddress[2];
    adaptation_field_control =
        (pPacketAddress[3] & 0x30) >> 4;

    continuity_counter =
        pPacketAddress[3] & 0x0F;
    pDataAddress = &pPacketAddress[4];
    //
    // 1. check sync_byte
    //
    if (sync_byte != VALID_SYNC_BYTE)
    {
        return;
    }

    if (!ptDecoder->bFirstPacket)
    {
        //
        // 2. check continuity_counter
        //
        if (continuity_counter == ptDecoder->continuity_counter)
        {
            // duplicate
            return;
        }

        if (continuity_counter != ((ptDecoder->continuity_counter + 1) & CONTINUITY_COUNTER_MASK))
        {
            // discontinuity
            ptDecoder->bDiscontinuity = MMP_TRUE;

            counterGap = continuity_counter - ptDecoder->continuity_counter;
            if (counterGap <= 0)
                counterGap += 15;
            errorCount += counterGap;
        }
    }
    else
        ptDecoder->bFirstPacket = MMP_FALSE;
    ptDecoder->continuity_counter = continuity_counter;

    //
    // 3. check adaptation_field_control
    //
    if ((adaptation_field_control & PAYLOAD_EXIST) != PAYLOAD_EXIST)
    {
        // payload doesn't exist. that is all stuffing bytes behind,
        // therefore, simply return.
        return;
    }

    if ((adaptation_field_control & ADAPTATION_FIELD_EXIST)
     == ADAPTATION_FIELD_EXIST)
    {
        pAdapationFieldAddress = pDataAddress;

        adaptation_field_length =
            pAdapationFieldAddress[0];

        // add 1 for "adaptation_field_length" occupied 1 byte
        pPayloadAddress = (pAdapationFieldAddress + 1 + adaptation_field_length);
    }
    else
    {
        pPayloadAddress = pDataAddress;
    }

    availableByte = (pPacketAddress + TS_PACKET_BYTE_NUMBER) - pPayloadAddress;

    //
    // 4. check payload_unit_start_indicator
    //
    if (payload_unit_start_indicator)
    {
        // parse the previous completed PES packet.

        // two possible cases are described below:
        // 1. when the stream is video stream because the PES_packet_length is
        // possible to be 0. therefore, it's impossible for us to identify the
        // end of the PES packet unless we get the start of next PES packet.

        // 2. some TS packets lost. this mechanism helps us to process the
        // uncompleted PES packet as well.
        if (ptDecoder->ptBuildingPes
         && ptDecoder->ptBuildingPes->pData)
        {
            // 已有資料存在 output buffer 內
            _PES_ParsePes(ptDecoder);

            // Reinit the some parameters of the decoder to handle
            // the next PES packet.
            ptDecoder->ptBuildingPes = MMP_NULL;
            ptDecoder->priorHeaderReadByte = 0;
            PalMemset(ptDecoder->pPriorHeader, 0x0, PES_PRIOR_HEADER_SIZE);
            ptDecoder->bDiscontinuity = ptDecoder->bErrorIndicator
                                      = MMP_NULL;
        }

        // create a new PES_INFO to store the PES
        ptDecoder->ptBuildingPes = _PES_CreatePesInfo(ptDecoder, PID);
        // No memory. TODO: Error handler?
        if (MMP_NULL == ptDecoder->ptBuildingPes)
            return;
        ptDecoder->ptBuildingPes->errorCount += errorCount;
        if (MMP_SUCCESS != queueMgr_GetFree(ptDecoder->ptQueue, (void**)&ptDecoder->ptBuildingPes->pData))
        {
            ptDecoder->ptBuildingPes = MMP_NULL;
            return;
        }

        _PES_GatherPesSegment(availableByte, pPayloadAddress, ptDecoder);
    }
    else // payload_unit_start_indicator == 0
    {
        // a broken packet, we don't need to take care of it
        //if (MMP_NULL == ptDecoder->ptFirstSegment)
        if (MMP_NULL == ptDecoder->ptBuildingPes
         || MMP_NULL == ptDecoder->ptBuildingPes->pData)
            return;

        ptDecoder->ptBuildingPes->errorCount += errorCount;
        _PES_GatherPesSegment(availableByte, pPayloadAddress, ptDecoder);
    }
}

//=============================================================================
/**
 * Destroy the PES packet.
 *
 * @param ptPesInfo A struct with PES packet information and data payload.
 * @return none
 */
//=============================================================================
void
pesPacket_DestroyPes(   // destroy 有兩種，一種是 get ready，一種是根本不理並保持 write index 的位置
    PES_INFO* ptPesInfo)
{
    if (ptPesInfo)
    {
        pesPacket_DestroyPesData(ptPesInfo);
    }
}

void
pesPacket_DestroyPesData(
    PES_INFO* ptPesInfo)
{
    ptPesInfo->pData        = MMP_NULL;
    ptPesInfo->gatherSize   = 0;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Create a new info container to store result of parsing, including whole ES
 * data.
 *
 * @param elementary_ID The unique PID of the elementary stream.
 * @return              PES_INFO* to store result of parsing, including whole
 *                      ES data.
 */
//=============================================================================
static PES_INFO*
_PES_CreatePesInfo(
    PES_DECODER*    ptDecoder,
    uint32          elementary_ID)
{
    PES_INFO* ptPesInfo = MMP_NULL;

    ptPesInfo = &ptDecoder->tBuildingPes;

    if (ptPesInfo)
    {
        PalMemset(ptPesInfo, 0x0, sizeof(PES_INFO));
        ptPesInfo->elementary_PID = elementary_ID;
    }

    return ptPesInfo;
}

//=============================================================================
/**
 * Create a new segment to store part of PES packet.
 *
 * @param size  Total needed size for the segment.
 * @return      PES_SEGMENT* to store the PES data of this TS packet.
 */
//=============================================================================
static MMP_BOOL
_PES_CollectSegment(
    PES_DECODER*    ptDecoder,
    uint32          size,
    uint8*          pDataSource)
{
    MMP_UINT remainSize = ptDecoder->queueSampleSize - ptDecoder->ptBuildingPes->gatherSize;

    if (remainSize < size)
        return MMP_FALSE;   // no enough space

    PalMemcpy(ptDecoder->ptBuildingPes->pData + ptDecoder->ptBuildingPes->gatherSize,
              pDataSource,
              size);

    ptDecoder->ptBuildingPes->gatherSize += size;
    ptDecoder->ptBuildingPes->pPayloadEndAddress = ptDecoder->ptBuildingPes->pData
                                                 + ptDecoder->ptBuildingPes->gatherSize;
    return MMP_TRUE;
}

//=============================================================================
/**
 * Decode the PES section of incoming TS packet
 *
 * @param availableByte     The rest avaiable bits of the TS packet.
 * @param pPayloadAddress   The start address of payload of TS packet.
 * @param ptDecoder The private PES decoder with segment data.
 * @return none
 */
//=============================================================================
static void
_PES_GatherPesSegment(
    int32           availableByte,
    uint8*          pPayloadAddress,
    PES_DECODER*    ptDecoder)
{
    uint32      remainingByte = 0;

    // The rest of TS packet is greater than 0
    while (availableByte > 0)
    {
        // We have to collect the partial PES header section to help us
        // to evaluate the integrity of a PES packet. Therefore, we have
        // to get the first 6 bytes data (till PES_packet_length).
        if (ptDecoder->priorHeaderReadByte < PES_PRIOR_HEADER_SIZE)
        {
            if (availableByte >= (int32)(PES_PRIOR_HEADER_SIZE - ptDecoder->priorHeaderReadByte))
            {
                // 先處理 prior header 的部份
                PalMemcpy(&(ptDecoder->pPriorHeader[ptDecoder->priorHeaderReadByte]),
                          pPayloadAddress,
                          (PES_PRIOR_HEADER_SIZE - ptDecoder->priorHeaderReadByte));

                // Parse the first 6 bytes to get the PES_packet_length for us to
                // evaluate the end of PES packet.
                _PES_ParsePesPriorHeader(ptDecoder);

                ptDecoder->priorHeaderReadByte = PES_PRIOR_HEADER_SIZE;
                continue;
            }
            else
            {
                // Copy the data to the special array which keeps the first 6 bytes
                // of PES header only.
                PalMemcpy(&(ptDecoder->pPriorHeader[ptDecoder->priorHeaderReadByte]),
                          pPayloadAddress,
                          availableByte);

                // Create a new segment which keeps the data of part of PES packet.
                if (!_PES_CollectSegment(ptDecoder, availableByte, pPayloadAddress))
                {
                    // 失敗時的error handling
                }

                ptDecoder->priorHeaderReadByte += availableByte;
                return;
            }
        }
        else // ptDecoder->priorHeaderReadByte == PES_PRIOR_HEADER_SIZE.
        {
            // Full PES packet can be completed in this TS packet.
            if ((ptDecoder->ptBuildingPes->PES_packet_length != 0)
             && ((ptDecoder->ptBuildingPes->gatherSize + availableByte)
                 >= (ptDecoder->ptBuildingPes->PES_packet_length + PES_PRIOR_HEADER_SIZE)))
            {
                remainingByte = (ptDecoder->ptBuildingPes->PES_packet_length)
                             - (ptDecoder->ptBuildingPes->gatherSize - PES_PRIOR_HEADER_SIZE);

                if (!_PES_CollectSegment(ptDecoder, remainingByte, pPayloadAddress))
                {
                    // 失敗時的error handling
                }

                _PES_ParsePes(ptDecoder);

                // Reinit the some parameters of the decoder to handle
                // the next PES packet.
                ptDecoder->ptBuildingPes = MMP_NULL;
                ptDecoder->priorHeaderReadByte = 0;
                PalMemset(ptDecoder->pPriorHeader, 0x0, PES_PRIOR_HEADER_SIZE);
                ptDecoder->bDiscontinuity = ptDecoder->bErrorIndicator
                                          = MMP_FALSE;
                return;
            }
            else
            {
                // video
                if (!_PES_CollectSegment(ptDecoder, availableByte, pPayloadAddress))   // 將 _PES_CreateSegment() 改成 ring alloc
                {
                    // 失敗時的error handling
                }

                availableByte = 0;
            }
        }
    }
}

//=============================================================================
/**
 * Parse/Decode a PES packet prior header (the first 6 bytes of the header).
 *
 * @param ptDecoder The private PES decoder with segment data.
 * @return none
 */
//=============================================================================
static void
_PES_ParsePesPriorHeader(
    PES_DECODER*    ptDecoder)
{
    if (ptDecoder  && ptDecoder->ptBuildingPes)
    {
        ptDecoder->ptBuildingPes->packet_start_code_prefix =
            ptDecoder->pPriorHeader[0] << 16 |
            ptDecoder->pPriorHeader[1] << 8 |
            ptDecoder->pPriorHeader[2];
        ptDecoder->ptBuildingPes->stream_id =
            ptDecoder->pPriorHeader[3];
        ptDecoder->ptBuildingPes->PES_packet_length =
            ptDecoder->pPriorHeader[4] << 8 |
            ptDecoder->pPriorHeader[5];
    }
}

//=============================================================================
/**
 * Parse/Decode a complete PES packet.
 *
 * @param ptDecoder The private PES decoder with segment data.
 * @return none
 */
//=============================================================================
static void
_PES_ParsePes(
    PES_DECODER*    ptDecoder)
{
    uint32  PTS_DTS_flags = 0;
    uint32  PES_header_data_length = 0;

    uint8*  pData = MMP_NULL;
    uint8*  pPayloadStartAddress = MMP_NULL;
    uint8*  pPtsStartAddress = MMP_NULL;
    uint8*  pDtsStartAddress = MMP_NULL;

    if (MMP_NULL == ptDecoder || MMP_NULL == ptDecoder->ptBuildingPes)
        return;

    // Invalid start_code_prefix.
    if (ptDecoder->ptBuildingPes->packet_start_code_prefix != PES_START_CODE_PREFIX)
    {
        pesPacket_DestroyPes(ptDecoder->ptBuildingPes);
        ptDecoder->ptBuildingPes = MMP_NULL;
        return;
    }

    // The operation of segment chain and buffer copy is failed.
    pData = ptDecoder->ptBuildingPes->pData;

    // The information of partial header section is retrieved already.
    // (packet_start_code_prefix, stream_id, and PES_packet_length).
    // Therefore, we jump the address behide the field PES_packet_length.
    pData += PES_PRIOR_HEADER_SIZE;

    switch (ptDecoder->ptBuildingPes->stream_id)
    {
        case PROGRAM_STREAM_MAP_ID:
        case PADDING_STREAM_ID:
        case PRIVATE_STREAM_2_ID:
        case ECM_ID:
        case EMM_ID:
        case PROGRAM_STREAM_DIRECTORY_ID:
        case DSMCC_STREAM_ID:
        case ITU_H222_1_TYPE_E_STREAM_ID:
            pesPacket_DestroyPes(ptDecoder->ptBuildingPes); // 改寫, 這是release pes header info 和 data 的部份
            ptDecoder->ptBuildingPes = MMP_NULL;
            return;
        default:
            // Currently we only interests the PTS_DTS_flags and
            // PES_header_data_length

            // '10'                         2 bits
            // PES_scrambling_control       2 bits
            // PES_priority                 1 bit
            // data_alignment_indicator     1 bit
            // Copyright                    1 bit
            // original_or_copy             1 bit
            // PTS_DTS_flags                2 bits
            // ESCR_flag                    1 bit
            // ES_rate_flag                 1 bit
            // DSM_trick_mode_flag          1 bit
            // Additional_copy_info_flag    1 bit
            // PES_CRC_flag                 1 bit
            // PES_extension_flag           1 bit
            // BitStreamKit_SkipBits(&tBitStream, 16);

            ptDecoder->ptBuildingPes->PES_header_data_length =
            PES_header_data_length =
                pData[2];

            pPayloadStartAddress = &pData[3] +
                                   PES_header_data_length;
            break;
    }

    if (ptDecoder->bDiscontinuity)
        ptDecoder->ptBuildingPes->errorFlag |= PES_DISCONTINUITY;

    if (ptDecoder->bErrorIndicator)
        ptDecoder->ptBuildingPes->errorFlag |= PES_TRANSPORT_ERROR_INDICATOR;

    ptDecoder->ptBuildingPes->pPayloadStartAddress = pPayloadStartAddress;

    if (ptDecoder->pfCallback)
    {
        ptDecoder->pfCallback(ptDecoder->pCallbackData,
                              ptDecoder->ptBuildingPes);
    }
    queueMgr_SetReady(ptDecoder->ptQueue, (void**)&ptDecoder->ptBuildingPes->pData, &ptDecoder->ptBuildingPes->gatherSize);

    // 在這裡設法讓之後會自動取出下一個空的 video sample來填充
    // do destroy pes header info
    ptDecoder->ptBuildingPes = MMP_NULL;
}
