///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <typedef.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2011/07/19
//   @fileversion: ITE_HDMITX_SAMPLE_2.03
//******************************************/

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

//////////////////////////////////////////////////
// data type
//////////////////////////////////////////////////
#ifdef _MCU_8051_
typedef bit BOOL ;
#define _CODE code
#define _IDATA idata
#define _XDATA xdata
#define _DATA data
#else
typedef int BOOL ;
#define _DATA
#define _CODE
#define _IDATA
#define _XDATA
#endif // _MCU_8051_


typedef char CHAR,*PCHAR ;
typedef unsigned char uchar,*puchar ;
typedef unsigned char UCHAR,*PUCHAR ;
typedef unsigned char byte,*pbyte ;
typedef unsigned char BYTE,*PBYTE ;

typedef short SHORT,*PSHORT ;
//typedef unsigned short ushort int,*pushort ;
//typedef unsigned short USHORT,*PUSHORT ;
typedef unsigned short word,*pword ;
typedef unsigned short WORD,*PWORD ;

typedef long LONG,*PLONG ;
typedef unsigned long ulong,*pulong ;
typedef unsigned long ULONG,*PULONG ;
typedef unsigned long dword,*pdword ;
typedef unsigned long DWORD,*PDWORD ;

#define FALSE 0
#define TRUE 1

#define SUCCESS 0
#define FAIL -1

#define ON 1
#define OFF 0

typedef enum _SYS_STATUS {
    ER_SUCCESS = 0,
    ER_FAIL,
    ER_RESERVED
} SYS_STATUS ;

//2009/11/11 modified by Jau-chih.tseng@ite.com.tw
#define ABS(x) (((x)>=0)?(x):(-(x)))
//~Jau-chih.tseng@ite.com.tw 2009/11/11


typedef enum _Video_State_Type {
    VSTATE_PwrOff = 0,
    VSTATE_SyncWait ,
    VSTATE_SWReset,
    VSTATE_SyncChecking,
    VSTATE_HDCPSet,
    VSTATE_HDCP_Reset,
    VSTATE_ModeDetecting,
    VSTATE_VideoOn,
    VSTATE_Reserved
} Video_State_Type ;


typedef enum _Audio_State_Type {
    ASTATE_AudioOff = 0,
    ASTATE_RequestAudio ,
    ASTATE_ResetAudio,
    ASTATE_WaitForReady,
    ASTATE_AudioOn ,
    ASTATE_Reserved
} Audio_State_Type ;

typedef enum _TXVideo_State_Type {
    TXVSTATE_Unplug = 0,
    TXVSTATE_HPD,
    TXVSTATE_WaitForMode,
    TXVSTATE_WaitForVStable,
    TXVSTATE_VideoInit,
    TXVSTATE_VideoSetup,
    TXVSTATE_VideoOn,
    TXVSTATE_Reserved
} TXVideo_State_Type ;


typedef enum _TXAudio_State_Type {
    TXASTATE_AudioOff = 0,
    TXASTATE_AudioPrepare,
    TXASTATE_AudioOn,
    TXASTATE_AudioFIFOFail,
    TXASTATE_Reserved
} TXAudio_State_Type ;


typedef enum tagTXInput_Device{
    TXIN_HDMIRX,
    TXIN_CVBS,    
    TXIN_YPBPR       
} TXInput_Device;


typedef enum {
    PCLK_LOW = 0 ,
    PCLK_MEDIUM,
    PCLK_HIGH
} VIDEOPCLKLEVEL ;

///////////////////////////////////////////////////////////////////////
// Video Data Type
///////////////////////////////////////////////////////////////////////
#define F_MODE_RGB24  0
#define F_MODE_RGB444  0
#define F_MODE_YUV422 1
#define F_MODE_YUV444 2
#define F_MODE_CLRMOD_MASK 3


#define F_MODE_INTERLACE  1

#define F_MODE_ITU709  (1<<4)
#define F_MODE_ITU601  0

#define F_MODE_0_255   0
#define F_MODE_16_235  (1<<5)

#define F_MODE_EN_UDFILT (1<<6) // output mode only,and loaded from EEPROM
#define F_MODE_EN_DITHER (1<<7) // output mode only,and loaded from EEPROM


typedef union _VideoFormatCode
{
    struct _VFC
    {
        BYTE colorfmt:2 ;
        BYTE interlace:1 ;
        BYTE Colorimetry:1 ;
        BYTE Quantization:1 ;
        BYTE UpDownFilter:1 ;
        BYTE Dither:1 ;
    } VFCCode ;
    unsigned char VFCByte ;
} VideoFormatCode ;

#define T_MODE_CCIR656 (1<<0)
#define T_MODE_SYNCEMB (1<<1)
#define T_MODE_INDDR   (1<<2)
#define T_MODE_PCLKDIV2 (1<<3)
#define T_MODE_DEGEN (1<<4)
#define T_MODE_SYNCGEN (1<<5)
//////////////////////////////////////////////////////////////////
// Audio relate definition and macro.
//////////////////////////////////////////////////////////////////


// 2008/08/15 added by jj_tseng@chipadvanced
#define F_AUDIO_ON  (1<<7)
#define F_AUDIO_HBR (1<<6)
#define F_AUDIO_DSD (1<<5)
#define F_AUDIO_NLPCM (1<<4)
#define F_AUDIO_LAYOUT_1 (1<<3)
#define F_AUDIO_LAYOUT_0 (0<<3)


// HBR - 1100
// DSD - 1010
// NLPCM - 1001
// LPCM - 1000

#define T_AUDIO_MASK 0xF0
#define T_AUDIO_OFF 0
#define T_AUDIO_HBR (F_AUDIO_ON|F_AUDIO_HBR)
#define T_AUDIO_DSD (F_AUDIO_ON|F_AUDIO_DSD)
#define T_AUDIO_NLPCM (F_AUDIO_ON|F_AUDIO_NLPCM)
#define T_AUDIO_LPCM (F_AUDIO_ON)

// for sample clock
#define AUDFS_22p05KHz  4
#define AUDFS_44p1KHz 0
#define AUDFS_88p2KHz 8
#define AUDFS_176p4KHz    12

#define AUDFS_24KHz  6
#define AUDFS_48KHz  2
#define AUDFS_96KHz  10
#define AUDFS_192KHz 14

#define AUDFS_768KHz 9

#define AUDFS_32KHz  3
#define AUDFS_OTHER    1

// Audio Enable
#define ENABLE_SPDIF    (1<<4)
#define ENABLE_I2S_SRC3  (1<<3)
#define ENABLE_I2S_SRC2  (1<<2)
#define ENABLE_I2S_SRC1  (1<<1)
#define ENABLE_I2S_SRC0  (1<<0)

#define AUD_SWL_NOINDICATE  0x0
#define AUD_SWL_16          0x2
#define AUD_SWL_17          0xC
#define AUD_SWL_18          0x4
#define AUD_SWL_20          0xA // for maximum 20 bit
#define AUD_SWL_21          0xD
#define AUD_SWL_22          0x5
#define AUD_SWL_23          0x9
#define AUD_SWL_24          0xB


/////////////////////////////////////////////////////////////////////
// Packet and Info Frame definition and datastructure.
/////////////////////////////////////////////////////////////////////

#define VENDORSPEC_INFOFRAME_TYPE 0x01
#define AVI_INFOFRAME_TYPE  0x02
#define SPD_INFOFRAME_TYPE 0x03
#define AUDIO_INFOFRAME_TYPE 0x04
#define MPEG_INFOFRAME_TYPE 0x05

#define VENDORSPEC_INFOFRAME_VER 0x01
#define AVI_INFOFRAME_VER  0x02
#define SPD_INFOFRAME_VER 0x01
#define AUDIO_INFOFRAME_VER 0x01
#define MPEG_INFOFRAME_VER 0x01

#define VENDORSPEC_INFOFRAME_LEN 8
#define AVI_INFOFRAME_LEN 13
#define SPD_INFOFRAME_LEN 25
#define AUDIO_INFOFRAME_LEN 10
#define MPEG_INFOFRAME_LEN 10

#define ACP_PKT_LEN 9
#define ISRC1_PKT_LEN 16
#define ISRC2_PKT_LEN 16

typedef union _AVI_InfoFrame
{
    struct {
        BYTE Type ;
        BYTE Ver ;
        BYTE Len ;

        BYTE Scan:2 ;
        BYTE BarInfo:2 ;
        BYTE ActiveFmtInfoPresent:1 ;
        BYTE ColorMode:2 ;
        BYTE FU1:1 ;

        BYTE ActiveFormatAspectRatio:4 ;
        BYTE PictureAspectRatio:2 ;
        BYTE Colorimetry:2 ;

        BYTE Scaling:2 ;
        BYTE FU2:6 ;

        BYTE VIC:7 ;
        BYTE FU3:1 ;

        BYTE PixelRepetition:4 ;
        BYTE FU4:4 ;

        SHORT Ln_End_Top ;
        SHORT Ln_Start_Bottom ;
        SHORT Pix_End_Left ;
        SHORT Pix_Start_Right ;
    } info ;
    struct {
        BYTE AVI_HB[3] ;
        BYTE AVI_DB[AVI_INFOFRAME_LEN] ;
    } pktbyte ;
} AVI_InfoFrame ;

typedef union _Audio_InfoFrame {

    struct {
        BYTE Type ;
        BYTE Ver ;
        BYTE Len ;

        BYTE AudioChannelCount:3 ;
        BYTE RSVD1:1 ;
        BYTE AudioCodingType:4 ;

        BYTE SampleSize:2 ;
        BYTE SampleFreq:3 ;
        BYTE Rsvd2:3 ;

        BYTE FmtCoding ;

        BYTE SpeakerPlacement ;

        BYTE Rsvd3:3 ;
        BYTE LevelShiftValue:4 ;
        BYTE DM_INH:1 ;
    } info ;

    struct {
        BYTE AUD_HB[3] ;
        BYTE AUD_DB[AUDIO_INFOFRAME_LEN] ;
    } pktbyte ;

} Audio_InfoFrame ;

typedef union _MPEG_InfoFrame {
    struct {
        BYTE Type ;
        BYTE Ver ;
        BYTE Len ;

        ULONG MpegBitRate ;

        BYTE MpegFrame:2 ;
        BYTE Rvsd1:2 ;
        BYTE FieldRepeat:1 ;
        BYTE Rvsd2:3 ;
    } info ;
    struct {
        BYTE MPG_HB[3] ;
        BYTE MPG_DB[MPEG_INFOFRAME_LEN] ;
    } pktbyte ;
} MPEG_InfoFrame ;

// Source Product Description
typedef union _SPD_InfoFrame {
    struct {
        BYTE Type ;
        BYTE Ver ;
        BYTE Len ;

        char VN[8] ; // vendor name character in 7bit ascii characters
        char PD[16] ; // product description character in 7bit ascii characters
        BYTE SourceDeviceInfomation ;
    } info ;
    struct {
        BYTE SPD_HB[3] ;
        BYTE SPD_DB[SPD_INFOFRAME_LEN] ;
    } pktbyte ;
} SPD_InfoFrame ;

///////////////////////////////////////////////////////////////////////////
// Using for interface.
///////////////////////////////////////////////////////////////////////////
struct VideoTiming {
    ULONG VideoPixelClock ;
    BYTE VIC ;
    BYTE pixelrep ;
    BYTE outputVideoMode ;
} ;


/////////////////////////////////////////
// RX Capability.
/////////////////////////////////////////
typedef struct {
    BYTE b16bit:1 ;
    BYTE b20bit:1 ;
    BYTE b24bit:1 ;
    BYTE Rsrv:5 ;
} LPCM_BitWidth ;

typedef enum {
    AUD_RESERVED_0 = 0 ,
    AUD_LPCM,
    AUD_AC3,
    AUD_MPEG1,
    AUD_MP3,
    AUD_MPEG2,
    AUD_AAC,
    AUD_DTS,
    AUD_ATRAC,
    AUD_ONE_BIT_AUDIO,
    AUD_DOLBY_DIGITAL_PLUS,
    AUD_DTS_HD,
    AUD_MAT_MLP,
    AUD_DST,
    AUD_WMA_PRO,
    AUD_RESERVED_15
} AUDIO_FORMAT_CODE ;

typedef union {
    struct {
        BYTE channel:3 ;
        BYTE AudioFormatCode:4 ;
        BYTE Rsrv1:1 ;

        BYTE b32KHz:1 ;
        BYTE b44_1KHz:1 ;
        BYTE b48KHz:1 ;
        BYTE b88_2KHz:1 ;
        BYTE b96KHz:1 ;
        BYTE b176_4KHz:1 ;
        BYTE b192KHz:1 ;
        BYTE Rsrv2:1 ;
        BYTE ucCode ;
    } s ;
    BYTE uc[3] ;

} AUDDESCRIPTOR ;

typedef union {
    struct {
        BYTE FL_FR:1 ;
        BYTE LFE:1 ;
        BYTE FC:1 ;
        BYTE RL_RR:1 ;
        BYTE RC:1 ;
        BYTE FLC_FRC:1 ;
        BYTE RLC_RRC:1 ;
        BYTE Reserve:1 ;
        BYTE Unuse[2] ;
    } s ;
    BYTE uc[3] ;
} SPK_ALLOC ;

#define CEA_SUPPORT_UNDERSCAN (1<<7)
#define CEA_SUPPORT_AUDIO (1<<6)
#define CEA_SUPPORT_YUV444 (1<<5)
#define CEA_SUPPORT_YUV422 (1<<4)
#define CEA_NATIVE_MASK 0xF


#define HDMI_DC_SUPPORT_AI (1<<7)
#define HDMI_DC_SUPPORT_48 (1<<6)
#define HDMI_DC_SUPPORT_36 (1<<5)
#define HDMI_DC_SUPPORT_30 (1<<4)
#define HDMI_DC_SUPPORT_Y444 (1<<3)
#define HDMI_DC_SUPPORT_DVI_DUAL 1

typedef union _tag_DCSUPPORT {
    struct {
        BYTE DVI_Dual:1 ;
        BYTE Rsvd:2 ;
        BYTE DC_Y444:1 ;
        BYTE DC_30Bit:1 ;
        BYTE DC_36Bit:1 ;
        BYTE DC_48Bit:1 ;
        BYTE SUPPORT_AI:1 ;
    } info ;
    BYTE uc ;
} DCSUPPORT ;

typedef union _LATENCY_SUPPORT{
    struct {
        BYTE Rsvd:6 ;
        BYTE I_Latency_Present:1 ;
        BYTE Latency_Present:1 ;
    } info ;
    BYTE uc ;
} LATENCY_SUPPORT ;

#define HDMI_IEEEOUI 0x0c03

typedef struct _RX_CAP{
    BYTE VideoMode ;
    BYTE VDOModeCount ;
    BYTE idxNativeVDOMode ;
    BYTE VDOMode[32] ;
    BYTE AUDDesCount ;
    AUDDESCRIPTOR AUDDes[8] ;
    ULONG IEEEOUI ;
    DCSUPPORT dc ;
    BYTE MaxTMDSClock ;
    LATENCY_SUPPORT lsupport ;
    BYTE V_Latency ;
    BYTE A_Latency ;
    BYTE V_I_Latency ;
    BYTE A_I_Latency ;
    SPK_ALLOC   SpeakerAllocBlk ;
    BYTE ValidCEA:1 ;
    BYTE ValidHDMI:1 ;
} RX_CAP ;


#define PROG 1
#define INTERLACE 0
#define Vneg 0
#define Hneg 0
#define Vpos 1
#define Hpos 1

typedef struct {
    unsigned int HActive ;
    unsigned int VActive ;
    unsigned int HTotal ;
    unsigned int VTotal ;
    ULONG  PCLK ;
    unsigned int xCnt ;
    unsigned int HFrontPorch ;
    unsigned int HSyncWidth ;
    unsigned int HBackPorch ;
    unsigned int VFrontPorch ;
    unsigned int VSyncWidth ;
    unsigned int VBackPorch ;
    unsigned int ScanMode:1 ;
    unsigned int VPolarity:1 ;
    unsigned int HPolarity:1 ;
} VIDEO_Timing ;

// 2008/02/27 added by jj_tseng@chipadvanced.com
typedef enum _mode_id {
    UNKNOWN_MODE=0,
    CEA_640x480p60,
    CEA_720x480p60,
    CEA_1280x720p60,
    CEA_1920x1080i60,
    CEA_720x480i60,
    CEA_720x240p60,
    CEA_1440x480i60,
    CEA_1440x240p60,
    CEA_2880x480i60,
    CEA_2880x240p60,
    CEA_1440x480p60,
    CEA_1920x1080p60,
    CEA_720x576p50,
    CEA_1280x720p50,
    CEA_1920x1080i50,
    CEA_720x576i50,
    CEA_1440x576i50,
    CEA_720x288p50,
    CEA_1440x288p50,
    CEA_2880x576i50,
    CEA_2880x288p50,
    CEA_1440x576p50,
    CEA_1920x1080p50,
    CEA_1920x1080p24,
    CEA_1920x1080p25,
    CEA_1920x1080p30,
    VESA_640x350p85,
    VESA_640x400p85,
    VESA_720x400p85,
    VESA_640x480p60,
    VESA_640x480p72,
    VESA_640x480p75,
    VESA_640x480p85,
    VESA_800x600p56,
    VESA_800x600p60,
    VESA_800x600p72,
    VESA_800x600p75,
    VESA_800X600p85,
    VESA_840X480p60,
    VESA_1024x768p60,
    VESA_1024x768p70,
    VESA_1024x768p75,
    VESA_1024x768p85,
    VESA_1152x864p75,
    VESA_1280x768p60R,
    VESA_1280x768p60,
    VESA_1280x768p75,
    VESA_1280x768p85,
    VESA_1280x800p60,
    VESA_1280x960p60,
    VESA_1280x960p85,
    VESA_1280x1024p60,
    VESA_1280x1024p75,
    VESA_1280X1024p85,
    VESA_1360X768p60,
    VESA_1400x768p60R,
    VESA_1400x768p60,
    VESA_1400x1050p75,
    VESA_1400x1050p85,
    VESA_1440x900p60R,
    VESA_1440x900p60,
    VESA_1440x900p75,
    VESA_1440x900p85,
    VESA_1600x900p60,
    VESA_1600x1200p60,
    VESA_1600x1200p65,
    VESA_1600x1200p70,
    VESA_1600x1200p75,
    VESA_1600x1200p85,
    VESA_1680x1050p60R,
    VESA_1680x1050p60,
    VESA_1680x1050p75,
    VESA_1680x1050p85,
    VESA_1792x1344p60,
    VESA_1792x1344p75,
    VESA_1856x1392p60,
    VESA_1856x1392p75,
    VESA_1920x1200p60R,
    VESA_1920x1200p60,
    VESA_1920x1200p75,
    VESA_1920x1200p85,
    VESA_1920x1440p60,
    VESA_1920x1440p75,
    VESA_GET_HDMIRX_DE,
} MODE_ID ;

#endif // _TYPEDEF_H_
