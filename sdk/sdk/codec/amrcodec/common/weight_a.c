/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : weight_a.c
*      Purpose          : Spectral expansion of LP coefficients.  (order==10)
*      Description      : a_exp[i] = a[i] * fac[i-1]    ,i=1,10
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "weight_a.h"
const char weight_a_id[] = "@(#)$Id $" weight_a_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
//#include "basic_op.h"
//#include "count.h"
#include "cnst.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
/*
*--------------------------------------*
* Constants (defined in cnst.h         *
*--------------------------------------*
*  M         : LPC order               *
*--------------------------------------*
*/

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
void Weight_Ai (
    Word16 a[],         /* (i)     : a[M+1]  LPC coefficients   (M=10)    */
    const Word16 fac[], /* (i)     : Spectral expansion factors.          */
    Word16 a_exp[]      /* (o)     : Spectral expanded LPC coefficients   */
)
{
    //Word32 i;
    //Word32 L_temp;

    a_exp[0] = a[0];
/*   for (i = 1; i <= M; i++)
    {
        L_temp = a[i] * fac[i-1];
        a_exp[i] = (Word16)( (L_temp + 0x00004000L) >> 15 );
    }
*/
#ifdef NO_ROUNDING
    a_exp[1]=(Word16)(((a[1] * fac[0]))>>15) ;
    a_exp[2]=(Word16)(((a[2] * fac[1]))>>15) ;
    a_exp[3]=(Word16)(((a[3] * fac[2]))>>15) ;
    a_exp[4]=(Word16)(((a[4] * fac[3]))>>15) ;
    a_exp[5]=(Word16)(((a[5] * fac[4]))>>15) ;
    a_exp[6]=(Word16)(((a[6] * fac[5]))>>15) ;
    a_exp[7]=(Word16)(((a[7] * fac[6]))>>15) ;
    a_exp[8]=(Word16)(((a[8] * fac[7]))>>15) ;
    a_exp[9]=(Word16)(((a[9] * fac[8]))>>15) ;
    a_exp[10]=(Word16)(((a[10] * fac[9]))>>15) ;
#else
    a_exp[1]=(Word16)(((a[1] * fac[0]) + 0x00004000L)>>15) ;
    a_exp[2]=(Word16)(((a[2] * fac[1]) + 0x00004000L)>>15) ;
    a_exp[3]=(Word16)(((a[3] * fac[2]) + 0x00004000L)>>15) ;
    a_exp[4]=(Word16)(((a[4] * fac[3]) + 0x00004000L)>>15) ;
    a_exp[5]=(Word16)(((a[5] * fac[4]) + 0x00004000L)>>15) ;
    a_exp[6]=(Word16)(((a[6] * fac[5]) + 0x00004000L)>>15) ;
    a_exp[7]=(Word16)(((a[7] * fac[6]) + 0x00004000L)>>15) ;
    a_exp[8]=(Word16)(((a[8] * fac[7]) + 0x00004000L)>>15) ;
    a_exp[9]=(Word16)(((a[9] * fac[8]) + 0x00004000L)>>15) ;
    a_exp[10]=(Word16)(((a[10] * fac[9]) + 0x00004000L)>>15) ;
#endif
    return;
}

void Weight_Ai2 (
    Word16 a[],         /* (i)     : a[M+1]  LPC coefficients   (M=10)    */
    const Word16 fac[], /* (i)     : Spectral expansion factors.          */
    const Word16 fac2[],
    Word16 a_exp[],      /* (o)     : Spectral expanded LPC coefficients   */
    Word16 a_exp2[]
)
{
    Word32 i;
    Word32 L_temp;

    a_exp[0] = a[0];
    a_exp2[0] = a[0];
    for (i = 1; i <= M; i++)
    {
        L_temp = a[i] * fac[i-1];
        a_exp[i] = (Word16)( (L_temp + 0x00004000L) >> 15 );
        L_temp = a[i] * fac2[i-1];
        a_exp2[i] = (Word16)( (L_temp + 0x00004000L) >> 15 );
    }

    return;
}
