/* ***** BEGIN LICENSE BLOCK *****
 * Source last modified: $Id: aaccommon.h,v 1.1 2005/02/26 01:47:34 jrecker Exp $
 *
 * Portions Copyright (c) 1995-2005 RealNetworks, Inc. All Rights Reserved.
 *
 * The contents of this file, and the files included with this file,
 * are subject to the current version of the RealNetworks Public
 * Source License (the "RPSL") available at
 * http://www.helixcommunity.org/content/rpsl unless you have licensed
 * the file under the current version of the RealNetworks Community
 * Source License (the "RCSL") available at
 * http://www.helixcommunity.org/content/rcsl, in which case the RCSL
 * will apply. You may also obtain the license terms directly from
 * RealNetworks.  You may not use this file except in compliance with
 * the RPSL or, if you have a valid RCSL with RealNetworks applicable
 * to this file, the RCSL.  Please see the applicable RPSL or RCSL for
 * the rights, obligations and limitations governing use of the
 * contents of the file.
 *
 * This file is part of the Helix DNA Technology. RealNetworks is the
 * developer of the Original Code and owns the copyrights in the
 * portions it created.
 *
 * This file, and the files included with this file, is distributed
 * and made available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS
 * ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET
 * ENJOYMENT OR NON-INFRINGEMENT.
 *
 * Technology Compatibility Kit Test Suite(s) Location:
 *    http://www.helixcommunity.org/content/tck
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

/**************************************************************************************
 * Fixed-point HE-AAC decoder
 * Jon Recker (jrecker@real.com)
 * February 2005
 *
 * aaccommon.h - implementation-independent API's, datatypes, and definitions
 **************************************************************************************/

#ifndef _AACCOMMON_H
#define _AACCOMMON_H

#include "config.h"
#include "aacdec.h"

#if defined(USE_STATNAME)
#  include "statname.h"
#endif // defined(USE_STATNAME)

/* 12-bit syncword */
#define SYNCWORDH           0xff
#define SYNCWORDL           0xf0
#ifdef AAC_SUPPORT_MULTICHANNELS
#define MAX_NCHANS_ELEM     6   /* max number of channels in any single bitstream element (SCE,CPE,CCE,LFE) */
#else
#define MAX_NCHANS_ELEM     2   /* max number of channels in any single bitstream element (SCE,CPE,CCE,LFE) */
#endif
#define ADTS_HEADER_BYTES   7
#define NUM_SAMPLE_RATES    12
#define NUM_DEF_CHAN_MAPS   8
#define NUM_ELEMENTS        8
#define MAX_NUM_PCE_ADIF    16

#define MAX_WIN_GROUPS      8
#define MAX_SFB_SHORT       15
#define MAX_SF_BANDS        (MAX_SFB_SHORT*MAX_WIN_GROUPS)  /* worst case = 15 sfb's * 8 windows for short block */
#define MAX_MS_MASK_BYTES   ((MAX_SF_BANDS + 7) >> 3)
#define MAX_PRED_SFB        41
#define MAX_TNS_FILTERS     8
#define MAX_TNS_COEFS       60
#define MAX_TNS_ORDER       20
#define MAX_PULSES          4
#define MAX_GAIN_BANDS      3
#define MAX_GAIN_WIN        8
#define MAX_GAIN_ADJUST     7

#define NSAMPS_LONG         1024
#define NSAMPS_SHORT        128

#define NUM_SYN_ID_BITS     3
#define NUM_INST_TAG_BITS   4

#define EXT_SBR_DATA        0x0d
#define EXT_SBR_DATA_CRC    0x0e

#define IS_ADIF(p)      ((p)[0] == 'A' && (p)[1] == 'D' && (p)[2] == 'I' && (p)[3] == 'F')
#define IS_LATM(p)      ((p)[0] == 0x56 && ((p)[1]&0xe0) == 0xe0 )
#define GET_ELE_ID(p)   ((AACElementID)(*(p) >> (8-NUM_SYN_ID_BITS)))

/* AAC file format */
enum {
    AAC_FF_Unknown = 0,     /* should be 0 on init */

    AAC_FF_ADTS = 1,
    AAC_FF_ADIF = 2,
    AAC_FF_RAW =  3,
    AAC_FF_LATM = 4
};

/* syntactic element type */
enum {
    AAC_ID_INVALID = -1,

    AAC_ID_SCE =  0,
    AAC_ID_CPE =  1,
    AAC_ID_CCE =  2,
    AAC_ID_LFE =  3,
    AAC_ID_DSE =  4,
    AAC_ID_PCE =  5,
    AAC_ID_FIL =  6,
    AAC_ID_END =  7
};

typedef struct _AACDecInfo {
    /* pointers to platform-specific state information */
    void *psInfoBase;   /* baseline MPEG-4 LC decoding */
    void *psInfoSBR;    /* MPEG-4 SBR decoding */

    /* raw decoded data, before rounding to 16-bit PCM (for postprocessing such as SBR) */
    void *rawSampleBuf[AAC_MAX_NCHANS];
    int rawSampleBytes;
    int rawSampleFBits;

    /* fill data (can be used for processing SBR or other extensions) */
    unsigned char *fillBuf;
    int fillCount;
    int fillExtType;

    /* block information */
    int prevBlockID;
    int currBlockID;
    int currInstTag;
    int sbDeinterleaveReqd[MAX_NCHANS_ELEM];
    int adtsBlocksLeft;

    /* user-accessible info */
    int bitRate;
    int nChans;
    int sampRate;
    int profile;
    int format;
    int sbrEnabled;
    int tnsUsed;
    int pnsUsed;
    int frameCount;
    int nFrameLength; // frame length
    int nAvgFrameLength;
    int nTempLength;
    int nEnableRecompense;
    int nPayloadSize;
    int nBitOffsets;
#if defined(ADTS_EXTENSION_FOR_PTS)
    int PTS;
#endif // defined(ADTS_EXTENSION_FOR_PTS)

} AACDecInfo;

/* decoder functions which must be implemented for each platform */
/*MMP_INLINE*/ AACDecInfo *AllocateBuffers(void);

/*MMP_INLINE*/ int UnpackADTSHeader(AACDecInfo *aacDecInfo, unsigned char **buf, int *bitOffset, int *bitsAvail);
/*MMP_INLINE*/ int GetADTSChannelMapping(AACDecInfo *aacDecInfo, unsigned char *buf, int bitOffset, int bitsAvail);
/*MMP_INLINE*/ int UnpackADIFHeader(AACDecInfo *aacDecInfo, unsigned char **buf, int *bitOffset, int *bitsAvail);
/*MMP_INLINE*/ int SetRawBlockParams(AACDecInfo *aacDecInfo, int copyLast, int nChans, int sampRate, int profile);
/*MMP_INLINE*/ int PrepareRawBlock(AACDecInfo *aacDecInfo);
/*MMP_INLINE*/ int FlushCodec(AACDecInfo *aacDecInfo);

/*MMP_INLINE*/ int DecodeNextElement(AACDecInfo *aacDecInfo, unsigned char **buf, int *bitOffset, int *bitsAvail);
/*MMP_INLINE*/ int DecodeNoiselessData(AACDecInfo *aacDecInfo, unsigned char **buf, int *bitOffset, int *bitsAvail, int ch);

/*MMP_INLINE*/ int Dequantize(AACDecInfo *aacDecInfo, int ch);
/*MMP_INLINE*/ int StereoProcess(AACDecInfo *aacDecInfo);
/*MMP_INLINE*/ int DeinterleaveShortBlocks(AACDecInfo *aacDecInfo, int ch);
/*MMP_INLINE*/ int PNS(AACDecInfo *aacDecInfo, int ch);
/*MMP_INLINE*/ int TNSFilter(AACDecInfo *aacDecInfo, int ch);
/*MMP_INLINE*/ int IMDCT(AACDecInfo *aacDecInfo, int ch, int chBase, PCM_WORD *outbuf);

#if defined(AAC_ENABLE_SBR)
/* SBR specific functions */
/*MMP_INLINE*/ int InitSBR(AACDecInfo *aacDecInfo);
/*MMP_INLINE*/ void FreeSBR(AACDecInfo *aacDecInfo);
/*MMP_INLINE*/ int DecodeSBRBitstream(AACDecInfo *aacDecInfo, int chBase);
/*MMP_INLINE*/ int DecodeSBRData(AACDecInfo *aacDecInfo, int chBase, short *outbuf);
/*MMP_INLINE*/ int FlushCodecSBR(AACDecInfo *aacDecInfo);
#endif // AAC_ENABLE_SBR

/* aactabs.c - global ROM tables */
extern const int sampRateTab[NUM_SAMPLE_RATES];
extern const int predSFBMax[NUM_SAMPLE_RATES];
extern const int channelMapTab[NUM_DEF_CHAN_MAPS];
extern const int elementNumChans[NUM_ELEMENTS];
extern const unsigned char sfBandTotalShort[NUM_SAMPLE_RATES];
extern const unsigned char sfBandTotalLong[NUM_SAMPLE_RATES];
extern const int sfBandTabShortOffset[NUM_SAMPLE_RATES];
extern const short sfBandTabShort[76];
extern const int sfBandTabLongOffset[NUM_SAMPLE_RATES];
extern const short sfBandTabLong[325];
extern const int tnsMaxBandsShortOffset[AAC_NUM_PROFILES];
extern const unsigned char tnsMaxBandsShort[2*NUM_SAMPLE_RATES];
extern const unsigned char tnsMaxOrderShort[AAC_NUM_PROFILES];
extern const int tnsMaxBandsLongOffset[AAC_NUM_PROFILES];
extern const unsigned char tnsMaxBandsLong[2*NUM_SAMPLE_RATES];
extern const unsigned char tnsMaxOrderLong[AAC_NUM_PROFILES];

#endif  /* _AACCOMMON_H */
