#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H

# define HZ (1000)
# define NOFILE	(60)
# define PATHSIZE (1024)

#define BIG_ENDIAN      4321
#define LITTLE_ENDIAN   1234

#define BYTE_ORDER LITTLE_ENDIAN

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

#endif
