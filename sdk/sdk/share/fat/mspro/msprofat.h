#ifndef _MSPRO_FAT_H_
#define _MSPRO_FAT_H_

#include "fat/fat.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * only one externed function
 *
 *****************************************************************************/

F_DRIVER * mspro_initfunc(unsigned long driver_param);

#ifdef __cplusplus
}
#endif

#endif /* _MSPRO_FAT_H_ */

