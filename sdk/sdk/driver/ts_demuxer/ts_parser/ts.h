/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file ts.h
 * Used to provide transport stream operations
 * @author I-Chun Lai
 * @version 0.1
 */

#ifndef TS_H
#define TS_H

#include "ite/mmp_types.h"
#include "psi_packet.h"
#include "pes_packet.h"
#include "psi_table_pat.h"
#include "psi_table_pmt.h"
#include "psi_table_nit.h"
#include "psi_table_sdt.h"
#include "psi_table_eit.h"
#include "psi_table_tdt.h"
#include "psi_table_tot.h"
#include "pes_share_info.h"
#include "ts_demuxer_defs.h"
#ifdef ENABLE_DSM_CC
    #include "dsm_cc.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#define TOTAL_PID_COUNT                             (MAX_CHANNEL_COUNT * MAX_PID_COUNT_PER_CHANNEL)
#define TOTAL_PES_COUNT                             TOTAL_PID_COUNT

#define MAX_PID_NUMBER                              (8192)
#define TS_PACKET_SIZE                              (188)
#define VALID_SYNC_BYTE                             (0x47)

#define ACTUAL_NIT_TBL_ID                           (0x40)
#define OTHER_NIT_TBL_ID                            (0x41)

// The table_id of "event_information_section - actual transport_stream,
// schedule" is from 0x50 to 0x5F, the count is 16.
#define MAX_EIT_SCHEDULE_TABLE_COUNT                (16)

#define INVALID_TRANSPORT_STREAM_ID                 (0xFFFFFFFF)
#define INVALID_ORIGINAL_NETWORK_ID                 (0xFFFFFFFF)

// [20101019] vincent: not install service with private temporary use NID and ONID
//#define PRIVATE_TEMPORARY_USE_NID       (0xFF00)

#define NIT_ACTUAL_TABLE_ID                         (0x40)
#define NIT_OTHER_TABLE_ID                          (0x41)
#define SDT_ACTUAL_TABLE_ID                         (0x42)
#define EIT_ACTUAL_PRESENT_FOLLOWING_EVENT_TABLE_ID (0x4E)
#define EIT_ACTUAL_SCHEDULE_EVENT_MIN_TABLE_ID      (0x50)

#if defined SCHEDULE_EVENT_TABLE_NUMBER
    #define EIT_ACTIAL_SCHEDULE_EVENT_MAX_TABLE_ID  (0x4F + SCHEDULE_EVENT_TABLE_NUMBER)
#else
    #define EIT_ACTIAL_SCHEDULE_EVENT_MAX_TABLE_ID  (0x50)
#endif

#define TDT_TABLE_ID                                (0x70)
#define TOT_TABLE_ID                                (0x73)

//=============================================================================
//                              Macro Definition
//=============================================================================

#define TS_SetPatCallBack(TS_DEMUX, PatCallBack)                  {(TS_DEMUX)->pfPatCallBack = (PSI_PAT_CALLBACK)PatCallBack; }
#define TS_SetPmtCallBack(TS_DEMUX, PmtCallBack)                  {(TS_DEMUX)->pfPmtCallBack = (PSI_PMT_CALLBACK)PmtCallBack; }
#define TS_SetNitCallBack(TS_DEMUX, NitCallBack)                  {(TS_DEMUX)->pfNitCallBack = (PSI_NIT_CALLBACK)NitCallBack; }
#define TS_SetSdtCallBack(TS_DEMUX, SdtCallBack)                  {(TS_DEMUX)->pfSdtCallBack = (PSI_SDT_CALLBACK)SdtCallBack; }
#define TS_SetEitPfCallBack(TS_DEMUX, EitPfCallBack)              {(TS_DEMUX)->pfEitPfCallBack = (PSI_EIT_CALLBACK)EitPfCallBack; }
#define TS_SetEitSchCallBack(TS_DEMUX, EitSchCallBack)            {(TS_DEMUX)->pfEitSchCallBack = (PSI_EIT_CALLBACK)EitSchCallBack; }
#define TS_SetEitSectionFilterCallBack(TS_DEMUX, EitSectCallBack) {(TS_DEMUX)->pfEitSectionFilterCallBack = (PSI_SECTION_FILTER_CALLBACK)EitSectCallBack; }
#define TS_SetTdtCallBack(TS_DEMUX, TdtCallBack)                  {(TS_DEMUX)->pfTdtCallBack = (PSI_TDT_CALLBACK)TdtCallBack; }
#define TS_SetTotCallBack(TS_DEMUX, TotCallBack)                  {(TS_DEMUX)->pfTotCallBack = (PSI_TOT_CALLBACK)TotCallBack; }
#define TS_SetPesCallBack(TS_DEMUX, PesCallBack)                  {(TS_DEMUX)->pfPesCallBack = (PES_CALLBACK)PesCallBack; }
#ifdef ENABLE_DSM_CC
    #define TS_SetDsmCcCallBack(TS_DEMUX, DsmCcCallBack)          {(TS_DEMUX)->pfDsmCcCallBack = (DSM_CC_CALLBACK)DsmCcCallBack; }
#endif

#define TS_EnablePid(ptDemux, pid)                                TS_SetPid(ptDemux, pid, MMP_TRUE)
#define TS_DisablePid(ptDemux, pid)                               TS_SetPid(ptDemux, pid, MMP_FALSE)

#define PAT_PID     (0x00)
#define NIT_PID     (0x10)
#define SDT_PID     (0x11)
#define EIT_PID     (0x12)
#define TDT_TOT_PID (0x14)

typedef enum TS_VIDEO_TYPE_TAG
{
    TS_MPEG_VIDEO = 0,
    TS_UNKNOWN_VIDEO
} TS_VIDEO_TYPE;

typedef enum TS_AUDIO_TYPE_TAG
{
    TS_MPEG_AUDIO = 0,
    TS_AC3_AUDIO,
    TS_AAC_AUDIO,
    TS_UNKNOWN_AUDIO
} TS_AUDIO_TYPE;

typedef enum TS_PID_TYPE_TAG
{
    TS_PID_PSI = 0,
    TS_PID_PES,
    TS_PID_DSM_CC
} TS_PID_TYPE;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

// Map to service descriptor (tag value 0x48), define in ETSI EN 300 468.
// This descriptor appears in Service Description Table (SDT).
typedef struct TS_PSI_SDT_ENTRY_TAG
{
    // MMP_UINT32 running_status;   // the status of the service (program)
    // MMP_UINT32 free_CA_mode;     // if one or more stream may be controlled by a CA system

    // service descriptor+
    MMP_UINT32 service_type;           // the type of the service.
                                       // ex. 0x01 => digital television service
                                       //     0x02 => digital radio sound service
                                       //     0x03 => teletext service
                                       //     0x10 => DVB MHP service
    MMP_UINT32 service_provider_name_length;
    MMP_UINT8  *service_provider_name; // the name of the service provider
    MMP_UINT32 service_name_length;
    MMP_UINT8  *service_name;          // the name of the service
    // service descriptor-

    // MMP_UINT32 reference_count;
} TS_PSI_SDT_ENTRY;

// Map to short event descriptor (tag value 0x4D), define in ETSI EN 300 468.
// This descriptor appears in Event Information Table (EIT).
typedef struct TS_PSI_EIT_EVENT_TAG
{
    MMP_UINT32      event_id;
    PSI_MJDBCD_TIME start_time;
    MMP_UINT32      duration;
    // MMP_UINT32      running_status;
    // MMP_UINT32      free_CA_mode;

    // short event descriptor+
    MMP_UINT8  ISO_639_language_code[4];
    MMP_UINT32 event_name_length;
    MMP_UINT8  *event_name;
    MMP_UINT32 text_length;
    MMP_UINT8  *text;
    // short event descriptor-
} TS_PSI_EIT_EVENT;

typedef struct TS_PSI_EIT_TABLE_TAG
{
    MMP_UINT32       totalEventCount;
    TS_PSI_EIT_EVENT *ptEitEvent;           // Eit event array
} TS_PSI_EIT_TABLE;

// Define for "event_information_section - actual transport_stream, present/following"
// (table_id value 0x4E)
typedef struct TS_PSI_EIT_PRESENT_FOLLOWING_TAG
{
    TS_PSI_EIT_EVENT tPresentEvent;
    TS_PSI_EIT_EVENT tFollowingEvent;
} TS_PSI_EIT_PRESENT_FOLLOWING;

// Define for "event_information_section - actual transport_stream, schedule"
// (table_id value 0x50 - 0x5F)
typedef struct TS_PSI_EIT_TAG
{
    MMP_UINT32 version_number;

    // Used to check if we have received the complete schedule table.
    // Anyone who wants to display the schedule events needs to check this value.
    MMP_UINT32       last_table_id;
    TS_PSI_EIT_TABLE table[MAX_EIT_SCHEDULE_TABLE_COUNT];
} TS_PSI_EIT_SCHEDULE;

// Define for Program Specific Information (PSI) transport stream,
// including PAT, PMT, SDT, EIT, etc..
struct PAT_TBL_SECTION
{
    MMP_UINT32 transport_stream_id;
    MMP_UINT32 totalPmtPidCount;
    MMP_UINT32 *pPmtPid;
};

// a link list of PMT to handle the case that
// many PMT have the same PID but with different program number
struct PMT_TBL_SECTION
{
    MMP_UINT32             pmtPid;
    MMP_UINT32             version_number;
    MMP_UINT32             program_number;
    MMP_UINT32             totalEsPidCount;
    MMP_UINT32             *pEsPid;
    struct PMT_TBL_SECTION *ptNextPmt;
};

struct NIT_TBL_SECTION
{
    MMP_UINT32 actual_version_number;
    MMP_UINT32 other_version_number;
};

typedef struct TS_PSI_TAG
{
    // For PAT/PMT/SDT/EIT
    PSI_DECODER *ptPsiDecoder;
    MMP_UINT32  version_number;

    union _table
    {
        struct PAT_TBL_SECTION PAT;
        struct PMT_TBL_SECTION PMT;
        struct NIT_TBL_SECTION NIT;
    } table;
} TS_PSI;

// Define for Packetized Elementary Stream (PES) transport stream.
typedef struct TS_PES_TAG
{
    PES_DECODER *ptPesDecoder;
    //MMP_UINT32      stream_type;
} TS_PES;

#ifdef ENABLE_DSM_CC
typedef struct TS_DSM_CC_TAG
{
    PSI_DECODER *ptPsiDecoder;
} TS_DSM_CC;
#endif

// Define for transport packet identify by PID.
typedef struct TS_PID_TAG
{
    MMP_UINT32 PID;
    MMP_BOOL   bValid;

    TS_PSI     *ptPsi;
    TS_PES     *ptPes;
#ifdef ENABLE_DSM_CC
    TS_DSM_CC  *ptDsmCc;
#endif
} TS_PID;

typedef struct TS_DEMUX_TAG
{
    // All pid
    TS_PID     *ptTsPid[MAX_PID_NUMBER];

    MMP_BOOL   bReceivePat;
    MMP_BOOL   bReceiveSdt;
    MMP_BOOL   bWaitNitReady;
    MMP_UINT32 original_network_id;
    MMP_UINT32 transport_stream_id;
    MMP_BOOL   bEnableEsPID;
    // A temp ptr to keep the actual network of NIT, which is received prior
    // SDT on scan stage. It will cause the NIT table is useless due to no
    // correponding information from SDT to judge which information is
    // necessary or not.
    PSI_NIT_INFO                *ptTmpActualNit;

    PSI_PAT_CALLBACK            pfPatCallBack;
    PSI_PMT_CALLBACK            pfPmtCallBack;
    PSI_NIT_CALLBACK            pfNitCallBack;
    PSI_SDT_CALLBACK            pfSdtCallBack;
    PSI_EIT_CALLBACK            pfEitPfCallBack;
    PSI_EIT_CALLBACK            pfEitSchCallBack;
    PSI_SECTION_FILTER_CALLBACK pfEitSectionFilterCallBack;
    PSI_TDT_CALLBACK            pfTdtCallBack;
    PSI_TOT_CALLBACK            pfTotCallBack;
    PES_CALLBACK                pfPesCallBack;
#ifdef ENABLE_DSM_CC
    DSM_CC_CALLBACK             pfDsmCcCallBack;
#endif

    void                        *pHTsSrvc; // for link service info database
    void                        *pHTsEpg;  // for link epg info database
    void                        *pHTsp;    // for link tsp handle
    bool                        bOnPesOut; // enable pes output
    PID_INFO                    pidInfo_a; // audeo pid info
    PID_INFO                    pidInfo_v; // video pid info
    PID_INFO                    pidInfo_s; // subtitle pid info
    PID_INFO                    pidInfo_t; // teletext pid info
} TS_DEMUX;

//=============================================================================
//                              Function  Definition
//=============================================================================

TS_DEMUX *
TS_Init(
    MMP_BOOL bWaitNit,
    MMP_BOOL bInScan,
    MMP_BOOL bCollectEit);

void
TS_Terminate(
    TS_DEMUX *ptDemux);

void
TS_ReCreateEitHandle(
    TS_DEMUX *ptDemux);

void
TS_CreateEitHandle(
    TS_DEMUX *ptDemux);

MMP_BOOL
TS_DestroyEitHandle(
    TS_DEMUX *ptDemux);

MMP_BOOL
TS_Decode(
    TS_DEMUX  *ptDemux,
    MMP_UINT8 *pData);

MMP_BOOL
TS_SetValidTransportStream(
    TS_DEMUX   *ptTsDemux,
    MMP_UINT32 transport_stream_id,
    MMP_UINT32 original_network_id);

MMP_BOOL
TS_InsertEsPid(
    TS_DEMUX     *ptTsDemux,
    uint         channelId,
    PES_PID_TYPE pesIndex,
    PID_INFO     *ptPidInfo,
    uint8        *pDestBaseAddr);

void
TS_SetPid(
    TS_DEMUX   *ptTsDemux,
    MMP_UINT32 pid,
    MMP_BOOL   bEnable);

#ifdef ENABLE_DSM_CC
MMP_BOOL
TS_InsertDsmCcPid(
    TS_DEMUX   *ptTsDemux,
    MMP_UINT32 PID);

void
TS_DestroyDsmCcPid(
    TS_DEMUX   *ptTsDemux,
    MMP_UINT32 PID);
#endif

#ifdef __cplusplus
}
#endif

#endif