/* OK
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
*****************************************************************************
*
*      File             : c2_9pf.c
*      Purpose          : Searches a 9 bit algebraic codebook containing
*                         2 pulses in a frame of 40 samples.
*
*****************************************************************************
*/

/*
*****************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
*****************************************************************************
*/
#include "c2_9pf.h"
const char c2_9pf_id[] = "@(#)$Id $" c2_9pf_h;

/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
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
*****************************************************************************
*                         LOCAL VARIABLES AND TABLES
*****************************************************************************
*/
#define NB_PULSE  2

#include "c2_9pf.tab"

/*
*****************************************************************************
*                         DECLARATION OF PROTOTYPES
*****************************************************************************
*/
static void search_2i40(
    Word16 subNr,       /* i : subframe number                               */
    Word16 dn[],        /* i : correlation between target and h[]            */
    Word16 rr[][L_CODE],/* i : matrix of autocorrelation                     */
    Word32 codvec[]     /* o : algebraic codebook vector                     */
);
static Word16 build_code(
    Word16 subNr,       /* i : subframe number                               */
    Word32 codvec[],    /* i : algebraic codebook vector                     */
    Word16 dn_sign[],   /* i : sign of dn[]                                  */
    Word16 cod[],       /* o : algebraic (fixed) codebook excitation         */
    Word16 h[],         /* i : impulse response of weighted synthesis filter */
    Word16 y[],         /* o : filtered fixed codebook excitation            */
    Word16 sign[]       /* o : sign of 2 pulses                              */
);

/*
*****************************************************************************
*                         PUBLIC PROGRAM CODE
*****************************************************************************
*/
/*************************************************************************
 *
 *  FUNCTION:  code_2i40_9bits()
 *
 *  PURPOSE:  Searches a 9 bit algebraic codebook containing 2 pulses
 *            in a frame of 40 samples.
 *
 *  DESCRIPTION:
 *    The code length is 40, containing 2 nonzero pulses: i0...i1.
 *    All pulses can have two possible amplitudes: +1 or -1.
 *    Pulse i0 can have 8 possible positions, pulse i1 can have
 *    8 positions. Also coded is which track pair should be used,
 *    i.e. first or second pair. Where each pair contains 2 tracks.
 *
 *     First subframe:
 *     first   i0 :  0, 5, 10, 15, 20, 25, 30, 35.
 *             i1 :  2, 7, 12, 17, 22, 27, 32, 37.
 *     second  i0 :  1, 6, 11, 16, 21, 26, 31, 36.
 *             i1 :  3, 8, 13, 18, 23, 28, 33, 38.
 *
 *     Second subframe:
 *     first   i0 :  0, 5, 10, 15, 20, 25, 30, 35.
 *             i1 :  3, 8, 13, 18, 23, 28, 33, 38.
 *     second  i0 :  2, 7, 12, 17, 22, 27, 32, 37.
 *             i1 :  4, 9, 14, 19, 24, 29, 34, 39.
 *
 *     Third subframe:
 *     first   i0 :  0, 5, 10, 15, 20, 25, 30, 35.
 *             i1 :  2, 7, 12, 17, 22, 27, 32, 37.
 *     second  i0 :  1, 6, 11, 16, 21, 26, 31, 36.
 *             i1 :  4, 9, 14, 19, 24, 29, 34, 39.
 *
 *     Fourth subframe:
 *     first   i0 :  0, 5, 10, 15, 20, 25, 30, 35.
 *             i1 :  3, 8, 13, 18, 23, 28, 33, 38.
 *     second  i0 :  1, 6, 11, 16, 21, 26, 31, 36.
 *             i1 :  4, 9, 14, 19, 24, 29, 34, 39.
 *
 *************************************************************************/

Word16 code_2i40_9bits(
    Word16 subNr,       /* i : subframe number                               */
    Word16 x[],         /* i : target vector                                 */
    Word16 h[],         /* i : impulse response of weighted synthesis filter */
                        /*     h[-L_subfr..-1] must be set to zero.          */
    Word16 T0,          /* i : Pitch lag                                     */
    Word16 pitch_sharp, /* i : Last quantized pitch gain                     */
    Word16 code[],      /* o : Innovative codebook                           */
    Word16 y[],         /* o : filtered fixed codebook excitation            */
    Word16 * sign       /* o : Signs of 2 pulses                             */
)
{
    Word32 codvec[NB_PULSE];
    Word16 dn[L_CODE], dn_sign[L_CODE];
    Word16 rr[L_CODE][L_CODE];
    Word16 index, sharp, val;
    Word32 i;
    //Word16 dn2[L_CODE];

    sharp = pitch_sharp <<1;
    if (T0 < L_CODE)
    {
#ifdef CODE2I409BIT_ENG
VOLATILE (
      *(volatile int *)UsrDefC0 = (int)(sharp);
      *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) h;
      *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) &h[T0];
      *(volatile int *)Dst0Base = (FALSE << P_RdDec) | (int) &h[T0];
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

      *(volatile int *)(ALU_OP0)   =   (SRC_C0    << P_Mul3L) |
                                       (SRC_C0    << P_Mul3R) |
                                       (R_SHIFT15 << P_SAdd0) |
                                       (NoSHIFT   << P_SAdd1) |
                                       (DST_P0    << P_Dst0 ) |
                                       (DST_NoWr  << P_Dst1 ) |
                                       (DST_NoWr  << P_Dst2 ) |
                                       (DST_NoWr  << P_Dst3 ) ;
      *(volatile int *)RQ_TYPE  =   (TRUE       << P_Fire      )|
                                    (FALSE      << P_EnINT     )|
                                    (FALSE      << P_EnInitAcc )|
                                    (TRUE       << P_Sat       )|
                                    (DATA16     << P_RdDT      )|
                                    (DATA16     << P_WrDT      )|
                                    (LARGE      << P_RdGranSize)|
                                    (GRANULE_2  << P_RdGranule )|
                                    (LARGE      << P_WrGranSize)|
                                    (GRANULE_1  << P_WrGranule )|
                                    ((L_CODE-T0)<< P_Len       )|
                                    (0          << P_RdIncr    )|
                                    (0          << P_WrIncr    );
      WaitEngineDone();
 );
#elif defined CODE2I409BIT_PUREC
       for (i = T0; i < L_CODE; i++)
          h[i] += (Word16)((h[i - T0]*sharp)>>15);
#else
#error "error"
#endif
    }
    cor_h_x(h, x, dn, 1);
    /*set_sign(dn, dn_sign, dn2, 8);  dn2[] not used in this codebook search */
    for (i = 0; i < L_CODE; i++) {
      val = dn[i];
      if (val >= 0)
      {
         dn_sign[i] = 32767;
      } else
      {
         dn_sign[i] = -32767;
         val = -val;
      }
      dn[i] = val;    /* modify dn[] according to the fixed sign */
      //dn2[i] = val;
   }

    cor_h(h, dn_sign, rr);
    search_2i40(subNr, dn, rr, codvec); /* function result */
    index = build_code(subNr, codvec, dn_sign, code, h, y, sign);

  /*-----------------------------------------------------------------*
   * Compute innovation vector gain.                                 *
   * Include fixed-gain pitch contribution into code[].              *
   *-----------------------------------------------------------------*/

    if (T0 < L_CODE)
       for (i = T0; i < L_CODE; i++) {
          code[i] += (Word16)((code[i - T0]*sharp)>>15);
       }
    return index;
}

/*
*****************************************************************************
*                         PRIVATE PROGRAM CODE
*****************************************************************************
*/

/*************************************************************************
 *
 *  FUNCTION  search_2i40()
 *
 *  PURPOSE: Search the best codevector; determine positions of the 2 pulses
 *           in the 40-sample frame.
 *
 *************************************************************************/

#define _1_2    (Word16)(32768L/2)
#define _1_4    (Word16)(32768L/4)
#define _1_8    (Word16)(32768L/8)
#define _1_16   (Word16)(32768L/16)

static void search_2i40(
    Word16 subNr,        /* i : subframe number                    */
    Word16 dn[],         /* i : correlation between target and h[] */
    Word16 rr[][L_CODE], /* i : matrix of autocorrelation          */
    Word32 codvec[]      /* o : algebraic codebook vector          */
)
{
    Word32 i0, i1;
    Word32 ix = 0; /* initialization only needed to keep gcc silent */
    Word16 ipos[NB_PULSE];
    Word16 psk, ps0, ps1, sq, sq1;
    Word16 alpk, alp, alp_16;
    Word32 s, alp0, alp1;
    Word32 track1;

    psk = -1;
    alpk = 1;
    //for (i = 0; i < NB_PULSE; i++)
    //{
    //   codvec[i] = i;
    //}
    codvec[0]=0;
    codvec[1]=1;

    for (track1 = 0; track1 < 2; track1++) {
       /* fix starting position */

       ipos[0] = startPos[subNr*2+8*track1];
       ipos[1] = startPos[subNr*2+1+8*track1];

          /*----------------------------------------------------------------*
           * i0 loop: try 8 positions.                                      *
           *----------------------------------------------------------------*/
                              /* account for ptr. init. (rr[io]) */
          for (i0 = ipos[0]; i0 < L_CODE; i0 += STEP)
          {
             ps0 = dn[i0];
             alp0 = (rr[i0][i0] * _1_4);

          /*----------------------------------------------------------------*
           * i1 loop: 8 positions.                                          *
           *----------------------------------------------------------------*/

             sq = -1;
             alp = 1;
             ix = ipos[1];

        /*-------------------------------------------------------------------*
        *  These index have low complexity address computation because      *
        *  they are, in fact, pointers with fixed increment.  For example,  *
        *  "rr[i0][i2]" is a pointer initialized to "&rr[i0][ipos[2]]"      *
        *  and incremented by "STEP".                                       *
        *-------------------------------------------------------------------*/

             for (i1 = ipos[1]; i1 < L_CODE; i1 += STEP) {
                ps1 = ps0 + dn[i1];   /* idx increment = STEP */
                sq1 = (Word16)((ps1*ps1)>>15);
                /* alp1 = alp0 + rr[i0][i1] + 1/2*rr[i1][i1]; */

//                alp1 = L_mac(alp0, rr[i1][i1], _1_4); /* idx incr = STEP */
//                alp1 = L_mac(alp1, rr[i0][i1], _1_2); /* idx incr = STEP */
                alp1 = alp0;
                alp1 += (rr[i1][i1] * _1_4);
                alp1 += (rr[i0][i1] * _1_2);

                alp_16 = (Word16)((alp1+0x00004000L)>>15);

                s = (alp*sq1) - (sq*alp_16);

                if (s > 0) {
                   sq = sq1;
                   alp = alp_16;
                   ix = i1;
                }

             /*   ps1 = ps0 + dn[i1+5];  // for performance optimum
                sq1 = (Word16)((ps1*ps1)>>15);
                alp1 = alp0;
                alp1 += (rr[i1+5][i1+5] * _1_4);
                alp1 += (rr[i0][i1+5] * _1_2);
                alp_16 = (Word16)((alp1+0x00004000L)>>15);
                s = (alp*sq1) - (sq*alp_16);

                if (s > 0) {
                   sq = sq1;
                   alp = alp_16;
                   ix = i1+5;
                }   */

             }

          /*----------------------------------------------------------------*
           * memorise codevector if this one is better than the last one.   *
           *----------------------------------------------------------------*/

             s = (alpk * sq) - (psk * alp);

             if (s > 0) {
                psk = sq;
                alpk = alp;
                codvec[0] = i0;
                codvec[1] = ix;
             }
          }
    }

    return;
}

/*************************************************************************
 *
 *  FUNCTION:  build_code()
 *
 *  PURPOSE: Builds the codeword, the filtered codeword and index of the
 *           codevector, based on the signs and positions of 2 pulses.
 *
 *************************************************************************/

static Word16 build_code(
    Word16 subNr,     /* i : subframe number                               */
    Word32 codvec[],  /* i : position of pulses                            */
    Word16 dn_sign[], /* i : sign of pulses                                */
    Word16 cod[],     /* o : innovative code vector                        */
    Word16 h[],       /* i : impulse response of weighted synthesis filter */
    Word16 y[],       /* o : filtered innovative code                      */
    Word16 sign[]     /* o : sign of 2 pulses                              */
)
{
    Word32 i, j, k, track, first, index, _sign[NB_PULSE], indx, rsign;
    Word16 *p0, *p1, *pt;
    Word32 s;
    static Word16 trackTable[4*5] = {
       0, 1, 0, 1, -1, /* subframe 1; track to code; -1 do not code this position */
       0, -1, 1, 0, 1, /* subframe 2 */
       0, 1, 0, -1, 1, /* subframe 3 */
       0, 1, -1, 0, 1};/* subframe 4 */

    pt = &trackTable[(subNr*5)];

//  for (i = 0; i < L_CODE; i++) {
//     cod[i] = 0;
//  }
#ifdef C29PF_SET
    memset16( 0, cod, L_CODE );
#else
    Preset( 0, cod, L_CODE );
#endif

    indx = 0;
    rsign = 0;
    for (k = 0; k < NB_PULSE; k++) {
       i = codvec[k];    /* read pulse position */
       j = dn_sign[i];   /* read sign           */

       index = (Word16)((i*6554)>>15);    /* index = pos/5 */
                                 /* track = pos%5 */
       track = i - index * 5;

       first = pt[track];

       if (first == 0) {
          if (k == 0) {
             track = 0;
          } else {
             track = 1;
             index <<= 3;
          }
       } else {
          if (k == 0) {
             track = 0;
             index += 64;    /* table bit is MSB */
          } else {
             track = 1;
             index <<= 3;
          }
       }

       if (j > 0) {
          cod[i] = 8191;
          _sign[k] = 32767;
          rsign += 1<<track;
       } else {
          cod[i] = -8192;
          _sign[k] = (Word16) - 32768L;
        }

       indx += index;
    }
    *sign = rsign;

    p0 = h - codvec[0];
    p1 = h - codvec[1];
#ifdef C29PF_ENG
VOLATILE (
    *(volatile int *)UsrDefC0 = (int)(_sign[0]);
    *(volatile int *)UsrDefC1 = (int)(_sign[1]);
    *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) p0;
    *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) p1;
    //*(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) &p0[20];
    //*(volatile int *)Src3Base = (FALSE << P_RdDec) | (int) &p1[20];
    *(volatile int *)Dst0Base = (FALSE << P_RdDec) | (int) y;
    //*(volatile int *)Dst2Base = (FALSE << P_RdDec) | (int) &y[20];
    *(volatile int *)(ALU_OP0+4) =
                                    (FALSE     << P_BP   ) |
                                    (0x4       << P_SAdd2) |
                                    (0x4       << P_SAdd3) |
                                    (SRC_FIFOA << P_Mul0L) |
                                    (SRC_UDC0  << P_Mul0R) |
                                    (SRC_FIFOC << P_Mul1L) |
                                    (SRC_UDC1  << P_Mul1R) |
                                    (SRC_C0    << P_Mul2L) |
                                    (SRC_C0    << P_Mul2R) ;

        *(volatile int *)(ALU_OP0)   = (SRC_C0    << P_Mul3L) |
                                       (SRC_C0    << P_Mul3R) |
                                       (R_SHIFT15 << P_SAdd0) |
                                       (R_SHIFT15 << P_SAdd1) |
                                       (DST_P0    << P_Dst0 ) |
                                       (DST_NoWr  << P_Dst1 ) |
                                       (DST_NoWr  << P_Dst2 ) |
                                       (DST_NoWr  << P_Dst3 ) ;
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
                                    (L_CODE     << P_Len       )|
                                    (0          << P_RdIncr    )|
                                    (0          << P_WrIncr    );
        WaitEngineDone();
);
#else
    for (i = 0; i < L_CODE; i++)
    {
       s = (p0[i] * _sign[0]);
       s += (p1[i] * _sign[1]);
       y[i] = (Word16)((s+0x00004000L)>>15);
    }
#endif

    return indx;
}
