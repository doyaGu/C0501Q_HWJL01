/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : q_plsf.c
*      Purpose          : common part (init, exit, reset) of LSF quantization
*                         module (rest in q_plsf_3.c and q_plsf_5.c)
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "q_plsf.h"
const char q_plsf_id[] = "@(#)$Id $" q_plsf_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
#include "basic_op.h"

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
Q_plsfState Q_plsfS;
/*
**************************************************************************
*
*  Function    : Q_plsf_init
*  Purpose     : Allocates memory and initializes state variables
*
**************************************************************************
*/
int Q_plsf_init (Q_plsfState **state)
{
  Q_plsfState* s;

//  if (state == (Q_plsfState **) NULL){
//      fprintf(stderr, "Q_plsf_init: invalid parameter\n");
//      return -1;
//  }
//  *state = NULL;

  /* allocate memory */
//  if ((s= (Q_plsfState *) malloc(sizeof(Q_plsfState))) == NULL){
//      fprintf(stderr, "Q_plsf_init: can not malloc state structure\n");
//      return -1;
//  }

  s = &Q_plsfS;
  Q_plsf_reset(s);
  *state = s;

  return 0;
}

/*
**************************************************************************
*
*  Function    : Q_plsf_reset
*  Purpose     : Resets state memory
*
**************************************************************************
*/
int Q_plsf_reset (Q_plsfState *state)
{
  Word16 i;

//  if (state == (Q_plsfState *) NULL){
//      fprintf(stderr, "Q_plsf_reset: invalid parameter\n");
//      return -1;
//  }

  for ( i = 0; i < M; i++)
      state->past_rq[i] = 0;

  return 0;
}

/*
**************************************************************************
*
*  Function    : Q_plsf_exit
*  Purpose     : The memory used for state memory is freed
*
**************************************************************************
*/
//void Q_plsf_exit (Q_plsfState **state)
//{
//  if (state == NULL || *state == NULL)
//      return;

  /* deallocate memory */
//  free(*state);
//  *state = NULL;

//  return;
//}
