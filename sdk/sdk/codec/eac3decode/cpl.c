/*!
*  \file spxd.c
*
* \brief SPX module decode-side utility functions.
*
*  Part of the Spectral Extension Module.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "ac3dec.h"

/*! <i>cplbegf</i> to bit allocation band translation table */
const uint8_t cpl_bndtab[16] =
{	31, 35, 37, 39, 41, 42, 43, 44, 45, 45, 46, 46, 47, 47, 48, 48 };

/*! Enhanced coupling begin frequency to bit allocation band lookup table */
const uint8_t ecpd_begfbabndtab[16] =
{
	/* begf     babnd */
	/*	  0 */   13,
	/*	  1 */   25,
	/*	  2 */   31,
	/*	  3 */   35,
	/*	  4 */   37,
	/*	  5 */   39,
	/*	  6 */   41,
	/*	  7 */   42,
	/*	  8 */   43,
	/*	  9 */   44,
	/*	 10 */   45,
	/*	 11 */   45,
	/*   12 */   46,
	/*   13 */   47,
	/*   14 */   48,
	/*   15 */   49
};

/*! Default enhanced coupling band structure */
static const uint8_t ecpd_defecplbndstrc[22] =
{
	/* sbnd  link */
	/*  0 */	0,
	/*  1 */	0,
	/*  2 */	0,
	/*  3 */	0,
	/*  4 */	0,
	/*  5 */	0,
	/*  6 */	0,
	/*  7 */	0,
	/*  8 */	0,
	/*  9 */	1,
	/* 10 */	0,
	/* 11 */	1,
	/* 12 */	0,
	/* 13 */	1,
	/* 14 */	0,
	/* 15 */	1,
	/* 16 */	1,
	/* 17 */	1,
	/* 18 */	0,
	/* 19 */	1,
	/* 20 */	1,
	/* 21 */	1
};

/*! Enhanced coupling begin frequency to start subband lookup table */
static const uint8_t ecpd_startsbndtab[AC3_ECPD_BEGFTABSIZE] =
{
	/* begf   sbnd */
	/*  0 */	 0,
	/*  1 */	 2,
	/*  2 */	 4,
	/*  3 */	 5,
	/*  4 */	 6,
	/*  5 */	 7,
	/*  6 */	 8,
	/*  7 */	 9,
	/*  8 */	10,
	/*  9 */	11,
	/* 10 */	12,
	/* 11 */	13,
	/* 12 */	14,
	/* 13 */	16,
	/* 14 */	18,
	/* 15 */	20
};

/*! Enhanced coupling sub-band start bin lookup table */
static const uint8_t ecpd_subbndtab[AC3_ECPD_MAXNUMECPBNDS + 1] =
{
	/*  0 */		13,
	/*  1 */		19,
	/*  2 */		25,
	/*  3 */		31,
	/*  4 */		37,
	/*  5 */		49,
	/*  6 */		61,
	/*  7 */		73,
	/*  8 */		85,
	/*  9 */		97,
	/* 10 */		109,
	/* 11 */		121,
	/* 12 */		133,
	/* 13 */		145,
	/* 14 */		157,
	/* 15 */		169,
	/* 16 */		181,
	/* 17 */		193,
	/* 18 */		205,
	/* 19 */		217,
	/* 20 */		229,
	/* 21 */		241,
	/* 22 */		253
};

/*! Enhanced coupling amplitude mantissa lookup table */
static quantizer_t ecplampmanttab[32] =
{
	/* Index	Mantissa */
	/*  0 */	QUANT(0.50000000),		/* Use 0.5 x 2^1 in order to get +1.0 in a signed fixed point container */
	/*  1 */	QUANT(0.84375000),
	/*  2 */	QUANT(0.71875000),
	/*  3 */	QUANT(0.59375000),
	/*  4 */	QUANT(0.50000000),
	/*  5 */	QUANT(0.84375000),
	/*  6 */	QUANT(0.71875000),
	/*  7 */	QUANT(0.59375000),
	/*  8 */	QUANT(0.50000000),
	/*  9 */	QUANT(0.84375000),
	/* 10 */	QUANT(0.71875000),
	/* 11 */	QUANT(0.59375000),
	/* 12 */	QUANT(0.50000000),
	/* 13 */	QUANT(0.84375000),
	/* 14 */	QUANT(0.71875000),
	/* 15 */	QUANT(0.59375000),
	/* 16 */	QUANT(0.50000000),
	/* 17 */	QUANT(0.84375000),
	/* 18 */	QUANT(0.71875000),
	/* 19 */	QUANT(0.59375000),
	/* 20 */	QUANT(0.50000000),
	/* 21 */	QUANT(0.84375000),
	/* 22 */	QUANT(0.71875000),
	/* 23 */	QUANT(0.59375000),
	/* 24 */	QUANT(0.50000000),
	/* 25 */	QUANT(0.84375000),
	/* 26 */	QUANT(0.71875000),
	/* 27 */	QUANT(0.59375000),
	/* 28 */	QUANT(0.50000000),
	/* 29 */	QUANT(0.84375000),
	/* 30 */	QUANT(0.71875000),
	/* 31 */	QUANT(0.00000000)
};

/*! Enhanced coupling amplitude exponent lookup table */
static const int8_t ecplampexptab[32] =
{
	/* Index	Exponent */
	/*  0 */ 	-1,			/* Use 0.5 x 2^1 in order to get +1.0 in a signed fixed point container */
	/*  1 */	0,
	/*  2 */	0,
	/*  3 */	0,
	/*  4 */	0,
	/*  5 */	1,
	/*  6 */	1,
	/*  7 */	1,
	/*  8 */	1,
	/*  9 */	2,
	/* 10 */	2,
	/* 11 */	2,
	/* 12 */	2,
	/* 13 */	3,
	/* 14 */	3,
	/* 15 */	3,
	/* 16 */	3,
	/* 17 */	4,
	/* 18 */	4,
	/* 19 */	4,
	/* 20 */	4,
	/* 21 */	5,
	/* 22 */	5,
	/* 23 */	5,
	/* 24 */	5,
	/* 25 */	6,
	/* 26 */	6,
	/* 27 */	6,
	/* 28 */	6,
	/* 29 */	7,
	/* 30 */	7,
	/* 31 */	24
};


/*****************************************************************
* cpl_unp:
*****************************************************************/
int cpl_unp(AC3DecodeContext *s, int blk)
{
    int bnd, ch;
    int fbw_channels = s->fbw_channels;
    int channel_mode = s->channel_mode;
    GetBitContext *gbc = &s->gbc;

    /* coupling strategy */
    if (s->eac3 ? s->cpl_strategy_exists[blk] : get_bits1(gbc)) {
        memset(s->bit_alloc_stages, 3, AC3_MAX_CHANNELS);
        if (!s->eac3)
            s->cpl_in_use[blk] = get_bits1(gbc);
        if (s->cpl_in_use[blk]) {
            int cpl_start_subband, cpl_end_subband, start_freq, subbnd;

            /* coupling in use */

            if (channel_mode < AC3_CHMODE_STEREO) {
                PRINTF("coupling not allowed in mono or dual-mono\n");
                return -1;
            }

            /* check for enhanced coupling */
            if (s->eac3)
                s->ecplinu = get_bits1(gbc);

            if (s->eac3 && channel_mode == AC3_CHMODE_STEREO) {
                /* if 2/0 mode, both channels are coupled */
                s->channel_in_cpl[1] = 1;
                s->channel_in_cpl[2] = 1;
            } else {
                for (ch = 1; ch <= fbw_channels; ch++)
                    s->channel_in_cpl[ch] =  get_bits1(gbc);
            }

            if (s->eac3 && s->ecplinu) { // enhanced coupling
                int necplbnd;

                start_freq = get_bits(gbc,4);
                cpl_start_subband = ecpd_startsbndtab[start_freq];
                s->cpl_start_band = cpl_start_subband;
                if (s->spx_in_use == 0)
                    cpl_end_subband = get_bits(gbc,4) + 7;
                else {/* spx in use - calculate ecpl endsubbnd */
                    if (s->spx_begf < 6)
                        cpl_end_subband = s->spx_begf + 5;
                    else
                        cpl_end_subband = s->spx_begf * 2;
                }


                /* unpack ecpl banding structure info */
                if (get_bits1(gbc)) {
                    for (bnd = FFMAX(AC3_ECPD_MINNUMINDPBNDS, cpl_start_subband + 1); bnd < cpl_end_subband; bnd++)
                        s->ecplbndstrc[bnd] = get_bits1(gbc);
                } else if(!blk) {
                    for (bnd = 0; bnd < AC3_ECPD_MAXNUMECPBNDS; bnd++)
                        s->ecplbndstrc[bnd] = ecpd_defecplbndstrc[bnd];
                }

                /* first subband bndstrc bit never sent */
                s->ecplbndstrc[cpl_start_subband] = 0;

                /* determine bin range and put it in standard coupling bin range */
                s->start_freq[CPL_CH] = ecpd_subbndtab[cpl_start_subband];
                s->end_freq[CPL_CH] =   ecpd_subbndtab[cpl_end_subband];
                s->babnd_start[CPL_CH] = ecpd_begfbabndtab[start_freq];

                //p_cplchan->cplpkexms.chbai.babndrng.begin = ecpd_begfbabndtab[p_ecpblk->bse_ecplbegf];

                /* calculate enhanced coupling helper variables */
                necplbnd = 0;

                /* Calculate necplbnd and bnd bin offset helper variables */
                for (bnd = cpl_start_subband; bnd < cpl_end_subband; bnd++) {
                    /* Look for break in banding structure - next band */
                    if (s->ecplbndstrc[bnd] == 0) {
                        /* Assign bin offset for this sbnd from table */
                        s->ecpbndbinoffst[necplbnd] = ecpd_subbndtab[bnd];

                        /* Count only sbnds with zero in bndstrc - means start a new band */
                        necplbnd++;
                    }
                }

                /* Calculate last bnd bin offset */
                s->ecpbndbinoffst[necplbnd] = ecpd_subbndtab[bnd];

                /* Assign value from temp variable */
                s->necplbnd = necplbnd;

            } else {  // not enhanced coupling
                int n_subbands;
                /* phase flags in use */
                if (channel_mode == AC3_CHMODE_STEREO)
                    s->phase_flags_in_use = get_bits1(gbc);
                else
                    s->phase_flags_in_use = 0;
                /* coupling frequency range */
                cpl_start_subband = get_bits(gbc, 4);
                if (s->spx_in_use)
                    cpl_end_subband = s->spx_beg_subbnd - 1;
                else
                    cpl_end_subband = get_bits(gbc, 4) + 3;
                if (cpl_start_subband >= cpl_end_subband) {
                    PRINTF("invalid coupling range (%d >= %d)\n", cpl_start_subband, cpl_end_subband);                    
                    return -1;
                }
                s->cpl_start_band = cpl_start_subband;
                s->start_freq[CPL_CH] = cpl_start_subband * 12 + 37;
                s->end_freq[CPL_CH]   = cpl_end_subband   * 12 + 37;
                s->babnd_start[CPL_CH] = cpl_bndtab[cpl_start_subband];

                /* decode band structure from bitstream or use default */
                n_subbands = cpl_end_subband - cpl_start_subband;
                s->cpl_band_struct[cpl_start_subband] = 0;
                if (!s->eac3 || get_bits1(gbc)) {
                    for (subbnd = 1; subbnd < n_subbands; subbnd++)
                        s->cpl_band_struct[subbnd+cpl_start_subband] = get_bits1(gbc);
                } else if (!blk) {
                    for (subbnd = 1; subbnd < n_subbands; subbnd++)
                        s->cpl_band_struct[subbnd+cpl_start_subband] = ff_eac3_default_cpl_band_struct[subbnd+cpl_start_subband];
                }

                s->num_cpl_bands = n_subbands;
                s->cpl_band_sizes[0] = 12;//s->eac3 ? 6 : 12;
                for (subbnd = 1; subbnd < n_subbands; subbnd++)
                    s->cpl_band_sizes[subbnd] = 12;//(s->eac3 && subbnd < 4) ? 6 : 12;
            }
        } else {
            /* coupling not in use */
            for (ch = 1; ch <= fbw_channels; ch++) {
                s->channel_in_cpl[ch] = 0;
                s->first_cpl_coords[ch] = 1;
            }
            s->first_cpl_leak = s->eac3;
            s->phase_flags_in_use = 0;
        }
    } else if (!s->eac3) {
        if(!blk) {
            PRINTF("new coupling strategy must be present in block 0\n");
            return -1;
        } else {
            s->cpl_in_use[blk] = s->cpl_in_use[blk-1];
        }
    }

    /* coupling coordinates */
    if (s->cpl_in_use[blk]) {
        if (s->ecplinu == 0) {
            int cpl_coords_exist = 0;

            for (ch = 1; ch <= fbw_channels; ch++) {
                if (s->channel_in_cpl[ch]) {
                    if ((s->eac3 && s->first_cpl_coords[ch]) || get_bits1(gbc)) {
                        int cpl_exp_bias, cpl_exp_max, cpl_coord_exp, cpl_coord_mant;
                        s->first_cpl_coords[ch] = 0;
                        cpl_coords_exist = 1;
                        cpl_exp_bias = 3 * get_bits(gbc, 2);
                        cpl_exp_max = cpl_exp_bias + 15;
                        for (bnd = 0; bnd < s->num_cpl_bands; bnd++) {
                            if(s->cpl_band_struct[bnd+s->cpl_start_band]==0) {
                                cpl_coord_exp = get_bits(gbc, 4);
                                cpl_coord_mant = get_bits(gbc, 4);
                                cpl_coord_exp += cpl_exp_bias;
                                s->cplco_exp[ch][bnd] = cpl_coord_exp;
                                if (cpl_coord_exp == cpl_exp_max)
                                    s->cplco_mant[ch][bnd] = cpl_coord_mant<<(QUANT_SHIFT-4);
                                else
                                    s->cplco_mant[ch][bnd] = (cpl_coord_mant | 0x10)<<(QUANT_SHIFT-5);
                            } else {
                                s->cplco_exp[ch][bnd]  = s->cplco_exp[ch][bnd - 1];
                                s->cplco_mant[ch][bnd] = s->cplco_mant[ch][bnd - 1];
                            }
                        }
                    } else if (!blk) {
                        PRINTF("new coupling coordinates must be present in block 0\n");
                        return -1;
                    }
                } else {
                    /* channel not in coupling */
                    s->first_cpl_coords[ch] = 1;
                }
            }
            /* phase flags */
            if (s->phase_flags_in_use && channel_mode == AC3_CHMODE_STEREO && cpl_coords_exist) {
                for (bnd = 0; bnd < s->num_cpl_bands; bnd++) {
                    if (s->cpl_band_struct[bnd+s->cpl_start_band] == 0)
                        s->phase_flags = get_bits1(gbc);
                    if (s->phase_flags)
                        s->cplco_mant[2][bnd] = -FFABS(s->cplco_mant[2][bnd]);
                    else
                        s->cplco_mant[2][bnd] = FFABS(s->cplco_mant[2][bnd]);
                }
            }
        } else {  // enhanced coupling
            int firstchincpl = -1;
            int ecplparam1e, ecplparam2e;
            int ecplangleintrp = get_bits1(gbc);

            for (ch = 1; ch <= fbw_channels; ch++) {
                if (s->channel_in_cpl[ch]) {
                    /* setup temp pointer for convenience */
                    if (firstchincpl == -1)
                        firstchincpl = ch;

                    if (s->first_cpl_coords[ch]) {
                        ecplparam1e = 1;

                        if (ch > firstchincpl)
                            ecplparam2e = 1;
                        else
                            ecplparam2e = 0;

                        s->first_cpl_coords[ch] = 0;
                    } else { /* !firstcplcos[ch] */
                        ecplparam1e = get_bits1(gbc);

                        if (ch > firstchincpl)
                            ecplparam2e = get_bits1(gbc);
                        else
                            ecplparam2e = 0;
                    }

                    /* unpack ecp amplitudes if present */
                    if (ecplparam1e)
                        for (bnd = 0; bnd < s->necplbnd; bnd++)
                            s->ecplamp[ch][bnd] = get_bits(gbc, 5);

                    /* else amplitudes from last blk should still exist from blkcopy */

                    if (ecplparam2e)
                        for (bnd = 0; bnd < s->necplbnd; bnd++) {
                            int ecplangle = get_bits(gbc, 6);
                            int ecplchaos = get_bits(gbc, 3);
                        }
                    /* else angles and chaos from last blk should still exist from blkcopy */

                        if (ch > firstchincpl) {
                            int ecpltrans = get_bits1(gbc);
                        }

                } else { /* !chincpl[ch] */
                    /* Covers case where channels are pulled in and out independently */
                    s->first_cpl_coords[ch] = 1;
                } /* chincpl[ch] */
            } /* for ch */
        }
    }
    return 0;

} /* cpl_unp */


/**
 * Generate transform coefficients for each coupled channel in the coupling
 * range using the coupling coefficients and coupling coordinates.
 * reference: Section 7.4.3 Coupling Coordinate Format
 */
void cpld_decouple(AC3DecodeContext *s)
{
    int bin, band, ch;

    bin = s->start_freq[CPL_CH];
    for (band = 0; band < s->num_cpl_bands; band++) {
        int band_start = bin;
        int band_end = bin + s->cpl_band_sizes[band];
        for (ch = 1; ch <= s->fbw_channels; ch++) {
            if (s->channel_in_cpl[ch]) {
                for (bin = band_start; bin < band_end; bin++) {
        /* Encoder divides by 8 when calculating coupling coordinates           */
        /* decoder must multiply by 8 to undo this encoder processing           */
        /* (subtract by 3 on negative exponent is equivalent to multiply by 8)  */
                    s->dexps[ch][bin] = s->dexps[CPL_CH][bin] + s->cplco_exp[ch][band] -3;
                    s->fixed_coeffs[ch][bin] = MUL_Shift_23(s->fixed_coeffs[CPL_CH][bin], s->cplco_mant[ch][band], QUANT_SHIFT);
                }
            }
        }
        bin = band_end;
    }
}

/**
 * Generate transform coefficients for each coupled channel in the enhanced coupling
 * range using the coupling coefficients and coupling coordinates.
 * reference: Section 7.4.3 Coupling Coordinate Format
 */
void ecpd_decpamponly(AC3DecodeContext *s)
{
    int bin, bnd, ch;
    int ampmnt, ampexp;

    for (bnd = 0; bnd < s->necplbnd; bnd++)
    {
        for (ch = 1; ch <= s->fbw_channels; ch++) {
            if (s->channel_in_cpl[ch]) {
                // Lookup amplitude mantissa and exponent values for bit-exact path 
                int idx = s->ecplamp[ch][bnd];

                ampmnt = ecplampmanttab[idx];
                ampexp = ecplampexptab [idx];
                // Bin loop
                for (bin = s->ecpbndbinoffst[bnd]; bin < s->ecpbndbinoffst[bnd + 1]; bin++) {
                    s->dexps[ch][bin] = s->dexps[CPL_CH][bin] + ampexp;
                    s->fixed_coeffs[ch][bin] = MUL_Shift_23(s->fixed_coeffs[CPL_CH][bin], ampmnt, (QUANT_SHIFT/*+ampexp*/));
                }
            }
        }
    }
}
