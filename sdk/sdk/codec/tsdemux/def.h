/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Platform definitions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#ifndef MMP_DEF_H
#define MMP_DEF_H

/** Debug definition */
// #define MMP_DEBUG

/** Endian definition */
//#define MMP_BIG_ENDIAN

/** Inline definition */
#define MMP_INLINE __inline

/** Type definition */
typedef unsigned int    MMP_BOOL;
typedef char            MMP_INT8;   /**< 8-bit integer type */
typedef unsigned char   MMP_UINT8;  /**< 8-bit unsigned integer type */
typedef short           MMP_INT16;  /**< 16-bit integer type */
typedef unsigned short  MMP_UINT16; /**< 16-bit unsigned integer type */
typedef int             MMP_INT32;  /**< 32-bit integer type */
typedef unsigned int    MMP_UINT32; /**< 32-bit unsigned integer type */
typedef int             MMP_INT;    /**< Integer type */
typedef unsigned int    MMP_UINT;   /**< Unsigned integer type */
typedef unsigned short  MMP_WINT;   /**< 16-bit UNICODE integer type */
typedef long            MMP_LONG;   /**< Long integer type */
typedef unsigned long   MMP_ULONG;  /**< Unsigned long integer type */
typedef float           MMP_FLOAT;  /**< Floating-point type */
typedef char            MMP_CHAR;   /**< 8-bit Ansi character type */
typedef unsigned short  MMP_WCHAR;  /**< 16-bit UNICODE character type */

typedef char            int8;   /**< 8-bit integer type */
typedef unsigned char   uint8;  /**< 8-bit unsigned integer type */
typedef short           int16;  /**< 16-bit integer type */
typedef unsigned short  uint16; /**< 16-bit unsigned integer type */
typedef int             int32;  /**< 32-bit integer type */
typedef unsigned int    uint32; /**< 32-bit unsigned integer type */
typedef unsigned int    uint;   /**< Unsigned integer type */

#define MMP_FALSE       0
#define MMP_TRUE        1

/** NULL definition */
#define MMP_NULL        0

/** Size definition */
typedef unsigned int MMP_SIZE_T;

#endif /* MMP_DEF_H */
