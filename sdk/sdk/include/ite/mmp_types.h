/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * ITE Driver Type definitions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef MMP_TYPES_H
#define MMP_TYPES_H

#ifdef WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <stdio.h>
    #include <stdarg.h>
    #pragma warning(disable:4996)
#endif

/**
 * 8-bit signed quantity
 */
typedef signed char MMP_INT8;

/**
 * 8-bit unsigned quantity
 */
typedef unsigned char MMP_UINT8;

/**
 * 16-bit signed quantity
 */
typedef signed short MMP_INT16;

/**
 * 16-bit unsigned quantity
 */
typedef unsigned short MMP_UINT16;

/**
 * 32-bit signed quantity
 */
typedef signed long MMP_INT32;

/**
 * 32-bit unsigned quantity
 */
typedef unsigned long MMP_UINT32;

/**
 * Signed integer type
 */
typedef signed int MMP_INT;

/**
 * Unsigned integer type
 */
typedef unsigned int MMP_UINT;

/**
 * Signed long type
 */
typedef signed long MMP_LONG;

/**
 * Unsigned long type
 */
typedef unsigned long MMP_ULONG;

/**
 * Float type
 */
typedef float MMP_FLOAT;

/**
 * Float type
 */
typedef char MMP_CHAR;

/**
 * Register type.
 */
typedef MMP_UINT16 MMP_REG;

/**
 * 64-bit signed quantity
 */
#ifdef _MSC_VER
typedef signed __int64 MMP_INT64;
#elif defined(__TMS470__)
typedef signed long int MMP_INT64;
#else
typedef signed long long MMP_INT64;
#endif

/**
 * 64-bit unsigned quantity
 */
#ifdef _MSC_VER
typedef unsigned __int64 MMP_UINT64;
#elif defined(__TMS470__)
typedef unsigned long int MMP_UINT64;
#else
typedef unsigned long long MMP_UINT64;
#endif

/**
 * Surface handle
 */
typedef void *MMP_SURFACE;

/**
 * Real number type.
 * Is either a 32-bit fixed-point or true floating-point point quantity
 * depending on the target platform.
 */
#ifdef MMP_FIXED_POINT
typedef MMP_INT32 MMP_REAL;
#else
typedef float MMP_REAL;
#endif

/**
 *  Data type of Register.
 */
typedef MMP_UINT16 MMP_DATA;

/**
 *  ADDRESS type.
 */
typedef union
{
    struct
    {
        MMP_UINT16 low;
        MMP_UINT16 high;
    } addr;
    MMP_UINT32 fullAddr;
} MMP_ADDRESS;

//typedef MMP_UINT32 MMP_ADDRESS ;

/**
 * Boolean quantity.
 */
typedef enum MMP_BOOL_TAG
{
    /** Indicates a condition to be untrue */
    MMP_FALSE,

    /** Indicates a condition to be true */
    MMP_TRUE
} MMP_BOOL;

/**
 * Pixel format.
 */
typedef enum MMP_PIXEL_FORMAT_TAG
{
    MMP_PIXEL_FORMAT_ARGB8888,
    MMP_PIXEL_FORMAT_ARGB1555,
    MMP_PIXEL_FORMAT_ARGB4444,
    MMP_PIXEL_FORMAT_RGB565,
    MMP_PIXEL_FORMAT_UYVY,
    MMP_PIXEL_FORMAT_VYUY,
    MMP_PIXEL_FORMAT_YUY2,
    MMP_PIXEL_FORMAT_YVYU,
    MMP_PIXEL_FORMAT_YUYV,
    MMP_PIXEL_FORMAT_YUV422,
    MMP_PIXEL_FORMAT_YV12,
    MMP_PIXEL_FORMAT_YUV565
} MMP_PIXEL_FORMAT;

/**
 * Result codes.
 */
typedef enum MMP_RESULT_TAG
{
    /** No errors occurred */
    MMP_RESULT_SUCCESS = 0,

    /** Unknown error */
    MMP_RESULT_ERROR   = 1,

    /** Error if invalid display handle */
    MMP_RESULT_ERROR_INVALID_DISP,

    /** Error if invalid object (pen, brush, or font)*/
    MMP_RESULT_ERROR_INVALID_OBJECT,

    /** Error if invalid font)*/
    MMP_RESULT_ERROR_INVALID_FONT,

    /** Error if invalid rop3 value */
    MMP_RESULT_ERROR_INVALID_ROP3,

    /** Error if destination is virtual display */
    MMP_RESULT_ERROR_VIRTUAL_DEST_DISP,

    /** Error if image format of destination display is incorrect */
    MMP_RESULT_ERROR_DEST_DISP_FORMAT,

    /**
     * Error if the image format of source display is unmatched with the image
     * format of destination display.
     */
    MMP_RESULT_ERROR_SRC_DISP_FORMAT,

    /** Error if source display without alpha channel */
    MMP_RESULT_ERROR_SRC_ALPHA,

    MMP_RESULT_ERROR_MAX = 0x7FFFFFFF
} MMP_RESULT;

#define MMP_ERROR_OFFSET 16   /**< Error offset */

typedef enum MMP_MODULE_TAG   /**< Module type */
{
    MMP_MODULE_CORE,
    MMP_MODULE_LCD,
    MMP_MODULE_MMC,
    MMP_MODULE_M2D,
    MMP_MODULE_M3D,
    MMP_MODULE_DSC,
    MMP_MODULE_JPEG,
    MMP_MODULE_MPEG,
    MMP_MODULE_AUDIO,
    MMP_MODULE_3GPP,
    MMP_MODULE_USB,
    MMP_MODULE_VIDEO,
} MMP_MODULE;

/** Module bits definition */
#define MMP_BIT_CORE     (1ul << MMP_MODULE_CORE)
#define MMP_BIT_LCD      (1ul << MMP_MODULE_LCD)
#define MMP_BIT_MMC      (1ul << MMP_MODULE_MMC)
#define MMP_BIT_M2D      (1ul << MMP_MODULE_M2D)
#define MMP_BIT_M3D      (1ul << MMP_MODULE_M3D)
#define MMP_BIT_DSC      (1ul << MMP_MODULE_DSC)
#define MMP_BIT_JPEG     (1ul << MMP_MODULE_JPEG)
#define MMP_BIT_MPEG     (1ul << MMP_MODULE_MPEG)
#define MMP_BIT_AUDIO    (1ul << MMP_MODULE_AUDIO)
#define MMP_BIT_3GPP     (1ul << MMP_MODULE_3GPP)
#define MMP_BIT_USB      (1ul << MMP_MODULE_USB)
#define MMP_BIT_ALL      (~0ul)

/** Log levels definition */
#define MMP_LOG_SEVERE   0    /**< Indicating a serious failure */
#define MMP_LOG_WARNING  1    /**< Indicating a potential problem */
#define MMP_LOG_INFO     2    /**< For informational messages */
#define MMP_LOG_CONFIG   3    /**< For static configuration messages */
#define MMP_LOG_FINE     4    /**< Providing tracing information */
#define MMP_LOG_FINER    5    /**< Indicates a fairly detailed tracing message */
#define MMP_LOG_FINEST   6    /**< Indicates a highly detailed tracing message */

/** New definitions in V2 */
#define MMP_SUCCESS      MMP_RESULT_SUCCESS
#define MMP_NULL         0
#define MMP_INLINE       __inline

#define MMP_WCHAR        wchar_t

#define MMP_ZONE_ERROR   (1ul << 0)
#define MMP_ZONE_WARNING (1ul << 1)
#define MMP_ZONE_INFO    (1ul << 2)
#define MMP_ZONE_DEBUG   (1ul << 3)
#define MMP_ZONE_ENTER   (1ul << 4)
#define MMP_ZONE_LEAVE   (1ul << 5)
#define MMP_ZONE_ALL     (~0ul)

typedef unsigned int MMP_SIZE_T;
typedef void *MMP_MUTEX;
typedef void *MMP_EVENT;

#ifdef _WIN32
    #define MMP_DEBUG
#else
//#define MMP_AUDIO_CODEC_WM8778
//#define MMP_AUDIO_CODEC_WM8728
//#define MMP_AUDIO_CODEC_WM8750BL
#endif

#if !defined(WIN32)
    #define SMTK_FILE_PAL
// File Browse definitions
    #define SMTK_FILE_BROWSE_ENABLE
#endif

#define CHIP_ID_680_A0 0x68000000
#define CHIP_ID_680_A1 0x68000001

//=============================================================================
//                              Macro Definition
//=============================================================================
#if !defined CUSTOMER_RELEASE
    #define ENABLE_DEBUG_MSG_OUT
#endif

/////////////////////////////////////////////
// for Ap debug mode
extern MMP_UINT32 enable_DbgMsgFlag;

typedef enum _DBG_MSG_TYPE
{
    DBG_MSG_TYPE_ERROR          = (0x1 << 0),
    DBG_MSG_TYPE_INFO           = (0x1 << 1),
    DBG_MSG_TYPE_STREAM_READER  = (0x1 << 2),
    DBG_MSG_TYPE_JPEG_TRACE     = (0x1 << 3),
    DBG_MSG_TYPE_JPEG_INFO      = (0x1 << 4),
    DBG_MSG_TYPE_STORG_INFO     = (0x1 << 5),
    DBG_MSG_TYPE_STORG_TRACE    = (0x1 << 6),
    DBG_MSG_TYPE_FILE_INFO      = (0x1 << 7),
    DBG_MSG_TYPE_RESOURCE_ERROR = (0x1 << 8),
    DBG_MSG_TYPE_RESOURCE_INFO  = (0x1 << 9),
} DBG_MSG_TYPE;

#ifdef _WIN32
    #ifdef ENABLE_DEBUG_MSG_OUT
        #define dbg_enable(type)  (enable_DbgMsgFlag |= type)
        #define dbg_disable(type) (enable_DbgMsgFlag &= ~(type))

MMP_INLINE void
dbg_msg(DBG_MSG_TYPE type, char *string, ...)
{
    va_list  ap;
    MMP_CHAR buf[384];
    MMP_INT  result;

    if (enable_DbgMsgFlag & type)
    {
        va_start(ap, string);
        result = vsprintf(buf, string, ap);
        va_end(ap);

        if (result >= 0)
        {
            printf(buf);
        }
    }
}

MMP_INLINE void
dbg_msg_ex(DBG_MSG_TYPE type, char *string, ...)
{
}

MMP_INLINE void
trac(char *string, ...)
{
}

    #else
        #define dbg_enable(type)
        #define dbg_disable(type)
MMP_INLINE void
dbg_msg(DBG_MSG_TYPE type, char *string, ...)
{
}

MMP_INLINE void
dbg_msg_ex(DBG_MSG_TYPE type, char *string, ...)
{
}

MMP_INLINE void
trac(char *string, ...)
{
}

    #endif
#else
    #define trac(string, args ...)                 do { printf(string, ## args);                    \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__); \
} while (0)

    #ifdef ENABLE_DEBUG_MSG_OUT
        #define dbg_msg(type, string, args ...)    ((void)((type & enable_DbgMsgFlag) ? printf(string, ## args) : MMP_NULL))
        #define dbg_enable(type)                   (enable_DbgMsgFlag |= type)
        #define dbg_disable(type)                  (enable_DbgMsgFlag &= ~(type))

        #define dbg_msg_ex(type, string, args ...) do { if (type & enable_DbgMsgFlag) {               \
                                                            printf(string,         ## args);                    \
                                                            printf("  %s [#%d]\n", __FILE__, __LINE__); } \
} while (0)
    #else
        #define dbg_msg(type, string, args ...)
        #define dbg_enable(type)
        #define dbg_disable(type)
        #define dbg_msg_ex(type, string, args ...)
    #endif
#endif
/////////////////////////////////////////////

/////////////////////////////////////////////
// for SDK debug mode
extern MMP_UINT32 enable_SdkMsgFlag;

typedef enum _SDK_MSG_TYPE
{
    SDK_MSG_TYPE_ERROR     = (0x1 << 0),
    SDK_MSG_TYPE_INFO      = (0x1 << 1),
    SDK_MSG_TYPE_JPG_INFO  = (0x1 << 2),
    SDK_MSG_TYPE_JPG_REG   = (0x1 << 3),
    SDK_MSG_TYPE_JPG_DATA  = (0x1 << 4),
    SDK_MSG_TYPE_PAL_INFO  = (0x1 << 5),
    SDK_MSG_TYPE_PAL_TRACE = (0x1 << 6),
} SDK_MSG_TYPE;

#ifdef _WIN32
    #ifdef ENABLE_DEBUG_MSG_OUT
        #define sdk_enable(type)  (enable_SdkMsgFlag |= type)
        #define sdk_disable(type) (enable_SdkMsgFlag &= ~(type))

MMP_INLINE void
sdk_msg(SDK_MSG_TYPE type, char *string, ...)
{
    va_list  ap;
    MMP_CHAR buf[384];
    MMP_INT  result;

    if (enable_SdkMsgFlag & type)
    {
        va_start(ap, string);
        result = vsprintf((char *)buf, string, ap);
        va_end(ap);

        if (result >= 0)
        {
            printf((char *)buf);
        }
    }
}

        #define sdk_msg_ex(type, string, ...) do { if (enable_SdkMsgFlag & type)                    \
                                                       printf(string, __VA_ARGS__);                    \
                                                   printf("  %s [#%d]\n", __FILE__, __LINE__);     \
} while (0)

    #else
        #define sdk_enable(type)
        #define sdk_disable(type)
MMP_INLINE void
sdk_msg(SDK_MSG_TYPE type, char *string, ...)
{
}

        #define sdk_msg_ex(type, string, args, ...)

    #endif
#else
    #ifdef ENABLE_DEBUG_MSG_OUT
        #define sdk_msg(type, string, args ...)    ((void)((type & enable_SdkMsgFlag) ? printf(string, ## args) : MMP_NULL))
        #define sdk_enable(type)                   (enable_SdkMsgFlag |= type)
        #define sdk_disable(type)                  (enable_SdkMsgFlag &= ~(type))
        #define sdk_msg_ex(type, string, args ...) do { if (type & enable_SdkMsgFlag) {               \
                                                            printf(string,         ## args);                    \
                                                            printf("  %s [#%d]\n", __FILE__, __LINE__); } \
} while (0)
    #else
        #define sdk_msg(type, string, args ...)
        #define sdk_enable(type)
        #define sdk_disable(type)
        #define sdk_msg_ex(type, string, args ...)
    #endif
#endif

// For Bootloader Booting and Verifying.
#ifdef DTV_680_8M
    #ifdef KERNEL_UPDATE_ONLY
        #define IMG_RESERVED_SIZE (640 * 1024)
    #else
        #define IMG_RESERVED_SIZE (896 * 1024)
    #endif
#else
    #ifdef KERNEL_UPDATE_ONLY
        #define IMG_RESERVED_SIZE (704 * 1024)
    #else
        #define IMG_RESERVED_SIZE (1856 * 1024)
    #endif
#endif

#define HEADER_A_OFFSET_ADDR      (64 * 1024)                       // 64KB
#define HEADER_B_OFFSET_ADDR      ((64 * 1024) + IMG_RESERVED_SIZE) // 64KB + 640KB

typedef enum _HEADER_STATUS
{
    HEADER_NEW       = 0xFF,
    HEADER_VERIFYING = 0xFE,
    HEADER_VERIFIED  = 0xFC,
    HEADER_FAIL      = 0xF8,
    HEADER_OLD       = 0xF0,
    HEADER_UNKNOWN   = 0,
} HEADER_STATUS;

#define RESERVED_IMG_HEADER_SIZE (32)    // try to wirte 4 bytes but error
                                         // propagation to 32 bytes.

#define HEADER_STAT(x)             (x & 0x000000FF)
#define SET_HEADER_TO_VERIFYING(x) (x = HEADER_VERIFYING)
#define SET_HEADER_TO_FAIL(x)      (x = HEADER_FAIL)
#define SET_HEADER_TO_OLD(x)       (x = HEADER_OLD)

#define NTFS_RESULT_FLAG                 (0x8000000)

#include "pthread.h"

#define PAL_THREAD                       pthread_t

#include "ite/audio.h"

#define MMP_AUDIO_ENGINE                 ITE_AUDIO_ENGINE

#define MMP_MP3_DECODE                   ITE_MP3_DECODE
#define MMP_AAC_DECODE                   ITE_AAC_DECODE
#define MMP_AACPLUS_DECODE               ITE_AACPLUS_DECODE
#define MMP_BSAC_DECODE                  ITE_BSAC_DECODE
#define MMP_WMA_DECODE                   ITE_WMA_DECODE
#define MMP_AMR_ENCODE                   ITE_AMR_ENCODE
#define MMP_AMR_DECODE                   ITE_AMR_DECODE
#define MMP_AMR_CODEC                    ITE_AMR_CODEC
#define MMP_MIXER                        ITE_MIXER
#define MMP_MIDI                         ITE_MIDI
#define MMP_PCM_CODEC                    ITE_PCM_CODEC
#define MMP_WAV_DECODE                   ITE_WAV_DECODE
#define MMP_AC3_DECODE                   ITE_AC3_DECODE
#define MMP_OGG_DECODE                   ITE_OGG_DECODE
#define MMP_AC3_SPDIF_DECODE             ITE_AC3_SPDIF_DECODE
#define MMP_RESERVED                     ITE_RESERVED

#define MMP_WAVE_FORMAT                  ITE_WAVE_FORMAT
#define MMP_WAVE_FORMAT_PCM              ITE_WAVE_FORMAT_PCM
#define MMP_WAVE_FORMAT_ALAW             ITE_WAVE_FORMAT_ALAW
#define MMP_WAVE_FORMAT_MULAW            ITE_WAVE_FORMAT_MULAW
#define MMP_WAVE_FORMAT_DVI_ADPCM        ITE_WAVE_FORMAT_DVI_ADPCM
#define MMP_WAVE_FORMAT_SWF_ADPCM        ITE_WAVE_FORMAT_SWF_ADPCM

#define MMP_AUDIO_MODE                   ITE_AUDIO_MODE
#define MMP_AUDIO_STEREO                 ITE_AUDIO_STEREO
#define MMP_AUDIO_LEFT_CHANNEL           ITE_AUDIO_LEFT_CHANNEL
#define MMP_AUDIO_RIGHT_CHANNEL          ITE_AUDIO_RIGHT_CHANNEL
#define MMP_AUDIO_MIX_LEFT_RIGHT_CHANNEL ITE_AUDIO_MIX_LEFT_RIGHT_CHANNEL

#include "stdio.h"

//#define PAL_FILE                            FILE
//#define PAL_FILE_WB                         "wb"
//#define PAL_FILE_AB                         "ab"
//#define PAL_FILE_RB                         "rb"
//#define PAL_SEEK_SET                        SEEK_SET
//#define PAL_SEEK_END                        SEEK_END
//#define PAL_SEEK_CUR                        SEEK_CUR
#define PAL_HEAP_DEFAULT 0
//#define PalFileClose(f,z)                   fclose(f)
//#define PalFileSeek(f,p,g,z)                fseek(f,p,g)
//#define PalFileTell(f,z)                    ftell(f)
//#define PalTFileRead(d,c,s,f,z)             fread(d,c,s,f)
//#define PalFileRead(d,c,s,f,z)              fread(d,c,s,f)
//#define PalTFileWrite(d,c,s,f,z)            fwrite(d,c,s,f)
//#define PalFileWrite(d,c,s,f,z)             fwrite(d,c,s,f)
//#define PalWFileOpen(f,m,z)                 fopen(f,m)
//#define PalFileOpen(f,m,z)                  fopen(f,m)
//#define PalTFileDelete(p,z)                 remove(p)

#define infoMgr_SetServiceAttribute(a, b, c, d)
#define PalDestroyThread(thread) {   \
        void *thread_result;                                \
        pthread_join(thread, &thread_result);               \
}

static MMP_INT
mmpAudioGetAC3HWTrap(void)
{
    return MMP_RESULT_SUCCESS;
}

#endif /* MMP_TYPES_H */