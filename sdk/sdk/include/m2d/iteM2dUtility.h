/*
 * Copyright (c) 2011 ITE, Awin Huang
 *
 */

#ifndef __ITEM2DUTILITY_H
#define __ITEM2DUTILITY_H

#include "m2d/iteM2dDefs.h"

ITEM2Dboolean
CheckAlignment(
	const void* memAddr,
	ITEM2Duint      align);

ITEM2Duint
GetAlignment(
	ITEM2Duint value,
	ITEM2Duint align);

ITEM2Duint
_mathFloatToS12_15(
	ITEM2Duint*	f);

/*--------------------------------------------
 * IEEE 754, s8.23, base 127
 *--------------------------------------------*/
ITEM2Duint
GetSingleFloatMantissa(
	ITEM2Dfloat value);

#endif /* __ITEM2DUTILITY_H */
