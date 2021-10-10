/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file iso6937.h
 * Ultility functions used to convert ISO/IEC 6937 series of character sets
 * to Unicode. The mapping table from ISO 6937 character sets to Unicode can
 * be found in http://en.wikipedia.org/wiki/ISO/IEC_6937.
 *
 * @author I-Chun Lai
 * @version 0.1
 */


#include "iso6937.h"

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct ISO6937_MAP_TAG
{
    const uint16_t in;
    const uint16_t out;
} ISO6937_MAP;

typedef struct ISO6937_MAP_LIST_TAG
{
    const ISO6937_MAP * const   ptMap;
    const MMP_UINT              mapSize;
} ISO6937_MAP_LIST;

//=============================================================================
//                              Global Data Definition
//=============================================================================

static const uint16_t _ISO6937[96] =
{
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x20AC, 0x00A5, 0x00A6, 0x00A7,
    0x00A4, 0x2018, 0x201C, 0x00AB, 0x2190, 0x2191, 0x2192, 0x2193,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00D7, 0x00B5, 0x00B6, 0x00B7,
    0x00F7, 0x2019, 0x201D, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x2015, 0x00B9, 0x00AE, 0x00A9, 0x2122, 0x266A, 0x00AC, 0x00A6,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x215B, 0x215C, 0x215D, 0x215E,
    0x2126, 0x00C6, 0x0110, 0x00AA, 0x0126, 0x00E5, 0x0132, 0x013F,
    0x0141, 0x00D8, 0x0152, 0x00BA, 0x00DE, 0x0166, 0x014A, 0x0149,
    0x0138, 0x00E6, 0x0111, 0x00F0, 0x0127, 0x0131, 0x0133, 0x0140,
    0x0142, 0x00F8, 0x0153, 0x00DF, 0x00FE, 0x0167, 0x014B, 0x00AD
};

static const ISO6937_MAP _ISO6937_0xC1[10] =
{
    {0x0041, 0x00C0},
    {0x0045, 0x00C8},
    {0x0049, 0x00CC},
    {0x004F, 0x00D2},
    {0x0055, 0x00D9},
    {0x0061, 0x00E0},
    {0x0065, 0x00E8},
    {0x0069, 0x00EC},
    {0x006F, 0x00F2},
    {0x0075, 0x00F9},
};

static const ISO6937_MAP _ISO6937_0xC2[24] =
{
    {0x0041, 0x00C1},
    {0x0043, 0x0106},
    {0x0045, 0x00C9},
    {0x0049, 0x00CD},
    {0x004C, 0x0139},
    {0x004E, 0x0143},
    {0x004F, 0x00D3},
    {0x0052, 0x0154},
    {0x0053, 0x015A},
    {0x0055, 0x00DA},
    {0x0059, 0x00DD},
    {0x005A, 0x0179},
    {0x0061, 0x00E1},
    {0x0063, 0x0107},
    {0x0065, 0x00E9},
    {0x0069, 0x00ED},
    {0x006C, 0x013A},
    {0x006E, 0x0144},
    {0x006F, 0x00F3},
    {0x0072, 0x0155},
    {0x0073, 0x015B},
    {0x0075, 0x00FA},
    {0x0079, 0x00FD},
    {0x007A, 0x017A},
};

static const ISO6937_MAP _ISO6937_0xC3[24] =
{
    {0x0041, 0x00C2},
    {0x0043, 0x0108},
    {0x0045, 0x00CA},
    {0x0047, 0x011C},
    {0x0048, 0x0124},
    {0x0049, 0x00CE},
    {0x004A, 0x0134},
    {0x004F, 0x00D4},
    {0x0053, 0x015C},
    {0x0055, 0x00DB},
    {0x0057, 0x0174},
    {0x0059, 0x0176},
    {0x0061, 0x00E2},
    {0x0063, 0x0109},
    {0x0065, 0x00EA},
    {0x0067, 0x011D},
    {0x0068, 0x0125},
    {0x0069, 0x00EE},
    {0x006A, 0x0135},
    {0x006F, 0x00F4},
    {0x0073, 0x015D},
    {0x0075, 0x00FB},
    {0x0077, 0x0175},
    {0x0079, 0x0177},
};

static const ISO6937_MAP _ISO6937_0xC4[10] =
{
    {0x0041, 0x00C3},
    {0x0049, 0x0128},
    {0x004E, 0x00D1},
    {0x004F, 0x00D5},
    {0x0055, 0x0168},
    {0x0061, 0x00E3},
    {0x0069, 0x0129},
    {0x006E, 0x00F1},
    {0x006F, 0x00F5},
    {0x0075, 0x0169},
};

static const ISO6937_MAP _ISO6937_0xC5[10] =
{
    {0x0041, 0x0100},
    {0x0045, 0x0112},
    {0x0049, 0x012A},
    {0x004F, 0x014C},
    {0x0055, 0x016A},
    {0x0061, 0x0101},
    {0x0065, 0x0113},
    {0x0069, 0x012B},
    {0x006F, 0x014D},
    {0x0075, 0x016B},
};

static const ISO6937_MAP _ISO6937_0xC6[6] =
{
    {0x0041, 0x0102},
    {0x0047, 0x011E},
    {0x0055, 0x016C},
    {0x0061, 0x0103},
    {0x0067, 0x011F},
    {0x0075, 0x016D},
};

static const ISO6937_MAP _ISO6937_0xC7[10] =
{
    {0x0043, 0x010A},
    {0x0045, 0x0116},
    {0x0047, 0x0120},
    {0x0049, 0x0130},
    {0x005A, 0x017B},
    {0x0063, 0x010B},
    {0x0065, 0x0117},
    {0x0067, 0x0121},
    {0x0069, 0x0131},
    {0x007A, 0x017C},
};

static const ISO6937_MAP _ISO6937_0xC8[12] =
{
    {0x0041, 0x00C4},
    {0x0045, 0x00CB},
    {0x0049, 0x00CF},
    {0x004F, 0x00D6},
    {0x0055, 0x00DC},
    {0x0059, 0x0178},
    {0x0061, 0x00E4},
    {0x0065, 0x00EB},
    {0x0069, 0x00EF},
    {0x006F, 0x00F6},
    {0x0075, 0x00FC},
    {0x0079, 0x00FF},
};

static const ISO6937_MAP _ISO6937_0xCA[4] =
{
    {0x0041, 0x00C5},
    {0x0055, 0x016E},
    {0x0061, 0x00E5},
    {0x0075, 0x016F},
};

static const ISO6937_MAP _ISO6937_0xCB[16] =
{
    {0x0043, 0x00C7},
    {0x0047, 0x0122},
    {0x004B, 0x0136},
    {0x004C, 0x013B},
    {0x004E, 0x0145},
    {0x0052, 0x0156},
    {0x0053, 0x015E},
    {0x0054, 0x0162},
    {0x0063, 0x00E7},
    {0x0067, 0x0123},
    {0x006B, 0x0137},
    {0x006C, 0x013C},
    {0x006E, 0x0146},
    {0x0072, 0x0157},
    {0x0073, 0x015F},
    {0x0074, 0x0163},
};

static const ISO6937_MAP _ISO6937_0xCD[4] =
{
    {0x004F, 0x0150},
    {0x0055, 0x0170},
    {0x006F, 0x0151},
    {0x0075, 0x0171},
};

static const ISO6937_MAP _ISO6937_0xCE[8] =
{
    {0x0041, 0x0104},
    {0x0045, 0x0118},
    {0x0049, 0x012E},
    {0x0055, 0x0172},
    {0x0061, 0x0105},
    {0x0065, 0x0119},
    {0x0069, 0x012F},
    {0x0075, 0x0173},
};

static const ISO6937_MAP _ISO6937_0xCF[18] =
{
    {0x0043, 0x010C},
    {0x0044, 0x010E},
    {0x0045, 0x011A},
    {0x004C, 0x013D},
    {0x004E, 0x0147},
    {0x0052, 0x0158},
    {0x0053, 0x0160},
    {0x0054, 0x0164},
    {0x005A, 0x017D},
    {0x0063, 0x010D},
    {0x0064, 0x010F},
    {0x0065, 0x011B},
    {0x006C, 0x013E},
    {0x006E, 0x0148},
    {0x0072, 0x0159},
    {0x0073, 0x0161},
    {0x0074, 0x0165},
    {0x007A, 0x017E},
};

static const ISO6937_MAP_LIST _ISO6937_0xCX[16] =
{
    {MMP_NULL,       0},
    {_ISO6937_0xC1, 10},
    {_ISO6937_0xC2, 24},
    {_ISO6937_0xC3, 24},
    {_ISO6937_0xC4, 10},
    {_ISO6937_0xC5, 10},
    {_ISO6937_0xC6,  6},
    {_ISO6937_0xC7, 10},
    {_ISO6937_0xC8, 12},
    {MMP_NULL,       0},
    {_ISO6937_0xCA,  4},
    {_ISO6937_0xCB, 16},
    {MMP_NULL,       0},
    {_ISO6937_0xCD,  4},
    {_ISO6937_0xCE,  8},
    {_ISO6937_0xCF, 18},
};

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static MMP_INLINE uint16_t
_ISO6937_0xCXToUTF16(
    MMP_UINT8   *pIn);

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Convert the DVB text to UTF-16
 * @param pOut      The output UTF-16 string.
 * @param pIn       The input ISO/IEC string.
 * @param inLength  The length of the input ISO/IEC string.
 * @return The number of UTF-16 characters in a UTF-16 character string
 */
//=============================================================================
MMP_UINT32
ISO6937ToUTF16(
    uint16_t    *pOut,
    MMP_UINT8   *pIn,
    MMP_UINT32  length)
{
    if (pIn && length > 0)
    {
        MMP_UINT32 strIndex  = 0;
        MMP_UINT32 outLength = length;

        if (pOut)
        {
            for (strIndex = 0; strIndex < length; ++strIndex, ++pOut, ++pIn)
            {
                if ((*pIn) < 0xA0)
                {
                    if ((*pIn) < 0x20)
                        (*pOut) = 0x0;
                    else
                        (*pOut) = (*pIn);
                }
                else
                {
                    if (0xC0 == ((*pIn) & 0xF0))
                    {
                        (*pOut) = _ISO6937_0xCXToUTF16(pIn);
                        ++strIndex;
                        ++pIn;
                        --outLength;
                    }
                    else
                    {
                        (*pOut) = _ISO6937[(*pIn) - 0xA0];
                    }
                }
            }
            (*pOut) = 0;
        }
        else
        {
            for (strIndex = 0; strIndex < length; ++strIndex, ++pIn)
            {
                if (0xC0 == ((*pIn) & 0xF0))
                {
                    ++strIndex;
                    ++pIn;
                    --outLength;
                }
            }
        }
        return outLength;
    }
    else if (pOut)
    {
        (*pOut) = 0;
    }
    return 0;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

static MMP_INLINE uint16_t
_ISO6937_0xCXToUTF16(
    MMP_UINT8* pIn)
{
    const ISO6937_MAP*  ptMap;
    MMP_UINT            size;

    PalAssert(pIn);
    PalAssert(0xC0 <= (*pIn) && (*pIn) <= 0xCF);

    ptMap = _ISO6937_0xCX[(*pIn) & 0xF].ptMap;
    size  = _ISO6937_0xCX[(*pIn) & 0xF].mapSize;

    if (ptMap)
    {
        MMP_UINT  i;
        uint16_t   ch;

        ch = (*(pIn + 1));
        for (i = 0; i < size; ++i)
        {
            if (ch == ptMap[i].in)
                return ptMap[i].out;
        }
    }

    return 0x20;
}
