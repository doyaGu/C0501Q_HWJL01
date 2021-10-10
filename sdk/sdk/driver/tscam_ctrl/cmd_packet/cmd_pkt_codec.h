#ifndef __cmd_pkt_codec_H_cxQip3iI_C7qi_fprA_qq8c_8XgAF00IXSTk__
#define __cmd_pkt_codec_H_cxQip3iI_C7qi_fprA_qq8c_8XgAF00IXSTk__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "cmd_pkt_register.h"
#include "tscam_cmd_service.h"
#include "tscam_ctrl_def.h"
#include "tscam_ctrl.h"
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
#define _SET_DWORD(ptr, dwValue)        do{ (*(ptr)) = (((dwValue) >> 24) & 0xFF); ((ptr)++);\
                                            (*(ptr)) = (((dwValue) >> 16) & 0xFF); ((ptr)++);\
                                            (*(ptr)) = (((dwValue) >> 8) & 0xFF); ((ptr)++);\
                                            (*(ptr)) = ((dwValue) & 0xFF); ((ptr)++);\
                                        }while(0)

#define _SET_WORD(ptr, wValue)          do{ (*(ptr)) = (((wValue) >> 8) & 0xFF); ((ptr)++);\
                                            (*(ptr)) = ((wValue) & 0xFF); ((ptr)++);\
                                        }while(0)

#define _SET_BYTE(ptr, value)           do{(*(ptr)) = ((value) & 0xFF); ((ptr)++);}while(0)

#define _SET_STRING(ptr, pStr, size)    do{memcpy((ptr), (pStr), (size)); (ptr) += (size);}while(0)

#define _SET_PAYLOAD(ptr, pData, size)  do{memcpy((ptr), (pData), (size)); (ptr) += (size);}while(0)

#define _GET_DWORD(ptr)                 (*(ptr)<<24 | *((ptr)+1)<<16 | *((ptr)+2)<<8 | *((ptr)+3))
#define _GET_WORD(ptr)                  (*(ptr)<<8 | *((ptr)+1))
#define _GET_BYTE(ptr)                  *(ptr)
//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct CMD_PKT_ARG_T
{
#define CMD_PKT_ARG_TYPE_SET_CODEC      0x2225

    uint32_t        type;

    union{
        struct{
            uint32_t            cmd_code;
            CMD_PKT_CODEC       cmd_pkt_encode;
            CMD_PKT_CODEC       cmd_pkt_decode;
            CMD_CTXT_CREATE     cmd_ctxt_create;
            CMD_CTXT_DESTROY    cmd_ctxt_destroy;
        }set_codec;
    }arg;

}CMD_PKT_ARG;


/**
 * cmd packet initial info
 **/
typedef struct CMD_PKT_INIT_INFO_T
{
    uint32_t    (*set_codec)(struct CMD_PKT_INIT_INFO_T *pInit_info, CMD_PKT_ARG *pArg, void *extraData);

    void        *pTunnelInfo;
}CMD_PKT_INIT_INFO;


/**
 * return channel packet header
 **/
typedef struct CMD_RCP_HDR_T
{
    uint16_t    rx_dev_addr_id;
    uint16_t    tx_dev_addr_id;
    uint16_t    total_pkt_num;
    uint16_t    pkt_num;
    uint16_t    seq_num;
    uint16_t    pkt_length;
}CMD_RCP_HDR;


/**
 * cmd packet pareser info (decode)
 **/
typedef struct CMD_PKT_PARSER_T
{
    uint32_t        bCollecteDone;
    uint8_t         *pCmd_pkt_buf;    // input packet buffer
    uint32_t        cmd_pkt_buf_size; // input packet size

    uint32_t        cmd_info_size; // structure info instance size + cmd_ctxt_size
    uint8_t         *pCur_cmd_ctxt;  // current pointer of cmd_ctxt
    uint8_t         *pEnd_cmd_ctxt;  // end pointer of cmd_ctxt

    // cmd service detail data
    void            *pCmd_pkt_privData;

    // user operator
    TSCM_USER_OPR   *pUser_cmd_pkts_opr;
}CMD_PKT_PARSER;


/**
 * cmd packet muxer info(encode)
 **/
typedef struct CMD_PKT_MUXER_T
{
    uint32_t        bSplitDone;
    uint32_t        pkt_size;
    uint32_t        rcp_cmd_data_size; // max size of a return_channel_cmd_data
    uint32_t        pkt_rcp_data_offset; // Retrun Channel Packet offset in a packet
    uint8_t         *pCmd_pkts_buf;
    uint32_t        cmd_pkts_buf_size;
    uint8_t         *pCur_cmd_ctxt;  // current pointer of cmd_ctxt
    uint8_t         *pEnd_cmd_ctxt;  // end pointer of cmd_ctxt

    uint16_t        total_pkt_num;
    uint16_t        pkt_num;
    uint16_t        pkt_length;

    // cmd service detail data
    void            *pCmd_pkt_privData;

    // user operator
    TSCM_USER_OPR   *pUser_cmd_pkts_opr;
}CMD_PKT_MUXER;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
uint32_t
cmd_pkt_Init(
    CMD_PKT_INIT_INFO   *pInit_info,
    void                *extraData);

uint32_t
cmd_pkt_General_Cmd_Ctxt_Del(
    bool bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData);


uint32_t
cmd_pkt_General_Cmd_Pkt_Encode(
    void    *input_info,
    void    *output_info,
    void    *extraData);


#ifdef __cplusplus
}
#endif

#endif
