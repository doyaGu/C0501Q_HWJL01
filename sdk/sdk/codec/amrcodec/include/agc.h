/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
*****************************************************************************
*
*      File             : agc.h
*      Purpose          : Scales the postfilter output on a subframe basis
*                       : by automatic control of the subframe gain.
*
*****************************************************************************
*/
#ifndef agc_h
#define agc_h "$Id $"

/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/
#include "typedef.h"

/*
*****************************************************************************
*                         DEFINITION OF DATA TYPES
*****************************************************************************
*/
typedef struct {
    Word16 past_gain;
} agcState;

/*
*****************************************************************************
*                         DECLARATION OF PROTOTYPES
*****************************************************************************
*/
/*
**************************************************************************
*
*  Function    : agc_init
*  Purpose     : Allocates memory for agc state and initializes
*                state memory
*  Description : Stores pointer to agc status struct in *st. This pointer
*                has to be passed to agc in each call.
*  Returns     : 0 on success
*
**************************************************************************
*/
void agc_init(agcState **st);

/*
**************************************************************************
*
*  Function    : agc_reset
*  Purpose     : Reset of agc (i.e. set state memory to 1.0)
*  Returns     : 0 on success
*
**************************************************************************
*/
void agc_reset (agcState *st);

/*
**************************************************************************
*
*  Function    : agc_exit
*  Purpose     : The memory used for state memory is freed,
*                de-initialize agc
*
**************************************************************************
*/
void agc_exit (agcState **st);

/*
**************************************************************************
*
*  Function    : agc
*  Purpose     : Scales the postfilter output on a subframe basis
*  Description : sig_out[n] = sig_out[n] * gain[n];
*                where gain[n] is the gain at the nth sample given by
*                gain[n] = agc_fac * gain[n-1] + (1 - agc_fac) g_in/g_out
*                g_in/g_out is the square root of the ratio of energy at
*                the input and output of the postfilter.
*
**************************************************************************
*/
void agc (
    agcState *st,      /* i/o : agc state                         */
    Word16 *sig_in,    /* i   : postfilter input signal, (l_trm)  */
    Word16 *sig_out,   /* i/o : postfilter output signal, (l_trm) */
    Word16 agc_fac,    /* i   : AGC factor                        */
    Word16 l_trm       /* i   : subframe size                     */
);

/*
**************************************************************************
*
*  Function:  agc2
*  Purpose:   Scales the excitation on a subframe basis
*
**************************************************************************
*/
void agc2 (
    Word16 *sig_in,    /* i   : postfilter input signal   */
    Word16 *sig_out,   /* i/o : postfilter output signal  */
    Word16 l_trm       /* i   : subframe size             */
);

#endif
