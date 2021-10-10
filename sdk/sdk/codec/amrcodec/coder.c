/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
*****************************************************************************
*
*      File             : coder.c
*      Purpose          : Speech encoder main program.
*
*****************************************************************************
*
*    Usage : coder speech_file  bitstream_file
*
*    Format for speech_file:
*      Speech is read from a binary file of 16 bits data.
*
*    Format for bitstream_file:
*        1 word (2-byte) for the TX frame type
*          (see frame.h for possible values)
*      244 words (2-byte) containing 244 bits.
*          Bit 0 = 0x0000 and Bit 1 = 0x0001
*        1 word (2-byte) for the mode indication
*          (see mode.h for possible values)
*        4 words for future use, currently written as zero
*
*****************************************************************************
*/

/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/
//#ifdef AMRDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "n_proc.h"
//#endif
#include "typedef.h"
#include "cnst.h"
#include "mode.h"
#include "frame.h"
#include "strfunc.h"
#include "sp_enc.h"
#include "pre_proc.h"
#include "sid_sync.h"
#include "vadname.h"
#include "e_homing.h"
#include "sys.h"
#include "i2s.h"
#include "mmio.h"

#ifdef ENABLE_PERFORMANCE_MEASURE
#include "ticktimer.h"
#define MAX_TIMELOG 12000

unsigned int max_time = 0, min_time = 0xffffffff;
unsigned int elapsed = -1;
unsigned int time_log[MAX_TIMELOG];
#endif

#ifdef MMS_IO
#define AMR_MAGIC_NUMBER "#!AMR\n"
#define MAX_PACKED_SIZE (MAX_SERIAL_SIZE / 8 + 2)       //32
#endif

// mini-seconds per frame @ 1.31 fixed point format.
#define AMR_FRAME_STEP 0x028F5C28 // (160<<31/8000)
static unsigned int amrEncFrameAccu;
static unsigned int encTime;

const char coder_id[] = "@(#)$Id $";

/* frame size in serial bitstream file (frame type + serial stream + flags) */
#define SERIAL_FRAMESIZE (1+MAX_SERIAL_SIZE+5)

/*
*****************************************************************************
*                         LOCAL PROGRAM CODE
*****************************************************************************
*/

/*
*****************************************************************************
*                             MAIN PROGRAM
*****************************************************************************
*/
#define INBUFLEN 10*320
#define FRAMENUMBER 500

#ifdef MMS_IO
#define OUTBUFLEN 300*14
UWord8 serialout[OUTBUFLEN] __attribute__ ((aligned(2)));
#else
#define OUTBUFLEN 50*500
short serialout[OUTBUFLEN / 2] __attribute__ ((aligned(2)));
#endif

short speech[INBUFLEN / 2] __attribute__ ((aligned(2)));

int volatile inwrptr, wrptrend, inrdptr = 0;
Word16 serial[250];             /* Output bitstream buffer           */
Word16 outrdptr = 0;
Word16 outwrptr = 0;
Word16 framecountout = 0;
Word16 framecountin = 0;

int nFrames = 0;

int main(void)
{

    Word16 new_speech[L_FRAME]; /* Pointer to new speech data        */

    #if defined(MMS_IO)
    UWord8 packed_bits[MAX_PACKED_SIZE];
    Word16 packed_size;
    #endif // MMS_IO

    Word16 dtx = 0;             /* enable encoder DTX                */

    /* changed eedodr */
    Word16 reset_flag;

    int i;
    enum Mode mode;
    enum Mode used_mode;
    enum TXFrameType tx_type;

    #if defined(ENABLE_PERFORMANCE_MEASURE)
    for(i=0; i<MAX_TIMELOG; i++) {
        time_log[i]=0;
    }
    max_time = 0;
    min_time = 0xffffffff;
    elapsed = -1;
    #endif // ENABLE_PERFORMANCE_MEASURE

    nFrames = 0;
    Speech_Encode_FrameState *speech_encoder_state = NULL;
    sid_syncState *sid_state = NULL;

    amrEncFrameAccu = 0;
    encTime = 0;

  /*-----------------------------------------------------------------------*
   * Process speech frame by frame                                         *
   *-----------------------------------------------------------------------*/
    MMIO_Write(DrvEncode_WrPtr, 0);
    MMIO_Write(DrvEncode_RdPtr, 0);

    switch((MMIO_Read(DrvAudioCtrl) & DrvAMR_Mode) >> DrvAMR_Mode_Bits) {
        case 0:  mode = MR475; break;
        case 1:  mode = MR515; break;
        case 2:  mode = MR59;  break;
        case 3:  mode = MR67;  break;
        case 4:  mode = MR74;  break;
        case 5:  mode = MR795; break;
        case 6:  mode = MR102; break;
        case 7:  mode = MR122; break;
        default: mode = MR515;
    }

    dtx = (MMIO_Read(DrvAudioCtrl) & DrvAMR_DTX) ? 1 : 0;

    Speech_Encode_Frame_init(&speech_encoder_state, dtx, "encoder");
    sid_sync_init(&sid_state);

    initADC((unsigned char *) speech, 1, 8000, INBUFLEN, 0);

    while (1) {
        inwrptr = GetInWrPtr();

        if ((inwrptr - inrdptr) >= 323 || (inwrptr - inrdptr) < 0) {
            for (i = 0; i < 160; i++) {
                new_speech[i] = speech[i + (framecountin * 160)];
            }

            if (inrdptr == (INBUFLEN - 320)) {
                inrdptr = 0;
                framecountin = 0;
                dc_invalidate();
            } else {
                inrdptr += 320;
                framecountin++;
            }
            SetInRdPtr(inrdptr);

            #if defined(ENABLE_PERFORMANCE_MEASURE)
            start_timer();
            #endif

            /* zero flags and parameter bits */
            for (i = 0; i < 250; i++)
                serial[i] = 0;

            /* check for homing frame */
            //reset_flag = encoder_homing_frame_test(new_speech);
            //reset_flag = encoder_homing_frame_test(new_speech);
            for (i = 0; i < L_FRAME; i++)
            {
               reset_flag = new_speech[i] ^ EHF_MASK;
               if (reset_flag)
                 break;
            }
            reset_flag = (!reset_flag);

            /* encode speech */
            Speech_Encode_Frame(speech_encoder_state, mode, new_speech, &serial[1], &used_mode);

            /* include frame type and mode information in serial bitstream */
            sid_sync(sid_state, used_mode, &tx_type);

            #if !defined(MMS_IO)
            serial[0] = tx_type;
            serial[245] = mode;

            do {
                outrdptr = MMIO_Read(DrvEncode_RdPtr);
                #if defined(__OR32__) && !defined(__FREERTOS__)
                if (0xffff == (unsigned short)outrdptr) asm volatile("l.trap 15");
                #endif
            }
            while (((outrdptr - outwrptr) <= 500 && (outrdptr > outwrptr)) || (OUTBUFLEN + outrdptr - outwrptr) < 502);

            for (i = 0; i < 250; i++)
                serialout[i + (framecountout * 250)] = serial[i];

            if (outwrptr == (OUTBUFLEN - 500)) {
                outwrptr = (OUTBUFLEN - 2);
                framecountout = 0;
            } else if (outwrptr == (OUTBUFLEN - 2)) {
                outwrptr = 500;
                framecountout++;
            } else {
                outwrptr += 500;
                framecountout++;
            }

            MMIO_Write(DrvEncode_WrPtr, outwrptr);
            #else // defined(MMS_IO)
            packed_size = PackBits(used_mode, mode, tx_type, &serial[1], packed_bits);

            #if defined(ENABLE_PERFORMANCE_MEASURE)
            elapsed = get_timer();
            if (nFrames < MAX_TIMELOG)
               time_log[nFrames] = elapsed;
            if (elapsed > max_time) {
                max_time = elapsed;
            }
            if (elapsed < min_time) {
                min_time = elapsed;
            }
            start_timer();
            #endif // ENABLE_PERFORMANCE_MEASURE

            do {
                outrdptr = MMIO_Read(DrvEncode_RdPtr);
                #if defined(__OR32__) && !defined(__FREERTOS__)
                if (0xffff == (unsigned short)outrdptr) asm volatile("l.trap 15");
                #endif
            }
            while (((outrdptr - outwrptr) <= packed_size && (outrdptr > outwrptr))
                   || (OUTBUFLEN + outrdptr - outwrptr) < (packed_size + 2));

            for (i = 0; i < packed_size; i++)
            {
                if((i + outwrptr) < OUTBUFLEN) {
                    serialout[i + outwrptr] = packed_bits[i];
                } else {
                    serialout[(i + outwrptr)-OUTBUFLEN] = packed_bits[i];
                }

            }

            if (outwrptr > (OUTBUFLEN - packed_size)) {
                outwrptr = (outwrptr+packed_size-OUTBUFLEN);
                framecountout = 0;
            } else if (outwrptr == OUTBUFLEN) {
                outwrptr = packed_size;
                framecountout++;
            } else {
                outwrptr += packed_size;
                framecountout++;
            }

            MMIO_Write(DrvEncode_WrPtr, outwrptr);
            #endif // defined(MMS_IO)

            /* perform homing if homing frame was detected at encoder input */
            if (reset_flag != 0) {
                Speech_Encode_Frame_reset(speech_encoder_state);
                sid_sync_reset(sid_state);
            }

            nFrames++;

            #if 0
            MMIO_Write(DrvEncode_Frame  , (short)( ((unsigned int)(nFrames*((20<<16)/1000))) & 0xffff));
            MMIO_Write(DrvEncode_Frame+2, (short)( ((unsigned int)(nFrames*((20<<16)/1000))) >> 16)   );
            #else
            amrEncFrameAccu += (AMR_FRAME_STEP & 0x7fff);
            encTime = encTime + (AMR_FRAME_STEP >> 15) + (amrEncFrameAccu >> 15);
            amrEncFrameAccu = amrEncFrameAccu & 0x7fff;

            MMIO_Write(DrvDecode_Frame  , (short)( ((unsigned int)encTime) & 0xffff));
            MMIO_Write(DrvDecode_Frame+2, (short)( ((unsigned int)encTime) >> 16)   );
            #endif
        }
    }

  /*-----------------------------------------------------------------------*
   * Close down speech coder                                               *
   *-----------------------------------------------------------------------*/

    return 0;
}

