﻿/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : weight_a.h
*      Purpose          : Spectral expansion of LP coefficients.  (order==10)
*      Description      : a_exp[i] = a[i] * fac[i-1]    ,i=1,10
*
*
********************************************************************************
*/
#ifndef weight_a_h
#define weight_a_h "$Id $"

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"

/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/

void Weight_Ai (
    Word16 a[],        /* (i)  : a[m+1]  LPC coefficients   (m=10)          */
    const Word16 fac[],/* (i)  : Spectral expansion factors.                */
    Word16 a_exp[]     /* (o)  : Spectral expanded LPC coefficients         */
);

void Weight_Ai2 (
    Word16 a[],         /* (i)     : a[M+1]  LPC coefficients   (M=10)    */
    const Word16 fac[], /* (i)     : Spectral expansion factors.          */
    const Word16 fac2[],
    Word16 a_exp[],      /* (o)     : Spectral expanded LPC coefficients   */
    Word16 a_exp2[]
);

#endif
