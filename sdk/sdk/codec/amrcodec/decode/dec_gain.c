﻿/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : dec_gain.c
*      Purpose          : Decode the pitch and codebook gains
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "dec_gain.h"
const char dec_gain_id[] = "@(#)$Id $" dec_gain_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
#include "basic_op.h"
//#include "oper_32b.h"
//#include "count.h"
#include "mode.h"
#include "cnst.h"
#include "pow2.h"
#include "log2.h"
#include "gc_pred.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/

#include "qua_gain.tab"
#include "qgain475.tab"

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/

/*************************************************************************
 *
 *   FUNCTION:  Dec_gain()
 *
 *   PURPOSE: Decode the pitch and codebook gains
 *
 ************************************************************************/
void Dec_gain(
    gc_predState *pred_state, /* i/o: MA predictor state           */
    enum Mode mode,           /* i  : AMR mode                     */
    Word16 index,             /* i  : index of quantization.       */
    Word16 code[],            /* i  : Innovative vector.           */
    Word16 evenSubfr,         /* i  : Flag for even subframes      */
    Word16 * gain_pit,        /* o  : Pitch gain.                  */
    Word16 * gain_cod         /* o  : Code gain.                   */
)
{
    const Word16 *p;
    Word16 frac, gcode0, exp, qua_ener, qua_ener_MR122;
    Word16 g_code;
    Word32 L_tmp;

    /* Read the quantized gains (table depends on mode) */
    index <<= 2;

    if ( ((Word16)mode == MR102) ||
         ((Word16)mode == MR74)  ||
         ((Word16)mode == MR67))
    {
        p = &table_gain_highrates[index];

        *gain_pit = *p++;
        g_code = *p++;
        qua_ener_MR122 = *p++;
        qua_ener = *p;
    }
    else
    {
        if ((Word16)mode == MR475)
        {
            index = index + ((1 - evenSubfr) << 1);
            p = &table_gain_MR475[index];

            *gain_pit = *p++;
            g_code = *p++;

            /*---------------------------------------------------------*
             *  calculate predictor update values (not stored in 4.75  *
             *  quantizer table to save space):                        *
             *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  *
             *                                                         *
             *   qua_ener       = log2(g)                              *
             *   qua_ener_MR122 = 20*log10(g)                          *
             *---------------------------------------------------------*/

            /* Log2(x Q12) = log2(x) + 12 */
            Log2 ((Word16)g_code, &exp, &frac);
            exp -= 12;

            qua_ener_MR122 = ((frac+0x0020)>>5) + (exp<<10);

            /* 24660 Q12 ~= 6.0206 = 20*log10(2) */
//            L_tmp = Mpy_32_16(exp, frac, 24660);
//            qua_ener = round16(L_shl (L_tmp, 13)); /* Q12 * Q0 = Q13 -> Q10 */
            L_tmp = (Word32)(exp*24660) + (Word32)((frac*24660)>>15);
            qua_ener = (Word16)((L_tmp+0x00000002L)>>2);
        }
        else
        {
            p = &table_gain_lowrates[index];

            *gain_pit = *p++;
            g_code = *p++;
            qua_ener_MR122 = *p++;
            qua_ener = *p;
        }
    }

    /*-------------------------------------------------------------------*
     *  predict codebook gain                                            *
     *  ~~~~~~~~~~~~~~~~~~~~~                                            *
     *  gc0     = Pow2(int(d)+frac(d))                                   *
     *          = 2^exp + 2^frac                                         *
     *                                                                   *
     *  gcode0 (Q14) = 2^14*2^frac = gc0 * 2^(14-exp)                    *
     *-------------------------------------------------------------------*/
    // sis3830
    //gc_pred(pred_state, mode, code, &exp, &frac, NULL, NULL);
    gc_pred_de(pred_state, mode, code, &exp, &frac, 0, 0);

    gcode0 = (Word16)(Pow2(14, frac));

    /*------------------------------------------------------------------*
     *  read quantized gains, update table of past quantized energies   *
     *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   *
     *  st->past_qua_en(Q10) = 20 * Log10(g_fac) / constant             *
     *                       = Log2(g_fac)                              *
     *                       = qua_ener                                 *
     *                                           constant = 20*Log10(2) *
     *------------------------------------------------------------------*/

    L_tmp = (g_code * gcode0);
//    L_tmp >>= (10 - exp);
    *gain_cod = (Word16)(L_shr(L_tmp, (25 - exp)));

    /* update table of past quantized energies */

    gc_pred_update(pred_state, qua_ener_MR122, qua_ener);

    return;
}
