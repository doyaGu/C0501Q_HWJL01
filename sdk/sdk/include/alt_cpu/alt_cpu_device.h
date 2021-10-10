/** @file
 * ALT CPU device library.
 *
 * @author Steven Hsiao
 * @version 1.0
 * @date 2011-2012
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup ALT CPU device library
 *  @{
 */
#ifndef ALT_CPU_DEVICE_H
#define ALT_CPU_DEVICE_H

#include "ite/itp.h"
#include "ite/ite_risc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALT_CPU_UNKNOWN_DEVICE  0
#define ALT_CPU_RSL_MASTER      1
#define ALT_CPU_RSL_SLAVE       2
#define ALT_CPU_SW_PWM          3

#define ITP_IOCTL_ALT_CPU_SWITCH_ENG    ITP_IOCTL_CUSTOM_CTL_ID0

#ifdef __cplusplus
}
#endif

#endif // ALT_CPU_DEVICE_H
/** @} */ // end of alt_cpu_device
