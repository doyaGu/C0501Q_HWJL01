/*
 * Copyright (c) SmediaTech
 * All rights reserved
 *
 * Filename: drc.c
 * Abstract: Dynamic range control subroutine for aac
 *
 * Current version:
 * Author: Hu-Ren-Hua
 * Date: March, 2006
 *
 * previous version: MP3 version
 * Author: Huang-Yi-Wei
 * Date:
 *
 */
#include "aacdec.h"
#include "drc.h"

#include "assembly.h"

#if defined(DRCTL)
int drcoutbuf[AAC_MAX_NCHANS * AAC_MAX_NSAMPS];

// The or32 toolchains dose not implement the 64-bits divide.
#define __USE_INT64_LIB__
#if defined(__USE_INT64_LIB__)
uint64 __div64_32(uint64 n, unsigned int base)
{
    uint64 rem = n;
    uint64 b = base;
    uint64 res, d = 1;
    unsigned int high = rem >> 32;

    /* Reduce the thing a bit first */
    res = 0;
    if (high >= base) {
        high /= base;
        res = (uint64) high << 32;
        rem -= (uint64) (high*base) << 32;
    }

    while ((int64)b > 0 && b < rem) {
        b = b+b;
        d = d+d;
    }

    do {
        if (rem >= b) {
            rem -= b;
            res += d;
        }
        b >>= 1;
        d >>= 1;
    } while (d);

    return res;
}
#endif // __USE_INT64_LIB__

/*
Fuction: initDRC
Description: initial DRC's information
Input: DRCINFO *info, //information for DRC
       sample_t *buf0, //sample buffer

       int nch, //number of channels
       int blksize, //block size: framesize
       int blkcnt, //block count: framecount
Output:
Note:
*/
void initDRC(DRCINFO * info, sample_t * buf0, int nch, int blksize,
             int blkcnt)
{
    info->buffer[0] = buf0;
    info->buffer[1] = buf0 + DECBUFSIZE;
    info->nch = nch;
    info->statenow = 1;
    info->stateprv = 0;
    info->blksize = blksize;
    info->blkcnt = blkcnt;
    info->curblk = 0;
    info->prvblk = blkcnt - 1;
}

/*
Fuction: computeDrcGain
Description: Computer the DRC gain
Input: DRCINFO *info //information for DRC
Output:
Note:
*/
static void computeDrcGain(DRCINFO * info)
{
    int i, j, minGainIdx;
    sample_t *bufptr, val, maxval;
    sample_t minGain, maxAdjGain;

    for (i = 0; i < info->nch; i++) {
        info->prvMinGain[i] = info->curMinGain[i];
        info->prvMinGainIdx[i] = info->curMinGainIdx[i];

        bufptr = &info->buffer[i][info->curblk * info->blksize];
        maxval = 0;
        minGainIdx = 0;

        for (j = 0; j < info->blksize; j++) {
            val = bufptr[j];
            if (val < 0) {
                val = -val;
            }
            if (val > maxval) {
                maxval = val;
                minGainIdx = j;
            }
        }
        //if((sample_t)maxval * info.globalGain >= 32767.0f){
        //  minGain = 32767.0 / maxval;
        if (MULSHIFT(maxval, info->globalGain, DRC_SHIFT) >= 32767) {
            #if defined(__USE_INT64_LIB__)
            minGain = (int) __div64_32(((int64) 32767 * (1 << DRC_SHIFT)), maxval);
            #else
            minGain = (int) (((int64) 32767 * (1 << DRC_SHIFT)) / maxval);
            #endif
        } else {
            minGain = info->globalGain;
        }

        maxAdjGain = info->prvMinGain[i] + info->globalGain / 4;

        if (minGain > maxAdjGain) {
            minGain = maxAdjGain;
        }

        info->curMinGain[i] = minGain;
        info->curMinGainIdx[i] = minGainIdx;
    }
}

/*
Fuction: adjustRange
Description: adjust output sample's range
Input: DRCINFO *info //information for DRC
Output:
Note:
*/
static void adjustRange(DRCINFO * info, int *outbuf)
{
    int i, j;
    sample_t *bufptr, *drcoutptr;
    sample_t dynamic_gain, delta, val;

    for (i = 0; i < info->nch; i++) {
        bufptr = &info->buffer[i][info->prvblk * info->blksize];
        drcoutptr = outbuf + (i * AAC_MAX_NSAMPS);
        dynamic_gain = info->n0gain[i];
        delta = info->n0delta[i];

        for (j = 0; j < info->prvMinGainIdx[i]; j++) {
            val = bufptr[j];
            //val = val * dynamic_gain;
            val = MULSHIFT(val, dynamic_gain, DRC_SHIFT);
            dynamic_gain += delta;
            drcoutptr[j] = val;
        }
    }

    for (i = 0; i < info->nch; i++) {
        bufptr = &info->buffer[i][info->prvblk * info->blksize];
        drcoutptr = outbuf + (i * AAC_MAX_NSAMPS);
        dynamic_gain = info->prvMinGain[i];

        delta = (info->curMinGain[i] - info->prvMinGain[i]) /
                (info->curMinGainIdx[i] + info->blksize - info->prvMinGainIdx[i]);

        for (j = info->prvMinGainIdx[i]; j < info->blksize; j++) {
            val = bufptr[j];
            val = MULSHIFT(val, dynamic_gain, DRC_SHIFT);
            dynamic_gain += delta;
            drcoutptr[j] = val;
        }

        info->n0gain[i] = dynamic_gain;
        info->n0delta[i] = delta;
    }
}

DRCINFO drcinfo;
void DRCData(DRCINFO * info, int *outbuf)
{

    int i;

#  if defined(__OR32__)
    info->statenow = ((MMIO_Read(DrvAudioCtrl) & DrvDecode_EnDRC) != 0);
#  endif // defined(__OR32__)

    //initial DRC related parameters
    if (!info->stateprv && info->statenow) {
        info->globalGain = (1 << DRC_SHIFT);    // get global gain from non-cachable memory (for driver mode)
        for (i = 0; i < AAC_MAX_NCHANS; i++) {  // reset drc statistics variable
            info->n0gain[i] = info->prvMinGain[i] = info->curMinGain[i] =
                info->globalGain;
            info->n0delta[i] = 0;
            info->prvMinGainIdx[i] = info->curMinGainIdx[i] = 0;
        }
    }
    if (info->statenow) {
        computeDrcGain(info);
        if (info->stateprv) {
            adjustRange(info, outbuf);
        }
    }

    // update index
    info->prvblk = info->curblk;
    info->curblk++;
    if (info->curblk == info->blkcnt) {
        info->curblk = 0;
    }

    info->stateprv = info->statenow;
}

#endif // DRCTL

