#ifndef __UIENC_DEFS_H_AO386GIF_U8EN_YSI4_1CWE_YYEKPF4H08D4__
#define __UIENC_DEFS_H_AO386GIF_U8EN_YSI4_1CWE_YYEKPF4H08D4__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <string.h>


#include "uiEnc_config.h"
//=============================================================================
//				  Constant Definition
//=============================================================================

/**
 *  base type
 */
#if 1
 
#define UIE_INT8      signed char
#define UIE_UINT8     unsigned char
#define UIE_INT16     signed short
#define UIE_UINT16    unsigned short
#define UIE_INT32     signed long
#define UIE_UINT32    unsigned long
#define UIE_INT       signed int
#define UIE_UINT      unsigned int
#define UIE_LONG      signed long 
#define UIE_ULONG     unsigned long
#define UIE_FLOAT     float
#define UIE_CHAR      char


#ifdef _MSC_VER // WIN32
    #define UIE_INT64     signed __int64
    #define UIE_UINT64    unsigned __int64
    #define UIE_INLINE    _inline

    #if defined(DTV_PRODUCT_MODE)
        #define UIE_WCHAR     wchar_t
    #else
        #define UIE_WCHAR     char
    #endif

#else
    #define UIE_INT64     signed long long
    #define UIE_UINT64    unsigned long long
    #define UIE_INLINE    inline
    
    #if defined(DTV_PRODUCT_MODE)
        #define UIE_WCHAR     unsigned int
    #else
        #define UIE_WCHAR     char
    #endif
#endif

#define UIE_NULL          0
typedef enum UIE_BOOL_TAG 
{
    UIE_FALSE = 0,
    UIE_TRUE
} UIE_BOOL;

#if 1
    // for Host read/write case
    #define UIE_REG           UIE_UINT16
#else
    // for some day
    #define UIE_REG           UIE_UINT32
#endif
#else

typedef signed char     UIE_INT8;
typedef unsigned char   UIE_UINT8;
typedef signed short    UIE_INT16;
typedef unsigned short  UIE_UINT16;
typedef signed long     UIE_INT32;
typedef unsigned long   UIE_UINT32;
typedef signed int      UIE_INT;
typedef unsigned int    UIE_UINT;
typedef signed long     UIE_LONG;
typedef unsigned long   UIE_ULONG;
typedef float           UIE_FLOAT;
typedef char            UIE_CHAR;

#ifdef _MSC_VER // WIN32
    typedef signed __int64      UIE_INT64;
    typedef unsigned __int64    UIE_UINT64;
    #define UIE_INLINE          _inline

    #ifdef DTV_PRODUCT_MODE
        typedef wchar_t             UIE_WCHAR;
    #else
        typedef char                UIE_WCHAR;
    #endif
#else
    typedef signed long long    UIE_INT64;
    typedef unsigned long long  UIE_UINT64;
    #define UIE_INLINE          inline
    
    #ifdef DTV_PRODUCT_MODE
        typedef unsigned int        UIE_WCHAR;
    #else
        typedef char                UIE_WCHAR;
    #endif
#endif

#define UIE_NULL          0
typedef enum UIE_BOOL_TAG 
{
    UIE_FALSE = 0,
    UIE_TRUE
} UIE_BOOL;

#if 1
    // for Host read/write case
    typedef UIE_UINT16        UIE_REG;
#else
    // for some day
    typedef UIE_UINT32        UIE_REG;
#endif

#endif

//=============================================================================
//				  Macro Definition
//=============================================================================
#if defined(DTV_PRODUCT_MODE)

    #include "host/host.h"
    #include "pal/pal.h"
    #include "sys/sys.h"
    
    // Host
    #define uie_ReadHwReg(add, data)                HOST_ReadRegister(add, data)
    #define uie_WriteHwReg(add, data)               HOST_WriteRegister(add, data)
    #define uie_WriteHwRegMask(add, data, mark)     HOST_WriteRegisterMask(add, data, mark)

    #define uie_EnginClockOn()                      //HOST_uie_EnableClock()
    #define uie_EnginClockOff()                     //HOST_uie_DisableClock()
    #define uie_EnginReset()                        //HOST_uie_Reset()
    
    #define uie_GetVramBaseAddr()                   HOST_GetVramBaseAddress()
    
    #define uie_WriteBlkVram(dst,src,size)          HOST_WriteBlockMemory(dst,src,size)
    #define uie_ReadBlkVram(dst,src,size)           HOST_ReadBlockMemory(dst,src,size)

    // memory
    #define uie_Malloc(size)                        PalHeapAlloc(0, size)
    #define uie_Free(ptr)                           PalHeapFree(0, ptr)

    #define uie_Memset(ptr, val, leng)              PalMemset(ptr, val, leng) 
    #define uie_sleep(ms)                           PalSleep(ms)

    // vram, In win32 case, vram and sysRam is different
    #define uie_VramAllocate(size, id)              MEM_Allocate(size, id)
    #define uie_VramFree(ptr)                       MEM_Release(ptr)

    // Semaphore
    // #define uie_CreateSemaphore(cnt, id)            SYS_CreateSemaphore(cnt, id)
    // #define uie_WaitSemaphore(hSema)                SYS_WaitSemaphore(hSema)
    // #define uie_ReleaseSemaphore(hSema)             SYS_ReleaseSemaphore(hSema)
    // #define uie_DeleteSemaphore(hSema)              SYS_DeleteSemaphore(hSema)

#elif defined(DPF_PRODUCT_MODE)

    #include "ite/itp.h"
  #ifdef _MSC_VER // WIN32
    #include <sys/types.h>
    #include <semaphore.h>
  #else
    #include <sys/semaphore.h>
    #include <unistd.h>
  #endif

    // Host
    #define uie_ReadHwReg(add, data)                (*data = ithReadRegH(add))
    #define uie_WriteHwReg(add, data)               ithWriteRegH(add, data)
    #define uie_WriteHwRegMask(add, data, mark)     ithWriteRegMaskH(add, data, mark)

    #define uie_EnginClockOn()                      ithUiEncEnableClock()
    #define uie_EnginClockOff()                     ithUiEncDisableClock()
    #define uie_EnginReset()                        ithUiEncResetEngine()

    #define uie_GetVramBaseAddr()                   0
    
    static void UIE_INLINE 
    uie_WriteBlkVram(UIE_UINT32 dst, UIE_UINT32 src, UIE_UINT32 size)
    {
        void    *mappingSysRam = ithMapVram(dst, size, ITH_VRAM_WRITE);
        memcpy(mappingSysRam, (void*)src, size);
        ithFlushDCacheRange(mappingSysRam, size);
        ithUnmapVram(mappingSysRam, size);
    }
    
    static void UIE_INLINE 
    uie_ReadBlkVram(UIE_UINT32 dst, UIE_UINT32 src, UIE_UINT32 size)
    {
        void    *mappingSysRam = ithMapVram(src, size, ITH_VRAM_READ);
        memcpy((void*)dst, mappingSysRam, size);
        ithUnmapVram(mappingSysRam, size);
    }
    
    // memory
    #define uie_Malloc(size)                        malloc(size) 
    #define uie_Free(ptr)                           free(ptr)
    
    #define uie_Memset(ptr, val, leng)              memset(ptr, val, leng)
    #define uie_sleep(ms)                           usleep(1000*ms)

    // vram, In win32 case, vram and sysRam is different
    #define uie_VramAllocate(size, id)              itpVmemAlloc(size)    
    #define uie_VramFree(ptr)                       itpVmemFree(ptr)

    #if defined(ISSUE_CODE)
        #error "need to define Semaphore function"
    #endif
    // Semaphore
    #define uie_CreateSemaphore(pHsem, cnt, name)   sem_init(*(pHsem), 0, cnt)
    #define uie_WaitSemaphore(hSema)                sem_wait(hSema)
    #define uie_ReleaseSemaphore(hSema)             sem_post(hSema)
    #define uie_DeleteSemaphore(hSema)              sem_destroy(hSema)
    
#else
    // Host
    #define uie_ReadHwReg(add, data)
    #define uie_WriteHwReg(add, data)
    #define uie_WriteHwRegMask(add, data, mark)

    #define uie_EnginClockOn()
    #define uie_EnginClockOff()
    #define uie_EnginReset()
    
    #define uie_GetVramBaseAddr()                   0
    
    #define uie_WriteBlkVram(dst,src,size)          memcpy((void*)(dst),(void*)(src),size)
    #define uie_ReadBlkVram(dst,src,size)           memcpy((void*)(dst),(void*)(src),size)
    
    // memory
    #define uie_Malloc(size)                        malloc(size) 
    #define uie_Free(ptr)                           free(ptr)
    
    #define uie_Memset(ptr, val, leng)              memset(ptr, val, leng)    
    #define uie_sleep(ms)

    // vram, In win32 case, vram and sysRam is different
    #define uie_VramAllocate(size, id)              malloc(size)       
    #define uie_VramFree(ptr)                       free(ptr)

    // Semaphore
    // #define uie_CreateSemaphore(cnt, id)            uie_NULL
    // #define uie_WaitSemaphore(hSema)                
    // #define uie_ReleaseSemaphore(hSema)             
    // #define uie_DeleteSemaphore(hSema)              
    
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


//=============================================================================
//				  Public Function Definition
//=============================================================================


#ifdef __cplusplus
}
#endif

#endif

