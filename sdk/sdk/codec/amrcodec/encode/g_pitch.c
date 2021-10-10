/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : g_pitch.c
*      Purpose          : Compute the pitch (adaptive codebook) gain.
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "g_pitch.h"
const char g_pitch_id[] = "@(#)$Id $" g_pitch_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
#include "mode.h"
#include "basic_op.h"
#include "oper_32b.h"
//#include "count.h"
#include "cnst.h"

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
/*************************************************************************
 *
 *  FUNCTION:  G_pitch
 *
 *  PURPOSE:  Compute the pitch (adaptive codebook) gain.
 *            Result in Q14 (NOTE: 12.2 bit exact using Q12)
 *
 *  DESCRIPTION:
 *      The adaptive codebook gain is given by
 *
 *              g = <x[], y[]> / <y[], y[]>
 *
 *      where x[] is the target vector, y[] is the filtered adaptive
 *      codevector, and <> denotes dot product.
 *      The gain is limited to the range [0,1.2] (=0..19661 Q14)
 *
 *************************************************************************/
Word16 G_pitch     (    /* o : Gain of pitch lag saturated to 1.2       */
    enum Mode mode,     /* i : AMR mode                                 */
    Word16 xn[],        /* i : Pitch target.                            */
    Word16 y1[],        /* i : Filtered adaptive codebook.              */
    Word16 g_coeff[],   /* i : Correlations need for gain quantization  */
    Word16 L_subfr      /* i : Length of subframe.                      */
)
{
    Word32 i;
    Word16 xy, yy, exp_xy, exp_yy, gain;
    Word32 s;

    Word16 scaled_y1[L_SUBFR];
    Overflow = 0;
#ifdef GPITCH_ENG1

VOLATILE (
  *(volatile int *)(ALU_OP0+4) = (FALSE     << P_BP   ) |
                                 (0x4       << P_SAdd2) |
                                 (0x4       << P_SAdd3) |
                                 (SRC_FIFOA << P_Mul0L) |
                                 (SRC_C1    << P_Mul0R) |
                                 (SRC_C0    << P_Mul1L) |
                                 (SRC_C0    << P_Mul1R) |
                                 (SRC_C0    << P_Mul2L) |
                                 (SRC_C0    << P_Mul2R) ;

  *(volatile int *)(ALU_OP0)   = (SRC_C0    << P_Mul3L) |
                                 (SRC_C0    << P_Mul3R) |
                                 (R_SHIFT2  << P_SAdd0) |
                                 (NoSHIFT   << P_SAdd1) |
                                 (DST_P0    << P_Dst0 ) |
                                 (DST_NoWr  << P_Dst1 ) |
                                 (DST_NoWr  << P_Dst2 ) |
                                 (DST_NoWr  << P_Dst3 ) ;

  *(volatile int *)(Src0Base) = (FALSE << P_RdDec) | ((int) y1);
  *(volatile int *)(Dst0Base) = (FALSE << P_RdDec) | ((int) scaled_y1);

  *(volatile int *)(RQ_TYPE)  = (TRUE      << P_Fire      ) |
                                (FALSE     << P_EnInitAcc ) |
                                (FALSE     << P_Sat       ) |
                                (DATA16    << P_RdDT      ) |
                                (DATA16    << P_WrDT      ) |
                                (LARGE     << P_RdGranSize) |
                                (GRANULE_1 << P_RdGranule ) |
                                (LARGE     << P_WrGranSize) |
                                (GRANULE_1 << P_WrGranule ) |
                                (L_subfr   << P_Len       ) |
                                (0         << P_RdIncr    ) |
                                (0         << P_WrIncr    ) ;

  WaitEngineDone();
  );
#elif defined GPITCH_PUREC1

    for (i = 0; i < L_subfr; i++) {
        scaled_y1[i] = y1[i] >> 2;
    }
    /* Q12 scaling / MR122 */
#else
#error "error"
#endif

#ifdef GPITCH_ENG2
VOLATILE (
  *(volatile int *)(ALU_OP0+4) = (FALSE     << P_BP   ) |
                                 (0x3       << P_SAdd2) |
                                 (0x3       << P_SAdd3) |
                                 (SRC_FIFOA << P_Mul0L) |
                                 (SRC_FIFOC << P_Mul0R) |
                                 (SRC_C0    << P_Mul1L) |
                                 (SRC_C0    << P_Mul1R) |
                                 (SRC_C0    << P_Mul2L) |
                                 (SRC_C0    << P_Mul2R) ;

  *(volatile int *)(ALU_OP0)   = (SRC_C0    << P_Mul3L) |
                                 (SRC_C0    << P_Mul3R) |
                                 (L_SHIFT1  << P_SAdd0) |
                                 (L_SHIFT1  << P_SAdd1) |
                                 (DST_NoWr  << P_Dst0 ) |
                                 (DST_NoWr  << P_Dst1 ) |
                                 (DST_NoWr  << P_Dst2 ) |
                                 (DST_NoWr  << P_Dst3 ) ;

  *(volatile int *)(Src0Base) = (FALSE << P_RdDec) | ((int) y1);
  *(volatile int *)(Src2Base) = (FALSE << P_RdDec) | ((int) y1);

  *(volatile int *)(RQ_TYPE)  = (TRUE      << P_Fire      ) |
                                (FALSE     << P_EnInitAcc ) |
                                (TRUE      << P_Sat       ) |
                                (DATA16    << P_RdDT      ) |
                                (DATA32    << P_WrDT      ) |
                                (LARGE     << P_RdGranSize) |
                                (GRANULE_2 << P_RdGranule ) |
                                (LARGE     << P_WrGranSize) |
                                (GRANULE_1 << P_WrGranule ) |
                                (L_subfr   << P_Len       ) |
                                (0         << P_RdIncr    ) |
                                (0         << P_WrIncr    ) ;

  WaitEngineDone();
  s = *(volatile int *)ALU_P0;
  if(s>0x7ffffffeL)
    Overflow = 1;

  );
#elif defined GPITCH_PUREC2
    s = 0L;                         /* Avoid case of all zeros */
    for (i = 0; i < L_subfr; i++) {
        s += (y1[i] * y1[i]);
        if ( s > (Word32)0x3fffffffL || s < (Word32)0xc0000000L ) {
           Overflow = 1;
           break;
        }
    }
    s <<= 1;
#else
#error "error"
#endif

    s++;
    if (Overflow == 0) {      /* Test for overflow */
        exp_yy = norm_l (s);
        s <<= exp_yy;
        if ( s > (Word32)0x7fff7fffL )
           s = 0x7fff7fffL;
        yy = (Word16)( (s+0x00008000L)>>16 );
    } else {
#ifdef GPITCH_ENG3
        *(volatile int *)(Src0Base) = (FALSE << P_RdDec) | ((int) scaled_y1);
        *(volatile int *)(Src2Base) = (FALSE << P_RdDec) | ((int) scaled_y1);

        *(volatile int *)(RQ_TYPE)  = (TRUE      << P_Fire      ) |
                                      (FALSE     << P_EnInitAcc ) |
                                      (TRUE      << P_Sat       ) |
                                      (DATA16    << P_RdDT      ) |
                                      (DATA32    << P_WrDT      ) |
                                      (LARGE     << P_RdGranSize) |
                                      (GRANULE_2 << P_RdGranule ) |
                                      (LARGE     << P_WrGranSize) |
                                      (GRANULE_1 << P_WrGranule ) |
                                      (L_subfr   << P_Len       ) |
                                      (0         << P_RdIncr    ) |
                                      (0         << P_WrIncr    ) ;

        WaitEngineDone();
        s = *(volatile int *)ALU_P0;

#elif defined GPITCH_PUREC3

        s = 0L;
        for (i = 0; i < L_subfr; i++) {
            s += (scaled_y1[i] * scaled_y1[i]);
        }
        s <<= 1;
#else
#error "error"
#endif
        s ++;       // avoid zero
        exp_yy = norm_l (s);
        s <<= exp_yy;
        yy = (Word16)( (s+0x00008000L)>>16 );
        exp_yy -= 4;
    }

    /* Compute scalar product <xn[],y1[]> */
    Overflow = 0;

#ifdef GPITCH_ENG4
    *(volatile int *)(Src0Base) = (FALSE << P_RdDec) | ((int) xn);
    *(volatile int *)(Src2Base) = (FALSE << P_RdDec) | ((int) y1);

    *(volatile int *)(RQ_TYPE)  = (TRUE      << P_Fire      ) |
                                  (FALSE     << P_EnInitAcc ) |
                                  (TRUE      << P_Sat       ) |
                                  (DATA16    << P_RdDT      ) |
                                  (DATA32    << P_WrDT      ) |
                                  (LARGE     << P_RdGranSize) |
                                  (GRANULE_2 << P_RdGranule ) |
                                  (LARGE     << P_WrGranSize) |
                                  (GRANULE_1 << P_WrGranule ) |
                                  (L_subfr   << P_Len       ) |
                                  (0         << P_RdIncr    ) |
                                  (0         << P_WrIncr    ) ;

    WaitEngineDone();
    s = *(volatile int *)ALU_P0;
    if( s > (Word32)0x7ffffffeL || s < (Word32)0x80000000L )
      Overflow = 1;
#elif defined GPITCH_PUREC4
    s = 0L;
    for (i = 0; i < L_subfr; i++) {
        s += (xn[i] * y1[i]);
        if ( s > (Word32)0x3fffffffL || s < (Word32)0xc0000000L ) {
           Overflow = 1;
           break;
        }
    }
    s <<= 1;
#else
#error "error"
#endif

    s ++;       /* Avoid case of all zeros */
    if (Overflow == 0)
    {
        exp_xy = norm_l (s);
        s <<= exp_xy;
        if ( s > (Word32)0x7fff7fffL )
           s = 0x7fff7fffL;
        xy = (Word16)( (s+0x00008000L)>>16 );
    } else {
#ifdef GPITCH_ENG5
        *(volatile int *)(Src0Base) = (FALSE << P_RdDec) | ((int) xn);
        *(volatile int *)(Src2Base) = (FALSE << P_RdDec) | ((int) scaled_y1);

        *(volatile int *)(RQ_TYPE)  = (TRUE      << P_Fire      ) |
                                      (FALSE     << P_EnInitAcc ) |
                                      (TRUE      << P_Sat       ) |
                                      (DATA16    << P_RdDT      ) |
                                      (DATA32    << P_WrDT      ) |
                                      (LARGE     << P_RdGranSize) |
                                      (GRANULE_2 << P_RdGranule ) |
                                      (LARGE     << P_WrGranSize) |
                                      (GRANULE_1 << P_WrGranule ) |
                                      (L_subfr   << P_Len       ) |
                                      (0         << P_RdIncr    ) |
                                      (0         << P_WrIncr    ) ;

        WaitEngineDone();
        s = *(volatile int *)ALU_P0;

#elif defined GPITCH_PUREC5
        s = 0L;
        for (i = 0; i < L_subfr; i++) {
            s += (xn[i] * scaled_y1[i]);
        }
        s <<= 1;
#else
#error "error"
#endif
        s ++;       // Avoid case of all zeros
        exp_xy = norm_l (s);
        s <<= exp_xy;
        xy = (Word16)( (s+0x00008000L)>>16 );
        exp_xy -= 2;
    }

    g_coeff[0] = yy;
    g_coeff[1] = 15 - exp_yy;
    g_coeff[2] = xy;
    g_coeff[3] = 15 - exp_xy;

    /* If (xy < 4) gain = 0 */

    i = xy - 4;

    if (i < 0)
        return ((Word16) 0);

    /* compute gain = xy/yy */

    xy >>= 1;                  /* Be sure xy < yy */
    gain = div_s (xy, yy);

    i = exp_xy - exp_yy;      /* Denormalization of division */
#ifdef GPITCH_INLINE
    if(i<0) {
        i = -i;
        s = ((Word32) gain) << i;
        if ((i > 15 && gain != 0) || (s != (Word32) ((Word16) s))) {
            gain = (gain > 0) ? MAX_16 : MIN_16;
        } else {
            gain = (Word16) s;
        }
    } else {
        if (i >= 15)
        {
            gain = (gain < 0) ? -1 : 0;
        } else {
            gain = gain >> i;
        }
    }
#else
    gain = shr (gain, i);
#endif

    /* if(gain >1.2) gain = 1.2 */

    if (gain > 19661) {
        gain = 19661;
    }

    if ((Word16)mode == MR122) {
        /* clear 2 LSBits */
        gain = gain & 0xfffC;
    }

    return (gain);
}
