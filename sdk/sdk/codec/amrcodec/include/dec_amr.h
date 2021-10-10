/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
*****************************************************************************
*
*      File             : dec_amr.h
*      Purpose          : Speech decoder routine.
*
*****************************************************************************
*/
#ifndef dec_amr_h
#define dec_amr_h "$Id $"

/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/
#include "typedef.h"
#include "cnst.h"
#include "mode.h"
#include "dtx_dec.h"
#include "d_plsf.h"
#include "gc_pred.h"
#include "ec_gains.h"
#include "ph_disp.h"
#include "c_g_aver.h"
#include "bgnscd.h"
#include "lsp_avg.h"
#include "frame.h"

/*
*****************************************************************************
*                         LOCAL VARIABLES AND TABLES
*****************************************************************************
*/
/*---------------------------------------------------------------*
 *    Postfilter constant parameters (defined in "cnst.h")       *
 *---------------------------------------------------------------*
 *   L_FRAME     : Frame size.                                   *
 *   PIT_MAX     : Maximum pitch lag.                            *
 *   L_INTERPOL  : Length of filter for interpolation.           *
 *   M           : LPC order.                                    *
 *---------------------------------------------------------------*/

/*
*****************************************************************************
*                         DEFINITION OF DATA TYPES
*****************************************************************************
*/
typedef struct{
  /* Excitation vector */
  Word16 old_exc[L_SUBFR + PIT_MAX + L_INTERPOL];
  Word16 *exc;

  /* Lsp (Line spectral pairs) */
   /* Word16 lsp[M]; */      /* Used by CN codec */
  Word16 lsp_old[M];

  /* Filter's memory */
  Word16 mem_syn[M];

  /* pitch sharpening */
  Word16 sharp;
  Word16 old_T0;

  /* Memories for bad frame handling */
  Word16 prev_bf;
  Word16 prev_pdf;
  Word16 state;
  Word16 excEnergyHist[9];

  /* Variable holding received ltpLag, used in background noise and BFI */
  Word16 T0_lagBuff;

  /* Variables for the source characteristic detector (SCD) */
  Word16 inBackgroundNoise;
  Word16 voicedHangover;
  Word16 ltpGainHistory[9];

  Bgn_scdState* background_state;
  Word16 nodataSeed;

  Cb_gain_averageState *Cb_gain_averState;
  lsp_avgState *lsp_avg_st;

   D_plsfState* lsfState;
   ec_gain_pitchState* ec_gain_p_st;
   ec_gain_codeState* ec_gain_c_st;
   gc_predState* pred_state;
   ph_dispState* ph_disp_st;
   dtx_decState* dtxDecoderState;
} Decoder_amrState;

/*
*****************************************************************************
*                         DECLARATION OF PROTOTYPES
*****************************************************************************
*/
/*
**************************************************************************
*
*  Function    : Decoder_amr_init
*  Purpose     : Allocates initializes state memory
*  Description : Stores pointer to filter status struct in *st. This
*                pointer has to be passed to Decoder_amr in each call.
*  Returns     : 0 on success
*
**************************************************************************
*/
void Decoder_amr_init (Decoder_amrState **st);

/*
**************************************************************************
*
*  Function    : Decoder_amr_reset
*  Purpose     : Resets state memory
*  Returns     : 0 on success
*
**************************************************************************
*/
void Decoder_amr_reset (Decoder_amrState *st,enum Mode mode);

/*
**************************************************************************
*
*  Function    : Decoder_amr_exit
*  Purpose     : The memory used for state memory is freed
*  Description : Stores NULL in *s
*  Returns     : void
*
**************************************************************************
*/
void Decoder_amr_exit (Decoder_amrState **st);

/*
**************************************************************************
*
*  Function    : Decoder_amr
*  Purpose     : Speech decoder routine.
*  Returns     : 0
*
**************************************************************************
*/
void Decoder_amr (
    Decoder_amrState *st,  /* i/o : State variables                       */
    enum Mode mode,        /* i   : AMR mode                              */
    Word16 parm[],         /* i   : vector of synthesis parameters
                                    (PRM_SIZE)                            */
    enum RXFrameType frame_type, /* i   : received frame type               */
    Word16 synth[],        /* o   : synthesis speech (L_FRAME)            */
    Word16 A_t[]           /* o   : decoded LP filter in 4 subframes
                                    (AZ_SIZE)                             */
);

#endif
