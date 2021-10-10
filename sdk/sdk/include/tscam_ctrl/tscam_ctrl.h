#ifndef __tscam_ctrl_H_7VrFao9g_EVNb_RahV_uDmJ_CrQRm0dAIIA5__
#define __tscam_ctrl_H_7VrFao9g_EVNb_RahV_uDmJ_CrQRm0dAIIA5__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>
#include "tscam_ctrl_err.h"
#include "tscam_cmd_service.h"
//=============================================================================
//                Constant Definition
//=============================================================================
/**
 * ts camera transmit type
 **/
typedef enum TSCM_TYPE_T
{
    TSCM_TYPE_UNKNOW        = 0,
    TSCM_TYPE_SIMPLEX_RS232,
    TSCM_TYPE_DUPLEX_RS232,
}TSCM_TYPE;


/**
 * ts camera the priority of cmd
 **/
typedef enum TSCM_CMD_PRIORITY_T
{
    TSCM_CMD_PRIORITY_APPT       = 0, // appointment
    TSCM_CMD_PRIORITY_ER,             // emergency
}TSCM_CMD_PRIORITY;


/**
 * ts camera stream packet format
 **/
typedef enum TSCM_STREAM_PKT_FMT_T
{
    TSCM_STREAM_PKT_TS              = 0x188,
    TSCM_STREAM_PKT_RS232_422_485   = 0x184,
}TSCM_STREAM_PKT_FMT;

//=============================================================================
//                Macro Definition
//=============================================================================
struct TSCM_CMD_BOX_T;
struct TSCM_DESC_T;
struct TSCM_HANDLE_T;

/**
 * Receiver flow   : H/W module -> cmd_packet_decode -> user_handler
 * Transmitter flow: cmd_packet_encoder -> user_hanlder -> H/W module
 **/
typedef TSCM_ERR (*CMD_HANDLER)(struct TSCM_CMD_BOX_T *pCmd_box, void *extraData);
//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * ts camera: msg box arguments
 **/
typedef struct TSCM_ARG_T
{
#define TSCM_AGR_TYPE_USER_HANDLE_RECV         0xCED000  // arg context will be different with TSCM_AGR_TYPE_USER_HANDLE_TRANS
#define TSCM_AGR_TYPE_USER_HANDLE_TRANS        0xCED001  // arg context will be different with TSCM_AGR_TYPE_USER_HANDLE_RECV

    uint32_t        type;

    union{
        struct{
            uint32_t                cmd_code;
            TSCM_CMD_PRIORITY       priority;
            CMD_HANDLER             handler;
            void                    *pCmd_info; // cmd structure instance
            void                    *pTunnel_info;
            TSCM_STREAM_PKT_FMT     stream_pkt_fmt;
            uint32_t                rx_dev_addr_id;
            uint32_t                tx_dev_addr_id;
            uint8_t                 *pPkt_start_addr;
            uint32_t                pkt_size;
        }user_handle;

        struct{
            uint32_t    arg_0;
            uint32_t    arg_1;
            uint32_t    arg_2;
            uint32_t    arg_3;
        }def;
    }arg;

}TSCM_ARG;


/**
 * ts camera: user operator
 **/
typedef struct TSCM_USER_OPR_T
{
    union{
        // def
        struct {
            void        *pTunnel_info;
        }def;
    }privData;

    void*   (*heap_cmd_buffer)(struct TSCM_USER_OPR_T *pUser_opr, uint32_t requestSize, uint32_t *realSize);
    void    (*free_cmd_buffer)(struct TSCM_USER_OPR_T *pUser_opr, void *ptr);

}TSCM_USER_OPR;


/**
 *  ts camera: msg box
 **/
typedef struct TSCM_MBOX_T
{
    uint32_t    (*func)(TSCM_ARG *pArg, void *extraData);

    TSCM_ARG     box_arg;

}TSCM_MBOX;


/**
 * ts camrera: setup hanlde info
 **/
typedef struct TSCM_SETUP_INFO_T
{
    TSCM_TYPE               tscm_type;
    TSCM_STREAM_PKT_FMT     stream_recv_pkt_fmt;
    TSCM_STREAM_PKT_FMT     stream_trans_pkt_fmt;

    TSCM_USER_OPR           user_cmd_pkts_opr;

    TSCM_ERR    (*reset_desc)(struct TSCM_DESC_T *pDesc, void *extraData);
}TSCM_SETUP_INFO;


/**
 * ts camera: initialize protocol info
 **/
typedef struct TSCM_INIT_INFO_T
{
    uint32_t    reserve;
}TSCM_INIT_INFO;


/**
 * ts camera: stream received info
 **/
typedef struct TSCM_RECV_INFO_T
{
    uint32_t    stream_buf_size;
    uint8_t     *pSteam_buf;
}TSCM_RECV_INFO;


/**
 * ts camera: stream transported info
 **/
typedef struct TSCM_TRANS_INFO_T
{
    uint32_t    stream_buf_size;
    uint8_t     *pSteam_buf;
}TSCM_TRANS_INFO;


/**
 * ts camera: stream analysis info
 **/
typedef struct TSCM_ANAL_INFO_T
{
    uint32_t                stream_buf_size;
    uint8_t                 *pSteam_buf;

    // packet format setting
    //TSCM_STREAM_PKT_FMT     stream_pkt_format;
}TSCM_ANAL_INFO;


/**
 * ts camera: stream generate info
 **/
typedef struct TSCM_GEN_INFO_T
{
    uint32_t    stream_buf_size;
    uint8_t     *pSteam_buf;
}TSCM_GEN_INFO;


/**
 * ts camera: H/W layer descriptor
 **/
typedef struct TSCM_DESC_T
{
    char                *name;
    struct TSCM_DESC_T  *next;
    TSCM_TYPE           id;

    void        *pExtraInfo;

    TSCM_ERR    (*init)(struct TSCM_HANDLE_T *pHTscam, TSCM_INIT_INFO *pInit_info, void *extraData);
    TSCM_ERR    (*deinit)(struct TSCM_HANDLE_T *pHTscam, void *extraData);
    TSCM_ERR    (*stream_recv)(struct TSCM_HANDLE_T *pHTscam, TSCM_RECV_INFO *pRecv_info, void *extraData);
    TSCM_ERR    (*stream_trans)(struct TSCM_HANDLE_T *pHTscam, TSCM_TRANS_INFO *pTrans_info, void *extraData);
    TSCM_ERR    (*control)(struct TSCM_HANDLE_T *pHTscam, TSCM_ARG *pArg, void *extraData);

}TSCM_DESC;


/**
 * ts camera: cmd box (cmd unit, for queue)
 **/
typedef struct TSCM_CMD_BOX_T
{
    struct TSCM_CMD_BOX_T  *next, *prev;

    TSCM_ARG     box_arg;

}TSCM_CMD_BOX;


/**
 * ts camera: single cmd handler info (user layer)
 **/
typedef struct TSCM_CMD_HANDLE_INFO_T
{
    uint32_t            cmd_code;
    CMD_HANDLER         user_hanlder;

    void                *pTunnel_info;

    TSCM_CMD_PRIORITY   priority;
}TSCM_CMD_HANDLE_INFO;


/**
 * ts camera: handle
 **/
typedef struct TSCM_HANDLE_T
{
    TSCM_DESC       cur_desc;

    // received cmd queue
    uint32_t        recv_cmd_num;
    TSCM_CMD_BOX    *pCmd_recv_head;
    TSCM_CMD_BOX    *pCmd_recv_queue;

    // wait to transport cmd queue
    uint32_t        trans_cmd_num;
    TSCM_CMD_BOX    *pCmd_trans_head;
    TSCM_CMD_BOX    *pCmd_trans_queue;

    void            *pUser_tunnel_info[2];
}TSCM_HANDLE;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
TSCM_ERR
tscam_CreateHandle(
    TSCM_HANDLE     **ppHTscam,
    TSCM_SETUP_INFO *pSetup_info,
    void            *extraData);


TSCM_ERR
tscam_DestroyHandle(
    TSCM_HANDLE     **ppHTscam,
    void            *extraData);


TSCM_ERR
tscam_Init(
    TSCM_HANDLE         *pHTscam,
    TSCM_INIT_INFO      *pInit_info,
    void                *extraData);


TSCM_ERR
tscam_Deinit(
    TSCM_HANDLE         *pHTscam,
    void                *extraData);


TSCM_ERR
tscam_Stream_Recv(
    TSCM_HANDLE     *pHTscam,
    TSCM_RECV_INFO  *pRecv_info,
    void            *extraData);


TSCM_ERR
tscam_Stream_Trans(
    TSCM_HANDLE     *pHTscam,
    TSCM_TRANS_INFO *pTrans_info,
    void            *extraData);


TSCM_ERR
tscam_Cotrol(
    TSCM_HANDLE     *pHTscam,
    TSCM_ARG        *pArg,
    void            *extraData);


TSCM_ERR
tscam_Stream_Analyse(
    TSCM_HANDLE     *pHTscam,
    TSCM_ANAL_INFO  *pAnal_info,
    void            *extraData);


TSCM_ERR
tscam_Stream_Destroy(
    TSCM_HANDLE     *pHTscam,
    TSCM_GEN_INFO   *pGenInfo,
    void            *extraData);


TSCM_ERR
tscam_Stream_Generate(
    TSCM_HANDLE     *pHTscam,
    TSCM_GEN_INFO   *pGenInfo,
    void            *extraData);


TSCM_ERR
tscam_Attach_Cmd_Handler(
    TSCM_HANDLE             *pHTscam,
    TSCM_CMD_HANDLE_INFO    *pCmd_handle_info,
    void                    *extraData);


TSCM_ERR
tscam_Detach_Cmd_Handler(
    TSCM_HANDLE             *pHTscam,
    TSCM_CMD_HANDLE_INFO    *pCmd_handle_info,
    void                    *extraData);


TSCM_ERR
tscam_Cmd_Destroy(
    TSCM_HANDLE     *pHTscam,
    TSCM_CMD_BOX    **ppCmd_box,
    void            *extraData);


TSCM_ERR
tscam_Cmd_Fetch(
    TSCM_HANDLE     *pHTscam,
    TSCM_CMD_BOX    **ppCmd_box,
    void            *extraData);


TSCM_ERR
tscam_Cmd_Post(
    TSCM_HANDLE     *pHTscam,
    TSCM_CMD_BOX    *pCmd_box,
    void            *extraData);


#ifdef __cplusplus
}
#endif

#endif
