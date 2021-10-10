#ifndef ITE_ITH_GPIO_H
#define ITE_ITH_GPIO_H

#include "ite/ith.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup ith_gpio GPIO
 *  @{
 */

/**
 * GPIO mode definition.
 */
typedef enum
{
    ITH_GPIO_MODE0         = 0,  ///< mode 0
    ITH_GPIO_MODE1         = 1,  ///< mode 1
    ITH_GPIO_MODE2         = 2,  ///< mode 2
    ITH_GPIO_MODE3         = 3,  ///< mode 3
    ITH_GPIO_MODE4         = 4,  ///< mode 3
    ITH_GPIO_MODE_TX       = 5,  ///< mode for UART TX
    ITH_GPIO_MODE_RX       = 6,  ///< mode for UART RX
    ITH_GPIO_MODE_TX0      = 5,  ///< mode for UART TX 0
    ITH_GPIO_MODE_RX0      = 6,  ///< mode for UART RX 0
    ITH_GPIO_MODE_TX1      = 7,  ///< mode for UART TX 1
    ITH_GPIO_MODE_RX1      = 8,  ///< mode for UART RX 1
    ITH_GPIO_MODE_TX2      = 9,  ///< mode for UART TX 2
    ITH_GPIO_MODE_RX2      = 10,  ///< mode for UART RX 2 from GPIO
    ITH_GPIO_MODE_RX2WGAND = 11, ///< mode for UART RX 2 from Wiegand
    ITH_GPIO_MODE_TX3      = 12, ///< mode for UART TX 3
    ITH_GPIO_MODE_RX3      = 13, ///< mode for UART RX 3  from GPIO
    ITH_GPIO_MODE_RX3WGAND = 14  ///< mode for UART RX 3 from Wiegand
} ITHGpioMode;

typedef enum
{
	ITH_GPIO_DRIVING_0 = 0,
	ITH_GPIO_DRIVING_1 = 1,
	ITH_GPIO_DRIVING_2 = 2, //default
	ITH_GPIO_DRIVING_3 = 3,
}ITHGpioDriving;

typedef enum
{
    ITH_GPIO_PIN0 = 0,
    ITH_GPIO_PIN1,
    ITH_GPIO_PIN2,
    ITH_GPIO_PIN3,
    ITH_GPIO_PIN4,
    ITH_GPIO_PIN5,
    ITH_GPIO_PIN6,
    ITH_GPIO_PIN7,
    ITH_GPIO_PIN8,
    ITH_GPIO_PIN9,
    ITH_GPIO_PIN10,
    ITH_GPIO_PIN11,
    ITH_GPIO_PIN12,
    ITH_GPIO_PIN13,
    ITH_GPIO_PIN14,
    ITH_GPIO_PIN15,
    ITH_GPIO_PIN16,
    ITH_GPIO_PIN17,
    ITH_GPIO_PIN18,
    ITH_GPIO_PIN19,
    ITH_GPIO_PIN20,
    ITH_GPIO_PIN21,
    ITH_GPIO_PIN22,
    ITH_GPIO_PIN23,
    ITH_GPIO_PIN24,
    ITH_GPIO_PIN25,
    ITH_GPIO_PIN26,
    ITH_GPIO_PIN27,
    ITH_GPIO_PIN28,
    ITH_GPIO_PIN29,
    ITH_GPIO_PIN30,
    ITH_GPIO_PIN31,
    ITH_GPIO_PIN32,
    ITH_GPIO_PIN33,
    ITH_GPIO_PIN34,
    ITH_GPIO_PIN35,
    ITH_GPIO_PIN36,
    ITH_GPIO_PIN37,
    ITH_GPIO_PIN38,
    ITH_GPIO_PIN39,
    ITH_GPIO_PIN40,
    ITH_GPIO_PIN41,
    ITH_GPIO_PIN42,
    ITH_GPIO_PIN43,
    ITH_GPIO_PIN44,
    ITH_GPIO_PIN45,
    ITH_GPIO_PIN46,
    ITH_GPIO_PIN47,
    ITH_GPIO_PIN48,
    ITH_GPIO_PIN49,
    ITH_GPIO_PIN50,
    ITH_GPIO_PIN51,
    ITH_GPIO_PIN52,
    ITH_GPIO_PIN53,
    ITH_GPIO_PIN54,
    ITH_GPIO_PIN55,
    ITH_GPIO_PIN56,
    ITH_GPIO_PIN57,
    ITH_GPIO_PIN58,
    ITH_GPIO_PIN59,
    ITH_GPIO_PIN60,
    ITH_GPIO_PIN61,
    ITH_GPIO_PIN62,
    ITH_GPIO_PIN63,
    ITH_GPIO_PIN64,
    ITH_GPIO_PIN65,
    ITH_GPIO_PIN66,
    ITH_GPIO_PIN67,
    ITH_GPIO_PIN68,
    ITH_GPIO_PIN69,
    ITH_GPIO_PIN70,
    ITH_GPIO_PIN71,
    ITH_GPIO_PIN72,
    ITH_GPIO_PIN73,
    ITH_GPIO_PIN74,
    ITH_GPIO_PIN75,
    ITH_GPIO_PIN76,
    ITH_GPIO_PIN77,
    ITH_GPIO_PIN78,
    ITH_GPIO_PIN79,
    ITH_GPIO_PIN80,
    ITH_GPIO_PIN81,
    ITH_GPIO_PIN82,
    ITH_GPIO_PIN83,
    ITH_GPIO_PIN84,
    ITH_GPIO_PIN85,
    ITH_GPIO_PIN86,
    ITH_GPIO_PIN87,
    ITH_GPIO_PIN88,
    ITH_GPIO_PIN89,
    ITH_GPIO_PIN90,
    ITH_GPIO_PIN91,
    ITH_GPIO_PIN92,
    ITH_GPIO_PIN93,
    ITH_GPIO_PIN94,
    ITH_GPIO_PIN95,
    ITH_GPIO_PIN96,
    ITH_GPIO_PIN97,
    ITH_GPIO_PIN98,
    ITH_GPIO_PIN99,
    ITH_GPIO_PIN100,
    ITH_GPIO_PIN101,
    ITH_GPIO_PIN102,
} ITHGpioPin;

/**
 * Sets the mode of GPIO pin.
 *
 * @param pin the GPIO pin to set.
 * @param mode the mode to set.
 */
void ithGpioSetMode(unsigned int pin, ITHGpioMode mode);

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
 * Sets GPIO pin to input mode.
 *
 * @param pin the GPIO pin to set to input mode.
 */
extern void ithGpioSetIn(unsigned int pin);

/**
 * Sets GPIO pin to output mode.
 *
 * @param pin the GPIO pin to set to output mode.
 */
extern void ithGpioSetOut(unsigned int pin);

/**
 * Sets GPIO pin to 1.
 *
 * @param pin the GPIO pin to set to 1.
 */
extern void ithGpioSet(unsigned int pin);

/**
 * Sets GPIO pin to 0.
 *
 * @param pin the GPIO pin to set to 0.
 */
extern void ithGpioClear(unsigned int pin);

/**
 * Gets the value of GPIO pin.
 *
 * @param pin the GPIO pin to get.
 */
extern uint32_t ithGpioGet(unsigned int pin);

/**
 * Sets GPIO pin to input mode macro.
 *
 * @param pin the GPIO pin to set to input mode.
 */
#define ITH_GPIO_SET_IN(pin)  ithGpioSetIn(pin)

/**
 * Sets GPIO pin to output mode macro.
 *
 * @param pin the GPIO pin to set to output mode.
 */
#define ITH_GPIO_SET_OUT(pin) ithGpioSetOut(pin)

/**
 * Sets GPIO pin to 1 macro.
 *
 * @param pin the GPIO pin to set to 1.
 */
#define ITH_GPIO_SET(pin)     ithGpioSet(pin)

/**
 * Sets GPIO pin to 0 macro.
 *
 * @param pin the GPIO pin to set to 0.
 */
#define ITH_GPIO_CLEAR(pin)   ithGpioClear(pin)

/**
 * Gets the value of GPIO pin macro.
 *
 * @param pin the GPIO pin to get.
 */
#define ITH_GPIO_GET(pin)     ithGpioGet(pin)

/**
 * GPIO interupt controls definition.
 */
typedef enum
{
    ITH_GPIO_PULL_ENABLE         = 0,   ///< GPIO pin pull enable
    ITH_GPIO_PULL_UP             = 1,   ///< GPIO pin pull up
    ITH_GPIO_INTR_LEVELTRIGGER   = 2,   ///< GPIO interrupt trigger method
    ITH_GPIO_INTR_BOTHEDGE       = 3,   ///< GPIO interrupt edge trigger by both edge
    ITH_GPIO_INTR_TRIGGERFALLING = 4,   ///< GPIO interrupt trigger by falling edge
    ITH_GPIO_INTR_TRIGGERLOW     = 4    ///< GPIO interrupt trigger by low level
} ITHGpioCtrl;

/**
 * Enables specified controls.
 *
 * @param pin the GPIO pin to enable.
 * @param ctrl the controls to enable.
 */
void ithGpioCtrlEnable(unsigned int pin, ITHGpioCtrl ctrl);

/**
 * Disables specified controls.
 *
 * @param pin the GPIO pin to disable.
 * @param ctrl the controls to disable.
 */
void ithGpioCtrlDisable(unsigned int pin, ITHGpioCtrl ctrl);

/**
 * GPIO interrupt handler.
 *
 * @param pin The GPIO pin.
 * @param arg Custom argument.
 */
typedef void (*ITHGpioIntrHandler)(unsigned int pin, void *arg);

/**
 * Registers GPIO interrupt handler.
 *
 * @param pin The GPIO pin to register.
 * @param handler The callback function.
 * @param arg Custom argument to pass to handler.
 */
void ithGpioRegisterIntrHandler(unsigned int pin, ITHGpioIntrHandler handler, void *arg);

/**
 * Dispatches GPIO interrupt to handlers.
 */
void ithGpioDoIntr(void);

/**
 * Enables the interrupt of GPIO pin.
 *
 * @param pin the GPIO pin to enable.
 */
extern void ithGpioEnableIntr(unsigned int pin);

/**
 * Disables the interrupt of GPIO pin.
 *
 * @param pin the GPIO pin to disable.
 */
extern void ithGpioDisableIntr(unsigned int pin);

/**
 * Clears the interrupt of GPIO pin.
 *
 * @param pin the GPIO pin to clear.
 */
extern void ithGpioClearIntr(unsigned int pin);

/**
 * Enables the bounce of GPIO pin.
 *
 * @param pin the GPIO pin.
 */
extern void ithGpioEnableBounce(unsigned int pin);

/**
 * Disables the bounce of GPIO pin.
 *
 * @param pin the GPIO pin.
 */
extern void ithGpioDisableBounce(unsigned int pin);

/**
 * Sets the debounce clock.
 *
 * @param clk the clock.
 */
extern void ithGpioSetDebounceClock(unsigned int clk);

/**
 * Initializes GPIO module.
 */
void ithGpioInit(void);

/**
 * Enables the clock of GPIO module.
 */
void ithGpioEnableClock(void);

/**
 * Disables the clock of GPIO module.
 */
void ithGpioDisableClock(void);

/**
 * Suspends GPIO module.
 */
void ithGpioSuspend(void);

/**
 * Resumes GPIO module.
 */
void ithGpioResume(void);

/**
 * Print GPIO information.
 */
void ithGpioStats(void);

void ithGpioSetDriving (unsigned int pin, ITHGpioDriving level);

/** @} */ // end of ith_gpio

#ifdef __cplusplus
}
#endif

#endif