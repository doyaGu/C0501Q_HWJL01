﻿/*****************************************************************************
 *  $Id $
 *
 *  This file contains operations in double precision.                       *
 *  These operations are not standard double precision operations.           *
 *  They are used where single precision is not enough but the full 32 bits  *
 *  precision is not necessary. For example, the function Div_32() has a     *
 *  24 bits precision which is enough for our purposes.                      *
 *                                                                           *
 *  The double precision numbers use a special representation:               *
 *                                                                           *
 *     L_32 = hi<<16 + lo<<1                                                 *
 *                                                                           *
 *  L_32 is a 32 bit integer.                                                *
 *  hi and lo are 16 bit signed integers.                                    *
 *  As the low part also contains the sign, this allows fast multiplication. *
 *                                                                           *
 *      0x8000 0000 <= L_32 <= 0x7fff fffe.                                  *
 *                                                                           *
 *  We will use DPF (Double Precision Format )in this file to specify        *
 *  this special format.                                                     *
 *****************************************************************************
*/

#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
//#include "count.h"

/*****************************************************************************
 *                                                                           *
 *  Function L_Extract()                                                     *
 *                                                                           *
 *  Extract from a 32 bit integer two 16 bit DPF.                            *
 *                                                                           *
 *  Arguments:                                                               *
 *                                                                           *
 *   L_32      : 32 bit integer.                                             *
 *               0x8000 0000 <= L_32 <= 0x7fff ffff.                         *
 *   hi        : b16 to b31 of L_32                                          *
 *   lo        : (L_32 - hi<<16)>>1                                          *
 *****************************************************************************
*/

void L_Extract (Word32 L_32, Word16 *hi, Word16 *lo)
{
    //*hi = extract_h (L_32);
    //*lo = extract_l (L_msu (L_shr (L_32, 1), *hi, 16384));
    *hi = (Word16) (L_32 >> 16);
    *lo = (Word16) ((L_32 >> 1) - (Word32)(*hi << 15));
    return;
}

void L_ExtractX (Word32 L_32, Word16 *hi, Word16 *lo)
{
    *hi = (Word16) (L_32 >> 15);
    *lo = (Word16) (L_32 - (Word32)(*hi<<15));
    return;
}

/*****************************************************************************
 *                                                                           *
 *  Function L_Comp()                                                        *
 *                                                                           *
 *  Compose from two 16 bit DPF a 32 bit integer.                            *
 *                                                                           *
 *     L_32 = hi<<16 + lo<<1                                                 *
 *                                                                           *
 *  Arguments:                                                               *
 *                                                                           *
 *   hi        msb                                                           *
 *   lo        lsf (with sign)                                               *
 *                                                                           *
 *   Return Value :                                                          *
 *                                                                           *
 *             32 bit long signed integer (Word32) whose value falls in the  *
 *             range : 0x8000 0000 <= L_32 <= 0x7fff fff0.                   *
 *                                                                           *
 *****************************************************************************
*/

Word32 L_Comp (Word16 hi, Word16 lo)
{
    //Word32 L_32;

    //L_32 = L_deposit_h (hi);
    //return (L_mac (L_32, lo, 1));       /* = hi<<16 + lo<<1 */
    return ( (Word32)(hi << 16) + (Word32)(lo << 1));       /* = hi<<16 + lo<<1 */
}

Word32 L_CompX (Word16 hi, Word16 lo)
{
    Word32 L_32;

    L_32 = (Word32) (hi << 16) | (Word32) lo;
    return (L_32);
}

/*****************************************************************************
 * Function Mpy_32()                                                         *
 *                                                                           *
 *   Multiply two 32 bit integers (DPF). The result is divided by 2**31      *
 *                                                                           *
 *   L_32 = (hi1*hi2)<<1 + ( (hi1*lo2)>>15 + (lo1*hi2)>>15 )<<1              *
 *                                                                           *
 *   This operation can also be viewed as the multiplication of two Q31      *
 *   number and the result is also in Q31.                                   *
 *                                                                           *
 * Arguments:                                                                *
 *                                                                           *
 *  hi1         hi part of first number                                      *
 *  lo1         lo part of first number                                      *
 *  hi2         hi part of second number                                     *
 *  lo2         lo part of second number                                     *
 *                                                                           *
 *****************************************************************************
*/

Word32 Mpy_32 (Word16 hi1, Word16 lo1, Word16 hi2, Word16 lo2)
{
    Word32 L_32;

    //L_32 = L_mult (hi1, hi2);
    //L_32 = L_mac (L_32, mult (hi1, lo2), 1);
    //L_32 = L_mac (L_32, mult (lo1, hi2), 1);
    // after mult, won't overflow
    L_32 = L_mult (hi1, hi2) + (((Word32) mult(hi1, lo2)) << 1) + (((Word32) mult(lo1, hi2)) << 1);

    return (L_32);
}

/*****************************************************************************
 * Function Mpy_32_16()                                                      *
 *                                                                           *
 *   Multiply a 16 bit integer by a 32 bit (DPF). The result is divided      *
 *   by 2**15                                                                *
 *                                                                           *
 *                                                                           *
 *   L_32 = (hi1*lo2)<<1 + ((lo1*lo2)>>15)<<1                                *
 *                                                                           *
 * Arguments:                                                                *
 *                                                                           *
 *  hi          hi part of 32 bit number.                                    *
 *  lo          lo part of 32 bit number.                                    *
 *  n           16 bit number.                                               *
 *                                                                           *
 *****************************************************************************
*/

Word32 Mpy_32_16 (Word16 hi, Word16 lo, Word16 n)
{
    Word32 L_32;

    //L_32 = L_mult (hi, n);
    //L_32 = L_mac (L_32, mult (lo, n), 1);
    L_32 = L_mult (hi, n) + (((Word32) mult (lo, n)) << 1);

    return (L_32);
}

/*****************************************************************************
 *                                                                           *
 *   Function Name : Div_32                                                  *
 *                                                                           *
 *   Purpose :                                                               *
 *             Fractional integer division of two 32 bit numbers.            *
 *             L_num / L_denom.                                              *
 *             L_num and L_denom must be positive and L_num < L_denom.       *
 *             L_denom = denom_hi<<16 + denom_lo<<1                          *
 *             denom_hi is a normalize number.                               *
 *                                                                           *
 *   Inputs :                                                                *
 *                                                                           *
 *    L_num                                                                  *
 *             32 bit long signed integer (Word32) whose value falls in the  *
 *             range : 0x0000 0000 < L_num < L_denom                         *
 *                                                                           *
 *    L_denom = denom_hi<<16 + denom_lo<<1      (DPF)                        *
 *                                                                           *
 *       denom_hi                                                            *
 *             16 bit positive normalized integer whose value falls in the   *
 *             range : 0x4000 < hi < 0x7fff                                  *
 *       denom_lo                                                            *
 *             16 bit positive integer whose value falls in the              *
 *             range : 0 < lo < 0x7fff                                       *
 *                                                                           *
 *   Return Value :                                                          *
 *                                                                           *
 *    L_div                                                                  *
 *             32 bit long signed integer (Word32) whose value falls in the  *
 *             range : 0x0000 0000 <= L_div <= 0x7fff ffff.                  *
 *                                                                           *
 *  Algorithm:                                                               *
 *                                                                           *
 *  - find = 1/L_denom.                                                      *
 *      First approximation: approx = 1 / denom_hi                           *
 *      1/L_denom = approx * (2.0 - L_denom * approx )                       *
 *                                                                           *
 *  -  result = L_num * (1/L_denom)                                          *
 *****************************************************************************
*/

// sis3830 sth overflow here
Word32 Div_32_ori (Word32 L_num, Word16 denom_hi, Word16 denom_lo)
{
    Word16 approx, hi, lo, n_hi, n_lo;
    Word32 L_32;

    /* First approximation: 1 / L_denom = 1/denom_hi */

    approx = div_s ((Word16) 0x3fff, denom_hi);

    /* 1/L_denom = approx * (2.0 - L_denom * approx) */

    // sis3830 Mpy_32_16 overflow
    L_32 = Mpy_32_16 (denom_hi, denom_lo, approx);

    // sis3830 L_sub overflow
    L_32 = L_sub ((Word32) 0x7fffffffL, L_32);

    // sis3830 L_Extract overflow
    L_Extract (L_32, &hi, &lo);

    // sis3830 Mpy_32_16 overflow
    L_32 = Mpy_32_16 (hi, lo, approx);

    /* L_num * (1/L_denom) */

    L_Extract (L_32, &hi, &lo);
    L_Extract (L_num, &n_hi, &n_lo);
    L_32 = Mpy_32 (n_hi, n_lo, hi, lo);
    L_32 = L_shl (L_32, 2);

    return (L_32);
}

Word32 Div_32 (Word32 L_num, Word16 denom_hi, Word16 denom_lo)
{
    Word16 approx, hi, lo, n_hi, n_lo;
    Word32 L_32;

    /* First approximation: 1 / L_denom = 1/denom_hi */

    approx = div_s ((Word16) 0x3fff, denom_hi);

    /* 1/L_denom = approx * (2.0 - L_denom * approx) */

    // sis3830 Mpy_32_16 overflow
    //L_32 = Mpy_32_16 (denom_hi, denom_lo, approx);
    L_32 = denom_hi*approx + ((denom_lo*approx)>>15);

    // sis3830 L_sub overflow
    //L_32 = L_sub ((Word32) 0x7fffffffL, L_32);
    L_32 = 0x3fffffffL - L_32;

    // sis3830 L_Extract overflow
    //L_Extract (L_32, &hi, &lo);
    hi = (Word16) (L_32 >> 15);
    lo = (Word16) (L_32 - (Word32)(hi<<15));

    // sis3830 Mpy_32_16 overflow
    //L_32 = Mpy_32_16 (hi, lo, approx);
    L_32 = hi*approx + ((lo*approx)>>15);

    /* L_num * (1/L_denom) */

    //L_Extract (L_32, &hi, &lo);
    hi = (Word16) (L_32 >> 15);
    lo = (Word16) (L_32 - (Word32)(hi<<15));

    //L_Extract (L_num, &n_hi, &n_lo);
    n_hi = (Word16) (L_num >> 15);
    n_lo = (Word16) (L_num - (Word32)(n_hi<<15));

    //L_32 = Mpy_32 (n_hi, n_lo, hi, lo);
    L_32 = n_hi*hi + ((n_hi*lo)>>15) + ((n_lo*hi)>>15);
    L_32<<=2;

    //L_32 = L_shl (L_32, 2);

    return (L_32);
}
