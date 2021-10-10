/**************************************************************************************
 * Source last modified: $Id: main.c,v 1.2 2006/3/29 14:45:30 $
 *
 * Copyright (c) 1995-2005 SMedia Tech. Corp., All Rights Reserved.
 *
 * AC3 wrapped program
 **************************************************************************************/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "a52.h"
#include "a52_internal.h"

#if defined(__CYGWIN__)
#include "wavfilefmt.h"
#endif
#include "win32.h"

#if defined(__OR32__)
#include "mmio.h"
#include "sys.h"
#endif

#if defined(__INPUT_CRC_CHECK__)
#include "crc32.h"
#endif

#if defined(ENABLE_CODECS_PLUGIN)
#  include "plugin.h"
#endif

///////////////////////////////////////////////////////////////////////////
//                              Constant Definition
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//                              Globale Variable
///////////////////////////////////////////////////////////////////////////
#define AC3_HI_BIT_OVER_FLOW_THRESHOLD      (0x3E8)
#define AC3_INVALID_PTS_OVER_FLOW_VALUE     (0xFFFFFFFF)
#define AC3_WRAP_AROUND_THRESHOLD           (0x3E80000) // 65536 seconds
#define AC3_JUDGE_MAXIMUM_GAP               (0x1F40000) // 36728 seconds

static unsigned int decTime = 0; // Decoding time in seconds on S15.16 fixed point.
static unsigned int gLastPtsOverFlowSection = AC3_INVALID_PTS_OVER_FLOW_VALUE;
static unsigned int ac3FrameStep;
static unsigned int ac3FrameAccu;

static unsigned short nAc3DecodedByte=0;

static unsigned int gnAc3DropTime;

#if defined(ENABLE_PERFORMANCE_MEASURE)
#  include "ticktimer.h"
    #define MAX_TIME_LOG 10000
    unsigned int time_log[MAX_TIME_LOG];
#endif // defined(ENABLE_PERFORMANCE_MEASURE)

#if defined(__GNUC__)
static unsigned char pcmWriteBuf[I2SBUFSIZE] __attribute__ ((aligned(16), section (".sbss")));
#else
static unsigned char pcmWriteBuf[I2SBUFSIZE];
#endif
static unsigned int  pcmReadIdx;
static unsigned int  pcmWriteIdx;

/*************************************************************************************
 * Input Buffer (AC3 Stream data) from driver
 *
 *    |<--------------------- READBUF_SIZE ---------------------->|
 *    +-------------------+---------------------------------------+
 *    |                   |                                       |
 *    |                   |         Driver Input Buffer           |
 *    |                   |                                       |
 *    +-------------------+---------------------------------------+
 *    |<----------------->|<------------------------------------->|
 *      READBUF_BEGIN                    READBUF_LEN
 *                        ^
 *                        |
 *                        |
 *                   READBUF_BEGIN
 *
 *************************************************************************************/
#if defined(INPUT_MEMMODE)
unsigned char streamBuf[] = {
#  include "ac3file.h"
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
static unsigned int  ac3ReadIdx;
static unsigned int  ac3WriteIdx;

/* Decode status */
static int lastRound;
static int eofReached;
unsigned int nFrames;

// vincent: move from updateTime()
static int last_rdptr = 0;

///////////////////////////////////////////////////////////////////////////
//                              Function Decleration
///////////////////////////////////////////////////////////////////////////
/* Function decleration */
static int  AdjustBuffer(int waitNBytes);
static int  FillReadBuffer(int nReadBytes);
static void FillWriteBuffer(int nPCMBytes);
static void ClearRdBuffer(void);

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
static __inline void updateTime(void) {
    #if defined(USE_PTS_EXTENSION)
    // vincent: move to global
    //static int last_rdptr = 0;
    int pts_upd   = 0;
    int pts_wridx = MMIO_Read(MMIO_PTS_WRIDX);
    int pts_hi    = MMIO_Read(MMIO_PTS_HI);
    int pts_lo    = ((int)MMIO_Read(MMIO_PTS_LO))&0xffff;
    int cur_rdptr = MMIO_Read(DrvDecode_RdPtr);
    #endif // USE_PTS_EXTENSION

    ac3FrameAccu += (ac3FrameStep & 0x7fff);
    decTime = decTime + (ac3FrameStep >> 15) + (ac3FrameAccu >> 15);
    ac3FrameAccu = ac3FrameAccu & 0x7fff;

    #if defined(USE_PTS_EXTENSION)
    if (pts_wridx || pts_hi || pts_lo) {         // Get the PTS info
        if (cur_rdptr == 0 && last_rdptr == 0) { // In guard region, ignore
            MMIO_Write(MMIO_PTS_WRIDX, 0);       //   the PTS
            MMIO_Write(MMIO_PTS_HI, 0);
            MMIO_Write(MMIO_PTS_LO, 0);
        } else if (cur_rdptr > last_rdptr) {
            if ((cur_rdptr >= pts_wridx && last_rdptr <= pts_wridx) ||
                (cur_rdptr == pts_wridx))
                pts_upd = 1;
        } else if(cur_rdptr < last_rdptr) {
            if (cur_rdptr >= pts_wridx || last_rdptr <= pts_wridx)
                pts_upd = 1;
        }
    }

#if 1
        if (pts_upd) {
            MMIO_Write(MMIO_PTS_WRIDX, 0);
            MMIO_Write(MMIO_PTS_HI, 0);
            MMIO_Write(MMIO_PTS_LO, 0);
            unsigned int pts = (((pts_hi % AC3_HI_BIT_OVER_FLOW_THRESHOLD) << 16) + pts_lo);
            unsigned int time = ((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16);
            unsigned int timeGap = 0;
            unsigned int ptsOverFlowSection = pts_hi / AC3_HI_BIT_OVER_FLOW_THRESHOLD;
            unsigned int bPtsGreaterThanDec = 0;

            PRINTF("#%d, decTime: 0x%08X, time: %u, pts: %u, full_pts: %u\n",
                   nFrames,
                   decTime,
                   time,
                   pts,
                   ((pts_hi << 16) + pts_lo));

            if (AC3_INVALID_PTS_OVER_FLOW_VALUE == gLastPtsOverFlowSection)
                gLastPtsOverFlowSection = ptsOverFlowSection;

            if (ptsOverFlowSection < gLastPtsOverFlowSection) // hit bit decrement(possible??)
            {
                timeGap = (AC3_WRAP_AROUND_THRESHOLD - pts) + time;
            }
            else
            {
                // hi bit increment. the accuray is only 16 bit ms. more is ignored due to
                // the gap is too huge and not catchable.

                // pts is wrapped around but decode time not.
                if (ptsOverFlowSection > gLastPtsOverFlowSection
                 && (pts + time) >= AC3_JUDGE_MAXIMUM_GAP)
                {
                    timeGap = pts + (AC3_WRAP_AROUND_THRESHOLD - time);
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
                printf("#%d, decTime: 0x%08X, time: %u, pts: %u, full_pts: %u, bPtsGreaterThanDec: %d, time_gap: %u\n",
                       nFrames,
                       decTime,
                       time,
                       pts,
                       ((pts_hi << 16) + pts_lo),
                       bPtsGreaterThanDec,
                       timeGap);

                if (bPtsGreaterThanDec)
                {
                    decTime = decTime + ((((timeGap & 0xFFFF0000) / 1000) << 16) + (((timeGap & 0xFFFF) << 16) / 1000));
                    printf("(%d) #PTS: Adjust +%d ms (%d->%d) %u\n", __LINE__, timeGap,
                           time,
                           (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                           ((pts_hi << 16) + pts_lo));
                }
                else
                {
                    decTime = decTime - ((((timeGap & 0xFFFF0000) / 1000) << 16) + (((timeGap & 0xFFFF) << 16) / 1000));
                    printf("(%d) #PTS: Adjust -%d ms (%d->%d) %u\n", __LINE__, timeGap, time,
                           (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                           ((pts_hi << 16) + pts_lo));
                }
            }
            gLastPtsOverFlowSection = ptsOverFlowSection;
        }
        else
        {
            int time = ((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16);
            PRINTF("#%d, decTime: 0x%08X, time: %u, cur_rd: 0x%08X, last_rd: 0x%08X\n",
                   nFrames,
                   decTime,
                   time,
                   cur_rdptr,
                   last_rdptr);
        }
#else
    if (pts_upd) {
        MMIO_Write(MMIO_PTS_WRIDX, 0);
        MMIO_Write(MMIO_PTS_HI, 0);
        MMIO_Write(MMIO_PTS_LO, 0);
        unsigned int pts = (pts_hi << 16) + pts_lo;
        unsigned int time = ((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16);
        
        PRINTF("#%d, decTime: 0x%08X, time: %u, cur_rd: 0x%08X, last_rd: 0x%08X, wr_idx: 0x%08X, pts: %u\n",
               nFrames,
               decTime,
               time,
               cur_rdptr,
               last_rdptr,
               pts_wridx,
               pts);

        if ((int) (pts - time) >= 40) {
            decTime = decTime + (((pts - time - 1) << 16) /  1000);
            PRINTF("#PTS: Adjust +%d ms (%d->%d) %d\n", pts-time,
                   time,
                   (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                   pts);
        } else if ((int) (time - pts) >= 40) {
            decTime = decTime - (((time - pts - 1) << 16) /  1000);
            PRINTF("#PTS: Adjust -%d ms (%d->%d) %d\n", time-pts, time,
                   (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                   pts);
        } else {
            PRINTF("#PTS: (%d->%d)\n", time, pts);
        }
    }
    else
    {
        int time = ((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16);
        PRINTF("#%d, decTime: 0x%08X, time: %u, cur_rd: 0x%08X, last_rd: 0x%08X\n",
               nFrames,
               decTime,
               time,
               cur_rdptr,
               last_rdptr);

    }
#endif
    last_rdptr = MMIO_Read(DrvDecode_RdPtr);
    #endif // USE_PTS_EXTENSION

    MMIO_Write(DrvDecode_Frame  , (short)( ((unsigned int)decTime) & 0xffff));
    MMIO_Write(DrvDecode_Frame+2, (short)( ((unsigned int)decTime) >> 16)   );
}

static __inline void checkControl(void) {
    static int curPause = 0;
    static int prePause = 0;

    do {
        eofReached  = isEOF() || isSTOP();
        curPause = isPAUSE();
        if (!curPause) {  // Un-pause
            if (prePause) pauseDAC(0);
            break;
        } else { // Pause
            if (!prePause && curPause) {
                pauseDAC(1);
            }
            #if defined(__FREERTOS__)
            taskYIELD();
            #else
            or32_delay(1); // delay 1ms
            #endif
        }
        prePause = curPause;
    } while(!eofReached);

    prePause = curPause;
}

static __inline unsigned int getStreamWrPtr(void) {
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

/**************************************************************************************
 * Function     : AdjustBuffer
 *
 * Description  : move the reset of data to the front of buffer.
 *
 * Inputs       : waitNBytes: number of bytes to read
 *
 * Global var   : ac3WriteIdx: write pointer of AC3 buffer
 *                ac3ReadIdx : read pointer of AC3 buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The AC3 buffer is circular buffer.
 *
 **************************************************************************************/
static int AdjustBuffer(int waitNBytes) {
    int len;

    // It should be invalidate the cache line which it is in the
    // begin of input buffer. The previous frame will
    // prefetch the data to the cache line, but the driver dose
    // not yet put it in the input buffer. It will cause the
    // unconsistency of cache.
    #if !defined(__FREERTOS__)
    or32_invalidate_cache(&streamBuf[ac3ReadIdx], 1);
    #endif

    if (ac3ReadIdx + waitNBytes >= READBUF_SIZE) { // do memory move when exceed guard band
        // Update Read Pointer
        MMIO_Write(DrvDecode_RdPtr, (unsigned short)(((ac3ReadIdx-READBUF_BEGIN)>>1) << 1));

        // Wait the ac3WriteIdx get out of the area of rest data.
        do {
            checkControl();
            ac3WriteIdx = getStreamWrPtr();

            if (ac3WriteIdx >= ac3ReadIdx && !eofReached) {
                #if defined(__FREERTOS__)
                taskYIELD();
                #else
                or32_delay(1); // enter sleep mode for power saving
                #endif
            } else {
                break;
            }
        } while(1);

        if (ac3WriteIdx < ac3ReadIdx) {
            #if defined(__FREERTOS__)
            dc_invalidate(); // Flush Data Cache
            #endif
            // Move the rest data to the front of data buffer
            memcpy(&streamBuf[READBUF_BEGIN - (READBUF_SIZE - ac3ReadIdx)],
                   &streamBuf[ac3ReadIdx], READBUF_SIZE-ac3ReadIdx);
            ac3ReadIdx = READBUF_BEGIN - (READBUF_SIZE - ac3ReadIdx);
        }
    }

    ac3WriteIdx = getStreamWrPtr();

    if (ac3WriteIdx >= ac3ReadIdx) {
        len = ac3WriteIdx - ac3ReadIdx;
    } else {
        len = READBUF_LEN - (ac3ReadIdx - ac3WriteIdx);
    }

    return len;
}

/**************************************************************************************
 * Function     : FillReadBuffer
 *
 * Description  : Update the read pointer of AC3 Buffer and return the valid data length
 *                of input buffer (AC3 Stream data)
 *
 * Inputs       : nReadBytes: number of bytes to read
 *
 * Global var   : ac3WriteIdx: write pointer of AC3 buffer
 *                ac3ReadIdx : read pointer of AC3 buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The AC3 buffer is circular buffer.
 *
 **************************************************************************************/
static int FillReadBuffer(int nReadBytes) {
    int len;
    int waitNBytes;
    int done;

    enum {
        WAIT_HEADER  = 0,
        WAIT_FRAME   = 1
    } readState;

    #if defined(INPUT_MEMMODE)
    ac3ReadIdx += nReadBytes;
    ac3WriteIdx = sizeof(streamBuf);
    if (ac3ReadIdx >= sizeof(streamBuf)) {
        lastRound = 1;
        ac3ReadIdx = sizeof(streamBuf);
        return 0;
    } else {
        return (sizeof(streamBuf) - ac3ReadIdx);
    }
    #endif

    // Update Read Buffer
    if (nReadBytes > 0) {

        PRINTF("nReadBytes: %d\n", nReadBytes);

        #if defined(__INPUT_CRC_CHECK__)
        {
            int crc = crc32(&streamBuf[ac3ReadIdx], nReadBytes);
            PRINTF("CRC: 0x%08x\n", crc);
        }
        #endif // defined(__INPUT_CRC_CHECK__)

        ac3ReadIdx = ac3ReadIdx + nReadBytes; // update ac3ReadIdx each frame

        // It never happen.
        if (ac3ReadIdx > READBUF_SIZE) {
            ac3ReadIdx = ac3ReadIdx - READBUF_SIZE + READBUF_BEGIN;
            PRINTF("AC3: ac3ReadIdx > READBUF_SIZE");
            asm volatile("l.trap 15");
        }

        if (ac3ReadIdx >= READBUF_BEGIN) {
            MMIO_Write(DrvDecode_RdPtr, (unsigned short)(((ac3ReadIdx-READBUF_BEGIN)>>1) << 1));
        } else {
            MMIO_Write(DrvDecode_RdPtr, 0);
        }
    }

    waitNBytes = 7; // wait 7 bytes for frame header.
    done       = 0;
    readState  = WAIT_HEADER;

    // Wait Read Buffer avaliable
    do {
        checkControl();
        len = AdjustBuffer(waitNBytes);

        if ((!eofReached) && (len < waitNBytes)) {
            #if defined(__FREERTOS__)
            taskYIELD();
            #else
            or32_delay(1); // enter sleep mode for power saving
            #endif
        } else {
            int flags, sample_rate, bit_rate, length;
            if (eofReached && len <= 0) lastRound = 1;
            if (done || (eofReached && len <= 0)) break;
            switch(readState) {
                case WAIT_HEADER : length = a52_syncinfo(&streamBuf[ac3ReadIdx],
                                            &flags, &sample_rate, &bit_rate);
                                   if (length < 0) {
                                        // Find next sync info
                                        ac3ReadIdx++;
                                        ac3ReadIdx = ac3ReadIdx < READBUF_SIZE ? ac3ReadIdx : READBUF_BEGIN;
									    if (ac3ReadIdx >= READBUF_BEGIN) {
									        MMIO_Write(DrvDecode_RdPtr, (unsigned short)(((ac3ReadIdx-READBUF_BEGIN)>>1) << 1));
									    } else {
									        MMIO_Write(DrvDecode_RdPtr, 0);
									    }
                                   } else {
                                        readState  = WAIT_FRAME;
                                        waitNBytes = length;
                                   }
                                   done = 0;
                                   break;
                case WAIT_FRAME  : done = 1;
                                   readState = WAIT_HEADER;
                                   break;
            }
        }
    }while (1);

    //PRINTF("ac3WriteIdx(%d) ac3ReadIdx(%d) len(%d) nReadBytes(%d)\n", ac3WriteIdx, ac3ReadIdx, len, nReadBytes);

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
static void FillWriteBuffer(int nPCMBytes) {
    int len;

    #if defined(__OUTPUT_CRC_CHECK__)
    {
        static int crc = 0;
        crc = crc32(&pcmWriteBuf[pcmWriteIdx], nPCMBytes);
        PRINTF("CRC: 0x%08x\n", crc);
    }
    #endif // defined(__OUTPUT_CRC_CHECK__)

    // Update Write Buffer
    if (nPCMBytes > 0) {
        pcmWriteIdx = pcmWriteIdx + nPCMBytes;

        #if defined(OUTPUT_MEMMODE)
        if (pcmWriteIdx >= sizeof(pcmWriteBuf))
            pcmWriteIdx = sizeof(pcmWriteBuf);

        SetOutWrPtr(pcmWriteIdx);
        return;
        #else
        if (pcmWriteIdx >= sizeof(pcmWriteBuf))
            pcmWriteIdx -= sizeof(pcmWriteBuf);

        SetOutWrPtr(pcmWriteIdx);
        #endif // defined(OUTPUT_MEMMODE)
    }
    updateTime();

    // Wait output buffer avaliable
    do {
        checkControl();
        pcmReadIdx = GetOutRdPtr();

        if (pcmReadIdx <= pcmWriteIdx) {
            len = sizeof(pcmWriteBuf) - (pcmWriteIdx - pcmReadIdx);
        } else {
            len = pcmReadIdx - pcmWriteIdx;
        }

        if ((len-2) < MAXFRAME && !isSTOP()) {
            #if defined(__FREERTOS__)
            taskYIELD();
            #else
            or32_delay(1); // enter sleep mode for power saving
            #endif
        } else {
            break;
        }
    } while(1);

    //PRINTF("pcmWriteIdx(%d) pcmReadIdx(%d) len(%d) nPCMBytes(%d)\n", pcmWriteIdx, pcmReadIdx, len, nPCMBytes);
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
static void ClearRdBuffer(void) {

    #if defined(OUTPUT_MEMMODE)
    exit(-1);
    #endif

    if (isEOF() && !isSTOP()) {
        #if defined(DUMP_PCM_DATA)
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) | DrvDecodePCM_EOF);
        #endif // defined(DUMP_PCM_DATA)

        // wait I2S buffer empty
        do {
            if (GetOutRdPtr() == GetOutWrPtr()) {
                break;
            } else {
                #if defined(__FREERTOS__)
                taskYIELD();
                #else
                or32_delay(1); // enter sleep mode for power saving
                #endif
            }
        } while(1);

        #if defined(DUMP_PCM_DATA)
        while(MMIO_Read(DrvAudioCtrl) & DrvDecodePCM_EOF) {
            #if defined(__FREERTOS__)
            taskYIELD();
            #else
            or32_delay(1);
            #endif
        }
        #endif // defined(DUMP_PCM_DATA)
    }

    MMIO_Write(DrvDecode_WrPtr, 0);
    MMIO_Write(DrvDecode_RdPtr, 0);
    
    // vincent: reset last_rdptr
    last_rdptr = 0;

    #if defined(USE_PTS_EXTENSION)
    MMIO_Write(MMIO_PTS_WRIDX, 0);
    MMIO_Write(MMIO_PTS_HI, 0);
    MMIO_Write(MMIO_PTS_LO, 0);
    #endif // USE_PTS_EXTENSION

    #if !defined(WIN32) && !defined(__CYGWIN__)
    MMIO_Write(DrvDecode_Frame  , 0);
    MMIO_Write(DrvDecode_Frame+2, 0);
    #endif // !defined(WIN32) && !defined(__CYGWIN__)

    #if defined(DUMP_PCM_DATA)
    SetOutWrPtr(0);
    SetOutRdPtr(0);
    #endif // !defined(DUMP_PCM_DATA)

    if (isEOF()) {
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvDecode_EOF);
    }

    if (isSTOP()) {
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvDecode_STOP);
    }

    deactiveDAC();   // Disable I2S interface
    #if defined(__FREERTOS__)
    dc_invalidate(); // Flush DC Cache
    #endif
}

#if defined(__FREERTOS__)
/**************************************************************************************
 * Function     : AC3_GetBufInfo
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
void AC3_GetBufInfo(unsigned* inbuf, unsigned* inlen)
{
    *inbuf = (unsigned)&streamBuf[READBUF_BEGIN];
    *inlen = READBUF_LEN;
}
#endif

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
portTASK_FUNCTION(ac3decode_task, params)
#else
int main(int argc, char **argv)
#endif
{
    int flags;
    int err;
    int bytesLeft;
    int prebyteLeft;
    int bInitDAC;
    int nReadBytes;
    int nPCMBytes;
    static a52_state_t * state;

    #if defined(WIN32) || defined(__CYGWIN__)
    GetParam(argc, argv);
    win32_init();
    #endif // defined(WIN32) || defined(__CYGWIN__)

    #if defined(ENABLE_PERFORMANCE_MEASURE)
    {
        int i;
        for(i=0; i<sizeof(time_log)/sizeof(int); i++) {
            time_log[i]=0;
        }
    }
    #endif

    while(1) { /* forever loop */
        bInitDAC    = 0;
        #if !defined(INPUT_MEMMODE)
        ac3ReadIdx = READBUF_BEGIN;
        ac3WriteIdx= READBUF_BEGIN;
        #else
        ac3ReadIdx  = 0;
        ac3WriteIdx = 0;
        #endif
        pcmWriteIdx = 0;
        pcmReadIdx  = 0;
        bytesLeft   = 0;
        prebyteLeft = 0;
        eofReached  = 0;
        nFrames     = 0;
        nReadBytes  = 0;
        nPCMBytes   = 0;
        lastRound   = 0;
        decTime     = 0;
        err         = 0;
        flags       = 0;
        ac3FrameAccu= 0;

        #if defined(__INPUT_CRC_CHECK__) || defined(__OUTPUT_CRC_CHECK__)
        crc32_init();
        #endif // defined(__INPUT_CRC_CHECK__) || defined(__OUTPUT_CRC_CHECK__)

        MMIO_Write(DrvDecode_Frame  , 0);
        MMIO_Write(DrvDecode_Frame+2, 0);

        /* initializer AC3 Decoder */
        state = a52_init ();
        if (state == NULL) break;

        do { // AC3 Decode Loop
            #if defined(ENABLE_PERFORMANCE_MEASURE)
            start_timer();
            #endif // defined(ENABLE_PERFORMANCE_MEASURE)

            // Update the write buffer and wait the write buffer (PCM data) avaliable.
            // If the I2S interface dose not initialize yet, the read/write pointer is on the wrong
            // position.
            if (bInitDAC) 
            {
                gnAc3DropTime = MMIO_Read(AUDIO_DECODER_DROP_DATA);
                if (gnAc3DropTime>0)
                {
                    printf("[AC3]drop data \n");
                    updateTime();                    
                }
                else
                {               
                    FillWriteBuffer(nPCMBytes);
                }           
            }

            // Updates the read buffer and returns the avaliable size of input buffer (AC3 stream).
            bytesLeft = FillReadBuffer(nReadBytes);

            if (isSTOP() || lastRound) break;

            PRINTF("Frames : %d\n", nFrames);

            /* Decode one AC3 Frame */
            {
                unsigned char *inbuf, *outbuf;
                prebyteLeft = bytesLeft;
                inbuf = &streamBuf[ac3ReadIdx];
                outbuf = &pcmWriteBuf[pcmWriteIdx];
                err = a52_decode (state, &flags, inbuf, &bytesLeft, outbuf);
                nReadBytes = prebyteLeft - bytesLeft;
                nPCMBytes  = OUTPUT_BYTES;
            }
#ifdef AC3_RESET_DECODED_BYTE    
            if (isResetAudioDecodedBytes())
            {
                MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvResetAudioDecodedBytes);
                nAc3DecodedByte = 0;
                MMIO_Write(DrvAudioDecodedBytes, nAc3DecodedByte);    

                PalSleep(5);
            }                        
            // write ac3 decoded byte to register
            if (nReadBytes)
            {
                nAc3DecodedByte += nReadBytes;
                MMIO_Write(DrvAudioDecodedBytes, nAc3DecodedByte); 
            }
            else
            {
                //printf("[AC3] nReadBytes %d <=  0 %d\n",StreamBuf.rdptr,nKeepByte);
            }
#endif  // def AC3_RESET_DECODED_BYTE       

            //PRINTF("#%04d: error %d %d\n", nFrames, err, nReadBytes);

            if (err) {
                PRINTF("Error Code: %d\n", err);
                nPCMBytes  = 0;
                nReadBytes = 1;
                if (eofReached) lastRound = 1;
                continue; // Skip error
            }

            /* Initialize DAC */
            if (!bInitDAC) {
                // ac3FrameStep is 1.31 format.
                unsigned long long n = ((unsigned long long)OUTPUT_SAMPLES) << 31;
                ac3FrameStep = (unsigned int)(n / state->sample_rate);

                #if !defined(DUMP_PCM_DATA)
                initDAC(pcmWriteBuf, OUTPUT_CHANNELS, state->sample_rate, sizeof(pcmWriteBuf), 0);
                #endif // !defined(DUMP_PCM_DATA)
                bInitDAC = 1;
            }

            #if defined(LITTLE_ENDIAN_PCM) && defined(DUMP_PCM_DATA)
            // Byte swap to little endian
            {
                int i;
                char tmp;
                char *buf = (char *)&pcmWriteBuf[pcmWriteIdx];
                for(i=0; i<OUTPUT_BYTES; i+=2) {
                    tmp      = buf[i  ];
                    buf[i  ] = buf[i+1];
                    buf[i+1] = tmp;
                }
            }
            #endif

            nFrames++;
            checkControl();

            if (isSTOP()) break;
            if (isEOF() && bytesLeft <= 0) lastRound = 1;

            #if defined(ENABLE_PERFORMANCE_MEASURE)
            {
                int elapsed = get_timer();
                if (nFrames < (sizeof(time_log)/sizeof(int)))
                        time_log[nFrames] = elapsed;
            }
            #endif // defined(ENABLE_PERFORMANCE_MEASURE)
            PalSleep(1);
        } while (1);

        a52_free(state);
        ClearRdBuffer();
#ifdef AC3_RESET_DECODED_BYTE    
	    if (isResetAudioDecodedBytes())
	    {
	        MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvResetAudioDecodedBytes);
	    }
	    nAc3DecodedByte = 0;
	    MMIO_Write(DrvAudioDecodedBytes, nAc3DecodedByte);                            
#endif  // def AC3_RESET_DECODED_BYTE        

        // Only do once on WIN32 platform.
        #if defined(WIN32) || defined(__CYGWIN__)
        break;
        #endif // defined(WIN32) || defined(__CYGWIN__)
    } /* end of forever loop */

#if !defined(__FREERTOS__)
    return 0;
#endif
}

