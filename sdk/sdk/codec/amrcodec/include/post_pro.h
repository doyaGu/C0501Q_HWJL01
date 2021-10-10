/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : post_pro.h
*      Purpose          : Postprocessing of output speech.
*
*                         - 2nd order high pass filtering with cut
*                           off frequency at 60 Hz.
*                         - Multiplication of output by two.
*
********************************************************************************
*/
#ifndef post_pro_h
#define post_pro_h "$Id $"

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"

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
   // 2004.9.22 sis3830
   //Word16 y2_hi;
   //Word16 y2_lo;
   //Word16 y1_hi;
   //Word16 y1_lo;
   Word32 L_y2;
   Word32 L_y1;
   Word16 x0;
   Word16 x1;
} Post_ProcessState;

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/

void Post_Process_init (Post_ProcessState **st);
/* initialize one instance of the Post processing state.
   Stores pointer to filter status struct in *st. This pointer has to
   be passed to Post_Process in each call.
   returns 0 on success
 */

void Post_Process_reset (Post_ProcessState *st);
/* reset of Post processing state (i.e. set state memory to zero)
   returns 0 on success
 */
void Post_Process_exit (Post_ProcessState **st);
/* de-initialize Post processing state (i.e. free status struct)
   stores NULL in *st
 */

void Post_Process (
    Post_ProcessState *st,  /* i/o : post process state                   */
    Word16 signal[],        /* i/o : signal                               */
    Word16 lg               /* i   : lenght of signal                     */
    );

#endif
