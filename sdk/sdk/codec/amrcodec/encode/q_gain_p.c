/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : q_gain_p.c
*      Purpose          : Scalar quantization of the pitch gain
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "q_gain_p.h"
const char q_gain_p_id[] = "@(#)$Id $" q_gain_p_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
//#include "basic_op.h"
//#include "oper_32b.h"
//#include "count.h"
#include "cnst.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
#include "gains.tab"

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
Word16 q_gain_pitch (   /* Return index of quantization                      */
    enum Mode mode,     /* i  : AMR mode                                     */
    Word16 gp_limit,    /* i  : pitch gain limit                             */
    Word16 *gain,       /* i/o: Pitch gain (unquant/quant),              Q14 */
    Word16 gain_cand[], /* o  : pitch gain candidates (3),   MR795 only, Q14 */
    Word16 gain_cind[]  /* o  : pitch gain cand. indices (3),MR795 only, Q0  */
)
{
    Word16 index, err, err_min;
    Word32 i;
//    err_min = abs_s (sub (*gain, qua_gain_pitch[0]));
    err_min = *gain - qua_gain_pitch[0];
    if ( err_min < 0 )
        err_min = - err_min;

    index = 0;
    for (i = 1; i < NB_QUA_PITCH; i++)
    {
        if (qua_gain_pitch[i] <= gp_limit)
        {
//            err = abs_s (sub (*gain, qua_gain_pitch[i]));
            err = *gain - qua_gain_pitch[i];
            if ( err < 0 )
                err = - err;

            if (err < err_min)
            {
                err_min = err;
                index = (Word16)i;
            }
        }
    }

    if ((Word16)mode == MR795)
    {
        /* in MR795 mode, compute three gain_pit candidates around the index
         * found in the quantization loop: the index found and the two direct
         * neighbours, except for the extreme cases (i=0 or i=NB_QUA_PITCH-1),
         * where the direct neighbour and the neighbour to that is used.
         */
        Word16 ii;

        if (index == 0)
        {
            ii = index;
        }
        else
        {
            if (   (index == NB_QUA_PITCH-1)
                || (qua_gain_pitch[index+1] > gp_limit) )
            {
                ii = index - 2;
            }
            else
            {
                ii = index - 1;
            }
        }

        /* store candidate indices and values */
        for (i = 0; i < 3; i++)
        {
            gain_cind[i] = ii;
            gain_cand[i] = qua_gain_pitch[ii];
            ii ++;
        }

        *gain = qua_gain_pitch[index];
    }
    else
    {
        /* in MR122 mode, just return the index and gain pitch found.
         * If bitexactness is required, mask away the two LSBs (because
         * in the original EFR, gain_pit was scaled Q12)
         */

       if ((Word16)mode == MR122)
       {
          /* clear 2 LSBits */
          *gain = qua_gain_pitch[index] & 0xFFFC;
       }
       else
       {
          *gain = qua_gain_pitch[index];
       }
    }
    return index;
}
