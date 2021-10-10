
#include "sensor/mmp_sensor.h"
#include "capture/sensor_device_table.h"

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
static MMP_BOOL gtDeviceReboot = MMP_TRUE;

extern MMP_UINT32 CapMemBuffer[CAPTURE_MEM_BUF_COUNT];

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
    return MMP_CAP_DEV_SENSOR;
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

#if defined (IT9919_144TQFP)
    #if defined (REF_BOARD_CAMERA)
        Cap_UnFire();
        return;
    #endif
#endif

    //Tri-State All Device
    //mmpHDMIRXOutputPinTriState(MMP_TRUE);
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

    mmpSensorInitialize();

    mmpCapDeviceAllDeviceTriState();

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
    MMP_RESULT result = MMP_SUCCESS;
    MMP_UINT16 status;

    mmpCapDeviceAllDeviceTriState();

    Cap_UnFire();

    result = Cap_WaitEngineIdle();
    if (result)
        cap_msg_ex(CAP_MSG_TYPE_ERR, "!!!! capture not idle !!!!\n");

    Cap_UnFire();
    Cap_Engine_Reset();
    Cap_Clean_Intr();

    memset(&DevInfo, 0, sizeof(MMP_CAP_SHARE));
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
    MMP_UINT16 captureErrState = mmpCapGetEngineErrorStatus(MMP_CAP_LANE0_STATUS);

    if (captureErrState & 0x0F00)
        isSignalChange = MMP_FALSE;
    else
        isSignalChange = MMP_TRUE;


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

    data->IsInterlaced = MMP_FALSE;

    data->Width = (MMP_UINT16)SENSOR_TABLE.HActive;
    data->Height = (MMP_UINT16)SENSOR_TABLE.VActive;
    data->HSyncPol = SENSOR_TABLE.HPolarity;
    data->VSyncPol = SENSOR_TABLE.VPolarity;
    data->FrameRate = SENSOR_TABLE.FrameRate;
    Capctxt->Skippattern = SENSOR_TABLE.Skippattern;

    Capctxt->funen.EnCSFun = MMP_FALSE;

    Capctxt->ininfo.HNum1 = 0;
    Capctxt->ininfo.HNum2 = 0;
    Capctxt->ininfo.LineNum1 = 0;
    Capctxt->ininfo.LineNum2 = 0;
    Capctxt->ininfo.LineNum3 = 0;
    Capctxt->ininfo.LineNum4 = 0;

    /* Set ROI */
    ROIPosX   = SENSOR_TABLE.ROIPosX;
    ROIPosY   = SENSOR_TABLE.ROIPosY;
    ROIWidth  = SENSOR_TABLE.ROIWidth;
    ROIHeight = SENSOR_TABLE.ROIHeight;

    data->OutWidth = ROIWidth;
    data->OutHeight = ROIHeight;

    data->OutAddrY[0]  = CapMemBuffer[CAP_MEM_Y0];
    data->OutAddrUV[0] = CapMemBuffer[CAP_MEM_UV0];
    data->OutAddrY[1]  = CapMemBuffer[CAP_MEM_Y1];
    data->OutAddrUV[1] = CapMemBuffer[CAP_MEM_UV1];
    data->OutAddrY[2]  = CapMemBuffer[CAP_MEM_Y2];
    data->OutAddrUV[2] = CapMemBuffer[CAP_MEM_UV2];

    data->bMatchResolution = MMP_TRUE;
    memcpy(&DevInfo, data, sizeof(MMP_CAP_SHARE));
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
        cap_msg_ex(CAP_MSG_TYPE_ERR, "------Not Reboot Device-------\n");
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
    return 0;
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
    (*info) = MMP_CAP_INPUT_INFO_CAMERA;
    return MMP_TRUE;
}

