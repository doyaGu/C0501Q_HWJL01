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

#ifndef __ITEDEFS_H
#define __ITEDEFS_H

/* Standard headers */

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#ifdef WIN32
#define VGINLINE _inline
#else
#define VGINLINE inline
#endif

#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include "ite/itp.h"

#ifndef __APPLE__
#  include <malloc.h>
#endif

/* Disable VGHandle-pointer conversion warnings since we
   do deal with it by defining VGHandle properly */

#if defined(_MSC_VER)
#  pragma warning(disable:4311)
#  pragma warning(disable:4312)
#endif

/* Type definitions */

#if defined(HAVE_CONFIG_H)
#include "../config.h"
#
#  if HAVE_INTTYPES_H
#  include <inttypes.h>
#  endif
#
#else
#
#  define int8_t    char
#  define uint8_t   unsigned char
#  define int16_t   short
#  define uint16_t  unsigned short
#  define int32_t   int
#  define uint32_t  unsigned int
#  define int64_t   long long
#  define uint64_t  unsigned long long
#
#endif

typedef enum _ITE_VG_OBJ_TYPE
{
	ITE_VG_OBJ_PATH,
	ITE_VG_OBJ_IMAGE,
	ITE_VG_OBJ_PAINT,
	ITE_VG_OBJ_MASK,
	ITE_VG_OBJ_MASKLAYER
}ITE_VG_OBJ_TYPE;

typedef int8_t      ITEint8;
typedef uint8_t     ITEuint8;
typedef int16_t     ITEint16;
typedef uint16_t    ITEuint16;
typedef int32_t     ITEint32;
typedef uint32_t    ITEuint32;
typedef int64_t     ITEint64;
typedef uint64_t    ITEuint64;
typedef float       ITEfloat32;

#define ITEint		ITEint32
#define ITEuint		ITEuint32
#define ITEfloat	ITEfloat32

#define ITEs12p3	ITEint16
#define ITEs7p8     ITEint16
#define ITEs8p4     ITEint16
#define ITEs15p16	ITEint32

typedef enum {
	ITE_FALSE = 0,
	ITE_TRUE  = 1,
} ITEboolean;

/* Maximum / minimum values */

#define ITE_MAX_INT  (0x7fffffff)
#define ITE_MIN_INT (-0x7fffffff-1)

#define ITE_MANTISSA_BITS   23
#define ITE_EXPONENT_BITS   8

/* all 1s in exponent yields NaN in IEEE 754 so we take
   1 less then maximum representable with exponent bits */
#define ITE_MAX_EXPONENT ((1 << ITE_EXPONENT_BITS) - 2)
/* maximum representable with mantissa bits */
#define ITE_MAX_MANTISSA ((1 << ITE_MANTISSA_BITS) - 1)
/* compose into IEEE754 floating point bit value */
#define ITE_MAX_FLOAT_BITS (ITE_MAX_EXPONENT << ITE_MANTISSA_BITS) | ITE_MAX_MANTISSA

typedef union {
  float f;
  unsigned int i;
} ITEfloatint;

ITEfloat getMaxFloat();

typedef struct
{
	ITEuint8 r, g, b, a;
}ITEColor;

typedef struct
{
	ITEfloat r,g,b,a;
}ITEFloatColor;

typedef struct
{
	ITEint16 r,g,b,a;
}ITEHColor;

/*-------------------------------------------------------
 * Color operators
 *-------------------------------------------------------*/

#define CSET(c, rr,gg,bb,aa) { c.r=rr; c.g=gg; c.b=bb; c.a=aa; }
#define CSETC(c1, c2) { c1.r=c2.r; c1.g=c2.g; c1.b=c2.b; c1.a=c2.a; }

#define CSUB(c1, rr,gg,bb,aa) { c.r-=rr; c.g-=gg; c.b-=bb; c.a-=aa; }
#define CSUBC(c1, c2) { c1.r-=c2.r; c1.g-=c2.g; c1.b-=c2.b; c1.a-=c2.a; }
#define CSUBCTO(c1, c2, c3) { c3.r=c1.r-c2.r; c3.g=c1.g-c2.g;  c3.b=c1.b-c2.b; c3.a=c1.a-c2.a; }

#define CADD(c1, rr,gg,bb,aa) { c.r+=rr; c.g+=gg; c.b+=bb; c.a+=aa; }
#define CADDC(c1, c2) { c1.r+=c2.r; c1.g+=c2.g; c1.b+=c2.b; c1.a+=c2.a; }
#define CADDTO(c1, c2, c3) { c3.r=c1.r+c2.r; c3.g=c1.g+c2.g;  c3.b=c1.b+c2.b; c3.a=c1.a+c2.a; }
#define CADDCK(c1, c2, k) { c1.r+=k*c2.r; c1.g+=k*c2.g; c1.b+=k*c2.b; c1.a+=k*c2.a; }

#define CMUL(c, s) { c.r=(ITEuint8)(c.r*s); c.g=(ITEuint8)(c.g*s); c.b=(ITEuint8)(c.b*s); c.a=(ITEuint8)(c.a*s); }
//#define CDIV(c, s) { c.r/=s; c.g/=s; c.b/=s; c.a/=s; }

//#define CPREMUL(c) { c.r*=c.a; c.g*=c.a; c.b*=c.a; }
//#define CUNPREMUL(c) { c.r/=c.a; c.g/=c.a; c.b/=c.a; }

/*--------------------------------------------------------
 * Macros for typical vector operations. The only way to
 * inline in C is to actually write a macro
 *--------------------------------------------------------- */

#define SET2(v,xs,ys) { v.x=xs; v.y=ys; }
#define SET3(v,xs,ys,zs) { v.x=xs; v.y=ys; v.z=zs; }
#define SET4(v,xs,ys,zs,ws) { v.x=xs; v.y=ys; v.z=zs; v.w=ws; }

#define SET2V(v1,v2) { v1.x=v2.x; v1.y=v2.y; }
#define SET3V(v1,v2) { v1.x=v2.x; v1.y=v2.y; v1.z=v2.z; }
#define SET4V(v1,v2) { v1.x=v2.x; v1.y=v2.y; v1.z=v2.z; v1.w=v2.w; }

#define SET2VP(v1,v2) { v1->x=v2.x; v1->y=v2.y; }
#define SET3VP(v1,v2) { v1->x=v2.x; v1->y=v2.y; v1->z=v2.z; }
#define SET4VP(v1,v2) { v1->x=v2.x; v1->y=v2.y; v1->z=v2.z; v1->sw=v2.w; }

#define EQ2(v,xx,yy)       ( v.x==xx && v.y==yy )
#define EQ3(v,xx,yy,zz)    ( v.x==xx && v.y==yy && v.z==zz )
#define EQ4(v,xx,yy,zz,ww) ( v.x==xx && v.y==yy && v.z==zz && v.w==ww )

#define ISZERO2(v) ( v.x==0.0f && v.y==0.0f )
#define ISZERO3(v) ( v.x==0.0f && v.y==0.0f && v.z==0.0f)
#define ISZERO4(v) ( v.x==0.0f && v.y==0.0f && v.z==0.0f && v.w==0.0f )

#define EQ2V(v1,v2) ( v1.x==v2.x && v1.y==v2.y )
#define EQ3V(v1,v2) ( v1.x==v2.x && v1.y==v2.y && v1.z==v2.z )
#define EQ4V(v1,v2) ( v1.x==v2.x && v1.y==v2.y && v1.z==v2.z && v1.w==v2.w )

#define ADD2(v,xx,yy)       { v.x+=xx; v.y+=yy; }
#define ADD3(v,xx,yy,zz)    { v.x+=xx; v.y+=yy; v.z+=zz; }
#define ADD4(v,xx,yy,zz,ww) { v.x+=xx; v.y+=yy; v.z+=zz; v.w+=ww; }

#define ADD2V(v1,v2) { v1.x+=v2.x; v1.y+=v2.y; }
#define ADD3V(v1,v2) { v1.x+=v2.x; v1.y+=v2.y; v1.z+=v2.z; }
#define ADD4V(v1,v2) { v1.x+=v2.x; v1.y+=v2.y; v1.z+=v2.z; v1.w+=v2.w; }

#define SUB2(v,xx,yy)       { v.x-=xx; v.y-=yy; }
#define SUB3(v,xx,yy,zz)    { v.x-=xx; v.y-=yy; v.z-=zz; }
#define SUB4(v,xx,yy,zz,ww) { v.x-=xx; v.y-=yy; v.z-=zz; v.w-=v2.w; }

#define SUB2V(v1,v2) { v1.x-=v2.x; v1.y-=v2.y; }
#define SUB3V(v1,v2) { v1.x-=v2.x; v1.y-=v2.y; v1.z-=v2.z; }
#define SUB4V(v1,v2) { v1.x-=v2.x; v1.y-=v2.y; v1.z-=v2.z; v1.w-=v2.w; }

#define MUL2(v,f) { v.x*=f; v.y*=f; }
#define MUL3(v,f) { v.x*=f; v.y*=f; v.z*=f; }
#define MUL4(v,f) { v.x*=f; v.y*=f; v.z*=f; v.w*=f; }

#define DIV2(v,f) { v.x/=f; v.y/=f; }
#define DIV3(v,f) { v.x/=f; v.y/=f; v.z/=f; }
#define DIV4(v,f) { v.x/=f; v.y/=f; v.z/=f; v.w/=f; }

#define ABS2(v) { v.x=ITE_ABS(v.x); v.y=ITE_ABS(v.y); }
#define ABS3(v) { v.x=ITE_ABS(v.x); v.y=ITE_ABS(v.y); v.z=ITE_ABS(v.z); }
#define ABS4(v) { v.x=ITE_ABS(v.x); v.y=ITE_ABS(v.y); v.z=ITE_ABS(v.z); v.w=ITE_ABS(v.w); }

#define NORMSQ2(v) (v.x*v.x + v.y*v.y)
#define NORMSQ3(v) (v.x*v.x + v.y*v.y + v.z*v.z)
#define NORMSQ4(v) (v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w)

#define NORM2(v) ITE_SQRT(NORMSQ2(v))
#define NORM3(v) ITE_SQRT(NORMSQ3(v))
#define NORM4(v) ITE_SQRT(NORMSQ4(v))

#define NORMALIZE2(v) { ITEfloat n=NORM2(v); v.x/=n; v.y/=n; }
#define NORMALIZE3(v) { ITEfloat n=NORM3(v); v.x/=n; v.y/=n; v.z/=n; }
#define NORMALIZE4(v) { ITEfloat n=NORM4(v); v.x/=n; v.y/=n; v.z/=n; v.w/=w; }

#define DOT2(v1,v2) (v1.x*v2.x + v1.y*v2.y)
#define DOT3(v1,v2) (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z)
#define DOT4(v1,v2) (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w)

#define CROSS2(v1,v2) (v1.x*v2.y - v2.x*v1.y)

#define ANGLE2(v1,v2) (ITE_ACOS( DOT2(v1,v2) / (NORM2(v1)*NORM2(v2)) ))
#define ANGLE2N(v1,v2) (ITE_ACOS( DOT2(v1,v2) ))

#define OFFSET2V(v, o, s) { v.x += o.x*s; v.y += o.y*s; }
#define OFFSET3V(v, o, s) { v.x += o.x*s; v.y += o.y*s; v.z += o.z*s; }
#define OFFSET4V(v, o, s) { v.x += o.x*s; v.y += o.y*s; v.z += o.z*s; v.w += o.w*s; }

#define MIN2V(vmin,v) { if(vmin.x>v.x) vmin.x=v.x; if(vmin.y>v.y) vmin.y=v.y; }
#define MAX2V(vmax,v) { if(vmax.x<v.x) vmax.x=v.x; if(vmax.y<v.y) vmax.y=v.y; }


/* Portable function definitions */

#define ITE_SQRT   (float)sqrt
#define ITE_COS    (float)cos
#define ITE_SIN    (float)sin
#define ITE_ACOS   (float)acos
#define ITE_ASIN   (float)asin
#define ITE_ATAN   (float)atan
#define ITE_FLOOR  (float)floor
#define ITE_CEIL   (float)ceil
#define ITE_ASSERT assert

#if defined(__isnan) || (defined(__APPLE__) && (__GNUC__ == 3))
#  define ITE_ISNAN __isnan
#elif defined(_isnan) || defined(WIN32)
#  define ITE_ISNAN _isnan
#else
#  define ITE_ISNAN isnan
#endif


/* Helper macros */

#define PI 3.141592654f
#define ITE_DEG2RAD(a) (a * PI / 180.0f)
#define ITE_RAD2DEG(a) (a * 180.0f / PI)
#define ITE_ABS(a) ((a < 0.0f) ? -a : a)
#define ITE_MAX(a,b) ((a > b) ? a : b)
#define ITE_MIN(a,b) ((a < b) ? a : b)
#define ITE_NEARZERO(a) (a >= -0.0001 && a < 0.0001)
#define ITE_SWAP(a,b) {ITEfloat t=a; a=b; b=t;}
#define ITE_CLAMP(a,min,max) {if (a<min) a=min; if (a>max) a=max; }

#define ITE_NEWOBJ(type,obj) { obj = (type*)malloc(sizeof(type)); if(obj) type ## _ctor(obj); }
#define ITE_INITOBJ(type,obj){ type ## _ctor(&obj); }
#define ITE_DEINITOBJ(type,obj) { type ## _dtor(&obj); }
#define ITE_DELETEOBJ(type,obj) { if(obj) type ## _dtor(obj); free(obj); }

/* Implementation limits */

#define ITE_MAX_SCISSOR_RECTS             32
#define ITE_MAX_DASH_COUNT                17
#define ITE_MAX_IMAGE_WIDTH               0x1000
#define ITE_MAX_IMAGE_HEIGHT              0x1000
#define ITE_MAX_IMAGE_PIXELS              0x1000000 // Awin@20100118
#define ITE_MAX_IMAGE_BYTES				  0x4000000 //0x40000000
#define ITE_MAX_COLOR_RAMP_STOPS          256
#define ITE_MAX_COLOR_RAMP_PIXELS         1024
#define ITE_MAX_STROKE_DIVIDE_NUMBER	  8 // Awin@20110307

/* Constant definition */
#define ITE_PATH_CMD_INIT_SIZE            256

/* Convolution */
#define ITE_MAX_KERNEL_SIZE				  7
#define ITE_MAX_SEPARABLE_KERNEL_SIZE     15
#define ITE_MAX_GAUSSIAN_STD_DEVIATION    16.0f

#define ITE_MAX_VERTICES 999999999
#define ITE_MAX_RECURSE_DEPTH 16

#define ITE_GRADIENT_TEX_SIZE       1024
#define ITE_GRADIENT_TEX_COORDSIZE   4096 /* 1024 * RGBA */

/* Memory relative */
#define VG_Malloc		malloc
#define VG_Free			free
#define VG_Realloc		realloc
#define VG_Memset		memset
#define VG_Memcpy		memcpy
#define VG_VMemAlloc	itpVmemAlloc
#define VG_VMemFree		itpVmemFree

#endif /* __ITEDEFS_H */

