/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*      File             : copy.h
*      Purpose          : Copy vector x[] to y[]
*
********************************************************************************
*/
#ifndef copy_h
#define copy_h "$Id $"

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

/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/

/*
**************************************************************************
*
*  Function    : Copy
*  Purpose     : Copy vector x[] to y[], vector length L
*  Returns     : void
*
**************************************************************************
*/
void Copy (
    const Word16 x[],  /* i : input vector (L)    */
    Word16 y[],        /* o : output vector (L)   */
    Word32 L           /* i : vector length       */
);

/*
**************************************************************************
*
*  Function    : Preset
*  Purpose     : Copy constant x to y[], vector length L
*  Returns     : void
*
**************************************************************************
*/
void Preset (
    Word16 x,  /* i : input vector (L)    */
    Word16 y[],        /* o : output vector (L)   */
    Word32 L           /* i : vector length       */
);

#endif
