#ifndef __ite_ts_extractor_H_i70AldZ9_fgMa_L3u1_ao6R_z1r8zEBUztdr__
#define __ite_ts_extractor_H_i70AldZ9_fgMa_L3u1_ao6R_z1r8zEBUztdr__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "ts_extractor_demod_attr.h"
#include "ts_extractor_err.h"
//=============================================================================
//                Constant Definition
//=============================================================================
#if (CFG_DEMOD_SUPPORT_COUNT > 0)
    #if (CFG_DEMOD_SUPPORT_COUNT > 3)
        #define TSE_MAX_DEMOD_PER_AGGRE         4
    #else
        #define TSE_MAX_DEMOD_PER_AGGRE         CFG_DEMOD_SUPPORT_COUNT // 4
    #endif
#else
    #define TSE_MAX_DEMOD_PER_AGGRE         1
#endif

#define TSE_MAX_SERVICE_PRE_DEMOD       4

/**
 * ts extractor control cmd
 **/
typedef enum TSE_CTRL_CMD_T
{
    TSE_CTRL_UNKNOW     = 0,
    TSE_CTRL_SET_DEMOD_STATUS,

}TSE_CTRL_CMD;


/**
 * ts extractor ts aggre chip type
 **/
typedef enum TSE_AGGRE_TYPE_T
{
    TSE_AGGRE_TYPE_UNKNOW       = 0,
    TSE_AGGRE_TYPE_ENDEAVOUR,

}TSE_AGGRE_TYPE;

/**
 * ts extractor aggregation module bus type
 **/
typedef enum TSE_AGGRE_BUS_TYPE_T
{
    TSE_AGGRE_BUS_UNKNOW    = 0,
    TSE_AGGRE_BUS_I2C,
    TSE_AGGRE_BUS_USB,
}TSE_AGGRE_BUS_TYPE;

/**
 * ts extractor ts split level type
 **/
typedef enum TSE_SPLIT_LEVEL_T
{
    TSE_SPLIT_NONE       = 0,
    TSE_SPLIT_DEMOD,
    TSE_SPLIT_SERVICE,
}TSE_SPLIT_LEVEL;

/**
 * demod H/W engine status
 **/
typedef enum TSE_DEMOD_STATUS_TAG
{
    TSE_DEMOD_STATUS_UNKNOW    = 0,
    TSE_DEMOD_STATUS_IDLE      = 1,
    TSE_DEMOD_STATUS_RUNNING   = 2,

}TSE_DEMOD_STATUS;

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * user msg box arguments (for api)
 **/
typedef struct TSE_USER_ARG_T
{
#define TSE_USER_ARG_TYPE_SCAN_STATUS      0x4000
#define TSE_USER_ARG_TYPE_SET_RECV_OPR     0x4100
#define TSE_USER_ARG_TYPE_CMD_PKT_RECV     0x4200
#define TSE_USER_ARG_TYPE_GET_USER_INFO    0x4300
#define TSE_USER_ARG_TYPE_GET_VIDEO_INFO   0x4301

    uint32_t        type;

    union{
        struct{
            uint32_t    port_index;   // demod index
            void        *pTunnelInfo[2];
        }scan;

        struct{
            uint8_t     **ppSampleAddr;
            uint32_t    *pSampleLength;
            void        *pTunnelInfo[2];
        }recv_opr;

        struct{
            uint32_t    pid;
            uint8_t     *pPkt_addr;
            uint32_t    pkt_size;
            void        *pTunnelInfo[2];
        }cmd_pkt;

        struct{
            uint32_t    pid;
            void        *pTunnelInfo[2];
        }user_info;

        struct{
            uint32_t    port_index;   // demod index
            uint32_t    v_pid;
            uint32_t    width;
            uint32_t    height;
            uint32_t    srvc_buf_size;
            void        *pTunnelInfo[2];
        }v_info;
    }arg;

}TSE_USER_ARG;

/**
 *  user msg box (for api)
 **/
typedef struct TSE_USER_MBOX_T
{
    uint32_t    (*func)(TSE_USER_ARG *pUser_arg, void *extraData);

    TSE_USER_ARG     tse_user_arg;

}TSE_USER_MBOX;


/**
 * ts receiver operator (tsi/usb/file/...)
 **/
typedef struct TSE_RECV_OPR_T
{
#define TSE_RECV_TYPE_FILE      0x30
#define TSE_RECV_TYPE_TSI       0x31
#define TSE_RECV_TYPE_USB       0x32


    uint32_t        ts_recv_type;

    union{
        // file
        struct{
            uint32_t    index;
            uint8_t     *pTsi_buf[2];
            void        *fin[2];
        }file;

        // tsi
        struct {
            uint32_t    tsi_index;
            void        *data;
        }tsi;
    }privData;

    uint32_t    (*ts_recv_init)(struct TSE_RECV_OPR_T *pTs_recv_opr, void *extraData);
    uint32_t    (*ts_recv_deinit)(struct TSE_RECV_OPR_T *pTs_recv_opr, void *extraData);
    uint32_t    (*ts_recv_turn_on)(struct TSE_RECV_OPR_T *pTs_recv_opr, void *extraData);
    uint32_t    (*ts_recv_turn_off)(struct TSE_RECV_OPR_T *pTs_recv_opr, void *extraData);
    uint32_t    (*ts_recv_get_data)(struct TSE_RECV_OPR_T *pTs_recv_opr,
                                    uint8_t **ppSampleAddr, uint32_t *pSampleSize, void *extraData);

}TSE_RECV_OPR;

/**
 * cmd packet info for ccHDTv
 **/
typedef struct TSE_CMD_PKT_INFO_T
{
    uint32_t        cmd_pkt_pid;
}TSE_CMD_PKT_INFO;


/**
 * initial paraments when create handle
 **/
typedef struct TSE_INIT_PARAM_T
{
    uint32_t            tsi_idx;
    TSE_AGGRE_TYPE      aggre_type;
    bool                bSkip_aggre;
    uint32_t            aggre_idx;
    TSE_AGGRE_BUS_TYPE  aggre_bus_type;
    uint32_t            aggre_i2c_addr;

    uint32_t            total_demod;
    TSE_DEMOD_ATTR      demod_attr[TSE_MAX_DEMOD_PER_AGGRE];

    TSE_SPLIT_LEVEL     ts_split_level;

    // if need, compare the pid for getting service resolution
    TSE_USER_MBOX       get_user_info;

    // for rest ts input operator
    uint32_t  (*reset_ts_recv_opr)(TSE_RECV_OPR *pRecv_opr, void *extraData);

    // for less memory
    bool                bSpareMem;

}TSE_INIT_PARAM;


/**
 * single service info
 **/
typedef struct TSE_SERVICE_INFO_T
{
#define TSE_SRVC_MAX_AUD_COUNT      3

    uint32_t        video_pid;
    uint32_t        videoType;
    uint16_t        audioCount;
    uint32_t        audioPID[TSE_SRVC_MAX_AUD_COUNT];

    uint8_t         *pRing_buf;
    uint32_t        ring_buf_size;

}TSE_SERVICE_INFO;


/**
 * single demod context info
 **/
typedef struct TSE_PORT_INFO_T
{
    uint32_t            total_services;

    TSE_SERVICE_INFO    serviceInfo[TSE_MAX_SERVICE_PRE_DEMOD];

}TSE_PORT_INFO;


/**
 * user info
 **/
typedef struct TSE_USER_INFO_T
{
    uint32_t        total_demod;

    TSE_PORT_INFO   demodInfo[TSE_MAX_DEMOD_PER_AGGRE];

}TSE_USER_INFO;

/**
 * ts extracor sample info
 *
 *
 *                                           |->service_0 (service_idx) => support multi read pointer (customer_idx)
 *                     |->demod_0 (port_idx)-|->service_1 (service_idx) => support multi read pointer (customer_idx)
 * ->ite_ts_extractor -|                     |-> ...
 *                     |
 *                     |                     |->service_0 (service_idx) => support multi read pointer (customer_idx)
 *                     |->demod_1 (port_idx)-|->service_0 (service_idx) => support multi read pointer (customer_idx)
 *                     |                     |-> ...
 *                     |-> ...
 **/
typedef struct TSE_SAMPLE_INFO_T
{
// service index of cmd data stream
#define TSE_CMD_SRVC_INDEX      0xCED1DA

    uint32_t        port_idx;
    uint32_t        service_idx;
    uint32_t        customer_idx;  // support multi read pointer
    uint8_t         *pBufAddr;
    uint32_t        bufLength;
}TSE_SAMPLE_INFO;


/**
 * frequency scan parameters for AP application.
 **/
typedef struct TSE_SCAN_PARAM_T
{
#define TSE_ALL_DEMOD_PORT      0xa11

    // if demod_idx == TSE_ALL_DEMOD_PORT -> all demod scan
    uint32_t        demod_idx; // follow demod_attr

    union{
        struct{
            uint32_t        scanFreq;
            uint32_t        bandwidth;
        }single;

        struct{
            uint32_t        scanFreq[TSE_MAX_DEMOD_PER_AGGRE];
            uint32_t        bandwidth[TSE_MAX_DEMOD_PER_AGGRE];
        }all;
    }channel;

    uint32_t        timeout_ms;
    TSE_USER_MBOX   scan_state_recv;  // for get scan result in AP layer

}TSE_SCAN_PARAM;


/**
 * ts extractor handle
 **/
typedef struct TSE_HANDLE_TAG
{
    uint32_t            reserved;

    TSE_USER_MBOX       scan_state_recv;

}TSE_HANDLE;
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
tse_CreateHandle(
     TSE_HANDLE        **pHTse,
     TSE_INIT_PARAM    *pInitParam,
     void              *extraData);


TSE_ERR
tse_DestroyHandle(
    TSE_HANDLE        **pHTse,
    void              *extraData);


TSE_ERR
tse_Action_Proc(
    TSE_HANDLE     *pHTse,
    void           *extraData);


TSE_ERR
tse_Set_Scan_Info(
    TSE_HANDLE      *pHTse,
    TSE_SCAN_PARAM  *pScan_param,
    void            *extraData);


TSE_ERR
tse_Set_Service_Receive(
    TSE_HANDLE     *pHTse,
    uint32_t       port_idx,
    uint32_t       service_idx,
    bool           bReceive,
    void           *extraData);


TSE_ERR
tse_Get_Service_Sample(
    TSE_HANDLE          *pHTse,
    TSE_SAMPLE_INFO     *pSample_info,
    void                *extraData);


TSE_ERR
tse_Get_Service_Info(
    TSE_HANDLE      *pHTse,
    TSE_USER_INFO   *pUser_info,
    void            *extraData);


TSE_ERR
tse_Control(
    TSE_HANDLE      *pHTse,
    TSE_CTRL_CMD    cmd,
    uint32_t        value,
    void            *extraData);


TSE_ERR
tse_Attach_Cmd_Pkt_Recv(
    TSE_HANDLE          *pHTse,
    uint32_t            port_index,
    TSE_CMD_PKT_INFO   *pCmd_pkt_info,
    void                *extraData);



#ifdef __cplusplus
}
#endif

#endif
