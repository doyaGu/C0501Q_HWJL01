#include "mmp_hdmirx.h"

#ifdef COMPONENT_DEV
    #include "mmp_cat9883.h"
#endif

#ifdef COMPOSITE_DEV
    #include "mmp_adv7180.h"
#endif

//=============================================================================
//                Constant Definition
//=============================================================================
extern MMP_UINT32 CapMemBuffer[CAPTURE_MEM_BUF_COUNT];

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================
static MMP_BOOL gtDeviceReboot = MMP_TRUE;
static MMP_UINT16 gtHDMIResolution = 0xFF;
static MMP_UINT16 gtCVBSResolution = 0xFF;
static MMP_UINT16 gtYPBPRResolution = 0xFF;

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
//=============================================================================
/**
 * Cap Select Device.
 */
//=============================================================================
MMP_RESULT
mmpCapSetCaptureDevice(
    MMP_CAP_DEVICE_ID id)
{
    INPUT_DEVICE = id;
}

//=============================================================================
/**
 * Cap Get Device.
 */
//=============================================================================
MMP_CAP_DEVICE_ID
mmpCapGetCaptureDevice(
    void)
{
    return INPUT_DEVICE;
}

//=============================================================================
/**
 * Cap Auto Detect Device.
 */
//=============================================================================
MMP_CAP_DEVICE_ID
mmpCapAutoDetectDevice(
    void)
{
    MMP_CAP_DEVICE_ID AutoDetectDev;

    switch (INPUT_DEVICE)
    {
////////////////////////////////////////////////////////////////////////////
#ifdef COMPOSITE_DEV
        case MMP_CAP_DEV_ADV7180:
            if (mmpADV7180IsSignalStable())
            {
                if (mmpHDMIRXIsSignalStable())
                {
                    AutoDetectDev = MMP_CAP_DEV_HDMIRX;
                    break;
                }
#ifdef COMPONENT_DEV
                if (mmpCAT9883IsSignalStable(MMP_FALSE))
                {
                    AutoDetectDev = MMP_CAP_DEV_CAT9883;
                    break;
                }
#endif
                AutoDetectDev = MMP_CAP_DEV_ADV7180;
            }
            else
                AutoDetectDev = MMP_CAP_UNKNOW_DEVICE;
            break;
#endif

////////////////////////////////////////////////////////////////////////////
#ifdef COMPONENT_DEV
        case MMP_CAP_DEV_CAT9883:
            if (mmpCAT9883IsSignalStable(MMP_TRUE))
            {
                if (mmpHDMIRXIsSignalStable())
                {
                    AutoDetectDev = MMP_CAP_DEV_HDMIRX;
                    break;
                }
#ifdef COMPOSITE_DEV
                if (mmpADV7180IsSignalStable())
                {
                    AutoDetectDev = MMP_CAP_DEV_ADV7180;
                    break;
                }
#endif
                AutoDetectDev = MMP_CAP_DEV_CAT9883;
            }
            else
                AutoDetectDev = MMP_CAP_UNKNOW_DEVICE;
            break;
#endif

////////////////////////////////////////////////////////////////////////////
        case MMP_CAP_DEV_HDMIRX:
            if (mmpHDMIRXIsSignalStable())
            {
#ifdef COMPONENT_DEV
                if (mmpCAT9883IsSignalStable(MMP_FALSE))
                {
                    AutoDetectDev = MMP_CAP_DEV_CAT9883;
                    break;
                }
#endif
#ifdef COMPOSITE_DEV
                if (mmpADV7180IsSignalStable())
                {
                    AutoDetectDev = MMP_CAP_DEV_ADV7180;
                    break;
                }
#endif
                AutoDetectDev = MMP_CAP_DEV_HDMIRX;
            }
            else
                AutoDetectDev = MMP_CAP_UNKNOW_DEVICE;
            break;

        default:
            cap_msg_ex(CAP_MSG_TYPE_ERR, "No Match Enable Type !\n");
            break;
    }

    if (AutoDetectDev == MMP_CAP_UNKNOW_DEVICE)
    {
        if (mmpHDMIRXIsSignalStable())
        {
            AutoDetectDev = MMP_CAP_DEV_HDMIRX;
        }

#ifdef COMPONENT_DEV
        else if (mmpCAT9883IsSignalStable(MMP_FALSE))
        {
            AutoDetectDev = MMP_CAP_DEV_CAT9883;
        }
#endif

#ifdef COMPOSITE_DEV
        else if (mmpADV7180IsSignalStable())
        {
            AutoDetectDev = MMP_CAP_DEV_ADV7180;
        }
#endif
    }

    return AutoDetectDev;
}

//=============================================================================
/**
 * Cap Device Tri-State.
 */
//=============================================================================
void
mmpCapDeviceAllDeviceTriState(
    void)
{
    //Tri-State All Device
    mmpHDMIRXOutputPinTriState(MMP_TRUE);

#ifdef COMPONENT_DEV
    mmpCAT9883OutputPinTriState(MMP_TRUE);
#endif

#ifdef COMPOSITE_DEV
    mmpADV7180OutputPinTriState(MMP_TRUE);
#endif

}

//=============================================================================
/**
 * Cap Device initialization.
 */
//=============================================================================
MMP_RESULT
mmpCapDeviceInitialize(
    void)
{
    MMP_RESULT result = MMP_SUCCESS;

    if (gtDeviceReboot)
    {
        gtHDMIResolution = 0xFF;
        gtCVBSResolution = 0xFF;
        gtYPBPRResolution = 0xFF;

        //Initialize All Device and Tri-State All Device
        //if (INPUT_DEVICE != MMP_CAP_DEV_HDMIRX)
        //    mmpHDMIRXPowerDown(MMP_TRUE);
        //else
            mmpHDMIRXInitialize();
        mmpHDMIRXOutputPinTriState(MMP_TRUE);

#ifdef COMPONENT_DEV
        //if (INPUT_DEVICE != MMP_CAP_DEV_CAT9883)
        //    mmpCAT9883PowerDown(MMP_TRUE);
        //else
            mmpCAT9883Initialize();
        mmpCAT9883OutputPinTriState(MMP_TRUE);
#endif

#ifdef COMPOSITE_DEV
        //if (INPUT_DEVICE != MMP_CAP_DEV_ADV7180)
        //    mmpADV7180PowerDown(MMP_TRUE);
        //else
            mmpADV7180Initialize();
        mmpADV7180OutputPinTriState(MMP_TRUE);
#endif

        switch (INPUT_DEVICE)
        {
#ifdef COMPOSITE_DEV
            case MMP_CAP_DEV_ADV7180:
                mmpADV7180OutputPinTriState(MMP_FALSE);
                break;
#endif

#ifdef COMPONENT_DEV
            case MMP_CAP_DEV_CAT9883:
                mmpCAT9883OutputPinTriState(MMP_FALSE);
                break;
#endif

            case MMP_CAP_DEV_HDMIRX:
                mmpHDMIRXOutputPinTriState(MMP_FALSE);
                break;

            default:
                cap_msg_ex(CAP_MSG_TYPE_ERR, "No Match Enable Type !\n");
                break;
        }
    }

    memset(&DevInfo, 0, sizeof(MMP_CAP_SHARE));

    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "%s error %d", __FUNCTION__, __LINE__);

    return (MMP_RESULT)result;
}

//=============================================================================
/**
 * Cap Device terminate.
 */
//=============================================================================
void
mmpCapDeviceTerminate(
    void)
{
    Cap_UnFire();

    mmpCapOnflyResetAllEngine();

    if (gtDeviceReboot)
    {
        mmpCapDeviceAllDeviceTriState();

        mmpHDMIRXTerminate();

#ifdef COMPONENT_DEV
        mmpCAT9883Terminate();
#endif

#ifdef COMPOSITE_DEV
        mmpADV7180Terminate();
#endif

        gtHDMIResolution = 0xFF;
        gtCVBSResolution = 0xFF;
        gtYPBPRResolution = 0xFF;

        memset(&DevInfo, 0, sizeof(MMP_CAP_SHARE));
    }
}

//=============================================================================
/**
 * Cap Device Signal State.
 */
//=============================================================================
MMP_BOOL
mmpCapDeviceIsSignalStable(
    void)
{
    MMP_BOOL isSignalChange = MMP_FALSE;

    switch (INPUT_DEVICE)
    {
#ifdef COMPOSITE_DEV
        case MMP_CAP_DEV_ADV7180:
            isSignalChange = mmpADV7180IsSignalStable();
            break;
#endif

#ifdef COMPONENT_DEV
        case MMP_CAP_DEV_CAT9883:
            isSignalChange = mmpCAT9883IsSignalStable(MMP_TRUE);
            break;
#endif

        case MMP_CAP_DEV_HDMIRX:
            isSignalChange = mmpHDMIRXIsSignalStable();
            break;

        default:
            cap_msg_ex(CAP_MSG_TYPE_ERR, " No Match Enable Type !\n");
            break;
    }

    return (MMP_BOOL)isSignalChange;
}

//=============================================================================
/**
 * Cap Get Device Signal State.
 */
//=============================================================================
MMP_BOOL
mmpCapGetDeviceIsSignalStable(
    MMP_CAP_DEVICE_ID id)
{
    MMP_BOOL isSignalChange = MMP_FALSE;

    switch (id)
    {
#ifdef COMPOSITE_DEV
        case MMP_CAP_DEV_ADV7180:
            isSignalChange = mmpADV7180IsSignalStable();
            break;
#endif

#ifdef COMPONENT_DEV
        case MMP_CAP_DEV_CAT9883:
            isSignalChange = mmpCAT9883IsSignalStable(MMP_FALSE);
            break;
#endif

        case MMP_CAP_DEV_HDMIRX:
            isSignalChange = mmpHDMIRXIsSignalStable();
            break;

        default:
            cap_msg_ex(CAP_MSG_TYPE_ERR, " No Match Enable Type !\n");
            break;
    }

    return (MMP_BOOL)isSignalChange;
}


//=============================================================================
/**
 * mmpCapGetDeviceInfo
 */
//=============================================================================
void
mmpCapGetDeviceInfo(
    MMP_CAP_SHARE *data)
{
    MMP_UINT32 HTotal, VTotal, ColorDepth;
    MMP_UINT16 i, rate, Index;
    MMP_BOOL matchResolution = MMP_FALSE;
    MMP_CAP_INPUT_INFO infoIdx;

    Capctxt->ininfo.ColorDepth = COLOR_DEPTH_8_BITS;

    if (INPUT_DEVICE == MMP_CAP_DEV_HDMIRX)
    {
        data->IsInterlaced = (MMP_BOOL)mmpHDMIRXGetProperty(HDMIRX_IS_INTERLACE);

        if (data->IsInterlaced)
            data->Height = (MMP_UINT16)mmpHDMIRXGetProperty(HDMIRX_HEIGHT) << 1;
        else
            data->Height = (MMP_UINT16)mmpHDMIRXGetProperty(HDMIRX_HEIGHT);

        data->Width = (MMP_UINT16)mmpHDMIRXGetProperty(HDMIRX_WIDTH);


        HTotal = mmpHDMIRXGetProperty(HDMIRX_HTOTAL);
        VTotal = mmpHDMIRXGetProperty(HDMIRX_VTOTAL);

        for (i = 0; i < (sizeof(HDMI_TABLE)/sizeof(CAP_HDMI_TIMINFO_TABLE)); ++i)
        {
            if ((HTotal == HDMI_TABLE[i].HTotal) &&
                (VTotal == HDMI_TABLE[i].VTotal || VTotal == HDMI_TABLE[i].VTotal+1) )
            {
                if (i == 6 || i == 7) //1080p60/1080p50 or 1080p60/1080p25
                {
                    MMP_UINT32 hdmirxPCLK = mmpHDMIRXGetProperty(HDMIRX_PCLK);
                    if (hdmirxPCLK < 100000000) //100MHz
                        i = i + 6; //1080p60/1080p25
                }

                data->HSyncPol = HDMI_TABLE[i].HPolarity;
                data->VSyncPol = HDMI_TABLE[i].VPolarity;
                Capctxt->Skippattern = HDMI_TABLE[i].Skippattern;
                Capctxt->SkipPeriod = HDMI_TABLE[i].SkipPeriod;
                data->FrameRate = HDMI_TABLE[i].FrameRate;
                matchResolution = MMP_TRUE;
                gtHDMIResolution = i;
                break;
            }
        }


#ifdef IT9913_128LQFP
        Capctxt->funen.EnCSFun = MMP_FALSE;
#else
        if (mmpHDMIRXGetProperty(HDMIRX_OUTPUT_VIDEO_MODE) == 0)
            Capctxt->funen.EnCSFun = MMP_TRUE;  //HDMI Rx RGB444 Output
        else
            Capctxt->funen.EnCSFun = MMP_FALSE; //HDMI Rx YUV444/YUV422 Output
#endif

        if (matchResolution == MMP_FALSE)
        {
            MMP_CAP_INPUT_INFO infoIdx;
            data->HSyncPol = 1;
            data->VSyncPol = 1;
            Capctxt->Skippattern = 0xAAAA;
            Capctxt->SkipPeriod = 0xF;
            data->FrameRate = MMP_CAP_FRAMERATE_60HZ;
            gtHDMIResolution = CAP_HDMI_INPUT_VESA;
            Capctxt->ininfo.HNum1 = 0;
            Capctxt->ininfo.HNum2 = 0;
            Capctxt->ininfo.LineNum1 = 0;
            Capctxt->ininfo.LineNum2 = 0;
            Capctxt->ininfo.LineNum3 = 0;
            Capctxt->ininfo.LineNum4 = 0;

            Capctxt->funen.EnDEMode = MMP_TRUE;
            matchResolution = mmpCapVESATimingCheck(data->Width, data->Height, &infoIdx);
        }
        else
        {
            Capctxt->ininfo.HNum1 = HDMI_TABLE[i].HStar;
            Capctxt->ininfo.HNum2 = HDMI_TABLE[i].HEnd;
            Capctxt->ininfo.LineNum1 = HDMI_TABLE[i].VStar1;
            Capctxt->ininfo.LineNum2 = HDMI_TABLE[i].VEnd1;
            Capctxt->ininfo.LineNum3 = HDMI_TABLE[i].VStar2;
            Capctxt->ininfo.LineNum4 = HDMI_TABLE[i].VEnd2;

            Capctxt->funen.EnDEMode = MMP_FALSE;
        }

        printf("Htotal = %d, Vtotal = %d, w = %d h = %d, res = %d, fps = %d\n", HTotal, VTotal, data->Width, data->Height, gtHDMIResolution, mmpCapGetInputFrameRate());

        /* Set ROI */
        ROIPosX   = 0;
        ROIPosY   = 0;
        ROIWidth  = data->Width;
        ROIHeight = data->Height;

        data->OutWidth = data->Width;
        data->OutHeight = data->Height;

        ColorDepth = mmpHDMIRXGetProperty(HDMIRX_COLOR_DEPTH);

        if (ColorDepth == 36)
            Capctxt->ininfo.ColorDepth = COLOR_DEPTH_12_BITS;
        else if (ColorDepth == 30)
            Capctxt->ininfo.ColorDepth = COLOR_DEPTH_10_BITS;
        else //(ColorDepth == 24)
            Capctxt->ininfo.ColorDepth = COLOR_DEPTH_8_BITS;
    }

#ifdef COMPONENT_DEV
    if (INPUT_DEVICE == MMP_CAP_DEV_CAT9883)
    {
        data->IsInterlaced = (MMP_BOOL)mmpCAT9883GetProperty(CAT9883_IS_INTERLACE);
        data->Height = (MMP_UINT16)mmpCAT9883GetProperty(CAT9883_HEIGHT);
        data->Width = (MMP_UINT16)mmpCAT9883GetProperty(CAT9883_WIDTH);
        rate = (MMP_UINT16)mmpCAT9883GetProperty(CAT9883_FRAMERATE);

        for (i = 0; i < (sizeof(CAT9883_TABLE)/sizeof(CAP_CAT9883_TIMINFO_TABLE)); ++i)
        {
            if ((data->Width == CAT9883_TABLE[i].HActive) &&
                (data->Height == CAT9883_TABLE[i].VActive) &&
                rate == CAT9883_TABLE[i].Rate)
            {
                data->HSyncPol = CAT9883_TABLE[i].HPolarity;
                data->VSyncPol = CAT9883_TABLE[i].VPolarity;
                data->FrameRate = CAT9883_TABLE[i].FrameRate;
                Capctxt->Skippattern = CAT9883_TABLE[i].Skippattern;
                Capctxt->SkipPeriod = CAT9883_TABLE[i].SkipPeriod;
                matchResolution = MMP_TRUE;
                gtYPBPRResolution = Index = i;
                break;
            }
        }

        if (i == (sizeof(CAT9883_TABLE)/sizeof(CAP_CAT9883_TIMINFO_TABLE)))
            cap_msg_ex(CAP_MSG_TYPE_ERR, "No Support Resolution! %dx%d @%dHz\n", data->Width, data->Height, (rate / 100));

        if (mmpCAT9883GetProperty(CAT9883_IS_TV_MODE))
            Capctxt->funen.EnCSFun = MMP_FALSE;
        else
            Capctxt->funen.EnCSFun = MMP_TRUE;

        Capctxt->ininfo.HNum1 = CAT9883_TABLE[i].HStar;
        Capctxt->ininfo.HNum2 = CAT9883_TABLE[i].HEnd;
        Capctxt->ininfo.LineNum1 = CAT9883_TABLE[i].VStar1;
        Capctxt->ininfo.LineNum2 = CAT9883_TABLE[i].VEnd1;
        Capctxt->ininfo.LineNum3 = CAT9883_TABLE[i].VStar2;
        Capctxt->ininfo.LineNum4 = CAT9883_TABLE[i].VEnd2;

        Capctxt->funen.EnCrossLineDE = CAT9883_TABLE[i].CrossLineDE;
        Capctxt->funen.EnYPbPrTopVSMode = CAT9883_TABLE[i].YPbPrTopVSMode;
        Capctxt->funen.EnDlyVS = CAT9883_TABLE[i].DlyVS;
        Capctxt->funen.EnHSPosEdge = CAT9883_TABLE[i].HSPosEdge;

        /* Set ROI */
        ROIPosX     = CAT9883_TABLE[i].ROIPosX;
        ROIPosY     = CAT9883_TABLE[i].ROIPosY;
        ROIWidth    = ((CAT9883_TABLE[i].ROIWidth >> 2) << 2);
        ROIHeight   = CAT9883_TABLE[i].ROIHeight;

        data->OutWidth = ROIWidth;
        data->OutHeight = ROIHeight;

        Capctxt->inmux_info.UCLKInv = CAT9883_TABLE[i].UCLKInv;
    }
#endif

#ifdef COMPOSITE_DEV
    if (INPUT_DEVICE == MMP_CAP_DEV_ADV7180)
    {
        data->IsInterlaced = (MMP_BOOL)mmpADV7180GetProperty(ADV7180_IS_INTERLACE);
        data->Height = (MMP_UINT16)mmpADV7180GetProperty(ADV7180_HEIGHT);
        data->Width = (MMP_UINT16)mmpADV7180GetProperty(ADV7180_WIDTH);
        rate = (MMP_UINT16)mmpADV7180GetProperty(ADV7180_FRAMERATE);

        for (i = 0; i < (sizeof(ADV7180_TABLE)/sizeof(CAP_ADV7180_TIMINFO_TABLE)); ++i)
        {
            if ((data->Width == ADV7180_TABLE[i].HActive) &&
                (data->Height == ADV7180_TABLE[i].VActive) &&
                rate == ADV7180_TABLE[i].Rate)
            {
                data->HSyncPol = ADV7180_TABLE[i].HPolarity;
                data->VSyncPol = ADV7180_TABLE[i].VPolarity;
                data->FrameRate = ADV7180_TABLE[i].FrameRate;
                Capctxt->Skippattern = ADV7180_TABLE[i].Skippattern;
                Capctxt->SkipPeriod = ADV7180_TABLE[i].SkipPeriod;
                matchResolution = MMP_TRUE;
                gtCVBSResolution = i;
                break;
            }
        }

        if (i == (sizeof(ADV7180_TABLE)/sizeof(CAP_ADV7180_TIMINFO_TABLE)))
            cap_msg_ex(CAP_MSG_TYPE_ERR, "No Support Resolution! %dx%d @%dHz\n", data->Width, data->Height, (rate / 100));

        Capctxt->funen.EnCSFun = MMP_FALSE;

        Capctxt->ininfo.HNum1 = ADV7180_TABLE[i].HStar;
        Capctxt->ininfo.HNum2 = ADV7180_TABLE[i].HEnd;
        Capctxt->ininfo.LineNum1 = ADV7180_TABLE[i].VStar1;
        Capctxt->ininfo.LineNum2 = ADV7180_TABLE[i].VEnd1;
        Capctxt->ininfo.LineNum3 = ADV7180_TABLE[i].VStar2;
        Capctxt->ininfo.LineNum4 = ADV7180_TABLE[i].VEnd2;

        /* Set ROI */
        ROIPosX     = ADV7180_TABLE[i].ROIPosX;
        ROIPosY     = ADV7180_TABLE[i].ROIPosY;
        ROIWidth    = ((ADV7180_TABLE[i].ROIWidth >> 2) << 2);
        ROIHeight   = ADV7180_TABLE[i].ROIHeight;

        data->OutWidth = ROIWidth;
        data->OutHeight = ROIHeight;
    }
#endif

    data->OutAddrY[0]  = CapMemBuffer[CAP_MEM_Y0];
    data->OutAddrUV[0] = CapMemBuffer[CAP_MEM_UV0];
    data->OutAddrY[1]  = CapMemBuffer[CAP_MEM_Y1];
    data->OutAddrUV[1] = CapMemBuffer[CAP_MEM_UV1];
    data->OutAddrY[2]  = CapMemBuffer[CAP_MEM_Y2];
    data->OutAddrUV[2] = CapMemBuffer[CAP_MEM_UV2];

    data->bMatchResolution = matchResolution;
    memcpy(&DevInfo, data, sizeof(MMP_CAP_SHARE));

    if (matchResolution == MMP_FALSE)
        printf("---Resolutin not Suppott !---\n");

}

//=============================================================================
/**
 * Device reboot.
 */
//=============================================================================
MMP_RESULT
mmpCapSetDeviceReboot(
    MMP_BOOL flag)
{
    gtDeviceReboot = flag;

    if (!gtDeviceReboot)
        printf("------Not Reboot Device-------\n");
}

MMP_BOOL
mmpCapGetDeviceReboot(
    void)
{
    return gtDeviceReboot;
}

//=============================================================================
/**
 * Capture Resolution.
 */
//=============================================================================
MMP_UINT16
mmpCapGetResolutionIndex(
    MMP_CAP_DEVICE_ID id)
{
    MMP_UINT16 resIndex;

    switch (id)
    {
        case MMP_CAP_DEV_HDMIRX:
            resIndex = gtHDMIResolution;
            break;

        case MMP_CAP_DEV_CAT9883:
            resIndex = gtYPBPRResolution;
            break;

        case MMP_CAP_DEV_ADV7180:
            resIndex = gtCVBSResolution;
            break;

        default:
            resIndex = 0;
            break;
    }

    return resIndex;
}

//=============================================================================
/**
 * Capture VESA Timing Check.
 */
//=============================================================================
MMP_BOOL
mmpCapVESATimingCheck(
    MMP_UINT16 width,
    MMP_UINT16 height,
    MMP_CAP_INPUT_INFO *info)
{
    MMP_CAP_INPUT_INFO infoIdx;
    MMP_CAP_FRAMERATE frameRateMode;

    frameRateMode = mmpCapGetInputFrameRate();

    switch (frameRateMode)
    {
        case MMP_CAP_FRAMERATE_59_94HZ:
        case MMP_CAP_FRAMERATE_60HZ:
            if (width == 800 && height == 600)
            {
            	infoIdx = MMP_CAP_INPUT_INFO_800X600_60P;
            }
            else if (width == 1024 && height == 768)
            {
            	infoIdx = MMP_CAP_INPUT_INFO_1024X768_60P;
            }
            else if (width == 1280 && height == 720)
            {
            	infoIdx = MMP_CAP_INPUT_INFO_1280X720_60P;
            }
            else if (width == 1280 && height == 768)
            {
            	infoIdx = MMP_CAP_INPUT_INFO_1280X768_60P;
            }
            else if (width == 1280 && height == 800)
            {
            	infoIdx = MMP_CAP_INPUT_INFO_1280X800_60P;
            }
            else if (width == 1280 && height == 960)
            {
            	infoIdx = MMP_CAP_INPUT_INFO_1280X960_60P;
            }
            else if (width == 1280 && height == 1024)
            {
            	infoIdx = MMP_CAP_INPUT_INFO_1280X1024_60P;
            }
            else if (width == 1360 && height == 768)
            {
            	infoIdx = MMP_CAP_INPUT_INFO_1360X768_60P;
            }
            else if (width == 1366 && height == 768)
            {
            	infoIdx = MMP_CAP_INPUT_INFO_1366X768_60P;
            }
            else if (width == 1440 && height == 900)
            {
            	infoIdx = MMP_CAP_INPUT_INFO_1440X900_60P;
            }
            else if (width == 1400 && height == 1050)
            {
            	infoIdx = MMP_CAP_INPUT_INFO_1400X1050_60P;
            }
            else if (width == 1440 && height == 1050)
            {
            	infoIdx = MMP_CAP_INPUT_INFO_1440X1050_60P;
            }
            else if (width == 1600 && height == 900)
            {
            	infoIdx = MMP_CAP_INPUT_INFO_1600X900_60P;
            }
            //else if (width == 1600 && height == 1200)
            //{
            //	infoIdx = MMP_CAP_INPUT_INFO_1600X1200_60P;
            //}
            else if (width == 1680 && height == 1050)
            {
            	infoIdx = MMP_CAP_INPUT_INFO_1680X1050_60P;
            }
            else if (width == 1920 && height == 1080)
            {
            	infoIdx = MMP_CAP_INPUT_INFO_1920X1080_60P;
            }
            else
            {
            	infoIdx = MMP_CAP_INPUT_INFO_UNKNOWN;
            }
            break;
        default:
            infoIdx = MMP_CAP_INPUT_INFO_UNKNOWN;
            break;
    }

    (*info) = infoIdx;

    if (infoIdx == MMP_CAP_INPUT_INFO_UNKNOWN)
        return MMP_FALSE;
    else
        return MMP_TRUE;
}



