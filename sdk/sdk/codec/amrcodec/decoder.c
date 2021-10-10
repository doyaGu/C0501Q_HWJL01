
/*************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : decoder.c
*      Purpose          : Speech decoder main program.
*
********************************************************************************
*
*         Usage : decoder  bitstream_file  synth_file
*
*
*         Format for bitstream_file:
*             1 word (2-byte) for the frame type
*               (see frame.h for possible values)
*               Normally, the TX frame type is expected.
*               RX frame type can be forced with "-rxframetype"
*           244 words (2-byte) containing 244 bits.
*               Bit 0 = 0x0000 and Bit 1 = 0x0001
*             1 word (2-byte) for the mode indication
*               (see mode.h for possible values)
*             4 words for future use, currently unused
*
*         Format for synth_file:
*           Synthesis is written to a binary file of 16 bits data.
*
********************************************************************************
*/

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/

#include "defines.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "typedef.h"
#include "n_proc.h"
#include "cnst.h"
#include "mode.h"
#include "frame.h"
#include "strfunc.h"
#include "sp_dec.h"
#include "d_homing.h"
#include "bits2prm.h"

#ifdef MMS_IO
#define AMR_MAGIC_NUMBER "#!AMR\n"
#define MAX_PACKED_SIZE (MAX_SERIAL_SIZE / 8 + 2)
#endif /* MMS_IO */

// micro-seconds per frame @ 1.31 fixed point format.
#define AMR_FRAME_STEP 0x028F5C28 // (160<<31/8000)
static unsigned int amrDecFrameAccu;
static unsigned int decTime;

#if (WMOPS)
#include "count.h"
extern BASIC_OP multiCounter[MAXCOUNTERS];
extern int currCounter;

#endif /* WMOPS */

const char decoder_id[] = "@(#)$Id $";

#ifdef CHECKSUM
Word16 wCheckSum;
#endif /* CHECKSUM */

#ifndef DRIVERIN
#include "amr.h"
#define MAXFILESIZE 2048000
Word16 sim_file_syn[MAXFILESIZE];
#endif /* DRIVERIN */

#ifdef ENABLE_PERFORMANCE_MEASURE
#include "ticktimer.h"
#define MAX_TIMELOG 12000
#endif

#ifdef ENI2S
#include "engine.h"
#include "i2s.h"
#include "mmio.h"
#include "sys.h"

// All in BYTE
#define MAXLEN          (65535)
#define FRAMESIZE       (160 * 2)
#define DOUBLEFRAMESIZE (FRAMESIZE * 2)
#define BUFLEN          (FRAMESIZE * 8)

#define READBUF_SIZE    (2560)

//int i2s_inbuf[BUFLEN];
//int i2s_outbuf[BUFLEN];
Word16 i2s_outbuf[(BUFLEN >> 1)] __attribute__ ((aligned(2)));

Word32 i2s_rdptr;
Word32 i2s_wrptr;

//driver Mode
unsigned char InputBuf[READBUF_SIZE] __attribute__ ((aligned(2)));
Word32 wInputBufferRdIdx;
Word32 wInputBufferWrIdx;
Word32 wEOFReached;

#endif /* ENI2S */

/* frame size in serial bitstream file (frame type + serial stream + flags) */
#define SERIAL_FRAMESIZE (1+MAX_SERIAL_SIZE+5)

/*
********************************************************************************
*                         LOCAL PROGRAM CODE
********************************************************************************
*/
#ifndef MMS_IO
static enum RXFrameType tx_to_rx (enum TXFrameType tx_type)
{
    switch (tx_type) {
      case TX_SPEECH_GOOD:      return RX_SPEECH_GOOD;
      case TX_SPEECH_DEGRADED:  return RX_SPEECH_DEGRADED;
      case TX_SPEECH_BAD:       return RX_SPEECH_BAD;
      case TX_SID_FIRST:        return RX_SID_FIRST;
      case TX_SID_UPDATE:       return RX_SID_UPDATE;
      case TX_SID_BAD:          return RX_SID_BAD;
      case TX_ONSET:            return RX_ONSET;
      case TX_NO_DATA:          return RX_NO_DATA;
      default:
          return RX_SPEECH_BAD;
        //fprintf(stderr, "tx_to_rx: unknown TX frame type %d\n", tx_type);
        //exit(1);
    }
}
#endif /* MMS_IO */

/*
********************************************************************************
*                             MAIN PROGRAM
********************************************************************************
*/

int main (int argc, char *argv[])
{
#ifdef ENABLE_PERFORMANCE_MEASURE
    static unsigned int max_time = 0, min_time = 0xffffffff;
    static unsigned int elapsed = -1;
    static unsigned int time_log[MAX_TIMELOG];
    static unsigned int nFrames = 0;
#endif

#ifdef FILEIN
    FILE *file_serial;
    char *serialFileName = NULL;
#endif /* FILEIN */

#ifndef ENI2S
#   ifdef FILEOUT
    FILE *file_syn;
    char *fileName = NULL;
#   endif
#endif /* ENI2S && FILEOUT */

    Speech_Decode_FrameState *speech_decoder_state = NULL;

    Word16 serial[SERIAL_FRAMESIZE];   /* coded bits                    */
    Word16 parm[MAX_PRM_SIZE + 1];     /* Synthesis parameters          */
    Word16 synth[L_FRAME];             /* Synthesis                     */

    Word32 frame;

#ifndef __OR32__
    char *progname = argv[0];
#endif

    enum Mode mode = (enum Mode)0;
    enum RXFrameType rx_type = (enum RXFrameType)0;
    Word16 reset_flag = 0;
    Word16 reset_flag_old = 1;
    Word32 i = 0;

#ifdef MMS_IO
    UWord8 toc, q, ft = 0;
#   ifdef MAGICWORD
    Word8 magic[8];
#   endif /* MAGICWORD */
    UWord8 packed_bits[MAX_PACKED_SIZE];
    static Word16 packed_size[] = {12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0};
#else
    int rxframetypeMode = 0;           /* use RX frame type codes       */
    enum TXFrameType tx_type = (enum TXFrameType)0;
#endif /* MMSIO */

#ifdef CHECKSUM
    wCheckSum = 0;
#endif

#ifdef ENI2S
    Word32 iWrIdx = 0;
#endif

#ifndef MMS_IO
    /*----------------------------------------------------------------------*
     * process command line options                                         *
     *----------------------------------------------------------------------*/
    while (argc > 1) {
      if (strcmp(argv[1], "-rxframetype") == 0)
          rxframetypeMode = 1;
      else break;

      argc--;
      argv++;
    }
#endif /* MMSIO */

#ifndef __OR32__
    /*----------------------------------------------------------------------*
     * check number of arguments                                            *
     *----------------------------------------------------------------------*/
    if (argc != 3)
    {
        fprintf (stderr,
          " Usage:\n\n"
#   ifndef MMS_IO
          "   %s  [-rxframetype] bitstream_file synth_file\n\n"
          " -rxframetype expects the RX frame type in bitstream_file (instead of TX)\n\n",
#   else
          "   %s  bitstream_file synth_file\n\n",
#   endif
          progname);
        exit (1);
    }
#endif

#ifdef ENABLE_PERFORMANCE_MEASURE
    for(i=0; i<MAX_TIMELOG; i++) {
        time_log[i]=0;
    }
#endif

#ifdef FILEIN
    serialFileName = argv[1];
#endif /* FILEIN */

#if !defined(ENI2S) && defined(FILEOUT)
    fileName = argv[2];
#endif /* !ENI2S && FILEOUT */

#ifdef FILEIN
    /*----------------------------------------------------------------------*
     * Open serial bit stream and output speech file                        *
     *----------------------------------------------------------------------*/
    if (strcmp(serialFileName, "-") == 0) {
        file_serial = stdin;
    }
    else if ((file_serial = fopen (serialFileName, "rb")) == NULL)
    {
        fprintf (stderr, "Input file '%s' does not exist !!\n", serialFileName);
        exit (0);
    }
    fprintf (stderr, "Input bitstream file:   %s\n", serialFileName);
#endif /* FILEIN */

#if !defined(ENI2S) && defined(FILEOUT)

    // for fileouput of MMS_IO
    if (strcmp(fileName, "-") == 0) {
        file_syn = stdout;
    }
    else if ((file_syn = fopen (fileName, "wb")) == NULL)
    {
        fprintf (stderr, "Cannot create output file '%s' !!\n", fileName);
        exit (0);
    }
    fprintf (stderr, "Synthesis speech file:  %s\n", fileName);

#endif /* !ENI2S && FILEOUT */

#if defined(MMS_IO) && defined(MAGICWORD) && defined(FILEIN)
    /* read and verify magic number */
    /* S034
        No Need in Common USE */
    fread(magic, sizeof(Word8), strlen(AMR_MAGIC_NUMBER), file_serial);
    if (strncmp((const char *)magic, AMR_MAGIC_NUMBER, strlen(AMR_MAGIC_NUMBER)))
    {
        fprintf(stderr, "%s%s\n", "Invalid magic number: ", magic);
        fclose(file_serial);
        fclose(file_syn);
        return 1;
    }
#endif /* MMS_IO && MAGICWORD && FILEIN */

#ifdef ENI2S
    /*-----------------------------------------------------------------------*
     * Initialization of I2S                                                 *
     *-----------------------------------------------------------------------*/
    /* 2006.06.28 S034 Merge function call of I2S init */
    initDAC((unsigned char*)i2s_outbuf, 1, 8000, BUFLEN, 0);
    //SetOutWrPtr(0);

    /* End 2006.06.28 S034*/
#if defined(DRIVERIN)

    unsigned char *pInputBufferPtr;

    wInputBufferRdIdx = 0;
    pInputBufferPtr = InputBuf;

    MMIO_Write(DrvDecode_RdPtr, (wInputBufferRdIdx >> 1) << 1);
    MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl)& ~DrvDecode_EOF);

    wInputBufferWrIdx = MMIO_Read(DrvDecode_WrPtr);
    #if defined(__OR32__) && !defined(__FREERTOS__)
    if (0xffff == wInputBufferWrIdx) asm volatile("l.trap 15");
    #endif
#endif /* DRIVERIN */

#endif /* ENI2S */

    /*-----------------------------------------------------------------------*
     * Initialization of decoder                                             *
     *-----------------------------------------------------------------------*/
    Speech_Decode_Frame_init(&speech_decoder_state, "Decoder");

    /*-----------------------------------------------------------------------*
     * process serial bitstream frame by frame                               *
     *-----------------------------------------------------------------------*/

    frame = 0;

    amrDecFrameAccu = 0;
    decTime = 0;

#ifdef FILEIN
    /* With file input */
#   ifndef MMS_IO
    while (fread (serial, sizeof (Word16), SERIAL_FRAMESIZE, file_serial)
        == SERIAL_FRAMESIZE)
        {
        /* get frame type and mode information from frame */
        if (rxframetypeMode) {
            rx_type = (enum RXFrameType)serial[0];
        } else {
            tx_type = (enum TXFrameType)serial[0];
            rx_type = tx_to_rx (tx_type);
        }
        mode = (enum Mode) serial[1+MAX_SERIAL_SIZE];
#   else
    while (fread (&toc, sizeof(UWord8), 1, file_serial) == 1)
    {
        /* read rest of the frame based on ToC byte */
        q  = (toc >> 2) & 0x01;
        ft = (toc >> 3) & 0x0F;
        fread (packed_bits, sizeof(UWord8), packed_size[ft], file_serial);

        rx_type = UnpackBits(q, ft, packed_bits, &mode, &serial[1]);
#   endif /* MMS_IO */

#else /* Without file input*/

#   ifndef MMS_IO
    while (frame < sim_frameno)
    {
        int t;
        for (t = 0; t < 250; t++) {
            serial[t] = sim_file_serial[t+frame*250];
        }
        /* get frame type and mode information from frame */
        if (rxframetypeMode) {
            rx_type = (enum RXFrameType)serial[0];
        } else {
            tx_type = (enum TXFrameType)serial[0];
            rx_type = tx_to_rx (tx_type);
        }
        mode = (enum Mode) serial[1+MAX_SERIAL_SIZE];

#   else /* MMS_IO */

#       ifdef DRIVERIN
    while(1)
    {
        Word32 wDataSize = wInputBufferWrIdx - wInputBufferRdIdx;
        if (wDataSize < 0) {
            wDataSize = wDataSize + READBUF_SIZE;
        }
        if (wEOFReached == 0) {
            if (wDataSize > 0) {
                toc = InputBuf[wInputBufferRdIdx];
                q  = (toc >> 2) & 0x01;
                ft = (toc >> 3) & 0x0F;
                if ( wDataSize < (packed_size[ft] + 1) ) {
                    for(i=0; i<100; i++);
                        wEOFReached = MMIO_Read(DrvAudioCtrl) & DrvDecode_EOF;
                        wInputBufferWrIdx = MMIO_Read(DrvDecode_WrPtr);
                        #if defined(__OR32__) && !defined(__FREERTOS__)
                        if (0xffff == wInputBufferWrIdx) asm volatile("l.trap 15");
                        #endif
                        continue;
                }
            } else {
                wEOFReached = MMIO_Read(DrvAudioCtrl) & DrvDecode_EOF;
                wInputBufferWrIdx = MMIO_Read(DrvDecode_WrPtr);
                #if defined(__OR32__) && !defined(__FREERTOS__)
                if (0xffff == wInputBufferWrIdx) asm volatile("l.trap 15");
                #endif
                continue;
            }
        } else {
            if (wDataSize > 0) {
                toc = InputBuf[wInputBufferRdIdx];
                q  = (toc >> 2) & 0x01;
                ft = (toc >> 3) & 0x0F;
            }
            if (wDataSize == 0 || (wDataSize > 0 && wDataSize < (packed_size[ft] + 1)) ) {
#ifdef DUMP_PCM_DATA
                    MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) | DrvDecodePCM_EOF);
#endif
                    while(GetOutRdPtr() != i2s_wrptr);

                    frame = 0;
                    wInputBufferRdIdx = 0;
                    wInputBufferWrIdx = 0;
                    wEOFReached = 0;
                    MMIO_Write(DrvDecode_RdPtr, 0);
                    MMIO_Write(DrvDecode_WrPtr, 0);
                    MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvDecode_EOF);
                    continue;
                }
            }

        toc = InputBuf[wInputBufferRdIdx];
        q  = (toc >> 2) & 0x01;
        ft = (toc >> 3) & 0x0F;
        if (wInputBufferRdIdx == READBUF_SIZE - 1) {
            wInputBufferRdIdx = 0;
            dc_invalidate();
        } else {
            wInputBufferRdIdx++;
        }

        i = 0;
        while (i < packed_size[ft]) {
            packed_bits[i] = InputBuf[wInputBufferRdIdx];
            if (wInputBufferRdIdx == READBUF_SIZE - 1) {
                wInputBufferRdIdx = 0;
                dc_invalidate();
            } else {
                wInputBufferRdIdx++;
            }
            i++;
        };

        MMIO_Write(DrvDecode_RdPtr, (wInputBufferRdIdx >> 1) << 1);
        //MMIO_Write(DrvDecode_RdPtr, (wInputBufferRdIdx);

#       else /* DRIVERIN */
    int t = 0;
    while (t < sim_mmsio_input_datano)
    {

        toc = sim_mmsio_input[t];
        t++;
        q  = (toc >> 2) & 0x01;
        ft = (toc >> 3) & 0x0F;
        //fread (packed_bits, sizeof(UWord8), packed_size[ft], file_serial);
        i = 0;
        do
        {
            packed_bits[i] = sim_mmsio_input[t+i];
            i++;
        } while (i < packed_size[ft]);
        t = t + packed_size[ft];

#       endif /* DRIVERIN */

#       ifdef ENABLE_PERFORMANCE_MEASURE
        start_timer();
#       endif

        rx_type = UnpackBits(q, ft, packed_bits, &mode, &serial[1]);

/*
  while (fread (&toc, sizeof(UWord8), 1, file_serial) == 1)
  {
      q  = (toc >> 2) & 0x01;
      ft = (toc >> 3) & 0x0F;
      fread (packed_bits, sizeof(UWord8), packed_size[ft], file_serial);

      rx_type = UnpackBits(q, ft, packed_bits, &mode, &serial[1]);
*/
#   endif
#endif

        /* Serial to parameters   */
        if ((rx_type == RX_SID_BAD) ||
          (rx_type == RX_SID_UPDATE)) {
            /* Override mode to MRDTX */
            Bits2prm (MRDTX, &serial[1], parm);
        } else {
            Bits2prm (mode, &serial[1], parm);
        }

        ++frame;

        if (rx_type == RX_NO_DATA) {
            mode = speech_decoder_state->prev_mode;
        } else {
            speech_decoder_state->prev_mode = mode;
        }

        /* if homed: check if this frame is another homing frame */
        if (reset_flag_old == 1)
        {
            /* only check until end of first subframe */
            reset_flag = decoder_homing_frame_test_first(&serial[1], mode);
        }
        /* produce encoder homing frame if homed & input=decoder homing frame */
        if ((reset_flag != 0) && (reset_flag_old != 0))
        {
            for (i = 0; i < L_FRAME; i++)
            {
                synth[i] = EHF_MASK;
            }
        }
        else
        {
            /* decode frame */
            Speech_Decode_Frame(speech_decoder_state, mode, &parm[0],
              rx_type, synth);
        }

        #if defined(ENI2S) && defined(DRIVERIN) && !defined(DUMP_PCM_DATA)
        # if 0
        MMIO_Write(DrvDecode_Frame  , (short)( ((unsigned int)(frame*((20<<16)/1000))) & 0xffff));
        MMIO_Write(DrvDecode_Frame+2, (short)( ((unsigned int)(frame*((20<<16)/1000))) >> 16)   );
        # else
        amrDecFrameAccu += (AMR_FRAME_STEP & 0x7fff);
        decTime = decTime + (AMR_FRAME_STEP >> 15) + (amrDecFrameAccu >> 15);
        amrDecFrameAccu = amrDecFrameAccu & 0x7fff;

        MMIO_Write(DrvDecode_Frame  , (short)( ((unsigned int)decTime) & 0xffff));
        MMIO_Write(DrvDecode_Frame+2, (short)( ((unsigned int)decTime) >> 16)   );
        # endif
        #endif

#ifdef ENABLE_PERFORMANCE_MEASURE
        elapsed = get_timer();
        nFrames++;
        if (nFrames < MAX_TIMELOG)
           time_log[nFrames] = elapsed;
        if (elapsed > max_time) {
            max_time = elapsed;
        }
        if (elapsed < min_time) {
            min_time = elapsed;
        }
#endif

#ifdef ENI2S

        Word32 iRemainSize = 0;
        Word32 iRdInx = 0;

        do
        {
            // iRdInx: Byte index
            // iWrIdx: Word16 index
            iRdInx = GetOutRdPtr();

            //Dump for i2s debug
            i2s_rdptr = iRdInx;

            iRemainSize = iRdInx - (iWrIdx * 2);
            if(iRemainSize <= 0) {
                iRemainSize += BUFLEN;
            }
        } while (iRemainSize < DOUBLEFRAMESIZE);

        //memcpy16((void*)i2s_outbuf[iWrIdx], (void*)synth[0], FRAMESIZE);
        for(i = 0; i < L_FRAME; i++){
            //sim_file_syn[(frame-1)*L_FRAME + i] = synth[i];
            i2s_outbuf[iWrIdx + i] = synth[i];
            // doing debug
#   ifdef CHECKSUM
            wCheckSum += synth[i];
#   endif
        }

        iWrIdx += L_FRAME;
        if ((iWrIdx * 2) == (Word32)(BUFLEN)) {
            iWrIdx = 0;
        }
        i2s_wrptr = iWrIdx * 2;
        SetOutWrPtr(i2s_wrptr);

#else /* ENI2S */
#   ifdef FILEOUT

        // for fileouput of MMS_IO
        /* write synthesized speech to file */
        if (fwrite (synth, sizeof (Word16), L_FRAME, file_syn) != L_FRAME) {
            fprintf(stderr, "\nerror writing output file: %s\n",
              strerror(errno));
        };
        fflush(file_syn);

#       ifdef CHECKSUM
        // doing debug
        for(i = 0; i < L_FRAME; i++){
            wCheckSum += synth[i];
        }
        printf("CheckSum: %X\n", wCheckSum);
#       endif

#   else /* FILEOUT */

        for(i = 0; i < L_FRAME; i++){
            if ((frame-1)*L_FRAME + i < MAXFILESIZE) {
                sim_file_syn[(frame-1)*L_FRAME + i] = synth[i];
            }
            //sim_file_syn[i] = synth[i];
        }
        if ((frame-1)*L_FRAME + i > MAXFILESIZE) {
            break;
        }
        //printf("frame: %d\n",frame);

#   endif
#endif /* ENI2S */

        /* if not homed: check whether current frame is a homing frame */
        if (reset_flag_old == 0)
        {
            /* check whole frame */
            reset_flag = decoder_homing_frame_test(&serial[1], mode);
        }
        /* reset decoder if current frame is a homing frame */
        if (reset_flag != 0)
        {
            Speech_Decode_Frame_reset(speech_decoder_state);
        }
        reset_flag_old = reset_flag;

#ifdef ENI2S
        wInputBufferWrIdx = MMIO_Read(DrvDecode_WrPtr);
        #if defined(__OR32__) && !defined(__FREERTOS__)
        if (0xffff == wInputBufferWrIdx) asm volatile("l.trap 15");
        #endif
#endif

#ifdef TEST5FRAME
        if (frame == 5) {
            break;
        }
#endif

    }

    PRINTF(stderr, "\n%d frame(s) processed\n", frame);

    /*-----------------------------------------------------------------------*
     * Close down speech decoder                                             *
     *-----------------------------------------------------------------------*/
    //Speech_Decode_Frame_exit(&speech_decoder_state);
#if WMOPS
    setCounter(speech_decoder_state->complexityCounter);
    WMOPS_output(0);
    setCounter(0); /* set counter to global counter */
#endif
    //wait = getch();

/*
  if ((file_syn = fopen ("Testout.OUT", "wb")) == NULL)
  {
      fprintf (stderr, "Cannot create output file '%s' !!\n", fileName);
      exit (0);
  }
  if (fwrite (sim_file_syn, sizeof (Word16), L_FRAME*sim_frameno, file_syn) != L_FRAME*5) {
      fprintf(stderr, "\nerror writing output file: %s\n",
      strerror(errno));
  };
  fflush(file_syn);
  fclose(file_syn);
*/

    return 0;
}
