/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : calc_cor.c
*      Purpose          : Calculate all correlations for prior the OL LTP
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "calc_cor.h"
const char calc_cor_id[] = "@(#)$Id $" calc_cor_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
//#include <stdio.h>
//#include <stdlib.h>
#include "typedef.h"
#include "basic_op.h"
//#include "oper_32b.h"
//#include "count.h"
#include "cnst.h"

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
/*************************************************************************
 *
 *  FUNCTION: comp_corr
 *
 *  PURPOSE: Calculate all correlations of scal_sig[] in a given delay
 *           range.
 *
 *  DESCRIPTION:
 *      The correlation is given by
 *           cor[t] = <scal_sig[n],scal_sig[n-t]>,  t=lag_min,...,lag_max
 *      The functions outputs the all correlations
 *
 *************************************************************************/
void comp_corr (
    Word16 scal_sig[],  /* i   : scaled signal.                          */
    Word16 L_frame,     /* i   : length of frame to compute pitch        */
    Word16 lag_max,     /* i   : maximum lag                             */
    Word16 lag_min,     /* i   : minimum lag                             */
    Word32 corr[])      /* o   : correlation of selected lag             */
{
    //Word32 i, j;
    //Word16 *p, *p1;
    //Word32 t1,t2,t3,t4;
    //Word32 mac_hi,mac_lo;

    //asm volatile("l.macrc %0,0" : "=r" (t0));
#ifdef CALCCOR_ENG
  Word32 i;
  //Word32 tmp1,tmp2;
VOLATILE (
  *(volatile int *)(ALU_OP0+4) =
                                 (FALSE     << P_BP   ) |
                                 (0x3       << P_SAdd2) |
                                 (0x3       << P_SAdd3) |
                                 (SRC_FIFOA << P_Mul0L) |
                                 (SRC_FIFOB << P_Mul0R) |
                                 (SRC_C0    << P_Mul1L) |
                                 (SRC_C0    << P_Mul1R) |
                                 (SRC_FIFOC << P_Mul2L) |
                                 (SRC_FIFOD << P_Mul2R) ;

  *(volatile int *)(ALU_OP0)   = (SRC_C0    << P_Mul3L) |
                                 (SRC_C0    << P_Mul3R) |
                                 (L_SHIFT1  << P_SAdd0) |
                                 (L_SHIFT1  << P_SAdd1) |
                                 (DST_NoWr  << P_Dst0 ) |
                                 (DST_NoWr  << P_Dst1 ) |
                                 (DST_NoWr  << P_Dst2 ) |
                                 (DST_NoWr  << P_Dst3 ) ;
 *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) scal_sig;
 *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) scal_sig;

    for (i = lag_max; i >= lag_min; i-=4) // for performance optimum
    {

        *(volatile int *)Src1Base = (FALSE << P_RdDec) | (int) (&scal_sig[-i]);
        *(volatile int *)Src3Base = (FALSE << P_RdDec) | (int) (&scal_sig[-i+2]); // for performance optimum

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
                                    (L_frame    << P_Len       )|
                                    (0          << P_RdIncr    )|
                                    (0          << P_WrIncr    );

        WaitEngineDone();
        corr[-i]   = *(volatile int *)ALU_P0;
        corr[-i+2] = *(volatile int *)ALU_P2;   // for performance optimum
);
    //tmp1   = *(volatile int *)ALU_P0;
        //tmp2   = *(volatile int *)ALU_P2;
    //corr[-i] = tmp1;
    //corr[-i+1] = tmp2;

    }
#elif defined CALCCOR_PUREC
    Word32 i, j;
    Word32 t1,t2,t3,t4;

    for (i = lag_max; i >= lag_min; i-=8)
    {
       //p = scal_sig;
       //p1 = &scal_sig[-i];

       t1 = 0;
       t2 = 0;
       t3 = 0;
       t4 = 0;

       for (j = 0; j < L_frame; j+=5)
       {

          //asm volatile("l.mac %0,%1" : : "r"((Word32)*p), "r"((Word32)*p1));
          //t0 += *p * *p1;
          t1 += (scal_sig[j+0] * scal_sig[j-i]);

          t2 += (scal_sig[j+0] * scal_sig[j-i+1]);
          t1 += (scal_sig[j+1] * scal_sig[j-i+1]);

      t3 += (scal_sig[j+0] * scal_sig[j-i+2]);
      t2 += (scal_sig[j+1] * scal_sig[j-i+2]);
      t1 += (scal_sig[j+2] * scal_sig[j-i+2]);

      t4 += (scal_sig[j+0] * scal_sig[j-i+3]);
      t3 += (scal_sig[j+1] * scal_sig[j-i+3]);
      t2 += (scal_sig[j+2] * scal_sig[j-i+3]);
      t1 += (scal_sig[j+3] * scal_sig[j-i+3]);

      t4 += (scal_sig[j+1] * scal_sig[j-i+4]);
      t3 += (scal_sig[j+2] * scal_sig[j-i+4]);
      t2 += (scal_sig[j+3] * scal_sig[j-i+4]);
      t1 += (scal_sig[j+4] * scal_sig[j-i+4]);

      t4 += (scal_sig[j+2] * scal_sig[j-i+5]);
          t3 += (scal_sig[j+3] * scal_sig[j-i+5]);
      t2 += (scal_sig[j+4] * scal_sig[j-i+5]);

      t4 += (scal_sig[j+3] * scal_sig[j-i+6]);
          t3 += (scal_sig[j+4] * scal_sig[j-i+6]);

          t4 += (scal_sig[j+4] * scal_sig[j-i+7]);

       }

      corr[-i] = (t1<<1);
      corr[-i+2] = (t2<<1);
      corr[-i+4] = (t3<<1);
      corr[-i+6] = (t4<<1);

       //corr[-i] = t
    }
#else
#error "error"
#endif
    return;
}

