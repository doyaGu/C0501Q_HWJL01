/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file iso6937.h
 * Ultility functions used to convert ISO/IEC 6937 series of character sets
 * to Unicode. The mapping table from ISO 6937 character sets to Unicode can
 * be found in http://en.wikipedia.org/wiki/ISO/IEC_6937.
 *
 * @author I-Chun Lai
 * @version 0.1
 */

#ifndef ISO6937_H
#define ISO6937_H

#include "ite/mmp_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Convert the DVB text to UTF-16
 * @param pOut      The output UTF-16 string.
 * @param pIn       The input ISO/IEC string.
 * @param length    The length of the input ISO/IEC string.
 * @return The number of UTF-16 characters in a UTF-16 character string
 */
//=============================================================================
MMP_UINT32
ISO6937ToUTF16(
    uint16_t   *pOut,
    MMP_UINT8  *pIn,
    MMP_UINT32 length);

#ifdef __cplusplus
}
#endif

#endif