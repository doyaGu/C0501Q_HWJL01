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
 * mp3dec.c - platform-independent top level MP3 decoder API
 **************************************************************************************/
#include "mp3_config.h"

#if defined(__OR32__)
//#include "sys.h"
#endif

#if defined(EQUALIZER)
#include "equalizer.h"
#endif  // defined(EQUALIZER)

#if defined(FREQINFO)
#include "freqinfo.h"
#endif  // defined(FREQINFO)

#include "io.h"

#include "layer12.h"

/**************************************************************************************
 * Function:    MP3FindFreeSync
 *
 * Description: figure out number of bytes between adjacent sync words in "free" mode
 *
 * Inputs:      buffer to search for next sync word
 *              the 4-byte frame header starting at the current sync word
 *              max number of bytes to search in buffer
 *
 * Outputs:     none
 *
 * Return:      offset to next sync word, minus any pad byte (i.e. nSlots)
 *              -1 if sync not found after searching nBytes
 *
 * Notes:       this checks that the first 22 bits of the next frame header are the
 *                same as the current frame header, but it's still not foolproof
 *                (could accidentally find a sequence in the bitstream which
 *                 appears to match but is not actually the next frame header)
 *              this could be made more error-resilient by checking several frames
 *                in a row and verifying that nSlots is the same in each case
 *              since free mode requires CBR (see spec) we generally only call
 *                this function once (first frame) then store the result (nSlots)
 *                and just use it from then on
 **************************************************************************************/
static __inline int MP3FindFreeSync(unsigned char *buf, unsigned char firstFH[4], int nBytes)
{
    int i = 0;
    unsigned char *bufPtr;

    // loop until we either:
    //  - run out of nBytes (FindMP3SyncWord() returns -1)
    //  - find the next valid frame header (sync word, version, layer, CRC flag, bitrate, and sample rate
    //      in next header must match current header)

    for ( ; i < nBytes - 2; i++) {
        if ( (buf[i+0] & SYNCWORDH) == SYNCWORDH && (buf[i+1] & SYNCWORDL) == SYNCWORDL ){
            bufPtr = buf + i;
            if ( (bufPtr[0] == firstFH[0]) && (bufPtr[1] == firstFH[1]) && ((bufPtr[2] & 0xfc) == (firstFH[2] & 0xfc)) ) {
                // want to return number of bytes per frame, NOT counting the padding byte, so subtract one if padFlag == 1
                if ((firstFH[2] >> 1) & 0x01){
                    bufPtr--;
                }
                return bufPtr - buf;
            }
        }
    }

    return -1;
}

/**************************************************************************************
 * Function:    MP3GetLastFrameInfo
 *
 * Description: get info about last MP3 frame decoded (number of sampled decoded,
 *                sample rate, bitrate, etc.)
 *
 * Inputs:      valid MP3 decoder instance pointer (HMP3Decoder)
 *              pointer to MP3FrameInfo struct
 *
 * Outputs:     filled-in MP3FrameInfo struct
 *
 * Return:      none
 *
 * Notes:       call this right after calling MP3Decode
 **************************************************************************************/
/*
void MP3GetLastFrameInfo()
{
    if (mp3DecInfo.layer != 3) {
        mp3FrameInfo.bitrate = 0;
        mp3FrameInfo.nChans = 0;
        mp3FrameInfo.samprate = 0;
        mp3FrameInfo.bitsPerSample = 0;
        mp3FrameInfo.outputSamps = 0;
        mp3FrameInfo.layer = 0;
        mp3FrameInfo.version = 0;
    } else {
        mp3FrameInfo.bitrate = mp3DecInfo.bitrate;
        mp3FrameInfo.nChans = mp3DecInfo.nChans;
        mp3FrameInfo.samprate = mp3DecInfo.samprate;
        mp3FrameInfo.bitsPerSample = 16;
#if defined(OUTPUT_ALWAYS_2CH)
        mp3FrameInfo.outputSamps = 2 * (int)samplesPerFrameTab[mp3DecInfo.version][mp3DecInfo.layer - 1];
#else
        mp3FrameInfo.outputSamps = mp3DecInfo.nChans * (int)samplesPerFrameTab[mp3DecInfo.version][mp3DecInfo.layer - 1];
#endif
        mp3FrameInfo.layer = mp3DecInfo.layer;
        mp3FrameInfo.version = mp3DecInfo.version;
    }
}
*/

/**************************************************************************************
 * Function:    MP3GetNextFrameInfo
 *
 * Description: parse MP3 frame header
 *
 * Inputs:      valid MP3 decoder instance pointer (HMP3Decoder)
 *              pointer to MP3FrameInfo struct
 *              pointer to buffer containing valid MP3 frame header (located using
 *                MP3FindSyncWord(), above)
 *
 * Outputs:     filled-in MP3FrameInfo struct
 *
 * Return:      error code, defined in mp3dec.h (0 means no error, < 0 means error)
 **************************************************************************************/
/*
int MP3GetNextFrameInfo(unsigned char *buf)
{
    if (UnpackFrameHeader(buf) == -1 || mp3DecInfo.layer != 3)
        return ERR_MP3_INVALID_FRAMEHEADER;

    MP3GetLastFrameInfo();

    return ERR_MP3_NONE;
}
*/

/**************************************************************************************
 * Function:    MP3ClearBadFrame
 *
 * Description: zero out pcm buffer if error decoding MP3 frame
 *
 * Inputs:      mp3DecInfo struct with correct frame size parameters filled in
 *              pointer pcm output buffer
 *
 * Outputs:     zeroed out pcm buffer
 *
 * Return:      none
 **************************************************************************************/
static __inline void MP3ClearBadFrame(int *outbuf)   //static void MP3ClearBadFrame(short *outbuf)
{
    int j;
    for (j = 0; j < mp3DecInfo.nChans * mp3DecInfo.nFrameSamps; j++){
        outbuf[j] = 0;
    }
}

/**************************************************************************************
 * Function:    MP3Decode
 *
 * Description: decode one frame of MP3 data
 *
 * Inputs:      valid MP3 decoder instance pointer (HMP3Decoder)
 *              double pointer to buffer of MP3 data (containing headers + mainData)
 *              number of valid bytes remaining in inbuf
 *              pointer to outbuf, big enough to hold one frame of decoded PCM samples
 *
 * Outputs:     PCM data in outbuf, interleaved LRLRLR... if stereo
 *                number of output samples = nGrans * nGranSamps * nChans
 *              updated inbuf pointer
 *
 * Return:      error code, defined in mp3dec.h (0 means no error, < 0 means error)
 **************************************************************************************/

/* PHASE4: from PHASE3, corresponding to PHASE12 of imdct_new.c
 * seperate IMDCT(gr,ch) into Antialias(gr,ch,..) and HybridTransform(gr,ch,..)
 */
#if defined(USER_DEFINE_DIV)
#define NBITS32     32

// this function is used to compute unsigned integer / only
static void udiv32bit(unsigned int dividend, unsigned int divisor, unsigned int *quotient)
{
    #if defined(WIN32) || defined(__CYGWIN__)
    unsigned int dividendref = dividend,
                divisorref = divisor;
    #endif
    register int counter;
    register unsigned int numzeros, q;
    numzeros = 0;
    while (!(divisor & (1<<(NBITS32-1)) )) {    // assume dividend, divisor is 12bit unsigned integer
        numzeros++;
        divisor <<= 1;
    }
    q=0;
    counter = numzeros;
    while(counter >= 0){
        q = q << 1;
        if(dividend >= divisor){
            dividend-= divisor;
            q++;
        }
        divisor >>= 1;
        counter--;
    }
    *quotient  = q;
    #if defined(WIN32) || defined(__CYGWIN__)
    if(dividendref / divisorref != *quotient){
        printf("user defined div error: %d / %d = %d\n",
            dividendref, divisorref, *quotient);
    }
    #endif
}
#endif

//*************************************************************************************
// Function:    UnpackFrameHeader
//
// Description: parse the fields of the MP3 frame header
//
// Inputs:      buffer pointing to a complete MP3 frame header (4 bytes, plus 2 if CRC)
//
// Outputs:     filled frame header info in the MP3DecInfo structure
//              updated platform-specific FrameHeader struct
//
// Return:      length (in bytes) of frame header (for caller to calculate offset to
//                first byte following frame header)
//              -1 if null frameHeader or invalid header
//
// TODO:        check for valid modes, depending on capabilities of decoder
//              test CRC on actual stream (verify no endian problems)
//
// PS:          UnpackFrameHeader() is only called at MP3Decode() & MP3GetNextFrameInfo()
//              of mp3dec.c. However, MP3GetNextFrameInfo() is never called in the decoder
//              so we make UnpackFrameHeader() inline function
//*************************************************************************************
int UnpackFrameHeader(unsigned char *buf)
{
    int verIdx;
    FrameHeader *fh;
    register unsigned char byte;
    register int srIdx, layer, brIdx, ver, sMode, crc;

    fh = &(mp3DecInfo.FrameHeaderPS);

    // read header fields - use bitmasks instead of GetBits() for speed, since format never varies
    byte = buf[1];
    verIdx = (byte >> 3) & 0x03;
    fh->ver = ver = (MPEGVersion)( verIdx == 0 ? MPEG25 : ((verIdx & 0x01) ? MPEG1 : MPEG2) );
    fh->layer = layer = 4 - ((byte >> 1) & 0x03);     // easy mapping of index to layer number, 4 = error
    fh->crc = crc = 1 - ((byte >> 0) & 0x01);

    byte = buf[2];
    fh->brIdx = brIdx = (byte >> 4) & 0x0f;
    fh->srIdx = srIdx = (byte >> 2) & 0x03;
    fh->paddingBit = (byte >> 1) & 0x01;
    fh->privateBit = (byte >> 0) & 0x01;

    byte = buf[3];
    fh->sMode = sMode = (StereoMode)((byte >> 6) & 0x03);      // maps to correct enum (see definition)
    fh->modeExt =  (byte >> 4) & 0x03;
    fh->copyFlag = (byte >> 3) & 0x01;
    fh->origFlag = (byte >> 2) & 0x01;
    fh->emphasis = (byte >> 0) & 0x03;

    // check parameters to avoid indexing tables with bad values

    if (srIdx == 3 || layer == 4 || brIdx == 15)
        return -1;

    fh->sfBand = &sfBandTable[ver][srIdx];  // for readability (we reference sfBandTable many times in decoder)
    if (sMode != Joint)     // just to be safe (dequant, stproc check fh->modeExt)
        fh->modeExt = 0;

    if (ver  != mp3DecInfo.version || layer !=mp3DecInfo.layer)
    {
        mp3DecInfo.nChans      = (sMode == Mono ? 1 : 2);
        mp3DecInfo.samprate    = samplerateTab[ver][srIdx];
        mp3DecInfo.nGrans      = (layer == 1 ? 1 : (ver == MPEG1 ? NGRANS_MPEG1 : NGRANS_MPEG2));
        mp3DecInfo.nGranSamps  = ((int)samplesPerFrameTab[ver][layer - 1]) >> (mp3DecInfo.nGrans - 1);
        mp3DecInfo.nFrameSamps = mp3DecInfo.nGrans * mp3DecInfo.nGranSamps;
        mp3DecInfo.layer       = layer;
        mp3DecInfo.version     = ver;
        mp3DecInfo.nSubbands   = (layer == 1 ? 12 : 18);
        if (brIdx) 
        {
            mp3DecInfo.bitrate = ((int)bitrateTab[ver][layer - 1][brIdx]) * 1000;
        }

        //return ERR_MP3_INVALID_ENCODEFILE;
    }
    else
    {
        mp3DecInfo.nChans      = (sMode == Mono ? 1 : 2);
        mp3DecInfo.samprate    = samplerateTab[ver][srIdx];
        mp3DecInfo.nGrans      = (layer == 1 ? 1 : (ver == MPEG1 ? NGRANS_MPEG1 : NGRANS_MPEG2));
        mp3DecInfo.nGranSamps  = ((int)samplesPerFrameTab[ver][layer - 1]) >> (mp3DecInfo.nGrans - 1);
        mp3DecInfo.nFrameSamps = mp3DecInfo.nGrans * mp3DecInfo.nGranSamps;
        mp3DecInfo.layer       = layer;
        mp3DecInfo.version     = ver;
        mp3DecInfo.nSubbands   = (layer == 1 ? 12 : 18);
        mp3DecInfo.bitrate = ((int)bitrateTab[ver][layer - 1][brIdx]) * 1000;       
    }

    PRINTF("mp3DecInfo.nChans      %d\n", mp3DecInfo.nChans);
    PRINTF("mp3DecInfo.samprate    %d\n", mp3DecInfo.samprate);
    PRINTF("mp3DecInfo.nGrans      %d\n", mp3DecInfo.nGrans);
    PRINTF("mp3DecInfo.nGranSamps  %d\n", mp3DecInfo.nGranSamps);
    PRINTF("mp3DecInfo.nFrameSamps %d\n", mp3DecInfo.nFrameSamps);
    PRINTF("mp3DecInfo.nSubbands   %d\n", mp3DecInfo.nSubbands);
    PRINTF("mp3DecInfo.layer       %d\n", mp3DecInfo.layer);
    PRINTF("mp3DecInfo.version     %d\n", mp3DecInfo.version);

    // get bitrate and nSlots from table, unless brIdx == 0 (free mode) in which case caller must figure it out himself
    // question - do we want to overwrite mp3DecInfo->bitrate with 0 each time if it's free mode, and
    //  copy the pre-calculated actual free bitrate into it in mp3dec.c (according to the spec,
    //  this shouldn't be necessary, since it should be either all frames free or none free)

    if (brIdx) 
    {
        mp3DecInfo.bitrate = ((int)bitrateTab[ver][layer - 1][brIdx]) * 1000;

        if (layer == 3)
        {
            // nSlots = total frame bytes (from table) - sideInfo bytes - header - CRC (if present) + pad (if present)
            mp3DecInfo.nSlots = (int)slotTab[ver][srIdx][brIdx] - // yiwei, slot .. byte/frame
                (int)sideBytesTab[ver][(sMode == Mono ? 0 : 1)] -
                4 - (crc ? 2 : 0) + (fh->paddingBit ? 1 : 0);
        }
        else if (layer == 2) 
        {
            mp3DecInfo.nSlots =
            #if defined(DUMP_PCM_DATA) || defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
                (144 * mp3DecInfo.bitrate / mp3DecInfo.samprate) -
                (int)sideBytesTab[ver][(sMode == Mono ? 0 : 1)] -
                4 - (crc ? 2 : 0) + fh->paddingBit;
            #else
                (144 * mp3DecInfo.bitrate / mp3DecInfo.samprate) + fh->paddingBit;
            #endif
            mp3DecInfo.nGrans = 2;
        }
        else if (layer == 1) 
        {
            mp3DecInfo.nSlots =
            #if defined(DUMP_PCM_DATA) || defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
                ((12 * mp3DecInfo.bitrate / mp3DecInfo.samprate) + fh->paddingBit)*4 - 
                 4 - (crc ? 2 : 0) - (int)sideBytesTab[ver][(sMode == Mono ? 0 : 1)];
            #else
                ((12 * mp3DecInfo.bitrate / mp3DecInfo.samprate) + fh->paddingBit)*4;
            #endif
            mp3DecInfo.nGrans = 1;
        }

    }

    if (mp3DecInfo.freeBitrateFlag) 
    {
         mp3DecInfo.freeBitrateFlag = 0;
    }

    // load crc word, if enabled, and return length of frame header (in bytes)
    if (crc) {
        fh->CRCWord = ((int)buf[4] << 8 | (int)buf[5] << 0);
        return 6;
    } else {
        fh->CRCWord = 0;
        return 4;
    }
}

//*************************************************************************************
// Function:    UnpackSideInfo
//
// Description: parse the fields of the MP3 side info header
//
// Inputs:      MP3DecInfo structure filled by UnpackFrameHeader()
//              buffer pointing to the MP3 side info data
//
// Outputs:     updated mainDataBegin in MP3DecInfo struct
//              updated private (platform-specific) SideInfo struct
//
// Return:      length (in bytes) of side info data
//              -1 if null input pointers
// PS.          only used at once so inline (at MP3Decode(...) of mp3dec.c)
//*************************************************************************************
static __inline int UnpackSideInfo(unsigned char *buf)
{
    int gr, ch, bd, nBytes;
    BitStreamInfo bitStreamInfo, *bsi;
    FrameHeader *fh;
    SideInfo *si;
    SideInfoSub *sis;

    fh = &(mp3DecInfo.FrameHeaderPS);
    si = &(mp3DecInfo.SideInfoPS);

    bsi = &bitStreamInfo;
    if (fh->ver == MPEG1) {
        // MPEG 1
        nBytes = (fh->sMode == Mono ? SIBYTES_MPEG1_MONO : SIBYTES_MPEG1_STEREO);
        SetBitstreamPointer(bsi, nBytes, buf);
        si->mainDataBegin = GetBits(bsi, 9);
        si->privateBits =   GetBits(bsi, (fh->sMode == Mono ? 5 : 3));

        for (ch = 0; ch < mp3DecInfo.nChans; ch++)
            for (bd = 0; bd < MAX_SCFBD; bd++)
                si->scfsi[ch][bd] = GetBits(bsi, 1);
    } else {
        // MPEG 2, MPEG 2.5
        nBytes = (fh->sMode == Mono ? SIBYTES_MPEG2_MONO : SIBYTES_MPEG2_STEREO);
        SetBitstreamPointer(bsi, nBytes, buf);
        si->mainDataBegin = GetBits(bsi, 8);
        si->privateBits =   GetBits(bsi, (fh->sMode == Mono ? 1 : 2));
    }

    for(gr =0; gr < mp3DecInfo.nGrans; gr++) {
        for (ch = 0; ch < mp3DecInfo.nChans; ch++) {
            sis = &si->sis[gr][ch];                     // side info subblock for this granule, channel

            sis->part23Length =    GetBits(bsi, 12);
            sis->nBigvals =        GetBits(bsi, 9);
            sis->globalGain =      GetBits(bsi, 8);
            sis->sfCompress =      GetBits(bsi, (fh->ver == MPEG1 ? 4 : 9));
            sis->winSwitchFlag =   GetBits(bsi, 1);

            if(sis->winSwitchFlag) {
                // this is a start, stop, short, or mixed block
                sis->blockType =       GetBits(bsi, 2);     // 0 = normal, 1 = start, 2 = short, 3 = stop
                sis->mixedBlock =      GetBits(bsi, 1);     // 0 = not mixed, 1 = mixed
                sis->tableSelect[0] =  GetBits(bsi, 5);
                sis->tableSelect[1] =  GetBits(bsi, 5);
                sis->tableSelect[2] =  0;                   // unused
                sis->subBlockGain[0] = GetBits(bsi, 3);
                sis->subBlockGain[1] = GetBits(bsi, 3);
                sis->subBlockGain[2] = GetBits(bsi, 3);

                // TODO - check logic
                if (sis->blockType == 0) {
                    // this should not be allowed, according to spec
                    sis->nBigvals = 0;
                    sis->part23Length = 0;
                    sis->sfCompress = 0;
                } else if (sis->blockType == 2 && sis->mixedBlock == 0) {
                    // short block, not mixed
                    sis->region0Count = 8;
                } else {
                    // start, stop, or short-mixed
                    sis->region0Count = 7;
                }
                sis->region1Count = 20 - sis->region0Count;
            } else {
                // this is a normal block
                sis->blockType = 0;
                sis->mixedBlock = 0;
                sis->tableSelect[0] =  GetBits(bsi, 5);
                sis->tableSelect[1] =  GetBits(bsi, 5);
                sis->tableSelect[2] =  GetBits(bsi, 5);
                sis->region0Count =    GetBits(bsi, 4);
                sis->region1Count =    GetBits(bsi, 3);
            }
            sis->preFlag =           (fh->ver == MPEG1 ? GetBits(bsi, 1) : 0);
            sis->sfactScale =        GetBits(bsi, 1);
            sis->count1TableSelect = GetBits(bsi, 1);
        }
    }
    mp3DecInfo.mainDataBegin = si->mainDataBegin;   // needed by main decode loop

    ASSERT(nBytes == CalcBitsUsed(bsi, buf, 0) >> 3);
    if (nBytes != CalcBitsUsed(bsi, buf, 0) >> 3) {
        return ERR_MP3_INVALID_SIDEINFO;
    }

    return nBytes;
}

int MP3Decode(STREAMBUF *inbuf, SAMPBUF *outbuf, param_eq_struct *p_param)
{
    int offset, bitOffset, mainBits, gr, ch, fhBytes, siBytes, freeFrameBytes;
    int prevBitOffset, sfBlockBits, huffBlockBits;
    unsigned char *buf, *mainPtr;

    #if defined(DUMP_PCM_DATA) || defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
    buf = inbuf->buf;
    #else
    buf = inbuf->buf + inbuf->rdptr;
    #endif

    # if defined(DUMP_FRAME_CHECKSUM)
    SetChksumRegionStart(inbuf->buf, inbuf->rdptr);
    # endif

    // unpack frame header
    fhBytes = UnpackFrameHeader(buf);
    if (fhBytes < 0){
        inbuf->rdptr += 1;
        # if defined(DUMP_FRAME_CHECKSUM)
        SetChksumRegionEnd(inbuf->buf, inbuf->rdptr);
        # endif
        return ERR_MP3_INVALID_FRAMEHEADER;
    }
    buf += fhBytes;

    if (mp3DecInfo.FrameHeaderPS.layer == 3)
    {
        // unpack side info
        siBytes = UnpackSideInfo(buf);
        buf += siBytes;
        inbuf->rdptr += (fhBytes + siBytes);

        // if free mode, need to calculate bitrate and nSlots manually, based on frame size
        if (mp3DecInfo.bitrate == 0 || mp3DecInfo.freeBitrateFlag) {
            if (!mp3DecInfo.freeBitrateFlag) {
                // first time through, need to scan for next sync word and figure out frame size
                #if defined(DUMP_PCM_DATA) || defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
                register int N = inbuf->wtptr - inbuf->rdptr - CheckPadBit();
                #else
                register int N = MP3FindFreeSync(buf, buf - fhBytes - siBytes, MAINBUF_SIZE);
                #endif

                if (N < 0) {
                    //MP3ClearBadFrame(outbuf->buf);
                    return ERR_MP3_FREE_BITRATE_SYNC;
                }
                mp3DecInfo.freeBitrateSlots = N;
                mp3DecInfo.freeBitrateFlag = 1;
                freeFrameBytes = mp3DecInfo.freeBitrateSlots + fhBytes + siBytes;
                mp3DecInfo.bitrate = (freeFrameBytes * mp3DecInfo.samprate * 8) / mp3DecInfo.nFrameSamps;
            }
            mp3DecInfo.nSlots = mp3DecInfo.freeBitrateSlots + CheckPadBit();    // add pad byte, if required
        }

        // out of data - assume last or truncated frame
        if (mp3DecInfo.nSlots > (inbuf->wtptr - inbuf->rdptr)){
            # if defined(DUMP_FRAME_CHECKSUM)
            SetChksumRegionEnd(inbuf->buf, inbuf->rdptr);
            # endif
            return ERR_MP3_INDATA_UNDERFLOW;
        }

        // fill main data buffer with enough new data for this frame
        if (mp3DecInfo.mainDataBytes >= mp3DecInfo.mainDataBegin) {
            // adequate "old" main data available (i.e. bit reservoir)
            memmove(mp3DecInfo.mainBuf, mp3DecInfo.mainBuf + mp3DecInfo.mainDataBytes - mp3DecInfo.mainDataBegin, mp3DecInfo.mainDataBegin);
            memcpy(mp3DecInfo.mainBuf + mp3DecInfo.mainDataBegin, buf, mp3DecInfo.nSlots);
            mp3DecInfo.mainDataBytes = mp3DecInfo.mainDataBegin + mp3DecInfo.nSlots;
            mainPtr = mp3DecInfo.mainBuf;
            inbuf->rdptr += mp3DecInfo.nSlots;
            # if defined(DUMP_FRAME_CHECKSUM)
            SetChksumRegionEnd(inbuf->buf, inbuf->rdptr);
            # endif
        } else {
            // not enough data in bit reservoir from previous frames (perhaps starting in middle of file)
            memcpy(mp3DecInfo.mainBuf + mp3DecInfo.mainDataBytes, buf, mp3DecInfo.nSlots);
            mp3DecInfo.mainDataBytes += mp3DecInfo.nSlots;
            //MP3ClearBadFrame(outbuf->buf);
            inbuf->rdptr += mp3DecInfo.nSlots;
            # if defined(DUMP_FRAME_CHECKSUM)
            SetChksumRegionEnd(inbuf->buf, inbuf->rdptr);
            # endif
            return ERR_MP3_MAINDATA_UNDERFLOW;
        }

        bitOffset = 0;
        mainBits = mp3DecInfo.mainDataBytes * 8;

        // decode one complete frame
        for (gr = 0; gr < mp3DecInfo.nGrans; gr++) {
            for (ch = 0; ch < mp3DecInfo.nChans; ch++) {
                // unpack scale factors and compute size of scale factor block
                prevBitOffset = bitOffset;
                offset = UnpackScaleFactors(mainPtr, &bitOffset, mainBits, gr, ch);

                sfBlockBits = 8*offset - prevBitOffset + bitOffset;
                huffBlockBits = mp3DecInfo.part23Length[gr][ch] - sfBlockBits;
                mainPtr += offset;
                mainBits -= sfBlockBits;

                if (offset < 0 || mainBits < huffBlockBits) {
                    //MP3ClearBadFrame(outbuf->buf);
                    return ERR_MP3_INVALID_SCALEFACT;
                }

                // decode Huffman code words
                prevBitOffset = bitOffset;
                offset = DecodeHuffman(mainPtr, &bitOffset, huffBlockBits, gr, ch);
                if (offset < 0) {
                    //MP3ClearBadFrame(outbuf->buf);
                    return ERR_MP3_INVALID_HUFFCODES;
                }

                mainPtr += offset;
                mainBits -= (8*offset - prevBitOffset + bitOffset);
            }
        }

        DequantizeGr();
        ProcessStereoGr();
        AntiAliasGrCh();

    #if defined(EQUALIZER)
        if(p_param->enable){
            equalization_filter(p_param);
        }
    #endif

        HybridTransformGrCh();
    }
    else if(mp3DecInfo.FrameHeaderPS.layer == 2) {
        int errcode, count;

    #if defined(DUMP_PCM_DATA) || defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
        if((inbuf->wtptr - inbuf->rdptr - fhBytes) < mp3DecInfo.nSlots) {
            return ERR_MP3_MAINDATA_UNDERFLOW;
        }

        count = inbuf->wtptr;
    #else
        if((inbuf->wtptr - inbuf->rdptr) < mp3DecInfo.nSlots) {
            return ERR_MP3_MAINDATA_UNDERFLOW;
        }

        count = mp3DecInfo.nSlots;
    #endif

        if(mad_layer_II(buf, &errcode) > (count - fhBytes)) {
            errcode =  ERR_MP3_INVALID_HUFFCODES;
        }

        inbuf->rdptr += count;

        if (errcode != ERR_MP3_NONE) {
            return errcode;
        }
    } else if(mp3DecInfo.FrameHeaderPS.layer == 1) {
        int errcode, count;

    #if defined(DUMP_PCM_DATA) || defined(FORCE_FINDSYNCWORD_ON_FILLBUFFER)
        if((inbuf->wtptr - inbuf->rdptr - fhBytes) < mp3DecInfo.nSlots) {
            return ERR_MP3_MAINDATA_UNDERFLOW;
        }

        count = inbuf->wtptr;
    #else
        if((inbuf->wtptr - inbuf->rdptr) < mp3DecInfo.nSlots) {
            return ERR_MP3_MAINDATA_UNDERFLOW;
        }

        count = mp3DecInfo.nSlots;
    #endif

        if(mad_layer_I(buf, &errcode) > (count - fhBytes)) {
            errcode =  ERR_MP3_INVALID_HUFFCODES;
        }

        inbuf->rdptr += count;

        if (errcode != ERR_MP3_NONE) {
            return errcode;
        }
    }

    #if defined(FREQINFO)
    updateFreqInfo();
    #endif

    SubbandTransformGr(outbuf->buf);

    outbuf->nch = mp3DecInfo.nChans;
    outbuf->nsamples = mp3DecInfo.nFrameSamps;
    return ERR_MP3_NONE;
}

