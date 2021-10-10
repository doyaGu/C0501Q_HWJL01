/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : levinson.h
*      Purpose          : Levinson-Durbin algorithm in double precision.
*                       : To compute the LP filter parameters from the
*                       : speech autocorrelations.
*
********************************************************************************
*/
#ifndef levinson_h
#define levinson_h "$Id $"

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "cnst.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/

/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/
typedef struct {
  Word16 old_A[M + 1];     /* Last A(z) for case of unstable filter */
} LevinsonState;

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/

int Levinson_init (LevinsonState **st);
/* initialize one instance of the pre processing state.
   Stores pointer to filter status struct in *st. This pointer has to
   be passed to Levinson in each call.
   returns 0 on success
 */

int Levinson_reset (LevinsonState *st);
/* reset of pre processing state (i.e. set state memory to zero)
   returns 0 on success
 */
//void Levinson_exit (LevinsonState **st);
/* de-initialize pre processing state (i.e. free status struct)
   stores NULL in *st
 */

int Levinson (
    LevinsonState *st,
    Word16 Rh[],       /* i : Rh[m+1] Vector of autocorrelations (msb) */
    Word16 Rl[],       /* i : Rl[m+1] Vector of autocorrelations (lsb) */
    Word16 A[],        /* o : A[m]    LPC coefficients  (m = 10)       */
    Word16 rc[]        /* o : rc[4]   First 4 reflection coefficients  */
);

#endif
