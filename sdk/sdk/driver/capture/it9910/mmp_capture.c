//#include "host/host.h"
//#include "pal/pal.h"
//#include "sys/sys.h"
//#include "mmp_types.h"

#include "capture_config.h"
#include "capture/capture_types.h"
#include "capture_reg.h"
#include "capture_hw.h"
#include "capture.h"
#include "capture/video_device_table.h"
#include "capture/mmp_capture.h"
#include "ite/ith.h"
#include "vp/mmp_vp.h"

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
CAP_CONTEXT       *Capctxt     = MMP_NULL;
MMP_CAP_DEVICE_ID INPUT_DEVICE = MMP_CAP_UNKNOW_DEVICE;

static MMP_UINT16 ROIPosX;
static MMP_UINT16 ROIPosY;
static MMP_UINT16 ROIWidth;
static MMP_UINT16 ROIHeight;

MMP_CAP_SHARE     DevInfo;

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

#if defined (SENSOR_DEV)
    #include "device_sensor.c"
#else
    #include "device_video.c"
#endif

//=============================================================================
/**
 * Cap context initialization.
 */
//=============================================================================
MMP_RESULT
mmpCapInitialize(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_UINT16 Value;

    if (Capctxt == MMP_NULL)
    {
        Capctxt = PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(CAP_CONTEXT));
        if (Capctxt == MMP_NULL)
        {
            result = MMP_RESULT_ERROR;
            cap_msg_ex(CAP_MSG_TYPE_ERR, "Allocate memeory fail\n");
            goto end;
        }
    }

    if (mmpCapGetDeviceReboot()) //for keep loopthrough
        Cap_Engine_Register_Reset();

    PalMemset((void *)Capctxt, 0, sizeof(CAP_CONTEXT));

    if (INPUT_DEVICE == MMP_CAP_UNKNOW_DEVICE)
    {
        result = MMP_RESULT_ERROR;
        cap_msg_ex(CAP_MSG_TYPE_ERR, "No Match Device Type !\n");
        goto end;
    }

    Cap_Initialize();

#if !defined (EXTERNAL_HDMIRX)
    if (mmpCapGetCaptureDevice() == MMP_CAP_DEV_HDMIRX)
    {
        HOST_ReadRegister(GEN_SETTING_REG_34, &Value);

        //Enable video source from internal HDMI RX
        Value |= ((0x1 & 0x1) << 0);
        HOST_WriteRegister(GEN_SETTING_REG_34, Value);
    }
    else
#endif
    {
        HOST_ReadRegister(GEN_SETTING_REG_34, &Value);
        //Enable video source from IO pad
        Value |= ((0x0 & 0x1) << 0);
        HOST_WriteRegister(GEN_SETTING_REG_34, Value);
    }

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s error %d", __FUNCTION__, __LINE__);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Cap terminate.
 */
//=============================================================================
MMP_RESULT
mmpCapTerminate(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

    if (Capctxt == MMP_NULL)
    {
        return (MMP_RESULT)result;
    }

    //
    // Disable Cap engine
    //
    result = Cap_WaitEngineIdle();
    if (result)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, " err 0x%x !\n", result);
        goto end;
    }

    Cap_Engine_Reset();

    PalMemset((void *)Capctxt, 0, sizeof(CAP_CONTEXT));

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, " %s() err 0x%x !\n", __FUNCTION__, result);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * mmpCapFunEnable
 */
//=============================================================================
MMP_RESULT
mmpCapFunEnable(
    MMP_CAP_FUN_FLAG capfun)
{
    MMP_RESULT result = MMP_SUCCESS;

    if (Capctxt == MMP_NULL)
    {
        result = MMP_RESULT_ERROR;
        cap_msg_ex(CAP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    switch (capfun)
    {
    case MMP_CAP_INTERRUPT:
        Capctxt->EnableInterrupt = MMP_TRUE;
        break;

    case MMP_CAP_ONFLY_MODE:
        Capctxt->EnableOnflyMode = MMP_TRUE;
        break;

    default:
        cap_msg_ex(CAP_MSG_TYPE_ERR, " No Match Enable Type !\n");
        result = MMP_RESULT_ERROR;
        break;
    }

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s error (%d)", __FUNCTION__, __LINE__);

    return result;
}

//=============================================================================
/**
 * mmpCapFunDisable
 */
//=============================================================================
MMP_RESULT
mmpCapFunDisable(
    MMP_CAP_FUN_FLAG capfun)
{
    MMP_RESULT result = MMP_SUCCESS;

    if (Capctxt == MMP_NULL)
    {
        result = MMP_RESULT_ERROR;
        cap_msg_ex(CAP_MSG_TYPE_ERR, " NULL pointer !\n");
        goto end;
    }

    switch (capfun)
    {
    case MMP_CAP_INTERRUPT:
        Capctxt->EnableInterrupt = MMP_FALSE;
        break;

    case MMP_CAP_ONFLY_MODE:
        Capctxt->EnableOnflyMode = MMP_FALSE;
        break;

    default:
        cap_msg_ex(CAP_MSG_TYPE_ERR, " No Match Disable Type !\n");
        result = MMP_RESULT_ERROR;
        break;
    }

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s error (%d)", __FUNCTION__, __LINE__);

    return result;
}

//=============================================================================
/**
 * mmpCapWaitEngineIdle
 */
//=============================================================================
MMP_RESULT
mmpCapWaitEngineIdle(
    void)
{
    return Cap_WaitEngineIdle();
}

//=============================================================================
/**
 * mmpCapIsFire
 */
//=============================================================================
MMP_BOOL
mmpCapIsFire(
    void)
{
    return IsCapFire();
}

//=============================================================================
/**
 * mmpCapGetEngineErrorStatus
 */
//=============================================================================
MMP_UINT16
mmpCapGetEngineErrorStatus(
    MMP_CAP_LANE_STATUS lanenum)
{
    return Cap_Get_Lane_status(lanenum);
}

//=============================================================================
/**
 * mmpCapOnflyResetAllEngine
 */
//=============================================================================
MMP_RESULT
mmpCapOnflyResetAllEngine(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_UINT16 status;

    result = Cap_WaitEngineIdle();
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "!!!! capture not idle !!!!\n");

    result = mmpVPWaitEngineIdle();
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "!!!! ISP not idle !!!!\n");

    result = mmpVPWaitInterruptIdle();
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "!!!! ISP interrupt not idle !!!!\n");

    //if (result)
    {
        //HOST_ISP_Reset();
        mmpVPResetEngine();
        Cap_UnFire();
        Cap_Engine_Reset();
        Cap_Clean_Intr();
    }

    return result;
}

//=============================================================================
/**
 * mmpCapResetEngine
 */
//=============================================================================
MMP_RESULT
mmpCapResetEngine(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

    Cap_UnFire();
    Cap_Engine_Reset();
    Cap_Clean_Intr();

    return result;
}

//=============================================================================
/**
 * mmpCapParameterSetting
 */
//=============================================================================
MMP_RESULT
mmpCapParameterSetting(
    const MMP_CAP_SHARE *data)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_UINT8  i;

    if (Capctxt == MMP_NULL)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, "Capture not initialize\n");
        result = MMP_RESULT_ERROR;
        goto end;
    }

    Capctxt->ininfo.PitchY         = CAP_MEM_BUF_PITCH;
    Capctxt->ininfo.PitchUV        = CAP_MEM_BUF_PITCH;

    // Set Active Region
    Capctxt->ininfo.capwidth       = data->Width;
    Capctxt->ininfo.capheight      = data->Height;

    // Set ROI
    Capctxt->ininfo.ROIPosX        = ROIPosX;
    Capctxt->ininfo.ROIPosY        = ROIPosY;
    Capctxt->ininfo.ROIWidth       = ROIWidth;
    Capctxt->ininfo.ROIHeight      = ROIHeight;

    // Set Interlace or Prograssive
    Capctxt->ininfo.Interleave     = data->IsInterlaced;

    // Set Polarity
    Capctxt->ininfo.HSyncPol       = data->HSyncPol;
    Capctxt->ininfo.VSyncPol       = data->VSyncPol;

    // Set Output Parameter
    Capctxt->outinfo.OutWidth      = data->OutWidth;
    Capctxt->outinfo.OutHeight     = data->OutHeight;

    Capctxt->outinfo.OutAddrOffset = data->OutAddrOffset;

    //Update parameter
    result                         = Cap_Update_Reg();

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) ERROR !!!!\n", __FUNCTION__, __LINE__);
}

//=============================================================================
/**
 * mmpCapFire
 */
//=============================================================================
void
mmpCapFire(
    void)
{
    Cap_Fire();
}

//=============================================================================
/**
 * mmpCapMemoryInitialize
 */
//=============================================================================
MMP_RESULT
mmpCapMemoryInitialize(
    void)
{
    return Cap_Memory_Initialize();
}

//=============================================================================
/**
 * mmpCapMemoryClear
 */
//=============================================================================
MMP_RESULT
mmpCapMemoryClear(
    void)
{
    return Cap_Memory_Clear();
}

//=============================================================================
/**
 * mmpCapEnableInterrupt
 */
//=============================================================================
void
mmpCapRegisterIRQ(
    ITHIntrHandler caphandler)
{
    // Initialize Capture IRQ
    ithIntrDisableIrq(ITH_INTR_CAPTURE);
    ithIntrClearIrq(ITH_INTR_CAPTURE);

#if defined (__OPENRTOS__)
    // register NAND Handler to IRQ
    ithIntrRegisterHandlerIrq(ITH_INTR_CAPTURE, caphandler, MMP_NULL);
#endif     // defined (__OPENRTOS__)

    // set IRQ to edge trigger
    ithIntrSetTriggerModeIrq(ITH_INTR_CAPTURE, ITH_INTR_EDGE);

    // set IRQ to detect rising edge
    ithIntrSetTriggerLevelIrq(ITH_INTR_CAPTURE, ITH_INTR_HIGH_RISING);

    // Enable IRQ
    ithIntrEnableIrq(ITH_INTR_CAPTURE);
}

//=============================================================================
/**
 * mmpCapDisableIRQ
 */
//=============================================================================
void
mmpCapDisableIRQ(
    void)
{
    // Initialize Capture IRQ
    ithIntrDisableIrq(ITH_INTR_CAPTURE);
    ithIntrClearIrq(ITH_INTR_CAPTURE);
}

//=============================================================================
/**
 * mmpCapEnableInterrupt
 */
//=============================================================================
void
mmpCapEnableInterrupt(
    MMP_BOOL flag)
{
    Cap_Set_Enable_Interrupt(flag);
}

//=============================================================================
/**
 * mmpCapClearInterrupt
 */
//=============================================================================
MMP_UINT16
mmpCapClearInterrupt(
    void)
{
    MMP_UINT16 value, CapErrorType;

    value        = Cap_Clean_Intr();

    CapErrorType = value & 0x000F;

    return CapErrorType;
}

//=============================================================================
/**
 * Cap Write Buffer Index
 * @return index number
 */
//=============================================================================
MMP_UINT16
mmpCapReturnWrBufIndex(
    void)
{
    MMP_UINT16 CapWrBufIndex;
    MMP_UINT16 status = Cap_Get_Lane_status(CAP_LANE0_STATUS);

    CapWrBufIndex = ((status & 0x7000) >> 12);

    return CapWrBufIndex;
}

//=============================================================================
/**
 * mmpCapGetInputFrameRate
 */
//=============================================================================
MMP_CAP_FRAMERATE
mmpCapGetInputFrameRate(
    void)
{
    MMP_UINT16 RawVTotal;
    MMP_UINT32 framerate;
    MMP_UINT16 FrameRate_mode;
    MMP_UINT32 MemClk;
    MMP_FLOAT  MCLK_Freq;

    HOST_ReadRegister (0x1F2E, (MMP_UINT16 *)&RawVTotal);

    MemClk    = ithGetMemClock();
    MCLK_Freq = (MMP_FLOAT)(MemClk / 1000.0);

    framerate = (MMP_UINT32)((3906 * MCLK_Freq) / RawVTotal);
    //printf("RawVTotal = %x MCLK_Freq = %f framerate = %d\n", RawVTotal, MCLK_Freq, framerate);

    if ((23988 > framerate) && (framerate > 23946))       // 23.976fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_23_97HZ;
    }
    else if ((24030 > framerate) && (framerate > 23987))  // 24fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_24HZ;
    }
    else if ((25030 > framerate) && (framerate > 24970))  // 25fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_25HZ;
    }
    else if ((29985 > framerate) && (framerate > 29940))  // 29.97fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_29_97HZ;
    }
    else if ((30030 > framerate) && (framerate > 29984))  // 30fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_30HZ;
    }
    else if ((50030 > framerate) && (framerate > 49970))  // 50fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_50HZ;
    }
    else if ((57000 > framerate) && (framerate > 55000))  // 56fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_56HZ;
    }
    else if ((59970 > framerate) && (framerate > 57001))  // 59.94fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_59_94HZ;
    }
    else if ((62030 > framerate) && (framerate > 59969))  // 60fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_60HZ;
    }
    else if ((70999 > framerate) && (framerate > 69000))  // 70fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_70HZ;
    }
    else if ((73000 > framerate) && (framerate > 71000))  // 72fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_72HZ;
    }
    else if ((76000 > framerate) && (framerate > 74000))  // 75fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_75HZ;
    }
    else if ((86000 > framerate) && (framerate > 84000))  // 85fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_85HZ;
    }
    else
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_UNKNOW;
    }

    return FrameRate_mode;
}

//=============================================================================
/**
 * mmpCapGetOutputFrameRate
 */
//=============================================================================
MMP_CAP_FRAMERATE
mmpCapGetOutputFrameRate(
    MMP_UINT32 *FramePeriod)
{
    MMP_UINT16        RawVTotal;
    MMP_UINT32        framerate;
    MMP_UINT32        MemClk;
    MMP_FLOAT         MCLK_Freq;
    MMP_CAP_FRAMERATE FrameRate_mode;

    HOST_ReadRegister (0x1F2E, (MMP_UINT16 *)&RawVTotal);

    if (INPUT_DEVICE == MMP_CAP_DEV_SENSOR)
    {
#if defined (SENSOR_OMNIVISION_OV7725)
        FrameRate_mode = MMP_CAP_FRAMERATE_30HZ;
#elif defined (SENSOR_HIMAX_HM1375)
        FrameRate_mode = MMP_CAP_FRAMERATE_30HZ;
#elif defined (SENSOR_PIXELPLUS_PO3100)
        FrameRate_mode = MMP_CAP_FRAMERATE_30HZ;
#elif defined (SENSOR_NOVATEK_NT99141)
        FrameRate_mode = MMP_CAP_FRAMERATE_30HZ;
#elif defined (SENSOR_AR0130)
		FrameRate_mode = MMP_CAP_FRAMERATE_25HZ;
#endif
        return FrameRate_mode;
    }

    MemClk    = ithGetMemClock();
    MCLK_Freq = (MMP_FLOAT)(MemClk / 1000.0);

    framerate = ((3906 * MCLK_Freq) / RawVTotal);
    //printf("RawVTotal = %x MCLK_Freq = %f framerate = %d\n", RawVTotal, MCLK_Freq, framerate);

    if (INPUT_DEVICE == MMP_CAP_DEV_HDMIRX && mmpCapGetResolutionIndex(MMP_CAP_DEV_HDMIRX) == CAP_HDMI_INPUT_VESA)
    {
        if (Capctxt->skip_mode == CAPTURE_SKIP_BY_TWO)
            FrameRate_mode = MMP_CAP_FRAMERATE_VESA_30HZ;
        else
            FrameRate_mode = MMP_CAP_FRAMERATE_VESA_60HZ;
    }
    else if ((23988 > framerate) && (framerate > 23946))  // 23.976fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_23_97HZ;
    }
    else if ((24030 > framerate) && (framerate > 23987))  // 24fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_24HZ;
    }
    else if ((25030 > framerate) && (framerate > 24970))  // 25fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_25HZ;
    }
    else if ((29985 > framerate) && (framerate > 29940))  // 29.97fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_29_97HZ;
    }
    else if ((30030 > framerate) && (framerate > 29984))  // 30fps
    {
        FrameRate_mode = MMP_CAP_FRAMERATE_30HZ;
    }
    else if ((50030 > framerate) && (framerate > 49970))  // 50fps
    {
        if (Capctxt->ininfo.Interleave == MMP_TRUE || Capctxt->skip_mode == CAPTURE_SKIP_BY_TWO)
            FrameRate_mode = MMP_CAP_FRAMERATE_25HZ;
        else
            FrameRate_mode = MMP_CAP_FRAMERATE_50HZ;
    }
    else if ((59970 > framerate) && (framerate > 59910))  // 59.94fps
    {
        if (Capctxt->ininfo.Interleave == MMP_TRUE || Capctxt->skip_mode == CAPTURE_SKIP_BY_TWO)
            FrameRate_mode = MMP_CAP_FRAMERATE_29_97HZ;
        else
            FrameRate_mode = MMP_CAP_FRAMERATE_59_94HZ;
    }
    else if ((60030 > framerate) && (framerate > 59969))  // 60fps
    {
        if (Capctxt->ininfo.Interleave == MMP_TRUE || Capctxt->skip_mode == CAPTURE_SKIP_BY_TWO)
            FrameRate_mode = MMP_CAP_FRAMERATE_30HZ;
        else
            FrameRate_mode = MMP_CAP_FRAMERATE_60HZ;
    }
    else
    {
        if (Capctxt->ininfo.Interleave == MMP_TRUE || Capctxt->skip_mode == CAPTURE_SKIP_BY_TWO)
            FrameRate_mode = MMP_CAP_FRAMERATE_30HZ;
        else
            FrameRate_mode = MMP_CAP_FRAMERATE_60HZ;
    }

    if (Capctxt->ininfo.Interleave == MMP_TRUE || Capctxt->skip_mode == CAPTURE_SKIP_BY_TWO)
        *FramePeriod = (framerate >> 1);
    else
        *FramePeriod = framerate;

    return FrameRate_mode;
}

//=============================================================================
/**
 * mmpCapGetInputSrcInfo
 */
//=============================================================================
MMP_CAP_INPUT_INFO
mmpCapGetInputSrcInfo(
    void)
{
    MMP_CAP_INPUT_INFO info;
    MMP_CAP_FRAMERATE  frameRateMode;

    if (DevInfo.bMatchResolution == MMP_FALSE)
        return MMP_CAP_INPUT_INFO_UNKNOWN;

    frameRateMode = mmpCapGetInputFrameRate();

    if (INPUT_DEVICE == MMP_CAP_DEV_HDMIRX && mmpCapGetResolutionIndex(MMP_CAP_DEV_HDMIRX) == CAP_HDMI_INPUT_VESA)
    {
        mmpCapVESATimingCheck(DevInfo.Width, DevInfo.Height, &info);
    }
    else
    {
        switch (frameRateMode)
        {
        case MMP_CAP_FRAMERATE_23_97HZ:
            if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
                info = MMP_CAP_INPUT_INFO_1920X1080_23P;
            else
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            break;

        case MMP_CAP_FRAMERATE_24HZ:
            if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
                info = MMP_CAP_INPUT_INFO_1920X1080_24P;
            else
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            break;

        case MMP_CAP_FRAMERATE_25HZ:
            if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
                info = MMP_CAP_INPUT_INFO_1920X1080_25P;
            else
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            break;

        case MMP_CAP_FRAMERATE_29_97HZ:
            if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
                info = MMP_CAP_INPUT_INFO_1920X1080_29P;
            else
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            break;

        case MMP_CAP_FRAMERATE_30HZ:
            if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
                info = MMP_CAP_INPUT_INFO_1920X1080_30P;
            else
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            break;

        case MMP_CAP_FRAMERATE_50HZ:
            if (DevInfo.Width == 720 && DevInfo.Height == 576)
            {
                if (DevInfo.IsInterlaced)
                    info = MMP_CAP_INPUT_INFO_720X576_50I;
                else
                    info = MMP_CAP_INPUT_INFO_720X576_50P;
            }
            else if (DevInfo.Width == 1280 && DevInfo.Height == 720)
            {
                info = MMP_CAP_INPUT_INFO_1280X720_50P;
            }
            else if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
            {
                if (DevInfo.IsInterlaced)
                    info = MMP_CAP_INPUT_INFO_1920X1080_50I;
                else
                    info = MMP_CAP_INPUT_INFO_1920X1080_50P;
            }
            else
            {
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            }
            break;

        case MMP_CAP_FRAMERATE_59_94HZ:
            if (DevInfo.Width == 720 && DevInfo.Height == 480)
            {
                if (DevInfo.IsInterlaced)
                    info = MMP_CAP_INPUT_INFO_720X480_59I;
                else
                    info = MMP_CAP_INPUT_INFO_720X480_59P;
            }
            else if (DevInfo.Width == 1280 && DevInfo.Height == 720)
            {
                info = MMP_CAP_INPUT_INFO_1280X720_59P;
            }
            else if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
            {
                if (DevInfo.IsInterlaced)
                    info = MMP_CAP_INPUT_INFO_1920X1080_59I;
                else
                    info = MMP_CAP_INPUT_INFO_1920X1080_59P;
            }
            else if (DevInfo.Width == 640 && DevInfo.Height == 480)
            {
                info = MMP_CAP_INPUT_INFO_640X480_60P;
            }
            else
            {
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            }
            break;

        case MMP_CAP_FRAMERATE_60HZ:
            if (DevInfo.Width == 720 && DevInfo.Height == 480)
            {
                if (DevInfo.IsInterlaced)
                    info = MMP_CAP_INPUT_INFO_720X480_60I;
                else
                    info = MMP_CAP_INPUT_INFO_720X480_60P;
            }
            else if (DevInfo.Width == 1280 && DevInfo.Height == 720)
            {
                info = MMP_CAP_INPUT_INFO_1280X720_60P;
            }
            else if (DevInfo.Width == 1920 && DevInfo.Height == 1080)
            {
                if (DevInfo.IsInterlaced)
                    info = MMP_CAP_INPUT_INFO_1920X1080_60I;
                else
                    info = MMP_CAP_INPUT_INFO_1920X1080_60P;
            }
            else if (DevInfo.Width == 640 && DevInfo.Height == 480)
            {
                info = MMP_CAP_INPUT_INFO_640X480_60P;
            }
            else
            {
                info = MMP_CAP_INPUT_INFO_UNKNOWN;
            }
            break;

        default:
            info = MMP_CAP_INPUT_INFO_UNKNOWN;
            break;
        }
    }

    return info;
}

//=============================================================================
/**
 * mmpCapSetPolarity
 */
//=============================================================================
void
mmpCapSetPolarity(
    MMP_UINT16 HPolarity,
    MMP_UINT16 VPolarity)
{
    /* Set Hsync & Vsync Porlarity */
    HOST_WriteRegisterMask(GEN_SETTING_REG_1, (HPolarity << 7), 0x0080);
    HOST_WriteRegisterMask(GEN_SETTING_REG_1, (VPolarity << 8), 0x0100);
}

//=============================================================================
/**
 * mmpAVSyncCounterInit
 */
//=============================================================================
void
mmpAVSyncCounterCtrl(AV_SYNC_COUNTER_CTRL mode, MMP_UINT16 divider)
{
    AVSync_CounterCtrl(mode, divider);
}

//=============================================================================
/**
 * mmpAVSyncCounterReset
 */
//=============================================================================
void
mmpAVSyncCounterReset(AV_SYNC_COUNTER_CTRL mode)
{
    AVSync_CounterReset(mode);
}

//=============================================================================
/**
 * mmpAVSyncCounterLatch
 */
//=============================================================================
MMP_UINT32
mmpAVSyncCounterLatch(AV_SYNC_COUNTER_CTRL cntSel)
{
    AVSync_CounterLatch(cntSel);
}

//=============================================================================
/**
 * mmpAVSyncCounterRead
 */
//=============================================================================
MMP_UINT32
mmpAVSyncCounterRead(AV_SYNC_COUNTER_CTRL mode)
{
    return AVSync_CounterRead(mode);
}

//=============================================================================
/**
 * mmpAVSyncMuteDetect
 */
//=============================================================================
MMP_BOOL
mmpAVSyncMuteDetect(void)
{
    return AVSync_MuteDetect();
}

//=============================================================================
/**
 * mmpCapSetSkipPattern
 */
//=============================================================================
MMP_RESULT
mmpCapSetSkipMode(
    MMP_CAP_SKIP_MODE mode)
{
    MMP_RESULT result = MMP_SUCCESS;

    if (Capctxt == MMP_NULL)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, "Capture not initialize\n");
        result = MMP_RESULT_ERROR;
        goto end;
    }

    Cap_SetSkipMode((CAP_SKIP_MODE) mode);

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) ERROR !!!!\n", __FUNCTION__, __LINE__);
}

//=============================================================================
/**
 * mmpCapIsOnflyMode
 */
//=============================================================================
MMP_BOOL
mmpCapIsOnflyMode(
    void)
{
    if (Capctxt->EnableOnflyMode)
        return MMP_TRUE;
    else
        return MMP_FALSE;
}

//=============================================================================
/**
 * mmpCapTurnOnClock
 */
//=============================================================================
MMP_BOOL
mmpCapTurnOnClock(
    MMP_BOOL flag)
{
    Cap_TurnOnClock_Reg(flag);
}

//=============================================================================
/**
 * Set color control value.
 */
//=============================================================================
void
mmpCapSetColorCtrl(
    const MMP_CAP_COLOR_CTRL *data)
{
#if defined (CAP_USE_COLOR_EFFECT)
    if (data->brightness > 127)
        Capctxt->ColorCtrl.brightness = 127;
    else if (data->brightness < -128)
        Capctxt->ColorCtrl.brightness = -128;
    else
        Capctxt->ColorCtrl.brightness = data->brightness;

    if (data->contrast > 4.0)
        Capctxt->ColorCtrl.contrast = 4.0;
    else if (data->contrast < 0.0)
        Capctxt->ColorCtrl.contrast = 0.0;
    else
        Capctxt->ColorCtrl.contrast = data->contrast;

    if (data->hue > 359)
        Capctxt->ColorCtrl.hue = 359;
    else if (data->hue < 0)
        Capctxt->ColorCtrl.hue = 0;
    else
        Capctxt->ColorCtrl.hue = data->hue;

    if (data->saturation > 4.0)
        Capctxt->ColorCtrl.saturation = 4.0;
    else if (data->saturation < 0.0)
        Capctxt->ColorCtrl.saturation = 0.0;
    else
        Capctxt->ColorCtrl.saturation = data->saturation;

    Cap_SetColorCorrMatrix(
        &Capctxt->CCFun,
        Capctxt->ColorCtrl.brightness,
        Capctxt->ColorCtrl.contrast,
        Capctxt->ColorCtrl.hue,
        Capctxt->ColorCtrl.saturation,
        Capctxt->ColorCtrl.colorEffect);

    Capctxt->UpdateFlags |= CAP_FLAGS_UPDATE_CCMatrix;
#endif
}

//=============================================================================
/**
 * Get color control value.
 */
//=============================================================================
void
mmpCapGetColorCtrl(
    MMP_CAP_COLOR_CTRL *data)
{
#if defined (CAP_USE_COLOR_EFFECT)
    data->brightness = Capctxt->ColorCtrl.brightness;
    data->contrast   = Capctxt->ColorCtrl.contrast;
    data->hue        = Capctxt->ColorCtrl.hue;
    data->saturation = Capctxt->ColorCtrl.saturation;
#endif
}

//=============================================================================
/**
 * Update Color Matrix.
 */
//=============================================================================
void
mmpCapUpdateColorMatrix(
    void)
{
#if defined (CAP_USE_COLOR_EFFECT)
    Cap_UpdateColorMatrix();
#endif
}

//=============================================================================
/**
 * Capture Power Up.
 */
//=============================================================================
void
mmpCapPowerUp(
    void)
{
	Cap_Reset();
    Cap_EnableClock();
}

//=============================================================================
/**
 * Capture Power Down.
 */
//=============================================================================
void
mmpCapPowerDown(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_UINT16 status;

    Cap_UnFire();

    result = Cap_WaitEngineIdle();
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "!!!! capture not idle !!!!\n");

    Cap_Clean_Intr();

    Cap_DisableClock();
}