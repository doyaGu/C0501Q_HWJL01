/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file ts.h
 * Used to provide transport stream operations
 * @author I-Chun Lai
 * @version 0.1
 */

#ifndef TS_H
#define TS_H

#include "def.h"
#include "share_info.h"
#include "pes_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

#define TOTAL_PID_COUNT                 (MAX_CHANNEL_COUNT * MAX_PID_COUNT_PER_CHANNEL)
#define TOTAL_PES_COUNT                 TOTAL_PID_COUNT

#define MAX_PID_NUMBER                  (8192)
#define TS_PACKET_SIZE                  (188)
#define VALID_SYNC_BYTE                 (0x47)

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

typedef enum TS_DEMUX_STATE_TAG
{
    TS_DEMUX_STATE_SEARCH_PACKET_START      = 0,
    TS_DEMUX_STATE_LESS_THAN_188
} TS_DEMUX_STATE;

typedef enum TS_BUF_CTRL_STATE_TAG
{
    TS_BUF_CTRL_STATE_CHECK_BUFFER_EMPTY    = 0,
    TS_BUF_CTRL_STATE_DO_DECODE
} TS_BUF_CTRL_STATE;

//=============================================================================
//                              Structure Definition
//=============================================================================

// Define for Packetized Elementary Stream (PES) transport stream.
typedef struct TS_PES_TAG
{
    PES_DECODER*    ptPesDecoder;
    uint32          stream_type;
    uint32          stream_subtype;
    MMP_BOOL        bValid;
} TS_PES;

// Define for transport packet identify by PID.
typedef struct TS_PID_TAG
{
    uint32          PID;
    MMP_BOOL        bValid;

    TS_PES*         ptPes;
} TS_PID;

typedef struct TS_DEMUX_TAG
{
    CHANNEL_INFO*       ptChanInfo;
    // All pid
    TS_PID*             ptTsPid[MAX_PID_NUMBER];

    TS_DEMUX_STATE      demuxState;
    TS_BUF_CTRL_STATE   bufCtrlState;
    uint32              collectedByte;
    uint8               incompleteTsPacket[TS_PACKET_SIZE];
} TS_DEMUX;

//=============================================================================
//                              Function  Definition
//=============================================================================

// parser 的部份要提供的 API 有:
// 1. init
// 2. terminate
// 3. decode
// 4. reset
// 5. add es pid
//      param 1. demux handler
//      param 2. pid
//      param 3. ring start address
//      param 4. ring block size
//      param 5. ring block count
// 6. get next frame (for client)
//      param 1. demux handler
//      param 2. pid
//      param 3. start address of the next frame (return null if there is no new data)
// 7. free frame (for client)
//      param 1. demux handler
//      param 2. pid
//      param 3. start address of the frame
// 8. output buffer control API
//      1. create
//          param 1. ring start address
//          param 2. ring block size
//          param 3. ring block count
//          return output buffer id
//      2. delete
//      3. get block remain size (identified by output buffer id)
//      4. get free (get next free block)
//      5. set ready
//      6. get ready
//      7. set free
MMP_BOOL
TS_Init(
    uint            channelId,
    CHANNEL_INFO*   ptChanInfo);

void
TS_Terminate(
    uint        channelId);

uint32
TS_Decode(
    uint        channelId,
    uint8*      pData,
    uint32      size);

MMP_BOOL
TS_InsertEsPid(
    uint        channelId,
    uint        pesIndex,
    PID_INFO*   ptPidInfo,
    uint8*      pRiscBaseAddr);

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef TS_H
 