#ifndef _NORDRV_F_H_
#define _NORDRV_F_H_

#include "fat/fat.h"

#ifdef __cplusplus
extern "C" {
#endif

extern F_DRIVER *nor_initfunc(unsigned long driver_panor);
extern int nor_getCapacity(unsigned int* sec_num, unsigned int* block_size);
extern int nor_readmultiplesector(F_DRIVER *driver,void *data, unsigned long sector, int cnt);
extern int nor_writemultiplesector(F_DRIVER *driver,void *data, unsigned long sector, int cnt);

enum {
  NOR_NO_ERROR,
  NOR_ERR_SECTOR=101,
  NOR_ERR_NOTAVAILABLE
};

#ifdef __cplusplus
}
#endif

/******************************************************************************
 *
 *  End of nordrv.c
 *
 *****************************************************************************/

#endif /* _NORDRV_H_ */
