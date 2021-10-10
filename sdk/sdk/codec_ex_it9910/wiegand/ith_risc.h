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

/**
 * GPIO mode definition.
 */
typedef enum
{
    ITH_GPIO_MODE0      = 0, ///< mode 0
    ITH_GPIO_MODE1      = 1, ///< mode 1
    ITH_GPIO_MODE2      = 2, ///< mode 2
    ITH_GPIO_MODE3      = 3, ///< mode 3
	ITH_GPIO_MODE_TX    = 4, ///< mode for UART TX
	ITH_GPIO_MODE_RX    = 5  ///< mode for UART RX
} ITHGpioMode;
 
#define ITH_GPIO_BASE                   0xDE000000
#define ITH_GPIO1_INTRTRIG_REG          0x30
#define ITH_GPIO1_BOUNCEEN_REG          0x3C
#define ITH_GPIO1_PINDIR_REG            0x08
#define ITH_GPIO1_INTREN_REG            0x1C
#define ITH_GPIO1_INTRCLR_REG           0x2C
#define ITH_GPIO1_INTRRAWSTATE_REG      0x20
#define ITH_GPIO2_INTRRAWSTATE_REG      0x60
#define ITH_GPIO1_DATASET_REG           0x0C
#define ITH_GPIO2_DATASET_REG           0x4C
#define ITH_GPIO1_DATACLR_REG           0x10
#define ITH_GPIO2_DATACLR_REG           0x50
#define ITH_GPIO_HOSTSEL_GPIO           0x4
#define ITH_GPIO_HOSTSEL_MASK           (0x7 << ITH_GPIO_HOSTSEL_BIT)
#define ITH_GPIO_HOSTSEL_BIT            4
#define ITH_GPIO_HOSTSEL_REG            0xD0
#define ITH_GPIO_URTXSEL1_REG           0xD8
#define ITH_GPIO_URTXSEL2_REG           0xDC
#define ITH_GPIO_URRXSEL1_REG           0xE0
#define ITH_GPIO_URRXSEL2_REG           0xE4
#define ITH_GPIO1_DATAIN_REG            0x04
#define ITH_GPIO2_DATAIN_REG            0x44
#define ITH_GPIO1_MODE_REG              0x90
#define ITH_GPIO2_MODE_REG              0x94
#define ITH_GPIO3_MODE_REG              0x98
#define ITH_GPIO4_MODE_REG              0x9C




#define ITH_GPIO2_BOUNCEPRESCALE_REG    0x80
#define ITH_GPIO2_BOUNCEEN_REG          0x7C
#define ITH_GPIO2_PINDIR_REG            0x48
#define ITH_GPIO2_INTREN_REG            0x5C
#define ITH_GPIO2_INTRCLR_REG           0x6C

#ifdef __cplusplus
extern "C" {
#endif
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
 * Sets the mode of GPIO pin.
 *
 * @param pin the GPIO pin to set.
 * @param mode the mode to set.
 */
void ithGpioSetMode(unsigned int pin, ITHGpioMode mode);

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
 * Sets GPIO pin to input mode.
 *
 * @param pin the GPIO pin to set to input mode.
 */
static inline void ithGpioSetIn(unsigned int pin)
{
    //ithEnterCritical();

    if (pin < 32)
        ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO1_PINDIR_REG, pin);
    else
        ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO2_PINDIR_REG, pin - 32);

    //ithExitCritical();
}

/**
 * Sets GPIO pin to output mode.
 *
 * @param pin the GPIO pin to set to output mode.
 */
static inline void ithGpioSetOut(unsigned int pin)
{
    //ithEnterCritical();

    if (pin < 32)
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO1_PINDIR_REG, pin);
    else
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO2_PINDIR_REG, pin - 32);

    //ithExitCritical();
}

/**
 * Sets GPIO pin to 1.
 *
 * @param pin the GPIO pin to set to 1.
 */
static inline void ithGpioSet(unsigned int pin)
{
    //ithEnterCritical();

    if (pin < 32)
        ithWriteRegA(ITH_GPIO_BASE + ITH_GPIO1_DATASET_REG, 0x1 << pin);
    else
        ithWriteRegA(ITH_GPIO_BASE + ITH_GPIO2_DATASET_REG, 0x1 << (pin - 32));

    //ithExitCritical();
}

/**
 * Gets the value of GPIO pin.
 *
 * @param pin the GPIO pin to get.
 */
static inline uint32_t ithGpioGet(unsigned int pin)
{
    if (pin < 32)
        return ithReadRegA(ITH_GPIO_BASE + ITH_GPIO1_DATAIN_REG) & (0x1 << pin);
    else
        return ithReadRegA(ITH_GPIO_BASE + ITH_GPIO2_DATAIN_REG) & (0x1 << (pin - 32));
}

/**
 * Sets GPIO pin to 0.
 *
 * @param pin the GPIO pin to set to 0.
 */
static inline void ithGpioClear(unsigned int pin)
{
    //ithEnterCritical();

    if (pin < 32)
        ithWriteRegA(ITH_GPIO_BASE + ITH_GPIO1_DATACLR_REG, 0x1 << pin);
    else
        ithWriteRegA(ITH_GPIO_BASE + ITH_GPIO2_DATACLR_REG, 0x1 << (pin - 32));

    //ithExitCritical();
}

/**
 * Enables GPIO pin.
 *
 * @param pin the GPIO pin to enable.
 */
static inline void ithGpioEnable(unsigned int pin)
{
    // GPIO0~GPIO3 will be switch to GPIO mode form SPI(or JTAG) by setting this register
    if (pin < 4)
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_HOSTSEL_REG, ITH_GPIO_HOSTSEL_GPIO << ITH_GPIO_HOSTSEL_BIT, ITH_GPIO_HOSTSEL_MASK);

    ithGpioSetMode(pin, ITH_GPIO_MODE0);
}


/**
 * Sets the debounce clock.
 *
 * @param clk the clock.
 */
static inline void ithGpioSetDebounceClock(unsigned int clk)
{
    ithWriteRegA(ITH_GPIO_BASE + ITH_GPIO2_BOUNCEPRESCALE_REG, ithGetBusClock() / clk - 1);
}

/**
 * Enables the interrupt of GPIO pin.
 *
 * @param pin the GPIO pin to enable.
 */
static inline void ithGpioEnableIntr(unsigned int pin)
{
    if (pin < 32)
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO1_INTREN_REG, pin);
    else
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO2_INTREN_REG, pin - 32);
}

/**
 * Enables the bounce of GPIO pin.
 *
 * @param pin the GPIO pin.
 */
static inline void ithGpioEnableBounce(unsigned int pin)
{
    if (pin < 32)
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO1_BOUNCEEN_REG, pin);
    else
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO2_BOUNCEEN_REG, pin - 32);
}

/**
 * Clears the interrupt of GPIO pin.
 *
 * @param pin the GPIO pin to clear.
 */
static inline void ithGpioClearIntr(unsigned int pin)
{
    if (pin < 32)
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO1_INTRCLR_REG, pin);
    else
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO2_INTRCLR_REG, pin - 32);
}


 
#ifdef __cplusplus
}
#endif

#endif // ITE_ITH_H
/** @} */ // end of ith
