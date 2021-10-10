/*
 * Copyright (c) 2011 ITE, Awin Huang
 *
 */

#ifndef __ITEUTILITY_H
#define __ITEUTILITY_H

#include "openvg.h"

#ifdef __SM32__
VGuint
ivgByteSwap32(
	VGuint value);
#else
#define ivgByteSwap32(x)	x
#endif

VGboolean
CheckAlignment(
	const void* memAddr,
	VGuint      align);

VGuint
GetAlignment(
	VGuint value,
	VGuint align);

VGuint
_mathFloatToS12_15(
	VGuint*	f);

/*--------------------------------------------
 * IEEE 754, s8.23, base 127
 *--------------------------------------------*/
VGuint
GetSingleFloatMantissa(
	VGfloat value);

#endif /* __ITEUTILITY_H */
