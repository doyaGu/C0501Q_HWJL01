/*
 */

#include "stdio.h"
#include "openvg.h"
#include "iteDefs.h"
#include "iteContext.h"
#include "itePath.h"
#include "iteImage.h"
#include "iteGeometry.h"
#include "itePaint.h"
#include "iteVectors.h"
#include "iteHardware.h"

unsigned int CoverageDraw = 0;
unsigned int CoverageClear = 0;
unsigned int CoverageAA	= 0;
unsigned int CoverageNoAA = 0;

/*-----------------------------------------------------------
 * Get the grid point (centrial aligment) which is followed the Left-Top rule
 * eq means that the centrial point belongs its pixel
 *    x-------o
 *-----------------------------------------------------------*/
static ITEint iteGetGridPoint(ITEs15p16 Point) 
{
	ITEint Grid;

	Grid = (ITEint)(Point - 0x8000)>>16;		// follow left-top rule

	return Grid;
}


/*-----------------------------------------------------------
 * Anti-aliasing with Pattern Sample
 * 		1. 8 queens with box filiter.
 *		2. 32 Hammersley points (van der Corput sequence) with box filiter
 *-----------------------------------------------------------*/
static ITEint16 itePatternSample(HWVector2 Point, HWVector2 Top, HWVector2 Mid, HWVector2 Bot, 
						ITEint triangletype, ITEfloat direction, HWRenderingQuality  renderingQuality) {

	ITEint PatternX[32];
	ITEint InsideTriangle[3];
	ITEint Value[32];
	ITEint ReturnValue = 0;
	ITEs12p3 CentrialX;
	ITEs12p3 CentrialY;
	ITEint i,j;
		
	HWVector2 Vector[3];	
	HWVector2 Vertice[3];
	ITEint	  LeftTop[3];

	SET2V(Vertice[0],Top);						// A
	SET2V(Vertice[1],Mid);						// B
	SET2V(Vertice[2],Bot);						// C

	if (triangletype==1 || triangletype==5) {
		SET2(Vector[0], Mid.x-Top.x, Mid.y-Top.y);	// AB
		SET2(Vector[1], Bot.x-Mid.x, Bot.y-Mid.y);	// BC
		SET2(Vector[2], Top.x-Bot.x, Top.y-Bot.y);	// CA
		
	} else if (triangletype==2 || triangletype==6) {
		SET2(Vector[0], Bot.x-Top.x, Bot.y-Top.y);	// AC
		SET2(Vector[1], Top.x-Mid.x, Top.y-Mid.y);	// BA
		SET2(Vector[2], Mid.x-Bot.x, Mid.y-Bot.y);	// CB
	}

	if (triangletype==1 || triangletype==6) {
		LeftTop[0] = 1;
		LeftTop[1] = 0;
		LeftTop[2] = 0;

	} else if (triangletype==2 || triangletype==5) {
		LeftTop[0] = 1;
		LeftTop[1] = 1;
		LeftTop[2] = 0;
	}

	if (renderingQuality == HW_RENDERING_QUALITY_NONANTIALIASED) {

		j = 0;
		CentrialX = 4;
		CentrialY = 4;
				
		for (i=0; i<3; i++) {
			// centrial aligment   (x0+0.5)-(x1+0.5) = x0-x1 
			if ( (ITEint)(CentrialX + Point.x - Vertice[i].x)*Vector[i].y - (ITEint)(CentrialY + Point.y - Vertice[i].y)*Vector[i].x < 0 ) {
				InsideTriangle[i] = 0;
			} else if ( (ITEint)(CentrialX + Point.x - Vertice[i].x)*Vector[i].y - (ITEint)(CentrialY + Point.y - Vertice[i].y)*Vector[i].x==0 
						&& LeftTop[i]==0) {						 
				InsideTriangle[i] = 0;
			} else {
				InsideTriangle[i] = 1;
			}
		}
		Value[j] = InsideTriangle[0] & InsideTriangle[1] & InsideTriangle[2];

		if (direction==1) {
			ReturnValue = Value[j];
		} else {
			ReturnValue = -Value[j];
		}
			
		return (ITEint16)ReturnValue << 5;

	} else if (renderingQuality == HW_RENDERING_QUALITY_FASTER) {

	#if 1
		PatternX[ 0] = 0x0;	//00		// 00
		PatternX[ 1] = 0x2;	//10		// 01
		PatternX[ 2] = 0x1;	//01		// 10
		PatternX[ 3] = 0x3;	//11		// 11
	
		for (j=0; j<4; j++) {
			CentrialX = (ITEs12p3)PatternX[j]<<1;
			CentrialY = (ITEs12p3)j<<1;
			
			for (i=0; i<3; i++) {
				// centrial aligment   (x0+0.5)-(x1+0.5) = x0-x1 
				if ( (ITEint)(CentrialX + Point.x - Vertice[i].x)*Vector[i].y - (ITEint)(CentrialY + Point.y - Vertice[i].y)*Vector[i].x < 0 ) {
					InsideTriangle[i] = 0;
				} else if ( (ITEint)(CentrialX + Point.x - Vertice[i].x)*Vector[i].y - (ITEint)(CentrialY + Point.y - Vertice[i].y)*Vector[i].x==0 
							&& LeftTop[i]==0) {						 
					InsideTriangle[i] = 0;
				} else {
					InsideTriangle[i] = 1;
				}
			}
			Value[j] = InsideTriangle[0] & InsideTriangle[1] & InsideTriangle[2];

		}			

		for (i=0; i<4; i++) {
			if (direction == 1) {
				ReturnValue += Value[i];
			} else {
				ReturnValue -= Value[i];
			}
		}
		
		return (ITEint16)ReturnValue << 3;
	#endif

	#if 0
		// 8 queen
		PatternX[0] = 3;
		PatternX[1] = 7;
		PatternX[2] = 0;
		PatternX[3] = 2;
		PatternX[4] = 5;
		PatternX[5] = 1;
		PatternX[6] = 6;
		PatternX[7] = 4;
			
		for (j=0; j<8; j++) {
			CentrialX = (ITEs12p3)PatternX[j];
			CentrialY = (ITEs12p3)j;
				
			for (i=0; i<3; i++) {
				// centrial aligment   (x0+0.5)-(x1+0.5) = x0-x1 
				if ( (ITEint)(CentrialX + Point.x - Vertice[i].x)*Vector[i].y - (ITEint)(CentrialY + Point.y - Vertice[i].y)*Vector[i].x < 0 ) {
					InsideTriangle[i] = 0;
				} else if ( (ITEint)(CentrialX + Point.x - Vertice[i].x)*Vector[i].y - (ITEint)(CentrialY + Point.y - Vertice[i].y)*Vector[i].x==0 
							&& LeftTop[i]==0) {						 
					InsideTriangle[i] = 0;
				} else {
					InsideTriangle[i] = 1;
				}
			}
			Value[j] = InsideTriangle[0] & InsideTriangle[1] & InsideTriangle[2];
		}			

		for (i=0; i<8; i++) {
			if (direction == 1) {
				ReturnValue += Value[i];
			} else {
				ReturnValue -= Value[i];
			}
		}
		
		return (ITEint16)ReturnValue << 2;
	#endif

	} else {
	#if 1
	// The van der Corput sequence 16
		PatternX[ 0] = 0x0;	//0000		// 0000
		PatternX[ 1] = 0x8;	//1000		// 0001
		PatternX[ 2] = 0x4;	//0100		// 0010
		PatternX[ 3] = 0xC;	//1100		// 0011
		PatternX[ 4] = 0x2;	//0010		// 0100
		PatternX[ 5] = 0xA;	//1010		// 0101
		PatternX[ 6] = 0x6;	//0110		// 0110
		PatternX[ 7] = 0xE;	//1110		// 0111
		PatternX[ 8] = 0x1;	//0001		// 1000
		PatternX[ 9] = 0x9;	//1001		// 1001
		PatternX[10] = 0x5;	//0101		// 1010
		PatternX[11] = 0xD;	//1101		// 1011
		PatternX[12] = 0x3;	//0011		// 1100
		PatternX[13] = 0xB;	//1011		// 1101
		PatternX[14] = 0x7;	//0111		// 1110
		PatternX[15] = 0xF;	//1111		// 1111

		for (j=0; j<16; j++) {
			CentrialX = PatternX[j]>>1;
			CentrialY = j>>1;
		
			for (i=0; i<3; i++) {
				// centrial aligment   (x0+0.5)-(x1+0.5) = x0-x1 
				if ( (ITEint)(CentrialX + Point.x - Vertice[i].x)*Vector[i].y - (ITEint)(CentrialY + Point.y - Vertice[i].y)*Vector[i].x < 0 ) {
					InsideTriangle[i] = 0;
				} else if ( (ITEint)(CentrialX + Point.x - Vertice[i].x)*Vector[i].y - (ITEint)(CentrialY + Point.y - Vertice[i].y)*Vector[i].x==0 
							&& LeftTop[i]==0) {						 
					InsideTriangle[i] = 0;
				} else {
					InsideTriangle[i] = 1;
				}
			}
			Value[j] = InsideTriangle[0] & InsideTriangle[1] & InsideTriangle[2];
		}			


		for (i=0; i<16; i++) {
			if (direction == 1) {
				ReturnValue += Value[i];
			} else {
				ReturnValue -= Value[i];
			}
		}

		return (ITEint16)ReturnValue << 1;
	#endif

	#if 0
	// The van der Corput sequence 32
		PatternX[ 0] = 0x00;	//00000		// 00000
		PatternX[ 1] = 0x10;	//10000		// 00001
		PatternX[ 2] = 0x08;	//01000		// 00010
		PatternX[ 3] = 0x18;	//11000		// 00011
		PatternX[ 4] = 0x04;	//00100		// 00100
		PatternX[ 5] = 0x14;	//10100		// 00101
		PatternX[ 6] = 0x0C;	//01100		// 00110
		PatternX[ 7] = 0x1C;	//11100		// 00111
		PatternX[ 8] = 0x02;	//00010		// 01000
		PatternX[ 9] = 0x12;	//10010		// 01001
		PatternX[10] = 0x0A;	//01010		// 01010
		PatternX[11] = 0x1A;	//11010		// 01011
		PatternX[12] = 0x06;	//00110		// 01100
		PatternX[13] = 0x16;	//10110		// 01101
		PatternX[14] = 0x0E;	//01110		// 01110
		PatternX[15] = 0x1E;	//11110		// 01111
		PatternX[16] = 0x01;	//00001		// 10000
		PatternX[17] = 0x11;	//10001		// 10001
		PatternX[18] = 0x09;	//01001		// 10010
		PatternX[19] = 0x19;	//11001		// 10011
		PatternX[20] = 0x05;	//00101		// 10100
		PatternX[21] = 0x15;	//10101		// 10101
		PatternX[22] = 0x0D;	//01101		// 10110
		PatternX[23] = 0x1D;	//11101		// 10111
		PatternX[24] = 0x03;	//00011		// 11000
		PatternX[25] = 0x13;	//10011		// 11001
		PatternX[26] = 0x0B;	//01011		// 11010
		PatternX[27] = 0x1B;	//11011		// 11011
		PatternX[28] = 0x07;	//00111		// 11100
		PatternX[29] = 0x17;	//10111		// 11101
		PatternX[30] = 0x0F;	//01111		// 11110
		PatternX[31] = 0x1F;	//11111		// 11111

			
		for (j=0; j<32; j++) {
			CentrialX = PatternX[j]>>2;
			CentrialY = j>>2;
				
			for (i=0; i<3; i++) {
				// centrial aligment   (x0+0.5)-(x1+0.5) = x0-x1 
				if ( (ITEint)(CentrialX + Point.x - Vertice[i].x)*Vector[i].y - (ITEint)(CentrialY + Point.y - Vertice[i].y)*Vector[i].x < 0 ) {
					InsideTriangle[i] = 0;
				} else if ( (ITEint)(CentrialX + Point.x - Vertice[i].x)*Vector[i].y - (ITEint)(CentrialY + Point.y - Vertice[i].y)*Vector[i].x==0 
							&& LeftTop[i]==0) {						 
					InsideTriangle[i] = 0;
				} else {
					InsideTriangle[i] = 1;
				}
			}
			Value[j] = InsideTriangle[0] & InsideTriangle[1] & InsideTriangle[2];
		}			


		for (i=0; i<32; i++) {
			if (direction == 1) {
				ReturnValue += Value[i];
			} else {
				ReturnValue -= Value[i];
			}
		}

		return (ITEint16)ReturnValue;
	#endif
	}

/*
	} else {
	// 4 8-queen patterns from WIKI
		PatternX[ 0 + 0] = 4;
		PatternX[ 4 + 0] = 1;
		PatternX[ 8 + 0] = 3;
		PatternX[12 + 0] = 6;
		PatternX[16 + 0] = 2;
		PatternX[20 + 0] = 7;
		PatternX[24 + 0] = 5;
		PatternX[28 + 0] = 0;

		PatternX[ 0 + 1] = 3;
		PatternX[ 4 + 1] = 6;
		PatternX[ 8 + 1] = 2;
		PatternX[12 + 1] = 7;
		PatternX[16 + 1] = 1;
		PatternX[20 + 1] = 4;
		PatternX[24 + 1] = 0;
		PatternX[28 + 1] = 5;

		PatternX[ 0 + 2] = 5;
		PatternX[ 4 + 2] = 3;
		PatternX[ 8 + 2] = 6;
		PatternX[12 + 2] = 0;
		PatternX[16 + 2] = 7;
		PatternX[20 + 2] = 1;
		PatternX[24 + 2] = 4;
		PatternX[28 + 2] = 2;

		PatternX[ 0 + 3] = 2;
		PatternX[ 4 + 3] = 5;
		PatternX[ 8 + 3] = 7;
		PatternX[12 + 3] = 0;
		PatternX[16 + 3] = 3;
		PatternX[20 + 3] = 6;
		PatternX[24 + 3] = 4;
		PatternX[28 + 3] = 1;
		
		for (j=0; j<32; j++) {
			CentrialX = PatternX[j];
			CentrialY = j>>2;
				
			for (i=0; i<3; i++) {
				// centrial aligment   (x0+0.5)-(x1+0.5) = x0-x1 
				if ( (ITEint)(CentrialX + Point.x - Vertice[i].x)*Vector[i].y - (ITEint)(CentrialY + Point.y - Vertice[i].y)*Vector[i].x < 0 ) {
					InsideTriangle[i] = 0;
				} else if ( (ITEint)(CentrialX + Point.x - Vertice[i].x)*Vector[i].y - (ITEint)(CentrialY + Point.y - Vertice[i].y)*Vector[i].x==0 
							&& LeftTop[i]==0) {						 
					InsideTriangle[i] = 0;
				} else {
					InsideTriangle[i] = 1;
				}
			}
			Value[j] = InsideTriangle[0] & InsideTriangle[1] & InsideTriangle[2];
		}			


		for (i=0; i<32; i++) {
			if (direction == 1) {
				ReturnValue += Value[i];
			} else {
				ReturnValue -= Value[i];
			}
		}

		return (ITEint16)ReturnValue;
	}
*/	
}


static void iteHardwareCache(ITEint16 *coverage, ITEuint offset, ITEuint8 *valid, ITEuint validoffset, 
							 ITEuint bitoffset, ITECache16 *CoverageCache, ITECache8 *ValidCache, ITEint16 word)
{
	ITEint8 i;
	ITEint8 *CovMark, *ValMark;
	ITEint16 *memAddress;
	ITEuint32 addr;
	ITEuint32 tag;

	ITEint8 *validAddress;
	ITEuint8 Validbyte, Validbit;
	ITEint8 CurrSet = 0, Hit = 0;
	ITEuint32 validaddr;
	ITEuint32 validtag;


	validAddress = valid + validoffset;
	memAddress = coverage + offset;

	if (CACHE == 1) {
		validaddr = (ITEuint32)validAddress & (CACHESIZE-1);
		validtag = (ITEuint32)validAddress >> ADDRESSBIT;
		ValMark = &ValidCache[validaddr].replaceMark;

		// if hit --> read data from cache.
		// if no hit and cache no valid --> read data from memory to cache.
		// if no hit and cache valid --> write back to memory then read data from memory to cache.
		
		for (i=0; i<CACHESET; i++) {
			if (ValidCache[validaddr].valid[i] == 1 && ValidCache[validaddr].tag[i] == validtag) {
				Validbyte = ValidCache[validaddr].word[i];		
				Hit = 1;
				CurrSet = i;
				break;
			}						
		}

		if (Hit == 0) {
			if (ValidCache[validaddr].valid[*ValMark] == 1) {

				*( (ITEuint8*)(validaddr + (ValidCache[validaddr].tag[*ValMark]<<ADDRESSBIT)) ) = ValidCache[validaddr].word[*ValMark];
			}

			Validbyte = *validAddress;
			CurrSet = *ValMark;
			
			if (*ValMark == (CACHESET-1)) {
				*ValMark = 0;
			} else {
				*ValMark ++;
			}
		}

		if (word == 0) {
			ValidCache[validaddr].tag[CurrSet] = validtag;
			ValidCache[validaddr].valid[CurrSet] = 1;
			ValidCache[validaddr].word[CurrSet] = Validbyte;
			return;
		}

	} else {

		Validbyte = *(validAddress);
		if (word == 0)	return;
	}	

	Validbit = Validbyte & (1 << bitoffset);

	if (CACHE == 1) {
		addr = (ITEuint32)memAddress & (CACHESIZE-1);
		tag = (ITEuint32)memAddress >> ADDRESSBIT;
		CovMark = &CoverageCache[addr].replaceMark;

		for (i=0; i<CACHESET; i++) {
			if (CoverageCache[addr].valid[i] == 1 && CoverageCache[addr].tag[i] == tag) {

				if (Validbit == 0) {
					CoverageCache[addr].word[i] = word;	


					ValidCache[validaddr].word[CurrSet] = Validbyte;

				} else {
					CoverageCache[addr].word[i] += word;		
				}

				if (*CovMark == (CACHESET-1)) {
					*CovMark = 0;
				} else {
					*CovMark ++;
				}
				return;
			}		
		}

		// add another flag to mark the first loading data 0
		// if this first loading data is still 0, do not write back when replaced cache
		if (CoverageCache[addr].valid[*CovMark] == 1) {

			*( (ITEint16*)(addr + (CoverageCache[addr].tag[*CovMark]<<ADDRESSBIT)) ) = CoverageCache[addr].word[*CovMark];
			if (COVERAGEDRAW) CoverageDraw++;
		}

		CoverageCache[addr].tag[*CovMark] = tag;

		ValidCache[validaddr].tag[CurrSet] = validtag;
		ValidCache[validaddr].valid[CurrSet] = 1;

		if (Validbit == 0) {
			CoverageCache[addr].word[*CovMark] = word;
			CoverageCache[addr].valid[*CovMark] = 1;
			
			ValidCache[validaddr].word[CurrSet] = Validbyte | (1 << bitoffset);

		} else {
			word += *memAddress;

			if (word == 0) {
				CoverageCache[addr].valid[*CovMark] = 0;			
			
				// We don't know others three coverage pixels are zeros or not.
				// So we don't clear the valid plane bit.
				//ValidCache[validaddr].word[CurrSet] = Validbyte & ~(1 << bitoffset);
		
				ValidCache[validaddr].word[CurrSet] = Validbyte & ~(1 << bitoffset);
			} else {
				CoverageCache[addr].word[*CovMark] = word;
				CoverageCache[addr].valid[*CovMark] = 1;
			
				ValidCache[validaddr].word[CurrSet] = Validbyte | (1 << bitoffset);
			}
		}

		if (*CovMark == (CACHESET-1)) {
			*CovMark = 0;
		} else {
			*CovMark ++;
		}
		return;

	} else {

		if (Validbit == 0) {
			*(memAddress) = word;
			*(validAddress) = Validbyte | (1 << bitoffset);
		} else {
			word += *memAddress;

			if (word == 0) {
				// We don't know others three coverage pixels are zeros or not.
				// So we don't clear the valid plane bit.
				*(validAddress) = Validbyte & ~(1 << bitoffset);

				//*(validAddress) = Validbyte | (1 << bitoffset);
			} else {
				*(memAddress) = word;
				*(validAddress) = Validbyte | (1 << bitoffset);
			}
		}
		
		if (COVERAGEDRAW) CoverageDraw++;
		return;
	}
}

static void iteCacheWriteBack(ITECache16 *CoverageCache, ITECache8 *ValidCache)
{
	ITEint16 i, j;

	for (i=0; i<CACHESET; i++) {
		for (j=0; j<CACHESIZE; j++) {

			if (CoverageCache[j].valid[i] == 1)	{ 
				*( (ITEint16*)(j + (CoverageCache[j].tag[i]<<ADDRESSBIT)) ) = CoverageCache[j].word[i];
				if (COVERAGEDRAW) CoverageDraw++;
			}

			if (ValidCache[j].valid[i] == 1)	{ 
				*( (ITEuint8*)(j + (ValidCache[j].tag[i]<<ADDRESSBIT)) ) = ValidCache[j].word[i];
			}
		}
	}

}
/*-----------------------------------------------------------
 * Fill the triangle in coverage buffer by three points
 * Anti-aliasing with sample pattern
 * point: float   coverage buffer: s10.5
 *-----------------------------------------------------------*/
static void iteFillTriangles(HWVector2 *inA, HWVector2 *inB, HWVector2 *inC, HWVector2 *Base, 
							 ITEint16 *coverage, ITEint width, ITEuint8 *valid, ITEint16 validpitch,
							 HWRenderingQuality renderingQuality, ITECache16 *CoverageCache,
							 ITECache8 *ValidCache)
{
	ITEint16 direction = 0;
	ITEint16 CoverageValue = 0;
	ITEint triangletype;
	
	HWVector2 A, B, C;
	HWVector2 AtoB;
	HWVector2 BtoC;
	HWVector2 AtoC;
	HWVector2 P;

	BOOL AmoreB;
	BOOL BmoreC;
	BOOL AmoreC;
	
	HWVector2 Top;
	HWVector2 Mid;
	HWVector2 Bot;

	ITEs15p16 SlopeT2M;
	ITEs15p16 SlopeM2B;
	ITEs15p16 SlopeT2B;

	BOOL testT2M = 0;
	BOOL testM2B = 0;
	BOOL testT2B = 0;

	HWPXVector2 LeftPoint;
	HWPXVector2 RightPoint;
	HWPXVector2 MidPoint;

	ITEs15p16 LeftSlope;
	ITEs15p16 RightSlope;

	ITEs15p16 EdgeStart;
	ITEs15p16 StartX;
	ITEs15p16 EndX;
	ITEs15p16 EdgeEnd;

	ITEs12p3 TopDistant;
	ITEs12p3 MidDistant;

	ITEint x, y;

	// move to the center of pixels
	//SET2(A, (ITEfloat)floor(inA->x * 32 + 0.5f)/32 + 0.5f, (ITEfloat)floor(inA->y * 32 + 0.5f)/32 + 0.5f);
	//SET2(B, (ITEfloat)floor(inB->x * 32 + 0.5f)/32 + 0.5f, (ITEfloat)floor(inB->y * 32 + 0.5f)/32 + 0.5f);
	//SET2(C, (ITEfloat)floor(inC->x * 32 + 0.5f)/32 + 0.5f, (ITEfloat)floor(inC->y * 32 + 0.5f)/32 + 0.5f);
	SET2(A, inA->x+0x4, inA->y+0x4);
	SET2(B, inB->x+0x4, inB->y+0x4);
	SET2(C, inC->x+0x4, inC->y+0x4);

	SET2(AtoB, B.x - A.x, B.y - A.y);
	SET2(BtoC, C.x - B.x, C.y - B.y);
	SET2(AtoC, C.x - A.x, C.y - A.y);

	// direction
	if (AtoB.x * BtoC.y < BtoC.x * AtoB.y) {
		direction = 1;		//CW
	} else {
		direction = -1;		//CCW
	}

	// Reorder
	if (AtoB.y == 0) {
		if (AtoB.x == 0) {
			AmoreB = 0;
		} else if (AtoB.x > 0) {
			AmoreB = 0;
		} else if (AtoB.x < 0) {
			AmoreB = 1;
		}
	} else if (AtoB.y > 0) {
		AmoreB = 0;
	} else if (AtoB.y < 0) {
		AmoreB = 1;
	}

	if (BtoC.y == 0) {
		if (BtoC.x == 0) {
			BmoreC = 0;
		} else if (BtoC.x > 0) {
			BmoreC = 0;
		} else if (BtoC.x < 0) {
			BmoreC = 1;
		}
	} else if (BtoC.y > 0) {
		BmoreC = 0;
	} else if (BtoC.y < 0) {
		BmoreC = 1;
	}

	if (AtoC.y == 0) {
		if (AtoC.x == 0) {
			AmoreC = 0;
		} else if (AtoC.x > 0) {
			AmoreC = 0;
		} else if (AtoC.x < 0) {
			AmoreC = 1;
		}
	} else if (AtoC.y > 0) {
		AmoreC = 0;
	} else if (AtoC.y < 0) {
		AmoreC = 1;
	}

	if (AmoreB == 0 && BmoreC == 0) {							// 000 001
		SET2(Top, A.x, A.y);
		SET2(Mid, B.x, B.y);
		SET2(Bot, C.x, C.y);
	} else if (AmoreB == 0 && BmoreC == 1 && AmoreC == 0) {		// 010
		SET2(Top, A.x, A.y);
		SET2(Mid, C.x, C.y);
		SET2(Bot, B.x, B.y);
	} else if (AmoreB == 0 && BmoreC == 1 && AmoreC == 1) {		// 011
		SET2(Top, C.x, C.y);
		SET2(Mid, A.x, A.y);
		SET2(Bot, B.x, B.y);
	} else if (AmoreB == 1 && BmoreC == 0 && AmoreC == 0) {		// 100
		SET2(Top, B.x, B.y);
		SET2(Mid, A.x, A.y);
		SET2(Bot, C.x, C.y);
	} else if (AmoreB == 1 && BmoreC == 0 && AmoreC == 1) {		// 101
		SET2(Top, B.x, B.y);
		SET2(Mid, C.x, C.y);
		SET2(Bot, A.x, A.y);
	} else {													// 110 111
		SET2(Top, C.x, C.y);
		SET2(Mid, B.x, B.y);
		SET2(Bot, A.x, A.y);
	}	

	//determine triangle type
	if (Top.y == Mid.y && Mid.y == Bot.y) {
		triangletype = 0;
		return;
	} else if (Mid.y == Bot.y) {
		triangletype = 1;
	} else if (Top.y == Mid.y) {
		triangletype = 2;
	} else if (Top.x == Mid.x && Mid.x == Bot.x) {
		triangletype = 3;
		return;
	} else if (Mid.x < Top.x && Mid.x < Bot.x) {
		triangletype = 5;
	} else if (Mid.x > Top.x && Mid.x > Bot.x) {
		triangletype = 6;
	} else {
		if ( ((ITEint)(Top.y-Mid.y)*(Top.x-Bot.x) - (ITEint)(Top.y-Bot.y)*(Top.x-Mid.x)) == 0 ) {
			triangletype = 4;
			return;
		} else if ( ((ITEint)(Top.y-Mid.y)*(Top.x-Bot.x) - (ITEint)(Top.y-Bot.y)*(Top.x-Mid.x)) > 0) {
			triangletype = 5;
		} else {
			triangletype = 6;
		}
	}

	// find the slopes
	if (Top.y == Mid.y)		testT2M = 1;
	if (Mid.y == Bot.y)		testM2B = 1;
	if (Top.y == Bot.y)		testT2B = 1;	
	
	if (testT2M == 0) {
		SlopeT2M = (ITEs15p16)( (Top.x - Mid.x)<<16 ) / (ITEs15p16)(Top.y - Mid.y);     // s12.19/s12.3 = s15.16   
	}

	if (testM2B == 0) {
		SlopeM2B = (ITEs15p16)( (Mid.x - Bot.x)<<16 )  / ((ITEs15p16)Mid.y - Bot.y);    // s12.19/s12.3 = s15.16     
	}

	if (testT2B == 0) {
		SlopeT2B = (ITEs15p16)( (Top.x - Bot.x)<<16 ) / (ITEs15p16)(Top.y - Bot.y);     // s12.19/s12.3 = s15.16  
	}

	// fill the triangle and using sample pattern
	// find the intersection of line and pixel boundary.
	TopDistant = 0x8 - (Top.y&0x7);
	MidDistant = 0x8 - (Mid.y&0x7);

	// To find first start point
	if (triangletype == 1) {
		SET2(LeftPoint, (ITEs15p16)(Top.x<<13) + (ITEs15p16)(SlopeT2M*TopDistant>>3), (Top.y>>3)<<3);	
		SET2(RightPoint, (ITEs15p16)(Top.x<<13) + (ITEs15p16)(SlopeT2B*TopDistant>>3), (Top.y>>3)<<3);

		LeftSlope   	= SlopeT2M;
		RightSlope 		= SlopeT2B;

	} else if (triangletype == 2) {
		SET2(LeftPoint, (ITEs15p16)(Top.x<<13) + (ITEs15p16)(SlopeT2B*MidDistant>>3) , (Top.y>>3)<<3);	
		SET2(RightPoint, (ITEs15p16)(Mid.x<<13) + (ITEs15p16)(SlopeM2B*MidDistant>>3) , (Mid.y>>3)<<3);

		LeftSlope   	= SlopeT2B;
		RightSlope 		= SlopeM2B;

	} else if (triangletype == 5) {
		//SET2(LeftPoint, (ITEs15p16)(Top.x<<13) + (ITEs15p16)(SlopeT2M*TopDistant>>3), (Top.y>>3)<<3);	
		//SET2(RightPoint, (ITEs15p16)(Top.x<<13) + (ITEs15p16)(SlopeT2B*TopDistant>>3), (Top.y>>3)<<3);

		// Top and Mid at the same pixel in y direction
		if ( (Top.y>>3)==(Mid.y>>3) ) {
			if ( (SlopeT2M>=0 && SlopeM2B<0) || (SlopeT2M<0 && SlopeM2B>=0) ) {
				SET2(LeftPoint, (ITEs15p16)(Mid.x<<13), (Top.y>>3)<<3);	
			} else {
				SET2(LeftPoint, (ITEs15p16)(Mid.x<<13) + (ITEs15p16)(SlopeM2B*MidDistant>>3), (Top.y>>3)<<3);
			}
		} else {
			SET2(LeftPoint, (ITEs15p16)(Top.x<<13) + (ITEs15p16)(SlopeT2M*TopDistant>>3), (Top.y>>3)<<3);	
		}
	
		// Top and Bot at the same pixel in y direction
		if ( (Top.y>>3)==(Bot.y>>3) ) {
			SET2(RightPoint, (ITEs15p16)(Bot.x<<13), (Top.y>>3)<<3);	
		} else {
			SET2(RightPoint, (ITEs15p16)(Top.x<<13) + (ITEs15p16)(SlopeT2B*TopDistant>>3), (Top.y>>3)<<3);
		}
				
		LeftSlope   	= SlopeT2M;
		RightSlope 		= SlopeT2B;
	
	} else if (triangletype == 6) {
		//SET2(LeftPoint, (ITEs15p16)(Top.x<<13) + (ITEs15p16)(SlopeT2B*TopDistant>>3) , (Top.y>>3)<<3);
		//SET2(RightPoint, (ITEs15p16)(Top.x<<13) + (ITEs15p16)(SlopeT2M*TopDistant>>3) , (Top.y>>3)<<3);
		
		// Top and Bot at the same pixel in y direction
		if ( (Top.y>>3)==(Bot.y>>3) ) {
			SET2(LeftPoint, (ITEs15p16)(Bot.x<<13), (Top.y>>3)<<3);	
		} else {
			SET2(LeftPoint, (ITEs15p16)(Top.x<<13) + (ITEs15p16)(SlopeT2B*TopDistant>>3) , (Top.y>>3)<<3);
		}

		// Top and Mid at the same pixel in y direction
		if ( (Top.y>>3)==(Mid.y>>3) ) {
			if ( (SlopeT2M>=0 && SlopeM2B<0) || (SlopeT2M<0 && SlopeM2B>=0) ) {
				SET2(RightPoint, (ITEs15p16)(Mid.x<<13), (Top.y>>3)<<3);	
			} else {
				SET2(RightPoint, (ITEs15p16)(Mid.x<<13) + (ITEs15p16)(SlopeM2B*MidDistant>>3), (Top.y>>3)<<3);
			}
		} else {
			SET2(RightPoint, (ITEs15p16)(Top.x<<13) + (ITEs15p16)(SlopeT2M*TopDistant>>3) , (Top.y>>3)<<3);
		}
	
		LeftSlope   	= SlopeT2B;
		RightSlope 		= SlopeT2M;
	}

	y = (LeftPoint.y)>>3;

	// Scan Top line
	// Top state
	if (y == (ITEint)(Top.y>>3)) {

		if (LeftSlope < 0) {
			EdgeStart	= LeftPoint.x;
			StartX      = (ITEs15p16)Top.x<<13;
		} else {
			EdgeStart	= (ITEs15p16)Top.x<<13;
			StartX      = LeftPoint.x;
		}

		if (triangletype == 2) { 
			if (RightSlope >= 0) {
				EndX 		= (ITEs15p16)Mid.x<<13;
				EdgeEnd		= RightPoint.x;
			} else {
				EndX 		= RightPoint.x;
				EdgeEnd		= (ITEs15p16)Mid.x<<13;
			}
		} else {
			if (RightSlope >= 0) {
				EndX 		= (ITEs15p16)Top.x<<13;
				EdgeEnd		= RightPoint.x;
			} else {
				EndX 		= RightPoint.x;
				EdgeEnd		= (ITEs15p16)Top.x<<13;
			}
		}

		for (x= (ITEint)(EdgeStart>>16); x <= (ITEint)(EdgeEnd>>16); x++) {
			if ((x-(int)(Base->x>>3)) >= 0 && (x-(int)(Base->x>>3)) < width) {
				ITEuint offset = (x-(int)(Base->x>>3)) + (y-(int)(Base->y>>3))*width;
				// 1 valid bit for 4 coverage pixels
				//ITEuint validoffset = ( (x>>5)-((int)(Base->x>>3)>>5) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
				//ITEuint bitoffset = (x>>2) & 0x7;
				ITEuint validoffset = ( (x>>3)-((int)(Base->x>>3)>>3) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
				ITEuint bitoffset = x & 0x7;
				SET2(P, (ITEs12p3)(x<<3), (ITEs12p3)(y<<3));
				CoverageValue = itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
				iteHardwareCache(coverage, offset, valid, validoffset, bitoffset, CoverageCache, ValidCache, CoverageValue);
				if (COVERAGEDRAW) CoverageAA++;
				//*(coverage+offset) += itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
			}
		}
		y++;
	}

	// Scan line from Top to Mid
	// T2M state
	for ( ; y<(ITEint)(Mid.y>>3); y++) {
		if (LeftSlope < 0) {
			StartX      = EdgeStart;
			EdgeStart	= EdgeStart + LeftSlope;
		} else {
			EdgeStart	= StartX;
			StartX      = StartX + LeftSlope;
		}

		if (RightSlope < 0) {
			EdgeEnd		= EndX;
			EndX 		= EndX + RightSlope;
		} else {
			EndX 		= EdgeEnd;
			EdgeEnd		= EdgeEnd + RightSlope;
		}

		x = (ITEint)(EdgeStart>>16);
			
		for ( ; x <= (ITEint)(StartX>>16); x++) {
			if ((x-(int)(Base->x>>3)) >= 0 && (x-(int)(Base->x>>3)) < width) {	
				ITEuint offset = (x-(int)(Base->x>>3)) + (y-(int)(Base->y>>3))*width;
				// 1 valid bit for 4 coverage pixels
				//ITEuint validoffset = ( (x>>5)-((int)(Base->x>>3)>>5) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
				//ITEuint bitoffset = (x>>2) & 0x7;
				ITEuint validoffset = ( (x>>3)-((int)(Base->x>>3)>>3) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
				ITEuint bitoffset = x & 0x7;
				SET2(P,(ITEs12p3)(x<<3), (ITEs12p3)(y<<3));
				CoverageValue = itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
				iteHardwareCache(coverage, offset, valid, validoffset, bitoffset, CoverageCache, ValidCache, CoverageValue);
				if (COVERAGEDRAW) CoverageAA++;
				//*(coverage+offset) += itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
			}
		}

		for ( ; x < (ITEint)(EndX>>16); x++) {
			if ((x-(int)(Base->x>>3)) >= 0 && (x-(int)(Base->x>>3)) < width) {
				ITEuint offset = (x-(int)(Base->x>>3)) + (y-(int)(Base->y>>3))*width;
				// 1 valid bit for 4 coverage pixels
				//ITEuint validoffset = ( (x>>5)-((int)(Base->x>>3)>>5) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
				//ITEuint bitoffset = (x>>2) & 0x7;
				ITEuint validoffset = ( (x>>3)-((int)(Base->x>>3)>>3) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
				ITEuint bitoffset = x & 0x7;
				CoverageValue = direction<<5;
				iteHardwareCache(coverage, offset, valid, validoffset, bitoffset, CoverageCache, ValidCache, CoverageValue);
				if (COVERAGEDRAW) CoverageNoAA++;
				//*(coverage+offset) += (direction<<5);
			}
		}

		for (; x <= (ITEint)(EdgeEnd>>16); x++) {
			if ((x-(int)(Base->x>>3)) >= 0 && (x-(int)(Base->x>>3)) < width) {	
				ITEuint offset = (x-(int)(Base->x>>3)) + (y-(int)(Base->y>>3))*width;
				// 1 valid bit for 4 coverage pixels
				//ITEuint validoffset = ( (x>>5)-((int)(Base->x>>3)>>5) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
				//ITEuint bitoffset = (x>>2) & 0x7;
				ITEuint validoffset = ( (x>>3)-((int)(Base->x>>3)>>3) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
				ITEuint bitoffset = x & 0x7;
				SET2(P,(ITEs12p3)(x<<3), (ITEs12p3)(y<<3));
				CoverageValue = itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
				iteHardwareCache(coverage, offset, valid, validoffset, bitoffset, CoverageCache, ValidCache, CoverageValue);
				if (COVERAGEDRAW) CoverageAA++;
				//*(coverage+offset) += itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
			}
		}
	}

	// Scan Mid line
	if (y == (ITEint)(Mid.y>>3)) {

		// MID0 state
		if (triangletype == 1) {
			if (LeftSlope < 0) {
				StartX      = EdgeStart;
				EdgeStart	= (ITEs15p16)(Mid.x<<13);
			} else {
				EdgeStart	= StartX;
				StartX      = (ITEs15p16)(Mid.x<<13);
			}

			if (RightSlope < 0) {
				EdgeEnd		= EndX;
				EndX 		= (ITEs15p16)(Bot.x<<13);
			} else {
				EndX 		= EdgeEnd;
				EdgeEnd		= (ITEs15p16)(Bot.x<<13);
			}
		} else if (triangletype == 2) {
			if (LeftSlope < 0) {
				StartX      = EdgeStart;
				EdgeStart	= (ITEs15p16)(Bot.x<<13);
			} else {
				EdgeStart	= StartX;
				StartX      = (ITEs15p16)(Bot.x<<13);
			}

			if (RightSlope < 0) {
				EdgeEnd		= EndX;
				EndX 		= (ITEs15p16)(Bot.x<<13);
			} else {
				EndX 		= EdgeEnd;
				EdgeEnd		= (ITEs15p16)(Bot.x<<13);
			}
		} else {
			if (LeftSlope < 0) {
				StartX      = EdgeStart;
				EdgeStart	= EdgeStart + LeftSlope;
			} else {
				EdgeStart	= StartX;
				StartX      = StartX + LeftSlope;
			}

			if (RightSlope < 0) {
				EdgeEnd		= EndX;
				EndX 		= EndX + RightSlope;
			} else {
				EndX 		= EdgeEnd;
				EdgeEnd		= EdgeEnd + RightSlope;
			}
		}

		// MID1 state
		if (triangletype == 5) {
			// Mid and Bot at the same pixel in y direction
			if ( (Mid.y>>3)==(Bot.y>>3) ) {
				SET2(MidPoint, (ITEs15p16)(Bot.x<<13), (Mid.y>>3)<<3);
			} else {
				SET2(MidPoint, (ITEs15p16)(Mid.x<<13) + (ITEs15p16)(SlopeM2B*MidDistant>>3), (Mid.y>>3)<<3);
			}

			//SET2(MidPoint, (ITEs15p16)(Mid.x<<13) + (ITEs15p16)(SlopeM2B*MidDistant>>3), (Mid.y>>3)<<3);
			LeftSlope  		= SlopeM2B;

			if (LeftSlope < 0) {
				EdgeStart	= MidPoint.x;
				StartX		= StartX;
			} else {
				if (SlopeT2M > 0)
					EdgeStart	= EdgeStart;
				else
					EdgeStart	= (Mid.x<<13);
				//EdgeStart	= EdgeStart;


				if (MidPoint.x > StartX) 
					StartX	= MidPoint.x;
				else
					StartX	= StartX;
			}

			// Mid and Bot at the same pixel in y direction
			if ( (Mid.y>>3)==(Bot.y>>3) ) {
				if (RightSlope < 0) {
					EndX		= (Bot.x<<13);
					EdgeEnd		= EdgeEnd;
				} else {
					EndX		= EndX;
					EdgeEnd		= (Bot.x<<13);
				}
			}
		} 
			
		if (triangletype == 6) {
			// Mid and Bot at the same pixel in y direction
			if ( (Mid.y>>3)==(Bot.y>>3) ) {
				SET2(MidPoint, (ITEs15p16)(Bot.x<<13), (Mid.y>>3)<<3);
			} else {
				SET2(MidPoint, (ITEs15p16)(Mid.x<<13) + (ITEs15p16)(SlopeM2B*MidDistant>>3), (Mid.y>>3)<<3);		
			}

			//SET2(MidPoint, (ITEs15p16)(Mid.x<<13) + (ITEs15p16)(SlopeM2B*MidDistant>>3), (Mid.y>>3)<<3);
			RightSlope 		= SlopeM2B;

			// Mid and Bot at the same pixel in y direction
			if ( (Mid.y>>3)==(Bot.y>>3) ) {
				if (LeftSlope > 0) {
					EdgeStart	= EdgeStart;
					StartX		= (Bot.x<<13);
				} else {
					EdgeStart	= (Bot.x<<13);
					StartX		= StartX;
				}
			}

			if (RightSlope < 0) {
				if (MidPoint.x < EndX) 
					EndX	= MidPoint.x;
				else 
					EndX	= EndX;
	
				if (SlopeT2M < 0)
					EdgeEnd	= EdgeEnd;
				else
					EdgeEnd	= (Mid.x<<13);
				//EdgeEnd		= EdgeEnd;

			} else {
				EndX		= EndX;
				EdgeEnd		= MidPoint.x;
			}
		}

		if (triangletype == 1 || triangletype == 2) {
			for (x= (ITEint)(EdgeStart>>16); x <= (ITEint)(EdgeEnd>>16); x++) {
				if ((x-(int)(Base->x>>3)) >= 0 && (x-(int)(Base->x>>3)) < width) {	
					ITEuint offset = (x-(int)(Base->x>>3)) + (y-(int)(Base->y>>3))*width;
					// 1 valid bit for 4 coverage pixels
					//ITEuint validoffset = ( (x>>5)-((int)(Base->x>>3)>>5) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
					//ITEuint bitoffset = (x>>2) & 0x7;
					ITEuint validoffset = ( (x>>3)-((int)(Base->x>>3)>>3) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
					ITEuint bitoffset = x & 0x7;
					SET2(P,(ITEs12p3)(x<<3), (ITEs12p3)(y<<3));
					CoverageValue = itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
					iteHardwareCache(coverage, offset, valid, validoffset, bitoffset, CoverageCache, ValidCache, CoverageValue);
					if (COVERAGEDRAW) CoverageAA++;
					//*( coverage+offset) += itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
				}
			}
		} 
		
		if (triangletype == 5 || triangletype == 6) {

			x = (ITEint)(EdgeStart>>16);
				
			for ( ; x <= (ITEint)(StartX>>16); x++) {
				if ((x-(int)(Base->x>>3)) >= 0 && (x-(int)(Base->x>>3)) < width) {	
					ITEuint offset = (x-(int)(Base->x>>3)) + (y-(int)(Base->y>>3))*width;
					// 1 valid bit for 4 coverage pixels
					//ITEuint validoffset = ( (x>>5)-((int)(Base->x>>3)>>5) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
					//ITEuint bitoffset = (x>>2) & 0x7;
					ITEuint validoffset = ( (x>>3)-((int)(Base->x>>3)>>3) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
					ITEuint bitoffset = x & 0x7;
					SET2(P,(ITEs12p3)(x<<3), (ITEs12p3)(y<<3));
					CoverageValue = itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
					iteHardwareCache(coverage, offset, valid, validoffset, bitoffset, CoverageCache, ValidCache, CoverageValue);
					if (COVERAGEDRAW) CoverageAA++;
					//*(coverage + offset) += itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
				}
			}

			for ( ; x < (ITEint)(EndX>>16); x++) {
				if ((x-(int)(Base->x>>3)) >= 0 && (x-(int)(Base->x>>3)) < width) {	
					ITEuint offset = (x-(int)(Base->x>>3)) + (y-(int)(Base->y>>3))*width;
					// 1 valid bit for 4 coverage pixels
					//ITEuint validoffset = ( (x>>5)-((int)(Base->x>>3)>>5) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
					//ITEuint bitoffset = (x>>2) & 0x7;
					ITEuint validoffset = ( (x>>3)-((int)(Base->x>>3)>>3) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
					ITEuint bitoffset = x & 0x7;
					CoverageValue = direction<<5;
					iteHardwareCache(coverage, offset, valid, validoffset, bitoffset, CoverageCache, ValidCache, CoverageValue);
					if (COVERAGEDRAW) CoverageNoAA++;
					//*(coverage + offset) += (direction<<5);
				}
			}

			for (; x <= (ITEint)(EdgeEnd>>16); x++) {
				if ((x-(int)(Base->x>>3)) >= 0 && (x-(int)(Base->x>>3)) < width) {	
					ITEuint offset = (x-(int)(Base->x>>3)) + (y-(int)(Base->y>>3))*width;
					// 1 valid bit for 4 coverage pixels
					//ITEuint validoffset = ( (x>>5)-((int)(Base->x>>3)>>5) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
					//ITEuint bitoffset = (x>>2) & 0x7;
					ITEuint validoffset = ( (x>>3)-((int)(Base->x>>3)>>3) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
					ITEuint bitoffset = x & 0x7;
					SET2(P,(ITEs12p3)(x<<3), (ITEs12p3)(y<<3));
					CoverageValue = itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
					iteHardwareCache(coverage, offset, valid, validoffset, bitoffset, CoverageCache, ValidCache, CoverageValue);
					if (COVERAGEDRAW) CoverageAA++;
					//*(coverage + offset) += itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
				}
			}
		}
		y++;
	}

	// To find second parameters
	// INI2ND state
	if (triangletype == 5) {
		SET2(MidPoint, (ITEs15p16)(Mid.x<<13) + (ITEs15p16)(SlopeM2B*MidDistant>>3), (Mid.y>>3)<<3);
		LeftSlope   	= SlopeM2B;

		if (LeftSlope < 0) {
			EdgeStart	= MidPoint.x;
		} else {
			StartX      = MidPoint.x;
		}
		
	} else if (triangletype == 6) {
		SET2(MidPoint, (ITEs15p16)(Mid.x<<13) + (ITEs15p16)(SlopeM2B*MidDistant>>3), (Mid.y>>3)<<3);
		RightSlope 	= SlopeM2B;

		if (RightSlope < 0) {
			EndX 		= MidPoint.x;
		} else {
			EdgeEnd		= MidPoint.x;
		}
	} 

	// Scan line from Mid to Bot
	// M2B state
	for ( ; y<(ITEint)(Bot.y>>3); y++) {

		if (LeftSlope < 0) {
			StartX      = EdgeStart;
			EdgeStart	= EdgeStart + LeftSlope;
		} else {
			EdgeStart	= StartX;
			StartX      = StartX + LeftSlope;
		}

		if (RightSlope < 0) {
			EdgeEnd		= EndX;
			EndX 		= EndX + RightSlope;
		} else {
			EndX 		= EdgeEnd;
			EdgeEnd		= EdgeEnd + RightSlope;
		}

		x = (ITEint)(EdgeStart>>16);
			
		for ( ; x <= (ITEint)(StartX>>16); x++) {
			if ((x-(int)(Base->x>>3)) >= 0 && (x-(int)(Base->x>>3)) < width) {	
				ITEuint offset = (x-(int)(Base->x>>3)) + (y-(int)(Base->y>>3))*width;
				// 1 valid bit for 4 coverage pixels
				//ITEuint validoffset = ( (x>>5)-((int)(Base->x>>3)>>5) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
				//ITEuint bitoffset = (x>>2) & 0x7;
				ITEuint validoffset = ( (x>>3)-((int)(Base->x>>3)>>3) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
				ITEuint bitoffset = x & 0x7;
				SET2(P,(ITEs12p3)(x<<3), (ITEs12p3)(y<<3));
				CoverageValue = itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
				iteHardwareCache(coverage, offset, valid, validoffset, bitoffset, CoverageCache, ValidCache, CoverageValue);
				if (COVERAGEDRAW) CoverageAA++;
				//*(coverage + offset) += itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
			}
		}

		for ( ; x < (ITEint)(EndX>>16); x++) {
			if ((x-(int)(Base->x>>3)) >= 0 && (x-(int)(Base->x>>3)) < width) {	
				ITEuint offset = (x-(int)(Base->x>>3)) + (y-(int)(Base->y>>3))*width;
				// 1 valid bit for 4 coverage pixels
				//ITEuint validoffset = ( (x>>5)-((int)(Base->x>>3)>>5) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
				//ITEuint bitoffset = (x>>2) & 0x7;
				ITEuint validoffset = ( (x>>3)-((int)(Base->x>>3)>>3) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
				ITEuint bitoffset = x & 0x7;
				CoverageValue = direction<<5;
				iteHardwareCache(coverage, offset, valid, validoffset, bitoffset, CoverageCache, ValidCache, CoverageValue);
				if (COVERAGEDRAW) CoverageNoAA++;
				//*(coverage + offset) += (direction<<5);
			}
		}

		for (; x <= (ITEint)(EdgeEnd>>16); x++) {
			if ((x-(int)(Base->x>>3)) >= 0 && (x-(int)(Base->x>>3)) < width) {	
				ITEuint offset = (x-(int)(Base->x>>3)) + (y-(int)(Base->y>>3))*width;
				// 1 valid bit for 4 coverage pixels
				//ITEuint validoffset = ( (x>>5)-((int)(Base->x>>3)>>5) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
				//ITEuint bitoffset = (x>>2) & 0x7;
				ITEuint validoffset = ( (x>>3)-((int)(Base->x>>3)>>3) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
				ITEuint bitoffset = x & 0x7;
				SET2(P,(ITEs12p3)(x<<3), (ITEs12p3)(y<<3));
				CoverageValue = itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
				iteHardwareCache(coverage, offset, valid, validoffset, bitoffset, CoverageCache, ValidCache, CoverageValue);
				if (COVERAGEDRAW) CoverageAA++;
				//*(coverage + offset) += itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
			}
		}
	}


	// Scan Bot line
	// BOT state
	if (y == (ITEint)(Bot.y>>3)) {

		if (LeftSlope < 0) {
			StartX	  = EdgeStart;
			EdgeStart = (ITEs15p16)Bot.x<<13;
		} else {
			EdgeStart = EdgeStart;
			StartX	  = (ITEs15p16)Bot.x<<13;
		}

		if (RightSlope < 0) {
			EdgeEnd = EdgeEnd;
			EndX	= (ITEs15p16)Bot.x<<13;
		} else {
			EndX	= EdgeEnd;
			EdgeEnd = (ITEs15p16)Bot.x<<13;
		}

			for (x= (ITEint)(EdgeStart>>16); x <= (ITEint)(EdgeEnd>>16); x++) {
				if ((x-(int)(Base->x>>3)) >= 0 && (x-(int)(Base->x>>3)) < width) {	
					ITEuint offset = (x-(int)(Base->x>>3)) + (y-(int)(Base->y>>3))*width;
					// 1 valid bit for 4 coverage pixels
					//ITEuint validoffset = ( (x>>5)-((int)(Base->x>>3)>>5) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
					//ITEuint bitoffset = (x>>2) & 0x7;
					ITEuint validoffset = ( (x>>3)-((int)(Base->x>>3)>>3) )  + (y-(int)(Base->y>>3))*validpitch;	// byte aligment
					ITEuint bitoffset = x & 0x7;
					SET2(P,(ITEs12p3)(x<<3), (ITEs12p3)(y<<3));
					CoverageValue = itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
					iteHardwareCache(coverage, offset, valid, validoffset, bitoffset, CoverageCache, ValidCache, CoverageValue);
					if (COVERAGEDRAW) CoverageAA++;
					//*(coverage + offset) += itePatternSample(P, Top, Mid, Bot, triangletype, direction, renderingQuality);
				}
			}
	}
}

void iteCoverageEngine()
{
	HWVector2 a, b, c;
	ITEint i,j, k, init;
	HWVector2 base;
	ITEint count=0;
	ITEHardware *h;
	ITECache16 CoverageCache[CACHESIZE];
	ITECache8  ValidCache[CACHESIZE];
	char filename[] = "dumptri000.list";
	FILE *out_file;
	
	VG_GETCONTEXT(VG_NO_RETVAL);
	h = context->hardware;

	if(h->enCoverage == HW_FALSE)
		return;

	base.x = h->coverageX;
	base.y = h->coverageY;
	
	for (i=0; i<CACHESIZE; i++) 
	{
		CoverageCache[i].replaceMark = 0;
		ValidCache[i].replaceMark = 0;
		for (j=0; j<CACHESET; j++) {	
			CoverageCache[i].valid[j] = 0;
			ValidCache[i].valid[j] = 0;
		}
	}

	if (DUMPDATA) {
		filename[7] = '0' + (context->printnum/100)%10;
		filename[8] = '0' + (context->printnum/10)%10;
		filename[9] = '0' + context->printnum%10;

		out_file = fopen(filename, "wb");
	}

	// for debug
	k=0;

	while( (h->cmdData[count]>>30) != 0x2 )
	{
		if( (h->cmdData[count]>>29) == 0x6 )
		{
			a.x = (*((ITEuint32*)&h->cmdData[count+1]))&0xffff;
			a.y = (*((ITEuint32*)&h->cmdData[count+1]))>>16;
			b = a;
			c = a;
			count+=2;
			init = 0;
		}
		else // if( (h->cmdData[count]>>29) == 0x7 )
		{
			ITEint lines = h->cmdData[count]&0xfff;
			for(i=0;i<lines;i++)
			{
				b = c;
				c.x = (*((ITEuint32*)&h->cmdData[count+i+1]))&0xffff;
				c.y = (*((ITEuint32*)&h->cmdData[count+i+1]))>>16;
				init++;
				if(init>1) {
					iteFillTriangles(&a, &b, &c, &base, h->coverageData, h->coverageWidth, h->coverageValid, 
									 h->coveragevalidpitch, h->renderingQuality, CoverageCache, ValidCache);
					k++;
					if (DUMPDATA)
						fprintf(out_file,"%04x %04x \t%04x %04x \t%04x %04x \n", a.x, a.y, b.x, b.y, c.x, c.y);
				}
			}
			count += (lines + 1);
		}
	}

	if (DUMPDATA) {
		fclose(out_file);
		printf("\nDump Triangle list No. %d file ok!\n", context->printnum);
	}

	iteCacheWriteBack(CoverageCache,ValidCache);

	if (COVERAGEDRAW) 
	{
		CoverageClear += (h->coverageWidth*h->coverageHeight);
		printf("CoverageDraw = %d\n",CoverageDraw);
		printf("CoverageNoAA = %d\n",CoverageNoAA);
		printf("CoverageAA = %d\n",CoverageAA);
		printf("CoverageClear = %d\n",CoverageClear);	
	}
		
	if (DUMPDATA)
	{
		ITEint16 *MemAddr;
		ITEuint8 *VldAddr;
		char filename[] = "dumpcov000.hex";
		int	upbound, downbound, leftbound, rightbound;
		int startx, endx;

		filename[7] = '0' + (context->printnum/100)%10;
		filename[8] = '0' + (context->printnum/10)%10;
		filename[9] = '0' + context->printnum%10;

		// scan the boundary
		for (i=0; i<h->coverageHeight; i++)
		{
			int k = 0;
			for (j=0; j<h->coverageWidth; j++)
			{
				VldAddr = h->coverageValid + h->coveragevalidpitch*i + ((j+((base.x>>3)&0x7)) >>3);
				if ( (*VldAddr) & (1<<(j+((base.x>>3)&0x7) & 0x7)) )  k=1; 
			}
			upbound = i;
			if (k==1) break;
		}

		for (i=h->coverageHeight-1; i>=0; i--)
		{
			int k = 0;
			for (j=0; j<h->coverageWidth; j++)
			{
				VldAddr = h->coverageValid + h->coveragevalidpitch*i + ((j+((base.x>>3)&0x7)) >>3);
				if ( (*VldAddr) & (1<<(j+((base.x>>3)&0x7) & 0x7)) )  k=1; 
			}
			downbound = i;
			if (k==1) break;
		}

		for (j=0; j<h->coverageWidth; j++)
		{
			int k = 0;
			for (i=0; i<h->coverageHeight; i++)
			{
				VldAddr = h->coverageValid + h->coveragevalidpitch*i + ((j+((base.x>>3)&0x7)) >>3);
				if ( (*VldAddr) & (1<<(j+((base.x>>3)&0x7) & 0x7)) )  k=1;
			}
			leftbound = j;
			if (k==1) break;
		}

		for (j=h->coverageWidth-1; j>=0; j--)
		{
			int k = 0;
			for (i=0; i<h->coverageHeight; i++)
			{
				VldAddr = h->coverageValid + h->coveragevalidpitch*i + ((j+((base.x>>3)&0x7)) >>3);
				if ( (*VldAddr) & (1<<(j+((base.x>>3)&0x7) & 0x7)) )  k=1; 
			}
			rightbound = j;
			if (k==1) break;
		}

		startx = (base.x>>3) + leftbound;
		endx   = (base.x>>3) + rightbound;

		// write file
		out_file = fopen(filename, "wb");
		for (i=upbound; i<=downbound; i++)
		{
			if (DUMPFORMAT || h->renderingQuality==HW_RENDERING_QUALITY_BETTER)			// s11.4
			{
				for (j=0; j<(startx & 0x3); j++)
					fprintf(out_file,"xx xx ");
			} else {																		// s5.2, s7
				for (j=0; j<(startx & 0x7); j++)
					fprintf(out_file,"xx ");
			}
			
			for (j=leftbound; j<=rightbound; j++)
			{
				MemAddr = h->coverageData + h->coverageWidth*i + j;
				VldAddr = h->coverageValid + h->coveragevalidpitch*i + ((j+((base.x>>3)&0x7)) >>3);
			
				if ( (*VldAddr) & (1<<(j+((base.x>>3)&0x7) & 0x7)) ) 
				{
					if (DUMPFORMAT || h->renderingQuality==HW_RENDERING_QUALITY_BETTER)	// s11.4
						fprintf(out_file,"%02x %02x ", (((*MemAddr)>>1) & 0xff), (((*MemAddr)>>(8+1)) & 0xff) );
					else {				// s5.2
						if (h->renderingQuality == HW_RENDERING_QUALITY_NONANTIALIASED)		// s7
							fprintf(out_file,"%02x ", (((*MemAddr)>>5) & 0xff) );
						else																// s5.2
							fprintf(out_file,"%02x ", (((*MemAddr)>>3) & 0xff) );
					}	
				} 
				else
				{
					if (DUMPFORMAT || h->renderingQuality==HW_RENDERING_QUALITY_BETTER)	// s11.4
						fprintf(out_file,"xx xx ");
					else																	// s5.2, s7
						fprintf(out_file,"xx ");
				}
			}
	
			if (DUMPFORMAT || h->renderingQuality==HW_RENDERING_QUALITY_BETTER)			// s11.4
			{
				for (j=endx+1; j<=((endx>>2)<<2)+3; j++)
					fprintf(out_file,"xx xx ");
			} else {																		// s5.2, s7
				for (j=endx+1; j<=((endx>>3)<<3)+7; j++)
					fprintf(out_file,"xx ");
			}

			fprintf(out_file,"\n");
		}

		fclose(out_file);
		printf("\nDump Coverage No. %d file ok!\n", context->printnum);
		context->printnum++;
	}

/*	if (h->cmdData[0] == HW_FILL_PATH) // triangle fan list
	{
		numFan = h->cmdData[1];
		count = 2;
		for(j=0;j<numFan;j++)
		{
			numVex = h->cmdData[count++];
			a.x = (*((ITEuint32*)&h->cmdData[count]))&0xffff;
			a.y = (*((ITEuint32*)&h->cmdData[count++]))>>16;
			for(i=0;i<numVex-2;i++)
			{
				b.x = (*((ITEuint32*)&h->cmdData[count]))&0xffff;
				b.y = (*((ITEuint32*)&h->cmdData[count]))>>16;
				c.x = (*((ITEuint32*)&h->cmdData[count+1]))&0xffff;
				c.y = (*((ITEuint32*)&h->cmdData[count+1]))>>16;
				count ++;
				iteFillTriangles(&a, &b, &c, &base, h->coverageData, h->coverageWidth, 
								 h->renderingQuality, CoverageCache, ValidCache);
			}
			count++;
		}
		iteCacheWriteBack(CoverageCache, ValidCache);
	}
	else if (h->cmdData[0] == HW_STROKE_PATH) // triangle list
	{ 
		count = 2;
		numVex = h->cmdData[count++];
		for(i=0;i<numVex;i+=3)
		{
			a.x = (*((ITEuint32*)&h->cmdData[count]))&0xffff;
			a.y = (*((ITEuint32*)&h->cmdData[count]))>>16;
			b.x = (*((ITEuint32*)&h->cmdData[count+1]))&0xffff;
			b.y = (*((ITEuint32*)&h->cmdData[count+1]))>>16;
			c.x = (*((ITEuint32*)&h->cmdData[count+2]))&0xffff;
			c.y = (*((ITEuint32*)&h->cmdData[count+2]))>>16;
			count += 3;
			iteFillTriangles(&a, &b, &c, &base, h->coverageData, h->coverageWidth, 
							 h->renderingQuality, CoverageCache,ValidCache);
		}
		iteCacheWriteBack(CoverageCache,ValidCache);
	}
	if (COVERAGEDRAW == 1) printf("CoverageDraw = %d\n",CoverageDraw);
*/
}
