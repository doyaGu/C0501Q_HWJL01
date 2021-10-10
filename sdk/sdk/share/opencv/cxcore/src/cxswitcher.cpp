/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/


/****************************************************************************************/
/*                         Dynamic detection and loading of IPP modules                 */
/****************************************************************************************/

#include "_cxcore.h"

#if defined _MSC_VER && _MSC_VER >= 1200
#pragma warning( disable: 4115 )        /* type definition in () */
#endif

#if defined _MSC_VER && defined WIN64 && !defined EM64T
#pragma optimize( "", off )
#endif

#if defined WIN32 || defined WIN64
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define CV_PROC_GENERIC             0
#define CV_PROC_SHIFT               10
#define CV_PROC_ARCH_MASK           ((1 << CV_PROC_SHIFT) - 1)
#define CV_PROC_IA32_GENERIC        1
#define CV_PROC_IA32_WITH_MMX       (CV_PROC_IA32_GENERIC|(2 << CV_PROC_SHIFT))
#define CV_PROC_IA32_WITH_SSE       (CV_PROC_IA32_GENERIC|(3 << CV_PROC_SHIFT))
#define CV_PROC_IA32_WITH_SSE2      (CV_PROC_IA32_GENERIC|(4 << CV_PROC_SHIFT))
#define CV_PROC_IA64                2
#define CV_PROC_EM64T               3
#define CV_GET_PROC_ARCH(model)     ((model) & CV_PROC_ARCH_MASK)

typedef struct CvProcessorInfo
{
    int model;
    int count;
    double frequency; // clocks per microsecond
}
CvProcessorInfo;

#undef MASM_INLINE_ASSEMBLY

#if defined WIN32 && !defined  WIN64

#if defined _MSC_VER
#define MASM_INLINE_ASSEMBLY 1
#elif defined __BORLANDC__

#if __BORLANDC__ >= 0x560
#define MASM_INLINE_ASSEMBLY 1
#endif

#endif

#endif

/*
   determine processor type
*/
static void
icvInitProcessorInfo( CvProcessorInfo* cpu_info )
{
    memset( cpu_info, 0, sizeof(*cpu_info) );
    cpu_info->model = CV_PROC_GENERIC;

#if defined WIN32 || defined WIN64

#ifndef PROCESSOR_ARCHITECTURE_AMD64
#define PROCESSOR_ARCHITECTURE_AMD64 9
#endif

#ifndef PROCESSOR_ARCHITECTURE_IA32_ON_WIN64
#define PROCESSOR_ARCHITECTURE_IA32_ON_WIN64 10
#endif

    SYSTEM_INFO sys;
    LARGE_INTEGER freq;

    GetSystemInfo( &sys );

    if( sys.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL &&
        sys.dwProcessorType == PROCESSOR_INTEL_PENTIUM && sys.wProcessorLevel >= 6 )
    {
        int version = 0, features = 0, family = 0;
        int id = 0;
        HKEY key = 0;

        cpu_info->count = (int)sys.dwNumberOfProcessors;
        unsigned long val = 0, sz = sizeof(val);

        if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\SYSTEM\\CentralProcessor\\0\\",
            0, KEY_QUERY_VALUE, &key ) >= 0 )
        {
            if( RegQueryValueEx( key, "~MHz", 0, 0, (uchar*)&val, &sz ) >= 0 )
                cpu_info->frequency = (double)val;
            RegCloseKey( key );
        }

#ifdef MASM_INLINE_ASSEMBLY
        __asm
        {
            /* use CPUID to determine the features supported */
            pushfd
            mov   eax, 1
            push  ebx
            push  esi
            push  edi
#ifdef __BORLANDC__
            db 0fh
            db 0a2h
#else
            _emit 0x0f
            _emit 0xa2
#endif
            pop   edi
            pop   esi
            pop   ebx
            mov   version, eax
            mov   features, edx
            popfd
        }
#elif defined WIN32 && __GNUC__ > 2
        asm volatile
        (
            "movl $1,%%eax\n\t"
            ".byte 0x0f; .byte 0xa2\n\t"
            "movl %%eax, %0\n\t"
            "movl %%edx, %1\n\t"
            : "=r"(version), "=r" (features)
            :
            : "%ebx", "%esi", "%edi"
        );
#else
        {
            static const char cpuid_code[] =
                "\x53\x56\x57\xb8\x01\x00\x00\x00\x0f\xa2\x5f\x5e\x5b\xc3";
            typedef int64 (CV_CDECL * func_ptr)(void);
            func_ptr cpuid = (func_ptr)(void*)cpuid_code;
            int64 cpuid_val = cpuid();
            version = (int)cpuid_val;
            features = (int)(cpuid_val >> 32);
        }
#endif

        #define ICV_CPUID_M6     ((1<<15)|(1<<23))  /* cmov + MMX */
        #define ICV_CPUID_A6     ((1<<25)|ICV_CPUID_M6) /* <all above> + SSE */
        #define ICV_CPUID_W7     ((1<<26)|ICV_CPUID_A6) /* <all above> + SSE2 */

        family = (version >> 8) & 15;
        if( family >= 6 && (features & ICV_CPUID_M6) != 0 ) /* Pentium II or higher */
            id = features & ICV_CPUID_W7;

        cpu_info->model = id == ICV_CPUID_W7 ? CV_PROC_IA32_WITH_SSE2 :
                          id == ICV_CPUID_A6 ? CV_PROC_IA32_WITH_SSE :
                          id == ICV_CPUID_M6 ? CV_PROC_IA32_WITH_MMX :
                          CV_PROC_IA32_GENERIC;
    }
    else
    {
#if defined EM64T
        if( sys.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 )
            cpu_info->model = CV_PROC_EM64T;
#elif defined WIN64
        if( sys.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
            cpu_info->model = CV_PROC_IA64;
#endif
        if( QueryPerformanceFrequency( &freq ) )
            cpu_info->frequency = (double)freq.QuadPart;
    }
#else
    cpu_info->frequency = 1;

#ifdef __x86_64__
    cpu_info->model = CV_PROC_EM64T;
#elif defined __ia64__
    cpu_info->model = CV_PROC_IA64;
#elif !defined __i386__
    cpu_info->model = CV_PROC_GENERIC;
#else
    // reading /proc/cpuinfo file (proc file system must be supported)
    FILE *file = fopen( "/proc/cpuinfo", "r" );

    if( file )
    {
        char buffer[1024];
        int max_size = sizeof(buffer)-1;

        for(;;)
        {
            const char* ptr = fgets( buffer, max_size, file );
            if( !ptr )
                break;
            if( strncmp( buffer, "flags", 5 ) == 0 )
            {
                if( strstr( buffer, "mmx" ) && strstr( buffer, "cmov" ))
                {
                    cpu_info->model = CV_PROC_IA32_WITH_MMX;
                    if( strstr( buffer, "xmm" ) || strstr( buffer, "sse" ))
                    {
                        cpu_info->model = CV_PROC_IA32_WITH_SSE;
                        if( strstr( buffer, "emm" ))
                            cpu_info->model = CV_PROC_IA32_WITH_SSE2;
                    }
                }
            }
            else if( strncmp( buffer, "cpu MHz", 7 ) == 0 )
            {
                char* pos = strchr( buffer, ':' );
                if( pos )
                    cpu_info->frequency = strtod( pos + 1, &pos );
            }
        }

        fclose( file );
        if( CV_GET_PROC_ARCH(cpu_info->model) != CV_PROC_IA32_GENERIC )
            cpu_info->frequency = 1;
        else
            assert( cpu_info->frequency > 1 );
    }
#endif
#endif
}


CV_INLINE const CvProcessorInfo*
icvGetProcessorInfo()
{
    static CvProcessorInfo cpu_info;
    static int init_cpu_info = 0;
    if( !init_cpu_info )
    {
        icvInitProcessorInfo( &cpu_info );
        init_cpu_info = 1;
    }
    return &cpu_info;
}


/****************************************************************************************/
/*                               Make functions descriptions                            */
/****************************************************************************************/

/*
   determine processor type, load appropriate dll and
   initialize all function pointers
*/
#if defined WIN32 || defined WIN64
#define DLL_PREFIX ""
#define DLL_SUFFIX ".dll"
#endif

#if 0 /*def _DEBUG*/
#define DLL_DEBUG_FLAG "d"
#else
#define DLL_DEBUG_FLAG ""
#endif

#define VERBOSE_LOADING 0

#if VERBOSE_LOADING
#define ICV_PRINTF(args)  printf args; fflush(stdout)
#else
#define ICV_PRINTF(args)
#endif

CV_IMPL int
cvRegisterModule( const CvModuleInfo* module )
{
    return -1;
}


CV_IMPL int
cvUseOptimized( int load_flag )
{
    return 0;
}

CV_IMPL void
cvGetModuleInfo( const char* name, const char **version, const char **plugin_list )
{
}


typedef int64 (CV_CDECL * rdtsc_func)(void);

/* helper functions for RNG initialization and accurate time measurement */
CV_IMPL  int64  cvGetTickCount( void )
{
    const CvProcessorInfo* cpu_info = icvGetProcessorInfo();

    if( CV_GET_PROC_ARCH(cpu_info->model) == CV_PROC_IA32_GENERIC )
    {
#ifdef MASM_INLINE_ASSEMBLY
    #ifdef __BORLANDC__
        __asm db 0fh
        __asm db 31h
    #else
        __asm _emit 0x0f;
        __asm _emit 0x31;
    #endif
#elif (defined __GNUC__ || defined CV_ICC) && defined __i386__
        int64 t;
        asm volatile (".byte 0xf; .byte 0x31" /* "rdtsc" */ : "=A" (t));
        return t;
#else
        static const char code[] = "\x0f\x31\xc3";
        rdtsc_func func = (rdtsc_func)(void*)code;
        return func();
#endif
    }
    else
    {
#if defined WIN32 || defined WIN64
        LARGE_INTEGER counter;
        QueryPerformanceCounter( &counter );
        return (int64)counter.QuadPart;
#else
        struct timeval tv;
        struct timezone tz;
        gettimeofday( &tv, &tz );
        return (int64)tv.tv_sec*1000000 + tv.tv_usec;
#endif
    }
}

CV_IMPL  double  cvGetTickFrequency()
{
    return icvGetProcessorInfo()->frequency;
}


static int icvNumThreads = 0;
static int icvNumProcs = 0;

CV_IMPL int cvGetNumThreads(void)
{
    if( !icvNumProcs )
        cvSetNumThreads(0);
    return icvNumThreads;
}

CV_IMPL void cvSetNumThreads( int threads )
{
    if( !icvNumProcs )
    {
#ifdef _OPENMP
        icvNumProcs = omp_get_num_procs();
        icvNumProcs = MIN( icvNumProcs, CV_MAX_THREADS );
#else
        icvNumProcs = 1;
#endif
    }

    if( threads <= 0 )
        threads = icvNumProcs;
    else
        threads = MIN( threads, icvNumProcs );

    icvNumThreads = threads;
}


CV_IMPL int cvGetThreadNum(void)
{
#ifdef _OPENMP
    return omp_get_thread_num();
#else
    return 0;
#endif
}


/* End of file. */
