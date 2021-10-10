#include <assert.h>
#include "isp_reg.h"
#include "isp_hw.h"
#include "isp.h"
#include "mmp_isp.h"
#include <pthread.h>
#ifndef WIN32
//#include "intr/intr.h"
#endif

static pthread_mutex_t pISPMutex  = PTHREAD_MUTEX_INITIALIZER;
//=============================================================================
//				  Constant Definition
//=============================================================================

//=============================================================================
//				  Macro Definition
//=============================================================================
#define OUT_OF_RANGE(value, min, max)                (((value) < (min)) || ((max) < (value)))
#define GOTO_END_IF_NOT_INITIALIZE(result, pISPctxt) {   \
        if ((pISPctxt) == MMP_NULL)                             \
        {                                                       \
            (result) = ISP_ERR_NOT_INITIALIZE;                  \
            isp_msg_ex(ISP_MSG_TYPE_ERR, " NULL pointer !\n");  \
            goto end;                                           \
        }                                                       \
}

#define RETURN_IF_NOT_INITIALIZE(pISPctxt) {   \
        if ((pISPctxt) == MMP_NULL)                             \
        {                                                       \
            isp_msg_ex(ISP_MSG_TYPE_ERR, " NULL pointer !\n");  \
            return;                                             \
        }                                                       \
}

//=============================================================================
//				  Structure Definition
//=============================================================================

//=============================================================================
//				  Global Data Definition
//=============================================================================
//ISP_CONTEXT      *ISPctxt            = MMP_NULL;
static MMP_BOOL  gbIsInterlaceSource = MMP_FALSE;
extern MMP_UINT8 ISPTilingTable[5][32];

//=============================================================================
//				  Private Function Definition
//=============================================================================
static MMP_UINT8 ReMapTableSel(MMP_UINT16 pitch)
{
    MMP_UINT8 index;

    switch (pitch)
    {
    case 512:
        index = 0;
        break;
    case 1024:
        index = 1;
        break;
    case 2048:
        index = 2;
        break;
    case 4096:
        index = 3;
        break;
    case 8192:
        index = 4;
        break;
    default:
        index = 0;
        break;
    }

    return index;
}

//=============================================================================
//				  Public Function Definition
//=============================================================================
//=============================================================================
/**
 * ISP context initialization.
 *
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 *
 * @remark Application must call this API first when it want to use ISP API.
 */
//=============================================================================
ISP_RESULT
mmpIspInitialize(
    ISP_DEVICE *ptDev)
{
    ISP_RESULT result = ISP_SUCCESS;

    assert(ptDev);
    pthread_mutex_lock(&pISPMutex);
    if ((*ptDev) == MMP_NULL)
        *ptDev = isp_Malloc(sizeof(ISP_CONTEXT));

    if ((*ptDev) == MMP_NULL)
    {
        result = ISP_ERR_CONTEXT_ALLOC_FAIL;
        goto end;
    }
    ISP_WaitEngineIdle();
    ISP_PowerUp();
    isp_Memset((void *)(*ptDev), 0, sizeof(ISP_CONTEXT));
    ISP_ContextInitialize(*ptDev);

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);
    pthread_mutex_unlock(&pISPMutex);
    return result;
}

//=============================================================================
/**
 * ISP powerdown.
 *
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 *
 * @remark Application must call this API when leaving ISP module.
 */
//=============================================================================
ISP_RESULT
mmpIspPowerdown(
    ISP_DEVICE ptDev)
{
    ISP_RESULT  result    = ISP_SUCCESS;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    GOTO_END_IF_NOT_INITIALIZE(result, pISPctxt);
    pthread_mutex_lock(&pISPMutex);
    //
    // Disable ISP engine
    //
    result = ISP_WaitEngineIdle();
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    if (pISPctxt->EnableInterrupt)
    {
        result = ISP_WaitInterruptIdle();
        if (result)
        {
            isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
            goto end;
        }

        mmpIspDisableInterrupt(pISPctxt);
    }

    ISP_PowerDown();
    isp_Memset((void *)(pISPctxt), 0, sizeof(ISP_CONTEXT));

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);
    pthread_mutex_unlock(&pISPMutex);
    return result;
}

//=============================================================================
/**
 * ISP terminate.
 *
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 *
 * @remark Application must call this API when leaving ISP module.
 */
//=============================================================================
ISP_RESULT
mmpIspTerminate(
    ISP_DEVICE *ptDev)
{
    ISP_RESULT  result    = ISP_SUCCESS;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)*ptDev;
    pthread_mutex_lock(&pISPMutex);
    if (ptDev == MMP_NULL || pISPctxt == MMP_NULL)
    {
        pthread_mutex_unlock(&pISPMutex);
        return result;
    }

    //
    // Disable ISP engine
    //
    result = ISP_WaitEngineIdle();
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    if (pISPctxt->EnableInterrupt)
    {
        result = ISP_WaitInterruptIdle();
        if (result)
        {
            isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
            goto end;
        }

        mmpIspDisableInterrupt(pISPctxt);
    }

    ISP_PowerDown();
    isp_Free(pISPctxt);
    *ptDev = MMP_NULL;

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);
    pthread_mutex_unlock(&pISPMutex);
    return result;
}

//=============================================================================
/**
 * ISP context reset.
 *
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 *
 * @remark Application must call this API first when it want to use ISP API.
 */
//=============================================================================
void
mmpIspContextReset(
    ISP_DEVICE ptDev)
{
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    RETURN_IF_NOT_INITIALIZE(pISPctxt);

    //Update Flag
    pISPctxt->UpdateFlags = 0xFFFFFFFF;

    //Input Information
    isp_Memset((void *)(&(pISPctxt->InInfo)), 0, sizeof(ISP_INPUT_INFO));

    //Deinterlace
    pISPctxt->DeInterlace.Enable = MMP_FALSE;

    //Video Windows
    isp_Memset((void *)(&(pISPctxt->ScaleFun)),  0, sizeof(ISP_SCALE_CTRL));

    //Frame Function
    isp_Memset((void *)(&(pISPctxt->FrameFun0)), 0, sizeof(ISP_FRMFUN_CTRL));

    //Output Information
    isp_Memset((void *)(&(pISPctxt->OutInfo)),   0, sizeof(ISP_OUTPUT_INFO));

    //Run-Length Enc
    isp_Memset((void *)(&(pISPctxt->RunLenEnc)), 0, sizeof(ISP_RUNLEN_ENC_CTRL));

    //Interrupt
    pISPctxt->EnableInterrupt = MMP_FALSE;

    //Context Init
    ISP_ContextInitialize(pISPctxt);
}

//=============================================================================
/**
 * Set or change ISP mode.
 *
 * @param  ispMode  indicate which mode will run.
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_RESULT
mmpIspSetMode(
    ISP_DEVICE   ptDev,
    MMP_ISP_MODE ispMode)
{
    ISP_RESULT  result    = ISP_SUCCESS;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    GOTO_END_IF_NOT_INITIALIZE(result, pISPctxt);

    //if ((pISPctxt->ispMode != MMP_ISP_MODE_NONE) &&
    //    (pISPctxt->ispMode != ispMode))
    //{
    //    // Reset the context.
    //    result = mmpIspInitialize(&ptDev);
    //    if (result)
    //    {
    //        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
    //        goto end;
    //    }
    //}

    pISPctxt->ispMode = ispMode;

    switch (pISPctxt->ispMode)
    {
    case MMP_ISP_MODE_SHOW_IMAGE:
        pISPctxt->EngineMode.EnableJPEGDECODE = MMP_TRUE;	
		if(ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 &&  ithIsTilingModeOn())
		{
			pISPctxt->InInfo.EnableRemapYAddr   = MMP_TRUE;
			pISPctxt->InInfo.EnableRemapUVAddr  = MMP_TRUE;	
		}
		else
		{
			pISPctxt->InInfo.EnableRemapYAddr   = MMP_FALSE;
			pISPctxt->InInfo.EnableRemapUVAddr  = MMP_FALSE;
		}
        break;

    case MMP_ISP_MODE_JPEG_TRANSFORM:
        pISPctxt->EngineMode.EnableJPEGDECODE = MMP_FALSE;
		if(ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 && ithIsTilingModeOn())
		{
			pISPctxt->InInfo.EnableRemapYAddr   = MMP_TRUE;
			pISPctxt->InInfo.EnableRemapUVAddr  = MMP_TRUE;	
		}
		else
		{
			pISPctxt->InInfo.EnableRemapYAddr   = MMP_FALSE;
			pISPctxt->InInfo.EnableRemapUVAddr  = MMP_FALSE;
		}
        break;

    case MMP_ISP_MODE_TRANSFORM:
        pISPctxt->EngineMode.EnableJPEGDECODE = MMP_FALSE;
        pISPctxt->InInfo.EnableRemapYAddr     = MMP_FALSE;
        pISPctxt->InInfo.EnableRemapUVAddr    = MMP_FALSE;
        break;

    case MMP_ISP_MODE_PLAY_VIDEO:
    case MMP_ISP_MODE_NONE:
        pISPctxt->EngineMode.EnableJPEGDECODE = MMP_FALSE;

#if !defined (CFG_CHIP_PKG_IT9852)
		if(ithIsTilingModeOn())
		{
			pISPctxt->InInfo.EnableRemapYAddr   = MMP_TRUE;
			pISPctxt->InInfo.EnableRemapUVAddr  = MMP_TRUE;	
		}
		else
		{
			pISPctxt->InInfo.EnableRemapYAddr   = MMP_FALSE;
			pISPctxt->InInfo.EnableRemapUVAddr  = MMP_FALSE;
		}    
#else
			pISPctxt->InInfo.EnableRemapYAddr   = MMP_TRUE;
			pISPctxt->InInfo.EnableRemapUVAddr  = MMP_TRUE;	
#endif
        break;
    }

    ISP_SetColorMatrix(pISPctxt);

    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_EngineMode;
    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputParameter;
    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_RemapAddr;

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * Enable ISP capability.
 *
 * @param  cap  Specifies a symbolic constant indicating a ISP capability.
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_RESULT
mmpIspEnable(
    ISP_DEVICE   ptDev,
    MMP_ISP_CAPS cap)
{
    ISP_RESULT  result    = ISP_SUCCESS;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    GOTO_END_IF_NOT_INITIALIZE(result, pISPctxt);

    switch (cap)
    {
    case MMP_ISP_DEINTERLACE:
        pISPctxt->DeInterlace.Enable = MMP_TRUE;
        pISPctxt->InInfo.UVRepeatMode = MMP_TRUE;
        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_DeInterlaceParam;
        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputParameter;
        break;

    case MMP_ISP_LOWLEVELEDGE:
        pISPctxt->DeInterlace.EnLowLevelEdge = MMP_TRUE;
        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_DeInterlaceParam;
        break;

    case MMP_ISP_UV_REPEAT_MODE:
        pISPctxt->InInfo.UVRepeatMode = MMP_TRUE;
        break;

    case MMP_ISP_FRAME_FUNCTION_0:
        pISPctxt->FrameFun0.Enable = MMP_TRUE;
        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_FrameFun0;
        break;
    //case MMP_ISP_REMAP_ADDRESS:
    //    pISPctxt->InInfo.EnableRemapYAddr = MMP_TRUE;
    //    pISPctxt->InInfo.EnableRemapUVAddr = MMP_TRUE;
    //    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputParameter;
    //    break;

    default:
        result = ISP_ERR_NO_MATCH_ENABLE_TYPE;
        break;
    }

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * Disable ISP capability.
 *
 * @param  cap  Specifies a symbolic constant indicating a ISP capability.
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_RESULT
mmpIspDisable(
    ISP_DEVICE   ptDev,
    MMP_ISP_CAPS cap)
{
    ISP_RESULT  result    = ISP_SUCCESS;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    GOTO_END_IF_NOT_INITIALIZE(result, pISPctxt);

    switch (cap)
    {
    case MMP_ISP_DEINTERLACE:
        pISPctxt->DeInterlace.Enable = MMP_FALSE;
        pISPctxt->InInfo.UVRepeatMode = MMP_FALSE;
        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_DeInterlaceParam;
        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputParameter;
        break;

    case MMP_ISP_LOWLEVELEDGE:
        pISPctxt->DeInterlace.EnLowLevelEdge = MMP_FALSE;
        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_DeInterlaceParam;
        break;

    case MMP_ISP_UV_REPEAT_MODE:
        pISPctxt->InInfo.UVRepeatMode = MMP_FALSE;
        break;

    case MMP_ISP_FRAME_FUNCTION_0:
        pISPctxt->FrameFun0.Enable = MMP_FALSE;
        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_FrameFun0;
        break;

    //case MMP_ISP_REMAP_ADDRESS:
    //    pISPctxt->InInfo.EnableRemapYAddr = MMP_FALSE;
    //    pISPctxt->InInfo.EnableRemapUVAddr = MMP_FALSE;
    //    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputParameter;
    //    break;

    default:
        //result = ISP_ERR_NO_MATCH_ENABLE_TYPE;
        break;
    }

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
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
mmpIspQuery(
    ISP_DEVICE   ptDev,
    MMP_ISP_CAPS cap)
{
    MMP_BOOL    result    = MMP_FALSE;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    GOTO_END_IF_NOT_INITIALIZE(result, pISPctxt);

    switch (cap)
    {
    case MMP_ISP_DEINTERLACE:
        result = pISPctxt->DeInterlace.Enable;
        break;

    case MMP_ISP_LOWLEVELEDGE:
        result = pISPctxt->DeInterlace.EnLowLevelEdge;
        break;

    case MMP_ISP_UV_REPEAT_MODE:
        result = pISPctxt->InInfo.UVRepeatMode;
        break;

        //case MMP_ISP_REMAP_ADDRESS:
        //    result = pISPctxt->InInfo.EnableRemapYAddr;
        break;

    default:
        result = MMP_FALSE;
        break;
    }

end:
    return result;
}

//=============================================================================
/**
 * Set the value of the specified color control type.
 *
 * @param  colorControl  Specifies a color control type.
 * @param  value         The adjusted value related to specified color control.
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 *
 * @remark Calling the mmpIspUpdate() method to active it.
 * @see MMP_ISP_COLOR_CONTROL
 */
//=============================================================================
ISP_RESULT
mmpIspSetColorCtrl(
    ISP_DEVICE         ptDev,
    MMP_ISP_COLOR_CTRL colorCtrl,
    MMP_INT            value)
{
    ISP_RESULT  result    = ISP_SUCCESS;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    GOTO_END_IF_NOT_INITIALIZE(result, pISPctxt);

    switch (colorCtrl)
    {
    case MMP_ISP_HUE:
        if (OUT_OF_RANGE(value, 0, 359))
        {
            result = ISP_ERR_COLOR_CTRL_OUT_OF_RANGE;
            isp_msg_ex(ISP_MSG_TYPE_ERR, "Hue value (%d) out range !!", value);
        }
        else
            pISPctxt->hue = (MMP_INT16) value;
        break;

    case MMP_ISP_CONTRAST:
        if (OUT_OF_RANGE(value, -64, 63))
        {
            result = ISP_ERR_COLOR_CTRL_OUT_OF_RANGE;
            isp_msg_ex(ISP_MSG_TYPE_ERR, "Contrast value (%d) out range !!", value);
        }
        else
            pISPctxt->contrast = (MMP_INT16) value;
        break;

    case MMP_ISP_SATURATION:
        if (OUT_OF_RANGE(value, 0, 255))
        {
            result = ISP_ERR_COLOR_CTRL_OUT_OF_RANGE;
            isp_msg_ex(ISP_MSG_TYPE_ERR, "Saturation value (%d) out range !!", value);
        }
        else
            pISPctxt->saturation = (MMP_INT16) value;
        break;

    case MMP_ISP_BRIGHTNESS:
        if (OUT_OF_RANGE(value, -64, 63))
        {
            result = ISP_ERR_COLOR_CTRL_OUT_OF_RANGE;
            isp_msg_ex(ISP_MSG_TYPE_ERR, "Brightness value (%d) out range !!", value);
        }
        else
            pISPctxt->brightness = (MMP_INT16) value;
        break;

    default:
        result = ISP_ERR_UNSUPPORTED_COLOR_CTRL;
        isp_msg_ex(ISP_MSG_TYPE_ERR, "Wrong color type !!");
        break;
    }

    if (result == ISP_SUCCESS)
    {
        ISP_SetColorCorrMatrix((void *)&pISPctxt->CCFun,
                               pISPctxt->hue,
                               pISPctxt->saturation,
                               pISPctxt->contrast,
                               pISPctxt->midPoint,
                               pISPctxt->colorEffect,
                               pISPctxt->brightness,
                               0);
    }

    pISPctxt->InInfo.EnableCCFun = MMP_TRUE;
    pISPctxt->UpdateFlags       |= ISP_FLAGS_UPDATE_CCMatrix;
    pISPctxt->UpdateFlags       |= ISP_FLAGS_UPDATE_InputParameter;

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * Set display window
 *
 * @param vramAddr base address of the destination buffer.
 * @param startX   starting X position at the output buffer
 * @param startY   starting Y position at the output buffer
 * @param width    width of content at the output buffer
 * @param height   height of content at the output buffer
 * @param pitch    pitch of the destination buffer
 * @param format   format of the destination buffer.
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_RESULT
mmpIspSetOutputWindow(
    ISP_DEVICE          ptDev,
    MMP_ISP_OUTPUT_INFO *outInfo)
{
    ISP_RESULT  result       = ISP_SUCCESS;
    ISP_CONTEXT *pISPctxt    = (ISP_CONTEXT *)ptDev;
    MMP_UINT32  offset       = 0;
    MMP_UINT16  bytePrePixel = 0;

    GOTO_END_IF_NOT_INITIALIZE(result, pISPctxt);

    if (outInfo == MMP_NULL)
    {
        result = ISP_ERR_INVALID_INPUT_PARAM;
        isp_msg_ex(ISP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    if ((outInfo->width == 0) || (outInfo->height == 0))
    {
        result = ISP_ERR_INVALID_DISPLAY_WINDOW;
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err outW = %d, outH = %d !\n", outInfo->width, outInfo->height);
        goto end;
    }

    mmpIspSetOutputFormat(ptDev, outInfo->format);

    switch (outInfo->format)
    {
    case MMP_ISP_OUT_DITHER444:
    case MMP_ISP_OUT_DITHER565:
        bytePrePixel             = 2;
        pISPctxt->OutInfo.Pitch  = (MMP_UINT16)outInfo->pitchRGB;
        pISPctxt->OutInfo.Width  = (MMP_UINT16)outInfo->width;
        pISPctxt->OutInfo.Height = (MMP_UINT16)outInfo->height;

        offset                   = outInfo->startY * pISPctxt->OutInfo.Pitch + outInfo->startX * bytePrePixel;

        pISPctxt->OutInfo.Addr   = (MMP_UINT8 *)((MMP_UINT32)outInfo->addrRGB + offset);
        break;

    case MMP_ISP_OUT_RGB888:
        bytePrePixel             = 4;
        pISPctxt->OutInfo.Pitch  = (MMP_UINT16)outInfo->pitchRGB;
        pISPctxt->OutInfo.Width  = (MMP_UINT16)outInfo->width;
        pISPctxt->OutInfo.Height = (MMP_UINT16)outInfo->height;

        offset                   = outInfo->startY * pISPctxt->OutInfo.Pitch + outInfo->startX * bytePrePixel;

        pISPctxt->OutInfo.Addr   = (MMP_UINT8 *)((MMP_UINT32)outInfo->addrRGB + offset);
        break;

    //case MMP_ISP_OUT_YUV420:
    //case MMP_ISP_OUT_YUV422:
    //    pISPctxt->OutInfo.PitchYRGB = (MMP_UINT16)outInfo->pitchY;
    //    pISPctxt->OutInfo.PitchUV   = (MMP_UINT16)outInfo->pitchUv;
    //    pISPctxt->OutInfo.Width     = (MMP_UINT16)outInfo->width;
    //    pISPctxt->OutInfo.Height    = (MMP_UINT16)outInfo->height;
    //
    //    pISPctxt->OutInfo.Addr0 = (MMP_UINT8*)outInfo->addrY;
    //    pISPctxt->OutInfo.Addr1 = (MMP_UINT8*)outInfo->addrU;
    //    pISPctxt->OutInfo.Addr2 = (MMP_UINT8*)outInfo->addrV;
    //    break;

    default:
        result = ISP_ERR_NO_MATCH_OUTPUT_FORMAT;
        goto end;
        break;
    }

    //pISPctxt->OutInfo.EnableDoubleBuf = MMP_FALSE;
    //pISPctxt->OutInfo.EnableTripleBuf = MMP_FALSE;

    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutParameter;
    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutBufInfo;
    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutAddress;

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * Set display window
 *
 * @param startX  starting X position for display
 * @param startY  starting Y position for display
 * @param width   width of window
 * @param height  height of window
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_RESULT
mmpIspSetDisplayWindow(
    ISP_DEVICE ptDev,
    MMP_INT    startX,
    MMP_INT    startY,
    MMP_UINT   width,
    MMP_UINT   height)
{
    ISP_RESULT  result    = ISP_SUCCESS;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;
    MMP_UINT32  offset;
    MMP_UINT16  pitch;

    GOTO_END_IF_NOT_INITIALIZE(result, pISPctxt);

    if ( (width == 0) || (height == 0) )
    {
        result = ISP_ERR_INVALID_DISPLAY_WINDOW;
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err outW = %d, outH = %d !\n", width, height);
        goto end;
    }

    pitch                    = (MMP_UINT16) isp_LcdGetPitch();
    pISPctxt->OutInfo.Pitch  = (MMP_UINT16)pitch;
    pISPctxt->OutInfo.Width  = (MMP_UINT16)width;
    pISPctxt->OutInfo.Height = (MMP_UINT16)height;

    offset                   = startY * pISPctxt->OutInfo.Pitch + startX * 2;

    pISPctxt->OutInfo.Addr   = (MMP_UINT8 *)(isp_LcdGetBaseAddr_A() + offset);

    //pISPctxt->OutInfo.EnableDoubleBuf = MMP_FALSE;
    //pISPctxt->OutInfo.EnableTripleBuf = MMP_FALSE;

//    if (pISPctxt->ispMode == MMP_ISP_MODE_PLAY_VIDEO || pISPctxt->ispMode == MMP_ISP_MODE_MOTION_JPEG)
//    {
//        pISPctxt->OutInfo.Addr1 = (MMP_UINT8*)(isp_LcdGetBaseAddr_B() + offset);
//        pISPctxt->OutInfo.EnableDoubleBuf = MMP_TRUE;
//        pISPctxt->OutInfo.EnableTripleBuf = MMP_FALSE;
//
//        //Set LCD VideoFlip Mode
//        isp_LcdEnableVideoFlip(MMP_TRUE);
//
//#if defined(LCD_TRIPLE_BUFFER)
//    #error "Lcd no LCD_PROPERTY_BASE_C"
//        pISPctxt->OutInfo.Addr2 = (MMP_UINT8*)(isp_LcdGetBaseAddr_C() + offset);
//        pISPctxt->OutInfo.EnableDoubleBuf = MMP_FALSE;
//        pISPctxt->OutInfo.EnableTripleBuf = MMP_TRUE;
//#endif
//
//        //Set Scale Output Size
//        mmpIspSetVideoWindow(pISPctxt, 0, 0, width, height);
//    }

    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutParameter;
    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutBufInfo;
    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutAddress;

    pISPctxt->WorkFlags   |= ISP_FLAGS_FIRST_VIDEO_FRAME;

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * Set display window
 *
 * @param startX  starting X position for display
 * @param startY  starting Y position for display
 * @param width   width of window
 * @param height  height of window
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_RESULT
mmpIspSetVideoWindow(
    ISP_DEVICE ptDev,
    MMP_UINT   startX,
    MMP_UINT   startY,
    MMP_UINT   width,
    MMP_UINT   height)
{
    ISP_RESULT  result    = ISP_SUCCESS;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    GOTO_END_IF_NOT_INITIALIZE(result, pISPctxt);

    if ((width == 0) || (height == 0) ||
        (pISPctxt->OutInfo.Width < (startX + width)) ||
        (pISPctxt->OutInfo.Height < (startY + height)))
    {
        result = ISP_ERR_INVALID_DISPLAY_WINDOW;
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err param(x,y,w,h)=(%d, %d, %d, %d), out(w,h)=(%d, %d) !\n",
                   startX, startY, width, height,
                   pISPctxt->OutInfo.Width,
                   pISPctxt->OutInfo.Height);
        goto end;
    }

    pISPctxt->ScaleFun.DstPosX   = (MMP_UINT16)startX;
    pISPctxt->ScaleFun.DstPosY   = (MMP_UINT16)startY;
    pISPctxt->ScaleFun.DstWidth  = (MMP_UINT16)width;
    pISPctxt->ScaleFun.DstHeight = (MMP_UINT16)height;

    pISPctxt->UpdateFlags       |= ISP_FLAGS_UPDATE_ScaleParam;

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * Set isp output format.
 **/
//=============================================================================
ISP_RESULT
mmpIspSetOutputFormat(
    ISP_DEVICE        ptDev,
    MMP_ISP_OUTFORMAT format)
{
    ISP_RESULT  result    = ISP_SUCCESS;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    GOTO_END_IF_NOT_INITIALIZE(result, pISPctxt);

    switch (format)
    {
    case MMP_ISP_OUT_DITHER565:
        //pISPctxt->OutInfo.OutFormat = RGBPacket;
        pISPctxt->OutInfo.RGBFormat = Dither565;
        break;

    case MMP_ISP_OUT_DITHER444:
        //pISPctxt->OutInfo.OutFormat = RGBPacket;
        pISPctxt->OutInfo.RGBFormat = Dither444;
        break;

    case MMP_ISP_OUT_RGB888:
        //pISPctxt->OutInfo.OutFormat = RGBPacket;
        pISPctxt->OutInfo.RGBFormat = NoDither888;
        break;

    //case MMP_ISP_OUT_YUYV:
    //    //pISPctxt->OutInfo.OutFormat = YUVPacket;
    //    pISPctxt->OutInfo.PacketFormat = YUYV;
    //    break;
    //
    //case MMP_ISP_OUT_YVYU:
    //    //pISPctxt->OutInfo.OutFormat = YUVPacket;
    //    pISPctxt->OutInfo.PacketFormat = YVYU;
    //    break;
    //
    //case MMP_ISP_OUT_UYVY:
    //    //pISPctxt->OutInfo.OutFormat = YUVPacket;
    //    pISPctxt->OutInfo.PacketFormat = UYVY;
    //    break;
    //
    //case MMP_ISP_OUT_VYUY:
    //    //pISPctxt->OutInfo.OutFormat = YUVPacket;
    //    pISPctxt->OutInfo.PacketFormat = VYUY;
    //    break;
    //
    //case MMP_ISP_OUT_YUV422:
    //    //pISPctxt->OutInfo.OutFormat = YUVPlane;
    //    pISPctxt->OutInfo.PlaneFormat = YUV422;
    //    break;
    //
    //case MMP_ISP_OUT_YUV420:
    //    //pISPctxt->OutInfo.OutFormat = YUVPlane;
    //    pISPctxt->OutInfo.PlaneFormat = YUV420;
    //    break;
    //
    //case MMP_ISP_OUT_YUV444:
    //    //pISPctxt->OutInfo.OutFormat = YUVPlane;
    //    pISPctxt->OutInfo.PlaneFormat = YUV444;
    //    break;
    //
    //case MMP_ISP_OUT_YUV422R:
    //    //pISPctxt->OutInfo.OutFormat = YUVPlane;
    //    pISPctxt->OutInfo.PlaneFormat = YUV422R;
    //    break;

    default:
        result = ISP_ERR_NO_MATCH_OUTPUT_FORMAT;
        break;
    }

    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_OutParameter;

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * JPEG decoder use this interface to set the buffer information.
 *
 * @param data     share data between JPEG and DSC.
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_RESULT
mmpIspPlayImageProcess(
    ISP_DEVICE          ptDev,
    const MMP_ISP_SHARE *data)
{
    ISP_RESULT     result    = ISP_SUCCESS;
    ISP_CONTEXT    *pISPctxt = (ISP_CONTEXT *)ptDev;
    ISP_INPUT_INFO *pInInfo;
    MMP_UINT16     Temp      = 0;
    MMP_UINT32     i, reMapAdr;

    GOTO_END_IF_NOT_INITIALIZE(result, pISPctxt);
    pthread_mutex_lock(&pISPMutex);
    pInInfo = &pISPctxt->InInfo;

	result = ISP_WaitEngineIdle();
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }
    //
    //Set Input Address, Width, Height and Pitch
    //
    
    if (pISPctxt->InInfo.EnableRemapYAddr)
    {
        pISPctxt->RemapTableYIdx = ReMapTableSel(data->pitchY);
        reMapAdr                 = 0;
        for (i = 0; i < 32; i++)
            reMapAdr |= ((((data->addrY) >> ISPTilingTable[pISPctxt->RemapTableYIdx][i]) & 0x1) << i);

        pInInfo->AddrY = (MMP_UINT8 *)reMapAdr;
    }
    else
    {
        pInInfo->AddrY = (MMP_UINT8 *)data->addrY;
    }

    if (pISPctxt->InInfo.EnableRemapUVAddr)
    {
        pISPctxt->RemapTableUVIdx = ReMapTableSel(data->pitchUv);
        reMapAdr                  = 0;
        for (i = 0; i < 32; i++)
            reMapAdr |= (((data->addrV) >> ISPTilingTable[pISPctxt->RemapTableUVIdx][i]) & 0x1) << i;

        pInInfo->AddrV = (MMP_UINT8 *)reMapAdr;

        reMapAdr       = 0;
        for (i = 0; i < 32; i++)
            reMapAdr |= (((data->addrU) >> ISPTilingTable[pISPctxt->RemapTableUVIdx][i]) & 0x1) << i;

        pInInfo->AddrU = (MMP_UINT8 *)reMapAdr;
    }
    else
    {
        pInInfo->AddrV = (MMP_UINT8 *)data->addrV;
        pInInfo->AddrU = (MMP_UINT8 *)data->addrU;
    }

    //pInInfo->AddrY = (MMP_UINT8*)data->addrY;
    //pInInfo->AddrV = (MMP_UINT8*)data->addrV;
    //pInInfo->AddrU = (MMP_UINT8*)data->addrU;

    // isp line base mode limit: width must be 2 alignment
    pInInfo->SrcWidth  = (data->width >> 2) << 2;

    pInInfo->SrcHeight = data->height;
    pInInfo->PitchY    = data->pitchY;
    pInInfo->PitchUV   = data->pitchUv;

    //pInInfo->SrcPosX = pInInfo->SrcExtedLeft;
    //pInInfo->SrcPosY = pInInfo->SrcExtedTop;
    //pInInfo->PanelWidth = pInInfo->SrcWidth + pInInfo->SrcExtedLeft + pInInfo->SrcExtedRight;
    //pInInfo->PanelHeight = pInInfo->SrcHeight + pInInfo->SrcExtedTop + pInInfo->SrcExtedDown;

    //pISPctxt->top_field_first = 1;

    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputBuf;
    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputAddr;

    //
    // Set Input Format
    //
    result = ISP_SetInputFormat(pISPctxt, data->format);
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    //
    //Set JPEG Slice Number
    //
    pISPctxt->EngineMode.TotalSliceNum = data->sliceCount;
    pISPctxt->UpdateFlags             |= ISP_FLAGS_UPDATE_EngineMode;

    //mmpIspDisable(MMP_ISP_KEEP_LAST_FIELD_MODE);

    //
    //CMYK Color Format
    //
    //if (data->isAdobe_CMYK)
    //    mmpIspEnable(MMP_ISP_NEGATIVE_EFFECT_MODE);
    //else
    //    mmpIspDisable(MMP_ISP_NEGATIVE_EFFECT_MODE);

    //result = ISP_WaitEngineIdle();
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    //if (pISPctxt->OutInfo.RotateType == Deg90 ||
    //    pISPctxt->OutInfo.RotateType == Deg270 )
    //{
    //    Temp = pISPctxt->OutInfo.Width;
    //    pISPctxt->OutInfo.Width = pISPctxt->OutInfo.Height;
    //    pISPctxt->OutInfo.Height = Temp;
    //}

    //Set Scale Output Size
    //mmpIspSetVideoWindow(pISPctxt, 0, 0, pISPctxt->OutInfo.Width, pISPctxt->OutInfo.Height);

    result = ISP_Update(pISPctxt);
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    //Fire ISP
    //ISP_LogReg();
    ISP_DriverFire_Reg();
end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);
    pthread_mutex_unlock(&pISPMutex);
    return result;
}

//=============================================================================
/**
 * MPEG decoder use this interface to set the buffer information.
 *
 * @param data     share data between MPEG and ISP.
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_RESULT
mmpIspPlayVideoProcess(
    ISP_DEVICE          ptDev,
    const MMP_ISP_SHARE *data)
{
    ISP_RESULT     result     = ISP_SUCCESS;
    ISP_CONTEXT    *pISPctxt  = (ISP_CONTEXT *)ptDev;
    ISP_INPUT_INFO *pInInfo;
    MMP_UINT16     status     = 0;
    MMP_UINT16     align_width, align_height;
    MMP_UINT16     DOWNSAMPLE = 2;
    MMP_UINT32     i, reMapAdr;

    GOTO_END_IF_NOT_INITIALIZE(result, pISPctxt);

    pInInfo      = &pISPctxt->InInfo;
    // isp limit: width must be 8 alignment
    align_width  = (data->width >> 3) << 3;
    align_height = (data->height >> 1) << 1;

    if (pInInfo->SrcWidth != align_width || pInInfo->SrcHeight != align_height)
    {
        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputBuf;
        pISPctxt->WorkFlags |= ISP_FLAGS_FIRST_VIDEO_FRAME;
    }

    // update buffer address
    if (pISPctxt->InInfo.EnableRemapYAddr)
    {
        pISPctxt->RemapTableYIdx = ReMapTableSel(data->pitchY);
        reMapAdr                 = 0;
        for (i = 0; i < 32; i++)
            reMapAdr |= ((((data->addrY) >> ISPTilingTable[pISPctxt->RemapTableYIdx][i]) & 0x1) << i);

        pInInfo->AddrY = (MMP_UINT8 *)reMapAdr;
    }
    else
    {
        pInInfo->AddrY = (MMP_UINT8 *)data->addrY;
    }

    if (pISPctxt->InInfo.EnableRemapUVAddr)
    {
        pISPctxt->RemapTableUVIdx = ReMapTableSel(data->pitchUv);
        reMapAdr                  = 0;
        for (i = 0; i < 32; i++)
            reMapAdr |= (((data->addrV) >> ISPTilingTable[pISPctxt->RemapTableUVIdx][i]) & 0x1) << i;

        pInInfo->AddrV = (MMP_UINT8 *)reMapAdr;

        reMapAdr       = 0;
        for (i = 0; i < 32; i++)
            reMapAdr |= (((data->addrU) >> ISPTilingTable[pISPctxt->RemapTableUVIdx][i]) & 0x1) << i;

        pInInfo->AddrU = (MMP_UINT8 *)reMapAdr;
    }
    else
    {
        pInInfo->AddrV = (MMP_UINT8 *)data->addrV;
        pInInfo->AddrU = (MMP_UINT8 *)data->addrU;
    }

    if (pISPctxt->InInfo.EnableRemapYAddr)
    {
        pISPctxt->RemapTableYIdx = ReMapTableSel(data->pitchY);
        reMapAdr                 = 0;
        for (i = 0; i < 32; i++)
            reMapAdr |= ((((data->addrYp) >> ISPTilingTable[pISPctxt->RemapTableYIdx][i]) & 0x1) << i);

        pInInfo->AddrYp = (MMP_UINT8 *)reMapAdr;
    }
    else
    {
        pInInfo->AddrYp = (MMP_UINT8 *)data->addrYp;
    }

    //pInInfo->AddrY  = (MMP_UINT8*)data->addrY;
    //pInInfo->AddrV  = (MMP_UINT8*)data->addrV;
    //pInInfo->AddrU  = (MMP_UINT8*)data->addrU;
    //pInInfo->AddrYp = (MMP_UINT8*)data->addrYp;
    //pInInfo->AddrVp  = (MMP_UINT8*)data->addrVp;
    //pInInfo->AddrUp  = (MMP_UINT8*)data->addrUp;

    //ISPctxt->Mpeg2TopBufferIndex = data->Mpeg2TopBufferIndex;
    //ISPctxt->Mpeg2BotBufferIndex = data->Mpeg2BotBufferIndex;
    //ISPctxt->top_field_first = data->top_field_first;
    //ISPctxt->Blank_Buffer_Index = data->Blank_Buffer_Index;

    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputAddr;
    //pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_Mpeg2BufferIdx;

    //set width, height, pitch at the first frame
    if (pISPctxt->WorkFlags & ISP_FLAGS_FIRST_VIDEO_FRAME)
    {
        //
        //Set Input Format
        //
        result = ISP_SetInputFormat(pISPctxt, data->format);
        if (result)
        {
            isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
            goto end;
        }

        //if (data->width == VIDEO_BLANK_WIDTH && data->height == VIDEO_BLANK_HEIGHT)
        //{
        //    mmpIspEnable(MMP_ISP_DISABLE_VIDEO_OUT);
        //}
        //else
        //{
        //    mmpIspDisable(MMP_ISP_DISABLE_VIDEO_OUT);
        //}

        pInInfo->SrcWidth      = data->width;

        pInInfo->SrcHeight     = (data->height >> 1) << 1;
        pInInfo->PitchY        = data->pitchY;
        pInInfo->PitchUV       = data->pitchUv;

        pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputBuf;

        // isp limit: width must be 8 alignment
        if (pInInfo->SrcWidth & 0x07)
        {
            pInInfo->SrcWidth = (pInInfo->SrcWidth >> 3) << 3;
            pInInfo->PitchY   = (pInInfo->PitchY >> 3) << 3;
            pInInfo->PitchUV  = (pInInfo->PitchUV >> 3) << 3;
        }
    }

//#if 0
//    if (ISPctxt->OutInfo.EnableFieldScale
//     && (pInInfo->SrcWidth > VIDEO_BLANK_WIDTH && pInInfo->SrcHeight > VIDEO_BLANK_HEIGHT)
//     && (pInInfo->SrcWidth  != ISPctxt->OutInfo.Width
//     ||  pInInfo->SrcHeight != ISPctxt->OutInfo.Height)
//     && data->isProgressive
//     && (!ISPctxt->OutInfo.EnableVideoPreview))
//    {
//        ISPctxt->OutInfo.EnableProgFieldMode = MMP_TRUE;
//    }
//    else
//#endif
//        ISPctxt->OutInfo.EnableProgFieldMode = MMP_FALSE;

    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_InputBuf;

    //if (pISPctxt->WorkFlags & ISP_FLAGS_FIRST_VIDEO_FRAME)
    //{
    //    mmpIspDisable(MMP_ISP_SUBTITLE_0);
    //    mmpIspDisable(MMP_ISP_SUBTITLE_1);
    //}

    result = ISP_WaitEngineIdle();
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    //if (data->EnableKeepLastField == MMP_TRUE
    // && pISPctxt->OutInfo.EnableFieldScale == MMP_TRUE
    // && (!data->isProgressive))
    //    mmpIspEnable(MMP_ISP_KEEP_LAST_FIELD_MODE);
    //else
    //    mmpIspDisable(MMP_ISP_KEEP_LAST_FIELD_MODE);

    //pISPctxt->DeInterlace.EnSrcBottomFieldFirst = !(pISPctxt->top_field_first);
    //if (!pISPctxt->OutInfo.EnableFieldScale)
    //{
    //    if (pISPctxt->DeInterlace.EnSrcBottomFieldFirst == 0)
    //        pISPctxt->DeInterlace.EnDeinterBottomField = 1;
    //    else
    //        pISPctxt->DeInterlace.EnDeinterBottomField = 0;
    //    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_DeInterlaceParam;
    //}

    result = ISP_Update(pISPctxt);
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    ISP_DriverFire_Reg();

    if (pISPctxt->WorkFlags & ISP_FLAGS_FIRST_VIDEO_FRAME)
        pISPctxt->WorkFlags |= ISP_FLAGS_SECOND_VIDEO_FRAME;

    pISPctxt->WorkFlags &= ~ISP_FLAGS_FIRST_VIDEO_FRAME;

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
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
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 *
 * @see mmpIspEnable() mmpIspDisable()
 */
//=============================================================================
ISP_RESULT
mmpIspSetFrameFunction(
    ISP_DEVICE       ptDev,
    void             *vramAddr,
    MMP_UINT         startX,
    MMP_UINT         startY,
    MMP_UINT         width,
    MMP_UINT         height,
    MMP_UINT         pitch,
    MMP_UINT         colorKeyR,
    MMP_UINT         colorKeyG,
    MMP_UINT         colorKeyB,
    MMP_UINT         constantAlpha,
    MMP_PIXEL_FORMAT format,
    MMP_UINT         uiBufferIndex)
{
    ISP_RESULT      result    = ISP_SUCCESS;
    ISP_CONTEXT     *pISPctxt = (ISP_CONTEXT *)ptDev;
    ISP_FRMFUN_CTRL *pIspFrameFunc0;

    GOTO_END_IF_NOT_INITIALIZE(result, pISPctxt);

    if (width < 16 || height < 16)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err, width(%d) < 16 or height(%d) < 16 !!", width, height);
        result = ISP_ERR_INVALID_PARAM;
        goto end;
    }

    pIspFrameFunc0 = &pISPctxt->FrameFun0;

    startX        -= (startX & 0x1); // startX%2;
    startY        -= (startY & 0x1); // startY%2;
    width         += (width & 0x1);  // width%2;
    height        += (height & 0x1); // height%2;

    switch (format)
    {
    case MMP_PIXEL_FORMAT_ARGB4444:
        pIspFrameFunc0->Format = ARGB4444;
        break;

    case MMP_PIXEL_FORMAT_ARGB8888:
        pIspFrameFunc0->Format = ARGB8888;
        break;

    case MMP_PIXEL_FORMAT_RGB565:
        pIspFrameFunc0->Format = CARGB565;
        break;

    default:
        result = ISP_ERR_NO_MATCH_OUTPUT_FORMAT;
        goto end;
        break;
    }

    pIspFrameFunc0->ColorKeyR     = (MMP_UINT16) colorKeyR;
    pIspFrameFunc0->ColorKeyG     = (MMP_UINT16) colorKeyG;
    pIspFrameFunc0->ColorKeyB     = (MMP_UINT16) colorKeyB;
    pIspFrameFunc0->ConstantAlpha = (MMP_UINT16) constantAlpha;
    pIspFrameFunc0->StartX        = (MMP_UINT16) startX;
    pIspFrameFunc0->StartY        = (MMP_UINT16) startY;
    pIspFrameFunc0->Width         = (MMP_UINT16) width;
    pIspFrameFunc0->Height        = (MMP_UINT16) height;

    pIspFrameFunc0->Pitch         = (MMP_UINT16) pitch;
    pIspFrameFunc0->Addr          = (MMP_UINT8 *) ((MMP_UINT) vramAddr);

    pISPctxt->UpdateFlags        |= ISP_FLAGS_UPDATE_FrameFun0;

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * Set clipping window (only for display)
 * @param id  = 1 or 2 or 3
 * @param type  Specify the clip type. Clip inside or outside.
 * @param startX  x coordinate of clipping window
 * @param startY    y coordinate of clipping window
 * @param width     width of clipping window
 * @param height    height of clipping window
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_RESULT
mmpIspSetClipWindow(
    ISP_DEVICE        ptDev,
    MMP_UINT          id,
    MMP_ISP_CLIP_TYPE type,
    MMP_UINT          startX,
    MMP_UINT          startY,
    MMP_UINT          width,
    MMP_UINT          height)
{
    ISP_RESULT result = ISP_SUCCESS;
    //TODO

    return result;
}

//=============================================================================
/**
 * Rotate image +90, -90, 180, flip, or mirror. It will apply to display.
 *
 * @param type      Specify the rotation type.
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_RESULT
mmpIspSetDisplayRotateType(
    ISP_DEVICE          ptDev,
    MMP_ISP_ROTATE_TYPE type)
{
    ISP_RESULT result = ISP_SUCCESS;

    //TODO

    return result;
}

//=============================================================================
/**
 * ISP Fire
 *
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_RESULT
mmpIspFire(
    ISP_DEVICE ptDev)
{
    ISP_RESULT result = ISP_SUCCESS;
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    GOTO_END_IF_NOT_INITIALIZE(result, pISPctxt);

    // Update parameter
    result = ISP_Update(pISPctxt);
    if (result)
    {
        isp_msg_ex(ISP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    // Fire ISP
    // ISP_LogReg();
    ISP_DriverFire_Reg();

end:
    if (result)
        isp_msg_ex(ISP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return result;
}

//=============================================================================
/**
 * Wait ISP Engine Idle
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_RESULT
mmpIspWaitEngineIdle(
    void)
{
    return ISP_WaitEngineIdle();
}

MMP_BOOL
mmpIspIsEngineIdle(
    void)
{
    MMP_UINT16 status = 0;

    //
    //  IS ISP engine idle?   0x6FC D[0]  0: idle, 1: busy
    //
    isp_ReadHwReg(ISP_REG_ISP_ENGINE_STATUS, (MMP_UINT16 *)&status);
    return !(status & 0x0001);
}

//=============================================================================
/**
 * mmpIspRegisterIRQ.
 */
//=============================================================================
//#ifndef WIN32
//void
//mmpIspRegisterIRQ(
//    Isp_handler isphandler)
//{
//    // Initialize ISP IRQ
//    ithIntrDisableIrq(ITH_INTR_ISP);
//    ithIntrClearIrq(ITH_INTR_ISP);
//
//    #if defined (__FREERTOS__)
//    // register NAND Handler to IRQ
//    ithIntrRegisterHandlerIrq(ITH_INTR_ISP, isphandler, MMP_NULL);
//    #endif // defined (__FREERTOS__)
//
//    // set IRQ to edge trigger
//    ithIntrSetTriggerModeIrq(ITH_INTR_ISP, ITH_INTR_EDGE);
//
//    // set IRQ to detect rising edge
//    ithIntrSetTriggerLevelIrq(ITH_INTR_ISP, ITH_INTR_HIGH_RISING);
//
//    // Enable IRQ
//    ithIntrEnableIrq(ITH_INTR_ISP);
//}
//#endif

//=============================================================================
/**
 * Enable ISP Interrupt.
 */
//=============================================================================
void
mmpIspEnableInterrupt(
    ISP_DEVICE ptDev)
{
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    RETURN_IF_NOT_INITIALIZE(pISPctxt);

    isp_WriteHwRegMask(ISP_REG_SET50E, ((0x1 & ISP_BIT_ISP_INTERRUPT_EN) << ISP_SHT_ISP_INTERRUPT_EN), (ISP_BIT_ISP_INTERRUPT_EN << ISP_SHT_ISP_INTERRUPT_EN));

    if (pISPctxt != MMP_NULL)
        pISPctxt->EnableInterrupt = MMP_FALSE;

    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_Interrupt;
}

//=============================================================================
/**
 * Disable ISP Interrupt.
 */
//=============================================================================
void
mmpIspDisableInterrupt(
    ISP_DEVICE ptDev)
{
    ISP_CONTEXT *pISPctxt = (ISP_CONTEXT *)ptDev;

    RETURN_IF_NOT_INITIALIZE(pISPctxt);

    isp_WriteHwRegMask(ISP_REG_SET50E, ((0x0 & ISP_BIT_ISP_INTERRUPT_EN) << ISP_SHT_ISP_INTERRUPT_EN), (ISP_BIT_ISP_INTERRUPT_EN << ISP_SHT_ISP_INTERRUPT_EN));

    if (pISPctxt != MMP_NULL)
        pISPctxt->EnableInterrupt = MMP_FALSE;

    pISPctxt->UpdateFlags |= ISP_FLAGS_UPDATE_Interrupt;
}

//=============================================================================
/**
 * Clear ISP Interrupt
 */
//=============================================================================
void
mmpIspClearInterrupt(
    void)
{
    ISP_ClearInterrupt_Reg();
}

//=============================================================================
/**
 * Wait ISP Interrupt Idle
 * @return ISP_RESULT_SUCCESS if succeed, error codes of ISP_RESULT_ERROR otherwise.
 */
//=============================================================================
ISP_RESULT
mmpIspWaitInterruptIdle(
    void)
{
    return ISP_WaitInterruptIdle();
}

//=============================================================================
/**
 * mmpIspResetEngine
 */
//=============================================================================
ISP_RESULT
mmpIspResetEngine(
    void)
{
    ISP_RESULT result = ISP_SUCCESS;

    isp_EnginReset();

    return result;
}