#ifndef ITE_ITP_IO_EXPANDER_H
#define ITE_ITP_IO_EXPANDER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup itp_gpio_expander GPIO Expander
 *  @{
 */

#define ITP_MAX_GPIO_EXPANDER_HANDLER 8       ///< Maximum GPIO expander handler count

/**
 * IO expander definition.
 */
typedef enum
{
    IOEXPANDER_0 = 0,  ///< IOEXPANDER_0 
    IOEXPANDER_1 = 1,  ///< IOEXPANDER_1 
} ITP_IOExpander;

/**
 * Sets GPIO Expander pin to output mode.
 *
 * @param pin the GPIO Expander pin to set to output mode.
 */
void itpIOExpanderSetOut(ITP_IOExpander IOEXP_Port,unsigned int pin);

/**
 * Sets GPIO Expander pin to input mode.
 *
 * @param pin the GPIO Expander pin to set to input mode.
 */
void itpIOExpanderSetIn(ITP_IOExpander IOEXP_Port,unsigned int pin);

/**
 * Sets GPIO Expander pin to 1.
 *
 * @param pin the GPIO Expander pin to set to 1.
 */
void itpIOExpanderSet(ITP_IOExpander IOEXP_Port,unsigned int pin);

/**
 * Sets GPIO Expander pin to 0.
 *
 * @param pin the GPIO Expander pin to set to 0.
 */
void itpIOExpanderClear(ITP_IOExpander IOEXP_Port,unsigned int pin);

/**
 * Gets the value of GPIO Expander pin.
 *
 * @param pin the GPIO Expander pin to get.
 */
uint8_t itpIOExpanderGet(ITP_IOExpander IOEXP_Port,unsigned int pin);

/**
 * Gets all the value of GPIO Expander pins.
 */
uint8_t itpIOExpanderGetAllPins(ITP_IOExpander IOEXP_Port);

/**
 * Gets input status of GPIO Expander pins.
 */
uint8_t itpIOExpanderGetInputStatus(ITP_IOExpander IOEXP_Port);

/**
 * GPIO expander deferred interrupt handler.
 */
typedef void (*ITPGpioExpanderDeferIntrHandler)( void *pvParameter1, uint32_t ulParameter2 );

/**
 * Second GPIO expander deferred interrupt handler.
 */
typedef void (*ITPSecondGpioExpanderDeferIntrHandler)( void *pvParameter1, uint32_t ulParameter2 );

/**
 * Registers GPIO expander deferred interrupt handler.
 *
 * @param handler The handler function.
 */
void itpRegisterIOExpanderDeferIntrHandler(ITPGpioExpanderDeferIntrHandler handler);

/**
 * Registers Second GPIO expander deferred interrupt handler.
 *
 * @param handler The handler function.
 */
void itpRegisterSecondIOExpanderDeferIntrHandler(ITPSecondGpioExpanderDeferIntrHandler handler);

/** @} */ // end of itp_gpio_expander

#ifdef __cplusplus
}
#endif

#endif