/***************************************************************************
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 *
 * @file
 * PCM/uLaw/aLaw/adpcm Decoder,Encoder
 *
 * @author Kuoping Hsu
 * @version 1.0
 * @date 2007/04/12
 *
 ***************************************************************************/

#include "wav_config.h"


#if defined(__OR32__)
#  include "mmio.h"
#  include "i2s.h"
#else
#  include "win32.h"
#endif

#include "statname.h"
#include "debug.h"

#if defined(__FREERTOS__)
#  include "FreeRTOS.h"
#  include "task.h"
#endif
#include <stdarg.h>
#include <string.h>

#if defined(ENABLE_CODECS_PLUGIN)
#  include "plugin.h"
#endif

#ifdef WAV_SPECTREM_PERFORMANCE_TEST
static PAL_CLOCK_T tClockPerformance;
static long nSpectrumPerformance;  // avg of frame
#endif
static unsigned short nWavDecodedByte=0;
static int nKeepByte = 0;

///////////////////////////////////////////////////////////////////////////
//                              Constant Definition
///////////////////////////////////////////////////////////////////////////
#define FRAME_SIZE              (512)   // Number of sample per frame
#define MAX_BYTES_PER_SAMPLE    (4)
#define PCM_BIT_PERSAMPLE       (16)

#if defined(__FREERTOS__)
#  define READBUF_SIZE        (22*FRAME_SIZE*MAX_BYTES_PER_SAMPLE)
#  define I2SBUF_SIZE           (READBUF_SIZE)
#else
#  define READBUF_SIZE          (8*FRAME_SIZE*MAX_BYTES_PER_SAMPLE)
#  define I2SBUF_SIZE           READBUF_SIZE
#endif // __FREERTOS__

#define WAIT_PCM_BYTES          (FRAME_SIZE * WaveInfo.nChans * PCM_BIT_PERSAMPLE / 8)
#define WAIT_WAV_BYTES          (FRAME_SIZE * WaveInfo.nChans * WaveInfo.bitsPerSample / 8)

#if READBUF_SIZE > 65535 || I2SBUF_SIZE > 65535
#  error "The buffer exceed 64K bytes."
#endif

typedef enum {
  WAVE_FORMAT_UNKNOWN    = 0x0000, /* Microsoft Unknown Wave Format */
  WAVE_FORMAT_PCM        = 0x0001, /* Microsoft PCM Format */
  WAVE_FORMAT_ADPCM      = 0x0002, /* Microsoft ADPCM Format */
  WAVE_FORMAT_ALAW       = 0x0006, /* Microsoft ALAW */
  WAVE_FORMAT_MULAW      = 0x0007, /* Microsoft MULAW */
  WAVE_FORMAT_DVI_ADPCM  = 0x0011, /* Intel's DVI ADPCM */
  WAVE_FORMAT_G723_ADPCM = 0x0014, /* G.723 ADPCM */
  WAVE_FORMAT_G726_ADPCM = 0x0064, /* G.726 ADPCM */
  WAVE_FORMAT_G722_ADPCM = 0x0065  /* G.722 ADPCM */
} WAVE_FORMAT;

typedef struct _WaveInfo {
    WAVE_FORMAT format;
    unsigned int nChans;
    unsigned int sampRate;
    unsigned int avgBytesPerSec;
    unsigned int blockAlign;
    unsigned int bitsPerSample;
    unsigned int samplesPerBlock;
} _WaveInfoMode;

static _WaveInfoMode WaveInfo;

const static int SampsRate[] = {
     6000,  8000, 11025, 12000,
    16000, 22050, 24000, 32000,
    44100, 48000, -1
};
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
static unsigned int* gtAudioPluginBufferLength;
static unsigned short* gtAudioPluginBuffer;
static int gnTemp;
static unsigned char tDumpWave[] = "C:/wav_dump.wav";
static int gCh;
static int gSampleRate;
int gPause = 0 ;
int gPrint = 0;
unsigned char *gBuf;
#endif
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
/* Code for general_printf() */
#define BITS_PER_BYTE    8
#define MINUS_SIGN       1
#define RIGHT_JUSTIFY    2
#define ZERO_PAD         4
#define CAPITAL_HEX      8

struct parameters
{
    int   number_of_output_chars;
    short minimum_field_width;
    char  options;
    short edited_string_length;
    short leading_zeros;
};
#endif        
///////////////////////////////////////////////////////////////////////////
//                              Local Variable
//
// PCM -> 16bits PCM data to/from I2S
// WAV -> Stream data to/from driver
//
///////////////////////////////////////////////////////////////////////////
static unsigned int wavReadIdx;
static unsigned int wavWriteIdx;
static unsigned int pcmReadIdx;
static unsigned int pcmWriteIdx;
static unsigned int wavFrameStep;
static unsigned int wavFrameAccu;
static int isEncode;
static int eofReached;
static int decTime;
static int headerErr; // Global variable for debugging.
static struct adpcm_state state;
static int wait_pcm_bytes;
static int wait_wav_bytes;
static int nDecEndian;
static int nShowSpectrum;
#if defined(__GNUC__)
static unsigned char streamBuf[READBUF_SIZE] __attribute__ ((aligned(16), section (".sbss")));
static unsigned char pcmWriteBuf[I2SBUF_SIZE] __attribute__ ((aligned(16), section (".sbss")));
#else
static unsigned char streamBuf[READBUF_SIZE];
static unsigned char pcmWriteBuf[I2SBUF_SIZE];
#endif // defined(__GNUC__)

#if defined(ENABLE_PERFORMANCE_MEASURE)
#  include "ticktimer.h"
    #define MAX_TIME_LOG 10000
    static unsigned int time_log[MAX_TIME_LOG];
#endif // ENABLE_PERFORMANCE_MEASURE

///////////////////////////////////////////////////////////////////////////
//                              Function Decleration
///////////////////////////////////////////////////////////////////////////
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
void WAV_GetBufInfo(unsigned* inDecBuf, unsigned* inDecLen,unsigned* inEnBuf, unsigned* inEnLen, unsigned* freqbuf,unsigned* audioPluginBuf, unsigned* audioPluginBufLen);
void AudioPluginAPI(int nType);
__inline int ithPrintf(char* control, ...);
#else
void WAV_GetBufInfo(unsigned* inDecBuf, unsigned* inDecLen,unsigned* inEnBuf, unsigned* inEnLen, unsigned* freqbuf);
#endif
static __inline void checkControl(void);
static __inline unsigned int getStreamWrPtr(void);
static int  ParseWaveHeader(void);
static int  DecFillWriteBuffer(int nPCMBytes);
static int  DecFillReadBuffer(int nReadBytes);
static void DecClearRdBuffer(void);
#if defined(HAVE_ENCODE)
static int  PackWaveHeader(void);
static int  EncFillWriteBuffer(int nWAVBytes);
static int  EncFillReadBuffer(int nReadBytes);
static void EncClearRdBuffer(void);
#endif // HAVE_ENCODE

///////////////////////////////////////////////////////////////////////////
//                              Function Body
///////////////////////////////////////////////////////////////////////////

#include "g711.c"
#include "adpcm.c"
#include "freqinfo.c"

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define getFreeLen(rdptr, wrptr, len) (((rdptr) <= (wrptr)) ? ((len) - ((wrptr) - (rdptr)) - 2) : ((rdptr) - (wrptr) - 2))
#define getUsedLen(rdptr, wrptr, len) (((wrptr) >= (rdptr)) ? ((wrptr) - (rdptr)) : ((len) - ((rdptr) - (wrptr))))

#if defined(__OPENRTOS__) || defined(WIN32) || defined(__CYGWIN__)

#if defined(FREQINFO)
    extern char freqinfo[FREQINFOCNT];
#endif
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
void WAV_GetBufInfo(unsigned* inDecBuf, unsigned* inDecLen,unsigned* inEnBuf, unsigned* inEnLen, unsigned* freqbuf,unsigned* audioPluginBuf, unsigned* audioPluginBufLen)
{
#if 0
    *inDecBuf = (unsigned)streamBuf;
    *inDecLen = sizeof(streamBuf);
    *inEnBuf =  (unsigned)streamBuf;//(unsigned) pcmWriteBuf;
    *inEnLen =  sizeof(streamBuf);//sizeof(pcmWriteBuf);
    #if defined(FREQINFO)
    *freqbuf = (unsigned)&freqinfo[0];
    #endif

    gtAudioPluginBuffer = (unsigned short*)audioPluginBuf;
    gtAudioPluginBufferLength = audioPluginBufLen;
    
    //printf("[Mp3] plugin buffer length %d  0x%x buf 0x%x \n",*gtAudioPluginBufferLength,audioPluginBufLen,audioPluginBuf);
#endif  
}
#else
void WAV_GetBufInfo(unsigned* inDecBuf, unsigned* inDecLen,unsigned* inEnBuf, unsigned* inEnLen, unsigned* freqbuf)
{

    *inDecBuf = (unsigned)streamBuf;
    *inDecLen = sizeof(streamBuf);
    *inEnBuf =  (unsigned)streamBuf;//(unsigned) pcmWriteBuf;
    *inEnLen =  sizeof(streamBuf);//sizeof(pcmWriteBuf);
#if defined(FREQINFO)
    *freqbuf = (unsigned)&freqinfo[0];
#endif

}
#endif

#endif // defined(__FREERTOS__)

/**************************************************************************************
 * Function     : checkControl
 *
 * Description  : Check the flow control signal.
 *
 * Input        : None
 *
 * Output       : None
 *
 * Note         :
 **************************************************************************************/
static __inline void checkControl(void) {
    static int curPause = 0;
    static int prePause = 0;

    do {
        eofReached = isEOF() || isSTOP() || isEncEOF();
        curPause = isPAUSE();
        if (!curPause) {  // Un-pause
            if (prePause) {
                if (isEncode)
                {
                    SetInRdPtr(GetInWrPtr());
                }
                else
                {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
                    gPause = 0;
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC);
#else
                    pauseDAC(0);
#endif 
                }
            }
            break;
        } else { // Pause
            if (!prePause && curPause)
            {
                if (!isEncode)
                {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
                    gPause = 1;
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC);
#else
                    pauseDAC(1);
#endif                
                }
            }
            #if defined(__FREERTOS__)
            PalSleep(2);
            #else
            //or32_delay(1); // delay 1ms
            #endif
        }
        prePause = curPause;
    } while(!eofReached);

    prePause = curPause;
}

/**************************************************************************************
 * Function     : getStreamWrPtr
 *
 * Description  : Get the stream of write pointer, if the pointer is 0xffff then stop
 *                decoding.
 *
 * Inputs       : None
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 **************************************************************************************/
// For Decoder
static __inline unsigned int getStreamWrPtr(void) {
    unsigned int wrPtr;

    wrPtr = MMIO_Read(DrvDecode_WrPtr);
    if (0xffff == wrPtr)
    {
        setAudioReset();
        while (1);
    }
#if defined(__OR32__) && !defined(__OPENRTOS__)
    if (0xffff == wrPtr) asm volatile("l.trap 15");
#endif

    return wrPtr;
}

// For Encoder
static __inline unsigned int getStreamRdPtr(void) {
    unsigned int rdPtr;
    rdPtr = MMIO_Read(DrvEncode_RdPtr);

    #if defined(__OR32__) && !defined(__OPENRTOS__)
    if (0xffff == rdPtr) asm volatile("l.trap 15");
    #endif

    return rdPtr;
}

static __inline int ENDIAN_LE32(unsigned char *n) {
    int num = (((unsigned int)n[0]      ) + ((unsigned int)n[1] <<  8) +
               ((unsigned int)n[2] << 16) + ((unsigned int)n[3] << 24) );
    return num;
}

static __inline short ENDIAN_LE16(unsigned char *n) {
    short num = (short)(((unsigned short)n[0]) + ((unsigned short)n[1] << 8));
    return num;
}

/**************************************************************************************
 * Function     : ParseWaveHeader
 *
 * Description  : Parse the header & update the read pointer
 *
 * Inputs       : None
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 **************************************************************************************/
static int ParseWaveHeader(void) {
    unsigned char *header;
    int nWaitBytes;
    int done;
    int header_size;

    enum
    {
        INIT_STATE = 0,
        CHK_TYPE   = 1,
        FMT_TYPE   = 2,
        FACT_TYPE  = 3,
        DATA_TYPE  = 4,
    } readState;

    nWaitBytes  = 12;    // Wait the wave header
    done        = 0;
    header_size = 0;
    header      = &streamBuf[wavReadIdx];
    readState   = INIT_STATE;
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
    //ithPrintf("[Wav] info %d %d 0%0x 0%0x 0%0x 0%0x 0%0x 0%0x 0%0x 0%0x 0%0x\n",wavReadIdx,wavWriteIdx,header,header[0],header[1],header[2],header[3],header[4],header[5],streamBuf[0],streamBuf[1]);
    //AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
    printf("Not .wav file\n");
#endif               

    while(!done)
    {
        // Wait the header on output buffer avaliable
        do
        {
            int len;
            checkControl();
            wavWriteIdx = getStreamWrPtr();

            len = getUsedLen(wavReadIdx, wavWriteIdx, sizeof(streamBuf));

            if (len < nWaitBytes && !eofReached) 
             {
                #if defined(__FREERTOS__)
                PalSleep(2);
                #else
                //or32_delay(1); // enter sleep mode for power saving
                #endif
            } 
            else 
            {
                break;
            }
        } while(1);
      dc_invalidate();

        
        switch(readState) {
        case INIT_STATE : if (!(header[ 0] == 'R' && header[ 1] == 'I' && header[ 2] == 'F' && header[ 3] == 'F' &&
                                header[ 8] == 'W' && header[ 9] == 'A' && header[10] == 'V' && header[11] == 'E')) {
                                #if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                                    ithPrintf("[Wav] Not .wav file \n");
                                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
                                #else
                              PRINTF("Not .wav file\n");
                                #endif               
                              return 0;
                          }
                          PRINTF("WAVINFO: FileSize: %d bytes\n", ENDIAN_LE32(&header[4])+8);
                          header      += 12;
                          header_size += 12;
                          nWaitBytes   = 8;
                          #if !defined(__FREERTOS__)
                          or32_invalidate_cache(&header[8], 4);
                          #endif
                          readState = CHK_TYPE;
                          break;
        case CHK_TYPE   : if (!strncmp(&header[0], "fmt ", 4)) {
                                readState  = FMT_TYPE;
                                nWaitBytes = ENDIAN_LE32(&header[4]);
                          } else if(!strncmp(&header[0], "data", 4)) {
                                readState  = DATA_TYPE;
                                nWaitBytes = 0;
                                done       = 1;
                          } else if(!strncmp(&header[0], "fact", 4)) {
                                readState  = FACT_TYPE;
                                nWaitBytes = ENDIAN_LE32(&header[4]);
                          } else {
                                #if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                                    ithPrintf("[Wav] Do not support the chunt type '%c%c%c%c'\n", header[0], header[1], header[2], header[3]);
                                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
                                #else
                              PRINTF("Do not support the chunt type '%c%c%c%c'\n", header[0], header[1], header[2], header[3]);
                                #endif
                              return 0;
                          }
                          header += 8;
                          header_size += (8+nWaitBytes);
                          #if !defined(__FREERTOS__)
                          or32_invalidate_cache(&header[4], 4);
                          #endif
                          break;
        case FMT_TYPE   : readState = CHK_TYPE;
                          WaveInfo.format         = (WAVE_FORMAT)ENDIAN_LE16(&header[0]);
                          WaveInfo.nChans         = (unsigned int)header[2];
                          WaveInfo.sampRate       = (unsigned int)ENDIAN_LE32(&header[4]);
                          WaveInfo.avgBytesPerSec = (unsigned int)ENDIAN_LE32(&header[8]);
                          WaveInfo.blockAlign     = (unsigned int)ENDIAN_LE16(&header[12]);
                          WaveInfo.bitsPerSample  = (unsigned int)header[14];
                          if (nWaitBytes > 16) { // extension header
                              WaveInfo.samplesPerBlock = (unsigned int)ENDIAN_LE16(&header[18]);
                          } else {
                              WaveInfo.samplesPerBlock = 0;
                          }
                          #if !defined(__FREERTOS__)
                          or32_invalidate_cache(&header[nWaitBytes], 4);
                          #endif
                          header    += nWaitBytes;
                          nWaitBytes = 8;
                          break;
        case FACT_TYPE  : readState  = CHK_TYPE;
                          header    += nWaitBytes;
                          nWaitBytes = 8;
                          break;
        default         : break;
        }
    } // while(!done)

    PRINTF("WAVINFO: format %d (%s)\n", WaveInfo.format,
                                        WaveInfo.format == WAVE_FORMAT_ALAW  ? "aLaw"  :
                                        WaveInfo.format == WAVE_FORMAT_MULAW ? "uLaw"  :
                                        WaveInfo.format == WAVE_FORMAT_PCM   ? "PCM"   :
                                        WaveInfo.format == WAVE_FORMAT_DVI_ADPCM ? "ADPCM" : "Unknown");
    PRINTF("WAVINFO: header size %d\n", header_size);
    PRINTF("WAVINFO: nChans %d\n", WaveInfo.nChans);
    PRINTF("WAVINFO: sampRate %d\n", WaveInfo.sampRate);
    PRINTF("WAVINFO: avgBytesPerSec %d\n", WaveInfo.avgBytesPerSec);
    PRINTF("WAVINFO: blockAlign %d\n", WaveInfo.blockAlign);
    PRINTF("WAVINFO: bitsPerSample %d\n", WaveInfo.bitsPerSample);
    PRINTF("WAVINFO: samplesPerBlock %d\n", WaveInfo.samplesPerBlock);

    if (!(WaveInfo.format != 0 && WaveInfo.nChans >= 1 && WaveInfo.nChans <= 6 &&
          WaveInfo.sampRate > 0 && WaveInfo.sampRate <= 96000 && WaveInfo.bitsPerSample >= 4 &&
          WaveInfo.bitsPerSample <= 16) ||
        (WaveInfo.format == WAVE_FORMAT_DVI_ADPCM && WaveInfo.samplesPerBlock == 0)) 
    {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
        ithPrintf("[Wav]Unkown .wav parameter.\n");
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
        printf("[Wav]Unkown .wav parameter.\n");
#endif                                             

        return 0;
    }

    if (WaveInfo.format != WAVE_FORMAT_ALAW && WaveInfo.format != WAVE_FORMAT_MULAW &&
        WaveInfo.format != WAVE_FORMAT_PCM  && WaveInfo.format != WAVE_FORMAT_DVI_ADPCM)
    {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
        ithPrintf("[Wav]Unsupport WAV format 0x%x\n",WaveInfo.format);
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
        printf("[Wav]Unsupport WAV format \n");
#endif                                             
        return 0;
    }

    if (WaveInfo.format == WAVE_FORMAT_DVI_ADPCM)
    {
        wait_pcm_bytes = WaveInfo.samplesPerBlock * WaveInfo.nChans * (PCM_BIT_PERSAMPLE / 8);
        wait_wav_bytes = ((WaveInfo.samplesPerBlock - 1) / 2 + 4) * WaveInfo.nChans;
        ASSERT(wait_wav_bytes == WaveInfo.blockAlign);
        wavReadIdx+=48;
    } 
    else
    {
        if (WaveInfo.nChans<=2)
        {
            wait_pcm_bytes = WAIT_PCM_BYTES;
        }
        else
        {
            wait_pcm_bytes = (FRAME_SIZE * 2 * PCM_BIT_PERSAMPLE / 8);
        }
        wait_wav_bytes = WAIT_WAV_BYTES;
        wavReadIdx+=44;
    }
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
        ithPrintf("[Wav] Support WAV format 0x%x channels %d sample rate %d %d\n",WaveInfo.format,WaveInfo.nChans,WaveInfo.sampRate,wait_wav_bytes);
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
        printf("[Wav] Support WAV format \n");
#endif                                             

    return header_size;
}

/**************************************************************************************
 * Function     : DecFillWriteBuffer
 *
 * Description  : Wait the avaliable length of the output buffer bigger than on
 *                frame of PCM data.
 *
 * Inputs       :
 *
 * Global Var   : pcmWriteIdx: write index of output buffer.
 *                pcmReadIdx : read index of output buffer.
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The buffer is a circular queue.
 *
 **************************************************************************************/
static int DecFillWriteBuffer(int nPCMBytes) {
    int len;

    // Update Write Buffer
    if (nPCMBytes > 0) {
        pcmWriteIdx = pcmWriteIdx + nPCMBytes;
        if (pcmWriteIdx >= sizeof(pcmWriteBuf))
            pcmWriteIdx -= sizeof(pcmWriteBuf);
        //SetOutWrPtr(pcmWriteIdx);
        CODEC_I2S_SET_OUTWR_PTR(pcmWriteIdx);
        wavFrameAccu += (wavFrameStep & 0x7fff);
        decTime = decTime + (wavFrameStep >> 15) + (wavFrameAccu >> 15);
        wavFrameAccu = wavFrameAccu & 0x7fff;
        MMIO_Write(DrvDecode_Frame  , (short)( ((unsigned int)decTime) & 0xffff));
        MMIO_Write(DrvDecode_Frame+2, (short)( ((unsigned int)decTime) >> 16)   );
    }

    // Wait output buffer avaliable
    do {
        checkControl();
        pcmReadIdx = CODEC_I2S_GET_OUTRD_PTR();

        len = getFreeLen(pcmReadIdx, pcmWriteIdx, sizeof(pcmWriteBuf));
        if (len < (wait_pcm_bytes+2) && !isSTOP()) {
            #if defined(__FREERTOS__)
            PalSleep(2);
            #else
            //or32_delay(1); // enter sleep mode for power saving
            #endif
        } else {
            break;
        }

        // PRINTF("Wait I2S %d bytes, current is %d bytes. rd(%d) wr(%d)\n", wait_pcm_bytes, len, pcmReadIdx, pcmWriteIdx);
    } while(1);

    PRINTF("pcmWriteIdx(%d) pcmReadIdx(%d) len(%d) nPCMBytes(%d)\n", pcmWriteIdx, pcmReadIdx, len, nPCMBytes);

    return len;
}

/**************************************************************************************
 * Function     : DecFillReadBuffer
 *
 * Description  : Update the read pointer of WAVE Buffer and return the valid data length
 *                of input buffer.
 *
 * Inputs       : nReadBytes: number of bytes will read
 *
 * Global var   : wavWriteIdx: write pointer of WAVE buffer
 *                wavReadIdx : read pointer of WAVE buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The WAVE buffer is circular buffer.
 *
 **************************************************************************************/
static int DecFillReadBuffer(int nReadBytes) {
    int len = 0;

    // Update Read Buffer
    if (nReadBytes > 0)
    {
        PRINTF("nReadBytes: %d\n", nReadBytes);
        wavReadIdx = wavReadIdx + nReadBytes;
        if (wavReadIdx >= sizeof(streamBuf)) {
            wavReadIdx -= sizeof(streamBuf);
        }
        MMIO_Write(DrvDecode_RdPtr, ((wavReadIdx >> 1) << 1));

        // It should be invalidate the cache line which it is in the
        // begin of input buffer. The previous data will
        // prefetch to the cache line, but the driver dose
        // not yet put it in the input buffer. It will cause the
        // unconsistency of cache.
/*
        #if !defined(__FREERTOS__)
        or32_invalidate_cache(&streamBuf[wavReadIdx], 4);
        #endif
*/        
    }
#if defined(__OPENRTOS__)
    //dc_invalidate(); // Flush Data Cache
#endif

    // Wait Read Buffer avaliable
    do
    {
        checkControl();
        wavWriteIdx = getStreamWrPtr();

        len = getUsedLen(wavReadIdx, wavWriteIdx, sizeof(streamBuf));
        if ((!eofReached) && (len < wait_wav_bytes))
        {
            #if defined(__FREERTOS__)
            PalSleep(2);
            #else
            //or32_delay(1); // enter sleep mode for power saving
            #endif
        }
        else
        {
            break;
        }
        PRINTF("Wait buffer %d bytes, current is %d bytes. rd(%d) wr(%d)\n", wait_wav_bytes, len, wavReadIdx, wavWriteIdx);
    } while(1);

    PRINTF("wavWriteIdx(%d) wavReadIdx(%d) len(%d) nReadBytes(%d)\n", wavWriteIdx, wavReadIdx, len, nReadBytes);

    return len;
}

/**************************************************************************************
 * Function     : DecClearRdBuffer
 *
 * Description  : Reset read buffer's read/write pointer
 *
 * Inputs       : None
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 **************************************************************************************/
static void DecClearRdBuffer(void) {
    #if defined(OUTPUT_MEMMODE)
    return;
    #endif

    if (isEOF() && !isSTOP()) 
    {
        #if defined(DUMP_PCM_DATA)
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) | DrvDecodePCM_EOF);
        #endif // defined(DUMP_PCM_DATA)

        // wait I2S buffer empty
        do
        {
            if (1) //(CODEC_I2S_GET_OUTRD_PTR() == GetOutWrPtr()) 
            {
                break;
            }
            else
            {
                #if defined(__FREERTOS__)
                PalSleep(2);
                #else
                //or32_delay(1); // enter sleep mode for power saving
                #endif
            }
        } while(1);

        #if defined(DUMP_PCM_DATA)
        while(MMIO_Read(DrvAudioCtrl) & DrvDecodePCM_EOF) {
            #if defined(__FREERTOS__)
            PalSleep(2);
            #else
            //or32_delay(1);
            #endif
        }
        #endif // defined(DUMP_PCM_DATA)
    }

    MMIO_Write(DrvDecode_WrPtr, 0);
    MMIO_Write(DrvDecode_RdPtr, 0);

    #if !defined(WIN32) && !defined(__CYGWIN__)
    MMIO_Write(DrvDecode_Frame  , 0);
    MMIO_Write(DrvDecode_Frame+2, 0);
    #endif // !defined(WIN32) && !defined(__CYGWIN__)

    #if defined(DUMP_PCM_DATA)
    SetOutWrPtr(0);
    SetOutRdPtr(0);
    #endif // !defined(DUMP_PCM_DATA)

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    //AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_DAC);
#else
    deactiveDAC();   // Disable I2S interface
#endif
    if (isEOF()) {
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvDecode_EOF);
    }

    if (isSTOP()) {
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvDecode_STOP);
    }


#if defined(__FREERTOS__)
    dc_invalidate(); // Flush DC Cache
#endif

    #if defined(WIN32) || defined(__CYGWIN__)
    exit(-1);
    #endif
}

/**************************************************************************************
 * Function     : wavDecodeInit
 *
 * Description  :
 *
 * Inputs       :
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         :
 *
 **************************************************************************************/
static int WavDecodeInit(void)
{
    unsigned int ctrl = (unsigned int)MMIO_Read(DrvAudioCtrl2);

    nDecEndian = (ctrl & DrvPCM_DecEndian) ? 1 : 0;    
    nShowSpectrum = (ctrl & DrvShowSpectrum) ? 1 : 0;
    return 0;
}

static int WavGetDecEndian(void)
{
    unsigned int ctrl = (unsigned int)MMIO_Read(DrvAudioCtrl2);
    nDecEndian = (ctrl & DrvPCM_DecEndian) ? 1 : 0;    
    return nDecEndian;
}


/**************************************************************************************
 * Function     : wavDecode
 *
 * Description  :
 *
 * Inputs       :
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         :
 *
 **************************************************************************************/
static int wavDecode(int size)
{
    switch(WaveInfo.format) {
        case WAVE_FORMAT_ALAW      : alawDecode(size); break;
        case WAVE_FORMAT_MULAW     : ulawDecode(size); break;
        case WAVE_FORMAT_PCM       : pcmDecode(size); break;
        case WAVE_FORMAT_DVI_ADPCM :
            adpcm_decoder(size, &state);
            WaveInfo.format = WAVE_FORMAT_DVI_ADPCM;
            break;
        default                    :
            break;
    }

    return 0;
}

/**************************************************************************************
 * Function     : Decoder
 *
 * Description  :
 *
 * Inputs       :
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         :
 *
 **************************************************************************************/
static int WAVDecoder(void) {
    int bytesLeft;
    int nPCMBytes;
    int nReadBytes;
    int lastRound;
    int m, temp;

    #if defined(ENABLE_PERFORMANCE_MEASURE)
    int nFrames = 0;
    #endif

    wavReadIdx  = 0;
    wavWriteIdx = 0;
    pcmWriteIdx = 0;
    pcmReadIdx  = 0;
    bytesLeft   = 0;
    eofReached  = 0;
    nPCMBytes   = 0;
    nReadBytes  = 0;
    decTime     = 0;
    headerErr   = 0;
    lastRound   = 0;
    wavFrameAccu= 0;

    MMIO_Write(DrvDecode_Frame  , 0);
    MMIO_Write(DrvDecode_Frame+2, 0);

    WavDecodeInit();

    memset((void*)&state, 0, sizeof(struct adpcm_state));

    nReadBytes = ParseWaveHeader();
    if (eofReached)
    {
        DecClearRdBuffer();
        return -1;
    }

    if (nReadBytes == 0)
    {
        headerErr = 1;
        #if defined(WIN32) || defined(__CYGWIN__)
        printf("Unknown Format\n");
        exit(-1);
        #else
            #if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                ithPrintf("[Wav] Unknown Format \n");
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
            #else
                printf("Unknown Format\n");
            #endif               

            #if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
                gCh = 0;
                gSampleRate = 0;
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC);
            #else
 
            #endif        
            
        return 0;
        //asm volatile("l.trap 15");
        #endif
    }
    else
    {
        int frame_size;
        headerErr = 0;
        frame_size = (WaveInfo.format == WAVE_FORMAT_DVI_ADPCM) ? WaveInfo.samplesPerBlock : FRAME_SIZE;
        unsigned long long n = ((unsigned long long)frame_size) << 31;
        wavFrameStep = (unsigned int)(n / WaveInfo.sampRate);
    }

#if !defined(DUMP_PCM_DATA)
        if (WaveInfo.nChans<=2)
        {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)            
            gCh = WaveInfo.nChans;
            gSampleRate = WaveInfo.sampRate;
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC);
#else
            initDAC((unsigned char*)pcmWriteBuf, WaveInfo.nChans, WaveInfo.sampRate, sizeof(pcmWriteBuf), 0);
#endif        

        }
        else if (WaveInfo.nChans>2 && WaveInfo.nChans<=6)
        {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)            
            gCh = 2;
            gSampleRate = WaveInfo.sampRate;
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC);
#else
            initDAC((unsigned char*)pcmWriteBuf, 2, WaveInfo.sampRate, sizeof(pcmWriteBuf), 0);
#endif                
        }
#endif

    if(nShowSpectrum)
    {
        freqInfoInitialize(WAIT_PCM_BYTES,0);
    }

#ifdef WAV_SPECTREM_PERFORMANCE_TEST
    nSpectrumPerformance = 0;
#endif    

    do
    { // WAVE Decode Loop
        #if defined(ENABLE_PERFORMANCE_MEASURE)
        start_timer();
        #endif

        // Updates the read buffer and returns the avaliable size
        // of input buffer. Wait a minimun FRAME_SIZE length.
        bytesLeft = DecFillReadBuffer(nReadBytes);

        /* Support Ensky request from stereo to mono by Viola Lee in 2009.01.12
           New left channel data = New right channel data = (Old left channel data + Old right channel data)/2 */
        if( isDownSample() && WaveInfo.nChans == 2)
        {
            short *pcm = (short*)&pcmWriteBuf[pcmWriteIdx];
            for(m = 0; m < nPCMBytes/sizeof(short); m += 2)
            {
                // pcm[m+1] = pcm[m] = (pcm[m] + pcm[m+1])/2;
                temp = (pcm[m] + pcm[m+1])/2;
                pcm[m] = temp;
                pcm[m+1] = temp;
            }
        }

    // Byte swap to little endian
    {
        if( WavGetDecEndian()==1)
        {
            int i;
            char *buf = (char *)&pcmWriteBuf[pcmWriteIdx];
            char *in  = (char *)&pcmWriteBuf[pcmWriteIdx];
            for(i=0; i<nPCMBytes; i+=sizeof(short))
            {
                buf[i]   = in[i+1];
                buf[i+1] = in[i];
            }
        }
    }

   #ifdef WAV_SPECTREM_PERFORMANCE_TEST
        tClockPerformance = PalGetClock();
   #endif
    if(nShowSpectrum)
    {
        updateFreqInfo(nPCMBytes);
    }

    #ifdef WAV_SPECTREM_PERFORMANCE_TEST
       //printf("Wav sampling rate %d  bytes %d duration %d \n",WaveInfo.sampRate,nPCMBytes,PalGetDuration(tClockPerformance));
   #endif

//        DecFillWriteBuffer(nPCMBytes);

        if (isSTOP() || lastRound)
        {
            break;
        }
        // Only support bitsPerSample == 4, 8, or 16
        nReadBytes = MIN(bytesLeft, wait_wav_bytes);
        if (WaveInfo.format == WAVE_FORMAT_DVI_ADPCM)
        {
            nPCMBytes = (nReadBytes - (4*WaveInfo.nChans)) * PCM_BIT_PERSAMPLE / WaveInfo.bitsPerSample +
                        WaveInfo.nChans*(PCM_BIT_PERSAMPLE/8);
            if (nReadBytes <= 4*WaveInfo.nChans)
            {
                nPCMBytes = 0;
            }
        }
        else
        {   
            if (WaveInfo.nChans<=2)
            {
                nPCMBytes = nReadBytes * PCM_BIT_PERSAMPLE / WaveInfo.bitsPerSample;
            }
            else if (WaveInfo.nChans>2 && WaveInfo.nChans<=6)
            {
                nPCMBytes = (nReadBytes * PCM_BIT_PERSAMPLE / WaveInfo.bitsPerSample)/(WaveInfo.nChans/2);                
            }
        }

        // Decode One Frames
        if (!headerErr)
        {
            wavDecode(nReadBytes);
        }

         DecFillWriteBuffer(nPCMBytes);

#ifdef WAV_RESET_DECODED_BYTE    
        if (isResetAudioDecodedBytes())
        {
            MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvResetAudioDecodedBytes);
            nWavDecodedByte = 0;
            MMIO_Write(DrvAudioDecodedBytes, nWavDecodedByte);    

            PalSleep(5);
        }                        
        // write wave decoded byte to register
        if (nReadBytes)
        {
            nWavDecodedByte += nReadBytes;
            MMIO_Write(DrvAudioDecodedBytes, nWavDecodedByte); 
            nKeepByte = 0;
        }
        else
        {
            //printf("[Mp3] StreamBuf rdptr %d <=  nKeepByte %d\n",StreamBuf.rdptr,nKeepByte);
        }
#endif  // def WAV_RESET_DECODED_BYTE       

         if(headerErr)
        {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
            ithPrintf("[Wav] headerErr\n");
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
            printf("[Wav] headerErr\n");
#endif                                                     
        }

//        PRINTF("nReadBytes(%d) nPCMBytes(%d) ", nReadBytes, nPCMBytes);
//        PRINTF("pcmWriteIdx(%d) pcmReadIdx(%d) ", pcmWriteIdx, pcmReadIdx);
//        PRINTF("wavWriteIdx(%d) wavReadIdx(%d)\n", wavWriteIdx, wavReadIdx);
 //       PRINTF("bytesLeft(%d)\n", bytesLeft);

        #if defined(__FREERTOS__)
//        PalSleep(2);
        #endif

//        checkControl();
//        if (isSTOP()) break;
//        if (isEOF() && bytesLeft <= 0) lastRound = 1;

        #if defined(ENABLE_PERFORMANCE_MEASURE)
        {
            int elapsed = get_timer();
            if (nFrames < (sizeof(time_log)/sizeof(int))) {
                time_log[nFrames++] = nReadBytes;
                time_log[nFrames++] = elapsed/nReadBytes;
            }
            //printf("#%3d: %d cycles per sample (average of %d samples).\n", nFrames, elapsed/nReadBytes, nReadBytes);
        }
        #endif // defined(ENABLE_PERFORMANCE_MEASURE)
    } while (1);

    if(nShowSpectrum)
    {
        freqInfoTerminiate();
    }
#ifdef WAV_RESET_DECODED_BYTE    
    if (isResetAudioDecodedBytes())
    {
        MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvResetAudioDecodedBytes);
    }
    nWavDecodedByte = 0;
    MMIO_Write(DrvAudioDecodedBytes, nWavDecodedByte);                            
#endif  // def WAV_RESET_DECODED_BYTE        

    
    return 0;
}

#if defined(HAVE_ENCODE)
/**************************************************************************************
 * Function     : PackWaveHeader
 *
 * Description  : Pack the header & update the write pointer
 *
 * Inputs       : None
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 **************************************************************************************/
static int PackWaveHeader(void) {
    // Chunk ID "RIFF"
    streamBuf[0] = 'R';
    streamBuf[1] = 'I';
    streamBuf[2] = 'F';
    streamBuf[3] = 'F';

    // Chunk Size (4+n) (dont care)
    streamBuf[4] = 0;
    streamBuf[5] = 0;
    streamBuf[6] = 0;
    streamBuf[7] = 0;

    // WAVE ID
    streamBuf[8] = 'W';
    streamBuf[9] = 'A';
    streamBuf[10] = 'V';
    streamBuf[11] = 'E';

    // Chunk ID "fmt "
    streamBuf[12] = 'f';
    streamBuf[13] = 'm';
    streamBuf[14] = 't';
    streamBuf[15] = ' ';

    // Chunk Size: ADPCM is 20, others is 16
    streamBuf[16] = (WaveInfo.format == WAVE_FORMAT_DVI_ADPCM) ? 20 : 16;
    streamBuf[17] = 0;
    streamBuf[18] = 0;
    streamBuf[19] = 0;

    // Format Code (1->PCM, 6->aLaw, 7->uLaw)
    streamBuf[20] = (unsigned char)WaveInfo.format;
    streamBuf[21] = 0;

    // Channel
    streamBuf[22] = (unsigned char)WaveInfo.nChans;
    streamBuf[23] = 0;

    // Sampling rate
    streamBuf[24] = (unsigned char)((WaveInfo.sampRate >>  0) & 0xff);
    streamBuf[25] = (unsigned char)((WaveInfo.sampRate >>  8) & 0xff);
    streamBuf[26] = (unsigned char)((WaveInfo.sampRate >> 16) & 0xff);
    streamBuf[27] = (unsigned char)((WaveInfo.sampRate >> 24) & 0xff);

    // Data rate (dont care)
    streamBuf[28] = 0;
    streamBuf[29] = 0;
    streamBuf[30] = 0;
    streamBuf[31] = 0;

    // Data block size (dont care)
    streamBuf[32] = 0;
    streamBuf[33] = 0;

    // Bits per sample, (PCM16->16, PCM8/aLaw/uLaw->8, ADPCM->4)
    streamBuf[34] = (char)WaveInfo.bitsPerSample;
    streamBuf[35] = 0;

    if (WaveInfo.format == WAVE_FORMAT_DVI_ADPCM) { // ADPCM
        streamBuf[36] = 2;
        streamBuf[37] = 0;
        streamBuf[38] = 0;
        streamBuf[39] = 0;
        streamBuf[40] = 'd';
        streamBuf[41] = 'a';
        streamBuf[42] = 't';
        streamBuf[43] = 'a';
        streamBuf[44] = 0;
        streamBuf[45] = 0;
        streamBuf[46] = 0;
        streamBuf[47] = 0;
        return 48;
    } else {
        streamBuf[36] = 'd';
        streamBuf[37] = 'a';
        streamBuf[38] = 't';
        streamBuf[39] = 'a';
        streamBuf[40] = 0;
        streamBuf[41] = 0;
        streamBuf[42] = 0;
        streamBuf[43] = 0;
	return 44;
    }
}

/**************************************************************************************
 * Function     : EncFillReadBuffer
 *
 * Description  : Update the read pointer of PCM Buffer and return the valid data length
 *                of input buffer.
 *
 * Inputs       : nReadBytes: number of bytes will read
 *
 * Global var   : pcmWriteIdx: write pointer of PCM buffer
 *                pcmReadIdx : read pointer of PCM buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The PCM buffer is circular buffer.
 *
 **************************************************************************************/
static int EncFillReadBuffer(int nReadBytes) {
    int len;

    int i;
    // Update Read Buffer
    if (nReadBytes > 0)
    {
        pcmReadIdx = pcmReadIdx + nReadBytes;
        if (pcmReadIdx >= sizeof(pcmWriteBuf))
            pcmReadIdx -= sizeof(pcmWriteBuf);

        wavFrameAccu += (wavFrameStep & 0x7fff);
        decTime = decTime + (wavFrameStep >> 15) + (wavFrameAccu >> 15);
        wavFrameAccu = wavFrameAccu & 0x7fff;
        MMIO_Write(DrvEncode_Frame  , (short)( ((unsigned int)decTime) & 0xffff));
        MMIO_Write(DrvEncode_Frame+2, (short)( ((unsigned int)decTime) >> 16)   );

        SetInRdPtr(pcmReadIdx);

        // It should be invalidate the cache line which it is in the
        // begin of input buffer. The previous data will
        // prefetch to the cache line, but the IIS dose
        // not yet put it in the input buffer. It will cause the
        // unconsistency of cache.
        #if !defined(__FREERTOS__)
        or32_invalidate_cache(&pcmWriteBuf[pcmReadIdx], 4);
        #endif
    }

    // Wait input buffer avaliable
    do
    {
        checkControl();
        pcmWriteIdx = GetInWrPtr();

        len = getUsedLen(pcmReadIdx, pcmWriteIdx, sizeof(pcmWriteBuf));
        if (len < wait_pcm_bytes && !eofReached) {
            #if defined(__FREERTOS__)
            PalSleep(2);
            #else
            //or32_delay(1); // enter sleep mode for power saving
            #endif
        } else {
            break;
        }

        PRINTF("Wait I2S %d bytes, current is %d bytes. rd(%d) wr(%d)\n", wait_pcm_bytes, len, pcmReadIdx, pcmWriteIdx);
    } while(1);

    PRINTF("pcmWriteIdx(%d) pcmReadIdx(%d) len(%d) nReadBytes(%d)\n", pcmWriteIdx, pcmReadIdx, len, nReadBytes);

    return len;
}

/**************************************************************************************
 * Function     : EncFillWriteBuffer
 *
 * Description  : Wait the avaliable length of the output buffer bigger than one
 *                frame of WAV data.
 *
 * Inputs       :
 *
 * Global Var   : wavWriteIdx: write index of output buffer.
 *                wavReadIdx : read index of output buffer.
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The buffer is a circular queue.
 *
 **************************************************************************************/
static int EncFillWriteBuffer(int nWAVBytes) {
    int len = 0;

    // Update Write Buffer
    if (nWAVBytes > 0)
    {
        PRINTF("nWAVBytes: %d\n", nWAVBytes);
        wavWriteIdx = wavWriteIdx + nWAVBytes;
        if (wavWriteIdx >= sizeof(streamBuf)) {
            wavWriteIdx -= sizeof(streamBuf);
        }
        MMIO_Write(DrvEncode_WrPtr, ((wavWriteIdx >> 1) << 1));
    }

    // Wait Write Buffer avaliable
    do {
        //checkControl();
        wavReadIdx = getStreamRdPtr();

        len = getFreeLen(wavReadIdx, wavWriteIdx, sizeof(streamBuf));
        //if ((!eofReached) && (len < wait_wav_bytes)) {
        if (len < (wait_wav_bytes+2)) {
            #if defined(__FREERTOS__)
            PalSleep(2);
            #else
            //or32_delay(1); // enter sleep mode for power saving
            #endif
        } else {
            break;
        }
        PRINTF("Wait buffer %d bytes, current is %d bytes. rd(%d) wr(%d)\n", wait_wav_bytes, len, wavReadIdx, wavWriteIdx);
    } while(1);

    PRINTF("wavWriteIdx(%d) wavReadIdx(%d) len(%d) nWAVBytes(%d)\n", wavWriteIdx, wavReadIdx, len, nWAVBytes);

    return len;
}

/**************************************************************************************
 * Function     : EncClearRdBuffer
 *
 * Description  : Reset read buffer's read/write pointer
 *
 * Inputs       : None
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 **************************************************************************************/
static void EncClearRdBuffer(void) {
    #if defined(OUTPUT_MEMMODE)
    return;
    #endif

    if (isEOF() && !isSTOP()) {
        #if defined(DUMP_PCM_DATA)
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) | DrvEncodePCM_EOF);
        #endif // defined(DUMP_PCM_DATA)

        // wait Output buffer empty
        do {
            if (MMIO_Read(DrvEncode_WrPtr) == (wavReadIdx = MMIO_Read(DrvEncode_RdPtr))) {
                break;
            } else {
                #if defined(__FREERTOS__)
                PalSleep(2);
                #else
                //or32_delay(1); // enter sleep mode for power saving
                #endif
            }
        } while(1);

        #if defined(DUMP_PCM_DATA)
        while(MMIO_Read(DrvAudioCtrl) & DrvEncodePCM_EOF) {
            #if defined(__FREERTOS__)
            PalSleep(2);
            #else
            //or32_delay(1);
            #endif
        }
        #endif // defined(DUMP_PCM_DATA)
    }

    MMIO_Write(DrvEncode_WrPtr, 0);
    MMIO_Write(DrvEncode_RdPtr, 0);

    #if !defined(WIN32) && !defined(__CYGWIN__)
    MMIO_Write(DrvEncode_Frame  , 0);
    MMIO_Write(DrvEncode_Frame+2, 0);
    #endif // !defined(WIN32) && !defined(__CYGWIN__)

    #if defined(DUMP_PCM_DATA)
    SetInWrPtr(0);
    SetInRdPtr(0);
    #endif // !defined(DUMP_PCM_DATA)

    if (isEOF()) {
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvEncode_EOF);
    }

    if (isSTOP()) {
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvEncode_STOP);
    }
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_ADC);
#else
    deactiveADC();   // Disable I2S interface
#endif

    #if defined(__FREERTOS__)
    dc_invalidate(); // Flush DC Cache
    #endif

    #if defined(WIN32) || defined(__CYGWIN__)
    exit(-1);
    #endif
}

/**************************************************************************************
 * Function     : wavEncodeInit
 *
 * Description  :
 *
 * Inputs       :
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         :
 *
 **************************************************************************************/
static int wavEncodeInit(void)
{
    unsigned int ctrl = (unsigned int)MMIO_Read(DrvAudioCtrl2);

    nShowSpectrum = (ctrl & DrvShowSpectrum) ? 1 : 0;

    WaveInfo.nChans = (ctrl & DrvWAV_EncChannel) ? 2 : 1;
    WaveInfo.sampRate = SampsRate[(ctrl & DrvWAV_EncSampRate) >> DrvWAV_EncSampRateBits];

    switch ((ctrl & DrvWAV_Type) >> DrvWAV_TypeBits) {
        case DrvWAV_TypePCM16 : WaveInfo.format = WAVE_FORMAT_PCM;
                                WaveInfo.bitsPerSample = 16;
                                break;
        case DrvWAV_TypePCM8  : WaveInfo.format = WAVE_FORMAT_PCM;
                                WaveInfo.bitsPerSample = 8;
                                break;
        case DrvWAV_TypeADPCM : WaveInfo.format = WAVE_FORMAT_DVI_ADPCM;
                                WaveInfo.bitsPerSample = 4;
                                break;
        case DrvWAV_TypeALAW  : WaveInfo.format = WAVE_FORMAT_ALAW;
                                WaveInfo.bitsPerSample = 8;
                                break;
        case DrvWAV_TypeULAW  : WaveInfo.format = WAVE_FORMAT_MULAW;
                                WaveInfo.bitsPerSample = 8;
                                break;
        default:                return -1;
    }

    if (WaveInfo.format == WAVE_FORMAT_DVI_ADPCM)
    {
        WaveInfo.samplesPerBlock = (256 * WaveInfo.nChans * ((WaveInfo.sampRate <= 11000) ? 1 : (WaveInfo.sampRate/11000)) - 4 * WaveInfo.nChans) * 8 /
                                   (WaveInfo.bitsPerSample * WaveInfo.nChans) + 1;
        wait_pcm_bytes = WaveInfo.samplesPerBlock * WaveInfo.nChans * (PCM_BIT_PERSAMPLE / 8);
        wait_wav_bytes = ((WaveInfo.samplesPerBlock - 1) / 2 + 4) * WaveInfo.nChans;

        //printf(" adpcm wait_pcm_bytes %d wait_wav_bytes %d \n",wait_pcm_bytes,wait_wav_bytes);

    }
    else
    {
        wait_pcm_bytes = WAIT_PCM_BYTES;
        wait_wav_bytes = WAIT_WAV_BYTES;
        WaveInfo.samplesPerBlock = 0;
    }

    return 0;
}

/**************************************************************************************
 * Function     : wavEncode
 *
 * Description  :
 *
 * Inputs       :
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         :
 *
 **************************************************************************************/
static int wavEncode(int size)
{
    switch(WaveInfo.format) {
        case WAVE_FORMAT_ALAW      : alawEncode(size); break;
        case WAVE_FORMAT_MULAW     : ulawEncode(size); break;
        case WAVE_FORMAT_PCM       :
            pcmEncode(size);
            break;
        case WAVE_FORMAT_DVI_ADPCM : adpcm_coder(size, &state); break;
        default                    : return -1;
    }

    return 0;
}

/**************************************************************************************
 * Function     : Encoder
 *
 * Description  :
 *
 * Inputs       :
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         :
 *
 **************************************************************************************/
static int WAVEncoder(void) {
    int bytesLeft;
    int nWAVBytes;
    int nReadBytes;

    #if defined(ENABLE_PERFORMANCE_MEASURE)
    int nFrames = 0;
    #endif

    wavReadIdx  = 0;
    wavWriteIdx = 0;
    pcmWriteIdx = 0;
    pcmReadIdx  = 0;
    bytesLeft   = 0;
    eofReached  = 0;
    nWAVBytes   = 0;
    nReadBytes  = 0;
    decTime     = 0;
    wavFrameAccu= 0;

    MMIO_Write(DrvEncode_Frame  , 0);
    MMIO_Write(DrvEncode_Frame+2, 0);

    memset((void*)&state, 0, sizeof(struct adpcm_state));
    // Get Wave Header
    wavEncodeInit();
    // Write Wave Header
    //nWAVBytes = PackWaveHeader();

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)            
        gCh = 2;
        gSampleRate = WaveInfo.sampRate;
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_ADC);
#else
        initADC((unsigned char*)pcmWriteBuf, WaveInfo.nChans, WaveInfo.sampRate, sizeof(pcmWriteBuf), 0);
#endif

    if(nShowSpectrum)
    {
        freqInfoInitialize(WAIT_PCM_BYTES,1);
    }
    
    {
        int frame_size;
        frame_size = (WaveInfo.format == WAVE_FORMAT_DVI_ADPCM) ? WaveInfo.samplesPerBlock : FRAME_SIZE;
        unsigned long long n = ((unsigned long long)frame_size) << 31;
        wavFrameStep = (unsigned int)(n / WaveInfo.sampRate);
    }

    do
    { // WAVE Encode Loop
        #if defined(ENABLE_PERFORMANCE_MEASURE)
        start_timer();
        #endif

        // Updates the read buffer and returns the avaliable size of input buffer.
        bytesLeft = EncFillReadBuffer(nReadBytes);
       
        EncFillWriteBuffer(nWAVBytes);

        #if defined(WIN32) || defined(__CYGWIN__)
        if (bytesLeft == 0 && (isSTOP() || isEOF())) break;
        #else
            if (isSTOP() || isEOF())
                break;
        #endif

        // Only support bitsPerSample == 4, 8, or 16
        nReadBytes = MIN(bytesLeft, wait_pcm_bytes);
        nWAVBytes  = nReadBytes * WaveInfo.bitsPerSample / PCM_BIT_PERSAMPLE;

        if(nShowSpectrum)
        {
            updateFreqInfo(nReadBytes);
        }

        // Encode One Frame
        wavEncode(nReadBytes);

        PRINTF("nReadBytes(%d) nWAVBytes(%d) ", nReadBytes, nWAVBytes);
        PRINTF("pcmWriteIdx(%d) pcmReadIdx(%d) ", pcmWriteIdx, pcmReadIdx);
        PRINTF("wavWriteIdx(%d) wavReadIdx(%d)\n", wavWriteIdx, wavReadIdx);
        PRINTF("bytesLeft(%d)\n", bytesLeft);

        #if defined(__FREERTOS__)
        PalSleep(2);
        #endif

        #if defined(ENABLE_PERFORMANCE_MEASURE)
        {
            int elapsed = get_timer();
            if (nFrames < (sizeof(time_log)/sizeof(int))) {
                time_log[nFrames++] = nReadBytes;
                time_log[nFrames++] = elapsed/nReadBytes;
            }
        }
        #endif // defined(ENABLE_PERFORMANCE_MEASURE)
    } while (1);

    if(nShowSpectrum)
    {
        freqInfoTerminiate();
    }

    return 0;
}
#endif


/**************************************************************************************
 * Function     : AudioPluginAPI
 *
 * Description  : AudioPluginAPI
 *
 * Input        : plugin type
 *
 * Output       : None
 *
 * Note         :
 **************************************************************************************/
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
static void output_and_count(struct parameters *p, int c)
{
    if (p->number_of_output_chars >= 0)
    {
        int n = c;
        gBuf[gPrint++] = c;
        if (n >= 0)
            p->number_of_output_chars++;
        else
            p->number_of_output_chars = n;
    }
}

static void output_field(struct parameters *p, char *s)
{
    short justification_length =
        p->minimum_field_width - p->leading_zeros - p->edited_string_length;
    if (p->options & MINUS_SIGN)
    {
        if (p->options & ZERO_PAD)
            output_and_count(p, '-');
        justification_length--;
    }
    if (p->options & RIGHT_JUSTIFY)
        while (--justification_length >= 0)
            output_and_count(p, p->options & ZERO_PAD ? '0' : ' ');
    if (p->options & MINUS_SIGN && !(p->options & ZERO_PAD))
        output_and_count(p, '-');
    while (--p->leading_zeros >= 0)
        output_and_count(p, '0');
    while (--p->edited_string_length >= 0){
        output_and_count(p, *s++);
    }
    while (--justification_length >= 0)
        output_and_count(p, ' ');
}

int ithGPrintf(const char *control_string, va_list va)/*const int *argument_pointer)*/
{
    struct parameters p;
    char              control_char;
    p.number_of_output_chars = 0;
    control_char             = *control_string++;
    
    while (control_char != '\0')
    {
        if (control_char == '%')
        {
            short precision     = -1;
            short long_argument = 0;
            short base          = 0;
            control_char          = *control_string++;
            p.minimum_field_width = 0;
            p.leading_zeros       = 0;
            p.options             = RIGHT_JUSTIFY;
            if (control_char == '-')
            {
                p.options    = 0;
                control_char = *control_string++;
            }
            if (control_char == '0')
            {
                p.options   |= ZERO_PAD;
                control_char = *control_string++;
            }
            if (control_char == '*')
            {
                //p.minimum_field_width = *argument_pointer++;
                control_char          = *control_string++;
            }
            else
            {
                while ('0' <= control_char && control_char <= '9')
                {
                    p.minimum_field_width =
                        p.minimum_field_width * 10 + control_char - '0';
                    control_char = *control_string++;
                }
            }
            if (control_char == '.')
            {
                control_char = *control_string++;
                if (control_char == '*')
                {
                    //precision    = *argument_pointer++;
                    control_char = *control_string++;
                }
                else
                {
                    precision = 0;
                    while ('0' <= control_char && control_char <= '9')
                    {
                        precision    = precision * 10 + control_char - '0';
                        control_char = *control_string++;
                    }
                }
            }
            if (control_char == 'l')
            {
                long_argument = 1;
                control_char  = *control_string++;
            }
            if (control_char == 'd')
                base = 10;
            else if (control_char == 'x')
                base = 16;
            else if (control_char == 'X')
            {
                base       = 16;
                p.options |= CAPITAL_HEX;
            }
            else if (control_char == 'u')
                base = 10;
            else if (control_char == 'o')
                base = 8;
            else if (control_char == 'b')
                base = 2;
            else if (control_char == 'c')
            {
                base       = -1;
                p.options &= ~ZERO_PAD;
            }
            else if (control_char == 's')
            {
                base       = -2;
                p.options &= ~ZERO_PAD;
            }
            if (base == 0) /* invalid conversion type */
            {
                if (control_char != '\0')
                {
                    output_and_count(&p, control_char);
                    control_char = *control_string++;
                }
            }
            else
            {
                if (base == -1) /* conversion type c */
                {
                    //char c = *argument_pointer++;
                    char c = (char)(va_arg(va, int));
                    p.edited_string_length = 1;
                    output_field(&p, &c);
                }
                else if (base == -2) /* conversion type s */
                {
                    char *string;
                    p.edited_string_length = 0;
                    //string                 = *(char **) argument_pointer;
                    //argument_pointer      += sizeof(char *) / sizeof(int);
                    string = va_arg(va, char*);
                    while (string[p.edited_string_length] != 0)
                        p.edited_string_length++;
                    if (precision >= 0 && p.edited_string_length > precision)
                        p.edited_string_length = precision;
                    output_field(&p, string);
                }
                else /* conversion type d, b, o or x */
                {
                    unsigned long x;
                    char          buffer[BITS_PER_BYTE * sizeof(unsigned long) + 1];
                    p.edited_string_length = 0;
                    if (long_argument)
                    {
                        //x                 = *(unsigned long *) argument_pointer;
                        //argument_pointer += sizeof(unsigned long) / sizeof(int);
                        va_arg(va, unsigned int);
                    }
                    else if (control_char == 'd')
                        //x = (long) *argument_pointer++;
                        x = va_arg(va, long);
                    else
                        //x = (unsigned) *argument_pointer++;
                        x = va_arg(va, int);
                    if (control_char == 'd' && (long) x < 0)
                    {
                        p.options |= MINUS_SIGN;
                        x          = -(long) x;
                    }
                    do
                    {
                        int c;
                        c = x % base + '0';
                        if (c > '9')
                        {
                            if (p.options & CAPITAL_HEX)
                                c += 'A' - '9' - 1;
                            else
                                c += 'a' - '9' - 1;
                        }
                        buffer[sizeof(buffer) - 1 - p.edited_string_length++] = c;
                    } while ((x /= base) != 0);
                    if (precision >= 0 && precision > p.edited_string_length)
                        p.leading_zeros = precision - p.edited_string_length;
                    output_field(&p, buffer + sizeof(buffer) - p.edited_string_length);
                }
                control_char = *control_string++;
            }
        }
        else
        {
            output_and_count(&p, control_char);
            control_char = *control_string++;
        }
    }
    return p.number_of_output_chars;
}

int ithPrintf(char* control, ...)
{
    va_list va;
    va_start(va,control);
    gPrint = 0;
    gBuf = (unsigned char*)gtAudioPluginBuffer;
    ithGPrintf(control, va);
    va_end(va);    
    return 0;
}
void AudioPluginAPI(int nType)
{
    unsigned short nRegister;
    int i;
    int nTemp,nTemp1;
    unsigned char* pBuf;
    
    nRegister = (SMTK_AUDIO_PROCESSOR_ID<<14) | nType;
    switch (nType)
    {
        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_OPEN:
            gnTemp = sizeof(tDumpWave);
            //printf("[Wav] name length %d \n",gnTemp);
            memcpy(&gtAudioPluginBuffer[0], &gnTemp, SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH);                           
            memcpy(&gtAudioPluginBuffer[SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH/sizeof(short)], tDumpWave, sizeof(tDumpWave));
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_WRITE:
          // printf("[Wav] name length %d \n",gnTemp);           
            memcpy(&gtAudioPluginBuffer[0], &gnTemp, SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH);                           
            {
                int i;
                char tmp;
                char *buf = (char *)&pcmWriteBuf[pcmWriteIdx];
                for(i=0; i<gnTemp; i+=2)
                {
                    tmp = buf[i];
                    buf[i] = buf[i+1];
                    buf[i+1] = tmp;
                }
            }           
            memcpy(&gtAudioPluginBuffer[SMTK_AUDIO_PLUGIN_CMD_FILE_WRITE_LENGTH/sizeof(short)], &pcmWriteBuf[pcmWriteIdx], gnTemp);           
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_CLOSE:
           
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC:
                nTemp  = pcmWriteBuf;
                nTemp1 = sizeof(pcmWriteBuf);
                pBuf = (unsigned char*)gtAudioPluginBuffer;
                memcpy(&pBuf[0], &nTemp, sizeof(int));
                memcpy(&pBuf[4], &gCh, sizeof(int));
                memcpy(&pBuf[8], &gSampleRate, sizeof(int));
                memcpy(&pBuf[12], &nTemp1, sizeof(int));
                //printf("[Wav] 0x%x %d %d %d \n",nTemp,gCh,gSampleRate,nTemp1);
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_ADC:
                nTemp  = pcmWriteBuf;
                nTemp1 = sizeof(pcmWriteBuf);
                pBuf = (unsigned char*)gtAudioPluginBuffer;
                memcpy(&pBuf[0], &nTemp, sizeof(int));
                memcpy(&pBuf[4], &gCh, sizeof(int));
                memcpy(&pBuf[8], &gSampleRate, sizeof(int));
                memcpy(&pBuf[12], &nTemp1, sizeof(int));            
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC:
                pBuf = (unsigned char*)gtAudioPluginBuffer;
                memcpy(&pBuf[0], &gPause, sizeof(int));
                //printf("[Wav] pause %d \n",gPause);
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_DAC:
        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_ADC: 
            break;
        
        case SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF:

            break;

        default:
            break;

    }
    setAudioPluginMessageStatus(nRegister);
    i=200000*20;
   
    do
    {
        nRegister = getAudioPluginMessageStatus();
        nRegister = (nRegister & 0xc000)>>14;
        if (nRegister== SMTK_MAIN_PROCESSOR_ID)
        {
            //printf("[Wav] get main procerror feedback \n");
            break;
        }
        i--;
    }while(i && !isSTOP());
    //if (i==0)
    //  printf("[Wav] audio api %d %d\n",i,nType);

}
#endif // defined(AUDIO_PLUGIN_MESSAGE_QUEUE)

/**************************************************************************************
 * Function     : main
 *
 * Description  :
 *
 * Inputs       :
 *
 * Outputs      :
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         :
 *
 **************************************************************************************/
#if defined(__FREERTOS__) && !defined(ENABLE_CODECS_PLUGIN)
portTASK_FUNCTION(wavdecode_task, params)
#else
int main(int argc, char **argv)
#endif
{
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    int* codecStream;
    AUDIO_CODEC_STREAM* audioStream;

    codecStream=CODEC_STREAM_START_ADDRESS;   
    //printf("[Wav] 0x%08x \n",*codecStream);
    audioStream = *codecStream;
    audioStream->codecStreamBuf = &streamBuf[0];
    audioStream->codecStreamLength =  sizeof(streamBuf);      
    //printf("[Wav] audioStream 0x%08x 0x%08x 0x%08x   \n",&audioStream,&audioStream->codecStreamBuf,&audioStream->codecStreamLength);
    gtAudioPluginBuffer = audioStream->codecAudioPluginBuf;
    gtAudioPluginBufferLength = audioStream->codecAudioPluginLength;
    //printf("[Wav] 0x%08x %d 0x%08x %d \n",audioStream->codecStreamBuf,audioStream->codecStreamLength ,audioStream->codecAudioPluginBuf,audioStream->codecAudioPluginLength);
    MMIO_Write(AUDIO_DECODER_START_FALG, 12);
#endif

    #if defined(WIN32) || defined(__CYGWIN__)
    GetParam(argc, argv);
    win32_init();
    #endif // defined(WIN32) || defined(__CYGWIN__)
    isEncode = 0;
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
        ithPrintf("[Wav] start 0x%x 0x%x 0x%x \n",&streamBuf[0],streamBuf[0],streamBuf[0+1]);
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
        printf("[wav] start \n");        
#endif

    while(1)
    {  // Forever Loop
        #if defined(HAVE_ENCODE)
        int mode = MMIO_Read(DrvAudioCtrl2) & DrvWAV_Mode;
        isEncode = (((mode & DrvWAV_Mode) >> DrvWAV_ModeBits) == DrvWAV_ENCODE);

        wait_pcm_bytes = -1;
        wait_wav_bytes = -1;

        if (!isEncode) 
        {
        #endif // HAVE_ENCODE
            if (WAVDecoder() < 0) continue;

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
        ithPrintf("[WAV DECODER] Stream Closed\n");
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
            PRINTF("[WAV DECODER] Stream Closed\n");
#endif

            DecClearRdBuffer();
            break;
        #if defined(HAVE_ENCODE)
        } else {
            if (WAVEncoder() < 0) continue;

            PRINTF("[WAV ENCODER] Stream Closed\n");
            EncClearRdBuffer();
        }
        #endif // HAVE_ENCODE

        // Only do once on WIN32 platform.
        #if defined(WIN32) || defined(__CYGWIN__)
        break;
        #endif
    } /* end of forever loop */

#if !defined(__FREERTOS__)
    return 0;
#endif
}

