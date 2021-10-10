/*!
*  \file spx.c
*
* \brief SPX module decode-side utility functions.
*
*  Part of the Spectral Extension Module.
*/
#include "debug.h"
#include "ac3dec.h"

#define SPXONEOVERSQRT3	FRACT(0.577350269190)	/*!< Used in signal scale factor calucation 1/sqrt(3) */
#define SPXSCLSBNDRMS	FRACT(0.288675134595)	/*!< Used in signal RMS calculation 1/sqrt(12) */
#define	NSAMPSCLMANT	FRACT(0.866025403784)

/* Band Notch Filter Tables */
/*! Band Notch Filter Tables */
static fract_t bndnotchtab[32][3] =
{
	{ FRACT(0.95484160391042), FRACT(0.91172248855822), FRACT(0.87055056329612) },
	{ FRACT(0.91172248855822), FRACT(0.83123789614279), FRACT(0.75785828325520) },
	{ FRACT(0.87055056329612), FRACT(0.75785828325520), FRACT(0.65975395538645) },
	{ FRACT(0.83123789614279), FRACT(0.69095643998389), FRACT(0.57434917749852) },
	{ FRACT(0.79370052598410), FRACT(0.62996052494744), FRACT(0.50000000000000) },
	{ FRACT(0.75785828325520), FRACT(0.57434917749852), FRACT(0.43527528164806) },
	{ FRACT(0.72363461872019), FRACT(0.52364706141031), FRACT(0.37892914162760) },
	{ FRACT(0.69095643998389), FRACT(0.47742080195521), FRACT(0.32987697769322) },
	{ FRACT(0.65975395538645), FRACT(0.43527528164806), FRACT(0.28717458874926) },
	{ FRACT(0.62996052494744), FRACT(0.39685026299205), FRACT(0.25000000000000) },
	{ FRACT(0.60151251804106), FRACT(0.36181730936009), FRACT(0.21763764082403) },
	{ FRACT(0.57434917749852), FRACT(0.32987697769322), FRACT(0.18946457081380) },
	{ FRACT(0.54841248984731), FRACT(0.30075625902053), FRACT(0.16493848884661) },
	{ FRACT(0.52364706141031), FRACT(0.27420624492365), FRACT(0.14358729437463) },
	{ FRACT(0.50000000000000), FRACT(0.25000000000000), FRACT(0.12500000000000) },
	{ FRACT(0.47742080195521), FRACT(0.22793062213956), FRACT(0.10881882041202) },
	{ FRACT(0.45586124427911), FRACT(0.20780947403570), FRACT(0.09473228540690) },
	{ FRACT(0.43527528164806), FRACT(0.18946457081380), FRACT(0.08246924442331) },
	{ FRACT(0.41561894807139), FRACT(0.17273910999597), FRACT(0.07179364718731) },
	{ FRACT(0.39685026299205), FRACT(0.15749013123686), FRACT(0.06250000000000) },
	{ FRACT(0.37892914162760), FRACT(0.14358729437463), FRACT(0.05440941020601) },
	{ FRACT(0.36181730936009), FRACT(0.13091176535258), FRACT(0.04736614270345) },
	{ FRACT(0.34547821999195), FRACT(0.11935520048880), FRACT(0.04123462221165) },
	{ FRACT(0.32987697769322), FRACT(0.10881882041202), FRACT(0.03589682359366) },
	{ FRACT(0.31498026247372), FRACT(0.09921256574801), FRACT(0.03125000000000) },
	{ FRACT(0.30075625902053), FRACT(0.09045432734002), FRACT(0.02720470510300) },
	{ FRACT(0.28717458874926), FRACT(0.08246924442331), FRACT(0.02368307135173) },
	{ FRACT(0.27420624492365), FRACT(0.07518906475513), FRACT(0.02061731110583) },
	{ FRACT(0.26182353070516), FRACT(0.06855156123091), FRACT(0.01794841179683) },
	{ FRACT(0.25000000000000), FRACT(0.06250000000000), FRACT(0.01562500000000) },
	{ FRACT(0.23871040097760), FRACT(0.05698265553489), FRACT(0.01360235255150) },
	{ FRACT(0.22793062213956), FRACT(0.05195236850892), FRACT(0.01184153567586) }
};

static const fract_t oneoverspxendtab[8] =
{
	0x0182,	/* endbin = 85	*/
	0x0152,	/* endbin = 97	*/
	0x012D,	/* endbin = 109	*/
	0x00F6,	/* endbin = 133	*/
	0x00D1,	/* endbin = 157	*/
	0x00B5,	/* endbin = 181	*/
	0x00A0,	/* endbin = 205	*/
	0x008F	/* endbin = 229	*/
};

static int oneoverspxbndsztab[16] =
{   //s5.26
	FRACT(0.0), FRACT(1./1.), FRACT(1./2.), FRACT(1./3.), FRACT(1./4.), FRACT(1./5.), FRACT(1./6.),
	FRACT(1./7.), FRACT(1./8.), FRACT(1./9.), FRACT(1./10.), FRACT(1./11.), FRACT(1./12.), FRACT(1./13.),
	FRACT(1./14.), FRACT(1./15.)
};

/*****************************************************************
* spxd_unp:
*****************************************************************/
int spx_unp(AC3DecodeContext *s, int blk)
{
    int bin, bnd, ch;
    int fbw_channels = s->fbw_channels;
    GetBitContext *gbc = &s->gbc;

    /* spectral extension strategy */
    if (s->eac3 && (!blk || get_bits1(gbc))) {
        s->spx_in_use = get_bits1(gbc);
        if (s->spx_in_use) {
            int beg_subbnd, end_subbnd;
            int pxbndstrce;

            /* determine which channels use spx */
            if (s->channel_mode == AC3_CHMODE_MONO) {
                s->channel_uses_spx[1] = 1;
            } else {
                for (ch = 1; ch <= fbw_channels; ch++)
                    s->channel_uses_spx[ch] = get_bits1(gbc);
            }

            /* get the frequency bins of the spx copy region and the spx start
               and end subbands */
            s->spx_start_subbnd = get_bits(gbc, 2);
            s->spx_begf = get_bits(gbc, 3);
            s->spx_endf = get_bits(gbc, 3);
            beg_subbnd  = s->spx_begf + 2;
            if (beg_subbnd > 7)
                beg_subbnd += beg_subbnd - 7;
            end_subbnd    = s->spx_endf + 5;
            if (end_subbnd   > 7)
                end_subbnd   += end_subbnd   - 7;

            s->spx_beg_subbnd = beg_subbnd;
            s->spx_end_subbnd = end_subbnd;

            /* check validity of spx ranges */
            if (beg_subbnd >= end_subbnd) {
                PRINTF("invalid spectral extension "
                       "range (%d >= %d)\n", beg_subbnd, end_subbnd);
                return -1;
            }
            if (s->spx_start_subbnd >= end_subbnd) {
                PRINTF("invalid spectral extension "
                       "copy start bin (%d >= %d)\n", s->spx_start_subbnd, end_subbnd);
                return -1;
            }

            pxbndstrce = get_bits1(gbc);
            if (pxbndstrce) {
                for (bnd = beg_subbnd + 1; bnd < end_subbnd; bnd++)
                    s->spx_band_struct[bnd] = get_bits1(gbc);
            }

            /* determine number of SPX bands and SPX band size array */
            s->num_spx_bands = 1;
            s->spx_band_sizes[0] = 1;
            for (bnd = beg_subbnd + 1; bnd < end_subbnd; bnd++)
            {
                if (s->spx_band_struct[bnd] == 0) {
                    /* start new band */
                    s->spx_band_sizes[s->num_spx_bands] = 1;
                    s->num_spx_bands++;
                } else {
                    /* group subband with current band */
                    s->spx_band_sizes[s->num_spx_bands - 1] += 1;
                }
            } /* bnd */
        } else {
            for (ch = 1; ch <= fbw_channels; ch++) {
                s->channel_uses_spx[ch] = 0;
                s->first_spx_coords[ch] = 1;
            }
        }
    }

    /* spectral extension coordinates */
    if (s->spx_in_use) {
        for (ch = 1; ch <= fbw_channels; ch++) {
            if (s->channel_uses_spx[ch]) {
                if (s->first_spx_coords[ch] || get_bits1(gbc)) {
                    int master_spx_coord;
                    fract_t spx_blend;

                    s->first_spx_coords[ch] = 0;
                    spx_blend = get_bits(gbc, 5) << (FRACT_SHIFT-5);
                    master_spx_coord = get_bits(gbc, 2) * 3;

                    bin = s->spx_beg_subbnd*12 + 25;
                    for (bnd = 0; bnd < s->num_spx_bands; bnd++) {
                        int spx_coord_exp, spx_coord_mant;
                        int bndsz;
                        fract_t nratio, spx_coord;
                        int32_t oneoverspxendbin = oneoverspxendtab[s->spx_endf];

                        spx_coord_exp  = get_bits(gbc, 4);
                        spx_coord_mant = get_bits(gbc, 2);
                        if (spx_coord_exp == 15) spx_coord_mant <<= 1;
                        else                     spx_coord_mant += 4;
                        spx_coord_exp += master_spx_coord;
                        spx_coord = spx_coord_mant << (FRACT_SHIFT-3 - spx_coord_exp);
                        bndsz = s->spx_band_sizes[bnd]*12;
                        nratio = ((bin + (bndsz>>1)) * oneoverspxendbin)<<15;   // to FRACT
                        nratio -= spx_blend;
                        bin += bndsz;

                        if(nratio>0) {
                            fract_t nblend, sblend, sratio;
                            int qnum;

                            /* calculate signal ratio (sratio = 1.0 - nratio) */
                            sratio = FRACT(1) - nratio;
                            nblend = sqrtfix(nratio, FRACT_SHIFT, &qnum); // noise is scaled by sqrt(3) to give unity variance                      
                            nblend <<= (FRACT_SHIFT - qnum); 
                            sblend = sqrtfix(sratio, FRACT_SHIFT, &qnum);
                            sblend <<= (FRACT_SHIFT - qnum);
                            s->spx_coord_nblend[ch][bnd] = MUL_Shift_30(nblend, spx_coord, FRACT_SHIFT);
                            sblend = MUL_Shift_30(sblend, spx_coord, FRACT_SHIFT);
                            s->spx_coord_sblend[ch][bnd] = MUL_Shift_30(sblend, SPXONEOVERSQRT3, FRACT_SHIFT);
                        } else {
                            s->spx_coord_nblend[ch][bnd] = FRACT(0);
                            s->spx_coord_sblend[ch][bnd] = MUL_Shift_30(spx_coord, SPXONEOVERSQRT3, FRACT_SHIFT);
                        }
                    }
                }
            } else {
                s->first_spx_coords[ch] = 1;
            }
        }
    }
    return 0;
}

/*****************************************************************
* spx_synthesizetcs:
*****************************************************************/
void spx_synthesizetcs(AC3DecodeContext *s, int ch)
{
    int bnd, sbnd, bin, qnum;
    int bndstrtbin, bndendbin;
    int spxstrtbin, spxbegbin;      
    int copyindex, writeindex, copysbnd;            
    fract_t  nsclmant, bndrmsmant;  
    fract_t sumsqmant[SPX_MAX_BANDS];
    fract_t accum;
    int wrap;
    int ncopysbnds = s->spx_beg_subbnd - s->spx_start_subbnd;
    int ndestsbnds = s->spx_end_subbnd - s->spx_beg_subbnd;
    int ncopysbndsumsqs;
    const fract_t *atten_tab = bndnotchtab[s->spx_atten_code[ch]];

    
    if(ncopysbnds>ndestsbnds)
        ncopysbndsumsqs = ndestsbnds;
    else
        ncopysbndsumsqs = ncopysbnds;

    if (ncopysbndsumsqs<0)
        ncopysbndsumsqs = 0;

    /* initialize local variables */
    spxstrtbin  = s->spx_start_subbnd*12 + 25;
    spxbegbin   = s->spx_beg_subbnd*12 + 25;
    writeindex  = spxbegbin;

    bndstrtbin = s->spx_start_subbnd*12 + 25;
    for (bnd = 0; bnd < ncopysbndsumsqs; bnd++)
    {
        /* initialize subband variables */
        bndendbin = bndstrtbin + 12;

        /* calculate subband sum square */
        accum = 0;
        for (bin = bndstrtbin; bin < bndendbin; bin++)
        {
            fract_t mant;
            /* square transform coefficient and accumulate */
            mant = MUL_Shift_30(s->fixed_coeffs[ch][bin], SPXSCLSBNDRMS, FRACT_SHIFT);
            accum += MUL_Shift_30(mant, mant, FRACT_SHIFT);
            if(accum<0)
                accum = 0;
        }
        sumsqmant[bnd] = accum;
        bndstrtbin = bndendbin;
    } /* sbnd */

    /* process bands */
    wrap        = 1;
    for (bnd = 0; bnd < s->num_spx_bands; bnd++) {
        if (wrap) {
            copyindex   = spxstrtbin;
            copysbnd    = 0;
        }

        /* process first subband */
        bndrmsmant = MUL_Shift_30(sumsqmant[copysbnd], oneoverspxbndsztab[s->spx_band_sizes[bnd]], FRACT_SHIFT);
        copysbnd++;

        /* perform translation */
        for (bin = 0; bin < 12; bin++)
            s->fixed_coeffs[ch][writeindex+bin] = s->fixed_coeffs[ch][copyindex+bin];
        writeindex += 12;
        copyindex += 12;

        /* Apply a notch filter at transitions between normal and extension bands and at all wrap points. */       
        if (wrap && s->spx_atten_code[ch] >= 0) {
            bin = writeindex - 12; 
            s->fixed_coeffs[ch][bin] = MUL_Shift_30(s->fixed_coeffs[ch][bin], atten_tab[2], FRACT_SHIFT);
            s->fixed_coeffs[ch][bin+1] = MUL_Shift_30(s->fixed_coeffs[ch][bin+1], atten_tab[1], FRACT_SHIFT);
            s->fixed_coeffs[ch][bin+2] = MUL_Shift_30(s->fixed_coeffs[ch][bin+2], atten_tab[0], FRACT_SHIFT);
        }

        /* process remaining subbands */
        for (sbnd = 1; sbnd < s->spx_band_sizes[bnd]; sbnd++) {
            /* check for wrap condition (this can only happen if bndsz > ncopybands) */
            if (copysbnd == ncopysbnds) {
                copyindex   = spxstrtbin;
                copysbnd    = 0;
            }

            bndrmsmant += MUL_Shift_30(sumsqmant[copysbnd], oneoverspxbndsztab[s->spx_band_sizes[bnd]], FRACT_SHIFT);

            /* perform translation */
            for (bin = 0; bin < 12; bin++)
                s->fixed_coeffs[ch][writeindex+bin] = s->fixed_coeffs[ch][copyindex+bin];
            
            writeindex += 12;
            copyindex += 12;
            copysbnd++;
        } /* sbnd */

        /* finish band RMS calculation by taking the square-root */
        bndrmsmant = sqrtfix(bndrmsmant, FRACT_SHIFT, &qnum);
        bndrmsmant <<= (FRACT_SHIFT - qnum);  

        /* scale noise coefficient by signal RMS to determine noise scale factor */
        nsclmant = MUL_Shift_30(s->spx_coord_nblend[ch][bnd], bndrmsmant, FRACT_SHIFT);

        /* determine if the next band is a wrapped band */
        if (bnd < s->num_spx_bands - 1) {
            if (copysbnd + s->spx_band_sizes[bnd+1] > ncopysbnds) {
                wrap = 1;
                /* apply notch to end of band */
                if (s->spx_atten_code[ch] >= 0) {
                    bin = writeindex - 2; 
                    s->fixed_coeffs[ch][bin] = MUL_Shift_30(s->fixed_coeffs[ch][bin], atten_tab[0], FRACT_SHIFT);
                    s->fixed_coeffs[ch][bin+1] = MUL_Shift_30(s->fixed_coeffs[ch][bin+1], atten_tab[1], FRACT_SHIFT);
                }
            } else
                wrap = 0;
        }

        /* blend samples */
        for (bin = writeindex - s->spx_band_sizes[bnd]*12; bin < writeindex; bin++) {
            fract_t sclnsampmant, sclssampmant;
            fract_t nsampmant;
            fract_t coeff = s->fixed_coeffs[ch][bin];

            nsampmant = dither_gen(1, &s->dith_state)>>(FRACT_SHIFT-QUANT_SHIFT);
            sclnsampmant = MUL_Shift_30(nsampmant, nsclmant, FRACT_SHIFT);
            sclssampmant = MUL_Shift_30(coeff, s->spx_coord_sblend[ch][bnd], FRACT_SHIFT);

            coeff = sclnsampmant + sclssampmant;
            s->fixed_coeffs[ch][bin] = MUL_Shift_24(coeff, NSAMPSCLMANT, (FRACT_SHIFT-6));
        } /* bin */

    } /* bnd */

    /* apply notch to end of baseband */
    if (s->spx_atten_code[ch] >= 0) {
        bin = spxbegbin - 2; 
        s->fixed_coeffs[ch][bin] = MUL_Shift_30(s->fixed_coeffs[ch][bin], atten_tab[0], FRACT_SHIFT);
        s->fixed_coeffs[ch][bin+1] = MUL_Shift_30(s->fixed_coeffs[ch][bin+1], atten_tab[1], FRACT_SHIFT);
    }

} /* spx_synthesizetcs */
