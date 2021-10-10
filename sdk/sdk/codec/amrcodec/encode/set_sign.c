/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : set_sign.c
*      Purpose          : Builds sign vector according to "dn[]" and "cn[]".
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "set_sign.h"
const char set_sign_id[] = "@(#)$Id $" set_sign_h;
/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "basic_op.h"
#include "count.h"
#include "inv_sqrt.h"
#include "cnst.h"

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/

/*************************************************************************
 *
 *  FUNCTION  set_sign()
 *
 *  PURPOSE: Builds sign[] vector according to "dn[]" and "cn[]".
 *           Also finds the position of maximum of correlation in each track
 *           and the starting position for each pulse.
 *
 *************************************************************************/
void set_sign(Word16 dn[],   /* i/o : correlation between target and h[]    */
              Word16 sign[], /* o   : sign of dn[]                          */
              Word16 dn2[],  /* o   : maximum of correlation in each track. */
              Word16 n       /* i   : # of maximum correlations in dn2[]    */
)
{
   Word32 i, j, k;
   Word16 val, min;
   Word32 pos = 0; /* initialization only needed to keep gcc silent */

   /* set sign according to dn[] */

   for (i = 0; i < L_CODE; i++) {
      val = dn[i];
      if (val >= 0)
      {
         sign[i] = 32767;
      } else
      {
         sign[i] = -32767;
         val = -val;
      }
      dn[i] = val;    /* modify dn[] according to the fixed sign */
      dn2[i] = val;
   }

   /* keep 8-n maximum positions/8 of each track and store it in dn2[] */

   for (i = 0; i < NB_TRACK; i++)
   {
      for (k = 0; k < (8-n); k++)
      {
         min = 0x7fff;
         for (j = i; j < L_CODE; j += STEP)
         {
            if (dn2[j] >= 0)
            {
               val = dn2[j] - min;
               if (val < 0)
               {
                  min = dn2[j];
                  pos = j;
               }
            }
         }
         dn2[pos] = -1;
      }
   }

   return;
}

/*************************************************************************
 *
 *  FUNCTION  set_sign12k2()
 *
 *  PURPOSE: Builds sign[] vector according to "dn[]" and "cn[]", and modifies
 *           dn[] to include the sign information (dn[i]=sign[i]*dn[i]).
 *           Also finds the position of maximum of correlation in each track
 *           and the starting position for each pulse.
 *
 *************************************************************************/
void set_sign12k2 (
    Word16 dn[],      /* i/o : correlation between target and h[]         */
    Word16 cn[],      /* i   : residual after long term prediction        */
    Word16 sign[],    /* o   : sign of d[n]                               */
    Word16 pos_max[], /* o   : position of maximum correlation            */
    Word16 nb_track,  /* i   : number of tracks tracks                    */
    Word16 ipos[],    /* o   : starting position for each pulse           */
    Word16 step       /* i   : the step size in the tracks                */
)
{
    Word32 i, j;
    Word16 val, cor, k_cn, k_dn, max, max_of_all;
    Word16 pos = 0; /* initialization only needed to keep gcc silent */
    Word16 en[L_CODE];                  /* correlation vector */
    Word32 s;

    /* calculate energy for normalization of cn[] and dn[] */
#ifdef SETSIGN12K2_ENG1
VOLATILE (
    *(volatile int *)INIT_ACC = 128;
    *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) cn;
    *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) cn;
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
                                    (L_CODE     << P_Len       )|
                                    (0          << P_RdIncr    )|
                                    (0          << P_WrIncr    );
        WaitEngineDone();
        s = *(volatile int *)ALU_P0;
    );
#elif defined SETSIGN12K2_PUREC1
    s = 128L;
    for (i = 0; i < L_CODE; i++)
    {
        s += (cn[i] * cn[i]);
        if ( s > (Word32)0x3fffffffL )
            s = 0x3fffffffL;
    }
    s<<=1;
#else
#error "error"
#endif

    s = Inv_sqrt (s);
    k_cn = (Word16)( s>>11 );

#ifdef SETSIGN12K2_ENG2
VOLATILE (
     *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) dn;
     *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) dn;
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
                                    (L_CODE     << P_Len       )|
                                    (0          << P_RdIncr    )|
                                    (0          << P_WrIncr    );
        WaitEngineDone();
        s = *(volatile int *)ALU_P0;
    );
#elif defined SETSIGN12K2_PUREC2
    s = 128L;
    for (i = 0; i < L_CODE; i++)
    {
        s += (dn[i] * dn[i]);
        if ( s > (Word32)0x3fffffffL )
            s = 0x3fffffffL;

    }
    s<<=1;
#else
#error "error"
#endif

    s = Inv_sqrt (s);
    k_dn = (Word16)( s>>11 );

    for (i = 0; i < L_CODE; i++)
    {
        val = dn[i];
        s = k_cn * cn[i];
        s += (k_dn * val);
        cor = (Word16)( (s+0x00000010L)>>5 );

        if (cor >= 0)
        {
            sign[i] = 32767;                 /* sign = +1 */
        }
        else
        {
            sign[i] = -32767;                /* sign = -1 */
            cor = -cor;
            val = -val;
        }
        /* modify dn[] according to the fixed sign */
        dn[i] = val;
        en[i] = cor;
    }

    max_of_all = -1;
    for (i = 0; i < nb_track; i++)
    {
        max = -1;
        for (j = i; j < L_CODE; j += step)
        {
            cor = en[j];
            val = cor - max;
            if (val > 0)
            {
                max = cor;
                pos = j;
            }
        }
        /* store maximum correlation position */
        pos_max[i] = pos;
        val = max - max_of_all;
        if (val > 0)
        {
            max_of_all = max;
            /* starting position for i0 */
            ipos[0] = i;
        }
    }

    /*----------------------------------------------------------------*
     *     Set starting position of each pulse.                       *
     *----------------------------------------------------------------*/

    pos = ipos[0];
    ipos[nb_track] = pos;

    for (i = 1; i < nb_track; i++)
    {
        pos++ ;
        if (pos >= nb_track)
        {
           pos = 0;
        }
        ipos[i] = pos;
        ipos[i+nb_track] = pos;
    }
}
