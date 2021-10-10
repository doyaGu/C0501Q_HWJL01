/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : pred_lt.c
*      Purpose          : Compute the result of long term prediction
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "pred_lt.h"
const char pred_lt_id[] = "@(#)$Id $" pred_lt_h;

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
#define UP_SAMP_MAX  6
#define L_INTER10    (L_INTERPOL-1)
#define FIR_SIZE     (UP_SAMP_MAX*L_INTER10+1)

/* 1/6 resolution interpolation filter  (-3 dB at 3600 Hz) */
/* Note: the 1/3 resolution filter is simply a subsampled
 *       version of the 1/6 resolution filter, i.e. it uses
 *       every second coefficient:
 *
 *          inter_3l[k] = inter_6[2*k], 0 <= k <= 3*L_INTER10
 */
static const Word16 inter_6[FIR_SIZE] =
{
    29443,
    28346, 25207, 20449, 14701, 8693, 3143,
    -1352, -4402, -5865, -5850, -4673, -2783,
    -672, 1211, 2536, 3130, 2991, 2259,
    1170, 0, -1001, -1652, -1868, -1666,
    -1147, -464, 218, 756, 1060, 1099,
    904, 550, 135, -245, -514, -634,
    -602, -451, -231, 0, 191, 308,
    340, 296, 198, 78, -36, -120,
    -163, -165, -132, -79, -19, 34,
    73, 91, 89, 70, 38, 0
};

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
/*************************************************************************
 *
 *  FUNCTION:   Pred_lt_3or6()
 *
 *  PURPOSE:  Compute the result of long term prediction with fractional
 *            interpolation of resolution 1/3 or 1/6. (Interpolated past
 *            excitation).
 *
 *  DESCRIPTION:
 *       The past excitation signal at the given delay is interpolated at
 *       the given fraction to build the adaptive codebook excitation.
 *       On return exc[0..L_subfr-1] contains the interpolated signal
 *       (adaptive codebook excitation).
 *
 *************************************************************************/
void Pred_lt_3or6 (
    Word16 exc[],     /* in/out: excitation buffer                         */
    Word16 T0,        /* input : integer pitch lag                         */
    Word16 frac,      /* input : fraction of lag                           */
    Word16 L_subfr,   /* input : subframe size                             */
    Word16 flag3      /* input : if set, upsampling rate = 3 (6 otherwise) */
)
{
    //Word32 i, j, k;
    Word32 j;
    Word16 *x0, *x1, *x2;
    const Word16 *c1, *c2;
    //Word32 s;

    x0 = &exc[-T0];

    frac = -frac;
    if (flag3 != 0)
    {
      frac <<= 1;   /* inter_3l[k] = inter_6[2*k] -> k' = 2*k */
    }

    if (frac < 0)
    {
        frac += UP_SAMP_MAX;
        x0--;
    }

    for (j = 0; j < L_subfr; j++)
    {
#ifndef OPRISCASM
        Word32 mac_hi, mac_lo, s;
        Word64 LL_temp;
#endif
        x1 = x0++;
        x2 = x0;
        c1 = &inter_6[frac];
        c2 = &inter_6[(UP_SAMP_MAX - frac)];

        //s = 0;
        //for (i = 0, k = 0; i < L_INTER10; i++, k += UP_SAMP_MAX)
        //{
        //    s += (x1[-i] * c1[k]);
        //    s += ( x2[i] * c2[k]);
        //}
#ifdef OPRISCASM
        //for (i = 0, k = 0; i < L_INTER10; i++, k += UP_SAMP_MAX)
        //{
        //  asm ("l.mac %0, %1" : : "r"((Word32)x1[-i]), "r"((Word32)c1[k]));
        //  asm ("l.mac %0, %1" : : "r"((Word32)x2[i]), "r"((Word32)c2[k]));
        //}
        asm ("l.mac %0, %1" : : "r"((Word32)x1[ 0]), "r"((Word32)c1[ 0]));
        asm ("l.mac %0, %1" : : "r"((Word32)x2[ 0]), "r"((Word32)c2[ 0]));
        asm ("l.mac %0, %1" : : "r"((Word32)x1[-1]), "r"((Word32)c1[ 6]));
        asm ("l.mac %0, %1" : : "r"((Word32)x2[ 1]), "r"((Word32)c2[ 6]));
        asm ("l.mac %0, %1" : : "r"((Word32)x1[-2]), "r"((Word32)c1[12]));
        asm ("l.mac %0, %1" : : "r"((Word32)x2[ 2]), "r"((Word32)c2[12]));
        asm ("l.mac %0, %1" : : "r"((Word32)x1[-3]), "r"((Word32)c1[18]));
        asm ("l.mac %0, %1" : : "r"((Word32)x2[ 3]), "r"((Word32)c2[18]));
        asm ("l.mac %0, %1" : : "r"((Word32)x1[-4]), "r"((Word32)c1[24]));
        asm ("l.mac %0, %1" : : "r"((Word32)x2[ 4]), "r"((Word32)c2[24]));
        asm ("l.mac %0, %1" : : "r"((Word32)x1[-5]), "r"((Word32)c1[30]));
        asm ("l.mac %0, %1" : : "r"((Word32)x2[ 5]), "r"((Word32)c2[30]));
        asm ("l.mac %0, %1" : : "r"((Word32)x1[-6]), "r"((Word32)c1[36]));
        asm ("l.mac %0, %1" : : "r"((Word32)x2[ 6]), "r"((Word32)c2[36]));
        asm ("l.mac %0, %1" : : "r"((Word32)x1[-7]), "r"((Word32)c1[42]));
        asm ("l.mac %0, %1" : : "r"((Word32)x2[ 7]), "r"((Word32)c2[42]));
        asm ("l.mac %0, %1" : : "r"((Word32)x1[-8]), "r"((Word32)c1[48]));
        asm ("l.mac %0, %1" : : "r"((Word32)x2[ 8]), "r"((Word32)c2[48]));
        asm ("l.mac %0, %1" : : "r"((Word32)x1[-9]), "r"((Word32)c1[54]));
        asm ("l.mac %0, %1" : : "r"((Word32)x2[ 9]), "r"((Word32)c2[54]));
        asm ("l.mac %0, %1" : : "r"((Word32)1     ), "r"((Word32)0x4000));

        //asm volatile("l.mfspr %0, r0, 0x2802" : "=r"(mac_hi));
        asm volatile("l.macrc %0, 15" : "=r" (exc[j]));
#else
        //LL_temp = 0;
        //for (i = 0, k = 0; i < L_INTER10; i++, k += UP_SAMP_MAX)
        //{
        //  LL_temp = LL_temp + x1[-i] * c1[k];
        //  LL_temp = LL_temp +  x2[i] * c2[k];
        //}
        LL_temp =           x1[ 0] * c1[ 0];
        LL_temp = LL_temp +  x2[0] * c2[ 0];
        LL_temp = LL_temp + x1[-1] * c1[ 6];
        LL_temp = LL_temp +  x2[1] * c2[ 6];
        LL_temp = LL_temp + x1[-2] * c1[12];
        LL_temp = LL_temp +  x2[2] * c2[12];
        LL_temp = LL_temp + x1[-3] * c1[18];
        LL_temp = LL_temp +  x2[3] * c2[18];
        LL_temp = LL_temp + x1[-4] * c1[24];
        LL_temp = LL_temp +  x2[4] * c2[24];
        LL_temp = LL_temp + x1[-5] * c1[30];
        LL_temp = LL_temp +  x2[5] * c2[30];
        LL_temp = LL_temp + x1[-6] * c1[36];
        LL_temp = LL_temp +  x2[6] * c2[36];
        LL_temp = LL_temp + x1[-7] * c1[42];
        LL_temp = LL_temp +  x2[7] * c2[42];
        LL_temp = LL_temp + x1[-8] * c1[48];
        LL_temp = LL_temp +  x2[8] * c2[48];
        LL_temp = LL_temp + x1[-9] * c1[54];
        LL_temp = LL_temp +  x2[9] * c2[54];

        mac_hi = (Word32) (LL_temp >> 32);
        mac_lo = (Word32) (LL_temp & 0xFFFFFFFF);
        s = mac_lo;
        exc[j] = (Word16)( (s+0x00004000L)>>15 );
#endif

//        exc[j] = round16(s);     move16 ();

    }

    return;
}
