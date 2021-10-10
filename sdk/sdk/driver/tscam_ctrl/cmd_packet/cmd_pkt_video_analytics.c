

#include "tscam_cmd_service.h"
#include "cmd_pkt_register.h"
#include "tscam_ctrl_def.h"
#include "cmd_pkt_codec.h"
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
#define _GET_RULE_ITEM_SIZE(pInfo, size)                                                                                                  \
    switch( pInfo->rule_type ){                                                                                                           \
        case RULE_ENGINE_LINE_DETECTOR:{                                                                                                  \
            RULE_ENG_LINE_DETECTOR  *pDetctor = (RULE_ENG_LINE_DETECTOR*)pInfo->pRule_engine_items;                                       \
            size = 4 + sizeof(POLYGON_POINT) * pDetctor->polyline_list_size;}                                                             \
            break;                                                                                                                        \
        case RULE_ENGINE_FIELD_DETECTOR:{                                                                                                 \
            RULE_ENG_FIELD_DETECTOR  *pDetctor = (RULE_ENG_FIELD_DETECTOR*)pInfo->pRule_engine_items;                                     \
            size = 2 + sizeof(POLYGON_POINT) * pDetctor->polyline_list_size;}                                                             \
            break;                                                                                                                        \
        case RULE_ENGINE_DECLARATIVE_MOTION_DETECTOR:{                                                                                    \
            RULE_ENG_DECLARATIVE_MOTION_DETECTOR  *pDetctor = (RULE_ENG_DECLARATIVE_MOTION_DETECTOR*)pInfo->pRule_engine_items;           \
            size = 4 + sizeof(POLYGON_POINT) * pDetctor->polyline_list_size;}                                                             \
            break;                                                                                                                        \
        case RULE_ENGINE_COUNTING_RULE:{                                                                                                  \
            RULE_ENG_COUNTING_RULE  *pDetctor = (RULE_ENG_COUNTING_RULE*)pInfo->pRule_engine_items;                                       \
            size = 12 + sizeof(POLYGON_POINT) * pDetctor->polyline_list_size;}                                                            \
            break;                                                                                                                        \
        case RULE_ENGINE_CELL_MOTION_DETECTOR:{                                                                                           \
            RULE_ENG_CELL_MOTION_DETECTOR  *pDetctor = (RULE_ENG_CELL_MOTION_DETECTOR*)pInfo->pRule_engine_items;                         \
            rule_items_size = 24 + (4+pDetctor->active_cells.length);}                                                                    \
            break;                                                                                                                        \
    }


#define _SET_RULE_ENG_ITEM_INFO(pInfo, pCur)                                                                                         \
    switch( pInfo->rule_type ){                                                                                                      \
        case RULE_ENGINE_LINE_DETECTOR:{                                                                                             \
                RULE_ENG_LINE_DETECTOR  *pDetctor = (RULE_ENG_LINE_DETECTOR*)pInfo->pRule_engine_items;                              \
                _SET_WORD(pCur, pDetctor->direction);                                                                                \
                _SET_BYTE(pCur, pDetctor->polyline_list_size);                                                                       \
                _SET_PAYLOAD(pCur, pDetctor->pPolygon_point, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);                    \
                _SET_BYTE(pCur, pDetctor->metadata_stream);}                                                                         \
            break;                                                                                                                   \
        case RULE_ENGINE_FIELD_DETECTOR:{                                                                                            \
                RULE_ENG_FIELD_DETECTOR  *pDetctor = (RULE_ENG_FIELD_DETECTOR*)pInfo->pRule_engine_items;                            \
                _SET_BYTE(pCur, pDetctor->polyline_list_size);                                                                       \
                _SET_PAYLOAD(pCur, pDetctor->pPolygon_point, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);                    \
                _SET_BYTE(pCur, pDetctor->metadata_stream);}                                                                         \
            break;                                                                                                                   \
        case RULE_ENGINE_DECLARATIVE_MOTION_DETECTOR:{                                                                               \
                RULE_ENG_DECLARATIVE_MOTION_DETECTOR  *pDetctor = (RULE_ENG_DECLARATIVE_MOTION_DETECTOR*)pInfo->pRule_engine_items;  \
                _SET_WORD(pCur, pDetctor->motion_expression);                                                                        \
                _SET_BYTE(pCur, pDetctor->polyline_list_size);                                                                       \
                _SET_PAYLOAD(pCur, pDetctor->pPolygon_point, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);                    \
                _SET_BYTE(pCur, pDetctor->metadata_stream);}                                                                         \
            break;                                                                                                                   \
        case RULE_ENGINE_COUNTING_RULE:{                                                                                             \
                RULE_ENG_COUNTING_RULE  *pDetctor = (RULE_ENG_COUNTING_RULE*)pInfo->pRule_engine_items;                              \
                _SET_DWORD(pCur, pDetctor->report_time_interval);                                                                    \
                _SET_DWORD(pCur, pDetctor->reset_time_interval);                                                                     \
                _SET_WORD(pCur, pDetctor->direction);                                                                                \
                _SET_BYTE(pCur, pDetctor->polyline_list_size);                                                                       \
                _SET_PAYLOAD(pCur, pDetctor->pPolygon_point, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);                    \
                _SET_BYTE(pCur, pDetctor->metadata_stream);}                                                                         \
            break;                                                                                                                   \
        case RULE_ENGINE_CELL_MOTION_DETECTOR:{                                                                                      \
                RULE_ENG_CELL_MOTION_DETECTOR  *pDetctor = (RULE_ENG_CELL_MOTION_DETECTOR*)pInfo->pRule_engine_items;                \
                _SET_WORD(pCur, pDetctor->min_count);                                                                                \
                _SET_DWORD(pCur, pDetctor->alarm_on_delay);                                                                          \
                _SET_DWORD(pCur, pDetctor->alarm_off_delay);                                                                         \
                _SET_WORD(pCur, pDetctor->active_cells_size);                                                                        \
                _SET_DWORD(pCur, pDetctor->active_cells.length);                                                                     \
                _SET_STRING(pCur, pDetctor->active_cells.pStream, pDetctor->active_cells.length);                                    \
                _SET_BYTE(pCur, pDetctor->sensitivity);                                                                              \
                _SET_WORD(pCur, pDetctor->layout_bounds_x);                                                                          \
                _SET_WORD(pCur, pDetctor->layout_bounds_y);                                                                          \
                _SET_WORD(pCur, pDetctor->layout_bounds_width);                                                                      \
                _SET_WORD(pCur, pDetctor->layout_bounds_height);                                                                     \
                _SET_BYTE(pCur, pDetctor->layout_columns);                                                                           \
                _SET_BYTE(pCur, pDetctor->layout_rows);                                                                              \
                _SET_BYTE(pCur, pDetctor->metadata_stream);}                                                                         \
            break;                                                                                                                   \
    }

#define _GET_RULE_ENG_ITEM_INFO(pInfo, pCur)                                                                          \
    switch( pInfo->rule_type ){                                                                                       \
        case RULE_ENGINE_LINE_DETECTOR:{                                                                              \
                RULE_ENG_LINE_DETECTOR  *pDetctor = 0;                                                                \
                pInfo->pRule_engine_items = tscm_malloc(sizeof(RULE_ENG_LINE_DETECTOR));                              \
                if( !pInfo->pRule_engine_items ){                                                                     \
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");                                                  \
                    break;                                                                                            \
                }                                                                                                     \
                memset(pInfo->pRule_engine_items, 0x0, sizeof(RULE_ENG_LINE_DETECTOR));                               \
                pDetctor = (RULE_ENG_LINE_DETECTOR*)pInfo->pRule_engine_items;                                        \
                pDetctor->direction          = _GET_WORD(pCur);  pCur += 2;                                           \
                pDetctor->polyline_list_size = _GET_BYTE(pCur);  pCur += 1;                                           \
                pDetctor->pPolygon_point = tscm_malloc(sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);           \
                if( !pDetctor->pPolygon_point ){                                                                      \
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");                                                  \
                    break;                                                                                            \
                }                                                                                                     \
                memcpy(pDetctor->pPolygon_point, pCur, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);           \
                pCur += (sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);                                         \
                pDetctor->metadata_stream = _GET_BYTE(pCur);  pCur += 1;}                                             \
            break;                                                                                                    \
        case RULE_ENGINE_FIELD_DETECTOR:{                                                                             \
                RULE_ENG_FIELD_DETECTOR  *pDetctor = 0;                                                               \
                pInfo->pRule_engine_items = tscm_malloc(sizeof(RULE_ENG_FIELD_DETECTOR));                             \
                if( !pInfo->pRule_engine_items ){                                                                     \
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");                                                  \
                    break;                                                                                            \
                }                                                                                                     \
                memset(pInfo->pRule_engine_items, 0x0, sizeof(RULE_ENG_FIELD_DETECTOR));                              \
                pDetctor = (RULE_ENG_FIELD_DETECTOR*)pInfo->pRule_engine_items;                                       \
                pDetctor->polyline_list_size = _GET_BYTE(pCur);  pCur += 1;                                           \
                pDetctor->pPolygon_point = tscm_malloc(sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);           \
                if( !pDetctor->pPolygon_point ){                                                                      \
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");                                                  \
                    break;                                                                                            \
                }                                                                                                     \
                memcpy(pDetctor->pPolygon_point, pCur, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);           \
                pCur += (sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);                                         \
                pDetctor->metadata_stream = _GET_BYTE(pCur);  pCur += 1;}                                             \
            break;                                                                                                    \
        case RULE_ENGINE_DECLARATIVE_MOTION_DETECTOR:{                                                                \
                RULE_ENG_DECLARATIVE_MOTION_DETECTOR  *pDetctor = 0;                                                  \
                pInfo->pRule_engine_items = tscm_malloc(sizeof(RULE_ENG_DECLARATIVE_MOTION_DETECTOR));                \
                if( !pInfo->pRule_engine_items ){                                                                     \
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");                                                  \
                    break;                                                                                            \
                }                                                                                                     \
                memset(pInfo->pRule_engine_items, 0x0, sizeof(RULE_ENG_DECLARATIVE_MOTION_DETECTOR));                 \
                pDetctor = (RULE_ENG_DECLARATIVE_MOTION_DETECTOR*)pInfo->pRule_engine_items;                          \
                pDetctor->motion_expression  = _GET_WORD(pCur);  pCur += 2;                                           \
                pDetctor->polyline_list_size = _GET_BYTE(pCur);  pCur += 1;                                           \
                pDetctor->pPolygon_point = tscm_malloc(sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);           \
                if( !pDetctor->pPolygon_point ){                                                                      \
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");                                                  \
                    break;                                                                                            \
                }                                                                                                     \
                memcpy(pDetctor->pPolygon_point, pCur, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);           \
                pCur += (sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);                                         \
                pDetctor->metadata_stream = _GET_BYTE(pCur);  pCur += 1;}                                             \
            break;                                                                                                    \
        case RULE_ENGINE_COUNTING_RULE:{                                                                              \
                RULE_ENG_COUNTING_RULE  *pDetctor = 0;                                                                \
                pInfo->pRule_engine_items = tscm_malloc(sizeof(RULE_ENG_COUNTING_RULE));                              \
                if( !pInfo->pRule_engine_items ){                                                                     \
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");                                                  \
                    break;                                                                                            \
                }                                                                                                     \
                memset(pInfo->pRule_engine_items, 0x0, sizeof(RULE_ENG_COUNTING_RULE));                               \
                pDetctor = (RULE_ENG_COUNTING_RULE*)pInfo->pRule_engine_items;                                        \
                pDetctor->report_time_interval = _GET_DWORD(pCur);  pCur += 4;                                        \
                pDetctor->reset_time_interval  = _GET_DWORD(pCur);  pCur += 4;                                        \
                pDetctor->direction            = _GET_WORD(pCur);  pCur += 2;                                         \
                pDetctor->polyline_list_size   = _GET_BYTE(pCur);  pCur += 1;                                         \
                pDetctor->pPolygon_point = tscm_malloc(sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);           \
                if( !pDetctor->pPolygon_point ){                                                                      \
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");                                                  \
                    break;                                                                                            \
                }                                                                                                     \
                memcpy(pDetctor->pPolygon_point, pCur, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);           \
                pCur += (sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);                                         \
                pDetctor->metadata_stream = _GET_BYTE(pCur);  pCur += 1;}                                             \
            break;                                                                                                    \
        case RULE_ENGINE_CELL_MOTION_DETECTOR:{                                                                       \
                RULE_ENG_CELL_MOTION_DETECTOR  *pDetctor = 0;                                                         \
                pInfo->pRule_engine_items = tscm_malloc(sizeof(RULE_ENG_CELL_MOTION_DETECTOR));                       \
                if( !pInfo->pRule_engine_items ){                                                                     \
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");                                                  \
                    break;                                                                                            \
                }                                                                                                     \
                memset(pInfo->pRule_engine_items, 0x0, sizeof(RULE_ENG_CELL_MOTION_DETECTOR));                        \
                pDetctor = (RULE_ENG_CELL_MOTION_DETECTOR*)pInfo->pRule_engine_items;                                 \
                pDetctor->min_count             = _GET_WORD(pCur);  pCur += 2;                                        \
                pDetctor->alarm_on_delay        = _GET_DWORD(pCur); pCur += 4;                                        \
                pDetctor->alarm_off_delay       = _GET_DWORD(pCur); pCur += 4;                                        \
                pDetctor->active_cells_size     = _GET_WORD(pCur);  pCur += 2;                                        \
                pDetctor->active_cells.length  = _GET_DWORD(pCur);  pCur += 4;                                        \
                pDetctor->active_cells.pStream = pCur;              pCur += pDetctor->active_cells.length;            \
                pDetctor->sensitivity           = _GET_BYTE(pCur);  pCur += 1;                                        \
                pDetctor->layout_bounds_x       = _GET_WORD(pCur);  pCur += 2;                                        \
                pDetctor->layout_bounds_y       = _GET_WORD(pCur);  pCur += 2;                                        \
                pDetctor->layout_bounds_width   = _GET_WORD(pCur);  pCur += 2;                                        \
                pDetctor->layout_bounds_height  = _GET_WORD(pCur);  pCur += 2;                                        \
                pDetctor->layout_columns        = _GET_BYTE(pCur);  pCur += 1;                                        \
                pDetctor->layout_rows           = _GET_BYTE(pCur);  pCur += 1;                                        \
                pDetctor->metadata_stream       = _GET_BYTE(pCur);  pCur += 1;                                        \
            }                                                                                                         \
            break;                                                                                                    \
    }

#define _FREE_POLYGON_POINT_DATA(pInfo)                                                                                            \
    switch( pInfo->rule_type ){                                                                                                    \
        case RULE_ENGINE_LINE_DETECTOR:{                                                                                           \
            RULE_ENG_LINE_DETECTOR  *pDetctor = (RULE_ENG_LINE_DETECTOR*)pInfo->pRule_engine_items;                                \
            if( pDetctor && pDetctor->pPolygon_point )  free(pDetctor->pPolygon_point);}                                           \
            break;                                                                                                                 \
        case RULE_ENGINE_FIELD_DETECTOR:{                                                                                          \
            RULE_ENG_FIELD_DETECTOR  *pDetctor = (RULE_ENG_FIELD_DETECTOR*)pInfo->pRule_engine_items;                              \
            if( pDetctor && pDetctor->pPolygon_point )  free(pDetctor->pPolygon_point);}                                           \
            break;                                                                                                                 \
        case RULE_ENGINE_DECLARATIVE_MOTION_DETECTOR:{                                                                             \
            RULE_ENG_DECLARATIVE_MOTION_DETECTOR  *pDetctor = (RULE_ENG_DECLARATIVE_MOTION_DETECTOR*)pInfo->pRule_engine_items;    \
            if( pDetctor && pDetctor->pPolygon_point )  free(pDetctor->pPolygon_point);}                                           \
            break;                                                                                                                 \
        case RULE_ENGINE_COUNTING_RULE:{                                                                                           \
            RULE_ENG_COUNTING_RULE  *pDetctor = (RULE_ENG_COUNTING_RULE*)pInfo->pRule_engine_items;                                \
            if( pDetctor && pDetctor->pPolygon_point )  free(pDetctor->pPolygon_point);}                                           \
            break;                                                                                                                 \
        case RULE_ENGINE_CELL_MOTION_DETECTOR:                                                                                     \
            break;                                                                                                                 \
    }
//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
//////////////////
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4            The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2            Code:0x0600
     * Reserved        1
     * User Name       n            [string] User name for security.
     * Password        n            [string] Password for security.
     * CheckSum        1            =(byte[1]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetSupportedRulesInput_Cmd_Ctxt_New, GetSupportedRulesInput_INFO, CMD_GetSupportedRulesInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetSupportedRulesInput_dec, GetSupportedRulesInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetSupportedRulesInput_Cmd_Pkt_Decode, GetSupportedRulesInput_INFO, _GetSupportedRulesInput_dec);

//////////////////
static uint32_t
GetSupportedRulesOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field                Length(Byte)    Descriptions
     * Command Length              4         The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code                2         Code: 0x8600
     * Return Code                 1         0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name                   n         [string] User name for security.
     * Password                    n         [string] Password for security.
     * SupportedRules List Size    1         Number of Supported rules in this video analytics configuration. The following entry would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { Rule type }.
     * Rule type                   N         Rule Type
     * CheckSum                    1         =(byte[1]+...+byte[N]) MOD 256
     **/
    GetSupportedRulesOutput_INFO  *pInfo = (GetSupportedRulesOutput_INFO*)input_info;
    uint32_t                              cmd_length = 0;
    uint8_t                               *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length)
                       + (4+pInfo->password.length)
                       + pInfo->supported_rules_list_size;

        pCmd_ctxt_Buf = tscm_malloc(cmd_length);
        if( !pCmd_ctxt_Buf )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }

        memset(pCmd_ctxt_Buf, 0x0, cmd_length);

        pCur = pCmd_ctxt_Buf;

        // Cmd Length
        // - real cmd_length of a cmd packet doesn't include check_sum size
        _SET_DWORD(pCur, cmd_length - 1);
        // Cmd Code
        _SET_WORD(pCur, CMD_GetSupportedRulesOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->supported_rules_list_size);
        _SET_PAYLOAD(pCur, pInfo->pRule_type, pInfo->supported_rules_list_size);
        //---------------------

        // CheckSum
        check_sum = _tscm_gen_check_sum(pCmd_ctxt_Buf, cmd_length - 1);
        pCur = pCmd_ctxt_Buf + cmd_length - 1;
        _SET_BYTE(pCur, check_sum);

        // return cmd_ctxt node buffer
        if( output_info )       *((uint8_t**)output_info) = pCmd_ctxt_Buf;
    }while(0);

    return 0;
}

static void
_GetSupportedRulesOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field                Length(Byte)    Descriptions
     * Command Length              4         The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code                2         Code: 0x8600
     * Return Code                 1         0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name                   n         [string] User name for security.
     * Password                    n         [string] Password for security.
     * SupportedRules List Size    1         Number of Supported rules in this video analytics configuration. The following entry would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { Rule type }.
     * Rule type                   n         Rule Type
     * CheckSum                    1         =(byte[1]+...+byte[N]) MOD 256
     **/
    GetSupportedRulesOutput_INFO   *pInfo = (GetSupportedRulesOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    do{
        //-----------------------------------
        // personal Cmd Data
        pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

        pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
        pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

        pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
        pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

        pInfo->supported_rules_list_size = _GET_BYTE(pCur);  pCur += 1;

        pInfo->pRule_type = tscm_malloc(pInfo->supported_rules_list_size);
        if( !pInfo->pRule_type )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }
        memcpy(pInfo->pRule_type, pCur, pInfo->supported_rules_list_size);

    }while(0);
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetSupportedRulesOutput_Cmd_Pkt_Decode, GetSupportedRulesOutput_INFO, _GetSupportedRulesOutput_dec);

static uint32_t
_GetSupportedRulesOutput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            GetSupportedRulesOutput_INFO   *pInfo = (GetSupportedRulesOutput_INFO*)(*((uint8_t**)input_info));
            if( pInfo->pRule_type )     free(pInfo->pRule_type);
        }

        free((*((uint8_t**)input_info)));
        (*((uint8_t**)input_info)) = 0;
    }while(0);

    return 0;
}
//////////////////
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4            The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2            Code:0x0601
     * Reserved        1
     * User Name       n            [string] User name for security.
     * Password        n            [string] Password for security.
     * CheckSum        1            =(byte[1]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetRulesInput_Cmd_Ctxt_New, GetRulesInput_INFO, CMD_GetRulesInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetRulesInput_dec, GetRulesInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetRulesInput_Cmd_Pkt_Decode, GetRulesInput_INFO, _GetRulesInput_dec);

//////////////////
static uint32_t
GetRulesOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field                   Length(Byte)    Descriptions
     * Command Length                  4       The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code                    2       Code: 0x8601
     * Return Code                     1       0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name                       n       [string] User name for security.
     * Password                        n       [string] Password for security.
     * RuleEngine Parameters List Size 1       List size of configuration parameters as defined in the correspding description. The following entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All GetRules parameters }.
     *   Name                            n       [string] Name of the rule.
     *   RuleToken                       n       [string] Unique rule token for this rule.
     *   Rule type                       1       Rule Type
     *   RuleEngine Items                N       The data will dependent on type of the rule (Type, Name, ...). They are defined in 10.1 RuleEngine Items section.
     * CheckSum                        1       =(byte[1]+...+byte[N]) MOD 256
     **/
    GetRulesOutput_INFO  *pInfo = (GetRulesOutput_INFO*)input_info;
    uint32_t             cmd_length = 0;
    uint8_t              *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0, *pPoint_info = 0;
        uint8_t     check_sum = 0;
        uint32_t    rule_eng_param_size = 0;
        uint32_t    polygon_point_offset = 0;
        uint32_t    point_list_size = 0;
        uint32_t    i = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length);
        for(i = 0; i < pInfo->rule_engine_para_list_size; i++)
        {
            rule_eng_param_size += (1
                                    + (4+pInfo->pRule_engine_para[i].name.length)
                                    + (4+pInfo->pRule_engine_para[i].rule_token.length));

            switch( pInfo->pRule_engine_para[i].rule_type )
            {
                case RULE_ENGINE_LINE_DETECTOR:
                    {
                        RULE_ENG_LINE_DETECTOR  *pDetctor = (RULE_ENG_LINE_DETECTOR*)pInfo->pRule_engine_para[i].pRule_engine_items;
                        rule_eng_param_size += (4 + sizeof(POLYGON_POINT) * pDetctor->polyline_list_size);
                    }
                    break;
                case RULE_ENGINE_FIELD_DETECTOR:
                    {
                        RULE_ENG_FIELD_DETECTOR  *pDetctor = (RULE_ENG_FIELD_DETECTOR*)pInfo->pRule_engine_para[i].pRule_engine_items;
                        rule_eng_param_size += (2 + sizeof(POLYGON_POINT) * pDetctor->polyline_list_size);
                    }
                    break;
                case RULE_ENGINE_DECLARATIVE_MOTION_DETECTOR:
                    {
                        RULE_ENG_DECLARATIVE_MOTION_DETECTOR  *pDetctor = (RULE_ENG_DECLARATIVE_MOTION_DETECTOR*)pInfo->pRule_engine_para[i].pRule_engine_items;
                        rule_eng_param_size += (4 + sizeof(POLYGON_POINT) * pDetctor->polyline_list_size);
                    }
                    break;
                case RULE_ENGINE_COUNTING_RULE:
                    {
                        RULE_ENG_COUNTING_RULE  *pDetctor = (RULE_ENG_COUNTING_RULE*)pInfo->pRule_engine_para[i].pRule_engine_items;
                        rule_eng_param_size += (12 + sizeof(POLYGON_POINT) * pDetctor->polyline_list_size);
                    }
                    break;
                case RULE_ENGINE_CELL_MOTION_DETECTOR:
                    {
                        RULE_ENG_CELL_MOTION_DETECTOR  *pDetctor = (RULE_ENG_CELL_MOTION_DETECTOR*)pInfo->pRule_engine_para[i].pRule_engine_items;
                        rule_eng_param_size += (24 + (4+pDetctor->active_cells.length));
                    }
                    break;
            }
        }

        cmd_length += rule_eng_param_size;
        pCmd_ctxt_Buf = tscm_malloc(cmd_length);
        if( !pCmd_ctxt_Buf )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }

        memset(pCmd_ctxt_Buf, 0x0, cmd_length);

        pCur = pCmd_ctxt_Buf;

        // Cmd Length
        // - real cmd_length of a cmd packet doesn't include check_sum size
        _SET_DWORD(pCur, cmd_length - 1);
        // Cmd Code
        _SET_WORD(pCur, CMD_GetRulesOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->rule_engine_para_list_size);

        for(i = 0; i < pInfo->rule_engine_para_list_size; i++)
        {
            _SET_DWORD(pCur, pInfo->pRule_engine_para[i].name.length);
            _SET_STRING(pCur, pInfo->pRule_engine_para[i].name.pStream, pInfo->pRule_engine_para[i].name.length);

            _SET_DWORD(pCur, pInfo->pRule_engine_para[i].rule_token.length);
            _SET_STRING(pCur, pInfo->pRule_engine_para[i].rule_token.pStream, pInfo->pRule_engine_para[i].rule_token.length);

            _SET_BYTE(pCur, (uint8_t)(pInfo->pRule_engine_para[i].rule_type & 0xFF));

            switch( pInfo->pRule_engine_para[i].rule_type )
            {
                case RULE_ENGINE_LINE_DETECTOR:
                    {
                        RULE_ENG_LINE_DETECTOR  *pDetctor = (RULE_ENG_LINE_DETECTOR*)pInfo->pRule_engine_para[i].pRule_engine_items;

                        _SET_WORD(pCur, pDetctor->direction);
                        _SET_BYTE(pCur, pDetctor->polyline_list_size);
                        _SET_PAYLOAD(pCur, pDetctor->pPolygon_point, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);
                        _SET_BYTE(pCur, pDetctor->metadata_stream);
                    }
                    break;
                case RULE_ENGINE_FIELD_DETECTOR:
                    {
                        RULE_ENG_FIELD_DETECTOR  *pDetctor = (RULE_ENG_FIELD_DETECTOR*)pInfo->pRule_engine_para[i].pRule_engine_items;

                        _SET_BYTE(pCur, pDetctor->polyline_list_size);
                        _SET_PAYLOAD(pCur, pDetctor->pPolygon_point, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);
                        _SET_BYTE(pCur, pDetctor->metadata_stream);
                    }
                    break;
                case RULE_ENGINE_DECLARATIVE_MOTION_DETECTOR:
                    {
                        RULE_ENG_DECLARATIVE_MOTION_DETECTOR  *pDetctor = (RULE_ENG_DECLARATIVE_MOTION_DETECTOR*)pInfo->pRule_engine_para[i].pRule_engine_items;

                        _SET_WORD(pCur, pDetctor->motion_expression);
                        _SET_BYTE(pCur, pDetctor->polyline_list_size);
                        _SET_PAYLOAD(pCur, pDetctor->pPolygon_point, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);
                        _SET_BYTE(pCur, pDetctor->metadata_stream);
                    }
                    break;
                case RULE_ENGINE_COUNTING_RULE:
                    {
                        RULE_ENG_COUNTING_RULE  *pDetctor = (RULE_ENG_COUNTING_RULE*)pInfo->pRule_engine_para[i].pRule_engine_items;

                        _SET_DWORD(pCur, pDetctor->report_time_interval);
                        _SET_DWORD(pCur, pDetctor->reset_time_interval);
                        _SET_WORD(pCur, pDetctor->direction);
                        _SET_BYTE(pCur, pDetctor->polyline_list_size);
                        _SET_PAYLOAD(pCur, pDetctor->pPolygon_point, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);
                        _SET_BYTE(pCur, pDetctor->metadata_stream);
                    }
                    break;
                case RULE_ENGINE_CELL_MOTION_DETECTOR:
                    {
                        RULE_ENG_CELL_MOTION_DETECTOR  *pDetctor = (RULE_ENG_CELL_MOTION_DETECTOR*)pInfo->pRule_engine_para[i].pRule_engine_items;

                        _SET_WORD(pCur, pDetctor->min_count);
                        _SET_DWORD(pCur, pDetctor->alarm_on_delay);
                        _SET_DWORD(pCur, pDetctor->alarm_off_delay);
                        _SET_WORD(pCur, pDetctor->active_cells_size);

                        _SET_DWORD(pCur, pDetctor->active_cells.length);
                        _SET_STRING(pCur, pDetctor->active_cells.pStream, pDetctor->active_cells.length);

                        _SET_BYTE(pCur, pDetctor->sensitivity);
                        _SET_WORD(pCur, pDetctor->layout_bounds_x);
                        _SET_WORD(pCur, pDetctor->layout_bounds_y);
                        _SET_WORD(pCur, pDetctor->layout_bounds_width);
                        _SET_WORD(pCur, pDetctor->layout_bounds_height);
                        _SET_BYTE(pCur, pDetctor->layout_columns);
                        _SET_BYTE(pCur, pDetctor->layout_rows);
                        _SET_BYTE(pCur, pDetctor->metadata_stream);
                    }
                    break;
            }
        }
        //---------------------

        // CheckSum
        check_sum = _tscm_gen_check_sum(pCmd_ctxt_Buf, cmd_length - 1);
        pCur = pCmd_ctxt_Buf + cmd_length - 1;
        _SET_BYTE(pCur, check_sum);

        // return cmd_ctxt node buffer
        if( output_info )       *((uint8_t**)output_info) = pCmd_ctxt_Buf;
    }while(0);

    return 0;
}

static void
_GetRulesOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field                   Length(Byte)    Descriptions
     * Command Length                  4       The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code                    2       Code: 0x8601
     * Return Code                     1       0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name                       n       [string] User name for security.
     * Password                        n       [string] Password for security.
     * RuleEngine Parameters List Size 1       List size of configuration parameters as defined in the correspding description. The following entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All GetRules parameters }.
     *   Name                            n       [string] Name of the rule.
     *   RuleToken                       n       [string] Unique rule token for this rule.
     *   Rule type                       1       Rule Type
     *   RuleEngine Items                N       The data will dependent on type of the rule (Type, Name, ...). They are defined in 10.1 RuleEngine Items section.
     * CheckSum                        1       =(byte[1]+...+byte[N]) MOD 256
     **/
    GetRulesOutput_INFO   *pInfo = (GetRulesOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    do{
        uint32_t    i = 0;

        pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

        pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
        pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

        pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
        pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

        pInfo->rule_engine_para_list_size = _GET_BYTE(pCur);  pCur += 1;

        pInfo->pRule_engine_para = tscm_malloc(sizeof(RULE_ENGINE_PARA) * pInfo->rule_engine_para_list_size);
        if( !pInfo->pRule_engine_para )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }
        memset(pInfo->pRule_engine_para, 0x0, sizeof(RULE_ENGINE_PARA) * pInfo->rule_engine_para_list_size);

        for(i = 0; i < pInfo->rule_engine_para_list_size; i++)
        {
            pInfo->pRule_engine_para[i].name.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pRule_engine_para[i].name.pStream = pCur;              pCur += pInfo->pRule_engine_para[i].name.length;

            pInfo->pRule_engine_para[i].rule_token.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pRule_engine_para[i].rule_token.pStream = pCur;              pCur += pInfo->pRule_engine_para[i].rule_token.length;

            pInfo->pRule_engine_para[i].rule_type = _GET_BYTE(pCur);  pCur += 1;

            switch( pInfo->pRule_engine_para[i].rule_type )
            {
                case RULE_ENGINE_LINE_DETECTOR:
                    {
                        RULE_ENG_LINE_DETECTOR  *pDetctor = 0;

                        pInfo->pRule_engine_para[i].pRule_engine_items = tscm_malloc(sizeof(RULE_ENG_LINE_DETECTOR));
                        if( !pInfo->pRule_engine_para[i].pRule_engine_items )
                        {
                            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                            break;
                        }
                        memset(pInfo->pRule_engine_para[i].pRule_engine_items, 0x0, sizeof(RULE_ENG_LINE_DETECTOR));

                        pDetctor = (RULE_ENG_LINE_DETECTOR*)pInfo->pRule_engine_para[i].pRule_engine_items;

                        pDetctor->direction          = _GET_WORD(pCur);  pCur += 2;
                        pDetctor->polyline_list_size = _GET_BYTE(pCur);  pCur += 1;

                        pDetctor->pPolygon_point = tscm_malloc(sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);
                        if( !pDetctor->pPolygon_point )
                        {
                            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                            break;
                        }

                        memcpy(pDetctor->pPolygon_point, pCur, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);
                        pCur += (sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);

                        pDetctor->metadata_stream = _GET_BYTE(pCur);  pCur += 1;
                    }
                    break;
                case RULE_ENGINE_FIELD_DETECTOR:
                    {
                        RULE_ENG_FIELD_DETECTOR  *pDetctor = 0;

                        pInfo->pRule_engine_para[i].pRule_engine_items = tscm_malloc(sizeof(RULE_ENG_FIELD_DETECTOR));
                        if( !pInfo->pRule_engine_para[i].pRule_engine_items )
                        {
                            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                            break;
                        }
                        memset(pInfo->pRule_engine_para[i].pRule_engine_items, 0x0, sizeof(RULE_ENG_FIELD_DETECTOR));

                        pDetctor = (RULE_ENG_FIELD_DETECTOR*)pInfo->pRule_engine_para[i].pRule_engine_items;

                        pDetctor->polyline_list_size = _GET_BYTE(pCur);  pCur += 1;

                        pDetctor->pPolygon_point = tscm_malloc(sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);
                        if( !pDetctor->pPolygon_point )
                        {
                            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                            break;
                        }

                        memcpy(pDetctor->pPolygon_point, pCur, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);
                        pCur += (sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);

                        pDetctor->metadata_stream = _GET_BYTE(pCur);  pCur += 1;
                    }
                    break;
                case RULE_ENGINE_DECLARATIVE_MOTION_DETECTOR:
                    {
                        RULE_ENG_DECLARATIVE_MOTION_DETECTOR  *pDetctor = 0;

                        pInfo->pRule_engine_para[i].pRule_engine_items = tscm_malloc(sizeof(RULE_ENG_DECLARATIVE_MOTION_DETECTOR));
                        if( !pInfo->pRule_engine_para[i].pRule_engine_items )
                        {
                            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                            break;
                        }
                        memset(pInfo->pRule_engine_para[i].pRule_engine_items, 0x0, sizeof(RULE_ENG_DECLARATIVE_MOTION_DETECTOR));

                        pDetctor = (RULE_ENG_DECLARATIVE_MOTION_DETECTOR*)pInfo->pRule_engine_para[i].pRule_engine_items;

                        pDetctor->motion_expression  = _GET_WORD(pCur);  pCur += 2;
                        pDetctor->polyline_list_size = _GET_BYTE(pCur);  pCur += 1;

                        pDetctor->pPolygon_point = tscm_malloc(sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);
                        if( !pDetctor->pPolygon_point )
                        {
                            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                            break;
                        }

                        memcpy(pDetctor->pPolygon_point, pCur, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);
                        pCur += (sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);

                        pDetctor->metadata_stream = _GET_BYTE(pCur);  pCur += 1;
                    }
                    break;
                case RULE_ENGINE_COUNTING_RULE:
                    {
                        RULE_ENG_COUNTING_RULE  *pDetctor = 0;

                        pInfo->pRule_engine_para[i].pRule_engine_items = tscm_malloc(sizeof(RULE_ENG_COUNTING_RULE));
                        if( !pInfo->pRule_engine_para[i].pRule_engine_items )
                        {
                            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                            break;
                        }
                        memset(pInfo->pRule_engine_para[i].pRule_engine_items, 0x0, sizeof(RULE_ENG_COUNTING_RULE));

                        pDetctor = (RULE_ENG_COUNTING_RULE*)pInfo->pRule_engine_para[i].pRule_engine_items;

                        pDetctor->report_time_interval = _GET_DWORD(pCur);  pCur += 4;
                        pDetctor->reset_time_interval  = _GET_DWORD(pCur);  pCur += 4;
                        pDetctor->direction            = _GET_WORD(pCur);  pCur += 2;
                        pDetctor->polyline_list_size   = _GET_BYTE(pCur);  pCur += 1;

                        pDetctor->pPolygon_point = tscm_malloc(sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);
                        if( !pDetctor->pPolygon_point )
                        {
                            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                            break;
                        }

                        memcpy(pDetctor->pPolygon_point, pCur, sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);
                        pCur += (sizeof(POLYGON_POINT)*pDetctor->polyline_list_size);

                        pDetctor->metadata_stream = _GET_BYTE(pCur);  pCur += 1;
                    }
                    break;
                case RULE_ENGINE_CELL_MOTION_DETECTOR:
                    {
                        RULE_ENG_CELL_MOTION_DETECTOR  *pDetctor = 0;

                        pInfo->pRule_engine_para[i].pRule_engine_items = tscm_malloc(sizeof(RULE_ENG_CELL_MOTION_DETECTOR));
                        if( !pInfo->pRule_engine_para[i].pRule_engine_items )
                        {
                            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                            break;
                        }
                        memset(pInfo->pRule_engine_para[i].pRule_engine_items, 0x0, sizeof(RULE_ENG_CELL_MOTION_DETECTOR));

                        pDetctor = (RULE_ENG_CELL_MOTION_DETECTOR*)pInfo->pRule_engine_para[i].pRule_engine_items;

                        pDetctor->min_count             = _GET_WORD(pCur);  pCur += 2;
                        pDetctor->alarm_on_delay        = _GET_DWORD(pCur); pCur += 4;
                        pDetctor->alarm_off_delay       = _GET_DWORD(pCur); pCur += 4;
                        pDetctor->active_cells_size     = _GET_WORD(pCur);  pCur += 2;

                        pDetctor->active_cells.length  = _GET_DWORD(pCur);  pCur += 4;
                        pDetctor->active_cells.pStream = pCur;              pCur += pDetctor->active_cells.length;

                        pDetctor->sensitivity           = _GET_BYTE(pCur);  pCur += 1;
                        pDetctor->layout_bounds_x       = _GET_WORD(pCur);  pCur += 2;
                        pDetctor->layout_bounds_y       = _GET_WORD(pCur);  pCur += 2;
                        pDetctor->layout_bounds_width   = _GET_WORD(pCur);  pCur += 2;
                        pDetctor->layout_bounds_height  = _GET_WORD(pCur);  pCur += 2;
                        pDetctor->layout_columns        = _GET_BYTE(pCur);  pCur += 1;
                        pDetctor->layout_rows           = _GET_BYTE(pCur);  pCur += 1;
                        pDetctor->metadata_stream       = _GET_BYTE(pCur);  pCur += 1;
                    }
                    break;
            }
        }
    }while(0);
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetRulesOutput_Cmd_Pkt_Decode, GetRulesOutput_INFO, _GetRulesOutput_dec);

static uint32_t
_GetRulesOutput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        uint32_t    i = 0;

        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            GetRulesOutput_INFO   *pInfo = (GetRulesOutput_INFO*)(*((uint8_t**)input_info));

            for(i = 0; i < pInfo->rule_engine_para_list_size; i++)
            {
                switch( pInfo->pRule_engine_para[i].rule_type )
                {
                    case RULE_ENGINE_LINE_DETECTOR:
                        {
                            RULE_ENG_LINE_DETECTOR  *pDetctor = (RULE_ENG_LINE_DETECTOR*)pInfo->pRule_engine_para[i].pRule_engine_items;
                            if( pDetctor && pDetctor->pPolygon_point )  free(pDetctor->pPolygon_point);
                        }
                        break;
                    case RULE_ENGINE_FIELD_DETECTOR:
                        {
                            RULE_ENG_FIELD_DETECTOR  *pDetctor = (RULE_ENG_FIELD_DETECTOR*)pInfo->pRule_engine_para[i].pRule_engine_items;
                            if( pDetctor && pDetctor->pPolygon_point )  free(pDetctor->pPolygon_point);
                        }
                        break;
                    case RULE_ENGINE_DECLARATIVE_MOTION_DETECTOR:
                        {
                            RULE_ENG_DECLARATIVE_MOTION_DETECTOR  *pDetctor = (RULE_ENG_DECLARATIVE_MOTION_DETECTOR*)pInfo->pRule_engine_para[i].pRule_engine_items;
                            if( pDetctor && pDetctor->pPolygon_point )  free(pDetctor->pPolygon_point);
                        }
                        break;
                    case RULE_ENGINE_COUNTING_RULE:
                        {
                            RULE_ENG_COUNTING_RULE  *pDetctor = (RULE_ENG_COUNTING_RULE*)pInfo->pRule_engine_para[i].pRule_engine_items;
                            if( pDetctor && pDetctor->pPolygon_point )  free(pDetctor->pPolygon_point);
                        }
                        break;
                    case RULE_ENGINE_CELL_MOTION_DETECTOR:
                        break;
                }

                if( pInfo->pRule_engine_para[i].pRule_engine_items )     free(pInfo->pRule_engine_para[i].pRule_engine_items);
            }

            if( pInfo->pRule_engine_para )     free(pInfo->pRule_engine_para);
        }

        free((*((uint8_t**)input_info)));
        (*((uint8_t**)input_info)) = 0;
    }while(0);

    return 0;
}

//////////////////
static uint32_t
CreateRuleInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code:0x0680
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] Name of the rule.
     * RuleToken           n           [string] Unique rule token for this rule.
     * Rule type           1           Rule Type
     * RuleEngine Item     N           The data will dependent on type of the rule (Type, Name, ...). They are defined in 10.1 RuleEngine Items section.
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    CreateRuleInput_INFO  *pInfo = (CreateRuleInput_INFO*)input_info;
    uint32_t              cmd_length = 0;
    uint8_t               *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;
        uint32_t    rule_items_size = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length)
                       + (4+pInfo->password.length)
                       + (4+pInfo->name.length)
                       + (4+pInfo->rule_token.length);

        _GET_RULE_ITEM_SIZE(pInfo, rule_items_size);

        cmd_length += rule_items_size;
        pCmd_ctxt_Buf = tscm_malloc(cmd_length);
        if( !pCmd_ctxt_Buf )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }

        memset(pCmd_ctxt_Buf, 0x0, cmd_length);

        pCur = pCmd_ctxt_Buf;

        // Cmd Length
        // - real cmd_length of a cmd packet doesn't include check_sum size
        _SET_DWORD(pCur, cmd_length - 1);
        // Cmd Code
        _SET_WORD(pCur, CMD_CreateRuleInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->name.length);
        _SET_STRING(pCur, pInfo->name.pStream, pInfo->name.length);

        _SET_DWORD(pCur, pInfo->rule_token.length);
        _SET_STRING(pCur, pInfo->rule_token.pStream, pInfo->rule_token.length);

        _SET_BYTE(pCur, (uint8_t)(pInfo->rule_type & 0xFF));

        _SET_RULE_ENG_ITEM_INFO(pInfo, pCur);
        //---------------------

        // CheckSum
        check_sum = _tscm_gen_check_sum(pCmd_ctxt_Buf, cmd_length - 1);
        pCur = pCmd_ctxt_Buf + cmd_length - 1;
        _SET_BYTE(pCur, check_sum);

        // return cmd_ctxt node buffer
        if( output_info )       *((uint8_t**)output_info) = pCmd_ctxt_Buf;
    }while(0);

    return 0;
}

static void
_CreateRuleInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code:0x0680
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] Name of the rule.
     * RuleToken           n           [string] Unique rule token for this rule.
     * Rule type           1           Rule Type
     * RuleEngine Item     N           The data will dependent on type of the rule (Type, Name, ...). They are defined in 10.1 RuleEngine Items section.
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    CreateRuleInput_INFO   *pInfo = (CreateRuleInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->name.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->name.pStream = pCur;              pCur += pInfo->name.length;

    pInfo->rule_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->rule_token.pStream = pCur;              pCur += pInfo->rule_token.length;

    pInfo->rule_type = _GET_BYTE(pCur);  pCur += 1;

    _GET_RULE_ENG_ITEM_INFO(pInfo, pCur);
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(CreateRuleInput_Cmd_Pkt_Decode, CreateRuleInput_INFO, _CreateRuleInput_dec);

static uint32_t
_CreateRuleInput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            CreateRuleInput_INFO   *pInfo = (CreateRuleInput_INFO*)(*((uint8_t**)input_info));

            _FREE_POLYGON_POINT_DATA(pInfo);

            if( pInfo->pRule_engine_items )     free(pInfo->pRule_engine_items);
        }

        free((*((uint8_t**)input_info)));
        (*((uint8_t**)input_info)) = 0;
    }while(0);

    return 0;
}
//////////////////
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4            The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2            Code:0x8680
     * Reserved        1
     * User Name       n            [string] User name for security.
     * Password        n            [string] Password for security.
     * CheckSum        1            =(byte[1]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(CreateRuleOutput_Cmd_Ctxt_New, CreateRuleOutput_INFO, CMD_CreateRuleOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_CreateRuleOutput_dec, CreateRuleOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(CreateRuleOutput_Cmd_Pkt_Decode, CreateRuleOutput_INFO, _CreateRuleOutput_dec);

//////////////////
static uint32_t
ModifyRuleInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code:0x0681
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] Name of the rule.
     * RuleToken           n           [string] Unique rule token for this rule
     * Rule type           1           Rule Type
     * RuleEngine Items    n           The data will dependent on type of the rule (Type, Name, ...). They are defined in 10.1 RuleEngine Items section.
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    ModifyRuleInput_INFO  *pInfo = (ModifyRuleInput_INFO*)input_info;
    uint32_t              cmd_length = 0;
    uint8_t               *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;
        uint32_t    rule_items_size = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length)
                       + (4+pInfo->password.length)
                       + (4+pInfo->name.length)
                       + (4+pInfo->rule_token.length);

        _GET_RULE_ITEM_SIZE(pInfo, rule_items_size);

        cmd_length += rule_items_size;
        pCmd_ctxt_Buf = tscm_malloc(cmd_length);
        if( !pCmd_ctxt_Buf )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }

        memset(pCmd_ctxt_Buf, 0x0, cmd_length);

        pCur = pCmd_ctxt_Buf;

        // Cmd Length
        // - real cmd_length of a cmd packet doesn't include check_sum size
        _SET_DWORD(pCur, cmd_length - 1);
        // Cmd Code
        _SET_WORD(pCur, CMD_ModifyRuleInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->name.length);
        _SET_STRING(pCur, pInfo->name.pStream, pInfo->name.length);

        _SET_DWORD(pCur, pInfo->rule_token.length);
        _SET_STRING(pCur, pInfo->rule_token.pStream, pInfo->rule_token.length);

        _SET_BYTE(pCur, (uint8_t)(pInfo->rule_type & 0xFF));

        _SET_RULE_ENG_ITEM_INFO(pInfo, pCur);
        //---------------------

        // CheckSum
        check_sum = _tscm_gen_check_sum(pCmd_ctxt_Buf, cmd_length - 1);
        pCur = pCmd_ctxt_Buf + cmd_length - 1;
        _SET_BYTE(pCur, check_sum);

        // return cmd_ctxt node buffer
        if( output_info )       *((uint8_t**)output_info) = pCmd_ctxt_Buf;
    }while(0);

    return 0;
}

static void
_ModifyRuleInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code:0x0681
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] Name of the rule.
     * RuleToken           n           [string] Unique rule token for this rule
     * Rule type           1           Rule Type
     * RuleEngine Items    n           The data will dependent on type of the rule (Type, Name, ...). They are defined in 10.1 RuleEngine Items section.
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    ModifyRuleInput_INFO   *pInfo = (ModifyRuleInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->name.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->name.pStream = pCur;              pCur += pInfo->name.length;

    pInfo->rule_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->rule_token.pStream = pCur;              pCur += pInfo->rule_token.length;

    pInfo->rule_type = _GET_BYTE(pCur);  pCur += 1;

    _GET_RULE_ENG_ITEM_INFO(pInfo, pCur);
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(ModifyRuleInput_Cmd_Pkt_Decode, ModifyRuleInput_INFO, _ModifyRuleInput_dec);

static uint32_t
_ModifyRuleInput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            ModifyRuleInput_INFO   *pInfo = (ModifyRuleInput_INFO*)(*((uint8_t**)input_info));

            _FREE_POLYGON_POINT_DATA(pInfo);

            if( pInfo->pRule_engine_items )     free(pInfo->pRule_engine_items);
        }

        free((*((uint8_t**)input_info)));
        (*((uint8_t**)input_info)) = 0;
    }while(0);

    return 0;
}
//////////////////
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4            The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2            Code:0x8681
     * Reserved        1
     * User Name       n            [string] User name for security.
     * Password        n            [string] Password for security.
     * CheckSum        1            =(byte[1]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(ModifyRuleOutput_Cmd_Ctxt_New, ModifyRuleOutput_INFO, CMD_ModifyRuleOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_ModifyRuleOutput_dec, ModifyRuleOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(ModifyRuleOutput_Cmd_Pkt_Decode, ModifyRuleOutput_INFO, _ModifyRuleOutput_dec);

//////////////////
static uint32_t
DeleteRuleInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code:0x0682
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * RuleToken           n           [string] References the specific rule
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    DeleteRuleInput_INFO  *pInfo = (DeleteRuleInput_INFO*)input_info;
    uint32_t              cmd_length = 0;
    uint8_t               *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->rule_token.length);

        pCmd_ctxt_Buf = tscm_malloc(cmd_length);
        if( !pCmd_ctxt_Buf )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }

        memset(pCmd_ctxt_Buf, 0x0, cmd_length);

        pCur = pCmd_ctxt_Buf;

        // Cmd Length
        // - real cmd_length of a cmd packet doesn't include check_sum size
        _SET_DWORD(pCur, cmd_length - 1);
        // Cmd Code
        _SET_WORD(pCur, CMD_DeleteRuleInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->rule_token.length);
        _SET_STRING(pCur, pInfo->rule_token.pStream, pInfo->rule_token.length);
        //---------------------

        // CheckSum
        check_sum = _tscm_gen_check_sum(pCmd_ctxt_Buf, cmd_length - 1);
        pCur = pCmd_ctxt_Buf + cmd_length - 1;
        _SET_BYTE(pCur, check_sum);

        // return cmd_ctxt node buffer
        if( output_info )       *((uint8_t**)output_info) = pCmd_ctxt_Buf;
    }while(0);

    return 0;
}

static void
_DeleteRuleInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code:0x0682
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * RuleToken           n           [string] References the specific rule
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    DeleteRuleInput_INFO   *pInfo = (DeleteRuleInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->rule_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->rule_token.pStream = pCur;              pCur += pInfo->rule_token.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(DeleteRuleInput_Cmd_Pkt_Decode, DeleteRuleInput_INFO, _DeleteRuleInput_dec);

//////////////////
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4            The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2            Code:0x8682
     * Reserved        1
     * User Name       n            [string] User name for security.
     * Password        n            [string] Password for security.
     * CheckSum        1            =(byte[1]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(DeleteRuleOutput_Cmd_Ctxt_New, DeleteRuleOutput_INFO, CMD_DeleteRuleOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_DeleteRuleOutput_dec, DeleteRuleOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(DeleteRuleOutput_Cmd_Pkt_Decode, DeleteRuleOutput_INFO, _DeleteRuleOutput_dec);

//=============================================================================
//                Public Function Definition
//=============================================================================

// DEFINE_CMD_PKT_CODEC(CMD_xxxInput, xxxInput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
// DEFINE_CMD_PKT_CODEC(CMD_xxxOutput, xxxOutput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);

DEFINE_CMD_PKT_CODEC(CMD_GetSupportedRulesInput, GetSupportedRulesInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetSupportedRulesInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetSupportedRulesOutput, GetSupportedRulesOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetSupportedRulesOutput_Cmd_Pkt_Decode, _GetSupportedRulesOutput_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetRulesInput, GetRulesInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetRulesInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetRulesOutput, GetRulesOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetRulesOutput_Cmd_Pkt_Decode, _GetRulesOutput_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_CreateRuleInput, CreateRuleInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, CreateRuleInput_Cmd_Pkt_Decode, _CreateRuleInput_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_CreateRuleOutput, CreateRuleOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, CreateRuleOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_ModifyRuleInput, ModifyRuleInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, ModifyRuleInput_Cmd_Pkt_Decode, _ModifyRuleInput_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_ModifyRuleOutput, ModifyRuleOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, ModifyRuleOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_DeleteRuleInput, DeleteRuleInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, DeleteRuleInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_DeleteRuleOutput, DeleteRuleOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, DeleteRuleOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);



