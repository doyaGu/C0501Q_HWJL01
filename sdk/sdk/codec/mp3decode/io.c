
//#include <stdio.h>

#include "mp3_config.h"
#include "io.h"
#include <stdarg.h>

#if defined(USE_WOW)
#   include "wow_api.h"
#endif
#if defined(__OR32__)
#   include "mmio.h"
#   include "i2s.h"
#endif

#ifdef __FREERTOS__
#   include "FreeRTOS.h"
#   include "task.h"
#endif

#if defined(ENABLE_CODECS_PLUGIN)
#   include "plugin.h"
#endif

#ifndef min
#   define min(a,b)    (((a)<(b)) ? (a) : (b))
#endif

#if defined(INPUT_MEMMODE)
    #undef  isEOF
    #define isEOF() 1
    unsigned char streamBuf[]={
#   include "mp3file.hex"
};
    int streamLen = sizeof(streamBuf);
    static int streamRdPtr = 0;
    static int streamWrPtr = (sizeof(streamBuf) & 0xfffffffe);
#else
# if !defined(API_V2)
#   if defined(__GNUC__)
    unsigned char streamBuf[READBUF_SIZE] __attribute__ ((aligned(16)));
#   else
    unsigned char streamBuf[READBUF_SIZE];
#   endif // defined(__GNUC__)
    int streamLen = sizeof(streamBuf);
#  else
    unsigned char *streamBuf = 0;
    int streamLen = 0;
#  endif // defined(API_V2)
    static int streamRdPtr = 0;
    static int streamWrPtr = 0;
#endif

#define MP3_HI_BIT_OVER_FLOW_THRESHOLD      (0x3E8)
#define MP3_INVALID_PTS_OVER_FLOW_VALUE     (0xFFFFFFFF)
#define MP3_WRAP_AROUND_THRESHOLD           (0x3E80000) // 65536 seconds
#define MP3_JUDGE_MAXIMUM_GAP               (0x1F40000) // 36728 seconds
static int streamCurPos = 0;
static int bInitDAC = 0;
static int decTime = 0; // Decoding time in seconds on S15.16 fixed point.
static int gInitOutput = 0; //count init output device 
static unsigned int gLastPtsOverFlowSection = MP3_INVALID_PTS_OVER_FLOW_VALUE;
static unsigned int mp3FrameStep;
static unsigned int mp3FrameAccu;
#  if defined(__GNUC__)
    unsigned char pcmWriteBuf[I2SBUFSIZE] __attribute__ ((aligned(16)));
#  else
    unsigned char pcmWriteBuf[I2SBUFSIZE];
#  endif

static unsigned int  pcmReadIdx;
static unsigned int  pcmWriteIdx;

#if defined(DUMP_FRAME_CHECKSUM)
#   if defined(COMPARE_CHECKSUM)
    static unsigned short compared_chksum[] = {
    #include "checksum.hex"
    };
#   endif
    int g_checksum_cnt = 0;
    unsigned short g_checksum_array[16384];
    int g_checksum_region_array[16384*2];
    static unsigned char *checksum_startptr;
    static unsigned char *checksum_endptr;
#endif

#if defined(ENABLE_SBC)
    extern sbc_t sbc;
#endif // defined(ENABLE_SBC)

#if defined(ENABLE_MIXER)
#   include "mixinit.h"
#   include "mixer.h"

static unsigned int mixPcmReadIdx;
static unsigned int mixPcmWriteIdx;
HWORD pcminbuf[I2SBUFSIZE]; // PCM Input Buffer
HWORD pcmoutbuf[OBUFFSIZE]; // PCM Output buffer to Audio CODEC

// 2 sampleinput buffer, 2 sample output buffer
HWORD sampleinbuf0[IBUFFSIZE], sampleoutbuf0[OBUFFSIZE];
HWORD sampleinbuf1[IBUFFSIZE], sampleoutbuf1[OBUFFSIZE];

RSQUEUE  pcminque0 = {{0,0,0,0},0,0,0,0, };  // Input Queue
RSQUEUE  pcminque1 = {{0,0,0,0},0,0,0,0, };  // Input Queue
MIXQUEUE mixque    = {{0,0,0,0},0,0,     };  // Output Queue

RESAMPLEINFO rsinfo0, rsinfo1;  // resample information

int targetSampRate, targetChans;
int pcm0nchans, pcm0samps;
int pcm1nchans, pcm1samps;
int mix_enable;
static int mix_on = 0;
static int pcm1eof = 1;
#endif // defined(ENABLE_MIXER)

#if defined(API_V2)
#include "driver.h"
extern SHAREINFO shareinfo;
#endif // defined(API_V2)

#if defined(USE_PTS_EXTENSION)
static int last_rdptr = 0;
#endif // defined(USE_PTS_EXTENSION)

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
unsigned int* gtAudioPluginBufferLength;
unsigned short* gtAudioPluginBuffer;
static int gnTemp;
static unsigned char tDumpWave[] = "C:/mp3_dump.wav";
static int gCh;
static int gSampleRate;
int gPause = 0 ;
int gPrint = 0;
unsigned char *gBuf;
#endif

#ifdef MP3_DUMP_PCM
static PAL_FILE *tMp3ToPcmFileOutput = NULL;
static PAL_CLOCK_T tClockStartTimer;
#endif
static short tUpSample[5000];
static unsigned int gnMp3DropTime;
static unsigned int gnMp3DropTimeStart,gnMp3DropTimeEnd;
static int gnStartOuputEmpty =0;


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
//
// Get the stream write pointer.
//
static __inline unsigned int getStreamWrPtr(void) 
{
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

#if defined(__OPENRTOS__)
/**************************************************************************************
 * Function     : MP3GetBufInfo
 * Description  : Get buffer pointer and bufer length from MP3decoder
 * inbuf        : input streambuf pointer
 * inlen        : input streambuf length
 * Return       : none
 **************************************************************************************/
#if defined(FREQINFO)
extern char freqinfo[FREQINFOCNT];
extern char gSpectrum[FREQINFOCNT*FREQINFOARRAY];
#endif

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
void MP3_GetBufInfo(unsigned* inbuf, unsigned* inlen , unsigned* freqbuf, unsigned* pos,unsigned* audioPluginBuf, unsigned* audioPluginBufLen)
{
#if 0
    *inbuf = (unsigned)&streamBuf[0];
    *inlen = streamLen;
    #if defined(FREQINFO)
    *freqbuf = (unsigned)&freqinfo[0];
    #endif
    *pos = (unsigned)&streamCurPos;

    gtAudioPluginBuffer = (unsigned short*)audioPluginBuf;
    gtAudioPluginBufferLength = audioPluginBufLen;
    //printf("[Mp3] plugin buffer length %d  0x%x buf 0x%x \n",*gtAudioPluginBufferLength,audioPluginBufLen,audioPluginBuf);
#endif    
}

#else // defined(AUDIO_PLUGIN_MESSAGE_QUEUE)

void MP3_GetBufInfo(unsigned* inbuf, unsigned* inlen, unsigned* freqbuf, unsigned* pos)
{
    *inbuf = (unsigned)&streamBuf[0];
    *inlen = streamLen;
    #if defined(FREQINFO)
    *freqbuf = (unsigned)&freqinfo[0];
    #endif
    *pos = (unsigned)&streamCurPos;
}
#endif // defined(AUDIO_PLUGIN_MESSAGE_QUEUE)

#endif //defined(__FREERTOS__)

// point to the input buffer
void init_inputBuf()
{
#if defined(API_V2)
    streamBuf = (unsigned char*)shareinfo.decBufBase;
    streamLen = (int)shareinfo.decBufLength;
#endif // defined(API_V2)
}

// test if stream buffer has at least nwaitbytes
// return value: 1 if wait stream buffer has at least nwaitbytes
//              -1 isEOF
//               0 otherwise
int streamHasNBytes(int nwaitbytes)
{
    # if defined(INPUT_MEMMODE)
    int nbytes = streamWrPtr - streamRdPtr;
    # else
	int nbytes;
    streamWrPtr = getStreamWrPtr();
    nbytes = (streamRdPtr <= streamWrPtr) ? (streamWrPtr - streamRdPtr) : (streamLen - (streamRdPtr - streamWrPtr));
    # endif

    if(nbytes >= nwaitbytes)
        return 1;

    if(isEOF())
    {
        MMIO_Write(DrvDecode_RdPtr, getStreamWrPtr());
        return -1;
    }

    return 0;

#if 0
    int nbytes;
    if (isEOF()) {
        return -1;
    }
    else {
        # if defined(INPUT_MEMMODE)
        nbytes = streamWrPtr - streamRdPtr;
        # else
        streamWrPtr = getStreamWrPtr();
        if (streamRdPtr <= streamWrPtr) {
            nbytes = streamWrPtr - streamRdPtr;
        }
        else {
            nbytes = streamLen - (streamRdPtr - streamWrPtr);
        }
        # endif
        if (nbytes >= nwaitbytes)
            return 1;
        else
            return 0;
    }
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

void output_and_count(struct parameters *p, int c)
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

void output_field(struct parameters *p, char *s)
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


int ithGMp3Printf(const char *control_string, va_list va)
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

int ithMp3Printf(char* control, ...)
{
    va_list va;
    va_start(va,control);
    gPrint = 0;
    gBuf = (unsigned char*)gtAudioPluginBuffer;
    ithGMp3Printf(control, va);
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
            //printf("[Mp3] name length %d \n",gnTemp);
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

        case SMTK_AUDIO_PLUGIN_CMD_ID_SPECTRUM:
                pBuf = (unsigned char*)gtAudioPluginBuffer;

                if (nFrames==0)            
                    nTemp  = 0;
                else
                    nTemp =nFrames ;
                memcpy(&pBuf[0], &nTemp, sizeof(int));
                memcpy(&pBuf[4], gSpectrum, FREQINFOCNT*FREQINFOARRAY);

                memset(gSpectrum,0,FREQINFOCNT*FREQINFOARRAY);

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
    //if (i==0)
        //printf("[Mp3] audio api %d %d\n",i,nType);

}
#endif // defined(AUDIO_PLUGIN_MESSAGE_QUEUE)


#if defined(DUMP_PCM_DATA) || defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
int streamCompareSyncword(unsigned char* sync)
{
    int rdInx = streamRdPtr;
#if defined(__DEBUG__)
    int rdInx2 = streamRdPtr;
    int i = 0;
    PRINTF(" Report@streamCompareSyncword rdptr: %d, wrptr: %d, syncword:", streamRdPtr, getStreamWrPtr());
    for(i=0; i<4; i++)
    {
        PRINTF(" %02x", streamBuf[rdInx2]);
        rdInx2++;
        rdInx2 -= (rdInx2 >= streamLen ? streamLen : 0);
    }
    PRINTF("\n");
#endif

    if(streamBuf[rdInx] != sync[0])
        return -1;

    rdInx++;
    rdInx -= (rdInx >= streamLen ? streamLen : 0);
    if(streamBuf[rdInx] != sync[1])
        return -1;

    rdInx++;
    rdInx -= (rdInx >= streamLen ? streamLen : 0);
    if((streamBuf[rdInx] & 0xfc) != (sync[2] & 0xfc))
        return -1;

    rdInx++;
    rdInx -= (rdInx >= streamLen ? streamLen : 0);
    if(streamBuf[rdInx] != sync[3])
        return -1;

    return 1;
}

int streamSeekSyncword(int updateStreamRdPtr, int* nRead)
{
    register int rdcnt = 0;
    register int rdInx = 0;
    register int strRdInx = streamRdPtr;
    int result = 0;
    register int srIdx, layer, brIdx;

#if defined(__DEBUG__)
    int rdInx2 = streamRdPtr;
    int i = 0;
    PRINTF(" Report@streamSeekSyncword rdptr: %d, wrptr: %d, syncword:", rdInx2, getStreamWrPtr());
    for(i=0; i<4; i++)
    {
        PRINTF(" %02x", streamBuf[rdInx2]);
        rdInx2++;
        rdInx2 -= (rdInx2 >= streamLen ? streamLen : 0);
    }
    PRINTF("\n");
#endif

    streamWrPtr = getStreamWrPtr();
    if(strRdInx <= streamWrPtr)
    {
        rdcnt = streamWrPtr - strRdInx;
    }
    else
    {
        rdcnt = streamLen - (strRdInx - streamWrPtr);
    }

    PRINTF("streamSeekSyncword(%d, %d){\n", updateStreamRdPtr, *nRead);
    PRINTF("+++ scan stream buffer: WrPtr: %d, RdPtr: %d, count=%d\n", streamWrPtr, strRdInx, rdcnt);

    for( ; rdcnt > 3; rdcnt--, strRdInx++)
    {
        if(strRdInx >= streamLen) strRdInx -= streamLen;
        rdInx = strRdInx;

        // hi part syncword 0xff
        if ( (streamBuf[rdInx] & 0xff) != 0xff )
            continue;

        // low part syncword 0xe0
        rdInx++;
        rdInx -= (rdInx >= streamLen ? streamLen : 0);

        layer = 4 - ((streamBuf[rdInx] >> 1) & 0x03);     // easy mapping of index to layer number, 4 = error
        if (layer == 4)
        {
            continue;
        }
        
        if ( (streamBuf[rdInx] & 0xe0) != 0xe0 )
            continue;

        // Version != 0x1 (reversed) and Layer != 0x0 (reversed)
        if ( (streamBuf[rdInx] & 0x1e) == 0x08 )
            continue;

        // Layer = 0x3 (Layer 1 is acceptable) by powei
        //if ( (streamBuf[rdInx] & 0x06) == 0x06 )
        //    continue;

        rdInx++;
        rdInx -= (rdInx >= streamLen ? streamLen : 0);
        // Bitrate rate index != 0xf0 (bad) && Sample rate index != 0xc (reserve)

        brIdx = (streamBuf[rdInx] >> 4) & 0x0f;
        srIdx = (streamBuf[rdInx] >> 2) & 0x03;
        if (srIdx == 3 || brIdx == 15)
        {
            continue;
        }
        
        if ( (streamBuf[rdInx] & 0xfc) == 0xfc )
            continue;

        result = 1;
        break;
    }

    if(isEOF() && result != 1)
    {
        strRdInx = getStreamWrPtr();
        PRINTF(" streamSeekSyncword(): isEOF\n");
        result = -1;
    }

    PRINTF("    streamRdPtr: %d, strRdInx: %d \n", streamRdPtr, strRdInx);

    *nRead = strRdInx - streamRdPtr;
    *nRead = *nRead < 0 ? *nRead + streamLen : *nRead;

    if(updateStreamRdPtr == 1)
    {
        streamRdPtr = strRdInx;
        MMIO_Write(DrvDecode_RdPtr, streamRdPtr & 0xfffe);
        streamCurPos += *nRead;
    }

    PRINTF("--- scan stream buffer: WrPtr: %d, RdPtr: %d, nRead: %d\n}\n", streamWrPtr, streamRdPtr, *nRead);
    return result;
}
#endif // defined(DUMP_PCM_DATA) || defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)

int streamRead(unsigned char *bufptr, int nbytes)
{
    int rdcnt = 0;

    # if defined(INPUT_MEMMODE)
    rdcnt = min(streamWrPtr - streamRdPtr, nbytes);
    memcpy(bufptr, streamBuf + streamRdPtr, rdcnt);
    streamRdPtr += rdcnt;
    return rdcnt;

    # else // #if defined(INPUT_FILEMODE)

    PRINTF("streamRead(){\n");
    PRINTF("DrvDecode_WrPtr(%d) DrvDecode_RdPtr(%d)\n", streamWrPtr, streamRdPtr);

    while(rdcnt < nbytes)
    {
        int count;
        streamWrPtr = getStreamWrPtr();
        //ASSERT((streamWrPtr & 0x1)==0);

        if(streamRdPtr <= streamWrPtr)
        {
            count = min( (streamWrPtr - streamRdPtr), (nbytes - rdcnt) );
            
        }
        else
        {
            count = min( (streamLen - streamRdPtr), (nbytes - rdcnt) );
        }
        if(count == 0)
        {
            if(isEOF())
            {
                break;
            }
            else
            {
                # if defined(__FREERTOS__)
                    PRINTF("streamRead() assertion!\n");
                    while(1);
                #else
                    ASSERT(0);
                #endif
            }
        }
        else
        {
            #if defined(__OPENRTOS__)
            dc_invalidate();            // Flush Data Cache
            //or32_invalidate_cache(streamBuf + streamRdPtr, count);
            #endif
            memcpy(bufptr, streamBuf + streamRdPtr, count);
            bufptr += count;
            rdcnt += count;

            streamRdPtr += count;
            if(streamRdPtr >= streamLen)
                streamRdPtr -= streamLen;

            //ASSERT((streamRdPtr & 0x1)==0);
            MMIO_Write(DrvDecode_RdPtr, streamRdPtr & 0xfffe);
            streamCurPos += count;
        }
    }

    PRINTF("DrvDecode_WrPtr(%d) DrvDecode_RdPtr(%d)\n}\n", streamWrPtr, streamRdPtr);

    return rdcnt;
    # endif
}

static __inline int decodeQueueFull()
{
    int freebytes;
    pcmReadIdx = CODEC_I2S_GET_OUTRD_PTR();
    PRINTF("decodeQueueFull(){\n");

    freebytes = pcmReadIdx - pcmWriteIdx;
    if (freebytes <= 0)
    {
        freebytes += sizeof(pcmWriteBuf);
    }

    #ifdef MP3_FLOW_CONTROL
    if(mp3DecInfo.samprate >=32000 && mp3DecInfo.samprate<=48000)
    {
        if (freebytes < sizeof(pcmWriteBuf)/3 )
        {  // decode mp3 enough,return 
            return 1;        
        }
    }
    #endif  
    if (isUpSample())
    {
        if (freebytes < MAX_FRAMEBYTES*(GetUpSampleRate(mp3DecInfo.samprate)/mp3DecInfo.samprate) + 2)
        {
            PRINTF("TRUE, ");
            PRINTF("pcmWriteIdx(%d) pcmReadIdx(%d)\n}\n", pcmWriteIdx, pcmReadIdx);
            return 1;
        }
        else 
        {
            PRINTF("FALSE,");
            PRINTF("pcmWriteIdx(%d) pcmReadIdx(%d)\n}\n", pcmWriteIdx, pcmReadIdx);
            return 0;
        }
    }
    else
    {
        if (freebytes < MAX_FRAMEBYTES + 2)
        {
            PRINTF("TRUE, ");
            PRINTF("pcmWriteIdx(%d) pcmReadIdx(%d)\n}\n", pcmWriteIdx, pcmReadIdx);
            return 1;
        }
        else 
        {
            PRINTF("FALSE,");
            PRINTF("pcmWriteIdx(%d) pcmReadIdx(%d)\n}\n", pcmWriteIdx, pcmReadIdx);
            return 0;
        }
    }
}

// output pcm data to I2S interface
// return value: 0 .. not complete due to I2S buffer full
//               1 .. otherwise
int outputpcm(short *pcmbuf, int nch,int nNewCh, int nsamples, int sr,int nsr)
{
    int frameSamps = 0;

    int nTemp,timeout;
    
    if (sr != nsr  && nsr>0 && nsr<=48000) //change sampling rate
    {
        sr = nsr ;   
        bInitDAC = 0;   
    }
    if (nch != nNewCh && nNewCh>0 && nNewCh<=2) //change ch
    {
        nch = nNewCh ;   
        bInitDAC = 0;   
    }

    //  check nSamples
    if (nsamples>5000 || nsamples<=0)
    {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
        ithMp3Printf("[Mp3] error frame nsamples %d \n",nsamples);
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
        printf("[Mp3] error frame nsamples %d \n",nsamples);
#endif
        return 1;
    }
    if (bInitDAC == 0 && sr>0 )
    {
        #if defined(API_V2)
            shareinfo.sampleRate = sr;
            shareinfo.nChanels   = nch;
        #endif // defined(API_V2)
        pcmReadIdx = 0;
        pcmWriteIdx = 0;
        bInitDAC = 1;
        SetFrameNo(0);
        gnMp3DropTime = 0;
        {
            unsigned long long n;
            // mp3 FrameStep is 1.31 format.
            if (isUpSample())             
            {
                n = ((unsigned long long)nsamples*(GetUpSampleRate(sr)/sr)) << 31;
                mp3FrameStep = (unsigned int)(n / GetUpSampleRate(sr));
            }
            else 
            {
                n = ((unsigned long long)nsamples) << 31;
                mp3FrameStep = (unsigned int)(n / sr);
            }
        }
        #if defined(ENABLE_SBC)
            sbc_init(&sbc, SBC_NULL);
            sbc.rate     = sr;
            sbc.channels = nch;
            sbc.subbands = 8;
            sbc.joint    = 0;
            sbc.blocks   = 16;
            sbc.bitpool  = 32;
        #elif !defined(ENABLE_MIXER)
        #  if (defined(WIN32) || defined(__CYGWIN__)) || \
              (!defined(DUMP_PCM_DATA) && !defined(OUTPUT_MEMMODE))
            if (isUpSample())              
            {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
                gCh = nch;
                gSampleRate = GetUpSampleRate(sr);
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC);
#else
                initDAC(pcmWriteBuf, nch, GetUpSampleRate(sr), sizeof(pcmWriteBuf), 0);
#endif
                //printf("[mp3decode] upsampling nch=%d, sr=%d\n", nch, GetUpSampleRate(sr));        
            }
            else
            {       
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)            
                if (gInitOutput){
                    ithMp3Printf("[Mp3] init gInitOutput %d \n",gInitOutput);
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
                    
                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_DAC);
                } else {
                    gInitOutput++;
                }

                gCh = nch;
                gSampleRate = sr;
                AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC);
#else
                initDAC(pcmWriteBuf, nch, sr, sizeof(pcmWriteBuf), 0);
#endif
                //printf("[mp3decode] nch=%d, sr=%d\n", nch, sr);        
            }
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(MP3_DUMP_PCM)
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_FILE_OPEN);
#endif // defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(MP3_DUMP_PCM)
            
        #  endif // !defined(DUMP_PCM_DATA) && !defined(OUTPUT_MEMMODE)
        #else
        mix_on = 0;
        targetSampRate = sr;
        targetChans    = 2;
        init_mixqueue(&mixque, (HWORD*)pcmoutbuf,
                      sizeof(pcmoutbuf)/sizeof(HWORD),
                      targetSampRate);
        init_pcmqueue(&rsinfo0, &pcminque0,
                      (HWORD*)pcmWriteBuf, sizeof(pcmWriteBuf)/sizeof(HWORD),
                      sampleinbuf0, sizeof(sampleinbuf0)/sizeof(HWORD),
                      sampleoutbuf0, sizeof(sampleoutbuf0)/sizeof(HWORD),
                      nch,
                      sr,
                      targetSampRate);
        #  if !defined(DUMP_PCM_DATA) && !defined(OUTPUT_MEMMODE)
        initDAC((unsigned char*)pcmoutbuf, targetChans,
                targetSampRate, sizeof(pcmoutbuf), 0);
        #  endif // !defined(DUMP_PCM_DATA) && !defined(OUTPUT_MEMMODE)
        #endif // defined(ENABLE_SBC)
    }

    if (decodeQueueFull()) 
    {
        #if defined(ENABLE_MIXER)
        mixpcm(0);
        #endif
        #if defined(OUTPUT_MEMMODE)
        exit(-1);
        #else
        return 0;
        #endif
    }

    #if defined(FREQINFO)
    if (isMusicShowSpectrum()){
        if (nFrames%FREQINFOARRAY ==0){
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_SPECTRUM);
        }
    }
    #endif


    gnMp3DropTime = MMIO_Read(AUDIO_DECODER_DROP_DATA);
    if (gnMp3DropTime>0)
    {
        if (gnMp3DropTime)
        {
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
            ithMp3Printf("[Mp3]drop data \n");
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
            printf("[Mp3]drop data \n");
#endif
            return 1;
        }
    }
    else if (gnMp3DropTime==0)
    {
        if (gnMp3DropTime>0)
        {
            GetTime(&gnMp3DropTimeStart);
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
            ithMp3Printf("[Mp3]drop data start %d %d\n",gnMp3DropTimeStart,gnMp3DropTime);
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
            printf("[Mp3]drop data start %d %d\n",gnMp3DropTimeStart,gnMp3DropTime);
#endif         
            return 1;
        }
    }

    if (nsamples > 0)
    {
        frameSamps = nch * nsamples;
        #if defined(LITTLE_ENDIAN_PCM) && defined(DUMP_PCM_DATA)
        // Byte swap to little endian
        {
            int i;
            char *buf = (char *)&pcmWriteBuf[pcmWriteIdx];
            char *in  = (char *)pcmbuf;
            for(i=0; i<frameSamps*sizeof(short); i+=sizeof(short))
            {
                buf[i]   = in[i+1];
                buf[i+1] = in[i];
            }
            memcpy(pcmWriteBuf + pcmWriteIdx, pcmbuf, frameSamps*sizeof(short));            
        }
        #else
        {       
            if (isUpSample())            
            {
                UpSampling(sr, GetUpSampleRate(sr),frameSamps,nch,pcmbuf,tUpSample,16);                
                frameSamps = frameSamps* (GetUpSampleRate(sr)/sr);
                if (GetUpSampleRate(sr)==sr)
                {
                #ifdef MP3_NO_OUTPUT_TEST_PERFORMANCE
                    return 1;
                #else
                    memcpy(pcmWriteBuf + pcmWriteIdx, pcmbuf, frameSamps*sizeof(short));                    
                #endif
                }
                else
                {
                #ifdef MP3_NO_OUTPUT_TEST_PERFORMANCE
                    return 1;
                #else                
                    memcpy(pcmWriteBuf + pcmWriteIdx, tUpSample, frameSamps*sizeof(short));                   
                #endif                
                }
            }
            else
            {          
            #ifdef MP3_NO_OUTPUT_TEST_PERFORMANCE
                return 1;
            #else
                if (isEnableOutputEmpty() ){
                    // output empty
                } else {
                    memcpy(pcmWriteBuf + pcmWriteIdx, pcmbuf, frameSamps*sizeof(short));
                }
            #endif
            }
        }

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(MP3_DUMP_PCM)
        gnTemp = frameSamps*sizeof(short);
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_FILE_WRITE);
#endif // defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(MP3_DUMP_PCM)
        
//  Castor2 using message queue        
//            #if defined(MP3_DUMP_PCM) && defined(__FREERTOS__)
//            {
//                int i;
//                char tmp;
//                char *buf = (char *)&pcmWriteBuf[pcmWriteIdx];
//                int nTemp;
//                for(i=0; i<frameSamps*2; i+=2)
//                {
//                    tmp = buf[i];
//                    buf[i] = buf[i+1];
//                    buf[i+1] = tmp;
//                }            
//                if ( tMp3ToPcmFileOutput )
//                {
//                    nTemp = PalFileWrite(&pcmWriteBuf[pcmWriteIdx], sizeof(char), frameSamps*2, tMp3ToPcmFileOutput, MMP_NULL);
//                    if (nTemp != (frameSamps*2) )
//                    {
//                        printf("[Mp3] : PCM PalFileWrite FAIL! file_length = %d, aacFrameInfo.outputSamps*sizeof(short)  = %d\n", nTemp, frameSamps*2 );
//                    }
//                    else
//                    {
//                        printf("[Mp3]  : PCM PalFileWrite SUCCESS! file_length = %d, frame = %d\n", nTemp, nFrames);
//                    }
//                }
//            }
//            #endif //defined(MP3_DUMP_PCM) && defined(__FREERTOS__)        
        #endif

        pcmWriteIdx += (frameSamps * sizeof(short));
        if(pcmWriteIdx >= sizeof(pcmWriteBuf)) 
        {
            pcmWriteIdx = 0;
        }
    #if defined(ENABLE_SBC)
        // Update the write index of SBC input buffer
    #elif !defined(ENABLE_MIXER)
        // protect writing  register
        MMIO_Write(AUDIO_PROCESSOR_WRITE_REGISTER_PROTECTION, 1);
        //SetOutWrPtr(pcmWriteIdx);
         
        //SetOutWrPtrLo(pcmWriteIdx);
        //SetOutWrPtrHi(0);
        CODEC_I2S_SET_OUTWR_PTR(pcmWriteIdx);
    #else
        pcminque0.iobuf.wrptr = pcmWriteIdx / sizeof(short);
    #endif
    }


    return 1;
}

int  UpSampling(int nInputSampleRate,int nOutputSampleRate,int nSamples,int nChannels,short *pInputPcmbuf,short *pOutputPcmBuf,int nOutputBits)
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
int GetUpSampleRate(int nInputSampleRate)
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
        //printf("[Mp3] unknown sample rate %d \n",nInputSampleRate);
        return 1;
    }                

}


#if defined(ENABLE_MIXER)

int mixpcm(int pcm0eof)
{
    if(!bInitDAC)
        return 0;

    pcminque0.enable = !pcm0eof;

    mix_enable = isEnableMix();
    if (mix_enable == 1 && mix_on == 0)
    {
        mix_on = 1;

        // Temporary solution. The real parameter should get from driver.
        #if defined(__OR32__)
        pcm1nchans = 2;
        pcm1samps  = 22050;
        #endif // defined(__OR32__)

        init_pcmqueue(&rsinfo1, &pcminque1,
                      pcminbuf, sizeof(pcminbuf)/sizeof(HWORD),
                      sampleinbuf1, sizeof(sampleinbuf1)/sizeof(HWORD),
                      sampleoutbuf1, sizeof(sampleoutbuf1)/sizeof(HWORD),
                      pcm1nchans, pcm1samps, targetSampRate);
        pcm1eof = 0;
    }

    if(mix_enable == 1)
    {
        mixPcmReadIdx = pcminque1.iobuf.rdptr*2;
        SetMixRdPtr(mixPcmReadIdx);
        if (isMixEOF()) {
            pcminque1.enable_resample = 0;
            pcminque1.enable = 0;
            pcm1eof = 1;
        }
        mixPcmWriteIdx = GetMixWrPtr();
        pcminque1.iobuf.wrptr = mixPcmWriteIdx/2;
    }
    else if(mix_enable == 0 && mix_on == 1)
    {
        if (isMixEOF()) {
            pcminque1.enable_resample = 0;
            pcminque1.enable = 0;
            pcm1eof = 1;
            mix_on = 0;
        }
    }
    PRINTF("PCM Mix: WrPtr(%d), RdPtr(%d)\n",  pcminque1.iobuf.wrptr,  pcminque1.iobuf.rdptr);

    mixer(&pcminque0, &pcminque1, &mixque, &rsinfo0, &rsinfo1);

    SetOutWrPtr(mixque.iobuf.wrptr * sizeof(short));
    mixque.iobuf.rdptr = GetOutRdPtr() / sizeof(short);

    if(pcm0eof && pcm1eof &&
        (pcminque0.iobuf.rdptr == pcminque0.iobuf.wrptr) &&
        (pcminque1.iobuf.rdptr == pcminque1.iobuf.wrptr) &&
        (mixque.iobuf.rdptr == mixque.iobuf.wrptr) )
        return 1;
    else
        return 0;
}

#endif  // defined(ENABLE_MIXER)

void SetFrameNo(int frameno)
{
    #if defined(ENABLE_SET_FRAME)
        #if !defined(API_V2)
            MMIO_Write(DrvDecode_Frame  , frameno & 0xffff);
            MMIO_Write(DrvDecode_Frame+2, frameno >> 16);
        #else
            shareinfo.decodeTime = frameno;
        #endif // !defined(API_V2)
    #else
        #if 0
        decTime = frameno * ((mp3DecInfo.nFrameSamps << 16) / mp3DecInfo.samprate);
        #else

        #if defined(USE_PTS_EXTENSION)
        int pts_upd   = 0;
        int pts_wridx = MMIO_Read(MMIO_PTS_WRIDX);
        int pts_hi    = MMIO_Read(MMIO_PTS_HI);
        int pts_lo    = ((int)MMIO_Read(MMIO_PTS_LO))&0xffff;
        int cur_rdptr = MMIO_Read(DrvDecode_RdPtr);
        #endif // USE_PTS_EXTENSION

        if (frameno == 0) 
        {
            mp3FrameAccu = 0;
            gLastPtsOverFlowSection = MP3_INVALID_PTS_OVER_FLOW_VALUE; 
        }
        mp3FrameAccu += (mp3FrameStep & 0x7fff);
        decTime = decTime + (mp3FrameStep >> 15) + (mp3FrameAccu >> 15);
        mp3FrameAccu = mp3FrameAccu & 0x7fff;

//        Castor2 using message queue
//        #if defined(MP3_DUMP_PCM) && defined(__FREERTOS__)
//            if (!tMp3ToPcmFileOutput && frameno == 0)
//            {
//                tMp3ToPcmFileOutput = PalWFileOpen( tMp3DumpWave, PAL_FILE_WB, MMP_NULL);
//                if (tMp3ToPcmFileOutput == NULL)
//                {
//                   printf("[Mp3] : PCM PalFileOpen FAIL\n");
//                }
//                else
//                {
//                    printf("[Mp3] : PCM PalFileOpen SUCCESS\n");
//                }
//            }
//        #endif
//        #if defined(MP3_DUMP_PCM) && defined(__FREERTOS__)
//            if( frameno == 500 )
//            {
//                if (tMp3ToPcmFileOutput)
//                {
//                    PalFileClose( tMp3ToPcmFileOutput, MMP_NULL);
//                    tMp3ToPcmFileOutput = NULL;
//                    printf("[Mp3] : PCM PalFileClose Done\n");
//                }
//            }
//        #endif

        #if defined(USE_PTS_EXTENSION)
        if (pts_wridx || pts_hi || pts_lo) {      // Get the PTS info
            if (cur_rdptr == 0 && last_rdptr == 0) { // In guard region, ignore
                MMIO_Write(MMIO_PTS_WRIDX, 0);       //   the PTS
                MMIO_Write(MMIO_PTS_HI, 0);
                MMIO_Write(MMIO_PTS_LO, 0);
            } else if (cur_rdptr > last_rdptr) {
                if ((cur_rdptr >= pts_wridx && last_rdptr <= pts_wridx) || (cur_rdptr == pts_wridx))
                    pts_upd = 1;
            } else if(cur_rdptr < last_rdptr) {
                if (cur_rdptr >= pts_wridx || last_rdptr <= pts_wridx)
                    pts_upd = 1;
            }
        }

#if 1
        if (pts_upd) 
        {
            MMIO_Write(MMIO_PTS_WRIDX, 0);
            MMIO_Write(MMIO_PTS_HI, 0);
            MMIO_Write(MMIO_PTS_LO, 0);
            unsigned int pts = (((pts_hi % MP3_HI_BIT_OVER_FLOW_THRESHOLD) << 16) + pts_lo);
            unsigned int time = ((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16);
            unsigned int timeGap = 0;
            unsigned int ptsOverFlowSection = pts_hi / MP3_HI_BIT_OVER_FLOW_THRESHOLD;
            unsigned int bPtsGreaterThanDec = 0;

            PRINTF("#%d, decTime: 0x%08X, time: %u, pts: %u, full_pts: %u\n",
                   frameno,
                   decTime,
                   time,
                   pts,
                   ((pts_hi << 16) + pts_lo));

            if (MP3_INVALID_PTS_OVER_FLOW_VALUE == gLastPtsOverFlowSection)
                gLastPtsOverFlowSection = ptsOverFlowSection;

            if (ptsOverFlowSection < gLastPtsOverFlowSection) // hit bit decrement(possible??)
            {
                timeGap = (MP3_WRAP_AROUND_THRESHOLD - pts) + time;
            }
            else
            {
                // hi bit increment. the accuray is only 16 bit ms. more is ignored due to
                // the gap is too huge and not catchable.

                // pts is wrapped around but decode time not.
                if (ptsOverFlowSection > gLastPtsOverFlowSection
                 && (pts + time) >= MP3_JUDGE_MAXIMUM_GAP)
                {
                    timeGap = pts + (MP3_WRAP_AROUND_THRESHOLD - time);
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
                ithMp3Printf("#%d, decTime: 0x%08X, time: %u, pts: %u, full_pts: %u, bPtsGreaterThanDec: %d, time_gap: %u\n",
                       frameno,
                       decTime,
                       time,
                       pts,
                       ((pts_hi << 16) + pts_lo),
                       bPtsGreaterThanDec,
                       timeGap);

            	AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                printf("#%d, decTime: 0x%08X, time: %u, pts: %u, full_pts: %u, bPtsGreaterThanDec: %d, time_gap: %u\n",
                       frameno,
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
                    ithMp3Printf("(%d) #PTS: Adjust +%d ms (%d->%d) %u\n", __LINE__, timeGap,
                           time,
                           (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                           ((pts_hi << 16) + pts_lo));

                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                    printf("(%d) #PTS: Adjust +%d ms (%d->%d) %u\n", __LINE__, timeGap,
                           time,
                           (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                           ((pts_hi << 16) + pts_lo));
#endif            
                    
                }
                else
                {
                    decTime = decTime - ((((timeGap & 0xFFFF0000) / 1000) << 16) + (((timeGap & 0xFFFF) << 16) / 1000));
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)        
                    ithMp3Printf("(%d) #PTS: Adjust -%d ms (%d->%d) %u\n", __LINE__, timeGap, time,
                           (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                           ((pts_hi << 16) + pts_lo));

                    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);        
#else
                    printf("(%d) #PTS: Adjust -%d ms (%d->%d) %u\n", __LINE__, timeGap, time,
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
                   frameno,
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
                   frameno,
                   decTime,
                   time,
                   cur_rdptr,
                   last_rdptr,
                   pts_wridx,
                   pts);                   

            if ((int) (pts - time) >= 100) {
                decTime = decTime + (((pts - time - 1) << 16) /  1000);
                printf("#PTS: Adjust +%d ms (%d->%d) %d\n", pts-time,
                       time,
                       (((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16)),
                       pts);
            } else if ((int) (time - pts) >= 100) {
                decTime = decTime - (((time - pts - 1) << 16) /  1000);
                printf("#PTS: Adjust -%d ms (%d->%d) %d\n", time-pts, time,
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
                   frameno,
                   decTime,
                   time,
                   cur_rdptr,
                   last_rdptr);
            
        }
#endif
        last_rdptr = MMIO_Read(DrvDecode_RdPtr);
        
        #endif // USE_PTS_EXTENSION

        #endif
        #if !defined(API_V2)
        MMIO_Write(DrvDecode_Frame  , (short)( ((unsigned int)decTime) & 0xffff));
        MMIO_Write(DrvDecode_Frame+2, (short)( ((unsigned int)decTime) >> 16)   );
        #else
        shareinfo.decodeTime = decTime;
        #endif // !defined(API_V2)
    #endif
}

void GetFrameNo(int *frameno)
{
#if !defined(WIN32) && !defined(__CYGWIN__)
    #if defined(ENABLE_SET_FRAME)
        #if !defined(API_V2)
            *frameno = (MMIO_Read(DrvDecode_Frame+2) << 16) | MMIO_Read(DrvDecode_Frame);
        #else
            *frameno = shareinfo.decodeTime;
        #endif
    #else
        int time;
        #if !defined(API_V2)
            time = (MMIO_Read(DrvDecode_Frame+2) << 16) | MMIO_Read(DrvDecode_Frame);
        #else
            time = shareinfo.decodeTime;
        #endif
        *frameno = time  / ((mp3DecInfo.nFrameSamps << 16) / mp3DecInfo.samprate);
    #endif
#endif // !defined(API_V2)
}

void GetTime(unsigned int *pTime)
{
    *pTime  = ((decTime >> 16) & 0xffff) * 1000 + (((decTime & 0xffff) * 1000) >> 16);
}

void ResetRdBufferPointer()
{

    streamWrPtr = 0;
    streamRdPtr = 0;
    MMIO_Write(DrvDecode_WrPtr, 0);
    MMIO_Write(DrvDecode_RdPtr, 0);

    memset(streamBuf,0,sizeof(streamBuf));
    memset(pcmWriteBuf,0,sizeof(pcmWriteBuf));

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

void ClearRdBuffer(void) {

    #if defined(OUTPUT_MEMMODE)
    return ;
    #endif

    memset(mp3DecInfo.IMDCTInfoPS.prevblck, 0, sizeof(mp3DecInfo.IMDCTInfoPS.prevblck));

    if (isEOF() && !isSTOP()) {
        #if defined(DUMP_PCM_DATA)
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) | DrvDecodePCM_EOF);
        #endif // defined(DUMP_PCM_DATA)

        // wait I2S buffer empty
        do {
            if (1)// (GetOutRdPtr() == GetOutWrPtr()) 
            {
                break;
            } else {
                #if defined(__FREERTOS__)
                taskYIELD();
                #else
                //or32_delay(1); // enter sleep mode for power saving
                #endif
            }
        } while(1);

        #if defined(DUMP_PCM_DATA)
        while(MMIO_Read(DrvAudioCtrl) & DrvDecodePCM_EOF) {
            #if defined(__FREERTOS__)
            taskYIELD();
            #else
            //or32_delay(1);
            #endif
        }
        #endif // defined(DUMP_PCM_DATA)
    }

    SetFrameNo(0);
    decTime = 0;
    MMIO_Write(DrvDecode_WrPtr, 0);
    MMIO_Write(DrvDecode_RdPtr, 0);

    #if defined(USE_PTS_EXTENSION)
    last_rdptr = 0;
    MMIO_Write(MMIO_PTS_WRIDX, 0);
    MMIO_Write(MMIO_PTS_HI, 0);
    MMIO_Write(MMIO_PTS_LO, 0);
    MMIO_Write(DrvDecode_Frame  , 0);
    MMIO_Write(DrvDecode_Frame+2, 0);
    #endif // USE_PTS_EXTENSION

    streamWrPtr = 0;
    streamRdPtr = 0;

    PRINTF("ClearRdBuffer()\n");
    PRINTF("DrvDecode_WrPtr(%d) DrvDecode_RdPtr(%d)\n", streamWrPtr, streamRdPtr);

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

    #if defined(ENABLE_SBC)
    sbc_finish(&sbc);
    #endif // defined(ENABLE_SBC)


    #if defined(__OPENRTOS__)
    dc_invalidate(); // Flush DC Cache
    #endif

    streamRdPtr = 0;
    streamWrPtr = 0;
    bInitDAC = 0;

    gInitOutput = 0;
//    #if defined(MP3_DUMP_PCM) && defined(__FREERTOS__)
//        if (tMp3ToPcmFileOutput)
//        {
//            PalFileClose( tMp3ToPcmFileOutput, MMP_NULL);
//            tMp3ToPcmFileOutput = NULL;
//            printf("[Mp3] : PCM PalFileClose Done\n");
//        }
//    #endif
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(MP3_DUMP_PCM)
    AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_FILE_CLOSE);
#endif //defined(AUDIO_PLUGIN_MESSAGE_QUEUE) && defined(MP3_DUMP_PCM)

#if defined(DUMP_FRAME_CHECKSUM) && defined(WIN32)
    {
        #define XCHG_LONG(x) ( ((((x)&0xFF)<<24) | (((x)&0xFF00)<<8) | \
                            (((x)&0xFF0000)>>8) | (((x)>>24)&0xFF)) )
        int i;
        FILE *file;

        file = fopen("checksum.hex", "w");
        for (i=0; i<g_checksum_cnt; i++) {
            fprintf(file, "0x%.4X,", g_checksum_array[i]);
            if ((i&0x7) == 0x7){
                fprintf(file, "\n");
            }
        }
        fprintf(file, "\n");
        fclose(file);

        file = fopen("checksum.bin", "wb");
        for (i=0; i<g_checksum_cnt; i++) {
            // LEtoBE
            g_checksum_array[i] = XCHG_LONG(g_checksum_array[i]);
        }
        fwrite(g_checksum_array, sizeof(short), g_checksum_cnt, file);
        fclose(file);

        file = fopen("region.bin", "wb");
        for (i=0; i<g_checksum_cnt*2; i++) {
            // LEtoBE
            g_checksum_region_array[i] = XCHG_LONG(g_checksum_region_array[i]);
        }
        fwrite(g_checksum_region_array, sizeof(int), g_checksum_cnt*2, file);
        fclose(file);
    }
#endif
}

#if defined(DUMP_FRAME_CHECKSUM)
void SetChksumRegionStart(unsigned char *baseptr, int startidx)
{
    checksum_startptr = baseptr + startidx;
    g_checksum_region_array[g_checksum_cnt*2] = startidx;
}
void SetChksumRegionEnd(unsigned char *baseptr, int endidx)
{
    unsigned short checksum = 0;
    unsigned char *ptr = checksum_startptr;

    checksum_endptr = baseptr + endidx;
    g_checksum_region_array[g_checksum_cnt*2+1] = endidx;

    while(ptr != checksum_endptr){
        checksum += (unsigned short)(*ptr);
        ptr++;
    }
    # if defined(COMPARE_CHECKSUM)
    if (checksum != compared_chksum[g_checksum_cnt]) {
        while(1);
    }
    # endif
    g_checksum_array[g_checksum_cnt++] = checksum;
}
#endif
