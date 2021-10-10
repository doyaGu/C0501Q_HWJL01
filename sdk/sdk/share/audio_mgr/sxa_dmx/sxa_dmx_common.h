//*****************************************************************************
// Name: sxa_dmx_common.h
//
// Description:
//     Common Header File for SXA Demuxers
//
// Copyright (c) 2008 ITE Technology Corp. All Rights Reserved.
//*****************************************************************************

#ifndef _SXA_DMX_COMMON_H_
#define _SXA_DMX_COMMON_H_

//=============================================================================
//                              Include Files
//=============================================================================
#include    "sxa_dmx.h"

#if         (SXA_CONFIG_DPF)
//#include    "mmp_types.h"
#else       /* (SXA_CONFIG_DPF) */
//#include    "mmp/type.h"
#endif      /* (SXA_CONFIG_DPF) */

//=============================================================================
//                              Constant Definition
//=============================================================================

#ifndef NULL
    #define NULL 0
#endif

typedef unsigned long GPP_RESULT;

#ifndef GPP_SUCCESS
    #define GPP_SUCCESS                                    (0x00000000)
#endif

#ifndef GPP_ERROR_BASE
    #define GPP_ERROR_BASE                                 (MMP_MODULE_3GPP << MMP_ERROR_OFFSET)
// == encode error code
    #define GPP_ERROR_ENCODE_UNSUPPORT_VIDEO_TYPE          (GPP_ERROR_BASE + 0x0001)
    #define GPP_ERROR_ENCODE_MALLOC                        (GPP_ERROR_BASE + 0x0002)

// decode error code
    #define GPP_ERROR_DECODE_FILE_PARSE                    (GPP_ERROR_BASE + 0x1000)
    #define GPP_ERROR_DECODE_MOOV_PARSE                    (GPP_ERROR_BASE + 0x1001)

    #define GPP_ERROR_DECODE_GET_MVHD_BOX                  (GPP_ERROR_BASE + 0x1002)
    #define GPP_ERROR_DECODE_GET_TKHD_BOX                  (GPP_ERROR_BASE + 0x1003)
    #define GPP_ERROR_DECODE_GET_MDHD_BOX                  (GPP_ERROR_BASE + 0x1004)
    #define GPP_ERROR_DECODE_GET_HDLR_BOX                  (GPP_ERROR_BASE + 0x1005)
    #define GPP_ERROR_DECODE_GET_STSD_BOX                  (GPP_ERROR_BASE + 0x1006)
    #define GPP_ERROR_DECODE_GET_STTS_BOX                  (GPP_ERROR_BASE + 0x1007)
    #define GPP_ERROR_DECODE_GET_STSC_BOX                  (GPP_ERROR_BASE + 0x1008)
    #define GPP_ERROR_DECODE_GET_STSZ_BOX                  (GPP_ERROR_BASE + 0x1009)
    #define GPP_ERROR_DECODE_GET_STCO_BOX                  (GPP_ERROR_BASE + 0x100a)
    #define GPP_ERROR_DECODE_GET_STSS_BOX                  (GPP_ERROR_BASE + 0x100b)
    #define GPP_ERROR_DECODE_GET_H263_SAMPLE_ENTRY         (GPP_ERROR_BASE + 0x100c)
    #define GPP_ERROR_DECODE_GET_MP4V_SAMPLE_ENTRY         (GPP_ERROR_BASE + 0x100d)
    #define GPP_ERROR_DECODE_GET_AMR_SAMPLE_ENTRY          (GPP_ERROR_BASE + 0x100e)
    #define GPP_ERROR_DECODE_GET_ESD_BOX                   (GPP_ERROR_BASE + 0x100f)
    #define GPP_ERROR_DECODE_GET_MDAT_BOX                  (GPP_ERROR_BASE + 0x1010)
    #define GPP_ERROR_DECODE_GET_MOOV_BOX                  (GPP_ERROR_BASE + 0x1011)
    #define GPP_ERROR_DECODE_GET_SAMPLE_INFO               (GPP_ERROR_BASE + 0x1012)
    #define GPP_ERROR_DECODE_GET_FIRST_VIDEO_TRAK          (GPP_ERROR_BASE + 0x1013)
    #define GPP_ERROR_DECODE_GET_FIRST_AUDIO_TRAK          (GPP_ERROR_BASE + 0x1014)

    #define GPP_ERROR_DECODE_GPPDATA_HANDLER_INVALID       (GPP_ERROR_BASE + 0x1100)
    #define GPP_ERROR_DECODE_FILE_OPEN                     (GPP_ERROR_BASE + 0x1101)
    #define GPP_ERROR_DECODE_FILE_SEEK                     (GPP_ERROR_BASE + 0x1102)
    #define GPP_ERROR_DECODE_FILE_READ                     (GPP_ERROR_BASE + 0x1103)
    #define GPP_ERROR_DECODE_MALLOC                        (GPP_ERROR_BASE + 0x1104)
    #define GPP_ERROR_DECODE_BUFFER_NULL                   (GPP_ERROR_BASE + 0x1105)

    #define GPP_ERROR_DECODE_MPG_VOL_START_CODE            (GPP_ERROR_BASE + 0x1200)
    #define GPP_ERROR_DECODE_MPG_VOL_TYPE                  (GPP_ERROR_BASE + 0x1201)
    #define GPP_ERROR_DECODE_MPG_VOL_SHAP                  (GPP_ERROR_BASE + 0x1202)
    #define GPP_ERROR_DECODE_MPG_VOL_INTERLACE             (GPP_ERROR_BASE + 0x1203)
    #define GPP_ERROR_DECODE_MPG_VOL_OBMC                  (GPP_ERROR_BASE + 0x1204)
    #define GPP_ERROR_DECODE_MPG_VOL_SPRITE                (GPP_ERROR_BASE + 0x1205)
    #define GPP_ERROR_DECODE_MPG_VOL_QUANT_TYPE            (GPP_ERROR_BASE + 0x1206)
    #define GPP_ERROR_DECODE_MPG_VOL_COMPLEXITY            (GPP_ERROR_BASE + 0x1207)
    #define GPP_ERROR_DECODE_MPG_VOL_SCALABILITY           (GPP_ERROR_BASE + 0x1208)

    #define GPP_ERROR_DECODE_MPG_VOP_CODING_TYPE           (GPP_ERROR_BASE + 0x1300)
    #define GPP_ERROR_DECODE_MPG_SHEADER_RESOLUTION        (GPP_ERROR_BASE + 0x1301)

    #define GPP_ERROR_DECODE_AUDIO_PROFILE_SUPPORT         (GPP_ERROR_BASE + 0x1400)
    #define GPP_ERROR_DECODE_AUDIO_SAMPLE_FREQ_NOT_SUPPORT (GPP_ERROR_BASE + 0x1401)

    #define GPP_ERROR_DECODE_SEEK_AUDIO_SAMPLE_NOT_EXIST   (GPP_ERROR_BASE + 0x1500)

// == stream error code
    #define GPP_ERROR_STREAM_FILE_OPEN                     (GPP_ERROR_BASE + 0x2000)
    #define GPP_ERROR_STREAM_FILE_READ                     (GPP_ERROR_BASE + 0x2001)
    #define GPP_ERROR_STREAM_MALLOC                        (GPP_ERROR_BASE + 0x2002)

    #define GPP_ERROR_STREAM_CREATE_TASK                   (GPP_ERROR_BASE + 0x2020)
    #define GPP_ERROR_STREAM_CREATE_SEMAPHORE              (GPP_ERROR_BASE + 0x2021)
    #define GPP_ERROR_STREAM_OBTAIN_SEMAPHORE              (GPP_ERROR_BASE + 0x2022)
    #define GPP_ERROR_STREAM_CREATE_EVENT                  (GPP_ERROR_BASE + 0x2023)
    #define GPP_ERROR_STREAM_SET_EVENT                     (GPP_ERROR_BASE + 0x2024)
    #define GPP_ERROR_STREAM_DELETE_TASK                   (GPP_ERROR_BASE + 0x2025)

    #define GPP_ERROR_STREAM_READ_BUFFER_NOT_ENOUGH        (GPP_ERROR_BASE + 0x2101)
    #define GPP_ERROR_STREAM_READ_SIZE_LARGE_THEN_BUFFER   (GPP_ERROR_BASE + 0x2102)
    #define GPP_ERROR_STREAM_READ_MMC_SLOW                 (GPP_ERROR_BASE + 0x2103)
    #define GPP_ERROR_TEST1                                (GPP_ERROR_BASE + 0x2104)
    #define GPP_ERROR_TEST2                                (GPP_ERROR_BASE + 0x2105)
    #define GPP_ERROR_TEST3                                (GPP_ERROR_BASE + 0x2106)
#endif

#ifndef _3GPPCODEC_H_
    #define _3GPPCODEC_H_
    #define GPP4_FILE_TYPE                                 0x33677034 //'3gp4'
    #define ISOM_FILE_TYPE                                 0x69736f6d //'isom'

    #define FTYP_BOX_TYPE                                  0x66747970 //'ftyp'
    #define MOOV_BOX_TYPE                                  0x6d6f6f76 //'moov'
    #define MDAT_BOX_TYPE                                  0x6d646174 //'mdat'
    #define MVHD_BOX_TYPE                                  0x6d766864 //'mvhd'
    #define TRAK_BOX_TYPE                                  0x7472616b //'trak'
    #define TREF_BOX_TYPE                                  0x74726566 //'tref'
    #define EDTS_BOX_TYPE                                  0x65647473 //'edts'
    #define ELST_BOX_TYPE                                  0x656c7374 //'elst'
    #define TKHD_BOX_TYPE                                  0x746b6864 //'tkhd'
    #define MDIA_BOX_TYPE                                  0x6d646961 //'mdia'
    #define MDHD_BOX_TYPE                                  0x6d646864 //'mdhd'
    #define HDLR_BOX_TYPE                                  0x68646c72 //'hdlr'
    #define MINF_BOX_TYPE                                  0x6d696e66 //'minf'
    #define VMHD_BOX_TYPE                                  0x766d6864 //'vmhd'
    #define SMHD_BOX_TYPE                                  0x736d6864 //'smhd'
    #define HMHD_BOX_TYPE                                  0x686d6864 //'hmhd'
    #define DINF_BOX_TYPE                                  0x64696e66 //'dinf'
    #define DREF_BOX_TYPE                                  0x64726566 //'dref'
    #define STBL_BOX_TYPE                                  0x7374626c //'stbl'
    #define STSD_BOX_TYPE                                  0x73747364 //'stsd'
    #define STSZ_BOX_TYPE                                  0x7374737a //'stsz'
    #define STTS_BOX_TYPE                                  0x73747473 //'stts'
    #define STSC_BOX_TYPE                                  0x73747363 //'stsc'
    #define STCO_BOX_TYPE                                  0x7374636f //'stco'
    #define STSS_BOX_TYPE                                  0x73747373 //'stss'
    #define UDTA_BOX_TYPE                                  0x75647461 //'udta'
    #define CPRT_BOX_TYPE                                  0x63707274 //'cprt'
    #define FREE_BOX_TYPE                                  0x66726565 //'free'
    #define SKIP_BOX_TYPE                                  0x736b6970 //'skip'
    #define IODS_BOX_TYPE                                  0x696f6473 //'iods'
    #define NMHD_BOX_TYPE                                  0x6e6d6864 //'nmhd'
    #define ESDS_BOX_TYPE                                  0x65736473 //'esds'
    #define META_BOX_TYPE                                  0x6d657461 //'meta'
    #define ILST_BOX_TYPE                                  0x696c7374 //'ilst'
    #define CNAM_BOX_TYPE                                  0xa96e616d //'(c)nam'
    #define CART_BOX_TYPE                                  0xa9415254 //'(c)ART'
    #define CALB_BOX_TYPE                                  0xa9616c62 //'(c)alb'

    #define VIDE_HANDLER_TYPE                              0x76696465 //'vide'
    #define SOUN_HANDLER_TYPE                              0x736f756e //'soun'
    #define HINT_HANDLER_TYPE                              0x68696e74 //'hint'
    #define SDSM_HANDLER_TYPE                              0x7364736d //'sdsm'
    #define ODSM_HANDLER_TYPE                              0x6f64736d //'odsm'
    #define URL__HANDLER_TYPE                              0x75726c20 //'url '
    #define ALIS_HANDLER_TYPE                              0x616c6973 //'alis'  //dkwang 0327

    #define AVC_VIDEO_TYPE                                 0x61766331 //'avc1'
    #define H263_VIDEO_TYPE                                0x73323633 //'s263'
    #define MP4V_VIDEO_TYPE                                0x6d703476 //'mp4v'
    #define MP4A_SOUND_TYPE                                0x6d703461 //'mp4a'
    #define ADIF_ID                                        0x41444946 //'ADIF'
    #define SAMR_SOUND_TYPE                                0x73616d72 //'samr'
    #define SAWB_SOUND_TYPE                                0x73617762 //'sawb'
    #define RAW_SOUND_TYPE                                 0x72617720 //'raw ' //dkwang 0327
    #define ULAW_SOUND_TYPE                                0x756c6177 //'uLaw' //dkwang 0327
    #define JPEG_VIDEO_TYPE                                0x6a706567 //'jpeg' //dkwang 0327
    #define MJPA_VIDEO_TYPE                                0x6d6a7061 //'mjpa' //cory 0524
    #define SOWT_SOUND_TYPE                                0x736f7774 //'sowt' //dkwang 0414
    #define TWOS_SOUND_TYPE                                0x74776f73 //'twos'

    #define DAMR_AMR_SPECIFIC_BOX                          0x64616d72
    #define SMED_VENDER_TYPE                               0x736d6564
    #define D263_H263_SPECIFIC_BOX                         0x64323633

    #define VISUAL_OBJECT_SEQUENCE_START_CODE              0x000001b0
    #define VISUAL_OBJECT_START_CODE                       0x000001b5
    #define VIDEO_OBJECT_START_CODE                        0x00000100
    #define VIDEO_OBJECT_LAYER_START_CODE                  0x00000120

    #define AMR_BITRATE_Mode475                            0x04
    #define AMR_BITRATE_Mode515                            0x0c
    #define AMR_BITRATE_Mode59                             0x14
    #define AMR_BITRATE_Mode67                             0x1c
    #define AMR_BITRATE_Mode74                             0x24
    #define AMR_BITRATE_Mode795                            0x2c
    #define AMR_BITRATE_Mode102                            0x34
    #define AMR_BITRATE_Mode122                            0x3c
    #define AMR_BITRATE_SID                                0x44
    #define AMR_BITRATE_No_Data                            0x7c
#endif

//=============================================================================
//                              Macro Definition
//=============================================================================

#ifndef _3GPPUTILITY
    #define _3GPPUTILITY

    #define SKIP_DWORD(ptr)        ptr += sizeof(unsigned long);
    #define SKIP_DWORDS(ptr, size) ptr += (sizeof(unsigned long) * (size));
    #define SKIP_WORD(ptr)         ptr += sizeof(unsigned short);
    #define SKIP_BYTE(ptr)         ptr += sizeof(unsigned char);
    #define SKIP_BYTES(ptr, size)  ptr += (sizeof(unsigned char) * (size));
//change by miller 2005/1214
    #define GET_DWORD_VALUE(ptr, dwValue)       \
    {                                   \
        dwValue  = (((unsigned long) (*(ptr))) << 24);    \
        (ptr)++;                        \
        dwValue |= (((unsigned long) (*(ptr))) << 16);    \
        (ptr)++;                        \
        dwValue |= (((unsigned long) (*(ptr))) << 8);     \
        (ptr)++;                        \
        dwValue |= (*(ptr));            \
        (ptr)++;                        \
    }

    #define SET_DWORD_VALUE(ptr, dwValue)           \
    {                                       \
        (*ptr) = (unsigned char)(dwValue >> 24);     \
        ptr++;                              \
        (*ptr) = (unsigned char)(dwValue >> 16);     \
        ptr++;                              \
        (*ptr) = (unsigned char)(dwValue >> 8);      \
        ptr++;                              \
        (*ptr) = (unsigned char)dwValue;             \
        ptr++;                              \
    }

    #define GET_WORD_VALUE(ptr, wValue) \
    {                           \
        wValue  = ((*ptr) << 8); \
        ptr++;                  \
        wValue |= (*ptr);       \
        ptr++;                  \
    }

    #define SET_WORD_VALUE(ptr, dwValue)           \
    {                                       \
        (*ptr) = (unsigned char)(dwValue >> 8);      \
        ptr++;                              \
        (*ptr) = (unsigned char)dwValue;             \
        ptr++;                              \
    }

    #define GET_BYTE_VALUE(ptr, wValue) \
    {                           \
        wValue = (*ptr);        \
        ptr++;                  \
    }

    #define SET_BYTE_VALUE(ptr, dwValue)           \
    {                                       \
        (*ptr) = (unsigned char)dwValue;             \
        ptr++;                              \
    }
    #if defined(__FREERTOS__)
        #define BS_WAP(a)      do {} while (0)
        #define BS_WAP_WORD(a) do {} while (0)
    #else // if defined(__FREERTOS__)
        #define BS_WAP(a)      ((a) = ( ((a) & 0xff) << 24) | (((a) & 0xff00) << 8) | (((a) >> 8) & 0xff00) | (((a) >> 24) & 0xff))
        #define BS_WAP_WORD(a) ((a) = (((a) & 0xff) << 8) | (((a) >> 8) & 0xff))
    #endif // if defined(__FREERTOS__)

#endif

//=============================================================================
//                              Structure Definition
//=============================================================================

#endif