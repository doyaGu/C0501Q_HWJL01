/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : pre_proc.c
*      Purpose          : Preprocessing of input speech.
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "pre_proc.h"
const char pre_proc_id[] = "@(#)$Id $" pre_proc_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
//#include "basic_op.h"
//#include "oper_32b.h"
//#include "count.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
/* filter coefficients (fc = 80 Hz, coeff. b[] is divided by 2) */
static const Word16 b[3] = {1899, -3798, 1899};
static const Word16 a[3] = {4096, 7807, -3733};

static Pre_ProcessState Pre_Proc;

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
/*************************************************************************
*
*  Function:   Pre_Process_init
*  Purpose:    Allocates state memory and initializes state memory
*
**************************************************************************
*/
int Pre_Process_init (Pre_ProcessState **state)
{
//  Pre_ProcessState* s;

//  if (state == (Pre_ProcessState **) NULL){
//      fprintf(stderr, "Pre_Process_init: invalid parameter\n");
//      return -1;
//  }
//  *state = NULL;

  /* allocate memory */
//  if ((s= (Pre_ProcessState *) malloc(sizeof(Pre_ProcessState))) == NULL){
//      fprintf(stderr, "Pre_Process_init: can not malloc state structure\n");
//      return -1;
//  }

//  Pre_Process_reset(s);

//  *state = s;

    Pre_Process_reset(&Pre_Proc);
    *state = &Pre_Proc;

  return 0;
}

/*************************************************************************
*
*  Function:   Pre_Process_reset
*  Purpose:    Initializes state memory to zero
*
**************************************************************************
*/
int Pre_Process_reset (Pre_ProcessState *state)
{
//  if (state == (Pre_ProcessState *) NULL){
//      fprintf(stderr, "Pre_Process_reset: invalid parameter\n");
//      return -1;
//  }

  state->y2_hi = 0;
  state->y2_lo = 0;
  state->y1_hi = 0;
  state->y1_lo = 0;
  //state->L_y2 = 0;
  //state->L_y1 = 0;
  state->x0 = 0;
  state->x1 = 0;

  return 0;
}

/*************************************************************************
*
*  Function:   Pre_Process_exit
*  Purpose:    The memory used for state memory is freed
*
**************************************************************************
*/
//void Pre_Process_exit (Pre_ProcessState **state)
//{
//  if (state == NULL || *state == NULL)
//      return;

  /* deallocate memory */
//  free(*state);
//  *state = NULL;

//  return;
//}

/*************************************************************************
 *
 *  FUNCTION:  Pre_Process()
 *
 *  PURPOSE: Preprocessing of input speech.
 *
 *  DESCRIPTION:
 *     - 2nd order high pass filtering with cut off frequency at 80 Hz.
 *     - Divide input by two.
 *
 *
 * Algorithm:
 *
 *  y[i] = b[0]*x[i]/2 + b[1]*x[i-1]/2 + b[2]*x[i-2]/2
 *                     + a[1]*y[i-1]   + a[2]*y[i-2];
 *
 *
 *  Input is divided by two in the filtering process.
 *
 *************************************************************************/
int Pre_Process (
    Pre_ProcessState *st,
    Word16 signal[], /* input/output signal */
    Word16 lg)       /* lenght of signal    */
{
    Word16 x2;
    Word32 L_tmp,i;

    for (i = 0; i < lg; i++)
    {
        x2 = st->x1;
        st->x1 = st->x0;
        st->x0 = signal[i];

        /*  y[i] = b[0]*x[i]/2 + b[1]*x[i-1]/2 + b140[2]*x[i-2]/2  */
        /*                     + a[1]*y[i-1] + a[2] * y[i-2];      */

        L_tmp = (st->y1_hi * a[1]) + ((st->y1_lo * a[1])>>15);
        L_tmp += ( (st->y2_hi * a[2]) + ((st->y2_lo * a[2])>>15));
        L_tmp += ((Word32)st->x0 * b[0]);
        L_tmp += ((Word32)st->x1 * b[1]);
        L_tmp += ((Word32)x2 * b[2]);
        //if ( L_tmp > (Word32)0x0fffffffL )
        //  L_tmp = (Word32)0x7fffffffL;
        //else if ( L_tmp < (Word32)0xf0000000L )
        //  L_tmp = (Word32)0x80000000L;
        //else
            L_tmp <<= 3;
        signal[i]= (Word16)((L_tmp+(Word32)0x00004000L)>>15);

        st->y2_hi = st->y1_hi;
        st->y2_lo = st->y1_lo;
        st->y1_hi = (Word16)(L_tmp >> 15);
        st->y1_lo = (Word16)(L_tmp - (((Word32)(st->y1_hi))<<15));

//        L_tmp = Mpy_32_16 (st->y1_hi, st->y1_lo, a[1]);
//        L_tmp = L_add (L_tmp, Mpy_32_16 (st->y2_hi, st->y2_lo, a[2]));
//        L_tmp = L_mac (L_tmp, st->x0, b[0]);
//        L_tmp = L_mac (L_tmp, st->x1, b[1]);
//        L_tmp = L_mac (L_tmp, x2, b[2]);
//        L_tmp = L_shl (L_tmp, 3);
//        signal[i] = round (L_tmp);

//        st->y2_hi = st->y1_hi;
//        st->y2_lo = st->y1_lo;
//        L_Extract (L_tmp, &st->y1_hi, &st->y1_lo);
    }
    return 0;
}
