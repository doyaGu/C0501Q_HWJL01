/////////////////////////////////////////////////////////////////
//                      Include File
/////////////////////////////////////////////////////////////////
#include <stdio.h>

#include "mp3_config.h"

#if defined(__OR32__)
#   include "i2s.h"
#   include "mmio.h"
//#   include "sys.h"
#endif

#if defined(__FREERTOS__)
#   include "FreeRTOS.h"
#   include "task.h"
#endif

#include "io.h"

#if defined(VOICEOFF)
#  include "voiceoff.h"
#endif

#if defined(DRC)
# include "drc.h"
#endif

#if defined(__INPUT_CRC_CHECK__)
# include "crc32.h"
#endif

//#if defined(ENABLE_CODECS_PLUGIN)
# include "plugin.h"
//#endif

//#include "ticktimer.h"
extern unsigned char streamBuf[];
extern int streamLen;
extern unsigned int* gtAudioPluginBufferLength;
extern unsigned short* gtAudioPluginBuffer;

/////////////////////////////////////////////////////////////////
//                      Local Variable
/////////////////////////////////////////////////////////////////
static unsigned char inbuf[READBUF_SIZE-2];
static int sampbuf[MAX_NCHAN * MAX_FRAMESIZE];
static short pcmbuf[MAX_NCHAN * MAX_FRAMESIZE];

#if defined(ENABLE_PERFORMANCE_MEASURE)
# include "ticktimer.h"
# define MAX_TIME_LOG 12000
  static unsigned int time_log[MAX_TIME_LOG];
  static unsigned int timeIdx = 0;
#endif // ENABLE_PERFORMANCE_MEASURE

#ifdef MP3_FLOW_CONTROL
   static PAL_CLOCK_T tContextSwitch;      
#endif

#ifdef MP3_PERFORMANCE_TEST
static PAL_CLOCK_T tClockPerformance;
static long nDecPerformance;  // avg of frame
#endif
typedef enum _DECODER_STATE {
    DECODER_STATE_RESET         = 0x0,
    DECODER_STATE_WAIT_STREAM_BUF = 0x1,
    DECODER_STATE_WAIT_I2S_BUF    = 0x2
} DECODER_STATE;

/////////////////////////////////////////////////////////////////
//                      Global Variable
/////////////////////////////////////////////////////////////////
MP3DecInfo mp3DecInfo;
MP3FrameInfo mp3FrameInfo;
int nFrames;
int nDiffInfoCount = -1;
#if defined(DUMP_PCM_DATA) || defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
int nFillSize;
#endif // defined(DUMP_PCM_DATA) || defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)

#if defined(ENABLE_SBC)
sbc_t sbc;
#endif // defined(ENABLE_SBC)

#if defined(EQUALIZER)
# include "equalizer.h"
typedef struct _EQINFO {
  short int updEQ;
  short int endianEQ;
  short int bandcenter[16];
  short int dbtab[16];
} EQINFO;

# if defined(WIN32) || defined(__CYGWIN__)
EQINFO eqinfo = {
    1, 1,
    {  0,  82, 251, 404, 557, 732, 907, 1098, 1328, 1600, 3300, 6000, 13000, 16000, 25000,    -1},
    {  1,   2,   2,   3,   3,   4,   1,    0,   -4,   -7,   -7,   -7,    -7,    -7,    -7,     0},
};
# else
extern EQINFO eqinfo;
# endif // defined(WIN32) || defined(__CYGWIN__)
static EQINFO *_eqinfo;
#endif  // defined(EQUALIZER)

#if defined(REVERBERATION)
# include "reverb.h"
# if defined(REVERB_DYNAMIC_MALLOC)
  int *reverbBuf = NULL;
# else
  int reverbBuf[(REVERB_BUF_SIZE/sizeof(int))];
# endif   // REVERB_DYNAMIC_MALLOC
typedef struct _REVERBINFO {
    short int updReverb;
    short int endianReverb;
    int src_gain;
    int reverb_gain;
    int cross_gain;
    int filter[2][8];
} REVERBINFO;

#if defined(WIN32) || defined(__CYGWIN__)
REVERBINFO revbinfo = {
    1, 1,
    DATA_CONST(0.5),
    DATA_CONST(0.5),
    DATA_CONST(0.0),
    { { 15, DATA_CONST(0.35), 40, DATA_CONST(0.25), 00, DATA_CONST(0.00), 30, DATA_CONST(0.45) } ,
      { 15, DATA_CONST(0.35), 40, DATA_CONST(0.25), 00, DATA_CONST(0.00), 30, DATA_CONST(0.45) }
    }
};
#else
extern REVERBINFO revbinfo;
#endif  // defined(WIN32) || defined(__CYGWIN__)
static REVERBINFO *_revbinfo;
#endif  // REVERBERATION

static unsigned short nMp3DecodedByte=0;

#if defined(API_V2)
extern unsigned char pcmWriteBuf[I2SBUFSIZE];

#if defined(ENABLE_MIXER)
extern HWORD pcmoutbuf[OBUFFSIZE];
#endif // defined(ENABLE_MIXER)

SHAREINFO shareinfo __attribute__ ((section (".common_var")));
SHAREINFO shareinfo = {
    .tag           = 1,
    .reverbLength  = REVERB_BUF_SIZE,
    .decBufOffset  = 0,
    .minDecBufSize = MAINBUF_SIZE,
    .minEncBufSize = 0,
    .minMixBufSize = 0,
    .inBufBase     = 0,
    .inBufLen      = 0,
#if !defined(ENABLE_MIXER)
    .outBufBase    = (int)pcmWriteBuf,
    .outBufLen     = sizeof(pcmWriteBuf),
#else
    .outBufBase    = (int)pcmoutbuf,
    .outBufLen     = sizeof(pcmoutbuf),
#endif // defined(ENABLE_MIXER)
    .decBufBase    = 0,
    .decBufLength  = 0,
    .encBufBase    = 0,
    .encBufLength  = 0,
    .mixBufBase    = 0,
    .mixBufLength  = 0,
    .mixNChannels  = 0,
    .mixSampleRate = 0,
    .reverbBase    = 0,
    .decodeTime    = 0,
    .encodeTime    = 0,
    .sampleRate    = 0,
    .nChanels      = 0
};
#endif // defined(API_V2)

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
extern int gPause;
#endif
/////////////////////////////////////////////////////////////////
//                      Local Function
/////////////////////////////////////////////////////////////////
#if defined(REVERBERATION) && defined(REVERB_DYNAMIC_MALLOC)
static void allocate_reverbbuf(void)
{
    #if defined(WIN32) || defined(__CYGWIN__)
    reverbBuf = SYS_Malloc(REVERB_BUF_SIZE);
    #else
    #  if defined(API_V2)
    reverbBuf = (int*)shareinfo.reverbBase;
    #  else
    reverbBuf = (int*)((((unsigned int)MMIO_Read(DrvDecode_RevbBase))&0xffff) +
                       (((unsigned int)(MMIO_Read(DrvDecode_RevbBase+2))) << 16));
    #  endif // defined(API_V2)
    #endif // defined(WIN32) || defined(__CYGWIN__)

    #if defined(WIN32) || defined(__CYGWIN__)
    if(!reverbBuf){
        PRINTF("memory insufficient for reverberation\n");
        exit(-1);
    }
    #endif
}
#endif

static void update_parameter(param_struct *p_param, int nchans, int samprate,
                             int nsamples)
{
    int i=0;

    #if defined(REVERBERATION)
    static int previousReverb = 0;
    #endif

    #if defined(DRC)
    static int previousDRC = 0;
    #endif

    p_param->param_eq.enable     = isEQ();
    p_param->param_reverb.enable = isReverbOn();
    p_param->param_drc.enable    = isDrcOn();
    p_param->param_voff.enable   = isVoiceOff();

    #if defined(DRC)
    if (p_param->param_drc.enable)
    {
        if (!previousDRC) 
        {
            drc_init(nchans, nsamples);
        }
        previousDRC = 1;
    } 
    else
    {
        previousDRC = 0;
    }
    #endif // defined(DRC)

    #if defined(EQUALIZER)
    if(p_param->param_eq.enable)
    {
        #if !defined(__FREERTOS__)
        //or32_invalidate_cache(_eqinfo, sizeof(EQINFO));
        #endif
        if(_eqinfo->updEQ != 0) 
        {
            // if (p_param->param_drc.enable) drc_init(nchans, nsamples);
            if(_eqinfo->endianEQ != 1) 
            {
                unsigned short tmp;
                _eqinfo->endianEQ = 1;
                for (i=0; i<sizeof(_eqinfo->dbtab)/sizeof(short); i++) 
                {
                    tmp = _eqinfo->dbtab[i];
                    _eqinfo->dbtab[i] = (tmp << 8) + (tmp >> 8);
                    tmp = _eqinfo->bandcenter[i];
                    _eqinfo->bandcenter[i] = (tmp << 8) + (tmp >> 8);
                }
            }

            for(i=0; i<16; i++)
            {
                p_param->param_eq.dbtab[i]      = _eqinfo->dbtab[i];
                p_param->param_eq.bandcenter[i] = _eqinfo->bandcenter[i];
            }
            PRINTF("updateEQ\n");
            _eqinfo->updEQ = 0;
        }
    }
    #endif

    #if defined(REVERBERATION)
    if(p_param->param_reverb.enable)
    {
        if (previousReverb == 0) 
        {
            #if defined(REVERB_DYNAMIC_MALLOC)
            allocate_reverbbuf();
            #endif
            // if (p_param->param_drc.enable) drc_init(nchans, nsamples);
            reverb_init(nchans, samprate, reverbBuf);
        }
        previousReverb = 1;

        #if !defined(__FREERTOS__)
        //or32_invalidate_cache(_revbinfo, sizeof(REVERBINFO));
        #endif
        if(_revbinfo->updReverb != 0) 
        {
            // if (p_param->param_drc.enable) drc_init(nchans, nsamples);
            reverb_init(nchans, samprate, reverbBuf);

            // Translate the little endian to big endian
            if(_revbinfo->endianReverb != 1) 
            {
                unsigned int *ptr = (unsigned int *)_revbinfo->filter;

                _revbinfo->endianReverb = 1;
                _revbinfo->src_gain     = TO_INT(_revbinfo->src_gain);
                _revbinfo->reverb_gain  = TO_INT(_revbinfo->reverb_gain);
                _revbinfo->cross_gain   = TO_INT(_revbinfo->cross_gain);

                for(i=0; i<sizeof(_revbinfo->filter)/sizeof(int); i++) 
                {
                    ptr[i] = TO_INT(ptr[i]);
                }
            }

            p_param->param_reverb.src_gain = _revbinfo->src_gain;
            p_param->param_reverb.reverb_gain = _revbinfo->reverb_gain;
            p_param->param_reverb.delay[0] = _revbinfo->filter[0][0];
            p_param->param_reverb.gain[0]  = _revbinfo->filter[0][1];
            p_param->param_reverb.delay[1] = _revbinfo->filter[0][2];
            p_param->param_reverb.gain[1]  = _revbinfo->filter[0][3];
            p_param->param_reverb.delay[2] = _revbinfo->filter[0][4];
            p_param->param_reverb.gain[2]  = _revbinfo->filter[0][5];

            _revbinfo->updReverb = 0;
            PRINTF("updateReverb\n");
        }
    }
    else 
    {
        previousReverb = 0;
    }
    #endif

    #if defined(DRC)
    if(p_param->param_drc.enable)
    {
        p_param->param_drc.digital_gain = 80;  // 80% volume
    }
    #endif

}

static void init_parameter(param_struct *p_param)
{
    memset(p_param, 0, sizeof(param_struct)/sizeof(short));

    // Force to reload the parameter of EQ & Reverb
    #if defined(REVERBERATION)
    _revbinfo->updReverb = 1;
    #endif

    #if defined(EQUALIZER)
    _eqinfo->updEQ     = 1;
    #endif
}

//**************************************************************************************
// Function:    FillReadBuf()
//  RW: bytesLeft

//  R:  streamBuf (update data in streamBuf)
//  RW: mp3ReadPtr
//  return: 0 ... bitstream in buffer is still not enough
//             1 ... bitstream in buffer is enough
//**************************************************************************************
static __inline int FillReadBuffer(STREAMBUF *StreamBuf, int bForceFillBuffer)
{
#if !defined(DUMP_PCM_DATA) && !defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
    int bytesLeft, nRead, readCnt, align_ptr;

    ASSERT((StreamBuf->wtptr & 0x1) == 0);

    if ((StreamBuf->length - StreamBuf->rdptr < MAINBUF_SIZE) || bForceFillBuffer) 
    {
        // move last, small chunk from end of buffer to start, then fill with new data
        align_ptr = StreamBuf->rdptr & 0xfffffffe;
        bytesLeft = StreamBuf->length - align_ptr;
        memmove(StreamBuf->buf, StreamBuf->buf + align_ptr, bytesLeft);
        StreamBuf->rdptr = StreamBuf->rdptr & 0x1;
        StreamBuf->wtptr -= align_ptr;
    }

    if (StreamBuf->wtptr < StreamBuf->length && !StreamBuf->eof) 
    {
        if (streamHasNBytes(MAINBUF_SIZE) == 0)
            return 0;

        readCnt = MIN((MAINBUF_SIZE & 0xfffffffe), (StreamBuf->length - StreamBuf->wtptr));
        nRead = streamRead(StreamBuf->buf + StreamBuf->wtptr, readCnt);

        #if defined(__INPUT_CRC_CHECK__)
        {
            static int crc = 0;
            crc = crc32(StreamBuf->buf + StreamBuf->wtptr, readCnt);
            PRINTF("CRC: 0x%08x\n", crc);
        }
        #endif // defined(__INPUT_CRC_CHECK__)

        if (nRead == 0)
        {
            StreamBuf->eof = 1;
        }
        StreamBuf->wtptr += nRead;
    }

    // It should be invalidate the cache line which it is in the
    // begin of input buffer. The previous frame will
    // prefetch the data to the cache line, but the driver dose
    // not yet put it in the input buffer. It will cause the
    // unconsistency of cache.
    #if !defined(__FREERTOS__)
    //or32_invalidate_cache(StreamBuf->buf + StreamBuf->rdptr, 4);
    #endif
#else // defined(DUMP_PCM_DATA)
    int nRead = 0;

    PRINTF("FillReadBuffer(){\n");
    PRINTF("++ FillReadBuffer nFillSize: %d\n", nFillSize);

    // if stream buffer is end or it has one frame in it,
    // return true.
    if(StreamBuf->eof || nFillSize == 0)
    {
        PRINTF("++ Stream EOF\n}\n");
        return 1;
    }

    // clean stream buffer status
    // if nFillSize == 4 means the stream buffer is clean
    // and need to read a completely frame (with header) later.
    StreamBuf->rdptr = 0;
    StreamBuf->wtptr = nFillSize == 4 ? 0 : StreamBuf->wtptr;

    // check if input stream has at least nFillSize to read
    if ((nRead = streamHasNBytes(nFillSize)) != 1)
    {
        StreamBuf->eof = (nRead == -1) ? 1 : 0;
        PRINTF("++ Insufficient frame data%s\n}\n", (StreamBuf->eof ? ", Stream End" : ", wait for more data"));
        return 0;
    }

    // It should be invalidate the cache line which it is in the
    // begin of input buffer. The previous frame will
    // prefetch the data to the cache line, but the driver dose
    // not yet put it in the input buffer. It will cause the
    // unconsistency of cache.
    #if !defined(__FREERTOS__)
        //or32_invalidate_cache(StreamBuf->buf + StreamBuf->rdptr, 4);
    #endif

    // if StreamBuf->wtptr == 0 means we do not read frame header yet,
    // seek input stream to a frame header and read header to stream buffer first.
    if (StreamBuf->wtptr == 0)
    {
        switch(streamSeekSyncword(1, &nRead))
        {
        case 1:     // syncword is found
            break;
        case -1:    // syncword is not found and input stream is ended
            PRINTF("++ Seek to end of stream\n}\n");
            return ((StreamBuf->eof = 1) == 1);
        default:
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
            if (nFrames>20){
               // ithMp3Printf("[Mp3] FillReadBuffer can not find syncword %d \n",nFrames);
              //  AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
            }
#else
            printf("[Mp3] FillReadBuffer can not find syncword\n");
#endif
            return 0;
        }
        StreamBuf->wtptr = streamRead(StreamBuf->buf, 4);
    }

    // if StreamBuf has a frame header in it, unpack the header and get
    // actual frame size.
    nRead = UnpackFrameHeader(StreamBuf->buf) - 4;
    if (nRead ==ERR_MP3_INVALID_ENCODEFILE-4)
    {
        nDiffInfoCount++;
        //return ((nFillSize = 4) == 0);
    }

    if(nRead < 0 || nDiffInfoCount>ENCODE_ERROR_COUNT)
    {
        PRINTF("++ Error: invalid frame header (%d)\n", nRead);
        return ((nFillSize = 4) == 0);
    }
#if 0
    // check if the format of input frame is free format
    if ( (mp3DecInfo.bitrate == 0 && mp3DecInfo.freeBitrateFlag == 0)) 
    {
        // if the input frame is the first frame, then we should
        // search its possible frame lenght.
        if(mp3DecInfo.freeBitrateFlag == 0)
        {
            int result = 0;
            PRINTF("++ MP3 free format: search for the second syncword\n");

            // searching the start position of the second frame by seeking
            // syncword and read the complete frame bytes to StreamBuf.
            while((result = streamSeekSyncword(0, &nFillSize)) != 0)
            {
                // judge retuned result of streamSeekSyncword
                // if result = -1 means end of stream (usually can not happend)
                // return it immediatly.
                if(result == -1)
                {
                    PRINTF("++ Unknow error, cannot search next syncword\n}\n");
                    return ((StreamBuf->eof = 1) == 1);
                }

                // check if StreamBuf has enough free space.
                if((StreamBuf->wtptr + nFillSize + 4) >  StreamBuf->length)
                    break;

                // read data bytes to StreamBuf
                nRead = streamRead(StreamBuf->buf + StreamBuf->wtptr, nFillSize);
                StreamBuf->wtptr += nRead;

                // compare the finding syncword with the one of the first frame
                // if they are not similar, searching for next candidate.
                // the comparision can avoid seeking wrong syncword.
                if(streamCompareSyncword(StreamBuf->buf) != 1)
                {
                    nRead = streamRead(StreamBuf->buf + StreamBuf->wtptr, 4);
                    StreamBuf->wtptr += nRead;
                    continue;
                }

                // syncword is found and the data has been read, no more data
                // have to read until the frame has been decode.
                PRINTF("++ MP3 free format: read a complete frame\n}\n");
                return ((nFillSize = 0) == 0);
            }

            // syncword can not be found and buffer is empty,
            // return failed
            PRINTF("++ MP3 free format: cannot read a complete frame\n}\n");
            return ((nFillSize = 4) == 0);
        }

        // caculate partial size of the actual frame size
        // note1: after first frame is successfully decoded, the slot size for this stream
        //        has been decided and saved to mp3DecInfo.freeBitrateSlots.
        // note2: padding byte and possible remain size of header frame have not been added.
        nFillSize = mp3DecInfo.freeBitrateSlots + mp3DecInfo.FrameHeaderPS.paddingBit + nRead;
    }
    else
#endif        
    {
        // caculate partial size of the actual frame size
        // note1: UnpackFrameHeader returns actually header size (4 or 6 bytes).
        //        We have already read 4 bytes, thus we must read remaider.
        // note2: mp3DecInfo.nSlots only specifies its frame data size, thus the
        //        total frame size should be:
        //              mp3DecInfo.nSlots + frame header size + side info size.
        nFillSize = mp3DecInfo.nSlots + nRead;
    }

    // judge side info size
    if(mp3DecInfo.FrameHeaderPS.ver == MPEG1)
    {
        nFillSize += mp3DecInfo.FrameHeaderPS.sMode == Mono ? SIBYTES_MPEG1_MONO : SIBYTES_MPEG1_STEREO;
    }
    else
    {
        nFillSize += mp3DecInfo.FrameHeaderPS.sMode == Mono ? SIBYTES_MPEG2_MONO : SIBYTES_MPEG2_STEREO;
    }

    // check if input stream has enough data to read.
    if((nRead = streamHasNBytes(nFillSize)) != 1)
    {
        StreamBuf->eof = (nRead == -1) ? 1 : 0;
        PRINTF("++ Insufficient frame slot data%s\n}\n", (StreamBuf->eof ? ", Stream End" : ", wait for more data"));
        return 0;
    }

    // read data bytes to StreamBuf
    nRead = streamRead(StreamBuf->buf + StreamBuf->wtptr, nFillSize);
    StreamBuf->wtptr += nRead;

    PRINTF("++ Syncword: %02x %02x %02x %02x Frame length:%d, nRead=%d, StreamBuf->wtptr=%d\n}\n", StreamBuf->buf[0], StreamBuf->buf[1], StreamBuf->buf[2], StreamBuf->buf[3], mp3DecInfo.nSlots, nRead, StreamBuf->wtptr);

    // if stream buffer has a completely frame, then we do not need to fill stream bufer
    // until the frame has been decode.
    nFillSize = 0;
#endif

    return 1;
}

//**************************************************************************************
// Function:    MP3FindSyncWord
// MP3FindSyncWord for INPUT_FILEMODE & INPUT_MEMMODE
// Description: locate the next byte-alinged sync word in the raw mp3 stream
// RW: mp3ReadIdx (update mp3ReadIdx at first sync word or mp3WriteIdx - 1 if not found)
// R:  mp3WriteIdx
// R:  streamBuf
// return:  1 = find syncword, 0 = not find syncword
//
// MP3FindSyncWord for INPUT_DRIVERMODE
// Description: locate the next byte-alinged sync word in the raw mp3 stream
//              during find syncword, update mp3ReadIdx, mp3WriteIdx
//              if run out buffer but not found syncword, this fucntion will
//              wait driver to refill buffer and continue to find until
//              1. syncword is found
//              2. syncword not found but end of file
//              perform memory movement if necessary
//
// RW: mp3ReadIdx
// R:  mp3WriteIdx
// R:  streamBuf
// return:  1 = find syncword, 0 = not find syncword
//**************************************************************************************
#if !defined(DUMP_PCM_DATA) && !defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
static int MP3FindSyncWord(STREAMBUF *StreamBuf)
{
    int i, findSync=0;

    // find byte-aligned syncword - need 12 (MPEG 1,2) or 11 (MPEG 2.5) matching bits
    for (i = StreamBuf->rdptr; i < StreamBuf->wtptr - 1; i++) 
    {
        if ( (StreamBuf->buf[i+0] & SYNCWORDH) == SYNCWORDH && (StreamBuf->buf[i+1] & SYNCWORDL) == SYNCWORDL ) 
        {
            findSync = 1;
            break;
        }
    }
    StreamBuf->rdptr = i;
    return findSync;
}
#endif // !defined(DUMP_PCM_DATA) && !defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)

static void init_streambuf(STREAMBUF *StreamBuf, unsigned char *bufptr, int length)
{
    StreamBuf->buf    = bufptr;
    StreamBuf->rdptr  = StreamBuf->wtptr = 0;
    StreamBuf->length = length;
    StreamBuf->eof    = 0;
}

static void init_sampbuf(SAMPBUF *SampBuf, int *bufptr, int nch, int nsamples)
{
    SampBuf->buf = bufptr;
    SampBuf->nch = nch;
    SampBuf->nsamples = nsamples;
}

#define SCALE   32768

static __inline short CLIPTOSHORT(int x)
{
    int sign;

    /* clip to [-32768, 32767] */
    sign = x >> 31;
    if (sign != (x >> 15))
        x = sign ^ ((1 << 15) - 1);

    return (short)x;
}

static void sample2pcm(SAMPBUF *SampBuf,  short *writeptr)
{
    int i;
    int (*buf)[MAX_FRAMESIZE], nch, nsamples;

    nch = SampBuf->nch;
    nsamples = SampBuf->nsamples;
    buf = (int(*)[MAX_FRAMESIZE])SampBuf->buf;

    if (nch == 1) 
    {
        for (i = 0; i < nsamples; i++) 
        {
            *writeptr++ = CLIPTOSHORT(buf[0][i]);
        }
    }
    else
    {
        for (i = 0; i < nsamples; i++) 
        {
            *writeptr++ = CLIPTOSHORT(buf[0][i]);
            *writeptr++ = CLIPTOSHORT(buf[1][i]);
        }
    }
}

//#define DEBUG_PRINTSTATE
#if defined(DEBUG_PRINTSTATE)
static void PrintState(DECODER_STATE decstate)
{
    char *ptr;
    static DECODER_STATE decstate_prev = DECODER_STATE_RESET;
    if (decstate != decstate_prev) 
    {
        switch(decstate)
        {
        case DECODER_STATE_RESET:
            ptr = "DECODER_STATE_RESET\n";
            break;
        case DECODER_STATE_WAIT_STREAM_BUF:
            ptr = "DECODER_STATE_WAIT_STREAM_BUF\n";
            break;
        case DECODER_STATE_WAIT_I2S_BUF:
            ptr = "DECODER_STATE_WAIT_I2S_BUF\n";
            break;
        default:
            ptr = "DECODER_STATE_ERROR\n";
        }
        PRINTF("# %s", ptr);
        decstate_prev = decstate;
    }
}
#else
#define PrintState(decstate)
#endif

/////////////////////////////////////////////////////////////////
//                      Global Function
/////////////////////////////////////////////////////////////////
# if defined(__FREERTOS__) && !defined(ENABLE_CODECS_PLUGIN)
portTASK_FUNCTION(mp3decode_task, params)
# else
int main(int argc, char **argv)
# endif
{
    param_struct param;
    int* codecStream;
    AUDIO_CODEC_STREAM* audioStream;
    
    int nKeepByte = 0;
#ifdef MP3_PERFORMANCE_TEST_BY_TICK
    int  nNewTicks,nTotalTicks;
    int  nTotalTime;
#endif

    #if defined(EQUALIZER)
        #if defined(ENABLE_CODECS_PLUGIN)
        //_eqinfo   = ci->eqinfo;
        #else
        _eqinfo   = &eqinfo;
        #endif
    #endif // EQUALIZER

    #if defined(REVERBERATION)
        #if defined(ENABLE_CODECS_PLUGIN)
        _revbinfo = ci->revbinfo;
        #else
        _revbinfo = &revbinfo;
        #endif
    #endif // REVERBERATION

    #if defined(WIN32) || defined(__CYGWIN__)
        GetParam(argc, argv);
        win32_init();
        initDbg();
    #endif // defined(WIN32) || defined(__CYGWIN__)

    #if defined(MP3_DUMP_PCM) && defined(__FREERTOS__)
        f_enterFS();
    #endif

    #if defined(ENABLE_PERFORMANCE_MEASURE)
    {
        int i;
        for(i=0; i<sizeof(time_log)/sizeof(int); i++) 
        {
            time_log[i]=0;
        }
        timeIdx = 0;
    }
    #endif // defined(ENABLE_PERFORMANCE_MEASURE)

    
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    codecStream=CODEC_STREAM_START_ADDRESS;   
    //printf("[Mp3] 0x%08x \n",*codecStream);
    audioStream = *codecStream;
    audioStream->codecStreamBuf = &streamBuf[0];
    audioStream->codecStreamLength = streamLen;   
    //printf("[Mp3] audioStream 0x%08x 0x%08x 0x%08x   \n",&audioStream,&audioStream->codecStreamBuf,&audioStream->codecStreamLength);
    gtAudioPluginBuffer = audioStream->codecAudioPluginBuf;
    gtAudioPluginBufferLength = audioStream->codecAudioPluginLength;
    //printf("[Mp3] 0x%08x %d 0x%08x %d \n",audioStream->codecStreamBuf,audioStream->codecStreamLength ,audioStream->codecAudioPluginBuf,audioStream->codecAudioPluginLength);

    //MMIO_Write(0x16ea, (short)gtAudioPluginBuffer);
    //MMIO_Write(0x16ee, (short)((int)gtAudioPluginBuffer>>16));	
    //setAudioMessageBuffer((int)audioStream->codecAudioPluginBuf);
    MMIO_Write(AUDIO_DECODER_START_FALG, 1);
#endif

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
        ithMp3Printf("[Mp3] start \n");
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);  
#else
        printf("[Mp3] start \n");        
#endif

    for(;;) // forever loop
    {
        STREAMBUF StreamBuf;
        SAMPBUF SampBuf;
        DECODER_STATE decstate;
        int keep_sr, keep_nsamp, keep_nch;  // keep unchanged after 1st frame decoded
        int bForceFillBuffer = 1;
        int init = 0;
        int err = 0;
        int exitflag1 = 0;
    #if defined(ENABLE_MIXER)
        int exitflag2 = 0;
    #endif // defined(ENABLE_MIXER)
        int temp;
        nDiffInfoCount = -1;

    #if defined(__INPUT_CRC_CHECK__)
        crc32_init();
    #endif // defined(__INPUT_CRC_CHECK__)

        nFrames = 0;

        keep_sr = keep_nsamp = keep_nch = -1;
        decstate = DECODER_STATE_RESET;
        PrintState(decstate);

        init_parameter(&param);
        init_streambuf(&StreamBuf, inbuf, sizeof(inbuf));
        init_inputBuf();

    #if defined(DUMP_PCM_DATA) || defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
        nFillSize = 4; // first loop read 4 bytes of mp3 header
    #endif // defined(DUMP_PCM_DATA) || defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)

        memset(&mp3DecInfo, 0, sizeof(MP3DecInfo)/sizeof(short));
        init_sampbuf(&SampBuf, sampbuf, 0, 0);

   #ifdef MP3_PERFORMANCE_TEST
        nDecPerformance=0;
   #endif
    #ifdef MP3_PERFORMANCE_TEST_BY_TICK
        nTotalTicks=0;
        nTotalTime = 0;
    #endif

        while(1)    // decode frame loop
        {
            //GetFrameNo(&nFrames);

            #if defined(ENABLE_PERFORMANCE_MEASURE)
            start_timer();
            #endif // defined(ENABLE_PERFORMANCE_MEASURE)

            if (!FillReadBuffer(&StreamBuf, bForceFillBuffer)) 
            {
                 // FillReadBuffer() return 0, bitstream in buffer is still not enough
                decstate |= DECODER_STATE_WAIT_STREAM_BUF;
                PrintState(decstate);
            }
            else
            {
                 // FillReadBuffer() return 1, bitstream in buffer is enough      
                decstate &= ~DECODER_STATE_WAIT_STREAM_BUF;
                PrintState(decstate);
                nKeepByte = StreamBuf.rdptr;
            }

            if (!(decstate & (DECODER_STATE_WAIT_STREAM_BUF|DECODER_STATE_WAIT_I2S_BUF)) )
            {
                // dec mp3 frame
                #if !defined(DUMP_PCM_DATA) && !defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
                if (!MP3FindSyncWord(&StreamBuf))
                {
                    PRINTF("Cannot find syncword\n");
                    if(StreamBuf.eof)
                    {
                        PRINTF("stream eof\n");
                        exitflag1 = 1;
                    }
                }
                #else // with new seeksyncword mechanism, we do not need to find syncword anymore.
                if (StreamBuf.eof)
                {
                    PRINTF("stream eof\n");
                    exitflag1 = 1;
                }
                #endif // !defined(DUMP_PCM_DATA) && !defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
                else
                {
                    // no more force fill with new seeksyncword mechanism.
                    #if !defined(DUMP_PCM_DATA) && !defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
                        if (bForceFillBuffer && (StreamBuf.rdptr < 2)) 
                        {
                            bForceFillBuffer = 0;
                            PRINTF("force fill buffer\n");
                        }
                        else if (!bForceFillBuffer)
                    #endif // !defined(DUMP_PCM_DATA) && !defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
                        {
                        #ifdef MP3_PERFORMANCE_TEST
                            tClockPerformance = PalGetClock();
                        #endif
                        #ifdef MP3_PERFORMANCE_TEST_BY_TICK
                            start_timer();
                        #endif                        
                            err = MP3Decode(&StreamBuf, &SampBuf, &param.param_eq);
                            PRINTF("[mp3decode] decode %dth frame, result=0x%02x\n", nFrames+1, err);
                        #ifdef MP3_RESET_DECODED_BYTE    
                            if (isResetAudioDecodedBytes())
                            {
                                MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvResetAudioDecodedBytes);
                                nMp3DecodedByte = 0;
                                MMIO_Write(DrvAudioDecodedBytes, nMp3DecodedByte);    

                                PalSleep(5);
                            }                        
                            // write mp3 decoded byte to register
                            if (StreamBuf.rdptr > nKeepByte)
                            {
                                nMp3DecodedByte += (StreamBuf.rdptr - nKeepByte);
                                MMIO_Write(DrvAudioDecodedBytes, nMp3DecodedByte); 
                                nKeepByte = StreamBuf.rdptr;
                            }
                            else
                            {
                                //printf("[Mp3] StreamBuf rdptr %d <=  nKeepByte %d\n",StreamBuf.rdptr,nKeepByte);
                            }
                       #endif  // def MP3_RESET_DECODED_BYTE       
                            
                            //PRINTF("#%d err(%d) srate(%d)\n", nFrames, err, mp3DecInfo.samprate);
                        #ifdef MP3_PERFORMANCE_TEST
                            if(!err)
                            {
                                nDecPerformance+= PalGetDuration(tClockPerformance);
                            }
                            if ( nFrames%500 == 0 )
                            {
                                printf("[MP3] average of decode time %d nDecPerformance %d nFrames %d \n",(nDecPerformance/nFrames),nDecPerformance,nFrames );
                            }
                        #endif
                        #ifdef MP3_PERFORMANCE_TEST_BY_TICK
                            nNewTicks = get_timer();            
                            nTotalTicks += nNewTicks;
                            if (nFrames % 50 == 0 && nFrames>0)
                            {
                                nTotalTime += (nTotalTicks/(PalGetSysClock()/1000));                            
                                printf("[MP3] (%d~%d) total %d (ms) average %d (ms) nFrames %d system clock %d\n",(nFrames+1-50),(nFrames+1),(nTotalTicks/(PalGetSysClock()/1000)),((nTotalTicks/(PalGetSysClock()/1000))/50),nFrames+1,PalGetSysClock());
                                nTotalTicks=0;                    
                            }
                        #endif                 

                        // if any error, output zero sample and continue to find next syncword and decode
                        #if defined(__FREERTOS__)
                            if(!err && SampBuf.nsamples > 0 )
                        #else
                            if(!err && SampBuf.nsamples > 0)
                       #endif
                            {
                                PRINTF("frame:%d, no error\n", nFrames+1);

                                if(init == 0)
                                {
                                #if defined(VOICEOFF)
                                    voiceoff_init(mp3DecInfo.samprate);
                                #endif
                                #if defined(DRC)
                                    drc_init(SampBuf.nch, SampBuf.nsamples);
                                #endif
                                    // initDAC
                                    keep_nch   = SampBuf.nch;
                                    keep_sr    = mp3DecInfo.samprate;
                                    keep_nsamp = SampBuf.nsamples;
                                    init = 1;
                                }

                                //SampBuf.nch      = keep_nch;
                                //SampBuf.nsamples = keep_nsamp;

                                #if defined(REVERBERATION)
                                if(param.param_reverb.enable)
                                    reverb_filter(&SampBuf, &SampBuf, &param.param_reverb);  // parameter
                                #endif
                                #if defined(VOICEOFF)
                                if(param.param_voff.enable)
                                    voiceoff_filter(&SampBuf);
                                #endif
                                #if defined(DRC)
                                if(param.param_drc.enable)
                                    drc_filter(&SampBuf, &SampBuf, &param.param_drc);
                                else
                                    delay_filter(&SampBuf, &SampBuf);
                                #endif

                                sample2pcm(&SampBuf, pcmbuf);
                                //nFrames++;
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                                //ithMp3Printf("[Mp3] nFrames %d\n",nFrames);
                                //AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                                printf("[Mp3] nFrames %d \n",nFrames);        
#endif
                                decstate |= DECODER_STATE_WAIT_I2S_BUF;
                                PrintState(decstate);
                            }
                            else
                            {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
                        if (nFrames>20){
                             ithMp3Printf("[Mp3] frame %d dec error 0x%x \n",nFrames+1,err);
                             AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
                        }
#else
                                printf("[Mp3] frame %d dec error 0x%x \n",nFrames+1,err);
#endif
                                MMIO_Write(AUDIO_DECODER_WRITE_DECODE_ERROR, 1);                    

                                if(StreamBuf.eof && err & (ERR_MP3_INDATA_UNDERFLOW | ERR_MP3_MAINDATA_UNDERFLOW))
                                {
                                    exitflag1 = 1;
                                }
                                decstate |= DECODER_STATE_WAIT_STREAM_BUF;
                            }

                            #if defined(DUMP_PCM_DATA) || defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
                                 nFillSize = 4;
                            #endif
                            //PalSleep(1);
                    }
                }
            }

            if (decstate & DECODER_STATE_WAIT_I2S_BUF) 
            {
                int i;
                short temp;
                
                PRINTF("outputpcm() frameno: %d\n", nFrames);
                //  stereo to mono
                if (mp3DecInfo.nChans==2 && isDownSample())
                {
                    for(i = 0; i < SampBuf.nsamples; i++)
                    {
                        temp = (pcmbuf[2*i] + pcmbuf[2*i+1])/2;
                        pcmbuf[2*i] = temp;
                        pcmbuf[2*i+1] = temp;
                    }
                }

                if (mp3DecInfo.nChans==2){                   
                    switch( getChMixMode() ) 
                    {
                        case CH_MIX_NO   : break;
                        case CH_MIX_LEFT :
                             for(i = 0; i < SampBuf.nsamples; i++)
                             {
                                 pcmbuf[2*i+1] = pcmbuf[2*i];
                             }
                             break;
                        case CH_MIX_RIGHT:
                             for(i = 0; i < SampBuf.nsamples; i++)
                             {
                                 pcmbuf[2*i] = pcmbuf[2*i+1];
                             }
                             break;
                        case CH_MIX_BOTH :
                             for(i = 0; i < SampBuf.nsamples; i++)
                             {
                                 temp = (pcmbuf[2*i] + pcmbuf[2*i+1])/2;
                                 pcmbuf[2*i] = temp;
                                 pcmbuf[2*i+1] = temp;
                             }
                             break;
                        default:
                             break;
                    }
                }
                MMIO_Write(AUDIO_PROCESSOR_WRITE_REGISTER_PROTECTION, 0);                    
                if (outputpcm(pcmbuf, keep_nch,SampBuf.nch, SampBuf.nsamples, keep_sr, mp3DecInfo.samprate))
                {
                    // outputpcm complete
                    decstate &= ~DECODER_STATE_WAIT_I2S_BUF;
                    PRINTF("output to I2S complete\n");
                    PrintState(decstate);
                    // must update time after outputpcm. Please make sure which
                    // there is no context switch between ouputpcm and
                    // SetFrameNo();
                    SetFrameNo(nFrames);
                    MMIO_Write(AUDIO_PROCESSOR_WRITE_REGISTER_PROTECTION, 0);                    
                }
                else
                {
                    PRINTF("output to I2S Incomplete\n");
                }
                MMIO_Write(AUDIO_PROCESSOR_WRITE_REGISTER_PROTECTION, 0);                    
                
                if (keep_sr != mp3DecInfo.samprate && mp3DecInfo.samprate>0) // change new sampling rate
                {
                    keep_sr = mp3DecInfo.samprate;
                }
                if (keep_nch !=SampBuf.nch && SampBuf.nch>0) // change new ch
                {
                    keep_nch = SampBuf.nch;
                }
                
                #if defined(ENABLE_SBC)
                {
                    int byteLeft = 0;
                    int nBytes = sbc_encode(&sbc, pcmbuf, byteLeft);
                }
                #endif // defined(ENABLE_SBC)
            }

            if (decstate & (DECODER_STATE_WAIT_STREAM_BUF|DECODER_STATE_WAIT_I2S_BUF))
            {
                #ifdef MP3_RESET_DECODED_BYTE    
                    if (isResetAudioDecodedBytes())
                    {
                        MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvResetAudioDecodedBytes);
                        nMp3DecodedByte = 0;
                        MMIO_Write(DrvAudioDecodedBytes, nMp3DecodedByte);                            
                    }
                
                    // write mp3 decoded byte to register
                    if (StreamBuf.rdptr > nKeepByte)
                    {
                        nMp3DecodedByte += (StreamBuf.rdptr - nKeepByte);
                        MMIO_Write(DrvAudioDecodedBytes, nMp3DecodedByte); 
                        nKeepByte = StreamBuf.rdptr;
                    }
               #endif  // def MP3_RESET_DECODED_BYTE       
            
                #if defined(__FREERTOS__)
                    PalSleep(1);
                    #ifdef MP3_FLOW_CONTROL
                        tContextSwitch = PalGetClock();
                    #endif
                #else
                    //or32_delay(10);  // enter sleep mode for power saving
                #endif
            }

            {
                int paused = 0;
                while (isPAUSE())
                {
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
                    #if defined(__FREERTOS__)
                       PalSleep(1);
                    #else

                    #endif
                    if (isSTOP()) 
                    {
                        exitflag1 = 1;
                        break;
                    }
                }
                if (paused && !isPAUSE()) 
                {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
                    gPause = 0;
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC);
#else
                    pauseDAC(0);
#endif
                }
            }

            if (isSTOP())
            {
                exitflag1 = 1;
            }

            if (init)
                update_parameter(&param, SampBuf.nch, mp3DecInfo.samprate, SampBuf.nsamples);

            #if defined(ENABLE_PERFORMANCE_MEASURE)
            {
                int elapsed = get_timer();
                PRINTF("time=%d\n", elapsed);
                if (timeIdx < (sizeof(time_log)/sizeof(int)))
                    time_log[timeIdx++] = elapsed;
            }
            #endif // defined(ENABLE_PERFORMANCE_MEASURE)

            #if defined(ENABLE_MIXER)
                exitflag2 = mixpcm(exitflag1);
                if(exitflag2)
                    break;
            #else
                if(exitflag1)
                    break;
            #endif

            #if defined(API_V2) && (defined(WIN32) || defined(__CYGWIN__))
            {
                int time = shareinfo.decodeTime / (1<<16);
                fprintf(stderr, "%02d:%02d\r", time/60, time%60);
                fflush(stderr);
            }
            #endif // defined(WIN32) || defined(__CYGWIN__)
            // reset 
            if (isResetMp3RdBufPointer())
            {

                ResetRdBufferPointer();

                resetMp3RdBufPointer();
                ithMp3Printf("[Mp3] isResetMp3RdBufPointer %d\n",isResetMp3RdBufPointer());
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
                memset(mp3DecInfo.IMDCTInfoPS.prevblck, 0, sizeof(mp3DecInfo.IMDCTInfoPS.prevblck));
            }
        }
        ClearRdBuffer();

   #ifdef MP3_RESET_DECODED_BYTE    
        if (isResetAudioDecodedBytes())
        {
            MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvResetAudioDecodedBytes);
        }
        nMp3DecodedByte = 0;
        MMIO_Write(DrvAudioDecodedBytes, nMp3DecodedByte);                            
    #endif  // def MP3_RESET_DECODED_BYTE        
                
    #ifdef MP3_PERFORMANCE_TEST
         printf("[MP3] Average of decode time %d nDecPerformance %d nFrames %d \n",(nDecPerformance/nFrames),nDecPerformance,nFrames );
    #endif
    #ifdef MP3_PERFORMANCE_TEST_BY_TICK
        if (nFrames)
        {
            printf("[MP3] Average of decode time %d (ms)  total time %d (ms) nFrames %d \n",(nTotalTime/nFrames),nTotalTime,nFrames );
        }
     #endif    
    
        #if defined(WIN32) || \
            defined(__CYGWIN__) || \
            defined(MANUFACTURE_MEMMODE) || \
            defined(MANUFACTURE_I2SMODE)
        break;
        #endif // defined(WIN32) || defined(__CYGWIN__)
    }


# if !defined(__FREERTOS__)
    return 0;
# endif

}
