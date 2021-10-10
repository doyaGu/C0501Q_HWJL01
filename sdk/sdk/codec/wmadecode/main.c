/**************************************************************************************
 * Source last modified: $Id: main.c,v 1.2 2007/10/19 17:10:30 $
 *
 * Copyright (c) 2006-2007 SMedia Tech. Corp., All Rights Reserved.
 *
 * WMA wrapped program
 **************************************************************************************/
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "asf.h"
#include "wmadec.h"
#include "wmafixed.h"
#include "freqinfo.h"

#if defined(FREQINFO)
extern char freqinfo[FREQINFOCNT];
#endif

#if defined(__CYGWIN__) || defined(WIN32)
#include <assert.h>
#include "wavfilefmt.h"
#include "win32.h"
#endif

#if defined(__OR32__)
#include "mmio.h"
//#include "sys.h"
//#include "or32.h"
#include "i2s.h"
//#include "engine.h"
#endif

#if defined(__FREERTOS__)
#   include "FreeRTOS.h"
#   include "task.h"
#endif

#if defined(__INPUT_CRC_CHECK__)
#include "crc32.h"
#endif

//#if defined(ENABLE_CODECS_PLUGIN)
#include "plugin.h"
//#endif

#define getUsedLen(rdptr, wrptr, len) (((wrptr) >= (rdptr)) ? ((wrptr) - (rdptr)) : ((len) - ((rdptr) - (wrptr))))

///////////////////////////////////////////////////////////////////////////
//                              Constant Definition
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//                              Globale Variable
///////////////////////////////////////////////////////////////////////////
static unsigned int decTime = 0; // Decoding time in seconds on S15.16 fixed point.
static unsigned int wmaFrameStep;
static unsigned int wmaFrameAccu;

unsigned int nFrames;

#if defined(WMA_FORWARD_CODEC)
unsigned int forward = 0;
unsigned int firstsync_after_forward = 0;
unsigned int forward_sync = 0;
unsigned int packet_flags_temp = 0;             // packet flag
unsigned int packet_property_temp = 0;          // packet property
#endif // WMA_FORWARD_CODEC

#if defined(WMA_LAG_OPTIMIZE)
unsigned int lag_optimize = 0;
#endif // WMA_LAG_OPTIMIZE

#ifdef WMA_PERFORMANCE_TEST
static PAL_CLOCK_T tClockPerformance;
static long nDecPerformance;  // avg of frame

#endif

#if defined(ENABLE_PERFORMANCE_MEASURE)
#  include "ticktimer.h"
    #define MAX_TIME_LOG 10000
    unsigned int time_log[MAX_TIME_LOG];
    int timer_start = 0;
#endif // defined(ENABLE_PERFORMANCE_MEASURE)

///////////////////////////////////////////////////////////////////////////
// Output PCM buffer
///////////////////////////////////////////////////////////////////////////
#if defined(__GNUC__)
static unsigned char pcmWriteBuf[I2SBUFSIZE/sizeof(short)] __attribute__ ((aligned(16), section (".sbss")));
#else
static unsigned char pcmWriteBuf[I2SBUFSIZE/sizeof(short)];
#endif
//static uint16_t decoded[BLOCK_MAX_SIZE * MAX_CHANNELS];
static unsigned int  pcmReadIdx;
static unsigned int  pcmWriteIdx;

static unsigned short emptyBuffer[4096];
///////////////////////////////////////////////////////////////////////////
// Input stream buffer
///////////////////////////////////////////////////////////////////////////
#if defined(INPUT_MEMMODE)
unsigned char streamBuf[] = {
#  include "wmafile.h"
};
#  undef  isEOF
#  define isEOF() 1
#else
#  if defined(__GNUC__)
unsigned char streamBuf[READBUF_SIZE] __attribute__ ((aligned(16), section (".sbss")));
#  else
unsigned char streamBuf[READBUF_SIZE];
#  endif // defined(__GNUC__)
#endif // defined(INPUT_MEMMODE)

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
static unsigned int* gtAudioPluginBufferLength;
static unsigned short* gtAudioPluginBuffer;
static int gnTemp;
static unsigned char tDumpWave[] = "C:/wma_dump.wav";
static int gCh;
static int gSampleRate;
int gPause = 0 ;
int gPrint = 0;
unsigned char *gBuf;
asf_waveformatex_t gWfx;

#endif
///////////////////////////////////////////////////////////////////////////
//                              Local Variable
///////////////////////////////////////////////////////////////////////////
static unsigned int wmaReadIdx;
static unsigned int wmaWriteIdx;
static unsigned int nWmaTemp;

/* Decode status */
static int lastRound;
static int eofReached;
static int nStop;
//static unsigned int nFrames;
static unsigned char gFrame[5000];
static int gFrameUse;
static int gWMA8 =0;
/* The output buffer containing the decoded samples (channels 0 and 1)
   BLOCK_MAX_SIZE is 2048 (samples) and MAX_CHANNELS is 2.
 */

/* NOTE: WMADecodeContext is 120152 bytes (on x86) */
static WMADecodeContext wmadec;

static short tUpSample[5000];

/* TAG metadata */
struct mp3entry id3;

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
/* Code for general_printf() */
#define BITS_PER_BYTE    8
#define MINUS_SIGN       1
#define RIGHT_JUSTIFY    2
#define ZERO_PAD         4
#define CAPITAL_HEX      8

struct parameters
{
    int number_of_output_chars;
    short minimum_field_width;
    char  options;
    short edited_string_length;
    short leading_zeros;
};
#endif        
///////////////////////////////////////////////////////////////////////////
//                              Function Decleration
///////////////////////////////////////////////////////////////////////////
/* Function decleration */
static __inline unsigned int getStreamWrPtr(void);
static int  AdjustBuffer(int waitNBytes);
//static int  FillReadBuffer(int nReadBytes);
static void FillWriteBuffer(int nPCMBytes);
static void ClearRdBuffer(void);
static void buffer_init(void);
static int  UpSampling(int nInputSampleRate,int nOutputSampleRate,int nSamples,int nChannels,short *pInputPcmbuf,short *pOutputPcmBuf,int nOutputBits);
static int GetUpSampleRate(int nInputSampleRate);
static int ParseWmaInfo(asf_waveformatex_t *wfx);
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
void AudioPluginAPI(int nType);
__inline int ithPrintf(char* control, ...);
#endif

/**************************************************************************************
 * Function     : updateTime
 *
 * Description  : update decoding time.
 *
 * Input        : None
 *
 * Output       : None
 *
 * Note         :
 **************************************************************************************/
static __inline void updateTime(void)
{

    wmaFrameAccu += (wmaFrameStep & 0x7fff);
    decTime = decTime + (wmaFrameStep >> 15) + (wmaFrameAccu >> 15);
    wmaFrameAccu = wmaFrameAccu & 0x7fff;

    MMIO_Write(DrvDecode_Frame  , (short)( ((unsigned int)decTime) & 0xffff));
    MMIO_Write(DrvDecode_Frame+2, (short)( ((unsigned int)decTime) >> 16)   );
}

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
static __inline void checkControl(void)
{
    static int curPause = 0;
    static int prePause = 0;
    do
    {
        eofReached  = isEOF();
        nStop = isSTOP();
        curPause = isPAUSE();
        if (!curPause)
        {  // Un-pause
            if (prePause)
            {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
                gPause = 0;
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC);
#else
                pauseDAC(0);
#endif
            }
            break;
        }
        else
        { // Pause
            if (!prePause && curPause) 
            {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
                gPause = 1;
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC);
#else
                pauseDAC(1);
#endif
            }
            #if defined(__FREERTOS__)
            PalSleep(2);
            #else
            //or32_delay(1); // delay 1ms
            #endif
        }
        prePause = curPause;
    } while(!eofReached && !nStop);
    prePause = curPause;
}

static int UpSampling(int nInputSampleRate,int nOutputSampleRate,int nSamples,int nChannels,short *pInputPcmbuf,short *pOutputPcmBuf,int nOutputBits)
{
    int i;
    int j;
    char tmp;
    short *pInput = pInputPcmbuf;
    short *pOutput = pOutputPcmBuf;
    if (nChannels == 1)
    {
        if (nOutputSampleRate/nInputSampleRate == 2)
        {
            // interpolation 2x
            for(i=0,j=0; i<nSamples; i+=2,j+=4)
            {
                if (i+1<nSamples)
                {
                    pOutput[j]     = pInput[i];
                    pOutput[j+1] = (pInput[i]+pInput[i+1])/2;
                    pOutput[j+2] = (pInput[i]+pInput[i+1])/2;
                    pOutput[j+3] = pInput[i+1];
                }
                else
                {
                    pOutput[j]     = pInput[i];
                    pOutput[j+1] = pInput[i];
                    pOutput[j+2] = pInput[i+1];
                    pOutput[j+3] = pInput[i+1];
                }
            }    
        }
        else if (nOutputSampleRate/nInputSampleRate == 4)
        {
            // interpolation 4x    
            for(i=0,j=0; i<nSamples; i+=2,j+=8)
            {
                pOutput[j]     = pInput[i];
                pOutput[j+1] = pInput[i];
                pOutput[j+2] = (pInput[i]+pInput[i+1])/2;
                pOutput[j+3] = (pInput[i]+pInput[i+1])/2;
                pOutput[j+4] = (pInput[i]+pInput[i+1])/2;
                pOutput[j+5] = (pInput[i]+pInput[i+1])/2;
                pOutput[j+6] = pInput[i+1];
                pOutput[j+7] = pInput[i+1];            
            }    
        }   

    }
    else if (nChannels ==2)
    {
        if (nOutputSampleRate/nInputSampleRate == 2)
        {
            // interpolation 2x
            for(i=0,j=0; i<nSamples; i+=4,j+=8)
            {
                if (i+3<nSamples)            
                {
                    pOutput[j]     = pInput[i];
                    pOutput[j+1] = pInput[i+1];
                    pOutput[j+2] = (pInput[i]+pInput[i+2])/2;
                    pOutput[j+3] = (pInput[i+1]+pInput[i+3])/2;
                    pOutput[j+4] = (pInput[i]+pInput[i+2])/2;
                    pOutput[j+5] = (pInput[i+1]+pInput[i+3])/2;
                    pOutput[j+6] = pInput[i+2];
                    pOutput[j+7] = pInput[i+3];                                
                }
                else
                {
                    pOutput[j]     = pInput[i];
                    pOutput[j+1] = pInput[i+1];
                    pOutput[j+2] = pInput[i];
                    pOutput[j+3] = pInput[i+1];
                    pOutput[j+4] = pInput[i+2];
                    pOutput[j+5] = pInput[i+3];
                    pOutput[j+6] = pInput[i+2];
                    pOutput[j+7] = pInput[i+3];                                
                }
            }    
        }
        else if (nOutputSampleRate/nInputSampleRate == 4)
        {
            // interpolation 4x    
            for(i=0,j=0; i<nSamples; i+=4,j+=16)
            {
                pOutput[j]     = pInput[i];
                pOutput[j+1] = pInput[i+1];
                pOutput[j+2] = pInput[i];
                pOutput[j+3] = pInput[i+1];
                pOutput[j+4] = (pInput[i]+pInput[i+2])/2;
                pOutput[j+5] = (pInput[i+1]+pInput[i+3])/2;
                pOutput[j+6] = (pInput[i]+pInput[i+2])/2;
                pOutput[j+7] = (pInput[i+1]+pInput[i+3])/2;     
                pOutput[j+8] = (pInput[i]+pInput[i+2])/2;
                pOutput[j+9] = (pInput[i+1]+pInput[i+3])/2;
                pOutput[j+10] =(pInput[i]+pInput[i+2])/2;
                pOutput[j+11] = (pInput[i+1]+pInput[i+3])/2;
                pOutput[j+12] = pInput[i+2];
                pOutput[j+13] = pInput[i+3];
                pOutput[j+14] = pInput[i+2];
                pOutput[j+15] = pInput[i+3];                            
            }    
        }   
    }

}

static int GetUpSampleRate(int nInputSampleRate)
{
    if ((nInputSampleRate > 45600) && (nInputSampleRate < 50400))
    {
        return 48000;
    }
    else if ((nInputSampleRate > 41895) && (nInputSampleRate < 46305))
    {
        return 44100;    
    }
    else if ((nInputSampleRate > 30400) && (nInputSampleRate < 33600))
    {
        return 32000;
    }
    else if ((nInputSampleRate > 22800) && (nInputSampleRate < 25200))
    {
        return 48000;        
    }
    else if ((nInputSampleRate > 20900) && (nInputSampleRate < 23100))
    {
        return 44100;    
    }
    else if ((nInputSampleRate > 15200) && (nInputSampleRate < 16800))
    {
        return 32000;   
    }
    else if ((nInputSampleRate > 11400) && (nInputSampleRate < 12600))
    {    
        if (isUpSampleOnly2x())
        {
            return 24000;        
        }
        else
        {
            return 48000;        
        }
    }
    else if ((nInputSampleRate > 10450) && (nInputSampleRate < 11550))
    {
        if (isUpSampleOnly2x())
        {
            return 22050;
        }
        else
        {
            return 44100;    
        }
    }
    else if ((nInputSampleRate > 7600) && (nInputSampleRate < 8400))
    {
        if (isUpSampleOnly2x())
        {
            return 16000;
        }
        else
        {
            return 32000;
        }
    }
    else
    {
        //printf("[Wma] unknown sample rate %d \n",nInputSampleRate);
        return 1;
    }                

}
/**************************************************************************************
 * Function     : FillWriteBuffer
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
 * Note         : Output buffer is a circular queue.
 *
 **************************************************************************************/
static void FillWriteBuffer(int nPCMBytes)
{
    int len;
    int nTemp;

    #if defined(__OUTPUT_CRC_CHECK__)
    {
        static int crc = 0;
        crc = crc32(&pcmWriteBuf[pcmWriteIdx], nPCMBytes);
        PRINTF("CRC: 0x%08x\n", crc);
    }
    #endif // defined(__OUTPUT_CRC_CHECK__)
    // Update Write Buffer
    if (nPCMBytes > 0) 
    {
        pcmWriteIdx = pcmWriteIdx + nPCMBytes;
        #if defined(OUTPUT_MEMMODE)
        if (pcmWriteIdx >= sizeof(pcmWriteBuf))
            pcmWriteIdx = sizeof(pcmWriteBuf);

        //SetOutWrPtr(pcmWriteIdx);
        CODEC_I2S_SET_OUTWR_PTR(pcmWriteIdx);
        return;
        #else
        if (pcmWriteIdx>sizeof(pcmWriteBuf))
        {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
            ithPrintf("[Wma] pcmWriteIdx %d > %d (pcmWriteBuf)\n",pcmWriteIdx,sizeof(pcmWriteBuf));
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
            printf("[Wma] pcmWriteIdx %d > %d (pcmWriteBuf)\n",pcmWriteIdx,sizeof(pcmWriteBuf));
#endif
        }
        if (pcmWriteIdx >= sizeof(pcmWriteBuf))
            pcmWriteIdx -= sizeof(pcmWriteBuf);

        //SetOutWrPtr(pcmWriteIdx);
        CODEC_I2S_SET_OUTWR_PTR(pcmWriteIdx);        
        #endif // defined(OUTPUT_MEMMODE)
    }

    // Wait output buffer avaliable
    do 
    {
        checkControl();
        if (nStop)
        {
            return; 
        }
        //pcmReadIdx = GetOutRdPtr();
        pcmReadIdx = CODEC_I2S_GET_OUTRD_PTR();
        
        if (pcmReadIdx <= pcmWriteIdx) 
        {
            len = sizeof(pcmWriteBuf) - (pcmWriteIdx - pcmReadIdx);
        }
        else
        {
            len = pcmReadIdx - pcmWriteIdx;
        }
        if (isUpSample())
        {
            nTemp = GetUpSampleRate(wmadec.sample_rate)/wmadec.sample_rate;
        }
        else
        {
            nTemp =1;
        }
        if ((len-2) < MAXFRAME*nTemp && !isSTOP()) 
        {
            #if defined(__FREERTOS__)
            PalSleep(2);
            #else
            //or32_delay(1); // enter sleep mode for power saving
            #endif
        }
        else
        {
            #if defined(WMA_ADD_CONTENT_SWITCH)
            if ((nFrames % 3 == 0) && (lag_optimize == 1))
            {
                lag_optimize = 0;
                PalSleep(2);
            }
            #endif // WMA_ADD_CONTENT_SWITCH
            break;
        }
    } while(1);

    //PRINTF("pcmWriteIdx(%d) pcmReadIdx(%d) len(%d) nPCMBytes(%d)\n", pcmWriteIdx, pcmReadIdx, len, nPCMBytes);
}

/**************************************************************************************
 * Function     : AdjustBuffer
 *
 * Description  : move the reset of data to the front of buffer.
 *
 * Inputs       : waitNBytes: number of bytes to read
 *
 * Global var   : wmaWriteIdx: write pointer of WMA buffer
 *                wmaReadIdx : read pointer of WMA buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The WMA buffer is circular buffer.
 *
 **************************************************************************************/
static int AdjustBuffer(int waitNBytes)
{
    int len;

    // It should be invalidate the cache line which it is in the
    // begin of input buffer. The previous frame will
    // prefetch the data to the cache line, but the driver dose
    // not yet put it in the input buffer. It will cause the
    // unconsistency of cache.
    #if !defined(__FREERTOS__)
    or32_invalidate_cache(&streamBuf[wmaReadIdx], 1);
    #endif

    #if defined(INPUT_MEMMODE)
    wmaWriteIdx = sizeof(streamBuf);
    if (wmaReadIdx + waitNBytes >= sizeof(streamBuf)) 
    {
        lastRound = 1;
        return sizeof(streamBuf) - wmaReadIdx;
    }
    else
    {
        return waitNBytes;
    }
    #endif
#if defined(__OPENRTOS__)
     dc_invalidate(); // Flush Data Cache
#endif

    if (wmaReadIdx + waitNBytes >= READBUF_SIZE)
    { // do memory move when exceed guard band
        // Check the buffer size
        if ((waitNBytes - (READBUF_SIZE - wmaReadIdx)) > READBUF_BEGIN) 
        {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
            ithPrintf("[WMA] AdjustBuffer waitNBytes %d wmaReadIdx %d wmaWriteIdx %d\n",waitNBytes,wmaReadIdx,wmaWriteIdx);
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
            printf("[WMA] AdjustBuffer waitNBytes %d wmaReadIdx %d wmaWriteIdx %d\n",waitNBytes,wmaReadIdx,wmaWriteIdx);            
#endif           
            #if defined(__OR32__)
            asm volatile("l.trap 15");
            #else
            //printf("Buffer underflow! the READBUF_BEGIN should large than %d, current is %d\n", waitNBytes - (READBUF_SIZE - wmaReadIdx), READBUF_BEGIN);
            exit(-1);
            #endif
        }

        // Wait the wmaWriteIdx get out of the area of rest data.
        do 
        {
            checkControl();
            if (nStop)
            {
                return 0; 
            }
            wmaWriteIdx = getStreamWrPtr();

            if (wmaWriteIdx >= wmaReadIdx && !eofReached) 
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

        if (wmaWriteIdx < wmaReadIdx)
        {
            // Move the rest data to the front of data buffer
            memcpy(&streamBuf[READBUF_BEGIN - (READBUF_SIZE - wmaReadIdx)],
                   &streamBuf[wmaReadIdx], READBUF_SIZE-wmaReadIdx);
            wmaReadIdx = READBUF_BEGIN - (READBUF_SIZE - wmaReadIdx);
        }
    }

    // Wait data avaliable.
    do 
    {
        checkControl();
        if (nStop)
        {
            return 0; 
        }        
        wmaWriteIdx = getStreamWrPtr();

        if (wmaWriteIdx >= wmaReadIdx)
        {
            len = wmaWriteIdx - wmaReadIdx;
        }
        else 
        {
            len = READBUF_LEN - (wmaReadIdx - wmaWriteIdx);
        }

        if (len < waitNBytes && !eofReached) {
            #if defined(__FREERTOS__)
            PalSleep(2);
            #else
            //or32_delay(1); // enter sleep mode for power saving
            #endif
        } else {
            break;
        }
    } while(1);

    if (len < waitNBytes && eofReached)
        lastRound = 1;

    //PRINTF("AdjustBuffer() %d, get %d bytes.\n", waitNBytes, ((len < waitNBytes) ? len : waitNBytes));
    return ((len < waitNBytes) ? len : waitNBytes);
}

/**************************************************************************************
 * Function     : ClearRdBuffer
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
#if 0 // defined(__INPUT_CRC_CHECK__)
static FILE *fp = NULL;
#endif
static void ClearRdBuffer(void)
{
    int i;
    #if defined(OUTPUT_MEMMODE)
    exit(-1);
    #endif

    #if defined(__INPUT_CRC_CHECK__)
    {
        int crc;
        while(wmaReadIdx != wmaWriteIdx) {
            crc = crc32(&streamBuf[wmaReadIdx], 1);
            #if 0
            if (!fp) fp = fopen("input.dat", "wb");
            if (fp) {
                fwrite(&streamBuf[wmaReadIdx], sizeof(char), 1, fp);
                fflush(fp);
            }
            #endif
            wmaReadIdx++;
            if (wmaReadIdx >= READBUF_SIZE)
                wmaReadIdx = wmaReadIdx - READBUF_SIZE + READBUF_BEGIN;
        }
        PRINTF("CRC: 0x%08x\n", crc);
    }
    #endif // defined(__INPUT_CRC_CHECK__)


    if (isEOF() && !isSTOP())
    {
        #if defined(DUMP_PCM_DATA)
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) | DrvDecodePCM_EOF);
        #endif // defined(DUMP_PCM_DATA)

        // wait I2S buffer empty
        do 
        {
            if (1)// (GetOutRdPtr() == GetOutWrPtr()) 
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
            if (isSkip()==1) 
            {
                MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvDecode_Skip);
                forward_sync = 1;                                   
                break;
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
    for (i=0;i<sizeof(streamBuf);i++)
    {
       streamBuf[i]=0;
    }
    
    #if defined(__OPENRTOS__)
        dc_invalidate(); // Flush DC Cache
    #endif

    if (isEOF()) 
    {
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvDecode_EOF);
    }

    if (isSTOP()) 
    {
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvDecode_STOP);

    }

}

/**************************************************************************************
 * Function     : WMA_GetBufInfo
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
#if defined(__FREERTOS__)

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
void WMA_GetBufInfo(unsigned* inbuf, unsigned* inlen , unsigned* freqbuf,unsigned* audioPluginBuf, unsigned* audioPluginBufLen)
{
#if 0
    *inbuf = (unsigned)&streamBuf[READBUF_BEGIN];
    *inlen = READBUF_LEN;
    #if defined(FREQINFO)
    *freqbuf = (unsigned)&freqinfo[0];
    #endif

    gtAudioPluginBuffer = (unsigned short*)audioPluginBuf;
    gtAudioPluginBufferLength = audioPluginBufLen;
    
    //printf("[Mp3] plugin buffer length %d  0x%x buf 0x%x \n",*gtAudioPluginBufferLength,audioPluginBufLen,audioPluginBuf);
#endif    
    
}

#else // defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
void WMA_GetBufInfo(unsigned* inbuf, unsigned* inlen, unsigned *freqbuf)
{
    *inbuf = (unsigned)&streamBuf[READBUF_BEGIN];
    *inlen = READBUF_LEN;
    #if defined(FREQINFO)
    *freqbuf = (unsigned)&freqinfo[0];
    #else
    *freqbuf = 0;
    #endif
}
#endif // defined(AUDIO_PLUGIN_MESSAGE_QUEUE)

#endif // __FREERTOS__

///////////////////////////////////////////////////////////////////////////
// get stream write pointer
///////////////////////////////////////////////////////////////////////////
static __inline unsigned int getStreamWrPtr(void)
{
#if !defined(INPUT_MEMMODE)
    unsigned int wrPtr;

    wrPtr = MMIO_Read(DrvDecode_WrPtr);
    if (0xffff == wrPtr)
    {
        setAudioReset();
        while (1);
    }
#if defined(__OR32__) && !defined(__FREERTOS__)
    if (0xffff == wrPtr) asm volatile("l.trap 15");
#endif

    return (wrPtr + READBUF_BEGIN);
#else
    return sizeof(streamBuf);
#endif
}

///////////////////////////////////////////////////////////////////////////
// set stream read pointer
///////////////////////////////////////////////////////////////////////////
static __inline void setStreamRdPtr(int rdPtr)
{
    nWmaTemp = rdPtr;

    if (rdPtr >= READBUF_BEGIN) 
    {
        MMIO_Write(DrvDecode_RdPtr, (unsigned short)(((rdPtr-READBUF_BEGIN)>>1) << 1));
    }
    else
    {
        MMIO_Write(DrvDecode_RdPtr, 0);
    }
}

///////////////////////////////////////////////////////////////////////////
// Update stream buffer
///////////////////////////////////////////////////////////////////////////
static __inline void updateRdIdx(int nReadBytes)
{
    #if defined(__INPUT_CRC_CHECK__)
    {
        int crc = crc32(&streamBuf[wmaReadIdx], nReadBytes);
        #if 0
        if (!fp) fp = fopen("input.dat", "wb");
        if (fp) {
            fwrite(&streamBuf[wmaReadIdx], sizeof(char), nReadBytes, fp);
            PRINTF("IDX: 0x%08x %d\n", idx, nReadBytes);
            fflush(fp);
        }
        #endif
        PRINTF("CRC: 0x%08x\n", crc);
    }
    #endif // defined(__INPUT_CRC_CHECK__)

    wmaReadIdx += nReadBytes;

    #if defined(INPUT_MEMMODE)
    if (wmaReadIdx >= READBUF_SIZE)
        wmaReadIdx = READBUF_SIZE;
    #else
    // It never happen.
    if (wmaReadIdx >= READBUF_SIZE) {
        wmaReadIdx = wmaReadIdx - READBUF_SIZE + READBUF_BEGIN;
        PRINTF("AC3: wmaReadIdx > READBUF_SIZE");
        #if defined(WIN32) || defined(__CYGWIN__)
        assert(0);
        #else
        asm volatile("l.trap 15");
        #endif
    }

    setStreamRdPtr(wmaReadIdx);
    #endif // INPUT_MEMMODE
}

///////////////////////////////////////////////////////////////////////////
// read data
///////////////////////////////////////////////////////////////////////////
int read_filebuf(void *buf, unsigned int nReadBytes)
{
    int n;

    n = AdjustBuffer(nReadBytes);
    memcpy(buf, &streamBuf[wmaReadIdx], n);

    updateRdIdx(n);

    #if defined(__DEBUG__)
    {
        #if 0
        unsigned char *ptr = buf;
        int i;
        for(i=0; i<nReadBytes; i++)
            PRINTF("%02x, ", ptr[i]);
        PRINTF("\n");
        #endif
        PRINTF("ReadFileBuf() %d wmaReadIdx(%d) wmaWriteIdx(%d)\n", nReadBytes, wmaReadIdx, wmaWriteIdx);
    }
    #endif

    return n;
}

///////////////////////////////////////////////////////////////////////////
// skip data
///////////////////////////////////////////////////////////////////////////
void advance_buffer(int nReadBytes)
{
    int n,j;
    int nTemp;
    if (nReadBytes<10000)
    {
    n = AdjustBuffer(nReadBytes);
    updateRdIdx(n);
    }
    else
    {
        j = nReadBytes;
        //printf("[WMA]advance_buffer advance_buffer %d \n",nReadBytes);
        for (nTemp=0;j>0;nTemp=0)
        {
            if (j-nTemp>=10000)
            {
                nTemp = 10000;
            }
            else
            {
                nTemp = j;
            }
            j-= nTemp;
            n = AdjustBuffer(nTemp);
            updateRdIdx(n);
        }
    }

    //PRINTF("AdvanceBuffer() %d wmaReadIdx(%d) wmaWriteIdx(%d)\n", nReadBytes, wmaReadIdx, wmaWriteIdx);
}

///////////////////////////////////////////////////////////////////////////
// fill data, and make sure the data dose not wrap
///////////////////////////////////////////////////////////////////////////
static void *request_buffer(size_t *realsize, int nReadBytes)
{
    *realsize = AdjustBuffer(nReadBytes);
    //PRINTF("RequestBuffer() %d\n", nReadBytes);
    return &streamBuf[wmaReadIdx];
}

#define GETLEN2b(bits) (((bits) == 0x03) ? 4 : bits)

#define GETVALUE2b(bits, data) \
        (((bits) != 0x03) ? ((bits) != 0x02) ? ((bits) != 0x01) ? \
         0 : *(data) : get_short_le(data) : get_long_le(data))

static int read_rawData(uint8_t* audiobuf, int* audiobufsize, int* packetlength, asf_waveformatex_t* wfx)
{
    int nWaitBytes;
    int len;
    nWaitBytes = wfx->blockalign; 
    do
    {
        checkControl();
        wmaWriteIdx = getStreamWrPtr();
        len = getUsedLen(wmaReadIdx, wmaWriteIdx, sizeof(streamBuf));

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
    } while(!eofReached && !nStop);
    
    *audiobufsize = nWaitBytes;
    *packetlength = nWaitBytes;
    return 1;
}

static int asf_read_packet(uint8_t** audiobuf, int* audiobufsize, int* packetlength, asf_waveformatex_t* wfx)
{
    uint8_t tmp8, packet_flags, packet_property;
    int stream_id;
    int ec_length, opaque_data, ec_length_type;
    int datalen;
    uint8_t data[18];
    uint8_t* datap;
    uint32_t length;
    uint32_t padding_length;
    uint32_t send_time;
    uint16_t duration;
    uint16_t payload_count;
    int payload_length_type;
    uint32_t payload_hdrlen;
    int payload_datalen;
    int multiple;
    uint32_t replicated_length;
    uint32_t media_object_number;
    uint32_t media_object_offset;
    uint32_t bytesread = 0;
    uint8_t* buf;
    size_t bufsize;
    int i;

    #if defined(WMA_FORWARD_CODEC)
sync_tmp8:
    #endif // WMA_FORWARD_CODEC

    if (read_filebuf(&tmp8, 1) == 0) 
    {
        return ASF_ERROR_EOF;
    }
    bytesread++;

    /* TODO: We need a better way to detect endofstream */
    if (tmp8 != 0x82)
    {
        #if defined(WMA_FORWARD_CODEC)
        if (firstsync_after_forward == 1)
        {
            forward = 1;
            bytesread = 0;
            goto sync_tmp8;
        }
        else
        #endif // WMA_FORWARD_CODEC
        {
            return -1;
        }
    }

    if (tmp8 & 0x80) 
    {
       ec_length = tmp8 & 0x0f;
       opaque_data = (tmp8 >> 4) & 0x01;
       ec_length_type = (tmp8 >> 5) & 0x03;

       if (ec_length_type != 0x00 || opaque_data != 0 || ec_length != 0x02) 
       {
            PRINTF("incorrect error correction flags\n");
            return ASF_ERROR_INVALID_VALUE;
       }

       /* Skip ec_data */
       advance_buffer(ec_length);
       bytesread += ec_length;
    } 
    else
    {
        ec_length = 0;
    }

    if (read_filebuf(&packet_flags, 1) == 0) {
        return ASF_ERROR_EOF;
    }
    if (read_filebuf(&packet_property, 1) == 0) {
        return ASF_ERROR_EOF;
    }

    #if defined(WMA_FORWARD_CODEC)
    if (nFrames == 0)
    {
        packet_flags_temp = packet_flags;
        packet_property_temp = packet_property;
    }

    if (packet_flags != packet_flags_temp || packet_property != packet_property_temp)
    {

        if (firstsync_after_forward == 1)
        {
            forward = 1;
            bytesread = 0;
            goto sync_tmp8;
        }
        else
        {
            return -1;
        }
    }
    #endif // WMA_FORWARD_CODEC

    bytesread += 2;

    datalen = GETLEN2b((packet_flags >> 1) & 0x03) +
              GETLEN2b((packet_flags >> 3) & 0x03) +
              GETLEN2b((packet_flags >> 5) & 0x03) + 6;

    #if 1
    if (datalen > sizeof(data))
    {
        #if defined(WMA_FORWARD_CODEC)
        if (firstsync_after_forward == 1)
        {
            forward = 1;
            bytesread = 0;
            goto sync_tmp8;
        }
        else
        #endif // WMA_FORWARD_CODEC
        {
            return ASF_ERROR_OUTOFMEM;
        }
    }
    #endif // 1

    if (read_filebuf(data, datalen) == 0)
    {
        #if defined(WMA_FORWARD_CODEC)
        if (firstsync_after_forward== 1)
        {
            forward = 1;
            bytesread = 0;
            goto sync_tmp8;
        }
        else
        #endif // WMA_FORWARD_CODEC
        {
            return ASF_ERROR_EOF;
        }
    }

    bytesread += datalen;

    datap = data;
    length = GETVALUE2b((packet_flags >> 5) & 0x03, datap);
    datap += GETLEN2b((packet_flags >> 5) & 0x03);
    /* sequence value is not used */
    GETVALUE2b((packet_flags >> 1) & 0x03, datap);
    datap += GETLEN2b((packet_flags >> 1) & 0x03);
    padding_length = GETVALUE2b((packet_flags >> 3) & 0x03, datap);
    datap += GETLEN2b((packet_flags >> 3) & 0x03);
    send_time = get_long_le(datap);
    datap += 4;
    duration = get_short_le(datap);
    datap += 2;

    /* this is really idiotic, packet length can (and often will) be
     * undefined and we just have to use the header packet size as the size
     * value */
    if (!((packet_flags >> 5) & 0x03)) 
    {
         length = wfx->packet_size;
    }

    /* this is also really idiotic, if packet length is smaller than packet
     * size, we need to manually add the additional bytes into padding length
     */
    if (length < wfx->packet_size)
    {
        padding_length += wfx->packet_size - length;
        length = wfx->packet_size;
    }

    if (length > wfx->packet_size)
    {
        #if defined(WMA_FORWARD_CODEC)
        if (firstsync_after_forward == 1)
        {
            forward = 1;
            bytesread = 0;
            goto sync_tmp8;
        }
        else
        #endif // WMA_FORWARD_CODEC
        {
            return ASF_ERROR_INVALID_LENGTH;
        }
    }

    /* check if we have multiple payloads */
    if (packet_flags & 0x01)
    {
        if (read_filebuf(&tmp8, 1) == 0)
        {
            #if defined(WMA_FORWARD_CODEC)
            if (firstsync_after_forward == 1)
            {
                forward = 1;
                bytesread = 0;
                goto sync_tmp8;
            }
            else
            #endif // WMA_FORWARD_CODEC
            {
                return ASF_ERROR_EOF;
            }
        }
        payload_count = tmp8 & 0x3f;
        payload_length_type = (tmp8 >> 6) & 0x03;
        bytesread++;
    }
    else
    {
        payload_count = 1;
        payload_length_type = 0x02; /* not used */
    }

    if (length < bytesread)
    {
        #if defined(WMA_FORWARD_CODEC)
        if (firstsync_after_forward == 1)
        {
            forward = 1;
            bytesread = 0;
            goto sync_tmp8;
        }
        else
        #endif // WMA_FORWARD_CODEC
        {
            /* FIXME: should this be checked earlier? */
            return ASF_ERROR_INVALID_LENGTH;
        }
    }

    /* We now parse the individual payloads, and move all payloads
       belonging to our audio stream to a contiguous block, starting at
       the location of the first payload.
    */

    *audiobuf = NULL;
    *audiobufsize = 0;
    *packetlength = length - bytesread;

    buf = request_buffer(&bufsize, length);
    datap = buf;

    #if 0 // test request buffer
    {
        int i;
        int rdptr = wmaReadIdx;
        for(i=0; i<length; i++) {
            if (buf[i] != streamBuf[rdptr]) {
                printf("Request data mismatch. @%d with(%d,%d)\n", i, bufsize, (int)length);
                exit(-1);
            }
            rdptr++;
        }
    }
    #endif

    if (bufsize != length)
    {
        /* This should only happen with packets larger than 32KB (the
           guard buffer size).  All the streams I've seen have
           relatively small packets less than about 8KB), but I don't
           know what is expected.*/
        #if defined(WMA_FORWARD_CODEC)
        if (firstsync_after_forward == 1)
        {
            forward = 1;
            bytesread = 0;
            goto sync_tmp8;
        }
        else
        #endif // WMA_FORWARD_CODEC
        {
            //printf("wma read packet error \n");
            return -1;
        }
    }

    for (i=0; i<payload_count; i++) {
        stream_id = datap[0]&0x7f;
        datap++;
        bytesread++;

        payload_hdrlen = GETLEN2b(packet_property & 0x03) +
                         GETLEN2b((packet_property >> 2) & 0x03) +
                         GETLEN2b((packet_property >> 4) & 0x03);

#if 0
        /* TODO */
        if (payload_hdrlen > size) {
            return ASF_ERROR_INVALID_LENGTH;
        }
#endif
        if (payload_hdrlen > sizeof(data))
        {
            #if defined(WMA_FORWARD_CODEC)
            if (firstsync_after_forward == 1)
            {
                forward = 1;
                bytesread = 0;
                goto sync_tmp8;
            }
            else
            #endif // WMA_FORWARD_CODEC
            {
                return ASF_ERROR_OUTOFMEM;
            }
        }

        bytesread += payload_hdrlen;
        media_object_number = GETVALUE2b((packet_property >> 4) & 0x03, datap);
        datap += GETLEN2b((packet_property >> 4) & 0x03);
        media_object_offset = GETVALUE2b((packet_property >> 2) & 0x03, datap);
        datap += GETLEN2b((packet_property >> 2) & 0x03);
        replicated_length = GETVALUE2b(packet_property & 0x03, datap);
        datap += GETLEN2b(packet_property & 0x03);

        /* TODO: Validate replicated_length */
        /* TODO: Is the content of this important for us? */
        datap += replicated_length;
        bytesread += replicated_length;

        multiple = packet_flags & 0x01;

        if (multiple) 
        {
            int x;

            x = GETLEN2b(payload_length_type);

            if (x != 2)
            {
                /* in multiple payloads datalen should be a word */
                #if defined(WMA_FORWARD_CODEC)
                if (firstsync_after_forward == 1)
                {
                    forward = 1;
                    bytesread = 0;
                    goto sync_tmp8;
                }
                else
                #endif // WMA_FORWARD_CODEC
                {
                    return ASF_ERROR_INVALID_VALUE;
                }
            }

#if 0
            if (skip + tmp > datalen) {
                /* not enough data */
                return ASF_ERROR_INVALID_LENGTH;
            }
#endif
            payload_datalen = GETVALUE2b(payload_length_type, datap);
            datap += x;
            bytesread += x;
        } 
        else 
        {
            payload_datalen = length - bytesread - padding_length;
        }

        /* Fix bug for reading packet when replicated length doesn't equal to eight by Viola Lee on 2009.01.21 */
        if (1 == replicated_length)
        {
            datap++;
        }

        if (stream_id == wfx->audiostream)
        {
            if (*audiobuf == NULL) {
                /* The first payload can stay where it is */
                *audiobuf = datap;
                *audiobufsize = payload_datalen;
            } else {
                /* The second and subsequent payloads in this packet
                   that belong to the audio stream need to be moved to be
                   contiguous with the first payload.
                */
                memmove(*audiobuf + *audiobufsize, datap, payload_datalen);
                *audiobufsize += payload_datalen;
            }
        }
        datap += payload_datalen;
        bytesread += payload_datalen;
    }

    if (*audiobuf != NULL)
        return 1;
    else
        return 0;
}

static __inline int ENDIAN_LE32(unsigned char *n) {
    int num = (((unsigned int)n[0]) + ((unsigned int)n[1] <<  8) +
               ((unsigned int)n[2] << 16) + ((unsigned int)n[3] << 24));
    return num;
}

static __inline short ENDIAN_LE16(unsigned char *n) {
    short num = (short)(((unsigned short)n[0]) + ((unsigned short)n[1] << 8));
    return num;
}

static __inline int ENDIAN_32(unsigned char *n) {
    int num = (((unsigned int)n[0]<< 24) + ((unsigned int)n[1] <<  16) +
               ((unsigned int)n[2] << 8) + ((unsigned int)n[3] ));
    return num;
}

static __inline short ENDIAN_16(unsigned char *n) {
    short num = (short)(((unsigned short)n[0] << 8) + ((unsigned short)n[1]));
    return num;
}

/**************************************************************************************
 * Function     : ParseWmaInfo
 *
 * Description  : Parse the info & update the read pointer
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
static int ParseWmaInfo(asf_waveformatex_t *wfx) 
{
    unsigned char *header;
    int nWaitBytes;
    int header_size;

    nWaitBytes  = 32;    // Wait the wave header
    header_size = 0;
    header      = &streamBuf[wmaReadIdx];

    // Wait the header on output buffer avaliable
    do
    {
        int len;
        checkControl();
        wmaWriteIdx = getStreamWrPtr();

        len = getUsedLen(wmaReadIdx, wmaWriteIdx, sizeof(streamBuf));

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
    } while(!eofReached && !nStop);
    if (isWMAWithoutASFHeader()==1){
        //uint32_t packet_size;
        wfx->packet_size = (uint32_t)ENDIAN_LE32(&header[0]);
        //int audiostream;
        wfx->audiostream = 1;
        //uint16_t codec_id;
        wfx->codec_id = (uint16_t)ENDIAN_LE16(&header[8]);
        //uint16_t channels;
        wfx->channels = (uint16_t)ENDIAN_LE16(&header[10]);
        //uint32_t rate;
       wfx->rate = (uint32_t)ENDIAN_LE32(&header[12]);
        //uint32_t bitrate;
       wfx->bitrate = (uint32_t)ENDIAN_LE32(&header[16]);
        //uint16_t blockalign;
        wfx->blockalign = (uint16_t)ENDIAN_LE16(&header[20]);
        //uint16_t bitspersample;
        wfx->bitspersample = (uint16_t)ENDIAN_LE16(&header[22]);
        //uint16_t datalen;
        wfx->datalen = (uint16_t)ENDIAN_LE16(&header[24]);
        //uint8_t data[6];
    }else if (isMusicWithoutASFHeader()==1){
        wfx->packet_size = (uint32_t)ENDIAN_32(&header[0]);
        //int audiostream;
        wfx->audiostream = 1;
        //uint16_t codec_id;
        wfx->codec_id = (uint16_t)ENDIAN_16(&header[8]);
        //uint16_t channels;
        wfx->channels = (uint16_t)ENDIAN_16(&header[10]);
        //uint32_t rate;
       wfx->rate = (uint32_t)ENDIAN_32(&header[12]);
        //uint32_t bitrate;
       wfx->bitrate = (uint32_t)ENDIAN_32(&header[16]);
        //uint16_t blockalign;
        wfx->blockalign = (uint16_t)ENDIAN_16(&header[20]);
        //uint16_t bitspersample;
        wfx->bitspersample = (uint16_t)ENDIAN_16(&header[22]);
        //uint16_t datalen;
        wfx->datalen = (uint16_t)ENDIAN_16(&header[24]);
        //uint8_t data[6];
    }
    wfx->data[0] = header[26];
    wfx->data[1] = header[27];
    wfx->data[2] = header[28];
    wfx->data[3] = header[29];
    wfx->data[4] = header[30];
    wfx->data[5] = header[31];

    wmaReadIdx+=32;

    return 0;
}
//========================================================================
static void buffer_init(void)
{
    int index1, index2;

    // buffer
    for(index1 = 0; index1 < BLOCK_NB_SIZES; index1++)
    {
        wmadec.exponent_sizes[index1] = 0;
        wmadec.high_band_start[index1] = 0;
        wmadec.coefs_end[index1] = 0;
        wmadec.exponent_high_sizes[index1] = 0;

        for(index2 = 0; index2 < 25 ; index2++)
            wmadec.exponent_bands[index1][index2] = 0;
        for(index2 = 0; index2 < HIGH_BAND_MAX_SIZE ; index2++)
            wmadec.exponent_high_bands[index1][index2] = 0;
    }

    for(index1 = 0; index1 < MAX_CHANNELS; index1++)
    {
        wmadec.channel_coded[index1] = 0;
        wmadec.exponents_bsize[index1] = 0;
        wmadec.max_exponent[index1] = 0;

        for(index2 = 0; index2 < HIGH_BAND_MAX_SIZE ; index2++)
        {
            wmadec.high_band_coded[index1][index2] = 0;
            wmadec.high_band_values[index1][index2] = 0;
        }

        for(index2 = 0; index2 < BLOCK_MAX_SIZE*2; index2++)
            wmadec.frame_out[index1][index2] = 0;

        for(index2 = 0; index2 < BLOCK_MAX_SIZE; index2++)
        {
            wmadec.exponents[index1][index2] = 0;
            wmadec.coefs[index1][index2] = 0;
        }
    }

    for(index1 = 0; index1 < MAX_CODED_SUPERFRAME_SIZE+4; index1++)
        wmadec.last_superframe[index1] = 0;

    for(index1 = 0; index1 < 2048; index1++)
        wmadec.cos_table[index1] = 0;

    for(index1 = 0; index1 < (1 << LSP_POW_BITS); index1++)
    {
        wmadec.lsp_pow_m_table1[index1] = 0;
        wmadec.lsp_pow_m_table2[index1] = 0;
    }
}


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
            //printf("[WMA] name length %d \n",gnTemp);
            memcpy(&gtAudioPluginBuffer[0], &gnTemp, SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH);                           
            memcpy(&gtAudioPluginBuffer[SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH/sizeof(short)], tDumpWave, sizeof(tDumpWave));
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_WRITE:
          // printf("[Mp3] name length %d \n",gnTemp);           
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
                //printf("[Mp3] 0x%x %d %d %d \n",nTemp,gCh,gSampleRate,nTemp1);
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC:
                pBuf = (unsigned char*)gtAudioPluginBuffer;
                memcpy(&pBuf[0], &gPause, sizeof(int));
                //printf("[Mp3] pause %d \n",gPause);
            break;
        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_DAC:
            break;
        
        case SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF:

            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_WMA_INFO:
                pBuf = (unsigned char*)gtAudioPluginBuffer;
                memcpy(pBuf,&gWfx,sizeof(gWfx));

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
            //printf("[Mp3] get main procerror feedback \n");
            break;
        }
        i--;
    }while(i);
    if (i==0)
        printf("[WMA] audio api %d %d\n",i,nType);

}
#endif // defined(AUDIO_PLUGIN_MESSAGE_QUEUE)


/* this is the codec entry point */
#if defined(__FREERTOS__) && !defined(ENABLE_CODECS_PLUGIN)
portTASK_FUNCTION(wmadecode_task, params)
#else
int main(int argc, char **argv)
#endif
{
    uint32_t samplesdone;
    uint32_t elapsedtime;
    asf_waveformatex_t wfx;
    uint32_t currentframe;
    int i;
    int wmares, res;
    uint8_t* audiobuf;
    int audiobufsize;
    int packetlength;
    int nPCMBytes;
    int err;
    int m, temp;
    int superframe_init, frame_counter;
    int exitflag1 = 0;
    int    nTemp;
    unsigned int ctrl;    
    #if defined(WMA_FORWARD_CODEC)
    unsigned long skip_index;
    #endif
    uint16_t *pcm;
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    int* codecStream;
    AUDIO_CODEC_STREAM* audioStream;

    codecStream=CODEC_STREAM_START_ADDRESS;   
    //printf("[E-AC3] 0x%08x \n",*codecStream);
    audioStream = *codecStream;
    audioStream->codecStreamBuf = &streamBuf[READBUF_BEGIN];
    audioStream->codecStreamLength = READBUF_LEN;   
    //printf("[E-AC3] audioStream 0x%08x 0x%08x 0x%08x   \n",&audioStream,&audioStream->codecStreamBuf,&audioStream->codecStreamLength);
    gtAudioPluginBuffer = audioStream->codecAudioPluginBuf;
    gtAudioPluginBufferLength = audioStream->codecAudioPluginLength;
    //printf("[E-AC3] 0x%08x %d 0x%08x %d \n",audioStream->codecStreamBuf,audioStream->codecStreamLength ,audioStream->codecAudioPluginBuf,audioStream->codecAudioPluginLength);
    MMIO_Write(AUDIO_DECODER_START_FALG, 1);
#endif
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
        ithPrintf("[WMA]start %d \n",isMusicWithoutASFHeader());
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
        printf("[WMA]get_asf_metadata error %d, skip stream\n",nTemp);
#endif

    #if defined(WIN32) || defined(__CYGWIN__)
    GetParam(argc, argv);
    win32_init();
    #endif // defined(WIN32) || defined(__CYGWIN__)

    while (1) 
    {
        #if !defined(INPUT_MEMMODE)
        wmaReadIdx  = READBUF_BEGIN;
        wmaWriteIdx = READBUF_BEGIN;
        #else
        wmaReadIdx  = 0;
        wmaWriteIdx = 0;
        #endif
        pcmReadIdx  = 0;
        pcmWriteIdx = 0;
        nFrames     = 0;
        eofReached  = 0;
        lastRound   = 0;
        decTime     = 0;
        nPCMBytes   = 0;
        wmaFrameAccu= 0;

        samplesdone = elapsedtime = currentframe = i = wmares = res = audiobufsize = packetlength = err = 0;
        superframe_init = frame_counter = 0; 

        #if defined(WMA_LAG_OPTIMIZE)
        lag_optimize = 0;
        #endif // WMA_LAG_OPTIMIZE
        #if defined(ENABLE_PERFORMANCE_MEASURE)
        start_timer();
        #endif // defined(ENABLE_PERFORMANCE_MEASURE)
        #if defined(__INPUT_CRC_CHECK__) || defined(__OUTPUT_CRC_CHECK__)
        crc32_init();
        #endif // defined(__INPUT_CRC_CHECK__) || defined(__OUTPUT_CRC_CHECK__)

        MMIO_Write(DrvDecode_Frame  , 0);
        MMIO_Write(DrvDecode_Frame+2, 0);
        // Add buffer initial by Viola Lee on 2009.02.17
        buffer_init();
        /* Wait for the metadata to be read */
        
        if ((isWMAWithoutASFHeader()==0 && isMusicWithoutASFHeader()==0 ) && !get_asf_metadata(&id3)) 
        {
            PRINTF("asf parsing error\n");
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
            ithPrintf("[Wma]get_asf_metadata error , skip stream  \n");
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
            printf("[Wma]get_asf_metadata error , skip stream  \n");
#endif
            // Skip all of stream
            do 
            {
                checkControl();
                setStreamRdPtr(getStreamWrPtr());
                #if defined(__FREERTOS__)
                PalSleep(2);
                #else
                //or32_delay(1); // enter sleep mode for power saving
                #endif
            } while(!eofReached && !nStop);
            goto exit;
        }

        /* Copy the format metadata we've stored in the id3 TOC field.  This
           saves us from parsing it again here. */
        memcpy(&wfx, id3.toc, sizeof(wfx));

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
            ithPrintf("[WMA] packet size %d audio stream %d codec id %d channels %d rate %d bitrate %d blockalign %d bitspersample %d datalen %d 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",            
                       wfx.packet_size,wfx.audiostream,wfx.codec_id,wfx.channels,wfx.rate,wfx.bitrate,wfx.blockalign,wfx.bitspersample,wfx.datalen,wfx.data[0],wfx.data[1],wfx.data[2],wfx.data[3],wfx.data[4],wfx.data[5]);
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
            printf("[WMA] packet size %d audio stream %d codec id %d channels %d rate %d bitrate %d blockalign %d bitspersample %d datalen %d 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",            
                       wfx.packet_size,wfx.audiostream,wfx.codec_id,wfx.channels,wfx.rate,wfx.bitrate,wfx.blockalign,wfx.bitspersample,wfx.datalen,wfx.data[0],wfx.data[1],wfx.data[2],wfx.data[3],wfx.data[4],wfx.data[5]);                
#endif
        // save wma info to arm
        if (isMusicWithoutASFHeader()==0){
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
            memcpy(&gWfx,&wfx,sizeof(wfx));
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_WMA_INFO);
#endif
        }
        
        if (isWMAWithoutASFHeader()==1 || isMusicWithoutASFHeader()==1)        
        {
            // parsing asf info           
            nTemp = ParseWmaInfo(&wfx);
            if (nTemp!=0)
            {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                ithPrintf("[WMA] ParseWmaInfo error %d, skip stream\n",nTemp);
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                printf("[WMA]get_asf_metadata error %d, skip stream\n",nTemp);
#endif
                // Skip all of stream
                do 
                {
                    checkControl();
                    setStreamRdPtr(getStreamWrPtr());
                    #if defined(__FREERTOS__)
                    PalSleep(2);
                    #else
                    //or32_delay(1); // enter sleep mode for power saving
                    #endif
                } while(!eofReached && !nStop);
                goto exit;
            }
            //wfx.packet_size = 682;
            //wfx.audiostream = 1;
            //wfx.codec_id = ASF_CODEC_ID_WMAV2;
            //wfx.channels = 2;
            //wfx.rate = 48000;
            //wfx.bitrate = 128000;
            //wfx.blockalign = 4096;
            //wfx.bitspersample = 16;
            //wfx.datalen = 10;
            //wfx.data[0]=0;wfx.data[1]=0x88;wfx.data[2]=0;wfx.data[3]=0;wfx.data[4]=0xf;wfx.data[5]=0;
            if (isMusicWithoutASFHeader()==1){
                firstsync_after_forward=1;
            }

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
            ithPrintf("[WMA] packet size %d audio stream %d codec id %d channels %d rate %d bitrate %d blockalign %d bitspersample %d datalen %d 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",            
                       wfx.packet_size,wfx.audiostream,wfx.codec_id,wfx.channels,wfx.rate,wfx.bitrate,wfx.blockalign,wfx.bitspersample,wfx.datalen,wfx.data[0],wfx.data[1],wfx.data[2],wfx.data[3],wfx.data[4],wfx.data[5]);
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
            printf("[WMA] packet size %d audio stream %d codec id %d channels %d rate %d bitrate %d blockalign %d bitspersample %d datalen %d 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",            
                       wfx.packet_size,wfx.audiostream,wfx.codec_id,wfx.channels,wfx.rate,wfx.bitrate,wfx.blockalign,wfx.bitspersample,wfx.datalen,wfx.data[0],wfx.data[1],wfx.data[2],wfx.data[3],wfx.data[4],wfx.data[5]);                
#endif
            
            
        }
    #if defined(WMA_FORWARD_CODEC)
         wmaInitDAC:
    #endif

        #if !defined(DUMP_PCM_DATA)
        if (isUpSample())
        {
        
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)            
            gCh = wfx.channels;
            gSampleRate = GetUpSampleRate(wfx.rate);
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC);
#else
            initDAC((unsigned char*)pcmWriteBuf, wfx.channels, GetUpSampleRate(wfx.rate), sizeof(pcmWriteBuf), 0);            
#endif        
        }        
        else
        {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)            
            gCh = wfx.channels;
            gSampleRate = wfx.rate;
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC);
#else
            initDAC((unsigned char*)pcmWriteBuf, wfx.channels, wfx.rate, sizeof(pcmWriteBuf), 0);
#endif        
        }
        #endif // defined(DUMP_PCM_DATA)

         #ifdef WMA_PERFORMANCE_TEST
             printf("[WMA] init sample rate %d channels %d \n",wfx.rate,wfx.channels);
             nDecPerformance = 0;
         #endif            
        if(forward_sync==1)
        {
            do
            {
                PalSleep(2);                           
                
            }while( isSkip()!=1 );
            if (isSkip()==1) 
            {
                MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvDecode_Skip);
            }
            // wait 50 ms to get new data
            PalSleep(50);
        }

        //wfx.codec_id = ASF_CODEC_ID_WMAV2;    // Bad assignment! Mark it by Viola Lee on 2009.01.21
        //wfx.codec_id = ASF_CODEC_ID_WMAV1;

        ctrl = (unsigned int)MMIO_Read(DrvAudioCtrl2);
        //wmadec.nShowSpectrum = (ctrl & DrvShowSpectrum) ? 1 : 0;

        if ((err = wma_decode_init(&wmadec,&wfx)) < 0) 
        {
            //PRINTF("WMA: Unsupported or corrupt file, err = %d\n", err);
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
            ithPrintf("[wma] wma init decdde error %d \n",err);
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
            printf("[wma] wma init decdde error %d \n",err);
#endif
            // Skip all of stream
            do 
            {
                checkControl();
                setStreamRdPtr(getStreamWrPtr());

                #if defined(__FREERTOS__)
                PalSleep(2);
                #else
                //or32_delay(1); // enter sleep mode for power saving
                #endif
            } while(!eofReached && !nStop);

            goto exit;
        }

        /* The main decoding loop */
        currentframe = 0;
        samplesdone = 0;

        //PRINTF("**************** IN WMA.C ******************\n");
        //wma_decode_init(&wmadec,&wfx);

        /* calculate the time per one frame */
        {
            // wmaFrameStep is 1.31 format.
            uint64_t n = ((uint64_t)wmadec.frame_len) << 31;
            wmaFrameStep = (unsigned int)(n / wmadec.sample_rate);
            //PRINTF("Sample Rate: %d, Frame Length: %d\n", wmadec.sample_rate, wmadec.frame_len);
        }

        #if defined(ENABLE_PERFORMANCE_MEASURE)
        {
            int elapsed = get_timer();
            if (nFrames < (sizeof(time_log)/sizeof(int)))
                time_log[nFrames] = elapsed;
        }
        #endif // defined(ENABLE_PERFORMANCE_MEASURE)

        res = 1;
        while (res >= 0 && !lastRound)
        { // WMA Decode Loop
            #if defined(ENABLE_PERFORMANCE_MEASURE)
            start_timer();
            timer_start = 1;
            #endif // defined(ENABLE_PERFORMANCE_MEASURE)

            #if defined(WMA_FORWARD_CODEC)
                if (forward_sync == 1)
                {
                    PalSleep(2);
                    forward_sync = 0;
                    firstsync_after_forward = 1;
                }
                //printf("wma isSkip() %d \n",isSkip());
                while (isSkip() == 1)
                {
                    forward_sync = 1;                                   
                    if (isSkip()==1) 
                    {
                        MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvDecode_Skip);
                        goto exit;
                    }
                }
                if (forward_sync == 1)
                {               
                    PalSleep(2);
                    break;
                }
            #endif // WMA_FORWARD_CODEC

            if (isWMAWithoutASFHeader()==1)
            {
                res = read_rawData(audiobuf, &audiobufsize, &packetlength, &wfx);
                audiobuf = (uint8_t*)request_buffer((size_t*)&audiobufsize, packetlength);                
                //printf("[Wma] %d %d %d %d\n",audiobuf[0],packetlength,audiobufsize,wmaReadIdx);                
            }                   
            else
            {
                res = asf_read_packet(&audiobuf, &audiobufsize, &packetlength, &wfx);
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
            //ithPrintf("[wma] asf_read_packet %d %d 0x%x \n",res,firstsync_after_forward,wmaReadIdx);
            //AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else

#endif                
            }

            #if defined(WMA_FORWARD_CODEC)
            if (forward == 1)
            {
                forward = 0;
                firstsync_after_forward = 0;
            }
            #endif // WMA_FORWARD_CODEC
            if (wfx.blockalign!=audiobufsize)
            {
                if (isMusicWithoutASFHeader()==1){
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                    ithPrintf("[wma] blockalign %d != audiobufsize %d  \n",wfx.blockalign,audiobufsize);
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);     
                    firstsync_after_forward=1;
#else

#endif
                }
            
                //printf("blockalign %d != audiobufsize %d \n",wfx.blockalign,audiobufsize);
                memcpy(gFrame,audiobuf,audiobufsize);
                gFrameUse=0;
                gWMA8 = 1;
            }
            else
            {
                gWMA8 = 0;
            }
            
            if (gWMA8)
            {
                for (gFrameUse=0;gFrameUse<audiobufsize;gFrameUse+=wfx.blockalign)
                {
                    wma_decode_superframe_init(&wmadec, &gFrame[gFrameUse],wfx.blockalign);
                    FillWriteBuffer(nPCMBytes);

                    if (isEnableOutputEmpty() ){
                        // output empty
                        pcm = emptyBuffer;
                    } else {
                        pcm = (int16_t*)&pcmWriteBuf[pcmWriteIdx];
                    }
                    
                    wmares = wma_decode_superframe_frame(&wmadec, pcm, //decoded,
                                                         &gFrame[gFrameUse], wfx.blockalign);
                    if (wmares < 0) {
                        //PRINTF("WMA decode error %d\n",wmares);
                        nPCMBytes = 0;
                        continue;
                    } else if (wmares > 0) {
                        nPCMBytes = wmares*sizeof(short)*wfx.channels;
                        samplesdone += wmares;
                        elapsedtime = (samplesdone*10)/(wfx.rate/100);
                    }
                    #if defined(WMA_FORWARD_CODEC)      
                        while (isSkip() == 1)
                        {
                            //PalSleep(2);
                            forward_sync = 1;                         
                            if (isSkip()==1) 
                            {
                                MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvDecode_Skip);
                                goto exit;
                            }
                            
                        }
                        
                        if (forward_sync == 1)
                        {                        
                            PalSleep(2);
                            goto exit;
                        }
                   #endif // WMA_FORWARD_CODEC

                    nFrames++;
                    updateTime();
                }

            }
            else
            {
            
            if (res > 0)
            {
                superframe_init = wma_decode_superframe_init(&wmadec, audiobuf, audiobufsize);
                /* If superframe counter equal to zero, go to next packet by Viola Lee on 2009.01.22 */
                if(ERR_NB_FRAME_ZERO == superframe_init)
                {
                    goto SuperFrameEnd;
                }

                frame_counter = 0;
                for (i=0; i < wmadec.nb_frames; i++) 
                {
                    frame_counter++;

                    #if defined(WMA_FORWARD_CODEC)      
                        while (isSkip() == 1)
                        {
                            //PalSleep(2);
                            forward_sync = 1;                         
                            if (isSkip()==1) 
                            {
                                MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvDecode_Skip);
                                goto exit;
                            }                            
                                
                        }                        
                            
                        if (forward_sync == 1)
                        {
                            PalSleep(2);
                            goto exit;
                        }
                   #endif // WMA_FORWARD_CODEC
                    
                    #if defined(ENABLE_PERFORMANCE_MEASURE)
                    if (!timer_start)
                        start_timer();
                    #endif // defined(ENABLE_PERFORMANCE_MEASURE)

                    FillWriteBuffer(nPCMBytes);

                    if (isEnableOutputEmpty() ){
                        // output empty
                        pcm = emptyBuffer;
                    } else {
                        pcm = (int16_t*)&pcmWriteBuf[pcmWriteIdx];
                    }
                    

                    if (nStop)
                    {
                        goto exit;
                    }
                 #ifdef WMA_PERFORMANCE_TEST
                      tClockPerformance = PalGetClock();
                 #endif            
                    
                    wmares = wma_decode_superframe_frame(&wmadec, pcm, //decoded,
                                                         audiobuf, audiobufsize);

                    /* Support Ensky request from stereo to mono by Viola Lee on 2009.01.12
                       New left channel data = New right channel data = (Old left channel data + Old right channel data)/2 */
                    if(isDownSample() && wmadec.nb_channels == 2)
                    {
                        for(m = 0; m < wmares; m += 2)
                        {
                            // pcm[m+1] = pcm[m] = (pcm[m] + pcm[m+1])/2;
                            temp = (pcm[m] + pcm[m+1])/2;
                            pcm[m] = temp;
                            pcm[m+1] = temp;
                        }
                    }
                    if (wmares < 0) 
                    {
                        //PRINTF("WMA decode error %d\n",wmares);
                        nPCMBytes = 0;
                        continue;
                    }
                    else if (wmares > 0)
                    {
                        nPCMBytes = wmares*sizeof(short)*wfx.channels;
                        samplesdone += wmares;
                        elapsedtime = (samplesdone*10)/(wfx.rate/100);
                        if (isUpSample() && (GetUpSampleRate(wfx.rate)>wfx.rate) )
                        {
                            nPCMBytes = nPCMBytes*(GetUpSampleRate(wfx.rate)/wfx.rate);
                            pcm = (int16_t*)&pcmWriteBuf[pcmWriteIdx];                            
                            UpSampling(wfx.rate,GetUpSampleRate(wfx.rate),wmares*wfx.channels,wfx.channels,pcm,tUpSample,16);                                                                        
                            memcpy(pcmWriteBuf + pcmWriteIdx, tUpSample, nPCMBytes*sizeof(short));    
                        }                                                
                    }
                #ifdef WMA_PERFORMANCE_TEST
                    if(wmares > 0)
                    {
                        nDecPerformance+= PalGetDuration(tClockPerformance);
                    }
                    if ( nFrames % 500 == 0 )
                    {
                        printf("[WMA] average of decode time %d nDecPerformance %d nFrames %d \n",(nDecPerformance/nFrames),nDecPerformance,nFrames );
                    }
                #endif
                      
                    nFrames++;
                    wmadec.nFrames = nFrames;
                    updateTime();
                    //PRINTF("*** Frames #%d.\n", nFrames);

                    #if defined(ENABLE_PERFORMANCE_MEASURE)
                    {
                        int elapsed = get_timer();
                        if (nFrames < (sizeof(time_log)/sizeof(int)))
                                time_log[nFrames] = elapsed;
                        timer_start = 0;
                    }
                    #endif // defined(ENABLE_PERFORMANCE_MEASURE)
                }
            }
            else
            {
                packetlength = 4;
            }
        }

SuperFrameEnd:
            advance_buffer(packetlength);

            // Check AP Status by Viola Lee on 2009.02.16
            int paused = 0;

            #if defined(SUPPORT_HW_FADING)
            int I2S_rd_pointer1 = 0;
            int I2S_rd_pointer2 = 0;
            #endif

            while (isPAUSE())
            {
                #if defined(SUPPORT_HW_FADING)
                    I2S_rd_pointer1 = MMIO_Read(0x165C);
                #endif
                
                #if defined(__FREERTOS__)
                PalSleep(2);
                #else
                //or32_delay(10);
                #endif

                #if defined(SUPPORT_HW_FADING)
                    I2S_rd_pointer2 = MMIO_Read(0x165C);
                    if (I2S_rd_pointer1 == I2S_rd_pointer2)
                    {
                        paused = 1;
                        break;
                    }
                #endif

                if (isSTOP())
                {
                    exitflag1 = 1;
                    break;
                }
                if (isEOF())
                {
                    break;
                }
                if (!paused)
                {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
                    gPause = 1;
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC);
#else
                    pauseDAC(1);
#endif                                 
                    paused = 1;
                }
            }
            if (paused)
            {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
                gPause = 0;
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC);
#else
                pauseDAC(0);
#endif

            }

            if (isSTOP())
            {            
                exitflag1 = 1;
                break;
            }

            if (isEOF())
            {
                #if defined(WMA_FORWARD_CODEC)            
                    if (isSkip() == 1)
                    {
                        forward_sync = 1;                       
                        if (isSkip()==1) 
                        {
                            MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvDecode_Skip);
                        }        
                        goto exit;                        
                    }
                #endif            
            }            
        }

exit:

        //PRINTF("WMA: Decoded %d samples\n", (int)samplesdone);

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                ithPrintf("[WMA] ClearRdBuffer\n");
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                printf("[WMA]get_asf_metadata error %d, skip stream\n",nTemp);
#endif
        
        ClearRdBuffer();
        //printf("wma reset buffer complete  \n");

        #if defined(WMA_FORWARD_CODEC)
        if (forward_sync == 1)
        {
            // Avoid AP to jump to next next song when reaching to EOF in the fast forward status.
            if (isPAUSE())
            {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                ithPrintf("[Wma] forever_loop_end \n");
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
               printf("wma forever_loop_end \n");
#endif
              // goto forever_loop_end;
            }

#if !defined(INPUT_MEMMODE)
            wmaReadIdx  = READBUF_BEGIN;
            wmaWriteIdx = READBUF_BEGIN;
#else
            wmaReadIdx  = 0;
            wmaWriteIdx = 0;
#endif
            setStreamRdPtr(wmaReadIdx);
            pcmReadIdx  = 0;
            pcmWriteIdx = 0;
            nFrames     = 1;                // for sync ASF packet
            eofReached  = 0;
            nStop = 0;
            lastRound   = 0;
            decTime     = 0;
            nPCMBytes   = 0;
            wmaFrameAccu= 0;

            samplesdone = elapsedtime = currentframe = i = wmares = res = audiobufsize = packetlength = err = 0;
            superframe_init = frame_counter = 0;

            #if defined(ENABLE_PERFORMANCE_MEASURE)
            start_timer();
            #endif

            #if defined(__INPUT_CRC_CHECK__) || defined(__OUTPUT_CRC_CHECK__)
            crc32_init();
            #endif

            MMIO_Write(DrvDecode_Frame  , 0);
            MMIO_Write(DrvDecode_Frame+2, 0);

            buffer_init();

            //printf("wma goto dac init \n");
         #if defined(WMA_FORWARD_CODEC)            
             forward_sync = 1;
             firstsync_after_forward = 1;        
        #endif // WMA_FORWARD_CODEC

        
            goto wmaInitDAC;
        }
        #endif // WMA_FORWARD_CODEC

        #if defined(WMA_FORWARD_CODEC)
forever_loop_end:
        #endif

        samplesdone = 0;                    // useless

        #if defined(WIN32) || defined(__CYGWIN__)
        break;
        #endif
    }

#if !defined(__OPENRTOS__)
    return 0;
#endif
}

