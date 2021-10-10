/*
 * Copyright (c) SmediaTech
 * All rights reserved
 *
 * Filename: reverb.c
 * Abstract: Rerverberation subroutine for aac
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
// buffer connection:
//       mpdaac   reverb    reverb      drc       drc         i2s
//       outputbuf = inputbuf  outputbuf = inputbuf  outputbuf = buf
// SIZE  DECBUFFRAMES          DECBUFFRAMES          I2SBUFFRAMES
// seperate drc outputbuf with inputbuf to avoid
// conflict between reverberation in[n] = in[n-m] + out[n-m] with drc output write
//
// *****************************************************************************************
// !! before calling init_reverb_filter(), one should init the following global variables !!
// *****************************************************************************************
// int decbufIdx;
// int decbufsize;

#include <stdio.h>

#include "aacdec.h"
#include "reverb.h"
#include "drc.h"
#include "assembly.h"

#if defined(WIN32) || defined(__CYGWIN__)
#include <malloc.h>
#endif // defined(WIN32) || defined(__CYGWIN__)

#if defined(REVERBERATION)
#  define NCOMB           3
#  define NALLPASS        1
#  define REVERB_PRECISION    28
#  define DATA_CONST(A)       ((data_t)(((A)*(1<<REVERB_PRECISION)+0.5)))
#  define MUL(x,coef)         MULSHIFT(x, coef, REVERB_PRECISION)

#define TO_INT(j) (((((j)>> 0)&0xff) << 24) + ((((j)>> 8)&0xff) << 16) + \
                   ((((j)>>16)&0xff) <<  8) + ((((j)>>24)&0xff) <<  0) )

//#  define REVERB_GAIN     DATA_CONST(0.5)
//#  define SRC_GAIN        DATA_CONST(0.5)

#if defined(WIN32) || defined(__CYGWIN__)
REVERBINFO revbinfo = {
    1, 1,
    DATA_CONST(0.5),
    DATA_CONST(0.5),
    DATA_CONST(0.0),
    { { 15, DATA_CONST(0.35), 40, DATA_CONST(0.25), 00, DATA_CONST(0.00), 30, DATA_CONST(0.45) } ,
      { 15, DATA_CONST(0.35), 40, DATA_CONST(0.25), 00, DATA_CONST(0.00), 30, DATA_CONST(0.45) }
    }
};
#else
    extern REVERBINFO revbinfo;
#endif  // defined(WIN32) || defined(__CYGWIN__)

typedef struct _filter_info {
    // variables which doesn't change with sample rate
    // filter characters
    int delaytime;              // in msec
    data_t gain;
    data_t *inbuf;              // +: input circular buffer pointer (must be set before it used !!)

    // variables which change with sample rate
    int delaysamp;              // delay samples (=sr * delaytime / 1000)
#  if defined(REVERB_DYNAMIC_MALLOC)
    data_t *outbuf;
#  else
    data_t outbuf[AAC_MAX_NCHANS * DECBUFSIZE];
#  endif
} filter_info;

//int dec_nch;
//int dec_framesize;

// declare filter and buffer for reverberation
filter_info combinfo[NCOMB];

#  if defined(ALLPASS_FILTER)
filter_info allpassinfo[NALLPASS];
#  endif

#  if defined(REVERB_DYNAMIC_MALLOC)
data_t *reverbbuf = NULL;
data_t *work2buf  = NULL;
data_t *allocmem  = NULL;
#  else
data_t reverbbuf[AAC_MAX_NCHANS * DECBUFSIZE];
data_t work2buf[AAC_MAX_NCHANS * DECBUFSIZE];
#  endif // defined(REVERB_DYNAMIC_MALLOC)

// variables which doesn't change with sample rate should be set outside this function
// as input of init_comb_filter(...)
//
// (X) if we change delay when playing, we should wait idx = 0,
//     then adjust buffer size ?
// (O) if we set the bufsize to (floor(MAX_COMB_DELAY/info->framesize)+1) * framesize,
//     we don't need to ajust bufsize when changing delay
//     and adjust delaytime, delaysamp, idx_m,
//     clear outbuf[idx-idx_m .. idx-1] because clear old output doesn't affect current output
//     and sample in inbuf[idx-idx_m .. idx-1] is correct, no need to clear
// (*) One should clear input buffer & output buffer manually
static void init_filter(filter_info * info, int sr, int delaytime,
                        data_t gain, data_t * inbuf, int framesize)
{
    info->delaytime = delaytime;
    info->gain      = gain;
    info->inbuf     = inbuf;
    info->delaysamp = sr * delaytime / 1000;

    if(info->delaysamp > (AAC_MAX_NSAMPS * (MAX_DELAY_FRAMES-1))){
        info->delaysamp = (AAC_MAX_NSAMPS * (MAX_DELAY_FRAMES-1));
    }
}

/*
Fuction: init_reverb_memory
Description: declare momory for reverberation
Input:
Output:
Note:
DECBUFSIZE = MAX_FILTER_BUFSIZE = 5*1024
AAC_MAX_NCHANS = 2
NCOMB = 2
NALLPASS = 1,
*/
int init_reverb_memory()
{
#  if defined(REVERB_DYNAMIC_MALLOC)
//    int ch;
    data_t *bufptr = NULL;
#    if defined(WIN32) || defined(__CYGWIN__)
    bufptr = allocmem = (data_t *) malloc(DECBUFSIZE * AAC_MAX_NCHANS * (NCOMB + NALLPASS + 1 + 1) * sizeof(data_t));   // +1 <=reverbbuf  ,+1 <=workbuf
    if (!allocmem) {
        printf("memory insufficient for reverberation\n");
        return 0;               // fail initializing reverb filter
    }
#    else
    bufptr = allocmem =
        (data_t *) ((((unsigned int)MMIO_Read(DrvDecode_RevbBase))&0xffff) +
                    (((unsigned int)MMIO_Read(DrvDecode_RevbBase + 2)) << 16));
    PRINTF("Reverb Base: %d\n", (int)bufptr);
    if (!allocmem) {
        PRINTF("allocmem = %x\n", (int)allocmem);
        //report_error("memory insufficient for reverberation\n");
        return 0;               // fail initializing reverb filter
    }
#    endif
    {
        int i;
        for(i=0;i<NCOMB;i++){
        combinfo[i].outbuf = bufptr;
        bufptr += (AAC_MAX_NCHANS * DECBUFSIZE);
        }
        /*combinfo[1].outbuf = bufptr;
        bufptr += (AAC_MAX_NCHANS * DECBUFSIZE);
        combinfo[2].outbuf = bufptr;
        bufptr += (AAC_MAX_NCHANS * DECBUFSIZE);*/
        for(i=0;i<NALLPASS;i++){
        allpassinfo[i].outbuf = bufptr;
        bufptr += (AAC_MAX_NCHANS * DECBUFSIZE);
        }
    }
    work2buf = bufptr;
    bufptr += (AAC_MAX_NCHANS * DECBUFSIZE);
    reverbbuf = bufptr;
#  endif
    return 1;                   // success initializing reverb filter
}

/*
Fuction: release_reverb_memory
Description: free momory for reverberation
Input:
Output:
*/
void release_reverb_memory()
{
#  if defined(REVERB_DYNAMIC_MALLOC)
#    if defined(WIN32) || defined(__CYGWIN__)
    free(allocmem);
#    endif
#  endif
}

/*
Fuction: init_reverb_filter
Description: initalize filter for reverberation
Input: int nch(number of channels)
       int sr(sample rate)
       int framesize(samples per frame)
       int bufindex(index in buffers)
       int bufsize(the size of the buffer) = (c * framesize)
   __________________________________
  |  AB  | BC   |decode| DE   | EA   |
  |      |      |Data  |      |      |
  |______|______|______|______|______|
  A      B      C      D      E
  When index is in C, you must clear AB, BC, DE and EA.

Output:
*/
void init_reverb_filter(int nch, int sr, int framesize, int bufindex,
                        int bufsize)
{
    int i, ch;
    int idx;

//  dec_nch = nch;
//  dec_framesize = framesize;
    or32_invalidate_cache(&revbinfo, sizeof(revbinfo));
    if (revbinfo.endianReverb != 1) {
        unsigned int *ptr = (unsigned int *)&revbinfo.filter;
        revbinfo.endianReverb = 1;
        revbinfo.src_gain     = TO_INT(revbinfo.src_gain);
        revbinfo.reverb_gain  = TO_INT(revbinfo.reverb_gain);
        revbinfo.cross_gain   = TO_INT(revbinfo.cross_gain);
        for(i=0; i<sizeof(revbinfo.filter)/sizeof(int); i++) {
            ptr[i] = TO_INT(ptr[i]);
        }
    }

    // global clear buffer
    for (ch = 0; ch < nch; ch++) {
        for (i = 0; i < bufindex; i++) {
            reverbbuf[i + (ch * DECBUFSIZE)] = 0;
            for(idx=0;idx<NCOMB;idx++)
                combinfo[idx].outbuf[i + (ch * DECBUFSIZE)] = 0;
            /*combinfo[1].outbuf[i + (ch * DECBUFSIZE)] = 0;
            combinfo[2].outbuf[i + (ch * DECBUFSIZE)] = 0;*/
            work2buf[i + (ch * DECBUFSIZE)] = 0;
            for(idx=0;idx<NALLPASS;idx++)
                allpassinfo[idx].outbuf[i + (ch * DECBUFSIZE)] = 0;
            decbuf[i + (ch * DECBUFSIZE)] = 0;
        }
        for (i = bufindex + framesize; i < bufsize; i++) {
            reverbbuf[i + (ch * DECBUFSIZE)] = 0;
            for(idx=0;idx<NCOMB;idx++)
                combinfo[idx].outbuf[i + (ch * DECBUFSIZE)] = 0;
            /*combinfo[1].outbuf[i + (ch * DECBUFSIZE)] = 0;
            combinfo[2].outbuf[i + (ch * DECBUFSIZE)] = 0;*/
            work2buf[i + (ch * DECBUFSIZE)] = 0;
            for(idx=0;idx<NALLPASS;idx++)
                allpassinfo[idx].outbuf[i + (ch * DECBUFSIZE)] = 0;
            decbuf[i + (ch * DECBUFSIZE)] = 0;
        }
    }

    // Init comb filter
    //for(ch=0; ch<nch; ch++){
    for(idx=0;idx<NCOMB;idx++){
        init_filter(&combinfo[idx], sr, revbinfo.filter[0][idx*2], revbinfo.filter[0][idx*2+1], &reverbbuf[0],
                framesize);
    }
    /*
    init_filter(&combinfo[1], sr, 80, DATA_CONST(0.50), &reverbbuf[0],
                framesize);
    init_filter(&combinfo[2], sr, 40, DATA_CONST(0.20), &reverbbuf[0],
                framesize);*/

#  if defined(ALLPASS_FILTER)
    // cominfo[ch][n].outbuf[] => work2buf[ch][]
    init_filter(&allpassinfo[0], sr, revbinfo.filter[0][6], revbinfo.filter[0][7], &work2buf[0],
                framesize);
#  endif
    //}
}

//  ** because decbufIdx always jump in dec_framesize, there is impossible for n to loop back to 0 in comb_filter(...)
//  comb_filter precedure
//  1. fill input buffer
//  2. perform filtering
//  3. update framenum
//  implement:
//  y[n] = x[n-m] + gy[n-m];
static void comb_filter(filter_info * info, int ch, int framesize,
                        int bufidx, int bufsize)
{
    int cnt0, cnt1;
    int n, n_m;
    int i;

    data_t g;
    data_t *inbuf, *outbuf;
    g = info->gain;

    n = bufidx;
    n_m = bufidx - info->delaysamp;
    if (n_m < 0)
        n_m += bufsize;

    inbuf = info->inbuf + ch * DECBUFSIZE;
    outbuf = info->outbuf + ch * DECBUFSIZE;
//  original code
//  for(i=0; i<dec_framesize; i++){
//      outbuf[n] = inbuf[n_m] + MUL(g, outbuf[n_m]);
//      n++; n_m++;
//      if(n == decbufsize) n = 0;
//      if(n_m == decbufsize) n_m = 0;
//  }

    if (n_m + framesize >= bufsize) {
        cnt0 = bufsize - n_m;
        cnt1 = framesize - cnt0;
    } else {
        cnt0 = framesize;
        cnt1 = 0;
    }

    for (i = 0; i < cnt0; i++) {
        outbuf[n + i] = inbuf[n_m + i] + MUL(g, outbuf[n_m + i]);
    }

    if (cnt1) {
        n += cnt0;
        for (i = 0; i < cnt1; i++) {
            outbuf[n + i] = inbuf[0 + i] + MUL(g, outbuf[0 + i]);
        }
    }

    // we update idx after output samples in outbuf is retrieved...
}

//  allpass_filter precudure is the same as comb_filter
//  implement:
//  y[n] = x[n-m] + gy[n-m] - gx[n];
static void allpass_filter(filter_info * info, int ch, int framesize,
                           int bufidx, int bufsize)
{
    int cnt0, cnt1;
    int n, n_m;
#  if !defined(REVERB_ENGINE)
    int i;
#  endif
    data_t g;
    data_t *inbuf, *outbuf;
    g = info->gain;

    n = bufidx;
    n_m = bufidx - info->delaysamp;
    if (n_m < 0)
        n_m += bufsize;

    inbuf = info->inbuf + (ch * DECBUFSIZE);
    outbuf = info->outbuf + +(ch * DECBUFSIZE);
//  for(i=0; i<dec_framesize; i++){
//      outbuf[n] = inbuf[n_m] + MUL(g, (outbuf[n_m] - inbuf[n]));
//      n++; n_m++;
//      if(n == decbufsize) n = 0;
//      if(n_m == decbufsize) n_m = 0;
//  }

    if (n_m + framesize >= bufsize) {
        cnt0 = bufsize - n_m;
        cnt1 = framesize - cnt0;
    } else {
        cnt0 = framesize;
        cnt1 = 0;
    }

    for (i = 0; i < cnt0; i++) {
        //  outbuf[n+i] = inbuf[n_m+i] + MUL(g, (outbuf[n_m+i] - inbuf[n+i]));
        outbuf[n + i] =
            inbuf[n_m + i] + MSUBSHIFT(outbuf[n_m + i], g, inbuf[n + i], g,
                                       REVERB_PRECISION);
    }
    if (cnt1) {
        n += cnt0;
        for (i = 0; i < cnt1; i++) {
            //  outbuf[n+i] = inbuf[0+i] + MUL(g, (outbuf[0+i] - inbuf[n+i]));
            outbuf[n + i] =
                inbuf[0 + i] + MSUBSHIFT(outbuf[0 + i], g, inbuf[n + i], g,
                                         REVERB_PRECISION);
        }
    }

    // we update idx after output samples in outbuf is retrieved...
}

/*
Fuction: reverb_filter
Description: do reverberation
Input: int nChans(number of channels)
       int framesize(samples per frame)
       int bufsize(buffer size)
       int bufidx(buffer index)
       int *combbuf0 //buffer of comb filter 0
       int *combbuf1 //buffer of comb filter 1
       int *combbuf2 //buffer of comb filter 2
       int *comboutbuf //buffer for output combbuf0+combbuf1+combbuf2
       int *rbbuf //reverb buffer
Implication input:
Output:
   ___________            ________________
  |comb_filter| work2buf | allpass_filter |   decbuf
  |___________|   ===>   |________________|    ===>
*/
void reverb_filter(int nChans, int framesize, int bufidx, int bufsize)
{
    int ch, j;
    int n;

    for (ch = 0; ch < nChans; ch++) {
        for (j = 0; j < NCOMB; j++) {
            comb_filter(&combinfo[j], ch, framesize, bufidx, bufsize);
        }

        for (n = bufidx; n < bufidx + framesize; n++) {
            int sum, i;
            sum=0;
            for(i=0;i<NCOMB;i++)
                sum += combinfo[i].outbuf[n + (ch * DECBUFSIZE)];

            work2buf[n + (ch * DECBUFSIZE)] = sum;
            /*    work2buf[n + (ch * DECBUFSIZE)] = combinfo[0].outbuf[n + (ch * DECBUFSIZE)] +
                combinfo[1].outbuf[n + (ch * DECBUFSIZE)] +
                combinfo[2].outbuf[n + (ch * DECBUFSIZE)];*/
        }

#  if defined(ALLPASS_FILTER)
        allpass_filter(&allpassinfo[0], ch, framesize, bufidx, bufsize);
#  endif

        for (n = bufidx; n < bufidx + framesize; n++) {
#    if defined(ALLPASS_FILTER)
            decbuf[n + (ch * DECBUFSIZE)] =
                MADDSHIFT(reverbbuf[n + (ch * DECBUFSIZE)],  revbinfo.src_gain, //SRC_GAIN ,
                          allpassinfo[0].outbuf[n + (ch * DECBUFSIZE)],
                          revbinfo.reverb_gain,//REVERB_GAIN,
                          REVERB_PRECISION);
#    else
            decbuf[n + (ch * DECBUFSIZE)] =
                MADDSHIFT(reverbbuf[n + (ch * DECBUFSIZE)], revbinfo.src_gain, //SRC_GAIN,
                          work2buf[n + (ch * DECBUFSIZE)], revbinfo.reverb_gain,//REVERB_GAIN,
                          REVERB_PRECISION);
#    endif
        }

    }
}

#endif // defined(REVERBERATION)

