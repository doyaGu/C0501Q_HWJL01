/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : q_gain_c.c
*      Purpose          : Scalar quantization of the innovative
*                       : codebook gain.
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "q_gain_c.h"
const char q_gain_c_id[] = "@(#)$Id $" q_gain_c_h;

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
#include "oper_32b.h"
#include "count.h"
#include "log2.h"
#include "pow2.h"

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

/*--------------------------------------------------------------------------*
 * Function q_gain_code()                                                   *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                  *
 * Scalar quantization of the innovative codebook gain.                     *
 *                                                                          *
 *--------------------------------------------------------------------------*/
Word16 q_gain_code (        /* o  : quantization index,            Q0  */
    enum Mode mode,         /* i  : AMR mode                           */
    Word16 exp_gcode0,      /* i  : predicted CB gain (exponent),  Q0  */
    Word16 frac_gcode0,     /* i  : predicted CB gain (fraction),  Q15 */
    Word16 *gain,           /* i/o: quantized fixed codebook gain, Q1  */
    Word16 *qua_ener_MR122, /* o  : quantized energy error,        Q10 */
                            /*      (for MR122 MA predictor update)    */
    Word16 *qua_ener        /* o  : quantized energy error,        Q10 */
                            /*      (for other MA predictor update)    */
)
{
    const Word16 *p;
    Word16 i, index;
    Word16 gcode0, err, err_min;
    Word16 g_q0;

    g_q0 = 0;

    //if (sub((Word16)mode, MR122) == 0)
    //{
    //   g_q0 = shr (*gain, 1); /* Q1 -> Q0 */
    //}
    if ((Word16)mode == MR122)
    {
       g_q0 = *gain >> 1; /* Q1 -> Q0 */
    }

    /*-------------------------------------------------------------------*
     *  predicted codebook gain                                          *
     *  ~~~~~~~~~~~~~~~~~~~~~~~                                          *
     *  gc0     = Pow2(int(d)+frac(d))                                   *
     *          = 2^exp + 2^frac                                         *
     *                                                                   *
     *-------------------------------------------------------------------*/

    //gcode0 = extract_l (Pow2 (exp_gcode0, frac_gcode0));  /* predicted gain */
    gcode0 = (Word16)(Pow2 (exp_gcode0, frac_gcode0));  /* predicted gain */

    //if (sub((Word16)mode, MR122) == 0)
    if ((Word16)mode == MR122)
    {
       //if(gcode0>=0x800)
       //   gcode0 = 0x7fff;
       //else
          gcode0 <<= 4;
       //gcode0 = shl (gcode0, 4);
       //gcode0 <<= 4;
    }
    else
    {
       //gcode0 = shl (gcode0, 5);
       gcode0 <<= 5;
    }

    /*-------------------------------------------------------------------*
     *                   Search for best quantizer                        *
     *-------------------------------------------------------------------*/

    p = &qua_gain_code[0];

    //if (sub((Word16)mode, MR122) == 0)
    if ((Word16)mode == MR122)
    {
       //err_min = abs_s (sub (g_q0, mult (gcode0, *p++)));
       err_min = *gain - (Word16)( (gcode0 * *p++)>>15 );
       if ( err_min < 0 )
           err_min = - err_min;
    }
    else
    {
       //err_min = abs_s (sub (*gain, mult (gcode0, *p++)));
       err_min = *gain - (Word16)( (gcode0 * *p++)>>16 );
       if ( err_min < 0 )
           err_min = - err_min;
    }
    p += 2;                                  /* skip quantized energy errors */
    index = 0;

    for (i = 1; i < NB_QUA_CODE; i+=2)
    {

       //if (sub((Word16)mode, MR122) == 0)
       if ((Word16)mode == MR122)
       {
          //err = abs_s (sub (g_q0,  mult (gcode0, *p++)));
          //err = g_q0 -  (Word16)(mult (gcode0, *p++));
          //err = sub (g_q0,  mult (gcode0, *p++));
          err = g_q0 - (Word16)( (gcode0 * *p++)>>15 );
          if ( err < 0 )
             err = - err;
       }
       else
       {
          //err = abs_s (sub (*gain, mult (gcode0, *p++)));
          err = *gain - (Word16)( (gcode0 * *p++)>>15 );
          if ( err < 0 )
             err = - err;
       }

       p += 5;                              /* skip quantized energy error */

       //if (sub (err, err_min) < 0)
       if (err < err_min)
       {
          err_min = err;
          index = i;
       }
    }

    //p = &qua_gain_code[add (add (index,index), index)];
    p = &qua_gain_code[(index * 3)];

    //if (sub((Word16)mode, MR122) == 0)
    if ((Word16)mode == MR122)
    {
       //*gain = shl (mult (gcode0, *p++), 1);
       *gain = (Word16)( (gcode0 * *p++)>>15 );
       if ( *gain > (Word16)0x3fff )
           *gain = (Word16)0x3fff;
       else
           *gain <<= 1;
    }
    else
    {
       //*gain = mult (gcode0, *p++);
       *gain = (Word16)( (gcode0 * *p++)>>15 );
    }

    /* quantized error energies (for MA predictor update) */
    *qua_ener_MR122 = *p++;
    *qua_ener = *p;

    return index;
}
