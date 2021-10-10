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

#include <stdlib.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////
//                              Constant
///////////////////////////////////////////////////////////////////////////
#include "mmio.h"

///////////////////////////////////////////////////////////////////////////
//                              Function
///////////////////////////////////////////////////////////////////////////
void or32_invalidate_cache(void *start, unsigned bytes);
void dc_invalidate(void);
void win32_init(void);
int  isEncEOF(void);
int  isEOF(void);
int  isSTOP(void);
int  isPAUSE(void);
void MMIO_Write(int reg, int val);
int  MMIO_Read(int reg);
void SetOutWrPtr(int val);
int  GetOutRdPtr(void);
int  GetOutWrPtr(void);
void SetInRdPtr(int val);
int  GetInWrPtr(void);
int  GetInRdPtr(void);
void report_error(char *str);
void or32_sleep(int ticks);
void or32_delay(int ms);
void GetParam(int argc, char **argv);
void pauseDAC(int flag);
void deactiveDAC(void);
void pauseADC(int flag);
void deactiveADC(void);
void initDAC(unsigned char *bufptr, int nChannel, int sample_rate, int buflen, int bigendian);
void initADC(unsigned char *bufptr, int nChannel, int sample_rate, int buflen, int bigendian);

#endif // defined(WIN32) || defined(__CYGWIN__)
#endif // __WIN32_H__

