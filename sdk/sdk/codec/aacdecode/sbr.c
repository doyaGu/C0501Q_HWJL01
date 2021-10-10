/* ***** BEGIN LICENSE BLOCK *****
 * Source last modified: $Id: sbr.c,v 1.3 2005/05/24 16:01:55 albertofloyd Exp $
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
 * sbr.c - top level functions for SBR
 **************************************************************************************/

#include "aacdec.h"

#if defined(ENABLE_CODECS_PLUGIN)
#include "plugin.h"
#endif

#if defined(AAC_ENABLE_SBR)
#include "sbr.h"
#include "assembly.h"

#define RND_VAL (1 << (FBITS_OUT_IMDCT-1))
//#define FAAD_DECODE

#ifdef PARSING_HE_AAC_V2            
    #ifdef FAAD_DECODE
    #define FBITS_OUT_QMFS  (FBITS_IN_QMFS - FBITS_LOST_DCT4_64 + 6 - 1+7)
    void dct4_kernel(real_t * in_real, real_t * in_imag, real_t * out_real, real_t * out_imag);
    static void fft_dif(real_t * Real, real_t * Imag);
    void sbr_qmf_synthesis_64(int *delay, int *delayIdx, qmf_t X[40][64],short *output);
    #endif
#endif
static PSInfoSBR PSInfoSBR_;

#ifdef AAC_ENABLE_INTERNAL_SD
static unsigned int* gBufSD= INTERNAL_SD;
static unsigned int* gBufSD2 = INTERNAL_SD+4096;
#endif
/**************************************************************************************
 * Function:    InitSBRState
 *
 * Description: initialize PSInfoSBR struct at start of stream or after flush
 *
 * Inputs:      valid AACDecInfo struct
 *
 * Outputs:     PSInfoSBR struct with proper initial state
 *
 * Return:      none
 **************************************************************************************/
static void InitSBRState(PSInfoSBR *psi)
{
    int i, ch;
    unsigned char *c;

    if (!psi)
        return;

    /* clear SBR state structure */
    c = (unsigned char *)psi;
    for (i = 0; i < (int)sizeof(PSInfoSBR); i++)
        *c++ = 0;

    /* initialize non-zero state variables */
    for (ch = 0; ch < AAC_MAX_NCHANS; ch++) {
        psi->sbrChan[ch].reset = 1;
        psi->sbrChan[ch].laPrev = -1;
    }
}

/**************************************************************************************
 * Function:    InitSBR
 *
 * Description: initialize SBR decoder
 *
 * Inputs:      valid AACDecInfo struct
 *
 * Outputs:     PSInfoSBR struct to hold SBR state information
 *
 * Return:      0 if successful, error code (< 0) if error
 *
 * Note:        memory allocation for SBR is only done here
 **************************************************************************************/
int InitSBR(AACDecInfo *aacDecInfo)
{
    PSInfoSBR *psi;

    psi = &PSInfoSBR_;
    InitSBRState(psi);

    aacDecInfo->psInfoSBR = psi;
    return ERR_AAC_NONE;
}

/**************************************************************************************
 * Function:    FreeSBR
 *
 * Description: free SBR decoder
 *
 * Inputs:      valid AACDecInfo struct
 *
 * Outputs:     none
 *
 * Return:      none
 *
 * Note:        memory deallocation for SBR is only done here
 **************************************************************************************/
/*
void FreeSBR(AACDecInfo *aacDecInfo)
{
    if (aacDecInfo && aacDecInfo->psInfoSBR)
        free(aacDecInfo->psInfoSBR);

    return;
}
*/

/**************************************************************************************
 * Function:    DecodeSBRBitstream
 *
 * Description: decode sideband information for SBR
 *
 * Inputs:      valid AACDecInfo struct
 *              fill buffer with SBR extension block
 *              number of bytes in fill buffer
 *              base output channel (range = [0, nChans-1])
 *
 * Outputs:     initialized state structs (SBRHdr, SBRGrid, SBRFreq, SBRChan)
 *
 * Return:      0 if successful, error code (< 0) if error
 *
 * Notes:       SBR payload should be in aacDecInfo->fillBuf
 *              returns with no error if fill buffer is not an SBR extension block,
 *                or if current block is not a fill block (e.g. for LFE upsampling)
 **************************************************************************************/
int DecodeSBRBitstream(AACDecInfo *aacDecInfo, int chBase)
{
    int headerFlag;
    BitStreamInfo bsi;
    PSInfoSBR *psi;
    int nResult;
    
    /* validate pointers */
    if (!aacDecInfo || !aacDecInfo->psInfoSBR)
        return ERR_AAC_NULL_POINTER;
    psi = (PSInfoSBR *)(aacDecInfo->psInfoSBR);

    if (aacDecInfo->currBlockID != AAC_ID_FIL || (aacDecInfo->fillExtType != EXT_SBR_DATA && aacDecInfo->fillExtType != EXT_SBR_DATA_CRC))
        return ERR_AAC_NONE;

    SetBitstreamPointer(&bsi, aacDecInfo->fillCount, aacDecInfo->fillBuf);
    if (GetBits(&bsi, 4) != (unsigned int)aacDecInfo->fillExtType)
        return ERR_AAC_SBR_BITSTREAM;

    if (aacDecInfo->fillExtType == EXT_SBR_DATA_CRC)
        psi->crcCheckWord = GetBits(&bsi, 10);

    headerFlag = GetBits(&bsi, 1);
    if (headerFlag) {
        /* get sample rate index for output sample rate (2x base rate) */
        psi->sampRateIdx = GetSampRateIdx(2 * aacDecInfo->sampRate);
        if (psi->sampRateIdx < 0 || psi->sampRateIdx >= NUM_SAMPLE_RATES)
            return ERR_AAC_SBR_BITSTREAM;
        else if (psi->sampRateIdx >= NUM_SAMPLE_RATES_SBR)
            return ERR_AAC_SBR_SINGLERATE_UNSUPPORTED;

        /* reset flag = 1 if header values changed */
        if (UnpackSBRHeader(&bsi, &(psi->sbrHdr[chBase])))
            psi->sbrChan[chBase].reset = 1;

        /* first valid SBR header should always trigger CalcFreqTables(), since psi->reset was set in InitSBR() */
        if (psi->sbrChan[chBase].reset)
        {
            nResult =CalcFreqTables(&(psi->sbrHdr[chBase+0]), &(psi->sbrFreq[chBase]), psi->sampRateIdx);
            if (nResult)
            {
                return nResult;
            }
        }

        /* copy and reset state to right channel for CPE */
        if (aacDecInfo->prevBlockID == AAC_ID_CPE)
            psi->sbrChan[chBase+1].reset = psi->sbrChan[chBase+0].reset;
    }

    /* if no header has been received, upsample only */
    if (psi->sbrHdr[chBase].count == 0)
        return ERR_AAC_NONE;

    if (aacDecInfo->prevBlockID == AAC_ID_SCE)
    {
        nResult = UnpackSBRSingleChannel(&bsi, psi, chBase);
    }
    else if (aacDecInfo->prevBlockID == AAC_ID_CPE) 
    {
        nResult = UnpackSBRChannelPair(&bsi, psi, chBase);
    }
    else
    {
        return ERR_AAC_SBR_BITSTREAM;
    }
    if (nResult)
    {
        return nResult;
    }

    ByteAlignBitstream(&bsi);

    return ERR_AAC_NONE;
}

/**************************************************************************************
 * Function:    DecodeSBRData
 *
 * Description: apply SBR to one frame of PCM data
 *
 * Inputs:      1024 samples of decoded 32-bit PCM, before SBR
 *              size of input PCM samples (must be 4 bytes)
 *              number of fraction bits in input PCM samples
 *              base output channel (range = [0, nChans-1])
 *              initialized state structs (SBRHdr, SBRGrid, SBRFreq, SBRChan)
 *
 * Outputs:     2048 samples of decoded 16-bit PCM, after SBR
 *
 * Return:      0 if successful, error code (< 0) if error
 **************************************************************************************/
int DecodeSBRData(AACDecInfo *aacDecInfo, int chBase, short *outbuf)
{
#if 0 //defined(AAC_ENABLE_SBR)
    int i, tmp0, tmp1, predit, ch;
    int *timeinbuf;
    PSInfoSBR *psi;

    for (ch = 0; ch < aacDecInfo->nChans; ch++) {
        timeinbuf = (int*)aacDecInfo->rawSampleBuf[ch];
        tmp0 = timeinbuf[0];
        for (i = 1; i < AAC_MAX_NSAMPS; i++) {
            //Upsample 2x
            tmp1 = timeinbuf[i];
            predit = (tmp1>>1) + (tmp0>>1);
            #if defined(REVERBERATION) || defined(DRCTL)
            outbuf[2*aacDecInfo->nChans*(i-1) + ch] = CLIPTOSHORT(tmp0);
            outbuf[2*aacDecInfo->nChans*(i-1) + aacDecInfo->nChans + ch] = CLIPTOSHORT(predit);
            #else
            outbuf[2*aacDecInfo->nChans*(i-1) + ch] = CLIPTOSHORT((tmp0+ RND_VAL) >> FBITS_OUT_IMDCT);
            outbuf[2*aacDecInfo->nChans*(i-1) + aacDecInfo->nChans + ch] = CLIPTOSHORT((predit+ RND_VAL) >> FBITS_OUT_IMDCT);
            #endif // defined(REVERBERATION) || defined(DRCTL)
            tmp0 = tmp1;
        }
        #if defined(REVERBERATION) || defined(DRCTL)
        outbuf[2*aacDecInfo->nChans*(i-1) + ch] = CLIPTOSHORT(tmp0);
        outbuf[2*aacDecInfo->nChans*(i-1) + aacDecInfo->nChans + ch] = CLIPTOSHORT(tmp0);
        #else
        outbuf[2*aacDecInfo->nChans*(i-1) + ch] = CLIPTOSHORT((tmp0+ RND_VAL) >> FBITS_OUT_IMDCT);
        outbuf[2*aacDecInfo->nChans*(i-1) + aacDecInfo->nChans + ch] = CLIPTOSHORT((tmp0+ RND_VAL) >> FBITS_OUT_IMDCT);
        #endif // defined(REVERBERATION) || defined(DRCTL)
    }

    psi = (PSInfoSBR *)(aacDecInfo->psInfoSBR);
#else
    int k, l, ch, chBlock, qmfaBands, qmfsBands;
    int upsampleOnly, gbIdx, gbMask;
    int *inbuf;
    short *outptr;
    int nResult;
    PSInfoSBR *psi;
    SBRHeader *sbrHdr;
    SBRGrid *sbrGrid;
    SBRFreq *sbrFreq;
    SBRChan *sbrChan;

    /* validate pointers */
    if (!aacDecInfo || !aacDecInfo->psInfoSBR)
        return ERR_AAC_NULL_POINTER;
    psi = (PSInfoSBR *)(aacDecInfo->psInfoSBR);

    /* same header and freq tables for both channels in CPE */
    sbrHdr =  &(psi->sbrHdr[chBase]);
    sbrFreq = &(psi->sbrFreq[chBase]);

    /* upsample only if we haven't received an SBR header yet or if we have an LFE block */
    if (aacDecInfo->currBlockID == AAC_ID_LFE) {
        chBlock = 1;
        upsampleOnly = 1;
    } else if (aacDecInfo->currBlockID == AAC_ID_FIL) {
        if (aacDecInfo->prevBlockID == AAC_ID_SCE)
            chBlock = 1;
        else if (aacDecInfo->prevBlockID == AAC_ID_CPE)
            chBlock = 2;
        else
            return ERR_AAC_NONE;

        upsampleOnly = (sbrHdr->count == 0 ? 1 : 0);
        if (aacDecInfo->fillExtType != EXT_SBR_DATA && aacDecInfo->fillExtType != EXT_SBR_DATA_CRC)
            return ERR_AAC_NONE;
    } else {
        /* ignore non-SBR blocks */
        return ERR_AAC_NONE;
    }

    if (upsampleOnly) {
        sbrFreq->kStart = 32;
        sbrFreq->numQMFBands = 0;
    }
#ifdef AAC_ENABLE_INTERNAL_SD
    psi->XbufTmp1 = gBufSD;
    psi->XbufTmp2 = gBufSD2;
#endif
    for (ch = 0; ch < chBlock; ch++) 
    {
        sbrGrid = &(psi->sbrGrid[chBase + ch]);
        sbrChan = &(psi->sbrChan[chBase + ch]);

        if (aacDecInfo->rawSampleBuf[ch] == 0 || aacDecInfo->rawSampleBytes != 4)
            return ERR_AAC_SBR_PCM_FORMAT;
        inbuf = (int *)aacDecInfo->rawSampleBuf[ch];
        outptr = outbuf + chBase + ch;

        /* restore delay buffers (could use ring buffer or keep in temp buffer for nChans == 1) */
#ifdef AAC_ENABLE_INTERNAL_SD
        for (l = 0; l < HF_GEN; l++) {
            for (k = 0; k < 64; k++) {
                psi->XbufTmp1->bufTemp[l][k][0] = psi->XBufDelay[chBase + ch][l][k][0];
                psi->XbufTmp1->bufTemp[l][k][1] = psi->XBufDelay[chBase + ch][l][k][1];
            }
        }
#else
        for (l = 0; l < HF_GEN; l++) {
            for (k = 0; k < 64; k++) {
                psi->XBuf[l][k][0] = psi->XBufDelay[chBase + ch][l][k][0];
                psi->XBuf[l][k][1] = psi->XBufDelay[chBase + ch][l][k][1];
            }
        }
#endif

        /* step 1 - analysis QMF */
        qmfaBands = sbrFreq->kStart;
#ifdef AAC_ENABLE_INTERNAL_SD
        for (l = 0; l < HF_GEN; l++) {
            gbMask = QMFAnalysis(inbuf + l*32, psi->delayQMFA[chBase + ch], psi->XbufTmp2->bufTemp[l][0],
                aacDecInfo->rawSampleFBits, &(psi->delayIdxQMFA[chBase + ch]), qmfaBands);

            gbIdx = ((l + HF_GEN) >> 5) & 0x01;
            sbrChan->gbMask[gbIdx] |= gbMask;   /* gbIdx = (0 if i < 32), (1 if i >= 32) */
        }
        for (l = HF_GEN; l < 32; l++) {
            gbMask = QMFAnalysis(inbuf + l*32, psi->delayQMFA[chBase + ch], psi->XBuf[l + HF_GEN][0],
                                                aacDecInfo->rawSampleFBits, &(psi->delayIdxQMFA[chBase + ch]), qmfaBands);

            gbIdx = ((l + HF_GEN) >> 5) & 0x01;
            sbrChan->gbMask[gbIdx] |= gbMask;   /* gbIdx = (0 if i < 32), (1 if i >= 32) */
            }

#else
        for (l = 0; l < 32; l++) {
            gbMask = QMFAnalysis(inbuf + l*32, psi->delayQMFA[chBase + ch], psi->XBuf[l + HF_GEN][0],
                aacDecInfo->rawSampleFBits, &(psi->delayIdxQMFA[chBase + ch]), qmfaBands);

            gbIdx = ((l + HF_GEN) >> 5) & 0x01;
            sbrChan->gbMask[gbIdx] |= gbMask;   /* gbIdx = (0 if i < 32), (1 if i >= 32) */
        }
#endif

        if (upsampleOnly) {
            /* no SBR - just run synthesis QMF to upsample by 2x */
            qmfsBands = 32;
#ifdef AAC_ENABLE_INTERNAL_SD
            for (l = 0; l < 32; l++) {
                /* step 4 - synthesis QMF */
                if (l + HF_ADJ<8) {
                    QMFSynthesis(psi->XbufTmp1->bufTemp[l + HF_ADJ][0], psi->delayQMFS[chBase + ch], &(psi->delayIdxQMFS[chBase + ch]), qmfsBands, outptr, aacDecInfo->nChans);
                } else if (l + HF_ADJ<16) {
                    QMFSynthesis(psi->XbufTmp2->bufTemp[l + HF_ADJ -8][0], psi->delayQMFS[chBase + ch], &(psi->delayIdxQMFS[chBase + ch]), qmfsBands, outptr, aacDecInfo->nChans);
                } else {
                    QMFSynthesis(psi->XBuf[l + HF_ADJ][0], psi->delayQMFS[chBase + ch], &(psi->delayIdxQMFS[chBase + ch]), qmfsBands, outptr, aacDecInfo->nChans);
                }
                outptr += 64*aacDecInfo->nChans;
            }
#else
            for (l = 0; l < 32; l++) {
                /* step 4 - synthesis QMF */
                QMFSynthesis(psi->XBuf[l + HF_ADJ][0], psi->delayQMFS[chBase + ch], &(psi->delayIdxQMFS[chBase + ch]), qmfsBands, outptr, aacDecInfo->nChans);
                outptr += 64*aacDecInfo->nChans;
            }
#endif
        } else {
            /* if previous frame had lower SBR starting freq than current, zero out the synthesized QMF
             *   bands so they aren't used as sources for patching
             * after patch generation, restore from delay buffer
             * can only happen after header reset
             */
#ifdef AAC_ENABLE_INTERNAL_SD
            for (k = sbrFreq->kStartPrev; k < sbrFreq->kStart; k++) {
                for (l = 0; l < sbrGrid->envTimeBorder[0] + HF_ADJ; l++) {
                    if (l<8){
                        psi->XbufTmp1->bufTemp[l][k][0] = 0;
                        psi->XbufTmp1->bufTemp[l][k][1] = 0;
                    } else if (l<16) {
                        psi->XbufTmp2->bufTemp[l-8][k][0] = 0;
                        psi->XbufTmp2->bufTemp[l-8][k][1] = 0;
                    } else if (l>=16){
                    psi->XBuf[l][k][0] = 0;
                    psi->XBuf[l][k][1] = 0;
                }
            }
            }

#else
            for (k = sbrFreq->kStartPrev; k < sbrFreq->kStart; k++) {
                for (l = 0; l < sbrGrid->envTimeBorder[0] + HF_ADJ; l++) {
                    psi->XBuf[l][k][0] = 0;
                    psi->XBuf[l][k][1] = 0;
            }           
            }           
#endif
            /* step 2 - HF generation */
            GenerateHighFreq(psi, sbrGrid, sbrFreq, sbrChan, ch);

            /* restore SBR bands that were cleared before patch generation (time slots 0, 1 no longer needed) */
#ifdef AAC_ENABLE_INTERNAL_SD
            for (k = sbrFreq->kStartPrev; k < sbrFreq->kStart; k++) {
                for (l = HF_ADJ; l < sbrGrid->envTimeBorder[0] + HF_ADJ; l++) {
                    if (l<8){
                        psi->XbufTmp1->bufTemp[l][k][0] = psi->XBufDelay[chBase + ch][l][k][0];
                        psi->XbufTmp1->bufTemp[l][k][1] = psi->XBufDelay[chBase + ch][l][k][1];
                    } else if (l<16) {
                        psi->XbufTmp2->bufTemp[l-8][k][0] = psi->XBufDelay[chBase + ch][l][k][0];
                        psi->XbufTmp2->bufTemp[l-8][k][1] = psi->XBufDelay[chBase + ch][l][k][1];
                    } else if (l>=16){
                    psi->XBuf[l][k][0] = psi->XBufDelay[chBase + ch][l][k][0];
                    psi->XBuf[l][k][1] = psi->XBufDelay[chBase + ch][l][k][1];
                }
            }
            }
#else
            for (k = sbrFreq->kStartPrev; k < sbrFreq->kStart; k++) {
                for (l = HF_ADJ; l < sbrGrid->envTimeBorder[0] + HF_ADJ; l++) {
                    psi->XBuf[l][k][0] = psi->XBufDelay[chBase + ch][l][k][0];
                    psi->XBuf[l][k][1] = psi->XBufDelay[chBase + ch][l][k][1];
                }
            }
#endif

            /* step 3 - HF adjustment */
            nResult = AdjustHighFreq(psi, sbrHdr, sbrGrid, sbrFreq, sbrChan, ch);
            if (nResult) {
                printf("[AAC] AdjustHighFreq err%d \n",nResult);
                return nResult;
            }

            /* step 4 - synthesis QMF */
#ifdef PARSING_HE_AAC_V2            
            if (!psi->ps_used)
#endif                
            {
                qmfsBands = sbrFreq->kStartPrev + sbrFreq->numQMFBandsPrev;
#ifdef AAC_ENABLE_INTERNAL_SD
                for (l = 0; l < sbrGrid->envTimeBorder[0]; l++) {
                    /* if new envelope starts mid-frame, use old settings until start of first envelope in this frame */
                    if (l+ HF_ADJ<8) {
                        QMFSynthesis(psi->XbufTmp1->bufTemp[l + HF_ADJ][0], psi->delayQMFS[chBase + ch], &(psi->delayIdxQMFS[chBase + ch]), qmfsBands, outptr, aacDecInfo->nChans);
                    } else if(l+ HF_ADJ<16) {
                        QMFSynthesis(psi->XbufTmp2->bufTemp[l + HF_ADJ-8][0], psi->delayQMFS[chBase + ch], &(psi->delayIdxQMFS[chBase + ch]), qmfsBands, outptr, aacDecInfo->nChans);
                    } else {
                        QMFSynthesis(psi->XBuf[l + HF_ADJ][0], psi->delayQMFS[chBase + ch], &(psi->delayIdxQMFS[chBase + ch]), qmfsBands, outptr, aacDecInfo->nChans);
                    }
                    outptr += 64*aacDecInfo->nChans;
                }
                qmfsBands = sbrFreq->kStart + sbrFreq->numQMFBands;
                for (     ; l < 32; l++) 
                {
                    /* use new settings for rest of frame (usually the entire frame, unless the first envelope starts mid-frame) */
                    if (l+ HF_ADJ<8) {
                        QMFSynthesis(psi->XbufTmp1->bufTemp[l + HF_ADJ][0], psi->delayQMFS[chBase + ch], &(psi->delayIdxQMFS[chBase + ch]), qmfsBands, outptr, aacDecInfo->nChans);
                    } else if(l+ HF_ADJ<16) {
                        QMFSynthesis(psi->XbufTmp2->bufTemp[l + HF_ADJ-8][0], psi->delayQMFS[chBase + ch], &(psi->delayIdxQMFS[chBase + ch]), qmfsBands, outptr, aacDecInfo->nChans);
                    } else {
                        QMFSynthesis(psi->XBuf[l + HF_ADJ][0], psi->delayQMFS[chBase + ch], &(psi->delayIdxQMFS[chBase + ch]), qmfsBands, outptr, aacDecInfo->nChans);
                    }
                    outptr += 64*aacDecInfo->nChans;
                }
#else
                for (l = 0; l < sbrGrid->envTimeBorder[0]; l++) {
                    /* if new envelope starts mid-frame, use old settings until start of first envelope in this frame */
                    QMFSynthesis(psi->XBuf[l + HF_ADJ][0], psi->delayQMFS[chBase + ch], &(psi->delayIdxQMFS[chBase + ch]), qmfsBands, outptr, aacDecInfo->nChans);
                    outptr += 64*aacDecInfo->nChans;
                }
                qmfsBands = sbrFreq->kStart + sbrFreq->numQMFBands;
                for (     ; l < 32; l++) 
                {
                    /* use new settings for rest of frame (usually the entire frame, unless the first envelope starts mid-frame) */
                    QMFSynthesis(psi->XBuf[l + HF_ADJ][0], psi->delayQMFS[chBase + ch], &(psi->delayIdxQMFS[chBase + ch]), qmfsBands, outptr, aacDecInfo->nChans);
                    outptr += 64*aacDecInfo->nChans;
                }
#endif
            }
        }

        /* save delay */
#ifdef PARSING_HE_AAC_V2                    
        if (!psi->ps_used)
#endif            
        {
            for (l = 0; l < HF_GEN; l++) 
            {
                for (k = 0; k < 64; k++) 
                {
                  psi->XBufDelay[chBase + ch][l][k][0] = psi->XBuf[l+32][k][0];
                  psi->XBufDelay[chBase + ch][l][k][1] = psi->XBuf[l+32][k][1];
                }
            }
        }
        sbrChan->gbMask[0] = sbrChan->gbMask[1];
        sbrChan->gbMask[1] = 0;
        if (sbrHdr->count > 0)
            sbrChan->reset = 0;
    }

#ifdef PARSING_HE_AAC_V2            
    if (!psi->ps_used)
#endif        
    {
        sbrFreq->kStartPrev = sbrFreq->kStart;
        sbrFreq->numQMFBandsPrev = sbrFreq->numQMFBands;
    }
#endif // defined(AAC_ENABLE_SBR)

    if (aacDecInfo->nChans > 0 && (chBase + ch) == aacDecInfo->nChans)
        psi->frameCount++;

    return ERR_AAC_NONE;
}

/**************************************************************************************
 * Function:    FlushCodecSBR
 *
 * Description: flush internal SBR codec state (after seeking, for example)
 *
 * Inputs:      valid AACDecInfo struct
 *
 * Outputs:     updated state variables for SBR
 *
 * Return:      0 if successful, error code (< 0) if error
 *
 * Notes:       SBR is heavily dependent on state from previous frames
 *                (e.g. delta coded scalefactors, previous envelope boundaries, etc.)
 *              On flush, we reset everything as if SBR had just been initialized
 *                for the first time. This triggers "upsample-only" mode until
 *                the first valid SBR header is received. Then SBR starts as usual.
 **************************************************************************************/
int FlushCodecSBR(AACDecInfo *aacDecInfo)
{
    PSInfoSBR *psi;

    /* validate pointers */
    if (!aacDecInfo || !aacDecInfo->psInfoSBR)
        return ERR_AAC_NULL_POINTER;
    psi = (PSInfoSBR *)(aacDecInfo->psInfoSBR);

    InitSBRState(psi);

    return 0;
}

#endif /* defined(AAC_ENABLE_SBR) */

#if (defined(PS_DEC) || defined(DRM_PS))
int sbrDecodeSingleFramePS(AACDecInfo *aacDecInfo, short *outbuf,
                               const uint8_t just_seeked, const uint8_t downSampledSBR)
{
    uint8_t l, k,j;
    uint8_t dont_process = 0;
    uint8_t ret = 0;
    ALIGN qmf_t X_left[38][64] = {{0}};
    ALIGN qmf_t X_right[38][64] = {{0}}; /* must set this to 0 */
    int nResult;
    int numTimeSlotsRate;
    int qmfsBands;
    PSInfoSBR *psi;
    SBRFreq *sbrFreq;
    SBRGrid *sbrGrid;
    short* ptLeftChannel;
    short* ptRightChannel;

    if (!aacDecInfo || !aacDecInfo->psInfoSBR)
    {
        return ERR_AAC_NULL_POINTER;
    }
    psi = (PSInfoSBR *)(aacDecInfo->psInfoSBR);
    sbrFreq = &(psi->sbrFreq[0]);
    sbrGrid = &(psi->sbrGrid[0]);
    /* case can occur due to bit errors */
    if (aacDecInfo->prevBlockID != AAC_ID_SCE && aacDecInfo->prevBlockID != AAC_ID_LFE)
        return ERR_AAC_NONE;

    //if (sbr->ret || (sbr->header_count == 0))
    //{
        /* don't process just upsample */
      //  dont_process = 1;

        /* Re-activate reset for next frame */
      //  if (sbr->ret && sbr->Reset)
      //      sbr->bs_start_freq_prev = -1;
    //}

    if (just_seeked)
    {
        //sbr->just_seeked = 1;
    } else {
        //sbr->just_seeked = 0;
    }

    //if (sbr->qmfs[1] == NULL)
    //{
        //sbr->qmfs[1] = qmfs_init((downSampledSBR)?32:64);
    //}
    numTimeSlotsRate = SAMPLES_PER_SLOT*NUM_TIME_SLOTS;
    //sbr->ret += sbr_process_channel(sbr, left_channel, X_left, 0, dont_process, downSampledSBR);
    for (l = 0; l < numTimeSlotsRate ; l++)
    {
        uint8_t kx_band, M_band, bsco_band;

        if (l < sbrGrid->envTimeBorder[0])
        {
            kx_band = sbrFreq->kStartPrev;
            M_band = sbrFreq->numQMFBands;
            bsco_band = 0;
        } 
        else
        {
            kx_band = sbrFreq->kStart;
            M_band =  sbrFreq->numQMFBandsPrev;
            bsco_band = 0;
        }
#ifdef AAC_ENABLE_INTERNAL_SD
        for (k = 0; k < kx_band + bsco_band; k++)
        {
            if (l+HF_ADJ<8){
                QMF_RE(X_left[l][k]) = psi->XbufTmp1->bufTemp[l+HF_ADJ][k][0];
                QMF_IM(X_left[l][k]) = psi->XbufTmp1->bufTemp[l+HF_ADJ][k][1];
            } else if (l+HF_ADJ<16) {
                QMF_RE(X_left[l][k]) = psi->XbufTmp2->bufTemp[l+HF_ADJ-8][k][0];
                QMF_IM(X_left[l][k]) = psi->XbufTmp2->bufTemp[l+HF_ADJ-8][k][1];
            } else {
                QMF_RE(X_left[l][k]) = psi->XBuf[l+HF_ADJ][k][0];
                QMF_IM(X_left[l][k]) = psi->XBuf[l+HF_ADJ][k][1];
            }
        }
        for (k = kx_band + bsco_band; k < kx_band + M_band; k++)
        {
            if (l+HF_ADJ<8){
                QMF_RE(X_left[l][k]) = psi->XbufTmp1->bufTemp[l+HF_ADJ][k][0];
                QMF_IM(X_left[l][k]) = psi->XbufTmp1->bufTemp[l+HF_ADJ][k][1];
            } else if (l+HF_ADJ<16) {
                QMF_RE(X_left[l][k]) = psi->XbufTmp2->bufTemp[l+HF_ADJ-8][k][0];
                QMF_IM(X_left[l][k]) = psi->XbufTmp2->bufTemp[l+HF_ADJ-8][k][1];
            } else {
                QMF_RE(X_left[l][k]) = psi->XBuf[l+HF_ADJ][k][0];
                QMF_IM(X_left[l][k]) = psi->XBuf[l+HF_ADJ][k][1];
            }     
        }
#else
        for (k = 0; k < kx_band + bsco_band; k++)
        {
            QMF_RE(X_left[l][k]) = psi->XBuf[l+HF_ADJ][k][0];
            QMF_IM(X_left[l][k]) = psi->XBuf[l+HF_ADJ][k][1];
        }
        for (k = kx_band + bsco_band; k < kx_band + M_band; k++)
        {
            QMF_RE(X_left[l][k]) = psi->XBuf[l+HF_ADJ][k][0];
            QMF_IM(X_left[l][k]) = psi->XBuf[l+HF_ADJ][k][1];
        }
#endif
        for (k = max(kx_band + bsco_band, kx_band + M_band); k < 64; k++)
        {
            QMF_RE(X_left[l][k]) = 0;
            QMF_IM(X_left[l][k]) = 0;
        }
    }
    /* copy some extra data for PS */
#ifdef AAC_ENABLE_INTERNAL_SD
    for (l = numTimeSlotsRate; l < numTimeSlotsRate + 6; l++)
    {
        for (k = 0; k < 5; k++)
        {
            if (l+HF_ADJ<8){
                QMF_RE(X_left[l][k]) = psi->XbufTmp1->bufTemp[l+HF_ADJ][k][0];
                QMF_IM(X_left[l][k]) = psi->XbufTmp1->bufTemp[l+HF_ADJ][k][1];
            } else if (l+HF_ADJ<16) {
                QMF_RE(X_left[l][k]) = psi->XbufTmp2->bufTemp[l+HF_ADJ-8][k][0];
                QMF_IM(X_left[l][k]) = psi->XbufTmp2->bufTemp[l+HF_ADJ-8][k][1];
            } else {
            QMF_RE(X_left[l][k]) = psi->XBuf[l+HF_ADJ][k][0];
            QMF_IM(X_left[l][k]) = psi->XBuf[l+HF_ADJ][k][1];
        }
    }
    }

#else    
    for (l = numTimeSlotsRate; l < numTimeSlotsRate + 6; l++)
    {
        for (k = 0; k < 5; k++)
        {
            QMF_RE(X_left[l][k]) = psi->XBuf[l+HF_ADJ][k][0];
            QMF_IM(X_left[l][k]) = psi->XBuf[l+HF_ADJ][k][1];
        }
    }
#endif    
    
    /* perform parametric stereo */
#ifdef PS_DEC
    ps_decode(psi->ps, X_left, X_right);
#endif

    /* subband synthesis */
    if (downSampledSBR)
    {
        //sbr_qmf_synthesis_32(sbr, sbr->qmfs[0], X_left, left_channel);
        //sbr_qmf_synthesis_32(sbr, sbr->qmfs[1], X_right, right_channel);
    } else {
        //sbr_qmf_synthesis_64(sbr, sbr->qmfs[0], X_left, left_channel);
        //sbr_qmf_synthesis_64(sbr, sbr->qmfs[1], X_right, right_channel);
    }
#ifndef FAAD_DECODE  
    qmfsBands = sbrFreq->kStartPrev + sbrFreq->numQMFBandsPrev;
    ptLeftChannel = &outbuf[0];
    ptRightChannel = &outbuf[1];
    for (l = 0; l < sbrGrid->envTimeBorder[0]; l++) 
    {
         /* if new envelope starts mid-frame, use old settings until start of first envelope in this frame */
         QMFSynthesis((int*)X_left[l][0], psi->delayQMFS[0], &(psi->delayIdxQMFS[0]), qmfsBands, outbuf, aacDecInfo->nChans);
         QMFSynthesis((int*)X_right[l][0], psi->delayQMFSPS[0], &(psi->delayIdxQMFS[1]), qmfsBands, ptRightChannel, aacDecInfo->nChans);
         outbuf += 64*aacDecInfo->nChans;
         ptRightChannel = &outbuf[1];
    }

    qmfsBands = sbrFreq->kStart + sbrFreq->numQMFBands;
    ptLeftChannel = &outbuf[0];
    ptRightChannel = &outbuf[1];
    for ( ; l < 32; l++)
    {
        /* use new settings for rest of frame (usually the entire frame, unless the first envelope starts mid-frame) */        
        QMFSynthesis((int*)X_left[l][0], psi->delayQMFS[0], &(psi->delayIdxQMFS[0]), qmfsBands, outbuf, aacDecInfo->nChans);
        QMFSynthesis((int*)X_right[l][0], psi->delayQMFSPS[0], &(psi->delayIdxQMFS[1]), qmfsBands, ptRightChannel, aacDecInfo->nChans);
        outbuf += 64*aacDecInfo->nChans;
        ptRightChannel = &outbuf[1];
    }


#else
    ptLeftChannel = &outbuf[0];
    ptRightChannel = &outbuf[1];
    sbr_qmf_synthesis_64(psi->delayQMFS[0], &(psi->delayIdxQMFS[0]),X_left,ptLeftChannel);
    sbr_qmf_synthesis_64(psi->delayQMFSPS[0], &(psi->delayIdxQMFS[1]),X_right,ptRightChannel);
#endif
    //if (sbr->bs_header_flag)
    //    sbr->just_seeked = 0;

    //if (sbr->header_count != 0 && sbr->ret == 0)
    //{
       //  ret = sbr_save_prev_data(sbr, 0);
       //  if (ret) return ret;
    //}

    //sbr_save_matrix(sbr, 0);
#if 1
    /* save delay */
    for (l = 0; l < HF_GEN; l++) 
    {
        for (k = 0; k < 64; k++) 
        {
            psi->XBufDelay[0][l][k][0] = psi->XBuf[l+32][k][0];
            psi->XBufDelay[0][l][k][1] = psi->XBuf[l+32][k][1];
        }
    }
    sbrFreq->kStartPrev = sbrFreq->kStart;
    sbrFreq->numQMFBandsPrev = sbrFreq->numQMFBands;
#endif 

    return 0;
}


#ifdef FAAD_DECODE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ALIGN static const real_t qmf_c[640] = {
    FRAC_CONST(0), FRAC_CONST(-0.00055252865047),
    FRAC_CONST(-0.00056176925738), FRAC_CONST(-0.00049475180896),
    FRAC_CONST(-0.00048752279712), FRAC_CONST(-0.00048937912498),
    FRAC_CONST(-0.00050407143497), FRAC_CONST(-0.00052265642972),
    FRAC_CONST(-0.00054665656337), FRAC_CONST(-0.00056778025613),
    FRAC_CONST(-0.00058709304852), FRAC_CONST(-0.00061327473938),
    FRAC_CONST(-0.00063124935319), FRAC_CONST(-0.00065403333621),
    FRAC_CONST(-0.00067776907764), FRAC_CONST(-0.00069416146273),
    FRAC_CONST(-0.00071577364744), FRAC_CONST(-0.00072550431222),
    FRAC_CONST(-0.00074409418541), FRAC_CONST(-0.00074905980532),
    FRAC_CONST(-0.0007681371927), FRAC_CONST(-0.00077248485949),
    FRAC_CONST(-0.00078343322877), FRAC_CONST(-0.00077798694927),
    FRAC_CONST(-0.000780366471), FRAC_CONST(-0.00078014496257),
    FRAC_CONST(-0.0007757977331), FRAC_CONST(-0.00076307935757),
    FRAC_CONST(-0.00075300014201), FRAC_CONST(-0.00073193571525),
    FRAC_CONST(-0.00072153919876), FRAC_CONST(-0.00069179375372),
    FRAC_CONST(-0.00066504150893), FRAC_CONST(-0.00063415949025),
    FRAC_CONST(-0.0005946118933), FRAC_CONST(-0.00055645763906),
    FRAC_CONST(-0.00051455722108), FRAC_CONST(-0.00046063254803),
    FRAC_CONST(-0.00040951214522), FRAC_CONST(-0.00035011758756),
    FRAC_CONST(-0.00028969811748), FRAC_CONST(-0.0002098337344),
    FRAC_CONST(-0.00014463809349), FRAC_CONST(-6.173344072E-005),
    FRAC_CONST(1.349497418E-005), FRAC_CONST(0.00010943831274),
    FRAC_CONST(0.00020430170688), FRAC_CONST(0.00029495311041),
    FRAC_CONST(0.0004026540216), FRAC_CONST(0.00051073884952),
    FRAC_CONST(0.00062393761391), FRAC_CONST(0.00074580258865),
    FRAC_CONST(0.00086084433262), FRAC_CONST(0.00098859883015),
    FRAC_CONST(0.00112501551307), FRAC_CONST(0.00125778846475),
    FRAC_CONST(0.00139024948272), FRAC_CONST(0.00154432198471),
    FRAC_CONST(0.00168680832531), FRAC_CONST(0.00183482654224),
    FRAC_CONST(0.00198411407369), FRAC_CONST(0.00214615835557),
    FRAC_CONST(0.00230172547746), FRAC_CONST(0.00246256169126),
    FRAC_CONST(0.00262017586902), FRAC_CONST(0.00278704643465),
    FRAC_CONST(0.00294694477165), FRAC_CONST(0.00311254206525),
    FRAC_CONST(0.00327396134847), FRAC_CONST(0.00344188741828),
    FRAC_CONST(0.00360082681231), FRAC_CONST(0.00376039229104),
    FRAC_CONST(0.00392074323703), FRAC_CONST(0.00408197531935),
    FRAC_CONST(0.0042264269227), FRAC_CONST(0.00437307196781),
    FRAC_CONST(0.00452098527825), FRAC_CONST(0.00466064606118),
    FRAC_CONST(0.00479325608498), FRAC_CONST(0.00491376035745),
    FRAC_CONST(0.00503930226013), FRAC_CONST(0.00514073539032),
    FRAC_CONST(0.00524611661324), FRAC_CONST(0.00534716811982),
    FRAC_CONST(0.00541967759307), FRAC_CONST(0.00548760401507),
    FRAC_CONST(0.00554757145088), FRAC_CONST(0.00559380230045),
    FRAC_CONST(0.00562206432097), FRAC_CONST(0.00564551969164),
    FRAC_CONST(0.00563891995151), FRAC_CONST(0.00562661141932),
    FRAC_CONST(0.0055917128663), FRAC_CONST(0.005540436394),
    FRAC_CONST(0.0054753783077), FRAC_CONST(0.0053838975897),
    FRAC_CONST(0.00527157587272), FRAC_CONST(0.00513822754514),
    FRAC_CONST(0.00498396877629), FRAC_CONST(0.004810946906),
    FRAC_CONST(0.00460395301471), FRAC_CONST(0.00438018617447),
    FRAC_CONST(0.0041251642327), FRAC_CONST(0.00384564081246),
    FRAC_CONST(0.00354012465507), FRAC_CONST(0.00320918858098),
    FRAC_CONST(0.00284467578623), FRAC_CONST(0.00245085400321),
    FRAC_CONST(0.0020274176185), FRAC_CONST(0.00157846825768),
    FRAC_CONST(0.00109023290512), FRAC_CONST(0.0005832264248),
    FRAC_CONST(2.760451905E-005), FRAC_CONST(-0.00054642808664),
    FRAC_CONST(-0.00115681355227), FRAC_CONST(-0.00180394725893),
    FRAC_CONST(-0.00248267236449), FRAC_CONST(-0.003193377839),
    FRAC_CONST(-0.00394011240522), FRAC_CONST(-0.004722259624),
    FRAC_CONST(-0.00553372111088), FRAC_CONST(-0.00637922932685),
    FRAC_CONST(-0.00726158168517), FRAC_CONST(-0.00817982333726),
    FRAC_CONST(-0.00913253296085), FRAC_CONST(-0.01011502154986),
    FRAC_CONST(-0.01113155480321), FRAC_CONST(-0.01218499959508),
    FRAC_CONST(0.01327182200351), FRAC_CONST(0.01439046660792),
    FRAC_CONST(0.01554055533423), FRAC_CONST(0.01673247129989),
    FRAC_CONST(0.01794333813443), FRAC_CONST(0.01918724313698),
    FRAC_CONST(0.02045317933555), FRAC_CONST(0.02174675502535),
    FRAC_CONST(0.02306801692862), FRAC_CONST(0.02441609920285),
    FRAC_CONST(0.02578758475467), FRAC_CONST(0.02718594296329),
    FRAC_CONST(0.02860721736385), FRAC_CONST(0.03005026574279),
    FRAC_CONST(0.03150176087389), FRAC_CONST(0.03297540810337),
    FRAC_CONST(0.03446209487686), FRAC_CONST(0.03596975605542),
    FRAC_CONST(0.03748128504252), FRAC_CONST(0.03900536794745),
    FRAC_CONST(0.04053491705584), FRAC_CONST(0.04206490946367),
    FRAC_CONST(0.04360975421304), FRAC_CONST(0.04514884056413),
    FRAC_CONST(0.04668430272642), FRAC_CONST(0.04821657200672),
    FRAC_CONST(0.04973857556014), FRAC_CONST(0.05125561555216),
    FRAC_CONST(0.05276307465207), FRAC_CONST(0.05424527683589),
    FRAC_CONST(0.05571736482138), FRAC_CONST(0.05716164501299),
    FRAC_CONST(0.0585915683626), FRAC_CONST(0.05998374801761),
    FRAC_CONST(0.06134551717207), FRAC_CONST(0.06268578081172),
    FRAC_CONST(0.06397158980681), FRAC_CONST(0.0652247106438),
    FRAC_CONST(0.06643675122104), FRAC_CONST(0.06760759851228),
    FRAC_CONST(0.06870438283512), FRAC_CONST(0.06976302447127),
    FRAC_CONST(0.07076287107266), FRAC_CONST(0.07170026731102),
    FRAC_CONST(0.07256825833083), FRAC_CONST(0.07336202550803),
    FRAC_CONST(0.07410036424342), FRAC_CONST(0.07474525581194),
    FRAC_CONST(0.07531373362019), FRAC_CONST(0.07580083586584),
    FRAC_CONST(0.07619924793396), FRAC_CONST(0.07649921704119),
    FRAC_CONST(0.07670934904245), FRAC_CONST(0.07681739756964),
    FRAC_CONST(0.07682300113923), FRAC_CONST(0.07672049241746),
    FRAC_CONST(0.07650507183194), FRAC_CONST(0.07617483218536),
    FRAC_CONST(0.07573057565061), FRAC_CONST(0.0751576255287),
    FRAC_CONST(0.07446643947564), FRAC_CONST(0.0736406005762),
    FRAC_CONST(0.07267746427299), FRAC_CONST(0.07158263647903),
    FRAC_CONST(0.07035330735093), FRAC_CONST(0.06896640131951),
    FRAC_CONST(0.06745250215166), FRAC_CONST(0.06576906686508),
    FRAC_CONST(0.06394448059633), FRAC_CONST(0.06196027790387),
    FRAC_CONST(0.0598166570809), FRAC_CONST(0.05751526919867),
    FRAC_CONST(0.05504600343009), FRAC_CONST(0.05240938217366),
    FRAC_CONST(0.04959786763445), FRAC_CONST(0.04663033051701),
    FRAC_CONST(0.04347687821958), FRAC_CONST(0.04014582784127),
    FRAC_CONST(0.03664181168133), FRAC_CONST(0.03295839306691),
    FRAC_CONST(0.02908240060125), FRAC_CONST(0.02503075618909),
    FRAC_CONST(0.02079970728622), FRAC_CONST(0.01637012582228),
    FRAC_CONST(0.01176238327857), FRAC_CONST(0.00696368621617),
    FRAC_CONST(0.00197656014503), FRAC_CONST(-0.00320868968304),
    FRAC_CONST(-0.00857117491366), FRAC_CONST(-0.01412888273558),
    FRAC_CONST(-0.01988341292573), FRAC_CONST(-0.02582272888064),
    FRAC_CONST(-0.03195312745332), FRAC_CONST(-0.03827765720822),
    FRAC_CONST(-0.04478068215856), FRAC_CONST(-0.05148041767934),
    FRAC_CONST(-0.05837053268336), FRAC_CONST(-0.06544098531359),
    FRAC_CONST(-0.07269433008129), FRAC_CONST(-0.08013729344279),
    FRAC_CONST(-0.08775475365593), FRAC_CONST(-0.09555333528914),
    FRAC_CONST(-0.10353295311463), FRAC_CONST(-0.1116826931773),
    FRAC_CONST(-0.120007798468), FRAC_CONST(-0.12850028503878),
    FRAC_CONST(-0.13715517611934), FRAC_CONST(-0.1459766491187),
    FRAC_CONST(-0.15496070710605), FRAC_CONST(-0.16409588556669),
    FRAC_CONST(-0.17338081721706), FRAC_CONST(-0.18281725485142),
    FRAC_CONST(-0.19239667457267), FRAC_CONST(-0.20212501768103),
    FRAC_CONST(-0.21197358538056), FRAC_CONST(-0.22196526964149),
    FRAC_CONST(-0.23206908706791), FRAC_CONST(-0.24230168845974),
    FRAC_CONST(-0.25264803095722), FRAC_CONST(-0.26310532994603),
    FRAC_CONST(-0.27366340405625), FRAC_CONST(-0.28432141891085),
    FRAC_CONST(-0.29507167170646), FRAC_CONST(-0.30590985751916),
    FRAC_CONST(-0.31682789136456), FRAC_CONST(-0.32781137272105),
    FRAC_CONST(-0.33887226938665), FRAC_CONST(-0.3499914122931),
    FRAC_CONST(0.36115899031355), FRAC_CONST(0.37237955463061),
    FRAC_CONST(0.38363500139043), FRAC_CONST(0.39492117615675),
    FRAC_CONST(0.40623176767625), FRAC_CONST(0.41756968968409),
    FRAC_CONST(0.42891199207373), FRAC_CONST(0.44025537543665),
    FRAC_CONST(0.45159965356824), FRAC_CONST(0.46293080852757),
    FRAC_CONST(0.47424532146115), FRAC_CONST(0.48552530911099),
    FRAC_CONST(0.49677082545707), FRAC_CONST(0.50798175000434),
    FRAC_CONST(0.51912349702391), FRAC_CONST(0.53022408956855),
    FRAC_CONST(0.54125534487322), FRAC_CONST(0.55220512585061),
    FRAC_CONST(0.5630789140137), FRAC_CONST(0.57385241316923),
    FRAC_CONST(0.58454032354679), FRAC_CONST(0.59511230862496),
    FRAC_CONST(0.6055783538918), FRAC_CONST(0.61591099320291),
    FRAC_CONST(0.62612426956055), FRAC_CONST(0.63619801077286),
    FRAC_CONST(0.64612696959461), FRAC_CONST(0.65590163024671),
    FRAC_CONST(0.66551398801627), FRAC_CONST(0.67496631901712),
    FRAC_CONST(0.68423532934598), FRAC_CONST(0.69332823767032),
    FRAC_CONST(0.70223887193539), FRAC_CONST(0.71094104263095),
    FRAC_CONST(0.71944626349561), FRAC_CONST(0.72774489002994),
    FRAC_CONST(0.73582117582769), FRAC_CONST(0.74368278636488),
    FRAC_CONST(0.75131374561237), FRAC_CONST(0.75870807608242),
    FRAC_CONST(0.76586748650939), FRAC_CONST(0.77277808813327),
    FRAC_CONST(0.77942875190216), FRAC_CONST(0.7858353120392),
    FRAC_CONST(0.79197358416424), FRAC_CONST(0.797846641377),
    FRAC_CONST(0.80344857518505), FRAC_CONST(0.80876950044491),
    FRAC_CONST(0.81381912706217), FRAC_CONST(0.81857760046468),
    FRAC_CONST(0.82304198905409), FRAC_CONST(0.8272275347336),
    FRAC_CONST(0.8311038457152), FRAC_CONST(0.83469373618402),
    FRAC_CONST(0.83797173378865), FRAC_CONST(0.84095413924722),
    FRAC_CONST(0.84362382812005), FRAC_CONST(0.84598184698206),
    FRAC_CONST(0.84803157770763), FRAC_CONST(0.84978051984268),
    FRAC_CONST(0.85119715249343), FRAC_CONST(0.85230470352147),
    FRAC_CONST(0.85310209497017), FRAC_CONST(0.85357205739107),
    FRAC_CONST(0.85373856005937 /*max*/), FRAC_CONST(0.85357205739107),
    FRAC_CONST(0.85310209497017), FRAC_CONST(0.85230470352147),
    FRAC_CONST(0.85119715249343), FRAC_CONST(0.84978051984268),
    FRAC_CONST(0.84803157770763), FRAC_CONST(0.84598184698206),
    FRAC_CONST(0.84362382812005), FRAC_CONST(0.84095413924722),
    FRAC_CONST(0.83797173378865), FRAC_CONST(0.83469373618402),
    FRAC_CONST(0.8311038457152), FRAC_CONST(0.8272275347336),
    FRAC_CONST(0.82304198905409), FRAC_CONST(0.81857760046468),
    FRAC_CONST(0.81381912706217), FRAC_CONST(0.80876950044491),
    FRAC_CONST(0.80344857518505), FRAC_CONST(0.797846641377),
    FRAC_CONST(0.79197358416424), FRAC_CONST(0.7858353120392),
    FRAC_CONST(0.77942875190216), FRAC_CONST(0.77277808813327),
    FRAC_CONST(0.76586748650939), FRAC_CONST(0.75870807608242),
    FRAC_CONST(0.75131374561237), FRAC_CONST(0.74368278636488),
    FRAC_CONST(0.73582117582769), FRAC_CONST(0.72774489002994),
    FRAC_CONST(0.71944626349561), FRAC_CONST(0.71094104263095),
    FRAC_CONST(0.70223887193539), FRAC_CONST(0.69332823767032),
    FRAC_CONST(0.68423532934598), FRAC_CONST(0.67496631901712),
    FRAC_CONST(0.66551398801627), FRAC_CONST(0.65590163024671),
    FRAC_CONST(0.64612696959461), FRAC_CONST(0.63619801077286),
    FRAC_CONST(0.62612426956055), FRAC_CONST(0.61591099320291),
    FRAC_CONST(0.6055783538918), FRAC_CONST(0.59511230862496),
    FRAC_CONST(0.58454032354679), FRAC_CONST(0.57385241316923),
    FRAC_CONST(0.5630789140137), FRAC_CONST(0.55220512585061),
    FRAC_CONST(0.54125534487322), FRAC_CONST(0.53022408956855),
    FRAC_CONST(0.51912349702391), FRAC_CONST(0.50798175000434),
    FRAC_CONST(0.49677082545707), FRAC_CONST(0.48552530911099),
    FRAC_CONST(0.47424532146115), FRAC_CONST(0.46293080852757),
    FRAC_CONST(0.45159965356824), FRAC_CONST(0.44025537543665),
    FRAC_CONST(0.42891199207373), FRAC_CONST(0.41756968968409),
    FRAC_CONST(0.40623176767625), FRAC_CONST(0.39492117615675),
    FRAC_CONST(0.38363500139043), FRAC_CONST(0.37237955463061),
    FRAC_CONST(-0.36115899031355), FRAC_CONST(-0.3499914122931),
    FRAC_CONST(-0.33887226938665), FRAC_CONST(-0.32781137272105),
    FRAC_CONST(-0.31682789136456), FRAC_CONST(-0.30590985751916),
    FRAC_CONST(-0.29507167170646), FRAC_CONST(-0.28432141891085),
    FRAC_CONST(-0.27366340405625), FRAC_CONST(-0.26310532994603),
    FRAC_CONST(-0.25264803095722), FRAC_CONST(-0.24230168845974),
    FRAC_CONST(-0.23206908706791), FRAC_CONST(-0.22196526964149),
    FRAC_CONST(-0.21197358538056), FRAC_CONST(-0.20212501768103),
    FRAC_CONST(-0.19239667457267), FRAC_CONST(-0.18281725485142),
    FRAC_CONST(-0.17338081721706), FRAC_CONST(-0.16409588556669),
    FRAC_CONST(-0.15496070710605), FRAC_CONST(-0.1459766491187),
    FRAC_CONST(-0.13715517611934), FRAC_CONST(-0.12850028503878),
    FRAC_CONST(-0.120007798468), FRAC_CONST(-0.1116826931773),
    FRAC_CONST(-0.10353295311463), FRAC_CONST(-0.09555333528914),
    FRAC_CONST(-0.08775475365593), FRAC_CONST(-0.08013729344279),
    FRAC_CONST(-0.07269433008129), FRAC_CONST(-0.06544098531359),
    FRAC_CONST(-0.05837053268336), FRAC_CONST(-0.05148041767934),
    FRAC_CONST(-0.04478068215856), FRAC_CONST(-0.03827765720822),
    FRAC_CONST(-0.03195312745332), FRAC_CONST(-0.02582272888064),
    FRAC_CONST(-0.01988341292573), FRAC_CONST(-0.01412888273558),
    FRAC_CONST(-0.00857117491366), FRAC_CONST(-0.00320868968304),
    FRAC_CONST(0.00197656014503), FRAC_CONST(0.00696368621617),
    FRAC_CONST(0.01176238327857), FRAC_CONST(0.01637012582228),
    FRAC_CONST(0.02079970728622), FRAC_CONST(0.02503075618909),
    FRAC_CONST(0.02908240060125), FRAC_CONST(0.03295839306691),
    FRAC_CONST(0.03664181168133), FRAC_CONST(0.04014582784127),
    FRAC_CONST(0.04347687821958), FRAC_CONST(0.04663033051701),
    FRAC_CONST(0.04959786763445), FRAC_CONST(0.05240938217366),
    FRAC_CONST(0.05504600343009), FRAC_CONST(0.05751526919867),
    FRAC_CONST(0.0598166570809), FRAC_CONST(0.06196027790387),
    FRAC_CONST(0.06394448059633), FRAC_CONST(0.06576906686508),
    FRAC_CONST(0.06745250215166), FRAC_CONST(0.06896640131951),
    FRAC_CONST(0.07035330735093), FRAC_CONST(0.07158263647903),
    FRAC_CONST(0.07267746427299), FRAC_CONST(0.0736406005762),
    FRAC_CONST(0.07446643947564), FRAC_CONST(0.0751576255287),
    FRAC_CONST(0.07573057565061), FRAC_CONST(0.07617483218536),
    FRAC_CONST(0.07650507183194), FRAC_CONST(0.07672049241746),
    FRAC_CONST(0.07682300113923), FRAC_CONST(0.07681739756964),
    FRAC_CONST(0.07670934904245), FRAC_CONST(0.07649921704119),
    FRAC_CONST(0.07619924793396), FRAC_CONST(0.07580083586584),
    FRAC_CONST(0.07531373362019), FRAC_CONST(0.07474525581194),
    FRAC_CONST(0.07410036424342), FRAC_CONST(0.07336202550803),
    FRAC_CONST(0.07256825833083), FRAC_CONST(0.07170026731102),
    FRAC_CONST(0.07076287107266), FRAC_CONST(0.06976302447127),
    FRAC_CONST(0.06870438283512), FRAC_CONST(0.06760759851228),
    FRAC_CONST(0.06643675122104), FRAC_CONST(0.0652247106438),
    FRAC_CONST(0.06397158980681), FRAC_CONST(0.06268578081172),
    FRAC_CONST(0.06134551717207), FRAC_CONST(0.05998374801761),
    FRAC_CONST(0.0585915683626), FRAC_CONST(0.05716164501299),
    FRAC_CONST(0.05571736482138), FRAC_CONST(0.05424527683589),
    FRAC_CONST(0.05276307465207), FRAC_CONST(0.05125561555216),
    FRAC_CONST(0.04973857556014), FRAC_CONST(0.04821657200672),
    FRAC_CONST(0.04668430272642), FRAC_CONST(0.04514884056413),
    FRAC_CONST(0.04360975421304), FRAC_CONST(0.04206490946367),
    FRAC_CONST(0.04053491705584), FRAC_CONST(0.03900536794745),
    FRAC_CONST(0.03748128504252), FRAC_CONST(0.03596975605542),
    FRAC_CONST(0.03446209487686), FRAC_CONST(0.03297540810337),
    FRAC_CONST(0.03150176087389), FRAC_CONST(0.03005026574279),
    FRAC_CONST(0.02860721736385), FRAC_CONST(0.02718594296329),
    FRAC_CONST(0.02578758475467), FRAC_CONST(0.02441609920285),
    FRAC_CONST(0.02306801692862), FRAC_CONST(0.02174675502535),
    FRAC_CONST(0.02045317933555), FRAC_CONST(0.01918724313698),
    FRAC_CONST(0.01794333813443), FRAC_CONST(0.01673247129989),
    FRAC_CONST(0.01554055533423), FRAC_CONST(0.01439046660792),
    FRAC_CONST(-0.01327182200351), FRAC_CONST(-0.01218499959508),
    FRAC_CONST(-0.01113155480321), FRAC_CONST(-0.01011502154986),
    FRAC_CONST(-0.00913253296085), FRAC_CONST(-0.00817982333726),
    FRAC_CONST(-0.00726158168517), FRAC_CONST(-0.00637922932685),
    FRAC_CONST(-0.00553372111088), FRAC_CONST(-0.004722259624),
    FRAC_CONST(-0.00394011240522), FRAC_CONST(-0.003193377839),
    FRAC_CONST(-0.00248267236449), FRAC_CONST(-0.00180394725893),
    FRAC_CONST(-0.00115681355227), FRAC_CONST(-0.00054642808664),
    FRAC_CONST(2.760451905E-005), FRAC_CONST(0.0005832264248),
    FRAC_CONST(0.00109023290512), FRAC_CONST(0.00157846825768),
    FRAC_CONST(0.0020274176185), FRAC_CONST(0.00245085400321),
    FRAC_CONST(0.00284467578623), FRAC_CONST(0.00320918858098),
    FRAC_CONST(0.00354012465507), FRAC_CONST(0.00384564081246),
    FRAC_CONST(0.0041251642327), FRAC_CONST(0.00438018617447),
    FRAC_CONST(0.00460395301471), FRAC_CONST(0.004810946906),
    FRAC_CONST(0.00498396877629), FRAC_CONST(0.00513822754514),
    FRAC_CONST(0.00527157587272), FRAC_CONST(0.0053838975897),
    FRAC_CONST(0.0054753783077), FRAC_CONST(0.005540436394),
    FRAC_CONST(0.0055917128663), FRAC_CONST(0.00562661141932),
    FRAC_CONST(0.00563891995151), FRAC_CONST(0.00564551969164),
    FRAC_CONST(0.00562206432097), FRAC_CONST(0.00559380230045),
    FRAC_CONST(0.00554757145088), FRAC_CONST(0.00548760401507),
    FRAC_CONST(0.00541967759307), FRAC_CONST(0.00534716811982),
    FRAC_CONST(0.00524611661324), FRAC_CONST(0.00514073539032),
    FRAC_CONST(0.00503930226013), FRAC_CONST(0.00491376035745),
    FRAC_CONST(0.00479325608498), FRAC_CONST(0.00466064606118),
    FRAC_CONST(0.00452098527825), FRAC_CONST(0.00437307196781),
    FRAC_CONST(0.0042264269227), FRAC_CONST(0.00408197531935),
    FRAC_CONST(0.00392074323703), FRAC_CONST(0.00376039229104),
    FRAC_CONST(0.00360082681231), FRAC_CONST(0.00344188741828),
    FRAC_CONST(0.00327396134847), FRAC_CONST(0.00311254206525),
    FRAC_CONST(0.00294694477165), FRAC_CONST(0.00278704643465),
    FRAC_CONST(0.00262017586902), FRAC_CONST(0.00246256169126),
    FRAC_CONST(0.00230172547746), FRAC_CONST(0.00214615835557),
    FRAC_CONST(0.00198411407369), FRAC_CONST(0.00183482654224),
    FRAC_CONST(0.00168680832531), FRAC_CONST(0.00154432198471),
    FRAC_CONST(0.00139024948272), FRAC_CONST(0.00125778846475),
    FRAC_CONST(0.00112501551307), FRAC_CONST(0.00098859883015),
    FRAC_CONST(0.00086084433262), FRAC_CONST(0.00074580258865),
    FRAC_CONST(0.00062393761391), FRAC_CONST(0.00051073884952),
    FRAC_CONST(0.0004026540216), FRAC_CONST(0.00029495311041),
    FRAC_CONST(0.00020430170688), FRAC_CONST(0.00010943831274),
    FRAC_CONST(1.349497418E-005), FRAC_CONST(-6.173344072E-005),
    FRAC_CONST(-0.00014463809349), FRAC_CONST(-0.0002098337344),
    FRAC_CONST(-0.00028969811748), FRAC_CONST(-0.00035011758756),
    FRAC_CONST(-0.00040951214522), FRAC_CONST(-0.00046063254803),
    FRAC_CONST(-0.00051455722108), FRAC_CONST(-0.00055645763906),
    FRAC_CONST(-0.0005946118933), FRAC_CONST(-0.00063415949025),
    FRAC_CONST(-0.00066504150893), FRAC_CONST(-0.00069179375372),
    FRAC_CONST(-0.00072153919876), FRAC_CONST(-0.00073193571525),
    FRAC_CONST(-0.00075300014201), FRAC_CONST(-0.00076307935757),
    FRAC_CONST(-0.0007757977331), FRAC_CONST(-0.00078014496257),
    FRAC_CONST(-0.000780366471), FRAC_CONST(-0.00077798694927),
    FRAC_CONST(-0.00078343322877), FRAC_CONST(-0.00077248485949),
    FRAC_CONST(-0.0007681371927), FRAC_CONST(-0.00074905980532),
    FRAC_CONST(-0.00074409418541), FRAC_CONST(-0.00072550431222),
    FRAC_CONST(-0.00071577364744), FRAC_CONST(-0.00069416146273),
    FRAC_CONST(-0.00067776907764), FRAC_CONST(-0.00065403333621),
    FRAC_CONST(-0.00063124935319), FRAC_CONST(-0.00061327473938),
    FRAC_CONST(-0.00058709304852), FRAC_CONST(-0.00056778025613),
    FRAC_CONST(-0.00054665656337), FRAC_CONST(-0.00052265642972),
    FRAC_CONST(-0.00050407143497), FRAC_CONST(-0.00048937912498),
    FRAC_CONST(-0.00048752279712), FRAC_CONST(-0.00049475180896),
    FRAC_CONST(-0.00056176925738), FRAC_CONST(-0.00055252865047)
};

void sbr_qmf_synthesis_64(int *delay, int *delayIdx, qmf_t X[40][64],
                          short *output)
{
#ifndef SBR_LOW_POWER
    ALIGN real_t in_real1[32], in_imag1[32], out_real1[32], out_imag1[32];
    ALIGN real_t in_real2[32], in_imag2[32], out_real2[32], out_imag2[32];
#endif
    qmf_t * pX;
    real_t * pring_buffer_1, * pring_buffer_3;

    int32_t n, k, out = 0;
    uint8_t l;

    /* qmf subsample l */
    for (l = 0; l < 32; l++)
    {
        /* shift buffer v */
		/* buffer is not shifted, we use double ringbuffer */
		//memmove(qmfs->v + 128, qmfs->v, (1280-128)*sizeof(real_t));

        /* calculate 128 samples */

        pX = X[l];

        in_imag1[31] = QMF_RE(pX[1]) >> 1;
        in_real1[0]  = QMF_RE(pX[0]) >> 1;
        in_imag2[31] = QMF_IM(pX[62]) >> 1;
        in_real2[0]  = QMF_IM(pX[63]) >> 1;
        for (k = 1; k < 31; k++)
        {
            in_imag1[31 - k] = QMF_RE(pX[2*k + 1]) >> 1;
            in_real1[     k] = QMF_RE(pX[2*k    ]) >> 1;
            in_imag2[31 - k] = QMF_IM(pX[63 - (2*k + 1)]) >> 1;
            in_real2[     k] = QMF_IM(pX[63 - (2*k    )]) >> 1;
        }
        in_imag1[0]  = QMF_RE(pX[63]) >> 1;
        in_real1[31] = QMF_RE(pX[62]) >> 1;
        in_imag2[0]  = QMF_IM(pX[0]) >> 1;
        in_real2[31] = QMF_IM(pX[1]) >> 1;

        // dct4_kernel is DCT_IV without reordering which is done before and after FFT
        dct4_kernel(in_real1, in_imag1, out_real1, out_imag1);
        dct4_kernel(in_real2, in_imag2, out_real2, out_imag2);

        pring_buffer_1 = delay + *delayIdx;
        pring_buffer_3 = pring_buffer_1 + 1280;

        for (n = 0; n < 32; n++)
        {
            // pring_buffer_3 and pring_buffer_4 are needed only for double ring buffer
            pring_buffer_1[2*n]         = pring_buffer_3[2*n]         = out_real2[n] - out_real1[n];
            pring_buffer_1[127-2*n]     = pring_buffer_3[127-2*n]     = out_real2[n] + out_real1[n];
            pring_buffer_1[2*n+1]       = pring_buffer_3[2*n+1]       = out_imag2[31-n] + out_imag1[31-n];
            pring_buffer_1[127-(2*n+1)] = pring_buffer_3[127-(2*n+1)] = out_imag2[31-n] - out_imag1[31-n];
        }

        pring_buffer_1 = delay + *delayIdx;

        /* calculate 64 output samples and window */
        for (k = 0; k < 64; k++)
        {
            output[out] =CLIPTOSHORT(
                (
                MUL_F(pring_buffer_1[k+0],          qmf_c[k+0])   +
                MUL_F(pring_buffer_1[k+192],        qmf_c[k+64])  +
                MUL_F(pring_buffer_1[k+256],        qmf_c[k+128]) +
                MUL_F(pring_buffer_1[k+(448)],  qmf_c[k+192]) +
                MUL_F(pring_buffer_1[k+512],        qmf_c[k+256]) +
                MUL_F(pring_buffer_1[k+(704)],  qmf_c[k+320]) +
                MUL_F(pring_buffer_1[k+768],        qmf_c[k+384]) +
                MUL_F(pring_buffer_1[k+(960)],  qmf_c[k+448]) +
                MUL_F(pring_buffer_1[k+1024],       qmf_c[k+512]) +
                MUL_F(pring_buffer_1[k+(1216)], qmf_c[k+576])
                ) >> FBITS_OUT_QMFS
                );
            out+=2;
            
        }
        /* update ringbuffer index */
        *delayIdx -= 128;
        if (*delayIdx < 0)
            *delayIdx = (1152);
    }

}

static const real_t dct4_64_tab[] = {
    COEF_CONST(0.999924719333649), COEF_CONST(0.998118102550507),
    COEF_CONST(0.993906974792480), COEF_CONST(0.987301409244537),
    COEF_CONST(0.978317379951477), COEF_CONST(0.966976463794708),
    COEF_CONST(0.953306019306183), COEF_CONST(0.937339007854462),
    COEF_CONST(0.919113874435425), COEF_CONST(0.898674488067627),
    COEF_CONST(0.876070082187653), COEF_CONST(0.851355195045471),
    COEF_CONST(0.824589252471924), COEF_CONST(0.795836925506592),
    COEF_CONST(0.765167236328125), COEF_CONST(0.732654273509979),
    COEF_CONST(0.698376238346100), COEF_CONST(0.662415742874146),
    COEF_CONST(0.624859452247620), COEF_CONST(0.585797846317291),
    COEF_CONST(0.545324981212616), COEF_CONST(0.503538429737091),
    COEF_CONST(0.460538715124130), COEF_CONST(0.416429549455643),
    COEF_CONST(0.371317148208618), COEF_CONST(0.325310230255127),
    COEF_CONST(0.278519600629807), COEF_CONST(0.231058135628700),
    COEF_CONST(0.183039888739586), COEF_CONST(0.134580686688423),
    COEF_CONST(0.085797272622585), COEF_CONST(0.036807164549828),
    COEF_CONST(-1.012196302413940), COEF_CONST(-1.059438824653626),
    COEF_CONST(-1.104129195213318), COEF_CONST(-1.146159529685974),
    COEF_CONST(-1.185428738594055), COEF_CONST(-1.221842169761658),
    COEF_CONST(-1.255311965942383), COEF_CONST(-1.285757660865784),
    COEF_CONST(-1.313105940818787), COEF_CONST(-1.337290763854981),
    COEF_CONST(-1.358253836631775), COEF_CONST(-1.375944852828980),
    COEF_CONST(-1.390321016311646), COEF_CONST(-1.401347875595093),
    COEF_CONST(-1.408998727798462), COEF_CONST(-1.413255214691162),
    COEF_CONST(-1.414107084274292), COEF_CONST(-1.411552190780640),
    COEF_CONST(-1.405596733093262), COEF_CONST(-1.396255016326904),
    COEF_CONST(-1.383549690246582), COEF_CONST(-1.367511272430420),
    COEF_CONST(-1.348178386688232), COEF_CONST(-1.325597524642944),
    COEF_CONST(-1.299823284149170), COEF_CONST(-1.270917654037476),
    COEF_CONST(-1.238950133323669), COEF_CONST(-1.203998088836670),
    COEF_CONST(-1.166145324707031), COEF_CONST(-1.125483393669128),
    COEF_CONST(-1.082109928131104), COEF_CONST(-1.036129593849182),
    COEF_CONST(-0.987653195858002), COEF_CONST(-0.936797380447388),
    COEF_CONST(-0.883684754371643), COEF_CONST(-0.828443288803101),
    COEF_CONST(-0.771206021308899), COEF_CONST(-0.712110757827759),
    COEF_CONST(-0.651300072669983), COEF_CONST(-0.588920354843140),
    COEF_CONST(-0.525121808052063), COEF_CONST(-0.460058242082596),
    COEF_CONST(-0.393886327743530), COEF_CONST(-0.326765477657318),
    COEF_CONST(-0.258857429027557), COEF_CONST(-0.190325915813446),
    COEF_CONST(-0.121335685253143), COEF_CONST(-0.052053272724152),
    COEF_CONST(0.017354607582092), COEF_CONST(0.086720645427704),
    COEF_CONST(0.155877828598022), COEF_CONST(0.224659323692322),
    COEF_CONST(0.292899727821350), COEF_CONST(0.360434412956238),
    COEF_CONST(0.427100926637650), COEF_CONST(0.492738455533981),
    COEF_CONST(0.557188928127289), COEF_CONST(0.620297133922577),
    COEF_CONST(0.681910991668701), COEF_CONST(0.741881847381592),
    COEF_CONST(0.800065577030182), COEF_CONST(0.856321990489960),
    COEF_CONST(0.910515367984772), COEF_CONST(0.962515234947205),
    COEF_CONST(1.000000000000000), COEF_CONST(0.998795449733734),
    COEF_CONST(0.995184719562531), COEF_CONST(0.989176511764526),
    COEF_CONST(0.980785250663757), COEF_CONST(0.970031261444092),
    COEF_CONST(0.956940352916718), COEF_CONST(0.941544055938721),
    COEF_CONST(0.923879504203796), COEF_CONST(0.903989315032959),
    COEF_CONST(0.881921231746674), COEF_CONST(0.857728600502014),
    COEF_CONST(0.831469595432281), COEF_CONST(0.803207516670227),
    COEF_CONST(0.773010432720184), COEF_CONST(0.740951120853424),
    COEF_CONST(0.707106769084930), COEF_CONST(0.671558916568756),
    COEF_CONST(0.634393274784088), COEF_CONST(0.595699310302734),
    COEF_CONST(0.555570185184479), COEF_CONST(0.514102697372437),
    COEF_CONST(0.471396654844284), COEF_CONST(0.427555114030838),
    COEF_CONST(0.382683426141739), COEF_CONST(0.336889833211899),
    COEF_CONST(0.290284633636475), COEF_CONST(0.242980122566223),
    COEF_CONST(0.195090234279633), COEF_CONST(0.146730497479439),
    COEF_CONST(0.098017133772373), COEF_CONST(0.049067649990320),
    COEF_CONST(-1.000000000000000), COEF_CONST(-1.047863125801086),
    COEF_CONST(-1.093201875686646), COEF_CONST(-1.135906934738159),
    COEF_CONST(-1.175875544548035), COEF_CONST(-1.213011503219605),
    COEF_CONST(-1.247225046157837), COEF_CONST(-1.278433918952942),
    COEF_CONST(-1.306562900543213), COEF_CONST(-1.331544399261475),
    COEF_CONST(-1.353317975997925), COEF_CONST(-1.371831417083740),
    COEF_CONST(-1.387039899826050), COEF_CONST(-1.398906826972961),
    COEF_CONST(-1.407403707504273), COEF_CONST(-1.412510156631470),
    COEF_CONST(0), COEF_CONST(-1.412510156631470),
    COEF_CONST(-1.407403707504273), COEF_CONST(-1.398906826972961),
    COEF_CONST(-1.387039899826050), COEF_CONST(-1.371831417083740),
    COEF_CONST(-1.353317975997925), COEF_CONST(-1.331544399261475),
    COEF_CONST(-1.306562900543213), COEF_CONST(-1.278433918952942),
    COEF_CONST(-1.247225046157837), COEF_CONST(-1.213011384010315),
    COEF_CONST(-1.175875544548035), COEF_CONST(-1.135907053947449),
    COEF_CONST(-1.093201875686646), COEF_CONST(-1.047863125801086),
    COEF_CONST(-1.000000000000000), COEF_CONST(-0.949727773666382),
    COEF_CONST(-0.897167563438416), COEF_CONST(-0.842446029186249),
    COEF_CONST(-0.785694956779480), COEF_CONST(-0.727051079273224),
    COEF_CONST(-0.666655659675598), COEF_CONST(-0.604654192924500),
    COEF_CONST(-0.541196048259735), COEF_CONST(-0.476434230804443),
    COEF_CONST(-0.410524487495422), COEF_CONST(-0.343625843524933),
    COEF_CONST(-0.275899350643158), COEF_CONST(-0.207508206367493),
    COEF_CONST(-0.138617098331451), COEF_CONST(-0.069392144680023),
    COEF_CONST(0), COEF_CONST(0.069392263889313),
    COEF_CONST(0.138617157936096), COEF_CONST(0.207508206367493),
    COEF_CONST(0.275899469852448), COEF_CONST(0.343625962734222),
    COEF_CONST(0.410524636507034), COEF_CONST(0.476434201002121),
    COEF_CONST(0.541196107864380), COEF_CONST(0.604654192924500),
    COEF_CONST(0.666655719280243), COEF_CONST(0.727051138877869),
    COEF_CONST(0.785695075988770), COEF_CONST(0.842446029186249),
    COEF_CONST(0.897167563438416), COEF_CONST(0.949727773666382)
};

/* size 64 only! */
void dct4_kernel(real_t * in_real, real_t * in_imag, real_t * out_real, real_t * out_imag)
{
    // Tables with bit reverse values for 5 bits, bit reverse of i at i-th position
    const uint8_t bit_rev_tab[32] = { 0,16,8,24,4,20,12,28,2,18,10,26,6,22,14,30,1,17,9,25,5,21,13,29,3,19,11,27,7,23,15,31 };
    uint32_t i, i_rev;

    /* Step 2: modulate */
    // 3*32=96 multiplications
    // 3*32=96 additions
    for (i = 0; i < 32; i++)
    {
        real_t x_re, x_im, tmp;
        x_re = in_real[i];
        x_im = in_imag[i];
        tmp = MUL_C(x_re + x_im, dct4_64_tab[i]);
        in_real[i] = MUL_C(x_im, dct4_64_tab[i + 64]) + tmp;
        in_imag[i] = MUL_C(x_re, dct4_64_tab[i + 32]) + tmp;
    }

    /* Step 3: FFT, but with output in bit reverse order */
    fft_dif(in_real, in_imag);

    /* Step 4: modulate + bitreverse reordering */
    // 3*31+2=95 multiplications
    // 3*31+2=95 additions
    for (i = 0; i < 16; i++)
    {
        real_t x_re, x_im, tmp;
        i_rev = bit_rev_tab[i];
        x_re = in_real[i_rev];
        x_im = in_imag[i_rev];

        tmp = MUL_C(x_re + x_im, dct4_64_tab[i + 96]);
        out_real[i] = MUL_C(x_im, dct4_64_tab[i + 160]) + tmp;
        out_imag[i] = MUL_C(x_re, dct4_64_tab[i + 128]) + tmp;
    }
    // i = 16, i_rev = 1 = rev(16);
    out_imag[16] = MUL_C(in_imag[1] - in_real[1], dct4_64_tab[112]);
    out_real[16] = MUL_C(in_real[1] + in_imag[1], dct4_64_tab[112]);
    for (i = 17; i < 32; i++)
    {
        real_t x_re, x_im, tmp;
        i_rev = bit_rev_tab[i];
        x_re = in_real[i_rev];
        x_im = in_imag[i_rev];
        tmp = MUL_C(x_re + x_im, dct4_64_tab[i + 96]);
        out_real[i] = MUL_C(x_im, dct4_64_tab[i + 160]) + tmp;
        out_imag[i] = MUL_C(x_re, dct4_64_tab[i + 128]) + tmp;
    }

}


#define n 32
#define log2n 5

// w_array_real[i] = cos(2*M_PI*i/32)
static const real_t w_array_real[] = {
    FRAC_CONST(1.000000000000000), FRAC_CONST(0.980785279337272),
    FRAC_CONST(0.923879528329380), FRAC_CONST(0.831469603195765),
    FRAC_CONST(0.707106765732237), FRAC_CONST(0.555570210304169),
    FRAC_CONST(0.382683402077046), FRAC_CONST(0.195090284503576),
    FRAC_CONST(0.000000000000000), FRAC_CONST(-0.195090370246552),
    FRAC_CONST(-0.382683482845162), FRAC_CONST(-0.555570282993553),
    FRAC_CONST(-0.707106827549476), FRAC_CONST(-0.831469651765257),
    FRAC_CONST(-0.923879561784627), FRAC_CONST(-0.980785296392607)
};

// w_array_imag[i] = sin(-2*M_PI*i/32)
static const real_t w_array_imag[] = {
    FRAC_CONST(0.000000000000000), FRAC_CONST(-0.195090327375064),
    FRAC_CONST(-0.382683442461104), FRAC_CONST(-0.555570246648862),
    FRAC_CONST(-0.707106796640858), FRAC_CONST(-0.831469627480512),
    FRAC_CONST(-0.923879545057005), FRAC_CONST(-0.980785287864940),
    FRAC_CONST(-1.000000000000000), FRAC_CONST(-0.980785270809601),
    FRAC_CONST(-0.923879511601754), FRAC_CONST(-0.831469578911016),
    FRAC_CONST(-0.707106734823616), FRAC_CONST(-0.555570173959476),
    FRAC_CONST(-0.382683361692986), FRAC_CONST(-0.195090241632088)
};

// FFT decimation in frequency
// 4*16*2+16=128+16=144 multiplications
// 6*16*2+10*8+4*16*2=192+80+128=400 additions
static void fft_dif(real_t * Real, real_t * Imag)
{
    real_t w_real, w_imag; // For faster access
    real_t point1_real, point1_imag, point2_real, point2_imag; // For faster access
    uint32_t j, i, i2, w_index; // Counters

    // First 2 stages of 32 point FFT decimation in frequency
    // 4*16*2=64*2=128 multiplications
    // 6*16*2=96*2=192 additions
	// Stage 1 of 32 point FFT decimation in frequency
    for (i = 0; i < 16; i++)
    {
        point1_real = Real[i];
        point1_imag = Imag[i];
        i2 = i+16;
        point2_real = Real[i2];
        point2_imag = Imag[i2];

        w_real = w_array_real[i];
        w_imag = w_array_imag[i];

        // temp1 = x[i] - x[i2]
        point1_real -= point2_real;
        point1_imag -= point2_imag;

        // x[i1] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = (x[i] - x[i2]) * w
        Real[i2] = (MUL_F(point1_real,w_real) - MUL_F(point1_imag,w_imag));
        Imag[i2] = (MUL_F(point1_real,w_imag) + MUL_F(point1_imag,w_real));
     }
    // Stage 2 of 32 point FFT decimation in frequency
    for (j = 0, w_index = 0; j < 8; j++, w_index += 2)
    {
        w_real = w_array_real[w_index];
        w_imag = w_array_imag[w_index];

    	i = j;
        point1_real = Real[i];
        point1_imag = Imag[i];
        i2 = i+8;
        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // temp1 = x[i] - x[i2]
        point1_real -= point2_real;
        point1_imag -= point2_imag;

        // x[i1] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = (x[i] - x[i2]) * w
        Real[i2] = (MUL_F(point1_real,w_real) - MUL_F(point1_imag,w_imag));
        Imag[i2] = (MUL_F(point1_real,w_imag) + MUL_F(point1_imag,w_real));

        i = j+16;
        point1_real = Real[i];
        point1_imag = Imag[i];
        i2 = i+8;
        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // temp1 = x[i] - x[i2]
        point1_real -= point2_real;
        point1_imag -= point2_imag;

        // x[i1] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = (x[i] - x[i2]) * w
        Real[i2] = (MUL_F(point1_real,w_real) - MUL_F(point1_imag,w_imag));
        Imag[i2] = (MUL_F(point1_real,w_imag) + MUL_F(point1_imag,w_real));
    }

    // Stage 3 of 32 point FFT decimation in frequency
    // 2*4*2=16 multiplications
    // 4*4*2+6*4*2=10*8=80 additions
    for (i = 0; i < n; i += 8)
    {
        i2 = i+4;
        point1_real = Real[i];
        point1_imag = Imag[i];

        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // out[i1] = point1 + point2
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // out[i2] = point1 - point2
        Real[i2] = point1_real - point2_real;
        Imag[i2] = point1_imag - point2_imag;
    }
    w_real = w_array_real[4]; // = sqrt(2)/2
    // w_imag = -w_real; // = w_array_imag[4]; // = -sqrt(2)/2
    for (i = 1; i < n; i += 8)
    {
        i2 = i+4;
        point1_real = Real[i];
        point1_imag = Imag[i];

        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // temp1 = x[i] - x[i2]
        point1_real -= point2_real;
        point1_imag -= point2_imag;

        // x[i1] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = (x[i] - x[i2]) * w
        Real[i2] = MUL_F(point1_real+point1_imag, w_real);
        Imag[i2] = MUL_F(point1_imag-point1_real, w_real);
    }
    for (i = 2; i < n; i += 8)
    {
        i2 = i+4;
        point1_real = Real[i];
        point1_imag = Imag[i];

        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // x[i] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = (x[i] - x[i2]) * (-i)
        Real[i2] = point1_imag - point2_imag;
        Imag[i2] = point2_real - point1_real;
    }
    w_real = w_array_real[12]; // = -sqrt(2)/2
    // w_imag = w_real; // = w_array_imag[12]; // = -sqrt(2)/2
    for (i = 3; i < n; i += 8)
    {
        i2 = i+4;
        point1_real = Real[i];
        point1_imag = Imag[i];

        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // temp1 = x[i] - x[i2]
        point1_real -= point2_real;
        point1_imag -= point2_imag;

        // x[i1] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = (x[i] - x[i2]) * w
        Real[i2] = MUL_F(point1_real-point1_imag, w_real);
        Imag[i2] = MUL_F(point1_real+point1_imag, w_real);
    }


    // Stage 4 of 32 point FFT decimation in frequency (no multiplications)
    // 16*4=64 additions
    for (i = 0; i < n; i += 4)
    {
        i2 = i+2;
        point1_real = Real[i];
        point1_imag = Imag[i];

        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // x[i1] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = x[i] - x[i2]
        Real[i2] = point1_real - point2_real;
        Imag[i2] = point1_imag - point2_imag;
    }
    for (i = 1; i < n; i += 4)
    {
        i2 = i+2;
        point1_real = Real[i];
        point1_imag = Imag[i];

        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // x[i] = x[i] + x[i2]
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // x[i2] = (x[i] - x[i2]) * (-i)
        Real[i2] = point1_imag - point2_imag;
        Imag[i2] = point2_real - point1_real;
    }

    // Stage 5 of 32 point FFT decimation in frequency (no multiplications)
    // 16*4=64 additions
    for (i = 0; i < n; i += 2)
    {
        i2 = i+1;
        point1_real = Real[i];
        point1_imag = Imag[i];

        point2_real = Real[i2];
        point2_imag = Imag[i2];

        // out[i1] = point1 + point2
        Real[i] += point2_real;
        Imag[i] += point2_imag;

        // out[i2] = point1 - point2
        Real[i2] = point1_real - point2_real;
        Imag[i2] = point1_imag - point2_imag;
    }


}
#endif // defined FAAD_DECODE


#endif
