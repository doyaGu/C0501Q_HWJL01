/*!
*  \file bitalloc.c
*
* \brief bit alloc module decode-side utility functions.
*
*  Part of the Spectral Extension Module.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "ac3dec.h"

/*! Bit allocation band-to-start bin translation table */
static uint8_t babndtab[50] =
{	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 31, 34, 37, 40, 43, 46, 49, 55,
	61, 67, 73, 79, 85, 97, 109, 121, 133, 157, 181, 205, 229, 253 };

static const uint16_t ff_ac3_hearing_threshold_tab[AC3_CRITICAL_BANDS][3]= {
{ 0x04d0,0x04f0,0x0580 },
{ 0x04d0,0x04f0,0x0580 },
{ 0x0440,0x0460,0x04b0 },
{ 0x0400,0x0410,0x0450 },
{ 0x03e0,0x03e0,0x0420 },
{ 0x03c0,0x03d0,0x03f0 },
{ 0x03b0,0x03c0,0x03e0 },
{ 0x03b0,0x03b0,0x03d0 },
{ 0x03a0,0x03b0,0x03c0 },
{ 0x03a0,0x03a0,0x03b0 },
{ 0x03a0,0x03a0,0x03b0 },
{ 0x03a0,0x03a0,0x03b0 },
{ 0x03a0,0x03a0,0x03a0 },
{ 0x0390,0x03a0,0x03a0 },
{ 0x0390,0x0390,0x03a0 },
{ 0x0390,0x0390,0x03a0 },
{ 0x0380,0x0390,0x03a0 },
{ 0x0380,0x0380,0x03a0 },
{ 0x0370,0x0380,0x03a0 },
{ 0x0370,0x0380,0x03a0 },
{ 0x0360,0x0370,0x0390 },
{ 0x0360,0x0370,0x0390 },
{ 0x0350,0x0360,0x0390 },
{ 0x0350,0x0360,0x0390 },
{ 0x0340,0x0350,0x0380 },
{ 0x0340,0x0350,0x0380 },
{ 0x0330,0x0340,0x0380 },
{ 0x0320,0x0340,0x0370 },
{ 0x0310,0x0320,0x0360 },
{ 0x0300,0x0310,0x0350 },
{ 0x02f0,0x0300,0x0340 },
{ 0x02f0,0x02f0,0x0330 },
{ 0x02f0,0x02f0,0x0320 },
{ 0x02f0,0x02f0,0x0310 },
{ 0x0300,0x02f0,0x0300 },
{ 0x0310,0x0300,0x02f0 },
{ 0x0340,0x0320,0x02f0 },
{ 0x0390,0x0350,0x02f0 },
{ 0x03e0,0x0390,0x0300 },
{ 0x0420,0x03e0,0x0310 },
{ 0x0460,0x0420,0x0330 },
{ 0x0490,0x0450,0x0350 },
{ 0x04a0,0x04a0,0x03c0 },
{ 0x0460,0x0490,0x0410 },
{ 0x0440,0x0460,0x0470 },
{ 0x0440,0x0440,0x04a0 },
{ 0x0520,0x0480,0x0460 },
{ 0x0800,0x0630,0x0440 },
{ 0x0840,0x0840,0x0450 },
{ 0x0840,0x0840,0x04e0 },
};

static const uint8_t ff_ac3_log_add_tab[260]= {
0x40,0x3f,0x3e,0x3d,0x3c,0x3b,0x3a,0x39,0x38,0x37,
0x36,0x35,0x34,0x34,0x33,0x32,0x31,0x30,0x2f,0x2f,
0x2e,0x2d,0x2c,0x2c,0x2b,0x2a,0x29,0x29,0x28,0x27,
0x26,0x26,0x25,0x24,0x24,0x23,0x23,0x22,0x21,0x21,
0x20,0x20,0x1f,0x1e,0x1e,0x1d,0x1d,0x1c,0x1c,0x1b,
0x1b,0x1a,0x1a,0x19,0x19,0x18,0x18,0x17,0x17,0x16,
0x16,0x15,0x15,0x15,0x14,0x14,0x13,0x13,0x13,0x12,
0x12,0x12,0x11,0x11,0x11,0x10,0x10,0x10,0x0f,0x0f,
0x0f,0x0e,0x0e,0x0e,0x0d,0x0d,0x0d,0x0d,0x0c,0x0c,
0x0c,0x0c,0x0b,0x0b,0x0b,0x0b,0x0a,0x0a,0x0a,0x0a,
0x0a,0x09,0x09,0x09,0x09,0x09,0x08,0x08,0x08,0x08,
0x08,0x08,0x07,0x07,0x07,0x07,0x07,0x07,0x06,0x06,
0x06,0x06,0x06,0x06,0x06,0x06,0x05,0x05,0x05,0x05,
0x05,0x05,0x05,0x05,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x03,0x03,0x03,0x03,0x03,
0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x02,
0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,
0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

static const uint8_t ff_eac3_hebap_tab[64] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 8,
    8, 8, 9, 9, 9, 10, 10, 10, 10, 11,
    11, 11, 11, 12, 12, 12, 12, 13, 13, 13,
    13, 14, 14, 14, 14, 15, 15, 15, 15, 16,
    16, 16, 16, 17, 17, 17, 17, 18, 18, 18,
    18, 18, 18, 18, 18, 19, 19, 19, 19, 19,
    19, 19, 19, 19,
};

/**
 * Starting frequency coefficient bin for each critical band.
 */
static const uint8_t ff_ac3_band_start_tab[AC3_CRITICAL_BANDS+1] = {
      0,  1,   2,   3,   4,   5,   6,   7,   8,   9,
     10,  11, 12,  13,  14,  15,  16,  17,  18,  19,
     20,  21, 22,  23,  24,  25,  26,  27,  28,  31,
     34,  37, 40,  43,  46,  49,  55,  61,  67,  73,
     79,  85, 97, 109, 121, 133, 157, 181, 205, 229, 253
};

/**
 * Map each frequency coefficient bin to the critical band that contains it.
 */
static const uint8_t ff_ac3_bin_to_band_tab[253] = {
     0,
     1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30,
    31, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34, 34,
    35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36,
    37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38,
    39, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 40,
    41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
    42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
    43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
    44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
    45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
    45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
    47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
    47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
    48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
    48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
    49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
    49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49
};

static __inline int calc_lowcomp1(int a, int b0, int b1, int c)
{
    if ((b0 + 256) == b1) {
        a = c;
    } else if (b0 > b1) {
        a = FFMAX(a - 64, 0);
    }
    return a;
}

static __inline int calc_lowcomp(int a, int b0, int b1, int bin)
{
    if (bin < 7) {
        return calc_lowcomp1(a, b0, b1, 384);
    } else if (bin < 20) {
        return calc_lowcomp1(a, b0, b1, 320);
    } else {
        return FFMAX(a - 128, 0);
    }
}

static void ff_ac3_bit_alloc_calc_psd(int8_t *exp, int start, int end, int16_t *psd,
                               int16_t *band_psd)
{
    int bin, band;

    /* exponent mapping to PSD */
    for (bin = start; bin < end; bin++) {
        psd[bin]=(3072 - (exp[bin] << 7));
    }

    /* PSD integration */
    bin  = start;
    band = ff_ac3_bin_to_band_tab[start];
    do {
        int v = psd[bin++];
        int band_end = FFMIN(ff_ac3_band_start_tab[band+1], end);
        for (; bin < band_end; bin++) {
            int max = FFMAX(v, psd[bin]);
            /* logadd */
            int adr = FFMIN(max - ((v + psd[bin] + 1) >> 1), 255);
            v = max + ff_ac3_log_add_tab[adr];
        }
        band_psd[band++] = v;
    } while (end > ff_ac3_band_start_tab[band]);
}

static int ff_ac3_bit_alloc_calc_mask(AC3BitAllocParameters *s, int16_t *band_psd,
                               int start, int end, int band_start,
							   int fast_gain, int is_lfe,
                               int dba_mode, int dba_nsegs, uint8_t *dba_offsets,
                               uint8_t *dba_lengths, uint8_t *dba_values,
                               int16_t *mask)
{
    int bnd, bin;
    int bndpsd, nextpsd, lastpsd;
    int lowcomp, fastleak, slowleak, leakflag = 0;
	int16_t excite;

	bin = start;
	bnd = band_start;
	leakflag = 0;
	lowcomp = 0;

    fastleak = (s->cpl_fast_leak << 8) + 768;
    slowleak = (s->cpl_slow_leak << 8) + 768;

	while (bin < end) {
		bin = FFMIN(babndtab[bnd], end);

		/* Compute excitation function */
		bndpsd = band_psd[bnd];
		if(bin==end)
			nextpsd = bndpsd;
		else
		    nextpsd = band_psd[bnd+1];

		if (bnd <= 2) {
			if (bndpsd <=(nextpsd - 256))
				lowcomp = 384;
			else if (bndpsd > nextpsd)
				lowcomp = FFMAX(0, lowcomp - 64);
			lastpsd = bndpsd;
		} else if (bnd <= 5) {
			if (bndpsd <= (nextpsd - 256))
				lowcomp = 384;
			else if (bndpsd > nextpsd)
				lowcomp = FFMAX(0, lowcomp - 64);
			if (lastpsd <= bndpsd)
				leakflag = 1;
			lastpsd = bndpsd;
		} else if (bnd == 6) {
			if (end != 7) {
				if (bndpsd <= (nextpsd - 256))
					lowcomp = 384;
				else if (bndpsd > nextpsd)
					lowcomp = FFMAX(0, lowcomp - 64);
			}
			if (lastpsd <= bndpsd)
				leakflag = 1;
		} else if (bnd <= 19) {
			if (bndpsd <= (nextpsd - 256))
				lowcomp = 320;
			else if (bndpsd > nextpsd)
				lowcomp = FFMAX(0, lowcomp - 64);
			leakflag = 1;
		} else if (bnd <= 22) {
			lowcomp = FFMAX(0, lowcomp - 128);
			leakflag = 1;
		}
		else
		{
			leakflag = 1;
		}

		if (leakflag) {
            fastleak = FFMAX(fastleak - s->fast_decay, band_psd[bnd] - fast_gain);
            slowleak = FFMAX(slowleak - s->slow_decay, band_psd[bnd] - s->slow_gain);
            excite = FFMAX(fastleak - lowcomp, slowleak);
	    } else {
			fastleak = band_psd[bnd] - fast_gain;
			slowleak = band_psd[bnd] - s->slow_gain; 
			excite = fastleak - lowcomp;
	    }

		if (bndpsd < s->db_per_bit)
			excite += (s->db_per_bit - bndpsd) >> 2;

		mask[bnd] = FFMAX(excite, ff_ac3_hearing_threshold_tab[bnd >> s->sr_shift][s->sr_code]);
		bnd++;

	}	/* while */
	//s->babnd_end = bnd;

    /* delta bit allocation */

    if (dba_mode == DBA_REUSE || dba_mode == DBA_NEW) {
        int i, seg, delta;
        //if (dba_nsegs >= 8)
        //    return -1;
        bnd = band_start;    // solved advanced.ac3 bug
        for (seg = 0; seg < dba_nsegs; seg++) {
            bnd += dba_offsets[seg];
            if (bnd >= AC3_CRITICAL_BANDS || dba_lengths[seg] > AC3_CRITICAL_BANDS-bnd)
                return -1;
            if (dba_values[seg] >= 4) {
                delta = (dba_values[seg] - 3) << 7;
            } else {
                delta = (dba_values[seg] - 4) << 7;
            }
            for (i = 0; i < dba_lengths[seg]; i++) {
                mask[bnd++] += delta;
            }
        }
    }

    return 0;
}

static void ac3_bit_alloc_calc_bap(int16_t *mask, int16_t *psd,
                                     int start, int end,
                                     int snr_offset, int floor,
                                     const uint8_t *bap_tab, uint8_t *bap)
{
    int bin, band;

    /* special case, if snr offset is -960, set all bap's to zero */
    if (snr_offset == -960) {
        memset(bap, 0, AC3_MAX_COEFS);
        return;
    }

    bin  = start;
    band = ff_ac3_bin_to_band_tab[start];
    do {
        int m = (FFMAX(mask[band] - snr_offset - floor, 0) & 0x1FE0) + floor;
        int band_end = FFMIN(ff_ac3_band_start_tab[band+1], end);
        for (; bin < band_end; bin++) {
            int address = (psd[bin] - m) >> 5;
			if (address>63) address = 63;
			else if (address<0) address = 0;
            bap[bin] = bap_tab[address];
        }
    } while (end > ff_ac3_band_start_tab[band++]);
}

int bitalloc_unp(AC3DecodeContext *s, int blk)
{
    int ch, i, seg;
    int fbw_channels = s->fbw_channels;
    int cpl_in_use = s->cpl_in_use[blk];
    GetBitContext *gbc = &s->gbc;

    /* bit allocation information */
    if (s->bit_allocation_syntax) {
        if (get_bits1(gbc)) {
            s->bit_alloc_params.slow_decay = ff_ac3_slow_decay_tab[get_bits(gbc, 2)] >> s->bit_alloc_params.sr_shift;
            s->bit_alloc_params.fast_decay = ff_ac3_fast_decay_tab[get_bits(gbc, 2)] >> s->bit_alloc_params.sr_shift;
            s->bit_alloc_params.slow_gain  = ff_ac3_slow_gain_tab[get_bits(gbc, 2)];
            s->bit_alloc_params.db_per_bit = ff_ac3_db_per_bit_tab[get_bits(gbc, 2)];
            s->bit_alloc_params.floor  = ff_ac3_floor_tab[get_bits(gbc, 3)];
            for(ch=!cpl_in_use; ch<=s->channels; ch++)
                s->bit_alloc_stages[ch] = FFMAX(s->bit_alloc_stages[ch], 2);
        } else if (!blk) {
            PRINTF("new bit allocation info must be present in block 0\n");
            return -1;
        }
    }

    /* signal-to-noise ratio offsets and fast gains (signal-to-mask ratios) */
    if(!s->eac3 || s->snr_offset_strategy) {
		if((s->eac3 && !blk) || get_bits1(gbc)) {
            int snr = 0;
            int csnr;
            csnr = (get_bits(gbc, 6) - 15) << 4;
            for (i = ch = !cpl_in_use; ch <= s->channels; ch++) {
                /* snr offset */
                if (ch == i || s->snr_offset_strategy == 2)
                    snr = (csnr + get_bits(gbc, 4)) << 2;
                /* run at least last bit allocation stage if snr offset changes */
                if(blk && s->snr_offset[ch] != snr) {
                    s->bit_alloc_stages[ch] = FFMAX(s->bit_alloc_stages[ch], 1);
                }
                s->snr_offset[ch] = snr;

                /* fast gain (normal AC-3 only) */
                if (!s->eac3) {
                    int prev = s->fast_gain[ch];
                    s->fast_gain[ch] = ff_ac3_fast_gain_tab[get_bits(gbc, 3)];
                    /* run last 2 bit allocation stages if fast gain changes */
                    if(blk && prev != s->fast_gain[ch])
                        s->bit_alloc_stages[ch] = FFMAX(s->bit_alloc_stages[ch], 2);
				}
			}
	    } 
	} else if (!s->eac3 && !blk) {
        PRINTF("new snr offsets must be present in block 0\n");
        return -1;
    }

    /* fast gain (E-AC-3 only) */
    if (s->fast_gain_syntax && get_bits1(gbc)) {
        for (ch = !cpl_in_use; ch <= s->channels; ch++) {
            int prev = s->fast_gain[ch];
            s->fast_gain[ch] = ff_ac3_fast_gain_tab[get_bits(gbc, 3)];
            /* run last 2 bit allocation stages if fast gain changes */
            if(blk && prev != s->fast_gain[ch])
                s->bit_alloc_stages[ch] = FFMAX(s->bit_alloc_stages[ch], 2);
        }
    } else if (s->eac3 && !blk) {
        for (ch = 0/*!cpl_in_use*/; ch <= s->channels; ch++)  // initial all fast_gain
            s->fast_gain[ch] = ff_ac3_fast_gain_tab[4];
    }

    /* E-AC-3 to AC-3 converter SNR offset */
    if (s->frame_type == EAC3_FRAME_TYPE_INDEPENDENT && get_bits1(gbc)) {
        skip_bits(gbc, 10); // skip converter snr offset
    }

    /* coupling leak information */
    if (cpl_in_use) {
        if (s->first_cpl_leak || get_bits1(gbc)) {
            int fl = get_bits(gbc, 3);
            int sl = get_bits(gbc, 3);
            /* run last 2 bit allocation stages for coupling channel if
               coupling leak changes */
            if(blk && (fl != s->bit_alloc_params.cpl_fast_leak ||
                       sl != s->bit_alloc_params.cpl_slow_leak)) {
                s->bit_alloc_stages[CPL_CH] = FFMAX(s->bit_alloc_stages[CPL_CH], 2);
            }
            s->bit_alloc_params.cpl_fast_leak = fl;
            s->bit_alloc_params.cpl_slow_leak = sl;
        } else if (!s->eac3 && !blk) {
            PRINTF("new coupling leak info must be present in block 0\n");
            return -1;
        }
        s->first_cpl_leak = 0;
    }

    /* delta bit allocation information */
    if (s->dba_syntax && get_bits1(gbc)) {
        /* delta bit allocation exists (strategy) */
        for (ch = !cpl_in_use; ch <= fbw_channels; ch++) {
            s->dba_mode[ch] = get_bits(gbc, 2);
            //if (s->dba_mode[ch] == DBA_RESERVED) {
            //    PRINTF("delta bit allocation strategy reserved\n");
            //    return -1;
            //}
            s->bit_alloc_stages[ch] = FFMAX(s->bit_alloc_stages[ch], 2);
        }
        /* channel delta offset, len and bit allocation */
        for (ch = !cpl_in_use; ch <= fbw_channels; ch++) {
            if (s->dba_mode[ch] == DBA_NEW) {
                s->dba_nsegs[ch] = get_bits(gbc, 3);
				s->dba_nsegs[ch] ++;  // solved advanced.ac3 bug
                for (seg = 0; seg < s->dba_nsegs[ch]; seg++) {
                    s->dba_offsets[ch][seg] = get_bits(gbc, 5);
                    s->dba_lengths[ch][seg] = get_bits(gbc, 4);
                    s->dba_values[ch][seg] = get_bits(gbc, 3);
                }
                /* run last 2 bit allocation stages if new dba values */
                s->bit_alloc_stages[ch] = FFMAX(s->bit_alloc_stages[ch], 2);
            }
        }
    } else if(blk == 0) {
        for(ch=0; ch<=s->channels; ch++) {
            s->dba_mode[ch] = DBA_NONE;
        }
    }

    /* Bit allocation */
    for(ch=!cpl_in_use; ch<=s->channels; ch++) {
        if(s->bit_alloc_stages[ch] > 2) {
            /* Exponent mapping into PSD and PSD integration */
            ff_ac3_bit_alloc_calc_psd(s->dexps[ch],
                                      s->start_freq[ch], s->end_freq[ch],
                                      s->psd[ch], s->band_psd[ch]);
        }
        if(s->bit_alloc_stages[ch] > 1) {
            /* Compute excitation function, Compute masking curve, and
               Apply delta bit allocation */
            if (ff_ac3_bit_alloc_calc_mask(&s->bit_alloc_params, s->band_psd[ch],
                                           s->start_freq[ch], s->end_freq[ch], s->babnd_start[ch],
                                           s->fast_gain[ch], (ch == s->lfe_ch),
                                           s->dba_mode[ch], s->dba_nsegs[ch],
                                           s->dba_offsets[ch], s->dba_lengths[ch],
                                           s->dba_values[ch], s->mask[ch])) {
                PRINTF("error in bit allocation\n");
                return -1;
            }
        }
        if(s->bit_alloc_stages[ch] > 0) {
            /* Compute bit allocation */
            const uint8_t *bap_tab = s->channel_uses_aht[ch] ?
                                     ff_eac3_hebap_tab : ff_ac3_bap_tab;
            ac3_bit_alloc_calc_bap(s->mask[ch], s->psd[ch],
                                      s->start_freq[ch], s->end_freq[ch],
                                      s->snr_offset[ch],
                                      s->bit_alloc_params.floor,
                                      bap_tab, s->bap[ch]);
        }
    }
    return 0;
}
