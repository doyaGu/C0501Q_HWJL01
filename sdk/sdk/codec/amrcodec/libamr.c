/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/** @file libamr.c
 * libamr source file
 *
 * @author Cory Tong
 * @version 1.0
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "typedef.h"
#include "libamr.h"
// decode
#include "sp_dec.h"
#include "bits2prm.h"
#include "d_homing.h"
//encode
#include "cnst.h"
#include "mode.h"
#include "frame.h"
#include "strfunc.h"
#include "sp_enc.h"
#include "pre_proc.h"
#include "sid_sync.h"
#include "vadname.h"
#include "e_homing.h"
//=============================================================================
//                              Constant Definition
//=============================================================================
#define MAX_PACKED_SIZE     (MAX_SERIAL_SIZE / 8 + 2)
#define SERIAL_FRAMESIZE    (1 + MAX_SERIAL_SIZE + 5)
#define TIME_PER_FRAME      (20)
//=============================================================================
//                              Global Data Definition
//=============================================================================
static Bool gbDecodeIsInit = false;
static Bool gbDecodeIsFinalize = true;
static UWord32 guiDecodeFrame;

//static Bool gbEncodeIsInit = false;
static Bool gbEncodeIsFinalize = true;
static UWord32 guiEncodeFrame;
/* split from decode main function */
static Speech_Decode_FrameState *speech_decoder_state;
static Word16 reset_flag = 0;
static Word16 reset_flag_old = 1;
/* split from encode main function */
static Speech_Encode_FrameState *speech_encoder_state;
static sid_syncState *sid_state;
static enum Mode modeEncode;
static Word16 dtx;
//=============================================================================
//                              Private Function Declaration
//=============================================================================
//=============================================================================
//                              Private Function Definition
//=============================================================================
//=============================================================================
//                              Public Function Definition
//=============================================================================
Bool AMR_initDecode(void)
{
    Speech_Decode_Frame_init(&speech_decoder_state, "Decoder");
    //iFrame = 0;
    guiDecodeFrame = 0;
    gbDecodeIsInit = true;
    gbDecodeIsFinalize = false;
    return true;
}

Bool AMR_decode(Word8 * pIn, Word32 iInSize, Word8 * pOut8, Word32 * iOutSize8)
{
    static Word16 packed_size[] = { 12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0 };

    enum Mode mode = (enum Mode) 0;
    enum RXFrameType rx_type = (enum RXFrameType) 0;

    Word16 serial[SERIAL_FRAMESIZE];    /* coded bits                    */
    Word16 parm[MAX_PRM_SIZE + 1];      /* Synthesis parameters          */
    Word16 synth[L_FRAME];      /* Synthesis                     */

    UWord8 toc, q, ft;
    UWord8 packed_bits[MAX_PACKED_SIZE];

    Word32 iReadSize;
    Word32 iWriteSize;
    Word32 iOutSize;
    Word16 *pOut;

    Word32 i;

    iReadSize = 0;
    iWriteSize = 0;

    pOut = (Word16 *) pOut8;
    iOutSize = *iOutSize8 / 2;

    if (gbDecodeIsFinalize == true) {
        // NOT INIT
        return false;
    }
    // NEED MAGIC WORD DETECT
    if (gbDecodeIsInit == true) {
        if (pIn[iReadSize] == '#') {
            // AMR_MAGIC_NUMBER "#!AMR\n"
            iReadSize = iReadSize + 6;
        }
        gbDecodeIsInit = false;
    }

    while (iReadSize < iInSize) {
        toc = pIn[iReadSize];
        q = (toc >> 2) & 0x01;
        ft = (toc >> 3) & 0x0F;

        // ERROR Detect
        if ((iReadSize + 1 + packed_size[ft]) > iInSize) {
            return false;
        }

        if (iWriteSize + L_FRAME > iOutSize) {
            return false;
        }

        for (i = 0; i < packed_size[ft]; i++) {
            packed_bits[i] = pIn[iReadSize + 1 + i];
        }
        rx_type = UnpackBits(q, ft, packed_bits, &mode, &serial[1]);

        /* Serial to parameters   */
        if ((rx_type == RX_SID_BAD) || (rx_type == RX_SID_UPDATE)) {
            /* Override mode to MRDTX */
            Bits2prm(MRDTX, &serial[1], parm);
        } else {
            Bits2prm(mode, &serial[1], parm);
        }

        if (rx_type == RX_NO_DATA) {
            mode = speech_decoder_state->prev_mode;
        } else {
            speech_decoder_state->prev_mode = mode;
        }

        /* if homed: check if this frame is another homing frame */
        if (reset_flag_old == 1) {
            /* only check until end of first subframe */
            reset_flag = decoder_homing_frame_test_first(&serial[1], mode);
        }
        /* produce encoder homing frame if homed & input=decoder homing frame */
        if ((reset_flag != 0) && (reset_flag_old != 0)) {
            for (i = 0; i < L_FRAME; i++) {
                synth[i] = EHF_MASK;
            }
        } else {
            /* decode frame */
            Speech_Decode_Frame(speech_decoder_state, mode, &parm[0], rx_type, synth);
        }

        if (reset_flag_old == 0) {
            /* check whole frame */
            reset_flag = decoder_homing_frame_test(&serial[1], mode);
        }
        /* reset decoder if current frame is a homing frame */
        if (reset_flag != 0) {
            Speech_Decode_Frame_reset(speech_decoder_state);
        }
        reset_flag_old = reset_flag;

        for (i = 0; i < L_FRAME; i++) {
            //Word8* pOutTemp = &pOut[(iWriteSize + i) * 2]
            pOut[iWriteSize + i] = synth[i];
        }

        iWriteSize += L_FRAME;
        iReadSize += packed_size[ft] + 1;
        guiDecodeFrame++;
    }

    *iOutSize8 = iWriteSize * 2;

    return true;
}

UWord32 AMR_getDecodeFrame(void)
{
    return (guiDecodeFrame);
}

UWord32 AMR_getDecodeTime(void)
{
    return (guiDecodeFrame * TIME_PER_FRAME);
}

Bool AMR_finalizeDecode(void)
{
    guiDecodeFrame = 0;
    gbDecodeIsFinalize = true;
    return true;
}

Bool AMR_initEncode(enum MODE_BITRATE bitrate, enum MODE_MODE cvbr)
{
    speech_encoder_state = NULL;
    sid_state = NULL;

    switch (bitrate) {
    case (MODE_BR475): modeEncode = MR475; break;
    case (MODE_BR515): modeEncode = MR515; break;
    case (MODE_BR59 ): modeEncode = MR59 ; break;
    case (MODE_BR67 ): modeEncode = MR67 ; break;
    case (MODE_BR74 ): modeEncode = MR74 ; break;
    case (MODE_BR795): modeEncode = MR795; break;
    case (MODE_BR102): modeEncode = MR102; break;
    case (MODE_BR122): modeEncode = MR122; break;
    default:           modeEncode = MR515; break;
    }

    switch (cvbr) {
    case (MODE_CBR): dtx = 0; break;
    case (MODE_VBR): dtx = 1; break;
    default:         dtx = 0; break;
    }

    Speech_Encode_Frame_init(&speech_encoder_state, dtx, "encoder");
    sid_sync_init(&sid_state);

    guiEncodeFrame = 0;
    gbEncodeIsFinalize = false;
    return true;
}

Bool AMR_encode(Word8 * pIn8, Word32 iInSize8, Word8 * pOut, Word32 * iOutSize)
{
    Word32 iReadSize;
    Word32 iWriteSize;
    Word32 iInSize;
    Word16 *pIn;

    Word16 new_speech[L_FRAME]; /* Pointer to new speech data        */
    Word16 serial[250];         /* Output bitstream buffer           */
    UWord8 packed_bits[MAX_PACKED_SIZE];
    Word16 packed_size;

    /* changed eedodr */
    Word16 reset_flag;

    Word32 i;
    enum Mode used_mode;
    enum TXFrameType tx_type;

    iReadSize = 0;
    iWriteSize = 0;
    pIn = (Word16 *) pIn8;
    iInSize = (iInSize8) / 2;

    if (gbEncodeIsFinalize) {
        // NOT INIT
        return false;
    }

    while (iReadSize < iInSize) {
        if (iInSize - iReadSize < 160) {
            // PCM not enough
            return false;
        }
        for (i = 0; i < 160; i++) {
            new_speech[i] = pIn[i + iReadSize];
        }
        iReadSize = iReadSize + 160;

        /* zero flags and parameter bits */
        for (i = 0; i < 250; i++)
            serial[i] = 0;

        /* check for homing frame */
        //reset_flag = encoder_homing_frame_test(new_speech);
        //reset_flag = encoder_homing_frame_test(new_speech);
        for (i = 0; i < L_FRAME; i++) {
            reset_flag = new_speech[i] ^ EHF_MASK;
            if (reset_flag)
                break;
        }
        reset_flag = (!reset_flag);

        /* encode speech */
        Speech_Encode_Frame(speech_encoder_state, modeEncode, new_speech, &serial[1], &used_mode);

        /* include frame type and mode information in serial bitstream */
        sid_sync(sid_state, used_mode, &tx_type);

        packed_size = PackBits(used_mode, modeEncode, tx_type, &serial[1], packed_bits);

        if (*iOutSize - iWriteSize < packed_size) {
            // not enough output buffer
            return false;
        }
        for (i = 0; i < packed_size; i++)
            pOut[i + iWriteSize] = packed_bits[i];
        iWriteSize = iWriteSize + packed_size;

        /* perform homing if homing frame was detected at encoder input */
        if (reset_flag != 0) {
            Speech_Encode_Frame_reset(speech_encoder_state);
            sid_sync_reset(sid_state);
        }

        guiEncodeFrame++;
    }

    *iOutSize = iWriteSize;

    return true;
}

UWord32 AMR_getEncodeFrame(void)
{
    return (guiEncodeFrame);
}

UWord32 AMR_getEncodeTime(void)
{
    return (guiEncodeFrame * TIME_PER_FRAME);
}

Bool AMR_finalizeEncode(void)
{
    guiEncodeFrame = 0;
    gbEncodeIsFinalize = true;
    return true;
}

