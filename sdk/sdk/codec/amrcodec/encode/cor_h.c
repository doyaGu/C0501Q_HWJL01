/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
******************************************************************************
*
*      File             : cor_h.c
*      Purpose          : correlation functions for codebook search
*
*****************************************************************************
*/
/*
*****************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
*****************************************************************************
*/
#include "cor_h.h"
const char cor_h_id[] = "@(#)$Id $" cor_h_h;
/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/
#include "typedef.h"
#include "basic_op.h"
#include "count.h"
#include "inv_sqrt.h"
#include "cnst.h"

/*
*****************************************************************************
*                         PUBLIC PROGRAM CODE
*****************************************************************************
*/
/*************************************************************************
 *
 *  FUNCTION:  cor_h_x()
 *
 *  PURPOSE:  Computes correlation between target signal "x[]" and
 *            impulse response"h[]".
 *
 *  DESCRIPTION:
 *    The correlation is given by:
 *       d[n] = sum_{i=n}^{L-1} x[i] h[i-n]      n=0,...,L-1
 *
 *    d[n] is normalized such that the sum of 5 maxima of d[n] corresponding
 *    to each position track does not saturate.
 *
 *************************************************************************/
void cor_h_x (
    Word16 h[],    /* (i): impulse response of weighted synthesis filter */
    Word16 x[],    /* (i): target                                        */
    Word16 dn[],   /* (o): correlation between target and h[]            */
    Word16 sf      /* (i): scaling factor: 2 for 12.2, 1 for others      */
)
{
    cor_h_x2(h, x, dn, sf, NB_TRACK, STEP);
}

/*************************************************************************
 *
 *  FUNCTION:  cor_h_x2()
 *
 *  PURPOSE:  Computes correlation between target signal "x[]" and
 *            impulse response"h[]".
 *
 *  DESCRIPTION:
 *            See cor_h_x, d[n] can be normalized regards to sum of the
 *            five MR122 maxima or the four MR102 maxima.
 *
 *************************************************************************/
void cor_h_x2 (
    Word16 h[],    /* (i): impulse response of weighted synthesis filter */
    Word16 x[],    /* (i): target                                        */
    Word16 dn[],   /* (o): correlation between target and h[]            */
    Word16 sf,     /* (i): scaling factor: 2 for 12.2, 1 for others      */
    Word16 nb_track,/* (i): the number of ACB tracks                     */
    Word16 step    /* (i): step size from one pulse position to the next
                           in one track                                  */
)
{
    Word32 i, j, k;
    Word32 s, y32[L_CODE], max, tot;

    //Word32 mac_hi,mac_lo;

    /* first keep the result on 32 bits and find absolute maximum */
#ifdef CORHX2_ENG
VOLATILE (
    *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) h;
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
);
#endif

    tot = 5;
    for (k = 0; k < nb_track; k++)
    {
        max = 0;
        for (i = k; i < L_CODE; i += step)
        {
#ifdef CORHX2_ENG
VOLATILE (
            *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) &x[i];
            /*(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) h;
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
                                    (DST_NoWr   << P_Dst3 ) ;*/
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
                                    ((L_CODE-i) << P_Len       )|
                                    (0          << P_RdIncr    )|
                                    (0          << P_WrIncr    );
            WaitEngineDone();
            s = *(volatile int *)ALU_P0;
            if ( s >= 0x7ffffffeL )
              s = 0x7ffffffeL;
);
#elif defined CORHX2_OPRISC
            //s = 0;
            asm volatile("l.macrc %0,0" : "=r" (s));
            for (j = i; j < L_CODE; j++)
            {
                asm volatile("l.mac %0,%1" : : "r"((Word32)x[j]), "r"((Word32)h[j - i]));
            }
            asm volatile("l.maclc %0,1" : "=r" (s));
#elif defined CORHX2_PUREC
            s = 0;
            for (j = i; j < L_CODE; j++)
            {
                s += (x[j] * h[j - i]);
                if ( s > (Word32)0x3fffffffL )
                    s = 0x7fffffffL;
            }
            // s<<=1; ??? Why do not shift left? Add comment by Kuoping. 2006.11.29.
#else
#error "error"
#endif
            y32[i] = s;

            //s = L_abs (s);
            if( s < 0 )
                s = -s;

            //if (L_sub (s, max) > (Word32) 0L)
            if ( s > max)
                max = s;
        }
        //tot = L_add (tot, L_shr (max, 1));
        tot += (max);
    }

    //j = sub (norm_l (tot), sf);
    j = norm_l(tot) - sf;

    for (i = 0; i < L_CODE; i++)
    {
        //dn[i] = round (L_shl (y32[i], j));
        if ( j > 0 )
            s = y32[i] << j;
        else
            s = y32[i] >> (-j);
        dn[i]= (Word16)((s+0x00008000L)>>16);
    }
}

/*************************************************************************
 *
 *  FUNCTION:  cor_h()
 *
 *  PURPOSE:  Computes correlations of h[] needed for the codebook search;
 *            and includes the sign information into the correlations.
 *
 *  DESCRIPTION: The correlations are given by
 *         rr[i][j] = sum_{n=i}^{L-1} h[n-i] h[n-j];   i>=j; i,j=0,...,L-1
 *
 *  and the sign information is included by
 *         rr[i][j] = rr[i][j]*sign[i]*sign[j]
 *
 *************************************************************************/

void cor_h (
    Word16 h[],         /* (i) : impulse response of weighted synthesis
                                 filter                                  */
    Word16 sign[],      /* (i) : sign of d[n]                            */
    Word16 rr[][L_CODE] /* (o) : matrix of autocorrelation               */
)
{
    Word32 i, j, k, dec,tmp1,tmp2;
    Word32 s;
    Word16 h2[L_CODE];
    //Word32 mac_hi,mac_lo;

    /* Scaling for maximum precision */

    //s = 2;
#ifdef CORH_ENG1
VOLATILE (
    *(volatile int *)INIT_ACC = 1;
    *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) h;
    *(volatile int *)Src2Base = (FALSE << P_RdDec) | (int) h;
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

#elif defined CORH_OPRISC1
    asm volatile("l.macrc %0,0" : "=r" (s));
    for (i = 0; i < L_CODE; i++)
    {
     //s = L_mac (s, h[i], h[i]);

     asm volatile("l.mac %0,%1" : : "r"((Word32)h[i]), "r"((Word32)h[i]));

    }
    //asm volatile("l.mfspr %0, r0, 0x2802" : "=r"(mac_hi));
    asm volatile("l.mac %0,%1" : : "r"((Word32)1), "r"((Word32)1));
    asm volatile("l.maclc %0,1" : "=r" (s));
#elif defined CORH_PUREC1
    s = 1;
    for (i = 0; i < L_CODE; i++)
        s += (h[i] * h[i]);
    s<<= 1;
#else
#error "error"
#endif
    /*if(mac_hi > 0)
       {
         s = 0x7fffffffL;  // overflow
       }
     else
       {
         if((mac_lo & 0xc0000000)!=0)
           s = 0x7fffffffL;  // overflow
         else
           s = ((mac_lo<<1)+2);
       }
    */
    //j = sub (extract_h (s), 32767);
    j=(s>>16);

    if (j == 32767)
    {
        for (i = 0; i < L_CODE; i++)
        {
            //h2[i] = shr (h[i], 1);
            h2[i] = h[i] >> 1;
        }
    }
    else
    {
        //s = L_shr (s, 1);
        s >>= 1;
        //k = extract_h (L_shl (Inv_sqrt (s), 7));
        k = ( Inv_sqrt(s) >> 9 );

        //k = mult (k, 32440);                     /* k = 0.99*k */
        k = ( (k * 32440)>>15 );

#ifdef CORH_ENG2
VOLATILE (

        *(volatile int *)UsrDefC0 = (int)(k);
        *(volatile int *)UsrDefC1 = (int)(0x20L);
        *(volatile int *)Src0Base = (FALSE << P_RdDec) | (int) h;

        *(volatile int *)Dst0Base = (FALSE << P_RdDec) | (int) h2;

        *(volatile int *)(ALU_OP0+4) =
                                    (FALSE     << P_BP   ) |
                                    (0x4       << P_SAdd2) |
                                    (0x4       << P_SAdd3) |
                                    (SRC_FIFOA << P_Mul0L) |
                                    (SRC_UDC0  << P_Mul0R) |
                                    (SRC_C1    << P_Mul1L) |
                                    (SRC_UDC1  << P_Mul1R) |
                                    (SRC_C0    << P_Mul2L) |
                                    (SRC_C0    << P_Mul2R) ;

    *(volatile int *)(ALU_OP0)   = (SRC_C0     << P_Mul3L) |
                                   (SRC_C0     << P_Mul3R) |
                                   (R_SHIFT6   << P_SAdd0) |
                                   (R_SHIFT6   << P_SAdd1) |
                                   (DST_P0     << P_Dst0 ) |
                                   (DST_NoWr   << P_Dst1 ) |
                                   (DST_NoWr   << P_Dst2 ) |
                                   (DST_NoWr   << P_Dst3 ) ;

    *(volatile int *)RQ_TYPE  = (TRUE       << P_Fire      )|
                                (FALSE      << P_EnINT     )|
                                (FALSE      << P_EnInitAcc )|
                                (TRUE       << P_Sat       )|
                                (DATA16     << P_RdDT      )|
                                (DATA16     << P_WrDT      )|
                                (LARGE      << P_RdGranSize)|
                                (GRANULE_1  << P_RdGranule )|
                                (LARGE      << P_WrGranSize)|
                                (GRANULE_1  << P_WrGranule )|
                                (L_CODE     << P_Len       )|
                                (0          << P_RdIncr    )|
                                (0          << P_WrIncr    );
    WaitEngineDone();
);
#elif defined CORH_PUREC2
        for (i = 0; i < L_CODE; i++)
        {
            //h2[i] = round (L_shl (L_mult (h[i], k), 9));
            s = (h[i] * k)<<10;
#ifdef NO_ROUNDING
            h2[i] = (s>>16);
#else
            h2[i] = ( (s + 0x00008000L)>>16 );
#endif
        }
#else
#error "error"
#endif
    }

#ifdef CORH_OPRISC3
    asm volatile("l.macrc %0,0" : "=r" (s));
    for (k = 0; k < L_CODE; k++)
    {
        asm volatile("l.mac %0,%1" : : "r"(h2[k]), "r"(h2[k]));
        asm volatile("l.mfspr %0, r0, 0x2801" : "=r"(s));
#ifdef NO_ROUNDING
        rr[39-k][39-k] = (Word16)(s>>15);
#else
        rr[39-k][39-k] = (Word16)( (s + 0x00004000L)>>15 );
#endif
    }
#elif defined CORH_PUREC3
    /* build matrix rr[] */
    s = 0;
    //i = L_CODE - 1;
    for (k = 0; k < L_CODE; k++)
    {
        //s = L_mac (s, h2[k], h2[k]);
        s += (h2[k] * h2[k]);
        //rr[i][i] = round (s);

#ifdef NO_ROUNDING
    rr[39-k][39-k] = (Word16)(s>>15);
#else
        rr[39-k][39-k] = (Word16)( (s + 0x00004000L)>>15 );
#endif

    }
#else
#error "error"
#endif

    for (dec = 1; dec < L_CODE; dec++)
    {
        s = 0;
        //j = L_CODE - 1;
        //i = sub (j, dec);
    //i = j - dec;

        for (k = 0; k < (L_CODE - dec); k++)
        {
            //s = L_mac (s, h2[k], h2[k + dec]);
            s += (h2[k] * h2[k + dec]);
            //rr[j][i] = mult (round (s), mult (sign[i], sign[j]));

#ifdef NO_ROUNDING
        tmp1 = (s>>15);
#else
            tmp1 = ( (s + 0x00004000L) >> 15 );
#endif
        tmp2 = ( (sign[39 - dec-k]*sign[39-k]) >> 15 );
            rr[39-k][39 - dec-k] = (Word16)( (tmp1*tmp2) >>15 );
            rr[39 - dec-k][39-k] = rr[39-k][39 - dec-k];
        }
    }
}
