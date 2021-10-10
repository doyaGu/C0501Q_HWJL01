
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
 * PIXELPLUS_PO3100 Set Input Data Pin.
 */
//=============================================================================
static void
_SENSOR_Input_Data_Pin(
    void)
{
    //10 bit
    //MMP_UINT8 Y_16Pin[CAP_DATA_BUS_WIDTH] = { 0,  1,  2,  3,  4,  5,  6,  7, 8, 9, 0, 0};
    // 8 bit

    // sunnie
    //MMP_UINT8 Y_16Pin[CAP_DATA_BUS_WIDTH] = { 2,  3,  4,  5,  6,  7, 8, 9, 0, 0, 0, 0};
    //MMP_UINT8 U_16Pin[CAP_DATA_BUS_WIDTH] = { 0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0};

    // twowin
    printf("_SENSOR_Input_Data_Pin\n");
   
    //MMP_UINT8 U_16Pin[CAP_DATA_BUS_WIDTH] = { 15, 14, 13, 12, 11, 10, 9, 8, 0, 0, 0, 0};

	#if 1  //By H.C try it.
	MMP_UINT8 Y_16Pin[CAP_DATA_BUS_WIDTH] = { 16, 17, 18,  19,  20,  21, 22, 23, 0, 0, 0, 0};   //Benson
	MMP_UINT8 U_16Pin[CAP_DATA_BUS_WIDTH] = { 8,  9,  10,  11,  12,	13,  14,  15, 0, 0, 0, 0}; //Benson
	MMP_UINT8 V_16Pin[CAP_DATA_BUS_WIDTH] = { 0,  1,   2,   3,   4,  5,  6,  7, 0, 0, 0, 0}; //Benson
	#else
	MMP_UINT8 Y_16Pin[CAP_DATA_BUS_WIDTH] = { 0,  1,  2,  3,  4,  5,  6,  7, 0, 0, 0, 0};   //Benson
	MMP_UINT8 U_16Pin[CAP_DATA_BUS_WIDTH] = { 8,  9,  10,  11,  12,	13,  14,  15, 0, 0, 0, 0}; //Benson
	MMP_UINT8 V_16Pin[CAP_DATA_BUS_WIDTH] = { 16, 17, 18,  19,  20,  21,  22,  23, 0, 0, 0, 0}; //Benson
	#endif

    //MMP_UINT8 V_16Pin[CAP_DATA_BUS_WIDTH] = { 0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0};

	//P_UINT8 HS_16Pin[CAP_DATA_BUS_WIDTH] = { 24, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0};
	//P_UINT8 VS_16Pin[CAP_DATA_BUS_WIDTH] = { 25, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0};
	//P_UINT8 DE_16Pin[CAP_DATA_BUS_WIDTH] = { 26, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0};


    /* Input  pin mux setting */
    PalMemcpy(&Capctxt->inmux_info.Y_Pin_Num, &Y_16Pin, CAP_DATA_BUS_WIDTH);
    PalMemcpy(&Capctxt->inmux_info.U_Pin_Num, &U_16Pin, CAP_DATA_BUS_WIDTH);
    PalMemcpy(&Capctxt->inmux_info.V_Pin_Num, &V_16Pin, CAP_DATA_BUS_WIDTH);

	Capctxt->inmux_info.HS_Pin_Num = 24;
	Capctxt->inmux_info.VS_Pin_Num = 25;
	Capctxt->inmux_info.DE_Pin_Num = 26;

	//lMemcpy(&Capctxt->inmux_info.HS_Pin_Num, &HS_16Pin, CAP_DATA_BUS_WIDTH);
    //lMemcpy(&Capctxt->inmux_info.VS_Pin_Num, &VS_16Pin, CAP_DATA_BUS_WIDTH);
    //lMemcpy(&Capctxt->inmux_info.DE_Pin_Num, &DE_16Pin, CAP_DATA_BUS_WIDTH);

}

//=============================================================================
/**
 * ADV7180 Setting.
 */
//=============================================================================
static void
_SENSOR_Setting(
    void)
{
    MMP_UINT16 i;
	printf("run ADV7180 setting\n");

    _SENSOR_Input_Data_Pin();

    Capctxt->iomode_info.EnInternalRxHDMI = 0x0; // 0: VDO Source from IO Pad.

    Capctxt->inmux_info.UCLKSrc = 0x0; //0:nothing (Needs to ask H.C.)  1: extern IO, 2: Internal colorbar , 4:Internal LCD  // Benson
    Capctxt->inmux_info.UCLKInv = 0x0;
    Capctxt->inmux_info.UCLKDly = 0x3; //Uclk delay setting to 0x3 , but I don`t know why, ask H.C.  
    Capctxt->inmux_info.UCLKRatio = 0x0;
    Capctxt->inmux_info.EnUCLK = 0x1;
    Capctxt->inmux_info.UCLKVDSel = 27;  //still don`t know.
    Capctxt->inmux_info.UCLKAutoDlyEn = MMP_TRUE; 

    /* Output pin mux setting */
    for (i = 0; i < 36; i++)
        Capctxt->outpin_info.CAPIOVDOUTSEL[i] = 0;  //win32 test seems doesn`t need it.
    Capctxt->outpin_info.CAPIOVDOUTSEL_X = 0;

    /* I/O Setting */
	#if 0
    Capctxt->iomode_info.CAPIOEN_VD_15_0 = 0xFFFF;  //1111_1111_1111_1111
    Capctxt->iomode_info.CAPIOEN_VD_31_16 = 0x0; //1111_1111_1111_1111
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
	#endif

	Capctxt->iomode_info.CAPIOFFEn_VD_00_31 = 0x070000ff;//0xFFFFFFFF;  //Still ask H.C.
	Capctxt->iomode_info.CAPIOFFEn_VD_35_32 = 0;  // still ask H.C.


	#if 0 // Benson
    Capctxt->iomode_info.CAPIOFFEn_VD_00_15 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_16_31 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_32_47 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_48_54 = 0x7F;
	#endif

    /* Input Format default Setting */
    Capctxt->ininfo.EmbeddedSync = BT_601;
    Capctxt->ininfo.Interleave = Interleaving;
	Capctxt->ininfo.VSyncSkip = 0; //Benson 
	Capctxt->ininfo.HSyncSkip = 3; //Benson 
	Capctxt->ininfo.HSnapV = 1; //Benson
	Capctxt->ininfo.FRCPattern = 26; //(0x1A)Benson ,ask H.C.
	Capctxt->ininfo.FRCPeriod = 0x4; //Benson  ,ask H.C.
	Capctxt->ininfo.WrMergeThresld =16; //Benson ,ask H.C. 

    /* Input Data Format Setting */
    // 8bit bus
    Capctxt->YUVinfo.InputMode = RGB888;
    //twowin
	Capctxt->YUVinfo.ColorOrder =  Capctxt->YUVinfo.YUV422Format = CAP_IN_YUYV; //Benson
	
    //Capctxt->YUVinfo.ClockPerPixel = CPP_1P1T; //don`t need it. Benson

    Capctxt->funen.EnDEMode = MMP_TRUE;
    Capctxt->funen.EnInBT656 = MMP_FALSE;

	Capctxt->funen.EnAutoDetHSPol = MMP_TRUE; //Benson 
	Capctxt->funen.EnAutoDetVSPol = MMP_TRUE; //Benson 
    
	#if 1
	Capctxt->funen.EnHSync = MMP_TRUE;
	#else
	Capctxt->funen.EnCrossLineDE = MMP_FALSE;
    Capctxt->funen.EnUseExtDE = MMP_FALSE;
    Capctxt->funen.EnUseExtVRst = MMP_FALSE;
    Capctxt->funen.EnUseExtHRst = MMP_FALSE;
    Capctxt->funen.EnNoHSyncForSensor = MMP_FALSE;        
	#endif
}
