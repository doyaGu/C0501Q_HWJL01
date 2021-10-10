#ifndef ITP_SYS_PARAM_H
#define ITP_SYS_PARAM_H

#include_next <sys/param.h>

#undef HZ
#define HZ 1000

/*
 * Round p (pointer or byte index) up to a correctly-aligned value
 * for all data types (int, long, ...).   The result is unsigned int
 * and must be cast to any desired pointer type.
 */
#ifndef _ALIGNBYTES
#define _ALIGNBYTES	(sizeof(int) - 1)
#endif
#ifndef _ALIGN
#define _ALIGN(p)	(((unsigned)(p) + _ALIGNBYTES) & ~_ALIGNBYTES)
#endif

#endif // ITP_SYS_PARAM_H
