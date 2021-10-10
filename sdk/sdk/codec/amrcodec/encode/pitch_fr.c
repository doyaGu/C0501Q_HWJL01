/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : pitch_fr.c
*      Purpose          : Find the pitch period with 1/3 or 1/6 subsample
*                       : resolution (closed loop).
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "pitch_fr.h"
const char pitch_fr_id[] = "@(#)$Id $" pitch_fr_h;
/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
#include "basic_op.h"
//#include "oper_32b.h"
//#include "count.h"
#include "cnst.h"
#include "enc_lag3.h"
#include "enc_lag6.h"
#include "inter_36.h"
#include "inv_sqrt.h"
#include "convolve.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
Pitch_frState   PitchFr;

/*
 * mode dependent parameters used in Pitch_fr()
 * Note: order of MRxx in 'enum Mode' is important!
 */
static const struct {
    Word16 max_frac_lag;     /* lag up to which fractional lags are used    */
    Word16 flag3;            /* enable 1/3 instead of 1/6 fract. resolution */
    Word16 first_frac;       /* first fractional to check                   */
    Word16 last_frac;        /* last fractional to check                    */
    Word16 delta_int_low;    /* integer lag below TO to start search from   */
    Word16 delta_int_range;  /* integer range around T0                     */
    Word16 delta_frc_low;    /* fractional below T0                         */
    Word16 delta_frc_range;  /* fractional range around T0                  */
    Word16 pit_min;          /* minimum pitch                               */
} mode_dep_parm[N_MODES] = {
    /* MR475 */  { 84,  1, -2,  2,  5, 10,  5,  9, PIT_MIN },
    /* MR515 */  { 84,  1, -2,  2,  5, 10,  5,  9, PIT_MIN },
    /* MR59  */  { 84,  1, -2,  2,  3,  6,  5,  9, PIT_MIN },
    /* MR67  */  { 84,  1, -2,  2,  3,  6,  5,  9, PIT_MIN },
    /* MR74  */  { 84,  1, -2,  2,  3,  6,  5,  9, PIT_MIN },
    /* MR795 */  { 84,  1, -2,  2,  3,  6, 10, 19, PIT_MIN },
    /* MR102 */  { 84,  1, -2,  2,  3,  6,  5,  9, PIT_MIN },
    /* MR122 */  { 94,  0, -3,  3,  3,  6,  5,  9, PIT_MIN_MR122 }
};

/*
********************************************************************************
*                         LOCAL PROGRAM CODE
********************************************************************************
*/
/*************************************************************************
 *
 *  FUNCTION:   Norm_Corr()
 *
 *  PURPOSE: Find the normalized correlation between the target vector
 *           and the filtered past excitation.
 *
 *  DESCRIPTION:
 *     The normalized correlation is given by the correlation between the
 *     target and filtered past excitation divided by the square root of
 *     the energy of filtered excitation.
 *                   corr[k] = <x[], y_k[]>/sqrt(y_k[],y_k[])
 *     where x[] is the target vector and y_k[] is the filtered past
 *     excitation at delay k.
 *
 *************************************************************************/
static void Norm_Corr (Word16 exc[], Word16 xn[], Word16 h[], Word16 L_subfr,
                       Word16 t_min, Word16 t_max, Word16 corr_norm[])
{
    Word32 i, j, k;
    Word16 corr_h, corr_l, norm_h, norm_l;
    Word32 s;

    /* Usally dynamic allocation of (L_subfr) */
    Word16 excf[L_SUBFR];
    Word16 scaling, h_fac, *s_excf, scaled_excf[L_SUBFR];

    k = -t_min;

    /* compute the filtered excitation for the first delay t_min */

    Convolve (&exc[k], h, excf, L_subfr);

    /* scale "excf[]" to avoid overflow */

    for (j = 0; j < L_subfr; j++) {
        scaled_excf[j] = excf[j] >> 2;
    }

    /* Compute 1/sqrt(energy of excf[]) */
#ifdef NORMCORR_ENG1
VOLATILE (
        *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) excf;
        *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) excf;
        *(volatile int *)(ALU_OP0+4) =
                                    (FALSE     << P_BP   ) |
                                    (0x3       << P_SAdd2) |
                                    (0x3       << P_SAdd3) |
                                    (SRC_FIFOA << P_Mul0L) |
                                    (SRC_FIFOC << P_Mul0R) |
                                    (SRC_C0    << P_Mul1L) |
                                    (SRC_C0    << P_Mul1R) |
                                    (SRC_C0    << P_Mul2L) |
                                    (SRC_C0    << P_Mul2R) ;

        *(volatile int *)(ALU_OP0)= (SRC_C0     << P_Mul3L) |
                                    (SRC_C0     << P_Mul3R) |
                                    (NoSHIFT    << P_SAdd0) |
                                    (NoSHIFT    << P_SAdd1) |
                                    (DST_NoWr   << P_Dst0 ) |
                                    (DST_NoWr   << P_Dst1 ) |
                                    (DST_NoWr   << P_Dst2 ) |
                                    (DST_NoWr   << P_Dst3 ) ;

        *(volatile int *)RQ_TYPE  = (TRUE       << P_Fire      )|
                                    (FALSE      << P_EnINT     )|
                                    (FALSE      << P_EnInitAcc )|
                                    (TRUE       << P_Sat       )|
                                    (DATA16     << P_RdDT      )|
                                    (DATA32     << P_WrDT      )|
                                    (LARGE      << P_RdGranSize)|
                                    (GRANULE_2  << P_RdGranule )|
                                    (LARGE      << P_WrGranSize)|
                                    (GRANULE_2  << P_WrGranule )|
                                    (L_subfr    << P_Len       )|
                                    (0          << P_RdIncr    )|
                                    (0          << P_WrIncr    );
        WaitEngineDone();
        s = *(volatile int *)ALU_P0;
);

#elif defined NORMCORR_OPRISC1
    asm volatile("l.macrc %0,0" : "=r" (s));
    for (j = 0; j < L_subfr; j++)
    {
        //s += (excf[j] * excf[j]);
        asm volatile("l.mac %0,%1" : : "r"((Word32)excf[j]), "r"((Word32)excf[j]));
    }
    asm volatile("l.macrc %0,0" : "=r" (s));
#elif defined NORMCORR_PUREC1
    s = 0;
    for (j = 0; j < L_subfr; j++)
        s += (excf[j] * excf[j]);
    //s <<=1;
#else
#error "error"
#endif
//  s = L_smac (0, excf, excf, L_subfr, 0 );
    if (s <= 33554432L)         /* if (s <= 2^25) */
    {
        s_excf = excf;
        h_fac = 3;
        scaling = 0;
    }
    else
    {
        /* "excf[]" is divided by 2 */
        s_excf = scaled_excf;
        h_fac = 1;
        scaling = 2;
    }

    /* loop for every possible period */

    for (i = t_min; i <= t_max; i++)
    {
        /* Compute 1/sqrt(energy of excf[]) */

#ifdef NORMCORR_ENG2
VOLATILE (
        *(volatile int *)(ALU_OP0)= (SRC_C0     << P_Mul3L) |
                                    (SRC_C0     << P_Mul3R) |
                                    (NoSHIFT    << P_SAdd0) |
                                    (NoSHIFT    << P_SAdd1) |
                                    (DST_NoWr   << P_Dst0 ) |
                                    (DST_NoWr   << P_Dst1 ) |
                                    (DST_NoWr   << P_Dst2 ) |
                                    (DST_NoWr   << P_Dst3 ) ;
        *(volatile int *)(ALU_OP0+4) =
                                    (FALSE     << P_BP   ) |
                                    (0x3       << P_SAdd2) |
                                    (0x3       << P_SAdd3) |
                                    (SRC_FIFOA << P_Mul0L) |
                                    (SRC_FIFOC << P_Mul0R) |
                                    (SRC_C0    << P_Mul1L) |
                                    (SRC_C0    << P_Mul1R) |
                                    (SRC_C0    << P_Mul2L) |
                                    (SRC_C0    << P_Mul2R) ;
        *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) s_excf;
        *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) s_excf;
        *(volatile int *)RQ_TYPE  = (TRUE       << P_Fire      )|
                                    (FALSE      << P_EnINT     )|
                                    (FALSE      << P_EnInitAcc )|
                                    (TRUE       << P_Sat       )|
                                    (DATA16     << P_RdDT      )|
                                    (DATA32     << P_WrDT      )|
                                    (LARGE      << P_RdGranSize)|
                                    (GRANULE_2  << P_RdGranule )|
                                    (LARGE      << P_WrGranSize)|
                                    (GRANULE_2  << P_WrGranule )|
                                    (L_subfr    << P_Len       )|
                                    (0          << P_RdIncr    )|
                                    (0          << P_WrIncr    );
        WaitEngineDone();
        s = *(volatile int *)ALU_P0;
        s<<=1;
);

#elif defined NORMCORR_OPRISC2

        for (j = 0; j < L_subfr; j++)
        {
            //s += (s_excf[j] * s_excf[j]);
            asm volatile("l.mac %0,%1" : : "r"((Word32)s_excf[j]), "r"((Word32)s_excf[j]));
        }
        asm volatile("l.maclc %0,1" : "=r" (s));
#elif defined NORMCORR_PUREC2
        s = 0;
        for (j = 0; j < L_subfr; j++)
            s += (s_excf[j] * s_excf[j]);
        s <<= 1;
#else
#error "error"
#endif

//      s = L_smac (0, s_excf, s_excf, L_subfr, 0);
        s = Inv_sqrt (s);
//        L_Extract (s, &norm_h, &norm_l);
        s >>= 1;
        norm_h = (Word16) (s >> 15);
        norm_l = (Word16) (s - (Word32)(norm_h<<15));

        /* Compute correlation between xn[] and excf[] */
#ifdef NORMCORR_ENG3
VOLATILE (
        *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) xn;
        *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) s_excf;
        /**(volatile int *)(ALU_OP0)= (SRC_C0     << P_Mul3L) |
                                    (SRC_C0     << P_Mul3R) |
                                    (NoSHIFT    << P_SAdd0) |
                                    (NoSHIFT    << P_SAdd1) |
                                    (DST_NoWr   << P_Dst0 ) |
                                    (DST_NoWr   << P_Dst1 ) |
                                    (DST_NoWr   << P_Dst2 ) |
                                    (DST_NoWr   << P_Dst3 ) ;*/
        *(volatile int *)RQ_TYPE  = (TRUE       << P_Fire      )|
                                    (FALSE      << P_EnINT     )|
                                    (FALSE      << P_EnInitAcc )|
                                    (TRUE       << P_Sat       )|
                                    (DATA16     << P_RdDT      )|
                                    (DATA32     << P_WrDT      )|
                                    (LARGE      << P_RdGranSize)|
                                    (GRANULE_2  << P_RdGranule )|
                                    (LARGE      << P_WrGranSize)|
                                    (GRANULE_2  << P_WrGranule )|
                                    (L_subfr    << P_Len       )|
                                    (0          << P_RdIncr    )|
                                    (0          << P_WrIncr    );
        WaitEngineDone();
        s = *(volatile int *)ALU_P0;
);

#elif defined NORMCORR_OPRISC3
        for (j = 0; j < L_subfr; j++) {
            //s += (xn[j] * s_excf[j]);
            asm volatile("l.mac %0,%1" : : "r"((Word32)xn[j]), "r"((Word32)s_excf[j]));
        }
        asm volatile("l.macrc %0,0" : "=r" (s));
#elif defined NORMCORR_PUREC3
        s = 0;
        for (j = 0; j < L_subfr; j++)
            s += (xn[j] * s_excf[j]);
#else
#error "error"
#endif
//      s = L_smac (0, xn, s_excf, L_subfr, 0 );
//        L_Extract (s, &corr_h, &corr_l);
        corr_h = (Word16) (s >> 15);
        corr_l = (Word16) (s - (Word32)(corr_h<<15));

        /* Normalize correlation = correlation * (1/sqrt(energy)) */

//        s = Mpy_32 (corr_h, corr_l, norm_h, norm_l);
        s = corr_h * norm_h;
        s += (Word32)( (corr_h * norm_l)>>15 );
        s += (Word32)( (corr_l * norm_h)>>15 );
        corr_norm[i] = (Word16)( s<<1 );
//        corr_norm[i] = extract_h (L_shl (s, 16));

            /* modify the filtered excitation excf[] for the next iteration */

        if (i != t_max)
    {
            k--;
#ifdef NORMCORR_ENG4
VOLATILE (
            *(volatile int *)UsrDefC0 = (int)(exc[k]);
        *(volatile int *)Src0Base = (TRUE  << P_RdDec) | (int) &h[39];
            *(volatile int *)Src2Base = (TRUE  << P_RdDec) | (int) &s_excf[38];
            *(volatile int *)Dst0Base = (TRUE  << P_RdDec) | (int) &s_excf[39];
            *(volatile int *)(ALU_OP0+4) =
                                        (FALSE     << P_BP   ) |
                                        (0x1       << P_SAdd2) |
                                        (0x1       << P_SAdd3) |
                                        (SRC_FIFOA << P_Mul0L) |
                                        (SRC_UDC0  << P_Mul0R) |
                                        (SRC_C0    << P_Mul1L) |
                                        (SRC_C0    << P_Mul1R) |
                                        (SRC_FIFOC << P_Mul2L) |
                                        (SRC_C1    << P_Mul2R) ;
            if(h_fac==1) // s >> 14
        {

               //*(volatile int *)Src2Base = (TRUE  << P_RdDec) | (int) &scaled_excf[38];
               //*(volatile int *)Dst0Base = (TRUE  << P_RdDec) | (int) &scaled_excf[39];
               *(volatile int *)(ALU_OP0)= (SRC_C0     << P_Mul3L) |
                                           (SRC_C0     << P_Mul3R) |
                                           (R_SHIFT14  << P_SAdd0) |
                                           (NoSHIFT    << P_SAdd1) |
                                           (DST_P0     << P_Dst0 ) |
                                           (DST_NoWr   << P_Dst1 ) |
                                           (DST_NoWr   << P_Dst2 ) |
                                           (DST_NoWr   << P_Dst3 ) ;

               *(volatile int *)RQ_TYPE  = (TRUE       << P_Fire      )|
                                           (FALSE      << P_EnINT     )|
                                           (FALSE      << P_EnInitAcc )|
                                           (TRUE       << P_Sat       )|
                                           (DATA16     << P_RdDT      )|
                                           (DATA16     << P_WrDT      )|
                                           (LARGE      << P_RdGranSize)|
                                           (GRANULE_2  << P_RdGranule )|
                                           (LARGE      << P_WrGranSize)|
                                           (GRANULE_1  << P_WrGranule )|
                                           (39         << P_Len       )|
                                           (0          << P_RdIncr    )|
                                           (0          << P_WrIncr    );
               WaitEngineDone();

        }
            else  // s >> 12
        {
           //*(volatile int *)Src2Base = (TRUE  << P_RdDec) | (int) &excf[38];
               //*(volatile int *)Dst0Base = (TRUE  << P_RdDec) | (int) &excf[39];
           *(volatile int *)(ALU_OP0)= (SRC_C0     << P_Mul3L) |
                                           (SRC_C0     << P_Mul3R) |
                                           (R_SHIFT12  << P_SAdd0) |
                                           (NoSHIFT    << P_SAdd1) |
                                           (DST_P0     << P_Dst0 ) |
                                           (DST_NoWr   << P_Dst1 ) |
                                           (DST_NoWr   << P_Dst2 ) |
                                           (DST_NoWr   << P_Dst3 ) ;

               *(volatile int *)RQ_TYPE  = (TRUE       << P_Fire      )|
                                           (FALSE      << P_EnINT     )|
                                           (FALSE      << P_EnInitAcc )|
                                           (TRUE       << P_Sat       )|
                                           (DATA16     << P_RdDT      )|
                                           (DATA16     << P_WrDT      )|
                                           (LARGE      << P_RdGranSize)|
                                           (GRANULE_2  << P_RdGranule )|
                                           (LARGE      << P_WrGranSize)|
                                           (GRANULE_1  << P_WrGranule )|
                                           (39         << P_Len       )|
                                           (0          << P_RdIncr    )|
                                           (0          << P_WrIncr    );
               WaitEngineDone();
            }
);
#elif defined NORMCORR_PUREC4
            for (j = L_subfr - 1; j > 0; j--)
        {
                s = exc[k] * h[j];
//                s <<= h_fac;
                s_excf[j] = (Word16)( s>>(15-h_fac) ) + s_excf[j - 1];
            }
#else
#error "error"
#endif
            s_excf[0] = exc[k] >> scaling;
        }
    }
    return;
}

/*************************************************************************
 *
 *  FUNCTION:   searchFrac()
 *
 *  PURPOSE: Find fractional pitch
 *
 *  DESCRIPTION:
 *     The function interpolates the normalized correlation at the
 *     fractional positions around lag T0. The position at which the
 *     interpolation function reaches its maximum is the fractional pitch.
 *     Starting point of the search is frac, end point is last_frac.
 *     frac is overwritten with the fractional pitch.
 *
 *************************************************************************/
static void searchFrac (
    Word16 *lag,       /* i/o : integer pitch           */
    Word16 *frac,      /* i/o : start point of search -
                                fractional pitch        */
    Word16 last_frac,  /* i   : endpoint of search      */
    Word16 corr[],     /* i   : normalized correlation  */
    Word16 flag3       /* i   : subsample resolution
                                (3: =1 / 6: =0)         */
)
{
    Word16 i;
    Word16 max;
    Word16 corr_int;

    /* Test the fractions around T0 and choose the one which maximizes   */
    /* the interpolated normalized correlation.                          */

    max = Interpol_3or6 (&corr[*lag], *frac, flag3); /* function result */

    for (i = (*frac + 1); i <= last_frac; i++) {
        corr_int = Interpol_3or6 (&corr[*lag], i, flag3);
        if (corr_int > max)
        {
            max = corr_int;
            *frac = i;
        }
    }

    if (flag3 == 0) {
        /* Limit the fraction value in the interval [-2,-1,0,1,2,3] */

        if (*frac == -3)
        {
            *frac = 3;
            *lag = *lag - 1;
        }
    }
    else {
        /* limit the fraction value between -1 and 1 */

        if (*frac == -2)
        {
            *frac = 1;
            *lag = *lag - 1;
        }
        if (*frac == 2)
        {
            *frac = -1;
            *lag = *lag + 1;
        }
    }
}
/*************************************************************************
 *
 *  FUNCTION:   getRange()
 *
 *  PURPOSE: Sets range around open-loop pitch or integer pitch of last subframe
 *
 *  DESCRIPTION:
 *     Takes integer pitch T0 and calculates a range around it with
 *       t0_min = T0-delta_low  and t0_max = (T0-delta_low) + delta_range
 *     t0_min and t0_max are bounded by pitmin and pitmax
 *
 *************************************************************************/
static void getRange (
    Word16 T0,           /* i : integer pitch          */
    Word16 delta_low,    /* i : search start offset    */
    Word16 delta_range,  /* i : search range           */
    Word16 pitmin,       /* i : minimum pitch          */
    Word16 pitmax,       /* i : maximum pitch          */
    Word16 *t0_min,      /* o : search range minimum   */
    Word16 *t0_max)      /* o : search range maximum   */
{
    *t0_min = T0 - delta_low;
    if (*t0_min < pitmin)
    {
        *t0_min = pitmin;
    }
    *t0_max = *t0_min + delta_range;
    if (*t0_max > pitmax)
    {
        *t0_max = pitmax;
        *t0_min = *t0_max - delta_range;
    }
}

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/

/*************************************************************************
*
*  Function:   Pitch_fr_init
*  Purpose:    Allocates state memory and initializes state memory
*
**************************************************************************
*/
int Pitch_fr_init (Pitch_frState **state)
{
//    Pitch_frState* s;

//    if (state == (Pitch_frState **) NULL){
//        fprintf(stderr, "Pitch_fr_init: invalid parameter\n");
//        return -1;
//    }
//    *state = NULL;

    /* allocate memory */
//    if ((s= (Pitch_frState *) malloc(sizeof(Pitch_frState))) == NULL){
//        fprintf(stderr, "Pitch_fr_init: can not malloc state structure\n");
//        return -1;
//    }

    Pitch_fr_reset(&PitchFr);
    *state = &PitchFr;

    return 0;
}

/*************************************************************************
*
*  Function:   Pitch_fr_reset
*  Purpose:    Initializes state memory to zero
*
**************************************************************************
*/
int Pitch_fr_reset (Pitch_frState *state)
{

//    if (state == (Pitch_frState *) NULL){
//        fprintf(stderr, "Pitch_fr_reset: invalid parameter\n");
//        return -1;
//    }

    state->T0_prev_subframe = 0;

    return 0;
}

/*************************************************************************
*
*  Function:   Pitch_fr_exit
*  Purpose:    The memory used for state memory is freed
*
**************************************************************************
*/
//void Pitch_fr_exit (Pitch_frState **state)
//{
//    if (state == NULL || *state == NULL)
//        return;

    /* deallocate memory */
//    free(*state);
//    *state = NULL;

//    return;
//}

/*************************************************************************
 *
 *  FUNCTION:   Pitch_fr()
 *
 *  PURPOSE: Find the pitch period with 1/3 or 1/6 subsample resolution
 *           (closed loop).
 *
 *  DESCRIPTION:
 *        - find the normalized correlation between the target and filtered
 *          past excitation in the search range.
 *        - select the delay with maximum normalized correlation.
 *        - interpolate the normalized correlation at fractions -3/6 to 3/6
 *          with step 1/6 around the chosen delay.
 *        - The fraction which gives the maximum interpolated value is chosen.
 *
 *************************************************************************/
Word16 Pitch_fr (        /* o   : pitch period (integer)                    */
    Pitch_frState *st,   /* i/o : State struct                              */
    enum Mode mode,      /* i   : codec mode                                */
    Word16 T_op[],       /* i   : open loop pitch lags                      */
    Word16 exc[],        /* i   : excitation buffer                      Q0 */
    Word16 xn[],         /* i   : target vector                          Q0 */
    Word16 h[],          /* i   : impulse response of synthesis and
                                  weighting filters                     Q12 */
    Word16 L_subfr,      /* i   : Length of subframe                        */
    Word16 i_subfr,      /* i   : subframe offset                           */
    Word16 *pit_frac,    /* o   : pitch period (fractional)                 */
    Word16 *resu3,       /* o   : subsample resolution 1/3 (=1) or 1/6 (=0) */
    Word16 *ana_index    /* o   : index of encoding                         */
)
{
    Word32 i;
    Word16 t_min, t_max;
    Word16 t0_min, t0_max;
    Word16 max, lag, frac;
    Word16 tmp_lag;
    Word16 *corr;
    Word16 corr_v[40];    /* Total length = t0_max-t0_min+1+2*L_INTER_SRCH */

    Word16 max_frac_lag;
    Word16 flag3, flag4;
    Word16 last_frac;
    Word16 delta_int_low, delta_int_range;
    Word16 delta_frc_low, delta_frc_range;
    Word16 pit_min;
    Word16 frame_offset;
    Word16 delta_search;

    /*-----------------------------------------------------------------------*
     *                      set mode specific variables                      *
     *-----------------------------------------------------------------------*/

    max_frac_lag    = mode_dep_parm[mode].max_frac_lag;
    flag3           = mode_dep_parm[mode].flag3;
    frac            = mode_dep_parm[mode].first_frac;
    last_frac       = mode_dep_parm[mode].last_frac;
    delta_int_low   = mode_dep_parm[mode].delta_int_low;
    delta_int_range = mode_dep_parm[mode].delta_int_range;

    delta_frc_low   = mode_dep_parm[mode].delta_frc_low;
    delta_frc_range = mode_dep_parm[mode].delta_frc_range;
    pit_min         = mode_dep_parm[mode].pit_min;

    /*-----------------------------------------------------------------------*
     *                 decide upon full or differential search               *
     *-----------------------------------------------------------------------*/

    delta_search = 1;

    if ((i_subfr == 0) || (i_subfr == L_FRAME_BY2) )
    {
        /* Subframe 1 and 3 */
        if ( ( ((Word16)mode!= MR475) && ((Word16)mode != MR515) ) ||
            (i_subfr != L_FRAME_BY2) )
        {
            /* set t0_min, t0_max for full search */
            /* this is *not* done for mode MR475, MR515 in subframe 3 */

            delta_search = 0; /* no differential search */

            /* calculate index into T_op which contains the open-loop */
            /* pitch estimations for the 2 big subframes */

            frame_offset = 1;
            if (i_subfr == 0)
                frame_offset = 0;

            /* get T_op from the corresponding half frame and */
            /* set t0_min, t0_max */

            getRange (T_op[frame_offset], delta_int_low, delta_int_range,
                      pit_min, PIT_MAX, &t0_min, &t0_max);
        }
        else {

            /* mode MR475, MR515 and 3. Subframe: delta search as well */
            getRange (st->T0_prev_subframe, delta_frc_low, delta_frc_range,
                      pit_min, PIT_MAX, &t0_min, &t0_max);
        }
    }
    else {

        /* for Subframe 2 and 4 */
        /* get range around T0 of previous subframe for delta search */

        getRange (st->T0_prev_subframe, delta_frc_low, delta_frc_range,
                  pit_min, PIT_MAX, &t0_min, &t0_max);
    }

    /*-----------------------------------------------------------------------*
     *           Find interval to compute normalized correlation             *
     *-----------------------------------------------------------------------*/

    t_min = t0_min - L_INTER_SRCH; // L_INTER_SRCH = 4
    t_max = t0_max + L_INTER_SRCH;

    corr = &corr_v[-t_min];
    /*-----------------------------------------------------------------------*
     * Compute normalized correlation between target and filtered excitation *
     *-----------------------------------------------------------------------*/

    Norm_Corr (exc, xn, h, L_subfr, t_min, t_max, corr);

    /*-----------------------------------------------------------------------*
     *                           Find integer pitch                          *
     *-----------------------------------------------------------------------*/

    max = corr[t0_min];
    lag = t0_min;

    for (i = t0_min + 1; i <= t0_max; i++)
    {
        if (corr[i] >= max)
        {
            max = corr[i];
            lag = (Word16)i;
        }
    }

    /*-----------------------------------------------------------------------*
     *                        Find fractional pitch                          *
     *-----------------------------------------------------------------------*/
    if ((delta_search == 0) && (lag > max_frac_lag))
    {
        /* full search and integer pitch greater than max_frac_lag */
        /* fractional search is not needed, set fractional to zero */

        frac = 0;
    }
    else
    {
        /* if differential search AND mode MR475 OR MR515 OR MR59 OR MR67   */
        /* then search fractional with 4 bits resolution           */
       if ((delta_search != 0) &&
           (((Word16)mode == MR475) ||
            ((Word16)mode == MR515) ||
            ((Word16)mode == MR59) ||
            ((Word16)mode == MR67)))
       {

          /* modify frac or last_frac according to position of last */
          /* integer pitch: either search around integer pitch, */
          /* or only on left or right side */

          tmp_lag = st->T0_prev_subframe;
          if ( (tmp_lag - t0_min) > 5)
             tmp_lag = t0_min + 5;
          if ( (t0_max - tmp_lag) > 4)
               tmp_lag = t0_max - 4;

          if ( (lag == tmp_lag) || ((tmp_lag - 1) == lag))

          {
             /* normal search in fractions around T0 */

             searchFrac (&lag, &frac, last_frac, corr, flag3);

          }
          else if ((tmp_lag - 2) == lag)
          {
             /* limit search around T0 to the right side */
             frac = 0;
             searchFrac (&lag, &frac, last_frac, corr, flag3);
          }
          else if ((tmp_lag + 1) == lag) {
             /* limit search around T0 to the left side */
             last_frac = 0;
             searchFrac (&lag, &frac, last_frac, corr, flag3);
          }
          else {
             /* no fractional search */
             frac = 0;
            }
       }
       else
          /* test the fractions around T0 */
          searchFrac (&lag, &frac, last_frac, corr, flag3);
    }

    /*-----------------------------------------------------------------------*
     *                           encode pitch                                *
     *-----------------------------------------------------------------------*/

    if (flag3 != 0)
    {
       /* flag4 indicates encoding with 4 bit resolution;         */
       /* this is needed for mode MR475, MR515 and MR59           */

       flag4 = 0;
       if ( ((Word16)mode == MR475) ||
            ((Word16)mode == MR515) ||
            ((Word16)mode == MR59) ||
            ((Word16)mode == MR67) )
       {
          flag4 = 1;
       }

       /* encode with 1/3 subsample resolution */

       *ana_index = Enc_lag3(lag, frac, st->T0_prev_subframe,
                             t0_min, t0_max, delta_search, flag4); /* function result */

    }
    else
    {
       /* encode with 1/6 subsample resolution */

       *ana_index = Enc_lag6(lag, frac, t0_min, delta_search); /* function result */
    }

    /*-----------------------------------------------------------------------*
     *                          update state variables                       *
     *-----------------------------------------------------------------------*/

    st->T0_prev_subframe = lag;

    /*-----------------------------------------------------------------------*
     *                      update output variables                          *
     *-----------------------------------------------------------------------*/

    *resu3    = flag3;

    *pit_frac = frac;

    return (lag);
}
