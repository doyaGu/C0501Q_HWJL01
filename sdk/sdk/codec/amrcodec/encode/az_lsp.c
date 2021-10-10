/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : az_lsp.c
*      Purpose          : Compute the LSPs from the LP coefficients
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "az_lsp.h"
const char az_lsp_id[] = "@(#)$Id $" az_lsp_h;
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
#include "grid.tab"
#define NC   M/2                  /* M = LPC order, NC = M/2 */

/*
********************************************************************************
*                         LOCAL PROGRAM CODE
********************************************************************************
*/
/*
**************************************************************************
*
*  Function    : Chebps
*  Purpose     : Evaluates the Chebyshev polynomial series
*  Description : - The polynomial order is   n = m/2 = 5
*                - The polynomial F(z) (F1(z) or F2(z)) is given by
*                   F(w) = 2 exp(-j5w) C(x)
*                  where
*                   C(x) = T_n(x) + f(1)T_n-1(x) + ... +f(n-1)T_1(x) + f(n)/2
*                  and T_m(x) = cos(mw) is the mth order Chebyshev
*                  polynomial ( x=cos(w) )
*  Returns     : C(x) for the input x.
*
**************************************************************************
*/
static Word16 Chebps (Word16 x,
                      Word16 f[], /* (n) */
                      Word16 n)
{
    Word16 cheb;
    Word32 t0,i;
#ifdef AZLSP_OPRISC
    Word32 b1,b2,l_tmp;
    b2 = 0x800000;
    b1 = (x << 9);
    b1 += (f[1] << 13);
    asm volatile("l.macrc %0, 0" : "=r" (l_tmp));
    for (i = 2; i < n; i++)
    {
    asm volatile("l.mac %0, %1" : : "r"(b1), "r"((Word32)x));
    asm volatile("l.macrc %0, 15" : "=r" (l_tmp));
    l_tmp <<= 1;
    l_tmp -= b2;
    l_tmp += (f[i]*8192);
    b2 = b1;
    b1 = l_tmp;
    }
    asm volatile("l.mac %0, %1" : : "r"(b1), "r"((Word32)x));
    asm volatile("l.macrc %0, 15" : "=r" (t0));
    t0 -= b2;
    t0 += (f[n]*4096);

#else // AZLSP_PUREC

    Word32 b0_h, b0_l, b1_h, b1_l, b2_h, b2_l;
    b2_h = 256;                                  //b2 = 1.0
    b2_l = 0;

    // sis3830 L_mult & L_mac overflow
    //t0 = L_mult (x, 512);           2*x
    t0 = (x << 9);
    //t0 = L_mac (t0, f[1], 8192);    + f[1]
    t0 += (f[1] << 13);
    //L_Extract (t0, &b1_h, &b1_l);   b1 = 2*x + f[1]
    b1_h = (t0 >> 15);
    b1_l = (t0 - (b1_h<<15));

    for (i = 2; i < n; i++)
    {
        //t0 = Mpy_32_16 (b1_h, b1_l, x);          t0 = 2.0*x*b1
        t0 = b1_h * x;
        t0 += (b1_l * x)>>15;
        //t0 = t0 << 1;
        //t0 = L_shl (t0, 1);
        t0 = t0 << 1;

        //t0 = L_mac (t0, b2_h, (Word16) 0x8000);  t0 = 2.0*x*b1 - b2
        //t0 += (b2_h*(Word16)0x8000)<<1;
        t0 += (b2_h*0xffff8000);

        //t0 = L_msu (t0, b2_l, 1);
        //t0 = t0 - (((Word32)b2_l) << 1);
        t0 = t0 - b2_l ;

        //t0 = L_mac (t0, f[i], 8192);             t0 = 2.0*x*b1 - b2 + f[i]
        //t0 += (f[i]*8192)<<1;
        t0 += (f[i]*8192);

        //t0 = t0 << 1;

        //L_Extract (t0, &b0_h, &b0_l);            b0 = 2.0*x*b1 - b2 + f[i]
        //b0_h = (t0 >> 16);
        //b0_l = ( (t0>>1) - (b0_h<<15));
        b0_h = (t0 >> 15);
        b0_l = (t0 - (b0_h<<15));

        b2_l = b1_l;                            // b2 = b1;
        b2_h = b1_h;
        b1_l = b0_l;                            // b1 = b0;
        b1_h = b0_h;
    }

    //t0 = Mpy_32_16 (b1_h, b1_l, x);              t0 = x*b1;
    t0 = b1_h * x;
    t0 += (b1_l * x)>>15;

    //t0 = L_mac (t0, b2_h, (Word16) 0x8000);      t0 = x*b1 - b2
    t0 += b2_h*0xffff8000;
    //t0 <<=1;
    //t0 = L_msu (t0, b2_l, 1);
    t0 = t0 - (b2_l);

    //t0 = L_mac (t0, f[i], 4096);                 t0 = x*b1 - b2 + f[i]/2
    t0 += (f[n]*4096);
#endif

    //t0 <<=1;
    // sis3830 L_shl overflow
    //t0 = L_shl (t0, 6);
    if(t0>(Word32)0xffffff)
       t0 = 0x7fffffff;
    else if(t0<=(Word32)0xfeffffff)
       t0 = 0x80000000;
    else
       t0<<=7;
    //cheb = extract_h (t0);
    cheb = (Word16)(t0 >> 16);

    return (cheb);
}

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
/*
**************************************************************************
*
*  Function    : Az_lsp
*  Purpose     : Compute the LSPs from  the LP coefficients
*
**************************************************************************
*/
void Az_lsp (
    Word16 a[],         /* (i)  : predictor coefficients (MP1)               */
    Word16 lsp[],       /* (o)  : line spectral pairs (M)                    */
    Word16 old_lsp[]    /* (i)  : old lsp[] (in case not found 10 roots) (M) */
)
{
    Word32 i, j, nf, ip;
    Word16 xlow, ylow, xhigh, yhigh, xmid, ymid, xint;
    Word16 x, y, sign, exp;
    Word16 *coef;
    Word16 f1[M / 2 + 1], f2[M / 2 + 1];
    Word32 t0;

    /*-------------------------------------------------------------*
     *  find the sum and diff. pol. F1(z) and F2(z)                *
     *    F1(z) <--- F1(z)/(1+z**-1) & F2(z) <--- F2(z)/(1-z**-1)  *
     *                                                             *
     * f1[0] = 1.0;                                                *
     * f2[0] = 1.0;                                                *
     *                                                             *
     * for (i = 0; i< NC; i++)                                     *
     * {                                                           *
     *   f1[i+1] = a[i+1] + a[M-i] - f1[i] ;                       *
     *   f2[i+1] = a[i+1] - a[M-i] + f2[i] ;                       *
     * }                                                           *
     *-------------------------------------------------------------*/

    f1[0] = 1024;                               /* f1[0] = 1.0 */
    f2[0] = 1024;                               /* f2[0] = 1.0 */

    for (i = 0; i < NC; i++)
    {
        //t0 = L_mult (a[i + 1], 8192);   /* x = (a[i+1] + a[M-i]) >> 2  */
        t0 = a[i + 1] * 8192;

        //t0 = L_mac (t0, a[M - i], 8192);
        t0 += a[M - i] * 8192;
        //t0 = a[i + 1] + a[M - i];
        //x = extract_h (t0);
        x = (Word16)( t0>>15 );

        /* f1[i+1] = a[i+1] + a[M-i] - f1[i] */
        //f1[i + 1] = sub (x, f1[i]);
        f1[i + 1] = x - f1[i];

        //t0 = L_mult (a[i + 1], 8192);   /* x = (a[i+1] - a[M-i]) >> 2 */
        t0 = a[i + 1] * 8192;

        //t0 = L_msu (t0, a[M - i], 8192);
        t0 -= a[M - i] * 8192;
    //t0 = a[i + 1] - a[M - i];
        //x = extract_h (t0);
        x = (Word16)( t0>>15 );

        /* f2[i+1] = a[i+1] - a[M-i] + f2[i] */
        //f2[i + 1] = add (x, f2[i]);
        f2[i + 1] = x + f2[i];
    }

    /*-------------------------------------------------------------*
     * find the LSPs using the Chebychev pol. evaluation           *
     *-------------------------------------------------------------*/

    nf = 0;                                     /* number of found frequencies */
    ip = 0;                                     /* indicator for f1 or f2      */

    coef = f1;

    xlow = grid[0];
    ylow = Chebps (xlow, coef, NC);

    j = 0;

    //while ((sub (nf, M) < 0) && (sub (j, grid_points) < 0))
    while ((nf < M) && (j < grid_points))
    {
        j++;
        xhigh = xlow;
        yhigh = ylow;
        xlow = grid[j];
        ylow = Chebps (xlow, coef, NC);

        //if (L_mult (ylow, yhigh) <= (Word32) 0L)
        if ((ylow * yhigh) <= 0)
        {

            /* divide 4 times the interval */

            for (i = 0; i < 4; i++)
            {
                /* xmid = (xlow + xhigh)/2 */
                //xmid = add (shr (xlow, 1), shr (xhigh, 1));
                xmid = (xlow>>1) + (xhigh>>1);

                ymid = Chebps (xmid, coef, NC);

                //if (L_mult (ylow, ymid) <= (Word32) 0L)
                if ( (ylow * ymid) <=  0)
                {
                    yhigh = ymid;
                    xhigh = xmid;
                }
                else
                {
                    ylow = ymid;
                    xlow = xmid;
                }
            }

            /*-------------------------------------------------------------*
             * Linear interpolation                                        *
             *    xint = xlow - ylow*(xhigh-xlow)/(yhigh-ylow);            *
             *-------------------------------------------------------------*/

            //x = sub (xhigh, xlow);
            //y = sub (yhigh, ylow);
            x = xhigh - xlow;
            y = yhigh - ylow;

            if (y == 0)
            {
                xint = xlow;
            }
            else
            {
                sign = y;
                //y = abs_s (y);
                y = ( y>0 ) ? y : -y;

                exp = norm_s (y);

                //y = shl (y, exp);
                y <<= exp;

                y = div_s ((Word16) 16383, y);

                //t0 = L_mult (x, y);
                //t0 = L_shr (t0, sub (20, exp));
                //y = extract_l (t0);     /* y= (xhigh-xlow)/(yhigh-ylow) */
                t0 = x * y;
                t0 >>= (19 - exp);
                y = (Word16)t0;

                //if (sign < 0)
                //    y = negate (y);
                if (sign < 0)
                    y = -y;

                //t0 = L_mult (ylow, y);
                //t0 = L_shr (t0, 11);
                //xint = sub (xlow, extract_l (t0)); /* xint = xlow - ylow*y */
                t0 = ylow * y;
                t0 >>= 10;
                xint = xlow - (Word16)t0;

            }

            lsp[nf] = xint;
            xlow = xint;
            nf++;

            if (ip == 0)
            {
                ip = 1;
                coef = f2;
            }
            else
            {
                ip = 0;
                coef = f1;
            }
            ylow = Chebps (xlow, coef, NC);

        }

    }

    /* Check if M roots found */

    //if (sub (nf, M) < 0)
    if (nf < M)
    {
        for (i = 0; i < M; i++)
        {
            lsp[i] = old_lsp[i];
        }

    }
    return;
}

