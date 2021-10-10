/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : d1035pf.c
*      Purpose          : Builds the innovative codevector
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "d1035pf.h"
const char d1035pf_id[] = "@(#)$Id $" d1035pf_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "basic_op.h"
//#include "count.h"
#include "cnst.h"
#include "copy.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
#define NB_PULSE  10            /* number of pulses  */

#include "gray.tab"

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
/*************************************************************************
 *
 *  FUNCTION:   dec_10i40_35bits()
 *
 *  PURPOSE:  Builds the innovative codevector from the received
 *            index of algebraic codebook.
 *
 *   See  c1035pf.c  for more details about the algebraic codebook structure.
 *
 *************************************************************************/
void dec_10i40_35bits (
    Word16 index[],    /* (i)     : index of 10 pulses (sign+position)       */
    Word16 cod[]       /* (o)     : algebraic (fixed) codebook excitation    */
)
{
    Word16 i, j, pos1, pos2, sign, tmp;

//    for (i = 0; i < L_CODE; i++)
//    {
//        cod[i] = 0;
//    }
    Preset( 0, cod, L_CODE );

    /* decode the positions and signs of pulses and build the codeword */

    for (j = 0; j < NB_TRACK; j++)
    {
        /* compute index i */

        tmp = index[j];
        i = tmp & 7;
        i = dgray[i];

        i = (Word16)(i * 5);
        pos1 = i + j; /* position of pulse "j" */

        i = (tmp >> 3) & 1;
        if (i == 0)
        {
            sign = 4096;                                 /* +1.0 */
        }
        else
        {
            sign = -4096;                                /* -1.0 */
        }

        cod[pos1] = sign;

        /* compute index i */

        i = index[(j + 5)] & 7;
        i = dgray[i];
        i = (Word16)(i * 5);

        pos2 = i + j;      /* position of pulse "j+5" */

        //if (pos2 < pos1)
        //{
        //    sign = - sign;
        //}
        //cod[pos2] += sign;
        if (pos2 < pos1)
            cod[pos2] -= sign;
        else
            cod[pos2] += sign;
    }

    return;
}
