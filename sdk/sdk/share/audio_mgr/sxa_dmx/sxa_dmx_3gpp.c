
//*****************************************************************************
// Name: sxa_dmx_3gpp.c
//
// Description:
//     3GPP Decoder
//
// Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
//*****************************************************************************

//=============================================================================
//                              Include Files
//=============================================================================
#include "sxa_dmx_3gpp.h"

#define WATCHDOG_ENABLE 0
#if (WATCHDOG_ENABLE)
#include "mmp_watchdog.h"
#endif

#define SXA_DATA_SIZE 30000

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Compile Option
//=============================================================================
#define OPTION_MM360

//=============================================================================
//                              Constant Definition
//=============================================================================
// Video Object layer
#define MPG_VOL_TYPE_SIMPLE                  1
#define MPG_VOL_AR_EXTPAR                    15
#define MPG_VOL_SHAPE_RECTANGULAR            0
#define MPG_VOL_SHAPE_BINARY_ONLY            2

// Video Object Plane
#define MPG_I_VOP                            0
#define MPG_P_VOP                            1
#define MPG_B_VOP                            2
#define MPG_S_VOP                            3

#if (WATCHDOG_ENABLE)
#define RESET_WATCHDOG                       3000
#endif

//=============================================================================
//                              Macro Definition
//=============================================================================
#define ABS(a)      (((a) > 0) ? (a) : -(a))

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
unsigned long gppFilePos;

static SXA_DMXOPENEXPARAM_T gppOpenExParam;
//SXA_DMXSAMPLEINFO_T* gtSample_info;

SXA_DMXSAMPLEINFO_T gtSample_info[SXA_DATA_SIZE];


//=============================================================================
//                              Private Function Declaration
//=============================================================================
static GPP_RESULT sxaDmx3GPP_GetJPEGSampleEntry(unsigned char *pReadPtr,unsigned long dwBoxSize,MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetRAWSampleEntry(unsigned char *pReadPtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetULAWSampleEntry(unsigned char *pReadPtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetSOWTSampleEntry(unsigned char *pReadPtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetTWOSSampleEntry(unsigned char *pReadPtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetH263SampleEntry(unsigned char *pReadPtr,unsigned long dwBoxSize,MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetMp4VisualSampleEntry(unsigned char *pReadPtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetMp4AudioSampleEntry(unsigned char *pReadPtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetAMRSampleEntry(unsigned char *pReadPtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetVideoSampleEntry(unsigned char *pReadPtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetAudioSampleEntry(unsigned char *pReadPtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetSampleDescription(unsigned char *pReadPtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetDecodingTime(PAL_FILE *pFilePtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetSampleToChunk(PAL_FILE *pFilePtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetSampleSize(PAL_FILE *pFilePtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetChunkOffset(PAL_FILE *pFilePtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetSyncSample(PAL_FILE *pFilePtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);   // [dvyu] 2007.11.22 Modified
static GPP_RESULT sxaDmx3GPP_GetHandlerReference(unsigned char *pReadPtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetMediaHeader(unsigned char *pReadPtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetTrackHeader(unsigned char *pReadPtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
static GPP_RESULT sxaDmx3GPP_GetMovieHeader(unsigned char *pReadPtr, unsigned long dwBoxSize, MMP_3GPP_DECODE_3GPPDATA* pGppData);
#if (SUPPORT_NEW_PCM_INFO)
static GPP_RESULT GetMaxBytePerSample(MMP_3GPP_DECODE_3GPPDATA* pGppData, unsigned long dwAudioType);
static signed int GetAudioSliceCount(MMP_3GPP_DECODE_3GPPDATA* hDecGppData);
static signed int GetSampleIdxFromSliceIdx(MMP_3GPP_DECODE_TRAK* pTrak, unsigned int nSliceIdx);
static signed int GetSliceIdxFromSampleIdx(MMP_3GPP_DECODE_TRAK* pTrak, unsigned int nSampleIdx);
#endif



#if (WATCHDOG_ENABLE)
static PAL_CLOCK_T    lastClock;   // for watch dog

static void CheckResetWatchDog(int line_num) // check if watch dog need to reset
{
    if (PalGetDuration(lastClock) > RESET_WATCHDOG)
    {
        printf("[DMX][3GPP] Line#%ld reset watch dog, duration = %ld\r\n", line_num, PalGetDuration(lastClock));
        mmpWatchDogRefreshTimer();
        lastClock = PalGetClock();
    }
}
#endif



//-----< BitstreamShowBits >-----
static __inline unsigned long
BitstreamShowBits(BIT_STREAM *bs, unsigned long bits)
{
    unsigned long ret_value;

    if ((bits + bs->pos) > 32)
    {
        unsigned long nbit = (bits + bs->pos) - 32;
        ret_value =  ((bs->bufa & (0xffffffff >> bs->pos)) << nbit) |
            (bs->bufb >> (32 - nbit));
    }
    else
    {
        ret_value = (bs->bufa & (0xffffffff >> bs->pos)) >> (32 - bs->pos - bits);
    }
    return ret_value;
}



//-----< BitstreamSkip >-----
static __inline void
BitstreamSkip(BIT_STREAM *bs, unsigned long bits)
{
    bs->pos += bits;

    if (bs->pos >= 32)
    {
        unsigned long tmp;

        bs->bufa = bs->bufb;
        tmp = *(bs->tail + 2);
        BS_WAP(tmp);
        bs->bufb = tmp;
        bs->tail++;
        bs->pos -= 32;
    }
}



//-----< BitstreamPos >-----
static __inline unsigned long
BitstreamPos(BIT_STREAM *bs)
{
    return (8 * ((unsigned long)bs->tail - (unsigned long)bs->start) + bs->pos);
}



//-----< BitstreamForward >-----
static __inline void
BitstreamForward(BIT_STREAM *bs, unsigned long bits)
{
    bs->pos += bits;

    if (bs->pos >= 32)
    {
        unsigned long b = bs->buf;

        BS_WAP(b);
        *bs->tail++ = b;
        bs->buf = 0;
        bs->pos -= 32;
    }
}



//-----< BitstreamByteAlign >-----
static __inline void
BitstreamByteAlign(BIT_STREAM *bs)
{
    unsigned long remainder = bs->pos % 8;

    if (remainder)
    {
        BitstreamSkip(bs, (8 - remainder));
    }
}



//-----< FindNextStartCode >-----
static __inline void
FindNextStartCode(BIT_STREAM *bs)
{
    BitstreamByteAlign(bs);

    while ((BitstreamPos(bs) >> 3) < bs->length)
    {
        if (BitstreamShowBits(bs, 24) == 0x000001)
        {
            return;
        }
        BitstreamSkip(bs, 8);
    }
}



//-----< FindNextShortVideoStartCode >-----
static __inline void
FindNextShortVideoStartCode(BIT_STREAM *bs)
{
    unsigned long start_code = 0;

    BitstreamByteAlign(bs);
    while ((BitstreamPos(bs) >> 3) < bs->length)
    {
        start_code = BitstreamShowBits(bs, 32);
        if ((start_code & ~0x3fff) == 0x00008000)
        {
            break;
        }

        BitstreamSkip(bs, 8);
    }
}



//-----< BitstreamGetBits >-----
static __inline unsigned long
BitstreamGetBits(BIT_STREAM *bs, unsigned long n)
{
    unsigned long ret = BitstreamShowBits(bs, n);

    BitstreamSkip(bs, n);
    return ret;
}



//-----< BitstreamGetBit >-----
static __inline unsigned long
BitstreamGetBit(BIT_STREAM *bs)
{
    return BitstreamGetBits(bs, 1);
}



//-----< BitstreamPutBits >-----
static __inline void
BitstreamPutBits(BIT_STREAM *bs,
                 unsigned long value,
                 unsigned long size)
{
    unsigned long shift = 32 - bs->pos - size;

    if (shift <= 32)
    {
        bs->buf |= value << shift;
        BitstreamForward(bs, size);
    }
    else
    {
        unsigned long remainder;

        shift = size - (32 - bs->pos);
        bs->buf |= value >> shift;
        BitstreamForward(bs, size - shift);
        remainder = shift;

        shift = 32 - shift;

        bs->buf |= value << shift;
        BitstreamForward(bs, remainder);
    }
}



//-----< BitstreamPutBit >-----
static __inline void
BitstreamPutBit(BIT_STREAM *bs, unsigned long bit)
{
    if (bit)
    {
        bs->buf |= (0x80000000 >> bs->pos);
    }
    BitstreamForward(bs, 1);
}



//-----< READ_MARKER >-----
static __inline void
READ_MARKER(BIT_STREAM *bs)
{
    BitstreamSkip(bs, 1);
}



//-----< getAudioObjectType >-----
static __inline unsigned long getAudioObjectType(BIT_STREAM *bs)
{
    unsigned long  dwObjectType;

    dwObjectType = BitstreamGetBits(bs, 5);
    if (dwObjectType == 31)
    {
        dwObjectType = 32 + BitstreamGetBits(bs, 6);
    }

    return (dwObjectType);
}



//=============================================================================
//  GPP_RESULT getGASpecificConfig
//
// Description:
//      Get GASpecificConfig
//      Temp Solution, not completed
//
// Parameters:
//      bs : ESD bitstream
//      pESDBox : Structure of ES Description
//
//=============================================================================
static __inline void
getGASpecificConfig(
                    BIT_STREAM      *bs,
                    MMP_3GPP_DECODE_MP4AESDBOX  *pESDBox)
{
    unsigned char   frameLengthFlag;
    unsigned char   dependsOnCoreCoder;
    unsigned short  coreCoderDelay;
    unsigned char   extensionFlag;

    unsigned char   channel;

    //frameLengthFlag
    frameLengthFlag = (unsigned char)BitstreamGetBits(bs ,1);

    dependsOnCoreCoder = (unsigned char)BitstreamGetBits(bs, 1);
    if (dependsOnCoreCoder)
    {
        //coreCoderDelay
        coreCoderDelay = (unsigned short)BitstreamGetBits(bs ,14);
    }

    // extensionFlag
    extensionFlag = (unsigned char)BitstreamGetBits(bs ,1);
    if (pESDBox->channelConfiguration == 0)
    {
        //program_config_element()
        BitstreamGetBits(bs ,4);
        pESDBox->audioObjectType = (unsigned char)BitstreamGetBits(bs, 2);  // 1/2
        pESDBox->samplingFrequencyIndex = (unsigned char)BitstreamGetBits(bs, 4);   // 4
        //pESDBox->channelConfiguration = (unsigned char)BitstreamGetBits(bs, 2);   // 2

        //printf("ObjectType %d\n", pESDBox->audioObjectType);
        //printf("FrequencyIdx %d\n", pESDBox->samplingFrequencyIndex);

        channel = (unsigned char)BitstreamGetBits(bs, 4);
        //printf("front channel %d\n", channel);

        channel = (unsigned char)BitstreamGetBits(bs, 4);
        //printf("side channel %d\n", channel);

        channel = (unsigned char)BitstreamGetBits(bs, 4);
        //printf("back channel %d\n", channel);

        channel = (unsigned char)BitstreamGetBits(bs, 2);
        //printf("lfe channel %d\n", channel);

        channel = (unsigned char)BitstreamGetBits(bs, 3);
        //printf("assoc channel %d\n", channel);

        channel = (unsigned char)BitstreamGetBits(bs, 4);
        //printf("valid channel %d\n", channel);
    }

    // Not Completed Solution   S034
}



#if (SUPPORT_NEW_PCM_INFO)
static GPP_RESULT
GetMaxBytePerSample(
                    MMP_3GPP_DECODE_3GPPDATA* pGppData,
                    unsigned long             dwAudioType)
{
    unsigned int dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_RAWENTRY*  rawEntry;
    MMP_3GPP_DECODE_ULAWENTRY* ulawEntry;
    MMP_3GPP_DECODE_SOWTENTRY* sowtEntry;
    MMP_3GPP_DECODE_TWOSENTRY* twosEntry;

    unsigned int channel, sample_size, sample_rate;
    float bytesPerSample;

    // default is Play25ms
#define Play25ms
#if defined(Play100ms)
    int B8000 = 800;
    int B16000 = 1600;
    int B32000 = 3200;
    int B11024 = 1104;
    int B22050 = 2208;
    int B44100 = 4412;
    float BOTHER = 0.1;
#elif defined(Play50ms)
    int B8000 = 400;
    int B16000 = 800;
    int B32000 = 1600;
    int B11024 = 552;
    int B22050 = 1104;
    int B44100 = 2208;
    float BOTHER = 0.05;
#else // Play25ms
    int B8000 = 200;
    int B16000 = 400;
    int B32000 = 800;
    int B11024 = 276;
    int B22050 = 552;
    int B44100 = 1104;
    float BOTHER = 0.025;
#endif

    switch (dwAudioType)
    {
    case RAW_SOUND_TYPE:
        rawEntry = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd.rawEntry);
        channel     = rawEntry->wChannelNum;
        sample_size = rawEntry->wSampleSize;
        sample_rate = rawEntry->dwSampleRate / 65536;
        break;
    case ULAW_SOUND_TYPE:
        ulawEntry = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd.ulawEntry);
        channel     = ulawEntry ->wChannelNum;
        sample_size = ulawEntry ->wSampleSize;
        sample_rate = ulawEntry ->dwSampleRate / 65536;
        break;
    case SOWT_SOUND_TYPE:
        sowtEntry = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd.sowtEntry);
        channel     = sowtEntry->wChannelNum;
        sample_size = sowtEntry->wSampleSize;
        sample_rate = sowtEntry->dwSampleRate / 65536;
        break;
    case TWOS_SOUND_TYPE:
        twosEntry = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd.twosEntry);
        channel     = twosEntry->wChannelNum;
        sample_size = twosEntry->wSampleSize;
        sample_rate = twosEntry->dwSampleRate / 65536;
        break;
    default:
        pGppData->moov.trak[dwTrakIdx].nMaxBytePerSlice = 256 * 256 * 256; // a large value
        return GPP_SUCCESS;
        break;
    }

    bytesPerSample = (sample_size / 8.0) * channel;

    switch (sample_rate)
    {
    case  8000:
        pGppData->moov.trak[dwTrakIdx].nMaxBytePerSlice = B8000 * bytesPerSample;
        break;
    case 16000:
        pGppData->moov.trak[dwTrakIdx].nMaxBytePerSlice = B16000 * bytesPerSample;
        break;
    case 32000:
        pGppData->moov.trak[dwTrakIdx].nMaxBytePerSlice = B32000 * bytesPerSample;
        break;
    case 11024:
        pGppData->moov.trak[dwTrakIdx].nMaxBytePerSlice = B11024 * bytesPerSample;
        break;
    case 22050:
        pGppData->moov.trak[dwTrakIdx].nMaxBytePerSlice = B22050 * bytesPerSample;
        break;
    case 44100:
        pGppData->moov.trak[dwTrakIdx].nMaxBytePerSlice = B44100 * bytesPerSample;
        break;
    default:
		pGppData->moov.trak[dwTrakIdx].nMaxBytePerSlice = (int)(sample_rate * BOTHER / 4 + 0.9999) * 4 * bytesPerSample;
        break;
    }

    return GPP_SUCCESS;
}



static signed int
GetAudioSliceCount(
                   MMP_3GPP_DECODE_3GPPDATA* hDecGppData)
{
    GPP_RESULT ret;

    MMP_3GPP_DECODE_TRAK	*pTrak = hDecGppData->pFirstAudioTrak;
    MMP_3GPP_DECODE_STSZ    *pStsz;    // size
    MMP_3GPP_DECODE_STSC    *pStsc;    // samples per chunk
    MMP_3GPP_DECODE_STCO    *pStco;    // chunk offset

    unsigned int    nSliceCount = 0;
    unsigned int    nCurSample;
    unsigned int    nMaxSTSCIdx, nCurSTSCIdx;
    unsigned long   nCurChunkIdx, nNextSTSCChunkIdx;
    unsigned long   nSamplePerChunk, nCurSampleInChunk;
    unsigned long   nSize, nSliceSize;

    if (pTrak == NULL)
    {
        return 0;
    }

    pStsz = &(pTrak->mdia.minf.stbl.stsz);
    pStsc = &(pTrak->mdia.minf.stbl.stsc);
    pStco = &(pTrak->mdia.minf.stbl.stco);

    nMaxSTSCIdx = pStsc->dwEntryCount - 1;
    nCurSTSCIdx = 0;
    nCurSample  = 1;	// 1'idx

#if (WATCHDOG_ENABLE)
    CheckResetWatchDog(__LINE__);
#endif

    // Iterate each STSC entry
    while (nCurSTSCIdx <= nMaxSTSCIdx)
    {
#if (WATCHDOG_ENABLE)
        CheckResetWatchDog(__LINE__);
#endif

        // Get the 'sample per chunk' of current STSC entry
        ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, nCurSTSCIdx, 1, &nSamplePerChunk);
        if (ret != GPP_SUCCESS)
        {
            return -1;
        }

        // Get the 'first chunk index' of current STSC entry
        ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, nCurSTSCIdx, 0, &nCurChunkIdx);
        if (ret != GPP_SUCCESS)
        {
            return -1;
        }

        // Get the 'first chunk index' of next STSC entry
        if (nCurSTSCIdx + 1 >= pStsc->dwEntryCount)
        {
            nNextSTSCChunkIdx = pStco->dwEntryCount;
        }
        else
        {
            ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, nCurSTSCIdx + 1, 0, &nNextSTSCChunkIdx);
            if (ret != GPP_SUCCESS)
            {
                return -1;
            }
        }
#if (WATCHDOG_ENABLE)
        CheckResetWatchDog(__LINE__);
#endif

        // Iterate each chunk in this STSC entry
        while (nCurChunkIdx <= nNextSTSCChunkIdx)
        {
            // The condition (nCurChunkIdx == nNextSTSCChunkIdx) is only for the last chunk of audio stream.
            // So if nCurChunkIdx is not equal to the last chunk of audio stream, its iteration should be skipped.
            if ((nCurChunkIdx == nNextSTSCChunkIdx) &&
                !((nCurSTSCIdx == nMaxSTSCIdx) && (nNextSTSCChunkIdx == pStco->dwEntryCount)))
            {
                break;
            }

            nCurSampleInChunk = 0;
            nSliceSize = 0;
#if (WATCHDOG_ENABLE)
            CheckResetWatchDog(__LINE__);
#endif

            // Iterate each sample in this chunk
            while (nCurSampleInChunk < nSamplePerChunk)
            {
                // Get the size of current sample
                if (pStsz->dwSampleSize == 0)
                {
                    ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, nCurSample-1, 0, &nSize);
                    if (ret != GPP_SUCCESS)
                    {
                        return ret;
                    }
                }
                else
                {
                    nSize = pStsz->dwSampleSize;
                }
                nSliceSize += nSize;

                // If nSliceSize reach the nMaxBytePerSlice, increase nSliceCount
                if (nSliceSize > pTrak->nMaxBytePerSlice)
                {
                    nSliceCount++;
                    nSliceSize = 0;
                }
                else
                {
                    nCurSample++;
                    nCurSampleInChunk++;
                }
            }

            // Last slice in this chunk
            if (nSliceSize > 0)
            {
                nSliceCount++;
                nSliceSize = 0;
            }

            // Move to next chunk
            nCurChunkIdx++;
        }

        // Move to next STSC entry
        nCurSTSCIdx++;
    }

    return nSliceCount;
}



static signed int
GetSampleIdxFromSliceIdx(
                         MMP_3GPP_DECODE_TRAK*     pTrak,
                         unsigned int              nSliceIdx)
{
    GPP_RESULT ret;

    MMP_3GPP_DECODE_STSZ    *pStsz = &(pTrak->mdia.minf.stbl.stsz);    // size
    MMP_3GPP_DECODE_STSC    *pStsc = &(pTrak->mdia.minf.stbl.stsc);    // samples per chunk
    MMP_3GPP_DECODE_STCO    *pStco = &(pTrak->mdia.minf.stbl.stco);    // chunk offset

    int             nMaxSTSCIdx, nSliceSize;
    unsigned long   nThisSTSCChunkIdx, nNextSTSCChunkIdx;  // 1'idx
    unsigned long   nSamplePerChunk, nSize;
    // For Backward
    int          nCurSmpIdx;            // current sample index
    int          nSmpInChunk;           // sample in chunk
    int          nLastSliceSmpIdx;      // sample index of last slice
    int          nLastCurSmpIdx;        // sample in chunk of last slice

    if (pTrak == NULL)
    {
        return 0;
    }

    nMaxSTSCIdx = pStsc->dwEntryCount - 1;
    nSliceSize  = 0;

    if (nSliceIdx == 0)
    {
        return 0;
    }

    // Initialize
    if (pTrak->nS1SliceIdx == 0)
    {
        pTrak->nS1SliceIdx    = 1;   // 1'idx
        pTrak->nS1SmpIdx      = 1;   // 1'idx
        pTrak->nS1STSCIdx     = 0;
        pTrak->nS1ChunkIdx    = 1;   // 1'idx
        pTrak->nS1SmpInChunk  = 0;
    }

    // Match
    if (pTrak->nS1SliceIdx == nSliceIdx)
    {
        return pTrak->nS1SmpIdx;
    }
    // Forward
    else if (pTrak->nS1SliceIdx < nSliceIdx)
    {
        // Iterate each STSC entry
        while (pTrak->nS1STSCIdx <= nMaxSTSCIdx)
        {
            // Get the 'sample per chunk' of current STSC entry
            ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->nS1STSCIdx, 1, &nSamplePerChunk);
            if (ret != GPP_SUCCESS)
            {
                return -1;
            }

            // Get the 'first chunk index' of next STSC entry
            if (pTrak->nS1STSCIdx + 1 >= pStsc->dwEntryCount)
            {
                nNextSTSCChunkIdx = pStco->dwEntryCount;
            }
            else
            {
                ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->nS1STSCIdx + 1, 0, &nNextSTSCChunkIdx);
                if (ret != GPP_SUCCESS)
                {
                    return -1;
                }
            }

            // Iterate each chunk in this STSC entry
            while (pTrak->nS1ChunkIdx <= nNextSTSCChunkIdx)
            {
                // The condition (pTrak->nS1ChunkIdx == nNextSTSCChunkIdx) is only for the last chunk of audio stream.
                // So if pTrak->nS1ChunkIdx is not equal to the last chunk of audio stream, its iteration should be skipped.
                if ((pTrak->nS1ChunkIdx == nNextSTSCChunkIdx) &&
                    !((pTrak->nS1STSCIdx == nMaxSTSCIdx) && (nNextSTSCChunkIdx == pStco->dwEntryCount)))
                {
                    break;
                }

                // Iterate each sample in this chunk
                while (pTrak->nS1SmpInChunk < nSamplePerChunk)
                {
                    // Get the size of current sample
                    if (pStsz->dwSampleSize == 0)
                    {
                        ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, pTrak->nS1SmpIdx-1, 0, &nSize);
                        if (ret != GPP_SUCCESS)
                        {
                            return -1;
                        }
                    }
                    else
                    {
                        nSize = pStsz->dwSampleSize;
                    }
                    nSliceSize += nSize;

                    // If nSliceSize reach the nMaxBytePerSlice, increase slice index
                    if (nSliceSize > pTrak->nMaxBytePerSlice)
                    {
                        pTrak->nS1SliceIdx++;
                        nSliceSize = 0;
                        if (pTrak->nS1SliceIdx == nSliceIdx)
                        {
                            // found!
                            return pTrak->nS1SmpIdx;
                        }
                    }
                    else
                    {
                        pTrak->nS1SmpIdx++;
                        pTrak->nS1SmpInChunk++;
                    }
                }

                // Reach the end of this chunk, move to next, increase slice index
                pTrak->nS1ChunkIdx++;
                pTrak->nS1SmpInChunk = 0;

                // Last slice in this chunk
                if (nSliceSize > 0)
                {
                    pTrak->nS1SliceIdx++;
                    nSliceSize = 0;
                    if (pTrak->nS1SliceIdx == nSliceIdx)
                    {
                        // found!
                        return pTrak->nS1SmpIdx;
                    }
                }
            }

            // Reach the end of this STSC entry, move to next
            pTrak->nS1STSCIdx++;
        }
    }
    // Backward
    else	// (pTrak->nS1SliceIdx > nSliceIdx)
    {
        if (pTrak->nS1STSCIdx >= 0)
        {
            // Get the 'sample per chunk' of current STSC entry
            ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->nS1STSCIdx, 1, &nSamplePerChunk);
            if (ret != GPP_SUCCESS)
            {
                return -1;
            }

            // Get the 'first chunk index' of this STSC entry
            if (pTrak->nS1STSCIdx <= 0)
            {
                nThisSTSCChunkIdx = 1;
            }
            else
            {
                ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->nS1STSCIdx, 0, &nThisSTSCChunkIdx);
                if (ret != GPP_SUCCESS)
                {
                    return -1;
                }
            }

            // Iterate each STSC entry
            while (pTrak->nS1STSCIdx >= 0)
            {
                // Iterate each chunk in this STSC entry
                while (pTrak->nS1ChunkIdx >= nThisSTSCChunkIdx)
                {
                    // Iterate each sample in this chunk
                    while (pTrak->nS1SmpInChunk > 0)
                    {
                        // Come into this chunk first time when Backward
                        if (pTrak->nS1SmpInChunk == nSamplePerChunk)
                        {
                            nCurSmpIdx = pTrak->nS1SmpIdx - nSamplePerChunk;	// the first sample index of this chunk
                            nSmpInChunk = 0;
                            nLastSliceSmpIdx = nCurSmpIdx;
                            nLastCurSmpIdx = nSmpInChunk;
                            nSliceSize = 0;

                            // Search for the sample index of last slice in this chunk
                            while (nSmpInChunk < nSamplePerChunk)
                            {
                                // Get the size of this sample
                                if (pStsz->dwSampleSize == 0)
                                {
                                    ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, nCurSmpIdx-1, 0, &nSize);
                                    if (ret != GPP_SUCCESS)
                                    {
                                        return -1;
                                    }
                                }
                                else
                                {
                                    nSize = pStsz->dwSampleSize;
                                }
                                nSliceSize += nSize;

                                // If nSliceSize reach the nMaxBytePerSlice, increase slice index
                                if (nSliceSize > pTrak->nMaxBytePerSlice)
                                {
                                    nLastSliceSmpIdx = nCurSmpIdx;
                                    nLastCurSmpIdx = nSmpInChunk;
                                    nSliceSize = 0;
                                }
                                else
                                {
                                    nCurSmpIdx++;
                                    nSmpInChunk++;
                                }
                            }

                            pTrak->nS1SmpIdx = nLastSliceSmpIdx;
                            pTrak->nS1SmpInChunk = nLastCurSmpIdx;
                            pTrak->nS1SliceIdx--;
                            nSliceSize = 0;

                            if (pTrak->nS1SliceIdx == nSliceIdx)
                            {
                                // found!
                                return pTrak->nS1SmpIdx;
                            }
                        }

                        pTrak->nS1SmpIdx--;
                        pTrak->nS1SmpInChunk--;

                        // Get the size of current sample
                        if (pStsz->dwSampleSize == 0)
                        {
                            ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, pTrak->nS1SmpIdx-1, 0, &nSize);
                            if (ret != GPP_SUCCESS)
                            {
                                return -1;
                            }
                        }
                        else
                        {
                            nSize = pStsz->dwSampleSize;
                        }
                        nSliceSize += nSize;

                        // If nSliceSize reach the nMaxBytePerSlice, decrease slice index
                        if (nSliceSize >= pTrak->nMaxBytePerSlice)
                        {
                            pTrak->nS1SliceIdx--;
                            nSliceSize = 0;

                            // If nSliceSize exceeds, move back to one previous sample
                            if (nSliceSize > pTrak->nMaxBytePerSlice)
                            {
                                pTrak->nS1SmpIdx++;
                                pTrak->nS1SmpInChunk++;
                            }

                            if (pTrak->nS1SliceIdx == nSliceIdx)
                            {
                                // found!
                                return pTrak->nS1SmpIdx;
                            }
                        }
                    }

                    // Reach the begin of this chunk, move to previous, decrease slice index
                    pTrak->nS1ChunkIdx--;
                    pTrak->nS1SmpInChunk = nSamplePerChunk;

                    // Last slice in this chunk
                    if (nSliceSize > 0)
                    {
                        pTrak->nS1SliceIdx--;
                        nSliceSize = 0;

                        if (pTrak->nS1SliceIdx == nSliceIdx)
                        {
                            // found!
                            return pTrak->nS1SmpIdx;
                        }
                    }
                }

                // Reach the begin of this STSC entry, move to previous
                pTrak->nS1STSCIdx--;

                if (pTrak->nS1STSCIdx >= 0)
                {
                    // Get the 'sample per chunk' of current STSC entry
                    ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->nS1STSCIdx, 1, &nSamplePerChunk);
                    if (ret != GPP_SUCCESS)
                    {
                        return -1;
                    }
                    pTrak->nS1SmpInChunk = nSamplePerChunk;

                    // Get the 'first chunk index' of this STSC entry
                    if (pTrak->nS1STSCIdx <= 0)
                    {
                        nThisSTSCChunkIdx = 1;
                    }
                    else
                    {
                        ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->nS1STSCIdx, 0, &nThisSTSCChunkIdx);
                        if (ret != GPP_SUCCESS)
                        {
                            return -1;
                        }
                    }
                }
            }
        }
    }

    return 0;
}



static signed int
GetSliceIdxFromSampleIdx(
                         MMP_3GPP_DECODE_TRAK*     pTrak,
                         unsigned int              nSampleIdx)
{
    GPP_RESULT ret;

    MMP_3GPP_DECODE_STSZ    *pStsz = &(pTrak->mdia.minf.stbl.stsz);    // size
    MMP_3GPP_DECODE_STSC    *pStsc = &(pTrak->mdia.minf.stbl.stsc);    // samples per chunk
    MMP_3GPP_DECODE_STCO    *pStco = &(pTrak->mdia.minf.stbl.stco);    // chunk offset

    int             i1;
    int             nMaxSTSCIdx, nSliceSize;
    unsigned long   nThisSTSCChunkIdx, nNextSTSCChunkIdx;  // 1'idx
    unsigned long   nSamplePerChunk, nSize;
    // For Backward
    int          nCurSmpIdx;            // current sample index
    int          nSmpInChunk;           // sample in chunk

    if (pTrak == NULL)
    {
        return 0;
    }

    nMaxSTSCIdx = pStsc->dwEntryCount - 1;
    nSliceSize  = 0;

    if (nSampleIdx == 0)
    {
        return 0;
    }

    // Initialize
    if (pTrak->nS2SmpIdx == 0)
    {
        pTrak->nS2SliceIdx    = 1;   // 1'idx
        pTrak->nS2SmpIdx      = 0;   // 1'idx
        pTrak->nS2STSCIdx     = 0;
        pTrak->nS2ChunkIdx    = 1;   // 1'idx
        pTrak->nS2SmpInChunk  = 0;	// [1 ~ nSamplePerChunk]
        pTrak->nSliceCntInChunk  = 0;
        PalMemset(pTrak->tSliceInChunk, 0, sizeof(MMP_3GPP_DECODE_SLICEINCHUNK_T)*MAX_SLICE_IN_CHUNK);
    }

    // Match
    if (pTrak->nS2SmpIdx == nSampleIdx)
    {
        return pTrak->nS2SliceIdx;
    }
    // Forward
    else if (pTrak->nS2SmpIdx < nSampleIdx)
    {
        // Iterate each STSC entry
        while (pTrak->nS2STSCIdx <= nMaxSTSCIdx)
        {
            // Get the 'sample per chunk' of current STSC entry
            ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->nS2STSCIdx, 1, &nSamplePerChunk);
            if (ret != GPP_SUCCESS)
            {
                return -1;
            }

            // Get the 'first chunk index' of next STSC entry
            if (pTrak->nS2STSCIdx + 1 >= pStsc->dwEntryCount)
            {
                nNextSTSCChunkIdx = pStco->dwEntryCount;
            }
            else
            {
                ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->nS2STSCIdx + 1, 0, &nNextSTSCChunkIdx);
                if (ret != GPP_SUCCESS)
                {
                    return -1;
                }
            }

            // Iterate each chunk in this STSC entry
            while (pTrak->nS2ChunkIdx <= nNextSTSCChunkIdx)
            {
                // The condition (pTrak->nS1ChunkIdx == nNextSTSCChunkIdx) is only for the last chunk of audio stream.
                // So if pTrak->nS1ChunkIdx is not equal to the last chunk of audio stream, its iteration should be skipped.
                if ((pTrak->nS2ChunkIdx == nNextSTSCChunkIdx) &&
                    !((pTrak->nS2STSCIdx == nMaxSTSCIdx) && (nNextSTSCChunkIdx == pStco->dwEntryCount)))
                {
                    break;
                }

                // Iterate each sample in this chunk
                while (pTrak->nS2SmpInChunk < nSamplePerChunk)
                {
                    pTrak->nS2SmpIdx++;
                    pTrak->nS2SmpInChunk++;

                    // Get the size of current sample
                    if (pStsz->dwSampleSize == 0)
                    {
                        ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, pTrak->nS2SmpIdx-1, 0, &nSize);
                        if (ret != GPP_SUCCESS)
                        {
                            return -1;
                        }
                    }
                    else
                    {
                        nSize = pStsz->dwSampleSize;
                    }
                    pTrak->nS2SliceSize += nSize;

                    // If nSliceSize reach the nMaxBytePerSlice, increase slice index
                    if (pTrak->nS2SliceSize > pTrak->nMaxBytePerSlice)
                    {
                        // Update slice info in chunk
                        pTrak->tSliceInChunk[pTrak->nSliceCntInChunk].nSliceIdx  = pTrak->nS2SliceIdx;
                        pTrak->tSliceInChunk[pTrak->nSliceCntInChunk].nSliceSize = pTrak->nS2SliceSize - nSize;
                        pTrak->nSliceCntInChunk++;

                        // Update slice index and size
                        pTrak->nS2SliceIdx++;
                        pTrak->nS2SliceSize = nSize;     // slice size is initialized to the size of current sample
                    }

                    if (pTrak->nS2SmpIdx == nSampleIdx)
                    {
                        // found!
                        return pTrak->nS2SliceIdx;
                    }
                }

                // Reach the end of this chunk, move to next, increase slice index
                pTrak->nS2ChunkIdx++;
                pTrak->nS2SmpInChunk = 0;

                // Last slice in this chunk
                if (pTrak->nS2SliceSize > 0)
                {
                    pTrak->nS2SliceIdx++;
                    pTrak->nS2SliceSize = 0;
                }

                // Move to next chunk, initialize slice info in chunk
                pTrak->nSliceCntInChunk = 0;
                PalMemset(pTrak->tSliceInChunk, 0, sizeof(MMP_3GPP_DECODE_SLICEINCHUNK_T)*MAX_SLICE_IN_CHUNK);
            }

            // Reach the end of this STSC entry, move to next
            pTrak->nS2STSCIdx++;
        }
    }
    // Backward
    else	// (pTrak->m_dwS2SampleIdx > nSampleIdx)
    {
        if (pTrak->nS2STSCIdx >= 0)
        {
            // Get the 'sample per chunk' of current STSC entry
            ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->nS2STSCIdx, 1, &nSamplePerChunk);
            if (ret != GPP_SUCCESS)
            {
                return -1;
            }

            // Get the 'first chunk index' of this STSC entry
            if (pTrak->nS2STSCIdx <= 0)
            {
                nThisSTSCChunkIdx = 1;
            }
            else
            {
                ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->nS2STSCIdx, 0, &nThisSTSCChunkIdx);
                if (ret != GPP_SUCCESS)
                {
                    return -1;
                }
            }

            // Iterate each STSC entry
            while (pTrak->nS2STSCIdx >= 0)
            {
                // Iterate each chunk in this STSC entry
                while (pTrak->nS2ChunkIdx >= nThisSTSCChunkIdx)
                {
                    // Iterate each sample in this chunk
                    while (pTrak->nS2SmpInChunk > 0)
                    {
                        // Come into this chunk first time when Backward
                        // But for the last chunk of audio stream, it should be skipped.
                        if ((pTrak->nS2SmpInChunk == nSamplePerChunk) &&
                            !((pTrak->nS2STSCIdx == nMaxSTSCIdx) && (pTrak->nS2ChunkIdx == pStco->dwEntryCount)))
                        {
                            nCurSmpIdx = pTrak->nS2SmpIdx - nSamplePerChunk;	// the first sample index of this chunk
                            nSmpInChunk = 0;
                            nSliceSize = 0;

                            // Initialize slice info in chunk
                            pTrak->nSliceCntInChunk = 0;
                            PalMemset(pTrak->tSliceInChunk, 0, sizeof(MMP_3GPP_DECODE_SLICEINCHUNK_T)*MAX_SLICE_IN_CHUNK);

                            // Update ALL slice info in this chunk
                            while (nSmpInChunk < nSamplePerChunk)
                            {
                                // Get the size of this sample
                                if (pStsz->dwSampleSize == 0)
                                {
                                    ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, nCurSmpIdx-1, 0, &nSize);
                                    if (ret != GPP_SUCCESS)
                                    {
                                        return -1;
                                    }
                                }
                                else
                                {
                                    nSize = pStsz->dwSampleSize;
                                }
                                nSliceSize += nSize;

                                // If nSliceSize reach the nMaxBytePerSlice, increase slice index
                                if (nSliceSize > pTrak->nMaxBytePerSlice)
                                {
                                    // Update slice info in chunk
                                    pTrak->tSliceInChunk[pTrak->nSliceCntInChunk].nSliceIdx  = pTrak->nSliceCntInChunk;     // Use related index temporarily
                                    pTrak->tSliceInChunk[pTrak->nSliceCntInChunk].nSliceSize = nSliceSize - nSize;
                                    pTrak->nSliceCntInChunk++;
                                    nSliceSize = nSize;
                                }

                                nCurSmpIdx++;
                                nSmpInChunk++;
                            }

                            // Last slice in the chunk
                            if (nSliceSize > 0)
                            {
                                // Update slice info in chunk
                                pTrak->tSliceInChunk[pTrak->nSliceCntInChunk].nSliceIdx  = pTrak->nSliceCntInChunk;
                                pTrak->tSliceInChunk[pTrak->nSliceCntInChunk].nSliceSize = nSliceSize;
                                pTrak->nSliceCntInChunk++;
                            }

                            // Update the nSliceIdx to absolute index
                            for (i1=0; i1<pTrak->nSliceCntInChunk; i1++)
                            {
                                pTrak->tSliceInChunk[i1].nSliceIdx = pTrak->nS2SliceIdx - (pTrak->nSliceCntInChunk - i1 -1);
                            }

                            if (pTrak->tSliceInChunk[pTrak->nSliceCntInChunk-1].nSliceIdx == pTrak->nS2SliceIdx)
                            {
                                pTrak->nS2SliceSize = pTrak->tSliceInChunk[pTrak->nSliceCntInChunk-1].nSliceSize;
                            }
                            else
                            {
                                // Something Wrong
                                printf("[SXA_DMX] <GetSliceIdxFromSampleIdx() ERROR 1> pTrak->nSliceCntInChunk = %ld\r\n", pTrak->nSliceCntInChunk);
                                printf("[SXA_DMX] pTrak->nS2SliceIdx = %ld\r\n", pTrak->nS2SliceIdx);
                                printf("[SXA_DMX] pTrak->tSliceInChunk[pTrak->nSliceCntInChunk-1].nSliceIdx = %ld\r\n", pTrak->tSliceInChunk[pTrak->nSliceCntInChunk-1].nSliceIdx);
                                return -2;
                            }
                            pTrak->nSliceCntInChunk--;
                        }

                        // Get the size of current sample
                        if (pStsz->dwSampleSize == 0)
                        {
                            ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, pTrak->nS2SmpIdx-1, 0, &nSize);
                            if (ret != GPP_SUCCESS)
                            {
                                return -1;
                            }
                        }
                        else
                        {
                            nSize = pStsz->dwSampleSize;
                        }
                        pTrak->nS2SliceSize -= nSize;

                        // If nSliceSize reach the nMaxBytePerSlice, decrease slice index
                        if (pTrak->nS2SliceSize <= 0)
                        {
                            pTrak->nS2SliceIdx--;
                            // Get the size of previous slice
                            if (pTrak->nSliceCntInChunk > 0)
                            {
                                if (pTrak->tSliceInChunk[pTrak->nSliceCntInChunk-1].nSliceIdx == pTrak->nS2SliceIdx)
                                {
                                    pTrak->nS2SliceSize = pTrak->tSliceInChunk[pTrak->nSliceCntInChunk-1].nSliceSize;
                                }
                                else
                                {
                                    // Something Wrong
                                    printf("[SXA_DMX] <GetSliceIdxFromSampleIdx() ERROR 2> pTrak->nSliceCntInChunk = %ld\r\n", pTrak->nSliceCntInChunk);
                                    printf("[SXA_DMX] pTrak->nS2SliceIdx = %ld\r\n", pTrak->nS2SliceIdx);
                                    printf("[SXA_DMX] pTrak->tSliceInChunk[pTrak->nSliceCntInChunk-1].nSliceIdx = %ld\r\n", pTrak->tSliceInChunk[pTrak->nSliceCntInChunk-1].nSliceIdx);
                                    return -2;
                                }
                                pTrak->nSliceCntInChunk--;
                            }
                            else
                            {
                                // if pTrak->nSliceCntInChunk == 0, it must be at the begin of a chunk
                            }
                        }

                        pTrak->nS2SmpIdx--;
                        pTrak->nS2SmpInChunk--;

                        if (pTrak->nS2SmpIdx == nSampleIdx)
                        {
                            // found!
                            return pTrak->nS2SliceIdx;
                        }
                    }

                    // Reach the begin of this chunk, move to previous, decrease slice index
                    pTrak->nS2ChunkIdx--;
                    pTrak->nS2SmpInChunk = nSamplePerChunk;

                    // Move to previous chunk, initialize slice info in chunk
                    pTrak->nSliceCntInChunk = 0;
                    PalMemset(pTrak->tSliceInChunk, 0, sizeof(MMP_3GPP_DECODE_SLICEINCHUNK_T)*MAX_SLICE_IN_CHUNK);
                }

                // Reach the begin of this STSC entry, move to previous
                pTrak->nS2STSCIdx--;

                if (pTrak->nS2STSCIdx >= 0)
                {
                    // Get the 'sample per chunk' of current STSC entry
                    ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->nS2STSCIdx, 1, &nSamplePerChunk);
                    if (ret != GPP_SUCCESS)
                    {
                        return -1;
                    }
                    pTrak->nS2SmpInChunk = nSamplePerChunk;

                    // Get the 'first chunk index' of this STSC entry
                    if (pTrak->nS2STSCIdx <= 0)
                    {
                        nThisSTSCChunkIdx = 1;
                    }
                    else
                    {
                        ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->nS2STSCIdx, 0, &nThisSTSCChunkIdx);
                        if (ret != GPP_SUCCESS)
                        {
                            return -1;
                        }
                    }

                    // Initialize slice info in chunk
                    pTrak->nSliceCntInChunk = 0;
                    PalMemset(pTrak->tSliceInChunk, 0, sizeof(MMP_3GPP_DECODE_SLICEINCHUNK_T)*MAX_SLICE_IN_CHUNK);
                }
            }
        }
    }

    return 0;
}

#endif



//=============================================================================
//                              Public Function Definition
//=============================================================================

MMP_3GPP_DECODE_3GPPDATA *sxaDmx3GPP_CreateDec3gppData(void)
{
    MMP_3GPP_DECODE_3GPPDATA *p1;

    p1 = (MMP_3GPP_DECODE_3GPPDATA *)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(MMP_3GPP_DECODE_3GPPDATA));
    if (p1 == NULL)
    {
        return NULL;
    }
    else
    {
        PalMemset(p1, 0, sizeof(MMP_3GPP_DECODE_3GPPDATA));
        return p1;
    }
}



SXA_DMXECODE_E sxaDmx3GPP_CreateADTSHeader(
                                       MMP_3GPP_DECODE_3GPPDATA *pGppData,
                                       unsigned int              aacFrameLength,
                                       unsigned char            *pDestBufAddr)
{
    //[dvyu] ISO 14496-3, Sec 1.1.2.1 and 1.1.2.2

    MMP_3GPP_DECODE_MP4AENTRY *pMp4aEntry;

    unsigned long    dwSynWord = 0xFFF;                    // 12bit
    unsigned long    dwID = 0;                             // 1bit
    unsigned long    dwLayer = 0;                          // 2bit    '00'
    unsigned long    dwPotectAbsent = 1;                   // 1bit
    unsigned long    dwProfile = 1;                        // 2bit
    unsigned long    dwSampleFreIdx = 3;                   // 4bit
    unsigned long    dwPrivateBit = 0;                     // 1bit
    unsigned long    dwChannelConfig = 2;                  // 3bit
    unsigned long    dwOriCopy = 0;                        // 1bit
    unsigned long    dwHome = 0;                           // 1bit
    unsigned long    dwCRIDBit = 0;                        // 1bit
    unsigned long    dwCRIDStart = 0;                      // 1bit
    unsigned long    dwFrameLength = aacFrameLength + 7;   // 13bit
    unsigned long    dwBufferFullness = 0x7FF;             // 11bit
    unsigned long    dwNoRawData = 0;                      // 2bit

    // [dvyu] Only for AAC audio format
    if (sxaDmx3GPP_GetAudioCodecType(pGppData) != SXA_DMXAFMT_MP4)
    {
        return SXA_DMXECODE_EFAIL;
    }

    pMp4aEntry = &(pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.mp4aEntry);

    /* Read from 3GP MOOV */
    dwSampleFreIdx  = pMp4aEntry->esdBox.samplingFrequencyIndex;
    dwChannelConfig = pMp4aEntry->esdBox.channelConfiguration;
    switch (pMp4aEntry->esdBox.audioObjectType)
    {
    case (1):
        dwProfile = 0;
        break;
    case (2):
        dwProfile = 1;
        break;
    default:
        // Other MODE Not Completed
        // Still have dwProfile = 2/3
        break;
    }

    pDestBufAddr[0] = (unsigned char)(dwSynWord >> 4);
    pDestBufAddr[1] = (unsigned char)(((dwSynWord & 0xF) << 4) | (dwID << 3) | (dwLayer << 1) | (dwPotectAbsent));
    pDestBufAddr[2] = (unsigned char)((dwProfile << 6) | (dwSampleFreIdx << 2) | (dwPrivateBit << 1) | (dwChannelConfig >> 2));
    pDestBufAddr[3] = (unsigned char)(((dwChannelConfig & 0x3) << 6) | (dwOriCopy << 5) | (dwHome << 4) |(dwCRIDBit << 3) | (dwCRIDStart << 2)| (dwFrameLength >> 11));
    pDestBufAddr[4] = (unsigned char)((dwFrameLength & 0x7F8) >> 3);
    pDestBufAddr[5] = (unsigned char)(((dwFrameLength & 0x7 ) << 5) | (dwBufferFullness >> 6));
    pDestBufAddr[6] = (unsigned char)(((dwBufferFullness & 0x3F) << 2) | (dwNoRawData));

    return SXA_DMXECODE_SOK;
}



GPP_RESULT
sxaDmx3GPP_GetFirstAudioTrack(
                              MMP_3GPP_DECODE_3GPPDATA* pGppData,
                              unsigned int *index)
{
    unsigned int  i;
    unsigned long dwHandleType = 0;
    unsigned long dwTrakCount = pGppData->dwTrakCount;
    for (i=0; i<dwTrakCount; i++)
    {
        dwHandleType = pGppData->moov.trak[i].mdia.hdlr.dwHandleType;
        if (dwHandleType == SOUN_HANDLER_TYPE)
        {
            *index = i;
            return (GPP_SUCCESS);
        }
    }
    return (GPP_ERROR_DECODE_GET_FIRST_AUDIO_TRAK);
}



GPP_RESULT
sxaDmx3GPP_GetFirstVideoTrack(
                              MMP_3GPP_DECODE_3GPPDATA* pGppData,
                              unsigned int *index)
{
    unsigned int  i;
    unsigned long dwHandleType = 0;
    unsigned long dwTrakCount = pGppData->dwTrakCount;
    for (i = 0; i < dwTrakCount; i++)
    {
        dwHandleType = pGppData->moov.trak[i].mdia.hdlr.dwHandleType;
        if (dwHandleType == VIDE_HANDLER_TYPE)
        {
            *index = i;
            return (GPP_SUCCESS);
        }
    }

    return (GPP_ERROR_DECODE_GET_FIRST_VIDEO_TRAK);
}



void
sxaDmx3GPP_Terminate(
                     MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned int i;
    MMP_3GPP_DECODE_STTS stts;
    MMP_3GPP_DECODE_STSC stsc;
    MMP_3GPP_DECODE_STSZ stsz;
    MMP_3GPP_DECODE_STCO stco;
    MMP_3GPP_DECODE_STSS stss;
    unsigned long dwTrakCount = pGppData->dwTrakCount;
    if (pGppData->moov.udta.pchCnam)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, pGppData->moov.udta.pchCnam);
        pGppData->moov.udta.pchCnam = 0;
    }
    if (pGppData->moov.udta.pchCart)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, pGppData->moov.udta.pchCart);
        pGppData->moov.udta.pchCart = 0;
    }
    if (pGppData->moov.udta.pchCalb)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, pGppData->moov.udta.pchCalb);
        pGppData->moov.udta.pchCalb = 0;
    }
    gppOpenExParam.nOffset = 0;
    gppOpenExParam.nLength = 0;

    for (i=0; i<dwTrakCount; i++)
    {
        // mstd_free STTS_D
        stts = pGppData->moov.trak[i].mdia.minf.stbl.stts;
        sxaDmx3GPP_TableStreamDestory(&stts.sttsEntry);

        // mstd_free STSC_D
        stsc = pGppData->moov.trak[i].mdia.minf.stbl.stsc;
        sxaDmx3GPP_TableStreamDestory(&stsc.stscEntry);

        // mstd_free STSZ_D
        stsz = pGppData->moov.trak[i].mdia.minf.stbl.stsz;
        sxaDmx3GPP_TableStreamDestory(&stsz.stszEntry);

        // mstd_free STCO_D
        stco = pGppData->moov.trak[i].mdia.minf.stbl.stco;
        sxaDmx3GPP_TableStreamDestory(&stco.stcoEntry);

        // mstd_free STSS_D
        stss = pGppData->moov.trak[i].mdia.minf.stbl.stss;
        if (stss.pSampleNumber)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, stss.pSampleNumber);
        }
    }

    if (pGppData->gppFile != NULL)
    {
        //PalTFileClose(pGppData->gppFile, MMP_NULL);
    }

    if (pGppData)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, pGppData);
        pGppData = NULL;
    }

    if (gtSample_info) {
        //PalHeapFree(PAL_HEAP_DEFAULT, gtSample_info);
        //gtSample_info = NULL;
    }
    printf("sxaDmx3GPP_Terminate gtSample_info\n");
    
}



unsigned int
sxaDmx3GPP_GetKeySampleCount(
                            MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    MMP_3GPP_DECODE_STSS *pStss;
    if (pGppData->pFirstVideoTrak == NULL)
    {
        return 0;
    }

    pStss = &(pGppData->pFirstVideoTrak->mdia.minf.stbl.stss);
    if (pStss != NULL)
    {
        return pStss->dwEntryCount;
    }
    else
    {
        return 0;
    }
}



unsigned int*
sxaDmx3GPP_GetKeySampleIndex(
                            MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    MMP_3GPP_DECODE_STSS *pStss;
    if (pGppData->pFirstVideoTrak == NULL)
    {
        return NULL;
    }

    pStss = &(pGppData->pFirstVideoTrak->mdia.minf.stbl.stss);
    if (pStss != NULL)
    {
        return (unsigned int *)(pStss->pSampleNumber);
    }
    else
    {
        return NULL;
    }
}



static GPP_RESULT
sxaDmx3GPP_GetSampleCount(
                          MMP_3GPP_DECODE_TRAK *pTrak,
                          unsigned int         *count)
{
    MMP_3GPP_DECODE_STSZ *pStsz = &(pTrak->mdia.minf.stbl.stsz);
    if (pStsz->dwSampleCount != 0)
    {
        *count = pStsz->dwSampleCount;
    }
    else
    {
        *count = pStsz->dwEntryCount;
    }
    return (GPP_SUCCESS);
}



unsigned int
sxaDmx3GPP_GetVideoSampleCount(
                     MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned int count = 0;

    if (pGppData->pFirstVideoTrak != NULL)
    {
        sxaDmx3GPP_GetSampleCount(pGppData->pFirstVideoTrak, &count);
    }

    return count;
}



unsigned int
sxaDmx3GPP_GetAudioSampleCount(
                     MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned int    count = 0;
#if (SUPPORT_NEW_PCM_INFO)
    unsigned long   nAudioType;
#endif

    if (pGppData->pFirstAudioTrak != NULL)
    {
#if (SUPPORT_NEW_PCM_INFO)
        nAudioType = pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.dwEntryType;
        if ((nAudioType == RAW_SOUND_TYPE) || (nAudioType == ULAW_SOUND_TYPE) || (nAudioType == SOWT_SOUND_TYPE) || (nAudioType == TWOS_SOUND_TYPE))
        {
            count = pGppData->pFirstAudioTrak->nSliceCount;
        }
        else
        {
            sxaDmx3GPP_GetSampleCount(pGppData->pFirstAudioTrak, &count);
        }
#else
        sxaDmx3GPP_GetSampleCount(pGppData->pFirstAudioTrak, &count);
#endif
    }

    return count;
}



GPP_RESULT
sxaDmx3GPP_GetChunkInfo(
                        MMP_3GPP_DECODE_TRAK*           pTrak,
                        unsigned long*                  pChunkCount,
                        unsigned long*                  pChunkIdx)
{
    GPP_RESULT              ret;
    unsigned long           tempValue;
    MMP_3GPP_DECODE_STSC    *pStsc      = &(pTrak->mdia.minf.stbl.stsc);    // samples per chunk

    ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->m_dwLstSTSCIdx, 1, &tempValue);
    *pChunkCount = tempValue;
    *pChunkIdx = pTrak->m_dwLstSTSCCount;

    return GPP_SUCCESS;
}



GPP_RESULT
sxaDmx3GPP_IsConstSizeDelta(
                            MMP_3GPP_DECODE_TRAK*           pTrak,
                            MMP_BOOL*                       pRet)
{
    MMP_3GPP_DECODE_STTS    *pStts      = &(pTrak->mdia.minf.stbl.stts);    // delta
    MMP_3GPP_DECODE_STSZ    *pStsz      = &(pTrak->mdia.minf.stbl.stsz);    // size

    if (pStsz->dwSampleSize != 0 && pStts->dwEntryCount == 1)
    {
        *pRet =  MMP_TRUE;
    }
    else
    {
        *pRet =  MMP_FALSE;
    }

    return GPP_SUCCESS;
}



static GPP_RESULT
sxaDmx3GPP_GetSampleInfo(
                         MMP_3GPP_DECODE_3GPPDATA*       hDecGppData,
                         MMP_3GPP_DECODE_TRAK*           pTrak,
                         SXA_DMXSAMPLEINFO_T*            pInfo )
{
    GPP_RESULT ret;
    MMP_3GPP_DECODE_STTS    *pStts      = &(pTrak->mdia.minf.stbl.stts);    // delta
    MMP_3GPP_DECODE_STSZ    *pStsz      = &(pTrak->mdia.minf.stbl.stsz);    // size
    MMP_3GPP_DECODE_STSC    *pStsc      = &(pTrak->mdia.minf.stbl.stsc);    // samples per chunk
    MMP_3GPP_DECODE_STCO    *pStco      = &(pTrak->mdia.minf.stbl.stco);    // chunk offset

    unsigned long           dwSampleIdx = pInfo->nIndex;    // 1'idx

    signed long       i;
    signed long       dwSampleDiff;
    signed long       dwNextSTSCChunkIdx;        // 1'idx    // [dvyu] first chunk index of next STSC entry
    signed long       dwCurrSTSCChunkIdx;        // 1'idx    // [dvyu] first chunk index of current STSC entry
    signed long       dwCount;
    signed long       dwSttsCount, dwSttsDuration;
    signed long       dwSamplePerChunk, dwSampleSize;

    signed long       is_forward = MMP_TRUE;

#if (SUPPORT_NEW_PCM_INFO)
    unsigned int    dwSliceIdx;                 // 1'idx
    int             bIsAudio = (pTrak->mdia.hdlr.dwHandleType == SOUN_HANDLER_TYPE) ? MMP_TRUE : MMP_FALSE;
    int             nCurSmpInChunk;
    int             nCurSmpIdx;                 // current sample index
    unsigned long   nSliceSize = 0, nSize = 0;
    unsigned long   nAudioType = pTrak->mdia.minf.stbl.stsd.dwEntryType;
#endif
    unsigned short  nChannel = 1;
    unsigned long   nOriginalSize = 0;

    if (dwSampleIdx == 0)
    {
        pInfo->nTimeStamp = 0;
        pInfo->nOffset = 0;
        pInfo->nSize = 0;
        ret = sxaDmx3GPP_GetValueFromTableStream(&pStts->sttsEntry, 0, 1, (unsigned long *)&pInfo->nDelta);
        return (GPP_SUCCESS);
    }

    if (pTrak->m_dwLstSampleIdx == 0)
    {
        pTrak->m_dwLstSampleIdx    = 0;
        pTrak->m_dwTimeStampBase   = 0;
        pTrak->m_dwTimeStampRemain = 0;
        pTrak->m_dwLstSTTSIdx      = 0;
        pTrak->m_dwLstSTTSCount    = 0;
        pTrak->m_dwLstSTSCIdx      = 0;
        pTrak->m_dwLstSTSCChunkIdx = 0;
        pTrak->m_dwLstSTSCCount    = 0;
        ret = sxaDmx3GPP_GetValueFromTableStream(&pStco->stcoEntry, 0, 0, &pTrak->m_dwLstSTSCOffset);
        //printf("sxaDmx3GPP_GetValueFromTableStream %d \n",pTrak->m_dwLstSTSCOffset);
        if (ret != GPP_SUCCESS)
        {
            return ret;
        }
        pTrak->m_dwLstSTCOIdx      = 0;
        pTrak->m_dwLstSTSZIdx      = 0;
    }

#if (SUPPORT_NEW_PCM_INFO)
    if ((bIsAudio) &&
        ((nAudioType == RAW_SOUND_TYPE) || (nAudioType == ULAW_SOUND_TYPE) || (nAudioType == SOWT_SOUND_TYPE) || (nAudioType == TWOS_SOUND_TYPE)))
    {
        dwSliceIdx = pInfo->nIndex;
        // Get sample index from slice index
        dwSampleIdx = GetSampleIdxFromSliceIdx(pTrak, dwSliceIdx);
    }
#endif

    if ((bIsAudio) &&
        (nAudioType == ULAW_SOUND_TYPE))
    {
        nChannel = pTrak->mdia.minf.stbl.stsd.ulawEntry.wChannelNum;
        nOriginalSize = pStsz->dwOriginalSize;
    }

    /*
    * decode delta
    */
    dwSampleDiff = dwSampleIdx - pTrak->m_dwLstSampleIdx;

    if (dwSampleDiff >= 0)
    {
        is_forward = MMP_TRUE;

        // [dvyu] Get the number of sample in current STTS entry
        ret = sxaDmx3GPP_GetValueFromTableStream(&pStts->sttsEntry, pTrak->m_dwLstSTTSIdx, 0, &dwSttsCount);
        if (ret != GPP_SUCCESS)
        {
            return ret;
        }

        while (pTrak->m_dwLstSTTSCount + dwSampleDiff >= dwSttsCount)
        {
            // [dvyu] Get the number of sample in current STTS entry
            ret = sxaDmx3GPP_GetValueFromTableStream(&pStts->sttsEntry, pTrak->m_dwLstSTTSIdx, 0, &dwSttsCount);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }
            dwCount = dwSttsCount - pTrak->m_dwLstSTTSCount;

            // [dvyu] Get the duration of current STTS entry
            ret = sxaDmx3GPP_GetValueFromTableStream(&pStts->sttsEntry, pTrak->m_dwLstSTTSIdx, 1, &dwSttsDuration);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }

            if ((bIsAudio) &&
                (nAudioType == ULAW_SOUND_TYPE))
            {
                pTrak->m_dwTimeStampRemain += (dwSttsDuration * dwCount);// / (nChannel / nOriginalSize);
            }
            else
            {
                pTrak->m_dwTimeStampRemain += dwSttsDuration * dwCount;
            }

            // Update param
            if (pTrak->m_dwLstSTTSIdx < pStts->dwEntryCount - 1 )
            {
                pTrak->m_dwLstSTTSIdx++;
            }

            pTrak->m_dwLstSTTSCount = 0;
            dwSampleDiff -= dwCount;

            ret = sxaDmx3GPP_GetValueFromTableStream(&pStts->sttsEntry, pTrak->m_dwLstSTTSIdx, 0, &dwSttsCount);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }
        }

        ret = sxaDmx3GPP_GetValueFromTableStream(&pStts->sttsEntry, pTrak->m_dwLstSTTSIdx, 1, &dwSttsDuration);
        if (ret != GPP_SUCCESS)
        {
            return ret;
        }

        if ((bIsAudio) &&
            (nAudioType == ULAW_SOUND_TYPE))
        {
            pTrak->m_dwTimeStampRemain += (dwSttsDuration * dwSampleDiff);// / (nChannel / nOriginalSize);
        }
        else
        {
            pTrak->m_dwTimeStampRemain += dwSttsDuration * dwSampleDiff;
        }

        pTrak->m_dwTimeStampBase += pTrak->m_dwTimeStampRemain / pTrak->mdia.mdhd.dwTimeScale;
        pTrak->m_dwTimeStampRemain = pTrak->m_dwTimeStampRemain % pTrak->mdia.mdhd.dwTimeScale;

        // Update param
        pTrak->m_dwLstSTTSCount += dwSampleDiff;
    }
    else	// [dvyu] dwSampleDiff < 0
    {
        is_forward = MMP_FALSE;

        while (pTrak->m_dwLstSTTSCount + dwSampleDiff < 0)
        {
            // [dvyu] Get the duration of current STTS entry
            ret = sxaDmx3GPP_GetValueFromTableStream(&pStts->sttsEntry, pTrak->m_dwLstSTTSIdx, 1, &dwSttsDuration);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }

            if ((bIsAudio) &&
                (nAudioType == ULAW_SOUND_TYPE))
            {
                pTrak->m_dwTimeStampRemain -= (dwSttsDuration * pTrak->m_dwLstSTTSCount);// / (signed long)(nChannel / nOriginalSize);
            }
            else
            {
                pTrak->m_dwTimeStampRemain -= (dwSttsDuration * pTrak->m_dwLstSTTSCount);
            }

            // Update param
            dwSampleDiff += pTrak->m_dwLstSTTSCount;

            if (pTrak->m_dwLstSTTSIdx > 0 )
            {
                pTrak->m_dwLstSTTSIdx--;
            }

            ret = sxaDmx3GPP_GetValueFromTableStream(&pStts->sttsEntry, pTrak->m_dwLstSTTSIdx, 0, &pTrak->m_dwLstSTTSCount);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }
        }

        ret = sxaDmx3GPP_GetValueFromTableStream(&pStts->sttsEntry, pTrak->m_dwLstSTTSIdx, 1, &dwSttsDuration);
        if (ret != GPP_SUCCESS)
        {
            return ret;
        }

        if ((bIsAudio) &&
            (nAudioType == ULAW_SOUND_TYPE))
        {
            pTrak->m_dwTimeStampRemain += (dwSttsDuration * dwSampleDiff);// / (signed long)(nChannel / nOriginalSize);
        }
        else
        {
            pTrak->m_dwTimeStampRemain += dwSttsDuration * dwSampleDiff;
        }

        if (pTrak->m_dwTimeStampRemain < 0)
        {
            signed long qq, rr;

            qq = ABS(pTrak->m_dwTimeStampRemain) / (signed long)pTrak->mdia.mdhd.dwTimeScale;
            rr = ABS(pTrak->m_dwTimeStampRemain) % (signed long)pTrak->mdia.mdhd.dwTimeScale;

            if (rr == 0)
            {
                pTrak->m_dwTimeStampBase -= qq;
                pTrak->m_dwTimeStampRemain = rr;
            }
            else
            {
                pTrak->m_dwTimeStampBase -= (qq+1);
                pTrak->m_dwTimeStampRemain += (qq+1) * (signed long)pTrak->mdia.mdhd.dwTimeScale;
            }
        }
        else
        {
            pTrak->m_dwTimeStampBase += pTrak->m_dwTimeStampRemain / (signed long)pTrak->mdia.mdhd.dwTimeScale;
            pTrak->m_dwTimeStampRemain = pTrak->m_dwTimeStampRemain % (signed long)pTrak->mdia.mdhd.dwTimeScale;
        }

        // Update param
        pTrak->m_dwLstSTTSCount += dwSampleDiff;
    }

    /*
    * decode offset
    */

    dwSampleDiff = dwSampleIdx - pTrak->m_dwLstSampleIdx;

    if (dwSampleDiff >= 0)
    {
        is_forward = MMP_TRUE;

        if (pTrak->m_dwLstSTSCIdx + 1 >= pStsc->dwEntryCount)
        {
            dwNextSTSCChunkIdx = pStco->dwEntryCount;
        }
        else
        {
            ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->m_dwLstSTSCIdx + 1, 0, &dwNextSTSCChunkIdx);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }
        }

        // [dvyu] Get the 'sample per chunk' of current STSC entry
        ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->m_dwLstSTSCIdx, 1, &dwSamplePerChunk);
        if (ret != GPP_SUCCESS)
        {
            return ret;
        }

        while (pTrak->m_dwLstSTSCCount + dwSampleDiff > dwSamplePerChunk)
        {
            // [dvyu] Get the 'sample per chunk' of current STSC entry
            ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->m_dwLstSTSCIdx, 1, &dwSamplePerChunk);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }

            dwCount = dwSamplePerChunk - pTrak->m_dwLstSTSCCount;

            // Update param
            // [dvyu] Check if it should move to next STSC entry
            if (pTrak->m_dwLstSTSCChunkIdx + 1 == dwNextSTSCChunkIdx - 1)
            {
                if (pTrak->m_dwLstSTSCIdx + 1 < pStsc->dwEntryCount)
                {
                    pTrak->m_dwLstSTSCIdx++;
                }
                if (pTrak->m_dwLstSTSCIdx + 1 >= pStsc->dwEntryCount)
                {
                    dwNextSTSCChunkIdx = pStco->dwEntryCount;
                }
                else
                {
                    ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->m_dwLstSTSCIdx + 1, 0, &dwNextSTSCChunkIdx);
                    if (ret != GPP_SUCCESS)
                    {
                        return ret;
                    }
                }
            }

            pTrak->m_dwLstSTSCChunkIdx++;
            if (pTrak->m_dwLstSTSCChunkIdx > pStco->dwEntryCount)
            {
                return (GPP_ERROR_DECODE_GET_SAMPLE_INFO);
            }

            pTrak->m_dwLstSampleIdx += dwCount;
            pTrak->m_dwLstSTSCCount = 0;
            // [dvyu] Get the chunk offset of pTrak->m_dwLstSTSCChunkIdx
            ret = sxaDmx3GPP_GetValueFromTableStream(&pStco->stcoEntry, pTrak->m_dwLstSTSCChunkIdx, 0, &pTrak->m_dwLstSTSCOffset);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }

            dwSampleDiff -= dwCount;

            ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->m_dwLstSTSCIdx, 1, &dwSamplePerChunk);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }
        }

        // Update param
        for (i = 0; i < dwSampleDiff; i++)
        {
            pTrak->m_dwLstSampleIdx++;
            if (pStsz->dwSampleSize == 0)
            {
                // Add the sample size of each current index
                ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, pTrak->m_dwLstSampleIdx - 1, 0, &dwSampleSize);
                if (ret != GPP_SUCCESS)
                {
                    return ret;
                }
                pTrak->m_dwLstSTSCOffset += dwSampleSize;
            }
            else
            {
                pTrak->m_dwLstSTSCOffset += pStsz->dwSampleSize;
            }
        }
        pTrak->m_dwLstSTSCCount += dwSampleDiff;
    }
    else    // [dvyu] dwSampleDiff < 0
    {
        is_forward = MMP_FALSE;

        ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->m_dwLstSTSCIdx, 0, &dwCurrSTSCChunkIdx);
        if (ret != GPP_SUCCESS)
        {
            return ret;
        }

        // Update param
        while (pTrak->m_dwLstSTSCCount > 0)
        {
            if (pStsz->dwSampleSize == 0)
            {
                // Minus the sample size of each current index
                ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, pTrak->m_dwLstSampleIdx - 1, 0, &dwSampleSize);
                if (ret != GPP_SUCCESS)
                {
                    return ret;
                }
                pTrak->m_dwLstSTSCOffset -= dwSampleSize;
            }
            else
            {
                pTrak->m_dwLstSTSCOffset -= pStsz->dwSampleSize;
            }
            dwSampleDiff++;
            pTrak->m_dwLstSampleIdx--;
            pTrak->m_dwLstSTSCCount--;

            if (dwSampleDiff == 0)
            {
                break;
            }
        }

        while (pTrak->m_dwLstSTSCCount + dwSampleDiff <= 0)
        {
            pTrak->m_dwLstSampleIdx -= pTrak->m_dwLstSTSCCount;
            dwSampleDiff += pTrak->m_dwLstSTSCCount;
            pTrak->m_dwLstSTSCCount = 0;

            // Update param
            // [dvyu] Check if it should move to previous STSC entry
            if (pTrak->m_dwLstSTSCChunkIdx - 1 < dwCurrSTSCChunkIdx - 1)
            {
                if (pTrak->m_dwLstSTSCIdx - 1 >= 0)
                {
                    pTrak->m_dwLstSTSCIdx--;
                }

                ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->m_dwLstSTSCIdx, 0, &dwCurrSTSCChunkIdx);
                if (ret != GPP_SUCCESS)
                {
                    return ret;
                }
            }
            pTrak->m_dwLstSTSCChunkIdx--;

            // [dvyu] Get the chunk offset of pTrak->m_dwLstSTSCChunkIdx
            ret = sxaDmx3GPP_GetValueFromTableStream(&pStco->stcoEntry, pTrak->m_dwLstSTSCChunkIdx, 0, &pTrak->m_dwLstSTSCOffset);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }

            ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->m_dwLstSTSCIdx, 1, &pTrak->m_dwLstSTSCCount);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }

            for (i = 0; i < pTrak->m_dwLstSTSCCount; i++)
            {
                if (pStsz->dwSampleSize == 0)
                {
                    ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, pTrak->m_dwLstSampleIdx - pTrak->m_dwLstSTSCCount + i, 0, &dwSampleSize);
                    if (ret != GPP_SUCCESS)
                    {
                        return ret;
                    }
                    pTrak->m_dwLstSTSCOffset += dwSampleSize;
                }
                else
                {
                    pTrak->m_dwLstSTSCOffset += pStsz->dwSampleSize;
                }
            }
        }

        // Update param
        while (dwSampleDiff < 0)
        {
            if (pStsz->dwSampleSize == 0)
            {
                // Minus the sample size of each current index
                ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, pTrak->m_dwLstSampleIdx - 1, 0, &dwSampleSize);
                if (ret != GPP_SUCCESS)
                {
                    return ret;
                }
                pTrak->m_dwLstSTSCOffset -= dwSampleSize;
            }
            else
            {
                pTrak->m_dwLstSTSCOffset -= pStsz->dwSampleSize;
            }
            dwSampleDiff++;
            pTrak->m_dwLstSampleIdx--;
            pTrak->m_dwLstSTSCCount--;
        }
    }

    /*
    * decode size
    */

    /*
    * Output
    */
    // Time
    pInfo->nTimeStamp = pTrak->m_dwTimeStampBase * 1000 + (pTrak->m_dwTimeStampRemain * 1000) / pTrak->mdia.mdhd.dwTimeScale;
    // backward compatiable
    ret = sxaDmx3GPP_GetValueFromTableStream(&pStts->sttsEntry, pTrak->m_dwLstSTTSIdx, 1, (unsigned long *)&pInfo->nDelta);
    if (ret != GPP_SUCCESS)
    {
        return ret;
    }
    // ~backward compatiable

    // Offset
    {
        if (pStsz->dwSampleSize != 0)
        {
            pInfo->nOffset = pTrak->m_dwLstSTSCOffset - pStsz->dwSampleSize + gppOpenExParam.nOffset;
        }
        else
        {
            if (pTrak->m_dwLstSampleIdx == dwSampleIdx)
            {
                ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, pTrak->m_dwLstSampleIdx - 1, 0, &dwSampleSize);
                if (ret != GPP_SUCCESS)
                {
                    return ret;
                }
            }
            else
            {
                ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, pTrak->m_dwLstSampleIdx, 0, &dwSampleSize);
                if (ret != GPP_SUCCESS)
                {
                    return ret;
                }
            }
            pInfo->nOffset = pTrak->m_dwLstSTSCOffset - dwSampleSize + gppOpenExParam.nOffset;
        }
        //printf("offset info %d %d %d %d \n",pInfo->nOffset,pTrak->m_dwLstSTSCOffset,dwSampleSize,gppOpenExParam.nOffset);
    }

    // Size
#if (SUPPORT_NEW_PCM_INFO)
    if ((bIsAudio) &&
        ((nAudioType == RAW_SOUND_TYPE) || (nAudioType == ULAW_SOUND_TYPE) || (nAudioType == SOWT_SOUND_TYPE) || (nAudioType == TWOS_SOUND_TYPE)))
    {
        // When Audio Type is RAW, ULAW or SOWT
        nSliceSize = 0;
        nCurSmpIdx = dwSampleIdx;
        nCurSmpInChunk = pTrak->m_dwLstSTSCCount;
        // Get the 'sample per chunk' of current STSC entry
        ret = sxaDmx3GPP_GetValueFromTableStream(&pStsc->stscEntry, pTrak->m_dwLstSTSCIdx, 1, &dwSamplePerChunk);
        if (ret != GPP_SUCCESS)
        {
            return ret;
        }

        // Iterate each sample in this chunk
        while (nCurSmpInChunk <= dwSamplePerChunk)
        {
            // Get the size of current sample
            if (pStsz->dwSampleSize == 0)
            {
                ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, nCurSmpIdx-1, 0, &nSize);
                if (ret != GPP_SUCCESS)
                {
                    return ret;
                }
            }
            else
            {
                nSize = pStsz->dwSampleSize;
            }
            nSliceSize += nSize;

            // If nSliceSize reach the nMaxBytePerSlice, stop
            if (nSliceSize >= pTrak->nMaxBytePerSlice)
            {
                // Don't exceed the nMaxBytePerSlice
                if (nSliceSize > pTrak->nMaxBytePerSlice)
                {
                    nSliceSize -= nSize;
                }
                break;
            }
            else
            {
                nCurSmpIdx++;
                nCurSmpInChunk++;
            }
        }
        pInfo->nSize = nSliceSize;
    }
    else
    {
        // For Video or other Audio types
        if (pStsz->dwSampleSize == 0)
        {
            ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, dwSampleIdx - 1, 0, (unsigned long *)&pInfo->nSize);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }
        }
        else
        {
            pInfo->nSize = pStsz->dwSampleSize;
        }
    }
#else
    if (pStsz->dwSampleSize == 0)
    {
        ret = sxaDmx3GPP_GetValueFromTableStream(&pStsz->stszEntry, dwSampleIdx - 1, 0, (unsigned long *)&pInfo->nSize);
        if (ret != GPP_SUCCESS)
        {
            return ret;
        }
    }
    else
    {
        pInfo->nSize = pStsz->dwSampleSize;
    }
#endif

    // Update Param
    pTrak->m_dwLstSampleIdx = dwSampleIdx;

    // [dvyu] Update Seek's param
    pTrak->m_dwLstSeekIdx  = pTrak->m_dwLstSampleIdx;
    pTrak->sttsDeltaCount  = pTrak->m_dwLstSTTSCount;
    pTrak->sttsDeltaIdx    = pTrak->m_dwLstSTTSIdx;
    pTrak->sampleDeltaBase = pTrak->m_dwTimeStampBase;
    pTrak->sampleDeltaAcc  = pTrak->m_dwTimeStampRemain;

    // check the logical error of offset and size
    if (pInfo->nOffset > gppOpenExParam.nOffset + hDecGppData->dwFileSize)
    {
        //printf("[Sxa] offset error %d %d %d \n",pInfo->nOffset,gppOpenExParam.nOffset,hDecGppData->dwFileSize);
        
        return SXA_DMXECODE_ESAMPLE_OFFSET;
    }
    else if (pInfo->nOffset + pInfo->nSize > gppOpenExParam.nOffset + hDecGppData->dwFileSize)
    {
        return SXA_DMXECODE_ESAMPLE_OFFSET_SIZE;
    }

    return (GPP_SUCCESS);
}



SXA_DMXECODE_E
sxaDmx3GPP_GetVideoSampleInfo(
                         MMP_3GPP_DECODE_3GPPDATA*       hDecGppData,
                         SXA_DMXSAMPLEINFO_T*            pInfo )
{
    MMP_RESULT Ret;

    if (hDecGppData->pFirstVideoTrak == NULL)
    {
        return SXA_DMXECODE_EFAIL;
    }

    Ret = sxaDmx3GPP_GetSampleInfo(hDecGppData, hDecGppData->pFirstVideoTrak, pInfo);
    if (Ret == GPP_SUCCESS)
    {
        return SXA_DMXECODE_SOK;
    }
    else if (Ret == SXA_DMXECODE_ESAMPLE_OFFSET ||
             Ret == SXA_DMXECODE_ESAMPLE_OFFSET_SIZE)
    {
        return Ret;
    }
    else
    {
        return SXA_DMXECODE_EFAIL;
    }
}



SXA_DMXECODE_E
sxaDmx3GPP_GetAudioSampleInfo(
                         MMP_3GPP_DECODE_3GPPDATA*       hDecGppData,
                         SXA_DMXSAMPLEINFO_T*            pInfo )
{
    MMP_RESULT Ret;

    if (hDecGppData->pFirstAudioTrak == NULL)
    {
        return SXA_DMXECODE_EFAIL;
    }

    Ret = sxaDmx3GPP_GetSampleInfo(hDecGppData, hDecGppData->pFirstAudioTrak, pInfo);
    if (Ret == GPP_SUCCESS)
    {
        return SXA_DMXECODE_SOK;
    }
    else if (Ret == SXA_DMXECODE_ESAMPLE_OFFSET ||
             Ret == SXA_DMXECODE_ESAMPLE_OFFSET_SIZE)
    {
        return Ret;
    }
    else
    {
        return SXA_DMXECODE_EFAIL;
    }
}



GPP_RESULT
sxaDmx3GPP_SeekSampleInfoFromIdx(
                                 MMP_3GPP_DECODE_3GPPDATA*       hDecGppData,
                                 unsigned long                   uiVidSample,
                                 SXA_DMXSAMPLEINFO_T*            pVidInfo,
                                 SXA_DMXSAMPLEINFO_T*            pAudInfo)
{
    GPP_RESULT ret;
    /********************************************
    * Can make a new, single structure to store INFO
    ********************************************/
    // Pointer From Param
    MMP_3GPP_DECODE_TRAK* pVidTrak = hDecGppData->pFirstVideoTrak;
    MMP_3GPP_DECODE_TRAK* pAudTrak = hDecGppData->pFirstAudioTrak;

    MMP_3GPP_DECODE_STTS* pVidStts = &(pVidTrak->mdia.minf.stbl.stts);
    MMP_3GPP_DECODE_STTS* pAudStts = &(pAudTrak->mdia.minf.stbl.stts);
    MMP_3GPP_DECODE_STSZ* pAudStsz = &(pAudTrak->mdia.minf.stbl.stsz);

    unsigned int VidSampleCount;
    unsigned int AudSampleCount;

    // Local
    unsigned long  uiVidTimeStamp;
    unsigned long  uiAudTimeStamp;
    signed long    iLastAudTimeStamp, iTimeDiff;
    signed long    dwSampleDiff;
    signed long    dwSttsCount, dwSttsDuration, dwTempDeltaAcc;
    signed long    dwCount;
#if (SUPPORT_NEW_PCM_INFO)
    unsigned long  nAudioType = 0;
#endif
    unsigned short  nChannel = 1;
    unsigned long   nOriginalSize = 0;

#if (WATCHDOG_ENABLE)
    lastClock = PalGetClock();
#endif

    if (uiVidSample == 0)
    {
        pVidInfo->nIndex         = 0;
        pVidInfo->nTimeStamp     = 0;
        pAudInfo->nIndex         = 0;
        pAudInfo->nTimeStamp     = 0;
        return (GPP_SUCCESS);
    }

    VidSampleCount = 0;
    AudSampleCount = 0;

    if (pVidTrak != NULL)
    {
        sxaDmx3GPP_GetSampleCount(pVidTrak, &VidSampleCount);
    }

    if (pAudTrak != NULL)
    {
        sxaDmx3GPP_GetSampleCount(pAudTrak, &AudSampleCount);
#if (SUPPORT_NEW_PCM_INFO)
        nAudioType = pAudTrak->mdia.minf.stbl.stsd.dwEntryType;
#endif
    }

    if (pVidTrak->m_dwLstSeekIdx == 0)
    {
        pVidTrak->sttsDeltaCount   = 0;
        pVidTrak->sttsDeltaIdx     = 0;
        pVidTrak->sampleDeltaBase  = 0;
        pVidTrak->sampleDeltaAcc   = 0;
    }

    if (pAudTrak->m_dwLstSeekIdx == 0)
    {
        pAudTrak->sttsDeltaCount   = 0;
        pAudTrak->sttsDeltaIdx     = 0;
        pAudTrak->sampleDeltaBase  = 0;
        pAudTrak->sampleDeltaAcc   = 0;
    }

    // [dvyu] Seek Video
    dwSampleDiff = uiVidSample - pVidTrak->m_dwLstSeekIdx;

    if (dwSampleDiff >= 0)
    {
        // [dvyu] Get the number of sample in current STTS entry
        ret = sxaDmx3GPP_GetValueFromTableStream(&pVidStts->sttsEntry, pVidTrak->sttsDeltaIdx, 0, &dwSttsCount);
        if (ret != GPP_SUCCESS)
        {
            return ret;
        }

        while (pVidTrak->sttsDeltaCount + dwSampleDiff >= dwSttsCount)
        {
#if (WATCHDOG_ENABLE)
            CheckResetWatchDog(__LINE__);
#endif

            // [dvyu] Get the number of sample in current STTS entry
            ret = sxaDmx3GPP_GetValueFromTableStream(&pVidStts->sttsEntry, pVidTrak->sttsDeltaIdx, 0, &dwSttsCount);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }

            dwCount = dwSttsCount - pVidTrak->sttsDeltaCount;

            // [dvyu] Get the duration of current STTS entry
            ret = sxaDmx3GPP_GetValueFromTableStream(&pVidStts->sttsEntry, pVidTrak->sttsDeltaIdx, 1, &dwSttsDuration);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }

            pVidTrak->sampleDeltaAcc += dwSttsDuration * dwCount;

            // Update param
            if (pVidTrak->sttsDeltaIdx < pVidStts->dwEntryCount - 1 )
            {
                pVidTrak->sttsDeltaIdx++;
            }

            pVidTrak->sttsDeltaCount = 0;
            dwSampleDiff -= dwCount;

            // [dvyu] Get the number of sample in current STTS entry
            ret = sxaDmx3GPP_GetValueFromTableStream(&pVidStts->sttsEntry, pVidTrak->sttsDeltaIdx, 0, &dwSttsCount);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }
        }
#if (WATCHDOG_ENABLE)
        CheckResetWatchDog(__LINE__);
#endif

        // [dvyu] Get the duration of current STTS entry
        ret = sxaDmx3GPP_GetValueFromTableStream(&pVidStts->sttsEntry, pVidTrak->sttsDeltaIdx, 1, &dwSttsDuration);
        if (ret != GPP_SUCCESS)
        {
            return ret;
        }

        pVidTrak->sampleDeltaAcc += dwSttsDuration * dwSampleDiff;
        pVidTrak->sampleDeltaBase += pVidTrak->sampleDeltaAcc / pVidTrak->mdia.mdhd.dwTimeScale;
        pVidTrak->sampleDeltaAcc = pVidTrak->sampleDeltaAcc % pVidTrak->mdia.mdhd.dwTimeScale;

        // Update param
        pVidTrak->sttsDeltaCount += dwSampleDiff;
    }
    else	// [dvyu] dwSampleDiff < 0
    {
        while (pVidTrak->sttsDeltaCount + dwSampleDiff < 0)
        {
#if (WATCHDOG_ENABLE)
            CheckResetWatchDog(__LINE__);
#endif

            // [dvyu] Get the duration of current STTS entry
            ret = sxaDmx3GPP_GetValueFromTableStream(&pVidStts->sttsEntry, pVidTrak->sttsDeltaIdx, 1, &dwSttsDuration);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }

            pVidTrak->sampleDeltaAcc -= dwSttsDuration * pVidTrak->sttsDeltaCount;

            // Update param
            dwSampleDiff += pVidTrak->sttsDeltaCount;

            if (pVidTrak->sttsDeltaIdx > 0 )
            {
                pVidTrak->sttsDeltaIdx--;
            }

            ret = sxaDmx3GPP_GetValueFromTableStream(&pVidStts->sttsEntry, pVidTrak->sttsDeltaIdx, 0, &pVidTrak->sttsDeltaCount);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }
        }

        ret = sxaDmx3GPP_GetValueFromTableStream(&pVidStts->sttsEntry, pVidTrak->sttsDeltaIdx, 1, &dwSttsDuration);
        if (ret != GPP_SUCCESS)
        {
            return ret;
        }

        pVidTrak->sampleDeltaAcc += dwSttsDuration * dwSampleDiff;

        if (pVidTrak->sampleDeltaAcc < 0)
        {
            signed long qq, rr;

            qq = ABS(pVidTrak->sampleDeltaAcc) / (signed long)pVidTrak->mdia.mdhd.dwTimeScale;
            rr = ABS(pVidTrak->sampleDeltaAcc) % (signed long)pVidTrak->mdia.mdhd.dwTimeScale;

            if (rr == 0)
            {
                pVidTrak->sampleDeltaBase -= qq;
                pVidTrak->sampleDeltaAcc = rr;
            }
            else
            {
                pVidTrak->sampleDeltaBase -= (qq+1);
                pVidTrak->sampleDeltaAcc += (qq+1) * (signed long)pVidTrak->mdia.mdhd.dwTimeScale;
            }
        }
        else
        {
            pVidTrak->sampleDeltaBase += pVidTrak->sampleDeltaAcc / (signed long)pVidTrak->mdia.mdhd.dwTimeScale;
            pVidTrak->sampleDeltaAcc = pVidTrak->sampleDeltaAcc % (signed long)pVidTrak->mdia.mdhd.dwTimeScale;
        }

        // Update param
        pVidTrak->sttsDeltaCount += dwSampleDiff;
    }

    pVidTrak->m_dwLstSeekIdx = uiVidSample;
    uiVidTimeStamp = pVidTrak->sampleDeltaBase * 1000 + (pVidTrak->sampleDeltaAcc * 1000) / pVidTrak->mdia.mdhd.dwTimeScale;

    // Seek specify audio sample number from video timestamp
    if (pAudTrak == NULL)
    {
        AudSampleCount = 0;
        uiAudTimeStamp = 0;
        goto end;
    }

    if (nAudioType == ULAW_SOUND_TYPE)
    {
        nChannel = pAudTrak->mdia.minf.stbl.stsd.ulawEntry.wChannelNum;
        nOriginalSize = pAudStsz->dwOriginalSize;
    }

    iLastAudTimeStamp = pAudTrak->sampleDeltaBase * 1000 + (pAudTrak->sampleDeltaAcc * 1000) / pAudTrak->mdia.mdhd.dwTimeScale;
    iTimeDiff = (signed long)uiVidTimeStamp - iLastAudTimeStamp;

    dwTempDeltaAcc = 0;

    if (iTimeDiff >= 0)
    {
        while (pAudTrak->m_dwLstSeekIdx < AudSampleCount)
        {
#if (WATCHDOG_ENABLE)
            CheckResetWatchDog(__LINE__);
#endif

            ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 0, &dwSttsCount);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }

            if (dwSttsCount - pAudTrak->sttsDeltaCount == 0)
            {
                if (pAudTrak->sttsDeltaIdx < pAudStts->dwEntryCount - 1 )
                {
                    pAudTrak->sttsDeltaIdx++;        // [dvyu] STTS entry index
                }
                pAudTrak->sttsDeltaCount = 0;
            }
            pAudTrak->sttsDeltaCount++;
            pAudTrak->m_dwLstSeekIdx++;

            ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 1, &dwSttsDuration);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }

            if (nAudioType == ULAW_SOUND_TYPE)
            {
                //dwTempDeltaAcc += dwSttsDuration;
                //if (dwTempDeltaAcc / (nChannel / nOriginalSize) > 0)
                //{
                //    pAudTrak->sampleDeltaAcc += dwTempDeltaAcc / (nChannel / nOriginalSize);
                //    dwTempDeltaAcc = dwTempDeltaAcc % (nChannel / nOriginalSize);
                //}
                pAudTrak->sampleDeltaAcc += dwSttsDuration;
            }
            else
            {
                pAudTrak->sampleDeltaAcc += dwSttsDuration;
            }

            pAudTrak->sampleDeltaBase += pAudTrak->sampleDeltaAcc / pAudTrak->mdia.mdhd.dwTimeScale;
            pAudTrak->sampleDeltaAcc = pAudTrak->sampleDeltaAcc % pAudTrak->mdia.mdhd.dwTimeScale;

            uiAudTimeStamp = pAudTrak->sampleDeltaBase * 1000 + (pAudTrak->sampleDeltaAcc * 1000) / pAudTrak->mdia.mdhd.dwTimeScale;

            if (uiAudTimeStamp > uiVidTimeStamp)
            {
                // Not seek the nearest, just seek bigger, let avsyn cover the timedelay
                pVidInfo->nIndex     = uiVidSample;
                pVidInfo->nTimeStamp = uiVidTimeStamp;
                pAudInfo->nIndex     = pAudTrak->m_dwLstSeekIdx;
                pAudInfo->nTimeStamp = uiAudTimeStamp;
#if (SUPPORT_NEW_PCM_INFO)
                if ((nAudioType == RAW_SOUND_TYPE) || (nAudioType == ULAW_SOUND_TYPE) || (nAudioType == SOWT_SOUND_TYPE) || (nAudioType == TWOS_SOUND_TYPE))
                {
                    // When Audio type is RAW, ULAW or SOWT, need to seek the slice index
                    int TmpSmpIdx;

                    pAudInfo->nIndex = GetSliceIdxFromSampleIdx(pAudTrak, pAudInfo->nIndex);
                    if (pAudInfo->nIndex > 0)
                    {
                        // Correct the TimpStamp
                        TmpSmpIdx = GetSampleIdxFromSliceIdx(pAudTrak, pAudInfo->nIndex);
                        while (pAudTrak->m_dwLstSeekIdx > TmpSmpIdx)
                        {
#if (WATCHDOG_ENABLE)
                            CheckResetWatchDog(__LINE__);
#endif

                            if (pAudTrak->sttsDeltaCount == 0)
                            {
                                if (pAudTrak->sttsDeltaIdx > 0)
                                {
                                    pAudTrak->sttsDeltaIdx--;
                                }

                                ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 0, &pAudTrak->sttsDeltaCount);
                                if (ret != GPP_SUCCESS)
                                {
                                    return ret;
                                }
                            }
                            pAudTrak->sttsDeltaCount--;
                            pAudTrak->m_dwLstSeekIdx--;

                            ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 1, &dwSttsDuration);
                            if (ret != GPP_SUCCESS)
                            {
                                return ret;
                            }

                            if (nAudioType == ULAW_SOUND_TYPE)
                            {
                                //dwTempDeltaAcc -= dwSttsDuration;
                                //if (dwTempDeltaAcc / (signed long)(nChannel / nOriginalSize) < 0)
                                //{
                                //    pAudTrak->sampleDeltaAcc += dwTempDeltaAcc / (signed long)(nChannel / nOriginalSize);
                                //    dwTempDeltaAcc = dwTempDeltaAcc % (signed long)(nChannel / nOriginalSize);
                                //}
                                pAudTrak->sampleDeltaAcc -= dwSttsDuration;
                            }
                            else
                            {
                                pAudTrak->sampleDeltaAcc -= dwSttsDuration;
                            }

                            while (pAudTrak->sampleDeltaAcc < 0)
                            {
                                pAudTrak->sampleDeltaAcc += pAudTrak->mdia.mdhd.dwTimeScale;
                                pAudTrak->sampleDeltaBase -= 1;
                            }
                        }
                        uiAudTimeStamp = pAudTrak->sampleDeltaBase * 1000 + (pAudTrak->sampleDeltaAcc * 1000) / pAudTrak->mdia.mdhd.dwTimeScale;
                        pAudInfo->nTimeStamp = uiAudTimeStamp;
                        return (GPP_SUCCESS);
                    }
                }
                else
                {
                    // For other Audio types, jest return
                    return (GPP_SUCCESS);
                }
#else
                return (GPP_SUCCESS);
#endif
            }
        }
    }
    else
    {
        while (pAudTrak->m_dwLstSeekIdx > 0)
        {
#if (WATCHDOG_ENABLE)
            CheckResetWatchDog(__LINE__);
#endif

            if (pAudTrak->sttsDeltaCount == 0)
            {
                if (pAudTrak->sttsDeltaIdx > 0)
                {
                    pAudTrak->sttsDeltaIdx--;
                }

                ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 0, &pAudTrak->sttsDeltaCount);
                if (ret != GPP_SUCCESS)
                {
                    return ret;
                }
            }
            pAudTrak->sttsDeltaCount--;
            pAudTrak->m_dwLstSeekIdx--;

            ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 1, &dwSttsDuration);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }

            if (nAudioType == ULAW_SOUND_TYPE)
            {
                //dwTempDeltaAcc -= dwSttsDuration;
                //if (dwTempDeltaAcc / (signed long)(nChannel / nOriginalSize) < 0)
                //{
                //    pAudTrak->sampleDeltaAcc += dwTempDeltaAcc / (signed long)(nChannel / nOriginalSize);
                //    dwTempDeltaAcc = dwTempDeltaAcc % (signed long)(nChannel / nOriginalSize);
                //}
                pAudTrak->sampleDeltaAcc -= dwSttsDuration;
            }
            else
            {
                pAudTrak->sampleDeltaAcc -= dwSttsDuration;
            }

            if (pAudTrak->sampleDeltaAcc < 0)
            {
                signed long qq, rr;

                qq = ABS(pAudTrak->sampleDeltaAcc) / (signed long)pAudTrak->mdia.mdhd.dwTimeScale;
                rr = ABS(pAudTrak->sampleDeltaAcc) % (signed long)pAudTrak->mdia.mdhd.dwTimeScale;

                if (rr == 0)
                {
                    pAudTrak->sampleDeltaBase -= qq;
                    pAudTrak->sampleDeltaAcc = rr;
                }
                else
                {
                    pAudTrak->sampleDeltaBase -= (qq+1);
                    pAudTrak->sampleDeltaAcc += (qq+1) * (signed long)pAudTrak->mdia.mdhd.dwTimeScale;
                }
            }
            else
            {
                pAudTrak->sampleDeltaBase += pAudTrak->sampleDeltaAcc / (signed long)pAudTrak->mdia.mdhd.dwTimeScale;
                pAudTrak->sampleDeltaAcc = pAudTrak->sampleDeltaAcc % (signed long)pAudTrak->mdia.mdhd.dwTimeScale;
            }

            uiAudTimeStamp = pAudTrak->sampleDeltaBase * 1000 + (pAudTrak->sampleDeltaAcc * 1000) / pAudTrak->mdia.mdhd.dwTimeScale;

            if (uiAudTimeStamp < uiVidTimeStamp)
            {
                // Not seek the nearest, just seek bigger, let avsyn cover the timedelay
                // [dvyu] add one sample back
                pAudTrak->m_dwLstSeekIdx++;
                pAudTrak->sampleDeltaAcc += dwSttsDuration;
                pAudTrak->sampleDeltaBase += pAudTrak->sampleDeltaAcc / pAudTrak->mdia.mdhd.dwTimeScale;
                pAudTrak->sampleDeltaAcc = pAudTrak->sampleDeltaAcc % pAudTrak->mdia.mdhd.dwTimeScale;
                uiAudTimeStamp = pAudTrak->sampleDeltaBase * 1000 + (pAudTrak->sampleDeltaAcc * 1000) / pAudTrak->mdia.mdhd.dwTimeScale;

                pVidInfo->nIndex     = uiVidSample;
                pVidInfo->nTimeStamp = uiVidTimeStamp;
                pAudInfo->nIndex     = pAudTrak->m_dwLstSeekIdx;
                pAudInfo->nTimeStamp = uiAudTimeStamp;
#if (SUPPORT_NEW_PCM_INFO)
                if ((nAudioType == RAW_SOUND_TYPE) || (nAudioType == ULAW_SOUND_TYPE) || (nAudioType == SOWT_SOUND_TYPE) || (nAudioType == TWOS_SOUND_TYPE))
                {
                    // When Audio type is RAW, ULAW or SOWT, need to seek the slice index
                    int TmpSmpIdx;

                    pAudInfo->nIndex = GetSliceIdxFromSampleIdx(pAudTrak, pAudInfo->nIndex);
                    if (pAudInfo->nIndex > 0)
                    {
                        // Correct the TimpStamp
                        TmpSmpIdx = GetSampleIdxFromSliceIdx(pAudTrak, pAudInfo->nIndex);
                        while (pAudTrak->m_dwLstSeekIdx > TmpSmpIdx)
                        {
#if (WATCHDOG_ENABLE)
                            CheckResetWatchDog(__LINE__);
#endif

                            if (pAudTrak->sttsDeltaCount == 0)
                            {
                                if (pAudTrak->sttsDeltaIdx > 0)
                                {
                                    pAudTrak->sttsDeltaIdx--;
                                }

                                ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 0, &pAudTrak->sttsDeltaCount);
                                if (ret != GPP_SUCCESS)
                                {
                                    return ret;
                                }
                            }
                            pAudTrak->sttsDeltaCount--;
                            pAudTrak->m_dwLstSeekIdx--;

                            ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 1, &dwSttsDuration);
                            if (ret != GPP_SUCCESS)
                            {
                                return ret;
                            }

                            if (nAudioType == ULAW_SOUND_TYPE)
                            {
                                //dwTempDeltaAcc -= dwSttsDuration;
                                //if (dwTempDeltaAcc / (signed long)(nChannel / nOriginalSize) < 0)
                                //{
                                //    pAudTrak->sampleDeltaAcc += dwTempDeltaAcc / (signed long)(nChannel / nOriginalSize);
                                //    dwTempDeltaAcc = dwTempDeltaAcc % (signed long)(nChannel / nOriginalSize);
                                //}
                                pAudTrak->sampleDeltaAcc -= dwSttsDuration;
                            }
                            else
                            {
                                pAudTrak->sampleDeltaAcc -= dwSttsDuration;
                            }

                            while (pAudTrak->sampleDeltaAcc < 0)
                            {
#if (WATCHDOG_ENABLE)
                                CheckResetWatchDog(__LINE__);
#endif

                                pAudTrak->sampleDeltaAcc += pAudTrak->mdia.mdhd.dwTimeScale;
                                pAudTrak->sampleDeltaBase -= 1;
                            }
                        }
                        uiAudTimeStamp = pAudTrak->sampleDeltaBase * 1000 + (pAudTrak->sampleDeltaAcc * 1000) / pAudTrak->mdia.mdhd.dwTimeScale;
                        pAudInfo->nTimeStamp = uiAudTimeStamp;
                        return (GPP_SUCCESS);
                    }
                }
                else
                {
                    // For other Audio types, jest return
                    return (GPP_SUCCESS);
                }
#else
                return (GPP_SUCCESS);
#endif
            }
        }
    }

    // Can't seek the proper sample
    uiAudTimeStamp = pAudTrak->sampleDeltaBase * 1000 + (pAudTrak->sampleDeltaAcc * 1000) / pAudTrak->mdia.mdhd.dwTimeScale;

end:
    pVidInfo->nIndex     = uiVidSample;
    pVidInfo->nTimeStamp = uiVidTimeStamp;
    pAudInfo->nIndex     = AudSampleCount;
    pAudInfo->nTimeStamp = uiAudTimeStamp;

    return (GPP_SUCCESS);
}



GPP_RESULT
sxaDmx3GPP_SeekSampleInfoFromTime(
                                  MMP_3GPP_DECODE_3GPPDATA*       hDecGppData,
                                  unsigned long                   uiTime,
                                  SXA_DMXSAMPLEINFO_T*            pVidInfo,
                                  SXA_DMXSAMPLEINFO_T*            pAudInfo)
{
    GPP_RESULT ret;
    /********************************************
    * Can make a new, single structure to store INFO
    ********************************************/
    // Pointer From Param
    MMP_3GPP_DECODE_TRAK* pVidTrak = hDecGppData->pFirstVideoTrak;
    MMP_3GPP_DECODE_TRAK* pAudTrak = hDecGppData->pFirstAudioTrak;

    MMP_3GPP_DECODE_STTS  *pVidStts = &(pVidTrak->mdia.minf.stbl.stts);
    MMP_3GPP_DECODE_STTS  *pAudStts = &(pAudTrak->mdia.minf.stbl.stts);
    MMP_3GPP_DECODE_STSZ  *pAudStsz = &(pAudTrak->mdia.minf.stbl.stsz);

    unsigned int  VidSampleCount;
    unsigned int  AudSampleCount;

    // Local
    unsigned long  uiVidTimeStamp = 0;
    unsigned long  uiAudTimeStamp = 0;
    MMP_BOOL       bfind = 0;

    signed long    iLastVidTimeStamp, iLastAudTimeStamp, iTimeDiff;
    signed long    dwSttsCount, dwSttsDuration, dwTempDeltaAcc;
#if (SUPPORT_NEW_PCM_INFO)
    unsigned long  nAudioType = 0;
#endif
    unsigned short  nChannel = 1;
    unsigned long   nOriginalSize = 0;

#if (WATCHDOG_ENABLE)
    lastClock = PalGetClock();
#endif

    VidSampleCount = 0;
    AudSampleCount = 0;

    if (pVidTrak != NULL)
    {
        sxaDmx3GPP_GetSampleCount(pVidTrak, &VidSampleCount);
    }

    if (pAudTrak != NULL)
    {
        sxaDmx3GPP_GetSampleCount(pAudTrak, &AudSampleCount);
#if (SUPPORT_NEW_PCM_INFO)
        nAudioType = pAudTrak->mdia.minf.stbl.stsd.dwEntryType;
#endif
    }

	if (pVidTrak->m_dwLstSeekIdx == 0)
	{
	    pVidTrak->sttsDeltaCount   = 0;
	    pVidTrak->sttsDeltaIdx     = 0;
	    pVidTrak->sampleDeltaBase  = 0;
	    pVidTrak->sampleDeltaAcc   = 0;
	}

	if (pAudTrak->m_dwLstSeekIdx == 0)
	{
	    pAudTrak->sttsDeltaCount   = 0;
	    pAudTrak->sttsDeltaIdx     = 0;
	    pAudTrak->sampleDeltaBase  = 0;
	    pAudTrak->sampleDeltaAcc   = 0;
	}

	iLastVidTimeStamp = pVidTrak->sampleDeltaBase * 1000 + (pVidTrak->sampleDeltaAcc * 1000) / pVidTrak->mdia.mdhd.dwTimeScale;
	iLastAudTimeStamp = pAudTrak->sampleDeltaBase * 1000 + (pAudTrak->sampleDeltaAcc * 1000) / pAudTrak->mdia.mdhd.dwTimeScale;

	// [dvyu] Seek Video
	iTimeDiff = (signed long)uiTime - iLastVidTimeStamp;

	if (iTimeDiff >= 0)
	{
		while (pVidTrak->m_dwLstSeekIdx < VidSampleCount)
		{
#if (WATCHDOG_ENABLE)
            CheckResetWatchDog(__LINE__);
#endif
            ret = sxaDmx3GPP_GetValueFromTableStream(&pVidStts->sttsEntry, pVidTrak->sttsDeltaIdx, 0, &dwSttsCount);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }

		    if (dwSttsCount - pVidTrak->sttsDeltaCount == 0)
	        {
	        	if (pVidTrak->sttsDeltaIdx < pVidStts->dwEntryCount - 1 )
	        	{
	            	pVidTrak->sttsDeltaIdx++;        // [dvyu] STTS entry index
	        	}
   	            pVidTrak->sttsDeltaCount = 0;
			}
        	pVidTrak->sttsDeltaCount++;
			pVidTrak->m_dwLstSeekIdx++;

	        ret = sxaDmx3GPP_GetValueFromTableStream(&pVidStts->sttsEntry, pVidTrak->sttsDeltaIdx, 1, &dwSttsDuration);
	        if (ret != GPP_SUCCESS)
	        {
	            return ret;
	        }
	        pVidTrak->sampleDeltaAcc += dwSttsDuration;

			pVidTrak->sampleDeltaBase += pVidTrak->sampleDeltaAcc / pVidTrak->mdia.mdhd.dwTimeScale;
			pVidTrak->sampleDeltaAcc = pVidTrak->sampleDeltaAcc % pVidTrak->mdia.mdhd.dwTimeScale;

	        uiVidTimeStamp = pVidTrak->sampleDeltaBase * 1000 + (pVidTrak->sampleDeltaAcc * 1000) / pVidTrak->mdia.mdhd.dwTimeScale;

	        if (uiVidTimeStamp > uiTime)
	        {
	            pVidInfo->nIndex = pVidTrak->m_dwLstSeekIdx;
	            pVidInfo->nTimeStamp = uiVidTimeStamp;
				bfind = MMP_TRUE;
				break;
	        }
		}

	    if (bfind == MMP_FALSE)
	    {
	        pVidInfo->nIndex = VidSampleCount;
	        pVidInfo->nTimeStamp = uiVidTimeStamp;
	    }
	}
	else
	{
		while (pVidTrak->m_dwLstSeekIdx > 0)
		{
#if (WATCHDOG_ENABLE)
            CheckResetWatchDog(__LINE__);
#endif
			if (pVidTrak->sttsDeltaCount == 0)
	        {
	        	if (pVidTrak->sttsDeltaIdx > 0)
        		{
        			pVidTrak->sttsDeltaIdx--;
        		}

				ret = sxaDmx3GPP_GetValueFromTableStream(&pVidStts->sttsEntry, pVidTrak->sttsDeltaIdx, 0, &pVidTrak->sttsDeltaCount);
	            if (ret != GPP_SUCCESS)
	            {
	                return ret;
	            }
			}
			pVidTrak->sttsDeltaCount--;
			pVidTrak->m_dwLstSeekIdx--;

	        ret = sxaDmx3GPP_GetValueFromTableStream(&pVidStts->sttsEntry, pVidTrak->sttsDeltaIdx, 1, &dwSttsDuration);
	        if (ret != GPP_SUCCESS)
	        {
	            return ret;
	        }
	        pVidTrak->sampleDeltaAcc -= dwSttsDuration;

            if (pVidTrak->sampleDeltaAcc < 0)
            {
                signed long qq, rr;

                qq = ABS(pVidTrak->sampleDeltaAcc) / (signed long)pVidTrak->mdia.mdhd.dwTimeScale;
                rr = ABS(pVidTrak->sampleDeltaAcc) % (signed long)pVidTrak->mdia.mdhd.dwTimeScale;

                if (rr == 0)
                {
                    pVidTrak->sampleDeltaBase -= qq;
                    pVidTrak->sampleDeltaAcc = rr;
                }
                else
                {
                    pVidTrak->sampleDeltaBase -= (qq+1);
                    pVidTrak->sampleDeltaAcc += (qq+1) * (signed long)pVidTrak->mdia.mdhd.dwTimeScale;
                }
            }
            else
            {
                pVidTrak->sampleDeltaBase += pVidTrak->sampleDeltaAcc / (signed long)pVidTrak->mdia.mdhd.dwTimeScale;
                pVidTrak->sampleDeltaAcc = pVidTrak->sampleDeltaAcc % (signed long)pVidTrak->mdia.mdhd.dwTimeScale;
            }

	        uiVidTimeStamp = pVidTrak->sampleDeltaBase * 1000 + (pVidTrak->sampleDeltaAcc * 1000) / pVidTrak->mdia.mdhd.dwTimeScale;

	        if (uiVidTimeStamp < uiTime)
	        {
	            // [dvyu] add one sample back
				pVidTrak->m_dwLstSeekIdx++;
				pVidTrak->sampleDeltaAcc += dwSttsDuration;
				pVidTrak->sampleDeltaBase += pVidTrak->sampleDeltaAcc / pVidTrak->mdia.mdhd.dwTimeScale;
				pVidTrak->sampleDeltaAcc = pVidTrak->sampleDeltaAcc % pVidTrak->mdia.mdhd.dwTimeScale;
				uiVidTimeStamp = pVidTrak->sampleDeltaBase * 1000 + (pVidTrak->sampleDeltaAcc * 1000) / pAudTrak->mdia.mdhd.dwTimeScale;

	            pVidInfo->nIndex = pVidTrak->m_dwLstSeekIdx;
	            pVidInfo->nTimeStamp = uiVidTimeStamp;
				bfind = MMP_TRUE;
	            break;
	        }
		}

	    if (bfind == MMP_FALSE)
	    {
	        pVidInfo->nIndex = 0;
			pVidInfo->nTimeStamp = 0;
	    }
	}

	// [dvyu] Seek Audio
    if (pAudTrak == NULL)
    {
        pAudInfo->nIndex = 0;
        pAudInfo->nTimeStamp = 0;
        goto end;
    }

    if (nAudioType == ULAW_SOUND_TYPE)
    {
        nChannel = pAudTrak->mdia.minf.stbl.stsd.ulawEntry.wChannelNum;
        nOriginalSize = pAudStsz->dwOriginalSize;
    }

    dwTempDeltaAcc = 0;
    bfind = MMP_FALSE;
    iTimeDiff = (signed long)uiTime - iLastAudTimeStamp;

	if (iTimeDiff >= 0)
	{
		while (pAudTrak->m_dwLstSeekIdx < AudSampleCount)
		{
#if (WATCHDOG_ENABLE)
            CheckResetWatchDog(__LINE__);
#endif
            ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 0, &dwSttsCount);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }

		    if (dwSttsCount - pAudTrak->sttsDeltaCount == 0)
	        {
	        	if (pAudTrak->sttsDeltaIdx < pAudStts->dwEntryCount - 1 )
	        	{
	            	pAudTrak->sttsDeltaIdx++;        // [dvyu] STTS entry index
	        	}
   	            pAudTrak->sttsDeltaCount = 0;
			}
        	pAudTrak->sttsDeltaCount++;
			pAudTrak->m_dwLstSeekIdx++;

	        ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 1, &dwSttsDuration);
	        if (ret != GPP_SUCCESS)
	        {
	            return ret;
	        }

            if (nAudioType == ULAW_SOUND_TYPE)
            {
                //dwTempDeltaAcc += dwSttsDuration;
                //if (dwTempDeltaAcc / (nChannel / nOriginalSize) > 0)
                //{
                //    pAudTrak->sampleDeltaAcc += dwTempDeltaAcc / (nChannel / nOriginalSize);
                //    dwTempDeltaAcc = dwTempDeltaAcc % (nChannel / nOriginalSize);
                //}
                pAudTrak->sampleDeltaAcc += dwSttsDuration;
            }
            else
            {
                pAudTrak->sampleDeltaAcc += dwSttsDuration;
            }

			pAudTrak->sampleDeltaBase += pAudTrak->sampleDeltaAcc / pAudTrak->mdia.mdhd.dwTimeScale;
			pAudTrak->sampleDeltaAcc = pAudTrak->sampleDeltaAcc % pAudTrak->mdia.mdhd.dwTimeScale;

	        uiAudTimeStamp = pAudTrak->sampleDeltaBase * 1000 + (pAudTrak->sampleDeltaAcc * 1000) / pAudTrak->mdia.mdhd.dwTimeScale;

	        if (uiAudTimeStamp > uiTime)
	        {
	            pAudInfo->nIndex = pAudTrak->m_dwLstSeekIdx;
	            pAudInfo->nTimeStamp = uiAudTimeStamp;
				bfind = MMP_TRUE;
#if (SUPPORT_NEW_PCM_INFO)
                if ((nAudioType == RAW_SOUND_TYPE) || (nAudioType == ULAW_SOUND_TYPE) || (nAudioType == SOWT_SOUND_TYPE) || (nAudioType == TWOS_SOUND_TYPE))
                {
                    // When Audio type is RAW, ULAW or SOWT, need to seek the slice index
                    int TmpSmpIdx;

                    pAudInfo->nIndex = GetSliceIdxFromSampleIdx(pAudTrak, pAudInfo->nIndex);
                    if (pAudInfo->nIndex > 0)
                    {
                        // Correct the TimpStamp
                        TmpSmpIdx = GetSampleIdxFromSliceIdx(pAudTrak, pAudInfo->nIndex);
                        while (pAudTrak->m_dwLstSeekIdx > TmpSmpIdx)
                        {
#if (WATCHDOG_ENABLE)
                            CheckResetWatchDog(__LINE__);
#endif
                            if (pAudTrak->sttsDeltaCount == 0)
                            {
                                if (pAudTrak->sttsDeltaIdx > 0)
                                {
                                    pAudTrak->sttsDeltaIdx--;
                                }

                                ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 0, &pAudTrak->sttsDeltaCount);
                                if (ret != GPP_SUCCESS)
                                {
                                    return ret;
                                }
                            }
                            pAudTrak->sttsDeltaCount--;
                            pAudTrak->m_dwLstSeekIdx--;

                            ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 1, &dwSttsDuration);
                            if (ret != GPP_SUCCESS)
                            {
                                return ret;
                            }

                            if (nAudioType == ULAW_SOUND_TYPE)
                            {
                                //dwTempDeltaAcc -= dwSttsDuration;
                                //if (dwTempDeltaAcc / (signed long)(nChannel / nOriginalSize) < 0)
                                //{
                                //    pAudTrak->sampleDeltaAcc += dwTempDeltaAcc / (signed long)(nChannel / nOriginalSize);
                                //    dwTempDeltaAcc = dwTempDeltaAcc % (signed long)(nChannel / nOriginalSize);
                                //}
                                pAudTrak->sampleDeltaAcc -= dwSttsDuration;
                            }
                            else
                            {
                                pAudTrak->sampleDeltaAcc -= dwSttsDuration;
                            }

                            while (pAudTrak->sampleDeltaAcc < 0)
                            {
#if (WATCHDOG_ENABLE)
                                CheckResetWatchDog(__LINE__);
#endif
                                pAudTrak->sampleDeltaAcc += pAudTrak->mdia.mdhd.dwTimeScale;
                                pAudTrak->sampleDeltaBase -= 1;
                            }
                        }
                        uiAudTimeStamp = pAudTrak->sampleDeltaBase * 1000 + (pAudTrak->sampleDeltaAcc * 1000) / pAudTrak->mdia.mdhd.dwTimeScale;
                        pAudInfo->nTimeStamp = uiAudTimeStamp;
                        break;
                    }
                }
                else
                {
                    // For other Audio types, jest break
                    break;
                }
#else
                break;
#endif
	        }
		}

	    if (bfind == MMP_FALSE)
	    {
	        pAudInfo->nIndex = AudSampleCount;
	        pAudInfo->nTimeStamp = uiAudTimeStamp;
	    }
	}
	else
	{
		while (pAudTrak->m_dwLstSeekIdx > 0)
		{
#if (WATCHDOG_ENABLE)
            CheckResetWatchDog(__LINE__);
#endif
			if (pAudTrak->sttsDeltaCount == 0)
	        {
	        	if (pAudTrak->sttsDeltaIdx > 0)
        		{
        			pAudTrak->sttsDeltaIdx--;
        		}

				ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 0, &pAudTrak->sttsDeltaCount);
	            if (ret != GPP_SUCCESS)
	            {
	                return ret;
	            }
			}
			pAudTrak->sttsDeltaCount--;
			pAudTrak->m_dwLstSeekIdx--;

	        ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 1, &dwSttsDuration);
	        if (ret != GPP_SUCCESS)
	        {
	            return ret;
	        }

            if (nAudioType == ULAW_SOUND_TYPE)
            {
                //dwTempDeltaAcc -= dwSttsDuration;
                //if (dwTempDeltaAcc / (signed long)(nChannel / nOriginalSize) < 0)
                //{
                //    pAudTrak->sampleDeltaAcc += dwTempDeltaAcc / (signed long)(nChannel / nOriginalSize);
                //    dwTempDeltaAcc = dwTempDeltaAcc % (signed long)(nChannel / nOriginalSize);
                //}
                pAudTrak->sampleDeltaAcc -= dwSttsDuration;
            }
            else
            {
                pAudTrak->sampleDeltaAcc -= dwSttsDuration;
            }

            if (pAudTrak->sampleDeltaAcc < 0)
            {
                signed long qq, rr;

                qq = ABS(pAudTrak->sampleDeltaAcc) / (signed long)pAudTrak->mdia.mdhd.dwTimeScale;
                rr = ABS(pAudTrak->sampleDeltaAcc) % (signed long)pAudTrak->mdia.mdhd.dwTimeScale;

                if (rr == 0)
                {
                    pAudTrak->sampleDeltaBase -= qq;
                    pAudTrak->sampleDeltaAcc = rr;
                }
                else
                {
                    pAudTrak->sampleDeltaBase -= (qq+1);
                    pAudTrak->sampleDeltaAcc += (qq+1) * (signed long)pAudTrak->mdia.mdhd.dwTimeScale;
                }
            }
            else
            {
                pAudTrak->sampleDeltaBase += pAudTrak->sampleDeltaAcc / (signed long)pAudTrak->mdia.mdhd.dwTimeScale;
                pAudTrak->sampleDeltaAcc = pAudTrak->sampleDeltaAcc % (signed long)pAudTrak->mdia.mdhd.dwTimeScale;
            }

	        uiAudTimeStamp = pAudTrak->sampleDeltaBase * 1000 + (pAudTrak->sampleDeltaAcc * 1000) / pAudTrak->mdia.mdhd.dwTimeScale;

	        if (uiAudTimeStamp < uiTime)
	        {
	            // [dvyu] add one sample back
				pAudTrak->m_dwLstSeekIdx++;
				pAudTrak->sampleDeltaAcc += dwSttsDuration;
				pAudTrak->sampleDeltaBase += pAudTrak->sampleDeltaAcc / pAudTrak->mdia.mdhd.dwTimeScale;
				pAudTrak->sampleDeltaAcc = pAudTrak->sampleDeltaAcc % pAudTrak->mdia.mdhd.dwTimeScale;
				uiAudTimeStamp = pAudTrak->sampleDeltaBase * 1000 + (pAudTrak->sampleDeltaAcc * 1000) / pAudTrak->mdia.mdhd.dwTimeScale;

	            pAudInfo->nIndex = pAudTrak->m_dwLstSeekIdx;
	            pAudInfo->nTimeStamp = uiAudTimeStamp;
				bfind = MMP_TRUE;
#if (SUPPORT_NEW_PCM_INFO)
                if ((nAudioType == RAW_SOUND_TYPE) || (nAudioType == ULAW_SOUND_TYPE) || (nAudioType == SOWT_SOUND_TYPE) || (nAudioType == TWOS_SOUND_TYPE))
                {
                    // When Audio type is RAW, ULAW or SOWT, need to seek the slice index
                    int TmpSmpIdx;

                    pAudInfo->nIndex = GetSliceIdxFromSampleIdx(pAudTrak, pAudInfo->nIndex);
                    if (pAudInfo->nIndex > 0)
                    {
                        // Correct the TimpStamp
                        TmpSmpIdx = GetSampleIdxFromSliceIdx(pAudTrak, pAudInfo->nIndex);
                        while (pAudTrak->m_dwLstSeekIdx > TmpSmpIdx)
                        {
#if (WATCHDOG_ENABLE)
                            CheckResetWatchDog(__LINE__);
#endif
                            if (pAudTrak->sttsDeltaCount == 0)
                            {
                                if (pAudTrak->sttsDeltaIdx > 0)
                                {
                                    pAudTrak->sttsDeltaIdx--;
                                }

                                ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 0, &pAudTrak->sttsDeltaCount);
                                if (ret != GPP_SUCCESS)
                                {
                                    return ret;
                                }
                            }
                            pAudTrak->sttsDeltaCount--;
                            pAudTrak->m_dwLstSeekIdx--;

                            ret = sxaDmx3GPP_GetValueFromTableStream(&pAudStts->sttsEntry, pAudTrak->sttsDeltaIdx, 1, &dwSttsDuration);
                            if (ret != GPP_SUCCESS)
                            {
                                return ret;
                            }

                            if (nAudioType == ULAW_SOUND_TYPE)
                            {
                                //dwTempDeltaAcc -= dwSttsDuration;
                                //if (dwTempDeltaAcc / (signed long)(nChannel / nOriginalSize) < 0)
                                //{
                                //    pAudTrak->sampleDeltaAcc += dwTempDeltaAcc / (signed long)(nChannel / nOriginalSize);
                                //    dwTempDeltaAcc = dwTempDeltaAcc % (signed long)(nChannel / nOriginalSize);
                                //}
                                pAudTrak->sampleDeltaAcc -= dwSttsDuration;
                            }
                            else
                            {
                                pAudTrak->sampleDeltaAcc -= dwSttsDuration;
                            }

                            while (pAudTrak->sampleDeltaAcc < 0)
                            {
#if (WATCHDOG_ENABLE)
                                CheckResetWatchDog(__LINE__);
#endif
                                pAudTrak->sampleDeltaAcc += pAudTrak->mdia.mdhd.dwTimeScale;
                                pAudTrak->sampleDeltaBase -= 1;
                            }
                        }
                        uiAudTimeStamp = pAudTrak->sampleDeltaBase * 1000 + (pAudTrak->sampleDeltaAcc * 1000) / pAudTrak->mdia.mdhd.dwTimeScale;
                        pAudInfo->nTimeStamp = uiAudTimeStamp;
                        break;
                    }
                }
                else
                {
                    // For other Audio types, jest break
                    break;
                }
#else
                break;
#endif
	        }
		}

	    if (bfind == MMP_FALSE)
	    {
	        pAudInfo->nIndex = 0;
	        pAudInfo->nTimeStamp = 0;
	    }
	}

end:
    return (GPP_SUCCESS);
}



//=============================================================================
//                              Private Function Definition
//=============================================================================
static unsigned long Log2BitNum(unsigned long value)
{
    unsigned long n = 0;

    while (value)
    {
        value >>= 1;
        n++;
    }
    return n;
}



//=============================================================================
//  GPP_RESULT GetH263SampleEntry
//
// Description:
//      Get h263 sample entry
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetH263SampleEntry(
                              unsigned char *pReadPtr,
                              unsigned long  dwBoxSize,
                              MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_H263ENTRY* h263Entry = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd.h263Entry);

    //   MMP_UINT8 *pReadPtrEnd = pReadPtr + dwBoxSize - 8;

    h263Entry->dwEntryType = H263_VIDEO_TYPE;

    // [dvyu] 3GPP TS 26.244, Sec.6.6

    // skip reserved_6 [byte]
    SKIP_BYTES(pReadPtr, 6);

    // get Data-reference-index
    GET_WORD_VALUE(pReadPtr, h263Entry->wDataRefIdx);

    // skip reserved_16 [byte]
    SKIP_DWORDS(pReadPtr, 4);

    // get width
    GET_WORD_VALUE(pReadPtr, h263Entry->wWidth);
    // get height
    GET_WORD_VALUE(pReadPtr, h263Entry->wHeight);

    // skip reserved_4 = 0x00480000 [byte]
    //      reserved_4 = 0x00480000 [byte]
    //      reserved_4 = 0
    //      reserved_2 = 1
    //      reserved_32= 0
    //      reserved_2 = 24
    //      reserved_2 = -1
    SKIP_BYTES(pReadPtr, 50);

    //
    // get H263 Specific Box
    //
    // [dvyu] 3GPP TS 26.244, Sec.6.8

    // skip size & type
    SKIP_DWORDS(pReadPtr, 2);

    // skip vender & decoder_version
    SKIP_DWORD(pReadPtr);
    SKIP_BYTE(pReadPtr);

    // get H263 specific box
    GET_BYTE_VALUE(pReadPtr, h263Entry->h263spec.Level);
    GET_BYTE_VALUE(pReadPtr, h263Entry->h263spec.Profile);

    // check box end
    //   if (pReadPtr != pReadPtrEnd)
    //   {
    //       return GPP_ERROR_DECODE_GET_H263_SAMPLE_ENTRY;
    //   }

    return GPP_SUCCESS;
}



static GPP_RESULT
sxaDmx3GPP_GetJPEGSampleEntry(
                              unsigned char *pReadPtr,
                              unsigned long  dwBoxSize,
                              MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_JPEGENTRY* jpegEntry = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd.jpegEntry);

    jpegEntry->dwEntryType = JPEG_VIDEO_TYPE;

    // skip reserved_6 [byte]
    SKIP_BYTES(pReadPtr, 6);

    // get Data-reference-index
    GET_WORD_VALUE(pReadPtr, jpegEntry->wDataRefIdx);

    // skip 4 bytes for Version and Revision level
    SKIP_BYTES(pReadPtr, 4);

    // skip 4 bytes for Vendor
    SKIP_BYTES(pReadPtr, 4);

    // skip 2 dwords for Temporal and Spatial quality
    SKIP_DWORDS(pReadPtr, 2);

    // get width
    GET_WORD_VALUE(pReadPtr, jpegEntry->wWidth);
    // get height
    GET_WORD_VALUE(pReadPtr, jpegEntry->wHeight);

    // TO DO
    SKIP_BYTES(pReadPtr, 22);

    //printf("GPP_DECODE_GetJPEGSampleEntry %d %d\n", jpegEntry->wWidth, jpegEntry->wHeight);

    return GPP_SUCCESS;
}



//=============================================================================
//  GPP_RESULT GPP_DECODE_GetDecoderSpecificInfo
//
// Description:
//
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetDecoderSpecificInfo(
                                  MMP_3GPP_DECODE_MP4AESDBOX *pESDBox,
                                  unsigned long               size,
                                  unsigned char              *pReadPtr)
{
    unsigned long  samplingFrequency                  = 0;
    unsigned char  extensionSamplingFrequencyIndex    = 0;
    unsigned long  extensionSamplingFrequency         = 0;
    unsigned char  extensionAudioObjectType           = 0;
    BIT_STREAM     esdbitstream                       = {0};
    BIT_STREAM     *bs                                = &esdbitstream;
    unsigned char  *tempBuf                           = NULL;

    // [dvyu] IEC_100_1384_DC.pdf, 4.7.1.4 DecoderSpecificInfo
    // [dvyu] ISO-IEC_14496-3, 3.1.1 Audio DecoderSpecificInfo

    SKIP_BYTES(pReadPtr, 1);
    size -=1;
    if (pReadPtr[0] == 0x80 &&  pReadPtr[1] == 0x80 && pReadPtr[2] == 0x80)
    {
        SKIP_BYTES(pReadPtr, 3);
        size -= 3;
    }
    SKIP_BYTES(pReadPtr, 1);
    size -=1;

    if ((unsigned long)pReadPtr % 4)
    {
        tempBuf = PalHeapAlloc(PAL_HEAP_DEFAULT, size);
        if (tempBuf == NULL)
        {
            return GPP_ERROR_DECODE_MALLOC;
        }
        PalMemcpy(tempBuf, pReadPtr, size);
        sxaDmx3GPP_BitstreamInit(bs,(unsigned long *)tempBuf, size);
    }
    else
    {
        sxaDmx3GPP_BitstreamInit(bs,(unsigned long *)pReadPtr, size);
    }

    //audioObjectType
    pESDBox->audioObjectType = (unsigned char)getAudioObjectType(bs);
    //samplingFrequencyIndex
    pESDBox->samplingFrequencyIndex = (unsigned char)BitstreamGetBits(bs, 4);
    if (pESDBox->samplingFrequencyIndex == 0x0f)
    {
        samplingFrequency = BitstreamGetBits(bs, 24);
    }
    // channelConfiguration
    pESDBox->channelConfiguration = (unsigned char)BitstreamGetBits(bs, 4);
    if (pESDBox->audioObjectType == 5)
    {
        extensionAudioObjectType = pESDBox->audioObjectType;
        //extensionSamplingFrequencyIndex
        extensionSamplingFrequencyIndex = (unsigned char)BitstreamGetBits(bs, 4);
        if (extensionSamplingFrequencyIndex == 0x0f)
        {
            extensionSamplingFrequency = BitstreamGetBits(bs, 24);
        }
        pESDBox->audioObjectType = (unsigned char)getAudioObjectType(bs);
    }
    else
    {
        extensionAudioObjectType = 0;
    }

    if (tempBuf)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, tempBuf);
    }
    return GPP_SUCCESS;
}



//=============================================================================
//  GPP_RESULT GPP_DECODE_GetMP4AESDBox
//
// Description:
//      Get ESD Box
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetMP4AESDBox(
                         unsigned char *pReadPtr,
                         unsigned long  dwBoxSize,
                         MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned char  tempByte;
    unsigned char  tempValue;
    unsigned long  dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_MP4AESDBOX* pESDBox = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd.mp4aEntry.esdBox);

    // [dvyu] IEC_100_1384_DC.pdf, 4.7.1.2 ESDescriptor

    SKIP_BYTES(pReadPtr, 4);    // ESDS {0x65,0x73,0x64,0x73}
    dwBoxSize -= 4;

    SKIP_BYTES(pReadPtr, 4);    // {0x00 0x00 0x00 0x00}
    dwBoxSize -= 4;

    SKIP_BYTES(pReadPtr, 1);    // TAG:ID
    dwBoxSize -= 1;

    if (pReadPtr[0] == 0x80 &&  pReadPtr[1] == 0x80 && pReadPtr[2] == 0x80)
    {
        SKIP_BYTES(pReadPtr, 3);
        dwBoxSize -= 3;
    }

    SKIP_BYTES(pReadPtr, 1);    // TAG:Size
    dwBoxSize -= 1;

    SKIP_BYTES(pReadPtr, 2);    // ES_ID
    dwBoxSize -= 2;

    // streamDependenceFlag (1)
    // URL_Flag             (1)
    // OCRstreamFlag        (1)
    // streamPriority       (5)
    GET_BYTE_VALUE(pReadPtr, tempByte);
    dwBoxSize -= 1;
    if (tempByte & 0x80)         // streamDependenceFlag
    {
        SKIP_BYTES(pReadPtr, 2);
        dwBoxSize -= 2;
    }
    if (tempByte & 0x40)         // URL_Flag
    {
        GET_BYTE_VALUE(pReadPtr, tempValue);
        SKIP_BYTES(pReadPtr, tempValue);
        dwBoxSize -= tempValue;
    }
    if (tempByte & 0x20)         // OCRstreamFlag
    {
        SKIP_BYTES(pReadPtr, 2);
        dwBoxSize -= 2;
    }

    //DecoderConfigDescriptor
    SKIP_BYTES(pReadPtr, 1);
    dwBoxSize -=1;
    if (pReadPtr[0] == 0x80 &&  pReadPtr[1] == 0x80 && pReadPtr[2] == 0x80)
    {
        SKIP_BYTES(pReadPtr, 3);
        dwBoxSize -= 3;
    }
    SKIP_BYTES(pReadPtr, 14);
    dwBoxSize -= 14;

    //?
    //DecoderSpecificInfo

    sxaDmx3GPP_GetDecoderSpecificInfo(pESDBox, dwBoxSize, pReadPtr);

    return GPP_SUCCESS;
}



//=============================================================================
//  GPP_RESULT GPP_DECODE_GetMP4VESDBox
//
// Description:
//      Get ESD Box
// size 69-4
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetMP4VESDBox(
                         unsigned char *pReadPtr,
                         unsigned long  dwBoxSize,
                         MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    GPP_RESULT Ret = GPP_SUCCESS;
    unsigned char *pVOLStream;
    unsigned char *tempAddr = pReadPtr;
    unsigned long  tempInt;
    unsigned char  tempByte = 0;
    BIT_STREAM     volStream = {0};
    unsigned long  dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_MP4VESDBOX* pESDBox = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd.mp4vEntry.esdBox);

    do
    {
        GET_DWORD_VALUE(tempAddr, tempInt);

        if (tempInt == VISUAL_OBJECT_SEQUENCE_START_CODE)
        {
            // check profile_and_level_indication
            GET_BYTE_VALUE(tempAddr, tempByte);
            tempAddr -= 1;
            if (tempByte < 0x01 || tempByte > 0x0F)
            {
                // not simple profile
                return (GPP_ERROR_DECODE_MPG_VOL_TYPE);
            }
        }

        tempAddr -= 3;
        dwBoxSize--;
        if (dwBoxSize <= 3)
        {
            return (GPP_ERROR_DECODE_MPG_VOL_START_CODE);
        }
    } while ((tempInt & 0xfffffff0) != VIDEO_OBJECT_LAYER_START_CODE);
    dwBoxSize++;

    pReadPtr = tempAddr - 1;
    // parse VOL
    pVOLStream = (unsigned char *)PalHeapAlloc(PAL_HEAP_DEFAULT, dwBoxSize);
    if (pVOLStream == NULL)
    {
        return GPP_ERROR_DECODE_MALLOC;
    }
    PalMemcpy(pVOLStream, pReadPtr, dwBoxSize);
    sxaDmx3GPP_BitstreamInit(&volStream,(unsigned long *)pVOLStream, dwBoxSize);
    Ret = sxaDmx3GPP_GetVOLParameter(&pESDBox->volInfo, &volStream);
    if (Ret == GPP_SUCCESS)
    {
        //printf("             [VOLVerID]           : %d\n", pESDBox->volInfo.dwVOLVerID);
        //printf("             [Shape]              : %d\n", pESDBox->volInfo.dwShape);
        //printf("             [TimeIncRes]         : %d\n", pESDBox->volInfo.dwTimeIncRes);
        //printf("             [TimeCodeBit]        : %d\n", pESDBox->volInfo.dwTimeCodeBit);
        //printf("             [FixedTimeInc]       : %d\n", pESDBox->volInfo.dwFixedTimeInc);
        //printf("             [FrameWidth]         : %d\n", pESDBox->volInfo.dwFrameWidth);
        //printf("             [FrameHeight]        : %d\n", pESDBox->volInfo.dwFrameHeight);
        //printf("             [QuantBits]          : %d\n", pESDBox->volInfo.dwQuantBits);
        //printf("             [QuantType]          : %d\n", pESDBox->volInfo.dwQuantType);
        //printf("             [ResyncMarker]       : %d\n", pESDBox->volInfo.dwResyncMarker);
        //printf("             [DataPartition]      : %d\n", pESDBox->volInfo.dwDataPartition);
        //printf("             [RevVLC]             : %d\n", pESDBox->volInfo.dwRevVLC);
    }
    PalHeapFree(PAL_HEAP_DEFAULT, pVOLStream);
    return Ret;
}



//=============================================================================
//  GPP_RESULT GPP_DECODE_GetMp4AudioSampleEntry
//
// Description:
//      Get MP4AudioSampleEntry
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetMp4AudioSampleEntry(
                                  unsigned char *pReadPtr,
                                  unsigned long  dwBoxSize,
                                  MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    GPP_RESULT result;
    unsigned long dwSubBoxSize = 0;
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_MP4AENTRY* mp4Entry = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd.mp4aEntry);

    //    MMP_UINT8 *pReadPtrEnd = pReadPtr + dwBoxSize - 4;

    mp4Entry->dwEntryType = MP4A_SOUND_TYPE;

    // [dvyu] 3GPP TS 26.244, Sec 6.4

    // skip reserved_6 [byte]
    SKIP_BYTES(pReadPtr, 6);

    // get Data-reference-index
    GET_WORD_VALUE(pReadPtr, mp4Entry->wDataRefIdx);
    // skip reserved_8
    // skip reserved_2
    // skip reserved_2
    // skip reserved_4
    SKIP_DWORDS(pReadPtr, 4);

    // get timescale
    GET_WORD_VALUE(pReadPtr, mp4Entry->wTimescale);
    //      reserved_2 = 0
    SKIP_BYTES(pReadPtr, 2);

    //
    // get ESD Box , added by Calvin Wu 2005/07/25
    //
    GET_DWORD_VALUE(pReadPtr, dwSubBoxSize);
    result = sxaDmx3GPP_GetMP4AESDBox(pReadPtr, dwSubBoxSize, pGppData);
    if (result != GPP_SUCCESS)
    {
        return result;
    }
    return GPP_SUCCESS;
}



//=============================================================================
//  GPP_RESULT GetMp4SampleEntry
//
// Description:
//      Get mp4 sample entry
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetMp4VisualSampleEntry(
                                   unsigned char *pReadPtr,
                                   unsigned long  dwBoxSize,
                                   MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    GPP_RESULT result;
    unsigned long dwSubBoxSize = 0;
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_MP4VENTRY* mp4Entry = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd.mp4vEntry);

    //    MMP_UINT8 *pReadPtrEnd = pReadPtr + dwBoxSize - 4;

    mp4Entry->dwEntryType = MP4V_VIDEO_TYPE;

    // [dvyu] 3GPP TS 26.244, Sec 6.3

    // skip reserved_6 [byte]
    SKIP_BYTES(pReadPtr, 6);

    // get Data-reference-index
    GET_WORD_VALUE(pReadPtr, mp4Entry->wDataRefIdx);
    // skip reserved_16 [byte]
    SKIP_DWORDS(pReadPtr, 4);

    // get width
    GET_WORD_VALUE(pReadPtr, mp4Entry->wWidth);

    // get height
    GET_WORD_VALUE(pReadPtr, mp4Entry->wHeight);
    // skip reserved_4 = 0x00480000 [byte]
    //      reserved_4 = 0x00480000 [byte]
    //      reserved_4 = 0
    //      reserved_2 = 1
    //      reserved_32= 0
    //      reserved_2 = 24
    //      reserved_2 = -1
    SKIP_BYTES(pReadPtr, 50);

    //
    // get ESD Box , added by Calvin Wu 2005/03/25
    //
    GET_DWORD_VALUE(pReadPtr, dwSubBoxSize);
    result = sxaDmx3GPP_GetMP4VESDBox(pReadPtr, dwSubBoxSize, pGppData);
    if (result != GPP_SUCCESS)
    {
        return result;
    }
    //    pReadPtr += dwSubBoxSize - 4;

    return GPP_SUCCESS;
}



//2004.11.22 s034 adding sound support
//=============================================================================
//  GPP_RESULT GetAMRSampleEntry
//
// Description:
//      Get narrow-band AMR sample entry
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetAMRSampleEntry(
                             unsigned char *pReadPtr,
                             unsigned long  dwBoxSize,
                             MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_AMRENTRY* amrEntry = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd.amrEntry);

    unsigned char *pReadPtrEnd = pReadPtr + dwBoxSize - 8;

    amrEntry->dwEntryType = SAMR_SOUND_TYPE;

    // [dvyu] 3GPP TS 26.244, Sec 6.5

    // skip reserved_6 [byte]
    SKIP_BYTES(pReadPtr, 6);

    // get Data-reference-index
    GET_WORD_VALUE(pReadPtr, amrEntry->wDataRefIdx);

    // skip reserved_8 [byte]
    SKIP_DWORDS(pReadPtr, 2);

    // skip reserved_2 [byte]
    SKIP_WORD(pReadPtr);

    // skip reserved_2 [byte]
    SKIP_WORD(pReadPtr);

    // skip reserved_4 [byte]
    SKIP_DWORD(pReadPtr);

    // get TimeScale
    GET_WORD_VALUE(pReadPtr, amrEntry->wTimeScale);

    // skip reserved_2 [byte]
    SKIP_WORD(pReadPtr);

    //
    // get AMR Specific Box
    //

    // skip size & type
    SKIP_DWORDS(pReadPtr, 2);

    // skip vender & decoder_version
    SKIP_DWORD(pReadPtr);
    SKIP_BYTE(pReadPtr);

    // get AMR specific box
    GET_WORD_VALUE(pReadPtr, amrEntry->amrspec.ModeSet);
    GET_BYTE_VALUE(pReadPtr, amrEntry->amrspec.ModeChangePeriod);
    GET_BYTE_VALUE(pReadPtr, amrEntry->amrspec.FramesPerSample);

    // check box end
    if (pReadPtr != pReadPtrEnd)
    {
        return GPP_ERROR_DECODE_GET_AMR_SAMPLE_ENTRY;
    }
    return GPP_SUCCESS;
}
//2004.11.22 end



static GPP_RESULT
sxaDmx3GPP_GetRAWSampleEntry(
                             unsigned char *pReadPtr,
                             unsigned long  dwBoxSize,
                             MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_RAWENTRY* rawEntry = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd.rawEntry);

    //MMP_UINT8 *pReadPtrEnd = pReadPtr + dwBoxSize - 8;

    rawEntry->dwEntryType = RAW_SOUND_TYPE;

    // [dvyu] QTFF P.124

    // skip reserved_6 [byte]
    SKIP_BYTES(pReadPtr, 6);

    // get Data-reference-index
    GET_WORD_VALUE(pReadPtr, rawEntry->wDataRefIdx);

    // skip 4 bytes for Version and Revision level
    SKIP_BYTES(pReadPtr, 4);

    // skip 4 bytes for Vendor
    SKIP_BYTES(pReadPtr, 4);

    // get Number of channel
    GET_WORD_VALUE(pReadPtr, rawEntry->wChannelNum);

    // get Sample Size
    GET_WORD_VALUE(pReadPtr, rawEntry->wSampleSize);

    // skip 4 bytes for Compression ID and Packet Size
    SKIP_BYTES(pReadPtr, 4);

    // get Sample Rate
    GET_DWORD_VALUE(pReadPtr, rawEntry->dwSampleRate);

    //printf("GPP_DECODE_GetRAWSampleEntry %d %d %d\n", rawEntry->wChannelNum, rawEntry->wSampleSize, rawEntry->dwSampleRate);
    return GPP_SUCCESS;
}



static GPP_RESULT
sxaDmx3GPP_GetULAWSampleEntry(
                              unsigned char *pReadPtr,
                              unsigned long  dwBoxSize,
                              MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_ULAWENTRY* ulawEntry = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd.ulawEntry);

    //MMP_UINT8 *pReadPtrEnd = pReadPtr + dwBoxSize - 8;

    ulawEntry->dwEntryType = ULAW_SOUND_TYPE;

    // [dvyu] QTFF P.124

    // skip reserved_6 [byte]
    SKIP_BYTES(pReadPtr, 6);

    // get Data-reference-index
    GET_WORD_VALUE(pReadPtr, ulawEntry->wDataRefIdx);

    // skip 4 bytes for Version and Revision level
    SKIP_BYTES(pReadPtr, 4);

    // skip 4 bytes for Vendor
    SKIP_BYTES(pReadPtr, 4);

    // get Number of channel
    GET_WORD_VALUE(pReadPtr, ulawEntry->wChannelNum);

    // get Sample Size
    GET_WORD_VALUE(pReadPtr, ulawEntry->wSampleSize);
    ulawEntry->wSampleSize /= ulawEntry->wChannelNum;

    // skip 4 bytes for Compression ID and Packet Size
    SKIP_BYTES(pReadPtr, 4);

    // get Sample Rate
    GET_DWORD_VALUE(pReadPtr, ulawEntry->dwSampleRate);

    //printf("GPP_DECODE_GetULAWSampleEntry %d %d %d\n", ulawEntry->wChannelNum, ulawEntry->wSampleSize, ulawEntry->dwSampleRate);
    return GPP_SUCCESS;
}



static GPP_RESULT
sxaDmx3GPP_GetSOWTSampleEntry(
                              unsigned char *pReadPtr,
                              unsigned long  dwBoxSize,
                              MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_SOWTENTRY* sowtEntry = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd.sowtEntry);

    //MMP_UINT8 *pReadPtrEnd = pReadPtr + dwBoxSize - 8;

    sowtEntry->dwEntryType = SOWT_SOUND_TYPE;

    // [dvyu] QTFF P.124

    // skip reserved_6 [byte]
    SKIP_BYTES(pReadPtr, 6);

    // get Data-reference-index
    GET_WORD_VALUE(pReadPtr, sowtEntry->wDataRefIdx);

    // skip 4 bytes for Version and Revision level
    SKIP_BYTES(pReadPtr, 4);

    // skip 4 bytes for Vendor
    SKIP_BYTES(pReadPtr, 4);

    // get Number of channel
    GET_WORD_VALUE(pReadPtr, sowtEntry->wChannelNum);

    // get Sample Size
    GET_WORD_VALUE(pReadPtr, sowtEntry->wSampleSize);

    // skip 4 bytes for Compression ID and Packet Size
    SKIP_BYTES(pReadPtr, 4);

    // get Sample Rate
    GET_DWORD_VALUE(pReadPtr, sowtEntry->dwSampleRate);

    //printf("GPP_DECODE_GetRAWSampleEntry %d %d %d\n", sowtEntry->wChannelNum, sowtEntry->wSampleSize, sowtEntry->dwSampleRate);
    return GPP_SUCCESS;
}



static GPP_RESULT
sxaDmx3GPP_GetTWOSSampleEntry(
                              unsigned char *pReadPtr,
                              unsigned long  dwBoxSize,
                              MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_TWOSENTRY* twosEntry = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd.twosEntry);

    //MMP_UINT8 *pReadPtrEnd = pReadPtr + dwBoxSize - 8;

    twosEntry->dwEntryType = TWOS_SOUND_TYPE;

    // [dvyu] QTFF P.124

    // skip reserved_6 [byte]
    SKIP_BYTES(pReadPtr, 6);

    // get Data-reference-index
    GET_WORD_VALUE(pReadPtr, twosEntry->wDataRefIdx);

    // skip 4 bytes for Version and Revision level
    SKIP_BYTES(pReadPtr, 4);

    // skip 4 bytes for Vendor
    SKIP_BYTES(pReadPtr, 4);

    // get Number of channel
    GET_WORD_VALUE(pReadPtr, twosEntry->wChannelNum);

    // get Sample Size
    GET_WORD_VALUE(pReadPtr, twosEntry->wSampleSize);

    // skip 4 bytes for Compression ID and Packet Size
    SKIP_BYTES(pReadPtr, 4);

    // get Sample Rate
    GET_DWORD_VALUE(pReadPtr, twosEntry->dwSampleRate);

    //printf("GPP_DECODE_GetRAWSampleEntry %d %d %d\n", twosEntry->wChannelNum, twosEntry->wSampleSize, twosEntry->dwSampleRate);
    return GPP_SUCCESS;
}



//=============================================================================
//  GPP_RESULT GetVideoSampleEntry
//
// Description:
//      Get video sample entry
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetVideoSampleEntry(
                               unsigned char *pReadPtr,
                               unsigned long  dwBoxSize,
                               MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    GPP_RESULT result;
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_STSD* pStsd = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd);
    unsigned long dwVideoType = 0;

    GET_DWORD_VALUE(pReadPtr, dwVideoType);
    switch (dwVideoType)
    {
        //        case AVC_VIDEO_TYPE:
        //            pStsd->dwEntryType = AVC_VIDEO_TYPE;
        //            result = GPP_DECODE_GetAVCSampleEntry(pReadPtr, dwBoxSize, pGppData);
        //            if (result != GPP_SUCCESS) return result;
        //            break;
    case H263_VIDEO_TYPE :
        result = sxaDmx3GPP_GetH263SampleEntry(pReadPtr, dwBoxSize, pGppData);
        if (result != GPP_SUCCESS)
        {
            return result;
        }
        pStsd->dwEntryType = H263_VIDEO_TYPE;
        break;
    case MP4V_VIDEO_TYPE :
        result = sxaDmx3GPP_GetMp4VisualSampleEntry(pReadPtr, dwBoxSize, pGppData);
        if (result != GPP_SUCCESS)
        {
            return result;
        }
        pStsd->dwEntryType = MP4V_VIDEO_TYPE;
        break;
    case MJPA_VIDEO_TYPE :
    case JPEG_VIDEO_TYPE :
        result = sxaDmx3GPP_GetJPEGSampleEntry(pReadPtr, dwBoxSize, pGppData);
        if (result != GPP_SUCCESS)
        {
            return result;
        }
        pStsd->dwEntryType = JPEG_VIDEO_TYPE;
        break;
    default :
        break;
    }
    return GPP_SUCCESS;
}



//2004.11.22 s034 adding sound support
//=============================================================================
//  GPP_RESULT GetAudioSampleEntry
//
// Description:
//      Get Audio sample entry
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetAudioSampleEntry(
                               unsigned char *pReadPtr,
                               unsigned long  dwBoxSize,
                               MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    GPP_RESULT result;
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_STSD* pStsd = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd);
    unsigned long dwAudioType = 0;

    GET_DWORD_VALUE(pReadPtr, dwAudioType);

    switch (dwAudioType)
    {
    case MP4A_SOUND_TYPE :
        result = sxaDmx3GPP_GetMp4AudioSampleEntry(pReadPtr, dwBoxSize, pGppData);
        if (result != GPP_SUCCESS)
        {
            return result;
        }
        pStsd->dwEntryType = MP4A_SOUND_TYPE;
        break;
    case SAMR_SOUND_TYPE :
        result = sxaDmx3GPP_GetAMRSampleEntry(pReadPtr, dwBoxSize, pGppData);
        if (result != GPP_SUCCESS)
        {
            return result;
        }
        pStsd->dwEntryType = SAMR_SOUND_TYPE;
        break;
    case SAWB_SOUND_TYPE :
        //printf("[3GPP:DEC] Decode AWB Sample Entry not ready!\n");
        pStsd->dwEntryType = SAWB_SOUND_TYPE;
        break;
    case RAW_SOUND_TYPE : //dkwang 0327
        result = sxaDmx3GPP_GetRAWSampleEntry(pReadPtr, dwBoxSize, pGppData);
        if (result != GPP_SUCCESS)
        {
            return result;
        }
        pStsd->dwEntryType = RAW_SOUND_TYPE;
        // learned from vlc-0.9.9a
        if ((pStsd->rawEntry.wSampleSize + 7) / 8 != 1)
        {
            pStsd->dwEntryType = TWOS_SOUND_TYPE;
            pStsd->twosEntry.dwSampleRate = pStsd->rawEntry.dwSampleRate;
            pStsd->twosEntry.wChannelNum  = pStsd->rawEntry.wChannelNum;
            pStsd->twosEntry.wDataRefIdx  = pStsd->rawEntry.wDataRefIdx;
            pStsd->twosEntry.wSampleSize  = pStsd->rawEntry.wSampleSize;
            dwAudioType = TWOS_SOUND_TYPE;
        }
        break;
    case ULAW_SOUND_TYPE : //dkwang 0327
        result = sxaDmx3GPP_GetULAWSampleEntry(pReadPtr, dwBoxSize, pGppData);
        if (result != GPP_SUCCESS){
            return result;
        }
        pStsd->dwEntryType = ULAW_SOUND_TYPE;
        break;
    case SOWT_SOUND_TYPE :
        result = sxaDmx3GPP_GetSOWTSampleEntry(pReadPtr, dwBoxSize, pGppData);
        if (result != GPP_SUCCESS)
        {
            return result;
        }
        pStsd->dwEntryType = SOWT_SOUND_TYPE;
    case TWOS_SOUND_TYPE :
        result = sxaDmx3GPP_GetTWOSSampleEntry(pReadPtr, dwBoxSize, pGppData);
        if (result != GPP_SUCCESS)
        {
            return result;
        }
        pStsd->dwEntryType = TWOS_SOUND_TYPE;
    default :
        break;
    }

#if (SUPPORT_NEW_PCM_INFO)
    GetMaxBytePerSample(pGppData, dwAudioType);
#endif

    return GPP_SUCCESS;
}



//=============================================================================
//  GPP_RESULT GetSampleDescription
//
// Description:
//      Get sample description
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetSampleDescription(
                                unsigned char *pReadPtr,
                                unsigned long  dwBoxSize,
                                MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    GPP_RESULT result;
    unsigned long  dwSubBoxSize = 0;
    unsigned long  dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_STSD* stsd = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd);
    unsigned long  dwHandleType = pGppData->moov.trak[dwTrakIdx].mdia.hdlr.dwHandleType;
    unsigned int   i;
    unsigned char *pReadPtrEnd = pReadPtr + dwBoxSize - 8;
    unsigned long  dwMediaType = 0;

    // [dvyu] QTFF P.87

    // skip version & flag
    SKIP_DWORD(pReadPtr);

    // get entry-count
    GET_DWORD_VALUE(pReadPtr, stsd->dwEntryCount);

    // get sample-entry
    for (i=0; i<stsd->dwEntryCount; i++)
    {
        GET_DWORD_VALUE(pReadPtr, dwSubBoxSize);

        switch (dwHandleType)
        {
        case VIDE_HANDLER_TYPE :
            result = sxaDmx3GPP_GetVideoSampleEntry(pReadPtr, dwSubBoxSize, pGppData);
            if ( result != GPP_SUCCESS)
            {
                return result;
            }
            break;
        case SOUN_HANDLER_TYPE :
            //GetAudioSamplerEntry();
            //2004.11.22 s034 adding sound support
            result = sxaDmx3GPP_GetAudioSampleEntry(pReadPtr, dwSubBoxSize, pGppData);
            if ( result != GPP_SUCCESS)
            {
                return result;
            }
            break;
        case HINT_HANDLER_TYPE :
            //GetHintSamplerEntry();
            break;
        case SDSM_HANDLER_TYPE :
            break;
        case ODSM_HANDLER_TYPE :
            break;
        default :
            // Unknown Handler type !
            // For some special media files, we check its next 4 bytes
            dwMediaType = ((unsigned long *)pReadPtr)[0];
            BS_WAP(dwMediaType);
            switch (dwMediaType)
            {
            case H263_VIDEO_TYPE :
            case MP4V_VIDEO_TYPE :
            case MJPA_VIDEO_TYPE :
            case JPEG_VIDEO_TYPE :
                pGppData->moov.trak[dwTrakIdx].mdia.hdlr.dwHandleType = VIDE_HANDLER_TYPE;
                result = sxaDmx3GPP_GetVideoSampleEntry(pReadPtr, dwSubBoxSize, pGppData);
                if ( result != GPP_SUCCESS)
                {
                    return result;
                }
                break;
            case MP4A_SOUND_TYPE :
            case SAMR_SOUND_TYPE :
            case SAWB_SOUND_TYPE :
            case RAW_SOUND_TYPE :
            case ULAW_SOUND_TYPE :
            case SOWT_SOUND_TYPE :
            case TWOS_SOUND_TYPE :
                pGppData->moov.trak[dwTrakIdx].mdia.hdlr.dwHandleType = SOUN_HANDLER_TYPE;
                result = sxaDmx3GPP_GetAudioSampleEntry(pReadPtr, dwSubBoxSize, pGppData);
                if ( result != GPP_SUCCESS)
                {
                    return result;
                }
                break;
            default:
                break;
            }
            break;
        }
        pReadPtr += dwSubBoxSize - 4;
    }

    // check box end
    if (pReadPtr != pReadPtrEnd)
    {
        return GPP_ERROR_DECODE_GET_STSD_BOX;
    }

    return GPP_SUCCESS;
}



//=============================================================================
//  GPP_RESULT GetDecodingTime
//
// Description:
//      Get decoding time
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetDecodingTime(
                           PAL_FILE *pFilePtr,
                           unsigned long dwBoxSize,
                           MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    GPP_RESULT    ret;
    signed long   dwFileERRCode;
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_STTS* stts = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stts);

    // skip version & flag
    dwFileERRCode = PalTFileSeek(pFilePtr, 4L, PAL_SEEK_CUR, MMP_NULL);
    if (dwFileERRCode < 0)
    {
        return (GPP_ERROR_DECODE_FILE_SEEK);
    }

    dwFileERRCode = PalTFileRead(&stts->dwEntryCount, 4, 1, pFilePtr, MMP_NULL);
    if (dwFileERRCode < 0)
    {
        return (GPP_ERROR_DECODE_FILE_READ);
    }
    gppFilePos += 4L + 4;

    BS_WAP(stts->dwEntryCount);

    // allocate memory for sampler-count & sample-delta
    if (stts->dwEntryCount != 0)
    {
        unsigned long offset = PalTFileTell(pFilePtr, MMP_NULL);
        ret = sxaDmx3GPP_TableStreamInitial(pFilePtr, offset , 2, stts->dwEntryCount, STTS_ENTRY_BUFFER_SIZE, &stts->sttsEntry);
        if (ret != GPP_SUCCESS)
        {
            return ret;
        }

        dwFileERRCode = PalTFileSeek(pFilePtr, stts->dwEntryCount*8 + offset, PAL_SEEK_SET, MMP_NULL);
        gppFilePos = stts->dwEntryCount*8 + offset;
    }

    return GPP_SUCCESS;
}



//=============================================================================
//  GPP_RESULT GetSampleToChunk
//
// Description:
//      Get sample to chunk
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetSampleToChunk(
                            PAL_FILE *pFilePtr,
                            unsigned long dwBoxSize,
                            MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    GPP_RESULT    ret;
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_STSC* stsc = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsc);
    signed long   dwFileERRCode;

    // skip version & flag
    dwFileERRCode = PalTFileSeek(pFilePtr, 4L, PAL_SEEK_CUR, MMP_NULL);
    if (dwFileERRCode < 0)
    {
        return (GPP_ERROR_DECODE_FILE_SEEK);
    }
    dwFileERRCode = PalTFileRead(&stsc->dwEntryCount, 4, 1, pFilePtr, MMP_NULL);
    if (dwFileERRCode < 0)
    {
        return (GPP_ERROR_DECODE_FILE_READ);
    }
    gppFilePos += 4L + 4;

    BS_WAP(stsc->dwEntryCount);

    // allocate memory for first-chunk, sample-per-chunk & sample-description-index
    if (stsc->dwEntryCount != 0)
    {
        unsigned long offset = PalTFileTell(pFilePtr, MMP_NULL);
        ret = sxaDmx3GPP_TableStreamInitial(pFilePtr, offset , 3, stsc->dwEntryCount, STSC_ENTRY_BUFFER_SIZE, &stsc->stscEntry);
        if (ret != GPP_SUCCESS)
        {
            return ret;
        }

        dwFileERRCode = PalTFileSeek(pFilePtr, stsc->dwEntryCount*12 + offset, PAL_SEEK_SET, MMP_NULL);
        gppFilePos = stsc->dwEntryCount*12 + offset;
    }

    return GPP_SUCCESS;
}



//=============================================================================
//  GPP_RESULT GetSampleSize
//
// Description:
//      Get sample size
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetSampleSize(
                         PAL_FILE *pFilePtr,
                         unsigned long dwBoxSize,
                         MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    GPP_RESULT    ret;
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_STSZ* stsz = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsz);
    MMP_3GPP_DECODE_STSD* pStsd = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stsd);
    signed long   dwFileERRCode = 0;

    // skip version & flag
    dwFileERRCode = PalTFileSeek(pFilePtr, 4L, PAL_SEEK_CUR, MMP_NULL);
    if (dwFileERRCode < 0)
    {
        return (GPP_ERROR_DECODE_FILE_SEEK);
    }
    dwFileERRCode = PalTFileRead(&stsz->dwSampleSize, 4, 1, pFilePtr, MMP_NULL);
    if (dwFileERRCode < 0)
    {
        return (GPP_ERROR_DECODE_FILE_READ);
    }
    gppFilePos += 4L + 4;

    BS_WAP(stsz->dwSampleSize);

    stsz->dwOriginalSize = stsz->dwSampleSize;

    switch (pStsd->dwEntryType)
    {
    case ULAW_SOUND_TYPE:
        stsz->dwSampleSize *= (pStsd->ulawEntry.wChannelNum / stsz->dwSampleSize);
        break;
    case RAW_SOUND_TYPE:
        // learned from vlc-0.9.9a
        stsz->dwSampleSize = ((pStsd->rawEntry.wSampleSize + 7) / 8) * pStsd->rawEntry.wChannelNum;
        break;
    case SOWT_SOUND_TYPE:
        // learned from vlc-0.9.9a
        stsz->dwSampleSize = ((pStsd->sowtEntry.wSampleSize + 7) / 8) * pStsd->sowtEntry.wChannelNum;
        break;
    case TWOS_SOUND_TYPE:
        // learned from vlc-0.9.9a
        stsz->dwSampleSize = ((pStsd->twosEntry.wSampleSize + 7) / 8) * pStsd->twosEntry.wChannelNum;
        break;
    }

    // get sample-size
    dwFileERRCode = PalTFileRead(&stsz->dwEntryCount, 4, 1, pFilePtr, MMP_NULL);
    gppFilePos += 4;

    BS_WAP(stsz->dwEntryCount);

    // if sample-size is 0, then the samples have different size,
    // and those size are stored in the sample size table.
    if (!(stsz->dwSampleSize))
    {
        // allocate memory for entry-size
        if (stsz->dwEntryCount != 0)
        {
            unsigned long offset = PalTFileTell(pFilePtr, MMP_NULL);
            ret = sxaDmx3GPP_TableStreamInitial(pFilePtr, offset , 1, stsz->dwEntryCount, STSZ_ENTRY_BUFFER_SIZE, &stsz->stszEntry);
            if (ret != GPP_SUCCESS)
            {
                return ret;
            }

            dwFileERRCode = PalTFileSeek(pFilePtr, stsz->dwEntryCount*4 + offset, PAL_SEEK_SET, MMP_NULL);
            gppFilePos = stsz->dwEntryCount*4 + offset;
        }
    }

    return GPP_SUCCESS;
}



//=============================================================================
//  GPP_RESULT GetChunkOffset
//
// Description:
//      Get chunk offset
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetChunkOffset(
                          PAL_FILE *pFilePtr,
                          unsigned long dwBoxSize,
                          MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    GPP_RESULT    ret;
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_STCO* stco = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stco);
    signed long   dwFileERRCode = 0;

    // skip version & flag
    dwFileERRCode = PalTFileSeek(pFilePtr, 4L, PAL_SEEK_CUR, MMP_NULL);
    if (dwFileERRCode < 0)
    {
        return (GPP_ERROR_DECODE_FILE_SEEK);
    }
    dwFileERRCode = PalTFileRead(&stco->dwEntryCount, 4, 1, pFilePtr, MMP_NULL);
    if (dwFileERRCode < 0)
    {
        return (GPP_ERROR_DECODE_FILE_READ);
    }
    gppFilePos += 4L + 4;

    BS_WAP(stco->dwEntryCount);

    // allocate memory for chunk-offset
    if (stco->dwEntryCount != 0)
    {
        unsigned long offset = PalTFileTell(pFilePtr, MMP_NULL);
        ret = sxaDmx3GPP_TableStreamInitial(pFilePtr, offset , 1, stco->dwEntryCount, STCO_ENTRY_BUFFER_SIZE, &stco->stcoEntry);
        if (ret != GPP_SUCCESS)
        {
            return ret;
        }

        dwFileERRCode = PalTFileSeek(pFilePtr, stco->dwEntryCount*4 + offset, PAL_SEEK_SET, MMP_NULL);
        gppFilePos = stco->dwEntryCount*4 + offset;
    }

    return GPP_SUCCESS;
}



//=============================================================================
//  GPP_RESULT GetSyncSample
//
// Description:
//      Get sync sample
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//=============================================================================
// [dvyu] 2007.11.22 Modified
static GPP_RESULT
sxaDmx3GPP_GetSyncSample(
                          PAL_FILE *pFilePtr,
                          unsigned long dwBoxSize,
                          MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_STSS* stss = &(pGppData->moov.trak[dwTrakIdx].mdia.minf.stbl.stss);
    signed long   dwFileERRCode = 0;
    unsigned int  i;

    // [dvyu] skip version & flag
    dwFileERRCode = PalTFileSeek(pFilePtr, 4L, PAL_SEEK_CUR, MMP_NULL);
    if (dwFileERRCode < 0)
    {
        return (GPP_ERROR_DECODE_FILE_SEEK);
    }
    // [dvyu] read Entry Count
    dwFileERRCode = PalTFileRead(&stss->dwEntryCount, 4, 1, pFilePtr, MMP_NULL);
    if (dwFileERRCode < 0)
    {
        return (GPP_ERROR_DECODE_FILE_READ);
    }

    gppFilePos += (4L + 4);

    BS_WAP(stss->dwEntryCount);

    // allocate memory for sync-sample
    if (stss->dwEntryCount != 0)
    {
        stss->pSampleNumber = (unsigned long*)PalHeapAlloc(PAL_HEAP_DEFAULT, (stss->dwEntryCount)*4);
        if (stss->pSampleNumber == NULL)
        {
            return GPP_ERROR_DECODE_MALLOC;
        }
        PalMemset(stss->pSampleNumber, 0, (stss->dwEntryCount)*4);

        dwFileERRCode = PalTFileRead(stss->pSampleNumber, (stss->dwEntryCount)*4, 1, pFilePtr, MMP_NULL);
        if (dwFileERRCode < 0)
        {
            return (GPP_ERROR_DECODE_FILE_READ);
        }

        gppFilePos += (stss->dwEntryCount)*4;

        for (i=0; i<stss->dwEntryCount; i++)
        {
            BS_WAP(stss->pSampleNumber[i]);
        }
    }

    return GPP_SUCCESS;
}
// [dvyu] 2007.11.22 Modified



//=============================================================================
//  GPP_RESULT GPP_DECODE_GetHandlerReference
//
// Description:
//      Get handler reference
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetHandlerReference(
                               unsigned char *pReadPtr,
                               unsigned long  dwBoxSize,
                               MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned long dwTempType;
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_HDLR* hdlr = &(pGppData->moov.trak[dwTrakIdx].mdia.hdlr);

    // skip version & flag
    SKIP_DWORD(pReadPtr);
    // skip pre-defined = 0
    SKIP_DWORD(pReadPtr);

    // get handler-type
    //GET_DWORD_VALUE(pReadPtr, hdlr->dwHandleType);
    GET_DWORD_VALUE(pReadPtr, dwTempType);
    //dkwang 0327
    if (dwTempType != ALIS_HANDLER_TYPE)
    {
        hdlr->dwHandleType = dwTempType;
    }
    //printf("HDLR_Type: %d %x\n", dwTrakIdx, hdlr->dwHandleType);
    // skip reserved[3] = 0
    SKIP_DWORDS(pReadPtr, 3);
    // skip name
    return GPP_SUCCESS;
}



//=============================================================================
//  GPP_RESULT GPP_DECODE_GetMediaHeader
//
// Description:
//      Get media header
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetMediaHeader(
                          unsigned char *pReadPtr,
                          unsigned long  dwBoxSize,
                          MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_MDHD* mdhd = &(pGppData->moov.trak[dwTrakIdx].mdia.mdhd);

    unsigned char *pReadPtrEnd = pReadPtr + dwBoxSize - 8;

    // skip version & flag
    SKIP_DWORD(pReadPtr);
    // get create-time
    GET_DWORD_VALUE(pReadPtr, mdhd->dwCreateTime);
    // get modification-time
    GET_DWORD_VALUE(pReadPtr, mdhd->dwModTime);
    // get timescale
    GET_DWORD_VALUE(pReadPtr, mdhd->dwTimeScale);
    // get duration
    GET_DWORD_VALUE(pReadPtr, mdhd->dwDuration);
    // skip pad = 0 & language
    SKIP_WORD(pReadPtr);
    // skip pre-defined = 0
    SKIP_WORD(pReadPtr);

    // check box end
    if (pReadPtr != pReadPtrEnd)
    {
        return GPP_ERROR_DECODE_GET_MDHD_BOX;
    }

    return GPP_SUCCESS;
}



//=============================================================================
//  GPP_RESULT GetTrackHeader
//
// Description:
//      Get track header
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetTrackHeader(
                          unsigned char *pReadPtr,
                          unsigned long  dwBoxSize,
                          MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned long dwTrakIdx = pGppData->dwTrakCount - 1;
    MMP_3GPP_DECODE_TKHD* tkhd = &(pGppData->moov.trak[dwTrakIdx].tkhd);
    unsigned char *pReadPtrEnd = pReadPtr + dwBoxSize - 8;
    int i, j;

    // skip version & flag
    SKIP_DWORD(pReadPtr);
    // get create-time
    GET_DWORD_VALUE(pReadPtr, tkhd->dwCreateTime);
    // get modification-time
    GET_DWORD_VALUE(pReadPtr, tkhd->dwModTime);
    // get timescale
    GET_DWORD_VALUE(pReadPtr, tkhd->dwTrackID);
    // skip reserved = 0
    SKIP_DWORD(pReadPtr);
    // get duration
    GET_DWORD_VALUE(pReadPtr, tkhd->dwDuration);
    // skip reserved[2] = 0
    SKIP_DWORDS(pReadPtr, 2);
    // skip layer = 0
    SKIP_WORD(pReadPtr);
    // skip pre-defined = 0
    SKIP_WORD(pReadPtr);
    // skip volume
    SKIP_WORD(pReadPtr);
    // skip reserved = 0
    SKIP_WORD(pReadPtr);

    // get matrix
    for (i=0; i<3; i++)
    {
        for (j=0; j<3; j++)
        {
            GET_DWORD_VALUE(pReadPtr, tkhd->dwMatrix[i][j]);
        }
    }

    // get width
    GET_DWORD_VALUE(pReadPtr, tkhd->dwWidth);
    // get height
    GET_DWORD_VALUE(pReadPtr, tkhd->dwHeight);
    // check box end
    if (pReadPtr != pReadPtrEnd)
    {
        return GPP_ERROR_DECODE_GET_TKHD_BOX;
    }

    return GPP_SUCCESS;
}



//=============================================================================
//  GPP_RESULT GPP_DECODE_GetMovieHeader
//
// Description:
//      Get movie header
//
// Parameters:
//      pReadPtr : 3GPP bitstream
//      dwBoxSize : box size
//=============================================================================
static GPP_RESULT
sxaDmx3GPP_GetMovieHeader(
                          unsigned char *pReadPtr,
                          unsigned long  dwBoxSize,
                          MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    int i, j;
    MMP_3GPP_DECODE_MVHD* mvhd = &(pGppData->moov.mvhd);
    unsigned char *pReadPtrEnd = pReadPtr + dwBoxSize - 8;

    // skip version & flag
    SKIP_DWORD(pReadPtr);
    // get create-time
    GET_DWORD_VALUE(pReadPtr, mvhd->dwCreateTime);
    // get modification-time
    GET_DWORD_VALUE(pReadPtr, mvhd->dwModTime);
    // get timescale
    GET_DWORD_VALUE(pReadPtr, mvhd->dwTimeScale);
    // get duration
    GET_DWORD_VALUE(pReadPtr, mvhd->dwDuration);
    // skip rate = 0x00010000
    SKIP_DWORD(pReadPtr);
    // skip volume = 0x0100
    SKIP_WORD(pReadPtr);
    // skip reserved = 0
    SKIP_WORD(pReadPtr);
    // skip reserved[2] = 0
    SKIP_DWORDS(pReadPtr, 2);

    // get matrix
    for (i=0; i<3; i++)
    {
        for (j=0; j<3; j++)
        {
            GET_DWORD_VALUE(pReadPtr, mvhd->dwMatrix[i][j]);
        }
    }

    // skip reserved[6] = 0
    SKIP_DWORDS(pReadPtr, 6);

    // get next-track-ID
    GET_DWORD_VALUE(pReadPtr, mvhd->dwNextTrackID);
    // check box end
    if (pReadPtr != pReadPtrEnd)
    {
        return GPP_ERROR_DECODE_GET_MVHD_BOX;
    }

    return GPP_SUCCESS;
}



static GPP_RESULT
sxaDmx3GPP_ParseUDTA(
    PAL_FILE *pFilePtr,
    unsigned long dwBoxSize,
    MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    signed long   dwFileERRCode;
    unsigned long dwReadByte = 0;
    unsigned long dwAtomSize = 0;
    unsigned long dwAtomType = 0;
    int atom_skip_byte = 16;          // skip <size> + "data00010000"
    int str_len = 0;

    while (dwReadByte < (dwBoxSize - 8L))
    {
        // atom size
        dwFileERRCode = PalTFileRead(&dwAtomSize, 4, 1, pFilePtr, MMP_NULL);
        if (dwFileERRCode != 1)
        {
            return (GPP_ERROR_DECODE_FILE_READ);
        }
        dwReadByte += 4;
        BS_WAP(dwAtomSize);
        if (dwAtomSize < 8)
        {
            // The size of an atom should be 8 at least.
            break;
        }

        // atom type
        dwFileERRCode = PalTFileRead(&dwAtomType, sizeof(unsigned long), 1, pFilePtr, MMP_NULL);
        if (dwFileERRCode != 1)
        {
            return (GPP_ERROR_DECODE_FILE_READ);
        }
        dwReadByte += sizeof(unsigned long);
        BS_WAP(dwAtomType);
        switch (dwAtomType)
        {
        case META_BOX_TYPE:
            // skip version & flag
            dwFileERRCode = PalTFileSeek(pFilePtr, 4, PAL_SEEK_CUR, MMP_NULL);
            if (dwFileERRCode < 0)
            {
                return (GPP_ERROR_DECODE_FILE_SEEK);
            }
            dwReadByte += 4;
            break;
        case ILST_BOX_TYPE:
            // do nothing
            break;
        case CNAM_BOX_TYPE:
            str_len = dwAtomSize - 8 - atom_skip_byte;
            if (str_len <= 0 || pGppData->moov.udta.pchCnam != NULL)
            {
                goto skip_atom;
            }

            dwFileERRCode = PalTFileSeek(pFilePtr, atom_skip_byte, PAL_SEEK_CUR, MMP_NULL);
            if (dwFileERRCode < 0)
            {
                return (GPP_ERROR_DECODE_FILE_SEEK);
            }

            pGppData->moov.udta.pchCnam = (unsigned char *)PalHeapAlloc(PAL_HEAP_DEFAULT, str_len + 1);
            if (pGppData->moov.udta.pchCnam == NULL)
            {
                return GPP_ERROR_DECODE_MALLOC;
            }
            PalMemset(pGppData->moov.udta.pchCnam, 0, (str_len + 1) * sizeof(unsigned char));

            dwFileERRCode = PalTFileRead(pGppData->moov.udta.pchCnam, str_len, 1, pFilePtr, MMP_NULL);
            if (dwFileERRCode != 1)
            {
                return (GPP_ERROR_DECODE_FILE_READ);
            }
            dwReadByte += (dwAtomSize - 8L);
            break;
        case CART_BOX_TYPE:
            str_len = dwAtomSize - 8 - atom_skip_byte;
            if (str_len <= 0 || pGppData->moov.udta.pchCart != NULL)
            {
                goto skip_atom;
            }

            dwFileERRCode = PalTFileSeek(pFilePtr, atom_skip_byte, PAL_SEEK_CUR, MMP_NULL);
            if (dwFileERRCode < 0)
            {
                return (GPP_ERROR_DECODE_FILE_SEEK);
            }

            pGppData->moov.udta.pchCart = (unsigned char *)PalHeapAlloc(PAL_HEAP_DEFAULT, str_len + 1);
            if (pGppData->moov.udta.pchCart == NULL)
            {
                return GPP_ERROR_DECODE_MALLOC;
            }
            PalMemset(pGppData->moov.udta.pchCart, 0, (str_len + 1) * sizeof(unsigned char));

            dwFileERRCode = PalTFileRead(pGppData->moov.udta.pchCart, str_len, 1, pFilePtr, MMP_NULL);
            if (dwFileERRCode != 1)
            {
                return (GPP_ERROR_DECODE_FILE_READ);
            }
            dwReadByte += (dwAtomSize - 8L);
            break;
        case CALB_BOX_TYPE:
            str_len = dwAtomSize - 8 - atom_skip_byte;
            if (str_len <= 0 || pGppData->moov.udta.pchCalb != NULL)
            {
                goto skip_atom;
            }

            dwFileERRCode = PalTFileSeek(pFilePtr, atom_skip_byte, PAL_SEEK_CUR, MMP_NULL);
            if (dwFileERRCode < 0)
            {
                return (GPP_ERROR_DECODE_FILE_SEEK);
            }

            pGppData->moov.udta.pchCalb = (unsigned char *)PalHeapAlloc(PAL_HEAP_DEFAULT, str_len + 1);
            if (pGppData->moov.udta.pchCalb == NULL)
            {
                return GPP_ERROR_DECODE_MALLOC;
            }
            PalMemset(pGppData->moov.udta.pchCalb, 0, (str_len + 1) * sizeof(unsigned char));

            dwFileERRCode = PalTFileRead(pGppData->moov.udta.pchCalb, str_len, 1, pFilePtr, MMP_NULL);
            if (dwFileERRCode != 1)
            {
                return (GPP_ERROR_DECODE_FILE_READ);
            }
            dwReadByte += (dwAtomSize - 8L);
            break;
        case HDLR_BOX_TYPE:
        default:
skip_atom:
            dwFileERRCode = PalTFileSeek(pFilePtr, (dwAtomSize - 8L), PAL_SEEK_CUR, MMP_NULL);
            if (dwFileERRCode < 0)
            {
                return (GPP_ERROR_DECODE_FILE_SEEK);
            }
            dwReadByte += (dwAtomSize - 8L);
            break;
        }
    }

    gppFilePos += dwReadByte;

    return GPP_SUCCESS;
}



//-----< mpg_GetVOLParameter >----- from honda
GPP_RESULT
sxaDmx3GPP_GetVOLParameter(SXA_DMXVOLINFO_T *pVOLInfo, BIT_STREAM *bs)
{
    // [dvyu] ISO 14496-2, Sec 6.2.3 (P.35)

    // video_object_layer_start_code
    BitstreamSkip(bs, 32);

    // random_accessible_vol
    BitstreamSkip(bs, 1);

    // video_object_type_indication
    BitstreamShowBits(bs, 8);

    if (BitstreamShowBits(bs, 8) > MPG_VOL_TYPE_SIMPLE)
    {
        //printf("[GPP_ERROR_DECODE_MPG_VOL_TYPE]\n");
        return (GPP_ERROR_DECODE_MPG_VOL_TYPE);
    }
    BitstreamSkip(bs, 8);

    // is_object_layer_identifier
    if (BitstreamGetBit(bs))
    {
        // video_object_layer_verid
        pVOLInfo->dwVOLVerID = BitstreamGetBits(bs, 4);
        // video_object_layer_priority
        BitstreamSkip(bs, 3);
    }
    else
    {
        pVOLInfo->dwVOLVerID = 1;
    }

    // aspect_ratio_info
    if (BitstreamGetBits(bs, 4) == MPG_VOL_AR_EXTPAR)
    {
        // par_width
        BitstreamSkip(bs, 8);
        // par_height
        BitstreamSkip(bs, 8);
    }

    // vol_control_parameters
    if (BitstreamGetBit(bs))
    {
        // chroma_format
        BitstreamSkip(bs, 2);
        // low_delay
        BitstreamSkip(bs, 1);

        // vbv_parameters
        if (BitstreamGetBit(bs))
        {
            // first_half_bitrate
            BitstreamSkip(bs, 15);
            READ_MARKER(bs);

            // latter_half_bitrate
            BitstreamSkip(bs, 15);
            READ_MARKER(bs);

            // first_half_vbv_buffer_size
            BitstreamSkip(bs, 15);
            READ_MARKER(bs);

            // latter_half_vbv_buffer_size
            BitstreamSkip(bs, 3);
            // first_half_vbv_occupancy
            BitstreamSkip(bs, 11);
            READ_MARKER(bs);

            // latter_half_vbv_occupancy
            BitstreamSkip(bs, 15);
            READ_MARKER(bs);
        }
    }

    // video_object_layer_shape
    pVOLInfo->dwShape = BitstreamGetBits(bs, 2);
    if (pVOLInfo->dwShape != 0)  // 0 : rectangular shape
    {
        return (GPP_ERROR_DECODE_MPG_VOL_SHAP);
    }
    READ_MARKER(bs);

    // vop_time_increment_resolution
    pVOLInfo->dwTimeIncRes = BitstreamGetBits(bs, 16);

    // calculate need bits
    if (pVOLInfo->dwTimeIncRes > 0)
    {
        pVOLInfo->dwTimeCodeBit = Log2BitNum((pVOLInfo->dwTimeIncRes - 1));
    }
    READ_MARKER(bs);

    // fixed_vop_rate
    if (BitstreamGetBit(bs))
    {
        // fixed_vop_time_increment
        pVOLInfo->dwFixedTimeInc = BitstreamGetBits(bs, pVOLInfo->dwTimeCodeBit);
    }

    if (pVOLInfo->dwShape != MPG_VOL_SHAPE_BINARY_ONLY)
    {
        if (pVOLInfo->dwShape == MPG_VOL_SHAPE_RECTANGULAR)
        {
            READ_MARKER(bs);

            // video_object_layer_width
            pVOLInfo->dwFrameWidth = BitstreamGetBits(bs, 13);
            READ_MARKER(bs);

            // video_object_layer_height
            pVOLInfo->dwFrameHeight = BitstreamGetBits(bs, 13);
            READ_MARKER(bs);
        }

        // interlaced
        if (BitstreamGetBit(bs))
        {
            return (GPP_ERROR_DECODE_MPG_VOL_INTERLACE);
        }

        // obmc_disable
        if (!BitstreamGetBit(bs))
        {
            return (GPP_ERROR_DECODE_MPG_VOL_OBMC);
        }

        // sprite_enable
        if (BitstreamGetBits(bs, (pVOLInfo->dwVOLVerID == 1 ? 1 : 2)))
        {
            return (GPP_ERROR_DECODE_MPG_VOL_SPRITE);
        }

        // not_8_bit
        if (BitstreamGetBit(bs))
        {
            // quant_precision
            pVOLInfo->dwQuantBits = BitstreamGetBits(bs, 4);

            // bits_per_pixel
            BitstreamSkip(bs, 4);
        }
        else
        {
            pVOLInfo->dwQuantBits = 5;
        }

        // quant_type
        pVOLInfo->dwQuantType = BitstreamGetBit(bs);

        if (pVOLInfo->dwQuantType)
        {
            unsigned long tmpvar, i1, j1;

            tmpvar = BitstreamGetBit(bs); // load_intra_quant_mat
            if (tmpvar == 1)
            {
                i1=0;
                do {
                    tmpvar = BitstreamGetBits(bs, 8);
                    pVOLInfo->QTable[0][*(zigzag_i+i1)] = (unsigned char) tmpvar;
                } while ((pVOLInfo->QTable[0][*(zigzag_i+i1)]!=0)&&(++i1<64));

                for (j1=i1;j1<64;j1++)
                    pVOLInfo->QTable[0][*(zigzag_i+j1)]=pVOLInfo->QTable[0][*(zigzag_i+i1-1)];
            }
            else
            {
                for (i1=0; i1<64; i1++)
                    pVOLInfo->QTable[0][i1] = mpeg_iqmat_def[i1];
            }

            tmpvar = BitstreamGetBit(bs); // load_nonintra_quant_mat
            if (tmpvar == 1)
            {
                i1=0;
                do {
                    tmpvar = BitstreamGetBits(bs, 8);
                    pVOLInfo->QTable[1][*(zigzag_i+i1)] = (unsigned char) tmpvar;
                } while ((pVOLInfo->QTable[1][*(zigzag_i+i1)]!=0)&&(++i1<64));

                for (j1=i1;j1<64;j1++)
                    pVOLInfo->QTable[1][*(zigzag_i+j1)]=pVOLInfo->QTable[1][*(zigzag_i+i1-1)];
            }
            else
            {
                for (i1=0; i1<64; i1++)
                    pVOLInfo->QTable[1][i1] = mpeg_nqmat_def[i1];
            }
        }

        // quarter_sample
        if(pVOLInfo->dwVOLVerID != 1)
        {
            BitstreamSkip(bs, 1);
        }

        // complexity_estimation_disable
        //<-- Honda add start
        pVOLInfo->dwComplexity_estimation_disable = BitstreamGetBit(bs);
        if (!pVOLInfo->dwComplexity_estimation_disable)
        {
            pVOLInfo->vopComplexity.dwEstimation_method = BitstreamGetBits(bs, 2);
            if ((pVOLInfo->vopComplexity.dwEstimation_method == 0) ||
                (pVOLInfo->vopComplexity.dwEstimation_method == 1))
            {
                pVOLInfo->vopComplexity.dwShape_complexity_estimation_disable = BitstreamGetBit(bs);
                if (!pVOLInfo->vopComplexity.dwShape_complexity_estimation_disable)
                {
                    pVOLInfo->vopComplexity.dwOpaque = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwTransparent = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwIntra_cae = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwInter_cae = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwNo_update = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwUpSampling = BitstreamGetBit(bs);
                }
                pVOLInfo->vopComplexity.dwTexture_complexity_estimation_set_1_disable = BitstreamGetBit(bs);
                if (!pVOLInfo->vopComplexity.dwTexture_complexity_estimation_set_1_disable)
                {
                    pVOLInfo->vopComplexity.dwIntra_blocks = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwInter_blocks = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwInter4v_blocks = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwNo_coded_blocks = BitstreamGetBit(bs);
                }
                READ_MARKER(bs);
                pVOLInfo->vopComplexity.dwTexture_complexity_estimation_set_2_disable = BitstreamGetBit(bs);
                if (!pVOLInfo->vopComplexity.dwTexture_complexity_estimation_set_2_disable)
                {
                    pVOLInfo->vopComplexity.dwDct_coefs = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwDct_lines = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwVlc_symbols = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwVlc_bits = BitstreamGetBit(bs);
                }
                pVOLInfo->vopComplexity.dwMotion_compensation_complexity_disable = BitstreamGetBit(bs);
                if (!pVOLInfo->vopComplexity.dwMotion_compensation_complexity_disable)
                {
                    pVOLInfo->vopComplexity.dwApm = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwNpm = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwInterpolate_mc_q = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwForw_back_mc_q = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwHalfpel2 = BitstreamGetBit(bs);
                    pVOLInfo->vopComplexity.dwHalfpel4 = BitstreamGetBit(bs);
                }
                READ_MARKER(bs);
                if (pVOLInfo->vopComplexity.dwEstimation_method == 1)
                {
                    pVOLInfo->vopComplexity.dwVersion2_complexity_estimation_disable = BitstreamGetBit(bs);
                    if (!pVOLInfo->vopComplexity.dwVersion2_complexity_estimation_disable)
                    {
                        pVOLInfo->vopComplexity.dwSadct = BitstreamGetBit(bs);
                        pVOLInfo->vopComplexity.dwQuarterpel = BitstreamGetBit(bs);
                    }
                }
            }
        }
        // Honda add ebd -->

        // resync_marker_disable
        pVOLInfo->dwResyncMarker = BitstreamGetBit(bs);

        // data_partitioned
        pVOLInfo->dwDataPartition = BitstreamGetBit(bs);
        if (pVOLInfo->dwDataPartition)
        {
            // reversible_vlc
            pVOLInfo->dwRevVLC = BitstreamGetBit(bs);
        }

        if (pVOLInfo->dwVOLVerID != 1)
        {
            if (BitstreamGetBit(bs)) // newpred_enable
            {
                BitstreamSkip(bs, 2); // requested_upstream_message_type
                BitstreamSkip(bs, 1); // mewpred_segment_type
            }
            else
            {
                BitstreamSkip(bs, 1); // reduce_resolution_vop_enable
            }
        }

        // scalability
        if (BitstreamGetBit(bs))
        {
            return (GPP_ERROR_DECODE_MPG_VOL_SCALABILITY);
        }
    }

    return (GPP_SUCCESS);
}



//-----< mpg_GetShortHeaderParameter >-----  from honda
SXA_DMXECODE_E
sxaDmx3GPP_GetShortHeaderParameter(SXA_DMXSHORTHEADERINFO_T  *pSHeaderInfo,
                                   SXA_DMXDECODESIZE_T       *pDecodeSize,
                                   BIT_STREAM                *bs)
{
    // [dvyu] ISO 14496-2, Sec 6.3.5.2 (P.127)

    // Short video header.  Skip short_video_start_marker,
    BitstreamSkip(bs, 22);

    // temporal reference
    pSHeaderInfo->dwVOPTimeInc = BitstreamGetBits(bs, 8);

    // skip marker & zero bit
    BitstreamSkip(bs, 5);

    // source_format
    pSHeaderInfo->dwSrcFmt = BitstreamGetBits(bs, 3);

    switch (pSHeaderInfo->dwSrcFmt)
    {
        // sub-QCIF
    case 1:
        pSHeaderInfo->dwFrameWidth = 128;
        pSHeaderInfo->dwFrameHeight = 96;
        break;
        // QCIF
    case 2:
        pSHeaderInfo->dwFrameWidth = 176;
        pSHeaderInfo->dwFrameHeight = 144;
        break;
        // CIF
    case 3:
        pSHeaderInfo->dwFrameWidth = 352;
        pSHeaderInfo->dwFrameHeight = 288;
        break;
        // 4CIF
    case 4:
        pSHeaderInfo->dwFrameWidth = 704;
        pSHeaderInfo->dwFrameHeight = 576;
        break;
        // 16CIF
    case 5:
        pSHeaderInfo->dwFrameWidth = 1408;
        pSHeaderInfo->dwFrameHeight = 1152;
        break;
    default:
        return (GPP_ERROR_DECODE_MPG_SHEADER_RESOLUTION);
    }

    // picture_coding_type
    if (BitstreamGetBit(bs))
    {
        // P frame
        pSHeaderInfo->VOPInfo.dwCodingType = MPG_P_VOP;
    }
    else
    {
        pSHeaderInfo->VOPInfo.dwCodingType = MPG_I_VOP;
    }

    // skip 4 reserved 0 bits
    BitstreamSkip(bs, 4);

    // vop_quant
    pSHeaderInfo->VOPInfo.dwVOPQuant = BitstreamGetBits(bs, 5);

    // skip zero_bit
    BitstreamSkip(bs, 1);

    while (BitstreamGetBit(bs) == 1)
    {
        // pei and psupp
        BitstreamSkip(bs, 8);
    }

    pDecodeSize->pDataStart = (unsigned char*)bs->tail + (bs->pos / 8);
    pDecodeSize->dwStartBits = (bs->pos % 8);

    // find next short header start code
    //FindNextShortVideoStartCode(bs);

    //calculate data size to decoder
    //pDecodeSize->pDataEnd = (MMP_UINT8*)bs->tail + (bs->pos / 8)+1024;
    //pDecodeSize->dwDataSize = (MMP_UINT32)(pDecodeSize->pDataEnd - pDecodeSize->pDataStart);

    // store info. to g_BSData
    pSHeaderInfo->VOPInfo.dwRounding   = 0;
    pSHeaderInfo->VOPInfo.dwIntraDCThr = 0;
    pSHeaderInfo->VOPInfo.dwFCode      = 1;
    pSHeaderInfo->dwTimeCodeBit = 8;
    // add by dkwang for vopcoded flag
    pSHeaderInfo->VOPInfo.dwVopCoded   = 1;

    return (SXA_DMXECODE_SOK);
}



//-----< mpg_GetVOPParameter >----- from honda
SXA_DMXECODE_E
sxaDmx3GPP_GetVOPParameter(SXA_DMXVOPINFO_T    *pVOPInfo,
                           SXA_DMXDECODESIZE_T *pDecodeSize,
                           SXA_DMXVOLINFO_T    *pVOLInfo,
                           BIT_STREAM          *bs)
{
    // [dvyu] ISO 14496-2, Sec 6.2.5 (P.40)

    pVOPInfo->dwCodingType = 0;
    pVOPInfo->dwFCode      = 0;
    pVOPInfo->dwIntraDCThr = 0;
    pVOPInfo->dwRounding   = 0;
    pVOPInfo->dwTimeBase   = 0;
    pVOPInfo->dwVOPQuant   = 0;
    pVOPInfo->dwVOPTimeInc = 0;
    // vop_start_code
    while ((BitstreamShowBits(bs, 32) != 0x000001b6) && ((BitstreamPos(bs) >> 3) < bs->length))
    {
        BitstreamSkip(bs, 8);
        FindNextStartCode(bs);
    }

    if ((BitstreamPos(bs) >> 3) >= bs->length)
    {
        return (SXA_DMXECODE_EVOP_CODING_TYPE);
    }

    if (BitstreamShowBits(bs, 32) == 0x000001b6)
    {
        BitstreamGetBits(bs, 32);
    }

    BitstreamByteAlign(bs);
    //    BitstreamSkip(bs, 32);

    // vop_coding_type
    pVOPInfo->dwCodingType = BitstreamGetBits(bs, 2);

    // B-picture & sprite-picture are not supported
    if ((pVOPInfo->dwCodingType == MPG_B_VOP) || (pVOPInfo->dwCodingType == MPG_S_VOP))
    {
        return (SXA_DMXECODE_EVOP_CODING_TYPE);
    }

    // modulo_time_base
    while (BitstreamGetBit(bs) != 0)
    {
        pVOPInfo->dwTimeBase++;
    }
    READ_MARKER(bs);

    // vop_time_increment
    if (pVOLInfo->dwTimeCodeBit)
    {
        // get value per frame
        pVOPInfo->dwVOPTimeInc = BitstreamGetBits(bs, pVOLInfo->dwTimeCodeBit);
    }
    READ_MARKER(bs);

    // vop_coded
    //<-- Honda add start
    //if (!BitstreamGetBit(bs))
    pVOPInfo->dwVopCoded = BitstreamGetBit(bs);
    if (!pVOPInfo->dwVopCoded)
        // Honda add end -->
    {
        // not coded, find next start code
        //    FindNextStartCode(bs);
        return (SXA_DMXECODE_SOK);
    }

    if (pVOLInfo->dwShape != MPG_VOL_SHAPE_BINARY_ONLY)
    {
        if (pVOPInfo->dwCodingType == MPG_P_VOP)
        {
            // rounding_type
            pVOPInfo->dwRounding = BitstreamGetBit(bs);
        }
        //<-- Honda add start
        if (!pVOLInfo->dwComplexity_estimation_disable)
        {
            if ((pVOLInfo->vopComplexity.dwEstimation_method == 0) ||
                (pVOLInfo->vopComplexity.dwEstimation_method == 1))
            {
                if (pVOPInfo->dwCodingType == MPG_I_VOP)
                {
                    if (pVOLInfo->vopComplexity.dwOpaque)
                    {
                        pVOPInfo->vopDcecs.dwOpaque = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwTransparent)
                    {
                        pVOPInfo->vopDcecs.dwTransparent = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwIntra_cae)
                    {
                        pVOPInfo->vopDcecs.dwIntra_cae = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwInter_cae)
                    {
                        pVOPInfo->vopDcecs.dwInter_cae = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwNo_update)
                    {
                        pVOPInfo->vopDcecs.dwNo_update = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwUpSampling)
                    {
                        pVOPInfo->vopDcecs.dwUpSampling = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwIntra_blocks)
                    {
                        pVOPInfo->vopDcecs.dwIntra_blocks = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwNo_coded_blocks)
                    {
                        pVOPInfo->vopDcecs.dwNo_coded_blocks = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwDct_coefs)
                    {
                        pVOPInfo->vopDcecs.dwDct_coefs = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwDct_lines)
                    {
                        pVOPInfo->vopDcecs.dwDct_lines = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwVlc_symbols)
                    {
                        pVOPInfo->vopDcecs.dwVlc_symbols = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwVlc_bits)
                    {
                        pVOPInfo->vopDcecs.dwVlc_bits = BitstreamGetBits(bs,4);
                    }
                    if (pVOLInfo->vopComplexity.dwSadct)
                    {
                        pVOPInfo->vopDcecs.dwSadct = BitstreamGetBits(bs,8);
                    }
                }
                else if (pVOPInfo->dwCodingType == MPG_P_VOP)
                {
                    if (pVOLInfo->vopComplexity.dwOpaque)
                    {
                        pVOPInfo->vopDcecs.dwOpaque = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwTransparent)
                    {
                        pVOPInfo->vopDcecs.dwTransparent = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwIntra_cae)
                    {
                        pVOPInfo->vopDcecs.dwIntra_cae = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwInter_cae)
                    {
                        pVOPInfo->vopDcecs.dwInter_cae = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwNo_update)
                    {
                        pVOPInfo->vopDcecs.dwNo_update = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwUpSampling)
                    {
                        pVOPInfo->vopDcecs.dwUpSampling = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwIntra_blocks)
                    {
                        pVOPInfo->vopDcecs.dwIntra_blocks = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwNo_coded_blocks)
                    {
                        pVOPInfo->vopDcecs.dwNo_coded_blocks = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwDct_coefs)
                    {
                        pVOPInfo->vopDcecs.dwDct_coefs = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwDct_lines)
                    {
                        pVOPInfo->vopDcecs.dwDct_lines = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwVlc_symbols)
                    {
                        pVOPInfo->vopDcecs.dwVlc_symbols = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwVlc_bits)
                    {
                        pVOPInfo->vopDcecs.dwVlc_bits = BitstreamGetBits(bs,4);
                    }
                    if (pVOLInfo->vopComplexity.dwInter_blocks)
                    {
                        pVOPInfo->vopDcecs.dwInter_blocks = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwInter4v_blocks)
                    {
                        pVOPInfo->vopDcecs.dwInter4v_blocks = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwApm)
                    {
                        pVOPInfo->vopDcecs.dwApm = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwNpm)
                    {
                        pVOPInfo->vopDcecs.dwNpm = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwForw_back_mc_q)
                    {
                        pVOPInfo->vopDcecs.dwForw_back_mc_q = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwHalfpel2)
                    {
                        pVOPInfo->vopDcecs.dwHalfpel2 = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwHalfpel4)
                    {
                        pVOPInfo->vopDcecs.dwHalfpel4 = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwSadct)
                    {
                        pVOPInfo->vopDcecs.dwSadct = BitstreamGetBits(bs,8);
                    }
                    if (pVOLInfo->vopComplexity.dwQuarterpel)
                    {
                        pVOPInfo->vopDcecs.dwQuarterpel = BitstreamGetBits(bs,8);
                    }
                }
            }
        }
        // Honda add end -->

        // intra_dc_vlc_threshold
        pVOPInfo->dwIntraDCThr = BitstreamGetBits(bs,3);
    }

    // vop_quant
    pVOPInfo->dwVOPQuant = BitstreamGetBits(bs, pVOLInfo->dwQuantBits);

    if (pVOPInfo->dwCodingType != MPG_I_VOP)
    {
        // fcode_forward
        pVOPInfo->dwFCode = BitstreamGetBits(bs, 3);
    }

    pDecodeSize->pDataStart = (unsigned char*)bs->tail + (bs->pos / 8);
    pDecodeSize->dwStartBits = (bs->pos%8);


    // find next start code
    //FindNextStartCode(bs);

    //calculate data size to decoder
    pDecodeSize->pDataEnd = (unsigned char*)bs->start + bs->length;
    pDecodeSize->dwDataSize = (unsigned long)(bs->length - ((unsigned char*)pDecodeSize->pDataStart - (unsigned char*)bs->start));

    return (SXA_DMXECODE_SOK);
}


#define SMTK_AUDIO_M4A_HEADER_SIZE 7
static unsigned char    adtsHeader[8];   
static char sampleBuf[64*1024];

int dump_AudioSampleInfo(PAL_TCHAR* filename,SXA_HANDLE_T hDmx)
{
    unsigned int            i1;
    int dump_all, from_idx, to_idx;
    SXA_DMXSAMPLEINFO_T     sample_info;
    SXA_DMXMEDIAINFO_T      mediaInfo;
    PAL_FILE *fOut = NULL;
    PAL_FILE *pFile = NULL;
    //MMP_WCHAR* outfile = L"D:\\audio_testing\\m4a\\s.aac";
    char* outfile = "D:\\audio_testing\\m4a\\s.aac";
    SXA_DMXADTSHEADER_T sxaAdts;
    int nResult;
    int nAACSize = 0;
    int nPos;

    dump_all = 1;
    sxaDmxGetProp(hDmx, SXA_DMXPROP_MEDIAIFNO, &mediaInfo);

    /*--- open file ---*/
  //  fOut = PalTFileOpen(outfile, PAL_FILE_WB, MMP_NULL);
  //  pFile = PalTFileOpen(filename, PAL_FILE_RB, MMP_NULL);
  //fOut = fopen(outfile,"wb");
    if (pFile == NULL)
    {
       // return (GPP_ERROR_DECODE_FILE_OPEN);
    }

    if (dump_all)
    {
        from_idx = 1;
        to_idx = (int)mediaInfo.nASampleCount;
    }
    else
    {
        from_idx = (from_idx >= 1) ? from_idx : 1;
        to_idx = (to_idx <= (int)mediaInfo.nASampleCount) ? to_idx : mediaInfo.nASampleCount;
    }

    //gtSample_info = PalHeapAlloc(PAL_HEAP_DEFAULT, (mediaInfo.nASampleCount+1)*sizeof(SXA_DMXSAMPLEINFO_T));

    //PalHeapFree(PAL_HEAP_DEFAULT, gtSample_info);

   // nResult = PalFileTell(0,NULL);
    gtSample_info[0].nIndex = mediaInfo.nASampleCount;
    if ( gtSample_info[0].nIndex > SXA_DATA_SIZE) {
        printf("[Sxa] index %d > %d , #line %d \n",gtSample_info[0].nIndex,SXA_DATA_SIZE,__LINE__);
        gtSample_info[0].nIndex = SXA_DATA_SIZE;
    }
    
    for (i1 = 1; i1 <= mediaInfo.nASampleCount; i1++)
    {
        sample_info.nIndex = i1;
        nResult = sxaDmxGetProp(hDmx, SXA_DMXPROP_ASAMPLEINFO, &sample_info);
        /*m_OutputStr.AppendFormat(_T("nIndex     = %ld\r\n"), sample_info.nIndex);
        m_OutputStr.AppendFormat(_T("nOffset    = 0x%X (%ld)\r\n"), sample_info.nOffset, sample_info.nOffset);
        m_OutputStr.AppendFormat(_T("nSize      = 0x%X (%ld)\r\n"), sample_info.nSize, sample_info.nSize);
        m_OutputStr.AppendFormat(_T("nTimeStamp = %ld\r\n"), sample_info.nTimeStamp);
        m_OutputStr.AppendFormat(_T("nDelta     = %ld\r\n"), sample_info.nDelta);
        m_OutputStr.AppendFormat(_T("---------------------------\r\n"));*/
        gtSample_info[i1].nIndex = sample_info.nIndex;
        gtSample_info[i1].nOffset = sample_info.nOffset;
        gtSample_info[i1].nSize = sample_info.nSize;
        if (sample_info.nSize==0 || sample_info.nSize>=1500){
            printf("dump_AudioSampleInfo size error %d \n",sample_info.nSize);
            gtSample_info[i1].nSize=0;
        }
        gtSample_info[i1].nTimeStamp = sample_info.nTimeStamp;
        gtSample_info[i1].nDelta = sample_info.nDelta;
#if 0
        printf("nIndex     = %ld,  %d \r\n", sample_info.nIndex,nResult);
        printf("nOffset    = 0x%X (%ld)\r\n", sample_info.nOffset, sample_info.nOffset);
        printf("nSize      = 0x%X (%ld)\r\n", sample_info.nSize, sample_info.nSize);
        printf("nTimeStamp = %ld\r\n", sample_info.nTimeStamp);
        printf("nDelta     = %ld\r\n", sample_info.nDelta);
        printf("---------------------------\r\n");
#endif

    }
    //PalFileClose(fOut,MMP_NULL);
    //PalFileClose(pFile,MMP_NULL);
    gtSample_info[0].nTimeStamp = gtSample_info[i1-1].nTimeStamp;
    gtSample_info[0].nOffset = gtSample_info[i1-1].nOffset;
    
    printf("[Sxa] audio dump_AudioSampleInfo index %d time %d \n", gtSample_info[0].nIndex,gtSample_info[0].nTimeStamp);
    setDataParsing(0);    
    // write data
/*

    for (i1 = 1; i1 <= gtSample_info[0].nIndex; i1++){
        memset(adtsHeader, 0, 8);
        sxaAdts.pAdtsHeader = adtsHeader;
        sxaAdts.nAacFrameLength = gtSample_info[i1].nSize;
        sxaDmxGetProp(
                    hDmx,
                    SXA_DMXPROP_ADTSHEADER,
                    &sxaAdts);
        fwrite(adtsHeader,1,SMTK_AUDIO_M4A_HEADER_SIZE,fOut);
        nPos = PalTFileTell(0,NULL);
        nResult = PalTFileSeek(pFile, gtSample_info[i1].nOffset-nPos, PAL_SEEK_CUR, MMP_NULL);
        nResult = PalTFileRead(sampleBuf, 1, gtSample_info[i1].nSize, pFile, MMP_NULL);
        fwrite(sampleBuf,1,gtSample_info[i1].nSize,fOut);

    }
    fclose(fOut);
*/
    return gtSample_info[0].nIndex;
}


int sxaDmxGetAudioDuration()
{

    return gtSample_info[0].nTimeStamp;
}


char* getAudioSample(SXA_HANDLE_T hDmx,int nIndex,unsigned int* pSize,unsigned int nSeekSize)
{
    int nResult=0;
    MMP_LONG nPos;
    SXA_DMXADTSHEADER_T sxaAdts;

    memset(adtsHeader, 0, 8);
    if (gtSample_info==NULL){
        printf("[Sxa] getAudioSample gtSample_info 0x%x  #line %d  \n",gtSample_info,__LINE__);
    }
    sxaAdts.pAdtsHeader = adtsHeader;
    sxaAdts.nAacFrameLength = gtSample_info[nIndex].nSize;

    sxaDmxGetProp(
                hDmx,
                SXA_DMXPROP_ADTSHEADER,
                &sxaAdts);
    //fwrite(adtsHeader,1,SMTK_AUDIO_M4A_HEADER_SIZE,fOut);
    nPos = PalTFileTell(0,NULL);
    if (gtSample_info[nIndex].nOffset <  nSeekSize) {
        printf("[Sxa] getAudioSample nIndex %d  err %d < %d \n",nIndex,gtSample_info[nIndex].nOffset,nSeekSize);
    }
    
    if (gtSample_info[nIndex].nOffset-nPos > 16*1024){
        printf("[Sxa] getAudioSample offset %d > nPos %d , nIndex %d  ,#line %d \n",gtSample_info[nIndex].nOffset,nPos,nIndex,__LINE__);
    }
    
    if (nIndex%200==0)
        printf("[Sxa] index %d pos %d nOffset %d size %d ,total %d %d\n",nIndex,nPos,gtSample_info[nIndex].nOffset,gtSample_info[nIndex].nSize,gtSample_info[0].nIndex,gtSample_info[0].nOffset);


    nResult = PalTFileSeek(NULL, gtSample_info[nIndex].nOffset-(unsigned int)nPos/*-nSeekSize*/, PAL_SEEK_CUR, MMP_NULL);
    memcpy(sampleBuf,adtsHeader,7);
    nResult = PalTFileRead(&sampleBuf[7], 1, gtSample_info[nIndex].nSize, MMP_NULL, MMP_NULL);
    //fwrite(sampleBuf,1,gtSample_info[i1].nSize,fOut);
//    printf("0x%x \n",sampleBuf);
    *pSize = gtSample_info[nIndex].nSize+7;
    
    return sampleBuf;

}

int getAudioSeekIndex(int nTime,int* pIndex,unsigned int* pOffset)
{
    int nResult =0;
    int i=0;
    if (nTime > gtSample_info[0].nTimeStamp){
        printf("[Sxa] getAudioSeekIndex time error %d > %d \n",nTime,gtSample_info[0].nTimeStamp);
        nResult = -1;
        return nResult;
    }

//    startM4aStreamBuffer();
    
    for (i=1;i<gtSample_info[0].nIndex;i++) {
        if (gtSample_info[i].nTimeStamp > nTime) {

            break;
        }            
    }
    
    if (i>=gtSample_info[0].nIndex){
        printf("[Sxa] getAudioSeekIndex not found %d \n",i);
        nResult = -1;
        return nResult;
    }

    printf("[Sxa] getAudioSeekIndex index %d nOffset %d \n",gtSample_info[i-1].nIndex, gtSample_info[i-1].nOffset);
    
    *pIndex = gtSample_info[i-1].nIndex;
    *pOffset = gtSample_info[i-1].nOffset;
    return nResult;
}

int setAudioSeekOffset(unsigned int  Offset){
    int nResult =0;
    startM4aStreamBuffer();
    printf("[Sxa] setAudioSeekOffset %d \n",Offset);
    setDataPosition(Offset);

    return nResult;

}

GPP_RESULT
sxaDmx3GPP_ParseMoovBox(
                        MMP_3GPP_DECODE_3GPPDATA  *pGppData,
                        SXA_DMXOPENEXPARAM_T      *pParam,
						int nInType)
{
    GPP_RESULT          result = 0;
    unsigned long       dwBoxSize = 0;
    unsigned long       dwBoxType = 0;
    //2004.12.09 s034 adding new data boxes
    unsigned long       dwTRAKBoxSize = 0;
    //2004.12.09 end
    signed long         dwFileERRCode = 0;
    int                 mdatFlag = 0;
    int                 moovFlag = 0;
    PAL_FILE           *pFile = NULL;
    unsigned int        i;
    int nStopParsing = 0;
#if (SUPPORT_NEW_PCM_INFO)
    unsigned long       nAudioType;
#endif
    printf("sxaDmx3GPP_ParseMoovBox set data input \n\n");

    startM4aStreamBuffer();
    setDataInput(nInType);
    setDataParsing(1);
#if (WATCHDOG_ENABLE)
    lastClock = PalGetClock();
#endif

    gppOpenExParam.nOffset = pParam->nOffset;
    gppOpenExParam.nLength = pParam->nLength;

    /*--- open file ---*/
    pFile = PalTFileOpen(pParam->ptPathName, PAL_FILE_RB | ((pParam->nKey << 24) | (pParam->nMode << 16)), MMP_NULL);
    if (pFile == NULL)
    {
        //return (GPP_ERROR_DECODE_FILE_OPEN);
    }

    pGppData->gppFile = pFile;
    gppFilePos = gppOpenExParam.nOffset;

    /*--- get file size ---*/
    pGppData->dwFileSize = gppOpenExParam.nLength;

    dwFileERRCode = PalTFileSeek(pFile, gppFilePos, PAL_SEEK_SET, MMP_NULL);
    if (dwFileERRCode < 0)
    {
        return (GPP_ERROR_DECODE_FILE_SEEK);
    }

#if (WATCHDOG_ENABLE)
    CheckResetWatchDog(__LINE__);
#endif

    while (/*gppFilePos < gppOpenExParam.nOffset + pGppData->dwFileSize && */ nStopParsing==0)
    {
#if (WATCHDOG_ENABLE)
        CheckResetWatchDog(__LINE__);
#endif
        
        /*--- read box size from file ---*/
        dwFileERRCode = PalTFileRead(&dwBoxSize, 4, 1, pFile, MMP_NULL);
        if (dwFileERRCode != 1)
        {
            //printf("(0) GPP_ERROR_DECODE_FILE_READ (%d)\n", dwFileERRCode);
            return (GPP_ERROR_DECODE_FILE_READ);
        }
        
        if (getDataParsing()==0){
            printf("[Sxa] sxaDmx3GPP_ParseMoovBox stop parsing #line %d  \n",__LINE__);
            return (GPP_ERROR_DECODE_FILE_READ);
        }
        gppFilePos += 4;
        BS_WAP(dwBoxSize);

        if (dwBoxSize < 8)
        {
            // The size of an atom should be 8 at least.
            break;
        }

        /*--- read box type from file ---*/
        dwFileERRCode = PalTFileRead(&dwBoxType, sizeof(unsigned long), 1, pFile, MMP_NULL);
        if (dwFileERRCode != 1)
        {
            return (GPP_ERROR_DECODE_FILE_READ);
        }

        gppFilePos += sizeof(unsigned long);
        BS_WAP(dwBoxType);

        switch(dwBoxType)
        {
            /*--- default, FTYP, VMHD, SMHD, DREF, CPRT, FREE, SKIP, IODS, NMHD, ELST, UDTA ---*/
        default : // unknown Box type
        case FTYP_BOX_TYPE : //file type and compatibility
        case VMHD_BOX_TYPE : //video media header, overall information
        case SMHD_BOX_TYPE : //sound media header, overall information
        case HMHD_BOX_TYPE : //hint media header, overall information
        case DREF_BOX_TYPE : //data reference box
        case CPRT_BOX_TYPE : //copyright
        case FREE_BOX_TYPE : //free space
        case SKIP_BOX_TYPE : //free space
        case IODS_BOX_TYPE : //Object description box
        case NMHD_BOX_TYPE : //mpeg-4 media header
        case ELST_BOX_TYPE : //an edit list
            dwFileERRCode = PalTFileSeek(pFile, (dwBoxSize - 8L),PAL_SEEK_CUR, MMP_NULL);
            if (dwFileERRCode < 0)
            {
                return (GPP_ERROR_DECODE_FILE_SEEK);
            }
            gppFilePos += (dwBoxSize - 8L);
            break;

            /*--- MINF, DINF, STBL ---*/
        case MINF_BOX_TYPE : //media information containere
        case DINF_BOX_TYPE : //data information box, container
        case STBL_BOX_TYPE : //sample table box
            break;

            /*--- EDTS, MDIA ---*/
        case EDTS_BOX_TYPE : //edit list container
        case MDIA_BOX_TYPE : //container for the media information in a track
            dwTRAKBoxSize = dwTRAKBoxSize - dwBoxSize;
            break;

            /*--- MOOV ---*/
        case MOOV_BOX_TYPE : //container for all the meta-data
            if (moovFlag)
            {
                PalTFileClose(pFile, MMP_NULL);
                return (GPP_ERROR_DECODE_FILE_PARSE);
            }
            else
            {
                moovFlag = 1;
            }
            break;

            /*--- TREF ---*/
        case TREF_BOX_TYPE : //track reference container
            dwFileERRCode = PalTFileSeek(pFile, (dwBoxSize - 8L),PAL_SEEK_CUR, MMP_NULL);
            if (dwFileERRCode < 0)
            {
                return (GPP_ERROR_DECODE_FILE_SEEK);
            }
            gppFilePos += (dwBoxSize - 8L);
            dwTRAKBoxSize = dwTRAKBoxSize - dwBoxSize;
            break;

            /*--- TRAK ---*/
        case TRAK_BOX_TYPE : //container for an individual track or stream
            if (pGppData->dwTrakCount + 1 > MAX_TRAK_COUNT)
            {
                return (GPP_ERROR_DECODE_MOOV_PARSE);
            }
            (pGppData->dwTrakCount)++;
            dwTRAKBoxSize = dwBoxSize - 8;
            break;

            /*--- MDAT ---*/
        case MDAT_BOX_TYPE :
            if (mdatFlag)
            {
                // return (GPP_ERROR_DECODE_FILE_PARSE);
            }
            else
            {
                mdatFlag = 1;
            }
            dwFileERRCode = PalTFileSeek(pFile, (dwBoxSize - 8L),PAL_SEEK_CUR, MMP_NULL);
            if (dwFileERRCode < 0)
            {
                return (GPP_ERROR_DECODE_FILE_SEEK);
            }
            gppFilePos += (dwBoxSize - 8L);
            break;

            /*--- MVHD ---*/
        case MVHD_BOX_TYPE : //movie header, overall declarations
            {
                unsigned char* temp = PalHeapAlloc(PAL_HEAP_DEFAULT, dwBoxSize-8);
                if (temp == NULL)
                {
                    return GPP_ERROR_DECODE_MALLOC;
                }

                dwFileERRCode = PalTFileRead(temp, dwBoxSize-8, 1, pFile, MMP_NULL);
                if (dwFileERRCode < 0)
                {
                    PalHeapFree(PAL_HEAP_DEFAULT, temp);
                    return (GPP_ERROR_DECODE_FILE_READ);
                }
                gppFilePos += (dwBoxSize-8);

                result = sxaDmx3GPP_GetMovieHeader(temp, dwBoxSize, pGppData);
                PalHeapFree(PAL_HEAP_DEFAULT, temp);
                if (result != GPP_SUCCESS)
                {
                    return result;
                }
            }
            break;

            //--- UDTA ---
        case UDTA_BOX_TYPE : //user-data container
            if (sxaDmx3GPP_ParseUDTA(pFile, dwBoxSize, pGppData) != GPP_SUCCESS)
            {
                return result;
            }
            break;

            /*--- TKHD ---*/
        case TKHD_BOX_TYPE : //track header, overall information about the track
            {
                unsigned char* temp = PalHeapAlloc(PAL_HEAP_DEFAULT, dwBoxSize-8);
                if (temp == NULL)
                {
                    return GPP_ERROR_DECODE_MALLOC;
                }
                dwFileERRCode = PalTFileRead(temp, dwBoxSize-8, 1, pFile, MMP_NULL);
                if (dwFileERRCode < 0)
                {
                    PalHeapFree(PAL_HEAP_DEFAULT, temp);
                    return (GPP_ERROR_DECODE_FILE_READ);
                }
                gppFilePos += (dwBoxSize-8);

                result = sxaDmx3GPP_GetTrackHeader(temp, dwBoxSize, pGppData);
                PalHeapFree(PAL_HEAP_DEFAULT, temp);
                if (result != GPP_SUCCESS)
                {
                    return result;
                }
            }
            dwTRAKBoxSize = dwTRAKBoxSize - dwBoxSize;
            break;

            /*--- MDHD ---*/
        case MDHD_BOX_TYPE : //media header, overall information about the media
            {
                unsigned char* temp = PalHeapAlloc(PAL_HEAP_DEFAULT, dwBoxSize-8);
                if (temp == NULL)
                {
                    return GPP_ERROR_DECODE_MALLOC;
                }
                dwFileERRCode = PalTFileRead(temp, dwBoxSize-8, 1, pFile, MMP_NULL);
                if (dwFileERRCode < 0)
                {
                    PalHeapFree(PAL_HEAP_DEFAULT, temp);
                    return (GPP_ERROR_DECODE_FILE_READ);
                }
                gppFilePos += (dwBoxSize-8);

                result = sxaDmx3GPP_GetMediaHeader(temp, dwBoxSize, pGppData);
                PalHeapFree(PAL_HEAP_DEFAULT, temp);
                if (result != GPP_SUCCESS)
                {
                    return result;
                }
            }
            break;

            /*--- HDLR ---*/
        case HDLR_BOX_TYPE : //handler, declares the media (handler) type
            {
                unsigned char* temp = PalHeapAlloc(PAL_HEAP_DEFAULT, dwBoxSize-8);
                if (temp == NULL)
                {
                    return GPP_ERROR_DECODE_MALLOC;
                }
                dwFileERRCode = PalTFileRead(temp, dwBoxSize-8, 1, pFile, MMP_NULL);
                if (dwFileERRCode < 0)
                {
                    PalHeapFree(PAL_HEAP_DEFAULT, temp);
                    return (GPP_ERROR_DECODE_FILE_READ);
                }
                gppFilePos += (dwBoxSize-8);

                result = sxaDmx3GPP_GetHandlerReference(temp, dwBoxSize, pGppData);
                PalHeapFree(PAL_HEAP_DEFAULT, temp);
                if (result != GPP_SUCCESS)
                {
                    return result;
                }
            }
            break;

            /*--- STSD ---*/
        case STSD_BOX_TYPE : //sample description
            {
                unsigned char* temp = PalHeapAlloc(PAL_HEAP_DEFAULT, dwBoxSize-8);
                if (temp == NULL)
                {
                    return GPP_ERROR_DECODE_MALLOC;
                }
                dwFileERRCode = PalTFileRead(temp, dwBoxSize-8, 1, pFile, MMP_NULL);
                if (dwFileERRCode < 0)
                {
                    PalHeapFree(PAL_HEAP_DEFAULT, temp);
                    return (GPP_ERROR_DECODE_FILE_READ);
                }
                gppFilePos += (dwBoxSize-8);

                result = sxaDmx3GPP_GetSampleDescription(temp, dwBoxSize, pGppData);
                PalHeapFree(PAL_HEAP_DEFAULT, temp);
                if (result != GPP_SUCCESS)
                {
                    return result;
                }
            }
            break;

            /*--- STTS ---*/
        case STTS_BOX_TYPE : //(decoding) time-to-sample
            if ( sxaDmx3GPP_GetDecodingTime(pFile, dwBoxSize, pGppData) != GPP_SUCCESS)
            {
                return result;
            }
            break;

            /*--- STSC ---*/
        case STSC_BOX_TYPE : //sample-to-chunk
            if ( sxaDmx3GPP_GetSampleToChunk(pFile, dwBoxSize, pGppData) != GPP_SUCCESS)
            {
                return result;
            }
            break;

            /*--- STSZ ---*/
        case STSZ_BOX_TYPE : //sample sizes (framing)
            if ( sxaDmx3GPP_GetSampleSize(pFile, dwBoxSize, pGppData) != GPP_SUCCESS)
            {
                return result;
            }
            break;

            /*--- STCO ---*/
        case STCO_BOX_TYPE : //chunk offset
            if ( sxaDmx3GPP_GetChunkOffset(pFile, dwBoxSize, pGppData) != GPP_SUCCESS)
            {
                return result;
            }
            printf("sxaDmx3GPP_ParseMoovBox get STCO_BOX_TYPE \n");
            nStopParsing = 1;
            break;

            /*--- STSS ---*/
        case STSS_BOX_TYPE : //sync sample table (random access points)
            // { [dvyu] 2007.11.22 Modified
            if (sxaDmx3GPP_GetSyncSample(pFile, dwBoxSize, pGppData) != GPP_SUCCESS)
            {
                return result;
            }
            // } [dvyu] 2007.11.22
            break;
        }
    }   // end of while (dwFileSize)

    result = sxaDmx3GPP_GetFirstVideoTrack(pGppData, &i);
    if (result == GPP_SUCCESS)
    {
        pGppData->pFirstVideoTrak = &(pGppData->moov.trak[i]);
    }
    else
    {
        pGppData->pFirstVideoTrak = NULL;
    }

    result = sxaDmx3GPP_GetFirstAudioTrack(pGppData, &i);
    if (result == GPP_SUCCESS)
    {
        pGppData->pFirstAudioTrak = &(pGppData->moov.trak[i]);
    }
    else
    {
        pGppData->pFirstAudioTrak = NULL;
    }
#if (WATCHDOG_ENABLE)
    CheckResetWatchDog(__LINE__);
#endif

#if (SUPPORT_NEW_PCM_INFO)
    if (pGppData->pFirstAudioTrak)
    {
        nAudioType = pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.dwEntryType;
        if ((nAudioType == RAW_SOUND_TYPE) || (nAudioType == ULAW_SOUND_TYPE) || (nAudioType == SOWT_SOUND_TYPE) || (nAudioType == TWOS_SOUND_TYPE))
        {
            pGppData->pFirstAudioTrak->nSliceCount = GetAudioSliceCount(pGppData);
        }
        else
        {
            pGppData->pFirstAudioTrak->nSliceCount = 0;
        }
    }
#endif

    return (GPP_SUCCESS);
}



// from honda
void sxaDmx3GPP_BitstreamInit(BIT_STREAM *bs,
                              unsigned long *bitstream,
                              unsigned long dwStreamSize)
{
    //<-- Honda add start
    unsigned long tmp;
    unsigned long streamAddr = 0;
    unsigned long streamAddrAlign = 0;
    unsigned long offset = 0;

    streamAddr = (unsigned long)bitstream;
    streamAddrAlign = ((streamAddr >> 2) << 2);
    offset = (streamAddr % 4);

    bs->start = bs->tail = (unsigned long*)streamAddrAlign;
    if (offset != 0)
    {
        unsigned char stuff = 0;
        unsigned long i = 0;
        bs->bufa = 0;
        for (i = offset; i<4; i++)
        {
            stuff = (unsigned char)(*((unsigned char*)(streamAddrAlign + i)));
            bs->bufa |= (stuff << ((3 - i) * 8));
        }
        tmp = *(bs->start + 1);
        BS_WAP(tmp);
        bs->bufb = tmp;
    }
    else
    {
        tmp = *bitstream;
        BS_WAP(tmp);
        bs->bufa = tmp;
        tmp = *(bitstream + 1);
        BS_WAP(tmp);
        bs->bufb = tmp;
    }

    bs->buf = 0;
    bs->pos = offset * 8;
    bs->length = dwStreamSize + offset;
    // Honda add end-->
}



SXA_DMXVOLINFO_T *sxaDmx3GPP_GetVideoVolInfo(MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    if (pGppData->pFirstVideoTrak == NULL)
    {
        return NULL;
    }

    if (pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.dwEntryType == MP4V_VIDEO_TYPE)
    {
        return &(pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.mp4vEntry.esdBox.volInfo);
    }
    else
    {
        return NULL;
    }
}



unsigned short sxaDmx3GPP_GetVideoWidth(MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    if (pGppData->pFirstVideoTrak == NULL)
    {
        return 0;
    }

    switch (pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.dwEntryType)
    {
    case MP4V_VIDEO_TYPE:
        // Kevin@2006.3.29  always return size within esdBox
        //return pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.mp4vEntry.wWidth;
        return (unsigned short)pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.mp4vEntry.esdBox.volInfo.dwFrameWidth;
        // ~Kevin@2006.3.29
        //  case AVC_VIDEO_TYPE:
        //  // TO DO should parse vui to get the real display size
        //   //return (MMP_UINT16)((pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.avcEntry.avcspec.avcInfo.avcSPS.mbWidthMinus1+1)<<4);
        //   return (MMP_UINT16)pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.avcEntry.wWidth;
    case H263_VIDEO_TYPE:
        return (unsigned short)pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.h263Entry.wWidth;
    case JPEG_VIDEO_TYPE:
        return (unsigned short)pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.jpegEntry.wWidth;
    default:
        return 0;
    }
}



unsigned short sxaDmx3GPP_GetVideoHeight(MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    if (pGppData->pFirstVideoTrak == NULL)
    {
        return 0;
    }

    switch (pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.dwEntryType)
    {
    case MP4V_VIDEO_TYPE:
        // Kevin@2006.3.29  always return size within esdBox
        //return pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.mp4vEntry.wHeight;
        return (unsigned short)pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.mp4vEntry.esdBox.volInfo.dwFrameHeight;
        // ~Kevin@2006.3.29
        //	case AVC_VIDEO_TYPE:
        //	    // TO DO should parse vui to get the real display size
        //	    //return (MMP_UINT16)((pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.avcEntry.avcspec.avcInfo.avcSPS.mbHeightMinus1+1)<<4);
        //	    return (MMP_UINT16)pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.avcEntry.wHeight;
    case H263_VIDEO_TYPE:
        return (unsigned short)pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.h263Entry.wHeight;
    case JPEG_VIDEO_TYPE:
        return (unsigned short)pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.jpegEntry.wHeight;
    default:
        return 0;
    }
}



SXA_DMXVFMT_E sxaDmx3GPP_GetVideoCodecType(MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    if (pGppData->pFirstVideoTrak == NULL)
    {
        return SXA_DMXVFMT_NULL;
    }

    switch (pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.dwEntryType)
    {
    case H263_VIDEO_TYPE:
        return SXA_DMXVFMT_H263;
        break;
    case MP4V_VIDEO_TYPE:
        return SXA_DMXVFMT_MPEG4;
        break;
    case MJPA_VIDEO_TYPE:
    case JPEG_VIDEO_TYPE:
        return SXA_DMXVFMT_JPEG;
        break;
    default:
        return SXA_DMXVFMT_NULL;
        break;
    }
}



SXA_DMXAFMT_E sxaDmx3GPP_GetAudioCodecType(MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    if (pGppData->pFirstAudioTrak == NULL)
    {
        return SXA_DMXAFMT_NULL;
    }

    switch (pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.dwEntryType)
    {
    case SAMR_SOUND_TYPE:
        return SXA_DMXAFMT_AMR;
    case RAW_SOUND_TYPE:
        return SXA_DMXAFMT_RAW;
    case SOWT_SOUND_TYPE:
        return SXA_DMXAFMT_SOWT;
    case TWOS_SOUND_TYPE:
        return SXA_DMXAFMT_TWOS;
    case ULAW_SOUND_TYPE:
        return SXA_DMXAFMT_ULAW;
    case MP4A_SOUND_TYPE:
        // Kevin@2006.3.27  For MM360 Playback ONLY
#ifdef OPTION_MM360
        switch (pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.mp4aEntry.esdBox.audioObjectType)
        {
        case 1:
            return SXA_DMXAFMT_MP4;
        case 2:
            return SXA_DMXAFMT_MP4;
        default:
            return SXA_DMXAFMT_NULL;
        }
#else
        return SXA_DMXAFMT_MP4;
#endif
        // ~Kevin@2006.3.27
    default:
        return SXA_DMXAFMT_NULL;
    }
}



unsigned int sxaDmx3GPP_GetTotalTime(MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned int time;

    if (pGppData->moov.mvhd.dwDuration != 0 )
    {
        time = (pGppData->moov.mvhd.dwDuration / pGppData->moov.mvhd.dwTimeScale) * 1000;
    }
    else
    {
        if (pGppData->pFirstVideoTrak == NULL && pGppData->pFirstAudioTrak == NULL)
        {
            time = 0;
        }
        else if (pGppData->pFirstVideoTrak != NULL && pGppData->pFirstAudioTrak == NULL)
        {
            time = (pGppData->pFirstVideoTrak->tkhd.dwDuration / pGppData->moov.mvhd.dwTimeScale) * 1000;
        }
        else if (pGppData->pFirstVideoTrak == NULL && pGppData->pFirstAudioTrak != NULL)
        {
            time = (pGppData->pFirstAudioTrak->tkhd.dwDuration / pGppData->moov.mvhd.dwTimeScale) * 1000;
        }
        else
        {
            if (pGppData->pFirstAudioTrak->tkhd.dwDuration > pGppData->pFirstVideoTrak->tkhd.dwDuration)
            {
                time = (pGppData->pFirstAudioTrak->tkhd.dwDuration / pGppData->moov.mvhd.dwTimeScale) * 1000;
            }
            else
            {
                time = (pGppData->pFirstVideoTrak->tkhd.dwDuration / pGppData->moov.mvhd.dwTimeScale) * 1000;
            }
        }
    }

    return time;
}



unsigned int sxaDmx3GPP_GetVideoTimeScale(MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    if (pGppData->pFirstVideoTrak != NULL)
    {
        return pGppData->pFirstVideoTrak->mdia.mdhd.dwTimeScale;
    }
    else
    {
        return 0;
    }
}



unsigned int sxaDmx3GPP_GetAudioTimeScale(MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    if (pGppData->pFirstAudioTrak != NULL)
    {
        return pGppData->pFirstAudioTrak->mdia.mdhd.dwTimeScale;
    }
    else
    {
        return 0;
    }
}



unsigned int sxaDmx3GPP_GetAudioFreq(MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned int f;

    if (pGppData->pFirstAudioTrak == NULL)
    {
        return 0;
    }

    switch (pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.dwEntryType)
    {
    case SAMR_SOUND_TYPE:
        //return MMP_3GPP_AUDIO_CODEC_AMR;
        f = 8000;
        break;
    case RAW_SOUND_TYPE:
        // I don't know why??
        f = pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.rawEntry.dwSampleRate / 65536;
        break;
    case SOWT_SOUND_TYPE:
        // I don't know why??
        f = pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.sowtEntry.dwSampleRate / 65536;
        break;
    case TWOS_SOUND_TYPE:
        // I don't know why??
        f = pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.twosEntry.dwSampleRate / 65536;
        break;
    case ULAW_SOUND_TYPE:
        // I don't know why??
        f = pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.ulawEntry.dwSampleRate / 65536;
        break;
    case MP4A_SOUND_TYPE:
        switch (pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.mp4aEntry.esdBox.samplingFrequencyIndex)
        {
        case 0x0:
            f = 96000;
            break;
        case 0x1:
            f = 88200;
            break;
        case 0x2:
            f = 64000;
            break;
        case 0x3:
            f = 48000;
            break;
        case 0x4:
            f = 44100;
            break;
        case 0x5:
            f = 32000;
            break;
        case 0x6:
            f = 24000;
            break;
        case 0x7:
            f = 22050;
            break;
        case 0x8:
            f = 16000;
            break;
        case 0x9:
            f = 12000;
            break;
        case 0xa:
            f = 11025;
            break;
        case 0xb:
            f = 8000;
            break;
        case 0xc:
            f = 7350;
            break;
        default:
            f = 0;
            break;
        }
        break;
    default:
        f = 0;
    }

    return f;
}



unsigned int sxaDmx3GPP_GetAudioChannel(MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned int c = 0;

    if (pGppData->pFirstAudioTrak == NULL)
    {
        return 0;
    }

    switch (pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.dwEntryType)
    {
    case SAMR_SOUND_TYPE:
        c = 1;
        break;
    case MP4A_SOUND_TYPE:
        c = pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.mp4aEntry.esdBox.channelConfiguration;
        break;
    case RAW_SOUND_TYPE:
        c = pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.rawEntry.wChannelNum;
        break;
    case SOWT_SOUND_TYPE:
        c = pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.sowtEntry.wChannelNum;
        break;
    case TWOS_SOUND_TYPE:
        c = pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.twosEntry.wChannelNum;
        break;
    case ULAW_SOUND_TYPE:
        c = pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.ulawEntry.wChannelNum;
        break;
    default:
        c = 0;
    }

    return c;
}



unsigned int sxaDmx3GPP_GetAudioSampleSize(MMP_3GPP_DECODE_3GPPDATA* pGppData)
{
    unsigned int size = 0;

    if (pGppData->pFirstAudioTrak == NULL)
    {
        return 0;
    }

    switch (pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.dwEntryType)
    {
    case SAMR_SOUND_TYPE:
    case MP4A_SOUND_TYPE:
        size = 16;
        break;
    case RAW_SOUND_TYPE:
        size = pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.rawEntry.wSampleSize;
        break;
    case SOWT_SOUND_TYPE:
        size = pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.sowtEntry.wSampleSize;
        break;
    case TWOS_SOUND_TYPE:
        size = pGppData->pFirstAudioTrak->mdia.minf.stbl.stsd.twosEntry.wSampleSize;
        break;
    case ULAW_SOUND_TYPE:
        size = 8;  // [dvyu] 2007.12.18, uLaw's sample size is always 8.
        break;
    default:
        size = 0;
    }

    return size;
}



GPP_RESULT
sxaDmx3GPP_GetUserData(
    MMP_3GPP_DECODE_3GPPDATA* pGppData,
    SXA_DMXUSERDATA_T*        pUserData)
{
    pUserData->pchCnam = pGppData->moov.udta.pchCnam;
    pUserData->pchCart = pGppData->moov.udta.pchCart;
    pUserData->pchCalb = pGppData->moov.udta.pchCalb;

    return (GPP_SUCCESS);
}



GPP_RESULT
sxaDmx3GPP_Get1stVideoFrameInfo(
                                SXA_DMXOPENEXPARAM_T*     pParam,
                                SXA_DMX1STFRAMEINFO_T*    pFrameInfo)
{
    MMP_3GPP_DECODE_3GPPDATA* pGppData = NULL;
    GPP_RESULT          result = 0;
    unsigned long       dwBoxSize = 0;
    unsigned long       dwBoxType = 0;
    unsigned long       dwTRAKBoxSize = 0;
    signed long         dwFileERRCode = 0;
    int                 mdatFlag = 0;
    int                 moovFlag = 0;
    PAL_FILE           *pFile = NULL;
    unsigned int        i;

    MMP_3GPP_DECODE_STTS stts;
    MMP_3GPP_DECODE_STSC stsc;
    MMP_3GPP_DECODE_STSZ stsz;
    MMP_3GPP_DECODE_STCO stco;
    MMP_3GPP_DECODE_STSS stss;

    PalMemset(pFrameInfo, 0, sizeof(SXA_DMX1STFRAMEINFO_T));

    pGppData = (MMP_3GPP_DECODE_3GPPDATA *)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(MMP_3GPP_DECODE_3GPPDATA));
    if (pGppData == NULL)
    {
        result = GPP_ERROR_DECODE_MALLOC;
        goto end;
    }

    PalMemset(pGppData, 0, sizeof(MMP_3GPP_DECODE_3GPPDATA));

#if (WATCHDOG_ENABLE)
    lastClock = PalGetClock();
#endif

    gppOpenExParam.nOffset = pParam->nOffset;
    gppOpenExParam.nLength = pParam->nLength;

    /*--- open file ---*/
    pFile = PalTFileOpen(pParam->ptPathName, PAL_FILE_RB | ((pParam->nKey << 24) | (pParam->nMode << 16)), MMP_NULL);
    if (pFile == NULL)
    {
        result = GPP_ERROR_DECODE_FILE_OPEN;
        goto end;
    }

	pGppData->gppFile = pFile;
    gppFilePos = gppOpenExParam.nOffset;

    /*--- get file size ---*/
    pGppData->dwFileSize = gppOpenExParam.nLength;

    dwFileERRCode = PalTFileSeek(pFile, gppFilePos, PAL_SEEK_SET, MMP_NULL);
    if (dwFileERRCode < 0)
    {
        result = GPP_ERROR_DECODE_FILE_SEEK;
        goto end;
    }

#if (WATCHDOG_ENABLE)
    CheckResetWatchDog(__LINE__);
#endif

    while (gppFilePos < gppOpenExParam.nOffset + pGppData->dwFileSize)
    {
#if (WATCHDOG_ENABLE)
        CheckResetWatchDog(__LINE__);
#endif

        /*--- read box size from file ---*/
        dwFileERRCode = PalTFileRead(&dwBoxSize, 4, 1, pFile, MMP_NULL);
        if (dwFileERRCode != 1)
        {
            //printf("(0) GPP_ERROR_DECODE_FILE_READ (%d)\n", dwFileERRCode);
            result = GPP_ERROR_DECODE_FILE_READ;
            goto end;
        }

        gppFilePos += 4;
        BS_WAP(dwBoxSize);

        if (dwBoxSize < 8)
        {
            // The size of an atom should be 8 at least.
            break;
        }

        /*--- read box type from file ---*/
        dwFileERRCode = PalTFileRead(&dwBoxType, sizeof(unsigned long), 1, pFile, MMP_NULL);
        if (dwFileERRCode != 1)
        {
            result = GPP_ERROR_DECODE_FILE_READ;
            goto end;
        }

        gppFilePos += sizeof(unsigned long);
        BS_WAP(dwBoxType);

        switch(dwBoxType)
        {
            /*--- default, FTYP, VMHD, SMHD, DREF, CPRT, FREE, SKIP, IODS, NMHD, ELST, UDTA ---*/
        default : // unknown Box type
        case FTYP_BOX_TYPE : //file type and compatibility
        case VMHD_BOX_TYPE : //video media header, overall information
        case SMHD_BOX_TYPE : //sound media header, overall information
        case HMHD_BOX_TYPE : //hint media header, overall information
        case DREF_BOX_TYPE : //data reference box
        case CPRT_BOX_TYPE : //copyright
        case FREE_BOX_TYPE : //free space
        case SKIP_BOX_TYPE : //free space
        case IODS_BOX_TYPE : //Object description box
        case NMHD_BOX_TYPE : //mpeg-4 media header
        case ELST_BOX_TYPE : //an edit list
        case UDTA_BOX_TYPE : //user-data container
            dwFileERRCode = PalTFileSeek(pFile, (dwBoxSize - 8L),PAL_SEEK_CUR, MMP_NULL);
            if (dwFileERRCode < 0)
            {
                result = GPP_ERROR_DECODE_FILE_SEEK;
                goto end;
            }
            gppFilePos += (dwBoxSize - 8L);
            break;

            /*--- MINF, DINF, STBL ---*/
        case MINF_BOX_TYPE : //media information containere
        case DINF_BOX_TYPE : //data information box, container
        case STBL_BOX_TYPE : //sample table box
            break;

            /*--- EDTS, MDIA ---*/
        case EDTS_BOX_TYPE : //edit list container
        case MDIA_BOX_TYPE : //container for the media information in a track
            dwTRAKBoxSize = dwTRAKBoxSize - dwBoxSize;
            break;

            /*--- MOOV ---*/
        case MOOV_BOX_TYPE : //container for all the meta-data
            if (moovFlag)
            {
                result = GPP_ERROR_DECODE_FILE_PARSE;
                goto end;
            }
            else
            {
                moovFlag = 1;
            }
            break;

            /*--- TREF ---*/
        case TREF_BOX_TYPE : //track reference container
            dwFileERRCode = PalTFileSeek(pFile, (dwBoxSize - 8L),PAL_SEEK_CUR, MMP_NULL);
            if (dwFileERRCode < 0)
            {
                result = GPP_ERROR_DECODE_FILE_SEEK;
                goto end;
            }
            gppFilePos += (dwBoxSize - 8L);
            dwTRAKBoxSize = dwTRAKBoxSize - dwBoxSize;
            break;

            /*--- TRAK ---*/
        case TRAK_BOX_TYPE : //container for an individual track or stream
            if (pGppData->dwTrakCount + 1 > MAX_TRAK_COUNT)
            {
                result = GPP_ERROR_DECODE_MOOV_PARSE;
                goto end;
            }
            (pGppData->dwTrakCount)++;
            dwTRAKBoxSize = dwBoxSize - 8;
            break;

            /*--- MDAT ---*/
        case MDAT_BOX_TYPE :
            if (mdatFlag)
            {
                // result = GPP_ERROR_DECODE_FILE_PARSE;
                // goto end;
            }
            else
            {
                mdatFlag = 1;
            }
            dwFileERRCode = PalTFileSeek(pFile, (dwBoxSize - 8L),PAL_SEEK_CUR, MMP_NULL);
            if (dwFileERRCode < 0)
            {
                result = GPP_ERROR_DECODE_FILE_SEEK;
                goto end;
            }
            gppFilePos += (dwBoxSize - 8L);
            break;

            /*--- MVHD ---*/
        case MVHD_BOX_TYPE : //movie header, overall declarations
            {
                unsigned char* temp = PalHeapAlloc(PAL_HEAP_DEFAULT, dwBoxSize-8);
                if (temp == NULL)
                {
                    result = GPP_ERROR_DECODE_MALLOC;
                    goto end;
                }

                dwFileERRCode = PalTFileRead(temp, dwBoxSize-8, 1, pFile, MMP_NULL);
                if (dwFileERRCode < 0)
                {
                    PalHeapFree(PAL_HEAP_DEFAULT, temp);
                    result = GPP_ERROR_DECODE_FILE_READ;
                    goto end;
                }
                gppFilePos += (dwBoxSize-8);

                result = sxaDmx3GPP_GetMovieHeader(temp, dwBoxSize, pGppData);
                PalHeapFree(PAL_HEAP_DEFAULT, temp);
                if (result != GPP_SUCCESS)
                {
                    goto end;
                }
            }
            break;
            /*--- TKHD ---*/
        case TKHD_BOX_TYPE : //track header, overall information about the track
            {
                unsigned char* temp = PalHeapAlloc(PAL_HEAP_DEFAULT, dwBoxSize-8);
                if (temp == NULL)
                {
                    result = GPP_ERROR_DECODE_MALLOC;
                    goto end;
                }
                dwFileERRCode = PalTFileRead(temp, dwBoxSize-8, 1, pFile, MMP_NULL);
                if (dwFileERRCode < 0)
                {
                    PalHeapFree(PAL_HEAP_DEFAULT, temp);
                    result = GPP_ERROR_DECODE_FILE_READ;
                    goto end;
                }
                gppFilePos += (dwBoxSize-8);

                result = sxaDmx3GPP_GetTrackHeader(temp, dwBoxSize, pGppData);
                PalHeapFree(PAL_HEAP_DEFAULT, temp);
                if (result != GPP_SUCCESS)
                {
                    goto end;
                }
            }
            dwTRAKBoxSize = dwTRAKBoxSize - dwBoxSize;
            break;

            /*--- MDHD ---*/
        case MDHD_BOX_TYPE : //media header, overall information about the media
            {
                unsigned char* temp = PalHeapAlloc(PAL_HEAP_DEFAULT, dwBoxSize-8);
                if (temp == NULL)
                {
                    result = GPP_ERROR_DECODE_MALLOC;
                    goto end;
                }
                dwFileERRCode = PalTFileRead(temp, dwBoxSize-8, 1, pFile, MMP_NULL);
                if (dwFileERRCode < 0)
                {
                    PalHeapFree(PAL_HEAP_DEFAULT, temp);
                    result = GPP_ERROR_DECODE_FILE_READ;
                    goto end;
                }
                gppFilePos += (dwBoxSize-8);

                result = sxaDmx3GPP_GetMediaHeader(temp, dwBoxSize, pGppData);
                PalHeapFree(PAL_HEAP_DEFAULT, temp);
                if (result != GPP_SUCCESS)
                {
                    goto end;
                }
            }
            break;

            /*--- HDLR ---*/
        case HDLR_BOX_TYPE : //handler, declares the media (handler) type
            {
                unsigned char* temp = PalHeapAlloc(PAL_HEAP_DEFAULT, dwBoxSize-8);
                if (temp == NULL)
                {
                    result = GPP_ERROR_DECODE_MALLOC;
                    goto end;
                }
                dwFileERRCode = PalTFileRead(temp, dwBoxSize-8, 1, pFile, MMP_NULL);
                if (dwFileERRCode < 0)
                {
                    PalHeapFree(PAL_HEAP_DEFAULT, temp);
                    result = GPP_ERROR_DECODE_FILE_READ;
                    goto end;
                }
                gppFilePos += (dwBoxSize-8);

                result = sxaDmx3GPP_GetHandlerReference(temp, dwBoxSize, pGppData);
                PalHeapFree(PAL_HEAP_DEFAULT, temp);
                if (result != GPP_SUCCESS)
                {
                    goto end;
                }
            }
            break;

            /*--- STSD ---*/
        case STSD_BOX_TYPE : //sample description
            {
                unsigned char* temp = PalHeapAlloc(PAL_HEAP_DEFAULT, dwBoxSize-8);
                if (temp == NULL)
                {
                    result = GPP_ERROR_DECODE_MALLOC;
                    goto end;
                }
                dwFileERRCode = PalTFileRead(temp, dwBoxSize-8, 1, pFile, MMP_NULL);
                if (dwFileERRCode < 0)
                {
                    PalHeapFree(PAL_HEAP_DEFAULT, temp);
                    result = GPP_ERROR_DECODE_FILE_READ;
                    goto end;
                }
                gppFilePos += (dwBoxSize-8);

                result = sxaDmx3GPP_GetSampleDescription(temp, dwBoxSize, pGppData);
                PalHeapFree(PAL_HEAP_DEFAULT, temp);
                if (result != GPP_SUCCESS)
                {
                    goto end;
                }
            }
            break;

            /*--- STTS ---*/
        case STTS_BOX_TYPE : //(decoding) time-to-sample
            dwFileERRCode = PalTFileSeek(pFile, (dwBoxSize - 8L),PAL_SEEK_CUR, MMP_NULL);
            if (dwFileERRCode < 0)
            {
                result = GPP_ERROR_DECODE_FILE_SEEK;
                goto end;
            }
            gppFilePos += (dwBoxSize - 8L);
            break;

            /*--- STSC ---*/
        case STSC_BOX_TYPE : //sample-to-chunk
            dwFileERRCode = PalTFileSeek(pFile, (dwBoxSize - 8L),PAL_SEEK_CUR, MMP_NULL);
            if (dwFileERRCode < 0)
            {
                result = GPP_ERROR_DECODE_FILE_SEEK;
                goto end;
            }
            gppFilePos += (dwBoxSize - 8L);
            break;

            /*--- STSZ ---*/
        case STSZ_BOX_TYPE : //sample sizes (framing)
            if (result = sxaDmx3GPP_GetSampleSize(pFile, dwBoxSize, pGppData) != GPP_SUCCESS)
            {
                goto end;
            }
            break;

            /*--- STCO ---*/
        case STCO_BOX_TYPE : //chunk offset
            if (result = sxaDmx3GPP_GetChunkOffset(pFile, dwBoxSize, pGppData) != GPP_SUCCESS)
            {
                goto end;
            }
            break;

            /*--- STSS ---*/
        case STSS_BOX_TYPE : //sync sample table (random access points)
            dwFileERRCode = PalTFileSeek(pFile, (dwBoxSize - 8L),PAL_SEEK_CUR, MMP_NULL);
            if (dwFileERRCode < 0)
            {
                result = GPP_ERROR_DECODE_FILE_SEEK;
                goto end;
            }
            gppFilePos += (dwBoxSize - 8L);
            break;
        }
    }

#if (WATCHDOG_ENABLE)
    CheckResetWatchDog(__LINE__);
#endif

    result = sxaDmx3GPP_GetFirstVideoTrack(pGppData, &i);
    if (result == GPP_SUCCESS)
    {
        pGppData->pFirstVideoTrak = &(pGppData->moov.trak[i]);
    }
    else
    {
        goto end;
    }

    // get the video format
    switch (pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.dwEntryType)
    {
    case H263_VIDEO_TYPE:
        pFrameInfo->nVFmt = SXA_DMXVFMT_H263;
        break;
    case MP4V_VIDEO_TYPE:
        pFrameInfo->nVFmt = SXA_DMXVFMT_MPEG4;
        break;
    case MJPA_VIDEO_TYPE:
    case JPEG_VIDEO_TYPE:
        pFrameInfo->nVFmt = SXA_DMXVFMT_JPEG;
        break;
    default:
        pFrameInfo->nVFmt = SXA_DMXVFMT_NULL;
        break;
    }

    // get the video width
    switch (pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.dwEntryType)
    {
    case MP4V_VIDEO_TYPE:
        pFrameInfo->nVWidth = (unsigned int)pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.mp4vEntry.esdBox.volInfo.dwFrameWidth;
        break;
    case H263_VIDEO_TYPE:
        pFrameInfo->nVWidth = (unsigned int)pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.h263Entry.wWidth;
        break;
    case JPEG_VIDEO_TYPE:
        pFrameInfo->nVWidth = (unsigned int)pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.jpegEntry.wWidth;
        break;
    default:
        pFrameInfo->nVWidth = 0;
        break;
    }

    // get the video height
    switch (pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.dwEntryType)
    {
    case MP4V_VIDEO_TYPE:
        pFrameInfo->nVHeight = (unsigned int)pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.mp4vEntry.esdBox.volInfo.dwFrameHeight;
        break;
    case H263_VIDEO_TYPE:
        pFrameInfo->nVHeight = (unsigned int)pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.h263Entry.wHeight;
        break;
    case JPEG_VIDEO_TYPE:
        pFrameInfo->nVHeight = (unsigned int)pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.jpegEntry.wHeight;
        break;
    default:
        pFrameInfo->nVHeight = 0;
        break;
    }

    // get the offset
    stco = pGppData->pFirstVideoTrak->mdia.minf.stbl.stco;
    result = sxaDmx3GPP_GetValueFromTableStream(&stco.stcoEntry, 0, 0, (unsigned long*)&pFrameInfo->nOffset);
    if (result != GPP_SUCCESS)
    {
        goto end;
    }
    pFrameInfo->nOffset += 	gppOpenExParam.nOffset;

    // get the size
    stsz = pGppData->pFirstVideoTrak->mdia.minf.stbl.stsz;
    if (stsz.dwSampleSize == 0)
    {
        result = sxaDmx3GPP_GetValueFromTableStream(&stsz.stszEntry, 0, 0, (unsigned long *)&pFrameInfo->nSize);
        if (result != GPP_SUCCESS)
        {
            goto end;
        }
    }
    else
    {
        pFrameInfo->nSize = stsz.dwSampleSize;
    }

    // get VOL info
    if (pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.dwEntryType == MP4V_VIDEO_TYPE)
    {
        PalMemcpy(&(pFrameInfo->u.volInfo), &(pGppData->pFirstVideoTrak->mdia.minf.stbl.stsd.mp4vEntry.esdBox.volInfo), sizeof(SXA_DMXVOLINFO_T));
    }

end:
    for (i=0; i<pGppData->dwTrakCount; i++)
    {
        // mstd_free STTS_D
        stts = pGppData->moov.trak[i].mdia.minf.stbl.stts;
        sxaDmx3GPP_TableStreamDestory(&stts.sttsEntry);

        // mstd_free STSC_D
        stsc = pGppData->moov.trak[i].mdia.minf.stbl.stsc;
        sxaDmx3GPP_TableStreamDestory(&stsc.stscEntry);

        // mstd_free STSZ_D
        stsz = pGppData->moov.trak[i].mdia.minf.stbl.stsz;
        sxaDmx3GPP_TableStreamDestory(&stsz.stszEntry);

        // mstd_free STCO_D
        stco = pGppData->moov.trak[i].mdia.minf.stbl.stco;
        sxaDmx3GPP_TableStreamDestory(&stco.stcoEntry);

        // mstd_free STSS_D
        stss = pGppData->moov.trak[i].mdia.minf.stbl.stss;
        if (stss.pSampleNumber)
        {
            PalHeapFree(PAL_HEAP_DEFAULT, stss.pSampleNumber);
        }
    }

    if (pFile != NULL)
    {
        PalTFileClose(pFile, MMP_NULL);
    }

    if (pGppData)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, (void *)pGppData);
        pGppData = NULL;
    }

    gppOpenExParam.nOffset = 0;
    gppOpenExParam.nLength = 0;

    return result;
}


