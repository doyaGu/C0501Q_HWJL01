#ifndef _FAT_H_
#define _FAT_H_

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

#include "udefs_f.h"
#include "api_f.h"
#include "chkdsk.h"

/* F_DEF_SECTOR_SIZE is the default sector size which is always 512 */
#define F_DEF_SECTOR_SIZE	512

/* check whether FATCACHE_READAHEAD is set properly */
#if FATCACHE_READAHEAD*(F_MAX_SECTOR_SIZE/F_DEF_SECTOR_SIZE) > 256
#error The expression above cannot be more than 256
#endif

typedef struct
{
	unsigned char jump_code[3];
	unsigned char OEM_name [8];
	unsigned short bytes_per_sector;
	unsigned char sector_per_cluster;
	unsigned short reserved_sectors;
	unsigned char number_of_FATs;
	unsigned short max_root_entry;
	unsigned short number_of_sectors_less32; /*  <32M  */
	unsigned char media_descriptor;
	unsigned short sector_per_FAT;
	unsigned short sector_per_Track;
	unsigned short number_of_heads;
	unsigned long number_of_hidden_sectors;
	unsigned long number_of_sectors;

	/* only on fat32 */
	unsigned long sector_per_FAT32;
	unsigned short extflags;
	unsigned short fsversion;
	unsigned long rootcluster;
	unsigned short fsinfo;
	unsigned short bkbootsec;
	unsigned char reserved[12];

	/* fat12-fat16-fat32 */
	unsigned short logical_drive_num;
	unsigned char extended_signature;
	unsigned long serial_number;
	unsigned char volume_name[11];
	unsigned char FAT_name[8];
	unsigned char executable_marker[2];
} F_BOOTRECORD;

/* number of sectors after mbr */
#define F_SPACE_AFTER_MBR	63

#if F_LONGFILENAME

typedef struct
{
   W_CHAR name[261]; /* with zero term */
   unsigned char ord;
   unsigned char chksum;
   unsigned char state;
   unsigned long start;
   unsigned long end;
} F_LFNINT;

enum
{
/* 0 */ F_LFNSTATE_LFN,  /* lfn is useable */
/* 1 */ F_LFNSTATE_SFN,  /* lfn is useable, contains short filename */
/* 2 */ F_LFNSTATE_NEXT, /* lfn need more entry */
/* 3 */ F_LFNSTATE_INV   /* invalid lfn */
};

#endif  /* F_LONGFILENAME */

typedef struct
{
	char name[F_MAXNAME];		/* 8+3 filename */
	char ext[F_MAXEXT];			/* 8+3 extension */
	unsigned char attr;			/* 00ADVSHR */

	unsigned char ntres;
	unsigned char crttimetenth;
	unsigned char crttime[2];
	unsigned char crtdate[2];
	unsigned char lastaccessdate[2];

	unsigned char clusterhi[2]; /* FAT32 only */
	unsigned char ctime[2];
	unsigned char cdate[2];
	unsigned char clusterlo[2]; /* fat12,fat16,fat32 */
	unsigned char filesize[4];
} F_DIRENTRY;

/* 1st char in 8+3 if entry is deleted*/
#define F_DELETED_CHAR ((char)0xe5)

/* lower case name */
#define NTRES_SL_NAME 0x08
#define NTRES_SL_EXT  0x10

/* define for long filename entry in directory entry*/
#define F_ATTR_LFN      (F_ATTR_VOLUME|F_ATTR_SYSTEM|F_ATTR_HIDDEN|F_ATTR_READONLY)

#if F_LONGFILENAME

typedef struct
{
	unsigned char ord;

	unsigned char lfn_1;
	unsigned char lfnhi_1;

	unsigned char lfn_2;
	unsigned char lfnhi_2;

	unsigned char lfn_3;
	unsigned char lfnhi_3;

	unsigned char lfn_4;
	unsigned char lfnhi_4;

	unsigned char lfn_5;
	unsigned char lfnhi_5;

	unsigned char attr;	/* 00ADVSHR */
	unsigned char type;  /* always 0 */

	unsigned char chksum;

	unsigned char lfn_6;
	unsigned char lfnhi_6;

	unsigned char lfn_7;
	unsigned char lfnhi_7;

	unsigned char lfn_8;
	unsigned char lfnhi_8;

	unsigned char lfn_9;
	unsigned char lfnhi_9;

	unsigned char lfn_10;
	unsigned char lfnhi_10;

	unsigned char lfn_11;
	unsigned char lfnhi_11;

	unsigned char clusterlo[2]; /* fat12,fat16,fat32 */

	unsigned char lfn_12;
	unsigned char lfnhi_12;

	unsigned char lfn_13;
	unsigned char lfnhi_13;

} F_LFN;

#endif /* F_LONGFILENAME  */

/* definitions for FAT entry */
#define F_CLUSTER_FREE	   ((unsigned long)0x00000000)
#define F_CLUSTER_RESERVED ((unsigned long)0x0ffffff0)
#define F_CLUSTER_BAD	   ((unsigned long)0x0ffffff7)
#define F_CLUSTER_LAST	   ((unsigned long)0x0ffffff8)
#define F_CLUSTER_LASTF32R ((unsigned long)0x0fffffff)

typedef struct
{
	unsigned long sector;  /* start sector */
	unsigned long num;     /* number of sectors */
} F_SECTOR;


#if F_MAXFILES>0xffff     /* maximum number of files */
#error F_MAXFILES should be less than 65535
#elif F_MAXFILES>0x7fff
#define F_MAXFILES_SHIFT 16
#elif F_MAXFILES>0x3fff
#define F_MAXFILES_SHIFT 15
#elif F_MAXFILES>0x1fff
#define F_MAXFILES_SHIFT 14
#elif F_MAXFILES>0x0fff
#define F_MAXFILES_SHIFT 13
#elif F_MAXFILES>0x07ff
#define F_MAXFILES_SHIFT 12
#elif F_MAXFILES>0x03ff
#define F_MAXFILES_SHIFT 11
#elif F_MAXFILES>0x01ff
#define F_MAXFILES_SHIFT 10
#elif F_MAXFILES>0x00ff
#define F_MAXFILES_SHIFT 9
#elif F_MAXFILES>0x007f
#define F_MAXFILES_SHIFT 8
#elif F_MAXFILES>0x003f
#define F_MAXFILES_SHIFT 7
#elif F_MAXFILES>0x001f
#define F_MAXFILES_SHIFT 6
#elif F_MAXFILES>0x000f
#define F_MAXFILES_SHIFT 5
#else
#define F_MAXFILES_SHIFT 4
#endif

/*  definitions for FN_FILE internally used  */

typedef struct
{
	int N;
	char *ptr;
#ifdef USE_MALLOC
	F_POS *pos;
#else
	F_POS *pos;
	F_POS posbuf[WR_DATACACHE_SIZE];
#endif
} t_WrDataCache;

typedef struct FN_FILEINT FN_FILEINT;

typedef struct FN_FILEINT
{
	FN_FILE file;
	long modified;
    int drivenum; 	/*  0-A 1-B 2-C  */
	unsigned long abspos;
	unsigned long relpos;
	unsigned long filesize;
	unsigned char data[F_MAX_SECTOR_SIZE];
	int datawritten;
	t_WrDataCache WrDataCache;
	unsigned long startcluster;
	F_POS pos;
	F_POS dirpos;
	long state;
#if F_MAXSEEKPOS
	long seekpos[F_MAXSEEKPOS];
	long seekprev[F_MAXSEEKPOS];
	long seekshift;
#endif
	FN_FILEINT *syncfile;
	char mode;
	char dummy[3];

#if F_FILE_CHANGED_EVENT
	char filename [FN_MAXPATH];
#endif
} _FN_FILEINT;

/* this bit signal if synchronization is required in append and read in state */
#define F_FILE_ST_SYNC 0x0001
#define F_FILE_ST_EOF  0x0002

typedef struct
{
	unsigned long clfree;
	unsigned long clused;
	unsigned long clbad;
} F_CLSPACE;

#ifdef FATCACHE_ENABLE
#define FATCACHE_SIZE (FATCACHE_BLOCKS*FATCACHE_READAHEAD)
typedef struct
{
	unsigned long sector;
	unsigned char modified;
} t_CacheDsc;						

typedef struct
{
	unsigned int N;			/* number of sectors in the cache */
	unsigned int Npos;		/* position in the cache */
	t_CacheDsc *dsc;
#ifdef USE_MALLOC
	unsigned char *data;
#else
	t_CacheDsc dsc_array[FATCACHE_SIZE*(F_MAX_SECTOR_SIZE/F_DEF_SECTOR_SIZE)];
	unsigned char data[FATCACHE_SIZE*F_MAX_SECTOR_SIZE];
#endif
} t_FatCache;
#endif

typedef struct
{
	long state;

	F_BOOTRECORD bootrecord;

	F_SECTOR firstfat;

	F_SECTOR root;
	F_SECTOR data;

#ifdef FATCACHE_ENABLE
	t_FatCache fatcache;
	unsigned char *fat;
#else
	unsigned char fat[F_MAX_SECTOR_SIZE];
#endif
	unsigned long fatsector;
	long fatmodified;

#if F_LONGFILENAME
#ifdef DIRCACHE_ENABLE
#ifdef USE_MALLOC
	unsigned char *dircache;
#else
	unsigned char dircache[DIRCACHE_SIZE*F_MAX_SECTOR_SIZE];
#endif
	unsigned long dircache_start;
	unsigned long dircache_size;
#endif
#endif
	unsigned char direntry[F_MAX_SECTOR_SIZE];
	unsigned long direntrysector;

	unsigned long lastalloccluster;

#if USE_TASK_SEPARATED_CWD
	W_CHAR *cwd;				/* current working folders in this volume points to task cwd */
#else
	W_CHAR cwd[FN_MAXPATH];		/* current working folders in this volume */
#endif

	long mediatype;
	F_CLSPACE clspace;	/* calculated disk space */
	char cspaceok;
#if defined FATBITFIELD_ENABLE && defined USE_MALLOC
	unsigned char *fatbitfield;
#endif
	int partition;
	unsigned long sectorstart;
	unsigned long sectornum;
	F_PHY phy;

	F_DRIVER *driver;

	unsigned char sectorbuffer[F_MAX_SECTOR_SIZE];

#if F_LONGFILENAME
	unsigned char pbitp[64];
#endif

} F_VOLUME;

#define F_FAT12_MAX_CLUSTER 0xFF0
#define F_FAT16_MAX_CLUSTER 0xFFF0

typedef struct {
	F_VOLUME volumes[FN_MAXVOLUME];	/* volumes */
	FN_FILEINT files[F_MAXFILES];
	unsigned long drvbldnum;	    /* drive build number for file.reference */
#if (!USE_TASK_SEPARATED_CWD)
	int f_curdrive;
#endif
} FN_FILESYSTEM;

extern FN_FILESYSTEM f_filesystem;

#if (!FN_CAPI_USED)
typedef struct
{
	long ID;						/* task id */

#if USE_TASK_SEPARATED_CWD
	int f_curdrive;					/* current drive */
	struct
	{
		W_CHAR cwd[FN_MAXPATH];		/* current working folders in this volume */
	} f_vols[FN_MAXVOLUME];
#endif

	FN_MUTEX_TYPE *pmutex;
	unsigned char current_bank;
	int lasterror;					/* last error in this task */
} F_MULTI;
#endif

/*  current file opening modes */

enum
{
/*	0 */ FN_FILE_CLOSE,
/*	1 */ FN_FILE_RD,
/*	2 */ FN_FILE_WR,
/*	3 */ FN_FILE_A,
/*	4 */ FN_FILE_RDP,
/*	5 */ FN_FILE_WRP,
/*	6 */ FN_FILE_AP,
/*	7 */ FN_FILE_WRERR,
/*	8 */ FN_FILE_LOCKED
};

#define FN_FILE_ABORT_FLAG 0x40 /* signal for file is aborted */

/*  current drive modes */

enum {
/*  0 */ F_STATE_NONE,
/*  1 */ F_STATE_NEEDMOUNT,
/*  2 */ F_STATE_WORKING,
/*  3 */ F_STATE_WORKING_WP //working but write protected
};

/****************************************************************************
 *
 * externed functions
 *
 ***************************************************************************/

extern int fn_delvolume (F_MULTI *fm,int drvnumber);
extern int fn_get_volume_count (F_MULTI *fm);
extern int fn_get_volume_list (F_MULTI *fm,int *buf);
extern int fn_checkvolume(F_MULTI *fm,int drvnumber);
extern int fn_format(F_MULTI *fm,int drivenum,long fattype);
extern int fn_getcwd(F_MULTI *fm,char *buffer, int maxlen );
extern int fn_getdcwd(F_MULTI *fm,int drivenum, char *buffer, int maxlen );
extern int fn_chdrive(F_MULTI *fm,int drivenum);
extern int fn_getdrive(F_MULTI *fm);
extern int fn_getfreespace(F_MULTI *fm,int drivenum, FN_SPACE *pspace);

extern int fn_chdir(F_MULTI *fm,const char *dirname);
extern int fn_mkdir(F_MULTI *fm,const char *dirname);
extern int fn_rmdir(F_MULTI *fm,const char *dirname);

extern int fn_findfirst(F_MULTI *fm,const char *filename,FN_FIND *find);
extern int fn_findnext(F_MULTI *fm,FN_FIND *find);
extern int fn_rename(F_MULTI *fm,const char *filename, const char *newname);
extern int fn_move(F_MULTI *fm,const char *filename, const char *newname);
extern long fn_filelength(F_MULTI *fm,const char *filename);

extern int fn_close(F_MULTI *fm,FN_FILE *filehandle);
extern int fn_flush(F_MULTI *fm,FN_FILE *file);
extern FN_FILE *fn_open(F_MULTI *fm,const char *filename,const char *mode);
extern FN_FILE *fn_truncate(F_MULTI *fm,const char *filename,unsigned long length);

extern long fn_read(F_MULTI *fm,void *buf,long size,long size_st,FN_FILE *filehandle);
extern long fn_write(F_MULTI *fm,const void *buf,long size,long size_st,FN_FILE *filehandle);

extern int fn_seek(F_MULTI *fm,FN_FILE *filehandle,long offset,long whence);
extern int fn_seteof(F_MULTI *fm,FN_FILE *filehandle);

extern long fn_tell(F_MULTI *fm,FN_FILE *filehandle);
extern int fn_getc(F_MULTI *fm,FN_FILE *filehandle);
extern int fn_putc(F_MULTI *fm,int ch,FN_FILE *filehandle);
extern int fn_rewind(F_MULTI *fm,FN_FILE *filehandle);
extern int fn_eof(F_MULTI*,FN_FILE *filehandle);

extern int fn_gettimedate(F_MULTI *fm,const char *filename,unsigned short *pctime,unsigned short *pcdate);
extern int fn_settimedate(F_MULTI *fm,const char *filename,unsigned short ctime,unsigned short cdate);
extern int fn_delete(F_MULTI *fm,const char *filename, unsigned char delcontent);
extern int fn_stat(F_MULTI *fm,const char *filename,F_STAT *stat);

extern int fn_getattr(F_MULTI *fm,const char *filename,unsigned char *attr);
extern int fn_setattr(F_MULTI *fm,const char *filename,unsigned char attr);

extern int fn_getlabel(F_MULTI *fm,int drivenum, char *label, long len);
extern int fn_setlabel(F_MULTI *fm,int drivenum, const char *label);

extern int fn_get_oem(F_MULTI *fm, int drivenum, char *str, long maxlen);

#ifdef HCC_UNICODE
extern int fn_wgetcwd(F_MULTI *fm,wchar *buffer, int maxlen );
extern int fn_wgetdcwd(F_MULTI *fm,int drivenum, wchar *buffer, int maxlen );
extern int fn_wchdir(F_MULTI *fm,const wchar *dirname);
extern int fn_wmkdir(F_MULTI *fm,const wchar *dirname);
extern int fn_wrmdir(F_MULTI *fm,const wchar *dirname);
extern int fn_wfindfirst(F_MULTI *fm,const wchar *filename,FN_WFIND *find);
extern int fn_wfindnext(F_MULTI *fm,FN_WFIND *find);
extern int fn_wrename(F_MULTI *fm,const wchar *filename, const wchar *newname);
extern int fn_wmove(F_MULTI *fm,const wchar *filename, const wchar *newname);
extern long fn_wfilelength(F_MULTI *fm,const wchar *filename);
extern FN_FILE *fn_wopen(F_MULTI *fm,const wchar *filename,const wchar *mode);
extern FN_FILE *fn_wtruncate(F_MULTI *fm,const wchar *filename,unsigned long length);
extern int fn_wstat(F_MULTI *fm,const wchar *filename,F_STAT *stat);
extern int fn_wgettimedate(F_MULTI *fm,const wchar *filename,unsigned short *pctime,unsigned short *pcdate);
extern int fn_wsettimedate(F_MULTI *fm,const wchar *filename,unsigned short ctime,unsigned short cdate);
extern int fn_wdelete(F_MULTI *fm,const wchar *filename,unsigned char delcontent);
extern int fn_wgetattr(F_MULTI *fm,const wchar *filename,unsigned char *attr);
extern int fn_wsetattr(F_MULTI *fm,const wchar *filename,unsigned char attr);
#endif

#include "fat_m.h"

#define _f_toupper(ch) (((ch)>='a' && (ch)<='z') ? ((ch)-'a'+'A') : (ch))

#ifdef HCC_UNICODE
extern wchar *_towchar(wchar *nconv, const char *s);
#endif
extern int _f_addentry(F_VOLUME *vi,F_NAME *fsname,F_POS *pos,F_DIRENTRY **pde);
extern int _f_getdirsector(F_VOLUME *vi,unsigned long sector);

#ifdef __cplusplus
}
#endif

/****************************************************************************
 *
 * end of fat.h
 *
 ***************************************************************************/

#endif /* _FAT_H_ */

