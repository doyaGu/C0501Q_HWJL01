/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : autocorr.c
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "autocorr.h"
const char autocorr_id[] = "@(#)$Id $" autocorr_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
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
/*
**************************************************************************
*
*  Function    : autocorr
*  Purpose     : Compute autocorrelations of signal with windowing
*
**************************************************************************
*/
Word16 Autocorr (
    Word16 x[],            /* (i)    : Input signal (L_WINDOW)            */
    Word16 m,              /* (i)    : LPC order                          */
    Word16 r_h[],          /* (o)    : Autocorrelations  (msb)            */
    Word16 r_l[],          /* (o)    : Autocorrelations  (lsb)            */
    const Word16 wind[]    /* (i)    : window for LPC analysis (L_WINDOW) */
)
{
    Word32 i;
    Word32 y[L_WINDOW];
    Word32 sum;
    Word32 overfl;
    Word16 norm;

    //Word32 mac_hi,mac_lo;
    /* Windowing of signal */
#ifdef AUTOCORR_ENG1
VOLATILE (
     *(volatile int *)UsrDefC0 = (int)(0x00004000L);
     *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) wind;
     *(volatile int *)Src1Base = (FALSE << P_RdDec) | (int) x;
     *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) &wind[120]; // L_WINDOW/2
     *(volatile int *)Src3Base = (FALSE << P_RdDec) | (int) &x[120];    // L_WINDOW/2
     *(volatile int *)Dst0Base = (FALSE << P_RdDec) | (int) y;
     *(volatile int *)Dst2Base = (FALSE << P_RdDec) | (int) &y[120];    // L_WINDOW/2

     *(volatile int *)(ALU_OP0+4) =
                                    (FALSE     << P_BP   ) |
                                    (0x4       << P_SAdd2) |
                                    (0x4       << P_SAdd3) |
                                    (SRC_FIFOA << P_Mul0L) |
                                    (SRC_FIFOB << P_Mul0R) |
                                    (SRC_C1    << P_Mul1L) |
                                    (SRC_UDC0  << P_Mul1R) |
                                    (SRC_FIFOC << P_Mul2L) |
                                    (SRC_FIFOD << P_Mul2R) ;

    *(volatile int *)(ALU_OP0)   = (SRC_C1     << P_Mul3L) |
                                   (SRC_UDC0   << P_Mul3R) |
                                   (R_SHIFT15  << P_SAdd0) |
                                   (R_SHIFT15  << P_SAdd1) |
                                   (DST_P0     << P_Dst0 ) |
                                   (DST_NoWr   << P_Dst1 ) |
                                   (DST_P2     << P_Dst2 ) |
                                   (DST_NoWr   << P_Dst3 ) ;

    *(volatile int *)RQ_TYPE  = (TRUE       << P_Fire      )|
                                (FALSE      << P_EnINT     )|
                                (FALSE      << P_EnInitAcc )|
                                (TRUE       << P_Sat       )|
                                (DATA16     << P_RdDT      )|
                                (DATA32     << P_WrDT      )|
                                (LARGE      << P_RdGranSize)|
                                (GRANULE_4  << P_RdGranule )|
                                (LARGE      << P_WrGranSize)|
                                (GRANULE_2  << P_WrGranule )|
                                (120        << P_Len       )|  //L_WINDOW/2
                                (0          << P_RdIncr    )|
                                (0          << P_WrIncr    );
    WaitEngineDone();
 );

#elif defined AUTOCORR_PUREC1
    for (i = 0; i < L_WINDOW; i++)
    {
        //y[i] = mult_r (x[i], wind[i]);
        sum = (Word32)0x00004000L + x[i] * wind[i];
    y[i] = (sum >> 15);
    }
#else
#error "error"
#endif
    /* Compute r[0] and test for overflow */

    //overfl_shft = 0;
#ifdef AUTOCORR_OPRISC2
    asm volatile("l.macrc %0,0" : "=r" (sum));
#endif
    do
    {
        overfl = 0;
#ifdef AUTOCORR_ENG2
VOLATILE (
        *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) y;
        *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) y;
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
                                    (NoSHIFT    << P_SAdd0) |
                                    (NoSHIFT    << P_SAdd1) |
                                    (DST_NoWr   << P_Dst0 ) |
                                    (DST_NoWr   << P_Dst1 ) |
                                    (DST_NoWr   << P_Dst2 ) |
                                    (DST_NoWr   << P_Dst3 ) ;

        *(volatile int *)RQ_TYPE  = (TRUE       << P_Fire      )|
                                    (FALSE      << P_EnINT     )|
                                    (FALSE      << P_EnInitAcc )|
                                    (TRUE       << P_Sat       )|
                                    (DATA32     << P_RdDT      )|
                                    (DATA32     << P_WrDT      )|
                                    (LARGE      << P_RdGranSize)|
                                    (GRANULE_2  << P_RdGranule )|
                                    (LARGE      << P_WrGranSize)|
                                    (GRANULE_2  << P_WrGranule )|
                                    (L_WINDOW   << P_Len       )|
                                    (0          << P_RdIncr    )|
                                    (0          << P_WrIncr    );
        WaitEngineDone();
        sum = *(volatile int *)ALU_P0;
);
        if ( sum >= 0x3fffffffL )
        {
           sum = 0x7ffffffeL;
#elif defined AUTOCORR_OPRISC2
        for (i = 0; i < L_WINDOW; i++)
        {
            // sis3830 L_mac overflow
            //sum = L_mac (sum, y[i], y[i]);
            asm volatile("l.mac %0,%1" : : "r"(y[i]), "r"(y[i]));
        }
        asm volatile("l.macrc %0,0" : "=r" (sum));
        if ( sum >= 0x3fffffffL )
        {
           sum = 0x7ffffffeL;
#elif defined AUTOCORR_PUREC2
        sum = 0L;
        for (i = 0; i < L_WINDOW; i++)
        {
            sum += (y[i] * y[i]);
            if ( sum > (Word32)0x3fffffffL )
                sum = 0x3fffffffL;
        }
        sum <<=1;
        if ( sum == 0x7ffffffeL )
        {
#else
#error "error"
#endif

            //overfl_shft = add (overfl_shft, 4);
            //overfl_shft = overfl_shft + 4;
            overfl = 1; /* Set the overflow flag */

            for (i = 0; i < L_WINDOW; i++)
            {
                //y[i] = shr (y[i], 2);
        y[i] = y[i] >> 2;
            }
        }
    }
    while (overfl != 0);

    //sum = L_add (sum, 1L);             /* Avoid the case of all zeros */
    sum = sum + 1;

    /* Normalization of r[0] */

    norm = norm_l (sum);
    //sum = L_shl (sum, norm);
    sum = sum << norm;
    //L_Extract (sum, &r_h[0], &r_l[0]); /* Put in DPF format (see oper_32b) */
    r_h[0] = (Word16)(sum>>16);
    r_l[0] = (Word16)( (sum-(Word32)(r_h[0]<<16))>>1 );

    /* r[1] to r[m] */

    for (i = 1; i <= m; i++)
    {
#ifdef AUTOCORR_ENG3
VOLATILE (
        *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) y;
        *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) &y[i];
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
                                    (0          << P_Dst0 ) |
                                    (0          << P_Dst1 ) |
                                    (0          << P_Dst2 ) |
                                    (0          << P_Dst3 ) ;

        *(volatile int *)RQ_TYPE  = (TRUE        << P_Fire      )|
                                    (FALSE       << P_EnINT     )|
                                    (FALSE       << P_EnInitAcc )|
                                    (TRUE        << P_Sat       )|
                                    (DATA32      << P_RdDT      )|
                                    (DATA32      << P_WrDT      )|
                                    (LARGE       << P_RdGranSize)|
                                    (GRANULE_2   << P_RdGranule )|
                                    (LARGE       << P_WrGranSize)|
                                    (GRANULE_2   << P_WrGranule )|
                                    ((L_WINDOW-i)<< P_Len      )|
                                    (0           << P_RdIncr    )|
                                    (0           << P_WrIncr    );
        WaitEngineDone();
        sum = *(volatile int *)ALU_P0;
);
#elif defined AUTOCORR_OPRISC3
        Word32 j;
        asm volatile("l.macrc %0,0" : "=r" (sum));
        for (j = 0; j < L_WINDOW - i; j++)
        {
            //sum = L_mac (sum, y[j], y[j + i]);
            asm volatile("l.mac %0,%1" : : "r"(y[j]), "r"(y[j+i]));
        }
        //asm volatile("l.mfspr %0, r0, 0x2802" : "=r"(mac_hi));
        //asm volatile("l.mfspr %0, r0, 0x2801" : "=r"(mac_lo));
        asm volatile("l.macrc %0,0" : "=r" (sum));
        sum <<=1;
#elif defined AUTOCORR_PUREC3
        Word32 j;
        sum = 0;
        for (j = 0; j < L_WINDOW - i; j++)
        {

            sum += (y[j] * y[j + i]);
        }
        sum <<=1;
#else
#error "error"
#endif
        //sum = L_shl (sum, norm);
    sum = sum << norm;
        //L_Extract (sum, &r_h[i], &r_l[i]);
        r_h[i] = (Word16)(sum>>16);
    r_l[i] = (Word16)( (sum-(Word32)(r_h[i]<<16))>>1 );
    }

    return norm;
}
