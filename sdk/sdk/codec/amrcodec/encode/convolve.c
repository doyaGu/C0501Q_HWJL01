/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : convolve.c
*      Purpose          : Perform the convolution between two vectors x[]
*                       : and h[] and write the result in the vector y[].
*                       : All vectors are of length L and only the first
*                       : L samples of the convolution are computed.
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "convolve.h"
const char convolve_id[] = "@(#)$Id $" convolve_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
//#include "basic_op.h"
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
 *  FUNCTION:   Convolve
 *
 *  PURPOSE:
 *     Perform the convolution between two vectors x[] and h[] and
 *     write the result in the vector y[]. All vectors are of length L
 *     and only the first L samples of the convolution are computed.
 *
 *  DESCRIPTION:
 *     The convolution is given by
 *
 *          y[n] = sum_{i=0}^{n} x[i] h[n-i],        n=0,...,L-1
 *
 *************************************************************************/
void Convolve (
    Word16 x[],        /* (i)     : input vector                           */
    Word16 h[],        /* (i)     : impulse response                       */
    Word16 y[],        /* (o)     : output vector                          */
    Word16 L           /* (i)     : vector size                            */
)
{
    Word32 n;
    Word32 s;
#ifdef CONVOLVE_ENG

VOLATILE (
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

    *(volatile int *)(ALU_OP0)   = (SRC_C0     << P_Mul3L) |
                                   (SRC_C0     << P_Mul3R) |
                                   (NoSHIFT    << P_SAdd0) |
                                   (NoSHIFT    << P_SAdd1) |
                                   (DST_NoWr   << P_Dst0 ) |
                                   (DST_NoWr   << P_Dst1 ) |
                                   (DST_NoWr   << P_Dst2 ) |
                                   (DST_NoWr   << P_Dst3 ) ;
    *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) x;
    for (n = 0; n < L; n++)
    {

       *(volatile int *)Src2Base = (TRUE  << P_RdDec) | (int) &h[n];

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
                                   ((n+1)      << P_Len       )| //remove
                                   (0          << P_RdIncr    )|
                                   (0          << P_WrIncr    );
       WaitEngineDone();
       s = *(volatile int *)ALU_P0;
       y[n] = (Word16)( s>>12 );
    }
);

#elif defined CONVOLVE_OPRISC
    Word32 i;
    asm volatile("l.macrc %0,0" : "=r" (s));
    for (n = 0; n < L; n++)
    {
        //s = 0;
        for (i = 0; i <= n; i++)
        {
            //s += (x[i] * h[n - i]);
            asm volatile("l.mac %0,%1" : : "r"((Word32)x[i]), "r"((Word32)h[n - i]));
        }
        asm volatile("l.macrc %0,0" : "=r" (s));

    y[n] = (Word16)( s>>12 );
    }
#elif defined CONVOLVE_PUREC
    Word32 i;
    for (n = 0; n < L; n++)
    {
       s = 0;
       for (i = 0; i <= n; i++)
        {
            s += (x[i] * h[n - i]);
        }
       y[n] = (Word16)( s>>12 );
    }
#else
#error "error"
#endif

    return;
}
