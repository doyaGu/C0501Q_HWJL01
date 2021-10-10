
#include "queue_template.h"
#include "register_template.h"
#include "tscam_ctrl_cfg.h"
#include "tscam_ctrl_def.h"
#include "cmd_pkt_codec.h"
#include "tscam_ctrl.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define TSCAM_VALID_SYNC_BYTE               (0x47)
#define TSCAM_TS_HDR_SIZE                   (4)
#define TSCAM_LEADING_TAG_SIZE              (1)
#define TSCAM_END_TAG_SIZE                  (1)

/**
 * unit: byte
 * |---------|----|------------|---------------|------------|--------|-------|
 * |ts_hdr(4)|#(1)| rcp_hdr(12)| rc_cmd_data(n)|check_sum(1)|dummy(m)|0x0d(1)|
 * |---------|----|------------|-------------------------------------|-------|
 *                             |<--- TSCAM_RC_CMD_DATA_MAX_SIZE  --->|
 **/
#define TSCAM_TS_PKT_SIZE                   188
#define TSCAM_RS232_PKT_SIZE                184
#define TSCAM_RC_CMD_DATA_MAX_SIZE          169
#define TSCAM_MAX_QUEUE_SIZE                5

typedef enum TSCAM_STREAM_STATE_T
{
    TSCAM_STREAM_STATE_SEARCH_PKT_START  = 0x00,
    TSCAM_STREAM_STATE_LESS_PKT_SIZE     = 0x22,
}TSCAM_STREAM_STATE;


/**
 * ts camera cmd service type index
 **/
typedef enum TSCM_CMD_SRVC_TYPE_T
{
    TSCM_CMD_SRVC_CCHDTV_IN    = 0,
    TSCM_CMD_SRVC_CCHDTV_OUT,
    TSCM_CMD_SRVC_DEV_MGT_IN,
    TSCM_CMD_SRVC_DEV_MGT_OUT,
    TSCM_CMD_SRVC_DEV_IO_IN,
    TSCM_CMD_SRVC_DEV_IO_OUT,
    TSCM_CMD_SRVC_IMG_IN,
    TSCM_CMD_SRVC_IMG_OUT,
    TSCM_CMD_SRVC_MEDIA_IN,
    TSCM_CMD_SRVC_MEDIA_OUT,
    TSCM_CMD_SRVC_PTZ_IN,
    TSCM_CMD_SRVC_PTZ_OUT,
    TSCM_CMD_SRVC_VIDEO_ANAL_IN,
    TSCM_CMD_SRVC_VIDEO_ANAL_OUT,
    TSCM_CMD_SRVC_META,

    TSCM_CMD_SRVC_MAX
}TSCM_CMD_SRVC_TYPE;

/**
 * ts camera directly report flag to user hanlder
 **/
typedef struct TSCAM_ACTIVE_FLAG_T
{
    uint16_t    bits_field[((MAX_CMD_NUM_PER_SERVICE)+0xF)>>4];
}TSCAM_ACTIVE_FLAG;

//=============================================================================
//                Macro Definition
//=============================================================================
#define TSCAM_BIT_SET(pZone_set_member, bit_order)     ((pZone_set_member)->bits_field[(bit_order)>>4] |=  (1<<((bit_order)&0xF)))
#define TSCAM_BIT_CLR(pZone_set_member, bit_order)     ((pZone_set_member)->bits_field[(bit_order)>>4] &= ~(1<<((bit_order)&0xF)))
#define TSCAM_BIT_IS_SET(pZone_set_member, bit_order)  ((pZone_set_member)->bits_field[(bit_order)>>4] &   (1<<((bit_order)&0xF)))
#define TSCAM_BIT_ZERO(pZone_set_member)               memset((void*)(pZone_set_member),0,sizeof(TSCAM_ACTIVE_FLAG))

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * ts camera single cmd attribute
 **/
typedef struct TSCM_CMD_SRVC_ATTR_T
{
    //uint32_t        cmd_type;
    CMD_HANDLER         user_hanlder;
    CMD_PKT_CODEC       cmd_pkt_encode;
    CMD_PKT_CODEC       cmd_pkt_decode;
    CMD_CTXT_CREATE     cmd_ctxt_create;
    CMD_CTXT_DESTROY    cmd_ctxt_destroy;
    void                *pTunnel_info;
    void                *pPriv_info[2];
}TSCM_CMD_SRVC_ATTR;

/**
 * ts camera internal mgr
 **/
typedef struct TSCAM_DEV_T
{
    TSCM_HANDLE            hTscam;

    TSCM_USER_OPR          user_cmd_pkts_opr;
    TSCM_STREAM_PKT_FMT    stream_recv_pkt_fmt;
    TSCM_STREAM_PKT_FMT    stream_trans_pkt_fmt;

    pthread_mutex_t        tscm_mutex;

    uint16_t               seq_num; // counter of trans_cmd_packet

    uint32_t               act_recv_cmd_code; // One cmd MUST be transmitted continuously.
    uint16_t               prev_pkt_num;

    // packet parsing
    TSCAM_STREAM_STATE     stream_state;
    uint32_t               collectedByte;
    uint8_t                *pIncompletePktCache;

    TSCAM_ACTIVE_FLAG      cmd_active_flag[TSCM_CMD_SRVC_MAX];
    TSCM_CMD_SRVC_ATTR     *pCmd_srvc_attr[TSCM_CMD_SRVC_MAX][MAX_CMD_NUM_PER_SERVICE];

}TSCAM_DEV;

//=============================================================================
//                Global Data Definition
//=============================================================================
uint32_t  tscmMsgOnFlag = 0x1;
//=============================================================================
//                Private Function Definition
//=============================================================================
DEFINE_REGISTER_TEMPLATE(TSCM_DESC, TSCM_TYPE);
DEFINE_QUEUE_TEMPLATE(TSCM_CMD_BOX);

static void
_tscam_Register_all(
    void)
{
    static int bInitialized = 0;

    if( bInitialized )
        return;
    bInitialized = 1;

    REGISTER_ITEM(TSCM_DESC, SIMPLEX_RS232, simplex_rs232);
}


static int
_tscam_search_cmd_box(
    int             cmpMode,
    TSCM_CMD_BOX    *pNode,
    void            *pattern)
{
    return (pNode->box_arg.arg.user_handle.cmd_code == (uint32_t)pattern ) ? 1 : 0;
}

static uint32_t
_tscam_cmd_analyse(
    TSCAM_DEV       *pTscamDev,
    uint8_t         *pCmd_pkt_addr,
    uint32_t        cmd_pkt_size,
    void            *extraData)
{
    /**
     * unit: byte
     * |---------|----|------------|---------------|------------|--------|-------|
     * |ts_hdr(4)|#(1)| rcp_hdr(12)| rc_cmd_data(n)|check_sum(1)|dummy(m)|0x0d(1)|
     * |---------|----|------------|-------------------------------------|-------|
     *                             |<--- TSCAM_RC_CMD_DATA_MAX_SIZE  --->|
     **/

    uint32_t            result = 0;
    uint8_t             *pCur = pCmd_pkt_addr;
    CMD_PKT_CODEC       pf_cmd_pkt_decoder = 0;
    CMD_CTXT_CREATE     pf_cmd_ctxt_create = 0;
    CMD_CTXT_DESTROY    pf_cmd_ctxt_destroy = 0;
    CMD_HANDLER         pf_user_hanlder = 0;
    void                **ppParser_info = 0;
    void                *pTunnel_info = 0;
    TSCAM_ACTIVE_FLAG   *pCmd_active_flag = 0;
    TSCM_CMD_SRVC_TYPE  cmd_srvc_type = TSCM_CMD_SRVC_MAX;

    do{
        uint8_t         *pRcp_hdr = 0;
        CMD_RCP_HDR     rcp_hdr = {0};
        uint16_t        cmd_code = (-1);
        uint16_t        sub_cmd_code = (-1);
        uint8_t         check_sum = (-1);

        if( !pCur || (*pCur) != '#' || (*(pCur + cmd_pkt_size-1)) != 0x0d )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, Not cmd packet !!");
            break;
        }

        pRcp_hdr = (++pCur);

        // return channel packet header
        rcp_hdr.rx_dev_addr_id = _GET_WORD(pCur); pCur += 2;
        rcp_hdr.tx_dev_addr_id = _GET_WORD(pCur); pCur += 2;
        rcp_hdr.total_pkt_num  = _GET_WORD(pCur); pCur += 2;
        rcp_hdr.pkt_num        = _GET_WORD(pCur); pCur += 2;
        rcp_hdr.seq_num        = _GET_WORD(pCur); pCur += 2;
        rcp_hdr.pkt_length     = _GET_WORD(pCur); pCur += 2;

        //-----------------------------------------
        // verify check_sum value (rx_dev_addr_id doesn't include check_sum, defined by spec)
        check_sum = _tscm_gen_check_sum((pRcp_hdr + 2), rcp_hdr.pkt_length + sizeof(CMD_RCP_HDR) - 2);
        if( check_sum != _GET_BYTE(pCur + rcp_hdr.pkt_length) )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, check_sum is NOT mapping !!");
            break;
        }

        // One cmd MUST be transmitted continuously.
        do{
            // start from index "1"
            if( rcp_hdr.pkt_num == 1 )
            {
                cmd_code = _GET_WORD(pCur + 4);
                pTscamDev->act_recv_cmd_code = cmd_code;
                pTscamDev->prev_pkt_num = 0;
            }
            else
                cmd_code = pTscamDev->act_recv_cmd_code;
        }while(0);

        if( rcp_hdr.pkt_num != (uint16_t)(pTscamDev->prev_pkt_num + 1) )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, cmd packet loss (prev_index=%d, cur_index=%d) !!",
                    pTscamDev->prev_pkt_num, rcp_hdr.pkt_num);
            break;
        }

        pTscamDev->prev_pkt_num = rcp_hdr.pkt_num;

        sub_cmd_code = (cmd_code & CMD_SERVICE_CMD_CODE_MASK);

        switch( cmd_code & CMD_SERVICE_TYPE_MASK )
        {
            case CCHDTV_INPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_CCHDTV_IN;        break;
            case CCHDTV_OUTPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_CCHDTV_OUT;       break;
            case DEV_MGT_INPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_DEV_MGT_IN;       break;
            case DEV_MGT_OUTPUT_SERVICE_TAG:    cmd_srvc_type = TSCM_CMD_SRVC_DEV_MGT_OUT;      break;
            case DEV_IO_INPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_DEV_IO_IN;        break;
            case DEV_IO_OUTPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_DEV_IO_OUT;       break;
            case IMG_SERVICE_INPUT_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_IMG_IN;           break;
            case IMG_SERVICE_OUTPUT_TAG:        cmd_srvc_type = TSCM_CMD_SRVC_IMG_OUT;          break;
            case MEDIA_INPUT_SERVICE_TAG:       cmd_srvc_type = TSCM_CMD_SRVC_MEDIA_IN;         break;
            case MEDIA_OUTPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_MEDIA_OUT;        break;
            case PTZ_INPUT_SERVICE_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_PTZ_IN;           break;
            case PTZ_OUTPUT_SERVICE_TAG:        cmd_srvc_type = TSCM_CMD_SRVC_PTZ_OUT;          break;
            case VIDEO_ANA_INPUT_SERVICE_TAG:   cmd_srvc_type = TSCM_CMD_SRVC_VIDEO_ANAL_IN;    break;
            case VIDEO_ANA_OUTPUT_SERVICE_TAG:  cmd_srvc_type = TSCM_CMD_SRVC_VIDEO_ANAL_OUT;   break;
            case META_STRM_SERVICE_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_META;             break;
        }

        if( cmd_srvc_type == TSCM_CMD_SRVC_MAX ||
            sub_cmd_code >= MAX_CMD_NUM_PER_SERVICE )
            break;

        if( !pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code] )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, pCmd_srvc_attr[%d][%d] = Null !!", cmd_srvc_type, sub_cmd_code);
            break;
        }

        pf_cmd_pkt_decoder  = pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->cmd_pkt_decode;
        pf_cmd_ctxt_create  = pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->cmd_ctxt_create;
        pf_cmd_ctxt_destroy = pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->cmd_ctxt_destroy;
        pf_user_hanlder     = pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->user_hanlder;
        ppParser_info       = &pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->pPriv_info[1];
        pTunnel_info        = pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->pTunnel_info;
        pCmd_active_flag    = &pTscamDev->cmd_active_flag[cmd_srvc_type];

        // call cmd packet codec handler
        if( pf_cmd_pkt_decoder )
        {
            uint32_t        output_info_ptr = 0;
            CMD_PKT_PARSER  *pCmd_pkt_parser = 0;

            if( !(*ppParser_info) )
            {
                pCmd_pkt_parser = tscm_malloc(sizeof(CMD_PKT_PARSER));
                if( !pCmd_pkt_parser )
                {
                    tscm_msg_ex(TSCM_MSG_ERR, "err, Allocate fail !!");
                    break;
                }
                memset(pCmd_pkt_parser, 0x0, sizeof(CMD_PKT_PARSER));
                (*ppParser_info) = pCmd_pkt_parser;
            }
            else
                pCmd_pkt_parser = (CMD_PKT_PARSER*)(*ppParser_info);

            pCmd_pkt_parser->pUser_cmd_pkts_opr = &pTscamDev->user_cmd_pkts_opr;
            pCmd_pkt_parser->pCmd_pkt_buf       = pCur;
            pCmd_pkt_parser->cmd_pkt_buf_size   = rcp_hdr.pkt_length;

            pf_cmd_pkt_decoder((void*)pCmd_pkt_parser, (void*)&output_info_ptr, extraData);

            // call back to user handle (for API layer)
            if( pCmd_pkt_parser->bCollecteDone == true )
            {
                TSCM_CMD_BOX    *pTscm_cmd_box = 0;

                pTscamDev->act_recv_cmd_code = (-1);
                if( TSCAM_BIT_IS_SET(pCmd_active_flag, sub_cmd_code) )
                {
                    pTscm_cmd_box = tscm_malloc(sizeof(TSCM_CMD_BOX));
                    if( !pTscm_cmd_box )
                    {
                        tscm_msg_ex(TSCM_MSG_ERR, "err, Allocate fail !!");
                        break;
                    }

                    memset(pTscm_cmd_box, 0x0, sizeof(TSCM_CMD_BOX));

                    pTscm_cmd_box->box_arg.type                         = TSCM_AGR_TYPE_USER_HANDLE_RECV;
                    pTscm_cmd_box->box_arg.arg.user_handle.cmd_code     = cmd_code;
                    pTscm_cmd_box->box_arg.arg.user_handle.priority     = TSCM_CMD_PRIORITY_ER;
                    pTscm_cmd_box->box_arg.arg.user_handle.handler      = pf_user_hanlder;
                    pTscm_cmd_box->box_arg.arg.user_handle.pTunnel_info = pTunnel_info;
                    pTscm_cmd_box->box_arg.arg.user_handle.pCmd_info    = pCmd_pkt_parser->pCmd_pkt_privData;
                    pTscm_cmd_box->box_arg.arg.user_handle.rx_dev_addr_id = rcp_hdr.rx_dev_addr_id;
                    pTscm_cmd_box->box_arg.arg.user_handle.tx_dev_addr_id = rcp_hdr.tx_dev_addr_id;

                    // directly call user_handler, and user need to destroy cmd box
                    if( pf_user_hanlder )
                        pf_user_hanlder(pTscm_cmd_box, extraData);
                }
                else
                {
                    // send to pCmd_recv_queue
                    TSCM_CMD_BOX    *pPrev_cmd_box = 0;

                    if( pTscamDev->hTscam.recv_cmd_num > TSCAM_MAX_QUEUE_SIZE )
                    {
                        tscm_msg_ex(TSCM_MSG_ERR, "err, recv message queue full !!");

                        if( !pf_cmd_ctxt_destroy )
                        {
                            tscm_msg_ex(TSCM_MSG_ERR, "err, pf_cmd_ctxt_destroy[%d][%d] = Null !!", cmd_srvc_type, sub_cmd_code);
                            break;
                        }

                        // destroy internal cmd node
                        pf_cmd_ctxt_destroy(
                            true,
                            (void*)&pCmd_pkt_parser->pCmd_pkt_privData,
                            0, extraData);

                        free((*ppParser_info));
                        (*ppParser_info) = 0;
                        break;
                    }

                    //-------------------------------
                    // Every cmd MUST be unique in recv_queue.
                    pPrev_cmd_box = QUEUE_FIND(TSCM_CMD_BOX, _tscam_search_cmd_box,
                                               0, pTscamDev->hTscam.pCmd_recv_head, (uint32_t)cmd_code);
                    if( pPrev_cmd_box )
                    {
                        pTscamDev->hTscam.pCmd_recv_head = QUEUE_DEL(TSCM_CMD_BOX, pPrev_cmd_box);
                        pTscamDev->hTscam.recv_cmd_num--;
                        if( pTscamDev->hTscam.recv_cmd_num < 0 )
                            pTscamDev->hTscam.recv_cmd_num = 0;

                        pTscamDev->hTscam.pCmd_recv_queue = QUEUE_FIND_TAIL(TSCM_CMD_BOX, pTscamDev->hTscam.pCmd_recv_head);

                        // destroy internal cmd node
                        pf_cmd_ctxt_destroy(
                            true,
                            (void*)&pPrev_cmd_box->box_arg.arg.user_handle.pCmd_info,
                            0, extraData);

                        free(pPrev_cmd_box);
                        pPrev_cmd_box = 0;
                    }

                    pTscm_cmd_box = tscm_malloc(sizeof(TSCM_CMD_BOX));
                    if( !pTscm_cmd_box )
                    {
                        tscm_msg_ex(TSCM_MSG_ERR, "err, Allocate fail !!");
                        break;
                    }

                    memset(pTscm_cmd_box, 0x0, sizeof(TSCM_CMD_BOX));

                    QUEUE_INIT(pTscm_cmd_box);

                    pTscm_cmd_box->box_arg.type                         = TSCM_AGR_TYPE_USER_HANDLE_RECV;
                    pTscm_cmd_box->box_arg.arg.user_handle.cmd_code     = cmd_code;
                    pTscm_cmd_box->box_arg.arg.user_handle.priority     = TSCM_CMD_PRIORITY_APPT;
                    pTscm_cmd_box->box_arg.arg.user_handle.handler      = pf_user_hanlder;
                    pTscm_cmd_box->box_arg.arg.user_handle.pTunnel_info = pTunnel_info;
                    pTscm_cmd_box->box_arg.arg.user_handle.pCmd_info    = pCmd_pkt_parser->pCmd_pkt_privData;
                    pTscm_cmd_box->box_arg.arg.user_handle.rx_dev_addr_id = rcp_hdr.rx_dev_addr_id;
                    pTscm_cmd_box->box_arg.arg.user_handle.tx_dev_addr_id = rcp_hdr.tx_dev_addr_id;
                    if( !pTscamDev->hTscam.pCmd_recv_queue )
                    {
                        pTscamDev->hTscam.pCmd_recv_queue = pTscm_cmd_box;
                        pTscamDev->hTscam.pCmd_recv_head  = pTscm_cmd_box;
                        pTscamDev->hTscam.recv_cmd_num    = 1;
                    }
                    else
                    {
                        pTscamDev->hTscam.pCmd_recv_queue = QUEUE_ADD(TSCM_CMD_BOX, pTscamDev->hTscam.pCmd_recv_queue, pTscm_cmd_box);
                        pTscamDev->hTscam.recv_cmd_num++;
                    }
                }

                free((*ppParser_info));
                (*ppParser_info) = 0;
            }
        }
    }while(0);
    return result;
}


static uint32_t
_tscam_cmd_generate(
    TSCAM_DEV       *pTscamDev,
    TSCM_CMD_BOX    *pCmd_box_cur,
    uint8_t         **ppCmd_pkts,
    uint32_t        *cmd_pkts_size,
    void            *extraData)
{
    /**
     * |<- user  ->|<--        _tscam_cmd_generate()            -->|
     * -------------------------------------------------------------
     * | ts_header | leading_tag | return_channel_packet | end_tag |
     * |  (4Bytes) |  (1Byte)    |    (182Bytes)         | (1Byte) |
     * -------------------------------------------------------------
     *                           |<-- cmd pkt encoder -->|
     **/
    uint32_t            result = 0;
    CMD_PKT_CODEC       pf_cmd_pkt_encoder = 0;
    CMD_CTXT_CREATE     pf_cmd_ctxt_create = 0;
    CMD_CTXT_DESTROY    pf_cmd_ctxt_destroy = 0;
    void                **ppMuxer_info = 0;
    CMD_HANDLER         pf_user_hanlder = 0;

    do{
        CMD_RCP_HDR         rcp_hdr = {0};
        uint16_t            cmd_code = (-1);
        uint16_t            sub_cmd_code = (-1);
        uint32_t            rcp_hdr_length = 0;
        TSCM_CMD_SRVC_TYPE  cmd_srvc_type = TSCM_CMD_SRVC_MAX;

        // packet header
        rcp_hdr.rx_dev_addr_id = pCmd_box_cur->box_arg.arg.user_handle.rx_dev_addr_id;
        rcp_hdr.tx_dev_addr_id = pCmd_box_cur->box_arg.arg.user_handle.tx_dev_addr_id;
        rcp_hdr.total_pkt_num  = 0;
        rcp_hdr.pkt_num        = 0;
        rcp_hdr.seq_num        = 0;
        rcp_hdr.pkt_length     = 0;

        cmd_code     = pCmd_box_cur->box_arg.arg.user_handle.cmd_code;
        sub_cmd_code = (cmd_code & CMD_SERVICE_CMD_CODE_MASK);

        switch( cmd_code & CMD_SERVICE_TYPE_MASK )
        {
            case CCHDTV_INPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_CCHDTV_IN;        break;
            case CCHDTV_OUTPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_CCHDTV_OUT;       break;
            case DEV_MGT_INPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_DEV_MGT_IN;       break;
            case DEV_MGT_OUTPUT_SERVICE_TAG:    cmd_srvc_type = TSCM_CMD_SRVC_DEV_MGT_OUT;      break;
            case DEV_IO_INPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_DEV_IO_IN;        break;
            case DEV_IO_OUTPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_DEV_IO_OUT;       break;
            case IMG_SERVICE_INPUT_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_IMG_IN;           break;
            case IMG_SERVICE_OUTPUT_TAG:        cmd_srvc_type = TSCM_CMD_SRVC_IMG_OUT;          break;
            case MEDIA_INPUT_SERVICE_TAG:       cmd_srvc_type = TSCM_CMD_SRVC_MEDIA_IN;         break;
            case MEDIA_OUTPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_MEDIA_OUT;        break;
            case PTZ_INPUT_SERVICE_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_PTZ_IN;           break;
            case PTZ_OUTPUT_SERVICE_TAG:        cmd_srvc_type = TSCM_CMD_SRVC_PTZ_OUT;          break;
            case VIDEO_ANA_INPUT_SERVICE_TAG:   cmd_srvc_type = TSCM_CMD_SRVC_VIDEO_ANAL_IN;    break;
            case VIDEO_ANA_OUTPUT_SERVICE_TAG:  cmd_srvc_type = TSCM_CMD_SRVC_VIDEO_ANAL_OUT;   break;
            case META_STRM_SERVICE_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_META;             break;
        }

        if( cmd_srvc_type == TSCM_CMD_SRVC_MAX ||
            sub_cmd_code >= MAX_CMD_NUM_PER_SERVICE )
            break;

        if( !pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code] )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, pCmd_srvc_attr[%d][%d] = Null !!", cmd_srvc_type, sub_cmd_code);
            break;
        }

        pf_cmd_pkt_encoder  = pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->cmd_pkt_encode;
        pf_cmd_ctxt_create  = pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->cmd_ctxt_create;
        pf_cmd_ctxt_destroy = pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->cmd_ctxt_destroy;
        pf_user_hanlder     = pCmd_box_cur->box_arg.arg.user_handle.handler; // follow cmd_post setting
        ppMuxer_info        = &pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->pPriv_info[0];

        //----------------------------------------------
        // call cmd packet codec handler
        if( pf_cmd_pkt_encoder )
        {
            uint32_t        output_info_ptr = 0;
            CMD_PKT_MUXER   *pCmd_pkt_muxer= 0;

            if( !(*ppMuxer_info) )
            {
                pCmd_pkt_muxer = tscm_malloc(sizeof(CMD_PKT_MUXER));
                if( !pCmd_pkt_muxer )
                {
                    tscm_msg_ex(TSCM_MSG_ERR, "err, Allocate fail !!");
                    break;
                }
                memset(pCmd_pkt_muxer, 0x0, sizeof(CMD_PKT_MUXER));
                (*ppMuxer_info) = pCmd_pkt_muxer;
            }
            else
                pCmd_pkt_muxer = (CMD_PKT_MUXER*)(*ppMuxer_info);

            //----------------------------------------------
            // set parameters
            pCmd_pkt_muxer->pUser_cmd_pkts_opr = &pTscamDev->user_cmd_pkts_opr;
            pCmd_pkt_muxer->rcp_cmd_data_size  = TSCAM_RC_CMD_DATA_MAX_SIZE;
            switch( pCmd_box_cur->box_arg.arg.user_handle.stream_pkt_fmt )
            {
                case TSCM_STREAM_PKT_TS:
                    pCmd_pkt_muxer->pkt_size            = TSCAM_TS_PKT_SIZE;
                    pCmd_pkt_muxer->pkt_rcp_data_offset = TSCAM_TS_HDR_SIZE + TSCAM_LEADING_TAG_SIZE;
                    break;
                case TSCM_STREAM_PKT_RS232_422_485:
                    pCmd_pkt_muxer->pkt_size            = TSCAM_RS232_PKT_SIZE;
                    pCmd_pkt_muxer->pkt_rcp_data_offset = TSCAM_LEADING_TAG_SIZE;
                    break;
            }

            // cmd structure -> return channel format
            rcp_hdr_length = sizeof(CMD_RCP_HDR);
            while( pCmd_pkt_muxer->bSplitDone == false )
            {
                uint32_t    check_sum_size = 0;
                uint8_t     *pCur = 0, *pTmp = 0;

                pCmd_pkt_muxer->pCmd_pkt_privData = pCmd_box_cur->box_arg.arg.user_handle.pCmd_info;
                if( !pCmd_pkt_muxer->pCmd_pkt_privData )    break;

                result = pf_cmd_pkt_encoder((void*)pCmd_pkt_muxer, (void*)&output_info_ptr, extraData);
                if( result )       break;

                rcp_hdr.total_pkt_num  = pCmd_pkt_muxer->total_pkt_num;
                rcp_hdr.pkt_num        = pCmd_pkt_muxer->pkt_num++;
                rcp_hdr.seq_num        = pTscamDev->seq_num++;
                rcp_hdr.pkt_length     = pCmd_pkt_muxer->pkt_length;

                pCur = pCmd_pkt_muxer->pCmd_pkts_buf;
                pCur += (pCmd_pkt_muxer->pkt_size * rcp_hdr.pkt_num);

                // end tag
                *(pCur + pCmd_pkt_muxer->pkt_size - 1) = 0x0d;

                //----------------------------------------------
                // user handle (ts header if need), follow cmd_post setting
                if( pf_user_hanlder )
                {
                    TSCM_ARG    *pArg = &pCmd_box_cur->box_arg;

                    pArg->type                            = TSCM_AGR_TYPE_USER_HANDLE_TRANS;
                    pArg->arg.user_handle.pPkt_start_addr = pCur;
                    pArg->arg.user_handle.pkt_size        = pCmd_pkt_muxer->pkt_size;

                    pf_user_hanlder(pCmd_box_cur, extraData);
                }

                //----------------------------------------------
                // leading tag
                pCur += (pCmd_pkt_muxer->pkt_rcp_data_offset - TSCAM_LEADING_TAG_SIZE);
                (*pCur) = '#';
                pCur += TSCAM_LEADING_TAG_SIZE;

                //----------------------------------------------
                // return channel packet header
                pTmp = pCur;
                _SET_WORD(pTmp, rcp_hdr.rx_dev_addr_id);
                _SET_WORD(pTmp, rcp_hdr.tx_dev_addr_id);
                _SET_WORD(pTmp, rcp_hdr.total_pkt_num);
                rcp_hdr.pkt_num++; // start from index "1" (defined by spec)
                _SET_WORD(pTmp, rcp_hdr.pkt_num);
                rcp_hdr.seq_num++; // start from index "1" (defined by spec)
                _SET_WORD(pTmp, rcp_hdr.seq_num);
                _SET_WORD(pTmp, rcp_hdr.pkt_length);

                //----------------------------------------------
                // check sum in return channel packet
                // - check sun size = rcp_hdr.pkt_length + 12
                pCur += 2; // In Spec, the rx_dev_addr_id doesn't be included in check_sum.
                check_sum_size = rcp_hdr_length - 2 + rcp_hdr.pkt_length;
                *(pCur + check_sum_size) = _tscm_gen_check_sum(pCur, check_sum_size);
            }

            (*ppCmd_pkts)    = pCmd_pkt_muxer->pCmd_pkts_buf;
            (*cmd_pkts_size) = pCmd_pkt_muxer->cmd_pkts_buf_size;

            // destroy internal cmd node
            pf_cmd_ctxt_destroy(
                false,
                (void*)&pCmd_box_cur->box_arg.arg.user_handle.pCmd_info,
                0, extraData);

            free(pCmd_pkt_muxer);
            (*ppMuxer_info) = 0;
        }
    }while(0);

    return result;
}


static uint32_t
_tscam_set_cmd_pkt_codec(
    CMD_PKT_INIT_INFO   *pInit_info,
    CMD_PKT_ARG         *pArg,
    void                *extraData)
{
    uint32_t            result = 0;
    TSCAM_DEV           *pTscamDev= 0;
    TSCM_CMD_SRVC_TYPE  cmd_srvc_type = TSCM_CMD_SRVC_MAX;

    do{
        uint32_t            cmd_code = -1;
        uint16_t            sub_cmd_code = (-1);
        CMD_PKT_CODEC       cmd_pkt_encode = 0;
        CMD_PKT_CODEC       cmd_pkt_decode = 0;
        CMD_CTXT_CREATE     cmd_ctxt_create = 0;
        CMD_CTXT_DESTROY    cmd_ctxt_destroy = 0;

        if( !pInit_info || !pArg )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, Null pointer !!");
            break;
        }

        pTscamDev        = (TSCAM_DEV*)pInit_info->pTunnelInfo;
        cmd_code         = pArg->arg.set_codec.cmd_code;
        sub_cmd_code     = (cmd_code & CMD_SERVICE_CMD_CODE_MASK);
        cmd_pkt_encode   = pArg->arg.set_codec.cmd_pkt_encode;
        cmd_pkt_decode   = pArg->arg.set_codec.cmd_pkt_decode;
        cmd_ctxt_create  = pArg->arg.set_codec.cmd_ctxt_create;
        cmd_ctxt_destroy = pArg->arg.set_codec.cmd_ctxt_destroy;

        switch( cmd_code & CMD_SERVICE_TYPE_MASK )
        {
            case CCHDTV_INPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_CCHDTV_IN;        break;
            case CCHDTV_OUTPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_CCHDTV_OUT;       break;
            case DEV_MGT_INPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_DEV_MGT_IN;       break;
            case DEV_MGT_OUTPUT_SERVICE_TAG:    cmd_srvc_type = TSCM_CMD_SRVC_DEV_MGT_OUT;      break;
            case DEV_IO_INPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_DEV_IO_IN;        break;
            case DEV_IO_OUTPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_DEV_IO_OUT;       break;
            case IMG_SERVICE_INPUT_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_IMG_IN;           break;
            case IMG_SERVICE_OUTPUT_TAG:        cmd_srvc_type = TSCM_CMD_SRVC_IMG_OUT;          break;
            case MEDIA_INPUT_SERVICE_TAG:       cmd_srvc_type = TSCM_CMD_SRVC_MEDIA_IN;         break;
            case MEDIA_OUTPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_MEDIA_OUT;        break;
            case PTZ_INPUT_SERVICE_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_PTZ_IN;           break;
            case PTZ_OUTPUT_SERVICE_TAG:        cmd_srvc_type = TSCM_CMD_SRVC_PTZ_OUT;          break;
            case VIDEO_ANA_INPUT_SERVICE_TAG:   cmd_srvc_type = TSCM_CMD_SRVC_VIDEO_ANAL_IN;    break;
            case VIDEO_ANA_OUTPUT_SERVICE_TAG:  cmd_srvc_type = TSCM_CMD_SRVC_VIDEO_ANAL_OUT;   break;
            case META_STRM_SERVICE_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_META;             break;
        }

        if( cmd_srvc_type == TSCM_CMD_SRVC_MAX ||
            sub_cmd_code >= MAX_CMD_NUM_PER_SERVICE )
            break;

        if( !pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code] )
            pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code] = tscm_malloc(sizeof(TSCM_CMD_SRVC_ATTR));

        if( pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code] )
        {
            memset(pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code], 0x0, sizeof(TSCM_CMD_SRVC_ATTR));
            pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->cmd_pkt_encode   = cmd_pkt_encode;
            pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->cmd_pkt_decode   = cmd_pkt_decode;
            pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->cmd_ctxt_create  = cmd_ctxt_create;
            pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->cmd_ctxt_destroy = cmd_ctxt_destroy;
        }
        else
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");;

    }while(0);

    return result;
}
//=============================================================================
//                Public Function Definition
//=============================================================================
TSCM_ERR
tscam_CreateHandle(
    TSCM_HANDLE     **ppHTscam,
    TSCM_SETUP_INFO *pSetup_info,
    void            *extraData)
{
    TSCM_ERR        result = TSCM_ERR_OK;
    TSCAM_DEV       *pTscamDev= 0;

    do{
        CMD_PKT_INIT_INFO   pkt_init_info = {0};
        TSCM_DESC           *pAct_desc = 0;

        if( ppHTscam && *ppHTscam != 0 )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, Exist tscam handle !!");
            result = TSCM_ERR_INVALID_PARAMETER;
            break;
        }
        if( !pSetup_info )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, invalid parameter !!");
            result = TSCM_ERR_INVALID_PARAMETER;
            break;
        }

        pTscamDev = tscm_malloc(sizeof(TSCAM_DEV));
        if( !pTscamDev )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, Allocate faile !!");
            result = TSCM_ERR_ALLOCATE_FAIL;
            break;
        }

        _tscam_Register_all();
        //-------------------------------
        // init parameters
        memset(pTscamDev, 0x0, sizeof(TSCAM_DEV));
        pTscamDev->act_recv_cmd_code    = (-1);
        pTscamDev->stream_recv_pkt_fmt  = pSetup_info->stream_recv_pkt_fmt;
        pTscamDev->stream_trans_pkt_fmt = pSetup_info->stream_trans_pkt_fmt;
        pTscamDev->user_cmd_pkts_opr    = pSetup_info->user_cmd_pkts_opr;

        TSCAM_BIT_ZERO(&pTscamDev->cmd_active_flag);

        // set packet enc/dec
        pkt_init_info.set_codec   = _tscam_set_cmd_pkt_codec;
        pkt_init_info.pTunnelInfo = (void*)pTscamDev;
        cmd_pkt_Init(&pkt_init_info, extraData);

        pTscamDev->pIncompletePktCache = tscm_malloc(TSCAM_TS_PKT_SIZE);
        if( !pTscamDev->pIncompletePktCache )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, Allocate faile !!");
            result = TSCM_ERR_ALLOCATE_FAIL;
            break;
        }
        memset(pTscamDev->pIncompletePktCache, 0x0, TSCAM_TS_PKT_SIZE);

        //-------------------------
        // find action descriptor
        pAct_desc = FIND_DESC_ITEM(TSCM_DESC, pSetup_info->tscm_type);
        if( !pAct_desc )
            tscm_msg_ex(TSCM_MSG_ERR, " err, can't find ts camera description (id=%d)!!", pSetup_info->tscm_type);
        else
        {
            pTscamDev->hTscam.cur_desc = *pAct_desc;
            tscm_msg(1, " action ts camera descriptor = %s !!\n", (pTscamDev->hTscam.cur_desc.name) ? pTscamDev->hTscam.cur_desc.name : "unknow");
        }

        if( pSetup_info->reset_desc )
            pSetup_info->reset_desc(&pTscamDev->hTscam.cur_desc, extraData);

        //----------------------
        // create mutex
        _mutex_init(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
        (*ppHTscam) = &pTscamDev->hTscam;

    }while(0);

    if( result != TSCM_ERR_OK )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }

    return result;
}


TSCM_ERR
tscam_DestroyHandle(
    TSCM_HANDLE     **ppHTscam,
    void            *extraData)
{
    TSCM_ERR            result = TSCM_ERR_OK;
    TSCAM_DEV           *pTscamDev = 0;
    pthread_mutex_t     tscm_mutex = 0;

    /**
     * Ap layer need to check all threads, which assess this handle, in STOP state.
     * Or system maybe crash.
     **/

    // _verify_handle(ppHTscam);

    if( ppHTscam && *ppHTscam )
    {
        do{
            uint32_t        i = 0, j = 0;

            pTscamDev = DOWN_CAST(TSCAM_DEV, (*ppHTscam), hTscam);

            _mutex_lock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
            _disable_irq();

            if( !pTscamDev )    break;

            if( pTscamDev->pIncompletePktCache )
            {
                free(pTscamDev->pIncompletePktCache);
                pTscamDev->pIncompletePktCache = 0;
            }

            for(i = TSCM_CMD_SRVC_CCHDTV_IN; i < TSCM_CMD_SRVC_MAX; i++)
            {
                for(j = 0; j < MAX_CMD_NUM_PER_SERVICE; j++)
                {
                    if( pTscamDev->pCmd_srvc_attr[i][j] )
                    {
                        free(pTscamDev->pCmd_srvc_attr[i][j]);
                        pTscamDev->pCmd_srvc_attr[i][j] = 0;
                    }
                }
            }
            //-------------------------------
            *ppHTscam = 0; // notice AP that handle has be destroyed

            tscm_mutex = pTscamDev->tscm_mutex;

            // destroy dev info
            free(pTscamDev);
        }while(0);

        _mutex_unlock(TSCM_MSG_TRACE_TSCM, tscm_mutex);
        _mutex_deinit(TSCM_MSG_TRACE_TSCM, tscm_mutex);

        _enable_irq();
    }

    return result;
}


TSCM_ERR
tscam_Init(
    TSCM_HANDLE         *pHTscam,
    TSCM_INIT_INFO      *pInit_info,
    void                *extraData)
{
    TSCM_ERR        result = TSCM_ERR_OK;
    TSCAM_DEV       *pTscamDev = 0;

    _verify_handle(pHTscam, result);

    pTscamDev = DOWN_CAST(TSCAM_DEV, pHTscam, hTscam);

    _mutex_lock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);

    if( pTscamDev )
    {
        if( pTscamDev->hTscam.cur_desc.init )
            result = pTscamDev->hTscam.cur_desc.init(pHTscam, pInit_info, extraData);
    }

    if( result != TSCM_ERR_OK )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
    return result;
}


TSCM_ERR
tscam_Deinit(
    TSCM_HANDLE         *pHTscam,
    void                *extraData)
{
    TSCM_ERR        result = TSCM_ERR_OK;
    TSCAM_DEV       *pTscamDev = 0;

    _verify_handle(pHTscam, result);

    pTscamDev = DOWN_CAST(TSCAM_DEV, pHTscam, hTscam);

    _mutex_lock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);

    if( pTscamDev )
    {
        if( pTscamDev->hTscam.cur_desc.deinit )
            result = pTscamDev->hTscam.cur_desc.deinit(pHTscam, extraData);
    }

    if( result != TSCM_ERR_OK )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
    return result;
}


TSCM_ERR
tscam_Stream_Recv(
    TSCM_HANDLE     *pHTscam,
    TSCM_RECV_INFO  *pRecv_info,
    void            *extraData)
{
    TSCM_ERR        result = TSCM_ERR_OK;
    TSCAM_DEV       *pTscamDev = 0;

    _verify_handle(pHTscam, result);

    pTscamDev = DOWN_CAST(TSCAM_DEV, pHTscam, hTscam);

    _mutex_lock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);

    if( pTscamDev )
    {
        if( pTscamDev->hTscam.cur_desc.stream_recv )
            result = pTscamDev->hTscam.cur_desc.stream_recv(pHTscam, pRecv_info, extraData);
    }

    if( result != TSCM_ERR_OK )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
    return result;
}


TSCM_ERR
tscam_Stream_Trans(
    TSCM_HANDLE     *pHTscam,
    TSCM_TRANS_INFO *pTrans_info,
    void            *extraData)
{
    TSCM_ERR        result = TSCM_ERR_OK;
    TSCAM_DEV       *pTscamDev = 0;

    _verify_handle(pHTscam, result);

    pTscamDev = DOWN_CAST(TSCAM_DEV, pHTscam, hTscam);

    _mutex_lock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);

    if( pTscamDev )
    {
        if( pTscamDev->hTscam.cur_desc.stream_trans )
            result = pTscamDev->hTscam.cur_desc.stream_trans(pHTscam, pTrans_info, extraData);
    }

    if( result != TSCM_ERR_OK )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
    return result;
}


TSCM_ERR
tscam_Cotrol(
    TSCM_HANDLE     *pHTscam,
    TSCM_ARG        *pArg,
    void            *extraData)
{
    TSCM_ERR        result = TSCM_ERR_OK;
    TSCAM_DEV       *pTscamDev = 0;

    _verify_handle(pHTscam, result);

    pTscamDev = DOWN_CAST(TSCAM_DEV, pHTscam, hTscam);

    _mutex_lock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);

    if( pTscamDev )
    {
        if( pTscamDev->hTscam.cur_desc.control )
            result = pTscamDev->hTscam.cur_desc.control(pHTscam, pArg, extraData);
    }

    if( result != TSCM_ERR_OK )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
    return result;
}


TSCM_ERR
tscam_Stream_Analyse(
    TSCM_HANDLE     *pHTscam,
    TSCM_ANAL_INFO  *pAnal_info,
    void            *extraData)
{
    TSCM_ERR        result = TSCM_ERR_OK;
    TSCAM_DEV       *pTscamDev = 0;

    _verify_handle(pHTscam, result);

    pTscamDev = DOWN_CAST(TSCAM_DEV, pHTscam, hTscam);

    _mutex_lock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);

    if( pTscamDev && pAnal_info )
    {
        uint8_t     *pData = pAnal_info->pSteam_buf;
        uint32_t    remainSize = pAnal_info->stream_buf_size;
        uint32_t    act_pkt_size = 0;
        uint32_t    sync_byte = 0;
        uint32_t    offset = 0;

        switch( pTscamDev->stream_recv_pkt_fmt )
        {
            case TSCM_STREAM_PKT_TS:
                act_pkt_size = TSCAM_TS_PKT_SIZE;
                sync_byte    = TSCAM_VALID_SYNC_BYTE;
                offset       = TSCAM_TS_HDR_SIZE;
                break;

            case TSCM_STREAM_PKT_RS232_422_485:
                act_pkt_size = TSCAM_RS232_PKT_SIZE;
                sync_byte    = '#';
                offset       = 0;
                break;
        }

        while( remainSize > 0 )
        {
            switch( pTscamDev->stream_state )
            {
                case TSCAM_STREAM_STATE_SEARCH_PKT_START:
                    if( (*pData) == sync_byte )
                    {
                        if( remainSize < act_pkt_size )
                            pTscamDev->stream_state = TSCAM_STREAM_STATE_LESS_PKT_SIZE;
                        else
                        {
                            _tscam_cmd_analyse(pTscamDev, (pData + offset), act_pkt_size - offset, extraData);

                            pData       += act_pkt_size;
                            remainSize  -= act_pkt_size;
                            break;
                        }
                    }
                    else
                    {
                        ++pData;
                        --remainSize;
                    }
                    break;

                case TSCAM_STREAM_STATE_LESS_PKT_SIZE:
                    if( pTscamDev->collectedByte > 0 &&
                        remainSize >= (int)(act_pkt_size - pTscamDev->collectedByte) )
                    {
                        memcpy(&pTscamDev->pIncompletePktCache[pTscamDev->collectedByte],
                               pData, (act_pkt_size - pTscamDev->collectedByte));

                        _tscam_cmd_analyse(pTscamDev, (pTscamDev->pIncompletePktCache + offset), act_pkt_size - offset, extraData);

                        pData       += (act_pkt_size - pTscamDev->collectedByte);
                        remainSize  -= (act_pkt_size - pTscamDev->collectedByte);

                        pTscamDev->collectedByte = 0;
                        pTscamDev->stream_state  = TSCAM_STREAM_STATE_SEARCH_PKT_START;
                        break;
                    }
                    else
                    {
                        memcpy(&pTscamDev->pIncompletePktCache[pTscamDev->collectedByte],
                               pData, remainSize);

                        pTscamDev->collectedByte += remainSize;
                        remainSize = 0;
                    }
                    break;
            }
        }
    }

    if( result != TSCM_ERR_OK )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
    return result;
}


TSCM_ERR
tscam_Stream_Destroy(
    TSCM_HANDLE     *pHTscam,
    TSCM_GEN_INFO   *pGenInfo,
    void            *extraData)
{
    TSCM_ERR        result = TSCM_ERR_OK;
    TSCAM_DEV       *pTscamDev = 0;

    _verify_handle(pHTscam, result);

    pTscamDev = DOWN_CAST(TSCAM_DEV, pHTscam, hTscam);

    /**
     * Input pGenInfo MUST be from tscam_Stream_Generate(), or it will crash.
     * Because the pGenInfo from tscam_Stream_Generate() is allocated independently,
     * it don't need mutex.
     **/

    //_mutex_lock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);

    if( pTscamDev && pGenInfo )
    {
        if( pGenInfo->pSteam_buf )
        {
            free(pGenInfo->pSteam_buf);
            pGenInfo->pSteam_buf      = 0;
            pGenInfo->stream_buf_size = 0;
        }
    }

    if( result != TSCM_ERR_OK )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }

    //_mutex_unlock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
    return result;
}


TSCM_ERR
tscam_Stream_Generate(
    TSCM_HANDLE     *pHTscam,
    TSCM_GEN_INFO   *pGenInfo,
    void            *extraData)
{
    TSCM_ERR            result = TSCM_ERR_OK;
    TSCAM_DEV           *pTscamDev = 0;

    _verify_handle(pHTscam, result);

    pTscamDev = DOWN_CAST(TSCAM_DEV, pHTscam, hTscam);

    _mutex_lock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);

    if( pTscamDev && pGenInfo )
    {
        do{
            TSCM_CMD_BOX    *pCmd_trans_cur = 0;
            TSCM_CMD_BOX    *pCmd_trans_tmp = 0;
            uint32_t        cmd_pkts_size = 0;
            uint8_t         *pCmd_pkts = 0;

            //---------------------------------------------
            // pull transmitted cmd from trans_queue
            if( !pTscamDev->hTscam.pCmd_trans_head ||
                !pTscamDev->hTscam.pCmd_trans_queue )
                break;

            pGenInfo->pSteam_buf      = 0;
            pGenInfo->stream_buf_size = 0;

            pCmd_trans_cur = pTscamDev->hTscam.pCmd_trans_head;

            pCmd_trans_tmp = QUEUE_DEL(TSCM_CMD_BOX, pCmd_trans_cur);
            pTscamDev->hTscam.trans_cmd_num--;
            if( pTscamDev->hTscam.trans_cmd_num < 0 )
                pTscamDev->hTscam.trans_cmd_num = 0;

            if( !pCmd_trans_tmp )
                pTscamDev->hTscam.pCmd_trans_queue = 0;

            pTscamDev->hTscam.pCmd_trans_head = QUEUE_FIND_HEAD(TSCM_CMD_BOX, pTscamDev->hTscam.pCmd_trans_queue);

            //---------------------------------------------
            // cmd structure -> return channel format
            _tscam_cmd_generate(pTscamDev, pCmd_trans_cur, &pCmd_pkts, &cmd_pkts_size, extraData);

            pGenInfo->pSteam_buf      = pCmd_pkts;
            pGenInfo->stream_buf_size = cmd_pkts_size;

            // destroy cmd_box
            free(pCmd_trans_cur);
        }while(0);
    }

    if( result != TSCM_ERR_OK )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
    return result;

}


TSCM_ERR
tscam_Attach_Cmd_Handler(
    TSCM_HANDLE             *pHTscam,
    TSCM_CMD_HANDLE_INFO    *pCmd_handle_info,
    void                    *extraData)
{
    TSCM_ERR            result = TSCM_ERR_OK;
    TSCAM_DEV           *pTscamDev = 0;
    TSCM_CMD_SRVC_TYPE  cmd_srvc_type = TSCM_CMD_SRVC_MAX;

    _verify_handle(pHTscam, result);

    pTscamDev = DOWN_CAST(TSCAM_DEV, pHTscam, hTscam);

    _mutex_lock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);

    if( pTscamDev && pCmd_handle_info )
    {
        do{
            uint32_t    cmd_code = pCmd_handle_info->cmd_code;
            uint16_t    sub_cmd_code = (-1);

            sub_cmd_code = (cmd_code & CMD_SERVICE_CMD_CODE_MASK);

            switch( cmd_code & CMD_SERVICE_TYPE_MASK )
            {
                case CCHDTV_INPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_CCHDTV_IN;        break;
                case CCHDTV_OUTPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_CCHDTV_OUT;       break;
                case DEV_MGT_INPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_DEV_MGT_IN;       break;
                case DEV_MGT_OUTPUT_SERVICE_TAG:    cmd_srvc_type = TSCM_CMD_SRVC_DEV_MGT_OUT;      break;
                case DEV_IO_INPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_DEV_IO_IN;        break;
                case DEV_IO_OUTPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_DEV_IO_OUT;       break;
                case IMG_SERVICE_INPUT_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_IMG_IN;           break;
                case IMG_SERVICE_OUTPUT_TAG:        cmd_srvc_type = TSCM_CMD_SRVC_IMG_OUT;          break;
                case MEDIA_INPUT_SERVICE_TAG:       cmd_srvc_type = TSCM_CMD_SRVC_MEDIA_IN;         break;
                case MEDIA_OUTPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_MEDIA_OUT;        break;
                case PTZ_INPUT_SERVICE_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_PTZ_IN;           break;
                case PTZ_OUTPUT_SERVICE_TAG:        cmd_srvc_type = TSCM_CMD_SRVC_PTZ_OUT;          break;
                case VIDEO_ANA_INPUT_SERVICE_TAG:   cmd_srvc_type = TSCM_CMD_SRVC_VIDEO_ANAL_IN;    break;
                case VIDEO_ANA_OUTPUT_SERVICE_TAG:  cmd_srvc_type = TSCM_CMD_SRVC_VIDEO_ANAL_OUT;   break;
                case META_STRM_SERVICE_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_META;             break;
            }

            if( cmd_srvc_type == TSCM_CMD_SRVC_MAX ||
                sub_cmd_code >= MAX_CMD_NUM_PER_SERVICE )
                break;

            if( !pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code] )
            {
                tscm_msg_ex(TSCM_MSG_ERR, "err, pCmd_srvc_attr[%d][%d] = Null !!", cmd_srvc_type, sub_cmd_code);
                break;
            }

            pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->user_hanlder = pCmd_handle_info->user_hanlder;
            pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->pTunnel_info = pCmd_handle_info->pTunnel_info;

            switch( pCmd_handle_info->priority )
            {
                case TSCM_CMD_PRIORITY_ER:
                    TSCAM_BIT_SET(&pTscamDev->cmd_active_flag[cmd_srvc_type], sub_cmd_code);
                    break;

                default:
                case TSCM_CMD_PRIORITY_APPT:
                    TSCAM_BIT_CLR(&pTscamDev->cmd_active_flag[cmd_srvc_type], sub_cmd_code);
                    break;
            }

        }while(0);
    }

    if( result != TSCM_ERR_OK )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
    return result;
}


TSCM_ERR
tscam_Detach_Cmd_Handler(
    TSCM_HANDLE             *pHTscam,
    TSCM_CMD_HANDLE_INFO    *pCmd_handle_info,
    void                    *extraData)
{
    TSCM_ERR            result = TSCM_ERR_OK;
    TSCAM_DEV           *pTscamDev = 0;
    TSCM_CMD_SRVC_TYPE  cmd_srvc_type = TSCM_CMD_SRVC_MAX;

    _verify_handle(pHTscam, result);

    pTscamDev = DOWN_CAST(TSCAM_DEV, pHTscam, hTscam);

    _mutex_lock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);

    if( pTscamDev && pCmd_handle_info )
    {
        do{
            uint32_t    cmd_code = pCmd_handle_info->cmd_code;
            uint16_t    sub_cmd_code = (-1);

            sub_cmd_code = (cmd_code & CMD_SERVICE_CMD_CODE_MASK);

            switch( cmd_code & CMD_SERVICE_TYPE_MASK )
            {
                case CCHDTV_INPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_CCHDTV_IN;        break;
                case CCHDTV_OUTPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_CCHDTV_OUT;       break;
                case DEV_MGT_INPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_DEV_MGT_IN;       break;
                case DEV_MGT_OUTPUT_SERVICE_TAG:    cmd_srvc_type = TSCM_CMD_SRVC_DEV_MGT_OUT;      break;
                case DEV_IO_INPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_DEV_IO_IN;        break;
                case DEV_IO_OUTPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_DEV_IO_OUT;       break;
                case IMG_SERVICE_INPUT_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_IMG_IN;           break;
                case IMG_SERVICE_OUTPUT_TAG:        cmd_srvc_type = TSCM_CMD_SRVC_IMG_OUT;          break;
                case MEDIA_INPUT_SERVICE_TAG:       cmd_srvc_type = TSCM_CMD_SRVC_MEDIA_IN;         break;
                case MEDIA_OUTPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_MEDIA_OUT;        break;
                case PTZ_INPUT_SERVICE_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_PTZ_IN;           break;
                case PTZ_OUTPUT_SERVICE_TAG:        cmd_srvc_type = TSCM_CMD_SRVC_PTZ_OUT;          break;
                case VIDEO_ANA_INPUT_SERVICE_TAG:   cmd_srvc_type = TSCM_CMD_SRVC_VIDEO_ANAL_IN;    break;
                case VIDEO_ANA_OUTPUT_SERVICE_TAG:  cmd_srvc_type = TSCM_CMD_SRVC_VIDEO_ANAL_OUT;   break;
                case META_STRM_SERVICE_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_META;             break;
            }

            if( cmd_srvc_type == TSCM_CMD_SRVC_MAX ||
                sub_cmd_code >= MAX_CMD_NUM_PER_SERVICE )
                break;

            if( !pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code] )
            {
                tscm_msg_ex(TSCM_MSG_ERR, "err, pCmd_srvc_attr[%d][%d] = Null !!", cmd_srvc_type, sub_cmd_code);
                break;
            }

            pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->user_hanlder = 0;
            pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->pTunnel_info = 0;

            TSCAM_BIT_CLR(&pTscamDev->cmd_active_flag[cmd_srvc_type], sub_cmd_code);
        }while(0);
    }

    if( result != TSCM_ERR_OK )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
    return result;
}


TSCM_ERR
tscam_Cmd_Destroy(
    TSCM_HANDLE     *pHTscam,
    TSCM_CMD_BOX    **ppCmd_box,
    void            *extraData)
{
    TSCM_ERR            result = TSCM_ERR_OK;
    TSCM_CMD_SRVC_TYPE  cmd_srvc_type = TSCM_CMD_SRVC_MAX;
    TSCAM_DEV           *pTscamDev = 0;

    _verify_handle(pHTscam, result);
    _verify_handle(ppCmd_box, result);

    pTscamDev = DOWN_CAST(TSCAM_DEV, pHTscam, hTscam);

    /**
     * Input cmd_box MUST be from tscam_Cmd_Fetch(), or it will crash.
     * Because the cmd_box from tscam_Cmd_Fetch() is independent with tscm module,
     * it don't need mutex.
     **/
    // _mutex_lock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);

    if( pTscamDev && *ppCmd_box )
    {
        do{
            CMD_CTXT_DESTROY    pf_cmd_ctxt_destroy = 0;
            uint32_t            cmd_code = (*ppCmd_box)->box_arg.arg.user_handle.cmd_code;
            uint16_t            sub_cmd_code = (-1);

            sub_cmd_code = (cmd_code & CMD_SERVICE_CMD_CODE_MASK);

            switch( cmd_code & CMD_SERVICE_TYPE_MASK )
            {
                case CCHDTV_INPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_CCHDTV_IN;        break;
                case CCHDTV_OUTPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_CCHDTV_OUT;       break;
                case DEV_MGT_INPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_DEV_MGT_IN;       break;
                case DEV_MGT_OUTPUT_SERVICE_TAG:    cmd_srvc_type = TSCM_CMD_SRVC_DEV_MGT_OUT;      break;
                case DEV_IO_INPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_DEV_IO_IN;        break;
                case DEV_IO_OUTPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_DEV_IO_OUT;       break;
                case IMG_SERVICE_INPUT_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_IMG_IN;           break;
                case IMG_SERVICE_OUTPUT_TAG:        cmd_srvc_type = TSCM_CMD_SRVC_IMG_OUT;          break;
                case MEDIA_INPUT_SERVICE_TAG:       cmd_srvc_type = TSCM_CMD_SRVC_MEDIA_IN;         break;
                case MEDIA_OUTPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_MEDIA_OUT;        break;
                case PTZ_INPUT_SERVICE_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_PTZ_IN;           break;
                case PTZ_OUTPUT_SERVICE_TAG:        cmd_srvc_type = TSCM_CMD_SRVC_PTZ_OUT;          break;
                case VIDEO_ANA_INPUT_SERVICE_TAG:   cmd_srvc_type = TSCM_CMD_SRVC_VIDEO_ANAL_IN;    break;
                case VIDEO_ANA_OUTPUT_SERVICE_TAG:  cmd_srvc_type = TSCM_CMD_SRVC_VIDEO_ANAL_OUT;   break;
                case META_STRM_SERVICE_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_META;             break;
            }

            if( cmd_srvc_type == TSCM_CMD_SRVC_MAX ||
                sub_cmd_code >= MAX_CMD_NUM_PER_SERVICE )
                break;

            if( !pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code] )
            {
                tscm_msg_ex(TSCM_MSG_ERR, "err, pCmd_srvc_attr[%d][%d] = Null !!", cmd_srvc_type, sub_cmd_code);
                break;
            }

            pf_cmd_ctxt_destroy = pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->cmd_ctxt_destroy;
            if( !pf_cmd_ctxt_destroy )
            {
                tscm_msg_ex(TSCM_MSG_ERR, "err, pf_cmd_ctxt_destroy[%d][%d] = Null !!", cmd_srvc_type, sub_cmd_code);
                break;
            }

            // destroy internal cmd node
            pf_cmd_ctxt_destroy(
                true,
                (void*)&(*ppCmd_box)->box_arg.arg.user_handle.pCmd_info,
                0, extraData);

            if( *ppCmd_box )
            {
                free(*ppCmd_box);
                *ppCmd_box = 0;
            }
        }while(0);
    }

    if( result != TSCM_ERR_OK )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }

    // _mutex_unlock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
    return result;
}


TSCM_ERR
tscam_Cmd_Fetch(
    TSCM_HANDLE     *pHTscam,
    TSCM_CMD_BOX    **ppCmd_box,
    void            *extraData)
{
    TSCM_ERR        result = TSCM_ERR_OK;
    TSCAM_DEV       *pTscamDev = 0;

    _verify_handle(pHTscam, result);

    pTscamDev = DOWN_CAST(TSCAM_DEV, pHTscam, hTscam);

    _mutex_lock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);

    if( pTscamDev && ppCmd_box )
    {
        do{
            TSCM_CMD_BOX    *pCmd_recv_cur = 0;
            TSCM_CMD_BOX    *pCmd_recv_tmp = 0;

            // get cmd service from pCmd_recv_queue
            if( !pTscamDev->hTscam.pCmd_recv_head ||
                !pTscamDev->hTscam.pCmd_recv_queue )
                break;

            pCmd_recv_cur = pTscamDev->hTscam.pCmd_recv_head;

            pCmd_recv_tmp = QUEUE_DEL(TSCM_CMD_BOX, pCmd_recv_cur);
            pTscamDev->hTscam.recv_cmd_num--;
            if( pTscamDev->hTscam.recv_cmd_num < 0 )
                pTscamDev->hTscam.recv_cmd_num = 0;

            if( !pCmd_recv_tmp )
                pTscamDev->hTscam.pCmd_recv_queue = 0;

            pTscamDev->hTscam.pCmd_recv_head = QUEUE_FIND_HEAD(TSCM_CMD_BOX, pTscamDev->hTscam.pCmd_recv_queue);

            *ppCmd_box = pCmd_recv_cur;
        }while(0);
    }

    if( result != TSCM_ERR_OK )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
    return result;
}


TSCM_ERR
tscam_Cmd_Post(
    TSCM_HANDLE     *pHTscam,
    TSCM_CMD_BOX    *pCmd_box,
    void            *extraData)
{
    TSCM_ERR            result = TSCM_ERR_OK;
    TSCM_CMD_SRVC_TYPE  cmd_srvc_type = TSCM_CMD_SRVC_MAX;
    TSCAM_DEV           *pTscamDev = 0;

    _verify_handle(pHTscam, result);

    pTscamDev = DOWN_CAST(TSCAM_DEV, pHTscam, hTscam);

    _mutex_lock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);

    if( pTscamDev && pCmd_box )
    {
        do{
            TSCAM_ACTIVE_FLAG   *pCmd_active_flag = 0;
            TSCM_CMD_BOX        *pCmd_trans_tmp = 0;
            CMD_HANDLER         pf_user_hanlder = 0;
            CMD_CTXT_CREATE     pf_cmd_ctxt_create = 0;
            uint32_t            cmd_code = pCmd_box->box_arg.arg.user_handle.cmd_code;
            uint16_t            sub_cmd_code = (-1);

            if( pCmd_box->box_arg.arg.user_handle.stream_pkt_fmt != TSCM_STREAM_PKT_TS &&
                pCmd_box->box_arg.arg.user_handle.stream_pkt_fmt != TSCM_STREAM_PKT_RS232_422_485 )
            {
                tscm_msg_ex(TSCM_MSG_ERR, "err, Wrong stream_pkt_fmt (0x%x)!",
                    pCmd_box->box_arg.arg.user_handle.stream_pkt_fmt);
                break;
            }

            sub_cmd_code = (cmd_code & CMD_SERVICE_CMD_CODE_MASK);

            switch( cmd_code & CMD_SERVICE_TYPE_MASK )
            {
                case CCHDTV_INPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_CCHDTV_IN;        break;
                case CCHDTV_OUTPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_CCHDTV_OUT;       break;
                case DEV_MGT_INPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_DEV_MGT_IN;       break;
                case DEV_MGT_OUTPUT_SERVICE_TAG:    cmd_srvc_type = TSCM_CMD_SRVC_DEV_MGT_OUT;      break;
                case DEV_IO_INPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_DEV_IO_IN;        break;
                case DEV_IO_OUTPUT_SERVICE_TAG:     cmd_srvc_type = TSCM_CMD_SRVC_DEV_IO_OUT;       break;
                case IMG_SERVICE_INPUT_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_IMG_IN;           break;
                case IMG_SERVICE_OUTPUT_TAG:        cmd_srvc_type = TSCM_CMD_SRVC_IMG_OUT;          break;
                case MEDIA_INPUT_SERVICE_TAG:       cmd_srvc_type = TSCM_CMD_SRVC_MEDIA_IN;         break;
                case MEDIA_OUTPUT_SERVICE_TAG:      cmd_srvc_type = TSCM_CMD_SRVC_MEDIA_OUT;        break;
                case PTZ_INPUT_SERVICE_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_PTZ_IN;           break;
                case PTZ_OUTPUT_SERVICE_TAG:        cmd_srvc_type = TSCM_CMD_SRVC_PTZ_OUT;          break;
                case VIDEO_ANA_INPUT_SERVICE_TAG:   cmd_srvc_type = TSCM_CMD_SRVC_VIDEO_ANAL_IN;    break;
                case VIDEO_ANA_OUTPUT_SERVICE_TAG:  cmd_srvc_type = TSCM_CMD_SRVC_VIDEO_ANAL_OUT;   break;
                case META_STRM_SERVICE_TAG:         cmd_srvc_type = TSCM_CMD_SRVC_META;             break;
            }

            if( cmd_srvc_type == TSCM_CMD_SRVC_MAX ||
                sub_cmd_code >= MAX_CMD_NUM_PER_SERVICE )
                break;

            if( !pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code] )
            {
                tscm_msg_ex(TSCM_MSG_ERR, "err, pCmd_srvc_attr[%d][%d] = Null !!", cmd_srvc_type, sub_cmd_code);
                break;
            }

            pf_cmd_ctxt_create = pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->cmd_ctxt_create;
            if( !pf_cmd_ctxt_create )
            {
                tscm_msg_ex(TSCM_MSG_ERR, "err, pf_cmd_ctxt_create[%d][%d] = Null !!", cmd_srvc_type, sub_cmd_code);
                break;
            }

            if( pCmd_box->box_arg.arg.user_handle.handler )
                pf_user_hanlder = pCmd_box->box_arg.arg.user_handle.handler;
            else
                pf_user_hanlder = pTscamDev->pCmd_srvc_attr[cmd_srvc_type][sub_cmd_code]->user_hanlder;

            pCmd_active_flag = &pTscamDev->cmd_active_flag[cmd_srvc_type];
            if( TSCAM_BIT_IS_SET(pCmd_active_flag, sub_cmd_code) )
            {
                TSCM_CMD_BOX        cur_cmd_box = {0};
                TSCM_TRANS_INFO     trans_info = {0};
                uint32_t            cmd_pkts_size = 0;
                uint8_t             *pCmd_pkts = 0;

                cur_cmd_box = (*pCmd_box);

                // create internal cmd node
                pf_cmd_ctxt_create(
                    (void*)pCmd_box->box_arg.arg.user_handle.pCmd_info,
                    (void*)&cur_cmd_box.box_arg.arg.user_handle.pCmd_info,
                    extraData);

                //---------------------------------------------
                // cmd structure -> return channel format
                cur_cmd_box.box_arg.arg.user_handle.priority = TSCM_CMD_PRIORITY_ER;
                _tscam_cmd_generate(pTscamDev, &cur_cmd_box, &pCmd_pkts, &cmd_pkts_size, extraData);

                //---------------------------------------------
                // set trans_info
                trans_info.pSteam_buf      = pCmd_pkts;
                trans_info.stream_buf_size = cmd_pkts_size;
                if( pTscamDev->hTscam.cur_desc.stream_trans )
                    result = pTscamDev->hTscam.cur_desc.stream_trans(pHTscam, &trans_info, extraData);

                if( pCmd_pkts )     free(pCmd_pkts);
            }
            else
            {
                TSCM_CMD_BOX        *pNew_cmd_box = 0;

                if( pTscamDev->hTscam.trans_cmd_num > TSCAM_MAX_QUEUE_SIZE )
                {
                    tscm_msg_ex(TSCM_MSG_ERR, "err, trans message queue full !!");
                    break;
                }

                pNew_cmd_box = tscm_malloc(sizeof(TSCM_CMD_BOX));
                if( !pNew_cmd_box  )
                {
                    result = TSCM_ERR_ALLOCATE_FAIL;
                    tscm_msg_ex(TSCM_MSG_ERR, "err, Allocate fail !!");
                    break;
                }

                memcpy(pNew_cmd_box, pCmd_box, sizeof(TSCM_CMD_BOX));

                QUEUE_INIT(pNew_cmd_box);

                pNew_cmd_box->box_arg.arg.user_handle.handler  = pf_user_hanlder;
                pNew_cmd_box->box_arg.arg.user_handle.priority = TSCM_CMD_PRIORITY_APPT;

                // create internal cmd node
                pf_cmd_ctxt_create(
                    (void*)pCmd_box->box_arg.arg.user_handle.pCmd_info,
                    (void*)&pNew_cmd_box->box_arg.arg.user_handle.pCmd_info,
                    extraData);

                if( !pTscamDev->hTscam.pCmd_trans_queue )
                {
                    pTscamDev->hTscam.pCmd_trans_queue = pNew_cmd_box;
                    pTscamDev->hTscam.pCmd_trans_head  = pNew_cmd_box;
                    pTscamDev->hTscam.trans_cmd_num    = 1;
                }
                else
                {
                    pTscamDev->hTscam.pCmd_trans_queue = QUEUE_ADD(TSCM_CMD_BOX, pTscamDev->hTscam.pCmd_trans_queue, pNew_cmd_box);
                    pTscamDev->hTscam.trans_cmd_num++;
                }
            }
        }while(0);
    }

    if( result != TSCM_ERR_OK )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
    return result;
}


/*
TSCM_ERR
tscam_tamplete(
    TSCM_HANDLE *pHTscam,
    void        *extraData)
{
    TSCM_ERR        result = TSCM_ERR_OK;
    TSCAM_DEV       *pTscamDev = 0;

    _verify_handle(pHTscam, result);

    pTscamDev = DOWN_CAST(TSCAM_DEV, pHTscam, hTscam);

    _mutex_lock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);

    if( pTscamDev )
    {
    }

    if( result != TSCM_ERR_OK )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }

    _mutex_unlock(TSCM_MSG_TRACE_TSCM, pTscamDev->tscm_mutex);
    return result;
}
*/
