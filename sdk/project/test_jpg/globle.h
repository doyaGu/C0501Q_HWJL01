#ifndef __GLOBLE_H_JCVPOKCP_NAER_DYO3_VJVJ_5TUYAWKIZTMA__
#define __GLOBLE_H_JCVPOKCP_NAER_DYO3_VJVJ_5TUYAWKIZTMA__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include "ite/itp.h"
#include "fat/fat.h"
#include "ite/ite_sd.h"
#include "ite/ith.h"

//=============================================================================
//				  Constant Definition
//=============================================================================


//=============================================================================
//				  Macro Definition
//=============================================================================
#undef _err
#if (_MSC_VER)
    #define _err(string, ...)               do { printf(string, __VA_ARGS__); \
                                                printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                while(1); \
                                            } while(0)
    #ifndef trace
    #define trace(string, args, ...)        do { printf(string, __VA_ARGS__); \
                                                printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                            } while(0)
    #endif
#else
    #define _err(string, args...)           do { printf(string, ## args); \
                                                printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                while(1); \
                                            } while(0)
    #ifndef trace
    #define trace(string, args...)          do { printf(string, ## args); \
                                                printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                            } while(0)
    #endif                                                
#endif
//=============================================================================
//				  Structure Definition
//=============================================================================


//=============================================================================
//				  Global Data Definition
//=============================================================================


//=============================================================================
//				  Private Function Definition
//=============================================================================
static void
_get_clock(
    struct timeval *startT)
{
    gettimeofday(startT, NULL);
}

static uint32_t
_get_duration(
    struct timeval *startT)
{
    struct timeval currT = {0};
    uint32_t  duration_time = 0;

    gettimeofday(&currT, NULL);
    duration_time = (currT.tv_sec - startT->tv_sec) * 1000;      // sec to ms
    duration_time += ((currT.tv_usec - startT->tv_usec) / 1000); // us to ms
    return (uint32_t)duration_time;
}

static int
_Vram_WriteBlkMem(
    unsigned int    dst,
    unsigned int    src,
    unsigned int    byteSize)
{
    ithWriteVram(dst, (const void*)src, byteSize);
    return 0;
}

static int
_Vram_ReadBlkMem(
    unsigned int    dst,
    unsigned int    src,
    unsigned int    byteSize)
{
    ithReadVram((void*)dst, src, byteSize);
    return 0;
}

static unsigned int
_Get_Lcd_Width(void)
{
    return ithLcdGetWidth();
}

static unsigned int
_Get_Lcd_Height(void)
{
    return ithLcdGetHeight();
}

static unsigned long
_Get_Lcd_Pitch(void)
{
    return ithLcdGetPitch();
}

static uint32_t
_Get_Lcd_Addr_A(void)
{
    return ithLcdGetBaseAddrA();
}

static uint32_t
_Get_Lcd_Addr_B(void)
{
    return ithLcdGetBaseAddrB();
}

static uint32_t
_Get_Lcd_Addr_C(void)
{
    return ithLcdGetBaseAddrC();
}

static void
_Sleep(unsigned long ms)
{
    usleep((ms*1000));
}

static void*
_Allocat_vram(
    unsigned int    size)
{
    return (void*)itpVmemAlloc(size); 
}

static void
_Free_vram(
    void    *ptr)
{
    itpVmemFree((uint32_t)ptr);
}
//=============================================================================
//				  Public Function Definition
//=============================================================================


#ifdef __cplusplus
}
#endif

#endif

