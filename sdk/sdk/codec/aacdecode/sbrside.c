/* ***** BEGIN LICENSE BLOCK *****
 * Source last modified: $Id: sbrside.c,v 1.2 2005/05/24 16:01:55 albertofloyd Exp $
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
 * sbrside.c - functions for unpacking side info from SBR bitstream
 **************************************************************************************/

#include "aacdec.h"

#if defined(ENABLE_CODECS_PLUGIN)
#include "plugin.h"
#endif

#if defined(AAC_ENABLE_SBR)

#include "sbr.h"

#ifdef PARSING_HE_AAC_V2
static unsigned short sbr_extension(BitStreamInfo *bsi, PSInfoSBR *psi,unsigned char bs_extension_id, unsigned short num_bits_left);
#endif

/**************************************************************************************
 * Function:    GetSampRateIdx
 *
 * Description: get index of given sample rate
 *
 * Inputs:      sample rate (in Hz)
 *
 * Outputs:     none
 *
 * Return:      index of sample rate (table 1.15 in 14496-3:2001(E))
 *              -1 if sample rate not found in table
 **************************************************************************************/
int GetSampRateIdx(int sampRate)
{
    int idx;

    for (idx = 0; idx < NUM_SAMPLE_RATES; idx++) {
        if (sampRate == sampRateTab[idx])
            return idx;
    }

    return -1;
}

/**************************************************************************************
 * Function:    UnpackSBRHeader
 *
 * Description: unpack SBR header (table 4.56)
 *
 * Inputs:      BitStreamInfo struct pointing to start of SBR header
 *
 * Outputs:     initialized SBRHeader struct for this SCE/CPE block
 *
 * Return:      non-zero if frame reset is triggered, zero otherwise
 **************************************************************************************/
int UnpackSBRHeader(BitStreamInfo *bsi, SBRHeader *sbrHdr)
{
    SBRHeader sbrHdrPrev;

    /* save previous values so we know whether to reset decoder */
    sbrHdrPrev.startFreq =     sbrHdr->startFreq;
    sbrHdrPrev.stopFreq =      sbrHdr->stopFreq;
    sbrHdrPrev.freqScale =     sbrHdr->freqScale;
    sbrHdrPrev.alterScale =    sbrHdr->alterScale;
    sbrHdrPrev.crossOverBand = sbrHdr->crossOverBand;
    sbrHdrPrev.noiseBands =    sbrHdr->noiseBands;

    sbrHdr->ampRes =        GetBits(bsi, 1);
    sbrHdr->startFreq =     GetBits(bsi, 4);
    sbrHdr->stopFreq =      GetBits(bsi, 4);
    sbrHdr->crossOverBand = GetBits(bsi, 3);
    sbrHdr->resBitsHdr =    GetBits(bsi, 2);
    sbrHdr->hdrExtra1 =     GetBits(bsi, 1);
    sbrHdr->hdrExtra2 =     GetBits(bsi, 1);

    if (sbrHdr->hdrExtra1)
    {
        sbrHdr->freqScale =    GetBits(bsi, 2);
        sbrHdr->alterScale =   GetBits(bsi, 1);
        sbrHdr->noiseBands =   GetBits(bsi, 2);
    }
    else
    {
        /* defaults */
        sbrHdr->freqScale =    2;
        sbrHdr->alterScale =   1;
        sbrHdr->noiseBands =   2;
    }

    if (sbrHdr->hdrExtra2) 
    {
        sbrHdr->limiterBands = GetBits(bsi, 2);
        sbrHdr->limiterGains = GetBits(bsi, 2);
        sbrHdr->interpFreq =   GetBits(bsi, 1);
        sbrHdr->smoothMode =   GetBits(bsi, 1);
    }
    else
    {
        /* defaults */
        sbrHdr->limiterBands = 2;
        sbrHdr->limiterGains = 2;
        sbrHdr->interpFreq =   1;
        sbrHdr->smoothMode =   1;
    }
    sbrHdr->count++;

    /* if any of these have changed from previous frame, reset the SBR module */
    if (sbrHdr->startFreq != sbrHdrPrev.startFreq || sbrHdr->stopFreq != sbrHdrPrev.stopFreq ||
        sbrHdr->freqScale != sbrHdrPrev.freqScale || sbrHdr->alterScale != sbrHdrPrev.alterScale ||
        sbrHdr->crossOverBand != sbrHdrPrev.crossOverBand || sbrHdr->noiseBands != sbrHdrPrev.noiseBands
        )
        return -1;
    else
        return 0;
}

/* cLog2[i] = ceil(log2(i)) (disregard i == 0) */
static const unsigned char cLog2[9] = {0, 0, 1, 2, 2, 3, 3, 3, 3};

/**************************************************************************************
 * Function:    UnpackSBRGrid
 *
 * Description: unpack SBR grid (table 4.62)
 *
 * Inputs:      BitStreamInfo struct pointing to start of SBR grid
 *              initialized SBRHeader struct for this SCE/CPE block
 *
 * Outputs:     initialized SBRGrid struct for this channel
 *
 * Return:      none
 **************************************************************************************/
static int UnpackSBRGrid(BitStreamInfo *bsi, SBRHeader *sbrHdr, SBRGrid *sbrGrid)
{
    int numEnvRaw, env, rel, pBits, border, middleBorder=0;
    unsigned char relBordLead[MAX_NUM_ENV], relBordTrail[MAX_NUM_ENV];
    unsigned char relBorder0[3], relBorder1[3], relBorder[3];
    unsigned char numRelBorder0, numRelBorder1, numRelBorder, numRelLead=0, numRelTrail;
    unsigned char absBordLead=0, absBordTrail=0, absBorder;

    sbrGrid->ampResFrame = sbrHdr->ampRes;
    sbrGrid->frameClass = GetBits(bsi, 2);
    switch (sbrGrid->frameClass) 
    {

    case SBR_GRID_FIXFIX:
        numEnvRaw = GetBits(bsi, 2);
        sbrGrid->numEnv = (1 << numEnvRaw);
        if (sbrGrid->numEnv == 1)
        {
            sbrGrid->ampResFrame = 0;
        }

        ASSERT(sbrGrid->numEnv == 1 || sbrGrid->numEnv == 2 || sbrGrid->numEnv == 4);

        sbrGrid->freqRes[0] = GetBits(bsi, 1);
        for (env = 1; env < sbrGrid->numEnv; env++)
             sbrGrid->freqRes[env] = sbrGrid->freqRes[0];

        absBordLead =  0;
        absBordTrail = NUM_TIME_SLOTS;
        numRelLead =   sbrGrid->numEnv - 1;
        numRelTrail =  0;
        if (numRelLead>=MAX_NUM_ENV)
        {
            return ERR_AAC_SBR_GRID_DATA_ERROR;
        }
        /* numEnv = 1, 2, or 4 */
        if (sbrGrid->numEnv == 1)
        {
            border = NUM_TIME_SLOTS / 1;
        }
        else if (sbrGrid->numEnv == 2)  
        {
            border = NUM_TIME_SLOTS / 2;
        }
        else if (sbrGrid->numEnv == 4)  
        {
            border = NUM_TIME_SLOTS / 4;
        }

        for (rel = 0; rel < numRelLead; rel++)
        {
            relBordLead[rel] = border;
        }

        middleBorder = (sbrGrid->numEnv >> 1);

        break;

    case SBR_GRID_FIXVAR:
        absBorder = GetBits(bsi, 2) + NUM_TIME_SLOTS;
        numRelBorder = GetBits(bsi, 2);
        sbrGrid->numEnv = numRelBorder + 1;
        for (rel = 0; rel < numRelBorder; rel++)
            relBorder[rel] = 2*GetBits(bsi, 2) + 2;

        pBits = cLog2[sbrGrid->numEnv + 1];
        sbrGrid->pointer = GetBits(bsi, pBits);

        for (env = sbrGrid->numEnv - 1; env >= 0; env--)
            sbrGrid->freqRes[env] = GetBits(bsi, 1);

        absBordLead =  0;
        absBordTrail = absBorder;
        numRelLead =   0;
        numRelTrail =  numRelBorder;
        if (numRelTrail>=MAX_NUM_ENV)
        {
            return ERR_AAC_SBR_GRID_DATA_ERROR;
        }
        for (rel = 0; rel < numRelTrail; rel++)
            relBordTrail[rel] = relBorder[rel];

        if (sbrGrid->pointer > 1)           middleBorder = sbrGrid->numEnv + 1 - sbrGrid->pointer;
        else                                middleBorder = sbrGrid->numEnv - 1;

        break;

    case SBR_GRID_VARFIX:
        absBorder = GetBits(bsi, 2);
        numRelBorder = GetBits(bsi, 2);
        sbrGrid->numEnv = numRelBorder + 1;
        for (rel = 0; rel < numRelBorder; rel++)
            relBorder[rel] = 2*GetBits(bsi, 2) + 2;

        pBits = cLog2[sbrGrid->numEnv + 1];
        sbrGrid->pointer = GetBits(bsi, pBits);

        for (env = 0; env < sbrGrid->numEnv; env++)
            sbrGrid->freqRes[env] = GetBits(bsi, 1);

        absBordLead =  absBorder;
        absBordTrail = NUM_TIME_SLOTS;
        numRelLead =   numRelBorder;
        numRelTrail =  0;

        if (numRelLead>=MAX_NUM_ENV)
        {
            return ERR_AAC_SBR_GRID_DATA_ERROR;
        }
        for (rel = 0; rel < numRelLead; rel++)
        {
            relBordLead[rel] = relBorder[rel];
        }

        if (sbrGrid->pointer == 0)          middleBorder = 1;
        else if (sbrGrid->pointer == 1)     middleBorder = sbrGrid->numEnv - 1;
        else                                middleBorder = sbrGrid->pointer - 1;

        break;

    case SBR_GRID_VARVAR:
        absBordLead =   GetBits(bsi, 2);    /* absBorder0 */
        absBordTrail =  GetBits(bsi, 2) + NUM_TIME_SLOTS;   /* absBorder1 */
        numRelBorder0 = GetBits(bsi, 2);
        numRelBorder1 = GetBits(bsi, 2);

        sbrGrid->numEnv = numRelBorder0 + numRelBorder1 + 1;
        ASSERT(sbrGrid->numEnv <= 5);

        for (rel = 0; rel < numRelBorder0; rel++)
            relBorder0[rel] = 2*GetBits(bsi, 2) + 2;

        for (rel = 0; rel < numRelBorder1; rel++)
            relBorder1[rel] = 2*GetBits(bsi, 2) + 2;

        pBits = cLog2[numRelBorder0 + numRelBorder1 + 2];
        sbrGrid->pointer = GetBits(bsi, pBits);

        for (env = 0; env < sbrGrid->numEnv; env++)
            sbrGrid->freqRes[env] = GetBits(bsi, 1);

        numRelLead =  numRelBorder0;
        numRelTrail = numRelBorder1;

        if (numRelLead>=MAX_NUM_ENV)
        {
            return ERR_AAC_SBR_GRID_DATA_ERROR;
        }
        for (rel = 0; rel < numRelLead; rel++)
        {
            relBordLead[rel] = relBorder0[rel];
        }
        if (numRelTrail>=MAX_NUM_ENV)
        {
            return ERR_AAC_SBR_GRID_DATA_ERROR;
        }
        for (rel = 0; rel < numRelTrail; rel++)
        {
            relBordTrail[rel] = relBorder1[rel];
        }

        if (sbrGrid->pointer > 1)           middleBorder = sbrGrid->numEnv + 1 - sbrGrid->pointer;
        else                                middleBorder = sbrGrid->numEnv - 1;

        break;
    }

    /* build time border vector */
    sbrGrid->envTimeBorder[0] = absBordLead * SAMPLES_PER_SLOT;

    rel = 0;
    border = absBordLead;
    if (numRelLead>=MAX_NUM_ENV)
    {
        return ERR_AAC_SBR_GRID_DATA_ERROR;
    }    
    for (env = 1; env <= numRelLead; env++) {
        border += relBordLead[rel++];
        sbrGrid->envTimeBorder[env] = border * SAMPLES_PER_SLOT;
    }

    rel = 0;
    border = absBordTrail;
    for (env = sbrGrid->numEnv - 1; env > numRelLead; env--) 
    {
        border -= relBordTrail[rel++];
        sbrGrid->envTimeBorder[env] = border * SAMPLES_PER_SLOT;
    }

    sbrGrid->envTimeBorder[sbrGrid->numEnv] = absBordTrail * SAMPLES_PER_SLOT;

    if (sbrGrid->numEnv > 1)
    {
        sbrGrid->numNoiseFloors = 2;
        sbrGrid->noiseTimeBorder[0] = sbrGrid->envTimeBorder[0];
        sbrGrid->noiseTimeBorder[1] = sbrGrid->envTimeBorder[middleBorder];
        sbrGrid->noiseTimeBorder[2] = sbrGrid->envTimeBorder[sbrGrid->numEnv];
    }
    else
    {
        sbrGrid->numNoiseFloors = 1;
        sbrGrid->noiseTimeBorder[0] = sbrGrid->envTimeBorder[0];
        sbrGrid->noiseTimeBorder[1] = sbrGrid->envTimeBorder[1];
    }

    return ERR_AAC_NONE;
}

/**************************************************************************************
 * Function:    UnpackDeltaTimeFreq
 *
 * Description: unpack time/freq flags for delta coding of SBR envelopes (table 4.63)
 *
 * Inputs:      BitStreamInfo struct pointing to start of dt/df flags
 *              number of envelopes
 *              number of noise floors
 *
 * Outputs:     delta flags for envelope and noise floors
 *
 * Return:      none
 **************************************************************************************/
static void UnpackDeltaTimeFreq(BitStreamInfo *bsi, int numEnv, unsigned char *deltaFlagEnv,
                                int numNoiseFloors, unsigned char *deltaFlagNoise)
{
    int env, noiseFloor;

    for (env = 0; env < numEnv; env++)
        deltaFlagEnv[env] = GetBits(bsi, 1);

    for (noiseFloor = 0; noiseFloor < numNoiseFloors; noiseFloor++)
        deltaFlagNoise[noiseFloor] = GetBits(bsi, 1);
}

/**************************************************************************************
 * Function:    UnpackInverseFilterMode
 *
 * Description: unpack invf flags for chirp factor calculation (table 4.64)
 *
 * Inputs:      BitStreamInfo struct pointing to start of invf flags
 *              number of noise floor bands
 *
 * Outputs:     invf flags for noise floor bands
 *
 * Return:      none
 **************************************************************************************/
static void UnpackInverseFilterMode(BitStreamInfo *bsi, int numNoiseFloorBands, unsigned char *mode)
{
    int n;

    for (n = 0; n < numNoiseFloorBands; n++)
        mode[n] = GetBits(bsi, 2);
}

/**************************************************************************************
 * Function:    UnpackSinusoids
 *
 * Description: unpack sinusoid (harmonic) flags for each SBR subband (table 4.67)
 *
 * Inputs:      BitStreamInfo struct pointing to start of sinusoid flags
 *              number of high resolution SBR subbands (nHigh)
 *
 * Outputs:     sinusoid flags for each SBR subband, zero-filled above nHigh
 *
 * Return:      none
 **************************************************************************************/
static void UnpackSinusoids(BitStreamInfo *bsi, int nHigh, int addHarmonicFlag, unsigned char *addHarmonic)
{
    int n;

    n = 0;
    if (addHarmonicFlag) {
        for (  ; n < nHigh; n++)
            addHarmonic[n] = GetBits(bsi, 1);
    }

    /* zero out unused bands */
    for (     ; n < MAX_QMF_BANDS; n++)
        addHarmonic[n] = 0;
}

/**************************************************************************************
 * Function:    CopyCouplingGrid
 *
 * Description: copy grid parameters from left to right for channel coupling
 *
 * Inputs:      initialized SBRGrid struct for left channel
 *
 * Outputs:     initialized SBRGrid struct for right channel
 *
 * Return:      none
 **************************************************************************************/
static void CopyCouplingGrid(SBRGrid *sbrGridLeft, SBRGrid *sbrGridRight)
{
    int env, noiseFloor;

    sbrGridRight->frameClass =     sbrGridLeft->frameClass;
    sbrGridRight->ampResFrame =    sbrGridLeft->ampResFrame;
    sbrGridRight->pointer =        sbrGridLeft->pointer;

    sbrGridRight->numEnv =         sbrGridLeft->numEnv;
    for (env = 0; env < sbrGridLeft->numEnv; env++) {
        sbrGridRight->envTimeBorder[env] = sbrGridLeft->envTimeBorder[env];
        sbrGridRight->freqRes[env] =       sbrGridLeft->freqRes[env];
    }
    sbrGridRight->envTimeBorder[env] = sbrGridLeft->envTimeBorder[env]; /* borders are [0, numEnv] inclusive */

    sbrGridRight->numNoiseFloors = sbrGridLeft->numNoiseFloors;
    for (noiseFloor = 0; noiseFloor <= sbrGridLeft->numNoiseFloors; noiseFloor++)
        sbrGridRight->noiseTimeBorder[noiseFloor] = sbrGridLeft->noiseTimeBorder[noiseFloor];

    /* numEnvPrev, numNoiseFloorsPrev, freqResPrev are updated in DecodeSBREnvelope() and DecodeSBRNoise() */
}

/**************************************************************************************
 * Function:    CopyCouplingInverseFilterMode
 *
 * Description: copy invf flags from left to right for channel coupling
 *
 * Inputs:      invf flags for left channel
 *              number of noise floor bands
 *
 * Outputs:     invf flags for right channel
 *
 * Return:      none
 **************************************************************************************/
static void CopyCouplingInverseFilterMode(int numNoiseFloorBands, unsigned char *modeLeft, unsigned char *modeRight)
{
    int band;

    for (band = 0; band < numNoiseFloorBands; band++)
        modeRight[band] = modeLeft[band];
}

/**************************************************************************************
 * Function:    UnpackSBRSingleChannel
 *
 * Description: unpack sideband info (grid, delta flags, invf flags, envelope and
 *                noise floor configuration, sinusoids) for a single channel
 *
 * Inputs:      BitStreamInfo struct pointing to start of sideband info
 *              initialized PSInfoSBR struct (after parsing SBR header and building
 *                frequency tables)
 *              base output channel (range = [0, nChans-1])
 *
 * Outputs:     updated PSInfoSBR struct (SBRGrid and SBRChan)
 *
 * Return:      none
 **************************************************************************************/
int UnpackSBRSingleChannel(BitStreamInfo *bsi, PSInfoSBR *psi, int chBase)
{
    int bitsLeft;
    int nResult;
#ifdef PARSING_HE_AAC_V2
    unsigned short nTemp;
    unsigned char  ps_ext_read = 0;
#endif

    SBRHeader *sbrHdr = &(psi->sbrHdr[chBase]);
    SBRGrid *sbrGridL = &(psi->sbrGrid[chBase+0]);
    SBRFreq *sbrFreq =  &(psi->sbrFreq[chBase]);
    SBRChan *sbrChanL = &(psi->sbrChan[chBase+0]);

    psi->dataExtra = GetBits(bsi, 1);
    if (psi->dataExtra)
        psi->resBitsData = GetBits(bsi, 4);

    nResult=UnpackSBRGrid(bsi, sbrHdr, sbrGridL);
    if (nResult)
    {
        return nResult;
    }
    UnpackDeltaTimeFreq(bsi, sbrGridL->numEnv, sbrChanL->deltaFlagEnv, sbrGridL->numNoiseFloors, sbrChanL->deltaFlagNoise);
    UnpackInverseFilterMode(bsi, sbrFreq->numNoiseFloorBands, sbrChanL->invfMode[1]);

    DecodeSBREnvelope(bsi, psi, sbrGridL, sbrFreq, sbrChanL, 0);
    DecodeSBRNoise(bsi, psi, sbrGridL, sbrFreq, sbrChanL, 0);

    sbrChanL->addHarmonicFlag[1] = GetBits(bsi, 1);
    UnpackSinusoids(bsi, sbrFreq->nHigh, sbrChanL->addHarmonicFlag[1], sbrChanL->addHarmonic[1]);

    psi->extendedDataPresent = GetBits(bsi, 1);
    if (psi->extendedDataPresent) 
    {
        psi->extendedDataSize = GetBits(bsi, 4);
        if (psi->extendedDataSize == 15)
        {
            psi->extendedDataSize += GetBits(bsi, 8);
        }
#ifdef PARSING_HE_AAC_V2
        // parsign HE-AAC V2
        nTemp = 8 * psi->extendedDataSize;
        while (nTemp > 7)
        {        
            unsigned short  tmp_nr_bits = 0;
            psi->nExtensionID = GetBits(bsi, 2);
            tmp_nr_bits += 2;
            if (psi->nExtensionID == EXTENSION_ID_PS)
            {
                if (ps_ext_read == 0)
                {
                    ps_ext_read = 1;
                } 
                else
                {
                    /* to be safe make it 3, will switch to "default"
                     * in sbr_extension() */
                    psi->nExtensionID = 3;
                }
            }
            /* allow only 1 PS extension element per extension data */
            tmp_nr_bits += sbr_extension(bsi,psi, psi->nExtensionID, nTemp);

            /* check if the data read is bigger than the number of available bits */
            if (tmp_nr_bits > nTemp)
                return 1;

            nTemp -= tmp_nr_bits;
        }
        /* Corrigendum */            
        if (nTemp > 0)
        {
            GetBits(bsi, nTemp); 
        }
#else
        bitsLeft = 8 * psi->extendedDataSize;
        /* get ID, unpack extension info, do whatever is necessary with it... */
        while (bitsLeft > 0) 
        {
            GetBits(bsi, 8);
            bitsLeft -= 8;
        }
#endif // ifdef PARSING_HE_AAC_V2
    }
    return 0;
}

/**************************************************************************************
 * Function:    UnpackSBRChannelPair
 *
 * Description: unpack sideband info (grid, delta flags, invf flags, envelope and
 *                noise floor configuration, sinusoids) for a channel pair
 *
 * Inputs:      BitStreamInfo struct pointing to start of sideband info
 *              initialized PSInfoSBR struct (after parsing SBR header and building
 *                frequency tables)
 *              base output channel (range = [0, nChans-1])
 *
 * Outputs:     updated PSInfoSBR struct (SBRGrid and SBRChan for both channels)
 *
 * Return:      none
 **************************************************************************************/
int UnpackSBRChannelPair(BitStreamInfo *bsi, PSInfoSBR *psi, int chBase)
{
    int bitsLeft;
    int nResult;    
    SBRHeader *sbrHdr = &(psi->sbrHdr[chBase]);
    SBRGrid *sbrGridL = &(psi->sbrGrid[chBase+0]), *sbrGridR = &(psi->sbrGrid[chBase+1]);
    SBRFreq *sbrFreq =  &(psi->sbrFreq[chBase]);
    SBRChan *sbrChanL = &(psi->sbrChan[chBase+0]), *sbrChanR = &(psi->sbrChan[chBase+1]);

    psi->dataExtra = GetBits(bsi, 1);
    if (psi->dataExtra) 
    {
        psi->resBitsData = GetBits(bsi, 4);
        psi->resBitsData = GetBits(bsi, 4);
    }

    psi->couplingFlag = GetBits(bsi, 1);
    if (psi->couplingFlag) 
    {
        nResult=UnpackSBRGrid(bsi, sbrHdr, sbrGridL);
        if (nResult)
        {
            return nResult;
        }        
        CopyCouplingGrid(sbrGridL, sbrGridR);

        UnpackDeltaTimeFreq(bsi, sbrGridL->numEnv, sbrChanL->deltaFlagEnv, sbrGridL->numNoiseFloors, sbrChanL->deltaFlagNoise);
        UnpackDeltaTimeFreq(bsi, sbrGridR->numEnv, sbrChanR->deltaFlagEnv, sbrGridR->numNoiseFloors, sbrChanR->deltaFlagNoise);

        UnpackInverseFilterMode(bsi, sbrFreq->numNoiseFloorBands, sbrChanL->invfMode[1]);
        CopyCouplingInverseFilterMode(sbrFreq->numNoiseFloorBands, sbrChanL->invfMode[1], sbrChanR->invfMode[1]);

        DecodeSBREnvelope(bsi, psi, sbrGridL, sbrFreq, sbrChanL, 0);
        DecodeSBRNoise(bsi, psi, sbrGridL, sbrFreq, sbrChanL, 0);
        DecodeSBREnvelope(bsi, psi, sbrGridR, sbrFreq, sbrChanR, 1);
        DecodeSBRNoise(bsi, psi, sbrGridR, sbrFreq, sbrChanR, 1);

        /* pass RIGHT sbrChan struct */
        UncoupleSBREnvelope(psi, sbrGridL, sbrFreq, sbrChanR);
        UncoupleSBRNoise(psi, sbrGridL, sbrFreq, sbrChanR);

    }
    else
    {
        nResult = UnpackSBRGrid(bsi, sbrHdr, sbrGridL);     
        if (nResult)
        {
            return nResult;
        }                
        nResult = UnpackSBRGrid(bsi, sbrHdr, sbrGridR);
        if (nResult)
        {
            return nResult;
        }                        
        UnpackDeltaTimeFreq(bsi, sbrGridL->numEnv, sbrChanL->deltaFlagEnv, sbrGridL->numNoiseFloors, sbrChanL->deltaFlagNoise);
        UnpackDeltaTimeFreq(bsi, sbrGridR->numEnv, sbrChanR->deltaFlagEnv, sbrGridR->numNoiseFloors, sbrChanR->deltaFlagNoise);
        UnpackInverseFilterMode(bsi, sbrFreq->numNoiseFloorBands, sbrChanL->invfMode[1]);
        UnpackInverseFilterMode(bsi, sbrFreq->numNoiseFloorBands, sbrChanR->invfMode[1]);

        DecodeSBREnvelope(bsi, psi, sbrGridL, sbrFreq, sbrChanL, 0);
        DecodeSBREnvelope(bsi, psi, sbrGridR, sbrFreq, sbrChanR, 1);
        DecodeSBRNoise(bsi, psi, sbrGridL, sbrFreq, sbrChanL, 0);
        DecodeSBRNoise(bsi, psi, sbrGridR, sbrFreq, sbrChanR, 1);
    }

    sbrChanL->addHarmonicFlag[1] = GetBits(bsi, 1);
    UnpackSinusoids(bsi, sbrFreq->nHigh, sbrChanL->addHarmonicFlag[1], sbrChanL->addHarmonic[1]);

    sbrChanR->addHarmonicFlag[1] = GetBits(bsi, 1);
    UnpackSinusoids(bsi, sbrFreq->nHigh, sbrChanR->addHarmonicFlag[1], sbrChanR->addHarmonic[1]);

    psi->extendedDataPresent = GetBits(bsi, 1);
    if (psi->extendedDataPresent) {
        psi->extendedDataSize = GetBits(bsi, 4);
        if (psi->extendedDataSize == 15)
            psi->extendedDataSize += GetBits(bsi, 8);

        bitsLeft = 8 * psi->extendedDataSize;

        /* get ID, unpack extension info, do whatever is necessary with it... */
        while (bitsLeft > 0) {
            GetBits(bsi, 8);
            bitsLeft -= 8;
        }
    }
    return nResult;    
}
#endif /* defined(AAC_ENABLE_SBR) */

#ifdef PARSING_HE_AAC_V2
/**************************************************************************************
 * Function:    sbr_extension
 *
 * Description: unpack sbr_extension
 *
 * Inputs:      BitStreamInfo struct pointing to start of sideband info
 *              initialized PSInfoSBR struct (after parsing SBR header and building
 *                frequency tables)
 *              base output channel (range = [0, nChans-1])
 *
 * Outputs:     updated PSInfoSBR struct (SBRGrid and SBRChan)
 *
 * Return:      none
 **************************************************************************************/
static unsigned short sbr_extension(BitStreamInfo *bsi, PSInfoSBR *psi,unsigned char bs_extension_id, unsigned short num_bits_left)
{
    unsigned char header;
    unsigned short ret;
    int numTimeSlotsRate;

    numTimeSlotsRate = SAMPLES_PER_SLOT*NUM_TIME_SLOTS;
    switch (bs_extension_id)
    {
    case EXTENSION_ID_PS:
        if (!psi->ps)
        {
            psi->ps = ps_init(psi->sampRateIdx,numTimeSlotsRate);
        }
        if (psi->psResetFlag)
        {
            psi->ps->header_read = 0;
        }
        ret = ps_data(psi->ps, bsi, &header);

        /* enable PS if and only if: a header has been decoded */
        //if (psi->ps_used == 0 && header == 1)
        if (psi->ps_used == 0)
        {
            psi->ps_used = 1;
        }                
        if (header == 1)
        {
            psi->psResetFlag = 0;
        }
        
        return ret;
    default:
        psi->bs_extension_data = (uint8_t)GetBits(bsi,6);
        
        //sbr->bs_extension_data = (uint8_t)faad_getbits(ld, 6
        //    DEBUGVAR(1,279,"sbr_single_channel_element(): bs_extension_data"));
        return 6;
    }
}

#endif /* ifdef PARSING_HE_AAC_V2 */
