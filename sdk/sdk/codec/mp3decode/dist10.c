#include "mp3_config.h"

#define SBLIMIT 32
#define SSLIMIT 18
#define SCALE   32768
#define DCTIISHIFT 30
#define SYNTHSHIFT 30
#define PCMOUTSHIFT 9

//#define LOOP  4
 
#include "Dtab.h"
#include "dctiitab.h"

//#include "plugin.h"

static int sbi_vindex[16] = {15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};

#if 1
static __inline void DCT_II32(int *x, int (*V)[2][16], int ch, int vindex)
{
    int k, index;
    int *vbase0, *vbase16;
    int tabbase0;
    int j, vadd;

    SETOVFLN(OVFLW_STAGE_DCTII);
    index = ((vindex & 0x1) ? 8:0) + (vindex >> 1);

    vbase16 = vbase0 = (int*)V+ch*16+index;
    tabbase0 = 128;
    vadd=32;

	for(j=0; j<2; j++)
	{
		for(k=0; k<4; k++)
		{
			register int reg0, reg1, reg2, reg3;
			WRACC64_ADD(x[0], dctii_32_costab[0][tabbase0+0]);
			WRACC64_ADD(x[1], dctii_32_costab[0][tabbase0+1]);
			WRACC64_ADD(x[2], dctii_32_costab[0][tabbase0+2]);
			WRACC64_ADD(x[3], dctii_32_costab[0][tabbase0+3]);
			WRACC64_ADD(x[4], dctii_32_costab[0][tabbase0+4]);
			WRACC64_ADD(x[5], dctii_32_costab[0][tabbase0+5]);
			WRACC64_ADD(x[6], dctii_32_costab[0][tabbase0+6]);
			WRACC64_ADD(x[7], dctii_32_costab[0][tabbase0+7]);
			WRACC64_ADD(x[8], dctii_32_costab[0][tabbase0+8]);
			WRACC64_ADD(x[9], dctii_32_costab[0][tabbase0+9]);
			WRACC64_ADD(x[10], dctii_32_costab[0][tabbase0+10]);
			WRACC64_ADD(x[11], dctii_32_costab[0][tabbase0+11]);
			WRACC64_ADD(x[12], dctii_32_costab[0][tabbase0+12]);
			WRACC64_ADD(x[13], dctii_32_costab[0][tabbase0+13]);
			WRACC64_ADD(x[14], dctii_32_costab[0][tabbase0+14]);
			WRACC64_ADD(x[15], dctii_32_costab[0][tabbase0+15]);
			WRACC64_ADD(x[16], dctii_32_costab[0][tabbase0+16]);
			WRACC64_ADD(x[17], dctii_32_costab[0][tabbase0+17]);
			WRACC64_ADD(x[18], dctii_32_costab[0][tabbase0+18]);
			WRACC64_ADD(x[19], dctii_32_costab[0][tabbase0+19]);
			WRACC64_ADD(x[20], dctii_32_costab[0][tabbase0+20]);
			WRACC64_ADD(x[21], dctii_32_costab[0][tabbase0+21]);
			WRACC64_ADD(x[22], dctii_32_costab[0][tabbase0+22]);
			WRACC64_ADD(x[23], dctii_32_costab[0][tabbase0+23]);
			WRACC64_ADD(x[24], dctii_32_costab[0][tabbase0+24]);
			WRACC64_ADD(x[25], dctii_32_costab[0][tabbase0+25]);
			WRACC64_ADD(x[26], dctii_32_costab[0][tabbase0+26]);
			WRACC64_ADD(x[27], dctii_32_costab[0][tabbase0+27]);
			WRACC64_ADD(x[28], dctii_32_costab[0][tabbase0+28]);
			WRACC64_ADD(x[29], dctii_32_costab[0][tabbase0+29]);
			WRACC64_ADD(x[30], dctii_32_costab[0][tabbase0+30]);
			WRACC64_ADD(x[31], dctii_32_costab[0][tabbase0+31]);
			reg0 = RDACC64_SHIFT(DCTIISHIFT);
			WRACC64_SUB(x[0], dctii_32_costab[1][tabbase0+0]);
			WRACC64_SUB(x[1], dctii_32_costab[1][tabbase0+1]);
			WRACC64_SUB(x[2], dctii_32_costab[1][tabbase0+2]);
			WRACC64_SUB(x[3], dctii_32_costab[1][tabbase0+3]);
			WRACC64_SUB(x[4], dctii_32_costab[1][tabbase0+4]);
			WRACC64_SUB(x[5], dctii_32_costab[1][tabbase0+5]);
			WRACC64_SUB(x[6], dctii_32_costab[1][tabbase0+6]);
			WRACC64_SUB(x[7], dctii_32_costab[1][tabbase0+7]);
			WRACC64_SUB(x[8], dctii_32_costab[1][tabbase0+8]);
			WRACC64_SUB(x[9], dctii_32_costab[1][tabbase0+9]);
			WRACC64_SUB(x[10], dctii_32_costab[1][tabbase0+10]);
			WRACC64_SUB(x[11], dctii_32_costab[1][tabbase0+11]);
			WRACC64_SUB(x[12], dctii_32_costab[1][tabbase0+12]);
			WRACC64_SUB(x[13], dctii_32_costab[1][tabbase0+13]);
			WRACC64_SUB(x[14], dctii_32_costab[1][tabbase0+14]);
			WRACC64_SUB(x[15], dctii_32_costab[1][tabbase0+15]);
			WRACC64_SUB(x[16], dctii_32_costab[1][tabbase0+16]);
			WRACC64_SUB(x[17], dctii_32_costab[1][tabbase0+17]);
			WRACC64_SUB(x[18], dctii_32_costab[1][tabbase0+18]);
			WRACC64_SUB(x[19], dctii_32_costab[1][tabbase0+19]);
			WRACC64_SUB(x[20], dctii_32_costab[1][tabbase0+20]);
			WRACC64_SUB(x[21], dctii_32_costab[1][tabbase0+21]);
			WRACC64_SUB(x[22], dctii_32_costab[1][tabbase0+22]);
			WRACC64_SUB(x[23], dctii_32_costab[1][tabbase0+23]);
			WRACC64_SUB(x[24], dctii_32_costab[1][tabbase0+24]);
			WRACC64_SUB(x[25], dctii_32_costab[1][tabbase0+25]);
			WRACC64_SUB(x[26], dctii_32_costab[1][tabbase0+26]);
			WRACC64_SUB(x[27], dctii_32_costab[1][tabbase0+27]);
			WRACC64_SUB(x[28], dctii_32_costab[1][tabbase0+28]);
			WRACC64_SUB(x[29], dctii_32_costab[1][tabbase0+29]);
			WRACC64_SUB(x[30], dctii_32_costab[1][tabbase0+30]);
			WRACC64_SUB(x[31], dctii_32_costab[1][tabbase0+31]);
			reg1 = RDACC64_SHIFT(DCTIISHIFT);
			WRACC64_ADD(x[0], dctii_32_costab[2][tabbase0+0]);
			WRACC64_ADD(x[1], dctii_32_costab[2][tabbase0+1]);
			WRACC64_ADD(x[2], dctii_32_costab[2][tabbase0+2]);
			WRACC64_ADD(x[3], dctii_32_costab[2][tabbase0+3]);
			WRACC64_ADD(x[4], dctii_32_costab[2][tabbase0+4]);
			WRACC64_ADD(x[5], dctii_32_costab[2][tabbase0+5]);
			WRACC64_ADD(x[6], dctii_32_costab[2][tabbase0+6]);
			WRACC64_ADD(x[7], dctii_32_costab[2][tabbase0+7]);
			WRACC64_ADD(x[8], dctii_32_costab[2][tabbase0+8]);
			WRACC64_ADD(x[9], dctii_32_costab[2][tabbase0+9]);
			WRACC64_ADD(x[10], dctii_32_costab[2][tabbase0+10]);
			WRACC64_ADD(x[11], dctii_32_costab[2][tabbase0+11]);
			WRACC64_ADD(x[12], dctii_32_costab[2][tabbase0+12]);
			WRACC64_ADD(x[13], dctii_32_costab[2][tabbase0+13]);
			WRACC64_ADD(x[14], dctii_32_costab[2][tabbase0+14]);
			WRACC64_ADD(x[15], dctii_32_costab[2][tabbase0+15]);
			WRACC64_ADD(x[16], dctii_32_costab[2][tabbase0+16]);
			WRACC64_ADD(x[17], dctii_32_costab[2][tabbase0+17]);
			WRACC64_ADD(x[18], dctii_32_costab[2][tabbase0+18]);
			WRACC64_ADD(x[19], dctii_32_costab[2][tabbase0+19]);
			WRACC64_ADD(x[20], dctii_32_costab[2][tabbase0+20]);
			WRACC64_ADD(x[21], dctii_32_costab[2][tabbase0+21]);
			WRACC64_ADD(x[22], dctii_32_costab[2][tabbase0+22]);
			WRACC64_ADD(x[23], dctii_32_costab[2][tabbase0+23]);
			WRACC64_ADD(x[24], dctii_32_costab[2][tabbase0+24]);
			WRACC64_ADD(x[25], dctii_32_costab[2][tabbase0+25]);
			WRACC64_ADD(x[26], dctii_32_costab[2][tabbase0+26]);
			WRACC64_ADD(x[27], dctii_32_costab[2][tabbase0+27]);
			WRACC64_ADD(x[28], dctii_32_costab[2][tabbase0+28]);
			WRACC64_ADD(x[29], dctii_32_costab[2][tabbase0+29]);
			WRACC64_ADD(x[30], dctii_32_costab[2][tabbase0+30]);
			WRACC64_ADD(x[31], dctii_32_costab[2][tabbase0+31]);
			reg2 = RDACC64_SHIFT(DCTIISHIFT);
			WRACC64_SUB(x[0], dctii_32_costab[3][tabbase0+0]);
			WRACC64_SUB(x[1], dctii_32_costab[3][tabbase0+1]);
			WRACC64_SUB(x[2], dctii_32_costab[3][tabbase0+2]);
			WRACC64_SUB(x[3], dctii_32_costab[3][tabbase0+3]);
			WRACC64_SUB(x[4], dctii_32_costab[3][tabbase0+4]);
			WRACC64_SUB(x[5], dctii_32_costab[3][tabbase0+5]);
			WRACC64_SUB(x[6], dctii_32_costab[3][tabbase0+6]);
			WRACC64_SUB(x[7], dctii_32_costab[3][tabbase0+7]);
			WRACC64_SUB(x[8], dctii_32_costab[3][tabbase0+8]);
			WRACC64_SUB(x[9], dctii_32_costab[3][tabbase0+9]);
			WRACC64_SUB(x[10], dctii_32_costab[3][tabbase0+10]);
			WRACC64_SUB(x[11], dctii_32_costab[3][tabbase0+11]);
			WRACC64_SUB(x[12], dctii_32_costab[3][tabbase0+12]);
			WRACC64_SUB(x[13], dctii_32_costab[3][tabbase0+13]);
			WRACC64_SUB(x[14], dctii_32_costab[3][tabbase0+14]);
			WRACC64_SUB(x[15], dctii_32_costab[3][tabbase0+15]);
			WRACC64_SUB(x[16], dctii_32_costab[3][tabbase0+16]);
			WRACC64_SUB(x[17], dctii_32_costab[3][tabbase0+17]);
			WRACC64_SUB(x[18], dctii_32_costab[3][tabbase0+18]);
			WRACC64_SUB(x[19], dctii_32_costab[3][tabbase0+19]);
			WRACC64_SUB(x[20], dctii_32_costab[3][tabbase0+20]);
			WRACC64_SUB(x[21], dctii_32_costab[3][tabbase0+21]);
			WRACC64_SUB(x[22], dctii_32_costab[3][tabbase0+22]);
			WRACC64_SUB(x[23], dctii_32_costab[3][tabbase0+23]);
			WRACC64_SUB(x[24], dctii_32_costab[3][tabbase0+24]);
			WRACC64_SUB(x[25], dctii_32_costab[3][tabbase0+25]);
			WRACC64_SUB(x[26], dctii_32_costab[3][tabbase0+26]);
			WRACC64_SUB(x[27], dctii_32_costab[3][tabbase0+27]);
			WRACC64_SUB(x[28], dctii_32_costab[3][tabbase0+28]);
			WRACC64_SUB(x[29], dctii_32_costab[3][tabbase0+29]);
			WRACC64_SUB(x[30], dctii_32_costab[3][tabbase0+30]);
			WRACC64_SUB(x[31], dctii_32_costab[3][tabbase0+31]);
			reg3 = RDACC64_SHIFT(DCTIISHIFT);
/*
			for(i=0; i<32; i++)
				WRACC64_ADD(x[i], dctii_32_costab[0][tabbase0+i]);
			reg0 = RDACC64_SHIFT(DCTIISHIFT);

			for(i=0; i<32; i++)
				WRACC64_SUB(x[i], dctii_32_costab[1][tabbase0+i]);
			reg1 = RDACC64_SHIFT(DCTIISHIFT);

			for(i=0; i<32; i++)
				WRACC64_ADD(x[i], dctii_32_costab[2][tabbase0+i]);
			reg2 = RDACC64_SHIFT(DCTIISHIFT);

			for(i=0; i<32; i++)
				WRACC64_SUB(x[i], dctii_32_costab[3][tabbase0+i]);
			reg3 = RDACC64_SHIFT(DCTIISHIFT);
*/
			if(j==1)
			{
				reg0=-reg0; reg1=-reg1; reg2=-reg2; reg3=-reg3;
			}
			*vbase0 = reg0; vbase0+=vadd;
			*vbase0 = reg1; vbase0+=vadd;
			*vbase0 = reg2; vbase0+=vadd;
			*vbase0 = reg3; vbase0+=vadd;
			tabbase0 +=32;
		}
		vbase0      = 33*32+vbase16;
		tabbase0    = 0;
		vadd        =-32;
	}

    vbase0  = vbase16 + 16*32;
    *vbase0 = 0;
    vbase0 +=32;
    *vbase0 = -(*vbase16);

}
#else
static __inline void DCT_II32(int *x, int (*V)[2][16], int ch, int vindex)
{
    int i;
    int k, index;
    int *vbase0, *vbase16;
    int tabbase0;
    int j, vadd;

    SETOVFLN(OVFLW_STAGE_DCTII);
    index = ((vindex & 0x1) ? 8:0) + (vindex >> 1);

    vbase16 = vbase0 = (int*)V+ch*16+index;
    tabbase0 = 128;
    vadd=32;

	for(j=0; j<2; j++)
	{
		for(k=0; k<4; k++)
		{
                register int reg0, reg1, reg2, reg3;
                    reg0 = reg1 = reg2 = reg3 = 0;
			for(i=0; i<32; i++)
			{
                        reg0 = ADD(reg0, MULSHIFT(x[i], dctii_32_costab[0][tabbase0+i], DCTIISHIFT));
                        reg1 = SUB(reg1, MULSHIFT(x[i], dctii_32_costab[1][tabbase0+i], DCTIISHIFT));
                        reg2 = ADD(reg2, MULSHIFT(x[i], dctii_32_costab[2][tabbase0+i], DCTIISHIFT));
                        reg3 = SUB(reg3, MULSHIFT(x[i], dctii_32_costab[3][tabbase0+i], DCTIISHIFT));
                    }
			if(j==1)
			{
                    reg0=-reg0; reg1=-reg1; reg2=-reg2; reg3=-reg3;
                }
                *vbase0 = reg0; vbase0+=vadd;
                *vbase0 = reg1; vbase0+=vadd;
                *vbase0 = reg2; vbase0+=vadd;
                *vbase0 = reg3; vbase0+=vadd;
                tabbase0 +=32;
            }
            vbase0      = 33*32+vbase16;
            tabbase0    = 0;
            vadd        =-32;
        }

    vbase0  = vbase16 + 16*32;
    *vbase0 = 0;
    vbase0 +=32;
    *vbase0 = -(*vbase16);

}
#endif

#if 1
void synthesis(int (*V)[2][16], int nch, int DOffset, int (*outbuf)[MAX_FRAMESIZE])
{
    int j,ch;
    int r,s;
    int turn;
    int d0tab, d1tab, d2tab, d3tab;
    int d0base, d1base, d2base, d3base;
    int *v0base, *v1base, *v2base, *v3base;

     int i;

    SETOVFLN(OVFLW_STAGE_SYNTH);

    DOffset = (16 - DOffset) & 0xf;
    turn = (DOffset+1) & 0x1;
    d0tab =(2*DOffset) & 0x3;
    d1tab =(d0tab+2) & 0x3;
    r =(d0tab+1) & 0x3;
    s =(d0tab+3) & 0x3;
    d0base = (DOffset  )/2*16;
    d1base = (DOffset+1)/2*16;
    if(turn==0)
    {
        d2tab = s;
        d3tab = r;
        d2base = (2*DOffset+3)/4*16;
        d3base = (2*DOffset+1)/4*16;
        v0base = (int*)V +17*32;
        v1base = (int*)V + 0*32+8;
        v2base = (int*)V +16*32+8;
        v3base = (int*)V +33*32;
    }
    else
    {
        d2tab = r;
        d3tab = s;
        d2base = (2*DOffset+1)/4*16;
        d3base = (2*DOffset+3)/4*16;
        v0base = (int*)V + 0*32;
        v1base = (int*)V +17*32+8;
        v2base = (int*)V +16*32;
        v3base = (int*)V +33*32+8;
    }

    for(ch=0; ch<nch; ch++)
    {
        int sum1, sum2;
        sum1 = sum2 = 0;
#if 1
        WRACC64_ADD(D[d0tab][d0base+0*16], *(v0base+0));
        WRACC64_ADD(D[d1tab][d1base+0*16], *(v1base+0));
        WRACC64_ADD(D[d0tab][d0base+1*16], *(v0base+1));
        WRACC64_ADD(D[d1tab][d1base+1*16], *(v1base+1));
        WRACC64_ADD(D[d0tab][d0base+2*16], *(v0base+2));
        WRACC64_ADD(D[d1tab][d1base+2*16], *(v1base+2));
        WRACC64_ADD(D[d0tab][d0base+3*16], *(v0base+3));
        WRACC64_ADD(D[d1tab][d1base+3*16], *(v1base+3));
        WRACC64_ADD(D[d0tab][d0base+4*16], *(v0base+4));
        WRACC64_ADD(D[d1tab][d1base+4*16], *(v1base+4));
        WRACC64_ADD(D[d0tab][d0base+5*16], *(v0base+5));
        WRACC64_ADD(D[d1tab][d1base+5*16], *(v1base+5));
        WRACC64_ADD(D[d0tab][d0base+6*16], *(v0base+6));
        WRACC64_ADD(D[d1tab][d1base+6*16], *(v1base+6));
        WRACC64_ADD(D[d0tab][d0base+7*16], *(v0base+7));
        WRACC64_ADD(D[d1tab][d1base+7*16], *(v1base+7));
        sum1 = RDACC64_SHIFT(SYNTHSHIFT+PCMOUTSHIFT);

        WRACC64_SUB(D[d2tab][d2base+0*16], *(v2base+0));
        WRACC64_ADD(D[d3tab][d3base+0*16], *(v3base+0));
        WRACC64_SUB(D[d2tab][d2base+1*16], *(v2base+1));
        WRACC64_ADD(D[d3tab][d3base+1*16], *(v3base+1));
        WRACC64_SUB(D[d2tab][d2base+2*16], *(v2base+2));
        WRACC64_ADD(D[d3tab][d3base+2*16], *(v3base+2));
        WRACC64_SUB(D[d2tab][d2base+3*16], *(v2base+3));
        WRACC64_ADD(D[d3tab][d3base+3*16], *(v3base+3));
        WRACC64_SUB(D[d2tab][d2base+4*16], *(v2base+4));
        WRACC64_ADD(D[d3tab][d3base+4*16], *(v3base+4));
        WRACC64_SUB(D[d2tab][d2base+5*16], *(v2base+5));
        WRACC64_ADD(D[d3tab][d3base+5*16], *(v3base+5));
        WRACC64_SUB(D[d2tab][d2base+6*16], *(v2base+6));
        WRACC64_ADD(D[d3tab][d3base+6*16], *(v3base+6));
        WRACC64_SUB(D[d2tab][d2base+7*16], *(v2base+7));
        WRACC64_ADD(D[d3tab][d3base+7*16], *(v3base+7));
        sum2 = RDACC64_SHIFT(SYNTHSHIFT+PCMOUTSHIFT);
#else
        for(i=0; i<8; i++)
        {
            WRACC64_ADD(D[d0tab][d0base+i*16], *(v0base+i));
            WRACC64_ADD(D[d1tab][d1base+i*16], *(v1base+i));
        }
        sum1 = RDACC64_SHIFT(SYNTHSHIFT+PCMOUTSHIFT);
        for(i=0; i<8; i++)
        {
            WRACC64_SUB(D[d2tab][d2base+i*16], *(v2base+i));
            WRACC64_ADD(D[d3tab][d3base+i*16], *(v3base+i));
        }
        sum2 = RDACC64_SHIFT(SYNTHSHIFT+PCMOUTSHIFT);
#endif        

        outbuf[ch][ 0] = sum1;
        outbuf[ch][16] = sum2;

        v0base +=16;    v1base +=16;    v2base +=16;    v3base +=16;
    }

    if(turn==0)
    {
        d2base = d1base+16;
        d3base = d0base+16;
        v0base = (int*)V +17*32  +32;
        v1base = (int*)V + 0*32+8+32;
        v2base = (int*)V + 0*32+8+32;
        v3base = (int*)V +17*32  +32;
    }
    else
    {
        d2base = d0base+16;
        d3base = d1base+16;
        v0base = (int*)V + 0*32  +32;
        v1base = (int*)V +17*32+8+32;
        v2base = (int*)V + 0*32  +32;
        v3base = (int*)V +17*32+8+32;
    }

    // TO DO , reduce data cache miss
    for(ch=0; ch<nch; ch++)
    {
        for(j=1; j<16; j++)
        {
            int sum1, sum2;
            d0base++;   d1base++;   d2base--;   d3base--;
#if 1            
            sum1 = sum2 = 0;
            WRACC64_ADD(D[d0tab][d0base+0*16], *(v0base+0));
            WRACC64_ADD(D[d1tab][d1base+0*16], *(v1base+0));
            WRACC64_ADD(D[d0tab][d0base+1*16], *(v0base+1));
            WRACC64_ADD(D[d1tab][d1base+1*16], *(v1base+1));
            WRACC64_ADD(D[d0tab][d0base+2*16], *(v0base+2));
            WRACC64_ADD(D[d1tab][d1base+2*16], *(v1base+2));
            WRACC64_ADD(D[d0tab][d0base+3*16], *(v0base+3));
            WRACC64_ADD(D[d1tab][d1base+3*16], *(v1base+3));
            WRACC64_ADD(D[d0tab][d0base+4*16], *(v0base+4));
            WRACC64_ADD(D[d1tab][d1base+4*16], *(v1base+4));
            WRACC64_ADD(D[d0tab][d0base+5*16], *(v0base+5));
            WRACC64_ADD(D[d1tab][d1base+5*16], *(v1base+5));
            WRACC64_ADD(D[d0tab][d0base+6*16], *(v0base+6));
            WRACC64_ADD(D[d1tab][d1base+6*16], *(v1base+6));
            WRACC64_ADD(D[d0tab][d0base+7*16], *(v0base+7));
            WRACC64_ADD(D[d1tab][d1base+7*16], *(v1base+7));
            sum1 = RDACC64_SHIFT(SYNTHSHIFT+PCMOUTSHIFT);

            WRACC64_SUB(D[d2tab][d2base+0*16], *(v2base+0));
            WRACC64_ADD(D[d3tab][d3base+0*16], *(v3base+0));
            WRACC64_SUB(D[d2tab][d2base+1*16], *(v2base+1));
            WRACC64_ADD(D[d3tab][d3base+1*16], *(v3base+1));
            WRACC64_SUB(D[d2tab][d2base+2*16], *(v2base+2));
            WRACC64_ADD(D[d3tab][d3base+2*16], *(v3base+2));
            WRACC64_SUB(D[d2tab][d2base+3*16], *(v2base+3));
            WRACC64_ADD(D[d3tab][d3base+3*16], *(v3base+3));
            WRACC64_SUB(D[d2tab][d2base+4*16], *(v2base+4));
            WRACC64_ADD(D[d3tab][d3base+4*16], *(v3base+4));
            WRACC64_SUB(D[d2tab][d2base+5*16], *(v2base+5));
            WRACC64_ADD(D[d3tab][d3base+5*16], *(v3base+5));
            WRACC64_SUB(D[d2tab][d2base+6*16], *(v2base+6));
            WRACC64_ADD(D[d3tab][d3base+6*16], *(v3base+6));
            WRACC64_SUB(D[d2tab][d2base+7*16], *(v2base+7));
            WRACC64_ADD(D[d3tab][d3base+7*16], *(v3base+7));
            sum2 = RDACC64_SHIFT(SYNTHSHIFT+PCMOUTSHIFT);
#else
            for(i=0; i<8; i++)
            {
                WRACC64_ADD(D[d0tab][d0base+i*16], *(v0base+i));
                WRACC64_ADD(D[d1tab][d1base+i*16], *(v1base+i));
            }
            sum1 = RDACC64_SHIFT(SYNTHSHIFT+PCMOUTSHIFT);
            for(i=0; i<8; i++)
            {
                WRACC64_SUB(D[d2tab][d2base+i*16], *(v2base+i));
                WRACC64_ADD(D[d3tab][d3base+i*16], *(v3base+i));
            }
            sum2 = RDACC64_SHIFT(SYNTHSHIFT+PCMOUTSHIFT);
#endif            

            outbuf[ch][   j] = sum1;
            outbuf[ch][32-j] = sum2;

            v0base +=32;    v1base +=32;    v2base +=32;    v3base +=32;
        }
        d0base -=15;    d1base -=15;    d2base +=15;    d3base +=15;
        v0base -=464;   v1base -=464;   v2base -=464;   v3base -=464;   //-32*15+16
    }
}

#else
void synthesis(int (*V)[2][16], int nch, int DOffset, int (*outbuf)[MAX_FRAMESIZE])
{
    int i;
    int j,ch;
    int r,s;
    int turn;
    int d0tab, d1tab, d2tab, d3tab;
    int d0base, d1base, d2base, d3base;
    int *v0base, *v1base, *v2base, *v3base;

    SETOVFLN(OVFLW_STAGE_SYNTH);

    DOffset = (16 - DOffset) & 0xf;
    turn = (DOffset+1) & 0x1;
    d0tab =(2*DOffset) & 0x3;
    d1tab =(d0tab+2) & 0x3;
    r =(d0tab+1) & 0x3;
    s =(d0tab+3) & 0x3;
    d0base = (DOffset  )/2*16;
    d1base = (DOffset+1)/2*16;

    if(turn==0){
        d2tab = s;
        d3tab = r;
        d2base = (2*DOffset+3)/4*16;
        d3base = (2*DOffset+1)/4*16;
        v0base = (int*)V +17*32;
        v1base = (int*)V + 0*32+8;
        v2base = (int*)V +16*32+8;
        v3base = (int*)V +33*32;
    }
    else{
        d2tab = r;
        d3tab = s;
        d2base = (2*DOffset+1)/4*16;
        d3base = (2*DOffset+3)/4*16;
        v0base = (int*)V + 0*32;
        v1base = (int*)V +17*32+8;
        v2base = (int*)V +16*32;
        v3base = (int*)V +33*32+8;
    }

    for(ch=0; ch<nch; ch++){
        int sum1, sum2;
            sum1 = sum2 = 0;
            for(i=0; i<8; i++){
                sum1 = ADD(sum1, MADDSHIFT(D[d0tab][d0base+i*16], *(v0base+i), D[d1tab][d1base+i*16], *(v1base+i), SYNTHSHIFT));
                sum2 = SUB(sum2, MSUBSHIFT(D[d2tab][d2base+i*16], *(v2base+i), D[d3tab][d3base+i*16], *(v3base+i), SYNTHSHIFT));
            }
        outbuf[ch][ 0] = sum1 >> PCMOUTSHIFT;
        outbuf[ch][16] = sum2 >> PCMOUTSHIFT;

        v0base +=16;    v1base +=16;    v2base +=16;    v3base +=16;
    }

    if(turn==0){
        d2base = d1base+16;
        d3base = d0base+16;
        v0base = (int*)V +17*32  +32;
        v1base = (int*)V + 0*32+8+32;
        v2base = (int*)V + 0*32+8+32;
        v3base = (int*)V +17*32  +32;
    }
    else{
        d2base = d0base+16;
        d3base = d1base+16;
        v0base = (int*)V + 0*32  +32;
        v1base = (int*)V +17*32+8+32;
        v2base = (int*)V + 0*32  +32;
        v3base = (int*)V +17*32+8+32;
    }

    for(ch=0; ch<nch; ch++){
        for(j=1; j<16; j++){
            int sum1, sum2;
            d0base++;   d1base++;   d2base--;   d3base--;
                sum1 = sum2 = 0;
                for(i=0; i<8; i++){
                    sum1 = ADD(sum1, MADDSHIFT(D[d0tab][d0base+i*16], *(v0base+i), D[d1tab][d1base+i*16], *(v1base+i), SYNTHSHIFT));
                    sum2 = SUB(sum2, MSUBSHIFT(D[d2tab][d2base+i*16], *(v2base+i), D[d3tab][d3base+i*16], *(v3base+i), SYNTHSHIFT));
                }
            outbuf[ch][   j] = sum1 >> PCMOUTSHIFT;
            outbuf[ch][32-j] = sum2 >> PCMOUTSHIFT;

            v0base +=32;    v1base +=32;    v2base +=32;    v3base +=32;
        }
        d0base -=15;    d1base -=15;    d2base +=15;    d3base +=15;
        v0base -=464;   v1base -=464;   v2base -=464;   v3base -=464;   //-32*15+16
    }
}
#endif

void SubbandTransformGr(int *outbuf)    //void SubbandTransformGr(short *outbuf)
{
    int b, gr, ch, nch;
    HuffmanInfo *hi;
    IMDCTInfo *mi;
    SubbandInfo *sbi;
    hi = &(mp3DecInfo.HuffmanInfoPS);
    mi = &(mp3DecInfo.IMDCTInfoPS);
    sbi = &(mp3DecInfo.SubbandInfoPS);
    nch = mp3DecInfo.nChans;

    for (gr = 0; gr < mp3DecInfo.nGrans; gr++) 
    {
        for (b = 0; b < mp3DecInfo.nSubbands; b++) 
        {
            sbi->vindex = sbi_vindex[sbi->vindex];
            for(ch=0; ch < nch; ch++)
            {
                DCT_II32(mi->outBuf[gr][ch][b], sbi->vbuf, ch, sbi->vindex);
            }
            synthesis(sbi->vbuf, nch, sbi->vindex, (int(*)[MAX_FRAMESIZE])outbuf);
            outbuf += NBANDS;                   
        }
    }  
}


#ifdef DATA_CACHE_MISS 
    for (gr = 0; gr < mp3DecInfo.nGrans; gr++) 
    {
        for (b = 0; b < mp3DecInfo.nSubbands; b++) 
        {
            sbi->vindex = sbi_vindex[sbi->vindex];
            nTempIndex[nTemp] = sbi->vindex;
            for(ch=0; ch < nch; ch++)
            {
                DCT_II32(mi->outBuf[gr][ch][b], sbi->vbuf, ch, sbi->vindex);
            }
            nTemp++;
               
            if (nTemp % LOOP ==0)
            {
                for (i=0;i<LOOP;i++)
                {
                    synthesis(sbi->vbuf, nch, nTempIndex[i], (int(*)[MAX_FRAMESIZE])outbuf);
                    outbuf += NBANDS;                   
                }
                nTemp = 0;
            }             

        }
    }  
#endif
