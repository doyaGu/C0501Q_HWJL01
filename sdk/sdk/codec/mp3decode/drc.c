#include "mp3_config.h"
#include "drc.h"

#if defined(DRC)

typedef struct _drc_info_struct {
    int nch;
    int idx, idx_o;
    int nsamples, bufsize;
    int maxsamp_idx[2], maxsamp_idx_o[2];
    int maxsamp_g[2],   maxsamp_g_o[2];
    int start_g_o[2],   start_delta_o[2];
    int global_g;

} drc_info_struct;

#define MAXBLOCKS       2   // min 2
#define DRC_SHIFT       28

static param_drc_struct param_drc = {0, 100};
static drc_info_struct drcinfo;
static int drcbuf[2][MAXBLOCKS * MAX_FRAMESIZE];

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

static __inline void set_volume(int percentage)
{
    drcinfo.global_g = (1 << DRC_SHIFT) / 100 * percentage;
}

void drc_init(int nch, int nsamples)
{
    int ch;
    drcinfo.nch = nch;
    drcinfo.idx = 0;
    drcinfo.idx_o = MAXBLOCKS * MAX_FRAMESIZE - nsamples;
    drcinfo.nsamples = nsamples;
    drcinfo.bufsize = MAXBLOCKS * MAX_FRAMESIZE;
    set_volume(80);

    memset(&drcbuf[0][0], 0, sizeof(drcbuf));

    for(ch = 0; ch < nch; ch++){
        drcinfo.maxsamp_idx_o[ch] = 0;
        drcinfo.maxsamp_g_o[ch] = drcinfo.maxsamp_g[ch] = drcinfo.global_g;
        drcinfo.start_g_o[ch] = drcinfo.global_g;
        drcinfo.start_delta_o[ch] = 0;
    }
}

static __inline void check_drc_param(param_drc_struct *p_param)
{
    if(p_param->digital_gain < 0)
        p_param->digital_gain = 0;
    else if(p_param->digital_gain > 1000)
        p_param->digital_gain = 1000;

    if(param_drc.digital_gain != p_param->digital_gain){
        param_drc.digital_gain = p_param->digital_gain;
        set_volume(param_drc.digital_gain);
    }
}

void drc_filter(SAMPBUF *InBuf, SAMPBUF *OutBuf, param_drc_struct *p_param)
{
    int ch, j, n, nsamples;
    int g, delta;

    int (*inputbuf)[MAX_FRAMESIZE] = (int(*)[MAX_FRAMESIZE])InBuf->buf;
    int (*outputbuf)[MAX_FRAMESIZE] = (int(*)[MAX_FRAMESIZE])OutBuf->buf;

    //OutBuf->nch = drcinfo.nch;
    //OutBuf->nsamples = nsamples = drcinfo.nsamples;
    drcinfo.nch = OutBuf->nch;
    nsamples = drcinfo.nsamples = OutBuf->nsamples ;
    
    n = drcinfo.idx;
    check_drc_param(p_param);

    for(ch = 0; ch < drcinfo.nch; ch++){
        for(j = 0; j < nsamples; j++){
            drcbuf[ch][n+j] = inputbuf[ch][j];
        }
    }

    // find maxsamp
    for(ch = 0; ch < drcinfo.nch; ch++){
        int val, min_g, maxsamp_idx, max_adjust_g, maxval;
        drcinfo.maxsamp_g_o[ch] = drcinfo.maxsamp_g[ch];
        drcinfo.maxsamp_idx_o[ch] = drcinfo.maxsamp_idx[ch];
        maxval = 0;
        maxsamp_idx = 0;

        for (j = 0; j < nsamples; j++) {
            val = drcbuf[ch][n+j];
            if (val < 0) {
                val = -val;
            }
            if (val > maxval) {
                maxval = val;
                maxsamp_idx = j;
            }
        }

        if (MULSHIFT(maxval, drcinfo.global_g, DRC_SHIFT) >= 32767) {
            #if defined(__USE_INT64_LIB__)
            min_g = (int) __div64_32(((int64) 32767 * (1 << DRC_SHIFT)), maxval);
            #else
            min_g = (int) (((int64) 32767 * (1 << DRC_SHIFT)) / maxval);
            #endif
        } else {
            min_g = drcinfo.global_g;
        }

        max_adjust_g = drcinfo.maxsamp_g_o[ch] + drcinfo.global_g / 4;

        if (min_g > max_adjust_g) {
            min_g = max_adjust_g;
        }

        drcinfo.maxsamp_g[ch] = min_g;
        drcinfo.maxsamp_idx[ch] = maxsamp_idx;
    }

    // adjust sample
    n = drcinfo.idx_o;
    for (ch = 0; ch < drcinfo.nch; ch++) {
        g = drcinfo.start_g_o[ch];
        delta = drcinfo.start_delta_o[ch];

        for (j = 0; j < drcinfo.maxsamp_idx_o[ch]; j++) {
            drcbuf[ch][n+j] = MULSHIFT(drcbuf[ch][n+j], g, DRC_SHIFT);
            g += delta;
        }
    }

    for (ch = 0; ch < drcinfo.nch; ch++) {
        g = drcinfo.maxsamp_g_o[ch];
        delta = (drcinfo.maxsamp_g[ch] - drcinfo.maxsamp_g_o[ch]) /
                (drcinfo.maxsamp_idx[ch] + nsamples - drcinfo.maxsamp_idx_o[ch]);

        for (j = drcinfo.maxsamp_idx_o[ch]; j < nsamples; j++) {
            drcbuf[ch][n+j] = MULSHIFT(drcbuf[ch][n+j], g, DRC_SHIFT);
            g += delta;
        }

        drcinfo.start_g_o[ch] = g;
        drcinfo.start_delta_o[ch] = delta;
    }

    for(ch = 0; ch < drcinfo.nch; ch++){
        for(j = 0; j < nsamples; j++){
             outputbuf[ch][j] = drcbuf[ch][n+j];
        }
    }

    drcinfo.idx_o = drcinfo.idx;
    drcinfo.idx += drcinfo.nsamples;
    if(drcinfo.idx >= drcinfo.bufsize){
        drcinfo.idx -= drcinfo.bufsize;
    }
}

void delay_filter(SAMPBUF *InBuf, SAMPBUF *OutBuf)
{
    int ch, n, nsamples;

    int (*inputbuf)[MAX_FRAMESIZE] = (int(*)[MAX_FRAMESIZE])InBuf->buf;
    int (*outputbuf)[MAX_FRAMESIZE] = (int(*)[MAX_FRAMESIZE])OutBuf->buf;
    
    drcinfo.nch = OutBuf->nch;
    nsamples = drcinfo.nsamples = OutBuf->nsamples ;

    n = drcinfo.idx;
    for(ch = 0; ch < drcinfo.nch; ch++){
        int j;
        for(j = 0; j < nsamples; j++){
            drcbuf[ch][n+j] = inputbuf[ch][j];
        }
    }

    n = drcinfo.idx_o;
    for(ch = 0; ch < drcinfo.nch; ch++){
        int j;
        for(j = 0; j < nsamples; j++){
             outputbuf[ch][j] = drcbuf[ch][n+j];
        }
    }

    drcinfo.idx_o = drcinfo.idx;
    drcinfo.idx += drcinfo.nsamples;
    if(drcinfo.idx >= drcinfo.bufsize){
        drcinfo.idx -= drcinfo.bufsize;
    }
}

#endif

