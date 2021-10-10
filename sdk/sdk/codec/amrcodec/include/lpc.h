/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : lpc.h
*      Purpose          : 2 LP analyses centered at 2nd and 4th subframe
*                         for mode 12.2. For all other modes a
*                         LP analysis centered at 4th subframe is
*                         performed.
*
********************************************************************************
*/
#ifndef lpc_h
#define lpc_h "$Id $"

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "levinson.h"
#include "mode.h"

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
   LevinsonState *levinsonSt;
} lpcState;

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
int lpc_init (lpcState **st);
/* initialize one instance of the pre processing state.
   Stores pointer to filter status struct in *st. This pointer has to
   be passed to lpc in each call.
   returns 0 on success
 */

int lpc_reset (lpcState *st);
/* reset of pre processing state (i.e. set state memory to zero)
   returns 0 on success
 */
//void lpc_exit (lpcState **st);
/* de-initialize pre processing state (i.e. free status struct)
   stores NULL in *st
 */

int lpc(
    lpcState *st,     /* i/o: State struct                */
    enum Mode mode,   /* i  : coder mode                  */
    Word16 x[],       /* i  : Input signal           Q15  */
    Word16 x_12k2[],  /* i  : Input signal (EFR)     Q15  */
    Word16 a[]        /* o  : predictor coefficients Q12  */
);

#endif
