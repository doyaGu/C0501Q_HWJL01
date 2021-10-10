/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : preemph.h
*      Purpose          : Preemphasis filtering
*      Description      : Filtering through 1 - g z^-1
*
********************************************************************************
*/
#ifndef preemph_h
#define preemph_h "$Id $"

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"

/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/
typedef struct {
  Word16 mem_pre;          /* filter state */
} preemphasisState;

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/

void preemphasis_init (preemphasisState **st);
/* initialize one instance of the preemphasis filter
   Stores pointer to filter status struct in *st. This pointer has to
   be passed to preemphasis in each call.
   returns 0 on success
 */

void preemphasis_reset (preemphasisState *st);
/* reset of preemphasis filter (i.e. set state memory to zero)
   returns 0 on success
 */
void preemphasis_exit (preemphasisState **st);
/* de-initialize preemphasis filter (i.e. free status struct)
   stores NULL in *st
 */

void preemphasis (
    preemphasisState *st, /* (i/o): preemphasis filter state                  */
    Word16 *signal,    /* (i/o): input signal overwritten by the output     */
    Word16 g,          /* (i)  : preemphasis coefficient                    */
    Word16 L           /* (i)  : size of filtering                          */
);

#endif
