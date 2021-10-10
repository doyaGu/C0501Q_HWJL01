/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL HCC functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/syslimits.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "fat/fat.h"
#include "itp_cfg.h"

extern long fn_gettaskID(void);
static bool fatInited;

// SD driver
extern F_DRIVER *mmcsd_initfunc(unsigned long driver_param);

struct
{
    int sd;                 // sd0 or sd1
    int removable;          // removable or not
    unsigned long reserved; // reserved size
} sdDrvParams[2] =
{
    {
        0,
    #ifdef CFG_SD0_STATIC
        0,
        CFG_SD0_RESERVED_SIZE
    #else
        1,
        0
    #endif // CFG_SD0_STATIC
    },
    {
        1,
    #ifdef CFG_SD1_STATIC
        0,
        CFG_SD1_RESERVED_SIZE
    #else
        1,
        0
    #endif // CFG_SD1_STATIC
    },
};


static int FatFormat(int volume);

// XD driver
extern F_DRIVER *xd_initfunc(unsigned long driver_param);

// NAND driver
extern F_DRIVER *ftl_initfunc(unsigned long driver_param);

// NOR driver
extern F_DRIVER *nor_initfunc(unsigned long driver_param);

#ifdef CFG_NOR_ENABLE
struct
{
    unsigned long reserved;     // reserved size
    unsigned long cacheSize;    // cache size
} norDrvParam =
{
    CFG_NOR_RESERVED_SIZE,
    CFG_NOR_CACHE_SIZE
};
#endif // CFG_NOR_ENABLE

// ms driver
#ifdef CFG_MS_ENABLE
extern F_DRIVER *mspro_initfunc(unsigned long driver_param);
#else
#define mspro_initfunc    NULL
#endif

// usb msc driver
#ifdef CFG_MSC_ENABLE
extern F_DRIVER *msc_initfunc(unsigned long driver_param);
#else
#define msc_initfunc    NULL
#endif

// RAM disk driver
extern F_DRIVER *ram_initfunc(unsigned long driver_param);

typedef struct
{
    F_DRIVER* driver;
    F_DRIVERINIT initfunc;
    unsigned long param;
} Driver;

static Driver driverTable[ITH_DISK_MAX] =
{
    {
        NULL,
#ifdef CFG_SD0_ENABLE
        mmcsd_initfunc,
        (unsigned long)&sdDrvParams[0]
#else
        NULL,
        0
#endif // CFG_SD0_ENABLE
    },
    {
        NULL,
#ifdef CFG_SD1_ENABLE
        mmcsd_initfunc,

        (unsigned long)&sdDrvParams[1]
#else
        NULL,
        0
#endif
    },
    { NULL, NULL, 0 },
    { NULL, mspro_initfunc, 0 },
    {
        NULL,
#ifdef CFG_XD_ENABLE
        xd_initfunc,
#else
        NULL,
#endif
        0
    },
    {
        NULL,
#ifdef CFG_NAND_ENABLE
        ftl_initfunc,
        CFG_NAND_RESERVED_SIZE
#else
        NULL,
        0
#endif
    },
    {
        NULL,
#ifdef CFG_NOR_ENABLE
        nor_initfunc,
        (unsigned long)&norDrvParam
#else
        NULL,
        0
#endif
    },
    { NULL, msc_initfunc, 0 },
    { NULL, msc_initfunc, 1 },
    { NULL, msc_initfunc, 2 },
    { NULL, msc_initfunc, 3 },
    { NULL, msc_initfunc, 4 },
    { NULL, msc_initfunc, 5 },
    { NULL, msc_initfunc, 6 },
    { NULL, msc_initfunc, 7 },
    { NULL, msc_initfunc, 8 },
    { NULL, msc_initfunc, 9 },
    { NULL, msc_initfunc, 10 },
    { NULL, msc_initfunc, 11 },
    { NULL, msc_initfunc, 12 },
    { NULL, msc_initfunc, 13 },
    { NULL, msc_initfunc, 14 },
    { NULL, msc_initfunc, 15 },
    {
        NULL,
#ifdef CFG_RAMDISK_ENABLE
        ram_initfunc,
#else
        NULL,
#endif
        F_AUTO_ASSIGN
    },
};

static F_FILE* openFiles[OPEN_MAX];

static int FatOpen(const char* name, int flags, int mode, void* info)
{
    F_FILE* file;
    int i;

#ifdef ITP_FAT_UTF8
    const wchar_t* fmode;
    wchar_t buf[PATH_MAX + 1];
    
    if (flags & O_APPEND)
    {
        if (flags & O_RDWR)
            fmode = L"a+";
        else
            fmode = L"a";
    }
    else if ((flags & O_CREAT) && !(flags & O_RDWR))
    {
        fmode = L"w";
    }
    else
    {
        if (flags & O_RDWR)
            fmode = L"r+";
        else
            fmode = L"r";
    }
    mbstowcs(buf, name, PATH_MAX + 1);
    file = f_wopen(buf, fmode);

    if (!file && (flags & O_CREAT) && (flags & O_RDWR))
    {
        fmode = L"w+";
        file = f_wopen(buf, fmode);
    }

#else
    const char* fmode;
    
    if (flags & O_APPEND)
    {
        if (flags & O_RDWR)
            fmode = "a+";
        else
            fmode = "a";
    }
    else if ((flags & O_CREAT) && !(flags & O_RDWR))
    {
        fmode = "w";
    }
    else
    {
        if (flags & O_RDWR)
            fmode = "r+";
        else
            fmode = "r";
    }
    file = f_open(name, fmode);
    
    if (!file && (flags & O_CREAT) && (flags & O_RDWR))
    {
        fmode = "w+";
        file = f_open(buf, fmode);
    }    
    
#endif // ITP_FAT_UTF8
    if (!file)
    {
        LOG_WARN "fat open %s fail: %d\n", name, f_getlasterror() LOG_END
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | f_getlasterror();
        return -1;
    }

    // find empty slot
    ithEnterCritical();
    for (i = 0; i < OPEN_MAX; i++)
    {
        if (openFiles[i] == NULL)
        {
            openFiles[i] = file;
            ithExitCritical();
            return i;
        }
    }
    ithExitCritical();

    errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | __LINE__;
    return -1;
}

static int FatClose(int file, void* info)
{
    int ret = f_close(openFiles[file]);

    ithEnterCritical();
    openFiles[file] = NULL;
    ithExitCritical();

    if (ret)
    {
        LOG_ERR "fat close %d fail: %d\n", file, f_getlasterror() LOG_END
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
        return -1;
    }
    return 0;
}

static int FatRead(int file, char *ptr, int len, void* info)
{
    int ret;

#ifdef CFG_DBG_STATS_FAT
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
#endif
    ret = f_read(ptr, 1, len, openFiles[file]);

#ifdef CFG_DBG_STATS_FAT
    gettimeofday(&t2, NULL);

    ithEnterCritical();
    itpStatsFat.readTime += itpTimevalDiff(&t1, &t2);
    itpStatsFat.readSize += len;
    ithExitCritical();

#endif // CFG_DBG_STATS_FAT

    return ret;
}

static int FatWrite(int file, char *ptr, int len, void* info)
{
    int ret;

#ifdef CFG_DBG_STATS_FAT
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
#endif
    ret = f_write(ptr, 1, len, openFiles[file]);

#ifdef CFG_DBG_STATS_FAT
    gettimeofday(&t2, NULL);

    ithEnterCritical();
    itpStatsFat.writeTime += itpTimevalDiff(&t1, &t2);
    itpStatsFat.writeSize += len;
    ithExitCritical();

#endif // CFG_DBG_STATS_FAT

    return ret;
}

static int FatLseek(int file, int ptr, int dir, void* info)
{
    int ret = f_seek(openFiles[file], ptr, dir);
    if (ret)
    {
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
        return -1;
    }
    return f_tell(openFiles[file]);
}

static int FatRemove(const char *path)
{
    int ret;
    
#ifdef ITP_FAT_UTF8
    wchar_t buf[PATH_MAX + 1];

    mbstowcs(buf, path, PATH_MAX + 1);
    ret = f_wdelete(buf);

#else
    ret = f_delete(path);
    
#endif // ITP_FAT_UTF8

    if (ret)
    {
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
        return -1;
    }
    return 0;
}

static int FatRename(const char *oldname, const char *newname)
{
    int ret;
    
#ifdef ITP_FAT_UTF8
    wchar_t oldbuf[PATH_MAX + 1], newbuf[PATH_MAX + 1];

    mbstowcs(oldbuf, oldname, PATH_MAX + 1);
    mbstowcs(newbuf, newname, PATH_MAX + 1);
    ret = f_wrename(oldbuf, newbuf);

#else
    ret = f_rename(oldname, newname);
    
#endif // ITP_FAT_UTF8

    if (ret)
    {
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
        return -1;
    }
    return 0;
}

static int FatChdir(const char *path)
{
    int ret;

    if (path[1] == ':')
    {
        ret = f_chdrive(toupper(path[0]) - 'A');
        if (ret)
        {
            errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
            return -1;
        }
    }
    
#ifdef ITP_FAT_UTF8
    {
        wchar_t buf[PATH_MAX + 1];
        
        mbstowcs(buf, path, PATH_MAX + 1);
        ret = f_wchdir(buf);
    }
#else
    ret = f_chdir(path);
    
#endif // ITP_FAT_UTF8

    if (ret)
    {
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
        return -1;
    }
    return 0;
}

static int FatChmod(const char *path, mode_t mode)
{
    int ret;
    unsigned char attr = 0;
    
    if ((mode & S_IREAD) == 0)
        attr |= F_ATTR_HIDDEN;

    if ((mode & S_IWRITE) == 0)
        attr |= F_ATTR_READONLY;

    if ((mode & S_IEXEC) == 0)
        attr |= F_ATTR_ARC;

#ifdef ITP_FAT_UTF8
    {
        wchar_t buf[PATH_MAX + 1];
    
        mbstowcs(buf, path, PATH_MAX + 1);
        ret = f_wsetattr(buf, attr);
    }
#else
    ret = f_setattr(path, attr);
    
#endif // ITP_FAT_UTF8

    if (ret)
    {
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
        return -1;
    }
    return 0;
}

static int FatMkdir(const char *path, mode_t mode)
{
    int ret;
    
#ifdef ITP_FAT_UTF8
    wchar_t buf[PATH_MAX + 1];

    mbstowcs(buf, path, PATH_MAX + 1);
    ret = f_wmkdir(buf);

#else
    ret = f_mkdir(path);
    
#endif // ITP_FAT_UTF8

    if (ret)
    {
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
        return -1;
    }
    return 0;
}

static int FatStat(const char *path, struct stat *sbuf)
{
    int ret;
    F_STAT s;
    struct tm t;
        
#ifdef ITP_FAT_UTF8
    wchar_t buf[PATH_MAX + 1];

    mbstowcs(buf, path, PATH_MAX + 1);
    ret = f_wstat(buf, &s);

#else
    ret = f_stat(path, &s);
    
#endif // ITP_FAT_UTF8

    if (ret)
    {
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
        return -1;
    }

    sbuf->st_dev    = ITP_DEVICE_FAT;
    sbuf->st_ino    = s.drivenum;

    sbuf->st_mode   = 0;
    if ((s.attr & F_ATTR_HIDDEN) == 0)
        sbuf->st_mode |= S_IREAD;

    if ((s.attr & F_ATTR_READONLY) == 0)
        sbuf->st_mode |= S_IWRITE;

    if ((s.attr & F_ATTR_ARC) == 0)
        sbuf->st_mode |= S_IEXEC;

    if (s.attr & F_ATTR_DIR)
        sbuf->st_mode |= S_IFDIR;
    else
        sbuf->st_mode |= S_IFREG;

    sbuf->st_size   = s.filesize;

    t.tm_sec        = 0;
    t.tm_min        = 0;
    t.tm_hour       = 0;
    t.tm_mday       = (s.lastaccessdate & F_CDATE_DAY_MASK) >> F_CDATE_DAY_SHIFT;
    t.tm_mon        = ((s.lastaccessdate & F_CDATE_MONTH_MASK) >> F_CDATE_MONTH_SHIFT) - 1;
    t.tm_year       = 1980 - 1900 + ((s.lastaccessdate & F_CDATE_YEAR_MASK) >> F_CDATE_YEAR_SHIFT);
    t.tm_isdst      = -1;
    sbuf->st_atime  = mktime(&t);
    
    t.tm_sec        = ((s.modifiedtime & F_CTIME_SEC_MASK) >> F_CTIME_SEC_SHIFT) << 1;
    t.tm_min        = (s.modifiedtime & F_CTIME_MIN_MASK) >> F_CTIME_MIN_SHIFT;
    t.tm_hour       = (s.modifiedtime & F_CTIME_HOUR_MASK) >> F_CTIME_HOUR_SHIFT;
    t.tm_mday       = (s.modifieddate & F_CDATE_DAY_MASK) >> F_CDATE_DAY_SHIFT;
    t.tm_mon        = ((s.modifieddate & F_CDATE_MONTH_MASK) >> F_CDATE_MONTH_SHIFT) - 1;
    t.tm_year       = 1980 - 1900 + ((s.modifieddate & F_CDATE_YEAR_MASK) >> F_CDATE_YEAR_SHIFT);
    sbuf->st_mtime  = mktime(&t);
    
    return 0;
}

static int FatStatvfs(const char *path, struct statvfs *sbuf)
{
    int ret;
    int volume = toupper(path[0]) - 'A';
    F_PHY phy;
    F_SPACE space;
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus;
    Driver* driver;
    uint64_t total, free;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);
    driveStatus = &driveStatusTable[volume];
    driver = &driverTable[driveStatus->disk];

    ret = driver->driver->getphy(driver->driver, &phy);
    if (ret)
    {
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
        return -1;
    }
    
	if (!phy.bytes_per_sector)
		phy.bytes_per_sector = F_DEF_SECTOR_SIZE;
    
    ret = f_getfreespace(volume, &space);
    if (ret)
    {
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
        return -1;
    }

    sbuf->f_bsize    = phy.bytes_per_sector;                        /* file system block size */
    sbuf->f_frsize   = phy.bytes_per_sector;                        /* fragment size */
    total = ((uint64_t)(space.total_high) << 32) | space.total;
    sbuf->f_blocks   = (fsblkcnt_t)(total / phy.bytes_per_sector);  /* size of fs in f_frsize units */
    free = ((uint64_t)(space.free_high) << 32) | space.free;
    sbuf->f_bfree    = (fsblkcnt_t)(free / phy.bytes_per_sector);   /* free blocks in fs */
    sbuf->f_bavail   = sbuf->f_bfree;                               /* free blocks avail to non-superuser */
    sbuf->f_files    = -1;                                          /* total file nodes in file system */
    sbuf->f_ffree    = -1;                                          /* free file nodes in fs */
    sbuf->f_favail   = -1;                                          /* avail file nodes in fs */
    sbuf->f_fsid     = ITP_DEVICE_FAT;                              /* file system id */
    sbuf->f_flag     = -1;	                                        /* mount flags */
    sbuf->f_namemax  = FILENAME_MAX;                                /* maximum length of filenames */

    return 0;
}

extern FN_FILEINT *_f_check_handle(FN_FILE *filehandle);

static int FatFstat(int file, struct stat *st)
{
	FN_FILEINT* f = _f_check_handle(openFiles[file]);
    if (!f)
    {
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        return -1;
    }

    st->st_dev    = ITP_DEVICE_FAT;
    st->st_ino    = f->drivenum;
    st->st_mode   = 0;
    st->st_size   = f->filesize;
    return 0;
}

static char* FatGetcwd(char *buf, size_t size)
{
    int ret;
    
#ifdef ITP_FAT_UTF8
    wchar_t wbuf[PATH_MAX + 1];

    ret = f_wgetcwd(wbuf, size);
    if (ret == 0)
        wcstombs(buf, wbuf, size);
        
#else
    ret = f_getcwd(buf, size);
    
#endif // ITP_FAT_UTF8

    if (ret)
    {
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
        return NULL;
    }   
    return buf;
}

static int FatRmdir(const char *path)
{
    int ret;
    
#ifdef ITP_FAT_UTF8
    wchar_t buf[PATH_MAX + 1];

    mbstowcs(buf, path, PATH_MAX + 1);
    ret = f_wrmdir(buf);

#else
    ret = f_rmdir(path);
    
#endif // ITP_FAT_UTF8

    if (ret)
    {
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
        return -1;
    }   
    return 0;
}

static int FatClosedir(DIR *dirp)
{
    free(dirp->dd_buf);
    free(dirp);
    return 0;
}

static DIR* FatOpendir(const char *dirname)
{
    int ret;
    DIR* dirp = malloc(sizeof (DIR));
    if (!dirp)
    {
        errno = ENOMEM;
        return NULL;
    }

#ifdef ITP_FAT_UTF8
    {
        wchar_t buf[PATH_MAX + 1];
        FN_WFIND* find = malloc(sizeof (FN_WFIND) + sizeof (struct dirent) + strlen(dirname) + 1);
        if (!find)
        {
            free(dirp);
            errno = ENOMEM;
            return NULL;
        }
    
        mbstowcs(buf, dirname, PATH_MAX + 1);

        if (buf[wcslen(buf) - 1] == L'/')
            wcscat(buf, L"*.*");
        else
            wcscat(buf, L"/*.*");

        ret = f_wfindfirst(buf, find);
        if (ret)
        {
            free(find);
            free(dirp);
            errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
            return NULL;
        }
        
        dirp->dd_fd     = ITP_DEVICE_FAT;
        dirp->dd_size   = sizeof (FN_WFIND);
        dirp->dd_buf    = (char*) find;
        dirp->dd_loc    = -1;
        
        strcpy(&dirp->dd_buf[sizeof(FN_WFIND) + sizeof(struct dirent)], dirname);
    }

#else
    {
        char buf[PATH_MAX + 1];
        FN_FIND* find = malloc(sizeof (FN_FIND) + sizeof (struct dirent) + strlen(dirname) + 1);
        if (!find)
        {
            free(dirp);
            errno = ENOMEM;
            return NULL;
        }

        strcpy(buf, dirname);
        if (buf[strlen(buf) - 1] == '/')
            strcat(buf, "*.*");
        else
            strcat(buf, "/*.*");

        ret = f_findfirst(buf, find);
        if (ret)
        {
            free(find);
            free(dirp);
            errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
            return NULL;
        }
        
        dirp->dd_fd     = ITP_DEVICE_FAT;
        dirp->dd_size   = sizeof (FN_FIND);
        dirp->dd_buf    = (char*) find;
        dirp->dd_loc    = -1;
        
        strcpy(&dirp->dd_buf[sizeof(FN_FIND) + sizeof(struct dirent)], dirname);
    }
#endif // ITP_FAT_UTF8

    return dirp;
}

static struct dirent* FatReaddir(DIR *dirp)
{
    struct dirent* d;
    
    if (dirp->dd_size == sizeof (FN_WFIND))
    {
        FN_WFIND* find = (FN_WFIND*)dirp->dd_buf;
        
        d = (struct dirent*)(dirp->dd_buf + sizeof (FN_WFIND));

        if (dirp->dd_loc >= 0)
        {
            int ret = f_wfindnext(find);
            if (ret)
                return NULL;
        }
        d->d_ino    = ++dirp->dd_loc;
        d->d_type   = (find->attr & F_ATTR_DIR) ? DT_DIR : DT_REG;
        d->d_namlen = wcstombs(d->d_name, find->filename, NAME_MAX + 1);
    }
    else
    {
        FN_FIND* find = (FN_FIND*)dirp->dd_buf;
        
        d = (struct dirent*)(dirp->dd_buf + sizeof (FN_FIND));

        if (dirp->dd_loc >= 0)
        {
            int ret = f_findnext(find);
            if (ret)
                return NULL;
        }
        d->d_ino    = ++dirp->dd_loc;
        d->d_type   = (find->attr & F_ATTR_DIR) ? DT_DIR : DT_REG;
        d->d_namlen = strlen(find->filename);
        strncpy(d->d_name, find->filename, d->d_namlen);
    }
    
    return d;
}

static void FatRewinddir(DIR *dirp)
{
    intptr_t ret;

    if (dirp->dd_size == sizeof(FN_WFIND))
    {
        FN_WFIND* find = (FN_WFIND*) dirp->dd_buf;
        wchar_t buf[PATH_MAX + 1];

        mbstowcs(buf, &dirp->dd_buf[sizeof(FN_WFIND) + sizeof(struct dirent)], PATH_MAX + 1);
        
        if (buf[wcslen(buf) - 1] == L'/')
            wcscat(buf, L"*.*");
        else
            wcscat(buf, L"/*.*");

        ret = f_wfindfirst(buf, find);
        if (ret)
        {
            errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
            return;
        }
    }
    else
    {
        FN_FIND* find = (FN_FIND*) dirp->dd_buf;
        char buf[PATH_MAX + 1];

        strcpy(buf, &dirp->dd_buf[sizeof(FN_FIND) + sizeof(struct dirent)]);
        if (buf[strlen(buf) - 1] == '/')
            strcat(buf, "*.*");
        else
            strcat(buf, "/*.*");

        ret = f_findfirst(buf, find);
        if (ret == -1)
        {
            errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
            return;
        }
    }
    dirp->dd_loc  = -1;
}

static long FatFtell(int file)
{
    return f_tell(openFiles[file]);
}

static int FatFflush(int file)
{
    int ret = f_flush(openFiles[file]);
    if (ret)
    {
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
        return EOF;
    }
    return 0;
}

static int FatFeof(int file)
{
    int ret = f_eof(openFiles[file]);
    if (ret)
    {
        return EOF;
    }
    return 0;
}

static int FatMount(ITPDisk disk, bool force)
{
    int ret, i, j;
    ITPDriveStatus* driveStatus = NULL;
    Driver* driver;
    F_PARTITION partitions[ITP_MAX_PARTITION];
    ITPDriveStatus* driveStatusTable;

    // init drives
    driver = &driverTable[disk];

    if (!driver->driver)
    {
        ret = f_createdriver(&driver->driver, driver->initfunc, driver->param);
        if (ret)
        {
            LOG_ERR "f_createdriver() fail: %d\n", ret LOG_END
            return ret;
        }
    }

    // get fat's partitions
    ret = f_getpartition(driver->driver, ITP_MAX_PARTITION, partitions);
    if (ret)
    {
        LOG_ERR "f_getpartition() fail: %d\n", ret LOG_END
        return ret;
    }

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    j = 0;
    for (i = 0; i < ITP_MAX_PARTITION; i++)
    {
        F_PARTITION* partition = &partitions[i];

        if (partition->secnum > 0)
        {
            // find an empty drive
            for (; j < ITP_MAX_DRIVE; j++)
            {
                driveStatus = &driveStatusTable[j];
                
                if (!driveStatus->avail)
                    break;
            }
            
            ret = f_initvolumepartition(j, driver->driver, i);
            if (ret && ret != F_ERR_NOTFORMATTED)
            {
                f_delvolume(j);
                LOG_ERR "f_initvolumepartition(%d, 0x%X, %d) fail: %d\n", j, driver->driver, i, ret LOG_END
                return ret;
            }

            if( (disk==5) && (ret == F_ERR_NOTFORMATTED) )
            {
                ret = (int)FatFormat((int)j);
            }

            if (ret == F_ERR_NOTFORMATTED && !force)
            {
                f_delvolume(j);
                continue;
            }

            driveStatus->disk       = disk;
            driveStatus->device     = ITP_DEVICE_FAT;
            driveStatus->avail      = true;
            driveStatus->name[0]    = 'A' + j;
            driveStatus->name[1]    = ':';
            driveStatus->name[2]    = '/';

            write(ITP_DEVICE_DRIVE, driveStatus, sizeof (ITPDriveStatus));
        }
    }
    return 0;
}

static int FatUnmount(ITPDisk disk)
{
    int i, ret1 = -1, ret2;
    ITPDriveStatus* driveStatusTable = NULL;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    // deinit all drives of this disk
    for (i = 0; i < ITP_MAX_DRIVE; i++)
    {
        ITPDriveStatus* driveStatus = &driveStatusTable[i];
        //printf("i=%d avail=%d disk=%d disk=%d\n", i, driveStatus->avail, driveStatus->disk, disk);
        if (driveStatus->avail && driveStatus->disk == disk && driveStatus->device == ITP_DEVICE_FAT)
        {
            ret2 = f_delvolume(i);
            if (ret2 == 0)
            {
                ret1 = 0;
            }
            else
            { 
                LOG_ERR "f_delvolume(%d) fail: %d\n", i, ret2 LOG_END
                if (ret1 <= 0)
                    ret1 = ret2;
            }
            driveStatus->avail  = false;
            write(ITP_DEVICE_DRIVE, driveStatus, sizeof (ITPDriveStatus));
        }
    }

    ret2 = f_releasedriver(driverTable[disk].driver);
    driverTable[disk].driver = NULL;

    if (ret2 > 0)
    {
        LOG_ERR "f_releasedriver() fail: %d\n", ret1 LOG_END
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret1;

        if (ret1 <= 0)
            ret1 = ret2;
    }
    return ret1;
}

static int FatGetPartition(ITPPartition* par)
{
    F_PARTITION partitions[ITP_MAX_PARTITION];
    Driver* driver;
    F_PHY phy;
    int ret, i;

    driver = &driverTable[par->disk];
    
    if (!driver->driver)
    {
        ret = f_createdriver(&driver->driver, driver->initfunc, driver->param);
        if (ret)
        {
            LOG_ERR "f_createdriver() fail: %d\n", ret LOG_END
            return ret;
        }
    }

    ret = f_getpartition(driver->driver, ITP_MAX_PARTITION, partitions);
    if (ret)
        return ret;

    ret = driver->driver->getphy(driver->driver, &phy);
    if (ret)
        return ret;

	if (!phy.bytes_per_sector)
		phy.bytes_per_sector = F_DEF_SECTOR_SIZE;

    par->count = 0;
    for (i = 0; i < ITP_MAX_PARTITION; i++)
    {
        if (partitions[i].secnum > 0)
        {
            par->count++;
            par->size[i] = (uint64_t)partitions[i].secnum * phy.bytes_per_sector;
        }
        else
            break;
    }

    return 0;
}

static int FatCreatePartition(ITPPartition* par)
{
    F_PARTITION partitions[ITP_MAX_PARTITION];
    Driver* driver;
    F_PHY phy;
    int ret, i;
    unsigned long secnum = 0;

    driver = &driverTable[par->disk];

    if (!driver->driver)
    {
        switch (par->disk)
        {
    #ifdef CFG_SD0_STATIC
        case ITP_DISK_SD0:
            sdDrvParams[0].reserved = (unsigned long)par->start[0];
            break;
    #endif // CFG_SD0_STATIC

    #ifdef CFG_SD0_STATIC
        case ITP_DISK_SD1:
            sdDrvParams[1].reserved = (unsigned long)par->start[0];
            break;
    #endif // CFG_SD0_STATIC

    #ifdef CFG_NOR_ENABLE
        case ITP_DISK_NOR:
            norDrvParam.reserved = (unsigned long)par->start[0];
            break;
    #endif // CFG_NOR_ENABLE

        default:
            break;
        }

        ret = f_createdriver(&driver->driver, driver->initfunc, driver->param);
        if (ret)
        {
            LOG_ERR "f_createdriver() fail: %d\n", ret LOG_END
            return ret;
        }
    }

    ret = driver->driver->getphy(driver->driver, &phy);
    if (ret)
        return ret;

	if (!phy.bytes_per_sector)
		phy.bytes_per_sector = F_DEF_SECTOR_SIZE;

    for (i = 0; i < par->count; i++)
    {
        uint64_t size;
        
        if (par->size[i] > 0)
        {
            partitions[i].secnum = (unsigned long)(par->size[i] / phy.bytes_per_sector);
            secnum += partitions[i].secnum;
        }
        else
        {
            partitions[i].secnum = phy.number_of_sectors - secnum - F_SPACE_AFTER_MBR;

            if (i != par->count - 1)
            {
                LOG_ERR "partition %d/%d has zero size.\n", i, par->count LOG_END
                return __LINE__;
            }
        }

        size = partitions[i].secnum * phy.bytes_per_sector;
        LOG_DBG "partition %d size: %llu\n", i, size LOG_END
        if (size < 32 * 1024 * 1024)
        {
            partitions[i].system_indicator = F_SYSIND_DOSFAT16UPTO32MB;
        }
        else if (size < 128 * 1024 * 1024)
        {
            partitions[i].system_indicator = F_SYSIND_DOSFAT16OVER32MB;
        }
        else
        {
            partitions[i].system_indicator = F_SYSIND_DOSFAT32;
        }
    }

    ret = f_createpartition(driver->driver, par->count, partitions);
    if (ret)
    {
        LOG_ERR "f_createpartition(,%d,) fail: %d\n", par->count, ret LOG_END
        return ret;
    }

    return 0;
}

static int FatFormat(int volume)
{
    int ret = f_format(volume, F_FAT12_MEDIA);
    if (ret)
    {
        int ret = f_format(volume, F_FAT16_MEDIA);
        if (ret)
        {
            ret = f_format(volume, F_FAT32_MEDIA);
            if (ret)
            {
                LOG_ERR "f_format(%d) fail: %d\n", volume, ret LOG_END
                return ret;
            }
        }
    }
    return 0;
}

static int FatIoctl(int file, unsigned long request, void* ptr, void* info)
{
    int ret;
    
    switch (request)
    {
    case ITP_IOCTL_MOUNT:
        ret = FatMount((ITPDisk)ptr, false);
        if (ret)
        {
            errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
            return -1;
        }
        break;

    case ITP_IOCTL_FORCE_MOUNT:
        ret = FatMount((ITPDisk)ptr, true);
        if (ret)
        {
            errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
            return -1;
        }
        break;

    case ITP_IOCTL_UNMOUNT:
        ret = FatUnmount((ITPDisk)ptr);
        if (ret)
        {
            errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
            return -1;
        }
        break;

    case ITP_IOCTL_ENABLE:
        if (!fatInited)
            break;
            
        ret = f_enterFS();
        if (ret)
        {
            LOG_ERR "f_enterFS() fail: %d\n", ret LOG_END
            errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
            return -1;
        }
        break;

    case ITP_IOCTL_DISABLE:
        if (!fatInited)
            break;
            
        f_releaseFS(fn_gettaskID());
        break;

    case ITP_IOCTL_GET_PARTITION:
        ret = FatGetPartition((ITPPartition*)ptr);
        if (ret)
        {
            errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
            return -1;
        }
        break;

    case ITP_IOCTL_CREATE_PARTITION:
        ret = FatCreatePartition((ITPPartition*)ptr);
        if (ret)
        {
            errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
            return -1;
        }
        break;

    case ITP_IOCTL_FORMAT:
        ret = FatFormat((int)ptr);
        if (ret)
        {
            errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
            return -1;
        }
        break;

    case ITP_IOCTL_INIT:
        ret = f_init();
        if (ret)
        {
            errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | ret;
            return -1;
        }
        fatInited = true;
        break;

    case ITP_IOCTL_EXIT:
        f_exit();
        fatInited = false;
        break;

    default:
        errno = (ITP_DEVICE_FAT << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        return -1;
    }
    return 0;
}

const ITPFSDevice itpFSDeviceFat =
{
    {
        ":fat",
        FatOpen,
        FatClose,
        FatRead,
        FatWrite,
        FatLseek,
        FatIoctl,
        NULL
    },
    FatRemove,
    FatRename,
    FatChdir,
    FatChmod,
    FatMkdir,
    FatStat,
    FatStatvfs,
    FatFstat,
    FatGetcwd,
    FatRmdir,
    FatClosedir,
    FatOpendir,
    FatReaddir,
    FatRewinddir,
    FatFtell,
    FatFflush,
    FatFeof
};
