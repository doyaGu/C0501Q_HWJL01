/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : hp_max.c
*      Purpose          : Find the maximum correlation of scal_sig[] in a given
*                         delay range.
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "hp_max.h"
const char hp_max_id[] = "@(#)$Id $" hp_max_h;

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

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
Word16 hp_max (
    Word32 corr[],      /* i   : correlation vector.                      */
    Word16 scal_sig[],  /* i   : scaled signal.                           */
    Word16 L_frame,     /* i   : length of frame to compute pitch         */
    Word16 lag_max,     /* i   : maximum lag                              */
    Word16 lag_min,     /* i   : minimum lag                              */
    Word16 *cor_hp_max) /* o   : max high-pass filtered norm. correlation */
{
    Word16 i;
    Word16 *p, *p1;
    Word32 max, t0, t1;
    Word16 max16, t016, cor_max;
    Word16 shift, shift1, shift2;

    max = MIN_32;
    t0 = 0L;

    for (i = lag_max-1; i > lag_min; i--)
    {
       /* high-pass filtering */
       //t0 = L_sub (L_sub(L_shl(corr[-i], 1), corr[-i-1]), corr[-i+1]);
       t0 = (corr[-i]<<1) - corr[-i-1] - corr[-i+1];

       //t0 = L_abs (t0);
       if ( t0 < 0 )
         t0 = -t0;

       //if (L_sub (t0, max) >= 0)
       if (t0 >= max)
       {
          max = t0;
       }
    }

    /* compute energy */
    p = scal_sig;
    p1 = &scal_sig[0];

#ifdef OPRISCASM
    asm volatile("l.macrc %0,0" : "=r" (t0));
    for (i = 0; i < L_frame; i++, p++, p1++)
    {
       //t0 = L_mac (t0, *p, *p1);
       //t0 += (*p * *p1);
       asm volatile("l.mac %0,%1" : : "r"((Word32)*p), "r"((Word32)*p1));
    }
    asm volatile("l.macrc %0,0" : "=r" (t0));
#else
    t0 = 0L;
    for (i = 0; i < L_frame; i++, p++, p1++)
       t0 += (*p * *p1);
#endif

    p = scal_sig;
    p1 = &scal_sig[-1];

#ifdef OPRISCASM
    for (i = 0; i < L_frame; i++, p++, p1++)
    {
       //t1 = L_mac (t1, *p, *p1);
       //t1 += (*p * *p1);
       asm volatile("l.mac %0,%1" : : "r"((Word32)*p), "r"((Word32)*p1));
    }
    asm volatile("l.macrc %0,0" : "=r" (t1));
#else
    t1 = 0L;
    for (i = 0; i < L_frame; i++, p++, p1++)
    {
       t1 += (*p * *p1);
    }
#endif
    /* high-pass filtering */
    //t0 = L_sub(L_shl(t0, 1), L_shl(t1, 1));
    t0 = (t0<<2) - (t1<<2);

    //t0 = L_abs (t0);
    if ( t0 < 0 )
      t0 = -t0;

    /* max/t0 */
    //shift1 = sub(norm_l(max), 1);
    shift1 = norm_l(max) - 1;

    //max16  = extract_h(L_shl(max, shift1));
    max16 = (Word16)( max>>(16-shift1) );

    shift2 = norm_l(t0);

    //t016 =  extract_h(L_shl(t0, shift2));
    t016 = (Word16)( t0>>(16-shift2));

    if (t016 != 0)
    {
       cor_max = div_s(max16, t016);
    }
    else
    {
       cor_max = 0;
    }

    //shift = sub(shift1, shift2);
    shift = shift1 - shift2;
    *cor_hp_max = shr(cor_max, shift);

   // if (shift >= 0)
   // {
       //*cor_hp_max = shr(cor_max, shift);           /* Q15 */
   //   *cor_hp_max = cor_max >> shift;          /* Q15 */

   // }
   // else
   // {
       //*cor_hp_max = shl(cor_max, negate(shift));   /* Q15 */
   //       *cor_hp_max = shl(cor_max, -shift);   /* Q15 */
       //*cor_hp_max = cor_max << -shift;  /* Q15 */
   // }

    return 0;
}
