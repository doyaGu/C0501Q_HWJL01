/*
 * Copyright (c) 2010 ITE. All Rights Reserved.
 */
/** @file
 * Configurations.
 *
 * @author Odin He
 * @version 1.0
 */
#include "ite/mmp_types.h"
#include "hdmitx/typedef.h"
#include "hdmitx/hdmitx_drv.h"
#include "hdmitx/hdmitx_sys.h"   
#include "hdmitx/mmp_hdmitx.h"

//=============================================================================
//                              Public Function Definition
//=============================================================================

static BOOL gbDeviceID_66121 = FALSE; 

//=============================================================================
/**
 * HDMI TX initialization.
 */
//=============================================================================
void 
mmpHDMITXInitialize(
    MMP_HDMITX_INPUT_DEVICE inputDevice)
{	
    if (mmpHDMITXIs66121Chip())
        InitHDMITX_Instance_66121(inputDevice);
    else
        InitHDMITX_Instance_6613(inputDevice);  
}

//=============================================================================
/**
 * HDMI TX Loop Process.
 */
//=============================================================================
void 
mmpHDMITXDevLoopProc(
    MMP_BOOL bHDMIRxModeChange, 
    MMP_UINT32 AudioSampleRate, 
    MMP_UINT32 AudioChannelNum,
    MMP_HDMITX_INPUT_DEVICE inputDevice)
{
    if (mmpHDMITXIs66121Chip())
        HDMITX_DevLoopProc_66121(bHDMIRxModeChange, AudioSampleRate, AudioChannelNum, inputDevice);
    else
        HDMITX_DevLoopProc_6613(bHDMIRxModeChange, AudioSampleRate, AudioChannelNum, inputDevice);
}


//=============================================================================
/**
 * HDMI TX Loop Process.
 */
//=============================================================================

void
mmpHDMITXDisable(
     void)
{
    if (mmpHDMITXIs66121Chip())
        HDMITXDisable_66121();
    else
        HDMITXDisable_6613();
}     

//=============================================================================
/**
 * HDMI TX Loop Process.
 */
//=============================================================================

void
mmpHDMITXAVMute(
     MMP_BOOL isEnable)
{
    if (isEnable)
    {
        if (mmpHDMITXIs66121Chip())
            SetAVMute_66121(TRUE);
        else
            SetAVMute_6613(TRUE);
    }
    else
    {
        if (mmpHDMITXIs66121Chip())
            SetAVMute_66121(FALSE);
        else
            SetAVMute_6613(FALSE);
    }
}     

//=============================================================================
/**
 * HDMI TX Set Display Option.
 */
//=============================================================================
void 
mmpHDMITXSetDisplayOption(
    MMP_HDMITX_VIDEO_TYPE VideoMode,
    MMP_HDMITX_VESA_ID    VesaTiming,
    MMP_UINT16            EnableHDCP,
    MMP_BOOL              IsYUVInput)
{
    if (mmpHDMITXIs66121Chip())
        HDMITX_ChangeDisplayOption_66121(VideoMode, VesaTiming, EnableHDCP, IsYUVInput);
    else
        HDMITX_ChangeDisplayOption_6613(VideoMode, VesaTiming, EnableHDCP, IsYUVInput);
}

//=============================================================================
/**
 * HDMI TX Set DE Timing.
 */
//=============================================================================
void 
mmpHDMITXSetDETiming(
    MMP_UINT32  HDES,
    MMP_UINT32  HDEE,
    MMP_UINT32  VDES,
    MMP_UINT32  VDEE)
{
    if (mmpHDMITXIs66121Chip())
    {
        timingDE_66121.HDES = HDES;
        timingDE_66121.HDEE = HDEE;
        timingDE_66121.VDES = VDES;
        timingDE_66121.VDEE = VDEE;
    }
    else
    {
        timingDE_6613.HDES = HDES;
        timingDE_6613.HDEE = HDEE;
        timingDE_6613.VDES = VDES;
        timingDE_6613.VDEE = VDEE;
    }
}

//=============================================================================
/**
 * HDMI TX check IC.
 */
//=============================================================================
MMP_BOOL
mmpHDMITXIsChipEmpty(
    void)
{
    BYTE regDeviceID = 0x0;
    MMP_BOOL chipEmpty = 0;

    regDeviceID = HDMITX_ReadI2C_Byte_6613(REG_TX_DEVICE_ID0);

    printf("regDeviceID:0x%x\n",regDeviceID);
    if (regDeviceID == 0x12)
        gbDeviceID_66121 = TRUE; 
    else
        gbDeviceID_66121 = FALSE;

    if (gbDeviceID_66121)
        chipEmpty = HDMITX_IsChipEmpty_66121();
    else
        chipEmpty = HDMITX_IsChipEmpty_6613();

    return chipEmpty;
}

MMP_BOOL
mmpHDMITXIs66121Chip(
    void)
{	
    return gbDeviceID_66121;
}
