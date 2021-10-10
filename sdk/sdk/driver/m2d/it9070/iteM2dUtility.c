#include "m2d/iteM2dUtility.h"

ITEM2Dboolean
CheckAlignment(
	const void* memAddr,
	ITEM2Duint      align)
{
	ITEM2Duint mask = ~(align - 1);
	ITEM2Duint addr = (ITEM2Duint)memAddr;
    ITEM2Duint ptr  = (addr + align - 1) & mask;
    
    return addr == ptr;
}

ITEM2Duint
GetAlignment(
	ITEM2Duint value,
	ITEM2Duint align)
{
	ITEM2Duint mask = ~(align - 1);
    ITEM2Duint ptr  = (value + align - 1) & mask;
    
    return ptr;
}

ITEM2Duint
_mathFloatToS12_15(
	ITEM2Duint*	f)
{
	ITEM2Duint value = *f;
	return (((value >> 7) & 0x7FFFFFF) | ((value >> 8) & ~0x7FFFFFF));
}

/*--------------------------------------------
 * IEEE 754, s8.23, base 127
 *--------------------------------------------*/
ITEM2Duint
GetSingleFloatMantissa(
	ITEM2Dfloat value)
{
	void*  pf   = &value;
	ITEM2Duint temp = *(ITEM2Duint*)pf;
	ITEM2Dint  s    = temp & 0x80000000;
	ITEM2Dint  e    = ((temp & 0x7F800000) >> 23) - 127;
	ITEM2Duint m    = 0;

	if ( e > 0 )
	{
		m = (((temp & 0x007FFFFF) + (1<<23)) << 1) << e;
	}
	else
	{
		e = -e;
		m = (((temp & 0x007FFFFF) + (1<<23)) << 1) >> e;
	}

	if ( s )
	{
		m = ~m + 1;
	}

	return m;
}
