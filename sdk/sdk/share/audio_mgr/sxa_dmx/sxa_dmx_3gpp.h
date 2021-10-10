//*****************************************************************************
// Name: sxa_dmx_3gpp.h
//
// Description:
//     Header File for sxa_dmx_3gpp.c
//
// Copyright (c) 2008 ITE Technology Corp. All Rights Reserved.
//*****************************************************************************

#ifndef _SXA_DMX_3GPP_H_
#define _SXA_DMX_3GPP_H_

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "sxa_dmx.h"
#include "sxa_dmx_3gpp_stbl_entry.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MAX_TRAK_COUNT          (8)
#define STSZ_ENTRY_BUFFER_SIZE  (1024)
#define STCO_ENTRY_BUFFER_SIZE  (1028)
#define STSC_ENTRY_BUFFER_SIZE  (500)
#define STTS_ENTRY_BUFFER_SIZE  (504)
#if (SUPPORT_NEW_PCM_INFO)
    #define MAX_SLICE_IN_CHUNK  (100)
#endif

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct
{
    unsigned long   bufa;   // [dvyu] current bit data
    unsigned long   bufb;   // [dvyu] the bit data of the byte just next bufa
    unsigned long   buf;
    unsigned long   pos;    // [dvyu] current bit position, in bits, always less than 32
    unsigned long   *tail;  // [dvyu] tail address pointer
    unsigned long   *start; // [dvyu] start address pointer
    unsigned long   length; // [dvyu] in bytes
} BIT_STREAM;

/*
                                Box structure
    ---------------------------------------------------------------------------
    ftyp
    moov
        mvhd
        trak
            tkhd
            tref
            edts
                elst
            mdia
                mdhd
                hdlr
                minf
                    vmhd
                    smhd
                    hmhd
                    dinf
                        dref
                    stbl
                        stsd
                        stts
                        ctts
                        stsc
                        stsz
                        stz2
                        stco
                        stss
                        stsh
                        padb
                        stdp
        mvex
            trex
    moof
        mfhd
        traf
            tfhd
            trun
    mdat
    free
    skip
        udta
    ---------------------------------------------------------------------------
 */

typedef struct MMP_3GPP_DECODE_H263SPEC_TAG
{
    unsigned char   Level;
    unsigned char   Profile;
} MMP_3GPP_DECODE_H263SPEC;

typedef struct MMP_3GPP_DECODE_H263ENTRY_TAG
{
    unsigned long               dwEntryType;
    unsigned short              wDataRefIdx;
    unsigned short              wWidth;
    unsigned short              wHeight;
    MMP_3GPP_DECODE_H263SPEC    h263spec;
} MMP_3GPP_DECODE_H263ENTRY;

typedef struct MMP_3GPP_DECODE_JPEGENTRY_TAG
{
    unsigned long   dwEntryType;
    unsigned short  wDataRefIdx;
    unsigned short  wWidth;
    unsigned short  wHeight;
} MMP_3GPP_DECODE_JPEGENTRY;

typedef struct MMP_3GPP_DECODE_MP4AESDBOX_TAG
{
    unsigned char   audioObjectType;
    unsigned char   samplingFrequencyIndex;
    unsigned char   channelConfiguration;
} MMP_3GPP_DECODE_MP4AESDBOX;

typedef struct MMP_3GPP_DECODE_MP4AENTRY_TAG
{
    unsigned long               dwEntryType;
    unsigned short              wDataRefIdx;
    unsigned short              wTimescale;
    MMP_3GPP_DECODE_MP4AESDBOX  esdBox;
} MMP_3GPP_DECODE_MP4AENTRY;

typedef struct MMP_3GPP_DECODE_MP4VESDBOX_TAG
{
    SXA_DMXVOLINFO_T volInfo;
} MMP_3GPP_DECODE_MP4VESDBOX;

typedef struct MMP_3GPP_DECODE_MP4VENTRY_TAG
{
    unsigned long               dwEntryType;
    unsigned short              wDataRefIdx;
    unsigned short              wWidth;
    unsigned short              wHeight;
    MMP_3GPP_DECODE_MP4VESDBOX  esdBox;
} MMP_3GPP_DECODE_MP4VENTRY;

typedef struct MMP_3GPP_DECODE_AMRSPEC_TAG
{
    unsigned short  ModeSet;
    unsigned char   ModeChangePeriod;
    unsigned char   FramesPerSample;
} MMP_3GPP_DECODE_AMRSPEC;

typedef struct MMP_3GPP_DECODE_AMRENTRY_TAG
{
    unsigned long           dwEntryType;
    unsigned short          wDataRefIdx;
    unsigned short          wTimeScale;
    MMP_3GPP_DECODE_AMRSPEC amrspec;
} MMP_3GPP_DECODE_AMRENTRY;

typedef struct MMP_3GPP_DECODE_RAWENTRY_TAG
{
    unsigned long   dwEntryType;
    unsigned short  wDataRefIdx;
    unsigned short  wChannelNum;
    unsigned short  wSampleSize;
    unsigned long   dwSampleRate;
} MMP_3GPP_DECODE_RAWENTRY;

typedef struct MMP_3GPP_DECODE_ULAWENTRY_TAG
{
    unsigned long   dwEntryType;
    unsigned short  wDataRefIdx;
    unsigned short  wChannelNum;
    unsigned short  wSampleSize;
    unsigned long   dwSampleRate;
} MMP_3GPP_DECODE_ULAWENTRY;

typedef struct MMP_3GPP_DECODE_SOWTENTRY_TAG
{
    unsigned long   dwEntryType;
    unsigned short  wDataRefIdx;
    unsigned short  wChannelNum;
    unsigned short  wSampleSize;
    unsigned long   dwSampleRate;
} MMP_3GPP_DECODE_SOWTENTRY;

typedef struct MMP_3GPP_DECODE_TWOSENTRY_TAG
{
    unsigned long   dwEntryType;
    unsigned short  wDataRefIdx;
    unsigned short  wChannelNum;
    unsigned short  wSampleSize;
    unsigned long   dwSampleRate;
} MMP_3GPP_DECODE_TWOSENTRY;

typedef struct MMP_3GPP_DECODE_STSD_TAG
{
    unsigned long               size;
    unsigned long               dwEntryType;
    unsigned long               dwEntryCount;
    MMP_3GPP_DECODE_H263ENTRY   h263Entry;
    MMP_3GPP_DECODE_MP4VENTRY   mp4vEntry;
    MMP_3GPP_DECODE_AMRENTRY    amrEntry;
    MMP_3GPP_DECODE_MP4AENTRY   mp4aEntry;
    MMP_3GPP_DECODE_JPEGENTRY   jpegEntry;
    MMP_3GPP_DECODE_RAWENTRY    rawEntry;
    MMP_3GPP_DECODE_ULAWENTRY   ulawEntry;
    MMP_3GPP_DECODE_SOWTENTRY   sowtEntry;
    MMP_3GPP_DECODE_TWOSENTRY   twosEntry;
} MMP_3GPP_DECODE_STSD;

typedef struct MMP_3GPP_DECODE_MVHD_TAG
{
    unsigned long   dwCreateTime;
    unsigned long   dwModTime;
    unsigned long   dwTimeScale;
    unsigned long   dwDuration;
    unsigned long   dwMatrix[3][3];
    unsigned long   dwNextTrackID;
} MMP_3GPP_DECODE_MVHD;

typedef struct MMP_3GPP_DECODE_TKHD_TAG
{
    unsigned long   dwCreateTime;
    unsigned long   dwModTime;
    unsigned long   dwTrackID;
    unsigned long   dwDuration;
    unsigned short  wLayer;
    unsigned short  wVolume;
    unsigned long   dwMatrix[3][3];
    unsigned long   dwWidth;
    unsigned long   dwHeight;
} MMP_3GPP_DECODE_TKHD;

typedef struct MMP_3GPP_DECODE_MDHD_TAG
{
    unsigned long   dwCreateTime;
    unsigned long   dwModTime;
    unsigned long   dwTimeScale;
    unsigned long   dwDuration;
} MMP_3GPP_DECODE_MDHD;

typedef struct MMP_3GPP_DECODE_HDLR_TAG
{
    unsigned long dwHandleType;
} MMP_3GPP_DECODE_HDLR;

typedef struct MMP_3GPP_DECODE_STSS_TAG
{
    unsigned long   size;
    unsigned long   dwEntryCount;
    unsigned long   *pSampleNumber; // [dvyu] this means the sample index
} MMP_3GPP_DECODE_STSS;

typedef struct MMP_3GPP_DECODE_STCO_TAG
{
    unsigned long           size;
    unsigned long           dwEntryCount;
    GPP_DECODE_TABLE_STREAM stcoEntry;
} MMP_3GPP_DECODE_STCO;

typedef struct MMP_3GPP_DECODE_STSZ_TAG
{
    unsigned long           size;
    unsigned long           dwEntryCount;
    unsigned long           dwSampleSize;
    unsigned long           dwSampleCount;
    unsigned long           dwOriginalSize; // for uLaw audio, record the original sample size
    GPP_DECODE_TABLE_STREAM stszEntry;
} MMP_3GPP_DECODE_STSZ;

typedef struct MMP_3GPP_DECODE_STSC_TAG
{
    unsigned long           size;
    unsigned long           dwEntryCount;
    GPP_DECODE_TABLE_STREAM stscEntry;
} MMP_3GPP_DECODE_STSC;

typedef struct MMP_3GPP_DECODE_STTS_TAG
{
    unsigned long           size;
    unsigned long           dwEntryCount;
    GPP_DECODE_TABLE_STREAM sttsEntry;
} MMP_3GPP_DECODE_STTS;

typedef struct MMP_3GPP_DECODE_STBL_TAG
{
    unsigned long           size;
    MMP_3GPP_DECODE_STSD    stsd;
    MMP_3GPP_DECODE_STTS    stts;
    MMP_3GPP_DECODE_STSC    stsc;
    MMP_3GPP_DECODE_STSZ    stsz;
    MMP_3GPP_DECODE_STCO    stco;
    MMP_3GPP_DECODE_STSS    stss;
} MMP_3GPP_DECODE_STBL;

typedef struct MMP_3GPP_DECODE_MINF_TAG
{
    unsigned long           size;
    MMP_3GPP_DECODE_STBL    stbl;
} MMP_3GPP_DECODE_MINF;

typedef struct MMP_3GPP_DECODE_MDIA_TAG
{
    unsigned long           size;
    MMP_3GPP_DECODE_MDHD    mdhd;
    MMP_3GPP_DECODE_HDLR    hdlr;
    MMP_3GPP_DECODE_MINF    minf;
} MMP_3GPP_DECODE_MDIA;

#if (SUPPORT_NEW_PCM_INFO)
typedef struct tagMMP_3GPP_DECODE_SLICEINCHUNK_T
{
    unsigned int    nSliceIdx;          // slice index
    unsigned int    nSliceSize;         // slice size
} MMP_3GPP_DECODE_SLICEINCHUNK_T;
#endif

typedef struct MMP_3GPP_DECODE_TRAK_TAG
{
    unsigned long           size;
    MMP_3GPP_DECODE_TKHD    tkhd;
    MMP_3GPP_DECODE_MDIA    mdia;

    // Calvin@20060113
    unsigned int            stcoIndex;          //= 0;
    unsigned int            stscIndex;          //= 0;
    unsigned int            sttsEntryIndex;     //= 0;
    unsigned long           sttsSampleCount;    //= pStts->pSampleCount[0];
    unsigned long           sampleStart;        //= 0;
    unsigned long           firstChunk;         //= 0;
    unsigned long           samplePerChunk;     //= 0;
    unsigned long           sampleIndex;        //= pInfo->dwSampleNumber-1;
    unsigned int            i;                  //= 0;
    unsigned long           offset;             //= 0;
    // ~Calvin@20060113
    signed long             m_dwLstSeekIdx;     // 1'idx    // [dvyu] 2008.1.23 Add, ¥Ø«e³Ì«á Seek ¨ìªº sample index
    // Kevin@2006.1.19
    signed long             sttsDeltaCount;     // Record the remain of delta count
    signed long             sttsDeltaIdx;       // Record the position from the table of the delta per track
    // ~Kevin@2006.1.19

    // Kevin@2006.1.25  Adding Total Time
    signed long sampleDeltaBase;
    signed long sampleDeltaAcc;
    // ~Kevin@2006.1.25

    // Kevin@2006.2.27
    // global
    unsigned long                   m_dwLstSampleIdx; // 1'idx    // [dvyu] ¥Ø«e³Ì«á³B²z¨ìªº sample index
    // delta
    signed long                     m_dwTimeStampBase;
    signed long                     m_dwTimeStampRemain;
    unsigned long                   m_dwLstSTTSIdx;         // [dvyu] ¥Ø«e³B²z¨ìªº STTS entry index
    signed long                     m_dwLstSTTSCount;       // [dvyu] ¥Ø«eªº STTS entry ¤w³B²zªº sample ¼Æ¥Ø
    // chunk
    signed long                     m_dwLstSTSCIdx;         // [dvyu] ¥Ø«e³B²z¨ìªº STSC entry index
    unsigned long                   m_dwLstSTSCChunkIdx;    // [dvyu] ¥Ø«e³B²z¨ìªº chunk index
    signed long                     m_dwLstSTSCCount;       // [dvyu] ¥Ø«eªº STSC entry ¤w³B²zªº sample ¼Æ¥Ø
    unsigned long                   m_dwLstSTSCOffset;      // [dvyu] ¥Ø«e³B²z¨ìªº sample offset
    // offset
    unsigned long                   m_dwLstSTCOIdx;
    // size
    unsigned long                   m_dwLstSTSZIdx;
    // ~Kevin@2006.2.27
#if (SUPPORT_NEW_PCM_INFO)
    int                             nMaxBytePerSlice;                   // Max byte per slice for audio stream
    int                             nSliceCount;                        // (for Audio trak ÅÞ¿è¤Wªº) ‧í§@ Aduio Sample Count
    int                             nSliceCntInChunk;                   // Slice count in current chunk
    MMP_3GPP_DECODE_SLICEINCHUNK_T  tSliceInChunk[MAX_SLICE_IN_CHUNK];  // Slice info in current Chunk
    // for getting sample index from slice index
    int                             nS1SliceIdx;                        // 1'idx, (for Audio trak ÅÞ¿è¤Wªº) ¥Ø«e³B²z¨ìªº slice Index
    int                             nS1SmpIdx;                          // 1'idx, (for Audio trak slice) ¥Ø«e slice ³B²z¨ìªº sample Index
    int                             nS1STSCIdx;                         // (for Audio trak slice) ¥Ø«e³B²z¨ìªº STSC entry index
    int                             nS1ChunkIdx;                        // 1'idx, (for Audio trak slice) ¥Ø«e³B²z¨ìªº Chunk index
    int                             nS1SmpInChunk;                      // (for Audio trak slice) ¥Ø«e³B²z¨ìªº ¦b Chunk ¤¤ªº²Ä´X­Ó sample
    // for getting slice index from sample index
    int                             nS2SliceIdx;                        // 1'idx, (for Audio trak ÅÞ¿è¤Wªº) ¥Ø«e³B²z¨ìªº slice Index
    int                             nS2SmpIdx;                          // 1'idx, (for Audio trak slice) ¥Ø«e slice ³B²z¨ìªº sample Index
    int                             nS2STSCIdx;                         // (for Audio trak slice) ¥Ø«e³B²z¨ìªº STSC entry index
    int                             nS2ChunkIdx;                        // 1'idx, (for Audio trak slice) ¥Ø«e³B²z¨ìªº Chunk index
    int                             nS2SmpInChunk;                      // (for Audio trak slice) ¥Ø«e³B²z¨ìªº ¦b Chunk ¤¤ªº²Ä´X­Ó sample
    int                             nS2SliceSize;                       // ¥Ø«e slice ²Ö¿nªº byte ¼Æ
#endif
} MMP_3GPP_DECODE_TRAK;

typedef struct MMP_3GPP_DECODE_UDTA_TAG
{
    unsigned char   *pchCnam;
    unsigned char   *pchCart;
    unsigned char   *pchCalb;
} MMP_3GPP_DECODE_UDTA;

typedef struct MMP_3GPP_DECODE_MOOV_TAG
{
    unsigned long           size;
    MMP_3GPP_DECODE_MVHD    mvhd;
    MMP_3GPP_DECODE_UDTA    udta;
    MMP_3GPP_DECODE_TRAK    trak[MAX_TRAK_COUNT];
} MMP_3GPP_DECODE_MOOV;

typedef struct MMP_3GPP_DECODE_3GPPDATA_TAG
{
    unsigned int            dwFileFormat;       // [dvyu] Common field of All Decoders to Keep the file format infomation
    PAL_FILE                *gppFile;
    unsigned long           dwTrakCount;        // [dvyu] total track number of this movie
    unsigned long           dwFileSize;
    unsigned long           dwVideoStreamSize;
    unsigned long           dwAudioStreamSize;
    unsigned long           dwErrorFlags;

    unsigned long           pMoovSize;
    unsigned long           pMdatStart;             // [dvyu] the file offset of MDAT box
    unsigned long           pMdatSize;
    unsigned char           *pMoovStream;           // [dvyu] the stream data of MOOV box
    MMP_3GPP_DECODE_MOOV    moov;
    MMP_3GPP_DECODE_TRAK    *pFirstVideoTrak;
    MMP_3GPP_DECODE_TRAK    *pFirstAudioTrak;
} MMP_3GPP_DECODE_3GPPDATA;

//=============================================================================
//                              Function Declaration
//=============================================================================
MMP_3GPP_DECODE_3GPPDATA *sxaDmx3GPP_CreateDec3gppData(void);
GPP_RESULT sxaDmx3GPP_ParseMoovBox(MMP_3GPP_DECODE_3GPPDATA *pGppData, SXA_DMXOPENEXPARAM_T      *pParam, int nInType);
unsigned int  sxaDmx3GPP_GetVideoSampleCount(MMP_3GPP_DECODE_3GPPDATA *pGppData);
unsigned int  sxaDmx3GPP_GetAudioSampleCount(MMP_3GPP_DECODE_3GPPDATA *pGppData);
unsigned int  sxaDmx3GPP_GetKeySampleCount(MMP_3GPP_DECODE_3GPPDATA *pGppData);
unsigned int *sxaDmx3GPP_GetKeySampleIndex(MMP_3GPP_DECODE_3GPPDATA *pGppData);
void       sxaDmx3GPP_Terminate(MMP_3GPP_DECODE_3GPPDATA *pGppData);
GPP_RESULT sxaDmx3GPP_GetFirstVideoTrack(MMP_3GPP_DECODE_3GPPDATA *pGppData, unsigned int *index);
GPP_RESULT sxaDmx3GPP_GetFirstAudioTrack(MMP_3GPP_DECODE_3GPPDATA *pGppData, unsigned int *index);
void       sxaDmx3GPP_BitstreamInit(BIT_STREAM *bs, unsigned long *bitstream, unsigned long dwStreamSize);
GPP_RESULT sxaDmx3GPP_GetVOLParameter(SXA_DMXVOLINFO_T *pVOLInfo, BIT_STREAM *bs);

SXA_DMXECODE_E sxaDmx3GPP_GetVOPParameter(
    SXA_DMXVOPINFO_T    *pVOPInfo,
    SXA_DMXDECODESIZE_T *pDecodeSize,
    SXA_DMXVOLINFO_T    *pVOLInfo,
    BIT_STREAM          *bs);

SXA_DMXECODE_E sxaDmx3GPP_GetShortHeaderParameter(
    SXA_DMXSHORTHEADERINFO_T    *pSHeaderInfo,
    SXA_DMXDECODESIZE_T         *pDecodeSize,
    BIT_STREAM                  *bs);

SXA_DMXECODE_E sxaDmx3GPP_CreateADTSHeader(
    MMP_3GPP_DECODE_3GPPDATA    *pGppData,
    unsigned int                aacFrameLength,
    unsigned char               *pDestBufAddr);

GPP_RESULT sxaDmx3GPP_SeekSampleInfoFromIdx(
    MMP_3GPP_DECODE_3GPPDATA    *hDecGppData,
    unsigned long               uiVidSample,
    SXA_DMXSAMPLEINFO_T         *pVidInfo,
    SXA_DMXSAMPLEINFO_T         *pAudInfo);

GPP_RESULT sxaDmx3GPP_SeekSampleInfoFromTime(
    MMP_3GPP_DECODE_3GPPDATA    *hDecGppData,
    unsigned long               uiTime,
    SXA_DMXSAMPLEINFO_T         *pVidInfo,
    SXA_DMXSAMPLEINFO_T         *pAudInfo);

SXA_DMXECODE_E sxaDmx3GPP_GetVideoSampleInfo(
    MMP_3GPP_DECODE_3GPPDATA    *hDecGppData,
    SXA_DMXSAMPLEINFO_T         *pInfo);

SXA_DMXECODE_E sxaDmx3GPP_GetAudioSampleInfo(
    MMP_3GPP_DECODE_3GPPDATA    *hDecGppData,
    SXA_DMXSAMPLEINFO_T         *pInfo);

GPP_RESULT sxaDmx3GPP_GetChunkInfo(
    MMP_3GPP_DECODE_TRAK    *pTrak,
    unsigned long           *pChunkCount,
    unsigned long           *pChunkIdx);

GPP_RESULT sxaDmx3GPP_IsConstSizeDelta(
    MMP_3GPP_DECODE_TRAK    *pTrak,
    MMP_BOOL                *pRet);

SXA_DMXVOLINFO_T *sxaDmx3GPP_GetVideoVolInfo(MMP_3GPP_DECODE_3GPPDATA *pGppData);
unsigned short sxaDmx3GPP_GetVideoWidth(MMP_3GPP_DECODE_3GPPDATA *pGppData);
unsigned short sxaDmx3GPP_GetVideoHeight(MMP_3GPP_DECODE_3GPPDATA *pGppData);
SXA_DMXVFMT_E sxaDmx3GPP_GetVideoCodecType(MMP_3GPP_DECODE_3GPPDATA *pGppData);
SXA_DMXAFMT_E sxaDmx3GPP_GetAudioCodecType(MMP_3GPP_DECODE_3GPPDATA *pGppData);
unsigned int  sxaDmx3GPP_GetTotalTime(MMP_3GPP_DECODE_3GPPDATA *pGppData);
unsigned int  sxaDmx3GPP_GetVideoTimeScale(MMP_3GPP_DECODE_3GPPDATA *pGppData);
unsigned int  sxaDmx3GPP_GetAudioTimeScale(MMP_3GPP_DECODE_3GPPDATA *pGppData);
unsigned int  sxaDmx3GPP_GetAudioFreq(MMP_3GPP_DECODE_3GPPDATA *pGppData);
unsigned int  sxaDmx3GPP_GetAudioChannel(MMP_3GPP_DECODE_3GPPDATA *pGppData);
unsigned int  sxaDmx3GPP_GetAudioSampleSize(MMP_3GPP_DECODE_3GPPDATA *pGppData);

GPP_RESULT
sxaDmx3GPP_GetUserData(
    MMP_3GPP_DECODE_3GPPDATA    *pGppData,
    SXA_DMXUSERDATA_T           *pUserData);

GPP_RESULT
sxaDmx3GPP_Get1stVideoFrameInfo(
    SXA_DMXOPENEXPARAM_T    *pParam,
    SXA_DMX1STFRAMEINFO_T   *pFrameInfo);

#ifdef __cplusplus
}
#endif

#endif