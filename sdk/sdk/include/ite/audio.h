/*
 * Copyright (c) 2005 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * ITE Audio Driver API header file.
 *
 * @author Peter Lin
 * @version 1.0
 */
#ifndef ITE_AUDIO_H
#define ITE_AUDIO_H

//#include "ith_cfg.h"
#include "ite/ith.h"
#define ENABLE_AUDIO_PROCESSOR

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================
#define ITE_AUDIO_PCM_FORMAT                         0x2000
#define ITE_AUDIO_MP3_FORMAT                         0x2001
#define ITE_AUDIO_AAC_FORMAT                         0x2002
#define ITE_AUDIO_AMR_FORMAT                         0x2003
#define ITE_AUDIO_MID_FORMAT                         0x2004
#define ITE_AUDIO_BSAC_FORMAT                        0x2005
#define ITE_AUDIO_AC3_FORMAT                         0x2006
#define ITE_AUDIO_NIL_FORMAT                         0x7FFF

#define ITE_AUDIO_AMR_ENCODE_475                     (0x0 << 11)
#define ITE_AUDIO_AMR_ENCODE_515                     (0x1 << 11)
#define ITE_AUDIO_AMR_ENCODE_590                     (0x2 << 11)
#define ITE_AUDIO_AMR_ENCODE_670                     (0x3 << 11)
#define ITE_AUDIO_AMR_ENCODE_740                     (0x4 << 11)
#define ITE_AUDIO_AMR_ENCODE_795                     (0x5 << 11)
#define ITE_AUDIO_AMR_ENCODE_1020                    (0x6 << 11)
#define ITE_AUDIO_AMR_ENCODE_1220                    (0x7 << 11)

#define AUDIO_ERROR_BASE                             0

#define AUDIO_ERROR_ENGINE_ALIGNMENT_ERROR           (AUDIO_ERROR_BASE + 0x0001)
#define AUDIO_ERROR_ENGINE_ILLEGAL_INSTRUCTION_ERROR (AUDIO_ERROR_BASE + 0x0002)
#define AUDIO_ERROR_ENGINE_RESET_ENGINE_ERROR        (AUDIO_ERROR_BASE + 0x0002)
#define AUDIO_ERROR_ENGINE_LOAD_FAIL_ERROR           (AUDIO_ERROR_BASE + 0x0003)
#define AUDIO_ERROR_ENGINE_UNKNOW_STATUS_ERROR       (AUDIO_ERROR_BASE + 0x0009)

#define AUDIO_ERROR_ENGINE_CAN_NOT_START             (AUDIO_ERROR_BASE + 0x1001)
#define AUDIO_ERROR_ENGINE_CAN_NOT_STOP              (AUDIO_ERROR_BASE + 0x1002)
#define AUDIO_ERROR_ENGINE_DO_NOT_SUPPORT_ENCODE     (AUDIO_ERROR_BASE + 0x1003)
#define AUDIO_ERROR_ENGINE_DO_NOT_SUPPORT_DECODE     (AUDIO_ERROR_BASE + 0x1004)
#define AUDIO_ERROR_ENGINE_IS_RUNNING                (AUDIO_ERROR_BASE + 0x1005)
#define AUDIO_ERROR_ENGINE_UNKNOW_ENGINE_TYPE        (AUDIO_ERROR_BASE + 0x1005)

#define AUDIO_ERROR_CODEC_DO_NOT_INITIALIZE          (AUDIO_ERROR_BASE + 0x1001)
#define AUDIO_ERROR_CODEC_UNKNOW_CODEC_MODE          (AUDIO_ERROR_BASE + 0x1002)
#define AUDIO_ERROR_CODEC_CAN_NOT_FOUND_ADC_RATIO    (AUDIO_ERROR_BASE + 0x1004)
#define AUDIO_ERROR_CODEC_CAN_NOT_FOUND_DAC_RATIO    (AUDIO_ERROR_BASE + 0x1005)
#define AUDIO_ERROR_CODEC_UNKNOW_ERROR               (AUDIO_ERROR_BASE + 0x1009)

#define AUDIO_ERROR_MIDI_CAN_NOT_ALLOCATE_SYS_RAM    (AUDIO_ERROR_BASE + 0x2001)
#define AUDIO_ERROR_MIDI_CAN_NOT_ALLOCATE_VRAM       (AUDIO_ERROR_BASE + 0x2002)

#define FLASH_AUDIO_INPUT                            3

/* Additional region for nand boot */
#if defined(HAVE_NANDBOOT)
    #if defined(DEBUG)
        #define BOOTSTRAP_SIZE                       0x8000
    #else
        #define BOOTSTRAP_SIZE                       0x2000
    #endif // defined(DEBUG)
#else
    #define BOOTSTRAP_SIZE                           0x0
#endif

/* puts the plugin to array */
#define CODEC_ARRAY_SIZE                             (180 * 1024) // the maximun size of each plug-ins

/* the start address of CODEC plugin */
#define CODEC_START_ADDR                             (0x1000 + BOOTSTRAP_SIZE)

/* magic for normal codecs */
#define CODEC_MAGIC                                  0x534D3020 // "SM0 "

/* increase this every time the api struct changes */
#define CODEC_API_VERSION                            0x00000002

/* machine target, no used currently */
#define TARGET_ID                                    0x00000220

/* the maximun size of codec plug-ins */
#if defined(DEBUG)
    #define RISC1_SIZE                               (900 * 1024)
#else
    #ifdef ENABLE_XCPU_MSGQ
        #define RISC1_SIZE                           (900 * 1024)
    #else
        #if defined(HAVE_HEAACV2)
            #define RISC1_SIZE                       (900 * 1024)  // aac plugin takes >350KB.
        #elif defined(HAVE_WMA)
            #define RISC1_SIZE                       (900 * 1024)  // eac3 plugin takes ~350KB.
        #else
            #define RISC1_SIZE                       (900 * 1024) // Others plugin take ~198KB.
        #endif
    #endif
#endif
#define RISC2_SIZE                       (130 * 1024)
//=============================================================================
//                              Structure Definition
//=============================================================================
//=============================
//  Type Definition
//=============================
struct _codec_api {
    void *eqinfo;
    void *revbinfo;
};

struct _codec_header {
    unsigned long  magic;
    unsigned short target_id;
    unsigned short api_version;
    unsigned char  *load_addr;
    unsigned char  *end_addr;
    int            (*entry_point)(struct _codec_api *);
    unsigned char  *pStream;
    int            (*codec_info)();
};

typedef struct AUDIO_CODEC_STREAM_TAG
{
    int codecStreamBuf;
    int codecStreamLength;
    int codecStreamBuf1;
    int codecStreamLength1;
    int codecStreamBuf2;
    int codecStreamLength2;
    int codecAudioPluginBuf;
    int codecAudioPluginLength;
    //int codecStreamBuf3;
    //int codecStreamLength3;
    //int codecStreamBuf4;
    //int codecStreamLength4;
    //int codecStreamBuf5;
    //int codecStreamLength5;
    //int codecStreamBuf6;
    //int codecStreamLength6;
    int codecParameterBuf;
    int codecParameterLength;
    int codecEchoStateBuf;
    int codecEchoStateLength;
} AUDIO_CODEC_STREAM;
//=============================
//  Enumeration Type Definition
//=============================
/**
 * The structure defines audio engine type
 */
typedef enum ITE_AUDIO_ENGINE_TAG
{
    ITE_MP3_DECODE         = 1,  /**< MP3 decode engine */
    ITE_AAC_DECODE         = 2,  /**< AAC decode engine */
    ITE_AACPLUS_DECODE     = 3,  /**< AAC plus decode engine */
    ITE_BSAC_DECODE        = 4,  /**< BSAC decode engine */
    ITE_WMA_DECODE         = 5,  /**< WMA decode engine */
    ITE_AMR_ENCODE         = 6,  /**< AMR encode engine */
    ITE_AMR_DECODE         = 7,  /**< AMR decode engine */
    ITE_AMR_CODEC          = 8,  /**< AMR encode/decode engine */
    ITE_MIXER              = 9,  /**< MIXER engine */
    ITE_MIDI               = 10, /**< MIDI engine */
    ITE_PCM_CODEC          = 11, /**< PCM engine */
    ITE_WAV_DECODE         = 12, /**< WAV (PCM16/PCM8/a-law/mu-law engine */
    ITE_AC3_DECODE         = 13, /**< AC3 engine */
    ITE_OGG_DECODE         = 14, /**< OGG Vorbis engine */
    ITE_AC3_SPDIF_DECODE   = 15, /**< AC3 SPDIF engine */
    ITE_FLAC_DECODE        = 16, /**< FLAC decode engine */
    ITE_SBC_CODEC          = 17, /**< SBC engine */
    EXTERNAL_ENGINE_PLUGIN = 18, /**< None audio engine */
    ITE_RESERVED           = 19
} ITE_AUDIO_ENGINE;

typedef enum ITE_AUDIO_ENGINE_ATTRIB_TAG
{
    ITE_AUDIO_ENGINE_TYPE   = 0,
    ITE_AUDIO_ENGINE_STATUS = 1
} ITE_AUDIO_ENGINE_ATTRIB;

typedef enum ITE_AUDIO_ENGINE_STATE_TAG
{
    ITE_AUDIO_ENGINE_IDLE        = 0,
    ITE_AUDIO_ENGINE_RUNNING     = 1,
    ITE_AUDIO_ENGINE_STOPPING    = 2,
    ITE_AUDIO_ENGINE_NO_RESPONSE = 3,
    ITE_AUDIO_ENGINE_UNKNOW      = 4,
    ITE_AUDIO_ENGINE_ERROR       = 9
} ITE_AUDIO_ENGINE_STATE;

/**
 * The structure defines codec parameters
 */
typedef enum ITE_AUDIO_CODEC_ATTRIB_TAG
{
    ITE_AUDIO_CODEC_NONE              = 0,
    ITE_AUDIO_CODEC_INITIALIZE        = 1,
    ITE_AUDIO_CODEC_TERMINATE         = 2,
    ITE_AUDIO_CODEC_SETVOLUME         = 3,
    ITE_AUDIO_CODEC_MODE              = 4,
    ITE_AUDIO_CODEC_ADC_RATIO         = 5,
    ITE_AUDIO_CODEC_DAC_RATIO         = 6,
    ITE_AUDIO_CODEC_OVERSAMPLING_RATE = 7
} ITE_AUDIO_CODEC_ATTRIB;

typedef enum ITE_AUDIO_CODEC_MODE_TYPE_TAG
{
    ITE_AUDIO_CODEC_ADMS_DAMS_MODE,     // ADC master / DAC master mode
    ITE_AUDIO_CODEC_ADSL_DASL_MODE,     // ADC slave  / DAC slave  mode
    ITE_AUDIO_CODEC_ADMS_DASL_MODE,     // ADC master / DAC slave  mode
    ITE_AUDIO_CODEC_ADSL_DAMS_MODE,     // ADC slave  / DAC master mode
    ITE_AUDIO_CODEC_CODEC0_MS_MODE,     // Codec 0 master mode
    ITE_AUDIO_CODEC_CODEC0_SL_MODE,     // Codec 0 slave  mode
    ITE_AUDIO_CODEC_CODEC1_MS_MODE,     // Codec 1 master mode
    ITE_AUDIO_CODEC_CODEC1_SL_MODE,     // Codec 1 slave  mode
    ITE_AUDIO_CODEC_BT_ONLY_MSB_MODE,   // Bluetooth only Long  frame (MSB) mode
    ITE_AUDIO_CODEC_BT_ONLY_I2S_MODE,   // Bluetooth only short frame (I2S) mode
    ITE_AUDIO_CODEC_BT_DAC_MSB_MODE,    // Bluetooth with DAC long  frame (MSB) mode
    ITE_AUDIO_CODEC_BT_DAC_I2S_MODE,    // Bluetooth with DAC short frame (I2S) mode
    ITE_AUDIO_CODEC_BT_ADC_MSB_MODE,    // ADC with Bluttooth long  frame (MSB) mode
    ITE_AUDIO_CODEC_BT_ADC_I2S_MODE     // ADC with Bluttooth short frame (I2S) mode
} ITE_AUDIO_CODEC_TYPE_MODE;

typedef enum ITE_AUDIO_BUFFER_TYPE_TAG
{
    ITE_AUDIO_INPUT_BUFFER,
    ITE_AUDIO_OUTPUT_BUFFER,
    ITE_AUDIO_MIXER_BUFFER
} ITE_AUDIO_BUFFER_TYPE;

typedef enum ITE_AUDIO_EQTYPE_TAG
{
    EQ_CLASSICAL,
    EQ_CLUB,
    EQ_LIVE,
    EQ_POP,
    EQ_ROCK,
    EQ_USERDEF
} ITE_AUDIO_EQTYPE;

typedef enum ITE_AUDIO_REVBTYPE_TAG
{
    REVB_TEST1,
    REVB_TEST2,
    REVB_TEST3
} ITE_AUDIO_REVBTYPE;

typedef enum {
    ITE_WAVE_FORMAT_PCM       = 0,           /* Microsoft PCM Format */
    ITE_WAVE_FORMAT_ALAW      = 1,           /* Microsoft ALAW */
    ITE_WAVE_FORMAT_MULAW     = 2,           /* Microsoft MULAW */
    ITE_WAVE_FORMAT_DVI_ADPCM = 3,
    ITE_WAVE_FORMAT_SWF_ADPCM = 4,
} ITE_WAVE_FORMAT;

typedef enum {
    ITE_WAVE_SRATE_6000  = 0,
    ITE_WAVE_SRATE_8000  = 1,
    ITE_WAVE_SRATE_11025 = 2,
    ITE_WAVE_SRATE_12000 = 3,
    ITE_WAVE_SRATE_16000 = 4,
    ITE_WAVE_SRATE_22050 = 5,
    ITE_WAVE_SRATE_24000 = 6,
    ITE_WAVE_SRATE_32000 = 7,
    ITE_WAVE_SRATE_44100 = 8,
    ITE_WAVE_SRATE_48000 = 9
} ITE_WAVE_SRATE;

//=============================
//  Structure Type Definition
//=============================

typedef struct ITE_AUDIO_EQINFO_TAG
{
    int16_t updEQ;
    int16_t endian;
    int16_t bandcenter[16];
    int16_t dbtab[16];
} ITE_AUDIO_EQINFO;

typedef struct ITE_AUDIO_REVBINFO_TAG
{
    uint16_t updReverb;
    uint16_t endian;
    uint32_t src_gain;
    uint32_t reverb_gain;
    uint32_t cross_gain;
    uint32_t filter[2][8];
} ITE_AUDIO_REVBINFO;

/**
 * The structure defines audio attributes.
 */
typedef enum ITE_AUDIO_ATTRIB_TAG
{
    //ITE_AUDIO_DEVICE_STATUS,

    //ITE_AUDIO_STREAM_MODES,
    //ITE_AUDIO_STREAM_PCM_TIME,
    //ITE_AUDIO_STREAM_ENC_TIME,
    //ITE_AUDIO_STREAM_DEC_TIME,
    //ITE_AUDIO_STREAM_PCM_AVAIL_LENGTH,
    //ITE_AUDIO_STREAM_DEC_AVAIL_LENGTH,
    //ITE_AUDIO_STREAM_DEC_I2S_AVAIL_LENGTH,
    //ITE_AUDIO_STREAM_ENC_AVAIL_LENGTH,
    //ITE_AUDIO_STREAM_ENC_I2S_AVAIL_LENGTH,
    //ITE_AUDIO_STREAM_TYPE,
    ITE_AUDIO_STREAM_SAMPLERATE,
    //ITE_AUDIO_STREAM_BITRATE,
    ITE_AUDIO_STREAM_CHANNEL,
    //ITE_AUDIO_STREAM_FREQUENCY_INFO,
    //ITE_AUDIO_STREAM_PLAYBACKRATE,

    //ITE_AUDIO_EFFECT_EN_DRC,
    //ITE_AUDIO_EFFECT_EN_EQUALIZER,
    //ITE_AUDIO_EFFECT_EN_REVERB,
    //ITE_AUDIO_EFFECT_EN_VOICE,
    //ITE_AUDIO_EFFECT_EQ_TYPE,
    //ITE_AUDIO_EFFECT_EQ_TABLE,
    //ITE_AUDIO_EFFECT_REVERB_TYPE,
    //ITE_AUDIO_EFFECT_REVERB_TABLE,
    ITE_AUDIO_PLUGIN_PATH,
    ITE_AUDIO_GET_IS_EOF,
    ITE_AUDIO_CODEC_SET_SAMPLE_RATE,
    ITE_AUDIO_CODEC_SET_CHANNEL,
    ITE_AUDIO_CODEC_SET_BUFFER_LENGTH,
    ITE_AUDIO_MUSIC_PLAYER_ENABLE,
    ITE_AUDIO_PTS_SYNC_ENABLE,
    ITE_AUDIO_ENGINE_ADDRESS,
    ITE_AUDIO_ENGINE_LENGTH,
    ITE_AUDIO_I2S_INIT,
    ITE_AUDIO_I2S_PTR,
    //ITE_AUDIO_FADE_IN,
    ITE_AUDIO_I2S_OUT_FULL,
    ITE_AUDIO_ENABLE_MPLAYER,
    //ITE_AUDIO_ADJUST_MPLAYER_PTS,
    ITE_AUDIO_MPLAYER_STC_READY,
    ITE_AUDIO_DECODE_ERROR,
    ITE_AUDIO_DECODE_DROP_DATA,
    ITE_AUDIO_PAUSE_STC_TIME,
    ITE_AUDIO_IS_PAUSE,
    ITE_AUDIO_ENABLE_STC_SYNC,
    ITE_AUDIO_DIRVER_DECODE_BUFFER_LENGTH,
    ITE_AUDIO_SPDIF_NON_PCM,
    ITE_AUDIO_FFMPEG_PAUSE_AUDIO,    
    ITE_AUDIO_NULL_ATTRIB
} ITE_AUDIO_ATTRIB;

typedef struct ITE_AUDIO_DECODER_TAG
{
    ITE_AUDIO_ENGINE engineType;
    unsigned int     sampleRate;
    unsigned int     channelNum;
    unsigned int     maxBufferSize;

    // for PTS
    unsigned int lastTimeStamp;
    unsigned int timeStamp;

    // stc sync
    int          nSTCDifferent;
    unsigned int nChangeAudioTimeCount;
    unsigned int nAudioBehindSTC;
    int          nPreSTCAudioGap;
    unsigned int nPauseSTCTime;
    unsigned int nSTCStartTime;
    bool         bEngineExisted;
} ITE_AUDIO_DECODER;

typedef struct ITE_WaveInfo_TAG {
    ITE_WAVE_FORMAT format;
    uint32_t        nChans;
    uint32_t        sampRate;
    uint32_t        bitsPerSample;
    void            *pvData;
    uint32_t        nDataSize;
} ITE_WaveInfo;

typedef struct ITE_WmaInfo_TAG {
    uint32_t packet_size;
    int32_t  audiostream;
    uint16_t codec_id;
    uint16_t channels;
    uint32_t sampleRate;
    uint32_t bitrate;
    uint16_t blockalign;
    uint16_t bitspersample;
    uint16_t datalen;
    uint8_t  data[6];
} ITE_WmaInfo;

typedef enum ITE_AUDIO_MODE_TAG
{
    ITE_AUDIO_STEREO = 0,
    ITE_AUDIO_LEFT_CHANNEL,
    ITE_AUDIO_RIGHT_CHANNEL,
    ITE_AUDIO_MIX_LEFT_RIGHT_CHANNEL
} ITE_AUDIO_MODE;

// flash codec structure
typedef struct ITE_FlashInfo_TAG {
    uint16_t     nAudioEnable;
    uint16_t     nFlashEnable;
    uint32_t     nInputUsing[FLASH_AUDIO_INPUT];
    ITE_WaveInfo wavInfo[FLASH_AUDIO_INPUT];
    uint16_t     nAudioFormat[FLASH_AUDIO_INPUT];
} ITE_FlashInfo;

enum {
    AEC_CMD_NULL = 0,
    AEC_CMD_INIT,
    AEC_CMD_PROCESS,
    AEC_CMD_RESET,
    AEC_CMD_LAST
};

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup group8 ITE Audio Driver API
 *  The supported API for audio.
 *  @{
 */

/**
 * Initialize audio engine.
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark Application must call this API first when it want to use audio API.
 * Before iteAudioTerminate() this API will be used once only.
 */
int32_t iteAudioInitialize(void);

/**
 * Initialize codec
 * @param  attribList
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark Application must call this API first when it want to use audio API.
 * Before iteAudioTerminate() this API will be used once only.
 */
int32_t iteAudioSetCodec(unsigned long *attribList);

/**
 * Specifies which type audio engine will be used.
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark Application must call this API to Specifies which type audio engine is needed.
 * Before iteAudioStopEngine() this API will be used once only.
 */
int32_t iteAudioOpenEngine(ITE_AUDIO_ENGINE audio_type);

/**
 * Active audio engine to start encode or decode except chosen MIDI engine.
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark IF application choice decode function, audio engine will start to decode stream data inputted by iteAudioWriteStream().
 * IF application choice encode function, audio engine will start to record and encode data
 * , and them application can use iteAudioReadStream() to get encoded data.
 * IF application choice MIDI engine,  iteAudioActiveMidiEngine() is desired to active midi engine.
 */
int32_t iteAudioActiveEngine(void);

/**
 * Active audio MIDI engine to work.
 *
 * @param midiData          Specifies midi data buffer.
 * @param midiDataSize      Specifies midi data size.
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark Now audio engine will play back MIDI.
 */
int32_t iteAudioActiveMidiEngine(uint8_t *midiData, unsigned long midiDataSize);

/**
 * Get current running engine attribute
 *
 * @param attrib            Specifies engine attribute to get
 * @param value             Pointer to attribute value
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 */
int32_t iteAudioGetEngineAttrib(ITE_AUDIO_ENGINE_ATTRIB attrib, void *value);

/**
 * Get available buffer length
 *
 * @param bufferType        Specifies buffer type
 * @param bufferLength      Pointer to get buffer size
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark Now audio engine will play back MIDI.
 */
int32_t iteAudioGetAvailableBufferLength(ITE_AUDIO_BUFFER_TYPE bufferType, unsigned long *bufferLength);

/**
 * Input stream data for audio engine to play back.
 *
 * @param stream                Specifies stream data buffer.
 * @param streamBufferSize      Specifies stream buffer size .
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark Audio engine play back stream data, the data format is according to audio engine type chosen by iteAudioOpenEngine() API.
 */
int32_t iteAudioWriteStream(uint8_t *stream, unsigned long streamBufferSize);

int32_t iteAudioGetFlashAvailableBufferLength(int nAudioInput, unsigned long *bufferLength);
int32_t iteAudioWriteFlashStream(int nAudioInput,  uint8_t *stream, unsigned long length);

/**
 * Get encoded stream data from audio engine.
 *
 * @param stream                Specifies stream data buffer.
 * @param streamBufferSize      Specifies stream buffer size.
 * @param streamthreshold       Specifies threshold value to avoid waitting.
 * @return encoded size.
 * @remark Audio engine record and encode data according to audio engine type chosen by iteAudioOpenEngine() API.
 * Application can specify threshold value to avoid waitting, if the encoded data is smaller than threshold,
 * the function will not get enocded data and return zero.
 */
unsigned long iteAudioReadStream(uint8_t *stream, unsigned long streamBufferSize, unsigned long streamthreshold);

/**
 * Tell engine now is end of a song.
 *
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark Application can call this API to tell audio engine this is end of song,
 * and engine can prepare for beginner of next song.
 * Application do not need to call this API when encode or MIDI engine is choosen.
 */
int32_t iteAudioEndStream(void);

/**
 * Stop audio engine.
 *
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark Audio engine will stop immediately when this API is called.
 * Application can use iteAudioActiveEngine() or iteAudioActiveMidiEngine() to restart engine.
 */
int32_t iteAudioStopEngine(void);

/**
 * Check if audio engine finish MIDI playback.
 *
 * @return 1 if engine finish MIDI playback, 0 otherwise.
 * @remark Application can use this API to know if engine finish MIDI playback.
 */
bool iteAudioIsMidiEnd(void);

/**
 * Get encoding frame ID of audio engine working now.
 *
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark Application can use this API to know audio time form starting encoding.
 */
int32_t iteAudioGetEncodeTime(uint32_t *time);

/**
 * Get decoding frame ID of audio engine working now.
 *
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark Application can use this API to know audio time form starting decoding .
 */
int32_t iteAudioGetDecodeTime(uint32_t *time);

int32_t iteAudioGetDecodeTimeV2(uint32_t *time);
int32_t iteAudioSetAttrib(const ITE_AUDIO_ATTRIB attrib, const void *value);
int32_t iteAudioGetAttrib(const ITE_AUDIO_ATTRIB attrib, void *value);

/**
 * Enable or disable equalizer for audio surrond effect.
 *
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark Application can use this API to turn on or tunr off equalizer effect during decoding process.
 */
int32_t iteAudioSetEqualizer(bool enableEq);

/**
 * Enable or disable voice remove function.
 *
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark Application can use this API to remove voice during decoding process.
 */
int32_t iteAudioSetVoiceRemove(bool enVoiceRemove);

/**
 * Set volume of audio
 *
 * @param volume            specifies the volume
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark Application can use this API to create reverbration effect during decoding process.
 */
int32_t iteAudioSetVolume(uint32_t volume);

/**
 * Get current volume of audio
 *
 * @param volume            specifies the volume pointer
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark Application can use this API to create reverbration effect during decoding process.
 */
int32_t iteAudioGetVolume(uint32_t *volume);

/**
 * Change default equalizer type.
 *
 * @param eqType            default equalizer type
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark Application can use this API to create reverbration effect during decoding process.
 */
int32_t iteAudioChangeEqualizer(ITE_AUDIO_EQTYPE eqType);

/**
 *  Audio terminate function.
 *
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark After call this function, application can not use any audio API expext iteAudioInitialize().
 */
int32_t iteAudioTerminate(void);

int32_t
iteAudioPause(
    bool enable);

int32_t
iteAudioSTCSyncPause(
    bool enable);

int32_t
iteAudioSeek(
    bool enable);

int32_t
iteAudioStop(
    void);

int32_t
iteAudioStopQuick(
    void);

int32_t
iteAudioStopImmediately(
    void);

int32_t
iteAudioPowerOnAmplifier(
    void);

int32_t
iteAudioPowerOffAmplifier(
    void);

int32_t
iteAudioSetWaveDecodeHeader(ITE_WaveInfo wavInfo);

int32_t
iteAudioGenWaveDecodeHeader(ITE_WaveInfo *wavInfo, uint8_t *wave_header);

int32_t
iteAudioWriteWavInfo();

int32_t
iteAudioSetWmaInfo(ITE_WmaInfo tWmaInfo);

int32_t
iteAudioSaveWmaInfo(ITE_WmaInfo tWmaInfo);

int32_t
iteAudioWriteWmaInfo();

/**
 *  Set the Audio play position.
 *
 * @param pos         specifies the current file position.
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark After call this function, application can not use any audio API expext iteAudioInitialize().
 */
int32_t iteAudioSetPlayPos(int32_t pos);

/**
 *  Get the Audio play position.
 *
 * @param pos         get the current file position which is decoded.
 * @return 0 if succeed, error codes of int32_t_ERROR
 *         otherwise.
 * @remark After call this function, application can not use any audio API expext iteAudioInitialize().
 */
int32_t iteAudioGetPlayPos(int32_t *pos);

int32_t
iteAudioSetPts(
    uint32_t pts);

void
iteAudioSetEarPhonePlugin(bool detect);

bool
iteAudioGetEarPhonePlugin(void);

int
iteAudioGetOutputEmpty();

void
iteAudioSetOutputEmpty(
    int32_t mode);

void
iteAudioSetMusicShowSpectrum (
    int32_t nEnable);

int32_t
iteAudioGetMp3RdBufPointer(void);

void
iteAudioSetMp3RdBufPointer (
    int32_t nReset);


void
iteAudioSetWmaWithoutASFHeader(
    int32_t nEnable);

void
iteAudioSuspendEngine();

uint16_t
iteAudioGetAuidoProcessorWritingStatus();

// get audio codec api buffers
uint16_t *
iteAudioGetAudioCodecAPIBuffer(uint32_t *nLength);

int32_t
iteAudioGetAC3HWTrap(void);

int32_t
iteAudioGetAC3DrcMode(void);

void
iteAudioSetAC3DrcMode(int32_t mode);

int32_t
iteAudioGetAC3DualMonoMode(void);

void
iteAudioSetAC3DualMonoMode(int32_t mode);

int32_t
iteAudioGetAC32ChDownmixMode(void);

void
iteAudioSetAC32ChDownmixMode(int32_t mode);

void
iteAudioSetAC3PrintMetaDataMode(int32_t mode);

int32_t
iteAudioGetFlashInputStatus(int nInputNumber, int *nFormat, int *nUsing);

int32_t
iteAudioSetFlashInputStatus(int nInputNumber, int nFormat, int nUsing);

// get audio codec buffer base address
uint32_t
iteAudioGetAudioCodecBufferBaseAddress(
    void);

unsigned int
iteAecCommand(
    unsigned short command,
    unsigned int   param0,
    unsigned int   param1,
    unsigned int   param2,
    unsigned int   param3,
    unsigned int   *value);

///////////////////////////////////////////////////////////////////////////////
//@}

//bool audioKeySoundPaused;

#ifdef __cplusplus
}
#endif

#endif //ITE_AUDIO_H//