/***************************************************************************
 * Copyright (c) 2004 SMedia Technology Corp. All Rights Reserved.
 *
 * @file
 * Win32 porting.
 *
 * @author Kuoping Hsu
 * @version 1.0
 *
 ***************************************************************************/
#if defined(WIN32) || defined(__CYGWIN__)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#include "win32.h"
#include "wavfilefmt.h"

///////////////////////////////////////////////////////////////////////////
//                              Local Variable
///////////////////////////////////////////////////////////////////////////
static char wav_out_file[128] = {0};
static char aac_info_fileName[] = "AAC_LATM.aac";
static int  isWaveFile = 0;

// It requires the libao.dll.a: Cross-Platform Audio Output Library.
#if defined(__CYGWIN__) && defined(__USE_LIBAO__)
#include <ao/ao.h>
static int audio_out = 0;
static ao_device *device = NULL;
#endif // __CYGWIN__

static FILE *fin  = NULL;
static FILE *fout = NULL;
static FILE *aacInfoFile = NULL;
static int fileEOF = 0;
static int fileReadIdx  = 0;
static int fileWriteIdx = 0;
static int voiceoff     = 0;
static int reverbOn     = 0;
static int eqEnable     = 0;

// I2S output buffer
static int i2sWrPtr  = 0;
static int i2sRdPtr  = 0;
static int i2sBufLen = 0;
static unsigned char *i2sBuf = NULL;

///////////////////////////////////////////////////////////////////////////
//                              Extern Variable
///////////////////////////////////////////////////////////////////////////
#if defined(__GNUC__)
extern unsigned char streamBuf[READBUF_SIZE] __attribute__ ((aligned(16)));
#else
extern unsigned char streamBuf[READBUF_SIZE];
#endif

///////////////////////////////////////////////////////////////////////////
//                              Flown Control Function
///////////////////////////////////////////////////////////////////////////
int  isEQ(void)        { return eqEnable;   }
int  isReverbOn(void)  { return reverbOn;   }
int  isVoiceOff(void)  { return voiceoff;   }
int  isEOF(void)       { return fileEOF;    }
int  isSTOP(void)      { return 0;          }
int  isPAUSE(void)     { return 0;          }
int  isDownSample(void){ return 0;          }

///////////////////////////////////////////////////////////////////////////
//                              OR32 Related Function
///////////////////////////////////////////////////////////////////////////
void dc_invalidate(void) { }
void or32_invalidate_cache(void *start, unsigned bytes) { }

void report_error(char *str) {
    printf("%s\n", str);
    exit(-1);
}

void or32_sleep(int ticks) {
    int i;
    for(i = 0; i<ticks; i++) ;
}

void or32_delay(int ms) {
    int i;
    for(i = 0; i<ms*10000; i++) ;
}

void win32_init(void) {
}

///////////////////////////////////////////////////////////////////////////
//                              MMIO Related Function
///////////////////////////////////////////////////////////////////////////
void MMIO_Write(int reg, int val) {
    switch (reg) {
        case DrvDecode_RdPtr   : fileReadIdx = val; break;
        case DrvDecode_WrPtr   : fileReadIdx = val; break;
        case DrvDecode_Frame   : break;
        case DrvDecode_Frame+2 :
                                 #if !defined(__DEBUG__)
                                 fprintf(stderr, "%02d:%02d\r", (int)(val/60), (val%60));
                                 #if defined(__CYGWIN__)
                                    fflush(stderr);
                                 #endif
                                 #endif // !defined(__DEBUG__)
                                 break;
        case DrvAudioCtrl      : break;
        default                : /*printf("MMIO_Write(0x%04x,0x%04x). Error!\n", reg, val);*/ break;
    }

    ASSERT((fileReadIdx  >= 0) && (fileReadIdx  <= READBUF_SIZE));
}

int MMIO_Read(int reg) {
    int len = 0;
    int rdlen = 0;

    switch(reg) {
        case DrvDecode_RdPtr : break;
        case DrvDecode_WrPtr : break;
        case DrvAudioCtrl    : return 0;
        default              : /*printf("MMIO_Read(0x%04x). Error!\n", reg);*/ break;
    }

    if(!fin) return 0;

    ASSERT((fileWriteIdx >= 0) && (fileWriteIdx <= READBUF_SIZE));
    ASSERT((fileReadIdx  >= 0) && (fileReadIdx  <= READBUF_SIZE));

    if (fileReadIdx > fileWriteIdx) {
        len = fileReadIdx - fileWriteIdx - 2;
    } else {
        len = READBUF_LEN - (fileWriteIdx - fileReadIdx) - 2;
    }

    // Limit read size foreach run
    // len = (len > 1024) ? 1024 : len;

    if (fileEOF || len < 512) {
        return (fileWriteIdx);
    }

    if (fileReadIdx > fileWriteIdx) {
        rdlen += fread(&streamBuf[fileWriteIdx+READBUF_BEGIN], sizeof(char), len, fin);
    } else {
        if (len < READBUF_LEN - fileWriteIdx) {
            rdlen += fread(&streamBuf[fileWriteIdx+READBUF_BEGIN], sizeof(char), len, fin);
        } else {
            rdlen += fread(&streamBuf[fileWriteIdx+READBUF_BEGIN], sizeof(char), READBUF_LEN - fileWriteIdx, fin);
            if ((len - (READBUF_LEN-fileWriteIdx)) > 0) {
               rdlen += fread(&streamBuf[READBUF_BEGIN], sizeof(char), len - (READBUF_LEN - fileWriteIdx), fin);
            }
        }
    }

    if (feof(fin)) {
        fileEOF = 1;
    }

    fileWriteIdx += rdlen;

    if (fileWriteIdx >= READBUF_LEN) {
        fileWriteIdx -= READBUF_LEN;
    }

    ASSERT((fileWriteIdx >= 0) && (fileWriteIdx <= READBUF_SIZE));

    return fileWriteIdx;
}

///////////////////////////////////////////////////////////////////////////
//                              Audio Output Control
///////////////////////////////////////////////////////////////////////////
FILE* GetInfoFile()
{
    return aacInfoFile;
}

void InfoFileClose()
{
    if (aacInfoFile != NULL)
    {
        fclose(aacInfoFile);        
    }
}
void SetOutWrPtr(int val) {
    i2sWrPtr = val;

    ASSERT(i2sWrPtr < i2sBufLen);

    if (fout != NULL) {
        if (i2sWrPtr >= i2sRdPtr) {
            fwrite(i2sBuf+i2sRdPtr, sizeof(char), i2sWrPtr - i2sRdPtr, fout);
        } else {
            fwrite(i2sBuf+i2sRdPtr, sizeof(char), i2sBufLen - i2sRdPtr, fout);
            fwrite(i2sBuf, sizeof(char), i2sWrPtr, fout);
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
    return i2sRdPtr;
}

int GetOutWrPtr(void) {
    return i2sWrPtr;
}

///////////////////////////////////////////////////////////////////////////
//                              CODEC Related Function
///////////////////////////////////////////////////////////////////////////
void pauseDAC(int flag) { }
void deactiveDAC(void) {

    if (fin  != NULL)
    {
        fclose(fin);
    }

    if (fout != NULL) 
    {
        fclose(fout);
    }

    if (isWaveFile) {
        //UpdateWAVHeader(wav_out_file);
    }

    #if defined(__CYGWIN__) && defined(__USE_LIBAO__)
    if (audio_out) {
        ao_close(device);
        ao_shutdown();
    }
    #endif // defined(__CYGWIN__) && defined(__USE_LIBAO__)

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

    /*if (wav_out_file[0] != 0) {
        if ((fout=fopen(wav_out_file, "wb")) == NULL) {
            printf("Can not create file '%s'\n", wav_out_file);
            exit(-1);
        }
    } else {
        fout = NULL;
    }*/
    // Assign the I2S output buffer
    i2sBuf    = bufptr;
    i2sBufLen = buflen;

    if (isWaveFile) {
        //WriteWAVHeader(fout, sample_rate, 16, nChannel);
    }
}

///////////////////////////////////////////////////////////////////////////
//                              Command Line Interface
///////////////////////////////////////////////////////////////////////////
void GetParam(int argc, char **argv)
{
    if (argc < 2) {
        #if !defined(INPUT_MEMMODE)
        printf("Usage: aacdecode [-ao] [-eq] [-off] [-reverb] file.aac [aac_out.[pcm|wav]] \n");
        #else
        printf("Usage: aacdecode [-ao] [-eq] [-off] [-reverb] [aac_out.[pcm|wav]] \n");
        #endif // !defined(INPUT_MEMMODE)
        printf("       -ao              use audio out library.\n");
        printf("       -eq              enable equalizer.\n");
        printf("       -reverb          enable reverberation.\n");
        printf("       -off             enable voice off.\n");
        printf("\n");
        #if !defined(INPUT_MEMMODE)
        printf("        file.ac3           input .ac3 file.\n");
        #endif // !defined(INPUT_MEMMODE)
        printf("        aac_out.[wav|pcm]  output filename of wav or pcm file.\n");
        printf("\n");
        #if !defined(INPUT_MEMMODE)
        printf("Example: aacdecode -ao in.aac out.wav\n");
        #else
        printf("Example: aacdecode -ao out.wav\n");
        #endif // !defined(INPUT_MEMMODE)
        exit(-1);
    }

    argv++; argc--;
    while(argc) {
        if (!strcmp(argv[0], "-eq")) {
            #if defined(EQUALIZER)
            eqEnable = 1;
            argv++; argc--;
            #else
            printf("Compile option dose not enable equalizer.\n");
            exit(-1);
            #endif
        } else if (!strcmp(argv[0], "-reverb")) {
            #if defined(REVERBERATION)
            reverbOn = 1;
            argv++; argc--;
            #else
            printf("Compile option dose not enable reverberation.\n");
            exit(-1);
            #endif
        } else if (!strcmp(argv[0], "-off")) {
            #if defined(VOICEOFF)
            voiceoff = 1;
            argv++; argc--;
            #else
            printf("Compile option dose not enable voice off.\n");
            exit(-1);
            #endif
        } else if (!strcmp(argv[0], "-ao")) {
            #if defined(__CYGWIN__) && defined(__USE_LIBAO__)
            audio_out = 1;
            argv++; argc--;
            #else
            printf("Compile option dose not enable audio out, it requires libao library.\n");
            exit(-1);
            #endif // defined(__CYGWIN__) && defined(__USE_LIBAO__)
        } else {
            int i;
            if (argv[0][0] == '-') {
                printf("Unknown option '%s'\n", argv[0]);
                exit(-1);
            }
            #if !defined(INPUT_MEMMODE)
            if ((fin = fopen(argv[0], "rb")) == NULL) {
                printf("Can not open file '%s'.\n", argv[0]);
                exit(-1);
            }
            argv++; argc--;
            #endif // !defined(INPUT_MEMMODE)
            if (argc > 0) {
                strcpy(wav_out_file, argv[0]);
                argv++; argc--;
            }

            for(i=strlen(wav_out_file)-1; wav_out_file[i] != '.' && i > 0; i--) ;
            isWaveFile = (i==0) ? 0 : (!strcmp(&wav_out_file[i], ".wav")) ? 1 : 0;
        }
    }
}
void openInputOutputStraem(char *input,char *output,int ch)
{
    unsigned char ucTmp[1024]={0};

    if ((fin = fopen(input, "rb")) == NULL) {
        printf("Can not open file '%s'.\n", input);
        exit(-1);
    }
    if ((fout=fopen(output, "wb")) == NULL) {
        printf("Can not create file '%s'\n", output);
        exit(-1);
    }
    fwrite(&ucTmp[0], 1, 512*ch, fout);
}
void setAudioReset(void)
{
}

int isResetAudioDecodedBytes(void)
{
	return 0;
}

void PalSleep(int time)
{
}

#endif // defined(WIN32) || defined(__CYGWIN__)

