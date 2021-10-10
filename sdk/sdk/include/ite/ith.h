/** @file
 * ITE Hardware Library.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2011-2012
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup ith ITE Hardware Library
 *  @{
 */
#ifndef ITE_ITH_H
#define ITE_ITH_H

#if defined(ANDROID)                // Android
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>

    extern uint32_t ithMmioBase;
    #define ITH_REG_BASE (ithMmioBase - ITH_MMIO_BASE)

#elif defined(__LINUX_ARM_ARCH__)    // Linux kernel
    #include <asm/io.h>

    extern uint32_t ithMmioBase;
    #define ITH_REG_BASE (ithMmioBase - ITH_MMIO_BASE)

#elif defined(CONFIG_ARM)            // U-Boot
    #include <stdbool.h>
    #include "compiler.h"

    #define ITH_REG_BASE 0

#elif defined(_WIN32)                // Win32
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>

    #define inline __inline
    #define __func__ __FUNCTION__
    #define __attribute__()

#elif defined(__OPENRTOS__)         // OpenRTOS
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>

    #define ITH_REG_BASE 0

#else
    #include <stdbool.h>
    #include <stddef.h>
    #include <stdint.h>

    #define ITH_REG_BASE 0

#endif // defined(ANDROID)

#include "ith_defs.h"
#include "mock_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup ith_platform Platform
 * Used by this module internal.
 *  @{
 */
/**
 * Called when assert fail.
 *
 * @param exp The evaluated expression.
 * @param file The evaluated file.
 * @param line The evaluated line of file.
 * @param func The evaluated function.
 */
extern void ithAssertFail(const char* exp, const char* file, int line, const char* func);

/**
 * Busy-waiting delay.
 *
 * @param us microseconds.
 */
extern void ithDelay(unsigned long us);

/**
 * Locks mutex.
 *
 * @param mutex The mutex object.
 */
extern void ithLockMutex(void* mutex);

/**
 * Unlocks mutex.
 *
 * @param mutex The mutex object.
 */
extern void ithUnlockMutex(void* mutex);

/**
 * Flushes CPU's data cache.
 */
extern void ithFlushDCache(void);

/**
 * Invalidates CPU's data cache.
 */
extern void ithInvalidateDCache(void);

/**
 * Flushes CPU's data cache in specified range.
 *
 * @param ptr the memory address to flush.
 * @param size the memory size to flush.
 */
extern void ithFlushDCacheRange(void* ptr, uint32_t size);

/**
 * Flushes CPU's data cache in specified range.
 *
 * @param ptr the memory address to flush.
 * @param size the memory size to flush.
 */
extern void ithInvalidateDCacheRange(void* ptr, uint32_t size);

/**
 * Invalidates CPU's instruction cache.
 */
extern void ithInvalidateICache(void);

/**
 * CPU Doze, it's gated CPU clock until interrupt happend.
 */
extern void ithCpuDoze(void);

/**
 * Flushes Memory buffer.
 */
extern void ithFlushMemBuffer(void);

/**
 * Flushes AHB Wrap.
 */
extern void ithFlushAhbWrap(void);

/**
 * Reschedules task after ISR is finished.
 *
 * @param yield task yield if yield is true.
 */
extern void ithYieldFromISR(bool yield);

/**
 * Saves current interrupt states and disable interrupt.
 */
extern void ithEnterCritical(void);

/**
 * Restore previous saved interrupt states.
 */
extern void ithExitCritical(void);

/**
 * Forces a context switch.
 */
extern void ithTaskYield(void);

/**
 * CPU mode definitions.
 */
typedef enum
{
    ITH_CPU_USR = 0x10, ///< USR_MODE
    ITH_CPU_FIQ = 0x11, ///< FIQ_MODE
    ITH_CPU_IRQ = 0x12, ///< IRQ_MODE
    ITH_CPU_SVC = 0x13, ///< SVC_MODE
    ITH_CPU_ABT = 0x17, ///< ABT_MODE
    ITH_CPU_UND = 0x1B, ///< UND_MODE
    ITH_CPU_SYS = 0x1F  ///< SYS_MODE
} ITHCpuMode;

/**
 * Gets CPU mode.
 * @return The CPU mode.
 */
extern ITHCpuMode ithGetCpuMode(void);

/**
 * Initialize this module.
 */
void ithInit(void);

/**
 * Exit this module.
 */
void ithExit(void);

/** @} */ // end of ith_platform

/** @defgroup ith_vram VRAM
 *  @{
 */
#define ITH_VRAM_READ   0x1 ///< Mapping flag for read.
#define ITH_VRAM_WRITE  0x2 ///< Mapping flag for write.

#if defined(ANDROID)|| defined(__LINUX_ARM_ARCH__)|| defined(_WIN32)
    void* ithMapVram(uint32_t vram_addr, uint32_t size, uint32_t flags);
    void ithUnmapVram(void* sys_addr, uint32_t size);
    uint32_t ithSysAddr2VramAddr(void* sys_addr);

#else // U-Boot and OpenRTOS

/**
 * Maps VRAM to an accessible address.
 *
 * @param vram_addr The start VRAM address to map.
 * @param size The mapping size.
 * @param flags The flags. Can be union of ITH_VRAM_READ and ITH_VRAM_WRITE.
 * @return The accessible address. NULL indicates failed.
 * @see ithUnmapVram()
 * @par Example:
 * @code
    uint16_t* base = ithMapVram(lcd_addr, lcd_pitch * lcd_height, ITH_VRAM_WRITE);
    uint16_t* ptr = base;

    // fill black to lcd screen
    for (y = 0; y < lcd_height; y++)
        for (x = 0; x < lcd_width; x++)
            *ptr++ = 0x0;

    ithUnmapVram(base, lcd_pitch * lcd_height);

 * @endcode
 */
    static inline void* ithMapVram(uint32_t vram_addr, uint32_t size, uint32_t flags)
    {
        return (void*) vram_addr;
    }

/**
 * Unmaps a VRAM accessible address.
 *
 * @param sys_addr The VRAM accessible address.
 * @param size The mapping size.
 * @see ithMapVram()
 */
    static inline void ithUnmapVram(void* sys_addr, uint32_t size)
    {
        // DO NOTHING
    }

    static inline uint32_t ithSysAddr2VramAddr(void* sys_addr)
    {
        return (uint32_t)sys_addr;
    }
#endif // defined(ANDROID)|| defined(__LINUX_ARM_ARCH__)|| defined(_WIN32)

/**
 * Reads VRAM data to system memory.
 *
 * @param dest The destination system memory address.
 * @param src The source VRAM address.
 * @param size The size will be read.
 */
void ithReadVram(void* dest, uint32_t src, uint32_t size);

/**
 * Writes system memory data to VRAM.
 *
 * @param dest The destination VRAM address.
 * @param src The source system memory address.
 * @param size The size will be written.
 */
void ithWriteVram(uint32_t dest, const void* src, uint32_t size);

/**
 * Copies memory block between VRAM.
 *
 * @param dest The VRAM address will be copied to.
 * @param src The VRAM address will be copied from.
 * @param size The block size will be copied.
 */
void ithCopyVram(uint32_t dest, uint32_t src, uint32_t size);

/**
 * Sets VRAM to a specified character.
 *
 * @param dest Address of destination.
 * @param c Character to set.
 * @param size The VRAM size to be set.
 */
void ithSetVram(uint32_t dest, int c, uint32_t size);

/** @} */ // end of ith_vram

/** @defgroup ith_reg Register
 *  @{
 */
#if defined(_WIN32)
    uint16_t ithReadRegH(uint16_t addr);
    DECLARE_COULD_BE_MOCKED_FUNC1(uint16_t, ithReadRegH, uint16_t);

    void ithWriteRegH(uint16_t addr, uint16_t data);
    DECLARE_COULD_BE_MOCKED_FUNC2(void, ithWriteRegH, uint16_t, uint16_t);

    uint32_t ithReadRegA(uint32_t addr);
    void ithWriteRegA(uint32_t addr, uint32_t data);

    uint8_t ithReadReg8(uint32_t addr);
    void ithWriteReg8(uint32_t addr, uint8_t data);

    uint16_t ithReadReg16(uint32_t addr);
    void ithWriteReg16(uint32_t addr, uint16_t data);

    #define ITH_READ_REG_H(addr)
    #define ITH_WRITE_REG_H(addr, data)

#elif defined(__LINUX_ARM_ARCH__)
    static inline uint16_t ithReadRegH(uint16_t addr)
    {
        return (uint16_t)readl(ITH_REG_BASE + ITH_HOST_BASE + addr);
    }

    static inline void ithWriteRegH(uint16_t addr, uint16_t data)
    {
        writel(ITH_REG_BASE + ITH_HOST_BASE + addr, (uint32_t)data);
    }

    static inline uint32_t ithReadRegA(uint32_t addr)
    {
        return readl(ITH_REG_BASE + addr);
    }

    static inline void ithWriteRegA(uint32_t addr, uint32_t data)
    {
        writel(ITH_REG_BASE + addr, data);
    }

    #define ITH_READ_REG_H(addr) \
        do { \
            *(uint16_t volatile*)(ITH_REG_BASE + ITH_HOST_BASE + (addr)); \
        } while (0)

    #define ITH_WRITE_REG_H(addr, data) \
        do { \
            *(uint16_t volatile*)(ITH_REG_BASE + ITH_HOST_BASE + (addr)) = (data); \
        } while (0)

#else
    /**
     * Reads host register value.
     *
     * @param addr The register address.
     * @return The register value.
     */
    static inline uint16_t ithReadRegH(uint16_t addr)
    {
        return *(uint16_t volatile*)(ITH_REG_BASE + ITH_HOST_BASE + addr);
    }

    /**
     * Writes host register value.
     *
     * @param addr The register address.
     * @param data The value to write.
     */
    static inline void ithWriteRegH(uint16_t addr, uint16_t data)
    {
        *(uint16_t volatile*)(ITH_REG_BASE + ITH_HOST_BASE + addr) = data;
    }

    /**
     * Reads amba register value.
     *
     * @param addr The register address.
     * @return The register value.
     */
    static inline uint32_t ithReadRegA(uint32_t addr)
    {
        return *(uint32_t volatile*)(ITH_REG_BASE + addr);
    }

    /**
     * Writes amba register value.
     *
     * @param addr The register address.
     * @param data The value to write.
     */
    static inline void ithWriteRegA(uint32_t addr, uint32_t data)
    {
        *(uint32_t volatile*)(ITH_REG_BASE + addr) = data;
    }

    /**
     * Reads amba register value, in byte.
     *
     * @param addr The register address.
     * @return The register value.
     */
    static inline uint8_t ithReadReg8(uint32_t addr)
    {
        return *(uint8_t volatile*)(ITH_REG_BASE + addr);
    }

    /**
     * Writes amba register value, in byte.
     *
     * @param addr The register address.
     * @param data The value to write.
     */
    static inline void ithWriteReg8(uint32_t addr, uint8_t data)
    {
        *(uint8_t volatile*)(ITH_REG_BASE + addr) = data;
    }

    /**
     * Reads amba register value, in 16-bit.
     *
     * @param addr The register address.
     * @return The register value.
     */
    static inline uint16_t ithReadReg16(uint32_t addr)
    {
        return *(uint16_t volatile*)(ITH_REG_BASE + addr);
    }

    /**
     * Writes amba register value, in 16-bit.
     *
     * @param addr The register address.
     * @param data The value to write.
     */
    static inline void ithWriteReg16(uint32_t addr, uint16_t data)
    {
        *(uint16_t volatile*)(ITH_REG_BASE + addr) = data;
    }

    /**
     * Reads host register value macro.
     *
     * @param addr The register address.
     * @return The register value.
     */
    #define ITH_READ_REG_H(addr) \
        *(uint16_t volatile*)(ITH_REG_BASE + ITH_HOST_BASE + (addr))

    /**
     * Writes host register value macro.
     *
     * @param addr The register address.
     * @param data The value to write.
     */
    #define ITH_WRITE_REG_H(addr, data) \
        do { \
            *(uint16_t volatile*)(ITH_REG_BASE + ITH_HOST_BASE + (addr)) = (data); \
        } while (0)

    /**
     * Reads AMBA register value macro.
     *
     * @param addr The register address.
     * @return The register value.
     */
    #define ITH_READ_REG_A(addr) \
        *(uint32_t volatile*)(ITH_REG_BASE + (addr))

    /**
     * Writes AMBA register value macro.
     *
     * @param addr The register address.
     * @param data The value to write.
     */
    #define ITH_WRITE_REG_A(addr, data) \
        do { \
            *(uint32_t volatile*)(ITH_REG_BASE + (addr)) = (data); \
        } while (0)

#endif // _WIN32

/**
 * Writes host register value with mask.
 * value = (value & mask) | (oldValue & ~mask)
 *
 * @param addr the register address
 * @param data the value will be operated
 * @param mask the mask
 */
static inline void ithWriteRegMaskH(uint16_t addr, uint16_t data, uint16_t mask)
{
    ithWriteRegH(addr, (ithReadRegH(addr) & ~mask) | (data & mask));
}

/**
 * Sets host register bit.
 *
 * @param addr the register address
 * @param bit the bit index to set to 1
 */
static inline void ithSetRegBitH(uint16_t addr, unsigned int bit)
{
    ithWriteRegMaskH(addr, 0x1 << bit, 0x1 << bit);
}

/**
 * Clears host register bit.
 *
 * @param addr the register address
 * @param bit the bit index to set to 0
 */
static inline void ithClearRegBitH(uint16_t addr, unsigned int bit)
{
    ithWriteRegMaskH(addr, 0, 0x1 << bit);
}

/**
 * Writes AMBA register value with mask.
 * value = (value & mask) | (oldValue & ~mask)
 *
 * @param addr the register address
 * @param data the value will be operated
 * @param mask the mask
 */
static inline void ithWriteRegMaskA(uint32_t addr, uint32_t data, uint32_t mask)
{
    ithWriteRegA(addr, (ithReadRegA(addr) & ~mask) | (data & mask));
}

/**
 * Sets AMBA register bit.
 *
 * @param addr the register address
 * @param bit the bit index to set to 1
 */
static inline void ithSetRegBitA(uint32_t addr, unsigned int bit)
{
    ithWriteRegMaskA(addr, 0x1 << bit, 0x1 << bit);
}

/**
 * Clears AMBA register bit.
 *
 * @param addr the register address
 * @param bit the bit index to set to 0
 */
static inline void ithClearRegBitA(uint32_t addr, unsigned int bit)
{
    ithWriteRegMaskA(addr, 0, 0x1 << bit);
}

/**
 * Writes host register value with mask macro.
 * value = (value & mask) | (oldValue & ~mask)
 *
 * @param addr the register address
 * @param data the value will be operated
 * @param mask the mask
 */
#define ITH_WRITE_REG_MASK_H(addr, data, mask) \
    do { \
        ITH_WRITE_REG_H((addr), (ITH_READ_REG_H(addr) & ~(mask)) | ((data) & (mask))); \
    } while (0)

/**
 * Sets host register bit macro.
 *
 * @param addr the register address
 * @param bit the bit index to set to 1
 */
#define ITH_SET_REG_BIT_H(addr, bit) \
    do { \
        ITH_WRITE_REG_MASK_H((addr), 0x1 << (bit), 0x1 << (bit)); \
    } while (0)

/**
 * Clears host register bit macro.
 *
 * @param addr the register address
 * @param bit the bit index to set to 0
 */
#define ITH_CLEAR_REG_BIT_H(addr, bit) \
    do { \
        ITH_WRITE_REG_MASK_H((addr), 0, 0x1 << (bit)); \
    } while (0)

/**
 * Writes AMBA register value with mask macro.
 * value = (value & mask) | (oldValue & ~mask)
 *
 * @param addr the register address
 * @param data the value will be operated
 * @param mask the mask
 */
#define ITH_WRITE_REG_MASK_A(addr, data, mask) \
    do { \
        ITH_WRITE_REG_A((addr), (ITH_READ_REG_A(addr) & ~(mask)) | ((data) & (mask))); \
    } while (0)

/**
 * Sets AMBA register bit macro.
 *
 * @param addr the register address
 * @param bit the bit index to set to 1
 */
#define ITH_SET_REG_BIT_A(addr, bit) \
    do { \
        ITH_WRITE_REG_MASK_A((addr), 0x1 << (bit), 0x1 << (bit)); \
    } while (0)

/**
 * Clears AMBA register bit macro.
 *
 * @param addr the register address
 * @param bit the bit index to set to 0
 */
#define ITH_CLEAR_REG_BIT_A(addr, bit) \
    do { \
        ITH_WRITE_REG_MASK_A((addr), 0, 0x1 << (bit)); \
    } while (0)

/** @} */ // end of ith_reg

/** @defgroup ith_chipid Chip ID
 *  @{
 */
typedef enum
{
    ITH_IT9072    = 0x6,   ///< 128 LQFP
    ITH_IT9076    = 0x4,   ///< 128 LQFP
    ITH_IT9078    = 0x1,   ///< 216 LQFP
    ITH_IT9079    = 0x5,   ///< 176 TQFP
    ITH_IT9079H   = 0x7,   ///< 144 TQFP
    ITH_IT9852_WB = 0x4,   ///< 128 LQFP
    ITH_IT9852_ET = 0x5,   ///< 128 LQFP
    ITH_IT9856_WB = 0x6,   ///< 144 TQFP
    ITH_IT9856_ET = 0x7,   ///< 144 TQFP
    ITH_IT9854_WB = 0x0,   ///< 128 LQFP
    ITH_IT9854_ET = 0x1,   ///< 128 LQFP
    ITH_IT9866_WB = 0x2,   ///< 144 TQFP
    ITH_IT9866_ET = 0x3    ///< 144 TQFP
} ITHPackageId;

/**
 * Gets device ID.
 *
 * @return The device ID.
 */
static inline unsigned int ithGetDeviceId(void)
{
    return ithReadRegH(ITH_DEVICE_ID_REG);
}

/**
 * Gets device revision ID.
 *
 * @return The device revision ID.
 */
static inline unsigned int ithGetRevisionId(void)
{
    // software workaround for IT9850 AX2
    if (ithReadRegH(ITH_DEVICE_ID_REG) == 0x9850)
    {
        static int revid = 0;
        if (revid & (1<<31))
        {
            return ((revid)&~(1<<31));
        }
        else if (ithReadRegH(ITH_REVISION_ID_REG) == 0)
        {
            if ((ithReadRegA(ITH_GPIO_BASE+ITH_GPIO1_PULLEN_REG) & (1<<1)) != 0)
            {
                revid = (1<<31) | 1;
                return 1; // IT9850AX2
            }
            else
            {
                revid = (1<<31) | 0;
                return 0; // IT9850AX1
            }
        }
        else
        {
            return ithReadRegH(ITH_REVISION_ID_REG);
        }
    }
    else
    {
        return ithReadRegH(ITH_REVISION_ID_REG);
    }
}

static inline unsigned int ithIsTilingModeOn(void)
{
//#if defined(CFG_TILING_MODE_OFF)
//	return 0;
//#else
//  #if defined(CFG_DOORBELL_INDOOR) || defined(CFG_DOORBELL_ADMIN)
//    if (ithGetRevisionId() == 0)
//    	return 0;
//    else
//    	return 1;
//  #else
//	return 1;
//  #endif  	
//#endif

#if defined(CFG_TILING_MODE_OFF)
	return 0;
#else    
    if (ithReadRegH(0x300) & 0x8000)
    	return 0;
    else
    	return 1;    		
#endif
}

static inline unsigned int ithTilingPitch(void)
{
    volatile uint16_t value;
    unsigned int pitch;
           
    value = ithReadRegH(0x300) & 0x7000;
    
    switch (value)
    {    
        case 0x3000:
            pitch = 2048;
            break;        
        case 0x4000:
            pitch = 3072;
            break;
        case 0x5000:
            pitch = 4096;
            break;           
        default:
        	pitch = 2048;
            break;
    }
    
    return pitch;
}
/**
 * Gets device package ID.
 *
 * @return The device package ID.
 */
ITHPackageId ithGetPackageId(void);

/** @} */ // end of ith_chipid

/** @defgroup ith_clock Clock
 *  @{
 */

/**
 * Clock definition.
 */
typedef enum
{
    ITH_BCLK = ITH_HOST_CLK1_REG,   ///< BCLK
    ITH_MCLK = ITH_MEM_CLK1_REG,    ///< MCLK
    ITH_NCLK = ITH_AHB_CLK1_REG,    ///< NCLK
    ITH_WCLK = ITH_APB_CLK1_REG,    ///< WCLK
    ITH_FCLK = ITH_ARM_CLK1_REG     ///< FCLK
} ITHClock;

/**
 * Clock output definition.
 */
typedef enum
{
    ITH_CLK_PLL1_OUTPUT1 = 0, ///< From PLL1 output1
    ITH_CLK_PLL1_OUTPUT2 = 1, ///< From PLL1 output2
    ITH_CLK_PLL2_OUTPUT1 = 2, ///< From PLL2 output1
    ITH_CLK_PLL2_OUTPUT2 = 3, ///< From PLL2 output2
    ITH_CLK_PLL3_OUTPUT1 = 4, ///< From PLL3 output1
    ITH_CLK_PLL3_OUTPUT2 = 5, ///< From PLL3 output2
    ITH_CLK_CKSYS        = 6, ///< From CKSYS (12MHz)
    ITH_CLK_RING_OSC     = 7  ///< From Ring OSC (200KHz)
} ITHClockSource;

/**
 * PLL definition.
 */
typedef enum
{
    ITH_PLL1 = 0, ///< PLL1
    ITH_PLL2 = 1, ///< PLL2
    ITH_PLL3 = 2  ///< PLL3
} ITHPll;

/**
 * Initialize clock module.
 */
void ithClockInit(void);

/**
 * Enter sleep mode on clock module.
 */
void ithClockSleep(void);

/**
 * Wakeup from sleep mode on clock module.
 */
void ithClockWakeup(void);

/**
 * Enter suspend mode on clock module.
 */
void ithClockSuspend(void);

/**
 * Resume from suspend mode on clock module.
 */
void ithClockResume(void);

/**
 * Gets clock source.
 *
 * @param clk The clock
 * @return The clock source
 */
static inline ITHClockSource ithClockGetSource(ITHClock clk)
{
    return (ITHClockSource)((ithReadRegH(clk) & ITH_MCLK_SRC_SEL_MASK) >> ITH_MCLK_SRC_SEL_BIT);
}

/**
 * Sets clock source.
 *
 * @param clk The clock
 * @param src The clock source to set
 */
void ithClockSetSource(ITHClock clk, ITHClockSource src);

static inline unsigned int ithClockGetRatio(ITHClock clk)
{
    return (ithReadRegH(clk) & ITH_MCLK_RATIO_MASK) >> ITH_MCLK_RATIO_BIT;
}

/**
 * Sets clock ratio.
 *
 * @param clk The clock
 * @param ratio The clock ratio to set.
 */
void ithClockSetRatio(ITHClock clk, unsigned int ratio);

/**
 * Gets CPU's clock ratio.
 *
 * @return The CPU clock ratio.
 */
static inline unsigned int ithGetCpuClockRatio(void)
{
    return ithClockGetRatio(ITH_FCLK);
}

/**
 * Sets CPU's clock ratio.
 *
 * @param ratio The clock ratio to set.
 */
static inline void ithSetCpuClockRatio(unsigned int ratio)
{
    ithClockSetRatio(ITH_FCLK, ratio);
}

/**
 * Sets CPU's clock ratio mutiple.
 *
 * @param ratio The recent clock ratio.
 * @param multiple The recent clock ratio`s multiple to set.
 */
static void ithSetCpuClockRatioMultiple(unsigned int ratio , unsigned int multiple)
{
	unsigned int value = 0;
	if(multiple)
	{
		value = ((ratio + 1)  * multiple) -1;
		ithClockSetRatio(ITH_FCLK, value);
	}else
		return;
}

/**
 * Gets memory's clock ratio.
 *
 * @return The memory clock ratio.
 */
static inline unsigned int ithGetMemClockRatio(void)
{
    return ithClockGetRatio(ITH_MCLK);
}

/**
 * Sets memory's clock ratio.
 *
 * @param ratio The clock ratio to set.
 */
static inline void ithSetMemClockRatio(unsigned int ratio)
{
    ithClockSetRatio(ITH_MCLK, ratio);
}

/**
 * Sets memory's clock ratio mutiple.
 *
 * @param ratio The recent clock ratio.
 * @param multiple The recent clock ratio`s multiple to set.
 */
static void ithSetMemClockRatioMultiple(unsigned int ratio , unsigned int multiple)
{
	unsigned int value = 0;
	if(multiple)
	{
		value = ((ratio + 1)  * multiple) -1;
		ithClockSetRatio(ITH_MCLK, value);
	}else
		return;
}

/**
 * Gets bus's clock ratio.
 *
 * @return The bus clock ratio.
 */
static inline unsigned int ithGetBusClockRatio(void)
{
    return ithClockGetRatio(ITH_WCLK);
}

/**
 * Sets bus's clock ratio.
 *
 * @param ratio The clock ratio to set.
 */
static inline void ithSetBusClockRatio(unsigned int ratio)
{
    ithClockSetRatio(ITH_WCLK, ratio);
}

/**
 * Sets bus's clock ratio mutiple.
 *
 * @param ratio The recent clock ratio.
 * @param multiple The recent clock ratio`s multiple to set.
 */
static void ithSetBusClockRatioMultiple(unsigned int ratio , unsigned int multiple)
{
	unsigned int value = 0;
	if(multiple)
	{
		value = ((ratio + 1)  * multiple) -1;
		ithClockSetRatio(ITH_WCLK, value);
	}else
		return;
}

/**
 * Gets CPU clock.
 *
 * @return The CPU clock value.
 */
unsigned int ithGetCpuClock(void);

/**
 * Gets memory clock.
 *
 * @return The memory clock value.
 */
unsigned int ithGetMemClock(void);

/**
 * Gets bus clock.
 *
 * @return The bus clock value.
 */
unsigned int ithGetBusClock(void);

/**
 * Enables specified PLL.
 *
 * @param pll The PLL to enable.
 */
void ithClockEnablePll(ITHPll pll);

/**
 * Disables specified PLL.
 *
 * @param pll The PLL to disable.
 */
void ithClockDisablePll(ITHPll pll);

/**
 * Clock output definition.
 */
typedef enum
{
    ITH_SP_UP       = 0, ///< up-spread
    ITH_SP_DOWN     = 1, ///< down-spread
    ITH_SP_CENTER   = 2  ///< center-spread
} ITHSpreadSpectrumMode;

/**
 * Enables spread spectrum.
 *
 * @param pll PLL to enable.
 */
static inline void ithEnableSpreadSpectrum(ITHPll pll)
{
    ithSetRegBitH(ITH_PLL1_SET6_REG + pll * 0x10, ITH_PLL1_SP_EN_BIT);
}

/**
 * Disables spread spectrum.
 *
 * @param pll PLL to disable.
 */
static inline void ithDisableSpreadSpectrum(ITHPll pll)
{
    ithClearRegBitH(ITH_PLL1_SET6_REG + pll * 0x10, ITH_PLL1_SP_EN_BIT);
}

/**
 * Sets spread spectrum parameters.
 *
 * @param pll PLL to set.
 * @param mode Spread spectrum mode.
 * @param width Modulation width in 1/1000 unit, recommand 1 ~ 5.
 * @param freq Modulation frequency in Hz, recommand 30K ~ 50KHz.
 */
void ithSetSpreadSpectrum(ITHPll pll, ITHSpreadSpectrumMode mode, uint32_t width, uint32_t freq);

/**
 * Print clock information.
 */
void ithClockStats(void);

/** @} */ // end of ith_clock

/** @defgroup ith_memdbg Memory Debug
 *  @{
 */

/**
 * Memory Debug module definition.
 */
typedef enum
{
    ITH_MEMDBG0 = ITH_MEMDBG0_BASE_REG, ///< MEMDBG #0
    ITH_MEMDBG1 = ITH_MEMDBG1_BASE_REG  ///< MEMDBG #1
} ITHMemDbgModule;

/**
 * Memory Debug mode definition.
 */
typedef enum
{
    ITH_MEMDBG_READWRITE = 0x0, ///< Both of Read and Write Requests
    ITH_MEMDGB_READONLY  = 0x1, ///< Only Read Request
    ITH_MEMDGB_WRITEONLY = 0x2  ///< Only Write Request
} ITHMemDbgMode;

/**
 * Enables memory debug. interrupt
 *
 */
static inline void ithMemDbgEnableIntr(void)
{
#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
    // do nothing
#elif (CFG_CHIP_FAMILY == 9850)
    ithSetRegBitH(ITH_MEMSTAT_INTR_REG, ITH_MEMSTAT_INTR_EN_BIT);
#else
    // TODO: others project
#endif
}

/**
 * Disable memory debug. interrupt
 *
 */
static inline void ithMemDbgDisableIntr(void)
{
#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
    // do nothing
#elif (CFG_CHIP_FAMILY == 9850)
    ithClearRegBitH(ITH_MEMSTAT_INTR_REG, ITH_MEMSTAT_INTR_EN_BIT);
#else
    // TODO: others project
#endif
}

/**
 * Enables memory debug.
 *
 * @param module The memory debug module.
 */
static inline void ithMemDbgEnable(ITHMemDbgModule module)
{
    ithSetRegBitH(module + ITH_MEMDBG_EN_REG, ITH_MEMDBG_EN_BIT);

#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
    // do nothing
#elif (CFG_CHIP_FAMILY == 9850)
    ithSetRegBitH(ITH_MEMSTAT_INTR_REG, (module == ITH_MEMDBG0) ? ITH_MEMSTAT_INTR_WP0_BIT : ITH_MEMSTAT_INTR_WP1_BIT);
#else
    // TODO: others project
#endif
}

/**
 * Disables memory debug.
 *
 * @param module The memory debug module.
 */
static inline void ithMemDbgDisable(ITHMemDbgModule module)
{
    ithClearRegBitH(module + ITH_MEMDBG_EN_REG, ITH_MEMDBG_EN_BIT);

#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
    // do nothing
#elif (CFG_CHIP_FAMILY == 9850)
    ithClearRegBitH(ITH_MEMSTAT_INTR_REG, (module == ITH_MEMDBG0) ? ITH_MEMSTAT_INTR_WP0_BIT : ITH_MEMSTAT_INTR_WP1_BIT);
#else
    // TODO: others project
#endif
}

/**
 * Sets memory debug mode.
 *
 * @param module The memory debug module.
 * @param mode The memory debug mode.
 */
static inline void ithMemDbgSetMode(ITHMemDbgModule module, ITHMemDbgMode mode)
{
    ithWriteRegMaskH(module + ITH_MEMDBG_MODE_REG, mode << ITH_MEMDBG_MODE_BIT, ITH_MEMDBG_MODE_MASK);
}

/**
 * Sets memory debug range.
 *
 * @param module The memory debug module.
 * @param topAddr The top address.
 * @param bottomAddr The bottom address.
 */
static inline void ithMemDbgSetRange(ITHMemDbgModule module, uint32_t topAddr, uint32_t bottomAddr)
{
    ithWriteRegH(module + ITH_MEMDBG_TOP_ADDR_LO_REG, topAddr >> 3);
    ithWriteRegMaskH(module + ITH_MEMDBG_TOP_ADDR_HI_REG, (topAddr >> 19) << ITH_MEMDBG_TOP_ADDR_HI_BIT, ITH_MEMDBG_TOP_ADDR_HI_MASK);
    ithWriteRegH(module + ITH_MEMDBG_BOT_ADDR_LO_REG, bottomAddr >> 3);
    ithWriteRegMaskH(module + ITH_MEMDBG_BOT_ADDR_HI_REG, (bottomAddr >> 19) << ITH_MEMDBG_BOT_ADDR_HI_BIT, ITH_MEMDBG_BOT_ADDR_HI_MASK);
}

/**
 * Gets memory debug status.
 *
 * @return The status.
 */
static inline uint16_t ithMemDbgGetStatus(void)
{
    return ithReadRegH(ITH_MEMDBG_FLAG_REG);
}

/**
 * Gets memory debug status.
 *
 * @param module The memory debug module.
 * @return The flag.
 */
static inline int32_t ithMemDbgGetFlag(ITHMemDbgModule module)
{
    uint16_t status = ithMemDbgGetStatus();

    if (module == ITH_MEMDBG0)
    {
        if (status & (1 << ITH_MEMDBG_FLAG_STATUS0_REG))
        {
            return ((status & ITH_MEMDBG_FLAG_STATUS0_MASK) >> ITH_MEMDBG_FLAG_STATUS0_BIT);
        }
        else
        {
            return -1;
        }
    }
    else
    {
        if (status & (1 << ITH_MEMDBG_FLAG_STATUS1_REG))
        {
            return ((status & ITH_MEMDBG_FLAG_STATUS1_MASK) >> ITH_MEMDBG_FLAG_STATUS1_BIT);
        }
        else
        {
            return -1;
        }
    }
}

/**
 * Clear interrupt.
 */
static inline void ithMemDbgClearIntr(void)
{
    ithSetRegBitH(ITH_MEMSTAT_INTR_REG, ITH_MEMSTAT_INTR_WC_BIT);
}

/** @} */ // end of ith_memdbg

/** @defgroup ith_memstat Memory Statistic
 *  @{
 */

/**
 * Memory Statistic request definition.
 */
typedef enum
{
    ITH_MEMSTAT_I2S_READ    = 0x0,  ///< I2S read request
    ITH_MEMSTAT_I2S_WRITE   = 0x1,  ///< I2S write request
    ITH_MEMSTAT_TSI_WRITE   = 0x2,  ///< TSI write request
    ITH_MEMSTAT_HOST_WRITE  = 0x3,  ///< Host bus controller write request
    ITH_MEMSTAT_HOST_READ   = 0x4,  ///< Host bus controller read request
    ITH_MEMSTAT_LCD_READ    = 0x5,  ///< LCD controller read request
    ITH_MEMSTAT_ISP_READ    = 0x6,  ///< ISP read request
    ITH_MEMSTAT_CAP_READ    = 0x7,  ///< CAP write request
    ITH_MEMSTAT_CMDQ_READ   = 0x8,  ///< Command queue read request
    ITH_MEMSTAT_IQUEUE_READ = 0x9,  ///< I queue read request
    ITH_MEMSTAT_USB         = 0xA,  ///< USB controller read/write request
    ITH_MEMSTAT_ARM         = 0xB,  ///< ARM read/write request
    ITH_MEMSTAT_VIDEO_READ  = 0xC,  ///< Video engine read request
    ITH_MEMSTAT_VIDEO_WRITE = 0xD,  ///< Video engine write request
    ITH_MEMSTAT_ISP_WRITE   = 0xE,  ///< ISP write request
    ITH_MEMSTAT_TS          = 0xF,  ///< TS parser read/write request
    ITH_MEMSTAT_RISC        = 0x10, ///< RISC read/write request
    ITH_MEMSTAT_AHB         = 0x11, ///< AHB read/write request
    ITH_MEMSTAT_UENC_READ   = 0x12, ///< UEncoder engine read request
    ITH_MEMSTAT_UENC_WRITE  = 0x13, ///< UEncoder engine write request
    ITH_MEMSTAT_OPENVG      = 0x14, ///< Open VG read/write request
    ITH_MEMSTAT_MEMBIST     = 0x15, ///< Memory BIST read/write request
    ITH_MEMSTAT_MEMBIST1    = 0x16  ///< Memory BIST1 read/write request
} ITHMemStatRequest;

/**
 * Sets memory access service count period.
 *
 * @param period The memory access service count period.
 */
static inline void ithMemStatSetServCountPeriod(uint16_t period)
{
    ithWriteRegH(ITH_MEMSTAT_SERV_PERIOD_REG, period);
}

/**
 * Enables Memory access service counter.
 */
static inline void ithMemStatServCounterEnable(void)
{
    ithSetRegBitH(ITH_MEMSTAT_SERV_CR_REG, ITH_MEMSTAT_SERV_EN_BIT);
}

/**
 * Disables Memory access service counter.
 */
static inline void ithMemStatServCounterDisable(void)
{
    ithClearRegBitH(ITH_MEMSTAT_SERV_CR_REG, ITH_MEMSTAT_SERV_EN_BIT);
}

/**
 * Sets memory service0 request.
 *
 * @param rq The memory stat request.
 */
static inline void ithMemStatSetServ0Request(ITHMemStatRequest rq)
{
    ithWriteRegMaskH(ITH_MEMSTAT_SERV_CR_REG, rq << ITH_MEMSTAT_SERV0_RQ_BIT, ITH_MEMSTAT_SERV0_RQ_MASK);
}

/**
 * Sets memory service1 request.
 *
 * @param rq The memory stat request.
 */
static inline void ithMemStatSetServ1Request(ITHMemStatRequest rq)
{
    ithWriteRegMaskH(ITH_MEMSTAT_SERV_CR_REG, rq << ITH_MEMSTAT_SERV1_RQ_BIT, ITH_MEMSTAT_SERV1_RQ_MASK);
}

/**
 * Gets memory service access number.
 *
 * @return The memory service access number.
 */
static inline uint32_t ithMemStatGetServNum(void)
{
    return ((uint32_t)ithReadRegH(ITH_MEMSTAT_SERVALL_NUM_HI_REG) << 16) |
        ithReadRegH(ITH_MEMSTAT_SERVALL_NUM_LO_REG);
}

/**
 * Gets memory service0 access number.
 *
 * @return The memory service0 access number.
 */
static inline uint32_t ithMemStatGetServ0Num(void)
{
    return ((uint32_t)ithReadRegH(ITH_MEMSTAT_SERV0_NUM_HI_REG) << 16) |
        ithReadRegH(ITH_MEMSTAT_SERV0_NUM_LO_REG);
}

/**
 * Gets memory service0 access number.
 *
 * @return The memory service0 access number.
 */
static inline uint32_t ithMemStatGetServ1Num(void)
{
    return ((uint32_t)ithReadRegH(ITH_MEMSTAT_SERV1_NUM_HI_REG) << 16) |
        ithReadRegH(ITH_MEMSTAT_SERV1_NUM_LO_REG);
}

/**
 * Gets memory all service requests time counts in indication count period.
 *
 * @return The memory all service requests time counts in indication count period.
 */
static inline uint32_t ithMemStatGetAllServCount(void)
{
    return ((uint32_t)ithReadRegH(ITH_MEMSTAT_SERVALL_CNT_HI_REG) << 16) |
        ithReadRegH(ITH_MEMSTAT_SERVALL_CNT_LO_REG);
}

/**
 * Gets memory time counts in indication count period.
 *
 * @return The memory time counts in indication count period.
 */
static inline uint32_t ithMemStatGetServCount(void)
{
    return ((uint32_t)ithReadRegH(ITH_MEMSTAT_SERVTIME_CNT_HI_REG) << 16) |
        ithReadRegH(ITH_MEMSTAT_SERVTIME_CNT_LO_REG);
}

/** @} */ // end of ith_memstat

/** @defgroup ith_host Host
 *  @{
 */

/**
 * Enters suspend mode on host module.
 */
void ithHostSuspend(void);

/**
 * Resumes from suspend mode on host module.
 */
void ithHostResume(void);

/** @} */ // end of ith_host

/** @defgroup ith_ahb0 AHB0
 *  @{
 */

/**
 * Set AHB0 control register as default value.
 */
static inline void ithAhb0SetCtrlReg(void)
{
    ithWriteRegA(ITH_AHB0_BASE + ITH_AHB0_CTRL_REG, ITH_AHB0_CTRL_REG_DEFAULT);
}

/** @} */ // end of ith_ahb0

/** @defgroup ith_isp ISP
 *  @{
 */

/**
 * Enables ISP's clock.
 */
void ithIspEnableClock(void);

/**
 * Disables ISP's clock.
 */
void ithIspDisableClock(void);

/**
 * Sets ISP's clock source.
 */
void ithIspSetClockSource(ITHClockSource src);

/**
 * Resets ISP's registers.
 */
void ithIspResetReg(void);

void ithIspQResetReg(void);

/**
 * Resets ISP's engine.
 */
void ithIspResetEngine(void);

/**
 * Gets ISP's clock source.
 */
static inline ITHClockSource ithIspGetClockSource(void)
{
    return (ITHClockSource)((ithReadRegH(ITH_ISP_CLK1_REG) & ITH_ICLK_SRC_SEL_MASK) >> ITH_ICLK_SRC_SEL_BIT);
}

/** @} */ // end of ith_isp

/** @defgroup ith_usb USB
 *  @{
 */

/**
 * USB module definition.
 */
typedef enum
{
    ITH_USB0    = ITH_USB0_BASE, ///< USB #0
    ITH_USB1    = ITH_USB1_BASE  ///< USB #1
} ITHUsbModule;

/**
 * Enters suspend mode on specified USB module.
 *
 * @param usb The USB module to suspend.
 */
void ithUsbSuspend(ITHUsbModule usb);

/**
 * Resumes from suspend mode on specified USB module.
 *
 * @param usb The USB module to resume.
 */
void ithUsbResume(ITHUsbModule usb);

/**
 * Enables the clock of USB module.
 */
void ithUsbEnableClock(void);

/**
 * Disables the clock of USB module.
 */
void ithUsbDisableClock(void);

/**
 * Reset the USB module.
 */
void ithUsbReset(void);

/**
 * USB AHB master interface type.
 */
typedef enum
{
    ITH_USB_WRAP    = 0, ///< Select USBWrap
    ITH_USB_AMBA    = 1  ///< Select AMBA
} ITHUsbInterface;

/**
 * USB AHB master interface select.
 */
void ithUsbInterfaceSel(ITHUsbInterface intf);

/**
 * USB PHY power on.
 */
void ithUsbPhyPowerOn(ITHUsbModule usb);

/**
 * Enables the clock of USB's PHY module.
 */
static inline void ithUsbPhyEnableClock(ITHUsbModule usb)
{
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
    if(usb == ITH_USB0)
        ithWriteRegMaskH(ITH_USB0_PHY_CTRL_REG, (0x1<<5) , (0x1<<5));
    if(usb == ITH_USB1)
        ithWriteRegMaskH(ITH_USB1_PHY_CTRL_REG, (0x1<<5) , (0x1<<5));
#else
    ithWriteRegMaskA(usb + 0x3C, 0x2, 0x2);
#endif
}

/**
 * Disables the clock of USB's PHY module.
 */
static inline void ithUsbPhyDisableClock(ITHUsbModule usb)
{
#if ((CFG_CHIP_FAMILY == 9070) || (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850))
    if (usb == ITH_USB0)
        ithWriteRegMaskH(ITH_USB0_PHY_CTRL_REG, (0x0<< 5) , (0x1<<5));
    if(usb == ITH_USB1)
        ithWriteRegMaskH(ITH_USB1_PHY_CTRL_REG, (0x0<< 5) , (0x1<<5));
#else
    ithWriteRegMaskA(usb + 0x3C, 0x0, 0x2);
#endif
}

/** @} */ // end of ith_usb

/** @defgroup ith_jpeg JPEG
 *  @{
 */

/**
 * Enables the clock of JPEG module.
 */
void ithJpegEnableClock(void);

/**
 * Disables the clock of JPEG module.
 */
void ithJpegDisableClock(void);

/**
 * Resets the regisers of JPEG module.
 */
void ithJpegResetReg(void);

/**
 * Resets the engine of JPEG module.
 */
void ithJpegResetEngine(void);

/** @} */ // end of ith_jpeg

/** @defgroup ith_uiEnc Ui Encode
 *  @{
 */

/**
 * Enables the clock of Ui Encode module.
 */
void ithUiEncEnableClock(void);

/**
 * Disables the clock of Ui Encod module.
 */
void ithUiEncDisableClock(void);

/**
 * Resets the regisers of Ui Encod module.
 */
void ithUiEncResetReg(void);

/**
 * Resets the engine of Ui Encod module.
 */
void ithUiEncResetEngine(void);

/** @} */ // end of ith_uiEnc

/** @defgroup ith_cap Capture controllor
 *  @{
 */

/**
 * Enables the clock of Capture controllo module.
 */
void ithCapEnableClock(void);

/**
 * Disables the clock of Capture controllo module.
 */
void ithCapDisableClock(void);

/**
 * Resets the regisers of Capture controllo module.
 */
void ithCapResetReg(void);

/**
 * Resets the engine of Capture controllo module.
 */
void ithCapResetEngine(void);

/**
 * Enables the clock of Capture controller with Video .
 */
void ithCapVideoEnableClock(void);

/**
 * Disable the clock of Capture controller with Video .
 */
void ithCapVideoDisableClock(void);

/** @} */ // end of ith_cap

/** @defgroup ith_video Video
 *  @{
 */

/**
 * Enables the clock of video module.
 */
void ithVideoEnableClock(void);

/**
 * Disables the clock of video module.
 */
void ithVideoDisableClock(void);

/** @} */ // end of ith_video

/** @defgroup ith_lcd LCD
 *  @{
 */
typedef enum
{
    ITH_LCD_RGB565      = 0, ///< RGB565 format
    ITH_LCD_ARGB1555    = 1, ///< ARGB1555 format
    ITH_LCD_ARGB4444    = 2, ///< ARGB4444 format
    ITH_LCD_ARGB8888    = 3, ///< ARGB8888 format
    ITH_LCD_YUV         = 4  ///< YUV format
} ITHLcdFormat;

/**
 * Resets LCD module.
 */
void ithLcdReset(void);

/**
 * Loads the first part of LCD initial script.
 *
 * @param script the script array.
 * @param count the item count in the script array.
 */
#if (CFG_CHIP_FAMILY == 9920)
void ithLcdLoadScriptFirst(const uint32_t* script, unsigned int count);
#else
void ithLcdLoadScriptFirst(const uint16_t* script, unsigned int count);
#endif

/**
 * Runs the next part of LCD initial script.
 */
void ithLcdLoadScriptNext(void);

/**
 * Enables LCD module.
 */
void ithLcdEnable(void);

/**
 * Disables LCD module.
 */
void ithLcdDisable(void);

/**
 * Sets LCD base address A.
 *
 * @param addr the address to set.
 */
void ithLcdSetBaseAddrA(uint32_t addr);

/**
 * Gets LCD base address A.
 *
 * @return the address A.
 */
static inline uint32_t ithLcdGetBaseAddrA(void)
{
    // this function must be an inline function, because lcd_clear.c will use it.
    // don't modify it.
#if (CFG_CHIP_FAMILY == 9920)
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_BASEA_REG) & ITH_LCD_BASEA_MASK);
#else
    return ((uint32_t)(ithReadRegH(ITH_LCD_BASEA_HI_REG) & ITH_LCD_BASEA_HI_MASK) << 16) |
        (ithReadRegH(ITH_LCD_BASEA_LO_REG) & ITH_LCD_BASEA_LO_MASK);
#endif
}

/**
 * Sets LCD base address A.
 *
 * @param addr the address to set.
 */
void ithLcdSetBaseAddrB(uint32_t addr);

/**
 * Gets LCD base address B.
 *
 * @return the address B.
 */
uint32_t ithLcdGetBaseAddrB(void);

/**
 * Sets LCD base address C.
 *
 * @param addr the address to set.
 */
void ithLcdSetBaseAddrC(uint32_t addr);

/**
 * Gets LCD base address C.
 *
 * @return the address C.
 */
uint32_t ithLcdGetBaseAddrC(void);

/**
 * Enables hardware flip.
 */
void ithLcdEnableHwFlip(void);

/**
 * Disables hardware flip.
 */
void ithLcdDisableHwFlip(void);

/**
 * Enables video flip.
 */
void ithLcdEnableVideoFlip(void);

/**
 * Disables video flip.
 */
void ithLcdDisableVideoFlip(void);

/**
 * Gets current LCD format.
 *
 * @return the LCD format.
 */
 ITHLcdFormat ithLcdGetFormat(void);

/**
 * Gets current LCD width.
 *
 * @return the LCD width.
 */
static inline unsigned int ithLcdGetWidth(void)
{
#if (CFG_CHIP_FAMILY == 9920)
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_WIDTH_REG) & ITH_LCD_WIDTH_MASK) >> ITH_LCD_WIDTH_BIT;
#else
    return (ithReadRegH(ITH_LCD_WIDTH_REG) & ITH_LCD_WIDTH_MASK) >> ITH_LCD_WIDTH_BIT;
#endif
}

/**
 * Sets current LCD width.
 *
 * @param width the LCD width.
 */
 void ithLcdSetWidth(uint32_t width);

/**
 * Gets current LCD height.
 *
 * @return the LCD height.
 */
static inline unsigned int ithLcdGetHeight(void)
{
    // this function must be an inline function, because lcd_clear.c will use it.
    // don't modify it.
#if (CFG_CHIP_FAMILY == 9920)
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_HEIGHT_REG) & ITH_LCD_HEIGHT_MASK) >> ITH_LCD_HEIGHT_BIT;
#else
    return (ithReadRegH(ITH_LCD_HEIGHT_REG) & ITH_LCD_HEIGHT_MASK) >> ITH_LCD_HEIGHT_BIT;
#endif
}

/**
 * Sets current LCD height.
 *
 * @param width the LCD height.
 */
 void ithLcdSetHeight(uint32_t height);

/**
 * Gets current LCD pitch.
 *
 * @return the LCD pitch.
 */
static inline unsigned int ithLcdGetPitch(void)
{
    // this function must be an inline function, because lcd_clear.c will use it.
    // don't modify it.
#if (CFG_CHIP_FAMILY == 9920)
    return (ithReadRegA(ITH_LCD_BASE + ITH_LCD_PITCH_REG) & ITH_LCD_PITCH_MASK) >> ITH_LCD_PITCH_BIT;
#else
    return (ithReadRegH(ITH_LCD_PITCH_REG) & ITH_LCD_PITCH_MASK) >> ITH_LCD_PITCH_BIT;
#endif
}

/**
 * Sets current LCD pitch.
 *
 * @param width the LCD pitch.
 */
 void ithLcdSetPitch(uint32_t pitch);

/**
 * Gets current LCD scanline on X coordinate.
 *
 * @return the LCD scanline on X coordinate.
 */
 unsigned int ithLcdGetXCounter(void);

/**
 * Gets current LCD scanline on Y coordinate.
 *
 * @return the LCD scanline on Y coordinate.
 */
 unsigned int ithLcdGetYCounter(void);

/**
 * Synchronize fire LCD.
 */
 void ithLcdSyncFire(void);

/**
 * Is LCD synchronize fire or not.
 *
 * @return Is LCD synchronize fire or not.
 */
 bool ithLcdIsSyncFired(void);

/**
 * Is LCD enabled or not.
 *
 * @return Is LCD enabled or not.
 */
 bool ithLcdIsEnabled(void);

/**
 * Gets current LCD flip counter.
 *
 * @return the flip counter. 0 is A, 1 is B, 2 is C.
 */
 unsigned int ithLcdGetFlip(void);

 unsigned int ithLcdGetMaxLcdBufCount(void);

/**
 * Software flip the LCD.
 *
 * @param index the index to flip. 0 is A, 1 is B, 2 is C.
 */
 void ithLcdSwFlip(unsigned int index);
// LCD Cursor

/**
 * Cursor control definition
 */
typedef enum
{
    ITH_LCD_CURSOR_ALPHABLEND_ENABLE = 0,     ///< Enable hardware cursor alpha blending function
    ITH_LCD_CURSOR_DEFDEST_ENABLE,            ///< Enable hardware cursor default destination data (RGB565)
    ITH_LCD_CURSOR_INVDEST_ENABLE             ///< Inverse hardware cursor re-define dstination color when hardware cursor is "11"
} ITHLcdCursorCtrl;

/**
 * Cursor color definition
 */
typedef enum
{
    ITH_LCD_CURSOR_DEF_COLOR    = 0,    ///< Hardware cursor default color (RGB565)
    ITH_LCD_CURSOR_FG_COLOR,            ///< Hardware cursor foreground color (RGB565)
    ITH_LCD_CURSOR_BG_COLOR             ///< Hardware cursor background color (RGB565)
} ITHLcdCursorColor;

/**
 * Enables hardware cursor
 */
 void ithLcdCursorEnable(void);

/**
 * Disables hardware cursor
 */
void ithLcdCursorDisable(void);

/**
 * Enables specified cursor controls.
 *
 * @param ctrl the controls to enable.
 */
 void ithLcdCursorCtrlEnable(ITHLcdCursorCtrl ctrl);

/**
 * Disables specified cursor controls.
 *
 * @param ctrl the controls to disable.
 */
 void ithLcdCursorCtrlDisable(ITHLcdCursorCtrl ctrl);

/**
 * Sets the width of cursor.
 *
 * @param width the width.
 */
 void ithLcdCursorSetWidth(unsigned int width);

/**
 * Sets the height of cursor.
 *
 * @param height the height.
 */
 void ithLcdCursorSetHeight(unsigned int height);

/**
 * Sets the pitch of cursor.
 *
 * @param pitch the pitch.
 */
 void ithLcdCursorSetPitch(unsigned int pitch);

/**
 * Sets the X coordinate of cursor.
 *
 * @param x the X coordinate.
 */
 void ithLcdCursorSetX(unsigned int x);

/**
 * Sets the Y coordinate of cursor.
 *
 * @param y the Y coordinate.
 */
 void ithLcdCursorSetY(unsigned int y);

/**
 * Sets base address of cursor.
 *
 * @param addr the base address to set.
 */
void ithLcdCursorSetBaseAddr(uint32_t addr);

/**
 * Sets the color of cursor.
 *
 * @param color the color type to set.
 * @param value the color value to set.
 */
 void ithLcdCursorSetColor(ITHLcdCursorColor color, uint16_t value);

/**
 * Sets the color weight of cursor.
 *
 * @param color the color type to set.
 * @param value the weight value to set.
 */
void ithLcdCursorSetColorWeight(ITHLcdCursorColor color, uint8_t value);

/**
 * Updates the cursor.
 */
 void ithLcdCursorUpdate(void);

/**
 * Is cursor updated or not.
 *
 * @return Is cursor updated or not.
 */
 bool ithLcdCursorIsUpdateDone(void);

// LCD Interrupt
/**
 * LCD interupt controls definition.
 */
typedef enum
{
    ITH_LCD_INTR_ENABLE = 0,   ///< Enable LCD Interrupt
    ITH_LCD_INTR_OUTPUT2,      ///< Interrupt from output2 (9070)
    ITH_LCD_INTR_FIELDMODE2,   ///< Output2 Interrupt on field mode (9070)
    ITH_LCD_INTR_OUTPUT1,      ///< Interrupt from output1 (9070)
    ITH_LCD_INTR_FIELDMODE1,   ///< output1 Interrupt on field mode (9070/9850)
} ITHLcdIntrCtrl;

/**
 * Enables specified interrupt controls.
 *
 * @param ctrl the controls to enable.
 */
 void ithLcdIntrCtrlEnable(ITHLcdIntrCtrl ctrl);

/**
 * Disables specified interrupt controls.
 *
 * @param ctrl the controls to disable.
 */
 void ithLcdIntrCtrlDisable(ITHLcdIntrCtrl ctrl);

/**
 * Enables LCD interrupt
 */
 void ithLcdIntrEnable(void);

/**
 * Disables LCD interrupt
 */
 void ithLcdIntrDisable(void);

/**
 * Clears LCD interrupt
 */
 void ithLcdIntrClear(void);

/**
 * Sets the first scanline to interrupt.
 *
 * @param line the scanline to interrupt.
 */
 void ithLcdIntrSetScanLine1(unsigned int line);

/**
 * Sets the second scanline to interrupt.
 *
 * @param line the scanline to interrupt.
 */
 void ithLcdIntrSetScanLine2(unsigned int line);

#if (CFG_CHIP_FAMILY == 9920)

 typedef enum
 {
     ITH_LCD_ROT0   = 0, ///< No Rotation
     ITH_LCD_FLIP   = 1, ///< Flip
     ITH_LCD_MIRROR = 2, ///< Mirror
     ITH_LCD_ROT180 = 3, ///< Rotation 180
 } ITHLcdRotMode;

 typedef enum
 {
     ITH_LCD_Horizontal = 0, ///< Horizontal
     ITH_LCD_Vertical   = 1, ///< Vertical
 } ITHLcdScanType;

 /**
 * LCD rotation display Mode
 *
 * @param type the LCD panel scan type.
 * @param mode the rotation mode.
 */
 void ithLcdSetRotMode(ITHLcdScanType type, ITHLcdRotMode mode);

#endif

/** @} */ // end of ith_lcd

/** @defgroup ith_cmdq Command Queue
 *  @{
 */
#define ITH_CMDQ_BURST_CMD_SIZE     8 ///< Burst command header size
#define ITH_CMDQ_SINGLE_CMD_SIZE    8 ///< Single command size

/**
 * Command queue global data definition.
 */
typedef struct
{
    uint32_t    addr;   ///< command queue address
    uint32_t    size;   ///< command queue size
    void*       mutex;  ///< command queue mutex for lock/unlock
} ITHCmdQ;

/**
 * Command queue control definition
 */
typedef enum
{
    ITH_CMDQ_FLIPBUFMODE    = 1,    ///< Flip Buffer Mode
    ITH_CMDQ_TURBOFLIP      = 2,    ///< Turbo Flip
    ITH_CMDQ_CMDERRSTOP     = 3,    ///< Enable Cmd Error Stop
    ITH_CMDQ_BIGENDIAN      = 12,   ///< Enable Big Endian
    ITH_CMDQ_INTR           = 13,   ///< Enable CmdQ Interrupt
    ITH_CMDQ_ISPCMDAHB      = 14,   ///< Enable ISP Cmd to AHB Interface
    ITH_CMDQ_WAITISPBUSYAHB = 15    ///< Enable Wait ISP Busy in AHB Interface
} ITHCmdQCtrl;

/**
 * Makes burst command header.
 *
 * @param ptr the place to make.
 * @param addr the address to send to.
 * @param size the command size.
 */
#define ITH_CMDQ_BURST_CMD(ptr, addr, size) \
    do { \
        *(ptr)++ = (addr) | 0x80000001; \
        *(ptr)++ = (size) / sizeof (uint32_t); \
    } while (0)

/**
 * Makes single command.
 *
 * @param ptr the place to make.
 * @param addr the address to send to.
 * @param data the command data.
 */
#define ITH_CMDQ_SINGLE_CMD(ptr, addr, data) \
    do { \
        *(ptr)++ = (addr); \
        *(ptr)++ = (data); \
    } while (0)

/**
* Global command queue instance
*/
extern ITHCmdQ* ithCmdQ;


#if (CFG_CHIP_FAMILY == 9920)

extern ITHCmdQ* ithCmdQ1;

/**
* Command queue port register offset definition
*/
typedef enum
{
    ITH_CMDQ0_OFFSET = 0x0,
    ITH_CMDQ1_OFFSET = ITH_CMDQ_BASE_OFFSET
}ITHCmdQPortOffset;

/**
* Locks command queue.
*/
static inline void ithCmdQLock(ITHCmdQPortOffset portOffset)
{
    if (portOffset == ITH_CMDQ0_OFFSET)
        ithLockMutex(ithCmdQ->mutex);
    else
        ithLockMutex(ithCmdQ1->mutex);
}

/**
* Unlocks command queue.
*/
static inline void ithCmdQUnlock(ITHCmdQPortOffset portOffset)
{
    if (portOffset == ITH_CMDQ0_OFFSET)
        ithUnlockMutex(ithCmdQ->mutex);
    else
        ithUnlockMutex(ithCmdQ1->mutex);
}
#else
/**
* Locks command queue.
*/
static inline void ithCmdQLock(void)
{
    ithLockMutex(ithCmdQ->mutex);
}

/**
* Unlocks command queue.
*/
static inline void ithCmdQUnlock(void)
{
    ithUnlockMutex(ithCmdQ->mutex);
}
#endif

/**
* Prints command queue status.
*/
void ithCmdQStats(void);

/** @} */ // end of ith_cmdq

/** @defgroup ith_openvg OpenVG
 *  @{
 */

/**
 * Enables OpenVG clock.
 */
void ithOvgEnableClock(void);

/**
 * Disables OpenVG clock.
 */
void ithOvgDisableClock(void);

/** @} */ // end of ith_openvg

/** @defgroup ith_tve TV Encoder
 *  @{
 */
/**
 * Resets TV encoder.
 */
void ithTveReset(void);

/**
 * Enables the clock of TV encoder.
 */
void ithTveEnableClock(void);

/**
 * Disables the clock of TV encoder.
 */
void ithTveDisableClock(void);

/**
 * Enables the power of TV encoder.
 */
void ithTveEnablePower(void);

/**
 * Disables the power of TV encoder.
 */
void ithTveDisablePower(void);

/** @} */ // end of ith_tve

/** @defgroup ith_fpc FPC & STC
 *  @{
 */

/**
 * Front panel controller interrupt definition.
 */
 #if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
typedef enum
{
    ITH_FPC_STC_TIMER   = 0, ///< STC Timer Interrupt Enable
    ITH_FPC_KEY_CHANGED = 1, ///< Key Status Change Interrupt Enable
    ITH_FPC_KEY_TIMEOUT = 2  ///< Key Status Time Out Interrupt Enable
} ITHFpcIntr;

/**
 * System time clock control definition.
 */
typedef enum
{
    ITH_STC_RESET       = 0, ///< System Time Clock Reset
    ITH_STC_FIRE        = 1, ///< System Time Clock Fire
    ITH_STC_PAUSE       = 2, ///< System Time Clock Pause
    ITH_STC_TIMER_RESET = 8, ///< System Time Clock Timer Reset
    ITH_STC_TIMER_FIRE  = 9  ///< System Time Clock Timer Fire
} ITHStcCtrl;

/**
 * Resets FPC module.
 */
void ithFpcReset(void);

/**
 * Enables specified STC control.
 *
 * @param ctrl the control to enable.
 */
static inline void ithStcCtrlEnable(ITHStcCtrl ctrl)
{
    ithSetRegBitH(ITH_STC_CTRL_REG, ctrl);
}

/**
 * Disables specified STC control.
 *
 * @param ctrl the control to disable.
 */
static inline void ithStcCtrlDisable(ITHStcCtrl ctrl)
{
    ithClearRegBitH(ITH_STC_CTRL_REG, ctrl);
}

/**
 * Enables specified FPC interrupt.
 *
 * @param intr the interrupt to enable.
 */
static inline void ithFpcEnableIntr(ITHFpcIntr intr)
{
    ithSetRegBitH(ITH_FPC_INTR_SETTING_REG, intr);
}

/**
 * Disables specified FPC interrupt.
 *
 * @param intr the interrupt to disable.
 */
static inline void ithFpcDisableIntr(ITHFpcIntr intr)
{
    ithClearRegBitH(ITH_FPC_INTR_SETTING_REG, intr);
}

/**
 * Clears specified FPC interrupt.
 *
 * @param intr the interrupt to clear.
 */
static inline void ithFpcClearIntr(ITHFpcIntr intr)
{
    ithSetRegBitH(ITH_FPC_INTR_SETTING_REG, 0x8 + intr);
}

/**
 * Gets the state of FPC interrupt.
 *
 * @return the state of FPC interrupt.
 */
static inline uint16_t ithFpcGetIntrState(void)
{
    return ithReadRegH(ITH_FPC_INTR_READBACK_REG);
}
#endif
/**
 * Gets base clock of STC.
 *
 * @return the base clock of STC.
 */
static inline uint32_t ithStcGetBaseClock(void)
{
#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
    return (ithReadRegH(ITH_STC_BASECNT_LO_REG) | (uint32_t)ithReadRegH(ITH_STC_BASECNT_HI_REG) << 16);
#elif (CFG_CHIP_FAMILY == 9850)
    return (ithReadRegH(ITH_PCR_BASECNT_LO_REG) | (uint32_t)ithReadRegH(ITH_PCR_BASECNT_HI_REG) << 16);
#endif
}

typedef enum
{
    STATE_FREE,
    STATE_STOP,
    STATE_PAUSE,
    STATE_RUN
} STC_STATE;

typedef struct STCInfo_TAG
{
    STC_STATE   state;
    int64_t     offset;
    uint64_t    last_pause;
    uint64_t    pause_duration;
    uint64_t    duration;
    uint32_t    stcBaseCountHi;
}STCInfo;

#define STC_MAX_CNT         4
/**
 * Gets base clock of STC. (64-bits)
 *
 * @return the base clock of STC. (64-bits)
 */
uint64_t ithStcGetBaseClock64(STCInfo *pstc_info);

/**
 * Gets extension clock of STC.
 *
 * @return the extension clock of STC.
 */
#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
static inline uint16_t ithStcGetExtClock(void)
{
    return ithReadRegH(ITH_STC_EXTCNT_REG) & ITH_STC_EXTCNT_MASK;
}

/**
 * Gets timer counter of STC.
 *
 * @return the timer counter of STC.
 */
static inline uint16_t ithStcGetTimerCounter(void)
{
    return ithReadRegH(ITH_STC_TIMERCNT_REG) & ITH_STC_TIMERCNT_MASK;
}

/**
 * Sets interrupt number of STC timer.
 *
 * @param num the interrupt number.
 */
static inline void ithStcSetTimerIntrNum(unsigned int num)
{
    ithWriteRegMaskH(ITH_STC_TIMER_INTR_NUM_REG, num, ITH_STC_TIMER_INTR_NUM_MASK);
}

/**
 * Sets video output frequence.
 *
 * @param intValue the integer part.
 * @param fracValue the fraction part.
 */
static inline void ithStcSetVideoOutFreq(unsigned int intValue, unsigned int fracValue)
{
    ithWriteRegH(ITH_STC_VIDEO_OUT_FREQ_REG, (intValue << ITH_STC_VIDEO_OUT_FREQ_INT_BIT) | (fracValue & 0x3));
}
#endif
/** @} */ // end of ith_fpc

/** @defgroup ith_nand NAND
 *  @{
 */

/**
 * NAND page size definition.
 */
typedef enum
{
    ITH_NAND_512    = 0,    ///< 512 bytes
    ITH_NAND_2K     = 1,    ///< 2k
    ITH_NAND_4K     = 2,    ///< 4k
    ITH_NAND_8K     = 3     ///< 8k
} ITHNandPageSize;

/**
 * Gets current NAND page size
 *
 * @return the page size.
 */
static inline ITHNandPageSize ithNandGetPageSize(void)
{
    return (ITHNandPageSize)((ithReadRegA(ITH_NAND_BASE + ITH_NAND_AUTOBOOTCFG_REG) & ITH_NAND_AUTOBOOTCFG_MASK) >> ITH_NAND_AUTOBOOTCFG_BIT);
}

/** @} */ // end of ith_nand

/** @defgroup ith_dpu encryption/descryption
 *  @{
 */
typedef enum
{
    AES_ECB_MODE=0,
    AES_CBC_MODE,
    AES_OFB_MODE,
    AES_CFB_MODE,
    AES_CTR_MODE,
    DES_ECB_MODE,
    DES_CBC_MODE,
    DES_OFB_MODE,
    DES_CFB_MODE,
    DES_CTR_MODE,
    DES3_ECB_MODE,
    DES3_CBC_MODE,
    DES3_OFB_MODE,
    DES3_CFB_MODE,
    DES3_CTR_MODE,
    CSA_MODE,
    CRC_MODE,
    UNKOWN_MODE
}ITH_DPU_MODE;

/**
 * To set the dpu control register by DPU mode and cipher.
 */
void ithDpuClearCtrl(void);

/**
 * To clear interrupt.
 */
void ithDpuClearIntr(void);

/**
 * To set the dpu control register by DPU mode and cipher.
 */
void ithDpuInitCtrl(ITH_DPU_MODE DpuMode);

/**
 * To set the dpu CRC initial value as 0xFFFFFFFF.
 */
void ithDpuInitCrc(void);

/**
 * To enable DPU interrupt function
 */
void ithDpuEnableIntr(void);

/**
 * To disable DPU interrupt function
 */
void ithDpuDisableIntr(void);

/**
 * To set the dpu CRC initial value as 0xFFFFFFFF.
 *
 * @param SrcAddr the source address
 */
void ithDpuSetSrcAddr(unsigned int SrcAddr);

/**
 * To set the dpu CRC result.
 *
 * @param Reg32 the source address
 */
void ithDpuGetCrcValue(unsigned int *Reg32);

/**
 * To set the dpu CRC data value to calculate a new CRC vlaue.
 *
 * @param CrcData the CRC data
 */
void ithDpuSetCrcData(unsigned int CrcData);

/**
 * To set the dpu CRC initial value as 0xFFFFFFFF.
 *
 * @param DstAddr the distnation address
 */
void ithDpuSetDstAddr(unsigned int DstAddr);

/**
 * To set the dpu as encryption mode.
 */
void ithDpuSetEncrypt(void);

/**
 * To set the dpu as descryption mode.
 */
void ithDpuSetDescrypt(void);

/**
 * To fire dpu engine.
 */
void ithDpuFire(void);

/**
 * To set the dpu excution size.
 *
 * @param size the DPU excution size
 */
void ithDpuSetSize(unsigned int size);

/**
 * To set the encryption key.
 *
 * @param key the pointer of key
 * @param len the key length(in bit)
 */
void ithDpuSetKey(uint32_t *key, uint8_t len);

/**
 * To set the encryption vector.
 *
 * @param vector the pointer of vector
 * @param len the vector length(in bit)
 */
void ithDpuSetVector(uint32_t *vector, uint8_t len);

/**
 * Suspends DPU module.
 */
void ithDpuSuspend(void);

/**
 * Resumes DPU module.
 */
void ithDpuResume(void);

/**
 * To wait the dpu engine done.
 *
 * @return true if succeed, false if failed.
 */
bool ithDpuWait(void);

/**
 * Set DPU endian.
 *
 * @param EndianIndex the endian index
 */
void ithDpuSetDpuEndian(unsigned int EndianIndex);

/**
 * Enable DPU clock.
 */
void ithDpuEnableClock(void);

/**
 * Disable DPU clock.
 */
void ithDpuDisableClock(void);

/** @} */ // end of ith_dpu

/** @defgroup ith_dcps Decompress
 *  @{
 */
/**
 * the structure of the decompress status.
 */
typedef struct
{
    uint32_t DcpsSetIndex;      ///< The number that the function "ithDcpsStart()" has been called.
    uint32_t DcpsDoneIndex;     ///< The number that the function "ithDcpsStart()" has been finished.
    uint32_t CmdQueSetIndex;    ///< The number that command queue has been set.
    uint32_t CmdQueDoneIndex;   ///< The number that command queue has been finished.
    uint32_t DcpsStatus;        ///< The status flag for debug
}ITH_DCPS_STATUS, *pITH_DCPS_STATUS;

/**
 * the structure of the decompress parameters.
 */
typedef struct
{
    uint8_t  *srcbuf;           ///< The source buffer address.
    uint8_t  *dstbuf;           ///< The destination buffer address.
    uint32_t srcLen;            ///< The source buffer length.
    uint32_t dstLen;            ///< The destination buffer length.
    uint32_t BlkSize;           ///<
    uint32_t TotalCmdqCount;    ///<
    uint32_t RegDcpsCmdqCnt;    ///<
    uint32_t RegDcpsStatus;     ///<
    uint8_t  IsEnableComQ;      ///< The flag of enabling command queue function.(0 is disable)
}ITH_DCPS_INFO, *pITH_DCPS_INFO;

/**
 * To initialize the decompress driver.
 *
 * @param DcpsInfo
 */
void ithDcpsInit(ITH_DCPS_INFO *DcpsInfo);

/**
 * To Enable decompress interrupt.
 */
void ithDcpsEnIntr(void);

/**
 * To Disable decompress interrupt.
 */
void ithDcpsDisIntr(void);

/**
 * To clear decompress interrupt.
 */
void ithDcpsClearIntr(void);

/**
 * To get the counter that command queue has excuted for decompressing.
 *
 * @param DcpsInfo The value that command queue has excuted for decompressing.
 */
void ithDcpsGetCmdqCount(ITH_DCPS_INFO *DcpsInfo);

/**
 * To get the counter that command queue has excuted for decompressing.
 *
 * @param DcpsDoneLen  The bytes that has decompressed.
 */
void ithDcpsGetDoneLen(uint32_t *DcpsDoneLen);

/**
 * Start to execute decompressing function.
 *
 * @param DcpsInfo the strucure contains all parameters of decompressing
 * @return true if succeed, false if failed.
 */
bool ithDcpsFire(ITH_DCPS_INFO *DcpsInfo);

/**
 * To get the status of decompress.
 *
 * @param DcpsInfo The current decompress status.
 */
void ithDcpsGetStatus(ITH_DCPS_INFO *DcpsInfo);

/**
 * To get the current decompress status.
 *
 * @param DcpsInfo The current value of register "ITH_DCPS_REG_DSR".
 */
void ithDcpsWait(ITH_DCPS_INFO *DcpsInfo);

/**
 * To terminate the decompress driver.
 */
void ithDcpsExit(void);

/**
 * Suspends DCPS module.
 */
void ithDcpsSuspend(void);

/**
 * Resumes DCPS module.
 */
void ithDcpsResume(void);

/**
 * Enable Decompress clock.
 */
void ithDcpsEnableClock(void);

/**
 * Disable DPU clock.
 */
void ithDcpsDisableClock(void);

/** @} */ // end of ith_dcps

/**
 * Interrupt number definition.
 */
typedef enum
{
#if (CFG_CHIP_FAMILY == 9070)
    ITH_INTR_SW0        = 0,    ///< Software (interrupt is invoking when write the software interrupt to 1)
    ITH_INTR_HDMI       = 1,    ///< HDMI / ISP Queue
    ITH_INTR_DMA        = 2,    ///< DMA / DMA Terminal Count /DMA Error
    ITH_INTR_TIMER      = 4,    ///< Timer (for All)
    ITH_INTR_TIMER1     = 5,    ///< Timer 1
    ITH_INTR_TIMER2     = 6,    ///< Timer 2
    ITH_INTR_TIMER3     = 7,    ///< Timer 3
    ITH_INTR_TIMER4     = 8,    ///< Timer 4
    ITH_INTR_TIMER5     = 9,    ///< Timer 5
    ITH_INTR_TIMER6     = 10,   ///< Timer 6
    ITH_INTR_RTC        = 11,   ///< Real time clock
    ITH_INTR_RTCSEC     = 12,   ///< Real time clock second
    ITH_INTR_RTCMIN     = 13,   ///< Real time clock minute
    ITH_INTR_RTCHOUR    = 14,   ///< Real time clock hour
    ITH_INTR_RTCDAY     = 15,   ///< Real time day
    ITH_INTR_RTCALARM   = 16,   ///< Real time alarm
    ITH_INTR_GPIO       = 17,   ///< GPIO
    ITH_INTR_WD         = 18,   ///< Watch dog
    ITH_INTR_I2C        = 19,   ///< I2C
    ITH_INTR_SSP0       = 20,   ///< SSP 0
    ITH_INTR_SSP1       = 21,   ///< SSP 1
    ITH_INTR_UART0      = 22,   ///< UART 0
    ITH_INTR_UART1      = 23,   ///< UART 1
    ITH_INTR_RC         = 24,   ///< Remote Controller
    ITH_INTR_CF         = 25,   ///< CF
    ITH_INTR_MC         = 26,   ///< Memory Controller

    ITH_INTR_SW1        = 32 + 0,   ///< Software (interrupt is invoking when write the software interrupt register to 1) // 0
    ITH_INTR_USB0       = 32 + 1,   ///< USB0
    ITH_INTR_USB1       = 32 + 2,   ///< USB1
    ITH_INTR_MAC        = 32 + 3,   ///< MAC / MAC WOL
    ITH_INTR_MS         = 32 + 4,   ///< Memory Stick
    ITH_INTR_SD         = 32 + 5,   ///< SD/MMC
    ITH_INTR_XD         = 32 + 6,   ///< XD
    ITH_INTR_NAND       = 32 + 8,   ///< NAND
    ITH_INTR_CPU0       = 32 + 9,   ///< CPU0
    ITH_INTR_CPU1       = 32 + 10,  ///< CPU1
    ITH_INTR_CMDQ       = 32 + 11,  ///< Command Queue
    ITH_INTR_FPC        = 32 + 12,  ///< FPC
    ITH_INTR_ISP        = 32 + 13,  ///< ISP
    ITH_INTR_LCD        = 32 + 14,  ///< LCD
    ITH_INTR_I2S        = 32 + 15,  ///< I2S
    ITH_INTR_OPENVG     = 32 + 16,  ///< Open VG
    ITH_INTR_DPU        = 32 + 17,  ///< DPU
    ITH_INTR_DECOMPRESS = 32 + 18,  ///< Decompress
    ITH_INTR_CAPTURE    = 32 + 19,  ///< CCIR601/656 Capture
    ITH_INTR_TSI0       = 32 + 20,  ///< TSI 0 Interface
    ITH_INTR_TSI1       = 32 + 21,  ///< TSI 1 Interface
    ITH_INTR_TSPARSER   = 32 + 22,  ///< TS Parser
#elif (CFG_CHIP_FAMILY == 9910)
    ITH_INTR_SW0        = 0,
    ITH_INTR_HDMI       = 1,
    ITH_INTR_DMA        = 2,
    ITH_INTR_TIMER      = 4, // no used
    ITH_INTR_TIMER1     = 5,
    ITH_INTR_TIMER2     = 6,
    ITH_INTR_TIMER3     = 7,
    ITH_INTR_TIMER4     = 8,
    ITH_INTR_TIMER5     = 9,
    ITH_INTR_TIMER6     = 10, // no used
    ITH_INTR_RTC        = 11,
    ITH_INTR_RTCSEC     = 12,
    ITH_INTR_RTCMIN     = 13,
    ITH_INTR_RTCHOUR    = 14,
    ITH_INTR_RTCDAY     = 15,
    ITH_INTR_RTCALARM   = 16,
    ITH_INTR_GPIO       = 17,
    ITH_INTR_WD         = 18,
    ITH_INTR_I2C        = 19,
    ITH_INTR_SSP0       = 20,
    ITH_INTR_SSP1       = 21,
    ITH_INTR_UART0      = 22,
    ITH_INTR_UART1      = 24,
    ITH_INTR_IR_RX      = 26,
    ITH_INTR_IR_TX      = 27,
    ITH_INTR_MC         = 28,
    ITH_INTR_CODA       = 29,
    ITH_INTR_HDMIRX     = 30,
    ITH_INTR_KB         = 31,

    ITH_INTR_SW1        = 32, // 0
    ITH_INTR_USB0       = 33, // 1
    ITH_INTR_USB1       = 34, // 2
    ITH_INTR_MAC        = 35, // 3
    // 4
    ITH_INTR_SD         = 37, // 5
    ITH_INTR_XD         = 38, // 6
    // 7
    // 8
    ITH_INTR_NAND       = 32 + 8,   ///< NAND
    ITH_INTR_CPU0       = 41, // 9
    ITH_INTR_CPU1       = 42, // 10
    ITH_INTR_CPU2       = 43, // 11
    ITH_INTR_FPC        = 32 + 12,  ///< FPC
    // 12
    ITH_INTR_ISP        = 45, // 13
    ITH_INTR_LCD        = 32 + 14,  ///< LCD
    // 14
    ITH_INTR_I2S        = 47, // 15
    // 16
    ITH_INTR_DPU        = 49, // 17
    ITH_INTR_DECOMPRESS = 50, // 18
    ITH_INTR_CAPTURE    = 51, // 19
    ITH_INTR_TSI0       = 52, // 20
    // 21
    ITH_INTR_TSMUX      = 54, // 22
#elif (CFG_CHIP_FAMILY == 9850)
    ITH_INTR_SW0        = 0,    ///< Software (interrupt is invoking when write the software interrupt to 1)
    ITH_INTR_DMA        = 2,    ///< DMA / DMA Terminal Count /DMA Error
    ITH_INTR_TIMER      = 4,    ///< Timer (for All)
    ITH_INTR_TIMER1     = 5,    ///< Timer 1
    ITH_INTR_TIMER2     = 6,    ///< Timer 2
    ITH_INTR_TIMER3     = 7,    ///< Timer 3
    ITH_INTR_TIMER4     = 8,    ///< Timer 4
    ITH_INTR_TIMER5     = 9,    ///< Timer 5
    ITH_INTR_TIMER6     = 10,   ///< Timer 6
    ITH_INTR_TIMER7     = 11,   ///< Timer 7
    ITH_INTR_TIMER8     = 12,   ///< Timer 8
    ITH_INTR_RTC        = 13,   ///< Real time clock
    ITH_INTR_RTCSEC     = 14,   ///< Real time clock second
    ITH_INTR_RTCMIN     = 15,   ///< Real time clock minute
    ITH_INTR_RTCHOUR    = 16,   ///< Real time clock hour
    ITH_INTR_RTCDAY     = 17,   ///< Real time day
    ITH_INTR_RTCALARM   = 18,   ///< Real time alarm
    ITH_INTR_WD         = 19,   ///< Watch dog
    ITH_INTR_SSP0       = 20,   ///< SSP 0
    ITH_INTR_SSP1       = 21,   ///< SSP 1
    ITH_INTR_UART0      = 22,   ///< UART 0
    ITH_INTR_UART1      = 23,   ///< UART 1
    ITH_INTR_UART2      = 24,   ///< UART 2
    ITH_INTR_UART3      = 25,   ///< UART 3
    //ITH_INTR_RC         = 24,   ///< Remote Controller
    //ITH_INTR_CF         = 25,   ///< CF
    ITH_INTR_MC         = 26,   ///< Memory Controller
    ITH_INTR_GPIO       = 27,   ///< GPIO
    ITH_INTR_WIEGAND0   = 28,   ///< Wiegand0
    ITH_INTR_WIEGAND1   = 29,   ///< Wiegand1
    ITH_INTR_I2C0       = 30,   ///< I2C0
    ITH_INTR_I2C1       = 31,   ///< I2C1

    ITH_INTR_SW1        = 32 + 0,   ///< Software (interrupt is invoking when write the software interrupt register to 1) // 0
    ITH_INTR_USB0       = 32 + 1,   ///< USB0
    ITH_INTR_USB1       = 32 + 2,   ///< USB1
    ITH_INTR_MAC        = 32 + 3,   ///< MAC / MAC WOL
    ITH_INTR_SD         = 32 + 5,   ///< SD/MMC
    ITH_INTR_XD         = 32 + 6,   ///< XD
    ITH_INTR_NAND       = 32 + 8,   ///< NAND
    ITH_INTR_CPU0       = 32 + 9,   ///< CPU0
    ITH_INTR_CMDQ       = 32 + 11,  ///< Command Queue
    ITH_INTR_FPC        = 32 + 12,  ///< FPC
    ITH_INTR_LCD        = 32 + 14,  ///< LCD
    ITH_INTR_I2S        = 32 + 15,  ///< I2S
    ITH_INTR_2D         = 32 + 16,  ///< 2D
    ITH_INTR_DPU        = 32 + 17,  ///< DPU
    ITH_INTR_DECOMPRESS = 32 + 18,  ///< Decompress
    ITH_INTR_CAPTURE    = 32 + 19,  ///< CCIR601/656 Capture
    ITH_INTR_TSI0       = 32 + 20,  ///< TSI 0 Interface
    ITH_INTR_TSI1       = 32 + 21,  ///< TSI 1 Interface
    ITH_INTR_VIDEO      = 32 + 23,  ///< Video
    ITH_INTR_TSM        = 32 + 25,  ///< TSM
    ITH_INTR_IRRX       = 32 + 26,  ///< Remote IR RX
    ITH_INTR_IRTX       = 32 + 27   ///< Remote IR TX
#else
    ITH_INTR_SW0		    = 0,	///< Software (interrupt is invoking when write the software interrupt to 1)
    ITH_INTR_MC         = 1,	///< Memory Controller
    ITH_INTR_DMA        = 2,	///< DMA / DMA Terminal Count /DMA Error
    ITH_INTR_TIMER		= 4,	///< Timer (for All)
    ITH_INTR_TIMER1 	    = 5,	///< Timer 1
    ITH_INTR_TIMER2     	= 6,	///< Timer 2
    ITH_INTR_TIMER3  	= 7,	///< Timer 3
    ITH_INTR_TIMER4     	= 8,	///< Timer 4
    ITH_INTR_TIMER5     	= 9,	///< Timer 5
    ITH_INTR_TIMER6     	= 10,	///< Timer 6
    ITH_INTR_TIMER7 	    = 11,	///< Timer 7
    ITH_INTR_TIMER8 	    = 12,	///< Timer 8
    ITH_INTR_RTC		    = 13,	///< Real time clock
    ITH_INTR_RTCSEC     	= 14,	///< Real time clock second
    ITH_INTR_RTCMIN     	= 15,	///< Real time clock minute
    ITH_INTR_RTCHOUR    	= 16,	///< Real time clock hour
    ITH_INTR_RTCDAY 	    = 17,	///< Real time day
    ITH_INTR_RTCALARM	= 18,	///< Real time alarm
    ITH_INTR_WD 		    = 19,	///< Watch dog
    ITH_INTR_CPU0       = 20,	///< CPU0
    ITH_INTR_CPU1       = 21,	///< CPU1
    ITH_INTR_ALTCPU     = 23,	///< ALTCPU
    ITH_INTR_CMDQ       = 24,	///< Command Queue
    ITH_INTR_LCD        = 25,	///< LCD
    ITH_INTR_2D         = 26,	///< 2D
    ITH_INTR_GPIO       = 27,	///< GPIO
    ITH_INTR_VIDEO      = 28,	///< Video
    ITH_INTR_JPEG       = 29,	///< Jpeg
    ITH_INTR_I2C0       = 30,	///< I2C0
    ITH_INTR_I2C1		= 31,	///< I2C1

    ITH_INTR_SW1		    = 32 + 0,	///< Software (interrupt is invoking when write the software interrupt register to 1) // 0
    ITH_INTR_USB0		= 32 + 1,	///< USB0
    ITH_INTR_USB1		= 32 + 2,	///< USB1
    ITH_INTR_MAC		    = 32 + 3,	///< MAC / MAC WOL
    ITH_INTR_WIEGAND0   = 32 + 4,	///< Wiegand0
    ITH_INTR_WIEGAND1   = 32 + 5,	///< Wiegand1
    ITH_INTR_UART0      = 32 + 6,	///< UART 0
    ITH_INTR_UART1      = 32 + 7,	///< UART 1
    ITH_INTR_UART2      = 32 + 8,	///< UART 2
    ITH_INTR_UART3      = 32 + 9,	///< UART 3
    ITH_INTR_UART4      = 32 + 10,	///< UART 4
    ITH_INTR_UART5      = 32 + 11,	///< UART 5
    ITH_INTR_ISP        = 32 + 14,	///< ISP
    ITH_INTR_IQ		    = 32 + 15,	///< IQ
    ITH_INTR_I2S        = 32 + 16,	///< I2S
    ITH_INTR_DPU		    = 32 + 17,	///< DPU
    ITH_INTR_DECOMPRESS = 32 + 18,	///< Decompress
    ITH_INTR_CAPTURE	    = 32 + 19,	///< CCIR601/656 Capture
    ITH_INTR_TSI0		= 32 + 20,	///< TSI 0 Interface
    ITH_INTR_TSI1		= 32 + 21,	///< TSI 1 Interface
    ITH_INTR_TSM        = 32 + 22,	///< TSM
    ITH_INTR_SARADC     = 32 + 23,	///< TSM
    ITH_INTR_SD0        = 32 + 24,	///< SD/MMC 0
    ITH_INTR_SD1        = 32 + 25,	///< SD/MMC 1
    ITH_INTR_IRRX		= 32 + 26,	///< Remote IR RX
    ITH_INTR_IRTX		= 32 + 27,	///< Remote IR TX
    ITH_INTR_SSP0       = 32 + 28,	///< SSP 0
    ITH_INTR_SSP1       = 32 + 29,	///< SSP 1
    ITH_INTR_AXISPI     = 32 + 30,	///< AXISPI
    ITH_INTR_MIPI       = 32 + 31	///< MIPI
#endif // (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
} ITHIntr;

/**
 * Interrupt trigger mode definition.
 */
typedef enum
{
    ITH_INTR_LEVEL  = 0, ///< Level-trigger mode
    ITH_INTR_EDGE   = 1  ///< Edge-trigger mode
} ITHIntrTriggerMode;

typedef enum
{
    ITH_INTR_HIGH_RISING    = 0,    ///< Active-high level trigger or rising-edge trigger
    ITH_INTR_LOW_FALLING    = 1     ///< Active-low level trigger or falling-edge trigger
} ITHIntrTriggerLevel;

/**
 * Interrupt handler.
 *
 * @arg Custom argument.
 */
typedef void (*ITHIntrHandler)(void* arg);

/**
 * Initializes interrupt module.
 */
void ithIntrInit(void);

/**
 * Resets interrupt module.
 */
void ithIntrReset(void);

/**
 * Enables specified IRQ.
 *
 * @param intr The IRQ.
 */
static inline void ithIntrEnableIrq(ITHIntr intr)
{
    ithEnterCritical();

    if (intr < 32)
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ1_EN_REG, intr);
    else
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ2_EN_REG, intr - 32);

    ithExitCritical();
}

/**
 * Disables specified IRQ.
 *
 * @param intr The IRQ.
 */
static inline void ithIntrDisableIrq(ITHIntr intr)
{
    ithEnterCritical();

    if (intr < 32)
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ1_EN_REG, intr);
    else
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ2_EN_REG, intr - 32);

    ithExitCritical();
}

/**
 * Clears specified IRQ.
 *
 * @param intr The IRQ.
 */
static inline void ithIntrClearIrq(ITHIntr intr)
{
    if (intr < 32)
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ1_CLR_REG, intr);
    else
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ2_CLR_REG, intr - 32);
}

/**
 * Gets status of IRQ.
 *
 * @param intr1 The IRQ status 1.
 * @param intr2 The IRQ status 2.
 */
static inline void ithIntrGetStatusIrq(uint32_t* intr1, uint32_t* intr2)
{
    *intr1 = ithReadRegA(ITH_INTR_BASE + ITH_INTR_IRQ1_STATUS_REG);
    *intr2 = ithReadRegA(ITH_INTR_BASE + ITH_INTR_IRQ2_STATUS_REG);
}

/**
 * Sets trigger mode of IRQ.
 *
 * @param intr The IRQ.
 * @param mode The trigger mode.
 */
void ithIntrSetTriggerModeIrq(ITHIntr intr, ITHIntrTriggerMode mode);

/**
 * Sets trigger level of IRQ.
 *
 * @param intr The IRQ.
 * @param level The trigger level.
 */
void ithIntrSetTriggerLevelIrq(ITHIntr intr, ITHIntrTriggerLevel level);

/**
 * Registers IRQ handler.
 *
 * @param intr The IRQ.
 * @param handler The callback function.
 * @param arg Custom argument to pass to handler.
 */
void ithIntrRegisterHandlerIrq(ITHIntr intr, ITHIntrHandler handler, void* arg);

/**
 * Sets software IRQ.
 *
 * @param num The IRQ0 or IRQ1.
 */
static inline void ithIntrSetSwIrq(int num)
{
    if (num)
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ2_SWINTR_REG, ITH_INTR_SWINT_BIT);
    else
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ1_SWINTR_REG, ITH_INTR_SWINT_BIT);
}

/**
 * Clears software IRQ.
 *
 * @param num The IRQ0 or IRQ1.
 */
static inline void ithIntrClearSwIrq(int num)
{
    if (num)
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ2_SWINTR_REG, ITH_INTR_SWINT_BIT);
    else
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_IRQ1_SWINTR_REG, ITH_INTR_SWINT_BIT);
}

/**
 * Dispatches IRQ to handlers.
 */
void ithIntrDoIrq(void);

/**
 * Enables specified FIQ.
 *
 * @param intr The FIQ.
 */
static inline void ithIntrEnableFiq(ITHIntr intr)
{
    if (intr < 32)
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_EN_REG, intr);
    else
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_EN_REG, intr - 32);
}

/**
 * Disables specified FIQ.
 *
 * @param intr The FIQ.
 */
static inline void ithIntrDisableFiq(ITHIntr intr)
{
    if (intr < 32)
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_EN_REG, intr);
    else
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_EN_REG, intr - 32);
}

/**
 * Clears specified FIQ.
 *
 * @param intr The FIQ.
 */
static inline void ithIntrClearFiq(ITHIntr intr)
{
    if (intr < 32)
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_CLR_REG, intr);
    else
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_CLR_REG, intr - 32);
}

/**
 * Gets status of FIQ.
 *
 * @param intr1 The FIQ status 1.
 * @param intr2 The FIQ status 2.
 */
static inline void ithIntrGetStatusFiq(uint32_t* intr1, uint32_t* intr2)
{
    *intr1 = ithReadRegA(ITH_INTR_BASE + ITH_INTR_FIQ1_STATUS_REG);
    *intr2 = ithReadRegA(ITH_INTR_BASE + ITH_INTR_FIQ2_STATUS_REG);
}

/**
 * Sets trigger mode of FIQ.
 *
 * @param intr The FIQ.
 * @param mode The trigger mode.
 */
void ithIntrSetTriggerModeFiq(ITHIntr intr, ITHIntrTriggerMode mode);

/**
 * Sets trigger level of FIQ.
 *
 * @param intr The FIQ.
 * @param level The trigger level.
 */
void ithIntrSetTriggerLevelFiq(ITHIntr intr, ITHIntrTriggerLevel level);

/**
 * Registers FIQ handler.
 *
 * @param intr The IRQ.
 * @param handler The callback function.
 * @param arg Custom argument to pass to handler.
 */
void ithIntrRegisterHandlerFiq(ITHIntr intr, ITHIntrHandler handler, void* arg);

/**
 * Sets software FIQ.
 *
 * @param num The FIQ0 or FIQ1.
 */
static inline void ithIntrSetSwFiq(int num)
{
    if (num)
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_SWINTR_REG, ITH_INTR_SWINT_BIT);
    else
        ithSetRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_SWINTR_REG, ITH_INTR_SWINT_BIT);
}

/**
 * Clears software FIQ.
 *
 * @param num The FIQ0 or FIQ1.
 */
static inline void ithIntrClearSwFiq(int num)
{
    if (num)
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ2_SWINTR_REG, ITH_INTR_SWINT_BIT);
    else
        ithClearRegBitA(ITH_INTR_BASE + ITH_INTR_FIQ1_SWINTR_REG, ITH_INTR_SWINT_BIT);
}

/**
 * Dispatches FIQ to handlers.
 */
void ithIntrDoFiq(void);

/**
 * Suspends interrupt module.
 */
void ithIntrSuspend(void);

/**
 * Resumes from suspends for interrupt module.
 */
void ithIntrResume(void);

/**
 * Print interrupt information.
 */
void ithIntrStats(void);

/** @} */ // end of ith_intr

/** @defgroup ith_rtc RTC
 *  @{
 */

/**
 * RTC control definition.
 */
typedef enum
{
    ITH_RTC_EN              = 0,    ///< RTC enable
    ITH_RTC_INTR_SEC        = 1,    ///< RTC auto alarm per second
    ITH_RTC_INTR_MIN        = 2,    ///< RTC auto alarm per minute
    ITH_RTC_INTR_HOUR       = 3,    ///< RTC auto alarm per hour
    ITH_RTC_INTR_DAY        = 4,    ///< RTC auto alarm per day
    ITH_RTC_ALARM_INTR      = 5,    ///< RTC alarm interrupt
    ITH_RTC_DAY_ALARM_INTR  = 6,    ///< RTC day alarm interrupt
    ITH_RTC_WEEK_ALARM_INTR = 7,    ///< RTC week alarm interrupt
    ITH_RTC_COUNTER_LOAD    = 8,    ///< RTC counter load
    ITH_RTC_PWREN_ALARM_SEL = 15,   ///< PWREN Alarm Type
    ITH_RTC_PWREN_CTRL1     = 16,   ///< PWREN Alarm Control
    ITH_RTC_SW_POWEROFF     = 17,   ///< Software power down (auto set zero)
    ITH_RTC_PWREN_IO_DIR    = 21,   ///< PWREN pin driving strength
    ITH_RTC_PWREN_GPIO      = 24,   ///< PWREN GPIO Data
    ITH_RTC_RESET           = 31    ///< Asynchronous reset
} ITHRtcCtrl;

/**
 * RTC interrupt definition.
 */
typedef enum
{
    ITH_RTC_SEC     = 0,    ///< Indicate that rtc_sec interrupt has occurred.
    ITH_RTC_MIN     = 1,    ///< Indicate that rtc_min interrupt has occurred.
    ITH_RTC_HOUR    = 2,    ///< Indicate that rtc_hour interrupt has occurred.
    ITH_RTC_DAY     = 3,    ///< Indicate that rtc_day interrupt has occurred.
    ITH_RTC_ALARM   = 4,    ///< Indicate that rtc_alarm interrupt has occurred.

    ITH_RTC_MAX_INTR
} ITHRtcIntr;

/**
 * RTC power enable I/O selection definition.
 */
typedef enum
{
    ITH_RTC_PWREN   = 0,    ///< PWREN
    ITH_RTC_VCC_OK  = 1,    ///< VCC_OK
    ITH_RTC_INTR    = 2,    ///< RTC interrupt
    ITH_RTC_GPIO    = 3     ///< GPIO
} ITHRtcPowerEnableIoSelection;

/**
 * RTC power enable I/O selection definition.
 */
typedef enum
{
    ITH_RTC_DIV_SRC_INNER_12MHZ   = 0,    ///< divider source is internal 12Mhz clock
    ITH_RTC_DIV_SRC_EXT_32KHZ     = 1,    ///< divider source is external 32kHz clock
} ITHRtcClockSource;

/**
 * Enables specified RTC control.
 *
 * @param ctrl the control to enable.
 */
static inline void ithRtcCtrlEnable(ITHRtcCtrl ctrl)
{
    ithSetRegBitA(ITH_RTC_BASE + ITH_RTC_CR_REG, ctrl);
}

/**
 * Disables specified RTC control.
 *
 * @param ctrl the control to disable.
 */
static inline void ithRtcCtrlDisable(ITHRtcCtrl ctrl)
{
    ithClearRegBitA(ITH_RTC_BASE + ITH_RTC_CR_REG, ctrl);
}

/**
 * Clears specified RTC interrupt.
 *
 * @param intr the interrupt to clear.
 */
static inline void ithRtcClearIntr(ITHRtcIntr intr)
{
    ithClearRegBitA(ITH_RTC_BASE + ITH_RTC_INTRSTATE_REG, intr);
}

/**
 * Gets the state of RTC interrupt.
 *
 * @return the state of RTC interrupt.
 */
static inline uint32_t ithRtcGetIntrState(void)
{
    return ithReadRegA(ITH_RTC_BASE + ITH_RTC_INTRSTATE_REG);
}

/**
 * Sets the RTC power enable I/O selection.
 *
 * @param sel the RTC power enable I/O selection.
 */
static inline void ithRtcSetPowerEnableIoSelection(ITHRtcPowerEnableIoSelection sel)
{
    ithWriteRegMaskA(ITH_RTC_BASE + ITH_RTC_PWREN_IOSEL_REG, sel << ITH_RTC_PWREN_IOSEL_BIT, ITH_RTC_PWREN_IOSEL_MASK);
}

/**
 * Set RTC source.
 *
 * @param clkSrc The clock source.
 */
void ithRtcSetDivSrc(ITHRtcClockSource clkSrc);

/**
 * Initializes RTC module.
 *
 * @param extClk The frequency of external clock.
 */
void ithRtcInit(unsigned long extClk);

/**
 * Enables RTC module.
 *
 * @return First booting or not.
 */
bool ithRtcEnable(void);

/**
 * Gets RTC time.
 *
 * @return the RTC time.
 */
unsigned long ithRtcGetTime(void);

/**
 * Sets RTC time.
 *
 * @param t the RTC time.
 */
void ithRtcSetTime(unsigned long t);

/**
 * Gets RTC state.
 *
 * @return the RTC state.
 */
static inline uint8_t ithRtcGetState(void)
{
    return (ithReadRegA(ITH_RTC_BASE + ITH_RTC_STATE_REG) & ITH_RTC_STATE_MASK) >> ITH_RTC_STATE_BIT;
}

/**
 * Sets RTC state.
 *
 * @return the RTC state.
 */
static inline void ithRtcSetState(uint8_t state)
{
    ithWriteRegMaskA(ITH_RTC_BASE + ITH_RTC_STATE_REG, state << ITH_RTC_STATE_BIT, ITH_RTC_STATE_MASK);
}

/** @} */ // end of ith_rtc

/** @defgroup ith_uart UART
 *  @{
 */

/**
 * UART port definition.
 */
typedef enum
{
    ITH_UART0   = ITH_UART0_BASE,   ///< UART port #0
    ITH_UART1   = ITH_UART1_BASE,   ///< UART port #1
    ITH_UART2   = ITH_UART2_BASE,   ///< UART port #2
    ITH_UART3   = ITH_UART3_BASE,   ///< UART port #3
} ITHUartPort;

/**
 * UART mode definition
 */
typedef enum
{
    ITH_UART_DEFAULT    = 0x0,   ///< UART TX/RX mode
    ITH_UART_SIR        = 0x1,   ///< SIR mode
    ITH_UART_FIR        = 0x2,   ///< FIR mode
    ITH_UART_TX         = 0x3    ///< UART TX only mode
} ITHUartMode;

/**
 * UART parity definition
 */
typedef enum
{
    ITH_UART_NONE   = 0,   ///< None
    ITH_UART_ODD    = 1,   ///< Odd
    ITH_UART_EVEN   = 2,   ///< Even
    ITH_UART_MARK   = 3,   ///< Mark
    ITH_UART_SPACE  = 4    ///< Space
} ITHUartParity;

/**
 * UART FIFO depth definition
 */
typedef enum
{
    ITH_UART_FIFO16     = 0x1,  ///< TX/RX FIFOs are 16-byte deep
    ITH_UART_FIFO32     = 0x2,  ///< TX/RX FIFOs are 32-byte deep
    ITH_UART_FIFO64     = 0x4,  ///< TX/RX FIFOs are 64-byte deep
    ITH_UART_FIFO128    = 0x8   ///< TX/RX FIFOs are 128-byte deep
} ITHUartFifoDepth;

/**
 * UART FIFO control definition
 */
typedef enum
{
    ITH_UART_FIFO_EN        = 0,    ///< Set this bit to logic 1 enables both transmit and receive FIFOs (And Status FIFO)
    ITH_UART_DMA            = 3     ///< This bit selects the UART DMA mode
} ITHUartFifoCtrl;

/**
 * UART trigger level of the TX/RX FIFO interrupt
 */
typedef enum
{
    ITH_UART_TRGL0  = 0x0,  ///< FIFO Trigger Level0
    ITH_UART_TRGL1  = 0x1,  ///< FIFO Trigger Level1
    ITH_UART_TRGL2  = 0x2,  ///< FIFO Trigger Level2
    ITH_UART_TRGL3  = 0x3   ///< FIFO Trigger Level3
} ITHUartTriggerLevel;

/**
 * UART interrupt definition
 */
typedef enum
{
    ITH_UART_RX_READY       = 0,   ///< Receiver Data Available
    ITH_UART_TX_READY       = 1,   ///< THR Empty
    ITH_UART_RECV_STATUS    = 2,   ///< Receiver Line Status
    ITH_UART_MODEM_STATUS   = 3    ///< MODEM Status
} ITHUartIntr;

/**
 * Sets UART mode.
 *
 * @param port The UART port
 * @param mode The UART mode
 * @param txPin The UART TX GPIO pin. for UART1 only.
 * @param rxPin The UART RX GPIO pin.
 */
void ithUartSetMode(
    ITHUartPort port,
    ITHUartMode mode,
    unsigned int txPin, // UART1 only
    unsigned int rxPin);

/**
 * Only sets UART parity.
 *
 * @param port The UART port.
 * @param parity The UART parity.
 * @param stop The UART stop bit.
 * @param len The UART word length.
 */
void ithUartSetParity(
    ITHUartPort port,
    ITHUartParity parity,
    unsigned int stop,
    unsigned int len);

/**
 * Only sets UART baudrate.
 *
 * @param port The UART port.
 * @param baud The UART baudrate.
 */
void ithUartSetBaudRate(  
	ITHUartPort port,
    unsigned int baud);

/**
 * Resets UART.
 *
 * @param port The UART port.
 * @param baud The UART baudrate.
 * @param parity The UART parity.
 * @param stop The UART stop bit.
 * @param len The UART word length.
 */
void ithUartReset(
    ITHUartPort port,
    unsigned int baud,
    ITHUartParity parity,
    unsigned int stop,
    unsigned int len);

/**
 * Enables specified FIFO controls.
 *
 * @param port The UART port.
 * @param ctrl the controls to enable.
 */
void ithUartFifoCtrlEnable(ITHUartPort port, ITHUartFifoCtrl ctrl);

/**
 * Disables specified FIFO controls.
 *
 * @param port The UART port.
 * @param ctrl the controls to disable.
 */
void ithUartFifoCtrlDisable(ITHUartPort port, ITHUartFifoCtrl ctrl);

/**
 * Sets UART TX interrupt trigger level.
 *
 * @param port The UART port
 * @param level The UART TX trigger level
 */
void ithUartSetTxTriggerLevel(ITHUartPort port, ITHUartTriggerLevel level);

/**
 * Sets UART RX interrupt trigger level.
 *
 * @param port The UART port
 * @param level The UART RX trigger level
 */
void ithUartSetRxTriggerLevel(ITHUartPort port, ITHUartTriggerLevel level);
	

/**
 * Whether RX is ready.
 *
 * @return the result of RX is ready or not
 */
static inline ITHUartFifoDepth ithUartGetFifoDepth(ITHUartPort port)
{
    return (ITHUartFifoDepth)((ithReadRegA(port + ITH_UART_FEATURE_REG) & ITH_UART_FIFO_DEPTH_MASK) >> ITH_UART_FIFO_DEPTH_BIT);
}

/**
 * Whether RX is ready.
 *
 * @return the result of RX is ready or not
 */
static inline bool ithUartIsRxReady(ITHUartPort port)
{
    return (ithReadRegA(port + ITH_UART_LSR_REG) & ITH_UART_LSR_DR) == ITH_UART_LSR_DR;
}

/**
 * Gets a character from UART RX.
 *
 * @param port The UART port.
 */
static inline char ithUartGetChar(ITHUartPort port)
{
    return (char)ithReadRegA(port + ITH_UART_RBR_REG);
}

/**
 * Whether TX FIFO is empty.
 *
 * @return the result of TX FIFO is empty or not
 */
static inline bool ithUartIsTxEmpty(ITHUartPort port)
{
    return (ithReadRegA(port + ITH_UART_LSR_REG) & ITH_UART_LSR_THRE) == ITH_UART_LSR_THRE;
}

static inline bool ithUartIsTxWIthFifoEmpty(ITHUartPort port)
{
    return (ithReadRegA(port + ITH_UART_LSR_REG) & ITH_UART_LSR_TRANS_EMPTY) == ITH_UART_LSR_TRANS_EMPTY;
}


/**
 * Whether TX FIFO is full.
 *
 * @return the result of TX FIFO is full or not
 */
static inline bool ithUartIsTxFull(ITHUartPort port)
{
    return (ithReadRegA(port + ITH_UART_IIR_REG) & ITH_UART_IIR_TXFIFOFULL) == ITH_UART_IIR_TXFIFOFULL;
}

/**
 * Puts character to UART TX.
 *
 * @param port The UART port.
 * @param c The character.
 */
static inline void ithUartPutChar(ITHUartPort port, char c)
{
    ithWriteRegA(port + ITH_UART_THR_REG, c);
}

/**
 * Enables UART interrupt.
 *
 * @param port The UART port.
 * @param intr The UART interrupt.
 */
static inline void ithUartEnableIntr(ITHUartPort port, ITHUartIntr intr)
{
    ithSetRegBitA(port + ITH_UART_IER_REG, intr);
}

/**
 * Disables UART interrupt.
 *
 * @param port The UART port.
 * @param intr The UART interrupt.
 */
static inline void ithUartDisableIntr(ITHUartPort port, ITHUartIntr intr)
{
    ithClearRegBitA(port + ITH_UART_IER_REG, intr);
}

/**
 * Clears UART interrupt.
 *
 * @param port The UART port.
 */
static inline uint32_t ithUartClearIntr(ITHUartPort port)
{
    return ithReadRegA(port + ITH_UART_IIR_REG);
}

/**
 * Enables UART DMA MODE2.
 *
 * @param port The UART port.
 */
static inline void ithUartEnableDmaMode2(ITHUartPort port)
{
    ithWriteRegMaskA(port + ITH_UART_MCR_REG, ITH_UART_MCR_DMAMODE2, ITH_UART_MCR_DMAMODE2);
}

/**
 * Disables UART DMA MODE2.
 *
 * @param port The UART port.
 */
static inline void ithUartDisableDmaMode2(ITHUartPort port)
{
    ithWriteRegMaskA(port + ITH_UART_MCR_REG, 0, ITH_UART_MCR_DMAMODE2);
}

/** @} */ // end of ith_uart

/** @defgroup ith_timer Timer
 *  @{
 */

/** @defgroup ith_rs485 RS485
 *  @{
 */

/**
 * RS485 port definition.
 */
typedef enum
{
    ITH_RS485_0   = 0,   ///< RS485 port #0
    ITH_RS485_1   = 1,   ///< RS485 port #1  UART1 using for debug , so RS485 didn`t use it.  
    ITH_RS485_2   = 2,   ///< RS485 port #2
    ITH_RS485_3   = 3,   ///< RS485 port #3
    ITH_RS485_4   = 4   ///< RS485 port #4   for softUART
} ITHRS485Port;

/** @} */ // end of ith_rs485

/** @defgroup ith_IoEx IoEx
 *  @{
 */

/**
 * Ioex port definition.
 */
 
#define GPIO_MAX_MAPPING_COUNT		16
typedef struct
{
    uint8_t MappingCount;							 ///< MappingCount
    uint8_t MappingGPIONum[GPIO_MAX_MAPPING_COUNT];  ///< MappingGPIONum
	uint8_t LevelTrigger;
	uint8_t EdgeTrigger;
	uint8_t BothEdge;
	uint8_t SigleEdge;
	uint8_t RisingTrigger;
	uint8_t FallingTrigger;
} ITHIOEXConfig;

/** @} */ // end of ith_IoEx



/**
 * IIC mode definition.
 */
typedef enum IIC_MODE_TAG
{
    SLAVE_MODE,
    MASTER_MODE
} IIC_MODE;
/** @} *


/**
 * Timer definition.
 */
typedef enum
{
    ITH_TIMER1 = 0, ///< Timer #1
    ITH_TIMER2 = 1, ///< Timer #2
    ITH_TIMER3 = 2, ///< Timer #3
    ITH_TIMER4 = 3, ///< Timer #4
    ITH_TIMER5 = 4, ///< Timer #5
    ITH_TIMER6 = 5, ///< Timer #6
    ITH_TIMER7 = 6, ///< Timer #7
    ITH_TIMER8 = 7  ///< Timer #8
} ITHTimer;

/**
 * Timer control definition.
 */
typedef enum
{
    ITH_TIMER_EN        = 0,    ///< Timer enable
    ITH_TIMER_EXTCLK    = 1,    ///< Timer clock source use external
    ITH_TIMER_UPCOUNT   = 2,    ///< Timer up or down count
    ITH_TIMER_ONESHOT   = 3,    ///< Timer one shot
    ITH_TIMER_PERIODIC  = 4,    ///< Timer periodic mode
    ITH_TIMER_PWM       = 5,    ///< PWM on Timer
    ITH_TIMER_EN64      = 6     ///< Merge Timers as 64-bit timer
} ITHTimerCtrl;

/**
 * Resets specified timer.
 *
 * @param timer the timer to reset.
 */
void ithTimerReset(ITHTimer timer);

/**
 * Enables specified timer control.
 *
 * @param timer the timer to enable.
 * @param ctrl the control to enable.
 */
void ithTimerCtrlEnable(ITHTimer timer, ITHTimerCtrl ctrl);

/**
 * Disables specified timer control.
 *
 * @param timer the timer to disable.
 * @param ctrl the control to disable.
 */
void ithTimerCtrlDisable(ITHTimer timer, ITHTimerCtrl ctrl);

/**
 * Enables specified timer.
 *
 * @param timer the timer to enable.
 */
static inline void ithTimerEnable(ITHTimer timer)
{
    ithTimerCtrlEnable(timer, ITH_TIMER_EN);
}

/**
 * Disables specified timer.
 *
 * @param timer the timer to disable.
 */
static inline void ithTimerDisable(ITHTimer timer)
{
    ithTimerCtrlDisable(timer, ITH_TIMER_EN);
}

/**
 * Gets the counter of specified timer.
 *
 * @param timer the timer to get.
 * @return the timer's counter.
 */
static inline uint32_t ithTimerGetCounter(ITHTimer timer)
{
    return ithReadRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_CNT_REG);
}

/**
 * Sets the counter of specified timer.
 *
 * @param timer the timer to set.
 * @param count the count value to set.
 */
static inline void ithTimerSetCounter(ITHTimer timer, uint32_t count)
{
    ithWriteRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_CNT_REG, count);
}

/**
 * Sets the load of specified timer.
 *
 * @param timer the timer to set.
 * @param load the load value to set.
 */
static inline void ithTimerSetLoad(ITHTimer timer, uint32_t load)
{
    ithWriteRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_LOAD_REG, load);
}

/**
 * Gets the load value of specified timer.
 *
 * @param timer the timer to get.
 * @return the load value.
 */
static inline uint32_t ithTimerGetLoad(ITHTimer timer)
{
    return ithReadRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_LOAD_REG);
}

/**
 * Gets the time value of specified timer.
 *
 * @param timer the timer to get.
 * @return the time value (us).
 */
static inline uint32_t ithTimerGetTime(ITHTimer timer)
{
    return (uint64_t)ithReadRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_CNT_REG) * 1000000 / ithGetBusClock();
}

/**
 * Sets the timeout value of specified timer.
 *
 * @param timer the timer to set.
 * @param us the timeout value (us).
 */
static inline void ithTimerSetTimeout(ITHTimer timer, unsigned int us)
{
    ithTimerSetCounter(timer, (uint64_t)ithGetBusClock() * us / 1000000);
}

/**
 * Sets the match value of specified timer.
 *
 * @param timer the timer to set.
 * @param count the count value.
 */
static inline void ithTimerSetMatch(ITHTimer timer, uint32_t count)
{
    ithWriteRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_MATCH1_REG, count);
    ithWriteRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_MATCH2_REG, count);
}

/**
 * Sets the PWM match value of specified timer.
 *
 * @param timer the timer to set.
 * @param v1 the count1 value.
 * @param v2 the count2 value.
 */
static inline void ithTimerSetPwmMatch(ITHTimer timer, uint32_t v1, uint32_t v2)
{
    ithWriteRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_MATCH1_REG, v1);
    ithWriteRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_MATCH2_REG, v2);
}

/**
 * Clears interrupt of specified timer.
 *
 * @param timer the timer to clear.
 */
static inline void ithTimerClearIntr(ITHTimer timer)
{
    ithWriteRegA(ITH_TIMER_BASE + ITH_TIMER_INTRSTATE_REG, 0x7 << (timer * 4));
}

/**
 * Gets interrupt state of specified timer.
 *
 * @return interrupt state.
 */
static inline uint32_t ithTimerGetIntrState(void)
{
    return ithReadRegA(ITH_TIMER_BASE + ITH_TIMER_INTRSTATE_REG);
}

/**
 * Suspends timer module.
 */
void ithTimerSuspend(void);

/**
 * Resumes from suspend mode for timer module.
 */
void ithTimerResume(void);

/** @} */ // end of ith_timer

/** @defgroup ith_wd Watch Dog
 *  @{
 */

/**
 * Watch dog control definition.
 */
typedef enum
{
    ITH_WD_EN       = 0,    ///< The Watch Dog Timer enable
    ITH_WD_RESET    = 1,    ///< The Watch Dog Timer system reset enable
    ITH_WD_INTR     = 2,    ///< The Watch Dog Timer system interrupt enable
    ITH_WD_EXT      = 3,    ///< The Watch Dog Timer external signal enable
    ITH_WD_CLOCK    = 4,    ///< The Watch Dog Timer clock source
} ITHWatchDogCtrl;

/**
 * Enables specified watch dog control.
 *
 * @param ctrl the control to enable.
 */
static inline void ithWatchDogCtrlEnable(ITHWatchDogCtrl ctrl)
{
    ithSetRegBitA(ITH_WD_BASE + ITH_WD_CR_REG, ctrl);
}

/**
 * Disables specified watch dog control.
 *
 * @param ctrl the control to disable.
 */
static inline void ithWatchDogCtrlDisable(ITHWatchDogCtrl ctrl)
{
    ithClearRegBitA(ITH_WD_BASE + ITH_WD_CR_REG, ctrl);
}

/**
 * Enables watch dog.
 */
static inline void ithWatchDogEnable(void)
{
    ithWatchDogCtrlEnable(ITH_WD_EN);
}

/**
 * Disables watch dog.
 */
static inline void ithWatchDogDisable(void)
{
    ithWatchDogCtrlDisable(ITH_WD_EN);
    ithWatchDogCtrlDisable(ITH_WD_RESET);
}

/**
 * Restarts watch dog.
 */
static inline void ithWatchDogRestart(void)
{
    ithWriteRegA(ITH_WD_BASE + ITH_WD_RESTART_REG, ITH_WD_AUTORELOAD);
}

/**
 * Sets the reload value of watch dog.
 */
static inline void ithWatchDogSetReload(uint32_t count)
{
    ithWriteRegA(ITH_WD_BASE + ITH_WD_LOAD_REG, count);
}

/**
 * Gets the reload value of watch dog.
 */
static inline uint32_t ithWatchDogGetReload(void)
{
    return ithReadRegA(ITH_WD_BASE + ITH_WD_LOAD_REG);
}

/**
 * Gets the counter of watch dog.
 */
static inline uint32_t ithWatchDogGetCounter(void)
{
    return ithReadRegA(ITH_WD_BASE + ITH_WD_COUNTER_REG);
}

/**
 * Sets the timeout of watch dog (ms).
 *
 * @param ms the timeout value (ms).
 */
static inline void ithWatchDogSetTimeout(unsigned int ms)
{
    ithWatchDogSetReload((uint64_t)ithGetBusClock() * ms / 1000);
}

/**
 * Suspends watch dog module.
 */
void ithWatchDogSuspend(void);

/**
 * Resumes from suspend mode for watch dog module.
 */
void ithWatchDogResume(void);

/** @} */ // end of ith_wd

/** @defgroup ith_ir IR
 *  @{
 */

/**
 * IR control definition.
 */
typedef enum
{
    ITH_IR_EN        = 0,    ///< Capture Enable
    ITH_IR_DEBOUNCE  = 1,    ///< Enable De-bounce circuit
    ITH_IR_INT       = 2,    ///< Enable Interrupt
    ITH_IR_PIO       = 3,    ///< PIO Mode
    ITH_IR_SIGINVESE = 8,    ///< Inverse the signal
    ITH_IR_TMRST     = 10,   ///< Reset the timer after sampled event
    ITH_IR_WARP      = 11    ///< Do not stop capture even time out interrupt occurs
} ITHIrCtrl;

/**
 * IR TX control definition.
 */

typedef enum
{
    ITH_IR_TX_EN        = 0,    ///< IR sender Enable
    ITH_IR_TX_LOOPBACK  = 1,    ///< Loopback  to the Receiver
    ITH_IR_TX_INT       = 2,    ///< Enable Interrupt
    ITH_IR_TX_PIO       = 3,    ///< PIO Mode
    ITH_IR_TX_SIGINVESE = 8,    ///< Inverse the signal
} ITHIrTxCtrl;

typedef enum
{
    ITH_IR_TRIGGER_LEVEL_1    = 0,    ///< 1
    ITH_IR_TRIGGER_LEVEL_32   = 1,    ///< (CFG_DEPTH/8)*4
    ITH_IR_TRIGGER_LEVEL_48   = 2,    ///< (CFG_DEPTH/8)*6
    ITH_IR_TRIGGER_LEVEL_64   = 3     ///< (CFG_DEPTH/8)*8
} ITHIrSenderFifoIntrTriggerLevel;

typedef enum
{
    ITH_IR_RISING    = 0,    ///< Rising Edge
    ITH_IR_FALLING   = 1,    ///< Falling Edge
    ITH_IR_BOTH      = 2     ///< Both Edge
} ITHIrCaptureMode;

typedef enum
{
    ITH_IR_DATA      = 8,    ///< Data trigger level available interrupt
    ITH_IR_FULL      = 9,    ///< FIFO Full Interrupt
    ITH_IR_EMPTY     = 10,   ///< FIFO Empty Interrupt
    ITH_IR_OE        = 11,   ///< Overrun Interrupt
    ITH_IR_TIMEOUT   = 12    ///< Timeout interrupt
} ITHIrIntrCtrl;

/**
 * Initializes IR module.
 *
 * @param pin the GPIO pin.
 * @param extClk the external clock. 0 to use PLL clock.
 * @param sample the sample clock in microseconds.
 * @param precision the precision.
 */
void ithIrInit(unsigned int pin, unsigned long extClk, unsigned long sample, unsigned int precision);

#if (CFG_CHIP_FAMILY == 9850)
/**
 * Initializes IR TX module.
 *
 * @param pin the GPIO pin.
 * @param extClk the external clock. 0 to use PLL clock.
 * @param sample the sample clock in microseconds.
 * @param precision the precision.
 */
void ithIrTxInit(unsigned int pin, unsigned long extClk, unsigned long sample, unsigned int precision);

/**
 * Sets IR RX GPIO pin.
 *
 * @param pin the IR RX GPIO pin.
 */
static inline void ithIrSetGpio(unsigned int pin)
{
    ithWriteRegA(ITH_IR_BASE + ITH_IR_GPIO_SEL_REG, pin | (1<<7) );
}
#endif

/**
 * Enables specified IR control.
 *
 * @param ctrl the control to enable.
 */
static inline void ithIrCtrlEnable(ITHIrCtrl ctrl)
{
    ithSetRegBitA(ITH_IR_BASE + ITH_IR_CAP_CTRL_REG, ctrl);
}

/**
 * Disables specified IR control.
 *
 * @param ctrl the control to disable.
 */
static inline void ithIrCtrlDisable(ITHIrCtrl ctrl)
{
    ithClearRegBitA(ITH_IR_BASE + ITH_IR_CAP_CTRL_REG, ctrl);
}

/**
 * Sets IR capture mode.
 *
 * @param mode the capture mode to set.
 */
static inline void ithIrSetCaptureMode(ITHIrCaptureMode mode)
{
    ithWriteRegMaskA(ITH_IR_BASE + ITH_IR_CAP_CTRL_REG, mode << ITH_IR_CAPMODE_BIT, ITH_IR_CAPMODE_MASK);
}

/**
 * Enables specified IR interrupt control.
 *
 * @param ctrl the control to enable.
 */
static inline void ithIrIntrCtrlEnable(ITHIrCtrl ctrl)
{
    ithSetRegBitA(ITH_IR_BASE + ITH_IR_CAP_STATUS_REG, ctrl);
}

/**
 * Disables specified IR interrupt control.
 *
 * @param ctrl the control to disable.
 */
static inline void ithIrIntrCtrlDisable(ITHIrCtrl ctrl)
{
    ithClearRegBitA(ITH_IR_BASE + ITH_IR_CAP_STATUS_REG, ctrl);
}

/**
 * Clears the receiver FIFO and reset control logic.
 */
static inline void ithIrClear(void)
{
    ithSetRegBitA(ITH_IR_BASE + ITH_IR_CAP_CTRL_REG, ITH_IR_CLEAR_BIT);
}

/**
 * Probes IR code.
 *
 * @return the code. -1 indicates no key input.
 */
int ithIrProbe(void);

#if (CFG_CHIP_FAMILY == 9850)
/**
 * Transmit IR code.
 *
 * int code: the IR code
 */
static inline void ithIrTxTransmit(int code)
{
    ithWriteRegA(ITH_IR_BASE + ITH_IRTX_SEND_DATA_REG, code);
}

/**
 * Set IR TX modulation frequency.
 *
 * @param freqDiv the modulation frequency.
 */
static inline void ithIrTxSetModFreq(int freqDiv)
{
    ithWriteRegA(ITH_IR_BASE + ITH_IRTX_MOD_REG, 0x8000 | freqDiv );
}

/**
 * Set IR RX modulation frequency.
 *
 * @param freqDiv the modulation frequency.
 */
static inline void ithIrRxSetModFilter(int minFreqDiv, int maxFreqDiv)
{
    ithWriteRegA(ITH_IR_BASE + ITH_IR_MOD_FILTER_REG, (0x1<<31)|(minFreqDiv&0x7FFF)<<16|(maxFreqDiv&0x7FFF) );
}

/**
 * Sets IR capture mode.
 *
 * @param mode the capture mode to set.
 */
static inline void ithIrTxSetCaptureMode(ITHIrCaptureMode mode)
{
    ithWriteRegMaskA(ITH_IR_BASE + ITH_IR_CAP_CTRL_REG, mode << ITH_IR_CAPMODE_BIT, ITH_IR_CAPMODE_MASK);
}

/**
 * Enables specified IR interrupt control.
 *
 * @param ctrl the control to enable.
 */
static inline void ithIrTxIntrCtrlEnable(ITHIrCtrl ctrl)
{
    ithSetRegBitA(ITH_IR_BASE + ITH_IR_CAP_STATUS_REG, ctrl);
}

/**
 * Enables specified IR control.
 *
 * @param ctrl the control to enable.
 */
static inline void ithIrTxCtrlEnable(ITHIrCtrl ctrl)
{
    ithSetRegBitA(ITH_IR_BASE + ITH_IRTX_CAP_CTRL_REG, ctrl);
}

/**
 * Disables specified IR control.
 *
 * @param ctrl the control to disable.
 */
static inline void ithIrTxCtrlDisable(ITHIrCtrl ctrl)
{
    ithClearRegBitA(ITH_IR_BASE + ITH_IRTX_CAP_CTRL_REG, ctrl);
}

/**
 * Disables specified IR interrupt control.
 *
 * @param ctrl the control to disable.
 */
static inline void ithIrTxIntrCtrlDisable(ITHIrCtrl ctrl)
{
    ithClearRegBitA(ITH_IR_BASE + ITH_IRTX_CAP_STATUS_REG, ctrl);
}

/**
 * Clears the receiver FIFO and reset control logic.
 */
static inline void ithIrTxClear(void)
{
    ithSetRegBitA(ITH_IR_BASE + ITH_IRTX_CAP_CTRL_REG, ITH_IR_CLEAR_BIT);
}
#endif

/** @} */ // end of ith_ir

/** @defgroup ith_pwm PWM
 *  @{
 */

/**
 * PWM definition.
 */
typedef enum
{
    ITH_PWM1 = ITH_TIMER1,  ///< PWM #1
    ITH_PWM2 = ITH_TIMER2,  ///< PWM #2
    ITH_PWM3 = ITH_TIMER3,  ///< PWM #3
    ITH_PWM4 = ITH_TIMER4,  ///< PWM #4
    ITH_PWM5 = ITH_TIMER5,  ///< PWM #5
    ITH_PWM6 = ITH_TIMER6   ///< PWM #6
} ITHPwm;

/**
 * Initialize PWM module.
 *
 * @param pwm The PWM.
 * @param freq The frequence.
 * @param duty The duty cycle.
 */
void ithPwmInit(ITHPwm pwm, unsigned int freq, unsigned int duty);

/**
 * Resets PWM module.
 *
 * @param pwm The PWM.
 * @param pin the GPIO pin.
 * @param gpio_mode the GPIO mode.
 */
void ithPwmReset(ITHPwm pwm, unsigned int pin , unsigned int gpio_mode);

/**
 * Sets duty cycle of PWM.
 *
 * @param pwm The PWM.
 * @param duty The duty cycle to set.
 */
void ithPwmSetDutyCycle(ITHPwm pwm, unsigned int duty);

/**
 * Enables PWM.
 *
 * @param pwm The PWM.
 * @param pin the GPIO pin.
 * @param gpio_mode the GPIO mode.
 */
void ithPwmEnable(ITHPwm pwm, unsigned int pin , unsigned int gpio_mode);

/**
 * Disables PWM.
 *
 * @param pwm The PWM.
 * @param pin the GPIO pin.
 */
void ithPwmDisable(ITHPwm pwm, unsigned int pin);

/** @} */ // end of ith_pwm

/** @defgroup ith_keypad Keypad
 *  @{
 */

/**
 * Initializes keypad module.
 *
 * @param pinCount The count of GPIO pins.
 * @param pinArray The array of GPIO pin numbers.
 */
void ithKeypadInit(unsigned int pinCount, unsigned int* pinArray);

/**
 * Enables keypad module.
 */
void ithKeypadEnable(void);

/**
 * Probes the input of keypad.
 */
int ithKeypadProbe(void);

/** @} */ // end of ith_keypad

/** @defgroup ith_nand NAND
 *  @{
 */

/**
 * Suspends NAND module.
 */
void ithNandSuspend(void);

/**
 * Resumes from suspend mode for NAND module.
 */
void ithNandResume(void);

/**
 * Enable NAND clock.
 */
void ithNandEnableClock(void);

/**
 * Disable NAND clock.
 */
void ithNandDisableClock(void);

/** @} */ // end of ith_nand

/** @defgroup ith_xd xD
 *  @{
 */

/**
 * Suspends XD module.
 */
void ithXdSuspend(void);

/**
 * Resumes from suspend mode for XD module.
 */
void ithXdResume(void);

/**
 * Enable XD clock.
 */
void ithXdEnableClock(void);

/**
 * Disable XD clock.
 */
void ithXdDisableClock(void);

/** @} */ // end of ith_xd

/** @defgroup ith_storage Storage
 *  @{
 */

/**
 * The pin of card definition.
 */
typedef enum
{
    ITH_CARDPIN_SD0 = 0,    ///< SD0 card pin
    ITH_CARDPIN_MS  = 0,    ///< MS card pin
    ITH_CARDPIN_XD  = 0,    ///< xD card pin
    ITH_CARDPIN_SD1,        ///< SD1 card pin
    ITH_CARDPIN_CF,         ///< CF card pin

    ITH_CARDPIN_MAX
} ITHCardPin;

#define SD_PIN_NUM      10
/**
 * The card configuration definition.
 */
typedef struct
{
    uint8_t cardDetectPins[ITH_CARDPIN_MAX];    ///< Array of card detection GPIO pins
    uint8_t powerEnablePins[ITH_CARDPIN_MAX];   ///< Array of power enable GPIO pins
    uint8_t writeProtectPins[ITH_CARDPIN_MAX];  ///< Array of write protect GPIO pins
    uint8_t sd0Pins[SD_PIN_NUM];
    uint8_t sd1Pins[SD_PIN_NUM];
} ITHCardConfig;

/**
 * Initializes card module.
 *
 * @param cfg The card configuration to set.
 */
void ithCardInit(const ITHCardConfig* cfg);

/**
 * Power on specified card.
 *
 * @param pin The card power on pin to power on.
 */
void ithCardPowerOn(ITHCardPin pin);

/**
 * Power off specified card.
 *
 * @param pin The card power on pin to power off.
 */
void ithCardPowerOff(ITHCardPin pin);

/**
 * Whether card is inserted or not.
 *
 * @param pin The card detect pin to detect.
 */
bool ithCardInserted(ITHCardPin pin);

/**
 * Whether card is locked or not.
 *
 * @param pin The card write protect pin to detect.
 */
bool ithCardLocked(ITHCardPin pin);

typedef enum
{
    ITH_STOR_NAND = 0,
    ITH_STOR_XD   = 1,
    ITH_STOR_SD   = 2,
    ITH_STOR_MS_0 = 3,
    ITH_STOR_MS_1 = 4,
    ITH_STOR_CF   = 5,
    ITH_STOR_NOR  = 6,
    ITH_STOR_SD1  = 7
} ITHStorage;

extern void* ithStorMutex;
void ithStorageSelect(ITHStorage storage);
void ithStorageUnSelect(ITHStorage storage);
#if (CFG_CHIP_FAMILY == 9850)
bool ithSdSwitchPin1(int idx);
#endif


/** @} */ // end of ith_storage

/** @defgroup ith_utility Utility
 *  @{
 */
// Macros
/**
 * Assertion at compile time.
 *
 * @param e Specifies any logical expression.
 */
#define ITH_STATIC_ASSERT(e) \
    typedef int ITH_STATIC_ASSERT_DUMMY_##__LINE__[(e) * 2 - 1]

/**
 * Counts the element number of an array.
 *
 * @param array The array.
 * @return The element number of array.
 */
#define ITH_COUNT_OF(array) (sizeof (array) / sizeof (array[0]))

/**
 * Aligns a value to its ceil.
 *
 * @param value The value will be aligned.
 * @param align The alignment. It must be a power of 2.
 * @return The aligned value.
 */
#define ITH_ALIGN_UP(value, align) (((value) + ((align) - 1)) & ~((align) - 1))

/**
 * Aligns a value to its floor.
 *
 * @param value The value will be aligned.
 * @param align The alignment. It must be a power of 2.
 * @return The aligned value.
 */
#define ITH_ALIGN_DOWN(value, align) ((value) & ~((align) - 1))

/**
 * Determines whether the value is aligned.
 *
 * @param value The value.
 * @param align The alignment. It must be a power of 2.
 * @return Whether the value is aligned.
 */
#define ITH_IS_ALIGNED(value, align) (((value) & ((align) - 1)) == 0)

/**
 * Determines whether the value is power of two.
 *
 * @param x The value.
 * @return Whether the value is power of two.
 */
#define ITH_IS_POWER_OF_TWO(x) ((((x) - 1) & (x)) == 0)

/**
 * Calculates the absolute value.
 *
 * @param x The value.
 * @return The absolute value.
 */
#define ITH_ABS(x) (((x) >= 0) ? (x) : -(x))

/**
 * Returns the larger of two values.
 *
 * @param a Values of any numeric type to be compared.
 * @param b Values of any numeric type to be compared.
 * @return The larger of its arguments.
 */
#define ITH_MAX(a, b)  (((a) > (b)) ? (a) : (b))

/**
 * Returns the smaller of two values.
 *
 * @param a Values of any numeric type to be compared.
 * @param b Values of any numeric type to be compared.
 * @return The smaller of its arguments.
 */
#define ITH_MIN(a, b)  (((a) < (b)) ? (a) : (b))

/**
 * Swaps two values.
 *
 * @param a The value.
 * @param b The another value.
 * @param type The value type.
 */
#define ITH_SWAP(a, b, type) \
    do { type tmp = (a); a = (b); b = tmp; } while (0)

/**
 * Converts 16-bit value to another endian integer.
 *
 * @param value The value.
 * @return The converted value.
 */
static inline uint16_t ithBswap16(uint16_t value)
{
    return ((value & 0x00FF) << 8) |
           ((value & 0xFF00) >> 8);
}

/**
 * Converts 32-bit value to another endian integer.
 *
 * @param value The value.
 * @return The converted value.
 */
static inline uint32_t ithBswap32(uint32_t value)
{
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0xFF000000) >> 24);
}

/**
 * Converts 64-bit value to another endian integer.
 *
 * @param value The value.
 * @return The converted value.
 */
static inline uint64_t ithBswap64(uint64_t value)
{
    return ((value & 0xff00000000000000ull) >> 56) |
           ((value & 0x00ff000000000000ull) >> 40) |
           ((value & 0x0000ff0000000000ull) >> 24) |
           ((value & 0x000000ff00000000ull) >> 8 ) |
           ((value & 0x00000000ff000000ull) << 8 ) |
           ((value & 0x0000000000ff0000ull) << 24) |
           ((value & 0x000000000000ff00ull) << 40) |
           ((value & 0x00000000000000ffull) << 56);
}

/**
 * Packs colors to a RGB565 format value.
 *
 * @param r red value.
 * @param g green value.
 * @param b blue value.
 * @return The packed value.
 */
#define ITH_RGB565(r, g, b) \
    ((((uint16_t)(r) >> 3) << 11) | (((uint16_t)(g) >> 2) << 5) | ((uint16_t)(b) >> 3))

/**
 * Packs colors to a ARGB1555 format value.
 *
 * @param a alpha value.
 * @param r red value.
 * @param g green value.
 * @param b blue value.
 * @return The packed value.
 */
#define ITH_ARGB1555(a, r, g, b) \
    ((((uint16_t)(a) >> 7) << 15) | (((uint16_t)(r) >> 3) << 10) | (((uint16_t)(g) >> 3) << 5) | ((uint16_t)(b) >> 3))

/**
 * Packs colors to a ARGB4444 format value.
 *
 * @param a alpha value.
 * @param r red value.
 * @param g green value.
 * @param b blue value.
 * @return The packed value.
 */
#define ITH_ARGB4444(a, r, g, b) \
    ((((uint16_t)(a) >> 4) << 12) | (((uint16_t)(r) >> 4) << 8) | (((uint16_t)(g) >> 4) << 4) | ((uint16_t)(b) >> 4))

/**
 * Packs colors to a ARGB8888 format value.
 *
 * @param a alpha value.
 * @param r red value.
 * @param g green value.
 * @param b blue value.
 * @return The packed value.
 */
#define ITH_ARGB8888(a, r, g, b) \
    (((uint32_t)(a) << 24) | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))

/**
 * Sets the first len words (16-bits) of the block of memory pointed by ptr to the specified value.
 *
 * @param dst Pointer to the block of memory to fill.
 * @param val Value to be set. The value is passed as an int, but the function fills the block of memory using the uint16_t conversion of this value.
 * @param len Number of words (16-bits) to be set to the value.
 */
static inline void ithMemset16(void *dst, int val, size_t len)
{
    uint16_t *p = (uint16_t*)dst;

    while (len--)
        *p++ = val;
}

/**
 * Sets the first len words (32-bits) of the block of memory pointed by ptr to the specified value.
 *
 * @param dst Pointer to the block of memory to fill.
 * @param val Value to be set. The value is passed as an int, but the function fills the block of memory using the uint32_t conversion of this value.
 * @param len Number of words (32-bits) to be set to the value.
 */
static inline void ithMemset32(void *dst, int val, size_t len)
{
    uint32_t *p = (uint32_t*)dst;

    while (len--)
        *p++ = val;
}

/** List node */
typedef struct ITHListTag
{
    struct ITHListTag* next; /**< Next node */
    struct ITHListTag* prev; /**< Previous node */
} ITHList;

/**
 * Pushes a node to the front of list.
 *
 * @param list The list.
 * @param node The node wiil be pushed.
 */
void ithListPushFront(ITHList* list, void* node);

/**
 * Pushes a node to the back of list.
 *
 * @param list The list.
 * @param node The node wiil be pushed.
 */
void ithListPushBack(ITHList* list, void* node);

/**
 * Inserts a node before another node.
 *
 * @param list The list.
 * @param listNode The node will be after the inserted node.
 * @param node The node wiil be inserted.
 */
void ithListInsertBefore(ITHList* list, void* listNode, void* node);

/**
 * Inserts a node after another node.
 *
 * @param list The list.
 * @param listNode The node will be before the inserted node.
 * @param node The node wiil be inserted.
 */
void ithListInsertAfter(ITHList* list, void* listNode, void* node);

/**
 * Removes a node from a list.
 *
 * @param list The list.
 * @param node The node wiil be removed.
 */
void ithListRemove(ITHList* list, void* node);

/**
 * Clears every nodes in a list.
 *
 * @param list The list.
 * @param dtor The destructor to destroy every node. Can be MMP_NULL.
 */
void ithListClear(ITHList* list, void (*dtor) (void*));

/**
 * Converts unsigned long value to binary format string.
 *
 * @param s The binary format string.
 * @param i The unsigned long value.
 */
char* ithUltob(char* s, unsigned long i);

/**
 * Prints host register values.
 *
 * @param addr the start register address to dump
 * @param size the size of register block to dump
 */
void ithPrintRegH(uint16_t addr, unsigned int size);

/**
 * Prints AMBA register values.
 *
 * @param addr the start register address to dump
 * @param size the size of register block to dump
 */
void ithPrintRegA(uint32_t addr, unsigned int size);

/**
 * Prints VRAM values on 8-bit format.
 *
 * @param addr the start address to dump
 * @param size the size of block to dump
 */
void ithPrintVram8(uint32_t addr, unsigned int size);

/**
 * Prints VRAM values on 16-bit format.
 *
 * @param addr the start address to dump
 * @param size the size of block to dump
 */
void ithPrintVram16(uint32_t addr, unsigned int size);

/**
 * Prints VRAM values on 32-bit format.
 *
 * @param addr the start address to dump
 * @param size the size of block to dump
 */
void ithPrintVram32(uint32_t addr, unsigned int size);

// Print functions

/**
 * Putchar callback function for ithPrintf().
 *
 * @param c the character to output
 * @return same as c
 */
extern int (*ithPutcharFunc)(int c);

/**
 * General printf() function.
 *
 * @param fmt Format control.
 * @param ... Optional arguments.
 * @return the number of characters printed, or a negative value if an error occurs.
 */
int ithPrintf(const char* fmt, ...);

// Video memroy management
#define ITH_VMEM_MCB_COUNT  2048 ///< Maximum Video memory control block

/**
 * Video memory control block definition.
 */
typedef struct
{
    ITHList*    next;           ///< next block
    ITHList*    prev;           ///< previous block
    uint32_t    addr;           ///< memory address
    uint32_t    size    : 30;   ///< memory size
    uint32_t    state   : 2;    ///< memory state
} ITHVmemMcb;

/**
 * Video memory management global data definition.
 */
typedef struct
{
    uint32_t    startAddr;                  ///< start address to management
    uint32_t    totalSize;                  ///< total size to management
    void*       mutex;                      ///< mutex to protect allocation/free
    ITHVmemMcb  mcbs[ITH_VMEM_MCB_COUNT];   ///< pre-allocate memory control blocks
    uint32_t    usedMcbCount;               ///< used memory control block count
    ITHList     usedMcbList;                ///< used memory control block list
    uint32_t    freeSize;                   ///< current free size
} ITHVmem;

/**
 * Global video memory management instance
 */
extern ITHVmem* ithVmem;

/**
 * Initializes video memory management module
 *
 * @param vmem the video memory management instance
 */
void ithVmemInit(ITHVmem* vmem);

/**
 * Allocates video memory.
 *
 * @param size The amount of memory you want to allocate, in bytes.
 * @return Allocated memory address, or 0 if an error occurred
 */
uint32_t ithVmemAlloc(uint32_t size);

/**
 * The ithVmemAlignedAlloc function is similar to the memalign function in that
 * it returns a video memory of size bytes aligned to a multiple of alignment.
 *
 * @param alignment The alignment that you want to use for the memory. This must be a multiple of size( void *).
 * @param size The amount of memory you want to allocate, in bytes.
 * @return Allocated memory address, or 0 if an error occurred
 */
uint32_t ithVmemAlignedAlloc(uint32_t alignment, uint32_t size);

/**
 * Releases allocated video memory.
 *
 * @param addr The allocated video memory address.
 */
void ithVmemFree(uint32_t addr);

/**
 * Prints the video memory management status.
 */
void ithVmemStats(void);

/** @} */ // end of ith_storage

/** @defgroup ith_codec Codec
 *  @{
 */

/**
 * Command for codec on risc.
 */
bool ithCodecCommand(int command, int parameter0, int parameter1, int parameter2);

/**
 * Open engine for codec on risc.
 */
//void ithCodecOpenEngine(void);

/**
 * Read card id from wiegand.
 */
int ithCodecWiegandReadCard(int index, unsigned long long* card_id);

/**
 * Redirect printf message to codec of sw uart.
 */
void ithCodecPrintfWrite(char* string, int length);

/**
 * Redirect ctrlborad message to codec of hearbeat.
 */

#define MAX_BUFFER_COUNT      100
typedef struct HEARTBEATDATA_TAG{
    int RisingPeriod;
    int upTime;
    int HeartbeatTimeoutResult;
    int CountID;
} HEARTBEATDATA;

typedef struct HEARTBEAT_INFO_TAG {
    HEARTBEATDATA data[MAX_BUFFER_COUNT];
    int ReadIndex;
    int WriteIndex;
    int WriteIndexForRISC;
    int Buffersize;
} HEARTBEAT_INFO;

void ithCodecCtrlBoardWrite(uint8_t* data, int length);

void ithCodecCtrlBoardRead(uint8_t* data, int length);

void ithCodecHeartBeatRead(uint8_t* data,int length);

/** @} */ // end of ith_wiegand

#include "ith_dma.h"
#include "ith_gpio.h"

#ifdef __cplusplus
}
#endif

#endif // ITE_ITH_H
/** @} */ // end of ith
