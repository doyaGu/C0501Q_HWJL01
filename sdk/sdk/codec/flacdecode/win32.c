/**************************************************************************************
 * Source last modified: $Id: main.c,v 1.2 2005/12/15 14:45:30 $
 *
 * Copyright (c) 1995-2005 SMedia Tech. Corp., All Rights Reserved.
 *
 **************************************************************************************/

#include "win32.h"
#include "wavfilefmt.h"
#include "config.h"

char wav_out_file[128] = {0};
int  isWaveFile = 1;

// It requires the libao.dll.a: Cross-Platform Audio Output Library.
#if defined(__CYGWIN__) && defined(__USE_LIBAO__)
    #include <ao/ao.h>
    int audio_out = 0;
    ao_device *device = NULL;
#endif // __CYGWIN__

static unsigned char *mp3FileStream;
static int mp3FileSize;
static unsigned char *mp3File2Stream;
static int mp3File2Size;



FILE *fout = NULL;
static int fileEOF = 0;
static int fileReadPtr  = 0;
static int fileReadIdx  = 0;
static int fileWriteIdx = 0;

FILE *fout2 = NULL;
static int file2EOF = 0;
static int file2ReadPtr  = 0;
static int file2ReadIdx  = 0;
static int file2WriteIdx = 0;


// I2S output buffer
static int i2sWrPtr  = 0;
static int i2sRdPtr  = 0;
static int i2sBufLen = 0;
static unsigned char *i2sBuf = NULL;

extern unsigned char streamBuf[READBUF_SIZE];
//extern unsigned char streamBuf2[READBUF_SIZE];

#if !defined(API_V2)
# if defined(__GNUC__)
    extern unsigned char streamBuf[READBUF_SIZE] __attribute__ ((aligned(16)));
# else
# endif // defined(__GNUC__)
    unsigned int   streamLen=READBUF_SIZE;
    unsigned int   streamLen2;
#else
    extern unsigned char *streamBuf;
    extern unsigned int   streamLen;
#endif // !defined(API_V2)

#if defined(API_V2)
#include "driver.h"
extern SHAREINFO shareinfo;
#endif // defined(API_V2)

///////////////////////////////////////////////////////////////////////////
//                              Private Function
///////////////////////////////////////////////////////////////////////////
static void pcmWriteOut(char *ptr, int len) {
    #if 0 // swap the byte order to bigendian
    int  i;
    char tmp;
    for(i=0; i<len; i+=2) {
        tmp      = ptr[i];
        ptr[i]   = ptr[i+1];
        ptr[i+1] = tmp;
    }
    #endif
    if (len == 0)
    {
        return;
    }
    fwrite(ptr, sizeof(char), len, fout);
}

void pcmWriteOut2(char *ptr, int len) {
    #if 0 // swap the byte order to bigendian
    int  i;
    char tmp;
    for(i=0; i<len; i+=2) {
        tmp      = ptr[i];
        ptr[i]   = ptr[i+1];
        ptr[i+1] = tmp;
    }
    #endif
    fwrite(ptr, sizeof(char), len, fout2);
}

///////////////////////////////////////////////////////////////////////////
//                              Flown Control Function
///////////////////////////////////////////////////////////////////////////
static short mmio_ctrl = 0;

int  isEQ(void)        { return ((mmio_ctrl & DrvDecode_EnEQ       ) != 0); }
int  isReverbOn(void)  { return ((mmio_ctrl & DrvDecode_EnReverb   ) != 0); }
int  isDrcOn(void)     { return ((mmio_ctrl & DrvDecode_EnDRC      ) != 0); }
int  isVoiceOff(void)  { return ((mmio_ctrl & DrvDecode_EnVoiceOff ) != 0); }
int  isEOF(void)       { return fileEOF;    }
int  isSTOP(void)      { return ((mmio_ctrl & DrvDecode_STOP       ) != 0); }
int  isPAUSE(void)     { return ((mmio_ctrl & DrvDecode_PAUSE      ) != 0); }
void clrDecSTOP()      { mmio_ctrl = mmio_ctrl & (short)~DrvDecode_STOP;  }
void clrDecPAUSE()     { mmio_ctrl = mmio_ctrl & (short)~DrvDecode_PAUSE; }
int  isDownSample(void){ return 0; }

void dc_invalidate(void) {
}

void win32_init(void) {
}

void or32_sleep(int ticks) {
    int i;
    for(i=0; i<ticks; i++);
}

void or32_delay(int ms) {
    int i;
    for(i=0; i<ms*10000; i++);
}

void or32_invalidate_cache(void *start, unsigned bytes) {
}

void pauseDAC(int pause){
}

void initDAC(unsigned char *bufptr, int nChannel, int sample_rate, int buflen, int bigendian) {
    #if defined(__CYGWIN__) && defined(__USE_LIBAO__)
    ao_sample_format format;

    if (audio_out) {
        ao_initialize();

        /* -- Setup for default driver -- */
        format.bits        = 16;
        format.channels    = nChannel;
        format.rate        = sample_rate;
        format.byte_format = AO_FMT_LITTLE;

        fprintf(stderr, "Open Audio Device with %d sample rate, and %d channel.\n",
                sample_rate, nChannel);

        /* -- Open driver -- */
        device = ao_open_live(ao_default_driver_id(), &format, NULL /* no options */);
        if (device == NULL) {
            fprintf(stderr, "Error opening device.\n");
            exit(-1);
        }
    }
    #endif // defined(__CYGWIN__) && defined(__USE_LIBAO__)

    if (wav_out_file[0] != 0)
    {
        if ((fout=fopen(wav_out_file, "wb")) == NULL) {
            printf("Can not create file '%s'\n", wav_out_file);
            exit(-1);
        }
    } else {
        fout = NULL;
    }

    // Assign the I2S output buffer
    i2sBuf    = bufptr;
    i2sBufLen = buflen;

    if (isWaveFile) {
        WriteWAVHeader(fout, sample_rate, 16, nChannel);
    }
}
void setAudioReset()
{

}

int isUpSample()
{
    return 0;
}

int isUpSampleOnly2x()
{
    return 0;
}

void MMIO_Write(int reg, int val) 
{
    switch (reg) 
    {
        case DrvDecode_RdPtr   : fileReadIdx = val; break;
        case DrvDecode_WrPtr   : fileReadIdx = val; break;
        case DrvDecode_RdPtr2   : file2ReadIdx = val; break;
        case DrvDecode_WrPtr2   : file2ReadIdx = val; break;
#if !defined(API_V2)
        case DrvDecode_Frame   : break;
        case DrvDecode_Frame+2 : fprintf(stderr, "%02d:%02d\r", (int)(val/60), (val%60));
                                 #if defined(__CYGWIN__)
                                    fflush(stderr);
                                 #endif
                                 break;
#endif // !defined(API_V2)
        case DrvAudioCtrl      : mmio_ctrl = val; break;
        default                : printf("MMIO_Write(0x%04x,0x%04x). Error!\n", reg, val); break;
    }

    //ASSERT((fileReadIdx  >= 0) && (fileReadIdx  <= streamLen));
}

int MMIO_Read(int reg) {
    int len = 0;

    switch(reg) {
        case DrvDecode_WrPtr : break;
        case DrvDecode_WrPtr2 : break;
        case DrvAudioCtrl    : return mmio_ctrl;
        case DrvDecode_Frame : return 0;
        case DrvDecode_Frame+2: return 0;
        case DrvDecode_RdPtr   : return fileReadIdx;
        case DrvDecode_RdPtr2  : return file2ReadIdx;
        default              : printf("MMIO_Read(0x%04x). Error!\n", reg); break;
    }

    //ASSERT((fileWriteIdx >= 0) && (fileWriteIdx <= streamLen));
    //ASSERT((fileReadIdx  >= 0) && (fileReadIdx  <= streamLen));

    if (reg == DrvDecode_WrPtr)
    {
        if (fileReadIdx > fileWriteIdx) {
            len = fileReadIdx - fileWriteIdx - 2;
        } else {
            len = streamLen - (fileWriteIdx - fileReadIdx) - 2;
        }

        if (fileReadPtr + len >= mp3FileSize) {
            len = mp3FileSize - fileReadPtr;
            fileEOF = 1;
            mmio_ctrl |= DrvDecode_EOF;
        }

        if (len <= 0) return fileWriteIdx;

        if (fileReadIdx > fileWriteIdx) {
            memcpy(&streamBuf[fileWriteIdx], &mp3FileStream[fileReadPtr], len);
        } else {
            if (len < streamLen - fileWriteIdx) {
                memcpy(&streamBuf[fileWriteIdx], &mp3FileStream[fileReadPtr], len);
            } else {
                memcpy(&streamBuf[fileWriteIdx], &mp3FileStream[fileReadPtr], streamLen - fileWriteIdx);
                if ((len - (streamLen-fileWriteIdx)) > 0) {
                   memcpy(&streamBuf[0], &mp3FileStream[fileReadPtr+streamLen-fileWriteIdx], len - (streamLen - fileWriteIdx));
                }
            }
        }

        fileWriteIdx += len;

        if (fileWriteIdx >= streamLen) {
            fileWriteIdx -= streamLen;
        }

        //ASSERT((fileWriteIdx >= 0) && (fileWriteIdx <= streamLen));

        fileReadPtr += len;

        #if defined(DEBUG)
        printf("fileReadPtr(%d) mp3FileSize(%d)\n", fileReadPtr, mp3FileSize);
        #endif // defined(DEBUG)

        return fileWriteIdx;
    }
    else if (reg == DrvDecode_WrPtr2)
    {
/*
        if (file2ReadIdx > file2WriteIdx) {
            len = file2ReadIdx - file2WriteIdx - 2;
        } else {
            len = streamLen2 - (file2WriteIdx - file2ReadIdx) - 2;
        }

        if (file2ReadPtr + len >= mp3File2Size) {
            len = mp3File2Size - file2ReadPtr;
            file2EOF = 1;
            //mmio_ctrl |= DrvDecode_EOF;
        }

        if (len <= 0) return file2WriteIdx;

        if (file2ReadIdx > file2WriteIdx) {
            memcpy(&streamBuf[1][file2WriteIdx], &mp3File2Stream[file2ReadPtr], len);
        } else {
            if (len < streamLen2 - file2WriteIdx) {
                memcpy(&streamBuf[1][file2WriteIdx], &mp3File2Stream[file2ReadPtr], len);
            } else {
                memcpy(&streamBuf[1][file2WriteIdx], &mp3File2Stream[file2ReadPtr], streamLen2 - file2WriteIdx);
                if ((len - (streamLen2-file2WriteIdx)) > 0) {
                   memcpy(&streamBuf[1][0], &mp3File2Stream[file2ReadPtr+streamLen2-file2WriteIdx], len - (streamLen2 - file2WriteIdx));
                }
            }
        }

        file2WriteIdx += len;

        if (file2WriteIdx >= streamLen2) {
            file2WriteIdx -= streamLen2;
        }

        file2ReadPtr += len;
*/
        return file2WriteIdx;

    }
}

void SetOutWrPtr(int val) {
    i2sWrPtr = val;
    if (fout != NULL) {
        if (i2sWrPtr >= i2sRdPtr) {
            pcmWriteOut(i2sBuf+i2sRdPtr, i2sWrPtr - i2sRdPtr);
        } else {
            pcmWriteOut(i2sBuf+i2sRdPtr, i2sBufLen - i2sRdPtr);
            pcmWriteOut(i2sBuf, i2sWrPtr);
        }
    }

    #if defined(__CYGWIN__) && defined(__USE_LIBAO__)
    if (audio_out) {
        if (i2sWrPtr >= i2sRdPtr) {
            ao_play(device, i2sBuf+i2sRdPtr, i2sWrPtr - i2sRdPtr);
        } else {
            ao_play(device, i2sBuf+i2sRdPtr, i2sBufLen - i2sRdPtr);
            ao_play(device, i2sBuf, i2sWrPtr);
        }
    }
    #endif // defined(__CYGWIN__) && defined(__USE_LIBAO__)

    i2sRdPtr = i2sWrPtr;
}

int GetOutRdPtr(void) {
    #if defined(OUTPUT_MEMMODE)
    return 0;
    #else
    return i2sRdPtr;
    #endif
}

int GetOutWrPtr(void) {
    return i2sWrPtr;
}

void report_error(char *str) {
    printf("%s\n", str);
    exit(-1);
}

void deactiveDAC(void) 
{
    char *fileName ="D:\\audio_testing\\Mp3\\BBC7.wav";

    if (fout != NULL) fclose(fout);

    if (isWaveFile) {
        UpdateWAVHeader(wav_out_file);
    }
    UpdateWAVHeader(fileName);
    #if defined(__CYGWIN__) && defined(__USE_LIBAO__)
    if (audio_out) {
        ao_close(device);
        ao_shutdown();
    }
    #endif // defined(__CYGWIN__) && defined(__USE_LIBAO__)

}

void GetParam(int argc, char **argv)
{
    #if !defined(INPUT_MEMMODE)
    FILE *infile = NULL;
    int size;
    #endif
#define DECODE2
    char *fileName ="D:\\audio_testing\\Mp3\\BBC48000.wav";
    char *outPcm = "D:\\audio_testing\\Mp3\\BBC7.wav";
#ifdef DECODE2

#endif
    if (argc < 2) {
        printf("Usage: mp3decode [-ao] [-eq] [-off] [-reverb] [-drc] file.mp3 [mp3_out.[pcm|wav]] \n");
        printf("       -ao              use audio out library.\n");
        printf("       -eq              enable equalizer (also enable DRC).\n");
        printf("       -reverb          enable reverberation (also enable DRC).\n");
        printf("       -drc             enable DRC.\n");
        printf("       -off             enable voice off.\n");
        printf("\n");
        printf("        file.mp3           input .mp3 file.\n");
        printf("        mp3_out.[wav|pcm]  output filename of wav or pcm file.\n");
        printf("\n");
        printf("Example: mp3decode -ao in.mp3 out.wav\n");
        exit(-1);
    }

    argv++; argc--;
    while(argc) {
        if (!strcmp(argv[0], "-eq")) {
            #if defined(EQUALIZER)
            mmio_ctrl |= DrvDecode_EnEQ;
            mmio_ctrl |= DrvDecode_EnDRC;
            argv++; argc--;
            #else
            printf("Compile option dose not enable equalizer.\n");
            exit(-1);
            #endif
        } else if (!strcmp(argv[0], "-reverb")) {
            #if defined(REVERBERATION)
            mmio_ctrl |= DrvDecode_EnReverb;
            mmio_ctrl |= DrvDecode_EnDRC;
            argv++; argc--;
            #else
            printf("Compile option dose not enable reverberation.\n");
            exit(-1);
            #endif
        } else if (!strcmp(argv[0], "-drc")) {
            #if defined(DRC)
            mmio_ctrl |= DrvDecode_EnDRC;
            argv++; argc--;
            #else
            printf("Compile option dose not enable drc.\n");
            exit(-1);
            #endif
        } else if (!strcmp(argv[0], "-off")) {
            mmio_ctrl |= DrvDecode_EnVoiceOff;
            argv++; argc--;
        } else if (!strcmp(argv[0], "-ao")) {
            #if defined(__CYGWIN__) && defined(__USE_LIBAO__)
            audio_out = 1;
            argv++; argc--;
            #else
            argv++; argc--;
			
		   /*if ((fout = fopen(argv[0], "rb")) == NULL)
		   {
				printf("Can not open file '%s'.\n", argv[0]);
				exit(-1);
			}*/
            

            //printf("Compile option dose not enable audio out, it requires libao library.\n");
            //exit(-1);
            #endif // defined(__CYGWIN__) && defined(__USE_LIBAO__)
        } else {
            int i;
            #if !defined(OUTPUT_MEMMODE)
            if (argv[0][0] == '-') {
                printf("Unknown option '%s'\n", argv[0]);
                exit(-1);
            }
            #endif

            #if !defined(INPUT_MEMMODE)
            if ((infile = fopen(argv[0], "rb")) == NULL) {
                printf("Can not open file '%s'.\n", argv[0]);
                exit(-1);
            }
            #endif

            argv++; argc--;
            if (argc > 0) {
                strcpy(wav_out_file, argv[0]);
                argv++; argc--;
            }

            for(i=strlen(wav_out_file)-1; wav_out_file[i] != '.' && i > 0; i--) ;
            isWaveFile = (i==0) ? 0 : (!strcmp(&wav_out_file[i], ".wav")) ? 1 : 0;
        }
    }

    #if !defined(INPUT_MEMMODE)
    fseek(infile, 0, SEEK_END);
    mp3FileSize = size = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    if (mp3FileSize & 0x1) mp3FileSize++;

    if ((mp3FileStream = malloc(mp3FileSize)) == NULL) {
        printf("Memory not enougth.\n");
        exit(-1);
    }

    if (fread(mp3FileStream, sizeof(char), size, infile) != (unsigned)size) {
        printf("Data read fails.\n");
        exit(-1);
    }

    fclose(infile);
    
    if ((infile = fopen(fileName, "rb")) == NULL) {
        printf("Can not open file '%s'.\n", argv[0]);
        exit(-1);
    }
    fseek(infile, 0, SEEK_END);
    mp3File2Size = size = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    if (mp3File2Size & 0x1) mp3FileSize++;

    if ((mp3File2Stream = malloc(mp3File2Size)) == NULL) {
        printf("Memory not enougth.\n");
        exit(-1);
    }

    if (fread(mp3File2Stream, sizeof(char), size, infile) != (unsigned)size) {
        printf("Data read fails.\n");
        exit(-1);
    }

    fclose(infile);
    if ((fout2=fopen(outPcm, "wb")) == NULL) 
    {
        printf("Can not create file '%s'\n", outPcm);
        exit(-1);
    }
    WriteWAVHeader(fout2, 44100, 16, 2);
    
    #endif // !defined(INPUT_MEMMODE)

    #if defined(API_V2)
    if ((shareinfo.decBufBase = (int)malloc(shareinfo.decBufOffset+shareinfo.minDecBufSize*20)) == 0) {
        printf("Memory not enougth.\n");
        exit(-1);
    } else {
        shareinfo.decBufLength = shareinfo.minDecBufSize*20;
    }

    if ((shareinfo.reverbBase = (int)malloc(shareinfo.reverbLength)) == 0) {
        printf("Memory not enougth.\n");
        exit(-1);
    }
    #endif // defined(API_V2)
}

void *memset16 (void *dst, const int c, unsigned int num)
{
    //ASSERT(((int)dst & 0x1)==0);
    return memset(dst, c, sizeof(short) * num);
}

void *memset32 (void *dst, const int c, unsigned int num)
{
    //ASSERT(((int)dst & 0x3)==0);
    return memset(dst, c, sizeof(long) * num);
}
