#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

//=============================================================================
//                             Workaround option
//=============================================================================
#define LCD_A0_WORKAROUND

//=============================================================================
//                             Storage Mgr Option
//=============================================================================
    /** Enable Card **/
//#define DTV_INTERNAL_DEV_ENABLE  // not ready
#ifdef DTV_INTERNAL_DEV_ENABLE
      /** only one can be set to internal device **/
//    #define DTV_SET_SD1_INTERNAL
//    #define DTV_SET_SD2_INTERNAL
//    #define DTV_SET_NAND_INTERNAL  // not support
#endif

#define DTV_SD1_ENABLE
//#define DTV_SD2_ENABLE  // The 2-nd SD device
//#define DTV_MS_ENABLE
#define DTV_MMC_ENABLE

//#define DTV_USB_ENABLE
//#define DTV_PTP_ENABLE
//#define DTV_CF_ENABLE     // not working
//#define DTV_xD_ENABLE     // not working

//=============================================================================
//                             other
//=============================================================================

//#define USE_BITSTREAM_KIT
//#define USE_DMA_ENGINE
#define SUBTITLE_ISP_SUPPORT

// unused, remove later
//#define ENABLE_WEAK_SIGNAL_PROTECT

//#define ENABLE_UART_CMD
#define ENABLE_TELETEXT_TOP
//#define ENABLE_PRINT_MSG

// one table for 4 days
#define SCHEDULE_EVENT_TABLE_NUMBER 3
#define ENABLE_EXTENDED_EVENT
#ifdef ENABLE_EXTENDED_EVENT
    // available number of extended event descriptor to be collected
    #define EXTENDED_EVENT_SUM  16
#endif

//#define ENABLE_MFN
#define LIMIT_SCHEDULE_EVENT_SIZE   819200
//#define SMTK_FONT_SUPPORT_ENCODING_CONVERTER

//#define ENABLE_AUDIO_UPSAMPLING
#ifdef ENABLE_AUDIO_UPSAMPLING
    //#define AUDIO_UPSAMPLING_2X
#endif

#ifdef CUSTOMER_RELEASE
    #define SMTK_WATCHDOG_ENABLE
    #define SMTK_WATCHDOG_TIMEOUT   6
#endif

// play dvb-subtitle of recording file
//#define ENABLE_PVR_SUBTITLE

// isdb-t one seg
//#define ENABLE_ISDB_T_ONE_SEG

//=============================================================================
//                             multi-media player
//=============================================================================
#define ENABLE_TV_STC_SYNC

#define BMK_DIR_NAME     "bookmark/"  // The char '/' can not skip if you want save to a folder

//#define ENABLE_FLV_FILEFORMAT

//=============================================================================
//                             video
//=============================================================================
    /** test h264 tv channel , video not support now, audio is aac **/
//#define ENABLE_TEST_TV_AUDIO_NOT_SYNC_VIDEO_TIME_STAMP

//#define TEST_AVC_DECODER

#define ENABLE_AFD
#define ENABLE_DEINTERLACE

#if !defined(DTV_DISABLE_JPG_ENC)
//    #define ENABLE_VIDEO_CAPTURE
#endif

//#define SKIP_BROKEN_VIDEO_FRAME

//=============================================================================
//                             audio
//=============================================================================
// codec selection
//#define ENABLE_MOSA
//#define ENABLE_UMC_MOSA
#define ENABLE_UMC
//#define ENABLE_WM8711
//#define ENABLE_WM8728

//#define MUSIC_PLAYER_SUPPORT_WAV

//=============================================================================
//                             Screen Size
//=============================================================================
#if !defined(TV_OUT) && !defined(LCD_TV_OUT)
#   define ENABLE_MODIFY_SCREENSIZE     0 // 0:720x576, 1:320x240
#else
#       define PAL_SCREEN_WIDTH     720
#       define PAL_SCREEN_HEIGHT    576
#       define NTSC_SCREEN_WIDTH    720
#       define NTSC_SCREEN_HEIGHT   480
#       define LCD_SCREEN_WIDTH     720
#       define LCD_SCREEN_HEIGHT    576
#endif

#if defined ENABLE_MODIFY_SCREENSIZE
#   if (ENABLE_MODIFY_SCREENSIZE == 0)
#       define LCD_SCREEN_WIDTH     720
#       define LCD_SCREEN_HEIGHT    576
#   else
#       define LCD_SCREEN_WIDTH     320
#       define LCD_SCREEN_HEIGHT    240
#   endif
#endif

//=============================================================================
//                             Channel Scan Wait parameter
//=============================================================================
//#define PAT_SDT_WAIT_MS 3000
//#define NIT_WAIT_MS     10000

#define TOTAL_DEMOD_COUNT           1

//=============================================================================
//                             Compressing Config
//=============================================================================
#define ENABLE_SERVICE_CONFIG_COMPRESS

#if defined (_WIN32)
    //#if defined(CORE_EXPORTS)
    //    #define CORE_API __declspec(dllexport)
    //#else
    //    #define CORE_API __declspec(dllimport)
    //#endif
    #define CORE_API extern
#else
    #define CORE_API extern
#endif

#endif
