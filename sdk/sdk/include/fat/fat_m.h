#ifndef _FAT_M_H_
#define _FAT_M_H_

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


#ifdef HCC_UNICODE
extern int fm_wgetcwd(wchar *buffer, int maxlen );
extern int fm_wgetdcwd(int drivenum, wchar *buffer, int maxlen );
extern int fm_wchdir(const wchar *dirname);
extern int fm_wmkdir(const wchar *dirname);
extern int fm_wrmdir(const wchar *dirname);
extern int fm_wfindfirst(const wchar *filename,FN_WFIND *find);
extern int fm_wfindnext(FN_WFIND *find);
extern int fm_wrename(const wchar *filename, const wchar *newname);
extern int fm_wmove(const wchar *filename, const wchar *newname);
extern long fm_wfilelength(const wchar *filename);
extern FN_FILE *fm_wopen(const wchar *filename,const wchar *mode);
extern FN_FILE *fm_wtruncate(const wchar *filename,unsigned long length);
extern int fm_wstat(const wchar *filename,F_STAT *stat);
extern int fm_wgettimedate(const wchar *filename,unsigned short *pctime,unsigned short *pcdate);
extern int fm_wsettimedate(const wchar *filename,unsigned short ctime,unsigned short cdate);
extern int fm_wdelete(const wchar *filename);
extern int fm_wgetattr(const wchar *filename,unsigned char *attr);
extern int fm_wsetattr(const wchar *filename,unsigned char attr);
#endif


#ifdef __cplusplus
}
#endif

/****************************************************************************
 *
 * end of fat_m.h
 *
 ***************************************************************************/

#endif /* _FAT_M_H_ */
