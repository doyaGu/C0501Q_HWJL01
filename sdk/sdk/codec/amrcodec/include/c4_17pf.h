﻿/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : c4_17pf.h
*      Purpose          : Searches a 17 bit algebraic codebook containing 4 pulses
*                         in a frame of 40 samples.
*
********************************************************************************
*/
#ifndef c4_17pf_h
#define c4_17pf_h "$Id $"

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/

/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
/*************************************************************************
 *
 *  FUNCTION:  code_4i40_17bits()
 *
 *  PURPOSE:  Searches a 17 bit algebraic codebook containing 4 pulses
 *            in a frame of 40 samples.
 *
 *  DESCRIPTION:
 *    The code length is 40, containing 4 nonzero pulses: i0...i3.
 *    All pulses can have two possible amplitudes: +1 or -1.
 *    Pulse i0 to i2 can have 8 possible positions, pulse i3 can have
 *    2x8=16 positions.
 *
 *       i0 :  0, 5, 10, 15, 20, 25, 30, 35.
 *       i1 :  1, 6, 11, 16, 21, 26, 31, 36.
 *       i2 :  2, 7, 12, 17, 22, 27, 32, 37.
 *       i3 :  3, 8, 13, 18, 23, 28, 33, 38.
 *             4, 9, 14, 19, 24, 29, 34, 39.
 *
 *************************************************************************/

Word16 code_4i40_17bits(
    Word16 x[], /* (i)   : target vector                                 */
    Word16 h[], /* (i)   : impulse response of weighted synthesis filter */
                /*         h[-L_subfr..-1] must be set to zero.          */
    Word16 T0,  /* (i)   : Pitch lag                                     */
    Word16 pitch_sharp, /* (i)   : Last quantized pitch gain             */
    Word16 code[],      /* (o)   : Innovative codebook                   */
    Word16 y[],         /* (o)   : filtered fixed codebook excitation    */
    Word16 * sign       /* (o)   : Signs of 4 pulses                     */
);

#endif
