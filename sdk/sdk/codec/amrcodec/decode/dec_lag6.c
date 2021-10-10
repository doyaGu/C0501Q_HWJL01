/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : dec_lag6.c
*      Purpose          : Decoding of fractional pitch lag with 1/6 resolution.
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "dec_lag6.h"
const char dec_lag6_id[] = "@(#)$Id $" dec_lag6_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
#include "basic_op.h"
//#include "count.h"

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
/*
************************************************************************
*   FUNCTION:   Dec_lag6
*
*   PURPOSE:  Decoding of fractional pitch lag with 1/6 resolution.
*             Extract the integer and fraction parts of the pitch lag from
*             the received adaptive codebook index.
*
*    See "Enc_lag6.c" for more details about the encoding procedure.
*
*    The fractional lag in 1st and 3rd subframes is encoded with 9 bits
*    while that in 2nd and 4th subframes is relatively encoded with 6 bits.
*    Note that in relative encoding only 61 values are used. If the
*    decoder receives 61, 62, or 63 as the relative pitch index, it means
*    that a transmission error occurred. In this case, the pitch lag from
*    previous subframe (actually from previous frame) is used.
*
************************************************************************
*/
void Dec_lag6 (
    Word16 index,      /* input : received pitch index           */
    Word16 pit_min,    /* input : minimum pitch lag              */
    Word16 pit_max,    /* input : maximum pitch lag              */
    Word16 i_subfr,    /* input : subframe flag                  */
    Word16 *T0,        /* in/out: integer part of pitch lag      */
    Word16 *T0_frac    /* output: fractional part of pitch lag   */
)
{
    Word32 i;
    Word16 T0_min, T0_max;

    if (i_subfr == 0)          /* if 1st or 3rd subframe */
    {
       if (index < 463)
       {
          /* T0 = (index+5)/6 + 17 */
          *T0 = (Word16)(((index+5)*5462)>>15) + 17;
          i = (*T0 * 3);
          /* *T0_frac = index - T0*6 + 105 */
          *T0_frac = index - (i<<1) + 105;

       }
       else
       {
          *T0 = index - 368;
          *T0_frac = 0;
       }
    }
    else
       /* second or fourth subframe */
    {
       /* find T0_min and T0_max for 2nd (or 4th) subframe */

       T0_min = (*T0 - 5);
       if (T0_min < pit_min)
       {
          T0_min = pit_min;
       }
       T0_max = T0_min + 9;
       if (T0_max > pit_max)
       {
          T0_max = pit_max;
          T0_min = T0_max - 9;
       }

       /* i = (index+5)/6 - 1 */
       i = (Word16)(((index+5)*5462)>>15) - 1;
       *T0 = i + T0_min;
       i = i * 3;
       *T0_frac = index - 3 - (i<<1);

    }
}
