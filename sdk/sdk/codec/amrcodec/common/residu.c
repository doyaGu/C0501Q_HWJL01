/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : residu.c
*      Purpose          : Computes the LP residual.
*      Description      : The LP residual is computed by filtering the input
*                       : speech through the LP inverse filter A(z).
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "residu.h"
const char residu_id[] = "@(#)$Id $" residu_h;

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
*--------------------------------------*
* Constants (defined in cnst.h         *
*--------------------------------------*
*  M         : LPC order               *
*--------------------------------------*
*/

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
void Residu_de (
    Word16 a[], /* (i)     : prediction coefficients                      */
    Word16 x[], /* (i)     : speech signal                                */
    Word16 y[], /* (o)     : residual signal                              */
    Word16 lg   /* (i)     : size of filtering                            */
)
{
    Word32 i,j;
    Word32 s1,s2,s3,s4;

    i = 0;
    do
    {
        //s = 0;
        //for (j = 0; j <= M; j++)
        //    s += a[j] * x[i - j];
/*
#ifdef OPRISCASM
        asm ("l.mac %0, %1" : : "r"((Word32)a[ 0]), "r"((Word32)x[i]));
        asm ("l.mac %0, %1" : : "r"((Word32)a[ 1]), "r"((Word32)x[i - 1]));
        asm ("l.mac %0, %1" : : "r"((Word32)a[ 2]), "r"((Word32)x[i - 2]));
        asm ("l.mac %0, %1" : : "r"((Word32)a[ 3]), "r"((Word32)x[i - 3]));
        asm ("l.mac %0, %1" : : "r"((Word32)a[ 4]), "r"((Word32)x[i - 4]));
        asm ("l.mac %0, %1" : : "r"((Word32)a[ 5]), "r"((Word32)x[i - 5]));
        asm ("l.mac %0, %1" : : "r"((Word32)a[ 6]), "r"((Word32)x[i - 6]));
        asm ("l.mac %0, %1" : : "r"((Word32)a[ 7]), "r"((Word32)x[i - 7]));
        asm ("l.mac %0, %1" : : "r"((Word32)a[ 8]), "r"((Word32)x[i - 8]));
        asm ("l.mac %0, %1" : : "r"((Word32)a[ 9]), "r"((Word32)x[i - 9]));
        asm ("l.mac %0, %1" : : "r"((Word32)a[10]), "r"((Word32)x[i -10]));
        //asm volatile("l.mfspr %0, r0, 0x2802" : "=r"(mac_hi));
        asm ("l.mac %0, %1" : : "r"((Word32)1), "r"((Word32)0x00000800L));
        asm volatile("l.macrc %0, 12" : "=r" (y[i]));
#else
        s =  a[ 0] * x[i];
        s += a[ 1] * x[i - 1];
        s += a[ 2] * x[i - 2];
        s += a[ 3] * x[i - 3];
        s += a[ 4] * x[i - 4];
        s += a[ 5] * x[i - 5];
        s += a[ 6] * x[i - 6];
        s += a[ 7] * x[i - 7];
        s += a[ 8] * x[i - 8];
        s += a[ 9] * x[i - 9];
        s += a[10] * x[i -10];

        y[i] = (Word16)( ( s + 0x00000800L) >> 12 );

#endif
*/

        s1 = a[0]*x[i];
        s2 = a[0]*x[i+1];
        s3 = a[0]*x[i+2];
        s4 = a[0]*x[i+3];
        for(j=1;j<=M;j+=5)
        {
            s1+=a[j+4] * x[i - j-4];

            s1+=a[j+3] * x[i - j-3];
            s2+=a[j+4] * x[i - j-3];

            s1+=a[j+2] * x[i - j-2];
            s2+=a[j+3] * x[i - j-2];
            s3+=a[j+4] * x[i - j-2];

            s1+=a[j+1] * x[i - j-1];
            s2+=a[j+2] * x[i - j-1];
            s3+=a[j+3] * x[i - j-1];
            s4+=a[j+4] * x[i - j-1];

            s1+=a[j+0] * x[i - j];
            s2+=a[j+1] * x[i - j];
            s3+=a[j+2] * x[i - j];
            s4+=a[j+3] * x[i - j];

            s2+=a[j+0] * x[i - j+1];
            s3+=a[j+1] * x[i - j+1];
            s4+=a[j+2] * x[i - j+1];

            s3+=a[j+0] * x[i - j+2];
            s4+=a[j+1] * x[i - j+2];

            s4+=a[j+0] * x[i - j+3];
        }
        y[i]   = (Word16)( ( s1 + 0x00000800L) >> 12 );
        y[i+1] = (Word16)( ( s2 + 0x00000800L) >> 12 );
        y[i+2] = (Word16)( ( s3 + 0x00000800L) >> 12 );
        y[i+3] = (Word16)( ( s4 + 0x00000800L) >> 12 );

        i = i + 4;
    } while (i < lg);
    return;
}

void Residu (
    Word16 a[], /* (i)     : prediction coefficients                      */
    Word16 x[], /* (i)     : speech signal                                */
    Word16 y[], /* (o)     : residual signal                              */
    Word16 lg   /* (i)     : size of filtering                            */
)
{
    Word32 i;
    Word32 s1;

    //asm volatile("l.macrc %0,0" : "=r" (j));
#ifdef RESIDU_ENG
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
    *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) a;
    for (i = 0; i < lg; i++)
    {

       *(volatile int *)Src2Base = (TRUE       << P_RdDec)     | (int) &x[i];
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
                                   (11         << P_Len       )| //remove
                                   (0          << P_RdIncr    )|
                                   (0          << P_WrIncr    );
       WaitEngineDone();
       s1 = *(volatile int *)ALU_P0;
#ifdef NO_ROUNDING
       y[i] = (Word16)(s1>>12);
#else
       y[i] = (Word16)( ( s1 + 0x00000800L) >> 12 );
#endif
    }
);
#elif defined RESIDU_PUREC
    Word32 s2,s3,s4,j;
    for (i = 0; i < lg; i+=4)
    {
        //s = 0;
        s1 = a[0]*x[i];
        s2 = a[0]*x[i+1];
        s3 = a[0]*x[i+2];
        s4 = a[0]*x[i+3];
        /*for (j = 0; j <= M; j++)
           asm volatile("l.mac %0,%1" : : "r"((Word32)a[j]), "r"((Word32)x[i - j]));
           // s += a[j] * x[i - j];
        asm volatile("l.mac %0,%1" : : "r"(0x00000800L), "r"(1));
        asm volatile("l.macrc %0,12" : "=r" (y[i]));
    //      y[i] = (Word16)( ( s + 0x00000800L) >> 12 );
        */
        for(j=1;j<=M;j+=5)
        {
           s1+=a[j+4] * x[i - j-4];

           s1+=a[j+3] * x[i - j-3];
           s2+=a[j+4] * x[i - j-3];

           s1+=a[j+2] * x[i - j-2];
           s2+=a[j+3] * x[i - j-2];
           s3+=a[j+4] * x[i - j-2];

           s1+=a[j+1] * x[i - j-1];
           s2+=a[j+2] * x[i - j-1];
           s3+=a[j+3] * x[i - j-1];
           s4+=a[j+4] * x[i - j-1];

           s1+=a[j+0] * x[i - j];
           s2+=a[j+1] * x[i - j];
           s3+=a[j+2] * x[i - j];
           s4+=a[j+3] * x[i - j];

           s2+=a[j+0] * x[i - j+1];
           s3+=a[j+1] * x[i - j+1];
           s4+=a[j+2] * x[i - j+1];

           s3+=a[j+0] * x[i - j+2];
           s4+=a[j+1] * x[i - j+2];

           s4+=a[j+0] * x[i - j+3];

        }
#ifdef NO_ROUNDING
       y[i]   = (Word16)(s1>>12);
       y[i+1] = (Word16)(s2>>12);
       y[i+2] = (Word16)(s3>>12);
       y[i+3] = (Word16)(s4>>12);
#else
       y[i] = (Word16)( ( s1 + 0x00000800L) >> 12 );
       y[i+1] = (Word16)( ( s2 + 0x00000800L) >> 12 );
       y[i+2] = (Word16)( ( s3 + 0x00000800L) >> 12 );
       y[i+3] = (Word16)( ( s4 + 0x00000800L) >> 12 );
#endif
    }
#else
#error "error"
#endif
    return;
}
