/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : calc_en.c
*      Purpose          : (pre-) quantization of pitch gain for MR795
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "calc_en.h"
const char calc_en_id[] = "@(#)$Id $" calc_en_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
//#include <stdio.h>
//#include <stdlib.h>
#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
//#include "count.h"
#include "cnst.h"
#include "log2.h"

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/

/*************************************************************************
 *
 * FUNCTION: calc_unfilt_energies
 *
 * PURPOSE:  calculation of several energy coefficients for unfiltered
 *           excitation signals and the LTP coding gain
 *
 *       frac_en[0]*2^exp_en[0] = <res res>   // LP residual energy
 *       frac_en[1]*2^exp_en[1] = <exc exc>   // LTP residual energy
 *       frac_en[2]*2^exp_en[2] = <exc code>  // LTP/CB innovation dot product
 *       frac_en[3]*2^exp_en[3] = <lres lres> // LTP residual energy
 *                                            // (lres = res - gain_pit*exc)
 *       ltpg = log2(LP_res_en / LTP_res_en)
 *
 *************************************************************************/
void
calc_unfilt_energies(
    Word16 res[],     /* i  : LP residual,                               Q0  */
    Word16 exc[],     /* i  : LTP excitation (unfiltered),               Q0  */
    Word16 code[],    /* i  : CB innovation (unfiltered),                Q13 */
    Word16 gain_pit,  /* i  : pitch gain,                                Q14 */
    Word16 L_subfr,   /* i  : Subframe length                                */

    Word16 frac_en[], /* o  : energy coefficients (4), fraction part,    Q15 */
    Word16 exp_en[],  /* o  : energy coefficients (4), exponent part,    Q0  */
    Word16 *ltpg      /* o  : LTP coding gain (log2()),                  Q13 */
)
{
    Word32 s, L_temp;
    Word16 i, exp, tmp;
    Word16 ltp_res_en, pred_gain;
    Word16 ltpg_exp, ltpg_frac;

    /* Compute residual energy */
#ifdef CALCUNFILT_ENG1
    VOLATILE (
    *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) res;
    *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) res;
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
                                    (L_SHIFT1   << P_SAdd0) |
                                    (L_SHIFT1   << P_SAdd1) |
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
#elif defined CALCUNFILT_PUREC1
    s = L_mac((Word32) 0, res[0], res[0]);
    for (i = 1; i < L_subfr; i++)
        s = L_mac(s, res[i], res[i]);
#else
#error "error"
#endif
    //s = 0L;
    //for (i = 0; i < L_subfr; i++)
    //  s += (res[i] * res[i]);

    //s <<= 1;

    /* ResEn := 0 if ResEn < 200.0 (= 400 Q1) */

    //if (L_sub (s, 400L) < 0)
    if (s < 400L)
    {
        frac_en[0] = 0;
        exp_en[0] = -15;
    }
    else
    {
        exp = norm_l(s);
        //frac_en[0] = extract_h(L_shl(s, exp));
        if ( exp < 16 )
          frac_en[0] = (Word16)( s>>(16-exp) );
        else
          frac_en[0] = (Word16)( s<<(exp-16) );

        //exp_en[0] = sub(15, exp);
        exp_en[0] = 15 - exp;
    }

    /* Compute ltp excitation energy */
#ifdef CALCUNFILT_ENG2
VOLATILE (
    *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) exc;
    *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) exc;
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
#elif defined CALCUNFILT_PUREC2
    s = L_mac((Word32) 0, exc[0], exc[0]);
    for (i = 1; i < L_subfr; i++)
        s = L_mac(s, exc[i], exc[i]);
#else
#error "error"
#endif

    exp = norm_l(s);

    //frac_en[1] = extract_h(L_shl(s, exp));
    if ( exp < 16 )
      frac_en[1] = (Word16)( s>>(16-exp) );
    else
      frac_en[1] = (Word16)( s<<(exp-16) );

    //exp_en[1] = sub(15, exp);
    exp_en[1] = 15 - exp;

    /* Compute scalar product <exc[],code[]> */
#ifdef CALCUNFILT_ENG3
VOLATILE (
    *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) exc;
    *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) code;
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
#elif defined CALCUNFILT_PUREC3
    s = L_mac((Word32) 0, exc[0], code[0]);
    for (i = 1; i < L_subfr; i++)
        s = L_mac(s, exc[i], code[i]);
#else
#error "error"
#endif

    exp = norm_l(s);
    //frac_en[2] = extract_h(L_shl(s, exp));
    if ( exp < 16 )
       frac_en[2] = (Word16)( s>>(16-exp) );
    else
       frac_en[2] = (Word16)( s<<(exp-16) );

    //exp_en[2] = sub(16-14, exp);
    exp_en[2] = 2 - exp;

    /* Compute energy of LTP residual */
    s = 0L;
    for (i = 0; i < L_subfr; i++)
    {
        //L_temp = L_mult(exc[i], gain_pit);
        L_temp = exc[i] * gain_pit;

        //L_temp = L_shl(L_temp, 1);
        L_temp <<= 1;

        //tmp = sub(res[i], round(L_temp));           /* LTP residual, Q0 */
        tmp = res[i] - (Word16)( (L_temp+0x00004000L)>> 15 );

        //s = L_mac (s, tmp, tmp);
        s+= tmp*tmp;
    }
    s<<=1;
    exp = norm_l(s);
    //ltp_res_en = extract_h (L_shl (s, exp));
    if ( exp < 16 )
        ltp_res_en = (Word16)( s>>(16-exp) );
    else
        ltp_res_en = (Word16)( s<<(exp-16) );

    //exp = sub (15, exp);
    exp = 15 - exp;

    frac_en[3] = ltp_res_en;
    exp_en[3] = exp;

    /* calculate LTP coding gain, i.e. energy reduction LP res -> LTP res */

    if (ltp_res_en > 0 && frac_en[0] != 0)
    {
        /* gain = ResEn / LTPResEn */
        //pred_gain = div_s (shr (frac_en[0], 1), ltp_res_en);
        pred_gain = div_s ((Word16)(frac_en[0]>>1), ltp_res_en);

        //exp = sub (exp, exp_en[0]);
        exp = exp - exp_en[0];

        /* L_temp = ltpGain * 2^(30 + exp) */
        //L_temp = L_deposit_h (pred_gain);
        L_temp = (Word32)(pred_gain<<16);

        /* L_temp = ltpGain * 2^27 */
        //L_temp = L_shr (L_temp, add (exp, 3));
        L_temp = L_shr (L_temp,(exp+3));
        //if((exp+3)>=0)
        //   L_temp >>= (exp+3);
        //else
        //   L_temp <<= -(exp+3);

        /* Log2 = log2() + 27 */
        Log2(L_temp, &ltpg_exp, &ltpg_frac);

        /* ltpg = log2(LtpGain) * 2^13 --> range: +- 4 = +- 12 dB */
        //L_temp = L_Comp (sub (ltpg_exp, 27), ltpg_frac);
        L_temp = (Word32)((ltpg_exp-27)<<16);
        L_temp += (ltpg_frac<<1);

        //*ltpg = round (L_shl (L_temp, 13)); /* Q13 */
    *ltpg = (Word16)( L_add((L_temp << 13) , 0x00008000L)>>16 );
        //*ltpg = (Word16)( L_temp << 13 );
    }
    else
    {
        *ltpg = 0;
    }
}

/*************************************************************************
 *
 * FUNCTION: calc_filt_energies
 *
 * PURPOSE:  calculation of several energy coefficients for filtered
 *           excitation signals
 *
 *     Compute coefficients need for the quantization and the optimum
 *     codebook gain gcu (for MR475 only).
 *
 *      coeff[0] =    y1 y1
 *      coeff[1] = -2 xn y1
 *      coeff[2] =    y2 y2
 *      coeff[3] = -2 xn y2
 *      coeff[4] =  2 y1 y2
 *
 *
 *      gcu = <xn2, y2> / <y2, y2> (0 if <xn2, y2> <= 0)
 *
 *     Product <y1 y1> and <xn y1> have been computed in G_pitch() and
 *     are in vector g_coeff[].
 *
 *************************************************************************/
void
calc_filt_energies(
    enum Mode mode,     /* i  : coder mode                                   */
    Word16 xn[],        /* i  : LTP target vector,                       Q0  */
    Word16 xn2[],       /* i  : CB target vector,                        Q0  */
    Word16 y1[],        /* i  : Adaptive codebook,                       Q0  */
    Word16 Y2[],        /* i  : Filtered innovative vector,              Q12 */
    Word16 g_coeff[],   /* i  : Correlations <xn y1> <y1 y1>                 */
                        /*      computed in G_pitch()                        */

    Word16 frac_coeff[],/* o  : energy coefficients (5), fraction part,  Q15 */
    Word16 exp_coeff[], /* o  : energy coefficients (5), exponent part,  Q0  */
    Word16 *cod_gain_frac,/* o: optimum codebook gain (fraction part),   Q15 */
    Word16 *cod_gain_exp  /* o: optimum codebook gain (exponent part),   Q0  */
)
{
    Word32 s, ener_init;
    Word16 i, exp, frac;
    Word16 y2[L_SUBFR];

    for (i = 0; i < L_SUBFR; i++) {
        //y2[i] = shr(Y2[i], 3);
        y2[i] = Y2[i] >> 3;
    }

    frac_coeff[0] = g_coeff[0];
    exp_coeff[0] = g_coeff[1];

    //frac_coeff[1] = negate(g_coeff[2]);     /* coeff[1] = -2 xn y1 */
    if(g_coeff[2]==-32768)
      frac_coeff[1] = 32767;
    else
      frac_coeff[1] = -g_coeff[2];

    //exp_coeff[1] = add(g_coeff[3], 1);
    exp_coeff[1] = g_coeff[3] + 1;

    //if (test(), sub((Word16)mode, MR795) == 0 || sub((Word16)mode, MR475) == 0)
    if (((Word16)mode == MR795) || ((Word16)mode == MR475))
    {
        ener_init = 0L;
    }
    else
    {
        ener_init = 1L;
    }

    /* Compute scalar product <y2[],y2[]> */
    //s = ener_init;
    //s = L_mac(ener_init, y2[0], y2[0]);
    //for (i = 0; i < L_SUBFR; i++)
    //    s = L_mac(s, y2[i], y2[i]);
#ifdef CALCFILTENERGIES_ENG1
VOLATILE (
    *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) y2;
    *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) y2;
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
                                    (L_SHIFT1   << P_SAdd0) |
                                    (L_SHIFT1   << P_SAdd1) |
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
                                    (L_SUBFR    << P_Len       )|
                                    (0          << P_RdIncr    )|
                                    (0          << P_WrIncr    );
        WaitEngineDone();
        s = *(volatile int *)ALU_P0;
);

#elif defined CALCFILTENERGIES_OPRISC1
    asm volatile("l.macrc %0,0" : "=r" (s));
    for (i = 0; i < 5; i++)
    {
       asm volatile("l.mac %0,%1" : : "r"((Word32)y2[i*8]), "r"((Word32)y2[i*8]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)y2[(i*8)+1]), "r"((Word32)y2[(i*8)+1]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)y2[(i*8)+2]), "r"((Word32)y2[(i*8)+2]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)y2[(i*8)+3]), "r"((Word32)y2[(i*8)+3]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)y2[(i*8)+4]), "r"((Word32)y2[(i*8)+4]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)y2[(i*8)+5]), "r"((Word32)y2[(i*8)+5]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)y2[(i*8)+6]), "r"((Word32)y2[(i*8)+6]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)y2[(i*8)+7]), "r"((Word32)y2[(i*8)+7]));
    }
    //asm volatile("l.mac %0,%1" : : "r"((Word32)ener_init), "r"(2));
    asm volatile("l.maclc %0,1" : "=r" (s));
#elif defined CALCFILTENERGIES_PUREC1
    s=0;
    for (i = 0; i < L_SUBFR; i++)
        s += (y2[i] * y2[i]);
    s<<=1;
#else
#error "error"
#endif

    s+=ener_init;

    exp = norm_l(s);

    //frac_coeff[2] = extract_h(L_shl(s, exp));
    //  if ( exp < 16 )
    //      frac_coeff[2] = (Word16)( s>>(16-exp) );
    //  else
    //      frac_coeff[2] = (Word16)( s<<(exp-16) );
    frac_coeff[2] = (Word16)((s<<exp)>>16);
    //exp_coeff[2] = sub(15 - 18, exp);
    exp_coeff[2] = -3 - exp;

    /* Compute scalar product -2*<xn[],y2[]> */

    //s = L_mac(ener_init, xn[0], y2[0]);
    //s = ener_init;
    //for (i = 0; i < L_SUBFR; i++)
    //    s = L_mac(s, xn[i], y2[i]);
#ifdef CALCFILTENERGIES_ENG2
VOLATILE (
    *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) xn;
    *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) y2;
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
                                    (L_SUBFR    << P_Len       )|
                                    (0          << P_RdIncr    )|
                                    (0          << P_WrIncr    );
    WaitEngineDone();
    s = *(volatile int *)ALU_P0;
);

#elif defined CALCFILTENERGIES_OPRISC2
    for (i = 0; i < 5; i++)
    {
       asm volatile("l.mac %0,%1" : : "r"((Word32)xn[i*8]), "r"((Word32)y2[i*8]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)xn[(i*8)+1]), "r"((Word32)y2[(i*8)+1]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)xn[(i*8)+2]), "r"((Word32)y2[(i*8)+2]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)xn[(i*8)+3]), "r"((Word32)y2[(i*8)+3]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)xn[(i*8)+4]), "r"((Word32)y2[(i*8)+4]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)xn[(i*8)+5]), "r"((Word32)y2[(i*8)+5]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)xn[(i*8)+6]), "r"((Word32)y2[(i*8)+6]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)xn[(i*8)+7]), "r"((Word32)y2[(i*8)+7]));
    }
    //asm volatile("l.mac %0,%1" : : "r"((Word32)ener_init), "r"(2));
    asm volatile("l.maclc %0,1" : "=r" (s));
    //s<<=1;
#elif defined CALCFILTENERGIES_PUREC2
    s=0;
    for (i = 0; i < L_SUBFR; i++)
        s += (xn[i] * y2[i]);
    s <<= 1;
#else
#error "error"
#endif
    s+=ener_init;

    exp = norm_l(s);

    //frac_coeff[3] = negate(extract_h(L_shl(s, exp)));
    frac_coeff[3] = (Word16)((s<<exp)>>16);
    if(frac_coeff[3]==-32768)
       frac_coeff[3] = 32767;
    else
       frac_coeff[3] = -frac_coeff[3];
    //exp_coeff[3] = sub(15 - 9 + 1, exp);
    exp_coeff[3] = 7 - exp;

    /* Compute scalar product 2*<y1[],y2[]> */

    //s = L_mac(ener_init, y1[0], y2[0]);
    //s = ener_init;
    //for (i = 0; i < L_SUBFR; i++)
    //    s = L_mac(s, y1[i], y2[i]);
#ifdef CALCFILTENERGIES_ENG3
VOLATILE (
    *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) y1;
    *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) y2;
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
                                    (L_SUBFR    << P_Len       )|
                                    (0          << P_RdIncr    )|
                                    (0          << P_WrIncr    );
    WaitEngineDone();
    s = *(volatile int *)ALU_P0;
);

#elif defined CALCFILTENERGIES_OPRISC3
    for (i = 0; i < 5; i++)
    {
       asm volatile("l.mac %0,%1" : : "r"((Word32)y1[i*8]), "r"((Word32)y2[i*8]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)y1[(i*8)+1]), "r"((Word32)y2[(i*8)+1]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)y1[(i*8)+2]), "r"((Word32)y2[(i*8)+2]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)y1[(i*8)+3]), "r"((Word32)y2[(i*8)+3]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)y1[(i*8)+4]), "r"((Word32)y2[(i*8)+4]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)y1[(i*8)+5]), "r"((Word32)y2[(i*8)+5]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)y1[(i*8)+6]), "r"((Word32)y2[(i*8)+6]));
       asm volatile("l.mac %0,%1" : : "r"((Word32)y1[(i*8)+7]), "r"((Word32)y2[(i*8)+7]));
    }

    //for (i = 0; i < L_SUBFR; i++)
    //   asm volatile("l.mac %0,%1" : : "r"((Word32)y1[i]), "r"((Word32)y2[i]));
    //asm volatile("l.mac %0,%1" : : "r"((Word32)ener_init), "r"(2));
    asm volatile("l.maclc %0,1" : "=r" (s));

#elif defined CALCFILTENERGIES_PUREC3
    s = 0;
    for (i = 0; i < L_SUBFR; i++)
        s += (y1[i] * y2[i]);
    s<<=1;
#else
#error "error"
#endif

    s+=ener_init;

    exp = norm_l(s);

    //frac_coeff[4] = extract_h(L_shl(s, exp));
    //if ( exp < 16 )
    //      frac_coeff[4] = (Word16)( s>>(16-exp) );
    //  else
    //      frac_coeff[4] = (Word16)( s<<(exp-16) );
    frac_coeff[4] = (Word16)((s<<exp)>>16);
    //exp_coeff[4] = sub(15 - 9 + 1, exp);
    exp_coeff[4] = 7 - exp;

    //if (test(), test (), sub((Word16)mode, MR475) == 0 || sub((Word16)mode, MR795) == 0)
    if (((Word16)mode == MR475) || ((Word16)mode == MR795))
    {
        /* Compute scalar product <xn2[],y2[]> */

#ifdef CALCFILTENERGIES_ENG795475
      VOLATILE (
    *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) xn2;
    *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) y2;
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
                                    (L_SHIFT1   << P_SAdd0) |
                                    (L_SHIFT1   << P_SAdd1) |
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
                                    (L_SUBFR    << P_Len       )|
                                    (0          << P_RdIncr    )|
                                    (0          << P_WrIncr    );
        WaitEngineDone();
        s = *(volatile int *)ALU_P0;
);

#elif defined CALCFILTENERGIES_PUREC795475
      s = L_mac(ener_init, xn2[0], y2[0]);
        for (i = 1; i < L_SUBFR; i++)
            s = L_mac(s, xn2[i], y2[i]);
#else
#error "error"
#endif
        exp = norm_l(s);

        //frac = extract_h(L_shl(s, exp));
        if ( exp < 16 )
            frac = (Word16)( s>>(16-exp) );
        else
            frac = (Word16)( s<<(exp-16) );

        //exp = sub(15 - 9, exp);
        exp = 6 - exp;

        if (frac <= 0)
        {
            *cod_gain_frac = 0;
            *cod_gain_exp = 0;
        }
        else
        {
            /*
              gcu = <xn2, y2> / c[2]
                  = (frac>>1)/frac[2]             * 2^(exp+1-exp[2])
                  = div_s(frac>>1, frac[2])*2^-15 * 2^(exp+1-exp[2])
                  = div_s * 2^(exp-exp[2]-14)
             */
            //*cod_gain_frac = div_s (shr (frac,1), frac_coeff[2]);
        *cod_gain_frac = div_s ((Word16)(frac>>1), frac_coeff[2]);

            //*cod_gain_exp = sub (sub (exp, exp_coeff[2]), 14);
            *cod_gain_exp = exp - exp_coeff[2] - 14;
        }
    }
}

/*************************************************************************
 *
 * FUNCTION: calc_target_energy
 *
 * PURPOSE:  calculation of target energy
 *
 *      en = <xn, xn>
 *
 *************************************************************************/
void
calc_target_energy(
    Word16 xn[],     /* i: LTP target vector,                       Q0  */
    Word16 *en_exp,  /* o: optimum codebook gain (exponent part),   Q0  */
    Word16 *en_frac  /* o: optimum codebook gain (fraction part),   Q15 */
)
{
    Word32 s;
    Word16 exp;

    /* Compute scalar product <xn[], xn[]> */
#ifdef CALCTARGETENERGY_ENG
    VOLATILE (
    *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) xn;
    *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) xn;
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
                                    (L_SHIFT1   << P_SAdd0) |
                                    (L_SHIFT1   << P_SAdd1) |
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
                                    (L_SUBFR    << P_Len       )|
                                    (0          << P_RdIncr    )|
                                    (0          << P_WrIncr    );
        WaitEngineDone();
        s = *(volatile int *)ALU_P0;
);

#elif defined CALCTARGETENERGY_PUREC
    Word32 i;
    s = L_mac(0L, xn[0], xn[0]);
    for (i = 1; i < L_SUBFR; i++)
        s = L_mac(s, xn[i], xn[i]);
#else
#error "error"
#endif
    /* s = SUM 2*xn(i) * xn(i) = <xn xn> * 2 */
    exp = norm_l(s);

    //*en_frac = extract_h(L_shl(s, exp));
    if ( exp < 16 )
        *en_frac = (Word16)( s>>(16-exp) );
    else
        *en_frac = (Word16)( s<<(exp-16) );

    //*en_exp = sub(16, exp);
    *en_exp = 16 - exp;
}
