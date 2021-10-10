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

#include "debug.h"
#include "win32.h"
#include "wavfilefmt.h"

#define LIMITTED_IO
#define LIMITTED_SIZE 116

#define getFreeLen(rdptr, wrptr, len) (((rdptr) <= (wrptr)) ? ((len) - ((wrptr) - (rdptr)) - 2) : ((rdptr) - (wrptr) - 2))
#define getUsedLen(rdptr, wrptr, len) (((wrptr) >= (rdptr)) ? ((wrptr) - (rdptr)) : ((len) - ((rdptr) - (wrptr))))

void WAV_GetBufInfo(unsigned* inbuf, unsigned* inlen);

///////////////////////////////////////////////////////////////////////////
//                              Local Variable
///////////////////////////////////////////////////////////////////////////
static int  audioCtrl  = 0;
static int  audioCtrl2 = 0;
static char wav_in_file[128] = { 0 };
static char wav_out_file[128] = { 0 };
static int  isWaveFile = 0;
static int  isEncode = 0;

// It requires the libao.dll.a: Cross-Platform Audio Output Library.
#if defined(__CYGWIN__) && defined(__USE_LIBAO__)
#include <ao/ao.h>
static int audio_out = 0;
static ao_device *device = NULL;
#endif // __CYGWIN__

static FILE *fin  = NULL;
static FILE *fout = NULL;
static int fileEOF = 0;
static int fileReadIdx  = 0;
static int fileWriteIdx = 0;

// I2S output buffer
static int i2sWrPtr  = 0;
static int i2sRdPtr  = 0;
static int i2sBufLen = 0;
static unsigned char *i2sBuf = NULL;

const static int SampsRate[] = {
     6000,  8000, 11025, 12000,
    16000, 22050, 24000, 32000,
    44100, 48000
};

static unsigned char *streamBuf;
static unsigned int  streamBufSize;

///////////////////////////////////////////////////////////////////////////
//                              Extern Variable
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//                              Flown Control Function
///////////////////////////////////////////////////////////////////////////
int  isEncEOF(void)    { return fileEOF;    }
int  isEOF(void)       { return fileEOF;    }
int  isSTOP(void)      { return 0;          }
int  isPAUSE(void)     { return 0;          }

///////////////////////////////////////////////////////////////////////////
//                              OR32 Related Function
///////////////////////////////////////////////////////////////////////////
void dc_invalidate(void) { }
void or32_invalidate_cache(void *start, unsigned bytes) { }

void or32_sleep(int ticks) {
    int i;
    for(i = 0; i<ticks; i++) ;
}

void or32_delay(int ms) {
    int i;
    for(i = 0; i<ms*10000; i++) ;
}

void win32_init(void) {
    unsigned ptr;
    WAV_GetBufInfo(&ptr, &streamBufSize);
    streamBuf = (unsigned char*)ptr;
}

///////////////////////////////////////////////////////////////////////////
//                              MMIO Related Function
///////////////////////////////////////////////////////////////////////////
void MMIO_Write(int reg, int val) {
    switch (reg) {
        // Decoder
        case DrvDecode_RdPtr   : fileReadIdx = val; break;
        case DrvDecode_WrPtr   : fileWriteIdx = val; break;
        case DrvDecode_Frame   : break;
        case DrvDecode_Frame+2 :
                                 #if !defined(__DEBUG__)
                                 fprintf(stderr, "%02d:%02d\r", (int)(val/60), (val%60));
                                 fflush(stderr);
                                 #endif // !defined(__DEBUG__)
                                 break;
        // Encoder
        case DrvEncode_RdPtr   : fileReadIdx = val; break;
        case DrvEncode_WrPtr   : fileWriteIdx = val; break;
        case DrvEncode_Frame   : break;
        case DrvEncode_Frame+2 :
                                 #if !defined(__DEBUG__)
                                 fprintf(stderr, "%02d:%02d\r", (int)(val/60), (val%60));
                                 fflush(stderr);
                                 #endif // !defined(__DEBUG__)
                                 break;
        case DrvAudioCtrl      : audioCtrl  = val; break;
        case DrvAudioCtrl2     : audioCtrl2 = val; break;
        default                : printf("MMIO_Write(0x%04x,0x%04x). Error!\n", reg, val); break;
    }

    ASSERT((fileReadIdx  >= 0) && (fileReadIdx  <= streamBufSize));
}

int MMIO_Read(int reg) {
    int len = 0;
    int rdlen = 0;

    switch(reg) {
        case DrvDecode_RdPtr : return fileReadIdx;
        case DrvDecode_WrPtr :
             ASSERT((fileWriteIdx >= 0) && (fileWriteIdx <= streamBufSize));
             ASSERT((fileReadIdx  >= 0) && (fileReadIdx  <= streamBufSize));

             len = getFreeLen(fileReadIdx, fileWriteIdx, streamBufSize);
             // Limit read size foreach run
             #if defined(LIMITTED_IO)
             len = (len > LIMITTED_SIZE) ? LIMITTED_SIZE : len;
             #endif

             if (fileEOF) {
                 return (fileWriteIdx);
             }

             if (fileReadIdx > fileWriteIdx) {
                 rdlen += fread(&streamBuf[fileWriteIdx], sizeof(char), len, fin);
             } else {
                 if (len < streamBufSize - fileWriteIdx) {
                     rdlen += fread(&streamBuf[fileWriteIdx], sizeof(char), len, fin);
                 } else {
                     rdlen += fread(&streamBuf[fileWriteIdx], sizeof(char), streamBufSize - fileWriteIdx, fin);
                     if ((len - (streamBufSize-fileWriteIdx)) > 0) {
                        rdlen += fread(&streamBuf[0], sizeof(char), len - (streamBufSize - fileWriteIdx), fin);
                     }
                 }
             }

             if (feof(fin)) {
                 fileEOF = 1;
             }

             fileWriteIdx += rdlen;

             if (fileWriteIdx >= streamBufSize) {
                 fileWriteIdx -= streamBufSize;
             }

             ASSERT((fileWriteIdx >= 0) && (fileWriteIdx <= streamBufSize));

             return fileWriteIdx;
        case DrvEncode_RdPtr :
             len = getUsedLen(fileReadIdx, fileWriteIdx, streamBufSize);
             // Limit write size foreach run
             #if defined(LIMITTED_IO)
             len = (len > LIMITTED_SIZE) ? LIMITTED_SIZE : len;
             #endif
             if (fileWriteIdx >= fileReadIdx) {
                 fwrite(streamBuf+fileReadIdx, sizeof(char), len, fout);
             } else {
                 if (len < streamBufSize - fileReadIdx) {
                     fwrite(streamBuf+fileReadIdx, sizeof(char), len, fout);
                 } else {
                     fwrite(streamBuf+fileReadIdx, sizeof(char), streamBufSize - fileReadIdx, fout);
                     if (len > streamBufSize - fileReadIdx) {
                         fwrite(streamBuf, sizeof(char), len - (streamBufSize - fileReadIdx), fout);
                     }
                 }
             }
             fileReadIdx += len;
             if (fileReadIdx >= streamBufSize) fileReadIdx -= streamBufSize;
             return fileReadIdx;
        case DrvEncode_WrPtr : return fileWriteIdx;
        case DrvAudioCtrl    : return audioCtrl;
        case DrvAudioCtrl2   : return audioCtrl2;
        default              : printf("MMIO_Read(0x%04x). Error!\n", reg); break;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////
//                              Audio Output Control
///////////////////////////////////////////////////////////////////////////
void SetOutWrPtr(int val) {
    i2sWrPtr = val;
}

int GetOutRdPtr(void) {
    int len;

    len = getUsedLen(i2sRdPtr, i2sWrPtr, i2sBufLen);
    // Limit write size foreach run
    #if defined(LIMITTED_IO)
    len = (len > LIMITTED_SIZE) ? LIMITTED_SIZE : len;
    #endif

    if (fout != NULL) {
        if (i2sWrPtr >= i2sRdPtr) {
            fwrite(i2sBuf+i2sRdPtr, sizeof(char), len, fout);
        } else {
            if (len < i2sBufLen - i2sRdPtr) {
                fwrite(i2sBuf+i2sRdPtr, sizeof(char), len, fout);
            } else {
                fwrite(i2sBuf+i2sRdPtr, sizeof(char), i2sBufLen - i2sRdPtr, fout);
                if ((len - (i2sBufLen - i2sRdPtr)) > 0) {
                    fwrite(i2sBuf, sizeof(char), len - (i2sBufLen - i2sRdPtr), fout);
                }
            }
        }
    }

    #if defined(__CYGWIN__) && defined(__USE_LIBAO__)
    if (audio_out) {
        if (i2sWrPtr >= i2sRdPtr) {
            ao_play(device, i2sBuf+i2sRdPtr, len);
        } else {
            if (len < i2sBufLen - i2sRdPtr) {
                ao_play(device, i2sBuf+i2sRdPtr, len);
            } else {
                ao_play(device, i2sBuf+i2sRdPtr, i2sBufLen - i2sRdPtr);
                if ((len - (i2sBufLen - i2sRdPtr)) > 0) {
                    ao_play(device, i2sBuf, len - (i2sBufLen - i2sRdPtr));
                }
            }
        }
    }
    #endif // defined(__CYGWIN__) && defined(__USE_LIBAO__)

    i2sRdPtr += len;
    if (i2sRdPtr >= i2sBufLen) i2sRdPtr -= i2sBufLen;

    return i2sRdPtr;
}

int GetOutWrPtr(void) {
    return i2sWrPtr;
}

///////////////////////////////////////////////////////////////////////////
//                              Audio Input Control
///////////////////////////////////////////////////////////////////////////
void SetInRdPtr(int val) {
    i2sRdPtr = val;
}

int  GetInWrPtr(void) {
    int len = 0;
    int rdlen = 0;

    ASSERT((i2sWrPtr >= 0) && (i2sWrPtr <= i2sBufLen));
    ASSERT((i2sRdPtr >= 0) && (i2sRdPtr <= i2sBufLen));

    len = getFreeLen(i2sRdPtr, i2sWrPtr, i2sBufLen);

    // Limit read size foreach run
    #if defined(LIMITTED_IO)
    len = (len > LIMITTED_SIZE) ? LIMITTED_SIZE : len;
    #endif

    if (fileEOF) {
        return (i2sWrPtr);
    }

    if (i2sRdPtr > i2sWrPtr) {
        rdlen += fread(&i2sBuf[i2sWrPtr], sizeof(char), len, fin);
    } else {
        if (len < i2sBufLen - i2sWrPtr) {
            rdlen += fread(&i2sBuf[i2sWrPtr], sizeof(char), len, fin);
        } else {
            rdlen += fread(&i2sBuf[i2sWrPtr], sizeof(char), i2sBufLen - i2sWrPtr, fin);
            if ((len - (i2sBufLen-i2sWrPtr)) > 0) {
               rdlen += fread(&i2sBuf[0], sizeof(char), len - (i2sBufLen - i2sWrPtr), fin);
            }
        }
    }

    if (feof(fin)) {
        fileEOF = 1;
    }

    i2sWrPtr += rdlen;

    if (i2sWrPtr >= i2sBufLen) {
        i2sWrPtr -= i2sBufLen;
    }

    ASSERT((i2sWrPtr >= 0) && (i2sWrPtr <= i2sBufLen));

    return i2sWrPtr;
}

int  GetInRdPtr(void) {
    return i2sRdPtr;
}

///////////////////////////////////////////////////////////////////////////
//                              CODEC Related Function
///////////////////////////////////////////////////////////////////////////
void pauseDAC(int flag) { }
void deactiveDAC(void) {
    if (fin  != NULL) fclose(fin);
    if (fout != NULL) fclose(fout);

    if (isWaveFile && !isEncode) {
        UpdateWAVHeader(wav_out_file);
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

    // Assign the I2S output buffer
    i2sBuf    = bufptr;
    i2sBufLen = buflen;

    if (isWaveFile) {
        WriteWAVHeader(fout, sample_rate, 16, nChannel);
    }
}

void pauseADC(int flag) { }
void deactiveADC(void) {
    if (fin  != NULL) PRINTF("Totally read %d bytes.\n", (int)ftell(fin));
    if (fout != NULL) PRINTF("Totally write %d bytes.\n", (int)ftell(fout));
    if (fin  != NULL) fclose(fin);
    if (fout != NULL) fclose(fout);
}

void initADC(unsigned char *bufptr, int nChannel, int sample_rate, int buflen, int bigendian) {
    int sr, ch, i;

    if (isWaveFile) {
        ReadWAVHeader(fin, &sr, &ch);
        for(i=0; i<sizeof(SampsRate)/sizeof(int) && sr != SampsRate[i]; i++) ;
        if (i==(sizeof(SampsRate)/sizeof(int))) {
            printf("Do not support sampling rate %d\n", sr);
            exit(-1);
        }
        audioCtrl2 = audioCtrl2 | ((i << DrvWAV_EncSampRateBits) & DrvWAV_EncSampRate);
        if (ch == 2) {
            audioCtrl2 = audioCtrl2 | DrvWAV_EncChannel;
        } else if (ch != 1) {
            printf("Do not support %d channel wave file.\n", ch);
            exit(-1);
        }
        printf("Init sampling rate %d with %d channel.\n", sr, ch);
    } else {
        printf("Init sampling rate %d with %d channel.\n", sample_rate, nChannel);
    }

    // Assign the I2S output buffer
    i2sBuf    = bufptr;
    i2sBufLen = buflen;
}

///////////////////////////////////////////////////////////////////////////
//                              Command Line Interface
///////////////////////////////////////////////////////////////////////////
void GetParam(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: wave [-dec] [-enc -rate n -ch n] [-ao] file.wav [pcm_out.[pcm|wav]] \n");
        printf("       -ao              use audio out library.\n");
        printf("\n");
        printf("        file.wav  input file.\n");
        printf("        pcm_out.[wav|pcm] output filename of wav or pcm file.\n");
        printf("\n");
        printf("Example: wavedec -ao in.alaw out.wav\n");
        exit(-1);
    }

    argv++; argc--;
    while(argc) {
        if (!strcmp(argv[0], "-ao")) {
            #if defined(__CYGWIN__) && defined(__USE_LIBAO__)
            if (((audioCtrl2 & DrvWAV_Mode) >> DrvWAV_Mode) == DrvWAV_ENCODE) {
                printf("Error! Can not use audio out on the encode mode.\n");
                exit(-1);
            }
            audio_out = 1;
            argv++; argc--;
            if (isEncode) {
                printf("Can not use audio output library on encode mode.\n");
                exit(-1);
            }
            #else
            printf("Compile option dose not enable audio out, it requires libao library.\n");
            exit(-1);
            #endif // defined(__CYGWIN__) && defined(__USE_LIBAO__)
        } else if (!strcmp(argv[0], "-enc")) {
            isEncode = 1;
            argv++; argc--;
            audioCtrl2 = audioCtrl2 | (DrvWAV_ENCODE << DrvWAV_ModeBits);
            if (audio_out) {
                printf("Can not use audio output library on encode mode.\n");
                exit(-1);
            }
        } else if (!strcmp(argv[0], "-dec")) {
            isEncode = 0;
            argv++; argc--;
            audioCtrl2 = audioCtrl2 | (DrvWAV_DECODE << DrvWAV_ModeBits);
        } else if (!strcmp(argv[0], "-rate")) {
            int i;
            argv++; argc--;
            for(i=0; i<sizeof(SampsRate)/sizeof(int) && atoi(argv[0]) != SampsRate[i]; i++) ;
            if (i==(sizeof(SampsRate)/sizeof(int))) {
                printf("Do not support sampling rate %d\n", atoi(argv[0]));
                exit(-1);
            }
            audioCtrl2 = audioCtrl2 | ((i << DrvWAV_EncSampRateBits) & DrvWAV_EncSampRate);
            argv++; argc--;
        } else if (!strcmp(argv[0], "-ch")) {
            argv++; argc--;
            if (atoi(argv[0]) == 2) {
                audioCtrl2 = audioCtrl2 | DrvWAV_EncChannel;
            }
            argv++; argc--;
        } else {
            int i;
            if (argv[0][0] == '-') {
                printf("Unknown option '%s'\n", argv[0]);
                exit(-1);
            }
            strcpy(wav_in_file, argv[0]);
            if ((fin = fopen(wav_in_file, "rb")) == NULL) {
                printf("Can not open file '%s'.\n", wav_in_file);
                exit(-1);
            }
            if (isEncode) {
                for(i=strlen(wav_in_file)-1; wav_in_file[i] != '.' && i > 0; i--) ;
                isWaveFile = (i==0) ? 0 : (!strcmp(&wav_in_file[i], ".wav")) ? 1 : 0;
            }

            argv++; argc--;
            if (argc > 0) {
                strcpy(wav_out_file, argv[0]);
                argv++; argc--;

                if ((fout=fopen(wav_out_file, "wb")) == NULL) {
                    printf("Can not create file '%s'\n", wav_out_file);
                    exit(-1);
                }
            } else {
                fout = NULL;
            }

            if (!isEncode) {
                for(i=strlen(wav_out_file)-1; wav_out_file[i] != '.' && i > 0; i--) ;
                isWaveFile = (i==0) ? 0 : (!strcmp(&wav_out_file[i], ".wav")) ? 1 : 0;
            }
        }
    }
}

#endif // defined(WIN32) || defined(__CYGWIN__)

