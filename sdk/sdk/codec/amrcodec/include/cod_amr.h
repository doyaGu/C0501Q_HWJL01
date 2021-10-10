/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
*****************************************************************************
*
*      File             : cod_amr.h
*      Purpose          : Main encoder routine operating on a frame basis.
*
*****************************************************************************
*/
#ifndef cod_amr_h
#define cod_amr_h "$Id $"

/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/
#include "typedef.h"
#include "cnst.h"
#include "mode.h"
#include "lpc.h"
#include "lsp.h"
#include "cl_ltp.h"
#include "gain_q.h"
#include "p_ol_wgh.h"
#include "ton_stab.h"
#include "vad.h"
#include "dtx_enc.h"

/*
*****************************************************************************
*                         DEFINITION OF DATA TYPES
*****************************************************************************
*/
/*-----------------------------------------------------------*
 *    Coder constant parameters (defined in "cnst.h")        *
 *-----------------------------------------------------------*
 *   L_WINDOW    : LPC analysis window size.                 *
 *   L_NEXT      : Samples of next frame needed for autocor. *
 *   L_FRAME     : Frame size.                               *
 *   L_FRAME_BY2 : Half the frame size.                      *
 *   L_SUBFR     : Sub-frame size.                           *
 *   M           : LPC order.                                *
 *   MP1         : LPC order+1                               *
 *   L_TOTAL7k4  : Total size of speech buffer.              *
 *   PIT_MIN7k4  : Minimum pitch lag.                        *
 *   PIT_MAX     : Maximum pitch lag.                        *
 *   L_INTERPOL  : Length of filter for interpolation        *
 *-----------------------------------------------------------*/
typedef struct {
   /* Speech vector */
   Word16 old_speech[L_TOTAL];
   Word16 *speech, *p_window, *p_window_12k2;
   Word16 *new_speech;             /* Global variable */

   /* Weight speech vector */
   Word16 old_wsp[L_FRAME + PIT_MAX];
   Word16 *wsp;

   /* OL LTP states */
   Word16 old_lags[5];
   Word16 ol_gain_flg[2];

   /* Excitation vector */
   Word16 old_exc[L_FRAME + PIT_MAX + L_INTERPOL];
   Word16 *exc;

   /* Zero vector */
   Word16 ai_zero[L_SUBFR + MP1];
   Word16 *zero;

   /* Impulse response vector */
   Word16 *h1;
   Word16 hvec[L_SUBFR * 2];

   /* Substates */
   lpcState   *lpcSt;
   lspState   *lspSt;
   clLtpState *clLtpSt;
   gainQuantState  *gainQuantSt;
   pitchOLWghtState *pitchOLWghtSt;
   tonStabState *tonStabSt;
   vadState *vadSt;
   Flag dtx;
   dtx_encState *dtx_encSt;

   /* Filter's memory */
   Word16 mem_syn[M], mem_w0[M], mem_w[M];
   Word16 mem_err[M + L_SUBFR], *error;

   Word16 sharp;
} cod_amrState;

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
/*
**************************************************************************
*
*  Function    : cod_amr_init
*  Purpose     : Allocates memory and initializes state variables
*  Description : Stores pointer to filter status struct in *st. This
*                pointer has to be passed to cod_amr in each call.
 *                 - initilize pointers to speech buffer
 *                 - initialize static  pointers
 *                 - set static vectors to zero
*  Returns     : 0 on success
*
**************************************************************************
*/
int cod_amr_init (cod_amrState **st, Flag dtx);

/*
**************************************************************************
*
*  Function    : cod_amr_reset
*  Purpose     : Resets state memory
*  Returns     : 0 on success
*
**************************************************************************
*/
int cod_amr_reset (cod_amrState *st);

/*
**************************************************************************
*
*  Function    : cod_amr_exit
*  Purpose     : The memory used for state memory is freed
*  Description : Stores NULL in *st
*
**************************************************************************
*/
//void cod_amr_exit (cod_amrState **st);

/***************************************************************************
 *   FUNCTION:   cod_amr_first
 *
 *   PURPOSE:  Copes with look-ahead.
 *
 *   INPUTS:
 *       No input argument are passed to this function. However, before
 *       calling this function, 40 new speech data should be copied to the
 *       vector new_speech[]. This is a global pointer which is declared in
 *       this file (it points to the end of speech buffer minus 200).
 *
 ***************************************************************************/

int cod_amr_first(cod_amrState *st,     /* i/o : State struct            */
                  Word16 new_speech[]   /* i   : speech input (L_FRAME)  */
);

/***************************************************************************
 *   FUNCTION:   cod_amr
 *
 *   PURPOSE:  Main encoder routine.
 *
 *   DESCRIPTION: This function is called every 20 ms speech frame,
 *       operating on the newly read 160 speech samples. It performs the
 *       principle encoding functions to produce the set of encoded parameters
 *       which include the LSP, adaptive codebook, and fixed codebook
 *       quantization indices (addresses and gains).
 *
 *   INPUTS:
 *       No input argument are passed to this function. However, before
 *       calling this function, 160 new speech data should be copied to the
 *       vector new_speech[]. This is a global pointer which is declared in
 *       this file (it points to the end of speech buffer minus 160).
 *
 *   OUTPUTS:
 *
 *       ana[]:     vector of analysis parameters.
 *       synth[]:   Local synthesis speech (for debugging purposes)
 *
 ***************************************************************************/

int cod_amr(cod_amrState *st,         /* i/o : State struct                 */
            enum Mode mode,           /* i   : AMR mode                     */
            Word16 new_speech[],      /* i   : speech input (L_FRAME)       */
            Word16 ana[],             /* o   : Analysis parameters          */
            enum Mode *usedMode,      /* o   : used mode                    */
            Word16 synth[]            /* o   : Local synthesis              */
);

#endif
