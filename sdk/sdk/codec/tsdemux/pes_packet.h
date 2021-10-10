/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file pes_packet.h
 * Used to process PES packet decode issue
 * @author Steven Hsiao
 * @version 0.1
 */
#ifndef PES_PACKET_H
#define PES_PACKET_H

#include "type.h"
#include "queue_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

#define PES_DISCONTINUITY               (0x10000000)
#define PES_TRANSPORT_ERROR_INDICATOR   (0x20000000)

// The PES header start with packet_start_code_prefix(24) + stream_id(8)
// + PES_packet_length(16) = 6 bytes. After read these 6 bytes, then we
// are able to get the packet length of PES packet.
#define PES_PRIOR_HEADER_SIZE           (6)


//=============================================================================
//                              Macro Definition
//=============================================================================

typedef struct PES_INFO_TAG PES_INFO;

// the callback function will be called after the decode of PAT table is
// is done.
typedef void (*PES_CALLBACK) (void* pCallbackData, PES_INFO* ptPesInfo);

//=============================================================================
//                              Structure Definition
//=============================================================================

struct PES_INFO_TAG
{
    uint32      elementary_PID;

    uint32      packet_start_code_prefix;
    uint32      stream_id;
    uint32      PES_packet_length;

    uint32      errorFlag;
    uint32      errorCount;
    // iclai+
    // teletext needs the following fields to verify if the PES data is correct
    // or not
    uint32      PES_header_data_length;
    // iclai-

    uint8*      pData; // Include header and payload
    uint8*      pPayloadStartAddress;
    uint8*      pPayloadEndAddress;
    uint32      gatherSize;
};

typedef struct PES_DECODER_TAG
{
    QUEUE_DATA_BLOCK*   ptQueue;
    uint                queueSampleSize;

    uint32              elementary_PID;
    uint32              continuity_counter;

    uint8               pPriorHeader[PES_PRIOR_HEADER_SIZE];
    uint32              priorHeaderReadByte; // total bytes read for prior header

    MMP_BOOL            bDiscontinuity;
    MMP_BOOL            bErrorIndicator;

    // Whether the first packet be handled by the decoder yet?
    MMP_BOOL            bFirstPacket;

    PES_INFO*           ptBuildingPes;
    PES_INFO            tBuildingPes;

    PES_CALLBACK        pfCallback;
    void*               pCallbackData;
} PES_DECODER;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
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
    void*               pCallbackData);

//=============================================================================
/**
 * Detach the PES decoder from the TS parser system.
 *
 * @param ptDecoder The decoder to deocde a incoming TS packet.
 * @return none
 */
//=============================================================================
void
pesPacket_DetachDecoder(
    PES_DECODER*    ptDecoder);


//=============================================================================
/**
 * Inject a TS packet into a Pes decoder for content decoding.
 *
 * @param ptDecoder         The decoder to deocde a incoming TS packet.
 * @param pPacketAddress    The pointer is pointing to the start of a 188 bytes
 *                          TS packet.
 * @return none
 */
//=============================================================================
void
pesPacket_DecodePacket(
    PES_DECODER*    ptDecoder,
    uint8*          pPacketAddress);


//=============================================================================
/**
 * Destroy the PES packet.
 *
 * @param ptPesInfo A struct with PES packet information and data payload.
 * @return none
 */
//=============================================================================
void
pesPacket_DestroyPes(
    PES_INFO*       ptPesInfo);

void
pesPacket_DestroyPesData(
    PES_INFO*       ptPesInfo);

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef PES_PACKET_H

