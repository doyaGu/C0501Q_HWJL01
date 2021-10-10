///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <hdmitx_drv.c>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2011/07/19
//   @fileversion: ITE_HDMITX_SAMPLE_2.03
//******************************************/

/////////////////////////////////////////////////////////////////////
// HDMITX.C
// Driver code for platform independent
/////////////////////////////////////////////////////////////////////
#include "linux/timer.h"
#include "hdmitx/typedef.h"
#include "hdmitx/hdmitx.h"
#include "hdmitx/hdmitx_drv.h"
#define FALLING_EDGE_TRIGGER


#define MSCOUNT 1000
#define LOADING_UPDATE_TIMEOUT (3000/32)    // 3sec
// unsigned int u8msTimer = 0 ;
// unsigned int TimerServF = TRUE ;



//////////////////////////////////////////////////////////////////////
// Authentication status
//////////////////////////////////////////////////////////////////////

// #define TIMEOUT_WAIT_AUTH MS(2000)

#define Switch_HDMITX_Bank_66121(x)   HDMITX_WriteI2C_Byte_66121(0x0f,(x)&1)

#define HDMITX_OrREG_Byte_66121(reg,ormask) HDMITX_WriteI2C_Byte_66121(reg,(HDMITX_ReadI2C_Byte_66121(reg) | (ormask)))
#define HDMITX_AndREG_Byte_66121(reg,andmask) HDMITX_WriteI2C_Byte_66121(reg,(HDMITX_ReadI2C_Byte_66121(reg) & (andmask)))
#define HDMITX_SetREG_Byte_66121(reg,andmask,ormask) HDMITX_WriteI2C_Byte_66121(reg,((HDMITX_ReadI2C_Byte_66121(reg) & (andmask))|(ormask)))


HDMITX_DETIMING timingDE_66121;
static INSTANCE Instance[HDMITX_INSTANCE_MAX] ;
static BOOL bForceCTS = FALSE;

//////////////////////////////////////////////////////////////////////
// Function Prototype
//////////////////////////////////////////////////////////////////////

// static BOOL IsRxSense();

static void WaitTxVidStable();
static void SetInputMode(BYTE InputMode,BYTE bInputSignalType);
static void SetCSCScale(BYTE bInputMode,BYTE bOutputMode);
// static void SetupAFE(BYTE ucFreqInMHz);
static void SetupAFE(VIDEOPCLKLEVEL PCLKLevel);
static void FireAFE();


//static SYS_STATUS SetAudioFormat(BYTE NumChannel,BYTE AudioEnable,BYTE bSampleFreq,BYTE AudSWL,BYTE AudioCatCode);
static SYS_STATUS SetNCTS(ULONG PCLK,BYTE Fs);

static void AutoAdjustAudio();
static void SetupAudioChannel();

static SYS_STATUS SetAVIInfoFrame(AVI_InfoFrame *pAVIInfoFrame);
static SYS_STATUS SetAudioInfoFrame(Audio_InfoFrame *pAudioInfoFrame);
static SYS_STATUS SetSPDInfoFrame(SPD_InfoFrame *pSPDInfoFrame);
static SYS_STATUS SetMPEGInfoFrame(MPEG_InfoFrame *pMPGInfoFrame);
SYS_STATUS ReadEDID_66121(BYTE *pData,BYTE bSegment,BYTE offset,SHORT Count);
static void AbortDDC();
static void ClearDDCFIFO();
static void ClearDDCFIFO();
static void GenerateDDCSCLK();
#ifdef SUPPORT_HDCP
static SYS_STATUS HDCP_EnableEncryption();
static void HDCP_ResetAuth();
static void HDCP_Auth_Fire();
static void HDCP_StartAnCipher();
static void HDCP_StopAnCipher();
static void HDCP_GenerateAn();
static SYS_STATUS HDCP_GetVr(BYTE *pVr);
static SYS_STATUS HDCP_GetBCaps(PBYTE pBCaps ,unsigned int *pBStatus);
static SYS_STATUS HDCP_GetBKSV(BYTE *pBKSV);
static SYS_STATUS HDCP_Authenticate();
static SYS_STATUS HDCP_Authenticate_Repeater();
static SYS_STATUS HDCP_VerifyIntegration();
static SYS_STATUS HDCP_GetKSVList(BYTE *pKSVList,BYTE cDownStream);
static SYS_STATUS HDCP_CheckSHA(BYTE M0[],unsigned int BStatus,BYTE KSVList[],int devno,BYTE Vr[]);
static void HDCP_ResumeAuthentication();
static void HDCP_Reset();
#endif



static void ENABLE_NULL_PKT();
static void ENABLE_ACP_PKT();
static void ENABLE_ISRC1_PKT();
static void ENABLE_ISRC2_PKT();
static void ENABLE_AVI_INFOFRM_PKT();
static void ENABLE_AUD_INFOFRM_PKT();
static void ENABLE_SPD_INFOFRM_PKT();
static void ENABLE_MPG_INFOFRM_PKT();
static void ENABLE_GeneralPurpose_PKT();

static void DISABLE_NULL_PKT();
static void DISABLE_ACP_PKT();
static void DISABLE_ISRC1_PKT();
static void DISABLE_ISRC2_PKT();
static void DISABLE_AVI_INFOFRM_PKT();
static void DISABLE_AUD_INFOFRM_PKT();
static void DISABLE_SPD_INFOFRM_PKT();
static void DISABLE_MPG_INFOFRM_PKT();
static void DISABLE_GeneralPurpose_PKT();

static BYTE countbit(BYTE b);

//void DumpCatHDMITXReg_66121();


//////////////////////////////////////////////////////////////////////
// Function Body.
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// utility function for main..
//////////////////////////////////////////////////////////////////////


#ifndef DISABLE_HDMITX_CSC
    extern _CODE BYTE bTxCSCOffset_16_235[] ;
    extern _CODE BYTE bTxCSCOffset_0_255[] ;
    #if (defined (SUPPORT_OUTPUTYUV)) && (defined (SUPPORT_INPUTRGB))

        extern _CODE BYTE bTxCSCMtx_RGB2YUV_ITU601_16_235[] ;
        extern _CODE BYTE bTxCSCMtx_RGB2YUV_ITU601_0_255[] ;
        extern _CODE BYTE bTxCSCMtx_RGB2YUV_ITU709_16_235[] ;
        extern _CODE BYTE bTxCSCMtx_RGB2YUV_ITU709_0_255[] ;
    #endif

    #if (defined (SUPPORT_OUTPUTRGB)) && (defined (SUPPORT_INPUTYUV))

        extern _CODE BYTE bTxCSCMtx_YUV2RGB_ITU601_16_235[] ;
        extern _CODE BYTE bTxCSCMtx_YUV2RGB_ITU601_0_255[] ;
        extern _CODE BYTE bTxCSCMtx_YUV2RGB_ITU709_16_235[] ;
        extern _CODE BYTE bTxCSCMtx_YUV2RGB_ITU709_0_255[] ;

    #endif
#endif// DISABLE_HDMITX_CSC

#define INIT_CLK_HIGH
// #define INIT_CLK_LOW

#ifndef INV_INPUT_PCLK
#define PCLKINV 0
#else
#define PCLKINV B_VDO_LATCH_EDGE
#endif

#ifndef INV_INPUT_ACLK
    #define InvAudCLK 0
#else
    #define InvAudCLK B_AUDFMT_FALL_EDGE_SAMPLE_WS
#endif


_CODE RegSetEntry HDMITX_Init_Table[] = {

    {0x0F, 0x40, 0x00},

    {0x62, 0x08, 0x00},
    {0x64, 0x04, 0x00},
    {0x01,0x00,0x00},//idle(100);

    {0x04, 0x20, 0x20},
    {0x04, 0x1D, 0x1D},
    {0x01,0x00,0x00},//idle(100);
    {0x0F, 0x01, 0x00}, // bank 0 ;
    #ifdef INIT_CLK_LOW
        {0x62, 0x90, 0x10},
        {0x64, 0x89, 0x09},
        {0x68, 0x10, 0x10},
    #endif
	

    {0xD1, 0x0E, 0x0C},
    {0x65, 0x03, 0x00},
    #ifdef NON_SEQUENTIAL_YCBCR422 // for ITE HDMIRX
        {0x71, 0xFC, 0x1C},
    #else
        {0x71, 0xFC, 0x18},
    #endif
	

    {0x8D, 0xFF, 0x9C},//CEC_I2C_SLAVE_ADDR
    {0x0F, 0x08, 0x08},

    {0xF8,0xFF,0xC3},
    {0xF8,0xFF,0xA5},  
    {0x20, 0x80, 0x80},
    {0x37, 0x01, 0x00},
    {0x20, 0x80, 0x00},
    
    {0xF8,0xFF,0xFF},
    // 2014/01/07 HW Request for ROSC stable

    {0x5D,0x03,0x01},
    //~2014/01/07
#ifdef USE_IT66120
    {0x5A, 0x02, 0x00},
    {0xE2, 0xFF, 0xFF},
#endif

    {0x59, 0xD8, 0x40|PCLKINV},
    {0xE1, 0x20, InvAudCLK},
    {0x05, 0xC0, 0x40},
    {REG_TX_INT_MASK1, 0xFF, ~(B_RXSEN_MASK|B_HPD_MASK)},
    {REG_TX_INT_MASK2, 0xFF, ~(B_KSVLISTCHK_MASK|B_T_AUTH_DONE_MASK|B_AUTH_FAIL_MASK)},
    {REG_TX_INT_MASK3, 0xFF, ~(B_VIDSTABLE_MASK)},
    {0x0C, 0xFF, 0xFF},
    {0x0D, 0xFF, 0xFF},
    {0x0E, 0x03, 0x03},

    {0x0C, 0xFF, 0x00},
    {0x0D, 0xFF, 0x00},
    {0x0E, 0x02, 0x00},
    {0x09, 0x03, 0x00}, // Enable HPD and RxSen Interrupt
    {0x20,0x01,0x00},

	{0xC9,0xFF,0x00},
	{0xCA,0xFF,0x00},
	{0xCB,0xFF,0x00},
	{0xCC,0xFF,0x00},
	{0xCD,0xFF,0x00},
	{0xCF,0xFF,0x00},
	{0xD0,0xFF,0x00},

    {0,0,0}
};
_CODE RegSetEntry HDMITX_DefaultVideo_Table[] = {

    ////////////////////////////////////////////////////
    // Config default output format.
    ////////////////////////////////////////////////////
    {0x72, 0xff, 0x00},
    {0x70, 0xff, 0x00},
#ifndef DEFAULT_INPUT_YCBCR
// GenCSC\RGB2YUV_ITU709_16_235.c
    {0x72, 0xFF, 0x02},
    {0x73, 0xFF, 0x00},
    {0x74, 0xFF, 0x80},
    {0x75, 0xFF, 0x00},
    {0x76, 0xFF, 0xB8},
    {0x77, 0xFF, 0x05},
    {0x78, 0xFF, 0xB4},
    {0x79, 0xFF, 0x01},
    {0x7A, 0xFF, 0x93},
    {0x7B, 0xFF, 0x00},
    {0x7C, 0xFF, 0x49},
    {0x7D, 0xFF, 0x3C},
    {0x7E, 0xFF, 0x18},
    {0x7F, 0xFF, 0x04},
    {0x80, 0xFF, 0x9F},
    {0x81, 0xFF, 0x3F},
    {0x82, 0xFF, 0xD9},
    {0x83, 0xFF, 0x3C},
    {0x84, 0xFF, 0x10},
    {0x85, 0xFF, 0x3F},
    {0x86, 0xFF, 0x18},
    {0x87, 0xFF, 0x04},
#else
// GenCSC\YUV2RGB_ITU709_16_235.c
    {0x0F, 0x01, 0x00},
    {0x72, 0xFF, 0x03},
    {0x73, 0xFF, 0x00},
    {0x74, 0xFF, 0x80},
    {0x75, 0xFF, 0x00},
    {0x76, 0xFF, 0x00},
    {0x77, 0xFF, 0x08},
    {0x78, 0xFF, 0x53},
    {0x79, 0xFF, 0x3C},
    {0x7A, 0xFF, 0x89},
    {0x7B, 0xFF, 0x3E},
    {0x7C, 0xFF, 0x00},
    {0x7D, 0xFF, 0x08},
    {0x7E, 0xFF, 0x51},
    {0x7F, 0xFF, 0x0C},
    {0x80, 0xFF, 0x00},
    {0x81, 0xFF, 0x00},
    {0x82, 0xFF, 0x00},
    {0x83, 0xFF, 0x08},
    {0x84, 0xFF, 0x00},
    {0x85, 0xFF, 0x00},
    {0x86, 0xFF, 0x87},
    {0x87, 0xFF, 0x0E},
#endif
    // 2012/12/20 added by Keming's suggestion test
    {0x88, 0xF0, 0x00},
    //~jauchih.tseng@ite.com.tw
    {0x04, 0x08, 0x00},
    {0,0,0}
};
_CODE RegSetEntry HDMITX_SetHDMI_Table[] = {

    ////////////////////////////////////////////////////
    // Config default HDMI Mode
    ////////////////////////////////////////////////////
    {0xC0, 0x01, 0x01},
    {0xC1, 0x03, 0x03},
    {0xC6, 0x03, 0x03},
    {0,0,0}
};

_CODE RegSetEntry HDMITX_SetDVI_Table[] = {

    ////////////////////////////////////////////////////
    // Config default HDMI Mode
    ////////////////////////////////////////////////////
    {0x0F, 0x01, 0x01},
    {0x58, 0xFF, 0x00},
    {0x0F, 0x01, 0x00},
    {0xC0, 0x01, 0x00},
    {0xC1, 0x03, 0x02},
    {0xC6, 0x03, 0x00},
    {0,0,0}
};

_CODE RegSetEntry HDMITX_DefaultAVIInfo_Table[] = {

    ////////////////////////////////////////////////////
    // Config default avi infoframe
    ////////////////////////////////////////////////////
    {0x0F, 0x01, 0x01},
    {0x58, 0xFF, 0x10},
    {0x59, 0xFF, 0x08},
    {0x5A, 0xFF, 0x00},
    {0x5B, 0xFF, 0x00},
    {0x5C, 0xFF, 0x00},
    {0x5D, 0xFF, 0x57},
    {0x5E, 0xFF, 0x00},
    {0x5F, 0xFF, 0x00},
    {0x60, 0xFF, 0x00},
    {0x61, 0xFF, 0x00},
    {0x62, 0xFF, 0x00},
    {0x63, 0xFF, 0x00},
    {0x64, 0xFF, 0x00},
    {0x65, 0xFF, 0x00},
    {0x0F, 0x01, 0x00},
    {0xCD, 0x03, 0x03},
    {0,0,0}
};
_CODE RegSetEntry HDMITX_DeaultAudioInfo_Table[] = {

    ////////////////////////////////////////////////////
    // Config default audio infoframe
    ////////////////////////////////////////////////////
    {0x0F, 0x01, 0x01},
    {0x68, 0xFF, 0x00},
    {0x69, 0xFF, 0x00},
    {0x6A, 0xFF, 0x00},
    {0x6B, 0xFF, 0x00},
    {0x6C, 0xFF, 0x00},
    {0x6D, 0xFF, 0x71},
    {0x0F, 0x01, 0x00},
    {0xCE, 0x03, 0x03},

    {0,0,0}
};

_CODE RegSetEntry HDMITX_Aud_CHStatus_LPCM_20bit_48Khz[] =
{
    {0x0F, 0x01, 0x01},
    {0x33, 0xFF, 0x00},
    {0x34, 0xFF, 0x18},
    {0x35, 0xFF, 0x00},
    {0x91, 0xFF, 0x00},
    {0x92, 0xFF, 0x00},
    {0x93, 0xFF, 0x01},
    {0x94, 0xFF, 0x00},
    {0x98, 0xFF, 0x02},
    {0x99, 0xFF, 0xDA},
    {0x0F, 0x01, 0x00},
    {0,0,0}//end of table
} ;

_CODE RegSetEntry HDMITX_AUD_SPDIF_2ch_24bit[] =
{
    {0x0F, 0x11, 0x00},
    {0x04, 0x14, 0x04},
    {0xE0, 0xFF, 0xD1},
    {0xE1, 0xFF, 0x01},
    {0xE2, 0xFF, 0xE4},
    {0xE3, 0xFF, 0x10},
    {0xE4, 0xFF, 0x00},
    {0xE5, 0xFF, 0x00},
    {0x04, 0x14, 0x00},
    {0,0,0}//end of table
} ;

_CODE RegSetEntry HDMITX_AUD_I2S_2ch_24bit[] =
{
    {0x0F, 0x11, 0x00},
    {0x04, 0x14, 0x04},
    {0xE0, 0xFF, 0xC1},
    {0xE1, 0xFF, 0x01},
#ifdef USE_IT66120
    {0x5A, 0x02, 0x00},
    {0xE2, 0xFF, 0xFF},
#else
    {0xE2, 0xFF, 0xE4},
#endif
    {0xE3, 0xFF, 0x00},
    {0xE4, 0xFF, 0x00},
    {0xE5, 0xFF, 0x00},
    {0x04, 0x14, 0x00},
    {0,0,0}//end of table
} ;

_CODE RegSetEntry HDMITX_DefaultAudio_Table[] = {

    ////////////////////////////////////////////////////
    // Config default audio output format.
    ////////////////////////////////////////////////////
    {0x0F, 0x21, 0x00},
    {0x04, 0x14, 0x04},
    {0xE0, 0xFF, 0xC1},
    {0xE1, 0xFF, 0x01},
#ifdef USE_IT66120
    {0xE2, 0xFF, 0xFF},
#else
    {0xE2, 0xFF, 0xE4},
#endif
    {0xE3, 0xFF, 0x00},
    {0xE4, 0xFF, 0x00},
    {0xE5, 0xFF, 0x00},
    {0x0F, 0x01, 0x01},
    {0x33, 0xFF, 0x00},
    {0x34, 0xFF, 0x18},
    {0x35, 0xFF, 0x00},
    {0x91, 0xFF, 0x00},
    {0x92, 0xFF, 0x00},
    {0x93, 0xFF, 0x01},
    {0x94, 0xFF, 0x00},
    {0x98, 0xFF, 0x02},
    {0x99, 0xFF, 0xDB},
    {0x0F, 0x01, 0x00},
    {0x04, 0x14, 0x00},

    {0x00, 0x00, 0x00} // End of Table.
} ;

_CODE RegSetEntry HDMITX_PwrDown_Table[] = {
     // Enable GRCLK
     {0x0F, 0x40, 0x00},
     // PLL Reset
     {0x61, 0x10, 0x10},   // DRV_RST
     {0x62, 0x08, 0x00},   // XP_RESETB
     {0x64, 0x04, 0x00},   // IP_RESETB
     {0x01, 0x00, 0x00}, // idle(100);

     // PLL PwrDn
     {0x61, 0x20, 0x20},   // PwrDn DRV
     {0x62, 0x44, 0x44},   // PwrDn XPLL
     {0x64, 0x40, 0x40},   // PwrDn IPLL

     // HDMITX PwrDn
     {0x05, 0x01, 0x01},   // PwrDn PCLK
     {0x0F, 0x78, 0x78},   // PwrDn GRCLK
     {0x00, 0x00, 0x00} // End of Table.
};

_CODE RegSetEntry HDMITX_PwrOn_Table[] = {
    {0x0F, 0x78, 0x38},   // PwrOn GRCLK
    {0x05, 0x01, 0x00},   // PwrOn PCLK

    // PLL PwrOn
    {0x61, 0x20, 0x00},   // PwrOn DRV
    {0x62, 0x44, 0x00},   // PwrOn XPLL
    {0x64, 0x40, 0x00},   // PwrOn IPLL

    // PLL Reset OFF
    {0x61, 0x10, 0x00},   // DRV_RST
    {0x62, 0x08, 0x08},   // XP_RESETB
    {0x64, 0x04, 0x04},   // IP_RESETB
    {0x0F, 0x78, 0x08},   // PwrOn IACLK
    {0x00, 0x00, 0x00} // End of Table.
};


static void delay1ms(unsigned int ms)
{
    //unsigned int tickCount = PalGetClock();
    //while(PalGetDuration(tickCount) < ms ) ;

    unsigned int tickCount = PalGetClock();
    while(PalGetDuration(tickCount) < ms )
        PalSleep(1);    
    
}

//////////////////////////////////////////////////////////////////////
// external Interface                                                         //
//////////////////////////////////////////////////////////////////////

void
HDMITX_InitInstance_66121(INSTANCE *pInstance)
{
    if(pInstance && 0 < HDMITX_INSTANCE_MAX)
    {
        Instance[0] = *pInstance ;
    }
}

static void C66121_Check_EMEM_sts()
{
    byte timeout ;
    byte uc ;

    Switch_HDMITX_Bank_66121(0);
    HDMITX_WriteI2C_Byte_66121(0xF8,0xC3);//unlock register
    HDMITX_WriteI2C_Byte_66121(0xF8,0xA5);//unlock register

    HDMITX_WriteI2C_Byte_66121(0x22,0x00);


    HDMITX_WriteI2C_Byte_66121(0x10,0x03);//

    HDMITX_WriteI2C_Byte_66121(0x11,0xA0);
    HDMITX_WriteI2C_Byte_66121(0x12,0x00);//
    HDMITX_WriteI2C_Byte_66121(0x13,0x08);
    HDMITX_WriteI2C_Byte_66121(0x14,0x00);//
    HDMITX_WriteI2C_Byte_66121(0x15,0x00);//
    // delay1ms(200);
    Instance[0].TxEMEMStatus=TRUE;


    for( timeout = 0 ; timeout < 250 ; timeout ++ )
    {
        delay1ms(1);
        uc = HDMITX_ReadI2C_Byte_66121(0x1c) ;
        if( 0 != (0x80 & uc))
        {
            Instance[0].TxEMEMStatus=FALSE;
            break ;
        }
        if( 0 != (0x38 & uc))
        {
            Instance[0].TxEMEMStatus=TRUE;
            break ;
        }
    }
     HDMITX_DEBUG_PRINTF(("timeout = %d, uc = %02X\n",(int)timeout,(int)uc)) ;


    HDMITX_WriteI2C_Byte_66121(0x04,0x3F);     //reset all reg
    HDMITX_WriteI2C_Byte_66121(0xF8,0xFF);//lock register
}

static void InitHDMITX_HDCPROM()
{

    Switch_HDMITX_Bank_66121(0);
    HDMITX_WriteI2C_Byte_66121(0xF8,0xC3);//unlock register
    HDMITX_WriteI2C_Byte_66121(0xF8,0xA5);//unlock register
    if(Instance[0].TxEMEMStatus==TRUE){
            // with internal eMem
            HDMITX_WriteI2C_Byte_66121(REG_TX_ROM_HEADER,0xE0);
            HDMITX_WriteI2C_Byte_66121(REG_TX_LISTCTRL,0x48);
    }else{
        // with external ROM
        HDMITX_WriteI2C_Byte_66121(REG_TX_ROM_HEADER,0xA0);
        HDMITX_WriteI2C_Byte_66121(REG_TX_LISTCTRL,0x00);
    }
    HDMITX_WriteI2C_Byte_66121(0xF8,0xFF);//lock register
}
//////////////////////////////////////////////////////////////////////
// Function: hdmitx_LoadRegSetting()
// Input: RegSetEntry SettingTable[] ;
// Return: N/A
// Remark: if an entry {0, 0, 0} will be terminated.
//////////////////////////////////////////////////////////////////////

void hdmitx_LoadRegSetting(RegSetEntry table[])
{
    int i ;

    for( i = 0 ;  ; i++ )
    {
        if( table[i].offset == 0 && table[i].invAndMask == 0 && table[i].OrMask == 0 )
        {
            return ;
        }
        else if( table[i].invAndMask == 0 && table[i].OrMask == 0 )
        {
            //HDMITX_DEBUG_PRINTF(("delay(%d)\n",(int)table[i].offset));
            delay1ms(table[i].offset);
        }
        else if( table[i].invAndMask == 0xFF )
        {
            //HDMITX_DEBUG_PRINTF(("HDMITX_WriteI2C_Byte_66121(%02x,%02x)\n",(int)table[i].offset,(int)table[i].OrMask));
            HDMITX_WriteI2C_Byte_66121(table[i].offset,table[i].OrMask);
        }
        else
        {
            //HDMITX_DEBUG_PRINTF(("HDMITX_SetI2C_Byte(%02x,%02x,%02x)\n",(int)table[i].offset,(int)table[i].invAndMask,(int)table[i].OrMask));
            HDMITX_SetI2C_Byte(table[i].offset,table[i].invAndMask,table[i].OrMask);
        }
    }
}
void DumpHDMITXReg()
{
    int i,j ;
    BYTE ucData ;

    HDMITX_DEBUG_PRINTF(("       "));
    for(j = 0 ; j < 16 ; j++)
    {
        HDMITX_DEBUG_PRINTF((" %02X",(int)j));
        if((j == 3)||(j==7)||(j==11))
        {
            HDMITX_DEBUG_PRINTF(("  "));
        }
    }
    HDMITX_DEBUG_PRINTF(("\n        -----------------------------------------------------\n"));

    Switch_HDMITX_Bank_66121(0);

    for(i = 0 ; i < 0x100 ; i+=16)
    {
        HDMITX_DEBUG_PRINTF(("[%3X]  ",i));
        for(j = 0 ; j < 16 ; j++)
        {
            if( (i+j)!= 0x17)
            {
                ucData = HDMITX_ReadI2C_Byte_66121((BYTE)((i+j)&0xFF));
                HDMITX_DEBUG_PRINTF((" %02X",(int)ucData));
            }
            else
            {
                HDMITX_DEBUG_PRINTF((" XX",(int)ucData)); // for DDC FIFO
            }
            if((j == 3)||(j==7)||(j==11))
            {
                HDMITX_DEBUG_PRINTF((" -"));
            }
        }
        HDMITX_DEBUG_PRINTF(("\n"));
        if((i % 0x40) == 0x30)
        {
            HDMITX_DEBUG_PRINTF(("        -----------------------------------------------------\n"));
        }
    }
    Switch_HDMITX_Bank_66121(1);
    for(i = 0x130; i < 0x200 ; i+=16)
    {
        HDMITX_DEBUG_PRINTF(("[%3X]  ",i));
        for(j = 0 ; j < 16 ; j++)
        {
            ucData = HDMITX_ReadI2C_Byte_66121((BYTE)((i+j)&0xFF));
            HDMITX_DEBUG_PRINTF((" %02X",(int)ucData));
            if((j == 3)||(j==7)||(j==11))
            {
                HDMITX_DEBUG_PRINTF((" -"));
            }
        }
        HDMITX_DEBUG_PRINTF(("\n"));
        if((i % 0x40) == 0x20)
        {
            HDMITX_DEBUG_PRINTF(("        -----------------------------------------------------\n"));
        }
    }
            HDMITX_DEBUG_PRINTF(("        -----------------------------------------------------\n"));
    Switch_HDMITX_Bank_66121(0);
}
void HDMITX_PowerOn()
{
    hdmitx_LoadRegSetting(HDMITX_PwrOn_Table);
}

void HDMITX_PowerDown()
{
    hdmitx_LoadRegSetting(HDMITX_PwrDown_Table);
}

void InitHDMITX_66121()
{
	hdmitx_LoadRegSetting(HDMITX_Init_Table);
    //HDMITX_WriteI2C_Byte_66121(REG_TX_INT_CTRL,hdmiTxDev[0].bIntType);
    Instance[0].bIntPOL = (Instance[0].bIntType&B_INTPOL_ACTH)?TRUE:FALSE ;

    // Avoid power loading in un play status.
	//////////////////////////////////////////////////////////////////
	// Setup HDCP ROM
	//////////////////////////////////////////////////////////////////
//#ifdef HDMITX_INPUT_INFO
//    Instance[0].RCLK = CalcRCLK();
//#endif
    hdmitx_LoadRegSetting(HDMITX_DefaultVideo_Table);
    hdmitx_LoadRegSetting(HDMITX_SetHDMI_Table);
    hdmitx_LoadRegSetting(HDMITX_DefaultAVIInfo_Table);
    hdmitx_LoadRegSetting(HDMITX_DeaultAudioInfo_Table);
    hdmitx_LoadRegSetting(HDMITX_Aud_CHStatus_LPCM_20bit_48Khz);
    hdmitx_LoadRegSetting(HDMITX_AUD_I2S_2ch_24bit);//(HDMITX_AUD_SPDIF_2ch_24bit);//

    //HDMITX_DEBUG_PRINTF((
    //    "-----------------------------------------------------\n"
    //    "Init HDMITX\n"
    //    "-----------------------------------------------------\n"));
    //DumpHDMITXReg();
}

//////////////////////////////////////////////////////////////////////
// export this for dynamic change input signal
//////////////////////////////////////////////////////////////////////
BOOL SetupVideoInputSignal_66121(BYTE inputSignalType)
{
    Instance[0].bInputVideoSignalType = inputSignalType ;
    // SetInputMode(inputColorMode,Instance[0].bInputVideoSignalType);
    return TRUE ;
}

static void WaitTxVidStable()
{
    BYTE i ;
    for( i = 0 ; i < 20 ; i++ )
    {
        delay1ms(15);
        if((HDMITX_ReadI2C_Byte_66121(REG_TX_SYS_STATUS) & B_TXVIDSTABLE) == 0 )
        {
            continue ;
        }
        delay1ms(15);
        if((HDMITX_ReadI2C_Byte_66121(REG_TX_SYS_STATUS) & B_TXVIDSTABLE) == 0 )
        {
            continue ;
        }
        delay1ms(15);
        if((HDMITX_ReadI2C_Byte_66121(REG_TX_SYS_STATUS) & B_TXVIDSTABLE) == 0 )
        {
            continue ;
        }
        delay1ms(15);
        if((HDMITX_ReadI2C_Byte_66121(REG_TX_SYS_STATUS) & B_TXVIDSTABLE) == 0 )
        {
            continue ;
        }
        break ;
    }
}

BOOL EnableVideoOutput_66121(VIDEOPCLKLEVEL level,BYTE inputColorMode,BYTE outputColorMode,BYTE bHDMI)
{
    // bInputVideoMode,bOutputVideoMode,Instance[0].bInputVideoSignalType,bAudioInputType,should be configured by upper F/W or loaded from EEPROM.
    // should be configured by initsys.c
    // VIDEOPCLKLEVEL level ;

    HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST,B_VID_RST_HDMITX|B_AUD_RST_HDMITX|B_AREF_RST|B_HDCP_RST_HDMITX);

    Instance[0].bHDMIMode = (BYTE)bHDMI ;
    // 2009/12/09 added by jau-chih.tseng@ite.com.tw
    Switch_HDMITX_Bank_66121(1);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AVIINFO_DB1,0x00);
    Switch_HDMITX_Bank_66121(0);
    //~jau-chih.tseng@ite.com.tw

    if(Instance[0].bHDMIMode)
    {
        SetAVMute_66121(TRUE);
    }

    SetInputMode(inputColorMode,Instance[0].bInputVideoSignalType);

    SetCSCScale(inputColorMode,outputColorMode);

    if(Instance[0].bHDMIMode)
    {
        HDMITX_WriteI2C_Byte_66121(REG_TX_HDMI_MODE,B_TX_HDMI_MODE);
    }
    else
    {
        HDMITX_WriteI2C_Byte_66121(REG_TX_HDMI_MODE,B_TX_DVI_MODE);
    }

#ifdef INVERT_VID_LATCHEDGE
    uc = HDMITX_ReadI2C_Byte_66121(REG_TX_CLK_CTRL1);
    uc |= B_VDO_LATCH_EDGE ;
    HDMITX_WriteI2C_Byte_66121(REG_TX_CLK_CTRL1, uc);
#endif

//    HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST,          B_AUD_RST_HDMITX|B_AREF_RST|B_HDCP_RST_HDMITX);

    SetupAFE(level); // pass if High Freq request
    HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST, B_AUD_RST_HDMITX|B_AREF_RST|B_HDCP_RST_HDMITX);
    // Clive suggestion.
    // clear int3 video stable interrupt.
    FireAFE();
	return TRUE ;
}
#if 0
BOOL EnableAudioOutput(ULONG VideoPixelClock,BYTE bAudioSampleFreq,BYTE ChannelNumber,BYTE bAudSWL,BYTE bSPDIF)
{
    BYTE bAudioChannelEnable ;
//    unsigned long N ;

    Instance[0].TMDSClock = VideoPixelClock ;
    Instance[0].bAudFs = bAudioSampleFreq ;

    HDMITX_DEBUG_PRINTF(("EnableAudioOutput(0,%ld",VideoPixelClock));
    HDMITX_DEBUG_PRINTF((",%x",(int)bAudioSampleFreq));
    HDMITX_DEBUG_PRINTF((",%d",(int)ChannelNumber));
    HDMITX_DEBUG_PRINTF((",%d",(int)bAudSWL));
    HDMITX_DEBUG_PRINTF((",%d);\n",(int)bSPDIF));

    switch(ChannelNumber)
    {
    case 7:
    case 8:
        bAudioChannelEnable = 0xF ;
        break ;
    case 6:
    case 5:
        bAudioChannelEnable = 0x7 ;
        break ;
    case 4:
    case 3:
        bAudioChannelEnable = 0x3 ;
        break ;
    case 2:
    case 1:
    default:
        bAudioChannelEnable = 0x1 ;
        break ;
    }

    if(bSPDIF) bAudioChannelEnable |= B_AUD_SPDIF ;

    if( bSPDIF )
    {
        Switch_HDMITX_Bank_66121(1);
        HDMITX_WriteI2C_Byte_66121(REGPktAudCTS0,0x50);
        HDMITX_WriteI2C_Byte_66121(REGPktAudCTS1,0x73);
        HDMITX_WriteI2C_Byte_66121(REGPktAudCTS2,0x00);

        HDMITX_WriteI2C_Byte_66121(REGPktAudN0,0);
        HDMITX_WriteI2C_Byte_66121(REGPktAudN1,0x18);
        HDMITX_WriteI2C_Byte_66121(REGPktAudN2,0);
        Switch_HDMITX_Bank_66121(0);

        HDMITX_WriteI2C_Byte_66121(0xC5, 2); // D[1] = 0, HW auto count CTS
    }
    else
    {
        SetNCTS(VideoPixelClock,bAudioSampleFreq);
    }

    /*
    if(VideoPixelClock != 0)
    {
        SetNCTS(VideoPixelClock,bAudioSampleFreq);
    }
    else
    {
        switch(bAudioSampleFreq)
        {
        case AUDFS_32KHz: N = 4096; break;
        case AUDFS_44p1KHz: N = 6272; break;
        case AUDFS_48KHz: N = 6144; break;
        case AUDFS_88p2KHz: N = 12544; break;
        case AUDFS_96KHz: N = 12288; break;
        case AUDFS_176p4KHz: N = 25088; break;
        case AUDFS_192KHz: N = 24576; break;
        default: N = 6144;
        }
        Switch_HDMITX_Bank_66121(1);
        HDMITX_WriteI2C_Byte_66121(REGPktAudN0,(BYTE)((N)&0xFF));
        HDMITX_WriteI2C_Byte_66121(REGPktAudN1,(BYTE)((N>>8)&0xFF));
        HDMITX_WriteI2C_Byte_66121(REGPktAudN2,(BYTE)((N>>16)&0xF));
        Switch_HDMITX_Bank_66121(0);
        HDMITX_WriteI2C_Byte_66121(REG_TX_PKT_SINGLE_CTRL,0); // D[1] = 0,HW auto count CTS
    }
    */

    //HDMITX_AndREG_Byte_66121(REG_TX_SW_RST,~(B_AUD_RST_HDMITX|B_AREF_RST));
    SetAudioFormat(ChannelNumber,bAudioChannelEnable,bAudioSampleFreq,bAudSWL,bSPDIF);

    #ifdef DEBUG
    DumpCatHDMITXReg_66121();
    #endif // DEBUG
    return TRUE ;
}
#endif

BOOL
GetEDIDData_66121(int EDIDBlockID,BYTE *pEDIDData)
{
    if(!pEDIDData)
    {
        return FALSE ;
    }

    if(ReadEDID_66121(pEDIDData,EDIDBlockID/2,(EDIDBlockID%2)*128,128) == ER_FAIL)
    {
        return FALSE ;
    }

    return TRUE ;
}


BOOL
EnableHDCP_66121(BYTE bEnable)
{

#ifdef SUPPORT_HDCP
    if(bEnable)
    {
        if(ER_FAIL == HDCP_Authenticate())
        {
            printf("hdmitx_drv.c(%d), 66121 autenticate is failed\n", __LINE__);

            HDCP_ResetAuth();
            return FALSE ;
        }
        else
        {
            printf("hdmitx_drv.c(%d), 66121 autenticate is success\n", __LINE__);
        }
    }
    else
    {
        HDCP_ResetAuth();
    }
#endif
    return TRUE ;
}

BOOL getHDMITX_LinkStatus()
{
	if(B_RXSENDETECT & HDMITX_ReadI2C_Byte_66121(REG_TX_SYS_STATUS))
    {
        if(0==HDMITX_ReadI2C_Byte_66121(REG_TX_AFE_DRV_CTRL))
        {
            //HDMITX_DEBUG_PRINTF(("getHDMITX_LinkStatus()!!\n") );
            return TRUE;
        }
    }
    HDMITX_DEBUG_PRINTF(("GetTMDS not Ready()!!\n") );

    return FALSE;
}

BYTE
CheckHDMITX_66121(BYTE *pHPD,BYTE *pHPDChange)
{
    BYTE intdata1,intdata2,intdata3,sysstat;
    BYTE  intclr3 = 0 ;
    BYTE PrevHPD = Instance[0].bHPD ;
    BYTE HPD ;
	

    sysstat = HDMITX_ReadI2C_Byte_66121(REG_TX_SYS_STATUS);
    // HDMITX_DEBUG_PRINTF(("REG_TX_SYS_STATUS = %X \n",sysstat));

    if((sysstat & (B_HPDETECT/*|B_RXSENDETECT*/)) == (B_HPDETECT/*|B_RXSENDETECT*/))
    {
        HPD = TRUE;
    }
    else
    {
        HPD = FALSE;		
    }
    // 2007/06/20 added by jj_tseng@chipadvanced.com

    if(pHPDChange)
    {
        *pHPDChange = (HPD!=PrevHPD)?TRUE:FALSE ; // default give pHPDChange value compared to previous HPD value.

    }
    //~jj_tseng@chipadvanced.com 2007/06/20

    if(HPD==FALSE)
    {
        Instance[0].bAuthenticated = FALSE ;
    }

    if(sysstat & B_INT_ACTIVE)
    {
        HDMITX_DEBUG_PRINTF(("REG_TX_SYS_STATUS = 0x%02X \n",(int)sysstat));

        intdata1 = HDMITX_ReadI2C_Byte_66121(REG_TX_INT_STAT1);
        HDMITX_DEBUG_PRINTF(("INT_Handler: reg%X = %X\n",(int)REG_TX_INT_STAT1,(int)intdata1));
        if(intdata1 & B_INT_AUD_OVERFLOW)
        {
#ifdef Debug_message
            HDMITX_DEBUG_PRINTF(("B_INT_AUD_OVERFLOW.\n"));
#endif
            HDMITX_OrREG_Byte_66121(REG_TX_SW_RST,(B_AUD_RST_HDMITX|B_AREF_RST));
            HDMITX_AndREG_Byte_66121(REG_TX_SW_RST,~(B_AUD_RST_HDMITX|B_AREF_RST));
            //AudioDelayCnt=AudioOutDelayCnt;
            //LastRefaudfreqnum=0;
        }
        if(intdata1 & B_INT_DDCFIFO_ERR)
        {
            HDMITX_DEBUG_PRINTF(("DDC FIFO Error.\n"));
            ClearDDCFIFO();
			Instance[0].bAuthenticated= FALSE ;
        }


        if(intdata1 & B_INT_DDC_BUS_HANG)
        {
            HDMITX_DEBUG_PRINTF(("DDC BUS HANG.\n"));
            AbortDDC();

            if(Instance[0].bAuthenticated)
            {
                HDMITX_DEBUG_PRINTF(("when DDC hang,and aborted DDC,the HDCP authentication need to restart.\n"));
                #ifdef SUPPORT_HDCP
                HDCP_ResumeAuthentication();
                #endif
            }
        }


        if(intdata1 & (B_INT_HPD_PLUG/*|B_INT_RX_SENSE*/))
        {

            if(pHPDChange)
            {
                *pHPDChange = TRUE ;
            }

            if(HPD == FALSE)
            {
                //HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST,B_AREF_RST|B_VID_RST_HDMITX|B_AUD_RST_HDMITX|B_HDCP_RST_HDMITX);
                //delay1ms(1);
                //HDMITX_WriteI2C_Byte_66121(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST|B_AFE_DRV_PWD);
                //HDMITX_DEBUG_PRINTF(("Unplug,%x %x\n",(int)HDMITX_ReadI2C_Byte_66121(REG_TX_SW_RST),(int)HDMITX_ReadI2C_Byte_66121(REG_TX_AFE_DRV_CTRL)));
            }
        }
		if(intdata1 & (B_INT_RX_SENSE))
		{
            Instance[0].bAuthenticated = FALSE;
		}

        intdata2 = HDMITX_ReadI2C_Byte_66121(REG_TX_INT_STAT2);
        HDMITX_DEBUG_PRINTF(("INT_Handler: reg%X = %X\n",(int)REG_TX_INT_STAT2,(int)intdata2));



        #ifdef SUPPORT_HDCP
        if(intdata2 & B_INT_AUTH_DONE)
        {
            HDMITX_DEBUG_PRINTF(("interrupt Authenticate Done.\n"));
            HDMITX_OrREG_Byte_66121(REG_TX_INT_MASK2,(BYTE)B_T_AUTH_DONE_MASK);
            //Instance[0].bAuthenticated = TRUE ;
            //SetAVMute_66121(FALSE);
        }

        if(intdata2 & B_INT_AUTH_FAIL)
        {
            HDMITX_DEBUG_PRINTF(("interrupt Authenticate Fail.\n"));
            AbortDDC();   // @emily add
            #ifdef SUPPORT_HDCP
            HDCP_ResumeAuthentication();
            #endif
        }
        #endif // SUPPORT_HDCP
		/*
        intdata3 = HDMITX_ReadI2C_Byte_66121(REG_TX_INT_STAT3);
        if(intdata3 & B_INT_VIDSTABLE)
        {
            sysstat = HDMITX_ReadI2C_Byte_66121(REG_TX_SYS_STATUS);
            if(sysstat & B_TXVIDSTABLE)
            {
                FireAFE();
            }
        }
		*/
		intdata3= HDMITX_ReadI2C_Byte_66121(0xEE);
        if( intdata3 )
        {
            HDMITX_WriteI2C_Byte_66121(0xEE,intdata3); // clear ext interrupt ;
            HDMITX_DEBUG_PRINTF(("%s%s%s%s%s%s%s\n",
                (intdata3&0x40)?"video parameter change ":"",
                (intdata3&0x20)?"HDCP Pj check done ":"",
                (intdata3&0x10)?"HDCP Ri check done ":"",
                (intdata3&0x8)? "DDC bus hang ":"",
                (intdata3&0x4)? "Video input FIFO auto reset ":"",
                (intdata3&0x2)? "No audio input interrupt  ":"",
                (intdata3&0x1)? "Audio decode error interrupt ":""));
        }
        HDMITX_WriteI2C_Byte_66121(REG_TX_INT_CLR0,0xFF);
        HDMITX_WriteI2C_Byte_66121(REG_TX_INT_CLR1,0xFF);
        intclr3 = (HDMITX_ReadI2C_Byte_66121(REG_TX_SYS_STATUS))|B_CLR_AUD_CTS | B_INTACTDONE ;
        HDMITX_WriteI2C_Byte_66121(REG_TX_SYS_STATUS,intclr3); // clear interrupt.
        intclr3 &= ~(B_INTACTDONE);
        HDMITX_WriteI2C_Byte_66121(REG_TX_SYS_STATUS,intclr3); // INTACTDONE reset to zero.
    }
    else
    {
        if(pHPDChange)
        {
            if(HPD != PrevHPD)
            {
                *pHPDChange = TRUE;
            }
            else
            {
               *pHPDChange = FALSE;
            }
        }
    }

    if(pHPDChange)
    {
        if((*pHPDChange==TRUE) &&(HPD==FALSE))
        {
            HDMITX_WriteI2C_Byte_66121(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST|B_AFE_DRV_PWD);
        }
    }

    //SetupAudioChannel(); // 2007/12/12 added by jj_tseng

    if(pHPD)
    {
         *pHPD = HPD    ;
    }

    Instance[0].bHPD = HPD ;
    return HPD ;
}




void
DisableHDMITX_66121()
{
    HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST,B_AREF_RST|B_VID_RST_HDMITX|B_AUD_RST_HDMITX|B_HDCP_RST_HDMITX);
    delay1ms(1);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST|B_AFE_DRV_PWD);
}

void
DisableVideoOutput_66121()
{
    BYTE uc = HDMITX_ReadI2C_Byte_66121(REG_TX_SW_RST) | B_VID_RST_HDMITX ;
    HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST,uc);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST|B_AFE_DRV_PWD);
}


void
DisableAudioOutput_66121()
{
    //BYTE uc = (HDMITX_ReadI2C_Byte_66121(REG_TX_SW_RST) | (B_AUD_RST_HDMITX | B_AREF_RST)) ;
    //HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST,uc);
    HDMITX_OrREG_Byte_66121(REG_TX_SW_RST,(B_AUD_RST_HDMITX | B_AREF_RST));
}



BOOL
EnableAVIInfoFrame_66121(BYTE bEnable,BYTE *pAVIInfoFrame)
{
    if(!bEnable)
    {
        DISABLE_AVI_INFOFRM_PKT();
        return TRUE ;
    }

    if(SetAVIInfoFrame((AVI_InfoFrame *)pAVIInfoFrame) == ER_SUCCESS)
    {
        return TRUE ;
    }

    return FALSE ;
}

BOOL
EnableAudioInfoFrame_66121(BYTE bEnable,BYTE *pAudioInfoFrame)
{
    if(!bEnable)
    {
        DISABLE_AVI_INFOFRM_PKT();
        return TRUE ;
    }


    if(SetAudioInfoFrame((Audio_InfoFrame *)pAudioInfoFrame) == ER_SUCCESS)
    {
        return TRUE ;
    }

    return FALSE ;
}

void
SetAVMute_66121(BYTE bEnable)
{
    BYTE uc ;

    Switch_HDMITX_Bank_66121(0);
    uc = HDMITX_ReadI2C_Byte_66121(REG_TX_GCP);
    uc &= ~B_TX_SETAVMUTE ;
    uc |= bEnable?B_TX_SETAVMUTE:0 ;
    HDMITX_WriteI2C_Byte_66121(REG_TX_GCP,uc);
    HDMITX_WriteI2C_Byte_66121(REG_TX_PKT_GENERAL_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}

void
SetOutputColorDepthPhase_66121(BYTE ColorDepth,BYTE bPhase)
{
    BYTE uc ;
    BYTE bColorDepth ;

    if(ColorDepth == 30)
    {
        bColorDepth = B_CD_30 ;
    }
    else if (ColorDepth == 36)
    {
        bColorDepth = B_CD_36 ;
    }
    else if (ColorDepth == 24)
    {
        bColorDepth = B_CD_24 ;
        //bColorDepth = 0 ;//modify JJ by mail 20100423 1800 // not indicated
    }
    else
    {
        bColorDepth = 0 ; // not indicated
    }

    Switch_HDMITX_Bank_66121(0);
    uc = HDMITX_ReadI2C_Byte_66121(REG_TX_GCP);
    uc &= ~B_COLOR_DEPTH_MASK ;
    uc |= bColorDepth&B_COLOR_DEPTH_MASK;
    HDMITX_WriteI2C_Byte_66121(REG_TX_GCP,uc);
}

void
Get66121Reg(BYTE *pReg)
{
    int i ;
    BYTE reg ;
    Switch_HDMITX_Bank_66121(0);
    for(i = 0 ; i < 0x100 ; i++)
    {
        reg = i & 0xFF ;
        pReg[i] = HDMITX_ReadI2C_Byte_66121(reg);
    }
    Switch_HDMITX_Bank_66121(1);
    for(reg = 0x30 ; reg < 0xB0 ; i++,reg++)
    {
        pReg[i] = HDMITX_ReadI2C_Byte_66121(reg);
    }
    Switch_HDMITX_Bank_66121(0);

}
//////////////////////////////////////////////////////////////////////
// SubProcedure process                                                       //
//////////////////////////////////////////////////////////////////////
#ifdef SUPPORT_DEGEN

typedef struct {
    MODE_ID id ;
    BYTE Reg90;
    BYTE Reg92;
    BYTE Reg93;
    BYTE Reg94;
    BYTE Reg9A;
    BYTE Reg9B;
    BYTE Reg9C;
    BYTE Reg9D;
    BYTE Reg9E;
    BYTE Reg9F;
} DEGEN_Setting ;

typedef struct {
    MODE_ID id;
    BYTE Reg90;
    BYTE Reg91;
    BYTE Reg92;
    BYTE Reg93;
    BYTE Reg94;
    BYTE Reg95;
    BYTE Reg96;
    BYTE Reg97;
    BYTE Reg9A;
    BYTE Reg9B;
    BYTE Reg9C;
    BYTE Reg9D;
    BYTE Reg9E;
    BYTE Reg9F;
    BYTE RegA0;
    BYTE RegA1;
    BYTE RegA2;
    BYTE RegA3;			    
} SYNCGEN_DEGEN_Setting;

static _CODE DEGEN_Setting DeGen_Table[] = {
    {CEA_640x480p60      ,0x01,0x8E,0x0E,0x30,0x22,0x02,0x20,0xFF,0xFF,0xFF},
    // HDES = 142, HDEE = 782, VDES = 34, VDEE = 514
    {CEA_720x480p60      ,0x01,0x78,0x48,0x30,0x23,0x03,0x20,0xFF,0xFF,0xFF},
    // HDES = 120, HDEE = 840, VDES = 35, VDEE = 515
    {CEA_1280x720p60     ,0x07,0x02,0x02,0x61,0x18,0xE8,0x20,0xFF,0xFF,0xFF},
    // HDES = 258, HDEE = 1538, VDES = 24, VDEE = 744
//    {CEA_1920x1080i60    ,0x07,0xBE,0x3E,0x80,0x13,0x2F,0x20,0x45,0x61,0x42},
//    // HDES = 190, HDEE = 2110, VDES = 19, VDEE = 559, VDS2 = 581, VDE2 = 1121
    {CEA_1920x1080i60    ,0x07,0xBE,0x3E,0x80,0x13,0x2F,0x20,0x46,0x62,0x42},
    // HDES = 190, HDEE = 2110, VDES = 19, VDEE = 559, VDS2 = 582, VDE2 = 1122
    {CEA_720x480i60      ,0x01,0x75,0x45,0x30,0x11,0x01,0x10,0x18,0x08,0x21},
    // HDES = 117, HDEE = 837, VDES = 17, VDEE = 257, VDS2 = 250, VDE2 = 520    
    {CEA_720x240p60      ,0x01,0x75,0x45,0x30,0x11,0x01,0x10,0xFF,0xFF,0xFF},
    // HDES = 117, HDEE = 837, VDES = 17, VDEE = 257
    {CEA_1440x480i60     ,0x01,0xEC,0x8C,0x60,0x11,0x01,0x10,0x17,0x07,0x21},
    // HDES = 236, HDEE = 1676, VDES = 17, VDEE = 257, VDS2 = 279, VDE2 = 519
    {CEA_1440x240p60     ,0x01,0xEC,0x8C,0x60,0x11,0x01,0x10,0xFF,0xFF,0xFF},
    // HDES = 236, HDEE = 1676, VDES = 17, VDEE = 257
    {CEA_2880x480i60     ,0x01,0x16,0x56,0xD2,0x11,0x01,0x10,0x17,0x07,0x21},
    // HDES = 534, HDEE = 3414, VDES = 17, VDEE = 257, VDS2 = 279, VDE2 = 519
    {CEA_2880x240p60     ,0x01,0x16,0x56,0xD2,0x11,0x01,0x10,0xFF,0xFF,0xFF},
    // HDES = 534, HDEE = 3414, VDES = 17, VDEE = 257
    {CEA_1440x480p60     ,0x01,0xF2,0x92,0x60,0x23,0x03,0x20,0xFF,0xFF,0xFF},
    // HDES = 242, HDEE = 1682, VDES = 35, VDEE = 515
    {CEA_1920x1080p60    ,0x07,0xBE,0x3E,0x80,0x28,0x60,0x40,0xFF,0xFF,0xFF},
    // HDES = 190, HDEE = 2110, VDES = 40, VDEE = 1120
    {CEA_720x576p50      ,0x01,0x82,0x52,0x30,0x2b,0x6b,0x20,0xFF,0xFF,0xFF},
    // HDES = 130, HDEE = 850, VDES = 43, VDEE = 619
    {CEA_1280x720p50     ,0x07,0x02,0x02,0x61,0x18,0xE8,0x20,0xFF,0xFF,0xFF},
    // HDES = 258, HDEE = 1538, VDES = 24, VDEE = 744
    {CEA_1920x1080i50    ,0x07,0xBE,0x3E,0x80,0x13,0x2F,0x20,0x46,0x62,0x42},
    // HDES = 190, HDEE = 2110, VDES = 19, VDEE = 559, VDS2 = 582, VDE2 = 1122
    {CEA_720x576i50      ,0x01,0x82,0x52,0x30,0x15,0x35,0x10,0x4E,0x6E,0x21},
    // HDES = 130, HDEE = 850, VDES = 21, VDEE = 309, VDS2 = 334, VDE2 = 622
    {CEA_1440x576i50     ,0x01,0x06,0xA6,0x61,0x15,0x35,0x10,0x4D,0x6D,0x21},
    // HDES = 262, HDEE = 1702, VDES = 21, VDEE = 309, VDS2 = 333, VDE2 = 621
    {CEA_720x288p50      ,0x01,0x82,0x52,0x30,0x15,0x35,0x10,0xFF,0xFF,0xFF},
    // HDES = 130, HDEE = 850, VDES = 21, VDEE = 309
    {CEA_1440x288p50     ,0x01,0x06,0xA6,0x61,0x15,0x35,0x10,0xFF,0xFF,0xFF},
    // HDES = 262, HDEE = 1702, VDES = 21, VDEE = 309
    {CEA_2880x576i50     ,0x01,0x0E,0x4E,0xD2,0x15,0x35,0x10,0x4D,0x6D,0x21},
    // HDES = 526, HDEE = 3406, VDES = 21, VDEE = 309, VDS2 = 333, VDE2 = 621
    {CEA_2880x288p50     ,0x01,0x0E,0x4E,0xD2,0x15,0x35,0x10,0xFF,0xFF,0xFF},
    // HDES = 526, HDEE = 3406, VDES = 21, VDEE = 309
    {CEA_1440x576p50     ,0x05,0x06,0xA6,0x61,0x2B,0x6B,0x20,0xFF,0xFF,0xFF},
    // HDES = 262, HDEE = 1702, VDES = 43, VDEE = 619
    {CEA_1920x1080p50    ,0x07,0xBE,0x3E,0x80,0x28,0x60,0x40,0xFF,0xFF,0xFF},
    // HDES = 190, HDEE = 2110, VDES = 40, VDEE = 1120
    {CEA_1920x1080p24    ,0x07,0xBE,0x3E,0x80,0x28,0x60,0x40,0xFF,0xFF,0xFF},
    // HDES = 190, HDEE = 2110, VDES = 40, VDEE = 1120
    {CEA_1920x1080p25    ,0x07,0xBE,0x3E,0x80,0x28,0x60,0x40,0xFF,0xFF,0xFF},
    // HDES = 190, HDEE = 2110, VDES = 40, VDEE = 1120
    {CEA_1920x1080p30    ,0x07,0xBE,0x3E,0x80,0x28,0x60,0x40,0xFF,0xFF,0xFF},
    // HDES = 190, HDEE = 2110, VDES = 40, VDEE = 1120
    {VESA_640x350p85     ,0x03,0x9E,0x1E,0x30,0x3E,0x9C,0x10,0xFF,0xFF,0xFF},
    // HDES = 158, HDEE = 798, VDES = 62, VDEE = 412
    {VESA_640x400p85     ,0x05,0x9E,0x1E,0x30,0x2B,0xBB,0x10,0xFF,0xFF,0xFF},
    // HDES = 158, HDEE = 798, VDES = 43, VDEE = 443
    {VESA_720x400p85     ,0x05,0xB2,0x82,0x30,0x2C,0xBC,0x10,0xFF,0xFF,0xFF},
    // HDES = 178, HDEE = 898, VDES = 44, VDEE = 444
    {VESA_640x480p60     ,0x01,0x8E,0x0E,0x30,0x22,0x02,0x20,0xFF,0xFF,0xFF},
    // HDES = 142, HDEE = 782, VDES = 34, VDEE = 514
    {VESA_640x480p72     ,0x01,0xA6,0x26,0x30,0x1E,0xFE,0x10,0xFF,0xFF,0xFF},
    // HDES = 166, HDEE = 806, VDES = 30, VDEE = 510
    {VESA_640x480p75     ,0x01,0xB6,0x36,0x30,0x12,0xF2,0x10,0xFF,0xFF,0xFF},
    // HDES = 182, HDEE = 822, VDES = 18, VDEE = 498
    {VESA_640x480p85     ,0x01,0x86,0x06,0x30,0x1B,0xFB,0x10,0xFF,0xFF,0xFF},
    // HDES = 134, HDEE = 774, VDES = 27, VDEE = 507
    {VESA_800x600p56     ,0x07,0xC6,0xE6,0x30,0x17,0x6F,0x20,0xFF,0xFF,0xFF},
    // HDES = 198, HDEE = 998, VDES = 23, VDEE = 623
    {VESA_800x600p60     ,0x07,0xD6,0xF6,0x30,0x1A,0x72,0x20,0xFF,0xFF,0xFF},
    // HDES = 214, HDEE = 1014, VDES = 26, VDEE = 626
    {VESA_800x600p72     ,0x07,0xB6,0xD6,0x30,0x1C,0x74,0x20,0xFF,0xFF,0xFF},
    // HDES = 182, HDEE = 982, VDES = 28, VDEE = 628
    {VESA_800x600p75     ,0x07,0xEE,0x0E,0x40,0x17,0x6F,0x20,0xFF,0xFF,0xFF},
    // HDES = 238, HDEE = 1038, VDES = 23, VDEE = 623
    {VESA_800X600p85     ,0x07,0xD6,0xF6,0x30,0x1D,0x75,0x20,0xFF,0xFF,0xFF},
    // HDES = 214, HDEE = 1014, VDES = 29, VDEE = 629
    {VESA_840X480p60     ,0x07,0xDE,0x2E,0x40,0x1E,0xFE,0x10,0xFF,0xFF,0xFF},
    // HDES = 222, HDEE = 1070, VDES = 30, VDEE = 510
    {VESA_1024x768p60    ,0x01,0x26,0x26,0x51,0x22,0x22,0x30,0xFF,0xFF,0xFF},
    // HDES = 294, HDEE = 1318, VDES = 34, VDEE = 802
    {VESA_1024x768p70    ,0x01,0x16,0x16,0x51,0x22,0x22,0x30,0xFF,0xFF,0xFF},
    // HDES = 278, HDEE = 1302, VDES = 34, VDEE = 802
    {VESA_1024x768p75    ,0x07,0x0E,0x0E,0x51,0x1E,0x1E,0x30,0xFF,0xFF,0xFF},
    // HDES = 270, HDEE = 1294, VDES = 30, VDEE = 798
    {VESA_1024x768p85    ,0x07,0x2E,0x2E,0x51,0x26,0x26,0x30,0xFF,0xFF,0xFF},
    // HDES = 302, HDEE = 1326, VDES = 38, VDEE = 806
    {VESA_1152x864p75    ,0x07,0x7E,0xFE,0x51,0x22,0x82,0x30,0xFF,0xFF,0xFF},
    // HDES = 382, HDEE = 1534, VDES = 34, VDEE = 898
    {VESA_1280x768p60R   ,0x03,0x6E,0x6E,0x50,0x12,0x12,0x30,0xFF,0xFF,0xFF},
    // HDES = 110, HDEE = 1390, VDES = 18, VDEE = 786
    {VESA_1280x768p60    ,0x05,0x3E,0x3E,0x61,0x1A,0x1A,0x30,0xFF,0xFF,0xFF},
    // HDES = 318, HDEE = 1598, VDES = 26, VDEE = 794
    {VESA_1280x768p75    ,0x05,0x4E,0x4E,0x61,0x21,0x21,0x30,0xFF,0xFF,0xFF},
    // HDES = 334, HDEE = 1614, VDES = 33, VDEE = 801
    {VESA_1280x768p85    ,0x05,0x5E,0x5E,0x61,0x25,0x25,0x30,0xFF,0xFF,0xFF},
    // HDES = 350, HDEE = 1630, VDES = 37, VDEE = 805    
    {VESA_1280x800p60    ,0x05,0x46,0x46,0x61,0x1D,0x3d,0x30,0xFF,0xFF,0xFF},
    // HDES = 326, HDEE = 1606, VDES = 29, VDEE = 829   
    {VESA_1280x960p60    ,0x07,0xA6,0xA6,0x61,0x26,0xE6,0x30,0xFF,0xFF,0xFF},
    // HDES = 422, HDEE = 1702, VDES = 38, VDEE = 998
    {VESA_1280x960p85    ,0x07,0x7E,0x7E,0x61,0x31,0xF1,0x30,0xFF,0xFF,0xFF},
    // HDES = 382, HDEE = 1662, VDES = 49, VDEE = 1009
    {VESA_1280x1024p60   ,0x07,0x66,0x66,0x61,0x28,0x28,0x40,0xFF,0xFF,0xFF},
    // HDES = 358, HDEE = 1638, VDES = 40, VDEE = 1064
    {VESA_1280x1024p75   ,0x07,0x86,0x86,0x61,0x28,0x28,0x40,0xFF,0xFF,0xFF},
    // HDES = 390, HDEE = 1670, VDES = 40, VDEE = 1064
    {VESA_1280X1024p85   ,0x07,0x7E,0x7E,0x61,0x2E,0x2E,0x40,0xFF,0xFF,0xFF},
    // HDES = 382, HDEE = 1662, VDES = 46, VDEE = 1070
    {VESA_1360X768p60    ,0x07,0x6E,0xBE,0x61,0x17,0x17,0x30,0xFF,0xFF,0xFF},
    // HDES = 366, HDEE = 1726, VDES = 23, VDEE = 791
    {VESA_1400x768p60R   ,0x03,0x6E,0xE6,0x50,0x1A,0x34,0x40,0xFF,0xFF,0xFF},
    // HDES = 110, HDEE = 1510, VDES = 26, VDEE = 1076
    {VESA_1400x768p60    ,0x05,0x76,0xEE,0x61,0x23,0x3D,0x40,0xFF,0xFF,0xFF},
    // HDES = 374, HDEE = 1774, VDES = 35, VDEE = 1085
    {VESA_1400x1050p75   ,0x05,0x86,0xFE,0x61,0x2D,0x47,0x40,0xFF,0xFF,0xFF},
    // HDES = 390, HDEE = 1790, VDES = 45, VDEE = 1095
    {VESA_1400x1050p85   ,0x05,0x96,0x0E,0x71,0x33,0x4D,0x40,0xFF,0xFF,0xFF},
    // HDES = 406, HDEE = 1806, VDES = 51, VDEE = 1101
    {VESA_1440x900p60R   ,0x03,0x6E,0x0E,0x60,0x16,0x9A,0x30,0xFF,0xFF,0xFF},
    // HDES = 110, HDEE = 1550, VDES = 22, VDEE = 922
    {VESA_1440x900p60    ,0x05,0x7E,0x1E,0x71,0x1E,0xA2,0x30,0xFF,0xFF,0xFF},
    // HDES = 382, HDEE = 1822, VDES = 30, VDEE = 930
    {VESA_1440x900p75    ,0x05,0x8E,0x2E,0x71,0x26,0xAA,0x30,0xFF,0xFF,0xFF},
    // HDES = 398, HDEE = 1838, VDES = 38, VDEE = 938
    {VESA_1440x900p85    ,0x05,0x96,0x36,0x71,0x2C,0xB0,0x30,0xFF,0xFF,0xFF},
    // HDES = 406, HDEE = 1846, VDES = 44, VDEE = 944
    {VESA_1600x900p60   ,0x07,0x6E,0xAE,0x60,0x16,0x9A,0x30,0xFF,0xFF,0xFF},
    // HDES = 110, HDEE = 1710, VDES = 22, VDEE = 922    
    {VESA_1600x1200p60   ,0x07,0xEE,0x2E,0x81,0x30,0xE0,0x40,0xFF,0xFF,0xFF},
    // HDES = 494, HDEE = 2094, VDES = 48, VDEE = 1248
    {VESA_1600x1200p65   ,0x07,0xEE,0x2E,0x81,0x30,0xE0,0x40,0xFF,0xFF,0xFF},
    // HDES = 494, HDEE = 2094, VDES = 48, VDEE = 1248
    {VESA_1600x1200p70   ,0x07,0xEE,0x2E,0x81,0x30,0xE0,0x40,0xFF,0xFF,0xFF},
    // HDES = 494, HDEE = 2094, VDES = 48, VDEE = 1248
    {VESA_1600x1200p75   ,0x07,0xEE,0x2E,0x81,0x30,0xE0,0x40,0xFF,0xFF,0xFF},
    // HDES = 494, HDEE = 2094, VDES = 48, VDEE = 1248
    {VESA_1600x1200p85   ,0x07,0xEE,0x2E,0x81,0x30,0xE0,0x40,0xFF,0xFF,0xFF},
    // HDES = 494, HDEE = 2094, VDES = 48, VDEE = 1248
    {VESA_1680x1050p60R  ,0x03,0x6E,0xFE,0x60,0x1A,0x34,0x40,0xFF,0xFF,0xFF},
    // HDES = 110, HDEE = 1790, VDES = 26, VDEE = 1076
    {VESA_1680x1050p60   ,0x05,0xC6,0x56,0x81,0x23,0x3D,0x40,0xFF,0xFF,0xFF},
    // HDES = 454, HDEE = 2134, VDES = 35, VDEE = 1085
    {VESA_1680x1050p75   ,0x05,0xD6,0x66,0x81,0x2D,0x47,0x40,0xFF,0xFF,0xFF},
    // HDES = 470, HDEE = 2150, VDES = 45, VDEE = 1095
    {VESA_1680x1050p85   ,0x05,0xDE,0x6E,0x81,0x33,0x4D,0x40,0xFF,0xFF,0xFF},
    // HDES = 478, HDEE = 2158, VDES = 51, VDEE = 1101
    {VESA_1792x1344p60   ,0x05,0x0E,0x0E,0x92,0x30,0x70,0x50,0xFF,0xFF,0xFF},
    // HDES = 526, HDEE = 2318, VDES = 48, VDEE = 1392
    {VESA_1792x1344p75   ,0x05,0x36,0x36,0x92,0x47,0x87,0x50,0xFF,0xFF,0xFF},
    // HDES = 566, HDEE = 2358, VDES = 71, VDEE = 1415
    {VESA_1856x1392p60   ,0x05,0x3E,0x7E,0x92,0x2D,0x9D,0x50,0xFF,0xFF,0xFF},
    // HDES = 574, HDEE = 2430, VDES = 45, VDEE = 1437
    {VESA_1856x1392p75   ,0x05,0x3E,0x7E,0x92,0x6A,0xDA,0x50,0xFF,0xFF,0xFF},
    // HDES = 574, HDEE = 2430, VDES = 106, VDEE = 1498
    {VESA_1920x1200p60R  ,0x03,0x6E,0xEE,0x70,0x1F,0xCF,0x40,0xFF,0xFF,0xFF},
    // HDES = 110, HDEE = 2030, VDES = 31, VDEE = 1231
    {VESA_1920x1200p60   ,0x05,0x16,0x96,0x92,0x29,0xD9,0x40,0xFF,0xFF,0xFF},
    // HDES = 534, HDEE = 2454, VDES = 41, VDEE = 1241
    {VESA_1920x1200p75   ,0x05,0x26,0xA6,0x92,0x33,0xE3,0x40,0xFF,0xFF,0xFF},
    // HDES = 550, HDEE = 2470, VDES = 51, VDEE = 1251
    {VESA_1920x1200p85   ,0x05,0x2E,0xAE,0x92,0x3A,0xEA,0x40,0xFF,0xFF,0xFF},
    // HDES = 558, HDEE = 2478, VDES = 58, VDEE = 1258
    {VESA_1920x1440p60   ,0x05,0x26,0xA6,0x92,0x3A,0xDA,0x50,0xFF,0xFF,0xFF},
    // HDES = 550, HDEE = 2470, VDES = 58, VDEE = 1498
    {VESA_1920x1440p75   ,0x05,0x3E,0xBE,0x92,0x3A,0xDA,0x50,0xFF,0xFF,0xFF},
    // HDES = 574, HDEE = 2494, VDES = 58, VDEE = 1498
    {UNKNOWN_MODE        ,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}
} ;

static _CODE SYNCGEN_DEGEN_Setting CVBS_DeGen_Table[] = {
    //                    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,        0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,      0xA0, 0xA1, 0xA2, 0xA3
    //{CEA_720x480i60      ,0xC9, 0x1B, 0x87, 0x57, 0x30, 0x0F, 0x4D, 0x00,        0x11, 0x01, 0x10, 0x18, 0x08, 0x21,      0x00, 0x30, 0x06, 0x91},
    {CEA_720x480i60      ,0xC9, 0x1B, 0x87, 0x57, 0x30, 0x0F, 0x4D, 0x00,        0x0E, 0xFE, 0x00, 0x15, 0x05, 0x21,      0x0A, 0x02, 0x03, 0x61},
    {CEA_720x576i50      ,0x89, 0x1B, 0x8D, 0x5D, 0x30, 0x08, 0x47, 0x00,        0x12, 0x32, 0x10, 0x4B, 0x6B, 0x21,      0x6E, 0x02, 0x35, 0x81},
    {UNKNOWN_MODE        ,0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,      0xFF, 0xFF, 0xFF, 0xFF}
} ;

static _CODE SYNCGEN_DEGEN_Setting YPBPR_DeGen_Table[] = {
    //                    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,        0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,      0xA0, 0xA1, 0xA2, 0xA3
    {CEA_720x480i60      ,0xB9, 0x1A, 0x75, 0x45, 0x30, 0x58, 0x3C, 0x03,        0x10, 0x00, 0x10, 0x17, 0x07, 0x21,      0x0B, 0x12, 0x05, 0x81},
    {CEA_720x480p60      ,0x99, 0x35, 0x77, 0x47, 0x30, 0x57, 0x3B, 0x03,        0x23, 0x03, 0x20, 0xFF, 0xFF, 0xFF,      0x0C, 0x52, 0xFF, 0x5F}, 
    {CEA_720x576i50      ,0xF9, 0x1A, 0x83, 0x53, 0x30, 0x5F, 0x3E, 0x03,        0x14, 0x34, 0x10, 0x4D, 0x6D, 0x21,      0x6F, 0x12, 0x37, 0xA1},
    {CEA_720x576p50      ,0xF9, 0x35, 0x83, 0x53, 0x30, 0x5F, 0x3F, 0x03,        0x2B, 0x6B, 0x20, 0xff, 0xff, 0xff,      0x70, 0x42, 0xff, 0x3f},         
    {CEA_1280x720p60     ,0x1F, 0x67, 0x2F, 0x2F, 0x61, 0x2B, 0x53, 0x00,        0x17, 0xE7, 0x20, 0xFF, 0xFF, 0xFF,      0xED, 0x32, 0xFF, 0x3F},                                                               
    {CEA_1280x720p50     ,0xBF, 0x7B, 0x2F, 0x2F, 0x61, 0x2B, 0x53, 0x00,        0x18, 0xE8, 0x20, 0xFF, 0xFF, 0xFF,      0x00, 0x30, 0xFF, 0x4F},                             
    {CEA_1920x1080i60    ,0xBF, 0x47, 0xEF, 0x6F, 0x80, 0x2F, 0x5B, 0x00,        0x13, 0x2F, 0x20, 0x46, 0x62, 0x42,      0x00, 0x50, 0x32, 0x72},
    {CEA_1920x1080i50    ,0x7F, 0x55, 0xEF, 0x6F, 0x80, 0x2F, 0x5B, 0x00,        0x13, 0x2F, 0x20, 0x46, 0x62, 0x42,      0x00, 0x50, 0x32, 0x72},
    {CEA_1920x1080p60    ,0x7F, 0x89, 0xe7, 0x67, 0x80, 0x27, 0x53, 0x00,        0x27, 0x5f, 0x40, 0xff, 0xff, 0xff,      0x64, 0x34, 0xff, 0x3f},    
    {CEA_1920x1080p50    ,0xFF, 0xA4, 0xe9, 0x69, 0x80, 0x29, 0x55, 0x00,        0x27, 0x5f, 0x40, 0xff, 0xff, 0xff,      0x64, 0x34, 0xff, 0x3f},
    {CEA_1920x1080p24    ,0xDF, 0xAB, 0xeB, 0x6B, 0x80, 0x2b, 0x57, 0x00,        0x27, 0x5f, 0x40, 0xff, 0xff, 0xff,      0x64, 0x34, 0xff, 0x3f}, 
    {UNKNOWN_MODE        ,0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,      0xFF, 0xFF, 0xFF, 0xFF}			    
} ;
    
BOOL ProgramDEGenModeByID_66121(MODE_ID id,BYTE bInputSignalType, TXInput_Device inputDevice)
{
    int i ;
    if( (bInputSignalType & (T_MODE_DEGEN|T_MODE_SYNCGEN|T_MODE_SYNCEMB) )==(T_MODE_DEGEN))
    {
        switch (inputDevice)
        {
            case TXIN_HDMIRX:
                if (id == VESA_GET_HDMIRX_DE)
                {
                    BYTE Reg90; 
                    BYTE Reg92, Reg93, Reg94; 
                    BYTE Reg9A, Reg9B, Reg9C;
                    BYTE Reg9D, Reg9E, Reg9F;
                    
                    Reg90 = 0x01;
                    
                    Reg92 = (timingDE_66121.HDES & 0xFF);
                    Reg93 = (timingDE_66121.HDEE & 0xFF);
                    Reg94 = (((timingDE_66121.HDEE >> 8) << 4) | (timingDE_66121.HDES >> 8));

                    Reg9A = (timingDE_66121.VDES & 0xFF);
                    Reg9B = (timingDE_66121.VDEE & 0xFF);
                    Reg9C = (((timingDE_66121.VDEE >> 8) << 4) | (timingDE_66121.VDES >> 8));
                    
                    Reg9D = Reg9E = Reg9F = 0xFF;
                                        
                    Switch_HDMITX_Bank_66121(0);
                    HDMITX_WriteI2C_Byte_66121(0x90, Reg90);
                    HDMITX_WriteI2C_Byte_66121(0x92, Reg92);
                    HDMITX_WriteI2C_Byte_66121(0x93, Reg93);
                    HDMITX_WriteI2C_Byte_66121(0x94, Reg94);
                    HDMITX_WriteI2C_Byte_66121(0x9A, Reg9A);
                    HDMITX_WriteI2C_Byte_66121(0x9B, Reg9B);
                    HDMITX_WriteI2C_Byte_66121(0x9C, Reg9C);
                    HDMITX_WriteI2C_Byte_66121(0x9D, Reg9D);
                    HDMITX_WriteI2C_Byte_66121(0x9E, Reg9E);
                    HDMITX_WriteI2C_Byte_66121(0x9F, Reg9F);                    
                } 
                else
                {
                    for( i = 0 ; DeGen_Table[i].id != UNKNOWN_MODE ; i++ )
                    {
                        if( id == DeGen_Table[i].id ) break ;
                    }
                    if( DeGen_Table[i].id == UNKNOWN_MODE )
                    {
                        return FALSE ;
                    }
    
                    Switch_HDMITX_Bank_66121(0);
                    HDMITX_WriteI2C_Byte_66121(0x90, DeGen_Table[i].Reg90);
                    HDMITX_WriteI2C_Byte_66121(0x92, DeGen_Table[i].Reg92);
                    HDMITX_WriteI2C_Byte_66121(0x93, DeGen_Table[i].Reg93);
                    HDMITX_WriteI2C_Byte_66121(0x94, DeGen_Table[i].Reg94);
                    HDMITX_WriteI2C_Byte_66121(0x9A, DeGen_Table[i].Reg9A);
                    HDMITX_WriteI2C_Byte_66121(0x9B, DeGen_Table[i].Reg9B);
                    HDMITX_WriteI2C_Byte_66121(0x9C, DeGen_Table[i].Reg9C);
                    HDMITX_WriteI2C_Byte_66121(0x9D, DeGen_Table[i].Reg9D);
                    HDMITX_WriteI2C_Byte_66121(0x9E, DeGen_Table[i].Reg9E);
                    HDMITX_WriteI2C_Byte_66121(0x9F, DeGen_Table[i].Reg9F);
                }
                return TRUE;                        

            case TXIN_CVBS:
                for( i = 0 ; CVBS_DeGen_Table[i].id != UNKNOWN_MODE ; i++ )
                {
                    if( id == CVBS_DeGen_Table[i].id ) break ;
                }
                if( CVBS_DeGen_Table[i].id == UNKNOWN_MODE )
                {
                    return FALSE ;
                }
       
                Switch_HDMITX_Bank_66121(0);
                HDMITX_WriteI2C_Byte_66121(0x90, CVBS_DeGen_Table[i].Reg90);
                HDMITX_WriteI2C_Byte_66121(0x91, CVBS_DeGen_Table[i].Reg91);
                HDMITX_WriteI2C_Byte_66121(0x92, CVBS_DeGen_Table[i].Reg92);
                HDMITX_WriteI2C_Byte_66121(0x93, CVBS_DeGen_Table[i].Reg93);
                HDMITX_WriteI2C_Byte_66121(0x94, CVBS_DeGen_Table[i].Reg94);
                HDMITX_WriteI2C_Byte_66121(0x95, CVBS_DeGen_Table[i].Reg95);
                HDMITX_WriteI2C_Byte_66121(0x96, CVBS_DeGen_Table[i].Reg96);
                HDMITX_WriteI2C_Byte_66121(0x97, CVBS_DeGen_Table[i].Reg97);
                HDMITX_WriteI2C_Byte_66121(0x9A, CVBS_DeGen_Table[i].Reg9A);
                HDMITX_WriteI2C_Byte_66121(0x9B, CVBS_DeGen_Table[i].Reg9B);
                HDMITX_WriteI2C_Byte_66121(0x9C, CVBS_DeGen_Table[i].Reg9C);
                HDMITX_WriteI2C_Byte_66121(0x9D, CVBS_DeGen_Table[i].Reg9D);
                HDMITX_WriteI2C_Byte_66121(0x9E, CVBS_DeGen_Table[i].Reg9E);
                HDMITX_WriteI2C_Byte_66121(0x9F, CVBS_DeGen_Table[i].Reg9F);
                HDMITX_WriteI2C_Byte_66121(0xA0, CVBS_DeGen_Table[i].RegA0);
                HDMITX_WriteI2C_Byte_66121(0xA1, CVBS_DeGen_Table[i].RegA1);
                HDMITX_WriteI2C_Byte_66121(0xA2, CVBS_DeGen_Table[i].RegA2);
                HDMITX_WriteI2C_Byte_66121(0xA3, CVBS_DeGen_Table[i].RegA3); 
                return TRUE;                    
                
            case TXIN_YPBPR: 
                for( i = 0 ; YPBPR_DeGen_Table[i].id != UNKNOWN_MODE ; i++ )
                {
                    if( id == YPBPR_DeGen_Table[i].id ) break ;
                }
                if( YPBPR_DeGen_Table[i].id == UNKNOWN_MODE )
                {
                    return FALSE ;
                }
                                
                Switch_HDMITX_Bank_66121(0);
                HDMITX_WriteI2C_Byte_66121(0x90, YPBPR_DeGen_Table[i].Reg90);
                HDMITX_WriteI2C_Byte_66121(0x91, YPBPR_DeGen_Table[i].Reg91);
                HDMITX_WriteI2C_Byte_66121(0x92, YPBPR_DeGen_Table[i].Reg92);
                HDMITX_WriteI2C_Byte_66121(0x93, YPBPR_DeGen_Table[i].Reg93);
                HDMITX_WriteI2C_Byte_66121(0x94, YPBPR_DeGen_Table[i].Reg94);
                HDMITX_WriteI2C_Byte_66121(0x95, YPBPR_DeGen_Table[i].Reg95);
                HDMITX_WriteI2C_Byte_66121(0x96, YPBPR_DeGen_Table[i].Reg96);
                HDMITX_WriteI2C_Byte_66121(0x97, YPBPR_DeGen_Table[i].Reg97);
                HDMITX_WriteI2C_Byte_66121(0x9A, YPBPR_DeGen_Table[i].Reg9A);
                HDMITX_WriteI2C_Byte_66121(0x9B, YPBPR_DeGen_Table[i].Reg9B);
                HDMITX_WriteI2C_Byte_66121(0x9C, YPBPR_DeGen_Table[i].Reg9C);
                HDMITX_WriteI2C_Byte_66121(0x9D, YPBPR_DeGen_Table[i].Reg9D);
                HDMITX_WriteI2C_Byte_66121(0x9E, YPBPR_DeGen_Table[i].Reg9E);
                HDMITX_WriteI2C_Byte_66121(0x9F, YPBPR_DeGen_Table[i].Reg9F);
                HDMITX_WriteI2C_Byte_66121(0xA0, YPBPR_DeGen_Table[i].RegA0);
                HDMITX_WriteI2C_Byte_66121(0xA1, YPBPR_DeGen_Table[i].RegA1);
                HDMITX_WriteI2C_Byte_66121(0xA2, YPBPR_DeGen_Table[i].RegA2);
                HDMITX_WriteI2C_Byte_66121(0xA3, YPBPR_DeGen_Table[i].RegA3);                               
                return TRUE;
                
            default:
                break;     
        }   
    }
    return FALSE ;
}

#endif

#ifdef SUPPORT_SYNCEMBEDDED
/* ****************************************************** */
// sync embedded table setting,defined as comment.
/* ****************************************************** */
struct SyncEmbeddedSetting {
    BYTE fmt ;
    BYTE RegHVPol ; // Reg90
    BYTE RegHfPixel ; // Reg91
    BYTE RegHSSL ; // Reg95
    BYTE RegHSEL ; // Reg96
    BYTE RegHSH ; // Reg97
    BYTE RegVSS1 ; // RegA0
    BYTE RegVSE1 ; // RegA1
    BYTE RegVSS2 ; // RegA2
    BYTE RegVSE2 ; // RegA3

    ULONG PCLK ;
    BYTE VFreq ;
} ;

#if 1
static _CODE struct SyncEmbeddedSetting SyncEmbTable[] = {
 // {FMT,0x90,0x91,
 //                 0x95,0x96,0x97,0xA0,0xA1,0xA2,0xA3,PCLK,VFREQ},
    {   1,0xF0,0x31,0x0E,0x6E,0x00,0x0A,0xC0,0xFF,0xFF,25175000,60},
    {   2,0xF0,0x31,0x0E,0x4c,0x00,0x09,0xF0,0xFF,0xFF,27000000,60},
    {   3,0xF0,0x31,0x0E,0x4c,0x00,0x09,0xF0,0xFF,0xFF,27000000,60},
    {   4,0x76,0x33,0x6c,0x94,0x00,0x05,0xA0,0xFF,0xFF,74175000,60},
    {   5,0x26,0x4A,0x56,0x82,0x00,0x02,0x70,0x34,0x92,74175000,60},
    {   6,0xE0,0x1B,0x11,0x4F,0x00,0x04,0x70,0x0A,0xD1,27000000,60},
    {   7,0xE0,0x1B,0x11,0x4F,0x00,0x04,0x70,0x0A,0xD1,27000000,60},
    {   8,0x00,0xff,0x11,0x4F,0x00,0x04,0x70,0xFF,0xFF,27000000,60},
    {   9,0x00,0xff,0x11,0x4F,0x00,0x04,0x70,0xFF,0xFF,27000000,60},
    {  10,0xe0,0x1b,0x11,0x4F,0x00,0x04,0x70,0x0A,0xD1,54000000,60},
    {  11,0xe0,0x1b,0x11,0x4F,0x00,0x04,0x70,0x0A,0xD1,54000000,60},
    {  12,0x00,0xff,0x11,0x4F,0x00,0x04,0x70,0xFF,0xFF,54000000,60},
    {  13,0x00,0xff,0x11,0x4F,0x00,0x04,0x70,0xFF,0xFF,54000000,60},
    {  14,0x00,0xff,0x1e,0x9A,0x00,0x09,0xF0,0xFF,0xFF,54000000,60},
    {  15,0x00,0xff,0x1e,0x9A,0x00,0x09,0xF0,0xFF,0xFF,54000000,60},
    {  16,0x06,0xff,0x56,0x82,0x00,0x04,0x90,0xFF,0xFF,148350000,60},
    {  17,0x00,0xff,0x0a,0x4A,0x00,0x05,0xA0,0xFF,0xFF,27000000,50},
    {  18,0x00,0xff,0x0a,0x4A,0x00,0x05,0xA0,0xFF,0xFF,27000000,50},
    {  19,0x06,0xff,0xB6,0xDE,0x11,0x05,0xA0,0xFF,0xFF,74250000,50},
    {  20,0x66,0x73,0x0e,0x3A,0x22,0x02,0x70,0x34,0x92,74250000,50},
    {  21,0xA0,0x1B,0x0a,0x49,0x00,0x02,0x50,0x3A,0xD1,27000000,50},
    {  22,0xA0,0x1B,0x0a,0x49,0x00,0x02,0x50,0x3A,0xD1,27000000,50},
    {  23,0x00,0xff,0x0a,0x49,0x00,0x02,0x50,0xFF,0xFF,27000000,50},
    {  24,0x00,0xff,0x0a,0x49,0x00,0x02,0x50,0xFF,0xFF,27000000,50},
    {  25,0xA0,0x1B,0x0a,0x49,0x00,0x02,0x50,0x3A,0xD1,54000000,50},
    {  26,0xA0,0x1B,0x0a,0x49,0x00,0x02,0x50,0x3A,0xD1,54000000,50},
    {  27,0x00,0xff,0x0a,0x49,0x00,0x02,0x50,0xFF,0xFF,54000000,50},
    {  28,0x00,0xff,0x0a,0x49,0x00,0x02,0x50,0xFF,0xFF,54000000,50},
    {  29,0x04,0xff,0x16,0x96,0x00,0x05,0xA0,0xFF,0xFF,54000000,50},
    {  30,0x04,0xff,0x16,0x96,0x00,0x05,0xA0,0xFF,0xFF,54000000,50},
    {  31,0x06,0xff,0x0e,0x3a,0x22,0x04,0x90,0xFF,0xFF,148500000,50},
    {  32,0xF6,0xFF,0x7C,0xA8,0x22,0x04,0x90,0xFF,0xFF,  74000000L,24},// 1920x1080@24Hz
    {  33,0xF6,0xFF,0x0E,0x3A,0x22,0x04,0x90,0xFF,0xFF,  74000000L,25},// 1920x1080@25Hz
    {  34,0xF6,0xFF,0x56,0x82,0x00,0x04,0x90,0xFF,0xFF,  74000000L,30},// 1920x1080@30Hz
    {  60,0xF0,0xFF,0xDE,0x06,0x76,0x05,0xA0,0xFF,0xFF,  59400000L,24},// 1280x720@24Hz
    {  61,0xF0,0xFF,0x72,0x9A,0x99,0x05,0xA0,0xFF,0xFF,  74250000L,25},// 1280x720@25Hz
    {  62,0xF0,0xFF,0xDE,0x06,0x76,0x05,0xA0,0xFF,0xFF,  74250000L,30},// 1280x720@30Hz
    {0xFF,0xFF,0xff,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0,0}
} ;

BOOL
ProgramSyncEmbeddedByVIC_66121(BYTE VIC,BYTE bInputSignalType)
{
    int i ;
    // if Embedded Video,need to generate timing with pattern register

    HDMITX_DEBUG_PRINTF(("ProgramSyncEmbeddedByVIC_66121(%d,%x)\n",(int)VIC,(int)bInputSignalType));

    if( bInputSignalType & T_MODE_SYNCEMB )
    {
        for(i = 0 ; SyncEmbTable[i].fmt != 0xFF ; i++)
        {
            if(VIC == SyncEmbTable[i].fmt)
            {
                break ;
            }
        }

        if(SyncEmbTable[i].fmt == 0xFF)
        {
            return FALSE ;
        }

        HDMITX_WriteI2C_Byte_66121(REG_TX_HVPol,SyncEmbTable[i].RegHVPol); // Reg90
        HDMITX_WriteI2C_Byte_66121(REG_TX_HfPixel,SyncEmbTable[i].RegHfPixel); // Reg91

        HDMITX_WriteI2C_Byte_66121(REG_TX_HSSL,SyncEmbTable[i].RegHSSL); // Reg95
        HDMITX_WriteI2C_Byte_66121(REG_TX_HSEL,SyncEmbTable[i].RegHSEL); // Reg96
        HDMITX_WriteI2C_Byte_66121(REG_TX_HSH,SyncEmbTable[i].RegHSH); // Reg97
        HDMITX_WriteI2C_Byte_66121(REG_TX_VSS1,SyncEmbTable[i].RegVSS1); // RegA0
        HDMITX_WriteI2C_Byte_66121(REG_TX_VSE1,SyncEmbTable[i].RegVSE1); // RegA1

        HDMITX_WriteI2C_Byte_66121(REG_TX_VSS2,SyncEmbTable[i].RegVSS2); // RegA2
        HDMITX_WriteI2C_Byte_66121(REG_TX_VSE2,SyncEmbTable[i].RegVSE2); // RegA3
    }

    return TRUE ;
}
#else
struct CRT_TimingSetting {
	BYTE fmt;
    WORD HActive;
    WORD VActive;
    WORD HTotal;
    WORD VTotal;
    WORD H_FBH;
    WORD H_SyncW;
    WORD H_BBH;
    WORD V_FBH;
    WORD V_SyncW;
    WORD V_BBH;
    BYTE Scan:1;
    BYTE VPolarity:1;
    BYTE HPolarity:1;
};

//   VDEE_L,   VDEE_H, VRS2S_L, VRS2S_H, VRS2E_L, VRS2E_H, HalfL_L, HalfL_H, VDE2S_L, VDE2S_H, HVP&Progress
_CODE struct CRT_TimingSetting TimingTable[] =
{
    //  VIC   H     V    HTotal VTotal  HFT   HSW     HBP VF VSW   VB
    {  1,  640,  480,    800,  525,   16,    96,    48, 10, 2,  33,      PROG, Vneg, Hneg},// 640x480@60Hz         - CEA Mode [ 1]
    {  2,  720,  480,    858,  525,   16,    62,    60,  9, 6,  30,      PROG, Vneg, Hneg},// 720x480@60Hz         - CEA Mode [ 2]
    {  3,  720,  480,    858,  525,   16,    62,    60,  9, 6,  30,      PROG, Vneg, Hneg},// 720x480@60Hz         - CEA Mode [ 3]
    {  4, 1280,  720,   1650,  750,  110,    40,   220,  5, 5,  20,      PROG, Vpos, Hpos},// 1280x720@60Hz        - CEA Mode [ 4]
    {  5, 1920,  540,   2200,  562,   88,    44,   148,  2, 5,  15, INTERLACE, Vpos, Hpos},// 1920x1080(I)@60Hz    - CEA Mode [ 5]
    {  6,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [ 6]
    {  7,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [ 7]
    // {  8,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15,      PROG, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [ 8]
    // {  9,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15,      PROG, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [ 9]
    // { 10,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [10]
    // { 11,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [11]
    // { 12,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15,      PROG, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [12]
    // { 13,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15,      PROG, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [13]
    // { 14, 1440,  480,   1716,  525,   32,   124,   120,  9, 6,  30,      PROG, Vneg, Hneg},// 1440x480@60Hz        - CEA Mode [14]
    // { 15, 1440,  480,   1716,  525,   32,   124,   120,  9, 6,  30,      PROG, Vneg, Hneg},// 1440x480@60Hz        - CEA Mode [15]
    { 16, 1920, 1080,   2200, 1125,   88,    44,   148,  4, 5,  36,      PROG, Vpos, Hpos},// 1920x1080@60Hz       - CEA Mode [16]
    { 17,  720,  576,    864,  625,   12,    64,    68,  5, 5,  39,      PROG, Vneg, Hneg},// 720x576@50Hz         - CEA Mode [17]
    { 18,  720,  576,    864,  625,   12,    64,    68,  5, 5,  39,      PROG, Vneg, Hneg},// 720x576@50Hz         - CEA Mode [18]
    { 19, 1280,  720,   1980,  750,  440,    40,   220,  5, 5,  20,      PROG, Vpos, Hpos},// 1280x720@50Hz        - CEA Mode [19]
    { 20, 1920,  540,   2640,  562,  528,    44,   148,  2, 5,  15, INTERLACE, Vpos, Hpos},// 1920x1080(I)@50Hz    - CEA Mode [20]
    { 21,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@50Hz     - CEA Mode [21]
    { 22,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@50Hz     - CEA Mode [22]
    // { 23,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19,      PROG, Vneg, Hneg},// 1440x288@50Hz        - CEA Mode [23]
    // { 24,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19,      PROG, Vneg, Hneg},// 1440x288@50Hz        - CEA Mode [24]
    // { 25,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@50Hz     - CEA Mode [25]
    // { 26,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@50Hz     - CEA Mode [26]
    // { 27,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19,      PROG, Vneg, Hneg},// 1440x288@50Hz        - CEA Mode [27]
    // { 28,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19,      PROG, Vneg, Hneg},// 1440x288@50Hz        - CEA Mode [28]
    // { 29, 1440,  576,   1728,  625,   24,   128,   136,  5, 5,  39,      PROG, Vpos, Hneg},// 1440x576@50Hz        - CEA Mode [29]
    // { 30, 1440,  576,   1728,  625,   24,   128,   136,  5, 5,  39,      PROG, Vpos, Hneg},// 1440x576@50Hz        - CEA Mode [30]
    { 31, 1920, 1080,   2640, 1125,  528,    44,   148,  4, 5,  36,      PROG, Vpos, Hpos},// 1920x1080@50Hz       - CEA Mode [31]
    { 32, 1920, 1080,   2750, 1125,  638,    44,   148,  4, 5,  36,      PROG, Vpos, Hpos},// 1920x1080@24Hz       - CEA Mode [32]
    { 33, 1920, 1080,   2640, 1125,  528,    44,   148,  4, 5,  36,      PROG, Vpos, Hpos},// 1920x1080@25Hz       - CEA Mode [33]
    { 34, 1920, 1080,   2200, 1125,   88,    44,   148,  4, 5,  36,      PROG, Vpos, Hpos},// 1920x1080@30Hz       - CEA Mode [34]
    // { 35, 2880,  480, 1716*2,  525, 32*2, 124*2, 120*2,  9, 6,  30,      PROG, Vneg, Hneg},// 2880x480@60Hz        - CEA Mode [35]
    // { 36, 2880,  480, 1716*2,  525, 32*2, 124*2, 120*2,  9, 6,  30,      PROG, Vneg, Hneg},// 2880x480@60Hz        - CEA Mode [36]
    // { 37, 2880,  576,   3456,  625, 24*2, 128*2, 136*2,  5, 5,  39,      PROG, Vneg, Hneg},// 2880x576@50Hz        - CEA Mode [37]
    // { 38, 2880,  576,   3456,  625, 24*2, 128*2, 136*2,  5, 5,  39,      PROG, Vneg, Hneg},// 2880x576@50Hz        - CEA Mode [38]
    // { 39, 1920,  540,   2304,  625,   32,   168,   184, 23, 5,  57, INTERLACE, Vneg, Hpos},// 1920x1080@50Hz       - CEA Mode [39]
    // { 40, 1920,  540,   2640,  562,  528,    44,   148,  2, 5,  15, INTERLACE, Vpos, Hpos},// 1920x1080(I)@100Hz   - CEA Mode [40]
    // { 41, 1280,  720,   1980,  750,  440,    40,   220,  5, 5,  20,      PROG, Vpos, Hpos},// 1280x720@100Hz       - CEA Mode [41]
    // { 42,  720,  576,    864,  625,   12,    64,    68,  5, 5,  39,      PROG, Vneg, Hneg},// 720x576@100Hz        - CEA Mode [42]
    // { 43,  720,  576,    864,  625,   12,    64,    68,  5, 5,  39,      PROG, Vneg, Hneg},// 720x576@100Hz        - CEA Mode [43]
    // { 44,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@100Hz    - CEA Mode [44]
    // { 45,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@100Hz    - CEA Mode [45]
    // { 46, 1920,  540,   2200,  562,   88,    44,   148,  2, 5,  15, INTERLACE, Vpos, Hpos},// 1920x1080(I)@120Hz   - CEA Mode [46]
    // { 47, 1280,  720,   1650,  750,  110,    40,   220,  5, 5,  20,      PROG, Vpos, Hpos},// 1280x720@120Hz       - CEA Mode [47]
    // { 48,  720,  480,    858,  525,   16,    62,    60,  9, 6,  30,      PROG, Vneg, Hneg},// 720x480@120Hz        - CEA Mode [48]
    // { 49,  720,  480,    858,  525,   16,    62,    60,  9, 6,  30,      PROG, Vneg, Hneg},// 720x480@120Hz        - CEA Mode [49]
    // { 50,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@120Hz     - CEA Mode [50]
    // { 51,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@120Hz     - CEA Mode [51]
    // { 52,  720,  576,    864,  625,   12,    64,    68,  5, 5,  39,      PROG, Vneg, Hneg},// 720x576@200Hz        - CEA Mode [52]
    // { 53,  720,  576,    864,  625,   12,    64,    68,  5, 5,  39,      PROG, Vneg, Hneg},// 720x576@200Hz        - CEA Mode [53]
    // { 54,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@200Hz    - CEA Mode [54]
    // { 55,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@200Hz    - CEA Mode [55]
    // { 56,  720,  480,    858,  525,   16,    62,    60,  9, 6,  30,      PROG, Vneg, Hneg},// 720x480@120Hz        - CEA Mode [56]
    // { 57,  720,  480,    858,  525,   16,    62,    60,  9, 6,  30,      PROG, Vneg, Hneg},// 720x480@120Hz        - CEA Mode [57]
    // { 58,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@120Hz     - CEA Mode [58]
    // { 59,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@120Hz     - CEA Mode [59]
    { 60, 1280,  720,   3300,  750, 1760,    40,   220,  5, 5,  20,      PROG, Vpos, Hpos},// 1280x720@24Hz        - CEA Mode [60]
    { 61, 1280,  720,   3960,  750, 2420,    40,   220,  5, 5,  20,      PROG, Vpos, Hpos},// 1280x720@25Hz        - CEA Mode [61]
    { 62, 1280,  720,   3300,  750, 1760,    40,   220,  5, 5,  20,      PROG, Vpos, Hpos},// 1280x720@30Hz        - CEA Mode [62]
    // { 63, 1920, 1080,   2200, 1125,   88,    44,   148,  4, 5,  36,      PROG, Vpos, Hpos},// 1920x1080@120Hz      - CEA Mode [63]
    // { 64, 1920, 1080,   2640, 1125,  528,    44,   148,  4, 5,  36,      PROG, Vpos, Hpos},// 1920x1080@100Hz      - CEA Mode [64]
};

#define MaxIndex (sizeof(TimingTable)/sizeof(struct CRT_TimingSetting))

BOOL
ProgramSyncEmbeddedByVIC_66121(BYTE VIC,BYTE bInputSignalType)
{
	int i ;
    BYTE fmt_index=0;

    // if Embedded Video,need to generate timing with pattern register
    Switch_HDMITX_Bank_66121(0);

    HDMITX_DEBUG_PRINTF(("setHDMITX_SyncEmbeddedByVIC(%d,%x)\n",(int)VIC,(int)bInputSignalType));
    for(i=0;i< MaxIndex;i ++)
    {
        if(TimingTable[i].fmt==VIC)
        {
            fmt_index=i;
            HDMITX_DEBUG_PRINTF(("fmt_index=%02x)\n",(int)fmt_index));
            HDMITX_DEBUG_PRINTF(("***Fine Match Table ***\n"));
            break;
        }
    }
    if(i>=MaxIndex)
    {
        //return FALSE;
        HDMITX_DEBUG_PRINTF(("***No Match VIC ***\n"));
        return FALSE ;
    }
    //if( bInputSignalType & T_MODE_SYNCEMB )
    {
        int HTotal, HDES, VTotal, VDES;
        int HDEW, VDEW, HFP, HSW, VFP, VSW;
        int HRS, HRE;
        int VRS, VRE;
        int H2ndVRRise;
        int VRS2nd, VRE2nd;
        BYTE Pol;

        HTotal  =TimingTable[fmt_index].HTotal;
        HDEW    =TimingTable[fmt_index].HActive;
        HFP     =TimingTable[fmt_index].H_FBH;
        HSW     =TimingTable[fmt_index].H_SyncW;
        HDES    =HSW+TimingTable[fmt_index].H_BBH;
        VTotal  =TimingTable[fmt_index].VTotal;
        VDEW    =TimingTable[fmt_index].VActive;
        VFP     =TimingTable[fmt_index].V_FBH;
        VSW     =TimingTable[fmt_index].V_SyncW;
        VDES    =VSW+TimingTable[fmt_index].V_BBH;

        Pol = (TimingTable[fmt_index].HPolarity==Hpos)?(1<<1):0 ;
        Pol |= (TimingTable[fmt_index].VPolarity==Vpos)?(1<<2):0 ;

        // SyncEmb case=====
        if( bInputSignalType & T_MODE_CCIR656)
        {
            HRS = HFP - 1;
        }
        else
        {
            HRS = HFP - 2;
            /*
            if(VIC==HDMI_1080p60 ||
               VIC==HDMI_1080p50 )
            {
                HDMITX_OrReg_Byte_66121(0x59, (1<<3));
            }
            else
            {
                HDMITX_AndReg_Byte_66121(0x59, ~(1<<3));
            }
            */
        }
        HRE = HRS + HSW;
        H2ndVRRise = HRS+ HTotal/2;

        VRS = VFP;
        VRE = VRS + VSW;

        // VTotal>>=1;

        if(PROG == TimingTable[fmt_index].Scan)
        { // progressive mode
            VRS2nd = 0xFFF;
            VRE2nd = 0x3F;
        }
        else
        { // interlaced mode
            if(39 == TimingTable[fmt_index].fmt)
            {
                VRS2nd = VRS + VTotal - 1;
                VRE2nd = VRS2nd + VSW;
            }
            else
            {
                VRS2nd = VRS + VTotal;
                VRE2nd = VRS2nd + VSW;
            }
        }
        #if 0
        if( EnSavVSync )
        {
            VRS -= 1;
            VRE -= 1;
            if( !pSetVTiming->ScanMode ) // interlaced mode
            {
                VRS2nd -= 1;
                VRE2nd -= 1;
            }
        }
        #endif // DETECT_VSYNC_CHG_IN_SAV
        HDMITX_SetI2C_Byte(0x90, 0x06, Pol);
        // write H2ndVRRise
        HDMITX_SetI2C_Byte(0x90, 0xF0, (H2ndVRRise&0x0F)<<4);
        HDMITX_WriteI2C_Byte_66121(0x91, (H2ndVRRise&0x0FF0)>>4);
        // write HRS/HRE
        HDMITX_WriteI2C_Byte_66121(0x95, HRS&0xFF);
        HDMITX_WriteI2C_Byte_66121(0x96, HRE&0xFF);
        HDMITX_WriteI2C_Byte_66121(0x97, ((HRE&0x0F00)>>4)+((HRS&0x0F00)>>8));
        // write VRS/VRE
        HDMITX_WriteI2C_Byte_66121(0xa0, VRS&0xFF);
        HDMITX_WriteI2C_Byte_66121(0xa1, ((VRE&0x0F)<<4)+((VRS&0x0F00)>>8));
        HDMITX_WriteI2C_Byte_66121(0xa2, VRS2nd&0xFF);
        HDMITX_WriteI2C_Byte_66121(0xa6, (VRE2nd&0xF0)+((VRE&0xF0)>>4));
        HDMITX_WriteI2C_Byte_66121(0xa3, ((VRE2nd&0x0F)<<4)+((VRS2nd&0xF00)>>8));
        HDMITX_WriteI2C_Byte_66121(0xa4, H2ndVRRise&0xFF);
        HDMITX_WriteI2C_Byte_66121(0xa5, (/*EnDEOnly*/0<<5)+((TimingTable[fmt_index].Scan==INTERLACE)?(1<<4):0)+((H2ndVRRise&0xF00)>>8));
        HDMITX_SetI2C_Byte(0xb1, 0x51, ((HRE&0x1000)>>6)+((HRS&0x1000)>>8)+((HDES&0x1000)>>12));
        HDMITX_SetI2C_Byte(0xb2, 0x05, ((H2ndVRRise&0x1000)>>10)+((H2ndVRRise&0x1000)>>12));
    }
    return TRUE ;
}

#endif
BOOL
ProgramSyncEmbeddedByTiming_66121(VIDEO_Timing *pVTiming,BYTE bInputSignalType)
{
    unsigned int temp, t2 ;
    BYTE uc ;
    // if Embedded Video,need to generate timing with pattern register

    HDMITX_DEBUG_PRINTF(("ProgramSyncEmbeddedByTiming_66121(%dx%d(%dx%d),%x)\n",
        (int)pVTiming->HActive,(int)pVTiming->VActive,
        (int)pVTiming->HTotal,(int)pVTiming->VTotal,
        (int)bInputSignalType));

    if( bInputSignalType & T_MODE_SYNCEMB )
    {
        temp = (pVTiming->HTotal / 2)+(pVTiming->HFrontPorch -2 ) ;


        if( pVTiming->ScanMode == INTERLACE )
        {
            uc = (BYTE)(temp&0xF)<<4;
            uc |=((pVTiming->HPolarity==Hpos)?(1<<1):0)|((pVTiming->VPolarity==Vpos)?(1<<2):0);
            HDMITX_WriteI2C_Byte_66121(REG_TX_HVPol,uc); // Reg90
            temp >>= 4 ;
            uc = (BYTE)(temp);
            HDMITX_WriteI2C_Byte_66121(REG_TX_HfPixel,uc); // Reg91
        }
        else
        {
            uc = 0xF0 ;
            uc |=((pVTiming->HPolarity==Hpos)?(1<<1):0)|((pVTiming->VPolarity==Vpos)?(1<<2):0);
            HDMITX_WriteI2C_Byte_66121(REG_TX_HVPol,uc); // Reg90

            HDMITX_WriteI2C_Byte_66121(REG_TX_HfPixel,0xFF); // Reg91
        }

        temp = pVTiming->HFrontPorch - 2;
        t2  = temp + pVTiming->HSyncWidth ;

        uc = (BYTE)temp & 0xFF ;
        HDMITX_WriteI2C_Byte_66121(REG_TX_HSSL,uc); // Reg95
        uc = (BYTE)t2 & 0xFF ;
        HDMITX_WriteI2C_Byte_66121(REG_TX_HSEL,uc); // Reg96

        temp >>= 8 ; temp &= 0xF ;
        t2 >>= 4 ; t2 &= 0xF0 ;
        temp |= t2 ;
        uc =  (BYTE)(temp);
        HDMITX_WriteI2C_Byte_66121(REG_TX_HSH,uc); // Reg97

        temp = pVTiming->VFrontPorch ;
        t2  = temp + pVTiming->VSyncWidth ;

        uc = (BYTE)temp & 0xFF ;
        HDMITX_WriteI2C_Byte_66121(REG_TX_VSS1,uc); // RegA0

        uc = (BYTE)((t2&0xF)<<4);
        uc |= (BYTE)(temp >> 8) ;

        HDMITX_WriteI2C_Byte_66121(REG_TX_VSE1,uc); // RegA1

        if( pVTiming->ScanMode == INTERLACE )
        {
            temp += pVTiming->VTotal ;
            t2 += pVTiming->VTotal ;

            uc = (BYTE)temp&0xFF ;
            HDMITX_WriteI2C_Byte_66121(REG_TX_VSS2,uc); // RegA2

            temp >>= 8 ; temp &= 0xF ;
            t2 &= 0xF ; t2 <<= 4 ;
            temp |= t2 ;
            uc = (BYTE)temp ;
            HDMITX_WriteI2C_Byte_66121(REG_TX_VSE2,uc); // RegA3
        }
        else
        {
            HDMITX_WriteI2C_Byte_66121(REG_TX_VSS2,0xFF); // RegA2
            HDMITX_WriteI2C_Byte_66121(REG_TX_VSE2,0xFF); // RegA3
        }
        HDMITX_DEBUG_PRINTF(("REG = {0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X}\n",
            (int)HDMITX_ReadI2C_Byte_66121(REG_TX_HVPol) ,
            (int)HDMITX_ReadI2C_Byte_66121(REG_TX_HfPixel) ,
            (int)HDMITX_ReadI2C_Byte_66121(REG_TX_HSSL) ,
            (int)HDMITX_ReadI2C_Byte_66121(REG_TX_HSEL) ,
            (int)HDMITX_ReadI2C_Byte_66121(REG_TX_HSH) ,
            (int)HDMITX_ReadI2C_Byte_66121(REG_TX_VSS1) ,
            (int)HDMITX_ReadI2C_Byte_66121(REG_TX_VSE1) ,
            (int)HDMITX_ReadI2C_Byte_66121(REG_TX_VSS2) ,
            (int)HDMITX_ReadI2C_Byte_66121(REG_TX_VSE2) )) ;

    }

    return TRUE ;
}
#endif // SUPPORT_SYNCEMBEDDED

//~jj_tseng@chipadvanced.com 2007/01/02


//////////////////////////////////////////////////////////////////////
// Function: SetInputMode
// Parameter: InputMode,bInputSignalType
//      InputMode - use [1:0] to identify the color space for reg70[7:6],
//                  definition:
//                     #define F_MODE_RGB444  0
//                     #define F_MODE_YUV422 1
//                     #define F_MODE_YUV444 2
//                     #define F_MODE_CLRMOD_MASK 3
//      bInputSignalType - defined the CCIR656 D[0],SYNC Embedded D[1],and
//                     DDR input in D[2].
// Return: N/A
// Remark: program Reg70 with the input value.
// Side-Effect: Reg70.
//////////////////////////////////////////////////////////////////////

static void
SetInputMode(BYTE InputMode,BYTE bInputSignalType)
{
    BYTE ucData ;

//    HDMITX_DEBUG_PRINTF(("SetInputMode(%02X,%02X)\n",InputMode,bInputSignalType));

    ucData = HDMITX_ReadI2C_Byte_66121(REG_TX_INPUT_MODE);

    ucData &= ~(M_INCOLMOD|B_2X656CLK|B_SYNCEMB|B_INDDR|B_PCLKDIV2);

    switch(InputMode & F_MODE_CLRMOD_MASK)
    {
    case F_MODE_YUV422:
        ucData |= B_IN_YUV422 ;
        break ;
    case F_MODE_YUV444:
        ucData |= B_IN_YUV444 ;
        break ;
    case F_MODE_RGB444:
    default:
        ucData |= B_IN_RGB ;
        break ;
    }

    if(bInputSignalType & T_MODE_PCLKDIV2)
    {
        ucData |= B_PCLKDIV2 ; HDMITX_DEBUG_PRINTF(("PCLK Divided by 2 mode\n"));
    }
    if(bInputSignalType & T_MODE_CCIR656)
    {
        ucData |= B_2X656CLK ; HDMITX_DEBUG_PRINTF(("CCIR656 mode\n"));
    }

    if(bInputSignalType & T_MODE_SYNCEMB)
    {
        ucData |= B_SYNCEMB ; HDMITX_DEBUG_PRINTF(("Sync Embedded mode\n"));
    }

    if(bInputSignalType & T_MODE_INDDR)
    {
        ucData |= B_INDDR ; HDMITX_DEBUG_PRINTF(("Input DDR mode\n"));
    }

    HDMITX_WriteI2C_Byte_66121(REG_TX_INPUT_MODE,ucData);
}

//////////////////////////////////////////////////////////////////////
// Function: SetCSCScale
// Parameter: bInputMode -
//             D[1:0] - Color Mode
//             D[4] - Colorimetry 0: ITU_BT601 1: ITU_BT709
//             D[5] - Quantization 0: 0_255 1: 16_235
//             D[6] - Up/Dn Filter 'Required'
//                    0: no up/down filter
//                    1: enable up/down filter when csc need.
//             D[7] - Dither Filter 'Required'
//                    0: no dither enabled.
//                    1: enable dither and dither free go "when required".
//            bOutputMode -
//             D[1:0] - Color mode.
// Return: N/A
// Remark: reg72~reg8D will be programmed depended the input with table.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static void
SetCSCScale(BYTE bInputMode,BYTE bOutputMode)
{
    BYTE ucData,csc ;
    BYTE filter = 0 ; // filter is for Video CTRL DN_FREE_GO,EN_DITHER,and ENUDFILT


    // (1) YUV422 in,RGB/YUV444 output (Output is 8-bit,input is 12-bit)
    // (2) YUV444/422  in,RGB output (CSC enable,and output is not YUV422)
    // (3) RGB in,YUV444 output   (CSC enable,and output is not YUV422)
    //
    // YUV444/RGB24 <-> YUV422 need set up/down filter.

    switch(bInputMode&F_MODE_CLRMOD_MASK)
    {
    #ifdef SUPPORT_INPUTYUV444
    case F_MODE_YUV444:
        HDMITX_DEBUG_PRINTF(("Input mode is YUV444 "));
        switch(bOutputMode&F_MODE_CLRMOD_MASK)
        {
        case F_MODE_YUV444:
            HDMITX_DEBUG_PRINTF(("Output mode is YUV444\n"));
            csc = B_HDMITX_CSC_BYPASS ;
            break ;

        case F_MODE_YUV422:
            HDMITX_DEBUG_PRINTF(("Output mode is YUV422\n"));
            if(bInputMode & F_MODE_EN_UDFILT) // YUV444 to YUV422 need up/down filter for processing.
            {
                filter |= B_TX_EN_UDFILTER ;
            }
            csc = B_HDMITX_CSC_BYPASS ;
            break ;
        case F_MODE_RGB444:
            HDMITX_DEBUG_PRINTF(("Output mode is RGB24\n"));
            csc = B_HDMITX_CSC_YUV2RGB ;
            if(bInputMode & F_MODE_EN_DITHER) // YUV444 to RGB24 need dither
            {
                filter |= B_TX_EN_DITHER | B_TX_DNFREE_GO ;
            }

            break ;
        }
        break ;
    #endif

    #ifdef SUPPORT_INPUTYUV422
    case F_MODE_YUV422:
        HDMITX_DEBUG_PRINTF(("Input mode is YUV422\n"));
        switch(bOutputMode&F_MODE_CLRMOD_MASK)
        {
        case F_MODE_YUV444:
            HDMITX_DEBUG_PRINTF(("Output mode is YUV444\n"));
            csc = B_HDMITX_CSC_BYPASS ;
            if(bInputMode & F_MODE_EN_UDFILT) // YUV422 to YUV444 need up filter
            {
                filter |= B_TX_EN_UDFILTER ;
            }

            if(bInputMode & F_MODE_EN_DITHER) // YUV422 to YUV444 need dither
            {
                filter |= B_TX_EN_DITHER | B_TX_DNFREE_GO ;
            }

            break ;
        case F_MODE_YUV422:
            HDMITX_DEBUG_PRINTF(("Output mode is YUV422\n"));
            csc = B_HDMITX_CSC_BYPASS ;

            break ;

        case F_MODE_RGB444:
            HDMITX_DEBUG_PRINTF(("Output mode is RGB24\n"));
            csc = B_HDMITX_CSC_YUV2RGB ;
            if(bInputMode & F_MODE_EN_UDFILT) // YUV422 to RGB24 need up/dn filter.
            {
                filter |= B_TX_EN_UDFILTER ;
            }

            if(bInputMode & F_MODE_EN_DITHER) // YUV422 to RGB24 need dither
            {
                filter |= B_TX_EN_DITHER | B_TX_DNFREE_GO ;
            }

            break ;
        }
        break ;
    #endif

    #ifdef SUPPORT_INPUTRGB
    case F_MODE_RGB444:
        HDMITX_DEBUG_PRINTF(("Input mode is RGB24\n"));
        switch(bOutputMode&F_MODE_CLRMOD_MASK)
        {
        case F_MODE_YUV444:
            HDMITX_DEBUG_PRINTF(("Output mode is YUV444\n"));
            csc = B_HDMITX_CSC_RGB2YUV ;

            if(bInputMode & F_MODE_EN_DITHER) // RGB24 to YUV444 need dither
            {
                filter |= B_TX_EN_DITHER | B_TX_DNFREE_GO ;
            }
            break ;

        case F_MODE_YUV422:
            HDMITX_DEBUG_PRINTF(("Output mode is YUV422\n"));
            if(bInputMode & F_MODE_EN_UDFILT) // RGB24 to YUV422 need down filter.
            {
                filter |= B_TX_EN_UDFILTER ;
            }

            if(bInputMode & F_MODE_EN_DITHER) // RGB24 to YUV422 need dither
            {
                filter |= B_TX_EN_DITHER | B_TX_DNFREE_GO ;
            }
            csc = B_HDMITX_CSC_RGB2YUV ;
            break ;

        case F_MODE_RGB444:
            HDMITX_DEBUG_PRINTF(("Output mode is RGB24\n"));
            csc = B_HDMITX_CSC_BYPASS ;
            break ;
        }
        break ;
    #endif
    }

#ifndef DISABLE_HDMITX_CSC

    #ifdef SUPPORT_INPUTRGB
    // set the CSC metrix registers by colorimetry and quantization
    if(csc == B_HDMITX_CSC_RGB2YUV)
    {
        HDMITX_DEBUG_PRINTF(("CSC = RGB2YUV %x ",csc));
        switch(bInputMode&(F_MODE_ITU709|F_MODE_16_235))
        {
        case F_MODE_ITU709|F_MODE_16_235:
            HDMITX_DEBUG_PRINTF(("ITU709 16-235 "));
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_YOFF,bTxCSCOffset_16_235,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_MTX11_L,bTxCSCMtx_RGB2YUV_ITU709_16_235,SIZEOF_CSCMTX);
            break ;
        case F_MODE_ITU709|F_MODE_0_255:
            HDMITX_DEBUG_PRINTF(("ITU709 0-255 "));
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_YOFF,bTxCSCOffset_0_255,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_MTX11_L,bTxCSCMtx_RGB2YUV_ITU709_0_255,SIZEOF_CSCMTX);
            break ;
        case F_MODE_ITU601|F_MODE_16_235:
            HDMITX_DEBUG_PRINTF(("ITU601 16-235 "));
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_YOFF,bTxCSCOffset_16_235,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_MTX11_L,bTxCSCMtx_RGB2YUV_ITU601_16_235,SIZEOF_CSCMTX);
            break ;
        case F_MODE_ITU601|F_MODE_0_255:
        default:
            HDMITX_DEBUG_PRINTF(("ITU601 0-255 "));
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_YOFF,bTxCSCOffset_0_255,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_MTX11_L,bTxCSCMtx_RGB2YUV_ITU601_0_255,SIZEOF_CSCMTX);
            break ;
        }

    }
    #endif

    #ifdef SUPPORT_INPUTYUV
    if (csc == B_HDMITX_CSC_YUV2RGB)
    {
        HDMITX_DEBUG_PRINTF(("CSC = YUV2RGB %x ",csc));

        switch(bInputMode&(F_MODE_ITU709|F_MODE_16_235))
        {
        case F_MODE_ITU709|F_MODE_16_235:
            HDMITX_DEBUG_PRINTF(("ITU709 16-235 "));
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_YOFF,bTxCSCOffset_16_235,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_MTX11_L,bTxCSCMtx_YUV2RGB_ITU709_16_235,SIZEOF_CSCMTX);
            break ;
        case F_MODE_ITU709|F_MODE_0_255:
            HDMITX_DEBUG_PRINTF(("ITU709 0-255 "));
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_YOFF,bTxCSCOffset_0_255,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_MTX11_L,bTxCSCMtx_YUV2RGB_ITU709_0_255,SIZEOF_CSCMTX);
            break ;
        case F_MODE_ITU601|F_MODE_16_235:
            HDMITX_DEBUG_PRINTF(("ITU601 16-235 "));
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_YOFF,bTxCSCOffset_16_235,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_MTX11_L,bTxCSCMtx_YUV2RGB_ITU601_16_235,SIZEOF_CSCMTX);
            break ;
        case F_MODE_ITU601|F_MODE_0_255:
        default:
            HDMITX_DEBUG_PRINTF(("ITU601 0-255 "));
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_YOFF,bTxCSCOffset_0_255,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN_66121(REG_TX_CSC_MTX11_L,bTxCSCMtx_YUV2RGB_ITU601_0_255,SIZEOF_CSCMTX);
            break ;
        }
    }
    #endif
#else// DISABLE_HDMITX_CSC
    csc = B_HDMITX_CSC_BYPASS ;
#endif// DISABLE_HDMITX_CSC

	if( csc == B_HDMITX_CSC_BYPASS )
	{
		HDMITX_SetI2C_Byte(0xF, 0x10, 0x10);
	}
	else
	{
		HDMITX_SetI2C_Byte(0xF, 0x10, 0x00);
	}
    ucData = HDMITX_ReadI2C_Byte_66121(REG_TX_CSC_CTRL) & ~(M_CSC_SEL|B_TX_DNFREE_GO|B_TX_EN_DITHER|B_TX_EN_UDFILTER);
    ucData |= filter|csc ;

    HDMITX_WriteI2C_Byte_66121(REG_TX_CSC_CTRL,ucData);

    // set output Up/Down Filter,Dither control

}


//////////////////////////////////////////////////////////////////////
// Function: SetupAFE
// Parameter: VIDEOPCLKLEVEL level
//            PCLK_LOW - for 13.5MHz (for mode less than 1080p)
//            PCLK MEDIUM - for 25MHz~74MHz
//            PCLK HIGH - PCLK > 80Hz (for 1080p mode or above)
// Return: N/A
// Remark: set reg62~reg65 depended on HighFreqMode
//         reg61 have to be programmed at last and after video stable input.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static void
// SetupAFE(BYTE ucFreqInMHz)
SetupAFE(VIDEOPCLKLEVEL level)
{
    BYTE uc ;
    // @emily turn off reg61 before SetupAFE parameters.
    HDMITX_WriteI2C_Byte_66121(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST);/* 0x10 */
    // HDMITX_WriteI2C_Byte_66121(REG_TX_AFE_DRV_CTRL,0x3);
    HDMITX_DEBUG_PRINTF(("SetupAFE()\n"));

    //TMDS Clock < 80MHz    TMDS Clock > 80MHz
    //Reg61    0x03    0x03

    //Reg62    0x18    0x88
    //Reg63    Default    Default
    //Reg64    0x08    0x80
    //Reg65    Default    Default
    //Reg66    Default    Default
    //Reg67    Default    Default
/*    uc = HDMITX_ReadI2C_Byte_66121(REG_TX_CLK_CTRL1);

#ifdef FALLING_EDGE_TRIGGER
    uc |= B_VDO_LATCH_EDGE ;
#else
    uc &= ~B_VDO_LATCH_EDGE ;
#endif
    HDMITX_WriteI2C_Byte_66121(REG_TX_CLK_CTRL1, uc);*/
    switch(level)
    {
    case PCLK_HIGH:
		HDMITX_SetI2C_Byte(0x62, 0x90, 0x80);
        HDMITX_SetI2C_Byte(0x64, 0x89, 0x80);
        HDMITX_SetI2C_Byte(0x68, 0x10, 0x80);
		HDMITX_DEBUG_PRINTF(("hdmitx_SetupAFE()===================HIGHT\n"));
        break ;
    default:
		HDMITX_SetI2C_Byte(0x62, 0x90, 0x10);
        HDMITX_SetI2C_Byte(0x64, 0x89, 0x09);
        HDMITX_SetI2C_Byte(0x68, 0x10, 0x10);
		HDMITX_DEBUG_PRINTF(("hdmitx_SetupAFE()===================LOW\n"));
        break ;
    }
    // 2009/01/15 modified by Jau-Chih.Tseng@ite.com.tw
    //uc = HDMITX_ReadI2C_Byte_66121(REG_TX_SW_RST);
    //uc &= ~(B_REF_RST_HDMITX|B_VID_RST_HDMITX);
    //HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST,uc);
	//delay1ms(1);
	HDMITX_SetI2C_Byte(REG_TX_SW_RST,B_REF_RST_HDMITX|B_VID_RST_HDMITX,0);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AFE_DRV_CTRL,0);
    delay1ms(1);
    // HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST,uc|B_VID_RST_HDMITX);
    // delay1ms(100);
    // HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST,uc);
    //~Jau-Chih.Tseng@ite.com.tw


}


//////////////////////////////////////////////////////////////////////
// Function: FireAFE
// Parameter: N/A
// Return: N/A
// Remark: write reg61 with 0x04
//         When program reg61 with 0x04,then audio and video circuit work.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////
static void
FireAFE()
{
    BYTE reg;
    Switch_HDMITX_Bank_66121(0);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AFE_DRV_CTRL,0);

    //for(reg = 0x61 ; reg <= 0x67 ; reg++)
    //{
    //    HDMITX_DEBUG_PRINTF(("Reg[%02X] = %02X\n",(int)reg,(int)HDMITX_ReadI2C_Byte_66121(reg)));
    //}
}

//////////////////////////////////////////////////////////////////////
// Audio Output
//////////////////////////////////////////////////////////////////////
#if 0
//////////////////////////////////////////////////////////////////////
// Function: SetAudioFormat
// Parameter:
//    NumChannel - number of channel,from 1 to 8
//    AudioEnable - Audio source and type bit field,value of bit field are
//        ENABLE_SPDIF    (1<<4)
//        ENABLE_I2S_SRC3  (1<<3)
//        ENABLE_I2S_SRC2  (1<<2)
//        ENABLE_I2S_SRC1  (1<<1)
//        ENABLE_I2S_SRC0  (1<<0)
//    SampleFreq - the audio sample frequence in Hz
//    AudSWL - Audio sample width,only support 16,18,20,or 24.
//    AudioCatCode - The audio channel catalogy code defined in IEC 60958-3
// Return: ER_SUCCESS if done,ER_FAIL for otherwise.
// Remark: program audio channel control register and audio channel registers
//         to enable audio by input.
// Side-Effect: register bank will keep in bank zero.
//////////////////////////////////////////////////////////////////////


static SYS_STATUS
SetAudioFormat(BYTE NumChannel,BYTE AudioEnable,BYTE bSampleFreq,BYTE AudSWL,BYTE AudioCatCode)
{
    BYTE fs = bSampleFreq ;
    BYTE SWL ;

    BYTE SourceValid ;
    BYTE SoruceNum ;


//    HDMITX_DEBUG_PRINTF(("SetAudioFormat(%d channel,%02X,SampleFreq %d,AudSWL %d,%02X)\n",(int)NumChannel,(int)AudioEnable,(int)bSampleFreq,(int)AudSWL,(int)AudioCatCode));


    Instance[0].bOutputAudioMode |= 0x41 ;
    if(NumChannel > 6)
    {
        SourceValid = B_AUD_ERR2FLAT | B_AUD_S3VALID | B_AUD_S2VALID | B_AUD_S1VALID ;
        SoruceNum = 4 ;
    }
    else if (NumChannel > 4)
    {
        SourceValid = B_AUD_ERR2FLAT | B_AUD_S2VALID | B_AUD_S1VALID ;
        SoruceNum = 3 ;
    }
    else if (NumChannel > 2)
    {
        SourceValid = B_AUD_ERR2FLAT | B_AUD_S1VALID ;
        SoruceNum = 2 ;
    }
    else
    {
        SourceValid = B_AUD_ERR2FLAT ; // only two channel.
        SoruceNum = 1 ;
        Instance[0].bOutputAudioMode &= ~0x40 ;
    }

    AudioEnable &= ~ (M_AUD_SWL|B_SPDIFTC);

    switch(AudSWL)
    {
    case 16:
        SWL = AUD_SWL_16 ;
        AudioEnable |= M_AUD_16BIT ;
        break ;
    case 18:
        SWL = AUD_SWL_18 ;
        AudioEnable |= M_AUD_18BIT ;
        break ;
    case 20:
        SWL = AUD_SWL_20 ;
        AudioEnable |= M_AUD_20BIT ;
        break ;
    case 24:
        SWL = AUD_SWL_24 ;
        AudioEnable |= M_AUD_24BIT ;
        break ;
    default:
        return ER_FAIL ;
    }


    Switch_HDMITX_Bank_66121(0);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL0,AudioEnable&0xF0);

    HDMITX_AndREG_Byte_66121(REG_TX_SW_RST,~(B_AUD_RST_HDMITX|B_AREF_RST));
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL1,Instance[0].bOutputAudioMode); // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_FIFOMAP,0xE4); // default mapping.
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL3,(Instance[0].bAudioChannelSwap&0xF)|(AudioEnable&B_AUD_SPDIF));
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUD_SRCVALID_FLAT,SourceValid);

    // suggested to be 0x41

//     Switch_HDMITX_Bank_66121(1);
//     HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_MODE,0 |((NumChannel == 1)?1:0)); // 2 audio channel without pre-emphasis,if NumChannel set it as 1.
//     HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_CAT,AudioCatCode);
//     HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_SRCNUM,SoruceNum);
//     HDMITX_WriteI2C_Byte_66121(REG_TX_AUD0CHST_CHTNUM,0x21);
//     HDMITX_WriteI2C_Byte_66121(REG_TX_AUD1CHST_CHTNUM,0x43);
//     HDMITX_WriteI2C_Byte_66121(REG_TX_AUD2CHST_CHTNUM,0x65);
//     HDMITX_WriteI2C_Byte_66121(REG_TX_AUD3CHST_CHTNUM,0x87);
//     HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_CA_FS,0x00|fs); // choose clock
//     fs = ~fs ; // OFS is the one's complement of FS
//     HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_OFS_WL,(fs<<4)|SWL);
//     Switch_HDMITX_Bank_66121(0);

    Switch_HDMITX_Bank_66121(1);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_MODE,0 |((NumChannel == 1)?1:0)); // 2 audio channel without pre-emphasis,if NumChannel set it as 1.
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_CAT,AudioCatCode);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_SRCNUM,SoruceNum);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUD0CHST_CHTNUM,0);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_CA_FS,0x00|fs); // choose clock
    fs = ~fs ; // OFS is the one's complement of FS
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_OFS_WL,(fs<<4)|SWL);
    Switch_HDMITX_Bank_66121(0);

    if(!(AudioEnable | B_AUD_SPDIF))
    {
        HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL0,AudioEnable);
    }

    Instance[0].bAudioChannelEnable = AudioEnable ;

    // HDMITX_AndREG_Byte_66121(REG_TX_SW_RST,B_AUD_RST_HDMITX);    // enable Audio
    return ER_SUCCESS;
}
#endif


static void
AutoAdjustAudio()
{
    unsigned long SampleFreq,cTMDSClock ;
    unsigned long N ;
    ULONG aCTS=0;
    BYTE fs, uc,LoopCnt=10;
    if(bForceCTS)
    {
        Switch_HDMITX_Bank_66121(0);
        HDMITX_WriteI2C_Byte_66121(0xF8, 0xC3) ;
        HDMITX_WriteI2C_Byte_66121(0xF8, 0xA5) ;
        HDMITX_AndREG_Byte_66121(REG_TX_PKT_SINGLE_CTRL,~B_SW_CTS) ; // D[1] = 0, HW auto count CTS
        HDMITX_WriteI2C_Byte_66121(0xF8, 0xFF) ;
    }
    //delay1ms(50);
    Switch_HDMITX_Bank_66121(1);
    N = ((unsigned long)HDMITX_ReadI2C_Byte_66121(REGPktAudN2)&0xF) << 16 ;
    N |= ((unsigned long)HDMITX_ReadI2C_Byte_66121(REGPktAudN1)) <<8 ;
    N |= ((unsigned long)HDMITX_ReadI2C_Byte_66121(REGPktAudN0));

    while(LoopCnt--)
    {   ULONG TempCTS=0;
        aCTS = ((unsigned long)HDMITX_ReadI2C_Byte_66121(REGPktAudCTSCnt2)) << 12 ;
        aCTS |= ((unsigned long)HDMITX_ReadI2C_Byte_66121(REGPktAudCTSCnt1)) <<4 ;
        aCTS |= ((unsigned long)HDMITX_ReadI2C_Byte_66121(REGPktAudCTSCnt0)&0xf0)>>4  ;
        if(aCTS==TempCTS)
        {break;}
        TempCTS=aCTS;
    }
    Switch_HDMITX_Bank_66121(0);
    if( aCTS == 0)
    {
        HDMITX_DEBUG_PRINTF(("aCTS== 0"));
        return;
    }

    uc = HDMITX_ReadI2C_Byte_66121(0xc1);

    cTMDSClock = Instance[0].TMDSClock ;
    //TMDSClock=GetInputPclk();
    HDMITX_DEBUG_PRINTF(("PCLK = %u0,000\n",(WORD)(cTMDSClock/10000)));
    switch(uc & 0x70)
    {
    case 0x50:
        cTMDSClock *= 5 ;
        cTMDSClock /= 4 ;
        break ;
    case 0x60:
        cTMDSClock *= 3 ;
        cTMDSClock /= 2 ;
    }
    SampleFreq = cTMDSClock/aCTS ;
    SampleFreq *= N ;
    SampleFreq /= 128 ;
    //SampleFreq=48000;

    HDMITX_DEBUG_PRINTF(("SampleFreq = %u0\n",(WORD)(SampleFreq/10)));
    if( SampleFreq>31000L && SampleFreq<=38050L ){fs = AUDFS_32KHz ;}
    else if (SampleFreq < 46550L )  {fs = AUDFS_44p1KHz ;}//46050
    else if (SampleFreq < 68100L )  {fs = AUDFS_48KHz ;}
    else if (SampleFreq < 92100L )  {fs = AUDFS_88p2KHz ;}
    else if (SampleFreq < 136200L ) {fs = AUDFS_96KHz ;}
    else if (SampleFreq < 184200L ) {fs = AUDFS_176p4KHz ;}
    else if (SampleFreq < 240200L ) {fs = AUDFS_192KHz ;}
    else if (SampleFreq < 800000L ) {fs = AUDFS_768KHz ;}
    else
    {
        fs = AUDFS_OTHER;
#ifdef Debug_message
        HDMITX_DEBUG_PRINTF(("fs = AUDFS_OTHER\n"));
#endif
    }
    if(Instance[0].bAudFs != fs)
    {
        Instance[0].bAudFs=fs;
        SetNCTS(Instance[0].TMDSClock,Instance[0].bAudFs); // set N, CTS by new generated clock.
        //CurrCTS=0;
        return;
    }
    return;
}


static void
SetupAudioChannel()
{
    static BYTE bEnableAudioChannel=FALSE ;
    BYTE uc ;
    if( (HDMITX_ReadI2C_Byte_66121(REG_TX_SW_RST) & (B_AUD_RST_HDMITX|B_AREF_RST)) == 0) // audio enabled
    {
        Switch_HDMITX_Bank_66121(0);
        uc =HDMITX_ReadI2C_Byte_66121(REG_TX_AUDIO_CTRL0);
        if((uc & 0x1f) == 0x10)
        {
            if(HDMITX_ReadI2C_Byte_66121(REG_TX_CLK_STATUS2) & B_OSF_LOCK)
            {
                SetNCTS(Instance[0].TMDSClock, Instance[0].bAudFs); // to enable automatic progress setting for N/CTS
                delay1ms(5);
                AutoAdjustAudio();
                Switch_HDMITX_Bank_66121(0);
                HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL0, Instance[0].bAudioChannelEnable);
                bEnableAudioChannel=TRUE ;
            }
        }
        else if((uc & 0xF) == 0x00 )
        {
            SetNCTS(Instance[0].TMDSClock, Instance[0].bAudFs); // to enable automatic progress setting for N/CTS
            delay1ms(5);
            AutoAdjustAudio();
            Switch_HDMITX_Bank_66121(0);
            HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL0, Instance[0].bAudioChannelEnable);
            bEnableAudioChannel=TRUE ;
        }
        else
        {
            /*
            if((HDMITX_ReadI2C_Byte_66121(REG_TX_CLK_STATUS2) & B_OSF_LOCK)==0)
            {
                // AutoAdjustAudio();
                // ForceSetNCTS(CurrentPCLK, CurrentSampleFreq);
                if( bEnableAudioChannel == TRUE )
                {
                    Switch_HDMITX_Bank_66121(0);
                    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL0, Instance[0].bAudioChannelEnable&0xF0);
                }
                bEnableAudioChannel=FALSE ;
            }
            */
        }
    }
}
//////////////////////////////////////////////////////////////////////
// Function: SetNCTS
// Parameter: PCLK - video clock in Hz.
//            Fs - Encoded audio sample rate
//                          AUDFS_22p05KHz  4
//                          AUDFS_44p1KHz 0
//                          AUDFS_88p2KHz 8
//                          AUDFS_176p4KHz    12
//
//                          AUDFS_24KHz  6
//                          AUDFS_48KHz  2
//                          AUDFS_96KHz  10
//                          AUDFS_192KHz 14
//
//                          AUDFS_768KHz 9
//
//                          AUDFS_32KHz  3
//                          AUDFS_OTHER    1

// Return: ER_SUCCESS if success
// Remark: set N value,the CTS will be auto generated by HW.
// Side-Effect: register bank will reset to bank 0.
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetNCTS(ULONG PCLK,BYTE Fs)
{
    ULONG n,SampleFreq;
    BYTE LoopCnt=255,CTSStableCnt=0;
    ULONG diff;
    ULONG CTS=0,LastCTS=0;
    BOOL HBR_mode;
    BYTE aVIC;
    if(B_HBR & HDMITX_ReadI2C_Byte_66121(REG_TX_AUD_HDAUDIO))
    {
        HBR_mode=TRUE;
    }
    else
    {
        HBR_mode=FALSE;
    }

    Switch_HDMITX_Bank_66121(1);
    aVIC = (HDMITX_ReadI2C_Byte_66121(REG_TX_AVIINFO_DB4)&0x7f);
    Switch_HDMITX_Bank_66121(0);

    if(aVIC)
    {
        switch(Fs)
        {
        case AUDFS_32KHz: n = 4096; break;
        case AUDFS_44p1KHz: n = 6272; break;
        case AUDFS_48KHz: n = 6144; break;
        case AUDFS_88p2KHz: n = 12544; break;
        case AUDFS_96KHz: n = 12288; break;
        case AUDFS_176p4KHz: n = 25088; break;
        case AUDFS_192KHz: n = 24576; break;
        default: n = 6144;
        }
    }
    else
    {
        switch(Fs)
        {
            case AUDFS_32KHz: SampleFreq = 32000L; break;
            case AUDFS_44p1KHz: SampleFreq = 44100L; break;
            case AUDFS_48KHz: SampleFreq = 48000L; break;
            case AUDFS_88p2KHz: SampleFreq = 88200L; break;
            case AUDFS_96KHz: SampleFreq = 96000L; break;
            case AUDFS_176p4KHz: SampleFreq = 176000L; break;
            case AUDFS_192KHz: SampleFreq = 192000L; break;
            default: SampleFreq = 768000L;
        }
        n = SampleFreq * 128 ; // MCLK = fs * 256 ;
        n /= 1000;
    }
    // tr_printf((" n = %ld\n",n)) ;
    Switch_HDMITX_Bank_66121(1) ;
    HDMITX_WriteI2C_Byte_66121(REGPktAudN0,(BYTE)((n)&0xFF)) ;
    HDMITX_WriteI2C_Byte_66121(REGPktAudN1,(BYTE)((n>>8)&0xFF)) ;
    HDMITX_WriteI2C_Byte_66121(REGPktAudN2,(BYTE)((n>>16)&0xF)) ;
    if(bForceCTS)
    {
        ULONG SumCTS=0;
        while(LoopCnt--)
        {
            delay1ms(30);
            CTS = ((unsigned long)HDMITX_ReadI2C_Byte_66121(REGPktAudCTSCnt2)) << 12 ;
            CTS |= ((unsigned long)HDMITX_ReadI2C_Byte_66121(REGPktAudCTSCnt1)) <<4 ;
            CTS |= ((unsigned long)HDMITX_ReadI2C_Byte_66121(REGPktAudCTSCnt0)&0xf0)>>4  ;
            if( CTS == 0)
            {
                continue;
            }
            else
            {
                if(LastCTS>CTS )
                    {diff=LastCTS-CTS;}
                else
                    {diff=CTS-LastCTS;}
                //HDMITX_DEBUG_PRINTF(("LastCTS= %u%u",(WORD)(LastCTS/10000),(WORD)(LastCTS%10000)));
                //HDMITX_DEBUG_PRINTF(("       CTS= %u%u\n",(WORD)(CTS/10000),(WORD)(CTS%10000)));
                LastCTS=CTS;
                if(5>diff)
                {
                    CTSStableCnt++;
                    SumCTS+=CTS;
                }
                else
                {
                    CTSStableCnt=0;
                    SumCTS=0;
                    continue;
                }
                if(CTSStableCnt>=32)
                {
                    LastCTS=(SumCTS>>5);
                    break;
                }
            }
        }
    }
    HDMITX_WriteI2C_Byte_66121(REGPktAudCTS0,(BYTE)((LastCTS)&0xFF)) ;
    HDMITX_WriteI2C_Byte_66121(REGPktAudCTS1,(BYTE)((LastCTS>>8)&0xFF)) ;
    HDMITX_WriteI2C_Byte_66121(REGPktAudCTS2,(BYTE)((LastCTS>>16)&0xF)) ;
    Switch_HDMITX_Bank_66121(0) ;
#ifdef Force_CTS
    bForceCTS = TRUE;
#endif
    HDMITX_WriteI2C_Byte_66121(0xF8, 0xC3) ;
    HDMITX_WriteI2C_Byte_66121(0xF8, 0xA5) ;
    if(bForceCTS)
    {
        HDMITX_OrREG_Byte_66121(REG_TX_PKT_SINGLE_CTRL,B_SW_CTS) ; // D[1] = 0, HW auto count CTS
    }
    else
    {
        HDMITX_AndREG_Byte_66121(REG_TX_PKT_SINGLE_CTRL,~B_SW_CTS) ; // D[1] = 0, HW auto count CTS
    }
    HDMITX_WriteI2C_Byte_66121(0xF8, 0xFF) ;

    if(FALSE==HBR_mode) //LPCM
    {
        BYTE uData;
        Switch_HDMITX_Bank_66121(1);
        HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_CA_FS,0x00|Fs);
        Fs = ~Fs ; // OFS is the one's complement of FS
        uData = (0x0f&HDMITX_ReadI2C_Byte_66121(REG_TX_AUDCHST_OFS_WL));
        HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_OFS_WL,(Fs<<4)|uData);
        Switch_HDMITX_Bank_66121(0);
    }

    return ER_SUCCESS ;
}


//////////////////////////////////////////////////////////////////////
// DDC Function.
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Function: ClearDDCFIFO
// Parameter: N/A
// Return: N/A
// Remark: clear the DDC FIFO.
// Side-Effect: DDC master will set to be HOST.
//////////////////////////////////////////////////////////////////////

static void
ClearDDCFIFO()
{
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_CMD,CMD_FIFO_CLR);
}

static void
GenerateDDCSCLK()
{
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_CMD,CMD_GEN_SCLCLK);
}
//////////////////////////////////////////////////////////////////////
// Function: AbortDDC
// Parameter: N/A
// Return: N/A
// Remark: Force abort DDC and reset DDC bus.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static void
AbortDDC()
{
    BYTE CPDesire,SWReset,DDCMaster ;
    BYTE uc, timeout, i ;
    // save the SW reset,DDC master,and CP Desire setting.
    //HDMITX_OrREG_Byte_66121(REG_TX_INT_CTRL,(1<<1));
    SWReset = HDMITX_ReadI2C_Byte_66121(REG_TX_SW_RST);
    CPDesire = HDMITX_ReadI2C_Byte_66121(REG_TX_HDCP_DESIRE);
    DDCMaster = HDMITX_ReadI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL);


    //HDMITX_WriteI2C_Byte_66121(REG_TX_HDCP_DESIRE,CPDesire&(~B_CPDESIRE)); // @emily change order
    HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST,SWReset|B_HDCP_RST_HDMITX);         // @emily change order
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);

    // 2009/01/15 modified by Jau-Chih.Tseng@ite.com.tw
    // do abort DDC twice.
    for( i = 0 ; i < 2 ; i++ )
    {
        HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_CMD,CMD_DDC_ABORT);

        for( timeout = 0 ; timeout < 200 ; timeout++ )
        {
            uc = HDMITX_ReadI2C_Byte_66121(REG_TX_DDC_STATUS);
            if (uc&B_DDC_DONE)
            {
                break ; // success
            }

            if( uc & (B_DDC_NOACK|B_DDC_WAITBUS|B_DDC_ARBILOSE) )
            {
//                HDMITX_DEBUG_PRINTF(("AbortDDC Fail by reg16=%02X\n",(int)uc));
                break ;
            }
            delay1ms(1); // delay 1 ms to stable.
        }
    }
    //~Jau-Chih.Tseng@ite.com.tw


    // 2009/01/15 modified by Jau-Chih.Tseng@ite.com.tw
    //// restore the SW reset,DDC master,and CP Desire setting.
    //HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST,SWReset);
    //HDMITX_WriteI2C_Byte_66121(REG_TX_HDCP_DESIRE,CPDesire);
    //HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL,DDCMaster);
    //~Jau-Chih.Tseng@ite.com.tw
    //HDMITX_AndReg_Byte_66121(REG_TX_INT_CTRL,~(1<<1));
}

//////////////////////////////////////////////////////////////////////
// Packet and InfoFrame
//////////////////////////////////////////////////////////////////////

// ////////////////////////////////////////////////////////////////////////////////
// // Function: SetAVMute_66121()
// // Parameter: N/A
// // Return: N/A
// // Remark: set AVMute as TRUE and enable GCP sending.
// // Side-Effect: N/A
// ////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// void
// SetAVMute_66121()
// {
//     Switch_HDMITX_Bank_66121(0);
//     HDMITX_WriteI2C_Byte_66121(REG_TX_GCP,B_SET_AVMUTE);
//     HDMITX_WriteI2C_Byte_66121(REG_TX_PKT_GENERAL_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
// }

// ////////////////////////////////////////////////////////////////////////////////
// // Function: SetAVMute_66121(FALSE)
// // Parameter: N/A
// // Return: N/A
// // Remark: clear AVMute as TRUE and enable GCP sending.
// // Side-Effect: N/A
// ////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// void
// SetAVMute_66121(FALSE)
// {
//     Switch_HDMITX_Bank_66121(0);
//     HDMITX_WriteI2C_Byte_66121(REG_TX_GCP,B_CLR_AVMUTE);
//     HDMITX_WriteI2C_Byte_66121(REG_TX_PKT_GENERAL_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
// }



//////////////////////////////////////////////////////////////////////
// Function: ReadEDID_66121
// Parameter: pData - the pointer of buffer to receive EDID ucdata.
//            bSegment - the segment of EDID readback.
//            offset - the offset of EDID ucdata in the segment. in byte.
//            count - the read back bytes count,cannot exceed 32
// Return: ER_SUCCESS if successfully getting EDID. ER_FAIL otherwise.
// Remark: function for read EDID ucdata from reciever.
// Side-Effect: DDC master will set to be HOST. DDC FIFO will be used and dirty.
//////////////////////////////////////////////////////////////////////

SYS_STATUS
ReadEDID_66121(BYTE *pData,BYTE bSegment,BYTE offset,SHORT Count)
{
    SHORT RemainedCount,ReqCount ;
    BYTE bCurrOffset ;
    SHORT TimeOut ;
    BYTE *pBuff = pData ;
    BYTE ucdata ;

    // HDMITX_DEBUG_PRINTF(("ReadEDID_66121(%08lX,%d,%d,%d)\n",(ULONG)pData,(int)bSegment,(int)offset,(int)Count));
    if(!pData)
    {
//        HDMITX_DEBUG_PRINTF(("ReadEDID_66121(): Invallid pData pointer %08lX\n",(ULONG)pData));
        return ER_FAIL ;
    }

    if(HDMITX_ReadI2C_Byte_66121(REG_TX_INT_STAT1) & B_INT_DDC_BUS_HANG)
    {
        HDMITX_DEBUG_PRINTF(("Called AboutDDC()\n"));
        AbortDDC();

    }
    HDMITX_OrREG_Byte_66121(REG_TX_INT_CTRL,(1<<1));

    ClearDDCFIFO();

    RemainedCount = Count ;
    bCurrOffset = offset ;

    Switch_HDMITX_Bank_66121(0);

    while(RemainedCount > 0)
    {

        ReqCount = (RemainedCount > DDC_FIFO_MAXREQ)?DDC_FIFO_MAXREQ:RemainedCount ;
        // HDMITX_DEBUG_PRINTF(("ReadEDID_66121(): ReqCount = %d,bCurrOffset = %d\n",(int)ReqCount,(int)bCurrOffset));

        HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
        HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_CMD,CMD_FIFO_CLR);

        for(TimeOut = 0 ; TimeOut < 200 ; TimeOut++)
        {
            ucdata = HDMITX_ReadI2C_Byte_66121(REG_TX_DDC_STATUS);

            if(ucdata&B_DDC_DONE)
            {
                break ;
            }

            if((ucdata & B_DDC_ERROR)||(HDMITX_ReadI2C_Byte_66121(REG_TX_INT_STAT1) & B_INT_DDC_BUS_HANG))
            {
                HDMITX_DEBUG_PRINTF(("Called AboutDDC()\n"));
                AbortDDC();
                return ER_FAIL ;
            }
        }

        HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
        HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_HEADER,DDC_EDID_ADDRESS); // for EDID ucdata get
        HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_REQOFF,bCurrOffset);
        HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_REQCOUNT,(BYTE)ReqCount);
        HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_EDIDSEG,bSegment);
        HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_CMD,CMD_EDID_READ);

        bCurrOffset += ReqCount ;
        RemainedCount -= ReqCount ;

        for(TimeOut = 250 ; TimeOut > 0 ; TimeOut --)
        {
            delay1ms(1);
            ucdata = HDMITX_ReadI2C_Byte_66121(REG_TX_DDC_STATUS);
            if(ucdata & B_DDC_DONE)
            {
                break ;
            }

            if(ucdata & B_DDC_ERROR)
            {
                HDMITX_DEBUG_PRINTF(("ReadEDID_66121(): DDC_STATUS = %02X,fail.\n",(int)ucdata));
                HDMITX_AndReg_Byte_66121(REG_TX_INT_CTRL,~(1<<1));
                return ER_FAIL ;
            }
        }

        if(TimeOut == 0)
        {
            HDMITX_DEBUG_PRINTF(("ReadEDID_66121(): DDC TimeOut. \n",(int)ucdata));
            HDMITX_AndReg_Byte_66121(REG_TX_INT_CTRL,~(1<<1));
            return ER_FAIL ;
        }

        do
        {
            *(pBuff++) = HDMITX_ReadI2C_Byte_66121(REG_TX_DDC_READFIFO);
            ReqCount -- ;
        }while(ReqCount > 0);

    }
    HDMITX_AndReg_Byte_66121(REG_TX_INT_CTRL,~(1<<1));
    return ER_SUCCESS ;
}



#ifdef SUPPORT_HDCP
//////////////////////////////////////////////////////////////////////
// Authentication
//////////////////////////////////////////////////////////////////////
static void
HDCP_ClearAuthInterrupt()
{
    BYTE uc ;
    uc = HDMITX_ReadI2C_Byte_66121(REG_TX_INT_MASK2) & (~(B_KSVLISTCHK_MASK|B_T_AUTH_DONE_MASK|B_AUTH_FAIL_MASK));
    HDMITX_WriteI2C_Byte_66121(REG_TX_INT_CLR0,B_CLR_AUTH_FAIL|B_CLR_AUTH_DONE|B_CLR_KSVLISTCHK);
    HDMITX_WriteI2C_Byte_66121(REG_TX_INT_CLR1,0);
    HDMITX_WriteI2C_Byte_66121(REG_TX_SYS_STATUS,B_INTACTDONE);
}

static void
HDCP_ResetAuth()
{
    HDMITX_WriteI2C_Byte_66121(REG_TX_LISTCTRL,0);
    HDMITX_WriteI2C_Byte_66121(REG_TX_HDCP_DESIRE,0);
    HDMITX_OrREG_Byte_66121(REG_TX_SW_RST,B_HDCP_RST_HDMITX);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
    HDCP_ClearAuthInterrupt();
    AbortDDC();
}
//////////////////////////////////////////////////////////////////////
// Function: HDCP_EnableEncryption
// Parameter: N/A
// Return: ER_SUCCESS if done.
// Remark: Set regC1 as zero to enable continue authentication.
// Side-Effect: register bank will reset to zero.
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
HDCP_EnableEncryption()
{
    Switch_HDMITX_Bank_66121(0);
    return HDMITX_WriteI2C_Byte_66121(REG_TX_ENCRYPTION,B_ENABLE_ENCRYPTION);
}


//////////////////////////////////////////////////////////////////////
// Function: HDCP_Auth_Fire()
// Parameter: N/A
// Return: N/A
// Remark: write anything to reg21 to enable HDCP authentication by HW
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static void
HDCP_Auth_Fire()
{
    // HDMITX_DEBUG_PRINTF(("HDCP_Auth_Fire():\n"));
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHDCP); // MASTERHDCP,no need command but fire.
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUTHFIRE,1);
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_StartAnCipher
// Parameter: N/A
// Return: N/A
// Remark: Start the Cipher to free run for random number. When stop,An is
//         ready in Reg30.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static void
HDCP_StartAnCipher()
{
    HDMITX_WriteI2C_Byte_66121(REG_TX_AN_GENERATE,B_START_CIPHER_GEN);
    delay1ms(1); // delay 1 ms
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_StopAnCipher
// Parameter: N/A
// Return: N/A
// Remark: Stop the Cipher,and An is ready in Reg30.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static void
HDCP_StopAnCipher()
{
    HDMITX_WriteI2C_Byte_66121(REG_TX_AN_GENERATE,B_STOP_CIPHER_GEN);
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_GenerateAn
// Parameter: N/A
// Return: N/A
// Remark: start An ciper random run at first,then stop it. Software can get
//         an in reg30~reg38,the write to reg28~2F
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static void
HDCP_GenerateAn()
{
    BYTE Data[8] ;
#if 1
    HDCP_StartAnCipher();
    // HDMITX_WriteI2C_Byte_66121(REG_TX_AN_GENERATE,B_START_CIPHER_GEN);
    // delay1ms(1); // delay 1 ms
    // HDMITX_WriteI2C_Byte_66121(REG_TX_AN_GENERATE,B_STOP_CIPHER_GEN);

    HDCP_StopAnCipher();

    Switch_HDMITX_Bank_66121(0);
    // new An is ready in reg30
    HDMITX_ReadI2C_ByteN_66121(REG_TX_AN_GEN,Data,8);
#else
    Data[0] = 0 ;Data[1] = 0 ;Data[2] = 0 ;Data[3] = 0 ;
    Data[4] = 0 ;Data[5] = 0 ;Data[6] = 0 ;Data[7] = 0 ;
#endif
    HDMITX_WriteI2C_ByteN_66121(REG_TX_AN,Data,8);

}


//////////////////////////////////////////////////////////////////////
// Function: HDCP_GetBCaps
// Parameter: pBCaps - pointer of byte to get BCaps.
//            pBStatus - pointer of two bytes to get BStatus
// Return: ER_SUCCESS if successfully got BCaps and BStatus.
// Remark: get B status and capability from HDCP reciever via DDC bus.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
HDCP_GetBCaps(PBYTE pBCaps ,unsigned int *pBStatus)
{
    BYTE ucdata ;
    BYTE TimeOut ;

    Switch_HDMITX_Bank_66121(0);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_HEADER,DDC_HDCP_ADDRESS);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_REQOFF,0x40); // BCaps offset
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_REQCOUNT,3);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD);

    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {
        delay1ms(1);

        ucdata = HDMITX_ReadI2C_Byte_66121(REG_TX_DDC_STATUS);
        if(ucdata & B_DDC_DONE)
        {
            //HDMITX_DEBUG_PRINTF(("HDCP_GetBCaps(): DDC Done.\n"));
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
//            HDMITX_DEBUG_PRINTF(("HDCP_GetBCaps(): DDC fail by reg16=%02X.\n",ucdata));
            return ER_FAIL ;
        }
    }

    if(TimeOut == 0)
    {
        return ER_FAIL ;
    }

    ucdata = HDMITX_ReadI2C_Byte_66121(REG_TX_BSTAT+1);
    *pBStatus = (unsigned int)ucdata ;
    *pBStatus <<= 8 ;
    ucdata = HDMITX_ReadI2C_Byte_66121(REG_TX_BSTAT);
    *pBStatus |= ((unsigned int)ucdata&0xFF) ;
    *pBCaps = HDMITX_ReadI2C_Byte_66121(REG_TX_BCAP);
    return ER_SUCCESS ;

}


//////////////////////////////////////////////////////////////////////
// Function: HDCP_GetBKSV
// Parameter: pBKSV - pointer of 5 bytes buffer for getting BKSV
// Return: ER_SUCCESS if successfuly got BKSV from Rx.
// Remark: Get BKSV from HDCP reciever.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
HDCP_GetBKSV(BYTE *pBKSV)
{
    BYTE ucdata ;
    BYTE TimeOut ;

    Switch_HDMITX_Bank_66121(0);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_HEADER,DDC_HDCP_ADDRESS);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_REQOFF,0x00); // BKSV offset
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_REQCOUNT,5);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD);

    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {
        delay1ms(1);

        ucdata = HDMITX_ReadI2C_Byte_66121(REG_TX_DDC_STATUS);
        if(ucdata & B_DDC_DONE)
        {
            HDMITX_DEBUG_PRINTF(("HDCP_GetBCaps(): DDC Done.\n"));
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
            HDMITX_DEBUG_PRINTF(("HDCP_GetBCaps(): DDC No ack or arbilose,%x,maybe cable did not connected. Fail.\n",ucdata));
            return ER_FAIL ;
        }
    }

    if(TimeOut == 0)
    {
        return ER_FAIL ;
    }

    HDMITX_ReadI2C_ByteN_66121(REG_TX_BKSV,(PBYTE)pBKSV,5);

    return ER_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////
// Function:HDCP_Authenticate
// Parameter: N/A
// Return: ER_SUCCESS if Authenticated without error.
// Remark: do Authentication with Rx
// Side-Effect:
//  1. Instance[0].bAuthenticated global variable will be TRUE when authenticated.
//  2. Auth_done interrupt and AUTH_FAIL interrupt will be enabled.
//////////////////////////////////////////////////////////////////////
static BYTE
countbit(BYTE b)
{
    BYTE i,count ;
    for( i = 0, count = 0 ; i < 8 ; i++ )
    {
        if( b & (1<<i) )
        {
            count++ ;
        }
    }
    return count ;
}

static void
HDCP_Reset()
{
    BYTE uc ;
    uc = HDMITX_ReadI2C_Byte_66121(REG_TX_SW_RST) | B_HDCP_RST_HDMITX ;
    HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST,uc);
    HDMITX_WriteI2C_Byte_66121(REG_TX_HDCP_DESIRE,0);
    HDMITX_WriteI2C_Byte_66121(REG_TX_LISTCTRL,0);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL,B_MASTERHOST);
    ClearDDCFIFO();
    AbortDDC();
}

static SYS_STATUS
HDCP_Authenticate()
{
    BYTE ucdata ;
    BYTE BCaps ;
    unsigned int BStatus ;
    unsigned int TimeOut ;

    BYTE revoked = FALSE ;
    BYTE BKSV[5] ;

    Instance[0].bAuthenticated = FALSE ;

    // Authenticate should be called after AFE setup up.

    HDMITX_DEBUG_PRINTF(("HDCP_Authenticate():\n"));
    HDCP_Reset();
    // ClearDDCFIFO();
    // AbortDDC();

    Switch_HDMITX_Bank_66121(0);

    for( TimeOut = 0 ; TimeOut < 20 ; TimeOut++ )
    {
        delay1ms(15);

        if(HDCP_GetBCaps(&BCaps,&BStatus) != ER_SUCCESS)
        {
            HDMITX_DEBUG_PRINTF(("HDCP_GetBCaps fail.\n"));
            return ER_FAIL ;
        }

        if(B_TX_HDMI_MODE == (HDMITX_ReadI2C_Byte_66121(REG_TX_HDMI_MODE) & B_TX_HDMI_MODE ))
        {
            if((BStatus & B_CAP_HDMI_MODE)==B_CAP_HDMI_MODE)
            {
                break;
            }
        }
        else
        {
            if((BStatus & B_CAP_HDMI_MODE)!=B_CAP_HDMI_MODE)
            {
                break;
            }
        }
    }

//    HDMITX_DEBUG_PRINTF(("BCaps = %02X BStatus = %04X\n",(int)BCaps,(int)BStatus));
    /*
    if((BStatus & M_DOWNSTREAM_COUNT)> 6)
    {
        HDMITX_DEBUG_PRINTF(("Down Stream Count %d is over maximum supported number 6,fail.\n",(int)(BStatus & M_DOWNSTREAM_COUNT)));
        return ER_FAIL ;
    }
    */

    HDCP_GetBKSV(BKSV);
//    HDMITX_DEBUG_PRINTF(("BKSV %02X %02X %02X %02X %02X\n",(int)BKSV[0],(int)BKSV[1],(int)BKSV[2],(int)BKSV[3],(int)BKSV[4]));

    for(TimeOut = 0, ucdata = 0 ; TimeOut < 5 ; TimeOut ++)
    {
        ucdata += countbit(BKSV[TimeOut]);
    }
    if( ucdata != 20 )
	{
		printf(("countbit error\n"));
		return ER_FAIL ;
	}

	if( (BKSV[4] == 0x93) &&
        (BKSV[3] == 0x43) &&
        (BKSV[2] == 0x5C) &&
        (BKSV[1] == 0xDE) &&
        (BKSV[0] == 0x23))
    {
        printf("Revoked BKSV.\n");
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank_66121(0); // switch bank action should start on direct register writting of each function.

    // 2006/08/11 added by jjtseng
    // enable HDCP on CPDired enabled.
    HDMITX_AndREG_Byte_66121(REG_TX_SW_RST,~(B_HDCP_RST_HDMITX));
    //~jjtseng 2006/08/11

    HDMITX_WriteI2C_Byte_66121(REG_TX_HDCP_DESIRE,B_CPDESIRE);
    HDCP_ClearAuthInterrupt();


    HDCP_GenerateAn();
    HDMITX_WriteI2C_Byte_66121(REG_TX_LISTCTRL,0);
    Instance[0].bAuthenticated = FALSE ;

    if((BCaps & B_CAP_HDMI_REPEATER) == 0)
    {
        HDCP_Auth_Fire();
        // wait for status ;

        for(TimeOut = 250 ; TimeOut > 0 ; TimeOut --)
        {
            delay1ms(5); // delay 1ms
            ucdata = HDMITX_ReadI2C_Byte_66121(REG_TX_AUTH_STAT);
            HDMITX_DEBUG_PRINTF(("reg46 = %02x reg16 = %02x\n",(int)ucdata,(int)HDMITX_ReadI2C_Byte_66121(0x16)));

            if(ucdata & B_T_AUTH_DONE)
            {
                Instance[0].bAuthenticated = TRUE ;
                break ;
            }

            ucdata = HDMITX_ReadI2C_Byte_66121(REG_TX_INT_STAT2);
            if(ucdata & B_INT_AUTH_FAIL)
            {
                /*
                HDMITX_WriteI2C_Byte_66121(REG_TX_INT_CLR0,B_CLR_AUTH_FAIL);
                HDMITX_WriteI2C_Byte_66121(REG_TX_INT_CLR1,0);
                HDMITX_WriteI2C_Byte_66121(REG_TX_SYS_STATUS,B_INTACTDONE);
                HDMITX_WriteI2C_Byte_66121(REG_TX_SYS_STATUS,0);
                */
                HDMITX_DEBUG_PRINTF(("HDCP_Authenticate(): Authenticate fail\n"));
                Instance[0].bAuthenticated = FALSE ;
                return ER_FAIL ;
            }
        }

        if(TimeOut == 0)
        {
             HDMITX_DEBUG_PRINTF(("HDCP_Authenticate(): Time out. return fail\n"));
             Instance[0].bAuthenticated = FALSE ;
             return ER_FAIL ;
        }
        return ER_SUCCESS ;
    }

    return HDCP_Authenticate_Repeater();
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_VerifyIntegration
// Parameter: N/A
// Return: ER_SUCCESS if success,if AUTH_FAIL interrupt status,return fail.
// Remark: no used now.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
HDCP_VerifyIntegration()
{
    // if any interrupt issued a Auth fail,returned the Verify Integration fail.

    if(HDMITX_ReadI2C_Byte_66121(REG_TX_INT_STAT1) & B_INT_AUTH_FAIL)
    {
        HDCP_ClearAuthInterrupt();
        Instance[0].bAuthenticated = FALSE ;
        return ER_FAIL ;
    }

    if(Instance[0].bAuthenticated == TRUE)
    {
        return ER_SUCCESS ;
    }

    return ER_FAIL ;
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_Authenticate_Repeater
// Parameter: BCaps and BStatus
// Return: ER_SUCCESS if success,if AUTH_FAIL interrupt status,return fail.
// Remark:
// Side-Effect: as Authentication
//////////////////////////////////////////////////////////////////////

static void
HDCP_CancelRepeaterAuthenticate()
{
    HDMITX_DEBUG_PRINTF(("HDCP_CancelRepeaterAuthenticate"));
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
    AbortDDC();
    HDMITX_WriteI2C_Byte_66121(REG_TX_LISTCTRL,B_LISTFAIL|B_LISTDONE);
    HDCP_ClearAuthInterrupt();
}

static void
HDCP_ResumeRepeaterAuthenticate()
{
    HDMITX_WriteI2C_Byte_66121(REG_TX_LISTCTRL,B_LISTDONE);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL,B_MASTERHDCP);
}

#ifdef SUPPORT_SHA
#define SHA_BUFF_COUNT 17
static _XDATA BYTE KSVList[32] ;
static _XDATA BYTE Vr[20] ;
static _XDATA BYTE M0[8] ;
static _XDATA BYTE V[20] ;
static _XDATA BYTE SHABuff[64] ;
static _XDATA ULONG w[SHA_BUFF_COUNT];

static _XDATA ULONG sha[5] ;

#define rol(x,y) (((x) << (y)) | (((ULONG)x) >> (32-y)))

static void SHATransform(ULONG * h)
{
    int t,i;
    ULONG tmp ;


    h[0] = 0x67452301 ;
    h[1] = 0xefcdab89;
    h[2] = 0x98badcfe;
    h[3] = 0x10325476;
    h[4] = 0xc3d2e1f0;
    for( t = 0 ; t < 80 ; t++ )
    {
        if((t>=16)&&(t<80)) {
            i=(t+SHA_BUFF_COUNT-3)%SHA_BUFF_COUNT;
            tmp = w[i];
            i=(t+SHA_BUFF_COUNT-8)%SHA_BUFF_COUNT;
            tmp ^= w[i];
            i=(t+SHA_BUFF_COUNT-14)%SHA_BUFF_COUNT;
            tmp ^= w[i];
            i=(t+SHA_BUFF_COUNT-16)%SHA_BUFF_COUNT;
            tmp ^= w[i];
            w[t%SHA_BUFF_COUNT] = rol(tmp,1);
            HDMITX_DEBUG_PRINTF(("w[%2d] = %08lX\n",t,w[t%SHA_BUFF_COUNT]));
        }

        if((t>=0)&&(t<20)) {
            tmp = rol(h[0],5) + ((h[1] & h[2]) | (h[3] & ~h[1])) + h[4] + w[t%SHA_BUFF_COUNT] + 0x5a827999;
            HDMITX_DEBUG_PRINTF(("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]));

            h[4] = h[3];
            h[3] = h[2];
            h[2] = rol(h[1],30);
            h[1] = h[0];
            h[0] = tmp;

        }
        if((t>=20)&&(t<40)) {
            tmp = rol(h[0],5) + (h[1] ^ h[2] ^ h[3]) + h[4] + w[t%SHA_BUFF_COUNT] + 0x6ed9eba1;
            HDMITX_DEBUG_PRINTF(("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]));
            h[4] = h[3];
            h[3] = h[2];
            h[2] = rol(h[1],30);
            h[1] = h[0];
            h[0] = tmp;
        }
        if((t>=40)&&(t<60)) {
            tmp = rol(h[0], 5) + ((h[1] & h[2]) | (h[1] & h[3]) | (h[2] & h[3])) + h[4] + w[t%SHA_BUFF_COUNT] +
                0x8f1bbcdc;
            HDMITX_DEBUG_PRINTF(("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]));
            h[4] = h[3];
            h[3] = h[2];
            h[2] = rol(h[1],30);
            h[1] = h[0];
            h[0] = tmp;
        }
        if((t>=60)&&(t<80)) {
            tmp = rol(h[0],5) + (h[1] ^ h[2] ^ h[3]) + h[4] + w[t%SHA_BUFF_COUNT] + 0xca62c1d6;
            HDMITX_DEBUG_PRINTF(("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]));
            h[4] = h[3];
            h[3] = h[2];
            h[2] = rol(h[1],30);
            h[1] = h[0];
            h[0] = tmp;
        }
    }
    HDMITX_DEBUG_PRINTF(("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]));

    h[0] += 0x67452301 ;
    h[1] += 0xefcdab89;
    h[2] += 0x98badcfe;
    h[3] += 0x10325476;
    h[4] += 0xc3d2e1f0;
//    HDMITX_DEBUG_PRINTF(("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]));
}

/* ----------------------------------------------------------------------
 * Outer SHA algorithm: take an arbitrary length byte string,
 * convert it into 16-word blocks with the prescribed padding at
 * the end,and pass those blocks to the core SHA algorithm.
 */


static void SHA_Simple(void *p,LONG len,BYTE *output)
{
    // SHA_State s;
    int i, t ;
    ULONG c ;
    char *pBuff = p ;


    for( i = 0 ; i < len ; i++ )
    {
        t = i/4 ;
        if( i%4 == 0 )
        {
            w[t] = 0 ;
        }
        c = pBuff[i] ;
        c &= 0xFF ;
        c <<= (3-(i%4))*8 ;
        w[t] |= c ;
//        HDMITX_DEBUG_PRINTF(("pBuff[%d] = %02x, c = %08lX, w[%d] = %08lX\n",i,pBuff[i],c,t,w[t]));
    }
    t = i/4 ;
    if( i%4 == 0 )
    {
        w[t] = 0 ;
    }
    c = 0x80 << ((3-i%4)*8);
    w[t]|= c ; t++ ;
    for( ; t < 15 ; t++ )
    {
        w[t] = 0 ;
    }
    w[15] = len*8  ;

    for( t = 0 ; t< 16 ; t++ )
    {
//        HDMITX_DEBUG_PRINTF(("w[%2d] = %08lX\n",t,w[t]));
    }

    SHATransform(sha);

    for( i = 0 ; i < 5 ; i++ )
    {
        output[i*4] = (BYTE)(sha[i]&0xFF);
        output[i*4+1] = (BYTE)((sha[i]>>8)&0xFF);
        output[i*4+2] = (BYTE)((sha[i]>>16)&0xFF);
        output[i*4+3]   = (BYTE)((sha[i]>>24)&0xFF);
    }
}

static SYS_STATUS
HDCP_CheckSHA(BYTE pM0[],unsigned int BStatus,BYTE pKSVList[],int cDownStream,BYTE Vr[])
{
    int i,n ;

    for(i = 0 ; i < cDownStream*5 ; i++)
    {
        SHABuff[i] = pKSVList[i] ;
    }
    SHABuff[i++] = BStatus & 0xFF ;
    SHABuff[i++] = (BStatus>>8) & 0xFF ;
    for(n = 0 ; n < 8 ; n++,i++)
    {
        SHABuff[i] = pM0[n] ;
    }
    n = i ;
    // SHABuff[i++] = 0x80 ; // end mask
    for(; i < 64 ; i++)
    {
        SHABuff[i] = 0 ;
    }
    // n = cDownStream * 5 + 2 /* for BStatus */ + 8 /* for M0 */ ;
    // n *= 8 ;
    // SHABuff[62] = (n>>8) & 0xff ;
    // SHABuff[63] = (n>>8) & 0xff ;
    for(i = 0 ; i < 64 ; i++)
    {
        if(i % 16 == 0)
        {
            HDMITX_DEBUG_PRINTF(("SHA[]: "));
        }
//        HDMITX_DEBUG_PRINTF((" %02X",SHABuff[i]));
        if((i%16)==15)
        {
            HDMITX_DEBUG_PRINTF(("\n"));
        }
    }
    SHA_Simple(SHABuff,n,V);
    HDMITX_DEBUG_PRINTF(("V[] ="));
    for(i = 0 ; i < 20 ; i++)
    {
//        HDMITX_DEBUG_PRINTF((" %02X",(int)V[i]));
    }
    HDMITX_DEBUG_PRINTF(("\nVr[] ="));
    for(i = 0 ; i < 20 ; i++)
    {
//        HDMITX_DEBUG_PRINTF((" %02X",(int)Vr[i]));
    }

    for(i = 0 ; i < 20 ; i++)
    {
        if(V[i] != Vr[i])
        {
            return ER_FAIL ;
        }
    }
    return ER_SUCCESS ;
}
#endif // SUPPORT_SHA

static SYS_STATUS
HDCP_GetKSVList(BYTE *pKSVList,BYTE cDownStream)
{
    BYTE TimeOut = 100 ;
    BYTE ucdata ;

    if( cDownStream == 0 )
    {
        return ER_SUCCESS ;
    }

    if( /* cDownStream == 0 || */ pKSVList == NULL)
    {
        return ER_FAIL ;
    }

    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL,B_MASTERHOST);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_HEADER,0x74);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_REQOFF,0x43);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_REQCOUNT,cDownStream * 5);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD);


    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {

        ucdata = HDMITX_ReadI2C_Byte_66121(REG_TX_DDC_STATUS);
        if(ucdata & B_DDC_DONE)
        {
            HDMITX_DEBUG_PRINTF(("HDCP_GetKSVList(): DDC Done.\n"));
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
            HDMITX_DEBUG_PRINTF(("HDCP_GetKSVList(): DDC Fail by REG_TX_DDC_STATUS = %x.\n",ucdata));
            return ER_FAIL ;
        }
        delay1ms(5);
    }

    if(TimeOut == 0)
    {
        return ER_FAIL ;
    }

    HDMITX_DEBUG_PRINTF(("HDCP_GetKSVList(): KSV"));
    for(TimeOut = 0 ; TimeOut < cDownStream * 5 ; TimeOut++)
    {
        pKSVList[TimeOut] = HDMITX_ReadI2C_Byte_66121(REG_TX_DDC_READFIFO);
//        HDMITX_DEBUG_PRINTF((" %02X",(int)pKSVList[TimeOut]));
    }
    HDMITX_DEBUG_PRINTF(("\n"));
    return ER_SUCCESS ;
}

static SYS_STATUS
HDCP_GetVr(BYTE *pVr)
{
    BYTE TimeOut  ;
    BYTE ucdata ;

    if(pVr == NULL)
    {
        return ER_FAIL ;
    }

    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_MASTER_CTRL,B_MASTERHOST);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_HEADER,0x74);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_REQOFF,0x20);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_REQCOUNT,20);
    HDMITX_WriteI2C_Byte_66121(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD);


    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {
        ucdata = HDMITX_ReadI2C_Byte_66121(REG_TX_DDC_STATUS);
        if(ucdata & B_DDC_DONE)
        {
            HDMITX_DEBUG_PRINTF(("HDCP_GetVr(): DDC Done.\n"));
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
            HDMITX_DEBUG_PRINTF(("HDCP_GetVr(): DDC fail by REG_TX_DDC_STATUS = %x.\n",(int)ucdata));
            return ER_FAIL ;
        }
        delay1ms(5);
    }

    if(TimeOut == 0)
    {
        HDMITX_DEBUG_PRINTF(("HDCP_GetVr(): DDC fail by timeout.\n"));
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank_66121(0);

    for(TimeOut = 0 ; TimeOut < 5 ; TimeOut++)
    {
        HDMITX_WriteI2C_Byte_66121(REG_TX_SHA_SEL ,TimeOut);
        pVr[TimeOut*4]  = (ULONG)HDMITX_ReadI2C_Byte_66121(REG_TX_SHA_RD_BYTE1);
        pVr[TimeOut*4+1] = (ULONG)HDMITX_ReadI2C_Byte_66121(REG_TX_SHA_RD_BYTE2);
        pVr[TimeOut*4+2] = (ULONG)HDMITX_ReadI2C_Byte_66121(REG_TX_SHA_RD_BYTE3);
        pVr[TimeOut*4+3] = (ULONG)HDMITX_ReadI2C_Byte_66121(REG_TX_SHA_RD_BYTE4);
//        HDMITX_DEBUG_PRINTF(("V' = %02X %02X %02X %02X\n",(int)pVr[TimeOut*4],(int)pVr[TimeOut*4+1],(int)pVr[TimeOut*4+2],(int)pVr[TimeOut*4+3]));
    }

    return ER_SUCCESS ;
}

static SYS_STATUS
HDCP_GetM0(BYTE *pM0)
{
    int i ;

    if(!pM0)
    {
        return ER_FAIL ;
    }

    HDMITX_WriteI2C_Byte_66121(REG_TX_SHA_SEL,5); // read m0[31:0] from reg51~reg54
    pM0[0] = HDMITX_ReadI2C_Byte_66121(REG_TX_SHA_RD_BYTE1);
    pM0[1] = HDMITX_ReadI2C_Byte_66121(REG_TX_SHA_RD_BYTE2);
    pM0[2] = HDMITX_ReadI2C_Byte_66121(REG_TX_SHA_RD_BYTE3);
    pM0[3] = HDMITX_ReadI2C_Byte_66121(REG_TX_SHA_RD_BYTE4);
    HDMITX_WriteI2C_Byte_66121(REG_TX_SHA_SEL,0); // read m0[39:32] from reg55
    pM0[4] = HDMITX_ReadI2C_Byte_66121(REG_TX_AKSV_RD_BYTE5);
    HDMITX_WriteI2C_Byte_66121(REG_TX_SHA_SEL,1); // read m0[47:40] from reg55
    pM0[5] = HDMITX_ReadI2C_Byte_66121(REG_TX_AKSV_RD_BYTE5);
    HDMITX_WriteI2C_Byte_66121(REG_TX_SHA_SEL,2); // read m0[55:48] from reg55
    pM0[6] = HDMITX_ReadI2C_Byte_66121(REG_TX_AKSV_RD_BYTE5);
    HDMITX_WriteI2C_Byte_66121(REG_TX_SHA_SEL,3); // read m0[63:56] from reg55
    pM0[7] = HDMITX_ReadI2C_Byte_66121(REG_TX_AKSV_RD_BYTE5);

    HDMITX_DEBUG_PRINTF(("M[] ="));
    for(i = 0 ; i < 8 ; i++){
//        HDMITX_DEBUG_PRINTF(("0x%02x,",(int)pM0[i]));
    }
    HDMITX_DEBUG_PRINTF(("\n"));
    return ER_SUCCESS ;
}



static SYS_STATUS
HDCP_Authenticate_Repeater()
{
    BYTE uc ;
    // BYTE revoked ;
    // int i ;
    BYTE cDownStream ;

    BYTE BCaps;
    unsigned int BStatus ;
    unsigned int TimeOut ;

    HDMITX_DEBUG_PRINTF(("Authentication for repeater\n"));
    // emily add for test,abort HDCP
    // 2007/10/01 marked by jj_tseng@chipadvanced.com
    // HDMITX_WriteI2C_Byte_66121(0x20,0x00);
    // HDMITX_WriteI2C_Byte_66121(0x04,0x01);
    // HDMITX_WriteI2C_Byte_66121(0x10,0x01);
    // HDMITX_WriteI2C_Byte_66121(0x15,0x0F);
    // delay1ms(100);
    // HDMITX_WriteI2C_Byte_66121(0x04,0x00);
    // HDMITX_WriteI2C_Byte_66121(0x10,0x00);
    // HDMITX_WriteI2C_Byte_66121(0x20,0x01);
    // delay1ms(100);
    // test07 = HDMITX_ReadI2C_Byte_66121(0x7);
    // test06 = HDMITX_ReadI2C_Byte_66121(0x6);
    // test08 = HDMITX_ReadI2C_Byte_66121(0x8);
    //~jj_tseng@chipadvanced.com
    // end emily add for test
    //////////////////////////////////////
    // Authenticate Fired
    //////////////////////////////////////

    HDCP_GetBCaps(&BCaps,&BStatus);
    delay1ms(2);
    HDCP_Auth_Fire();
    delay1ms(550); // emily add for test

    for(TimeOut = 250*6 ; TimeOut > 0 ; TimeOut --)
    {

        uc = HDMITX_ReadI2C_Byte_66121(REG_TX_INT_STAT1);
        if(uc & B_INT_DDC_BUS_HANG)
        {
            HDMITX_DEBUG_PRINTF(("DDC Bus hang\n"));
            goto HDCP_Repeater_Fail ;
        }

        uc = HDMITX_ReadI2C_Byte_66121(REG_TX_INT_STAT2);

        if(uc & B_INT_AUTH_FAIL)
        {
            /*
            HDMITX_WriteI2C_Byte_66121(REG_TX_INT_CLR0,B_CLR_AUTH_FAIL);
            HDMITX_WriteI2C_Byte_66121(REG_TX_INT_CLR1,0);
            HDMITX_WriteI2C_Byte_66121(REG_TX_SYS_STATUS,B_INTACTDONE);
            HDMITX_WriteI2C_Byte_66121(REG_TX_SYS_STATUS,0);
            */
            HDMITX_DEBUG_PRINTF(("HDCP_Authenticate_Repeater(): B_INT_AUTH_FAIL.\n"));
            goto HDCP_Repeater_Fail ;
        }
        // emily add for test
        // test =(HDMITX_ReadI2C_Byte_66121(0x7)&0x4)>>2 ;
        if(uc & B_INT_KSVLIST_CHK)
        {
            HDMITX_WriteI2C_Byte_66121(REG_TX_INT_CLR0,B_CLR_KSVLISTCHK);
            HDMITX_WriteI2C_Byte_66121(REG_TX_INT_CLR1,0);
            HDMITX_WriteI2C_Byte_66121(REG_TX_SYS_STATUS,B_INTACTDONE);
            HDMITX_WriteI2C_Byte_66121(REG_TX_SYS_STATUS,0);
            HDMITX_DEBUG_PRINTF(("B_INT_KSVLIST_CHK\n"));
            break ;
        }

        delay1ms(5);
    }

    if(TimeOut == 0)
    {
        HDMITX_DEBUG_PRINTF(("Time out for wait KSV List checking interrupt\n"));
        goto HDCP_Repeater_Fail ;
    }

    ///////////////////////////////////////
    // clear KSVList check interrupt.
    ///////////////////////////////////////

    for(TimeOut = 500 ; TimeOut > 0 ; TimeOut --)
    {
        if((TimeOut % 100) == 0)
        {
            HDMITX_DEBUG_PRINTF(("Wait KSV FIFO Ready %d\n",TimeOut));
        }

        if(HDCP_GetBCaps(&BCaps,&BStatus) == ER_FAIL)
        {
            HDMITX_DEBUG_PRINTF(("Get BCaps fail\n"));
            goto HDCP_Repeater_Fail ;
        }

        if(BCaps & B_CAP_KSV_FIFO_RDY)
        {
             HDMITX_DEBUG_PRINTF(("FIFO Ready\n"));
             break ;
        }
        delay1ms(5);

    }

    if(TimeOut == 0)
    {
        HDMITX_DEBUG_PRINTF(("Get KSV FIFO ready TimeOut\n"));
        goto HDCP_Repeater_Fail ;
    }

    HDMITX_DEBUG_PRINTF(("Wait timeout = %d\n",TimeOut));

    ClearDDCFIFO();
    GenerateDDCSCLK();
    cDownStream =  (BStatus & M_DOWNSTREAM_COUNT);

    if(/*cDownStream == 0 ||*/ cDownStream > 6 || BStatus & (B_MAX_CASCADE_EXCEEDED|B_DOWNSTREAM_OVER))
    {
        HDMITX_DEBUG_PRINTF(("Invalid Down stream count,fail\n"));
        goto HDCP_Repeater_Fail ;
    }

#ifdef SUPPORT_SHA
    if(HDCP_GetKSVList(KSVList,cDownStream) == ER_FAIL)
    {
        goto HDCP_Repeater_Fail ;
    }

#if 0
    for(i = 0 ; i < cDownStream ; i++)
    {
        revoked=FALSE ; uc = 0 ;
        for( TimeOut = 0 ; TimeOut < 5 ; TimeOut++ )
        {
            // check bit count
            uc += countbit(KSVList[i*5+TimeOut]);
        }
        if( uc != 20 ) revoked = TRUE ;

        if(revoked)
        {
//            HDMITX_DEBUG_PRINTF(("KSVFIFO[%d] = %02X %02X %02X %02X %02X is revoked\n",i,(int)KSVList[i*5],(int)KSVList[i*5+1],(int)KSVList[i*5+2],(int)KSVList[i*5+3],(int)KSVList[i*5+4]));
             goto HDCP_Repeater_Fail ;
        }
    }
#endif


    if(HDCP_GetVr(Vr) == ER_FAIL)
    {
        goto HDCP_Repeater_Fail ;
    }

    if(HDCP_GetM0(M0) == ER_FAIL)
    {
        goto HDCP_Repeater_Fail ;
    }

    // do check SHA
    if(HDCP_CheckSHA(M0,BStatus,KSVList,cDownStream,Vr) == ER_FAIL)
    {
        goto HDCP_Repeater_Fail ;
    }
#endif // SUPPORT_SHA


    HDCP_ResumeRepeaterAuthenticate();
    Instance[0].bAuthenticated = TRUE ;
    return ER_SUCCESS ;

HDCP_Repeater_Fail:
    HDCP_CancelRepeaterAuthenticate();
    return ER_FAIL ;
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_ResumeAuthentication
// Parameter: N/A
// Return: N/A
// Remark: called by interrupt handler to restart Authentication and Encryption.
// Side-Effect: as Authentication and Encryption.
//////////////////////////////////////////////////////////////////////

static void
HDCP_ResumeAuthentication()
{
    SetAVMute_66121(TRUE);
    if(HDCP_Authenticate() == ER_SUCCESS)
    {
        HDCP_EnableEncryption();
    }
    SetAVMute_66121(FALSE);
}



#endif // SUPPORT_HDCP


static void
ENABLE_NULL_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_NULL_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_ACP_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_ACP_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_ISRC1_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_ISRC1_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_ISRC2_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_ISRC2_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_AVI_INFOFRM_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_AVI_INFOFRM_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_AUD_INFOFRM_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_AUD_INFOFRM_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_SPD_INFOFRM_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_SPD_INFOFRM_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_MPG_INFOFRM_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_MPG_INFOFRM_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}

static void
ENABLE_GeneralPurpose_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_NULL_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}

static void
DISABLE_NULL_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_NULL_CTRL,0);
}


static void
DISABLE_ACP_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_ACP_CTRL,0);
}


static void
DISABLE_ISRC1_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_ISRC1_CTRL,0);
}


static void
DISABLE_ISRC2_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_ISRC2_CTRL,0);
}


static void
DISABLE_AVI_INFOFRM_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_AVI_INFOFRM_CTRL,0);
}


static void
DISABLE_AUD_INFOFRM_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_AUD_INFOFRM_CTRL,0);
}


static void
DISABLE_SPD_INFOFRM_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_SPD_INFOFRM_CTRL,0);
}


static void
DISABLE_MPG_INFOFRM_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_MPG_INFOFRM_CTRL,0);
}

static void
DISABLE_GeneralPurpose_PKT()
{

    HDMITX_WriteI2C_Byte_66121(REG_TX_NULL_CTRL,0);
}

//////////////////////////////////////////////////////////////////////
// Function: SetAVIInfoFrame()
// Parameter: pAVIInfoFrame - the pointer to HDMI AVI Infoframe ucData
// Return: N/A
// Remark: Fill the AVI InfoFrame ucData,and count checksum,then fill into
//         AVI InfoFrame registers.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetAVIInfoFrame(AVI_InfoFrame *pAVIInfoFrame)
{
    int i ;
    byte ucData ;

    if(!pAVIInfoFrame)
    {
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank_66121(1);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AVIINFO_DB1,pAVIInfoFrame->pktbyte.AVI_DB[0]);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AVIINFO_DB2,pAVIInfoFrame->pktbyte.AVI_DB[1]);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AVIINFO_DB3,pAVIInfoFrame->pktbyte.AVI_DB[2]);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AVIINFO_DB4,pAVIInfoFrame->pktbyte.AVI_DB[3]);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AVIINFO_DB5,pAVIInfoFrame->pktbyte.AVI_DB[4]);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AVIINFO_DB6,pAVIInfoFrame->pktbyte.AVI_DB[5]);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AVIINFO_DB7,pAVIInfoFrame->pktbyte.AVI_DB[6]);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AVIINFO_DB8,pAVIInfoFrame->pktbyte.AVI_DB[7]);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AVIINFO_DB9,pAVIInfoFrame->pktbyte.AVI_DB[8]);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AVIINFO_DB10,pAVIInfoFrame->pktbyte.AVI_DB[9]);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AVIINFO_DB11,pAVIInfoFrame->pktbyte.AVI_DB[10]);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AVIINFO_DB12,pAVIInfoFrame->pktbyte.AVI_DB[11]);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AVIINFO_DB13,pAVIInfoFrame->pktbyte.AVI_DB[12]);
    for(i = 0,ucData = 0; i < 13 ; i++)
    {
        ucData -= pAVIInfoFrame->pktbyte.AVI_DB[i] ;
    }
    /*
    HDMITX_DEBUG_PRINTF(("SetAVIInfo(): "));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte_66121(REG_TX_AVIINFO_DB1)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte_66121(REG_TX_AVIINFO_DB2)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte_66121(REG_TX_AVIINFO_DB3)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte_66121(REG_TX_AVIINFO_DB4)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte_66121(REG_TX_AVIINFO_DB5)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte_66121(REG_TX_AVIINFO_DB6)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte_66121(REG_TX_AVIINFO_DB7)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte_66121(REG_TX_AVIINFO_DB8)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte_66121(REG_TX_AVIINFO_DB9)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte_66121(REG_TX_AVIINFO_DB10)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte_66121(REG_TX_AVIINFO_DB11)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte_66121(REG_TX_AVIINFO_DB12)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte_66121(REG_TX_AVIINFO_DB13)));
    HDMITX_DEBUG_PRINTF(("\n"));
    */
    ucData -= 0x80+AVI_INFOFRAME_VER+AVI_INFOFRAME_TYPE+AVI_INFOFRAME_LEN ;
    HDMITX_WriteI2C_Byte_66121(REG_TX_AVIINFO_SUM,ucData);


    Switch_HDMITX_Bank_66121(0);
    ENABLE_AVI_INFOFRM_PKT();
    return ER_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////
// Function: SetAudioInfoFrame()
// Parameter: pAudioInfoFrame - the pointer to HDMI Audio Infoframe ucData
// Return: N/A
// Remark: Fill the Audio InfoFrame ucData,and count checksum,then fill into
//         Audio InfoFrame registers.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetAudioInfoFrame(Audio_InfoFrame *pAudioInfoFrame)
{
    BYTE uc ;

    if(!pAudioInfoFrame)
    {
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank_66121(1);
    uc = 0x80-(AUDIO_INFOFRAME_VER+AUDIO_INFOFRAME_TYPE+AUDIO_INFOFRAME_LEN );
    HDMITX_WriteI2C_Byte_66121(REG_TX_PKT_AUDINFO_CC,pAudioInfoFrame->pktbyte.AUD_DB[0]);
    uc -= HDMITX_ReadI2C_Byte_66121(REG_TX_PKT_AUDINFO_CC); uc &= 0xFF ;
    HDMITX_WriteI2C_Byte_66121(REG_TX_PKT_AUDINFO_SF,pAudioInfoFrame->pktbyte.AUD_DB[1]);
    uc -= HDMITX_ReadI2C_Byte_66121(REG_TX_PKT_AUDINFO_SF); uc &= 0xFF ;
    HDMITX_WriteI2C_Byte_66121(REG_TX_PKT_AUDINFO_CA,pAudioInfoFrame->pktbyte.AUD_DB[3]);
    uc -= HDMITX_ReadI2C_Byte_66121(REG_TX_PKT_AUDINFO_CA); uc &= 0xFF ;
    HDMITX_WriteI2C_Byte_66121(REG_TX_PKT_AUDINFO_DM_LSV,pAudioInfoFrame->pktbyte.AUD_DB[4]);
    uc -= HDMITX_ReadI2C_Byte_66121(REG_TX_PKT_AUDINFO_DM_LSV); uc &= 0xFF ;

    HDMITX_WriteI2C_Byte_66121(REG_TX_PKT_AUDINFO_SUM,uc);


    Switch_HDMITX_Bank_66121(0);
    ENABLE_AUD_INFOFRM_PKT();
    return ER_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////
// Function: SetSPDInfoFrame()
// Parameter: pSPDInfoFrame - the pointer to HDMI SPD Infoframe ucData
// Return: N/A
// Remark: Fill the SPD InfoFrame ucData,and count checksum,then fill into
//         SPD InfoFrame registers.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetSPDInfoFrame(SPD_InfoFrame *pSPDInfoFrame)
{
    int i ;
    BYTE ucData ;

    if(!pSPDInfoFrame)
    {
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank_66121(1);
    for(i = 0,ucData = 0 ; i < 25 ; i++)
    {
        ucData -= pSPDInfoFrame->pktbyte.SPD_DB[i] ;
        HDMITX_WriteI2C_Byte_66121(REG_TX_PKT_SPDINFO_PB1+i,pSPDInfoFrame->pktbyte.SPD_DB[i]);
    }
    ucData -= 0x80+SPD_INFOFRAME_VER+SPD_INFOFRAME_TYPE+SPD_INFOFRAME_LEN ;
    HDMITX_WriteI2C_Byte_66121(REG_TX_PKT_SPDINFO_SUM,ucData); // checksum
    Switch_HDMITX_Bank_66121(0);
    ENABLE_SPD_INFOFRM_PKT();
    return ER_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////
// Function: SetMPEGInfoFrame()
// Parameter: pMPEGInfoFrame - the pointer to HDMI MPEG Infoframe ucData
// Return: N/A
// Remark: Fill the MPEG InfoFrame ucData,and count checksum,then fill into
//         MPEG InfoFrame registers.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetMPEGInfoFrame(MPEG_InfoFrame *pMPGInfoFrame)
{
    int i ;
    BYTE ucData ;

    if(!pMPGInfoFrame)
    {
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank_66121(1);

    HDMITX_WriteI2C_Byte_66121(REG_TX_PKT_MPGINFO_FMT,pMPGInfoFrame->info.FieldRepeat|(pMPGInfoFrame->info.MpegFrame<<1));
    HDMITX_WriteI2C_Byte_66121(REG_TX_PKG_MPGINFO_DB0,pMPGInfoFrame->pktbyte.MPG_DB[0]);
    HDMITX_WriteI2C_Byte_66121(REG_TX_PKG_MPGINFO_DB1,pMPGInfoFrame->pktbyte.MPG_DB[1]);
    HDMITX_WriteI2C_Byte_66121(REG_TX_PKG_MPGINFO_DB2,pMPGInfoFrame->pktbyte.MPG_DB[2]);
    HDMITX_WriteI2C_Byte_66121(REG_TX_PKG_MPGINFO_DB3,pMPGInfoFrame->pktbyte.MPG_DB[3]);

    for(ucData = 0,i = 0 ; i < 5 ; i++)
    {
        ucData -= pMPGInfoFrame->pktbyte.MPG_DB[i] ;
    }
    ucData -= 0x80+MPEG_INFOFRAME_VER+MPEG_INFOFRAME_TYPE+MPEG_INFOFRAME_LEN ;

    HDMITX_WriteI2C_Byte_66121(REG_TX_PKG_MPGINFO_SUM,ucData);

    Switch_HDMITX_Bank_66121(0);
    ENABLE_SPD_INFOFRM_PKT();

    return ER_SUCCESS ;
}


//// 2009/12/04 added by Ming-chih.lung@ite.com.tw
//
///////////////////////////////////////////////////////////////////////////////////////
//// HDMITX part
///////////////////////////////////////////////////////////////////////////////////////
//void setHDMITX_ChStat_66121(BYTE ucIEC60958ChStat[]);
//void setHDMITX_UpdateChStatFs_66121(ULONG Fs);
//void setHDMITX_LPCMAudio_66121(BYTE AudioSrcNum, BYTE AudSWL, BOOL bSPDIF);
//void setHDMITX_NLPCMAudio_66121();
//void setHDMITX_HBRAudio_66121(BOOL bSPDIF);
//void setHDMITX_DSDAudio_66121();



void
setHDMITX_ChStat_66121(BYTE ucIEC60958ChStat[])
{
    BYTE uc ;

    Switch_HDMITX_Bank_66121(1);
    uc = (ucIEC60958ChStat[0] <<1)& 0x7C ;
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_MODE,uc);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_CAT,ucIEC60958ChStat[1]); // 192, audio CATEGORY
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_SRCNUM,ucIEC60958ChStat[2]&0xF);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUD0CHST_CHTNUM,(ucIEC60958ChStat[2]>>4)&0xF);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_CA_FS,ucIEC60958ChStat[3]); // choose clock
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_OFS_WL,ucIEC60958ChStat[4]);
    Switch_HDMITX_Bank_66121(0);
}

void
setHDMITX_UpdateChStatFs_66121(ULONG Fs)
{
    BYTE uc ;

    /////////////////////////////////////
    // Fs should be the following value.
    // #define AUDFS_22p05KHz  4
    // #define AUDFS_44p1KHz 0
    // #define AUDFS_88p2KHz 8
    // #define AUDFS_176p4KHz    12
    //
    // #define AUDFS_24KHz  6
    // #define AUDFS_48KHz  2
    // #define AUDFS_96KHz  10
    // #define AUDFS_192KHz 14
    //
    // #define AUDFS_768KHz 9
    //
    // #define AUDFS_32KHz  3
    // #define AUDFS_OTHER    1
    /////////////////////////////////////

    Switch_HDMITX_Bank_66121(1);
    uc = HDMITX_ReadI2C_Byte_66121(REG_TX_AUDCHST_CA_FS); // choose clock
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_CA_FS,uc); // choose clock
    uc &= 0xF0 ;
    uc |= (Fs&0xF);

    uc = HDMITX_ReadI2C_Byte_66121(REG_TX_AUDCHST_OFS_WL);
    uc &= 0xF ;
    uc |= ((~Fs) << 4)&0xF0 ;
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDCHST_OFS_WL,uc);

    Switch_HDMITX_Bank_66121(0);


}

void
setHDMITX_LPCMAudio_66121(BYTE AudioSrcNum, BYTE AudSWL, BOOL bSPDIF)
{

    BYTE AudioEnable, AudioFormat ;

    AudioEnable = 0 ;
    AudioFormat = Instance[0].bOutputAudioMode ;

    
    switch(AudSWL)
    {
    case 16:
        AudioEnable |= M_AUD_16BIT ;
        break ;
    case 18:
        AudioEnable |= M_AUD_18BIT ;
        break ;
    case 20:
        AudioEnable |= M_AUD_20BIT ;
        break ;
    case 24:
    default:
        AudioEnable |= M_AUD_24BIT ;
        break ;
    }

    if( bSPDIF )
    {
        AudioFormat &= ~0x40 ;
        AudioEnable |= B_AUD_SPDIF|B_AUD_EN_I2S0 ;
    }
    else
    {
        AudioFormat |= 0x40 ;
        switch(AudioSrcNum)
        {
        case 4:
            AudioEnable |= B_AUD_EN_I2S3|B_AUD_EN_I2S2|B_AUD_EN_I2S1|B_AUD_EN_I2S0 ;
            break ;

        case 3:
            AudioEnable |= B_AUD_EN_I2S2|B_AUD_EN_I2S1|B_AUD_EN_I2S0 ;
            break ;

        case 2:
            AudioEnable |= B_AUD_EN_I2S1|B_AUD_EN_I2S0 ;
            break ;

        case 1:
        default:
            AudioFormat &= ~0x40 ;
            AudioEnable |= B_AUD_EN_I2S0 ;
            break ;

        }
    }
    AudioFormat|=0x01;//mingchih add
    Instance[0].bAudioChannelEnable=AudioEnable;
    
    Switch_HDMITX_Bank_66121(0);
    //HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL0,AudioEnable&0xF0);//2015.2.6, my.wei mask
    HDMITX_OrREG_Byte_66121(REG_TX_AUDIO_CTRL0, AudioEnable);//2015.2.6, my.wei enable audio source

    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL1,AudioFormat); // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_FIFOMAP,0xE4); // default mapping.
#ifdef USE_SPDIF_CHSTAT
    if( bSPDIF )
    {
        HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL3,B_CHSTSEL);
    }
    else
    {
        HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL3,0);
    }
#else // not USE_SPDIF_CHSTAT
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL3,0);
#endif // USE_SPDIF_CHSTAT

    HDMITX_WriteI2C_Byte_66121(REG_TX_AUD_SRCVALID_FLAT,0x00);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUD_HDAUDIO,0x00); // regE5 = 0 ;

    if( bSPDIF )
    {
        BYTE i ;
        HDMI_OrREG_TX_Byte_66121(0x5c,(1<<6));
        for( i = 0 ; i < 100 ; i++ )
        {
            if(HDMITX_ReadI2C_Byte_66121(REG_TX_CLK_STATUS2) & B_OSF_LOCK)
            {
                break ; // stable clock.
            }
        }
    }
//    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL0, Instance[0].bAudioChannelEnable);
}

void
setHDMITX_NLPCMAudio_66121() // no Source Num, no I2S.
{
    BYTE AudioEnable, AudioFormat ;
    BYTE i ;

    AudioFormat = 0x01 ; // NLPCM must use standard I2S mode.
    AudioEnable = M_AUD_24BIT|B_AUD_SPDIF|B_AUD_EN_I2S0 ;

    Switch_HDMITX_Bank_66121(0);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_AUD_SPDIF);
    //HDMITX_AndREG_Byte_66121(REG_TX_SW_RST,~(B_AUD_RST_HDMITX|B_AREF_RST));

    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL1,0x01); // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_FIFOMAP,0xE4); // default mapping.

#ifdef USE_SPDIF_CHSTAT
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL3,B_CHSTSEL);
#else // not USE_SPDIF_CHSTAT
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL3,0);
#endif // USE_SPDIF_CHSTAT

    HDMITX_WriteI2C_Byte_66121(REG_TX_AUD_SRCVALID_FLAT,0x00);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUD_HDAUDIO,0x00); // regE5 = 0 ;

    for( i = 0 ; i < 100 ; i++ )
    {
        if(HDMITX_ReadI2C_Byte_66121(REG_TX_CLK_STATUS2) & B_OSF_LOCK)
        {
            break ; // stable clock.
        }
    }
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_AUD_SPDIF|B_AUD_EN_I2S0);
}

void
setHDMITX_HBRAudio_66121(BOOL bSPDIF)
{
    BYTE rst,uc ;
    Switch_HDMITX_Bank_66121(0);

    rst = HDMITX_ReadI2C_Byte_66121(REG_TX_SW_RST);

    //HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST, rst | (B_AUD_RST_HDMITX|B_AREF_RST) );

    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL1,0x47); // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_FIFOMAP,0xE4); // default mapping.

    if( bSPDIF )
    {
        HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_AUD_SPDIF);
        HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL3,B_CHSTSEL);
    }
    else
    {
        HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL0, M_AUD_24BIT);
        HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL3,0);
    }

    HDMITX_WriteI2C_Byte_66121(REG_TX_AUD_SRCVALID_FLAT,0x08);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUD_HDAUDIO,B_HBR); // regE5 = 0 ;
    uc = HDMITX_ReadI2C_Byte_66121(REG_TX_CLK_CTRL1);
    uc &= ~M_AUD_DIV ;
    HDMITX_WriteI2C_Byte_66121(REG_TX_CLK_CTRL1, uc);

    if( bSPDIF )
    {
        BYTE i ;
        for( i = 0 ; i < 100 ; i++ )
        {
            if(HDMITX_ReadI2C_Byte_66121(REG_TX_CLK_STATUS2) & B_OSF_LOCK)
            {
                break ; // stable clock.
            }
        }
        HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_AUD_SPDIF|B_AUD_EN_SPDIF);
    }
    else
    {
        HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_AUD_EN_I2S3|B_AUD_EN_I2S2|B_AUD_EN_I2S1|B_AUD_EN_I2S0);
    }
    HDMI_AndREG_TX_Byte_66121(0x5c,~(1<<6));
    Instance[0].bAudioChannelEnable=HDMITX_ReadI2C_Byte_66121(REG_TX_AUDIO_CTRL0);
    //HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST, rst & ~(B_AUD_RST_HDMITX|B_AREF_RST) );
}

void
setHDMITX_DSDAudio_66121()
{
    // to be continue
    BYTE rst, uc ;
    rst = HDMITX_ReadI2C_Byte_66121(REG_TX_SW_RST);

    //HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST, rst | (B_AUD_RST_HDMITX|B_AREF_RST) );

    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL1,0x41); // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_FIFOMAP,0xE4); // default mapping.

    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL0, M_AUD_24BIT);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL3,0);

    HDMITX_WriteI2C_Byte_66121(REG_TX_AUD_SRCVALID_FLAT,0x00);
    HDMITX_WriteI2C_Byte_66121(REG_TX_AUD_HDAUDIO,B_DSD); // regE5 = 0 ;
    //HDMITX_WriteI2C_Byte_66121(REG_TX_SW_RST, rst & ~(B_AUD_RST_HDMITX|B_AREF_RST) );

    uc = HDMITX_ReadI2C_Byte_66121(REG_TX_CLK_CTRL1);
    uc &= ~M_AUD_DIV ;
    HDMITX_WriteI2C_Byte_66121(REG_TX_CLK_CTRL1, uc);



    HDMITX_WriteI2C_Byte_66121(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_AUD_EN_I2S3|B_AUD_EN_I2S2|B_AUD_EN_I2S1|B_AUD_EN_I2S0);
}

void
EnableHDMIAudio_66121(BYTE AudioType, BOOL bSPDIF,  ULONG SampleFreq,  BYTE ChNum, BYTE *pIEC60958ChStat, ULONG TMDSClock)
{
    static _IDATA BYTE ucIEC60958ChStat[5] ;
    BYTE Fs ;
    Instance[0].TMDSClock=TMDSClock;
    Instance[0].bAudioChannelEnable=0;
    Instance[0].bSPDIF_OUT=bSPDIF;
    HDMITX_OrReg_Byte_66121(REG_TX_SW_RST,(B_AUD_RST_HDMITX | B_AREF_RST));
    HDMITX_WriteI2C_Byte_66121(REG_TX_CLK_CTRL0,B_AUTO_OVER_SAMPLING_CLOCK|B_EXT_256FS|0x01);
    if(bSPDIF)
    {
        if(AudioType==T_AUDIO_HBR)
        {
            HDMITX_WriteI2C_Byte_66121(REG_TX_CLK_CTRL0,0x81);
        }
        HDMITX_OrREG_Byte_66121(REG_TX_AUDIO_CTRL0,B_AUD_SPDIF);
    }
    else
    {
        HDMITX_AndREG_Byte_66121(REG_TX_AUDIO_CTRL0,(~B_AUD_SPDIF));
    }

    if( AudioType != T_AUDIO_DSD)
    {
        // one bit audio have no channel status.
        switch(SampleFreq)
        {
        case  44100L: Fs =  AUDFS_44p1KHz ; break ;
        case  88200L: Fs =  AUDFS_88p2KHz ; break ;
        case 176400L: Fs = AUDFS_176p4KHz ; break ;
        case  32000L: Fs =    AUDFS_32KHz ; break ;
        case  48000L: Fs =    AUDFS_48KHz ; break ;
        case  96000L: Fs =    AUDFS_96KHz ; break ;
        case 192000L: Fs =   AUDFS_192KHz ; break ;
        case 768000L: Fs =   AUDFS_768KHz ; break ;
        default:
            SampleFreq = 48000L ;
            Fs =    AUDFS_48KHz ;
            break ; // default, set Fs = 48KHz.
        }

#ifdef SUPPORT_AUDIO_MONITOR
    Instance[0].bAudFs=AUDFS_OTHER;
#else
    Instance[0].bAudFs=Fs;
#endif

        if( pIEC60958ChStat == NULL )
        {
            ucIEC60958ChStat[0] = 0 ;
            ucIEC60958ChStat[1] = 0 ;
            ucIEC60958ChStat[2] = (ChNum+1)/2 ;

            if(ucIEC60958ChStat[2]<1)
            {
                ucIEC60958ChStat[2] = 1 ;
            }
            else if( ucIEC60958ChStat[2] >4 )
            {
                ucIEC60958ChStat[2] = 4 ;
            }

            ucIEC60958ChStat[3] = Fs ;
#if(SUPPORT_AUDI_AudSWL==16)
            ucIEC60958ChStat[4] = ((~Fs)<<4) & 0xF0 | 0x02 ; // Fs | 24bit word length
#elif(SUPPORT_AUDI_AudSWL==18)
            ucIEC60958ChStat[4] = ((~Fs)<<4) & 0xF0 | 0x04 ; // Fs | 24bit word length
#elif(SUPPORT_AUDI_AudSWL==20)
            ucIEC60958ChStat[4] = ((~Fs)<<4) & 0xF0 | 0x03 ; // Fs | 24bit word length
#else
            ucIEC60958ChStat[4] = ((~Fs)<<4) & 0xF0 | 0x0B ; // Fs | 24bit word length
#endif
            pIEC60958ChStat = ucIEC60958ChStat ;
        }
    }

    switch(AudioType)
    {
    case T_AUDIO_HBR:
        HDMITX_DEBUG_PRINTF(("T_AUDIO_HBR\n"));
        pIEC60958ChStat[0] |= 1<<1 ;
        pIEC60958ChStat[2] = 0;
        pIEC60958ChStat[3] &= 0xF0 ;
        pIEC60958ChStat[3] |= AUDFS_768KHz ;
        pIEC60958ChStat[4] |= (((~AUDFS_768KHz)<<4) & 0xF0)| 0xB ;
        setHDMITX_ChStat_66121(pIEC60958ChStat);
        setHDMITX_HBRAudio_66121(bSPDIF);

        break ;
    case T_AUDIO_DSD:
        HDMITX_DEBUG_PRINTF(("T_AUDIO_DSD\n"));
        setHDMITX_DSDAudio_66121();
        break ;
    case T_AUDIO_NLPCM:
        HDMITX_DEBUG_PRINTF(("T_AUDIO_NLPCM\n"));
        pIEC60958ChStat[0] |= 1<<1 ;
        setHDMITX_ChStat_66121(pIEC60958ChStat);
        setHDMITX_NLPCMAudio_66121();
        break ;
    case T_AUDIO_LPCM:
        HDMITX_DEBUG_PRINTF(("T_AUDIO_LPCM\n"));
        pIEC60958ChStat[0] &= ~(1<<1);

        setHDMITX_ChStat_66121(pIEC60958ChStat);
        setHDMITX_LPCMAudio_66121((ChNum+1)/2, /*24*/SUPPORT_AUDI_AudSWL, bSPDIF);
        // can add auto adjust
        break ;
    }
    HDMITX_AndREG_Byte_66121(REG_TX_INT_MASK1,(~B_AUDIO_OVFLW_MASK));
    HDMITX_AndREG_Byte_66121(REG_TX_SW_RST,~(B_AUD_RST_HDMITX|B_AREF_RST));
}

static SYS_STATUS Set_GeneralPurpose_PKT(BYTE *pData)
{
    int i ;

    if( pData == NULL )
    {
        return ER_FAIL ;

    }

    Switch_HDMITX_Bank_66121(1);
    for( i = 0x38 ; i <= 0x56 ; i++)
    {
        HDMITX_WriteI2C_Byte_66121(i, pData[i-0x38] ) ;
    }
    Switch_HDMITX_Bank_66121(0);
    ENABLE_GeneralPurpose_PKT();
    //ENABLE_NULL_PKT() ;
    return ER_SUCCESS ;
}

BOOL
EnableVendorSpecificInfoFrame_66121(BYTE bEnable, BYTE *pInfoFrame)
{
    BYTE checksum ;
    int i ;
    if( !bEnable)
    {
        DISABLE_GeneralPurpose_PKT();
        //DISABLE_NULL_PKT() ;
        return TRUE ;
    }

    do
    {
        if( !pInfoFrame ){ break ; }

        if( pInfoFrame[0] != 0x81 || pInfoFrame[1] != 0x01 )
        {
            break ; // is not a valid VSIP
        }

        pInfoFrame[4] = 0x03 ;
        pInfoFrame[5] = 0x0C ;
        pInfoFrame[6] = 0x00 ; // HDMI vendor specific ID

        checksum = (0 - pInfoFrame[0]- pInfoFrame[1]- pInfoFrame[2])&0xFF  ;

        for( i = 0 ; i < pInfoFrame[2]&0x1F ; i++ )
        {
            checksum -= pInfoFrame[4+i] ;
        }
        checksum &= 0xFF ;
        pInfoFrame[3] = checksum ;

        if(Set_GeneralPurpose_PKT(pInfoFrame) == ER_SUCCESS)
        {
            return TRUE ;
        }
    }while(0) ;
    DISABLE_GeneralPurpose_PKT();
    //DISABLE_NULL_PKT() ;
    return FALSE ;
}


//~jj_tseng@chipadvanced.com 2008/08/18
//////////////////////////////////////////////////////////////////////
// Function: DumpCatHDMITXReg_66121()
// Parameter: N/A
// Return: N/A
// Remark: Debug function,dumps the registers of CAT6611.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

void
DumpCatHDMITXReg_66121()
{
#ifdef DEBUG
    int i,j ;
    BYTE ucData ;

    HDMITX_DEBUG_PRINTF(("       "));
    for(j = 0 ; j < 16 ; j++)
    {
        HDMITX_DEBUG_PRINTF((" %02X",(int)j));
        if((j == 3)||(j==7)||(j==11))
        {
            HDMITX_DEBUG_PRINTF(("  "));
        }
    }
    HDMITX_DEBUG_PRINTF(("\n        -----------------------------------------------------\n"));

    Switch_HDMITX_Bank_66121(0);

    for(i = 0 ; i < 0x100 ; i+=16)
    {
        HDMITX_DEBUG_PRINTF(("[%3X]  ",i));
        for(j = 0 ; j < 16 ; j++)
        {
            ucData = HDMITX_ReadI2C_Byte_66121((BYTE)((i+j)&0xFF));
            HDMITX_DEBUG_PRINTF((" %02X",(int)ucData));
            if((j == 3)||(j==7)||(j==11))
            {
                HDMITX_DEBUG_PRINTF((" -"));
            }
        }
        HDMITX_DEBUG_PRINTF(("\n"));
        if((i % 0x40) == 0x30)
        {
            HDMITX_DEBUG_PRINTF(("        -----------------------------------------------------\n"));
        }
    }

    Switch_HDMITX_Bank_66121(1);
    for(i = 0x130; i < 0x1B0 ; i+=16)
    {
        HDMITX_DEBUG_PRINTF(("[%3X]  ",i));
        for(j = 0 ; j < 16 ; j++)
        {
            ucData = HDMITX_ReadI2C_Byte_66121((BYTE)((i+j)&0xFF));
            HDMITX_DEBUG_PRINTF((" %02X",(int)ucData));
            if((j == 3)||(j==7)||(j==11))
            {
                HDMITX_DEBUG_PRINTF((" -"));
            }
        }
        HDMITX_DEBUG_PRINTF(("\n"));
        if(i == 0x160)
        {
            HDMITX_DEBUG_PRINTF(("        -----------------------------------------------------\n"));
        }

    }
    Switch_HDMITX_Bank_66121(0);
#endif
}

BOOL getHDMITX_AuthenticationDone()//2015.3.24, my.wei add
{
    //HDCP_DEBUG_PRINTF((" getHDMITX_AuthenticationDone() = %s\n",hdmiTxDev[0].bAuthenticated?"TRUE":"FALSE" ));
    return Instance[0].bAuthenticated;
}

