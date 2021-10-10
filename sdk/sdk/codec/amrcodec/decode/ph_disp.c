/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : ph_disp.c
*      Purpose          : Perform adaptive phase dispersion of the excitation
*                         signal.
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "ph_disp.h"
const char ph_disp_id[] = "@(#)$Id $" ph_disp_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
//#include <stdio.h>
//#include <stdlib.h>
#include "typedef.h"
#include "basic_op.h"
//#include "count.h"
#include "cnst.h"
#include "copy.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/

#include "ph_disp.tab"
ph_dispState ph_dispS;

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
/*************************************************************************
*
*  Function:   ph_disp_init
*
**************************************************************************
*/
void ph_disp_init (ph_dispState **state)
{
  ph_dispState *s;

//  if (state == (ph_dispState **) NULL){
//      fprintf(stderr, "ph_disp_init: invalid parameter\n");
//      return -1;
//  }
//  *state = NULL;

  /* allocate memory */
//  if ((s= (ph_dispState *) malloc(sizeof(ph_dispState))) == NULL){
//      fprintf(stderr, "ph_disp_init: can not malloc state structure\n");
//      return -1;
//  }
  s = &ph_dispS;
  ph_disp_reset(s);
  *state = s;

  return;

}

/*************************************************************************
*
*  Function:   ph_disp_reset
*
**************************************************************************
*/
void ph_disp_reset (ph_dispState *state)
{
//  Word16 i;

//   if (state == (ph_dispState *) NULL){
//      fprintf(stderr, "ph_disp_reset: invalid parameter\n");
//      return -1;
//   }
   //for (i=0; i<PHDGAINMEMSIZE; i++)
   //{
   //    state->gainMem[i] = 0;
   //}
   Preset(0 , state->gainMem, PHDGAINMEMSIZE);
   state->prevState = 0;
   state->prevCbGain = 0;
   state->lockFull = 0;
   state->onset = 0;          /* assume no onset in start */

   return;
}

/*************************************************************************
*
*  Function:   ph_disp_exit
*
**************************************************************************
*/
//void ph_disp_exit (ph_dispState **state)
//{
//  if ((state == NULL) || (*state == NULL))
//      return;

  /* deallocate memory */
//  free(*state);
//  *state = NULL;

//  return;
//}
/*************************************************************************
*
*  Function:   ph_disp_lock
*
**************************************************************************
*/
void ph_disp_lock (ph_dispState *state)
{
  state->lockFull = 1;
  return;
}

/*************************************************************************
*
*  Function:   ph_disp_release
*
**************************************************************************
*/
void ph_disp_release (ph_dispState *state)
{
  state->lockFull = 0;
  return;
}

/*************************************************************************
*
*  Function:   ph_disp
*
*              Adaptive phase dispersion; forming of total excitation
*              (for synthesis part of decoder)
*
**************************************************************************
*/
void ph_disp (
      ph_dispState *state, /* i/o     : State struct                     */
      enum Mode mode,      /* i       : codec mode                       */
      Word16 x[],          /* i/o Q0  : in:  LTP excitation signal       */
                           /*           out: total excitation signal     */
      Word16 cbGain,       /* i   Q1  : Codebook gain                    */
      Word16 ltpGain,      /* i   Q14 : LTP gain                         */
      Word16 inno[],       /* i/o Q13 : Innovation vector (Q12 for 12.2) */
      Word16 pitch_fac,    /* i   Q14 : pitch factor used to scale the
                                        LTP excitation (Q13 for 12.2)    */
      Word16 tmp_shift     /* i   Q0  : shift factor applied to sum of
                                        scaled LTP ex & innov. before
                                        rounding                         */
)
{
   Word32 i, i1;
   Word16 tmp1;
   Word32 L_temp;
   Word16 impNr;           /* indicator for amount of disp./filter used */

   Word16 inno_sav[L_SUBFR];
   Word16 ps_poss[L_SUBFR];
   Word16 j, nze, nPulse, ppos;
   const Word16 *ph_imp;   /* Pointer to phase dispersion filter */

   /* Update LTP gain memory */
   for (i = PHDGAINMEMSIZE-1; i > 0; i--)
   {
       state->gainMem[i] = state->gainMem[i-1];
   }
   state->gainMem[0] = ltpGain;

   /* basic adaption of phase dispersion */
   if (ltpGain < PHDTHR2LTP)
   {    /* if (ltpGain < 0.9) */
       if (ltpGain > PHDTHR1LTP)
       {  /* if (ltpGain > 0.6 */
          impNr = 1; /* medium dispersion */
       }
       else
       {
          impNr = 0; /* maximum dispersion */
       }
   }
   else
   {
      impNr = 2; /* no dispersion */
   }

   /* onset indicator */
   /* onset = (cbGain  > onFact * cbGainMem[0]) */

//   tmp1 = round16(L_shl(L_mult(state->prevCbGain, ONFACTPLUS1), 2));
   L_temp = (state->prevCbGain * ONFACTPLUS1);
   if ( L_temp > (Word32)0x0fffffffL )
      tmp1 = 0x7fff;
   else if ( L_temp < (Word32)0xf0000000L)
      tmp1 = (Word16)0x8000;
   else
      tmp1 = (Word16)((L_temp+0x00001000L)>>13);
//   tmp1 = (Word16)(((state->prevCbGain * ONFACTPLUS1)+0x00001000L)>>13);
   if (cbGain > tmp1)
   {
       state->onset = ONLENGTH;
   }
   else
   {
       if (state->onset > 0)
       {
           state->onset --;
       }
   }

   /* if not onset, check ltpGain buffer and use max phase dispersion if
      half or more of the ltpGain-parameters say so */
   if (state->onset == 0)
   {
       /* Check LTP gain memory and set filter accordingly */
       i1 = 0;
       for (i = 0; i < PHDGAINMEMSIZE; i++)
       {

           if (state->gainMem[i] < PHDTHR1LTP)
           {
               i1 ++;
           }
       }

       if (i1 > 2)
       {
           impNr = 0;
       }

   }
   /* Restrict decrease in phase dispersion to one step if not onset */
   if ((impNr > (state->prevState+1)) && (state->onset == 0))
   {
       impNr --;
   }
   /* if onset, use one step less phase dispersion */
   if((impNr < 2) && (state->onset > 0))
   {
       impNr ++;
   }

   /* disable for very low levels */
   if(cbGain < 10)
   {
       impNr = 2;
   }

   if(state->lockFull == 1)
   {
       impNr = 0;
   }

   /* update static memory */
   state->prevState = impNr;
   state->prevCbGain = cbGain;

   /* do phase dispersion for all modes but 12.2 and 7.4;
      don't modify the innovation if impNr >=2 (= no phase disp) */
   if (((Word16)mode != MR122) &&
       ((Word16)mode != MR102) &&
       ((Word16)mode != MR74) &&
       (impNr < 2))
   {
       /* track pulse positions, save innovation,
          and initialize new innovation          */
       nze = 0;
       for (i = 0; i < L_SUBFR; i++)
       {
           if (inno[i] != 0)
           {
               ps_poss[nze] = i;
               nze ++;
           }
           inno_sav[i] = inno[i];
           inno[i] = 0;
       }
       /* Choose filter corresponding to codec mode and dispersion criterium */
       if ((Word16)mode == MR795)
       {
           if (impNr == 0)
           {
               ph_imp = ph_imp_low_MR795;
           }
           else
           {
               ph_imp = ph_imp_mid_MR795;
           }
       }
       else
       {
           if (impNr == 0)
           {
               ph_imp = ph_imp_low;
           }
           else
           {
               ph_imp = ph_imp_mid;
           }
       }

       /* Do phase dispersion of innovation */
       for (nPulse = 0; nPulse < nze; nPulse++)
       {
           ppos = ps_poss[nPulse];

           /* circular convolution with impulse response */
           j = 0;
           for (i = ppos; i < L_SUBFR; i++)
           {
               /* inno[i1] += inno_sav[ppos] * ph_imp[i1-ppos] */
               tmp1 = (Word16)((inno_sav[ppos] * ph_imp[j++])>>15);
               inno[i] += tmp1;
           }

           for (i = 0; i < ppos; i++)
           {
               /* inno[i] += inno_sav[ppos] * ph_imp[L_SUBFR-ppos+i] */
               tmp1 = (Word16)((inno_sav[ppos] * ph_imp[j++])>>15);
               inno[i] += tmp1;
           }
       }
   }

   /* compute total excitation for synthesis part of decoder
      (using modified innovation if phase dispersion is active) */
   for (i = 0; i < L_SUBFR; i++)
   {
       /* x[i] = gain_pit*x[i] + cbGain*code[i]; */
       L_temp = (x[i] * pitch_fac);
       L_temp += (inno[i] * cbGain);
       L_temp <<= tmp_shift;                 /* Q16 */
       x[i] = (Word16)((L_temp+0x00004000L)>>15);
   }

   return;
}
