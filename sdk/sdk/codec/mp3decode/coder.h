/* ***** BEGIN LICENSE BLOCK *****
 * Version: RCSL 1.0/RPSL 1.0
 *
 * Portions Copyright (c) 1995-2002 RealNetworks, Inc. All Rights Reserved.
 *
 * The contents of this file, and the files included with this file, are
 * subject to the current version of the RealNetworks Public Source License
 * Version 1.0 (the "RPSL") available at
 * http://www.helixcommunity.org/content/rpsl unless you have licensed
 * the file under the RealNetworks Community Source License Version 1.0
 * (the "RCSL") available at http://www.helixcommunity.org/content/rcsl,
 * in which case the RCSL will apply. You may also obtain the license terms
 * directly from RealNetworks.  You may not use this file except in
 * compliance with the RPSL or, if you have a valid RCSL with RealNetworks
 * applicable to this file, the RCSL.  Please see the applicable RPSL or
 * RCSL for the rights, obligations and limitations governing use of the
 * contents of the file.
 *
 * This file is part of the Helix DNA Technology. RealNetworks is the
 * developer of the Original Code and owns the copyrights in the portions
 * it created.
 *
 * This file, and the files included with this file, is distributed and made
 * available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 *
 * Technology Compatibility Kit Test Suite(s) Location:
 *    http://www.helixcommunity.org/content/tck
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

/**************************************************************************************
 * Fixed-point MP3 decoder
 * Jon Recker (jrecker@real.com), Ken Cooke (kenc@real.com)
 * June 2003
 *
 * coder.h - private, implementation-specific header file
 **************************************************************************************/

#ifndef __CODER_H__
#define __CODER_H__

//#include "proj_defs.h"
#include "mp3_config.h"
#include "type.h"
#include "stdefs.h"

#if defined(WIN32) || defined(__CYGWIN__)
    //#define STAGE_DEBUG
    //#define ENGINE_DEBUG
    //#define ENGINE_DEBUG_PRINT_FUNC
    //#define OVERFLOW_DEBUG
    //#define LAYERII_DEBUG

    #if defined(STAGE_DEBUG)
    extern int *dbgST_UnpackSF;
    extern int *dbgST_DecodeHuffman;
    extern int *dbgST_Dequantize;
    extern int *dbgST_IMDCT;
    extern int *dbgST_Synthesis;
    #endif

    #if defined(ENGINE_DEBUG)
    extern int dbgENG_cnt;
    extern int *dbgENG_srcfile;
    extern int *dbgENG_dstfile0;
    extern int *dbgENG_dstfile1;
    extern int *dbgENG_dstfile2;
    extern int *dbgENG_dstfile3;
    #endif

    #if defined(LAYERII_DEBUG)
    extern FILE *dbgLayerII;
    #endif

    void initDbg();
    void finishDbg();
#endif

extern int nFrames;

/* map to 0,1,2 to make table indexing easier */
typedef enum {
    MPEG1 =  0,
    MPEG2 =  1,
    MPEG25 = 2
} MPEGVersion;

#define MAX_SCFBD       4       /* max scalefactor bands per channel */
#define NGRANS_MPEG1    2
#define NGRANS_MPEG2    1

/* 11-bit syncword if MPEG 2.5 extensions are enabled */
#define SYNCWORDH       0xff
#define SYNCWORDL       0xe0

/* 12-bit syncword if MPEG 1,2 only are supported
 * #define  SYNCWORDH       0xff
 * #define  SYNCWORDL       0xf0
 */

typedef struct _SFBandTable {
    short l[23];
    short s[14];
} SFBandTable;

///////////////////////////////////////////////////////////////////////////////////
//    ORIGINAL CODER.H
///////////////////////////////////////////////////////////////////////////////////
#ifndef MAX
#define MAX(a,b)    ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)    ((a) < (b) ? (a) : (b))
#endif

#define SIBYTES_MPEG1_MONO      17
#define SIBYTES_MPEG1_STEREO    32
#define SIBYTES_MPEG2_MONO       9
#define SIBYTES_MPEG2_STEREO    17

/* number of fraction bits for pow43Tab (see comments there) */
#define POW43_FRACBITS_LOW      22
#define POW43_FRACBITS_HIGH     12

//#define DQ_FRACBITS_OUT       25  /* number of fraction bits in output of dequant */
#define DQ_FRACBITS_OUT         26  // yiwei, by experiment, the output seems always S0.26
#define IMDCT_SCALE             2   /* additional scaling (by sqrt(2)) for fast IMDCT36 */

#define HUFF_PAIRTABS           32
#define BLOCK_SIZE              18
#define NBANDS                  32
#define MAX_REORDER_SAMPS       ((192-126)*3)       /* largest critical band for short blocks (see sfBandTable) */
#define VBUF_LENGTH             (17 * 2 * NBANDS)   /* for double-sized vbuf FIFO */

#define ENCODE_ERROR_COUNT      110
/* map these to the corresponding 2-bit values in the frame header */
typedef enum {
    Stereo = 0x00,  /* two independent channels, but L and R frames might have different # of bits */
    Joint = 0x01,   /* coupled channels - layer III: mix of M-S and intensity, Layers I/II: intensity and direct coding only */
    Dual = 0x02,    /* two independent channels, L and R always have exactly 1/2 the total bitrate */
    Mono = 0x03     /* one channel */
} StereoMode;

typedef struct _BitStreamInfo {
    unsigned char *bytePtr;
    unsigned int iCache;
    int cachedBits;
    int nBytes;
} BitStreamInfo;

typedef struct _FrameHeader {
    MPEGVersion ver;    /* version ID */
    int layer;          /* layer index (1, 2, or 3) */
    int crc;            /* CRC flag: 0 = disabled, 1 = enabled */
    int brIdx;          /* bitrate index (0 - 15) */
    int srIdx;          /* sample rate index (0 - 2) */
    int paddingBit;     /* padding flag: 0 = no padding, 1 = single pad byte */
    int privateBit;     /* unused */
    StereoMode sMode;   /* mono/stereo mode */
    int modeExt;        /* used to decipher joint stereo mode */
    int copyFlag;       /* copyright flag: 0 = no, 1 = yes */
    int origFlag;       /* original flag: 0 = copy, 1 = original */
    int emphasis;       /* deemphasis mode */
    int CRCWord;        /* CRC word (16 bits, 0 if crc not enabled) */

    const SFBandTable *sfBand;
} FrameHeader;

typedef struct _SideInfoSub {
    int part23Length;       /* number of bits in main data */
    int nBigvals;           /* 2x this = first set of Huffman cw's (maximum amplitude can be > 1) */
    int globalGain;         /* overall gain for dequantizer */
    int sfCompress;         /* unpacked to figure out number of bits in scale factors */
    int winSwitchFlag;      /* window switching flag */
    int blockType;          /* block type */
    int mixedBlock;         /* 0 = regular block (all short or long), 1 = mixed block */
    int tableSelect[3];     /* index of Huffman tables for the big values regions */
    int subBlockGain[3];    /* subblock gain offset, relative to global gain */
    int region0Count;       /* 1+region0Count = num scale factor bands in first region of bigvals */
    int region1Count;       /* 1+region1Count = num scale factor bands in second region of bigvals */
    int preFlag;            /* for optional high frequency boost */
    int sfactScale;         /* scaling of the scalefactors */
    int count1TableSelect;  /* index of Huffman table for quad codewords */
} SideInfoSub;

typedef struct _SideInfo {
    int mainDataBegin;
    int privateBits;
    int scfsi[MAX_NCHAN][MAX_SCFBD];                /* 4 scalefactor bands per channel */

    SideInfoSub sis[MAX_NGRAN][MAX_NCHAN];
} SideInfo;

typedef struct {
    int cbType;     /* pure long = 0, pure short = 1, mixed = 2 */
    int cbEndS[3];  /* number nonzero short cb's, per subbblock */
    int cbEndSMax;  /* max of cbEndS[] */
    int cbEndL;     /* number nonzero long cb's  */
} CriticalBandInfo;

typedef struct _DequantInfo {
    int workBuf[MAX_REORDER_SAMPS];     /* workbuf for reordering short blocks */
    CriticalBandInfo cbi[MAX_NCHAN];    /* filled in dequantizer, used in joint stereo reconstruction */
} DequantInfo;

typedef struct _HuffmanInfo {
    int huffDecBuf[MAX_NGRAN][MAX_NCHAN][MAX_NSAMP];        /* used both for decoded Huffman values and dequantized coefficients */
    int nonZeroBound[MAX_NGRAN][MAX_NCHAN];             /* number of coeffs in huffDecBuf[ch] which can be > 0 */
} HuffmanInfo;

typedef enum _HuffTabType {
    noBits,
    oneShot,
    loopNoLinbits,
    loopLinbits,
    quadA,
    quadB,
    invalidTab
} HuffTabType;

typedef struct _HuffTabLookup {
    int linBits;
    HuffTabType tabType;
} HuffTabLookup;

// for PHASE4 imdct_new.c
typedef struct _IMDCTInfo {
    int outBuf[MAX_NGRAN][MAX_NCHAN][BLOCK_SIZE][NBANDS];   // output of IMDCT
    int prevblck[MAX_NCHAN][NBANDS*BLOCK_SIZE]; // overlap-add buffer
    int rawout[2*BLOCK_SIZE];                   // working buffer
//  int numPrevIMDCT[MAX_NCHAN];                // how many IMDCT's calculated in this channel on prev. granule
//  int prevType[MAX_NCHAN];
//  int prevWinSwitch[MAX_NCHAN];
} IMDCTInfo;

/* max bits in scalefactors = 5, so use char's to save space */
typedef struct _ScaleFactorInfoSub {
    char l[23];            /* [band] */
    char s[13][3];         /* [band][window] */
} ScaleFactorInfoSub;

/* used in MPEG 2, 2.5 intensity (joint) stereo only */
typedef struct _ScaleFactorJS {
    int intensityScale;
    int slen[4];
    int nr[4];
} ScaleFactorJS;

typedef struct _ScaleFactorInfo {
    ScaleFactorInfoSub sfis[MAX_NGRAN][MAX_NCHAN];
    ScaleFactorJS sfjs[MAX_NGRAN];
} ScaleFactorInfo;

/* NOTE - could get by with smaller vbuf if memory is more important than speed
 *  (in Subband, instead of replicating each block in FDCT32 you would do a memmove on the
 *   last 15 blocks to shift them down one, a hardware style FIFO)
 */
typedef struct _SubbandInfo {
    //int vbuf[MAX_NCHAN * VBUF_LENGTH];    // original /* vbuf for fast DCT-based synthesis PQMF - double size for speed (no modulo indexing) */
    //double vbuf[34][2][16];               // before PHASE5
    int vbuf[34][2][16];
    int vindex;                             /* internal index for tracking position in vbuf */
} SubbandInfo;

/* hufftabs.c */
extern const HuffTabLookup huffTabLookup[HUFF_PAIRTABS];
extern const int huffTabOffset[HUFF_PAIRTABS];
extern const unsigned short huffTable[];
extern const unsigned char quadTable[64+16];
extern const int quadTabOffset[2];
extern const int quadTabMaxBits[2];

/* trigtabs.c */
extern const int imdctWin[4][36];
extern const int ISFMpeg1[2][7];
extern const int ISFMpeg2[2][2][16];
extern const int ISFIIP[2][2];
extern const int cs[8];
extern const int ca[8];
extern const int coef32[31];
extern const int polyCoef[264];

///////////////////////////////////////////////////////////////////////////////////
//  LOWER PART OF ORIGINAL MP3COMMON.H
///////////////////////////////////////////////////////////////////////////////////

typedef struct _MP3DecInfo {
    /* pointers to platform-specific data structures */
    FrameHeader     FrameHeaderPS;
    SideInfo        SideInfoPS;
    ScaleFactorInfo ScaleFactorInfoPS;
    HuffmanInfo     HuffmanInfoPS;
    DequantInfo     DequantInfoPS;
    IMDCTInfo       IMDCTInfoPS;
    SubbandInfo     SubbandInfoPS;

    /* buffer which must be large enough to hold largest possible main_data section */
    unsigned char mainBuf[MAINBUF_SIZE];

    /* special info for "free" bitrate files */
    int freeBitrateFlag;
    int freeBitrateSlots;

    /* user-accessible info */
    int bitrate;
    int nChans;
    int samprate;
    int nGrans;             /* granules per frame */
    int nGranSamps;         /* samples per granule */
    int nFrameSamps;        /* samples per frame(per ch) == nGrans * nGranSamps */
	int nSubbands;
    int nSlots;
    int layer;
    MPEGVersion version;

    int mainDataBegin;
    int mainDataBytes;

    int part23Length[MAX_NGRAN][MAX_NCHAN];

} MP3DecInfo;

int DecodeHuffman(unsigned char *buf, int *bitOffset, int huffBlockBits, int gr, int ch);
//int Dequantize(int gr);
int DequantBlock(int *inbuf, int *outbuf, int num, int scale);
void DequantizeGr();
void ProcessStereoGr();
void AntiAliasGrCh();   // added in PHASE12 of imdct_new.c
//void IMDCT(int gr, int ch);
void HybridTransformGrCh(); // added in PHASE13 of imdct_new.c
int UnpackScaleFactors(unsigned char *buf, int *bitOffset, int bitsAvail, int gr, int ch);
int UnpackFrameHeader(unsigned char *buf);
//int Subband(short *pcmBuf);
//void synthesis_subband(short *pcmBuf, int gr);
void SubbandTransformGr(int *pcmBuf);   //void SubbandTransformGr(short *pcmBuf);

void initOutput(int *buf0, int *buf1, int nch, int blksize, int blkcnt);
void outputData();

#if defined(PCMCHKSUM)
extern unsigned short pcmChksum;
void initPCMCheckSum();
void DumpPCMCheckSum();
#endif

/* mp3tabs.c - global ROM tables */
extern const int samplerateTab[3][3];
extern const short bitrateTab[3][3][15];
extern const short samplesPerFrameTab[3][3];
extern const short bitsPerSlotTab[3];
extern const short sideBytesTab[3][2];
extern const short slotTab[3][3][15];
extern const SFBandTable sfBandTable[3][3];

///////////////////////////////////////////////////////////////////////////////////
//  ORIGINAL MP3DEC.H
///////////////////////////////////////////////////////////////////////////////////

enum {
    ERR_MP3_NONE =                0x0,
    ERR_MP3_INDATA_UNDERFLOW    = 0x1,
    ERR_MP3_MAINDATA_UNDERFLOW  = 0x2,
    ERR_MP3_FREE_BITRATE_SYNC   = 0x4,
    ERR_MP3_INVALID_FRAMEHEADER = 0x8,
    ERR_MP3_INVALID_SIDEINFO    = 0x10,
    ERR_MP3_INVALID_SCALEFACT   = 0x20,
    ERR_MP3_INVALID_HUFFCODES   = 0x40,
    ERR_MP3_INVALID_ENCODEFILE  = 0x80
};

typedef struct _MP3FrameInfo {
    int bitrate;
    int nChans;
    int samprate;
    int bitsPerSample;
    int outputSamps;
    int layer;
    int version;
} MP3FrameInfo;

/* public API */
int MP3Decode(STREAMBUF *inbuf, SAMPBUF *outbuf, param_eq_struct *p_param);
void MP3GetLastFrameInfo();

extern MP3DecInfo mp3DecInfo;
extern MP3FrameInfo mp3FrameInfo;

//*************************************************************************************
// Function:    SetBitstreamPointer
//
// Description: initialize bitstream reader
//
// Inputs:      pointer to BitStreamInfo struct
//              number of bytes in bitstream
//              pointer to byte-aligned buffer of data to read from
//
// Outputs:     filled bitstream info struct
//
// Return:      none
//*************************************************************************************
static __inline void SetBitstreamPointer(BitStreamInfo *bsi, int nBytes, unsigned char *buf)
{
    // init bitstream
    bsi->bytePtr = buf;
    bsi->iCache = 0;        // 4-byte unsigned int
    bsi->cachedBits = 0;    // i.e. zero bits in cache
    bsi->nBytes = nBytes;
}

// *************************************************************************************
// Function:    GetBits
//
// Description: get bits from bitstream, advance bitstream pointer
//
// Inputs:      pointer to initialized BitStreamInfo struct
//              number of bits to get from bitstream
//
// Outputs:     updated bitstream info struct
//
// Return:      the next nBits bits of data from bitstream buffer
//
// Notes:       nBits must be in range [0, 31], nBits outside this range masked by 0x1f
//              for speed, does not indicate error if you overrun bit buffer
//              if nBits = 0, returns 0 (useful for scalefactor unpacking)
//
// TODO:        optimize for ARM
// *************************************************************************************
static __inline unsigned int GetBits(BitStreamInfo *bsi, int nBits)
{
    register unsigned int data, lowBits;
    register unsigned int iCache;
    register int cachedBits;

    iCache = bsi->iCache;
    cachedBits = bsi->cachedBits;

    nBits &= 0x1f;                  // nBits mod 32 to avoid unpredictable results like >> by negative amount
    data = iCache >> (31 - nBits);  // unsigned >> so zero-extend
    data >>= 1;                     // do as >> 31, >> 1 so that nBits = 0 works okay (returns 0)
    iCache <<= nBits;               // left-justify cache
    cachedBits -= nBits;            // how many bits have we drawn from the cache so far

    // if we cross an int boundary, refill the cache
    if (cachedBits < 0) {
        lowBits = -cachedBits;
        //RefillBitstreamCache(bsi);
        {
            register int nBytes = bsi->nBytes;
            register unsigned char *bytePtr = bsi->bytePtr;
            // optimize for common case, independent of machine endian-ness
            if (nBytes >= 4) {
                iCache  = (*bytePtr++) << 24;
                iCache |= (*bytePtr++) << 16;
                iCache |= (*bytePtr++) <<  8;
                iCache |= (*bytePtr++);
                cachedBits = 32;
                bsi->nBytes -= 4;
            } else {
                iCache = 0;
                while (nBytes--) {
                    iCache |= (*bytePtr++);
                    iCache <<= 8;
                }
                nBytes = (bsi->nBytes)*8;
                iCache <<= (24 - nBytes);
                cachedBits = nBytes;
                bsi->nBytes = 0;
            }
            bsi->bytePtr = bytePtr;
        }

        data |= iCache >> (32 - lowBits);       // get the low-order bits
        cachedBits -= lowBits;                  // how many bits have we drawn from the cache so far
        iCache <<= lowBits;                     // left-justify cache
    }

    bsi->iCache = iCache;
    bsi->cachedBits = cachedBits;

    return data;
}

//*************************************************************************************
// Function:    CalcBitsUsed
//
// Description: calculate how many bits have been read from bitstream
//
// Inputs:      pointer to initialized BitStreamInfo struct
//              pointer to start of bitstream buffer
//              bit offset into first byte of startBuf (0-7)
//
// Outputs:     none
//
// Return:      number of bits read from bitstream, as offset from startBuf:startOffset
//*************************************************************************************
static __inline int CalcBitsUsed(BitStreamInfo *bsi, unsigned char *startBuf, int startOffset)
{
    int bitsUsed;

    bitsUsed  = (bsi->bytePtr - startBuf) * 8;
    bitsUsed -= bsi->cachedBits;
    bitsUsed -= startOffset;

    return bitsUsed;
}

//*************************************************************************************
// Function:    CheckPadBit
//
// Description: check whether padding byte is present in an MP3 frame
//
// Inputs:      MP3DecInfo struct with valid FrameHeader struct
//                (filled by UnpackFrameHeader())
//
// Outputs:     none
//
// Return:      1 if pad bit is set, 0 if not, -1 if null input pointer
//*************************************************************************************
static __inline int CheckPadBit()
{
    return mp3DecInfo.FrameHeaderPS.paddingBit ? 1 : 0;
}

#if defined(PURE_C)
static __inline int MULSHIFT32(int x, int y)
{
    int64 xext,yext;
    xext =(int64)x; yext=(int64)y;
    yext=xext*yext;
    return (int)(yext>>32);
}

static __inline int FASTABS(int x)
{
    int sign;

    sign = x >> (sizeof(int) * 8 - 1);
    x ^= sign;
    x -= sign;

    return x;
}

static __inline int CLZ(int x)
{
    int numZeros;

    if (!x)
        return (sizeof(int) * 8);

    numZeros = 0;
    while (!(x & 0x80000000)) {
        numZeros++;
        x <<= 1;
    }

    return numZeros;
}

#elif defined(WIN32) || defined(__CYGWIN__)

static int64 macc64 = 0; 
static __inline void SETACC64(int lo, int hi)
{
	macc64 = ((int64)hi<<32) | lo;
}

static __inline void WRACC64_ADD(int x, int y)
{
    macc64 += (int64)x * y;
}

static __inline void WRACC64_SUB(int x, int y)
{
    macc64 -= (int64)x * y;
}

static __inline int RDACC64_SHIFT(int shift)
{
	int ret = (int)(macc64>>shift);
	macc64 = 0;
	return ret;
}
static __inline int ADD(int x, int y)
{
    int z = x+y;
#if defined(OVERFLOW_DEBUG)
    double fx, fy, diff;
    fx=x; fy = y;
    diff = (double)z - (fx+fy);
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
#endif

    return z;
}

static __inline int SUB(int x, int y)
{
    int z = x-y;
#if defined(OVERFLOW_DEBUG)
    double fx, fy, diff;
    fx=x; fy = y;
    diff = (double)z - (fx-fy);
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
#endif
    return z;
}

static __inline int MULSHIFT(int x, int y, int shift)
{
    int64 xext, yext;
    xext=(int64)x;  yext=(int64)y;
    xext = (xext * yext)>>shift;

#if defined(OVERFLOW_DEBUG)
    {
    double fx, fy, diff;
    fx=x; fy = y;
    fx = (fx*fy)*g_ovflw_shift_coef[shift];
    diff = (double)xext - fx;
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
    }
#endif

    return (int)xext;
}

static __inline int MULSHIFT32(int x, int y)
{
    int64 xext,yext;
    xext =(int64)x; yext=(int64)y;
    xext=(xext*yext)>>32;

#if defined(OVERFLOW_DEBUG)
    {
    double fx, fy, diff;
    fx=x; fy = y;
    fx = (fx*fy)*g_ovflw_shift_coef[32];
    diff = (double)xext - fx;
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
    }
#endif

    return (int)xext;
}

static __inline int MADDSHIFT(int x1, int y1, int x2, int y2, int shift)
{
    int64 ex1, ey1, ex2, ey2;
    ex1=(int64)x1;  ey1=(int64)y1;
    ex2=(int64)x2;  ey2=(int64)y2;

    ex1 = ex1 * ey1;
    ex2 = ex2 * ey2;
    ex1 = (ex1 + ex2) >> shift;

#if defined(OVERFLOW_DEBUG)
    {
    double fx1, fy1, fx2, fy2, diff;
    fx1=x1; fy1 = y1; fx2=x2; fy2 = y2;
    fx1 = ((fx1*fy1)+(fx2*fy2))*g_ovflw_shift_coef[shift];
    diff = (double)ex1 - fx1;
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
    }
#endif

    return (int)ex1;
}

static __inline int MSUBSHIFT(int x1, int y1, int x2, int y2, int shift)
{
    int64 ex1, ey1, ex2, ey2;
    ex1=(int64)x1;  ey1=(int64)y1;
    ex2=(int64)x2;  ey2=(int64)y2;

    ex1 = ex1 * ey1;
    ex2 = ex2 * ey2;
    ex1 = (ex1 - ex2) >> shift;

#if defined(OVERFLOW_DEBUG)
    {
    double fx1, fy1, fx2, fy2, diff;
    fx1=x1; fy1 = y1; fx2=x2; fy2 = y2;
    fx1 = ((fx1*fy1)-(fx2*fy2))*g_ovflw_shift_coef[shift];
    diff = (double)ex1 - fx1;
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
    }
#endif

    return (int)ex1;
}

static __inline int FASTABS(int x)
{
    int sign;

    sign = x >> (sizeof(int) * 8 - 1);
    x ^= sign;
    x -= sign;

    return x;
}

static __inline int CLZ(int x)
{
    int numZeros;

    if (!x)
        return (sizeof(int) * 8);

    numZeros = 0;
    while (!(x & 0x80000000)) {
        numZeros++;
        x <<= 1;
    }

    return numZeros;
}

#elif defined(__OR32__)

//#define OVERFLOW_DEBUG

#define SETACC64(lo, hi)   ({  \
    asm volatile ("l.mtspr r0, %0, 0x2801" : : "r"(lo)); \
    asm volatile ("l.mtspr r0, %0, 0x2802" : : "r"(hi)); \
})

#define WRACC64_ADD(x, y)   ({  \
    asm volatile ("l.mac %0, %1"::"%r" (x), "r"(y));  \
})

#define WRACC64_SUB(x, y)   ({  \
    asm volatile ("l.msb %0, %1"::"%r" (x), "r"(y));  \
})

#define RDACC64_SHIFT(shift)   ({  \
    register int ret;	\
    asm volatile ("l.macrc %0, %1" : "=r"(ret) : "i"(shift));	\
	ret;	\
})

static __inline int ADD(int x, int y)
{
    int z = x+y;
#if defined(OVERFLOW_DEBUG)
    double fx, fy, diff;
    fx=x; fy = y;
    diff = (double)z - (fx+fy);
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
#endif

    return z;
}

static __inline int SUB(int x, int y)
{
    int z = x-y;
#if defined(OVERFLOW_DEBUG)
    double fx, fy, diff;
    fx=x; fy = y;
    diff = (double)z - (fx-fy);
    if(fabs(diff) > 1){
        g_ovflw_cnt[g_ovflw_indx]++;
    }
#endif
    return z;
}

#define MULSHIFT(x, y, shift)   ({  \
    register int ret;   \
    asm volatile ("l.mac %0,%1" : : "%r"(x), "r"(y));   \
    asm volatile ("l.macrc %0,%1" : "=r"(ret) : "i"(shift));    \
    ret;    \
})

#define MADDSHIFT(x1, y1, x2, y2, shift)    ({  \
    register int ret;   \
    asm volatile ("l.mac %0,%1" : : "%r"(x1), "r"(y1)); \
    asm volatile ("l.mac %0,%1" : : "%r"(x2), "r"(y2)); \
    asm volatile ("l.macrc %0,%1" : "=r"(ret) : "i"(shift));    \
    ret;    \
})

#define MSUBSHIFT(x1, y1, x2, y2, shift)    ({  \
    register int ret;   \
    asm volatile ("l.mac %0,%1" : : "%r"(x1), "r"(y1)); \
    asm volatile ("l.msb %0,%1" : : "%r"(x2), "r"(y2)); \
    asm volatile ("l.macrc %0,%1" : "=r"(ret) : "i"(shift));    \
    ret;    \
})

static __inline int MULSHIFT32(int x, int y)
{
    asm volatile ("l.mac %0,%1" : : "%r"(x), "r"(y));
    asm volatile ("l.macrc %0,32" : "=r"(y));
    return y;
}

static __inline int FASTABS(int x)
{
    int sign;

    sign = x >> (sizeof(int) * 8 - 1);
    x ^= sign;
    x -= sign;

    return x;
}

static __inline int CLZ(int x)
{
    int numZeros;

    if (!x)
        return (sizeof(int) * 8);

    numZeros = 0;
    while (!(x & 0x80000000)) {
        numZeros++;
        x <<= 1;
    }

    return numZeros;
}

#else

#error Unsupported platform

#endif  /* platforms */

#if defined(OVERFLOW_DEBUG)
    #include <math.h>
    enum {OVFLW_STAGE_EQ, OVFLW_STAGE_ST, OVFLW_STAGE_AA, OVFLW_STAGE_IMDCT,
            OVFLW_STAGE_WINOVLP, OVFLW_STAGE_DCTII, OVFLW_STAGE_SYNTH, OVFLW_STAGE};
    #define OVFLW_MAXSHIFT  36
    extern int g_ovflw_indx;    // default to avoid calling MULSHIFT(...) with uninitialized g_ovflindx
    extern int g_ovflw_cnt[OVFLW_STAGE];
    extern double g_ovflw_shift_coef[OVFLW_MAXSHIFT];

    static __inline void setoverflowindex(int x){
        g_ovflw_indx = x;
    }
    #define SETOVFLN(x) setoverflowindex(x)
#else
    #define SETOVFLN(x)
#endif

#endif  /* __CODER_H__ */
