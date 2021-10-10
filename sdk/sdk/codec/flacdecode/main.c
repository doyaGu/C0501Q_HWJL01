/////////////////////////////////////////////////////////////////
//                      Include File
/////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "config.h"
#include "flac.h"

#if defined(ENABLE_CODECS_PLUGIN)
# include "plugin.h"
#endif

#if defined(__OR32__)
#include "i2s.h"
#include "mmio.h"
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

/////////////////////////////////////////////////////////////////
//                      Local Variable
/////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
//                      Global Variable
/////////////////////////////////////////////////////////////////

FLACContext gFLACContext;

/* Decode status */
static int lastRound;
static int eofReached;

static unsigned int  pcmReadIdx;
static unsigned int  pcmWriteIdx;

static unsigned int decTime = 0; // Decoding time in seconds on S15.16 fixed point.
static unsigned int flacFrameStep;
static unsigned int flacFrameAccu;
static int gnWaitSize = READBUF_SIZE/2;

#if defined(WIN32) || defined(__CYGWIN__)
FILE *fmp3 = NULL;
char *mp3save = "D:\\audio_testing\\Mp3\\MPA.mp3";
FILE *fwav = NULL;
char *wavSrc = "D:\\audio_testing\\FLAC\\bird44100FLAC.wav";
char gStream[8*1024*1024];
int gStreamRead=0;
#endif

char streamBuf[READBUF_SIZE+2] __attribute__ ((aligned(16))); // FLAC input buffer
char gOutBuf[OUTBUF_SIZE] __attribute__ ((aligned(16))); // decode pcm buffer 


unsigned char gTempFlac[FLAC_AVG_FRAME_SIZE];  // temp buffer for FLAC data
short gFrame[32*1024];

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
static unsigned int* gtAudioPluginBufferLength;
static unsigned short* gtAudioPluginBuffer;
static int gnTemp;
static unsigned char tDumpWave[] = "C:/flac_dump.wav";
static int gCh;
static int gSampleRate;
int gPause = 0 ;
int gPrint = 0;
unsigned char *gBuf;
#endif

static unsigned int nFrames;

/////////////////////////////////////////////////////////////////
//                      Global Function
/////////////////////////////////////////////////////////////////
#define getUsedLen(rdptr, wrptr, len) (((wrptr) >= (rdptr)) ? ((wrptr) - (rdptr)) : ((len) - ((rdptr) - (wrptr))))

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
static void AudioPluginAPI(int nType);
__inline int ithPrintf(char* control, ...);
#endif

#ifdef ITE_RISC
static int FillReadBuffer(int nReadBytes);

static void FillWriteBuffer(int nPcmBytes);
static void waitAvaliableReadBufferSize(int nWaitSize);

static __inline void updateTime(void)
{
    flacFrameAccu += (flacFrameStep & 0x7fff);
    decTime = decTime + (flacFrameStep >> 15) + (flacFrameAccu >> 15);
    flacFrameAccu = flacFrameAccu & 0x7fff;

    MMIO_Write(DrvDecode_Frame  , (short)( ((unsigned int)decTime) & 0xffff));
    MMIO_Write(DrvDecode_Frame+2, (short)( ((unsigned int)decTime) >> 16)   );
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
            //printf("[FLAC] name length %d \n",gnTemp);
            memcpy(&gtAudioPluginBuffer[0], &gnTemp, SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH);                           
            memcpy(&gtAudioPluginBuffer[SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH/sizeof(short)], tDumpWave, sizeof(tDumpWave));
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_WRITE:
          // printf("[FLAC] name length %d \n",gnTemp);           
            memcpy(&gtAudioPluginBuffer[0], &gnTemp, SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH);                           
            {
                int i;
                char tmp;
                char *buf = (char *)&gOutBuf[pcmWriteIdx];
                for(i=0; i<gnTemp; i+=2)
                {
                    tmp = buf[i];
                    buf[i] = buf[i+1];
                    buf[i+1] = tmp;
                }
            }           
            memcpy(&gtAudioPluginBuffer[SMTK_AUDIO_PLUGIN_CMD_FILE_WRITE_LENGTH/sizeof(short)], &gOutBuf[pcmWriteIdx], gnTemp);           
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_CLOSE:
           
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC:
                nTemp  = gOutBuf;
                nTemp1 = sizeof(gOutBuf);
                pBuf = (unsigned char*)gtAudioPluginBuffer;
                memcpy(&pBuf[0], &nTemp, sizeof(int));
                memcpy(&pBuf[4], &gCh, sizeof(int));
                memcpy(&pBuf[8], &gSampleRate, sizeof(int));
                memcpy(&pBuf[12], &nTemp1, sizeof(int));
                memcpy(&pBuf[16],&gFLACContext.curr_bps,sizeof(int));
                //printf("[FLAC] 0x%x %d %d %d \n",nTemp,gCh,gSampleRate,nTemp1);
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

static __inline unsigned int setStreamRdPtr(unsigned int wrPtr) {

    MMIO_Write(DrvDecode_RdPtr, wrPtr);
    return 0;
}
static __inline unsigned int setStreamWrPtr(unsigned int wrPtr) {

    MMIO_Write(DrvDecode_WrPtr, wrPtr);
    return 0;
}
static __inline unsigned int getStreamWrPtr() {
 
    unsigned int wrPtr;
    wrPtr = MMIO_Read(DrvDecode_WrPtr);

    return wrPtr;
}

__inline unsigned int getStreamRdPtr() {
    unsigned int rdPtr;
    rdPtr = MMIO_Read(DrvDecode_RdPtr);

    #if defined(__OR32__) && !defined(__OPENRTOS__)
    if (0xffff == rdPtr) asm volatile("l.trap 15");
    #endif

    return rdPtr;
}

__inline unsigned int getOutBufRdPtr() {
    unsigned int rdPtr;
    #if defined(__OPENRTOS__)
        rdPtr = CODEC_I2S_GET_OUTRD_PTR();
    #else
        rdPtr = MMIO_Read(DrvEncode_RdPtr);
    #endif
    return rdPtr;
}

__inline unsigned int getOutBufWrPtr() {
    unsigned int wrPtr;
    wrPtr = MMIO_Read(DrvEncode_WrPtr);
    return wrPtr;
}

static waitAvaliableReadBufferSize(int nWaitSize) {
    int len = 0;
    int i=0;
    unsigned int flacReadIdx,flacWriteIdx;
    do {
        flacReadIdx = getStreamRdPtr();
        // Wait Read Buffer avaliable
        flacWriteIdx = getStreamWrPtr();
        len = getUsedLen(flacReadIdx, flacWriteIdx, READBUF_SIZE);
    #if defined(__OPENRTOS__)
        //PalSleep(2);
        for (i=0;i<100;i++);
    #else
        or32_delay(1); // enter sleep mode for power saving
    #endif
    } while (len < nWaitSize && !isSTOP());
}

static int GetAvaliableReadBufferSize() {
    int len = 0;
    unsigned int flacReadIdx,flacWriteIdx;
    flacReadIdx = getStreamRdPtr();
    // Wait Read Buffer avaliable
    flacWriteIdx = getStreamWrPtr();
    len = getUsedLen(flacReadIdx, flacWriteIdx, READBUF_SIZE);
    return len;
}


/**************************************************************************************
 * Function     : FillReadBuffer
 *
 * Description  : Update the read pointer of WAVE Buffer and return the valid data length
 *                of input buffer.
 *
 * Inputs       : nReadBytes: number of bytes will read
 *
 * Global var   : wavWriteIdx: write pointer of WAVE buffer
 *                flacReadIdx : read pointer of WAVE buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The WAVE buffer is circular buffer.
 *
 **************************************************************************************/
static int FillReadBuffer(int nReadBytes) {
    int len = 0;
    unsigned int flacReadIdx,flacWriteIdx;
    flacReadIdx = getStreamRdPtr();
    // Update Read Buffer
    if (nReadBytes > 0)
    {
        flacReadIdx = flacReadIdx + nReadBytes;
        if (flacReadIdx >= READBUF_SIZE) {
            flacReadIdx -= READBUF_SIZE;
        }
        setStreamRdPtr(flacReadIdx);
    }
#if defined(__OPENRTOS__)
    dc_invalidate(); // Flush Data Cache
#endif
    // Wait Read Buffer avaliable
    flacWriteIdx = getStreamWrPtr();
    len = getUsedLen(flacReadIdx, flacWriteIdx, READBUF_SIZE);
    return len;
}

/**************************************************************************************
 * Function     : FillWriteBuffer
 *
 * Description  : Wait the avaliable length of the output buffer bigger than on
 *                frame of audio data.
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
static void FillWriteBuffer(int nPcmBytes) {
    int len;
    int i;
    // Update Write Buffer
    if (nPcmBytes > 0) {
        //nOutWriteIdx = nOutWriteIdx + nPcmBytes;
        if (pcmWriteIdx+nPcmBytes >= OUTBUF_SIZE) {
            memcpy(&gOutBuf[pcmWriteIdx],&gFrame[0],OUTBUF_SIZE-pcmWriteIdx);
            memcpy(&gOutBuf[0],&gFrame[(OUTBUF_SIZE-pcmWriteIdx)>>1],nPcmBytes-(OUTBUF_SIZE-pcmWriteIdx));
            pcmWriteIdx = nPcmBytes-(OUTBUF_SIZE-pcmWriteIdx);
        } else {
            memcpy(&gOutBuf[pcmWriteIdx],&gFrame[0],nPcmBytes);
            pcmWriteIdx+=nPcmBytes;
        }
        #if defined(__OPENRTOS__)
            CODEC_I2S_SET_OUTWR_PTR(pcmWriteIdx);
        #else
            SetOutWrPtr(pcmWriteIdx);
        #endif
        
    }
    updateTime();
   
    // Wait output buffer avaliable
    do
    {
        #if defined(__OPENRTOS__)
            pcmReadIdx = CODEC_I2S_GET_OUTRD_PTR();
        #else
            pcmReadIdx = GetOutRdPtr();
        #endif
    
        if (pcmReadIdx <= pcmWriteIdx) {
            len = OUTBUF_SIZE - (pcmWriteIdx - pcmReadIdx);
        } else {
            len = pcmReadIdx - pcmWriteIdx;
        }

        if (len < FLAC_MAX_FRAME_SIZE && !isSTOP()) {
            #if defined(__OPENRTOS__)
                //PalSleep(2);
                for (i=0;i<100000;i++);
            #else
                or32_delay(1); // enter sleep mode for power saving
            #endif
        } else {
            break;
        }
    } while(1);
    // PRINTF("pcmWriteIdx(%d) pcmReadIdx(%d) len(%d) nPCMBytes(%d)\n", pcmWriteIdx, pcmReadIdx, len, nPCMBytes);
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
            #if defined(__OPENRTOS__)

            #else
            or32_delay(1); // delay 1ms
            #endif
        }
        prePause = curPause;
    } while(!eofReached);
    prePause = curPause;

}
/**************************************************************************************
 * Function     : AdjustBuffer
 *
 * Description  : move the reset of data to the front of buffer.
 *
 * Inputs       : waitNBytes: number of bytes to read
 *
 * Global var   : flacWriteIdx: write pointer of FLAC buffer
 *                flacReadIdx : read pointer of FLAC buffer
 *
 * Return       :
 *
 * TODO         :
 *
 * Note         : The FLAC buffer is circular buffer.
 *
 **************************************************************************************/
static int AdjustBuffer(int waitNBytes) {
    int len;
    unsigned int flacReadIdx,flacWriteIdx;
    int nTemp = 0;

    flacReadIdx = getStreamRdPtr();
    flacWriteIdx = getStreamWrPtr();

    // It should be invalidate the cache line which it is in the
    // begin of input buffer. The previous frame will
    // prefetch the data to the cache line, but the driver dose
    // not yet put it in the input buffer. It will cause the
    // unconsistency of cache.

    if (flacReadIdx + waitNBytes >= READBUF_SIZE) { // do memory move when exceed guard band
        // Update Read Pointer
        MMIO_Write(DrvDecode_RdPtr, (unsigned short)(((flacReadIdx-READBUF_BEGIN)>>1) << 1));

        // Wait the flacWriteIdx get out of the area of rest data.
        do {
            checkControl();
            flacReadIdx = getStreamWrPtr();

            if (flacWriteIdx >= flacReadIdx && !eofReached) {
                #if defined(__OPENRTOS__)
                //taskYIELD();
                #else
                or32_delay(1); // enter sleep mode for power saving
                #endif
            } else {
                break;
            }
        } while(!eofReached);

        if (flacWriteIdx < flacReadIdx) {
            #if defined(__OPENRTOS__)
              dc_invalidate(); // Flush Data Cache
            #endif
            // Move the rest data to the front of data buffer
            memcpy(&streamBuf[READBUF_BEGIN - (READBUF_SIZE - flacReadIdx)],
                   &streamBuf[flacReadIdx], READBUF_SIZE-flacReadIdx);
            flacReadIdx = READBUF_BEGIN - (READBUF_SIZE - flacReadIdx);
        }
    }

    flacWriteIdx = getStreamWrPtr();

    if (flacWriteIdx >= flacReadIdx) {
        len = flacWriteIdx - flacReadIdx;
    } else {
        len = READBUF_LEN - (flacReadIdx - flacWriteIdx);
    }

    return len;
}


void ClearRdBuffer(void) {
    //SetFrameNo(0);
    MMIO_Write(DrvDecode_WrPtr, 0);
    MMIO_Write(DrvDecode_RdPtr, 0);

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    //AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_DAC);
#else
    deactiveDAC();   // Disable I2S interface
#endif   
    if (isSTOP()) {
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvDecode_STOP);
    }
#if !defined(WIN32) && !defined(__CYGWIN__)
    MMIO_Write(DrvDecode_Frame  , 0);
    MMIO_Write(DrvDecode_Frame+2, 0);
#endif // !defined(WIN32) && !defined(__CYGWIN__)


#if defined(__FREERTOS__)|| defined(__OPENRTOS__)
    dc_invalidate(); // Flush DC Cache
#endif
    gnWaitSize = READBUF_SIZE/2;

   
}

#endif // #ifdef ITE_RISC

// Prepare flac data to decode buffer,and return flac frame length
// Flac frame length need parsing 
static int getFLACFrame(){
    FLACFrameInfo tFlacInfo;
    int len = 0;
    unsigned int flacReadIdx,flacWriteIdx;
    int nTemp = 0;
    int frameLength;
    int keep;

    flacFrameAccu = 0;

    // get flac data
    flacReadIdx = getStreamRdPtr();
    flacWriteIdx = getStreamWrPtr();
   
    do {
#if 1
        //waitAvaliableReadBufferSize(MAX_FRAME_HEADER_SIZE);
        waitAvaliableReadBufferSize(gnWaitSize);
        if (MAX_FRAME_HEADER_SIZE+flacReadIdx<READBUF_SIZE) {
            //memcpy(&gTempFlac[0],&streamBuf[flacReadIdx],MAX_FRAME_HEADER_SIZE);
            if (((streamBuf[flacReadIdx]<<8|streamBuf[flacReadIdx+1]) & 0xFFFE) == 0xFFF8) {
                nTemp = frame_header_is_valid(&streamBuf[flacReadIdx],&tFlacInfo);
            }
        } else {
            memcpy(&gTempFlac[0],&streamBuf[flacReadIdx],READBUF_SIZE-flacReadIdx);
            memcpy(&gTempFlac[READBUF_SIZE-flacReadIdx],&streamBuf[0],MAX_FRAME_HEADER_SIZE-(READBUF_SIZE-flacReadIdx));
            if (((gTempFlac[0]<<8|gTempFlac[1]) & 0xFFFE) == 0xFFF8) {
                nTemp = frame_header_is_valid(&gTempFlac[0],&tFlacInfo);
            }

        }
#else
        if (((streamBuf[flacReadIdx]<<8|streamBuf[flacReadIdx+1]) & 0xFFFE) == 0xFFF8) {
            nTemp = frame_header_is_valid(&streamBuf[flacReadIdx],&tFlacInfo);
        }
#endif
        flacWriteIdx = getStreamWrPtr();
        len = getUsedLen(flacReadIdx, flacWriteIdx, READBUF_SIZE);
        #if defined(__OPENRTOS__)
            //PalSleep(2);
        #else
            or32_delay(1); // enter sleep mode for power saving
        #endif
        flacReadIdx++;
        if (flacReadIdx>=READBUF_SIZE)
            flacReadIdx = 0;
    } while(nTemp==0 && !isSTOP());
    if (flacReadIdx)    
        flacReadIdx-=1;
    
    keep = flacReadIdx; 
    setStreamRdPtr(flacReadIdx);
    flacReadIdx++;
    nTemp=0;
   
    // get flac frame length
    do {
#if 1
        waitAvaliableReadBufferSize(MAX_FRAME_HEADER_SIZE);
        if (MAX_FRAME_HEADER_SIZE+flacReadIdx<READBUF_SIZE) {
            //memcpy(&gTempFlac[0],&streamBuf[flacReadIdx],MAX_FRAME_HEADER_SIZE);
            if (((streamBuf[flacReadIdx]<<8|streamBuf[flacReadIdx+1]) & 0xFFFE) == 0xFFF8) {
                nTemp = frame_header_is_valid(&streamBuf[flacReadIdx],&tFlacInfo);
            }
        } else {
            memcpy(&gTempFlac[0],&streamBuf[flacReadIdx],READBUF_SIZE-flacReadIdx);
            memcpy(&gTempFlac[READBUF_SIZE-flacReadIdx],&streamBuf[0],MAX_FRAME_HEADER_SIZE-(READBUF_SIZE-flacReadIdx));
            if (((gTempFlac[0]<<8|gTempFlac[1]) & 0xFFFE) == 0xFFF8) {
                nTemp = frame_header_is_valid(&gTempFlac[0],&tFlacInfo);
            }

        }
#else
        if (((streamBuf[flacReadIdx]<<8|streamBuf[flacReadIdx+1]) & 0xFFFE) == 0xFFF8) {
            nTemp = frame_header_is_valid(&streamBuf[flacReadIdx],&tFlacInfo);
        }
#endif
        flacWriteIdx = getStreamWrPtr();
        len = getUsedLen(flacReadIdx, flacWriteIdx, READBUF_SIZE);
        #if defined(__OPENRTOS__)
            //PalSleep(2);
        #else
            or32_delay(1); // enter sleep mode for power saving
        #endif
        flacReadIdx++;
        if (flacReadIdx>=READBUF_SIZE)
            flacReadIdx = 0;
    } while(nTemp==0 && !isSTOP());
    if (flacReadIdx)
    flacReadIdx--;
    if (flacReadIdx>=keep)
        frameLength = flacReadIdx - keep;
    else 
        frameLength = READBUF_SIZE-keep+flacReadIdx;

    if (frameLength<=(READBUF_SIZE/2)){
        gnWaitSize = frameLength;
    } else {
        gnWaitSize = READBUF_SIZE/2;
    }

    // store flac data to decode buffer
    flacReadIdx = getStreamRdPtr();
    if (frameLength+flacReadIdx<READBUF_SIZE) {
        memcpy(&gTempFlac[0],&streamBuf[flacReadIdx],frameLength);
    } else {
        memcpy(&gTempFlac[0],&streamBuf[flacReadIdx],READBUF_SIZE-flacReadIdx);
        memcpy(&gTempFlac[READBUF_SIZE-flacReadIdx],&streamBuf[0],frameLength-(READBUF_SIZE-flacReadIdx));
    }
    gFLACContext.channels = tFlacInfo.channels;
    gFLACContext.samplerate= tFlacInfo.samplerate;
   
    return frameLength;

}


# if defined(__OPENRTOS__) && !defined(ENABLE_CODECS_PLUGIN)
portTASK_FUNCTION(flushdecode_task, params)
# else
int main(int argc, char **argv)
# endif
{
    int bytesLeft;
    int nTemp;
    int len = 0;
    unsigned int flacReadIdx;
    int init = 0;
    int frames = 0;
    int* codecStream;
    AUDIO_CODEC_STREAM* audioStream;
    
#ifdef FLAC_PERFORMANCE_TEST_BY_TICK
    int  nNewTicks,nTotalTicks;
#endif
    
#if defined(WIN32) || defined(__CYGWIN__)
    //param_struct param;
    char *fileName[]=
	{
	 "FlacDecode",
	 "-ao",
        //"D:\\audio_testing\\Mp3\\bird_48000.wav",
        //"D:\\audio_testing\\WAV\\swf_adpcm.wav",
        //"D:\\audio_testing\\WAV\\ima-adpcm.wav",     
        //"D:\\Castor3_Alpha\\test_data\\dump_data.mp3",     
        "D:\\audio_testing\\FLAC\\1.bird_44100.flac",     
        //"D:\\audio_testing\\Mp3\\BBC7.mp3",
        "D:\\audio_testing\\FLAC\\bird_44100.wav"
	};

    if ((fwav = fopen(wavSrc, "wb")) == NULL) {
        printf("Can not open file '%s'.\n", fwav);
        exit(-1);
    }

    fwrite(&gFrame[0], 1, 44, fwav);

    #if 0
    if ((fwav = fopen(wavSrc, "rb")) == NULL) {
        printf("Can not open file '%s'.\n", fwav);
        exit(-1);
    }
    fseek(fwav, 0, SEEK_END);
    size = ftell(fwav);
    fseek(fwav, 0, SEEK_SET);
    if (size & 0x1) size++;
    if (fread(gStream, sizeof(char), size, fwav) != (unsigned)size) {
        printf("Data read fails.\n");
        exit(-1);
    }
    fclose(fwav);
    if ((fmp3=fopen(mp3save, "wb")) == NULL) {
        printf("Can not create file '%s'\n", mp3save);
        exit(-1);
    }  
    #endif

    argc+=3;
    argv = fileName;
    GetParam(argc, argv);
    win32_init();
#endif // defined(WIN32) || defined(__CYGWIN__)
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    codecStream=CODEC_STREAM_START_ADDRESS;   
    //printf("[FLAC] 0x%08x \n",*codecStream);
    audioStream = *codecStream;
    audioStream->codecStreamBuf = &streamBuf[READBUF_BEGIN];
    audioStream->codecStreamLength = READBUF_LEN;   
    //printf("[FLAC] audioStream 0x%08x 0x%08x 0x%08x   \n",&audioStream,&audioStream->codecStreamBuf,&audioStream->codecStreamLength);
    gtAudioPluginBuffer = audioStream->codecAudioPluginBuf;
    gtAudioPluginBufferLength = audioStream->codecAudioPluginLength;
    //printf("[E-AC3] 0x%08x %d 0x%08x %d \n",audioStream->codecStreamBuf,audioStream->codecStreamLength ,audioStream->codecAudioPluginBuf,audioStream->codecAudioPluginLength);
    MMIO_Write(AUDIO_DECODER_START_FALG, 1); 
    #if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)    
        ithPrintf("[FLAC] %d %d start 0x%x 0x%x \n",flacReadIdx,READBUF_LEN,streamBuf[0],streamBuf[0+1]);
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
    #else
        
    #endif
    
#endif

#ifdef FLAC_PERFORMANCE_TEST_BY_TICK
        nTotalTicks=0;
        nFrames = 0;
#endif //FLAC_PERFORMANCE_TEST_BY_TICK

    pcmWriteIdx = 0;
    pcmReadIdx  = 0;


    eofReached = 0;
    setStreamRdPtr(0);
    memset(&gFLACContext,0,sizeof(gFLACContext));
    for(;;) // forever loop
    {
        int exitflag1 = 0;
        int frameSize;

        // Updates the read buffer and returns the avaliable size
        // of input buffer. Wait a minimun FRAME_SIZE length.
        bytesLeft = GetAvaliableReadBufferSize();
        flacReadIdx = getStreamRdPtr();
        nTemp = getFLACFrame();        
#ifdef FLAC_PERFORMANCE_TEST_BY_TICK
        start_timer();
#endif           
        
        frameSize = flac_decode_frame(&gFLACContext,&gTempFlac[0],nTemp,&gFrame[0]);
    #if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
        if (frameSize<0) { 
            ithPrintf("[FLAC] dec error %d \n",frameSize);
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
        }
    #else
        
    #endif
        
#ifdef FLAC_PERFORMANCE_TEST_BY_TICK
        nNewTicks = get_timer();            
        nTotalTicks += nNewTicks;
        if (nFrames % 100 == 0 && nFrames>0)
        {
            ithPrintf("[FLAC] (%d~%d) total %d (ms) average %d (ms) nFrames %d system clock %d  bps %d \n",(nFrames+1-100),(nFrames+1),(nTotalTicks/(PalGetSysClock()/1000)),((nTotalTicks/(PalGetSysClock()/1000))/100),nFrames+1,PalGetSysClock(),gFLACContext.bps);
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
            nTotalTicks=0;                    
        }
        nFrames++;
#endif //FLAC_PERFORMANCE_TEST_BY_TICK

        
        bytesLeft = FillReadBuffer(gFLACContext.read_byte);

        if (init==0 && gFLACContext.got_streaminfo == 1){
            // flacFrameStep is 1.31 format.
            unsigned long long n = ((unsigned long long)gFLACContext.blocksize) << 31;
            flacFrameStep = (unsigned int)(n / gFLACContext.samplerate);
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)            
            gCh = gFLACContext.channels;
            gSampleRate = gFLACContext.samplerate;
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC);
#else
            initDAC(&gOutBuf[0],gFLACContext.channels,gFLACContext.samplerate,sizeof(gOutBuf),0);
#endif                           
            init = 1;
        }
        FillWriteBuffer(frameSize);

        checkControl();

        if (isSTOP()) break;

    #if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)    
        //ithPrintf("[FLAC] decode frames %d %d %d \n",frames++,frameSize,nTemp);
        //AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
    #else
        
    #endif
//        frames++;
#if defined(WIN32) || defined(__CYGWIN__)
        printf("decode frames %d \n",frames++);
        if (frames >=303)
            break;

        fwrite(&gFrame[0], 1, frameSize, fwav);
#endif

    }
    //fclose(fmp3);
    #if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)    
        ithPrintf("[FLAC] ClearRdBuffer \n");
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
    #else
        
    #endif
    
    ClearRdBuffer();
}
