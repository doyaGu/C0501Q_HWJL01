/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
*****************************************************************************
*
*      File             : sp_enc.c
*      Purpose          : Pre filtering and encoding of one speech frame.
*
*****************************************************************************
*/

/*
*****************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
*****************************************************************************
*/

/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/
//#include <stdlib.h>
//#include <stdio.h>
#include "typedef.h"
#include "basic_op.h"
#include "cnst.h"
//#include "count.h"
//#include "set_zero.h"
#include "pre_proc.h"
#include "prm2bits.h"
#include "mode.h"
#include "cod_amr.h"

#ifdef MMS_IO
#include "frame.h"
#include "bitno.tab"
#endif

#include "sp_enc.h"
const char sp_enc_id[] = "@(#)$Id $" sp_enc_h;

/*
*****************************************************************************
*                         LOCAL VARIABLES AND TABLES
*****************************************************************************
*/
/*---------------------------------------------------------------*
 *    Constants (defined in "cnst.h")                            *
 *---------------------------------------------------------------*
 * L_FRAME     :
 * M           :
 * PRM_SIZE    :
 * AZ_SIZE     :
 * SERIAL_SIZE :
 *---------------------------------------------------------------*/

/*
*****************************************************************************
*                         PUBLIC PROGRAM CODE
*****************************************************************************
*/
Speech_Encode_FrameState sefs1;
/*************************************************************************
*
*  Function:   Speech_Encode_Frame_init
*  Purpose:    Allocates memory for filter structure and initializes
*              state memory
*
**************************************************************************
*/
int Speech_Encode_Frame_init (Speech_Encode_FrameState **state,
                              Flag dtx,
                              char *id)
{
  Speech_Encode_FrameState* s;

  //if (state == (Speech_Encode_FrameState **) NULL){
  //    fprintf(stderr, "Speech_Encode_Frame_init: invalid parameter\n");
  //    return -1;
  //}
  //*state = NULL;

  /* allocate memory */
  //if ((s= (Speech_Encode_FrameState *) malloc(sizeof(Speech_Encode_FrameState))) == NULL){
  //    fprintf(stderr, "Speech_Encode_Frame_init: can not malloc state "
  //                    "structure\n");
  //    return -1;
  //}

  s = &sefs1;
  s->complexityCounter = 0;

  s->pre_state = NULL;
  s->cod_amr_state = NULL;
  s->dtx = dtx;

  Pre_Process_init(&s->pre_state);
  cod_amr_init(&s->cod_amr_state, s->dtx);

  //if (Pre_Process_init(&s->pre_state) ||
  //    cod_amr_init(&s->cod_amr_state, s->dtx)) {
      //Speech_Encode_Frame_exit(&s);
      //return -1;
  //}

  Speech_Encode_Frame_reset(s);
  *state = s;

  return 0;
}

/*************************************************************************
*
*  Function:   Speech_Encode_Frame_reset
*  Purpose:    Resetses state memory
*
**************************************************************************
*/
int Speech_Encode_Frame_reset (Speech_Encode_FrameState *state)
{
  //if (state == (Speech_Encode_FrameState *) NULL){
  //    fprintf(stderr, "Speech_Encode_Frame_reset: invalid parameter\n");
  //    return -1;
  //}

  Pre_Process_reset(state->pre_state);
  cod_amr_reset(state->cod_amr_state);

  //setCounter(state->complexityCounter);
  //Init_WMOPS_counter();
  //setCounter(0); /* set counter to global counter */

  return 0;
}

/*************************************************************************
*
*  Function:   Speech_Encode_Frame_exit
*  Purpose:    The memory used for state memory is freed
*
**************************************************************************
*/
//void Speech_Encode_Frame_exit (Speech_Encode_FrameState **state)
//{
  //if (state == NULL || *state == NULL)
  //    return;

  //Pre_Process_exit(&(*state)->pre_state);
  //cod_amr_exit(&(*state)->cod_amr_state);

  //setCounter((*state)->complexityCounter);
  //WMOPS_output(0);
  //setCounter(0); /* set counter to global counter */

  /* deallocate memory */
  //free(*state);
  //*state = NULL;

  //return;
//}

int Speech_Encode_Frame_First (
    Speech_Encode_FrameState *st,  /* i/o : post filter states       */
    Word16 *new_speech)            /* i   : speech input             */
{
#if !defined(NO13BIT)
   Word16 i;
#endif

   //setCounter(st->complexityCounter);

#if !defined(NO13BIT)
  /* Delete the 3 LSBs (13-bit input) */
  for (i = 0; i < L_NEXT; i++)
  {
     new_speech[i] = new_speech[i] & 0xfff8;
  }
#endif

  /* filter + downscaling */
  Pre_Process (st->pre_state, new_speech, L_NEXT);

  cod_amr_first(st->cod_amr_state, new_speech);

  //Init_WMOPS_counter (); /* reset WMOPS counter for the new frame */

  return 0;
}

int Speech_Encode_Frame (
    Speech_Encode_FrameState *st, /* i/o : post filter states          */
    enum Mode mode,               /* i   : speech coder mode           */
    Word16 *new_speech,           /* i   : speech input                */
    Word16 *serial,               /* o   : serial bit stream           */
    enum Mode *usedMode           /* o   : used speech coder mode */
    )
{
  Word16 prm[MAX_PRM_SIZE];   /* Analysis parameters.                  */
  Word16 syn[L_FRAME];        /* Buffer for synthesis speech           */
  Word32 i;

  //setCounter(st->complexityCounter);
  //Reset_WMOPS_counter (); /* reset WMOPS counter for the new frame */

  /* initialize the serial output frame to zero */
  //for (i = 0; i < MAX_SERIAL_SIZE; i++)
  //{
  //  serial[i] = 0;
  //}

#if !defined(NO13BIT)
  /* Delete the 3 LSBs (13-bit input) */
  for (i = 0; i < L_FRAME; i++)
  {
     new_speech[i] = new_speech[i] & 0xfff8;
  }
#endif

  /* filter + downscaling */
  Pre_Process (st->pre_state, new_speech, L_FRAME);

  /* Call the speech encoder */
  cod_amr(st->cod_amr_state, mode, new_speech, prm, usedMode, syn);

  /* Parameters to serial bits */
  Prm2bits (*usedMode, prm, &serial[0]);

  //fwc();
  //setCounter(0); /* set counter to global counter */

  return 0;
}

#if defined(MMS_IO)
/*************************************************************************
 *
 *  FUNCTION:    PackBits
 *
 *  PURPOSE:     Sorts speech bits according decreasing subjective importance
 *               and packs into octets according to AMR file storage format
 *               as specified in RFC 3267 (Sections 5.1 and 5.3).
 *
 *  DESCRIPTION: Depending on the mode, different numbers of bits are
 *               processed. Details can be found in specification mentioned
 *               above and in file "bitno.tab".
 *
 *               Actually, it RTP payload format, not IF1 or IF2 format.
 *
 *************************************************************************/
Word16 PackBits(
    enum Mode used_mode,       /* i : actual AMR mode             */
    enum Mode mode,            /* i : requested AMR (speech) mode */
    enum TXFrameType fr_type,  /* i : frame type                  */
    Word16 bits[],             /* i : serial bits                 */
    UWord8 packed_bits[]       /* o : sorted&packed bits          */
)
{
   Word32 i;
   UWord8 temp;
   UWord8 *pack_ptr;

   temp = 0;
   pack_ptr = (UWord8*)packed_bits;

   /* file storage format can handle only speech frames, AMR SID frames and NO_DATA frames */
   /* -> force NO_DATA frame */
   if (used_mode < 0 || used_mode > 15 || (used_mode > 8 && used_mode < 15))
   {
       used_mode = (enum Mode)15;
   }

   /* mark empty frames between SID updates as NO_DATA frames */
   if (used_mode == MRDTX && fr_type == TX_NO_DATA)
   {
       used_mode = (enum Mode)15;
   }

   /* insert table of contents (ToC) byte at the beginning of the frame */
   *pack_ptr = toc_byte[used_mode];
   pack_ptr++;

   /* note that NO_DATA frames (used_mode==15) do not need further processing */
   if (used_mode == 15)
   {
       return 1;
   }

   temp = 0;

   /* sort and pack speech bits */
   for (i = 1; i < unpacked_size[used_mode] + 1; i++)
   {
       if (bits[sort_ptr[used_mode][i-1]] == BIT_1)
       {
           temp++;
       }

       if (i % 8)
       {
           temp <<= 1;
       }
       else
       {
           *pack_ptr = temp;
           pack_ptr++;
           temp = 0;
       }
   }

   /* insert SID t{pe indication and speech mode in case of SID frame */
   if (used_mode == MRDTX)
   {
       if (fr_type == TX_SID_UPDATE)
       {
           temp++;
       }
       temp <<= 3;

       temp += mode & 0x0007;
       temp <<= 1;
   }

   /* insert unused bits (zeros) at the tail of the last byte */
   temp <<= (unused_size[used_mode] - 1);
   *pack_ptr = temp;

   return packed_size[used_mode];
}
#endif

//////////////////////////////////////////////////////////////////////////////
// Convert RTP payload to IF2 format
//////////////////////////////////////////////////////////////////////////////
//
// RFC 3267 Section 5.3 Speech Frame
//   7   6   5   4   3   2   1   0   7   6   5   4   3   2   1   0
// +---+---------------+---+---+---+-------------------------------+----
// | P |       FT      | Q | P | P |        d(0) - d(7)            | ...
// +---+---------------+---+---+---+-------------------------------+----
//
// P  = padding
// FT = frame type
// Q  = quality indicator
//
// 3GPP TS 26.101 Annex A IF2 format
//   7   6   5   4   3   2   1   0   7   6   5   4   3   2   1   0
// +---------------+---------------+-------------------------------+----
// | d(3) - d(0)   |       FT      |       d(11) - d(4)            | ...
// +---------------+---------------+-------------------------------+----
//
// Note : The conversion needs to change the endian-ness of the payload
//        bits and shift the data by 4 bits
//
//////////////////////////////////////////////////////////////////////////////
#if 0
#warning "Not yet test!"
inline
static UINT8 FlipNibble(UINT8 ch)
{
    static const UINT8 z_flipTbl[16] = {
        0x0, 0x8, 0x4, 0xc,
        0x2, 0xa, 0x6, 0xe,
        0x1, 0x9, 0x5, 0xd,
        0x3, 0xb, 0x7, 0xf
    };

    return z_flipTbl[ch];
}
static void Convert2IF2(
            int                  frameBytes,
            const unsigned char* pSrc,
            unsigned char*       pDst)
{
    int frameType = (*pSrc++ >> 3) & 0xf;
    UINT8 outCh = (UINT8)frameType;

    if (frameBytes)
    {
        for (int i = 0; i < frameBytes; i++)
        {
            UINT8 inCh = *pSrc++;
            *pDst++ = outCh | (FlipNibble(inCh >> 4) << 4);
            outCh = FlipNibble(inCh & 0xf);
        }
    } else {
        *pDst = outCh;
    }
}
#endif

