/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
*****************************************************************************
*
*      File             : d_plsf.c
*      Purpose          : common part (init, exit, reset) of LSF decoder
*                         module (rest in d_plsf_3.c and d_plsf_5.c)
*
*****************************************************************************
*/

/*
*****************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
*****************************************************************************
*/
#include "d_plsf.h"
const char d_plsf_id[] = "@(#)$Id $" d_plsf_h;

/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
#include "basic_op.h"
//#include "count.h"
#include "cnst.h"
#include "copy.h"
#include "q_plsf_5.tab"

/*
*--------------------------------------------------*
* Constants (defined in cnst.h)                    *
*--------------------------------------------------*
*  M                    : LPC order
*--------------------------------------------------*
*/

/*
*****************************************************************************
*                         LOCAL MEMORY
*****************************************************************************
*/
D_plsfState D_p;
/*
*****************************************************************************
*                         PUBLIC PROGRAM CODE
*****************************************************************************
*/
/*
**************************************************************************
*
*  Function    : D_plsf_init
*  Purpose     : Allocates and initializes state memory
*
**************************************************************************
*/
void D_plsf_init (D_plsfState **state)
{
  D_plsfState* s;

//  if (state == (D_plsfState **) NULL){
//      fprintf(stderr, "D_plsf_init: invalid parameter\n");
//      return -1;
//  }
//  *state = NULL;

  /* allocate memory */
//  if ((s= (D_plsfState *) malloc(sizeof(D_plsfState))) == NULL){
//      fprintf(stderr, "D_plsf_init: can not malloc state structure\n");
//      return -1;
//  }
  s = &D_p;
  D_plsf_reset(s);
  *state = s;

  return;
}

/*
**************************************************************************
*
*  Function    : D_plsf_reset
*  Purpose     : Resets state memory
*
**************************************************************************
*/
void D_plsf_reset (D_plsfState *state)
{
//  Word16 i;

//  if (state == (D_plsfState *) NULL){
//      fprintf(stderr, "D_plsf_reset: invalid parameter\n");
//      return -1;
//  }

  //for (i = 0; i < M; i++){
  //    state->past_r_q[i] = 0;             /* Past quantized prediction error */
  //}
  Preset(0 , state->past_r_q, M);

  /* Past dequantized lsfs */
  Copy(mean_lsf, &state->past_lsf_q[0], M);

  return;
}

/*
**************************************************************************
*
*  Function    : D_plsf_exit
*  Purpose     : The memory used for state memory is freed
*
**************************************************************************
*/
//void D_plsf_exit (D_plsfState **state)
//{
//  if (state == NULL || *state == NULL)
//      return;

  /* deallocate memory */
//  free(*state);
//  *state = NULL;

//  return;
//}
