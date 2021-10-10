/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : p_ol_wgh.h
*      Purpose          : Compute the open loop pitch lag with weighting.
*
********************************************************************************
*/
#ifndef p_ol_wgh_h
#define p_ol_wgh_h "$Id $"

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "mode.h"
#include "vad.h"

/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/
/* state variable */
typedef struct {
   Word16 old_T0_med;
   Word16 ada_w;
   Word16 wght_flg;
} pitchOLWghtState;

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
int p_ol_wgh_init (pitchOLWghtState **st);
/* initialize one instance of the pre processing state.
   Stores pointer to filter status struct in *st. This pointer has to
   be passed to p_ol_wgh in each call.
   returns 0 on success
 */

int p_ol_wgh_reset (pitchOLWghtState *st);
/* reset of pre processing state (i.e. set state memory to zero)
   returns 0 on success
 */

//void p_ol_wgh_exit (pitchOLWghtState **st);
/* de-initialize pre processing state (i.e. free status struct)
   stores NULL in *st
 */

Word16 Pitch_ol_wgh (     /* o   : open loop pitch lag                            */
    pitchOLWghtState *st, /* i/o : State struct                                   */
    vadState *vadSt,      /* i/o : VAD state struct                               */
    Word16 signal[],      /* i   : signal used to compute the open loop pitch     */
                          /*       signal[-pit_max] to signal[-1] should be known */
    Word16 pit_min,       /* i   : minimum pitch lag                              */
    Word16 pit_max,       /* i   : maximum pitch lag                              */
    Word16 L_frame,       /* i   : length of frame to compute pitch               */
    Word16 old_lags[],    /* i   : history with old stored Cl lags                */
    Word16 ol_gain_flg[], /* i   : OL gain flag                                   */
    Word16 idx,           /* i   : index                                          */
    Flag dtx              /* i   : dtx flag; use dtx=1, do not use dtx=0          */
    );

#endif
