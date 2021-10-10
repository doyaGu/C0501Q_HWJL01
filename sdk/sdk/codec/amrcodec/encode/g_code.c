/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : g_code.c
*      Purpose          : Compute the innovative codebook gain.
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "g_code.h"
const char g_code_id[] = "@(#)$Id $" g_code_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "basic_op.h"
//#include "count.h"
#include "cnst.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
/*************************************************************************
 *
 *  FUNCTION:   G_code
 *
 *  PURPOSE:  Compute the innovative codebook gain.
 *
 *  DESCRIPTION:
 *      The innovative codebook gain is given by
 *
 *              g = <x[], y[]> / <y[], y[]>
 *
 *      where x[] is the target vector, y[] is the filtered innovative
 *      codevector, and <> denotes dot product.
 *
 *************************************************************************/
Word16 G_code (         /* out   : Gain of innovation code         */
    Word16 xn2[],       /* in    : target vector                   */
    Word16 y2[]         /* in    : filtered innovation vector      */
)
{
    Word16 i;
    Word16 xy, yy, exp_xy, exp_yy, gain;
    Word16 scal_y2[L_SUBFR];
    Word32 s;

    /* Scale down Y[] by 2 to avoid overflow */

    for (i = 0; i < L_SUBFR; i++)
    {
        //scal_y2[i] = shr (y2[i], 1);
        scal_y2[i] = y2[i] >> 1;
    }

    /* Compute scalar product <X[],Y[]> */

    //s = 1L;                            /* Avoid case of all zeros */
#ifdef GCODE_ENG1
VOLATILE (
    *(volatile int *)INIT_ACC = 1;
    *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) xn2;
    *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) scal_y2;
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
                                    (TRUE       << P_EnInitAcc )|
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

#elif defined GCODE_OPRISC1
    asm volatile("l.macrc %0,0" : "=r" (s));
    for (i = 0; i < L_SUBFR; i++)
    {
        //s = L_mac (s, xn2[i], scal_y2[i]);
        asm volatile("l.mac %0,%1" : : "r"((Word32)xn2[i]), "r"((Word32)scal_y2[i]));
    }
    asm volatile("l.maclc %0,1" : "=r" (s));
    //s= (s<<1)+1;
    s++;
#elif defined GCODE_PUREC1
    s = 0;
    for (i = 0; i < L_SUBFR; i++)
        s += (xn2[i] * scal_y2[i]);
    s <<= 1;
    s++;
#else
#error "error"
#endif

    exp_xy = norm_l (s);

    //xy = extract_h (L_shl (s, exp_xy));
    if ( exp_xy < 16 )
      xy = (Word16)( s>>(16-exp_xy) );
    else
      xy = (Word16)( s<<(exp_xy-16) );

    /* If (xy < 0) gain = 0  */

    if (xy <= 0)
        return ((Word16) 0);

    /* Compute scalar product <Y[],Y[]> */

    //s = 0L;

#ifdef GCODE_ENG2
VOLATILE (
        *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) scal_y2;
        *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) scal_y2;
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

#elif defined GCODE_OPRISC2
    for (i = 0; i < L_SUBFR; i++)
    {
        //s = L_mac (s, scal_y2[i], scal_y2[i]);
        asm volatile("l.mac %0,%1" : : "r"((Word32)scal_y2[i]), "r"((Word32)scal_y2[i]));
    }
    asm volatile("l.maclc %0,1" : "=r" (s));

#elif defined GCODE_PUREC2
    s = 0;
    for (i = 0; i < L_SUBFR; i++)
        s += (scal_y2[i] * scal_y2[i]);
    s <<= 1;
#endif
    //s<<=1;
    exp_yy = norm_l (s);

    //yy = extract_h (L_shl (s, exp_yy));
    if ( exp_yy < 16 )
       yy = (Word16)( s>>(16-exp_yy) );
    else
       yy = (Word16)( s<<(exp_yy-16) );

    /* compute gain = xy/yy */

    //xy = shr (xy, 1);                 /* Be sure xy < yy */
    xy>>=1;

    gain = div_s (xy, yy);

    /* Denormalization of division */
    //i = add (exp_xy, 5);              /* 15-1+9-18 = 5 */
    i = exp_xy + 5;

    //i = sub (i, exp_yy);
    i -= exp_yy;

    //gain = shl (shr (gain, i), 1);    /* Q0 -> Q1 */
    //gain = gain>>(i-1);    /* Q0 -> Q1 */
    if((i-1)>0)
       gain >>=(i-1);
    else
       gain <<=(-(i-1));
    return (gain);
}
