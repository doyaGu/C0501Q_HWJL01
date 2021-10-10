/*
 * Copyright (c) 2007 Ivan Leben
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library in the file COPYING;
 * if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __ITEM2DDEFS_H
#define __ITEM2DDEFS_H

/* Standard headers */

#if defined(WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include "ite/itp.h"

#ifndef __APPLE__
    #include <malloc.h>
#endif

/* Disable VGHandle-pointer conversion warnings since we
   do deal with it by defining VGHandle properly */

#if defined(_MSC_VER)
    #pragma warning(disable:4311)
    #pragma warning(disable:4312)
#endif

/* Type definitions */
#include <inttypes.h>

typedef enum _ITEM2D_VG_OBJ_TYPE
{
    ITEM2D_VG_OBJ_PATH,
    ITEM2D_VG_OBJ_IMAGE,
    ITEM2D_VG_OBJ_PAINT,
    ITEM2D_VG_OBJ_MASK,
    ITEM2D_VG_OBJ_MASKLAYER
} ITEM2D_VG_OBJ_TYPE;

typedef int8_t ITEM2Dint8;
typedef uint8_t ITEM2Duint8;
typedef int16_t ITEM2Dint16;
typedef uint16_t ITEM2Duint16;
typedef int32_t ITEM2Dint32;
typedef uint32_t ITEM2Duint32;
typedef int64_t ITEM2Dint64;
typedef uint64_t ITEM2Duint64;
typedef float ITEM2Dfloat32;

#define ITEM2Dint    ITEM2Dint32
#define ITEM2Duint   ITEM2Duint32
#define ITEM2Dfloat  ITEM2Dfloat32

#define ITEM2Ds12p3  ITEM2Dint16
#define ITEM2Ds7p8   ITEM2Dint16
#define ITEM2Ds8p4   ITEM2Dint16
#define ITEM2Ds15p16 ITEM2Dint32

typedef enum {
    ITEM2D_FALSE = 0,
    ITEM2D_TRUE  = 1,
} ITEM2Dboolean;

/* Maximum / minimum values */

#define ITEM2D_MAX_INT        (0x7fffffff)
#define ITEM2D_MIN_INT        (-0x7fffffff - 1)

#define ITEM2D_MANTISSA_BITS  23
#define ITEM2D_EXPONENT_BITS  8

/* all 1s in exponent yields NaN in IEEE 754 so we take
   1 less then maximum representable with exponent bits */
#define ITEM2D_MAX_EXPONENT   ((1 << ITEM2D_EXPONENT_BITS) - 2)
/* maximum representable with mantissa bits */
#define ITEM2D_MAX_MANTISSA   ((1 << ITEM2D_MANTISSA_BITS) - 1)
/* compose into IEEE754 floating point bit value */
#define ITEM2D_MAX_FLOAT_BITS (ITEM2D_MAX_EXPONENT << ITEM2D_MANTISSA_BITS) | ITEM2D_MAX_MANTISSA

typedef union {
    float        f;
    unsigned int i;
} ITEM2Dfloatint;

ITEM2Dfloat getMaxFloat();

typedef struct
{
    ITEM2Duint8 r, g, b, a;
} ITEM2DColor;

typedef struct
{
    ITEM2Dfloat r, g, b, a;
} ITEM2DFloatColor;

typedef struct
{
    ITEM2Dint16 r, g, b, a;
} ITEM2DHColor;

/*-------------------------------------------------------
 * Color operators
 *-------------------------------------------------------*/

#define CSET(c, rr, gg, bb, aa)  { c.r = rr; c.g = gg; c.b = bb; c.a = aa; }
#define CSETC(c1, c2)            { c1.r = c2.r; c1.g = c2.g; c1.b = c2.b; c1.a = c2.a; }

#define CSUB(c1, rr, gg, bb, aa) { c.r -= rr; c.g -= gg; c.b -= bb; c.a -= aa; }
#define CSUBC(c1, c2)            { c1.r -= c2.r; c1.g -= c2.g; c1.b -= c2.b; c1.a -= c2.a; }
#define CSUBCTO(c1, c2, c3)      { c3.r = c1.r - c2.r; c3.g = c1.g - c2.g;  c3.b = c1.b - c2.b; c3.a = c1.a - c2.a; }

#define CADD(c1, rr, gg, bb, aa) { c.r += rr; c.g += gg; c.b += bb; c.a += aa; }
#define CADDC(c1, c2)            { c1.r += c2.r; c1.g += c2.g; c1.b += c2.b; c1.a += c2.a; }
#define CADDTO(c1, c2, c3)       { c3.r = c1.r + c2.r; c3.g = c1.g + c2.g;  c3.b = c1.b + c2.b; c3.a = c1.a + c2.a; }
#define CADDCK(c1, c2, k)        { c1.r += k * c2.r; c1.g += k * c2.g; c1.b += k * c2.b; c1.a += k * c2.a; }

#define CMUL(c, s)               { c.r = (ITEM2Duint8)(c.r * s); c.g = (ITEM2Duint8)(c.g * s); c.b = (ITEM2Duint8)(c.b * s); c.a = (ITEM2Duint8)(c.a * s); }
//#define CDIV(c, s) { c.r/=s; c.g/=s; c.b/=s; c.a/=s; }

//#define CPREMUL(c) { c.r*=c.a; c.g*=c.a; c.b*=c.a; }
//#define CUNPREMUL(c) { c.r/=c.a; c.g/=c.a; c.b/=c.a; }

/*--------------------------------------------------------
 * Macros for typical vector operations. The only way to
 * inline in C is to actually write a macro
 *--------------------------------------------------------- */

#define SET2(v, xs, ys)         { v.x = xs; v.y = ys; }
#define SET3(v, xs, ys, zs)     { v.x = xs; v.y = ys; v.z = zs; }
#define SET4(v, xs, ys, zs, ws) { v.x = xs; v.y = ys; v.z = zs; v.w = ws; }

#define SET2V(v1, v2)           { v1.x = v2.x; v1.y = v2.y; }
#define SET3V(v1, v2)           { v1.x = v2.x; v1.y = v2.y; v1.z = v2.z; }
#define SET4V(v1, v2)           { v1.x = v2.x; v1.y = v2.y; v1.z = v2.z; v1.w = v2.w; }

#define SET2VP(v1, v2)          { v1->x = v2.x; v1->y = v2.y; }
#define SET3VP(v1, v2)          { v1->x = v2.x; v1->y = v2.y; v1->z = v2.z; }
#define SET4VP(v1, v2)          { v1->x = v2.x; v1->y = v2.y; v1->z = v2.z; v1->sw = v2.w; }

#define EQ2(v, xx, yy)          ( v.x == xx && v.y == yy )
#define EQ3(v, xx, yy, zz)      ( v.x == xx && v.y == yy && v.z == zz )
#define EQ4(v, xx, yy, zz, ww)  ( v.x == xx && v.y == yy && v.z == zz && v.w == ww )

#define ISZERO2(v)              ( v.x == 0.0f && v.y == 0.0f )
#define ISZERO3(v)              ( v.x == 0.0f && v.y == 0.0f && v.z == 0.0f)
#define ISZERO4(v)              ( v.x == 0.0f && v.y == 0.0f && v.z == 0.0f && v.w == 0.0f )

#define EQ2V(v1, v2)            ( v1.x == v2.x && v1.y == v2.y )
#define EQ3V(v1, v2)            ( v1.x == v2.x && v1.y == v2.y && v1.z == v2.z )
#define EQ4V(v1, v2)            ( v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w )

#define ADD2(v, xx, yy)         { v.x += xx; v.y += yy; }
#define ADD3(v, xx, yy, zz)     { v.x += xx; v.y += yy; v.z += zz; }
#define ADD4(v, xx, yy, zz, ww) { v.x += xx; v.y += yy; v.z += zz; v.w += ww; }

#define ADD2V(v1, v2)           { v1.x += v2.x; v1.y += v2.y; }
#define ADD3V(v1, v2)           { v1.x += v2.x; v1.y += v2.y; v1.z += v2.z; }
#define ADD4V(v1, v2)           { v1.x += v2.x; v1.y += v2.y; v1.z += v2.z; v1.w += v2.w; }

#define SUB2(v, xx, yy)         { v.x -= xx; v.y -= yy; }
#define SUB3(v, xx, yy, zz)     { v.x -= xx; v.y -= yy; v.z -= zz; }
#define SUB4(v, xx, yy, zz, ww) { v.x -= xx; v.y -= yy; v.z -= zz; v.w -= v2.w; }

#define SUB2V(v1, v2)           { v1.x -= v2.x; v1.y -= v2.y; }
#define SUB3V(v1, v2)           { v1.x -= v2.x; v1.y -= v2.y; v1.z -= v2.z; }
#define SUB4V(v1, v2)           { v1.x -= v2.x; v1.y -= v2.y; v1.z -= v2.z; v1.w -= v2.w; }

#define MUL2(v, f)              { v.x *= f; v.y *= f; }
#define MUL3(v, f)              { v.x *= f; v.y *= f; v.z *= f; }
#define MUL4(v, f)              { v.x *= f; v.y *= f; v.z *= f; v.w *= f; }

#define DIV2(v, f)              { v.x /= f; v.y /= f; }
#define DIV3(v, f)              { v.x /= f; v.y /= f; v.z /= f; }
#define DIV4(v, f)              { v.x /= f; v.y /= f; v.z /= f; v.w /= f; }

#define ABS2(v)                 { v.x = ITEM2D_ABS(v.x); v.y = ITEM2D_ABS(v.y); }
#define ABS3(v)                 { v.x = ITEM2D_ABS(v.x); v.y = ITEM2D_ABS(v.y); v.z = ITEM2D_ABS(v.z); }
#define ABS4(v)                 { v.x = ITEM2D_ABS(v.x); v.y = ITEM2D_ABS(v.y); v.z = ITEM2D_ABS(v.z); v.w = ITEM2D_ABS(v.w); }

#define NORMSQ2(v)              (v.x * v.x + v.y * v.y)
#define NORMSQ3(v)              (v.x * v.x + v.y * v.y + v.z * v.z)
#define NORMSQ4(v)              (v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w)

#define NORM2(v)                ITEM2D_SQRT(NORMSQ2(v))
#define NORM3(v)                ITEM2D_SQRT(NORMSQ3(v))
#define NORM4(v)                ITEM2D_SQRT(NORMSQ4(v))

#define NORMALIZE2(v)           { ITEM2Dfloat n = NORM2(v); v.x /= n; v.y /= n; }
#define NORMALIZE3(v)           { ITEM2Dfloat n = NORM3(v); v.x /= n; v.y /= n; v.z /= n; }
#define NORMALIZE4(v)           { ITEM2Dfloat n = NORM4(v); v.x /= n; v.y /= n; v.z /= n; v.w /= w; }

#define DOT2(v1, v2)            (v1.x * v2.x + v1.y * v2.y)
#define DOT3(v1, v2)            (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z)
#define DOT4(v1, v2)            (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w)

#define CROSS2(v1, v2)          (v1.x * v2.y - v2.x * v1.y)

#define ANGLE2(v1, v2)          (ITEM2D_ACOS(DOT2(v1, v2) / (NORM2(v1) * NORM2(v2)) ))
#define ANGLE2N(v1, v2)         (ITEM2D_ACOS(DOT2(v1, v2) ))

#define OFFSET2V(v, o, s)       { v.x += o.x * s; v.y += o.y * s; }
#define OFFSET3V(v, o, s)       { v.x += o.x * s; v.y += o.y * s; v.z += o.z * s; }
#define OFFSET4V(v, o, s)       { v.x += o.x * s; v.y += o.y * s; v.z += o.z * s; v.w += o.w * s; }

#define MIN2V(vmin, v)          { if (vmin.x > v.x) vmin.x = v.x; if (vmin.y > v.y) vmin.y = v.y; }
#define MAX2V(vmax, v)          { if (vmax.x < v.x) vmax.x = v.x; if (vmax.y < v.y) vmax.y = v.y; }

/* Portable function definitions */

#define ITEM2D_SQRT      (float)sqrt
#define ITEM2D_COS       (float)cos
#define ITEM2D_SIN       (float)sin
#define ITEM2D_ACOS      (float)acos
#define ITEM2D_ASIN      (float)asin
#define ITEM2D_ATAN      (float)atan
#define ITEM2D_FLOOR     (float)floor
#define ITEM2D_CEIL      (float)ceil
#define ITEM2D_ASSERT    assert

#if defined(__isnan) || (defined(__APPLE__) && (__GNUC__ == 3))
    #define ITEM2D_ISNAN __isnan
#elif defined(_isnan) || defined(WIN32)
    #define ITEM2D_ISNAN _isnan
#else
    #define ITEM2D_ISNAN isnan
#endif

/* Helper macros */

#define PI               3.141592654f
#define ITEM2D_DEG2RAD(a)           (a * PI / 180.0f)
#define ITEM2D_RAD2DEG(a)           (a * 180.0f / PI)
#define ITEM2D_ABS(a)               ((a < 0.0f) ? -a : a)
#define ITEM2D_MAX(a, b)            ((a > b) ? a : b)
#define ITEM2D_MIN(a, b)            ((a < b) ? a : b)
#define ITEM2D_NEARZERO(a)          (a >= -0.0001 && a < 0.0001)
#define ITEM2D_SWAP(a, b)           {ITEfloat t = a; a = b; b = t; }
#define ITEM2D_CLAMP(a, min, max)   {if (a < min) a = min; if (a > max) a = max; }

#define ITEM2D_NEWOBJ(type, obj)    { obj = (type *)malloc(sizeof(type)); if (obj) type ## _ctor(obj); }
#define ITEM2D_INITOBJ(type, obj)   { type ## _ctor(&obj); }
#define ITEM2D_DEINITOBJ(type, obj) { type ## _dtor(&obj); }
#define ITEM2D_DELETEOBJ(type, obj) { if (obj) type ## _dtor(obj); free(obj); }

/* Implementation limits */

#define ITEM2D_MAX_SCISSOR_RECTS          32
#define ITEM2D_MAX_DASH_COUNT             16
#define ITEM2D_MAX_IMAGE_WIDTH            0x1000
#define ITEM2D_MAX_IMAGE_HEIGHT           0x1000
#define ITEM2D_MAX_IMAGE_PIXELS           0x1000000    // Awin@20100118
#define ITEM2D_MAX_IMAGE_BYTES            0x4000000    //0x40000000
#define ITEM2D_MAX_COLOR_RAMP_STOPS       256
#define ITEM2D_MAX_COLOR_RAMP_PIXELS      1024
#define ITEM2D_MAX_STROKE_DIVIDE_NUMBER   128 // Awin@20110307

/* Constant definition */
#define ITEM2D_PATH_CMD_INIT_SIZE         256

/* Convolution */
#define ITEM2D_MAX_KERNEL_SIZE            7
#define ITEM2D_MAX_SEPARABLE_KERNEL_SIZE  15
#define ITEM2D_MAX_GAUSSIAN_STD_DEVIATION 16.0f

#define ITEM2D_MAX_VERTICES               999999999
#define ITEM2D_MAX_RECURSE_DEPTH          16

#define ITEM2D_GRADIENT_TEX_SIZE          1024
#define ITEM2D_GRADIENT_TEX_COORDSIZE     4096 /* 1024 * RGBA */

#endif /* __ITEM2DDEFS_H */