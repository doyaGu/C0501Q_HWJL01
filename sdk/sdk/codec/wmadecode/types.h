
/**
 * @file types.h
 *  typedef
 */

#ifndef __TYPES_H__
#define __TYPES_H__

typedef int                fixed32;

#if defined(WIN32)
# define INT64_T(n)        (n##i64)
typedef unsigned __int64   uint64_t;
#else
# define INT64_T(n)        (n##LL)
typedef long long          fixed64;
#endif

#if !defined(__CYGWIN__)
typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;

#if defined(WIN32)
typedef __int64            fixed64;
typedef __int64            int64_t;
#else
typedef long long          int64_t;
typedef unsigned long long uint64_t;
#endif
#else
#include <inttypes.h>
#endif // __CYGWIN__

#define TRUE    1
#define FALSE   0

enum {
    ERR_NONE                               =   0,
    ERR_EXCEED_MAX_CODED_SUPERFRAME_SIZE   =  -1,
    ERR_DECODE_BLK_ERR1                    =  -2,
    ERR_DECODE_BLK_ERR2                    =  -3,
    ERR_DECODE_BLK_ERR3                    =  -4,
    ERR_DECODE_BLK_ERR4                    =  -5,
    ERR_DECODE_BLK_ERR5                    =  -6,
    ERR_DECODE_BLK_ERR6                    =  -7,
    ERR_DECODE_BLK_ERR7                    =  -8,
    ERR_DECODE_BLK_ERR8                    =  -9,
    ERR_VLC_INIT_ERR                       = -10,
    ERR_INVLAID_VLC                        = -11,
    ERR_NB_FRAME_ZERO                      = -12,  // Add error control for superframe counter = 0 by Viola on 2009.01.22
    ERR_UNKNOWN                            = -9999
};

int read_filebuf(void *buf, unsigned int nReadBytes);
void advance_buffer(int nReadBytes);

#endif // __TYPES_H__

