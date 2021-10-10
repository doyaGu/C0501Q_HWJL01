/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file iso8859.c
 * Ultility functions used to convert ISO/IEC 8859 series of character sets
 * to Unicode. The mapping table from ISO 8859 character sets to Unicode can
 * be found in http://unicode.org/Public/MAPPINGS/ISO8859/.
 *
 * @author I-Chun Lai
 * @version 0.1
 */


#include "iso8859.h"

//=============================================================================
//                              Private Function Declaration
//=============================================================================
MMP_INLINE uint16_t
_ISO8859_1ToUTF16(
    MMP_UINT8 ch);

MMP_INLINE uint16_t
_ISO8859_2ToUTF16(
    MMP_UINT8 ch);

MMP_INLINE uint16_t
_ISO8859_3ToUTF16(
    MMP_UINT8 ch);

MMP_INLINE uint16_t
_ISO8859_4ToUTF16(
    MMP_UINT8 ch);

MMP_INLINE uint16_t
_ISO8859_5ToUTF16(
    MMP_UINT8 ch);

MMP_INLINE uint16_t
_ISO8859_6ToUTF16(
    MMP_UINT8 ch);

MMP_INLINE uint16_t
_ISO8859_7ToUTF16(
    MMP_UINT8 ch);

MMP_INLINE uint16_t
_ISO8859_8ToUTF16(
    MMP_UINT8 ch);

MMP_INLINE uint16_t
_ISO8859_9ToUTF16(
    MMP_UINT8 ch);

MMP_INLINE uint16_t
_ISO8859_10ToUTF16(
    MMP_UINT8 ch);

MMP_INLINE uint16_t
_ISO8859_11ToUTF16(
    MMP_UINT8 ch);

MMP_INLINE uint16_t
_ISO8859_13ToUTF16(
    MMP_UINT8 ch);

MMP_INLINE uint16_t
_ISO8859_14ToUTF16(
    MMP_UINT8 ch);

MMP_INLINE uint16_t
_ISO8859_15ToUTF16(
    MMP_UINT8 ch);

//=============================================================================
//                              Global Data Definition
//=============================================================================

static uint16_t _ISO8859_2[96] =
{
    0x00A0, 0x0104, 0x02D8, 0x0141, 0x00A4, 0x013D, 0x015A, 0x00A7,
    0x00A8, 0x0160, 0x015E, 0x0164, 0x0179, 0x00AD, 0x017D, 0x017B,
    0x00B0, 0x0105, 0x02DB, 0x0142, 0x00B4, 0x013E, 0x015B, 0x02C7,
    0x00B8, 0x0161, 0x015F, 0x0165, 0x017A, 0x02DD, 0x017E, 0x017C,
    0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7,
    0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
    0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7,
    0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
    0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7,
    0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
    0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7,
    0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9
};

static uint16_t _ISO8859_3_0xA0TO0xBF[32] =
{
    0x00A0, 0x0126, 0x02D8, 0x00A3, 0x00A4, 0x00A5, 0x0124, 0x00A7,
    0x00A8, 0x0130, 0x015E, 0x011E, 0x0134, 0x00AD, 0x00AE, 0x017B,
    0x00B0, 0x0127, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x0125, 0x00B7,
    0x00B8, 0x0131, 0x015F, 0x011F, 0x0135, 0x00BD, 0x00BE, 0x017C
};

static uint16_t _ISO8859_3_0xF0TO0xFF[16] =
{
    0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x0121, 0x00F6, 0x00F7,
    0x011D, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x016D, 0x015D, 0x02D9
};

static uint16_t _ISO8859_4[96] =
{
    0x00A0, 0x0104, 0x0138, 0x0156, 0x00A4, 0x0128, 0x013B, 0x00A7,
    0x00A8, 0x0160, 0x0112, 0x0122, 0x0166, 0x00AD, 0x017D, 0x00AF,
    0x00B0, 0x0105, 0x02DB, 0x0157, 0x00B4, 0x0129, 0x013C, 0x02C7,
    0x00B8, 0x0161, 0x0113, 0x0123, 0x0167, 0x014A, 0x017E, 0x014B,
    0x0100, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x012E,
    0x010C, 0x00C9, 0x0118, 0x00CB, 0x0116, 0x00CD, 0x00CE, 0x012A,
    0x0110, 0x0145, 0x014C, 0x0136, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
    0x00D8, 0x0172, 0x00DA, 0x00DB, 0x00DC, 0x0168, 0x016A, 0x00DF,
    0x0101, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x012F,
    0x010D, 0x00E9, 0x0119, 0x00EB, 0x0117, 0x00ED, 0x00EE, 0x012B,
    0x0111, 0x0146, 0x014D, 0x0137, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
    0x00F8, 0x0173, 0x00FA, 0x00FB, 0x00FC, 0x0169, 0x016B, 0x02D9
};

static uint16_t _ISO8859_7_0xA0TO0xBD[30] =
{
    0x00A0, 0x2018, 0x2019, 0x00A3, 0x20AC, 0x20AF, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x037A, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x2015,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x0385, 0x0386, 0x00B7,
    0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD
};

static uint16_t _ISO8859_10[96] =
{
    0x00A0, 0x0104, 0x0112, 0x0122, 0x012A, 0x0128, 0x0136, 0x00A7,
    0x013B, 0x0110, 0x0160, 0x0166, 0x017D, 0x00AD, 0x016A, 0x014A,
    0x00B0, 0x0105, 0x0113, 0x0123, 0x012B, 0x0129, 0x0137, 0x00B7,
    0x013C, 0x0111, 0x0161, 0x0167, 0x017E, 0x2015, 0x016B, 0x014B,
    0x0100, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x012E,
    0x010C, 0x00C9, 0x0118, 0x00CB, 0x0116, 0x00CD, 0x00CE, 0x00CF,
    0x00D0, 0x0145, 0x014C, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x0168,
    0x00D8, 0x0172, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
    0x0101, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x012F,
    0x010D, 0x00E9, 0x0119, 0x00EB, 0x0117, 0x00ED, 0x00EE, 0x00EF,
    0x00F0, 0x0146, 0x014D, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x0169,
    0x00F8, 0x0173, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x0138
};

static uint16_t _ISO8859_13[96] =
{
    0x00A0, 0x201D, 0x00A2, 0x00A3, 0x00A4, 0x201E, 0x00A6, 0x00A7,
    0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x201C, 0x00B5, 0x00B6, 0x00B7,
    0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6,
    0x0104, 0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112,
    0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012A, 0x013B,
    0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7,
    0x0172, 0x0141, 0x015A, 0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF,
    0x0105, 0x012F, 0x0101, 0x0107, 0x00E4, 0x00E5, 0x0119, 0x0113,
    0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C,
    0x0161, 0x0144, 0x0146, 0x00F3, 0x014D, 0x00F5, 0x00F6, 0x00F7,
    0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0x2019
};

static uint16_t _ISO8859_14_0xA0TO0xBF[32] =
{
    0x00A0, 0x1E02, 0x1E03, 0x00A3, 0x010A, 0x010B, 0x1E0A, 0x00A7,
    0x1E80, 0x00A9, 0x1E82, 0x1E0B, 0x1EF2, 0x00AD, 0x00AE, 0x0178,
    0x1E1E, 0x1E1F, 0x0120, 0x0121, 0x1E40, 0x1E41, 0x00B6, 0x1E56,
    0x1E81, 0x1E57, 0x1E83, 0x1E60, 0x1EF3, 0x1E84, 0x1E85, 0x1E61
};

static uint16_t _ISO8859_15_0xA4TO0xA8[5] =
{
    0x20AC, 0x00A5, 0x0160, 0x00A7, 0x0161
};

static uint16_t _ISO8859_15_0xB4TO0xBE[11] =
{
    0x017D, 0x00B5, 0x00B6, 0x00B7, 0x017E, 0x00B9, 0x00BA, 0x00BB,
    0x0152, 0x0153, 0x0178
};

static ISO8859ToUTF16 _ISO8859ToUTF16_TABLE[15] =
{
    _ISO8859_1ToUTF16,
    _ISO8859_2ToUTF16,
    _ISO8859_3ToUTF16,
    _ISO8859_4ToUTF16,
    _ISO8859_5ToUTF16,
    _ISO8859_6ToUTF16,
    _ISO8859_7ToUTF16,
    _ISO8859_8ToUTF16,
    _ISO8859_9ToUTF16,
    _ISO8859_10ToUTF16,
    _ISO8859_11ToUTF16,
    MMP_NULL,
    _ISO8859_13ToUTF16,
    _ISO8859_14ToUTF16,
    _ISO8859_15ToUTF16,
};

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Get the ISO 8859 to Unicode converter.
 *
 * @param stdIndex  An index value indicating which standard the converter
 *                  supports.
 *                  ex.  1 => ISO8559-1
 *                       2 => ISO8559-2
 *                       3 => ISO8559-3
 *                         ...
 *                      15 => ISO8559-15
 * @return          The ISO 8859 to Unicode converter.
 */
//=============================================================================
ISO8859ToUTF16
ISO8859_GetConverter(
    MMP_UINT stdIndex)
{
    if (1 <= stdIndex && stdIndex <= 15)
    {
        return _ISO8859ToUTF16_TABLE[stdIndex - 1];
    }
    return MMP_NULL;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Convert an ISO 8859-1 character to Unicode.
 *
 * @param ch    An ISO 8859-1 character.
 * @return      The corresponding Unicode character of the input character.
 */
//=============================================================================
MMP_INLINE uint16_t
_ISO8859_1ToUTF16(
    MMP_UINT8 ch)
{
    return ch;
}

//=============================================================================
/**
 * Convert an ISO 8859-2 character to Unicode.
 *
 * @param ch    An ISO 8859-2 character.
 * @return      The corresponding Unicode character of the input character.
 */
//=============================================================================
MMP_INLINE uint16_t
_ISO8859_2ToUTF16(
    MMP_UINT8 ch)
{
    return (0xA0 < ch) ? _ISO8859_2[ch - 0xA0] : ch;
}

//=============================================================================
/**
 * Convert an ISO 8859-3 character to Unicode.
 *
 * @param ch    An ISO 8859-3 character.
 * @return      The corresponding Unicode character of the input character.
 */
//=============================================================================
MMP_INLINE uint16_t
_ISO8859_3ToUTF16(
    MMP_UINT8 ch)
{
    if (0xA0 < ch)
    {
        switch(ch & 0xF0)
        {
        case 0xA0:
        case 0xB0:
            return _ISO8859_3_0xA0TO0xBF[ch - 0xA0];
        case 0xC0:
            switch (ch)
            {
            case 0xC5:  return 0x10A;
            case 0xC6:  return 0x108;
            }
            break;
        case 0xD0:
            switch (ch)
            {
            case 0xD5:  return 0x120;
            case 0xD8:  return 0x11C;
            case 0xDD:  return 0x16C;
            case 0xDE:  return 0x15C;
            }
            break;
        case 0xE0:
            switch (ch)
            {
            case 0xE5:  return 0x10B;
            case 0xE6:  return 0x109;
            }
            break;
        case 0xF0:
            return _ISO8859_3_0xF0TO0xFF[ch - 0xF0];
        }
    }
    return ch;
}

//=============================================================================
/**
 * Convert an ISO 8859-4 character to Unicode.
 *
 * @param ch    An ISO 8859-4 character.
 * @return      The corresponding Unicode character of the input character.
 */
//=============================================================================
MMP_INLINE uint16_t
_ISO8859_4ToUTF16(
    MMP_UINT8 ch)
{
    return (0xA0 < ch) ? _ISO8859_4[ch - 0xA0] : ch;
}

//=============================================================================
/**
 * Convert an ISO 8859-5 character to Unicode.
 *
 * @param ch    An ISO 8859-5 character.
 * @return      The corresponding Unicode character of the input character.
 */
//=============================================================================
MMP_INLINE uint16_t
_ISO8859_5ToUTF16(
    MMP_UINT8 ch)
{
    if (0xA0 < ch)
    {
        switch (ch)
        {
        case 0xAD:  return 0x00AD;
        case 0xF0:  return 0x2116;
        case 0xFD:  return 0x00A7;
        }
        return ch + 0x360;
    }
    return ch;
}

//=============================================================================
/**
 * Convert an ISO 8859-6 character to Unicode.
 *
 * @param ch    An ISO 8859-6 character.
 * @return      The corresponding Unicode character of the input character.
 */
//=============================================================================
MMP_INLINE uint16_t
_ISO8859_6ToUTF16(
    MMP_UINT8 ch)
{
    if (0xA0 <  ch
     && 0xA4 != ch
     && 0xAD != ch)
    {
        return ch + 0x560;
    }
    return ch;
}

//=============================================================================
/**
 * Convert an ISO 8859-7 character to Unicode.
 *
 * @param ch    An ISO 8859-7 character.
 * @return      The corresponding Unicode character of the input character.
 */
//=============================================================================
MMP_INLINE uint16_t
_ISO8859_7ToUTF16(
    MMP_UINT8 ch)
{
    if (0xA0 < ch)
    {
        if (ch <= 0xBD)
            return _ISO8859_7_0xA0TO0xBD[ch - 0xA0];
        else
            return ch + 0x2D0;
    }
    return ch;
}

//=============================================================================
/**
 * Convert an ISO 8859-8 character to Unicode.
 *
 * @param ch    An ISO 8859-8 character.
 * @return      The corresponding Unicode character of the input character.
 */
//=============================================================================
MMP_INLINE uint16_t
_ISO8859_8ToUTF16(
    MMP_UINT8 ch)
{
    if (0xAA <= ch)
    {
        if (ch <= 0xDF)         // ch = 0xAA ~ 0xDF
        {
            switch (ch)
            {
            case 0xAA:  return 0x00D7;
            case 0xBA:  return 0x00F7;
            case 0xDF:  return 0x2017;
            default:    return ch;
            }
        }
        else if (ch <= 0xFA)    // ch = 0xE0 ~ 0xFA
        {
            return ch + 0x4F0;
        }
        else                    // ch = 0xFB ~ 0xFF
        {
            return ch + 0x1F11;
        }
    }
    return ch;
}

//=============================================================================
/**
 * Convert an ISO 8859-9 character to Unicode.
 *
 * @param ch    An ISO 8859-9 character.
 * @return      The corresponding Unicode character of the input character.
 */
//=============================================================================
MMP_INLINE uint16_t
_ISO8859_9ToUTF16(
    MMP_UINT8 ch)
{
    if (0xD0 <= ch)
    {
        switch (ch)
        {
        case 0xD0:  return 0x11E;
        case 0xDD:  return 0x130;
        case 0xDE:  return 0x15E;
        case 0xF0:  return 0x11F;
        case 0xFD:  return 0x131;
        case 0xFE:  return 0x15F;
        }
    }
    return ch;
}

//=============================================================================
/**
 * Convert an ISO 8859-10 character to Unicode.
 *
 * @param ch    An ISO 8859-10 character.
 * @return      The corresponding Unicode character of the input character.
 */
//=============================================================================
MMP_INLINE uint16_t
_ISO8859_10ToUTF16(
    MMP_UINT8 ch)
{
    return (0xA0 < ch) ? _ISO8859_10[ch - 0xA0] : ch;
}

//=============================================================================
/**
 * Convert an ISO 8859-11 character to Unicode.
 *
 * @param ch    An ISO 8859-11 character.
 * @return      The corresponding Unicode character of the input character.
 */
//=============================================================================
MMP_INLINE uint16_t
_ISO8859_11ToUTF16(
    MMP_UINT8 ch)
{
    return (0xA0 < ch) ? (ch + 0xD60) : ch;
}

//=============================================================================
/**
 * Convert an ISO 8859-13 character to Unicode.
 *
 * @param ch    An ISO 8859-13 character.
 * @return      The corresponding Unicode character of the input character.
 */
//=============================================================================
MMP_INLINE uint16_t
_ISO8859_13ToUTF16(
    MMP_UINT8 ch)
{
    return (0xA0 < ch) ? _ISO8859_13[ch - 0xA0] : ch;
}

//=============================================================================
/**
 * Convert an ISO 8859-14 character to Unicode.
 *
 * @param ch    An ISO 8859-14 character.
 * @return      The corresponding Unicode character of the input character.
 */
//=============================================================================
MMP_INLINE uint16_t
_ISO8859_14ToUTF16(
    MMP_UINT8 ch)
{
    if (0xA0 < ch)
    {
        switch (ch & 0xF0)
        {
        case 0xA0:
        case 0xB0:
            return _ISO8859_14_0xA0TO0xBF[ch - 0xA0];
        case 0xC0:
        case 0xE0:
            return ch;
        case 0xD0:
            switch (ch)
            {
            case 0xD0:  return 0x0174;
            case 0xD7:  return 0x1E6A;
            case 0xDE:  return 0x0176;
            }
            return ch;
        case 0xF0:
            switch (ch)
            {
            case 0xF0:  return 0x0175;
            case 0xF7:  return 0x1E6B;
            case 0xFE:  return 0x0177;
            }
            return ch;
        }

    }
    return ch;
}

//=============================================================================
/**
 * Convert an ISO 8859-15 character to Unicode.
 *
 * @param ch    An ISO 8859-15 character.
 * @return      The corresponding Unicode character of the input character.
 */
//=============================================================================
MMP_INLINE uint16_t
_ISO8859_15ToUTF16(
    MMP_UINT8 ch)
{
    if (0xA4 <= ch && ch <= 0xBE)
    {
        if (ch <= 0xA8)         // ch = 0xA4 ~ 0xA8
        {
            return _ISO8859_15_0xA4TO0xA8[ch - 0xA4];
        }
        else if (ch <= 0xB3)    // ch = 0xA9 ~ 0xB3
        {
            return ch;
        }
        else                    // ch = 0xB4 ~ 0xBE
        {
            return _ISO8859_15_0xB4TO0xBE[ch - 0xB4];
        }
    }
    return ch;
}
