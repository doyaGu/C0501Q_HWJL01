/* OK
*************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : pstfilt.c
*      Purpose          : Performs adaptive postfiltering on the synthesis
*                       : speech
*
********************************************************************************
*/

/*
********************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
********************************************************************************
*/
#include "pstfilt.h"
const char pstfilt_id[] = "@(#)$Id $" pstfilt_h;

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
#include "mode.h"
#include "basic_op.h"
//#include "set_zero.h"
#include "weight_a.h"
#include "residu.h"
#include "copy.h"
#include "syn_filt.h"
#include "preemph.h"
//#include "count.h"
#include "cnst.h"

#if WMOPS
#include "count.h"
#endif
/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
/*---------------------------------------------------------------*
 *    Postfilter constant parameters (defined in "cnst.h")       *
 *---------------------------------------------------------------*
 *   L_FRAME     : Frame size.                                   *
 *   L_SUBFR     : Sub-frame size.                               *
 *   M           : LPC order.                                    *
 *   MP1         : LPC order+1                                   *
 *   MU          : Factor for tilt compensation filter           *
 *   AGC_FAC     : Factor for automatic gain control             *
 *---------------------------------------------------------------*/

#define L_H 22  /* size of truncated impulse response of A(z/g1)/A(z/g2) */

Post_FilterState  Post_FilterS;

/* Spectral expansion factors */
static const Word16 gamma3_MR122[M] = {
  22938, 16057, 11240, 7868, 5508,
  3856, 2699, 1889, 1322, 925
};

static const Word16 gamma3[M] = {
  18022, 9912, 5451, 2998, 1649, 907, 499, 274, 151, 83
};

static const Word16 gamma4_MR122[M] = {
  24576, 18432, 13824, 10368, 7776,
  5832, 4374, 3281, 2461, 1846
};

static const Word16 gamma4[M] = {
  22938, 16057, 11240, 7868, 5508, 3856, 2699, 1889, 1322, 925
};

/*
********************************************************************************
*                         PUBLIC PROGRAM CODE
********************************************************************************
*/

/*************************************************************************
*
*  Function:   Post_Filter_init
*  Purpose:    Allocates memory for filter structure and initializes
*              state memory
*
**************************************************************************
*/
void Post_Filter_init (Post_FilterState **state)
{
  Post_FilterState* s;

//  if (state == (Post_FilterState **) NULL){
//      fprintf(stderr, "Post_Filter_init: invalid parameter\n");
//      return -1;
//  }
//  *state = NULL;

  /* allocate memory */
//  if ((s= (Post_FilterState *) malloc(sizeof(Post_FilterState))) == NULL){
//      fprintf(stderr, "Post_Filter_init: can not malloc state structure\n");
//      return -1;
//  }
  s = &Post_FilterS;

  //s->preemph_state = NULL;
  //s->agc_state = NULL;
  s->preemph_state = 0;
  s->agc_state = 0;

//  if (preemphasis_init(&s->preemph_state) || agc_init(&s->agc_state)) {
//      Post_Filter_exit(&s);
//      return -1;
//  }

  preemphasis_init(&s->preemph_state);
  agc_init(&s->agc_state);

  Post_Filter_reset(s);
  *state = s;

  return;
}

/*************************************************************************
*
*  Function:   Post_Filter_reset
*  Purpose:    Initializes state memory to zero
*
**************************************************************************
*/
void Post_Filter_reset (Post_FilterState *state)
{
//  if (state == (Post_FilterState *) NULL){
//      fprintf(stderr, "Post_Filter_reset: invalid parameter\n");
//      return -1;
//  }

//  Set_zero (state->mem_syn_pst, M);
//  Set_zero (state->res2, L_SUBFR);
//  Set_zero (state->synth_buf, L_FRAME + M);
  Preset (0, state->mem_syn_pst, M);
  Preset (0, state->res2, L_SUBFR);
  Preset (0, state->synth_buf, L_FRAME + M);

  agc_reset(state->agc_state);
  preemphasis_reset(state->preemph_state);

  return;
}

/*************************************************************************
*
*  Function:   Post_Filter_exit
*  Purpose:    The memory used for state memory is freed
*
**************************************************************************
*/
//void Post_Filter_exit (Post_FilterState **state)
//{
//  if (state == NULL || *state == NULL)
//      return;

//  agc_exit(&(*state)->agc_state);
//  preemphasis_exit(&(*state)->preemph_state);

  /* deallocate memory */
//  free(*state);
//  *state = NULL;

//  return;
//}

/*
**************************************************************************
*  Function:  Post_Filter
*  Purpose:   postfiltering of synthesis speech.
*  Description:
*      The postfiltering process is described as follows:
*
*          - inverse filtering of syn[] through A(z/0.7) to get res2[]
*          - tilt compensation filtering; 1 - MU*k*z^-1
*          - synthesis filtering through 1/A(z/0.75)
*          - adaptive gain control
*
**************************************************************************
*/
void Post_Filter (
    Post_FilterState *st, /* i/o : post filter states                        */
    enum Mode mode,       /* i   : AMR mode                                  */
    Word16 *syn,          /* i/o : synthesis speech (postfiltered is output) */
    Word16 *Az_4          /* i   : interpolated LPC parameters in all subfr. */
)
{
    /*-------------------------------------------------------------------*
     *           Declaration of parameters                               *
     *-------------------------------------------------------------------*/

    Word16 Ap3[MP1], Ap4[MP1];  /* bandwidth expanded LP parameters */
    Word16 *Az;                 /* pointer to Az_4:                 */
                                /*  LPC parameters in each subframe */
    Word32 i_subfr;             /* index for beginning of subframe  */
    Word16 h[L_H];

    Word32 i;
    Word16 temp1, temp2;
    //Word16 temp3;
    //Word32 L_tmp;
    //Word32 temp4;
    Word16 *syn_work = &st->synth_buf[M];
    Word16 tmp[80];
    //Word16 *tmp;
    Word32 L_tmp1, L_tmp2;

    const Word16 *pD_gamma3, *pD_gamma4;

    if (((Word16)mode == MR122) || ((Word16)mode == MR102))
    {
        pD_gamma3 = gamma3_MR122;
        pD_gamma4 = gamma4_MR122;
    }
    else
    {
        pD_gamma3 = gamma3;
        pD_gamma4 = gamma4;
    }

    /*-----------------------------------------------------*
     * Post filtering                                      *
     *-----------------------------------------------------*/

    Copy (syn, syn_work , L_FRAME);

    Az = Az_4;

    for (i_subfr = 0; i_subfr < L_FRAME; i_subfr += L_SUBFR)
    {
#ifndef OPRISCASM
        Word64 s;
#endif
        /* Find weighted filter coefficients Ap3[] and ap[4] */
        Weight_Ai (Az, pD_gamma3, Ap3);
        Weight_Ai (Az, pD_gamma4, Ap4);

        /* tilt compensation filter */

        /* impulse response of A(z/0.7)/A(z/0.75) */

        Copy (Ap3, h, M + 1);
        Preset (0, &h[M + 1], L_H - M - 1);

        Copy( &h[M+1], tmp, M);

#ifdef OPRISCASM
        asm ("l.mac %0, %1" : : "r"((Word32)Ap4[0] ), "r"((Word32)h[0]  ));
        asm ("l.msb %0, %1" : : "r"((Word32)Ap4[1] ), "r"((Word32)tmp[9]));
        asm ("l.msb %0, %1" : : "r"((Word32)Ap4[2] ), "r"((Word32)tmp[8]));
        asm ("l.msb %0, %1" : : "r"((Word32)Ap4[3] ), "r"((Word32)tmp[7]));
        asm ("l.msb %0, %1" : : "r"((Word32)Ap4[4] ), "r"((Word32)tmp[6]));
        asm ("l.msb %0, %1" : : "r"((Word32)Ap4[5] ), "r"((Word32)tmp[5]));
        asm ("l.msb %0, %1" : : "r"((Word32)Ap4[6] ), "r"((Word32)tmp[4]));
        asm ("l.msb %0, %1" : : "r"((Word32)Ap4[7] ), "r"((Word32)tmp[3]));
        asm ("l.msb %0, %1" : : "r"((Word32)Ap4[8] ), "r"((Word32)tmp[2]));
        asm ("l.msb %0, %1" : : "r"((Word32)Ap4[9] ), "r"((Word32)tmp[1]));
        asm ("l.msb %0, %1" : : "r"((Word32)Ap4[10]), "r"((Word32)tmp[0]));

        asm ("l.mac %0, %1" : : "r"((Word32)1      ), "r"((Word32)0x800 ));

        asm volatile("l.macrc %0, 12" : "=r" (tmp[M]));

#else

        s =  Ap4[0] *   h[0];
        s -= Ap4[1] * tmp[9];
        s -= Ap4[2] * tmp[8];
        s -= Ap4[3] * tmp[7];
        s -= Ap4[4] * tmp[6];
        s -= Ap4[5] * tmp[5];
        s -= Ap4[6] * tmp[4];
        s -= Ap4[7] * tmp[3];
        s -= Ap4[8] * tmp[2];
        s -= Ap4[9] * tmp[1];
        s -= Ap4[10] * tmp[0];
        tmp[M] = (Word16)((s+(Word32)0x00000800L)>>12);
#endif

        L_tmp1 = tmp[M] * tmp[M];
        L_tmp2 = 0L;
        for (i = 1; i < L_H; i++)
        {

#ifdef OPRISCASM
            asm ("l.mac %0, %1" : : "r"((Word32)Ap4[0] ), "r"((Word32)  h[i]  ));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[1] ), "r"((Word32)tmp[i+9]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[2] ), "r"((Word32)tmp[i+8]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[3] ), "r"((Word32)tmp[i+7]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[4] ), "r"((Word32)tmp[i+6]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[5] ), "r"((Word32)tmp[i+5]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[6] ), "r"((Word32)tmp[i+4]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[7] ), "r"((Word32)tmp[i+3]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[8] ), "r"((Word32)tmp[i+2]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[9] ), "r"((Word32)tmp[i+1]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[10]), "r"((Word32)tmp[i+0]));

            asm ("l.mac %0, %1" : : "r"((Word32)1      ), "r"((Word32)0x800 ));

            asm volatile("l.macrc %0, 12" : "=r" (tmp[M+i]));

#else
            s =  Ap4[0] * h[i];
            s -= Ap4[1] * tmp[i+9];
            s -= Ap4[2] * tmp[i+8];
            s -= Ap4[3] * tmp[i+7];
            s -= Ap4[4] * tmp[i+6];
            s -= Ap4[5] * tmp[i+5];
            s -= Ap4[6] * tmp[i+4];
            s -= Ap4[7] * tmp[i+3];
            s -= Ap4[8] * tmp[i+2];
            s -= Ap4[9] * tmp[i+1];
            s -= Ap4[10] * tmp[i];

            tmp[M+i] = (Word16)((s+(Word32)0x00000800L)>>12);

#endif
            L_tmp1 += tmp[M+i] * tmp[M+i];
            L_tmp2 += tmp[M+i-1] * tmp[M+i];
        }

        temp1 = (Word16)(L_tmp1>>15);
        temp2 = (Word16)(L_tmp2>>15);

        if (temp2 <= 0)
        {
            temp2 = 0;
        }
        else
        {
            temp2 = (Word16)((temp2 * MU)>>15);
            temp2 = div_s (temp2, temp1);
        }

        /* filtering of synthesis speech by A(z/0.7) to find res2[] */
        Residu_de (Ap3, &syn_work[i_subfr], st->res2, L_SUBFR);
        preemphasis (st->preemph_state, st->res2, temp2, L_SUBFR);
        Syn_filt_de (Ap4, st->res2, &syn[i_subfr], L_SUBFR, st->mem_syn_pst, 1);
/*
        Copy( st->mem_syn_pst, tmp, M);
        temp1 = st->preemph_state->mem_pre;
        for (i = 0; i < L_SUBFR; i++)
        {
#ifdef OPRISC_
            asm ("l.mac %0, %1" : : "r"((Word32)Ap3[0] ), "r"((Word32)syn_work[i_subfr + i]));
            asm ("l.mac %0, %1" : : "r"((Word32)Ap3[1] ), "r"((Word32)syn_work[i_subfr + i - 1]));
            asm ("l.mac %0, %1" : : "r"((Word32)Ap3[2] ), "r"((Word32)syn_work[i_subfr + i - 2]));
            asm ("l.mac %0, %1" : : "r"((Word32)Ap3[3] ), "r"((Word32)syn_work[i_subfr + i - 3]));
            asm ("l.mac %0, %1" : : "r"((Word32)Ap3[4] ), "r"((Word32)syn_work[i_subfr + i - 4]));
            asm ("l.mac %0, %1" : : "r"((Word32)Ap3[5] ), "r"((Word32)syn_work[i_subfr + i - 5]));
            asm ("l.mac %0, %1" : : "r"((Word32)Ap3[6] ), "r"((Word32)syn_work[i_subfr + i - 6]));
            asm ("l.mac %0, %1" : : "r"((Word32)Ap3[7] ), "r"((Word32)syn_work[i_subfr + i - 7]));
            asm ("l.mac %0, %1" : : "r"((Word32)Ap3[8] ), "r"((Word32)syn_work[i_subfr + i - 8]));
            asm ("l.mac %0, %1" : : "r"((Word32)Ap3[9] ), "r"((Word32)syn_work[i_subfr + i - 9]));
            asm ("l.mac %0, %1" : : "r"((Word32)Ap3[10]), "r"((Word32)syn_work[i_subfr + i - 10]));

            asm ("l.mac %0, %1" : : "r"((Word32)1      ), "r"((Word32)0x800 ));

            asm volatile("l.macrc %0, 12" : "=r" (temp3));

#else
            s = 0;
            s += Ap3[0] * syn_work[i_subfr + i];
            s += Ap3[1] * syn_work[i_subfr + i - 1];
            s += Ap3[2] * syn_work[i_subfr + i - 2];
            s += Ap3[3] * syn_work[i_subfr + i - 3];
            s += Ap3[4] * syn_work[i_subfr + i - 4];
            s += Ap3[5] * syn_work[i_subfr + i - 5];
            s += Ap3[6] * syn_work[i_subfr + i - 6];
            s += Ap3[7] * syn_work[i_subfr + i - 7];
            s += Ap3[8] * syn_work[i_subfr + i - 8];
            s += Ap3[9] * syn_work[i_subfr + i - 9];
            s += Ap3[10] * syn_work[i_subfr + i - 10];

            temp3 = (Word16)( ( s + 0x00000800L) >> 12 );
#endif
            st->res2[i] = temp3 - (Word16)((temp2 * temp1)>>15);
            temp1 = temp3;

#if defined(OPRISC_) && defined(HAVE_MACLC)
            asm ("l.mac %0, %1" : : "r"((Word32)Ap4[0] ), "r"((Word32)st->res2[i]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[1] ), "r"((Word32)tmp[M+i-1]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[2] ), "r"((Word32)tmp[M+i-2]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[3] ), "r"((Word32)tmp[M+i-3]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[4] ), "r"((Word32)tmp[M+i-4]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[5] ), "r"((Word32)tmp[M+i-5]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[6] ), "r"((Word32)tmp[M+i-6]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[7] ), "r"((Word32)tmp[M+i-7]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[8] ), "r"((Word32)tmp[M+i-8]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[9] ), "r"((Word32)tmp[M+i-9]));
            asm ("l.msb %0, %1" : : "r"((Word32)Ap4[10]), "r"((Word32)tmp[M+i-10]));

            asm ("l.mac %0, %1" : : "r"((Word32)1      ), "r"((Word32)0x800 ));

            asm volatile("l.maclc %0, 4" : "=r" (temp4));

#else
            s =  Ap4[0] * st->res2[i];
            s -= Ap4[1] * tmp[M+i-1];
            s -= Ap4[2] * tmp[M+i-2];
            s -= Ap4[3] * tmp[M+i-3];
            s -= Ap4[4] * tmp[M+i-4];
            s -= Ap4[5] * tmp[M+i-5];
            s -= Ap4[6] * tmp[M+i-6];
            s -= Ap4[7] * tmp[M+i-7];
            s -= Ap4[8] * tmp[M+i-8];
            s -= Ap4[9] * tmp[M+i-9];
            s -= Ap4[10] * tmp[M+i-10];
            s += 0x800;
            Word32 mac_hi, mac_lo;
            mac_hi = (Word32)(s >> 32) ;
            mac_lo = (Word32) s;
            _CheckOverflow(mac_hi, mac_lo, temp4);

            if ( temp4 > (Word32)0x0fffffffL )
                temp4 = (Word32)0x7fffffffL;
            else if ( temp4 < (Word32)0xf0000000L )
                temp4 = (Word32)0x80000000L;
            else
                temp4 <<= 3;
#endif

            tmp[M+i] = (Word16)(temp4 >>16);
        }
        st->preemph_state->mem_pre = temp1;
        Copy( &tmp[M], &syn[i_subfr], L_SUBFR);
        Copy( &syn[i_subfr+L_SUBFR-M], st->mem_syn_pst, M);
*/
        /* scale output to input */
        agc (st->agc_state, &syn_work[i_subfr], &syn[i_subfr],
            AGC_FAC, L_SUBFR);

        Az += MP1;
    }

    /* update syn_work[] buffer */

    Copy (&syn_work[L_FRAME - M], &syn_work[-M], M);

    return;
}
