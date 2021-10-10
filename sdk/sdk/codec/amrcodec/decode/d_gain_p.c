/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : d_gain_p.c
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "d_gain_p.h"
const char d_gain_p_id[] = "@(#)$Id $" d_gain_p_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
#include "mode.h"
#include "basic_op.h"
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

/*
**************************************************************************
*
*  Function    : d_gain_pitch
*  Purpose     : Decodes the pitch gain using the received index.
*                output is in Q14
*
**************************************************************************
*/
Word16 d_gain_pitch (      /* return value: gain (Q14)                */
    enum Mode mode,        /* i   : AMR mode                          */
    Word16 index           /* i   : index of quantization             */
)
{
    Word16 gain;

    if ((Word16)mode == MR122)
    {
       /* clear 2 LSBits */
       //gain = (qua_gain_pitch[index] >> 2) << 2;
        gain = qua_gain_pitch[index] & 0xFFFC;
    }
    else
    {
       gain = qua_gain_pitch[index];
    }

    return gain;
}
