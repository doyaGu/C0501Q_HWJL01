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
#include <stdarg.h>

#include "ac3dec.h"

#if defined(__CYGWIN__)
#include "wavfilefmt.h"
#endif
#include "win32.h"

#if defined(__OR32__)
#include "mmio.h"
#endif

#if defined(__INPUT_CRC_CHECK__)
#include "crc32.h"
#endif

#if defined(ENABLE_CODECS_PLUGIN)
#  include "plugin.h"
#endif

//#define TVA_TEST
///////////////////////////////////////////////////////////////////////////
//                              Constant Definition
///////////////////////////////////////////////////////////////////////////
/* compression mode */
enum { GBL_COMP_CUSTOM_0=0, GBL_COMP_CUSTOM_1, GBL_COMP_LINE, GBL_COMP_RF };
enum { GBL_STEREOMODE_AUTO=0, GBL_STEREOMODE_SRND, GBL_STEREOMODE_STEREO };
enum { GBL_STEREODMIX_LTRT=0, GBL_STEREODMIX_LORO, GBL_STEREODMIX_PLII };
/* dual mono downmix mode */
enum { GBL_DUAL_STEREO=0, GBL_DUAL_LEFTMONO, GBL_DUAL_RGHTMONO, GBL_DUAL_MIXMONO };

typedef enum
{
    LFEOUTOFF=0,
    LFEOUTSINGLE=1,
    LFEOUTDUAL=2
} LFEOUTMODES;

/* define verbose mode levels */
typedef enum
{
    VERBOSELEVEL0=0,
    VERBOSELEVEL1,
    VERBOSELEVEL2
} VERBOSEMODELEVEL;

#define		MINSCALEFACTOR      (0)     /* minimum scale factor */
#define		MAXSCALEFACTOR      (100)   /* maximum scale factor */

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
static unsigned int* gtAudioPluginBufferLength;
static unsigned short* gtAudioPluginBuffer;
static int gnTemp;
static unsigned char tDumpWave[] = "C:/ac3_dump.wav";
static int gCh;
static int gSampleRate;
int gPause = 0 ;
int gPrint = 0;
unsigned char *gBuf;
#endif

#define		ERR_NO_ERROR	0		/*!< Error code representing no errors	*/

#define		ERR_INVALID_PCM_WORD_SIZE		1
#define		ERR_INVALID_KARAOKE_MODE		2
#define		ERR_INVALID_DBG_ARG				3
#define		ERR_INVALID_DBGINFO_ARG			4
#define		ERR_INVALID_DBGB4B_ARG			5
#define		ERR_INVALID_COMPR_MODE			6
#define		ERR_INVALID_LFE_MODE			7
#define		ERR_INVALID_OUTPUT_MODE			8
#define		ERR_INVALID_NOUTPUT_CHANS		9
#define		ERR_INVALID_PCM_SCL_FACTOR		10
#define		ERR_INVALID_FRAME_RGN_STRT		11
#define		ERR_INVALID_FRAME_RGN_END		12
#define		ERR_INVALID_STEREO_MODE			13
#define		ERR_INVALID_DUAL_MONO_MODE		14
#define		ERR_INVALID_DRC_CUT				15
#define		ERR_INVALID_DRC_BOOST			16
#define		ERR_INVALID_COMMAND_LINE		17
#define		ERR_DISPLAY_USAGE_ONLY			18
#define		ERR_INVALID_OUTFILE_ARG			19
#define		ERR_INVALID_VERBOSE_MODE		20
#define		ERR_UNKNOWN_INPUTFILE			21
#define		ERR_UNKNOWN_OUTMETADATAFILE		22
#define		ERR_UNKNOWN_OUTPCMFILE			23
#define		ERR_INVALID_FILESUPPRESS_ARG	24
#if defined(SRC)
#define		ERR_INVALID_UPSAMPLE_MODE		26
#endif /* defined(SRC) */
#define     ERR_INVALID_EXTCHAN_ROUTE_ARG   27
#define		ERR_ILLEGAL_CHAN_ROUTE_ARRAY    28
/* output file suppression options */
typedef enum
{
    FILENOTSUPPRESSED=0,
    FILESUPPRESSED
} FILESUPPRESSIONOPTIONS;
//#include "ticktimer.h"

///////////////////////////////////////////////////////////////////////////
//                              Globale Variable
///////////////////////////////////////////////////////////////////////////
#define AC3_HI_BIT_OVER_FLOW_THRESHOLD      (0x3E8)
#define AC3_INVALID_PTS_OVER_FLOW_VALUE     (0xFFFFFFFF)
#define AC3_WRAP_AROUND_THRESHOLD           (0x3E80000) // 65536 seconds
#define AC3_JUDGE_MAXIMUM_GAP               (0x1F40000) // 36728 seconds

/* DSP type definitions */
typedef char					DSPchar;	/*!< DSP char */
typedef unsigned char			DSPuchar;	/*!< DSP unsigned char	*/
typedef short					DSPshort;	/*!< DSP integer */
typedef unsigned short			DSPushort;	/*!< DSP unsigned integer */
typedef int						DSPint;		/*!< Integer */
typedef unsigned int			DSPuint;	/*!< Unsigned integer */
typedef	long					DSPlong;	/*!< DSP long */
typedef unsigned long			DSPulong;	/*!< DSP unsigned long */
typedef double					DSPfract;	/*!< DSP fractional data type */
typedef enum {FALSE=0, TRUE=1}	DSPbool;	/*!< DSP boolean */

typedef struct
{
    const DSPchar *p_ddpinfilename;        /* input file name                  */
    const DSPchar *p_pcmoutfilename;    /* output PCM file name             */
    const DSPchar *p_chanmapoutfilename;    /* output channel map file name     */
    DSPshort outmapon;				    /* output channel map metadata      */
    DSPshort suppresspcmfileout;        /* output PCM file suppression      */
    DSPshort pcmwordtype;               /* output PCM file word type        */
    DSPlong  framestart;                /* frame region start               */
    DSPlong  frameend;                  /* frame region end                 */
    DSPshort verbose;                   /* verbose mode                     */
    DSPshort outnchans;                 /* number of output channels        */
    DSPshort chanrouting[2]; /* channel routing array         */
#if defined(SRC)
    DSPshort upsample;                  /* upsample PCM output              */
#endif /* defined(SRC) */
    DSPshort quitonerr;					/* if 0, continue on process err    */
                                        /* if 1, quit on process err	    */
    DSPshort mixdataout;				/* output mixing/panning metadata   */
} EXECPARAMS;
typedef struct
{
    DSPshort    compmode;               /* dynamic range compression mode   */
    DSPshort	outlfe;                 /* output LFE channel present       */
    DSPshort	outchanconfig;          /* output channel configuration     */
    DSPshort    pcmscale;               /* PCM scale factor                 */
    DSPshort	stereomode;             /* stereo output mode               */
    DSPshort	dualmode;               /* dual mono reproduction mode      */
    DSPshort	dynscalehigh;           /* dynamic range compression cut scale factor   */
    DSPshort	dynscalelow;            /* dynamic range compression boost scale factor */
#if defined(KCAPABLE)
    DSPshort    kmode;					/* Karaoke capable mode             */
#endif /* defined(KCAPABLE) */
#if defined(DEBUG)
    DSPulong	frm_debugflags;         /* Frame debug flags                */
    DSPulong	dec_debugflags;         /* Decode debug flags               */
#endif /* defined(DEBUG) */
} SUBPARAMS;
#define DDSYNCWORD          (0x0B77)
#define DDSYNCWORDREV       (0x770B)

static unsigned int decTime = 0; // Decoding time in seconds on S15.16 fixed point.
static unsigned int gLastPtsOverFlowSection = AC3_INVALID_PTS_OVER_FLOW_VALUE;
static unsigned int ac3FrameStep;
static unsigned int ac3FrameAccu;

static unsigned short nAc3DecodedByte=0;

static unsigned int gnAc3DropTime;

static int gnFadeInSize = 1024;
static int gnFadeIn = 0;
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

static AC3DecodeContext ac3Context;
static unsigned char tempWriteBuf[6144];

static int eac3NChans;
static int eac3SampRateOut;
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
///////////////////////////////////////////////////////////////////////////
static unsigned int  ac3ReadIdx;
static unsigned int  ac3WriteIdx;

/* Decode status */
static int lastRound;
static int eofReached;
unsigned int nFrames;
unsigned int gnAudioBlock;
unsigned int gnPCMBytes;

static int last_rdptr = 0;

///////////////////////////////////////////////////////////////////////////
//                              Function Decleration
///////////////////////////////////////////////////////////////////////////
/* Function decleration */
static int  AdjustBuffer(int waitNBytes);
static int  FillReadBuffer(int nReadBytes);
static void FillWriteBuffer(int nPCMBytes);
static void ClearRdBuffer(void);


#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
static void AudioPluginAPI(int nType);
__inline int ithPrintf(char* control, ...);
#endif
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
    int i;
#if defined(USE_PTS_EXTENSION)
    int pts_upd   = 0;
    int pts_wridx = MMIO_Read(MMIO_PTS_WRIDX);
    int pts_hi    = MMIO_Read(MMIO_PTS_HI);
    int pts_lo    = ((int)MMIO_Read(MMIO_PTS_LO))&0xffff;
    int cur_rdptr = MMIO_Read(DrvDecode_RdPtr);
#endif // USE_PTS_EXTENSION

    // number block is 6 (AC3) ,may be 1,2,3 in E-AC3
    for (i=0;i<ac3Context.num_blocks;i++)
    {
        ac3FrameAccu += (ac3FrameStep & 0x7fff);
        decTime = decTime + (ac3FrameStep >> 15) + (ac3FrameAccu >> 15);
        ac3FrameAccu = ac3FrameAccu & 0x7fff;
    }


#if defined(USE_PTS_EXTENSION)
    if (pts_wridx || pts_hi || pts_lo)
    {         // Get the PTS info
        if (cur_rdptr == 0 && last_rdptr == 0) { // In guard region, ignore
            MMIO_Write(MMIO_PTS_WRIDX, 0);       //   the PTS
            MMIO_Write(MMIO_PTS_HI, 0);
            MMIO_Write(MMIO_PTS_LO, 0);
        }
        else if (cur_rdptr > last_rdptr) 
        {
            if ((cur_rdptr >= pts_wridx && last_rdptr <= pts_wridx) || (cur_rdptr == pts_wridx))
                pts_upd = 1;
        }
        else if(cur_rdptr < last_rdptr) 
        {
            if (cur_rdptr >= pts_wridx || last_rdptr <= pts_wridx)
                pts_upd = 1;
        }

    }

#if 1
        if (pts_upd) 
        {
            unsigned int pts = (((pts_hi % AC3_HI_BIT_OVER_FLOW_THRESHOLD) << 16) + pts_lo);
            unsigned int time = ((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16);
            unsigned int timeGap = 0;
            unsigned int ptsOverFlowSection = pts_hi / AC3_HI_BIT_OVER_FLOW_THRESHOLD;
            unsigned int bPtsGreaterThanDec = 0;

            MMIO_Write(MMIO_PTS_WRIDX, 0);
            MMIO_Write(MMIO_PTS_HI, 0);
            MMIO_Write(MMIO_PTS_LO, 0);

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
                PRINTF("[EAC3]#%d, decTime: 0x%08X, time: %u, pts: %u, full_pts: %u, bPtsGreaterThanDec: %d, time_gap: %u\n",
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
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                    ithPrintf("[E-AC3](%d) #PTS: Adjust +%d ms (%d->%d) %u\n", __LINE__, timeGap,
                           time,
                           (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                           ((pts_hi << 16) + pts_lo));
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                    printf("[EAC3](%d) #PTS: Adjust +%d ms (%d->%d) %u\n", __LINE__, timeGap,
                           time,
                           (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                           ((pts_hi << 16) + pts_lo));
#endif
                    
                }
                else
                {
                    decTime = decTime - ((((timeGap & 0xFFFF0000) / 1000) << 16) + (((timeGap & 0xFFFF) << 16) / 1000));
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                    ithPrintf("[E-AC3](%d) #PTS: Adjust -%d ms (%d->%d) %u\n", __LINE__, timeGap, time,
                           (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                           ((pts_hi << 16) + pts_lo));
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                    printf("[EAC3](%d) #PTS: Adjust -%d ms (%d->%d) %u\n", __LINE__, timeGap, time,
                           (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                           ((pts_hi << 16) + pts_lo));
#endif                    
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
            //taskYIELD();
            PalSleep(1);
            #else
            //or32_delay(1); // delay 1ms
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
#if defined(__FREERTOS__) || defined(__OPENRTOS__)
    dc_invalidate(); // Flush Data Cache
#endif

    if (ac3ReadIdx + waitNBytes >= READBUF_SIZE)
    { // do memory move when exceed guard band
        // Update Read Pointer
        MMIO_Write(DrvDecode_RdPtr, (unsigned short)(((ac3ReadIdx-READBUF_BEGIN)>>1) << 1));

        // Wait the ac3WriteIdx get out of the area of rest data.
        do 
        {
            checkControl();
            ac3WriteIdx = getStreamWrPtr();

            if (ac3WriteIdx >= ac3ReadIdx && !eofReached) 
            {
#if defined(__FREERTOS__)
                //taskYIELD();
                PalSleep(1);
#else
                //or32_delay(1); // enter sleep mode for power saving
#endif
            } else {
                break;
            }
        } while(1 && !isSTOP());

        if (ac3WriteIdx < ac3ReadIdx)
        {
        #if defined(__FREERTOS__) || defined(__OPENRTOS__)
            dc_invalidate(); // Flush Data Cache
            #endif
            // Move the rest data to the front of data buffer
            //printf("[EAC3] memcpy %d %d %d \n",READBUF_BEGIN - (READBUF_SIZE - ac3ReadIdx),ac3ReadIdx,READBUF_SIZE-ac3ReadIdx);
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

unsigned int findSyncWord()
{
    short  syncwd;
    syncwd = (streamBuf[ac3ReadIdx]<<8)|(streamBuf[ac3ReadIdx+1]);
    if (syncwd == DDSYNCWORDREV)
    {
        return ac3ReadIdx;
    }
    else if (syncwd == DDSYNCWORD)
    {
        return ac3ReadIdx;    
    }
    else /* no valid syncword found */
    {
        ac3ReadIdx++;
        ac3ReadIdx = ac3ReadIdx < READBUF_SIZE ? ac3ReadIdx : READBUF_BEGIN;        
        if (ac3ReadIdx >= READBUF_BEGIN) 
        {
            MMIO_Write(DrvDecode_RdPtr, (unsigned short)(((ac3ReadIdx-READBUF_BEGIN)>>1) << 1));
        }
        else
        {
            MMIO_Write(DrvDecode_RdPtr, 0);
        }
        
        return 0;
    }    
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
    int i,findSync;

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
    if (nReadBytes > 0)
    {
        PRINTF("nReadBytes: %d\n", nReadBytes);
#if defined(__INPUT_CRC_CHECK__)
        {
            int crc = crc32(&streamBuf[ac3ReadIdx], nReadBytes);
            PRINTF("CRC: 0x%08x\n", crc);
        }
#endif // defined(__INPUT_CRC_CHECK__)

        ac3ReadIdx = ac3ReadIdx + nReadBytes; // update ac3ReadIdx each frame

        // It never happen.
        if (ac3ReadIdx > READBUF_SIZE) 
        {
            ac3ReadIdx = ac3ReadIdx - READBUF_SIZE + READBUF_BEGIN;
            PRINTF("AC3: ac3ReadIdx > READBUF_SIZE");
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
            ithPrintf("[E-AC3]  ac3ReadIdx > READBUF_SIZE \n");
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
            printf("[EAC3] ac3ReadIdx > READBUF_SIZE \n");
#endif

            //asm volatile("l.trap 15");
        }

        if (ac3ReadIdx >= READBUF_BEGIN) {
            MMIO_Write(DrvDecode_RdPtr, (unsigned short)(((ac3ReadIdx-READBUF_BEGIN)>>1) << 1));
        } else {
            MMIO_Write(DrvDecode_RdPtr, 0);
        }
        
    }
#if defined(__FREERTOS__) || defined(__OPENRTOS__)
        dc_invalidate(); // Flush Data Cache
#endif

    waitNBytes = 7; // wait 7 bytes for frame header.
    done       = 0;
    readState  = WAIT_HEADER;

    // Wait Read Buffer avaliable
    do {
        checkControl();
        //if (readState!=WAIT_HEADER)
        len = AdjustBuffer(waitNBytes);               

        if ((!eofReached) && (len < waitNBytes)) {
#if defined(__FREERTOS__)
            //taskYIELD();
            PalSleep(1);                            
#else
            //or32_delay(1); // enter sleep mode for power saving
#endif
        } else {
            int length;
            if (eofReached && len <= 0) lastRound = 1;
            if (done || (eofReached && len <= 0)) break;
            switch (readState) 
            {
                case WAIT_HEADER :                   
                    length = ac3_syncinfo(&streamBuf[ac3ReadIdx]);                                           
                    if (length < 0)
                    {
                        // Find next sync info
                        ac3ReadIdx++;
                        ac3ReadIdx = ac3ReadIdx < READBUF_SIZE ? ac3ReadIdx : READBUF_BEGIN;
                        if (ac3ReadIdx >= READBUF_BEGIN) 
                        {
                            //printf("[EAC3] ac3ReadIdx %d > READBUF_BEGIN %d \n",ac3ReadIdx,READBUF_BEGIN);
                            MMIO_Write(DrvDecode_RdPtr, (unsigned short)(((ac3ReadIdx-READBUF_BEGIN)>>1) << 1));
                        }
                        else
                        {
                            MMIO_Write(DrvDecode_RdPtr, 0);
                        }
                    }
                    else
                    {
                        readState  = WAIT_FRAME;
                        waitNBytes = length;
                    }
                    done = 0;
                    break;
                case WAIT_FRAME  : 
                    
                    done = 1;
                    readState = WAIT_HEADER;
                    break;
            }
        }
    }while (1 && !isSTOP());

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
static void FillWriteBuffer(int nPCMBytes)
{
    int len;
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
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(AC3_DUMP_PCM)
        gnTemp = nPCMBytes;
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_FILE_WRITE);
#endif // defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(AC3_DUMP_PCM)
        pcmWriteIdx = pcmWriteIdx + nPCMBytes;

#if defined(OUTPUT_MEMMODE)

#else


//if (pcmWriteIdx > sizeof(pcmWriteBuf))
   //printf("nFrames %d %d %d %d\n",nFrames,nPCMBytes,pcmWriteIdx,sizeof(pcmWriteBuf));
        if (pcmWriteIdx >= sizeof(pcmWriteBuf))
        {
#ifdef EAC3_MEMORY_COPY_TO_I2S
            pcmWriteIdx -= sizeof(pcmWriteBuf);
#else            
            pcmWriteIdx = 0;
#endif
#ifdef TVA_TEST        
	        if (lastRound)
	        {                
	            pcmWriteIdx -= 512*(ac3Context.dev_channels);
	        }
#endif        
        }
        //SetOutWrPtr(pcmWriteIdx);
        CODEC_I2S_SET_OUTWR_PTR(pcmWriteIdx);
#endif // defined(OUTPUT_MEMMODE)
    updateTime();

    }
    // Wait output buffer avaliable
    do {
        checkControl();
        //pcmReadIdx = GetOutRdPtr();
        pcmReadIdx = CODEC_I2S_GET_OUTRD_PTR();
        if (pcmReadIdx <= pcmWriteIdx) {
            len = sizeof(pcmWriteBuf) - (pcmWriteIdx - pcmReadIdx);
        } else {
            len = pcmReadIdx - pcmWriteIdx;
        }

        if ((len-2) < MAXFRAME && !isSTOP()) {
            #if defined(__FREERTOS__)
            PalSleep(2);
            #else
            //or32_delay(1); // enter sleep mode for power saving
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

    if (isEOF() && !isSTOP())
    {
#if defined(DUMP_PCM_DATA)
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) | DrvDecodePCM_EOF);
#endif // defined(DUMP_PCM_DATA)

        // wait I2S buffer empty
        /*do
        {
            if (GetOutRdPtr() == GetOutWrPtr())
            {
                break;
            }
            else 
            {
                #if defined(__FREERTOS__)
                //taskYIELD();
                PalSleep(1);
                #else
                //or32_delay(1); // enter sleep mode for power saving
                #endif
            }
        } while(1);*/

    }

    MMIO_Write(DrvDecode_WrPtr, 0);
    MMIO_Write(DrvDecode_RdPtr, 0);
    
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
    

    #if defined(__FREERTOS__)|| defined(__OPENRTOS__)
    dc_invalidate(); // Flush DC Cache
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
            //printf("[E-AC3] name length %d \n",gnTemp);
            memcpy(&gtAudioPluginBuffer[0], &gnTemp, SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH);                           
            memcpy(&gtAudioPluginBuffer[SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH/sizeof(short)], tDumpWave, sizeof(tDumpWave));
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_WRITE:
          // printf("[E-AC3] name length %d \n",gnTemp);           
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
                //printf("[E-AC3] 0x%x %d %d %d \n",nTemp,gCh,gSampleRate,nTemp1);
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC:
                pBuf = (unsigned char*)gtAudioPluginBuffer;
                memcpy(&pBuf[0], &gPause, sizeof(int));
                //printf("[E-AC3] pause %d \n",gPause);
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
            //printf("[Mp3] get main procerror feedback \n");
            break;
        }
        i--;
    }while(i && !isSTOP());
    //if (i==0)
        //printf("[E-AC3] audio api %d %d\n",i,nType);

}

#endif


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
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
void AC3_GetBufInfo(unsigned* inbuf, unsigned* inlen, unsigned* audioPluginBuf, unsigned* audioPluginBufLen)
{
#if 0
    *inbuf = (unsigned)&streamBuf[READBUF_BEGIN];
    *inlen = READBUF_LEN;
    gtAudioPluginBuffer = (unsigned short*)audioPluginBuf;
    gtAudioPluginBufferLength = audioPluginBufLen;    
    //printf("[EAC3] plugin buffer length %d  0x%x buf 0x%x \n",*gtAudioPluginBufferLength,audioPluginBufLen,audioPluginBuf);    
#endif    
}
#else //defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
void AC3_GetBufInfo(unsigned* inbuf, unsigned* inlen)
{
    *inbuf = (unsigned)&streamBuf[READBUF_BEGIN];
    *inlen = READBUF_LEN;
}
#endif //defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
 
#endif

static void EAC3_UpdateParameter()
{
    if (getAC3DRCMode()==0 || getAC3DRCMode()==1)
    {
        ac3Context.dev_compmode = getAC3DRCMode();
    }
    switch (getAC32ChDownmixMode())
    {
        case 0:
            ac3Context.dev_stereomode = AC3_STEREOMODE_STEREO;
            break;
        case 1:
            ac3Context.dev_stereomode = AC3_STEREOMODE_SRND;
            break;
        case 2:
            ac3Context.dev_stereomode = AC3_STEREOMODE_AUTO;
            break;
        default:
            break;
    }
    if (getAC3DualMonoMode()>=0 && getAC3DualMonoMode()<=3)
    {
        ac3Context.dev_dualmode = getAC3DualMonoMode();
    }

    //printf("[EAC3] drc %d 2 ch %d dual  %d mode \n",ac3Context.dev_compmode,ac3Context.dev_stereomode,ac3Context.dev_dualmode);

}
#ifdef TVA_TEST
/*****************************************************************\
* range: parse ranges of the form start[:[end]]
\*****************************************************************/
static short range(
        const DSPchar		*p_inputarg,	/* input	*/
        DSPlong				*p_lower,		/* output	*/
        DSPlong				*p_upper)		/* output	*/
{
    /* declare local variables */
    DSPchar *p_colon;
    /* check input arguments */
    //ERR_CHKARG(p_inputarg);
    //ERR_CHKARG(p_lower);
    //ERR_CHKARG(p_upper);

    if ((p_colon = strchr(p_inputarg, ':')) == NULL)
    {
        *p_lower = atol(p_inputarg);
        *p_upper = -1;
    }
    else
    {
        *p_colon = '\0';
        *p_lower = atol(p_inputarg);
        if (*(p_colon + 1))
        {
            *p_upper = atol(p_colon + 1);
        }
        else
        {
            *p_upper = -1;
        }
    }

    return (ERR_NO_ERROR);

}
/*****************************************************************\
* parsecmdline: parses the command-line input
\*****************************************************************/
static short parsecmdline(
    const int           argc,                         /* input  */
    const char          *p_argv[],                    /* input  */
    EXECPARAMS          *p_execparams,                /* output */
    SUBPARAMS           *p_subparams)                 /* output */
{

    /* declare local variables */
    short		err;
    DSPshort	ch;
    DSPshort	count;
    DSPshort	chanval;
    const DSPchar *p_route;
    const DSPchar *p_input;
    DSPshort	routing_index;
    DSPbool     pcmchan_value_set[2];
#if 0 //defined(WIN32) || defined(__CYGWIN__)
    //char *InputfileName ="D:\\audio_testing\\eac3\\music2.ac3";
    //char *InputfileName ="D:\\audio_testing\\eac3\\kara1.ec3";
    //char *OutputfileName ="D:\\audio_testing\\eac3\\kara1.wav";
    //char *InputfileName = "D:\\CASTOR2\\test_data\\eac3\\chanfigswp_0_7.ec3";
    //char *OutputfileName ="D:\\CASTOR2\\test_data\\eac3\\chanfigswp_0_7_dobly.wav";
    //char *InputfileName = "D:\\CASTOR2\\test_data\\eac3\\samplerate.ec3";
    //char *OutputfileName ="D:\\CASTOR2\\test_data\\eac3\\samplerate_dolby.wav";
    //char *InputfileName = "D:\\CASTOR2\\test_data\\eac3\\2_dynrng_16.ec3";
    //char *OutputfileName ="D:\\CASTOR2\\test_data\\eac3\\2_dynrng_16_dolby.wav";
    //char *InputfileName = "D:\\CASTOR2\\test_data\\eac3\\substrmvar_a.ec3";
    //char *OutputfileName ="D:\\CASTOR2\\test_data\\eac3\\substrmvar_a_dolby.wav";
    //char *InputfileName = "D:\\CASTOR2\\test_data\\eac3\\lfe_mixlevs.ec3";
    //char *OutputfileName ="D:\\CASTOR2\\test_data\\eac3\\lfe_mixlevs_dolby.wav";
    //char *InputfileName = "D:\\CASTOR2\\test_data\\eac3\\dualmono.ec3";
    //char *OutputfileName ="D:\\CASTOR2\\test_data\\eac3\\dualmono_dolby.wav";
    char *InputfileName = "D:\\CASTOR2\\test_data\\eac3\\2ch_CPL_ON.ec3";
    char *OutputfileName ="D:\\CASTOR2\\test_data\\eac3\\2ch_CPL_ON_dolby.wav";     
#endif
    /* check input arguments */
    //ERR_CHKARG(p_argv);
    //ERR_CHKARG(p_execparams);
    //ERR_CHKARG(p_subparams);

    /* initialize executive parameters to defaults */
    //err = initexecparams(p_execparams);
    //ERR_CHKRTN(err);

    /* initialize subroutine parameters to defaults */
    //err = initsubparams(p_subparams);
    //ERR_CHKRTN(err);

    /* init local variables */
    //for (ch = 0; ch < DEC_MAXPCMCHANS; ch++)
    //{
        //pcmchan_value_set[ch] = FALSE;
    //}

    //p_execparams->p_ddpinfilename = InputfileName;
    //p_execparams->p_pcmoutfilename = OutputfileName;
    //p_execparams->outnchans = 2;
    //p_subparams->outchanconfig =2;
    //p_subparams->compmode = GBL_COMP_RF;
    //p_subparams->outlfe = LFEOUTOFF;
#if 1
    /* parse command line */
    for (count = 1; count < argc; count++)
    {
        if (*(p_argv[count]) == '-')
        {
            p_input = p_argv[count] + 1;

            switch (*(p_input))
            {
                case 'b':
                case 'B':
                    /*p_execparams->pcmwordtype = atoi(p_input + 1);
                    if (!((p_execparams->pcmwordtype == PCMINT16) ||
                          (p_execparams->pcmwordtype == PCMFLOAT64) ||
                         ((p_execparams->pcmwordtype >= PCMBITS17) &&
                          (p_execparams->pcmwordtype <= PCMBITS24))))
                    {
                        ERR_PRINTERRMSG("Invalid pcm word size (-b)");
                        return (ERR_INVALID_PCM_WORD_SIZE);
                    }*/
                    break;
                case 'c':
                case 'C':
                    atoi(p_input + 1);
                    break;
#if defined(KCAPABLE)
                case 'c':
                case 'C':
                    p_subparams->kmode = atoi(p_input + 1);
                    if ((p_subparams->kmode != GBL_NO_VOCALS) &&
                        (p_subparams->kmode != GBL_VOCAL1) &&
                        (p_subparams->kmode != GBL_VOCAL2) &&
                        (p_subparams->kmode != GBL_BOTH_VOCALS))
                    {
                        ERR_PRINTERRMSG("Invalid karaoke mode (-c)");
                        return (ERR_INVALID_KARAOKE_MODE);
                    }
                    break;
#endif /* defined(KCAPABLE) */
#if defined(DEBUG)
                case 'd':
                case 'D':
                    switch (*(p_input + 1))
                    {
                        case 'f':
                        case 'F':
                            if (!sscanf(p_input + 2, "%10lX", &p_subparams->frm_debugflags))
                            {
                                ERR_PRINTERRMSG("Invalid frame debug argument (-df)");
                                return (ERR_INVALID_DBGINFO_ARG);
                            }
                            break;
                        case 'p':
                        case 'P':
                            if (!sscanf(p_input + 2, "%10lX", &p_subparams->dec_debugflags))
                            {
                                ERR_PRINTERRMSG("Invalid decoder debug argument (-dp");
                                return (ERR_INVALID_DBGB4B_ARG);
                            }
                            break;
                        default:
                            ERR_PRINTERRMSG("Invalid debug argument (-d?)");
                            return (ERR_INVALID_DBG_ARG);
                    }
                    break;
#endif /* defined(DEBUG) */
                case 'h':
                case 'H':
                    //showusage();
                    //return (ERR_DISPLAY_USAGE_ONLY);
                    break;
                case 'i':
                case 'I':
                    p_execparams->p_ddpinfilename = p_input + 1;
                    break;
                case 'k':
                case 'K':
                    p_subparams->compmode = atoi(p_input + 1);
                    if ((p_subparams->compmode != GBL_COMP_CUSTOM_0) &&
                        (p_subparams->compmode != GBL_COMP_CUSTOM_1) &&
                        (p_subparams->compmode != GBL_COMP_LINE) &&
                        (p_subparams->compmode != GBL_COMP_RF))
                    {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                        ithPrintf("[E-AC3] Invalid dynamic range compression mode (-k)\n");
                        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                        printf("Invalid dynamic range compression mode (-k)");
#endif
                        return (ERR_INVALID_COMPR_MODE);
                    }
                    break;
                case 'l':
                case 'L':
                    p_subparams->outlfe = atoi(p_input + 1);
                    if ((p_subparams->outlfe != LFEOUTOFF) &&
                        (p_subparams->outlfe != LFEOUTSINGLE) &&
                        (p_subparams->outlfe != LFEOUTDUAL))
                    {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                        ithPrintf("[E-AC3] Invalid LFE mode (-l)\n");
                        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                        printf("Invalid LFE mode (-l)");
#endif                   
                        return (ERR_INVALID_LFE_MODE);
                    }
                    break;
                case 'm':
                case 'M':
                    switch (*(p_input + 1))
                    {
                        case 'r':
                        case 'R':
                            //p_subparams->outchanconfig = DEC71_MODE_RAW;
                            break;
                        default:
                            p_subparams->outchanconfig = (DSPshort)(atoi(p_input + 1));
                            if ((p_subparams->outchanconfig == 0) &&
                                (*(p_input + 1) != '0'))
                            {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                                ithPrintf("[E-AC3] Invalid output channel configuration (-m)\n");
                                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                                printf("Invalid output channel configuration (-m)");
#endif
                            

                                return (ERR_INVALID_OUTPUT_MODE);
                            }
                            /*if ((p_subparams->outchanconfig < GBL_MODE_RSVD) ||
                                (p_subparams->outchanconfig >= DEC71_NOUTMODES))
                            {
                                printf("Invalid output channel configuration (-m)");
                                return (ERR_INVALID_OUTPUT_MODE);
                            }*/
                        break;
                    }
                    break;
                case 'n':
                case 'N':
                    p_execparams->outnchans = atoi(p_input + 1);
                    if ((p_execparams->outnchans < 1) ||
                        (p_execparams->outnchans > 2))
                    {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                        ithPrintf("[E-AC3] Invalid number of output channels (-n)\n");
                        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                        printf("Invalid number of output channels (-n)");
#endif
                    

                        return (ERR_INVALID_NOUTPUT_CHANS);
                    }
                    break;
                case 'o':
                case 'O':
                    switch (*(p_input + 1))
                    {
                        case 'c':
                        case 'C':
                            p_execparams->outmapon = TRUE;
                            p_execparams->p_chanmapoutfilename = p_input + 2;
                            break;
                        case 'p':
                        case 'P':
                            p_execparams->p_pcmoutfilename = p_input + 2;
                            break;
                        case 'd':
                        case 'D':
                            break; /* Ignore legacy DCV output DD file name */
                        default:
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                            ithPrintf("[E-AC3] Invalid PCM scale factor (-p) \n");
                            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                            printf("Invalid PCM scale factor (-p)");
#endif
                            

                            return (ERR_INVALID_OUTFILE_ARG);
                    }
                    break;
                case 'p':
                case 'P':
                    p_subparams->pcmscale = (DSPshort)((atof(p_input + 1) + 0.005) * 100);
                    if ((p_subparams->pcmscale < MINSCALEFACTOR) ||
                        (p_subparams->pcmscale > MAXSCALEFACTOR))
                    {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                        ithPrintf("[E-AC3] Invalid PCM scale factor (-p) \n");
                        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                        printf("Invalid PCM scale factor (-p)");
#endif
                        return (ERR_INVALID_PCM_SCL_FACTOR);
                    }
                    break;
                case 'q':
                case 'Q':
                    p_execparams->quitonerr = FALSE;
                    break;
                case 'r':
                case 'R':
                    range(p_input + 1, &p_execparams->framestart, &p_execparams->frameend);
                    if (p_execparams->framestart < 0)
                    {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                        ithPrintf("[E-AC3] Invalid frame region end (-r#:#) \n");
                        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                        printf("Invalid frame region start (-r#)");
#endif                    
                        return (ERR_INVALID_FRAME_RGN_STRT);
                    }
                    if ((p_execparams->frameend >= 0) &&
                        (p_execparams->frameend <= p_execparams->framestart))
                    {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                        ithPrintf("[E-AC3] Invalid frame region end (-r#:#) \n");
                        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                        printf("Invalid frame region end (-r#:#)");
#endif
                        return (ERR_INVALID_FRAME_RGN_END);
                    }
                    break;
                case 's':
                case 'S':
                    p_subparams->stereomode = (DSPshort)(atoi(p_input + 1));
                    if ((p_subparams->stereomode != GBL_STEREOMODE_AUTO) &&
                        (p_subparams->stereomode != GBL_STEREOMODE_SRND) &&
                        (p_subparams->stereomode != GBL_STEREOMODE_STEREO))
                    {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                        ithPrintf("[E-AC3] Invalid stereo output mode (-s) \n");
                        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                        printf("Invalid stereo output mode (-s)");
#endif
                        return (ERR_INVALID_STEREO_MODE);
                    }
                    break;
                case 'u':
                case 'U':
                    p_subparams->dualmode = (DSPshort)(atoi(p_input + 1));
                    if ((p_subparams->dualmode != GBL_DUAL_STEREO) &&
                        (p_subparams->dualmode != GBL_DUAL_LEFTMONO) &&
                        (p_subparams->dualmode != GBL_DUAL_RGHTMONO) &&
                        (p_subparams->dualmode != GBL_DUAL_MIXMONO))
                    {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                        ithPrintf("[E-AC3] Invalid dual mono reproduction mode (-u) \n");
                        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                        printf("Invalid dual mono reproduction mode (-u)");
#endif                    
                        return (ERR_INVALID_DUAL_MONO_MODE);
                    }
                    break;
                case 'v':
                case 'V':
                    p_execparams->verbose = atoi(p_input + 1);
                    if ((p_execparams->verbose != VERBOSELEVEL0) &&
                        (p_execparams->verbose != VERBOSELEVEL1) &&
                        (p_execparams->verbose != VERBOSELEVEL2))
                    {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                        ithPrintf("[E-AC3] Invalid verbose mode (-v) \n");
                        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                        printf("Invalid verbose mode (-v)");
#endif
                        return (ERR_INVALID_VERBOSE_MODE);
                    }
                    break;
#if defined(SRC)
                case 'w':
                case 'W':
                    p_execparams->upsample = atoi(p_input + 1);
                    if ((p_execparams->upsample != SRCUSEMETADATA) &&
                        (p_execparams->upsample != SRCALWAYSUPSAMPLE) &&
                        (p_execparams->upsample != SRCNEVERUPSAMPLE))
                    {
                        ERR_PRINTERRMSG("Invalid upsample mode (-w)");
                        return (ERR_INVALID_UPSAMPLE_MODE);
                    }
                    break;
#endif /* defined(SRC) */
                case 'x':
                case 'X':
                    p_subparams->dynscalehigh = (DSPshort)((atof(p_input + 1) + 0.005) * 100);
                    if ((p_subparams->dynscalehigh < MINSCALEFACTOR) ||
                        (p_subparams->dynscalehigh > MAXSCALEFACTOR))
                    {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                        ithPrintf("[E-AC3] Invalid output file suppression argument (-zp) \n");
                        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                        printf("Invalid dynamic range compression cut (-x)");
#endif
                        return (ERR_INVALID_DRC_CUT);
                    }
                    break;
                case 'y':
                case 'Y':
                    p_subparams->dynscalelow = (DSPshort)((atof(p_input + 1) + 0.005) * 100);
                    if ((p_subparams->dynscalelow < MINSCALEFACTOR) ||
                        (p_subparams->dynscalelow > MAXSCALEFACTOR))
                    {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                        ithPrintf("[E-AC3] Invalid output file suppression argument (-zp) \n");
                        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                        printf("Invalid dynamic range compression boost (-y)");
#endif                                                                                
                        return (ERR_INVALID_DRC_BOOST);
                    }
                    break;
                case 'z':
                case 'Z':
                    switch (*(p_input + 1))
                    {
                        case 'p':
                        case 'P':
                            p_execparams->suppresspcmfileout = FILESUPPRESSED;
                            break;
                        case 'd':
                        case 'D':
                            break; /* Ignore legacy DCV DD file suppress */
                        default:
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                            ithPrintf("[E-AC3] Invalid output file suppression argument (-zp) \n");
                            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                            printf("Invalid output file suppression argument (-zp)");
#endif                                                             
                            return (ERR_INVALID_FILESUPPRESS_ARG);
                            break;
                    }
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    chanval = *p_input - '0';
                    p_route = p_input + 1;
                    switch (*p_route)
                    {
                        case 'L':
                            routing_index = 0;
                            break;
                        case 'C':
                            routing_index = 1;
                            break;
                        case 'R':
                            routing_index = 2;
                            break;
                        case 'l':
                            routing_index = 3;
                            break;
                        case 'r':
                            routing_index = 4;
                            break;
                        case 's':
                            routing_index = 5;
                            break;
                        case 'x':
                            if (*(p_route+1) == '1')
                            {
                                routing_index = 6;
                            }
                            else if (*(p_route+1) == '2')
                            {
                                routing_index = 7;
                            }
                            else
                            {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                                ithPrintf("[E-AC3] Invalid command line argument (?) \n");
                                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                                printf("Invalid command line argument (?)");
#endif                                  
                                return (ERR_INVALID_EXTCHAN_ROUTE_ARG);
                            }
                            break;
                        default:
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                            ithPrintf("[E-AC3] Invalid command line argument (-#?) \n");
                            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                            printf("Invalid channel routing argument (-#?)");
#endif
                            return (ERR_INVALID_EXTCHAN_ROUTE_ARG);
                    }

                    /* Use routing_index to set routing arrays */
                    p_execparams->chanrouting[routing_index] = chanval;
                    pcmchan_value_set[chanval] = TRUE;

                    break;
                default:
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                    ithPrintf("[E-AC3] Invalid command line argument (?) \n");
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                    printf("Invalid command line argument (?)");
#endif                           
                    return (ERR_INVALID_COMMAND_LINE);
            }
        }
        else
        {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
            ithPrintf("[E-AC3] Invalid command line argument (?) \n");
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
            printf("Invalid command line argument (?)");
#endif       
            return (ERR_INVALID_COMMAND_LINE);
        }
    }
#endif
    /*if ((p_execparams->p_ddpinfilename == P_NULL))
    {
        ERR_PRINTERRMSG("Input filename must be specified (-i)");
        return (ERR_UNKNOWN_INPUTFILE);
    }
    if ((p_execparams->suppresspcmfileout == FILENOTSUPPRESSED) &&
        (p_execparams->p_pcmoutfilename == P_NULL))
    {
        ERR_PRINTERRMSG("Output PCM filename must be specified (-op), or file must be suppressed (-zp)");
        return (ERR_UNKNOWN_OUTPCMFILE);
    }

    if ((p_execparams->outmapon == TRUE) &&
        (p_execparams->p_chanmapoutfilename == P_NULL))
    {
        ERR_PRINTERRMSG("Output filename must be specified when output channel map metadata is selected (-oc)");
        return (ERR_UNKNOWN_OUTPCMFILE);
    }*/

    /* Check that all channels of the channel routing array are properly filled */
    /*err = validate_routing_array(
        p_execparams->chanrouting,
        pcmchan_value_set);
    if (err)
    {
        ERR_PRINTERRMSG("Problem encountered during channel routing array validation");
        return (err);
    }*/
    return (ERR_NO_ERROR);
}
#endif // TVA_TEST

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
    int  nNewTicks,nTotalTicks;
    int* codecStream;
    AUDIO_CODEC_STREAM* audioStream;
    
#ifdef TVA_TEST
    EXECPARAMS               execparams;                  /* user-specified executive parameters     */
    SUBPARAMS                subparams;                   /* user-specified subroutine parameters    */
//#define WRITE_PARAMETER
    #ifdef WRITE_PARAMETER
    char inFileName[256] = "D:\\TVA_Dolby\\Phase1_TestVectorAnalysis\\dith0.ac3";
    char outFileName[256]= "D:\\TVA_Dolby\\Phase1_TestVectorAnalysis\\dith0.pcm";
    #endif
#endif
#if defined(WIN32) || defined(__CYGWIN__)
    //GetParam(argc, argv);
    //win32_init();
    parsecmdline(argc,argv,&execparams,&subparams);

#endif // defined(WIN32) || defined(__CYGWIN__)
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
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

#if defined(ENABLE_PERFORMANCE_MEASURE)
    {
        int i;
        for(i=0; i<sizeof(time_log)/sizeof(int); i++) {
            time_log[i]=0;
        }
    }
#endif
    gnFadeIn = 0;
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
        err = 0;
        flags = 0;
        ac3FrameAccu = 0;
        gnAudioBlock = 0;
        gnPCMBytes = 0;
#if defined(__INPUT_CRC_CHECK__) || defined(__OUTPUT_CRC_CHECK__)
        crc32_init();
#endif // defined(__INPUT_CRC_CHECK__) || defined(__OUTPUT_CRC_CHECK__)

        MMIO_Write(DrvDecode_Frame  , 0);
        MMIO_Write(DrvDecode_Frame+2, 0);


        memset(&ac3Context,0,sizeof(ac3Context));

        /* initializer AC3 Decoder */
        ac3_decode_init(&ac3Context);

        //device related setting
        ac3Context.dev_channels = 2;    // n 2
        ac3Context.dev_mode = 2;    // m 2
        ac3Context.dev_lfeon = 0;
        ac3Context.dev_compmode = AC3_COMP_LINE;
        ac3Context.dev_stereomode = AC3_STEREOMODE_AUTO;
        ac3Context.dev_dualmode = AC3_DUAL_STEREO;
        ac3Context.dev_gain = LEVEL(1.0);
        ac3Context.dev_dynscalelow = 100;
        ac3Context.dev_dynscalehigh = 100;
        memset(&tempWriteBuf[0], 0, 6144);
        memset(&pcmWriteBuf[pcmWriteIdx], 0, sizeof(pcmWriteBuf));

        //ac3Context.outmode = 2;
#ifdef TVA_TEST        
        if (subparams.compmode == GBL_COMP_RF)
        {
            ac3Context.dev_compmode = AC3_COMP_RF;
            printf("set parameter compmode = RF mode \n");
        }
        else if (subparams.compmode == GBL_COMP_LINE)
        {
            ac3Context.dev_compmode = AC3_COMP_LINE;
            printf("set parameter compmode = Line mode \n");
        }
        else if (subparams.compmode == GBL_COMP_CUSTOM_0)
        {
            ac3Context.dev_compmode = AC3_COMP_CUSTOM_0;
            printf("set parameter compmode = Custom mode 0\n");
        }
        else if (subparams.compmode == GBL_COMP_CUSTOM_1)
        {
            ac3Context.dev_compmode = AC3_COMP_CUSTOM_1;
            printf("set parameter compmode = Custom mode 1\n");
        }

        if (subparams.stereomode == GBL_STEREOMODE_AUTO)
        {
    	     ac3Context.dev_stereomode = AC3_STEREOMODE_AUTO;
            printf("set parameter stereomode = AutoStereo mode \n");
        }
        else if (subparams.stereomode == GBL_STEREOMODE_SRND)
        {
    	     ac3Context.dev_stereomode = AC3_STEREOMODE_SRND;
            printf("set parameter stereomode = Surround mode \n");
        }
        else if (subparams.stereomode == GBL_STEREOMODE_STEREO)
        {
    	     ac3Context.dev_stereomode = AC3_STEREOMODE_STEREO;
            printf("set parameter stereomode = Stereo mode \n");
        }

        if (subparams.dualmode == GBL_DUAL_LEFTMONO)
        {
    	     ac3Context.dev_dualmode = AC3_DUAL_LEFTMONO;
        }
        else if (subparams.dualmode == GBL_DUAL_RGHTMONO)
        {
    	     ac3Context.dev_dualmode = AC3_DUAL_RGHTMONO;
        }
        else if (subparams.dualmode == GBL_DUAL_MIXMONO)
        {
    	     ac3Context.dev_dualmode = AC3_DUAL_MIXMONO;
        }
        if (subparams.dynscalelow>=0 && subparams.dynscalelow<=100)
        {
            ac3Context.dev_dynscalelow = subparams.dynscalelow;
        }
        if (subparams.dynscalehigh>=0 && subparams.dynscalehigh<=100)
        {
            ac3Context.dev_dynscalehigh = subparams.dynscalehigh;
        }
        printf("set parameter dynscle %d %d \n",ac3Context.dev_dynscalehigh,ac3Context.dev_dynscalelow);

        if (subparams.outchanconfig < 8 && subparams.outchanconfig>=0)            
        {
            ac3Context.dev_mode = subparams.outchanconfig;
            printf("set parameter outchanconfig %d \n",subparams.outchanconfig);
        }
        if (execparams.outnchans < 3 && execparams.outnchans>0)            
        {
            ac3Context.dev_channels = execparams.outnchans;
            printf("set parameter outnchans %d \n",execparams.outnchans);
        }
    #ifdef WRITE_PARAMETER
        openInputOutputStraem(inFileName,outFileName,ac3Context.dev_channels);
    #else
        openInputOutputStraem(execparams.p_ddpinfilename,execparams.p_pcmoutfilename,ac3Context.dev_channels);   
    #endif    
#endif // TVA_TEST    

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
        ithPrintf("[E-AC3] start 0x%x 0x%x 0x%x \n",&streamBuf[ac3ReadIdx],streamBuf[ac3ReadIdx],streamBuf[ac3ReadIdx+1]);
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
        printf("[E-AC3] start \n");        
#endif

       do { // AC3 Decode Loop
#if defined(ENABLE_PERFORMANCE_MEASURE)
            start_timer();
#endif // defined(ENABLE_PERFORMANCE_MEASURE)

            // Update the write buffer and wait the write buffer (PCM data) avaliable.
            // If the I2S interface dose not initialize yet, the read/write pointer is on the wrong
            // position.
            if (bInitDAC) {
                gnAc3DropTime = MMIO_Read(AUDIO_DECODER_DROP_DATA);
                if (gnAc3DropTime>0) {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                    ithPrintf("[E-AC3]drop data \n");
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                    printf("[EAC3]drop data \n");
#endif
                    updateTime();                    
                } else {
                    FillWriteBuffer(nPCMBytes);
                }           
            }
#if defined(EAC3_ACCEPT_PARAMETER)
            // DRC, 2 channels downmix , dual mono
            EAC3_UpdateParameter();
#endif
            // Updates the read buffer and returns the avaliable size of input buffer (AC3 stream).
            bytesLeft = FillReadBuffer(nReadBytes);

            if (isSTOP() || lastRound) break;

            PRINTF("Frames : %d\n", nFrames);

            /* Decode one AC3 Frame */
            {
                unsigned char *inbuf, *outbuf,*temp;
                prebyteLeft = bytesLeft;
                inbuf = &streamBuf[ac3ReadIdx];
                outbuf = &pcmWriteBuf[pcmWriteIdx];
                temp = &tempWriteBuf[0];

#ifdef EAC3_PERFORMANCE_TEST_BY_TICK
                start_timer();
#endif                          
#ifdef EAC3_MEMORY_COPY_TO_I2S
                if (pcmWriteIdx+MAXFRAME <= sizeof(pcmWriteBuf)) {
                    err = ac3_decode_frame(&ac3Context, inbuf, &bytesLeft, outbuf, &nPCMBytes);
                } else {
                    err = ac3_decode_frame(&ac3Context, inbuf, &bytesLeft, temp, &nPCMBytes);
                    if (err) {

                    } else {
                        if (pcmWriteIdx+nPCMBytes > sizeof(pcmWriteBuf)) {
                            memcpy(&pcmWriteBuf[pcmWriteIdx], &tempWriteBuf[0], sizeof(pcmWriteBuf)-pcmWriteIdx);
                            memcpy(&pcmWriteBuf[0], &tempWriteBuf[sizeof(pcmWriteBuf)-pcmWriteIdx], nPCMBytes-(sizeof(pcmWriteBuf)-pcmWriteIdx));
                        } else{
                            memcpy(&pcmWriteBuf[pcmWriteIdx], &tempWriteBuf[0], nPCMBytes);
                        }
                    }
                }
#else
                err = ac3_decode_frame(&ac3Context, inbuf, &bytesLeft, outbuf, &nPCMBytes);
#endif
                nReadBytes = prebyteLeft - bytesLeft;
            }
             /*{
                    int i;
                    for (i=0;i<sizeof(pcmWriteBuf);i++)
                        if (pcmWriteBuf[i])
                        {
                            printf("[EAC3] buf %d nFrame %d\n",pcmWriteBuf[i],nFrames);
                            asm volatile("l.trap 15");
                        }

                }
                */
            if (0) //(getAC3PrintMetaDataMode())
            {
                printf("Frame    %d :\n\n",nFrames);
                printf("dmixmod %d \n",ac3Context.dmixmod);
                printf("cmixlev %d \n",ac3Context.legacy_cmixlev);
                printf("surmixlev %d \n",ac3Context.legacy_surmixlev);
                printf("lorocmixlev %d \n",ac3Context.loro_cmixlev);
                printf("lorosurmixlev %d \n",ac3Context.loro_surmixlev);
                printf("ltrt_cmixlev %d \n",ac3Context.ltrt_cmixlev);
                printf("ltrt_surmixlev %d \n",ac3Context.ltrt_surmixlev);
                printf("pgmscl[0] %d \n",ac3Context.pgmscl[0]);
                printf("pgmscl[1] %d \n",ac3Context.pgmscl[1]);
                printf("paninfo[0] %d \n",ac3Context.paninfo[0]);
                printf("paninfo[1] %d \n",ac3Context.paninfo[1]);
                printf("panmean[0] %d \n",ac3Context.panmean[0]);
                printf("panmean[1] %d \n",ac3Context.panmean[1]);
                printf("extpgmscl %d \n",ac3Context.extpgmscl);
                printf("premixcmpsel %d \n",ac3Context.premixcmpsel);
                printf("drcsrc %d \n",ac3Context.drcsrc);
                printf("premixcmpscl %d \n",ac3Context.premixcmpscl);
                printf("lfemixlevcod %d \n",ac3Context.lfemixlevcod);
                printf("extpgmlscl %d \n",ac3Context.extpgmlscl);
                printf("extpgmcscl %d \n",ac3Context.extpgmcscl);
                printf("extpgmrscl %d \n",ac3Context.extpgmrscl);
                printf("extpgmlsscl %d \n",ac3Context.extpgmlsscl);
                printf("extpgmrsscl %d \n",ac3Context.extpgmrsscl);
                printf("extpgmlfescl %d \n",ac3Context.extpgmlfescl);
                printf("dmixscl %d \n",ac3Context.dmixscl);
                printf("extpgmaux1scl %d \n",ac3Context.extpgmaux1scl);
                printf("extpgmaux2scl %d \n",ac3Context.extpgmaux2scl);
                printf("\n\n\n\n");
            }
                
#ifdef AC3_RESET_DECODED_BYTE    
            if (isResetAudioDecodedBytes()) {
                MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvResetAudioDecodedBytes);
                nAc3DecodedByte = 0;
                MMIO_Write(DrvAudioDecodedBytes, nAc3DecodedByte);    

                PalSleep(5);
            }                        
            // write ac3 decoded byte to register
            if (nReadBytes) {
                nAc3DecodedByte += nReadBytes;
                MMIO_Write(DrvAudioDecodedBytes, nAc3DecodedByte); 
            } else {
                //printf("[AC3] nReadBytes %d <=  0 %d\n",StreamBuf.rdptr,nKeepByte);
            }
#endif  // def AC3_RESET_DECODED_BYTE       

            //PRINTF("#%04d: error %d %d\n", nFrames, err, nReadBytes);
            if (err)
            {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                ithPrintf("[E-AC3] dec error %d \n",err);
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                printf("[EAC3] dec error %d \n",err);
#endif                      
                nPCMBytes  = 0;
                nReadBytes = 1;
                
                if (eofReached)
                    lastRound = 1;
                continue; // Skip error
            }
#ifdef EAC3_PERFORMANCE_TEST_BY_TICK
            nNewTicks = get_timer();            
            nTotalTicks += nNewTicks;
            if (nFrames % 50 == 0 && nFrames>0) {
                ithPrintf("[EAC3] (%d~%d) total %d (ms) average %d (ms) nFrames %d system clock %d\n",(nFrames+1-50),(nFrames+1),(nTotalTicks/(PalGetSysClock()/1000)),((nTotalTicks/(PalGetSysClock()/1000))/50),nFrames+1,PalGetSysClock());
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
                nTotalTicks=0;                    
                nFrames++;
            }
#endif //EAC3_PERFORMANCE_TEST_BY_TICK

            if (eac3NChans != ac3Context.dev_channels && ac3Context.dev_channels>0 && ac3Context.dev_channels <=2)            
            {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                ithPrintf("[E-AC3] channel different %d %d \n",eac3NChans,ac3Context.dev_channels);
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                printf("[EAC3] channel different %d %d \n",eac3NChans,ac3Context.dev_channels);
#endif           
                eac3NChans = ac3Context.dev_channels;
                bInitDAC=0;
            }
            if (eac3SampRateOut !=  ac3Context.sample_rate  &&  ac3Context.sample_rate>=32000 &&  ac3Context.sample_rate<=48000)
            {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                ithPrintf("[E-AC3] sample rate different %d %d \n",eac3SampRateOut,ac3Context.sample_rate);
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                printf("[EAC3] sample rate different %d %d \n",eac3SampRateOut,ac3Context.sample_rate);
#endif
                eac3SampRateOut =  ac3Context.sample_rate;
                bInitDAC=0;
            }

            /* Initialize DAC */
            if (!bInitDAC) 
            {
                // ac3FrameStep is 1.31 format.
                unsigned long long n = ((unsigned long long)OUTPUT_SAMPLES) << 31;
                // 6 is full audio block
                ac3FrameStep = (unsigned int)(n / ac3Context.sample_rate)/6;

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(AC3_DUMP_PCM)
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_FILE_OPEN);
#endif // defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(AC3_DUMP_PCM)

                bInitDAC = 1;
                eac3NChans = ac3Context.dev_channels;
                eac3SampRateOut = ac3Context.sample_rate;
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)            
                gCh = ac3Context.dev_channels;
                gSampleRate = ac3Context.sample_rate;
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC);
#else
                initDAC(pcmWriteBuf, ac3Context.dev_channels, ac3Context.sample_rate, sizeof(pcmWriteBuf), 0);
#endif                
                
            }

#if defined(LITTLE_ENDIAN_PCM) && defined(DUMP_PCM_DATA)
            // Byte swap to little endian
            {
                int i;
                char tmp;
                char *buf = (char *)&pcmWriteBuf[pcmWriteIdx];
                for(i=0; i<OUTPUT_BYTES; i+=2) {
                    tmp      = buf[i];
                    buf[i  ] = buf[i+1];
                    buf[i+1] = tmp;
                }
            }
#endif
            gnAudioBlock+=ac3Context.num_blocks;
            if (gnAudioBlock>=6)
            {
                gnAudioBlock-=6;
                nFrames++;
            }                            
            //nFrames++;

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
            //ithPrintf("[E-AC3] nFrames %d %d 0x%x err %d 0x%x\n",nFrames,ac3ReadIdx,&streamBuf[ac3ReadIdx],err,audioStream->codecStreamBuf);
            //AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
            printf("[E-AC3] nFrames %d \n",nFrames);        
#endif

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
            //PalSleep(1);
        } while (1);

        ac3_decode_end(&ac3Context);
        ClearRdBuffer();

#ifdef AC3_RESET_DECODED_BYTE    
        if (isResetAudioDecodedBytes())
        {
            MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvResetAudioDecodedBytes);
        }
        nAc3DecodedByte = 0;
        MMIO_Write(DrvAudioDecodedBytes, nAc3DecodedByte);                            
#endif  // def AC3_RESET_DECODED_BYTE        

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(AC3_DUMP_PCM)
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_FILE_CLOSE);
#endif //defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(AC3_DUMP_PCM)

        // Only do once on WIN32 platform.
        #if defined(WIN32) || defined(__CYGWIN__)
        break;
        #endif // defined(WIN32) || defined(__CYGWIN__)
    } /* end of forever loop */

#if !defined(__FREERTOS__)
    return 0;
#endif
}

