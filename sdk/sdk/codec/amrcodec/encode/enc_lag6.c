/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : enc_lag6.c
*      Purpose          : Encoding of fractional pitch lag with 1/6 resolution.
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "enc_lag6.h"
const char enc_lag6_id[] = "@(#)$Id $" enc_lag6_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "basic_op.h"
//#include "count.h"

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
 *   FUNCTION:  Enc_lag6
 *
 *   PURPOSE:  Encoding of fractional pitch lag with 1/6 resolution.
 *
 *   DESCRIPTION:
 *                    First and third subframes:
 *                    --------------------------
 *   The pitch range is divided as follows:
 *           17 3/6  to   94 3/6   resolution 1/6
 *           95      to   143      resolution 1
 *
 *   The period is encoded with 9 bits.
 *   For the range with fractions:
 *     index = (T-17)*6 + frac - 3;
 *                         where T=[17..94] and frac=[-2,-1,0,1,2,3]
 *   and for the integer only range
 *     index = (T - 95) + 463;        where T=[95..143]
 *
 *                    Second and fourth subframes:
 *                    ----------------------------
 *   For the 2nd and 4th subframes a resolution of 1/6 is always used,
 *   and the search range is relative to the lag in previous subframe.
 *   If t0 is the lag in the previous subframe then
 *   t_min=t0-5   and  t_max=t0+4   and  the range is given by
 *       (t_min-1) 3/6   to  (t_max) 3/6
 *
 *   The period in the 2nd (and 4th) subframe is encoded with 6 bits:
 *     index = (T-(t_min-1))*6 + frac - 3;
 *                 where T=[t_min-1..t_max] and frac=[-2,-1,0,1,2,3]
 *
 *   Note that only 61 values are used. If the decoder receives 61, 62,
 *   or 63 as the relative pitch index, it means that a transmission
 *   error occurred and the pitch from previous subframe should be used.
 *
 *************************************************************************/
Word16 Enc_lag6 (        /* o : Return index of encoding             */
    Word16 T0,           /* i : Pitch delay                          */
    Word16 T0_frac,      /* i : Fractional pitch delay               */
    Word16 T0_min,       /* i : minimum of search range              */
    Word16 delta_flag    /* i : Flag for 1st (or 3rd) subframe       */
)
{
    Word16 index;

    if (delta_flag == 0)          /* if 1st or 3rd subframe */
    {
       /* encode pitch delay (with fraction) */
       if (T0 <= 94)
       {
          index = T0*6 - 105 + T0_frac;
//          i = add (add (T0, T0), T0);
//          index = add (sub (add (i, i), 105), T0_frac);
       }
       else
       {
          index = T0 + 368;
       }

    }
    else
       /* if second or fourth subframe */
    {
        index = 6*(T0-T0_min) + 3 + T0_frac;
//       i = T0 - T0_min;
//       i = add (add (i, i), i);
//       index = add (add (add (i, i), 3), T0_frac);
    }

    return index;
}
