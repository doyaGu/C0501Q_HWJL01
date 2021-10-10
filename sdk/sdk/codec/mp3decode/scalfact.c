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
 * scalfact.c - scalefactor unpacking functions
 **************************************************************************************/

#include "mp3_config.h"

// scale factor lengths (num bits)
static const char SFLenTab[16][2] = {
    {0, 0},    {0, 1},
    {0, 2},    {0, 3},
    {3, 0},    {1, 1},
    {1, 2},    {1, 3},
    {2, 1},    {2, 2},
    {2, 3},    {3, 1},
    {3, 2},    {3, 3},
    {4, 2},    {4, 3},
};

#define NBITS10     10

#if defined(USER_DEFINE_DIV)
// this function is used to compute unsigned integer / and %
static void udivmod10bit(unsigned int dividend, unsigned int divisor, unsigned int *quotient, unsigned int *remainder)
{
    #if defined(WIN32) || defined(__CYGWIN__)
    unsigned int dividendref = dividend,
                divisorref = divisor;
    #endif
    register int counter;
    register unsigned int numzeros, q;
    numzeros = 0;
    while (!(divisor & (1<<(NBITS10-1)) )) {    // assume dividend, divisor is 12bit unsigned integer
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
    *remainder = dividend;

    #if defined(WIN32) || defined(__CYGWIN__)
    if(dividendref / divisorref != *quotient || dividendref % divisorref != *remainder){
        printf("user defined div error: %d / %d = %d ... %d\n",
            dividendref, divisorref, *quotient, *remainder);
    }
    #endif
}
#endif // defined(USER_DEFINE_DIV)

static void UnpackSFMPEG1(BitStreamInfo *bsi, SideInfoSub *sis, ScaleFactorInfoSub *sfis, int *scfsi, int gr, ScaleFactorInfoSub *sfisGr0)
{
    int sfb;
    int slen0, slen1;

    // these can be 0, so make sure GetBits(bsi, 0) returns 0 (no >> 32 or anything)
    slen0 = (int)SFLenTab[sis->sfCompress][0];
    slen1 = (int)SFLenTab[sis->sfCompress][1];

    if (sis->blockType == 2) {
        // short block, type 2 (implies winSwitchFlag == 1)
        if (sis->mixedBlock) {
            // do long block portion
            for (sfb = 0; sfb < 8; sfb++)
                sfis->l[sfb] =    (char)GetBits(bsi, slen0);
            sfb = 3;
        } else {
            // all short blocks
            sfb = 0;
        }

        for (      ; sfb < 6; sfb++) {
            sfis->s[sfb][0] = (char)GetBits(bsi, slen0);
            sfis->s[sfb][1] = (char)GetBits(bsi, slen0);
            sfis->s[sfb][2] = (char)GetBits(bsi, slen0);
        }

        for (      ; sfb < 12; sfb++) {
            sfis->s[sfb][0] = (char)GetBits(bsi, slen1);
            sfis->s[sfb][1] = (char)GetBits(bsi, slen1);
            sfis->s[sfb][2] = (char)GetBits(bsi, slen1);
        }

        // last sf band not transmitted
        sfis->s[12][0] = sfis->s[12][1] = sfis->s[12][2] = 0;
    } else {
        // long blocks, type 0, 1, or 3
        if(gr == 0) {
            // first granule
            for (sfb = 0;  sfb < 11; sfb++)
                sfis->l[sfb] = (char)GetBits(bsi, slen0);
            for (sfb = 11; sfb < 21; sfb++)
                sfis->l[sfb] = (char)GetBits(bsi, slen1);
        } else {
            // second granule
            // scfsi: 0 = different scalefactors for each granule, 1 = copy sf's from granule 0 into granule 1
            // for block type == 2, scfsi is always 0
            //
            sfb = 0;
            if(scfsi[0])  for(  ; sfb < 6 ; sfb++) sfis->l[sfb] = sfisGr0->l[sfb];
            else          for(  ; sfb < 6 ; sfb++) sfis->l[sfb] = (char)GetBits(bsi, slen0);
            if(scfsi[1])  for(  ; sfb <11 ; sfb++) sfis->l[sfb] = sfisGr0->l[sfb];
            else          for(  ; sfb <11 ; sfb++) sfis->l[sfb] = (char)GetBits(bsi, slen0);
            if(scfsi[2])  for(  ; sfb <16 ; sfb++) sfis->l[sfb] = sfisGr0->l[sfb];
            else          for(  ; sfb <16 ; sfb++) sfis->l[sfb] = (char)GetBits(bsi, slen1);
            if(scfsi[3])  for(  ; sfb <21 ; sfb++) sfis->l[sfb] = sfisGr0->l[sfb];
            else          for(  ; sfb <21 ; sfb++) sfis->l[sfb] = (char)GetBits(bsi, slen1);
            // last sf band not transmitted
            sfis->l[21] = 0;
            sfis->l[22] = 0;
        }
    }
    #if defined(STAGE_DEBUG)
    {
        int i;
        fprintf(dbgST_UnpackSF, "nFrames=%d, gr=%d\n", nFrames, gr);
        fprintf(dbgST_UnpackSF, "sfis->l[23]=\n");
        for(sfb=0; sfb<23; sfb++){
            fprintf(dbgST_UnpackSF, "%d,", sfis->l[sfb]);
        }
        fprintf(dbgST_UnpackSF, "\nsfis->s[13][3]=\n");
        for(sfb=0; sfb<13; sfb++){
            for(i=0; i<3; i++){
                fprintf(dbgST_UnpackSF, "%d,", sfis->s[sfb][i]);
            }
        }
        fprintf(dbgST_UnpackSF, "\n\n");
    }
    #endif
}

static const char NRTab[6][3][4] = {
    // non-intensity stereo
    {   {6, 5, 5, 5},
        {3, 3, 3, 3},   // includes / 3
        {6, 3, 3, 3},   // includes / 3 except for first entry
    },
    {   {6, 5, 7, 3},
        {3, 3, 4, 2},
        {6, 3, 4, 2},
    },
    {   {11, 10, 0, 0},
        {6, 6, 0, 0},
        {6, 3, 6, 0},  // spec = [15,18,0,0], but 15 = 6L + 9S, so move 9/3=3 into col 1, 18/3=6 into col 2 and adj. slen[1,2] below
    },
    // intensity stereo, right chan
    {   {7, 7, 7, 0},
        {4, 4, 4, 0},
        {6, 5, 4, 0},
    },
    {   {6, 6, 6, 3},
        {4, 3, 3, 2},
        {6, 4, 3, 2},
    },
    {   {8, 8, 5, 0},
        {5, 4, 3, 0},
        {6, 6, 3, 0},
    }
};

static void UnpackSFMPEG2(BitStreamInfo *bsi, SideInfoSub *sis, ScaleFactorInfoSub *sfis, int gr, int ch, int modeExt, ScaleFactorJS *sfjs)
{
    int i, sfb, sfcIdx, btIdx, nrIdx, iipTest;
    int slen[4], nr[4];
    int sfCompress, preFlag, intensityScale;

    sfCompress = sis->sfCompress;
    preFlag = 0;
    intensityScale = 0;

    // stereo mode bits (1 = on): bit 1 = mid-side on/off, bit 0 = intensity on/off
    if (! ((modeExt & 0x01) && (ch == 1)) ) {
        // in other words: if ((modeExt & 0x01) == 0 || ch == 0)
        if (sfCompress < 400) {
            // max slen = floor[(399/16) / 5] = 4
            #if defined(USER_DEFINE_DIV)
            udivmod10bit((sfCompress >> 4), 5, &slen[0], &slen[1]);
            #else
            slen[0]= (sfCompress >> 4) / 5;
            slen[1]= (sfCompress >> 4) % 5;
            #endif
            slen[2]= (sfCompress & 0x0f) >> 2;
            slen[3]= (sfCompress & 0x03);
            sfcIdx = 0;
        } else if (sfCompress < 500) {
            // max slen = floor[(99/4) / 5] = 4
            sfCompress -= 400;
            #if defined(USER_DEFINE_DIV)
            udivmod10bit((sfCompress >> 2), 5, &slen[0], &slen[1]);
            #else
            slen[0]= (sfCompress >> 2) / 5;
            slen[1]= (sfCompress >> 2) % 5;
            #endif
            slen[2]= (sfCompress & 0x03);
            slen[3]= 0;
            sfcIdx = 1;
        } else {
            // max slen = floor[11/3] = 3 (sfCompress = 9 bits in MPEG2)
            sfCompress -= 500;
            #if defined(USER_DEFINE_DIV)
            udivmod10bit(sfCompress, 3, &slen[0], &slen[1]);
            #else
            slen[0] = sfCompress / 3;
            slen[1] = sfCompress % 3;
            #endif
            slen[2] = slen[3] = 0;
            if (sis->mixedBlock) {
                // adjust for long/short mix logic (see comment above in NRTab[] definition)
                slen[2] = slen[1];
                slen[1] = slen[0];
            }
            preFlag = 1;
            sfcIdx = 2;
        }
    } else {
        // intensity stereo ch = 1 (right)
        intensityScale = sfCompress & 0x01;
        sfCompress >>= 1;
        if (sfCompress < 180) {
            #if defined(USER_DEFINE_DIV)
            int mod36;
            udivmod10bit(sfCompress, 36, &slen[0], &mod36);
            udivmod10bit(mod36,     6, &slen[1], &slen[2]);
            #else
            // max slen = floor[35/6] = 5 (from mod 36)
            slen[0] = (sfCompress / 36);
            slen[1] = (sfCompress % 36) / 6;
            slen[2] = (sfCompress % 36) % 6;
            #endif
            slen[3] = 0;
            sfcIdx = 3;
        } else if (sfCompress < 244) {
            // max slen = floor[63/16] = 3
            sfCompress -= 180;
            slen[0] = (sfCompress & 0x3f) >> 4;
            slen[1] = (sfCompress & 0x0f) >> 2;
            slen[2] = (sfCompress & 0x03);
            slen[3] = 0;
            sfcIdx = 4;
        } else {
            // max slen = floor[11/3] = 3 (max sfCompress >> 1 = 511/2 = 255)
            sfCompress -= 244;
            #if defined(USER_DEFINE_DIV)
            udivmod10bit(sfCompress, 3, &slen[0], &slen[1]);
            #else
            slen[0] = (sfCompress / 3);
            slen[1] = (sfCompress % 3);
            #endif
            slen[2] = slen[3] = 0;
            sfcIdx = 5;
        }
    }

    // set index based on block type: (0,1,3) --> 0, (2 non-mixed) --> 1, (2 mixed) ---> 2
    btIdx = 0;
    if (sis->blockType == 2)
        btIdx = (sis->mixedBlock ? 2 : 1);
    for (i = 0; i < 4; i++)
        nr[i] = (int)NRTab[sfcIdx][btIdx][i];

    // save intensity stereo scale factor info
    if( (modeExt & 0x01) && (ch == 1) ) {
        for (i = 0; i < 4; i++) {
            sfjs->slen[i] = slen[i];
            sfjs->nr[i] = nr[i];
        }
        sfjs->intensityScale = intensityScale;
    }
    sis->preFlag = preFlag;

    // short blocks
    if(sis->blockType == 2) {
        if(sis->mixedBlock) {
            // do long block portion
            iipTest = (1 << slen[0]) - 1;
            for (sfb=0; sfb < 6; sfb++) {
                sfis->l[sfb] = (char)GetBits(bsi, slen[0]);
            }
            sfb = 3;  // start sfb for short
            nrIdx = 1;
        } else {
            // all short blocks, so start nr, sfb at 0
            sfb = 0;
            nrIdx = 0;
        }

        // remaining short blocks, sfb just keeps incrementing
        for (    ; nrIdx <= 3; nrIdx++) {
            iipTest = (1 << slen[nrIdx]) - 1;
            for (i=0; i < nr[nrIdx]; i++, sfb++) {
                sfis->s[sfb][0] = (char)GetBits(bsi, slen[nrIdx]);
                sfis->s[sfb][1] = (char)GetBits(bsi, slen[nrIdx]);
                sfis->s[sfb][2] = (char)GetBits(bsi, slen[nrIdx]);
            }
        }
        // last sf band not transmitted
        sfis->s[12][0] = sfis->s[12][1] = sfis->s[12][2] = 0;
    } else {
        // long blocks
        sfb = 0;
        for (nrIdx = 0; nrIdx <= 3; nrIdx++) {
            iipTest = (1 << slen[nrIdx]) - 1;
            for(i=0; i < nr[nrIdx]; i++, sfb++) {
                sfis->l[sfb] = (char)GetBits(bsi, slen[nrIdx]);
            }
        }
        // last sf band not transmitted
        sfis->l[21] = sfis->l[22] = 0;

    }
    #if defined(STAGE_DEBUG)
    {
        fprintf(dbgST_UnpackSF, "nFrames=%d, gr=%d\n", nFrames, gr);
        fprintf(dbgST_UnpackSF, "sfjs->intensityScale=%d\n", sfjs->intensityScale);
        fprintf(dbgST_UnpackSF, "sfjs->slen[4]=\n");
        for (i = 0; i < 4; i++) {
            fprintf(dbgST_UnpackSF, "%d", sfjs->slen[i]);
        }
        fprintf(dbgST_UnpackSF, "\nsfjs->nr[4]=\n");
        for (i = 0; i < 4; i++) {
            fprintf(dbgST_UnpackSF, "%d", sfjs->nr[i]);
        }
        fprintf(dbgST_UnpackSF, "\nsfis->l[23]=\n");
        for(sfb=0; sfb<23; sfb++){
            fprintf(dbgST_UnpackSF, "%d,", sfis->l[sfb]);
        }
        fprintf(dbgST_UnpackSF, "\nsfis->s[13][3]=\n");
        for(sfb=0; sfb<13; sfb++){
            for(i=0; i<3; i++){
                fprintf(dbgST_UnpackSF, "%d,", sfis->s[sfb][i]);
            }
        }
        fprintf(dbgST_UnpackSF, "\n\n");
    }
    #endif
}

int UnpackScaleFactors(unsigned char *buf, int *bitOffset, int bitsAvail, int gr, int ch)
{
    int bitsUsed;
    unsigned char *startBuf;
    BitStreamInfo bitStreamInfo, *bsi;
    FrameHeader *fh;
    SideInfo *si;
    ScaleFactorInfo *sfi;

    fh = &(mp3DecInfo.FrameHeaderPS);
    si = &(mp3DecInfo.SideInfoPS);
    sfi = &(mp3DecInfo.ScaleFactorInfoPS);

    // init GetBits reader
    startBuf = buf;
    bsi = &bitStreamInfo;
    SetBitstreamPointer(bsi, (bitsAvail + *bitOffset + 7) / 8, buf);
    if (*bitOffset)
        GetBits(bsi, *bitOffset);

    if (fh->ver == MPEG1)
        UnpackSFMPEG1(bsi, &si->sis[gr][ch], &sfi->sfis[gr][ch], si->scfsi[ch], gr, &sfi->sfis[0][ch]);
    else
        UnpackSFMPEG2(bsi, &si->sis[gr][ch], &sfi->sfis[gr][ch], gr, ch, fh->modeExt, &sfi->sfjs[gr]);

    mp3DecInfo.part23Length[gr][ch] = si->sis[gr][ch].part23Length;

    bitsUsed = CalcBitsUsed(bsi, buf, *bitOffset);
    buf += (bitsUsed + *bitOffset) >> 3;
    *bitOffset = (bitsUsed + *bitOffset) & 0x07;

    return (buf - startBuf);
}
