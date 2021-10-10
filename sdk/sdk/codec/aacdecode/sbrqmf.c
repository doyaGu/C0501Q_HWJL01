/* ***** BEGIN LICENSE BLOCK *****
 * Source last modified: $Id: sbrqmf.c,v 1.2 2005/05/19 20:45:20 jrecker Exp $
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
 * sbrqmf.c - analysis and synthesis QMF filters for SBR
 **************************************************************************************/

#include "aacdec.h"

#include "plugin.h"

#if defined(AAC_ENABLE_SBR)

#include "sbr.h"
#include "assembly.h"

/* PreMultiply64() table
 * format = Q30
 * reordered for sequential access
 *
 * for (i = 0; i < 64/4; i++) {
 *   angle = (i + 0.25) * M_PI / nmdct;
 *   x = (cos(angle) + sin(angle));
 *   x =  sin(angle);
 *
 *   angle = (nmdct/2 - 1 - i + 0.25) * M_PI / nmdct;
 *   x = cos(angle);
 *   x =  sin(angle);
 * }
 */
static const int cos4sin4tab64[64] = {
 0x3ffec42d, 0x00c90e90, 0x025b0caf, 0x3ff4e5e0, 0x3fe12acb, 0x03ecadcf, 0x057db403, 0x3fc395f9,
 0x3f9c2bfb, 0x070de172, 0x089cf868, 0x3f6af2e3, 0x3f2ff249, 0x0a2abb59, 0x0bb6ecef, 0x3eeb3347,
 0x3e9cc076, 0x0d415013, 0x0ec9a7f3, 0x3e44a5ef, 0x3de2f148, 0x104fb80e, 0x11d3443f, 0x3d77b192,
 0x3d02f756, 0x135410c3, 0x14d1e243, 0x3c84d496, 0x3bfd5cc5, 0x164c7ddd, 0x17c3a932, 0x3b6ca4c4,
 0x3ad2c2e7, 0x19372a64, 0x1aa6c82c, 0x3a2fcee8, 0x3983e1e8, 0x1c1249d8, 0x1d79775c, 0x38cf1669,
 0x3811884d, 0x1edc1953, 0x2039f90f, 0x374b54ce, 0x367c9a7e, 0x2192e09b, 0x22e69ac8, 0x35a5793c,
 0x34c61236, 0x2434f332, 0x257db64c, 0x33de87de, 0x32eefdea, 0x26c0b162, 0x27fdb2a6, 0x31f79948,
 0x30f88020, 0x29348937, 0x2a650525, 0x2ff1d9c7, 0x2ee3cebe, 0x2b8ef77d, 0x2cb2324c, 0x2dce88aa,
};

/* PostMultiply64() table
 * format = Q30
 * reordered for sequential access
 *
 * for (i = 0; i <= (32/2); i++) {
 *   angle = i * M_PI / 64;
 *   x = (cos(angle) + sin(angle));
 *   x = sin(angle);
 *   x = (cos(angle) - sin(angle));
 * }
 */
static const int cos1sin1tab64[34] = {
 0x40000000, 0x00000000, 0x3fec43c7, 0x0323ecbe, 0x3fb11b48, 0x0645e9af,
 0x3f4eaafe, 0x09640837, 0x3ec52fa0, 0x0c7c5c1e, 0x3e14fdf7, 0x0f8cfcbe,
 0x3d3e82ad, 0x1294062f, 0x3c42420a, 0x158f9a76, 0x3b20d79e, 0x187de2a7,
 0x39daf5e8, 0x1b5d100a, 0x387165e3, 0x1e2b5d38, 0x36e5068a, 0x20e70f32,
 0x3536cc52, 0x238e7673, 0x3367c090, 0x261feffa, 0x317900d6, 0x2899e64a,
 0x2f6bbe45, 0x2afad269, 0x2d413ccd, 0x2d413ccd,
};

static const int anyIdxTab[10][10] = 
{ { 31, 319, 287, 255, 223, 191, 159, 127, 95, 63},
  { 63, 31, 319, 287, 255, 223, 191, 159, 127, 95},
  { 95, 63, 31, 319, 287, 255, 223, 191, 159, 127},
  { 127, 95, 63, 31, 319, 287, 255, 223, 191, 159},
  { 159, 127, 95, 63, 31, 319, 287, 255, 223, 191},
  { 191, 159, 127, 95, 63, 31, 319, 287, 255, 223},
  { 223, 191, 159, 127, 95, 63, 31, 319, 287, 255},
  { 255, 223, 191, 159, 127, 95, 63, 31, 319, 287},
  { 287, 255, 223, 191, 159, 127, 95, 63, 31, 319},
  { 319, 287, 255, 223, 191, 159, 127, 95, 63, 31},
};

static const int synIdxTab0[10][5] = 
{ {    0, 1024,  768,  512,  256},
  {  128, 1152,  896,  640,  384},
  {  256,    0, 1024,  768,  512},
  {  384,  128, 1152,  896,  640},
  {  512,  256,    0, 1024,  768},
  {  640,  384,  128, 1152,  896},
  {  768,  512,  256,    0, 1024},
  {  896,  640,  384,  128, 1152},
  { 1024,  768,  512,  256,    0},
  { 1152,  896,  640,  384,  128} };

static const int synIdxTab1[10][5] = 
{ { 1279, 1023,  767,  511,  255},
  {  127, 1151,  895,  639,  383},
  {  255, 1279, 1023,  767,  511},
  {  383,  127, 1151,  895,  639},
  {  511,  255, 1279, 1023,  767},
  {  639,  383,  127, 1151,  895},
  {  767,  511,  255, 1279, 1023},
  {  895,  639,  383,  127, 1151},
  { 1023,  767,  511,  255, 1279},
  { 1151,  895,  639,  383,  127} };

/**************************************************************************************
 * Function:    PreMultiply64
 *
 * Description: pre-twiddle stage of 64-point DCT-IV
 *
 * Inputs:      buffer of 64 samples
 *
 * Outputs:     processed samples in same buffer
 *
 * Return:      none
 *
 * Notes:       minimum 1 GB in, 2 GB out, gains 2 int bits
 *              gbOut = gbIn + 1
 *              output is limited to sqrt(2)/2 plus GB in full GB
 *              uses 3-mul, 3-add butterflies instead of 4-mul, 2-add
 **************************************************************************************/
static void PreMultiply64(int *zbuf1)
{
    int i, ar1, ai1, ar2, ai2;
    int cos2a, sin2a, cos2b, sin2b;
    int *zbuf2;
    const int *csptr;

    zbuf2 = zbuf1 + 64 - 1;
    csptr = cos4sin4tab64;

    /* whole thing should fit in registers - verify that compiler does this */
    for (i = 64 >> 2; i != 0; i--) {
        /* cps2 = (cos+sin), sin2 = sin, cms2 = (cos-sin) */
        cos2a = *csptr;
        sin2a = *(csptr+1);
        cos2b = *(csptr+2);
        sin2b = *(csptr+3);

        ar1 = *(zbuf1 + 0);
        ai2 = *(zbuf1 + 1);
        ai1 = *(zbuf2 + 0);
        ar2 = *(zbuf2 - 1);

        /* gain 2 ints bit from MULSHIFT32 by Q30
         * max per-sample gain (ignoring implicit scaling) = MAX(sin(angle)+cos(angle)) = 1.414
         * i.e. gain 1 GB since worst case is sin(angle) = cos(angle) = 0.707 (Q30), gain 2 from
         *   extra sign bits, and eat one in adding
         */
        *zbuf1 = MULSHIFT32_ADD(cos2a, ar1, sin2a, ai1);  /* cos*ar1 + sin*ai1 */
        *(zbuf1+1) = MULSHIFT32_SUB(cos2a, ai1, sin2a, ar1);  /* cos*ai1 - sin*ar1 */

        *zbuf2 = MULSHIFT32_SUB(cos2b, ai2, sin2b, ar2);  /* cos*ai2 - sin*ar2 */
        *(zbuf2-1) = MULSHIFT32_ADD(cos2b, ar2, sin2b, ai2);  /* cos*ar2 + sin*ai2 */

		csptr += 4;
		zbuf1 += 2;
		zbuf2 -= 2;

    }
}

/**************************************************************************************
 * Function:    PostMultiply64
 *
 * Description: post-twiddle stage of 64-point type-IV DCT
 *
 * Inputs:      buffer of 64 samples
 *              number of output samples to calculate
 *
 * Outputs:     processed samples in same buffer
 *
 * Return:      none
 *
 * Notes:       minimum 1 GB in, 2 GB out, gains 2 int bits
 *              gbOut = gbIn + 1
 *              output is limited to sqrt(2)/2 plus GB in full GB
 *              nSampsOut is rounded up to next multiple of 4, since we calculate
 *                4 samples per loop
 **************************************************************************************/
static void PostMultiply64(int *fft1, int nSampsOut)
{
    int i, ar1, ai1, ar2, ai2;
    int cos2, sin2;
    int *fft2;
    const int *csptr;

    csptr = cos1sin1tab64;
    fft2 = fft1 + 64 - 1;

    /* load coeffs for first pass
     * cps2 = (cos+sin)/2, sin2 = sin/2, cms2 = (cos-sin)/2
     */
    cos2 = *csptr++;
    sin2 = *csptr++;

    for (i = (nSampsOut + 3) >> 2; i != 0; i--) {
        ar1 = *fft1;
        ai1 = *(fft1 + 1);
        ar2 = *(fft2 - 1);
        ai2 = *fft2;

        /* gain 2 int bits (multiplying by Q30), max gain = sqrt(2) */
        *fft2 = MULSHIFT32_SUB(sin2, ar1, cos2, ai1);	//sin*ar1 - cos*ai1
        *fft1 = MULSHIFT32_ADD(sin2, ai1, cos2, ar1);	//sin*ai1 + cos*ar1

        cos2 = *csptr;
        sin2 = *(csptr+1);

        *(fft2-1) = MULSHIFT32_ADD(cos2, ai2, sin2, ar2);	//cos*ai2 + sin*ar2
        *(fft1+1) = MULSHIFT32_SUB(cos2, ar2, sin2, ai2);	//cos*ar2 - sin*ai2
		csptr += 2;
		fft2 -= 2;
		fft1 += 2;
    }
}

/**************************************************************************************
 * Function:    QMFAnalysisConv
 *
 * Description: convolution kernel for analysis QMF
 *
 * Inputs:      pointer to coefficient table, reordered for sequential access
 *              delay buffer of size 32*10 = 320 real-valued PCM samples
 *              index for delay ring buffer (range = [0, 9])
 *
 * Outputs:     64 consecutive 32-bit samples
 *
 * Return:      none
 *
 * Notes:       this is carefully written to be efficient on ARM
 *              use the assembly code version in sbrqmfak.s when building for ARM!
 **************************************************************************************/
#if !defined(OR32_ASM)
int QMFAnalysisConv(int *cTab, int *delay, int dIdx, int *uBuf)
{
    int k, dOff;
    int *cPtr0, *cPtr1;
    U64 u64lo, u64hi;

    dOff = dIdx*32 + 31;
    cPtr0 = cTab;
    cPtr1 = cTab + 33*5 - 1;

    /* special first pass since we need to flip sign to create cTab[384], cTab[512] */
    u64lo.w64 = 0;
    u64hi.w64 = 0;
    u64lo.w64 = MADD64(u64lo.w64,  *cPtr0++,   delay[dOff]);    dOff -= 32; if (dOff < 0) {dOff += 320;}
    u64hi.w64 = MADD64(u64hi.w64,  *cPtr0++,   delay[dOff]);    dOff -= 32; if (dOff < 0) {dOff += 320;}
    u64lo.w64 = MADD64(u64lo.w64,  *cPtr0++,   delay[dOff]);    dOff -= 32; if (dOff < 0) {dOff += 320;}
    u64hi.w64 = MADD64(u64hi.w64,  *cPtr0++,   delay[dOff]);    dOff -= 32; if (dOff < 0) {dOff += 320;}
    u64lo.w64 = MADD64(u64lo.w64,  *cPtr0++,   delay[dOff]);    dOff -= 32; if (dOff < 0) {dOff += 320;}
    u64hi.w64 = MADD64(u64hi.w64,  *cPtr1--,   delay[dOff]);    dOff -= 32; if (dOff < 0) {dOff += 320;}
    u64lo.w64 = MADD64(u64lo.w64, -(*cPtr1--), delay[dOff]);    dOff -= 32; if (dOff < 0) {dOff += 320;}
    u64hi.w64 = MADD64(u64hi.w64,  *cPtr1--,   delay[dOff]);    dOff -= 32; if (dOff < 0) {dOff += 320;}
    u64lo.w64 = MADD64(u64lo.w64, -(*cPtr1--), delay[dOff]);    dOff -= 32; if (dOff < 0) {dOff += 320;}
    u64hi.w64 = MADD64(u64hi.w64,  *cPtr1--,   delay[dOff]);    dOff -= 32; if (dOff < 0) {dOff += 320;}

    uBuf[0]  = u64lo.r.hi32;
    uBuf[32] = u64hi.r.hi32;
    uBuf++;
    dOff--;

    /* max gain for any sample in uBuf, after scaling by cTab, ~= 0.99
     * so we can just sum the uBuf values with no overflow problems
     */
    for (k = 1; k <= 31; k++) {
        u64lo.w64 = 0;
        u64hi.w64 = 0;
        u64lo.w64 = MADD64(u64lo.w64, *cPtr0++, delay[dOff]);   dOff -= 32; if (dOff < 0) {dOff += 320;}
        u64hi.w64 = MADD64(u64hi.w64, *cPtr0++, delay[dOff]);   dOff -= 32; if (dOff < 0) {dOff += 320;}
        u64lo.w64 = MADD64(u64lo.w64, *cPtr0++, delay[dOff]);   dOff -= 32; if (dOff < 0) {dOff += 320;}
        u64hi.w64 = MADD64(u64hi.w64, *cPtr0++, delay[dOff]);   dOff -= 32; if (dOff < 0) {dOff += 320;}
        u64lo.w64 = MADD64(u64lo.w64, *cPtr0++, delay[dOff]);   dOff -= 32; if (dOff < 0) {dOff += 320;}
        u64hi.w64 = MADD64(u64hi.w64, *cPtr1--, delay[dOff]);   dOff -= 32; if (dOff < 0) {dOff += 320;}
        u64lo.w64 = MADD64(u64lo.w64, *cPtr1--, delay[dOff]);   dOff -= 32; if (dOff < 0) {dOff += 320;}
        u64hi.w64 = MADD64(u64hi.w64, *cPtr1--, delay[dOff]);   dOff -= 32; if (dOff < 0) {dOff += 320;}
        u64lo.w64 = MADD64(u64lo.w64, *cPtr1--, delay[dOff]);   dOff -= 32; if (dOff < 0) {dOff += 320;}
        u64hi.w64 = MADD64(u64hi.w64, *cPtr1--, delay[dOff]);   dOff -= 32; if (dOff < 0) {dOff += 320;}

        uBuf[0]  = u64lo.r.hi32;
        uBuf[32] = u64hi.r.hi32;
        uBuf++;
        dOff--;
    }
}
#else
int QMFAnalysisConv(int *cTab, int *delay, int dIdx, int *uBuf)
{
    int k, df0, df1, df2, df3, df4, df5, df6, df7, df8, df9;
    int *cPtr0, *cPtr1;
    if (!cTab || dIdx>10 || dIdx < 0 || !uBuf)
    {
        printf("[AAC] QMFAnalysisConv err \n");
        return ERR_AAC_SBR_QMF_DATA_ERROR;
    }

    cPtr0 = cTab;
    cPtr1 = cTab + 33*5 - 1;

    df0 = anyIdxTab[dIdx][0];
    df1 = anyIdxTab[dIdx][1];
    df2 = anyIdxTab[dIdx][2];
    df3 = anyIdxTab[dIdx][3];
    df4 = anyIdxTab[dIdx][4];
    df5 = anyIdxTab[dIdx][5];
    df6 = anyIdxTab[dIdx][6];
    df7 = anyIdxTab[dIdx][7];
    df8 = anyIdxTab[dIdx][8];
    df9 = anyIdxTab[dIdx][9];

    /* special first pass since we need to flip sign to create cTab[384], cTab[512] */
    asm ("l.mac %0, %1" : : "r"( *cPtr0),   "r"(delay[df0])); df0--;
    asm ("l.mac %0, %1" : : "r"( *(cPtr0+2)),   "r"(delay[df2])); df2--;
    asm ("l.mac %0, %1" : : "r"( *(cPtr0+4)),   "r"(delay[df4])); df4--;
    asm ("l.mac %0, %1" : : "r"(-(*(cPtr1-1))), "r"(delay[df6])); df6--;
    asm ("l.mac %0, %1" : : "r"(-(*(cPtr1-3))), "r"(delay[df8])); df8--;
    asm volatile ("l.macrc %0, 32" : "=r"(uBuf[0]));

    asm ("l.mac %0, %1" : : "r"(*(cPtr0+1)), "r"(delay[df1])); df1--;
    asm ("l.mac %0, %1" : : "r"(*(cPtr0+3)), "r"(delay[df3])); df3--;
    asm ("l.mac %0, %1" : : "r"(*cPtr1), "r"(delay[df5])); df5--;
    asm ("l.mac %0, %1" : : "r"(*(cPtr1-2)), "r"(delay[df7])); df7--;
    asm ("l.mac %0, %1" : : "r"(*(cPtr1-4)), "r"(delay[df9])); df9--;
    asm volatile ("l.macrc %0, 32" : "=r"(uBuf[32]));
    cPtr0 += 5;
    cPtr1 -= 5;
    uBuf++;

    /* max gain for any sample in uBuf, after scaling by cTab, ~= 0.99
     * so we can just sum the uBuf values with no overflow problems
     */
    for (k = 1; k <= 31; k++) 
    {
        asm ("l.mac %0, %1" : : "r"(*cPtr0), "r"(delay[df0])); df0--;
        asm ("l.mac %0, %1" : : "r"(*(cPtr0+2)), "r"(delay[df2])); df2--;
        asm ("l.mac %0, %1" : : "r"(*(cPtr0+4)), "r"(delay[df4])); df4--;
        asm ("l.mac %0, %1" : : "r"(*(cPtr1-1)), "r"(delay[df6])); df6--;
        asm ("l.mac %0, %1" : : "r"(*(cPtr1-3)), "r"(delay[df8])); df8--;
        asm volatile ("l.macrc %0, 32" : "=r"(uBuf[0]));

        asm ("l.mac %0, %1" : : "r"(*(cPtr0+1)), "r"(delay[df1])); df1--;
        asm ("l.mac %0, %1" : : "r"(*(cPtr0+3)), "r"(delay[df3])); df3--;
        asm ("l.mac %0, %1" : : "r"(*cPtr1), "r"(delay[df5])); df5--;
        asm ("l.mac %0, %1" : : "r"(*(cPtr1-2)), "r"(delay[df7])); df7--;
        asm ("l.mac %0, %1" : : "r"(*(cPtr1-4)), "r"(delay[df9])); df9--;
        asm volatile ("l.macrc %0, 32" : "=r"(uBuf[32]));

        cPtr0 += 5;
        cPtr1 -= 5;
        uBuf++;
    }
    return 0;    
}
#endif /* OR32_ASM */

/**************************************************************************************
 * Function:    QMFAnalysis
 *
 * Description: 32-subband analysis QMF (4.6.18.4.1)
 *
 * Inputs:      32 consecutive samples of decoded 32-bit PCM, format = Q(fBitsIn)
 *              delay buffer of size 32*10 = 320 PCM samples
 *              number of fraction bits in input PCM
 *              index for delay ring buffer (range = [0, 9])
 *              number of subbands to calculate (range = [0, 32])
 *
 * Outputs:     qmfaBands complex subband samples, format = Q(FBITS_OUT_QMFA)
 *              updated delay buffer
 *              updated delay index
 *
 * Return:      guard bit mask
 *
 * Notes:       output stored as RE{X0}, IM{X0}, RE{X1}, IM{X1}, ... RE{X31}, IM{X31}
 *              output stored in int buffer of size 64*2 = 128
 *                (zero-filled from XBuf[2*qmfaBands] to XBuf[127])
 **************************************************************************************/
int QMFAnalysis(int *inbuf, int *delay, int *XBuf, int fBitsIn, int *delayIdx, int qmfaBands)
{
    int n, y, shift, gbMask = 0;
    int *delayPtr, *uBuf, *tBuf;
    int nResult=0;

    if (!inbuf || *delayIdx>10 || *delayIdx < 0 || !XBuf)
    {
        printf("[AAC] QMFAnalysis err \n");
        return ERR_AAC_SBR_QMF_DATA_ERROR;
    }
    
    /* use XBuf[128] as temp buffer for reordering */
    uBuf = XBuf;        /* first 64 samples */
    tBuf = XBuf + 64;   /* second 64 samples */

    /* overwrite oldest PCM with new PCM
     * delay[n] has 1 GB after shifting (either << or >>)
     */
    delayPtr = delay + (*delayIdx * 32);
    if (fBitsIn > FBITS_IN_QMFA) 
    {
        shift = MIN(fBitsIn - FBITS_IN_QMFA, 31);
        for (n = 32; n != 0; n--) 
        {
            y = (*inbuf) >> shift;
            inbuf++;
            *delayPtr++ = y;
        }
    }
    else
    {
        shift = MIN(FBITS_IN_QMFA - fBitsIn, 30);
        for (n = 32; n != 0; n--) 
        {
            y = *inbuf++;
            CLIP_2N_SHIFT(y, shift);
            *delayPtr++ = y;
        }
    }

    nResult = QMFAnalysisConv((int *)cTabA, delay, *delayIdx, uBuf);
    if (nResult)
    {
        return ERR_AAC_SBR_QMF_DATA_ERROR;
    }

    /* uBuf has at least 2 GB right now (1 from clipping to Q(FBITS_IN_QMFA), one from
     *   the scaling by cTab (MULSHIFT32(*delayPtr--, *cPtr++), with net gain of < 1.0)
     * TODO - fuse with QMFAnalysisConv to avoid separate reordering
     */
    tBuf[2*0 + 0] = uBuf[0];
    tBuf[2*0 + 1] = uBuf[1];
    for (n = 1; n < 31; n++) {
        tBuf[2*n + 0] = -uBuf[64-n];
        tBuf[2*n + 1] =  uBuf[n+1];
    }
    tBuf[2*31 + 1] =  uBuf[32];
    tBuf[2*31 + 0] = -uBuf[33];

    /* fast in-place DCT-IV - only need 2*qmfaBands output samples */
    PreMultiply64(tBuf);    /* 2 GB in, 3 GB out */
    FFT32C(tBuf);           /* 3 GB in, 1 GB out */
    PostMultiply64(tBuf, qmfaBands*2);  /* 1 GB in, 2 GB out */

    /* TODO - roll into PostMultiply (if enough registers) */
    gbMask = 0;
    for (n = 0; n < qmfaBands; n++) {
        XBuf[2*n+0] =  tBuf[ n + 0];    /* implicit scaling of 2 in our output Q format */
        gbMask |= FASTABS(XBuf[2*n+0]);
        XBuf[2*n+1] = -tBuf[63 - n];
        gbMask |= FASTABS(XBuf[2*n+1]);
    }

    /* fill top section with zeros for HF generation */
    for (    ; n < 64; n++) {
        XBuf[2*n+0] = 0;
        XBuf[2*n+1] = 0;
    }

    *delayIdx = (*delayIdx == NUM_QMF_DELAY_BUFS - 1 ? 0 : *delayIdx + 1);

    /* minimum of 2 GB in output */
    return gbMask;
}

/* lose FBITS_LOST_DCT4_64 in DCT4, gain 6 for implicit scaling by 1/64, lose 1 for cTab multiply (Q31) */
#define FBITS_OUT_QMFS  (FBITS_IN_QMFS - FBITS_LOST_DCT4_64 + 6 - 1)
#define RND_VAL         (1 << (FBITS_OUT_QMFS-1))

/**************************************************************************************
 * Function:    QMFSynthesisConv
 *
 * Description: final convolution kernel for synthesis QMF
 *
 * Inputs:      pointer to coefficient table, reordered for sequential access
 *              delay buffer of size 64*10 = 640 complex samples (1280 ints)
 *              index for delay ring buffer (range = [0, 9])
 *              number of QMF subbands to process (range = [0, 64])
 *              number of channels
 *
 * Outputs:     64 consecutive 16-bit PCM samples, interleaved by factor of nChans
 *
 * Return:      none
 *
 * Notes:       this is carefully written to be efficient on ARM
 *              use the assembly code version in sbrqmfsk.s when building for ARM!
 **************************************************************************************/
#if !defined(OR32_ASM)
void QMFSynthesisConv(int *cPtr, int *delay, int dIdx, short *outbuf, int nChans)
{
    int k, dOff0, dOff1;
    U64 sum64;

    dOff0 = (dIdx)*128;
    dOff1 = dOff0 - 1;
    if (dOff1 < 0)
        dOff1 += 1280;

    /* scaling note: total gain of coefs (cPtr[0]-cPtr[9] for any k) is < 2.0, so 1 GB in delay values is adequate */
    for (k = 0; k <= 63; k++) {
        sum64.w64 = 0;
        sum64.w64 = MADD64(sum64.w64, *cPtr++, delay[dOff0]);   dOff0 -= 256; if (dOff0 < 0) {dOff0 += 1280;}
        sum64.w64 = MADD64(sum64.w64, *cPtr++, delay[dOff1]);   dOff1 -= 256; if (dOff1 < 0) {dOff1 += 1280;}
        sum64.w64 = MADD64(sum64.w64, *cPtr++, delay[dOff0]);   dOff0 -= 256; if (dOff0 < 0) {dOff0 += 1280;}
        sum64.w64 = MADD64(sum64.w64, *cPtr++, delay[dOff1]);   dOff1 -= 256; if (dOff1 < 0) {dOff1 += 1280;}
        sum64.w64 = MADD64(sum64.w64, *cPtr++, delay[dOff0]);   dOff0 -= 256; if (dOff0 < 0) {dOff0 += 1280;}
        sum64.w64 = MADD64(sum64.w64, *cPtr++, delay[dOff1]);   dOff1 -= 256; if (dOff1 < 0) {dOff1 += 1280;}
        sum64.w64 = MADD64(sum64.w64, *cPtr++, delay[dOff0]);   dOff0 -= 256; if (dOff0 < 0) {dOff0 += 1280;}
        sum64.w64 = MADD64(sum64.w64, *cPtr++, delay[dOff1]);   dOff1 -= 256; if (dOff1 < 0) {dOff1 += 1280;}
        sum64.w64 = MADD64(sum64.w64, *cPtr++, delay[dOff0]);   dOff0 -= 256; if (dOff0 < 0) {dOff0 += 1280;}
        sum64.w64 = MADD64(sum64.w64, *cPtr++, delay[dOff1]);   dOff1 -= 256; if (dOff1 < 0) {dOff1 += 1280;}

        dOff0++;
        dOff1--;
        *outbuf = CLIPTOSHORT((sum64.r.hi32 + RND_VAL) >> FBITS_OUT_QMFS);
        outbuf += nChans;
    }
}
#else
void QMFSynthesisConv(int *cPtr, int *delay, int dIdx, short *outbuf, int nChans)
{
    int k, df00, df01, df02, df03, df04, df10, df11, df12, df13, df14;
    int n;

    if (!cPtr || dIdx>10 || dIdx < 0 || !outbuf)
    {
        printf("[AAC] QMFSynthesisConv err %d %d %d %d \n",cPtr,dIdx,dIdx,outbuf);
        return; //ERR_AAC_SBR_QMF_DATA_ERROR
    }

    df00 = synIdxTab0[dIdx][0];
    df01 = synIdxTab0[dIdx][1];
    df02 = synIdxTab0[dIdx][2];
    df03 = synIdxTab0[dIdx][3];
    df04 = synIdxTab0[dIdx][4];
    df10 = synIdxTab1[dIdx][0];
    df11 = synIdxTab1[dIdx][1];
    df12 = synIdxTab1[dIdx][2];
    df13 = synIdxTab1[dIdx][3];
    df14 = synIdxTab1[dIdx][4];

    /* scaling note: total gain of coefs (cPtr[0]-cPtr[9] for any k) is < 2.0, so 1 GB in delay values is adequate */
    for (k = 0; k <= 63; k++) 
    {
        asm ("l.mac %0, %1" : : "r"(*cPtr), "r"(delay[df00])); df00++;
        asm ("l.mac %0, %1" : : "r"(*(cPtr+1)), "r"(delay[df10])); df10--;
        asm ("l.mac %0, %1" : : "r"(*(cPtr+2)), "r"(delay[df01])); df01++;
        asm ("l.mac %0, %1" : : "r"(*(cPtr+3)), "r"(delay[df11])); df11--;
        asm ("l.mac %0, %1" : : "r"(*(cPtr+4)), "r"(delay[df02])); df02++;
        asm ("l.mac %0, %1" : : "r"(*(cPtr+5)), "r"(delay[df12])); df12--;
        asm ("l.mac %0, %1" : : "r"(*(cPtr+6)), "r"(delay[df03])); df03++;
        asm ("l.mac %0, %1" : : "r"(*(cPtr+7)), "r"(delay[df13])); df13--;
        asm ("l.mac %0, %1" : : "r"(*(cPtr+8)), "r"(delay[df04])); df04++;
        asm ("l.mac %0, %1" : : "r"(*(cPtr+9)), "r"(delay[df14])); df14--;
        asm volatile ("l.macrc %0, 32" : "=r"(n));
        *outbuf = CLIPTOSHORT((n + RND_VAL) >> FBITS_OUT_QMFS);


        cPtr += 10;
        outbuf += nChans;
    }
}
#endif /* OR32_ASM */

/**************************************************************************************
 * Function:    QMFSynthesis
 *
 * Description: 64-subband synthesis QMF (4.6.18.4.2)
 *
 * Inputs:      64 consecutive complex subband QMF samples, format = Q(FBITS_IN_QMFS)
 *              delay buffer of size 64*10 = 640 complex samples (1280 ints)
 *              index for delay ring buffer (range = [0, 9])
 *              number of QMF subbands to process (range = [0, 64])
 *              number of channels
 *
 * Outputs:     64 consecutive 16-bit PCM samples, interleaved by factor of nChans
 *              updated delay buffer
 *              updated delay index
 *
 * Return:      none
 *
 * Notes:       assumes MIN_GBITS_IN_QMFS guard bits in input, either from
 *                QMFAnalysis (if upsampling only) or from MapHF (if SBR on)
 **************************************************************************************/
void QMFSynthesis(int *inbuf, int *delay, int *delayIdx, int qmfsBands, short *outbuf, int nChans)
{
    int n, a0, a1, b0, b1, dOff0, dOff1, dIdx;
    int *tBufLo, *tBufHi;

    dIdx = *delayIdx;
    tBufLo = delay + dIdx*128 + 0;
    tBufHi = delay + dIdx*128 + 127;

    /* reorder inputs to DCT-IV, only use first qmfsBands (complex) samples
     * TODO - fuse with PreMultiply64 to avoid separate reordering steps
     */
    for (n = 0; n < qmfsBands >> 1; n++) 
    {
        a0 = *inbuf++;
        b0 = *inbuf++;
        a1 = *inbuf++;
        b1 = *inbuf++;
        *tBufLo++ = a0;
        *tBufLo++ = a1;
        *tBufHi-- = b0;
        *tBufHi-- = b1;
    }
    if (qmfsBands & 0x01) 
    {
        a0 = *inbuf++;
        b0 = *inbuf++;
        *tBufLo++ = a0;
        *tBufHi-- = b0;
        *tBufLo++ = 0;
        *tBufHi-- = 0;
        n++;
    }
    for (     ; n < 32; n++) 
    {
        *tBufLo++ = 0;
        *tBufHi-- = 0;
        *tBufLo++ = 0;
        *tBufHi-- = 0;
    }

    tBufLo = delay + dIdx*128 + 0;
    tBufHi = delay + dIdx*128 + 64;

    /* 2 GB in, 3 GB out */
    PreMultiply64(tBufLo);
    PreMultiply64(tBufHi);

    /* 3 GB in, 1 GB out */
    FFT32C(tBufLo);
    FFT32C(tBufHi);

    /* 1 GB in, 2 GB out */
    PostMultiply64(tBufLo, 64);
    PostMultiply64(tBufHi, 64);

    /* could fuse with PostMultiply64 to avoid separate pass */
    dOff0 = dIdx*128;
    dOff1 = dIdx*128 + 64;
    for (n = 32; n != 0; n--) {
        a0 =  (*tBufLo++);
        a1 =  (*tBufLo++);
        b0 =  (*tBufHi++);
        b1 = -(*tBufHi++);

        delay[dOff0++] = (b0 - a0);
        delay[dOff0++] = (b1 - a1);
        delay[dOff1++] = (b0 + a0);
        delay[dOff1++] = (b1 + a1);
    }

    QMFSynthesisConv((int *)cTabS, delay, dIdx, outbuf, nChans);

    *delayIdx = (*delayIdx == NUM_QMF_DELAY_BUFS - 1 ? 0 : *delayIdx + 1);
}

#endif /* defined(AAC_ENABLE_SBR) */
