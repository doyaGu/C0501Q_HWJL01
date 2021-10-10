/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file iso8859.h
 * Ultility functions used to convert ISO/IEC 8859 series of character sets
 * to Unicode. The mapping table from ISO 8859 character sets to Unicode can
 * be found in http://unicode.org/Public/MAPPINGS/ISO8859/.
 *
 * @author I-Chun Lai
 * @version 0.1
 */

#ifndef ISO8859_H
#define ISO8859_H

#include "ite/mmp_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
/**
 * Convert an ISO 8859-X character to Unicode.
 *
 * @param ch    An ISO 8859-X character.
 * @return      The corresponding Unicode character of the input character.
 */
//=============================================================================
typedef uint16_t (*ISO8859ToUTF16)(MMP_UINT8 ch);

//=============================================================================
//                              Function Declaration
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
 *
 * @example         #define ISO8859_9   9
 *
 *                  ISO8859ToUTF16 pfConverter = ISO8859_GetConverter(ISO8859_9);
 *                  if (pfConverter)
 *                  {
 *                      MMP_UINT8 src[4] = {0xA8, 0x32, 0xB6, 0x0};
 *                      uint16_t dst[4] = {0};
 *                      MMP_INT strIndex;
 *
 *                      for (strIndex; ; ++strIndex)
 *                      {
 *                          dst[strIndex] = pfConverter(src[strIndex]);
 *                          if (src[strIndex] == 0)
 *                              break;
 *                      }
 *                  }
 */
//=============================================================================
ISO8859ToUTF16
ISO8859_GetConverter(
    MMP_UINT stdIndex);

#ifdef __cplusplus
}
#endif

#endif

