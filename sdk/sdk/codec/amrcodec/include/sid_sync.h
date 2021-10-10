/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
*****************************************************************************
*
*      File             : sid_sync.h
*      Purpose          : To ensure that the mode only switches to a
*                         neighbouring mode
*
*****************************************************************************
*/
#ifndef sid_sync_h
#define sid_sync_h "$Id $"

/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/
#include "typedef.h"
#include "mode.h"
#include "frame.h"

/*
******************************************************************************
*                         CONSTANTS
******************************************************************************
*/

/*
******************************************************************************
*                         DEFINITION OF DATA TYPES
******************************************************************************
*/
typedef struct {
    Word16 sid_update_rate;  /* Send SID Update every sid_update_rate frame */
    Word16 sid_update_counter; /* Number of frames since last SID          */
    Word16 sid_handover_debt;  /* Number of extra SID_UPD frames to schedule*/
    enum TXFrameType prev_ft;
} sid_syncState;

/*
*****************************************************************************
*                         LOCAL VARIABLES AND TABLES
*****************************************************************************
*/

/*
*****************************************************************************
*                         DECLARATION OF PROTOTYPES
*****************************************************************************
*/
int sid_sync_init (sid_syncState **st);
/* initialize one instance of the sid_sync module
   Stores pointer to state struct in *st. This pointer has to
   be passed to sid_sync in each call.
   returns 0 on success
 */

int sid_sync_reset (sid_syncState *st);
/* reset of sid_sync module (i.e. set state memory to zero)
   returns 0 on success
 */
//void sid_sync_exit (sid_syncState **st);
/* de-initialize sid_sync module (i.e. free status struct)
   stores NULL in *st
 */

int sid_sync_set_handover_debt (sid_syncState *st, /* i/o: sid_sync state  */
                                Word16 debtFrames);
/*  update handover debt
    debtFrames extra SID_UPD are scheduled .
    to update remote decoder CNI states, right after an handover.
    (primarily for use on MS UL side )
*/

void sid_sync(sid_syncState *st , /* i/o: sid_sync state      */
              enum Mode mode,
              enum TXFrameType *tx_frame_type);
#endif
