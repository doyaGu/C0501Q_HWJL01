/**************************************************************************************
 * Source last modified: $Id: main.c,v 1.2 2005/12/15 14:45:30 $
 *
 * Copyright (c) 1995-2005 ITE Tech. Corp., All Rights Reserved.
 *
 **************************************************************************************/

#ifndef __WIN32_H__
#define __WIN32_H__


#define WIN32

#if defined(WIN32) || defined(__CYGWIN__)

///////////////////////////////////////////////////////////////////////////
// Constant
///////////////////////////////////////////////////////////////////////////
#define DrvDecode_RdPtr 0
#define DrvDecode_WrPtr 2
#define DrvAudioCtrl    4
#define DrvDecode_Frame 6
// define DrvDecode_Frame+2 8
#define DrvDecode_RdPtr2 10
#define DrvDecode_WrPtr2 12

#define DrvEncode_RdPtr 14
#define DrvEncode_WrPtr 16

#define DrvDecode_PAUSE_Bits      0
#define DrvDecode_EOF_Bits        1
#define DrvDecode_EnEQ_Bits       2
#define DrvDecode_EnDRC_Bits      3
#define DrvDecode_EnReverb_Bits   4
#define DrvDecode_EnVoiceOff_Bits 5
#define DrvDecode_STOP_Bits       6
#define DrvDecode_EnMix_Bits      7
#define DrvDecode_MixEOF_Bits    10

/* Control bit for Audio Out */
#define DrvDecode_PAUSE      (1 << DrvDecode_PAUSE_Bits)      // D[0]
#define DrvDecode_EOF        (1 << DrvDecode_EOF_Bits)        // D[1]
#define DrvDecode_EnEQ       (1 << DrvDecode_EnEQ_Bits)       // D[2]
#define DrvDecode_EnDRC      (1 << DrvDecode_EnDRC_Bits)      // D[3]
#define DrvDecode_EnReverb   (1 << DrvDecode_EnReverb_Bits)   // D[4]
#define DrvDecode_EnVoiceOff (1 << DrvDecode_EnVoiceOff_Bits) // D[5]
#define DrvDecode_STOP       (1 << DrvDecode_STOP_Bits)       // D[6]
#define DrvDecode_EnMix      (1 << DrvDecode_EnMix_Bits)      // D[7]
#define DrvDecode_MixEOF     (1 << DrvDecode_MixEOF_Bits)     // D[10]

#define AUDIO_DECODER_DROP_DATA
///////////////////////////////////////////////////////////////////////////
// Function
///////////////////////////////////////////////////////////////////////////
void win32_init(void);
void or32_sleep(int ticks);
void or32_delay(int ms);
void or32_invalidate_cache(void *start, unsigned bytes);
void dc_invalidate(void);
void MMIO_Write(int reg, int val);
int  MMIO_Read(int reg);
void SetOutWrPtr(int val);
int  GetOutRdPtr(void);
int  GetOutWrPtr(void);
void report_error(char *str);
void initDAC(unsigned char *bufptr, int nChannel, int sample_rate, int buflen, int bigendian);
void pauseDAC(int pause);
void GetParam(int argc, char **argv);
void deactiveDAC();
void *memset16 (void *dst, const int c, unsigned int num);
void *memset32 (void *dst, const int c, unsigned int num);
int  isEQ(void);
int  isReverbOn(void);
int  isDrcOn(void);
int  isVoiceOff(void);
int  isEOF(void);
int  isSTOP(void);
int  isPAUSE(void);
int  isDownSample(void);
void clrDecSTOP();
void clrDecPAUSE();

#endif // defined(WIN32) || defined(__CYGWIN__)

#endif // __WIN32_H__

