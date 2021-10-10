
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
     * Command Code        2           Code: 0x0400
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetProfilesInput_Cmd_Ctxt_New, GetProfilesInput_INFO, CMD_GetProfilesInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetProfilesInput_dec, GetProfilesInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetProfilesInput_Cmd_Pkt_Decode, GetProfilesInput_INFO, _GetProfilesInput_dec);

//////////////////
static uint32_t
GetProfilesOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field                                               Length(Byte)    Descriptions
     * Command Length                                          4           The total length of this command. It doesn't include the CheckSum.
     * Command Code                                            2           Code: 0x8400
     * Return Code                                             1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                                               n           [string] User name for security.
     * Password                                                n           [string] Password for security.
     * Profiles List Size                                      1           List size of all profiles that exist in the media service. The following 44 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All GetProfile parameters }.
     *   Name                                                    n           [string]
     *   Token                                                   n           [string]
     *   VideoSource Configuration Name                          n           [sting] User readable name.
     *   VideoSource Configruation token                         n           [sting] Token that uniquely refernces this configuration.
     *   VideoSource Configruation SourceToken                   n           [sting] Reference to the physical input.
     *   VideoSource Configuration UseCount                      1           Number of internal references currently using this configuration.
     *   VideoSource Configuration Bounds x                      2           Rectangle specifying the Video capturing area.
     *   VideoSource Configuration Bounds y                      2
     *   VideoSource Configuration Bounds width                  2
     *   VideoSource Configuration Bounds height                 2
     *   VideoSource Configuration Rotate Mode                   1           0: OFF, 1: ON, 2: AUTO
     *   VideoSource Configuration Rotate Degree                 2           How much degree of clockwise rotation of image for On mode.
     *   VideoSource Configuration Mirror Mode                   1           0: OFF, 1: Horizontal, 2: Vertical, 3: Both
     *   AudioSource Configuration Name                          n           [sting] User readable name.
     *   AudioSource Configuration token                         n           [sting] Token that uniquely refernces this configuration.
     *   AudioSource Configuration SourceToken                   n           [sting] Token of the Audio Source the configuration applies to
     *   AudioSource Configuration UseCount                      1           Number of internal references currently using this configuration.
     *   VideoEncoder Configuration Name                         n           [sting] User readable name.
     *   VideoEncoder Configuration token                        n           [sting] Token that uniquely refernces this configuration.
     *   VideoEncoder Configuration UseCount                     1           Number of internal references currently using this configuration.
     *   VideoEncoder Configuration Encoding Mode                1           0: JPEG, 1: MPEG4, 2: H264, 3: MPEG2
     *   VideoEncoder Configuration Resolution Width             2           Number of the columns of the Video image.
     *   VideoEncoder Configuration Resolution Height            2           Number of the lines of the Video image.
     *   VideoEncoder Configuration Quality                      1           Relative value for the video quantizers and the quality of the video.
     *   VideoEncoder Configuration RateControl FrameRateLimit   1           Maximum output framerate in fps. If an EncodingInterval is provided the resulting encoded framerate will be reduced by the given factor.
     *   VideoEncoder Configuration RateControl EncodingInterval 1           Interval at which images are encoded and transmitted. (A value of 1 means that every frame is encoded, a value of 2 means that every 2nd frame is encoded ...)
     *   VideoEncoder Configuration RateControl BitrateLimit     2           The maximum output bitrate in kbps
     *   VideoEncoder Configuration RateControlType              1           0: CBR
     *   VideoEncoder Configuration GovLength                    1           Determines the interval in which the I-Frames will be coded. An entry of 1 indicates I-Frames are continuously generated. An entry of 2 indicates that every 2nd image is an I-Frame, and 3 only every 3rd frame, etc. The frames in between are coded as P or B Frames.
     *   VideoEncoder Configuration Profile                      1           For MPEG4:
     *   AudioEncoder Configuration Name                         n           [sting] User readable name.
     *   AudioEncoder Configuration token                        n           [sting] Token that uniquely refernces this configuration.
     *   AudioEncoder Configuration UseCount                     1           Number of internal references currently using this configuration.
     *   AudioEncoder Configuration Encoding                     1           0: G711, 1: G726, 2:AAC, 3: G729, 4: MPEG1, 5: MPEG2, 6: AC3, 7: PCM, 8: ADPCM
     *   AudioEncoder Configuration Bitrate                      2           The output bitrate in kbps.
     *   AudioEncoder Configuration SampleRate                   2           The output sample rate in 100Hz.
     *   AudioOutput Configuration Name                          n           [string] User readable name.
     *   AudioOutput Configuration token                         n           [string] Token that uniquely refernces this configuration.
     *   AudioOutput Configuration OutputToken                   n           [string] Token of the phsycial Audio output.
     *   AudioOutput Configuration UseCount                      1           Number of internal references currently using this configuration.
     *   AudioOutput Configuration SendPrimacy                   1           0: Server, 1: Client, 2: Auto
     *   AudioOutput Configuration OutputLevel                   1           Volume setting of the output. The applicable range is defined via the option AudioOutputOptions.OutputLevelRange.
     *   AudioDecoder Name                                       n           [string] User readable name.
     *   AudioDecoder token                                      n           [string] Token that uniquely refernces this configuration.
     *   AudioDecoder UseCount                                   1           Number of internal references currently using this configuration.
     * CheckSum                                                1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetProfilesOutput_INFO  *pInfo = (GetProfilesOutput_INFO*)input_info;
    uint32_t                cmd_length = 0;
    uint8_t                 *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;
        uint32_t    i = 0;
        // uint32_t    profile_size = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length);
        for(i = 0; i < pInfo->profiles_list_size; i++)
        {
            cmd_length += (38
                           + (4+pInfo->pProfiles_data[i].name.length)
                           + (4+pInfo->pProfiles_data[i].token.length)
                           + (4+pInfo->pProfiles_data[i].video_source_configuration_name.length)
                           + (4+pInfo->pProfiles_data[i].video_source_configruation_token.length)
                           + (4+pInfo->pProfiles_data[i].video_source_configruation_sourcetoken.length)
                           + (4+pInfo->pProfiles_data[i].audio_source_configuration_name.length)
                           + (4+pInfo->pProfiles_data[i].audio_source_configuration_token.length)
                           + (4+pInfo->pProfiles_data[i].audio_source_configuration_sourcetoken.length)
                           + (4+pInfo->pProfiles_data[i].video_encoder_configuration_name.length)
                           + (4+pInfo->pProfiles_data[i].video_encoder_configuration_token.length)
                           + (4+pInfo->pProfiles_data[i].audio_encoder_configuration_name.length)
                           + (4+pInfo->pProfiles_data[i].audio_encoder_configuration_token.length)
                           + (4+pInfo->pProfiles_data[i].audio_output_configuration_name.length)
                           + (4+pInfo->pProfiles_data[i].audio_output_configuration_token.length)
                           + (4+pInfo->pProfiles_data[i].audio_output_configuration_outputtoken.length)
                           + (4+pInfo->pProfiles_data[i].audio_decoder_name.length)
                           + (4+pInfo->pProfiles_data[i].audio_decoder_token.length));
        }

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
        _SET_WORD(pCur, CMD_GetProfilesOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->profiles_list_size);
        for(i = 0; i < pInfo->profiles_list_size; i++)
        {
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].name.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].name.pStream, pInfo->pProfiles_data[i].name.length);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].token.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].token.pStream, pInfo->pProfiles_data[i].token.length);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].video_source_configuration_name.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].video_source_configuration_name.pStream, pInfo->pProfiles_data[i].video_source_configuration_name.length);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].video_source_configruation_token.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].video_source_configruation_token.pStream, pInfo->pProfiles_data[i].video_source_configruation_token.length);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].video_source_configruation_sourcetoken.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].video_source_configruation_sourcetoken.pStream, pInfo->pProfiles_data[i].video_source_configruation_sourcetoken.length);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].video_source_configuration_usecount);
            _SET_WORD(pCur, pInfo->pProfiles_data[i].video_source_configuration_bounds_x);
            _SET_WORD(pCur, pInfo->pProfiles_data[i].video_source_configuration_bounds_y);
            _SET_WORD(pCur, pInfo->pProfiles_data[i].video_source_configuration_bounds_width);
            _SET_WORD(pCur, pInfo->pProfiles_data[i].video_source_configuration_bounds_height);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].video_source_configuration_rotate_mode);
            _SET_WORD(pCur, pInfo->pProfiles_data[i].video_source_configuration_rotate_degree);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].video_source_configuration_mirror_mode);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].audio_source_configuration_name.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].audio_source_configuration_name.pStream, pInfo->pProfiles_data[i].audio_source_configuration_name.length);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].audio_source_configuration_token.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].audio_source_configuration_token.pStream, pInfo->pProfiles_data[i].audio_source_configuration_token.length);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].audio_source_configuration_sourcetoken.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].audio_source_configuration_sourcetoken.pStream, pInfo->pProfiles_data[i].audio_source_configuration_sourcetoken.length);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].audio_source_configuration_usecount);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].video_encoder_configuration_name.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].video_encoder_configuration_name.pStream, pInfo->pProfiles_data[i].video_encoder_configuration_name.length);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].video_encoder_configuration_token.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].video_encoder_configuration_token.pStream, pInfo->pProfiles_data[i].video_encoder_configuration_token.length);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].video_encoder_configuration_usecount);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].video_encoder_configuration_encoding_mode);
            _SET_WORD(pCur, pInfo->pProfiles_data[i].video_encoder_configuration_resolution_width);
            _SET_WORD(pCur, pInfo->pProfiles_data[i].video_encoder_configuration_resolution_height);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].video_encoder_configuration_quality);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].video_encoder_configuration_ratecontrol_frameratelimit);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].video_encoder_configuration_ratecontrol_encodinginterval);
            _SET_WORD(pCur, pInfo->pProfiles_data[i].video_encoder_configuration_ratecontrol_bitratelimit);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].video_encoder_configuration_ratecontroltype);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].video_encoder_configuration_govlength);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].video_encoder_configuration_profile);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].audio_encoder_configuration_name.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].audio_encoder_configuration_name.pStream, pInfo->pProfiles_data[i].audio_encoder_configuration_name.length);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].audio_encoder_configuration_token.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].audio_encoder_configuration_token.pStream, pInfo->pProfiles_data[i].audio_encoder_configuration_token.length);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].audio_encoder_configuration_usecount);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].audio_encoder_configuration_encoding);
            _SET_WORD(pCur, pInfo->pProfiles_data[i].audio_encoder_configuration_bitrate);
            _SET_WORD(pCur, pInfo->pProfiles_data[i].audio_encoder_configuration_samplerate);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].audio_output_configuration_name.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].audio_output_configuration_name.pStream, pInfo->pProfiles_data[i].audio_output_configuration_name.length);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].audio_output_configuration_token.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].audio_output_configuration_token.pStream, pInfo->pProfiles_data[i].audio_output_configuration_token.length);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].audio_output_configuration_outputtoken.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].audio_output_configuration_outputtoken.pStream, pInfo->pProfiles_data[i].audio_output_configuration_outputtoken.length);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].audio_output_configuration_usecount);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].audio_output_configuration_sendprimacy);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].audio_output_configuration_outputlevel);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].audio_decoder_name.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].audio_decoder_name.pStream, pInfo->pProfiles_data[i].audio_decoder_name.length);
            _SET_DWORD(pCur, pInfo->pProfiles_data[i].audio_decoder_token.length);
            _SET_STRING(pCur, pInfo->pProfiles_data[i].audio_decoder_token.pStream, pInfo->pProfiles_data[i].audio_decoder_token.length);
            _SET_BYTE(pCur, pInfo->pProfiles_data[i].audio_decoder_usecount);
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
_GetProfilesOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field                                               Length(Byte)    Descriptions
     * Command Length                                          4           The total length of this command. It doesn't include the CheckSum.
     * Command Code                                            2           Code: 0x8400
     * Return Code                                             1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                                               n           [string] User name for security.
     * Password                                                n           [string] Password for security.
     * Profiles List Size                                      1           List size of all profiles that exist in the media service. The following 44 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All GetProfile parameters }.
     *   Name                                                    n           [string]
     *   Token                                                   n           [string]
     *   VideoSource Configuration Name                          n           [sting] User readable name.
     *   VideoSource Configruation token                         n           [sting] Token that uniquely refernces this configuration.
     *   VideoSource Configruation SourceToken                   n           [sting] Reference to the physical input.
     *   VideoSource Configuration UseCount                      1           Number of internal references currently using this configuration.
     *   VideoSource Configuration Bounds x                      2           Rectangle specifying the Video capturing area.
     *   VideoSource Configuration Bounds y                      2
     *   VideoSource Configuration Bounds width                  2
     *   VideoSource Configuration Bounds height                 2
     *   VideoSource Configuration Rotate Mode                   1           0: OFF, 1: ON, 2: AUTO
     *   VideoSource Configuration Rotate Degree                 2           How much degree of clockwise rotation of image for On mode.
     *   VideoSource Configuration Mirror Mode                   1           0: OFF, 1: Horizontal, 2: Vertical, 3: Both
     *   AudioSource Configuration Name                          n           [sting] User readable name.
     *   AudioSource Configuration token                         n           [sting] Token that uniquely refernces this configuration.
     *   AudioSource Configuration SourceToken                   n           [sting] Token of the Audio Source the configuration applies to
     *   AudioSource Configuration UseCount                      1           Number of internal references currently using this configuration.
     *   VideoEncoder Configuration Name                         n           [sting] User readable name.
     *   VideoEncoder Configuration token                        n           [sting] Token that uniquely refernces this configuration.
     *   VideoEncoder Configuration UseCount                     1           Number of internal references currently using this configuration.
     *   VideoEncoder Configuration Encoding Mode                1           0: JPEG, 1: MPEG4, 2: H264, 3: MPEG2
     *   VideoEncoder Configuration Resolution Width             2           Number of the columns of the Video image.
     *   VideoEncoder Configuration Resolution Height            2           Number of the lines of the Video image.
     *   VideoEncoder Configuration Quality                      1           Relative value for the video quantizers and the quality of the video.
     *   VideoEncoder Configuration RateControl FrameRateLimit   1           Maximum output framerate in fps. If an EncodingInterval is provided the resulting encoded framerate will be reduced by the given factor.
     *   VideoEncoder Configuration RateControl EncodingInterval 1           Interval at which images are encoded and transmitted. (A value of 1 means that every frame is encoded, a value of 2 means that every 2nd frame is encoded ...)
     *   VideoEncoder Configuration RateControl BitrateLimit     2           The maximum output bitrate in kbps
     *   VideoEncoder Configuration RateControlType              1           0: CBR
     *   VideoEncoder Configuration GovLength                    1           Determines the interval in which the I-Frames will be coded. An entry of 1 indicates I-Frames are continuously generated. An entry of 2 indicates that every 2nd image is an I-Frame, and 3 only every 3rd frame, etc. The frames in between are coded as P or B Frames.
     *   VideoEncoder Configuration Profile                      1           For MPEG4:
     *   AudioEncoder Configuration Name                         n           [sting] User readable name.
     *   AudioEncoder Configuration token                        n           [sting] Token that uniquely refernces this configuration.
     *   AudioEncoder Configuration UseCount                     1           Number of internal references currently using this configuration.
     *   AudioEncoder Configuration Encoding                     1           0: G711, 1: G726, 2:AAC, 3: G729, 4: MPEG1, 5: MPEG2, 6: AC3, 7: PCM, 8: ADPCM
     *   AudioEncoder Configuration Bitrate                      2           The output bitrate in kbps.
     *   AudioEncoder Configuration SampleRate                   2           The output sample rate in 100Hz.
     *   AudioOutput Configuration Name                          n           [string] User readable name.
     *   AudioOutput Configuration token                         n           [string] Token that uniquely refernces this configuration.
     *   AudioOutput Configuration OutputToken                   n           [string] Token of the phsycial Audio output.
     *   AudioOutput Configuration UseCount                      1           Number of internal references currently using this configuration.
     *   AudioOutput Configuration SendPrimacy                   1           0: Server, 1: Client, 2: Auto
     *   AudioOutput Configuration OutputLevel                   1           Volume setting of the output. The applicable range is defined via the option AudioOutputOptions.OutputLevelRange.
     *   AudioDecoder Name                                       n           [string] User readable name.
     *   AudioDecoder token                                      n           [string] Token that uniquely refernces this configuration.
     *   AudioDecoder UseCount                                   1           Number of internal references currently using this configuration.
     * CheckSum                                                1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetProfilesOutput_INFO   *pInfo = (GetProfilesOutput_INFO*)pStruct_Info;

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

        pInfo->profiles_list_size = _GET_BYTE(pCur);    pCur += 1;

        pInfo->pProfiles_data = tscm_malloc(sizeof(PROFILES_DATA) * pInfo->profiles_list_size);
        if( !pInfo->pProfiles_data )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }
        memset(pInfo->pProfiles_data, 0x0, sizeof(PROFILES_DATA) * pInfo->profiles_list_size);

        for(i = 0; i < pInfo->profiles_list_size; i++)
        {
            pInfo->pProfiles_data[i].name.length          = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].name.pStream         = pCur;             pCur += pInfo->pProfiles_data[i].name.length;

            pInfo->pProfiles_data[i].token.length         =_GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].token.pStream        = pCur;            pCur += pInfo->pProfiles_data[i].token.length;

            pInfo->pProfiles_data[i].video_source_configuration_name.length       = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].video_source_configuration_name.pStream      = pCur;             pCur += pInfo->pProfiles_data[i].video_source_configuration_name.length;

            pInfo->pProfiles_data[i].video_source_configruation_token.length      = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].video_source_configruation_token.pStream     = pCur;             pCur += pInfo->pProfiles_data[i].video_source_configruation_token.length;

            pInfo->pProfiles_data[i].video_source_configruation_sourcetoken.length  = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].video_source_configruation_sourcetoken.pStream = pCur;             pCur += pInfo->pProfiles_data[i].video_source_configruation_sourcetoken.length;

            pInfo->pProfiles_data[i].video_source_configuration_usecount      = _GET_BYTE(pCur);  pCur += 1;
            pInfo->pProfiles_data[i].video_source_configuration_bounds_x      = _GET_WORD(pCur);  pCur += 2;
            pInfo->pProfiles_data[i].video_source_configuration_bounds_y      = _GET_WORD(pCur);  pCur += 2;
            pInfo->pProfiles_data[i].video_source_configuration_bounds_width  = _GET_WORD(pCur);  pCur += 2;
            pInfo->pProfiles_data[i].video_source_configuration_bounds_height = _GET_WORD(pCur);  pCur += 2;
            pInfo->pProfiles_data[i].video_source_configuration_rotate_mode   = _GET_BYTE(pCur);  pCur += 1;
            pInfo->pProfiles_data[i].video_source_configuration_rotate_degree = _GET_WORD(pCur);  pCur += 2;
            pInfo->pProfiles_data[i].video_source_configuration_mirror_mode   = _GET_BYTE(pCur);  pCur += 1;

            pInfo->pProfiles_data[i].audio_source_configuration_name.length   = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].audio_source_configuration_name.pStream  = pCur;             pCur += pInfo->pProfiles_data[i].audio_source_configuration_name.length;

            pInfo->pProfiles_data[i].audio_source_configuration_token.length  = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].audio_source_configuration_token.pStream = pCur;             pCur += pInfo->pProfiles_data[i].audio_source_configuration_token.length;

            pInfo->pProfiles_data[i].audio_source_configuration_sourcetoken.length  = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].audio_source_configuration_sourcetoken.pStream = pCur;             pCur += pInfo->pProfiles_data[i].audio_source_configuration_sourcetoken.length;

            pInfo->pProfiles_data[i].audio_source_configuration_usecount      = _GET_BYTE(pCur);   pCur += 1;

            pInfo->pProfiles_data[i].video_encoder_configuration_name.length  = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].video_encoder_configuration_name.pStream = pCur;             pCur += pInfo->pProfiles_data[i].video_encoder_configuration_name.length;

            pInfo->pProfiles_data[i].video_encoder_configuration_token.length  = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].video_encoder_configuration_token.pStream = pCur;             pCur += pInfo->pProfiles_data[i].video_encoder_configuration_token.length;

            pInfo->pProfiles_data[i].video_encoder_configuration_usecount                     = _GET_BYTE(pCur); pCur += 1;
            pInfo->pProfiles_data[i].video_encoder_configuration_encoding_mode                = _GET_BYTE(pCur); pCur += 1;
            pInfo->pProfiles_data[i].video_encoder_configuration_resolution_width             = _GET_WORD(pCur); pCur += 2;
            pInfo->pProfiles_data[i].video_encoder_configuration_resolution_height            = _GET_WORD(pCur); pCur += 2;
            pInfo->pProfiles_data[i].video_encoder_configuration_quality                      = _GET_BYTE(pCur); pCur += 1;
            pInfo->pProfiles_data[i].video_encoder_configuration_ratecontrol_frameratelimit   = _GET_BYTE(pCur); pCur += 1;
            pInfo->pProfiles_data[i].video_encoder_configuration_ratecontrol_encodinginterval = _GET_BYTE(pCur); pCur += 1;
            pInfo->pProfiles_data[i].video_encoder_configuration_ratecontrol_bitratelimit     = _GET_WORD(pCur); pCur += 2;
            pInfo->pProfiles_data[i].video_encoder_configuration_ratecontroltype              = _GET_BYTE(pCur); pCur += 1;
            pInfo->pProfiles_data[i].video_encoder_configuration_govlength                    = _GET_BYTE(pCur); pCur += 1;
            pInfo->pProfiles_data[i].video_encoder_configuration_profile                      = _GET_BYTE(pCur); pCur += 1;

            pInfo->pProfiles_data[i].audio_encoder_configuration_name.length      = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].audio_encoder_configuration_name.pStream     = pCur;             pCur += pInfo->pProfiles_data[i].audio_encoder_configuration_name.length;

            pInfo->pProfiles_data[i].audio_encoder_configuration_token.length     = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].audio_encoder_configuration_token.pStream    = pCur;             pCur += pInfo->pProfiles_data[i].audio_encoder_configuration_token.length;

            pInfo->pProfiles_data[i].audio_encoder_configuration_usecount         = _GET_BYTE(pCur); pCur += 1;
            pInfo->pProfiles_data[i].audio_encoder_configuration_encoding         = _GET_BYTE(pCur); pCur += 1;
            pInfo->pProfiles_data[i].audio_encoder_configuration_bitrate          = _GET_WORD(pCur); pCur += 2;
            pInfo->pProfiles_data[i].audio_encoder_configuration_samplerate       = _GET_WORD(pCur); pCur += 2;

            pInfo->pProfiles_data[i].audio_output_configuration_name.length       = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].audio_output_configuration_name.pStream      = pCur;             pCur += pInfo->pProfiles_data[i].audio_output_configuration_name.length;

            pInfo->pProfiles_data[i].audio_output_configuration_token.length      = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].audio_output_configuration_token.pStream     = pCur;             pCur += pInfo->pProfiles_data[i].audio_output_configuration_token.length;

            pInfo->pProfiles_data[i].audio_output_configuration_outputtoken.length  = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].audio_output_configuration_outputtoken.pStream = pCur;             pCur += pInfo->pProfiles_data[i].audio_output_configuration_outputtoken.length;

            pInfo->pProfiles_data[i].audio_output_configuration_usecount          = _GET_BYTE(pCur); pCur += 1;
            pInfo->pProfiles_data[i].audio_output_configuration_sendprimacy       = _GET_BYTE(pCur); pCur += 1;
            pInfo->pProfiles_data[i].audio_output_configuration_outputlevel       = _GET_BYTE(pCur); pCur += 1;

            pInfo->pProfiles_data[i].audio_decoder_name.length                    = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].audio_decoder_name.pStream                   = pCur;             pCur += pInfo->pProfiles_data[i].audio_decoder_name.length;

            pInfo->pProfiles_data[i].audio_decoder_token.length                   = _GET_DWORD(pCur); pCur += 4;
            pInfo->pProfiles_data[i].audio_decoder_token.pStream                  = pCur;             pCur += pInfo->pProfiles_data[i].audio_decoder_token.length;

            pInfo->pProfiles_data[i].audio_decoder_usecount                       = _GET_BYTE(pCur);  pCur += 1;
        }
    }while(0);
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetProfilesOutput_Cmd_Pkt_Decode, GetProfilesOutput_INFO, _GetProfilesOutput_dec);

static uint32_t
_GetProfilesOutput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            GetProfilesOutput_INFO   *pInfo = (GetProfilesOutput_INFO*)(*((uint8_t**)input_info));
            if( pInfo->pProfiles_data )     free(pInfo->pProfiles_data);
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
     * Command Code        2           Code: 0x0401
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetVideoSourcesInput_Cmd_Ctxt_New, GetVideoSourcesInput_INFO, CMD_GetVideoSourcesInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetVideoSourcesInput_dec, GetVideoSourcesInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetVideoSourcesInput_Cmd_Pkt_Decode, GetVideoSourcesInput_INFO, _GetVideoSourcesInput_dec);

//////////////////
static uint32_t
GetVideoSourcesOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length          4            The total length of this command. It doesn't include the CheckSum.
     * Command Code            2            Code: 0x8401
     * Return Code             1            0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name               n            [string] User name for security.
     * Password                n            [string] Password for security.
     * VideoSources List Size  1            The list size of existing Video Sources. The following 4 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All VideoSource parameters }.
     *   token                   n            [srting] Unique identifier referencing the physical entity.
     *   Framerate               1            Frame rate in frames per second.
     *   Resolution Width        2            Number of the columns of the Video image.
     *   Resolution Height       2            Number of the lines of the Video image.
     * CheckSum                1            =(byte[0]+...+byte[N]) MOD 256
     **/
    GetVideoSourcesOutput_INFO  *pInfo = (GetVideoSourcesOutput_INFO*)input_info;
    uint32_t                    cmd_length = 0;
    uint8_t                     *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;
        uint32_t    i = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length);
        for(i = 0; i < pInfo->video_sources_list_size; i++)
            cmd_length += (5 + (4+pInfo->pVideo_sources_dat[i].token.length));

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
        _SET_WORD(pCur, CMD_GetVideoSourcesOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->video_sources_list_size);

        for(i = 0; i < pInfo->video_sources_list_size; i++)
        {
            _SET_DWORD(pCur, pInfo->pVideo_sources_dat[i].token.length);
            _SET_STRING(pCur, pInfo->pVideo_sources_dat[i].token.pStream, pInfo->pVideo_sources_dat[i].token.length);

            _SET_BYTE(pCur, pInfo->pVideo_sources_dat[i].frame_rate);
            _SET_WORD(pCur, pInfo->pVideo_sources_dat[i].resolution_width);
            _SET_WORD(pCur, pInfo->pVideo_sources_dat[i].resolution_height);
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
_GetVideoSourcesOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length          4            The total length of this command. It doesn't include the CheckSum.
     * Command Code            2            Code: 0x8401
     * Return Code             1            0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name               n            [string] User name for security.
     * Password                n            [string] Password for security.
     * VideoSources List Size  1            The list size of existing Video Sources. The following 4 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All VideoSource parameters }.
     * token                   n            [srting] Unique identifier referencing the physical entity.
     * Framerate               1            Frame rate in frames per second.
     * Resolution Width        2            Number of the columns of the Video image.
     * Resolution Height       2            Number of the lines of the Video image.
     * CheckSum                1            =(byte[0]+...+byte[N]) MOD 256
     **/
    GetVideoSourcesOutput_INFO   *pInfo = (GetVideoSourcesOutput_INFO*)pStruct_Info;

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

        pInfo->video_sources_list_size = _GET_BYTE(pCur); pCur += 1;

        pInfo->pVideo_sources_dat = tscm_malloc(sizeof(VIDEO_SOURCES_DATA) * pInfo->video_sources_list_size);
        if( !pInfo->pVideo_sources_dat )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }
        memset(pInfo->pVideo_sources_dat, 0x0, sizeof(VIDEO_SOURCES_DATA) * pInfo->video_sources_list_size);

        for(i = 0; i < pInfo->video_sources_list_size; i++)
        {
            pInfo->pVideo_sources_dat[i].token.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pVideo_sources_dat[i].token.pStream = pCur;              pCur += pInfo->pVideo_sources_dat[i].token.length;

            pInfo->pVideo_sources_dat[i].frame_rate        = _GET_BYTE(pCur);  pCur += 1;
            pInfo->pVideo_sources_dat[i].resolution_width  = _GET_WORD(pCur);  pCur += 2;
            pInfo->pVideo_sources_dat[i].resolution_height = _GET_WORD(pCur);  pCur += 2;
        }
    }while(0);
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetVideoSourcesOutput_Cmd_Pkt_Decode, GetVideoSourcesOutput_INFO, _GetVideoSourcesOutput_dec);

static uint32_t
_GetVideoSourcesOutput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            GetVideoSourcesOutput_INFO   *pInfo = (GetVideoSourcesOutput_INFO*)(*((uint8_t**)input_info));
            if( pInfo->pVideo_sources_dat )     free(pInfo->pVideo_sources_dat);
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
     * Command Code        2           Code: 0x0401
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetVideoSourceConfigurationsInput_Cmd_Ctxt_New, GetVideoSourceConfigurationsInput_INFO, CMD_GetVideoSourceConfigurationsInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetVideoSourceConfigurationsInput_dec, GetVideoSourceConfigurationsInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetVideoSourceConfigurationsInput_Cmd_Pkt_Decode, GetVideoSourceConfigurationsInput_INFO, _GetVideoSourceConfigurationsInput_dec);

//////////////////
static uint32_t
GetVideoSourceConfigurationsOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field                   Length(Byte)    Descriptions
     * Command Length              4           The total length of this command. It doesn't include the CheckSum.
     * Command Code                2           Code: 0x8402
     * Return Code                 1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                   n           [string] User name for security.
     * Password                    n           [string] Password for security.
     * Configurations List Size    1           The list size of video source configurations. The following 10 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All VideoSourceConfiguration parameters }.
     *   Name                        n           [string] User readable name.
     *   token                       n           [string] Token that uniquely refernces this configuration.
     *   SourceToken                 n           [string] Reference to the physical input.
     *   UseCount                    1           Number of internal references currently using this configuration.
     *   Bounds x                    2           Rectangle specifying the Video capturing area. The capturing area shall not be larger than the whole Video source area.
     *   Bounds y                    2
     *   Bounds width                2
     *   Bounds height               2
     *   Rotate Mode                 1           0: OFF, 1: ON, 2: AUTO.
     *   Rotate Degree               2           Optional parameter to configure how much degree of clockwise rotation of image for On mode. Omitting this parameter for On mode means 180 degree rotation.
     *   Mirror Mode                 1           0: OFF, 1: Horizontal, 2: Vertical, 3: Both
     * CheckSum                    1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetVideoSourceConfigurationsOutput_INFO  *pInfo = (GetVideoSourceConfigurationsOutput_INFO*)input_info;
    uint32_t                                 cmd_length = 0;
    uint8_t                                  *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;
        uint32_t    i = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length);
        for(i = 0; i < pInfo->configurations_list_size; i++)
        {
            cmd_length += (13
                           + (4+pInfo->pVideo_configurations[i].name.length)
                           + (4+pInfo->pVideo_configurations[i].token.length)
                           + (4+pInfo->pVideo_configurations[i].source_token.length));
        }

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
        _SET_WORD(pCur, CMD_GetVideoSourceConfigurationsOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->configurations_list_size);

        for(i = 0; i < pInfo->configurations_list_size; i++)
        {
            _SET_DWORD(pCur, pInfo->pVideo_configurations[i].name.length);
            _SET_STRING(pCur, pInfo->pVideo_configurations[i].name.pStream, pInfo->pVideo_configurations[i].name.length);
            _SET_DWORD(pCur, pInfo->pVideo_configurations[i].token.length);
            _SET_STRING(pCur, pInfo->pVideo_configurations[i].token.pStream, pInfo->pVideo_configurations[i].token.length);
            _SET_DWORD(pCur, pInfo->pVideo_configurations[i].source_token.length);
            _SET_STRING(pCur, pInfo->pVideo_configurations[i].source_token.pStream, pInfo->pVideo_configurations[i].source_token.length);

            _SET_BYTE(pCur, pInfo->pVideo_configurations[i].use_count);
            _SET_WORD(pCur, pInfo->pVideo_configurations[i].bounds_x);
            _SET_WORD(pCur, pInfo->pVideo_configurations[i].bounds_y);
            _SET_WORD(pCur, pInfo->pVideo_configurations[i].bounds_width);
            _SET_WORD(pCur, pInfo->pVideo_configurations[i].bounds_height);
            _SET_BYTE(pCur, pInfo->pVideo_configurations[i].rotate_mode);
            _SET_WORD(pCur, pInfo->pVideo_configurations[i].rotate_degree);
            _SET_BYTE(pCur, pInfo->pVideo_configurations[i].mirror_mode);
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
_GetVideoSourceConfigurationsOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field                   Length(Byte)    Descriptions
     * Command Length              4           The total length of this command. It doesn't include the CheckSum.
     * Command Code                2           Code: 0x8402
     * Return Code                 1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                   n           [string] User name for security.
     * Password                    n           [string] Password for security.
     * Configurations List Size    1           The list size of video source configurations. The following 10 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All VideoSourceConfiguration parameters }.
     *   Name                        n           [string] User readable name.
     *   token                       n           [string] Token that uniquely refernces this configuration.
     *   SourceToken                 n           [string] Reference to the physical input.
     *   UseCount                    1           Number of internal references currently using this configuration.
     *   Bounds x                    2           Rectangle specifying the Video capturing area. The capturing area shall not be larger than the whole Video source area.
     *   Bounds y                    2
     *   Bounds width                2
     *   Bounds height               2
     *   Rotate Mode                 1           0: OFF, 1: ON, 2: AUTO.
     *   Rotate Degree               2           Optional parameter to configure how much degree of clockwise rotation of image for On mode. Omitting this parameter for On mode means 180 degree rotation.
     *   Mirror Mode                 1           0: OFF, 1: Horizontal, 2: Vertical, 3: Both
     * CheckSum                    1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetVideoSourceConfigurationsOutput_INFO   *pInfo = (GetVideoSourceConfigurationsOutput_INFO*)pStruct_Info;

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

        pInfo->configurations_list_size = _GET_BYTE(pCur); pCur += 1;

        pInfo->pVideo_configurations = tscm_malloc(sizeof(VIDEO_CONFIGURATIONS) * pInfo->configurations_list_size);
        if( !pInfo->pVideo_configurations )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }
        memset(pInfo->pVideo_configurations, 0x0, sizeof(VIDEO_CONFIGURATIONS) * pInfo->configurations_list_size);

        for(i = 0; i < pInfo->configurations_list_size; i++)
        {
            pInfo->pVideo_configurations[i].name.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pVideo_configurations[i].name.pStream = pCur;              pCur += pInfo->pVideo_configurations[i].name.length;

            pInfo->pVideo_configurations[i].token.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pVideo_configurations[i].token.pStream = pCur;              pCur += pInfo->pVideo_configurations[i].token.length;

            pInfo->pVideo_configurations[i].source_token.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pVideo_configurations[i].source_token.pStream = pCur;              pCur += pInfo->pVideo_configurations[i].source_token.length;

            pInfo->pVideo_configurations[i].use_count     = _GET_BYTE(pCur); pCur += 1;
            pInfo->pVideo_configurations[i].bounds_x      = _GET_WORD(pCur); pCur += 2;
            pInfo->pVideo_configurations[i].bounds_y      = _GET_WORD(pCur); pCur += 2;
            pInfo->pVideo_configurations[i].bounds_width  = _GET_WORD(pCur); pCur += 2;
            pInfo->pVideo_configurations[i].bounds_height = _GET_WORD(pCur); pCur += 2;
            pInfo->pVideo_configurations[i].rotate_mode   = _GET_BYTE(pCur); pCur += 1;
            pInfo->pVideo_configurations[i].rotate_degree = _GET_WORD(pCur); pCur += 2;
            pInfo->pVideo_configurations[i].mirror_mode   = _GET_BYTE(pCur); pCur += 1;
        }
    }while(0);
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetVideoSourceConfigurationsOutput_Cmd_Pkt_Decode, GetVideoSourceConfigurationsOutput_INFO, _GetVideoSourceConfigurationsOutput_dec);

static uint32_t
_GetVideoSourceConfigurationsOutput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            GetVideoSourceConfigurationsOutput_INFO   *pInfo = (GetVideoSourceConfigurationsOutput_INFO*)(*((uint8_t**)input_info));
            if( pInfo->pVideo_configurations )     free(pInfo->pVideo_configurations);
        }

        free((*((uint8_t**)input_info)));
        (*((uint8_t**)input_info)) = 0;
    }while(0);

    return 0;
}
//////////////////
static uint32_t
GetGuaranteedNumberOfVideoEncoderInstancesInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0403
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * ConfigurationToken  n           [string] Token of the video source configuration
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetGuaranteedNumberOfVideoEncoderInstancesInput_INFO  *pInfo = (GetGuaranteedNumberOfVideoEncoderInstancesInput_INFO*)input_info;
    uint32_t                                              cmd_length = 0;
    uint8_t                                               *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->configuration_token.length);

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
        _SET_WORD(pCur, CMD_GetGuaranteedNumberOfVideoEncoderInstancesInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->configuration_token.length);
        _SET_STRING(pCur, pInfo->configuration_token.pStream, pInfo->configuration_token.length);
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
_GetGuaranteedNumberOfVideoEncoderInstancesInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0403
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * ConfigurationToken  n           [string] Token of the video source configuration
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetGuaranteedNumberOfVideoEncoderInstancesInput_INFO   *pInfo = (GetGuaranteedNumberOfVideoEncoderInstancesInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->configuration_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->configuration_token.pStream = pCur;              pCur += pInfo->configuration_token.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetGuaranteedNumberOfVideoEncoderInstancesInput_Cmd_Pkt_Decode, GetGuaranteedNumberOfVideoEncoderInstancesInput_INFO, _GetGuaranteedNumberOfVideoEncoderInstancesInput_dec);

//////////////////
static uint32_t
GetGuaranteedNumberOfVideoEncoderInstancesOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8403
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * TotalNumber         1           The minimum guaranteed total number of encoder instances (applications) per VideoSourceConfiguration. The device is able to deliver the TotalNumber of streams
     * JPEG Number         1           How many Jpeg streams can be set up at the same time per VideoSource.
     * H264 Number         1           How many H264 streams can be set up at the same time per VideoSource.
     * MPEG4 Number        1           How many Mpeg4 streams can be set up at the same time per VideoSource.
     * MPEG2 Number        1           How many Mpeg2 streams can be set up at the same time per VideoSource.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetGuaranteedNumberOfVideoEncoderInstancesOutput_INFO  *pInfo = (GetGuaranteedNumberOfVideoEncoderInstancesOutput_INFO*)input_info;
    uint32_t                                               cmd_length = 0;
    uint8_t                                                *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 13 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

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
        _SET_WORD(pCur, CMD_GetGuaranteedNumberOfVideoEncoderInstancesOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->total_number);
        _SET_BYTE(pCur, pInfo->jpeg_number);
        _SET_BYTE(pCur, pInfo->h264_number);
        _SET_BYTE(pCur, pInfo->mpeg4_number);
        _SET_BYTE(pCur, pInfo->mpeg2_number);
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
_GetGuaranteedNumberOfVideoEncoderInstancesOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8403
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * TotalNumber         1           The minimum guaranteed total number of encoder instances (applications) per VideoSourceConfiguration. The device is able to deliver the TotalNumber of streams
     * JPEG Number         1           How many Jpeg streams can be set up at the same time per VideoSource.
     * H264 Number         1           How many H264 streams can be set up at the same time per VideoSource.
     * MPEG4 Number        1           How many Mpeg4 streams can be set up at the same time per VideoSource.
     * MPEG2 Number        1           How many Mpeg2 streams can be set up at the same time per VideoSource.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetGuaranteedNumberOfVideoEncoderInstancesOutput_INFO   *pInfo = (GetGuaranteedNumberOfVideoEncoderInstancesOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->total_number = _GET_BYTE(pCur); pCur += 1;
    pInfo->jpeg_number  = _GET_BYTE(pCur); pCur += 1;
    pInfo->h264_number  = _GET_BYTE(pCur); pCur += 1;
    pInfo->mpeg4_number = _GET_BYTE(pCur); pCur += 1;
    pInfo->mpeg2_number = _GET_BYTE(pCur); pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetGuaranteedNumberOfVideoEncoderInstancesOutput_Cmd_Pkt_Decode, GetGuaranteedNumberOfVideoEncoderInstancesOutput_INFO, _GetGuaranteedNumberOfVideoEncoderInstancesOutput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0404
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[8]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetVideoEncoderConfigurationsInput_Cmd_Ctxt_New, GetVideoEncoderConfigurationsInput_INFO, CMD_GetVideoEncoderConfigurationsInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetVideoEncoderConfigurationsInput_dec, GetVideoEncoderConfigurationsInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetVideoEncoderConfigurationsInput_Cmd_Pkt_Decode, GetVideoEncoderConfigurationsInput_INFO, _GetVideoEncoderConfigurationsInput_dec);

//////////////////
static uint32_t
GetVideoEncoderConfigurationsOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length           4          The total length of this command. It doesn't include the CheckSum.
     * Command Code             2          Code: 0x8404
     * Return Code              1          0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                n          [string] User name for security.
     * Password                 n          [string] Password for security.
     * Configuration List Size  1          The list size of video encoder configurations. The following 14 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All VideoEncoderConfiguration parameters }.
     *   Name                     n          [string] User readable name.
     *   token                    n          [string] Token that uniquely refernces this configuration.
     *   UseCount                 1          Number of internal references currently using this configuration.
     *   Encoding                 1          0:JPEG, 1: MPEG4, 2: H264, 3: MPEG2
     *   Width                    2          Number of the columns of the Video image.
     *   Height                   2          Number of the lines of the Video image.
     *   Quality                  1          Relative value for the video quantizers and the quality of the video. A high value within supported quality range means higher quality
     *   FrameRateLimit           1          Maximum output framerate in fps.
     *   EncodingInterval         1          Interval at which images are encoded and transmitted. (A value of 1 means that every frame is encoded, a value of 2 means that every 2nd frame is encoded ...)
     *   BitrateLimit             2          The maximum output bitrate in kbps
     *   RateControlType          1          0: CBR
     *   GovLength                1          Group of Video frames length. Determines the interval in which the I-Frames will be coded. An entry of 1 indicates I-Frames are continuously generated. An entry of 2 indicates that every 2nd image is an I-Frame, and 3 only every 3rd frame, etc. The frames in between are coded as P or B Frames.
     *   Profile                  1          For MPEG4:
     * CheckSum                 1          =(byte[0]+...+byte[N]) MOD 256
     **/
    GetVideoEncoderConfigurationsOutput_INFO  *pInfo = (GetVideoEncoderConfigurationsOutput_INFO*)input_info;
    uint32_t                                  cmd_length = 0;
    uint8_t                                   *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;
        uint32_t    i = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length);
        for(i = 0; i < pInfo->configuration_list_size; i++)
        {
            cmd_length += (14
                           + (4+pInfo->pVideo_enc_configuration_data[i].name.length)
                           + (4+pInfo->pVideo_enc_configuration_data[i].token.length));
        }

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
        _SET_WORD(pCur, CMD_GetVideoEncoderConfigurationsOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->configuration_list_size);

        for(i = 0; i < pInfo->configuration_list_size; i++)
        {
            _SET_DWORD(pCur, pInfo->pVideo_enc_configuration_data[i].name.length);
            _SET_STRING(pCur, pInfo->pVideo_enc_configuration_data[i].name.pStream, pInfo->pVideo_enc_configuration_data[i].name.length);

            _SET_DWORD(pCur, pInfo->pVideo_enc_configuration_data[i].token.length);
            _SET_STRING(pCur, pInfo->pVideo_enc_configuration_data[i].token.pStream, pInfo->pVideo_enc_configuration_data[i].token.length);

            _SET_BYTE(pCur, pInfo->pVideo_enc_configuration_data[i].use_count);
            _SET_BYTE(pCur, pInfo->pVideo_enc_configuration_data[i].encoding);
            _SET_WORD(pCur, pInfo->pVideo_enc_configuration_data[i].width);
            _SET_WORD(pCur, pInfo->pVideo_enc_configuration_data[i].height);
            _SET_BYTE(pCur, pInfo->pVideo_enc_configuration_data[i].quality);
            _SET_BYTE(pCur, pInfo->pVideo_enc_configuration_data[i].frame_rate_limit);
            _SET_BYTE(pCur, pInfo->pVideo_enc_configuration_data[i].encoding_interval);
            _SET_WORD(pCur, pInfo->pVideo_enc_configuration_data[i].bitrate_limit);
            _SET_BYTE(pCur, pInfo->pVideo_enc_configuration_data[i].rate_control_type);
            _SET_BYTE(pCur, pInfo->pVideo_enc_configuration_data[i].gov_length);
            _SET_BYTE(pCur, pInfo->pVideo_enc_configuration_data[i].profile);
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
_GetVideoEncoderConfigurationsOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length           4          The total length of this command. It doesn't include the CheckSum.
     * Command Code             2          Code: 0x8404
     * Return Code              1          0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name                n          [string] User name for security.
     * Password                 n          [string] Password for security.
     * Configuration List Size  1          The list size of video encoder configurations. The following 14 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All VideoEncoderConfiguration parameters }.
     *   Name                     n          [string] User readable name.
     *   token                    n          [string] Token that uniquely refernces this configuration.
     *   UseCount                 1          Number of internal references currently using this configuration.
     *   Encoding                 1          0:JPEG, 1: MPEG4, 2: H264, 3: MPEG2
     *   Width                    2          Number of the columns of the Video image.
     *   Height                   2          Number of the lines of the Video image.
     *   Quality                  1          Relative value for the video quantizers and the quality of the video. A high value within supported quality range means higher quality
     *   FrameRateLimit           1          Maximum output framerate in fps.
     *   EncodingInterval         1          Interval at which images are encoded and transmitted. (A value of 1 means that every frame is encoded, a value of 2 means that every 2nd frame is encoded ...)
     *   BitrateLimit             2          The maximum output bitrate in kbps
     *   RateControlType          1          0: CBR
     *   GovLength                1          Group of Video frames length. Determines the interval in which the I-Frames will be coded. An entry of 1 indicates I-Frames are continuously generated. An entry of 2 indicates that every 2nd image is an I-Frame, and 3 only every 3rd frame, etc. The frames in between are coded as P or B Frames.
     *   Profile                  1          For MPEG4:
     * CheckSum                 1          =(byte[0]+...+byte[N]) MOD 256
     **/
    GetVideoEncoderConfigurationsOutput_INFO   *pInfo = (GetVideoEncoderConfigurationsOutput_INFO*)pStruct_Info;

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

        pInfo->configuration_list_size = _GET_BYTE(pCur);  pCur += 1;

        pInfo->pVideo_enc_configuration_data = tscm_malloc(sizeof(VIDEO_ENC_CONFIGURATION_DATA) * pInfo->configuration_list_size);
        if( !pInfo->pVideo_enc_configuration_data )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }
        memset(pInfo->pVideo_enc_configuration_data, 0x0, sizeof(VIDEO_ENC_CONFIGURATION_DATA) * pInfo->configuration_list_size);

        for(i = 0; i < pInfo->configuration_list_size; i++)
        {
            pInfo->pVideo_enc_configuration_data[i].name.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pVideo_enc_configuration_data[i].name.pStream = pCur;              pCur += pInfo->pVideo_enc_configuration_data[i].name.length;

            pInfo->pVideo_enc_configuration_data[i].token.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pVideo_enc_configuration_data[i].token.pStream = pCur;              pCur += pInfo->pVideo_enc_configuration_data[i].token.length;

            pInfo->pVideo_enc_configuration_data[i].use_count              = _GET_BYTE(pCur);  pCur += 1;
            pInfo->pVideo_enc_configuration_data[i].encoding               = _GET_BYTE(pCur);  pCur += 1;
            pInfo->pVideo_enc_configuration_data[i].width                  = _GET_WORD(pCur);  pCur += 2;
            pInfo->pVideo_enc_configuration_data[i].height                 = _GET_WORD(pCur);  pCur += 2;
            pInfo->pVideo_enc_configuration_data[i].quality                = _GET_BYTE(pCur);  pCur += 1;
            pInfo->pVideo_enc_configuration_data[i].frame_rate_limit       = _GET_BYTE(pCur);  pCur += 1;
            pInfo->pVideo_enc_configuration_data[i].encoding_interval      = _GET_BYTE(pCur);  pCur += 1;
            pInfo->pVideo_enc_configuration_data[i].bitrate_limit          = _GET_WORD(pCur);  pCur += 2;
            pInfo->pVideo_enc_configuration_data[i].rate_control_type      = _GET_BYTE(pCur);  pCur += 1;
            pInfo->pVideo_enc_configuration_data[i].gov_length             = _GET_BYTE(pCur);  pCur += 1;
            pInfo->pVideo_enc_configuration_data[i].profile                = _GET_BYTE(pCur);  pCur += 1;
        }
    }while(0);
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetVideoEncoderConfigurationsOutput_Cmd_Pkt_Decode, GetVideoEncoderConfigurationsOutput_INFO, _GetVideoEncoderConfigurationsOutput_dec);

static uint32_t
_GetVideoEncoderConfigurationsOutput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            GetVideoEncoderConfigurationsOutput_INFO   *pInfo = (GetVideoEncoderConfigurationsOutput_INFO*)(*((uint8_t**)input_info));
            if( pInfo->pVideo_enc_configuration_data )     free(pInfo->pVideo_enc_configuration_data);
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
     * Command Code        2           Code: 0x0405
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[8]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetAudioSourcesInput_Cmd_Ctxt_New, GetAudioSourcesInput_INFO, CMD_GetAudioSourcesInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetAudioSourcesInput_dec, GetAudioSourcesInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetAudioSourcesInput_Cmd_Pkt_Decode, GetAudioSourcesInput_INFO, _GetAudioSourcesInput_dec);

//////////////////
static uint32_t
GetAudioSourcesOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length          4           The total length of this command. It doesn't include the CheckSum.
     * Command Code            2           Code: 0x8405
     * Return Code             1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name               n           [string] User name for security.
     * Password                n           [string] Password for security.
     * AudioSources List Size  1           List size of existing Audio Sources. The following 2 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { AudioSources Token, Channels }.
     *   AudioSources Token      n           [string] The list of unique identifier referencing the physical entity.
     *   Channels                1           Number of available audio channels. (1: mono, 2: stereo)
     * CheckSum                1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetAudioSourcesOutput_INFO  *pInfo = (GetAudioSourcesOutput_INFO*)input_info;
    uint32_t                    cmd_length = 0;
    uint8_t                     *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;
        uint32_t    i = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length);
        for(i = 0; i < pInfo->audio_sources_list_size; i++)
            cmd_length += (1 + (4+pInfo->pAudio_sources_data[i].audio_sources_token.length));

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
        _SET_WORD(pCur, CMD_GetAudioSourcesOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->audio_sources_list_size);

        for(i = 0; i < pInfo->audio_sources_list_size; i++)
        {
            _SET_DWORD(pCur, pInfo->pAudio_sources_data[i].audio_sources_token.length);
            _SET_STRING(pCur, pInfo->pAudio_sources_data[i].audio_sources_token.pStream, pInfo->pAudio_sources_data[i].audio_sources_token.length);

            _SET_BYTE(pCur, pInfo->pAudio_sources_data[i].channels);
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
_GetAudioSourcesOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field               Length(Byte)    Descriptions
     * Command Length          4           The total length of this command. It doesn't include the CheckSum.
     * Command Code            2           Code: 0x8405
     * Return Code             1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name               n           [string] User name for security.
     * Password                n           [string] Password for security.
     * AudioSources List Size  1           List size of existing Audio Sources. The following 2 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { AudioSources Token, Channels }.
     *   AudioSources Token      n           [string] The list of unique identifier referencing the physical entity.
     *   Channels                1           Number of available audio channels. (1: mono, 2: stereo)
     * CheckSum                1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetAudioSourcesOutput_INFO   *pInfo = (GetAudioSourcesOutput_INFO*)pStruct_Info;

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

        pInfo->audio_sources_list_size = _GET_BYTE(pCur);           pCur += 1;

        pInfo->pAudio_sources_data = tscm_malloc(sizeof(AUDIO_SOURCES_DATA) * pInfo->audio_sources_list_size);
        if( !pInfo->pAudio_sources_data )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }
        memset(pInfo->pAudio_sources_data, 0x0, sizeof(AUDIO_SOURCES_DATA) * pInfo->audio_sources_list_size);

        for(i = 0; i < pInfo->audio_sources_list_size; i++)
        {
            pInfo->pAudio_sources_data[i].audio_sources_token.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pAudio_sources_data[i].audio_sources_token.pStream = pCur;              pCur += pInfo->pAudio_sources_data[i].audio_sources_token.length;

            pInfo->pAudio_sources_data[i].channels = _GET_BYTE(pCur);           pCur += 1;
        }
    }while(0);
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetAudioSourcesOutput_Cmd_Pkt_Decode, GetAudioSourcesOutput_INFO, _GetAudioSourcesOutput_dec);

static uint32_t
_GetAudioSourcesOutput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            GetAudioSourcesOutput_INFO   *pInfo = (GetAudioSourcesOutput_INFO*)(*((uint8_t**)input_info));
            if( pInfo->pAudio_sources_data )     free(pInfo->pAudio_sources_data);
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
     * Command Code        2           Code: 0x0406
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[8]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetAudioSourceConfigurationsInput_Cmd_Ctxt_New, GetAudioSourceConfigurationsInput_INFO, CMD_GetAudioSourceConfigurationsInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetAudioSourceConfigurationsInput_dec, GetAudioSourceConfigurationsInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetAudioSourceConfigurationsInput_Cmd_Pkt_Decode, GetAudioSourceConfigurationsInput_INFO, _GetAudioSourceConfigurationsInput_dec);

//////////////////
static uint32_t
GetAudioSourceConfigurationsOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field                   Length(Byte)    Descriptions
     * Command Length              4           The total length of this command. It doesn't include the CheckSum.
     * Command Code                2           Code: 0x8406
     * Return Code                 1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name                   n           [string] User name for security.
     * Password                    n           [string] Password for security.
     * Configurations List size    1           This element contains a list of audio source configurations. The following 4 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All AudioOutputConfiguration parameters }.
     *   Name                        n           [string] User readable name.
     *   Token                       n           [string] Token that uniquely refernces this configuration.
     *   SourceToken                 n           [string] Token of the Audio Source the configuration applies to
     *   UseCount                    1           Number of internal references currently using this configuration.
     * CheckSum                    1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetAudioSourceConfigurationsOutput_INFO  *pInfo = (GetAudioSourceConfigurationsOutput_INFO*)input_info;
    uint32_t                                 cmd_length = 0;
    uint8_t                                  *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;
        uint32_t    i = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length);
        for(i = 0; i < pInfo->configurations_list_size; i++)
        {
            cmd_length += (1
                           + (4+pInfo->pAudio_configurations[i].name.length)
                           + (4+pInfo->pAudio_configurations[i].token.length)
                           + (4+pInfo->pAudio_configurations[i].source_token.length));
        }

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
        _SET_WORD(pCur, CMD_GetAudioSourceConfigurationsOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->configurations_list_size);

        for(i = 0; i < pInfo->configurations_list_size; i++)
        {
            _SET_DWORD(pCur, pInfo->pAudio_configurations[i].name.length);
            _SET_STRING(pCur, pInfo->pAudio_configurations[i].name.pStream, pInfo->pAudio_configurations[i].name.length);

            _SET_DWORD(pCur, pInfo->pAudio_configurations[i].token.length);
            _SET_STRING(pCur, pInfo->pAudio_configurations[i].token.pStream, pInfo->pAudio_configurations[i].token.length);

            _SET_DWORD(pCur, pInfo->pAudio_configurations[i].source_token.length);
            _SET_STRING(pCur, pInfo->pAudio_configurations[i].source_token.pStream, pInfo->pAudio_configurations[i].source_token.length);

            _SET_BYTE(pCur, pInfo->pAudio_configurations[i].use_count);
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
_GetAudioSourceConfigurationsOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field                   Length(Byte)    Descriptions
     * Command Length              4           The total length of this command. It doesn't include the CheckSum.
     * Command Code                2           Code: 0x8406
     * Return Code                 1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name                   n           [string] User name for security.
     * Password                    n           [string] Password for security.
     * Configurations List size    1           This element contains a list of audio source configurations. The following 4 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All AudioOutputConfiguration parameters }.
     *   Name                        n           [string] User readable name.
     *   Token                       n           [string] Token that uniquely refernces this configuration.
     *   SourceToken                 n           [string] Token of the Audio Source the configuration applies to
     *   UseCount                    1           Number of internal references currently using this configuration.
     * CheckSum                    1           =(byte[0]+...+byte[N]) MOD 256
     **/
    GetAudioSourceConfigurationsOutput_INFO   *pInfo = (GetAudioSourceConfigurationsOutput_INFO*)pStruct_Info;

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

        pInfo->configurations_list_size = _GET_BYTE(pCur);           pCur += 1;

        pInfo->pAudio_configurations = tscm_malloc(sizeof(AUDIO_CONFIGURATIONS) * pInfo->configurations_list_size);
        if( !pInfo->pAudio_configurations )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }
        memset(pInfo->pAudio_configurations, 0x0, sizeof(AUDIO_CONFIGURATIONS) * pInfo->configurations_list_size);

        for(i = 0; i < pInfo->configurations_list_size; i++)
        {
            pInfo->pAudio_configurations[i].name.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pAudio_configurations[i].name.pStream = pCur;              pCur += pInfo->pAudio_configurations[i].name.length;

            pInfo->pAudio_configurations[i].token.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pAudio_configurations[i].token.pStream = pCur;              pCur += pInfo->pAudio_configurations[i].token.length;

            pInfo->pAudio_configurations[i].source_token.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pAudio_configurations[i].source_token.pStream = pCur;              pCur += pInfo->pAudio_configurations[i].source_token.length;

            pInfo->pAudio_configurations[i].use_count = _GET_BYTE(pCur);          pCur += 1;
        }
    }while(0);
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetAudioSourceConfigurationsOutput_Cmd_Pkt_Decode, GetAudioSourceConfigurationsOutput_INFO, _GetAudioSourceConfigurationsOutput_dec);

static uint32_t
_GetAudioSourceConfigurationsOutput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            GetAudioSourceConfigurationsOutput_INFO   *pInfo = (GetAudioSourceConfigurationsOutput_INFO*)(*((uint8_t**)input_info));
            if( pInfo->pAudio_configurations )     free(pInfo->pAudio_configurations);
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
     * Command Code        2           Code: 0x0407
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[8]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(GetAudioEncoderConfigurationsInput_Cmd_Ctxt_New, GetAudioEncoderConfigurationsInput_INFO, CMD_GetAudioEncoderConfigurationsInput, reserved);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_GetAudioEncoderConfigurationsInput_dec, GetAudioEncoderConfigurationsInput_INFO, reserved);
CMD_PKT_DECODE_INSTANCE(GetAudioEncoderConfigurationsInput_Cmd_Pkt_Decode, GetAudioEncoderConfigurationsInput_INFO, _GetAudioEncoderConfigurationsInput_dec);

//////////////////
static uint32_t
GetAudioEncoderConfigurationsOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field                Length(Byte)    Descriptions
     * Command Length              4         The total length of this command. It doesn't include the CheckSum.
     * Command Code                2         Code: 0x8407
     * Return Code                 1         0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name                   n         [string] User name for security.
     * Password                    n         [string] Password for security.
     * Configurations List size    1         This element contains a list of audio encoder configurations. The following 6 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All Configuration parameters }.
     *   Token                       n         [string] Token that uniquely refernces this configuration.
     *   Name                        n         [string] User readable name
     *   UseCount                    1         Number of internal references currently using this configuration.
     *   Encoding                    1         0: G711, 1: G726, 2: AAC, 3: G729, 4: MPEG1, 5: MPEG2, 6: AC3, 7: PCM, 8: ADPCM
     *   Bitrate                     2         The output bitrate in kbps.
     *   SampleRate                  2         The output sample rate in 100Hz.
     * CheckSum                    1         =(byte[0]+...+byte[N]) MOD 256
     **/
    GetAudioEncoderConfigurationsOutput_INFO  *pInfo = (GetAudioEncoderConfigurationsOutput_INFO*)input_info;
    uint32_t                                  cmd_length = 0;
    uint8_t                                   *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;
        uint32_t    i = 0;

        // internal malloc need to include check_sum size
        cmd_length = 9 + (4+pInfo->user_name.length) + (4+pInfo->password.length);
        for(i = 0; i < pInfo->configurations_list_size; i++)
        {
            cmd_length += (6
                           + (4+pInfo->pAudio_enc_configurations[i].token.length)
                           + (4+pInfo->pAudio_enc_configurations[i].name.length));
        }

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
        _SET_WORD(pCur, CMD_GetAudioEncoderConfigurationsOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_BYTE(pCur, pInfo->configurations_list_size);

        for(i = 0; i < pInfo->configurations_list_size; i++)
        {
            _SET_DWORD(pCur, pInfo->pAudio_enc_configurations[i].token.length);
            _SET_STRING(pCur, pInfo->pAudio_enc_configurations[i].token.pStream, pInfo->pAudio_enc_configurations[i].token.length);

            _SET_DWORD(pCur, pInfo->pAudio_enc_configurations[i].name.length);
            _SET_STRING(pCur, pInfo->pAudio_enc_configurations[i].name.pStream, pInfo->pAudio_enc_configurations[i].name.length);

            _SET_BYTE(pCur, pInfo->pAudio_enc_configurations[i].use_count);
            _SET_BYTE(pCur, pInfo->pAudio_enc_configurations[i].encoding);
            _SET_WORD(pCur, pInfo->pAudio_enc_configurations[i].bitrate);
            _SET_WORD(pCur, pInfo->pAudio_enc_configurations[i].sample_rate);
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
_GetAudioEncoderConfigurationsOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field                Length(Byte)    Descriptions
     * Command Length              4         The total length of this command. It doesn't include the CheckSum.
     * Command Code                2         Code: 0x8407
     * Return Code                 1         0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name                   n         [string] User name for security.
     * Password                    n         [string] Password for security.
     * Configurations List size    1         This element contains a list of audio encoder configurations. The following 6 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All Configuration parameters }.
     *   Token                       n         [string] Token that uniquely refernces this configuration.
     *   Name                        n         [string] User readable name
     *   UseCount                    1         Number of internal references currently using this configuration.
     *   Encoding                    1         0: G711, 1: G726, 2: AAC, 3: G729, 4: MPEG1, 5: MPEG2, 6: AC3, 7: PCM, 8: ADPCM
     *   Bitrate                     2         The output bitrate in kbps.
     *   SampleRate                  2         The output sample rate in 100Hz.
     * CheckSum                    1         =(byte[0]+...+byte[N]) MOD 256
     **/
    GetAudioEncoderConfigurationsOutput_INFO   *pInfo = (GetAudioEncoderConfigurationsOutput_INFO*)pStruct_Info;

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

        pInfo->configurations_list_size = _GET_BYTE(pCur);           pCur += 1;

        pInfo->pAudio_enc_configurations = tscm_malloc(sizeof(AUDIO_ENC_CONFIGURATIONS) * pInfo->configurations_list_size);
        if( !pInfo->pAudio_enc_configurations )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }
        memset(pInfo->pAudio_enc_configurations, 0x0, sizeof(AUDIO_ENC_CONFIGURATIONS) * pInfo->configurations_list_size);

        for(i = 0; i < pInfo->configurations_list_size; i++)
        {
            pInfo->pAudio_enc_configurations[i].token.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pAudio_enc_configurations[i].token.pStream = pCur;              pCur += pInfo->pAudio_enc_configurations[i].token.length;

            pInfo->pAudio_enc_configurations[i].name.length  = _GET_DWORD(pCur);  pCur += 4;
            pInfo->pAudio_enc_configurations[i].name.pStream = pCur;              pCur += pInfo->pAudio_enc_configurations[i].name.length;

            pInfo->pAudio_enc_configurations[i].use_count   = _GET_BYTE(pCur); pCur += 1;
            pInfo->pAudio_enc_configurations[i].encoding    = _GET_BYTE(pCur); pCur += 1;
            pInfo->pAudio_enc_configurations[i].bitrate     = _GET_WORD(pCur); pCur += 2;
            pInfo->pAudio_enc_configurations[i].sample_rate = _GET_WORD(pCur); pCur += 2;
        }
    }while(0);
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(GetAudioEncoderConfigurationsOutput_Cmd_Pkt_Decode, GetAudioEncoderConfigurationsOutput_INFO, _GetAudioEncoderConfigurationsOutput_dec);

static uint32_t
_GetAudioEncoderConfigurationsOutput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            GetAudioEncoderConfigurationsOutput_INFO   *pInfo = (GetAudioEncoderConfigurationsOutput_INFO*)(*((uint8_t**)input_info));
            if( pInfo->pAudio_enc_configurations )     free(pInfo->pAudio_enc_configurations);
        }

        free((*((uint8_t**)input_info)));
        (*((uint8_t**)input_info)) = 0;
    }while(0);

    return 0;
}
//////////////////
static uint32_t
SetSynchronizationPointInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field         Length(Byte)    Descriptions
     * Command Length  4               The total length of this command. It doesn't include the CheckSum.
     * Command Code    2               Code: 0x0480
     * Reserved        1
     * User Name       n               [string] User name for security.
     * Password        n               [string] Password for security.
     * ProfileToken    n               [string] Contains a Profile reference for which a Synchronization Point is requested.
     * CheckSum        1               =(byte[0]+...+byte[N]) MOD 256
     **/
    SetSynchronizationPointInput_INFO  *pInfo = (SetSynchronizationPointInput_INFO*)input_info;
    uint32_t                           cmd_length = 0;
    uint8_t                            *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length) + (4+pInfo->profile_token.length);

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
        _SET_WORD(pCur, CMD_SetSynchronizationPointInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->profile_token.length);
        _SET_STRING(pCur, pInfo->profile_token.pStream, pInfo->profile_token.length);
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
_SetSynchronizationPointInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field         Length(Byte)    Descriptions
     * Command Length  4               The total length of this command. It doesn't include the CheckSum.
     * Command Code    2               Code: 0x0480
     * Reserved        1
     * User Name       n               [string] User name for security.
     * Password        n               [string] Password for security.
     * ProfileToken    n               [string] Contains a Profile reference for which a Synchronization Point is requested.
     * CheckSum        1               =(byte[0]+...+byte[N]) MOD 256
     **/
    SetSynchronizationPointInput_INFO   *pInfo = (SetSynchronizationPointInput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->reserved = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->profile_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->profile_token.pStream = pCur;              pCur += pInfo->profile_token.length;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetSynchronizationPointInput_Cmd_Pkt_Decode, SetSynchronizationPointInput_INFO, _SetSynchronizationPointInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8480
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(SetSynchronizationPointOutput_Cmd_Ctxt_New, SetSynchronizationPointOutput_INFO, CMD_SetSynchronizationPointOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_SetSynchronizationPointOutput_dec, SetSynchronizationPointOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(SetSynchronizationPointOutput_Cmd_Pkt_Decode, SetSynchronizationPointOutput_INFO, _SetSynchronizationPointOutput_dec);

//////////////////
static uint32_t
SetVideoSourceConfigurationInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0482
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] User readable name.
     * token               n           [string] Token that uniquely refernces this configuration.
     * SourceToken         n           [string] Reference to the physical input.
     * UseCount            1           Number of internal references currently using this configuration.
     * Bounds x            2           Rectangle specifying the Video capturing area. The capturing area shall not be larger than the whole Video source area.
     * Bounds y            2
     * Bounds width        2
     * Bounds height       2
     * Rotate Mode         1           0: OFF, 1: ON, 2: AUTO.
     * Rotate Degree       2           Optional parameter to configure how much degree of clockwise rotation of image for On mode. Omitting this parameter for On mode means 180 degree rotation.
     * Mirror Mode         1           0: OFF, 1: Horizontal, 2: Vertical, 3: Both
     * ForcePersistence    1           Determines if the configuration changes shall be stored and remain after reboot.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetVideoSourceConfigurationInput_INFO  *pInfo = (SetVideoSourceConfigurationInput_INFO*)input_info;
    uint32_t                               cmd_length = 0;
    uint8_t                                *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 22 + (4+pInfo->user_name.length)
                        + (4+pInfo->password.length)
                        + (4+pInfo->name.length)
                        + (4+pInfo->token.length)
                        + (4+pInfo->source_token.length);

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
        _SET_WORD(pCur, CMD_SetVideoSourceConfigurationInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->name.length);
        _SET_STRING(pCur, pInfo->name.pStream, pInfo->name.length);

        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);

        _SET_DWORD(pCur, pInfo->source_token.length);
        _SET_STRING(pCur, pInfo->source_token.pStream, pInfo->source_token.length);

        _SET_BYTE(pCur, pInfo->use_count);
        _SET_WORD(pCur, pInfo->bounds_x);
        _SET_WORD(pCur, pInfo->bounds_y);
        _SET_WORD(pCur, pInfo->bounds_width);
        _SET_WORD(pCur, pInfo->bounds_height);
        _SET_BYTE(pCur, pInfo->rotate_mode);
        _SET_WORD(pCur, pInfo->rotate_degree);
        _SET_BYTE(pCur, pInfo->mirror_mode);
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
_SetVideoSourceConfigurationInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0482
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] User readable name.
     * token               n           [string] Token that uniquely refernces this configuration.
     * SourceToken         n           [string] Reference to the physical input.
     * UseCount            1           Number of internal references currently using this configuration.
     * Bounds x            2           Rectangle specifying the Video capturing area. The capturing area shall not be larger than the whole Video source area.
     * Bounds y            2
     * Bounds width        2
     * Bounds height       2
     * Rotate Mode         1           0: OFF, 1: ON, 2: AUTO.
     * Rotate Degree       2           Optional parameter to configure how much degree of clockwise rotation of image for On mode. Omitting this parameter for On mode means 180 degree rotation.
     * Mirror Mode         1           0: OFF, 1: Horizontal, 2: Vertical, 3: Both
     * ForcePersistence    1           Determines if the configuration changes shall be stored and remain after reboot.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetVideoSourceConfigurationInput_INFO   *pInfo = (SetVideoSourceConfigurationInput_INFO*)pStruct_Info;

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

    pInfo->token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->token.pStream = pCur;              pCur += pInfo->token.length;

    pInfo->source_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->source_token.pStream = pCur;              pCur += pInfo->source_token.length;

    pInfo->use_count         = _GET_BYTE(pCur); pCur += 1;
    pInfo->bounds_x          = _GET_WORD(pCur); pCur += 2;
    pInfo->bounds_y          = _GET_WORD(pCur); pCur += 2;
    pInfo->bounds_width      = _GET_WORD(pCur); pCur += 2;
    pInfo->bounds_height     = _GET_WORD(pCur); pCur += 2;
    pInfo->rotate_mode       = _GET_BYTE(pCur); pCur += 1;
    pInfo->rotate_degree     = _GET_WORD(pCur); pCur += 2;
    pInfo->mirror_mode       = _GET_BYTE(pCur); pCur += 1;
    pInfo->force_persistence = _GET_BYTE(pCur); pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetVideoSourceConfigurationInput_Cmd_Pkt_Decode, SetVideoSourceConfigurationInput_INFO, _SetVideoSourceConfigurationInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8482
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 2
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(SetVideoSourceConfigurationOutput_Cmd_Ctxt_New, SetVideoSourceConfigurationOutput_INFO, CMD_SetVideoSourceConfigurationOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_SetVideoSourceConfigurationOutput_dec, SetVideoSourceConfigurationOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(SetVideoSourceConfigurationOutput_Cmd_Pkt_Decode, SetVideoSourceConfigurationOutput_INFO, _SetVideoSourceConfigurationOutput_dec);

//////////////////
static uint32_t
SetVideoEncoderConfigurationInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0484
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] User readable name.
     * token               n           [string] Token that uniquely refernces this configuration.
     * UseCount            1           Number of internal references currently using this configuration.
     * Encoding            1           0: JPEG, 1: MPEG4, 2: H264, 3: MPEG2.
     * Resolution Width    2           Number of the columns of the Video image.
     * Resolution Height   2           Number of the lines of the Video image.
     * Quality             1           Relative value for the video quantizers and the quality of the video. A high value within supported quality range means higher quality.
     * FrameRateLimit      1           Maximum output framerate in fps. If an EncodingInterval is provided the resulting encoded framerate will be reduced by the given factor.
     * EncodingInterval    1           Interval at which images are encoded and transmitted. (A value of 1 means that every frame is encoded, a value of 2 means that every 2nd frame is encoded ...)
     * BitrateLimit        2           the maximum output bitrate in kbps
     * RateControlType     1           0: CBR
     * GovLength           1           Determines the interval in which the I-Frames will be coded.
     * Profile             1           For MPEG4:
     * ForcePersistence    1           Determines if the configuration changes shall be stored and remain after reboot.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetVideoEncoderConfigurationInput_INFO  *pInfo = (SetVideoEncoderConfigurationInput_INFO*)input_info;
    uint32_t                                cmd_length = 0;
    uint8_t                                 *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 23 + (4+pInfo->user_name.length)
                        + (4+pInfo->password.length)
                        + (4+pInfo->name.length)
                        + (4+pInfo->token.length);

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
        _SET_WORD(pCur, CMD_SetVideoEncoderConfigurationInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->name.length);
        _SET_STRING(pCur, pInfo->name.pStream, pInfo->name.length);

        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);

        _SET_BYTE(pCur, pInfo->use_count);
        _SET_BYTE(pCur, pInfo->encoding);
        _SET_WORD(pCur, pInfo->resolution_width);
        _SET_WORD(pCur, pInfo->resolution_height);
        _SET_BYTE(pCur, pInfo->quality);
        _SET_BYTE(pCur, pInfo->frame_rate_limit);
        _SET_BYTE(pCur, pInfo->encoding_interval);
        _SET_WORD(pCur, pInfo->bitrate_limit);
        _SET_BYTE(pCur, pInfo->rate_control_type);
        _SET_BYTE(pCur, pInfo->gov_length);
        _SET_BYTE(pCur, pInfo->profile);
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
_SetVideoEncoderConfigurationInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0484
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] User readable name.
     * token               n           [string] Token that uniquely refernces this configuration.
     * UseCount            1           Number of internal references currently using this configuration.
     * Encoding            1           0: JPEG, 1: MPEG4, 2: H264, 3: MPEG2.
     * Resolution Width    2           Number of the columns of the Video image.
     * Resolution Height   2           Number of the lines of the Video image.
     * Quality             1           Relative value for the video quantizers and the quality of the video. A high value within supported quality range means higher quality.
     * FrameRateLimit      1           Maximum output framerate in fps. If an EncodingInterval is provided the resulting encoded framerate will be reduced by the given factor.
     * EncodingInterval    1           Interval at which images are encoded and transmitted. (A value of 1 means that every frame is encoded, a value of 2 means that every 2nd frame is encoded ...)
     * BitrateLimit        2           the maximum output bitrate in kbps
     * RateControlType     1           0: CBR
     * GovLength           1           Determines the interval in which the I-Frames will be coded.
     * Profile             1           For MPEG4:
     * ForcePersistence    1           Determines if the configuration changes shall be stored and remain after reboot.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetVideoEncoderConfigurationInput_INFO   *pInfo = (SetVideoEncoderConfigurationInput_INFO*)pStruct_Info;

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

    pInfo->token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->token.pStream = pCur;              pCur += pInfo->token.length;

    pInfo->use_count         = _GET_BYTE(pCur); pCur += 1;
    pInfo->encoding          = _GET_BYTE(pCur); pCur += 1;
    pInfo->resolution_width  = _GET_WORD(pCur); pCur += 2;
    pInfo->resolution_height = _GET_WORD(pCur); pCur += 2;
    pInfo->quality           = _GET_BYTE(pCur); pCur += 1;
    pInfo->frame_rate_limit  = _GET_BYTE(pCur); pCur += 1;
    pInfo->encoding_interval = _GET_BYTE(pCur); pCur += 1;
    pInfo->bitrate_limit     = _GET_WORD(pCur); pCur += 2;
    pInfo->rate_control_type = _GET_BYTE(pCur); pCur += 1;
    pInfo->gov_length        = _GET_BYTE(pCur); pCur += 1;
    pInfo->profile           = _GET_BYTE(pCur); pCur += 1;
    pInfo->force_persistence = _GET_BYTE(pCur); pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetVideoEncoderConfigurationInput_Cmd_Pkt_Decode, SetVideoEncoderConfigurationInput_INFO, _SetVideoEncoderConfigurationInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8484
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 2
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(SetVideoEncoderConfigurationOutput_Cmd_Ctxt_New, SetVideoEncoderConfigurationOutput_INFO, CMD_SetVideoEncoderConfigurationOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_SetVideoEncoderConfigurationOutput_dec, SetVideoEncoderConfigurationOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(SetVideoEncoderConfigurationOutput_Cmd_Pkt_Decode, SetVideoEncoderConfigurationOutput_INFO, _SetVideoEncoderConfigurationOutput_dec);

//////////////////
static uint32_t
SetAudioSourceConfigurationInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0486
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] User readable name.
     * token               n           [string] Token that uniquely refernces this configuration.
     * SourceToken         n           [string] Token of the Audio Source the configuration applies to
     * UseCount            1           Number of internal references currently using this configuration.
     * ForcePersistence    1           Determines if the configuration changes shall be stored and remain after reboot.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetAudioSourceConfigurationInput_INFO  *pInfo = (SetAudioSourceConfigurationInput_INFO*)input_info;
    uint32_t                               cmd_length = 0;
    uint8_t                                *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 10 + (4+pInfo->user_name.length)
                        + (4+pInfo->password.length)
                        + (4+pInfo->name.length)
                        + (4+pInfo->token.length)
                        + (4+pInfo->source_token.length);

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
        _SET_WORD(pCur, CMD_SetAudioSourceConfigurationInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->name.length);
        _SET_STRING(pCur, pInfo->name.pStream, pInfo->name.length);

        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);

        _SET_DWORD(pCur, pInfo->source_token.length);
        _SET_STRING(pCur, pInfo->source_token.pStream, pInfo->source_token.length);

        _SET_BYTE(pCur, pInfo->use_count);
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
_SetAudioSourceConfigurationInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0486
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] User readable name.
     * token               n           [string] Token that uniquely refernces this configuration.
     * SourceToken         n           [string] Token of the Audio Source the configuration applies to
     * UseCount            1           Number of internal references currently using this configuration.
     * ForcePersistence    1           Determines if the configuration changes shall be stored and remain after reboot.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetAudioSourceConfigurationInput_INFO   *pInfo = (SetAudioSourceConfigurationInput_INFO*)pStruct_Info;

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

    pInfo->token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->token.pStream = pCur;              pCur += pInfo->token.length;

    pInfo->source_token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->source_token.pStream = pCur;              pCur += pInfo->source_token.length;

    pInfo->use_count         = _GET_BYTE(pCur);  pCur += 1;
    pInfo->force_persistence = _GET_BYTE(pCur);  pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetAudioSourceConfigurationInput_Cmd_Pkt_Decode, SetAudioSourceConfigurationInput_INFO, _SetAudioSourceConfigurationInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8486
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 2
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(SetAudioSourceConfigurationOutput_Cmd_Ctxt_New, SetAudioSourceConfigurationOutput_INFO, CMD_SetAudioSourceConfigurationOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_SetAudioSourceConfigurationOutput_dec, SetAudioSourceConfigurationOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(SetAudioSourceConfigurationOutput_Cmd_Pkt_Decode, SetAudioSourceConfigurationOutput_INFO, _SetAudioSourceConfigurationOutput_dec);

//////////////////
static uint32_t
SetAudioEncoderConfigurationInput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)   Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0487
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] User readable name.
     * token               n           [string] Token that uniquely refernces this configuration.
     * UseCount            1           Number of internal references currently using this configuration.
     * Encoding            1           0: G711, 1: G726, 2: AAC, 3: G729, 4: MPEG1, 5: MPEG2, 6: AC3, 7: PCM, 8: ADPCM
     * Bitrate             2           The output bitrate in kbps.
     * SampleRate          2           The output sample rate in 100Hz.
     * ForcePersistence    1           Determines if the configuration changes shall be stored and remain after reboot.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetAudioEncoderConfigurationInput_INFO  *pInfo = (SetAudioEncoderConfigurationInput_INFO*)input_info;
    uint32_t                                cmd_length = 0;
    uint8_t                                 *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = 15 + (4+pInfo->user_name.length)
                        + (4+pInfo->password.length)
                        + (4+pInfo->name.length)
                        + (4+pInfo->token.length);

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
        _SET_WORD(pCur, CMD_SetAudioEncoderConfigurationInput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->reserved);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_DWORD(pCur, pInfo->name.length);
        _SET_STRING(pCur, pInfo->name.pStream, pInfo->name.length);

        _SET_DWORD(pCur, pInfo->token.length);
        _SET_STRING(pCur, pInfo->token.pStream, pInfo->token.length);

        _SET_BYTE(pCur, pInfo->use_count);
        _SET_BYTE(pCur, pInfo->encoding);
        _SET_WORD(pCur, pInfo->bitrate);
        _SET_WORD(pCur, pInfo->sample_rate);
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
_SetAudioEncoderConfigurationInput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)   Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x0487
     * Reserved            1
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Name                n           [string] User readable name.
     * token               n           [string] Token that uniquely refernces this configuration.
     * UseCount            1           Number of internal references currently using this configuration.
     * Encoding            1           0: G711, 1: G726, 2: AAC, 3: G729, 4: MPEG1, 5: MPEG2, 6: AC3, 7: PCM, 8: ADPCM
     * Bitrate             2           The output bitrate in kbps.
     * SampleRate          2           The output sample rate in 100Hz.
     * ForcePersistence    1           Determines if the configuration changes shall be stored and remain after reboot.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    SetAudioEncoderConfigurationInput_INFO   *pInfo = (SetAudioEncoderConfigurationInput_INFO*)pStruct_Info;

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

    pInfo->token.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->token.pStream = pCur;              pCur += pInfo->token.length;

    pInfo->use_count         = _GET_BYTE(pCur);  pCur += 1;
    pInfo->encoding          = _GET_BYTE(pCur);  pCur += 1;
    pInfo->bitrate           = _GET_WORD(pCur);  pCur += 2;
    pInfo->sample_rate       = _GET_WORD(pCur);  pCur += 2;
    pInfo->force_persistence = _GET_BYTE(pCur);  pCur += 1;
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(SetAudioEncoderConfigurationInput_Cmd_Pkt_Decode, SetAudioEncoderConfigurationInput_INFO, _SetAudioEncoderConfigurationInput_dec);

//////////////////
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0x8487
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail,
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 2
     **/
CMD_PKT_BASIC_CTXT_NEW_INSTANCE(SetAudioEncoderConfigurationOutput_Cmd_Ctxt_New, SetAudioEncoderConfigurationOutput_INFO, CMD_SetAudioEncoderConfigurationOutput, return_code);
CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(_SetAudioEncoderConfigurationOutput_dec, SetAudioEncoderConfigurationOutput_INFO, return_code);
CMD_PKT_DECODE_INSTANCE(SetAudioEncoderConfigurationOutput_Cmd_Pkt_Decode, SetAudioEncoderConfigurationOutput_INFO, _SetAudioEncoderConfigurationOutput_dec);



//=============================================================================
//                Public Function Definition
//=============================================================================

// DEFINE_CMD_PKT_CODEC(CMD_xxxInput, xxxInput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
// DEFINE_CMD_PKT_CODEC(CMD_xxxOutput, xxxOutput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetProfilesInput, GetProfilesInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetProfilesInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetProfilesOutput, GetProfilesOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetProfilesOutput_Cmd_Pkt_Decode, _GetProfilesOutput_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetVideoSourcesInput, GetVideoSourcesInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetVideoSourcesInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetVideoSourcesOutput, GetVideoSourcesOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetVideoSourcesOutput_Cmd_Pkt_Decode, _GetVideoSourcesOutput_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetVideoSourceConfigurationsInput, GetVideoSourceConfigurationsInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetVideoSourceConfigurationsInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetVideoSourceConfigurationsOutput, GetVideoSourceConfigurationsOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetVideoSourceConfigurationsOutput_Cmd_Pkt_Decode, _GetVideoSourceConfigurationsOutput_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetGuaranteedNumberOfVideoEncoderInstancesInput, GetGuaranteedNumberOfVideoEncoderInstancesInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetGuaranteedNumberOfVideoEncoderInstancesInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetGuaranteedNumberOfVideoEncoderInstancesOutput, GetGuaranteedNumberOfVideoEncoderInstancesOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetGuaranteedNumberOfVideoEncoderInstancesOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetVideoEncoderConfigurationsInput, GetVideoEncoderConfigurationsInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetVideoEncoderConfigurationsInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetVideoEncoderConfigurationsOutput, GetVideoEncoderConfigurationsOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetVideoEncoderConfigurationsOutput_Cmd_Pkt_Decode, _GetVideoEncoderConfigurationsOutput_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetAudioSourcesInput, GetAudioSourcesInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetAudioSourcesInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetAudioSourcesOutput, GetAudioSourcesOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetAudioSourcesOutput_Cmd_Pkt_Decode, _GetAudioSourcesOutput_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetAudioSourceConfigurationsInput, GetAudioSourceConfigurationsInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetAudioSourceConfigurationsInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetAudioSourceConfigurationsOutput, GetAudioSourceConfigurationsOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetAudioSourceConfigurationsOutput_Cmd_Pkt_Decode, _GetAudioSourceConfigurationsOutput_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_GetAudioEncoderConfigurationsInput, GetAudioEncoderConfigurationsInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetAudioEncoderConfigurationsInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_GetAudioEncoderConfigurationsOutput, GetAudioEncoderConfigurationsOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, GetAudioEncoderConfigurationsOutput_Cmd_Pkt_Decode, _GetAudioEncoderConfigurationsOutput_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetSynchronizationPointInput, SetSynchronizationPointInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetSynchronizationPointInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetSynchronizationPointOutput, SetSynchronizationPointOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetSynchronizationPointOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetVideoSourceConfigurationInput, SetVideoSourceConfigurationInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetVideoSourceConfigurationInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetVideoSourceConfigurationOutput, SetVideoSourceConfigurationOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetVideoSourceConfigurationOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetVideoEncoderConfigurationInput, SetVideoEncoderConfigurationInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetVideoEncoderConfigurationInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetVideoEncoderConfigurationOutput, SetVideoEncoderConfigurationOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetVideoEncoderConfigurationOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetAudioSourceConfigurationInput, SetAudioSourceConfigurationInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetAudioSourceConfigurationInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetAudioSourceConfigurationOutput, SetAudioSourceConfigurationOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetAudioSourceConfigurationOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_SetAudioEncoderConfigurationInput, SetAudioEncoderConfigurationInput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetAudioEncoderConfigurationInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
DEFINE_CMD_PKT_CODEC(CMD_SetAudioEncoderConfigurationOutput, SetAudioEncoderConfigurationOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, SetAudioEncoderConfigurationOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


