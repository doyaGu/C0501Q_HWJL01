#include "mp3_config.h"

//#define CODEADDR_ADJUST

// note: we should shift right 31 bits after performing imdct because
//      imdct table format is S.31. From experiments, we know the output
//      of imdct has gained one more integer bit. To avoid overflow, we
//      shift right 32 bits.
// PS.  we can not adjust accuracy at the point of Antialias because
//      Antialis only perform at long block

#define AASHIFT     31
#define IMDCTSHIFT  32
#define WINSHIFT    30
#define BYPASS_CONST    0x40000000  // added in PHASE 7

//static void AntiAlias(int *x, int nBfly);
void IMDCT12x3(int *fsIn, int *xPrev, int *tsOut, int *rawout, int sb);
static void IMDCT36(int *fsIn, int *xPrev, int *tsOut, int *rawout, int sb, int bt);
//static __inline void HybridTransform(int *xCurr, int *xPrev, int y[BLOCK_SIZE][NBANDS], int *rawout, SideInfoSub *sis, int nBlocksTotal);

/**************************************************************************************
 * Function:    AntiAlias
 *
 * Description: smooth transition across DCT block boundaries (every 18 coefficients)
 *
 * Inputs:      vector of dequantized coefficients, length = (nBfly+1) * 18
 *              number of "butterflies" to perform (one butterfly means one
 *                inter-block smoothing operation)
 *
 * Outputs:     updated coefficient vector x
 *
 * Return:      none
 *
 * Notes:       weighted average of opposite bands (pairwise) from the 8 samples
 *                before and after each block boundary
 *              nBlocks = (nonZeroBound + 7) / 18, since nZB is the first ZERO sample
 *                above which all other samples are also zero
 *              max gain per sample = 1.372
 *                MAX(i) (abs(csa[i][0]) + abs(csa[i][1]))
 *              bits gained = 0
 *              assume at least 1 guard bit in x[] to avoid overflow
 *                (should be guaranteed from dequant, and max gain from stproc * max
 *                 gain from AntiAlias < 2.0)
 **************************************************************************************/
 #include "imdct_tab.h"
#if 0
static void IMDCT36(int *fsIn, int *xPrev, int *tsOut, int *rawout, int sb, int bt)
{
    int i,j;
    int reg0, reg1, reg2, reg3;
    int tablebase, *rawoutbase;
    int *windowa, *windowb;

    tablebase=0;
    rawoutbase = rawout;

	for (j=0; j<2; j++)
	{
		for (i=0; i<2; i++) 
		{
			reg0 = reg1 = reg2 = reg3 = 0;
			WRACC64_ADD(fsIn[0], costab36_0[tablebase+0]);
			WRACC64_ADD(fsIn[1], costab36_0[tablebase+1]);
			WRACC64_ADD(fsIn[2], costab36_0[tablebase+2]);
			WRACC64_ADD(fsIn[3], costab36_0[tablebase+3]);
			WRACC64_ADD(fsIn[4], costab36_0[tablebase+4]);
			WRACC64_ADD(fsIn[5], costab36_0[tablebase+5]);
			WRACC64_ADD(fsIn[6], costab36_0[tablebase+6]);
			WRACC64_ADD(fsIn[7], costab36_0[tablebase+7]);
			WRACC64_ADD(fsIn[8], costab36_0[tablebase+8]);
			WRACC64_ADD(fsIn[9], costab36_0[tablebase+9]);
			WRACC64_ADD(fsIn[10], costab36_0[tablebase+10]);
			WRACC64_ADD(fsIn[11], costab36_0[tablebase+11]);
			WRACC64_ADD(fsIn[12], costab36_0[tablebase+12]);
			WRACC64_ADD(fsIn[13], costab36_0[tablebase+13]);
			WRACC64_ADD(fsIn[14], costab36_0[tablebase+14]);
			WRACC64_ADD(fsIn[15], costab36_0[tablebase+15]);
			WRACC64_ADD(fsIn[16], costab36_0[tablebase+16]);
			WRACC64_ADD(fsIn[17], costab36_0[tablebase+17]);
			reg0 = RDACC64_SHIFT(IMDCTSHIFT);
			WRACC64_SUB(fsIn[0], costab36_1[tablebase+0]);
			WRACC64_SUB(fsIn[1], costab36_1[tablebase+1]);
			WRACC64_SUB(fsIn[2], costab36_1[tablebase+2]);
			WRACC64_SUB(fsIn[3], costab36_1[tablebase+3]);
			WRACC64_SUB(fsIn[4], costab36_1[tablebase+4]);
			WRACC64_SUB(fsIn[5], costab36_1[tablebase+5]);
			WRACC64_SUB(fsIn[6], costab36_1[tablebase+6]);
			WRACC64_SUB(fsIn[7], costab36_1[tablebase+7]);
			WRACC64_SUB(fsIn[8], costab36_1[tablebase+8]);
			WRACC64_SUB(fsIn[9], costab36_1[tablebase+9]);
			WRACC64_SUB(fsIn[10], costab36_1[tablebase+10]);
			WRACC64_SUB(fsIn[11], costab36_1[tablebase+11]);
			WRACC64_SUB(fsIn[12], costab36_1[tablebase+12]);
			WRACC64_SUB(fsIn[13], costab36_1[tablebase+13]);
			WRACC64_SUB(fsIn[14], costab36_1[tablebase+14]);
			WRACC64_SUB(fsIn[15], costab36_1[tablebase+15]);
			WRACC64_SUB(fsIn[16], costab36_1[tablebase+16]);
			WRACC64_SUB(fsIn[17], costab36_1[tablebase+17]);
			reg1 = RDACC64_SHIFT(IMDCTSHIFT);
			WRACC64_ADD(fsIn[0], costab36_2[tablebase+0]);
			WRACC64_ADD(fsIn[1], costab36_2[tablebase+1]);
			WRACC64_ADD(fsIn[2], costab36_2[tablebase+2]);
			WRACC64_ADD(fsIn[3], costab36_2[tablebase+3]);
			WRACC64_ADD(fsIn[4], costab36_2[tablebase+4]);
			WRACC64_ADD(fsIn[5], costab36_2[tablebase+5]);
			WRACC64_ADD(fsIn[6], costab36_2[tablebase+6]);
			WRACC64_ADD(fsIn[7], costab36_2[tablebase+7]);
			WRACC64_ADD(fsIn[8], costab36_2[tablebase+8]);
			WRACC64_ADD(fsIn[9], costab36_2[tablebase+9]);
			WRACC64_ADD(fsIn[10], costab36_2[tablebase+10]);
			WRACC64_ADD(fsIn[11], costab36_2[tablebase+11]);
			WRACC64_ADD(fsIn[12], costab36_2[tablebase+12]);
			WRACC64_ADD(fsIn[13], costab36_2[tablebase+13]);
			WRACC64_ADD(fsIn[14], costab36_2[tablebase+14]);
			WRACC64_ADD(fsIn[15], costab36_2[tablebase+15]);
			WRACC64_ADD(fsIn[16], costab36_2[tablebase+16]);
			WRACC64_ADD(fsIn[17], costab36_2[tablebase+17]);
			reg2 = RDACC64_SHIFT(IMDCTSHIFT);
			WRACC64_SUB(fsIn[0], costab36_3[tablebase+0]);
			WRACC64_SUB(fsIn[1], costab36_3[tablebase+1]);
			WRACC64_SUB(fsIn[2], costab36_3[tablebase+2]);
			WRACC64_SUB(fsIn[3], costab36_3[tablebase+3]);
			WRACC64_SUB(fsIn[4], costab36_3[tablebase+4]);
			WRACC64_SUB(fsIn[5], costab36_3[tablebase+5]);
			WRACC64_SUB(fsIn[6], costab36_3[tablebase+6]);
			WRACC64_SUB(fsIn[7], costab36_3[tablebase+7]);
			WRACC64_SUB(fsIn[8], costab36_3[tablebase+8]);
			WRACC64_SUB(fsIn[9], costab36_3[tablebase+9]);
			WRACC64_SUB(fsIn[10], costab36_3[tablebase+10]);
			WRACC64_SUB(fsIn[11], costab36_3[tablebase+11]);
			WRACC64_SUB(fsIn[12], costab36_3[tablebase+12]);
			WRACC64_SUB(fsIn[13], costab36_3[tablebase+13]);
			WRACC64_SUB(fsIn[14], costab36_3[tablebase+14]);
			WRACC64_SUB(fsIn[15], costab36_3[tablebase+15]);
			WRACC64_SUB(fsIn[16], costab36_3[tablebase+16]);
			WRACC64_SUB(fsIn[17], costab36_3[tablebase+17]);
			reg3 = RDACC64_SHIFT(IMDCTSHIFT);

/*			for (k=0; k<18; k++) 
			{
				reg0 = ADD(reg0, MULSHIFT(fsIn[k], costab36_0[tablebase+k], IMDCTSHIFT));
				reg1 = SUB(reg1, MULSHIFT(fsIn[k], costab36_1[tablebase+k], IMDCTSHIFT));
				reg2 = ADD(reg2, MULSHIFT(fsIn[k], costab36_2[tablebase+k], IMDCTSHIFT));
				reg3 = SUB(reg3, MULSHIFT(fsIn[k], costab36_3[tablebase+k], IMDCTSHIFT));
			}
*/
			*rawoutbase = reg0;
			*(rawoutbase+1) = reg1;
			*(rawoutbase+2) = reg2;
			*(rawoutbase+3) = reg3;
			rawoutbase+=4;
			tablebase+=18;
		}   // after j=0, rawoutbase +=8;
		rawoutbase +=10;
	}

	reg0 = reg2 = 0;
	WRACC64_ADD(fsIn[0], costab36_0[72+0]);
	WRACC64_ADD(fsIn[1], costab36_0[72+1]);
	WRACC64_ADD(fsIn[2], costab36_0[72+2]);
	WRACC64_ADD(fsIn[3], costab36_0[72+3]);
	WRACC64_ADD(fsIn[4], costab36_0[72+4]);
	WRACC64_ADD(fsIn[5], costab36_0[72+5]);
	WRACC64_ADD(fsIn[6], costab36_0[72+6]);
	WRACC64_ADD(fsIn[7], costab36_0[72+7]);
	WRACC64_ADD(fsIn[8], costab36_0[72+8]);
	WRACC64_ADD(fsIn[9], costab36_0[72+9]);
	WRACC64_ADD(fsIn[10], costab36_0[72+10]);
	WRACC64_ADD(fsIn[11], costab36_0[72+11]);
	WRACC64_ADD(fsIn[12], costab36_0[72+12]);
	WRACC64_ADD(fsIn[13], costab36_0[72+13]);
	WRACC64_ADD(fsIn[14], costab36_0[72+14]);
	WRACC64_ADD(fsIn[15], costab36_0[72+15]);
	WRACC64_ADD(fsIn[16], costab36_0[72+16]);
	WRACC64_ADD(fsIn[17], costab36_0[72+17]);
	reg0 = RDACC64_SHIFT(IMDCTSHIFT);
	WRACC64_ADD(fsIn[0], costab36_2[72+0]);
	WRACC64_ADD(fsIn[1], costab36_2[72+1]);
	WRACC64_ADD(fsIn[2], costab36_2[72+2]);
	WRACC64_ADD(fsIn[3], costab36_2[72+3]);
	WRACC64_ADD(fsIn[4], costab36_2[72+4]);
	WRACC64_ADD(fsIn[5], costab36_2[72+5]);
	WRACC64_ADD(fsIn[6], costab36_2[72+6]);
	WRACC64_ADD(fsIn[7], costab36_2[72+7]);
	WRACC64_ADD(fsIn[8], costab36_2[72+8]);
	WRACC64_ADD(fsIn[9], costab36_2[72+9]);
	WRACC64_ADD(fsIn[10], costab36_2[72+10]);
	WRACC64_ADD(fsIn[11], costab36_2[72+11]);
	WRACC64_ADD(fsIn[12], costab36_2[72+12]);
	WRACC64_ADD(fsIn[13], costab36_2[72+13]);
	WRACC64_ADD(fsIn[14], costab36_2[72+14]);
	WRACC64_ADD(fsIn[15], costab36_2[72+15]);
	WRACC64_ADD(fsIn[16], costab36_2[72+16]);
	WRACC64_ADD(fsIn[17], costab36_2[72+17]);
	reg2 = RDACC64_SHIFT(IMDCTSHIFT);
/*
	for (k=0; k<18; k++) 
	{
		reg0 = ADD(reg0, MULSHIFT(fsIn[k], costab36_0[k+72], IMDCTSHIFT));
		reg2 = ADD(reg2, MULSHIFT(fsIn[k], costab36_2[k+72], IMDCTSHIFT));
	}
*/
    rawout[8]  = reg0;
    rawout[26] = reg2;

    // perform window overlap
    // we setup engine destination and source address of memory because they are the same in both {if(sb & 0x1)  else}

	if(sb & 0x1) 
	{  // odd subband
		if(bt!=3)
		{
			windowa = (int*)window_la_freqinv;
			windowb = (int*)window_lb_freqinv;
		}
		else
		{
			windowa = (int*)window_ma_freqinv;
			windowb = (int*)window_mb_freqinv;
		}
		for(i=0; i<9; i++) 
		{
			tsOut[32*i]      = MADDSHIFT(xPrev[i], freqinv_a[i],    rawout[i], windowa[i], WINSHIFT);
			tsOut[32*(17-i)] = MSUBSHIFT(xPrev[17-i], freqinv_b[i], rawout[i], windowb[i], WINSHIFT);
		}
    }
	else
	{
		if(bt!=3)
		{
			windowa = (int*)window_la;
			windowb = (int*)window_lb;
		}
		else
		{
			windowa = (int*)window_ma;
			windowb = (int*)window_mb;
		}
		for(i=0; i<9; i++) 
		{
			tsOut[32*i]      = MADDSHIFT(xPrev[i], BYPASS_CONST,    rawout[i], windowa[i], WINSHIFT);
			tsOut[32*(17-i)] = MSUBSHIFT(xPrev[17-i], BYPASS_CONST, rawout[i], windowb[i], WINSHIFT);
		}
	}

	if(bt!=1)
	{
		windowa = (int*)window_lb;
		windowb = (int*)window_la;
    }
	else
	{
		windowa = (int*)window_mb;
		windowb = (int*)window_ma;
	}

	for(i=0; i<9; i++) 
	{
		xPrev[i]    = MULSHIFT(rawout[i+18], windowa[i], WINSHIFT);
		xPrev[17-i] = MULSHIFT(rawout[i+18], windowb[i], WINSHIFT);
	}
}
#else
static void IMDCT36(int *fsIn, int *xPrev, int *tsOut, int *rawout, int sb, int bt)
{
    int k;
    int i,j;
    int reg0, reg1, reg2, reg3;
    int tablebase, *rawoutbase;
    int *windowa, *windowb;

    tablebase=0;
    rawoutbase = rawout;

	for (j=0; j<2; j++)
	{
		for (i=0; i<2; i++) 
		{
                    reg0 = reg1 = reg2 = reg3 = 0;
			for (k=0; k<18; k++) 
			{
                        reg0 = ADD(reg0, MULSHIFT(fsIn[k], costab36_0[tablebase+k], IMDCTSHIFT));
                        reg1 = SUB(reg1, MULSHIFT(fsIn[k], costab36_1[tablebase+k], IMDCTSHIFT));
                        reg2 = ADD(reg2, MULSHIFT(fsIn[k], costab36_2[tablebase+k], IMDCTSHIFT));
                        reg3 = SUB(reg3, MULSHIFT(fsIn[k], costab36_3[tablebase+k], IMDCTSHIFT));
                    }
                *rawoutbase++ = reg0;
                *rawoutbase++ = reg1;
                *rawoutbase++ = reg2;
                *rawoutbase++ = reg3;
                tablebase+=18;
            }   // after j=0, rawoutbase +=8;
            rawoutbase +=10;
        }
            reg0 = reg2 = 0;
	for (k=0; k<18; k++) 
	{
                reg0 = ADD(reg0, MULSHIFT(fsIn[k], costab36_0[k+72], IMDCTSHIFT));
                reg2 = ADD(reg2, MULSHIFT(fsIn[k], costab36_2[k+72], IMDCTSHIFT));
            }

    rawout[8]  = reg0;
    rawout[26] = reg2;

    // perform window overlap
    // we setup engine destination and source address of memory because they are the same in both {if(sb & 0x1)  else}

	if(sb & 0x1) 
	{  // odd subband
		if(bt!=3)
		{
                windowa = (int*)window_la_freqinv;
                windowb = (int*)window_lb_freqinv;
            }
		else
		{
                windowa = (int*)window_ma_freqinv;
                windowb = (int*)window_mb_freqinv;
            }
		for(i=0; i<9; i++) 
		{
                tsOut[32*i]      = MADDSHIFT(xPrev[i], freqinv_a[i],    rawout[i], windowa[i], WINSHIFT);
                tsOut[32*(17-i)] = MSUBSHIFT(xPrev[17-i], freqinv_b[i], rawout[i], windowb[i], WINSHIFT);
            }
    }
	else
	{
		if(bt!=3)
		{
                windowa = (int*)window_la;
                windowb = (int*)window_lb;
            }
		else
		{
                windowa = (int*)window_ma;
                windowb = (int*)window_mb;
            }
		for(i=0; i<9; i++) 
		{
                tsOut[32*i]      = MADDSHIFT(xPrev[i], BYPASS_CONST,    rawout[i], windowa[i], WINSHIFT);
                tsOut[32*(17-i)] = MSUBSHIFT(xPrev[17-i], BYPASS_CONST, rawout[i], windowb[i], WINSHIFT);
            }
        }

	if(bt!=1)
	{
            windowa = (int*)window_lb;
            windowb = (int*)window_la;
        }
	else
	{
            windowa = (int*)window_mb;
            windowb = (int*)window_ma;
        }

	for(i=0; i<9; i++) 
	{
            xPrev[i]    = MULSHIFT(rawout[i+18], windowa[i], WINSHIFT);
            xPrev[17-i] = MULSHIFT(rawout[i+18], windowb[i], WINSHIFT);
        }
}
#endif

void AntiAliasGrCh()
{
    int gr, ch;
    int nBfly;
    FrameHeader *fh;
    SideInfo *si;
    HuffmanInfo *hi;

    // si is an array of up to 4 structs, stored as gr0ch0, gr0ch1, gr1ch0, gr1ch1
    fh = &(mp3DecInfo.FrameHeaderPS);
    si = &(mp3DecInfo.SideInfoPS);
    hi = &(mp3DecInfo.HuffmanInfoPS);

        // anti-aliasing done on whole long blocks only
        // for mixed blocks, nBfly always 1, except 3 for 8 kHz MPEG 2.5 (see sfBandTab)
        //   nLongBlocks = number of blocks with (possibly) non-zero power
        //   nBfly = number of butterflies to do (nLongBlocks - 1, unless no long blocks)
	for (gr = 0; gr < mp3DecInfo.nGrans; gr++) 
	{
            // alias reduction, inverse MDCT, overlap-add, frequency inversion
		for (ch = 0; ch < mp3DecInfo.nChans; ch++)
		{
                int *x= hi->huffDecBuf[gr][ch];
			if (si->sis[gr][ch].blockType != 2) 
			{
                    nBfly = MIN((hi->nonZeroBound[gr][ch] + 7) / 18, 31);
            } 
			else if (si->sis[gr][ch].blockType == 2 && si->sis[gr][ch].mixedBlock) 
			{
                    // mixed block - long transforms until cutoff, then short transforms
                    nBfly = fh->sfBand->l[(fh->ver == MPEG1 ? 8 : 6)] / 18 - 1;
			} 
			else 
			{
                    // all short transforms
                    nBfly = 0;
                }
                hi->nonZeroBound[gr][ch] = MAX(hi->nonZeroBound[gr][ch], (nBfly * 18) + 8);
                ASSERT(hi->nonZeroBound[gr][ch] <= MAX_NSAMP);

                //Begin of AntiAlias(hi->huffDecBuf[gr][ch], nBfly);
                {
                    int *x1, *x2;
                    int k;
                    int i, reg1, reg2;

				for (k = nBfly; k > 0; k--) 
				{
                        x += 18;
                        x1 = x-1;
                        x2 = x;
					for(i=0; i<8; i++)
					{
                                reg1 = MSUBSHIFT(cs[i], *x1,  ca[i], *x2, AASHIFT);
                                reg2 = MADDSHIFT(ca[i], *x1,  cs[i], *x2, AASHIFT);
                                *x1-- = reg1;
                                *x2++ = reg2;
                            }
                    }
                }
                //End of AntiAlias(hi->huffDecBuf[gr][ch], nBfly);
            }   // ch
        }   // gr
}

#if 0
void IMDCT12x3(int *fsIn, int *xPrev, int *tsOut, int *rawout, int sb)
{
    register int m, k;
    register int reg0, reg1, reg2, reg3, prev0, prev1, prev2, prev3, prev4, prev5;
    register int *ptr;

    ptr = rawout;
    for(k=0; k<6; k++){
        *(ptr+k) = *(xPrev+k);
    }
    prev0 = *(xPrev+6);
    prev1 = *(xPrev+7);
    prev2 = *(xPrev+8);
    prev3 = *(xPrev+9);
    prev4 = *(xPrev+10);
    prev5 = *(xPrev+11);
	ptr += 6;
	xPrev += 12;

	for(m=0;m<3;m++)
	{
		reg0 = reg1 = reg2 = reg3 = 0;
		WRACC64_ADD(fsIn[m+3*k], costab12_0[0]);
		WRACC64_ADD(fsIn[m+3*1], costab12_0[1]);
		WRACC64_ADD(fsIn[m+3*2], costab12_0[2]);
		WRACC64_ADD(fsIn[m+3*3], costab12_0[3]);
		WRACC64_ADD(fsIn[m+3*4], costab12_0[4]);
		WRACC64_ADD(fsIn[m+3*5], costab12_0[5]);
		reg0 = RDACC64_SHIFT(IMDCTSHIFT);
		WRACC64_SUB(fsIn[m+3*k], costab12_1[0]);
		WRACC64_SUB(fsIn[m+3*1], costab12_1[1]);
		WRACC64_SUB(fsIn[m+3*2], costab12_1[2]);
		WRACC64_SUB(fsIn[m+3*3], costab12_1[3]);
		WRACC64_SUB(fsIn[m+3*4], costab12_1[4]);
		WRACC64_SUB(fsIn[m+3*5], costab12_1[5]);
		reg1 = RDACC64_SHIFT(IMDCTSHIFT);
		WRACC64_ADD(fsIn[m+3*k], costab12_2[0]);
		WRACC64_ADD(fsIn[m+3*1], costab12_2[1]);
		WRACC64_ADD(fsIn[m+3*2], costab12_2[2]);
		WRACC64_ADD(fsIn[m+3*3], costab12_2[3]);
		WRACC64_ADD(fsIn[m+3*4], costab12_2[4]);
		WRACC64_ADD(fsIn[m+3*5], costab12_2[5]);
		reg2 = RDACC64_SHIFT(IMDCTSHIFT);
		WRACC64_SUB(fsIn[m+3*k], costab12_3[0]);
		WRACC64_SUB(fsIn[m+3*1], costab12_3[1]);
		WRACC64_SUB(fsIn[m+3*2], costab12_3[2]);
		WRACC64_SUB(fsIn[m+3*3], costab12_3[3]);
		WRACC64_SUB(fsIn[m+3*4], costab12_3[4]);
		WRACC64_SUB(fsIn[m+3*5], costab12_3[5]);
		reg3 = RDACC64_SHIFT(IMDCTSHIFT);
/*
		for (k=0; k<6; k++) 
		{
            reg0 = ADD(reg0, MULSHIFT(fsIn[m+3*k], costab12_0[k], IMDCTSHIFT));
            reg1 = SUB(reg1, MULSHIFT(fsIn[m+3*k], costab12_1[k], IMDCTSHIFT));
            reg2 = ADD(reg2, MULSHIFT(fsIn[m+3*k], costab12_2[k], IMDCTSHIFT));
            reg3 = SUB(reg3, MULSHIFT(fsIn[m+3*k], costab12_3[k], IMDCTSHIFT));
        }
*/
		// m=0.. ptr=rawout+6, m=1.. ptr=rawout+12, m=2.. ptr=prevblck[ch][sb][0]
        *ptr = ADD(prev0, reg0);
        *(ptr+1) = ADD(prev1, reg1);
        *(ptr+2) = ADD(prev2, reg2);
        *(ptr+3) = ADD(prev3, reg3);

		reg0 = reg1 = prev0 = prev1 = 0;
		WRACC64_ADD(fsIn[m+3*k], costab12_0[0]);
		WRACC64_ADD(fsIn[m+3*1], costab12_0[1]);
		WRACC64_ADD(fsIn[m+3*2], costab12_0[2]);
		WRACC64_ADD(fsIn[m+3*3], costab12_0[3]);
		WRACC64_ADD(fsIn[m+3*4], costab12_0[4]);
		WRACC64_ADD(fsIn[m+3*5], costab12_0[5]);
		reg0 = RDACC64_SHIFT(IMDCTSHIFT);
		WRACC64_SUB(fsIn[m+3*k], costab12_1[0]);
		WRACC64_SUB(fsIn[m+3*1], costab12_1[1]);
		WRACC64_SUB(fsIn[m+3*2], costab12_1[2]);
		WRACC64_SUB(fsIn[m+3*3], costab12_1[3]);
		WRACC64_SUB(fsIn[m+3*4], costab12_1[4]);
		WRACC64_SUB(fsIn[m+3*5], costab12_1[5]);
		prev0 = RDACC64_SHIFT(IMDCTSHIFT);
		WRACC64_ADD(fsIn[m+3*k], costab12_2[0]);
		WRACC64_ADD(fsIn[m+3*1], costab12_2[1]);
		WRACC64_ADD(fsIn[m+3*2], costab12_2[2]);
		WRACC64_ADD(fsIn[m+3*3], costab12_2[3]);
		WRACC64_ADD(fsIn[m+3*4], costab12_2[4]);
		WRACC64_ADD(fsIn[m+3*5], costab12_2[5]);
		reg2 = RDACC64_SHIFT(IMDCTSHIFT);
		WRACC64_SUB(fsIn[m+3*k], costab12_3[0]);
		WRACC64_SUB(fsIn[m+3*1], costab12_3[1]);
		WRACC64_SUB(fsIn[m+3*2], costab12_3[2]);
		WRACC64_SUB(fsIn[m+3*3], costab12_3[3]);
		WRACC64_SUB(fsIn[m+3*4], costab12_3[4]);
		WRACC64_SUB(fsIn[m+3*5], costab12_3[5]);
		prev1 = RDACC64_SHIFT(IMDCTSHIFT);
/*
		for (k=0; k<6; k++) 
		{
			reg0 = ADD(reg0, MULSHIFT(fsIn[m+3*k], costab12_0[k+6], IMDCTSHIFT));
			reg1 = SUB(reg1, MULSHIFT(fsIn[m+3*k], costab12_1[k+6], IMDCTSHIFT));
			prev0= ADD(prev0, MULSHIFT(fsIn[m+3*k], costab12_2[k+6], IMDCTSHIFT));
			prev1= SUB(prev1, MULSHIFT(fsIn[m+3*k], costab12_3[k+6], IMDCTSHIFT));
		}
*/
		*(ptr+4) = ADD(prev4, reg0);
		*(ptr+5) = ADD(prev5, reg1);
		ptr += 6;

		prev2 = prev3 = prev4 = prev5 = 0;
		WRACC64_ADD(fsIn[m+3*k], costab12_0[0]);
		WRACC64_ADD(fsIn[m+3*1], costab12_0[1]);
		WRACC64_ADD(fsIn[m+3*2], costab12_0[2]);
		WRACC64_ADD(fsIn[m+3*3], costab12_0[3]);
		WRACC64_ADD(fsIn[m+3*4], costab12_0[4]);
		WRACC64_ADD(fsIn[m+3*5], costab12_0[5]);
		prev2 = RDACC64_SHIFT(IMDCTSHIFT);
		WRACC64_SUB(fsIn[m+3*k], costab12_1[0]);
		WRACC64_SUB(fsIn[m+3*1], costab12_1[1]);
		WRACC64_SUB(fsIn[m+3*2], costab12_1[2]);
		WRACC64_SUB(fsIn[m+3*3], costab12_1[3]);
		WRACC64_SUB(fsIn[m+3*4], costab12_1[4]);
		WRACC64_SUB(fsIn[m+3*5], costab12_1[5]);
		prev3 = RDACC64_SHIFT(IMDCTSHIFT);
		WRACC64_ADD(fsIn[m+3*k], costab12_2[0]);
		WRACC64_ADD(fsIn[m+3*1], costab12_2[1]);
		WRACC64_ADD(fsIn[m+3*2], costab12_2[2]);
		WRACC64_ADD(fsIn[m+3*3], costab12_2[3]);
		WRACC64_ADD(fsIn[m+3*4], costab12_2[4]);
		WRACC64_ADD(fsIn[m+3*5], costab12_2[5]);
		prev4 = RDACC64_SHIFT(IMDCTSHIFT);
		WRACC64_SUB(fsIn[m+3*k], costab12_3[0]);
		WRACC64_SUB(fsIn[m+3*1], costab12_3[1]);
		WRACC64_SUB(fsIn[m+3*2], costab12_3[2]);
		WRACC64_SUB(fsIn[m+3*3], costab12_3[3]);
		WRACC64_SUB(fsIn[m+3*4], costab12_3[4]);
		WRACC64_SUB(fsIn[m+3*5], costab12_3[5]);
		prev5 = RDACC64_SHIFT(IMDCTSHIFT);
/*
		for (k=0; k<6; k++) 
		{
			prev2 = ADD(prev2, MULSHIFT(fsIn[m+3*k], costab12_0[k+12], IMDCTSHIFT));
			prev3 = SUB(prev3, MULSHIFT(fsIn[m+3*k], costab12_1[k+12], IMDCTSHIFT));
			prev4 = ADD(prev4, MULSHIFT(fsIn[m+3*k], costab12_2[k+12], IMDCTSHIFT));
			prev5 = SUB(prev5, MULSHIFT(fsIn[m+3*k], costab12_3[k+12], IMDCTSHIFT));
		}
*/
		if(m==1)
			ptr=xPrev-12;
	}

    *ptr = prev0;
    *(ptr+1) = prev1;
    *(ptr+2) = prev2;
    *(ptr+3) = prev3;
    *(ptr+4) = prev4;
    *(ptr+5) = prev5;
    *(ptr+6) = 0;
    *(ptr+7) = 0;
    *(ptr+8) = 0;
    *(ptr+9) = 0;
    *(ptr+10) = 0;
    *(ptr+11) = 0;
	ptr += 12;

	if(sb & 0x1) 
	{  // odd subband
		for(k=0; k<18; k++)
			tsOut[32*k] = MULSHIFT(rawout[k], freqinv_a[k], WINSHIFT);
	}
	else
	{
		for(k=0; k<18; k++)
			tsOut[32*k] = rawout[k];
	}
}
#else
void IMDCT12x3(int *fsIn, int *xPrev, int *tsOut, int *rawout, int sb)
{
    register int m, k;
    register int reg0, reg1, reg2, reg3, prev0, prev1, prev2, prev3, prev4, prev5;
    register int *ptr;

    ptr = rawout;
    for(k=0; k<6; k++){
        *ptr++ = *xPrev++;
    }
    prev0 = *xPrev++;
    prev1 = *xPrev++;
    prev2 = *xPrev++;
    prev3 = *xPrev++;
    prev4 = *xPrev++;
    prev5 = *xPrev++;

	for(m=0;m<3;m++)
	{
		reg0 = reg1 = reg2 = reg3 = 0;
		for (k=0; k<6; k++) 
		{
            reg0 = ADD(reg0, MULSHIFT(fsIn[m+3*k], costab12_0[k], IMDCTSHIFT));
            reg1 = SUB(reg1, MULSHIFT(fsIn[m+3*k], costab12_1[k], IMDCTSHIFT));
            reg2 = ADD(reg2, MULSHIFT(fsIn[m+3*k], costab12_2[k], IMDCTSHIFT));
            reg3 = SUB(reg3, MULSHIFT(fsIn[m+3*k], costab12_3[k], IMDCTSHIFT));
        }

		// m=0.. ptr=rawout+6, m=1.. ptr=rawout+12, m=2.. ptr=prevblck[ch][sb][0]
        *ptr++ = ADD(prev0, reg0);
        *ptr++ = ADD(prev1, reg1);
        *ptr++ = ADD(prev2, reg2);
        *ptr++ = ADD(prev3, reg3);

		reg0 = reg1 = prev0 = prev1 = 0;
		for (k=0; k<6; k++) 
		{
			reg0 = ADD(reg0, MULSHIFT(fsIn[m+3*k], costab12_0[k+6], IMDCTSHIFT));
			reg1 = SUB(reg1, MULSHIFT(fsIn[m+3*k], costab12_1[k+6], IMDCTSHIFT));
			prev0= ADD(prev0, MULSHIFT(fsIn[m+3*k], costab12_2[k+6], IMDCTSHIFT));
			prev1= SUB(prev1, MULSHIFT(fsIn[m+3*k], costab12_3[k+6], IMDCTSHIFT));
		}

		*ptr++ = ADD(prev4, reg0);
		*ptr++ = ADD(prev5, reg1);

		prev2 = prev3 = prev4 = prev5 = 0;
		for (k=0; k<6; k++) 
		{
			prev2 = ADD(prev2, MULSHIFT(fsIn[m+3*k], costab12_0[k+12], IMDCTSHIFT));
			prev3 = SUB(prev3, MULSHIFT(fsIn[m+3*k], costab12_1[k+12], IMDCTSHIFT));
			prev4 = ADD(prev4, MULSHIFT(fsIn[m+3*k], costab12_2[k+12], IMDCTSHIFT));
			prev5 = SUB(prev5, MULSHIFT(fsIn[m+3*k], costab12_3[k+12], IMDCTSHIFT));
		}

		if(m==1)
			ptr=xPrev-12;
		}

    *ptr++ = prev0;
    *ptr++ = prev1;
    *ptr++ = prev2;
    *ptr++ = prev3;
    *ptr++ = prev4;
    *ptr++ = prev5;
    *ptr++ = 0;
    *ptr++ = 0;
    *ptr++ = 0;
    *ptr++ = 0;
    *ptr++ = 0;
    *ptr++ = 0;

	if(sb & 0x1) 
	{  // odd subband
		for(k=0; k<18; k++)
			tsOut[32*k] = MULSHIFT(rawout[k], freqinv_a[k], WINSHIFT);
		}
	else
	{
		for(k=0; k<18; k++)
			tsOut[32*k] = rawout[k];
		}
	}
#endif

void HybridTransformGrCh()
{
    int gr, ch;
    SideInfo *si;
    HuffmanInfo *hi;
    IMDCTInfo *mi;
    int nBlocksTotal[MAX_NGRAN][MAX_NCHAN];

    // si is an array of up to 4 structs, stored as gr0ch0, gr0ch1, gr1ch0, gr1ch1
    si = &(mp3DecInfo.SideInfoPS);
    hi = &(mp3DecInfo.HuffmanInfoPS);
    mi = &(mp3DecInfo.IMDCTInfoPS);

    nBlocksTotal[0][0] = (hi->nonZeroBound[0][0] + 17) / 18;
    nBlocksTotal[0][1] = (hi->nonZeroBound[0][1] + 17) / 18;
    nBlocksTotal[1][0] = (hi->nonZeroBound[1][0] + 17) / 18;
    nBlocksTotal[1][1] = (hi->nonZeroBound[1][1] + 17) / 18;

    for (gr = 0; gr < mp3DecInfo.nGrans; gr++) 
	{
        for (ch = 0; ch < mp3DecInfo.nChans; ch++)
		{
            int i,j, sb;
            int *xCurr, *xPrev, *y;
            SideInfoSub *sis = &si->sis[gr][ch];

            xCurr = hi->huffDecBuf[gr][ch];
            xPrev = mi->prevblck[ch];
            y = &mi->outBuf[gr][ch][0][0];  // outBuf[MAX_NGRAN][MAX_NCHAN][BLOCK_SIZE][NBANDS]

            for(sb = 0; sb < nBlocksTotal[gr][ch]; sb++) 
			{
                // currWinIdx picks the right window for long blocks (if mixed, long blocks use window type 0)
                i = sis->blockType; //  0 = normal, 1 = start, 2 = short, 3 = stop
                if (sis->winSwitchFlag && sis->mixedBlock && (sb < 2))
                    i = 0;
                if(i == 2)
                    IMDCT12x3(xCurr, xPrev, &mi->outBuf[gr][ch][0][sb], mi->rawout, sb);
                else
                    IMDCT36(xCurr, xPrev, &mi->outBuf[gr][ch][0][sb], mi->rawout, sb, i);
                xCurr += 18;
                xPrev += 18;
			}

                // do odd subband first
                xCurr = xPrev;
                i=sb;
			if(!(sb & 0x1))
			{
                    i++;
                    xCurr += 18;
                }
			for( ; i < NBANDS; i+=2)
			{
				for(j=0; j<18; j++) 
                        *(y+j*NBANDS+i) = MULSHIFT(xCurr[j], freqinv_a[j], WINSHIFT);
                    xCurr += 36;
                }

                // do even subband next
                xCurr = xPrev;
                i=sb;
			if(sb & 0x1)
			{
                    i++;
                    xCurr += 18;
                }
			for( ; i < NBANDS; i+=2)
			{
				for(j=0; j<18; j++) 
                        *(y+j*NBANDS+i) = MULSHIFT(xCurr[j], BYPASS_CONST, WINSHIFT);
                    xCurr += 36;
                }
			for(i = 0; i < (NBANDS-sb)*18; i++)
                    xPrev[i] = 0;
                }
        }
    // output has gained 2 int bits
}
