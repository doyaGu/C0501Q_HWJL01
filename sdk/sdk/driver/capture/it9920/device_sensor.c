
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

	printf("mmpCapDeviceInitialize\n");
    //mmpSensorInitialize();

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

    //mmpCapDeviceAllDeviceTriState();

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

#if 0

typedef struct CAP_ADV7180_TIMINFO_TABLE_TAG
{
    MMP_UINT16 Index;
    MMP_UINT16 HActive;
    MMP_UINT16 VActive;
    MMP_UINT16 Rate;
    MMP_UINT16 FrameRate;
    MMP_UINT16 Skippattern;
    MMP_UINT16 SkipPeriod;
    MMP_UINT16 ROIPosX;
    MMP_UINT16 ROIPosY;
    MMP_UINT16 ROIWidth;
    MMP_UINT16 ROIHeight;    
    MMP_UINT16 HPolarity:1;
    MMP_UINT16 VPolarity:1;
    MMP_UINT16 HStar;
    MMP_UINT16 HEnd;
    MMP_UINT16 VStar1;
    MMP_UINT16 VEnd1;
    MMP_UINT16 VStar2;
    MMP_UINT16 VEnd2;       
        
}CAP_ADV7180_TIMINFO_TABLE;


//ADV7180 Table
static CAP_ADV7180_TIMINFO_TABLE ADV7180_TABLE[] = {
	//Index, HActive, VActive,  Rate,      FrameRate,           SkipPattern,  SkipPeriod,  ROIX, ROIY, ROIW, ROIH,   Hpor,   Vpor,  HStar,     HEnd,   VStar1,   VEnd1,  VStar2,   VEnd2, 
	{ 0, 720, 480, 2997, MMP_CAP_FRAMERATE_29_97HZ, 0x0000, 	0xF, 	  0	/*8*/,   0/* 8*/, 720/*704*/,  480/* 460*/,   0, 	 0,  238 + 32, 1677 + 32, 22 - 7, 261 - 7, 285 - 7, 524 - 7 },//480i60    
	{ 1, 720, 576, 2500, MMP_CAP_FRAMERATE_25HZ, 0x0000, 0xF, 8, 8, 704, 560, 0, 0, 264 + 18, 1703 + 18, 23 - 4, 310 - 4, 336 - 4, 623 - 4 },//576i50    
};
#endif

//=============================================================================
/**
 * mmpCapGetDeviceInfo
 */
//=============================================================================
void
mmpCapGetDeviceInfo(
    MMP_CAP_SHARE *data)
{

	//just for ADV 7180 tmp.
    data->IsInterlaced = MMP_TRUE;

    data->Width = (MMP_UINT16)ADV7180_TABLE[0].HActive;
    data->Height = (MMP_UINT16)ADV7180_TABLE[0].VActive;
    data->HSyncPol = ADV7180_TABLE[0].HPolarity;
    data->VSyncPol = ADV7180_TABLE[0].VPolarity;
    data->FrameRate = ADV7180_TABLE[0].FrameRate;
    Capctxt->Skippattern = ADV7180_TABLE[0].Skippattern;

    Capctxt->funen.EnCSFun = MMP_TRUE; // /Benson

    Capctxt->ininfo.HNum1 = 0;
    Capctxt->ininfo.HNum2 = 0;
    Capctxt->ininfo.LineNum1 = 0;
    Capctxt->ininfo.LineNum2 = 0;
    Capctxt->ininfo.LineNum3 = 0;
    Capctxt->ininfo.LineNum4 = 0;

    /* Set ROI */
    ROIPosX   = ADV7180_TABLE[0].ROIPosX;
    ROIPosY   = ADV7180_TABLE[0].ROIPosY;
    ROIWidth  = ADV7180_TABLE[0].ROIWidth;
    ROIHeight = ADV7180_TABLE[0].ROIHeight;

    data->OutWidth = ROIWidth;
    data->OutHeight = ROIHeight;

    data->OutAddrY[0]  = CapMemBuffer[CAP_MEM_Y0];
    data->OutAddrUV[0] = CapMemBuffer[CAP_MEM_UV0];
    data->OutAddrY[1]  = CapMemBuffer[CAP_MEM_Y1];
    data->OutAddrUV[1] = CapMemBuffer[CAP_MEM_UV1];
    data->OutAddrY[2]  = CapMemBuffer[CAP_MEM_Y2];
    data->OutAddrUV[2] = CapMemBuffer[CAP_MEM_UV2];

	printf("data->OutAddrY[0]=0x%x\n",data->OutAddrY[0]);

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

