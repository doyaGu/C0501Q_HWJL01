
#include "iso6937.h"
#include "iso8859.h"
#include "utf8.h"
#include "ts_txt_conv.h"
//=============================================================================
//                  Constant Definition
//=============================================================================


//=============================================================================
//                  Macro Definition
//=============================================================================


//=============================================================================
//                  Structure Definition
//=============================================================================


//=============================================================================
//                  Global Data Definition
//=============================================================================


//=============================================================================
//                  Private Function Definition
//=============================================================================
static uint32_t
_DTV_TextAddCtrlCode(
    uint16_t    *pOut,
    uint32_t    length)
{
    uint32_t    strIndex = 0;
    uint32_t    lastChrCount = 0;

    for (;strIndex < length; ++strIndex, ++pOut)
    {
        switch( *pOut )
        {
            case 0x000D:    //for evora test 697
            case 0x008A:
            case 0xE08A:
                //add new line ("CR/LF","\n")
                (*pOut) = 0x0A;
                break;

            case 0x0:       //nil("\0")
            case 0x0086:    //emphasis on
            case 0xE086:    //emphasis on
            case 0x0087:    //emphasis off
            case 0xE087:    //emphasis off
                //remove nil("\0") & emphasis on/off character
                lastChrCount = length - strIndex - 1;

                memcpy(pOut, pOut + 1, lastChrCount * sizeof(uint16_t));
                (*(pOut + lastChrCount)) = 0x00;
                --pOut;
                --strIndex;
                --length;
                break;
        }
    }

    return length;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
//=============================================================================
/**
 * Convert the DVB text to UTF-16
 * @param pOut      The output UTF-16 string.
 * @param pIn       The input DVB text. The coding of DVB text can be found in
 *                  EN 300 468 Annex A.
 * @param length    The length of the input DVB text.
 * @return The number of UTF-16 characters in a UTF-16 character string
 */
//=============================================================================
uint32_t
tsTxt_ConvToUtf16(
    uint16_t        *pOut,
    uint8_t         *pIn,
    uint32_t        length,
    TS_CHAR_CODE    charCode)
{
    uint32_t    strIndex = 0;

    if (pIn && length > 0)
    {
        if ((*pIn) < 0x20)
        {
            if (0x01 <= (*pIn) && (*pIn) <= 0x0B && length > 1)
            {
                ISO8859ToUTF16 pfConverter = ISO8859_GetConverter((*pIn) + 4);
                if (pfConverter)
                {
                    --length;
                    if (pOut)
                    {
                        for (strIndex = 0;
                             strIndex < length;
                             ++strIndex, ++pOut)
                        {
                            (*pOut) = pfConverter(*(++pIn));
                        }
                        (*pOut) = 0;
                    }
                }
            }
            else if (0x10 == (*pIn) && length > 3)
            {
                ISO8859ToUTF16 pfConverter = MMP_NULL;

                pIn += 2;
                pfConverter = ISO8859_GetConverter((*pIn));
                if (pfConverter)
                {
                    length -= 3;
                    if (pOut)
                    {
                        for (strIndex = 0;
                             strIndex < length;
                             ++strIndex, ++pOut)
                        {
                            (*pOut) = pfConverter(*(++pIn));
                        }
                        (*pOut) = 0;
                    }
                }
            }
            else if (0x11 <= (*pIn) && (*pIn) <= 0x14 && length > 1)
            {
                ++pIn;
                length = (--length) >> 1;
                if (pOut)
                {
                    for (strIndex = 0;
                         strIndex < length;
                         ++strIndex, ++pOut, pIn += 2)
                    {
                        (*pOut) = ((*(pIn)) << 8) + (*(pIn + 1));
                    }
                    (*pOut) = 0;
                }
            }
            else if ((*pIn) == 0x15 && length > 1)
            {
                ++pIn;
                --length;
                length = UTF8ToUTF16(pOut, pIn, length);
            }
            else if ((*pIn) == 0)
            {
                if (pOut)
                {
                    (*pOut) = 0;
                }
                length = 0;
            }
        }
        else
        {
            ISO8859ToUTF16  pfConverter;
            switch (charCode)
            {
            case TS_ISO_IEC_8859_1:
            case TS_ISO_IEC_8859_2:
            case TS_ISO_IEC_8859_3:
            case TS_ISO_IEC_8859_4:
            case TS_ISO_IEC_8859_5:
            case TS_ISO_IEC_8859_6:
            case TS_ISO_IEC_8859_7:
            case TS_ISO_IEC_8859_8:
            case TS_ISO_IEC_8859_9:
            case TS_ISO_IEC_8859_10:
            case TS_ISO_IEC_8859_11:
            case TS_ISO_IEC_8859_13:
            case TS_ISO_IEC_8859_14:
            case TS_ISO_IEC_8859_15:
                pfConverter = ISO8859_GetConverter(charCode);
                if (pfConverter)
                {
                    if (pOut)
                    {
                        for (strIndex = 0;
                             strIndex < length;
                             ++strIndex, ++pOut)
                        {
                            (*pOut) = pfConverter(*(pIn++));
                        }
                        (*pOut) = 0;
                    }
                }
                break;

            default:
                length = ISO6937ToUTF16(pOut, pIn, length);
                break;
            }
        }
    }
    else
    {
        if (pOut)
        {
            (*pOut) = 0;
        }
        length = 0;
    }

    if (pOut)
        length = _DTV_TextAddCtrlCode(pOut - strIndex, length);

    return length;
}

