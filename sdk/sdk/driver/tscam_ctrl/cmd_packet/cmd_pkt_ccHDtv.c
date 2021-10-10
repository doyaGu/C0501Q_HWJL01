
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
static uint32_t
GetTxDeviceAddressIDInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)  Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0000
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetTxDeviceAddressIDInput_INFO      *pInfo = (GetTxDeviceAddressIDInput_INFO*)input_info;
    uint32_t                            cmd_length = 0;
    uint8_t                             *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetTxDeviceAddressIDInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);
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
_GetTxDeviceAddressIDInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)  Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0000
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetTxDeviceAddressIDInput_INFO   *pInfo = (GetTxDeviceAddressIDInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetTxDeviceAddressIDInput_Cmd_Pkt_Decode, GetTxDeviceAddressIDInput_INFO, _GetTxDeviceAddressIDInput_dec);


//////////////////
static uint32_t
GetTxDeviceAddressIDOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field             Length(Byte)  Descriptions
     * Command Length           4       The total length of this command. It doesn't include the CheckSum.
     * Command Code             2       Code: 0x8000
     * Return Code              1       0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                n       [string] User name for security.
     * Password                 n       [string] Password for security.
     * TX Device Address ID     2       Return address ID from TX device
     * CheckSum                 1       =(byte[2]+...+byte[N]) MOD 256
     **/
    GetTxDeviceAddressIDOutput_INFO     *pInfo = (GetTxDeviceAddressIDOutput_INFO*)input_info;
    uint32_t                            cmd_length = 0;
    uint8_t                             *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 10 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

        pCmd_ctxt_Buf = tscm_malloc(cmd_length);
        if( !pCmd_ctxt_Buf )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }

        memset(pCmd_ctxt_Buf, 0x0, cmd_length);

        pCur = pCmd_ctxt_Buf;

        // real cmd_length of a cmd packet doesn't include check_sum size
        _SET_DWORD(pCur, cmd_length - 1);
        _SET_WORD(pCur, CMD_GetTxDeviceAddressIDOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_WORD(pCur, pInfo->tx_dev_addr_id);
        //---------------------

        // CheckSum
        check_sum = _tscm_gen_check_sum(pCmd_ctxt_Buf, cmd_length - 1);
        pCur = pCmd_ctxt_Buf + cmd_length - 1;
        _SET_BYTE(pCur, check_sum);

        if( output_info )       *((uint8_t**)output_info) = pCmd_ctxt_Buf;
    }while(0);

    return 0;
}

static void
_GetTxDeviceAddressIDOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field             Length(Byte)  Descriptions
     * Command Length           4       The total length of this command. It doesn't include the CheckSum.
     * Command Code             2       Code: 0x8000
     * Return Code              1       0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                n       [string] User name for security.
     * Password                 n       [string] Password for security.
     * TX Device Address ID     2       Return address ID from TX device
     * CheckSum                 1       =(byte[2]+...+byte[N]) MOD 256
     **/
    GetTxDeviceAddressIDOutput_INFO   *pInfo = (GetTxDeviceAddressIDOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);        pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->tx_dev_addr_id = _GET_WORD(pCur);     pCur += 2;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetTxDeviceAddressIDOutput_Cmd_Pkt_Decode, GetTxDeviceAddressIDOutput_INFO, _GetTxDeviceAddressIDOutput_dec);

//////////////////
static uint32_t
GetTransmissionParameterCapabilitiesInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0001
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetTransmissionParameterCapabilitiesInput_INFO  *pInfo = (GetTransmissionParameterCapabilitiesInput_INFO*)input_info;
    uint32_t                                        cmd_length = 0;
    uint8_t                                         *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetTransmissionParameterCapabilitiesInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);
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
_GetTransmissionParameterCapabilitiesInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0001
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetTransmissionParameterCapabilitiesInput_INFO   *pInfo = (GetTransmissionParameterCapabilitiesInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetTransmissionParameterCapabilitiesInput_Cmd_Pkt_Decode, GetTransmissionParameterCapabilitiesInput_INFO, _GetTransmissionParameterCapabilitiesInput_dec);

//////////////////
static uint32_t
GetTransmissionParameterCapabilitiesOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length          4           The total length of this command. It doesn't include the CheckSum.
     * Command Code            2           Code: 0x8001
     * Return Code             1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name               n           [string] User name for security.
     * Password                n           [string] Password for security.
     * Bandwidth Options       1           In MHz. 0: Unsupported, 1: Supported.
     * Frequency Min           4           Minimum frequency supported in KHz.
     * Frequency Max           4           Maxmum frequency suppoted in KHz.
     * Constellation Options   1           0: Unsupported, 1: Supported.
     * FFT Options             1           0: Unsupported, 1: Supported.
     * Code Rate Options       1           0: Unsupported, 1: Supported.
     * Guard Interval          1           0: Unsupported, 1: Supported.
     * RF Attenuation Min      1           -100~100, in 1db. Or 0x80 for positive flag.
     * RF Attenuation Max      1           -100~100, in 1db. Or 0x80 for positive flag.
     * CheckSum                1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetTransmissionParameterCapabilitiesOutput_INFO  *pInfo = (GetTransmissionParameterCapabilitiesOutput_INFO*)input_info;
    uint32_t                                         cmd_length = 0;
    uint8_t                                          *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 23 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetTransmissionParameterCapabilitiesOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->bandwidth_options);

        _SET_DWORD(pCur, pInfo->freq_min);
        _SET_DWORD(pCur, pInfo->freq_max);

        _SET_BYTE(pCur, pInfo->constellation_options);
        _SET_BYTE(pCur, pInfo->fft_options);
        _SET_BYTE(pCur, pInfo->code_rate_options);
        _SET_BYTE(pCur, pInfo->guard_interval);
        _SET_BYTE(pCur, pInfo->rf_attenuation_min);
        _SET_BYTE(pCur, pInfo->rf_attenuation_max);
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
_GetTransmissionParameterCapabilitiesOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length          4           The total length of this command. It doesn't include the CheckSum.
     * Command Code            2           Code: 0x8001
     * Return Code             1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name               n           [string] User name for security.
     * Password                n           [string] Password for security.
     * Bandwidth Options       1           In MHz. 0: Unsupported, 1: Supported.
     * Frequency Min           4           Minimum frequency supported in KHz.
     * Frequency Max           4           Maxmum frequency suppoted in KHz.
     * Constellation Options   1           0: Unsupported, 1: Supported.
     * FFT Options             1           0: Unsupported, 1: Supported.
     * Code Rate Options       1           0: Unsupported, 1: Supported.
     * Guard Interval          1           0: Unsupported, 1: Supported.
     * RF Attenuation Min      1           -100~100, in 1db. Or 0x80 for positive flag.
     * RF Attenuation Max      1           -100~100, in 1db. Or 0x80 for positive flag.
     * CheckSum                1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetTransmissionParameterCapabilitiesOutput_INFO   *pInfo = (GetTransmissionParameterCapabilitiesOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);        pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->bandwidth_options = _GET_BYTE(pCur);  pCur += 1;

    pInfo->freq_min         = _GET_DWORD(pCur);  pCur += 4;
    pInfo->freq_max         = _GET_DWORD(pCur);  pCur += 4;

    pInfo->constellation_options = _GET_BYTE(pCur);  pCur += 1;
    pInfo->fft_options           = _GET_BYTE(pCur);  pCur += 1;
    pInfo->code_rate_options     = _GET_BYTE(pCur);  pCur += 1;
    pInfo->guard_interval        = _GET_BYTE(pCur);  pCur += 1;
    pInfo->rf_attenuation_min    = _GET_BYTE(pCur);  pCur += 1;
    pInfo->rf_attenuation_max    = _GET_BYTE(pCur);  pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetTransmissionParameterCapabilitiesOutput_Cmd_Pkt_Decode, GetTransmissionParameterCapabilitiesOutput_INFO, _GetTransmissionParameterCapabilitiesOutput_dec);

//////////////////
static uint32_t
GetTransmissionParametersInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0002
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetTransmissionParametersInput_INFO  *pInfo = (GetTransmissionParametersInput_INFO*)input_info;
    uint32_t                             cmd_length = 0;
    uint8_t                              *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetTransmissionParametersInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);
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
_GetTransmissionParametersInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0002
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetTransmissionParametersInput_INFO   *pInfo = (GetTransmissionParametersInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetTransmissionParametersInput_Cmd_Pkt_Decode, GetTransmissionParametersInput_INFO, _GetTransmissionParametersInput_dec);

//////////////////
static uint32_t
GetTransmissionParametersOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8002
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Bandwidth           1           In MHz, Valid Settings:1,2,3,4,5,6,7,8
     * Frequency           4           In KHz, Valid Setting, 50000KHz~995000KHz
     * Constellation       1           0: QPSK, 1: 16QAM, 2: 64QAM
     * FFT                 1           0: 2K, 1:8K, 2: 4K
     * Code Rate           1           0: 1/2, 1: 2/3, 2: 3/4, 3: 5/6, 4: 7/8
     * Guard Interval      1           0: 1/32, 1: 1/16, 2: 1/8, 3: 1/4
     * Attenuation         1           -100~100, in 1db. Or 0x80 for positive flag.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetTransmissionParametersOutput_INFO  *pInfo = (GetTransmissionParametersOutput_INFO*)input_info;
    uint32_t                              cmd_length = 0;
    uint8_t                               *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 18 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetTransmissionParametersOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->bandwidth);
        _SET_DWORD(pCur, pInfo->frequency);

        _SET_BYTE(pCur, pInfo->constellation);
        _SET_BYTE(pCur, pInfo->fft);
        _SET_BYTE(pCur, pInfo->code_rate);
        _SET_BYTE(pCur, pInfo->guard_interval);
        _SET_BYTE(pCur, pInfo->attenuation);
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
_GetTransmissionParametersOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8002
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Bandwidth           1           In MHz, Valid Settings:1,2,3,4,5,6,7,8
     * Frequency           4           In KHz, Valid Setting, 50000KHz~995000KHz
     * Constellation       1           0: QPSK, 1: 16QAM, 2: 64QAM
     * FFT                 1           0: 2K, 1:8K, 2: 4K
     * Code Rate           1           0: 1/2, 1: 2/3, 2: 3/4, 3: 5/6, 4: 7/8
     * Guard Interval      1           0: 1/32, 1: 1/16, 2: 1/8, 3: 1/4
     * Attenuation         1           -100~100, in 1db. Or 0x80 for positive flag.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetTransmissionParametersOutput_INFO   *pInfo = (GetTransmissionParametersOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);        pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->bandwidth        = _GET_BYTE(pCur);  pCur += 1;
    pInfo->frequency        = _GET_DWORD(pCur); pCur += 4;
    pInfo->constellation    = _GET_BYTE(pCur);  pCur += 1;
    pInfo->fft              = _GET_BYTE(pCur);  pCur += 1;
    pInfo->code_rate        = _GET_BYTE(pCur);  pCur += 1;
    pInfo->guard_interval   = _GET_BYTE(pCur);  pCur += 1;
    pInfo->attenuation      = _GET_BYTE(pCur);  pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetTransmissionParametersOutput_Cmd_Pkt_Decode, GetTransmissionParametersOutput_INFO, _GetTransmissionParametersOutput_dec);

//////////////////
static uint32_t
GetHwRegisterValuesInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length              4        The total length of this command. It doesn't include the CheckSum.
     * Command Code                2        Code: 0x0003
     * Reserved                    1
     * User Name                   n        [string] User name for security.
     * Password                    n        [string] Password for security.
     * Processor                   1        0: OFDM, 8: LINK
     * Register Address            4        The start address for register read.
     * Register Value List Size    1        The size for multi-byte register read.
     * CheckSum                    1        =(byte[0]+...+byte[N]) MOD 256
     **/
    GetHwRegisterValuesInput_INFO   *pInfo = (GetHwRegisterValuesInput_INFO*)input_info;
    uint32_t                        cmd_length = 0;
    uint8_t                         *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 14 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetHwRegisterValuesInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->processor);
        _SET_DWORD(pCur, pInfo->register_address);
        _SET_BYTE(pCur, pInfo->register_value_list_size);
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
_GetHwRegisterValuesInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length              4        The total length of this command. It doesn't include the CheckSum.
     * Command Code                2        Code: 0x0003
     * Reserved                    1
     * User Name                   n        [string] User name for security.
     * Password                    n        [string] Password for security.
     * Processor                   1        0: OFDM, 8: LINK
     * Register Address            4        The start address for register read.
     * Register Value List Size    1        The size for multi-byte register read.
     * CheckSum                    1        =(byte[0]+...+byte[N]) MOD 256
     **/
    GetHwRegisterValuesInput_INFO   *pInfo = (GetHwRegisterValuesInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->processor                = _GET_BYTE(pCur);  pCur += 1;
    pInfo->register_address         = _GET_DWORD(pCur); pCur += 4;
    pInfo->register_value_list_size = _GET_BYTE(pCur);  pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetHwRegisterValuesInput_Cmd_Pkt_Decode, GetHwRegisterValuesInput_INFO, _GetHwRegisterValuesInput_dec);

//////////////////
static uint32_t
GetHwRegisterValuesOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length              4        The total length of this command. It doesn't include the CheckSum.
     * Command Code                2        Code: 0x8003
     * Return Code                 1        0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                   n        [string] User name for security.
     * Password                    n        [string] Password for security.
     * Register Value List Size    1        The size for multi-byte register read. The following entry would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { Register Value }.
     * Register Value List         1*n      Register value list in byte.
     * CheckSum                    1        =(byte[0]+...+byte[N]) MOD 256
     **/
    GetHwRegisterValuesOutput_INFO      *pInfo = (GetHwRegisterValuesOutput_INFO*)input_info;
    uint32_t                            cmd_length = 0;
    uint8_t                             *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + pInfo->register_value_list_size;

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
        _SET_WORD(pCur, CMD_GetHwRegisterValuesOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->register_value_list_size);
        _SET_PAYLOAD(pCur, pInfo->pRegister_value_list, pInfo->register_value_list_size);
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
_GetHwRegisterValuesOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length              4        The total length of this command. It doesn't include the CheckSum.
     * Command Code                2        Code: 0x8003
     * Return Code                 1        0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                   n        [string] User name for security.
     * Password                    n        [string] Password for security.
     * Register Value List Size    1        The size for multi-byte register read. The following entry would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { Register Value }.
     * Register Value List         1*n      Register value list in byte.
     * CheckSum                    1        =(byte[0]+...+byte[N]) MOD 256
     **/
    GetHwRegisterValuesOutput_INFO   *pInfo = (GetHwRegisterValuesOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);        pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->register_value_list_size = _GET_BYTE(pCur);   pCur += 1;
    pInfo->pRegister_value_list     = pCur;              pCur += pInfo->register_value_list_size;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetHwRegisterValuesOutput_Cmd_Pkt_Decode, GetHwRegisterValuesOutput_INFO, _GetHwRegisterValuesOutput_dec);

//////////////////
static uint32_t
GetAdvanceOptionsInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0004
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetAdvanceOptionsInput_INFO  *pInfo = (GetAdvanceOptionsInput_INFO*)input_info;
    uint32_t                             cmd_length = 0;
    uint8_t                              *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetAdvanceOptionsInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);
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
_GetAdvanceOptionsInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0004
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetAdvanceOptionsInput_INFO   *pInfo = (GetAdvanceOptionsInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetAdvanceOptionsInput_Cmd_Pkt_Decode, GetAdvanceOptionsInput_INFO, _GetAdvanceOptionsInput_dec);

//////////////////
static uint32_t
GetAdvanceOptionsOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field                                  Length(Byte)    Descriptions
     * Command Length                                4        The total length of this command. It doesn't include the CheckSum.
     * Command Code                                  2        Code: 0x8004
     * Return Code                                   1        0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                                     n        [string] User name for security.
     * Password                                      n        [string] Password for security.
     * PTS_PCR Latency                               2        In ms for PTS and PCR Latency
     * Rx Latency Recover Time Interval              2        In hrs for Rx latency recover time interval. This is a mechanism to patch Rx latency issue.
     * Rx Latency Recover Frame Skip Number          1        Frame skip nmaber for Rx latency recover.
     * Input Frame Rate Over Flow Number             1        Frame skip when input frame count over system frame count.
     * Frame encode frame data rate over flow size   2        In kbps. Skip frame when a frame encode data rate over this size.
     * CheckSum                                      1        =(byte[0]+...+byte[N]) MOD 256
     **/
    GetAdvanceOptionsOutput_INFO  *pInfo = (GetAdvanceOptionsOutput_INFO*)input_info;
    uint32_t                      cmd_length = 0;
    uint8_t                       *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 16 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetAdvanceOptionsOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_WORD(pCur, pInfo->pts_pcr_latency);
        _SET_WORD(pCur, pInfo->rx_latency_recover_time_interval);
        _SET_BYTE(pCur, pInfo->rx_latency_recover_frame_skip_number);
        _SET_BYTE(pCur, pInfo->input_frame_rate_over_flow_number);
        _SET_WORD(pCur, pInfo->frame_encode_frame_data_rate_over_flow_size);
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
_GetAdvanceOptionsOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field                                 Length(Byte)    Descriptions
     * Command Length                                4        The total length of this command. It doesn't include the CheckSum.
     * Command Code                                  2        Code: 0x8004
     * Return Code                                   1        0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                                     n        [string] User name for security.
     * Password                                      n        [string] Password for security.
     * PTS_PCR Latency                               2        In ms for PTS and PCR Latency
     * Rx Latency Recover Time Interval              2        In hrs for Rx latency recover time interval. This is a mechanism to patch Rx latency issue.
     * Rx Latency Recover Frame Skip Number          1        Frame skip nmaber for Rx latency recover.
     * Input Frame Rate Over Flow Number             1        Frame skip when input frame count over system frame count.
     * Frame encode frame data rate over flow size   2        In kbps. Skip frame when a frame encode data rate over this size.
     * CheckSum                                      1        =(byte[0]+...+byte[N]) MOD 256
     **/
    GetAdvanceOptionsOutput_INFO   *pInfo = (GetAdvanceOptionsOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);        pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->pts_pcr_latency                             = _GET_WORD(pCur); pCur += 2;
    pInfo->rx_latency_recover_time_interval            = _GET_WORD(pCur); pCur += 2;
    pInfo->rx_latency_recover_frame_skip_number        = _GET_BYTE(pCur); pCur += 1;
    pInfo->input_frame_rate_over_flow_number           = _GET_BYTE(pCur); pCur += 1;
    pInfo->frame_encode_frame_data_rate_over_flow_size = _GET_WORD(pCur); pCur += 2;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetAdvanceOptionsOutput_Cmd_Pkt_Decode, GetAdvanceOptionsOutput_INFO, _GetAdvanceOptionsOutput_dec);

//////////////////
static uint32_t
GetSiPsiTableInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0010
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetSiPsiTableInput_INFO  *pInfo = (GetSiPsiTableInput_INFO*)input_info;
    uint32_t                 cmd_length = 0;
    uint8_t                  *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetSiPsiTableInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);
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
_GetSiPsiTableInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0010
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetSiPsiTableInput_INFO   *pInfo = (GetSiPsiTableInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetSiPsiTableInput_Cmd_Pkt_Decode, GetSiPsiTableInput_INFO, _GetSiPsiTableInput_dec);

//////////////////
static uint32_t
GetSiPsiTableOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4   The total length of this command. It doesn't include the CheckSum.
     * Command Code        2   Code: 0x8010
     * Return Code         1   0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n   [string] User name for security.
     * Password            n   [string] Password for security.
     * ONID                2
     * NID                 2
     * TSID                2
     * Network Name        32  String in Unicode
     * CheckSum            1   =(byte[0]+...+byte[N]) MOD 256
     **/
    GetSiPsiTableOutput_INFO  *pInfo = (GetSiPsiTableOutput_INFO*)input_info;
    uint32_t                  cmd_length = 0;
    uint8_t                   *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 46 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetSiPsiTableOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_WORD(pCur, pInfo->onid);
        _SET_WORD(pCur, pInfo->nid);
        _SET_WORD(pCur, pInfo->tsid);

        _SET_PAYLOAD(pCur, pInfo->network_name, 32);
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
_GetSiPsiTableOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4   The total length of this command. It doesn't include the CheckSum.
     * Command Code        2   Code: 0x8010
     * Return Code         1   0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n   [string] User name for security.
     * Password            n   [string] Password for security.
     * ONID                2
     * NID                 2
     * TSID                2
     * Network Name        32  String in Unicode
     * CheckSum            1   =(byte[0]+...+byte[N]) MOD 256
     **/
    GetSiPsiTableOutput_INFO   *pInfo = (GetSiPsiTableOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);        pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->onid = _GET_WORD(pCur);  pCur += 2;
    pInfo->nid  = _GET_WORD(pCur);  pCur += 2;
    pInfo->tsid = _GET_WORD(pCur);  pCur += 2;

    memcpy(pInfo->network_name, pCur, 32); pCur += 32;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetSiPsiTableOutput_Cmd_Pkt_Decode, GetSiPsiTableOutput_INFO, _GetSiPsiTableOutput_dec);

//////////////////
static uint32_t
GetNitLocationInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0011
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetNitLocationInput_INFO  *pInfo = (GetNitLocationInput_INFO*)input_info;
    uint32_t                  cmd_length = 0;
    uint8_t                   *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetNitLocationInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);
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
_GetNitLocationInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0011
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetNitLocationInput_INFO   *pInfo = (GetNitLocationInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetNitLocationInput_Cmd_Pkt_Decode, GetNitLocationInput_INFO, _GetNitLocationInput_dec);

//////////////////
static uint32_t
GetNitLocationOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x8011
     * Return Code         1            0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * Latitude            2            cell_list_descriptor:cell_latitude
     * Longitude           2            cell_list_descriptor:cell_longitude
     * Extent_Latitude     2            cell_list_descriptor:cell_extent_of_latitude
     * Extent_Longitude    2            cell_list_descriptor: cell_extent_of_longitude
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
    GetNitLocationOutput_INFO  *pInfo = (GetNitLocationOutput_INFO*)input_info;
    uint32_t                   cmd_length = 0;
    uint8_t                    *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 16 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetNitLocationOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_WORD(pCur, pInfo->latitude);
        _SET_WORD(pCur, pInfo->longitude);
        _SET_WORD(pCur, pInfo->extent_latitude);
        _SET_WORD(pCur, pInfo->extent_longitude);
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
_GetNitLocationOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x8011
     * Return Code         1            0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * Latitude            2            cell_list_descriptor:cell_latitude
     * Longitude           2            cell_list_descriptor:cell_longitude
     * Extent_Latitude     2            cell_list_descriptor:cell_extent_of_latitude
     * Extent_Longitude    2            cell_list_descriptor: cell_extent_of_longitude
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
    GetNitLocationOutput_INFO   *pInfo = (GetNitLocationOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);        pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->latitude         = _GET_WORD(pCur);  pCur += 2;
    pInfo->longitude        = _GET_WORD(pCur);  pCur += 2;
    pInfo->extent_latitude  = _GET_WORD(pCur);  pCur += 2;
    pInfo->extent_longitude = _GET_WORD(pCur);  pCur += 2;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetNitLocationOutput_Cmd_Pkt_Decode, GetNitLocationOutput_INFO, _GetNitLocationOutput_dec);

//////////////////
static uint32_t
GetSdtServiceInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x0012
     * Reserved            1
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
    GetSdtServiceInput_INFO  *pInfo = (GetSdtServiceInput_INFO*)input_info;
    uint32_t                 cmd_length = 0;
    uint8_t                  *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetSdtServiceInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);
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
_GetSdtServiceInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x0012
     * Reserved            1
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
    GetSdtServiceInput_INFO   *pInfo = (GetSdtServiceInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetSdtServiceInput_Cmd_Pkt_Decode, GetSdtServiceInput_INFO, _GetSdtServiceInput_dec);

//////////////////
static uint32_t
GetSdtServiceOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x8012
     * Return Code         1            0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * Service ID          2            Program Number (SID)
     * Enable              1            0:Disable, 1:Enable, 2:Not available
     * LCN                 2            Logical Channel Number
     * Service Name        n            [string] In Unicode String
     * Provider            n            [string] In Unicode String
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
    GetSdtServiceOutput_INFO  *pInfo = (GetSdtServiceOutput_INFO*)input_info;
    uint32_t                  cmd_length = 0;
    uint8_t                   *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 13 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->service_name.length) + (4+pInfo->provider.length);

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
        _SET_WORD(pCur, CMD_GetSdtServiceOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_WORD(pCur, pInfo->service_id);
        _SET_BYTE(pCur, pInfo->enable);
        _SET_WORD(pCur, pInfo->lcn);

        _SET_DWORD(pCur, pInfo->service_name.length);
        _SET_STRING(pCur, pInfo->service_name.pStream, pInfo->service_name.length);

        _SET_DWORD(pCur, pInfo->provider.length);
        _SET_STRING(pCur, pInfo->provider.pStream, pInfo->provider.length);
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
_GetSdtServiceOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x8012
     * Return Code         1            0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * Service ID          2            Program Number (SID)
     * Enable              1            0:Disable, 1:Enable, 2:Not available
     * LCN                 2            Logical Channel Number
     * Service Name        n            [string] In Unicode String
     * Provider            n            [string] In Unicode String
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
    GetSdtServiceOutput_INFO   *pInfo = (GetSdtServiceOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);        pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->service_id = _GET_WORD(pCur);  pCur += 2;
    pInfo->enable     = _GET_BYTE(pCur);  pCur += 1;
    pInfo->lcn        = _GET_WORD(pCur);  pCur += 2;

    pInfo->service_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->service_name.pStream = pCur;             pCur += pInfo->service_name.length;

    pInfo->provider.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->provider.pStream = pCur;              pCur += pInfo->provider.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetSdtServiceOutput_Cmd_Pkt_Decode, GetSdtServiceOutput_INFO, _GetSdtServiceOutput_dec);

//////////////////
static uint32_t
SetTxDeviceAddressIDInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length          4        The total length of this command. It doesn't include the CheckSum.
     * Command Code            2        Code: 0x0080
     * Reserved                1
     * User Name               n        [string] User name for security.
     * Password                n        [string] Password for security.
     * ID Type                 1        0: Random generate by TX Device (Ex. Camera).
     *                                  1: Set by ¡¨New Device Address ID¡¨ parameter.
     * New Device Address ID   2        1~65535
     * CheckSum                1        =(byte[0]+...+byte[N]) MOD 256
     **/
    SetTxDeviceAddressIDInput_INFO  *pInfo = (SetTxDeviceAddressIDInput_INFO*)input_info;
    uint32_t                        cmd_length = 0;
    uint8_t                         *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 11 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SetTxDeviceAddressIDInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->id_type);
        _SET_WORD(pCur, pInfo->new_device_address_id);
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
_SetTxDeviceAddressIDInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length          4        The total length of this command. It doesn't include the CheckSum.
     * Command Code            2        Code: 0x0080
     * Reserved                1
     * User Name               n        [string] User name for security.
     * Password                n        [string] Password for security.
     * ID Type                 1        0: Random generate by TX Device (Ex. Camera).
     *                                  1: Set by ¡¨New Device Address ID¡¨ parameter.
     * New Device Address ID   2        1~65535
     * CheckSum                1        =(byte[0]+...+byte[N]) MOD 256
     **/
    SetTxDeviceAddressIDInput_INFO   *pInfo = (SetTxDeviceAddressIDInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->id_type               = _GET_BYTE(pCur);  pCur += 1;
    pInfo->new_device_address_id = _GET_WORD(pCur);  pCur += 2;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetTxDeviceAddressIDInput_Cmd_Pkt_Decode, SetTxDeviceAddressIDInput_INFO, _SetTxDeviceAddressIDInput_dec);

//////////////////
static uint32_t
SetTxDeviceAddressIDOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x8080
     * Return Code         1            0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
    SetTxDeviceAddressIDOutput_INFO  *pInfo = (SetTxDeviceAddressIDOutput_INFO*)input_info;
    uint32_t                         cmd_length = 0;
    uint8_t                          *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SetTxDeviceAddressIDOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);
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
_SetTxDeviceAddressIDOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x8080
     * Return Code         1            0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
    SetTxDeviceAddressIDOutput_INFO   *pInfo = (SetTxDeviceAddressIDOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetTxDeviceAddressIDOutput_Cmd_Pkt_Decode, SetTxDeviceAddressIDOutput_INFO, _SetTxDeviceAddressIDOutput_dec);

//////////////////
static uint32_t
SetCalibrationTableInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x0081
     * Reserved            1
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * Access Option       1            0: Write to EEPROM,
     * Table Type          1            0: IQ table,
     *                                  1: DC table
     * Table Data          n            [string]
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
    SetCalibrationTableInput_INFO  *pInfo = (SetCalibrationTableInput_INFO*)input_info;
    uint32_t                       cmd_length = 0;
    uint8_t                        *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 10 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + + (4+pInfo->table_data.length);

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
        _SET_WORD(pCur, CMD_SetCalibrationTableInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->access_option);
        _SET_BYTE(pCur, pInfo->table_type);

        _SET_DWORD(pCur, pInfo->table_data.length);
        _SET_STRING(pCur, pInfo->table_data.pStream, pInfo->table_data.length);
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
_SetCalibrationTableInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x0081
     * Reserved            1
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * Access Option       1            0: Write to EEPROM,
     * Table Type          1            0: IQ table,
     *                                  1: DC table
     * Table Data          n            [string]
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
    SetCalibrationTableInput_INFO   *pInfo = (SetCalibrationTableInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->access_option = _GET_BYTE(pCur);  pCur += 1;
    pInfo->table_type    = _GET_BYTE(pCur);  pCur += 1;

    pInfo->table_data.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->table_data.pStream = pCur;              pCur += pInfo->table_data.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetCalibrationTableInput_Cmd_Pkt_Decode, SetCalibrationTableInput_INFO, _SetCalibrationTableInput_dec);

//////////////////
static uint32_t
SetCalibrationTableOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x8081
     * Return Code         1            0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
    SetCalibrationTableOutput_INFO  *pInfo = (SetCalibrationTableOutput_INFO*)input_info;
    uint32_t                        cmd_length = 0;
    uint8_t                         *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SetCalibrationTableOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);
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
_SetCalibrationTableOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x8081
     * Return Code         1            0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
    SetCalibrationTableOutput_INFO   *pInfo = (SetCalibrationTableOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetCalibrationTableOutput_Cmd_Pkt_Decode, SetCalibrationTableOutput_INFO, _SetCalibrationTableOutput_dec);

//////////////////
static uint32_t
SetTransmissionParametersInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)  Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0082
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Bandwidth           1           In MHz. Valid Settings: 1,2,3,4,5,6,7,8,
     * Frequency           4           In KHz. Valid Setting, 50000KHz~995000KHz
     * Constellation       1           0: QPSK, 1: 16QAM, 2: 64QAM,
     * FFT                 1           0: 2K, 1:8K, 2: 4K,
     * Code Rate           1           0: 1/2, 1: 2/3, 2: 3/4, 3: 5/6, 4: 7/8,
     * Guard Interval      1           0: 1/32, 1: 1/16, 2: 1/8, 3: 1/4,
     * RF Attenuation      1           -100~100, in 1db. Or 0x80 for positive flag.
     * Reserved            11
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetTransmissionParametersInput_INFO  *pInfo = (SetTransmissionParametersInput_INFO*)input_info;
    uint32_t                             cmd_length = 0;
    uint8_t                              *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 29 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SetTransmissionParametersInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->bandwidth);
        _SET_DWORD(pCur, pInfo->frequency);
        _SET_BYTE(pCur, pInfo->constellation);
        _SET_BYTE(pCur, pInfo->fft);
        _SET_BYTE(pCur, pInfo->code_rate);
        _SET_BYTE(pCur, pInfo->guard_interval);
        _SET_BYTE(pCur, pInfo->rf_attenuation);
        _SET_PAYLOAD(pCur, pInfo->reserved_1, 11);
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
_SetTransmissionParametersInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)  Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0082
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Bandwidth           1           In MHz. Valid Settings: 1,2,3,4,5,6,7,8,
     * Frequency           4           In KHz. Valid Setting, 50000KHz~995000KHz
     * Constellation       1           0: QPSK, 1: 16QAM, 2: 64QAM,
     * FFT                 1           0: 2K, 1:8K, 2: 4K,
     * Code Rate           1           0: 1/2, 1: 2/3, 2: 3/4, 3: 5/6, 4: 7/8,
     * Guard Interval      1           0: 1/32, 1: 1/16, 2: 1/8, 3: 1/4,
     * RF Attenuation      1           -100~100, in 1db. Or 0x80 for positive flag.
     * Reserved            11
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetTransmissionParametersInput_INFO   *pInfo = (SetTransmissionParametersInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->bandwidth        = _GET_BYTE(pCur);  pCur += 1;
    pInfo->frequency        = _GET_DWORD(pCur); pCur += 4;
    pInfo->constellation    = _GET_BYTE(pCur);  pCur += 1;
    pInfo->fft              = _GET_BYTE(pCur);  pCur += 1;
    pInfo->code_rate        = _GET_BYTE(pCur);  pCur += 1;
    pInfo->guard_interval   = _GET_BYTE(pCur);  pCur += 1;
    pInfo->rf_attenuation   = _GET_BYTE(pCur);  pCur += 1;

    memcpy(pInfo->reserved_1, pCur, 11); pCur += 11;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetTransmissionParametersInput_Cmd_Pkt_Decode, SetTransmissionParametersInput_INFO, _SetTransmissionParametersInput_dec);

//////////////////
static uint32_t
SetTransmissionParametersOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)  Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8082
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetTransmissionParametersOutput_INFO  *pInfo = (SetTransmissionParametersOutput_INFO*)input_info;
    uint32_t                              cmd_length = 0;
    uint8_t                               *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SetTransmissionParametersOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);
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
_SetTransmissionParametersOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)  Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8082
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetTransmissionParametersOutput_INFO   *pInfo = (SetTransmissionParametersOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetTransmissionParametersOutput_Cmd_Pkt_Decode, SetTransmissionParametersOutput_INFO, _SetTransmissionParametersOutput_dec);
//////////////////

static uint32_t
SetHwRegisterValuesInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field                Length(Byte)   Descriptions
     * Command Length              4       The total length of this command. It doesn't include the CheckSum.
     * Command Code                2       Code: 0x0083
     * Reserved                    1
     * User Name                   n       [string] User name for security.
     * Password                    n       [string] Password for security.
     * Processor                   1       0: OFDM, 8: LINK
     * Register Address            4       The start address for register write.
     * Register Value List Size    1       The size for multi-byte register write. The following entry would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { Register Value }.
     * Register Value List         1*n     Register value list in byte.
     * CheckSum                    1       =(byte[0]+...+byte[N]) MOD 256
     **/
    SetHwRegisterValuesInput_INFO  *pInfo = (SetHwRegisterValuesInput_INFO*)input_info;
    uint32_t                       cmd_length = 0;
    uint8_t                        *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 14 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + pInfo->register_value_list_size;

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
        _SET_WORD(pCur, CMD_SetHwRegisterValuesInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->processor);
        _SET_DWORD(pCur, pInfo->register_address);
        _SET_BYTE(pCur, pInfo->register_value_list_size);
        _SET_PAYLOAD(pCur, pInfo->pRegister_value_list, pInfo->register_value_list_size);
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
_SetHwRegisterValuesInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field                Length(Byte)   Descriptions
     * Command Length              4       The total length of this command. It doesn't include the CheckSum.
     * Command Code                2       Code: 0x0083
     * Reserved                    1
     * User Name                   n       [string] User name for security.
     * Password                    n       [string] Password for security.
     * Processor                   1       0: OFDM, 8: LINK
     * Register Address            4       The start address for register write.
     * Register Value List Size    1       The size for multi-byte register write. The following entry would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { Register Value }.
     * Register Value List         1*n     Register value list in byte.
     * CheckSum                    1       =(byte[0]+...+byte[N]) MOD 256
     **/
    SetHwRegisterValuesInput_INFO   *pInfo = (SetHwRegisterValuesInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->processor                = _GET_BYTE(pCur);   pCur += 1;
    pInfo->register_address         = _GET_DWORD(pCur);  pCur += 4;
    pInfo->register_value_list_size = _GET_BYTE(pCur);   pCur += 1;
    pInfo->pRegister_value_list     = pCur;              pCur += pInfo->register_value_list_size;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetHwRegisterValuesInput_Cmd_Pkt_Decode, SetHwRegisterValuesInput_INFO, _SetHwRegisterValuesInput_dec);

//////////////////
static uint32_t
SetHwRegisterValuesOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8083
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetHwRegisterValuesOutput_INFO  *pInfo = (SetHwRegisterValuesOutput_INFO*)input_info;
    uint32_t                        cmd_length = 0;
    uint8_t                         *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SetHwRegisterValuesOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);
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
_SetHwRegisterValuesOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8083
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetHwRegisterValuesOutput_INFO   *pInfo = (SetHwRegisterValuesOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetHwRegisterValuesOutput_Cmd_Pkt_Decode, SetHwRegisterValuesOutput_INFO, _SetHwRegisterValuesOutput_dec);

//////////////////
static uint32_t
SetAdvaneOptionsInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field                                   Length(Byte)    Descriptions
     * Command Length                                  4       The total length of this command. It doesn't include the CheckSum.
     * Command Code                                    2       Code: 0x0084
     * Reserved                                        1
     * User Name                                       n       [string] User name for security.
     * Password                                        n       [string] Password for security.
     * PTS_PCR Latency                                 2       In ms for PTS and PCR Latency
     * Rx Latency Recover Time Interval                2       In hrs for Rx latency recover time interval. This is a mechanism to patch Rx latency issue.
     * Rx Latency Recover Frame Skip Number            1       Frame skip nmaber for Rx latency recover.
     * Input Frame Rate Over Flow Number               1       Frame skip when input frame count over system frame count.
     * Frame encode frame data rate over flow size     2       In kbps. Skip frame when a frame encode data rate over this size.
     * CheckSum                                        1       =(byte[0]+...+byte[N]) MOD 256
     **/
    SetAdvaneOptionsInput_INFO  *pInfo = (SetAdvaneOptionsInput_INFO*)input_info;
    uint32_t                    cmd_length = 0;
    uint8_t                     *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 16 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SetAdvaneOptionsInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_WORD(pCur, pInfo->pts_pcr_latency);
        _SET_WORD(pCur, pInfo->rx_latency_recover_time_interval);
        _SET_BYTE(pCur, pInfo->rx_latency_recover_frame_skip_number);
        _SET_BYTE(pCur, pInfo->input_frame_rate_over_flow_number);
        _SET_WORD(pCur, pInfo->frame_encode_frame_data_rate_over_flow_size);
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
_SetAdvaneOptionsInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field                                   Length(Byte)    Descriptions
     * Command Length                                  4       The total length of this command. It doesn't include the CheckSum.
     * Command Code                                    2       Code: 0x0084
     * Reserved                                        1
     * User Name                                       n       [string] User name for security.
     * Password                                        n       [string] Password for security.
     * PTS_PCR Latency                                 2       In ms for PTS and PCR Latency
     * Rx Latency Recover Time Interval                2       In hrs for Rx latency recover time interval. This is a mechanism to patch Rx latency issue.
     * Rx Latency Recover Frame Skip Number            1       Frame skip nmaber for Rx latency recover.
     * Input Frame Rate Over Flow Number               1       Frame skip when input frame count over system frame count.
     * Frame encode frame data rate over flow size     2       In kbps. Skip frame when a frame encode data rate over this size.
     * CheckSum                                        1       =(byte[0]+...+byte[N]) MOD 256
     **/
    SetAdvaneOptionsInput_INFO   *pInfo = (SetAdvaneOptionsInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->pts_pcr_latency                             = _GET_WORD(pCur); pCur += 2;
    pInfo->rx_latency_recover_time_interval            = _GET_WORD(pCur); pCur += 2;
    pInfo->rx_latency_recover_frame_skip_number        = _GET_BYTE(pCur); pCur += 1;
    pInfo->input_frame_rate_over_flow_number           = _GET_BYTE(pCur); pCur += 1;
    pInfo->frame_encode_frame_data_rate_over_flow_size = _GET_WORD(pCur); pCur += 2;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetAdvaneOptionsInput_Cmd_Pkt_Decode, SetAdvaneOptionsInput_INFO, _SetAdvaneOptionsInput_dec);

//////////////////
static uint32_t
SetAdvaneOptionsOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions    Example
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8084    0x8084
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetAdvaneOptionsOutput_INFO  *pInfo = (SetAdvaneOptionsOutput_INFO*)input_info;
    uint32_t                     cmd_length = 0;
    uint8_t                      *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SetAdvaneOptionsOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);
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
_SetAdvaneOptionsOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions    Example
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8084    0x8084
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetAdvaneOptionsOutput_INFO   *pInfo = (SetAdvaneOptionsOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetAdvaneOptionsOutput_Cmd_Pkt_Decode, SetAdvaneOptionsOutput_INFO, _SetAdvaneOptionsOutput_dec);

//////////////////
static uint32_t
SetSiPsiTableInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0090
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * ONID                2
     * NID                 2
     * TSID                2
     * Network Name        32          String in Unicode
     * Reserved            16
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetSiPsiTableInput_INFO  *pInfo = (SetSiPsiTableInput_INFO*)input_info;
    uint32_t                 cmd_length = 0;
    uint8_t                  *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 62 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SetSiPsiTableInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_WORD(pCur, pInfo->onid);
        _SET_WORD(pCur, pInfo->nid);
        _SET_WORD(pCur, pInfo->tsid);
        _SET_PAYLOAD(pCur, pInfo->network_name, 32);
        _SET_PAYLOAD(pCur, pInfo->reserved_1, 16);
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
_SetSiPsiTableInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0090
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * ONID                2
     * NID                 2
     * TSID                2
     * Network Name        32          String in Unicode
     * Reserved            16
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetSiPsiTableInput_INFO   *pInfo = (SetSiPsiTableInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->onid = _GET_WORD(pCur); pCur += 2;
    pInfo->nid  = _GET_WORD(pCur); pCur += 2;
    pInfo->tsid = _GET_WORD(pCur); pCur += 2;

    memcpy(pInfo->network_name, pCur, 32); pCur += 32;
    memcpy(pInfo->reserved_1, pCur, 16);   pCur += 16;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetSiPsiTableInput_Cmd_Pkt_Decode, SetSiPsiTableInput_INFO, _SetSiPsiTableInput_dec);

//////////////////
static uint32_t
SetSiPsiTableOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8090
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetSiPsiTableOutput_INFO  *pInfo = (SetSiPsiTableOutput_INFO*)input_info;
    uint32_t                  cmd_length = 0;
    uint8_t                   *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SetSiPsiTableOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);
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
_SetSiPsiTableOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8090
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetSiPsiTableOutput_INFO   *pInfo = (SetSiPsiTableOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetSiPsiTableOutput_Cmd_Pkt_Decode, SetSiPsiTableOutput_INFO, _SetSiPsiTableOutput_dec);

//////////////////
static uint32_t
SetNitLocationInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0091
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Latitude            2           cell_list_descriptor:cell_latitude
     * Longitude           2           cell_list_descriptor:cell_longitude
     * Extent_Latitude     2           cell_list_descriptor:cell_extent_of_latitude
     * Extent_Longitude    2           cell_list_descriptor: cell_extent_of_longitude
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetNitLocationInput_INFO  *pInfo = (SetNitLocationInput_INFO*)input_info;
    uint32_t                  cmd_length = 0;
    uint8_t                   *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 16 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SetNitLocationInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_WORD(pCur, pInfo->latitude);
        _SET_WORD(pCur, pInfo->longitude);
        _SET_WORD(pCur, pInfo->extent_latitude);
        _SET_WORD(pCur, pInfo->extent_longitude);
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
_SetNitLocationInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0091
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Latitude            2           cell_list_descriptor:cell_latitude
     * Longitude           2           cell_list_descriptor:cell_longitude
     * Extent_Latitude     2           cell_list_descriptor:cell_extent_of_latitude
     * Extent_Longitude    2           cell_list_descriptor: cell_extent_of_longitude
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetNitLocationInput_INFO   *pInfo = (SetNitLocationInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->latitude         = _GET_WORD(pCur); pCur += 2;
    pInfo->longitude        = _GET_WORD(pCur); pCur += 2;
    pInfo->extent_latitude  = _GET_WORD(pCur); pCur += 2;
    pInfo->extent_longitude = _GET_WORD(pCur); pCur += 2;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetNitLocationInput_Cmd_Pkt_Decode, SetNitLocationInput_INFO, _SetNitLocationInput_dec);

//////////////////
static uint32_t
SetNitLocationOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It doesn't include the CheckSum.
     * Command Code    2           Code: 0x8091
     * Return Code     1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * CheckSum        1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetNitLocationOutput_INFO  *pInfo = (SetNitLocationOutput_INFO*)input_info;
    uint32_t                   cmd_length = 0;
    uint8_t                    *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SetNitLocationOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);
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
_SetNitLocationOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field       Length(Byte)    Descriptions
     * Command Length  4           The total length of this command. It doesn't include the CheckSum.
     * Command Code    2           Code: 0x8091
     * Return Code     1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name       n           [string] User name for security.
     * Password        n           [string] Password for security.
     * CheckSum        1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetNitLocationOutput_INFO   *pInfo = (SetNitLocationOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetNitLocationOutput_Cmd_Pkt_Decode, SetNitLocationOutput_INFO, _SetNitLocationOutput_dec);

//////////////////
static uint32_t
SetSdtServiceInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0092
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Service ID          2           Program Number (SID)
     * Enable              1           0:Disable, 1:Enable, 2:Not available
     * LCN                 2           Logical Channel Number
     * Service Name        n           [string] In Unicode String
     * Provider            n           [string] In Unicode String
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetSdtServiceInput_INFO  *pInfo = (SetSdtServiceInput_INFO*)input_info;
    uint32_t                 cmd_length = 0;
    uint8_t                  *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 13 + (4+pInfo->user_name.length) + (4+pInfo->password.length)
                        + (4+pInfo->service_name.length) + (4+pInfo->provider.length);

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
        _SET_WORD(pCur, CMD_SetSdtServiceInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_WORD(pCur, pInfo->service_id);
        _SET_BYTE(pCur, pInfo->enable);
        _SET_WORD(pCur, pInfo->lcn);

        _SET_DWORD(pCur, pInfo->service_name.length);
        _SET_STRING(pCur, pInfo->service_name.pStream, pInfo->service_name.length);

        _SET_DWORD(pCur, pInfo->provider.length);
        _SET_STRING(pCur, pInfo->provider.pStream, pInfo->provider.length);
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
_SetSdtServiceInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0092
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Service ID          2           Program Number (SID)
     * Enable              1           0:Disable, 1:Enable, 2:Not available
     * LCN                 2           Logical Channel Number
     * Service Name        n           [string] In Unicode String
     * Provider            n           [string] In Unicode String
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetSdtServiceInput_INFO   *pInfo = (SetSdtServiceInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->service_id = _GET_WORD(pCur);  pCur += 2;
    pInfo->enable     = _GET_BYTE(pCur);  pCur += 1;
    pInfo->lcn        = _GET_WORD(pCur);  pCur += 2;

    pInfo->service_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->service_name.pStream = pCur;             pCur += pInfo->service_name.length;

    pInfo->provider.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->provider.pStream = pCur;              pCur += pInfo->provider.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetSdtServiceInput_Cmd_Pkt_Decode, SetSdtServiceInput_INFO, _SetSdtServiceInput_dec);

//////////////////
static uint32_t
SetSdtServiceOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8092
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetSdtServiceOutput_INFO  *pInfo = (SetSdtServiceOutput_INFO*)input_info;
    uint32_t                  cmd_length = 0;
    uint8_t                   *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_SetSdtServiceOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);
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
_SetSdtServiceOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8092
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetSdtServiceOutput_INFO   *pInfo = (SetSdtServiceOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetSdtServiceOutput_Cmd_Pkt_Decode, SetSdtServiceOutput_INFO, _SetSdtServiceOutput_dec);

//=============================================================================
//                Public Function Definition
//=============================================================================

// DEFINE_CMD_PKT_CODEC(CMD_xxxInput, xxxInput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
// DEFINE_CMD_PKT_CODEC(CMD_xxxOutput, xxxOutput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);

// ccHDtv service
DEFINE_CMD_PKT_CODEC(CMD_GetTxDeviceAddressIDInput, GetTxDeviceAddressIDInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetTxDeviceAddressIDInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetTxDeviceAddressIDOutput, GetTxDeviceAddressIDOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetTxDeviceAddressIDOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetTransmissionParameterCapabilitiesInput, GetTransmissionParameterCapabilitiesInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetTransmissionParameterCapabilitiesInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetTransmissionParameterCapabilitiesOutput, GetTransmissionParameterCapabilitiesOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetTransmissionParameterCapabilitiesOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetTransmissionParametersInput, GetTransmissionParametersInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetTransmissionParametersInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetTransmissionParametersOutput, GetTransmissionParametersOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetTransmissionParametersOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetHwRegisterValuesInput, GetHwRegisterValuesInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetHwRegisterValuesInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetHwRegisterValuesOutput, GetHwRegisterValuesOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetHwRegisterValuesOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetAdvanceOptionsInput, GetAdvanceOptionsInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetAdvanceOptionsInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetAdvanceOptionsOutput, GetAdvanceOptionsOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetAdvanceOptionsOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetSiPsiTableInput, GetSiPsiTableInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetSiPsiTableInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetSiPsiTableOutput, GetSiPsiTableOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetSiPsiTableOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetNitLocationInput, GetNitLocationInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetNitLocationInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetNitLocationOutput, GetNitLocationOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetNitLocationOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetSdtServiceInput, GetSdtServiceInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetSdtServiceInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetSdtServiceOutput, GetSdtServiceOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetSdtServiceOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetTxDeviceAddressIDInput, SetTxDeviceAddressIDInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetTxDeviceAddressIDInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetTxDeviceAddressIDOutput, SetTxDeviceAddressIDOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetTxDeviceAddressIDOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetCalibrationTableInput, SetCalibrationTableInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetCalibrationTableInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetCalibrationTableOutput, SetCalibrationTableOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetCalibrationTableOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetTransmissionParametersInput, SetTransmissionParametersInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetTransmissionParametersInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetTransmissionParametersOutput, SetTransmissionParametersOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetTransmissionParametersOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetHwRegisterValuesInput, SetHwRegisterValuesInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetHwRegisterValuesInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetHwRegisterValuesOutput, SetHwRegisterValuesOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetHwRegisterValuesOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetAdvaneOptionsInput, SetAdvaneOptionsInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetAdvaneOptionsInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetAdvaneOptionsOutput, SetAdvaneOptionsOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetAdvaneOptionsOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetSiPsiTableInput, SetSiPsiTableInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetSiPsiTableInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetSiPsiTableOutput, SetSiPsiTableOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetSiPsiTableOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetNitLocationInput, SetNitLocationInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetNitLocationInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetNitLocationOutput, SetNitLocationOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetNitLocationOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetSdtServiceInput, SetSdtServiceInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetSdtServiceInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetSdtServiceOutput, SetSdtServiceOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetSdtServiceOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
