/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file pes_packet.h
 * Used to process PES packet decode issue
 * @author Steven Hsiao
 * @version 0.1
 */
#ifndef PES_PACKET_H_H4s1VdLL_qQQ7_oNfd_hVoi_qkSyndrxXEaB__
#define PES_PACKET_H_H4s1VdLL_qQQ7_oNfd_hVoi_qkSyndrxXEaB__

#include "pes_queue_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

#define PES_DISCONTINUITY             (0x10000000)
#define PES_TRANSPORT_ERROR_INDICATOR (0x20000000)

// The PES header start with packet_start_code_prefix(24) + stream_id(8)
// + PES_packet_length(16) = 6 bytes. After read these 6 bytes, then we
// are able to get the packet length of PES packet.
#define PES_PRIOR_HEADER_SIZE         (6)

typedef enum STREAM_TYPE_TAG {
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
} STREAM_TYPE;

//=============================================================================
//                              Macro Definition
//=============================================================================

typedef struct PES_INFO_TAG PES_INFO;

// the callback function will be called after the decode of PAT table is
// is done.
typedef void (*PES_CALLBACK) (void *pCallbackData, PES_INFO *ptPesInfo);

//=============================================================================
//                              Structure Definition
//=============================================================================

struct PES_INFO_TAG
{
    uint32 elementary_PID;

    uint32 packet_start_code_prefix;
    uint32 stream_id;
    uint32 PES_packet_length;

    uint32 errorFlag;
    uint32 errorCount;
    // iclai+
    // teletext needs the following fields to verify if the PES data is correct
    // or not
    uint32       PES_header_data_length;
    // iclai-

    uint8        *pData; // Include header and payload
    uint8        *pPayloadStartAddress;
    uint8        *pPayloadEndAddress;
    uint32       gatherSize;

    //
    PES_PID_TYPE pesIndex;
};

typedef struct PES_DECODER_TAG
{
    QUEUE_DATA_BLOCK *ptQueue;
    uint             queueSampleSize;

    uint32           elementary_PID;
    uint32           continuity_counter;

    uint8            pPriorHeader[PES_PRIOR_HEADER_SIZE];
    uint32           priorHeaderReadByte;    // total bytes read for prior header

    MMP_BOOL         bDiscontinuity;
    MMP_BOOL         bErrorIndicator;

    // Whether the first packet be handled by the decoder yet?
    MMP_BOOL         bFirstPacket;

    PES_INFO         *ptBuildingPes;
    PES_INFO         tBuildingPes;

    PES_CALLBACK     pfCallback;
    void             *pCallbackData;
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
PES_DECODER *
pesPacket_AttachDecoder(
    uint         channelId,
    uint         pesIndex,
    PID_INFO     *ptPidInfo,
    uint8        *pRiscBaseAddr,
    PES_CALLBACK pfCallback,
    void         *pCallbackData);

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
    PES_DECODER *ptDecoder);

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
    PES_DECODER *ptDecoder,
    uint8       *pPacketAddress);

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
    PES_INFO *ptPesInfo);

void
pesPacket_DestroyPesData(
    PES_INFO *ptPesInfo);

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef PES_PACKET_H