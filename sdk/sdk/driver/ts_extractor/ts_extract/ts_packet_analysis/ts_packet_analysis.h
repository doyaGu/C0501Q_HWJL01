#ifndef __ts_packet_analysis_H_cpUuYbuj_PrCk_V2Ry_wq6B_TBVJlTxQDIB2__
#define __ts_packet_analysis_H_cpUuYbuj_PrCk_V2Ry_wq6B_TBVJlTxQDIB2__

#ifdef __cplusplus
extern "C" {
#endif

#include "ite_ts_extractor.h"
#include "ts_extractor_defs.h"

#include "psi_table_cfg.h"
#include "pes_packet_decode.h"
#include "psi_packet_decode.h"
//=============================================================================
//                Constant Definition
//=============================================================================
#define TSPA_ENABLE_PES_DECODE        0
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * ts packet analyseiser setup info
 **/
typedef struct TSPA_SETUP_PARAM_T
{
    bool        bInScan;
    bool        bWaitNit;
    bool        bEnableEsPID; // not ready

}TSPA_SETUP_PARAM;

/**
 * Define for Packetized Elementary Stream (PES) transport stream.
 **/
typedef struct TSPA_PES_T
{
    PES_PKT_DECODER     *pPesPktDecoder;
    PES_STREAM_ID       pes_stream_id;
    uint32_t            stream_subtype;
    bool                bValid;
}TSPA_PES;

/**
 *  Pat section info
 **/
typedef struct TSPA_PAT_SECT_T
{
    uint32_t    transport_stream_id;
    uint32_t    totalPmtPidCount;
    uint32_t    *pPmtPid;
}TSPA_PAT_SECT;

/**
 *  Pmt section info
 **/
typedef struct TSPA_PMT_SECT_T
{
    struct TSPA_PMT_SECT_T  *pNextPmt;

    uint32_t    pmtPid;
    uint32_t    version_number;
    uint32_t    program_number;
    uint32_t    totalEsPidCount;
    uint32_t    *pEsPid;
}TSPA_PMT_SECT;

/**
 * Define for program specific information (PSI) transport stream.
 **/
typedef struct TSPA_PSI_T
{
    // For PAT/PMT/SDT/EIT
    PSI_PKT_DECODER     *pPsiPktDecoder;

    uint32_t            version_number;

    union _table{
        TSPA_PAT_SECT   pat;
        TSPA_PMT_SECT   pmt;
    } table;
}TSPA_PSI;

/**
 * Define for transport packet identify by PID.
 **/
typedef struct TSPA_PID_T
{
    uint32_t        pid;
    bool            bValid;
    uint32_t        proportion; // pid Statistics

    TSPA_PSI        *pPsi_handler;
    TSPA_PES        *pPes_handler;
} TSPA_PID;

/**
 * user msg box arguments (for api)
 **/
typedef struct TSPA_USER_MBOX_ARG_T
{
#define TSPA_USER_MBOX_TYPE_PAT     1
#define TSPA_USER_MBOX_TYPE_PMT     2
#define TSPA_USER_MBOX_TYPE_SDT     3

    uint32_t        type;

    union{
        struct{
            void    *pPat_info;
            void    *pTunnelInfo;
        }pat;

        struct{
            void    *pPmt_info;
            void    *pTunnelInfo;
        }pmt;

        struct{
            void    *pSdt_info;
            void    *pTunnelInfo;
        }sdt;
    }arg;

}TSPA_USER_MBOX_ARG;

/**
 *  user msg box (for api)
 **/
typedef struct TSPA_USER_MBOX_T
{
    uint32_t    (*func)(TSPA_USER_MBOX_ARG *pUser_mbox_arg, void *extraData);

    TSPA_USER_MBOX_ARG     tspa_user_mbox_arg;

}TSPA_USER_MBOX;

/**
 * ts packet analysiser
 **/
typedef struct TS_PACKET_ANAL_T
{
    // All pid
    TSPA_PID          *pTsPid_handler[TSPA_MAX_PID_NUM];

    uint32_t          transport_stream_id;

    bool              bReceivePat;
    bool              bReceiveSdt;
    bool              bWaitNitReady;

    bool              bEnableEsPID; // enable PES pid_handler

    uint32_t          video_stream_type;  // video codec type
    bool              bGetPesVideo; // get pes video attribute or not

    // for api
    TSPA_USER_MBOX    pat_user_mbox;
    TSPA_USER_MBOX    pmt_user_mbox;
    TSPA_USER_MBOX    sdt_user_mbox;

}TS_PACKET_ANAL;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
TSE_ERR
tspa_CreateHandle(
    TS_PACKET_ANAL      **ppHTspa,
    TSPA_SETUP_PARAM    *pSetupParam,
    void                *extraData);


TSE_ERR
tspa_DestroyHandle(
    TS_PACKET_ANAL      **ppHTspa,
    void                *extraData);


TSE_ERR
tspa_Analysis(
    TS_PACKET_ANAL      *pHTspa,
    uint8_t             *pPacketBuf,
    void                *extraData);

#ifdef __cplusplus
}
#endif

#endif
