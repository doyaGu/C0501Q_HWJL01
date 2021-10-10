// OK
/*************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : bgnscd.c
*      Purpose          : Background noise source charateristic detector (SCD)
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "bgnscd.h"
const char bgnscd_id[] = "@(#)$Id $" bgnscd_h;

//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
#include "basic_op.h"
//#include "count.h"
#include "cnst.h"
#include "copy.h"
//#include "set_zero.h"
#include "gmed_n.h"
#include "sqrt_l.h"
#if WMOPS
#include "count.h"
#endif
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
Bgn_scdState   Bgn_scdS;

/*
**************************************************************************
*
*  Function    : Bgn_scd_init
*  Purpose     : Allocates and initializes state memory
*
**************************************************************************
*/
void Bgn_scd_init (Bgn_scdState **state)
{
   Bgn_scdState* s;

//   if (state == (Bgn_scdState **) NULL){
//      fprintf(stderr, "Bgn_scd_init: invalid parameter\n");
//      return -1;
//   }
//   *state = NULL;

   /* allocate memory */
//   if ((s= (Bgn_scdState *) malloc(sizeof(Bgn_scdState))) == NULL){
//     fprintf(stderr, "Bgn_scd_init: can not malloc state structure\n");
//     return -1;
//   }

   s = &Bgn_scdS;
   Bgn_scd_reset(s);
   *state = s;

   return;
}

/*
**************************************************************************
*
*  Function    : Bgn_scd_reset
*  Purpose     : Resets state memory
*
**************************************************************************
*/
void Bgn_scd_reset (Bgn_scdState *state)
{
//   if (state == (Bgn_scdState *) NULL){
//      fprintf(stderr, "Bgn_scd_reset: invalid parameter\n");
//      return -1;
//   }

   /* Static vectors to zero */
//   Set_zero (state->frameEnergyHist, L_ENERGYHIST);
    Preset (0, state->frameEnergyHist, L_ENERGYHIST);
    /* Initialize hangover handling */
    state->bgHangover = 0;

    return;
}

/*
**************************************************************************
*
*  Function    : Bgn_scd_exit
*  Purpose     : The memory used for state memory is freed
*
**************************************************************************
*/
//void Bgn_scd_exit (Bgn_scdState **state)
//{
//   if (state == NULL || *state == NULL)
//      return;

   /* deallocate memory */
//   free(*state);
//   *state = NULL;

//   return;
//}

/*
**************************************************************************
*
*  Function    : Bgn_scd
*  Purpose     : Charaterice synthesis speech and detect background noise
*  Returns     : background noise decision; 0 = no bgn, 1 = bgn
*
**************************************************************************
*/
Word16 Bgn_scd (Bgn_scdState *st,      /* i : State variables for bgn SCD */
                Word16 ltpGainHist[],  /* i : LTP gain history            */
                Word16 speech[],       /* o : synthesis speech frame      */
                Word16 *voicedHangover /* o : # of frames after last
                                              voiced frame                */
                )
{
   Word32 i;
   Word16 prevVoiced, inbgNoise;
   Word16 temp;
   Word16 ltpLimit, frameEnergyMin;
   Word16 currEnergy, noiseFloor, maxEnergy, maxEnergyLastPart;
   //Word32 s;

   /* Update the inBackgroundNoise flag (valid for use in next frame if BFI) */
   /* it now works as a energy detector floating on top                      */
   /* not as good as a VAD.                                                  */

   //currEnergy = 0;
   //s = (Word32) 0;
   //
   //for (i = 0; i < L_FRAME; i++)
   //{
   //    s = L_mac (s, speech[i], speech[i]);
   //}
   //
   //s = L_shl(s, 2);
   //
   //currEnergy = extract_h (s);

   // 2004.9.10 sis3830 RISC Implementation
   //Word32 mac_hi;
   Word32 mac_lo;
#ifdef OPRISCASM
    for (i=0; i < L_FRAME; i++)
    {
        asm ("l.mac %0, %1"         : : "r"((Word32)speech[i]), "r"((Word32)speech[i]));
    }
  //asm ("l.mfspr %0, r0, 0x2802"   : "=r"(mac_hi));
    asm ("l.macrc %0, 0"            : "=r"(mac_lo));
    asm ("l.sfgts %0, %1"           :               : "r"((Word32)mac_lo), "r"((Word32)0x0fffffffL) );
    asm ("l.cmov %0, %1, %2"        : "=r"(mac_lo)  : "r"((Word32)0x0fffffffL), "0"((Word32)mac_lo) );
#else
    Word64 LL_temp;
    LL_temp = 0;
    for (i=0; i < L_FRAME; i++)
    {
        LL_temp = LL_temp + speech[i] * speech[i];
    }
    //mac_hi = (Word32) (LL_temp >> 32);
    mac_lo = (Word32) (LL_temp & 0xFFFFFFFF);
    if ( mac_lo > (Word32)0x0fffffffL )
        mac_lo = 0x0fffffffL;
#endif

   //s = mac_lo;
   currEnergy = (Word16)(mac_lo>>13);

   frameEnergyMin = 32767;

   for (i = 0; i < L_ENERGYHIST; i++)
   {
      if (st->frameEnergyHist[i] < frameEnergyMin)
         frameEnergyMin = st->frameEnergyHist[i];
   }

   noiseFloor = frameEnergyMin << 4; /* Frame Energy Margin of 16 */

   maxEnergy = st->frameEnergyHist[0];
   for (i = 1; i < L_ENERGYHIST-4; i++)
   {
      if ( maxEnergy < st->frameEnergyHist[i])
      {
         maxEnergy = st->frameEnergyHist[i];
      }
   }
   // L_ENERGYHIST = 60
   //maxEnergyLastPart = st->frameEnergyHist[2*L_ENERGYHIST/3];
   maxEnergyLastPart = st->frameEnergyHist[40];
   //for (i = 2*L_ENERGYHIST/3+1; i < L_ENERGYHIST; i++)
   for (i = 41; i < 60; i++)
   {
      if ( maxEnergyLastPart < st->frameEnergyHist[i] )
      {
         maxEnergyLastPart = st->frameEnergyHist[i];
      }
   }

   inbgNoise = 0;        /* false */

   /* Do not consider silence as noise */
   /* Do not consider continuous high volume as noise */
   /* Or if the current noise level is very low */
   /* Mark as noise if under current noise limit */
   /* OR if the maximum energy is below the upper limit */

   if ( (maxEnergy > LOWERNOISELIMIT) &&
        (currEnergy < FRAMEENERGYLIMIT) &&
        (currEnergy > LOWERNOISELIMIT) &&
        ( (currEnergy < noiseFloor) ||
          (maxEnergyLastPart < UPPERNOISELIMIT)))
   {

      if (st->bgHangover > 29)
      {
         st->bgHangover = 30;
      } else
      {
         st->bgHangover ++;
      }
   }
   else
   {
      st->bgHangover = 0;
   }

   /* make final decision about frame state , act somewhat cautiosly */
   if (st->bgHangover > 1)
      inbgNoise = 1;       /* true  */

   //for (i = 0; i < L_ENERGYHIST-1; i++)
   for (i = 0; i < 59; i++)
   {
      st->frameEnergyHist[i] = st->frameEnergyHist[i+1];
   }
   //st->frameEnergyHist[L_ENERGYHIST-1] = currEnergy;
   st->frameEnergyHist[59] = currEnergy;

   /* prepare for voicing decision; tighten the threshold after some
      time in noise */
   ltpLimit = 13926;             /* 0.85  Q14 */
   if (st->bgHangover > 8)
   {
      ltpLimit = 15565;          /* 0.95  Q14 */
   }
   if (st->bgHangover > 15)
   {
      ltpLimit = 16383;          /* 1.00  Q14 */
   }

   /* weak sort of voicing indication. */
   prevVoiced = 0;        /* false */

   if (gmed_n(&ltpGainHist[4], 5) > ltpLimit)
   {
      prevVoiced = 1;     /* true  */
   }
   if (st->bgHangover > 20)
   {
      if (gmed_n(ltpGainHist, 9) > ltpLimit)
      {
         prevVoiced = 1;  /* true  */
      }
      else
      {
         prevVoiced = 0;  /* false */
      }
   }

   if (prevVoiced)
   {
      *voicedHangover = 0;
   }
   else
   {
      temp = *voicedHangover + 1;
      if (temp > 10)
      {
         *voicedHangover = 10;
      }
      else
      {
         *voicedHangover = temp;
      }
   }

   return inbgNoise;
}
