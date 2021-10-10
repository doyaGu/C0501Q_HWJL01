/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : d_gain_c.c
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "d_gain_c.h"
const char d_gain_c_id[] = "@(#)$Id $" d_gain_c_h;

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
#include "log2.h"
#include "pow2.h"
#include "gc_pred.h"

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
*  Function    : d_gain_code
*  Purpose     : Decode the fixed codebook gain using the received index.
*
**************************************************************************
*/
void d_gain_code (
    gc_predState *pred_state, /* i/o : MA predictor state               */
    enum Mode mode,           /* i   : AMR mode (MR795 or MR122)        */
    Word16 index,             /* i   : received quantization index      */
    Word16 code[],            /* i   : innovation codevector            */
    Word16 *gain_code         /* o   : decoded innovation gain          */
)
{
    Word16 gcode0, exp, frac;
    const Word16 *p;
    Word16 qua_ener_MR122, qua_ener;
    Word16 exp_inn_en;
    Word16 frac_inn_en;
    Word32 L_tmp;

    /*-------------- Decode codebook gain ---------------*/

    /*-------------------------------------------------------------------*
     *  predict codebook gain                                            *
     *  ~~~~~~~~~~~~~~~~~~~~~                                            *
     *  gc0     = Pow2(int(d)+frac(d))                                   *
     *          = 2^exp + 2^frac                                         *
     *                                                                   *
     *-------------------------------------------------------------------*/

    gc_pred_de(pred_state, mode, code, &exp, &frac,
            &exp_inn_en, &frac_inn_en);

    p = &qua_gain_code[(index*3)];

    /* Different scalings between MR122 and the other modes */
    if ((Word16)mode == MR122)
    {
        gcode0 = (Word16)(Pow2 (exp, frac));  /* predicted gain */
        // sis3830 shl overflow
        gcode0 = shl (gcode0, 4);
        *gain_code = (Word16)((gcode0 * *p++)>>15)<<1;
    }
    else
    {
        gcode0 = (Word16)(Pow2 (14, frac));
        L_tmp = (*p++ * gcode0);
//        L_tmp = L_shr(L_tmp, sub(9, exp));
        *gain_code = (Word16)(L_shr(L_tmp, (24-exp)));          /* Q1 */
    }

    /*-------------------------------------------------------------------*
     *  update table of past quantized energies                          *
     *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                          *
     *-------------------------------------------------------------------*/
    qua_ener_MR122 = *p++;
    qua_ener = *p++;
    gc_pred_update(pred_state, qua_ener_MR122, qua_ener);

    return;
}
