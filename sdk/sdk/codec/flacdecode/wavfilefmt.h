/***************************************************************************
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 *
 * @file
 * Header file of wave out function.
 *
 * @author Kuoping Hsu
 * @version 1.0
 *
 ***************************************************************************/

#ifndef __WAVFILEFMT_H__
#define __WAVFILEFMT_H__


#include <stdio.h>
int  TestBigEndian(void);
void WriteWAVHeader(FILE * fp, int sampRateOut, int bitsPerSample,
                    int nChans);
void UpdateWAVHeader(char *pFileName);
void ReadWAVHeader(FILE * fp, int *sampRate, int *nChans);

#endif // __WAVFILEFMT_H__
