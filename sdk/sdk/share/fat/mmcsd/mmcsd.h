#ifndef _MMCSD_H_
#define _MMCSD_H_

/****************************************************************************
 *
 *            Copyright (c) 2003-2009 by HCC Embedded
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

#include "fat/fat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SD_ALLOW_4BIT

/******************************************************************************
 *
 * Errors in CFC
 *
 *****************************************************************************/

#define MMCSD_ERR_NOTPLUGGED (-1)
enum {
/*  0 */  MMCSD_NO_ERROR, 
/*  1 */  MMCSD_ERR_INIT,
/*  2 */  MMCSD_ERR_CMD,
/*  3 */  MMCSD_ERR_START,
/*  4 */  MMCSD_ERR_CRC,
/*  5 */  MMCSD_ERR_WRITEPROTECT,
/*  6 */  MMCSD_ERR_ACCEPT,
/*  7 */  MMCSD_ERR_BUSY,
/*  8 */  MMCSD_ERR_SPEED,
/*  9 */  MMCSD_ERR_TIMEOUT,
/* 10 */  MMCSD_ERR_TRANS
};

/******************************************************************************
 *
 * only one externed function
 *
 *****************************************************************************/

F_DRIVER * mmcsd_initfunc(unsigned long driver_param);

#ifdef __cplusplus
}
#endif

#endif /* _MMCSD_H_ */

