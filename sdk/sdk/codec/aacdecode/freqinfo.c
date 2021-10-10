/*
 * Copyright (c) SmediaTech
 * All rights reserved
 *
 * Filename: frequinfo.c
 * Abstract: Get Frequency information subroutine for aac
 *
 * Current version:
 * Author: Hu-Ren-Hua
 * Date: April, 2006
 *
 * previous version: MP3 version
 * Author: Huang-Yi-Wei
 * Date:
 *
 */
#include <stdlib.h>
#include "aacdec.h"
#include "coder.h"
#include "assembly.h"

#if defined(FREQINFO)

#define PRECISION 26 // Format: S.26 on psi->coef ?
static char freqinfo[FREQINFOCNT];
static const int tab[FREQINFOCNT + 1] = {
      0,  10,  21,  43,  64,  96, 128, 160, 192,  227,
    284, 341, 398, 455, 512, 612, 712, 810, 910, 1013,
   1020
};

/**************************************************************************************
 * Function:    updateFreqInfo
 *
 * Description: AAC updateFreqInfo
 *
 *
 *
 * Inputs:      sampleBuf  // frequncy data array. [1024] for long, [128] for short
 *              nchans // number of chans
 *
 * Outputs:     none
 *
 * Return:
 **************************************************************************************/
void updateFreqInfo(void *psinfo, int nchans)
{
    PSInfoBase *psi = (PSInfoBase *) (psinfo);
    int i, j;

    if (nchans == 2) { // stereo
        for (i = 0; i < FREQINFOCNT; i++) {
            int f = 0;
            for (j = tab[i]; j < tab[i + 1]; j++) {
                f += FASTABS(psi->coef[0][j]) + FASTABS(psi->coef[1][j]);
            }

            // cliping the 'f' is between 0~255.
            f = f >> (PRECISION-8);
            if (f > 255) {
                f = 255;
            }
            freqinfo[i] = f;
        }
    } else { // mono
        for (i = 0; i < FREQINFOCNT; i++) {
            int f = 0;
            for (j = tab[i]; j < tab[i + 1]; j++) {
                f  += FASTABS(psi->coef[0][j]);
            }

            // cliping the 'f' is between 0~255.
            f = f >> (PRECISION-8);
            if (f > 255) {
                f = 255;
            }
            freqinfo[i] = f;
        }
    }
}
#endif // defined(FREQINFO)

