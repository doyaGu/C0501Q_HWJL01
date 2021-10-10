
#include "mp3_config.h"
#include "reverb.h"

#if defined(REVERBERATION)

#if defined(WIN32) || defined(__CYGWIN__)
#   include <memory.h>
#   include <malloc.h>
#endif

typedef struct _filter_info_struct{
    int delaytime;  // in msec : variables which doesn't change with sample rate, filter characters
    int gain;
    int delaysamp;  // delay samples (=sr * delaytime / 1000) , variables which change with sample rate
}filter_info_struct;

typedef struct _reverb_info_struct
{
    int sr;
    int nch;
    int idx;
    int bufsize;
    int (*inputbuf)[MAX_FILTER_BUFSIZE];        // inputbuf[2][MAX_FILTER_BUFSIZE];
    int (*combbuf)[NCOMB][MAX_FILTER_BUFSIZE];  // combbuf[2][NCOMB][MAX_FILTER_BUFSIZE];
    int (*workbuf)[MAX_FILTER_BUFSIZE];         // workbuf[2][MAX_FILTER_BUFSIZE];
    int (*allpassbuf)[MAX_FILTER_BUFSIZE];      // allpassbuf[2][MAX_FILTER_BUFSIZE];
    int (*outputbuf)[MAX_FILTER_BUFSIZE];       // outputbuf[2][MAX_FILTER_BUFSIZE];
    int *bufptr;
} reverb_info_struct;

static param_reverb_struct param_reverb = {0, 0, 0, {0,0,0}, {0,0,0} };
static filter_info_struct combinfo[NCOMB];
static filter_info_struct allpassinfo;
static reverb_info_struct reverbinfo;

static void init_filter(filter_info_struct *info, int sr, int delaytime, int gain)
{
    info->delaytime = delaytime;
    info->gain = gain;
    info->delaysamp = sr * delaytime / 1000;

    if(info->delaysamp > (MAX_FRAMESIZE * (MAX_FRAMEDELAY-1))){
        info->delaysamp = (MAX_FRAMESIZE * (MAX_FRAMEDELAY-1));
#if defined(WIN32) || defined(__CYGWIN__)
        printf("Exception: delaytime(%d)ms except the limit\n", delaytime);
        while(1);
#endif
    }
}

// number of channel, sample rate, reverb buffer pointer
void reverb_init(int nch, int sr, int *bufptr)
{
    int ch, j;
    reverbinfo.nch = nch;
    reverbinfo.sr = sr;
    reverbinfo.bufptr = bufptr;
    ASSERT(nch > 0 && nch <= 2);
    ASSERT(sr > 0  && sr <= 48000);
    ASSERT(bufptr != 0);

    reverbinfo.inputbuf = (int(*)[MAX_FILTER_BUFSIZE])bufptr;
    bufptr += nch * MAX_FILTER_BUFSIZE;
    reverbinfo.combbuf = (int(*)[NCOMB][MAX_FILTER_BUFSIZE])bufptr;
    bufptr += nch * NCOMB * MAX_FILTER_BUFSIZE;
    reverbinfo.workbuf = (int(*)[MAX_FILTER_BUFSIZE])bufptr;
    bufptr += nch * MAX_FILTER_BUFSIZE;
    reverbinfo.allpassbuf = (int(*)[MAX_FILTER_BUFSIZE])bufptr;
    bufptr += nch * MAX_FILTER_BUFSIZE;
    reverbinfo.outputbuf = (int(*)[MAX_FILTER_BUFSIZE])bufptr;

         for(ch=0; ch<nch; ch++){
            memset(&reverbinfo.inputbuf[ch][0],   0, MAX_FILTER_BUFSIZE*4);
            for(j=0; j<NCOMB; j++){
                memset(&reverbinfo.combbuf[ch][j][0], 0, MAX_FILTER_BUFSIZE*4);
            }
            memset(&reverbinfo.workbuf[ch][0],    0, MAX_FILTER_BUFSIZE*4);
            memset(&reverbinfo.allpassbuf[ch][0], 0, MAX_FILTER_BUFSIZE*4);
            memset(&reverbinfo.outputbuf[ch][0],  0, MAX_FILTER_BUFSIZE*4);
        }
}

static void check_reverb_param(param_reverb_struct *p_param)
{
    int ch, j;
    int sr = reverbinfo.sr;
    int nch = reverbinfo.nch;

    if((param_reverb.src_gain != p_param->src_gain)
    || (param_reverb.reverb_gain != p_param->reverb_gain)
    || (param_reverb.delay[0] != p_param->delay[0])
    || (param_reverb.delay[1] != p_param->delay[1])
    || (param_reverb.delay[2] != p_param->delay[2])
    || (param_reverb.gain[0] != p_param->gain[0])
    || (param_reverb.gain[1] != p_param->gain[1])
    || (param_reverb.gain[2] != p_param->gain[2]))
    {
        param_reverb.src_gain = p_param->src_gain;
        param_reverb.reverb_gain = p_param->reverb_gain;
        param_reverb.delay[0] = p_param->delay[0];
        param_reverb.delay[1] = p_param->delay[1];
        param_reverb.delay[2] = p_param->delay[2];
        param_reverb.gain[0] = p_param->gain[0];
        param_reverb.gain[1] = p_param->gain[1];
        param_reverb.gain[2] = p_param->gain[2];

        reverbinfo.idx = 0;
        reverbinfo.bufsize = MAX_FILTER_BUFSIZE;

        init_filter(&combinfo[0], sr, param_reverb.delay[0], param_reverb.gain[0]);
        init_filter(&combinfo[1], sr, param_reverb.delay[1], param_reverb.gain[1]);
        init_filter(&allpassinfo, sr, param_reverb.delay[2], param_reverb.gain[2]);

        for(ch=0; ch<nch; ch++){
            memset(&reverbinfo.inputbuf[ch][0],   0, MAX_FILTER_BUFSIZE*4);
            for(j=0; j<NCOMB; j++){
                memset(&reverbinfo.combbuf[ch][j][0], 0, MAX_FILTER_BUFSIZE*4);
            }
            memset(&reverbinfo.workbuf[ch][0],    0, MAX_FILTER_BUFSIZE*4);
            memset(&reverbinfo.allpassbuf[ch][0], 0, MAX_FILTER_BUFSIZE*4);
            memset(&reverbinfo.outputbuf[ch][0],  0, MAX_FILTER_BUFSIZE*4);
        }
    }
}

//  implement:
//  y[n] = x[n-m] + gy[n-m];
static void comb_filter(filter_info_struct *info, int *inbuf, int *outbuf, int nsamples)
{
    int n, n_m, cnt0, cnt1, bufsize;
#if !defined(REVERB_ENGINE)
    int i;
#endif
    int g = info->gain;
    n = reverbinfo.idx;
    n_m = n - info->delaysamp;
    bufsize = reverbinfo.bufsize;

    if(n_m < 0) n_m += bufsize;

//  original code
//  for(i=0; i<dec_framesize; i++){
//      outbuf[n] = inbuf[n_m] + MUL(g, outbuf[n_m]);
//      n++; n_m++;
//      if(n == bufsize) n = 0;
//      if(n_m == bufsize) n_m = 0;
//  }

    if(n_m + nsamples >= bufsize){
        cnt0 = bufsize - n_m;
        cnt1 = nsamples - cnt0;
    }
    else{
        cnt0 = nsamples;
        cnt1 = 0;
    }

    for(i=0; i<cnt0; i++){
        outbuf[n+i] = inbuf[n_m+i] + MUL(g, outbuf[n_m+i]);
    }
    if(cnt1){
        n += cnt0;
        for(i=0; i<cnt1; i++){
            outbuf[n+i] = inbuf[0+i] + MUL(g, outbuf[0+i]);
        }
    }

    // we update idx after output samples in outbuf is retrieved...
}

//  implement:
//  y[n] = x[n-m] + gy[n-m] - gx[n];
static void allpass_filter(filter_info_struct *info, int *inbuf, int *outbuf, int nsamples)
{
    int n, n_m, cnt0, cnt1, bufsize;
#if !defined(REVERB_ENGINE)
    int i;
#endif
    int g = info->gain;
    n = reverbinfo.idx;
    n_m = n - info->delaysamp;
    bufsize = reverbinfo.bufsize;

    if(n_m < 0) n_m += bufsize;

//  for(i=0; i<dec_framesize; i++){
//      outbuf[n] = inbuf[n_m] + MUL(g, (outbuf[n_m] - inbuf[n]));
//      n++; n_m++;
//      if(n == decbufsize) n = 0;
//      if(n_m == decbufsize) n_m = 0;
//  }

    if(n_m + nsamples >= bufsize){
        cnt0 = bufsize - n_m;
        cnt1 = nsamples - cnt0;
    }
    else{
        cnt0 = nsamples;
        cnt1 = 0;
    }

    for(i=0; i<cnt0; i++){
    //  outbuf[n+i] = inbuf[n_m+i] + MUL(g, (outbuf[n_m+i] - inbuf[n+i]));
        outbuf[n+i] = inbuf[n_m+i] + MSUBSHIFT(outbuf[n_m+i], g, inbuf[n+i], g, REVERB_PRECISION);
    }
    if(cnt1){
        n += cnt0;
        for(i=0; i<cnt1; i++){
        //  outbuf[n+i] = inbuf[0+i] + MUL(g, (outbuf[0+i] - inbuf[n+i]));
            outbuf[n+i] = inbuf[0+i] + MSUBSHIFT(outbuf[0+i], g, inbuf[n+i], g, REVERB_PRECISION);
        }
    }
    // we update idx after output samples in outbuf is retrieved...
}

void reverb_filter(SAMPBUF *InBuf, SAMPBUF *OutBuf, param_reverb_struct *p_param)   // REVERB_ENGINE OFF
{
    int ch;
    int j, n, nsamples;
    int (*inputbuf)[MAX_FRAMESIZE] = (int(*)[MAX_FRAMESIZE])InBuf->buf;
    int (*outputbuf)[MAX_FRAMESIZE] = (int(*)[MAX_FRAMESIZE])OutBuf->buf;

    OutBuf->nch = reverbinfo.nch;       // fixed nch after reverb_activate()
    OutBuf->nsamples = nsamples = InBuf->nsamples;
    if(nsamples == 0)
        return;
    n = reverbinfo.idx;

    check_reverb_param(p_param);

#if defined(WIN32) || defined(__CYGWIN__)
    for(j=0; j<NCOMB; j++){
        if(combinfo[j].delaysamp + nsamples > MAX_FILTER_BUFSIZE){
           printf("warning: buffer size is not enough for given delay time..\n");
        }
    }
#endif

    for(ch=0; ch< reverbinfo.nch; ch++){
        # if !defined(REVERB_ENGINE)
        for(j=0; j<nsamples; j++){
            reverbinfo.inputbuf[ch][n+j] = inputbuf[ch][j];
        }
        # else
            memcpy32(&reverbinfo.inputbuf[ch][n], &inputbuf[ch][0], nsamples);
        # endif
    }

    for(ch=0; ch< reverbinfo.nch; ch++){
        for(j=0; j<NCOMB; j++){
            comb_filter(&combinfo[j], reverbinfo.inputbuf[ch], reverbinfo.combbuf[ch][j], nsamples);
        }
    }

    for(ch=0; ch< reverbinfo.nch; ch++){
        for(j = n; j < n + nsamples; j++){
            reverbinfo.workbuf[ch][j] = reverbinfo.combbuf[ch][0][j] + reverbinfo.combbuf[ch][1][j];
        }
   }

    for(ch=0; ch< reverbinfo.nch; ch++){
        allpass_filter(&allpassinfo, reverbinfo.workbuf[ch], reverbinfo.allpassbuf[ch], nsamples);
    }
    for(ch=0; ch< reverbinfo.nch; ch++){
        for(j = n; j < n + nsamples; j++){
            reverbinfo.outputbuf[ch][j] =
            MADDSHIFT(reverbinfo.inputbuf[ch][j], param_reverb.src_gain,
                    reverbinfo.allpassbuf[ch][j], param_reverb.reverb_gain, REVERB_PRECISION);
        }
    }
    for(ch=0; ch< reverbinfo.nch; ch++){
        for(j=0; j< nsamples; j++){
            outputbuf[ch][j] = reverbinfo.outputbuf[ch][n+j];
        }
    }

    reverbinfo.idx += nsamples;
    if(reverbinfo.idx >= reverbinfo.bufsize){
        reverbinfo.idx -= reverbinfo.bufsize;
    }
}

#endif // defined(REVERBERATION)

