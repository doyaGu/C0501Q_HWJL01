﻿/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : c3_14pf.c
*      Purpose          : Searches a 14 bit algebraic codebook containing 3 pulses
*                         in a frame of 40 samples.
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "c3_14pf.h"
const char c3_14pf_id[] = "@(#)$Id $" c3_14pf_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "basic_op.h"
//#include "count.h"
#include "inv_sqrt.h"
#include "cnst.h"
#include "cor_h.h"
#include "set_sign.h"
#include "copy.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
#define NB_PULSE  3

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
static void search_3i40(
    Word16 dn[],        /* i : correlation between target and h[]            */
    Word16 dn2[],       /* i : maximum of corr. in each track.               */
    Word16 rr[][L_CODE],/* i : matrix of autocorrelation                     */
    Word16 codvec[]     /* o : algebraic codebook vector                     */
);

static Word16 build_code(
    Word16 codvec[],    /* i : algebraic codebook vector                     */
    Word16 dn_sign[],   /* i : sign of dn[]                                  */
    Word16 cod[],       /* o : algebraic (fixed) codebook excitation         */
    Word16 h[],         /* i : impulse response of weighted synthesis filter */
    Word16 y[],         /* o : filtered fixed codebook excitation            */
    Word16 sign[]       /* o : sign of 3 pulses                              */
);

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
/*************************************************************************
 *
 *  FUNCTION:  code_3i40_14bits()
 *
 *  PURPOSE:  Searches a 14 bit algebraic codebook containing 3 pulses
 *            in a frame of 40 samples.
 *
 *  DESCRIPTION:
 *    The code length is 40, containing 3 nonzero pulses: i0...i2.
 *    All pulses can have two possible amplitudes: +1 or -1.
 *    Pulse i0 can have 8 possible positions, pulses i1 and i2 can have
 *    2x8=16 positions.
 *
 *       i0 :  0, 5, 10, 15, 20, 25, 30, 35.
 *       i1 :  1, 6, 11, 16, 21, 26, 31, 36.
 *             3, 8, 13, 18, 23, 28, 33, 38.
 *       i2 :  2, 7, 12, 17, 22, 27, 32, 37.
 *             4, 9, 14, 19, 24, 29, 34, 39.
 *
 *************************************************************************/

Word16 code_3i40_14bits(
    Word16 x[],         /* i : target vector                                 */
    Word16 h[],         /* i : impulse response of weighted synthesis filter */
                        /*     h[-L_subfr..-1] must be set to zero.          */
    Word16 T0,          /* i : Pitch lag                                     */
    Word16 pitch_sharp, /* i : Last quantized pitch gain                     */
    Word16 code[],      /* o : Innovative codebook                           */
    Word16 y[],         /* o : filtered fixed codebook excitation            */
    Word16 * sign       /* o : Signs of 3 pulses                             */
)
{
    Word16 codvec[NB_PULSE];
    Word16 dn[L_CODE], dn2[L_CODE], dn_sign[L_CODE];
    Word16 rr[L_CODE][L_CODE];
    Word16 index, sharp;
    Word32 i;

    sharp = pitch_sharp << 1;
    if (T0 < L_CODE)
    {
       for (i = T0; i < L_CODE; i++) {
          h[i] += (Word16)((h[i - T0] * sharp)>>15);
       }
    }

    cor_h_x(h, x, dn, 1);
    set_sign(dn, dn_sign, dn2, 6);
    cor_h(h, dn_sign, rr);
    search_3i40(dn, dn2, rr, codvec);
                                     /* function result */
    index = build_code(codvec, dn_sign, code, h, y, sign);

  /*-----------------------------------------------------------------*
  * Compute innovation vector gain.                                 *
  * Include fixed-gain pitch contribution into code[].              *
  *-----------------------------------------------------------------*/

    if (T0 < L_CODE)
    {
       for (i = T0; i < L_CODE; i++) {
          code[i] += (Word16)((code[i - T0] * sharp)>>15);
       }
    }
    return index;
}

/*
********************************************************************************
*                         PRIVATE PROGRAM CODE
********************************************************************************
*/

/*************************************************************************
 *
 *  FUNCTION  search_3i40()
 *
 *  PURPOSE: Search the best codevector; determine positions of the 3 pulses
 *           in the 40-sample frame.
 *
 *************************************************************************/

#define _1_2    (Word16)(32768L/2)
#define _1_4    (Word16)(32768L/4)
#define _1_8    (Word16)(32768L/8)
#define _1_16   (Word16)(32768L/16)

static void search_3i40(
    Word16 dn[],         /* i : correlation between target and h[] */
    Word16 dn2[],        /* i : maximum of corr. in each track.    */
    Word16 rr[][L_CODE], /* i : matrix of autocorrelation          */
    Word16 codvec[]      /* o : algebraic codebook vector          */
)
{
    Word16 i0, i1, i2;
    Word16 ix = 0; /* initialization only needed to keep gcc silent */
    Word16 ps = 0; /* initialization only needed to keep gcc silent */
    Word16 track1, track2, ipos[NB_PULSE];
    Word16 psk, ps0, ps1, sq, sq1;
    Word16 alpk, alp, alp_16;
    Word32 s, alp0, alp1;
    //Word16 i, pos;

    psk = -1;
    alpk = 1;
    //for (i = 0; i < NB_PULSE; i++)
    //{
    //   codvec[i] = i;
    //}
    codvec[0] = 0;
    codvec[1] = 1;
    codvec[2] = 2;

    for (track1 = 1; track1 < 4; track1 += 2)
    {
       for (track2 = 2; track2 < 5; track2 += 2)
       {
          /* fix starting position */

          ipos[0] = 0;
          ipos[1] = track1;
          ipos[2] = track2;

          /*------------------------------------------------------------------*
           * main loop: try 3 tracks.                                         *
           *------------------------------------------------------------------*/

          //for (i = 0; i < NB_PULSE; i++)
          //{
             /*----------------------------------------------------------------*
              * i0 loop: try 8 positions.                                      *
              *----------------------------------------------------------------*/

             /* account for ptr. init. (rr[io]) */
             for (i0 = ipos[0]; i0 < L_CODE; i0 += STEP)
             {
                if (dn2[i0] >= 0)
                {
                   ps0 = dn[i0];
                   alp0 = (rr[i0][i0] * _1_4);

                   /*----------------------------------------------------------------*
                    * i1 loop: 8 positions.                                          *
                    *----------------------------------------------------------------*/

                   sq = -1;
                   alp = 1;
                   ps = 0;
                   ix = ipos[1];

                   /* initialize 4 index for next loop. */
                   /*-------------------------------------------------------------------*
                    *  These index have low complexity address computation because      *
                    *  they are, in fact, pointers with fixed increment.  For example,  *
                    *  "rr[i0][i2]" is a pointer initialized to "&rr[i0][ipos[2]]"      *
                    *  and incremented by "STEP".                                       *
                    *-------------------------------------------------------------------*/

                   for (i1 = ipos[1]; i1 < L_CODE; i1 += STEP)
                   {
                      ps1 = ps0 + dn[i1];   /* idx increment = STEP */

                      /* alp1 = alp0 + rr[i0][i1] + 1/2*rr[i1][i1]; */

//                      alp1 = L_mac(alp0, rr[i1][i1], _1_4); /* idx incr = STEP */
//                      alp1 = L_mac(alp1, rr[i0][i1], _1_2); /* idx incr = STEP */
                      alp1 = alp0;
                      alp1 += (rr[i1][i1] * _1_4);
                      alp1 += (rr[i0][i1] * _1_2);
                      sq1 = (Word16)((ps1 * ps1)>>15);

                      alp_16 = (Word16)((alp1+0x00004000L)>>15);

                      s = (alp * sq1) - (sq * alp_16);

                      if (s > 0)
                      {
                         sq = sq1;
                         ps = ps1;
                         alp = alp_16;
                         ix = i1;
                      }
                   }
                   i1 = ix;

                   /*----------------------------------------------------------------*
                    * i2 loop: 8 positions.                                          *
                    *----------------------------------------------------------------*/

                   ps0 = ps;
                   alp0 = (alp * _1_4);

                   sq = -1;
                   alp = 1;
                   ps = 0;
                   ix = ipos[2];

                   /* initialize 4 index for next loop (see i1 loop) */

                   for (i2 = ipos[2]; i2 < L_CODE; i2 += STEP)
                   {
                      ps1 = ps0 + dn[i2]; /* index increment = STEP */

                      /* alp1 = alp0 + rr[i0][i2] + rr[i1][i2] + 1/2*rr[i2][i2]; */

//                      alp1 = L_mac(alp0, rr[i2][i2], _1_16); /* idx incr = STEP */
//                      alp1 = L_mac(alp1, rr[i1][i2], _1_8);  /* idx incr = STEP */
//                      alp1 = L_mac(alp1, rr[i0][i2], _1_8);  /* idx incr = STEP */
                      alp1 = alp0;
                      alp1 += (rr[i2][i2] * _1_16);
                      alp1 += (rr[i1][i2] * _1_8);
                      alp1 += (rr[i0][i2] * _1_8);

                      sq1 = (Word16)((ps1 * ps1)>>15);

                      alp_16 = (Word16)((alp1+0x00004000L)>>15);

                      s = (alp * sq1) - (sq * alp_16);

                      if (s > 0)
                      {
                         sq = sq1;
                         ps = ps1;
                         alp = alp_16;
                         ix = i2;
                      }
                   }
                   i2 = ix;

                   /*----------------------------------------------------------------*
                    * memorise codevector if this one is better than the last one.   *
                    *----------------------------------------------------------------*/

                   s = (alpk * sq) - (psk * alp);

                   if (s > 0)
                   {
                      psk = sq;
                      alpk = alp;
                      codvec[0] = i0;
                      codvec[1] = i1;
                      codvec[2] = i2;
                   }
                }
             }
             /*----------------------------------------------------------------*
              * Cyclic permutation of i0, i1 and i2.                           *
              *----------------------------------------------------------------*/

          //   pos = ipos[2];
          //   ipos[2] = ipos[1];
          //   ipos[1] = ipos[0];
          //   ipos[0] = pos;
          //}
       }
    }
    return;
}

/*************************************************************************
 *
 *  FUNCTION:  build_code()
 *
 *  PURPOSE: Builds the codeword, the filtered codeword and index of the
 *           codevector, based on the signs and positions of 3 pulses.
 *
 *************************************************************************/

static Word16
build_code(
    Word16 codvec[],  /* i : position of pulses                            */
    Word16 dn_sign[], /* i : sign of pulses                                */
    Word16 cod[],     /* o : innovative code vector                        */
    Word16 h[],       /* i : impulse response of weighted synthesis filter */
    Word16 y[],       /* o : filtered innovative code                      */
    Word16 sign[]     /* o : sign of 3 pulses                              */
)
{
    Word16 j, track, index, _sign[NB_PULSE], indx, rsign;
    Word16 *p0, *p1, *p2;
    Word32 s, i, k;

//    for (i = 0; i < L_CODE; i++) {
//       cod[i] = 0;
//    }
    Preset( 0, cod, L_CODE);

    indx = 0;
    rsign = 0;
    for (k = 0; k < NB_PULSE; k++)
    {
       i = codvec[k];        /* read pulse position */
       j = dn_sign[i];       /* read sign           */

       index = (Word16)((i * 6554)>>15);    /* index = pos/5 */
                                 /* track = pos%5 */
       track = i - index * 5;

       if (track == 1)
          index <<= 4;
       else if (track == 2)
       {
          track = 2;
          index <<= 8;
       }
       else if (track == 3)
       {
          track = 1;
          index = (index << 4) + 8;
       }
       else if (track == 4)
       {
          track = 2;
          index = (index << 8) + 128;
       }

       if (j > 0)
       {
          cod[i] = 8191;
          _sign[k] = 32767;
          rsign += (1 << track);
       } else {
          cod[i] = -8192;
          _sign[k] = (Word16) - 32768L;
       }

       indx += index;
    }
    *sign = rsign;

    p0 = h - codvec[0];
    p1 = h - codvec[1];
    p2 = h - codvec[2];

    for (i = 0; i < L_CODE; i++)
    {
       s = (p0[i] * _sign[0]);
       s += (p1[i] * _sign[1]);
       s += (p2[i] * _sign[2]);
       y[i] = (Word16)((s+0x00004000L)>>15);
    }

    return indx;
}
