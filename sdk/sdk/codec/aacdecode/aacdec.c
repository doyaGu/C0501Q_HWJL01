/* ***** BEGIN LICENSE BLOCK *****
 * Source last modified: $Id: aacdec.c,v 1.1 2005/02/26 01:47:31 jrecker Exp $
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
 * Jon Recker (jrecker@real.com), Ken Cooke (kenc@real.com)
 * February 2005
 *
 * aacdec.c - platform-independent top level decoder API
 **************************************************************************************/
#include "coder.h"
#include "reverb.h"
#include "drc.h"
#include "aaccommon.h"
#include "assembly.h"
#include "config.h"
#include "aac_latm.h"
#include "sbr.h"

#if defined(ENABLE_CODECS_PLUGIN)
#include "plugin.h"
#endif
//DRC-dynammic range control
#if defined(REVERBERATION) || defined(DRCTL)
int decbuf[AAC_MAX_NCHANS*DECBUFSIZE];  // temp for drc buffer
#endif // defined(REVERBERATION) || defined(DRCTL)
#if defined (REVERBERATION)
extern REVERBINFO revbinfo;
#endif

#ifdef LATM_TO_ADTS
#include "win32.h"
#endif

/**************************************************************************************
 * Function:    AACInitDecoder
 *
 * Description: allocate memory for platform-specific data
 *              clear all the user-accessible fields
 *              initialize SBR decoder if enabled
 *
 * Inputs:      none
 *
 * Outputs:     none
 *
 * Return:      handle to AAC decoder instance, 0 if malloc fails
 **************************************************************************************/
HAACDecoder AACInitDecoder(void)
{
    AACDecInfo *aacDecInfo;

    aacDecInfo = AllocateBuffers();
    if (!aacDecInfo)
    {
        return 0;
    }

#if defined(AAC_ENABLE_SBR)
    if (InitSBR(aacDecInfo)) 
    {
        return 0;
    }
#endif // AAC_ENABLE_SBR

    return (HAACDecoder)aacDecInfo;
}

/**************************************************************************************
 * Function:    AACFindSyncWord
 *
 * Description: locate the next byte-alinged sync word in the raw AAC stream
 *
 * Inputs:      buffer to search for sync word
 *              max number of bytes to search in buffer
 *
 * Outputs:     none
 *
 * Return:      offset to first sync word (bytes from start of buf)
 *              -1 if sync not found after searching nBytes
 **************************************************************************************/
int AACFindSyncWord(unsigned char *buf, int nBytes)
{
    int i;
    /* find byte-aligned syncword (12 bits = 0xFFF) */
#if 0 // Original Code
    for (i = 0; i < nBytes - 1; i++) {
        if ( (buf[i+0] & SYNCWORDH) == SYNCWORDH && (buf[i+1] & SYNCWORDL) == SYNCWORDL )
#else // Accurate code to find sync word
    for (i = 0; i < nBytes - 3; i++)
    {
        if (!(((buf[i+0] & SYNCWORDH)== SYNCWORDH)&&((buf[i+1] & SYNCWORDL) == SYNCWORDL)))
        {
            continue;
        }
        // Layer
        #if defined(ADTS_EXTENSION_FOR_PTS)
            // Layer 0 -> AAC-LC, Layer 1 -> AAL-LC with PTS extension
            if( ((buf[i+1] & 0x06) >> 1) != 0x0 && ((buf[i+1] & 0x06) >> 1) != 0x1)
                continue;
        #else
            if( ((buf[i+1] & 0x06) >> 1) != 0x0 )
                continue;
        #endif

        // Sample rate index
        if( ((buf[i+2] & 0x3c) >> 2) >= NUM_SAMPLE_RATES )
        {
            continue;
        }
#endif
        return i;
    }

    return -1;
}

int ADTSSyncInfo( unsigned char *buf)
{
    int i_frame_size;

    /* Variable header */
    i_frame_size = ((buf[3] & 0x03) << 11) | (buf[4] << 3) | ((buf[5] >> 5) /*& 0x7*/);
    return i_frame_size;
}
/**************************************************************************************
 * Function:    AACGetLastFrameInfo
 *
 * Description: get info about last AAC frame decoded (number of samples decoded,
 *                sample rate, bit rate, etc.)
 *
 * Inputs:      valid AAC decoder instance pointer (HAACDecoder)
 *              pointer to AACFrameInfo struct
 *
 * Outputs:     filled-in AACFrameInfo struct
 *
 * Return:      none
 *
 * Notes:       call this right after calling AACDecode()
 **************************************************************************************/
void AACGetLastFrameInfo(HAACDecoder hAACDecoder, AACFrameInfo *aacFrameInfo)
{
    AACDecInfo *aacDecInfo = (AACDecInfo *)hAACDecoder;
    int nTemp;

    if (!aacDecInfo)
    {
        aacFrameInfo->bitRate =       0;
        aacFrameInfo->nChans =        0;
        aacFrameInfo->sampRateCore =  0;
        aacFrameInfo->sampRateOut =   0;
        aacFrameInfo->bitsPerSample = 0;
        aacFrameInfo->outputSamps =   0;
        aacFrameInfo->profile =       0;
        aacFrameInfo->tnsUsed =       0;
        aacFrameInfo->pnsUsed =       0;
    } 
    else
    {
        aacFrameInfo->bitRate =       aacDecInfo->bitRate;
        aacFrameInfo->nChans =        aacDecInfo->nChans;
        aacFrameInfo->sampRateCore =  aacDecInfo->sampRate;
        aacFrameInfo->sampRateOut =   aacDecInfo->sampRate * (aacDecInfo->sbrEnabled ? 2 : 1);
        aacFrameInfo->bitsPerSample = 16;
       #ifndef SUPPORT_MORE_THAN_2_CHANNELS
           if(aacFrameInfo->nChans>=2 && aacFrameInfo->nChans<=AAC_MAX_NCHANS)
           {
               nTemp = 2;
           }
           else
           {
               nTemp = aacFrameInfo->nChans;
           }
       #else
           nTemp=aacFrameInfo->nChans;
       #endif //#ifndef SUPPORT_MORE_THAN_2_CHANNELS

      #ifdef ENABLE_DOWNMIX_CHANNELS
          nTemp=aacFrameInfo->nChans;
      #endif

        aacFrameInfo->outputSamps = nTemp * AAC_MAX_NSAMPS * (aacDecInfo->sbrEnabled ? 2 : 1);
        aacFrameInfo->profile = aacDecInfo->profile;
        aacFrameInfo->tnsUsed = aacDecInfo->tnsUsed;
        aacFrameInfo->pnsUsed = aacDecInfo->pnsUsed;
    }
}

#if defined(RAW_BLOCK)
/**************************************************************************************
 * Function:    AACSetRawBlockParams
 *
 * Description: set internal state variables for decoding a stream of raw data blocks
 *
 * Inputs:      valid AAC decoder instance pointer (HAACDecoder)
 *              flag indicating source of parameters
 *              AACFrameInfo struct, with the members nChans, sampRate, and profile
 *                optionally filled-in
 *
 * Outputs:     updated codec state
 *
 * Return:      0 if successful, error code (< 0) if error
 *
 * Notes:       if copyLast == 1, then the codec sets up its internal state (for
 *                decoding raw blocks) based on previously-decoded ADTS header info
 *              if copyLast == 0, then the codec uses the values passed in
 *                aacFrameInfo to configure its internal state (useful when the
 *                source is MP4 format, for example)
 **************************************************************************************/
int AACSetRawBlockParams(HAACDecoder hAACDecoder, int copyLast, AACFrameInfo *aacFrameInfo)
{
    AACDecInfo *aacDecInfo = (AACDecInfo *)hAACDecoder;

    if (!aacDecInfo)
    {
        return ERR_AAC_NULL_POINTER;
    }
    aacDecInfo->format = AAC_FF_RAW;

    if (copyLast)
    {
        return SetRawBlockParams(aacDecInfo, 1, 0, 0, 0);
    }
    else
    {
        return SetRawBlockParams(aacDecInfo, 0, aacFrameInfo->nChans, aacFrameInfo->sampRateCore, aacFrameInfo->profile);
    }
}
#endif // defined(RAW_BLOCK)

/**************************************************************************************
 * Function:    AACFlushCodec
 *
 * Description: flush internal codec state (after seeking, for example)
 *
 * Inputs:      valid AAC decoder instance pointer (HAACDecoder)
 *
 * Outputs:     updated state variables in aacDecInfo
 *
 * Return:      0 if successful, error code (< 0) if error
 **************************************************************************************/
int AACFlushCodec(HAACDecoder hAACDecoder)
{
    int ch;
    int nResult;
    AACDecInfo *aacDecInfo = (AACDecInfo *)hAACDecoder;
    if (!aacDecInfo)
    {
      printf("[AAC] AACFlushCodec ERR_AAC_NULL_POINTER \n");
        return ERR_AAC_NULL_POINTER;
    }
    /* reset common state variables which change per-frame
     * don't touch state variables which are (usually) constant for entire clip
     *   (nChans, sampRate, profile, format, sbrEnabled)
     */
    aacDecInfo->prevBlockID = AAC_ID_INVALID;
    aacDecInfo->currBlockID = AAC_ID_INVALID;
    aacDecInfo->currInstTag = -1;
    for (ch = 0; ch < MAX_NCHANS_ELEM; ch++)
    {
        aacDecInfo->sbDeinterleaveReqd[ch] = 0;
    }
    aacDecInfo->adtsBlocksLeft = 0;
    aacDecInfo->tnsUsed = 0;
    aacDecInfo->pnsUsed = 0;

    /* reset internal codec state (flush overlap buffers, etc.) */
    nResult = FlushCodec(aacDecInfo);
    if (nResult)
    {
        printf("[AAC] AACFlushCodec FlushCodec %d \n",nResult);
    }
    
    #if defined(AAC_ENABLE_SBR)
        nResult = FlushCodecSBR(aacDecInfo);
        if (nResult)
        {
            printf("[AAC] AACFlushCodec FlushCodecSBR %d \n",nResult);
        }
    #endif // defined(AAC_ENABLE_SBR)
    return ERR_AAC_NONE;
}

/**************************************************************************************
 * Function:    AACDecode
 *
 * Description: decode AAC frame
 *
 * Inputs:      valid AAC decoder instance pointer (HAACDecoder)
 *              double pointer to buffer of AAC data
 *              pointer to number of valid bytes remaining in inbuf
 *              pointer to outbuf, big enough to hold one frame of decoded PCM samples
 *                (outbuf must be double-sized if SBR enabled)
 *
 * Outputs:     PCM data in outbuf, interleaved LRLRLR... if stereo
 *                number of output samples = 1024 per channel (2048 if SBR enabled)
 *              updated inbuf pointer
 *              updated bytesLeft
 *
 * Return:      0 if successful, error code (< 0) if error
 *
 * Notes:       inbuf pointer and bytesLeft are not updated until whole frame is
 *                successfully decoded, so if ERR_AAC_INDATA_UNDERFLOW is returned
 *                just call AACDecode again with more data in inbuf
 **************************************************************************************/
int AACDecode(HAACDecoder hAACDecoder, unsigned char **inbuf, int *bytesLeft, short *outbuf)
{

    int err, offset, bitOffset, bitsAvail;
    int ch, baseChan, baseChanSBR, elementChans;
    unsigned char *inptr;
    int nTemp;
    int nResult ;   
#if defined(AAC_ENABLE_SBR)
    int elementChansSBR;
#endif // defined(AAC_ENABLE_SBR)
    latm_mux_t* ptLatm;
    PSInfoSBR *psi;

    AACDecInfo *aacDecInfo = (AACDecInfo *)hAACDecoder;
    psi = (PSInfoSBR *)(aacDecInfo->psInfoSBR);

#if!defined(__OPENRTOS__) 
    ptLatm = NULL;
#endif
#if defined(REVERBERATION) || defined(DRCTL)
    int *ch0 = 0, *ch1 = 0;
    short *outpcm;
    int i;
    int *buf = 0;
    static int decbufIdx = 0;  // for index
#endif // defined(REVERBERATION) || defined(DRCTL)

    if (!aacDecInfo)
    {
        return ERR_AAC_NULL_POINTER;
    }

    /* make local copies (see "Notes" above) */
    inptr = *inbuf;
    bitOffset = 0;
    bitsAvail = (*bytesLeft) << 3;

    aacDecInfo->nBitOffsets=0;
    /* first time through figure out what the file format is */
    if (aacDecInfo->format == AAC_FF_Unknown) 
    {
        if (bitsAvail < 32)
        {
            return ERR_AAC_INDATA_UNDERFLOW;
        }

        if (IS_ADIF(inptr)) 
        {
            PRINTF("ADIF Format\n");
            /* unpack ADIF header */
            aacDecInfo->format = AAC_FF_ADIF;
            err = UnpackADIFHeader(aacDecInfo, &inptr, &bitOffset, &bitsAvail);
            if (err)
                return err;
        }
        else if( inptr[0] == 0x56 && (inptr[1] & 0xe0) == 0xe0)
        {
            //printf("[aac]LATM format \n");
            aacDecInfo->format = AAC_FF_LATM;
        }
        else
        {
            PRINTF("ADTS Format\n");
            /* assume ADTS by default */
            aacDecInfo->format = AAC_FF_ADTS;
        }
    }

    /* if ADTS, search for start of next frame */
    if (aacDecInfo->format == AAC_FF_ADTS) 
    {
        /* can have 1-4 raw data blocks per ADTS frame (header only present for first one) */
        if (aacDecInfo->adtsBlocksLeft == 0)
        {
            offset = AACFindSyncWord(inptr, bitsAvail >> 3);
            if (offset < 0)
                return ERR_AAC_INDATA_UNDERFLOW;
            inptr += offset;
            bitsAvail -= (offset << 3);

            err = UnpackADTSHeader(aacDecInfo, &inptr, &bitOffset, &bitsAvail);
            if (err)
                return err;

            if (aacDecInfo->nChans == -1) 
            {
                /* figure out implicit channel mapping if necessary */
                err = GetADTSChannelMapping(aacDecInfo, inptr, bitOffset, bitsAvail);
                if (err)
                    return err;
            }
        }
        aacDecInfo->adtsBlocksLeft--;
    }
    else if (aacDecInfo->format == AAC_FF_RAW) 
    {
        err = PrepareRawBlock(aacDecInfo);
        if (err)
            return err;
    }
    else if (aacDecInfo->format == AAC_FF_LATM) 
    {
        /* Check if frame is valid and get frame info */
        aacDecInfo->nFrameLength = LOASSyncInfo( inptr );
        //printf("AAC LATM frame length %d \n",aacDecInfo->nFrameLength);
        inptr += LOAS_HEADER_SIZE;
        nResult = LOASParse(hAACDecoder,inptr,aacDecInfo->nFrameLength);
        if (aacDecInfo->nFrameLength > *bytesLeft || aacDecInfo->nFrameLength>1500)
        {
            printf("[AAC] aacdec latm nFrameLength too large %d \n",aacDecInfo->nFrameLength);
            aacDecInfo->nFrameLength=0;
            aacDecInfo->format = AAC_FF_Unknown;
            return ERR_AAC_INVALID_FRAME;
        }
        if (nResult == 0)
        {
            inptr+=aacDecInfo->nFrameLength - aacDecInfo->nPayloadSize-1;            
            //printf("[AAC]  latm channel %d samplerate %d  \n",aacDecInfo->nChans,aacDecInfo->sampRate);
        }
        else
        {
            inptr+=aacDecInfo->nFrameLength;         
            return nResult;
        }

    }
#ifdef LATM_TO_ADTS

#else
    #if defined(__DEBUG__)
        if (aacDecInfo->tnsUsed   ) PRINTF("TNS\n");
        if (aacDecInfo->pnsUsed   ) PRINTF("PNS\n");
        if (aacDecInfo->sbrEnabled) PRINTF("SBR\n");
    #endif // defined(__DEBUG__)

    /* check for valid number of channels */
    if (aacDecInfo->nChans > AAC_MAX_NCHANS || aacDecInfo->nChans <= 0) 
    {
        PRINTF("Can not decode the AAC stream with %d channels.\n", aacDecInfo->nChans);
        return ERR_AAC_NCHANS_TOO_HIGH;
    }

    /* will be set later if active in this frame */
    aacDecInfo->tnsUsed = 0;
    aacDecInfo->pnsUsed = 0;

    //bitOffset = 0;
    bitOffset = aacDecInfo->nBitOffsets;
    baseChan = 0;
    baseChanSBR = 0;

    #if defined(DRCTL)
    if(aacDecInfo->frameCount == 0) //initialize DRC's related parameters
        initDRC(&drcinfo, decbuf, aacDecInfo->nChans, AAC_MAX_NSAMPS, DECBUFFRAMES);
    #endif // defined(DRCTL)

    do 
    {
        /* parse next syntactic element */
        err = DecodeNextElement(aacDecInfo, &inptr, &bitOffset, &bitsAvail);
        if (err)
        {
            return err;
        }

        if (aacDecInfo->currBlockID<NUM_ELEMENTS && aacDecInfo->currBlockID>=0)
        {
            elementChans = elementNumChans[aacDecInfo->currBlockID];        
        }
        else
        {
            return ERR_AAC_NCHANS_TOO_HIGH;
        }
        if (baseChan + elementChans > AAC_MAX_NCHANS)
        {
            printf("[AAC] aacdecode baseChan(%d) + elementChans(%d) > AAC_MAX_NCHANS,%d \n",baseChan,elementChans,AAC_MAX_NCHANS);
            return ERR_AAC_NCHANS_TOO_HIGH;
        }

        /* noiseless decoder and dequantizer */
        for (ch = 0; ch < elementChans; ch++) 
        {
            err = DecodeNoiselessData(aacDecInfo, &inptr, &bitOffset, &bitsAvail, ch);
            if (err)
            {
                return err;
            }

            if (Dequantize(aacDecInfo, ch))
            {
                return ERR_AAC_DEQUANT;
            }
        }
        /* mid-side and intensity stereo */
        if (aacDecInfo->currBlockID == AAC_ID_CPE) 
        {
            if (StereoProcess(aacDecInfo))
            {
                return ERR_AAC_STEREO_PROCESS;
            }
        }

        #if defined(REVERBERATION) || defined(DRCTL)
        #if defined(REVERBERATION)
        {
            reverbStateNow = isReverbOn();

            if(!reverbStatePrev && reverbStateNow)
            {
                reverbStateNow = init_reverb_memory();
                //init_reverb_filter(aacDecInfo->nChans, aacDecInfo->sampRate, AAC_MAX_NSAMPS, decbufIdx, DECBUFSIZE);// update decbufsize here
            }
            else if(reverbStatePrev && !reverbStateNow)
            {
                release_reverb_memory();
            }
            if (reverbStateNow)
                buf = reverbbuf + decbufIdx;
            else
                buf = decbuf + decbufIdx;
        }
        #else
        buf = decbuf + decbufIdx;
        #endif // defined(REVERBERATION)
        #endif // defined(REVERBERATION) || defined(DRCTL)

        /* PNS, TNS, inverse transform */
        for (ch = 0; ch < elementChans; ch++) 
        {
            if (PNS(aacDecInfo, ch))
            {
                return ERR_AAC_PNS;
            }

            if (aacDecInfo->sbDeinterleaveReqd[ch]) 
            {
                /* deinterleave short blocks, if required */
                if (DeinterleaveShortBlocks(aacDecInfo, ch)) 
                {
                    return ERR_AAC_SHORT_BLOCK_DEINT;
                }
                aacDecInfo->sbDeinterleaveReqd[ch] = 0;
            }

            if (TNSFilter(aacDecInfo, ch)) 
            {
                return ERR_AAC_TNS;
            }

            #if defined(REVERBERATION) || defined(DRCTL)
            if (IMDCT(aacDecInfo, ch, baseChan + ch, buf)) 
            {
                return ERR_AAC_IMDCT;
            }
            #else
            if (IMDCT(aacDecInfo, ch, baseChan + ch, outbuf)) 
            {
                return ERR_AAC_IMDCT;
            }
            #endif // defined(REVERBERATION) || defined(DRCTL)
        }

        #if defined(FREQINFO)
        if (elementChans != 0)
        {
            updateFreqInfo(aacDecInfo->psInfoBase, elementChans);
        }
        #endif // defined(FREQINFO)

        // Reverberation Here
        #if defined(REVERBERATION)
        //if (elementChans != 0){
            if (revbinfo.updReverb != 0 && reverbStateNow == 1) 
            {
              //initial reverberation filter
              init_reverb_filter(aacDecInfo->nChans, aacDecInfo->sampRate, AAC_MAX_NSAMPS, decbufIdx, DECBUFSIZE);// update decbufsize here
              revbinfo.updReverb = 0;
            }
            if (reverbStateNow)
            { //do reverberation
                if (elementChans != 0)
                {
                    reverb_filter(aacDecInfo->nChans, AAC_MAX_NSAMPS, decbufIdx, DECBUFSIZE);
                }
            }
            reverbStatePrev = reverbStateNow;
        //}
        #endif // defined(REVERBERATION)

        // DRC Here
        #if defined(DRCTL)
        if (drcinfo.statenow == 1) {
            if (elementChans != 0) {
                DRCData(&drcinfo, drcoutbuf);
            }
        }
        #endif // defined(DRCTL)

    #if defined(REVERBERATION) || defined(DRCTL)
        // output: cliptoshort and merge channels
        if (elementChans == 1)
        {
        #if !defined(DRCTL)
            ch0 = decbuf+decbufIdx;
        #else // defined(DRCTL)
            if (drcinfo.statenow == 1) {
                ch0 = drcoutbuf;
            } else {
                ch0 = decbuf+decbufIdx;
            }
        #endif // !defined(DRCTL)

            outpcm = outbuf;
            for (i = 0; i < AAC_MAX_NSAMPS; i++) {
                *outpcm = CLIPTOSHORT(*ch0);
                outpcm++;
                ch0++;
            }
        } else if (elementChans == 2) {
        #if !defined(DRCTL)
            ch0 = decbuf+decbufIdx;
            ch1 = ch0+DECBUFSIZE;
        #else // defined(DRCTL)
            if (drcinfo.statenow == 1) {
                ch0 = drcoutbuf;
                ch1 = ch0+AAC_MAX_NSAMPS;
            } else {
                ch0 = decbuf+decbufIdx;
                ch1 = ch0+DECBUFSIZE;
            }
        #endif // !defined(DRCTL)

            outpcm = outbuf;
            for (i = 0; i < AAC_MAX_NSAMPS; i++)
            {
                *outpcm++ = CLIPTOSHORT(*ch0);
                *outpcm++ = CLIPTOSHORT(*ch1);
                ch0++;
                ch1++;
            }
        }

        //Update Decoder's buffer index
        if (elementChans != 0) 
        {
            decbufIdx += AAC_MAX_NSAMPS;
            if (decbufIdx == DECBUFSIZE) 
            {
                decbufIdx = 0;
            }
        }
    #endif // defined(REVERBERATION) || defined(DRCTL)

#if defined(AAC_ENABLE_SBR)
        if (aacDecInfo->sbrEnabled && (aacDecInfo->currBlockID == AAC_ID_FIL || aacDecInfo->currBlockID == AAC_ID_LFE)) 
        {
            if (aacDecInfo->currBlockID == AAC_ID_LFE)
                elementChansSBR = elementNumChans[AAC_ID_LFE];
            else if (aacDecInfo->currBlockID == AAC_ID_FIL && (aacDecInfo->prevBlockID == AAC_ID_SCE || aacDecInfo->prevBlockID == AAC_ID_CPE))
                elementChansSBR = elementNumChans[aacDecInfo->prevBlockID];
            else
                elementChansSBR = 0;

            if (baseChanSBR + elementChansSBR > AAC_MAX_NCHANS)
                return ERR_AAC_SBR_NCHANS_TOO_HIGH;

            /* parse SBR extension data if present (contained in a fill element) */
            nResult = DecodeSBRBitstream(aacDecInfo, baseChanSBR);
            if (nResult)
            {
                printf("[AAC]aacdec DecodeSBRBitstream err %d \n",nResult);
                return nResult;
            }

            /* apply SBR */
        #if defined(REVERBERATION) || defined(DRCTL)
            // output: cliptoshort and merge channels
            if (elementChansSBR == 1) 
            {
            #if !defined(DRCTL)
                ch0 = decbuf+decbufIdx;
            #else // defined(DRCTL)
                if (drcinfo.statenow == 1) 
                {
                    ch0 = drcoutbuf;
                }
                else
                {
                    ch0 = decbuf+decbufIdx;
                }
            #endif // !defined(DRCTL)
            } 
            else if (elementChansSBR == 2) 
            {
            #if !defined(DRCTL)
                ch0 = decbuf+decbufIdx;
                ch1 = ch0+DECBUFSIZE;
            #else // defined(DRCTL)
                if (drcinfo.statenow == 1) 
                {
                    ch0 = drcoutbuf;
                    ch1 = ch0+AAC_MAX_NSAMPS;
                } else {
                    ch0 = decbuf+decbufIdx;
                    ch1 = ch0+DECBUFSIZE;
                }
            #endif // !defined(DRCTL)
            }
            aacDecInfo->rawSampleBuf[0] = ch0;
            if (aacDecInfo->nChans > 1)
                aacDecInfo->rawSampleBuf[1] = ch1;
        #endif //defined(REVERBERATION) || defined(DRCTL)
        
            nResult =DecodeSBRData(aacDecInfo, baseChanSBR, outbuf);
            if (nResult)
            {
                printf("[AAC]aacdec DecodeSBRData err %d \n",nResult);
                return nResult;
            }
    #ifdef PARSING_HE_AAC_V2            
            if (psi->ps_used)
            {
                if (aacDecInfo->nChans==1)
                {
                    aacDecInfo->nChans=2;
                }
                nResult = sbrDecodeSingleFramePS(aacDecInfo,outbuf,0,0);
                if (nResult)
                {
                    return nResult;
                }
            }
    #endif            

            baseChanSBR += elementChansSBR;
        }
#endif // defined(AAC_ENABLE_SBR)

        baseChan += elementChans;
    } while (aacDecInfo->currBlockID != AAC_ID_END);

    /* byte align after each raw_data_block */
    if (bitOffset)
    {
        inptr++;
        bitsAvail -= (8-bitOffset);
        bitOffset = 0;
        if (bitsAvail < 0) 
        {
            return ERR_AAC_INDATA_UNDERFLOW;
        }
    }

#endif
    /* update pointers */
    aacDecInfo->frameCount++;
     if (aacDecInfo->format == AAC_FF_LATM) 
     {
        inptr+=aacDecInfo->nFrameLength-(inptr - *inbuf)+LOAS_HEADER_SIZE;
     }
    *bytesLeft -= (inptr - *inbuf);
    *inbuf = inptr;

    return ERR_AAC_NONE;
}

