/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : typedef.c
*      Purpose          : Basic types.
*
********************************************************************************
*/

#ifndef typedef_h
#define typedef_h "$Id $"

#include "defines.h"
#if defined(__OR32__) || defined(OPRISCENG)
//#include "engine.h"
#endif

//#define WMOPS 1

//#define MM365AMR
//#define MM365ENGINE

//#define PureC
// 2004.9.8 sis3830 For OpenRISC compiler
#if defined(__OR32__) || defined(__CYGWIN__)
typedef long long   Word64;
#else
typedef __int64     Word64;
#endif

#define _CheckOverflow(_hi, _lo, _out)          \
    if( (_hi) > 0 ) {                           \
        (_out) = MAX_32;                        \
    } else if( (_hi) == 0) {                    \
        if(( (_lo) & 0xc0000000)!=0)            \
            (_out) = MAX_32;                    \
        else                                    \
            (_out) = (_lo) << 1;                \
    } else {                                    \
        if( (_hi) < -1)                         \
            (_out) = MIN_32;                    \
        else                                    \
        {                                       \
            if( (_lo) <= ((Word32)0xc0000000) || (_lo) > 0)     \
                (_out) = MIN_32;                \
            else                                \
                (_out) = (_lo) <<1;             \
        }                                       \
    }

#define _CheckOverflow_P(_hi, _lo, _out)        \
    if( (_hi) > 0) {                            \
         (_out) = MAX_32;                       \
    } else {                                    \
        if(( (_lo) & 0xc0000000) !=0 )          \
            (_out) = MAX_32;                    \
        else                                    \
            (_out) = (_lo) <<1;                 \
    }

// end

#undef ORIGINAL_TYPEDEF_H /* define to get "original" ETSI version
                            of typedef.h                           */

#ifdef ORIGINAL_TYPEDEF_H
/*
 * this is the original code from the ETSI file typedef.h
 */

#if defined(__BORLANDC__) || defined(__WATCOMC__) || defined(_MSC_VER) || defined(__ZTC__)
typedef signed char Word8;
typedef short Word16;
typedef long Word32;
typedef signed char Flag;

#elif defined(__sun)
typedef signed char Word8;
typedef short Word16;
typedef long Word32;
typedef signed char Flag;

#elif defined(__unix__) || defined(__unix)
typedef signed char Word8;
typedef short Word16;
typedef int Word32;
typedef signed char Flag;

#endif
#else /* not original typedef.h */

/*
 * use (improved) type definition file typdefs.h and add a "Flag" type
 */
#include "typedefs.h"
typedef signed char Flag;

#endif

/*
 * Merging ENGINE
 */

typedef unsigned char       UINT8;
typedef unsigned short      SWORD;
typedef unsigned long       DWORD;
typedef unsigned char        BOOL;
typedef char                INT08;
typedef signed short        INT16;
typedef long                INT32;

#if defined(__OR32__) || defined(__CYGWIN__)
typedef long long           INT64;
#else
typedef __int64             INT64;
#endif

#endif
