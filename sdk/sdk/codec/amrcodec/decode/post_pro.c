/* OK
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : post_pro.c
*      Purpose          : Postprocessing of output speech.
*
*                         - 2nd order high pass filtering with cut
*                           off frequency at 60 Hz.
*                         - Multiplication of output by two.
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "post_pro.h"
//const char post_pro_id[] = "@(#)$Id $" post_pro_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
//#include "count.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
/* filter coefficients (fc = 60 Hz) */
static const Word16 b[3] = {7699, -15398, 7699};
static const Word16 a[3] = {8192, 15836, -7667};

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/
Post_ProcessState  Post_ProcessS;
/*************************************************************************
*
*  Function:   Post_Process_init
*  Purpose:    Allocates state memory and initializes state memory
*
**************************************************************************
*/
void Post_Process_init (Post_ProcessState **state)
{
  Post_ProcessState* s;

//  if (state == (Post_ProcessState **) NULL){
//      fprintf(stderr, "Post_Process_init: invalid parameter\n");
//      return -1;
//  }
//  *state = NULL;

  /* allocate memory */
//  if ((s= (Post_ProcessState *) malloc(sizeof(Post_ProcessState))) == NULL){
//      fprintf(stderr, "Post_Process_init: can not malloc state structure\n");
//      return -1;
//  }

  s = &Post_ProcessS;
  Post_Process_reset(s);
  *state = s;

  return;
}

/*************************************************************************
*
*  Function:   Post_Process_reset
*  Purpose:    Initializes state memory to zero
*
**************************************************************************
*/
void Post_Process_reset (Post_ProcessState *state)
{
//  if (state == (Post_ProcessState *) NULL){
//      fprintf(stderr, "Post_Process_reset: invalid parameter\n");
//      return -1;
//  }

//  state->y2_hi = 0;
//  state->y2_lo = 0;
//  state->y1_hi = 0;
//  state->y1_lo = 0;
  state->x0 = 0;
  state->x1 = 0;

  // 2004.9.22 sis3830
  state->L_y1 = 0;
  state->L_y2 = 0;

  return;
}

/*************************************************************************
*
*  Function:   Post_Process_exit
*  Purpose:    The memory used for state memory is freed
*
**************************************************************************
*/
//void Post_Process_exit (Post_ProcessState **state)
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
 *  FUNCTION:  Post_Process()
 *
 *  PURPOSE: Postprocessing of input speech.
 *
 *  DESCRIPTION:
 *     - 2nd order high pass filtering with cut off frequency at 60 Hz.
 *     - Multiplication of output by two.
 *
 * Algorithm:
 *
 *  y[i] = b[0]*x[i]*2 + b[1]*x[i-1]*2 + b[2]*x[i-2]*2
 *                     + a[1]*y[i-1]   + a[2]*y[i-2];
 *
 *
 *************************************************************************/
void Post_Process (
    Post_ProcessState *st,  /* i/o : post process state                   */
    Word16 signal[],        /* i/o : signal                               */
    Word16 lg               /* i   : length of signal                     */
    )
{

    Word32 i;
    Word16 x2;
    Word32 L_tmp;

    for (i = 0; i < lg; i++)
    {
#ifndef OPRISCASM
        Word64 LL_tmp;
#endif

#if !defined(HAVE_MACLC) || !defined(OPRISCASM)
        Word32 mac_hi;
#endif

        Word32 mac_lo;
        Word32 s;
        x2 = st->x1;
        st->x1 = st->x0;
        st->x0 = signal[i];

/*
        L_tmp = st->y1_hi*a[1] + ((st->y1_lo*a[1])>>15);
        L_tmp += (st->y2_hi*a[2] + ((st->y2_lo*a[2])>>15));
        L_tmp += (st->x0*b[0] + st->x1*b[1] + x2*b[2]);
        L_tmp <<= 2;

        if ( L_tmp > (Word32)0x1fffc000L )
            signal[i] = 0x7fff;
        else if ( L_tmp < (Word32)0xe0000000L )
            signal[i] = (Word16)0x8000;
        else
            signal[i] = (Word16)((L_tmp+0x00002000L)>>14);

        st->y2_hi = st->y1_hi;
        st->y2_lo = st->y1_lo;
        st->y1_hi = (Word16) (L_tmp >> 15);
        st->y1_lo = (Word16) (L_tmp - (Word32)(st->y1_hi<<15));
*/
        /*  y[i] = b[0]*x[i]*2 + b[1]*x[i-1]*2 + b140[2]*x[i-2]/2  */
        /*                     + a[1]*y[i-1] + a[2] * y[i-2];      */

#ifdef OPRISCASM
/*
        Word32 mac_lo;
*/
        asm ("l.mac %0, %1" : : "r"(st->L_y1), "r"((Word32)a[1]));
        asm ("l.macrc %0, 16" : "=r" (L_tmp));
        asm ("l.mac %0, %1" : : "r"(st->L_y2), "r"((Word32)a[2]));
        asm ("l.macrc %0, 16" : "=r" (mac_lo));
        L_tmp += mac_lo;
        asm ("l.mac %0, %1" : : "r"((Word32)st->x0), "r"((Word32)b[0]));
        asm ("l.mac %0, %1" : : "r"((Word32)st->x1), "r"((Word32)b[1]));
        asm ("l.mac %0, %1" : : "r"((Word32)x2),     "r"((Word32)b[2]));
        asm volatile ("l.macrc %0, 0" : "=r" (mac_lo));
        L_tmp += mac_lo;
/*
        asm ("l.mac %0, %1" : : "r"((Word32)1),     "r"((Word32)L_tmp));
        asm volatile ("l.macrc %0, 0" : "=r" (L_tmp));
*/
        asm ("l.mac %0, %1" : : "r"((Word32)1), "r"((Word32)L_tmp));
        asm ("l.mac %0, %1" : : "r"((Word32)1), "r"((Word32)0x00000800L));

#   ifdef HAVE_MACLC
        asm volatile("l.maclc %0, 4" : "=r" (s));
        //signal[i] = mac_lo >> 16;
#   else
        asm volatile("l.mfspr %0, r0, 0x2802" : "=r"(mac_hi));
        asm volatile("l.macrc %0, 0" : "=r" (mac_lo));
#   endif
#else
        LL_tmp = (((Word64) st->L_y1) * a[1]) >> 16;
        L_tmp = (Word32)LL_tmp;
        LL_tmp = (((Word64) st->L_y2) * a[2]) >> 16;
        L_tmp += (Word32)LL_tmp;
        LL_tmp = st->x0*b[0] + st->x1*b[1] + x2*b[2];
        L_tmp += (Word32)LL_tmp;

        LL_tmp = L_tmp + 0x00000800L;
        mac_hi = (Word32)(LL_tmp >> 32) ;
        mac_lo = (Word32)LL_tmp;
#endif

#if !defined(OPRISCASM) || (defined(OPRISCASM) && !defined(HAVE_MACLC))
        _CheckOverflow(mac_hi, mac_lo, s);

        if ( s > (Word32)0x0fffffffL )
            s = (Word32)0x7fffffffL;
        else if ( s < (Word32)0xf0000000L )
            s = (Word32)0x80000000L;
        else
            s <<= 3;
#endif

        signal[i] = s >> 16;

/*
        if ( L_tmp > (Word32)0x7FFF000L )
            signal[i] = 0x7fff;
        else if ( L_tmp < (Word32)0xF8000000L )
            signal[i] = (Word16)0x8000;
        else
            signal[i] = (Word16)((L_tmp+0x00000800L)>>12);
*/
        //L_tmp <<= 2;
        /* Multiplication by two of output speech with saturation. */
        //L_tmp = Word32(LL_tmp);

        st->L_y2 = st->L_y1;
        st->L_y1 = L_tmp << 3;

    }

    return;
}
