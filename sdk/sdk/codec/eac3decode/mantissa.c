/*!
*  \file mantissa.c
*
* \brief SPX module decode-side utility functions.
*
*  Part of the Spectral Extension Module.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "ac3dec.h"

#define DITHSCALE     FRACT(0.707106781) /*!< Mantissa dither scale factor */

#define Q(x) (quantizer_t)((x) * (1 << QUANT_SHIFT))

#define Q0 Q (-2./3.)
#define Q1 Q (0)
#define Q2 Q (2./3.)

const quantizer_t b1_mantissas_0[32] = {
    Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0,
    Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1,
    Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2,
    0, 0, 0, 0, 0
};

const quantizer_t b1_mantissas_1[32] = {
    Q0, Q0, Q0, Q1, Q1, Q1, Q2, Q2, Q2,
    Q0, Q0, Q0, Q1, Q1, Q1, Q2, Q2, Q2,
    Q0, Q0, Q0, Q1, Q1, Q1, Q2, Q2, Q2,
    0, 0, 0, 0, 0
};

const quantizer_t b1_mantissas_2[32] = {
    Q0, Q1, Q2, Q0, Q1, Q2, Q0, Q1, Q2,
    Q0, Q1, Q2, Q0, Q1, Q2, Q0, Q1, Q2,
    Q0, Q1, Q2, Q0, Q1, Q2, Q0, Q1, Q2,
    0, 0, 0, 0, 0
};

#undef Q0
#undef Q1
#undef Q2

#define Q0 Q (-4./5.)
#define Q1 Q (-2./5.)
#define Q2 Q (0)
#define Q3 Q (2./5.)
#define Q4 Q (4./5.)

const quantizer_t b2_mantissas_0[128] = {
    Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0,
    Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1,
    Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2,
    Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3,
    Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4,
    0, 0, 0
};

const quantizer_t b2_mantissas_1[128] = {
    Q0, Q0, Q0, Q0, Q0, Q1, Q1, Q1, Q1, Q1, Q2, Q2, Q2, Q2, Q2, Q3, Q3, Q3, Q3, Q3, Q4, Q4, Q4, Q4, Q4,
    Q0, Q0, Q0, Q0, Q0, Q1, Q1, Q1, Q1, Q1, Q2, Q2, Q2, Q2, Q2, Q3, Q3, Q3, Q3, Q3, Q4, Q4, Q4, Q4, Q4,
    Q0, Q0, Q0, Q0, Q0, Q1, Q1, Q1, Q1, Q1, Q2, Q2, Q2, Q2, Q2, Q3, Q3, Q3, Q3, Q3, Q4, Q4, Q4, Q4, Q4,
    Q0, Q0, Q0, Q0, Q0, Q1, Q1, Q1, Q1, Q1, Q2, Q2, Q2, Q2, Q2, Q3, Q3, Q3, Q3, Q3, Q4, Q4, Q4, Q4, Q4,
    Q0, Q0, Q0, Q0, Q0, Q1, Q1, Q1, Q1, Q1, Q2, Q2, Q2, Q2, Q2, Q3, Q3, Q3, Q3, Q3, Q4, Q4, Q4, Q4, Q4,
    0, 0, 0
};

const quantizer_t b2_mantissas_2[128] = {
    Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4,
    Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4,
    Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4,
    Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4,
    Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4, Q0, Q1, Q2, Q3, Q4,
    0, 0, 0
};

#undef Q0
#undef Q1
#undef Q2
#undef Q3
#undef Q4

const quantizer_t b3_mantissas[8] = {
    Q(-6./7.), Q(-4./7.), Q(-2./7.), Q(0), Q(2./7.), Q(4./7.), Q(6./7.), 0
};

#define Q0 Q (-10./11.)
#define Q1 Q (-8./11.)
#define Q2 Q (-6./11.)
#define Q3 Q (-4./11.)
#define Q4 Q (-2./11.)
#define Q5 Q (0)
#define Q6 Q (2./11.)
#define Q7 Q (4./11.)
#define Q8 Q (6./11.)
#define Q9 Q (8./11.)
#define QA Q (10./11.)

const quantizer_t b4_mantissas_0[128] = {
    Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0, Q0,
    Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1, Q1,
    Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2, Q2,
    Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3, Q3,
    Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4, Q4,
    Q5, Q5, Q5, Q5, Q5, Q5, Q5, Q5, Q5, Q5, Q5,
    Q6, Q6, Q6, Q6, Q6, Q6, Q6, Q6, Q6, Q6, Q6,
    Q7, Q7, Q7, Q7, Q7, Q7, Q7, Q7, Q7, Q7, Q7,
    Q8, Q8, Q8, Q8, Q8, Q8, Q8, Q8, Q8, Q8, Q8,
    Q9, Q9, Q9, Q9, Q9, Q9, Q9, Q9, Q9, Q9, Q9,
    QA, QA, QA, QA, QA, QA, QA, QA, QA, QA, QA,
    0, 0, 0, 0, 0, 0, 0
};

const quantizer_t b4_mantissas_1[128] = {
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7, Q8, Q9, QA,
    0, 0, 0, 0, 0, 0, 0
};

#undef Q0
#undef Q1
#undef Q2
#undef Q3
#undef Q4
#undef Q5
#undef Q6
#undef Q7
#undef Q8
#undef Q9
#undef QA

const quantizer_t b5_mantissas[16] = {
    Q(-14./15.), Q(-12./15.), Q(-10./15.), Q(-8./15.), Q(-6./15.),
    Q(-4./15.), Q(-2./15.), Q(0), Q(2./15.), Q(4./15.),
    Q(6./15.), Q(8./15.), Q(10./15.), Q(12./15.), Q(14./15.), 0
};

/**
 * Quantization table: levels for symmetric. bits for asymmetric.
 * reference: Table 7.18 Mapping of bap to Quantizer
 */
static const uint8_t quantization_tab[16] = {
	0, 5, 7, 3, 7, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16};
//    0, 3, 5, 7, 11, 15,
//    5, 6, 7, 8, 9, 10, 11, 12, 14, 16};

/**
 * Decode the transform coefficients for a particular channel
 * reference: Section 7.3 Quantization and Decoding of Mantissas
 */
void ac3_mant_ch_unp(AC3DecodeContext *s, int ch_index, mant_groups *m)
{
    int start_freq = s->start_freq[ch_index];
    int end_freq = s->end_freq[ch_index];
    uint8_t *baps = s->bap[ch_index];
    //int8_t *exps = s->dexps[ch_index];
    int *coeffs = (int*)s->fixed_coeffs[ch_index];
    int dither = s->dither_flag[ch_index];
    GetBitContext *gbc = &s->gbc;
    int freq;

    for(freq = start_freq; freq < end_freq; freq++){
        int bap = baps[freq];
        int mantissa;  // QUANT format
        switch(bap){
            case 0:
                if (dither) {
                    mantissa = (quantizer_t)dither_gen(0, &s->dith_state);
                    mantissa = MUL_Shift_30(mantissa, DITHSCALE, FRACT_SHIFT);
                } else
                    mantissa = 0;
                break;
            case 1:
                if(m->b1){
                    m->b1--;
                    mantissa = m->b1_mant[m->b1];
                }
                else{
                    int bits      = get_bits(gbc, 5);
                    mantissa      = b1_mantissas_0[bits];
                    m->b1_mant[1] = b1_mantissas_1[bits];
                    m->b1_mant[0] = b1_mantissas_2[bits];
                    m->b1         = 2;
                }
                break;
            case 2:
                if(m->b2){
                    m->b2--;
                    mantissa = m->b2_mant[m->b2];
                }
                else{
                    int bits      = get_bits(gbc, 7);
                    mantissa      = b2_mantissas_0[bits];
                    m->b2_mant[1] = b2_mantissas_1[bits];
                    m->b2_mant[0] = b2_mantissas_2[bits];
                    m->b2         = 2;
                }
                break;
            case 3:
                mantissa = b3_mantissas[get_bits(gbc, 3)];
                break;
            case 4:
                if(m->b4){
                    m->b4 = 0;
                    mantissa = m->b4_mant;
                }
                else{
                    int bits   = get_bits(gbc, 7);
                    mantissa   = b4_mantissas_0[bits];
                    m->b4_mant = b4_mantissas_1[bits];
                    m->b4      = 1;
                }
                break;
            case 5:
                mantissa = b5_mantissas[get_bits(gbc, 4)];
                break;
            default: /* 6 to 15 */
                mantissa = get_bits(gbc, quantization_tab[bap]);
                /* Shift mantissa and sign-extend it. */
                mantissa = (mantissa << (32-quantization_tab[bap]))>>(31-QUANT_SHIFT);
                break;
        }
        coeffs[freq] = mantissa;// >> exps[freq];  //QUANT
    }
}
