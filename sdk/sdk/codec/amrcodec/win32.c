#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mmio.h"
#include "debug.h"
#include "win32.h"
#include "wavfilefmt.h"

/*********************************************************************
 * Status Flag
 *********************************************************************/
static int drvAudioCtrl = DrvAMR_Decode | (AMR_MR122 << DrvAMR_Mode_Bits); // Default is decode mode

/*********************************************************************
 * Input/Output File handle
 *********************************************************************/
static FILE*            fInFile;
static FILE*            fOutFile;
static char             OpFileName[128] = {0};

/*********************************************************************
 * Input Buffer (DEC)
 *********************************************************************/
static int              bIpIsEOF = 0;

static int              uiIpRdIdx = 0;
static int              uiIpWrIdx = 0;

extern unsigned char *  pDecIpBuf;
extern int              wDecIpBufLen;

/*********************************************************************
 * Ouput Buffer (DEC)
 *********************************************************************/
static int              uiOpRdIdx = 0;
static int              uiOpWrIdx = 0;

static unsigned char *  pDecOpBuf;
static int              wDecOpBufLen;

/*********************************************************************
 * Input Buffer (ENC)
 *********************************************************************/
static int              bEncIpIsEOF = 0;
static unsigned char *  pEncIpBuf;
static int              wEncIpBufLen;

/*********************************************************************
 * Ouput Buffer (ENC)
 *********************************************************************/
extern unsigned char *  pEncOpBuf;
extern int              wEncOpBufLen;

static int              bIpIsWave = 0;
static int              bOpIsWave = 0;
static int              bIsDecode = 1;

#if defined(__CYGWIN__) && defined(__USE_LIBAO__)
#include <ao/ao.h>
static int              bAudioOut = 0;
ao_device               *device;
#endif // __CYGWIN__

/*********************************************************************
 * Mixer
 *********************************************************************/
#if defined(ENABLE_MIXER)
static int              bMixEnable = 0;
static int              mixRdPtr = 0;
static int              mixWrPtr = 0;
//static int            mixBufLen = 0;
//static unsigned char  *mixBuf = NULL;
extern unsigned char    *pMixIpBuf;
extern int              wMixIpBufLen;

static FILE*            fMixFile = NULL;
static char             MixFileName[128] = {0};

static int              iPCMChans, iPCMSamps;
#endif // ENABLE_MIXER

void pauseDAC(int pause) {
}

void deactiveDAC(void) {
}

void or32_delay(int ms)
{
    int i;
    for(i=0; i<ms*1000; i++);
}

void dc_invalidate(void)
{
}

int isEncEOF(void)
{
    return (bEncIpIsEOF);
}

void setEncEOF(void)
{
    bEncIpIsEOF = 1;
}

void ClearEncIpEOF(void)
{
    bEncIpIsEOF = 0;
}

int isEncIpPAUSE(void)
{
    return 0;
}

int isEncSTOP(void)
{
    return 0;
}

int isDecIpSTOP(void)
{
    return 0;
}

void ClearDecIpSTOP(void)
{
}

int isDecIpPAUSE(void)
{
    return 0;
}

int isDecIpEOF(void)
{
    return (bIpIsEOF);
}

void ClearDecIpEOF(void)
{
    bIpIsEOF = 0;
}

void SetInRdPtr(int idx)
{
    if (!bIsDecode)
        uiIpRdIdx = idx;
}

void SetInWrPtr(int idx)
{
    if (!bIsDecode)
        uiIpWrIdx = idx;
}

int GetInWrPtr(void)
{
    if (!bIsDecode) {
        int iRSize;
        int n;

        if (uiIpRdIdx > uiIpWrIdx) {
            iRSize = uiIpRdIdx - uiIpWrIdx - 2;
        } else {
            iRSize = wEncIpBufLen - (uiIpWrIdx - uiIpRdIdx) - 2;
        }

        // Limit read size foreach run
        // iRSize = (iRSize > 1024) ? 1024 : iRSize;

        if (bEncIpIsEOF || iRSize < 512) {
            return (uiIpWrIdx);
        }

        PRINTF("Enc Input : RSize(%d) uiIpWrIdx(%d) uiIpRdIdx(%d)\n",
               iRSize, uiIpWrIdx, uiIpRdIdx);

        n = 0;
        if (uiIpRdIdx > uiIpWrIdx) {
            n += fread(&pEncIpBuf[uiIpWrIdx], sizeof(char), iRSize, fInFile);
        } else {
            if (iRSize < wEncIpBufLen - uiIpWrIdx) {
                n += fread(&pEncIpBuf[uiIpWrIdx], sizeof(char), iRSize, fInFile);
            } else {
                n += fread(&pEncIpBuf[uiIpWrIdx], sizeof(char), wEncIpBufLen-uiIpWrIdx, fInFile);
                if ((iRSize - (wEncIpBufLen-uiIpWrIdx)) > 0) {
                   n += fread(&pEncIpBuf[0], sizeof(char), iRSize - (wEncIpBufLen - uiIpWrIdx), fInFile);
                }
            }
        }

        // Check EOF
        if (feof(fInFile)) {
            bEncIpIsEOF = 1;
            drvAudioCtrl |= DrvEncodePCM_EOF;
        }

        uiIpWrIdx += n;

        if (uiIpWrIdx >= wEncIpBufLen) {
            uiIpWrIdx -= wEncIpBufLen;
        }

        return (uiIpWrIdx);
    } else {
        return 0;
    }
}

int GetOutRdPtr(void)
{
    if (bIsDecode)
        return (uiOpRdIdx);
    else
        return 0;
}

void SetOutRdPtr(int idx)
{
    if (bIsDecode)
        uiOpRdIdx = idx;
}

void SetOutWrPtr(int idx)
{
    uiOpWrIdx = idx;

    if (!bIsDecode || uiOpWrIdx == uiOpRdIdx) return;

    PRINTF("Dec Output: RSize(%d) uiOpWrIdx(%d) uiOpRdIdx(%d)\n",
            (uiOpWrIdx >= uiOpRdIdx) ? (uiOpWrIdx - uiOpRdIdx) :
                                       (wDecOpBufLen - (uiOpRdIdx - uiOpWrIdx)),
            uiOpWrIdx, uiOpRdIdx);

    if (fOutFile != NULL) {
        if (uiOpWrIdx >= uiOpRdIdx) {
            fwrite(pDecOpBuf + uiOpRdIdx, sizeof(char), uiOpWrIdx - uiOpRdIdx, fOutFile);
        } else {
            fwrite(pDecOpBuf + uiOpRdIdx, sizeof(char), wDecOpBufLen - uiOpRdIdx, fOutFile);
            fwrite(pDecOpBuf, sizeof(char), uiOpWrIdx, fOutFile);
        }
    }

    #if defined(__CYGWIN__) && defined(__USE_LIBAO__)
    if (bAudioOut) {
        if (uiOpWrIdx >= uiOpRdIdx) {
            ao_play(device, pDecOpBuf + uiOpRdIdx, uiOpWrIdx - uiOpRdIdx);
        } else {
            ao_play(device, pDecOpBuf + uiOpRdIdx, wDecOpBufLen - uiOpRdIdx);
            ao_play(device, pDecOpBuf, uiOpWrIdx);
        }
    }
    #endif // defined(__CYGWIN__) && defined(__USE_LIBAO__)

    uiOpRdIdx = uiOpWrIdx;

}

int MMIO_Read(int reg)
{
    switch(reg) {
        case DrvDecode_WrPtr:
            if (bIsDecode) {
                int iRSize;
                int n;

                if (uiIpRdIdx > uiIpWrIdx) {
                    iRSize = uiIpRdIdx - uiIpWrIdx - 2;
                } else {
                    iRSize = wDecIpBufLen - (uiIpWrIdx - uiIpRdIdx) - 2;
                }

                // Limit read size foreach run
                // iRSize = (iRSize > 1024) ? 1024 : iRSize;

                if (bIpIsEOF || iRSize < 512) {
                    return (uiIpWrIdx);
                }

                PRINTF("Dec Input : RSize(%d) uiIpWrIdx(%d) uiIpRdIdx(%d) EOF(%d)\n",
                       iRSize, uiIpWrIdx, uiIpRdIdx, bIpIsEOF);

                n = 0;
                if (uiIpRdIdx > uiIpWrIdx) {
                    n += fread(&pDecIpBuf[uiIpWrIdx], sizeof(char), iRSize, fInFile);
                } else {
                    if (iRSize < wDecIpBufLen - uiIpWrIdx) {
                        n += fread(&pDecIpBuf[uiIpWrIdx], sizeof(char), iRSize, fInFile);
                    } else {
                        n += fread(&pDecIpBuf[uiIpWrIdx], sizeof(char), wDecIpBufLen-uiIpWrIdx, fInFile);
                        if ((iRSize - (wDecIpBufLen-uiIpWrIdx)) > 0) {
                           n += fread(&pDecIpBuf[0], sizeof(char), iRSize - (wDecIpBufLen - uiIpWrIdx), fInFile);
                        }
                    }
                }

                // Check EOF
                if (feof(fInFile)) {
                    bIpIsEOF = 1;
                }

                uiIpWrIdx += n;

                if (uiIpWrIdx >= wDecIpBufLen) {
                    uiIpWrIdx -= wDecIpBufLen;
                }

                return (uiIpWrIdx);
            } else {
                return 0;
            }
        case DrvAudioCtrl:
            return (drvAudioCtrl);
            break;
        case DrvEncode_RdPtr:
            if(!bIsDecode) {
                int iRSize;

                if (!fOutFile)
                    return(uiOpRdIdx = uiOpWrIdx);

                if (uiOpWrIdx >= uiOpRdIdx) {
                    iRSize = uiOpWrIdx - uiOpRdIdx;
                } else {
                    iRSize = wEncOpBufLen - (uiOpRdIdx - uiOpWrIdx);
                }

                PRINTF("Enc Output: RSize(%d) uiOpWrIdx(%d) uiOpRdIdx(%d)\n",
                       iRSize, uiOpWrIdx, uiOpRdIdx);

                if (uiOpWrIdx >= uiOpRdIdx) {
                    fwrite(&pEncOpBuf[uiOpRdIdx], sizeof(char), iRSize, fOutFile);
                } else {
                    if (uiOpWrIdx > 0)
                        fwrite(&pEncOpBuf[0], sizeof(char), uiOpWrIdx, fOutFile);
                    if ((wEncOpBufLen - uiOpRdIdx) > 0)
                        fwrite(&pEncOpBuf[uiOpRdIdx], sizeof(char), wEncOpBufLen - uiOpRdIdx, fOutFile);
                }
                return (uiOpRdIdx = uiOpWrIdx);
            } else {
                return 0;
            }
            break;
        default:
            printf("MMIO_Read(0x%04x). Error!\n", reg);
            exit(-1);
            break;
    }
    return 0;
}

void MMIO_Write(int reg, int data)
{
    switch(reg) {
        case DrvDecode_RdPtr:
            if (bIsDecode) {
                uiIpRdIdx = data;
            }
            break;
        case DrvDecode_WrPtr:
            if (bIsDecode) {
                uiIpWrIdx = data;
            }
            break;
        case DrvEncode_Frame:
        case DrvDecode_Frame:
            break;
        case DrvEncode_Frame+2:
        case DrvDecode_Frame+2:
            fprintf(stderr, "%02d:%02d\r", (int)(data/60), (data%60));
            #if defined(__CYGWIN__)
                fflush(stderr);
            #endif
            break;
        case DrvAudioCtrl:
            drvAudioCtrl = data;
            break;
        case DrvEncode_WrPtr:
            if (!bIsDecode) {
                uiOpWrIdx = data;
            }
            break;
        case DrvEncode_RdPtr:
            if (!bIsDecode) {
                uiOpRdIdx = data;
            }
            break;
        default:
            printf("MMIO_Write(0x%04x). Error!\n", reg);
            exit(-1);
            break;
    }
}

#if defined(ENABLE_MIXER)
int isMixEOF(void) {
    return feof(fMixFile);
}

void ClearMixEOF()
{
}

int SetMixRdPtr(int rdptr){
    int rdlen = 0;
    mixRdPtr = rdptr;
    if (fMixFile != NULL) {
        int len;
        //if (feof(fMixFile)) return -1;

        if (mixRdPtr > mixWrPtr) {
            len = mixRdPtr - mixWrPtr - 2;
        //    if (len > 0) {
        //        rdlen = fread(pMixIpBuf+mixWrPtr, sizeof(char), len, fMixFile);
        //    }
        //} else if (mixRdPtr == mixWrPtr) {
        //    int len = wMixIpBufLen - (mixWrPtr - mixRdPtr) - 2;
        //    if (len - mixWrPtr > 0) {
        //        rdlen  = fread(pMixIpBuf+mixWrPtr, sizeof(char), len - mixWrPtr, fMixFile);
        //      //rdlen  = fread(pMixIpBuf+mixWrPtr, sizeof(char), wMixIpBufLen - mixWrPtr, fMixFile);
        //        if (mixRdPtr > 2) {
        //            rdlen += fread(pMixIpBuf, sizeof(char), mixRdPtr-2, fMixFile);
        //        }
        //    }
        } else {
            len = wMixIpBufLen - (mixWrPtr - mixRdPtr) - 2;
        //    if (len > 0) {
        //        //rdlen  = fread(pMixIpBuf+mixWrPtr, sizeof(char), len - mixWrPtr, fMixFile);
        //      rdlen  = fread(pMixIpBuf+mixWrPtr, sizeof(char), wMixIpBufLen - mixWrPtr, fMixFile);
        //        if (mixRdPtr > 2) {
        //            rdlen += fread(pMixIpBuf, sizeof(char), mixRdPtr-2, fMixFile);
        //        }
        //    }
        }

        if (len <= 0) return 0;

        if (mixRdPtr > mixWrPtr) {
            rdlen = fread(pMixIpBuf+mixWrPtr, sizeof(char), len, fMixFile);
        } else {
            if (mixWrPtr == 0) {
                rdlen = fread(pMixIpBuf+mixWrPtr, sizeof(char), len, fMixFile);
            } else {
                rdlen = fread(pMixIpBuf+mixWrPtr, sizeof(char), wMixIpBufLen - mixWrPtr, fMixFile);
                if (mixRdPtr > 2) {
                    rdlen += fread(pMixIpBuf, sizeof(char), mixRdPtr-2, fMixFile);
                }
            }
        }

    }

    PRINTF("Mix: RdPtr(%d), WrPtr(%d), read %d bytes.\n", mixRdPtr, mixWrPtr, rdlen);

    mixWrPtr += rdlen;
    if (mixWrPtr >= wMixIpBufLen) {
        mixWrPtr -= wMixIpBufLen;
    }
    return (rdlen);
}

int GetMixWrPtr(void) {
    return mixWrPtr;
}

int isEnableMix(void) {
    return bMixEnable;
}

void GetMixPCMParam(int* sample, int* channel)
{
    *sample = iPCMSamps;
    *channel = iPCMChans;
}
#endif // defined(ENABLE_MIXER)

void win32_init(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: amrcodec [-enc [-m mode] [-dtx 0|1] | -dec] [-ao] [-mix file.wav] file_in.[amr|pcm|wav] [file_out.[amr|pcm|wav]] \n");
        printf("       -ao              use audio out library (decoder only).\n");
        printf("       -enc|-dec        encoding or decoding mode\n");
        printf("           -m mode      encoded mode (for encoding only)\n\n");
        printf("                        mode 0: 475\n");
        printf("                        mode 1: 515\n");
        printf("                        mode 2: 59\n");
        printf("                        mode 3: 67\n");
        printf("                        mode 4: 74\n");
        printf("                        mode 5: 795\n");
        printf("                        mode 6: 102\n");
        printf("                        mode 7: 122 (default)\n\n");
        printf("           -dtx [0|1]   enable DTX (for encoding only)\n");
        printf("       -mix file.wav    mix with pcm.\n");
        printf("\n");
        printf("        file_in.[amr|wav|pcm]   input file.\n");
        printf("        file_out.[amr|wav|pcm]  output file.\n");
        printf("\n");
        printf("Example: amrcodec -ao in.amr out.wav\n");
        exit(-1);
    }

    argv++; argc--;
    while(argc) {
        if (!strcmp(argv[0], "-mix")) {
            #if defined(ENABLE_MIXER)
            bMixEnable = 1;
            argv++; argc--;
            strcpy(MixFileName, argv[0]);
            if ((fMixFile = fopen(MixFileName, "rb")) == NULL) {
                printf("Can not open MixFileName %s.\n", MixFileName);
                exit(-1);
            }
            ReadWAVHeader(fMixFile, &iPCMSamps, &iPCMChans);

            argv++; argc--;
            #else
            printf("Compile option dose not enable mixer.\n");
            exit(-1);
            #endif // defined(ENABLE_MIXER)
        } else if (!strcmp(argv[0], "-dec")) {
            drvAudioCtrl = (drvAudioCtrl & ~DrvAMR_Type) | DrvAMR_Decode;
            bIsDecode    = 1;
            argv++; argc--;
        } else if (!strcmp(argv[0], "-enc")) {
            drvAudioCtrl = (drvAudioCtrl & ~DrvAMR_Type) | DrvAMR_Encode;
            bIsDecode    = 0;
            argv++; argc--;
        } else if (!strcmp(argv[0], "-m")) {
            int m = 1;
            if (bIsDecode != 0) {
                printf("Error!! -m option is only for encoder\n");
                exit(-1);
            }
            argv++; argc--;
            m = atoi(argv[0]);
            drvAudioCtrl = (drvAudioCtrl & ~DrvAMR_Mode) | (m << DrvAMR_Mode_Bits);
            argv++; argc--;
        } else if (!strcmp(argv[0], "-dtx")) {
            int m = 1;
            if (bIsDecode != 0) {
                printf("Error!! -dtx option is only for encoder\n");
                exit(-1);
            }
            argv++; argc--;
            m = atoi(argv[0]);
            if (m != 0 && m != 1) {
                printf("Error!! DTX mode should be 0 or 1.\n");
                exit(-1);
            }
            drvAudioCtrl = (drvAudioCtrl & ~DrvAMR_DTX) | (m << DrvAMR_DTX_Bits);
            argv++; argc--;
        } else if (!strcmp(argv[0], "-ao")) {
            #if defined(__CYGWIN__) && defined(__USE_LIBAO__)
            if (!bIsDecode) {
                printf("Warning: there is no audio output on encoding mode!\n");
                bAudioOut = 0;
            } else {
                bAudioOut = 1;
            }
            argv++; argc--;
            #else
            printf("Compile option does not enable audio out, it requires libao library.\n");
            exit(-1);
            #endif // defined(__CYGWIN__) && defined(__USE_LIBAO__)
        } else {
            int i;
            if (argv[0][0] == '-') {
                printf("Unknown option '%s'\n", argv[0]);
                exit(-1);
            }

            for(i=strlen(argv[0])-1; argv[0][i] != '.' && i > 0; i--) ;
            if(bIsDecode) {
                if (strcmp(&argv[0][i], ".amr")) {
                    printf("The input file should be .amr in decoding mode.\n");
                    exit(-1);
                }
            } else {
                if (strcmp(&argv[0][i], ".wav") && strcmp(&argv[0][i], ".pcm")) {
                    printf("The input file should be .wav or .pcm in encoding mode.\n");
                    exit(-1);
                }
                bIpIsWave = (i==0) ? 0 : (!strcmp(&argv[0][i], ".wav")) ? 1 : 0;
            }

            if ((fInFile = fopen(argv[0], "rb")) == NULL) {
                printf("Can not open file '%s'.\n", argv[0]);
                exit(-1);
            }

            if (bIpIsWave && !bIsDecode) {
                int s, ch;
                ReadWAVHeader(fInFile, &s, &ch);
                if (s != 8000) {
                    printf("Input .wav is not with 8K sampling rate\n");
                    exit(-1);
                }
            }

            argv++; argc--;
            if (argc > 0) {
                strcpy(OpFileName, argv[0]);
                argv++; argc--;
            }

            if (OpFileName[0] != '-' && OpFileName[0] != 0) {
                for(i=strlen(OpFileName)-1; OpFileName[i] != '.' && i > 0; i--) ;

                if(bIsDecode) {
                    if (strcmp(&OpFileName[i], ".wav") && strcmp(&OpFileName[i], ".pcm")) {
                        printf("The output file should be .wav or .pcm to decode.\n");
                        exit(-1);
                    }
                } else {
                    if (strcmp(&OpFileName[i], ".amr")) {
                        printf("The output file should be .amr for encoding.\n");
                        exit(-1);
                    }
                }

                bOpIsWave = (i==0) ? 0 : (!strcmp(&OpFileName[i], ".wav")) ? 1 : 0;

                if ((fOutFile = fopen(OpFileName, "wb")) == NULL) {
                    printf("Can not create file '%s'\n", OpFileName);
                    exit(-1);
                }

                if(!bIsDecode) {
                    fwrite("#!AMR\n", sizeof(char), 6, fOutFile);
                }
            } else {
                fOutFile = NULL;
            }
        }
    }
}

void win32_destory()
{

    fclose(fInFile);
    fclose(fOutFile);
#if defined(__CYGWIN__) && defined(__USE_LIBAO__)
    if (bAudioOut) {
        ao_close(device);
        ao_shutdown();
    }
#endif
}

void initADC(
    unsigned char *inbuf,
    int channel,
    int sample,
    int inbuflen,
    int overhead)
{
    pEncIpBuf    = inbuf;
    wEncIpBufLen = inbuflen;
}

void initDAC(
    unsigned char *outbuf,
    int channel,
    int sample,
    int outbuflen,
    int overhead)
{
#if defined(__CYGWIN__) && defined(__USE_LIBAO__)
    ao_sample_format format;

    if (bAudioOut) {
        ao_initialize();

        /* -- Setup for default driver -- */
        format.bits        = 16;
        format.channels    = channel;
        format.rate        = sample;
        format.byte_format = AO_FMT_LITTLE;

        fprintf(stderr, "Open Audio Device with %d sample rate, and %d channel.\n",
                format.rate, format.channels);

        /* -- Open driver -- */
        device = ao_open_live(ao_default_driver_id(), &format, NULL /* no options */);
        if (device == NULL) {
            fprintf(stderr, "Error opening device.\n");
            exit(-1);
        }
    }
#endif

    pDecOpBuf    = outbuf;
    wDecOpBufLen = outbuflen;
}

void initCODEC(
    unsigned char *inbuf,
    unsigned char *outbuf,
    int channel,
    int sample,
    int inbuflen,
    int outbuflen,
    int overhead)
{
#if defined(__CYGWIN__) && defined(__USE_LIBAO__)
    ao_sample_format format;

    if (bAudioOut) {
        ao_initialize();

        /* -- Setup for default driver -- */
        format.bits        = 16;
        format.channels    = channel;
        format.rate        = sample;
        format.byte_format = AO_FMT_LITTLE;

        fprintf(stderr, "Open Audio Device with %d sample rate, and %d channel.\n",
                format.rate, format.channels);

        /* -- Open driver -- */
        device = ao_open_live(ao_default_driver_id(), &format, NULL /* no options */);
        if (device == NULL) {
            fprintf(stderr, "Error opening device.\n");
            exit(-1);
        }
    }
#endif

    pDecOpBuf    = outbuf;
    wDecOpBufLen = outbuflen;

    pEncIpBuf    = inbuf;
    wEncIpBufLen = inbuflen;
}

/*
 * $Id: crc32.c,v 1.1.1.1 1996/02/18 21:38:12 ylo Exp $
 * $Log: crc32.c,v $
 * Revision 1.1.1.1  1996/02/18 21:38:12  ylo
 *  Imported ssh-1.2.13.
 *
 * Revision 1.2  1995/07/13  01:21:34  ylo
 *  Added cvs log.
 *
 * $Endlog$
 */

/* The implementation here was originally done by Gary S. Brown.  I have
   borrowed the tables directly, and made some minor changes to the
   crc32-function (including changing the interface). //ylo */

#include "crc32.h"

  /* ============================================================= */
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

