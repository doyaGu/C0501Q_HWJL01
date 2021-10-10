
#include "mp3_config.h"

typedef int ARRAY3[3];  /* for short-block reordering */

/* optional pre-emphasis for high-frequency scale factor bands */
static const char preTab[22] = { 0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,3,3,3,2,0 };

/* pow(2,-i/4) for i=0..3, Q31 format */
static const int pow14[4] = {
    0x7fffffff, 0x6ba27e65, 0x5a82799a, 0x4c1bf829
};

/* pow(2,-i/4) * pow(j,4/3) for i=0..3 j=0..15, Q25 format */
static const int pow43_14[4][16] = {
{   0x00000000, 0x10000000, 0x285145f3, 0x453a5cdb, /* Q28 */
    0x0cb2ff53, 0x111989d6, 0x15ce31c8, 0x1ac7f203,
    0x20000000, 0x257106b9, 0x2b16b4a3, 0x30ed74b4,
    0x36f23fa5, 0x3d227bd3, 0x437be656, 0x49fc823c, },

{   0x00000000, 0x0d744fcd, 0x21e71f26, 0x3a36abd9,
    0x0aadc084, 0x0e610e6e, 0x12560c1d, 0x168523cf,
    0x1ae89f99, 0x1f7c03a4, 0x243bae49, 0x29249c67,
    0x2e34420f, 0x33686f85, 0x38bf3dff, 0x3e370182, },

{   0x00000000, 0x0b504f33, 0x1c823e07, 0x30f39a55,
    0x08facd62, 0x0c176319, 0x0f6b3522, 0x12efe2ad,
    0x16a09e66, 0x1a79a317, 0x1e77e301, 0x2298d5b4,
    0x26da56fc, 0x2b3a902a, 0x2fb7e7e7, 0x3450f650, },

{   0x00000000, 0x09837f05, 0x17f910d7, 0x2929c7a9,
    0x078d0dfa, 0x0a2ae661, 0x0cf73154, 0x0fec91cb,
    0x1306fe0a, 0x16434a6c, 0x199ee595, 0x1d17ae3d,
    0x20abd76a, 0x2459d551, 0x28204fbb, 0x2bfe1808, },
};

/* pow(j,4/3) for j=16..63, Q23 format */
static const int pow43[] = {
    0x1428a2fa, 0x15db1bd6, 0x1796302c, 0x19598d85,
    0x1b24e8bb, 0x1cf7fcfa, 0x1ed28af2, 0x20b4582a,
    0x229d2e6e, 0x248cdb55, 0x26832fda, 0x28800000,
    0x2a832287, 0x2c8c70a8, 0x2e9bc5d8, 0x30b0ff99,
    0x32cbfd4a, 0x34eca001, 0x3712ca62, 0x393e6088,
    0x3b6f47e0, 0x3da56717, 0x3fe0a5fc, 0x4220ed72,
    0x44662758, 0x46b03e7c, 0x48ff1e87, 0x4b52b3f3,
    0x4daaebfd, 0x5007b497, 0x5268fc62, 0x54ceb29c,
    0x5738c721, 0x59a72a59, 0x5c19cd35, 0x5e90a129,
    0x610b9821, 0x638aa47f, 0x660db90f, 0x6894c90b,
    0x6b1fc80c, 0x6daeaa0d, 0x70416360, 0x72d7e8b0,
    0x75722ef9, 0x78102b85, 0x7ab1d3ec, 0x7d571e09,
};

/* sqrt(0.5) in Q31 format */
#define SQRTHALF 0x5a82799a

/*
 * Minimax polynomial approximation to pow(x, 4/3), over the range
 *  poly43lo: x = [0.5, 0.7071]
 *  poly43hi: x = [0.7071, 1.0]
 *
 * Relative error < 1E-7
 * Coefs are scaled by 4, 2, 1, 0.5, 0.25
 */
static const int poly43lo[5] = { 0x29a0bda9, 0xb02e4828, 0x5957aa1b, 0x236c498d, 0xff581859 };
static const int poly43hi[5] = { 0x10852163, 0xd333f6a4, 0x46e9408b, 0x27c2cef0, 0xfef577b4 };

/* pow(2, i*4/3) as exp and frac */
static const int pow2exp[8]  = { 14, 13, 11, 10, 9, 7, 6, 5 };

static const int pow2frac[8] = {
    0x6597fa94, 0x50a28be6, 0x7fffffff, 0x6597fa94,
    0x50a28be6, 0x7fffffff, 0x6597fa94, 0x50a28be6
};

/**************************************************************************************
 * Function:    DequantBlock
 *
 * Description: Ken's highly-optimized, low memory dequantizer performing the operation
 *              y = pow(x, 4.0/3.0) * pow(2, 25 - scale/4.0)
 *
 * Inputs:      input buffer of decode Huffman codewords (signed-magnitude)
 *              output buffer of same length (in-place (outbuf = inbuf) is allowed)
 *              number of samples
 *
 * Outputs:     dequantized samples in Q25 format
 *
 * Return:      bitwise-OR of the unsigned outputs (for guard bit calculations)
 **************************************************************************************/
int DequantBlock(int *inbuf, int *outbuf, int num, int scale)
{
    int tab4[4];
    int scalef, scalei, shift;
    int sx, x, y;
    int mask = 0;
    const int *tab16, *coef;

    tab16 = pow43_14[scale & 0x3];
    scalef = pow14[scale & 0x3];
    scalei = MIN(scale >> 2, 31);   /* smallest input scale = -47, so smallest scalei = -12 */

    /* cache first 4 values */
    shift = MIN(scalei + 3, 31);
    shift = MAX(shift, 0);
    tab4[0] = 0;
    tab4[1] = tab16[1] >> shift;
    tab4[2] = tab16[2] >> shift;
    tab4[3] = tab16[3] >> shift;

    do {

        sx = *inbuf++;
        x = sx & 0x7fffffff;    /* sx = sign|mag */

        if (x < 4) {

            y = tab4[x];

        } else if (x < 16) {

            y = tab16[x];
            y = (scalei < 0) ? y << -scalei : y >> scalei;

        } else {

            if (x < 64) {

                y = pow43[x-16];

                /* fractional scale */
                y = MULSHIFT32(y, scalef);
                shift = scalei - 3;

            } else {

                /* normalize to [0x40000000, 0x7fffffff] */
                x <<= 17;
                shift = 0;
                if (x < 0x08000000)
                    x <<= 4, shift += 4;
                if (x < 0x20000000)
                    x <<= 2, shift += 2;
                if (x < 0x40000000)
                    x <<= 1, shift += 1;

                coef = (x < SQRTHALF) ? poly43lo : poly43hi;

                /* polynomial */
                y = coef[0];
                y = MULSHIFT32(y, x) + coef[1];
                y = MULSHIFT32(y, x) + coef[2];
                y = MULSHIFT32(y, x) + coef[3];
                y = MULSHIFT32(y, x) + coef[4];

//              y = MULSHIFT32(y, pow2frac[shift]) << 3;
                y = MULSHIFT(y, pow2frac[shift], 32-3);

                /* fractional scale */
                y = MULSHIFT32(y, scalef);
                shift = scalei - pow2exp[shift];
            }

            /* integer scale */
            if (shift < 0) {
                shift = -shift;
                if (y > (0x7fffffff >> shift))
                    y = 0x7fffffff;     /* clip */
                else
                    y <<= shift;
            } else {
                y >>= shift;
            }
        }

        /* sign and store */
        mask |= y;
        *outbuf++ = (sx < 0) ? -y : y;

    } while (--num);

    return mask;
}

/**************************************************************************************
 * Function:    DequantChannel
 *
 * Description: dequantize one granule, one channel worth of decoded Huffman codewords
 *
 * Inputs:      sample buffer (decoded Huffman codewords), length = MAX_NSAMP samples
 *              work buffer for reordering short-block, length = MAX_REORDER_SAMPS
 *                samples (3 * width of largest short-block critical band)
 *              non-zero bound for this channel/granule
 *              valid FrameHeader, SideInfoSub, ScaleFactorInfoSub, and CriticalBandInfo
 *                structures for this channel/granule
 *
 * Outputs:     MAX_NSAMP dequantized samples in sampleBuf
 *              updated non-zero bound (indicating which samples are != 0 after DQ)
 *              filled-in cbi structure indicating start and end critical bands
 *
 * Return:      minimum number of guard bits in dequantized sampleBuf
 *
 * Notes:       dequantized samples in Q(DQ_FRACBITS_OUT) format
 **************************************************************************************/
static int DequantChannel(int *sampleBuf, int *workBuf, int *nonZeroBound, FrameHeader *fh, SideInfoSub *sis,
                    ScaleFactorInfoSub *sfis, CriticalBandInfo *cbi)
{
    int i, j, w, cb;
    int cbStartL, cbEndL, cbStartS, cbEndS;     /* yiwei, critical band start/end, long/short */
    int nSamps, nonZero, sfactMultiplier, gbMask;
    int globalGain, gainI;
    int cbMax[3];
    ARRAY3 *buf;    /* short block reorder */

    /* set default start/end points for short/long blocks - will update with non-zero cb info */
    if (sis->blockType == 2) {
        cbStartL = 0;
        if (sis->mixedBlock) {
            cbEndL = (fh->ver == MPEG1 ? 8 : 6);
            cbStartS = 3;
        } else {
            cbEndL = 0;
            cbStartS = 0;
        }
        cbEndS = 13;
    } else {
        /* long block */
        cbStartL = 0;
        cbEndL =   22;
        cbStartS = 13;
        cbEndS =   13;
    }
    cbMax[2] = cbMax[1] = cbMax[0] = 0;
    gbMask = 0;
    i = 0;

    /* sfactScale = 0 --> quantizer step size = sqrt(2) , yiwei
     * sfactScale = 1 --> quantizer step size = 2       , yiwei
     *   so sfactMultiplier = 2 or 4 (jump through globalGain by powers of 2 or sqrt(2))
     */
    sfactMultiplier = 2 * (sis->sfactScale + 1);

    /* offset globalGain by -2 if midSide enabled, for 1/sqrt(2) used in MidSideProc()
     *  (DequantBlock() does 0.25 * gainI so knocking it down by two is the same as
     *   dividing every sample by sqrt(2) = multiplying by 2^-.5)
     */
    globalGain = sis->globalGain;
    if (fh->modeExt >> 1)
         globalGain -= 2;

//  yiwei, because we use normal algorithm, we do not scale everything by sqrt(2)
//  globalGain += IMDCT_SCALE;      /* scale everything by sqrt(2), for fast IMDCT36 */

    /* long blocks */
    for (cb = 0; cb < cbEndL; cb++) {

        nonZero = 0;
        nSamps = fh->sfBand->l[cb + 1] - fh->sfBand->l[cb];
        gainI = 210 - globalGain + sfactMultiplier * (sfis->l[cb] + (sis->preFlag ? (int)preTab[cb] : 0));

        nonZero |= DequantBlock(sampleBuf + i, sampleBuf + i, nSamps, gainI);
        i += nSamps;

        /* update highest non-zero critical band */
        if (nonZero)
            cbMax[0] = cb;
        gbMask |= nonZero;

        if (i >= *nonZeroBound)
            break;
    }

    /* set cbi (Type, EndS[], EndSMax will be overwritten if we proceed to do short blocks) */
    cbi->cbType = 0;            /* long only */
    cbi->cbEndL  = cbMax[0];
    cbi->cbEndS[0] = cbi->cbEndS[1] = cbi->cbEndS[2] = 0;
    cbi->cbEndSMax = 0;

    /* early exit if no short blocks */
    if (cbStartS >= 12)
        return CLZ(gbMask) - 1;

    /* short blocks */
    cbMax[2] = cbMax[1] = cbMax[0] = cbStartS;
    for (cb = cbStartS; cb < cbEndS; cb++) {

        nSamps = fh->sfBand->s[cb + 1] - fh->sfBand->s[cb];
        for (w = 0; w < 3; w++) {
            nonZero =  0;
            gainI = 210 - globalGain + 8*sis->subBlockGain[w] + sfactMultiplier*(sfis->s[cb][w]);

            nonZero |= DequantBlock(sampleBuf + i + nSamps*w, workBuf + nSamps*w, nSamps, gainI);

            /* update highest non-zero critical band */
            if (nonZero)
                cbMax[w] = cb;
            gbMask |= nonZero;
        }

        /* reorder blocks */
        buf = (ARRAY3 *)(sampleBuf + i);
        i += 3*nSamps;
        for (j = 0; j < nSamps; j++) {
            buf[j][0] = workBuf[0*nSamps + j];
            buf[j][1] = workBuf[1*nSamps + j];
            buf[j][2] = workBuf[2*nSamps + j];
        }

        ASSERT(3*nSamps <= MAX_REORDER_SAMPS);

        if (i >= *nonZeroBound)
            break;
    }

    /* i = last non-zero INPUT sample processed, which corresponds to highest possible non-zero
     *     OUTPUT sample (after reorder)
     * however, the original nzb is no longer necessarily true
     *   for each cb, buf[][] is updated with 3*nSamps samples (i increases 3*nSamps each time)
     *   (buf[j + 1][0] = 3 (input) samples ahead of buf[j][0])
     * so update nonZeroBound to i
     */
    *nonZeroBound = i;

    ASSERT(*nonZeroBound <= MAX_NSAMP);

    cbi->cbType = (sis->mixedBlock ? 2 : 1);    /* 2 = mixed short/long, 1 = short only */

    cbi->cbEndS[0] = cbMax[0];
    cbi->cbEndS[1] = cbMax[1];
    cbi->cbEndS[2] = cbMax[2];

    cbi->cbEndSMax = cbMax[0];
    cbi->cbEndSMax = MAX(cbi->cbEndSMax, cbMax[1]);
    cbi->cbEndSMax = MAX(cbi->cbEndSMax, cbMax[2]);

    return CLZ(gbMask) - 1;
}

/**************************************************************************************
 * Function:    Dequantize
 *
 * Description: dequantize coefficients (two granule-worth)
 *
 * Inputs:      MP3DecInfo structure filled by UnpackFrameHeader(), UnpackSideInfo(),
 *                UnpackScaleFactors(), and DecodeHuffman() (for this granule, total 2)
 *
 * Outputs:     dequantized coefficients in hi->huffDecBuf
 *                (total two granule-worth, all channels), format = Q26
 *              operates in-place on huffDecBuf but also needs di->workBuf
 *              updated hi->nonZeroBound index for both channels
 *
 * Notes:    1. In calling output Q(DQ_FRACBITS_OUT), we assume an implicit bias
 *                of 2^15. Some (floating-point) reference implementations factor this
 *                into the 2^(0.25 * gain) scaling explicitly. But to avoid precision
 *                loss, we don't do that. Instead take it into account in the final
 *                round to PCM (>> by 15 less than we otherwise would have).
 *              Equivalently, we can think of the dequantized coefficients as
 *                Q(DQ_FRACBITS_OUT - 15) with no implicit bias.
 *           2. yiwei: this function is modified from int Dequantize(int gr) from dequant.c
 *                We partition Dequantize(gr) into DequantizeGr() and ProcessStereoGr()
 *                to increase instruction cache hit rate
 **************************************************************************************/

void DequantizeGr()
{
    int i, gr, ch;
    FrameHeader *fh;
    SideInfo *si;
    ScaleFactorInfo *sfi;
    HuffmanInfo *hi;
    DequantInfo *di;
    CriticalBandInfo *cbi;
    int gb[MAX_NCHAN];  /* minimum number of guard bits in huffDecBuf[ch] */

    fh = &(mp3DecInfo.FrameHeaderPS);

    /* si is an array of up to 4 structs, stored as gr0ch0, gr0ch1, gr1ch0, gr1ch1 */
    si = &(mp3DecInfo.SideInfoPS);
    sfi = &(mp3DecInfo.ScaleFactorInfoPS);
    hi = &(mp3DecInfo.HuffmanInfoPS);
    di = &(mp3DecInfo.DequantInfoPS);
    cbi = di->cbi;

    for (gr = 0; gr < mp3DecInfo.nGrans; gr++) {
        /* dequantize all the samples in each channel */
        for (ch = 0; ch < mp3DecInfo.nChans; ch++) {
            gb[ch] = DequantChannel(hi->huffDecBuf[gr][ch], di->workBuf, &hi->nonZeroBound[gr][ch], fh,
                &si->sis[gr][ch], &sfi->sfis[gr][ch], &cbi[ch]);
        }

        /* joint stereo processing assumes one guard bit in input samples
         * it's extremely rare not to have at least one gb, so if this is the case
         *   just make a pass over the data and clip to [-2^30+1, 2^30-1]
         * in practice this may never happen
         */
        // yiwei, by experiment, the input seems always S0.26 format and the following saturation code can be removed...
        if (fh->modeExt && (gb[0] < 1 || gb[1] < 1)) {
            for (i = 0; i < hi->nonZeroBound[gr][0]; i++) {
                if (hi->huffDecBuf[gr][0][i] < -0x3fffffff)  hi->huffDecBuf[gr][0][i] = -0x3fffffff;
                if (hi->huffDecBuf[gr][0][i] >  0x3fffffff)  hi->huffDecBuf[gr][0][i] =  0x3fffffff;
            }
            for (i = 0; i < hi->nonZeroBound[gr][1]; i++) {
                if (hi->huffDecBuf[gr][1][i] < -0x3fffffff)  hi->huffDecBuf[gr][1][i] = -0x3fffffff;
                if (hi->huffDecBuf[gr][1][i] >  0x3fffffff)  hi->huffDecBuf[gr][1][i] =  0x3fffffff;
            }
        }
    }   // end of gr
}
