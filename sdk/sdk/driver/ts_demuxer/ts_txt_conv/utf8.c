/*
 * Copyright (c) 2011 ITE Technology Corp. All Rights Reserved.
 */
/** @file utf8.c
 * Ultility functions used to convert UTF-8 character sets to Unicode.
 *
 * @author I-Chun Lai
 * @version 0.1
 */

#include "utf8.h"

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

// Reference: http://en.wikipedia.org/wiki/Utf8
static const MMP_UINT8 cUtf8Limits[] = {
    0xC0,   // Start of a 2-byte sequence
    0xE0,   // Start of a 3-byte sequence
    0xF0,   // Start of a 4-byte sequence
    0xF8,   // Start of a 5-byte sequence
    0xFC,   // Start of a 6-byte sequence
    0xFE    // Invalid: not defined by original UTF-8 specification
};

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
* Convert the UTF-8 to UTF-16
* @param pOut      The output UTF-16 string.
* @param pIn       The input UTF-8 string.
* @param length    The length of the input UTF-8 string.
* @return The number of UTF-16 characters in a UTF-16 character string
*/
//=============================================================================
MMP_UINT32
UTF8ToUTF16(
    uint16_t* pOut,
    MMP_UINT8* pIn,
    MMP_UINT32 length)
{
    if (pIn && length > 0)
    {
        MMP_UINT32 inIdx  = 0;
        MMP_UINT32 outIdx = 0;
        MMP_UINT32 outLength = 0;

        if (pOut)
        {
            do
            {
                MMP_UINT8   c;
                MMP_UINT32  numAdds;
                MMP_UINT32  value;

                if (inIdx >= length || pIn[inIdx] == '\0')
                {
                    outLength = outIdx;
                    pOut[outIdx] = 0;
                    return outLength;
                }

                c = pIn[inIdx++];

                if  (c < 0x80)
                {   // 0-127, US-ASCII (single byte)
                    pOut[outIdx++] = (uint16_t)c;
                    continue;
                }

                if (c < 0xC0)   // The first octet for each code point should within 0x00-0xBF
                    break;

                for (numAdds = 1; numAdds < 5; ++numAdds)
                    if (c < cUtf8Limits[numAdds])
                        break;
                value = c - cUtf8Limits[numAdds - 1];

                do {
                    MMP_UINT8 c2;
                    if (inIdx >= length || pIn[inIdx] == '\0')
                        break;
                    c2 = pIn[inIdx++];
                    if (c2 < 0x80 || c2 >= 0xC0)
                        break;
                    value <<= 6;
                    value |= (c2 - 0x80);
                } while(--numAdds != 0);

                if (value < 0x10000)
                {
                    pOut[outIdx] = (uint16_t)value;
                    ++outIdx;
                }
                else
                {
                    value -= 0x10000;
                    if (value >= 0x100000)
                        break;
                    pOut[outIdx + 0] = (uint16_t)(0xD800 + (value >> 10));
                    pOut[outIdx + 1] = (uint16_t)(0xDC00 + (value & 0x3FF));
                    outIdx += 2;
                }
            }
            while(1);

            outLength = outIdx;
            pOut[outIdx] = 0;
        }
        else
        {
            do
            {
                MMP_UINT8   c;
                MMP_UINT32  numAdds;
                MMP_UINT32  value;

                if (inIdx >= length || pIn[inIdx] == '\0')
                {
                    outLength = outIdx;
                    return outLength;
                }

                c = pIn[inIdx++];

                if  (c < 0x80)
                {   // 0-127, US-ASCII (single byte)
                    ++outIdx;
                    continue;
                }

                if (c < 0xC0)   // The first octet for each code point should within 0-191
                    break;

                for (numAdds = 1; numAdds < 5; ++numAdds)
                    if (c < cUtf8Limits[numAdds])
                        break;
                value = c - cUtf8Limits[numAdds - 1];

                do
                {
                    MMP_UINT8 c2;
                    if (inIdx >= length || pIn[inIdx] == '\0')
                        break;
                    c2 = pIn[inIdx++];
                    if (c2 < 0x80 || c2 >= 0xC0)
                        break;
                    value <<= 6;
                    value |= (c2 - 0x80);
                } while(--numAdds != 0);

                if (value < 0x10000)
                {
                    ++outIdx;
                }
                else
                {
                    value -= 0x10000;
                    if (value >= 0x100000)
                        break;
                    outIdx += 2;
                }
            }
            while(1);

            outLength = outIdx;
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

