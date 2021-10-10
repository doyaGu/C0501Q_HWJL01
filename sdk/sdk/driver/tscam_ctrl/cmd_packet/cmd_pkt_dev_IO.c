

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
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0200
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetDigitalInputsInput_Cmd_Ctxt_New, GetDigitalInputsInput_INFO, CMD_GetDigitalInputsInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetDigitalInputsInput_dec, GetDigitalInputsInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetDigitalInputsInput_Cmd_Pkt_Decode, GetDigitalInputsInput_INFO, _GetDigitalInputsInput_dec);

//////////////////
static uint32_t
GetDigitalInputsOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field                   Length(Byte)    Descriptions
     * Command Length              4           The total length of this command. It doesn't include the CheckSum.
     * Command Code                2           Code: 0x8200
     * Return Code                 1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                   n           [string] User name for security.
     * Password                    n           [string] Password for security.
     * DigitalInputs List Size     1           The following entry would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = {Token}.
     * Token List                  n*n         [stringList] The list of the unique identifier referencing the physical entity.
     * CheckSum                    1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetDigitalInputsOutput_INFO  *pInfo = (GetDigitalInputsOutput_INFO*)input_info;
    uint32_t                     cmd_length = 0;
    uint8_t                      *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;
        uint32_t    i = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length);// + 4;
        for(i = 0; i < pInfo->token_list.list_size; i++)
            cmd_length += (4+pInfo->token_list.pCmd_string[i].length);

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
        _SET_WORD(pCur, CMD_GetDigitalInputsOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->digitalinputs_list_size);

        // _SET_DWORD(pCur, pInfo->token_list.list_size);
        for(i = 0; i < pInfo->token_list.list_size; i++)
        {
            _SET_DWORD(pCur, pInfo->token_list.pCmd_string[i].length);
            _SET_STRING(pCur, pInfo->token_list.pCmd_string[i].pStream, pInfo->token_list.pCmd_string[i].length);
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
_GetDigitalInputsOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field                   Length(Byte)    Descriptions
     * Command Length              4           The total length of this command. It doesn't include the CheckSum.
     * Command Code                2           Code: 0x8200
     * Return Code                 1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                   n           [string] User name for security.
     * Password                    n           [string] Password for security.
     * DigitalInputs List Size     1           The following entry would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = {Token}.
     * Token List                  n*n         [stringList] The list of the unique identifier referencing the physical entity.
     * CheckSum                    1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetDigitalInputsOutput_INFO   *pInfo = (GetDigitalInputsOutput_INFO*)pStruct_Info;

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

        pInfo->digitalinputs_list_size = _GET_BYTE(pCur); pCur += 1;

        // lenght(uint32_t) need 32-alignment, but input stream is uint8_t type
        pInfo->token_list.list_size   = pInfo->digitalinputs_list_size; //_GET_DWORD(pCur);  pCur += 4;
        pInfo->token_list.pCmd_string = tscm_malloc(sizeof(CMD_STRING_T)*pInfo->token_list.list_size);
        if( !pInfo->token_list.pCmd_string )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }

        memset(pInfo->token_list.pCmd_string, 0x0, sizeof(CMD_STRING_T)*pInfo->token_list.list_size);
        for(i = 0; i < pInfo->token_list.list_size; i++)
        {
            pInfo->token_list.pCmd_string[i].length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->token_list.pCmd_string[i].pStream = pCur;              pCur += pInfo->token_list.pCmd_string[i].length;
        }
    }while(0);
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetDigitalInputsOutput_Cmd_Pkt_Decode, GetDigitalInputsOutput_INFO, _GetDigitalInputsOutput_dec);

static uint32_t
_GetDigitalInputsOutput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            GetDigitalInputsOutput_INFO   *pInfo = (GetDigitalInputsOutput_INFO*)(*((uint8_t**)input_info));
            if( pInfo->token_list.pCmd_string )     free(pInfo->token_list.pCmd_string);
        }

        free((*((uint8_t**)input_info)));
        (*((uint8_t**)input_info)) = 0;
    }while(0);

    return 0;
}

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0201
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetRelayOutputsInput_Cmd_Ctxt_New, GetRelayOutputsInput_INFO, CMD_GetRelayOutputsInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetRelayOutputsInput_dec, GetRelayOutputsInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetRelayOutputsInput_Cmd_Pkt_Decode, GetRelayOutputsInput_INFO, _GetRelayOutputsInput_dec);

//////////////////
static uint32_t
GetRelayOutputsOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length          4       The total length of this command. It doesn't include the CheckSum.
     * Command Code            2       Code: 0x8201
     * Return Code             1       0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name               n       [string] User name for security.
     * Password                n       [string] Password for security.
     * RelayOutputs List Size  1       The following 4 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All RelayOutput parameters }.
     *   Token List              n       [string] The list of unique identifier referencing the physical entity.
     *   Mode                    1       0: Monostable, 1: Bistable
     *   DelayTime               4       In micro-second . Time after which the relay returns to its idle state if it is in monostable mode. If the Mode field is set to bistable mode the value of the parameter can be ignored.
     *   IdleState               1       0: closed, 1:open
     * CheckSum                1       =(byte[0]+...+byte[N]) MOD 256
     **/
    GetRelayOutputsOutput_INFO  *pInfo = (GetRelayOutputsOutput_INFO*)input_info;
    uint32_t                    cmd_length = 0;
    uint8_t                     *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;
        uint32_t    i = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length);
        for(i = 0; i < pInfo->relayoutputs_list_size; i++)
            cmd_length += (6 + (4+pInfo->pRelayoutputs_data[i].token_list.length));

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
        _SET_WORD(pCur, CMD_GetRelayOutputsOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->relayoutputs_list_size);

        // _SET_DWORD(pCur, pInfo->token_list.list_size);
        for(i = 0; i < pInfo->relayoutputs_list_size; i++)
        {
            _SET_DWORD(pCur, pInfo->pRelayoutputs_data[i].token_list.length);
            _SET_STRING(pCur, pInfo->pRelayoutputs_data[i].token_list.pStream, pInfo->pRelayoutputs_data[i].token_list.length);

            _SET_BYTE(pCur, pInfo->pRelayoutputs_data[i].mode);
            _SET_DWORD(pCur, pInfo->pRelayoutputs_data[i].delay_time);
            _SET_BYTE(pCur, pInfo->pRelayoutputs_data[i].idle_state);
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
_GetRelayOutputsOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length          4       The total length of this command. It doesn't include the CheckSum.
     * Command Code            2       Code: 0x8201
     * Return Code             1       0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name               n       [string] User name for security.
     * Password                n       [string] Password for security.
     * RelayOutputs List Size  1       The following 4 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All RelayOutput parameters }.
     *   Token List              N       [stringList] The list of unique identifier referencing the physical entity.
     *   Mode                    1       0: Monostable, 1: Bistable
     *   DelayTime               4       In micro-second . Time after which the relay returns to its idle state if it is in monostable mode. If the Mode field is set to bistable mode the value of the parameter can be ignored.
     *   IdleState               1       0: closed, 1:open
     * CheckSum                1       =(byte[0]+...+byte[N]) MOD 256
     **/
    GetRelayOutputsOutput_INFO   *pInfo = (GetRelayOutputsOutput_INFO*)pStruct_Info;

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

        pInfo->relayoutputs_list_size = _GET_BYTE(pCur); pCur += 1;

        // lenght(uint32_t) need 32-alignment, but input stream is uint8_t type
        pInfo->pRelayoutputs_data = tscm_malloc(sizeof(RELAY_OUTPUTS_DATA) * pInfo->relayoutputs_list_size);
        if( !pInfo->pRelayoutputs_data )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }

        memset(pInfo->pRelayoutputs_data, 0x0, sizeof(RELAY_OUTPUTS_DATA) * pInfo->relayoutputs_list_size);
        for(i = 0; i < pInfo->relayoutputs_list_size; i++)
        {
            pInfo->pRelayoutputs_data[i].token_list.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pRelayoutputs_data[i].token_list.pStream = pCur;              pCur += pInfo->pRelayoutputs_data[i].token_list.length;

            pInfo->pRelayoutputs_data[i].mode       = _GET_BYTE(pCur);  pCur += 1;
            pInfo->pRelayoutputs_data[i].delay_time = _GET_DWORD(pCur); pCur += 4;
            pInfo->pRelayoutputs_data[i].idle_state = _GET_BYTE(pCur);  pCur += 1;
        }
    }while(0);
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetRelayOutputsOutput_Cmd_Pkt_Decode, GetRelayOutputsOutput_INFO, _GetRelayOutputsOutput_dec);

static uint32_t
_GetRelayOutputsOutput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            GetRelayOutputsOutput_INFO   *pInfo = (GetRelayOutputsOutput_INFO*)(*((uint8_t**)input_info));
            if( pInfo->pRelayoutputs_data )     free(pInfo->pRelayoutputs_data);
        }

        free((*((uint8_t**)input_info)));
        (*((uint8_t**)input_info)) = 0;
    }while(0);

    return 0;
}
//////////////////
static uint32_t
SetRelayOutputStateInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0280
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * RelayOutputToken    n           [string]
     * LogicalState        1           0: active, 1: inactive
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetRelayOutputStateInput_INFO  *pInfo = (SetRelayOutputStateInput_INFO*)input_info;
    uint32_t                       cmd_length = 0;
    uint8_t                        *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->relay_output_token.length);

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
        _SET_WORD(pCur, CMD_SetRelayOutputStateInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->relay_output_token.length);
        _SET_STRING(pCur, pInfo->relay_output_token.pStream, pInfo->relay_output_token.length);

        _SET_BYTE(pCur, pInfo->logical_state);
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
_SetRelayOutputStateInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0280
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * RelayOutputToken    n           [string]
     * LogicalState        1           0: active, 1: inactive
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetRelayOutputStateInput_INFO   *pInfo = (SetRelayOutputStateInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->relay_output_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->relay_output_token.pStream = pCur;              pCur += pInfo->relay_output_token.length;

    pInfo->logical_state = _GET_BYTE(pCur);      pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetRelayOutputStateInput_Cmd_Pkt_Decode, SetRelayOutputStateInput_INFO, _SetRelayOutputStateInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8280
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(SetRelayOutputStateOutput_Cmd_Ctxt_New, SetRelayOutputStateOutput_INFO, CMD_SetRelayOutputStateOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_SetRelayOutputStateOutput_dec, SetRelayOutputStateOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(SetRelayOutputStateOutput_Cmd_Pkt_Decode, SetRelayOutputStateOutput_INFO, _SetRelayOutputStateOutput_dec);

//////////////////
static uint32_t
SetRelayOutputSettingsInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0281
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * token               n           [string] Unique identifier referencing the physical entity.
     * Mode                1           0: Monostable, 1: Bistable
     * DelayTime           4           In micro-second. Time after which the relay returns to its idle state if it is in monostable mode. If the Mode field is set to bistable mode the value of the parameter can be ignored.
     * IdleState           1           0: closed, 1: open
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetRelayOutputSettingsInput_INFO  *pInfo = (SetRelayOutputSettingsInput_INFO*)input_info;
    uint32_t                          cmd_length = 0;
    uint8_t                           *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 14 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->token.length);

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
        _SET_WORD(pCur, CMD_SetRelayOutputSettingsInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);

        _SET_BYTE(pCur, pInfo->mode);
        _SET_DWORD(pCur, pInfo->delay_time);
        _SET_BYTE(pCur, pInfo->idle_state);
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
_SetRelayOutputSettingsInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0281
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * token               n           [string] Unique identifier referencing the physical entity.
     * Mode                1           0: Monostable, 1: Bistable
     * DelayTime           4           In micro-second. Time after which the relay returns to its idle state if it is in monostable mode. If the Mode field is set to bistable mode the value of the parameter can be ignored.
     * IdleState           1           0: closed, 1: open
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetRelayOutputSettingsInput_INFO   *pInfo = (SetRelayOutputSettingsInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->token.pStream = pCur;              pCur += pInfo->token.length;

    pInfo->mode = _GET_BYTE(pCur);         pCur += 1;
    pInfo->delay_time = _GET_DWORD(pCur);  pCur += 4;
    pInfo->idle_state = _GET_BYTE(pCur);   pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetRelayOutputSettingsInput_Cmd_Pkt_Decode, SetRelayOutputSettingsInput_INFO, _SetRelayOutputSettingsInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8281
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(SetRelayOutputSettingsOutput_Cmd_Ctxt_New, SetRelayOutputSettingsOutput_INFO, CMD_SetRelayOutputSettingsOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_SetRelayOutputSettingsOutput_dec, SetRelayOutputSettingsOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(SetRelayOutputSettingsOutput_Cmd_Pkt_Decode, SetRelayOutputSettingsOutput_INFO, _SetRelayOutputSettingsOutput_dec);


//=============================================================================
//                Public Function Definition
//=============================================================================

// DEFINE_CMD_PKT_CODEC(CMD_xxxInput, xxxInput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
// DEFINE_CMD_PKT_CODEC(CMD_xxxOutput, xxxOutput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);

DEFINE_CMD_PKT_CODEC(CMD_GetDigitalInputsInput, GetDigitalInputsInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetDigitalInputsInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetDigitalInputsOutput, GetDigitalInputsOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetDigitalInputsOutput_Cmd_Pkt_Decode, _GetDigitalInputsOutput_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetRelayOutputsInput, GetRelayOutputsInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetRelayOutputsInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetRelayOutputsOutput, GetRelayOutputsOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetRelayOutputsOutput_Cmd_Pkt_Decode, _GetRelayOutputsOutput_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetRelayOutputStateInput, SetRelayOutputStateInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetRelayOutputStateInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetRelayOutputStateOutput, SetRelayOutputStateOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetRelayOutputStateOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetRelayOutputSettingsInput, SetRelayOutputSettingsInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetRelayOutputSettingsInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetRelayOutputSettingsOutput, SetRelayOutputSettingsOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetRelayOutputSettingsOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


