/* OK
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
*****************************************************************************
*
*      File             : gc_pred.c
*      Purpose          : codebook gain MA prediction
*
*****************************************************************************
*/

/*
*****************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
*****************************************************************************
*/
#include "gc_pred.h"
const char gc_pred_id[] = "@(#)$Id $" gc_pred_h;

/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/
//#include <stdio.h>
//#include <stdlib.h>
#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "cnst.h"
//#include "count.h"
#include "log2.h"
#include "copy.h"

/*
*****************************************************************************
*                         LOCAL VARIABLES AND TABLES
*****************************************************************************
*/
gc_predState en_gc_predS;
gc_predState en_gc_predUnqS;
gc_predState de_gc_predS;
//gc_predState de_gc_predUnqS;

#define NPRED 4  /* number of prediction taps */

/* MA prediction coefficients (Q13) */
static const Word16 pred[NPRED] = {5571, 4751, 2785, 1556};

/* average innovation energy.                               */
/* MEAN_ENER  = 36.0/constant, constant = 20*Log10(2)       */
#define MEAN_ENER_MR122  783741L  /* 36/(20*log10(2)) (Q17) */

/* MA prediction coefficients (Q6)  */
static const Word16 pred_MR122[NPRED] = {44, 37, 22, 12};

/* minimum quantized energy: -14 dB */
#define MIN_ENERGY       -14336       /* 14                 Q10 */
#define MIN_ENERGY_MR122  -2381       /* 14 / (20*log10(2)) Q10 */
/*
*****************************************************************************
*                         PUBLIC PROGRAM CODE
*****************************************************************************
*/
/*************************************************************************
*
*  Function:   qua_gain_init
*  Purpose:    Allocates state memory and initializes state memory
*
**************************************************************************
*/
int gc_pred_init (gc_predState **state, Word32 isdecode)
{
    gc_predState* s;

//  if (state == (gc_predState **) NULL){
//      fprintf(stderr, "gc_pred_init: invalid parameter\n");
//      return -1;
//  }
//  *state = NULL;

  /* allocate memory */
//  if ((s= (gc_predState *) malloc(sizeof(gc_predState))) == NULL){
//      fprintf(stderr, "gc_pred_init: can not malloc state structure\n");
//      return -1;
//  }

    if (isdecode) {
        s = &de_gc_predS;
    } else {
        s = &en_gc_predS;
    }

    gc_pred_reset(s);
    *state = s;

    return 0;
}

int gc_predUnq_init (gc_predState **state, Word32 isdecode)
{
    gc_predState* s;

//  if (state == (gc_predState **) NULL){
//      fprintf(stderr, "gc_pred_init: invalid parameter\n");
//      return -1;
//  }
//  *state = NULL;

  /* allocate memory */
//  if ((s= (gc_predState *) malloc(sizeof(gc_predState))) == NULL){
//      fprintf(stderr, "gc_pred_init: can not malloc state structure\n");
//      return -1;
//  }

    s = &en_gc_predUnqS;
/*
    if (isdecode) {
        s = &de_gc_predUnqS;
    } else {
        s = &en_gc_predUnqS;
    }
*/
    gc_pred_reset(s);
    *state = s;

  return 0;
}
/*************************************************************************
*
*  Function:   gc_pred_reset
*  Purpose:    Initializes state memory to zero
*
**************************************************************************
*/
int gc_pred_reset (gc_predState *state)
{
   //Word32 i;

//   if (state == (gc_predState *) NULL){
//      fprintf(stderr, "gc_pred_reset: invalid parameter\n");
//      return -1;
//   }

//   for(i = 0; i < NPRED; i++)
//   {
//      state->past_qua_en[i] = MIN_ENERGY;
//      state->past_qua_en_MR122[i] = MIN_ENERGY_MR122;
//   }
   state->past_qua_en[0]=MIN_ENERGY;
   state->past_qua_en[1]=MIN_ENERGY;
   state->past_qua_en[2]=MIN_ENERGY;
   state->past_qua_en[3]=MIN_ENERGY;

   state->past_qua_en_MR122[0] = MIN_ENERGY_MR122;
   state->past_qua_en_MR122[1] = MIN_ENERGY_MR122;
   state->past_qua_en_MR122[2] = MIN_ENERGY_MR122;
   state->past_qua_en_MR122[3] = MIN_ENERGY_MR122;

  return 0;
}

/*************************************************************************
*
*  Function:   gc_pred_exit
*  Purpose:    The memory used for state memory is freed
*
**************************************************************************
*/
//void gc_pred_exit (gc_predState **state)
//{
//  if (state == NULL || *state == NULL)
//      return;

  /* deallocate memory */
//  free(*state);
//  *state = NULL;

//  return;
//}

/*************************************************************************
 *
 * FUNCTION:  gc_pred_copy()
 *
 * PURPOSE: Copy MA predictor state variable
 *
 *************************************************************************/
//void
//gc_pred_copy(
//    gc_predState *st_src,  /* i : State struct                           */
//    gc_predState *st_dest  /* o : State struct                           */
//)
//{
//    Copy (st_src->past_qua_en, st_dest->past_qua_en, NPRED);
//    Copy (st_src->past_qua_en_MR122, st_dest->past_qua_en_MR122, NPRED);
//}

/*************************************************************************
 *
 * FUNCTION:  gc_pred()
 *
 * PURPOSE: MA prediction of the innovation energy
 *          (in dB/(20*log10(2))) with mean  removed).
 *
 *************************************************************************/
void
gc_pred(
    gc_predState *st,   /* i/o: State struct                           */
    enum Mode mode,     /* i  : AMR mode                               */
    Word16 *code,       /* i  : innovative codebook vector (L_SUBFR)   */
                        /*      MR122: Q12, other modes: Q13           */
    Word16 *exp_gcode0, /* o  : exponent of predicted gain factor, Q0  */
    Word16 *frac_gcode0,/* o  : fraction of predicted gain factor  Q15 */
    Word16 *exp_en,     /* o  : exponent of innovation energy,     Q0  */
                        /*      (only calculated for MR795)            */
    Word16 *frac_en     /* o  : fraction of innovation energy,     Q15 */
                        /*      (only calculated for MR795)            */
)
{
    Word32 i;
    Word32 ener_code;
    Word16 exp, frac;
    Word32 ener;
    Word32 L_tmp;
    Word16 exp_code, gcode0;

    /*-------------------------------------------------------------------*
     *  energy of code:                                                  *
     *  ~~~~~~~~~~~~~~~                                                  *
     *  ener_code = sum(code[i]^2)                                       *
     *-------------------------------------------------------------------*/
//    ener_code = L_mac((Word32) 0, code[0], code[0]);
#ifdef GCPRED_ENG
VOLATILE (
        *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) code;
        *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) code;
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
        ener_code = *(volatile int *)ALU_P0;
);
#elif defined GCPRED_PUREC

    ener_code = 0;
    for (i = 0; i < L_SUBFR; i++)
    {
        ener_code += (code[i] * code[i]);
        if ( ener_code > (Word32)0x3fffffffL )
            ener_code = 0x3fffffffL;
    }
    ener_code <<= 1;
#else
#error "error"
#endif
//  ener_code = L_smac(ener_code, code, code, L_SUBFR, 0);      // add by powei
    if ((Word16)mode == MR122)
    {

        /* ener_code = ener_code / lcode; lcode = 40; 1/40 = 26214 Q20       */
//      ener_code = L_mult (round (ener_code), 26214);   /* Q9  * Q20 -> Q30 */
        i = (Word16)( (ener_code+0x00008000L)>>16 );
        ener_code = (Word32)(i * 26214);
        ener_code <<= 1;

        /*-------------------------------------------------------------------*
         *  energy of code:                                                  *
         *  ~~~~~~~~~~~~~~~                                                  *
         *  ener_code(Q17) = 10 * Log10(energy) / constant                   *
         *                 = 1/2 * Log2(energy)                              *
         *                                           constant = 20*Log10(2)  *
         *-------------------------------------------------------------------*/
        /* ener_code = 1/2 * Log2(ener_code); Note: Log2=log2+30 */
        Log2(ener_code, &exp, &frac);
//        ener_code = L_Comp (sub (exp, 30), frac);     /* Q16 for log()    */
                                                    /* ->Q17 for 1/2 log()*/
        ener_code = (Word32)( (exp-30)<<15 ) + frac;
        ener_code <<= 1;
        /*-------------------------------------------------------------------*
         *  predicted energy:                                                *
         *  ~~~~~~~~~~~~~~~~~                                                *
         *  ener(Q24) = (Emean + sum{pred[i]*past_en[i]})/constant           *
         *            = MEAN_ENER + sum(pred[i]*past_qua_en[i])              *
         *                                           constant = 20*Log10(2)  *
         *-------------------------------------------------------------------*/

        ener = MEAN_ENER_MR122>>1;                  /* Q24 (Q17) */
        for (i = 0; i < NPRED; i++)
        {
            ener += (st->past_qua_en_MR122[i] * pred_MR122[i]);
            if ( ener > (Word32)0x3fffffffL )
                ener = 0x3fffffffL;

        }
        ener <<= 1;
//      ener = L_smac ((Word32)MEAN_ENER_MR122, &st->past_qua_en_MR122[0],
//                      &pred_MR122[0], (Word16)NPRED, 0);

        /*-------------------------------------------------------------------*
         *  predicted codebook gain                                          *
         *  ~~~~~~~~~~~~~~~~~~~~~~~                                          *
         *  gc0     = Pow10( (ener*constant - ener_code*constant) / 20 )     *
         *          = Pow2(ener-ener_code)                                   *
         *          = Pow2(int(d)+frac(d))                                   *
         *                                                                   *
         *  (store exp and frac for pow2())                                  *
         *-------------------------------------------------------------------*/

        ener = (ener - ener_code) >> 2;                /* Q16 */
//        L_Extract(ener, exp_gcode0, frac_gcode0);
        *exp_gcode0 = (Word16) (ener >> 15);
        *frac_gcode0 = (Word16) (ener - (Word32)(*exp_gcode0<<15));

    }
    else /* all modes except 12.2 */
    {

        /*-----------------------------------------------------------------*
         *  Compute: means_ener - 10log10(ener_code/ L_sufr)               *
         *-----------------------------------------------------------------*/

        exp_code = norm_l (ener_code);
        ener_code <<= exp_code;

        /* Log2 = log2 + 27 */
        Log2_norm (ener_code, exp_code, &exp, &frac);

        /* fact = 10/log2(10) = 3.01 = 24660 Q13 */
//        L_tmp = Mpy_32_16(exp, frac, -24660); /* Q0.Q15 * Q13 -> Q14 */
        L_tmp = (Word32)exp*(-24660)  + ((Word32)(frac * (-24660))>>15);
        /*   L_tmp = means_ener - 10log10(ener_code/L_SUBFR)
         *         = means_ener - 10log10(ener_code) + 10log10(L_SUBFR)
         *         = K - fact * Log2(ener_code)
         *         = K - fact * log2(ener_code) - fact*27
         *
         *   ==> K = means_ener + fact*27 + 10log10(L_SUBFR)
         *
         *   means_ener =       33    =  540672    Q14  (MR475, MR515, MR59)
         *   means_ener =       28.75 =  471040    Q14  (MR67)
         *   means_ener =       30    =  491520    Q14  (MR74)
         *   means_ener =       36    =  589824    Q14  (MR795)
         *   means_ener =       33    =  540672    Q14  (MR102)
         *   10log10(L_SUBFR) = 16.02 =  262481.51 Q14
         *   fact * 27                = 1331640    Q14
         *   -----------------------------------------
         *   (MR475, MR515, MR59)   K = 2134793.51 Q14 ~= 16678 * 64 * 2
         *   (MR67)                 K = 2065161.51 Q14 ~= 32268 * 32 * 2
         *   (MR74)                 K = 2085641.51 Q14 ~= 32588 * 32 * 2
         *   (MR795)                K = 2183945.51 Q14 ~= 17062 * 64 * 2
         *   (MR102)                K = 2134793.51 Q14 ~= 16678 * 64 * 2
         */

        if ((Word16)mode == MR102)
        {
            /* mean = 33 dB */
//            L_tmp = L_mac(L_tmp, 16678, 64);     /* Q14 */
            L_tmp += 1067392;
        }
        else if ((Word16)mode == MR795)
        {
            /* ener_code  = <xn xn> * 2^27*2^exp_code
               frac_en    = ener_code / 2^16
                          = <xn xn> * 2^11*2^exp_code
               <xn xn>    = <xn xn>*2^11*2^exp * 2^exp_en
                         := frac_en            * 2^exp_en

               ==> exp_en = -11-exp_code;
             */
//            *frac_en = extract_h (ener_code);
            *frac_en = (Word16)( ener_code>>16 );
            *exp_en = -11 - exp_code;

            /* mean = 36 dB */
//            L_tmp = L_mac(L_tmp, 17062, 64);     /* Q14 */
            L_tmp += 1091968;
        }
        else if ((Word16)mode == MR74)
        {
            /* mean = 30 dB */
//            L_tmp = L_mac(L_tmp, 32588, 32);     /* Q14 */
            L_tmp += 1042816;
        }
        else if ((Word16)mode == MR67)
        {
            /* mean = 28.75 dB */
//            L_tmp = L_mac(L_tmp, 32268, 32);     /* Q14 */
            L_tmp += 1032576;
        }
        else /* MR59, MR515, MR475 */
        {
            /* mean = 33 dB */
//            L_tmp = L_mac(L_tmp, 16678, 64);     /* Q14 */
            L_tmp += 1067392;
        }
        /*-----------------------------------------------------------------*
         * Compute gcode0.                                                 *
         *  = Sum(i=0,3) pred[i]*past_qua_en[i] - ener_code + mean_ener    *
         *-----------------------------------------------------------------*/

//        L_tmp = L_shl(L_tmp, 10);                /* Q24 */
        L_tmp <<= 10;
        for (i = 0; i < 4; i++)
            L_tmp += (pred[i] * st->past_qua_en[i]);
                                                 /* Q13 * Q10 -> Q24 */
//      L_tmp = L_smac(L_tmp, pred, st->past_qua_en, 4, 0);
//        gcode0 = extract_h(L_tmp);               /* Q8  */
        gcode0 = (Word16)( L_tmp>>15 );

        /*-----------------------------------------------------------------*
         * gcode0 = pow(10.0, gcode0/20)                                   *
         *        = pow(2, 3.3219*gcode0/20)                               *
         *        = pow(2, 0.166*gcode0)                                   *
         *-----------------------------------------------------------------*/

        /* 5439 Q15 = 0.165985                                        */
        /* (correct: 1/(20*log10(2)) 0.166096 = 5443 Q15)             */
        if ((Word16)mode == MR74) /* For IS641 bitexactness */
            L_tmp = (Word32)gcode0 * 5439;  /* Q8 * Q15 -> Q24 */
        else
            L_tmp = (Word32)gcode0 * 5443;  /* Q8 * Q15 -> Q24 */

        L_tmp >>= 8;                   /*          -> Q16 */
//        L_Extract(L_tmp, exp_gcode0, frac_gcode0); /*       -> Q0.Q15 */
        *exp_gcode0 = (Word16) (L_tmp >> 15);
        *frac_gcode0 = (Word16) (L_tmp - (Word32)(*exp_gcode0<<15));

    }
}

void
gc_pred_de(
    gc_predState *st,   /* i/o: State struct                           */
    enum Mode mode,     /* i  : AMR mode                               */
    Word16 *code,       /* i  : innovative codebook vector (L_SUBFR)   */
                        /*      MR122: Q12, other modes: Q13           */
    Word16 *exp_gcode0, /* o  : exponent of predicted gain factor, Q0  */
    Word16 *frac_gcode0,/* o  : fraction of predicted gain factor  Q15 */
    Word16 *exp_en,     /* o  : exponent of innovation energy,     Q0  */
                        /*      (only calculated for MR795)            */
    Word16 *frac_en     /* o  : fraction of innovation energy,     Q15 */
                        /*      (only calculated for MR795)            */
)
{
    Word32 i;
    Word32 ener_code;
    Word16 exp, frac;
    Word32 ener;
    Word32 L_tmp;
    Word16 exp_code, gcode0;

#   if !defined(HAVE_MACLC) || !defined(OPRISCASM)
    Word32 mac_hi, mac_lo;
#   endif

    /*-------------------------------------------------------------------*
     *  energy of code:                                                  *
     *  ~~~~~~~~~~~~~~~                                                  *
     *  ener_code = sum(code[i]^2)                                       *
     *-------------------------------------------------------------------*/
//    ener_code = L_mac((Word32) 0, code[0], code[0]);
/*  ener_code = 0;
    for (i = 0; i < L_SUBFR; i++)
    {
        ener_code += (code[i] * code[i]);
        if ( ener_code > (Word32)0x3fffffffL )
            ener_code = 0x3fffffffL;
    }
    ener_code <<= 1;
*/

#ifdef OPRISCASM

    //asm volatile("l.macrc r0");
    //asm volatile("l.mtspr r0, r0, 0x2801");
    //asm volatile("l.mtspr r0, r0, 0x2802");
    for (i=0; i < L_SUBFR; i++)
    {
        asm ("l.mac %0, %1" : : "r"((Word32)code[i]), "r"((Word32)code[i]));
    }
#   ifdef HAVE_MACLC
    asm ("l.maclc %0, 1" : "=r" (ener_code));
#   else
    asm volatile("l.mfspr %0, r0, 0x2802" : "=r"(mac_hi));
    asm volatile("l.macrc %0, 0" : "=r" (mac_lo));
#   endif

#else
    Word64 temp;
    temp = 0;
    for (i=0; i < L_SUBFR; i++)
    {
        temp = temp + code[i] * code[i];
    }
    mac_hi = (Word32) (temp >> 32);
    mac_lo = (Word32) (temp & 0xFFFFFFFF);

#endif

#if !defined(OPRISCASM) || (defined(OPRISCASM) && !defined(HAVE_MACLC))
    _CheckOverflow_P(mac_hi, mac_lo, ener_code);
#endif

/*
    if(mac_hi > 0 || mac_lo < 0 || mac_lo > 1073741823) {
        ener_code = MAX_32;  // overflow
    } else {
        ener_code = (mac_lo<<1);
    }
*/

//    ener_code = L_smac(ener_code, code, code, L_SUBFR, 0);      // add by powei
    if ((Word16)mode == MR122)
    {

        /* ener_code = ener_code / lcode; lcode = 40; 1/40 = 26214 Q20       */
//        ener_code = L_mult (round16(ener_code), 26214);   /* Q9  * Q20 -> Q30 */
        ener_code = (Word16)( (ener_code+0x00008000L)>>16 );
        ener_code = (Word32)(ener_code * 26214);
        ener_code <<= 1;

        /*-------------------------------------------------------------------*
         *  energy of code:                                                  *
         *  ~~~~~~~~~~~~~~~                                                  *
         *  ener_code(Q17) = 10 * Log10(energy) / constant                   *
         *                 = 1/2 * Log2(energy)                              *
         *                                           constant = 20*Log10(2)  *
         *-------------------------------------------------------------------*/
        /* ener_code = 1/2 * Log2(ener_code); Note: Log2=log2+30 */
        Log2(ener_code, &exp, &frac);
//        ener_code = L_Comp (sub (exp, 30), frac);     /* Q16 for log()    */
                                                    /* ->Q17 for 1/2 log()*/
        ener_code = (Word32)( (exp-30)<<15 ) + frac;
        ener_code <<= 1;
        /*-------------------------------------------------------------------*
         *  predicted energy:                                                *
         *  ~~~~~~~~~~~~~~~~~                                                *
         *  ener(Q24) = (Emean + sum{pred[i]*past_en[i]})/constant           *
         *            = MEAN_ENER + sum(pred[i]*past_qua_en[i])              *
         *                                           constant = 20*Log10(2)  *
         *-------------------------------------------------------------------*/

        ener = MEAN_ENER_MR122>>1;                  /* Q24 (Q17) */
        for (i = 0; i < NPRED; i++)
        {
            ener += (st->past_qua_en_MR122[i] * pred_MR122[i]);
        //          if ( ener > (Word32)0x3fffffffL )
        //      ener = 0x3fffffffL;
        }
        ener <<= 1;
//      ener = L_smac ((Word32)MEAN_ENER_MR122, &st->past_qua_en_MR122[0],
//                      &pred_MR122[0], (Word16)NPRED, 0);

        /*-------------------------------------------------------------------*
         *  predicted codebook gain                                          *
         *  ~~~~~~~~~~~~~~~~~~~~~~~                                          *
         *  gc0     = Pow10( (ener*constant - ener_code*constant) / 20 )     *
         *          = Pow2(ener-ener_code)                                   *
         *          = Pow2(int(d)+frac(d))                                   *
         *                                                                   *
         *  (store exp and frac for pow2())                                  *
         *-------------------------------------------------------------------*/

        ener = (ener - ener_code) >> 2;                /* Q16 */
//        L_Extract(ener, exp_gcode0, frac_gcode0);
        *exp_gcode0 = (Word16) (ener >> 15);
        *frac_gcode0 = (Word16) (ener - (Word32)(*exp_gcode0<<15));

    }
    else /* all modes except 12.2 */
    {

        /*-----------------------------------------------------------------*
         *  Compute: means_ener - 10log10(ener_code/ L_sufr)               *
         *-----------------------------------------------------------------*/

        exp_code = norm_l (ener_code);
        ener_code <<= exp_code;

        /* Log2 = log2 + 27 */
        Log2_norm (ener_code, exp_code, &exp, &frac);

        /* fact = 10/log2(10) = 3.01 = 24660 Q13 */
//        L_tmp = Mpy_32_16(exp, frac, -24660); /* Q0.Q15 * Q13 -> Q14 */
        L_tmp = (Word32)exp*(-24660)  + ((Word32)(frac * (-24660))>>15);
        /*   L_tmp = means_ener - 10log10(ener_code/L_SUBFR)
         *         = means_ener - 10log10(ener_code) + 10log10(L_SUBFR)
         *         = K - fact * Log2(ener_code)
         *         = K - fact * log2(ener_code) - fact*27
         *
         *   ==> K = means_ener + fact*27 + 10log10(L_SUBFR)
         *
         *   means_ener =       33    =  540672    Q14  (MR475, MR515, MR59)
         *   means_ener =       28.75 =  471040    Q14  (MR67)
         *   means_ener =       30    =  491520    Q14  (MR74)
         *   means_ener =       36    =  589824    Q14  (MR795)
         *   means_ener =       33    =  540672    Q14  (MR102)
         *   10log10(L_SUBFR) = 16.02 =  262481.51 Q14
         *   fact * 27                = 1331640    Q14
         *   -----------------------------------------
         *   (MR475, MR515, MR59)   K = 2134793.51 Q14 ~= 16678 * 64 * 2
         *   (MR67)                 K = 2065161.51 Q14 ~= 32268 * 32 * 2
         *   (MR74)                 K = 2085641.51 Q14 ~= 32588 * 32 * 2
         *   (MR795)                K = 2183945.51 Q14 ~= 17062 * 64 * 2
         *   (MR102)                K = 2134793.51 Q14 ~= 16678 * 64 * 2
         */

        if ((Word16)mode == MR102)
        {
            /* mean = 33 dB */
//            L_tmp = L_mac(L_tmp, 16678, 64);     /* Q14 */
            L_tmp += 1067392;
        }
        else if ((Word16)mode == MR795)
        {
            /* ener_code  = <xn xn> * 2^27*2^exp_code
               frac_en    = ener_code / 2^16
                          = <xn xn> * 2^11*2^exp_code
               <xn xn>    = <xn xn>*2^11*2^exp * 2^exp_en
                         := frac_en            * 2^exp_en

               ==> exp_en = -11-exp_code;
             */
//            *frac_en = extract_h (ener_code);
            *frac_en = (Word16)( ener_code>>16 );
            *exp_en = -11 - exp_code;

            /* mean = 36 dB */
//            L_tmp = L_mac(L_tmp, 17062, 64);     /* Q14 */
            L_tmp += 1091968;
        }
        else if ((Word16)mode == MR74)
        {
            /* mean = 30 dB */
//            L_tmp = L_mac(L_tmp, 32588, 32);     /* Q14 */
            L_tmp += 1042816;
        }
        else if ((Word16)mode == MR67)
        {
            /* mean = 28.75 dB */
//            L_tmp = L_mac(L_tmp, 32268, 32);     /* Q14 */
            L_tmp += 1032576;
        }
        else /* MR59, MR515, MR475 */
        {
            /* mean = 33 dB */
//            L_tmp = L_mac(L_tmp, 16678, 64);     /* Q14 */
            L_tmp += 1067392;
        }
        /*-----------------------------------------------------------------*
         * Compute gcode0.                                                 *
         *  = Sum(i=0,3) pred[i]*past_qua_en[i] - ener_code + mean_ener    *
         *-----------------------------------------------------------------*/

//        L_tmp = L_shl(L_tmp, 10);                /* Q24 */
        L_tmp <<= 10;
        for (i = 0; i < 4; i++)
            L_tmp += (pred[i] * st->past_qua_en[i]);
                                                 /* Q13 * Q10 -> Q24 */
//      L_tmp = L_smac(L_tmp, pred, st->past_qua_en, 4, 0);
//        gcode0 = extract_h(L_tmp);               /* Q8  */
        gcode0 = (Word16)( L_tmp>>15 );

        /*-----------------------------------------------------------------*
         * gcode0 = pow(10.0, gcode0/20)                                   *
         *        = pow(2, 3.3219*gcode0/20)                               *
         *        = pow(2, 0.166*gcode0)                                   *
         *-----------------------------------------------------------------*/

        /* 5439 Q15 = 0.165985                                        */
        /* (correct: 1/(20*log10(2)) 0.166096 = 5443 Q15)             */
        if ((Word16)mode == MR74) /* For IS641 bitexactness */
            L_tmp = (Word32)gcode0 * 5439;  /* Q8 * Q15 -> Q24 */
        else
            L_tmp = (Word32)gcode0 * 5443;  /* Q8 * Q15 -> Q24 */

        L_tmp >>= 8;                   /*          -> Q16 */
//        L_Extract(L_tmp, exp_gcode0, frac_gcode0); /*       -> Q0.Q15 */
        *exp_gcode0 = (Word16) (L_tmp >> 15);
        *frac_gcode0 = (Word16) (L_tmp - (Word32)(*exp_gcode0<<15));

    }
}

/*************************************************************************
 *
 * FUNCTION:  gc_pred_update()
 *
 * PURPOSE: update MA predictor with last quantized energy
 *
 *************************************************************************/
void gc_pred_update(
    gc_predState *st,      /* i/o: State struct                     */
    Word16 qua_ener_MR122, /* i  : quantized energy for update, Q10 */
                           /*      (log2(qua_err))                  */
    Word16 qua_ener        /* i  : quantized energy for update, Q10 */
                           /*      (20*log10(qua_err))              */
)
{
    //Word32 i;

    //for (i = 3; i > 0; i--)
    //{
    //    st->past_qua_en[i] = st->past_qua_en[i - 1];
    //    st->past_qua_en_MR122[i] = st->past_qua_en_MR122[i - 1];
    //}
    st->past_qua_en[3] = st->past_qua_en[2];
    st->past_qua_en[2] = st->past_qua_en[1];
    st->past_qua_en[1] = st->past_qua_en[0];
    st->past_qua_en[0] = qua_ener;

    st->past_qua_en_MR122[3] = st->past_qua_en_MR122[2];
    st->past_qua_en_MR122[2] = st->past_qua_en_MR122[1];
    st->past_qua_en_MR122[1] = st->past_qua_en_MR122[0];
    st->past_qua_en_MR122[0] = qua_ener_MR122;  /*    log2 (qua_err), Q10 */

                  /* 20*log10(qua_err), Q10 */
}
/*************************************************************************
 *
 * FUNCTION:  gc_pred_average_limited()
 *
 * PURPOSE: get average of MA predictor state values (with a lower limit)
 *          [used in error concealment]
 *
 *************************************************************************/
void gc_pred_average_limited(
    gc_predState *st,       /* i: State struct                    */
    Word16 *ener_avg_MR122, /* o: everaged quantized energy,  Q10 */
                            /*    (log2(qua_err))                 */
    Word16 *ener_avg        /* o: averaged quantized energy,  Q10 */
                            /*    (20*log10(qua_err))             */
)
{
    Word16 av_pred_en;
    Word32 i;

    /* do average in MR122 mode (log2() domain) */
    av_pred_en = 0;
    i = 0;
    do
    {
        av_pred_en += st->past_qua_en_MR122[i];
        i++;
    } while (i < NPRED);

    /* av_pred_en = 0.25*av_pred_en */
    av_pred_en = (Word16)((av_pred_en * 8192)>>15);

    /* if (av_pred_en < -14/(20Log10(2))) av_pred_en = .. */
    if (av_pred_en < MIN_ENERGY_MR122)
    {
        av_pred_en = MIN_ENERGY_MR122;
    }
    *ener_avg_MR122 = av_pred_en;

    /* do average for other modes (20*log10() domain) */
    av_pred_en = 0;
    i = 0;
    do
    {
        av_pred_en += st->past_qua_en[i];
        i++;
    } while (i < NPRED);

    /* av_pred_en = 0.25*av_pred_en */
    av_pred_en = (Word16)((av_pred_en * 8192)>>15);

    /* if (av_pred_en < -14) av_pred_en = .. */
    if (av_pred_en < MIN_ENERGY)
    {
        av_pred_en = MIN_ENERGY;
    }
    *ener_avg = av_pred_en;
}
