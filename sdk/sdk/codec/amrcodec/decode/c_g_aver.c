/*************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : c_g_aver.c
*      Purpose          :
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "c_g_aver.h"
const char c_g_aver_id[] = "@(#)$Id $" c_g_aver_h;

//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
#include "mode.h"
#include "basic_op.h"
//#include "count.h"
#include "cnst.h"
//#include "set_zero.h"
#include "copy.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
/*-----------------------------------------------------------------*
 *   Decoder constant parameters (defined in "cnst.h")             *
 *-----------------------------------------------------------------*
 *   L_FRAME       : Frame size.                                   *
 *   L_SUBFR       : Sub-frame size.                               *
 *-----------------------------------------------------------------*/

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
Cb_gain_averageState Cb_gain_averageS;

/*
**************************************************************************
*
*  Function    : Cb_gain_average_init
*  Purpose     : Allocates and initializes state memory
*
**************************************************************************
*/
void Cb_gain_average_init (Cb_gain_averageState **state)
{
   Cb_gain_averageState* s;

//   if (state == (Cb_gain_averageState **) NULL){
//      fprintf(stderr, "Cb_gain_average_init: invalid parameter\n");
//      return -1;
//   }
//   *state = NULL;

   /* allocate memory */
//   if ((s= (Cb_gain_averageState *) malloc(sizeof(Cb_gain_averageState))) == NULL){
//     fprintf(stderr, "Cb_gain_average_init: can not malloc state structure\n");
//     return -1;
//   }

   s = &Cb_gain_averageS;
   Cb_gain_average_reset(s);
   *state = s;

   return;
}

/*
**************************************************************************
*
*  Function    : Cb_gain_average_reset
*  Purpose     : Resets state memory
*
**************************************************************************
*/
void Cb_gain_average_reset (Cb_gain_averageState *state)
{
//   if (state == (Cb_gain_averageState *) NULL){
//      fprintf(stderr, "Cb_gain_average_reset: invalid parameter\n");
//      return -1;
//   }

   /* Static vectors to zero */
//   Set_zero (state->cbGainHistory, L_CBGAINHIST);
   Preset(0, state->cbGainHistory, L_CBGAINHIST);

   /* Initialize hangover handling */
   state->hangVar = 0;
   state->hangCount= 0;

   return;
}

/*
**************************************************************************
*
*  Function    : Cb_gain_average_exit
*  Purpose     : The memory used for state memory is freed
*
**************************************************************************
*/
//void Cb_gain_average_exit (Cb_gain_averageState **state)
//{
////   if (state == NULL || *state == NULL)
////      return;
//
//   /* deallocate memory */
////   free(*state);
////   *state = NULL;
//
//      return;
//}

/*
**************************************************************************
*
*  Function    : Cb_gain_average
*  Purpose     :
*  Returns     : The mix cb gains for MR475, MR515, MR59, MR67, MR102; gain_code other modes
*
**************************************************************************
*/
Word16 Cb_gain_average (
   Cb_gain_averageState *st, /* i/o : State variables for CB gain avergeing   */
   enum Mode mode,           /* i   : AMR mode                                */
   Word16 gain_code,         /* i   : CB gain                              Q1 */
   Word16 lsp[],             /* i   : The LSP for the current frame       Q15 */
   Word16 lspAver[],         /* i   : The average of LSP for 8 frames     Q15 */
   Word16 bfi,               /* i   : bad frame indication flag               */
   Word16 prev_bf,           /* i   : previous bad frame indication flag      */
   Word16 pdfi,              /* i   : potential degraded bad frame ind flag   */
   Word16 prev_pdf,          /* i   : prev pot. degraded bad frame ind flag   */
   Word16 inBackgroundNoise, /* i   : background noise decision               */
   Word16 voicedHangover     /* i   : # of frames after last voiced frame     */
   )
{
   /*---------------------------------------------------------*
    * Compute mixed cb gain, used to make cb gain more        *
    * smooth in background noise for modes 5.15, 5.9 and 6.7  *
    * states that needs to be updated by all                  *
    *---------------------------------------------------------*/
   Word32 i;
   //Word16 cbGainMix, diff, tmp_diff, bgMix, cbGainMean;
   Word16 cbGainMix, bgMix, cbGainMean;
   Word32 L_sum;
   //Word16 tmp[M], tmp1, tmp2, shift1, shift2, shift;
   Word16 tmp1, tmp2, shift1, shift2, shift;

   Word32 diff, tmp_diff, tmp0;

   /* set correct cbGainMix for MR74, MR795, MR122 */
   cbGainMix = gain_code;

   /*-------------------------------------------------------*
    *   Store list of CB gain needed in the CB gain         *
    *   averaging                                           *
    *-------------------------------------------------------*/
   //for (i = 0; i < (L_CBGAINHIST-1); i++)
   //{
   //   st->cbGainHistory[i] = st->cbGainHistory[i+1];
   //}
   //st->cbGainHistory[L_CBGAINHIST-1] = gain_code;
   st->cbGainHistory[0] = st->cbGainHistory[1];
   st->cbGainHistory[1] = st->cbGainHistory[2];
   st->cbGainHistory[2] = st->cbGainHistory[3];
   st->cbGainHistory[3] = st->cbGainHistory[4];
   st->cbGainHistory[4] = st->cbGainHistory[5];
   st->cbGainHistory[5] = st->cbGainHistory[6];
   st->cbGainHistory[6] = gain_code;

   /* compute lsp difference */
   diff = 0;
   for (i = 0; i < M; i++) {
      //tmp1 = abs_s(sub(lspAver[i], lsp[i]));  /* Q15       */
      tmp1 = lspAver[i] - lsp[i];
      if( tmp1 < 0 )
         tmp1 = - tmp1;
      //shift1 = sub(norm_s(tmp1), 1);          /* Qn        */
      shift1 = norm_s(tmp1) - 1;                /* Qn        */
      //tmp1 = shl(tmp1, shift1);                 /* Q15+Qn    */
      tmp1 = tmp1 << shift1;                 /* Q15+Qn    */
      shift2 = norm_s(lspAver[i]);              /* Qm        */
      //tmp2 = shl(lspAver[i], shift2);           /* Q15+Qm    */
      tmp2 = lspAver[i] << shift2;           /* Q15+Qm    */
      //shift = sub(add(2, shift1), shift2);
      shift = 2 + shift1 - shift2;
      //tmp[i] = div_s(tmp1, tmp2);               /* Q15+(Q15+Qn)-(Q15+Qm) */
      tmp0 = div_s(tmp1, tmp2);               /* Q15+(Q15+Qn)-(Q15+Qm) */

      if (shift >= 0)
      {
         //tmp[i] = shr(tmp[i], shift);         /* Q15+Qn-Qm-Qx=Q13 */
         tmp0 >>= shift;                      /* Q15+Qn-Qm-Qx=Q13 */
      }
      else
      {
         //tmp[i] = shl(tmp[i], negate(shift)); /* Q15+Qn-Qm-Qx=Q13 */
         tmp0 <<= (-shift);                 /* Q15+Qn-Qm-Qx=Q13 */
      }
      diff = diff + tmp0;
   }

//   diff = tmp[0];
//   for (i = 1; i < M; i++)
//   {
//      // sis3830 add overflow
//      //diff = add(diff, tmp[i]);       /* Q13 */
//     diff = diff + tmp[i];
//   }

   /* Compute hangover */
   if (diff > 5325)  /* 0.65 in Q11 */
   {
      st->hangVar++;
   }
   else
   {
      st->hangVar = 0;
   }

   if (st->hangVar > 10)
   {
      st->hangCount = 0;  /* Speech period, reset hangover variable */
   }

   /* Compute mix constant (bgMix) */
   //bgMix = 8192;    /* 1 in Q13 */

   if (((Word16)mode <= MR67) || ((Word16)mode == MR102))
      /* MR475, MR515, MR59, MR67, MR102 */
   {
      /* if errors and presumed noise make smoothing probability stronger */
      if (((((pdfi != 0) && (prev_pdf != 0)) || (bfi != 0) || (prev_bf != 0)) &&
          (voicedHangover > 1) && (inBackgroundNoise != 0) &&
          (((Word16)mode == MR475) ||
           ((Word16)mode == MR515) ||
           ((Word16)mode == MR59)) ))
      {
         /* bgMix = min(0.25, max(0.0, diff-0.55)) / 0.25; */
         tmp_diff = diff - 4506;   /* 0.55 in Q13 */
      }
      else
      {
         /* bgMix = min(0.25, max(0.0, diff-0.40)) / 0.25; */
         tmp_diff = diff - 3277; /* 0.4 in Q13 */
      }

      /* max(0.0, diff-0.55) */
      if (tmp_diff > 0)
      {
        tmp1 = tmp_diff;
      }
      else
      {
         tmp1 = 0;
      }

      /* min(0.25, tmp1) */
      if ( (2048 < tmp1) ||
          (st->hangCount < 40) || (diff > 5325)
          )
      {
        bgMix = 8192;
      }
      else
      {
        bgMix = tmp1 << 2;
      }

//      if ((st->hangCount < 40) || (diff > 5325)) /* 0.65 in Q13 */
//      {
//         bgMix = 8192;  /* disable mix if too short time since */
//      }

      /* Smoothen the cb gain trajectory  */
      /* smoothing depends on mix constant bgMix */
      // sis3830 L_mult L_mac overflow
      //L_sum = L_mult(6554, st->cbGainHistory[2]);     /* 0.2 in Q15; L_sum in Q17 */

      // L_CBGAINHIST = 7
      L_sum = 0;
#ifdef OPRISCASM
      asm ("l.mac %0, %1" : : "r"((Word32)6554), "r"((Word32)st->cbGainHistory[2]));
      asm ("l.mac %0, %1" : : "r"((Word32)6554), "r"((Word32)st->cbGainHistory[3]));
      asm ("l.mac %0, %1" : : "r"((Word32)6554), "r"((Word32)st->cbGainHistory[4]));
      asm ("l.mac %0, %1" : : "r"((Word32)6554), "r"((Word32)st->cbGainHistory[5]));
      asm ("l.mac %0, %1" : : "r"((Word32)6554), "r"((Word32)st->cbGainHistory[6]));
      asm ("l.mac %0, %1" : : "r"((Word32)6554), "r"((Word32)st->cbGainHistory[7]));
      asm ("l.mac %0, %1" : : "r"((Word32)1), "r"((Word32)0x4000));
      asm volatile("l.macrc %0, 15" : "=r" (cbGainMean));
#else
      for (i = 2; i < L_CBGAINHIST; i++)
      {
         //L_sum = L_mac(L_sum, 6554, st->cbGainHistory[i]);
          L_sum += st->cbGainHistory[i];
      }
      L_sum = L_sum * 6554;
      // sis3830 Q round
      cbGainMean = (Word16)((L_sum+0x00004000L)>>15);
      //cbGainMean = round16(L_sum);                      /* Q1 */
#endif

      /* more smoothing in error and bg noise (NB no DFI used  here) */
      if (((bfi != 0) || (prev_bf != 0)) && (inBackgroundNoise != 0) &&
          (((Word16)mode == MR475) ||
           ((Word16)mode == MR515) ||
           ((Word16)mode == MR59)) )
      {
         //L_sum = L_mult(4681, st->cbGainHistory[0]);  /* 0.143 in Q15; L_sum in Q17 */
         //for (i = 1; i < L_CBGAINHIST; i++)
         //{
         //   L_sum = L_mac(L_sum, 4681, st->cbGainHistory[i]);
         //}
         L_sum = 0L;  /* 0.143 in Q15; L_sum in Q17 */
         for (i = 0; i < L_CBGAINHIST; i++)
         {
            L_sum += st->cbGainHistory[i];
         }
         L_sum *= 4681;
         //cbGainMean = round16(L_sum);                   /* Q1 */
         cbGainMean = (Word16)((L_sum+0x00004000L)>>15);                   /* Q1 */
      }

      /* cbGainMix = bgMix*cbGainMix + (1-bgMix)*cbGainMean; */
      L_sum = (bgMix * cbGainMix);               /* L_sum in Q15 */
      //L_sum += (8192 * cbGainMean);
      //L_sum -= (bgMix * cbGainMean);
      L_sum += ((8192-bgMix) * cbGainMean);

      cbGainMix = (Word16)((L_sum+0x00001000L)>>13);             /* Q1 */
   }

   st->hangCount ++;
   return cbGainMix;
}
