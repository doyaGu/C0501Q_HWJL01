/*
 * WMA compatible decoder
 * Copyright (c) 2002 The FFmpeg Project.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * @file wmadec.c
 * WMA compatible decoder.
 */

#include <string.h>
#include "config.h"
#include "types.h"
#include "asf.h"
#include "bitstream.h"
#include "wmadec.h"
#include "wmafixed.h"
#include "freqinfo.h"

#ifdef WMA_PERFORMANCE_TEST
#include "plugin.h"

static PAL_CLOCK_T tClockSpectrumPerformance;
static long nSpectrumPerformance;  // avg of frame

#endif            

#ifdef WMA_LAG_OPTIMIZE
extern int lag_optimize;
int block_length_512 = 0;
int block_length_1024 = 0;
int block_length_2048 = 0;
#endif

#define VLCBITS 5       /*7 is the lowest without glitching*/
#define VLCMAX ((22+VLCBITS-1)/VLCBITS)

#define EXPVLCBITS 5
#define EXPMAX ((19+EXPVLCBITS-1)/EXPVLCBITS)

#define HGAINVLCBITS 7
#define HGAINMAX ((13+HGAINVLCBITS-1)/HGAINVLCBITS)

typedef struct CoefVLCTable
{
    int n; /* total number of codes */
    const uint32_t *huffcodes; /* VLC bit values */
    const uint8_t *huffbits;   /* VLC bit size */
    const uint16_t *levels; /* table to build run/level tables */
}
CoefVLCTable;

static void wma_lsp_to_curve_init(WMADecodeContext *s, int frame_len);

#define VLCBUF1SIZE 2790//4598
#define VLCBUF2SIZE 2550//3574
#define VLCBUF3SIZE 200//360
#define VLCBUF4SIZE 210//110

/*putting these in IRAM actually makes PP slower*/
static VLC_TYPE vlcbuf1[VLCBUF1SIZE];
static VLC_TYPE vlcbuf2[VLCBUF2SIZE];
static VLC_TYPE vlcbuf3[VLCBUF3SIZE];
static VLC_TYPE vlcbuf4[VLCBUF4SIZE];

#include "wmadata.h" // PJJ

/*
 * Helper functions for wma_window.
 *
 *
 */

#ifdef CPU_ARM
static __inline
void vector_fmul_add_add(fixed32 *dst, const fixed32 *data,
                         const fixed32 *window, int n)
{
    /* Block sizes are always power of two */
    asm volatile (
        "0:"
        "ldmia %[d]!, {r0, r1};"
        "ldmia %[w]!, {r4, r5};"
        /* consume the first data and window value so we can use those
         * registers again */
        "smull r8, r9, r0, r4;"
        "ldmia %[dst], {r0, r4};"
        "add   r0, r0, r9, lsl #1;"  /* *dst=*dst+(r9<<1)*/
        "smull r8, r9, r1, r5;"
        "add   r1, r4, r9, lsl #1;"
        "stmia %[dst]!, {r0, r1};"
        "subs  %[n], %[n], #2;"
        "bne   0b;"
        : [d] "+r" (data), [w] "+r" (window), [dst] "+r" (dst), [n] "+r" (n)
        : : "r0", "r1", "r4", "r5", "r8", "r9", "memory", "cc");
}

static inline
void vector_fmul_reverse(fixed32 *dst, const fixed32 *src0, const fixed32 *src1,
                         int len)
{
    /* Block sizes are always power of two */
    asm volatile (
        "add   %[s1], %[s1], %[n], lsl #2;"
        "0:"
        "ldmia %[s0]!, {r0, r1};"
        "ldmdb %[s1]!, {r4, r5};"
        "smull r8, r9, r0, r5;"
        "mov   r0, r9, lsl #1;"
        "smull r8, r9, r1, r4;"
        "mov   r1, r9, lsl #1;"
        "stmia %[dst]!, {r0, r1};"
        "subs  %[n], %[n], #2;"
        "bne   0b;"
        : [s0] "+r" (src0), [s1] "+r" (src1), [dst] "+r" (dst), [n] "+r" (len)
        : : "r0", "r1", "r4", "r5", "r8", "r9", "memory", "cc");
}

#elif defined(CPU_COLDFIRE)

static __inline
void vector_fmul_add_add(fixed32 *dst, const fixed32 *data,
                         const fixed32 *window, int n)
{
    /* Block sizes are always power of two. Smallest block is always way bigger
     * than four too.*/
    asm volatile (
        "0:"
        "movem.l (%[d]), %%d0-%%d3;"
        "movem.l (%[w]), %%d4-%%d5/%%a0-%%a1;"
        "mac.l %%d0, %%d4, %%acc0;"
        "mac.l %%d1, %%d5, %%acc1;"
        "mac.l %%d2, %%a0, %%acc2;"
        "mac.l %%d3, %%a1, %%acc3;"
        "lea.l (16, %[d]), %[d];"
        "lea.l (16, %[w]), %[w];"
        "movclr.l %%acc0, %%d0;"
        "movclr.l %%acc1, %%d1;"
        "movclr.l %%acc2, %%d2;"
        "movclr.l %%acc3, %%d3;"
        "movem.l (%[dst]), %%d4-%%d5/%%a0-%%a1;"
        "add.l %%d4, %%d0;"
        "add.l %%d5, %%d1;"
        "add.l %%a0, %%d2;"
        "add.l %%a1, %%d3;"
        "movem.l %%d0-%%d3, (%[dst]);"
        "lea.l (16, %[dst]), %[dst];"
        "subq.l #4, %[n];"
        "jne 0b;"
        : [d] "+a" (data), [w] "+a" (window), [dst] "+a" (dst), [n] "+d" (n)
        : : "d0", "d1", "d2", "d3", "d4", "d5", "a0", "a1", "memory", "cc");
}

static inline
void vector_fmul_reverse(fixed32 *dst, const fixed32 *src0, const fixed32 *src1,
                         int len)
{
    /* Block sizes are always power of two. Smallest block is always way bigger
     * than four too.*/
    asm volatile (
        "lea.l (-16, %[s1], %[n]*4), %[s1];"
        "0:"
        "movem.l (%[s0]), %%d0-%%d3;"
        "movem.l (%[s1]), %%d4-%%d5/%%a0-%%a1;"
        "mac.l %%d0, %%a1, %%acc0;"
        "mac.l %%d1, %%a0, %%acc1;"
        "mac.l %%d2, %%d5, %%acc2;"
        "mac.l %%d3, %%d4, %%acc3;"
        "lea.l (16, %[s0]), %[s0];"
        "lea.l (-16, %[s1]), %[s1];"
        "movclr.l %%acc0, %%d0;"
        "movclr.l %%acc1, %%d1;"
        "movclr.l %%acc2, %%d2;"
        "movclr.l %%acc3, %%d3;"
        "movem.l %%d0-%%d3, (%[dst]);"
        "lea.l (16, %[dst]), %[dst];"
        "subq.l #4, %[n];"
        "jne 0b;"
        : [s0] "+a" (src0), [s1] "+a" (src1), [dst] "+a" (dst), [n] "+d" (len)
        : : "d0", "d1", "d2", "d3", "d4", "d5", "a0", "a1", "memory", "cc");
}

#else

static __inline void vector_fmul_add_add(int16_t *dst, const int16_t *src0, int shift, fixed32* cos_tab, int len){
    int i;
    for(i=0; i<len; i++)
    {
        //dst[i] = fixmul32b(src0[i], src1[i]) + dst[i];
        dst[i] = fixmul16b(src0[i], cos_tab[2047-(i<<shift)]) + dst[i];  // sin() rather than cos()
    }
}

static __inline void vector_fmul_reverse(int16_t *dst, const int16_t *src0,  int shift, fixed32* cos_tab, int len){
    int i;
    //src1 += len-1;
    for(i=0; i<len; i++)
    {
        //dst[i] = fixmul32b(src0[i], src1[-i]);
        dst[i] = fixmul16b(src0[i], cos_tab[i<<shift]);  // sin() rather than cos()
    }
}

#endif

/**
  * Apply MDCT window and add into output.
  *
  * We ensure that when the windows overlap their squared sum
  * is always 1 (MDCT reconstruction rule).
  *
  * The Vorbis I spec has a great diagram explaining this process.
  * See section 1.3.2.3 of http://xiph.org/vorbis/doc/Vorbis_I_spec.html
  */
static void wma_window(WMADecodeContext *s, int16_t *in, int16_t *out)
{
    int block_len, bsize, n, shift;

    /* left part */
    /*previous block was larger, so we'll use the size of the current block to set the window size*/
    if (s->block_len_bits <= s->prev_block_len_bits) {
        block_len = s->block_len;
        bsize = s->frame_len_bits - s->block_len_bits;
        shift = 11 - s->frame_len_bits + bsize;  // cos table is 2048 entry

        vector_fmul_add_add(out, in, shift, s->cos_table, block_len);

    } else {
        /*previous block was smaller or the same size, so use it's size to set the window length*/
        block_len = 1 << s->prev_block_len_bits;
        /*find the middle of the two overlapped blocks, this will be the first overlapped sample*/
        n = (s->block_len - block_len) / 2;
        bsize = s->frame_len_bits - s->prev_block_len_bits;
        shift = 11 - s->frame_len_bits + bsize;  // cos table is 2048 entry

        vector_fmul_add_add(out+n, in+n, shift, s->cos_table, block_len);

        //memcpy(out+n+block_len, in+n+block_len, n*sizeof(fixed32));
        memcpy(out+n+block_len, in+n+block_len, n*sizeof(int16_t));

    }
   /* Advance to the end of the current block and prepare to window it for the next block.
    * Since the window function needs to be reversed, we do it backwards starting with the
    * last sample and moving towards the first
    */
    out += s->block_len;
    in += s->block_len;

    /* right part */
    if (s->block_len_bits <= s->next_block_len_bits) {
        block_len = s->block_len;
        bsize = s->frame_len_bits - s->block_len_bits;
        shift = 11 - s->frame_len_bits + bsize;  // cos table is 2048 entry

        vector_fmul_reverse(out, in,  shift, s->cos_table, block_len);
    } else {
        block_len = 1 << s->next_block_len_bits;
        n = (s->block_len - block_len) / 2;
        bsize = s->frame_len_bits - s->next_block_len_bits;
        shift = 11 - s->frame_len_bits + bsize;  // cos table is 2048 entry

        //memcpy(out, in, n*sizeof(fixed32));
        memcpy(out, in, n*sizeof(int16_t));

        vector_fmul_reverse(out+n, in+n,  shift, s->cos_table, block_len);

        //memset(out+n+block_len, 0, n*sizeof(fixed32));
        memset(out+n+block_len, 0, n*sizeof(int16_t));
    }
}

/* XXX: use same run/length optimization as mpeg decoders */
static int init_coef_vlc(VLC *vlc,
                         uint16_t **plevels_table,
                         const CoefVLCTable *vlc_table, int tab)
{
    int n = vlc_table->n;
    const uint8_t *table_bits = vlc_table->huffbits;
    const uint32_t *table_codes = vlc_table->huffcodes;

    if (init_vlc(vlc, VLCBITS, n, table_bits, 1, 1, table_codes, 4, 4, 0) < 0)
        return ERR_VLC_INIT_ERR;

    *plevels_table = (uint16_t*)vlc_table->levels;

    return ERR_NONE;
}

int wma_decode_init(WMADecodeContext* s, asf_waveformatex_t *wfx)
{
    //WMADecodeContext *s = avctx->priv_data;
    int i, flags1, flags2;
    uint8_t *extradata;
    fixed64 bps1;
    fixed32 high_freq;
    fixed64 bps;
    int sample_rate1;
    int coef_vlc_table;

    #ifdef CPU_COLDFIRE
    coldfire_set_macsr(EMAC_FRACTIONAL | EMAC_SATURATE);
    #endif

    s->sample_rate = wfx->rate;
    s->nb_channels = wfx->channels;
    s->bit_rate = wfx->bitrate;
    s->block_align = wfx->blockalign;
    s->noise_seed = 314159;

    if (wfx->codec_id == ASF_CODEC_ID_WMAV1) 
    {
        s->version = 1;
    }
    else if (wfx->codec_id == ASF_CODEC_ID_WMAV2 ) 
    {
        s->version = 2;
    }
    else
    {
        /*one of those other wma flavors that don't have GPLed decoders */
        return ERR_UNKNOWN;
    }

    /* extract flag infos */
    flags1 = 0;
    flags2 = 0;
    extradata = wfx->data;
    if (s->version == 1 && wfx->datalen >= 4) 
    {
        flags1 = extradata[0] | (extradata[1] << 8);
        flags2 = extradata[2] | (extradata[3] << 8);
    }
    else if (s->version == 2 && wfx->datalen >= 6)
    {
        flags1 = extradata[0] | (extradata[1] << 8) |
                 (extradata[2] << 16) | (extradata[3] << 24);
        flags2 = extradata[4] | (extradata[5] << 8);
    }
    s->use_exp_vlc = flags2 & 0x0001;
    s->use_bit_reservoir = flags2 & 0x0002;
    s->use_variable_block_len = flags2 & 0x0004;

    /* compute MDCT block size */
    if (s->sample_rate <= 16000) 
    {
        s->frame_len_bits = 9;
    }
    else if (s->sample_rate <= 22050 ||
             (s->sample_rate <= 32000 && s->version == 1)){
        s->frame_len_bits = 10;
    } 
    else
    {
        s->frame_len_bits = 11;
    }
    s->frame_len = 1 << s->frame_len_bits;

    if (s-> use_variable_block_len) 
    {
        int nb_max, nb;
        nb = ((flags2 >> 3) & 3) + 1;
        if ((s->bit_rate / s->nb_channels) >= 32000) 
        {
            nb += 2;
        }
        nb_max = s->frame_len_bits - BLOCK_MIN_BITS;        //max is 11-7
        if (nb > nb_max)
            nb = nb_max;
        s->nb_block_sizes = nb + 1;    // max_num_subframes
    }
    else
    {
        s->nb_block_sizes = 1;
    }

    /* init rate dependant parameters */
    s->use_noise_coding = 1;
    high_freq = itofix64(s->sample_rate) >> 1;

    /* if version 2, then the rates are normalized */
    sample_rate1 = s->sample_rate;
    if (s->version == 2) 
    {
        if (sample_rate1 >= 44100)
            sample_rate1 = 44100;
        else if (sample_rate1 >= 22050)
            sample_rate1 = 22050;
        else if (sample_rate1 >= 16000)
            sample_rate1 = 16000;
        else if (sample_rate1 >= 11025)
            sample_rate1 = 11025;
        else if (sample_rate1 >= 8000)
            sample_rate1 = 8000;
    }

    {
        fixed64 tmp, tmp2, tim, tmpi;
        tmp = itofix64(s->bit_rate);
        tmp2 = itofix64(s->nb_channels * s->sample_rate);
        bps = fixdiv64(tmp, tmp2);
        tim = bps * s->frame_len;
        tmpi = fixdiv64(tim,itofix64(8));
        s->byte_offset_bits = av_log2(fixtoi64(tmpi+0x8000)) + 2;
    }
    /* compute high frequency value and choose if noise coding should
       be activated */
    bps1 = bps;
    if (s->nb_channels == 2)
        bps1 = fixmul32(bps,0x1999a);
    if (sample_rate1 == 44100) {
        if (bps1 >= 0x9c29)
            s->use_noise_coding = 0;
        else
            high_freq = fixmul32(high_freq,0x6666);  //first_noise_freq
    } else if (sample_rate1 == 22050) {
        if (bps1 >= 0x128f6)
            s->use_noise_coding = 0;
        else if (bps1 >= 0xb852)
            high_freq = fixmul32(high_freq,0xb333);
        else
            high_freq = fixmul32(high_freq,0x999a);
    } else if (sample_rate1 == 16000) {
        if (bps > 0x8000)
            high_freq = fixmul32(high_freq,0x8000);
        else
            high_freq = fixmul32(high_freq,0x4ccd);
    } else if (sample_rate1 == 11025) {
        high_freq = fixmul32(high_freq,0xb333);
    } else if (sample_rate1 == 8000) {
        if (bps <= 0xa000) {
           high_freq = fixmul32(high_freq,0x8000);
        } else if (bps > 0xc000) {
            s->use_noise_coding = 0;
        } else {
            high_freq = fixmul32(high_freq,0xa666);
        }
    } else {
        if (bps >= 0xcccd) {
            high_freq = fixmul32(high_freq,0xc000);
        } else if (bps >= 0x999a) {
            high_freq = fixmul32(high_freq,0x999a);
        } else {
            high_freq = fixmul32(high_freq,0x8000);
        }
    }

    /* compute the scale factor band sizes for each MDCT block size */
    {
        int a, b, pos, lpos, k, block_len, i, j, n;
        const uint8_t *table;

        if (s->version == 1)
        {
            s->coefs_start = 3;
        }
        else
        {
            s->coefs_start = 0;
        }

        for(k = 0; k < s->nb_block_sizes; ++k) 
        {
            block_len = s->frame_len >> k;

            if (s->version == 1) 
            {
                lpos = 0;
                for(i=0;i<25;++i) 
                {
                    a = wma_critical_freqs[i];
                    b = s->sample_rate;
                    pos = ((block_len * 2 * a)  + (b >> 1)) / b;
                    if (pos > block_len)
                        pos = block_len;
                    s->exponent_bands[0][i] = pos - lpos;
                    if (pos >= block_len) 
                    {
                        ++i;
                        break;
                    }
                    lpos = pos;
                }
                s->exponent_sizes[0] = i;
            }
            else 
            {
                /* hardcoded tables */
                table = NULL;
                a = s->frame_len_bits - BLOCK_MIN_BITS - k;
                if (a < 3) 
                {
                    if (s->sample_rate >= 44100)
                        table = exponent_band_44100[a];
                    else if (s->sample_rate >= 32000)
                        table = exponent_band_32000[a];
                    else if (s->sample_rate >= 22050)
                        table = exponent_band_22050[a];
                }

                if (table) 
                {
                    n = *table++;
                    for(i=0;i<n;++i)
                        s->exponent_bands[k][i] = table[i];
                    s->exponent_sizes[k] = n;
                }
                else
                {
                    j = 0;
                    lpos = 0;
                    for(i=0;i<25;++i) 
                    {
                        a = wma_critical_freqs[i];
                        b = s->sample_rate;
                        pos = ((block_len * 2 * a)  + (b << 1)) / (4 * b);
                        pos <<= 2;
                        if (pos > block_len)
                            pos = block_len;
                        if (pos > lpos)
                            s->exponent_bands[k][j++] = pos - lpos;
                        if (pos >= block_len)
                            break;
                        lpos = pos;
                    }
                    s->exponent_sizes[k] = j;
                }
            }

            /* max number of coefs */
            s->coefs_end[k] = (s->frame_len - ((s->frame_len * 9) / 100)) >> k;
            /* high freq computation */
            {
                fixed32 tmp1 = high_freq*2;            /* high_freq is a fixed32!*/
                fixed32 tmp2=itofix32(s->sample_rate>>1);
                s->high_band_start[k] = fixtoi32( fixdiv32(tmp1, tmp2) * (block_len>>1) +0x8000);
            }
            /*
            s->high_band_start[k] = (int)((block_len * 2 * high_freq) /
                                          s->sample_rate + 0.5);*/

            n = s->exponent_sizes[k];
            j = 0;
            pos = 0;
            for(i=0;i<n;++i) 
            {
                int start, end;
                start = pos;
                pos += s->exponent_bands[k][i];
                end = pos;
                if (start < s->high_band_start[k])
                    start = s->high_band_start[k];
                if (end > s->coefs_end[k])
                    end = s->coefs_end[k];
                if (end > start)
                    s->exponent_high_bands[k][j++] = end - start;
            }
            s->exponent_high_sizes[k] = j;
        }
    }

    // initialize cos_table 0~1/2pi
    {
        int i;
        for (i=0; i<2048; ++i) 
        {
            fsincos(i<<19, &s->cos_table[i]);
        }
    }

    mdct_init_global();

    for(i = 0; i < s->nb_block_sizes; ++i) 
    {
        ff_mdct_init(&s->mdct_ctx[i], s->frame_len_bits - i + 1, 1);
    }

    s->reset_block_lengths = 1;

    if (s->use_noise_coding) 
    {
        /* init the noise generator */
        if (s->use_exp_vlc) 
        {
            s->noise_mult = 0x51f;
            //s->noise_table = noisetable_exp;
            s->noise_bits = PRECISION;
        }
        else
        {
            s->noise_mult = 0xa3d;
            /* LSP values are simply 2x the EXP values */
            //for (i=0;i<NOISE_TAB_SIZE;++i)
            //    noisetable_exp[i] = noisetable_exp[i]<< 1;
            //s->noise_table = noisetable_exp;
            s->noise_bits = PRECISION - 1;
        }
#if 0
        {
            unsigned int seed;
            fixed32 norm;
            seed = 1;
            norm = 0;   // PJJ: near as makes any diff to 0!
            for (i=0;i<NOISE_TAB_SIZE;++i) {
                seed = seed * 314159 + 1;
                s->noise_table[i] = itofix32((int)seed) * norm;
            }
        }
#endif

        s->hgain_vlc.table = vlcbuf4;
        s->hgain_vlc.table_allocated = VLCBUF4SIZE;

        if (init_vlc(&s->hgain_vlc, HGAINVLCBITS, sizeof(hgain_huffbits),
                  hgain_huffbits, 1, 1,
                  hgain_huffcodes, 2, 2, 0) < 0)
            return ERR_VLC_INIT_ERR;
    }

    if (s->use_exp_vlc) {

        s->exp_vlc.table = vlcbuf3;
        s->exp_vlc.table_allocated = VLCBUF3SIZE;

        if(init_vlc(&s->exp_vlc, EXPVLCBITS, sizeof(scale_huffbits),
                  scale_huffbits, 1, 1,
                  scale_huffcodes, 4, 4, 0) < 0)
            return ERR_VLC_INIT_ERR;
    } else {
        wma_lsp_to_curve_init(s, s->frame_len);
    }

    /* choose the VLC tables for the coefficients */
    coef_vlc_table = 2;
    if (s->sample_rate >= 32000) {
        if (bps1 < 0xb852)
            coef_vlc_table = 0;
        else if (bps1 < 0x128f6)
            coef_vlc_table = 1;
    }

    s->coef_vlc[0].table = vlcbuf1;
    s->coef_vlc[0].table_allocated = VLCBUF1SIZE;
    s->coef_vlc[1].table = vlcbuf2;
    s->coef_vlc[1].table_allocated = VLCBUF2SIZE;

    if (init_coef_vlc(&s->coef_vlc[0], &s->levels_table[0],
                  &coef_vlcs[coef_vlc_table * 2], 0) < 0)
        return ERR_VLC_INIT_ERR;

    if (init_coef_vlc(&s->coef_vlc[1], &s->levels_table[1],
                  &coef_vlcs[coef_vlc_table * 2 + 1], 1) < 0)
        return ERR_VLC_INIT_ERR;

    s->last_superframe_len = 0;
    s->last_bitoffset = 0;

    return ERR_NONE;
}

/* compute x^-0.25 with an exponent and mantissa table. We use linear
   interpolation to reduce the mantissa table size at a small speed
   expense (linear interpolation approximately doubles the number of
   bits of precision). */
static __inline fixed32 pow_m1_4(WMADecodeContext *s, fixed32 x)
{
    union {
        float f;
        unsigned int v;
    } u, t;
    unsigned int e, m;
    fixed32 a, b, tmp;
    fixed64 lsp_pow_e;
    int shift;

    u.f = fixtof64(x);
    e = u.v >> 23;
    m = (u.v >> (23 - LSP_POW_BITS)) & ((1 << LSP_POW_BITS) - 1);
    /* build interpolation scale: 1 <= t < 2. */
    t.v = ((u.v << LSP_POW_BITS) & ((1 << 23) - 1)) | (127 << 23);
    a = s->lsp_pow_m_table1[m];
    b = s->lsp_pow_m_table2[m];

    /* lsp_pow_e_table contains 32.32 format */
    /* TODO:  Since we're unlikely have value that cover the whole
     * IEEE754 range, we probably don't need to have all possible exponents */
    shift = 32 - (e>>2);
    if( shift>0 )
        lsp_pow_e = (fixed64)lsp_pow_e_table[e%4] << shift;
    else
        lsp_pow_e = (fixed64)lsp_pow_e_table[e%4] >> (-shift);

    //tmp = lsp_pow_e_table[e] * (a + fixmul32(b, ftofix32(t.f))) >>32;
    tmp = lsp_pow_e * (a + fixmul32(b, ftofix32(t.f))) >>32;
    return tmp;
}

static void wma_lsp_to_curve_init(WMADecodeContext *s, int frame_len)
{
    fixed32 a, b;
    int i, m;

/*
    fixed32 temp, temp2;
    temp = fixdiv32(itofix32(1), itofix32(frame_len));
    for (i=0; i<frame_len; ++i) {
        fsincos((temp*i)<<15, &temp2);
        s->lsp_cos_table[i] = temp2;   // lsp_cos_table is 0~pi
    }
*/
    /* NOTE: these two tables are needed to avoid two operations in
       pow_m1_4 */
    b = itofix32(1);
    {
        int ix = 0;
        /*double check this later*/
        for(i=(1 << LSP_POW_BITS) - 1;i>=0;i--) {
            m = (1 << LSP_POW_BITS) + i;
            a = (fixed32)pow_a_table[ix++]<<4;
            s->lsp_pow_m_table1[i] = 2 * a - b;
            s->lsp_pow_m_table2[i] = b - a;
            b = a;
        }
    }
}

/* NOTE: We use the same code as Vorbis here */
/* XXX: optimize it further with SSE/3Dnow */
static void wma_lsp_to_curve(WMADecodeContext *s,
                             fixed32 *out,
                             fixed32 *val_max_ptr,
                             fixed32 *lsp)
{
    int i, j, n, shift;
    fixed32 p, q, w, v, val_max, temp, temp2;
    int index_start = 0;
    int step_size, step_i;

    val_max = 0;
    n = s->block_len;
    //shift = s->frame_len_bits - s->block_len_bits;
    shift = 11 - s->block_len_bits;

    step_i = 1;

    #ifdef WMA_LAG_OPTIMIZE
    if (n == 512)                       // 2^9
    {
        step_i = 8;
        block_length_512 = 1;
    }
    if (n == 1024)                      // 2^10
    {
        step_i = 16;
        block_length_1024 = 1;
    }
    if (n == 2048)                      // 2^11
    {
        step_i = 32;
        block_length_2048 = 1;
    }
#endif

    for (i = index_start ; i < n; i+=step_i)
    {
        /* shift by 2 now to reduce rounding error,
         * we can renormalize right before pow_m1_4
         */

        p = 0x8000<<5;
        q = 0x8000<<5;
        //w = s->lsp_cos_table[i<<shift]>>3;
        if( i<n/2 )
            w = s->cos_table[i<<shift]>>3;
        else
            w = -(s->cos_table[2047-((i-n/2)<<shift)]>>3);

        for (j=1;j<NB_LSP_COEFS;j+=2) {
            /* w is 5.27 format, lsp is in 16.16, temp2 becomes 5.27 format */
            temp2 = ((w - (lsp[j - 1]<<11)));
            temp = q;

            /* q is 16.16 format, temp2 is 5.27, q becomes 16.16 */
            q = fixmul32b(q, temp2 )<<4;
            p = fixmul32b(p, (w - (lsp[j]<<11)))<<4;
        }

        /* 2 in 5.27 format is 0x10000000 */
        p = fixmul32(p, fixmul32b(p, (0x10000000 - w)))<<3;
        q = fixmul32(q, fixmul32b(q, (0x10000000 + w)))<<3;

        v = (p + q) >>9;  /* p/q end up as 16.16 */
        v = pow_m1_4(s, v);
        if (v > val_max)
            val_max = v;
        out[i] = v;
    }

#ifdef WMA_LAG_OPTIMIZE
    // linear interpolation
    if (block_length_512 || block_length_1024 || block_length_2048)
    {
        for ( i = 0; i < n; i += step_i )
        {
            step_size = (out[i + step_i] - out[i])/step_i;
            for ( j = 1; j < step_i; j++)
            {
                out[i + j] = out[i] + step_size*j;
            }
        }
    }
#endif

    *val_max_ptr = val_max;
}

/* decode exponents coded with LSP coefficients (same idea as Vorbis) */
static void decode_exp_lsp(WMADecodeContext *s, int ch)
{
    fixed32 lsp_coefs[NB_LSP_COEFS];
    int val, i;

    for (i = 0; i < NB_LSP_COEFS; ++i) {
        if (i == 0 || i >= 8)
            val = get_bits(&s->gb, 3);
        else
            val = get_bits(&s->gb, 4);
        lsp_coefs[i] = lsp_codebook[i][val];
    }

    wma_lsp_to_curve(s,
                     s->exponents[ch],
                     &s->max_exponent[ch],
                     lsp_coefs);
}

/* decode exponents coded with VLC codes */
static int decode_exp_vlc(WMADecodeContext *s, int ch)
{
    int last_exp, n, code;
    const uint16_t *ptr, *band_ptr;
    fixed32 v, max_scale;
    fixed32 *q,*q_end;

    /*accommodate the 61 negative indices */
    const fixed32 *pow_10_to_yover16_ptr = &pow_10_to_yover16[61];

    band_ptr = s->exponent_bands[s->frame_len_bits - s->block_len_bits];
    ptr = band_ptr;
    q = s->exponents[ch];
    q_end = q + s->block_len;
    max_scale = 0;

    if (s->version == 1) {      //wmav1 only
        last_exp = get_bits(&s->gb, 5) + 10;
        /* XXX: use a table */
        v = pow_10_to_yover16_ptr[last_exp];
        max_scale = v;
        n = *ptr++;
        do {
            *q++ = v;
        } while (--n);
    } else {
        last_exp = 36;
    }

    while (q < q_end) {
        code = get_vlc2(&s->gb, s->exp_vlc.table, EXPVLCBITS, EXPMAX);
        if (code < 0) {
            return ERR_INVLAID_VLC;
        }
        /* NOTE: this offset is the same as MPEG4 AAC ! */
        last_exp += code - 60;
        /* XXX: use a table */
        v = pow_10_to_yover16_ptr[last_exp];
        if (v > max_scale) {
            max_scale = v;
        }
        n = *ptr++;
        do {
            *q++ = v;
        } while (--n);
    }

    s->max_exponent[ch] = max_scale;
    return ERR_NONE;
}

/* return 0 if OK. return 1 if last block of frame. return -1 if
   unrecorrable error. */
static int wma_decode_block(WMADecodeContext *s, int16_t *samples)
{
    int n, v, a, ch, code, bsize;
    int coef_nb_bits, total_gain;
    int nb_coefs[MAX_CHANNELS];
    fixed32 mdct_norm;
    fixed64  noise_seed_temp;

    //PRINTF("***decode_block: %d  (%d samples of %d in frame)\n", s->block_num, s->frame_len, s->block_len);

    /* compute current block length */
    if (s->use_variable_block_len) {   // allow_subframes
        n = av_log2(s->nb_block_sizes - 1) + 1;

        if (s->reset_block_lengths) {    // new_header
            s->reset_block_lengths = 0;
            v = get_bits(&s->gb, n);  // ratio_log2_prev
            if (v >= s->nb_block_sizes) {
                return ERR_DECODE_BLK_ERR1;
            }
            s->prev_block_len_bits = s->frame_len_bits - v;
            v = get_bits(&s->gb, n);  // ratio_log2_curr
            if (v >= s->nb_block_sizes) {
                return ERR_DECODE_BLK_ERR2;
            }
            s->block_len_bits = s->frame_len_bits - v;
        } else {
            /* update block lengths */
            s->prev_block_len_bits = s->block_len_bits;
            s->block_len_bits = s->next_block_len_bits;
        }
        v = get_bits(&s->gb, n);  // ratio_log2_next

        if (v >= s->nb_block_sizes) {
            PRINTF("Error: v was %d >= %d\n", v, s->nb_block_sizes);        //5, 7
            return ERR_DECODE_BLK_ERR3;        //this is it
        } else {
            // rb->splash(HZ, "passed v block (%d)!", v);
        }
        s->next_block_len_bits = s->frame_len_bits - v;
    } else {
        /* fixed block len */
        s->next_block_len_bits = s->frame_len_bits;
        s->prev_block_len_bits = s->frame_len_bits;
        s->block_len_bits = s->frame_len_bits;
    }
    /* now check if the block length is coherent with the frame length */
    s->block_len = 1 << s->block_len_bits;

    if ((s->block_pos + s->block_len) > s->frame_len) {
        return ERR_DECODE_BLK_ERR4;
    }

    if (s->nb_channels == 2) {
        s->ms_stereo = get_bits(&s->gb, 1);
    }
    v = 0;
    for (ch = 0; ch < s->nb_channels; ++ch) {
        a = get_bits(&s->gb, 1);
        s->channel_coded[ch] = a;   // is_channel_coded
        v |= a;
    }
    /* if no channel coded, no need to go further */
    /* XXX: fix potential framing problems */
    if (!v) {
        goto next;
    }

    bsize = s->frame_len_bits - s->block_len_bits;

    /* read total gain and extract corresponding number of bits for
       coef escape coding */
    total_gain = 1;
    for(;;) {
        a = get_bits(&s->gb, 7);
        total_gain += a;
        if (a != 127) {
            break;
        }
    }

    if (total_gain < 15)
        coef_nb_bits = 13;
    else if (total_gain < 32)
        coef_nb_bits = 12;
    else if (total_gain < 40)
        coef_nb_bits = 11;
    else if (total_gain < 45)
        coef_nb_bits = 10;
    else
        coef_nb_bits = 9;

    /* compute number of coefficients */
    n = s->coefs_end[bsize] - s->coefs_start;

    for(ch = 0; ch < s->nb_channels; ++ch) {
        nb_coefs[ch] = n;
    }
    /* complex coding */
    if (s->use_noise_coding) {
        for(ch = 0; ch < s->nb_channels; ++ch) {
            if (s->channel_coded[ch]) {
                int i, n, a;
                n = s->exponent_high_sizes[bsize];
                for(i=0;i<n;++i) {
                    a = get_bits(&s->gb, 1);
                    s->high_band_coded[ch][i] = a;
                    /* if noise coding, the coefficients are not transmitted */
                    if (a)
                        nb_coefs[ch] -= s->exponent_high_bands[bsize][i];
                }
            }
        }
        for(ch = 0; ch < s->nb_channels; ++ch) {
            if (s->channel_coded[ch]) {
                int i, n, val, code;

                n = s->exponent_high_sizes[bsize];
                val = (int)0x80000000;
                for(i=0;i<n;++i) {
                    if (s->high_band_coded[ch][i]) {
                        if (val == (int)0x80000000) {
                            val = get_bits(&s->gb, 7) - 19;
                        } else {
                            //code = get_vlc(&s->gb, &s->hgain_vlc);
                            code = get_vlc2(&s->gb, s->hgain_vlc.table, HGAINVLCBITS, HGAINMAX);
                            if (code < 0) {
                                return ERR_DECODE_BLK_ERR5;
                            }
                            val += code - 18;
                        }
                        s->high_band_values[ch][i] = val;
                    }
                }
            }
        }
    }

    /* exponents can be reused in short blocks. */
    if ((s->block_len_bits == s->frame_len_bits) || get_bits(&s->gb, 1)) {  // update_mask
        for(ch = 0; ch < s->nb_channels; ++ch) {
            if (s->channel_coded[ch]) {
                if (s->use_exp_vlc) {
                    if (decode_exp_vlc(s, ch) < 0) {
                        return ERR_DECODE_BLK_ERR6;
                    }
                } else {
                    decode_exp_lsp(s, ch);
                }
                s->exponents_bsize[ch] = bsize;
            }
        }
    }

    /* parse spectral coefficients : just RLE encoding */
    for(ch = 0; ch < s->nb_channels; ++ch) {
        if (s->channel_coded[ch]) {
            VLC *coef_vlc;
            int level, run, sign, tindex;
            int16_t *ptr, *eptr;
            const int16_t *levels_table;

            /* special VLC tables are used for ms stereo because
               there is potentially less energy there */
            tindex = (ch == 1 && s->ms_stereo);
            coef_vlc = &s->coef_vlc[tindex];
            levels_table = s->levels_table[tindex];
            /* XXX: optimize */
            //ptr = &s->coefs1[ch][0];
            ptr = samples + ch*s->block_len;  // borrow samples buffer for coefs1
            eptr = ptr + nb_coefs[ch];
            memset(ptr, 0, s->block_len * sizeof(int16_t));

            for(;;) {
                code = get_vlc2(&s->gb, coef_vlc->table, VLCBITS, VLCMAX);
                //code = get_vlc(&s->gb, coef_vlc);
                if (code < 0) {
                    return ERR_DECODE_BLK_ERR7;
                }
                if (code == 1) {
                    /* EOB */
                    break;
                } else if (code == 0) {
                    /* escape */
                    level = get_bits(&s->gb, coef_nb_bits);
                    /* NOTE: this is rather suboptimal. reading
                       block_len_bits would be better */
                    run = get_bits(&s->gb, s->frame_len_bits);
                } else {
                    /* normal code */
                    const uint16_t *p = levels_table;
                    int tmp = code-2;
                    level = 1;
                    while(tmp>=*p)
                    {
                        level++;
                        tmp -= *p;
                        p++;
                    }
                    run = tmp;
                }
                sign = get_bits(&s->gb, 1);
                if (!sign)
                    level = -level;
                ptr += run;
                if (ptr >= eptr) {
                    break;
                }
                *ptr++ = level;

                /* NOTE: EOB can be omitted */
                if (ptr >= eptr)
                    break;
            }
        }
        if (s->version == 1 && s->nb_channels >= 2) {
            align_get_bits(&s->gb);
        }
    }

    {
        int n4 = s->block_len >> 1;
        mdct_norm = 0x10000>>(s->block_len_bits-1);        //theres no reason to do a divide by two in fixed precision ...

        if (s->version == 1) {
            mdct_norm *= fixtoi32(fixsqrt32(itofix32(n4)));
        }
    }

    /* finally compute the MDCT coefficients */
    for(ch = 0; ch < s->nb_channels; ++ch) {
        if (s->channel_coded[ch]) {
            int16_t *coefs1;
            fixed32 *exponents, *exp_ptr;
            fixed32 *coefs, atemp;
            fixed32 noise;
            int i, j, n, n1, last_high_band, esize;
            fixed32 exp_power[HIGH_BAND_MAX_SIZE];

            //total_gain, coefs1, mdctnorm are lossless

            //coefs1 = s->coefs1[ch];
            coefs1 = samples + ch*s->block_len;  // borrow samples buffer for coefs1
            exponents = s->exponents[ch];
            esize = s->exponents_bsize[ch];
            coefs = s->coefs[ch];
            n=0;

          /*
          *  Previously the IMDCT was run in 17.15 precision to avoid overflow. However rare files could
          *  overflow here as well, so switch to 17.15 during coefs calculation.
          */

            if (s->use_noise_coding) {
                int inc = 0;
                /*TODO:  mult should be converted to 32 bit to speed up noise coding*/
                uint32_t temp = pow0_table[total_gain+20];
                fixed32 mult = fixdiv32((fixed32)temp,s->max_exponent[ch]);
                //mult = fixdiv64(pow_table[total_gain+20],Fixed32To64(s->max_exponent[ch]));
                mult = fixmul32(mult, mdct_norm)>>1<<pow1_table[total_gain+20];

                /* very low freqs : noise */
                for(i = 0;i < s->coefs_start; ++i) {

                    // Fix noise overflow issue by Viola Lee on 2009.01.15
                    noise_seed_temp = (fixed64)(s->noise_seed + 1)*(fixed64)(s->noise_seed);
                    s->noise_seed = noise_seed_temp>>32;
//                  s->noise_seed *= s->noise_seed + 1;

                    noise = ((int)s->noise_seed)>>s->noise_bits;
                    *coefs++ = fixmul32( (fixmul32(noise,(*exponents))>>4),mult) >>1;
                    //*coefs++ = fixmul32( (fixmul32(s->noise_table[s->noise_index],(*exponents++))>>4),Fixed32From64(mult1)) >>1;
                    //s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);
                    inc++;
                    if((inc<<bsize>>esize)>0 ) {
                        exponents += (inc<<bsize>>esize);
                        inc = 0;
                    }
                }

                n1 = s->exponent_high_sizes[bsize];

                /* compute power of high bands */
                exp_ptr = exponents +
                          s->high_band_start[bsize] -
                          s->coefs_start;
                last_high_band = 0; /* avoid warning */
                for (j=0;j<n1;++j) {
                    n = s->exponent_high_bands[s->frame_len_bits -
                                               s->block_len_bits][j];
                    if (s->high_band_coded[ch][j]) {
                        fixed32 v;
                        fixed64 e2 = 0;
                        for(i = 0;i < n; ++i) {
                            /*v is noramlized later on so its fixed format is irrelevant*/
                            v = exp_ptr[i<<bsize>>esize]>>4;
                            e2 += fixmul32(v, v)>>3;
                        }
                        exp_power[j] = (fixed32)(e2/n); /*n is an int...*/
                        last_high_band = j;
                    }
                    exp_ptr += n;
                }

                /* main freqs and high freqs */
                inc = 0;
                for(j=-1;j<n1;++j) {
                    if (j < 0) {
                        n = s->high_band_start[bsize] -
                            s->coefs_start;
                    } else {
                        n = s->exponent_high_bands[s->frame_len_bits -
                                                   s->block_len_bits][j];
                    }
                    if (j >= 0 && s->high_band_coded[ch][j]) {
                        int k = s->high_band_values[ch][j]+20;
                        fixed64 tmp1 = (fixed64)pow0_table[k]<<pow1_table[k];
                        /* use noise with specified power */
                        fixed32 tmp = fixdiv32(exp_power[j],exp_power[last_high_band]);
                        fixed64 mult1 = (fixed64)fixsqrt32(tmp);
                        /* XXX: use a table */
                        /*mult1 is 48.16, pow_table is 48.16*/
                        //mult1 = (mult1 * pow_table[s->high_band_values[ch][j]+20]) >> PRECISION;
                        fixed64 mult2 = (mult1 * tmp1) >> PRECISION;

                        /*this step has a fairly high degree of error for some reason*/
                        fixed64 mult3 = fixdiv64(mult2,fixmul32(s->max_exponent[ch],s->noise_mult));

                        fixed64 mult4 = (mult3*mdct_norm)>>PRECISION;
                        for(i = 0;i < n; ++i) {
                            // Fix noise overflow issue by Viola Lee on 2009.01.15
                            noise_seed_temp = (fixed64)(s->noise_seed + 1)*(fixed64)(s->noise_seed);
                            s->noise_seed = noise_seed_temp>>32;
//                          s->noise_seed *= s->noise_seed + 1;

                            noise = ((int)s->noise_seed)>>s->noise_bits;
                            //noise = s->noise_table[s->noise_index];
                            //s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);
                            *coefs = fixmul32((fixmul32(*exponents,noise)>>4),Fixed32From64(mult4)) >>1;
//                          if( *coefs > 0x800000 || *coefs < -0x800000 )
//                              coefs = coefs;

                            coefs++;

                            inc++;
                            if((inc<<bsize>>esize)>0 ) {
                                exponents += (inc<<bsize>>esize);
                                inc = 0;
                            }
                        }
                    } else {
                        /* coded values + small noise */
                        for (i = 0;i < n; ++i) {
                            // PJJ: check code path

                            // Fix noise overflow issue by Viola Lee on 2009.01.15
                            noise_seed_temp = (fixed64)(s->noise_seed + 1)*(fixed64)(s->noise_seed);
                            s->noise_seed = noise_seed_temp>>32;
//                          s->noise_seed *= s->noise_seed + 1;

                            noise = ((int)s->noise_seed)>>s->noise_bits;
                            //noise = s->noise_table[s->noise_index];
                            //s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);

                            /*don't forget to renormalize the noise*/
                            //temp1 = (((int32_t)*coefs1++)<<16) + (noise>>4);
                            atemp = ((*coefs1++) * mult);
                            *coefs++ = fixmul32(atemp, *exponents);
                            inc++;
                            if((inc<<bsize>>esize)>0 ) {
                                int tmp = inc<<bsize>>esize;
                                exponents += tmp;
                                inc = 0;
                            }
                        }
                    }
                }

                /* very high freqs : noise */
                n = s->block_len - s->coefs_end[bsize];
                atemp = fixmul32(mult,exponents[-1]);  /*the work around for 32.32 vars are getting stupid*/
                for (i = 0; i < n; ++i) {
                    /*renormalize the noise product and then reduce to 17.15 precison*/

                    // Fix noise overflow issue by Viola Lee on 2009.01.15
                    noise_seed_temp = (fixed64)(s->noise_seed + 1)*(fixed64)(s->noise_seed);
                    s->noise_seed = noise_seed_temp>>32;
//                  s->noise_seed *= s->noise_seed + 1;

                    noise = ((int)s->noise_seed)>>s->noise_bits;
                    *coefs++ = fixmul32(noise>>4, atemp);
                    //*coefs++ = fixmul32(s->noise_table[s->noise_index],mult2) >>5;
                    //s->noise_index = (s->noise_index + 1) & (NOISE_TAB_SIZE - 1);
                }
            } else {
                /*Noise coding not used, simply convert from exp to fixed representation*/
                uint32_t temp = pow0_table[total_gain+20];
                fixed32 mult = fixdiv32((fixed32)temp,s->max_exponent[ch]);
                //fixed32 mult3 = (fixed32)(fixdiv64(pow_table[total_gain+20],Fixed32To64(s->max_exponent[ch])));
                mult = fixmul32(mult, mdct_norm)>>1<<pow1_table[total_gain+20];

                for(i=0; i<s->coefs_start; i++)
                    *coefs++=0;

                n = nb_coefs[ch];

                /* XXX: optimize more, unrolling this loop in asm might be a good idea */

                for(i = 0;i < n; ++i) {
                    atemp = (coefs1[i] * mult);
                    *coefs++=fixmul32(atemp,exponents[i<<bsize>>esize]);
                }
                n = s->block_len - s->coefs_end[bsize];
                memset(coefs, 0, n*sizeof(fixed32));
            }
        }
    }

    if (s->ms_stereo && s->channel_coded[1]) {
        fixed32 a, b;
        int i;

        /* nominal case for ms stereo: we do it before mdct */
        /* no need to optimize this case because it should almost
           never happen */
        if (!s->channel_coded[0]) {
            memset(s->coefs[0], 0, sizeof(fixed32) * s->block_len);
            s->channel_coded[0] = 1;
        }

        for(i = 0; i < s->block_len; ++i) {
            a = s->coefs[0][i];
            b = s->coefs[1][i];
            s->coefs[0][i] = a + b;
            s->coefs[1][i] = a - b;
        }
    }

 #ifdef WMA_PERFORMANCE_TEST
      tClockSpectrumPerformance = PalGetClock();
      if(s->nFrames ==1 )
      {
          nSpectrumPerformance = 0;
      }
 
 #endif            

    #if defined(FREQINFO)
        if(s->nShowSpectrum)
        {
            updateFreqInfo(s);
        }
    #endif

    #ifdef WMA_PERFORMANCE_TEST
        nSpectrumPerformance+= PalGetDuration(tClockSpectrumPerformance);
        if ( s->nFrames % 500 == 0 )
        {
            printf("[WMA] average of spectrum %d nSpectrumPerformance %d nFrames %d \n",(nSpectrumPerformance/s->nFrames),nSpectrumPerformance,s->nFrames );
        }
    #endif


    for(ch = 0; ch < s->nb_channels; ++ch) {
        if (s->channel_coded[ch]) {
            //static int16_t  output[BLOCK_MAX_SIZE * 2];
            int16_t* output;
            int n4, index, n;

            if (s->nb_channels > 1)
                output = samples;       // borrow the samples buffer for this temp use, then save 8k
            else
                output = s->frame_out[1];  // borrow frame_out buffer because this moment the samples size
                                           // is only half of the temp buffer needed

            n = s->block_len;
            n4 = s->block_len >>1;

            ff_imdct_calc(&s->mdct_ctx[bsize],
                          s->cos_table,
                          output,
                          s->coefs[ch]);

            /* add in the frame */
            index = (s->frame_len / 2) + s->block_pos - n4;

            wma_window(s, output, &s->frame_out[ch][index]);

            /* specific fast case for ms-stereo : add to second
               channel if it is not coded */
            if (s->ms_stereo && !s->channel_coded[1]) {
                wma_window(s, output, &s->frame_out[1][index]);
            }
        }
    }
next:
    /* update block number */
    ++s->block_num;
    s->block_pos += s->block_len;
    if (s->block_pos >= s->frame_len) {
        return 1;
    } else {
        return 0;
    }
}

/* decode a frame of frame_len samples */
static int wma_decode_frame(WMADecodeContext *s, int16_t *samples)
{
    int ret, i, n, ch, incr;
    int16_t *ptr;
    int16_t *iptr;
    // rb->splash(HZ, "in wma_decode_frame");

    /* read each block */
    s->block_num = 0;
    s->block_pos = 0;

    for(;;) {
        ret = wma_decode_block(s, samples);
        if (ret < 0) {
            PRINTF("wma_decode_block failed with code %d\n", ret);
            return ret;
        }
        if (ret) {
            break;
        }
    }

    /* return frame with full 30-bit precision */
    n = s->frame_len;
    incr = s->nb_channels;
    for(ch = 0; ch < s->nb_channels; ++ch) {
        ptr = samples + ch;
        iptr = s->frame_out[ch];

        for (i=0;i<n;++i) {
            *ptr = (*iptr++);
            ptr += incr;
        }
        /* prepare for next block */
        memmove(&s->frame_out[ch][0], &s->frame_out[ch][s->frame_len],
                s->frame_len * sizeof(int16_t));
    }

    return ERR_NONE;
}

/* Initialise the superframe decoding */

int wma_decode_superframe_init(WMADecodeContext* s,
                                 uint8_t *buf,  /*input*/
                                 int buf_size)
{
    if (buf_size==0) {
        s->last_superframe_len = 0;
        return 0;
    }

    s->current_frame = 0;

    init_get_bits(&s->gb, buf, buf_size*8);

    if (s->use_bit_reservoir) {
        /* read super frame header */
        get_bits(&s->gb, 4); /* super frame index */
        s->nb_frames = get_bits(&s->gb, 4);

        /* If superframe counter equal to zero, go to next packet by Viola Lee on 2009.01.22 */
        if(0 == s->nb_frames)
        {
            return ERR_NB_FRAME_ZERO;
        }

        if (s->last_superframe_len == 0)
            s->nb_frames --;
        else if (s->nb_frames == 0)
            s->nb_frames++;

        s->bit_offset = get_bits(&s->gb, s->byte_offset_bits + 3);
    } else {
        s->nb_frames = 1;
    }

    return 1;
}

/* Decode a single frame in the current superframe - return -1 if
   there was a decoding error, or the number of samples decoded.
*/

int wma_decode_superframe_frame(WMADecodeContext* s,
                                int16_t* samples, /*output*/
                                uint8_t *buf,  /*input*/
                                int buf_size)
{
    int pos, len;
    uint8_t *q;
    int done = 0;
    int err  = ERR_NONE;

    if ((s->use_bit_reservoir) && (s->current_frame == 0)) {
        if (s->last_superframe_len > 0) {
            /* add s->bit_offset bits to last frame */
            if ((s->last_superframe_len + ((s->bit_offset + 7) >> 3)) >
                 MAX_CODED_SUPERFRAME_SIZE) {
                err = ERR_EXCEED_MAX_CODED_SUPERFRAME_SIZE;
                PRINTF("superframe size too large error\n");
                goto fail;
            }
            q = s->last_superframe + s->last_superframe_len;
            len = s->bit_offset;
            while (len > 0) {
                *q++ = (get_bits)(&s->gb, 8);
                len -= 8;
            }
            if (len > 0) {
                *q++ = (get_bits)(&s->gb, len) << (8 - len);
            }

            /* XXX: s->bit_offset bits into last frame */
            init_get_bits(&s->gb, s->last_superframe, MAX_CODED_SUPERFRAME_SIZE*8);
            /* skip unused bits */
            if (s->last_bitoffset > 0)
                skip_bits(&s->gb, s->last_bitoffset);

            /* this frame is stored in the last superframe and in the
               current one */
            if ((err = wma_decode_frame(s, samples)) < 0) {
                goto fail;
            }
            done = 1;
        }

        /* read each frame starting from s->bit_offset */
        pos = s->bit_offset + 4 + 4 + s->byte_offset_bits + 3;
        init_get_bits(&s->gb, buf + (pos >> 3), (MAX_CODED_SUPERFRAME_SIZE - (pos >> 3))*8);
        len = pos & 7;
        if (len > 0)
            skip_bits(&s->gb, len);

        s->reset_block_lengths = 1;
    }

    /* If we haven't decoded a frame yet, do it now */
    if (!done) {
        if ((err = wma_decode_frame(s, samples)) < 0) {
            goto fail;
        }
    }

    s->current_frame++;

    if ((s->use_bit_reservoir) && (s->current_frame == s->nb_frames)) {
        /* we copy the end of the frame in the last frame buffer */
        pos = get_bits_count(&s->gb) + ((s->bit_offset + 4 + 4 + s->byte_offset_bits + 3) & ~7);
        s->last_bitoffset = pos & 7;
        pos >>= 3;
        len = buf_size - pos;
        if (len > MAX_CODED_SUPERFRAME_SIZE || len < 0) {
            err = ERR_EXCEED_MAX_CODED_SUPERFRAME_SIZE;
            PRINTF("superframe size too large error after decodeing\n");
            goto fail;
        }
        s->last_superframe_len = len;
        memcpy(s->last_superframe, buf + pos, len);
    }

    return s->frame_len;

fail:
    /* when error, we reset the bit reservoir */
    s->last_superframe_len = 0;
    return err;
}

