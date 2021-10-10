/* ***** BEGIN LICENSE BLOCK *****
 * Source last modified: $Id: sbrhfgen.c,v 1.2 2005/05/19 20:45:20 jrecker Exp $
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
 * sbrhfgen.c - high frequency generation for SBR
 **************************************************************************************/

#include "aacdec.h"

#if defined(AAC_ENABLE_SBR)

#include "sbr.h"
#include "assembly.h"

#define FBITS_LPCOEFS   29  /* Q29 for range of (-4, 4) */
#define MAG_16          (16 * (1 << (32 - (2*(32-FBITS_LPCOEFS)))))     /* i.e. 16 in Q26 format */
#define RELAX_COEF      0x7ffff79c  /* 1.0 / (1.0 + 1e-6), Q31 */

/* newBWTab[prev invfMode][curr invfMode], format = Q31 (table 4.158)
 * sample file which uses all of these: al_sbr_sr_64_2_fsaac32.aac
 */
static const int newBWTab[4][4] = {
    {0x00000000, 0x4ccccccd, 0x73333333, 0x7d70a3d7},
    {0x4ccccccd, 0x60000000, 0x73333333, 0x7d70a3d7},
    {0x00000000, 0x60000000, 0x73333333, 0x7d70a3d7},
    {0x00000000, 0x60000000, 0x73333333, 0x7d70a3d7},
};

/**************************************************************************************
 * Function:    CVKernel1
 *
 * Description: kernel of covariance matrix calculation for p01, p11, p12, p22
 *
 * Inputs:      buffer of low-freq samples, starting at time index = 0,
 *                freq index = patch subband
 *
 * Outputs:     64-bit accumulators for p01re, p01im, p12re, p12im, p11re, p22re
 *                stored in accBuf
 *
 * Return:      none
 *
 * Notes:       this is carefully written to be efficient on ARM
 *              use the assembly code version in sbrcov.s when building for ARM!
 **************************************************************************************/
#if !defined(OR32_ASM)
#ifdef AAC_INTERNAL_SRAM_FLOW
void CVKernel1(PSInfoSBR *psi,int p, int *accBuf)
#else
void CVKernel1(int *XBuf, int *accBuf)
#endif // AAC_INTERNAL_SRAM_FLOW
{
    U64 p01re, p01im, p12re, p12im, p11re, p22re;
    int n, x0re, x0im, x1re, x1im;
#ifdef AAC_INTERNAL_SRAM_FLOW
    int *XBuf = psi->XBuf[0][p];
    int nTemp =0;
#endif
#ifdef AAC_ENABLE_INTERNAL_SD
    XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
#endif

    x0re = XBuf[0];
    x0im = XBuf[1];
#ifdef AAC_INTERNAL_SRAM_FLOW
    nTemp+=1;
    XBuf = psi->XBuf[nTemp][p];
#else
    XBuf += (2*64);
#endif
#ifdef AAC_ENABLE_INTERNAL_SD
    XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
#endif

    x1re = XBuf[0];
    x1im = XBuf[1];
#ifdef AAC_INTERNAL_SRAM_FLOW
    nTemp+=1;
    XBuf = psi->XBuf[nTemp][p];
#else
    XBuf += (2*64);
#endif
#ifdef AAC_ENABLE_INTERNAL_SD
    XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
#endif

    p01re.w64 = p01im.w64 = 0;
    p12re.w64 = p12im.w64 = 0;
    p11re.w64 = 0;
    p22re.w64 = 0;

    p12re.w64 = MADD64(p12re.w64,  x1re, x0re);
    p12re.w64 = MADD64(p12re.w64,  x1im, x0im);
    p12im.w64 = MADD64(p12im.w64,  x0re, x1im);
    p12im.w64 = MADD64(p12im.w64, -x0im, x1re);
    p22re.w64 = MADD64(p22re.w64,  x0re, x0re);
    p22re.w64 = MADD64(p22re.w64,  x0im, x0im);
    for (n = (NUM_TIME_SLOTS*SAMPLES_PER_SLOT + 6); n != 0; n--) {
        /* 4 input, 3*2 acc, 1 ptr, 1 loop counter = 12 registers (use same for x0im, -x0im) */
        x0re = x1re;
        x0im = x1im;
        x1re = XBuf[0];
        x1im = XBuf[1];

        p01re.w64 = MADD64(p01re.w64,  x1re, x0re);
        p01re.w64 = MADD64(p01re.w64,  x1im, x0im);
        p01im.w64 = MADD64(p01im.w64,  x0re, x1im);
        p01im.w64 = MADD64(p01im.w64, -x0im, x1re);
        p11re.w64 = MADD64(p11re.w64,  x0re, x0re);
        p11re.w64 = MADD64(p11re.w64,  x0im, x0im);
#ifdef AAC_INTERNAL_SRAM_FLOW
        nTemp+=1;
    #ifdef AAC_ENABLE_INTERNAL_SD
        if (nTemp<8)
            XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
        else if (nTemp<16)
            XBuf = psi->XbufTmp2->bufTemp[nTemp-8][p];
        else
            XBuf = psi->XBuf[nTemp][p];
    #else
        XBuf = psi->XBuf[nTemp][p];
    #endif
#else
        XBuf += (2*64);
#endif
    }
    /* these can be derived by slight changes to account for boundary conditions */
    p12re.w64 += p01re.w64;
    p12re.w64 = MADD64(p12re.w64, x1re, -x0re);
    p12re.w64 = MADD64(p12re.w64, x1im, -x0im);
    p12im.w64 += p01im.w64;
    p12im.w64 = MADD64(p12im.w64, x0re, -x1im);
    p12im.w64 = MADD64(p12im.w64, x0im,  x1re);
    p22re.w64 += p11re.w64;
    p22re.w64 = MADD64(p22re.w64, x0re, -x0re);
    p22re.w64 = MADD64(p22re.w64, x0im, -x0im);

    accBuf[0]  = p01re.r.lo32;  accBuf[1]  = p01re.r.hi32;
    accBuf[2]  = p01im.r.lo32;  accBuf[3]  = p01im.r.hi32;
    accBuf[4]  = p11re.r.lo32;  accBuf[5]  = p11re.r.hi32;
    accBuf[6]  = p12re.r.lo32;  accBuf[7]  = p12re.r.hi32;
    accBuf[8]  = p12im.r.lo32;  accBuf[9]  = p12im.r.hi32;
    accBuf[10] = p22re.r.lo32;  accBuf[11] = p22re.r.hi32;
}
#else
#ifdef AAC_INTERNAL_SRAM_FLOW
void CVKernel1(PSInfoSBR *psi,int p, int *accBuf)
#else
void CVKernel1(int *XBuf, int *accBuf)
#endif // AAC_INTERNAL_SRAM_FLOW
{
    int n, x0re, x0im, x1re, x1im;
    int x0re_o, x0im_o, x1re_o, x1im_o;
	int buf0, buf1, buf2, buf3, buf4, buf5;
    int *ptr;

    //asm volatile("l.mtspr r0, r0, 0x2801");
    //asm volatile("l.mtspr r0, r0, 0x2802");
#ifdef AAC_INTERNAL_SRAM_FLOW
    int *XBuf = psi->XBuf[0][p];
    int nTemp =0;
#endif
#ifdef AAC_ENABLE_INTERNAL_SD
    XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
#endif
    x0re_o = XBuf[0];
    x0im_o = XBuf[1];
#ifdef AAC_INTERNAL_SRAM_FLOW
    nTemp+=1;
    XBuf = psi->XBuf[nTemp][p];
#else
    XBuf += (2*64);
#endif
#ifdef AAC_ENABLE_INTERNAL_SD
    XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
#endif

    x1re_o = XBuf[0];
    x1im_o = XBuf[1];
#ifdef AAC_INTERNAL_SRAM_FLOW
    nTemp+=1;
    XBuf = psi->XBuf[nTemp][p];
#else
    XBuf += (2*64);
#endif
#ifdef AAC_ENABLE_INTERNAL_SD
    XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
#endif

    //ptr = XBuf;
    x1re = x1re_o;
    x1im = x1im_o;
	buf0 = 0;
	buf1 = 0;
	buf2 = 0;
	buf3 = 0;
	buf4 = 0;
	buf5 = 0;

    for (n = (NUM_TIME_SLOTS*SAMPLES_PER_SLOT + 6); n != 0; n--) {
        x0re = x1re;
        x0im = x1im;
        x1re = XBuf[0];
        x1im = XBuf[1];
#ifdef AAC_INTERNAL_SRAM_FLOW
        nTemp+=1;
    #ifdef AAC_ENABLE_INTERNAL_SD
        if (nTemp<8)
            XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
        else if (nTemp<16)
            XBuf = psi->XbufTmp2->bufTemp[nTemp-8][p];
        else
            XBuf = psi->XBuf[nTemp][p];
    #else
        XBuf = psi->XBuf[nTemp][p];
    #endif
#else
        XBuf += (2*64);
#endif 

		asm volatile("l.mtspr r0, %0, 0x2801" : : "r"(buf0));
		asm volatile("l.mtspr r0, %0, 0x2802" : : "r"(buf1));
        asm ("l.mac %0, %1" : : "r"(x1re), "r"(x0re));
        asm ("l.mac %0, %1" : : "r"(x1im), "r"(x0im));
		asm volatile("l.mfspr %0, r0, 0x2801" :  "=r"(buf0));
		asm volatile("l.macrc %0, 32"         :  "=r"(buf1));

 		asm volatile("l.mtspr r0, %0, 0x2801" : : "r"(buf2));
		asm volatile("l.mtspr r0, %0, 0x2802" : : "r"(buf3));
        asm ("l.mac %0, %1" : : "r"( x0re), "r"(x1im));
        asm ("l.msb %0, %1" : : "r"( x0im), "r"(x1re));
		asm volatile("l.mfspr %0, r0, 0x2801" :  "=r"(buf2));
		asm volatile("l.macrc %0, 32"         :  "=r"(buf3));

		asm volatile("l.mtspr r0, %0, 0x2801" : : "r"(buf4));
		asm volatile("l.mtspr r0, %0, 0x2802" : : "r"(buf5));
        asm ("l.mac %0, %1" : : "r"(x0re), "r"(x0re));
        asm ("l.mac %0, %1" : : "r"(x0im), "r"(x0im));
		asm volatile("l.mfspr %0, r0, 0x2801" :  "=r"(buf4));
		asm volatile("l.macrc %0, 32"         :  "=r"(buf5));
    }

    /* these can be derived by slight changes to account for boundary conditions */
	accBuf[0] = buf0;
	accBuf[1] = buf1;
	accBuf[2] = buf2;
	accBuf[3] = buf3;
	accBuf[4] = buf4;
	accBuf[5] = buf5;

    asm volatile("l.mtspr r0, %0, 0x2801" : : "r"(buf0));
    asm volatile("l.mtspr r0, %0, 0x2802" : : "r"(buf1));
    asm ("l.mac   %0, %1"         : : "%r"(x1re_o), "r"(x0re_o));
    asm ("l.mac   %0, %1"         : : "%r"(x1im_o), "r"(x0im_o));
    asm ("l.msb   %0, %1"         : : "%r"( x1re), "r"( x0re));
    asm ("l.msb   %0, %1"         : : "%r"( x1im), "r"( x0im));
    asm volatile("l.mfspr %0, r0, 0x2801" :   "=r"(accBuf[6]));
    asm volatile("l.macrc %0, 32"         :   "=r"(accBuf[7]));

    asm volatile("l.mtspr r0, %0, 0x2801" : : "r"(buf2));
    asm volatile("l.mtspr r0, %0, 0x2802" : : "r"(buf3));
    asm ("l.mac   %0, %1"         : : "%r"( x0re_o), "r"(x1im_o));
    asm ("l.msb   %0, %1"         : : "%r"( x0im_o), "r"(x1re_o));
    asm ("l.msb   %0, %1"         : : "%r"( x0re), "r"( x1im));
    asm ("l.mac   %0, %1"         : : "%r"( x0im), "r"( x1re));
    asm volatile("l.mfspr %0, r0, 0x2801" :   "=r"(accBuf[8]));
    asm volatile("l.macrc %0, 32"         :   "=r"(accBuf[9]));

    asm volatile("l.mtspr r0, %0, 0x2801" : : "r"(buf4));
    asm volatile("l.mtspr r0, %0, 0x2802" : : "r"(buf5));
    asm ("l.mac   %0, %1"         : : "%r"(x0re_o), "r"(x0re_o));
    asm ("l.mac   %0, %1"         : : "%r"(x0im_o), "r"(x0im_o));
    asm ("l.msb   %0, %1"         : : "%r"( x0re), "r"( x0re));
    asm ("l.msb   %0, %1"         : : "%r"( x0im), "r"( x0im));
    asm volatile("l.mfspr %0, r0, 0x2801" :   "=r"(accBuf[10]));
    asm volatile("l.macrc %0, 32"         :   "=r"(accBuf[11]));

}
#endif /* OR32_ASM */

/**************************************************************************************
 * Function:    CalcCovariance1
 *
 * Description: calculate covariance matrix for p01, p12, p11, p22 (4.6.18.6.2)
 *
 * Inputs:      buffer of low-freq samples, starting at time index 0,
 *                freq index = patch subband
 *
 * Outputs:     complex covariance elements p01re, p01im, p12re, p12im, p11re, p22re
 *                (p11im = p22im = 0)
 *              format = integer (Q0) * 2^N, with scalefactor N >= 0
 *
 * Return:      scalefactor N
 *
 * Notes:       outputs are normalized to have 1 GB (sign in at least top 2 bits)
 **************************************************************************************/
#ifdef AAC_INTERNAL_SRAM_FLOW
static int CalcCovariance1(PSInfoSBR *psi,int p, int *p01reN, int *p01imN, int *p12reN, int *p12imN, int *p11reN, int *p22reN)
#else
static int CalcCovariance1(int *XBuf, int *p01reN, int *p01imN, int *p12reN, int *p12imN, int *p11reN, int *p22reN)
#endif
{
    int accBuf[2*6];
    int n, z, s, loShift, hiShift, gbMask;
    U64 p01re, p01im, p12re, p12im, p11re, p22re;
#ifdef AAC_INTERNAL_SRAM_FLOW
    int *XBuf = psi->XBuf[0][p];
    int nTemp =0;
#endif
#ifdef AAC_INTERNAL_SRAM_FLOW
    CVKernel1(psi,p, accBuf);
#else
    CVKernel1(XBuf, accBuf);
#endif
    p01re.r.lo32 = accBuf[0];   p01re.r.hi32 = accBuf[1];
    p01im.r.lo32 = accBuf[2];   p01im.r.hi32 = accBuf[3];
    p11re.r.lo32 = accBuf[4];   p11re.r.hi32 = accBuf[5];
    p12re.r.lo32 = accBuf[6];   p12re.r.hi32 = accBuf[7];
    p12im.r.lo32 = accBuf[8];   p12im.r.hi32 = accBuf[9];
    p22re.r.lo32 = accBuf[10];  p22re.r.hi32 = accBuf[11];

    /* 64-bit accumulators now have 2*FBITS_OUT_QMFA fraction bits
     * want to scale them down to integers (32-bit signed, Q0)
     *   with scale factor of 2^n, n >= 0
     * leave 2 GB's for calculating determinant, so take top 30 non-zero bits
     */
    gbMask  = ((p01re.r.hi32) ^ (p01re.r.hi32 >> 31)) | ((p01im.r.hi32) ^ (p01im.r.hi32 >> 31));
    gbMask |= ((p12re.r.hi32) ^ (p12re.r.hi32 >> 31)) | ((p12im.r.hi32) ^ (p12im.r.hi32 >> 31));
    gbMask |= ((p11re.r.hi32) ^ (p11re.r.hi32 >> 31)) | ((p22re.r.hi32) ^ (p22re.r.hi32 >> 31));
    if (gbMask == 0) {
        s = p01re.r.hi32 >> 31; gbMask  = (p01re.r.lo32 ^ s) - s;
        s = p01im.r.hi32 >> 31; gbMask |= (p01im.r.lo32 ^ s) - s;
        s = p12re.r.hi32 >> 31; gbMask |= (p12re.r.lo32 ^ s) - s;
        s = p12im.r.hi32 >> 31; gbMask |= (p12im.r.lo32 ^ s) - s;
        s = p11re.r.hi32 >> 31; gbMask |= (p11re.r.lo32 ^ s) - s;
        s = p22re.r.hi32 >> 31; gbMask |= (p22re.r.lo32 ^ s) - s;
        z = 32 + CLZ(gbMask);
    } else {
        gbMask  = FASTABS(p01re.r.hi32) | FASTABS(p01im.r.hi32);
        gbMask |= FASTABS(p12re.r.hi32) | FASTABS(p12im.r.hi32);
        gbMask |= FASTABS(p11re.r.hi32) | FASTABS(p22re.r.hi32);
        z = CLZ(gbMask);
    }

    n = 64 - z; /* number of non-zero bits in bottom of 64-bit word */
    if (n <= 30) {
        loShift = (30 - n);
        *p01reN = p01re.r.lo32 << loShift;  *p01imN = p01im.r.lo32 << loShift;
        *p12reN = p12re.r.lo32 << loShift;  *p12imN = p12im.r.lo32 << loShift;
        *p11reN = p11re.r.lo32 << loShift;  *p22reN = p22re.r.lo32 << loShift;
        return -(loShift + 2*FBITS_OUT_QMFA);
    } else if (n < 32 + 30) {
        loShift = (n - 30);
        hiShift = 32 - loShift;
        *p01reN = (p01re.r.hi32 << hiShift) | (p01re.r.lo32 >> loShift);
        *p01imN = (p01im.r.hi32 << hiShift) | (p01im.r.lo32 >> loShift);
        *p12reN = (p12re.r.hi32 << hiShift) | (p12re.r.lo32 >> loShift);
        *p12imN = (p12im.r.hi32 << hiShift) | (p12im.r.lo32 >> loShift);
        *p11reN = (p11re.r.hi32 << hiShift) | (p11re.r.lo32 >> loShift);
        *p22reN = (p22re.r.hi32 << hiShift) | (p22re.r.lo32 >> loShift);
        return (loShift - 2*FBITS_OUT_QMFA);
    } else {
        hiShift = n - (32 + 30);
        *p01reN = p01re.r.hi32 >> hiShift;  *p01imN = p01im.r.hi32 >> hiShift;
        *p12reN = p12re.r.hi32 >> hiShift;  *p12imN = p12im.r.hi32 >> hiShift;
        *p11reN = p11re.r.hi32 >> hiShift;  *p22reN = p22re.r.hi32 >> hiShift;
        return (32 - 2*FBITS_OUT_QMFA - hiShift);
    }

    return 0;
}

/**************************************************************************************
 * Function:    CVKernel2
 *
 * Description: kernel of covariance matrix calculation for p02
 *
 * Inputs:      buffer of low-freq samples, starting at time index = 0,
 *                freq index = patch subband
 *
 * Outputs:     64-bit accumulators for p02re, p02im stored in accBuf
 *
 * Return:      none
 *
 * Notes:       this is carefully written to be efficient on ARM
 *              use the assembly code version in sbrcov.s when building for ARM!
 **************************************************************************************/
#if !defined(OR32_ASM)
#ifdef AAC_INTERNAL_SRAM_FLOW
void CVKernel2(PSInfoSBR *psi,int p, int *accBuf)
#else
void CVKernel2(int *XBuf, int *accBuf)
#endif
{
    U64 p02re, p02im;
    int n, x0re, x0im, x1re, x1im, x2re, x2im;
#ifdef AAC_INTERNAL_SRAM_FLOW
    int *XBuf = psi->XBuf[0][p];
    int nTemp =0;
#endif
#ifdef AAC_ENABLE_INTERNAL_SD
    XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
#endif

    p02re.w64 = p02im.w64 = 0;

    x0re = XBuf[0];
    x0im = XBuf[1];
#ifdef AAC_INTERNAL_SRAM_FLOW
    nTemp+=1;
    XBuf = psi->XBuf[nTemp][p];
#else
    XBuf += (2*64);
#endif
#ifdef AAC_ENABLE_INTERNAL_SD
    XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
#endif

    x1re = XBuf[0];
    x1im = XBuf[1];
#ifdef AAC_INTERNAL_SRAM_FLOW
    nTemp+=1;
    XBuf = psi->XBuf[nTemp][p];
#else
    XBuf += (2*64);
#endif
#ifdef AAC_ENABLE_INTERNAL_SD
    XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
#endif

    for (n = (NUM_TIME_SLOTS*SAMPLES_PER_SLOT + 6); n != 0; n--) {
        /* 6 input, 2*2 acc, 1 ptr, 1 loop counter = 12 registers (use same for x0im, -x0im) */
        x2re = XBuf[0];
        x2im = XBuf[1];

        p02re.w64 = MADD64(p02re.w64,  x2re, x0re);
        p02re.w64 = MADD64(p02re.w64,  x2im, x0im);
        p02im.w64 = MADD64(p02im.w64,  x0re, x2im);
        p02im.w64 = MADD64(p02im.w64, -x0im, x2re);

        x0re = x1re;
        x0im = x1im;
        x1re = x2re;
        x1im = x2im;
#ifdef AAC_INTERNAL_SRAM_FLOW
        nTemp+=1;
    #ifdef AAC_ENABLE_INTERNAL_SD
        if (nTemp<8)
            XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
        else if (nTemp<16)
            XBuf = psi->XbufTmp2->bufTemp[nTemp-8][p];
        else
            XBuf = psi->XBuf[nTemp][p];
    #else
        XBuf = psi->XBuf[nTemp][p];
    #endif
#else
        XBuf += (2*64);
#endif

    }

    accBuf[0] = p02re.r.lo32;
    accBuf[1] = p02re.r.hi32;
    accBuf[2] = p02im.r.lo32;
    accBuf[3] = p02im.r.hi32;
}
#else
#ifdef AAC_INTERNAL_SRAM_FLOW
void CVKernel2(PSInfoSBR *psi,int p, int *accBuf)
#else
void CVKernel2(int *XBuf, int *accBuf)
#endif
{
    int n, x0re, x0im, x1re, x1im, x2re, x2im;
	int buf0, buf1, buf2, buf3;
    int *ptr;

    //asm volatile("l.mtspr r0, r0, 0x2801");
    //asm volatile("l.mtspr r0, r0, 0x2802");
#ifdef AAC_INTERNAL_SRAM_FLOW
    int *XBuf = psi->XBuf[0][p];
    int nTemp =0;
#endif
#ifdef AAC_ENABLE_INTERNAL_SD
    XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
#endif

    x0re = XBuf[0];
    x0im = XBuf[1];
#ifdef AAC_INTERNAL_SRAM_FLOW
    nTemp+=1;
    XBuf = psi->XBuf[nTemp][p];
#else
    XBuf += (2*64);
#endif
#ifdef AAC_ENABLE_INTERNAL_SD
    XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
#endif

    x1re = XBuf[0];
    x1im = XBuf[1];
#ifdef AAC_INTERNAL_SRAM_FLOW
    nTemp+=1;
    XBuf = psi->XBuf[nTemp][p];
#else
    XBuf += (2*64);
#endif
#ifdef AAC_ENABLE_INTERNAL_SD
    XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
#endif

    //ptr = XBuf;
	buf0 = 0;
	buf1 = 0;
	buf2 = 0;
	buf3 = 0;

    for (n = (NUM_TIME_SLOTS*SAMPLES_PER_SLOT + 6); n != 0; n--) {
        x2re = XBuf[0];
        x2im = XBuf[1];
#ifdef AAC_INTERNAL_SRAM_FLOW
        nTemp+=1;
    #ifdef AAC_ENABLE_INTERNAL_SD
        if (nTemp<8)
            XBuf = psi->XbufTmp1->bufTemp[nTemp][p];
        else if (nTemp<16)
            XBuf = psi->XbufTmp2->bufTemp[nTemp-8][p];
        else
            XBuf = psi->XBuf[nTemp][p];
    #else
        XBuf = psi->XBuf[nTemp][p];
    #endif
#else
        XBuf += (2*64);
#endif

		asm volatile("l.mtspr r0, %0, 0x2801" : : "r"(buf0));
		asm volatile("l.mtspr r0, %0, 0x2802" : : "r"(buf1));
        asm ("l.mac %0, %1" : : "r"(x2re), "r"(x0re));
        asm ("l.mac %0, %1" : : "r"(x2im), "r"(x0im));
		asm volatile("l.mfspr %0, r0, 0x2801" :  "=r"(buf0));
		asm volatile("l.macrc %0, 32"         :  "=r"(buf1));

		asm volatile("l.mtspr r0, %0, 0x2801" : : "r"(buf2));
		asm volatile("l.mtspr r0, %0, 0x2802" : : "r"(buf3));
        asm ("l.mac %0, %1" : : "r"(x0re), "r"(x2im));
        asm ("l.msb %0, %1" : : "r"(x0im), "r"(x2re));
		asm volatile("l.mfspr %0, r0, 0x2801" :  "=r"(buf2));
		asm volatile("l.macrc %0, 32"         :  "=r"(buf3));

        x0re = x1re;
        x0im = x1im;
        x1re = x2re;
        x1im = x2im;
    }
	accBuf[0] = buf0;
	accBuf[1] = buf1;
	accBuf[2] = buf2;
	accBuf[3] = buf3;

}
#endif /* OR32_ASM */

/**************************************************************************************
 * Function:    CalcCovariance2
 *
 * Description: calculate covariance matrix for p02 (4.6.18.6.2)
 *
 * Inputs:      buffer of low-freq samples, starting at time index = 0,
 *                freq index = patch subband
 *
 * Outputs:     complex covariance element p02re, p02im
 *              format = integer (Q0) * 2^N, with scalefactor N >= 0
 *
 * Return:      scalefactor N
 *
 * Notes:       outputs are normalized to have 1 GB (sign in at least top 2 bits)
 **************************************************************************************/
#ifdef AAC_INTERNAL_SRAM_FLOW
static int CalcCovariance2(PSInfoSBR *psi,int p, int *p02reN, int *p02imN)
#else
static int CalcCovariance2(int *XBuf, int *p02reN, int *p02imN)
#endif
{
    U64 p02re, p02im;
    int n, z, s, loShift, hiShift, gbMask;
    int accBuf[2*2];
#ifdef AAC_INTERNAL_SRAM_FLOW
    int *XBuf = psi->XBuf[0][p];
    int nTemp =0;
#endif

#ifdef AAC_INTERNAL_SRAM_FLOW
    CVKernel2(psi,p, accBuf);
#else
    CVKernel2(XBuf, accBuf);
#endif
    p02re.r.lo32 = accBuf[0];
    p02re.r.hi32 = accBuf[1];
    p02im.r.lo32 = accBuf[2];
    p02im.r.hi32 = accBuf[3];

    /* 64-bit accumulators now have 2*FBITS_OUT_QMFA fraction bits
     * want to scale them down to integers (32-bit signed, Q0)
     *   with scale factor of 2^n, n >= 0
     * leave 1 GB for calculating determinant, so take top 30 non-zero bits
     */
    gbMask  = ((p02re.r.hi32) ^ (p02re.r.hi32 >> 31)) | ((p02im.r.hi32) ^ (p02im.r.hi32 >> 31));
    if (gbMask == 0) {
        s = p02re.r.hi32 >> 31; gbMask  = (p02re.r.lo32 ^ s) - s;
        s = p02im.r.hi32 >> 31; gbMask |= (p02im.r.lo32 ^ s) - s;
        z = 32 + CLZ(gbMask);
    } else {
        gbMask  = FASTABS(p02re.r.hi32) | FASTABS(p02im.r.hi32);
        z = CLZ(gbMask);
    }
    n = 64 - z; /* number of non-zero bits in bottom of 64-bit word */

    if (n <= 30) {
        loShift = (30 - n);
        *p02reN = p02re.r.lo32 << loShift;
        *p02imN = p02im.r.lo32 << loShift;
        return -(loShift + 2*FBITS_OUT_QMFA);
    } else if (n < 32 + 30) {
        loShift = (n - 30);
        hiShift = 32 - loShift;
        *p02reN = (p02re.r.hi32 << hiShift) | (p02re.r.lo32 >> loShift);
        *p02imN = (p02im.r.hi32 << hiShift) | (p02im.r.lo32 >> loShift);
        return (loShift - 2*FBITS_OUT_QMFA);
    } else {
        hiShift = n - (32 + 30);
        *p02reN = p02re.r.hi32 >> hiShift;
        *p02imN = p02im.r.hi32 >> hiShift;
        return (32 - 2*FBITS_OUT_QMFA - hiShift);
    }

    return 0;
}

/**************************************************************************************
 * Function:    CalcLPCoefs
 *
 * Description: calculate linear prediction coefficients for one subband (4.6.18.6.2)
 *
 * Inputs:      buffer of low-freq samples, starting at time index = 0,
 *                freq index = patch subband
 *              number of guard bits in input sample buffer
 *
 * Outputs:     complex LP coefficients a0re, a0im, a1re, a1im, format = Q29
 *
 * Return:      none
 *
 * Notes:       output coefficients (a0re, a0im, a1re, a1im) clipped to range (-4, 4)
 *              if the comples coefficients have magnitude >= 4.0, they are all
 *                set to 0 (see spec)
 **************************************************************************************/
#ifdef AAC_INTERNAL_SRAM_FLOW
static void CalcLPCoefs(PSInfoSBR *psi,int p, int *a0re, int *a0im, int *a1re, int *a1im, int gb)
#else
static void CalcLPCoefs(int *XBuf, int *a0re, int *a0im, int *a1re, int *a1im, int gb)
#endif
{
    int zFlag, n1, n2, nd, d, dInv, tre, tim;
    int p01re, p01im, p02re, p02im, p12re, p12im, p11re, p22re;
#ifdef AAC_INTERNAL_SRAM_FLOW
    int *XBuf = psi->XBuf[0][p];
    int nTemp =0;
#endif
    /* pre-scale to avoid overflow - probably never happens in practice (see QMFA)
     *   max bit growth per accumulator = 38*2 = 76 mul-adds (X * X)
     *   using 64-bit MADD, so if X has n guard bits, X*X has 2n+1 guard bits
     *   gain 1 extra sign bit per multiply, so ensure ceil(log2(76/2) / 2) = 3 guard bits on inputs
     */
    if (gb < 3) {
        nd = 3 - gb;
        for (n1 = (NUM_TIME_SLOTS*SAMPLES_PER_SLOT + 6 + 2); n1 != 0; n1--) {
            XBuf[0] >>= nd; XBuf[1] >>= nd;
#ifdef AAC_INTERNAL_SRAM_FLOW
            nTemp+=1;
            XBuf = psi->XBuf[nTemp][p];
#else
            XBuf += (2*64);
#endif
        }
#ifdef AAC_INTERNAL_SRAM_FLOW
        nTemp-=(NUM_TIME_SLOTS*SAMPLES_PER_SLOT + 6 + 2);
        XBuf = psi->XBuf[nTemp][p];
#else
        XBuf -= (2*64*(NUM_TIME_SLOTS*SAMPLES_PER_SLOT + 6 + 2));
#endif
    }

    /* calculate covariance elements */
#ifdef AAC_INTERNAL_SRAM_FLOW
    n1 = CalcCovariance1(psi,p, &p01re, &p01im, &p12re, &p12im, &p11re, &p22re);  // powei 400
    n2 = CalcCovariance2(psi,p, &p02re, &p02im); // powei 400
#else
    n1 = CalcCovariance1(XBuf, &p01re, &p01im, &p12re, &p12im, &p11re, &p22re);  // powei 400
    n2 = CalcCovariance2(XBuf, &p02re, &p02im); // powei 400
#endif

    /* normalize everything to larger power of 2 scalefactor, call it n1 */
    if (n1 < n2) {
        nd = MIN(n2 - n1, 31);
        p01re >>= nd;   p01im >>= nd;
        p12re >>= nd;   p12im >>= nd;
        p11re >>= nd;   p22re >>= nd;
        n1 = n2;
    } else if (n1 > n2) {
        nd = MIN(n1 - n2, 31);
        p02re >>= nd;   p02im >>= nd;
    }

    /* calculate determinant of covariance matrix (at least 1 GB in pXX) */
    d = MULSHIFT32_ADD(p12re, p12re, p12im, p12im);
    d = MULSHIFT32(d, RELAX_COEF) << 1;
    d = MULSHIFT32(p11re, p22re) - d;
    ASSERT(d >= 0); /* should never be < 0 */

    zFlag = 0;
    *a0re = *a0im = 0;
    *a1re = *a1im = 0;
    if (d > 0) {
        /* input =   Q31  d    = Q(-2*n1 - 32 + nd) = Q31 * 2^(31 + 2*n1 + 32 - nd)
         * inverse = Q29  dInv = Q29 * 2^(-31 - 2*n1 - 32 + nd) = Q(29 + 31 + 2*n1 + 32 - nd)
         *
         * numerator has same Q format as d, since it's sum of normalized squares
         * so num * inverse = Q(-2*n1 - 32) * Q(29 + 31 + 2*n1 + 32 - nd)
         *                  = Q(29 + 31 - nd), drop low 32 in MULSHIFT32
         *                  = Q(29 + 31 - 32 - nd) = Q(28 - nd)
         */
        nd = CLZ(d) - 1;
        d <<= nd;
        dInv = InvRNormalized(d);

        /* 1 GB in pXX */
        tre = MULSHIFT32(p01re, p12re) - MULSHIFT32_ADD(p01im, p12im, p02re, p11re);
        tre = MULSHIFT32(tre, dInv);
        tim = MULSHIFT32(p01re, p12im) + MULSHIFT32_SUB(p01im, p12re, p02im, p11re);
        tim = MULSHIFT32(tim, dInv);

        /* if d is extremely small, just set coefs to 0 (would have poor precision anyway) */
        if (nd > 28 || (FASTABS(tre) >> (28 - nd)) >= 4 || (FASTABS(tim) >> (28 - nd)) >= 4) {
            zFlag = 1;
        } else {
            *a1re = tre << (FBITS_LPCOEFS - 28 + nd);   /* i.e. convert Q(28 - nd) to Q(29) */
            *a1im = tim << (FBITS_LPCOEFS - 28 + nd);
        }
    }

    if (p11re) {
        /* input =   Q31  p11re = Q(-n1 + nd) = Q31 * 2^(31 + n1 - nd)
         * inverse = Q29  dInv  = Q29 * 2^(-31 - n1 + nd) = Q(29 + 31 + n1 - nd)
         *
         * numerator is Q(-n1 - 3)
         * so num * inverse = Q(-n1 - 3) * Q(29 + 31 + n1 - nd)
         *                  = Q(29 + 31 - 3 - nd), drop low 32 in MULSHIFT32
         *                  = Q(29 + 31 - 3 - 32 - nd) = Q(25 - nd)
         */
        nd = CLZ(p11re) - 1;    /* assume positive */
        p11re <<= nd;
        dInv = InvRNormalized(p11re);

        /* a1re, a1im = Q29, so scaled by (n1 + 3) */
        tre = (p01re >> 3) + MULSHIFT32_ADD(p12re, *a1re, p12im, *a1im);
        tre = -MULSHIFT32(tre, dInv);
        tim = (p01im >> 3) + MULSHIFT32_SUB(p12re, *a1im, p12im, *a1re);
        tim = -MULSHIFT32(tim, dInv);

        if (nd > 25 || (FASTABS(tre) >> (25 - nd)) >= 4 || (FASTABS(tim) >> (25 - nd)) >= 4) {
            zFlag = 1;
        } else {
            *a0re = tre << (FBITS_LPCOEFS - 25 + nd);   /* i.e. convert Q(25 - nd) to Q(29) */
            *a0im = tim << (FBITS_LPCOEFS - 25 + nd);
        }
    }

    /* see 4.6.18.6.2 - if magnitude of a0 or a1 >= 4 then a0 = a1 = 0
     * i.e. a0re < 4, a0im < 4, a1re < 4, a1im < 4
     * Q29*Q29 = Q26
     */
    if (zFlag || MULSHIFT32_ADD(*a0re, *a0re, *a0im, *a0im) >= MAG_16 || MULSHIFT32_ADD(*a1re, *a1re, *a1im, *a1im) >= MAG_16) {
        *a0re = *a0im = 0;
        *a1re = *a1im = 0;
    }

    /* no need to clip - we never changed the XBuf data, just used it to calculate a0 and a1 */
    if (gb < 3) {
        nd = 3 - gb;
        for (n1 = (NUM_TIME_SLOTS*SAMPLES_PER_SLOT + 6 + 2); n1 != 0; n1--) {
            XBuf[0] <<= nd; XBuf[1] <<= nd;
#ifdef AAC_INTERNAL_SRAM_FLOW
            nTemp+=1;
            XBuf = psi->XBuf[nTemp][p];
#else
            XBuf += (2*64);
#endif
        }
    }
}

/**************************************************************************************
 * Function:    GenerateHighFreq
 *
 * Description: generate high frequencies with SBR (4.6.18.6)
 *
 * Inputs:      initialized PSInfoSBR struct
 *              initialized SBRGrid struct for this channel
 *              initialized SBRFreq struct for this SCE/CPE block
 *              initialized SBRChan struct for this channel
 *              index of current channel (0 for SCE, 0 or 1 for CPE)
 *
 * Outputs:     new high frequency samples starting at frequency kStart
 *
 * Return:      none
 **************************************************************************************/
#if 1
int GenerateHighFreq(PSInfoSBR *psi, SBRGrid *sbrGrid, SBRFreq *sbrFreq, SBRChan *sbrChan, int ch)
{
    int band, newBW, c, t, gb, gbMask, gbIdx;
    int currPatch, p, x, k, g, i, iStart, iEnd, bw, bwsq;
    int a0re, a0im, a1re, a1im;
    int x1re, x1im, x2re, x2im;
    int ACCre, ACCim;
    int *XBufLo, *XBufHi;
    int nTempHi,nTempLo;

    /* calculate array of chirp factors */
    for (band = 0; band < sbrFreq->numNoiseFloorBands; band++) {
        c = sbrChan->chirpFact[band];   /* previous (bwArray') */
        newBW = newBWTab[sbrChan->invfMode[0][band]][sbrChan->invfMode[1][band]];

        /* weighted average of new and old (can't overflow - total gain = 1.0) */
        if (newBW < c)
            t = MULSHIFT32_ADD(newBW, 0x60000000, 0x20000000, c);  /* new is smaller: 0.75*new + 0.25*old */
        else
            t = MULSHIFT32_ADD(newBW, 0x74000000, 0x0c000000, c);  /* new is larger: 0.90625*new + 0.09375*old */
        t <<= 1;

        if (t < 0x02000000) /* below 0.015625, clip to 0 */
            t = 0;
        if (t > 0x7f800000) /* clip to 0.99609375 */
            t = 0x7f800000;

        /* save curr as prev for next time */
        sbrChan->chirpFact[band] = t;
        sbrChan->invfMode[0][band] = sbrChan->invfMode[1][band];
    }

    iStart = sbrGrid->envTimeBorder[0] + HF_ADJ;
    iEnd =   sbrGrid->envTimeBorder[sbrGrid->numEnv] + HF_ADJ;

    /* generate new high freqs from low freqs, patches, and chirp factors */
    k = sbrFreq->kStart;
    g = 0;
    bw = sbrChan->chirpFact[g];
    bwsq = MULSHIFT32(bw, bw) << 1;

    gbMask = (sbrChan->gbMask[0] | sbrChan->gbMask[1]); /* older 32 | newer 8 */
    gb = CLZ(gbMask) - 1;

    for (currPatch = 0; currPatch < sbrFreq->numPatches; currPatch++) {
        for (x = 0; x < sbrFreq->patchNumSubbands[currPatch]; x++) {
            /* map k to corresponding noise floor band */
            if (k >= sbrFreq->freqNoise[g+1]) {
                g++;
                bw = sbrChan->chirpFact[g];     /* Q31 */
                bwsq = MULSHIFT32(bw, bw) << 1; /* Q31 */
            }

            p = sbrFreq->patchStartSubband[currPatch] + x;  /* low QMF band */
#ifdef AAC_ENABLE_INTERNAL_SD
            nTempHi = iStart;
            if (iStart<8)
                XBufHi = psi->XbufTmp1->bufTemp[iStart][k];
            else if (iStart<16)
                XBufHi = psi->XbufTmp2->bufTemp[iStart-8][k];
            else
            XBufHi = psi->XBuf[iStart][k];
#else
            XBufHi = psi->XBuf[iStart][k];
            nTempHi = iStart;
#endif

            if (bw) {
#ifdef AAC_ENABLE_INTERNAL_SD
                CalcLPCoefs(psi,p, &a0re, &a0im, &a1re, &a1im, gb);  // powei 860    
#else
    #ifdef AAC_INTERNAL_SRAM_FLOW
                CalcLPCoefs(psi,p, &a0re, &a0im, &a1re, &a1im, gb);  // powei 860    
    #else
                CalcLPCoefs(psi->XBuf[0][p], &a0re, &a0im, &a1re, &a1im, gb);  // powei 860
    #endif
#endif

                a0re = MULSHIFT32(bw, a0re);    /* Q31 * Q29 = Q28 */
                a0im = MULSHIFT32(bw, a0im);
                a1re = MULSHIFT32(bwsq, a1re);
                a1im = MULSHIFT32(bwsq, a1im);
#ifdef AAC_ENABLE_INTERNAL_SD
            nTempLo = iStart-2;
            if (nTempLo<8)
                XBufLo = psi->XbufTmp1->bufTemp[nTempLo][p];
            else if (nTempLo<16)
                XBufLo = psi->XbufTmp2->bufTemp[nTempLo-8][p];
            else 
                XBufLo = psi->XBuf[nTempLo][p];
#else
                XBufLo = psi->XBuf[iStart-2][p];
                nTempLo = iStart-2;
#endif

                x2re = XBufLo[0];   /* RE{XBuf[n-2]} */
                x2im = XBufLo[1];   /* IM{XBuf[n-2]} */
    #ifdef AAC_INTERNAL_SRAM_FLOW
                nTempLo+=1;
                XBufLo = psi->XBuf[nTempLo][p];
    #else
                XBufLo += (64*2);
    #endif
    #ifdef AAC_ENABLE_INTERNAL_SD
                if (nTempLo<8)
                    XBufLo = psi->XbufTmp1->bufTemp[nTempLo][p];
                else if (nTempLo<16)
                    XBufLo = psi->XbufTmp2->bufTemp[nTempLo-8][p];
                else
                    XBufLo = psi->XBuf[nTempLo][p];
    #endif

                x1re = XBufLo[0];   /* RE{XBuf[n-1]} */
                x1im = XBufLo[1];   /* IM{XBuf[n-1]} */
    #ifdef AAC_INTERNAL_SRAM_FLOW
                nTempLo+=1;
                XBufLo = psi->XBuf[nTempLo][p];
    #else
                XBufLo += (64*2);
    #endif
    #ifdef AAC_ENABLE_INTERNAL_SD
                if (nTempLo<8)
                    XBufLo = psi->XbufTmp1->bufTemp[nTempLo][p];
                else if (nTempLo<16)
                    XBufLo = psi->XbufTmp2->bufTemp[nTempLo-8][p];
                else
                    XBufLo = psi->XBuf[nTempLo][p];
    #endif
                if ( (iEnd-iStart)*128>sizeof(psi->XBuf))
                {
                    return ERR_AAC_SBR_BITSTREAM;
                }
                for (i = iStart; i < iEnd; i++) {
                    /* a0re/im, a1re/im are Q28 with at least 1 GB,
                     *   so the summing for AACre/im is fine (1 GB in, plus 1 from MULSHIFT32)
                     */
                    ACCre = MULSHIFT32_SUB(x2re, a1re, x2im, a1im);
                    ACCim = MULSHIFT32_ADD(x2re, a1im, x2im, a1re);
                    ACCre += MULSHIFT32_SUB(x1re, a0re, x1im, a0im);
                    ACCim += MULSHIFT32_ADD(x1re, a0im, x1im, a0re);

                    x2re = x1re;
                    x2im = x1im;
                    x1re = XBufLo[0];   /* RE{XBuf[n]} */
                    x1im = XBufLo[1];   /* IM{XBuf[n]} */
        #ifdef AAC_INTERNAL_SRAM_FLOW
                    nTempLo+=1;
                    XBufLo = psi->XBuf[nTempLo][p];
        #else
                    XBufLo += (64*2);
        #endif
        #ifdef AAC_ENABLE_INTERNAL_SD
                    if (nTempLo<8)
                        XBufLo = psi->XbufTmp1->bufTemp[nTempLo][p];
                    else if (nTempLo<16)
                        XBufLo = psi->XbufTmp2->bufTemp[nTempLo-8][p];
                    else
                        XBufLo = psi->XBuf[nTempLo][p];
        #endif

                    /* lost 4 fbits when scaling by a0re/im, a1re/im (Q28) */
                    CLIP_2N_SHIFT(ACCre, 4);
                    CLIP_2N_SHIFT(ACCim, 4);
                    ACCre += x1re;
                    ACCim += x1im;

                    XBufHi[0] = ACCre;
                    XBufHi[1] = ACCim;
    #ifdef AAC_INTERNAL_SRAM_FLOW
                    nTempHi+=1;
                    XBufHi = psi->XBuf[nTempHi][k];
    #else
                    XBufHi += (64*2);
    #endif
    #ifdef AAC_ENABLE_INTERNAL_SD
                    if (nTempHi<8)
                        XBufHi = psi->XbufTmp1->bufTemp[nTempHi][k];
                    else if (nTempHi<16)
                        XBufHi = psi->XbufTmp2->bufTemp[nTempHi-8][k];
                    else
                        XBufHi = psi->XBuf[nTempHi][k];                        
    #endif

                    /* update guard bit masks */
                    gbMask  = FASTABS(ACCre);
                    gbMask |= FASTABS(ACCim);
                    gbIdx = (i >> 5) & 0x01;    /* 0 if i < 32, 1 if i >= 32 */
                    sbrChan->gbMask[gbIdx] |= gbMask;
                }
            } else {
#ifdef AAC_ENABLE_INTERNAL_SD
                nTempLo = iStart;
                if (iStart<8)
                    XBufLo = psi->XbufTmp1->bufTemp[iStart][p];
                else if (iStart<16)
                    XBufLo = psi->XbufTmp2->bufTemp[iStart-8][p];
            else
                    XBufLo = psi->XBuf[iStart][p];
#else
                XBufLo = (int *)psi->XBuf[iStart][p];
                nTempLo = iStart;
#endif
                if ( (iEnd-iStart)*128>sizeof(psi->XBuf))
                    return ERR_AAC_SBR_BITSTREAM;

                for (i = iStart; i < iEnd; i++) {
                    XBufHi[0] = XBufLo[0];
                    XBufHi[1] = XBufLo[1];
    #ifdef AAC_INTERNAL_SRAM_FLOW
                    nTempLo+=1;
                    XBufLo = psi->XBuf[nTempLo][p];
    #else
                    XBufLo += (64*2);
    #endif
    #ifdef AAC_ENABLE_INTERNAL_SD
                    if (nTempLo<8)
                        XBufLo = psi->XbufTmp1->bufTemp[nTempLo][p];
                    else if (nTempLo<16)
                        XBufLo = psi->XbufTmp2->bufTemp[nTempLo-8][p];
                    else
                        XBufLo = psi->XBuf[nTempLo][p];
    #endif

    #ifdef AAC_INTERNAL_SRAM_FLOW
                    nTempHi+=1;
                    XBufHi = psi->XBuf[nTempHi][k];    
    #else
                    XBufHi += (64*2);
    #endif
    #ifdef AAC_ENABLE_INTERNAL_SD
                    if (nTempHi<8)
                        XBufHi = psi->XbufTmp1->bufTemp[nTempHi][k];
                    else if (nTempHi<16)
                        XBufHi = psi->XbufTmp2->bufTemp[nTempHi-8][k];
                    else
                        XBufHi = psi->XBuf[nTempHi][k];                        
    #endif
                }
                }
            k++;    /* high QMF band */
                }
    }
}
#else
                
void GenerateHighFreq(PSInfoSBR *psi, SBRGrid *sbrGrid, SBRFreq *sbrFreq, SBRChan *sbrChan, int ch)
                {
    int band, newBW, c, t, gb, gbMask, gbIdx;
    int currPatch, p, x, k, g, i, iStart, iEnd, bw, bwsq;
    int a0re, a0im, a1re, a1im;
    int x1re, x1im, x2re, x2im;
    int ACCre, ACCim;
    int *XBufLo, *XBufHi;

    /* calculate array of chirp factors */
    for (band = 0; band < sbrFreq->numNoiseFloorBands; band++) {
        c = sbrChan->chirpFact[band];   /* previous (bwArray') */
        newBW = newBWTab[sbrChan->invfMode[0][band]][sbrChan->invfMode[1][band]];

        /* weighted average of new and old (can't overflow - total gain = 1.0) */
        if (newBW < c)
            t = MULSHIFT32_ADD(newBW, 0x60000000, 0x20000000, c);  /* new is smaller: 0.75*new + 0.25*old */
        else
            t = MULSHIFT32_ADD(newBW, 0x74000000, 0x0c000000, c);  /* new is larger: 0.90625*new + 0.09375*old */
        t <<= 1;

        if (t < 0x02000000) /* below 0.015625, clip to 0 */
            t = 0;
        if (t > 0x7f800000) /* clip to 0.99609375 */
            t = 0x7f800000;

        /* save curr as prev for next time */
        sbrChan->chirpFact[band] = t;
        sbrChan->invfMode[0][band] = sbrChan->invfMode[1][band];
    }

    iStart = sbrGrid->envTimeBorder[0] + HF_ADJ;
    iEnd =   sbrGrid->envTimeBorder[sbrGrid->numEnv] + HF_ADJ;

    /* generate new high freqs from low freqs, patches, and chirp factors */
    k = sbrFreq->kStart;
    g = 0;
    bw = sbrChan->chirpFact[g];
    bwsq = MULSHIFT32(bw, bw) << 1;

    gbMask = (sbrChan->gbMask[0] | sbrChan->gbMask[1]); /* older 32 | newer 8 */
    gb = CLZ(gbMask) - 1;

    for (currPatch = 0; currPatch < sbrFreq->numPatches; currPatch++) {
        for (x = 0; x < sbrFreq->patchNumSubbands[currPatch]; x++) {
            /* map k to corresponding noise floor band */
            if (k >= sbrFreq->freqNoise[g+1]) {
                g++;
                bw = sbrChan->chirpFact[g];     /* Q31 */
                bwsq = MULSHIFT32(bw, bw) << 1; /* Q31 */
            }

            p = sbrFreq->patchStartSubband[currPatch] + x;  /* low QMF band */
            XBufHi = psi->XBuf[iStart][k];
            if (bw) {
                CalcLPCoefs(psi->XBuf[0][p], &a0re, &a0im, &a1re, &a1im, gb);  // powei 860

                a0re = MULSHIFT32(bw, a0re);    /* Q31 * Q29 = Q28 */
                a0im = MULSHIFT32(bw, a0im);
                a1re = MULSHIFT32(bwsq, a1re);
                a1im = MULSHIFT32(bwsq, a1im);

                XBufLo = psi->XBuf[iStart-2][p];

                x2re = XBufLo[0];   /* RE{XBuf[n-2]} */
                x2im = XBufLo[1];   /* IM{XBuf[n-2]} */
                XBufLo += (64*2);

                x1re = XBufLo[0];   /* RE{XBuf[n-1]} */
                x1im = XBufLo[1];   /* IM{XBuf[n-1]} */
                XBufLo += (64*2);

                for (i = iStart; i < iEnd; i++) {
                    /* a0re/im, a1re/im are Q28 with at least 1 GB,
                     *   so the summing for AACre/im is fine (1 GB in, plus 1 from MULSHIFT32)
                     */
                    ACCre = MULSHIFT32_SUB(x2re, a1re, x2im, a1im);
                    ACCim = MULSHIFT32_ADD(x2re, a1im, x2im, a1re);
                    ACCre += MULSHIFT32_SUB(x1re, a0re, x1im, a0im);
                    ACCim += MULSHIFT32_ADD(x1re, a0im, x1im, a0re);

                    x2re = x1re;
                    x2im = x1im;
                    x1re = XBufLo[0];   /* RE{XBuf[n]} */
                    x1im = XBufLo[1];   /* IM{XBuf[n]} */
                    XBufLo += (64*2);

                    /* lost 4 fbits when scaling by a0re/im, a1re/im (Q28) */
                    CLIP_2N_SHIFT(ACCre, 4);
                    CLIP_2N_SHIFT(ACCim, 4);
                    ACCre += x1re;
                    ACCim += x1im;

                    XBufHi[0] = ACCre;
                    XBufHi[1] = ACCim;
                    XBufHi += (64*2);

                    /* update guard bit masks */
                    gbMask  = FASTABS(ACCre);
                    gbMask |= FASTABS(ACCim);
                    gbIdx = (i >> 5) & 0x01;    /* 0 if i < 32, 1 if i >= 32 */
                    sbrChan->gbMask[gbIdx] |= gbMask;
                }
            } else {
                XBufLo = (int *)psi->XBuf[iStart][p];
                for (i = iStart; i < iEnd; i++) {
                    XBufHi[0] = XBufLo[0];
                    XBufHi[1] = XBufLo[1];
                    XBufLo += (64*2);
                    XBufHi += (64*2);
                }
            }
            k++;    /* high QMF band */
        }
    }
    return ERR_AAC_NONE;
}
#endif

#endif /* defined(AAC_ENABLE_SBR) */

