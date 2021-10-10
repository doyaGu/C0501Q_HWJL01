

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
DEFINE_CMD_REGISTER_TEMPLATE(CMD_PKT_CODEC_DESC, uint32_t);

static void
_cmd_pkt_Register_all(
    void)
{
    static int bInitialized = 0;

    if( bInitialized )
        return;
    bInitialized = 1;

    //---------------------------------
    // ccHDtv service
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetTxDeviceAddressIDInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetTxDeviceAddressIDOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetTransmissionParameterCapabilitiesInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetTransmissionParameterCapabilitiesOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetTransmissionParametersInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetTransmissionParametersOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetHwRegisterValuesInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetHwRegisterValuesOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetAdvanceOptionsInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetAdvanceOptionsOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetSiPsiTableInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetSiPsiTableOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetNitLocationInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetNitLocationOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetSdtServiceInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetSdtServiceOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetTxDeviceAddressIDInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetTxDeviceAddressIDOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetCalibrationTableInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetCalibrationTableOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetTransmissionParametersInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetTransmissionParametersOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetHwRegisterValuesInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetHwRegisterValuesOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetAdvaneOptionsInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetAdvaneOptionsOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetSiPsiTableInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetSiPsiTableOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetNitLocationInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetNitLocationOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetSdtServiceInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetSdtServiceOutput);

    //---------------------------------
    // device management service
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetCapabilitiesInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetCapabilitiesOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetDeviceInformationInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetDeviceInformationOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetHostnameInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetHostnameOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetSystemDateAndTimeInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetSystemDateAndTimeOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetSystemLogInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetSystemLogOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetOSDInformationInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetOSDInformationOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SystemRebootInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SystemRebootOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetSystemFactoryDefaultInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetSystemFactoryDefaultOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetHostnameInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetHostnameOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetSystemDateAndTimeInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetSystemDateAndTimeOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetOSDInformationInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetOSDInformationOutput);

    //---------------------------------
    // device I/O service
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetDigitalInputsInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetDigitalInputsOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetRelayOutputsInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetRelayOutputsOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetRelayOutputStateInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetRelayOutputStateOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetRelayOutputSettingsInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetRelayOutputSettingsOutput);

    //---------------------------------
    // image service
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetImagingSettingsInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetImagingSettingsOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_IMG_GetStatusInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_IMG_GetStatusOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetImagingSettingsInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetImagingSettingsOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_MoveInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_MoveOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_IMG_StopInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_IMG_StopOutput);

    //---------------------------------
    // media service
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetProfilesInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetProfilesOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetVideoSourcesInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetVideoSourcesOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetVideoSourceConfigurationsInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetVideoSourceConfigurationsOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetGuaranteedNumberOfVideoEncoderInstancesInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetGuaranteedNumberOfVideoEncoderInstancesOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetVideoEncoderConfigurationsInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetVideoEncoderConfigurationsOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetAudioSourcesInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetAudioSourcesOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetAudioSourceConfigurationsInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetAudioSourceConfigurationsOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetAudioEncoderConfigurationsInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetAudioEncoderConfigurationsOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetSynchronizationPointInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetSynchronizationPointOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetVideoSourceConfigurationInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetVideoSourceConfigurationOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetVideoEncoderConfigurationInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetVideoEncoderConfigurationOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetAudioSourceConfigurationInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetAudioSourceConfigurationOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetAudioEncoderConfigurationInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetAudioEncoderConfigurationOutput);

    //---------------------------------
    // PTZ service
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetConfigurationsInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetConfigurationsOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_PTZ_GetStatusInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_PTZ_GetStatusOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetPresetsInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetPresetsOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GotoPresetInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GotoPresetOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_RemovePresetInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_RemovePresetOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetPresetInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetPresetOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_AbsoluteMoveInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_AbsoluteMoveOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_RelativeMoveInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_RelativeMoveOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_ContinuousMoveInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_ContinuousMoveOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetHomePositionInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_SetHomePositionOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GotoHomePositionInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GotoHomePositionOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_PTZ_StopInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_PTZ_StopOutput);

    //---------------------------------
    // video analytics service
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetSupportedRulesInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetSupportedRulesOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetRulesInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_GetRulesOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_CreateRuleInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_CreateRuleOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_ModifyRuleInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_ModifyRuleOutput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_DeleteRuleInput);
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_DeleteRuleOutput);

    //---------------------------------
    // metadata stream service
    CMD_REGISTER_ITEM(CMD_PKT_CODEC_DESC, CMD_MetadataStreamOutput);
}
//=============================================================================
//                Public Function Definition
//=============================================================================
uint32_t
cmd_pkt_Init(
    CMD_PKT_INIT_INFO   *pInit_info,
    void                *extraData)
{
    uint32_t        result = 0;

    do{
        CMD_PKT_CODEC_DESC  *pPkt_codec_desc = 0;

        if( !pInit_info || !pInit_info->set_codec )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, invalid parameter !!");
            result = -1;
            break;
        }

        // register cmd
        _cmd_pkt_Register_all();

        while(1)
        {
            CMD_PKT_ARG     arg = {0};

            pPkt_codec_desc = CMD_PKT_GET_NEXT_DESC(CMD_PKT_CODEC_DESC);
            if( !pPkt_codec_desc )
                break;

            arg.type                           = CMD_PKT_ARG_TYPE_SET_CODEC;
            arg.arg.set_codec.cmd_code         = pPkt_codec_desc->cmd_code;
            arg.arg.set_codec.cmd_pkt_encode   = pPkt_codec_desc->cmd_pkt_encode;
            arg.arg.set_codec.cmd_pkt_decode   = pPkt_codec_desc->cmd_pkt_decode;
            arg.arg.set_codec.cmd_ctxt_create  = pPkt_codec_desc->cmd_ctxt_create;
            arg.arg.set_codec.cmd_ctxt_destroy = pPkt_codec_desc->cmd_ctxt_destroy;
            pInit_info->set_codec(pInit_info, &arg, extraData);
        }

    }while(0);

    if( result )
    {
        tscm_msg_ex(TSCM_MSG_ERR, "%s, err 0x%x !", __FUNCTION__, result);
    }
    return result;
}

uint32_t
cmd_pkt_General_Cmd_Ctxt_Del(
    bool    bUser_Cmd_destroy,
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    do{
        if( !input_info )   break;

        free((*((uint8_t**)input_info)));
        (*((uint8_t**)input_info)) = 0;
    }while(0);

    return 0;
}


uint32_t
cmd_pkt_General_Cmd_Pkt_Encode(
    void    *input_info,
    void    *output_info,
    void    *extraData)
{
    uint32_t            result = 0;
    CMD_PKT_MUXER       *pMuxer = (CMD_PKT_MUXER*)input_info;

    do{
        uint8_t     *pCur = 0;

        if( !pMuxer )       break;

        pMuxer->pCur_cmd_ctxt = (pMuxer->pCur_cmd_ctxt) ? pMuxer->pCur_cmd_ctxt : (uint8_t*)pMuxer->pCmd_pkt_privData;

        //-----------------------------------
        // get length/packet_num/end_pointer
        if( !pMuxer->total_pkt_num )
        {
            uint32_t    cmd_ctxt_length = 0;

            cmd_ctxt_length = _GET_DWORD((uint8_t*)pMuxer->pCmd_pkt_privData);
            cmd_ctxt_length += 1; // add check_sum size
            pMuxer->pEnd_cmd_ctxt = (uint8_t*)pMuxer->pCmd_pkt_privData + cmd_ctxt_length;

            pMuxer->total_pkt_num = cmd_ctxt_length / pMuxer->rcp_cmd_data_size;
            pMuxer->total_pkt_num = (cmd_ctxt_length - (pMuxer->total_pkt_num * pMuxer->rcp_cmd_data_size))
                                        ? pMuxer->total_pkt_num + 1: pMuxer->total_pkt_num;
        }

        // create packets buffer
        if( !pMuxer->pCmd_pkts_buf )
        {
            pMuxer->cmd_pkts_buf_size = pMuxer->total_pkt_num * pMuxer->pkt_size;
            if( !(pMuxer->pCmd_pkts_buf = tscm_malloc(pMuxer->cmd_pkts_buf_size)) )
            {
                tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                result = -1;
                break;
            }
            memset(pMuxer->pCmd_pkts_buf, 0xaa, pMuxer->cmd_pkts_buf_size);
            pMuxer->pkt_num = 0;
        }

        //-----------------------------------
        // get current pointer/length and copy cmd_ctxt to packet_buf
        pCur = pMuxer->pCmd_pkts_buf + (pMuxer->pkt_num * pMuxer->pkt_size)
                    + pMuxer->pkt_rcp_data_offset + sizeof(CMD_RCP_HDR);

        if( (pMuxer->pCur_cmd_ctxt + pMuxer->rcp_cmd_data_size) > pMuxer->pEnd_cmd_ctxt )
        {
            pMuxer->pkt_length = pMuxer->pEnd_cmd_ctxt - pMuxer->pCur_cmd_ctxt;
            pMuxer->bSplitDone = true;
        }
        else
            pMuxer->pkt_length = pMuxer->rcp_cmd_data_size;

        memcpy(pCur, pMuxer->pCur_cmd_ctxt, pMuxer->pkt_length);

        //-----------------------------------
        // update parameters
        pMuxer->pCur_cmd_ctxt += pMuxer->pkt_length;

    }while(0);

    return result;
}


