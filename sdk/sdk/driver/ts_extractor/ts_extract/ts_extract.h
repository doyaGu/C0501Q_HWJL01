#ifndef __ts_extract_H_vc16nxnc_PEkv_i4VO_iIkJ_w6qfbw6qwOl5__
#define __ts_extract_H_vc16nxnc_PEkv_i4VO_iIkJ_w6qfbw6qwOl5__

#ifdef __cplusplus
extern "C" {
#endif

#include "ite_ts_extractor.h"
#include "ts_extractor_defs.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#if (CFG_DEMOD_SUPPORT_COUNT > 0)
    #if (CFG_DEMOD_SUPPORT_COUNT > 3)
        #define TSEXT_MAX_PKT_ANALYZER_NUM         4
    #else
        #define TSEXT_MAX_PKT_ANALYZER_NUM         CFG_DEMOD_SUPPORT_COUNT // 4
    #endif
#else
    #define TSEXT_MAX_PKT_ANALYZER_NUM         1
#endif

#define TSEXT_SRVC_MAX_AUD_COUNT            4

/**
 * ts extract packet process mode type
 **/
typedef enum TSEXT_PKT_PROC_MODE_T
{
    TSEXT_PKT_PROC_IDLE         = 0,
    TSEXT_PKT_PROC_ANALYZE,
    TSEXT_PKT_PROC_SPLIT,

}TSEXT_PKT_PROC_MODE;

/**
 * ts extract packet split level type
 **/
typedef enum TSEXT_PKT_SPLIT_LEVEL_T
{
    TSEXT_PKT_SPLIT_NONE         = 0,
    TSEXT_PKT_SPLIT_DEMOD,
    TSEXT_PKT_SPLIT_SERVICE,

}TSEXT_PKT_SPLIT_LEVEL;

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * user msg box arguments (for api)
 **/
typedef struct TSEXT_USER_MBOX_ARG_T
{
#define TSEXT_USER_MBOX_TYPE_DEF            0xee0
#define TSEXT_USER_MBOX_TYPE_SORT_SRVC      0xee1
#define TSEXT_USER_MBOX_TYPE_CMD_PKT        0xee2

    uint32_t        type;

    union{
        struct{
            uint32_t    port_index;   // demod index
            void        *pTunnelInfo;
        }def;

        struct{
            uint32_t    port_index;   // demod index
            void        *pTunnelInfo;
        }sort_srvc;

        struct{
            uint32_t    port_index;   // demod index
            void        *pTunnelInfo;
        }cmd_pkt;
    }arg;

}TSEXT_USER_MBOX_ARG;

/**
 *  user msg box (for api)
 **/
typedef struct TSEXT_USER_MBOX_T
{
    uint32_t    (*func)(TSEXT_USER_MBOX_ARG *pUser_mbox_arg, void *extraData);

    TSEXT_USER_MBOX_ARG     user_mbox_arg;

}TSEXT_USER_MBOX;

/**
 * ts extract init paraments
 **/
typedef struct TSEXT_INIT_PARAM_T
{
    uint32_t                act_pkt_size;   // action packet size from argge module
    bool                    bBy_Pass_Tss;   // no argge module flag

    TSEXT_PKT_SPLIT_LEVEL   pkt_split_level;

    // for less memory
    bool                    bSpareMem;


}TSEXT_INIT_PARAM;

/**
 * info of aggre module
 **/
typedef struct TSEXT_PASSPORT_INFO_T
{
    // Coordinate with aggre module
    uint32_t        tag_len;

    // reg[tag_value3] << 24 | reg[tag_value2] << 16 | reg[tag_value1] << 8 | reg[tag_value0]
    uint32_t        tag_value;

}TSEXT_PASSPORT_INFO;

/**
 *  ts extract Cmd packet attribute
 **/
typedef struct TTSEXT_CMD_PKT_ATTR_T
{
    uint32_t        cmd_pkt_pid;
}TSEXT_CMD_PKT_ATTR;

/**
 * ts extract
 **/
typedef struct TSEXT_BUF_INFO_T
{
    uint8_t         *pBufAddr;
    uint32_t        bufLength;
}TSEXT_BUF_INFO;


/**
 * ts extract sample info
 *
 *
 *                                       |->service_0 (service_idx) => support multi read pointer (customer_idx)
 *                 |->demod_0 (port_idx)-|->service_1 (service_idx) => support multi read pointer (customer_idx)
 *  -->ts_extract -|                     |-> ...
 *                 |
 *                 |                     |->service_0 (service_idx) => support multi read pointer (customer_idx)
 *                 |->demod_1 (port_idx)-|->service_0 (service_idx) => support multi read pointer (customer_idx)
 *                 |                     |-> ...
 *                 |-> ...
 **/
typedef struct TSEXT_SAMPLE_INFO_T
{
// service index of cmd data stream
#define TSEXT_CMD_SRVC_INDEX      0xCED1DA

    uint32_t        port_idx;
    uint32_t        service_idx;
    uint32_t        customer_idx;  // support multi read pointer
    uint8_t         *pBufAddr;
    uint32_t        bufLength;
}TSEXT_SAMPLE_INFO;

/**
* ts extract single service pid info
**/
typedef struct TSEXT_SRVC_PID_INFO_T
{
    // PID info
    uint32_t            pmt_pid;
    uint32_t            version_number;
    uint32_t            programNumber;
    uint32_t            videoPID;
    uint32_t            videoType;
    uint16_t            audioCount;
    uint32_t            audioPID[TSEXT_SRVC_MAX_AUD_COUNT];
    uint32_t            proportion; // this service weighting

    uint32_t            width;
    uint32_t            height;
    uint8_t             *pRing_buf;
    uint32_t            ring_buf_size;
}TSEXT_SRVC_PID_INFO;

/**
 * ts extract packet analysis info
 **/
typedef struct TSEXT_PKT_ANAL_INFO_T
{
    TSEXT_PKT_PROC_MODE     proc_mode;
    bool                    bRe_init;

    void                    *pHTspa;
    void                    *pHTspd;

    uint32_t                total_service;
    uint32_t                pat_version_number;
    uint32_t                sdt_version_number;

    bool                    bPat_ready;
    bool                    bSdt_ready;
    bool                    bPmt_ready;

    uint32_t                used_srvc_pid_info_idx;
    TSEXT_SRVC_PID_INFO     *pSrvc_pid_info; // create pointer by pat total services

    // callback to ap layer for get resolution
    uint32_t                port_index;
    TSE_USER_MBOX           get_info;

    // ccHDTv

}TSEXT_PKT_ANAL_INFO;

/**
 * ts extract user info
 **/
typedef struct TSEXT_USER_INFO_T
{
    uint32_t    total_service[TSEXT_MAX_PKT_ANALYZER_NUM];
}TSEXT_USER_INFO;

/**
 * ts extract handle
 **/
typedef struct TSEXT_HANDLE_T
{
    TSEXT_PKT_ANAL_INFO     *pPkt_analyzer_info[TSEXT_MAX_PKT_ANALYZER_NUM];

    TSE_USER_MBOX           scan_state_recv; // for high level application program

    // callback to ap layer for get resolution
    TSE_USER_MBOX           get_info;

}TSEXT_HANDLE;
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
tsExt_CreateHandle(
    TSEXT_HANDLE        **pHTsExt,
    TSEXT_INIT_PARAM    *pInitParam,
    void                *extraData);


TSE_ERR
tsExt_DestroyHandle(
    TSEXT_HANDLE      **pHTsExt,
    void              *extraData);


TSE_ERR
tsExt_Add_Passport_Info(
    TSEXT_HANDLE            *pHTsExt,
    TSEXT_PASSPORT_INFO     *pPassportInfo,
    void                    *extraData);


TSE_ERR
tsExt_Add_Cmd_Pkt_Info(
    TSEXT_HANDLE        *pHTsExt,
    uint32_t            port_index,
    TSEXT_CMD_PKT_ATTR  *pCmd_pkt_attr,
    void                *extraData);


TSE_ERR
tsExt_Extract(
    TSEXT_HANDLE    *pHTsExt,
    TSEXT_BUF_INFO  *pBuf_info,
    void            *extraData);


TSE_ERR
tsExt_Set_Pkt_Proc_Mode(
    TSEXT_HANDLE            *pHTsExt,
    uint32_t                port_idx,
    TSEXT_PKT_PROC_MODE     mode,
    void                    *extraData);


TSE_ERR
tsExt_Set_Service_Recv(
    TSEXT_HANDLE    *pHTsExt,
    uint32_t        port_idx,
    uint32_t        service_idx,
    bool            bReceive,
    void            *extraData);


TSE_ERR
tsExt_Get_Sample(
    TSEXT_HANDLE        *pHTsExt,
    TSEXT_SAMPLE_INFO   *pSample_info,
    void                *extraData);



#ifdef __cplusplus
}
#endif

#endif
