#ifndef __ts_packet_demux_H_gnFDhTVG_iA2N_YcMO_8h2x_Pnp5bUYY9E5a__
#define __ts_packet_demux_H_gnFDhTVG_iA2N_YcMO_8h2x_Pnp5bUYY9E5a__

#ifdef __cplusplus
extern "C" {
#endif


#include "ite_ts_extractor.h"
#include "ts_extractor_defs.h"
#include "ring_buf_opt.h"
//=============================================================================
//                Constant Definition
//=============================================================================
#define TSPD_MAX_PID_NUM                    (8192)
#define TSPD_SRVC_MAX_AUD_NUM               (3)
#define TSPD_MAX_SERVICE_NUM                (14)

#ifdef _MSC_VER // WIN32
    #define TSPD_SRVC_BUF_SIZE_LEVEL_0          (1 << 20) // 1 MBbytes
    #define TSPD_SRVC_BUF_SIZE_LEVEL_1          (2 << 20) // 2 MBbytes
    #define TSPD_SRVC_BUF_SIZE_LEVEL_2          (3 << 20) // 3 MBbytes
    #define TSPD_SRVC_BUF_SIZE_LEVEL_3          (4 << 20) // 4 MBbytes
#else
    #define TSPD_SRVC_BUF_SIZE_LEVEL_0          (1 << 20) // 1 MBbytes
    #define TSPD_SRVC_BUF_SIZE_LEVEL_1          (2 << 20) // 2 MBbytes
    #define TSPD_SRVC_BUF_SIZE_LEVEL_2          (3 << 20) // 3 MBbytes
    #define TSPD_SRVC_BUF_SIZE_LEVEL_3          (4 << 20) // 4 MBbytes
#endif

/**
 * ts packet demux demux level
 **/
typedef enum TSPD_DEMUX_LEVEL_T
{
    TSPD_DEMUX_LEVEL_NONE       = 0,
    TSPD_DEMUX_LEVEL_DEMOD,         // demux demod stream
    TSPD_DEMUX_LEVEL_SERVICE,       // demux server stream
    TSPD_DEMUX_LEVEL_SRVC_PES,      // demux pes stream at one server
}TSPD_DEMUX_LEVEL;

/**
 * record destination cache buf index.
 *  not_used  cmd_pkt  cache_(n-1)      cache_1  cache_0
 *    |--------|--------|-- .......... --|--------|
 *   MSB     MSB-1                               LSB
 **/
#define TSPD_BIT_FIELD_SIZE                 (TSPD_MAX_SERVICE_NUM+2)
typedef struct TSPD_SRVC_MAP_T
{
    uint16_t    bits_field[((TSPD_BIT_FIELD_SIZE)+0xF)>>4];
}TSPD_SRVC_MAP;
//=============================================================================
//                Macro Definition
//=============================================================================
#define TSPD_BIT_SET(pZone_set_member, bit_order)     ((pZone_set_member)->bits_field[(bit_order)>>4] |=  (1<<((bit_order)&0xF)))
#define TSPD_BIT_CLR(pZone_set_member, bit_order)     ((pZone_set_member)->bits_field[(bit_order)>>4] &= ~(1<<((bit_order)&0xF)))
#define TSPD_BIT_IS_SET(pZone_set_member, bit_order)  ((pZone_set_member)->bits_field[(bit_order)>>4] &   (1<<((bit_order)&0xF)))
#define TSPD_BIT_ZERO(pZone_set_member)               memset((void*)(pZone_set_member),0,sizeof(TSPD_SRVC_MAP))
//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * user msg box arguments (for api)
 **/
typedef struct TSPD_USER_MBOX_ARG_T
{
#define TSPD_USER_MBOX_TYPE_SET_PID         0x55
#define TSPD_USER_MBOX_TYPE_GET_USER_INFO   0x56
#define TSPD_USER_MBOX_TYPE_SET_BUF_SIZE    0x57

    uint32_t        type;

    union{
        struct{
            uint32_t    total_service;
            void        *pSrvc_attr;
            void        *pTunnelInfo;
        }set_pid;
    }arg;

}TSPD_USER_MBOX_ARG;


/**
 *  user msg box (for api)
 **/
typedef struct TSPD_USER_MBOX_T
{
    uint32_t    (*func)(TSPD_USER_MBOX_ARG *pUser_mbox_arg, void *extraData);

    TSPD_USER_MBOX_ARG     user_mbox_arg;

}TSPD_USER_MBOX;


/**
 * ts packet demux setup info
 **/
typedef struct TSPD_SETUP_PARAM_T
{
    uint32_t            total_service;

    TSPD_DEMUX_LEVEL    pkt_demux_level;

    TSPD_USER_MBOX      get_user_info;

    TSPD_USER_MBOX      set_srvc_pid;

    // for less memory
    bool                bSpareMem;

}TSPD_SETUP_PARAM;


/**
 * ts packet demux
 **/
typedef struct TSPD_BUF_INFO_T
{
    uint8_t         *pBufAddr;
    uint32_t        bufLength;
}TSPD_BUF_INFO;


/**
 * ts packet demux sample info
 **/
typedef struct TSPD_SAMPLE_INFO_T
{
    uint32_t        service_idx;
    uint32_t        customer_idx;  // support multi read pointer
    uint8_t         *pBufAddr;
    uint32_t        bufLength;
}TSPD_SAMPLE_INFO;


/**
 * single service attrubute
 **/
typedef struct TSPD_SRVC_ATTR_T
{
    bool        bStopCache;
    bool        bGetPat;  // workaround to fix vlc bug (PAT must be got before PMT)

    uint32_t    pat_pid;
    uint32_t    sdt_pid;
    uint32_t    nit_pid;
    uint32_t    tdt_tot_pid;
    uint32_t    pmt_pid;
    uint32_t    video_pid;
    uint32_t    audio_pid[TSPD_SRVC_MAX_AUD_NUM];

    RB_OPT      ring_buf_opr;

    uint32_t    srvc_cache_buf_size;
    uint8_t     *pSrvc_cache_buf;

    // PES info
    // TO DO:

}TSPD_SRVC_ATTR;


/**
 *  ts packet demux Cmd packet attribute
 **/
typedef struct TSPD_CMD_PKT_ATTR_T
{
    uint32_t        cmd_pkt_pid;
}TSPD_CMD_PKT_ATTR;


/**
 *  ts packet demux
 **/
typedef struct TSPD_HANLDE_T
{
    uint32_t            total_service;
    TSPD_SRVC_MAP       pid_mapping[TSPD_MAX_PID_NUM];

    TSPD_DEMUX_LEVEL    pkt_demux_level;

    TSPD_SRVC_ATTR      *pSrvc_attr;

    // ccHDTv
    uint32_t            total_cmd_pkt;
    uint32_t            max_cmd_pkt_array;
    TSPD_CMD_PKT_ATTR   *pCmd_pkt_attr;
    // cmd packet buf cacsh
    RB_OPT              cmd_pkt_buf_opr;
    uint32_t            cmd_pkt_buf_size;
    uint8_t             *pCmd_pkt_buf;

    // for less memory
    bool                bSpareMem;
}TSPD_HANLDE;
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
tspd_CreateHandle(
    TSPD_HANLDE          **ppHTspd,
    TSPD_SETUP_PARAM     *pSetupParam,
    void                 *extraData);


TSE_ERR
tspd_DestroyHandle(
    TSPD_HANLDE     **ppHTspd,
    void            *extraData);


TSE_ERR
tspd_Pkt_Demux(
    TSPD_HANLDE     *pHTspd,
    TSPD_BUF_INFO   *pBuf_info,
    void            *extraData);


TSE_ERR
tspd_Get_Srvc_Stream(
    TSPD_HANLDE         *pHTspd,
    TSPD_SAMPLE_INFO    *pSample_info,
    void                *extraData);


TSE_ERR
tspd_Get_Cmd_Pkt_Stream(
    TSPD_HANLDE         *pHTspd,
    TSPD_SAMPLE_INFO    *pSample_info,
    void                *extraData);


TSE_ERR
tspd_Srvc_Start_Cache(
    TSPD_HANLDE     *pHTspd,
    uint32_t        service_idx,
    void            *extraData);


TSE_ERR
tspd_Srvc_Stop_Cache(
    TSPD_HANLDE     *pHTspd,
    uint32_t        service_idx,
    void            *extraData);


TSE_ERR
tspd_Attach_Cmd_Pkt_Info(
    TSPD_HANLDE         *pHTspd,
    TSPD_CMD_PKT_ATTR   *pCmd_pkt_attr,
    void                *extraData);


TSE_ERR
tspd_Detach_Cmd_Pkt_Info(
    TSPD_HANLDE         *pHTspd,
    TSPD_CMD_PKT_ATTR   *pCmd_pkt_attr,
    void                *extraData);




#ifdef __cplusplus
}
#endif

#endif
