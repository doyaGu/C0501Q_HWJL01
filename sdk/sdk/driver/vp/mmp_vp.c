//#include "host/host.h"
//#include "pal/pal.h"
//#include "sys/sys.h"

#include "vp/vp_config.h"
#include "vp/vp_types.h"
#include "vp/vp_reg.h"
#include "vp/vp_hw.h"
#include "vp/vp.h"
#include "vp/mmp_vp.h"
#include "ite/ith.h"

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
VP_CONTEXT* VPctxt = MMP_NULL;

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
 * ISP context initialization.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 *
 * @remark Application must call this API first when it want to use ISP API.
 */
//=============================================================================
MMP_RESULT
mmpVPInitialize(
    void)
{
    VP_RESULT  result = VP_SUCCESS;

    if (VPctxt == MMP_NULL)
    {
        VPctxt = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(VP_CONTEXT));
        if (!VPctxt)
        {
            result = VP_ERR_CONTEXT_ALLOC_FAIL;
            goto end;
        }
    }

    VP_PowerUp();

    PalMemset((void*)VPctxt, 0, sizeof(VP_CONTEXT));

    VP_ContextInitialize(MMP_TRUE);

end:
    if (result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * ISP terminate.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 *
 * @remark Application must call this API when leaving ISP module.
 */
//=============================================================================
MMP_RESULT
mmpVPTerminate(
    void)
{
    VP_RESULT  result = VP_SUCCESS;

    if (VPctxt == MMP_NULL)
    {
        return (MMP_RESULT)result;
    }

    //
    // Disable ISP engine
    //
    result = VP_WaitEngineIdle();
    if (result)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    if (VPctxt->EnableInterrupt == MMP_TRUE)
    {
        result = VP_WaitInterruptIdle();
        if (result)
        {
            VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
            goto end;
        }

        mmpVPDisableInterrupt();
    }

    VP_PowerDown();

    PalMemset((void*)VPctxt, 0, sizeof(VP_CONTEXT));

end:
    if (result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * ISP context reset.
 *
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 *
 * @remark Application must call this API first when it want to use ISP API.
 */
//=============================================================================
MMP_RESULT
mmpVPContextReset(
    void)
{
    //Update Flag
    VPctxt->UpdateFlags = 0xFFFFFFFF;

    //Input Information
    PalMemset((void*)(&(VPctxt->InInfo)), 0, sizeof(VP_INPUT_INFO));

    //Deinterlace
    VPctxt->DeInterlace.Enable = MMP_FALSE;

    //Jpeg encode
    PalMemset((void*)(&(VPctxt->JpegEncode)), 0, sizeof(VP_JEPG_ENCODE_CTRL));

    //Output Information
    PalMemset((void*)(&(VPctxt->OutInfo)), 0, sizeof(VP_OUTPUT_INFO));

    //Interrupt
    VPctxt->EnableInterrupt = MMP_FALSE;

    //Context Init
    VP_ContextInitialize(MMP_FALSE);
}

//=============================================================================
/**
 * Enable ISP capability.
 *
 * @param  cap  Specifies a symbolic constant indicating a ISP capability.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpVPEnable(
    MMP_VP_CAPS    cap)
{
    VP_RESULT  result = VP_SUCCESS;

    if (VPctxt == MMP_NULL)
    {
        result = VP_ERR_NOT_INITIALIZE;
        VP_msg_ex(VP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    switch (cap)
    {
    case MMP_VP_DEINTERLACE:
        VPctxt->DeInterlace.Enable = MMP_TRUE;
        VPctxt->DeInterlace.UVRepeatMode = MMP_TRUE;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_DeInterlaceParam;
        break;

    case MMP_VP_LOWLEVELEDGE:
        VPctxt->DeInterlace.EnLowLevelEdge = MMP_TRUE;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_DeInterlaceParam;
        break;

    case MMP_VP_FRAME_FUNCTION_0:
        VPctxt->FrameFun0.Enable = MMP_TRUE;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_FrameFun0;
        break;

    case MMP_VP_REMAP_ADDRESS:
        VPctxt->OutInfo.EnableRemapYAddr = MMP_TRUE;
        VPctxt->OutInfo.EnableRemapUVAddr = MMP_TRUE;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_OutAddress;
        break;

    case MMP_VP_INTERRUPT:
        VPctxt->EnableInterrupt = MMP_TRUE;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_Interrupt;
        break;

    case MMP_VP_DEINTER_FIELD_TOP:
        if(VPctxt->DeInterlace.EnSrcBottomFieldFirst == 0)
            VPctxt->DeInterlace.EnDeinterBottomField = 1;
        else
            VPctxt->DeInterlace.EnDeinterBottomField = 0;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_DeInterlaceParam;
        break;

    case MMP_VP_DEINTER_FIELD_BOTTOM:
        if(VPctxt->DeInterlace.EnSrcBottomFieldFirst == 0)
            VPctxt->DeInterlace.EnDeinterBottomField = 0;
        else
            VPctxt->DeInterlace.EnDeinterBottomField = 1;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_DeInterlaceParam;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_InputBuf;
        break;

    case MMP_VP_SCENECHANGE:
        VPctxt->InInfo.EnableSceneChg = MMP_TRUE;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_SceneChange;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_InputBuf;
        break;

    default:
        result = VP_ERR_NO_MATCH_ENABLE_TYPE;
        break;
    }

end:
    if (result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Disable ISP capability.
 *
 * @param  cap  Specifies a symbolic constant indicating a ISP capability.
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpVPDisable(
    MMP_VP_CAPS cap)
{
    VP_RESULT   result = VP_SUCCESS;

    if (VPctxt == MMP_NULL)
    {
        result = VP_ERR_NOT_INITIALIZE;
        VP_msg_ex(VP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    switch (cap)
    {
    case MMP_VP_DEINTERLACE:
        VPctxt->DeInterlace.Enable = MMP_FALSE;
        VPctxt->DeInterlace.UVRepeatMode = MMP_FALSE;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_DeInterlaceParam;
        break;

    case MMP_VP_LOWLEVELEDGE:
        VPctxt->DeInterlace.EnLowLevelEdge = MMP_FALSE;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_DeInterlaceParam;
        break;

    case MMP_VP_FRAME_FUNCTION_0:
        VPctxt->FrameFun0.Enable = MMP_FALSE;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_FrameFun0;
        break;

    case MMP_VP_REMAP_ADDRESS:
        VPctxt->OutInfo.EnableRemapYAddr = MMP_FALSE;
        VPctxt->OutInfo.EnableRemapUVAddr = MMP_FALSE;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_OutAddress;
        break;

    case MMP_VP_INTERRUPT:
        VPctxt->EnableInterrupt = MMP_FALSE;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_Interrupt;

    case MMP_VP_SCENECHANGE:
        VPctxt->InInfo.EnableSceneChg = MMP_FALSE;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_SceneChange;
        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_InputBuf;
        break;

    default:
        result = VP_ERR_NO_MATCH_ENABLE_TYPE;
        break;
    }

end:
    if (result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Query ISP capability.
 *
 * @param cap       Specifies a symbolic constant indicating a ISP capability.
 * @return MMP_TRUE if function enabled, MMP_FALSE if function disable.
 */
//=============================================================================
MMP_BOOL
mmpVPQuery(
    MMP_VP_CAPS    cap)
{
    MMP_BOOL   result = MMP_FALSE;

    if (VPctxt == MMP_NULL)
    {
        result = VP_ERR_NOT_INITIALIZE;
        VP_msg_ex(VP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    switch(cap)
    {
    case MMP_VP_DEINTERLACE:
        result = VPctxt->DeInterlace.Enable;
        break;

    case MMP_VP_LOWLEVELEDGE:
        result = VPctxt->DeInterlace.EnLowLevelEdge;
        break;

    case MMP_VP_FRAME_FUNCTION_0:
        result = VPctxt->FrameFun0.Enable;
        break;

    case MMP_VP_REMAP_ADDRESS:
        result = VPctxt->OutInfo.EnableRemapYAddr;
        break;

    case MMP_VP_INTERRUPT:
        result = VPctxt->EnableInterrupt;
        break;

    default:
        result = MMP_FALSE;
        break;
    }

end:
    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Set Remap Address Parameter
 * @param MMP_VP_REMAP_ADDR
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpVPSetRemapAddr(
    MMP_VP_REMAP_ADDR    *data)
{
    VP_RESULT         result = VP_SUCCESS;

    if (VPctxt == MMP_NULL)
    {
        result = VP_ERR_NOT_INITIALIZE;
        VP_msg_ex(VP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    VPctxt->RemapYAddr.Addr_03 = data->YAddr_03;
    VPctxt->RemapYAddr.Addr_04 = data->YAddr_04;
    VPctxt->RemapYAddr.Addr_05 = data->YAddr_05;
    VPctxt->RemapYAddr.Addr_06 = data->YAddr_06;
    VPctxt->RemapYAddr.Addr_07 = data->YAddr_07;
    VPctxt->RemapYAddr.Addr_08 = data->YAddr_08;
    VPctxt->RemapYAddr.Addr_09 = data->YAddr_09;
    VPctxt->RemapYAddr.Addr_10 = data->YAddr_10;
    VPctxt->RemapYAddr.Addr_11 = data->YAddr_11;
    VPctxt->RemapYAddr.Addr_12 = data->YAddr_12;
    VPctxt->RemapYAddr.Addr_13 = data->YAddr_13;
    VPctxt->RemapYAddr.Addr_14 = data->YAddr_14;
    VPctxt->RemapYAddr.Addr_15 = data->YAddr_15;
    VPctxt->RemapYAddr.Addr_16 = data->YAddr_16;
    VPctxt->RemapYAddr.Addr_17 = data->YAddr_17;
    VPctxt->RemapYAddr.Addr_18 = data->YAddr_18;
    VPctxt->RemapYAddr.Addr_19 = data->YAddr_19;
    VPctxt->RemapYAddr.Addr_20 = data->YAddr_20;
    VPctxt->RemapYAddr.Addr_21 = data->YAddr_21;
    VPctxt->RemapYAddr.Addr_22 = data->YAddr_22;
    VPctxt->RemapYAddr.Addr_23 = data->YAddr_23;
    VPctxt->RemapYAddr.Addr_24 = data->YAddr_24;
    VPctxt->RemapYAddr.Addr_25 = data->YAddr_25;
    VPctxt->RemapYAddr.Addr_26 = data->YAddr_26;
    VPctxt->RemapYAddr.Addr_27 = data->YAddr_27;
    VPctxt->RemapYAddr.Addr_28 = data->YAddr_28;
    VPctxt->RemapYAddr.Addr_29 = data->YAddr_29;
    VPctxt->RemapYAddr.Addr_30 = data->YAddr_30;
    VPctxt->RemapYAddr.Addr_31 = data->YAddr_31;

    VPctxt->RemapUVAddr.Addr_03 = data->UVAddr_03;
    VPctxt->RemapUVAddr.Addr_04 = data->UVAddr_04;
    VPctxt->RemapUVAddr.Addr_05 = data->UVAddr_05;
    VPctxt->RemapUVAddr.Addr_06 = data->UVAddr_06;
    VPctxt->RemapUVAddr.Addr_07 = data->UVAddr_07;
    VPctxt->RemapUVAddr.Addr_08 = data->UVAddr_08;
    VPctxt->RemapUVAddr.Addr_09 = data->UVAddr_09;
    VPctxt->RemapUVAddr.Addr_10 = data->UVAddr_10;
    VPctxt->RemapUVAddr.Addr_11 = data->UVAddr_11;
    VPctxt->RemapUVAddr.Addr_12 = data->UVAddr_12;
    VPctxt->RemapUVAddr.Addr_13 = data->UVAddr_13;
    VPctxt->RemapUVAddr.Addr_14 = data->UVAddr_14;
    VPctxt->RemapUVAddr.Addr_15 = data->UVAddr_15;
    VPctxt->RemapUVAddr.Addr_16 = data->UVAddr_16;
    VPctxt->RemapUVAddr.Addr_17 = data->UVAddr_17;
    VPctxt->RemapUVAddr.Addr_18 = data->UVAddr_18;
    VPctxt->RemapUVAddr.Addr_19 = data->UVAddr_19;
    VPctxt->RemapUVAddr.Addr_20 = data->UVAddr_20;
    VPctxt->RemapUVAddr.Addr_21 = data->UVAddr_21;
    VPctxt->RemapUVAddr.Addr_22 = data->UVAddr_22;
    VPctxt->RemapUVAddr.Addr_23 = data->UVAddr_23;
    VPctxt->RemapUVAddr.Addr_24 = data->UVAddr_24;
    VPctxt->RemapUVAddr.Addr_25 = data->UVAddr_25;
    VPctxt->RemapUVAddr.Addr_26 = data->UVAddr_26;
    VPctxt->RemapUVAddr.Addr_27 = data->UVAddr_27;
    VPctxt->RemapUVAddr.Addr_28 = data->UVAddr_28;
    VPctxt->RemapUVAddr.Addr_29 = data->UVAddr_29;
    VPctxt->RemapUVAddr.Addr_30 = data->UVAddr_30;
    VPctxt->RemapUVAddr.Addr_31 = data->UVAddr_31;

    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_RemapAddr;

end:
    if (result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * ISP Motion Detection Process
 *
 * @param data  MMP_VP_SINGLE_SHARE
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpVPMotionProcess(
    const MMP_VP_SINGLE_SHARE     *data)
{
    VP_RESULT  result = VP_SUCCESS;

    if (VPctxt == MMP_NULL)
    {
        result = VP_ERR_NOT_INITIALIZE;
        VP_msg_ex(VP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    //
    // Set Input Address, Width, Height and Pitch
    //
    VPctxt->InInfo.AddrY[0]    = (MMP_UINT8*)data->In_AddrY;
    VPctxt->InInfo.AddrUV[0]   = (MMP_UINT8*)data->In_AddrUV;
    VPctxt->InInfo.AddrYp      = (MMP_UINT8*)data->In_AddrYp;

    // width must be 2 alignment
    VPctxt->InInfo.SrcWidth    = (data->In_Width >> 2) << 2;
    VPctxt->InInfo.SrcHeight   = data->In_Height;

    VPctxt->InInfo.PitchY      = data->In_PitchY;
    VPctxt->InInfo.PitchUV     = data->In_PitchUV;

    // Set Input Format
    result = VP_SetInputFormat(data->In_Format);
    if (result)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    //
    // Set Output Address, Width, Height and Pitch
    //
    VPctxt->OutInfo.AddrY[0]   = (MMP_UINT8*)data->Out_AddrY;
    VPctxt->OutInfo.AddrU[0]   = (MMP_UINT8*)data->Out_AddrU;
    VPctxt->OutInfo.AddrV[0]   = (MMP_UINT8*)data->Out_AddrV;

    // width must be 2 alignment
    VPctxt->OutInfo.Width      = (data->Out_Width >> 2) << 2;
    VPctxt->OutInfo.Height     = data->Out_Height;

    VPctxt->OutInfo.PitchY     = data->Out_PitchY;
    VPctxt->OutInfo.PitchUV    = data->Out_PitchUV;

    // Set Output Format
    result = VP_SetOutputFormat(data->Out_Format);
    if (result)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    // Set single porcess
    VPctxt->InInfo.EnableReadMemoryMode = MMP_TRUE;
    VPctxt->InInfo.DisableCaptureCtrl = MMP_TRUE;
    VPctxt->InInfo.InputBufferNum = 0;

    VPctxt->OutInfo.OutputBufferNum = 0;
    VPctxt->OutInfo.EnableSWCtrlRdAddr = MMP_TRUE;
    VPctxt->OutInfo.EnableSWFlipMode = MMP_FALSE;
    VPctxt->OutInfo.EnableFieldMode = MMP_FALSE;
    VPctxt->OutInfo.EnableUVBiDownsample = MMP_TRUE;

    VPctxt->JpegEncode.EnableJPEGEncode = MMP_FALSE;

    // Enable Motion Detection Setting
    VP_EnableMotionDetectionParameter(&VPctxt->MotionParam);

    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_InputParameter;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_InputBuf;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_InputAddr;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_OutParameter;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_OutBufInfo;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_OutAddress;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_JpegEncode;

    // check isp engine idle
    result = VP_WaitEngineIdle();
    if (result)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    // Update parameter
    result = VP_Update();
    if (result)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    // Fire ISP
    // VP_LogReg();
    VP_DriverFire_Reg();

end:
    VP_DisableMotionDetectionParameter();

    if (result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * ISP Single Process
 *
 * @param data  MMP_VP_SINGLE_SHARE
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpVPSingleProcess(
    const MMP_VP_SINGLE_SHARE     *data)
{
    VP_RESULT         result = VP_SUCCESS;

    if (VPctxt == MMP_NULL)
    {
        result = VP_ERR_NOT_INITIALIZE;
        VP_msg_ex(VP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    //
    // Set Input Address, Width, Height and Pitch
    //
    VPctxt->InInfo.AddrY[0]    = (MMP_UINT8*)data->In_AddrY;
    VPctxt->InInfo.AddrUV[0]   = (MMP_UINT8*)data->In_AddrUV;
    VPctxt->InInfo.AddrYp      = (MMP_UINT8*)data->In_AddrYp;

    // width must be 2 alignment
    VPctxt->InInfo.SrcWidth  = (data->In_Width >> 2) << 2;
    VPctxt->InInfo.SrcHeight = (data->In_Height >> 2) << 2;

    VPctxt->InInfo.PitchY  = data->In_PitchY;
    VPctxt->InInfo.PitchUV = data->In_PitchUV;

    // Set Input Format
    result = VP_SetInputFormat(data->In_Format);
    if (result)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    //
    // Set Output Address, Width, Height and Pitch
    //
    VPctxt->OutInfo.AddrY[0] = (MMP_UINT8*)data->Out_AddrY;
    VPctxt->OutInfo.AddrU[0] = (MMP_UINT8*)data->Out_AddrU;
    VPctxt->OutInfo.AddrV[0] = (MMP_UINT8*)data->Out_AddrV;

    // width must be 2 alignment
    VPctxt->OutInfo.Width  = (data->Out_Width >> 2) << 2;
    VPctxt->OutInfo.Height = data->Out_Height;

    VPctxt->OutInfo.PitchY  = data->Out_PitchY;
    VPctxt->OutInfo.PitchUV = data->Out_PitchUV;

    // Set Output Format
    result = VP_SetOutputFormat(data->Out_Format);
    if (result)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    // Set single porcess
    VPctxt->InInfo.EnableReadMemoryMode = MMP_TRUE;
    VPctxt->InInfo.DisableCaptureCtrl = MMP_TRUE;
    VPctxt->InInfo.InputBufferNum = 0;

    VPctxt->OutInfo.OutputBufferNum = 0;
    VPctxt->OutInfo.EnableSWCtrlRdAddr = MMP_TRUE;
    VPctxt->OutInfo.EnableSWFlipMode = MMP_FALSE;
    VPctxt->OutInfo.EnableFieldMode = MMP_FALSE;
    VPctxt->OutInfo.EnableUVBiDownsample = MMP_TRUE;

    VPctxt->InterruptMode = 0x2;

    VPctxt->JpegEncode.EnableJPEGEncode = MMP_FALSE;

    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_InputParameter;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_InputBuf;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_InputAddr;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_OutParameter;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_OutBufInfo;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_OutAddress;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_JpegEncode;

    // Update parameter
    result = VP_Update();
    if (result)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    // Fire ISP
    // VP_LogReg();
    VP_DriverFire_Reg();

end:
    if (result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * ISP Sequence Process.
 * @param data MMP_VP_SEQUENCE_SHARE
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpVPSequenceProcess(
    const MMP_VP_SEQUENCE_SHARE     *data)
{
    VP_RESULT      result = VP_SUCCESS;
    MMP_UINT8       i;

    if (VPctxt == MMP_NULL)
    {
        result = VP_ERR_NOT_INITIALIZE;
        VP_msg_ex(VP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    if (VPctxt->OutInfo.Width == 0 || VPctxt->OutInfo.Height == 0)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    //
    // Set Input Address, Width, Height and Pitch
    //
    if (data->In_BufferNum == 0 && data->EnCapOnflyMode == MMP_FALSE)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }
    else if (data->EnCapOnflyMode == MMP_FALSE)
    {
        VPctxt->InInfo.InputBufferNum = data->In_BufferNum - 1;

        for (i = 0; i < 5; i++)
        {
            if (i < data->In_BufferNum)
            {
                VPctxt->InInfo.AddrY[i] = (MMP_UINT8*)data->In_AddrY[i];
                VPctxt->InInfo.AddrUV[i] = (MMP_UINT8*)data->In_AddrUV[i];
            }
            else
            {
                VPctxt->InInfo.AddrY[i] = 0x0;
                VPctxt->InInfo.AddrUV[i] = 0x0;
            }
        }
        VPctxt->InInfo.AddrYp = 0x0;

        VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_InputAddr;
    }

    //width must be 2 alignment
    VPctxt->InInfo.SrcWidth  = (data->In_Width >> 2) << 2;
    VPctxt->InInfo.SrcHeight = data->In_Height;

    VPctxt->InInfo.PitchY  = data->In_PitchY;
    VPctxt->InInfo.PitchUV = data->In_PitchUV;

    //Set Input Format
    result = VP_SetInputFormat(data->In_Format);
    if (result)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    //Set sqeuence porcess

    if (data->EnCapOnflyMode == MMP_TRUE)
        VPctxt->InInfo.EnableReadMemoryMode = MMP_FALSE;
    else
        VPctxt->InInfo.EnableReadMemoryMode = MMP_TRUE;

    VPctxt->InInfo.DisableCaptureCtrl = MMP_FALSE;

    VPctxt->OutInfo.EnableSWCtrlRdAddr = MMP_FALSE;
    VPctxt->OutInfo.EnableSWFlipMode = MMP_FALSE;

    if (data->EnOnflyInFieldMode == MMP_TRUE)
        VPctxt->OutInfo.EnableFieldMode = MMP_TRUE;
    else
        VPctxt->OutInfo.EnableFieldMode = MMP_FALSE;

    VPctxt->OutInfo.EnableUVBiDownsample = MMP_TRUE;

    VPctxt->JpegEncode.EnableJPEGEncode = MMP_FALSE;

    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_InputParameter;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_InputBuf;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_OutParameter;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_JpegEncode;

    //check isp engine idle
    result = VP_WaitEngineIdle();
    if (result)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    VPctxt->UpdateFlags = 0xFFFFFFFF;

    //Update parameter
    result = VP_Update();
    if (result)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    // Fire ISP
    // VP_LogReg();
    VP_RefreshFire_Reg();

end:
    if (result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
* Set Sequence Output Parameter
* @param MMP_VP_SEQUENCE_SHARE
* @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
*/
//=============================================================================
MMP_RESULT
mmpVPSetSequenceOutputInfo(
    const MMP_VP_SEQUENCE_SHARE    *data)
{
    VP_RESULT  result = VP_SUCCESS;
    MMP_UINT8   i;

    if (VPctxt == MMP_NULL)
    {
        result = VP_ERR_NOT_INITIALIZE;
        VP_msg_ex(VP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    //
    //Set Output Address, Width, Height and Pitch
    //
    if (data->Out_BufferNum == 0)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }
    else
    {
        VPctxt->OutInfo.OutputBufferNum = data->Out_BufferNum - 1;

        for (i = 0; i < 5; i++)
        {
            if (i < data->Out_BufferNum)
            {
                VPctxt->OutInfo.AddrY[i] = (MMP_UINT8*)data->Out_AddrY[i];
                VPctxt->OutInfo.AddrU[i] = (MMP_UINT8*)data->Out_AddrU[i];
                VPctxt->OutInfo.AddrV[i] = (MMP_UINT8*)data->Out_AddrV[i];
            }
            else
            {
                VPctxt->OutInfo.AddrY[i] = 0x0;
                VPctxt->OutInfo.AddrU[i] = 0x0;
                VPctxt->OutInfo.AddrV[i] = 0x0;
            }
        }
    }

    //width must be 2 alignment
    VPctxt->OutInfo.Width  = (data->Out_Width >> 2) << 2;
    VPctxt->OutInfo.Height = data->Out_Height;

    VPctxt->OutInfo.PitchY  = data->Out_PitchY;
    VPctxt->OutInfo.PitchUV = data->Out_PitchUV;

    //Set Output Format
    result = VP_SetOutputFormat(data->Out_Format);
    if (result)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_OutParameter;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_OutBufInfo;
    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_OutAddress;

end:
    if (result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Set frame function background image information & color key.  (For Direct Assign VRAM address. Ex.2D input)
 *
 * @param baseAddr      base address of the background image buffer.
 * @param startX        x position of the background image.
 * @param startY        y position of the background image.
 * @param width         width of the background image.
 * @param height        height of the background image.
 * @param colorKeyR     color key for R channel.
 * @param colorKeyG     color key for G channel.
 * @param colorKeyB     color key for B channel.
 * @param constantAlpha constant Alpha Value.
 * @param format        format of the picture & color key. only support RGB 888, RGB565
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 *
 * @see mmpVPEnable() mmpVPDisable()
 */
//=============================================================================
MMP_RESULT
mmpVPSetFrameFunction(
    void*                   vramAddr,
    MMP_UINT                startX,
    MMP_UINT                startY,
    MMP_UINT                width,
    MMP_UINT                height,
    MMP_UINT                pitch,
    MMP_UINT                colorKeyR,
    MMP_UINT                colorKeyG,
    MMP_UINT                colorKeyB,
    MMP_UINT                constantAlpha,
    MMP_PIXEL_FORMAT        format)
{
    VP_RESULT          result = VP_SUCCESS;
    VP_FRMFUN_CTRL     *pIspFrameFunc0 = &VPctxt->FrameFun0;

    pIspFrameFunc0->EnableRGB2YUV = MMP_TRUE;

    if (width < 16 || height < 16)
    {
        VP_msg_ex(VP_MSG_TYPE_ERR, " err, width(%d) < 16 or height(%d) < 16 !!", width, height);
        result = VP_ERR_INVALID_PARAM;
        goto end;
    }

    startX -= (startX & 0x1); // startX%2;
    startY -= (startY & 0x1); // startY%2;
    width  -= (width & 0x1);  // width%2;
    height -= (height & 0x1); // height%2;

    switch (format)
    {
    case MMP_PIXEL_FORMAT_ARGB4444:
        pIspFrameFunc0->Format = ARGB4444;
        break;

    case MMP_PIXEL_FORMAT_RGB565:
        pIspFrameFunc0->Format = CARGB565;
        break;

    default :
        result = VP_ERR_NO_MATCH_OUTPUT_FORMAT;
        goto end;
        break;
    }

    pIspFrameFunc0->ColorKeyR = (MMP_UINT16) colorKeyR;
    pIspFrameFunc0->ColorKeyG = (MMP_UINT16) colorKeyG;
    pIspFrameFunc0->ColorKeyB = (MMP_UINT16) colorKeyB;
    pIspFrameFunc0->ConstantAlpha = (MMP_UINT16) constantAlpha;
    pIspFrameFunc0->StartX = (MMP_UINT16) startX;
    pIspFrameFunc0->StartY = (MMP_UINT16) startY;
    pIspFrameFunc0->Width  = (MMP_UINT16) width;
    pIspFrameFunc0->Height = (MMP_UINT16) height;

    pIspFrameFunc0->Pitch  = (MMP_UINT16) pitch;
    pIspFrameFunc0->Addr   = (MMP_UINT8*) ((MMP_UINT) vramAddr);

    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_FrameFun0;

end:
    if (result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Wait ISP Engine Idle
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpVPWaitEngineIdle(
    void)
{
    return VP_WaitEngineIdle();
}

//=============================================================================
/**
 * Is ISP Engine Idle
 */
//=============================================================================
MMP_BOOL
mmpVPIsEngineIdle(
    void)
{
    return VP_IsEngineIdle();
}

//=============================================================================
/**
 * Wait ISP Interrupt Idle
 * @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
 */
//=============================================================================
MMP_RESULT
mmpVPWaitInterruptIdle(
    void)
{
    return VP_WaitInterruptIdle();
}

//=============================================================================
/**
 * Clear ISP Interrupt
 */
//=============================================================================
void
mmpVPClearInterrupt(
    void)
{
    VP_ClearInterrupt_Reg();
}

//=============================================================================
/**
 * ISP Write Buffer Index
 * @return index number
 */
//=============================================================================
MMP_UINT16
mmpVPReturnWrBufIndex(
    void)
{
    return VP_RetrunWrBufIndex_Reg();
}

//=============================================================================
/**
 * mmpVPRegisterIRQ.
 */
//=============================================================================
void
mmpVPRegisterIRQ(
    VP_handler isphandler)
{
    // Initialize ISP IRQ
    ithIntrDisableIrq(ITH_INTR_ISP);
    ithIntrClearIrq(ITH_INTR_ISP);

    #if defined (__OPENRTOS__)
    // register NAND Handler to IRQ
    ithIntrRegisterHandlerIrq(ITH_INTR_ISP, isphandler, MMP_NULL);
    #endif // defined (__OPENRTOS__)

    // set IRQ to edge trigger
    ithIntrSetTriggerModeIrq(ITH_INTR_ISP, ITH_INTR_EDGE);

    // set IRQ to detect rising edge
    ithIntrSetTriggerLevelIrq(ITH_INTR_ISP, ITH_INTR_HIGH_RISING);

    // Enable IRQ
    ithIntrEnableIrq(ITH_INTR_ISP);
}

//=============================================================================
/**
 * Disable ISP Interrupt.
 */
//=============================================================================
MMP_RESULT
mmpVPDisableInterrupt(
    void)
{
    HOST_WriteRegisterMask(VP_REG_SET50E, ((0x0 & VP_BIT_VP_INTERRUPT_EN) << VP_SHT_VP_INTERRUPT_EN), (VP_BIT_VP_INTERRUPT_EN << VP_SHT_VP_INTERRUPT_EN));

    if (VPctxt != MMP_NULL)
        VPctxt->EnableInterrupt = MMP_FALSE;
}

//=============================================================================
/**
 * mmpVPResetEngine
 */
//=============================================================================
MMP_RESULT
mmpVPResetEngine(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

    HOST_VP_Reset();

    return result;
}


//=============================================================================
/**
 * Set color control value.
 */
//=============================================================================
void
mmpVPSetColorCtrl(
    const MMP_VP_COLOR_CTRL *data)
{
#if defined (USE_COLOR_EFFECT)
    if (data->brightness > 127)
        VPctxt->ColorCtrl.brightness = 127;
    else if (data->brightness < -128)
        VPctxt->ColorCtrl.brightness = -128;
    else
        VPctxt->ColorCtrl.brightness = data->brightness;

    if (data->contrast > 4.0)
        VPctxt->ColorCtrl.contrast = 4.0;
    else if (data->contrast < 0.0)
        VPctxt->ColorCtrl.contrast = 0.0;
    else
        VPctxt->ColorCtrl.contrast = data->contrast;

    if (data->hue > 359)
        VPctxt->ColorCtrl.hue = 359;
    else if (data->hue < 0)
        VPctxt->ColorCtrl.hue = 0;
    else
        VPctxt->ColorCtrl.hue = data->hue;

    if (data->saturation > 4.0)
        VPctxt->ColorCtrl.saturation = 4.0;
    else if (data->saturation < 0.0)
        VPctxt->ColorCtrl.saturation = 0.0;
    else
        VPctxt->ColorCtrl.saturation = data->saturation;

    VP_SetColorCorrMatrix(
        &VPctxt->CCFun,
        VPctxt->ColorCtrl.brightness,
        VPctxt->ColorCtrl.contrast,
        VPctxt->ColorCtrl.hue,
        VPctxt->ColorCtrl.saturation,
        VPctxt->ColorCtrl.colorEffect);

    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_CCMatrix;
#endif
}

//=============================================================================
/**
 * Get color control value.
 */
//=============================================================================
void
mmpVPGetColorCtrl(
    MMP_VP_COLOR_CTRL *data)
{
#if defined (USE_COLOR_EFFECT)
    data->brightness    = VPctxt->ColorCtrl.brightness;
    data->contrast      = VPctxt->ColorCtrl.contrast;
    data->hue           = VPctxt->ColorCtrl.hue;
    data->saturation    = VPctxt->ColorCtrl.saturation;
#endif
}

//=============================================================================
/**
 * Update Color Matrix.
 */
//=============================================================================
void
mmpVPOnflyUpdateColorMatrix(
    void)
{
#if defined (USE_COLOR_EFFECT)
    VP_UpdateColorMatrix();
#endif
}

//=============================================================================
/**
 * ISP Scene Change total diff
 * @return total diff
 */
//=============================================================================
MMP_UINT16
mmpVPSceneChgTotalDiff(
    void)
{
    return VP_SceneChgTotalDiff_Reg();
}

//=============================================================================
/**
* Set Motion Parameter
* @param MMP_VP_MOTION_PARAM
* @return MMP_RESULT_SUCCESS if succeed, error codes of MMP_RESULT_ERROR otherwise.
*/
//=============================================================================
MMP_RESULT
mmpVPSetMotionParameter(
    const MMP_VP_MOTION_PARAM    *data)
{
    VP_RESULT  result = VP_SUCCESS;

    if (VPctxt == MMP_NULL)
    {
        result = VP_ERR_NOT_INITIALIZE;
        VP_msg_ex(VP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    //Set Motion Parameter
    VPctxt->MotionParam.MDThreshold_Low = data->MDThreshold_Low;
    VPctxt->MotionParam.MDThreshold_High = data->MDThreshold_High;
    VPctxt->MotionParam.EnLPFWeight = data->EnLPFWeight;
    VPctxt->MotionParam.EnLPFStaticPixel = data->EnLPFStaticPixel;
    VPctxt->MotionParam.DisableMV_A = data->DisableMV_A;
    VPctxt->MotionParam.DisableMV_B = data->DisableMV_B;
    VPctxt->MotionParam.DisableMV_C = data->DisableMV_C;
    VPctxt->MotionParam.DisableMV_D = data->DisableMV_D;
    VPctxt->MotionParam.DisableMV_E = data->DisableMV_E;
    VPctxt->MotionParam.DisableMV_F = data->DisableMV_F;
    VPctxt->MotionParam.DisableMV_G = data->DisableMV_G;

    VPctxt->UpdateFlags |= VP_FLAGS_UPDATE_MotionParameter;

end:
    if (result)
        VP_msg_ex(VP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * VP Power Up.
 */
//=============================================================================
void
mmpVPPowerUp(
    void)
{
    VP_EnableClock();    
}

//=============================================================================
/**
 * VP Power Down
 */
//=============================================================================
void
mmpVPPowerDown(
    void)
{    
    //VP_RESULT  result = VP_SUCCESS;
    
    //result = VP_WaitEngineIdle();
    //if (result)
    //	  VP_msg_ex(VP_MSG_TYPE_ERR, " err 0x%x n", result);
    	  
    if (!VP_IsEngineIdle())
    {        
        VP_msg_ex(VP_MSG_TYPE_ERR, " VP not Idle");        
    }
    
    VP_DisableClock();
}
