

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
     * Field           Length(Byte)  Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code:0x0500
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetConfigurationsInput_Cmd_Ctxt_New, GetConfigurationsInput_INFO, CMD_GetConfigurationsInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetConfigurationsInput_dec, GetConfigurationsInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetConfigurationsInput_Cmd_Pkt_Decode, GetConfigurationsInput_INFO, _GetConfigurationsInput_dec);

//////////////////
static uint32_t
GetConfigurationsOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field                     Length(Byte)    Descriptions
     * Command Length                  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code                    2           Code: 0x8500
     * Return Code                     1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name                       n           [string] User name for security.
     * Password                        n           [string] Password for security.
     * PTZConfiguration List Size      1           A list of all existing PTZConfigurations on the device. The following 15 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All PTZConfiguration parameters }.
     *   Name                            n           [string] User readable name. Length up to 64 characters.
     *   UseCount                        1           Number of internal references currently using this configuration.
     *   token                           n           [string] Token that uniquely refernces this configuration.
     *   DefaultPanSpeed                 2           [short] Pan speed.
     *   DefaultTiltSpeed                2           [short] Tilt speed.
     *   DefaultZoomSpeed                2           [short] Zoom speed.
     *   DefaultTimeout                  4           If the PTZ Node supports continuous movements, it shall specify a default timeout, after which the movement stops. (us)
     *   PanLimitMin                     2           Pan limit minmum
     *   PanLimitMax                     2           Pan limit maxmum
     *   TiltLimitMin                    2           Tilt limit minmum
     *   TiltLimitMax                    2           Tilt limit maxmum
     *   ZoomLimitMin                    2           Zoom limit minmum
     *   ZoomLimitMax                    2           Zoom limit maxmum
     *   PTZControlDirection EFlip Mode  1           0: OFF, 1: ON, 2: Extended
     *   PTControlDirection Reverse Mode 1           0: OFF, 1: ON, 2: AUTO, 3: Extended
     * CheckSum                        1           =(byte[1]+...+byte[N]) MOD 256
     **/
    GetConfigurationsOutput_INFO  *pInfo = (GetConfigurationsOutput_INFO*)input_info;
    uint32_t                      cmd_length = 0;
    uint8_t                       *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;
        uint32_t    i = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length);
        for(i = 0; i < pInfo->ptz_configuration_list_size; i++)
            cmd_length += (25 + (4+pInfo->pPtz_configuration[i].name.length) + (4+pInfo->pPtz_configuration[i].token.length));

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
        _SET_WORD(pCur, CMD_GetConfigurationsOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->ptz_configuration_list_size);

        for(i = 0; i < pInfo->ptz_configuration_list_size; i++)
        {
            _SET_DWORD(pCur, pInfo->pPtz_configuration[i].name.length);
            _SET_STRING(pCur, pInfo->pPtz_configuration[i].name.pStream, pInfo->pPtz_configuration[i].name.length);

            _SET_BYTE(pCur, pInfo->pPtz_configuration[i].use_count);

            _SET_DWORD(pCur, pInfo->pPtz_configuration[i].token.length);
            _SET_STRING(pCur, pInfo->pPtz_configuration[i].token.pStream, pInfo->pPtz_configuration[i].token.length);

            _SET_WORD(pCur, (uint16_t)pInfo->pPtz_configuration[i].default_pan_speed);
            _SET_WORD(pCur, (uint16_t)pInfo->pPtz_configuration[i].default_tilt_speed);
            _SET_WORD(pCur, (uint16_t)pInfo->pPtz_configuration[i].default_zoom_speed);
            _SET_DWORD(pCur, pInfo->pPtz_configuration[i].default_timeout);
            _SET_WORD(pCur, pInfo->pPtz_configuration[i].pan_limit_min);
            _SET_WORD(pCur, pInfo->pPtz_configuration[i].pan_limit_max);
            _SET_WORD(pCur, pInfo->pPtz_configuration[i].tilt_limit_min);
            _SET_WORD(pCur, pInfo->pPtz_configuration[i].tilt_limit_max);
            _SET_WORD(pCur, pInfo->pPtz_configuration[i].zoom_limit_min);
            _SET_WORD(pCur, pInfo->pPtz_configuration[i].zoom_limit_max);
            _SET_BYTE(pCur, pInfo->pPtz_configuration[i].ptz_control_direction_eflip_mode);
            _SET_BYTE(pCur, pInfo->pPtz_configuration[i].ptz_control_direction_reverse_mode);
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
_GetConfigurationsOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field                     Length(Byte)    Descriptions
     * Command Length                  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code                    2           Code: 0x8500
     * Return Code                     1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name                       n           [string] User name for security.
     * Password                        n           [string] Password for security.
     * PTZConfiguration List Size      1           A list of all existing PTZConfigurations on the device. The following 15 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All PTZConfiguration parameters }.
     *   Name                            n           [string] User readable name. Length up to 64 characters.
     *   UseCount                        1           Number of internal references currently using this configuration.
     *   token                           n           [string] Token that uniquely refernces this configuration.
     *   DefaultPanSpeed                 2           [short] Pan speed.
     *   DefaultTiltSpeed                2           [short] Tilt speed.
     *   DefaultZoomSpeed                2           [short] Zoom speed.
     *   DefaultTimeout                  4           If the PTZ Node supports continuous movements, it shall specify a default timeout, after which the movement stops. (us)
     *   PanLimitMin                     2           Pan limit minmum
     *   PanLimitMax                     2           Pan limit maxmum
     *   TiltLimitMin                    2           Tilt limit minmum
     *   TiltLimitMax                    2           Tilt limit maxmum
     *   ZoomLimitMin                    2           Zoom limit minmum
     *   ZoomLimitMax                    2           Zoom limit maxmum
     *   PTZControlDirection EFlip Mode  1           0: OFF, 1: ON, 2: Extended
     *   PTControlDirection Reverse Mode 1           0: OFF, 1: ON, 2: AUTO, 3: Extended
     * CheckSum                        1           =(byte[1]+...+byte[N]) MOD 256
     **/
    GetConfigurationsOutput_INFO   *pInfo = (GetConfigurationsOutput_INFO*)pStruct_Info;

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

        pInfo->ptz_configuration_list_size = _GET_BYTE(pCur);   pCur += 1;

        pInfo->pPtz_configuration = tscm_malloc(sizeof(PTZ_CONFIGURATION) * pInfo->ptz_configuration_list_size);
        if( !pInfo->pPtz_configuration )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }
        memset(pInfo->pPtz_configuration, 0x0, sizeof(PTZ_CONFIGURATION) * pInfo->ptz_configuration_list_size);

        for(i = 0; i < pInfo->ptz_configuration_list_size; i++)
        {
            pInfo->pPtz_configuration[i].name.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pPtz_configuration[i].name.pStream = pCur;              pCur += pInfo->pPtz_configuration[i].name.length;

            pInfo->pPtz_configuration[i].use_count = _GET_BYTE(pCur);   pCur += 1;

            pInfo->pPtz_configuration[i].token.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pPtz_configuration[i].token.pStream = pCur;              pCur += pInfo->pPtz_configuration[i].token.length;

            pInfo->pPtz_configuration[i].default_pan_speed                  = (int16_t)_GET_WORD(pCur); pCur += 2;
            pInfo->pPtz_configuration[i].default_tilt_speed                 = (int16_t)_GET_WORD(pCur); pCur += 2;
            pInfo->pPtz_configuration[i].default_zoom_speed                 = (int16_t)_GET_WORD(pCur); pCur += 2;
            pInfo->pPtz_configuration[i].default_timeout                    = _GET_DWORD(pCur); pCur += 4;
            pInfo->pPtz_configuration[i].pan_limit_min                      = _GET_WORD(pCur);  pCur += 2;
            pInfo->pPtz_configuration[i].pan_limit_max                      = _GET_WORD(pCur);  pCur += 2;
            pInfo->pPtz_configuration[i].tilt_limit_min                     = _GET_WORD(pCur);  pCur += 2;
            pInfo->pPtz_configuration[i].tilt_limit_max                     = _GET_WORD(pCur);  pCur += 2;
            pInfo->pPtz_configuration[i].zoom_limit_min                     = _GET_WORD(pCur);  pCur += 2;
            pInfo->pPtz_configuration[i].zoom_limit_max                     = _GET_WORD(pCur);  pCur += 2;
            pInfo->pPtz_configuration[i].ptz_control_direction_eflip_mode   = _GET_BYTE(pCur);  pCur += 1;
            pInfo->pPtz_configuration[i].ptz_control_direction_reverse_mode = _GET_BYTE(pCur);  pCur += 1;
        }
    }while(0);
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetConfigurationsOutput_Cmd_Pkt_Decode, GetConfigurationsOutput_INFO, _GetConfigurationsOutput_dec);

static uint32_t
_GetConfigurationsOutput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            GetConfigurationsOutput_INFO   *pInfo = (GetConfigurationsOutput_INFO*)(*((uint8_t**)input_info));
            if( pInfo->pPtz_configuration )     free(pInfo->pPtz_configuration);
        }

        free((*((uint8_t**)input_info)));
        (*((uint8_t**)input_info)) = 0;
    }while(0);

    return 0;
}

//////////////////
static uint32_t
PTZ_GetStatusInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code:0x0501
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Token               n           [string] A reference to the PTZ configuration
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    PTZ_GetStatusInput_INFO  *pInfo = (PTZ_GetStatusInput_INFO*)input_info;
    uint32_t                 cmd_length = 0;
    uint8_t                  *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->token.length);

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
        _SET_WORD(pCur, CMD_PTZ_GetStatusInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);
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
_PTZ_GetStatusInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code:0x0501
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Token               n           [string] A reference to the PTZ configuration
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    PTZ_GetStatusInput_INFO   *pInfo = (PTZ_GetStatusInput_INFO*)pStruct_Info;

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
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(PTZ_GetStatusInput_Cmd_Pkt_Decode, PTZ_GetStatusInput_INFO, _PTZ_GetStatusInput_dec);

//////////////////
static uint32_t
PTZ_GetStatusOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code: 0x8501
     * Return Code         1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * PanPosition         2           [short] Pan position.
     * TiltPosition        2           [short] Tilt position.
     * ZoomPosition        2           [short] Zoom position.
     * PanTilt MoveStatus  1           0: IDLE, 1: MOVING, 2: UNKNOWN.
     * Zoom MoveStatus     1           0: IDLE, 1: MOVING, 2: UNKNOWN.
     * Error               n           [string] States a current PTZ error.
     * UTC Hour            1           Range is 0 to 23.
     * UTC Minute          1           Range is 0 to 59.
     * UTC Second          1           Range is 0 to 61 (typically 59).
     * UTC Year            2
     * UTC Month           1           Range is 1 to 12.
     * UTC Day             1           Range is 1 to 31.
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    PTZ_GetStatusOutput_INFO  *pInfo = (PTZ_GetStatusOutput_INFO*)input_info;
    uint32_t                  cmd_length = 0;
    uint8_t                   *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 23 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->error_msg.length);

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
        _SET_WORD(pCur, CMD_PTZ_GetStatusOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_WORD(pCur, (uint16_t)pInfo->pan_position);
        _SET_WORD(pCur, (uint16_t)pInfo->tilt_position);
        _SET_WORD(pCur, (uint16_t)pInfo->zoom_position);
        _SET_BYTE(pCur, pInfo->pan_tilt_move_status);
        _SET_BYTE(pCur, pInfo->zoom_move_status);

        _SET_DWORD(pCur, pInfo->error_msg.length);
        _SET_STRING(pCur, pInfo->error_msg.pStream, pInfo->error_msg.length);

        _SET_BYTE(pCur, pInfo->utc_hour);
        _SET_BYTE(pCur, pInfo->utc_minute);
        _SET_BYTE(pCur, pInfo->utc_second);
        _SET_WORD(pCur, pInfo->utc_year);
        _SET_BYTE(pCur, pInfo->utc_month);
        _SET_BYTE(pCur, pInfo->utc_day);
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
_PTZ_GetStatusOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code: 0x8501
     * Return Code         1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * PanPosition         2           [short] Pan position.
     * TiltPosition        2           [short] Tilt position.
     * ZoomPosition        2           [short] Zoom position.
     * PanTilt MoveStatus  1           0: IDLE, 1: MOVING, 2: UNKNOWN.
     * Zoom MoveStatus     1           0: IDLE, 1: MOVING, 2: UNKNOWN.
     * Error               n           [string] States a current PTZ error.
     * UTC Hour            1           Range is 0 to 23.
     * UTC Minute          1           Range is 0 to 59.
     * UTC Second          1           Range is 0 to 61 (typically 59).
     * UTC Year            2
     * UTC Month           1           Range is 1 to 12.
     * UTC Day             1           Range is 1 to 31.
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    PTZ_GetStatusOutput_INFO   *pInfo = (PTZ_GetStatusOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->pan_position  = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->tilt_position = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->zoom_position = (int16_t)_GET_WORD(pCur);   pCur += 2;

    pInfo->pan_tilt_move_status = _GET_BYTE(pCur);   pCur += 1;
    pInfo->zoom_move_status     = _GET_BYTE(pCur);   pCur += 1;

    pInfo->error_msg.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->error_msg.pStream = pCur;              pCur += pInfo->error_msg.length;

    pInfo->utc_hour     = _GET_BYTE(pCur);   pCur += 1;
    pInfo->utc_minute   = _GET_BYTE(pCur);   pCur += 1;
    pInfo->utc_second   = _GET_BYTE(pCur);   pCur += 1;
    pInfo->utc_year     = _GET_WORD(pCur);   pCur += 2;
    pInfo->utc_month    = _GET_BYTE(pCur);   pCur += 1;
    pInfo->utc_day      = _GET_BYTE(pCur);   pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(PTZ_GetStatusOutput_Cmd_Pkt_Decode, PTZ_GetStatusOutput_INFO, _PTZ_GetStatusOutput_dec);

//////////////////
static uint32_t
GetPresetsInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code:0x0502
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Token               n           [string] A reference to the PTZ configuration
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    GetPresetsInput_INFO  *pInfo = (GetPresetsInput_INFO*)input_info;
    uint32_t              cmd_length = 0;
    uint8_t               *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->token.length);

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
        _SET_WORD(pCur, CMD_GetPresetsInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);
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
_GetPresetsInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code:0x0502
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Token               n           [string] A reference to the PTZ configuration
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    GetPresetsInput_INFO   *pInfo = (GetPresetsInput_INFO*)pStruct_Info;

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
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetPresetsInput_Cmd_Pkt_Decode, GetPresetsInput_INFO, _GetPresetsInput_dec);

//////////////////
static uint32_t
GetPresetsOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code: 0x8502
     * Return Code         1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Preset List Size    1           The list size of presets which are available for the requested MediaProfile. The following 5 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All GetPreset Parameters }.
     *   Name                n           [string] preset name.
     *   Token               n           [string] preset token
     *   PanPosition         2           [short] Pan position.
     *   TiltPosition        2           [short] Tilt position.
     *   ZoomPosition        2           [short] Zoom position.
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    GetPresetsOutput_INFO  *pInfo = (GetPresetsOutput_INFO*)input_info;
    uint32_t               cmd_length = 0;
    uint8_t                *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;
        uint32_t    i = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length);
        for(i = 0; i < pInfo->preset_list_size; i++)
            cmd_length += (6 + (4+pInfo->pPtz_preset_data[i].name.length) + (4+pInfo->pPtz_preset_data[i].token.length));

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
        _SET_WORD(pCur, CMD_GetPresetsOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->preset_list_size);

        for(i = 0; i < pInfo->preset_list_size; i++)
        {
            _SET_DWORD(pCur, pInfo->pPtz_preset_data[i].name.length);
            _SET_STRING(pCur, pInfo->pPtz_preset_data[i].name.pStream, pInfo->pPtz_preset_data[i].name.length);

            _SET_DWORD(pCur, pInfo->pPtz_preset_data[i].token.length);
            _SET_STRING(pCur, pInfo->pPtz_preset_data[i].token.pStream, pInfo->pPtz_preset_data[i].token.length);

            _SET_WORD(pCur, (uint16_t)pInfo->pPtz_preset_data[i].pan_position);
            _SET_WORD(pCur, (uint16_t)pInfo->pPtz_preset_data[i].tilt_position);
            _SET_WORD(pCur, (uint16_t)pInfo->pPtz_preset_data[i].zoom_position);
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
_GetPresetsOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code: 0x8502
     * Return Code         1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Preset List Size    1           The list size of presets which are available for the requested MediaProfile. The following 5 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All GetPreset Parameters }.
     *   Name                n           [string] preset name.
     *   Token               n           [string] preset token
     *   PanPosition         2           [short] Pan position.
     *   TiltPosition        2           [short] Tilt position.
     *   ZoomPosition        2           [short] Zoom position.
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    GetPresetsOutput_INFO   *pInfo = (GetPresetsOutput_INFO*)pStruct_Info;

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

        pInfo->preset_list_size = _GET_BYTE(pCur);  pCur += 1;

        pInfo->pPtz_preset_data = tscm_malloc(sizeof(PTZ_PRESET_DATA) * pInfo->preset_list_size);
        if( !pInfo->pPtz_preset_data )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }
        memset(pInfo->pPtz_preset_data, 0x0, sizeof(PTZ_PRESET_DATA) * pInfo->preset_list_size);

        for(i = 0; i < pInfo->preset_list_size; i++)
        {
            pInfo->pPtz_preset_data[i].name.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pPtz_preset_data[i].name.pStream = pCur;              pCur += pInfo->pPtz_preset_data[i].name.length;

            pInfo->pPtz_preset_data[i].token.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pPtz_preset_data[i].token.pStream = pCur;              pCur += pInfo->pPtz_preset_data[i].token.length;

            pInfo->pPtz_preset_data[i].pan_position  = (int16_t)_GET_WORD(pCur);   pCur += 2;
            pInfo->pPtz_preset_data[i].tilt_position = (int16_t)_GET_WORD(pCur);   pCur += 2;
            pInfo->pPtz_preset_data[i].zoom_position = (int16_t)_GET_WORD(pCur);   pCur += 2;
        }
    }while(0);
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetPresetsOutput_Cmd_Pkt_Decode, GetPresetsOutput_INFO, _GetPresetsOutput_dec);

static uint32_t
_GetPresetsOutput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            GetPresetsOutput_INFO   *pInfo = (GetPresetsOutput_INFO*)(*((uint8_t**)input_info));
            if( pInfo->pPtz_preset_data )     free(pInfo->pPtz_preset_data);
        }

        free((*((uint8_t**)input_info)));
        (*((uint8_t**)input_info)) = 0;
    }while(0);

    return 0;
}

//////////////////
static uint32_t
GotoPresetInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field        Length(Byte)   Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code:0x0580
     * Reserved        1
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * Token           n           [string] A reference to the PTZ configuration
     * PresetToken     n           [string] A requested preset token.
     * PanSpeed        2           [short] Pan speed. 0 means use default
     * TiltSpeed       2           [short] Tilt speed. 0 means use default
     * ZoomSpeed       2           [short] Zoom speed. 0 means use default
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
    GotoPresetInput_INFO  *pInfo = (GotoPresetInput_INFO*)input_info;
    uint32_t              cmd_length = 0;
    uint8_t               *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 14 + (4+pInfo->user_name.length)
                        + (4+pInfo->password.length)
                        + (4+pInfo->token.length)
                        + (4+pInfo->preset_token.length);

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
        _SET_WORD(pCur, CMD_GotoPresetInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);

        _SET_DWORD(pCur, pInfo->preset_token.length);
        _SET_STRING(pCur, pInfo->preset_token.pStream, pInfo->preset_token.length);

        _SET_WORD(pCur, (uint16_t)pInfo->pan_speed);
        _SET_WORD(pCur, (uint16_t)pInfo->tilt_speed);
        _SET_WORD(pCur, (uint16_t)pInfo->zoom_speed);
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
_GotoPresetInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field        Length(Byte)   Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code:0x0580
     * Reserved        1
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * Token           n           [string] A reference to the PTZ configuration
     * PresetToken     n           [string] A requested preset token.
     * PanSpeed        2           [short] Pan speed. 0 means use default
     * TiltSpeed       2           [short] Tilt speed. 0 means use default
     * ZoomSpeed       2           [short] Zoom speed. 0 means use default
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
    GotoPresetInput_INFO   *pInfo = (GotoPresetInput_INFO*)pStruct_Info;

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

    pInfo->preset_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->preset_token.pStream = pCur;              pCur += pInfo->preset_token.length;

    pInfo->pan_speed  = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->tilt_speed = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->zoom_speed = (int16_t)_GET_WORD(pCur);   pCur += 2;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GotoPresetInput_Cmd_Pkt_Decode, GotoPresetInput_INFO, _GotoPresetInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code: 0x8580
     * Return Code         1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GotoPresetOutput_Cmd_Ctxt_New, GotoPresetOutput_INFO, CMD_GotoPresetOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GotoPresetOutput_dec, GotoPresetOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(GotoPresetOutput_Cmd_Pkt_Decode, GotoPresetOutput_INFO, _GotoPresetOutput_dec);

//////////////////
static uint32_t
RemovePresetInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length      4        The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2        Code:0x0581
     * Reserved            1
     * User Name           n        [string] User name for security.
     * Password            n        [string] Password for security.
     * Token               n        [string] A reference to the PTZ configuration
     * PresetToken         n        [string] A requested preset token.
     * CheckSum            1        =(byte[1]+...+byte[N]) MOD 256
     **/
    RemovePresetInput_INFO  *pInfo = (RemovePresetInput_INFO*)input_info;
    uint32_t                             cmd_length = 0;
    uint8_t                              *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length)
                       + (4+pInfo->password.length)
                       + (4+pInfo->token.length)
                       + (4+pInfo->preset_token.length);

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
        _SET_WORD(pCur, CMD_RemovePresetInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);

        _SET_DWORD(pCur, pInfo->preset_token.length);
        _SET_STRING(pCur, pInfo->preset_token.pStream, pInfo->preset_token.length);
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
_RemovePresetInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length      4        The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2        Code:0x0581
     * Reserved            1
     * User Name           n        [string] User name for security.
     * Password            n        [string] Password for security.
     * Token               n        [string] A reference to the PTZ configuration
     * PresetToken         n        [string] A requested preset token.
     * CheckSum            1        =(byte[1]+...+byte[N]) MOD 256
     **/
    RemovePresetInput_INFO   *pInfo = (RemovePresetInput_INFO*)pStruct_Info;

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

    pInfo->preset_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->preset_token.pStream = pCur;              pCur += pInfo->preset_token.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(RemovePresetInput_Cmd_Pkt_Decode, RemovePresetInput_INFO, _RemovePresetInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code: 0x8581
     * Return Code         1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(RemovePresetOutput_Cmd_Ctxt_New, RemovePresetOutput_INFO, CMD_RemovePresetOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_RemovePresetOutput_dec, RemovePresetOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(RemovePresetOutput_Cmd_Pkt_Decode, RemovePresetOutput_INFO, _RemovePresetOutput_dec);

//////////////////
static uint32_t
SetPresetInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code:0x0582
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Token               n           [string] A reference to the PTZ configuration
     * PresetName          n           [string] A requested preset name.
     * PresetToken         n           [string] A requested preset token.
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    SetPresetInput_INFO  *pInfo = (SetPresetInput_INFO*)input_info;
    uint32_t                             cmd_length = 0;
    uint8_t                              *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length)
                       + (4+pInfo->password.length)
                       + (4+pInfo->token.length)
                       + (4+pInfo->preset_name.length)
                       + (4+pInfo->preset_token.length);

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
        _SET_WORD(pCur, CMD_SetPresetInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);

        _SET_DWORD(pCur, pInfo->preset_name.length);
        _SET_STRING(pCur, pInfo->preset_name.pStream, pInfo->preset_name.length);

        _SET_DWORD(pCur, pInfo->preset_token.length);
        _SET_STRING(pCur, pInfo->preset_token.pStream, pInfo->preset_token.length);
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
_SetPresetInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2           Code:0x0582
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Token               n           [string] A reference to the PTZ configuration
     * PresetName          n           [string] A requested preset name.
     * PresetToken         n           [string] A requested preset token.
     * CheckSum            1           =(byte[1]+...+byte[N]) MOD 256
     **/
    SetPresetInput_INFO   *pInfo = (SetPresetInput_INFO*)pStruct_Info;

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

    pInfo->preset_name.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->preset_name.pStream = pCur;              pCur += pInfo->preset_name.length;

    pInfo->preset_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->preset_token.pStream = pCur;              pCur += pInfo->preset_token.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetPresetInput_Cmd_Pkt_Decode, SetPresetInput_INFO, _SetPresetInput_dec);

//////////////////
static uint32_t
SetPresetOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code: 0x8582
     * Return Code     1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * PresetToken     n           [string] A token to the Preset which has been set.
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
    SetPresetOutput_INFO  *pInfo = (SetPresetOutput_INFO*)input_info;
    uint32_t                              cmd_length = 0;
    uint8_t                               *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->preset_token.length);

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
        _SET_WORD(pCur, CMD_SetPresetOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->preset_token.length);
        _SET_STRING(pCur, pInfo->preset_token.pStream, pInfo->preset_token.length);
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
_SetPresetOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code: 0x8582
     * Return Code     1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * PresetToken     n           [string] A token to the Preset which has been set.
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
    SetPresetOutput_INFO   *pInfo = (SetPresetOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->preset_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->preset_token.pStream = pCur;              pCur += pInfo->preset_token.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetPresetOutput_Cmd_Pkt_Decode, SetPresetOutput_INFO, _SetPresetOutput_dec);

//////////////////
static uint32_t
AbsoluteMoveInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code:0x0583
     * Reserved        1
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * Token           n           [string] A reference to the PTZ Configuration.
     * Pan Position    2           [short] Pan position.
     * Tilt Position   2           [short] Tilt position.
     * Zoom Position   2           [short] A zoom position.
     * Pan Speed       2           [short] Pan speed.
     * Tilt Speed      2           [short] Tilt speed.
     * Zoom Speed      2           [short] Zoom speed.
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
    AbsoluteMoveInput_INFO  *pInfo = (AbsoluteMoveInput_INFO*)input_info;
    uint32_t                cmd_length = 0;
    uint8_t                 *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 20 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->token.length);

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
        _SET_WORD(pCur, CMD_AbsoluteMoveInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);

        _SET_WORD(pCur, (uint16_t)pInfo->pan_position);
        _SET_WORD(pCur, (uint16_t)pInfo->tilt_position);
        _SET_WORD(pCur, (uint16_t)pInfo->zoom_position);
        _SET_WORD(pCur, (uint16_t)pInfo->pan_speed);
        _SET_WORD(pCur, (uint16_t)pInfo->tilt_speed);
        _SET_WORD(pCur, (uint16_t)pInfo->zoom_speed);
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
_AbsoluteMoveInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code:0x0583
     * Reserved        1
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * Token           n           [string] A reference to the PTZ Configuration.
     * Pan Position    2           [short] Pan position.
     * Tilt Position   2           [short] Tilt position.
     * Zoom Position   2           [short] A zoom position.
     * Pan Speed       2           [short] Pan speed.
     * Tilt Speed      2           [short] Tilt speed.
     * Zoom Speed      2           [short] Zoom speed.
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
    AbsoluteMoveInput_INFO   *pInfo = (AbsoluteMoveInput_INFO*)pStruct_Info;

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

    pInfo->pan_position  = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->tilt_position = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->zoom_position = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->pan_speed     = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->tilt_speed    = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->zoom_speed    = (int16_t)_GET_WORD(pCur);   pCur += 2;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(AbsoluteMoveInput_Cmd_Pkt_Decode, AbsoluteMoveInput_INFO, _AbsoluteMoveInput_dec);

//////////////////
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code: 0x8583
     * Return Code     1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(AbsoluteMoveOutput_Cmd_Ctxt_New, AbsoluteMoveOutput_INFO, CMD_AbsoluteMoveOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_AbsoluteMoveOutput_dec, AbsoluteMoveOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(AbsoluteMoveOutput_Cmd_Pkt_Decode, AbsoluteMoveOutput_INFO, _AbsoluteMoveOutput_dec);

//////////////////
static uint32_t
RelativeMoveInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length      4       The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2       Code:0x0584
     * Reserved            1
     * User Name           n       [string] User name for security.
     * Password            n       [string] Password for security.
     * Token               n       [string] A reference to the PTZ configuration.
     * PanTranslation      2       [short] Pan translation.
     * TiltTranslation     2       [short] Tilt translation.
     * ZoomTranslation     2       [short] Zoom translation.
     * PanSpeed            2       [short] Pan speed. 0 means use default
     * TiltSpeed           2       [short] Tilt speed. 0 means use default
     * ZoomSpeed           2       [short] Zoom speed. 0 means use default
     * CheckSum            1       =(byte[1]+...+byte[N]) MOD 256
     **/
    RelativeMoveInput_INFO  *pInfo = (RelativeMoveInput_INFO*)input_info;
    uint32_t                cmd_length = 0;
    uint8_t                 *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 20 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->token.length);

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
        _SET_WORD(pCur, CMD_RelativeMoveInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);


        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);

        _SET_WORD(pCur, (uint16_t)pInfo->pan_translation);
        _SET_WORD(pCur, (uint16_t)pInfo->tilt_translation);
        _SET_WORD(pCur, (uint16_t)pInfo->zoom_translation);
        _SET_WORD(pCur, (uint16_t)pInfo->pan_speed);
        _SET_WORD(pCur, (uint16_t)pInfo->tilt_speed);
        _SET_WORD(pCur, (uint16_t)pInfo->zoom_speed);
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
_RelativeMoveInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length      4       The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code        2       Code:0x0584
     * Reserved            1
     * User Name           n       [string] User name for security.
     * Password            n       [string] Password for security.
     * Token               n       [string] A reference to the PTZ configuration.
     * PanTranslation      2       [short] Pan translation.
     * TiltTranslation     2       [short] Tilt translation.
     * ZoomTranslation     2       [short] Zoom translation.
     * PanSpeed            2       [short] Pan speed. 0 means use default
     * TiltSpeed           2       [short] Tilt speed. 0 means use default
     * ZoomSpeed           2       [short] Zoom speed. 0 means use default
     * CheckSum            1       =(byte[1]+...+byte[N]) MOD 256
     **/
    RelativeMoveInput_INFO   *pInfo = (RelativeMoveInput_INFO*)pStruct_Info;

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

    pInfo->pan_translation  = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->tilt_translation = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->zoom_translation = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->pan_speed        = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->tilt_speed       = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->zoom_speed       = (int16_t)_GET_WORD(pCur);   pCur += 2;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(RelativeMoveInput_Cmd_Pkt_Decode, RelativeMoveInput_INFO, _RelativeMoveInput_dec);

//////////////////
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code: 0x8584
     * Return Code     1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(RelativeMoveOutput_Cmd_Ctxt_New, RelativeMoveOutput_INFO, CMD_RelativeMoveOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_RelativeMoveOutput_dec, RelativeMoveOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(RelativeMoveOutput_Cmd_Pkt_Decode, RelativeMoveOutput_INFO, _RelativeMoveOutput_dec);

//////////////////
static uint32_t
ContinuousMoveInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field       Length(Byte)  Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code:0x0585
     * Reserved        1
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * Token           n           [string] A reference to the PTZ Configuration.
     * Pan Velocity    2           [short] Pan speed.
     * Tilt Velocity   2           [short] Tilt speed.
     * Zoom Velocity   2           [short] Zoom speed.
     * Timeout         4           An optional Timeout parameter (us)
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
    ContinuousMoveInput_INFO  *pInfo = (ContinuousMoveInput_INFO*)input_info;
    uint32_t                  cmd_length = 0;
    uint8_t                   *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 18 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->token.length);

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
        _SET_WORD(pCur, CMD_ContinuousMoveInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);

        _SET_WORD(pCur, (uint16_t)pInfo->pan_velocity);
        _SET_WORD(pCur, (uint16_t)pInfo->tilt_velocity);
        _SET_WORD(pCur, (uint16_t)pInfo->zoom_velocity);
        _SET_DWORD(pCur, pInfo->timeout);
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
_ContinuousMoveInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field       Length(Byte)  Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code:0x0585
     * Reserved        1
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * Token           n           [string] A reference to the PTZ Configuration.
     * Pan Velocity    2           [short] Pan speed.
     * Tilt Velocity   2           [short] Tilt speed.
     * Zoom Velocity   2           [short] Zoom speed.
     * Timeout         4           An optional Timeout parameter (us)
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
    ContinuousMoveInput_INFO   *pInfo = (ContinuousMoveInput_INFO*)pStruct_Info;

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

    pInfo->pan_velocity  = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->tilt_velocity = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->zoom_velocity = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->timeout       = _GET_DWORD(pCur); pCur += 4;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(ContinuousMoveInput_Cmd_Pkt_Decode, ContinuousMoveInput_INFO, _ContinuousMoveInput_dec);

//////////////////
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code: 0x8585
     * Return Code     1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(ContinuousMoveOutput_Cmd_Ctxt_New, ContinuousMoveOutput_INFO, CMD_ContinuousMoveOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_ContinuousMoveOutput_dec, ContinuousMoveOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(ContinuousMoveOutput_Cmd_Pkt_Decode, ContinuousMoveOutput_INFO, _ContinuousMoveOutput_dec);

//////////////////
static uint32_t
SetHomePositionInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code:0x0586
     * Reserved        1
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * Token           n           [string] A reference to the PTZ configuration
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
    SetHomePositionInput_INFO  *pInfo = (SetHomePositionInput_INFO*)input_info;
    uint32_t                             cmd_length = 0;
    uint8_t                              *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->token.length);

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
        _SET_WORD(pCur, CMD_SetHomePositionInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);
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
_SetHomePositionInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code:0x0586
     * Reserved        1
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * Token           n           [string] A reference to the PTZ configuration
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
    SetHomePositionInput_INFO   *pInfo = (SetHomePositionInput_INFO*)pStruct_Info;

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
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetHomePositionInput_Cmd_Pkt_Decode, SetHomePositionInput_INFO, _SetHomePositionInput_dec);

//////////////////
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code: 0x8586
     * Return Code     1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(SetHomePositionOutput_Cmd_Ctxt_New, SetHomePositionOutput_INFO, CMD_SetHomePositionOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_SetHomePositionOutput_dec, SetHomePositionOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(SetHomePositionOutput_Cmd_Pkt_Decode, SetHomePositionOutput_INFO, _SetHomePositionOutput_dec);

//////////////////
static uint32_t
GotoHomePositionInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code:0x0587
     * Reserved        1
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * Token           n           [string] A reference to the PTZ configuration
     * PanSpeed        2           [short] Pan speed. 0 means use default
     * TiltSpeed       2           [short] Tilt speed. 0 means use default
     * ZoomSpeed       2           [short] Zoom speed. 0 means use default
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
    GotoHomePositionInput_INFO  *pInfo = (GotoHomePositionInput_INFO*)input_info;
    uint32_t                    cmd_length = 0;
    uint8_t                     *pCmd_ctxt_Buf = 0;

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
        _SET_WORD(pCur, CMD_GotoHomePositionInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);

        _SET_WORD(pCur, (uint16_t)pInfo->pan_speed);
        _SET_WORD(pCur, (uint16_t)pInfo->tilt_speed);
        _SET_WORD(pCur, (uint16_t)pInfo->zoom_speed);
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
_GotoHomePositionInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code:0x0587
     * Reserved        1
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * Token           n           [string] A reference to the PTZ configuration
     * PanSpeed        2           [short] Pan speed. 0 means use default
     * TiltSpeed       2           [short] Tilt speed. 0 means use default
     * ZoomSpeed       2           [short] Zoom speed. 0 means use default
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
    GotoHomePositionInput_INFO   *pInfo = (GotoHomePositionInput_INFO*)pStruct_Info;

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

    pInfo->pan_speed  = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->tilt_speed = (int16_t)_GET_WORD(pCur);   pCur += 2;
    pInfo->zoom_speed = (int16_t)_GET_WORD(pCur);   pCur += 2;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GotoHomePositionInput_Cmd_Pkt_Decode, GotoHomePositionInput_INFO, _GotoHomePositionInput_dec);

//////////////////
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code: 0x8587
     * Return Code     1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GotoHomePositionOutput_Cmd_Ctxt_New, GotoHomePositionOutput_INFO, CMD_GotoHomePositionOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GotoHomePositionOutput_dec, GotoHomePositionOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(GotoHomePositionOutput_Cmd_Pkt_Decode, GotoHomePositionOutput_INFO, _GotoHomePositionOutput_dec);

//////////////////
static uint32_t
PTZ_StopInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4            The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2            Code:0x0588
     * Reserved        1
     * User Name       n            [string] User name for security.
     * Password        n            [string] Password for security.
     * Token           n            [string] A reference to the PTZ configuration
     * PanTiltZoom     1            0: False - start, 1: True - stop.
     * CheckSum        1            =(byte[1]+...+byte[N]) MOD 256
     **/
    PTZ_StopInput_INFO  *pInfo = (PTZ_StopInput_INFO*)input_info;
    uint32_t            cmd_length = 0;
    uint8_t             *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->token.length);

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
        _SET_WORD(pCur, CMD_PTZ_StopInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);

        _SET_BYTE(pCur, pInfo->pan_tilt_zoom);
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
_PTZ_StopInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4            The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2            Code:0x0588
     * Reserved        1
     * User Name       n            [string] User name for security.
     * Password        n            [string] Password for security.
     * Token           n            [string] A reference to the PTZ configuration
     * PanTiltZoom     1            0: False - start, 1: True - stop.
     * CheckSum        1            =(byte[1]+...+byte[N]) MOD 256
     **/
    PTZ_StopInput_INFO   *pInfo = (PTZ_StopInput_INFO*)pStruct_Info;

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

    pInfo->pan_tilt_zoom = _GET_BYTE(pCur);      pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(PTZ_StopInput_Cmd_Pkt_Decode, PTZ_StopInput_INFO, _PTZ_StopInput_dec);

//////////////////
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It is not include the Leading Tag, CheckSum and End Tag.
     * Command Code    2           Code: 0x8588
     * Return Code     1           0:Success, 0xFD:unsupported 0xFE:Cheksum Err, 0xFF:Fail,
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * CheckSum        1           =(byte[1]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(PTZ_StopOutput_Cmd_Ctxt_New, PTZ_StopOutput_INFO, CMD_PTZ_StopOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_PTZ_StopOutput_dec, PTZ_StopOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(PTZ_StopOutput_Cmd_Pkt_Decode, PTZ_StopOutput_INFO, _PTZ_StopOutput_dec);

//=============================================================================
//                Public Function Definition
//=============================================================================

// DEFINE_CMD_PKT_CODEC(CMD_xxxInput, xxxInput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
// DEFINE_CMD_PKT_CODEC(CMD_xxxOutput, xxxOutput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetConfigurationsInput, GetConfigurationsInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetConfigurationsInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetConfigurationsOutput, GetConfigurationsOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetConfigurationsOutput_Cmd_Pkt_Decode, _GetConfigurationsOutput_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_PTZ_GetStatusInput, PTZ_GetStatusInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, PTZ_GetStatusInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_PTZ_GetStatusOutput, PTZ_GetStatusOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, PTZ_GetStatusOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetPresetsInput, GetPresetsInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetPresetsInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetPresetsOutput, GetPresetsOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetPresetsOutput_Cmd_Pkt_Decode, _GetPresetsOutput_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GotoPresetInput, GotoPresetInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GotoPresetInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GotoPresetOutput, GotoPresetOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GotoPresetOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_RemovePresetInput, RemovePresetInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, RemovePresetInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_RemovePresetOutput, RemovePresetOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, RemovePresetOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetPresetInput, SetPresetInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetPresetInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetPresetOutput, SetPresetOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetPresetOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_AbsoluteMoveInput, AbsoluteMoveInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, AbsoluteMoveInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_AbsoluteMoveOutput, AbsoluteMoveOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, AbsoluteMoveOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_RelativeMoveInput, RelativeMoveInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, RelativeMoveInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_RelativeMoveOutput, RelativeMoveOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, RelativeMoveOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_ContinuousMoveInput, ContinuousMoveInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, ContinuousMoveInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_ContinuousMoveOutput, ContinuousMoveOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, ContinuousMoveOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetHomePositionInput, SetHomePositionInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetHomePositionInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetHomePositionOutput, SetHomePositionOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetHomePositionOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GotoHomePositionInput, GotoHomePositionInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GotoHomePositionInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GotoHomePositionOutput, GotoHomePositionOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GotoHomePositionOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_PTZ_StopInput, PTZ_StopInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, PTZ_StopInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_PTZ_StopOutput, PTZ_StopOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, PTZ_StopOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
