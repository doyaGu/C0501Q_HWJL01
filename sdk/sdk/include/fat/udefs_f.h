#ifndef _UDEFS_F_H_
#define _UDEFS_F_H_

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

#include <sys/syslimits.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 *
 * enable this if CAPI (Common API) is used
 *
 ***************************************************************************/

#define FN_CAPI_USED 0

/****************************************************************************
 *
 * OEM name
 *
 ***************************************************************************/

#define OEM_NAME "MSDOS5.0"
/*#define OEM_NAME "EFFSFAT"*/

/****************************************************************************
 *
 * CAPI selected includes
 *
 ***************************************************************************/

#if FN_CAPI_USED
#include "../../fw_port.h"
#else

/****************************************************************************
 *
 *	if Unicode is used then comment in HCC_UNICODE define
 *
 ***************************************************************************/
#define HCC_UNICODE

#ifndef HCC_UNICODE
#define F_LONGFILENAME 0 /*  0 - 8+3 names   1 - long file names   */
#define W_CHAR char
#else
#define F_LONGFILENAME 1 /* don't change it, because unicode version alvays uses long file name */
#define W_CHAR wchar_t
#endif

#ifdef HCC_UNICODE
typedef wchar_t wchar;
#endif

/****************************************************************************
 *
 * volumes definitions
 *
 ***************************************************************************/

#define FN_MAXVOLUME	26	        /* maximum number of volumes */
#define FN_MAXTASK		CHILD_MAX	/* maximum number of task */

#define FN_MAXPATH		PATH_MAX	/* maximum allowed filename or pathname */

#define FN_CURRDRIVE	0	        /* setting the current drive at startup (-1 means no default current drive)*/

#define FN_MUTEX_TYPE unsigned long

/* select path separator */
#if 1
#define F_SEPARATORCHAR '/'
#else
#define F_SEPARATORCHAR '\\'
#endif

/****************************************************************************
 *
 * Last error usage
 *
 ***************************************************************************/

#if 0
/* simple asignment */
#define F_SETLASTERROR(ec) (fm->lasterror=(ec))
#define F_SETLASTERROR_NORET(ec) (fm->lasterror=(ec))
#elif 1
/* function calls used for it */
#define F_SETLASTERROR(ec) fn_setlasterror(fm,ec)
#define F_SETLASTERROR_NORET(ec) fn_setlasterror_noret(fm,ec)
#elif 0
/* no last error is used (save code space) */
#define F_SETLASTERROR(ec) (ec)
#define F_SETLASTERROR_NORET(ec)
#endif

/****************************************************************************
 *
 * Set F_FILE_CHANGED_EVENT to 1 if want event about file state changed
 *
 ***************************************************************************/

#define F_FILE_CHANGED_EVENT 0

/****************************************************************************
 *
 * set F_SUPPORT_TI64K to 1 when TI DSP is used. 
 *
 ***************************************************************************/

#define F_SUPPORT_TI64K 0

/****************************************************************************
 *
 * set USE_TASK_SEPARATED_CWD to 1 (default) if every task need separated
 * current working folder.
 *
 ***************************************************************************/

#define	USE_TASK_SEPARATED_CWD 1

/****************************************************************************
 *
 * Close bracket for non CAPI
 *
 ***************************************************************************/

#endif  /* FN_CAPI_USED */

/****************************************************************************
 *
 * Common defines (for non CAPI and CAPI)
 *
 ***************************************************************************/

/****************************************************************************
 *
 * F_MAXFILES cannot be less than 2 because FAT filesystem needs 1 file for
 * internally using. So If F_MAXFILES is set to 2 then user can use 1 file
 * for opening with safety the other is reserved for the system
 *
 ***************************************************************************/

#define F_MAXFILES  OPEN_MAX     /* maximum number of files */

#define F_MAXSEEKPOS 32	   /* number of division of fast seeking */

/****************************************************************************
 *
 * functions definitions
 *
 ***************************************************************************/

/* Use internal mem functions (memcpy,memset) or switch to library functions */
/*#define INTERNAL_MEMFN*/

/* Use malloc for cache items */
#define USE_MALLOC

#ifdef USE_MALLOC
#include <stdlib.h>
#endif

#ifdef USE_MALLOC
#define __malloc(x) malloc(x)		/* normally use malloc from library */
#define __free(x) free(x)		    /* normally use free from library */
#endif

/* maximum supported sector size */
#define F_MAX_SECTOR_SIZE 512

/* Enable FAT caching */
#define FATCACHE_ENABLE
#if F_LONGFILENAME
#define DIRCACHE_ENABLE
#endif

/* define of allocation of faster searching mechanism */
#ifdef USE_MALLOC
#define FATBITFIELD_ENABLE
#endif

#ifdef FATCACHE_ENABLE
#define FATCACHE_BLOCKS 2
#define FATCACHE_READAHEAD 4	/* max. 256 depending on F_MAX_SECTOR_SIZE */
#endif

#if F_LONGFILENAME
#ifdef DIRCACHE_ENABLE
#define DIRCACHE_SIZE 16	/* max. 32 (<=max. cluster size) */
#endif
#endif

#define WR_DATACACHE_SIZE 128	/* min. 1 !!!! */




#ifdef INTERNAL_MEMFN
#define _memcpy(d,s,l) _f_memcpy(d,s,l)
#define _memset(d,c,l) _f_memset(d,c,l)
#else
#include <string.h>
#define _memcpy(d,s,l) memcpy(d,s,l)
#define _memset(d,c,l) memset(d,c,l)
#endif

/****************************************************************************
 *
 * Last access date
 *
 ***************************************************************************/

#define F_UPDATELASTACCESSDATE 0
/* it defines if a file is opened for read to update lastaccess time */

/****************************************************************************
 *
 * Opened file size
 *
 ***************************************************************************/

#define F_FINDOPENFILESIZE 1
/* set F_FINDOPENFILESIZE to 0 if filelength needs to return with 0 for an opened file  */
/* other case filelength functions can return with opened file length also */

/****************************************************************************
 *
 * Removing enterFS function
 *
 * Backward compatibility for f_enterFS. If F_REMOVEENTERFS is set to 1 then
 * no f_enterFS call is necessary to be called. When this is enabled then 
 * some function will returns with F_ERR_TASKNOTFOUND instead of 
 * F_ERR_NOMOREENTRY or F_ERR_ALLOCATION. Also some error codes won't be
 * signaled with f_getlasterror. Don't call f_enterFS when F_REMOVEENTERFS
 * is set to 1. Normal setting of F_REMOVEENTERFS is 0. Please change 
 * this only for backward compatibility if necessary, other case add 
 * f_enterFS calls into your code.
 *
 ***************************************************************************/

#define F_REMOVEENTERFS 0

/****************************************************************************
 *
 * F_DELETE_CONTENT can be set to 1 when f_delete_content function is needed
 * to be used. This function delete file content from the disk (depending
 * on media type).
 *
 ***************************************************************************/

#define F_DELETE_CONTENT 0

/****************************************************************************
 *
 * F_MAXFILES cannot be less than 2 because FAT filesystem needs 1 file for
 * internally using. So If F_MAXFILES is set to 2 then user can use 1 file
 * for opening with safety the other is reserved for the system
 *
 ***************************************************************************/

#if F_MAXFILES<2
#error "F_MAXFILES cannot be less than 2"
#endif

/****************************************************************************
 *
 * closing bracket for C++
 *
 ***************************************************************************/

#ifdef __cplusplus
}
#endif

/****************************************************************************
 *
 * end of udefs_f.h
 *
 ***************************************************************************/

#endif /* _UDEFS_F_H_ */

