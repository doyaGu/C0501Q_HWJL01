/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : a_refl.c
*      Purpose          : Convert from direct form coefficients to
*                         reflection coefficients
*
********************************************************************************
*/
/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "a_refl.h"
const char a_refl_id[] = "@(#)$Id $" a_refl_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "cnst.h"
#include "copy.h"
#if WMOPS
#include "count.h"
#endif

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
/*
**************************************************************************
*
*  Function    : A_Refl
*
**************************************************************************
*/
void A_Refl(
   Word16 a[],        /* i   : Directform coefficients */
   Word16 refl[]      /* o   : Reflection coefficients */
)
{
   /* local variables */
   Word32 i,j;
   Word16 aState[M];
   Word16 bState[M];
   Word16 normShift;
   Word16 normProd;
   Word32 L_acc;
   Word16 scale;
   Word32 L_temp;
   //Word16 temp;
   Word16 mult;

   /* initialize states */
//   for (i = 0; i < M; i++)
//   {
//      aState[i] = a[i];
//   }
   Copy( a, aState, M );

   /* backward Levinson recursion */
   for (i = M-1; i >= 0; i--)
   {
      if ((aState[i] >= 4096) || (aState[i] <= -4096))
      {
         goto ExitRefl;
      }

      refl[i] = aState[i] << 3;

      L_acc = MAX_32 - ((refl[i] * refl[i]) << 1);

      normShift = norm_l(L_acc);
      //scale = sub(15, normShift);
      scale = 15 - normShift;

      //L_acc = L_shl(L_acc, normShift);
      //normProd = round16(L_acc);
      L_acc <<= normShift;
      if ( L_acc < (Word32)0x7fff7fffL )
          normProd = (Word16)((L_acc+0x00008000L)>>16);
      else
          normProd = 0x7fff;

      mult = div_s(16384, normProd);

      for (j = 0; j < i; j++)
      {
         // sis3830 L_deposit_h overflow
         //L_acc = L_deposit_h(aState[j]);
         L_acc = aState[j] << 16;
         //L_acc = L_msu(L_acc, refl[i], aState[i-j-1]);
         L_acc = L_acc - ( (refl[i] * aState[i-j-1]) << 1) ;

         // sis3830 Q
         //temp = round16(L_acc);
         //L_temp = L_mult(mult, temp);
         L_temp = (mult * round16(L_acc)) << 1;
         //L_temp = L_shr_r(L_temp, scale);
         L_temp >>= (scale - 1);
         L_temp = (L_temp+1) >> 1;

         if ((L_temp > 32767) || (L_temp < -32767))
         {
            goto ExitRefl;
         }

         bState[j] = (Word16)(L_temp);
      }

      //for (j = 0; j < i; j++)
      //{
      //   aState[j] = bState[j];
      //}
      Copy(bState, aState, i);
   }
   return;

ExitRefl:
//   for (i = 0; i < M; i++)
//   {
//      refl[i] = 0;
//   }
   Preset( 0, refl, M );
}
