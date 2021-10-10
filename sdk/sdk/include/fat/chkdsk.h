#ifndef __CHKDSK_H_
#define __CHKDSK_H_

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

#ifdef __cplusplus
extern "C" {
#endif

#define CHKDSK_LOG_ENABLE
#ifdef CHKDSK_LOG_ENABLE
#define CHKDSK_LOG_SIZE 8192
#endif
#define CHKDSK_MAX_DIR_DEPTH 64		/* max. stack= ~(CHKDSK_MAX_DIR_DEPTH*85)+1100 */

#define CHKDSK_ERASE_BAD_CHAIN 0x1	/* erase all bad chains */
#define CHKDSK_ERASE_LOST_CHAIN 0x2	/* erase all lost chains */
#define CHKDSK_ERASE_LOST_BAD_CHAIN 0x4	/* erase all lost bad chains */


enum {
  FC_NO_ERROR,
  FC_WRITE_ERROR=50,
  FC_READ_ERROR,
  FC_CLUSTER_ERROR,
  FC_ALLOCATION_ERROR
};

#if (!FN_CAPI_USED)
extern int f_checkdisk (int drivenum, int param);
#else
extern int fn_checkdisk (F_MULTI *fm,int drivenum, int param);
#endif

#ifdef __cplusplus
}
#endif

/****************************************************************************
 *
 * end of chkdsk.h
 *
 ***************************************************************************/

#endif /* __CHKDSK_H_ */
