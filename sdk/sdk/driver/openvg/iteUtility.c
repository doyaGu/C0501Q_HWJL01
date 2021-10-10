#include "iteUtility.h"

#ifdef __SM32__
VGuint
ivgByteSwap32(
	VGuint value)
{
	return ((value & 0x000000ff) << 24) |
           ((value & 0x0000ff00) <<  8) |
           ((value & 0x00ff0000) >>  8) |
           (value >> 24);
}
#endif

VGboolean
CheckAlignment(
	const void* memAddr,
	VGuint      align)
{
	VGuint mask = ~(align - 1);
	VGuint addr = (VGuint)memAddr;
    VGuint ptr  = (addr + align - 1) & mask;
    
    return addr == ptr;
}

VGuint
GetAlignment(
	VGuint value,
	VGuint align)
{
	VGuint mask = ~(align - 1);
    VGuint ptr  = (value + align - 1) & mask;
    
    return ptr;
}

VGuint
_mathFloatToS12_15(
	VGuint*	f)
{
	VGuint value = *f;
	return (((value >> 7) & 0x7FFFFFF) | ((value >> 8) & ~0x7FFFFFF));
}

/*--------------------------------------------
 * IEEE 754, s8.23, base 127
 *--------------------------------------------*/
VGuint
GetSingleFloatMantissa(
	VGfloat value)
{
	void*  pf   = &value;
	VGuint temp = *(VGuint*)pf;
	VGint  s    = temp & 0x80000000;
	VGint  e    = ((temp & 0x7F800000) >> 23) - 127;
	VGuint m    = 0;

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
