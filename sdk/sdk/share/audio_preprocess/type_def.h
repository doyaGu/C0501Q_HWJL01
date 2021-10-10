/*
 * type_def.h
 *
 *  Created on: 2015¦~3¤ë23¤é
 *      Author: ych
 */
#ifndef TYPE_DEF_H_
#define TYPE_DEF_H_

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_M_IX86)||defined(__i386__)
#elif defined(__arm__)
#define __ARM_ARCH_5E__
#define ARCHITECTURE 5TE
#elif defined(__OR32__)||defined(__OR1K__)
#if !defined(__OR32__)
#define __OR32__
#else
#define __OR1K__
#endif
#endif

#if !defined(_MSC_VER)&&!defined(__CC_ARM)
#include <limits.h>
#include <stdint.h>

#undef INT_MAX
#define INT_MAX 2147483647
#undef INT_MIN
#define INT_MIN (-INT_MAX-1)
#undef LLONG_MAX
#define LLONG_MAX 9223372036854775807LL
#undef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX-1)

#else
// C89 legacy compatible
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef __int64 int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;
typedef unsigned int uintptr_t;

#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX-1)
#define LLONG_MAX 9223372036854775807LL
#define LLONG_MIN (-LLONG_MAX-1)
#define __restrict
#define llabs(x) ABS(x)

#endif

#define pi 3.1415927
#define max(a, b) (((a) > (b))? (a): (b))
#define min(a, b) (((a) < (b))? (a): (b))
#define ABS(a) (((a) < 0)? -(a): (a))
#if	defined(DEBUG)
extern FILE *dptr;
extern FILE *dptr_i;
extern FILE *dptr_ii;
extern FILE *dptr_iii;
extern FILE *dptr_iv;
#define WRITE(src, size, count, p_file) fwrite((src), (size), (count), (p_file))
#define PRINT(f_,...) printf((f_),__VA_ARGS__)
#else
#define WRITE(src, size, count, p_file) ;
#endif

#if defined(_MSC_VER)
#define ALIGN4_BEGIN __declspec(align(4))
#define ALIGN4_END
#elif defined(__GNUC__)
#define ALIGN4_BEGIN
#define ALIGN4_END __attribute__((aligned(4)))
#elif defined(__CC_ARM)
#define ALIGN4_BEGIN __align(4)
#define ALIGN4_END
#endif

#if defined(false)||defined(true)
#undef false
#undef true
#endif

enum {
	false, true, tri
};

typedef int8_t Word8;
typedef int16_t Word16;
typedef uint16_t UWord16;
typedef int32_t Word32;
typedef uint32_t UWord32;
typedef int64_t Word64;
typedef uint64_t UWord64;
typedef float Float32; // single precision

typedef struct {
	UWord32 a0;
	UWord32 a1;
	UWord32 a2;
	UWord32 a3;
} UWRD128;

typedef struct {
	Word16 real;
	Word16 imag;
} Complex16_t; // 2-byte aligned

typedef struct {
	Word32 real;
	Word32 imag;
} Complex32_t; // 4-byte aligned

typedef struct {
	Float32 real;
	Float32 imag;
} ComplexFloat32; // 4-byte aligned

typedef Complex16_t ComplexInt16;
typedef Complex32_t ComplexInt32;

typedef struct {
	short a0 :2;
	short a1 :1;
	short :0; // next bit-field is aligned with addressable storage unit
	short a2 :2;
	short :0;
	short a3 :1;

} bitfields;

#endif
