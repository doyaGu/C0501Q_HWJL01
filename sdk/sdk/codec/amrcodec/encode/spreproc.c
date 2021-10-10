﻿/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : spreproc.c
*      Purpose          : Subframe preprocessing
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "spreproc.h"
const char spreproc_id[] = "@(#)$Id $" spreproc_h;

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
#include "weight_a.h"
#include "syn_filt.h"
#include "residu.h"
#include "copy.h"
//#include "count.h"

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
int subframePreProc(
    enum Mode mode,            /* i  : coder mode                            */
    const Word16 gamma1[],     /* i  : spectral exp. factor 1                */
    const Word16 gamma1_12k2[],/* i  : spectral exp. factor 1 for EFR        */
    const Word16 gamma2[],     /* i  : spectral exp. factor 2                */
    Word16 *A,                 /* i  : A(z) unquantized for the 4 subframes  */
    Word16 *Aq,                /* i  : A(z)   quantized for the 4 subframes  */
    Word16 *speech,            /* i  : speech segment                        */
    Word16 *mem_err,           /* i  : pointer to error signal               */
    Word16 *mem_w0,            /* i  : memory of weighting filter            */
    Word16 *zero,              /* i  : pointer to zero vector                */
    Word16 ai_zero[],          /* o  : history of weighted synth. filter     */
    Word16 exc[],              /* o  : long term prediction residual         */
    Word16 h1[],               /* o  : impulse response                      */
    Word16 xn[],               /* o  : target vector for pitch search        */
    Word16 res2[],             /* o  : long term prediction residual         */
    Word16 error[]             /* o  : error of LPC synthesis filter         */
)
{
   Word16 Ap1[MP1];              /* A(z) with spectral expansion         */
   Word16 Ap2[MP1];              /* A(z) with spectral expansion         */
   const Word16 *g1;             /* Pointer to correct gammma1 vector    */

   //Word32 tmp[M+L_SUBFR];
   //Word32 i;
   /*---------------------------------------------------------------*
    * mode specific pointer to gamma1 values                        *
    *---------------------------------------------------------------*/

    //if ( mode == MR122 || mode == MR102 )
    if ( ((Word16)mode - MR122) == 0 || ((Word16)mode - MR102) == 0 )
    g1 = gamma1_12k2;
    else
        g1 = gamma1;
   /*---------------------------------------------------------------*
    * Find the weighted LPC coefficients for the weighting filter.  *
    *---------------------------------------------------------------*/
   Weight_Ai(A, g1, Ap1);
   Weight_Ai(A, gamma2, Ap2);

   /*---------------------------------------------------------------*
    * Compute impulse response, h1[], of weighted synthesis filter  *
    *---------------------------------------------------------------*/
//   for (i = 0; i <= M; i++)
//   {
//      ai_zero[i] = Ap1[i];        move16 ();
//   }
   Copy( Ap1, ai_zero, M+1 );

   Syn_filt(Aq, ai_zero, h1, L_SUBFR, zero, 0);
   Syn_filt(Ap2, h1, h1, L_SUBFR, zero, 0);

   /*------------------------------------------------------------------------*
    *                                                                        *
    *          Find the target vector for pitch search:                      *
    *                                                                        *
    *------------------------------------------------------------------------*/

   /* LPC residual */
   Residu(Aq, speech, res2, L_SUBFR);
#ifdef SPREPRO_CPY
   memcpy16(exc, res2, L_SUBFR);
#else
   Copy(res2, exc, L_SUBFR);
#endif

   Syn_filt(Aq, exc, error, L_SUBFR, mem_err, 0);

   Residu(Ap1, error, xn, L_SUBFR);

   /* target signal xn[]*/
   Syn_filt(Ap2, xn, xn, L_SUBFR, mem_w0, 0);

   return 0;
}
