/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : d_plsf.h
*      Purpose          : common part (init, exit, reset) of LSF decoder
*                         module (rest in d_plsf_3.c and d_plsf_5.c)
*
********************************************************************************
*/
#ifndef d_plsf_h
#define d_plsf_h "$Id $"

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "cnst.h"
#include "mode.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
/*
*--------------------------------------------------*
* Constants (defined in cnst.h)                    *
*--------------------------------------------------*
* M            : LPC Order                         *
*--------------------------------------------------*
*/

/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/
typedef struct {
  Word16 past_r_q[M];   /* Past quantized prediction error, Q15 */
  Word16 past_lsf_q[M]; /* Past dequantized lsfs,           Q15 */
} D_plsfState;

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/

/*
**************************************************************************
*
*  Function    : D_plsf_init
*  Purpose     : Allocates and initializes state memory
*  Description : Stores pointer to state struct in *st. This
*                pointer has to be passed to D_plsf in each call.
*  Returns     : 0 on success
*
**************************************************************************
*/
void D_plsf_init (D_plsfState **st);

/*
**************************************************************************
*
*  Function    : D_plsf_reset
*  Purpose     : Resets state memory
*  Returns     : 0 on success
*
**************************************************************************
*/
void D_plsf_reset (D_plsfState *st);

/*
**************************************************************************
*
*  Function    : D_plsf_exit
*  Purpose     : The memory used for state memory is freed
*  Description : Stores NULL in *st
*  Returns     : void
*
**************************************************************************
*/
void D_plsf_exit (D_plsfState **st);

/*
**************************************************************************
*
*  Function    : D_plsf_5
*  Purpose     : Decodes the 2 sets of LSP parameters in a frame
*                using the received quantization indices.
*  Description : The two sets of LSFs are quantized using split by
*                5 matrix quantization (split-MQ) with 1st order MA
*                prediction.
*                See "q_plsf_5.c" for more details about the
*                quantization procedure
*  Returns     : 0
*
**************************************************************************
*/
int D_plsf_5 (
    D_plsfState *st,  /* i/o: State variables                            */
    Word16 bfi,       /* i  : bad frame indicator (set to 1 if a bad
                              frame is received)                         */
    Word16 *indice,   /* i  : quantization indices of 5 submatrices, Q0  */
    Word16 *lsp1_q,   /* o  : quantized 1st LSP vector (M)           Q15 */
    Word16 *lsp2_q    /* o  : quantized 2nd LSP vector (M)           Q15 */
);

/*************************************************************************
 *
 *  FUNCTION:   D_plsf_3()
 *
 *  PURPOSE: Decodes the LSP parameters using the received quantization
 *           indices.1st order MA prediction and split by 3 matrix
 *           quantization (split-MQ)
 *
 *************************************************************************/

void D_plsf_3(
    D_plsfState *st,  /* i/o: State struct                               */
    enum Mode mode,   /* i  : coder mode                                 */
    Word16 bfi,       /* i  : bad frame indicator (set to 1 if a         */
                      /*      bad frame is received)                     */
    Word16 * indice,  /* i  : quantization indices of 3 submatrices, Q0  */
    Word16 * lsp1_q   /* o  : quantized 1st LSP vector,              Q15 */
);

/*************************************************************************
 *
 *  FUNCTION:   Init_D_plsf_3()
 *
 *  PURPOSE: Set the past_r_q[M] vector to one of the eight
 *           past_rq_init vectors.
 *
 *************************************************************************/
void Init_D_plsf_3(D_plsfState *st,  /* i/o: State struct                */
           Word16 index      /* i  : past_rq_init[] index [0, 7] */
);

#endif
