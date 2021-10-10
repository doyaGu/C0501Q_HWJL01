

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
#define _GET_PARAM_SIZE(size, struct_name)                                                      \
            do{ struct_name  *pParam = (struct_name*)pInfo->pParameters;                        \
                size = sizeof(struct_name) + sizeof(POLYGON_POINT) * pParam->rule_token.length; \
            }while(0)
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
MetadataStreamOutput_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0xF000
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Version             2           ccHDtv return channel version
     * Type                1           Type of the configuration represented by a unique ID.
     * Parameters          N           Type=0x01: Device Information
     *                                 Type=0x02: Stream Information
     *                                 Type=0x10~0x14: RuleEngine Items
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    MetadataStreamOutput_INFO  *pInfo = (MetadataStreamOutput_INFO*)input_info;
    uint32_t                   cmd_length = 0;
    uint8_t                    *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;
        uint32_t    param_size = 0;

        // internal malloc need to include check_sum size
        cmd_length = 11 + (4+pInfo->user_name.length) + (4+pInfo->password.length);

        switch( pInfo->type )
        {
            case METADATA_DEVICE_INFO:
                {
                    DEV_INFO_PARAM  *pParam = (DEV_INFO_PARAM*)pInfo->pParameters;
                    param_size = sizeof(DEV_INFO_PARAM) + pParam->text_info.length;
                }
                break;
            case METADATA_STREAM_INFO:
                {
                    STREAM_INFO_PARAMS  *pParam = (STREAM_INFO_PARAMS*)pInfo->pParameters;
                    param_size = 1 + (19 * pParam->stream_information_list_size);
                }
                break;
            case METADATA_LINE_DETECTOR:                _GET_PARAM_SIZE(param_size, LINE_DETECTOR_METADATA);                break;
            case METADATA_FIELD_DETECTOR:               _GET_PARAM_SIZE(param_size, FIELD_DETECTOR_METADATA);               break;
            case METADATA_DECLARATIVE_MOTION_DETECTOR:  _GET_PARAM_SIZE(param_size, DECLARATIVE_MOTION_DETECTOR_METADATA);  break;
            case METADATA_COUNTING_RULE:                _GET_PARAM_SIZE(param_size, COUNTING_RULE_METADATA);                break;
            case METADATA_CELL_MOTION_DETECTOR:         _GET_PARAM_SIZE(param_size, CELL_MOTION_DETECTOR_METADATA);         break;
        }

        cmd_length += param_size;
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
        _SET_WORD(pCur, CMD_MetadataStreamOutput);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, pInfo->return_code);

        _SET_DWORD(pCur, pInfo->user_name.length);
        _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);

        _SET_DWORD(pCur, pInfo->password.length);
        _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);

        _SET_WORD(pCur, pInfo->version);

        _SET_BYTE(pCur, (uint8_t)(pInfo->type&0xFF));

        switch( pInfo->type )
        {
            case METADATA_DEVICE_INFO:
                {
                    DEV_INFO_PARAM  *pParam = (DEV_INFO_PARAM*)pInfo->pParameters;

                    _SET_WORD(pCur, pParam->dev_vendor_id);
                    _SET_WORD(pCur, pParam->dev_model_id);
                    _SET_WORD(pCur, pParam->hw_version_code);
                    _SET_DWORD(pCur, pParam->sw_version_code);

                    _SET_DWORD(pCur, pParam->text_info.length);
                    _SET_STRING(pCur, pParam->text_info.pStream, pParam->text_info.length);
                }
                break;
            case METADATA_STREAM_INFO:
                {
                    uint32_t            i = 0;
                    STREAM_INFO_PARAMS  *pParam = (STREAM_INFO_PARAMS*)pInfo->pParameters;

                    _SET_BYTE(pCur, pParam->stream_information_list_size);

                    for(i = 0; i < pParam->stream_information_list_size; i++)
                    {
                        _SET_WORD(pCur, pParam->pStream_info_data[i].video_pid);
                        _SET_BYTE(pCur, pParam->pStream_info_data[i].video_encoding_type);
                        _SET_WORD(pCur, pParam->pStream_info_data[i].video_resolution_width);
                        _SET_WORD(pCur, pParam->pStream_info_data[i].video_resolution_height);
                        _SET_BYTE(pCur, pParam->pStream_info_data[i].video_framerate);
                        _SET_WORD(pCur, pParam->pStream_info_data[i].video_bitrate);
                        _SET_WORD(pCur, pParam->pStream_info_data[i].audio_pid);
                        _SET_BYTE(pCur, pParam->pStream_info_data[i].audio_encoding_type);
                        _SET_WORD(pCur, pParam->pStream_info_data[i].audio_bitrate);
                        _SET_WORD(pCur, pParam->pStream_info_data[i].audio_samplerate);
                        _SET_WORD(pCur, pParam->pStream_info_data[i].pcr_pid);
                    }
                }
                break;
            case METADATA_LINE_DETECTOR:
                {
                    LINE_DETECTOR_METADATA  *pParam = (LINE_DETECTOR_METADATA*)pInfo->pParameters;

                    _SET_DWORD(pCur, pParam->rule_token.length);
                    _SET_STRING(pCur, pParam->rule_token.pStream, pParam->rule_token.length);

                    _SET_DWORD(pCur, pParam->object_id);
                    _SET_BYTE(pCur, pParam->utc_hour);
                    _SET_BYTE(pCur, pParam->utc_minute);
                    _SET_BYTE(pCur, pParam->utc_second);
                    _SET_WORD(pCur, pParam->utc_year);
                    _SET_BYTE(pCur, pParam->utc_month);
                    _SET_BYTE(pCur, pParam->utc_day);
                }
                break;
            case METADATA_FIELD_DETECTOR:
                {
                    FIELD_DETECTOR_METADATA  *pParam = (FIELD_DETECTOR_METADATA*)pInfo->pParameters;

                    _SET_DWORD(pCur, pParam->rule_token.length);
                    _SET_STRING(pCur, pParam->rule_token.pStream, pParam->rule_token.length);

                    _SET_DWORD(pCur, pParam->object_id);
                    _SET_BYTE(pCur, pParam->is_inside);
                    _SET_BYTE(pCur, pParam->utc_hour);
                    _SET_BYTE(pCur, pParam->utc_minute);
                    _SET_BYTE(pCur, pParam->utc_second);
                    _SET_WORD(pCur, pParam->utc_year);
                    _SET_BYTE(pCur, pParam->utc_month);
                    _SET_BYTE(pCur, pParam->utc_day);
                }
                break;
            case METADATA_DECLARATIVE_MOTION_DETECTOR:
                {
                    DECLARATIVE_MOTION_DETECTOR_METADATA  *pParam = (DECLARATIVE_MOTION_DETECTOR_METADATA*)pInfo->pParameters;

                    _SET_DWORD(pCur, pParam->rule_token.length);
                    _SET_STRING(pCur, pParam->rule_token.pStream, pParam->rule_token.length);

                    _SET_DWORD(pCur, pParam->object_id);
                    _SET_BYTE(pCur, pParam->utc_hour);
                    _SET_BYTE(pCur, pParam->utc_minute);
                    _SET_BYTE(pCur, pParam->utc_second);
                    _SET_WORD(pCur, pParam->utc_year);
                    _SET_BYTE(pCur, pParam->utc_month);
                    _SET_BYTE(pCur, pParam->utc_day);
                }
                break;
            case METADATA_COUNTING_RULE:
                {
                    COUNTING_RULE_METADATA  *pParam = (COUNTING_RULE_METADATA*)pInfo->pParameters;

                    _SET_DWORD(pCur, pParam->rule_token.length);
                    _SET_STRING(pCur, pParam->rule_token.pStream, pParam->rule_token.length);

                    _SET_DWORD(pCur, pParam->object_id);
                    _SET_DWORD(pCur, pParam->count);
                    _SET_BYTE(pCur, pParam->utc_hour);
                    _SET_BYTE(pCur, pParam->utc_minute);
                    _SET_BYTE(pCur, pParam->utc_second);
                    _SET_WORD(pCur, pParam->utc_year);
                    _SET_BYTE(pCur, pParam->utc_month);
                    _SET_BYTE(pCur, pParam->utc_day);
                }
                break;
            case METADATA_CELL_MOTION_DETECTOR:
                {
                    CELL_MOTION_DETECTOR_METADATA  *pParam = (CELL_MOTION_DETECTOR_METADATA*)pInfo->pParameters;

                    _SET_DWORD(pCur, pParam->rule_token.length);
                    _SET_STRING(pCur, pParam->rule_token.pStream, pParam->rule_token.length);

                    _SET_BYTE(pCur, pParam->is_motion);
                    _SET_BYTE(pCur, pParam->utc_hour);
                    _SET_BYTE(pCur, pParam->utc_minute);
                    _SET_BYTE(pCur, pParam->utc_second);
                    _SET_WORD(pCur, pParam->utc_year);
                    _SET_BYTE(pCur, pParam->utc_month);
                    _SET_BYTE(pCur, pParam->utc_day);
                }
                break;
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
_MetadataStreamOutput_dec(
    uint8_t     *pCur,
    void        *pStruct_Info)
{
    /**
     * Field           Length(Byte)    Descriptions
     * Command Length      4           The total length of this command. It doesn't include the CheckSum.
     * Command Code        2           Code: 0xF000
     * Return Code         1           0: Success, 0xFB: Username error, 0xFC: Password error, 0xFD: unsupported, 0xFE: Cheksum Err, 0xFF: Fail.
     * User Name           n           [string] User name for security.
     * Password            n           [string] Password for security.
     * Version             2           ccHDtv return channel version
     * Type                1           Type of the configuration represented by a unique ID.
     * Parameters          N           Type=0x01: Device Information
     *                                 Type=0x02: Stream Information
     *                                 Type=0x10~0x14: RuleEngine Items
     * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
     **/
    MetadataStreamOutput_INFO   *pInfo = (MetadataStreamOutput_INFO*)pStruct_Info;

    pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;

    //-----------------------------------
    // personal Cmd Data
    pInfo->return_code = _GET_BYTE(pCur);           pCur += 1;

    pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;
    pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;

    pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;
    pInfo->password.pStream = pCur;              pCur += pInfo->password.length;

    pInfo->version = _GET_WORD(pCur); pCur += 2;
    pInfo->type    = _GET_BYTE(pCur); pCur += 1;

    switch( pInfo->type )
    {
        case METADATA_DEVICE_INFO:
            {
                DEV_INFO_PARAM  *pParam = 0;

                pInfo->pParameters = tscm_malloc(sizeof(DEV_INFO_PARAM));
                if( !pInfo->pParameters )
                {
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                    break;
                }
                memset(pInfo->pParameters, 0x0, sizeof(DEV_INFO_PARAM));

                pParam = (DEV_INFO_PARAM*)pInfo->pParameters;
                pParam->dev_vendor_id   = _GET_WORD(pCur);  pCur += 2;
                pParam->dev_model_id    = _GET_WORD(pCur);  pCur += 2;
                pParam->hw_version_code = _GET_WORD(pCur);  pCur += 2;
                pParam->sw_version_code = _GET_DWORD(pCur); pCur += 4;

                pParam->text_info.length  = _GET_DWORD(pCur); pCur += 4;
                pParam->text_info.pStream = pCur;             pCur += pParam->text_info.length;
            }
            break;
        case METADATA_STREAM_INFO:
            {
                uint32_t            i = 0;
                STREAM_INFO_PARAMS  *pParam = 0;

                pInfo->pParameters = tscm_malloc(sizeof(STREAM_INFO_PARAMS));
                if( !pInfo->pParameters )
                {
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                    break;
                }
                memset(pInfo->pParameters, 0x0, sizeof(STREAM_INFO_PARAMS));

                pParam = (STREAM_INFO_PARAMS*)pInfo->pParameters;

                pParam->stream_information_list_size = _GET_BYTE(pCur);  pCur += 1;
                pParam->pStream_info_data = tscm_malloc(sizeof(STREAM_INFO_DATA) * pParam->stream_information_list_size);
                if( !pParam->pStream_info_data)
                {
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                    break;
                }
                memset(pParam->pStream_info_data, 0x0, sizeof(STREAM_INFO_DATA) * pParam->stream_information_list_size);

                for(i = 0; i < pParam->stream_information_list_size; i++)
                {
                    pParam->pStream_info_data[i].video_pid                    = _GET_WORD(pCur);  pCur += 2;
                    pParam->pStream_info_data[i].video_encoding_type          = _GET_BYTE(pCur);  pCur += 1;
                    pParam->pStream_info_data[i].video_resolution_width       = _GET_WORD(pCur);  pCur += 2;
                    pParam->pStream_info_data[i].video_resolution_height      = _GET_WORD(pCur);  pCur += 2;
                    pParam->pStream_info_data[i].video_framerate              = _GET_BYTE(pCur);  pCur += 1;
                    pParam->pStream_info_data[i].video_bitrate                = _GET_WORD(pCur);  pCur += 2;
                    pParam->pStream_info_data[i].audio_pid                    = _GET_WORD(pCur);  pCur += 2;
                    pParam->pStream_info_data[i].audio_encoding_type          = _GET_BYTE(pCur);  pCur += 1;
                    pParam->pStream_info_data[i].audio_bitrate                = _GET_WORD(pCur);  pCur += 2;
                    pParam->pStream_info_data[i].audio_samplerate             = _GET_WORD(pCur);  pCur += 2;
                    pParam->pStream_info_data[i].pcr_pid                      = _GET_WORD(pCur);  pCur += 2;
                }
            }
            break;
        case METADATA_LINE_DETECTOR:
            {
                LINE_DETECTOR_METADATA  *pParam = 0;

                pInfo->pParameters = tscm_malloc(sizeof(LINE_DETECTOR_METADATA));
                if( !pInfo->pParameters )
                {
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                    break;
                }
                memset(pInfo->pParameters, 0x0, sizeof(LINE_DETECTOR_METADATA));

                pParam = (LINE_DETECTOR_METADATA*)pInfo->pParameters;

                pParam->rule_token.length  = _GET_DWORD(pCur);  pCur += 4;
                pParam->rule_token.pStream = pCur;              pCur += pParam->rule_token.length;

                pParam->object_id   = _GET_DWORD(pCur);  pCur += 4;
                pParam->utc_hour    = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_minute  = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_second  = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_year    = _GET_WORD(pCur);  pCur += 2;
                pParam->utc_month   = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_day     = _GET_BYTE(pCur);  pCur += 1;
            }
            break;
        case METADATA_FIELD_DETECTOR:
            {
                FIELD_DETECTOR_METADATA  *pParam = 0;

                pInfo->pParameters = tscm_malloc(sizeof(FIELD_DETECTOR_METADATA));
                if( !pInfo->pParameters )
                {
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                    break;
                }
                memset(pInfo->pParameters, 0x0, sizeof(FIELD_DETECTOR_METADATA));

                pParam = (FIELD_DETECTOR_METADATA*)pInfo->pParameters;

                pParam->rule_token.length  = _GET_DWORD(pCur);  pCur += 4;
                pParam->rule_token.pStream = pCur;              pCur += pParam->rule_token.length;

                pParam->object_id   = _GET_DWORD(pCur);  pCur += 4;
                pParam->is_inside   = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_hour    = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_minute  = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_second  = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_year    = _GET_WORD(pCur);  pCur += 2;
                pParam->utc_month   = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_day     = _GET_BYTE(pCur);  pCur += 1;
            }
            break;
        case METADATA_DECLARATIVE_MOTION_DETECTOR:
            {
                DECLARATIVE_MOTION_DETECTOR_METADATA  *pParam = 0;

                pInfo->pParameters = tscm_malloc(sizeof(DECLARATIVE_MOTION_DETECTOR_METADATA));
                if( !pInfo->pParameters )
                {
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                    break;
                }
                memset(pInfo->pParameters, 0x0, sizeof(DECLARATIVE_MOTION_DETECTOR_METADATA));

                pParam = (DECLARATIVE_MOTION_DETECTOR_METADATA*)pInfo->pParameters;

                pParam->rule_token.length  = _GET_DWORD(pCur);  pCur += 4;
                pParam->rule_token.pStream = pCur;              pCur += pParam->rule_token.length;

                pParam->object_id   = _GET_DWORD(pCur);  pCur += 4;
                pParam->utc_hour    = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_minute  = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_second  = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_year    = _GET_WORD(pCur);  pCur += 2;
                pParam->utc_month   = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_day     = _GET_BYTE(pCur);  pCur += 1;
            }
            break;
        case METADATA_COUNTING_RULE:
            {
                COUNTING_RULE_METADATA  *pParam = 0;

                pInfo->pParameters = tscm_malloc(sizeof(COUNTING_RULE_METADATA));
                if( !pInfo->pParameters )
                {
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                    break;
                }
                memset(pInfo->pParameters, 0x0, sizeof(COUNTING_RULE_METADATA));

                pParam = (COUNTING_RULE_METADATA*)pInfo->pParameters;

                pParam->rule_token.length  = _GET_DWORD(pCur);  pCur += 4;
                pParam->rule_token.pStream = pCur;              pCur += pParam->rule_token.length;

                pParam->object_id   = _GET_DWORD(pCur);  pCur += 4;
                pParam->count       = _GET_DWORD(pCur);  pCur += 4;
                pParam->utc_hour    = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_minute  = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_second  = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_year    = _GET_WORD(pCur);  pCur += 2;
                pParam->utc_month   = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_day     = _GET_BYTE(pCur);  pCur += 1;
            }
            break;
        case METADATA_CELL_MOTION_DETECTOR:
            {
                CELL_MOTION_DETECTOR_METADATA  *pParam = 0;

                pInfo->pParameters = tscm_malloc(sizeof(CELL_MOTION_DETECTOR_METADATA));
                if( !pInfo->pParameters )
                {
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                    break;
                }
                memset(pInfo->pParameters, 0x0, sizeof(CELL_MOTION_DETECTOR_METADATA));

                pParam = (CELL_MOTION_DETECTOR_METADATA*)pInfo->pParameters;

                pParam->rule_token.length  = _GET_DWORD(pCur);  pCur += 4;
                pParam->rule_token.pStream = pCur;              pCur += pParam->rule_token.length;

                pParam->is_motion   = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_hour    = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_minute  = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_second  = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_year    = _GET_WORD(pCur);  pCur += 2;
                pParam->utc_month   = _GET_BYTE(pCur);  pCur += 1;
                pParam->utc_day     = _GET_BYTE(pCur);  pCur += 1;
            }
            break;
    }
    //---------------------
    return;
}
CMD_PKT_DECODE_INSTANCE(MetadataStreamOutput_Cmd_Pkt_Decode, MetadataStreamOutput_INFO, _MetadataStreamOutput_dec);

static uint32_t
_MetadataStreamOutput_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        if( bUser_Cmd_destroy == true )
        {
            MetadataStreamOutput_INFO   *pInfo = (MetadataStreamOutput_INFO*)(*((uint8_t**)input_info));

            if( pInfo && pInfo->pParameters )
            {
                switch( pInfo->type )
                {
                    case METADATA_STREAM_INFO:
                        {
                            STREAM_INFO_PARAMS  *pParam = (STREAM_INFO_PARAMS*)pInfo->pParameters;
                            if( pParam->pStream_info_data )     free(pParam->pStream_info_data);
                        }
                        break;
                }

                free(pInfo->pParameters);
            }
        }

        free((*((uint8_t**)input_info)));
        (*((uint8_t**)input_info)) = 0;
    }while(0);

    return 0;
}
//=============================================================================
//                Public Function Definition
//=============================================================================

// DEFINE_CMD_PKT_CODEC(CMD_xxxInput, xxxInput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxInput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);
// DEFINE_CMD_PKT_CODEC(CMD_xxxOutput, xxxOutput_Cmd_Ctxt_New,
//                      cmd_pkt_General_Cmd_Pkt_Encode, xxxOutput_Cmd_Pkt_Decode, cmd_pkt_General_Cmd_Ctxt_Del, 0);


DEFINE_CMD_PKT_CODEC(CMD_MetadataStreamOutput, MetadataStreamOutput_Cmd_Ctxt_New,
                     cmd_pkt_General_Cmd_Pkt_Encode, MetadataStreamOutput_Cmd_Pkt_Decode, _MetadataStreamOutput_Cmd_Ctxt_Del, 0);
