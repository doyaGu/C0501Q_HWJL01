/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : gainQuant.h
*      Purpose          : Quantazation of gains
*
********************************************************************************
*/
#ifndef gain_q_h
#define gain_q_h "$Id $"

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "mode.h"
#include "gc_pred.h"
#include "g_adapt.h"

/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/
typedef struct {
    Word16 sf0_exp_gcode0;
    Word16 sf0_frac_gcode0;
    Word16 sf0_exp_target_en;
    Word16 sf0_frac_target_en;
    Word16 sf0_exp_coeff[5];
    Word16 sf0_frac_coeff[5];
    Word16 *gain_idx_ptr;

    gc_predState     *gc_predSt;
    gc_predState     *gc_predUnqSt;
    GainAdaptState   *adaptSt;
} gainQuantState;

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
int gainQuant_init (gainQuantState **st);
/* initialize one instance of the pre processing state.
   Stores pointer to filter status struct in *st. This pointer has to
   be passed to gainQuant in each call.
   returns 0 on success
 */
int gainQuant_reset (gainQuantState *st);
/* reset of pre processing state (i.e. set state memory to zero)
   returns 0 on success
 */
//void gainQuant_exit (gainQuantState **st);
/* de-initialize pre processing state (i.e. free status struct)
   stores NULL in *st
 */

int gainQuant(
    gainQuantState *st,   /* i/o : State struct                      */
    enum Mode mode,       /* i   : coder mode                        */
    Word16 res[],         /* i   : LP residual,                 Q0   */
    Word16 exc[],         /* i   : LTP excitation (unfiltered), Q0   */
    Word16 code[],        /* i   : CB innovation (unfiltered),  Q13  */
                          /*       (unsharpened for MR475)           */
    Word16 xn[],          /* i   : Target vector.                    */
    Word16 xn2[],         /* i   : Target vector.                    */
    Word16 y1[],          /* i   : Adaptive codebook.                */
    Word16 Y2[],          /* i   : Filtered innovative vector.       */
    Word16 g_coeff[],     /* i   : Correlations <xn y1> <y1 y1>      */
                          /*       Compute in G_pitch().             */
    Word16 even_subframe, /* i   : even subframe indicator flag      */
    Word16 gp_limit,      /* i   : pitch gain limit                  */
    Word16 *sf0_gain_pit, /* o   : Pitch gain sf 0.   MR475          */
    Word16 *sf0_gain_cod, /* o   : Code gain sf 0.    MR475          */
    Word16 *gain_pit,     /* i/o : Pitch gain.                       */
    Word16 *gain_cod,     /* o   : Code gain.                        */
                          /*       MR475: gain_* unquantized in even */
                          /*       subframes, quantized otherwise    */
    Word16 **anap         /* o   : Index of quantization             */
);

#endif
