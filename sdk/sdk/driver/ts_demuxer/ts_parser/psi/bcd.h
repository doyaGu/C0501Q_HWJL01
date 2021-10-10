/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file bcd.h
 * BCD arithmetic for Teletext page numbers.
 * Teletext page numbers are expressed as packed binary coded decimal
 * numbers in range 0x100 to 0x8FF. The bcd format encodes one decimal
 * digit in every hex nibble (four bits) of the number. Page numbers
 * containing digits 0xA to 0xF are reserved for various system purposes
 * and not intended for display.
 *
 * @author I-Chun Lai
 * @version 0.1
 */

#ifndef BCD_H
#define BCD_H

#include "ite/mmp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Macro Definition
//=============================================================================
#define _BCD_10 (((MMP_INT32) 0x11111111L) << 4)
#define _BCD_06 (((MMP_INT32) 0x66666666L) >> 4)

#define BCD_MIN (0xF << (sizeof(MMP_INT32) * 8 - 4))
#define BCD_MAX (BCD_MIN ^ ~_BCD_06)

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Converts a two's complement binary between 0 ... 999 to a
 * packed bcd number in range  0x000 ... 0x999. Extra digits in
 * the input will be discarded.
 *
 * @param dec Decimal number.
 * @return BCD number.
 */
//=============================================================================
static MMP_INLINE MMP_UINT32
bcd_Dec2Bcd(
    MMP_UINT32 dec)
{
    return (dec % 10) + ((dec / 10) % 10) * 16 + ((dec / 100) % 10) * 256;
}

//=============================================================================
/**
 * Converts a packed bcd number between 0x000 ... 0xFFF to a two's
 * complement binary in range 0 ... 999. Extra digits in the input
 * will be discarded.
 *
 * @param bcd   BCD number.
 * @return Decimal number. The result is undefined when the bcd number contains
 *         hex digits 0xA ... 0xF.
 **/
//=============================================================================
static MMP_INLINE MMP_UINT32
bcd_Bcd2Dec(
    MMP_UINT32 bcd)
{
    return (bcd & 15) + ((bcd >> 4) & 15) * 10 + ((bcd >> 8) & 15) * 100;
}

//=============================================================================
/**
 * Adds two signed packed bcd numbers, returning a signed packed bcd sum.
 *
 * @param a     BCD number.
 * @param b     BCD number.
 * @return Packed bcd number. The result is undefined when any of the arguments
 *         contain hex digits 0xA ... 0xF, except for the sign nibble.
 */
//=============================================================================
static MMP_INLINE MMP_INT32
bcd_AddBcd(
    MMP_INT32 a,
    MMP_INT32 b)
{
    MMP_INT32 t = a + (b += _BCD_06);

    a  = ((~(b ^ a ^ t) & _BCD_10) >> 3) * 3;

    return t - a;
}

//=============================================================================
/**
 * Calculates the ten's complement of a signed packed bcd. The most
 * significant nibble is the sign, e.g. 0xF9999999 = bcd_NegBcd
 * (0x000000001), assumed sizeof(MMP_INT32) is 4.
 *
 * @param bcd BCD number.
 *
 * @return BCD number. The result is undefined when any of the arguments
 *         contain hex digits 0xA ... 0xF, except for the sign nibble.
 * @remark the ten's complement of BCD_MIN is not representable
 *         as signed packed bcd, this function will return BCD_MAX + 1
 *         (0x10000000) instead.
 */
//=============================================================================
static MMP_INLINE MMP_INT32
bcd_NegBcd(
    MMP_INT32 bcd)
{
    MMP_INT32 t = -bcd;

    return t - (((bcd ^ t) & _BCD_10) >> 3) * 3;
}

//=============================================================================
/**
 * @param a BCD number.
 * @param b BCD number.
 *
 * Subtracts two signed packed bcd numbers, returning a - b. The result
 * may be negative (ten's complement), see vbi3_neg_bcd().
 *
 * @return
 * BCD number. The result is undefined when any of the arguments
 * contain hex digits 0xA ... 0xF, except for the sign nibble.
 */
//=============================================================================
static MMP_INLINE MMP_INT32
bcd_SubBcd(
    MMP_INT32 a,
    MMP_INT32 b)
{
    return bcd_AddBcd(a, bcd_NegBcd(b));
}

//=============================================================================
/**
 * Tests if the input bcd value forms a valid signed packed bcd number.
 *
 * @param bcd BCD number.
 * @return  MMP_FALSE if the input bcd value contains hex digits 0xA ... 0xF.
 *          ignoring the four most significant bits i.e. the sign nibble.
 */
//=============================================================================
static MMP_INLINE MMP_BOOL
bcd_IsBcd(
    MMP_INT32 bcd)
{
    bcd &= ~BCD_MIN;

    return (0 == (((bcd + _BCD_06) ^ bcd ^ _BCD_06) & _BCD_10))
        ? MMP_TRUE : MMP_FALSE;
}

//=============================================================================
/**
 * Compares an unsigned packed bcd number digit-wise against a maximum
 * value, for example 0x295959. The input maximum can contain digits 0x0
 * ... 0xF.
 *
 * @param bcd       Unsigned BCD number.
 * @param maximum   Unsigned maximum value.
 * @return
 *      MMP_TRUE if any digit of bcd is greater than the corresponding digit
 *      of maximum.
 */
//=============================================================================
static MMP_INLINE MMP_BOOL
bcd_IsGreater(
    MMP_UINT bcd,
    MMP_UINT maximum)
{
    maximum ^= ~0;

    return (0 != (((bcd + maximum) ^ bcd ^ maximum) & 0x11111110))
        ? MMP_TRUE : MMP_FALSE;
}

#ifdef __cplusplus
}
#endif

#endif
