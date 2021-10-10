/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
*****************************************************************************
*
*      File             : dtx_dec.c
*      Purpose          : Decode comfort noise when in DTX
*
*****************************************************************************
*/
/*
*****************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
*****************************************************************************
*/
#include "dtx_dec.h"
const char dtx_dec_id[] = "@(#)$Id $" dtx_dec_h;

/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "copy.h"
//#include "set_zero.h"
#include "mode.h"
#include "log2.h"
#include "lsp_az.h"
#include "pow2.h"
#include "a_refl.h"
#include "b_cn_cod.h"
#include "syn_filt.h"
#include "lsp_lsf.h"
#include "reorder.h"
//#include "count.h"
#include "q_plsf_5.tab"
#include "lsp.tab"
#if WMOPS
#include "count.h"
#endif
/*
*****************************************************************************
*                         LOCAL VARIABLES AND TABLES
*****************************************************************************
*/
dtx_decState  dtx_decS;

#define PN_INITIAL_SEED 0x70816958L   /* Pseudo noise generator seed value  */

/***************************************************
 * Scaling factors for the lsp variability operation *
 ***************************************************/
static const Word16 lsf_hist_mean_scale[M] = {
   20000,
   20000,
   20000,
   20000,
   20000,
   18000,
   16384,
    8192,
       0,
       0
};

/*************************************************
 * level adjustment for different modes Q11      *
 *************************************************/
static const Word16 dtx_log_en_adjust[9] =
{
  -1023, /* MR475 */
   -878, /* MR515 */
   -732, /* MR59  */
   -586, /* MR67  */
   -440, /* MR74  */
   -294, /* MR795 */
   -148, /* MR102 */
      0, /* MR122 */
      0, /* MRDTX */
};

/*
*****************************************************************************
*                         PUBLIC PROGRAM CODE
*****************************************************************************
*/
/*
**************************************************************************
*
*  Function    : dtx_dec_init
*
**************************************************************************
*/
void dtx_dec_init (dtx_decState **st)
{
   dtx_decState* s;

//   if (st == (dtx_decState **) NULL){
//      fprintf(stderr, "dtx_dec_init: invalid parameter\n");
//      return -1;
//   }

//   *st = NULL;

   /* allocate memory */
//   if ((s= (dtx_decState *) malloc(sizeof(dtx_decState))) == NULL){
//      fprintf(stderr, "dtx_dec_init: can not malloc state structure\n");
//      return -1;
//   }

   s = &dtx_decS;
   dtx_dec_reset(s);
   *st = s;

   return;
}

/*
**************************************************************************
*
*  Function    : dtx_dec_reset
*
**************************************************************************
*/
void dtx_dec_reset (dtx_decState *st)
{
   Word32 i;

//   if (st == (dtx_decState *) NULL){
//      fprintf(stderr, "dtx_dec_reset: invalid parameter\n");
//      return -1;
//   }

   st->since_last_sid = 0;
   st->true_sid_period_inv = (1 << 13);

   st->log_en = 3500;
   st->old_log_en = 3500;
   /* low level noise for better performance in  DTX handover cases*/

   st->L_pn_seed_rx = PN_INITIAL_SEED;

   /* Initialize state->lsp [] and state->lsp_old [] */
   Copy(lsp_init_data, &st->lsp[0], M);
   Copy(lsp_init_data, &st->lsp_old[0], M);

   st->lsf_hist_ptr = 0;
   st->log_pg_mean = 0;
   st->log_en_hist_ptr = 0;

   /* initialize decoder lsf history */
   Copy(mean_lsf, &st->lsf_hist[0], M);

   //for (i = 1; i < DTX_HIST_SIZE; i++)
   //{
   //     //Copy(&st->lsf_hist[0], &st->lsf_hist[M*i], M);
   //       Word32 j;
   //       for (j = 0; j < M; j++)
   //       {
   //           st->lsf_hist[M*i+j] = st->lsf_hist[j];         //   move16 ();
   //       }
   //}

   for (i = 0; i < (DTX_HIST_SIZE-1)*M; i++)
   {
        //Copy(&st->lsf_hist[0], &st->lsf_hist[M*i], M);
        st->lsf_hist[M+i] = st->lsf_hist[i];         //   move16 ();
   }

   //   Set_zero(st->lsf_hist_mean, M*DTX_HIST_SIZE);
   Preset(0, st->lsf_hist_mean, M*DTX_HIST_SIZE);

   /* initialize decoder log frame energy */
   //for (i = 0; i < DTX_HIST_SIZE; i++)
   //{
   //   st->log_en_hist[i] = st->log_en;
   //}
   Preset(st->log_en, st->log_en_hist, DTX_HIST_SIZE);

   st->log_en_adjust = 0;

   st->dtxHangoverCount = DTX_HANG_CONST;
   st->decAnaElapsedCount = 32767;

   st->sid_frame = 0;
   st->valid_data = 0;
   st->dtxHangoverAdded = 0;

   st->dtxGlobalState = DTX;
   st->data_updated = 0;
   return;
}

/*
**************************************************************************
*
*  Function    : dtx_dec_exit
*
**************************************************************************
*/
//void dtx_dec_exit (dtx_decState **st)
//{
//   if (st == NULL || *st == NULL)
//      return;

   /* deallocate memory */
//   free(*st);
//   *st = NULL;

//   return;
//}

/*
**************************************************************************
*
*  Function    : dtx_dec
*
**************************************************************************
*/
void dtx_dec(
   dtx_decState *st,                /* i/o : State struct                    */
   Word16 mem_syn[],                /* i/o : AMR decoder state               */
   D_plsfState* lsfState,           /* i/o : decoder lsf states              */
   gc_predState* predState,         /* i/o : prediction states               */
   Cb_gain_averageState* averState, /* i/o : CB gain average states          */
   enum DTXStateType new_state,     /* i   : new DTX state                   */
   enum Mode mode,                  /* i   : AMR mode                        */
   Word16 parm[],                   /* i   : Vector of synthesis parameters  */
   Word16 synth[],                  /* o   : synthesised speech              */
   Word16 A_t[]                     /* o   : decoded LP filter in 4 subframes*/
   )
{
   Word16 log_en_index;
   Word32 i, j;
   Word16 int_fac;
   Word32 L_log_en_int;
   Word16 lsp_int[M];
   Word16 log_en_int_e;
   Word16 log_en_int_m;
   Word16 level;
   Word16 acoeff[M + 1];
   Word16 refl[M];
   Word16 pred_err;
   Word16 ex[L_SUBFR];
   Word16 ma_pred_init;
   Word16 log_pg_e, log_pg_m;
   Word16 log_pg;
   Flag negative;
   Word16 lsf_mean;
   Word32 L_lsf_mean;
   Word16 lsf_variab_index;
   Word16 lsf_variab_factor;
   Word16 lsf_int[M];
   Word16 lsf_int_variab[M];
   Word16 lsp_int_variab[M];
   Word16 acoeff_variab[M + 1];

   Word16 lsf[M];
   Word32 L_lsf[M];
   Word16 ptr;
   Word16 tmp_int_length;

   /*  This function is called if synthesis state is not SPEECH
    *  the globally passed  inputs to this function are
    * st->sid_frame
    * st->valid_data
    * st->dtxHangoverAdded
    * new_state  (SPEECH, DTX, DTX_MUTE)
    */

   if ((st->dtxHangoverAdded != 0) &&
       (st->sid_frame != 0))
   {
      /* sid_first after dtx hangover period */
      /* or sid_upd after dtxhangover        */

      /* set log_en_adjust to correct value */
      st->log_en_adjust = dtx_log_en_adjust[mode];

      ptr = st->lsf_hist_ptr + M;
      if (ptr == 80)
      {
         ptr = 0;
      }
      Copy( &st->lsf_hist[st->lsf_hist_ptr],&st->lsf_hist[ptr],M);

      ptr = st->log_en_hist_ptr + 1;
      if (ptr == DTX_HIST_SIZE)
      {
         ptr = 0;
      }
      st->log_en_hist[ptr] = st->log_en_hist[st->log_en_hist_ptr]; /* Q11 */

      /* compute mean log energy and lsp *
       * from decoded signal (SID_FIRST) */
      st->log_en = 0;
      for (i = 0; i < M; i++)
      {
         L_lsf[i] = 0;
      }

      /* average energy and lsp */
      for (i = 0; i < DTX_HIST_SIZE; i++)
      {
         st->log_en += (st->log_en_hist[i]>>3);
         for (j = 0; j < M; j++)
         {
            L_lsf[j] += (Word32)(st->lsf_hist[i * M + j]);
         }
      }

      for (j = 0; j < M; j++)
      {
         lsf[j] = (Word16)(L_lsf[j] >> 3); /* divide by 8 */
      }

      Lsf_lsp(lsf, st->lsp, M);

      /* make log_en speech coder mode independent */
      /* added again later before synthesis        */
      st->log_en -= st->log_en_adjust;

      /* compute lsf variability vector */
      Copy(st->lsf_hist, st->lsf_hist_mean, 80);

      for (i = 0; i < M; i++)
      {
         L_lsf_mean = 0;
         /* compute mean lsf */
         for (j = 0; j < 8; j++)
         {
            L_lsf_mean += (Word32)(st->lsf_hist_mean[i+j*M]);
         }

         lsf_mean = (Word16)(L_lsf_mean >> 3);
         /* subtract mean and limit to within reasonable limits  *
          * moreover the upper lsf's are attenuated              */
         for (j = 0; j < 8; j++)
         {
            /* subtract mean */
            st->lsf_hist_mean[i+j*M] -= lsf_mean;

            /* attenuate deviation from mean, especially for upper lsf's */
            st->lsf_hist_mean[i+j*M] =
                (Word16)( (st->lsf_hist_mean[i+j*M] * lsf_hist_mean_scale[i])>>15 );
            /* limit the deviation */
            if (st->lsf_hist_mean[i+j*M] < 0)
            {
               negative = 1;
            }
            else
            {
               negative = 0;
            }
            //if ( st->lsf_hist_mean[i+j*M] < 0)
            if ( negative !=0 )
                st->lsf_hist_mean[i+j*M] = - st->lsf_hist_mean[i+j*M];

            /* apply soft limit */
            if (st->lsf_hist_mean[i+j*M] > 655)
            {
               st->lsf_hist_mean[i+j*M] =
                  655 + ((st->lsf_hist_mean[i+j*M] - 655) >> 2);
                /* apply hard limit */
                if (st->lsf_hist_mean[i+j*M] > 1310)
                {
                   st->lsf_hist_mean[i+j*M] = 1310;
                }
            }

            if (negative != 0)
            {
               st->lsf_hist_mean[i+j*M] = -st->lsf_hist_mean[i+j*M];
            }

         }
      }
   }

   if (st->sid_frame != 0 )
   {
      /* Set old SID parameters, always shift */
      /* even if there is no new valid_data   */
      Copy(st->lsp, st->lsp_old, M);
      st->old_log_en = st->log_en;

      if (st->valid_data != 0 )  /* new data available (no CRC) */
      {
         /* Compute interpolation factor, since the division only works *
          * for values of since_last_sid < 32 we have to limit the      *
          * interpolation to 32 frames                                  */
         tmp_int_length = st->since_last_sid;
         st->since_last_sid = 0;

         if (tmp_int_length > 32)
         {
            tmp_int_length = 32;
         }
         if (tmp_int_length >= 2)
         {
            st->true_sid_period_inv = div_s((1 << 10), (tmp_int_length << 10));
         }
         else
         {
            st->true_sid_period_inv = 1 << 14; /* 0.5 it Q15 */
         }

         Init_D_plsf_3(lsfState, parm[0]);  /* temporay initialization */
         D_plsf_3(lsfState, MRDTX, 0, &parm[1], st->lsp);
//         Set_zero(lsfState->past_r_q, M);   /* reset for next speech frame */
         Preset (0, lsfState->past_r_q, M);

         log_en_index = parm[4];
         /* Q11 and divide by 4 */
         st->log_en = log_en_index << 9;

         /* Subtract 2.5 in Q11 */
         st->log_en = st->log_en - 5120;

         /* Index 0 is reserved for silence */

         if (log_en_index == 0)
         {
            st->log_en = MIN_16;
         }

         /* no interpolation at startup after coder reset        */
         /* or when SID_UPD has been received right after SPEECH */
         if ((st->data_updated == 0) ||
             ((Word16)st->dtxGlobalState == SPEECH)
             )
         {
            Copy(st->lsp, st->lsp_old, M);
            st->old_log_en = st->log_en;
         }
      } /* endif valid_data */

      /* initialize gain predictor memory of other modes */
      ma_pred_init = (st->log_en>>1) - 9000;
      if (ma_pred_init > 0)
      {
         ma_pred_init = 0;
      }
      if (ma_pred_init < -14436)
      {
         ma_pred_init = -14436;
      }

      predState->past_qua_en[0] = ma_pred_init;
      predState->past_qua_en[1] = ma_pred_init;
      predState->past_qua_en[2] = ma_pred_init;
      predState->past_qua_en[3] = ma_pred_init;

      /* past_qua_en for other modes than MR122 */
      ma_pred_init = (Word16)((5443 * ma_pred_init)>>15);
      /* scale down by factor 20*log10(2) in Q15 */
      predState->past_qua_en_MR122[0] = ma_pred_init;
      predState->past_qua_en_MR122[1] = ma_pred_init;
      predState->past_qua_en_MR122[2] = ma_pred_init;
      predState->past_qua_en_MR122[3] = ma_pred_init;
   } /* endif sid_frame */

   /* CN generation */
   /* recompute level adjustment factor Q11             *
    * st->log_en_adjust = 0.9*st->log_en_adjust +       *
    *                     0.1*dtx_log_en_adjust[mode]); */
   //st->log_en_adjust = (Word16)((st->log_en_adjust * 29491)>>15) +
   //                    (Word16)(((dtx_log_en_adjust[mode]<<5)*3277)>>20);
   st->log_en_adjust = (Word16)((st->log_en_adjust * 29491)>>15) +
                       (Word16)(((dtx_log_en_adjust[mode])*3277)>>15);

   /* Interpolate SID info */
   // sis3830 shl overflow
   int_fac = shl(add(1,st->since_last_sid), 10); /* Q10 */
   int_fac = (Word16)((int_fac * st->true_sid_period_inv)>>15); /* Q10 * Q15 -> Q10 */

   /* Maximize to 1.0 in Q10 */
   if (int_fac > 1024)
   {
      int_fac = 1024;
   }
   // sis3830 shl overflow
   int_fac = shl(int_fac, 4); /* Q10 -> Q14 */

   // sis3830 L_mult overflow
   L_log_en_int = L_mult(int_fac, st->log_en); /* Q14 * Q11->Q26 */
   for(i = 0; i < M; i++)
   {
      lsp_int[i] = (Word16)((int_fac * st->lsp[i])>>15);/* Q14 * Q15 -> Q14 */
   }

   int_fac = 16384 - int_fac; /* 1-k in Q14 */

   /* (Q14 * Q11 -> Q26) + Q26 -> Q26 */
   // sis3830 L_mac overflow
   L_log_en_int = L_mac(L_log_en_int, int_fac, st->old_log_en);
   for(i = 0; i < M; i++)
   {
      /* Q14 + (Q14 * Q15 -> Q14) -> Q14 */
      lsp_int[i] = lsp_int[i] + (Word16)((int_fac*st->lsp_old[i])>>15);
      lsp_int[i] <<= 1; /* Q14 -> Q15 */
   }

   /* compute the amount of lsf variability */
   lsf_variab_factor = st->log_pg_mean - 2457; /* -0.6 in Q12 */
   /* *0.3 Q12*Q15 -> Q12 */
   lsf_variab_factor = 4096 - (Word16)((lsf_variab_factor*9830)>>15);

   /* limit to values between 0..1 in Q12 */
   if (lsf_variab_factor > 4095)
   {
      // sis3830 Q
      //lsf_variab_factor = 4096;
      lsf_variab_factor = 4095;
   }
   if (lsf_variab_factor < 0)
   {
      lsf_variab_factor = 0;
   }
   lsf_variab_factor <<= 3; /* -> Q15 */

   /* get index of vector to do variability with */
   lsf_variab_index = pseudonoise(&st->L_pn_seed_rx, 3);

   /* convert to lsf */
   Lsp_lsf(lsp_int, lsf_int, M);

   /* apply lsf variability */
   Copy(lsf_int, lsf_int_variab, M);
   for(i = 0; i < M; i++)
   {
      lsf_int_variab[i] = lsf_int_variab[i] +
      (Word16)((lsf_variab_factor*st->lsf_hist_mean[i+lsf_variab_index*M])>>15);
   }

   /* make sure that LSP's are ordered */
   Reorder_lsf(lsf_int, LSF_GAP, M);
   Reorder_lsf(lsf_int_variab, LSF_GAP, M);

   /* copy lsf to speech decoders lsf state */
   Copy(lsf_int, lsfState->past_lsf_q, M);

   /* convert to lsp */
   Lsf_lsp(lsf_int, lsp_int, M);
   Lsf_lsp(lsf_int_variab, lsp_int_variab, M);

   /* Compute acoeffs Q12 acoeff is used for level    *
    * normalization and postfilter, acoeff_variab is  *
    * used for synthesis filter                       *
    * by doing this we make sure that the level       *
    * in high frequenncies does not jump up and down  */

   Lsp_Az(lsp_int, acoeff);
   Lsp_Az(lsp_int_variab, acoeff_variab);

   /* For use in postfilter */
   Copy(acoeff, &A_t[0],           M + 1);
   Copy(acoeff, &A_t[M + 1],       M + 1);
   Copy(acoeff, &A_t[2 * (M + 1)], M + 1);
   Copy(acoeff, &A_t[3 * (M + 1)], M + 1);

   /* Compute reflection coefficients Q15 */
   A_Refl(&acoeff[1], refl);

   /* Compute prediction error in Q15 */
   pred_err = MAX_16; /* 0.99997 in Q15 */
   for (i = 0; i < M; i++)
   {
      j = (Word16)((refl[i]*refl[i])>>15);
      pred_err = (Word16)((pred_err*(MAX_16 - j))>>15);
   }

   /* compute logarithm of prediction gain */
   Log2((Word32)pred_err, &log_pg_e, &log_pg_m);

   /* convert exponent and mantissa to Word16 Q12 */
   // sis3830 shl overflow
   log_pg = shl(sub(log_pg_e,15), 12);  /* Q12 */
   log_pg = (-(log_pg + (log_pg_m >> 3))) >> 1;
   st->log_pg_mean = (Word16)((29491*st->log_pg_mean)>>15) +
                     (Word16)((3277*log_pg)>>15);

   /* Compute interpolated log energy */
   // sis3830 L_shr overflow
   L_log_en_int = L_shr(L_log_en_int, 10); /* Q26 -> Q16 */

   /* Add 4 in Q16 */
   L_log_en_int += 4 * 65536L;

   /* subtract prediction gain */
   L_log_en_int -= ((Word32)log_pg << 4);

   /* adjust level to speech coder mode */
   L_log_en_int += ((Word32)st->log_en_adjust << 5);

   log_en_int_e = (Word16)(L_log_en_int>>16);
   log_en_int_m = (Word16)((L_log_en_int-(((Word32)log_en_int_e)<<16))>>1);
   level = (Word16)(Pow2(log_en_int_e, log_en_int_m)); /* Q4 */

   for (i = 0; i < 4; i++)
   {
      /* Compute innovation vector */
      build_CN_code(&st->L_pn_seed_rx, ex);
      for (j = 0; j < L_SUBFR; j++)
      {
         ex[j] = (Word16)((level * ex[j])>>15);
      }
      /* Synthesize */
      Syn_filt_de(acoeff_variab, ex, &synth[i * L_SUBFR], L_SUBFR,
               mem_syn, 1);

   } /* next i */

   /* reset codebook averaging variables */
   averState->hangVar = 20;
   averState->hangCount = 0;

   if ((Word16)new_state == DTX_MUTE)
   {
      /* mute comfort noise as it has been quite a long time since
       * last SID update  was performed                            */

      tmp_int_length = st->since_last_sid;
      if (tmp_int_length > 32)
      {
         tmp_int_length = 32;
      }

      /* safety guard against division by zero */
      if(tmp_int_length <= 0) {
         tmp_int_length = 8;
      }

      st->true_sid_period_inv = div_s(1 << 10, (tmp_int_length<<10));

      st->since_last_sid = 0;
      Copy(st->lsp, st->lsp_old, M);
      st->old_log_en = st->log_en;
      /* subtract 1/8 in Q11 i.e -6/8 dB */
      st->log_en -= 256;
   }

   /* reset interpolation length timer
    * if data has been updated.        */
   if ((st->sid_frame != 0) &&
       ((st->valid_data != 0) ||
        ((st->valid_data == 0) &&  (st->dtxHangoverAdded) != 0)))
   {
      st->since_last_sid =  0;
      st->data_updated = 1;
   }

   return;
}

void dtx_dec_activity_update(dtx_decState *st,
                             Word16 lsf[],
                             Word16 frame[])
{
#ifndef OPRISCASM
    Word64 temp;
#endif

#if !defined(HAVE_MACLC) || !defined(OPRISCASM)
    Word32 mac_hi, mac_lo;
#endif

   Word32 i;

   Word32 L_frame_en;
   Word16 log_en_e, log_en_m, log_en;

   /* update lsp history */
   st->lsf_hist_ptr += M;
   if (st->lsf_hist_ptr == 80)
   {
      st->lsf_hist_ptr = 0;
   }
   Copy(lsf, &st->lsf_hist[st->lsf_hist_ptr], M);

   /* compute log energy based on frame energy */
   //L_frame_en = 0;     /* Q0 */                                    move32();
   //for (i=0; i < L_FRAME; i++)
   //{
   //   L_frame_en = L_mac(L_frame_en, frame[i], frame[i]);
   //}
#ifdef OPRISCASM
    //asm volatile("l.macrc r0");
    //asm volatile("l.mtspr r0, r0, 0x2801");
    //asm volatile("l.mtspr r0, r0, 0x2802");
   for (i=0; i < L_FRAME; i++)
   {
        asm ("l.mac %0, %1" : : "r"((Word32)frame[i]), "r"((Word32)frame[i]));
   }
#   ifdef HAVE_MACLC
    asm ("l.maclc %0, 1" : "=r" (L_frame_en));
#   else
    asm volatile("l.mfspr %0, r0, 0x2802" : "=r"(mac_hi));
    asm volatile("l.macrc %0, 0" : "=r" (mac_lo));
#   endif
#else
    temp = 0;
    for (i=0; i < L_FRAME; i++)
    {
        temp = temp + frame[i] * frame[i];
    }
    mac_hi = (Word32) (temp >> 32);
    mac_lo = (Word32) (temp & 0xFFFFFFFF);
#endif

#if !defined(OPRISCASM) || (defined(OPRISCASM) && !defined(HAVE_MACLC))
    _CheckOverflow_P(mac_hi, mac_lo, L_frame_en);
#endif

   Log2(L_frame_en, &log_en_e, &log_en_m);

   /* convert exponent and mantissa to Word16 Q10 */
   log_en = (log_en_e << 10);  /* Q10 */
   log_en += (log_en_m >> 5);

   /* divide with L_FRAME i.e subtract with log2(L_FRAME) = 7.32193 */
   log_en -= 8521;

   /* insert into log energy buffer, no division by two as  *
    * log_en in decoder is Q11                              */
   st->log_en_hist_ptr ++;
   if (st->log_en_hist_ptr == DTX_HIST_SIZE)
   {
      st->log_en_hist_ptr = 0;
   }
   st->log_en_hist[st->log_en_hist_ptr] = log_en; /* Q11 */
}

/*
     Table of new SPD synthesis states

                           |     previous SPD_synthesis_state
     Incoming              |
     frame_type            | SPEECH       | DTX           | DTX_MUTE
     ---------------------------------------------------------------
     RX_SPEECH_GOOD ,      |              |               |
     RX_SPEECH_PR_DEGRADED | SPEECH       | SPEECH        | SPEECH
     ----------------------------------------------------------------
     RX_SPEECH_BAD,        | SPEECH       | DTX           | DTX_MUTE
     ----------------------------------------------------------------
     RX_SID_FIRST,         | DTX          | DTX/(DTX_MUTE)| DTX_MUTE
     ----------------------------------------------------------------
     RX_SID_UPDATE,        | DTX          | DTX           | DTX
     ----------------------------------------------------------------
     RX_SID_BAD,           | DTX          | DTX/(DTX_MUTE)| DTX_MUTE
     ----------------------------------------------------------------
     RX_NO_DATA            | SPEECH       | DTX/(DTX_MUTE)| DTX_MUTE
                           |(class2 garb.)|               |
     ----------------------------------------------------------------
     RX_ONSET              | SPEECH       | DTX/(DTX_MUTE)| DTX_MUTE
                           |(class2 garb.)|               |
     ----------------------------------------------------------------
*/

enum DTXStateType rx_dtx_handler(
                      dtx_decState *st,           /* i/o : State struct     */
                      enum RXFrameType frame_type /* i   : Frame type       */
                      )
{
   enum DTXStateType newState;
   enum DTXStateType encState;

   /* DTX if SID frame or previously in DTX{_MUTE} and (NO_RX OR BAD_SPEECH) */
   if (((Word16)frame_type == RX_SID_FIRST)   ||
       ((Word16)frame_type == RX_SID_UPDATE)  ||
       ((Word16)frame_type == RX_SID_BAD)     ||
       ((((Word16)st->dtxGlobalState == DTX) ||
         ((Word16)st->dtxGlobalState == DTX_MUTE)) &&
        (((Word16)frame_type == RX_NO_DATA) ||
         ((Word16)frame_type == RX_SPEECH_BAD) ||
         ((Word16)frame_type == RX_ONSET))))
   {
      newState = DTX;

      /* stay in mute for these input types */
      if (((Word16)st->dtxGlobalState == DTX_MUTE) &&
          (((Word16)frame_type == RX_SID_BAD) ||
           ((Word16)frame_type == RX_SID_FIRST) ||
           ((Word16)frame_type == RX_ONSET) ||
           ((Word16)frame_type == RX_NO_DATA)))
      {
         newState = DTX_MUTE;
      }

      /* evaluate if noise parameters are too old                     */
      /* since_last_sid is reset when CN parameters have been updated */
      st->since_last_sid ++;

      /* no update of sid parameters in DTX for a long while */
      /* Due to the delayed update of  st->since_last_sid counter
         SID_UPDATE frames need to be handled separately to avoid
         entering DTX_MUTE for late SID_UPDATE frames
         */
      if(((Word16)frame_type != RX_SID_UPDATE) &&
         ((Word16)st->since_last_sid > DTX_MAX_EMPTY_THRESH))
      {
         newState = DTX_MUTE;
      }
   }
   else
   {
      newState = SPEECH;
      st->since_last_sid = 0;
   }

   /*
      reset the decAnaElapsed Counter when receiving CNI data the first
      time, to robustify counter missmatch after handover
      this might delay the bwd CNI analysis in the new decoder slightly.
   */
   if ((st->data_updated == 0) &&
       ((Word16)frame_type == RX_SID_UPDATE))
   {
      st->decAnaElapsedCount = 0;
   }

   /* update the SPE-SPD DTX hangover synchronization */
   /* to know when SPE has added dtx hangover         */
   if ( st->decAnaElapsedCount < (Word16)0x7fff )
      st->decAnaElapsedCount ++;
   st->dtxHangoverAdded = 0;

   if (((Word16)frame_type == RX_SID_FIRST)  ||
       ((Word16)frame_type == RX_SID_UPDATE) ||
       ((Word16)frame_type == RX_SID_BAD)    ||
       ((Word16)frame_type == RX_ONSET)      ||
       ((Word16)frame_type == RX_NO_DATA))
   {
      encState = DTX;

      /*
         In frame errors simulations RX_NO_DATA may occasionally mean that
         a speech packet was probably sent by the encoder,
         the assumed _encoder_ state should be SPEECH in such cases.
      */

      if(((Word16)frame_type == RX_NO_DATA) &&
         ((Word16)newState == SPEECH))
      {
         encState = SPEECH;
      }

      /* Note on RX_ONSET operation differing from RX_NO_DATA operation:
         If a  RX_ONSET is received in the decoder (by "accident")
         it is still most likely that the encoder  state
         for the "ONSET frame" was DTX.
      */
   }
   else
   {
      encState = SPEECH;
   }

   if ((Word16)encState == SPEECH)
   {
      st->dtxHangoverCount = DTX_HANG_CONST;
   }
   else
   {
      if (st->decAnaElapsedCount > DTX_ELAPSED_FRAMES_THRESH)
      {
         st->dtxHangoverAdded = 1;
         st->decAnaElapsedCount = 0;
         st->dtxHangoverCount = 0;
      }
      else if (st->dtxHangoverCount == 0)
      {
         st->decAnaElapsedCount = 0;
      }
      else
      {
         st->dtxHangoverCount --;
      }
   }

   if ((Word16)newState != SPEECH)
   {
      /* DTX or DTX_MUTE
       * CN data is not in a first SID, first SIDs are marked as SID_BAD
       *  but will do backwards analysis if a hangover period has been added
       *  according to the state machine above
       */

      st->sid_frame = 0;
      st->valid_data = 0;

      if ((Word16)frame_type == RX_SID_FIRST)
      {
         st->sid_frame = 1;
      }
      else if ((Word16)frame_type == RX_SID_UPDATE)
      {
         st->sid_frame = 1;
         st->valid_data = 1;
      }
      else if ((Word16)frame_type == RX_SID_BAD)
      {
         st->sid_frame = 1;
         st->dtxHangoverAdded = 0; /* use old data */
      }
   }

   return newState;
   /* newState is used by both SPEECH AND DTX synthesis routines */
}
