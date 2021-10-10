/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
*****************************************************************************
*
*      File             : sp_dec.h
*      Purpose          : Decoding and post filtering of one speech frame.
*
*****************************************************************************
*/
#ifndef sp_dec_h
#define sp_dec_h "$Id $"

/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/
#include "typedef.h"
#include "cnst.h"
#include "dec_amr.h"
#include "pstfilt.h"
#include "post_pro.h"
#include "mode.h"

/*
*****************************************************************************
*                         DEFINITION OF DATA TYPES
*****************************************************************************
*/
typedef struct{
  Decoder_amrState* decoder_amrState;
  Post_FilterState*  post_state;
  Post_ProcessState* postHP_state;
  enum Mode prev_mode;

  int complexityCounter;   /* Only for complexity computation            */
} Speech_Decode_FrameState;

/*
*****************************************************************************
*                         DECLARATION OF PROTOTYPES
*****************************************************************************
*/

void Speech_Decode_Frame_init (Speech_Decode_FrameState **st,
                              char *id);
/* initialize one instance of the speech decoder
   Stores pointer to filter status struct in *st. This pointer has to
   be passed to Speech_Decode_Frame in each call.
   returns 0 on success
 */

void Speech_Decode_Frame_reset (Speech_Decode_FrameState *st);
/* reset speech decoder (i.e. set state memory to zero)
   returns 0 on success
 */

void Speech_Decode_Frame_exit (Speech_Decode_FrameState **st);
/* de-initialize speech decoder (i.e. free status struct)
   stores NULL in *s
 */

void Speech_Decode_Frame (
    Speech_Decode_FrameState *st, /* io: post filter states                */
    enum Mode mode,               /* i : AMR mode                          */
    //Word16 *serial,               /* i : serial bit stream                 */
    Word16 *parm,                 /* i : Synthesis parameters              */
    enum RXFrameType frame_type,  /* i : Frame type                        */
    Word16 *synth                 /* o : synthesis speech (postfiltered    */
                                  /*     output)                           */
);
/*    return 0 on success
 */

#ifdef MMS_IO

enum RXFrameType UnpackBits (
    Word8  q,              /* i : Q-bit (i.e. BFI)        */
    Word16 ft,             /* i : frame type (i.e. mode)  */
    UWord8 packed_bits[],  /* i : sorted & packed bits    */
    enum Mode *mode,       /* o : mode information        */
    Word16 bits[]          /* o : serial bits             */
);
#endif

#endif
