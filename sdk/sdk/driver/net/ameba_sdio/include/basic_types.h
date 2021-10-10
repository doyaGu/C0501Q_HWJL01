#ifndef __BASIC_TYPES_H__
#define __BASIC_TYPES_H__
#include <autoconf.h>

/* Define compilor specific symbol */
//
// inline function
//
#if defined ( __ICCARM__ )
#define __inline__                      inline
#define __inline                        inline
#define __inline_definition			//In dialect C99, inline means that a function's definition is provided 
								//only for inlining, and that there is another definition 
								//(without inline) somewhere else in the program. 
								//That means that this program is incomplete, because if 
								//add isn't inlined (for example, when compiling without optimization), 
								//then main will have an unresolved reference to that other definition.

								// Do not inline function is the function body is defined .c file and this 
								// function will be called somewhere else, otherwise there is compile error
#elif defined ( __CC_ARM   )
#define __inline__			__inline	//__linine__ is not supported in keil compilor, use __inline instead
#define inline				__inline
#define __inline_definition			// for dialect C99
#elif defined   (  __GNUC__  )

#endif

//
// pack
//

#if defined (__ICCARM__)

#define RTW_PACK_STRUCT_BEGIN
#define RTW_PACK_STRUCT_STRUCT 
#define RTW_PACK_STRUCT_END
#define RTW_PACK_STRUCT_USE_INCLUDES

#elif defined (__CC_ARM)

#define RTW_PACK_STRUCT_BEGIN __packed
#define RTW_PACK_STRUCT_STRUCT 
#define RTW_PACK_STRUCT_END

#elif defined (__GNUC__)

#define RTW_PACK_STRUCT_BEGIN
#define RTW_PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define RTW_PACK_STRUCT_END

#elif defined(PLATFORM_WINDOWS) || defined(WIN32) // Irene Lin
//#elif defined(PLATFORM_WINDOWS)

#define RTW_PACK_STRUCT_BEGIN
#define RTW_PACK_STRUCT_STRUCT 
#define RTW_PACK_STRUCT_END
#define RTW_PACK_STRUCT_USE_INCLUDES
#endif

#ifndef _func_enter_
#define _func_enter_	do{}while(0)
#endif
#ifndef _func_exit_
#define _func_exit_	do{}while(0)
#endif
#define SUCCESS	0
#define FAIL	-1

#ifndef TRUE
	#define _TRUE	1
#else
	#define _TRUE	TRUE	
#endif
		
#ifndef FALSE		
	#define _FALSE	0
#else
	#define _FALSE	FALSE	
#endif

#ifdef PLATFORM_WINDOWS

	typedef signed char s8;
	typedef unsigned char u8;

	typedef signed short s16;
	typedef unsigned short u16;

	typedef signed long s32;
	typedef unsigned long u32;
	
	typedef unsigned int	uint;
	typedef	signed int		sint;


	typedef signed long long s64;
	typedef unsigned long long u64;

	#ifdef NDIS50_MINIPORT
	
		#define NDIS_MAJOR_VERSION       5
		#define NDIS_MINOR_VERSION       0

	#endif

	#ifdef NDIS51_MINIPORT

		#define NDIS_MAJOR_VERSION       5
		#define NDIS_MINOR_VERSION       1

	#endif

	typedef NDIS_PROC proc_t;

	typedef LONG atomic_t;

#endif
#ifdef PLATFORM_FREERTOS
	#include <platform_stdlib.h>
#endif
#if 0//def PLATFORM_LINUX  // Irene Lin

	#include <linux/types.h>   // Irene Lin
	#define IN
	#define OUT
	#define VOID void
	#define NDIS_OID uint
	#define NDIS_STATUS uint

	typedef	signed int sint;

	#ifndef	PVOID
	typedef void * PVOID;
	//#define PVOID	(void *)
	#endif

        #define UCHAR u8
	#define USHORT u16
	#define UINT u32
	#define ULONG u32	

	typedef void (*proc_t)(void*);

	typedef 	__kernel_size_t	SIZE_T;	
	typedef	__kernel_ssize_t	SSIZE_T;
	#define FIELD_OFFSET(s,field)	((SSIZE_T)&((s*)(0))->field)
	
#endif

#ifdef PLATFORM_FREEBSD

	typedef signed char s8;
	typedef unsigned char u8;

	typedef signed short s16;
	typedef unsigned short u16;

	typedef signed int s32;
	typedef unsigned int u32;
	
	typedef unsigned int	uint;
	typedef	signed int		sint;
	typedef long atomic_t;

	typedef signed long long s64;
	typedef unsigned long long u64;
	#define IN
	#define OUT
	#define VOID void
	#define NDIS_OID uint
	#define NDIS_STATUS uint
	
	#ifndef	PVOID
	typedef void * PVOID;
	//#define PVOID	(void *)
	#endif
	typedef u32 dma_addr_t;
    #define UCHAR u8
	#define USHORT u16
	#define UINT u32
	#define ULONG u32	

	typedef void (*proc_t)(void*);
  
  typedef unsigned int __kernel_size_t;
  typedef int __kernel_ssize_t;
  
	typedef 	__kernel_size_t	SIZE_T;	
	typedef	__kernel_ssize_t	SSIZE_T;
	#define FIELD_OFFSET(s,field)	((SSIZE_T)&((s*)(0))->field)
	
#endif

#if 1//def PLATFORM_FREERTOS   // Irene Lin
typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef signed char			s8;
typedef signed short		s16;
typedef signed int			s32;
typedef signed long long 	s64;
typedef unsigned long long	u64;
typedef unsigned int		uint;
typedef signed int			sint;

#ifndef bool
typedef int				bool;
#define  true				1
#define  false				0
#endif

#define IN
#define OUT
#define VOID void
#define NDIS_OID uint
#define NDIS_STATUS uint
#ifndef	PVOID
typedef void *		PVOID;
#endif

typedef unsigned int		__kernel_size_t;
typedef int			__kernel_ssize_t;
typedef	__kernel_size_t		SIZE_T;	
typedef	__kernel_ssize_t	SSIZE_T;
#endif

#ifdef PLATFORM_ECOS
#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_endian.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/plf_intr.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <sys/param.h>
#include <sys/endian.h>
#include <sys/malloc.h>
#include <string.h>
/* 
 *	Type definition
 */
 #ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef VOID
#define VOID void
#endif
#ifndef	PVOID
typedef void * PVOID;
#endif
#ifndef u8
#define u8  cyg_uint8
#endif
#ifndef u16
#define u16 cyg_uint16
#endif
#ifndef u32
#define	u32 cyg_uint32
#endif
#ifndef u64
#define u64 cyg_uint64
#endif

#ifndef s8
#define s8 cyg_int8
#endif
#ifndef s16
#define s16 cyg_int16
#endif
#ifndef s32
#define	s32 cyg_int32
#endif

#ifndef uint
#define uint cyg_uint32
#endif
#ifndef sint
#define sint cyg_int32
#endif
#define	spinlock_t cyg_int32 //TBD

typedef long unsigned int LUINT;
typedef struct { volatile int counter; } atomic_t;
//typedef unsigned long long u64;
typedef unsigned short __be16;

typedef unsigned long __kernel_size_t;
typedef long __kernel_ssize_t;

typedef __kernel_size_t		SIZE_T;	
typedef __kernel_ssize_t	SSIZE_T;

#define SIZE_PTR SIZE_T
#define SSIZE_PTR	SSIZE_T

#endif

#ifndef _RND
#define _RND(sz, r) ((((sz)+((r)-1))/(r))*(r))
#define RND4(x)	(((x >> 2) + (((x & 3) == 0) ?  0: 1)) << 2)
__inline static u32 _RND4(u32 sz)
{

	u32	val;

	val = ((sz >> 2) + ((sz & 3) ? 1: 0)) << 2;
	
	return val;

}

__inline static u32 _RND8(u32 sz)
{

	u32	val;

	val = ((sz >> 3) + ((sz & 7) ? 1: 0)) << 3;
	
	return val;

}

__inline static u32 _RND128(u32 sz)
{

	u32	val;

	val = ((sz >> 7) + ((sz & 127) ? 1: 0)) << 7;
	
	return val;

}
#endif

#define SIZE_PTR SIZE_T
#define SSIZE_PTR SSIZE_T

typedef unsigned char	BOOLEAN,*PBOOLEAN;
// Get the N-bytes aligment offset from the current length
#define N_BYTE_ALIGMENT(__Value, __Aligment) ((__Aligment == 1) ? (__Value) : (((__Value + __Aligment - 1) / __Aligment) * __Aligment))

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#endif //__BASIC_TYPES_H__


