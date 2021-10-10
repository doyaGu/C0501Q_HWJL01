/*
 * Copyright (c) SmediaTech
 * All rights reserved
 *
 * Filename: drc.h
 * Abstract: Header file for drc.c
 *
 * Current version:
 * Author: Hu-Ren-Hua
 * Date: March, 2006
 *
 * previous version:
 * Author:
 * Date:
 *
 */
#ifndef __DRC_H__
#  define __DRC_H__

#  include "aacdec.h"
#  include "reverb.h"

typedef int sample_t;           // gain and delta use S3.28 fixed point format

#  define DRC_SHIFT       28
#  define MAXBLOCKS       2
#  define BLOCKSIZE       AAC_MAX_NCHANS * AAC_MAX_NSAMPS
#  define SCALE           32768

typedef struct _DRCINFO {
    sample_t *buffer[AAC_MAX_NCHANS];
    int blksize;
    int blkcnt;
    int nch;
    int statenow, stateprv;     // 0/1: disable/enable
    int prvblk, curblk;
    int prvMinGainIdx[AAC_MAX_NCHANS], curMinGainIdx[AAC_MAX_NCHANS];
    sample_t prvMinGain[AAC_MAX_NCHANS], curMinGain[AAC_MAX_NCHANS];
    sample_t n0gain[AAC_MAX_NCHANS], n0delta[AAC_MAX_NCHANS];
    sample_t globalGain;
} DRCINFO;

extern DRCINFO drcinfo;
extern int drcoutbuf[AAC_MAX_NCHANS * AAC_MAX_NSAMPS];
/*MMP_INLINE*/ void initDRC(DRCINFO * info, sample_t * buf0, int nch, int blksize,
                        int blkcnt);
/*MMP_INLINE*/ void DRCData(DRCINFO * info, int *outbuf);

#endif // __DRC_H__

