

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
GetImagingSettingsInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0300
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * VideoSourceToken    n           [string] Reference token to the VideoSource for which the ImagingSettings.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetImagingSettingsInput_INFO  *pInfo = (GetImagingSettingsInput_INFO*)input_info;
    uint32_t                      cmd_length = 0;
    uint8_t                       *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->video_source_token.length);

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
        _SET_WORD(pCur, CMD_GetImagingSettingsInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->video_source_token.length);
        _SET_STRING(pCur, pInfo->video_source_token.pStream, pInfo->video_source_token.length);
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
_GetImagingSettingsInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0300
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * VideoSourceToken    n           [string] Reference token to the VideoSource for which the ImagingSettings.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetImagingSettingsInput_INFO   *pInfo = (GetImagingSettingsInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->video_source_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->video_source_token.pStream = pCur;              pCur += pInfo->video_source_token.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetImagingSettingsInput_Cmd_Pkt_Decode, GetImagingSettingsInput_INFO, _GetImagingSettingsInput_dec);

//////////////////
static uint32_t
GetImagingSettingsOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length              4       The total length of this command. It doesn't include the CheckSum.
     * Command Code                2       Code: 0x8300
     * Return Code                 1       0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name                   n       [string] User name for security.
     * Password                    n       [string] Password for security.
     * BacklightCompensation Mode  1       0: Off, 1: On
     * BacklightCompensation Level 4       [float]
     * Brightness                  4       [float] Image brightness.
     * ColorSaturation             4       [float] Color saturation of the image.
     * Contrast                    4       [float] Contrast of the image.
     * Exposure Mode               1       0: Auto - Enabled the exposure algorithm.
     * Exposure Priority           1       0: LowNoise, 1:FrameRate
     * Exposure Window bottom      2
     * Exposure Window top         2
     * Exposure Window right       2
     * Exposure Window left        2
     * MinExposureTime             4       In micro-second. Minimum value of exposure time range.
     * MaxExposureTime             4       In micro-second. Maximum value of exposure time range.
     * Exposure MinGain            4       [float] Minimum value of the sensor gain range.
     * Exposure MaxGain            4       [float] Maximum value of the sensor gain range.
     * Exposure MinIris            4       [float] Minimum value of the iris range.
     * Exposure MaxIris            4       [float] Maximum value of the iris range.
     * ExposureTime                4       In micro-second. The fixed exposure time used by the image sensor.
     * Exposure Gain               4       [float] The fixed gain used by the image sensor (db).
     * Exposure Iris               4       [float] The fixed attenuation of input light affected by the iris (dB). 0dB maps to a fully opened iris
     * AutoFocusMode               1       0: AUTO, 1: MANUAL
     * Focus DefaultSpeed          4       [float]
     * Focus NearLimit             4       [float] Parameter to set autofocus near limit (unit: meter).
     * Focus FarLimit              4       [float] Parameter to set autofocus far limit (unit: meter)
     * IrCutFilter Mode            1       0: ON, 1: OFF, 2: AUTO
     * Sharpness                   4       [float] Sharpness of the Video image.
     * WideDynamicRange Mode       1       0: OFF, 1: ON
     * WideDynamicRange Level      4       [float]
     * WhiteBalance Mode           1       0: AUTO, 1: MANUAL
     * WhiteBalance CrGain         4       [float] Rgain.
     * WhiteBalance CbGain         4       [float] Bgain.
     * ImageStabilization Mode     1       0: OFF, 1: ON, 2:AUTO, 3:Extended
     * ImageStabilization Level    4       [float] Optional level parameter
     * CheckSum                    1       =(byte[0]+...+byte[N]) MOD 256
     **/
    GetImagingSettingsOutput_INFO  *pInfo = (GetImagingSettingsOutput_INFO*)input_info;
    uint32_t                              cmd_length = 0;
    uint8_t                               *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 108 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetImagingSettingsOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->backlight_compensation_mode);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->backlight_compensation_level));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->brightness));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->color_saturation));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->contrast));
        _SET_BYTE(pCur, pInfo->exposure_mode);
        _SET_BYTE(pCur, pInfo->exposure_priority);
        _SET_WORD(pCur, pInfo->exposure_window_bottom);
        _SET_WORD(pCur, pInfo->exposure_window_top);
        _SET_WORD(pCur, pInfo->exposure_window_right);
        _SET_WORD(pCur, pInfo->exposure_window_left);
        _SET_DWORD(pCur, pInfo->min_exposuretime);
        _SET_DWORD(pCur, pInfo->max_exposuretime);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->exposure_mingain));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->exposure_maxgain));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->exposure_miniris));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->exposure_maxiris));
        _SET_DWORD(pCur, pInfo->exposure_time);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->exposure_gain));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->exposure_iris));
        _SET_BYTE(pCur, pInfo->auto_focus_mode);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->focus_default_speed));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->focus_near_limit));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->focus_far_limit));
        _SET_BYTE(pCur, pInfo->ir_cut_filter_mode);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->sharpness));
        _SET_BYTE(pCur, pInfo->wide_dynamic_range_mode);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->wide_dynamic_range_level));
        _SET_BYTE(pCur, pInfo->white_balance_mode);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->white_balance_cr_gain));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->white_balance_cb_gain));
        _SET_BYTE(pCur, pInfo->image_stabilization_mode);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->image_stabilization_level));
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
_GetImagingSettingsOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length              4       The total length of this command. It doesn't include the CheckSum.
     * Command Code                2       Code: 0x8300
     * Return Code                 1       0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name                   n       [string] User name for security.
     * Password                    n       [string] Password for security.
     * BacklightCompensation Mode  1       0: Off, 1: On
     * BacklightCompensation Level 4       [float]
     * Brightness                  4       [float] Image brightness.
     * ColorSaturation             4       [float] Color saturation of the image.
     * Contrast                    4       [float] Contrast of the image.
     * Exposure Mode               1       0: Auto - Enabled the exposure algorithm.
     * Exposure Priority           1       0: LowNoise, 1:FrameRate
     * Exposure Window bottom      2
     * Exposure Window top         2
     * Exposure Window right       2
     * Exposure Window left        2
     * MinExposureTime             4       In micro-second. Minimum value of exposure time range.
     * MaxExposureTime             4       In micro-second. Maximum value of exposure time range.
     * Exposure MinGain            4       [float] Minimum value of the sensor gain range.
     * Exposure MaxGain            4       [float] Maximum value of the sensor gain range.
     * Exposure MinIris            4       [float] Minimum value of the iris range.
     * Exposure MaxIris            4       [float] Maximum value of the iris range.
     * ExposureTime                4       In micro-second. The fixed exposure time used by the image sensor.
     * Exposure Gain               4       [float] The fixed gain used by the image sensor (db).
     * Exposure Iris               4       [float] The fixed attenuation of input light affected by the iris (dB). 0dB maps to a fully opened iris
     * AutoFocusMode               1       0: AUTO, 1: MANUAL
     * Focus DefaultSpeed          4       [float]
     * Focus NearLimit             4       [float] Parameter to set autofocus near limit (unit: meter).
     * Focus FarLimit              4       [float] Parameter to set autofocus far limit (unit: meter)
     * IrCutFilter Mode            1       0: ON, 1: OFF, 2: AUTO
     * Sharpness                   4       [float] Sharpness of the Video image.
     * WideDynamicRange Mode       1       0: OFF, 1: ON
     * WideDynamicRange Level      4       [float]
     * WhiteBalance Mode           1       0: AUTO, 1: MANUAL
     * WhiteBalance CrGain         4       [float] Rgain.
     * WhiteBalance CbGain         4       [float] Bgain.
     * ImageStabilization Mode     1       0: OFF, 1: ON, 2:AUTO, 3:Extended
     * ImageStabilization Level    4       [float] Optional level parameter
     * CheckSum                    1       =(byte[0]+...+byte[N]) MOD 256
     **/
    GetImagingSettingsOutput_INFO   *pInfo = (GetImagingSettingsOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->backlight_compensation_mode  = _GET_BYTE(pCur);  pCur += 1;
    pInfo->backlight_compensation_level = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->brightness                   = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->color_saturation             = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->contrast                     = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->exposure_mode                = _GET_BYTE(pCur);  pCur += 1;
    pInfo->exposure_priority            = _GET_BYTE(pCur);  pCur += 1;
    pInfo->exposure_window_bottom       = _GET_WORD(pCur);  pCur += 2;
    pInfo->exposure_window_top          = _GET_WORD(pCur);  pCur += 2;
    pInfo->exposure_window_right        = _GET_WORD(pCur);  pCur += 2;
    pInfo->exposure_window_left         = _GET_WORD(pCur);  pCur += 2;
    pInfo->min_exposuretime             = _GET_DWORD(pCur); pCur += 4;
    pInfo->max_exposuretime             = _GET_DWORD(pCur); pCur += 4;
    pInfo->exposure_mingain             = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->exposure_maxgain             = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->exposure_miniris             = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->exposure_maxiris             = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->exposure_time                = _GET_DWORD(pCur); pCur += 4;
    pInfo->exposure_gain                = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->exposure_iris                = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->auto_focus_mode              = _GET_BYTE(pCur);  pCur += 1;
    pInfo->focus_default_speed          = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->focus_near_limit             = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->focus_far_limit              = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->ir_cut_filter_mode           = _GET_BYTE(pCur);  pCur += 1;
    pInfo->sharpness                    = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->wide_dynamic_range_mode      = _GET_BYTE(pCur);  pCur += 1;
    pInfo->wide_dynamic_range_level     = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->white_balance_mode           = _GET_BYTE(pCur);  pCur += 1;
    pInfo->white_balance_cr_gain        = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->white_balance_cb_gain        = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->image_stabilization_mode     = _GET_BYTE(pCur);  pCur += 1;
    pInfo->image_stabilization_level    = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetImagingSettingsOutput_Cmd_Pkt_Decode, GetImagingSettingsOutput_INFO, _GetImagingSettingsOutput_dec);

//////////////////
static uint32_t
IMG_GetStatusInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4   The total length of this command. It doesn't include the CheckSum.
     * Command Code        2   Code: 0x0301
     * Reserved            1
     * User Name           n   [string] User name for security.
     * Password            n   [string] Password for security.
     * VideoSourceToken    n   [string] Reference token to the VideoSource where the imaging status should be requested.
     * CheckSum            1   =(byte[0]+...+byte[N]) MOD 256
     **/
    IMG_GetStatusInput_INFO  *pInfo = (IMG_GetStatusInput_INFO*)input_info;
    uint32_t                 cmd_length = 0;
    uint8_t                  *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->video_source_token.length);

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
        _SET_WORD(pCur, CMD_IMG_GetStatusInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->video_source_token.length);
        _SET_STRING(pCur, pInfo->video_source_token.pStream, pInfo->video_source_token.length);
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
_IMG_GetStatusInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4   The total length of this command. It doesn't include the CheckSum.
     * Command Code        2   Code: 0x0301
     * Reserved            1
     * User Name           n   [string] User name for security.
     * Password            n   [string] Password for security.
     * VideoSourceToken    n   [string] Reference token to the VideoSource where the imaging status should be requested.
     * CheckSum            1   =(byte[0]+...+byte[N]) MOD 256
     **/
    IMG_GetStatusInput_INFO   *pInfo = (IMG_GetStatusInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->video_source_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->video_source_token.pStream = pCur;              pCur += pInfo->video_source_token.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(IMG_GetStatusInput_Cmd_Pkt_Decode, IMG_GetStatusInput_INFO, _IMG_GetStatusInput_dec);

//////////////////
static uint32_t
IMG_GetStatusOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x8301
     * Return Code         1            0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * Position            4            Status of focus position.
     * MoveStatus          1            0: IDLE, 1: MOVING, 2: UNKNOWN
     * Error               n            [string] Error status of focus.
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
    IMG_GetStatusOutput_INFO  *pInfo = (IMG_GetStatusOutput_INFO*)input_info;
    uint32_t                  cmd_length = 0;
    uint8_t                   *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 13 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->error_msg.length);

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
        _SET_WORD(pCur, CMD_IMG_GetStatusOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->position);
        _SET_BYTE(pCur, pInfo->move_status);

        _SET_DWORD(pCur, pInfo->error_msg.length);
        _SET_STRING(pCur, pInfo->error_msg.pStream, pInfo->error_msg.length);
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
_IMG_GetStatusOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x8301
     * Return Code         1            0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * Position            4            Status of focus position.
     * MoveStatus          1            0: IDLE, 1: MOVING, 2: UNKNOWN
     * Error               n            [string] Error status of focus.
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
    IMG_GetStatusOutput_INFO   *pInfo = (IMG_GetStatusOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->position    = _GET_DWORD(pCur); pCur += 4;
    pInfo->move_status = _GET_BYTE(pCur);  pCur += 1;

    pInfo->error_msg.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->error_msg.pStream = pCur;              pCur += pInfo->error_msg.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(IMG_GetStatusOutput_Cmd_Pkt_Decode, IMG_GetStatusOutput_INFO, _IMG_GetStatusOutput_dec);

//////////////////
static uint32_t
SetImagingSettingsInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field                       Length(Byte)    Descriptions
     * Command Length                  4            The total length of this command. It doesn't include the CheckSum.
     * Command Code                    2            Code: 0x0380
     * Reserved                        1
     * User Name                       n            [string] User name for security.
     * Password                        n            [string] Password for security.
     * VideoSourceToken                n            [string] Reference token to the VideoSource for which the ImagingSettings.
     * BacklightCompensation Mode      1            0: OFF, 1: ON
     * BacklightCompensation Level     4            [float]
     * Brightness                      4            [float] Image brightness.
     * ColorSaturation                 4            [float] Color saturation of the image.
     * Contrast                        4            [float] Contrast of the image.
     * Exposure Mode                   1            0: Auto - Enabled the exposure algorithm.
     * Exposure Priority               1            0: LowNoise, 1:FrameRate
     * Exposure Window bottom          2            Rectangular exposure mask.
     * Exposure Window top             2
     * Exposure Window right           2
     * Exposure Window left            2
     * MinExposureTime                 4            In micro-second. Minimum value of exposure time range.
     * MaxExposureTime                 4            In micro-second. Maximum value of exposure time range.
     * Exposure MinGain                4            [float] Minimum value of the sensor gain range.
     * Exposure MaxGain                4            [float] Maximum value of the sensor gain range.
     * Exposure MinIris                4            [float] Minimum value of the iris range.
     * Exposure MaxIris                4            [float] Maximum value of the iris range.
     * ExposureTime                    4            In micro-second. The fixed exposure time used by the image sensor.
     * Exposure Gain                   4            [float] The fixed gain used by the image sensor (dB).
     * Exposure Iris                   4            [float] The fixed attenuation of input light affected by the iris (dB). 0dB maps to a fully opened iris.
     * AutoFocusMode                   1            0: AUTO, 1: MANUAL
     * Focus DefaultSpeed              4            [float]
     * Focus NearLimit                 4            [float] Parameter to set autofocus near limit (unit: meter).
     * Focus FarLimit                  4            [float] Parameter to set autofocus far limit (unit: meter).
     * IrCutFilter Mode                1            0: ON, 1: OFF, 2: AUTO
     * Sharpness                       4            [float] Sharpness of the Video image.
     * WideDynamicRange Mode           1            0: OFF, 1: ON
     * WideDynamicRange Level          4            [float]
     * WhiteBalance Mode               1            0: AUTO, 1: MANUAL
     * WhiteBalance CrGain             4            [float] Rgain.
     * WhiteBalance CbGain             4            [float] Bgain.
     * ImageStabilization Mode         1            0: OFF, 1: ON, 2:AUTO, 3:Extended
     * ImageStabilization Level        4            [float] Optional level parameter.
     * ForcePersistence                1            0: OFF, 1: ON, default is OFF
     * CheckSum                        1            =(byte[0]+...+byte[N]) MOD 256
     **/
    SetImagingSettingsInput_INFO  *pInfo = (SetImagingSettingsInput_INFO*)input_info;
    uint32_t                      cmd_length = 0;
    uint8_t                       *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 109 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->video_source_token.length);

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
        _SET_WORD(pCur, CMD_SetImagingSettingsInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->video_source_token.length);
        _SET_STRING(pCur, pInfo->video_source_token.pStream, pInfo->video_source_token.length);

        _SET_BYTE(pCur, pInfo->backlight_compensation_mode);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->backlight_compensation_level));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->brightness));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->color_saturation));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->contrast));
        _SET_BYTE(pCur, pInfo->exposure_mode);
        _SET_BYTE(pCur, pInfo->exposure_priority);
        _SET_WORD(pCur, pInfo->exposure_window_bottom);
        _SET_WORD(pCur, pInfo->exposure_window_top);
        _SET_WORD(pCur, pInfo->exposure_window_right);
        _SET_WORD(pCur, pInfo->exposure_window_left);
        _SET_DWORD(pCur, pInfo->min_exposure_time);
        _SET_DWORD(pCur, pInfo->max_exposure_time);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->exposure_min_gain));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->exposure_max_gain));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->exposure_min_iris));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->exposure_max_iris));
        _SET_DWORD(pCur, pInfo->exposure_time);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->exposure_gain));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->exposure_iris));
        _SET_BYTE(pCur, pInfo->auto_focus_mode);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->focus_default_speed));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->focus_nearlimit));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->focus_farlimit));
        _SET_BYTE(pCur, pInfo->ir_cut_filter_mode);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->sharpness));
        _SET_BYTE(pCur, pInfo->wide_dynamicrange_mode);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->wide_dynamicrange_level));
        _SET_BYTE(pCur, pInfo->white_balance_mode);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->white_balance_cr_gain));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->whitebalance_cb_gain));
        _SET_BYTE(pCur, pInfo->image_stabilization_mode);
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->image_stabilization_level));
        _SET_BYTE(pCur, pInfo->force_persistence);
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
_SetImagingSettingsInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field                       Length(Byte)    Descriptions
     * Command Length                  4            The total length of this command. It doesn't include the CheckSum.
     * Command Code                    2            Code: 0x0380
     * Reserved                        1
     * User Name                       n            [string] User name for security.
     * Password                        n            [string] Password for security.
     * VideoSourceToken                n            [string] Reference token to the VideoSource for which the ImagingSettings.
     * BacklightCompensation Mode      1            0: OFF, 1: ON
     * BacklightCompensation Level     4            [float]
     * Brightness                      4            [float] Image brightness.
     * ColorSaturation                 4            [float] Color saturation of the image.
     * Contrast                        4            [float] Contrast of the image.
     * Exposure Mode                   1            0: Auto - Enabled the exposure algorithm.
     * Exposure Priority               1            0: LowNoise, 1:FrameRate
     * Exposure Window bottom          2            Rectangular exposure mask.
     * Exposure Window top             2
     * Exposure Window right           2
     * Exposure Window left            2
     * MinExposureTime                 4            In micro-second. Minimum value of exposure time range.
     * MaxExposureTime                 4            In micro-second. Maximum value of exposure time range.
     * Exposure MinGain                4            [float] Minimum value of the sensor gain range.
     * Exposure MaxGain                4            [float] Maximum value of the sensor gain range.
     * Exposure MinIris                4            [float] Minimum value of the iris range.
     * Exposure MaxIris                4            [float] Maximum value of the iris range.
     * ExposureTime                    4            In micro-second. The fixed exposure time used by the image sensor.
     * Exposure Gain                   4            [float] The fixed gain used by the image sensor (dB).
     * Exposure Iris                   4            [float] The fixed attenuation of input light affected by the iris (dB). 0dB maps to a fully opened iris.
     * AutoFocusMode                   1            0: AUTO, 1: MANUAL
     * Focus DefaultSpeed              4            [float]
     * Focus NearLimit                 4            [float] Parameter to set autofocus near limit (unit: meter).
     * Focus FarLimit                  4            [float] Parameter to set autofocus far limit (unit: meter).
     * IrCutFilter Mode                1            0: ON, 1: OFF, 2: AUTO
     * Sharpness                       4            [float] Sharpness of the Video image.
     * WideDynamicRange Mode           1            0: OFF, 1: ON
     * WideDynamicRange Level          4            [float]
     * WhiteBalance Mode               1            0: AUTO, 1: MANUAL
     * WhiteBalance CrGain             4            [float] Rgain.
     * WhiteBalance CbGain             4            [float] Bgain.
     * ImageStabilization Mode         1            0: OFF, 1: ON, 2:AUTO, 3:Extended
     * ImageStabilization Level        4            [float] Optional level parameter.
     * ForcePersistence                1            0: OFF, 1: ON, default is OFF
     * CheckSum                        1            =(byte[0]+...+byte[N]) MOD 256
     **/
    SetImagingSettingsInput_INFO   *pInfo = (SetImagingSettingsInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->video_source_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->video_source_token.pStream = pCur;              pCur += pInfo->video_source_token.length;

    pInfo->backlight_compensation_mode  = _GET_BYTE(pCur);  pCur += 1;
    pInfo->backlight_compensation_level = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->brightness                   = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->color_saturation             = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->contrast                     = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->exposure_mode                = _GET_BYTE(pCur);  pCur += 1;
    pInfo->exposure_priority            = _GET_BYTE(pCur);  pCur += 1;
    pInfo->exposure_window_bottom       = _GET_WORD(pCur);  pCur += 2;
    pInfo->exposure_window_top          = _GET_WORD(pCur);  pCur += 2;
    pInfo->exposure_window_right        = _GET_WORD(pCur);  pCur += 2;
    pInfo->exposure_window_left         = _GET_WORD(pCur);  pCur += 2;
    pInfo->min_exposure_time            = _GET_DWORD(pCur); pCur += 4;
    pInfo->max_exposure_time            = _GET_DWORD(pCur); pCur += 4;
    pInfo->exposure_min_gain            = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->exposure_max_gain            = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->exposure_min_iris            = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->exposure_max_iris            = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->exposure_time                = _GET_DWORD(pCur); pCur += 4;
    pInfo->exposure_gain                = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->exposure_iris                = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->auto_focus_mode              = _GET_BYTE(pCur);  pCur += 1;
    pInfo->focus_default_speed          = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->focus_nearlimit              = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->focus_farlimit               = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->ir_cut_filter_mode           = _GET_BYTE(pCur);  pCur += 1;
    pInfo->sharpness                    = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->wide_dynamicrange_mode       = _GET_BYTE(pCur);  pCur += 1;
    pInfo->wide_dynamicrange_level      = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->white_balance_mode           = _GET_BYTE(pCur);  pCur += 1;
    pInfo->white_balance_cr_gain        = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->whitebalance_cb_gain         = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->image_stabilization_mode     = _GET_BYTE(pCur);  pCur += 1;
    pInfo->image_stabilization_level    = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->force_persistence            = _GET_BYTE(pCur);  pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetImagingSettingsInput_Cmd_Pkt_Decode, SetImagingSettingsInput_INFO, _SetImagingSettingsInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x8380
     * Return Code         1            0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(SetImagingSettingsOutput_Cmd_Ctxt_New, SetImagingSettingsOutput_INFO, CMD_SetImagingSettingsOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_SetImagingSettingsOutput_dec, SetImagingSettingsOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(SetImagingSettingsOutput_Cmd_Pkt_Decode, SetImagingSettingsOutput_INFO, _SetImagingSettingsOutput_dec);

//////////////////
static uint32_t
MoveInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length            4         The total length of this command. It doesn't include the CheckSum.
     * Command Code              2         Code: 0x0381
     * Reserved                  1
     * User Name                 n         [string] User name for security.
     * Password                  n         [string] Password for security.
     * VideoSourceToken          n         [string] Reference to the VideoSource for the requested move (focus) operation.
     * Absolute Focus Position   4         [float] Parameters for the absolute focus in meter.
     * Absolute Focus Speed      4         [float] Speed parameter for the absolute focus control.
     * Relative Focus Distance   4         [float] Distance parameter for the relative focus control in meter.
     * Relative Focus Speed      4         [float] Speed parameter for the relative focus control.
     * Continuous Focus Speed    4         [float] Speed parameter for the Continuous focus control.
     * CheckSum                  1         =(byte[0]+...+byte[N]) MOD 256
     **/
    MoveInput_INFO  *pInfo = (MoveInput_INFO*)input_info;
    uint32_t        cmd_length = 0;
    uint8_t         *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 28 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->video_source_token.length);

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
        _SET_WORD(pCur, CMD_MoveInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->video_source_token.length);
        _SET_STRING(pCur, pInfo->video_source_token.pStream, pInfo->video_source_token.length);

        _SET_DWORD(pCur, _floatTo4Byte(pInfo->absolute_focus_position));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->absolute_focus_speed));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->relative_focus_distance));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->relative_focus_speed));
        _SET_DWORD(pCur, _floatTo4Byte(pInfo->continuous_focus_speed));
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
_MoveInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length            4         The total length of this command. It doesn't include the CheckSum.
     * Command Code              2         Code: 0x0381
     * Reserved                  1
     * User Name                 n         [string] User name for security.
     * Password                  n         [string] Password for security.
     * VideoSourceToken          n         [string] Reference to the VideoSource for the requested move (focus) operation.
     * Absolute Focus Position   4         [float] Parameters for the absolute focus in meter.
     * Absolute Focus Speed      4         [float] Speed parameter for the absolute focus control.
     * Relative Focus Distance   4         [float] Distance parameter for the relative focus control in meter.
     * Relative Focus Speed      4         [float] Speed parameter for the relative focus control.
     * Continuous Focus Speed    4         [float] Speed parameter for the Continuous focus control.
     * CheckSum                  1         =(byte[0]+...+byte[N]) MOD 256
     **/
    MoveInput_INFO   *pInfo = (MoveInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->video_source_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->video_source_token.pStream = pCur;              pCur += pInfo->video_source_token.length;

    pInfo->absolute_focus_position = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->absolute_focus_speed    = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->relative_focus_distance = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->relative_focus_speed    = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    pInfo->continuous_focus_speed  = _4ByteToFloat(_GET_DWORD(pCur)); pCur += 4;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(MoveInput_Cmd_Pkt_Decode, MoveInput_INFO, _MoveInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x8381
     * Return Code         1            0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(MoveOutput_Cmd_Ctxt_New, MoveOutput_INFO, CMD_MoveOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_MoveOutput_dec, MoveOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(MoveOutput_Cmd_Pkt_Decode, MoveOutput_INFO, _MoveOutput_dec);

//////////////////
static uint32_t
IMG_StopInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0382
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * VideoSourceToken    n           [string] Reference token to the VideoSource where the focus movement should be stopped.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    IMG_StopInput_INFO  *pInfo = (IMG_StopInput_INFO*)input_info;
    uint32_t            cmd_length = 0;
    uint8_t             *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->video_source_token.length);

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
        _SET_WORD(pCur, CMD_IMG_StopInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->video_source_token.length);
        _SET_STRING(pCur, pInfo->video_source_token.pStream, pInfo->video_source_token.length);
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
_IMG_StopInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0382
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * VideoSourceToken    n           [string] Reference token to the VideoSource where the focus movement should be stopped.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    IMG_StopInput_INFO   *pInfo = (IMG_StopInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->video_source_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->video_source_token.pStream = pCur;              pCur += pInfo->video_source_token.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(IMG_StopInput_Cmd_Pkt_Decode, IMG_StopInput_INFO, _IMG_StopInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4            The total length of this command. It doesn't include the CheckSum.
     * Command Code        2            Code: 0x8382
     * Return Code         1            0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n            [string] User name for security.
     * Password            n            [string] Password for security.
     * CheckSum            1            =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(IMG_StopOutput_Cmd_Ctxt_New, IMG_StopOutput_INFO, CMD_IMG_StopOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_IMG_StopOutput_dec, IMG_StopOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(IMG_StopOutput_Cmd_Pkt_Decode, IMG_StopOutput_INFO, _IMG_StopOutput_dec);

//=============================================================================
//                Public Function Definition
//=============================================================================

// DEFINE_CMD_PKT_CODEC(CMD_xxxInput, xxxInput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
// DEFINE_CMD_PKT_CODEC(CMD_xxxOutput, xxxOutput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetImagingSettingsInput, GetImagingSettingsInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetImagingSettingsInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetImagingSettingsOutput, GetImagingSettingsOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetImagingSettingsOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_IMG_GetStatusInput, IMG_GetStatusInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, IMG_GetStatusInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_IMG_GetStatusOutput, IMG_GetStatusOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, IMG_GetStatusOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetImagingSettingsInput, SetImagingSettingsInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetImagingSettingsInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetImagingSettingsOutput, SetImagingSettingsOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetImagingSettingsOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_MoveInput, MoveInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, MoveInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_MoveOutput, MoveOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, MoveOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_IMG_StopInput, IMG_StopInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, IMG_StopInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_IMG_StopOutput, IMG_StopOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, IMG_StopOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
