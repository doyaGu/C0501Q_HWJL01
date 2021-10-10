#ifndef __ISP_DEFS_H_DKDANK30_KXF1_ECCM_VN7U_JWG0VDM8TG08__
#define __ISP_DEFS_H_DKDANK30_KXF1_ECCM_VN7U_JWG0VDM8TG08__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>

#include "isp_config.h"
#include "mmp_isp.h"

//=============================================================================
//				  Constant Definition
//=============================================================================

//=============================================================================
//				  Macro Definition
//=============================================================================
#if defined(DTV_PRODUCT_MODE)

    #include "host/host.h"
    #include "pal/pal.h"
    #include "sys/sys.h"
    #include "lcd/lcd_controller.h"

    // Host
    #define isp_ReadHwReg(add, data)              HOST_ReadRegister(add, data)
    #define isp_WriteHwReg(add, data)             HOST_WriteRegister(add, data)
    #define isp_WriteHwRegMask(add, data, mark)   HOST_WriteRegisterMask(add, data, mark)

    #define isp_EnginClockOn()                    HOST_ISP_EnableClock()
    #define isp_EnginClockOff()                   HOST_ISP_DisableClock()
    #define isp_EnginReset()                      HOST_ISP_Reset()

    #define isp_GetVramBaseAddr()                 HOST_GetVramBaseAddress()

    #define isp_WriteBlkVram(dst, src, size)      HOST_WriteBlockMemory(dst, src, size)
    #define isp_ReadBlkVram(dst, src, size)       HOST_ReadBlockMemory(dst, src, size)

    // memory
    #define isp_Malloc(size)                      PalHeapAlloc(0, size)
    #define isp_Free(ptr)                         PalHeapFree(0, ptr)

    #define isp_Memset(ptr, val, leng)            PalMemset(ptr, val, leng)
    #define isp_sleep(ms)                         PalSleep(ms)

    // vram, In win32 case, vram and sysRam is different
    #define isp_VramAllocate(size, id)            MEM_Allocate(size, id)
    #define isp_VramFree(ptr)                     MEM_Release(ptr)

    // Semaphore
    #define isp_CreateSemaphore(pHsem, cnt, name) (*(pHsem) = SYS_CreateSemaphore(cnt, name))
    #define isp_WaitSemaphore(hSema)              SYS_WaitSemaphore(hSema)
    #define isp_ReleaseSemaphore(hSema)           SYS_ReleaseSemaphore(hSema)
    #define isp_DeleteSemaphore(hSema)            SYS_DeleteSemaphore(hSema)

    // Lcd api
    #define isp_LcdEnableVideoFlip(bOn)           LCD_EnableVideoFlip(bOn)
    #define isp_LcdWaitChangeMode()               LCD_WaitChangeMode(LCD_MEMORY_MODE)
    #define isp_LcdGetPitch()                     LCD_GetProperty(LCD_PROPERTY_PITCH)
    #define isp_LcdGetWidth()                     LCD_GetProperty(LCD_PROPERTY_WIDTH)
    #define isp_LcdGetBaseAddr_A()                LCD_GetProperty(LCD_PROPERTY_BASE_A)
    #define isp_LcdGetBaseAddr_B()                LCD_GetProperty(LCD_PROPERTY_BASE_B)
    //#define isp_LcdGetBaseAddr_C()                LCD_GetProperty(LCD_PROPERTY_BASE_C)

#elif defined(DPF_PRODUCT_MODE)

    #include "ite/itp.h"
    #include <unistd.h>
    #ifdef _MSC_VER // WIN32
        #include <sys/types.h>
        #include <semaphore.h>
    #else
        #include <sys/semaphore.h>
    #endif

    // Host
    #define isp_ReadHwReg(add, data)            (*data = ithReadRegH(add))
    #define isp_WriteHwReg(add, data)           ithWriteRegH(add, data)
    #define isp_WriteHwRegMask(add, data, mark) ithWriteRegMaskH(add, data, mark)

    #if defined(ISSUE_CODE)
        #error "need to define engine clock enable function"
    #endif
    #define isp_EnginClockOn()                  ithIspEnableClock()
    #define isp_EnginClockOff()                 ithIspDisableClock()
    #define isp_EnginReset()                    {ithIspResetEngine(); ithIspResetReg(); }

    #define isp_GetVramBaseAddr()               0

static void MMP_INLINE
isp_WriteBlkVram(MMP_UINT32 dst, MMP_UINT32 src, MMP_UINT32 size)
{
    void *mappingSysRam = ithMapVram(dst, size, ITH_VRAM_WRITE);
    memcpy(mappingSysRam, (void *)src, size);
    ithFlushDCacheRange(mappingSysRam, size);
    ithUnmapVram(mappingSysRam, size);
}

static void MMP_INLINE
isp_ReadBlkVram(MMP_UINT32 dst, MMP_UINT32 src, MMP_UINT32 size)
{
    void *mappingSysRam = ithMapVram(src, size, ITH_VRAM_READ);
    memcpy((void *)dst, mappingSysRam, size);
    ithUnmapVram(mappingSysRam, size);
}

    // memory
    #define isp_Malloc(size)           malloc(size)
    #define isp_Free(ptr)              free(ptr)

    #define isp_Memset(ptr, val, leng) memset(ptr, val, leng)
    #define isp_sleep(ms)              usleep(1000 * ms)

    #if defined(ISSUE_CODE)
        #error "need to define Vram malloc function"
    #endif
    // vram, In win32 case, vram and sysRam is different
    #define isp_VramAllocate(size, id) itpVmemAlloc(size)
    #define isp_VramFree(ptr)          itpVmemFree((MMP_UINT32)ptr)

    #if defined(ISSUE_CODE)
        #error "need to define Semaphore function"
    #endif
    // Semaphore
    #define isp_CreateSemaphore(pHsem, cnt, name) sem_init(*(pHsem), 0, cnt)
    #define isp_WaitSemaphore(hSema)              sem_wait(hSema)
    #define isp_ReleaseSemaphore(hSema)           sem_post(hSema)
    #define isp_DeleteSemaphore(hSema)            sem_destroy(hSema)

    // Lcd api
    #define isp_LcdEnableVideoFlip(bOn)
    #define isp_LcdWaitChangeMode()               0
    #define isp_LcdGetPitch()                     ithLcdGetPitch()
    #define isp_LcdGetWidth()                     ithLcdGetWidth()
    #define isp_LcdGetBaseAddr_A()                0//ithLcdGetBaseAddrA()
    #define isp_LcdGetBaseAddr_B()                0//ithLcdGetBaseAddrB()
    //#define isp_LcdGetBaseAddr_C()

#else
    // Host
    #define isp_ReadHwReg(add, data)
    #define isp_WriteHwReg(add, data)
    #define isp_WriteHwRegMask(add, data, mark)

    #define isp_EnginClockOn()
    #define isp_EnginClockOff()
    #define isp_EnginReset()

    #define isp_GetVramBaseAddr()                 0

    #define isp_WriteBlkVram(dst, src, size)      memcpy((void *)(dst), (void *)(src), size)
    #define isp_ReadBlkVram(dst, src, size)       memcpy((void *)(dst), (void *)(src), size)

    // memory
    #define isp_Malloc(size)                      malloc(size)
    #define isp_Free(ptr)                         free(ptr)

    #define isp_Memset(ptr, val, leng)            memset(ptr, val, leng)
    #define isp_sleep(ms)

    // vram, In win32 case, vram and sysRam is different
    #define isp_VramAllocate(size, id)            malloc(size)
    #define isp_VramFree(ptr)                     free(ptr)

    // Semaphore
    #define isp_CreateSemaphore(pHsem, cnt, name) MMP_NULL
    #define isp_WaitSemaphore(hSema)
    #define isp_ReleaseSemaphore(hSema)
    #define isp_DeleteSemaphore(hSema)

    // Lcd api
    #define isp_LcdEnableVideoFlip(bOn)
    #define isp_LcdWaitChangeMode() 0
    #define isp_LcdGetPitch()       0
    #define isp_LcdGetBaseAddr_A()  0
    #define isp_LcdGetBaseAddr_B()  0
    //#define isp_LcdGetBaseAddr_C()                    0
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