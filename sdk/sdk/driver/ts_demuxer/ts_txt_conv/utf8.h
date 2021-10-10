/*
 * Copyright (c) 2011 ITE Technology Corp. All Rights Reserved.
 */
/** @file utf8.h
 * Ultility functions used to convert UTF-8 character sets to Unicode.
 *
 * @author I-Chun Lai
 * @version 0.1
 */

#ifndef UTF8_H
#define UTF8_H

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
    MMP_UINT32 length);

#ifdef __cplusplus
}
#endif

#endif
