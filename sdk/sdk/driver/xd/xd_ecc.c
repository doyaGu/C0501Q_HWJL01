#include "xd/xd_ecc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

////////////////////////////////////////////////////////////
//
//	Constant variable or Marco
//
////////////////////////////////////////////////////////////
#define ECC_TABLESIZE   256
#define ECC_MASK_CPS    0x3F
#define ECC_CORRECTABLE 0x00555554L
#define ECC_BIT23       0x00800000L
#define ECC_BIT7        0x80 
#define ECC_BIT6        0x40
#define ECC_BIT5        0x20
#define ECC_BIT4        0x10
#define ECC_BIT3        0x08
#define ECC_BIT2        0x04
#define ECC_BIT1        0x02
#define ECC_BIT0        0x01
#define ECC_BIT1BIT0    0x03

////////////////////////////////////////////////////////////
//
//	Variable Declaration
//
////////////////////////////////////////////////////////////
static MMP_UINT8* g_pEccTable = MMP_NULL;

////////////////////////////////////////////////////////////
//
// Function Prototype
//
////////////////////////////////////////////////////////////
static MMP_BOOL InitialEccTable();
static void TransResult(MMP_UINT8 reg2, MMP_UINT8 reg3, MMP_UINT8* pEcc1, MMP_UINT8* pEcc2);

////////////////////////////////////////////////////////////
//
// Function Implement
//
////////////////////////////////////////////////////////////
MMP_BOOL XD_EccInitial()
{
	//////////////////////////////////////////////////////////////////////////
	// Step 1, Create ECC table
	return InitialEccTable();
}

MMP_BOOL XD_EccCalculate(MMP_UINT8* pBuffer, MMP_UINT8* pEcc1, MMP_UINT8* pEcc2, MMP_UINT8* pEcc3)
{
	MMP_UINT32 i;
	MMP_UINT8  a, reg1, reg2, reg3;

	if ( pBuffer == MMP_NULL )
	{
		printf("XD_EccCalculate: Input Null Buffer!\n");
		return MMP_FALSE;
	}

	reg1 = reg2 = reg3 = 0;

#if 0
	for ( i=0; i<256; i++ )
	{
		a = g_pEccTable[pBuffer[i]];
		reg1 ^= (a & ECC_MASK_CPS);
		if ( (a & ECC_BIT6) != 0 )
		{
			reg3 ^= (MMP_UINT8)i;
			reg2 ^= ~((MMP_UINT8)i);
		}
	}
#else
    for ( i=0; i<256; i+=8 )
	{
		a = g_pEccTable[pBuffer[i]];
		reg1 ^= (a & ECC_MASK_CPS);
		if ( (a & ECC_BIT6) != 0 )
		{
			reg3 ^= (MMP_UINT8)i;
			reg2 ^= ~((MMP_UINT8)i);
		}

		a = g_pEccTable[pBuffer[i+1]];
		reg1 ^= (a & ECC_MASK_CPS);
		if ( (a & ECC_BIT6) != 0 )
		{
			reg3 ^= (MMP_UINT8)(i+1);
			reg2 ^= ~((MMP_UINT8)(i+1));
		}

		a = g_pEccTable[pBuffer[i+2]];
		reg1 ^= (a & ECC_MASK_CPS);
		if ( (a & ECC_BIT6) != 0 )
		{
			reg3 ^= (MMP_UINT8)(i+2);
			reg2 ^= ~((MMP_UINT8)(i+2));
		}

		a = g_pEccTable[pBuffer[i+3]];
		reg1 ^= (a & ECC_MASK_CPS);
		if ( (a & ECC_BIT6) != 0 )
		{
			reg3 ^= (MMP_UINT8)(i+3);
			reg2 ^= ~((MMP_UINT8)(i+3));
		}

		a = g_pEccTable[pBuffer[i+4]];
		reg1 ^= (a & ECC_MASK_CPS);
		if ( (a & ECC_BIT6) != 0 )
		{
			reg3 ^= (MMP_UINT8)(i+4);
			reg2 ^= ~((MMP_UINT8)(i+4));
		}

		a = g_pEccTable[pBuffer[i+5]];
		reg1 ^= (a & ECC_MASK_CPS);
		if ( (a & ECC_BIT6) != 0 )
		{
			reg3 ^= (MMP_UINT8)(i+5);
			reg2 ^= ~((MMP_UINT8)(i+5));
		}

		a = g_pEccTable[pBuffer[i+6]];
		reg1 ^= (a & ECC_MASK_CPS);
		if ( (a & ECC_BIT6) != 0 )
		{
			reg3 ^= (MMP_UINT8)(i+6);
			reg2 ^= ~((MMP_UINT8)(i+6));
		}

		a = g_pEccTable[pBuffer[i+7]];
		reg1 ^= (a & ECC_MASK_CPS);
		if ( (a & ECC_BIT6) != 0 )
		{
			reg3 ^= (MMP_UINT8)(i+7);
			reg2 ^= ~((MMP_UINT8)(i+7));
		}
	}
#endif

	TransResult(reg2, reg3, pEcc1, pEcc2);

	*pEcc1 = ~(*pEcc1);
	*pEcc2 = ~(*pEcc2);
	*pEcc3 = ((~reg1) << 2) | ECC_BIT1BIT0;

	return MMP_TRUE;
}

MMP_UINT8 XD_EccCorrectData(MMP_UINT8* pBuffer, MMP_UINT8 Ecc1, MMP_UINT8 Ecc2, MMP_UINT8 Ecc3)
{
	MMP_UINT32 l, d, i;
	MMP_UINT8  d1, d2, d3;
	MMP_UINT8  a, b, add, bit;
	MMP_UINT8  bufEcc1, bufEcc2, bufEcc3;

	if ( pBuffer == MMP_NULL )
	{
		printf("XD_EccCorrectData: Input Null Buffer!\n");
		return XD_ECC_UNKNOWN_ERROR;
	}

	// Calculate ECC of buffer
	if ( XD_EccCalculate(pBuffer, &bufEcc1, &bufEcc2, &bufEcc3) )
	{
		d1 = Ecc1 ^ bufEcc1;
		d2 = Ecc2 ^ bufEcc2;
		d3 = Ecc3 ^ bufEcc3;
		d =   ((MMP_UINT32)d1 << 16) 
			+ ((MMP_UINT32)d2 << 8)
			+  (MMP_UINT32)d3;

		if ( d == 0 )
			return XD_ECC_NO_ERROR;
			
		printf("Input Ecc1 = 0x%02X,\tEcc2 = 0x%02X,\tEcc3 = 0x%02X\n", Ecc1, Ecc2, Ecc3);
		printf("Calc  Ecc1 = 0x%02X,\tEcc2 = 0x%02X,\tEcc3 = 0x%02X\n", bufEcc1, bufEcc2, bufEcc3);

		if ( ((d ^ (d >> 1)) & ECC_CORRECTABLE) == ECC_CORRECTABLE )
		{
			l = ECC_BIT23;
			add = 0;
			a = ECC_BIT7;
			for ( i=0; i<8; i++ )
			{
				if ( (d & l) != 0 )
				{
					add |= a;
				}
				l >>= 2;
				a >>= 1;
			}

			bit = 0;
			b = ECC_BIT2;
			for ( i=0; i<3; i++ )
			{
				if ( (d & l) != 0 )
				{
					bit |= b;
				}
				l >>= 2;
				b >>= 1;
			}
			b = ECC_BIT0;
			pBuffer[add] ^= (b << bit);
			return XD_ECC_CORRECTED;
		}

		i = 0;
		d &= 0x00FFFFFFL;
		while( d )
		{
			if ( d & ECC_BIT0 )
				i++;

			d >>= 1;
		}

		if ( i == 1 )
		{
			return XD_ECC_ERROR_ECC;
		}

		return XD_ECC_UNCORRECTABLE;
	}

	return XD_ECC_UNKNOWN_ERROR;
}

void XD_EccTerminate()
{
	if ( g_pEccTable != MMP_NULL )
	{
		free(g_pEccTable);
		g_pEccTable = MMP_NULL;
	}
}


static MMP_BOOL InitialEccTable()
{
	MMP_UINT32 i, j;
	MMP_UINT32 mask;
	MMP_UINT8  dall;
	MMP_UINT8  cp0, cp1, cp2, cp3, cp4, cp5;

	if ( g_pEccTable != MMP_NULL )
	{
		free(g_pEccTable);
		g_pEccTable = MMP_NULL;
	}
	g_pEccTable = (MMP_UINT8*)malloc(ECC_TABLESIZE);
	if ( g_pEccTable == MMP_NULL )
	{
		printf("InitialEccTable: Not Enough Memory, Allocate g_pEccTable Fail\n");
		return MMP_FALSE;
	}
	memset(g_pEccTable, 0x00, ECC_TABLESIZE);

	for ( i=0; i<ECC_TABLESIZE; i++ )
	{
		mask = ECC_BIT0;
		dall = cp0 = cp1 = cp2 = cp3 = cp4 = cp5 = 0;

		for ( j=0; j<7; j++ )
		{
			if ( ((MMP_UINT8)i) & ECC_BIT0 ) { ++dall; ++cp0; ++cp2; ++cp4; }
			if ( ((MMP_UINT8)i) & ECC_BIT1 ) { ++dall; ++cp1; ++cp2; ++cp4; }
			if ( ((MMP_UINT8)i) & ECC_BIT2 ) { ++dall; ++cp0; ++cp3; ++cp4; }
			if ( ((MMP_UINT8)i) & ECC_BIT3 ) { ++dall; ++cp1; ++cp3; ++cp4; }
			if ( ((MMP_UINT8)i) & ECC_BIT4 ) { ++dall; ++cp0; ++cp2; ++cp5; }
			if ( ((MMP_UINT8)i) & ECC_BIT5 ) { ++dall; ++cp1; ++cp2; ++cp5; }
			if ( ((MMP_UINT8)i) & ECC_BIT6 ) { ++dall; ++cp0; ++cp3; ++cp5; }
			if ( ((MMP_UINT8)i) & ECC_BIT7 ) { ++dall; ++cp1; ++cp3; ++cp5; }
		}

		if ( dall & ECC_BIT0 ) g_pEccTable[i] |= ECC_BIT6;
		if ( cp5 & ECC_BIT0 ) g_pEccTable[i] |= ECC_BIT5;
		if ( cp4 & ECC_BIT0 ) g_pEccTable[i] |= ECC_BIT4;
		if ( cp3 & ECC_BIT0 ) g_pEccTable[i] |= ECC_BIT3;
		if ( cp2 & ECC_BIT0 ) g_pEccTable[i] |= ECC_BIT2;
		if ( cp1 & ECC_BIT0 ) g_pEccTable[i] |= ECC_BIT1;
		if ( cp0 & ECC_BIT0 ) g_pEccTable[i] |= ECC_BIT0;
	}

	return MMP_TRUE;
}

static void TransResult(MMP_UINT8 reg2, MMP_UINT8 reg3, MMP_UINT8* pEcc1, MMP_UINT8* pEcc2)
{
	MMP_UINT8 a, b, i;

	a = b = ECC_BIT7;
	*pEcc1 = *pEcc2 = 0;

	for ( i=0; i<4; i++ )
	{
		if ( (reg3 & a) != 0 )
		{
			*pEcc1 |= b;
		}

		b >>= 1;

		if ( (reg2 & a) != 0 )
		{
			*pEcc1 |= b;
		}

		b >>= 1;
		a >>= 1;
	}

	b = ECC_BIT7;
	for ( i=0; i<4; i++ )
	{
		if ( (reg3 & a) != 0 )
		{
			*pEcc2 |= b;
		}

		b >>= 1;

		if ( (reg2 & a) != 0 )
		{
			*pEcc2 |= b;
		}

		b >>= 1;
		a >>= 1;
	}

}
