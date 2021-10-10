

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
     * Command Code        2           Code: 0x0100
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetCapabilitiesInput_Cmd_Ctxt_New, GetCapabilitiesInput_INFO, CMD_GetCapabilitiesInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetCapabilitiesInput_dec, GetCapabilitiesInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetCapabilitiesInput_Cmd_Pkt_Decode, GetCapabilitiesInput_INFO, _GetCapabilitiesInput_dec);

//////////////////
static uint32_t
GetCapabilitiesOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8100
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Supported Features  4           0: not supported, 1:Supported
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetCapabilitiesOutput_INFO  *pInfo = (GetCapabilitiesOutput_INFO*)input_info;
    uint32_t                    cmd_length = 0;
    uint8_t                     *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 12 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetCapabilitiesOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->supported_features);
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
_GetCapabilitiesOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8100
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Supported Features  4           0: not supported, 1:Supported
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetCapabilitiesOutput_INFO   *pInfo = (GetCapabilitiesOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->supported_features = _GET_DWORD(pCur);  pCur += 4;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetCapabilitiesOutput_Cmd_Pkt_Decode, GetCapabilitiesOutput_INFO, _GetCapabilitiesOutput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0101
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetDeviceInformationInput_Cmd_Ctxt_New, GetDeviceInformationInput_INFO, CMD_GetDeviceInformationInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetDeviceInformationInput_dec, GetDeviceInformationInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetDeviceInformationInput_Cmd_Pkt_Decode, GetDeviceInformationInput_INFO, _GetDeviceInformationInput_dec);

//////////////////
static uint32_t
GetDeviceInformationOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8101
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Manufacturer        n           [string] The manufactor of the device.
     * Model               n           [string] The device model.
     * FirmwareVersion     n           [string] The firmware version in the device.
     * SerialNumber        n           [string] The serial number of the device.
     * HardwareId          n           [string]The hardware ID of the device.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetDeviceInformationOutput_INFO  *pInfo = (GetDeviceInformationOutput_INFO*)input_info;
    uint32_t                         cmd_length = 0;
    uint8_t                          *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length)
                       + (4+pInfo->manufacturer.length) + (4+pInfo->model.length)
                       + (4+pInfo->firmware_version.length) + (4+pInfo->serial_number.length)
                       + (4+pInfo->hardware_id.length);

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
        _SET_WORD(pCur, CMD_GetDeviceInformationOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->manufacturer.length);
        _SET_STRING(pCur, pInfo->manufacturer.pStream, pInfo->manufacturer.length);

        _SET_DWORD(pCur, pInfo->model.length);
        _SET_STRING(pCur, pInfo->model.pStream, pInfo->model.length);

        _SET_DWORD(pCur, pInfo->firmware_version.length);
        _SET_STRING(pCur, pInfo->firmware_version.pStream, pInfo->firmware_version.length);

        _SET_DWORD(pCur, pInfo->serial_number.length);
        _SET_STRING(pCur, pInfo->serial_number.pStream, pInfo->serial_number.length);

        _SET_DWORD(pCur, pInfo->hardware_id.length);
        _SET_STRING(pCur, pInfo->hardware_id.pStream, pInfo->hardware_id.length);
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
_GetDeviceInformationOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8101
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Manufacturer        n           [string] The manufactor of the device.
     * Model               n           [string] The device model.
     * FirmwareVersion     n           [string] The firmware version in the device.
     * SerialNumber        n           [string] The serial number of the device.
     * HardwareId          n           [string]The hardware ID of the device.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetDeviceInformationOutput_INFO   *pInfo = (GetDeviceInformationOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->manufacturer.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->manufacturer.pStream = pCur;              pCur += pInfo->manufacturer.length;

    pInfo->model.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->model.pStream = pCur;              pCur += pInfo->model.length;

    pInfo->firmware_version.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->firmware_version.pStream = pCur;              pCur += pInfo->firmware_version.length;

    pInfo->serial_number.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->serial_number.pStream = pCur;              pCur += pInfo->serial_number.length;

    pInfo->hardware_id.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->hardware_id.pStream = pCur;              pCur += pInfo->hardware_id.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetDeviceInformationOutput_Cmd_Pkt_Decode, GetDeviceInformationOutput_INFO, _GetDeviceInformationOutput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0102
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetHostnameInput_Cmd_Ctxt_New, GetHostnameInput_INFO, CMD_GetHostnameInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetHostnameInput_dec, GetHostnameInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetHostnameInput_Cmd_Pkt_Decode, GetHostnameInput_INFO, _GetHostnameInput_dec);

//////////////////
static uint32_t
GetHostnameOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8102
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] Indicates the hostname.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetHostnameOutput_INFO  *pInfo = (GetHostnameOutput_INFO*)input_info;
    uint32_t                cmd_length = 0;
    uint8_t                 *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->host_name.length);

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
        _SET_WORD(pCur, CMD_GetHostnameOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->host_name.length);
        _SET_STRING(pCur, pInfo->host_name.pStream, pInfo->host_name.length);
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
_GetHostnameOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8102
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] Indicates the hostname.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetHostnameOutput_INFO   *pInfo = (GetHostnameOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->host_name.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->host_name.pStream = pCur;              pCur += pInfo->host_name.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetHostnameOutput_Cmd_Pkt_Decode, GetHostnameOutput_INFO, _GetHostnameOutput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0103
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetSystemDateAndTimeInput_Cmd_Ctxt_New, GetSystemDateAndTimeInput_INFO, CMD_GetSystemDateAndTimeInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetSystemDateAndTimeInput_dec, GetSystemDateAndTimeInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetSystemDateAndTimeInput_Cmd_Pkt_Decode, GetSystemDateAndTimeInput_INFO, _GetSystemDateAndTimeInput_dec);

//////////////////
static uint32_t
GetSystemDateAndTimeOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8103
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Country Code        3           ISO 639 3-byte Country Code
     * Country Region ID   1
     * DaylightSavings     1           0: Off, 1: On
     * TimeZone            1           +-12 according to UTC. Or 0x80 for negative TimeZone.
     * UTC Hour            1           Range is 0 to 23.
     * UTC Minute          1           Range is 0 to 59.
     * UTC Second          1           Range is 0 to 61 (typically 59).
     * UTC Year            2
     * UTC Month           1           Range is 1 to 12.
     * UTC Day             1           Range is 1 to 31.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetSystemDateAndTimeOutput_INFO  *pInfo = (GetSystemDateAndTimeOutput_INFO*)input_info;
    uint32_t                         cmd_length = 0;
    uint8_t                          *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 21 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetSystemDateAndTimeOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_PAYLOAD(pCur, pInfo->country_code, 3);

        _SET_BYTE(pCur, pInfo->country_region_id);
        _SET_BYTE(pCur, pInfo->daylightsavings);
        _SET_BYTE(pCur, pInfo->timezone);
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
_GetSystemDateAndTimeOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8103
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Country Code        3           ISO 639 3-byte Country Code
     * Country Region ID   1
     * DaylightSavings     1           0: Off, 1: On
     * TimeZone            1           +-12 according to UTC. Or 0x80 for negative TimeZone.
     * UTC Hour            1           Range is 0 to 23.
     * UTC Minute          1           Range is 0 to 59.
     * UTC Second          1           Range is 0 to 61 (typically 59).
     * UTC Year            2
     * UTC Month           1           Range is 1 to 12.
     * UTC Day             1           Range is 1 to 31.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetSystemDateAndTimeOutput_INFO   *pInfo = (GetSystemDateAndTimeOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    memcpy(pInfo->country_code, pCur, 3); pCur += 3;

    pInfo->country_region_id = _GET_BYTE(pCur); pCur += 1;
    pInfo->daylightsavings   = _GET_BYTE(pCur); pCur += 1;
    pInfo->timezone          = _GET_BYTE(pCur); pCur += 1;
    pInfo->utc_hour          = _GET_BYTE(pCur); pCur += 1;
    pInfo->utc_minute        = _GET_BYTE(pCur); pCur += 1;
    pInfo->utc_second        = _GET_BYTE(pCur); pCur += 1;
    pInfo->utc_year          = _GET_WORD(pCur); pCur += 2;
    pInfo->utc_month         = _GET_BYTE(pCur); pCur += 1;
    pInfo->utc_day           = _GET_BYTE(pCur); pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetSystemDateAndTimeOutput_Cmd_Pkt_Decode, GetSystemDateAndTimeOutput_INFO, _GetSystemDateAndTimeOutput_dec);

//////////////////
static uint32_t
GetSystemLogInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0104
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * LogType             1           0: System log, 1: Access log
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetSystemLogInput_INFO  *pInfo = (GetSystemLogInput_INFO*)input_info;
    uint32_t                cmd_length = 0;
    uint8_t                 *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetSystemLogInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->log_type);
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
_GetSystemLogInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0104
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * LogType             1           0: System log, 1: Access log
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetSystemLogInput_INFO   *pInfo = (GetSystemLogInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->log_type = _GET_BYTE(pCur);           pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetSystemLogInput_Cmd_Pkt_Decode, GetSystemLogInput_INFO, _GetSystemLogInput_dec);

//////////////////
static uint32_t
GetSystemLogOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8104
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * SystemLog           n           [string] Data of the system log.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetSystemLogOutput_INFO  *pInfo = (GetSystemLogOutput_INFO*)input_info;
    uint32_t                 cmd_length = 0;
    uint8_t                  *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->system_log.length);

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
        _SET_WORD(pCur, CMD_GetSystemLogOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->system_log.length);
        _SET_STRING(pCur, pInfo->system_log.pStream, pInfo->system_log.length);
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
_GetSystemLogOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8104
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * SystemLog           n           [string] Data of the system log.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetSystemLogOutput_INFO   *pInfo = (GetSystemLogOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->system_log.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->system_log.pStream = pCur;              pCur += pInfo->system_log.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetSystemLogOutput_Cmd_Pkt_Decode, GetSystemLogOutput_INFO, _GetSystemLogOutput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0105
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetOSDInformationInput_Cmd_Ctxt_New, GetOSDInformationInput_INFO, CMD_GetOSDInformationInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetOSDInformationInput_dec, GetOSDInformationInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetOSDInformationInput_Cmd_Pkt_Decode, GetOSDInformationInput_INFO, _GetOSDInformationInput_dec);

//////////////////
static uint32_t
GetOSDInformationOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field                   Length(Byte)    Descriptions
     * Command Length              4           The total length of this command. It doesn't include the CheckSum.
     * Command Code                2           Code: 0x8105
     * Return Code                 1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                   n           [string] User name for security.
     * Password                    n           [string] Password for security.
     * OSD Date Enable             1           0: Off, 1: On
     * OSD Date Position           1           0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Date Format             1           0: D/M/Y, 1: M/D/Y, 2: Y/M/D
     * OSD Time Enable             1           0: Off, 1: On
     * OSD Time Position           1           0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Date Format             1           0: AM/PM, 1: 24hr
     * OSD Logo Enable             1           0: Off, 1: On
     * OSD Logo Position           1           0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Logo option             1           0: Image1, 1: Image2.
     * OSD Detail Info Enable      1           0: Off, 1: On
     * OSD Detail Info Position    1           0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Detail Info option      1           0: Information1, 1: Information2.
     * OSD Text Enable             1           0: Off, 1: On
     * OSD Text Position           1           0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Text                    n           [string] Text string.
     * CheckSum                    1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetOSDInformationOutput_INFO  *pInfo = (GetOSDInformationOutput_INFO*)input_info;
    uint32_t                      cmd_length = 0;
    uint8_t                       *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 22 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->osd_text.length);

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
        _SET_WORD(pCur, CMD_GetOSDInformationOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->osd_date_enable);
        _SET_BYTE(pCur, pInfo->osd_date_position);
        _SET_BYTE(pCur, pInfo->osd_date_format);
        _SET_BYTE(pCur, pInfo->osd_time_enable);
        _SET_BYTE(pCur, pInfo->osd_time_position);
        _SET_BYTE(pCur, pInfo->osd_time_format);
        _SET_BYTE(pCur, pInfo->osd_logo_enable);
        _SET_BYTE(pCur, pInfo->osd_logo_position);
        _SET_BYTE(pCur, pInfo->osd_logo_option);
        _SET_BYTE(pCur, pInfo->osd_detail_info_enable);
        _SET_BYTE(pCur, pInfo->osd_detail_info_position);
        _SET_BYTE(pCur, pInfo->osd_detail_info_option);
        _SET_BYTE(pCur, pInfo->osd_text_enable);
        _SET_BYTE(pCur, pInfo->osd_text_position);

        _SET_DWORD(pCur, pInfo->osd_text.length);
        _SET_STRING(pCur, pInfo->osd_text.pStream, pInfo->osd_text.length);
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
_GetOSDInformationOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field                   Length(Byte)    Descriptions
     * Command Length              4           The total length of this command. It doesn't include the CheckSum.
     * Command Code                2           Code: 0x8105
     * Return Code                 1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                   n           [string] User name for security.
     * Password                    n           [string] Password for security.
     * OSD Date Enable             1           0: Off, 1: On
     * OSD Date Position           1           0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Date Format             1           0: D/M/Y, 1: M/D/Y, 2: Y/M/D
     * OSD Time Enable             1           0: Off, 1: On
     * OSD Time Position           1           0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Date Format             1           0: AM/PM, 1: 24hr
     * OSD Logo Enable             1           0: Off, 1: On
     * OSD Logo Position           1           0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Logo option             1           0: Image1, 1: Image2.
     * OSD Detail Info Enable      1           0: Off, 1: On
     * OSD Detail Info Position    1           0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Detail Info option      1           0: Information1, 1: Information2.
     * OSD Text Enable             1           0: Off, 1: On
     * OSD Text Position           1           0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Text                    n           [string] Text string.
     * CheckSum                    1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetOSDInformationOutput_INFO   *pInfo = (GetOSDInformationOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->osd_date_enable          = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_date_position        = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_date_format          = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_time_enable          = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_time_position        = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_time_format          = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_logo_enable          = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_logo_position        = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_logo_option          = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_detail_info_enable   = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_detail_info_position = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_detail_info_option   = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_text_enable          = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_text_position        = _GET_BYTE(pCur);  pCur += 1;

    pInfo->osd_text.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->osd_text.pStream = pCur;              pCur += pInfo->osd_text.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetOSDInformationOutput_Cmd_Pkt_Decode, GetOSDInformationOutput_INFO, _GetOSDInformationOutput_dec);

//////////////////
static uint32_t
SystemRebootInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0180
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Reboot Type         1           0: Reboot the device
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SystemRebootInput_INFO  *pInfo = (SystemRebootInput_INFO*)input_info;
    uint32_t                cmd_length = 0;
    uint8_t                 *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SystemRebootInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->reboot_type);
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
_SystemRebootInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0180
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Reboot Type         1           0: Reboot the device
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SystemRebootInput_INFO   *pInfo = (SystemRebootInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->reboot_type = _GET_BYTE(pCur);        pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SystemRebootInput_Cmd_Pkt_Decode, SystemRebootInput_INFO, _SystemRebootInput_dec);

//////////////////
static uint32_t
SystemRebootOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8180
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Message             n           [string] The reboot message sent by the device.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SystemRebootOutput_INFO  *pInfo = (SystemRebootOutput_INFO*)input_info;
    uint32_t                 cmd_length = 0;
    uint8_t                  *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->reboot_message.length);

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
        _SET_WORD(pCur, CMD_SystemRebootOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->reboot_message.length);
        _SET_STRING(pCur, pInfo->reboot_message.pStream, pInfo->reboot_message.length);
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
_SystemRebootOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8180
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Message             n           [string] The reboot message sent by the device.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SystemRebootOutput_INFO   *pInfo = (SystemRebootOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->reboot_message.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->reboot_message.pStream = pCur;              pCur += pInfo->reboot_message.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SystemRebootOutput_Cmd_Pkt_Decode, SystemRebootOutput_INFO, _SystemRebootOutput_dec);

//////////////////
static uint32_t
SetSystemFactoryDefaultInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0181
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * FactoryDefault      1           0: Hard factory default
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetSystemFactoryDefaultInput_INFO  *pInfo = (SetSystemFactoryDefaultInput_INFO*)input_info;
    uint32_t                           cmd_length = 0;
    uint8_t                            *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SetSystemFactoryDefaultInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->factory_default);
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
_SetSystemFactoryDefaultInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0181
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * FactoryDefault      1           0: Hard factory default
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetSystemFactoryDefaultInput_INFO   *pInfo = (SetSystemFactoryDefaultInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->factory_default = _GET_BYTE(pCur);    pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetSystemFactoryDefaultInput_Cmd_Pkt_Decode, SetSystemFactoryDefaultInput_INFO, _SetSystemFactoryDefaultInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8181
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(SetSystemFactoryDefaultOutput_Cmd_Ctxt_New, SetSystemFactoryDefaultOutput_INFO, CMD_SetSystemFactoryDefaultOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_SetSystemFactoryDefaultOutput_dec, SetSystemFactoryDefaultOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(SetSystemFactoryDefaultOutput_Cmd_Pkt_Decode, SetSystemFactoryDefaultOutput_INFO, _SetSystemFactoryDefaultOutput_dec);

//////////////////
static uint32_t
SetHostnameInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0182
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] The hostname to set.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetHostnameInput_INFO  *pInfo = (SetHostnameInput_INFO*)input_info;
    uint32_t               cmd_length = 0;
    uint8_t                *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->host_name.length);

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
        _SET_WORD(pCur, CMD_SetHostnameInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->host_name.length);
        _SET_STRING(pCur, pInfo->host_name.pStream, pInfo->host_name.length);
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
_SetHostnameInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0182
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] The hostname to set.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetHostnameInput_INFO   *pInfo = (SetHostnameInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->host_name.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->host_name.pStream = pCur;              pCur += pInfo->host_name.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetHostnameInput_Cmd_Pkt_Decode, SetHostnameInput_INFO, _SetHostnameInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8182
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(SetHostnameOutput_Cmd_Ctxt_New, SetHostnameOutput_INFO, CMD_SetHostnameOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_SetHostnameOutput_dec, SetHostnameOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(SetHostnameOutput_Cmd_Pkt_Decode, SetHostnameOutput_INFO, _SetHostnameOutput_dec);

//////////////////
static uint32_t
SetSystemDateAndTimeInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
    * Field           Length(Byte)    Descriptions
    * Command Length      4           The total length of this command. It doesn't include the CheckSum.
    * Command Code        2           Code: 0x0183
    * Reserved            1
    * User Name           n           [string] User name for security.
    * Password            n           [string] Password for security.
    * Country Code        3           ISO 639 3-byte Country Code
    * Country Region ID   1
    * DaylightSavings     1           0: Off, 1: On
    * TimeZone            1           +-12 according to UTC. Or 0x80 for negative TimeZone.
    * UTC Hour            1           Range is 0 to 23.
    * UTC Minute          1           Range is 0 to 59.
    * UTC Second          1           Range is 0 to 61 (typically 59).
    * UTC Year            2
    * UTC Month           1           Range is 1 to 12.
    * UTC Day             1           Range is 1 to 31.
    * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetSystemDateAndTimeInput_INFO  *pInfo = (SetSystemDateAndTimeInput_INFO*)input_info;
    uint32_t                        cmd_length = 0;
    uint8_t                         *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 21 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SetSystemDateAndTimeInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_PAYLOAD(pCur, pInfo->country_code, 3);

        _SET_BYTE(pCur, pInfo->country_region_id);
        _SET_BYTE(pCur, pInfo->daylightsavings);
        _SET_BYTE(pCur, pInfo->timezone);
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
_SetSystemDateAndTimeInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
    * Field           Length(Byte)    Descriptions
    * Command Length      4           The total length of this command. It doesn't include the CheckSum.
    * Command Code        2           Code: 0x0183
    * Reserved            1
    * User Name           n           [string] User name for security.
    * Password            n           [string] Password for security.
    * Country Code        3           ISO 639 3-byte Country Code
    * Country Region ID   1
    * DaylightSavings     1           0: Off, 1: On
    * TimeZone            1           +-12 according to UTC. Or 0x80 for negative TimeZone.
    * UTC Hour            1           Range is 0 to 23.
    * UTC Minute          1           Range is 0 to 59.
    * UTC Second          1           Range is 0 to 61 (typically 59).
    * UTC Year            2
    * UTC Month           1           Range is 1 to 12.
    * UTC Day             1           Range is 1 to 31.
    * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetSystemDateAndTimeInput_INFO   *pInfo = (SetSystemDateAndTimeInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    memcpy(pInfo->country_code, pCur, 3); pCur += 3;

    pInfo->country_region_id = _GET_BYTE(pCur); pCur += 1;
    pInfo->daylightsavings   = _GET_BYTE(pCur); pCur += 1;
    pInfo->timezone          = _GET_BYTE(pCur); pCur += 1;
    pInfo->utc_hour          = _GET_BYTE(pCur); pCur += 1;
    pInfo->utc_minute        = _GET_BYTE(pCur); pCur += 1;
    pInfo->utc_second        = _GET_BYTE(pCur); pCur += 1;
    pInfo->utc_year          = _GET_WORD(pCur); pCur += 2;
    pInfo->utc_month         = _GET_BYTE(pCur); pCur += 1;
    pInfo->utc_day           = _GET_BYTE(pCur); pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetSystemDateAndTimeInput_Cmd_Pkt_Decode, SetSystemDateAndTimeInput_INFO, _SetSystemDateAndTimeInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8183
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(SetSystemDateAndTimeOutput_Cmd_Ctxt_New, SetSystemDateAndTimeOutput_INFO, CMD_SetSystemDateAndTimeOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_SetSystemDateAndTimeOutput_dec, SetSystemDateAndTimeOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(SetSystemDateAndTimeOutput_Cmd_Pkt_Decode, SetSystemDateAndTimeOutput_INFO, _SetSystemDateAndTimeOutput_dec);

//////////////////
static uint32_t
SetOSDInformationInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length              4       The total length of this command. It doesn't include the CheckSum.
     * Command Code                2       Code: 0x0185
     * Reserved                    1
     * User Name                   n       [string] User name for security.
     * Password                    n       [string] Password for security.
     * OSD Date Enable             1       0: Off, 1: On
     * OSD Date Position           1       0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Date Format             1       0: D/M/Y, 1: M/D/Y, 2: Y/M/D
     * OSD Time Enable             1       0: Off, 1: On
     * OSD Time Position           1       0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Date Format             1       0: AM/PM, 1: 24hr
     * OSD Logo Enable             1       0: Off, 1: On
     * OSD Logo Position           1       0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Logo option             1       0: Image1, 1: Image2.
     * OSD Detail Info Enable      1       0: Off, 1: On
     * OSD Detail Info Position    1       0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Detail Info option      1       0: Information1, 1: Information2.
     * OSD Text Enable             1       0: Off, 1: On
     * OSD Text Position           1       0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Text                    n       [string] Text string.
     * CheckSum                    1       =(byte[0]+...+byte[N]) MOD 256
     **/
    SetOSDInformationInput_INFO  *pInfo = (SetOSDInformationInput_INFO*)input_info;
    uint32_t                     cmd_length = 0;
    uint8_t                      *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 22 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->osd_text.length);

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
        _SET_WORD(pCur, CMD_SetOSDInformationInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->osd_date_enable);
        _SET_BYTE(pCur, pInfo->osd_date_position);
        _SET_BYTE(pCur, pInfo->osd_date_format);
        _SET_BYTE(pCur, pInfo->osd_time_enable);
        _SET_BYTE(pCur, pInfo->osd_time_position);
        _SET_BYTE(pCur, pInfo->osd_time_format);
        _SET_BYTE(pCur, pInfo->osd_logo_enable);
        _SET_BYTE(pCur, pInfo->osd_logo_position);
        _SET_BYTE(pCur, pInfo->osd_logo_option);
        _SET_BYTE(pCur, pInfo->osd_detail_info_enable);
        _SET_BYTE(pCur, pInfo->osd_detail_info_position);
        _SET_BYTE(pCur, pInfo->osd_detail_info_option);
        _SET_BYTE(pCur, pInfo->osd_text_enable);
        _SET_BYTE(pCur, pInfo->osd_text_position);

        _SET_DWORD(pCur, pInfo->osd_text.length);
        _SET_STRING(pCur, pInfo->osd_text.pStream, pInfo->osd_text.length);
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
_SetOSDInformationInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length              4       The total length of this command. It doesn't include the CheckSum.
     * Command Code                2       Code: 0x0185
     * Reserved                    1
     * User Name                   n       [string] User name for security.
     * Password                    n       [string] Password for security.
     * OSD Date Enable             1       0: Off, 1: On
     * OSD Date Position           1       0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Date Format             1       0: D/M/Y, 1: M/D/Y, 2: Y/M/D
     * OSD Time Enable             1       0: Off, 1: On
     * OSD Time Position           1       0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Date Format             1       0: AM/PM, 1: 24hr
     * OSD Logo Enable             1       0: Off, 1: On
     * OSD Logo Position           1       0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Logo option             1       0: Image1, 1: Image2.
     * OSD Detail Info Enable      1       0: Off, 1: On
     * OSD Detail Info Position    1       0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Detail Info option      1       0: Information1, 1: Information2.
     * OSD Text Enable             1       0: Off, 1: On
     * OSD Text Position           1       0: Left-Top, 1: Left-Center, 2: Left-Down
     * OSD Text                    n       [string] Text string.
     * CheckSum                    1       =(byte[0]+...+byte[N]) MOD 256
     **/
    SetOSDInformationInput_INFO   *pInfo = (SetOSDInformationInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->osd_date_enable          = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_date_position        = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_date_format          = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_time_enable          = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_time_position        = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_time_format          = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_logo_enable          = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_logo_position        = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_logo_option          = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_detail_info_enable   = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_detail_info_position = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_detail_info_option   = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_text_enable          = _GET_BYTE(pCur);  pCur += 1;
    pInfo->osd_text_position        = _GET_BYTE(pCur);  pCur += 1;

    pInfo->osd_text.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->osd_text.pStream = pCur;              pCur += pInfo->osd_text.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetOSDInformationInput_Cmd_Pkt_Decode, SetOSDInformationInput_INFO, _SetOSDInformationInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8185
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(SetOSDInformationOutput_Cmd_Ctxt_New, SetOSDInformationOutput_INFO, CMD_SetOSDInformationOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_SetOSDInformationOutput_dec, SetOSDInformationOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(SetOSDInformationOutput_Cmd_Pkt_Decode, SetOSDInformationOutput_INFO, _SetOSDInformationOutput_dec);

//=============================================================================
//                Public Function Definition
//=============================================================================

// DEFINE_CMD_PKT_CODEC(CMD_xxxInput, xxxInput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
// DEFINE_CMD_PKT_CODEC(CMD_xxxOutput, xxxOutput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);

DEFINE_CMD_PKT_CODEC(CMD_GetCapabilitiesInput, GetCapabilitiesInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetCapabilitiesInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetCapabilitiesOutput, GetCapabilitiesOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetCapabilitiesOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetDeviceInformationInput, GetDeviceInformationInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetDeviceInformationInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetDeviceInformationOutput, GetDeviceInformationOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetDeviceInformationOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetHostnameInput, GetHostnameInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetHostnameInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetHostnameOutput, GetHostnameOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetHostnameOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetSystemDateAndTimeInput, GetSystemDateAndTimeInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetSystemDateAndTimeInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetSystemDateAndTimeOutput, GetSystemDateAndTimeOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetSystemDateAndTimeOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetSystemLogInput, GetSystemLogInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetSystemLogInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetSystemLogOutput, GetSystemLogOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetSystemLogOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetOSDInformationInput, GetOSDInformationInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetOSDInformationInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetOSDInformationOutput, GetOSDInformationOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetOSDInformationOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SystemRebootInput, SystemRebootInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SystemRebootInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SystemRebootOutput, SystemRebootOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SystemRebootOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetSystemFactoryDefaultInput, SetSystemFactoryDefaultInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetSystemFactoryDefaultInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetSystemFactoryDefaultOutput, SetSystemFactoryDefaultOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetSystemFactoryDefaultOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetHostnameInput, SetHostnameInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetHostnameInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetHostnameOutput, SetHostnameOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetHostnameOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetSystemDateAndTimeInput, SetSystemDateAndTimeInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetSystemDateAndTimeInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetSystemDateAndTimeOutput, SetSystemDateAndTimeOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetSystemDateAndTimeOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetOSDInformationInput, SetOSDInformationInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetOSDInformationInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetOSDInformationOutput, SetOSDInformationOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetOSDInformationOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);

