/* OK
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
*****************************************************************************
*
*      File             : lsp_avg.c
*      Purpose:         : LSP averaging and history
*
*****************************************************************************
*/

/*
*****************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
*****************************************************************************
*/
#include "lsp_avg.h"
const char lsp_avg_id[] = "@(#)$Id $" lsp_avg_h;

/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "basic_op.h"
//#include "oper_32b.h"
//#include "count.h"
#include "q_plsf_5.tab"
#include "copy.h"

/*
*****************************************************************************
*                         LOCAL VARIABLES AND TABLES
*****************************************************************************
*/
lsp_avgState lsp_avgS;
/*
*****************************************************************************
*                         PUBLIC PROGRAM CODE
*****************************************************************************
*/
/*
**************************************************************************
*
*  Function    : lsp_avg_init
*  Purpose     : Allocates memory and initializes state variables
*
**************************************************************************
*/
void lsp_avg_init (lsp_avgState **state)
{
  lsp_avgState* s;

//  if (state == (lsp_avgState **) NULL){
//      fprintf(stderr, "lsp_avg_init: invalid parameter\n");
//      return -1;
//  }
//  *state = NULL;

  /* allocate memory */
//  if ((s = (lsp_avgState *) malloc(sizeof(lsp_avgState))) == NULL){
//      fprintf(stderr, "lsp_avg_init: can not malloc state structure\n");
//      return -1;
//  }

  s = &lsp_avgS;
  lsp_avg_reset(s);
  *state = s;

  return;
}

/*
**************************************************************************
*
*  Function    : lsp_avg_reset
*  Purpose     : Resets state memory
*
**************************************************************************
*/
void lsp_avg_reset (lsp_avgState *st)
{
//  if (st == (lsp_avgState *) NULL){
//      fprintf(stderr, "lsp_avg_reset: invalid parameter\n");
//      return -1;
//  }

  Copy(mean_lsf, &st->lsp_meanSave[0], M);

  return;
}

/*
**************************************************************************
*
*  Function    : lsp_avg_exit
*  Purpose     : The memory used for state memory is freed
*
**************************************************************************
*/
//void lsp_avg_exit (lsp_avgState **state)
//{
//  if (state == NULL || *state == NULL)
//      return;

  /* deallocate memory */
//  free(*state);
//  *state = NULL;

//  return;
//}

/*
**************************************************************************
*
*  Function    : lsp_avg
*  Purpose     : Calculate the LSP averages
*
**************************************************************************
*/

void lsp_avg (
    lsp_avgState *st,         /* i/o : State struct                 Q15 */
    Word16 *lsp               /* i   : state of the state machine   Q15 */
)
{
    Word32 i;
    Word32 L_tmp;            /* Q31 */

    for (i = 0; i < M; i++) {

//       /* mean = 0.84*mean */
//       L_tmp = L_deposit_h(st->lsp_meanSave[i]);
//       L_tmp -= (EXPCONST * st->lsp_meanSave[i]);
//
//       /* Add 0.16 of newest LSPs to mean */
//       L_tmp += (EXPCONST * lsp[i]);

//       /* Save means */
//       st->lsp_meanSave[i] = round16(L_tmp);            /* Q15 */

        /* mean = 0.84*mean */
        L_tmp = -(EXPCONST * st->lsp_meanSave[i]);

        /* Add 0.16 of newest LSPs to mean */
        L_tmp += (EXPCONST * lsp[i]);
        /* Save means */
        st->lsp_meanSave[i] = (Word16)( ((L_tmp+0x00004000L)>>15) + st->lsp_meanSave[i]);
    }

    return;
}
