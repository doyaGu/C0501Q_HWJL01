#ifndef __MMP_HDMITX_H__
#define __MMP_HDMITX_H__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#if defined(WIN32)

#if defined(HDMITX_EXPORTS)
#define HDMITX_API __declspec(dllexport)
#else
#define HDMITX_API __declspec(dllimport)
#endif

#else
#define HDMITX_API extern
#endif //#if defined(WIN32)

#include "hdmitx/hdmitx_drv.h"
#include "hdmitx/typedef.h"
#include "hdmitx/hdmitx_sys.h"
#include "ite/mmp_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum MMP_HDMITX_VIDEO_TYPE_TAG {
    HDMITX_Unkown        = HDMI_Unkown,     
    HDMITX_640x480p60    = HDMI_640x480p60, 
    HDMITX_480p60        = HDMI_480p60,     
    HDMITX_480p60_16x9   = HDMI_480p60_16x9,
    HDMITX_720p60        = HDMI_720p60,     
    HDMITX_1080i60       = HDMI_1080i60,    
    HDMITX_480i60        = HDMI_480i60,     
    HDMITX_480i60_16x9   = HDMI_480i60_16x9,
    HDMITX_1080p60       = HDMI_1080p60,    
    HDMITX_576p50        = HDMI_576p50,     
    HDMITX_576p50_16x9   = HDMI_576p50_16x9,
    HDMITX_720p50        = HDMI_720p50,     
    HDMITX_1080i50       = HDMI_1080i50,    
    HDMITX_576i50        = HDMI_576i50,     
    HDMITX_576i50_16x9   = HDMI_576i50_16x9,
    HDMITX_1080p50       = HDMI_1080p50,    
    HDMITX_1080p24       = HDMI_1080p24,    
    HDMITX_1080p25       = HDMI_1080p25,    
    HDMITX_1080p30       = HDMI_1080p30,    
    HDMITX_720p30        = HDMI_720p30,
    HDMITX_VESA          = HDMI_VESA    
} MMP_HDMITX_VIDEO_TYPE;

typedef enum MMP_HDMITX_COLOR_MODE_TAG {
    HDMITX_RGB444        = HDMI_RGB444,
    HDMITX_YUV444        = HDMI_YUV444,
    HDMITX_YUV422        = HDMI_YUV422 
} MMP_HDMITX_COLOR_MODE;

typedef enum MMP_HDMITX_INPUT_DEVICE_TAG {
    MMP_HDHITX_IN_HDMIRX = TXIN_HDMIRX,
    MMP_HDHITX_IN_CVBS = TXIN_CVBS,    
    MMP_HDHITX_IN_YPBPR = TXIN_YPBPR    
} MMP_HDMITX_INPUT_DEVICE;


typedef enum MMP_HDMITX_VESA_ID_TAG {
    HDMITX_UNKNOWN_MODE          = UNKNOWN_MODE,
    HDMITX_CEA_640x480p60        = CEA_640x480p60,
    HDMITX_CEA_720x480p60        = CEA_720x480p60,
    HDMITX_CEA_1280x720p60       = CEA_1280x720p60,
    HDMITX_CEA_1920x1080i60      = CEA_1920x1080i60,
    HDMITX_CEA_720x480i60        = CEA_720x480i60,
    HDMITX_CEA_720x240p60        = CEA_720x240p60,
    HDMITX_CEA_1440x480i60       = CEA_1440x480i60,
    HDMITX_CEA_1440x240p60       = CEA_1440x240p60,
    HDMITX_CEA_2880x480i60       = CEA_2880x480i60,
    HDMITX_CEA_2880x240p60       = CEA_2880x240p60,
    HDMITX_CEA_1440x480p60       = CEA_1440x480p60,
    HDMITX_CEA_1920x1080p60      = CEA_1920x1080p60,
    HDMITX_CEA_720x576p50        = CEA_720x576p50,
    HDMITX_CEA_1280x720p50       = CEA_1280x720p50,
    HDMITX_CEA_1920x1080i50      = CEA_1920x1080i50,
    HDMITX_CEA_720x576i50        = CEA_720x576i50,
    HDMITX_CEA_1440x576i50       = CEA_1440x576i50,
    HDMITX_CEA_720x288p50        = CEA_720x288p50,
    HDMITX_CEA_1440x288p50       = CEA_1440x288p50,
    HDMITX_CEA_2880x576i50       = CEA_2880x576i50,
    HDMITX_CEA_2880x288p50       = CEA_2880x288p50,
    HDMITX_CEA_1440x576p50       = CEA_1440x576p50,
    HDMITX_CEA_1920x1080p50      = CEA_1920x1080p50,
    HDMITX_CEA_1920x1080p24      = CEA_1920x1080p24,
    HDMITX_CEA_1920x1080p25      = CEA_1920x1080p25,
    HDMITX_CEA_1920x1080p30      = CEA_1920x1080p30,
    HDMITX_VESA_640x350p85       = VESA_640x350p85,
    HDMITX_VESA_640x400p85       = VESA_640x400p85,
    HDMITX_VESA_720x400p85       = VESA_720x400p85,
    HDMITX_VESA_640x480p60       = VESA_640x480p60,
    HDMITX_VESA_640x480p72       = VESA_640x480p72,
    HDMITX_VESA_640x480p75       = VESA_640x480p75,
    HDMITX_VESA_640x480p85       = VESA_640x480p85,
    HDMITX_VESA_800x600p56       = VESA_800x600p56,
    HDMITX_VESA_800x600p60       = VESA_800x600p60,
    HDMITX_VESA_800x600p72       = VESA_800x600p72,
    HDMITX_VESA_800x600p75       = VESA_800x600p75,
    HDMITX_VESA_800X600p85       = VESA_800X600p85,
    HDMITX_VESA_840X480p60       = VESA_840X480p60,
    HDMITX_VESA_1024x768p60      = VESA_1024x768p60,
    HDMITX_VESA_1024x768p70      = VESA_1024x768p70,
    HDMITX_VESA_1024x768p75      = VESA_1024x768p75,
    HDMITX_VESA_1024x768p85      = VESA_1024x768p85,
    HDMITX_VESA_1152x864p75      = VESA_1152x864p75,
    HDMITX_VESA_1280x768p60R     = VESA_1280x768p60R,
    HDMITX_VESA_1280x768p60      = VESA_1280x768p60,
    HDMITX_VESA_1280x768p75      = VESA_1280x768p75,
    HDMITX_VESA_1280x768p85      = VESA_1280x768p85,
    HDMITX_VESA_1280x800p60      = VESA_1280x800p60,
    HDMITX_VESA_1280x960p60      = VESA_1280x960p60,
    HDMITX_VESA_1280x960p85      = VESA_1280x960p85,
    HDMITX_VESA_1280x1024p60     = VESA_1280x1024p60,
    HDMITX_VESA_1280x1024p75     = VESA_1280x1024p75,
    HDMITX_VESA_1280X1024p85     = VESA_1280X1024p85,
    HDMITX_VESA_1360X768p60      = VESA_1360X768p60,
    HDMITX_VESA_1400x768p60R     = VESA_1400x768p60R,
    HDMITX_VESA_1400x768p60      = VESA_1400x768p60,
    HDMITX_VESA_1400x1050p75     = VESA_1400x1050p75,
    HDMITX_VESA_1400x1050p85     = VESA_1400x1050p85,
    HDMITX_VESA_1440x900p60R     = VESA_1440x900p60R,
    HDMITX_VESA_1440x900p60      = VESA_1440x900p60,
    HDMITX_VESA_1440x900p75      = VESA_1440x900p75,
    HDMITX_VESA_1440x900p85      = VESA_1440x900p85,
    HDMITX_VESA_1600x900p60      = VESA_1600x900p60,
    HDMITX_VESA_1600x1200p60     = VESA_1600x1200p60,
    HDMITX_VESA_1600x1200p65     = VESA_1600x1200p65,
    HDMITX_VESA_1600x1200p70     = VESA_1600x1200p70,
    HDMITX_VESA_1600x1200p75     = VESA_1600x1200p75,
    HDMITX_VESA_1600x1200p85     = VESA_1600x1200p85,
    HDMITX_VESA_1680x1050p60R    = VESA_1680x1050p60R,
    HDMITX_VESA_1680x1050p60     = VESA_1680x1050p60,
    HDMITX_VESA_1680x1050p75     = VESA_1680x1050p75,
    HDMITX_VESA_1680x1050p85     = VESA_1680x1050p85,
    HDMITX_VESA_1792x1344p60     = VESA_1792x1344p60,
    HDMITX_VESA_1792x1344p75     = VESA_1792x1344p75,
    HDMITX_VESA_1856x1392p60     = VESA_1856x1392p60,
    HDMITX_VESA_1856x1392p75     = VESA_1856x1392p75,
    HDMITX_VESA_1920x1200p60R    = VESA_1920x1200p60R,
    HDMITX_VESA_1920x1200p60     = VESA_1920x1200p60,
    HDMITX_VESA_1920x1200p75     = VESA_1920x1200p75,
    HDMITX_VESA_1920x1200p85     = VESA_1920x1200p85,
    HDMITX_VESA_1920x1440p60     = VESA_1920x1440p60,
    HDMITX_VESA_1920x1440p75     = VESA_1920x1440p75,
    HDMITX_VESA_GET_HDMIRX_DE    = VESA_GET_HDMIRX_DE,
} MMP_HDMITX_VESA_ID;

//=============================================================================
//                              Structure Definition
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


//=============================================================================
//                Private Function Definition
//=============================================================================


//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
 * HDMI TX initialization.
 */
//=============================================================================
HDMITX_API void 
mmpHDMITXInitialize(
    MMP_HDMITX_INPUT_DEVICE inputDevice);

//=============================================================================
/**
 * HDMI TX Loop Process.
 */
//=============================================================================
void 
mmpHDMITXDevLoopProc(
    MMP_BOOL bHDMIRxStable, 
    MMP_UINT32 AudioSampleRate, 
    MMP_UINT32 AudioChannelNum,
    MMP_HDMITX_INPUT_DEVICE inputDevice);

//=============================================================================
/**
 * HDMI TX Disable.
 */
//=============================================================================

void
mmpHDMITXDisable(
    void);

//=============================================================================
/**
 * HDMI TX AV mute.
 */
//=============================================================================
void
mmpHDMITXAVMute(
     MMP_BOOL isEnable);

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
    MMP_BOOL              IsYUVInput);

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
    MMP_UINT32  VDEE);
    
//=============================================================================
/**
 * HDMI TX check IC.
 */
//=============================================================================
MMP_BOOL
mmpHDMITXIsChipEmpty(
    void);

MMP_BOOL
mmpHDMITXIs66121Chip(
    void);

void
mmpHDMITXSetAVForceMute(
		void);


#ifdef __cplusplus
}
#endif

#endif


