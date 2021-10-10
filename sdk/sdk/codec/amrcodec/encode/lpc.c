/*  OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : lpc.c
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "lpc.h"
const char lpc_id[] = "@(#)$Id $" lpc_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
//#include <stdio.h>
//#include <stdlib.h>
#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "autocorr.h"
#include "lag_wind.h"
#include "levinson.h"
#include "cnst.h"
#include "mode.h"
//#include "count.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
#include "window.tab"

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
lpcState lpcS;

/*************************************************************************
*
*  Function:   lpc_init
*
**************************************************************************
*/
int lpc_init (lpcState **state)
{
  lpcState* s;

//  if (state == (lpcState **) NULL){
//      fprintf(stderr, "lpc_init: invalid parameter\n");
//      return -1;
//  }
//  *state = NULL;

  /* allocate memory */
//  if ((s= (lpcState *) malloc(sizeof(lpcState))) == NULL){
//      fprintf(stderr, "lpc_init: can not malloc state structure\n");
//      return -1;
//  }

//  s->levinsonSt = NULL;

  s = &lpcS;

  /* Init sub states */
  Levinson_init(&s->levinsonSt);
  //if (Levinson_init(&s->levinsonSt)) {
  //   lpc_exit(&s);
  //   return -1;
  //}

  lpc_reset(s);
  *state = s;

  return 0;
}

/*************************************************************************
*
*  Function:   lpc_reset
*
**************************************************************************
*/
int lpc_reset (lpcState *state)
{

//  if (state == (lpcState *) NULL){
//      fprintf(stderr, "lpc_reset: invalid parameter\n");
//      return -1;
//  }

  Levinson_reset(state->levinsonSt);

  return 0;
}

/*************************************************************************
*
*  Function:   lpc_exit
*
**************************************************************************
*/
//void lpc_exit (lpcState **state)
//{
//  if (state == NULL || *state == NULL)
//      return;

//  Levinson_exit(&(*state)->levinsonSt);

  /* deallocate memory */
//  free(*state);
//  *state = NULL;

//  return;
//}

int lpc(
    lpcState *st,     /* i/o: State struct                */
    enum Mode mode,   /* i  : coder mode                  */
    Word16 x[],       /* i  : Input signal           Q15  */
    Word16 x_12k2[],  /* i  : Input signal (EFR)     Q15  */
    Word16 a[]        /* o  : predictor coefficients Q12  */
)
{
   Word16 rc[4];                  /* First 4 reflection coefficients Q15 */
   Word16 rLow[MP1], rHigh[MP1];  /* Autocorrelations low and hi      */
                                  /* No fixed Q value but normalized  */
                                  /* so that overflow is avoided      */

   if ((Word16)mode == MR122)
   {
       /* Autocorrelations */
       Autocorr(x_12k2, M, rHigh, rLow, window_160_80);
       /* Lag windowing    */
       Lag_window(M, rHigh, rLow);
       /* Levinson Durbin  */
       Levinson(st->levinsonSt, rHigh, rLow, &a[MP1], rc);

       /* Autocorrelations */
       Autocorr(x_12k2, M, rHigh, rLow, window_232_8);
       /* Lag windowing    */
       Lag_window(M, rHigh, rLow);
       /* Levinson Durbin  */
       Levinson(st->levinsonSt, rHigh, rLow, &a[MP1 * 3], rc);
   }
   else
   {
       /* Autocorrelations */
       Autocorr(x, M, rHigh, rLow, window_200_40);
       /* Lag windowing    */
       Lag_window(M, rHigh, rLow);
       /* Levinson Durbin  */
       Levinson(st->levinsonSt, rHigh, rLow, &a[MP1 * 3], rc);
   }

   return 0;
}
