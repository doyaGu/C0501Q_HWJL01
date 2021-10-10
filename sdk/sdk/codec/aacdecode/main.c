/***************************************************************************
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 *
 * @file
 * Top code of AAC decoder.
 *
 * @author Kuoping Hsu
 * @version 1.0
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "aac_latm.h"
#include "config.h"
#include "coder.h"
#include "aacdec.h"
#include "reverb.h"
#include "drc.h"
//#include "ticktimer.h"
#include "assembly.h"
//#define __OUTPUT_STREAM__
#if defined(__OUTPUT_STREAM__)
#define __BEGIN_FRAME__       10
#define __END_FRAME__         12
#endif // __OUTPUT_STREAM__

#if defined(__INPUT_CRC_CHECK__) || defined(__OUTPUT_CRC_CHECK__)
#include "crc32.h"
#endif // defined(__INPUT_CRC_CHECK__) || defined(__OUTPUT_CRC_CHECK__)

#if defined(__FREERTOS__)
#include "FreeRTOS.h"
#include "task.h"
#include "timer.h"
#endif


#if defined(ENABLE_CODECS_PLUGIN)
#include "plugin.h"
#endif

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
static unsigned int* gtAudioPluginBufferLength;
static unsigned short* gtAudioPluginBuffer;
static int gnTemp;
static unsigned char tDumpWave[] = "C:/aac_dump.pcm";
static int gCh;
static int gSampleRate;
int gPause = 0 ;
int gPrint = 0;
unsigned char *gBuf;
#endif

#ifdef AAC_DUMP_PCM
//static PAL_FILE *tPcmFileOutput = NULL;
//static PAL_CLOCK_T tClockStartTimer;
#endif

#ifdef AAC_PERFORMANCE_TEST
static PAL_CLOCK_T tClockPerformance;
static long nDecPerformance;  // avg of frame
static PAL_CLOCK_T tClockDownmixingPerformance;
static long nDownmixingPerformance;  // avg of frame
#endif

#ifdef AAC_FLOW_CONTROL
static PAL_CLOCK_T tContextSwitch;      
#endif

#if defined(USE_PTS_EXTENSION)
static int last_rdptr = 0;
#endif // defined(USE_PTS_EXTENSION)
static unsigned int gLastPtsOverFlowSection = AAC_INVALID_PTS_OVER_FLOW_VALUE;

///////////////////////////////////////////////////////////////////////////
//                              Constant Definition
///////////////////////////////////////////////////////////////////////////
#define ERROR_RECOVER

#define MAX_AAC_FRAME_LENGTH 1500

#define MAX_FRAME_SIZE  (AAC_MAX_NCHANS * AAC_MAX_NSAMPS * 2 * 2)

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
//                              Globale Variable
///////////////////////////////////////////////////////////////////////////
static int decTime = 0; // Decoding time in seconds on S15.16 fixed point.
AACFrameInfo aacFrameInfo;

// Reverberation
#if defined(REVERBERATION)
int reverbStateNow = 0, reverbStatePrev = 0;
#endif // REVERBERATION

#if defined(ENABLE_PERFORMANCE_MEASURE)
#  include "ticktimer.h"
#define MAX_TIME_LOG 10000
static unsigned int time_log[MAX_TIME_LOG];
#endif // !defined(ORIG)

// Output PCM buffer for AAC Decoding
#if defined(__GNUC__)
static unsigned char pcmWriteBuf[I2SBUFSIZE] __attribute__ ((aligned(16), section (".sbss")));
#else
static unsigned char pcmWriteBuf[I2SBUFSIZE];
#endif
static unsigned int  pcmReadIdx;
static unsigned int  pcmWriteIdx;

#ifdef ENABLE_DOWNMIX_CHANNELS
static unsigned char pcmDownMixBuf[1024*6*2];
#endif

static unsigned short emptyBuffer[4096*2];

/*************************************************************************************
 * Input Buffer (AAC Stream data) from driver
 *
 *    |<--------------------- READBUF_SIZE ---------------------->|
 *
 *    |<------------------------------------->|<----------------->|
 *                  READBUF_GUARD               READBUF_GUARDSIZE
 *    +-------------------+---------------------------------------+
 *    |                   |                                       |
 *    |                   |         Driver Input Buffer           |
 *    |                   |                                       |
 *    +-------------------+---------------------------------------+
 *    |<----------------->|<------------------------------------->|
 *      READBUF_GUARDSIZE               READBUF_GUARD
 *                        ^
 *                        |
 *                        |
 *                   READBUF_BEGIN
 *
 *************************************************************************************/
/* define in config.h
#define READBUF_SIZE      (3 * AAC_MAINBUF_SIZE * AAC_MAX_NCHANS)
#define READBUF_GUARDSIZE AAC_MAINBUF_SIZE
#define READBUF_BEGIN     READBUF_GUARDSIZE
#define READBUF_GUARD     (READBUF_SIZE - READBUF_GUARDSIZE)
*/

#if defined(INPUT_MEMMODE)
unsigned char streamBuf[] = {
#  include "aacfile.h"
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

///////////////////////////////////////////////////////////////////////////
//                              Local Variable
///////////////////////////////////////////////////////////////////////////
static unsigned int  aacReadIdx;
static unsigned int  aacWriteIdx;

/* Decode status */
static unsigned int aacFrameStep;
static unsigned int aacFrameAccu;
static int aacNChans;
static int aacSampRateOut;
static int lastRound;
static int eofReached;
static unsigned int nFrames;
static int enable_dec = 1;

// input data decode
static int nReadBuffers;
static int nKeepByte;
static int nKeep;

static int nNumber ;

static unsigned int gnAACDropTime;

///////////////////////////////////////////////////////////////////////////
//                              Function Decleration
///////////////////////////////////////////////////////////////////////////
static int  FillReadBuffer(HAACDecoder aacDecInfo, int nReadBytes, int init);
static void FillWriteBuffer(HAACDecoder aacDecInfo, int nPCMBytes);
static void ClearRdBuffer(void);
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
void AudioPluginAPI(int nType);
int ithPrintf(char* control, ...);
#endif
/**************************************************************************************
 * Function     : updateTime
 *
 * Description  : Update Decoding Time
 *
 * Input        : None
 *
 * Output       : None
 *
 * Note         :
 **************************************************************************************/
static __inline void updateTime(AACDecInfo* hAACDecoder)
{
    #if defined(USE_PTS_EXTENSION)
        int pts_upd   = 0;
        int pts_wridx = MMIO_Read(MMIO_PTS_WRIDX);
        int pts_hi    = MMIO_Read(MMIO_PTS_HI);
        int pts_lo    = ((int)MMIO_Read(MMIO_PTS_LO))&0xffff;
        int cur_rdptr = MMIO_Read(DrvDecode_RdPtr);
    #endif // USE_PTS_EXTENSION

    #if defined(ADTS_EXTENSION_FOR_PTS)
    // The PTS is the start time of current frame, so it must adjust the
    // decoding time before increment it.
    AACDecInfo *aacDecInfo = (AACDecInfo *)hAACDecoder;
    if (hAACDecoder->PTS != -1 && aacDecInfo->format == AAC_FF_ADTS)
    {
        int time = ((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16);
        if ((hAACDecoder->PTS - time) >= 2)
        {
            decTime = decTime + (((hAACDecoder->PTS - time - 1) << 16) /  1000);
            PRINTF("#%d: Adjust %d ms (%d->%d) %d\n", nFrames, hAACDecoder->PTS-time,
                   time,
                   (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                   hAACDecoder->PTS);
        }
        else if ((time - hAACDecoder->PTS) >= 2)
        {
            decTime = decTime - (((time - hAACDecoder->PTS - 1) << 16) /  1000);
            PRINTF("#%d: Adjust -%d ms (%d->%d) %d\n", nFrames, time-hAACDecoder->PTS, time,
                   (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                   hAACDecoder->PTS);
        }
        else
        {
            PRINTF("#%d: (%d->%d)\n", nFrames, time, hAACDecoder->PTS);
        }
    }
    #endif // defined(ADTS_EXTENSION_FOR_PTS)

    #if 0 // defined(ACCURATED_TIME)
    int rdptr = GetOutRdPtr();
    int wrptr = GetOutWrPtr();
    int i2sUsedLen = (rdptr > wrptr) ? (sizeof(pcmWriteBuf) - (rdptr - wrptr)) : (wrptr - rdptr);
    int i2sSampls  = (aacNChans == 2) ? i2sUsedLen / 4 : i2sUsedLen / 2;

    aacFrameAccu += (aacFrameStep & 0xf);
    decTime = decTime + (aacFrameStep >> 4) + (aacFrameAccu >> 4) - ((i2sSampls << 16) / aacSampRateOut);
    aacFrameAccu = aacFrameAccu & 0xf;
    #else
        aacFrameAccu += (aacFrameStep & 0x7fff);
        decTime = decTime + (aacFrameStep >> 15) + (aacFrameAccu >> 15);
        aacFrameAccu = aacFrameAccu & 0x7fff;
    #endif // defined(ACCURATED_TIME)

    #if defined(USE_PTS_EXTENSION)
        if (pts_wridx || pts_hi || pts_lo) 
        {      // Get the PTS info
            if (cur_rdptr == 0)
            { // In guard region, ignore
                MMIO_Write(MMIO_PTS_WRIDX, 0);       //   the PTS
                MMIO_Write(MMIO_PTS_HI, 0);
                MMIO_Write(MMIO_PTS_LO, 0);
            }
            else if (cur_rdptr > last_rdptr) 
            {
                if ((cur_rdptr >= pts_wridx && last_rdptr <= pts_wridx) || (cur_rdptr == pts_wridx))
                {
                    pts_upd = 1;
                }
            }
            else if(cur_rdptr < last_rdptr)
            {
                if (cur_rdptr >= pts_wridx || last_rdptr <= pts_wridx)
                {
                    pts_upd = 1;
                }
            }
        }

        if (pts_upd) 
        {
            MMIO_Write(MMIO_PTS_WRIDX, 0);
            MMIO_Write(MMIO_PTS_HI, 0);
            MMIO_Write(MMIO_PTS_LO, 0);
            unsigned int pts = (((pts_hi % AAC_HI_BIT_OVER_FLOW_THRESHOLD) << 16) + pts_lo);
            unsigned int time = ((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16);
            unsigned int timeGap = 0;
            unsigned int ptsOverFlowSection = pts_hi / AAC_HI_BIT_OVER_FLOW_THRESHOLD;
            unsigned int bPtsGreaterThanDec = 0;
        
            if (AAC_INVALID_PTS_OVER_FLOW_VALUE == gLastPtsOverFlowSection)
            {
                gLastPtsOverFlowSection = ptsOverFlowSection;
            }
    
            if (ptsOverFlowSection < gLastPtsOverFlowSection) // hit bit decrement(possible??)
            {
                timeGap = (AAC_WRAP_AROUND_THRESHOLD - pts) + time;
            }
            else
            {
                // hi bit increment. the accuray is only 16 bit ms. more is ignored due to
                // the gap is too huge and not catchable.

                // pts is wrapped around but decode time not.
                if (ptsOverFlowSection > gLastPtsOverFlowSection
                 && (pts + time) >= AAC_JUDGE_MAXIMUM_GAP)
                {
                    timeGap = pts + (AAC_WRAP_AROUND_THRESHOLD - time);
                    bPtsGreaterThanDec = 1;
                }
                else
                {
                    if (pts > time)
                    {
                        timeGap = pts - time;
                        bPtsGreaterThanDec = 1;
                    }
                    else
                        timeGap = time - pts;
                }
            }
    
            if (timeGap >= 100)
            {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
                ithPrintf("[AAC] #decTime: 0x%08X, time: %u, pts: %u, full_pts: %u, bPtsGreaterThanDec: %d, time_gap: %u\n",
                       decTime,
                       time,
                       pts,
                       ((pts_hi << 16) + pts_lo),
                       bPtsGreaterThanDec,
                       timeGap);
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
#else
                printf("[AAC] #decTime: 0x%08X, time: %u, pts: %u, full_pts: %u, bPtsGreaterThanDec: %d, time_gap: %u\n",
                       decTime,
                       time,
                       pts,
                       ((pts_hi << 16) + pts_lo),
                       bPtsGreaterThanDec,
                       timeGap);
#endif

                if (bPtsGreaterThanDec)
                {
                    //                  (high 16 bit / 1000 << 16) to avoid overflow problem.
                    decTime = decTime + ((((timeGap & 0xFFFF0000) / 1000) << 16) + (((timeGap & 0xFFFF) << 16) / 1000));
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
                    ithPrintf("[AAC] (%d) #PTS: Adjust +%d ms (%d->%d) %u\n", __LINE__, timeGap,
                           time,
                           (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                           ((pts_hi << 16) + pts_lo));
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
#else                    
                    printf("[AAC] (%d) #PTS: Adjust +%d ms (%d->%d) %u\n", __LINE__, timeGap,
                           time,
                           (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                           ((pts_hi << 16) + pts_lo));
#endif
                }
                else
                {
                    decTime = decTime - ((((timeGap & 0xFFFF0000) / 1000) << 16) + (((timeGap & 0xFFFF) << 16) / 1000));
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
                    ithPrintf("[AAC] (%d) #PTS: Adjust -%d ms (%d->%d) %u\n", __LINE__, timeGap, time,
                           (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                           ((pts_hi << 16) + pts_lo));
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
#else                    
                    printf("[AAC] (%d) #PTS: Adjust -%d ms (%d->%d) %u\n", __LINE__, timeGap, time,
                           (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                           ((pts_hi << 16) + pts_lo));
#endif
                }
            }
            gLastPtsOverFlowSection = ptsOverFlowSection;
        }
        last_rdptr = MMIO_Read(DrvDecode_RdPtr);
    
    #endif // USE_PTS_EXTENSION
    MMIO_Write(AUDIO_PROCESSOR_WRITE_REGISTER_PROTECTION, 1);                    

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
        eofReached = isEOF() || isSTOP();
        curPause = isPAUSE();
        if (!curPause)
        {   // Un-pause
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
        {  // Pause
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
            //MMP_Sleep(5);
            #else
            //or32_delay(1); // delay 1ms
            #endif
        }
        prePause = curPause;
    } while(!eofReached);

    prePause = curPause;
}

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
#if defined(__OR32__) && !defined(__OPENRTOS__)
    if (0xffff == wrPtr) asm volatile("l.trap 15");
#endif

    return (wrPtr + READBUF_BEGIN);
#else
    return sizeof(streamBuf);
#endif
}

/**************************************************************************************
 * Function     : FillReadBuffer
 *
 * Description  : Update the read pointer of AAC Buffer and return the valid data length
 *                of input buffer (AAC Stream data)
 *
 * Inputs       : nReadBytes: number of bytes to read
 *
 * Global var   : aacWriteIdx: write pointer of AAC buffer
 *                aacReadIdx : read pointer of AAC buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The AAC buffer is circular buffer.
 *
 **************************************************************************************/
static int FillReadBuffer(HAACDecoder aacDecInfo, int nReadBytes, int init) 
{
    int len = 0;
#if defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
    int waitNBytes = 0;
    int offset = 0;
    int nFrameLength = 0;
    int nTemp;
    int nAacType;
    int nTempFrameLength = 0;    
    int nKeepIndex;
    int i;
    enum
    {
        UNKNOWN_FORMAT   = 0,
        WAIT_ADTS_HEADER = 1,
        WAIT_ADTS_DATA   = 2,
        WAIT_ADIF_DATA   = 3,
        WAIT_LATM_DATA   = 4,
        WAIT_LATM_HEADER = 5,
        WAIT_NEXT_SYNC   =6
    } readState;
#endif

    //nNumber += nReadBytes;
    // Update Read Buffer
    if (nReadBytes > 0){
        #if defined(__INPUT_CRC_CHECK__)
        {
            static int crc = 0;
            crc = crc32(&streamBuf[aacReadIdx], nReadBytes);
            PRINTF("CRC: 0x%08x\n", crc);
        }
        #endif // defined(__INPUT_CRC_CHECK__)

        aacReadIdx = aacReadIdx + nReadBytes; // update aacReadIdx each frame

        // It never happen.
        if (aacReadIdx > READBUF_SIZE){
            printf("[AAC] aacReadIdx(%d) > READBUF_SIZE(%d) \n",aacReadIdx,READBUF_SIZE);
            PRINTF("aacReadIdx(%d) READBUF_SIZE(%d) nReadBytes(%d)\n",
                    aacReadIdx, READBUF_SIZE, nReadBytes);
            aacReadIdx = aacReadIdx - READBUF_SIZE + READBUF_BEGIN;
            ASSERT("AAC: aacReadIdx > READBUF_SIZE");
        }
    #if defined(__FREERTOS__) || defined(__OPENRTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

    #if defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
        if (aacReadIdx >= READBUF_GUARD && MMIO_Read(DrvDecode_WrPtr) < MMIO_Read(DrvDecode_RdPtr)) // do memory move when exceed guard band
    #else
        if (aacReadIdx >= READBUF_GUARD) // do memory move when exceed guard band
    #endif // defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
        {
            // Update Read Pointer
            MMIO_Write(DrvDecode_RdPtr, (unsigned short)(((aacReadIdx-READBUF_BEGIN)>>1) << 1));

            // Wait the aacWriteIdx get out of the area of guard band.
            do {
                checkControl();
                aacWriteIdx = getStreamWrPtr();

                if (aacWriteIdx >= aacReadIdx && !eofReached) {
                    #if defined(__FREERTOS__)
                        #ifdef AAC_FLOW_CONTROL
                            PalSleep(2);
                            tContextSwitch = PalGetClock();

                        #else
                            PalSleep(2);
                        #endif                    
                    #else
                    //or32_delay(1); // enter sleep mode for power saving
                    #endif
                } else {
                    break;
                }
            } while(1);

            if (aacWriteIdx < aacReadIdx) {
                // Move the rest data to the front of data buffer
                memcpy(&streamBuf[aacReadIdx-READBUF_GUARD], &streamBuf[aacReadIdx], READBUF_SIZE-aacReadIdx);
                aacReadIdx = aacReadIdx - READBUF_GUARD;
                MMIO_Write(DrvDecode_RdPtr, 0);
            }
        } else if (aacReadIdx >= READBUF_BEGIN) {
            MMIO_Write(DrvDecode_RdPtr, (unsigned short)(((aacReadIdx-READBUF_BEGIN)>>1) << 1));
        }
    }

    #if defined(__FREERTOS__) || defined(__OPENRTOS__)
        dc_invalidate(); // Flush Data Cache
    #endif

    #if defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
        if (((AACDecInfo*)aacDecInfo)->format == AAC_FF_Unknown) {    
            readState  = UNKNOWN_FORMAT;
            waitNBytes = 4;
        } else if (((AACDecInfo*)aacDecInfo)->format == AAC_FF_ADTS) {    
            readState  = WAIT_ADTS_HEADER;
            waitNBytes = 16;
        } else if (((AACDecInfo*)aacDecInfo)->format == AAC_FF_LATM) {    
            readState  = WAIT_LATM_HEADER;
            waitNBytes = 3;
        } else { // AAC_FF_ADIF or AAC_FF_RAW    
            readState  = WAIT_ADIF_DATA;
            waitNBytes = AAC_MAINBUF_SIZE;
        }
        nAacType = ((AACDecInfo*)aacDecInfo)->format;
    #endif // defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)

    // It should be invalidate the cache line which it is in the
    // begin of input buffer. The previous frame will
    // prefetch the data to the cache line, but the driver dose
    // not yet put it in the input buffer. It will cause the
    // unconsistency of cache.
    #if !defined(__OPENRTOS__)
        or32_invalidate_cache(&streamBuf[aacReadIdx], 4);
    #endif

    {
        if (aacWriteIdx >= aacReadIdx) {
            len = aacWriteIdx - aacReadIdx;
        } else {
            len = READBUF_GUARD - (aacReadIdx - aacWriteIdx);
        }

        if (len < AAC_MAINBUF_SIZE/2) {
            //printf("!!!!!!!!!!!!! AAC input buffer underflow !!!!!!!!!!!!!!!\n");
        }
    }

    #if defined(__FREERTOS__)
        PAL_CLOCK_T tSleep;
        tSleep = PalGetClock();
    #endif

    nReadBuffers = aacReadIdx;
    // Wait Read Buffer avaliable
    do {
    #if defined(__FREERTOS__)
            if(PalGetDuration(tSleep)>5)
            {
                PalSleep(2);            
        #ifdef AAC_FLOW_CONTROL
                tContextSwitch = PalGetClock();        
        #endif                                    
                tSleep = PalGetClock();                
            }

        if(init==0) {
            if(len > sizeof(streamBuf)/2) {
                    break;
                }
            }
    #endif        
        
    #if defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
        int done = 0;
        unsigned char *inbuf = &streamBuf[aacReadIdx];
    #endif // defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)

        checkControl();
        aacWriteIdx = getStreamWrPtr();
        if (aacWriteIdx >= aacReadIdx) {
            len = aacWriteIdx - aacReadIdx;
        } else {
            len = READBUF_GUARD - (aacReadIdx - aacWriteIdx);
            PRINTF("total:%d, aacReadIDx: %d, aacWriteIdx: %d, DrvRdPtr: %d, len: %d, waitNBytes: %d\n", READBUF_SIZE, aacReadIdx, MMIO_Read(DrvDecode_RdPtr), aacWriteIdx-READBUF_BEGIN, len, waitNBytes);
            #if defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
                if(aacReadIdx > READBUF_GUARD && len >= waitNBytes) {
                    memcpy(&streamBuf[aacReadIdx-READBUF_GUARD], &streamBuf[aacReadIdx], READBUF_SIZE-aacReadIdx);
                    aacReadIdx = aacReadIdx - READBUF_GUARD;
                    MMIO_Write(DrvDecode_RdPtr, 0);
                }
            #endif
        }
        
    #if defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
        if ((!eofReached) && (len < waitNBytes)) {
    #else
        if ((!eofReached) && (len < AAC_MAINBUF_SIZE)) {
    #endif // defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
    #if defined(__FREERTOS__) || defined(__OPENRTOS__)
        dc_invalidate(); // Flush Data Cache
#endif
    
    #if defined(__FREERTOS__)
            PalSleep(2);
        #ifdef AAC_FLOW_CONTROL
            tContextSwitch = PalGetClock();        
        #endif                    
    #else //defined(__FREERTOS__)
            //or32_delay(1); // enter sleep mode for power saving
    #endif  //defined(__FREERTOS__)
        }
        else
        {
            if (eofReached && len <= 0) {
                lastRound = 1;
            }
            
    #if defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
            PRINTF("state: %d, len: %d, waitNBytes: %d\n", readState, len, waitNBytes);
            switch(readState) 
            {
                case UNKNOWN_FORMAT  : 
                {
                    if (inbuf[0] == 'A' && inbuf[1] == 'D' && inbuf[2] == 'I' && inbuf[3] == 'F') {                            
                        waitNBytes = AAC_MAINBUF_SIZE;
                        readState  = WAIT_ADIF_DATA;
                    } else if ( inbuf[0] == 0x56 && (inbuf[1] & 0xe0) == 0xe0) {                            
                        waitNBytes = 20; // wait the minimun size of one frame.
                        readState  = WAIT_LATM_HEADER;
                    } else if( inbuf[0] == 0xff && (inbuf[1] & 0xf6) == 0xf0 ) {                            
                        waitNBytes = 16; // wait the minimun size of one frame.
                        readState  = WAIT_ADTS_HEADER;
                    } else {
                        aacReadIdx++;
                    }
                    break;
                }                    
                case WAIT_ADTS_HEADER: 
                {
                    if(len == 0) {
                        break;
                    }
                    len = (aacReadIdx + len > READBUF_SIZE ) ? (READBUF_SIZE - aacReadIdx) : len;
                    offset = AACFindSyncWord(inbuf, len);
                    if(offset < 0) {
                        PRINTF("Cannot find syncword, aacReadIdx:%d, DrvRdPtr:%d, offset:%d, len:%d\n", aacReadIdx, MMIO_Read(DrvDecode_RdPtr), offset, len);
                        aacReadIdx += len;
                        aacReadIdx = aacReadIdx < READBUF_SIZE ? aacReadIdx : READBUF_BEGIN;
                        MMIO_Write(DrvDecode_RdPtr, (unsigned short)(((aacReadIdx-READBUF_BEGIN)>>1) << 1));
                        readState = UNKNOWN_FORMAT;
                        break;
                    }
                    PRINTF("aacReadIdx:%d, DrvRdPtr:%d, offset:%d, len:%d\n", aacReadIdx, MMIO_Read(DrvDecode_RdPtr), offset, len);
                    aacReadIdx += offset;
                    if(offset + 5 > len) {
                        break;
                    }

                    waitNBytes = (((inbuf[offset+3] << 16) + (inbuf[offset+4] <<8) + (inbuf[offset+5])) >> 5) & 0x1fff;
                    // wait next adts header
                    waitNBytes+=7;                        
                    nFrameLength = ADTSSyncInfo(&inbuf[offset]);
                    nAacType = AAC_FF_ADTS;
                    readState = WAIT_NEXT_SYNC;
                    #if !defined(__OPENRTOS__)
                    // Clear the cache line
                    or32_invalidate_cache(&inbuf[offset+5], 1);
                    #endif
                    break;
                }
                case WAIT_LATM_HEADER: 
                {
                    if(len == 0) {
                        break;
                    }
                    len = (aacReadIdx + len > READBUF_SIZE ) ? (READBUF_SIZE - aacReadIdx) : len;
                    nFrameLength = ((inbuf[1] & 0x1f) << 8) + inbuf[2];
                    nFrameLength+=3;
                    waitNBytes =nFrameLength+3;
                    nAacType = AAC_FF_LATM;
                    readState = WAIT_NEXT_SYNC;
                    break;
                }
                case WAIT_ADTS_DATA  :
                {
                    done = 1;
                    if (((AACDecInfo*)aacDecInfo)->nEnableRecompense==1)
                    {
                    #ifdef AAC_RECOMPENSE_TIME
                        printf("[AAC] nKeepIndex %d index %d (%d) avg frame %d ",nKeepIndex,aacReadIdx,(aacReadIdx-nKeepIndex),((AACDecInfo*)aacDecInfo)->nAvgFrameLength);
                        nTemp = aacReadIdx-nKeepIndex;                            
                        i = 5;
                        if (aacReadIdx-nKeepIndex > 0)
                        {
                            // Recompense audio time
                            do
                            {
                                updateTime((AACDecInfo*)aacDecInfo);
                                nTemp -= ((AACDecInfo*)aacDecInfo)->nAvgFrameLength;
                                i--;
                            }while(nTemp>0 && i >0);
                            //printf("update %d \n",5-i);
                        }
                    #endif    
                        ((AACDecInfo*)aacDecInfo)->nEnableRecompense=0;
                    }
                    break;
                }
                case WAIT_ADIF_DATA  : 
                {
                    done = 1;
                    break;
                }
                case WAIT_LATM_DATA  : 
                {
                    done = 1;
                    break;
                }
                case WAIT_NEXT_SYNC:
                { //WAIT_NEXT_SYNC
                    //printf("[aac] wait next sync nFrameLength %d len %d \n",nFrameLength,len);
                    if(len == 0) break;
                    len = (aacReadIdx + len > READBUF_SIZE ) ? (READBUF_SIZE - aacReadIdx) : len;
                    if(nFrameLength > MAX_AAC_FRAME_LENGTH) {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                        ithPrintf("[AAC] nFrameLength > %d %d\n",MAX_AAC_FRAME_LENGTH,nFrameLength);
                        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                        printf("[AAC] nFrameLength > %d %d\n",MAX_AAC_FRAME_LENGTH,nFrameLength);
#endif                                         
                      nKeepIndex = aacReadIdx;
                      ((AACDecInfo*)aacDecInfo)->nEnableRecompense=1;
                        readState = UNKNOWN_FORMAT;
                        aacReadIdx++;
                    } else {
                        ((AACDecInfo*)aacDecInfo)->nFrameLength = nFrameLength;
                        nKeep = nFrameLength;
                        ((AACDecInfo*)aacDecInfo)->nTempLength+=((AACDecInfo*)aacDecInfo)->nFrameLength;
                        //printf("temp %d length %d \n",((AACDecInfo*)aacDecInfo)->nTempLength,((AACDecInfo*)aacDecInfo)->nFrameLength);
                        if (nFrames%5==0) {
                            ((AACDecInfo*)aacDecInfo)->nAvgFrameLength = ((AACDecInfo*)aacDecInfo)->nTempLength/5;
                            ((AACDecInfo*)aacDecInfo)->nTempLength = 0;
                        }

                    }
                    if (nFrameLength>len) {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
        ithPrintf("[AAC] WAIT_NEXT_SYNC nFrameLength %d > len %d \n",nFrameLength,len);
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
#else

#endif
                        break;
                    }

                    if (nFrameLength + aacReadIdx >= sizeof(streamBuf)) {
                        nTemp = sizeof(streamBuf)-(nFrameLength+aacReadIdx);
                    } else {
                        nTemp = nFrameLength;
                    }

                    // wait next adts or latm header
                    if (nAacType == AAC_FF_ADTS) {
                        if (inbuf[nTemp]==0xff && (inbuf[nTemp+1] & 0xf6) == 0xf0){
                            readState = WAIT_ADTS_DATA;
                        } else
                        {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                            ithPrintf("[AAC] adts UNKNOWN_FORMAT,read again %d nFrameLength %d,nTemp %d, 0x%x 0x%x \n",aacReadIdx,nFrameLength,nTemp,inbuf[nTemp],inbuf[nTemp+1]);
                            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else

#endif
                            ((AACDecInfo*)aacDecInfo)->format = AAC_FF_Unknown;
                            readState = UNKNOWN_FORMAT;
                            aacReadIdx++;
                        }
                    }
                    else if (nAacType == AAC_FF_LATM)
                    {
                        if (inbuf[nTemp]==0x56 && (inbuf[nTemp+1] & 0xe0) == 0xe0)
                        {
                            readState = WAIT_LATM_DATA;

                            nTempFrameLength = ((inbuf[nTemp+1] & 0x1f) << 8) + inbuf[nTemp+2];
                            if (nTempFrameLength>MAX_AAC_FRAME_LENGTH)
                            {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                                ithPrintf("[AAC] nTempFrameLength > %d %d\n",MAX_AAC_FRAME_LENGTH,nFrameLength);
                                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                                printf("[AAC] nTempFrameLength > %d %d\n",MAX_AAC_FRAME_LENGTH,nFrameLength);
#endif                                         
                            }
                        }
                        else
                        {
                            ((AACDecInfo*)aacDecInfo)->format = AAC_FF_Unknown;
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                            ithPrintf("[AAC] latm UNKNOWN_FORMAT,read again \n");
                            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                            printf("[AAC] latm UNKNOWN_FORMAT,read again \n");                            
#endif                                         
                            readState = UNKNOWN_FORMAT;
                            aacReadIdx++;
                        }
                    }

                    break;
                }//WAIT_NEXT_SYNC
            }
            
            PRINTF("len=%d, wait %d bytes\n", len, waitNBytes);
            if (eofReached || done)
                break;
    #else
        break;
    #endif // defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
        }
    }while (1);

    if (aacReadIdx-nReadBuffers>=0)
    {
        if (aacReadIdx-nReadBuffers>0)
        {
            //printf("[AAC] nReadBuffers %d \n",aacReadIdx-nReadBuffers);
        }
        nReadBuffers = aacReadIdx-nReadBuffers;
    }
    else
    {
        //printf("[AAC] nReadBuffers %d %d %d\n",aacReadIdx,nReadBuffers,nReadBuffers-aacReadIdx);
    }
    //PRINTF("aacWriteIdx(%d) aacReadIdx(%d) len(%d) nReadBytes(%d)\n", aacWriteIdx, aacReadIdx, len, nReadBytes);

    return len;
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
static void FillWriteBuffer(HAACDecoder aacDecInfo, int nPCMBytes) {
    int len;

#if defined(__OUTPUT_CRC_CHECK__)
    {
        static int crc = 0;
        crc = crc32(&pcmWriteBuf[pcmWriteIdx], nPCMBytes);
        PRINTF("CRC: 0x%08x\n", crc);
    }
#endif // defined(__OUTPUT_CRC_CHECK__)


    if (pcmReadIdx <= pcmWriteIdx){
        len = (pcmWriteIdx - pcmReadIdx);
    } else {
        len = sizeof(pcmWriteBuf) - (pcmReadIdx - pcmWriteIdx);
    }

    if (len < MAX_FRAME_SIZE/2)  {
        //printf("!!!!!!!!!!!!! AAC IIS buffer underflow !!!!!!!!!!!!!!!\n");
    }
#if defined(__FREERTOS__)
    #ifdef AAC_FLOW_CONTROL
    if( ((AACDecInfo*)aacDecInfo)->nChans <=2  )
    {
        if (len > sizeof(pcmWriteBuf)*2/3 )
        {  // decode aac enough,return 
            PalSleep(1);
            tContextSwitch = PalGetClock();
        }
    }
    #endif // AAC_FLOW_CONTROL
#endif //defined(__FREERTOS__)
    // Update Write Buffer
    if (nPCMBytes > 0)
    {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(AAC_DUMP_PCM)
        gnTemp = nPCMBytes;
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_FILE_WRITE);
#endif // defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(AAC_DUMP_PCM)
        pcmWriteIdx = pcmWriteIdx + nPCMBytes;

#if defined(OUTPUT_MEMMODE)
        if (pcmWriteIdx >= sizeof(pcmWriteBuf))
        {
            pcmWriteIdx = sizeof(pcmWriteBuf);
        }
        SetOutWrPtr(pcmWriteIdx);
        MMIO_Write(AUDIO_PROCESSOR_WRITE_REGISTER_PROTECTION, 0);                    

        return;
#else
        if (pcmWriteIdx >= sizeof(pcmWriteBuf))
        {
            //pcmWriteIdx -= sizeof(pcmWriteBuf);
            pcmWriteIdx = 0;
        }
        CODEC_I2S_SET_OUTWR_PTR(pcmWriteIdx);
        //SetOutWrPtr(pcmWriteIdx);
        MMIO_Write(AUDIO_PROCESSOR_WRITE_REGISTER_PROTECTION, 0);                    
        
#endif // defined(OUTPUT_MEMMODE)
    }

    // Wait output buffer avaliable
    do
    {
        checkControl();
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

        if ((len-2) < MAX_FRAME_SIZE && !isSTOP())
        {
#if defined(__FREERTOS__)
            PalSleep(2);
#else // defined(__FREERTOS__)
            //or32_delay(1); // enter sleep mode for power saving
#endif //defined(__FREERTOS__)
        }
        else
        {
            break;
        }
    } while(1);

    // PRINTF("pcmWriteIdx(%d) pcmReadIdx(%d) len(%d) nPCMBytes(%d)\n", pcmWriteIdx, pcmReadIdx, len, nPCMBytes);
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
static void ClearRdBuffer(void) 
{
#if defined(OUTPUT_MEMMODE)
    exit(-1);
#endif

    if (isEOF() && !isSTOP()) 
    {
#if defined(DUMP_PCM_DATA)
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) | DrvDecodePCM_EOF);
#endif // defined(DUMP_PCM_DATA)

        // wait I2S buffer empty
        do 
        {
            if (1) //(GetOutRdPtr() == GetOutWrPtr()) 
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
        while(MMIO_Read(DrvAudioCtrl) & DrvDecodePCM_EOF) 
        {
    #if defined(__FREERTOS__)
            PalSleep(2);
    #else //defined(__FREERTOS__)
            //or32_delay(1);
    #endif //defined(__FREERTOS__)
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

#if defined(USE_PTS_EXTENSION)
    last_rdptr = 0;
    MMIO_Write(MMIO_PTS_WRIDX, 0);
    MMIO_Write(MMIO_PTS_HI, 0);
    MMIO_Write(MMIO_PTS_LO, 0);
    MMIO_Write(DrvDecode_Frame  , 0);
    MMIO_Write(DrvDecode_Frame+2, 0);
#endif // USE_PTS_EXTENSION

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    //AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_DAC);
#else
    deactiveDAC();   // Disable I2S interface
#endif
    if (isEOF()) 
    {
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvDecode_EOF);
    }

    if (isSTOP()) 
    {
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvDecode_STOP);
    }


#if defined(__FREERTOS__) || defined(__OPENRTOS__)
    dc_invalidate(); // Flush Data Cache
#endif
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
            //printf("[AAC] name length %d \n",gnTemp);
            memcpy(&gtAudioPluginBuffer[0], &gnTemp, SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH);                           
            memcpy(&gtAudioPluginBuffer[SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH/sizeof(short)], tDumpWave, sizeof(tDumpWave));
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_WRITE:
          // printf("[AAC] name length %d \n",gnTemp);           
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
                //printf("[AAC] 0x%x %d %d %d \n",nTemp,gCh,gSampleRate,nTemp1);
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC:
                pBuf = (unsigned char*)gtAudioPluginBuffer;
                memcpy(&pBuf[0], &gPause, sizeof(int));
                //printf("[AAC] pause %d \n",gPause);
            break;
        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_DAC:
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
            //printf("[AAC] get main procerror feedback \n");
            break;
        }
        i--;
    }while(i);
    //if (i==0)
        //printf("[AAC] audio api %d %d\n",i,nType);

}
 #endif

#if defined(__FREERTOS__)
/**************************************************************************************
 * Function     : AAC_GetBufInfo
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
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
void AAC_GetBufInfo(unsigned* inbuf, unsigned* inlen, unsigned* audioPluginBuf, unsigned* audioPluginBufLen)
{
#if 0
    *inbuf = (unsigned)&streamBuf[READBUF_GUARDSIZE];
    *inlen = READBUF_GUARD;
    gtAudioPluginBuffer = (unsigned short*)audioPluginBuf;
    gtAudioPluginBufferLength = audioPluginBufLen;
    
    printf("[AAC] plugin buffer length %d  0x%x buf 0x%x \n",*gtAudioPluginBufferLength,audioPluginBufLen,audioPluginBuf);
#endif    
}

#else
void AAC_GetBufInfo(unsigned* inbuf, unsigned* inlen)
{
    *inbuf = (unsigned)&streamBuf[READBUF_GUARDSIZE];
    *inlen = READBUF_GUARD;
}
#endif //defined(AUDIO_PLUGIN_MESSAGE_QUEUE)

#endif //defined(__FREERTOS__)

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
portTASK_FUNCTION(aacdecode_task, params)
#else
int main(int argc, char **argv)
#endif
{
    int err;
    int bytesLeft;
    int bInitDAC;
    int nReadBytes;
    int nPCMBytes;
    int targetSamps;
#if defined(VOICEOFF)
    int enVoiceOff, preVoiceOff;
#endif // VOICEOFF
    int* codecStream;
    AUDIO_CODEC_STREAM* audioStream;

#ifdef AAC_PERFORMANCE_TEST_BY_TICK
    int  nNewTicks,nTotalTicks;
#endif
    int nTemp;
#if defined(ENABLE_SBC)
    static sbc_t sbc;
#endif // defined(ENABLE_SBC)
    AACDecInfo *aacDecInfo = NULL;

    HAACDecoder *hAACDecoder = NULL;
    nNumber = 0;
#if defined(REVERBERATION)
    reverbStateNow  = 0;
    reverbStatePrev = 0;
#endif // defined(REVERBERATION)

#if defined(WIN32) || defined(__CYGWIN__)
    GetParam(argc, argv);
    win32_init();
#endif // defined(WIN32) || defined(__CYGWIN__)

#if defined(ENABLE_PERFORMANCE_MEASURE)
    {
        int i;
        for(i=0; i<sizeof(time_log)/sizeof(int); i++) 
        {
            time_log[i]=0;
        }
    }
#endif //defined(ENABLE_PERFORMANCE_MEASURE)

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    codecStream=CODEC_STREAM_START_ADDRESS;   
    //printf("[AAC] 0x%08x \n",*codecStream);
    audioStream = *codecStream;
    audioStream->codecStreamBuf = &streamBuf[READBUF_BEGIN];
    audioStream->codecStreamLength = READBUF_GUARD;   
    //printf("[AAC] audioStream 0x%08x 0x%08x 0x%08x   \n",&audioStream,&audioStream->codecStreamBuf,&audioStream->codecStreamLength);
    gtAudioPluginBuffer = audioStream->codecAudioPluginBuf;
    gtAudioPluginBufferLength = audioStream->codecAudioPluginLength;
    //printf("[AAC] 0x%08x %d 0x%08x %d \n",audioStream->codecStreamBuf,audioStream->codecStreamLength ,audioStream->codecAudioPluginBuf,audioStream->codecAudioPluginLength);
    MMIO_Write(AUDIO_DECODER_START_FALG, 1);
#endif

    while(1)
    { /* forever loop */
        bInitDAC    = 0;
#if !defined(INPUT_MEMMODE)
        aacReadIdx  = READBUF_BEGIN;
        aacWriteIdx = READBUF_BEGIN;
#else //!(INPUT_MEMMODE)
        aacReadIdx  = 0;
        aacWriteIdx = 0;
#endif //!(INPUT_MEMMODE)
        pcmWriteIdx = 0;
        pcmReadIdx  = 0;
        bytesLeft   = 0;
        eofReached  = 0;
        nFrames     = 0;
        nReadBytes  = 0;
        nPCMBytes   = 0;
        lastRound   = 0;
        targetSamps = 0;
        decTime     = 0;
        err = 0;
        aacFrameAccu= 0;

#if defined(VOICEOFF)
        enVoiceOff  = 0;
        preVoiceOff = 0;
#endif // VOICEOFF

#if defined(__INPUT_CRC_CHECK__) || defined(__OUTPUT_CRC_CHECK__)
        crc32_init();
#endif // defined(__INPUT_CRC_CHECK__) || defined(__OUTPUT_CRC_CHECK__)

        /* initializer AAC Decoder */
        hAACDecoder = (HAACDecoder *)AACInitDecoder();
#if defined(RAW_BLOCK)
       AACSetRawBlockParams(hAACDecoder, 0, &aacFrameInfo);
#endif

#if defined(DRCTL)
        memset(drcoutbuf, 0, sizeof(drcoutbuf));
#endif

        MMIO_Write(DrvDecode_Frame  , 0);
        MMIO_Write(DrvDecode_Frame+2, 0);
#ifdef AAC_PERFORMANCE_TEST
        nDecPerformance=0;
        nDownmixingPerformance = 0;
#endif //AAC_PERFORMANCE_TEST
#ifdef AAC_PERFORMANCE_TEST_BY_TICK
        nTotalTicks=0;
#endif //AAC_PERFORMANCE_TEST_BY_TICK
       
#ifdef AAC_FLOW_CONTROL
       tContextSwitch = PalGetClock();
#endif //AAC_FLOW_CONTROL                

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)    
        ithPrintf("[AAC] %d %d start 0x%x 0x%x \n",aacReadIdx,READBUF_BEGIN,streamBuf[aacReadIdx],streamBuf[aacReadIdx+1]);
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
#else
        
#endif

        do
        { // AAC Decode Loop
#if 1 && defined(ENABLE_PERFORMANCE_MEASURE)
            start_timer();
#endif

            // Update the write buffer and wait the write buffer (PCM data) avaliable.
            // If the I2S interface dose not initialize yet, the read/write pointer is on the wrong
            // position.
            if (bInitDAC && !err){
                // accumulate two frames to decode
                // if (nFrames > 2) FillWriteBuffer(nPCMBytes);
                gnAACDropTime = MMIO_Read(AUDIO_DECODER_DROP_DATA);
                if (gnAACDropTime>0){
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                    ithPrintf("[AAC]drop data \n");
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                    printf("[AAC]drop data \n");
#endif                                         
                }else{               
                    FillWriteBuffer(hAACDecoder,nPCMBytes);
                }
            }
            MMIO_Write(AUDIO_PROCESSOR_WRITE_REGISTER_PROTECTION, 0);                    

            // Updates the read buffer and returns the avaliable size of input buffer (AAC stream).
            bytesLeft = FillReadBuffer(hAACDecoder, nReadBytes, bInitDAC);
            if (isSTOP()){
                break;
            }

            // if (lastRound && bytesLeft <= 0) break;
            if (lastRound){
                break;
            }
            /* Decode one AAC Frame */
            if (enable_dec)
            {
#ifdef AAC_PERFORMANCE_TEST
                tClockPerformance = PalGetClock();
#endif //AAC_PERFORMANCE_TEST
#ifdef AAC_PERFORMANCE_TEST_BY_TICK
                start_timer();
#endif           
                unsigned char *inbuf;
                int prebyteLeft;
                prebyteLeft = bytesLeft;
                inbuf = &streamBuf[aacReadIdx];

                //#if 1 // Check the Sync Word on ADTS frame.
#if !defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
                if (((AACDecInfo*)hAACDecoder)->format == AAC_FF_ADTS)
                {
                    int nSyncBytes;
                    nSyncBytes = AACFindSyncWord(inbuf, bytesLeft);

                    if (nSyncBytes < 0)
                    {
                        nReadBytes = bytesLeft;
                        nPCMBytes  = 0;
                        if (eofReached) lastRound = 1;
                        continue;
                    }
                    else
                    {
                        inbuf += nSyncBytes;
                        bytesLeft -= nSyncBytes;
                    }
                }
#endif //!defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
                nTemp = bytesLeft; 
#ifdef ENABLE_DOWNMIX_CHANNELS
                    if (isEnableOutputEmpty() ){
                        err = AACDecode(hAACDecoder, &inbuf, &bytesLeft, emptyBuffer);
                    }
                    else if(aacFrameInfo.nChans>2)
                    {
                        err = AACDecode(hAACDecoder, &inbuf, &bytesLeft, (short *)&pcmDownMixBuf[0]);
                    }
                    else
                    {
                        err = AACDecode(hAACDecoder, &inbuf, &bytesLeft, (short *)&pcmWriteBuf[pcmWriteIdx]);
                    }
#else //ENABLE_DOWNMIX_CHANNELS
                    if (isEnableOutputEmpty() ){
                        err = AACDecode(hAACDecoder, &inbuf, &bytesLeft, emptyBuffer);
                    }
                    else if {
                        err = AACDecode(hAACDecoder, &inbuf, &bytesLeft, (short *)&pcmWriteBuf[pcmWriteIdx]);
                    }
#endif //ENABLE_DOWNMIX_CHANNELS
                
#ifdef AAC_RESET_DECODED_BYTE    
                if (isResetAudioDecodedBytes())
                {
                    MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvResetAudioDecodedBytes);
                    nNumber += nKeepByte;                    
                    nKeepByte = 0;
                    //MMIO_Write(DrvAudioDecodedBytes,(unsigned short)nKeepByte);                            
                    PalSleep(5);
                }            
                // write AAC decoded byte to register
                if (nTemp > bytesLeft)
                {
                    nKeepByte += nTemp-bytesLeft;
                    nKeepByte += nReadBuffers;
                    //MMIO_Write(DrvAudioDecodedBytes, (unsigned short)nKeepByte); 
                }
                else
                {
                    nKeepByte += nKeep;
                    nKeepByte += nReadBuffers;
                    //MMIO_Write(DrvAudioDecodedBytes, (unsigned short)nKeepByte);                 
                   // printf("[AAC] nKeepByte %d <=  bytesLeft %d\n",nTemp,bytesLeft);
                }
#endif  // def AAC_RESET_DECODED_BYTE       
                
#ifdef AAC_PERFORMANCE_TEST
                if(!err)
                {
                    nDecPerformance+= PalGetDuration(tClockPerformance);
                }
                if ( nFrames % 500 == 0 )
                {
                    printf("[AAC] average of decode time %d nDecPerformance %d nFrames %d \n",(nDecPerformance/nFrames),nDecPerformance,nFrames );
                }
#endif //AAC_PERFORMANCE_TEST
#ifdef AAC_PERFORMANCE_TEST_BY_TICK
                nNewTicks = get_timer();            
                nTotalTicks += nNewTicks;
                if (nFrames % 100 == 0 && nFrames>0)
                {
                    ithPrintf("[AAC] (%d~%d) total %d (ms) average %d (ms) nFrames %d system clock %d\n",(nFrames+1-100),(nFrames+1),(nTotalTicks/(PalGetSysClock()/1000)),((nTotalTicks/(PalGetSysClock()/1000))/100),nFrames+1,PalGetSysClock());
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
                    nTotalTicks=0;                    
                }
#endif //AAC_PERFORMANCE_TEST_BY_TICK

#if defined(__FREERTOS__)
    #ifdef AAC_FLOW_CONTROL
                if(PalGetDuration(tContextSwitch)>40)
                {
                    PalSleep(2);
                    tContextSwitch = PalGetClock();
                 }
    #else //AAC_FLOW_CONTROL                       
                PalSleep(2);
    #endif  //AAC_FLOW_CONTROL                                  
#endif  // define freertos
               
                nReadBytes = prebyteLeft - bytesLeft;
                PRINTF("#%d: %d bytes\n", nFrames, nReadBytes);
            }

            //PRINTF("#%04d: error %d %d\r", nFrames, err, nReadBytes);

            if (err && err!=ERR_AAC_LATM_PARSING_ERROR) 
            {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                ithPrintf("[AAC] dec error %d ,main.c \n", err);
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                printf("[AAC] dec error %d ,main.c \n", err);
#endif                                         
            

                aacDecInfo = (AACDecInfo *)hAACDecoder;

                if(aacDecInfo->format == AAC_FF_LATM)
                {
                    aacReadIdx += aacDecInfo->nFrameLength+LOAS_HEADER_SIZE;
                    AACFlushCodec(hAACDecoder);
                    continue;
                }
                PRINTF("Error Code: %d\n", err);

                ////////////////////////////////////////////
                // Error Recovery, find the next sync word.
                ////////////////////////////////////////////
                #if defined(ERROR_RECOVER)
                {
                    #if 0 && defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
                        // We do not need to find syncword if we use FINDSYNCWORD on FILLBUFFER
                        if (aacReadIdx < aacWriteIdx)
                        {
                            nReadBytes = bytesLeft;
                        }
                        else
                        {
                            nReadBytes = READBUF_SIZE - aacReadIdx;
                        }
                    #else
                        int nSyncBytes;
                        nSyncBytes = AACFindSyncWord(&streamBuf[aacReadIdx], bytesLeft);
                        if (nSyncBytes == 0 && bytesLeft >= 16)
                        {
                            nReadBytes = 16;
                        }
                        else if (nSyncBytes <= 0)
                        {
                            nReadBytes = bytesLeft;
                        }
                        else
                        {
                            nReadBytes = nSyncBytes;
                        }
                    #endif
                }

                nPCMBytes = 0;
                if (eofReached)
                {
                    lastRound = 1;
                }
                AACFlushCodec(hAACDecoder); // flush internal codec state when
                // error occurs.
                continue; // Skip error

                ////////////////////////////////////////////
                // Stop the OpenRISC when the error occurs.
                ////////////////////////////////////////////
                #else
                    if (eofReached) 
                        break;

                    report_error("AAC Decode error"); // break point

                    /* error occurred */
                    if (err == ERR_AAC_INDATA_UNDERFLOW) 
                    {
                        /* need to provide more data on next call to AACDecode() (if possible) */
                        if (eofReached || bytesLeft == READBUF_SIZE) break;
                    }
                #endif
            }
            else if (err==ERR_AAC_LATM_PARSING_ERROR)
            {
                 AACDecInfo *aacDecInfo = (AACDecInfo *)hAACDecoder;
                 aacReadIdx += aacDecInfo->nFrameLength+LOAS_HEADER_SIZE;
                 AACFlushCodec(hAACDecoder); // flush internal codec state when
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                ithPrintf("[AAC] main ERR_AAC_LATM_PARSING_ERROR \n");
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                 printf("[AAC] main ERR_AAC_LATM_PARSING_ERROR \n");
#endif                                         
                 
                 continue;
            }

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
        //ithPrintf("[AAC] nFrames %d \n",nFrames);
        //AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
        printf("[AAC] nFrames \n",nFrames);        
#endif
            
            /* no error */
            AACGetLastFrameInfo(hAACDecoder, &aacFrameInfo);

            /* Initialize DAC */
            if (!bInitDAC && !err)
            {
                targetSamps = aacFrameInfo.outputSamps;
                bInitDAC = 1;
                #if defined(ENABLE_SBC)
                    sbc_init(&sbc, SBC_NULL);
                    sbc.rate     = aacFrameInfo.sampRateOut;
                    sbc.channels = aacFrameInfo.nChans;
                    sbc.subbands = 8;
                    sbc.joint    = 0;
                    sbc.blocks   = 16;
                    sbc.bitpool  = 32;
                #else
                #  if !defined(DUMP_PCM_DATA)
                   #ifndef SUPPORT_MORE_THAN_2_CHANNELS
                       if(aacFrameInfo.nChans>=2 && aacFrameInfo.nChans<=AAC_MAX_NCHANS)
                       {
                          nTemp = 2;
                       }
                       else
                       {
                          nTemp = aacFrameInfo.nChans;
                       }
                   #else
                       nTemp=aacFrameInfo.nChans;
                   #endif
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)            
                    gCh = nTemp;
                    gSampleRate = aacFrameInfo.sampRateOut;
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC);
#else
                    //printf("[AAC] init dac ch %d sampRate %d \n",nTemp,aacFrameInfo.sampRateOut);                   
                    initDAC(pcmWriteBuf, nTemp, aacFrameInfo.sampRateOut, sizeof(pcmWriteBuf), 0);
#endif                   

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(AAC_DUMP_PCM)
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_FILE_OPEN);
#endif // defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(AAC_DUMP_PCM)
                   
                #  endif // defined(DUMP_PCM_DATA)
                #endif // defined(ENABLE_SBC)

                aacNChans = aacFrameInfo.nChans;
                aacSampRateOut = aacFrameInfo.sampRateOut;
               #ifndef SUPPORT_MORE_THAN_2_CHANNELS
                   if(aacFrameInfo.nChans>=2 && aacFrameInfo.nChans<=AAC_MAX_NCHANS)
                   {
                       nTemp = 2;
                   }
                   else
                   {
                       nTemp = aacFrameInfo.nChans;
                   }
               #else
                   nTemp=aacFrameInfo.nChans;
               #endif
               #ifdef ENABLE_DOWNMIX_CHANNELS
                  nTemp=aacFrameInfo.nChans;
               #endif
                {
                    // aacFrameStep is 1.31 format.
                    int nSamps   = (aacNChans == 2) ? aacFrameInfo.outputSamps / 2 : aacFrameInfo.outputSamps;
                    unsigned long long n = ((unsigned long long)nSamps) << 31;
                    aacFrameStep = (unsigned int)(n / aacSampRateOut);
                    //printf("aacFrameStep %d aacSampRateOut %d \n",aacFrameStep,nSamps);
                }
            }

            if (aacNChans != aacFrameInfo.nChans && aacFrameInfo.nChans>0 && aacFrameInfo.nChans <=AAC_MAX_NCHANS)            
            {
                aacNChans = aacFrameInfo.nChans;
                bInitDAC=0;
            }
            if (aacSampRateOut != aacFrameInfo.sampRateOut  && aacFrameInfo.sampRateOut >0 && aacFrameInfo.sampRateOut<=96000)
            {
                aacSampRateOut = aacFrameInfo.sampRateOut ;
                bInitDAC=0;
            }
                       
            // Can not change the output samples on the fly, it will make the output buffer out of order.
            //if (targetSamps != aacFrameInfo.outputSamps)
            //{
            //    nPCMBytes = 0;
            //    bInitDAC = 1;                
            //}
            //else
            {
                #ifdef ENABLE_DOWNMIX_CHANNELS
                    AACDecInfo *aacDecInfo = (AACDecInfo *)hAACDecoder;
                    if(aacFrameInfo.nChans>=2)
                    {
                        nPCMBytes = 2* (aacFrameInfo.bitsPerSample/8)* AAC_MAX_NSAMPS * (aacDecInfo->sbrEnabled ? 2 : 1);
                    }
                    else
                    {
                        nPCMBytes = (aacFrameInfo.bitsPerSample/8)* AAC_MAX_NSAMPS * (aacDecInfo->sbrEnabled ? 2 : 1);
                    }
                #else
                    nPCMBytes = aacFrameInfo.bitsPerSample / 8 * aacFrameInfo.outputSamps;
                #endif
            }

/* Voice-Off Function */
#if defined(VOICEOFF)
            enVoiceOff = (isVoiceOff() != 0);
            if (!preVoiceOff && enVoiceOff)
            {
                initVoiceOff(aacFrameInfo.sampRateOut);
            }
            if (enVoiceOff)
            {
                voiceOff((short int *)&pcmWriteBuf[pcmWriteIdx]);
            }
            preVoiceOff = enVoiceOff;
#endif //defined(VOICEOFF)

#if defined(LITTLE_ENDIAN_PCM) && defined(DUMP_PCM_DATA)
            // Byte swap to little endian
            {
                int i;
                char tmp;
                char *buf = (char *)&pcmWriteBuf[pcmWriteIdx];
                for(i=0; i<aacFrameInfo.outputSamps*aacFrameInfo.nChans*2; i+=2)
                {
                    tmp      = buf[i];
                    buf[i] = buf[i+1];
                    buf[i+1] = tmp;
                }
            }
#endif

            if (isDownSample() && aacFrameInfo.nChans == 2)
            {
                int i;
                short *buf = (short *)&pcmWriteBuf[pcmWriteIdx];
                for(i=0; i<aacFrameInfo.outputSamps; i+=2)
                {
                    int tmp = ((int)buf[i] + (int)buf[i+1]) / 2;
                    buf[i]   = (short)tmp;
                    buf[i+1] = (short)tmp;
                }
            }
            
            // chanage channel outputs
            if (aacFrameInfo.nChans == 2)
            {                 
                int i;
                short *buf = (short *)&pcmWriteBuf[pcmWriteIdx];          
                short ntemp;
                switch (getChMixMode()) 
                {
                    case CH_MIX_NO   :
                        break;
                    case CH_MIX_LEFT :
                         for(i = 0; i < aacFrameInfo.outputSamps/2; i++)
                         {
                             buf[2*i+1] = buf[2*i];
                         }
                         break;
                    case CH_MIX_RIGHT:
                         for(i = 0; i < aacFrameInfo.outputSamps/2; i++)
                         {
                             buf[2*i] = buf[2*i+1];
                         }
                         break;
                    case CH_MIX_BOTH :
                         for(i = 0; i < aacFrameInfo.outputSamps/2; i++)
                         {
                             ntemp = (buf[2*i] + buf[2*i+1])/2;
                             buf[2*i] = ntemp;
                             buf[2*i+1] = ntemp;
                         }
                         break;
                    default:
                         break;
                }
            }         

#ifdef AAC_PERFORMANCE_TEST
          tClockDownmixingPerformance = PalGetClock();
#endif
            
#ifdef ENABLE_DOWNMIX_CHANNELS
           if(aacFrameInfo.nChans > 2 && aacFrameInfo.nChans <=AAC_MAX_NCHANS)
           {
                 //L' = 0.5 x L + 0.3535 x C + 0.433 x SL + 0.25 x SR
                 //R' = 0.5 x R + 0.3535 x C - 0.25 x SL - 0.433 x SR
                 int nLeftChannel;
                 int nRightChannel;
                 int i,j;      // index
                 short *buf = (short *)&pcmWriteBuf[pcmWriteIdx];
                 short *buf1 = (short *)&pcmDownMixBuf[0];
                 if (aacFrameInfo.nChans >= 5 && aacFrameInfo.nChans <=AAC_MAX_NCHANS)
                 {
                     for(i=0,j=0; j<aacFrameInfo.outputSamps; i+=2,j+=aacFrameInfo.nChans) 
                     {
                        // support 5 and 5.1 channels downmix
                        nLeftChannel = (MULSHIFT32((int)buf1[j],3535)>>10) + ((int)buf1[j+1]>>1) + (MULSHIFT32((int)buf1[j+3],433)>>10) + ((int)buf1[j+4]>>2) ;
                        nRightChannel = (MULSHIFT32((int)buf1[j],3535)>>10) + ((int)buf1[j+2]>>1) - ((int)buf1[j+3]>>2) - (MULSHIFT32((int)buf1[j+4],433)>>10);
                        buf[i]   = (short)nLeftChannel;
                        buf[i+1] = (short)nRightChannel;
                     }
                 }
                 else if (aacFrameInfo.nChans > 2 && aacFrameInfo.nChans < 5)
                 {
                     //TODO 3,4 channels downmixing
                     for(i=0,j=0; j<aacFrameInfo.outputSamps; i+=2,j+=aacFrameInfo.nChans) 
                     {
                         buf[i]   = (short)buf1[j];
                         buf[i+1] = (short)buf1[j+1];
                     }

                 }
           }
#endif //ENABLE_DOWNMIX_CHANNELS

#ifdef AAC_PERFORMANCE_TEST
            nDownmixingPerformance+= PalGetDuration(tClockDownmixingPerformance);
            if ( nFrames % 500 == 0 && nFrames>0)
            {
                printf("[AAC] average of nDownmixingPerformance decode time %d nDownmixingPerformance %d nFrames %d \n",(nDownmixingPerformance/nFrames),nDownmixingPerformance,nFrames );
            }
#endif

            checkControl();
            if (isSTOP()){
                break;
            }

            if (isEOF() && bytesLeft <= 0){
                lastRound = 1;
            }

#if 1 && defined(ENABLE_PERFORMANCE_MEASURE)
            {
                int elapsed = get_timer();
                if (nFrames < (sizeof(time_log)/sizeof(int)))
                {
                    time_log[nFrames] = elapsed;
                }
            }
#endif // defined(ENABLE_PERFORMANCE_MEASURE)

            if (enable_dec)
            {
                // update the Decoding time.
                updateTime((AACDecInfo*)hAACDecoder);
                nFrames ++;
            }
        } while (1);

        ClearRdBuffer();
        //printf("[AAC] nNumber %d \n",nNumber);
        nNumber =0;
#ifdef AAC_RESET_DECODED_BYTE    
        // write aac decoded byte to register
        nKeepByte = 0;
       // MMIO_Write(DrvAudioDecodedBytes, (unsigned short)nKeepByte); 
        if (isResetAudioDecodedBytes())
        {
            MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvResetAudioDecodedBytes);
        }
#endif  // def AAC_RESET_DECODED_BYTE       

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(AAC_DUMP_PCM)
    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_FILE_CLOSE);
#endif //defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(AAC_DUMP_PCM)

#ifdef AAC_PERFORMANCE_TEST
     printf("[AAC] Average of decode time %d nDecPerformance %d nFrames %d \n",(nDecPerformance/nFrames),nDecPerformance,nFrames );
#endif
        MMIO_Write(AUDIO_PROCESSOR_WRITE_REGISTER_PROTECTION, 0);                    

        // Only do once on WIN32 platform.
        #if defined(WIN32) || defined(__CYGWIN__)
            break;
        #endif
    } /* end of forever loop */

#if !defined(__OPENRTOS__)
    return 0;
#endif
}

