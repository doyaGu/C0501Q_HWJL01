/*
 * Copyright (c) 2004 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Co-processor API functoin file.
 *      Date: 2005/09/30
 *
 */
#include <pthread.h>
#include <assert.h>

#include "ite/audio.h"
#include "ite/ith.h"
#include "ite/itp.h"
#include "ite/ite_risc.h"
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#define MESSAGE_SIZE (32 * 1024)


#if (CFG_CHIP_FAMILY == 9070)
    #define TOINT(n)   ((((n) >> 24) & 0xff) + (((n) >> 8) & 0xff00) + (((n) << 8) & 0xff0000) + (((n) << 24) & 0xff000000))
    #define TOSHORT(n) ((((n) >> 8) & 0x00ff) + (((n) << 8) & 0xff00))

    #define CODEC_BASE    ((int)iteRiscGetTargetMemAddress(RISC1_IMAGE_MEM_TARGET) - 0x1000)
#elif (CFG_CHIP_FAMILY == 9850)
    #define TOINT(n)   ((((n) >> 24) & 0xff) + (((n) >> 8) & 0xff00) + (((n) << 8) & 0xff0000) + (((n) << 24) & 0xff000000))
    #define TOSHORT(n) ((((n) >> 8) & 0x00ff) + (((n) << 8) & 0xff00))

    #define CODEC_BASE    ((int)iteRiscGetTargetMemAddress(RISC1_IMAGE_MEM_TARGET))
#else     //if (CFG_CHIP_FAMILY == 9910)
    #define TOINT(n)   n
    #define TOSHORT(n) n
    #define CODEC_BASE 0
#endif

#include "mmio.h"

#define DrvDecode_Skip_Bits 2
#define DrvDecode_Skip      (1 << DrvDecode_Skip_Bits)  // D[2]
#define DrvAudioCtrl2       0x169c

//#include "i2s.h"

// to output file
//#define OUTPUT_TO_FILE
//#define DONOT_PLAY_AUDIO
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define AUDIO_WRAP_AROUND_THRESHOLD   (0x3E80000)     // 65536 seconds
#define AUDIO_WRAP_AROUND_JUDGE_RANGE (0x1F40000)     // 36768 seconds

//#define AUDIO_DRIVER_SHOW_WRITE_BITRATE

#define HAVE_MP3                      1
#define HAVE_AAC                      1
#define HAVE_AC3                      1 // AC3 decode cannot work
#define HAVE_WMA                      1
#define HAVE_AMR                      1
#define HAVE_WAV                      1
#define HAVE_FLAC                     1
#define HAVE_SBC                      1
#define HAVE_BSAC                     0
#define HAVE_MIDI                     0
#define HAVE_SPDIF_AC3                0
#define HAVE_MIXER                    0
#define HAVE_PCM                      0

#if defined(AUDIO_DRIVER_SHOW_WRITE_BITRATE)
PAL_CLOCK_T  gtAudioDriverWirteBitrate;
unsigned int gAudioWrite         = 0;
unsigned int gAudioWriteDuration = 0;
#endif

//#define CODEC_STREAM_START_ADDRESS 0x00001000+0x14

#if defined(OUTPUT_TO_FILE)
    #include <stdio.h>
static FILE *fout = NULL;
#endif

typedef void *TaskHandle_t;

//#include "aud/eqtable.h"
//#include "aud/revbtable.h"

ITE_AUDIO_EQINFO   eqinfo = {       // default EQ table
    1, 1,
    {  0,  82, 251, 404, 557, 732, 907, 1098, 1328, 1600, 3300, 6000, 13000, 16000, 25000,    -1},
    {  1,   2,   2,   3,   3,   4,   1,    0,   -4,   -7,   -7,   -7,    -7,    -7,    -7,     0},
};

#define DATA_CONST(A) ((int)(((A) * (1 << 28) + 0.5)))
ITE_AUDIO_REVBINFO revbinfo = {     // default Reverb table
    1, 1,
    DATA_CONST(0.5),
    DATA_CONST(0.5),
    DATA_CONST(0.0),
    { { 15, DATA_CONST(0.35), 40, DATA_CONST(0.25), 00, DATA_CONST(0.00), 30, DATA_CONST(0.45) },
      { 15, DATA_CONST(0.35), 40, DATA_CONST(0.25), 00, DATA_CONST(0.00), 30, DATA_CONST(0.45) }}
};
ITE_AUDIO_EQINFO   eqinfo_def[] =
{
    // 0. Classical
    {      1,     1,
           {    0,    82,   251,   404,   557,   732,   907,  1098,  1328,  1600,  3300,  6000, 13000, 16000, 25000,    -1},
           {    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,    -1,    -6,    -6,    -7,    -7,     0}, },

    // 1. Club
    {      1,     1,
           {    0,    82,   251,   404,   557,   732,   907,  1098,  1328,  1600,  3300,  6000, 13000, 16000, 25000,    -1},
           {    0,     0,     1,     2,     3,     3,     3,     3,     3,     3,     3,     1,     0,     0,     0,     0}, },

    // 2. Live
    {      1,     1,
           {    0,    82,   251,   404,   557,   732,   907,  1098,  1328,  1600,  3300,  6000, 13000, 16000, 25000,    -1},
           {   -4,    -3,     1,     2,     2,     3,     3,     3,     3,     3,     3,     2,     1,     1,     1,     0}, },

    // 3. Pop
    {      1,     1,
           {    0,    82,   251,   404,   557,   732,   907,  1098,  1328,  1600,  3300,  6000, 13000, 16000, 25000,    -1},
           {   -2,    -1,     3,     4,     4,     4,     3,     2,     2,     1,    -2,    -2,    -2,    -2,    -2,     0}, },

    // 4. Rock
    {      1,     1,
           {    0,    82,   251,   404,   557,   732,   907,  1098,  1328,  1600,  3300,  6000, 13000, 16000, 25000,    -1},
           {    4,     4,    -2,    -5,    -6,    -5,    -4,    -3,    -2,    -2,     2,     5,     6,     6,     6,     0}, },
};

#if (CFG_CHIP_FAMILY == 9910)
extern void dc_invalidate(void);
struct _codec_api codec_api = {
    dc_invalidate,
};
#endif

//#include "pal/file.h"
//#include "aud/codecs.h"
#include "openrtos/portmacro.h"
#include "openrtos/projdefs.h"

bool audioKeySoundPaused = false;

//static char codec_path[128] = { 0 };
//static char codec_file[128] = { 0 };
extern struct _codec_api codec_api;
static pthread_mutex_t   RiscMutex = PTHREAD_MUTEX_INITIALIZER;
#if (CFG_CHIP_FAMILY == 9910)

#endif
extern char              amr_codec[CODEC_ARRAY_SIZE];
extern char              aac_codec[CODEC_ARRAY_SIZE];
extern char              ac3_codec[CODEC_ARRAY_SIZE];
extern char              wav_codec[CODEC_ARRAY_SIZE];
extern char              wma_codec[CODEC_ARRAY_SIZE];

//struct _codec_header *__header = (struct _codec_header *)codecbuf;
struct _codec_header     *__header = 0;
static int               codec_start_addr;
static int               codec_end_addr;
#if defined(ENABLE_CODECS_ARRAY)
extern int               *codecptr[];
#endif

static uint32_t          gPtsTimeBaseOffset = 0;
static uint32_t          gLastDecTime       = 0;
//#define RevbBufSize  276480

//#include "aud/audio_error.h"

//=============================================================================
//                              Macro Definition
//=============================================================================
#define BufThreshold      64
#define BSPPOSITION       (AUD_baseAddress + 0)
#define PROPOSITION       (AUD_baseAddress + 0x10000)
#define EQTABLEPOSITION   (AUD_baseAddress + EQTABLE)
#define REVBTABLEPOSITION (AUD_baseAddress + REVBTABLE)
#define RISC_FIRE         (0x168c)
#define RISC_PC           (0x16b4)

#define MMIO_PTS_WRIDX    (0x16ae)
#define MMIO_PTS_HI       (0x16b2)
#define MMIO_PTS_LO       (0x16b0)

//#define AUDIO_PLUGIN_MESSAGE_QUEUE // defined in def.cmake
//#ifdef DEBUG_MODE
//#define LOG_ZONES    (ITH_BIT_ALL & ~ITH_ZONE_ENTER & ~ITH_ZONE_LEAVE & ~ITH_ZONE_DEBUG)
////#define LOG_ZONES    (ITH_BIT_ALL & ~ITH_ZONE_ENTER & ~ITH_ZONE_LEAVE)
////#define LOG_ZONES    (ITH_BIT_ALL)
//
//#define LOG_ERROR   ((void) ((ITH_ZONE_ERROR & LOG_ZONES) ? (printf("[SMEDIA][AUD][ERROR]"
//#define LOG_WARNING ((void) ((ITH_ZONE_WARNING & LOG_ZONES) ? (printf("[SMEDIA][AUD][WARNING]"
//#define LOG_INFO    ((void) ((ITH_ZONE_INFO & LOG_ZONES) ? (printf("[SMEDIA][AUD][INFO]"
//#define LOG_DEBUG   ((void) ((ITH_ZONE_DEBUG & LOG_ZONES) ? (printf("[SMEDIA][AUD][DEBUG]"
//#define LOG_ENTER   ((void) ((ITH_ZONE_ENTER & LOG_ZONES) ? (printf("[SMEDIA][AUD][ENTER]"
//#define LOG_LEAVE   ((void) ((ITH_ZONE_LEAVE & LOG_ZONES) ? (printf("[SMEDIA][AUD][LEAVE]"
//#define LOG_DATA    ((void) ((ITH_FALSE) ? (printf(
//#define LOG_END     )), 1 : 0));
//#else
#define LOG_ZONES
#define LOG_ERROR
#define LOG_WARNING
#define LOG_INFO
#define LOG_DEBUG
#define LOG_ENTER
#define LOG_LEAVE
#define LOG_DATA
#define LOG_END ;

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef void (*CODEC_INITIALIZE)(void);
typedef void (*CODEC_TERMINATE)(void);
typedef void (*CODEC_SETVOLUME)(uint32_t level);

typedef struct AUDIO_CODEC_TAG
{
    bool             codecInit;
    uint32_t         curVolume;
    uint32_t         adcRatio;
    uint32_t         dacRatio;
    uint32_t         sampleRate;
    uint32_t         codecMode;
    CODEC_INITIALIZE CODEC_Initialize;
    CODEC_TERMINATE  CODEC_Terminate;
    CODEC_SETVOLUME  CODEC_SetVolume;
} AUDIO_CODEC;

//=============================================================================
//                              Global Data Definition
//=============================================================================
static uint32_t         Audio_DefEncode_Bufptr;     // default encode buffer address
static uint32_t         Audio_Encode_Bufptr;        // encode buffer address
static uint32_t         Audio_Encode_Buflen;        // encode buffer length
static uint32_t         Audio_Encode_Rdptr;         // encode buffer read  port address
static uint32_t         Audio_Encode_Wrptr;         // encode buffer write port address

static uint32_t         Audio_DefDecode_Bufptr;     // default decode buffer address
static uint32_t         Audio_Decode_Bufptr;        // decode buffer address
static uint32_t         Audio_Decode_Buflen;        // decode buffer length
static uint32_t         Audio_Decode_Rdptr;         // decode buffer read  port address
static uint32_t         Audio_Decode_Wrptr;         // decode buffer write port address

static uint32_t         Audio_DefDecode_Bufptr1;    // default decode buffer address
static uint32_t         Audio_Decode_Bufptr1;       // decode buffer address
static uint32_t         Audio_Decode_Buflen1;       // decode buffer length
static uint32_t         Audio_Decode_Rdptr1;        // decode buffer read  port address
static uint32_t         Audio_Decode_Wrptr1;        // decode buffer write port address

static uint32_t         Audio_DefDecode_Bufptr2;    // default decode buffer address
static uint32_t         Audio_Decode_Bufptr2;       // decode buffer address
static uint32_t         Audio_Decode_Buflen2;       // decode buffer length
static uint32_t         Audio_Decode_Rdptr2;        // decode buffer read  port address
static uint32_t         Audio_Decode_Wrptr2;        // decode buffer write port address

static uint32_t         Audio_DefEcho_Bufptr;       // default echo buffer address
static uint32_t         Audio_Echo_Bufptr;          // echo buffer address
static uint32_t         Audio_Echo_Buflen;          // echo buffer length

static uint32_t         Audio_DefRef_Bufptr;        // default reference buffer address
static uint32_t         Audio_Ref_Bufptr;           // reference buffer address
static uint32_t         Audio_Ref_Buflen;           // reference buffer length

static uint32_t         Audio_DefEchocc_Bufptr;     // default echo cancellation buffer address
static uint32_t         Audio_Echocc_Bufptr;        // echo cancellation buffer address
static uint32_t         Audio_Echocc_Buflen;        // echo cancellation buffer length

static uint32_t         Audio_DefParameter_Cmdptr;  // default command buffer address
static uint32_t         Audio_Parameter_Cmdptr;     // command buffer address
static uint32_t         Audio_Parameter_Cmdlen;     // command buffer length

static bool             AUD_SKIP        = 0;
//static PAL_CLOCK_T          lastTime            = 0;
static uint32_t         AUD_baseAddress = 0;
static uint32_t         AUD_freqAddress = 0;
static uint32_t         AUD_curPlayPos  = 0;
//static uint16_t             AUD_HostPll         = 0;
static bool             AUD_Init        = 0;
static ITE_AUDIO_ENGINE AUD_EngineType  = 0;                    // engine type now
static ITE_AUDIO_ENGINE AUD_PlugInType  = 0;                    // engine type now
static uint32_t         AUD_EngineState = ITE_AUDIO_ENGINE_UNKNOW;
static AUDIO_CODEC      AUD_Codec       = {0, 0, 0, 0, 0, 0, NULL, NULL, NULL};
static bool             AUD_eqStatus              = 0;
static bool             AUD_eqPrevStatus          = 0;
static uint32_t         AUD_prevEqType            = 0;
static uint32_t         AUD_eqType                = 0;
static uint32_t         AUD_decSampleRate         = 0;
static uint32_t         AUD_decChannel            = 0;
static uint32_t         AUD_nCodecSampleRate      = 0; // audio codec set sample rate
static uint32_t         AUD_nCodecChannels        = 0; // audio codec set channels
static uint32_t         AUD_nCodecPCMBufferLength = 0; // audio codec set channels
static bool             AUD_bPlayMusic            = 0;
static bool             AUD_bPtsSync              = 0;
static bool             AUD_bI2SInit              = 0;
static char             *AUD_pI2Sptr;
static bool             AUD_bMPlayer              = 0;
static bool             AUD_bMPlayerSTCReady      = 0;
static bool             AUD_bDecodeError          = 0;
static bool             AUD_bIsPause              = 0;
static bool             AUD_bEnableSTC_Sync       = 0;
static bool             AUD_bSPDIFNonPCM          = 0;
static uint32_t         AUD_nFfmpegPauseAudio    = 0;
//static int32_t              AUD_nMPlayerPTS = 0;
static uint32_t         AUD_nDropData             = 0;
static uint32_t         AUD_nPauseSTCTime         = 0;
static uint8_t          *AUD_pEngineAddress;
static uint32_t         AUD_nEngineLength;
//static uint32_t             AUD_nFadeIn = 0;
static bool             AUD_bI2SOutFull       = 0;
static ITE_AUDIO_ENGINE AUD_SuspendEngineType = ITE_RESERVED;     // engine type now

//static uint16_t RISC_clock_reg = 0;
static uint8_t          wave_header[48];
static uint8_t          wma_info[32];
static uint32_t         write_waveHeader = 0;
static uint32_t         write_wmaInfo    = 0;
static uint32_t         wave_ADPCM       = 0;
static uint32_t         Bootmode         = 1;

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
static short        *Audio_Plugin_Message_Buf   = 0; //(short *)&gBuf[RISC1_SIZE + CODEC_EX_SIZE];
static unsigned int Audio_Plugin_Message_Buflen = 0; //MESSAGE_SIZE / sizeof(short);
AUDIO_CODEC_STREAM  *gAudioStream               = 0; //(AUDIO_CODEC_STREAM *)&gBuf[RISC1_SIZE + CODEC_EX_SIZE + MESSAGE_SIZE];
#endif

static uint16_t enReverb, enEQ;
//static bool   stopMute = 0;
//static bool   writeStream = 0;
//static bool   firstTime = 1;
//static bool   protectChangeMusic = 0;
//static bool   fireStopSwitch = 0;
//static uint16_t stopMusicReg = 0x0;
//static bool   firstTimePlay = 1;
//#define decVolLevel 0x0101

static bool g_EarPhoneDetect = 0;
//static uint32_t  nInputCount=0;

//#ifndef CFG_VIDEO_ENABLE
////#define DRIVER_CODEC
//#define CFG_COMPRESS_AUDIO_PLUGIN
//#else
#define DRIVER_CODEC
//#define CFG_COMPRESS_AUDIO_PLUGIN
//#endif

#if defined(DRIVER_CODEC) && !defined(CFG_COMPRESS_AUDIO_PLUGIN)

    #ifdef CFG_AUDIO_CODEC_MP3DEC
static unsigned char gMp3Decoder[] = {
        #include "mp3.hex"
};
    #endif

    #ifdef CFG_AUDIO_CODEC_WAV
static unsigned char gWaveDecoder[] = {
        #include "wave.hex"
};
    #endif

    #ifdef CFG_AUDIO_CODEC_WMADEC
static unsigned char gWmaDecoder[] = {
        #include "wma.hex"
};
    #endif

    #ifdef CFG_AUDIO_CODEC_AACDEC
static unsigned char gAacDecoder[] = {
        #include "aac.hex"
};
    #endif

    #ifdef CFG_AUDIO_CODEC_AMR
static unsigned char gAmrDecoder[] = {
        #include "amr.hex"
};
    #endif

    #ifdef CFG_AUDIO_CODEC_EAC3DEC
static unsigned char gAc3Decoder[] = {
        #include "eac3.hex"
};
    #endif

    #ifdef CFG_AUDIO_CODEC_FLACDEC
static unsigned char gFlacDecoder[] = {
        #include "flac.hex"
};
    #endif

static unsigned char gSbcCodec[] = {
    #include "sbc.hex"
};
#endif

//=============================================================================
//                              Extern Reference
//=============================================================================
int32_t
AUD_FireEngine(
    void);

int32_t
AUD_ResetEngine(
    void);

int32_t
AUD_ConfigEngine(
    void);

#ifdef ENABLE_AUDIO_PROCESSOR
static void
AUD_ResetAudioProcessor(
    void);

static void
AUD_FireAudioProcessor(
    void);
#endif

int32_t
AUD_ResetAudioEngineType(
    void);

//=============================================================================
//                              Function Definition
//=============================================================================
#ifdef ENABLE_AUDIO_PROCESSOR
static void
AUD_ResetAudioProcessor(
    void)
{
    uint32_t i;
    uint16_t reg   = 0;
    #ifdef AUDIO_FREERTOS
    uint32_t clock = PalGetClock();
    #endif
    LOG_ENTER "AUD_ResetAudioProcessor()\n" LOG_END

    // 0x44 bit14, Reset the RISC1, 0: Normal Operation, 1: Reset RISC1
    //printf("AUD_ResetAudioProcessor+\n");
    // force halt the second cpu.
    ithWriteRegH(DrvDecode_WrPtr, 0xFFFF);
    while (1)
    {
        reg = ithReadRegH(DrvAudioCtrl2);

    #ifdef AUDIO_FREERTOS
        if ((reg & DrvAudio_RESET)
            || PalGetDuration(clock) > 66)
        {
            if (PalGetDuration(clock) > 66)
            {
                //sdk_msg(SDK_MSG_TYPE_ERROR, "%s(%d), duration: %u\n", __FILE__, __LINE__, PalGetDuration(clock));
            }
            break;
        }
    #else
        for (i = 0; i < 64000; i++)
            asm ("");

        break;
    #endif
        for (i = 0; i < 64; i++)
            asm ("");
    }
    for (i = 0; i < 64; i++)
        asm ("");

    ithWriteRegH(DrvDecode_WrPtr, 0x0);
    ithWriteRegMaskH(DrvAudioCtrl2, (0 << DrvAudio_Reset_Bits), (1 << DrvAudio_Reset_Bits));

    iteRiscResetCpu(RISC1_CPU);
    //printf("AUD_ResetAudioProcessor-\n");
}

static void
AUD_FireAudioProcessor(
    void)
{
    iteRiscFireCpu(RISC1_CPU);
    ithWriteRegH(AUDIO_DECODER_START_FALG, 0);

    //iteRiscOpenEngine(ITE_SW_PERIPHERAL_ENGINE);
}
#endif         // ENABLE_AUDIO_PROCESSOR

int32_t
AUD_GetEngineStatus(
    uint32_t *status)
{
    uint32_t i = 0;
    LOG_ENTER "AUD_GetEngineStatus()\n" LOG_END

#if 0
    /*
     * Maximum delay time of AUD_GetEngineStatus:
     * check program counter state: 5 x 2 = 10 milisecond
     * check read/write pointer for each frame: 4 x 40 = 160 milisecond
     * total delay time = 10 + 160 = 170 milisecond
     */
    for (i = 5; i > 0; i--)
    {
        uint16_t pcLowDataRd1, pcHighDataRd1;
        uint16_t pcLowDataRd2, pcHighDataRd2;
        ithReadRegH(RISC_PC,   &pcLowDataRd1);
        ithReadRegH(RISC_PC + 2, &pcHighDataRd1);
        ithDelay(2);
        ithReadRegH(RISC_PC,   &pcLowDataRd2);
        ithReadRegH(RISC_PC + 2, &pcHighDataRd2);

        if (pcHighDataRd2 == 0x0)
        {
            switch (pcLowDataRd2 & 0xff00)
            {
            case 0x0600:
                LOG_ERROR "Alignment error\n" LOG_END
                * status = ITE_AUDIO_ENGINE_ERROR;
                return AUDIO_ERROR_ENGINE_ALIGNMENT_ERROR;

            case 0x0700:
                LOG_ERROR "Illegal instruction error\n" LOG_END
                * status = ITE_AUDIO_ENGINE_ERROR;
                return AUDIO_ERROR_ENGINE_ILLEGAL_INSTRUCTION_ERROR;
            }
        }

        // Judge if high part of the RISC program counter is equal.
        if (pcHighDataRd1 != pcHighDataRd2)
            goto CHECK_IS_IDLE;

        // RISC is not fire, return immediately
        if (pcLowDataRd1 == 0x0 && pcLowDataRd2 == 0x0)
        {
            *status = ITE_AUDIO_ENGINE_STOPPING;
            return 0;
        }

        // RISC is running
        if (pcLowDataRd1 != pcLowDataRd2)
            goto CHECK_IS_IDLE;
    }

    if (i == 0)
    {
        *status = ITE_AUDIO_ENGINE_NO_RESPONSE;
        return AUDIO_ERROR_ENGINE_UNKNOW_STATUS_ERROR;
    }

CHECK_IS_IDLE:
    for (i = 4; i > 0; i--)
    {
        uint16_t decRdPtr1, decRdPtr2;
        uint16_t encWrPtr1, encWrPtr2;
        ithReadRegH(DrvDecode_RdPtr, &decRdPtr1);
        ithReadRegH(DrvDecode_WrPtr, &encWrPtr1);
        ithDelay(40);
        ithReadRegH(DrvDecode_RdPtr, &decRdPtr2);
        ithReadRegH(DrvDecode_WrPtr, &encWrPtr2);

        if ((decRdPtr1 != decRdPtr2) || (encWrPtr1 != encWrPtr2))
        {
            *status = ITE_AUDIO_ENGINE_RUNNING;
            return 0;
        }
    }

    *status = ITE_AUDIO_ENGINE_IDLE;
    return 0;
#else
    *status = ITE_AUDIO_ENGINE_RUNNING;
    return 0;
#endif
}

// surround
int32_t
AUD_SetEnReverb(
    bool enable,
    uint32_t reverbAddress)
{
    LOG_ENTER "AUD_SetEnReverb(%d)\n", enable LOG_END

    if (enable == 1)
    {
        enReverb = 1;
        ithWriteRegH(DrvDecode_RevbBase,   (reverbAddress) & 0xffff);
        ithWriteRegH(DrvDecode_RevbBase + 2, (reverbAddress) >> 16);
        //ithWriteRegMaskH(DrvAudioCtrl, DrvDecode_EnReverb, 1<<4);
    }
    else
    {
        enReverb = 0;
        ithWriteRegMaskH(DrvAudioCtrl, 0, 1 << 4);
    }
    return 0;
}

// no use now
int32_t
AUD_SetEnDRC(
    int32_t enable)
{
    LOG_ENTER "AUD_SetEnDRC(%d)\n", enable LOG_END

    if (enable == 1)
    {
        ithWriteRegMaskH(DrvAudioCtrl, 0xffff, DrvDecode_EnDRC);
    }
    else if (enEQ == 0 && enReverb == 0)
    {
        ithWriteRegMaskH(DrvAudioCtrl, 0x0, DrvDecode_EnDRC);
    }
    return 0;
}

int32_t
AUD_SetEnVoiceRemove(
    bool enable)
{
    LOG_ENTER "AUD_SetEnVoiceRemove(%d)\n", enable LOG_END

    if (enable == 1)
    {
        ithWriteRegMaskH(DrvAudioCtrl, DrvDecode_EnVoiceOff, 1 << 5 );
    }
    else
    {
        ithWriteRegMaskH(DrvAudioCtrl, 0, 1 << 5 );
    }

    return 0;
}

int32_t
AUD_SetEnEQ(
    bool enableEq)
{
    LOG_ENTER "AUD_SetEnEQ()\n" LOG_END

    if (enableEq == 1)
    {
        enEQ = AUD_eqStatus = 1;

        ithWriteRegMaskH(DrvAudioCtrl, 0xffff, DrvDecode_EnEQ);
    }
    else
    {
        enEQ = AUD_eqStatus = 0;
        ithWriteRegMaskH(DrvAudioCtrl, 0x0, DrvDecode_EnEQ);
    }

    return 0;
}

//int32_t
//AUD_GetFreqInfo(
//    uint8_t* buffer)
//{
// LOG_ENTER "AUD_GetFreqInfo()\n" LOG_END
//HOST_ReadBlockMemory((uint32_t)buffer, AUD_freqAddress, 20);
//    return 0;
//}

// stream
int32_t
AUD_GetWriteAvailableBufferLength(
    uint32_t *wrBuflen)
{
    uint16_t hwpr;
    uint16_t swpr;

    //PRECONDITION(Audio_Decode_Buflen >=  4);
    //PRECONDITION(Audio_Decode_Buflen % 2 == 0); // word align
    //LOG_ENTER "AUD_GetWriteAvailableBufferLength()\n" LOG_END
    swpr = Audio_Decode_Wrptr;

    hwpr = ithReadRegH(DrvDecode_RdPtr);

    //if (AUD_EngineType != ITE_SBC_CODEC)
    //{
    //    hwpr = ((hwpr >> 1) << 1); // word align
    //}

    // Get the avalible length of buffer
    if (swpr == hwpr)
    {
        (*wrBuflen) = Audio_Decode_Buflen - 2;
    }
    else if (swpr > hwpr)
    {
        (*wrBuflen) = Audio_Decode_Buflen - (swpr - hwpr) - 2;
    }
    else
    {
        (*wrBuflen) = hwpr - swpr - 2;
    }

    return 0;
}

int32_t
AUD_GetReadAvailableBufferLength(
    uint32_t *rdBuflen)
{
    uint16_t hwpr;
    uint16_t swpr;

    //PRECONDITION(Audio_Encode_Buflen >=  4);
    //PRECONDITION(Audio_Encode_Buflen % 2 == 0); // word align

    //LOG_ENTER "AUD_GetReadAvailableBufferLength()\n" LOG_END

    swpr = Audio_Encode_Rdptr;

    hwpr = ithReadRegH(DrvEncode_WrPtr);

    //if (AUD_EngineType != ITE_SBC_CODEC)
    //{
    //    hwpr = ((hwpr >> 1) << 1); //word align
    //}

    if (swpr == hwpr)
    {
        (*rdBuflen) = 0;
    }
    else if (swpr > hwpr)
    {
        (*rdBuflen) = Audio_Encode_Buflen - (swpr - hwpr);
    }
    else
    {
        (*rdBuflen) = hwpr - swpr;
    }
    return 0;
}

int32_t
AUD_SetOutEOF(
    void)
{
    uint16_t regData = 0;
    uint16_t timeout = 0;
    LOG_ENTER "AUD_SetOutEOF()\n" LOG_END

    ithWriteRegMaskH(DrvAudioCtrl, 0xffff, DrvDecode_EOF);

    do
    {
        regData = ithReadRegH(DrvAudioCtrl);
        if (!(regData & DrvDecode_EOF))
        {
            return 0;
        }
        if ((regData & DrvDecode_STOP))
        {
            return 0;
        }

        usleep(1000);
    } while (1);

    return 0;
}

// midi
//int32_t
//AUD_SetMIDIAddress(
//    uint32_t address)
//{
//    LOG_ENTER "AUD_SetMIDIAddress()\n" LOG_END
//#if 0
//    address = address - (uint32_t)HOST_GetVramBaseAddress();
//    ithWriteRegH(DrvMIDI_BASE, (uint16_t)(address & 0xFFFF));
//    ithWriteRegH((DrvMIDI_BASE + 2), (uint16_t)(address >> 16));
//#endif

//    return 0;
//}

// engine
//int32_t
//AUD_PowerOnRisc(
//    void)
//{
//    return 0;
//}

//int32_t
//AUD_PowerOffRisc(
//    void)
//{
//    LOG_ENTER "AUD_PowerOffRisc()\n" LOG_END
//    return 0;
//}

//int32_t
//AUD_SetRISCIIS(
//    void)
//{
//uint16_t regData = 0;

//    LOG_ENTER "AUD_SetRISCIIS()\n" LOG_END

//    return 0;
//}

int32_t
AUD_InitialCodec(
    void)
{
    static set_I2C_data = 0;

    LOG_ENTER "AUD_InitialCodec()\n" LOG_END

    // Use script to initilize CODEC (script writen by FAE)
    if (AUD_Codec.codecInit != 1)
    {
        if (AUD_Codec.CODEC_Initialize != NULL)
        {
            AUD_Codec.CODEC_Initialize();
            AUD_Codec.codecInit = 1;
        }
        else
        {
            LOG_ERROR "Codec cannot be initialized\n" LOG_END
        }
    }

    //2007-11-19 jeimei added
    if (set_I2C_data == 0)
    {
        ithDelay(10);
        //disable ACLK clock
        //ithWriteRegMaskH(0x32,0x0,0x0001);
        //ithDelay(0x10);

        /*I2S MMIO*/
        //ithWriteRegMaskH(0x210,0x1,0x0001);
        //ithDelay(0x10);
        /*I2S interrupt*/
        //ithWriteRegMaskH(0x6,0x1,0x0001);
        //ithDelay(0x10);
        set_I2C_data = 1;
    }

    return 0; //AUD_SetRISCIIS();
}

int32_t
AUD_TerminateCodec(
    void)
{
    LOG_ENTER "AUD_TerminateCodec()\n" LOG_END

    if (AUD_SKIP)
        return 0;

#if defined(OUTPUT_TO_FILE)
    if (fout)
    {
        printf("[Audio Driver] PalFileClose\n");
        fclose(fout);
        fout = NULL;
    }
#endif

    return 0;
}

int AUD_CheckProcessor()
{
    unsigned short nTemp, nCount, nTemp1, nTemp2;

    nTemp  = 0;
    nCount = 0;
    do
    {
        nTemp = ithReadRegH(AUDIO_DECODER_START_FALG);
        ithDelay(1);
        nCount++;
    } while (nTemp == 0 && nCount <= 20000);
    if (nCount > 20000)
    {
        printf("[Audio Driver] wait decoder start fail #line %d \n", __LINE__);
    }
    
    nTemp1 = ithReadRegH(0x16B4);
    nTemp2 = ithReadRegH(0x16B6);
    //printf("AUD_CheckProcessor %d %d 0x%x 0x%x \n", nTemp, nCount, nTemp1, nTemp2);
}

int32_t
AUD_LoadEngine(
    ITE_AUDIO_ENGINE audio_type)
{
    //uint8_t* pTemp;
    //uint16_t nTemp,nCount;
    //bool bMPlayer;
    //int* streamAddress;

    LOG_ENTER "AUD_LoadEngine()\n" LOG_END

#if 0
    if (AUD_GetEngineStatus(&AUD_EngineState) == 0)
    {
        if (AUD_EngineState == ITE_AUDIO_ENGINE_RUNNING)
        {
            if (AUD_EngineType == audio_type)
                return 0;
            else
                return AUDIO_ERROR_ENGINE_IS_RUNNING;
        }
    }
#endif
    if (AUD_EngineType == audio_type)
    {
        //goto RESET_ENGINE;
    }
    AUD_SKIP            = 0;
    Audio_Encode_Wrptr  = 0;
    Audio_Encode_Rdptr  = 0;
    Audio_Decode_Wrptr  = 0;
    Audio_Decode_Rdptr  = 0;
    Audio_Decode_Wrptr1 = 0;
    Audio_Decode_Rdptr1 = 0;
    Audio_Decode_Wrptr2 = 0;
    Audio_Decode_Rdptr2 = 0;
    //Audio_Decode_Wrptr3 = 0;
    //Audio_Decode_Rdptr3 = 0;

    AUD_freqAddress = 0;    
    if (AUD_ResetEngine() != 0)
        return AUDIO_ERROR_ENGINE_RESET_ENGINE_ERROR;

#if defined(ENABLE_CODECS_PLUGIN) && !defined(ENABLE_CODECS_ARRAY)
    // Load Plug-Ins to memory
    if (1) //(AUD_PlugInType != audio_type)
    {
        signed portBASE_TYPE ret = pdFAIL;
        //static FILE *pFile = NULL;

/*
    #ifdef ENABLE_AUDIO_PROCESSOR
        AUD_ResetAudioProcessor();  // Stop CPU
    #endif
*/
        //if (!pAddress || length <=0){
        //    LOG_ERROR
        //        " engine load address error %d %d #line %d \n",pAddress,length,__LINE__
        //    LOG_END
        //    AUD_SKIP = 1;
        //    return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;
        //}
        //printf("[Audio Driver] audio_type %d get #line %d \n",audio_type,__LINE__);
        //PalWcscpy(codec_file, codec_path);
        switch (audio_type)
        {
        case ITE_MP3_DECODE:
            AUD_SKIP = !HAVE_MP3;
            break;

        case ITE_WMA_DECODE:
            AUD_SKIP = !HAVE_WMA;
            break;

        case ITE_AAC_DECODE:
            AUD_SKIP = !HAVE_AAC;
            break;

        case ITE_AC3_DECODE:
        case ITE_AC3_SPDIF_DECODE:
            break;

        case ITE_AMR_CODEC:
            AUD_SKIP = !HAVE_AMR;
            break;

        case ITE_BSAC_DECODE:
            AUD_SKIP = !HAVE_BSAC;
            break;

        case ITE_WAV_DECODE:
            AUD_SKIP = !HAVE_WAV;
            break;

        case ITE_FLAC_DECODE:
            AUD_SKIP = !HAVE_FLAC;
            break;

        case ITE_SBC_CODEC:
            AUD_SKIP = !HAVE_SBC;
            break;

        case EXTERNAL_ENGINE_PLUGIN:
            break;

        default:
            return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;
            break;
        }
        if (AUD_SKIP)
            return 0;

    #if (CFG_CHIP_FAMILY == 9910)
        //if (pFile)
        {
            //if (length != PalFileRead((void*)&codec_start_addr, sizeof(char), length, pFile, ITH_NULL))
            //{
            //    LOG_ERROR "Can not read CODEC plugin\n" LOG_END
            //    AUD_SKIP = 1;
            //    return int32_t_ERROR;
            //}
            //printf("[Audio Driver]codec_start_addr 0x%x __header 0x%x __header 0x%x\n",(int)&codec_start_addr,__header,(int)&codec_end_addr);
            //pTemp = (uint8_t*)(int)&codec_start_addr;
            //memcpy((uint8_t*)pTemp, (uint8_t*)pAddress, length);

            codec_start_addr  = (int) iteRiscGetTargetMemAddress(RISC1_IMAGE_MEM_TARGET);
            __header = (struct _codec_header*) iteRiscGetTargetMemAddress(RISC1_IMAGE_MEM_TARGET);
            codec_end_addr    = (int) codec_start_addr + RISC1_IMAGE_SIZE;

            if (__header->magic != CODEC_MAGIC ||
                __header->target_id != TARGET_ID ||
                __header->api_version < CODEC_API_VERSION ||
                (int)__header->load_addr != codec_start_addr ||
                (int)__header->end_addr > codec_end_addr)
            {
                printf("CODEC header error\n");
                printf("__header->magic       0x%08x, expect value = 0x%08x\n", __header->magic, CODEC_MAGIC);
                printf("__header->target_id   0x%04x, expect value = 0x%04x\n", __header->target_id, TARGET_ID);
                printf("__header->api_version 0x%04x, expect value < 0x%04x\n", __header->api_version, CODEC_API_VERSION);
                printf("__header->load_addr   0x%08x, expect value = 0x%08x\n", __header->load_addr, codec_start_addr);
                printf("__header->end_addr    0x%08x, expect value < 0x%08x\n", __header->end_addr, codec_end_addr);
                AUD_SKIP = 1;
                return AUDIO_ERROR_CODEC_DO_NOT_INITIALIZE;
            }
            //{
            //    unsigned char* pImgBuf = NULL;
            //    uint8_t* pTemp;
            //    uint32_t nImagesize = 0;
            //
            //    memset(gWBuf, 0x0, sizeof(gWBuf));
            //
            //    pImgBuf = gWiegand;
            //    nImagesize = sizeof(gWiegand);
            //
            //    memcpy(gWBuf, pImgBuf, nImagesize);
            //    ithInvalidateDCache();
            //
            //    pTemp = (uint8_t*)(int)&codec_ex_start_addr;
            //    memcpy((uint8_t*)pTemp, (uint8_t*)gWBuf, nImagesize);
            //}
        }
        //PalFileClose(pFile, ITH_NULL);
        //pFile = NULL;
        printf("Load audio plugin complete.\n");
    #else
        //if (pFile)
        {
            //if (length != fread((void*)&codec_start_addr, sizeof(char), length, pFile, ITH_NULL))
            //{
            //    LOG_ERROR "Can not read CODEC plugin\n" LOG_END
            //    AUD_SKIP = 1;
            //    return int32_t_ERROR;
            //}
            //printf("[Audio Driver]codec_start_addr 0x%x __header 0x%x __header 0x%x\n",(int)&codec_start_addr,__header,(int)&codec_end_addr);
            codec_start_addr  = (CODEC_BASE + CODEC_START_ADDR);
            __header          = (struct _codec_header *)(CODEC_BASE + CODEC_START_ADDR);
            codec_end_addr    = CODEC_BASE + CODEC_START_ADDR + RISC1_IMAGE_SIZE;
            //printf("codec_start_addr: 0x%X, __header: 0x%X, codec_end_addr: 0x%X\n",codec_start_addr, __header, codec_end_addr);
            //printf("__codec_ex_header->magic 0x%08x, expect value = 0x%08x  %d\n", TOINT(__codec_ex_header->magic), (CODEC_MAGIC),TOINT(__codec_ex_header->magic) != (CODEC_MAGIC) );
            //printf("__header->end_addr  0x%08x codec_end_addr 0x%08x RISC1_SIZE %d CODEC_START_ADDR 0x%x\n",TOINT((int)__header->end_addr),codec_end_addr,RISC1_SIZE,CODEC_START_ADDR);
            //printf(" CODEC_BASE 0x%x CODEC_START_ADDR 0x%x __header 0x%x 0x%x  0x%x,0x%x\n",CODEC_BASE,CODEC_START_ADDR,__header,codec_start_addr,codec_start_addr-CODEC_BASE,codec_end_addr-CODEC_BASE);
            if (TOINT(__header->magic) != (CODEC_MAGIC) ||
                TOSHORT(__header->target_id) != (TARGET_ID) ||
                TOSHORT(__header->api_version) < (CODEC_API_VERSION) ||
                TOINT((int)__header->load_addr) != (CODEC_START_ADDR) ||
                TOINT((int)__header->end_addr ) > (codec_end_addr - CODEC_BASE))
            {
                printf("[Audio Driver]CODEC header error\n");
                printf(" __header->magic       0x%08x, expect value = 0x%08x  %d\n", TOINT(__header->magic), (CODEC_MAGIC), TOINT(__header->magic) != (CODEC_MAGIC) );
                printf(" __header->target_id   0x%04x, expect value = 0x%04x  %d\n", TOSHORT(__header->target_id), (TARGET_ID), TOSHORT(__header->target_id) != (TARGET_ID) );
                printf(" __header->api_version 0x%04x, expect value < 0x%04x %d\n", TOSHORT(__header->api_version), (CODEC_API_VERSION), TOSHORT(__header->api_version) < (CODEC_API_VERSION));
                printf(" __header->load_addr   0x%08x, expect value = 0x%08x %d\n", TOINT((int)__header->load_addr), (CODEC_START_ADDR), TOINT((int)__header->load_addr) != (codec_start_addr - CODEC_BASE));
                printf(" __header->end_addr    0x%08x, expect value < 0x%08x %d\n", TOINT((int)__header->end_addr), (codec_end_addr - CODEC_BASE), TOINT((int)__header->end_addr ) > (CODEC_START_ADDR + RISC1_SIZE));
                AUD_SKIP = 1;
                return AUDIO_ERROR_CODEC_DO_NOT_INITIALIZE;
            }
        }

        //LOG_DEBUG "Load audio plugin complete.\n" LOG_END
        //printf("[Audio Driver] load codec complete\n");
    #endif
    #if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
        //printf("[Audio Driver] load_addr 0x%08x end_addr 0x%08x stream 0x%08x \n",TOINT((int)__header->load_addr),TOINT((int)__header->end_addr),TOINT((int)__header->pStream));
        // write audio stream structure
        gAudioStream = (AUDIO_CODEC_STREAM *)iteRiscGetTargetMemAddress(SHARE_MEM1_TARGET);
        __header->pStream          = (char *)TOINT((int)gAudioStream - CODEC_BASE);
        //streamAddress = (int*)(CODEC_BASE+CODEC_STREAM_START_ADDRESS);
        gAudioStream->codecAudioPluginBuf    = TOINT((int)Audio_Plugin_Message_Buf - CODEC_BASE);
        gAudioStream->codecAudioPluginLength = TOINT(Audio_Plugin_Message_Buflen);
        if ((int)Audio_Plugin_Message_Buf < CODEC_BASE)
        {
            printf("Audio_Plugin_Message_Buf 0x%x < CODEC_BASE 0x%x\n", Audio_Plugin_Message_Buf, CODEC_BASE);
            AUD_SKIP = 1;
            return AUDIO_ERROR_CODEC_DO_NOT_INITIALIZE;
        }
        //printf("[Audio Driver] codec streams ptr 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x  0x%08x  0x%08x 0x%08x 0x%08x\n",__header->pStream,gAudioStream,streamAddress,gAudioStream->codecStreamBuf,gAudioStream->codecStreamLength,gAudioStream->codecStreamBuf,gAudioStream->codecStreamLength,gAudioStream->codecAudioPluginBuf,gAudioStream->codecAudioPluginLength);
    #endif     //defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
               // Flush Instruction Cacahe
               //ic_invalidate();
               // Restart Task

#ifdef CFG_CPU_WB
        ithFlushDCacheRange(gAudioStream, SHARE_MEM1_SIZE);
        ithFlushDCacheRange((void*)codec_start_addr, codec_end_addr - codec_start_addr);
        ithFlushMemBuffer();
#endif

    #ifdef ENABLE_AUDIO_PROCESSOR
        AUD_FireAudioProcessor();
        ret = 1;
    #endif
        if (pdFAIL == ret)
        {
            LOG_ERROR "Failed to create audio task\n" LOG_END
            AUD_SKIP = 1;
            return AUDIO_ERROR_ENGINE_LOAD_FAIL_ERROR;
        }
        AUD_PlugInType = audio_type;
    }    // AUD_PlugInType != audio_type
    else // AUD_PlugInType == audio_type
    {
        // Resume the task that stores in the memory
    #ifdef ENABLE_AUDIO_PROCESSOR
        AUD_ResetAudioProcessor();
        AUD_FireAudioProcessor();
    #endif
    }
#elif defined(ENABLE_CODECS_PLUGIN) && defined(ENABLE_CODECS_ARRAY)
    switch (audio_type)
    {
    case ITE_MP3_DECODE:
        AUD_SKIP = !HAVE_MP3;
        break;

    case ITE_WMA_DECODE:
        AUD_SKIP = !HAVE_WMA;
        break;

    case ITE_AAC_DECODE:
        AUD_SKIP = !HAVE_AAC;
        break;

    case ITE_AC3_DECODE:
        //AUD_SKIP = !HAVE_AC3;
        break;

    case ITE_AMR_CODEC:
        AUD_SKIP = !HAVE_AMR;
        break;

    case ITE_BSAC_DECODE:
        AUD_SKIP = !HAVE_BSAC;
        break;

    case ITE_MIDI:
        AUD_SKIP = !HAVE_MIDI;
        break;

    case ITE_WAV_DECODE:
        AUD_SKIP = !HAVE_WAV;
        break;

    case ITE_AC3_SPDIF_DECODE:
        break;

    default:
        return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;
        break;
    }
    if (AUD_SKIP)
        return 0;

    // Load Plug-Ins to memory
    if (AUD_PlugInType == 0 && audio_type == ITE_MP3_DECODE)
    {
        // The MP3 CODEC had already in memory on initial state.
        if (__header->magic != CODEC_MAGIC ||
            __header->target_id != TARGET_ID ||
            __header->api_version < CODEC_API_VERSION ||
            (int)__header->load_addr != (int)&codec_start_addr ||
            (int)__header->end_addr > (int)&codec_end_addr)
        {
            LOG_ERROR "CODEC header error\n" LOG_END
            LOG_ERROR " __header->magic       0x%08x, expect value = 0x%08x\n", __header->magic, CODEC_MAGIC LOG_END
            LOG_ERROR " __header->target_id   0x%04x, expect value = 0x%04x\n", __header->target_id, TARGET_ID LOG_END
            LOG_ERROR " __header->api_version 0x%04x, expect value < 0x%04x\n", __header->api_version, CODEC_API_VERSION LOG_END
            LOG_ERROR " __header->load_addr   0x%08x, expect value = 0x%08x\n", __header->load_addr, (int)&codec_start_addr LOG_END
            LOG_ERROR " __header->end_addr    0x%08x, expect value < 0x%08x\n", __header->end_addr, (int)&codec_end_addr LOG_END
            AUD_SKIP = 1;
            return int32_t_ERROR;
        }
        AUD_PlugInType = audio_type;
    #ifdef ENABLE_AUDIO_PROCESSOR
        AUD_ResetAudioProcessor();
        AUD_FireAudioProcessor();
    #endif
    }
    else if (AUD_PlugInType != audio_type)
    {
        signed portBASE_TYPE ret = pdFAIL;
        if (codecptr[audio_type] != (int *)codecbuf)
        {
            int i, tmp;
            int *src, *dst;

    #ifdef ENABLE_AUDIO_PROCESSOR
            AUD_ResetAudioProcessor();  // Stop CPU
    #endif

            // SWAP the CODEC
            dst = (int *)codecbuf;
            src = codecptr[audio_type];
            for (i = 0; i < CODEC_ARRAY_SIZE / sizeof(int); i++)
            {
                tmp    = dst[i];
                dst[i] = src[i];
                src[i] = tmp;
            }

            // swap the CODEC pointer
            tmp                      = (int)codecptr[audio_type];
            codecptr[audio_type]     = codecptr[AUD_PlugInType];
            codecptr[AUD_PlugInType] = (int *)tmp;

            // Flush Instruction Cache
    #ifdef __OR32__
            ic_invalidate();
    #endif
        }
        if (__header->magic != CODEC_MAGIC ||
            __header->target_id != TARGET_ID ||
            __header->api_version < CODEC_API_VERSION ||
            (int)__header->load_addr != (int)&codec_start_addr ||
            (int)__header->end_addr > (int)&codec_end_addr)
        {
            LOG_ERROR "CODEC header error\n" LOG_END
            LOG_ERROR " __header->magic       0x%08x, expect value = 0x%08x\n", __header->magic, CODEC_MAGIC LOG_END
            LOG_ERROR " __header->target_id   0x%04x, expect value = 0x%04x\n", __header->target_id, TARGET_ID LOG_END
            LOG_ERROR " __header->api_version 0x%04x, expect value < 0x%04x\n", __header->api_version, CODEC_API_VERSION LOG_END
            LOG_ERROR " __header->load_addr   0x%08x, expect value = 0x%08x\n", __header->load_addr, (int)&codec_start_addr LOG_END
            LOG_ERROR " __header->end_addr    0x%08x, expect value < 0x%08x\n", __header->end_addr, (int)&codec_end_addr LOG_END
            AUD_SKIP = 1;
            return int32_t_ERROR;
        }
        // Restart Task
    #ifdef ENABLE_AUDIO_PROCESSOR
        AUD_FireAudioProcessor();
        ret = 1;
    #endif
        if (pdFAIL == ret)
        {
            LOG_ERROR "Failed to create audio task\n" LOG_END
            AUD_SKIP = 1;
            return int32_t_ERROR;
        }
        AUD_PlugInType = audio_type;
    }
    else
    {
        // Resume the task that stores in the memory
    #ifdef ENABLE_AUDIO_PROCESSOR
        AUD_ResetAudioProcessor();
        AUD_FireAudioProcessor();
    #endif
    }
#endif     // !defined(ENABLE_CODECS_PLUGIN)

    write_waveHeader = 0;
    wave_ADPCM       = 0;
    write_wmaInfo    = 0;

#if defined(ENABLE_AUDIO_PROCESSOR) && defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    AUD_CheckProcessor();
    ithInvalidateDCacheRange(gAudioStream, SHARE_MEM1_SIZE);
#endif
    printf("[Audio Driver] %s #%d audio_type = %d\n",__FUNCTION__,__LINE__,audio_type);
    switch (audio_type)
    {
        
    case ITE_MP3_DECODE:
#if HAVE_MP3
    #if defined(ENABLE_AUDIO_PROCESSOR) && defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
        //printf("[Audio Driver] mp3/wav audio plugin Audio_Plugin_Message_Buflen %d 0x%x 0x%x\n", Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buflen, Audio_Plugin_Message_Buf);
        //__header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen, &AUD_freqAddress, &AUD_curPlayPos ,&Audio_Plugin_Message_Buf,&Audio_Plugin_Message_Buflen);
        Audio_Decode_Bufptr  = TOINT(gAudioStream->codecStreamBuf) + CODEC_BASE;
        Audio_Decode_Buflen  = TOINT(gAudioStream->codecStreamLength);
        Audio_Decode_Bufptr1 = TOINT(gAudioStream->codecStreamBuf1) + CODEC_BASE;
        Audio_Decode_Buflen1 = TOINT(gAudioStream->codecStreamLength1);
        //printf("[Audio Driver] Audio_Decode_Buf 0x%08x buf1 0x%08x Audio_Decode_Buflen %d len1 %d 0x%08x %d  0x%08x %d \n", Audio_Decode_Bufptr, Audio_Decode_Bufptr1, Audio_Decode_Buflen, Audio_Decode_Buflen1, gAudioStream->codecStreamBuf, gAudioStream->codecStreamLength, gAudioStream->codecAudioPluginBuf, gAudioStream->codecAudioPluginLength);
    #else
        __header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen, &AUD_freqAddress, &AUD_curPlayPos);
    #endif
        Audio_Decode_Buflen1 = Audio_Decode_Buflen2 = Audio_Decode_Buflen;
        //printf("[Audio Driver] Audio_Decode_Bufptr1 0x%x \n", Audio_Decode_Bufptr1);
#endif
        AUD_SKIP             = !HAVE_MP3;
        break;

    case ITE_WMA_DECODE:
#if HAVE_WMA
    #if defined(ENABLE_AUDIO_PROCESSOR) && defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
        //printf("[Audio Driver] wma  audio plugin buffer-length %d 0x%x 0x%x\n", Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buf);
        //__header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen, &AUD_freqAddress ,&Audio_Plugin_Message_Buf,&Audio_Plugin_Message_Buflen);
        Audio_Decode_Bufptr = TOINT(gAudioStream->codecStreamBuf) + CODEC_BASE;
        Audio_Decode_Buflen = TOINT(gAudioStream->codecStreamLength);
        //printf("[Audio Driver] stream buffer 0x%08x length %d  0x%08x %d \n", Audio_Decode_Bufptr, Audio_Decode_Buflen, gAudioStream->codecStreamBuf, gAudioStream->codecStreamLength);
    #else
        __header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen, &AUD_freqAddress);
    #endif
#endif
        AUD_SKIP = !HAVE_WMA;
        break;

    case ITE_AAC_DECODE:
#if HAVE_AAC
    #if defined(ENABLE_AUDIO_PROCESSOR) && defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
        //printf("[Audio Driver] aac audio plugin message buffer-length %d 0x%x 0x%x\n", Audio_Plugin_Message_Buflen, Audio_Plugin_Message_Buflen, Audio_Plugin_Message_Buf);
        //__header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen,&Audio_Plugin_Message_Buf,&Audio_Plugin_Message_Buflen);
        Audio_Decode_Bufptr = TOINT(gAudioStream->codecStreamBuf) + CODEC_BASE;
        Audio_Decode_Buflen = TOINT(gAudioStream->codecStreamLength);
        //printf("[Audio Driver] stream buffer 0x%08x length %d  0x%08x %d \n", Audio_Decode_Bufptr, Audio_Decode_Buflen, gAudioStream->codecStreamBuf, gAudioStream->codecStreamLength);
        //ithPrintf("[Audio Driver] stream buffer 0x%08x length %d  0x%08x %d \n", Audio_Decode_Bufptr, Audio_Decode_Buflen, gAudioStream->codecStreamBuf, gAudioStream->codecStreamLength);
    #else
        __header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen);
    #endif
#endif
        AUD_SKIP = !HAVE_AAC;
        break;

    case ITE_AC3_DECODE:
#if HAVE_AC3
    #if defined(ENABLE_AUDIO_PROCESSOR) && defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
        //printf("[Audio Driver] ac3 audio plugin buffer-length %d 0x%x 0x%x\n", Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buf);
        //__header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen,&Audio_Plugin_Message_Buf,&Audio_Plugin_Message_Buflen);
        Audio_Decode_Bufptr = TOINT(gAudioStream->codecStreamBuf) + CODEC_BASE;
        Audio_Decode_Buflen = TOINT(gAudioStream->codecStreamLength);
        //printf("[Audio Driver] stream buffer 0x%08x 0x%x length %d  0x%08x %d \n", Audio_Decode_Bufptr, TOINT(gAudioStream->codecStreamBuf), Audio_Decode_Buflen, gAudioStream->codecStreamBuf, gAudioStream->codecStreamLength);
    #else
        __header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen);
    #endif
#endif
        break;

    case ITE_AC3_SPDIF_DECODE:
#if HAVE_SPDIF_AC3
        __header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen);
#endif
        break;

    case ITE_AACPLUS_DECODE:
        AUD_SKIP = 1;
        break;

    case ITE_AMR_CODEC:
#if HAVE_AMR
    #if defined(ENABLE_AUDIO_PROCESSOR) && defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
        //printf("[Audio Driver] Amr audio plugin buffer-length %d 0x%x 0x%x\n", Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buf);
        //__header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen,&Audio_Encode_Bufptr, &Audio_Encode_Buflen,&Audio_Plugin_Message_Buf,&Audio_Plugin_Message_Buflen);
        Audio_Decode_Bufptr = TOINT(gAudioStream->codecStreamBuf) + CODEC_BASE;
        Audio_Decode_Buflen = TOINT(gAudioStream->codecStreamLength);
        //printf("[Audio Driver] stream buffer 0x%08x 0x%x length %d  0x%08x %d \n", Audio_Decode_Bufptr, TOINT(gAudioStream->codecStreamBuf), Audio_Decode_Buflen, gAudioStream->codecStreamBuf, gAudioStream->codecStreamLength);
    #else
        __header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen,
                             &Audio_Encode_Bufptr, &Audio_Encode_Buflen);
    #endif
#endif
        AUD_SKIP = !HAVE_AMR;
        break;

    case ITE_BSAC_DECODE:
        AUD_SKIP = !HAVE_BSAC;
        break;

    case ITE_MIXER:
        AUD_SKIP = !HAVE_MIXER;
        break;

    case ITE_PCM_CODEC:
        AUD_SKIP = !HAVE_PCM;
        break;

    case ITE_MIDI:
        AUD_SKIP = !HAVE_MIDI;
        break;

    case ITE_WAV_DECODE:
#if HAVE_WAV
    #if defined(ENABLE_AUDIO_PROCESSOR) && defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
        //printf("[Audio Driver] wav audio plugin message buffer-length %d 0x%x 0x%x\n", Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buf);
        Audio_Decode_Bufptr = TOINT(gAudioStream->codecStreamBuf) + CODEC_BASE;
        Audio_Decode_Buflen = TOINT(gAudioStream->codecStreamLength);
        //printf("[Audio Driver] 0x%08x %d  0x%08x %d 0x%08x 0x%08x\n", gAudioStream->codecStreamBuf, gAudioStream->codecStreamLength, Audio_Decode_Bufptr, Audio_Decode_Buflen, TOINT(gAudioStream->codecStreamBuf), CODEC_BASE);
    #else
        __header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen, &Audio_Encode_Bufptr, &Audio_Encode_Buflen, &AUD_freqAddress);
    #endif
#endif
        AUD_SKIP = !HAVE_WAV;
        break;

    case ITE_FLAC_DECODE:
#if defined(HAVE_FLAC)
    #if defined(ENABLE_AUDIO_PROCESSOR) && defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
        //printf("[Audio Driver] flac audio plugin buffer-length %d 0x%x 0x%x\n", Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buflen, &Audio_Plugin_Message_Buf);
        //__header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen,&Audio_Plugin_Message_Buf,&Audio_Plugin_Message_Buflen);
        Audio_Decode_Bufptr = TOINT(gAudioStream->codecStreamBuf) + CODEC_BASE;
        Audio_Decode_Buflen = TOINT(gAudioStream->codecStreamLength);
        //printf("[Audio Driver] stream buffer 0x%08x 0x%x length %d  0x%08x %d \n", Audio_Decode_Bufptr, TOINT(gAudioStream->codecStreamBuf), Audio_Decode_Buflen, gAudioStream->codecStreamBuf, gAudioStream->codecStreamLength);
    #else
        __header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen);
    #endif
#endif
        break;

    case ITE_SBC_CODEC:
#if HAVE_SBC
    #if defined(ENABLE_AUDIO_PROCESSOR) && defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
        Audio_Echo_Bufptr      = TOINT(gAudioStream->codecStreamBuf) + CODEC_BASE;
        Audio_Echo_Buflen      = TOINT(gAudioStream->codecStreamLength);
        Audio_Ref_Bufptr       = TOINT(gAudioStream->codecStreamBuf1) + CODEC_BASE;
        Audio_Ref_Buflen       = TOINT(gAudioStream->codecStreamLength1);
        Audio_Echocc_Bufptr    = TOINT(gAudioStream->codecStreamBuf2) + CODEC_BASE;
        Audio_Echocc_Buflen    = TOINT(gAudioStream->codecStreamLength2);
        Audio_Parameter_Cmdptr = TOINT(gAudioStream->codecParameterBuf) + CODEC_BASE;
        Audio_Parameter_Cmdlen = TOINT(gAudioStream->codecParameterLength);

        //printf("[Audio Driver] 0x%08x %d 0x%08x %d 0x%08x 0x%08x\n", gAudioStream->codecStreamBuf, gAudioStream->codecStreamLength, Audio_Decode_Bufptr, Audio_Decode_Buflen, TOINT(gAudioStream->codecStreamBuf), CODEC_BASE);
    #else
        //__header->codec_info(&Audio_Decode_Bufptr, &Audio_Decode_Buflen,&Audio_Encode_Bufptr, &Audio_Encode_Buflen, &AUD_freqAddress);
    #endif
#endif
        AUD_SKIP = !HAVE_SBC;
        break;

    case EXTERNAL_ENGINE_PLUGIN:
        break;

    default:
        return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;
        break;
    }
    //printf("[Audio Driver] audio plugin buffer-length %d 0x%x\n",Audio_Decode_Buflen,Audio_Decode_Bufptr);
    //iteAudioGetAttrib(ITE_AUDIO_ENABLE_MPLAYER, &bMPlayer);
    //if (bMPlayer==1)
    //{
    //    iteAudioSetWmaWithoutASFHeader(1);
    //}

    //AUD_CheckProcessor();

    Audio_DefDecode_Bufptr    = Audio_Decode_Bufptr;
    Audio_DefDecode_Bufptr1   = Audio_Decode_Bufptr1;
    Audio_DefDecode_Bufptr2   = Audio_Decode_Bufptr2;
    Audio_DefEncode_Bufptr    = Audio_Encode_Bufptr;

    // add for sbc codec on risc
    Audio_DefEcho_Bufptr      = Audio_Echo_Bufptr;
    Audio_DefRef_Bufptr       = Audio_Ref_Bufptr;
    Audio_DefEchocc_Bufptr    = Audio_Echocc_Bufptr;
    Audio_DefParameter_Cmdptr = Audio_Parameter_Cmdptr;

    AUD_EngineType            = audio_type;
    goto END;

RESET_ENGINE:
    if (AUD_ResetEngine() != 0)
        return AUDIO_ERROR_ENGINE_RESET_ENGINE_ERROR;
END:
    if (AUD_ConfigEngine() != 0)
        return AUDIO_ERROR_ENGINE_RESET_ENGINE_ERROR;

    if (AUD_SKIP)
        return 0;

    //sdk_msg(SDK_MSG_TYPE_ERROR, "audio.c(%d), [Audio Driver] audio load engine success \n", __LINE__);
    return AUD_InitialCodec();
}

int32_t
AUD_ConfigEngine(
    void)
{
    LOG_ENTER "AUD_ConfigEngine()\n" LOG_END

    switch (AUD_EngineType)
    {
    case ITE_AMR_CODEC:
        // fix amr codec do not use AEC
        ithWriteRegMaskH(DrvAudioCtrl, (0x2 << 8),  (0x3 << 8));
        // fix amr codec decode rate to 5.15kbps
        ithWriteRegMaskH(DrvAudioCtrl, ITE_AUDIO_AMR_ENCODE_1220, (0x7 << 11));
        break;

#if HAVE_PCM
    case ITE_PCM_CODEC:
        // codec mode.
        ithWriteRegMaskH(DrvAudioCtrl, (0x3 << 8), (0x3 << 8));
        break;
#endif
    }
    return 0;
}

int32_t
AUD_FireEngine(
    void)
{
    uint16_t regdata = DrvEnable;
    uint16_t cnt     = 0;

    //LOG_ENTER "AUD_FireEngine()\n" LOG_END

    // 0. if engine is running, stop fire engine
    if (AUD_GetEngineStatus(&AUD_EngineState) == 0)
    {
        switch (AUD_EngineState)
        {
        case ITE_AUDIO_ENGINE_RUNNING:
            return AUDIO_ERROR_ENGINE_IS_RUNNING;

        case ITE_AUDIO_ENGINE_IDLE:
            return 0;

        default:
            break;
        }
    }

    // 1. write enable bit
    ithWriteRegMaskH(DrvAudioCtrl, 0xffff, DrvEnable);

    // 2. fire engine
    ithWriteRegH(RISC_FIRE, 0x1);
    ithWriteRegH(RISC_FIRE, 0x0);
    // 3. wait for HW clean the enable bit
    do
    {
        regdata  = ithReadRegH (DrvAudioCtrl);
        regdata &= DrvEnable;
        if (regdata == 0)
        {
            return 0;
        }
        cnt++;
        ithDelay(5);
    } while (cnt < 400);

    return AUDIO_ERROR_ENGINE_CAN_NOT_START;

    return 0;
}

int32_t
AUD_ResetEngine(
    void)
{
    //LOG_ENTER "AUD_ResetEngine()\n" LOG_END

    // reset engine buffer address to default.
    Audio_Encode_Bufptr    = Audio_DefEncode_Bufptr;
    Audio_Decode_Bufptr    = Audio_DefDecode_Bufptr;
    Audio_Decode_Bufptr1   = Audio_DefDecode_Bufptr1;
    Audio_Decode_Bufptr2   = Audio_DefDecode_Bufptr2;

    // add for sbc codec risc
    Audio_Echo_Bufptr      = Audio_DefEcho_Bufptr;
    Audio_Ref_Bufptr       = Audio_DefRef_Bufptr;
    Audio_Echocc_Bufptr    = Audio_DefEchocc_Bufptr;
    Audio_Parameter_Cmdptr = Audio_DefParameter_Cmdptr;

    Audio_Encode_Wrptr     = 0;
    Audio_Encode_Rdptr     = 0;
    Audio_Decode_Wrptr     = 0;
    Audio_Decode_Rdptr     = 0;
    Audio_Decode_Wrptr1    = 0;
    Audio_Decode_Rdptr1    = 0;
    Audio_Decode_Wrptr2    = 0;
    Audio_Decode_Rdptr2    = 0;

    return 0;
}

int32_t
AUD_StopRISC(
    void)
{
    //LOG_ENTER "AUD_StopRISC()\n" LOG_END

    AUD_ResetEngine();
    //AUD_PowerOffRisc();

    return 0;
}

/*
 *  reset AudioCtrl2
 *
 */
int32_t AUD_ResetUserDefineIO3(
    void)
{
    ithWriteRegH(DrvAudioCtrl2,  0);
    return 1;
}

/*
 *  reset AudioEngineType
 *
 */
int32_t AUD_ResetAudioEngineType(
    void)
{
    AUD_EngineType = ITE_RESERVED;
    return 1;
}

int32_t
iteAudioInitialize(
    void)
{
    LOG_ENTER "iteAudioInitialize()\n" LOG_END
#ifdef AUDIO_FREERTOS
    lastTime        = PalGetClock();
#endif

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    //#if (CFG_CHIP_FAMILY == 9070)
    //gCodecCtxt = (CODEC_CONTEXT *) (iteRiscGetTargetMemAddress(SHARE_MEM1_TARGET) + sizeof(AUDIO_CODEC_STREAM));
    Audio_Plugin_Message_Buf   = (short*) iteRiscGetTargetMemAddress(AUDIO_MESSAGE_MEM_TARGET);
    Audio_Plugin_Message_Buflen = AUDIO_MESSAGE_SIZE / sizeof(short);
    gAudioStream               = (AUDIO_CODEC_STREAM*) iteRiscGetTargetMemAddress(SHARE_MEM1_TARGET);
    //#endif
#endif
    
    AUD_baseAddress = 0;

    // init RISC Clock
///#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9850)    
//    ithWriteRegH(0x0044, 0x07FF);
//#else	
//    ithWriteRegH(0x0044, 0x03FF);
//#endif

    AUD_Init = 1;
    return 0;
}

int32_t
iteAudioSetCodec(
    unsigned long *attribList)
{
    LOG_ENTER "iteAudioSetCodec()\n" LOG_END

    if (!attribList)
        return AUDIO_ERROR_CODEC_DO_NOT_INITIALIZE;

    if (!AUD_Codec.codecInit)
    {
        while (*attribList != ITE_AUDIO_CODEC_NONE)
        {
            switch (*attribList++)
            {
            case ITE_AUDIO_CODEC_INITIALIZE:
                AUD_Codec.CODEC_Initialize = (CODEC_INITIALIZE) *attribList++;
                break;

            case ITE_AUDIO_CODEC_TERMINATE:
                AUD_Codec.CODEC_Terminate = (CODEC_TERMINATE) *attribList++;
                break;

            case ITE_AUDIO_CODEC_SETVOLUME:
                AUD_Codec.CODEC_SetVolume = (CODEC_SETVOLUME) *attribList++;
                break;

            case ITE_AUDIO_CODEC_MODE:
                AUD_Codec.codecMode = (uint32_t) *attribList++;
                break;

            case ITE_AUDIO_CODEC_ADC_RATIO:
                AUD_Codec.adcRatio = (uint32_t) *attribList++;
                break;

            case ITE_AUDIO_CODEC_DAC_RATIO:
                AUD_Codec.dacRatio = (uint32_t) *attribList++;
                break;

            case ITE_AUDIO_CODEC_OVERSAMPLING_RATE:
                AUD_Codec.sampleRate = (uint32_t) *attribList++;
                break;
            }
        }
    }

    if (AUD_InitialCodec() != 0)
        return AUDIO_ERROR_CODEC_DO_NOT_INITIALIZE;

    return 0;
}

int32_t
iteAudioGetAvailableBufferLength(
    ITE_AUDIO_BUFFER_TYPE bufferType,
    unsigned long *bufferLength)
{
    //LOG_ENTER "iteAudioGetAvailableBufferLength()\n" LOG_END

    if (AUD_SKIP)
    {
        *bufferLength = 65535;
        return 0;
    }

    switch (bufferType)
    {
    case ITE_AUDIO_INPUT_BUFFER:
        if (AUD_GetReadAvailableBufferLength((uint32_t *)bufferLength) == 0)
            return 0;
        break;

    case ITE_AUDIO_OUTPUT_BUFFER:
        if (AUD_GetWriteAvailableBufferLength((uint32_t *)bufferLength) == 0)
        {
            if (*bufferLength < BufThreshold)
                *bufferLength = 0;

            return 0;
        }
        break;

    case ITE_AUDIO_MIXER_BUFFER:
        return 0;

    default:
        break;
    }

    *bufferLength = 0;
    return AUDIO_ERROR_CODEC_UNKNOW_ERROR;
}

int32_t
iteAudioGetFlashAvailableBufferLength(
    int nAudioInput,
    unsigned long *bufferLength)
{
    //LOG_ENTER "mmpAudioGetAvailableBufferLength()\n" LOG_END
    uint16_t hwpr;
    uint16_t swpr;

    if (AUD_SKIP)
    {
        *bufferLength = 65535;
        return 0;
    }

    if (nAudioInput < 0)
        return 0;
    //PRECONDITION(Audio_Decode_Buflen >=  4);
    //PRECONDITION(Audio_Decode_Buflen % 2 == 0); // word align

    switch (nAudioInput)
    {
    case 0:
        swpr = Audio_Decode_Wrptr;
        hwpr = ithReadRegH(DrvDecode_RdPtr);
        //hwpr = ((hwpr >> 1) << 1); // word align
        // Get the avalible length of buffer
        if (swpr == hwpr)
        {
            (*bufferLength) = Audio_Decode_Buflen - 2;
        }
        else if (swpr > hwpr)
        {
            (*bufferLength) = Audio_Decode_Buflen - (swpr - hwpr) - 2;
        }
        else
        {
            (*bufferLength) = hwpr - swpr - 2;
        }
        break;

    case 1:
        swpr = Audio_Decode_Wrptr1;
        hwpr = ithReadRegH(DrvDecode_RdPtr1);
        //hwpr = ((hwpr >> 1) << 1); // word align
        // Get the avalible length of buffer
        if (swpr == hwpr)
        {
            (*bufferLength) = Audio_Decode_Buflen1 - 2;
        }
        else if (swpr > hwpr)
        {
            (*bufferLength) = Audio_Decode_Buflen1 - (swpr - hwpr) - 2;
        }
        else
        {
            (*bufferLength) = hwpr - swpr - 2;
        }
        break;

    case 2:
        swpr = Audio_Decode_Wrptr2;
        hwpr = ithReadRegH(DrvDecode_RdPtr2);
        //hwpr = ((hwpr >> 1) << 1); // word align
        // Get the avalible length of buffer
        if (swpr == hwpr)
        {
            (*bufferLength) = Audio_Decode_Buflen2 - 2;
        }
        else if (swpr > hwpr)
        {
            (*bufferLength) = Audio_Decode_Buflen2 - (swpr - hwpr) - 2;
        }
        else
        {
            (*bufferLength) = hwpr - swpr - 2;
        }
        break;

    default:
        break;
    }

    if (*bufferLength < BufThreshold)
        *bufferLength = 0;

    return 0;
}

int32_t
iteAudioGetEngineAttrib(
    ITE_AUDIO_ENGINE_ATTRIB attrib,
    void *value)
{
    LOG_ENTER "iteAudioGetEngineAttrib()\n" LOG_END

    switch (attrib)
    {
    case ITE_AUDIO_ENGINE_TYPE:
        *((ITE_AUDIO_ENGINE *)value) = AUD_EngineType;
        break;

    case ITE_AUDIO_ENGINE_STATUS:
        AUD_GetEngineStatus((uint32_t *)value);
        break;

    default:
        return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;
    }
    return 0;
}

unsigned long
iteAudioReadStream(
    uint8_t *stream,
    unsigned long streamBufferSize,
    unsigned long streamthreshold)
{
    uint32_t readableQueueLen;
    uint32_t bottomLen;
    uint32_t topLen;
    uint32_t residualLength;

    LOG_ENTER "iteAudioReadStream()\n" LOG_END

    if (AUD_SKIP)
    {
        return streamBufferSize;
    }

    switch (AUD_EngineType)
    {
    case ITE_AMR_CODEC:
    case ITE_AMR_ENCODE:
    case ITE_WAV_DECODE:
    case ITE_PCM_CODEC:
    case ITE_SBC_CODEC:
        break;

    default:
        return 0;
    }

    // word align
    //if (AUD_EngineType != ITE_SBC_CODEC)
    //{
    //    streamBufferSize = (streamBufferSize >> 1);
    //    streamBufferSize = (streamBufferSize << 1);
    //}

    residualLength = streamBufferSize;

    if (streamthreshold > Audio_Encode_Buflen)
        streamthreshold = (Audio_Encode_Buflen - 2);

    AUD_GetReadAvailableBufferLength(&readableQueueLen);

    // Wait buffer avaliable
    if (readableQueueLen < streamthreshold)
    {
        return 0;
    }

    // PRECONDITION(readableQueueLen < Audio_Encode_Buflen);

    if (readableQueueLen > residualLength)
        readableQueueLen = residualLength;

    //ithInvalidateDCache();
    ithInvalidateDCacheRange(Audio_Encode_Bufptr, Audio_Encode_Buflen);
    if (readableQueueLen != 0)
    {
        bottomLen = Audio_Encode_Buflen - Audio_Encode_Rdptr;

        if (bottomLen > readableQueueLen)
            bottomLen = readableQueueLen;

        memcpy((void *)stream,
               (const void *)(AUD_baseAddress + Audio_Encode_Bufptr + Audio_Encode_Rdptr),
               bottomLen);
        stream += bottomLen;

        topLen  = readableQueueLen - bottomLen;

        if (topLen > 0)
        {
            memcpy((void *)(stream),
                   (const void *)(AUD_baseAddress + Audio_Encode_Bufptr),
                   topLen );
            stream += topLen;
        }
    }

    // Update Read Pointer (word alignment)
    Audio_Encode_Rdptr += readableQueueLen;

    if (Audio_Encode_Rdptr >= Audio_Encode_Buflen)
    {
        Audio_Encode_Rdptr -= Audio_Encode_Buflen;
    }
    ithWriteRegH(DrvEncode_RdPtr, Audio_Encode_Rdptr);

    return readableQueueLen;
}

int32_t
iteAudioWriteStream(
    uint8_t *stream,
    unsigned long length)
{
    uint32_t drvTimeout = 0;
    uint32_t writableQueueLen;
    uint32_t bottomLen;
    uint32_t topLen;
    uint32_t residualLength;
    uint32_t wav_length   = 0;
    uint32_t adpcm_length = 4;

    // LOG_ENTER "iteAudioWriteStream()\n" LOG_END

    if (AUD_SKIP)
        return 0;

#if defined(AUDIO_DRIVER_SHOW_WRITE_BITRATE)
    gAudioWrite += length;
    if (PalGetDuration(gtAudioDriverWirteBitrate) - gAudioWriteDuration >= 1000)
    {
        gAudioWriteDuration += 1000;
        printf("[Audio Driver] w duration %d data %d \n", PalGetDuration(gtAudioDriverWirteBitrate), gAudioWrite);
        gAudioWrite          = 0;
    }
#endif

    if (write_waveHeader == 1)
    {
        if (wave_ADPCM == 4)
        {
            adpcm_length = 0;
        }
        do
        {
            AUD_GetWriteAvailableBufferLength(&wav_length);
            //printf("[Audio Driver] wav_length %d \n",wav_length);
            if ((sizeof(wave_header) - adpcm_length) > length)
            {
                ithDelay(1);
            }
            else
            {
                break;
            }
            drvTimeout++;
        } while (drvTimeout < 200);

        if (drvTimeout < 200)
        {
            memcpy((void *)(AUD_baseAddress + Audio_Decode_Bufptr + Audio_Decode_Wrptr),
                   (const void *)wave_header,
                   (sizeof(wave_header) - adpcm_length));
            Audio_Decode_Wrptr = (uint16_t)(Audio_Decode_Wrptr + (sizeof(wave_header) - adpcm_length) );
            write_waveHeader   = 0;
        }
        else if (drvTimeout >= 200)
        {
            write_waveHeader = 0;
        }
    }

    if (write_wmaInfo == 1)
    {
        do
        {
            AUD_GetWriteAvailableBufferLength(&wav_length);
            printf("[Audio Driver] wma_length %d \n", wav_length);
            if ((sizeof(wma_info)) > wav_length)
            {
                ithDelay(1);
            }
            else
            {
                break;
            }
            drvTimeout++;
        } while (drvTimeout < 200);

        if (drvTimeout < 200)
        {
            memcpy((void *)(AUD_baseAddress + Audio_Decode_Bufptr + Audio_Decode_Wrptr),
                   (const void *)wma_info,
                   (sizeof(wma_info)));
            Audio_Decode_Wrptr = (uint16_t)(Audio_Decode_Wrptr + (sizeof(wma_info)));
            write_wmaInfo      = 0;
        }
        else if (drvTimeout >= 200)
        {
            write_wmaInfo = 0;
        }
    }

    drvTimeout = 0;

#if defined(OUTPUT_TO_FILE)
    if (!fout && AUD_EngineType == ITE_MP3_DECODE)
    {
        fout = fopen( "c:/audio_mp3.mp3", "wb");
        printf("[audio driver] file  open  \n");
        // f_enterFS();

        if (fout == NULL)
        {
            //LOG_ERROR "Can not create audio output file %d \n",f_getlasterror() LOG_END
            printf("[Audio Driver] fopen null \n");
            // return int32_t_ERROR;
        }
        if (AUD_EngineType == ITE_WAV_DECODE)
        {
            //PalFileWrite(wave_header, sizeof(char), (sizeof(wave_header)-adpcm_length), fout, ITH_NULL);
            fwrite(wave_header, sizeof(char), (sizeof(wave_header) - adpcm_length), fout);
        }
    }

    if (fout)
    {
        if (length != fwrite(stream, sizeof(char), length, fout))
        {
            LOG_ERROR "Can not write audio output file\n" LOG_END
            printf("[Audio Driver]Can not write audio output file \n");
            //return int32_t_ERROR;
        }
        if (AUD_EngineType == ITE_WMA_DECODE)
        {
            return 0;
        }
    }
#endif

#if defined(DONOT_PLAY_AUDIO)
    return 0;
#endif

    switch (AUD_EngineType)
    {
    case ITE_AMR_ENCODE:
        return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;

    default:
        break;
    }

#if !defined(ENABLE_SXA_MPS) || !(ENABLE_SXA_MPS)
    //length = ((length + 1)>>1)<<1; //word align
#endif      /* !defined(ENABLE_SXA_MPS) || (ENABLE_SXA_MPS) */

    residualLength = length;

    while (residualLength)
    {
        // Wait buffer avaliable
        //PRECONDITION(Audio_Decode_Buflen > BufThreshold);

        do
        {
            AUD_GetWriteAvailableBufferLength(&writableQueueLen);
            if (writableQueueLen < BufThreshold)
            {
                ithDelay(1);
            }
            else
            {
                break;
            }
            drvTimeout++;
        } while (drvTimeout < 200);

        if (drvTimeout == 200 && (writableQueueLen < BufThreshold))
        {
            if (AUD_GetEngineStatus(&AUD_EngineState) != 0)
            {
                AUD_ResetEngine();
                return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;
            }
            else
            {
                continue;
            }
        }

        if (residualLength <= writableQueueLen)
        {
            writableQueueLen = residualLength;
        }

        // streamptr += writableQueueLen;
#if     !defined(ENABLE_SXA_MPS) || !(ENABLE_SXA_MPS)
        //writableQueueLen = (writableQueueLen>>1)<<1; // length should be word alignment.
#endif          /* !defined(ENABLE_SXA_MPS) || (ENABLE_SXA_MPS) */

        if (writableQueueLen != 0)
        {
            bottomLen = Audio_Decode_Buflen - Audio_Decode_Wrptr;
            if (bottomLen > writableQueueLen)
            {
                bottomLen = writableQueueLen;
            }

            memcpy((void *)(AUD_baseAddress + Audio_Decode_Bufptr + Audio_Decode_Wrptr),
                   (const void *)stream,
                   bottomLen);
            ithFlushDCacheRange((void *)(AUD_baseAddress + Audio_Decode_Bufptr + Audio_Decode_Wrptr), bottomLen);

            stream += bottomLen;
            topLen  = writableQueueLen - bottomLen;

            if (topLen > 0)
            {
                memcpy((void *)(AUD_baseAddress + Audio_Decode_Bufptr),
                       (const void *)(stream),
                       topLen);
                ithFlushDCacheRange((void *)(AUD_baseAddress + Audio_Decode_Bufptr), topLen);

                stream += topLen;
            }

            ithFlushMemBuffer();
        }

        // Update Write Pointer (word alignment)
        Audio_Decode_Wrptr = (uint16_t)(Audio_Decode_Wrptr + writableQueueLen);

        residualLength    -= writableQueueLen;

        if (Audio_Decode_Wrptr >= Audio_Decode_Buflen)
        {
            Audio_Decode_Wrptr -= Audio_Decode_Buflen;
        }

        ithWriteRegH(DrvDecode_WrPtr, Audio_Decode_Wrptr);
    }

    return 0;
}

int32_t
iteAudioWriteFlashStream(
    int nAudioInput,
    uint8_t *stream,
    unsigned long length)
{
    unsigned int drvTimeout = 0;
    unsigned int writableQueueLen;
    unsigned int bottomLen;
    unsigned int topLen;
    unsigned int residualLength;
    unsigned int wav_length   = 0;
    unsigned int adpcm_length = 4;
    unsigned int nTempBufptr;
    unsigned int nTempBuflen;
    unsigned int nTempWrptr;

    // LOG_ENTER "mmpAudioWriteStream()\n" LOG_END
    if (AUD_SKIP)
        return 0;

    if (nAudioInput < 0)
        return 0;
#if defined(AUDIO_DRIVER_SHOW_WRITE_BITRATE)
    gAudioWrite += length;
    if (PalGetDuration(gtAudioDriverWirteBitrate) - gAudioWriteDuration >= 1000)
    {
        gAudioWriteDuration += 1000;
        printf("[Audio Driver] w duration %d data %d \n", PalGetDuration(gtAudioDriverWirteBitrate), gAudioWrite);
        gAudioWrite          = 0;
    }
#endif
    drvTimeout = 0;

#if defined(DONOT_PLAY_AUDIO)
    return MMP_RESULT_SUCCESS;
#endif

    length         = ((length + 1) >> 1) << 1; //word align
    residualLength = length;

    while (residualLength)
    {
        // Wait buffer avaliable
        //PRECONDITION(Audio_Decode_Buflen > BufThreshold);

        do
        {
            iteAudioGetFlashAvailableBufferLength(nAudioInput, (unsigned long *)&writableQueueLen);
            if (writableQueueLen < BufThreshold)
            {
                ithDelay(1);
            }
            else
            {
                break;
            }
            drvTimeout++;
        } while (drvTimeout < 200);

        if (drvTimeout == 200 && (writableQueueLen < BufThreshold))
        {
            if (AUD_GetEngineStatus(&AUD_EngineState) != 0)
            {
                AUD_ResetEngine();
                return 1;
            }
            else
            {
                continue;
            }
        }

        if (residualLength <= writableQueueLen)
        {
            writableQueueLen = residualLength;
        }

        writableQueueLen = (writableQueueLen >> 1) << 1; // length should be word alignment.
        switch (nAudioInput)
        {
        case 0:
            nTempBufptr = Audio_Decode_Bufptr;
            nTempBuflen = Audio_Decode_Buflen;
            nTempWrptr  = Audio_Decode_Wrptr;
            break;

        case 1:
            nTempBufptr = Audio_Decode_Bufptr1;
            nTempBuflen = Audio_Decode_Buflen1;
            nTempWrptr  = Audio_Decode_Wrptr1;
            //printf("[Audio Driver] w buffer 0x%x %d %d \n",nTempBufptr,nTempBuflen,nTempWrptr);
            break;

        case 2:
            nTempBufptr = Audio_Decode_Bufptr2;
            nTempBuflen = Audio_Decode_Buflen2;
            nTempWrptr  = Audio_Decode_Wrptr2;
            //printf("[Audio Driver] w buffer 0x%x %d %d \n",nTempBufptr,nTempBuflen,nTempWrptr);
            break;

        default:
            break;
        }
        if (writableQueueLen != 0)
        {
            bottomLen = nTempBuflen - nTempWrptr;
            if (bottomLen > writableQueueLen)
            {
                bottomLen = writableQueueLen;
            }

            memcpy((void *)(AUD_baseAddress + nTempBufptr + nTempWrptr),
                   (const void *)stream,
                   bottomLen);
            stream += bottomLen;

            topLen  = writableQueueLen - bottomLen;

            if (topLen > 0)
            {
                memcpy((void *)(AUD_baseAddress + nTempBufptr),
                       (const void *)(stream),
                       topLen);
                stream += topLen;
            }
        }

        // Update Write Pointer (word alignment)
        nTempWrptr      = (unsigned short)(nTempWrptr + writableQueueLen);

        residualLength -= writableQueueLen;

        if (nTempWrptr >= nTempBuflen)
        {
            nTempWrptr -= nTempBuflen;
        }

        switch (nAudioInput)
        {
        case 0:
            ithWriteRegH(DrvDecode_WrPtr, nTempWrptr);
            Audio_Decode_Bufptr = nTempBufptr;
            Audio_Decode_Buflen = nTempBuflen;
            Audio_Decode_Wrptr  = nTempWrptr;
            break;

        case 1:
            ithWriteRegH(DrvDecode_WrPtr1, nTempWrptr);
            Audio_Decode_Bufptr1 = nTempBufptr;
            Audio_Decode_Buflen1 = nTempBuflen;
            Audio_Decode_Wrptr1  = nTempWrptr;
            break;

        case 2:
            ithWriteRegH(DrvDecode_WrPtr2, nTempWrptr);
            Audio_Decode_Bufptr2 = nTempBufptr;
            Audio_Decode_Buflen2 = nTempBuflen;
            Audio_Decode_Wrptr2  = nTempWrptr;
            break;

        default:
            break;
        }
    }

    return 0;
}

#if defined(DRIVER_CODEC) && !defined(CFG_COMPRESS_AUDIO_PLUGIN)
    #define ADDR_AND_SIZE(img) (img), sizeof(img)
#else
    #define ADDR_AND_SIZE(img) NULL, 0
#endif

#define END_OF_CODEC_IMAGE_TABLE { NULL, ITE_RESERVED, NULL, 0 }

typedef struct AUDIO_CODEC_IMAGE_TAG
{
    const char             *name;
    const ITE_AUDIO_ENGINE id;
    const unsigned char    *addr;
    const uint32_t         size;
} AUDIO_CODEC_IMAGE;

static const AUDIO_CODEC_IMAGE g_codec_image[] = {
#ifdef CFG_AUDIO_CODEC_MP3DEC
    { "mp3",    ITE_MP3_DECODE,  ADDR_AND_SIZE(gMp3Decoder) },
#endif
#ifdef CFG_AUDIO_CODEC_AACDEC
    { "aac",    ITE_AAC_DECODE,  ADDR_AND_SIZE(gAacDecoder) },
#endif
#ifdef CFG_AUDIO_CODEC_EAC3DEC
    { "eac3",   ITE_AC3_DECODE,  ADDR_AND_SIZE(gAc3Decoder) },
#endif
#ifdef CFG_AUDIO_CODEC_WAV
    { "wave",   ITE_WAV_DECODE,  ADDR_AND_SIZE(gWaveDecoder)},
#endif
#ifdef CFG_AUDIO_CODEC_WMADEC
    { "wma",    ITE_WMA_DECODE,  ADDR_AND_SIZE(gWmaDecoder) },
#endif
#ifdef CFG_AUDIO_CODEC_AMR
    { "amr",    ITE_AMR_DECODE,  ADDR_AND_SIZE(gAmrDecoder) },
#endif
#ifdef CFG_AUDIO_CODEC_FLACDEC
    { "flac",   ITE_FLAC_DECODE, ADDR_AND_SIZE(gFlacDecoder)},
#endif
    { "sbc",    ITE_SBC_CODEC,   ADDR_AND_SIZE(gSbcCodec)   },
    END_OF_CODEC_IMAGE_TABLE
};

uint32_t
audioReadCodec(ITE_AUDIO_ENGINE audio_type)
{
    uint32_t      nImagesize = 0;
    int           i;

#if defined(DRIVER_CODEC) && !defined(CFG_COMPRESS_AUDIO_PLUGIN)
    unsigned char *pImgBuf = NULL;

    for (i = 0; g_codec_image[i].id != ITE_RESERVED; ++i)
    {
        if (g_codec_image[i].id == audio_type)
        {
            printf("open %s engine\n", g_codec_image[i].name);
            pImgBuf    = g_codec_image[i].addr;
            nImagesize = g_codec_image[i].size;
            break;
        }
    }

    #if (CFG_CHIP_FAMILY == 9910)
    // copy audio codec (.hex) to start address
    if (pImgBuf != NULL && nImagesize != 0)
    {
        iteRiscLoadData(RISC1_IMAGE_MEM_TARGET, (uint8_t *)pImgBuf, nImagesize);
    }
    //if (pImgBuf != NULL && nImagesize != 0)
        //memcpy((uint8_t *)&codec_start_addr, (uint8_t *)pImgBuf, nImagesize);
    #else     //if (CFG_CHIP_FAMILY == 9070)
    //memset(gBuf, 0x0, sizeof(gBuf));
    // copy audio codec (.hex) to gBuf
    if (pImgBuf != NULL && nImagesize != 0)
    {
        iteRiscLoadData(RISC1_IMAGE_MEM_TARGET, (uint8_t *)pImgBuf, nImagesize);
    }

    //ithInvalidateDCache();
    #endif
#elif !defined(DRIVER_CODEC) && !defined(CFG_COMPRESS_AUDIO_PLUGIN)
    FILE     *f      = NULL;
    uint32_t nResult = 0;

    for (i = 0; g_codec_image[i].id != ITE_RESERVED; ++i)
    {
        if (g_codec_image[i].id == audio_type)
        {
            char codec_file_name[40]; // ex. CFG_PRIVATE_DRIVE ":/codec/mp3.codecs"

            printf("open %s engine\n", g_codec_image[i].name);
            assert(strlen(g_codec_image[i].name) < (sizeof(codec_driver_name) - strlen(CFG_PRIVATE_DRIVE ":/codec/.codecs")));
            sprintf(codec_file_name, CFG_PRIVATE_DRIVE ":/codec/%s.codecs", g_codec_image[i].name);

            f = fopen(codec_file_name, "rb");
            if (!f)
            {
                sprintf(codec_file_name, "A:/%s.codecs", g_codec_image[i].name);    // ex. "A:/mp3.codecs"
                f = fopen(codec_file_name, "rb");
            }
            break;
        }
    }

    if (!f)
    {
        printf("can not open codec \n");
        return 0;
    }
    fseek(f, 0, SEEK_END);
    nImagesize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (nImagesize < sizeof(gBuf))
    {
        nResult = fread(gBuf, 1, nImagesize, f);
        if (nResult != nImagesize)
            printf(" image size %d  wrong  %d \n", nImagesize, nResult);
    }
    else
    {
        printf("image size(%s) is too large.\n", nImagesize);
        nImagesize = 0;
    }

    printf(" buf 0x%x ,0x%x ,0x%x ,0x%x ,0x%x ,0x%x ,0x%x ,0x%x ,0x%x ,0x%x ,0x%x ,0x%x \n", gBuf[0], gBuf[1], gBuf[2], gBuf[3], gBuf[4], gBuf[5], gBuf[6], gBuf[7], gBuf[8], gBuf[9], gBuf[10], gBuf[11]);
    fclose(f);
#else
    for (i = 0; g_codec_image[i].id != ITE_RESERVED; ++i)
    {
        if (g_codec_image[i].id == audio_type)
        {
            char codec_driver_name[40]; // ex. ":codec:mp3"
            int  fd;

            printf("open %s engine\n", g_codec_image[i].name);
            assert(strlen(g_codec_image[i].name) < (sizeof(codec_driver_name) - strlen(":codec:")));
            sprintf(codec_driver_name, ":codec:%s", g_codec_image[i].name);

            fd = open(codec_driver_name, 0);
            if (fd)
            {
                ioctl(fd, ITP_IOCTL_GET_SIZE, &nImagesize);
                if (nImagesize < RISC1_IMAGE_SIZE)
                    read(fd, iteRiscGetTargetMemAddress(RISC1_IMAGE_MEM_TARGET), nImagesize);
                else
                {
                    printf("image size(%s) is too large.\n", nImagesize);
                    nImagesize = 0;
                }
                close(fd);
            }
            break;
        }
    }
#endif
    return nImagesize;
}

int32_t
iteAudioOpenEngine(
    ITE_AUDIO_ENGINE audio_type)
{
    LOG_ENTER "iteAudioOpenEngine()\n" LOG_END
    pthread_mutex_lock(&RiscMutex);
    if (!AUD_Init)
    {
        iteAudioInitialize();
    }
    //else return;

#if defined(AUDIO_DRIVER_SHOW_WRITE_BITRATE)
    gtAudioDriverWirteBitrate = PalGetClock();
    gAudioWriteDuration       = 0;
#endif

#ifdef ENABLE_AUDIO_PROCESSOR
    AUD_ResetAudioProcessor();
#endif
    //get the audio codec address , length
    audioReadCodec(audio_type);
    //load engine
    //printf("[Audio Driver] iteAudioOpenEngine 0x%x %d \n",pAddress,length);
    if (AUD_LoadEngine(audio_type) != 0)
    {
        pthread_mutex_unlock(&RiscMutex);
        return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;
    }
    pthread_mutex_unlock(&RiscMutex);
    return 0;
}

int32_t
iteAudioActiveEngine(
    void)
{
    LOG_ENTER "iteAudioActiveEngine()\n" LOG_END
#ifdef AUDIO_FREERTOS
    lastTime = PalGetClock();
#endif
    switch (AUD_FireEngine())
    {
    case AUDIO_ERROR_ENGINE_IS_RUNNING:
    case 0:
        {
            //        writeStream = 0;
            return 0;
        }

    default:
        return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;
    }
}

int32_t
iteAudioActiveMidiEngine(
    uint8_t *midiData,
    unsigned long midiDataSize)
{
    LOG_ENTER "iteAudioActiveMidiEngine()\n" LOG_END

    if (!AUD_Init)
    {
        iteAudioInitialize();
    }

    // fire engine
    AUD_FireEngine();

    return 0;
}

int32_t
iteAudioSetVolume(
    uint32_t vol)
{
    LOG_ENTER "iteAudioSetVolume()\n" LOG_END
    if (AUD_Codec.CODEC_SetVolume)
    {
        AUD_Codec.CODEC_SetVolume(vol);
        AUD_Codec.curVolume = vol;
    }
    return 0;
}

int32_t
iteAudioGetVolume(
    uint32_t *vol)
{
    LOG_ENTER "iteAudioGetVolume()\n" LOG_END

    if (AUD_Codec.CODEC_SetVolume)
    {
        *vol = AUD_Codec.curVolume;
    }

    return 0;
}

int32_t
iteAudioStopEngine(
    void)
{
    LOG_ENTER "iteAudioStopEngine()\n" LOG_END

    AUD_Init = 0; // need to init again
    return 0;
}

int32_t
iteAudioTerminate(
    void)
{
    LOG_ENTER "iteAudioTerminate()\n" LOG_END

    AUD_TerminateCodec();
    AUD_ResetUserDefineIO3();
    AUD_EngineType = 0;
    return 0;
}

int32_t
iteAudioSetEqualizer(
    bool enableEq)
{
    LOG_ENTER "iteAudioSetEqualizer()\n" LOG_END

    if (enableEq == 1 && AUD_eqStatus != 1)
    {
        AUD_SetEnEQ(enableEq);
        AUD_SetEnDRC(1);
    }
    else
    {
        AUD_SetEnEQ(enableEq);
        AUD_SetEnDRC(0);
    }
    return 0;
}

__inline int32_t
iteAudioChangeEqTable(
    ITE_AUDIO_EQINFO *eqTable)
{
    LOG_ENTER "iteAudioChangeEqTable()\n" LOG_END

    if (eqTable == NULL)
        return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;

    memcpy((void *)&eqinfo, (const void *)eqTable, sizeof(ITE_AUDIO_EQINFO));

    return 0;
}

int32_t
iteAudioChangeEqualizer(
    ITE_AUDIO_EQTYPE eqType)
{
    LOG_ENTER "iteAudioChangeEqualizer()\n" LOG_END

    if (eqType < 0 || eqType >= (sizeof(eqinfo_def) / sizeof(eqinfo_def[0])))
        return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;

    if (iteAudioChangeEqTable(&eqinfo_def[eqType]) == 0)
    {
        AUD_eqType = eqType;
        return 0;
    }
    return 0;
}

__inline int32_t
iteAudioSetReverbTable(
    ITE_AUDIO_REVBINFO *revbtable)
{
    LOG_ENTER "iteAudioSetReverbTable()\n" LOG_END

    memcpy((void *)&revbinfo, (const void *)revbtable, sizeof(ITE_AUDIO_REVBINFO));

    return 0;
}

#ifndef SHRINK_CODE_SIZE
int32_t
iteAudioSetVoiceRemove(
    bool enVoiceRemove)
{
    LOG_ENTER "iteAudioSetVoiceRemove()\n" LOG_END

    if (enVoiceRemove)
    {
        AUD_eqPrevStatus = AUD_eqStatus;
        AUD_prevEqType   = AUD_eqType;
        iteAudioSetEqualizer(1);
        iteAudioChangeEqualizer(0);
    }
    else
    {
        if (AUD_eqPrevStatus == 0)
            iteAudioSetEqualizer(0);
        else
            iteAudioChangeEqualizer(AUD_prevEqType);
    }
    AUD_SetEnVoiceRemove(enVoiceRemove);
    return 0;
}
#endif

#ifndef SHRINK_CODE_SIZE
bool
iteAudioIsMidiEnd(
    void)
{
    uint16_t regdatalow  = 0;
    uint16_t regdatahigh = 0;
    uint32_t pc;

    LOG_ENTER "iteAudioIsMidiEnd()\n" LOG_END

    //    ithReadRegH(RISC_PC, &regdatalow);
    //    ithReadRegH(RISC_PC + 2, &regdatahigh);

    pc = (regdatahigh << 16) + regdatalow;

    return ( pc < 0x1000 );
}
#endif

int32_t
iteAudioSetAttrib(
    const ITE_AUDIO_ATTRIB attrib,
    const void *value)
{
    int32_t  result = 0;
    uint16_t nTemp;
    LOG_ENTER "iteAudioSetAttrib()\n" LOG_END

    switch (attrib)
    {
    case ITE_AUDIO_STREAM_SAMPLERATE:
        AUD_decSampleRate = *((uint32_t *)(value));
        break;

    case ITE_AUDIO_STREAM_CHANNEL:
        AUD_decChannel = *((uint32_t *)(value));
        break;

    //#if defined(ENABLE_CODECS_PLUGIN)
    //        case ITE_AUDIO_PLUGIN_PATH:
    //            strcpy(codec_path, (char*)(value));
    //            break;
    //#endif

    case ITE_AUDIO_CODEC_SET_SAMPLE_RATE:
        if (*((uint32_t *)(value)) > 0)
        {
            AUD_nCodecSampleRate = *((uint32_t *)(value));
        }
        AUD_decSampleRate = AUD_nCodecSampleRate;
        break;

    case ITE_AUDIO_CODEC_SET_CHANNEL:
        if (*((uint32_t *)(value)) > 0)
        {
            AUD_nCodecChannels = *((uint32_t *)(value));
        }
        AUD_decChannel = AUD_nCodecChannels;
        break;

    case ITE_AUDIO_CODEC_SET_BUFFER_LENGTH:
        if (*((uint32_t *)(value)) > 0)
        {
            AUD_nCodecPCMBufferLength = *((uint32_t *)(value));
        }
        break;

    case ITE_AUDIO_MUSIC_PLAYER_ENABLE:
        if (*((uint32_t *)(value)) > 0)
        {
            AUD_bPlayMusic = *((uint32_t *)(value));
        }
        break;

    case ITE_AUDIO_PTS_SYNC_ENABLE:
        if (*((uint32_t *)(value)) > 0)
        {
            AUD_bPtsSync = *((uint32_t *)(value));
        }
        break;

    case ITE_AUDIO_ENGINE_ADDRESS:
        if ((uint8_t *)value > 0)
        {
            AUD_pEngineAddress = (uint8_t *)(value);
        }
        break;

    case ITE_AUDIO_ENGINE_LENGTH:
        if (*((uint32_t *)(value)) > 0)
        {
            AUD_nEngineLength = *((uint32_t *)(value));
        }
        break;

    case ITE_AUDIO_I2S_INIT:
        if (*((uint32_t *)(value)) >= 0)
        {
            AUD_bI2SInit = *((uint32_t *)(value));
        }
        break;

    case ITE_AUDIO_I2S_PTR:
        if ((uint8_t *)value > 0)
        {
            AUD_pI2Sptr = (uint8_t *)(value);
        }

        break;

    //        case ITE_AUDIO_FADE_IN:
    //            // fade in
    //            if (*((uint32_t*)(value))>0)
    //            {
    //                AUD_nFadeIn = *((uint32_t*)(value));

    //                //enableFadeInOut(AUD_nFadeIn);
    //            }
    //            break;

    case ITE_AUDIO_ENABLE_MPLAYER:
        if (*((uint32_t *)(value)) >= 0)
        {
            AUD_bMPlayer = *((uint32_t *)(value));
        }
        break;

    //        case ITE_AUDIO_ADJUST_MPLAYER_PTS:
    //            AUD_nMPlayerPTS= *((int32_t*)(value));
    //            break;

    case ITE_AUDIO_MPLAYER_STC_READY:
        AUD_bMPlayerSTCReady = *((uint32_t *)(value));
        break;

    case ITE_AUDIO_DECODE_ERROR:
        if (*((uint32_t *)(value)) == 0 || *((uint32_t *)(value)) == 1)
        {
            AUD_bDecodeError = *((uint32_t *)(value));
            nTemp            = AUD_bDecodeError;
            ithWriteRegH(AUDIO_DECODER_WRITE_DECODE_ERROR, nTemp);
        }
        break;

    case ITE_AUDIO_DECODE_DROP_DATA:
        if (*((uint32_t *)(value)) >= 0)
        {
            AUD_nDropData = *((uint32_t *)(value));
            nTemp         = AUD_nDropData;
            printf("[Audio Driver]set drop audio %d \n", nTemp);
            ithWriteRegH(AUDIO_DECODER_DROP_DATA, nTemp);
        }
        break;

    case ITE_AUDIO_PAUSE_STC_TIME:
        if (*((uint32_t *)(value)) >= 0)
        {
            AUD_nPauseSTCTime = *((uint32_t *)(value));
            //printf("[Audio Driver]set pause stc time %d \n",AUD_nPauseSTCTime);
        }
        break;

    case ITE_AUDIO_ENABLE_STC_SYNC:
        if (*((uint32_t *)(value)) >= 0)
        {
            AUD_bEnableSTC_Sync = *((uint32_t *)(value));
        }
        break;

    case ITE_AUDIO_SPDIF_NON_PCM:
        if (*((uint32_t *)(value)) >= 0)
        {
            AUD_bSPDIFNonPCM = *((uint32_t *)(value));
        }
        break;

    case ITE_AUDIO_FFMPEG_PAUSE_AUDIO:
        if (*((uint32_t *)(value)) >= 0)
        {
            AUD_nFfmpegPauseAudio = *((uint32_t *)(value));
            printf("Set  AUD_nFfmpegPauseAudio %d \n",AUD_nFfmpegPauseAudio);
        }
        break;



    default:
        result = AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;
        break;
    }

    return result;
}

int32_t
iteAudioGetAttrib(
    const ITE_AUDIO_ATTRIB attrib,
    void *value)
{
    int32_t  result = 0;
    uint32_t I2SBufLen;
    uint32_t I2SRdPtr;
    uint32_t I2SWrPtr;
    uint32_t I2SDataLength;
    uint16_t data = 0;

    LOG_ENTER "iteAudioSetAttrib()\n" LOG_END

    switch (attrib)
    {
    case ITE_AUDIO_GET_IS_EOF:
        //            *((uint32_t*)(value)) = isEOF();
        break;

    case ITE_AUDIO_CODEC_SET_SAMPLE_RATE:
        *((uint32_t *)(value)) = AUD_nCodecSampleRate;
        break;

    case ITE_AUDIO_CODEC_SET_CHANNEL:
        *((uint32_t *)(value)) = AUD_nCodecChannels;
        break;

    case ITE_AUDIO_MUSIC_PLAYER_ENABLE:
        *((uint32_t *)(value)) = AUD_bPlayMusic;
        break;

    case ITE_AUDIO_PTS_SYNC_ENABLE:
        *((uint32_t *)(value)) = AUD_bPtsSync;
        break;

    case ITE_AUDIO_CODEC_SET_BUFFER_LENGTH:
        *((uint32_t *)(value)) = AUD_nCodecPCMBufferLength;
        break;

    case ITE_AUDIO_ENGINE_ADDRESS:
        value = AUD_pEngineAddress;
        break;

    case ITE_AUDIO_ENGINE_LENGTH:
        *((uint32_t *)(value)) = AUD_nEngineLength;
        break;

    case ITE_AUDIO_I2S_PTR:
        *((uint32_t *)(value)) = AUD_pI2Sptr;
        break;

    case ITE_AUDIO_I2S_INIT:
        *((uint32_t *)(value)) = AUD_bI2SInit;
        break;

    case ITE_AUDIO_ENABLE_MPLAYER:
        *((uint32_t *)(value)) = AUD_bMPlayer;
        break;

    case ITE_AUDIO_MPLAYER_STC_READY:
        *((uint32_t *)(value)) = AUD_bMPlayerSTCReady;
        break;

    case ITE_AUDIO_DECODE_ERROR:
        AUD_bDecodeError = 0;
        data             = ithReadRegH(AUDIO_DECODER_WRITE_DECODE_ERROR);
        if (data > 0)
        {
            AUD_bDecodeError = 1;
        }
        *((uint32_t *)(value)) = AUD_bDecodeError;
        break;

    case ITE_AUDIO_I2S_OUT_FULL:
        AUD_bI2SOutFull = 0;
#if 0
        data            = ithReadRegH(MMIO_I2S_OUT_BUF_LEN);
        I2SBufLen       = (uint32_t)data;
        data            = ithReadRegH(MMIO_I2S_OUT_BUF_RDPTR);
        I2SRdPtr        = (uint32_t)data;
        data            = ithReadRegH(MMIO_I2S_OUT_BUF_WRPTR);
        I2SWrPtr        = (uint32_t)data;
        I2SDataLength   = (I2SRdPtr > I2SWrPtr) ? (I2SBufLen - (I2SRdPtr - I2SWrPtr)) : (I2SWrPtr - I2SRdPtr);

        if (I2SDataLength + (I2SBufLen / 2) > I2SBufLen)
        {
            AUD_bI2SOutFull = 1;
            //printf("[Audio Driver] i2s data %d buf length %d \n",I2SDataLength,I2SBufLen);
        }
#endif
        *((uint32_t *)(value)) = AUD_bI2SOutFull;
        break;

    case ITE_AUDIO_DECODE_DROP_DATA:
        AUD_nDropData = 0;
        data          = ithReadRegH(AUDIO_DECODER_DROP_DATA);
        if (data > 0)
        {
            AUD_nDropData = data;
        }
        //printf("[Audio Driver]get AUD_nDropData %d \n",AUD_nDropData);
        *((uint32_t *)(value)) = AUD_nDropData;
        break;

    case ITE_AUDIO_PAUSE_STC_TIME:
        *((uint32_t *)(value)) = AUD_nPauseSTCTime;
        break;

    case ITE_AUDIO_IS_PAUSE:
        AUD_bIsPause           = 0;
        data                   = ithReadRegH(DrvAudioCtrl);
        AUD_bIsPause           = (data & DrvDecode_PAUSE) != 0;
        *((uint32_t *)(value)) = AUD_bIsPause;
        break;

    case ITE_AUDIO_ENABLE_STC_SYNC:
        *((uint32_t *)(value)) = AUD_bEnableSTC_Sync;
        break;

    case ITE_AUDIO_DIRVER_DECODE_BUFFER_LENGTH:
        *((uint32_t *)(value)) = Audio_Decode_Buflen;
        break;

    case ITE_AUDIO_SPDIF_NON_PCM:
        *((uint32_t *)(value)) = AUD_bSPDIFNonPCM;
        break;

    case ITE_AUDIO_FFMPEG_PAUSE_AUDIO:
        *((uint32_t *)(value)) = AUD_nFfmpegPauseAudio;
        break;


    default:
        result = AUDIO_ERROR_CODEC_UNKNOW_CODEC_MODE;
        break;
    }
    return result;
}

int32_t
iteAudioGetDecodeTimeV2(
    uint32_t *time)
{
#if 1
    uint16_t data = 0;
    uint32_t I2SBufLen;
    uint32_t I2SRdPtr;
    uint32_t I2SWrPtr;
    uint32_t I2SDataLength;
    uint32_t decTime = 0;
    uint32_t offset  = 0;
    int32_t  result  = 0;
    uint16_t nTemp, timeout = 0;

    //LOG_ENTER "iteAudioGetDecodeTimeV2()\n" LOG_END

    if (AUD_SKIP)
    {
    #ifdef AUDIO_FREERTOS
        *time = PalGetDuration(lastTime);
    #endif
        return 0;
    }
    #ifndef CFG_RISC_TS_DEMUX_PLUGIN
    if (AUD_decChannel == 0 || AUD_decSampleRate == 0)
    {
        printf("[Audio Driver]iteAudioGetDecodeTimeV2  AUD_decChannel %d AUD_decSampleRate%d \n", AUD_decChannel, AUD_decSampleRate);
        result = AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;
    }
    else
    {
WAIT:
        timeout = 0;
        do
        {
            nTemp = iteAudioGetAuidoProcessorWritingStatus();
            if (nTemp > 0)
            {
                printf("[Audio Driver]wait %d \n", timeout);
                ithDelay(1);
                timeout++;
            }
        } while (nTemp > 0 && timeout < 5);

        data      = ithReadRegH(DrvDecode_Frame + 2);
        decTime   = (uint32_t)(data << 16);

        data      = ithReadRegH(DrvDecode_Frame);
        decTime  += (uint32_t)data;

        //data = ithReadRegH(0x1654);
        I2SBufLen = AUD_nCodecPCMBufferLength; //(uint32_t)data;
        //data = ithReadRegH(0x165C);
        I2SRdPtr  = I2S_DA32_GET_RP();         //(uint32_t)data;
        nTemp     = iteAudioGetAuidoProcessorWritingStatus();
        // need to re-read register
        if (nTemp > 0)
        {
            printf("[Audio Driver]wait write \n");
            goto WAIT;
        }
        //data = ithReadRegH(0x1656);

        I2SWrPtr      = I2S_DA32_GET_WP(); //(uint32_t)data;

        I2SDataLength = (I2SRdPtr > I2SWrPtr) ? (I2SBufLen - (I2SRdPtr - I2SWrPtr)) : (I2SWrPtr - I2SRdPtr);
        I2SDataLength = (AUD_decChannel == 2) ? (I2SDataLength >> 2) : (I2SDataLength >> 1);
        I2SDataLength = ((I2SDataLength << 16) / AUD_decSampleRate);

        if (gLastDecTime)
        {
            // time increment wrap around
            if (decTime < gLastDecTime
                && gLastDecTime - decTime >= AUDIO_WRAP_AROUND_JUDGE_RANGE)
            {
                gPtsTimeBaseOffset += AUDIO_WRAP_AROUND_THRESHOLD;
            }
            else if ((decTime - gLastDecTime) >= AUDIO_WRAP_AROUND_JUDGE_RANGE) // time decrement wrap around
            {
                if (gPtsTimeBaseOffset)
                    gPtsTimeBaseOffset -= AUDIO_WRAP_AROUND_THRESHOLD;
            }
        }

        gLastDecTime = decTime;
        offset       = gPtsTimeBaseOffset;

        if (decTime > I2SDataLength)
        {
            decTime -= I2SDataLength;
        }
        else // decTime <= I2SDataLength
        {
            if (offset)
            {
                offset -= (((((I2SDataLength - decTime) & 0xffff) * 1000) >> 16)
                           + (((I2SDataLength - decTime) >> 16) * 1000));
            }
            else
                decTime = I2SDataLength;
        }

        (*time) = ((((decTime) & 0xffff) * 1000) >> 16) + (((decTime) >> 16) * 1000) + offset;
    }
    #endif
    return result;

#else
    uint16_t data = 0;
    uint32_t I2SBufLen;
    uint32_t I2SRdPtr;
    uint32_t I2SWrPtr;
    uint32_t I2SDataLength;
    //uint32_t decTime = 0;
    int32_t  result = 0;

    //LOG_ENTER "iteAudioGetDecodeTimeV2()\n" LOG_END

    if (AUD_SKIP)
    {
        *time = PalGetDuration(lastTime);
        return 0;
    }

    if (AUD_decChannel == 0 || AUD_decSampleRate == 0)
    {
        result = int32_t_ERROR;
    }
    else
    {
        data          = ithReadRegH(DrvDecode_Frame + 2);
        (*time)       = (uint32_t)(data << 16);

        data          = ithReadRegH(DrvDecode_Frame);
        (*time)      += (uint32_t)data;

        data          = ithReadRegH(0x1654);
        I2SBufLen     = (uint32_t)data;

        data          = ithReadRegH(0x165C);
        I2SRdPtr      = (uint32_t)data;
        data          = ithReadRegH(0x1656);
        I2SWrPtr      = (uint32_t)data;

        I2SDataLength = (I2SRdPtr > I2SWrPtr) ? (I2SBufLen - (I2SRdPtr - I2SWrPtr)) : (I2SWrPtr - I2SRdPtr);
        I2SDataLength = (AUD_decChannel == 2) ? (I2SDataLength >> 2) : (I2SDataLength >> 1);
        I2SDataLength = ((I2SDataLength << 16) / AUD_decSampleRate);

        if ((*time) > I2SDataLength)
            (*time) -= I2SDataLength;
        else
            (*time) = I2SDataLength;

        (*time) = ((((*time) & 0xffff) * 1000) >> 16) + (((*time) >> 16) * 1000);
    }

    return result;
#endif
}

int32_t
iteAudioGetDecodeTime(
    uint32_t *time)
{
    uint16_t data = 0;

    //LOG_ENTER "iteAudioGetDecodeTime()\n" LOG_END
    if (AUD_SKIP)
    {
#ifdef AUDIO_FREERTOS
        *time = PalGetDuration(lastTime);
#endif
        return 0;
    }

    if (AUD_EngineType == ITE_AMR_ENCODE)
        return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;

    data     = ithReadRegH(DrvDecode_Frame + 2);
    (*time)  = (uint32_t)data * 1000;

    data     = ithReadRegH(DrvDecode_Frame);
    (*time) += ((uint32_t)(data * 1000) >> 16);

    return 0;
}

int32_t
iteAudioGetEncodeTime(
    uint32_t *time)
{
    uint16_t data = 0;

    //LOG_ENTER "iteAudioGetEncodeTime()\n" LOG_END

    if (AUD_SKIP)
    {
#ifdef AUDIO_FREERTOS
        *time = PalGetDuration(lastTime);
#endif
        return 0;
    }

    if (  (AUD_EngineType != ITE_AMR_ENCODE) & (AUD_EngineType != ITE_AMR_CODEC) && (AUD_EngineType != ITE_WAV_DECODE )  )
        return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;

    data     = ithReadRegH(DrvEncode_Frame + 2);
    (*time)  = (uint32_t)data * 1000;

    data     = ithReadRegH(DrvEncode_Frame);
    (*time) += ((uint32_t)(data * 1000) >> 16);

    return 0;
}

int32_t
iteAudioEndStream(
    void)
{
    LOG_ENTER "iteAudioEndStream()\n" LOG_END

    if (AUD_SKIP)
        return 0;

    if (AUD_SetOutEOF() != 0)
    {
        iteAudioStop();  // if codec timeout ,set stop bit to return clearbuffer
        return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;
    }
    //nInputCount=0;
    //stopMusicMute();

    //AUD_decChannel = 0;
    //AUD_decSampleRate = 0;
    Audio_Decode_Wrptr  = 0;
    Audio_Decode_Wrptr1 = 0;
    Audio_Decode_Wrptr2 = 0;
    //Audio_Decode_Wrptr3 = 0;
    gPtsTimeBaseOffset  = 0;
    gLastDecTime        = 0;

    return 0;
}

int32_t
iteAudioPause(
    bool enable)
{
    LOG_ENTER "iteAudioPause(%d)\n", enable LOG_END

    if (AUD_SKIP)
        return 0;

    if (AUD_bEnableSTC_Sync == 1)
    {
        // mplayer using iteAudioSTCSyncPause()
        if (!AUD_bMPlayer)
        {
            if (enable == 1)
            {
                ithWriteRegMaskH(DrvAudioCtrl, 1, DrvDecode_PAUSE);
            }
            else
            {
                ithWriteRegMaskH(DrvAudioCtrl, 0, DrvDecode_PAUSE);
            }
        }
    }
    else
    {
        if (enable == 1)
        {
            ithWriteRegMaskH(DrvAudioCtrl, 1, DrvDecode_PAUSE);
        }
        else
        {
            ithWriteRegMaskH(DrvAudioCtrl, 0, DrvDecode_PAUSE);
        }
    }
    return 0;
}

int32_t
iteAudioSTCSyncPause(
    bool enable)
{
    ithWriteRegMaskH(DrvAudioCtrl, !!enable, DrvDecode_PAUSE);
}

int32_t
iteAudioSeek(
    bool enable)
{
    return 0;
}

int32_t
iteAudioStop(
    void)
{
    uint16_t regData = 0;
    uint32_t timeout = 3000;
    uint32_t nTmp    = 0;
    uint32_t nTemp   = 0;

#if defined(AUDIO_DRIVER_SHOW_WRITE_BITRATE)
    gAudioWrite         = 0;
    gAudioWriteDuration = 0;
#endif

    LOG_ENTER "iteAudioStop()\n" LOG_END

#if defined(OUTPUT_TO_FILE)
    if (fout)
    {
        printf("[Audio Driver] PalFileClose\n");
        //PalFileClose(fout, NULL);
        fclose(fout);
        fout = NULL;
    }
#endif
    //nInputCount=0;
#ifdef AUDIO_FREERTOS
    lastTime = PalGetClock();
#endif
    if (AUD_SKIP)
        return 0;

    iteAudioSetAttrib(ITE_AUDIO_MPLAYER_STC_READY, (void *)&nTemp);
    //    iteAudioSetAttrib(ITE_AUDIO_ADJUST_MPLAYER_PTS, (void*)&nTemp);
    iteAudioSetAttrib(ITE_AUDIO_DECODE_DROP_DATA, (void *)&nTemp);
    iteAudioSetAttrib(ITE_AUDIO_PAUSE_STC_TIME, (void *)&nTmp);

    ithWriteRegMaskH(DrvAudioCtrl, (1 << 6), DrvDecode_STOP);
    ithWriteRegH(MMIO_PTS_WRIDX, 0);
    ithWriteRegH(MMIO_PTS_HI, 0);
    ithWriteRegH(MMIO_PTS_LO, 0);
    do
    {
        regData = ithReadRegH(DrvAudioCtrl);
        if (!(regData & DrvDecode_STOP))
        {
            Audio_Decode_Wrptr  = 0;
            Audio_Decode_Wrptr1 = 0;
            Audio_Decode_Wrptr2 = 0;
            //Audio_Decode_Wrptr3 = 0;
            gPtsTimeBaseOffset  = 0;
            gLastDecTime        = 0;
#ifndef CFG_RISC_TS_DEMUX_PLUGIN
            i2s_deinit_DAC();
#endif
            return 0;
        }
        usleep(1000);
        timeout--;
        if (timeout == 0)
        {
            ithWriteRegMaskH(DrvAudioCtrl, (0 << 6), DrvDecode_STOP);
#ifndef CFG_RISC_TS_DEMUX_PLUGIN
            i2s_deinit_DAC();
#endif
            printf("[Audio Driver] Stop ,Can not correctly set end of stream, reset engine\n");
            return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;
        }
    } while (1);
}

int32_t
iteAudioStopQuick(
    void)
{
    uint16_t regData = 0;

    uint32_t timeout = 200;
    //uint32_t nTmp=0;
    //uint32_t nTemp=0;

#if defined(OUTPUT_TO_FILE)
    if (fout)
    {
        printf("[Audio Driver] PalFileClose\n");
        //PalFileClose(fout, NULL);
        fclose(fout);
        fout = NULL;
    }
#endif
    //nInputCount=0;

    if (AUD_SKIP)
        return 0;

    ithWriteRegMaskH(DrvAudioCtrl, (1 << 6), DrvDecode_STOP);
    ithWriteRegH(MMIO_PTS_WRIDX, 0);
    ithWriteRegH(MMIO_PTS_HI, 0);
    ithWriteRegH(MMIO_PTS_LO, 0);

    do
    {
        regData = ithReadRegH(DrvAudioCtrl);
        if (!(regData & DrvDecode_STOP))
        {
            Audio_Decode_Wrptr = 0;
            return 0;
        }
        usleep(1000);
        timeout--;
        if (timeout == 0)
        {
            ithWriteRegMaskH(DrvAudioCtrl, (0 << 6), DrvDecode_STOP);
            printf("[Audio Driver] Stop ,Can not correctly set end of stream, reset engine\n");
            return AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;
        }
    } while (1);
}

int32_t
iteAudioPowerOnAmplifier(void)
{
    /*GPIO4 select DGPIO4*/
    ithWriteRegMaskA(0x68000048, 0x0004, 0x0004);
    ithDelay(10);
    /*GPIO4 Output data*/
    ithWriteRegMaskA(0x68000000, 0x0000, 0x0010);
    ithDelay(10);
    /*GPIO4 Output mode*/
    ithWriteRegMaskA(0x68000008, 0x0010, 0x0010);
    ithDelay(10);

    return 0;
}

int32_t
iteAudioPowerOffAmplifier(
    void)
{
    /*GPIO4 select HOST_WAIT*/
    ithWriteRegMaskA(0x68000048, 0x0000, 0x0004);
    ithDelay(10);

    return 0;
}

int32_t
iteAudioGetPlayPos(
    int32_t *pos)
{
    *pos = AUD_curPlayPos;
    return 0;
}

int32_t
iteAudioSetPlayPos(
    int32_t pos)
{
    AUD_curPlayPos = pos;
    return 0;
}

int32_t
iteAudioSetWaveDecodeHeader(ITE_WaveInfo wavInfo)
{
#if HAVE_WAV
    uint32_t nSamplesPerSec = 44100;
    uint16_t wFormatTag;
    uint16_t i;
    uint8_t  *ref;

    LOG_ENTER "iteAudioSetWaveDecodeHeader()\n" LOG_END
    AUD_decSampleRate = wavInfo.sampRate;
    if (wavInfo.nChans > 2 && wavInfo.nChans <= 6)
    {
        printf("[Audio Driver] set wav channels %d \n", wavInfo.nChans);
        AUD_decChannel = 2;
    }
    else if (wavInfo.nChans <= 2)
    {
        AUD_decChannel = wavInfo.nChans;
    }

    // set decode header
    for (i = 0; i < sizeof(wave_header); i++)
    {
        wave_header[i] = 0;
    }

    //ckID
    wave_header[0]  = 'R';
    wave_header[1]  = 'I';
    wave_header[2]  = 'F';
    wave_header[3]  = 'F';
    //cksize
    wave_header[4]  = 0x24;
    wave_header[5]  = 0x60;
    wave_header[6]  = 0x01;
    wave_header[7]  = 0x00;
    //WAVEID
    wave_header[8]  = 'W';
    wave_header[9]  = 'A';
    wave_header[10] = 'V';
    wave_header[11] = 'E';
    //ckID
    wave_header[12] = 'f';
    wave_header[13] = 'm';
    wave_header[14] = 't';
    wave_header[15] = ' ';
    //cksize ADPCM is 20, others is 16
    if (wavInfo.format == 3)
    {
        wave_header[16] = 0x14;
        wave_ADPCM      = 4;
        ref             = wavInfo.pvData;
        wave_header[36] = 0x02;
        wave_header[37] = 0;
        wave_header[38] = ref[0];
        wave_header[39] = ref[1];
    }
    else
    {
        wave_header[16] = 0x10;
    }
    wave_header[17] = 0;
    wave_header[18] = 0;
    wave_header[19] = 0;

    //wFormatTag
    // set format
    switch (wavInfo.format)
    {
    case ITE_WAVE_FORMAT_PCM:
        wFormatTag = 1;
        if (wavInfo.bitsPerSample == 8)
        {
            wave_header[34] = 0x08;
        }
        else
        {
            wave_header[34] = 0x10;
        }
        break;

    case ITE_WAVE_FORMAT_ALAW:
        wFormatTag      = 6;
        wave_header[34] = 0x08;
        break;

    case ITE_WAVE_FORMAT_MULAW:
        wFormatTag      = 7;
        wave_header[34] = 0x08;
        break;

    case ITE_WAVE_FORMAT_DVI_ADPCM:
        wFormatTag      = 0x11;
        wave_header[34] = 0x04;
        break;

    case ITE_WAVE_FORMAT_SWF_ADPCM:
        // 0xFE is ITE defined
        wFormatTag      = 0xFE;
        // block align is frame length
        wave_header[32] = (uint8_t)((wavInfo.nDataSize >> 24) & 0xff);
        wave_header[33] = (uint8_t)((wavInfo.nDataSize >> 16) & 0xff);;
        break;

    default:
        wFormatTag      = 0;
        wave_header[34] = 0x10;
        break;
    }

    wave_header[21] = (uint8_t) ((wFormatTag >> 8) & 0xff);
    wave_header[20] = (uint8_t) ((wFormatTag) & 0xff);

    if (wavInfo.nChans <= 6)
    {
        wave_header[23] = 0;
        wave_header[22] = wavInfo.nChans;
    }
    else
    {
        wave_header[23] = 0;
        wave_header[22] = 0x01;
        printf("[Audio Driver] set wave header channels %d > 6\n", wavInfo.nChans);
    }

    if (wavInfo.sampRate <= 96000)
    {
        nSamplesPerSec = wavInfo.sampRate;
    }

    wave_header[27]              = (uint8_t) ((nSamplesPerSec >> 24) & 0xff);
    wave_header[26]              = (uint8_t) ((nSamplesPerSec >> 16) & 0xff);
    wave_header[25]              = (uint8_t) ((nSamplesPerSec >> 8) & 0xff);
    wave_header[24]              = (uint8_t) ((nSamplesPerSec) & 0xff);

    wave_header[36 + wave_ADPCM] = 'd';
    wave_header[37 + wave_ADPCM] = 'a';
    wave_header[38 + wave_ADPCM] = 't';
    wave_header[39 + wave_ADPCM] = 'a';

    wave_header[40 + wave_ADPCM] = 0;
    wave_header[41 + wave_ADPCM] = 0;
    wave_header[42 + wave_ADPCM] = 0;
    wave_header[43 + wave_ADPCM] = 0;

    //write_waveHeader = 1;
#endif
    return 0;
}

int32_t
iteAudioGenWaveDecodeHeader(ITE_WaveInfo *wavInfo, uint8_t *wave_header)
{
#if HAVE_WAV
    uint32_t nSamplesPerSec = 44100;
    uint16_t wFormatTag;
    uint16_t i;
    uint8_t  *ref;

    LOG_ENTER "iteAudioSetWaveDecodeHeader()\n" LOG_END
    AUD_decSampleRate = wavInfo->sampRate;
    if (wavInfo->nChans > 2 && wavInfo->nChans <= 6)
    {
        printf("[Audio Driver] set wav channels %d \n", wavInfo->nChans);
        AUD_decChannel = 2;
    }
    else if (wavInfo->nChans <= 2)
    {
        AUD_decChannel = wavInfo->nChans;
    }

    // set decode header
    for (i = 0; i < 48; i++)
    {
        wave_header[i] = 0;
    }

    //ckID
    wave_header[0]  = 'R';
    wave_header[1]  = 'I';
    wave_header[2]  = 'F';
    wave_header[3]  = 'F';
    //cksize
    wave_header[4]  = 0x24;
    wave_header[5]  = 0x60;
    wave_header[6]  = 0x01;
    wave_header[7]  = 0x00;
    //WAVEID
    wave_header[8]  = 'W';
    wave_header[9]  = 'A';
    wave_header[10] = 'V';
    wave_header[11] = 'E';
    //ckID
    wave_header[12] = 'f';
    wave_header[13] = 'm';
    wave_header[14] = 't';
    wave_header[15] = ' ';
    //cksize ADPCM is 20, others is 16
    if (wavInfo->format == 3)
    {
        wave_header[16] = 0x14;
        wave_ADPCM      = 4;
        ref             = wavInfo->pvData;
        wave_header[36] = 0x02;
        wave_header[37] = 0;
        wave_header[38] = ref[0];
        wave_header[39] = ref[1];
    }
    else
    {
        wave_header[16] = 0x10;
    }
    wave_header[17] = 0;
    wave_header[18] = 0;
    wave_header[19] = 0;

    //wFormatTag
    // set format
    switch (wavInfo->format)
    {
    case ITE_WAVE_FORMAT_PCM:
        wFormatTag = 1;
        if (wavInfo->bitsPerSample == 8)
        {
            wave_header[34] = 0x08;
        }
        else
        {
            wave_header[34] = 0x10;
        }
        break;

    case ITE_WAVE_FORMAT_ALAW:
        wFormatTag      = 6;
        wave_header[34] = 0x08;
        break;

    case ITE_WAVE_FORMAT_MULAW:
        wFormatTag      = 7;
        wave_header[34] = 0x08;
        break;

    case ITE_WAVE_FORMAT_DVI_ADPCM:
        wFormatTag      = 0x11;
        wave_header[34] = 0x04;
        break;

    case ITE_WAVE_FORMAT_SWF_ADPCM:
        // 0xFE is ITE defined
        wFormatTag      = 0xFE;
        // block align is frame length
        wave_header[33] = (uint8_t)((wavInfo->nDataSize >> 8) & 0xff);
        wave_header[32] = (uint8_t)((wavInfo->nDataSize) & 0xff);
        wave_header[34] = 0x10;
        break;

    default:
        wFormatTag      = 0;
        wave_header[34] = 0x10;
        break;
    }

    wave_header[21] = (uint8_t) ((wFormatTag >> 8) & 0xff);
    wave_header[20] = (uint8_t) ((wFormatTag) & 0xff);

    if (wavInfo->nChans <= 6)
    {
        wave_header[23] = 0;
        wave_header[22] = wavInfo->nChans;
    }
    else
    {
        wave_header[23] = 0;
        wave_header[22] = 0x01;
        printf("[Audio Driver] set wave header channels %d > 6\n", wavInfo->nChans);
    }

    if (wavInfo->sampRate <= 96000)
    {
        nSamplesPerSec = wavInfo->sampRate;
    }

    wave_header[27]              = (uint8_t) ((nSamplesPerSec >> 24) & 0xff);
    wave_header[26]              = (uint8_t) ((nSamplesPerSec >> 16) & 0xff);
    wave_header[25]              = (uint8_t) ((nSamplesPerSec >> 8) & 0xff);
    wave_header[24]              = (uint8_t) ((nSamplesPerSec) & 0xff);

    wave_header[36 + wave_ADPCM] = 'd';
    wave_header[37 + wave_ADPCM] = 'a';
    wave_header[38 + wave_ADPCM] = 't';
    wave_header[39 + wave_ADPCM] = 'a';

    wave_header[40 + wave_ADPCM] = 0;
    wave_header[41 + wave_ADPCM] = 0;
    wave_header[42 + wave_ADPCM] = 0;
    wave_header[43 + wave_ADPCM] = 0;
#endif

    return 0;
}

int32_t
iteAudioSetWmaInfo(ITE_WmaInfo tWmaInfo)
{
#if HAVE_WMA
    uint16_t i;

    LOG_ENTER "iteAudioSetWmaInfo()\n" LOG_END
    AUD_decSampleRate = tWmaInfo.sampleRate;
    AUD_decChannel    = tWmaInfo.channels;
    for (i = 0; i < sizeof(wma_info); i++)
    {
        wma_info[i] = 0;
    }
    printf("[Audio Driver] packet size %d audio stream %d codec id %d channels %d rate %d bitrate %d blockalign %d bitspersample %d datalen %d 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
           tWmaInfo.packet_size, tWmaInfo.audiostream, tWmaInfo.codec_id, tWmaInfo.channels, tWmaInfo.sampleRate, tWmaInfo.bitrate, tWmaInfo.blockalign, tWmaInfo.bitspersample, tWmaInfo.datalen, tWmaInfo.data[0], tWmaInfo.data[1], tWmaInfo.data[2], tWmaInfo.data[3], tWmaInfo.data[4], tWmaInfo.data[5]);

    // packet_size;
    wma_info[3] = (uint8_t) ((tWmaInfo.packet_size >> 24) & 0xff);
    wma_info[2] = (uint8_t) ((tWmaInfo.packet_size >> 16) & 0xff);
    wma_info[1] = (uint8_t) ((tWmaInfo.packet_size >> 8) & 0xff);
    wma_info[0] = (uint8_t) ((tWmaInfo.packet_size) & 0xff);

    // audiostream wmaInfo[4~7], not use

    // codec_id;
    wma_info[9]   = (uint8_t) ((tWmaInfo.codec_id >> 8) & 0xff);
    wma_info[8]   = (uint8_t) ((tWmaInfo.codec_id) & 0xff);

    // channels;
    wma_info[11]  = (uint8_t) ((tWmaInfo.channels >> 8) & 0xff);
    wma_info[10]  = (uint8_t) ((tWmaInfo.channels) & 0xff);

    // sampleRate;
    wma_info[15]  = (uint8_t) ((tWmaInfo.sampleRate >> 24) & 0xff);
    wma_info[14]  = (uint8_t) ((tWmaInfo.sampleRate >> 16) & 0xff);
    wma_info[13]  = (uint8_t) ((tWmaInfo.sampleRate >> 8) & 0xff);
    wma_info[12]  = (uint8_t) ((tWmaInfo.sampleRate) & 0xff);

    // bitrate;
    wma_info[19]  = (uint8_t) ((tWmaInfo.bitrate >> 24) & 0xff);
    wma_info[18]  = (uint8_t) ((tWmaInfo.bitrate >> 16) & 0xff);
    wma_info[17]  = (uint8_t) ((tWmaInfo.bitrate >> 8) & 0xff);
    wma_info[16]  = (uint8_t) ((tWmaInfo.bitrate) & 0xff);

    // blockalign;
    wma_info[21]  = (uint8_t) ((tWmaInfo.blockalign >> 8) & 0xff);
    wma_info[20]  = (uint8_t) ((tWmaInfo.blockalign) & 0xff);

    //bitspersample;
    wma_info[23]  = (uint8_t) ((tWmaInfo.bitspersample >> 8) & 0xff);
    wma_info[22]  = (uint8_t) ((tWmaInfo.bitspersample) & 0xff);

    //uint16_t datalen;
    wma_info[25]  = (uint8_t) ((tWmaInfo.datalen >> 8) & 0xff);
    wma_info[24]  = (uint8_t) ((tWmaInfo.datalen) & 0xff);

    //uint8_t  data[6];
    wma_info[26]  = (uint8_t) tWmaInfo.data[0];
    wma_info[27]  = (uint8_t) tWmaInfo.data[1];
    wma_info[28]  = (uint8_t) tWmaInfo.data[2];
    wma_info[29]  = (uint8_t) tWmaInfo.data[3];
    wma_info[30]  = (uint8_t) tWmaInfo.data[4];
    wma_info[31]  = (uint8_t) tWmaInfo.data[5];

    write_wmaInfo = 1;
#endif

    return 0;
}

int32_t
iteAudioSaveWmaInfo(ITE_WmaInfo tWmaInfo)
{
    uint16_t i;

    LOG_ENTER "iteAudioSaveWmaInfo()\n" LOG_END
    AUD_decSampleRate = tWmaInfo.sampleRate;
    AUD_decChannel    = tWmaInfo.channels;
    for (i = 0; i < sizeof(wma_info); i++)
    {
        wma_info[i] = 0;
    }

#if 0
    // packet_size;
    wma_info[3] = (uint8_t) ((tWmaInfo.packet_size >> 24) & 0xff);
    wma_info[2] = (uint8_t) ((tWmaInfo.packet_size >> 16) & 0xff);
    wma_info[1] = (uint8_t) ((tWmaInfo.packet_size >> 8) & 0xff);
    wma_info[0] = (uint8_t) ((tWmaInfo.packet_size) & 0xff);

    // audiostream wmaInfo[4~7], not use

    // codec_id;
    wma_info[9]  = (uint8_t) ((tWmaInfo.codec_id >> 8) & 0xff);
    wma_info[8]  = (uint8_t) ((tWmaInfo.codec_id) & 0xff);

    // channels;
    wma_info[11] = (uint8_t) ((tWmaInfo.channels >> 8) & 0xff);
    wma_info[10] = (uint8_t) ((tWmaInfo.channels) & 0xff);

    // sampleRate;
    wma_info[15] = (uint8_t) ((tWmaInfo.sampleRate >> 24) & 0xff);
    wma_info[14] = (uint8_t) ((tWmaInfo.sampleRate >> 16) & 0xff);
    wma_info[13] = (uint8_t) ((tWmaInfo.sampleRate >> 8) & 0xff);
    wma_info[12] = (uint8_t) ((tWmaInfo.sampleRate) & 0xff);

    // bitrate;
    wma_info[19] = (uint8_t) ((tWmaInfo.bitrate >> 24) & 0xff);
    wma_info[18] = (uint8_t) ((tWmaInfo.bitrate >> 16) & 0xff);
    wma_info[17] = (uint8_t) ((tWmaInfo.bitrate >> 8) & 0xff);
    wma_info[16] = (uint8_t) ((tWmaInfo.bitrate) & 0xff);

    // blockalign;
    wma_info[21] = (uint8_t) ((tWmaInfo.blockalign >> 8) & 0xff);
    wma_info[20] = (uint8_t) ((tWmaInfo.blockalign) & 0xff);

    //bitspersample;
    wma_info[23] = (uint8_t) ((tWmaInfo.bitspersample >> 8) & 0xff);
    wma_info[22] = (uint8_t) ((tWmaInfo.bitspersample) & 0xff);

    //uint16_t datalen;
    wma_info[25] = (uint8_t) ((tWmaInfo.datalen >> 8) & 0xff);
    wma_info[24] = (uint8_t) ((tWmaInfo.datalen) & 0xff);

    //uint8_t  data[6];
    wma_info[26] = (uint8_t) tWmaInfo.data[0];
    wma_info[27] = (uint8_t) tWmaInfo.data[1];
    wma_info[28] = (uint8_t) tWmaInfo.data[2];
    wma_info[29] = (uint8_t) tWmaInfo.data[3];
    wma_info[30] = (uint8_t) tWmaInfo.data[4];
    wma_info[31] = (uint8_t) tWmaInfo.data[5];
#endif

    memcpy(&wma_info, &tWmaInfo, sizeof(wma_info));
    printf("[Audio Driver] packet size %d audio stream %d codec id %d channels %d rate %d bitrate %d blockalign %d bitspersample %d datalen %d 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
           tWmaInfo.packet_size, tWmaInfo.audiostream, tWmaInfo.codec_id, tWmaInfo.channels, tWmaInfo.sampleRate, tWmaInfo.bitrate, tWmaInfo.blockalign, tWmaInfo.bitspersample, tWmaInfo.datalen, tWmaInfo.data[0], tWmaInfo.data[1], tWmaInfo.data[2], tWmaInfo.data[3], tWmaInfo.data[4], tWmaInfo.data[5]);

    return 0;
}

int32_t
iteAudioWriteWmaInfo()
{
    write_wmaInfo = 1;
    return 0;
}

int32_t
iteAudioWriteWavInfo()
{
    write_waveHeader = 1;
    return 0;
}

int32_t
iteAudioSetPts(
    uint32_t pts)
{
    int32_t  result  = AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE;
    uint16_t swpr    = 0;
    uint16_t pts_low = 0;
    uint16_t pts_hi  = 0;
    bool     bPtsSync;

    swpr    = ithReadRegH(0x16AE);
    pts_low = ithReadRegH(0x16B0);
    pts_hi  = ithReadRegH(0x16B2);

    if (swpr == 0 && pts_low == 0 && pts_hi == 0)
    {
        uint16_t regdata;
        regdata = ithReadRegH(DrvDecode_WrPtr);
        ithWriteRegH(0x16AE, regdata);
        ithWriteRegH(0x16B0, (uint16_t)(pts & 0xFFFFL));
        ithWriteRegH(0x16B2, (uint16_t)(pts >> 16));
        result = 0;
    }
    bPtsSync = 1;
    iteAudioSetAttrib(ITE_AUDIO_PTS_SYNC_ENABLE, &bPtsSync);

    return result;
}

void
iteAudioSetEarPhonePlugin(bool detect)
{
    g_EarPhoneDetect = detect;
}

bool
iteAudioGetEarPhonePlugin(void)
{
    return g_EarPhoneDetect;
}

int
iteAudioGetOutputEmpty()
{
    uint16_t regdata;
    regdata = ithReadRegH(DrvAudioCtrl);
    return ((regdata >> DrvDecode_EnOutputEmpty_Bits) & 1);
}

// set 1 ,I2S output will be empty , mp3
void
iteAudioSetOutputEmpty(
    int32_t nEnable)
{
    ithWriteRegMaskH(DrvAudioCtrl, (!!nEnable) << DrvDecode_EnOutputEmpty_Bits, DrvDecode_EnOutputEmpty);
}

void
iteAudioSetWmaWithoutASFHeader(
    int32_t nEnable)
{
    ithWriteRegMaskH(DrvAudioCtrl2, (!!nEnable) << DrvWMAWithoutASFHeader_Bits, DrvWMAWithoutASFHeader);
}

void
iteAudioSetMusicShowSpectrum (
    int32_t nEnable)
{
    ithWriteRegMaskH(DrvAudioCtrl2, (!!nEnable) << DrvShowSpectrumMode_Bits, DrvShowSpectrum);
}

int32_t
iteAudioGetMp3RdBufPointer(void)
{
    uint16_t regdata;
    regdata = ithReadRegH(DrvAudioCtrl2);
    return ((regdata >> DrvResetMp3RdBufPointer_Bits) & 1);
}


void
iteAudioSetMp3RdBufPointer (
    int32_t nReset)
{
    ithWriteRegMaskH(DrvAudioCtrl2, (!!nReset) << DrvResetMp3RdBufPointer_Bits, DrvResetMp3RdBufPointer);
}


void
iteAudioSetMusicWithoutASFHeader(
    int32_t nEnable)
{
    ithWriteRegMaskH(DrvAudioCtrl2, (!!nEnable) << DrvMusicWithoutASFHeader_Bits, DrvMusicWithoutASFHeader);
}

void
iteAudioSetSuspendEngine(
    int32_t nSuspend,
    uint16_t volume_status)
{
    int32_t  nResult;
    uint8_t  *pAddress;
    uint32_t length;
    uint16_t value;
    uint32_t i;

    if (nSuspend == 0)
    {
        //  not nSuspend
        ithWriteRegH(0x3A, 0xB000);
        ithWriteRegH(0x3C, 0x800F);
        ithWriteRegH(0x3E, 0x00AA);

        ithWriteRegH(0x1640, 0x000F);
        ithWriteRegH(0x1642, 0x400F);
        ithWriteRegH(0x1644, 0x3003);
        ithDelay(10);

        ithWriteRegMaskH(0x1672, 0xF5F7, 0xffff);                // Mute

        // 166E ==> discharge
        ithWriteRegMaskH(0x1670, 0x00C0, 0xFFFF);               // AMP power on
        ithDelay(50);                                           // wait 100ms
        ithWriteRegMaskH(0x1670, 0x0000, 0xFFFF);               // DACIP power on
        ithDelay(50);                                           // wait 100ms
        ithWriteRegMaskH(0x166e, 0x0000, 0xFFFF);               // AMP charge
        ithDelay(100);

        value = volume_status >> 12;

        for (i = 15; i >= value; i--)
        {
            ithWriteRegMaskH(0x1672, ((i << 12) | (volume_status & 0xFFF)), 0xFFFF);
            ithDelay(1);
            //fix while loop issue
            if (i == 0)
                break;
        }

        iteAudioGetAttrib(ITE_AUDIO_ENGINE_ADDRESS, pAddress);
        iteAudioGetAttrib(ITE_AUDIO_ENGINE_LENGTH, &length);
        nResult = iteAudioOpenEngine(AUD_SuspendEngineType);
    }
    else if (nSuspend == 1)
    {   // nSuspend
        AUD_SuspendEngineType = AUD_EngineType;

        AUD_ResetAudioProcessor();
        AUD_ResetAudioEngineType();

        value = volume_status >> 12;

        for (i = value; i <= 15; i++)
        {
            ithWriteRegMaskH(0x1672, ((i << 12) | (volume_status & 0xFFF)), 0xFFFF);
            ithDelay(1);
        }

        ithWriteRegMaskH(0x1672, 0xF5F7, 0xffff);       // Mute
        ithWriteRegMaskH(0x166e, 0x1000, 0x1000);       // AMP discharge
        // hw need sleep 100 ms
        ithDelay(100);                                  // wait 100 ms
        ithWriteRegMaskH(0x1670, 0x00C0, 0x00C0);       // DACIP power down
        ithDelay(10);                                   // wait 10ms
        ithWriteRegMaskH(0x1670, 0x10C0, 0x10C0);       // AMP power down
    }
}

uint16_t
iteAudioGetAuidoProcessorWritingStatus()
{
    uint16_t nRegData;
    nRegData = ithReadRegH(AUDIO_PROCESSOR_WRITE_REGISTER_PROTECTION);
    return nRegData;
}

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
// get audio codec api buffers
uint16_t *
iteAudioGetAudioCodecAPIBuffer(uint32_t *nLength)
{
    ithInvalidateDCacheRange(Audio_Plugin_Message_Buf, Audio_Plugin_Message_Buflen);
    *nLength = Audio_Plugin_Message_Buflen;
    return (uint16_t *)Audio_Plugin_Message_Buf;
}
#endif

int32_t
iteAudioGetAC3HWTrap()
{
    uint16_t nRegData;
    nRegData = ithReadRegH(0x0);
    return (nRegData & (1 << 14) );
}

int32_t
iteAudioGetAC3DrcMode(void)
{
    uint16_t regdata;
    regdata = ithReadRegH(AC3_DECODER_PARAMETER);
    return ((regdata >> 0) & 0x03);
}

void
iteAudioSetAC3DrcMode(int32_t mode)
{
    if (mode < 0 || mode > 3)
        return;

    ithWriteRegMaskH(AC3_DECODER_PARAMETER, (mode << 0), (3 << 0));
}

int32_t
iteAudioGetAC3DualMonoMode(void)
{
    uint16_t regdata;
    regdata = ithReadRegH(AC3_DECODER_PARAMETER);
    return ((regdata >> 2) & 0x03);
}

void
iteAudioSetAC3DualMonoMode(int32_t mode)
{
    if (mode < 0 || mode > 3)
        return;

    ithWriteRegMaskH(AC3_DECODER_PARAMETER, (mode << 2), (3 << 2));
}

int32_t
iteAudioGetAC32ChDownmixMode(void)
{
    uint16_t regdata;
    regdata = ithReadRegH(AC3_DECODER_PARAMETER);
    return ((regdata >> 4) & 0x03);
}

void
iteAudioSetAC32ChDownmixMode(int32_t mode)
{
    if (mode < 0 || mode > 3)
        return;

    ithWriteRegMaskH(AC3_DECODER_PARAMETER, (mode << 4), (3 << 4));
}

void
iteAudioSetAC3PrintMetaDataMode(int32_t mode)
{
    if (mode < 0 || mode > 1)
        return;

    ithWriteRegMaskH(AC3_DECODER_PARAMETER, (mode << 6), (1 << 6));
}

int32_t
iteAudioGetFlashInputStatus(int nInputNumber, int *nFormat, int *nUsing)
{
    uint16_t regdata;

    regdata = ithReadRegH(FLASH_AUDIO_INPUT_PARAMETER);
    switch (nInputNumber)
    {
    case 0:
        *nFormat = ((regdata >> 0) & 0x01);
        *nUsing  = ((regdata >> 7) & 0x01);
        break;

    case 1:
        *nFormat = ((regdata >> 1) & 0x01);
        *nUsing  = ((regdata >> 8) & 0x01);
        break;

    case 2:
        *nFormat = ((regdata >> 2) & 0x01);
        *nUsing  = ((regdata >> 9) & 0x01);
        break;

    case 3:
        *nFormat = ((regdata >> 3) & 0x01);
        *nUsing  = ((regdata >> 10) & 0x01);
        break;

    default:
        printf("[Audio Driver] get flash input status input error %d \n", nInputNumber);
        break;
    }
    printf("[Audio Driver] get flash input 0x%x \n", regdata);

    return 0;
}

int32_t
iteAudioSetFlashInputStatus(int nInputNumber, int nFormat, int nUsing)
{
    // format 0:mp3 1:wav
    uint16_t regdata = 0;
    uint16_t temp    = 0;

    nFormat = (nFormat == ITE_WAV_DECODE);

    switch (nInputNumber)
    {
    case 0:
        regdata = (nFormat << 0) | (nUsing << 7);
        ithWriteRegMaskH(FLASH_AUDIO_INPUT_PARAMETER, regdata, regdata);
        break;

    case 1:
        regdata = (nFormat << 1) | (nUsing << 8);
        ithWriteRegMaskH(FLASH_AUDIO_INPUT_PARAMETER, regdata, regdata);
        break;

    case 2:
        regdata = (nFormat << 2) | (nUsing << 9);
        ithWriteRegMaskH(FLASH_AUDIO_INPUT_PARAMETER, regdata, regdata);
        break;

    case 3:
        regdata = (nFormat << 3) | (nUsing << 10);
        ithWriteRegMaskH(FLASH_AUDIO_INPUT_PARAMETER, regdata, regdata);
        break;

    default:
        printf("[Audio Driver] set flash input status input error %d \n", nInputNumber);
        break;
    }
    temp = ithReadRegH(FLASH_AUDIO_INPUT_PARAMETER);
    printf("[Audio Driver] set flash input 0x%x 0x%x\n", regdata, temp);

    return 0;
}

// get audio codec buffer base address
uint32_t
iteAudioGetAudioCodecBufferBaseAddress()
{
    return CODEC_BASE;
}

unsigned int
iteAecCommand(
    unsigned short command,
    unsigned int param0,
    unsigned int param1,
    unsigned int param2,
    unsigned int param3,
    unsigned int *value)
{
    unsigned short reply;
    unsigned int   *pParameter;
    unsigned int   result        = 0xE000; // TODO: define error code
    unsigned int   timeout_count = 100;
    unsigned int   debug         = 0;

    unsigned int   i;
    unsigned char  *src;
    unsigned char  *det;
    unsigned int   size;

    if (command <= AEC_CMD_NULL || command >= AEC_CMD_LAST)
        return result;

    switch (command)
    {
    case AEC_CMD_PROCESS:
        // write data to echo buffer in
        size = param3;
        det  = (unsigned char *)Audio_Echo_Bufptr;
        src  = (unsigned char *)param0;
#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9850)
        for (i = 0; i < size; i += 2)
        {
            det[i]     = src[i + 1];
            det[i + 1] = src[i];
        }
#else
        memcpy((void *)det, (void *)src, size);
#endif
        ithFlushDCacheRange((void *)Audio_Echo_Bufptr, size);

        // write data to ref buffer in
        det = (unsigned char *)Audio_Ref_Bufptr;
        src = (unsigned char *)param1;
#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9850)
        for (i = 0; i < size; i += 2)
        {
            det[i]     = src[i + 1];
            det[i + 1] = src[i];
        }
#else
        memcpy((void *)det, (void *)src, size);
#endif
        ithFlushDCacheRange((void *)Audio_Ref_Bufptr, size);
        break;

    default:
        break;
    }

    pParameter    = (unsigned int *)Audio_Parameter_Cmdptr;
    pParameter[0] = param0;
    pParameter[1] = param1;
    pParameter[2] = param2;
    pParameter[3] = param3;
    ithFlushDCacheRange((void *)pParameter, 16);
    ithFlushMemBuffer();

    // check if processing then send command to risc
    do
    {
        if (ithReadRegH(AEC_COMMAND_SEND) == 0 &&
            ithReadRegH(AEC_COMMAND_REPLY) == 0)
            break;
        if (timeout_count == 0)
        {
            printf("aec command process timeout, cmd: %d, reg cmd: %d, reg reply: %d\n",
                   command, ithReadRegH(AEC_COMMAND_SEND), ithReadRegH(AEC_COMMAND_REPLY));
            return result;
        }
        else
            timeout_count--;
        usleep(1000);
    } while (1);
    ithWriteRegH(AEC_COMMAND_SEND, command);

    if (command == AEC_CMD_RESET)
    {
        // exceptional case, no need to wait for reply
        return 0;
    }

    // processing in risc, wait for reply
    switch (command)
    {
    case AEC_CMD_PROCESS:
#ifdef CFG_CHIP_PKG_IT9910
        usleep(16500);         // frame size 144
#else
        usleep(12000);         // frame size 144
#endif
        break;

    default:
        break;
    }

    timeout_count = 100;
    do
    {
        if (ithReadRegH(AEC_COMMAND_REPLY) == command &&
            ithReadRegH(AEC_COMMAND_SEND) == 0)
            break;
        if (timeout_count == 0)
        {
            printf("aec command wait timeout, cmd: %d, reg cmd: %d, reg reply: %d\n",
                   command, ithReadRegH(AEC_COMMAND_SEND), ithReadRegH(AEC_COMMAND_REPLY));
            return result;
        }
        else
            timeout_count--;
        usleep(1000);
    } while (1);

    // get reply data
    reply  = ithReadRegH(AEC_COMMAND_REPLY);
    //ithInvalidateDCache();
    ithInvalidateDCacheRange(Audio_Parameter_Cmdptr, 16);
    result = pParameter[4];
    if (value)
        *value = pParameter[5];
    debug  = pParameter[6];
    //printf("debug = %d\n", debug);

    switch (reply)
    {
    case AEC_CMD_PROCESS:
        // read data to echo cancellation buffer out
        size = param3;
        det  = (unsigned char *)param2;
        src  = (unsigned char *)Audio_Echocc_Bufptr;		
		ithInvalidateDCacheRange(Audio_Echocc_Bufptr, size);
#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9850)
        for (i = 0; i < size; i += 2)
        {
            det[i]     = src[i + 1];
            det[i + 1] = src[i];
        }
#else
        memcpy((void *)det, (void *)src, size);
#endif
        break;

    default:
        break;
    }

    ithWriteRegH(AEC_COMMAND_REPLY, 0);
    return result;
}
