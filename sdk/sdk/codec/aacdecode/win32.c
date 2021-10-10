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
#include "aacdec.h"
#include "coder.h"

#include "win32.h"
#include "wavfilefmt.h"

///////////////////////////////////////////////////////////////////////////
//                              Local Variable
///////////////////////////////////////////////////////////////////////////
static char wav_out_file[128] = {0};
static char aac_info_fileName[] = "AAC_LATM.aac";
static int  isWaveFile = 0;

#if defined(ENABLE_MIXER)
#include "mixer.h"
static char mixfile[128] = {0};
static FILE *fp_mix = NULL;
#endif // defined(ENABLE_MIXER)

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

// PCM mix buffer
#if defined(ENABLE_MIXER)
static int mixRdPtr = 0;
static int mixWrPtr = 0;
static int mixBufLen = 0;
static unsigned char *mixBuf = NULL;
#endif // defined(ENABLE_MIXER)

///////////////////////////////////////////////////////////////////////////
//                              Extern Variable
///////////////////////////////////////////////////////////////////////////
#if defined(__GNUC__)
extern unsigned char streamBuf[READBUF_SIZE] __attribute__ ((aligned(16)));
#else
extern unsigned char streamBuf[READBUF_SIZE];
#endif

#if defined(ENABLE_MIXER)
extern HWORD pcminbuf[I2SBUFSIZE];
extern int pcm1nchans, pcm1samps;
extern int mix_enable;
#endif // defined(ENABLE_MIXER)

///////////////////////////////////////////////////////////////////////////
//                              Flown Control Function
///////////////////////////////////////////////////////////////////////////
int  isEQ(void)        { return eqEnable;   }
int  isReverbOn(void)  { return reverbOn;   }
int  isVoiceOff(void)  { return voiceoff;   }
int  isEOF(void)       { return fileEOF;    }
int  isSTOP(void)      { return 0;          }
int  isPAUSE(void)     { return 0;          }
#if defined(ENABLE_MIXER)
int  isEnableMix(void) { return mix_enable; }
#endif // defined(ENABLE_MIXER)
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
#if defined(ENABLE_MIXER)
    // Assign the PCM mix buffer
    mixBufLen = sizeof(pcminbuf);
    mixBuf    = (unsigned char*)pcminbuf;
#endif // defiend(ENABLE_MIXER)
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
        default                : printf("MMIO_Write(0x%04x,0x%04x). Error!\n", reg, val); break;
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
        default              : printf("MMIO_Read(0x%04x). Error!\n", reg); break;
    }

    if(!fin) return 0;

    ASSERT((fileWriteIdx >= 0) && (fileWriteIdx <= READBUF_SIZE));
    ASSERT((fileReadIdx  >= 0) && (fileReadIdx  <= READBUF_SIZE));

    if (fileReadIdx > fileWriteIdx) {
        len = fileReadIdx - fileWriteIdx - 2;
    } else {
        len = READBUF_GUARD - (fileWriteIdx - fileReadIdx) - 2;
    }

    // Limit read size foreach run
    // len = (len > 1024) ? 1024 : len;

    if (fileEOF || len < 512) {
        return (fileWriteIdx);
    }

    if (fileReadIdx > fileWriteIdx) {
        rdlen += fread(&streamBuf[fileWriteIdx+READBUF_BEGIN], sizeof(char), len, fin);
    } else {
        if (len < READBUF_GUARD - fileWriteIdx) {
            rdlen += fread(&streamBuf[fileWriteIdx+READBUF_BEGIN], sizeof(char), len, fin);
        } else {
            rdlen += fread(&streamBuf[fileWriteIdx+READBUF_BEGIN], sizeof(char), READBUF_GUARD - fileWriteIdx, fin);
            if ((len - (READBUF_GUARD-fileWriteIdx)) > 0) {
               rdlen += fread(&streamBuf[READBUF_BEGIN], sizeof(char), len - (READBUF_GUARD - fileWriteIdx), fin);
            }
        }
    }

    if (feof(fin)) {
        fileEOF = 1;
    }

    fileWriteIdx += rdlen;

    if (fileWriteIdx >= READBUF_GUARD) {
        fileWriteIdx -= READBUF_GUARD;
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
//                              Mixer Input Function
///////////////////////////////////////////////////////////////////////////
#if defined(ENABLE_MIXER)
int isMixEOF(void) {
    return feof(fp_mix);
}

void SetMixRdPtr(int rdptr){
    int rdlen = 0;
    mixRdPtr = rdptr;
    if (fp_mix != NULL) {
        int len;

        if (mixRdPtr > mixWrPtr) {
            len = mixRdPtr - mixWrPtr - 2;
        } else {
            len = mixBufLen - (mixWrPtr - mixRdPtr) - 2;
        }
        if (len <= 0) return;

        if (mixRdPtr > mixWrPtr) {
            rdlen = fread(mixBuf+mixWrPtr, sizeof(char), len, fp_mix);
        } else {
            if (mixWrPtr == 0) {
                rdlen = fread(mixBuf+mixWrPtr, sizeof(char), len, fp_mix);
            } else {
                rdlen = fread(mixBuf+mixWrPtr, sizeof(char), mixBufLen - mixWrPtr, fp_mix);
                if (mixRdPtr > 2) {
                    rdlen += fread(mixBuf, sizeof(char), mixRdPtr-2, fp_mix);
                }
            }
        }
    }

    PRINTF("Mix: RdPtr(%d), WrPtr(%d), read %d bytes.\n", mixRdPtr, mixWrPtr, rdlen);

    mixWrPtr += rdlen;
    if (mixWrPtr >= mixBufLen) {
        mixWrPtr -= mixBufLen;
    }
}

int GetMixWrPtr(void) {
    return mixWrPtr;
}
#endif // defined(ENABLE_MIXER)

///////////////////////////////////////////////////////////////////////////
//                              CODEC Related Function
///////////////////////////////////////////////////////////////////////////
void pauseDAC(int flag) { }
void deactiveDAC(void) {
    if (fin  != NULL) fclose(fin);
    if (fout != NULL) fclose(fout);

    #if defined(ENABLE_MIXER)
    if (fp_mix != NULL) fclose(fp_mix);
    #endif

    if (isWaveFile) {
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

    if (wav_out_file[0] != 0) {
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

///////////////////////////////////////////////////////////////////////////
//                              Command Line Interface
///////////////////////////////////////////////////////////////////////////
void GetParam(int argc, char **argv)
{
    if (argc < 2) {
        #if !defined(INPUT_MEMMODE)
        printf("Usage: aacdecode [-ao] [-eq] [-off] [-reverb] [-mix file.wav] file.aac [aac_out.[pcm|wav]] \n");
        #else
        printf("Usage: aacdecode [-ao] [-eq] [-off] [-reverb] [-mix file.wav] [aac_out.[pcm|wav]] \n");
        #endif // !defined(INPUT_MEMMODE)
        printf("       -ao              use audio out library.\n");
        printf("       -eq              enable equalizer.\n");
        printf("       -reverb          enable reverberation.\n");
        printf("       -off             enable voice off.\n");
        printf("       -mix file.wav    mix with pcm.\n");
        printf("\n");
        #if !defined(INPUT_MEMMODE)
        printf("        file.aac           input .aac file.\n");
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
        } else if (!strcmp(argv[0], "-mix")) {
            #if defined(ENABLE_MIXER)
            mix_enable = 1;
            argv++; argc--;
            strcpy(mixfile, argv[0]);
            if ((fp_mix = fopen(mixfile, "rb")) == NULL) {
                printf("Can not open mixfile %s.\n", mixfile);
                exit(-1);
            }
            ReadWAVHeader(fp_mix, &pcm1samps, &pcm1nchans);

            argv++; argc--;
            #else
            printf("Compile option dose not enable mixer.\n");
            exit(-1);
            #endif // defined(ENABLE_MIXER)
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

  /* ====================================================================== */
  /*  COPYRIGHT (C) 1986 Gary S. Brown.  You may use this program, or       */
  /*  code or tables extracted from it, as desired without restriction.     */
  /*                                                                        */
  /*  First, the polynomial itself and its table of feedback terms.  The    */
  /*  polynomial is                                                         */
  /*  X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0   */
  /*                                                                        */
  /*  Note that we take it "backwards" and put the highest-order term in    */
  /*  the lowest-order bit.  The X^32 term is "implied"; the LSB is the     */
  /*  X^31 term, etc.  The X^0 term (usually shown as "+1") results in      */
  /*  the MSB being 1.                                                      */
  /*                                                                        */
  /*  Note that the usual hardware shift register implementation, which     */
  /*  is what we're using (we're merely optimizing it by doing eight-bit    */
  /*  chunks at a time) shifts bits into the lowest-order term.  In our     */
  /*  implementation, that means shifting towards the right.  Why do we     */
  /*  do it this way?  Because the calculated CRC must be transmitted in    */
  /*  order from highest-order term to lowest-order term.  UARTs transmit   */
  /*  characters in order from LSB to MSB.  By storing the CRC this way,    */
  /*  we hand it to the UART in the order low-byte to high-byte; the UART   */
  /*  sends each low-bit to hight-bit; and the result is transmission bit   */
  /*  by bit from highest- to lowest-order term without requiring any bit   */
  /*  shuffling on our part.  Reception works similarly.                    */
  /*                                                                        */
  /*  The feedback terms table consists of 256, 32-bit entries.  Notes:     */
  /*                                                                        */
  /*      The table can be generated at runtime if desired; code to do so   */
  /*      is shown later.  It might not be obvious, but the feedback        */
  /*      terms simply represent the results of eight shift/xor opera-      */
  /*      tions for all combinations of data and CRC register values.       */
  /*                                                                        */
  /*      The values must be right-shifted by eight bits by the "updcrc"    */
  /*      logic; the shift must be unsigned (bring in zeroes).  On some     */
  /*      hardware you could probably optimize the shift in assembler by    */
  /*      using byte-swap instructions.                                     */
  /*      polynomial $edb88320                                              */
  /*                                                                        */
  /*  --------------------------------------------------------------------  */
#include "crc32.h"

static unsigned long crc32_tab[] = {
      0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
      0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
      0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
      0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
      0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
      0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
      0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
      0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
      0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
      0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
      0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
      0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
      0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
      0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
      0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
      0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
      0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
      0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
      0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
      0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
      0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
      0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
      0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
      0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
      0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
      0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
      0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
      0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
      0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
      0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
      0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
      0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
      0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
      0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
      0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
      0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
      0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
      0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
      0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
      0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
      0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
      0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
      0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
      0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
      0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
      0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
      0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
      0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
      0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
      0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
      0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
      0x2d02ef8dL
   };

static unsigned long crc32val = 0;
void crc32_init(void)
{
    crc32val = 0;
}

/* Return a 32-bit CRC of the contents of the buffer. */
unsigned long crc32(const unsigned char *s, unsigned int len)
{
    unsigned int i;
    for (i = 0;  i < len;  i ++) {
        crc32val = crc32_tab[(crc32val ^ s[i]) & 0xff] ^ (crc32val >> 8);
    }
    return crc32val;
}

#endif // defined(WIN32) || defined(__CYGWIN__)

