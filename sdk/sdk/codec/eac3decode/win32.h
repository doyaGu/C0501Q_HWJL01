/***************************************************************************
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 *
 * @file
 * Header file for Win32 porting.
 *
 * @author Kuoping Hsu
 * @version 1.0
 *
 ***************************************************************************/

#ifndef __WIN32_H__
#define __WIN32_H__

#if defined(WIN32) || defined(__CYGWIN__)

///////////////////////////////////////////////////////////////////////////
//                              Constant
///////////////////////////////////////////////////////////////////////////
#define AUDIO_DECODER_DROP_DATA 0xff
#define DrvAudioCtrl2 0xff
#define DrvResetAudioDecodedBytes 0xff
#define DrvAudioDecodedBytes 0xff


#define DrvDecode_RdPtr 0
#define DrvDecode_WrPtr 2
#define DrvAudioCtrl    4
#define DrvDecode_Frame 6
// define DrvDecode_Frame+2 8

#define DrvDecode_EOF  (1 << 0)
#define DrvDecode_STOP (1 << 1)

///////////////////////////////////////////////////////////////////////////
// Macro
///////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////
//                              Function
///////////////////////////////////////////////////////////////////////////
void or32_invalidate_cache(void *start, unsigned bytes);
void win32_init(void);
void dc_invalidate(void);
int  isEQ(void);
int  isReverbOn(void);
int  isVoiceOff(void);
int  isEOF(void);
int  isSTOP(void);
int  isPAUSE(void);
int  isDownSample(void);
void MMIO_Write(int reg, int val);
int  MMIO_Read(int reg);
void SetOutWrPtr(int val);
int  GetOutRdPtr(void);
int  GetOutWrPtr(void);
void report_error(char *str);
void or32_sleep(int ticks);
void or32_delay(int ms);
void initDAC(unsigned char *bufptr, int nChannel, int sample_rate, int buflen, int bigendian);
void GetParam(int argc, char **argv);
void pauseDAC(int flag);
void deactiveDAC(void);
void setAudioReset(void);
int isResetAudioDecodedBytes(void);
void PalSleep(int time);

FILE* GetInfoFile();
#endif // defined(WIN32) || defined(__CYGWIN__)
#endif // __WIN32_H__

