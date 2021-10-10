#ifndef _NORDRV_F_H_
#define _NORDRV_F_H_

#include "fat/fat.h"

#ifdef __cplusplus
extern "C" {
#endif


/** same with itp_fat.c */
struct xd_param
{
    int xd;                 //
    int removable;          // removable or not
    unsigned long reserved; // reserved size
};

extern F_DRIVER *xd_initfunc(unsigned long driver_panor);

#ifdef __cplusplus
}
#endif

/******************************************************************************
 *
 *  End of xddrv.c
 *
 *****************************************************************************/

#endif /* _XDDRV_H_ */
