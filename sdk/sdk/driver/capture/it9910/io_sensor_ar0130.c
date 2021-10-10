
#include "capture/sensor_device_table.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define CAP_DATA_BUS_WIDTH  12

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
/**
 * AR0130 Set Input Data Pin.
 */
//=============================================================================
static void
_SENSOR_Input_Data_Pin(
    void)
{
    //Reference Design 2012/09/12
    //YUV 16Bit Pin LSB--->MSB (D0--->D11)
    MMP_UINT8 Y_16Pin[CAP_DATA_BUS_WIDTH] = { 0, 01, 02, 03, 04, 05, 06, 07, 0, 0, 0, 0};
    MMP_UINT8 U_16Pin[CAP_DATA_BUS_WIDTH] = { 0, 01, 02, 03, 04, 05, 06, 07, 0, 0, 0, 0};
    MMP_UINT8 V_16Pin[CAP_DATA_BUS_WIDTH] = { 0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0};

    /* Input  pin mux setting */
    PalMemcpy(&Capctxt->inmux_info.Y_Pin_Num, &Y_16Pin, CAP_DATA_BUS_WIDTH);
    PalMemcpy(&Capctxt->inmux_info.U_Pin_Num, &U_16Pin, CAP_DATA_BUS_WIDTH);
    PalMemcpy(&Capctxt->inmux_info.V_Pin_Num, &V_16Pin, CAP_DATA_BUS_WIDTH);

}

//=============================================================================
/**
 * AR0130 Setting.
 */
//=============================================================================
static void
_SENSOR_Setting(
    void)
{
    MMP_UINT16 i;

    _SENSOR_Input_Data_Pin();

    Capctxt->iomode_info.EnInternalRxHDMI = 0x0; // 0: VDO Source from IO Pad.

    Capctxt->inmux_info.UCLKSrc = 0x0; //0: IO, 1: HDMI, 2: GPIO, 3: PLL3_n1, 4: PLL3_n2
    Capctxt->inmux_info.UCLKInv = 0x0;
    Capctxt->inmux_info.UCLKDly = 0x0;
    Capctxt->inmux_info.UCLKRatio = 0x0;
    Capctxt->inmux_info.EnUCLK = 0x1;
    Capctxt->inmux_info.UCLKVDSel = 36;

    /* Output pin mux setting */
    for (i = 0; i < 36; i++)
        Capctxt->outpin_info.CAPIOVDOUTSEL[i] = 0;
    Capctxt->outpin_info.CAPIOVDOUTSEL_X = 0;

    /* I/O Setting */
    Capctxt->iomode_info.CAPIOEN_VD_15_0 = 0xFFFF;  //1111_1111_1111_1111
    Capctxt->iomode_info.CAPIOEN_VD_31_16 = 0xFFFF; //1111_1111_1111_1111
    Capctxt->iomode_info.CAPIOEN_VD_32_35 = 0xF;    //0000_0000_0000_1111
    Capctxt->iomode_info.CAPIOEN_X = 0x1;
    Capctxt->iomode_info.CAPIOEN_HS = 0x1;
    Capctxt->iomode_info.CAPIOEN_VS = 0x1;
    Capctxt->iomode_info.CAPIOEN_DE = 0x1;
    Capctxt->iomode_info.CAPIOEN_Field = 0x1;
    Capctxt->iomode_info.CAPIOEN_PMCLK = 0x0;

    Capctxt->iomode_info.CAPIOFFEn_DE = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_Field = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_HS = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_VS = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_VD_00_15 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_16_31 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_32_47 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_48_54 = 0x7F;

    /* Input Format default Setting */
    Capctxt->ininfo.EmbeddedSync = BT_601;
    Capctxt->ininfo.Interleave = Progressive;

    /* Input Data Format Setting */
    // 8bit bus
    Capctxt->YUVinfo.InputMode = YUV422;
    Capctxt->YUVinfo.YUV422Format = CAP_IN_YUV422_UYVY;
    Capctxt->YUVinfo.ClockPerPixel = CPP_1P2T;

    Capctxt->funen.EnDEMode = MMP_TRUE;
    Capctxt->funen.EnInBT656 = MMP_FALSE;
    Capctxt->funen.EnCrossLineDE = MMP_FALSE;

    Capctxt->funen.EnUseExtDE = MMP_TRUE;
    Capctxt->funen.EnUseExtVRst = MMP_TRUE;
    Capctxt->funen.EnUseExtHRst = MMP_TRUE;

    Capctxt->funen.EnNoHSyncForSensor = MMP_TRUE;

}




