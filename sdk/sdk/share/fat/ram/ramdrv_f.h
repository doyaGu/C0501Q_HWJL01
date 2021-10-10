#ifndef _RAMDRV_F_H_
#define _RAMDRV_F_H_

/****************************************************************************
 *
 *            Copyright (c) 2003-2008 by HCC Embedded
 *
 * This software is copyrighted by and is the sole property of
 * HCC.  All rights, title, ownership, or other interests
 * in the software remain the property of HCC.  This
 * software may only be used in accordance with the corresponding
 * license agreement.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior
 * written consent of HCC.
 *
 * HCC reserves the right to modify this software without notice.
 *
 * HCC Embedded
 * Budapest 1133
 * Vaci ut 110
 * Hungary
 *
 * Tel:  +36 (1) 450 1302
 * Fax:  +36 (1) 450 1303
 * http: www.hcc-embedded.com
 * email: info@hcc-embedded.com
 *
 ***************************************************************************/

#include "fat\fat.h"

#ifdef __cplusplus
extern "C" {
#endif

extern F_DRIVER *ram_initfunc(unsigned long driver_param);

#define F_RAM_DRIVE0 0
#define F_RAM_DRIVE1 1

enum {
  RAM_NO_ERROR,
  RAM_ERR_SECTOR=101,
  RAM_ERR_NOTAVAILABLE
};

#ifdef __cplusplus
}
#endif

/******************************************************************************
 *
 *  End of ramdrv.c
 *
 *****************************************************************************/

#endif /* _RAMDRV_H_ */
