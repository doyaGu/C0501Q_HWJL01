/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : d3_14pf.c
*      Purpose          : Algebraic codebook decoder
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "d3_14pf.h"
const char d3_14pf_c_id[] = "@(#)$Id $" d3_14pf_h;

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
#define NB_PULSE 3           /* number of pulses  */

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
/*************************************************************************
 *
 *  FUNCTION:  decode_3i40_14bits (decod_ACELP())
 *
 *  PURPOSE:   Algebraic codebook decoder
 *
 *************************************************************************/

void decode_3i40_14bits(
    Word16 sign,   /* i : signs of 3 pulses.                       */
    Word16 index,  /* i : Positions of the 3 pulses.               */
    Word16 cod[]   /* o : algebraic (fixed) codebook excitation    */
)
{
    Word16 i, j;
    Word16 pos[NB_PULSE];

    /* Decode the positions */

    i = index & 7;

    pos[0] = i * 5;   /* pos0 =i*5 */

    index >>= 3;
    j = index & 1;
    index >>= 1;
    i = index & 7;

    pos[1] = i * 5 + 1 + (j<<1);

    index >>= 3;
    j = index & 1;
    index >>= 1;
    i = index & 7;

    pos[2] = i * 5 + 2 + (j<<1);

    /* decode the signs  and build the codeword */

//    for (i = 0; i < L_SUBFR; i++) {
//        cod[i] = 0;
//    }
    Preset( 0, cod, L_SUBFR );

    for (j = 0; j < NB_PULSE; j++) {
        i = sign & 1;
        sign >>= 1;

        if (i > 0) {
            cod[pos[j]] = 8191;                         /* +1.0 */
        } else {
            cod[pos[j]] = -8192;                        /* -1.0 */
        }
    }

    return;
}
