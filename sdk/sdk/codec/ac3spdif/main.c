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

#if defined(__OR32__)
#include "mmio.h"
#include "sys.h"
#endif

#if defined(__INPUT_CRC_CHECK__)
#include "crc32.h"
#endif

#include "ticktimer.h"

#if defined(ENABLE_CODECS_PLUGIN)
#  include "plugin.h"
#endif

///////////////////////////////////////////////////////////////////////////
//                              Constant Definition
///////////////////////////////////////////////////////////////////////////

#define A52_CHANNEL           0
#define A52_MONO              1
#define A52_STEREO            2
#define A52_3F                3
#define A52_2F1R              4
#define A52_3F1R              5
#define A52_2F2R              6
#define A52_3F2R              7
#define A52_CHANNEL1          8
#define A52_CHANNEL2          9
#define A52_DOLBY             10
#define A52_CHANNEL_MASK      15

#define A52_LFE               16
#define A52_ADJUST_LEVEL      32
#define OUTPUT_SAMPLES  (256*6)

enum {
    ERR_AC3_OK                  = 0,
    ERR_AC3_INDATA_UNDERFLOW    = -1,
    ERR_AC3_NULL_POINTER        = -2,

    ERR_AC3_INVALID_FRAME       = -3,
    ERR_AC3_INVALID_BSID        = -4,
    ERR_AC3_INVALID_FRAMCODE    = -5,
    ERR_AC3_INVALID_SAMPLERATE  = -6,
    ERR_AC3_INVALID_EXPONENT    = -7,

    ERR_AC3_DOWNMIX_INIT        = -8,
    ERR_AC3_PARSE_DELTBA        = -9,
    ERR_AC3_BLOCK_DECODE        = -10,

    ERR_AC3_UNKNOWN             = -9999
};

typedef struct SPDIF_BURST_HEADER_TAG
{
    unsigned short Pa; //Sync word 1
    unsigned short Pb; //Sync word 2
    unsigned short Pc; // Burst-info
    unsigned short Pd; // Length-code
} SPDIF_BURST_HEADER;

///////////////////////////////////////////////////////////////////////////
//                              Globale Variable
///////////////////////////////////////////////////////////////////////////

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
static unsigned int  gSampleRate;
static unsigned int  gTailLength;

static unsigned int gnOutput;
static unsigned int gnChannels;
static const unsigned char gnChans_tbl[] = { 2, 1, 2, 3, 3, 4, 4, 5, 1, 1, 2 };

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

static uint8_t halfrate[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3 };

///////////////////////////////////////////////////////////////////////////
//                              Function Decleration
///////////////////////////////////////////////////////////////////////////
/* Function decleration */
static int  AdjustBuffer(int waitNBytes);
static int  FillReadBuffer(int nReadBytes);
static void FillWriteBuffer(int nWriteDataBytes);
static void ClearRdBuffer(void);
static int  ac3GetSyncInfo(uint8_t * buf, int *flags, int *sample_rate, int *bit_rate, int* frm_size_code);
static int GetDuration();
static int GetStuffing(unsigned long frame_length);


static int GetStuffing(unsigned long frame_length)
{
    return MAXFRAME-frame_length-8;
}


static int ac3GetSyncInfo(uint8_t * buf, int *flags, int *sample_rate, int *bit_rate, int* frm_size_code)
{
    static int rate[] = { 32, 40, 48, 56, 64, 80, 96, 112,
        128, 160, 192, 224, 256, 320, 384, 448,
        512, 576, 640
    };
    static uint8_t lfeon[8] = { 0x10, 0x10, 0x04, 0x04, 0x04, 0x01, 0x04, 0x01 };
    int frmsizecod;
    int bitrate;
    int half;
    int acmod;

    if ((buf[0] != 0x0b) || (buf[1] != 0x77))   /* syncword */
        return ERR_AC3_INVALID_FRAME;

    if (buf[5] >= 0x60)         /* bsid >= 12 */
        return ERR_AC3_INVALID_BSID;
    half = halfrate[buf[5] >> 3];

    /* acmod, dsurmod and lfeon */
    acmod = buf[6] >> 5;
    *flags = ((((buf[6] & 0xf8) == 0x50) ? A52_DOLBY : acmod) | ((buf[6] & lfeon[acmod]) ? A52_LFE : 0));

    frmsizecod = buf[4] & 63;
    if (frmsizecod >= 38)
        return ERR_AC3_INVALID_FRAMCODE;
    bitrate = rate[frmsizecod >> 1];
    *bit_rate = (bitrate * 1000) >> half;

    gnChannels = gnChans_tbl[acmod];
    if (*flags & A52_LFE)
    {
        gnChannels+=1;
    }

    switch (buf[4] & 0xc0) {
    case 0:
        *sample_rate = 48000 >> half;
        return 4 * bitrate;
    case 0x40:
        *sample_rate = 44100 >> half;
        *frm_size_code = frmsizecod;
        return 2 * (320 * bitrate / 147 + (frmsizecod & 1));
    case 0x80:
        *sample_rate = 32000 >> half;
        return 6 * bitrate;
    default:
        return ERR_AC3_INVALID_SAMPLERATE;
    }
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
                //taskYIELD();
                PalSleep(1);
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
        if (ac3ReadIdx > READBUF_SIZE) {
            ac3ReadIdx = ac3ReadIdx - READBUF_SIZE + READBUF_BEGIN;
            PRINTF("AC3: ac3ReadIdx > READBUF_SIZE");
            asm volatile("l.trap 15");
        }

        if (ac3ReadIdx >= READBUF_BEGIN)
        {
            MMIO_Write(DrvDecode_RdPtr, (unsigned short)(((ac3ReadIdx-READBUF_BEGIN)>>1) << 1));
        }
        else
        {
            MMIO_Write(DrvDecode_RdPtr, 0);
        }
    }

    waitNBytes = 7; // wait 7 bytes for frame header.
    done       = 0;
    readState  = WAIT_HEADER;

    // Wait Read Buffer avaliable
    do
    {
        checkControl();
        len = AdjustBuffer(waitNBytes);

        if ((!eofReached) && (len < waitNBytes))
        {
#if defined(__FREERTOS__)
            taskYIELD();
#else
            or32_delay(1); // enter sleep mode for power saving
#endif
        }
        else
        {
            int flags, sample_rate, bit_rate, length, frm_size_code;
            if (eofReached && len <= 0) lastRound = 1;
            if (done || (eofReached && len <= 0)) break;
            switch(readState)
            {
                case WAIT_HEADER :
                   length = ac3GetSyncInfo(&streamBuf[ac3ReadIdx],
                            &flags, &sample_rate, &bit_rate, &frm_size_code);
                   if (length < 0) 
                   {
                        // Find next sync info
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
                   }
                   else
                   {
                        gSampleRate = sample_rate;
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
static void FillWriteBuffer(int nWriteDataBytes) {
    int len;

    #if defined(__OUTPUT_CRC_CHECK__)
    {
        static int crc = 0;
        crc = crc32(&pcmWriteBuf[pcmWriteIdx], nWriteDataBytes);
        PRINTF("CRC: 0x%08x\n", crc);
    }
    #endif // defined(__OUTPUT_CRC_CHECK__)

    // Update Write Buffer
    if (nWriteDataBytes > 0) {
        pcmWriteIdx = pcmWriteIdx + nWriteDataBytes;

        #if defined(OUTPUT_MEMMODE)
        if (pcmWriteIdx >= sizeof(pcmWriteBuf))
            pcmWriteIdx = sizeof(pcmWriteBuf);

        SetOutWrPtr(pcmWriteIdx);
        return;
        #else
        if (pcmWriteIdx >= sizeof(pcmWriteBuf))
            pcmWriteIdx -= sizeof(pcmWriteBuf);

        //printf("(%d), pcmWriteIndex: %u\n", __LINE__, pcmWriteIdx);
        SetOutWrPtr(pcmWriteIdx);
        #endif // defined(OUTPUT_MEMMODE)
    }

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

    //PRINTF("pcmWriteIdx(%d) pcmReadIdx(%d) len(%d) nWriteDataBytes(%d)\n", pcmWriteIdx, pcmReadIdx, len, nWriteDataBytes);
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
void AC3SPDIF_GetBufInfo(unsigned* inbuf, unsigned* inlen)
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
    int nWriteDataBytes;
    int  nNewTicks,nTotalTicks;
    int nTemp;

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

    gnOutput = 0;
    nTotalTicks = 0;
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
        nWriteDataBytes   = 0;
        lastRound   = 0;
        err         = 0;
        flags       = 0;
        SPDIF_BURST_HEADER ac3Header = { 0xF872, 0x4E1F, 0x1, 0x0 };
        SPDIF_BURST_HEADER nullHeader = { 0xF872, 0x4E1F, 0xE000, 0x0 };

        #if defined(__INPUT_CRC_CHECK__) || defined(__OUTPUT_CRC_CHECK__)
        crc32_init();
        #endif // defined(__INPUT_CRC_CHECK__) || defined(__OUTPUT_CRC_CHECK__)

        MMIO_Write(DrvDecode_Frame  , 0);
        MMIO_Write(DrvDecode_Frame+2, 0);

        do { // AC3 Decode Loop
            #if defined(ENABLE_PERFORMANCE_MEASURE)
            start_timer();
            #endif // defined(ENABLE_PERFORMANCE_MEASURE)

            // Update the write buffer and wait the write buffer (PCM data) avaliable.
            // If the I2S interface dose not initialize yet, the read/write pointer is on the wrong
            // position.
            if (bInitDAC) {
                FillWriteBuffer(nWriteDataBytes);
            }
            nNewTicks = get_timer();            
            nTotalTicks += nNewTicks;
            gnOutput+=nWriteDataBytes;
            if ( (nTotalTicks/(PalGetSysClock()/1000))>1000)
            {
                //printf("[AC3 SPDIF] 1 s(%d) output %d \n",(nTotalTicks/(PalGetSysClock()/1000)),gnOutput);
                gnOutput = 0;
                nTotalTicks = 0;
            }
            start_timer();
            if (isSPDIFMute())
            {
                //printf("[AC3 SPDIF] mute \n");
            }
   
            // Updates the read buffer and returns the avaliable size of input buffer (AC3 stream).
            bytesLeft = FillReadBuffer(nReadBytes);

            if (isSTOP() || lastRound) break;

            /* Decode one AC3 Frame */
            {
                unsigned char *inbuf, *outbuf;
                unsigned long frame_length = 0;
                unsigned long out_buf_pos = 0;
                unsigned long remain_size = 0;
                int flag, sample_rate, bit_rate, frm_size_code;
                unsigned char tmp_header_buf[sizeof(SPDIF_BURST_HEADER)] = { 0 };
                inbuf = &streamBuf[ac3ReadIdx];
                outbuf = &pcmWriteBuf[pcmWriteIdx];
                //printf("%s(%d), byte_left: %u bytes\n", __FILE__, __LINE__, bytesLeft);
                if (bytesLeft)
                {
                    frame_length = ac3GetSyncInfo(&streamBuf[ac3ReadIdx], &flag, &sample_rate, &bit_rate, &frm_size_code);
                    nWriteDataBytes = 0;
                    //printf("%s(%d), frame_length: %u bytes\n", __FILE__, __LINE__, frame_length);
                    if (frame_length)
                    {
                        nTemp = GetStuffing(frame_length);
                        gTailLength = 0;
                        if (nTemp>0)
                        {
                            gTailLength = nTemp;
                        }

                        /*
                        switch(sample_rate)
                        {
                            case 48000:
                            case 24000:
                            case 12000:
                                gTailLength = 4 * frame_length  - (frame_length + sizeof(SPDIF_BURST_HEADER));
                                break;
                            case 44100:
                            case 22050:
                            case 11025:
                                gTailLength = 2 * (320 * frame_length / 147 + (frm_size_code & 1)) - (frame_length + sizeof(SPDIF_BURST_HEADER));
                                break;
                            case 32000:
                            case 16000:
                            case 8000:
                                gTailLength = 6 * frame_length  - (frame_length + sizeof(SPDIF_BURST_HEADER));
                                break;
                        }*/
#if 1
                        remain_size = sizeof(pcmWriteBuf) - pcmWriteIdx;
                        // Write AC3 SPDIF header
                        if (remain_size < sizeof(SPDIF_BURST_HEADER))
                        {
                            memcpy(tmp_header_buf, &ac3Header, sizeof(SPDIF_BURST_HEADER));
                            tmp_header_buf[out_buf_pos + 6] = (unsigned char) (((frame_length << 3) & 0xFF00) >> 8);
                            tmp_header_buf[out_buf_pos + 7] = (unsigned char) (((frame_length << 3) & 0x00FF));

                            memcpy(&outbuf[out_buf_pos], tmp_header_buf, remain_size);
                            outbuf = &pcmWriteBuf[0];
                            out_buf_pos = 0;
                            memcpy(&outbuf[out_buf_pos], &tmp_header_buf[remain_size], (sizeof(SPDIF_BURST_HEADER) - remain_size));
                            out_buf_pos = (sizeof(SPDIF_BURST_HEADER) - remain_size);
                            remain_size = sizeof(pcmWriteBuf) - out_buf_pos;
                        }
                        else
                        {
                            memcpy(&outbuf[out_buf_pos], &ac3Header, sizeof(SPDIF_BURST_HEADER));
                            outbuf[out_buf_pos + 6] = (unsigned char) (((frame_length << 3) & 0xFF00) >> 8);
                            outbuf[out_buf_pos + 7] = (unsigned char) (((frame_length << 3) & 0x00FF));
                            out_buf_pos += sizeof(SPDIF_BURST_HEADER);
                            remain_size -= sizeof(SPDIF_BURST_HEADER);
                        }
                        nWriteDataBytes  += sizeof(SPDIF_BURST_HEADER);

                        // Write AC3 Data
                        if (remain_size < frame_length)
                        {
                            if (isSPDIFMute())
                            {
                                // software mute
                                memcpy(&outbuf[out_buf_pos], 0x0, remain_size);
                                outbuf = &pcmWriteBuf[0];
                                out_buf_pos = 0;
                                memcpy(&outbuf[out_buf_pos], 0x0, (frame_length - remain_size));
                                out_buf_pos = (frame_length - remain_size);
                                remain_size = sizeof(pcmWriteBuf) - out_buf_pos;                                
                            }
                            else
                            {
                                memcpy(&outbuf[out_buf_pos], inbuf, remain_size);
                                outbuf = &pcmWriteBuf[0];
                                out_buf_pos = 0;
                                memcpy(&outbuf[out_buf_pos], inbuf + remain_size, (frame_length - remain_size));
                                out_buf_pos = (frame_length - remain_size);
                                remain_size = sizeof(pcmWriteBuf) - out_buf_pos;
                            }
                        }
                        else
                        {
                            if (isSPDIFMute())
                            {
                                memcpy(&outbuf[out_buf_pos], 0x0, frame_length);
                                out_buf_pos += frame_length;
                                remain_size -= frame_length;
                            }
                            else
                            {
                                memcpy(&outbuf[out_buf_pos], inbuf, frame_length);
                                out_buf_pos += frame_length;
                                remain_size -= frame_length;
                            }
                        }
                        nWriteDataBytes  += frame_length;

                        // Write stuffing
                        if (remain_size < gTailLength)
                        {
                            memset(&outbuf[out_buf_pos], 0x0, remain_size);
                            outbuf = &pcmWriteBuf[0];
                            out_buf_pos = 0;
                            memset(&outbuf[out_buf_pos], 0x0, (gTailLength - remain_size));
                            out_buf_pos = (gTailLength - remain_size);
                        }
                        else
                        {
                            memset(&outbuf[out_buf_pos], 0x0, gTailLength);
                            out_buf_pos += gTailLength;
                        }
                        nReadBytes = frame_length;
                        nWriteDataBytes  += gTailLength;
#else
                        memcpy(&outbuf[out_buf_pos], &ac3Header, sizeof(SPDIF_BURST_HEADER));
                        outbuf[out_buf_pos + 6] = (unsigned char) (((frame_length << 3) & 0xFF00) >> 8);
                        outbuf[out_buf_pos + 7] = (unsigned char) (((frame_length << 3) & 0x00FF));
                        out_buf_pos += sizeof(SPDIF_BURST_HEADER);
                        memcpy(&outbuf[out_buf_pos], inbuf, frame_length);
                        out_buf_pos += frame_length;
                        nReadBytes = frame_length;
                        memset(&outbuf[out_buf_pos], 0x0, gTailLength);
                        out_buf_pos += gTailLength;
                        nWriteDataBytes  = out_buf_pos;
#endif
                    }
                    else
                    {
                        nReadBytes = bytesLeft;
                        nWriteDataBytes = 0;
                    }
                }
            }

            //PRINTF("#%04d: error %d %d\n", nFrames, err, nReadBytes);

            if (err) {
                PRINTF("Error Code: %d\n", err);
                nWriteDataBytes  = 0;
                nReadBytes = 1;
                if (eofReached) lastRound = 1;
                continue; // Skip error
            }

            /* Initialize DAC */
            if (!bInitDAC) {
                // ac3FrameStep is 1.31 format.
                unsigned long long n = ((unsigned long long)OUTPUT_SAMPLES) << 31;

                #if !defined(DUMP_PCM_DATA)
                printf("%s(%d), sizeof pcm buffer: %u bytes\n", __FILE__, __LINE__, sizeof(pcmWriteBuf));
                if (gSampleRate)
                {
                    initDAC(pcmWriteBuf, OUTPUT_CHANNELS, gSampleRate, sizeof(pcmWriteBuf), 0);
                }
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

        ClearRdBuffer();

        // Only do once on WIN32 platform.
        #if defined(WIN32) || defined(__CYGWIN__)
        break;
        #endif // defined(WIN32) || defined(__CYGWIN__)
    } /* end of forever loop */

#if !defined(__FREERTOS__)
    return 0;
#endif
}

