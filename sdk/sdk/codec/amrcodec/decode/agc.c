/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
*****************************************************************************
*
*      File             : agc.c
*
*****************************************************************************
*/

/*
*****************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
*****************************************************************************
*/
#include "agc.h"
const char agc_id[] = "@(#)$Id $" agc_h;

/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
#include "basic_op.h"
#include "cnst.h"
#include "inv_sqrt.h"

#if WMOPS
#include "count.h"
#endif

/*
*****************************************************************************
*                         LOCAL VARIABLES AND TABLES
*****************************************************************************
*/
agcState agcS;

/*
*****************************************************************************
*                         LOCAL PROGRAM CODE
*****************************************************************************
*/

static Word32 energy_old( /* o : return energy of signal     */
    Word16 in[],          /* i : input signal (length l_trm) */
    Word16 l_trm          /* i : signal length               */
)
{
    Word32 s;
    Word32 i;
    Word16 in_s;

#if !defined(HAVE_MACLC) || !defined(OPRISCASM)
    Word32 mac_hi, mac_lo;
#endif

/*
    temp = shr (in[0], 2);
    s = L_mult (temp, temp);
    for (i = 1; i < l_trm; i++)
    {
        temp = shr (in[i], 2);
        s = L_mac (s, temp, temp);
    }
*/

#ifdef OPRISCASM
    //asm volatile("l.macrc r0");
    //asm volatile("l.mtspr r0, r0, 0x2801");
    //asm volatile("l.mtspr r0, r0, 0x2802");
    for (i=0; i < l_trm; i++)
    {
        in_s = in[i] >> 2;
        asm ("l.mac %0, %1" : : "r"((Word32)in_s), "r"((Word32)in_s));
    }
#   ifdef HAVE_MACLC
    asm ("l.maclc %0, 1" : "=r" (s));
#   else
    asm volatile("l.mfspr %0, r0, 0x2802" : "=r"(mac_hi));
    asm volatile("l.macrc %0, 0" : "=r" (mac_lo));
#   endif

#else
    Word64 temp = 0;
    for (i=0; i < l_trm; i++)
    {
        in_s = in[i] >> 2;
        temp = temp + (in_s) * (in_s);
    }
    mac_hi = (Word32) (temp >> 32);
    mac_lo = (Word32) (temp & 0xFFFFFFFF);
#endif

#if !defined(OPRISCASM) || (defined(OPRISCASM) && !defined(HAVE_MACLC))
    _CheckOverflow_P(mac_hi, mac_lo, s);
#endif

    return s;

}

static Word32 energy_new( /* o : return energy of signal     */
    Word16 in[],          /* i : input signal (length l_trm) */
    Word16 l_trm          /* i : signal length               */
)
{
#ifndef OPRISCASM
    Word64 temp;
#endif

#if !defined(HAVE_MACLC) || !defined(OPRISCASM)
    Word32 mac_hi, mac_lo;
#endif

    Word32 s;
    Word32 i;
    Flag ov_save;

    ov_save = Overflow;            /* save overflow flag in case energy_old */
                                   /* must be called                        */

/*
    s = L_mult(in[0], in[0]);
    for (i = 1; i < l_trm; i++)
    {
        s = L_mac(s, in[i], in[i]);
    }
*/

#ifdef OPRISCASM
    //asm volatile("l.macrc r0");
    //asm volatile("l.mtspr r0, r0, 0x2801");
    //asm volatile("l.mtspr r0, r0, 0x2802");
    for (i=0; i < l_trm; i++)
    {
        asm ("l.mac %0, %1" : : "r"((Word32)in[i]), "r"((Word32)in[i]));
    }
#   ifdef HAVE_MACLC
    asm ("l.maclc %0, 1" : "=r" (s));
#   else
    asm volatile("l.mfspr %0, r0, 0x2802" : "=r"(mac_hi));
    asm volatile("l.macrc %0, 0" : "=r" (mac_lo));
#   endif
#else
    temp = 0;
    for (i=0; i < l_trm; i++)
    {
        temp = temp + in[i] * in[i];
    }
    mac_hi = (Word32) (temp >> 32);
    mac_lo = (Word32) (temp & 0xFFFFFFFF);
#endif

#if !defined(OPRISCASM) || (defined(OPRISCASM) && !defined(HAVE_MACLC))
    _CheckOverflow_P(mac_hi, mac_lo, s);
#endif

    /* check for overflow */
    if (s == (Word32)0x7fffffffL )
    {
        Overflow = ov_save;               /* restore overflow flag */
        s = energy_old (in, l_trm);       /* function result */
    }
    else
    {
       //s = L_shr(s, 4);
       s = s >> 4;
    }

    return s;
}
/*
*****************************************************************************
*                         PUBLIC PROGRAM CODE
*****************************************************************************
*/
/*
**************************************************************************
*
*  Function    : agc_init
*  Purpose     : Allocates memory for agc state and initializes
*                state memory
*
**************************************************************************
*/
void agc_init (agcState **state)
{
  agcState* s;

//  if (state == (agcState **) NULL){
//      fprintf(stderr, "agc_init: invalid parameter\n");
//      return -1;
//  }
//  *state = NULL;

  /* allocate memory */
//  if ((s= (agcState *) malloc(sizeof(agcState))) == NULL){
//      fprintf(stderr, "agc_init: can not malloc state structure\n");
//      return -1;
//  }

  s = &agcS;
  agc_reset(s);
  *state = s;

  return;
}

/*
**************************************************************************
*
*  Function    : agc_reset
*  Purpose     : Reset of agc (i.e. set state memory to 1.0)
*
**************************************************************************
*/
void agc_reset (agcState *state)
{
//  if (state == (agcState *) NULL){
//      fprintf(stderr, "agc_reset: invalid parameter\n");
//      return -1;
//  }

  state->past_gain = 4096;   /* initial value of past_gain = 1.0  */

  return;
}

/*
**************************************************************************
*
*  Function    : agc_exit
*  Purpose     : The memory used for state memory is freed
*
**************************************************************************
*/
//void agc_exit (agcState **state)
//{
////  if (state == NULL || *state == NULL)
////      return;
//
//  /* deallocate memory */
////  free(*state);
////  *state = NULL;
//
//  return;
//}

/*
**************************************************************************
*
*  Function    : agc
*  Purpose     : Scales the postfilter output on a subframe basis
*
**************************************************************************
*/
void agc (
    agcState *st,      /* i/o : agc state                        */
    Word16 *sig_in,    /* i   : postfilter input signal  (l_trm) */
    Word16 *sig_out,   /* i/o : postfilter output signal (l_trm) */
    Word16 agc_fac,    /* i   : AGC factor                       */
    Word16 l_trm       /* i   : subframe size                    */
)
{
    Word32 i;
    Word16 exp;
    Word16 gain_in, gain_out;
    Word16 g0, gain;
    Word32 s;

    /* calculate gain_out with exponent */
    s = energy_new(sig_out, l_trm);               /* function result */

    if (s == 0)
    {
        st->past_gain = 0;
        return;
    }
    //exp = sub (norm_l (s), 1);
    exp = norm_l (s) - 1;
    // sis3830 shl overflow
    //gain_out = round16(L_shl (s, exp));
    s = L_shl (s, exp);
    gain_out = (Word16)((s+0x00008000L)>>16);

    /* calculate gain_in with exponent */
    s = energy_new(sig_in, l_trm);                /* function result */

    if (s == 0)
    {
        g0 = 0;
    }
    else
    {
        i = norm_l (s);
        // sis3830 round overflow
        //gain_in = round16(L_shl (s, i));
        gain_in = round16(s << i);
        //exp = sub (exp, i);
        exp -= i;

        /*---------------------------------------------------*
         *  g0 = (1-agc_fac) * sqrt(gain_in/gain_out);       *
         *---------------------------------------------------*/

        //s = L_deposit_l (div_s (gain_out, gain_in));
        s = (Word32)(div_s (gain_out, gain_in));
        //s = L_shl (s, 7);       /* s = gain_out / gain_in */
        //s = L_shr (s, exp);     /* add exponent */
        s = (s << 7);      /* s = gain_out / gain_in */
        if ( exp > 0 )
            s >>= exp;     /* add exponent */
        else
            s <<= -exp;

        s = Inv_sqrt (s);               /* function result */
        //i = round16(L_shl (s, 9));
        i = (Word16)((s+0x00000040L)>>7);

        /* g0 = i * (1-agc_fac) */
        //g0 = mult (i, sub (32767, agc_fac));
        g0 = (Word16)((i * (32767 - agc_fac))>>15);
    }

    /* compute gain[n] = agc_fac * gain[n-1]
                        + (1-agc_fac) * sqrt(gain_in/gain_out) */
    /* sig_out[n] = gain[n] * sig_out[n]                        */

    gain = st->past_gain;

    for (i = 0; i < l_trm; i++)
    {
        // sis3830 mult overvlow
        //gain = mult (gain, agc_fac);
        gain = (gain * agc_fac) >> 15;
        //gain = add (gain, g0);
        gain = gain + g0;
        //sig_out[i] = extract_h (L_shl (L_mult (sig_out[i], gain), 3));
        //sig_out[i] = (Word16)( (L_mult (sig_out[i], gain)) >> 13);
        sig_out[i] = (Word16)( (sig_out[i] * gain) >> 12);
    }

    st->past_gain = gain;

    return;
}

/*
**************************************************************************
*
*  Function    : agc2
*  Purpose     : Scales the excitation on a subframe basis
*
**************************************************************************
*/
void agc2 (
 Word16 *sig_in,        /* i   : postfilter input signal  */
 Word16 *sig_out,       /* i/o : postfilter output signal */
 Word16 l_trm           /* i   : subframe size            */
)
{
    Word32 i;
    Word16 exp;
    Word16 gain_in, gain_out, g0;
    Word32 s;

    /* calculate gain_out with exponent */
    s = energy_new(sig_out, l_trm);               /* function result */

    if (s == 0)
    {
        return;
    }
    //exp = sub (norm_l (s), 1);
    exp = norm_l (s) - 1;

    //gain_out = round16(L_shl (s, exp));
    s = L_shl(s, exp);
    gain_out = (Word16)((s+0x00008000L)>>16);

    /* calculate gain_in with exponent */
    s = energy_new(sig_in, l_trm);                /* function result */

    if (s == 0)
    {
        g0 = 0;
    }
    else
    {
        i = norm_l (s);
        // sis3830 round overflow
        gain_in = round16(L_shl (s, i));
        //exp = sub (exp, i);
        exp -= i;

        /*---------------------------------------------------*
         *  g0 = sqrt(gain_in/gain_out);                     *
         *---------------------------------------------------*/

        //s = L_deposit_l (div_s (gain_out, gain_in));
        s = (Word32)(div_s (gain_out, gain_in));
        //s = L_shl (s, 7);       /* s = gain_out / gain_in */
        s = (s << 7);       /* s = gain_out / gain_in */
        //s = L_shr (s, exp);     /* add exponent */
        if ( exp > 0 )
            s >>= exp;     /* add exponent */
        else
            s <<= -exp;

        s = Inv_sqrt (s);               /* function result */
        //g0 = round16(L_shl (s, 9));
        g0 = (Word16)((s+0x00000040L)>>7);
   }

    /* sig_out(n) = gain(n) sig_out(n) */

    for (i = 0; i < l_trm; i++)
    {
        //sig_out[i] = extract_h (L_shl (L_mult (sig_out[i], g0), 3));
        s = sig_out[i] * g0;
//      if ( s > (Word32)0x07ffffffL )
//          sig_out[i] = 0x7fff;
//      else if ( s < (Word32)0xf8000000L )
//          sig_out[i] = (Word16) 0x8000;
//      else
            sig_out[i] = (Word16)(s>>12);
    }

    return;
}
