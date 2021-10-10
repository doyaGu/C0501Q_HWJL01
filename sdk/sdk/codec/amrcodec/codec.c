/*******************************************************************************
 *
 *  AMR codec integration Project   @2008.08
 *
 *      AMR + AEC
 *
 *      Through OpenRISC and Built-in Engine optimization, speed up algorithm
 *      executing at MM3XX Multimedia Chip
 *
 *******************************************************************************
 */

#include "defines.h"
#include "mmio.h"

#if defined(__INPUT_CRC_CHECK__)
#  include "crc32.h"
#endif

#if defined(WIN32) || defined(__CYGWIN__)
#  include "win32.h"
#else
//#  include "sys.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "debug.h"

#if defined (__FREERTOS__)
#  include "FreeRTOS.h"
#  include "task.h"
#endif

#if 1 || defined(HAVE_AMR)
#  define AMR_COEDC_DECODE_ONLY
#endif

// mini-seconds per frame @ 1.31 fixed point format.
#define AMR_FRAME_STEP 0x028F5C28       // (160<<31/8000)
#if !defined(AMR_COEDC_DECODE_ONLY)
static unsigned int amrEncFrameAccu;
static unsigned int encTime;
#endif // !defined(AMR_COEDC_DECODE_ONLY)
static unsigned int amrDecFrameAccu;
static unsigned int decTime;

#define MODE_DECODE (0)

#if !defined(AMR_COEDC_DECODE_ONLY)
#  define MODE_ENCODE (1)
#  define MODE_CODEC  (2)
//#define MODE_BPAEC  (3)
#endif

#if !defined(AMR_COEDC_DECODE_ONLY)
#  define AECON  (1)
#  define AECOFF (0)
#endif

/* AMR codec */
#include "typedef.h"
#include "n_proc.h"
#include "cnst.h"
#include "mode.h"
#include "frame.h"
#include "strfunc.h"
#include "sp_dec.h"
#include "sp_enc.h"
#include "d_homing.h"
#include "bits2prm.h"
#include "pre_proc.h"
#include "sid_sync.h"
#include "vadname.h"
#include "e_homing.h"

#ifdef ENABLE_PERFORMANCE_MEASURE
#  include "ticktimer.h"
#  define MAX_TIMELOG 12000

struct {
    unsigned int enc_cnt, enc_max, enc_min, enc_max_n, enc_min_n;
    unsigned int all_cnt, all_max, all_min, all_max_n, all_min_n;
} tm_record = { 0, 0, 0xffffffff, 0, 0, 0, 0, 0xffffffff, 0, 0};

unsigned int elapsed = -1;
unsigned int time_log[MAX_TIMELOG];
unsigned int nFrames = 0;
#endif // ENABLE_PERFORMANCE_MEASURE

#include "i2s.h"

#if !defined(AMR_COEDC_DECODE_ONLY)
/* AEC */
#  include "aec.h"
#endif

#ifdef OPRISCENG
//#  include "engine.h"
#endif

#if defined(ENABLE_CODECS_PLUGIN)
#  include "plugin.h"
#endif

//=============================================================================
// Macros
//=============================================================================
#define SWAPWORD(A)     ( (((A)&0xff)<<8) + (((A)>>8)&0xff) )
//=============================================================================

#ifdef MMS_IO
#  define AMR_MAGIC_NUMBER "#!AMR\n"
#  define MAX_PACKED_SIZE (MAX_SERIAL_SIZE / 8 + 2)     //32
#endif

/* frame size in serial bitstream file (frame type + serial stream + flags) */
#define SERIAL_FRAMESIZE (1+MAX_SERIAL_SIZE+5)

/* I2S */
// Buffer of Output, from decoder to I2S, All in BYTE
#if defined(DUMP_PCM_DATA)
#  define READBUF_SIZE    (4096)                /* Can be changed */
#  define BUFLEN          (FRAMESIZE * 16)      /* Can be changed */
#else
#  define READBUF_SIZE    (1024*16)                /* Can be changed */
#  define BUFLEN          (FRAMESIZE * 8)       /* Can be changed */
#endif

#define DECOPTHD          (50)
#define FRAMESIZE         (160 * 2)       /* 160 PCM 13bit Data = 160 * 2 bytes */
#define DOUBLEFRAMESIZE   (FRAMESIZE * 2) /* 160 PCM 13bit Data = 160 * 2 bytes */

#if defined(__GNUC__)
Word16 i2s_outbuf[(BUFLEN >> 1)] __attribute__ ((aligned(4)));
unsigned char InputBuf[READBUF_SIZE] __attribute__ ((aligned(4)));
#else
Word16 i2s_outbuf[(BUFLEN >> 1)];
unsigned char InputBuf[READBUF_SIZE];
#endif // __GNUC__

// Buffer of Input, from I2S to Encoder
#if defined(DUMP_PCM_DATA)
#  define INBUFLEN  (20 * L_FRAME * 2)
#else
#  define INBUFLEN  (3 * L_FRAME * 2)
#endif // DUMP_PCM_DATA

#if !defined(AMR_COEDC_DECODE_ONLY)
#  define ENCIPTHD  (1  * L_FRAME * 2)
#  define ENCIPTHD2 (ENCIPTHD * 2)
#  define AECPASSTHD (2)

#  if defined(__GNUC__)
short speech[INBUFLEN / 2] __attribute__ ((aligned(4)));        /* I2S to AEC input             */
#  else
short speech[INBUFLEN / 2];
#  endif // __GNUC__

#  ifdef MMS_IO
#    if defined(DUMP_PCM_DATA)
#      define OUTBUFLEN (20 * MAX_PACKED_SIZE)
#    else
#      define OUTBUFLEN 1024
#    endif
#    if defined(__GNUC__)
UWord8 serialout[OUTBUFLEN] __attribute__ ((aligned(4)));
#    else
UWord8 serialout[OUTBUFLEN];
#    endif // __GNUC__
#  else // MMS_IO
#    define OUTBUFLEN 50*500
#    if defined(__GNUC__)
short serialout[OUTBUFLEN / 2] __attribute__ ((aligned(4)));
#    else
short serialout[OUTBUFLEN / 2];
#    endif
#  endif // MMS_IO
#endif // !defined(AMR_COEDC_DECODE_ONLY)

/* DRIVERMODE */
Word32 volatile inwrptr;
Word32 volatile wrptrend;
Word32 volatile inrdptr;

#define AMRINFONUM 12

#if !defined(AMR_COEDC_DECODE_ONLY)
#  define ENC_DO     (0)
#  define ENC_PASS   (1)
#  define ENC_IPFULL (2)
#  define ENC_OPFULL (3)
#endif // AMR_COEDC_DECODE_ONLY

#define DEC_DO     (4)
#define DEC_PASS   (5)
#define DEC_IPFULL (6)
#define DEC_OPFULL (7)

#if !defined(AMR_COEDC_DECODE_ONLY)
#  define AEC_DO     (8)
#  define AEC_PASS   (9)
#  define AEC_SPK    (10)
#  define AEC_MIC    (11)
#endif // AMR_COEDC_DECODE_ONLY

Word32 static amrInfo[AMRINFONUM];

// Buffer of Output, from Encoder to Driver
#if !defined(AMR_COEDC_DECODE_ONLY)
#  if !defined(MEMOUT)
#    define BUFFBYTES (MAXFRAME * 3)
#  else
#    define BUFFBYTES (MAXFRAME * 10)
#  endif
#endif // !AMR_COEDC_DECODE_ONLY

///////////////////////////////////////////////////////////
// TEMP
///////////////////////////////////////////////////////////
static Word16 de_packed_size[] = { 12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0 };

//Decoder: Dump for i2s debug
Word32 static i2s_rdptr;
Word32 static i2s_wrptr;
// 2 IS2 Buffer
Word32 iWrIdx;
Word32 iRdInx;

Word32 bDecIpFull;
Word32 bDecIpRdy;

Word32 bDecOpRdy;
Word32 bDecDone;
Word32 bDec2AECRdy;

#if !defined(AMR_COEDC_DECODE_ONLY)
Word32 bAEC2EncRdy;

Word32 iEnc2I2SWrIdx;
Word32 iEnc2I2SRdIdx;

Word32 bEncDone = 0;

Word32 bEncIpFull = 0;
Word32 bEncIpRdy = 0;
Word32 bEncAECTHD = 0;
Word32 bEncOpRdy = 0;
Word32 iEncFrame = 0;

Word16 aec_echo_in[L_FRAME];              /* Aec ECHO in                  */
//Word16 aec_speech_in[L_FRAME];          /* AEC speech in                */
//Word16 aec_speech_out[L_FRAME];         /* AEC speech out               */
UWord8 en_packed_bits[MAX_PACKED_SIZE];

// Encoder
Word32 framecountin;
Word16 en_reset_flag = 0;
Word16 en_packed_size;
Speech_Encode_FrameState *speech_encoder_state = NULL;
sid_syncState *sid_state = NULL;
enum Mode en_mode;
enum Mode en_used_mode;
enum TXFrameType tx_type;

// AEC
Word32 wSpkPCM;
Word32 wMicPCM;
#endif // AMR_COEDC_DECODE_ONLY

Word32 wDecIpRdIdx;
Word32 wDecIpWrIdx;

Word32 iDecFrame = 0;

Word32 wDecEOFReached;
Word32 bDecEndHandShake;

// Mode
Word32 CodecMode;
Word32 g_bPCMMode;

// Detect Magic Word
Word32 bFirstByte;

// BUFFER
Word16 de_serial[SERIAL_FRAMESIZE];     /* coded bits                   */
Word16 parm[MAX_PRM_SIZE + 1];          /* Synthesis parameters         */
Word16 synth[L_FRAME];                  /* Synthesis                    */

#if !defined(AMR_COEDC_DECODE_ONLY)
Word16 en_serial[SERIAL_FRAMESIZE];     /* coded bits                   */
Word16 new_speech[L_FRAME];             /* Pointer to new speech data   */
#endif // !defined(AMR_COEDC_DECODE_ONLY)

static unsigned short nAmrDecodedByte=0;
static int nKeepByte;
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
static unsigned int* gtAudioPluginBufferLength;
static unsigned short* gtAudioPluginBuffer;
static int gnTemp;
static unsigned char tDumpWave[] = "C:/Amr_dump.wav";
static int gCh;
static int gSampleRate;
int gPause = 0;
int gPrint = 0;
unsigned char *gBuf;
#endif
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
/* Code for general_printf() */
#define BITS_PER_BYTE    8
#define MINUS_SIGN       1
#define RIGHT_JUSTIFY    2
#define ZERO_PAD         4
#define CAPITAL_HEX      8

struct parameters
{
    int   number_of_output_chars;
    short minimum_field_width;
    char  options;
    short edited_string_length;
    short leading_zeros;
};
#endif        
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
void AMR_GetBufInfo(unsigned *inbuf, unsigned *inlen, unsigned *outbuf, unsigned *outlen,unsigned* audioPluginBuf, unsigned* audioPluginBufLen);
void AudioPluginAPI(int nType);
__inline int ithPrintf(char* control, ...);
#else
void AMR_GetBufInfo(unsigned *inbuf, unsigned *inlen, unsigned *outbuf, unsigned *outlen);
#endif

/*
********************************************************************************
*                         LOCAL PROGRAM CODE
********************************************************************************
*/
#ifndef MMS_IO
static enum RXFrameType tx_to_rx(enum TXFrameType tx_type)
{
    switch (tx_type) {
        case TX_SPEECH_GOOD     : return RX_SPEECH_GOOD;
        case TX_SPEECH_DEGRADED : return RX_SPEECH_DEGRADED;
        case TX_SPEECH_BAD      : return RX_SPEECH_BAD;
        case TX_SID_FIRST       : return RX_SID_FIRST;
        case TX_SID_UPDATE      : return RX_SID_UPDATE;
        case TX_SID_BAD         : return RX_SID_BAD;
        case TX_ONSET           : return RX_ONSET;
        case TX_NO_DATA         : return RX_NO_DATA;
        default                 : return RX_SPEECH_BAD;
    }
}
#endif /* MMS_IO */

//=============================================================================
// For debug Only
//=============================================================================
#if defined(WIN32) || defined(__CYGWIN__)
static __inline void sleep(Word32 ms)
{
    sleep(ms);
}
static __inline void delay(Word32 tick)
{
    sleep(tick);
}

#elif !defined(__FREERTOS__)
static __inline void sleep(Word32 ms)
{
    //or32_delay_ms(ms);
}

static __inline void delay(Word32 tick)
{
    //or32_sleep(tick);
}
#endif

//=============================================================================
// Get/Set Encoder I/O Buffer Idx
//=============================================================================
#if !defined(AMR_COEDC_DECODE_ONLY)
static __inline void ResetEncIpIdx()
{
    inwrptr = 0;
    inrdptr = 0;
    SetInRdPtr(inrdptr);
    SetInWrPtr(inwrptr);
}

static __inline Word32 GetEncIpWtIdx()
{
    if (g_bPCMMode == 0 || CodecMode != MODE_DECODE) {
        //PRINTF("GetEncIpWtIdx: %d\n", GetInWrPtr());
        return GetInWrPtr();
    } else {
        return 0;
    }
}

static __inline void SetEncIpRdIdx(Word32 idx)
{
    //PRINTF("SetEncIpRdIdx\n");
    /*  Because MMIO corrupt with decoder input when DUMP_PCM_DATA is set! */
    if (g_bPCMMode == 0 || CodecMode != MODE_DECODE) {
        //PRINTF("SetEncIpRdIdx: %d\n", idx);
        SetInRdPtr(idx);
    }
}

static __inline Word32 GetEncOpRdIdx(void)
{
    Word32 n = MMIO_Read(DrvEncode_RdPtr);
#  if defined(__OR32__) && !defined(__FREERTOS__)
    if (0xffff == n)
        asm volatile ("l.trap 15");
#  endif
    return n;
}

static __inline void SetEncOpRdIdx(Word32 idx)
{
    PRINTF("WARNING! Encoder output read idx being SET!!\n");
    MMIO_Write(DrvEncode_RdPtr, (idx >> 1) << 1);
}

static __inline void SetEncOpWtIdx(Word32 idx)
{
    MMIO_Write(DrvEncode_WrPtr, (idx >> 1) << 1);
}
#endif // !defined(AMR_COEDC_DECODE_ONLY)

//=============================================================================

//=============================================================================
// Get/Set Encoder to PCM Output Buffer Idx
//=============================================================================
#if !defined(AMR_COEDC_DECODE_ONLY)
static __inline void ResetEncOpIdx(void)
{
    if (g_bPCMMode == 1) {
        //PRINTF("ResetEncOpIdx\n");
        iEnc2I2SRdIdx = 0;
        iEnc2I2SWrIdx = 0;
        SetEncOpRdIdx(iEnc2I2SRdIdx);
        SetEncOpWtIdx(iEnc2I2SWrIdx);
    }
}

static __inline int isEncIpEOF(void)
{
    if (CodecMode != MODE_DECODE) {
        if (!g_bPCMMode)
            return isEncEOF();
        else
            return ((MMIO_Read(DrvAudioCtrl) & DrvEncodePCM_EOF) != 0);
    } else {
        return 0;
    }
}

static __inline void WaitEncOpEOF(void)
{
#  if !defined(__FREERTOS__)
    if (g_bPCMMode == 1) {
        while (isEncEOF()) {
            //or32_delay(1);
        }
    }
#  endif // !defined(__FREERTOS__)
}

static __inline void SetEncOpEOF(void)
{
    if (g_bPCMMode == 1) {
        setEncEOF();
    }
}
#endif // !defined(AMR_COEDC_DECODE_ONLY)

//=============================================================================
// Get/Set Decoder to PCM Output Buffer Idx
//=============================================================================
static __inline void ResetPCMOpIdx(void)
{
    if (g_bPCMMode == 1) {
        //PRINTF("ResetPCMOpIdx\n");
        //SetOutWrPtr(0);
        CODEC_I2S_SET_OUTWR_PTR(0);
        SetOutRdPtr(0);
        iRdInx = 0;
        iWrIdx = 0;
    }
}

static __inline void WaitPCMEOF(void)
{
#if defined(WIN32) || defined(__CYGWIN__) || defined(__FREERTOS__)
#else
    if (g_bPCMMode == 1) {
        while (MMIO_Read(DrvAudioCtrl) & DrvDecodePCM_EOF) {
            //or32_delay(1);
        }
    }
#endif
}

static __inline void SetPCMEOF(void)
{
#if defined(WIN32) || defined(__CYGWIN__)
#else
    if (g_bPCMMode == 1) {
        MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) | DrvDecodePCM_EOF);
    }
#endif
}

//=============================================================================
// Clear Data Cache
//=============================================================================
static __inline void ClearDC(void)
{
#if defined(__FREERTOS__) || defined(__OPENRTOS__)
    dc_invalidate();
#endif
}

//=============================================================================
// Get/Set Decoder I/O Buffer Idx
//=============================================================================
/* Input Buffer */
static __inline void SetDecIpRdIdx(Word32 idx)
{
    //PRINTF("Update Decode Input Read Idx!\n");
    ClearDC();
#if !defined(AMR_COEDC_DECODE_ONLY)
    if (g_bPCMMode == 0 || CodecMode != MODE_ENCODE) {
        MMIO_Write(DrvDecode_RdPtr, (idx >> 1) << 1);
    }
#else
    if (g_bPCMMode == 0 || CodecMode == MODE_DECODE) 
    {
        MMIO_Write(DrvDecode_RdPtr, (idx >> 1) << 1);           
    }
#endif // !defined(AMR_COEDC_DECODE_ONLY)
}

static __inline void SetDecIpWtIdx(Word32 idx)
{
    PRINTF("WARNING! Decoder input write idx being SET!!\n");
    MMIO_Write(DrvDecode_WrPtr, (idx) >> 1 << 1);
}

static __inline void ResetDecIpIdx(void)
{
    wDecIpRdIdx = 0;
    wDecIpWrIdx = 0;
    SetDecIpRdIdx(wDecIpRdIdx);
    SetDecIpWtIdx(wDecIpWrIdx);
}

static __inline Word32 GetDecIpWtIdx(void)
{
    //PRINTF("[AMR] Decoder Input Write Idx (%d) Read Idx (%d)\n", MMIO_Read(DrvDecode_WrPtr), MMIO_Read(DrvDecode_RdPtr));
    Word32 n = MMIO_Read(DrvDecode_WrPtr);

    if (0xffff == n)
    {
        setAudioReset();
        while (1);
    }

#if defined(__OR32__) && !defined(__FREERTOS__)
    if (0xffff == n)
        asm volatile ("l.trap 15");
#endif

    return (n);
}

/* Output Buffer */
static __inline Word32 GetDecOpRdIdx(void)
{
    return CODEC_I2S_GET_OUTRD_PTR();
}

static __inline void SetDecOpWrIdx(Word32 ptr)
{
#if !defined(AMR_COEDC_DECODE_ONLY)
    if (g_bPCMMode == 0 || CodecMode != MODE_ENCODE) {
        //SetOutWrPtr(ptr);
        CODEC_I2S_SET_OUTWR_PTR(ptr);
    }
    //PRINTF("SetDecOpWrIdx\n");
#else
    if (g_bPCMMode == 0 || CodecMode == MODE_DECODE) {
        //SetOutWrPtr(ptr);
        CODEC_I2S_SET_OUTWR_PTR(ptr);
    }
#endif
}
//=============================================================================

//=============================================================================
// Get/Set EOF
//=============================================================================
#if defined(__OR32__)
#  define isEncIpPAUSE()      (0)
#  define ClearEncIpEOF()     clrEncEOF()
#  define isDecIpEOF()        isEOF()
#  define isDecIpPAUSE()      isPAUSE()
#  define ClearDecIpPAUSE()   clrDecPAUSE()
#  define isDecIpSTOP()       isSTOP()
#  define ClearDecIpEOF()     (MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvDecode_EOF))
#  define ClearDecIpSTOP()    (MMIO_Write(DrvAudioCtrl, MMIO_Read(DrvAudioCtrl) & ~DrvDecode_STOP))
#endif
//=============================================================================

//=============================================================================
// Get/Set Decoder Frame Number
//=============================================================================
static __inline void SetDecFrame(Word32 num)
{
    amrDecFrameAccu += (AMR_FRAME_STEP & 0x7fff);
    decTime = decTime + (AMR_FRAME_STEP >> 15) + (amrDecFrameAccu >> 15);
    amrDecFrameAccu = amrDecFrameAccu & 0x7fff;

    MMIO_Write(DrvDecode_Frame, (short) (((unsigned int) decTime) & 0xffff));
    MMIO_Write(DrvDecode_Frame + 2, (short) (((unsigned int) decTime) >> 16));
}
//=============================================================================

//=============================================================================
// Get/Set Decoder Frame Number
//=============================================================================

#if !defined(AMR_COEDC_DECODE_ONLY)
static __inline void SetEncFrame(Word32 num)
{
    amrEncFrameAccu += (AMR_FRAME_STEP & 0x7fff);
    encTime = encTime + (AMR_FRAME_STEP >> 15) + (amrEncFrameAccu >> 15);
    amrEncFrameAccu = amrEncFrameAccu & 0x7fff;

    MMIO_Write(DrvEncode_Frame, (short) (((unsigned int) encTime) & 0xffff));
    MMIO_Write(DrvEncode_Frame + 2, (short) (((unsigned int) encTime) >> 16));
}
#endif

//=============================================================================

//=============================================================================
// Non-Preemptive Task Switch
//=============================================================================
static __inline void SwitchTask(void)
{
#if defined(__FREERTOS__)
    PalSleep(2);
#endif
}
//=============================================================================

//=============================================================================
// Buffer Initial
//=============================================================================
UWord8 *pDecIpBuf = InputBuf;
Word16 *pDecOpBuf = i2s_outbuf;
Word32 wDecIpBufLen = READBUF_SIZE;
Word32 wDecOpBufLen = BUFLEN;

static __inline void init_AMRDecBuf(UWord8 * IPBuf, Word32 IPLen, Word16 * OPBuf, Word32 OPLen)
{
    // Decoder Buffer (Unit of Buffer length is byte)
    //  Set Buffer
    pDecIpBuf    = IPBuf;
    wDecIpBufLen = IPLen;
    pDecOpBuf    = OPBuf;
    wDecOpBufLen = OPLen;
    //  Reset Idx, Status
    wDecIpRdIdx = 0;
    wDecIpWrIdx = GetDecIpWtIdx();
    SetDecIpRdIdx(wDecIpRdIdx);
    //  Reset Status
    ClearDecIpEOF();

    PRINTF("Length of decoder input buffer (%d)\n", wDecIpBufLen);
    PRINTF("Length of decoder ouput buffer (%d)\n", wDecOpBufLen);

}

#if !defined(AMR_COEDC_DECODE_ONLY)
Word16 *pEncIpBuf = speech;
UWord8 *pEncOpBuf = serialout;
Word32 wEncIpBufLen = INBUFLEN;
Word32 wEncOpBufLen = OUTBUFLEN;

static __inline void init_AMREncBuf(Word16 * IPBuf, Word32 IPLen, UWord8 * OPBuf, Word32 OPLen)
{
    pEncIpBuf = IPBuf;
    wEncIpBufLen = IPLen;
    pEncOpBuf = OPBuf;
    wEncOpBufLen = OPLen;

    PRINTF("Length of encoder input buffer (%d)\n", wEncIpBufLen);
    PRINTF("Length of encoder ouput buffer (%d)\n", wEncOpBufLen);
}
#endif // !defined(AMR_COEDC_DECODE_ONLY)

//=============================================================================

/*
********************************************************************************
*                             MAIN PROGRAM
********************************************************************************
*/

/* MAIN for Codec */
/* Stage Machine Define */
#define SG_DEC (0)
#if !defined(AMR_COEDC_DECODE_ONLY)
#  define SG_AEC (1)
#  define SG_ENC (2)
#endif

#if !defined(AMR_COEDC_DECODE_ONLY)
static void CK_ENC_INPUT_BUFFER(void)
{
    Word32 wDataSize;
    Word32 bEndHandShake;

    bEndHandShake = 0;
    inwrptr = GetEncIpWtIdx();
    wDataSize = inwrptr - inrdptr;
    if (wDataSize < 0) {
        wDataSize = wDataSize + wEncIpBufLen;
    }
    if (wDataSize >= L_FRAME * 2) {
#  if 0 // 2007.05.03. Remove by Kuoping to insure the output size
        // is the same between Win32 & DUMP_PCM_DATA mode.
        if (isEncIpEOF() && !g_bPCMMode) {
            bEncIpRdy = 0;
            bEncIpFull = 0;
            bEncAECTHD = 0;
            bEndHandShake = 1;
        } else if (isEncIpPAUSE()) {
#  else
        if (isEncIpPAUSE()) {
#  endif
            bEncIpRdy = 0;
            bEncIpFull = 0;
            bEncAECTHD = 0;
            if (inrdptr == (wEncIpBufLen - L_FRAME * 2)) {
                inrdptr = 0;
                framecountin = 0;
                ClearDC();
            } else {
                inrdptr += L_FRAME * 2;
                framecountin++;
            }
            SetEncIpRdIdx(inrdptr);
            //iEncFrame = 0;
            //amrEncFrameAccu = 0;
            //encTime = 0;
            //SetEncFrame(iEncFrame);
        } else {
            bEncIpRdy = 1;
            if (wDataSize >= (L_FRAME * 2 * AECPASSTHD)) {
                bEncAECTHD = 1;
            } else {
                bEncAECTHD = 0;
            }
            if (wDataSize >= (wEncIpBufLen - ENCIPTHD)) {
                bEncIpFull = 1;
                amrInfo[ENC_IPFULL]++;
            } else {
                bEncIpFull = 0;
            }
        }
    } else {
        //if (isEncIpEOF() && wDataSize == 0) {
        if (isEncIpEOF()) {
            bEndHandShake = 1;
        }
        bEncIpRdy = 0;
        bEncIpFull = 0;
        bEncAECTHD = 0;
    }

    if (bEndHandShake) {
        SetEncOpEOF();
        WaitEncOpEOF();
        //wEncIpRdIdx = 0;
        //wEncIpWrIdx = 0;
        ResetEncIpIdx();
        ResetEncOpIdx();
        ClearEncIpEOF();
#  if defined(WIN32) || defined(__CYGWIN__)
        win32_destory();
        exit(0);
#  endif
    }
}
#endif // !defined(AMR_COEDC_DECODE_ONLY)

static void CK_DEC_INPUT_BUFFER(void)
{
    UWord8 toc, ft;
    Word32 wDataSize;

    wDecIpWrIdx = GetDecIpWtIdx();
    wDecEOFReached = isDecIpEOF();
    wDataSize = wDecIpWrIdx - wDecIpRdIdx;
    i2s_rdptr = GetDecOpRdIdx();
    if (wDataSize < 0) 
    {
        wDataSize = wDataSize + wDecIpBufLen;
    }
    //PRINTF("[DEC_IN]DataSize (%d) EOF (%d)\n", wDataSize, wDecEOFReached);
    //PRINTF("[DEC_IN]WrIdx (%d) RdIdx (%d)\n", wDecIpWrIdx, wDecIpRdIdx);
    if (wDataSize > 0)
    {
        if (bFirstByte == 1) {
            if (pDecIpBuf[wDecIpRdIdx] == '#') {
                wDecIpRdIdx = wDecIpRdIdx + 6;
                wDataSize = wDataSize - 6;
            }
            //SetDecOpWrIdx(0);
            if (g_bPCMMode == 0) {
#if !defined(AMR_COEDC_DECODE_ONLY)
                initCODEC((unsigned char *) pEncIpBuf, (unsigned char *) pDecOpBuf, 1, 8000, wEncIpBufLen, wDecOpBufLen, 0);
#else
                initDAC((unsigned char *) pDecOpBuf, 1, 8000, wDecOpBufLen, 0);
#endif // !defined(AMR_COEDC_DECODE_ONLY)
            }

            bFirstByte = 0;
        }
        if (wDataSize == 1 && wDecEOFReached) {
            SetPCMEOF();
            if (GetDecOpRdIdx() == i2s_wrptr) {
                bDecEndHandShake = 1;
            }
            bDecIpRdy = 0;
            bDecIpFull = 0;
        } else {
            toc = pDecIpBuf[wDecIpRdIdx];
            ft = (toc >> 3) & 0x0F;
            if (wDataSize >= (de_packed_size[ft] + 1)) {
                bDecIpRdy = 1;
                if (wDataSize >= (wDecIpBufLen - DECOPTHD)) {
                    bDecIpFull = 1;
                    amrInfo[DEC_IPFULL]++;
                } else {
                    bDecIpFull = 0;
                }
            } else {
                bDecIpRdy = 0;
                bDecIpFull = 0;
                if (wDecEOFReached) {
                    SetPCMEOF();
                    if (GetDecOpRdIdx() == i2s_wrptr) {
                        bDecEndHandShake = 1;
                    }
                }
            }
        }
    } else {
        bDecIpFull = 0;
        bDecIpRdy = 0;
        if (wDecEOFReached) {
            SetPCMEOF();
            if (GetDecOpRdIdx() == i2s_wrptr) {
                bDecEndHandShake = 1;
            }
        }
    }

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    if (isDecIpPAUSE()==1 && gPause==0) 
    {
        bDecIpRdy = 0;
        gPause = 1;
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC);
    }
    else if (isDecIpPAUSE()==0 && gPause) 
    {
        gPause = 0;
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC);
    }
#else
    if (isDecIpPAUSE()==1) 
    {
        PRINTF("AMRDECODE: PAUSE\n");
        bDecIpRdy = 0;
        pauseDAC(1);
    }
    else
    {
        pauseDAC(0);
    }
#endif
    if (isDecIpSTOP()) {
        PRINTF("AMRDECODE: STOP\n");
        bDecIpRdy = 0;
        bDecEndHandShake = 1;
    }

    if (bDecEndHandShake) 
    {
        PRINTF("AMRDECODE: EndHandShake\n");
        if (!isDecIpSTOP()) 
        {
            WaitPCMEOF();
        }
        ResetPCMOpIdx();
        //exit(-1);
        amrDecFrameAccu = 0;
        iDecFrame = 0;
        decTime = 0;
        SetDecFrame(iDecFrame);
        bDecEndHandShake = 0;
        bFirstByte = 1;
        // Reset Decoder Output Write Index
        iWrIdx = 0;
        //SetDecOpWrIdx(0);
        //wDecIpRdIdx = 0;
        //wDecIpWrIdx = 0;
        //wEOFReached = 0;
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
        //AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_DAC);
#else
        deactiveDAC();   // Disable I2S interface
#endif        
        ClearDC();
        ResetDecIpIdx();
        ClearDecIpEOF();
        ClearDecIpSTOP();
#ifdef AMR_RESET_DECODED_BYTE    
        nAmrDecodedByte = 0;
        if (isResetAudioDecodedBytes())
        {
            MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvResetAudioDecodedBytes);
            MMIO_Write(DrvAudioDecodedBytes, nAmrDecodedByte);    
        }                        
#endif  // def AMR_RESET_DECODED_BYTE       

        //ClearDecIpPAUSE();
#if defined(WIN32) || defined(__CYGWIN__)
        win32_destory();
        exit(0);
#endif
    }
}

#if !defined(AMR_COEDC_DECODE_ONLY)
static void CK_ENC_OUTPUT_BUFFER(void)
{
    Word32 iRemainSize;

    iEnc2I2SRdIdx = GetEncOpRdIdx();
    iRemainSize = iEnc2I2SRdIdx - iEnc2I2SWrIdx;

    if (iRemainSize <= 0) {
        iRemainSize += wEncOpBufLen;
    }
    if (iRemainSize > MAX_PACKED_SIZE) {
        bEncOpRdy = 1;
    } else {
        bEncOpRdy = 0;
        amrInfo[ENC_OPFULL]++;
    }
}
#endif // !defined(AMR_COEDC_DECODE_ONLY)

static void CK_DEC_OUTPUT_BUFFER(void)
{
    Word32 iRemainSize;
    iRdInx = GetDecOpRdIdx();
    iRemainSize = iRdInx - (iWrIdx * 2);

    if (iRemainSize <= 0) {
        iRemainSize += wDecOpBufLen;
    }
    //PRINTF("[DEC_OUT]RemainSize (%d)\n", iRemainSize);
    //PRINTF("[DEC_OUT]WrIdx (%d) RdIdx (%d)\n", iWrIdx * 2, iRdInx);
    if (iRemainSize > DOUBLEFRAMESIZE) {
        bDecOpRdy = 1;
    } else {
        bDecOpRdy = 0;
        amrInfo[DEC_OPFULL]++;
    }

    /* TODO: S034
       Strange Patch for Stop, futher modified needed */
    /* fix looping here if isSTOP() */
    if (isDecIpSTOP()) {
        bDecDone = 0;
        bDec2AECRdy = 1;
    }
}

#if !defined(AMR_COEDC_DECODE_ONLY)
static void FLUSH_ENC_OUT(void)
{
    /* Output to File */
    Word32 i;

    for (i = 0; i < en_packed_size; i++) {
        //serialout[iEnc2I2SWrIdx] = en_packed_bits[i];
#  if defined(LITTLE_ENDIAN_PCM) && defined(DUMP_PCM_DATA)
        // Swap the data to little endian.
        if (g_bPCMMode == 1) {
            pEncOpBuf[iEnc2I2SWrIdx] = SWAPWORD(en_packed_bits[i]);
        } else {
            pEncOpBuf[iEnc2I2SWrIdx] = en_packed_bits[i];
        }
#  else
        pEncOpBuf[iEnc2I2SWrIdx] = en_packed_bits[i];
#  endif

        if (iEnc2I2SWrIdx < (wEncOpBufLen - 1)) {
            iEnc2I2SWrIdx++;
        } else {
            iEnc2I2SWrIdx = 0;
        }
    }

    SetEncOpWtIdx(iEnc2I2SWrIdx);
    iEncFrame++;
    SetEncFrame(iEncFrame);

    bAEC2EncRdy = 0;
    bEncDone = 0;
}

#if 0
static void FLUSH_ENC_OUT_DUMMY(void)
{
    /* Output to File */
    /*for (i = 0; i < en_packed_size; i++) {                              */
    /*    serialout[iEnc2I2SWrIdx] = en_packed_bits[i];                   */
    /*    if (iEnc2I2SWrIdx < (wEncOpBufLen - 1)) {                       */
    /*        iEnc2I2SWrIdx++;                                            */
    /*    } else {                                                        */
    /*        iEnc2I2SWrIdx = 0;                                          */
    /*    }                                                               */
    /*}                                                                   */
    /*                                                                    */
    iEncFrame++;
    SetEncFrame(iEncFrame);

    bAEC2EncRdy = 0;
    bEncDone = 0;
}
#endif
#endif // !defined(AMR_COEDC_DECODE_ONLY)

static void FLUSH_DEC_OUT(void)
{
    Word32 i;
    for (i = 0; i < L_FRAME; i++) {
#if defined(LITTLE_ENDIAN_PCM) && defined(DUMP_PCM_DATA)
        // Swap the data to little endian.
        if (g_bPCMMode == 1) {
            pDecOpBuf[iWrIdx + i] = SWAPWORD(synth[i]);
        } else {
            pDecOpBuf[iWrIdx + i] = synth[i];
        }
#else
        pDecOpBuf[iWrIdx + i] = synth[i];
#endif

#if !defined(AMR_COEDC_DECODE_ONLY)
        aec_echo_in[i] = synth[i];
#endif // !defined(AMR_COEDC_DECODE_ONLY)

    }

    iWrIdx += L_FRAME;
    if ((iWrIdx * 2) == wDecOpBufLen) {
        iWrIdx = 0;
    }
    SetDecOpWrIdx(iWrIdx * 2);
    iDecFrame++;
    SetDecFrame(iDecFrame);

    /*Dump for i2s debug */
    i2s_wrptr = iWrIdx * 2;
    bDecDone = 0;
    bDec2AECRdy = 1;
    //PRINTF("[DEC_FLUSH_OUT]WrIdx (%d) RdIdx (%d)\n", iWrIdx * 2, iRdInx);
}

static Speech_Decode_FrameState *speech_decoder_state = NULL;
static Word16 de_reset_flag = 0;
static Word16 de_reset_flag_old = 1;
static void DO_DEC(void)
{
    UWord8 toc, q, ft;
    UWord8 de_packed_bits[MAX_PACKED_SIZE];
    Word32 i;
    int nTemp;
    enum Mode de_mode = (enum Mode) 0;
    enum RXFrameType rx_type = (enum RXFrameType) 0;

    /*************************
     * Read data from driver
     *************************/
    toc = pDecIpBuf[wDecIpRdIdx];
    PRINTF("[dec input] 0x%02X ", toc);
    q = (toc >> 2) & 0x01;
    ft = (toc >> 3) & 0x0F;
    if (wDecIpRdIdx == wDecIpBufLen - 1) {
        wDecIpRdIdx = 0;
        ClearDC();
    } else {
        wDecIpRdIdx++;
    }
    nTemp = wDecIpRdIdx;
    i = 0;
    while (i < de_packed_size[ft]) {
        de_packed_bits[i] = pDecIpBuf[wDecIpRdIdx];
        //PRINTF("%02X ", pDecIpBuf[wDecIpRdIdx]);
        if (wDecIpRdIdx == wDecIpBufLen - 1) {
            wDecIpRdIdx = 0;
            ClearDC();
        } else {
            wDecIpRdIdx++;
        }
        i++;
    }
    //PRINTF("\n");
    SetDecIpRdIdx(wDecIpRdIdx);
#ifdef AMR_RESET_DECODED_BYTE    
        if (isResetAudioDecodedBytes())
        {
            PalSleep(5);            
            MMIO_Write(DrvAudioCtrl2, MMIO_Read(DrvAudioCtrl2) & ~DrvResetAudioDecodedBytes);
            nAmrDecodedByte = 0;
            MMIO_Write(DrvAudioDecodedBytes, nAmrDecodedByte);    
        }                        
        // write Amr decoded byte to register
        nAmrDecodedByte += i;
        MMIO_Write(DrvAudioDecodedBytes, nAmrDecodedByte); 

#endif  // def AMR_RESET_DECODED_BYTE       

    /****************************
     * DECODE
     ****************************/
    rx_type = UnpackBits(q, ft, de_packed_bits, &de_mode, &de_serial[1]);

    if ((rx_type == RX_SID_BAD) || (rx_type == RX_SID_UPDATE)) {
        Bits2prm(MRDTX, &de_serial[1], parm);
    } else {
        Bits2prm(de_mode, &de_serial[1], parm);
    }

    /*******************************
     * OUTPUT
     *******************************/
    if (rx_type == RX_NO_DATA) {
        de_mode = speech_decoder_state->prev_mode;
    } else {
        speech_decoder_state->prev_mode = de_mode;
    }

    /* if homed: check if this frame is another homing frame */
    if (de_reset_flag_old == 1) {
        /* only check until end of first subframe */
        de_reset_flag = decoder_homing_frame_test_first(&de_serial[1], de_mode);
    }
    /* produce encoder homing frame if homed & input=decoder homing frame */
    if ((de_reset_flag != 0) && (de_reset_flag_old != 0)) {
        for (i = 0; i < L_FRAME; i++) {
            synth[i] = EHF_MASK;
        }
    } else {
        /* decode frame */
        Speech_Decode_Frame(speech_decoder_state, de_mode, &parm[0], rx_type, synth);
    }

    /* if not homed: check whether current frame is a homing frame */
    if (de_reset_flag_old == 0) {
        /* check whole frame */
        de_reset_flag = decoder_homing_frame_test(&de_serial[1], de_mode);
    }
    /* reset decoder if current frame is a homing frame */
    if (de_reset_flag != 0) {
        Speech_Decode_Frame_reset(speech_decoder_state);
    }
    de_reset_flag_old = de_reset_flag;

    bDecDone = 1;
    amrInfo[DEC_DO]++;
}

#if 0
static void PASS_DEC(void)
{
    //PRINTF("PASS_DEC()\n");

#if !defined(AMR_COEDC_DECODE_ONLY)
    Word32 i;
    for (i = 0; i < L_FRAME; i++) {
        aec_echo_in[i] = 0;
    }
#endif // !defined(AMR_COEDC_DECODE_ONLY)

    SetDecIpRdIdx(wDecIpRdIdx);
    bDec2AECRdy = 1;
    amrInfo[DEC_PASS]++;
}

static void DROP_DEC(void)
{
    // NOT IMPLEMENT
    //  AEC INPUT
    //PRINTF("DROP_DEC()\n");
    Word32 i;
    UWord8 toc, ft;

#if !defined(AMR_COEDC_DECODE_ONLY)
    for (i = 0; i < L_FRAME; i++) {
        aec_echo_in[i] = 0;
    }
#endif // !defined(AMR_COEDC_DECODE_ONLY)

    /* Skip Frame */
    toc = pDecIpBuf[wDecIpRdIdx];
    ft = (toc >> 3) & 0x0F;
    for (i = 0; i < (de_packed_size[ft] + 1); i++) {
        if (wDecIpRdIdx == wDecIpBufLen - 1) {
            wDecIpRdIdx = 0;
            ClearDC();
        } else {
            wDecIpRdIdx++;
        }
    }
    SetDecIpRdIdx(wDecIpRdIdx);
    iDecFrame++;
    bDec2AECRdy = 1;
    //amrInfo[DEC_DROP]++;
}
#endif

#if !defined(AMR_COEDC_DECODE_ONLY)
static void DO_AEC(void)
{                               /* NOT IMPLEMENT */
    Word32 i;
    //PRINTF("DO_AEC\n");
    for (i = 0; i < L_FRAME; i++) {
        Word32 wTmpPCM;
        wSpkPCM = aec_echo_in[i];
        wMicPCM = pEncIpBuf[i + (framecountin * L_FRAME)];

        if (wSpkPCM < 0) {
            wTmpPCM = -wSpkPCM;
        } else {
            wTmpPCM = wSpkPCM;
        }
        amrInfo[AEC_SPK] = amrInfo[AEC_SPK] - ((amrInfo[AEC_SPK] - wTmpPCM) >> 2);
        if (wMicPCM < 0) {
            wTmpPCM = -wMicPCM;
        } else {
            wTmpPCM = wMicPCM;
        }
        amrInfo[AEC_MIC] = amrInfo[AEC_MIC] - ((amrInfo[AEC_MIC] - wTmpPCM) >> 2);

        wMicPCM = doAEC(wMicPCM, wSpkPCM);
        new_speech[i] = wMicPCM;
    }

    if (inrdptr == (wEncIpBufLen - L_FRAME * 2)) {
        inrdptr = 0;
        framecountin = 0;
        ClearDC();
    } else {
        inrdptr += L_FRAME * 2;
        framecountin++;
    }
    SetEncIpRdIdx(inrdptr);
    bDec2AECRdy = 0;
    bAEC2EncRdy = 1;
    amrInfo[AEC_DO]++;
}

static void PASS_AEC(void)
{
    Word32 i;
    /* NOT IMPLEMENT */
    //PRINTF("PASS_AEC\n");
    for (i = 0; i < L_FRAME; i++) {
        Word32 wTmpPCM;
        wSpkPCM = aec_echo_in[i];
        wMicPCM = pEncIpBuf[i + (framecountin * L_FRAME)];

        if (wSpkPCM < 0) {
            wTmpPCM = -wSpkPCM;
        } else {
            wTmpPCM = wSpkPCM;
        }
        amrInfo[AEC_SPK] = amrInfo[AEC_SPK] - ((amrInfo[AEC_SPK] - wTmpPCM) >> 2);
        if (wMicPCM < 0) {
            wTmpPCM = -wMicPCM;
        } else {
            wTmpPCM = wMicPCM;
        }

        amrInfo[AEC_MIC] = amrInfo[AEC_MIC] - ((amrInfo[AEC_MIC] - wTmpPCM) >> 2);
        new_speech[i] = speech[i + (framecountin * L_FRAME)];
    }

    if (inrdptr == (wEncIpBufLen - L_FRAME * 2)) {
        inrdptr = 0;
        framecountin = 0;
        ClearDC();
    } else {
        inrdptr += L_FRAME * 2;
        framecountin++;
    }
    SetEncIpRdIdx(inrdptr);
    bDec2AECRdy = 0;
    bAEC2EncRdy = 1;
    amrInfo[AEC_PASS]++;
}

static void DO_ENC(void)
{
    /* Encoder */
    Word32 i;

#  if defined(ENABLE_PERFORMANCE_MEASURE)
    unsigned int t1, t2;

    t1 = get_timer();
#  endif // ENABLE_PERFORMANCE_MEASURE

    for (i = 0; i < 250; i++)
        en_serial[i] = 0;

#  if defined(__INPUT_CRC_CHECK__)
    {
        static int run = 0;
        static int crc;
        static char data[sizeof(new_speech)];
        int i;
        unsigned char *ptr = (unsigned char *) new_speech;
        for (i = 0; i < sizeof(new_speech); i += 2) {
#    if defined(__OR32__)
            data[i] = ptr[i + 1];
            data[i + 1] = ptr[i];
#    else
            data[i] = ptr[i];
            data[i + 1] = ptr[i + 1];
#    endif
        }
        crc = crc32(data, sizeof(data));
        PRINTF("#%d: CRC: 0x%08x\n", run++, crc);
    }
#  endif // defined(__INPUT_CRC_CHECK__)

    /* check for homing frame */
    en_reset_flag = encoder_homing_frame_test(new_speech);

    /* encode speech */
    Speech_Encode_Frame(speech_encoder_state, en_mode, new_speech, &en_serial[1], &en_used_mode);

    /* include frame type and mode information in serial bitstream */
    sid_sync(sid_state, en_used_mode, &tx_type);

    en_packed_size = PackBits(en_used_mode, en_mode, tx_type, &en_serial[1], en_packed_bits);

    if (en_reset_flag != 0) {
        Speech_Encode_Frame_reset(speech_encoder_state);
        sid_sync_reset(sid_state);
    }

    bEncDone = 1;
    bAEC2EncRdy = 0;
    amrInfo[ENC_DO]++;

#  if defined(ENABLE_PERFORMANCE_MEASURE)
    if (nFrames >= 1) {         // Skip first frame.
        t2 = ((get_timer() - t1) << 8) / or32_getTicks_ms();
        if (t2 >= (20 << 8))
            tm_record.enc_cnt++;
        if (t2 > tm_record.enc_max) {
            tm_record.enc_max = t2;
            tm_record.enc_max_n = nFrames;
        }
        if (t2 < tm_record.enc_min) {
            tm_record.enc_min = t2;
            tm_record.enc_min_n = nFrames;
        }
    }
#  endif // ENABLE_PERFORMANCE_MEASURE
}

#if 0
static void DROP_ENC(void)
{
    /* NOT IMPLEMENT */
    Word32 i;

    if (en_packed_size != -1) { /* Protect First Packet DROP */
        for (i = 0; i < en_packed_size; i++) {
            /* Using the last Package, Maybe not a good solution */
            pEncOpBuf[iEnc2I2SWrIdx] = en_packed_bits[i];
            if (iEnc2I2SWrIdx < (wEncOpBufLen - 1)) {
                iEnc2I2SWrIdx++;
            } else {
                iEnc2I2SWrIdx = 0;
            }
        }
    }

    SetEncOpWtIdx(iEnc2I2SWrIdx);
    iEncFrame++;
    SetEncFrame(iEncFrame);

    bEncDone = 0;
    bAEC2EncRdy = 0;
}
#endif
#endif // !defined(AMR_COEDC_DECODE_ONLY)

#if 0
int main(void)
{
    int i = 0;
    while (1) {
        wDecIpWrIdx = MMIO_Read(DrvDecode_WrPtr);
        for (; wDecIpRdIdx != wDecIpWrIdx;) {
            if (packed_bits_debug_idx < sizeof(packed_bits_debug))
                packed_bits_debug[packed_bits_debug_idx] = pDecIpBuf[wDecIpRdIdx];
            packed_bits_debug_idx++;
            if (wDecIpRdIdx == wDecIpBufLen - 1) {
                wDecIpRdIdx = 0;
                ClearDC();
            } else
                wDecIpRdIdx++;
        }
        wDecIpRdIdx = wDecIpRdIdx & ~1;
        MMIO_Write(DrvDecode_RdPtr, wDecIpRdIdx);
        for (i = 0; i < 1000; i++);
    }
    return 0;
}
#else

#  if defined(__FREERTOS__)
//=============================================================================
// API for __FREERTOS__ driver
//=============================================================================
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
void AMR_GetBufInfo(unsigned *inbuf, unsigned *inlen, unsigned *outbuf, unsigned *outlen,unsigned* audioPluginBuf, unsigned* audioPluginBufLen)
{
#if 0
    /* Careful! All the variables here must be initialized first!
       Or, when task suspend at the very first time, all the value will be '0'.
     */
    // Decoder Input Buffer (from DRV)
    *inbuf = (unsigned) pDecIpBuf;
    *inlen = wDecIpBufLen;
    gtAudioPluginBuffer = (unsigned short*)audioPluginBuf;
    gtAudioPluginBufferLength = audioPluginBufLen;

#    if !defined(AMR_COEDC_DECODE_ONLY)
    // Encoder Output Buffer (to DRV)
    *outbuf = (unsigned) pEncOpBuf;
    *outlen = wEncOpBufLen;
#    endif

#endif

}
#else
void AMR_GetBufInfo(unsigned *inbuf, unsigned *inlen, unsigned *outbuf, unsigned *outlen)
{
    /* Careful! All the variables here must be initialized first!
       Or, when task suspend at the very first time, all the value will be '0'.
     */
    // Decoder Input Buffer (from DRV)
    *inbuf = (unsigned) pDecIpBuf;
    *inlen = wDecIpBufLen;

#    if !defined(AMR_COEDC_DECODE_ONLY)
    // Encoder Output Buffer (to DRV)
    *outbuf = (unsigned) pEncOpBuf;
    *outlen = wEncOpBufLen;
#    endif
}
#endif
#  endif // __FREERTOS__

/**************************************************************************************
 * Function     : AudioPluginAPI
 *
 * Description  : AudioPluginAPI
 *
 * Input        : plugin type
 *
 * Output       : None
 *
 * Note         :
 **************************************************************************************/
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
static void output_and_count(struct parameters *p, int c)
{
    if (p->number_of_output_chars >= 0)
    {
        int n = c;
        gBuf[gPrint++] = c;
        if (n >= 0)
            p->number_of_output_chars++;
        else
            p->number_of_output_chars = n;
    }
}

static void output_field(struct parameters *p, char *s)
{
    short justification_length =
        p->minimum_field_width - p->leading_zeros - p->edited_string_length;
    if (p->options & MINUS_SIGN)
    {
        if (p->options & ZERO_PAD)
            output_and_count(p, '-');
        justification_length--;
    }
    if (p->options & RIGHT_JUSTIFY)
        while (--justification_length >= 0)
            output_and_count(p, p->options & ZERO_PAD ? '0' : ' ');
    if (p->options & MINUS_SIGN && !(p->options & ZERO_PAD))
        output_and_count(p, '-');
    while (--p->leading_zeros >= 0)
        output_and_count(p, '0');
    while (--p->edited_string_length >= 0){
        output_and_count(p, *s++);
    }
    while (--justification_length >= 0)
        output_and_count(p, ' ');
}


int ithGPrintf(const char *control_string, va_list va)/*const int *argument_pointer)*/
{
    struct parameters p;
    char              control_char;
    p.number_of_output_chars = 0;
    control_char             = *control_string++;
    
    while (control_char != '\0')
    {
        if (control_char == '%')
        {
            short precision     = -1;
            short long_argument = 0;
            short base          = 0;
            control_char          = *control_string++;
            p.minimum_field_width = 0;
            p.leading_zeros       = 0;
            p.options             = RIGHT_JUSTIFY;
            if (control_char == '-')
            {
                p.options    = 0;
                control_char = *control_string++;
            }
            if (control_char == '0')
            {
                p.options   |= ZERO_PAD;
                control_char = *control_string++;
            }
            if (control_char == '*')
            {
                //p.minimum_field_width = *argument_pointer++;
                control_char          = *control_string++;
            }
            else
            {
                while ('0' <= control_char && control_char <= '9')
                {
                    p.minimum_field_width =
                        p.minimum_field_width * 10 + control_char - '0';
                    control_char = *control_string++;
                }
            }
            if (control_char == '.')
            {
                control_char = *control_string++;
                if (control_char == '*')
                {
                    //precision    = *argument_pointer++;
                    control_char = *control_string++;
                }
                else
                {
                    precision = 0;
                    while ('0' <= control_char && control_char <= '9')
                    {
                        precision    = precision * 10 + control_char - '0';
                        control_char = *control_string++;
                    }
                }
            }
            if (control_char == 'l')
            {
                long_argument = 1;
                control_char  = *control_string++;
            }
            if (control_char == 'd')
                base = 10;
            else if (control_char == 'x')
                base = 16;
            else if (control_char == 'X')
            {
                base       = 16;
                p.options |= CAPITAL_HEX;
            }
            else if (control_char == 'u')
                base = 10;
            else if (control_char == 'o')
                base = 8;
            else if (control_char == 'b')
                base = 2;
            else if (control_char == 'c')
            {
                base       = -1;
                p.options &= ~ZERO_PAD;
            }
            else if (control_char == 's')
            {
                base       = -2;
                p.options &= ~ZERO_PAD;
            }
            if (base == 0) /* invalid conversion type */
            {
                if (control_char != '\0')
                {
                    output_and_count(&p, control_char);
                    control_char = *control_string++;
                }
            }
            else
            {
                if (base == -1) /* conversion type c */
                {
                    //char c = *argument_pointer++;
                    char c = (char)(va_arg(va, int));
                    p.edited_string_length = 1;
                    output_field(&p, &c);
                }
                else if (base == -2) /* conversion type s */
                {
                    char *string;
                    p.edited_string_length = 0;
                    //string                 = *(char **) argument_pointer;
                    //argument_pointer      += sizeof(char *) / sizeof(int);
                    string = va_arg(va, char*);
                    while (string[p.edited_string_length] != 0)
                        p.edited_string_length++;
                    if (precision >= 0 && p.edited_string_length > precision)
                        p.edited_string_length = precision;
                    output_field(&p, string);
                }
                else /* conversion type d, b, o or x */
                {
                    unsigned long x;
                    char          buffer[BITS_PER_BYTE * sizeof(unsigned long) + 1];
                    p.edited_string_length = 0;
                    if (long_argument)
                    {
                        //x                 = *(unsigned long *) argument_pointer;
                        //argument_pointer += sizeof(unsigned long) / sizeof(int);
                        va_arg(va, unsigned int);
                    }
                    else if (control_char == 'd')
                        //x = (long) *argument_pointer++;
                        x = va_arg(va, long);
                    else
                        //x = (unsigned) *argument_pointer++;
                        x = va_arg(va, int);
                    if (control_char == 'd' && (long) x < 0)
                    {
                        p.options |= MINUS_SIGN;
                        x          = -(long) x;
                    }
                    do
                    {
                        int c;
                        c = x % base + '0';
                        if (c > '9')
                        {
                            if (p.options & CAPITAL_HEX)
                                c += 'A' - '9' - 1;
                            else
                                c += 'a' - '9' - 1;
                        }
                        buffer[sizeof(buffer) - 1 - p.edited_string_length++] = c;
                    } while ((x /= base) != 0);
                    if (precision >= 0 && precision > p.edited_string_length)
                        p.leading_zeros = precision - p.edited_string_length;
                    output_field(&p, buffer + sizeof(buffer) - p.edited_string_length);
                }
                control_char = *control_string++;
            }
        }
        else
        {
            output_and_count(&p, control_char);
            control_char = *control_string++;
        }
    }
    return p.number_of_output_chars;
}

int ithPrintf(char* control, ...)
{
    va_list va;
    va_start(va,control);
    gPrint = 0;
    gBuf = (unsigned char*)gtAudioPluginBuffer;
    ithGPrintf(control, va);
    va_end(va);    
    return 0;
}
void AudioPluginAPI(int nType)
{
    unsigned short nRegister;
    int i;
    int nTemp,nTemp1;
    unsigned char* pBuf;
    
    nRegister = (SMTK_AUDIO_PROCESSOR_ID<<14) | nType;
    switch (nType)
    {
        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_OPEN:
            gnTemp = sizeof(tDumpWave);
            //printf("[AMR] name length %d \n",gnTemp);
            memcpy(&gtAudioPluginBuffer[0], &gnTemp, SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH);                           
            memcpy(&gtAudioPluginBuffer[SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH/sizeof(short)], tDumpWave, sizeof(tDumpWave));
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_WRITE:
          // printf("[AMR] name length %d \n",gnTemp);           
            memcpy(&gtAudioPluginBuffer[0], &gnTemp, SMTK_AUDIO_PLUGIN_CMD_FILE_NAME_LENGTH);                           
            {
                int i;
                char tmp;
                char *buf = (char *)&pDecOpBuf[iWrIdx];
                for(i=0; i<gnTemp; i+=2)
                {
                    tmp = buf[i];
                    buf[i] = buf[i+1];
                    buf[i+1] = tmp;
                }
            }           
            memcpy(&gtAudioPluginBuffer[SMTK_AUDIO_PLUGIN_CMD_FILE_WRITE_LENGTH/sizeof(short)], &pDecOpBuf[iWrIdx], gnTemp);           
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_FILE_CLOSE:
           
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC:
                nTemp  = i2s_outbuf;
                nTemp1 = BUFLEN;
                pBuf = (unsigned char*)gtAudioPluginBuffer;
                memcpy(&pBuf[0], &nTemp, sizeof(int));
                memcpy(&pBuf[4], &gCh, sizeof(int));
                memcpy(&pBuf[8], &gSampleRate, sizeof(int));
                memcpy(&pBuf[12], &nTemp1, sizeof(int));
                //printf("[AMR] 0x%x %d %d %d \n",nTemp,gCh,gSampleRate,nTemp1);
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_ADC:
                nTemp  = pDecOpBuf;
                nTemp1 = wDecOpBufLen;            
                //nTemp  = pEncOpBuf;
                //nTemp1 = wEncOpBufLen;
                pBuf = (unsigned char*)gtAudioPluginBuffer;
                memcpy(&pBuf[0], &nTemp, sizeof(int));
                memcpy(&pBuf[4], &gCh, sizeof(int));
                memcpy(&pBuf[8], &gSampleRate, sizeof(int));
                memcpy(&pBuf[12], &nTemp1, sizeof(int));            
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_PAUSE_DAC:
                pBuf = (unsigned char*)gtAudioPluginBuffer;
                memcpy(&pBuf[0], &gPause, sizeof(int));
            break;

        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_DAC:
        case SMTK_AUDIO_PLUGIN_CMD_ID_I2S_DEACTIVE_ADC: 
            break;
        
        case SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF:

            break;

        default:
            break;

    }
    setAudioPluginMessageStatus(nRegister);
    i=200000*20;
    do
    {
        nRegister = getAudioPluginMessageStatus();
        nRegister = (nRegister & 0xc000)>>14;
        if (nRegister== SMTK_MAIN_PROCESSOR_ID)
        {
            //printf("[AMR] get main procerror feedback \n");
            break;
        }
        i--;
    }while(i);
    //if (i==0)
        //printf("[Amr] audio api %d %d\n",i,nType);

}
#endif // defined(AUDIO_PLUGIN_MESSAGE_QUEUE)



#  if defined(__FREERTOS__) && !defined(ENABLE_CODECS_PLUGIN)
portTASK_FUNCTION(amrcodec_task, params)
#  else
int main(int argc, char **argv)
#  endif
{
    Word32 i = 0;
    int* codecStream;
    AUDIO_CODEC_STREAM* audioStream;

    /* Encoder Param */
#  if !defined(AMR_COEDC_DECODE_ONLY)
    Word16 dtx = 0;             /* enable encoder DTX */
#  endif

    Word32 wStage;

#  if !defined(AMR_COEDC_DECODE_ONLY)
    Word32 AECMode = AECOFF;
    iEnc2I2SWrIdx = 0;
    iEnc2I2SRdIdx = 0;
    en_packed_size = -1;
    framecountin = 0;
#  endif
    iWrIdx = 0;
    iRdInx = 0;
    inwrptr = 0;
    wrptrend = 0;
    inrdptr = 0;
    g_bPCMMode = 0;

#  if defined(DUMP_PCM_DATA)
    g_bPCMMode = 1;
#  endif

#  if defined(WIN32) || defined(__CYGWIN__)
    win32_init(argc, argv);
#  endif // defined(WIN32) || defined(__CYGWIN__)

    // mode 0: 475      > 68ms
    // mode 1: 515
    // mode 2: 59
    // mode 3: 67
    // mode 4: 74
    // mode 5: 795
    // mode 6: 102
    // mode 7: 122
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)
    codecStream=CODEC_STREAM_START_ADDRESS;   
    //printf("[Amr] 0x%08x \n",*codecStream);
    audioStream = *codecStream;
    audioStream->codecStreamBuf = &pDecIpBuf[0];
    audioStream->codecStreamLength =  wDecIpBufLen;      
    //printf("[Amr] audioStream 0x%08x 0x%08x 0x%08x   \n",&audioStream,&audioStream->codecStreamBuf,&audioStream->codecStreamLength);
    gtAudioPluginBuffer = audioStream->codecAudioPluginBuf;
    gtAudioPluginBufferLength = audioStream->codecAudioPluginLength;
    //printf("[Amr] 0x%08x %d 0x%08x %d \n",audioStream->codecStreamBuf,audioStream->codecStreamLength ,audioStream->codecAudioPluginBuf,audioStream->codecAudioPluginLength);
    MMIO_Write(AUDIO_DECODER_START_FALG, 1);
#endif

#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)    
        ithPrintf("[AMR] start \n");
        AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_PRINTF);
#else
        
#endif

#  if defined(__FREERTOS__)

#    if !defined(AMR_COEDC_DECODE_ONLY)
    en_mode = MR122;
    dtx = 0;
#    endif // !defined(AMR_COEDC_DECODE_ONLY)

#  elif !defined(AMR_COEDC_DECODE_ONLY)
    switch ((MMIO_Read(DrvAudioCtrl) & DrvAMR_Mode) >> DrvAMR_Mode_Bits) {
        case 0:  en_mode = MR475; break;
        case 1:  en_mode = MR515; break;
        case 2:  en_mode = MR59;  break;
        case 3:  en_mode = MR67;  break;
        case 4:  en_mode = MR74;  break;
        case 5:  en_mode = MR795; break;
        case 6:  en_mode = MR102; break;
        case 7:  en_mode = MR122; break;
        default: en_mode = MR515;
    }

    dtx = (MMIO_Read(DrvAudioCtrl) & DrvAMR_DTX) ? 1 : 0;
#  endif

#  if 0
    en_mode = MR515;
    dtx = 0;
#  endif

#  if defined(__INPUT_CRC_CHECK__)
    crc32_init();
#  endif // defined(__INPUT_CRC_CHECK__)

#  if !defined(AMR_COEDC_DECODE_ONLY)
    /* Encoder */
    SetEncOpWtIdx(0);
    SetEncOpRdIdx(0);
    init_AMREncBuf(speech, INBUFLEN, serialout, OUTBUFLEN);
#  endif

    /* Decoder */
    SetDecOpWrIdx(0);
    init_AMRDecBuf(InputBuf, READBUF_SIZE, i2s_outbuf, BUFLEN);

#  if 0
    initDAC((unsigned char *) pDecOpBuf, 1, 8000, wDecOpBufLen, 0);
#  else
    if (g_bPCMMode == 0) {
#    if !defined(AMR_COEDC_DECODE_ONLY)
        initCODEC((unsigned char *) pEncIpBuf, (unsigned char *) pDecOpBuf, 1, 8000, wEncIpBufLen, wDecOpBufLen, 0);
#    else
#if defined(AUDIO_PLUGIN_MESSAGE_QUEUE)            
            gCh = 1;
            gSampleRate = 8000;
            AudioPluginAPI(SMTK_AUDIO_PLUGIN_CMD_ID_I2S_INIT_DAC);
#else
            initDAC((unsigned char *) pDecOpBuf, 1, 8000, wDecOpBufLen, 0);
#endif        



#    endif
    }
#  endif

#  ifdef ENABLE_PERFORMANCE_MEASURE
    for (i = 0; i < MAX_TIMELOG; i++) {
        time_log[i] = 0;
    }
    tm_record.enc_cnt = 0;
    tm_record.enc_max = 0;
    tm_record.enc_min = 0xffffffff;
    tm_record.enc_max_n = 0;
    tm_record.enc_min_n = 0;
    tm_record.all_cnt = 0;
    tm_record.all_max = 0;
    tm_record.all_min = 0xffffffff;
    tm_record.all_max_n = 0;
    tm_record.all_min_n = 0;
    elapsed = -1;
    nFrames = 0;
#  endif

    Speech_Decode_Frame_init(&speech_decoder_state, "Decoder");

#  if !defined(AMR_COEDC_DECODE_ONLY)
    Speech_Encode_Frame_init(&speech_encoder_state, dtx, "Encoder");
    sid_sync_init(&sid_state);
#  endif

#  if !defined(AMR_COEDC_DECODE_ONLY)
    bEncIpFull = 0;
    bEncIpRdy = 0;
    bEncAECTHD = 0;

    bEncOpRdy = 0;
    bEncDone = 0;
    bAEC2EncRdy = 0;

    iEncFrame = 0;
    encTime = 0;
    amrEncFrameAccu = 0;
#  endif
    bDecIpFull = 0;
    bDecIpRdy = 0;
    bDecOpRdy = 0;
    bDecDone = 0;
    bDec2AECRdy = 0;
    iDecFrame = 0;
    decTime = 0;
    amrDecFrameAccu = 0;

    wDecEOFReached = 0;
    bDecEndHandShake = 0;

    // Detect Magic Word
    bFirstByte = 1;

    for (i = 0; i < AMRINFONUM; i++) {
        amrInfo[i] = 0;
    }

    //packed_bits_debug_idx = 0;
    //EncIpTest_speech_idx = 0;

    //Word32 CodecMode = MODE_CODEC;
    //Word32 CodecMode = MODE_ENCODE;
    //Word32 CodecMode = MODE_DECODE;
    //Word32 AECMode = AECOFF;

#  if defined(__FREERTOS__)
#    if !defined(AMR_COEDC_DECODE_ONLY)
    AECMode = AECOFF;
    CodecMode = MODE_CODEC;
#    else
    CodecMode = MODE_DECODE;
#    endif // !AMR_COEDC_DECODE_ONLY
#  else
#    if !defined(AMR_COEDC_DECODE_ONLY)
    {
        Word32 DrvMode = MMIO_Read(DrvAudioCtrl) & DrvAMR_Type;

        CodecMode = MODE_DECODE;
        AECMode = AECOFF;
        if (DrvMode == DrvAMR_Encode) {
            CodecMode = MODE_ENCODE;
        } else if (DrvMode == DrvAMR_Decode) {
            CodecMode = MODE_DECODE;
        } else if ((DrvMode == DrvAMR_Codec) || (DrvMode == DrvAMR_Codec_AEC)) {
            CodecMode = MODE_CODEC;
            if (DrvMode == DrvAMR_Codec_AEC) {
                AECMode = AECON;
            }
        }
    }
#    else
    CodecMode = MODE_DECODE;
#    endif // !defined(AMR_COEDC_DECODE_ONLY)
#  endif

#  if 0 /* TEST MODE */
    AECMode = AECOFF;
    //CodecMode = MODE_CODEC;
    //CodecMode = MODE_ENCODE;
    CodecMode = MODE_DECODE;
#  endif

    /* Print driver settings */
#  if 1
    PRINTF("MODE PARAM\n");
#    if !defined(AMR_COEDC_DECODE_ONLY)
    switch (CodecMode) {
        case (MODE_CODEC):  PRINTF("    MODE_CODEC\n");  break;
        case (MODE_DECODE): PRINTF("    MODE_DECODE\n"); break;
        case (MODE_ENCODE): PRINTF("    MODE_ENCODE\n"); break;
        default:            PRINTF("    MODE_XXXX\n");   break;
    }
    switch (AECMode) {
        case (AECOFF):      PRINTF("    MODE_AEC_OFF\n"); break;
        case (AECON):       PRINTF("    MODE_AEC_ON\n");  break;
        default:            PRINTF("    MODE_AEC_XXX\n"); break;
    }
#    endif // !defined(AMR_COEDC_DECODE_ONLY)
#  endif

#  if !defined(AMR_COEDC_DECODE_ONLY)
    if (CodecMode == MODE_ENCODE) {
        wStage = SG_AEC;
    } else {
        wStage = SG_DEC;
    }
#  else
    wStage = SG_DEC;
#  endif

#  if !defined(AMR_COEDC_DECODE_ONLY)
    if (CodecMode != MODE_CODEC) {
        AECMode = AECOFF;
    }
#  endif

    //wStage = SG_ENC;
    //wStage = SG_AEC;

#  ifdef ENABLE_PERFORMANCE_MEASURE
    start_timer();
#  endif

    //PRINTF("AMRCodec Begin\n");
    //PRINTF("CodecMode (%d) AEC (%d)\n", CodecMode, AECMode);

    while (1) {
        //PRINTF("wStage == %d\n", wStage);

#  if !defined(AMR_COEDC_DECODE_ONLY)
        GetEncOpRdIdx();
#  endif

        switch (wStage) {
        case SG_DEC:
#  if !defined(AMR_COEDC_DECODE_ONLY)
            if (CodecMode == MODE_ENCODE) {
                wStage = SG_AEC;
                break;
            }
#  endif
            if (!bDecDone) {

#  if !defined(AMR_COEDC_DECODE_ONLY)
                /* encode first priority */
                CK_ENC_INPUT_BUFFER();

                if (!bEncIpRdy || bEncDone || (CodecMode == MODE_DECODE))
#  endif
                {
                    CK_DEC_INPUT_BUFFER();
                    if (bDecIpRdy) {
                        DO_DEC();
                    } else {
                        //amrInfo[DEC_PASS]++;
                    }
                }

            }
            if (bDecDone) {
                CK_DEC_OUTPUT_BUFFER();
                if (bDecOpRdy) {
                    FLUSH_DEC_OUT();
                } else {
                    PRINTF("[AMRDEC] Wait Output Buffer\n");
                }
            }
#  if !defined(AMR_COEDC_DECODE_ONLY)
            wStage = SG_AEC;
#  else
            wStage = SG_DEC;
#  endif
            break;

#  if !defined(AMR_COEDC_DECODE_ONLY)
        case SG_AEC:
            /* TEST ENCODER ONLY */
            if (!bEncDone) {
                CK_ENC_INPUT_BUFFER();
                if (bEncIpRdy) {
                    if ((AECMode == AECON) && bDec2AECRdy && !bEncAECTHD) {
                        DO_AEC();
                    } else {
                        PASS_AEC();
                    }
                    wStage = SG_ENC;
                } else {
                    if (CodecMode == MODE_ENCODE) {
                        wStage = SG_ENC;
                    } else {
                        wStage = SG_DEC;
                    }
                }
            } else {
                wStage = SG_ENC;
            }
            break;

        case SG_ENC:
            /* Main */
            /* TODO S034
               Have to totally remove encoding when just MODE_DECODE
             */
            if (CodecMode == MODE_DECODE) {
                wStage = SG_DEC;
                break;
            }
            if (!bEncDone && bAEC2EncRdy) {
                DO_ENC();
            }
            if (bEncDone) {
                CK_ENC_OUTPUT_BUFFER();
                if (bEncOpRdy) {
                    FLUSH_ENC_OUT();
#    if defined(ENABLE_PERFORMANCE_MEASURE)
                    {
                        unsigned int tm;
                        elapsed = get_timer();
                        tm = (elapsed << 8) / or32_getTicks_ms();

                        if (nFrames < MAX_TIMELOG)
                            time_log[nFrames] = elapsed;

                        if (nFrames >= 1) {     // Skip first frame.
                            if (tm >= (20 << 8))
                                tm_record.all_cnt++;

                            if (tm > tm_record.all_max) {
                                tm_record.all_max = tm;
                                tm_record.all_max_n = nFrames;
                            }

                            if (tm < tm_record.all_min) {
                                tm_record.all_min = tm;
                                tm_record.all_min_n = nFrames;
                            }
                        }

                        nFrames++;
                        start_timer();
                    }
#    endif                      // ENABLE_PERFORMANCE_MEASURE
                }
            }
            wStage = SG_DEC;
            break;

        default:
            break;
#  endif
        }                       /* switch (wStage) */

        // Switch Task
        SwitchTask();

    }

#  if !defined(__FREERTOS__)
    return 0;
#  endif

}
#endif

