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

#ifdef COMPOSITE_DEV
//=============================================================================
/**
 * ADV7180 Set Input Data Pin.
 */
//=============================================================================
static void 
_ADV7180_Input_Data_Pin(
    CAP_CONTEXT *Capctxt)
{
	printf("_ADV7180_Input_Data_Pin\n");

    //Reference Design 2012/09/12
    //RGB 24Bit Pin LSB--->MSB (D0--->D11) 
    
	#if 1  //By H.C try it.
	//RGB888
	//MMP_UINT8 R_Y_24Pin[CAP_DATA_BUS_WIDTH] = { 16, 17, 18,  19,  20,  21, 22, 23, 0, 0, 0, 0};   //Benson
	//MMP_UINT8 G_U_24Pin[CAP_DATA_BUS_WIDTH] = { 8,  9,  10,  11,  12,	13,  14,  15, 0, 0, 0, 0}; //Benson
	//MMP_UINT8 B_V_24Pin[CAP_DATA_BUS_WIDTH] = { 0,  1,   2,   3,   4,  5,  6,  7, 0, 0, 0, 0}; //Benson
    //YUV444
	//MMP_UINT8 B_V_24Pin[CAP_DATA_BUS_WIDTH] = { 16, 17, 18,  19,  20,  21, 22, 23, 0, 0, 0, 0};   
    //MMP_UINT8 R_Y_24Pin[CAP_DATA_BUS_WIDTH] = { 8,  9,  10,  11,  12,	13,  14,  15, 0, 0, 0, 0};
	//MMP_UINT8 G_U_24Pin[CAP_DATA_BUS_WIDTH] = { 0,  1,   2,   3,   4,  5,  6,  7, 0, 0, 0, 0}; 
    //YUV422 16bit
	MMP_UINT8 G_U_24Pin[CAP_DATA_BUS_WIDTH] = { 16, 17, 18,  19,  20,  21, 22, 23, 0, 0, 0, 0};   
	MMP_UINT8 R_Y_24Pin[CAP_DATA_BUS_WIDTH] = { 8,  9,  10,  11,  12,	13,  14,  15, 0, 0, 0, 0}; //{ 0,  1,   2,   3,   4,  5,  6,  7, 0, 0, 0, 0};
	MMP_UINT8 B_V_24Pin[CAP_DATA_BUS_WIDTH] = { 0,  1,   2,   3,   4,  5,  6,  7, 0, 0, 0, 0}; 

	#else
	MMP_UINT8 Y_16Pin[CAP_DATA_BUS_WIDTH] = { 0,  1,  2,  3,  4,  5,  6,  7, 0, 0, 0, 0};
	MMP_UINT8 U_16Pin[CAP_DATA_BUS_WIDTH] = { 8,  9,  10,  11,  12,	13,  14,  15, 0, 0, 0, 0};
	MMP_UINT8 V_16Pin[CAP_DATA_BUS_WIDTH] = { 16, 17, 18,  19,  20,  21,  22,  23, 0, 0, 0, 0};
	#endif
    
    /* Input  pin mux setting */
    PalMemcpy(&Capctxt->inmux_info.Y_Pin_Num, &R_Y_24Pin, CAP_DATA_BUS_WIDTH);
    PalMemcpy(&Capctxt->inmux_info.U_Pin_Num, &G_U_24Pin, CAP_DATA_BUS_WIDTH);
    PalMemcpy(&Capctxt->inmux_info.V_Pin_Num, &B_V_24Pin, CAP_DATA_BUS_WIDTH);

	Capctxt->inmux_info.HS_Pin_Num = 0;//24;
	Capctxt->inmux_info.VS_Pin_Num = 0;//25;
	Capctxt->inmux_info.DE_Pin_Num = 0;//26;
}

//=============================================================================
/**
 * ADV7180 Setting.
 */
//=============================================================================
static void 
_ADV7180_Setting(
    CAP_CONTEXT *Capctxt)
{
    MMP_UINT16 i;

    _ADV7180_Input_Data_Pin(Capctxt);

    Capctxt->inmux_info.UCLKSrc = 0x0; //0:nothing (Needs to ask H.C.)  1: extern IO, 2: Internal colorbar , 4:Internal LCD  // Benson
    Capctxt->inmux_info.UCLKInv = 0x0;
    Capctxt->inmux_info.UCLKDly = 0x3;
    Capctxt->inmux_info.UCLKRatio = 0x0;
    Capctxt->inmux_info.EnUCLK = 0x1;
    Capctxt->inmux_info.UCLKVDSel = 27;
    Capctxt->inmux_info.UCLKAutoDlyEn = MMP_TRUE; 

    /* Output pin mux setting */
    for (i = 0; i < 36; i++)
        Capctxt->outpin_info.CAPIOVDOUTSEL[i] = 0;  //win32 test seems doesn`t need it.
    Capctxt->outpin_info.CAPIOVDOUTSEL_X = 0;

    /* I/O Setting */
	Capctxt->iomode_info.CAPIOFFEn_VD_00_31 = 0x070000ff;
	Capctxt->iomode_info.CAPIOFFEn_VD_35_32 = 0; 

    /* Input Format default Setting */
    Capctxt->ininfo.EmbeddedSync = BT_656;
    Capctxt->ininfo.Interleave = Interleaving;
	Capctxt->ininfo.VSyncSkip = 0; 
	Capctxt->ininfo.HSyncSkip = 3; 
	Capctxt->ininfo.HSnapV = 1;
	Capctxt->ininfo.WrMergeThresld =16;
	Capctxt->ininfo.NV12Format = UV;

    /* Input Data Format Setting */
    // 8bit bus
    Capctxt->YUVinfo.InputMode = YUV422;//RGB888;
	
    //twowin
	Capctxt->YUVinfo.ColorOrder =  Capctxt->YUVinfo.YUV422Format = CAP_IN_UYVY; //CAP_IN_YUYV;
	Capctxt->YUVinfo.InputWidth= PIN_8_10_12BITS;//PIN_16_20_24BITS;//PIN_24_30_36BITS;
    Capctxt->funen.EnDEMode = MMP_TRUE;
    Capctxt->funen.EnInBT656 = MMP_TRUE;//MMP_FALSE;
	Capctxt->funen.EnHSync = MMP_TRUE;
	Capctxt->funen.EnAutoDetHSPol = MMP_TRUE;
	Capctxt->funen.EnAutoDetVSPol = MMP_TRUE; 

	if(Capctxt->funen.EnHSync)
		Capctxt->ininfo.CheckHS = MMP_TRUE;

	if(Capctxt->funen.EnDEMode)
		Capctxt->ininfo.CheckDE = MMP_TRUE;
	
}

//=============================================================================
/**
 * TVP5150 Set Input Data Pin.
 */
//=============================================================================
static void 
_TVP5150_Input_Data_Pin(
    void)
{
    printf("_TVP5150_Input_Data_Pin\n");
    //Reference Design 2012/09/12
    //RGB 24Bit Pin LSB--->MSB (D0--->D11) 
    MMP_UINT8 Y_24Pin[CAP_DATA_BUS_WIDTH] = { 0,  1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0};
    MMP_UINT8 U_24Pin[CAP_DATA_BUS_WIDTH] = { 0,  1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0};
    MMP_UINT8 V_24Pin[CAP_DATA_BUS_WIDTH] = { 0,  1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0};
    
    /* Input  pin mux setting */
    PalMemcpy(&Capctxt->inmux_info.Y_Pin_Num, &Y_24Pin, CAP_DATA_BUS_WIDTH);
    PalMemcpy(&Capctxt->inmux_info.U_Pin_Num, &U_24Pin, CAP_DATA_BUS_WIDTH);
    PalMemcpy(&Capctxt->inmux_info.V_Pin_Num, &V_24Pin, CAP_DATA_BUS_WIDTH);
    
    
}

//=============================================================================
/**
 * TVP5150 Setting.
 */
//=============================================================================
static void 
_TVP5150_Setting(
    void)
{
    MMP_UINT16 i;
    
    printf("_TVP5150_Setting\n");
    _TVP5150_Input_Data_Pin();
    
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
    Capctxt->iomode_info.CAPIOEN_PMCLK = 0x1;    

    Capctxt->iomode_info.CAPIOFFEn_DE = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_Field = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_HS = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_VS = 0x1;
#if 0
    Capctxt->iomode_info.CAPIOFFEn_VD_00_15 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_16_31 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_32_47 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_48_54 = 0x7F;
#endif

    /* Input Format default Setting */
    Capctxt->ininfo.EmbeddedSync = BT_656; //BT_601; //Benson
    Capctxt->ininfo.Interleave = Interleaving; //Progressive;   

    /* Input Data Format Setting */
    // 8bit bus
    Capctxt->YUVinfo.InputMode = YUV422;
    Capctxt->YUVinfo.YUV422Format = CAP_IN_UYVY;
#if 0
    Capctxt->YUVinfo.ClockPerPixel = CPP_1P2T;
#endif
        
    Capctxt->funen.EnDEMode = MMP_FALSE;
    Capctxt->funen.EnInBT656 = MMP_TRUE; //MMP_FALSE; // Benson
    Capctxt->funen.EnCrossLineDE = MMP_FALSE;

}
#endif

#ifdef COMPONENT_DEV
//=============================================================================
/**
 * CAT9883 Set Input Data Pin.
 */
//=============================================================================
static void 
_CAT9883_Input_Data_Pin(
    void)
{
    //Reference Design 2012/09/12
    //RGB 24Bit Pin LSB--->MSB (D0--->D11)
    MMP_UINT8 G_Y_24Pin[CAP_DATA_BUS_WIDTH] = { 8,  9, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0};
    MMP_UINT8 B_U_24Pin[CAP_DATA_BUS_WIDTH] = { 0,  1,  2,  3,  4,  5,  6,  7, 0, 0, 0, 0}; 
    MMP_UINT8 R_V_24Pin[CAP_DATA_BUS_WIDTH] = {16, 17, 18, 19, 20, 21, 22, 23, 0, 0, 0, 0};

    /* Input  pin mux setting */
    if (Capctxt->funen.EnCSFun == MMP_TRUE) //RGB Input
    {
        PalMemcpy(&Capctxt->inmux_info.Y_Pin_Num, &R_V_24Pin, CAP_DATA_BUS_WIDTH);
        PalMemcpy(&Capctxt->inmux_info.U_Pin_Num, &G_Y_24Pin, CAP_DATA_BUS_WIDTH);
        PalMemcpy(&Capctxt->inmux_info.V_Pin_Num, &B_U_24Pin, CAP_DATA_BUS_WIDTH);      
    }
    else //YPbPr Input
    {
        PalMemcpy(&Capctxt->inmux_info.Y_Pin_Num, &G_Y_24Pin, CAP_DATA_BUS_WIDTH);
        PalMemcpy(&Capctxt->inmux_info.U_Pin_Num, &B_U_24Pin, CAP_DATA_BUS_WIDTH);
        PalMemcpy(&Capctxt->inmux_info.V_Pin_Num, &R_V_24Pin, CAP_DATA_BUS_WIDTH);
    }
}

//=============================================================================
/**
 * CAT9883 Setting.
 */
//=============================================================================
static void 
_CAT9883_Setting(
    void)
{
    MMP_UINT16 i;
    
    _CAT9883_Input_Data_Pin();
    
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
    Capctxt->iomode_info.CAPIOEN_PMCLK = 0x1;    

    Capctxt->iomode_info.CAPIOFFEn_DE = 0x0;
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
    // 16bit bus
    //Capctxt->YUVinfo.InputMode = YUV422;
    //Capctxt->YUVinfo.YUV422Format = CAP_IN_YUV422_VU_YY;
    //Capctxt->YUVinfo.ClockPerPixel = CPP_1P1T;

    //24bin bus
    Capctxt->YUVinfo.InputMode = YUV444;
    Capctxt->YUVinfo.ClockPerPixel = CPP_1P1T;
        
    Capctxt->funen.EnDEMode = MMP_FALSE;
    Capctxt->funen.EnInBT656 = MMP_FALSE;
    Capctxt->funen.EnCrossLineDE = MMP_FALSE;
}
#endif

#ifndef EXTERNAL_HDMIRX
//=============================================================================
/**
 * INTERNAL HDMIRX Set Input Data Pin.
 */
//=============================================================================
static void 
_HDMIRX_Input_Data_Pin(
    void)
{   
    //RGB 36Bit Pin LSB--->MSB (D0--->D11)
    MMP_UINT8 R_Y_36Pin[CAP_DATA_BUS_WIDTH] = {24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};
    MMP_UINT8 G_U_36Pin[CAP_DATA_BUS_WIDTH] = {12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23}; 
    MMP_UINT8 B_V_36Pin[CAP_DATA_BUS_WIDTH] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11};

    MMP_UINT8 R_Y_30Pin[CAP_DATA_BUS_WIDTH] = {26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 0, 0};
    MMP_UINT8 G_U_30Pin[CAP_DATA_BUS_WIDTH] = {14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 0, 0}; 
    MMP_UINT8 B_V_30Pin[CAP_DATA_BUS_WIDTH] = { 2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 0, 0};    
        
    MMP_UINT8 R_Y_24Pin[CAP_DATA_BUS_WIDTH] = {28, 29, 30, 31, 32, 33, 34, 35, 0, 0, 0, 0};
    MMP_UINT8 G_U_24Pin[CAP_DATA_BUS_WIDTH] = {16, 17, 18, 19, 20, 21, 22, 23, 0, 0, 0, 0}; 
    MMP_UINT8 B_V_24Pin[CAP_DATA_BUS_WIDTH] = { 4,  5,  6,  7,  8,  9, 10, 11, 0, 0, 0, 0};
      
    if (Capctxt->funen.EnCSFun) //HDMI Rx RGB444 Output
    {
    if (Capctxt->ininfo.ColorDepth == COLOR_DEPTH_12_BITS)
    {
        /* Input  pin mux setting */
        PalMemcpy(&Capctxt->inmux_info.Y_Pin_Num, &R_Y_36Pin, CAP_DATA_BUS_WIDTH);
        PalMemcpy(&Capctxt->inmux_info.U_Pin_Num, &G_U_36Pin, CAP_DATA_BUS_WIDTH);
        PalMemcpy(&Capctxt->inmux_info.V_Pin_Num, &B_V_36Pin, CAP_DATA_BUS_WIDTH);        
    }
    else if (Capctxt->ininfo.ColorDepth == COLOR_DEPTH_10_BITS)
    {
        /* Input  pin mux setting */
        PalMemcpy(&Capctxt->inmux_info.Y_Pin_Num, &R_Y_30Pin, CAP_DATA_BUS_WIDTH);
        PalMemcpy(&Capctxt->inmux_info.U_Pin_Num, &G_U_30Pin, CAP_DATA_BUS_WIDTH);
        PalMemcpy(&Capctxt->inmux_info.V_Pin_Num, &B_V_30Pin, CAP_DATA_BUS_WIDTH);        
    }
    else
    {
        /* Input  pin mux setting */
        PalMemcpy(&Capctxt->inmux_info.Y_Pin_Num, &R_Y_24Pin, CAP_DATA_BUS_WIDTH);
        PalMemcpy(&Capctxt->inmux_info.U_Pin_Num, &G_U_24Pin, CAP_DATA_BUS_WIDTH);
        PalMemcpy(&Capctxt->inmux_info.V_Pin_Num, &B_V_24Pin, CAP_DATA_BUS_WIDTH);            
    }
}
    else //HDMI Rx YUV444/YUV422 Output
    {
        if (Capctxt->ininfo.ColorDepth == COLOR_DEPTH_12_BITS)
        {
            /* Input  pin mux setting */
            PalMemcpy(&Capctxt->inmux_info.Y_Pin_Num, &G_U_36Pin, CAP_DATA_BUS_WIDTH);
            PalMemcpy(&Capctxt->inmux_info.U_Pin_Num, &B_V_36Pin, CAP_DATA_BUS_WIDTH);
            PalMemcpy(&Capctxt->inmux_info.V_Pin_Num, &R_Y_36Pin, CAP_DATA_BUS_WIDTH);
        }
        else if (Capctxt->ininfo.ColorDepth == COLOR_DEPTH_10_BITS)
        {
            /* Input  pin mux setting */
            PalMemcpy(&Capctxt->inmux_info.Y_Pin_Num, &G_U_30Pin, CAP_DATA_BUS_WIDTH);
            PalMemcpy(&Capctxt->inmux_info.U_Pin_Num, &B_V_30Pin, CAP_DATA_BUS_WIDTH);
            PalMemcpy(&Capctxt->inmux_info.V_Pin_Num, &R_Y_30Pin, CAP_DATA_BUS_WIDTH);
        }
        else
        {
            /* Input  pin mux setting */
            PalMemcpy(&Capctxt->inmux_info.Y_Pin_Num, &G_U_24Pin, CAP_DATA_BUS_WIDTH);
            PalMemcpy(&Capctxt->inmux_info.U_Pin_Num, &B_V_24Pin, CAP_DATA_BUS_WIDTH);
            PalMemcpy(&Capctxt->inmux_info.V_Pin_Num, &R_Y_24Pin, CAP_DATA_BUS_WIDTH);
        }
    }
}

static void
_HDMIRX_Output_Data_Pin(
    void)
{
    MMP_UINT8 CAPIOVDOUTSEL [36] = {
     0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00 };

   PalMemcpy(&Capctxt->outpin_info.CAPIOVDOUTSEL, &CAPIOVDOUTSEL, 36);  

}    

//=============================================================================
/**
 * INTERNAL HDMIRX Setting.
 */
//=============================================================================
static void 
_HDMIRX_Setting(
    void)
{
    MMP_UINT16 i;
    
    _HDMIRX_Input_Data_Pin();
    
    Capctxt->iomode_info.EnInternalRxHDMI = 0x1; //1: VDO from internal HDMI Rx. 

    Capctxt->inmux_info.UCLKSrc = 0x1; //0: IO, 1: HDMI, 2: GPIO, 3: PLL3_n1, 4: PLL3_n2
    Capctxt->inmux_info.UCLKInv = 0x0;
    Capctxt->inmux_info.UCLKDly = 0x1; //delay 1ns
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
    Capctxt->iomode_info.CAPIOEN_PMCLK = 0x1;    

    Capctxt->iomode_info.CAPIOFFEn_DE = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_Field = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_HS = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_VS = 0x1;
	#if 0
    Capctxt->iomode_info.CAPIOFFEn_VD_00_15 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_16_31 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_32_47 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_48_54 = 0x7F;
	#endif

    /* Input Format default Setting */
    Capctxt->ininfo.EmbeddedSync = BT_601;
    Capctxt->ininfo.Interleave = Progressive;

    /* Input Data Format Setting */
    Capctxt->YUVinfo.InputMode = YUV444;
	#if 0
    Capctxt->YUVinfo.ClockPerPixel = CPP_1P1T;
	#endif
    
    Capctxt->funen.EnDEMode = MMP_FALSE;
    Capctxt->funen.EnInBT656 = MMP_FALSE;
    Capctxt->funen.EnCrossLineDE = MMP_FALSE;     
    
}
static 
_HDMIRX_LoopThrough_Setting (
    void)
{

   _HDMIRX_Input_Data_Pin();
    
    Capctxt->iomode_info.EnInternalRxHDMI = 0x1; //1: VDO from internal HDMI Rx. 

    Capctxt->inmux_info.UCLKSrc = 0x1; //0: IO, 1: HDMI, 2: GPIO, 3: PLL3_n1, 4: PLL3_n2
    Capctxt->inmux_info.UCLKInv = 0x0;
    Capctxt->inmux_info.UCLKDly = 0x0;
    Capctxt->inmux_info.UCLKRatio = 0x0;
    Capctxt->inmux_info.EnUCLK = 0x1;
    Capctxt->inmux_info.UCLKVDSel = 0x0;    

    /* Output pin mux setting */
    _HDMIRX_Output_Data_Pin();
    Capctxt->outpin_info.CAPIOVDOUTSEL_X = 0x40;

    /* I/O Setting */
    Capctxt->iomode_info.CAPIOEN_VD_15_0 = 0x0; //0000_0000_0000_0000
    Capctxt->iomode_info.CAPIOEN_VD_31_16 = 0x0;    //0000_0000_0000_0000
    Capctxt->iomode_info.CAPIOEN_VD_32_35 = 0x0;    //0000_0000_0000_0000
    Capctxt->iomode_info.CAPIOEN_X = 0x0;
    Capctxt->iomode_info.CAPIOEN_HS = 0x0;
    Capctxt->iomode_info.CAPIOEN_VS = 0x0;
    Capctxt->iomode_info.CAPIOEN_DE = 0x1;
    Capctxt->iomode_info.CAPIOEN_Field = 0x0;
    Capctxt->iomode_info.CAPIOEN_PMCLK = 0x1;    

    Capctxt->iomode_info.CAPIOFFEn_DE = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_Field = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_HS = 0x1;
    Capctxt->iomode_info.CAPIOFFEn_VS = 0x1;
    Capctxt->iomode_info.HDMICLKDly = 0x0;
    Capctxt->iomode_info.HDMICLKInv = 0x1;

	#if 0
    Capctxt->iomode_info.CAPIOFFEn_VD_00_15 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_16_31 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_32_47 = 0xFFFF;
    Capctxt->iomode_info.CAPIOFFEn_VD_48_54 = 0x7F;
	#endif

    /* Input Format default Setting */
    Capctxt->ininfo.EmbeddedSync = BT_601;
    Capctxt->ininfo.Interleave = Progressive;

    /* Input Data Format Setting */
    Capctxt->YUVinfo.InputMode = YUV444;
	#if 0
    Capctxt->YUVinfo.ClockPerPixel = CPP_1P1T;
	#endif
    
    Capctxt->funen.EnDEMode = MMP_TRUE;
    Capctxt->funen.EnInBT656 = MMP_FALSE;  
    Capctxt->funen.EnCrossLineDE = MMP_FALSE;
}
#else
//=============================================================================
/**
 * EXTERNAL HDMIRX Set Input Data Pin.
 */
//=============================================================================
static void 
_HDMIRX_Input_Data_Pin(
    void)
{
    //RGB 24Bit Pin LSB--->MSB (D0--->D11)
    MMP_UINT8 R_Y_24Pin[CAP_DATA_BUS_WIDTH] = {16, 17, 18, 19, 20, 21, 22, 23, 0, 0, 0, 0};
    MMP_UINT8 G_U_24Pin[CAP_DATA_BUS_WIDTH] = { 8,  9, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0}; 
    MMP_UINT8 B_V_24Pin[CAP_DATA_BUS_WIDTH] = { 0,  1,  2,  3,  4,  5,  6,  7, 0, 0, 0, 0};
    
    /* Input  pin mux setting */
    PalMemcpy(&Capctxt->inmux_info.Y_Pin_Num, &R_Y_24Pin, CAP_DATA_BUS_WIDTH);
    PalMemcpy(&Capctxt->inmux_info.U_Pin_Num, &G_U_24Pin, CAP_DATA_BUS_WIDTH);
    PalMemcpy(&Capctxt->inmux_info.V_Pin_Num, &B_V_24Pin, CAP_DATA_BUS_WIDTH);      
    
}
//=============================================================================
/**
 * EXTERNAL HDMIRX Setting.
 */
//=============================================================================
static void 
_HDMIRX_Setting(
    void)
{
    MMP_UINT16 i;
    
    _HDMIRX_Input_Data_Pin();

    Capctxt->iomode_info.EnInternalRxHDMI = 0x0; //1: VDO Source from IO Pad. 

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
    Capctxt->iomode_info.CAPIOEN_PMCLK = 0x1;    

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
    Capctxt->YUVinfo.InputMode = YUV444;
    Capctxt->YUVinfo.ClockPerPixel = CPP_1P1T;
    
    Capctxt->funen.EnDEMode = MMP_FALSE;
    Capctxt->funen.EnInBT656 = MMP_FALSE;
    Capctxt->funen.EnCrossLineDE = MMP_FALSE;  
}
#endif



