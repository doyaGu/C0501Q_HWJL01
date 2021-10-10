/*
 * Copyright (c) SmediaTech
 * All rights reserved
 *
 * Filename: reverb.h
 * Abstract: Header file for reverb
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
#ifndef _REVERB_H
#  define _REVERB_H

#  include "aacdec.h"

#  if defined(REVERBERATION)

extern int reverbStateNow;
extern int reverbStatePrev;

#    define ALLPASS_FILTER
#    define MAX_DELAY_FRAMES        4
#    define MAX_FILTER_BUFFRAMES    (MAX_DELAY_FRAMES+1)
#    define MAX_FILTER_BUFSIZE      (MAX_FILTER_BUFFRAMES * AAC_MAX_NSAMPS)
                                                                        //5*1024
#    define DECBUFFRAMES            MAX_FILTER_BUFFRAMES
//#    if defined(AAC_ENABLE_SBR)
//#      define DECBUFSIZE            (MAX_FILTER_BUFSIZE*2)
//#    else
#      define DECBUFSIZE            MAX_FILTER_BUFSIZE
//#    endif
typedef int data_t;

typedef struct _REVERBINFO {
    short int updReverb;
    short int endianReverb;
    int src_gain;
    int reverb_gain;
    int cross_gain;
    int filter[2][8];
} REVERBINFO;

#    if defined(REVERB_DYNAMIC_MALLOC)
extern data_t *reverbbuf;
#    else
extern data_t reverbbuf[AAC_MAX_NCHANS * DECBUFSIZE];
#    endif
MMP_INLINE int init_reverb_memory();
MMP_INLINE void release_reverb_memory();
MMP_INLINE void init_reverb_filter(int nch, int sr, int framesize, int bufindex,
                                   int bufsize);
MMP_INLINE void reverb_filter(int nChans, int framesize, int bufidx, int bufsize);
#  elif defined DRCTL           //no-reverberation, but DRC
#    define DECBUFFRAMES        2
//#    if defined(AAC_ENABLE_SBR)
//#      define DECBUFSIZE        (DECBUFFRAMES * AAC_MAX_NSAMPS * 2)
//#    else
#      define DECBUFSIZE        (DECBUFFRAMES * AAC_MAX_NSAMPS)
//#    endif
#  else                         // no-reverb and no DRC
#    define DECBUFFRAMES        1
//#    if defined(AAC_ENABLE_SBR)
//#      define DECBUFSIZE        (DECBUFFRAMES * AAC_MAX_NSAMPS * 2)
//#    else
#      define DECBUFSIZE        (DECBUFFRAMES * AAC_MAX_NSAMPS)
//#    endif
#  endif // REVERBERATION

#  if defined(REVERBERATION) || defined(DRCTL)
extern int decbuf[AAC_MAX_NCHANS * DECBUFSIZE]; // temp for drc buffer
#  endif

#endif // _REVERB_H
