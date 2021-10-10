/*
 * Copyright (c) 2006 ITE Technology Corp. All Rights Reserved.
 */
/* @file
 * Include file for MMIO
 *
 * @author Kuoping Hsu
 * @date 2006.07.26.
 * @version 1.0
 *
 */

#ifndef __MMIO_H__
#define __MMIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////
// User Define I/O
//
// MMIO: 0x1698 ~ 0x16B2   0x16e2~0x16f0
//
// 0x1698  DrvEncode_Frame   / DrvDecode_RevbBase
// 0x169a  DrvEncode_Frame+2 / DrvDecode_RevbBase+2
// 0x169c  DrvAudioCtrl2
// 0x169e  DrvDecode_WrPtr   / DrvEncodePCM_WrPtr
// 0x16a0  DrvDecode_RdPtr   / DrvEncodePCM_RdPtr
// 0x16a2  DrvAudioCtrl
// 0x16a4  DrvDecode_Frame
// 0x16a6  DrvDecode_Frame+2
// 0x16a8  Reserved
// 0x16aa  DrvEncode_WrPtr   / DrvDecodePCM_WrPtr
// 0x16ac  DrvEncode_RdPtr   / DrvDecodePCM_RdPtr
//
// 0x16e4 Audio Processor Write register protection
// 0x16e6 Audio Decoder write decode error flag
// 0x16e8 Audio Plugin Message register
// 0x16ea Audio Decoder drop data
// 0x16ec Audio Decoder start
// 0x16ee AC3 Decoder Parameter

// 0x1698 DrvDecode_RdPtr2
// 0x169a DrvDecode_WrPtr2
// 0x16ee AC3 Decoder Parameter/Flash Auido Input(format,using)

/////////////////////////////////////////////////////////////////

#define DrvDecode_RevbBase                        0x1698 /* 32 bits */
#define DrvAudioCtrl                              0x16a2 /* 16 bits */
#define DrvAudioCtrl2                             0x169c /* 16 bits */

#define AUDIO_PROCESSOR_WRITE_REGISTER_PROTECTION 0x16e4
#define AUDIO_DECODER_WRITE_DECODE_ERROR          0x16e6
#define AUDIO_PLUGIN_MESSAGE_REGISTER             0x16e8
#define AUDIO_DECODER_DROP_DATA                   0x16ea
#define AUDIO_DECODER_START_FALG                  0x16ec
#define AC3_DECODER_PARAMETER                     0x16ee
#define FLASH_AUDIO_INPUT_PARAMETER               0x16ee
#define AEC_COMMAND_SEND                          0x16ee
#define AEC_COMMAND_REPLY                         0x16f0

/////////////////////////////////////////////////////////////////
// Audio Control
/////////////////////////////////////////////////////////////////
#define DrvDecode_PAUSE_Bits                      0
#define DrvDecode_EOF_Bits                        1
#define DrvDecode_EnEQ_Bits                       2
#define DrvDecode_EnDRC_Bits                      3
#define DrvDecode_EnReverb_Bits                   4
#define DrvDecode_EnVoiceOff_Bits                 5
#define DrvDecode_STOP_Bits                       6
#define DrvDecode_EnMix_Bits                      7
#define DrvDecode_MixEOF_Bits                     10
// redefined bit 7
#define DrvDecode_EnOutputEmpty_Bits              7

#define DrvAMR_Type_Bits                          8
#define DrvPCM_Type_Bits                          8
#define DrvEncode_EOF_Bits                        10
#define DrvAMR_Mode_Bits                          11
#define DrvAMR_DTX_Bits                           14
#define DrvParamUpd_Bits                          14
#define DrvEnable_Bits                            15

/* Control bit for Audio Reset (DrvAudioCtrl2 register) */
#define DrvAudio_Reset_Bits                       15

#define DrvEncode_STOP_Bits                       DrvDecode_STOP_Bits
#define DrvEncode_PAUSE_Bits                      DrvDecode_PAUSE_Bits

/* Control bit for Audio Out */
#define DrvDecode_PAUSE                           (1 << DrvDecode_PAUSE_Bits)         // D[0]
#define DrvDecode_EOF                             (1 << DrvDecode_EOF_Bits)           // D[1]
#define DrvDecode_EnEQ                            (1 << DrvDecode_EnEQ_Bits)          // D[2]
#define DrvDecode_EnDRC                           (1 << DrvDecode_EnDRC_Bits)         // D[3]
#define DrvDecode_EnReverb                        (1 << DrvDecode_EnReverb_Bits)      // D[4]
#define DrvDecode_EnVoiceOff                      (1 << DrvDecode_EnVoiceOff_Bits)    // D[5]
#define DrvDecode_STOP                            (1 << DrvDecode_STOP_Bits)          // D[6]
#define DrvDecode_EnMix                           (1 << DrvDecode_EnMix_Bits)         // D[7]
#define DrvDecode_MixEOF                          (1 << DrvDecode_MixEOF_Bits)        // D[10]
// redefined bit 7
#define DrvDecode_EnOutputEmpty                   (1 << DrvDecode_EnOutputEmpty_Bits) // D[7]

/* Control bit for Audio In */
#define DrvAMR_Type                               (3 << DrvAMR_Type_Bits)   // D[9:8]
#define DrvPCM_Type                               (3 << DrvPCM_Type_Bits)   // D[9:8]
#define DrvEncode_EOF                             (1 << DrvDecode_EOF_Bits) // D[1] share with decoder input EOF
#define DrvAMR_Mode                               (7 << DrvAMR_Mode_Bits)   // D[13:11]
#define DrvAMR_DTX                                (1 << DrvAMR_DTX_Bits)    // D[14]
#define DrvParamUpd                               (1 << DrvParamUpd_Bits)   // D[14]
#define DrvEnable                                 (1 << DrvEnable_Bits)     // D[15]

#define DrvEncode_STOP                            DrvDecode_STOP
#define DrvEncode_PAUSE                           DrvDecode_PAUSE

#define DrvAMR_Encode                             (0 << DrvAMR_Type_Bits)
#define DrvAMR_Decode                             (1 << DrvAMR_Type_Bits)
#define DrvAMR_Codec                              (2 << DrvAMR_Type_Bits)
#define DrvAMR_Codec_AEC                          (3 << DrvAMR_Type_Bits)

#define DrvPCM_Encode                             (1 << DrvAMR_Type_Bits)
#define DrvPCM_Decode                             (2 << DrvAMR_Type_Bits)
#define DrvPCM_Codec                              (3 << DrvAMR_Type_Bits)

/* AMR Encode Mode */
#define AMR_MR475                                 0
#define AMR_MR515                                 1
#define AMR_MR59                                  2
#define AMR_MR67                                  3
#define AMR_MR74                                  4
#define AMR_MR795                                 5
#define AMR_MR102                                 6
#define AMR_MR122                                 7

/* AMR DTX Mode */
#define AMR_NODTX                                 0
#define AMR_DTX                                   1

/////////////////////////////////////////////////////////////////
// Audio Control 2
/////////////////////////////////////////////////////////////////

/*
 * For PCM Codec
 */
#define DrvPCM_DecSampsRateBits 0
#define DrvPCM_DecEndianBits    4
#define DrvPCM_DecChannelBits   5
#define DrvPCM_EncSampsRateBits 6
#define DrvPCM_EncEndianBits    10
#define DrvPCM_EncChannelBits   11
#define DrvPCM_DecSampsRate     (15 << DrvPCM_DecSampsRateBits)         // D[3:0]
#define DrvPCM_DecEndian        (1 << DrvPCM_DecEndianBits)             // D[4]
#define DrvPCM_DecChannel       (1 << DrvPCM_DecChannelBits)            // D[5]
#define DrvPCM_EncSampsRate     (15 << DrvPCM_EncSampsRateBits)         // D[9:6]
#define DrvPCM_EncEndian        (1 << DrvPCM_EncEndianBits)             // D[10]
#define DrvPCM_EncChannel       (1 << DrvPCM_EncChannelBits)            // D[11]

/*
 * For WAVE Codec
 */
#define DrvWAV_TypeBits         0
#define DrvWAV_ModeBits         3
#define DrvWAV_EncSampRateBits  4
#define DrvWAV_EncChannelBits   8
#define DrvWAV_Type             (7 << DrvWAV_TypeBits)                  // D[2:0]
#define DrvWAV_Mode             (1 << DrvWAV_ModeBits)                  // D[3]
#define DrvWAV_EncSampRate      (15 << DrvWAV_EncSampRateBits)          // D[7:4]
#define DrvWAV_EncChannel       (1 << DrvWAV_EncChannelBits)            // D[8]

// Type defined at bits DrvWAV_TypeBits
#define DrvWAV_TypePCM16        0
#define DrvWAV_TypePCM8         1
#define DrvWAV_TypeADPCM        2
#define DrvWAV_TypeALAW         3
#define DrvWAV_TypeULAW         4

// Type defined at bits DrvWAV_ModeBits
#define DrvWAV_DECODE           0
#define DrvWAV_ENCODE           1

// Mode defined at bits DrvWAV_Enc
// Sampling Rate
enum {
    WAVE_SRATE_6000  = 0,
    WAVE_SRATE_8000  = 1,
    WAVE_SRATE_11025 = 2,
    WAVE_SRATE_12000 = 3,
    WAVE_SRATE_16000 = 4,
    WAVE_SRATE_22050 = 5,
    WAVE_SRATE_24000 = 6,
    WAVE_SRATE_32000 = 7,
    WAVE_SRATE_44100 = 8,
    WAVE_SRATE_48000 = 9
};

//
// Channel Mix Mode
//
enum {
    CH_MIX_NO    = 0,
    CH_MIX_LEFT  = 1,
    CH_MIX_RIGHT = 2,
    CH_MIX_BOTH  = 3
};

/////////////////////////////////////////////////////////////////
// Status for sleep
/////////////////////////////////////////////////////////////////
#define DrvAudioStatus                   DrvAudioCtrl2
#define DrvOR32_SleepBits                0
#define DrvDownSampleBits                1
#define DrvDecode_Skip_Bits              2
#define DrvChMixMode_Bits                3
#define DrvShowSpectrumMode_Bits         8    //avoid use wave encode mmio
#define DrvMusicWithoutASFHeader_Bits    9

#define DrvEnableUpSampling_Bits         10
#define DrvUpSampling2x_Bits             11

// reset mp3 rd buffer pointer
#define DrvResetMp3RdBufPointer_Bits             11   

#define DrvResetAudioDecodedBytes_Bits   12
#define DrvWMAWithoutASFHeader_Bits      13
#define DrvOR32_Sleep                    (1 << DrvOR32_SleepBits)              // D[0]
#define DrvDownSample                    (1 << DrvDownSampleBits)              // D[1]
#define DrvDecode_Skip                   (1 << DrvDecode_Skip_Bits)            // D[2]
#define DrvChMixMode                     (3 << DrvChMixMode_Bits)              // D[4:3]
#define DrvShowSpectrum                  (1 << DrvShowSpectrumMode_Bits)       //D[8]
// use same bit
#define DrvMusicWithoutASFHeader         (1 << DrvMusicWithoutASFHeader_Bits)  //D[9]

#define DrvEnableUpSampling              (1 << DrvEnableUpSampling_Bits)       //D[10]
/* Control bit for Audio Reset (DrvAudioCtrl2 register) */
#define DrvAudio_RESET                   (1 << DrvAudio_Reset_Bits)            // D[15]

#define DrvUpSampling2x                  (1 << DrvUpSampling2x_Bits)           //D[11]

// reset mp3 rd buffer pointer
#define DrvResetMp3RdBufPointer     (1 << DrvResetMp3RdBufPointer_Bits)           //D[11]

#define DrvResetAudioDecodedBytes        (1 << DrvResetAudioDecodedBytes_Bits) //D[12]
#define DrvWMAWithoutASFHeader           (1 << DrvWMAWithoutASFHeader_Bits)    //D[13]
/////////////////////////////////////////////////////////////////
// Dobly register
/////////////////////////////////////////////////////////////////
#define AC3_DRC_MODE_BITS                0                                       // 0~1
#define AC3_DUAL_MONO_MODE_BITS          2                                       // 2~3
#define AC3_2_CHANNELS_DOWNMIX_MODE_BITS 4                                       // 4~5
#define AC3_PRINT_METADATA_MODE_BITS     6                                       // 6

#define AC3_DRC_MODE                     (3 << AC3_DRC_MODE_BITS)                // D[1:0]
#define AC3_DUAL_MONO_MODE               (3 << AC3_DUAL_MONO_MODE_BITS)          // D[3:2]
#define AC3_2_CHANNELS_DOWNMIX_MODE      (3 << AC3_2_CHANNELS_DOWNMIX_MODE_BITS) // D[5:4]
#define AC3_PRINT_METADATA_MODE          (1 << AC3_PRINT_METADATA_MODE_BITS)     // D[6]
/////////////////////////////////////////////////////////////////
// Flash Audio Plugin
/////////////////////////////////////////////////////////////////
#define FLASH_AUDIO_INPUT_0_BIT          0
#define FLASH_AUDIO_INPUT_1_BIT          1
#define FLASH_AUDIO_INPUT_2_BIT          2
#define FLASH_AUDIO_INPUT_3_BIT          3

#define FLASH_AUDIO_INPUT_0_USING_BIT    7
#define FLASH_AUDIO_INPUT_1_USING_BIT    8
#define FLASH_AUDIO_INPUT_2_USING_BIT    9
#define FLASH_AUDIO_INPUT_3_USING_BIT    10

#define FLASH_INPUT_0_FORMAT             (1 << FLASH_AUDIO_INPUT_0_BIT)       // D[0:0]
#define FLASH_INPUT_1_FORMAT             (1 << FLASH_AUDIO_INPUT_1_BIT)       // D[1:1]
#define FLASH_INPUT_2_FORMAT             (1 << FLASH_AUDIO_INPUT_2_BIT)       // D[2:2]
#define FLASH_INPUT_3_FORMAT             (1 << FLASH_AUDIO_INPUT_3_BIT)       // D[3:3]
#define FLASH_INPUT_0_USING              (1 << FLASH_AUDIO_INPUT_0_USING_BIT) // D[7]
#define FLASH_INPUT_1_USING              (1 << FLASH_AUDIO_INPUT_1_USING_BIT) // D[8]
#define FLASH_INPUT_2_USING              (1 << FLASH_AUDIO_INPUT_2_USING_BIT) // D[9]
#define FLASH_INPUT_3_USING              (1 << FLASH_AUDIO_INPUT_3_USING_BIT) // D[10]
/////////////////////////////////////////////////////////////////
// Read/Write Pointer
/////////////////////////////////////////////////////////////////
/* For Audio Decode (MP3, AAC, AMR) */
#define DrvDecode_WrPtr                  0x169e /* 16 bits */
#define DrvDecode_RdPtr                  0x16a0 /* 16 bits */
#define DrvDecode_Frame                  0x16a4 /* 32 bits */

#define DrvDecode_WrPtr1                 0x169a /* 16 bits */
#define DrvDecode_RdPtr1                 0x1698 /* 16 bits */

#define DrvDecode_WrPtr2                 0x169a /* 16 bits */
#define DrvDecode_RdPtr2                 0x1698 /* 16 bits */

/* For Audio Decode. (PCM_DATA for Decode) */
#define DrvDecodePCM_WrPtr               0x16aa                    /* 16 bits */
#define DrvDecodePCM_RdPtr               0x16ac                    /* 16 bits */
#define DrvDecodePCM_EOF                 (1 << DrvEncode_EOF_Bits) /* share with the encoder EOF */

/* For Audio Encode (AMR) */
#define DrvEncode_WrPtr                  0x16aa /* 16 bits */
#define DrvEncode_RdPtr                  0x16ac /* 16 bits */
#define DrvEncode_Frame                  0x1698 /* 32 bits */

/* For Audio Decode. (PCM_DATA for Encode) */
#define DrvEncodePCM_WrPtr               0x169e                    /* 16 bits */
#define DrvEncodePCM_RdPtr               0x16a0                    /* 16 bits */
#define DrvEncodePCM_EOF                 (1 << DrvEncode_EOF_Bits) /* share with the decoder EOF */

// For Mplayer,get decoded bytes
#define DrvAudioDecodedBytes             0x16f0 /* 16 bits */

#define DrvSPDIFMute                     0x166C
/////////////////////////////////////////////////////////////////
/* For Audio PCM Mixer Input */
/////////////////////////////////////////////////////////////////
/* Notice: temporary use the seting of DrvDecodePCM */
#define DrvMixerPCM_RdPtr                DrvDecodePCM_RdPtr
#define DrvMixerPCM_WrPtr                DrvDecodePCM_WrPtr
#define DrvMixerPCM_EOF                  (1 << DrvEncode_EOF_Bits) /* share with the decoder EOF */

/////////////////////////////////////////////////////////////////
/* For Audio PLUGIN message queue */
/////////////////////////////////////////////////////////////////
#define getAudioPluginMessageStatus()      MMIO_Read(AUDIO_PLUGIN_MESSAGE_REGISTER)
#define setAudioPluginMessageStatus(value) MMIO_Write(AUDIO_PLUGIN_MESSAGE_REGISTER, value)

/////////////////////////////////////////////////////////////////
// END OF Audio Interface
/////////////////////////////////////////////////////////////////
#if defined (CFG_CHIP_REV_A0) || defined(CFG_CHIP_PKG_IT9910)
/* For MMIO interface */
    #define MMIO_ADDR 0xC0000000
#elif defined (CFG_CHIP_REV_A1)
    #define MMIO_ADDR 0xC0200000
#else
    #define MMIO_ADDR 0xC0200000
#endif

/* For Soft Interrupt to Host */
#define MMIO_MISC     0x80000024
#define MMIO_INT_Bits 16
#define MMIO_INT      (1 << 16)

#if defined(__OPENRTOS__)
    /* For data flow control */
    #define isEQ()                     ((MMIO_Read(DrvAudioCtrl) & DrvDecode_EnEQ       ) != 0)
    #define isReverbOn()               ((MMIO_Read(DrvAudioCtrl) & DrvDecode_EnReverb   ) != 0)
    #define isDrcOn()                  ((MMIO_Read(DrvAudioCtrl) & DrvDecode_EnDRC      ) != 0)
    #define isVoiceOff()               ((MMIO_Read(DrvAudioCtrl) & DrvDecode_EnVoiceOff ) != 0)
    #define isEOF()                    ((MMIO_Read(DrvAudioCtrl) & DrvDecode_EOF        ) != 0)
    #define isSTOP()                   ((MMIO_Read(DrvAudioCtrl) & DrvDecode_STOP       ) != 0)
    #define isPAUSE()                  ((MMIO_Read(DrvAudioCtrl) & DrvDecode_PAUSE      ) != 0)
    #define isEnableMix()              ((MMIO_Read(DrvAudioCtrl) & DrvDecode_EnMix      ) != 0)
    #define isEnableOutputEmpty()      ((MMIO_Read(DrvAudioCtrl) & DrvDecode_EnOutputEmpty      ) != 0)
    #define isMixEOF()                 ((MMIO_Read(DrvAudioCtrl) & DrvDecode_MixEOF     ) != 0)
    #define isDownSample()             ((MMIO_Read(DrvAudioCtrl2) & DrvDownSample        ) != 0)
    #define isSkip()                   ((MMIO_Read(DrvAudioCtrl2) & DrvDecode_Skip       ) != 0)
    #define getChMixMode()             ((MMIO_Read(DrvAudioCtrl2) & DrvChMixMode         ) >> DrvChMixMode_Bits)
    #define isUpSample()               ((MMIO_Read(DrvAudioCtrl2) & DrvEnableUpSampling  ) != 0)
    #define isUpSampleOnly2x()         ((MMIO_Read(DrvAudioCtrl2) & DrvUpSampling2x  ) != 0)

    #define isResetMp3RdBufPointer()    ((MMIO_Read(DrvAudioCtrl2) & DrvResetMp3RdBufPointer  ) != 0)    
    /* For Mplayer,get decoded bytes  */
    #define isResetAudioDecodedBytes() ((MMIO_Read(DrvAudioCtrl2) & DrvResetAudioDecodedBytes) != 0)
    // for movie, have no asf header
    #define isWMAWithoutASFHeader()    ((MMIO_Read(DrvAudioCtrl2) & DrvWMAWithoutASFHeader) != 0)

    // for music, show spectrum
    #define isMusicShowSpectrum()  ((MMIO_Read(DrvAudioCtrl2) & DrvShowSpectrum) != 0)

    // for music, have no asf header
    #define isMusicWithoutASFHeader()  ((MMIO_Read(DrvAudioCtrl2) & DrvMusicWithoutASFHeader) != 0)

    /* Flow control for Decoder */
    #define isDecEOF()                 ((MMIO_Read(DrvAudioCtrl) & DrvDecode_EOF        ) != 0)
    #define isDecSTOP()                ((MMIO_Read(DrvAudioCtrl) & DrvDecode_STOP       ) != 0)
    #define isDecPAUSE()               ((MMIO_Read(DrvAudioCtrl) & DrvDecode_PAUSE      ) != 0)
    #define setDecEOF()                MMIO_Write(DrvAudioCtrl, (MMIO_Read(DrvAudioCtrl) | DrvDecode_EOF   ))
    #define setDecSTOP()               MMIO_Write(DrvAudioCtrl, (MMIO_Read(DrvAudioCtrl) | DrvDecode_STOP  ))
    #define setDecPAUSE()              MMIO_Write(DrvAudioCtrl, (MMIO_Read(DrvAudioCtrl) | DrvDecode_PAUSE ))
    #define clrDecEOF()                MMIO_Write(DrvAudioCtrl, (MMIO_Read(DrvAudioCtrl) & ~DrvDecode_EOF   ))
    #define clrDecSTOP()               MMIO_Write(DrvAudioCtrl, (MMIO_Read(DrvAudioCtrl) & ~DrvDecode_STOP  ))
    #define clrDecPAUSE()              MMIO_Write(DrvAudioCtrl, (MMIO_Read(DrvAudioCtrl) & ~DrvDecode_PAUSE ))

    /* Flow control for Encoder */
    #define isEncEOF()                 ((MMIO_Read(DrvAudioCtrl) & DrvEncode_EOF        ) != 0)
    #define setEncEOF()                MMIO_Write(DrvAudioCtrl, (MMIO_Read(DrvAudioCtrl) | DrvEncode_EOF   ))
    #define clrEncEOF()                MMIO_Write(DrvAudioCtrl, (MMIO_Read(DrvAudioCtrl) & ~DrvEncode_EOF   ))

    /* Audio Engine Reset control for multiple cpu */
    #define setAudioReset()            MMIO_Write(DrvAudioCtrl2, (MMIO_Read(DrvAudioCtrl2) | DrvAudio_RESET))

    /* mp3 Reset read buffer pointer */
    #define resetMp3RdBufPointer()  MMIO_Write(DrvAudioCtrl2, (MMIO_Read(DrvAudioCtrl2) & ~DrvResetMp3RdBufPointer))


    /* SPDIF Mute */
    #define isSPDIFMute()              ((MMIO_Read(DrvSPDIFMute)) == 0)

    /* DOLBY Parameter */
    #define getAC3DRCMode()            ((MMIO_Read(AC3_DECODER_PARAMETER) & AC3_DRC_MODE) >> AC3_DRC_MODE_BITS)
    #define getAC3DualMonoMode()       ((MMIO_Read(AC3_DECODER_PARAMETER) & AC3_DUAL_MONO_MODE) >> AC3_DUAL_MONO_MODE_BITS)
    #define getAC32ChDownmixMode()     ((MMIO_Read(AC3_DECODER_PARAMETER) & AC3_2_CHANNELS_DOWNMIX_MODE) >> AC3_2_CHANNELS_DOWNMIX_MODE_BITS)
    /* Flash audio input Parameter */
    // format 1:mp3 ,2 wav
    #define getFlashInput0Format()     ((MMIO_Read(FLASH_AUDIO_INPUT_PARAMETER) & FLASH_INPUT_0_FORMAT) >> FLASH_AUDIO_INPUT_0_BIT) + 1
    #define getFlashInput1Format()     ((MMIO_Read(FLASH_AUDIO_INPUT_PARAMETER) & FLASH_INPUT_1_FORMAT) >> FLASH_AUDIO_INPUT_1_BIT) + 1
    #define getFlashInput2Format()     ((MMIO_Read(FLASH_AUDIO_INPUT_PARAMETER) & FLASH_INPUT_2_FORMAT) >> FLASH_AUDIO_INPUT_2_BIT) + 1
    #define getFlashInput3Format()     ((MMIO_Read(FLASH_AUDIO_INPUT_PARAMETER) & FLASH_INPUT_3_FORMAT) >> FLASH_AUDIO_INPUT_3_BIT) + 1
    #define getFlashInput0Using()      ((MMIO_Read(FLASH_AUDIO_INPUT_PARAMETER) & FLASH_INPUT_0_USING) >> FLASH_AUDIO_INPUT_0_USING_BIT)
    #define getFlashInput1Using()      ((MMIO_Read(FLASH_AUDIO_INPUT_PARAMETER) & FLASH_INPUT_1_USING) >> FLASH_AUDIO_INPUT_1_USING_BIT)
    #define getFlashInput2Using()      ((MMIO_Read(FLASH_AUDIO_INPUT_PARAMETER) & FLASH_INPUT_2_USING) >> FLASH_AUDIO_INPUT_2_USING_BIT)
    #define getFlashInput3Using()      ((MMIO_Read(FLASH_AUDIO_INPUT_PARAMETER) & FLASH_INPUT_3_USING) >> FLASH_AUDIO_INPUT_3_USING_BIT)

    //#include "sys.h"
    #if !defined(__OPENRTOS__)

static __inline void MMIO_Write(unsigned short addr, unsigned short data)
{
    *(volatile unsigned short *) (MMIO_ADDR + (unsigned int)addr) = data;
}

static __inline unsigned short MMIO_Read(unsigned short addr)
{
    return *(volatile unsigned short *) (MMIO_ADDR + (unsigned int)addr);
}

static __inline void MMIO_WriteMask(unsigned short addr, unsigned short data, unsigned short mask)
{
    MMIO_Write(addr, ((MMIO_Read(addr) & ~mask) | (data & mask)));
}

    #else // defined(__FREERTOS__)

void MMIO_Write(unsigned short addr, unsigned short data);
        #define MMIO_Write_From_ISR MMIO_Write

unsigned int MMIO_Read(unsigned short addr);
unsigned int MMIO_Read_From_ISR(unsigned short addr);

void MMIO_WriteMask(unsigned short addr, unsigned short data, unsigned short mask);
        #define MMIO_WriteMask_From_ISR MMIO_WriteMask

    #endif // defined(__FREERTOS__)

    #if defined(MM680)
        #define AHB_MMIO_Write(addr, n)           *(volatile int *)(addr) = (n)
        #define AHB_MMIO_Read(addr)               *(volatile int *)(addr)
        #define AHB_MMIO_WriteMask(addr, n, mask) *(volatile int *)(addr) = ((n & mask) | (AHB_MMIO_Read(addr) & ~mask))
    #endif
#endif   // defined(__OR32__)

#ifdef __cplusplus
}
#endif
#endif /* __MMIO_H__ */
