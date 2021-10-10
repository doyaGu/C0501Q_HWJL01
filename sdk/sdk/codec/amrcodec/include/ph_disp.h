/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : ph_disp.h
*      Purpose          : Phase dispersion of excitation signal
*
********************************************************************************
*/

#ifndef ph_disp_h
#define ph_disp_h "$Id $"

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "mode.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
#define PHDGAINMEMSIZE 5
#define PHDTHR1LTP     9830  /* 0.6 in Q14 */
#define PHDTHR2LTP     14746 /* 0.9 in Q14 */
#define ONFACTPLUS1    16384 /* 2.0 in Q13   */
#define ONLENGTH 2
/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/
typedef struct {
  Word16 gainMem[PHDGAINMEMSIZE];
  Word16 prevState;
  Word16 prevCbGain;
  Word16 lockFull;
  Word16 onset;
} ph_dispState;

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
/*************************************************************************
*
*  Function:   ph_disp_init
*  Purpose:    Allocates state memory and initializes state memory
*
**************************************************************************
*/
void ph_disp_init (ph_dispState **state);

/*************************************************************************
*
*  Function:   ph_disp_reset
*  Purpose:    Initializes state memory
*
**************************************************************************
*/
void ph_disp_reset (ph_dispState *state);

/*************************************************************************
*
*  Function:   ph_disp_exit
*  Purpose:    The memory used for state memory is freed
*
**************************************************************************
*/
void ph_disp_exit (ph_dispState **state);

/*************************************************************************
*
*  Function:   ph_disp_lock
*  Purpose:    mark phase dispersion as locked in state struct
*
**************************************************************************
*/
void ph_disp_lock (ph_dispState *state);

/*************************************************************************
*
*  Function:   ph_disp_release
*  Purpose:    mark phase dispersion as unlocked in state struct
*
**************************************************************************
*/
void ph_disp_release (ph_dispState *state);

/*************************************************************************
*
*  Function:   ph_disp
*  Purpose:    perform phase dispersion according to the specified codec
*              mode and computes total excitation for synthesis part
*              if decoder
*
**************************************************************************
*/
void ph_disp (
      ph_dispState *state, /* i/o     : State struct                     */
      enum Mode mode,      /* i       : codec mode                       */
      Word16 x[],          /* i/o Q0  : in:  LTP excitation signal       */
                           /*           out: total excitation signal     */
      Word16 cbGain,       /* i   Q1  : Codebook gain                    */
      Word16 ltpGain,      /* i   Q14 : LTP gain                         */
      Word16 inno[],       /* i   Q13 : Innovation vector (Q12 for 12.2) */
      Word16 pitch_fac,    /* i   Q14 : pitch factor used to scale the
                                        LTP excitation (Q13 for 12.2)    */
      Word16 tmp_shift     /* i   Q0  : shift factor applied to sum of
                                        scaled LTP ex & innov. before
                                        rounding                         */
);

#endif
