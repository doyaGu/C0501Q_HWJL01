/*!
*  \file block.c
*
* \brief block module decode-side utility functions.
*
*  Part of the Spectral Extension Module.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "ac3dec.h"
#include "math.h"
#include "imdct.h"

/**
 * Table of bin locations for rematrixing bands
 * reference: Section 7.5.2 Rematrixing : Frequency Band Definitions
 */
const uint8_t ff_ac3_rematrix_band_tab[5] = { 13, 25, 37, 61, 253 };
extern nFrames;
/* dialog normalization table */
/*! Table of dialnorm exponent values */
static int dialexp[32] =
{   0, -5,
    -4, -4, -4, -4, -4, -4,
    -3, -3, -3, -3, -3, -3,
    -2, -2, -2, -2, -2, -2,
    -1, -1, -1, -1, -1, -1,
    0, 0, 0, 0, 0, 0 
};


/*! Table of dialnorm mantissa values */
static int dialmant[32] =
{   0x00008000, 0x00008000,
    0x000047D6, 0x000050A3, 0x00005A82, 0x00006598, 0x00007209, 0x00008000,
    0x000047D6, 0x000050A3, 0x00005A82, 0x00006598, 0x00007209, 0x00008000,
    0x000047D6, 0x000050A3, 0x00005A82, 0x00006598, 0x00007209, 0x00008000,
    0x000047D6, 0x000050A3, 0x00005A82, 0x00006598, 0x00007209, 0x00008000,
    0x000047D6, 0x000050A3, 0x00005A82, 0x00006598, 0x00007209, 0x00008000
};

static const int chanbittab[8 * 2] =
{
    0xa000,     /*!< 1+1 mode */
    0x4000,     /*!< 1/0 mode */
    0xa000,     /*!< 2/0 mode */
    0xe000,     /*!< 3/0 mode */
    0xb000,     /*!< 2/1 mode */
    0xf000,     /*!< 3/1 mode */
    0xb800,     /*!< 2/2 mode */
    0xf800,     /*!< 3/2 mode */
    0xa400,     /*!< 1+1 mode w/lfe */
    0x4400,     /*!< 1/0 mode w/lfe */
    0xa400,     /*!< 2/0 mode w/lfe */
    0xe400,     /*!< 3/0 mode w/lfe */
    0xb400,     /*!< 2/1 mode w/lfe */
    0xf400,     /*!< 3/1 mode w/lfe */
    0xbc00,     /*!< 2/2 mode w/lfe */
    0xfc00      /*!< 3/2 mode w/lfe */
};

const uint8_t ff_ac3_ungroup_3_in_7_bits_tab[128][3] = {
    {0, 0, 0}, {0, 0, 1}, {0, 0, 2}, {0, 0, 3}, {0, 0, 4}, {0, 1, 0}, {0, 1, 1}, {0, 1, 2}, 
    {0, 1, 3}, {0, 1, 4}, {0, 2, 0}, {0, 2, 1}, {0, 2, 2}, {0, 2, 3}, {0, 2, 4}, {0, 3, 0}, 
    {0, 3, 1}, {0, 3, 2}, {0, 3, 3}, {0, 3, 4}, {0, 4, 0}, {0, 4, 1}, {0, 4, 2}, {0, 4, 3}, 
    {0, 4, 4}, {1, 0, 0}, {1, 0, 1}, {1, 0, 2}, {1, 0, 3}, {1, 0, 4}, {1, 1, 0}, {1, 1, 1}, 
    {1, 1, 2}, {1, 1, 3}, {1, 1, 4}, {1, 2, 0}, {1, 2, 1}, {1, 2, 2}, {1, 2, 3}, {1, 2, 4}, 
    {1, 3, 0}, {1, 3, 1}, {1, 3, 2}, {1, 3, 3}, {1, 3, 4}, {1, 4, 0}, {1, 4, 1}, {1, 4, 2}, 
    {1, 4, 3}, {1, 4, 4}, {2, 0, 0}, {2, 0, 1}, {2, 0, 2}, {2, 0, 3}, {2, 0, 4}, {2, 1, 0}, 
    {2, 1, 1}, {2, 1, 2}, {2, 1, 3}, {2, 1, 4}, {2, 2, 0}, {2, 2, 1}, {2, 2, 2}, {2, 2, 3}, 
    {2, 2, 4}, {2, 3, 0}, {2, 3, 1}, {2, 3, 2}, {2, 3, 3}, {2, 3, 4}, {2, 4, 0}, {2, 4, 1}, 
    {2, 4, 2}, {2, 4, 3}, {2, 4, 4}, {3, 0, 0}, {3, 0, 1}, {3, 0, 2}, {3, 0, 3}, {3, 0, 4}, 
    {3, 1, 0}, {3, 1, 1}, {3, 1, 2}, {3, 1, 3}, {3, 1, 4}, {3, 2, 0}, {3, 2, 1}, {3, 2, 2}, 
    {3, 2, 3}, {3, 2, 4}, {3, 3, 0}, {3, 3, 1}, {3, 3, 2}, {3, 3, 3}, {3, 3, 4}, {3, 4, 0}, 
    {3, 4, 1}, {3, 4, 2}, {3, 4, 3}, {3, 4, 4}, {4, 0, 0}, {4, 0, 1}, {4, 0, 2}, {4, 0, 3}, 
    {4, 0, 4}, {4, 1, 0}, {4, 1, 1}, {4, 1, 2}, {4, 1, 3}, {4, 1, 4}, {4, 2, 0}, {4, 2, 1}, 
    {4, 2, 2}, {4, 2, 3}, {4, 2, 4}, {4, 3, 0}, {4, 3, 1}, {4, 3, 2}, {4, 3, 3}, {4, 3, 4}, 
    {4, 4, 0}, {4, 4, 1}, {4, 4, 2}, {4, 4, 3}, {4, 4, 4}, {5, 0, 0}, {5, 0, 1}, {5, 0, 2} 
};

/**
 * Decode the grouped exponents according to exponent strategy.
 * reference: Section 7.1.3 Exponent Decoding
 */
static int decode_exponents(GetBitContext *gbc, int exp_strategy, int ngrps,
                            uint8_t absexp, int8_t *dexps)
{
    int i, j, grp, group_size;
    int dexp[256];
    int expacc, prevexp;

    /* unpack groups */
    group_size = exp_strategy + (exp_strategy == EXP_D45);
    for(grp=0,i=0; grp<ngrps; grp++) {
        expacc = get_bits(gbc, 7);
        dexp[i++] = ff_ac3_ungroup_3_in_7_bits_tab[expacc][0];
        dexp[i++] = ff_ac3_ungroup_3_in_7_bits_tab[expacc][1];
        dexp[i++] = ff_ac3_ungroup_3_in_7_bits_tab[expacc][2];
    }

    /* convert to absolute exps and expand groups */
    prevexp = absexp;
    for(i=0,j=0; i<ngrps*3; i++) {
        prevexp += dexp[i] - 2;
        if (prevexp > 24U)
            return -1;
        switch (group_size) {
            case 4: dexps[j++] = prevexp;
                    dexps[j++] = prevexp;
            case 2: dexps[j++] = prevexp;
            case 1: dexps[j++] = prevexp;
        }
    }
    return 0;
}

static void mant_ch_unp(AC3DecodeContext *s, int blk, int ch, mant_groups *m)                                  
{
    if (!s->channel_uses_aht[ch]) {
        ac3_mant_ch_unp(s, ch, m);
#ifdef CONFIG_EAC3_DECODER
    } else {
        /* if AHT is used, mantissas for all blocks are encoded in the first
           block of the frame. */
        int bin;
        if (!blk)
            aht_mant_ch_unp(s, ch);
        for (bin = s->start_freq[ch]; bin < s->end_freq[ch]; bin++) {
            s->fixed_coeffs[ch][bin] = s->pre_mantissa[ch][bin][blk];// >> s->dexps[ch][bin];
        }
#endif
    }
}

static int rematrix_unp(AC3DecodeContext *s, int blk)
{
    int bnd;
    int channel_mode = s->channel_mode;
    int cpl_in_use = s->cpl_in_use[blk];
    GetBitContext *gbc = &s->gbc;

    /* stereo rematrixing strategy and band structure */
    if (channel_mode == AC3_CHMODE_STEREO) {
        if(s->eac3) {
            if(blk==0||get_bits1(gbc)) {
                s->num_rematrixing_bands = 0;
                if (cpl_in_use) {
                    for (bnd = 0; bnd < 4; bnd++) {
                        if (ff_ac3_rematrix_band_tab[bnd] < s->start_freq[CPL_CH])
                            s->num_rematrixing_bands++;
                        else
                            break;
                    }
                } else if(s->spx_in_use && s->spx_beg_subbnd <= 3) {
                    s->num_rematrixing_bands = 3;
                } else
                    s->num_rematrixing_bands = 4;

                for (bnd = 0; bnd < s->num_rematrixing_bands; bnd++)
                    s->rematrixing_flags[bnd] = get_bits1(gbc);
            }
        } else if(get_bits1(gbc)) {
            s->num_rematrixing_bands = 4;
            if (cpl_in_use && s->start_freq[CPL_CH] <= 61) {
                s->num_rematrixing_bands -= 1 + (s->start_freq[CPL_CH] == 37);
            } else if (s->spx_in_use && s->spx_beg_subbnd <= 3) {
                s->num_rematrixing_bands--;
            }
            for(bnd=0; bnd<s->num_rematrixing_bands; bnd++)
                s->rematrixing_flags[bnd] = get_bits1(gbc);
        }
    }
    return 0;
}

static int exp_unp(AC3DecodeContext *s, int blk)
{
    int ch;
    int fbw_channels = s->fbw_channels;
    int cpl_in_use = s->cpl_in_use[blk];
    GetBitContext *gbc = &s->gbc;

    /* exponent strategies for each channel */
    for (ch = !cpl_in_use; ch <= s->channels; ch++) {
        if (!s->eac3)
            s->exp_strategy[blk][ch] = get_bits(gbc, 2 - (ch == s->lfe_ch));
        if(s->exp_strategy[blk][ch] != EXP_REUSE)
            s->bit_alloc_stages[ch] = 3;
    }

    /* channel bandwidth */
    for (ch = 1; ch <= fbw_channels; ch++) {
        s->start_freq[ch] = 0;
        s->babnd_start[ch] = 0;
        if (s->exp_strategy[blk][ch] != EXP_REUSE) {
            int group_size;
            int prev = s->end_freq[ch];
            if (s->channel_in_cpl[ch])
                s->end_freq[ch] = s->start_freq[CPL_CH];
            else if (s->channel_uses_spx[ch])
                s->end_freq[ch] = s->spx_beg_subbnd*12 + 25;
            else {
                int bandwidth_code = get_bits(gbc, 6);
                if (bandwidth_code > 60) {
                    PRINTF("bandwidth code = %d > 60\n", bandwidth_code);
                    return -1;
                }
                s->end_freq[ch] = bandwidth_code * 3 + 73;
            }
            group_size = 3 << (s->exp_strategy[blk][ch] - 1);
            s->num_exp_groups[ch] = (s->end_freq[ch]+group_size-4) / group_size;
            if(blk > 0 && s->end_freq[ch] != prev)
                memset(s->bit_alloc_stages, 3, AC3_MAX_CHANNELS);
        }
    }
    if (cpl_in_use && s->exp_strategy[blk][CPL_CH] != EXP_REUSE) {
        s->num_exp_groups[CPL_CH] = (s->end_freq[CPL_CH] - s->start_freq[CPL_CH]) /
                                    (3 << (s->exp_strategy[blk][CPL_CH] - 1));
    }

    /* decode exponents for each channel */
    for (ch = !cpl_in_use; ch <= s->channels; ch++) {
        if (s->exp_strategy[blk][ch] != EXP_REUSE) {
            s->dexps[ch][0] = get_bits(gbc, 4) << !ch;
            if (decode_exponents(gbc, s->exp_strategy[blk][ch],
                                 s->num_exp_groups[ch], s->dexps[ch][0],
                                 &s->dexps[ch][s->start_freq[ch]+!!ch])) {
                PRINTF("exponent out-of-range\n");
                return -1;
            }
            if(ch != CPL_CH && ch != s->lfe_ch)
                skip_bits(gbc, 2); /* skip gainrng */
        }
    }
    return 0;
}

/**
 * Stereo rematrixing.
 * reference: Section 7.5.4 Rematrixing : Decoding Technique
 */
static void do_rematrixing(AC3DecodeContext *s)
{
    int bnd, i;
    int end, bndend;

    end = FFMIN(s->end_freq[1], s->end_freq[2]);

    for(bnd=0; bnd<s->num_rematrixing_bands; bnd++) {
        if(s->rematrixing_flags[bnd]) {
            bndend = FFMIN(end, ff_ac3_rematrix_band_tab[bnd+1]);
            for(i=ff_ac3_rematrix_band_tab[bnd]; i<bndend; i++) {
                int tmp0 = s->fixed_coeffs[1][i];
                s->fixed_coeffs[1][i] += s->fixed_coeffs[2][i];
                s->fixed_coeffs[2][i]  = tmp0 - s->fixed_coeffs[2][i];
            }
        }
    }
}

/**
 * unpack the transform coefficients.
 */
static void mant_unp(AC3DecodeContext *s, int blk)
{
    int ch, end;
    int got_cplchan = 0;
    mant_groups m;

    m.b1 = m.b2 = m.b4 = 0;

    for (ch = 1; ch <= s->channels; ch++) {
        /* transform coefficients for full-bandwidth channel */
        mant_ch_unp(s, blk, ch, &m);
        /* tranform coefficients for coupling channel come right after the
           coefficients for the first coupled channel*/
        if (s->channel_in_cpl[ch])  {
            if (!got_cplchan) {
                mant_ch_unp(s, blk, CPL_CH, &m);
                if(s->ecplinu)
                    ecpd_decpamponly(s);
                else
                    cpld_decouple(s);

                got_cplchan = 1;
            }
            end = s->end_freq[CPL_CH];
        } else {
            end = s->end_freq[ch];
        }
        do
            s->fixed_coeffs[ch][end] = 0;
        while(++end < 256);
    }
}

static void do_dynamic_range(AC3DecodeContext *s)
{
    int ch,i, fact;
    int gainexp[2], gainmant[2];
    int comp, dyng;

    for(i = 0;i<2-(s->channel_mode != AC3_CHMODE_DUALMONO) ;i++) {
        comp = s->compr[i]<<8;
        dyng = s->dynamic_range[i]<<7;
        switch (s->dev_compmode)
        {
            case AC3_COMP_RF:               /* RF mode */
                if (s->compre[i]) {
                    if (comp)
                        fact = comp + PLUS11DB;
                    else
                        fact = dyng + PLUS11DB;
                }
                else if (s->dnmix_active)
                    fact = dyng + MINUS11DB;
                else
                    fact = dyng;
                break;
            case AC3_COMP_LINE:             /* line out mode */
                if (dyng > 0) {
                    fact = dyng * PERCENT2FRACT_DRCMAP(s->dev_dynscalelow);
                    fact >>= 15;
                }
                else if(s->dnmix_active)
                    fact = dyng;
                else {
                    fact = dyng * PERCENT2FRACT_DRCMAP(s->dev_dynscalehigh);
                    fact >>= 15;
                }
                break;
            case AC3_COMP_CUSTOM_0:         /* custom modes */
            case AC3_COMP_CUSTOM_1:
                if (dyng > 0) {
                    fact = dyng * PERCENT2FRACT_DRCMAP(s->dev_dynscalelow);
                    fact >>= 15;
                } else {
                    fact = dyng * PERCENT2FRACT_DRCMAP(s->dev_dynscalehigh);
                    fact >>= 15;
                }
                if (s->dnmix_active)
                    fact += MINUS11DB;
                break;
        }

        /* Grab top 4 bits of compfact and add 1 to get exponent */
        gainexp[i] = (fact>>12) + 1;
        /* Grab bottom 4-bits which represent the mantissa  */
        /* "or" with 0x40000000L adds leading 0.1 to mantissa   */
        gainmant[i] = ((fact<<2) & 0x3fffL) | 0x4000L;

        if (s->dev_compmode != AC3_COMP_CUSTOM_0) {
            gainexp[i] += dialexp[s->dialnorm[i]];
            gainmant[i] = gainmant[i] * dialmant[s->dialnorm[i]];
            gainmant[i] >>= 15;
        }
        gainmant[i] <<= 15;
    }

    for(ch=1; ch<=s->channels; ch++) {
        int j;
        level_t gain;
        int exp;
        if(s->channel_mode == AC3_CHMODE_DUALMONO) {
            gain = gainmant[ch-1]>>4;
            exp = gainexp[ch-1];
        }
        else {
            gain = gainmant[0]>>4;
            exp = gainexp[0];
        }

        for(j=0; j<256; j++) {
            int shift = exp - s->dexps[ch][j];
            if(shift>0)
                s->fixed_coeffs[ch][j]<<= shift;
            else
                s->fixed_coeffs[ch][j]>>= (-shift);
            s->fixed_coeffs[ch][j] = MUL_Shift_18(s->fixed_coeffs[ch][j], gain, (LEVEL_SHIFT-(31-QUANT_SHIFT))) ; // shift s7.24 to s.31
        }
    }

}

/********************************************************
* handle_acmod_chg:
*
* This function flushes delay buffers for any channel
* that is turning off due to a transition in acmod.
********************************************************/
void handle_acmod_chg(AC3DecodeContext *s)
{
    int i, inchan, outchan, last_acmod_chane, acmod_chane, chanebit;
    int out_channels = ff_ac3_channels_tab[s->dev_mode] + s->dev_lfeon;

    /* get channel exist bit flags for last and current acmod */
    if (s->last_channel_mode == -1)
    {
        s->last_channel_mode = s->channel_mode;
        s->last_lfe_on = s->lfe_on;
        memcpy(s->last_downmix, s->downmix_coeffs, sizeof(s->downmix_coeffs));
        return;
    }
    
    acmod_chane = chanbittab[s->channel_mode + (s->lfe_on * 8)];
    last_acmod_chane = chanbittab[s->last_channel_mode + (s->last_lfe_on * 8)];

    /* iterate over all channels, including LFE */
    for (inchan=0, chanebit = 0x8000; inchan<AC3_MAX_CHANNELS-1; inchan++, chanebit >>= 1)
    {
        /* if this channel is turning off */
        if ((chanebit & last_acmod_chane) && !(chanebit & acmod_chane))
        {
            /* flush delay buffers by sending zeros into woad_decode */
            memset(s->work_buf[inchan], 0, AC3_BLOCK_SIZE * sizeof(int));

            /* perform window/overlap-add decode to get back to PCM */
            woad_decode( s->work_buf[inchan], s->delay[inchan]);

            /* do not perform TPNP decoding since this is a program change */

            /* disable downmix coefficient cross-fading for this channel since
             * it will already be smoothly faded-out by window/overlap-add decode
             */
            for (i = 0; i < out_channels; i++) {
                outchan = ff_channeltab[s->dev_mode][i];
                s->downmix_coeffs[inchan][outchan] = s->last_downmix[inchan][outchan];
            }
            /* downmix current channel to PCM buffers.
             *
             * inchan is a static channel index (i.e. L=0, C=1, R=2, Ls=3, Rs=4)
             * and is not relative to any acmod.  therefore, pass in acmod 3/2
             * so that ch identifies correct coded channel to be downmixed.
             */
            ac3_downmix(s, inchan);
        }
        else if (!(chanebit & last_acmod_chane) && (chanebit & acmod_chane))
        {
            /* channel is turning on */

            /* disable downmix coefficient cross-fading for this channel since
             * it will already be smoothly faded-in by window/overlap-add decode
             */
            for (i = 0; i < out_channels; i++) {
                outchan = ff_channeltab[s->dev_mode][i];
                s->last_downmix[inchan][outchan] = s->downmix_coeffs[inchan][outchan];
            }
        }
    }
}

/**
 * Inverse MDCT Transform.
 * Convert frequency domain coefficients to time-domain audio samples.
 * reference: Section 7.9.4 Transformation Equations
 */
static __inline void do_imdct(AC3DecodeContext *s, int ch)
{
    if (s->block_switch[ch]) {
        ac3_imdct_256(s->fixed_coeffs[ch], s->delay[ch], s->work_buf[ch]);
    } else {
        ac3_imdct_512(s->fixed_coeffs[ch], s->delay[ch], s->work_buf[ch]);
	}
}

/**
 * Decode a single audio block from the AC-3 bitstream.
 */
int decode_audio_block(AC3DecodeContext *s, int blk)
{
    int fbw_channels = s->fbw_channels;
    int channel_mode = s->channel_mode;
    int i, ch;
    int cpl_in_use = s->cpl_in_use[blk];
    GetBitContext *gbc = &s->gbc;

    memset(s->bit_alloc_stages, 0, AC3_MAX_CHANNELS);

    /* block switch flags */
    if (s->block_switch_syntax) {
        for (i = 0; i < fbw_channels; i++) {
            ch = ff_channeltab[s->channel_mode][i];
            s->block_switch[ch] = get_bits1(gbc);
        }
    }

    /* dithering flags */
    if (s->dither_flag_syntax) {
        for (ch = 1; ch <= fbw_channels; ch++) {
            s->dither_flag[ch] = get_bits1(gbc);
        }
    }

    /* dynamic range */
    ch = !(s->channel_mode);
    for(i=0;i<=!(s->channel_mode);i++) {
        if(get_bits1(gbc))
            s->dynamic_range[i] = get_sbits(gbc, 8);
        else if(blk == 0)
            s->dynamic_range[i] = 0;
    }

    spx_unp(s, blk);
    cpl_unp(s, blk);
    rematrix_unp(s, blk);
    exp_unp(s, blk);
    bitalloc_unp(s, blk);

    /* unused dummy data */
    if (s->skip_syntax && get_bits1(gbc)) {
        int skipl = get_bits(gbc, 9);
        while(skipl--)
            skip_bits(gbc, 8);
    }

    // unpack mantissa
    mant_unp(s, blk);

    /* apply scaling to coefficients (headroom, dynrng) */
    do_dynamic_range(s);

    /* recover coefficients if rematrixing is in use */
    if(s->channel_mode == AC3_CHMODE_STEREO)
        do_rematrixing(s);

    for (ch = 1; ch <= s->channels; ch++) {
        if(s->channel_uses_spx[ch]) {
            spx_synthesizetcs(s, ch);
        }
    }

    for(i=0;i<AC3_MAX_CHANNELS-1;i++)
        s->dnmixbuf_used[i] = 0; // clear downmix output buffer flag

    // extend coeffs to identical position
    for(i=0; i<s->channels; i++)
    {
        ch = ff_channeltab[s->channel_mode][i];
        if(ch!=i+1)
            memcpy(s->fixed_coeffs[ch], s->fixed_coeffs[i+1], AC3_MAX_COEFS*sizeof(int)); 
    }

    if ((s->channel_mode != s->last_channel_mode) ||
        (s->lfe_on != s->last_lfe_on)) {
        handle_acmod_chg(s);
    }

    for(i=0;i<s->channels;i++)
    {
#if 1
        ch = ff_channeltab[s->channel_mode][i];
        do_imdct(s, ch);
        if(ch != LOW_FREQUENCY)
            eac3_tpnp_decode(s, blk, ch);
        ac3_downmix(s, ch);
#else
        ch = ff_channeltab[s->channel_mode][i];
        /* Perform inverse MDCT */
        xfmd_imdct(
            s->block_switch[ch],        /* input    */
            s->work_buf[ch],    /* modify   */
            s->fixed_coeffs[ch]);   /* modify   */                                  

        woad_decode_dol(
            100,                /* input    */
            s->fixed_coeffs[ch],        /* input    */
            s->delay[ch],   /* modify   */
            s->work_buf[ch],                /* output   */
            0);

        if(ch != LOW_FREQUENCY)
            eac3_tpnp_decode(s, blk, ch);
        ac3_downmix(s, ch);
#endif

    }

    s->last_channel_mode = s->channel_mode;
    s->last_lfe_on = s->lfe_on;
    memcpy(s->last_downmix, s->downmix_coeffs, sizeof(s->downmix_coeffs));

    return 0;
}
