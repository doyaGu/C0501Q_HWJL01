/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : d4_17pf.c
*      Purpose          : Algebraic codebook decoder
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "d4_17pf.h"
const char d4_17pf_c_id[] = "@(#)$Id $" d4_17pf_h;

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
#define NB_PULSE 4

#include "gray.tab"

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
/*************************************************************************
 *
 *  FUNCTION:  decod_ACELP()
 *
 *  PURPOSE:   Algebraic codebook decoder
 *
 *************************************************************************/

void decode_4i40_17bits(
    Word16 sign,   /* i : signs of 4 pulses.                       */
    Word16 index,  /* i : Positions of the 4 pulses.               */
    Word16 cod[]   /* o : algebraic (fixed) codebook excitation    */
)
{
    Word16 i, j;
    Word16 pos[NB_PULSE];

    /* Decode the positions */

    i = index & 7;
    i = dgray[i];
    pos[0] = i * 5;   /* pos0 =i*5 */

    index >>= 3;
    i = index & 7;
    i = dgray[i];

    pos[1] = i * 5 + 1;

    index >>= 3;
    i = index & 7;
    i = dgray[i];
    pos[2] = i * 5 + 2;

    index >>= 3;
    j = index & 1;
    index >>= 1;
    i = index & 7;
    i = dgray[i];

    pos[3] = i * 5 + 3 + j;

    /* decode the signs  and build the codeword */

//    for (i = 0; i < L_SUBFR; i++) {
//        cod[i] = 0;
//    }
    Preset( 0, cod, L_SUBFR );

    for (j = 0; j < NB_PULSE; j++) {
        i = sign & 1;
        sign >>= 1;

        if (i != 0) {
            cod[pos[j]] = 8191;
        } else {
            cod[pos[j]] = -8192;
        }
    }

    return;
}
