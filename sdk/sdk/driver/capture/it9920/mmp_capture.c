#include "capture_config.h"
#include "capture/capture_types.h"
#include "capture_reg.h"
#include "capture_hw.h"
#include "capture.h"
#include "capture/video_device_table.h"
#include "capture/mmp_capture_it9920.h"
#include "ite/ith.h"
#include "vp/mmp_vp.h"

//=============================================================================
//                Constant Definition
//=============================================================================
extern MMP_UINT32 CapMemBuffer[2][CAPTURE_MEM_BUF_COUNT];

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
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
#include "io_video.c" //Benson,tmp to add.

static void
cap_isr(
    void* arg);


static void
cap_isr(
    void* arg)
{
	MMP_UINT32          captureErrState0 = 0; 
    MMP_UINT32          KeepCapDevID = 0;
	CAP_DEVICE *ptDev    = (CAP_DEVICE *) arg;
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

	//ithPrintf("cap_isr\n");
	//ithPrintf("ptDev=0x%x,Cap_ID=%d\n",ptDev,Capctxt->Cap_ID);
	captureErrState0 = mmpCapGetEngineErrorStatus(ptDev,MMP_CAP_LANE0_STATUS);
	KeepCapDevID  = Capctxt->Cap_ID;
	
	if((captureErrState0 & B_CAP_GET_INTR))
	{
		//clear cap interrupt 0
		mmpCapClearInterrupt(ptDev);	
	}
	else
	{
		//clear cap interrupt 1
		Capctxt->Cap_ID = CAP_DEV_ID1;
		mmpCapClearInterrupt(ptDev);
	}
	Capctxt->Cap_ID = KeepCapDevID;
	return;
	
}

//=============================================================================
/**
 * Cap context initialization.
 */
//=============================================================================
MMP_RESULT
mmpCapInitialize(
    CAP_DEVICE **ptDev,CAPTURE_DEV_ID DEV_ID)
{
	CAP_CONTEXT *Capctxt = MMP_NULL;
    MMP_RESULT   result  = MMP_SUCCESS;
    MMP_UINT16   Value;

	if(DEV_ID < 2)
	{
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

		//if (mmpCapGetDeviceReboot()) //for keep loopthrough
    	Cap_Engine_Register_Reset();
		Cap_Engine_Reset();
	    memset((void *)Capctxt, 0, sizeof(CAP_CONTEXT));

        Capctxt->Cap_ID = DEV_ID;
	    Cap_Initialize(Capctxt);
	   
		//Video source from external IO ,  Benson.
		ithWriteRegMaskA(CapRegTable[Capctxt->Cap_ID][CAP_REG_01C],0x1 ,CAP_MSK_UCLKSRC);
		
	}
	else
		{result = MMP_RESULT_ERROR;}
	
	 (*ptDev) = (CAP_DEVICE)Capctxt;
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
    CAP_DEVICE *ptDev)
{
    MMP_RESULT result = MMP_SUCCESS;
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

    if (Capctxt == MMP_NULL)
    {
        return (MMP_RESULT)result;
    }

    //
    // Disable Cap engine
    //
    result = Cap_WaitEngineIdle(Capctxt->Cap_ID);
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
 * mmpCapIsFire
 */
//=============================================================================
MMP_BOOL
mmpCapIsFire(
    CAP_DEVICE *ptDev)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;
    return IsCapFire(Capctxt->Cap_ID);
}

//=============================================================================
/**
 * mmpCapGetEngineErrorStatus
 */
//=============================================================================
MMP_UINT32
mmpCapGetEngineErrorStatus(
    CAP_DEVICE *ptDev,MMP_CAP_LANE_STATUS lanenum)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;
    return Cap_Get_Lane_status(Capctxt->Cap_ID,lanenum);
}

//=============================================================================
/**
 * mmpCapGetEngineErrorStatus
 */
//=============================================================================
MMP_UINT32
mmpCapGetEngineErrorCodeStatus(
    CAP_DEVICE *ptDev)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;
    return Cap_Get_Error_Status(Capctxt->Cap_ID);
}

//=============================================================================
/**
 * mmpCapGetAllEngineStatus
 */
//=============================================================================
void
mmpCapGetAllEngineStatus(
    CAP_DEVICE *ptDev)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;
    Cap_Get_Detected_Region(Capctxt->Cap_ID);
}

//=============================================================================
/**
 * mmpCapOnflyResetAllEngine
 */
//=============================================================================
MMP_RESULT
mmpCapOnflyResetAllEngine(
    CAP_DEVICE *ptDev)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_UINT16 status;

	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

    result = Cap_WaitEngineIdle(Capctxt->Cap_ID);
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "!!!! capture not idle !!!!\n");

	#if 0 //Benson VP.
    result = mmpVPWaitEngineIdle();
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "!!!! ISP not idle !!!!\n");

    result = mmpVPWaitInterruptIdle();
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "!!!! ISP interrupt not idle !!!!\n");
	#endif

    //if (result)
    {
        //HOST_ISP_Reset();
        	//mmpVPResetEngine();
        Cap_UnFire(Capctxt->Cap_ID);
        Cap_Engine_Reset();
        Cap_Clean_Intr(Capctxt->Cap_ID);
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
    CAP_DEVICE *ptDev)
{
    MMP_RESULT result = MMP_SUCCESS;
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

    Cap_UnFire(Capctxt->Cap_ID);
    Cap_Engine_Reset();
    Cap_Clean_Intr(Capctxt->Cap_ID);

    return result;
}

//=============================================================================
/**
 * mmpCapParameterSetting
 */
//=============================================================================
MMP_RESULT
mmpCapParameterSetting(
    CAP_DEVICE *ptDev)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_UINT8  i;

	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

    if (Capctxt == MMP_NULL)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, "Capture not initialize\n");
        result = MMP_RESULT_ERROR;
        goto end;
    }

    //Update parameter
    result                         = Cap_Update_Reg(Capctxt);

end:
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s (%d) ERROR !!!!\n", __FUNCTION__, __LINE__);
}

//=============================================================================
/**
 * mmpCapUnFire
 */
//=============================================================================
void
mmpCapUnFire(
	CAP_DEVICE *ptDev)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;
	Cap_UnFire(Capctxt->Cap_ID);
}

//=============================================================================
/**
 * mmpCapFire
 */
//=============================================================================
void
mmpCapFire(
    CAP_DEVICE *ptDev)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;
    Cap_Fire(Capctxt->Cap_ID);
}

//=============================================================================
/**
 * mmpCapMemoryInitialize
 */
//=============================================================================
MMP_RESULT
mmpCapMemoryInitialize(
    CAP_DEVICE *ptDev)
{
    return Cap_Memory_Initialize(ptDev);
}

//=============================================================================
/**
 * mmpCapMemoryClear
 */
//=============================================================================
MMP_RESULT
mmpCapMemoryClear(
    CAP_DEVICE *ptDev)
{
    return Cap_Memory_Clear(ptDev);
}

//=============================================================================
/**
 * mmpCapEnableInterrupt
 */
//=============================================================================
void
mmpCapRegisterIRQ(
    ITHIntrHandler caphandler,CAP_DEVICE *ptDev)
{
    // Initialize Capture IRQ
    ithIntrDisableIrq(ITH_INTR_CAPTURE);
    ithIntrClearIrq(ITH_INTR_CAPTURE);
#if defined (__OPENRTOS__)
    // register NAND Handler to IRQ
    ithIntrRegisterHandlerIrq(ITH_INTR_CAPTURE, caphandler, (void *)ptDev);
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
    CAP_DEVICE *ptDev,MMP_BOOL flag)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

    Cap_Set_Enable_Interrupt(Capctxt->Cap_ID,flag);
}

//=============================================================================
/**
 * mmpCapClearInterrupt
 */
//=============================================================================
MMP_UINT16
mmpCapClearInterrupt(
    CAP_DEVICE *ptDev)
{
    MMP_UINT32 value, CapErrorType;
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

    Cap_Clean_Intr(Capctxt->Cap_ID);
	return 0;
}

//=============================================================================
/**
 * Cap Write Buffer Index
 * @return index number
 */
//=============================================================================
MMP_UINT16
mmpCapReturnWrBufIndex(
    CAP_DEVICE *ptDev)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

    MMP_UINT16 CapWrBufIndex;
    MMP_UINT16 status = Cap_Get_Lane_status(Capctxt->Cap_ID,CAP_LANE0_STATUS);

    CapWrBufIndex = ((status & 0x70) >> 4);

    return CapWrBufIndex;
}

//=============================================================================
/**
 * mmpCapGetInputFrameRate
 */
//=============================================================================
MMP_CAP_FRAMERATE
mmpCapGetInputFrameRate(
    CAP_DEVICE *ptDev)
{
    MMP_UINT16 RawVTotal;
    MMP_UINT32 framerate;
    MMP_UINT16 FrameRate_mode;
    MMP_UINT32 MemClk;
    MMP_FLOAT  MCLK_Freq;

	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

    RawVTotal = Cap_Get_MRawVTotal(Capctxt->Cap_ID);

    //MemClk    = ithGetMemClock();
    //MCLK_Freq = (MMP_FLOAT)(MemClk / 1000.0);
    MCLK_Freq = 27500;

    framerate = (MMP_UINT32)((3906 * MCLK_Freq) / RawVTotal);
    printf("RawVTotal = %x MCLK_Freq = %f framerate = %d\n", RawVTotal, MCLK_Freq, framerate);

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
   CAP_DEVICE *ptDev, MMP_UINT32 *FramePeriod)
{
    MMP_UINT16        RawVTotal;
    MMP_UINT32        framerate;
    MMP_UINT32        MemClk;
    MMP_FLOAT         MCLK_Freq;
    MMP_CAP_FRAMERATE FrameRate_mode;
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

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
    CAP_DEVICE *ptDev)
{
    MMP_CAP_INPUT_INFO info;
    MMP_CAP_FRAMERATE  frameRateMode;
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

    if (DevInfo.bMatchResolution == MMP_FALSE)
        return MMP_CAP_INPUT_INFO_UNKNOWN;

    frameRateMode = mmpCapGetInputFrameRate(Capctxt->Cap_ID);

    if (INPUT_DEVICE == MMP_CAP_DEV_HDMIRX && mmpCapGetResolutionIndex(MMP_CAP_DEV_HDMIRX) == CAP_HDMI_INPUT_VESA)
    {
        mmpCapVESATimingCheck(Capctxt->Cap_ID,DevInfo.Width, DevInfo.Height, &info);
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
 * mmpAVSyncCounterInit
 */
//=============================================================================
void
mmpAVSyncCounterCtrl(AV_SYNC_COUNTER_CTRL mode, MMP_UINT16 divider)
{
    //AVSync_CounterCtrl(mode, divider);
}

//=============================================================================
/**
 * mmpAVSyncCounterReset
 */
//=============================================================================
void
mmpAVSyncCounterReset(AV_SYNC_COUNTER_CTRL mode)
{
    //AVSync_CounterReset(mode);
}

//=============================================================================
/**
 * mmpAVSyncCounterLatch
 */
//=============================================================================
MMP_UINT32
mmpAVSyncCounterLatch(CAP_DEVICE *ptDev,AV_SYNC_COUNTER_CTRL cntSel)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

    AVSync_CounterLatch(Capctxt->Cap_ID,cntSel);
}

//=============================================================================
/**
 * mmpAVSyncCounterRead
 */
//=============================================================================
MMP_UINT32
mmpAVSyncCounterRead(AV_SYNC_COUNTER_CTRL mode)
{
    return;// AVSync_CounterRead(mode);
}

//=============================================================================
/**
 * mmpAVSyncMuteDetect
 */
//=============================================================================
MMP_BOOL
mmpAVSyncMuteDetect(void)
{
    //return AVSync_MuteDetect();
}

//=============================================================================
/**
 * mmpCapSetSkipPattern
 */
//=============================================================================
MMP_RESULT
mmpCapSetSkipMode(
    CAP_DEVICE *ptDev,MMP_CAP_SKIP_MODE mode)
{
    MMP_RESULT result = MMP_SUCCESS;
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;


    if (Capctxt == MMP_NULL)
    {
        cap_msg_ex(CAP_MSG_TYPE_ERR, "Capture not initialize\n");
        result = MMP_RESULT_ERROR;
        goto end;
    }

    Cap_SetSkipMode(Capctxt,(CAP_SKIP_MODE) mode);

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
    CAP_DEVICE *ptDev)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

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
	CAP_DEVICE *ptDev,MMP_BOOL flag)
{
    //Cap_TurnOnClock_Reg(flag);
}

//=============================================================================
/**
 * Set color control value.
 */
//=============================================================================
void
mmpCapSetColorCtrl(
    CAP_DEVICE *ptDev,const MMP_CAP_COLOR_CTRL *data)
{
#if defined (CAP_USE_COLOR_EFFECT)
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

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
    CAP_DEVICE *ptDev,MMP_CAP_COLOR_CTRL *data)
{
#if defined (CAP_USE_COLOR_EFFECT)
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

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
    CAP_DEVICE *ptDev)
{
#if defined (CAP_USE_COLOR_EFFECT)
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;
    Cap_UpdateColorMatrix(Capctxt);
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
    CAP_DEVICE *ptDev)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_UINT16 status;

	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

    Cap_UnFire(Capctxt->Cap_ID);

    result = Cap_WaitEngineIdle(Capctxt->Cap_ID);
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "!!!! capture not idle !!!!\n");

    Cap_Clean_Intr(Capctxt->Cap_ID);

    Cap_DisableClock();
}

//=============================================================================
/**
 * mmpCapGetDeviceInfo
 */
//=============================================================================
void
mmpCapGetDeviceInfo(
    CAP_DEVICE *ptDev)
{
    MMP_UINT32 HTotal, VTotal, ColorDepth;
    MMP_UINT16 i, rate, Index;
    MMP_BOOL matchResolution = MMP_FALSE;
    MMP_CAP_INPUT_INFO infoIdx;

	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

	#if 0
		MMP_UINT16		 rate;
		MMP_BOOL		 matchResolution = MMP_FALSE;
		CAP_GET_PROPERTY CapGetProperty  = {0};

		CaptureModuleDriver_GetProperty(CaptureModuleDrivers, &CapGetProperty);

		rate						= CapGetProperty.Rate;
		matchResolution 			= CapGetProperty.matchResolution;

		// Set Active Region
		Capctxt->ininfo.capwidth	= CapGetProperty.GetWidth;	   // this two parameters
		Capctxt->ininfo.capheight	= CapGetProperty.GetHeight;    // can change to AutoDetected.

		// Set Polarity
		Capctxt->ininfo.HSyncPol	= CapGetProperty.HPolarity;
		Capctxt->ininfo.VSyncPol	= CapGetProperty.VPolarity;
		Capctxt->ininfo.TopFieldPol = CapGetProperty.GetTopFieldPolarity; // fingerprint will set it false.

		Capctxt->ininfo.PitchY		= CAP_MEM_BUF_PITCH;
		Capctxt->ininfo.PitchUV 	= CAP_MEM_BUF_PITCH >> 1;

		// Set Interlace or Prograssive
		Capctxt->Interleave 		= CapGetProperty.GetModuleIsInterlace;

		Capctxt->ininfo.HNum1		= CapGetProperty.HStar;
		Capctxt->ininfo.HNum2		= CapGetProperty.HEnd;
		Capctxt->ininfo.LineNum1	= CapGetProperty.VStar1;
		Capctxt->ininfo.LineNum2	= CapGetProperty.VEnd1;
		Capctxt->ininfo.LineNum3	= CapGetProperty.VStar2;
		Capctxt->ininfo.LineNum4	= CapGetProperty.VEnd2;

		if (matchResolution != MMP_TRUE)
			cap_msg_ex(CAP_MSG_TYPE_ERR, "No Support Resolution! %dx%d @%dHz\n", CapGetProperty.GetWidth, CapGetProperty.GetHeight, (rate / 100));
	#endif

		//CaptureModuleDriver_GetProperty(CaptureModuleDrivers, &CapGetProperty);
		
		printf("tmp run ADV7180_Setting \n");
		_ADV7180_Setting(Capctxt); //should be remove it.
		
	    rate = ADV7180_TABLE[0].Rate;
        //data->FrameRate = ADV7180_TABLE[i].FrameRate;
        Capctxt->Skippattern = ADV7180_TABLE[0].Skippattern;
        Capctxt->SkipPeriod = ADV7180_TABLE[0].SkipPeriod;
        matchResolution = MMP_TRUE;
	
		// Set Active Region
		Capctxt->ininfo.capwidth  = (MMP_UINT16)ADV7180_TABLE[0].HActive;
	    Capctxt->ininfo.capheight = (MMP_UINT16)ADV7180_TABLE[0].VActive;

		Capctxt->ininfo.PitchY	  = Capctxt->ininfo.capwidth; //CAP_MEM_BUF_PITCH; 
		Capctxt->ininfo.PitchUV   = Capctxt->ininfo.capwidth; //CAP_MEM_BUF_PITCH;
	
		// Set ROI
		Capctxt->ininfo.ROIPosX 	   = ADV7180_TABLE[0].ROIPosX;
		Capctxt->ininfo.ROIPosY 	   = ADV7180_TABLE[0].ROIPosY;
		Capctxt->ininfo.ROIWidth	   = ((ADV7180_TABLE[0].ROIWidth >> 2) << 2);
		Capctxt->ininfo.ROIHeight	   = ADV7180_TABLE[0].ROIHeight;
	
		// Set Interlace or Prograssive
		Capctxt->ininfo.Interleave  = MMP_TRUE;
	
		// Set Polarity
		Capctxt->ininfo.HSyncPol =  ADV7180_TABLE[0].HPolarity;
	    Capctxt->ininfo.VSyncPol =  ADV7180_TABLE[0].VPolarity;
	
		// Set Output Parameter
		Capctxt->outinfo.OutWidth  = Capctxt->ininfo.ROIWidth; //ROIWidth;
        Capctxt->outinfo.OutHeight = Capctxt->ininfo.ROIHeight; //ROIHeight;
	
		Capctxt->outinfo.OutAddrOffset = 0;
		Capctxt->outinfo.OutMemFormat  = SEMI_PLANAR_420;  //9920 new

		Capctxt->funen.EnCSFun = MMP_FALSE;//MMP_TRUE; //Benson
        Capctxt->ininfo.HNum1 = ADV7180_TABLE[0].HStar;
        Capctxt->ininfo.HNum2 = ADV7180_TABLE[0].HEnd;
        Capctxt->ininfo.LineNum1 = ADV7180_TABLE[0].VStar1;
        Capctxt->ininfo.LineNum2 = ADV7180_TABLE[0].VEnd1;
        Capctxt->ininfo.LineNum3 = ADV7180_TABLE[0].VStar2;
        Capctxt->ininfo.LineNum4 = ADV7180_TABLE[0].VEnd2;
		Capctxt->ininfo.ColorDepth = COLOR_DEPTH_8_BITS; //Benson add.

		Capctxt->OutAddrY[0]  = ithSysAddr2VramAddr(CapMemBuffer[Capctxt->Cap_ID][CAP_MEM_Y0]);
	    Capctxt->OutAddrUV[0] = ithSysAddr2VramAddr(CapMemBuffer[Capctxt->Cap_ID][CAP_MEM_UV0]);
	    Capctxt->OutAddrY[1]  = ithSysAddr2VramAddr(CapMemBuffer[Capctxt->Cap_ID][CAP_MEM_Y1]);
	    Capctxt->OutAddrUV[1] = ithSysAddr2VramAddr(CapMemBuffer[Capctxt->Cap_ID][CAP_MEM_UV1]);
	    Capctxt->OutAddrY[2]  = ithSysAddr2VramAddr(CapMemBuffer[Capctxt->Cap_ID][CAP_MEM_Y2]);
	    Capctxt->OutAddrUV[2] = ithSysAddr2VramAddr(CapMemBuffer[Capctxt->Cap_ID][CAP_MEM_UV2]);
	    Capctxt->bMatchResolution = matchResolution;
	
		if (matchResolution == MMP_FALSE)
			 printf("---Resolutin not Suppott !---\n");    
}

//=============================================================================
/**
 * mmpCapFunEnable
 */
//=============================================================================
int32_t
mmpCapFunEnable(
     CAP_DEVICE *ptDev,MMP_CAP_FUN_FLAG capfun)
{
    MMP_RESULT result = MMP_SUCCESS;
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

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
int32_t
mmpCapFunDisable(
    CAP_DEVICE *ptDev, MMP_CAP_FUN_FLAG capfun)
{
    MMP_RESULT result = MMP_SUCCESS;
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

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
 * ithCapWaitEngineIdle
 */
//=============================================================================
uint32_t
ithCapWaitEngineIdle(
    CAP_DEVICE *ptDev)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;
	Cap_UnFire(Capctxt->Cap_ID);
    return Cap_WaitEngineIdle(Capctxt->Cap_ID);
}

//=============================================================================
/**
 * ithCapGetDetectedWidth
 */
//=============================================================================
uint32_t 
ithCapGetDetectedWidth(
	CAP_DEVICE *ptDev)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;
	return Cap_Get_Detected_Width(Capctxt->Cap_ID);
}

//=============================================================================
/**
 * ithCapGetDetectedHeight
 */
//=============================================================================
uint32_t 
ithCapGetDetectedHeight(
	CAP_DEVICE *ptDev)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;
	return Cap_Get_Detected_Height(Capctxt->Cap_ID);
}

//=============================================================================
/**
 * ithCapGetDetectedInterleave
 */
//=============================================================================
uint32_t 
ithCapGetDetectedInterleave(
	CAP_DEVICE *ptDev)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;
	return Cap_Get_Detected_Interleave(Capctxt->Cap_ID);
}

//=============================================================================
/**
 * ithCaptureGetNewFrame
 */
//=============================================================================
int32_t 
ithCaptureGetNewFrame(
    CAP_DEVICE *ptDev ,ITE_CAP_VIDEO_INFO *Outdata)
{
    static int      cap_idx;
    static int      NewBufferIdx      = 0;
    static int      OldBufferIdx      = 0;
    static bool 	GetFirstBufferIdx = false;
    uint16_t      	timeOut = 0,FirstBufferIdx = 1;
	uint32_t 		OutWidth=0,OutHeight =0,ROI_Flag=0,IntrStatus=0,Hstable=0;
	int32_t			NotDisplay = 0;
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

	#ifndef CAP_ONFLY_ENABLE
    if (!GetFirstBufferIdx)
    {
        OldBufferIdx      = mmpCapReturnWrBufIndex(ptDev);
        GetFirstBufferIdx = true;
    }
	
    while (NewBufferIdx == OldBufferIdx)
    {
        NewBufferIdx = mmpCapReturnWrBufIndex(ptDev);
	    usleep(1000);
        timeOut++;
        if (timeOut > 100)
        {
	        printf("Capture controller not moving!!\n");
            break;
        }
    }
	
    OldBufferIdx          = NewBufferIdx;
	#endif

    Outdata->OutHeight    = Capctxt->ininfo.capheight;
    Outdata->OutWidth     = Capctxt->ininfo.capwidth;
    Outdata->IsInterlaced = Capctxt->ininfo.Interleave;
    Outdata->PitchY       = Capctxt->ininfo.PitchY;
    Outdata->PitchUV      = Capctxt->ininfo.PitchUV;
	Outdata->OutMemFormat = Capctxt->outinfo.OutMemFormat;
	
	//Detected Height and Width.
	if( (Outdata->OutWidth != ithCapGetDetectedWidth(ptDev)) || (Outdata->OutHeight !=ithCapGetDetectedHeight(ptDev)))
	{
		ithCapWaitEngineIdle(ptDev);
		Outdata->OutWidth  = Capctxt->ininfo.capwidth = ithCapGetDetectedWidth(ptDev);
		Outdata->OutHeight = Capctxt->ininfo.capheight = ithCapGetDetectedHeight(ptDev);
		printf("Outdata->OutWidth=%d,Outdata->OutHeight=%d\n",Outdata->OutWidth,Outdata->OutHeight);
		
		if(Outdata->OutWidth==1280 || Outdata->OutWidth==320)
		{
			ithWriteRegMaskA(CapRegTable[CAP_DEV_ID0][CAP_REG_01C], 0x4 <<4 , 0xF <<4); //for FPGA
			ithWriteRegMaskA(CapRegTable[CAP_DEV_ID0][CAP_REG_050], 0x0 <<28 , 0x1 <<28); //for FPGA , disable CheckHS.
			Outdata->OutWidth = Capctxt->ininfo.capwidth = 320;
			usleep(1000);
		}else if(Outdata->OutWidth==1920 || Outdata->OutWidth==480)
		{
			ithWriteRegMaskA(CapRegTable[CAP_DEV_ID0][CAP_REG_01C], 0x4 <<4 , 0xF <<4); //for FPGA
			ithWriteRegMaskA(CapRegTable[CAP_DEV_ID0][CAP_REG_050], 0x0 <<28 , 0x1 <<28); //for FPGA , disable CheckHS.
			Outdata->OutWidth = Capctxt->ininfo.capwidth = 480;
			usleep(1000);
		}
		else
		{
			ithWriteRegMaskA(CapRegTable[CAP_DEV_ID0][CAP_REG_01C], 0x0     , 0xF <<4);
			ithWriteRegMaskA(CapRegTable[CAP_DEV_ID0][CAP_REG_050], 0x1 <<28 , 0x1 <<28); //for FPGA , disable CheckHS.
		}
		
		if(Outdata->OutWidth % 2)
		{
			OutWidth = Outdata->OutWidth -1;
			ROI_Flag = 1;
		}
		else
			OutWidth  = Outdata->OutWidth;

		if(Outdata->OutHeight % 2)
		{
			OutHeight = Outdata->OutHeight - 1;
			ROI_Flag = 1;
		}
		else
			OutHeight = Outdata->OutHeight;

		printf("OutWidth =%d,OutHeight=%d\n",OutWidth,OutHeight);
		ithCapSetWidthHeight(ptDev,Outdata->OutWidth,Outdata->OutHeight);
		if(ROI_Flag)
		{
			printf("ROI_Flag=%d\n",ROI_Flag);
			ithCapWaitEngineIdle(ptDev);
			ithCapSetROIWidthHeight(ptDev,OutWidth,OutHeight);
			ROI_Flag = 0;
		}
		
		printf("fire again!\n");
		usleep(1000);
		NotDisplay = true;
	}

	//Detected Interleave.
	Hstable = mmpCapGetEngineErrorStatus(ptDev,MMP_CAP_LANE0_STATUS);
	if((Outdata->IsInterlaced != ithCapGetDetectedInterleave(ptDev)) && (Hstable & 0x1) )
	{
		ithCapWaitEngineIdle(ptDev);
		Outdata->IsInterlaced = Capctxt->ininfo.Interleave = ithCapGetDetectedInterleave(ptDev);
		printf("Get interleave=%d\n",Outdata->IsInterlaced);
		ithCapSetInterleave(ptDev,Outdata->IsInterlaced);

		printf("fire again!\n");
		usleep(1000);
		NotDisplay = true;
	}

    if (!OldBufferIdx)
        cap_idx = 2;
    else
        cap_idx = OldBufferIdx - 1;

    switch (cap_idx)
    {
    case 0:
        Outdata->DisplayAddrY = Capctxt->OutAddrY[0];
        Outdata->DisplayAddrU = Capctxt->OutAddrUV[0];
        Outdata->DisplayAddrV = Capctxt->OutAddrUV[0];
        break;

    case 1:
        Outdata->DisplayAddrY = Capctxt->OutAddrY[1];
        Outdata->DisplayAddrU = Capctxt->OutAddrUV[1];
        Outdata->DisplayAddrV = Capctxt->OutAddrUV[1];
        break;

    case 2:
        Outdata->DisplayAddrY = Capctxt->OutAddrY[2];
        Outdata->DisplayAddrU = Capctxt->OutAddrUV[2];
        Outdata->DisplayAddrV = Capctxt->OutAddrUV[2];
        break;
    }
	
    return NotDisplay;
}

//=============================================================================
/**
 * ithCapSetPolarity
 */
//=============================================================================
void
ithCapSetPolarity(
	CAP_DEVICE *ptDev,
    uint16_t HPolarity,
    uint16_t VPolarity)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;

	/* Set Hsync & Vsync Porlarity */
	Cap_Set_Hsync_Polarity(Capctxt->Cap_ID,HPolarity);
	Cap_Set_Vsync_Polarity(Capctxt->Cap_ID,VPolarity);
	Cap_Fire(Capctxt->Cap_ID);
}

//=============================================================================
/**
 * ithCapSetInterleave
 */
//=============================================================================
void 
ithCapSetInterleave(
	CAP_DEVICE *ptDev , uint32_t interleave)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;
	Cap_Set_Interleave(Capctxt->Cap_ID,interleave);
	Cap_Fire(Capctxt->Cap_ID);
}

//=============================================================================
/**
 * ithCapSetWidthHeight
 */
//=============================================================================
void 
ithCapSetWidthHeight(
	CAP_DEVICE *ptDev , uint32_t width, uint32_t height)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;
	Cap_Set_Width_Height(Capctxt->Cap_ID,width, height);
	Cap_Fire(Capctxt->Cap_ID);
}

//=============================================================================
/**
 * ithCapSetROIWidthHeight
 */
//=============================================================================
void 
ithCapSetROIWidthHeight(
	CAP_DEVICE *ptDev , uint32_t width, uint32_t height)
{
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;
	Cap_Set_ROI_Width_Height(Capctxt->Cap_ID,width, height);
	Cap_Fire(Capctxt->Cap_ID);
}

//=============================================================================
/**
 * ithCapFire
 */
//=============================================================================
int32_t 
ithCapFire(
	CAP_DEVICE *ptDev)
{
    MMP_RESULT result = MMP_SUCCESS;
	CAP_CONTEXT *Capctxt = (CAP_CONTEXT *)ptDev;
	static bool RegCapIntr = false;

    //Capture setting
    //get device information
	mmpCapGetDeviceInfo(ptDev);
    if (Capctxt->bMatchResolution == MMP_FALSE)
    {
        printf("MatchResolution fail!!\n");
        return MMP_RESULT_ERROR;
    }
	
	//mmpVPResetEngine();
	//mmpVPContextReset();
	
	//Register IRQ
	if(!RegCapIntr)
	{
		mmpCapRegisterIRQ(cap_isr,ptDev);
		RegCapIntr = true;
	}
	//mmpVPRegisterIRQ(VP_mem_isr);
	#ifdef CAP_ONFLY_ENABLE
	mmpCapFunDisable(ptDev,MMP_CAP_INTERRUPT);
	mmpCapFunEnable(ptDev,MMP_CAP_ONFLY_MODE);
	#else //Memory Mode
	mmpCapFunEnable(ptDev,MMP_CAP_INTERRUPT);
	mmpCapFunDisable(ptDev,MMP_CAP_ONFLY_MODE);
	#endif
	 
#if defined(SENSOR_OMNIVISION_OV7725)    
	mmpCapSetSkipMode(ptDev,MMP_CAPTURE_SKIP_BY_TWO);
#elif defined(SENSOR_AR0130)
	mmpCapSetSkipMode(ptDev,MMP_CAPTURE_NO_DROP);
#endif    

    //set capture parameter
	mmpCapParameterSetting(ptDev);

    //Capture Fire
    mmpCapFire(ptDev);

	usleep(1000);
	//Cap_Dump_Reg(CAP_DEV_ID0);

    return result;
}

//=============================================================================
/**
 * ithCapInitialize
 */
//=============================================================================

int32_t
ithCapInitialize(CAP_DEVICE **ptDev,CAPTURE_DEV_ID DEV_ID)
{
	uint32_t i;
	MMP_RESULT result = MMP_SUCCESS;
	
	 for (i = ITH_GPIO_PIN38; i <= ITH_GPIO_PIN66; i++)   //GPIO70 ~ GPIO81  for capture setting
	 {
		 ithGpioSetMode(i, ITH_GPIO_MODE2);
		 ithGpioSetIn(i);
	 } 

	 ithCapEnableClock();

	 //1st Capture Init, switch IO direction  //move this function to be the first entry func().
	 mmpCapInitialize(ptDev,DEV_ID);
	 
	 mmpCapMemoryInitialize(*ptDev);
	 
    return result;
}

