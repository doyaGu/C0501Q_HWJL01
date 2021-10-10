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
#include <sys/reent.h>
#include <sys/syslimits.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "config.h"
#include "ntfs/ntfs.h"
#include "ntfsdir.h"
#include "ntfsfile.h"
#include "itp_cfg.h"

extern void ntfsInit(void);

const devoptab_t *devoptab_list[STD_MAX];

const devoptab_t* GetDeviceOpTab(const char *name)
{
    return devoptab_list[1];
}

static ntfs_file_state openFiles[OPEN_MAX];

static int NtfsOpen(const char* name, int flags, int mode, void* info)
{
    ntfs_file_state* file;
    int i;

    // find empty slot
    ithEnterCritical();
    for (i = 0; i < OPEN_MAX; i++)
    {
        if (openFiles[i].vd == NULL)
        {
            file = &openFiles[i];
            break;
        }
    }
    ithExitCritical();
    if (i == OPEN_MAX)
    {
        LOG_ERR "Out of file.\n" LOG_END
        errno = (ITP_DEVICE_NTFS << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        return -1;
    }

    if (ntfs_open_r(__getreent(), file, name, flags, mode) == -1)
    {
        LOG_ERR "ntfs open %s fail: %d\n", name, errno LOG_END
        return -1;
    }

    return i;
}

static int NtfsClose(int file, void* info)
{
    int ret = ntfs_close_r(__getreent(), (int)&openFiles[file]);
    ithEnterCritical();
    openFiles[file].vd = NULL;
    ithExitCritical();
    if (ret)
    {
        LOG_ERR "ntfs close %d fail: %d\n", file, errno LOG_END
        return -1;
    }
    return 0;
}

static int NtfsRead(int file, char *ptr, int len, void* info)
{
    int ret;

#ifdef CFG_DBG_STATS_NTFS
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
#endif
    ret = ntfs_read_r(__getreent(), (int)&openFiles[file], ptr, len);

#ifdef CFG_DBG_STATS_NTFS
    gettimeofday(&t2, NULL);

#ifdef __OPENRTOS__
    portENTER_CRITICAL();
#endif
    itpStatsNtfs.readTime += itpTimevalDiff(&t1, &t2);
    itpStatsNtfs.readSize += len;
#ifdef __OPENRTOS__
    portEXIT_CRITICAL();
#endif

#endif // CFG_DBG_STATS_NTFS

    return ret;
}

static int NtfsWrite(int file, char *ptr, int len, void* info)
{
    int ret;

#ifdef CFG_DBG_STATS_NTFS
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
#endif

    ret = ntfs_write_r(__getreent(), (int)&openFiles[file], ptr, len);

#ifdef CFG_DBG_STATS_NTFS
    gettimeofday(&t2, NULL);

#ifdef __OPENRTOS__
    portENTER_CRITICAL();
#endif
    itpStatsNtfs.writeTime += itpTimevalDiff(&t1, &t2);
    itpStatsNtfs.writeSize += len;
#ifdef __OPENRTOS__
    portEXIT_CRITICAL();
#endif

#endif // CFG_DBG_STATS_NTFS

    return ret;
}

static int NtfsLseek(int file, int ptr, int dir, void* info)
{
    return ntfs_seek_r(__getreent(), (int)&openFiles[file], ptr, dir);
}

static int NtfsRemove(const char *path)
{
    return ntfs_unlink_r(__getreent(), path);
}

static int NtfsRename(const char *oldname, const char *newname)
{
    return ntfs_rename_r(__getreent(), oldname, newname);
}

static int NtfsChdir(const char *path)
{
    return ntfs_chdir_r(__getreent(), path);
}

static int NtfsChmod(const char *path, mode_t mode)
{
    // DO NOTHING
    return 0;
}

static int NtfsMkdir(const char *path, mode_t mode)
{
    return ntfs_mkdir_r(__getreent(), path, mode);
}

static int NtfsStat(const char *path, struct stat *sbuf)
{
    return ntfs_stat_r(__getreent(), path, sbuf);
}

static int NtfsStatvfs(const char *path, struct statvfs *sbuf)
{
    return ntfs_statvfs_r(__getreent(), path, sbuf);
}

static int NtfsFstat(int file, struct stat *st)
{
    return ntfs_fstat_r(__getreent(), (int)&openFiles[file], st);
}

static char* NtfsGetcwd(char *buf, size_t size)
{
    // TODO: IMPLEMENT
    return NULL;
}

static int NtfsRmdir(const char *path)
{
    return ntfs_unlink_r(__getreent(), path);
}

static int NtfsClosedir(DIR *dirp)
{
    ntfs_dirclose_r(__getreent(), (DIR_ITER*) dirp->dd_buf);
    free(dirp->dd_buf);
    free(dirp);
    return 0;
}

static DIR* NtfsOpendir(const char *dirname)
{
    DIR_ITER* find;
    DIR* dirp = malloc(sizeof (DIR));
    if (!dirp)
    {
        errno = ENOMEM;
        return NULL;
    }

    find = malloc(sizeof (DIR_ITER) + sizeof (ntfs_dir_state) + sizeof (struct dirent));
    if (!find)
    {
        free(dirp);
        errno = ENOMEM;
        return NULL;
    }
    find->dirStruct = find + 1;
    
    find = ntfs_diropen_r(__getreent(), find, dirname);
    if (!find)
    {
        free(dirp);
        return NULL;
    }

    dirp->dd_fd     = ITP_DEVICE_NTFS;
    dirp->dd_size   = sizeof (DIR_ITER);
    dirp->dd_buf    = (char*) find;
    dirp->dd_loc    = -1;

    return dirp;
}


static struct dirent* NtfsReaddir(DIR *dirp)
{
    struct dirent* d;
    DIR_ITER* find = (DIR_ITER*)dirp->dd_buf;
    struct stat filestat;
    int ret;
    
    d = (struct dirent*)(dirp->dd_buf + sizeof (DIR_ITER) + sizeof (ntfs_dir_state));

    ret = ntfs_dirnext_r (__getreent(), find, d->d_name, &filestat);
    if (ret)
        return NULL;

    d->d_ino    = ++dirp->dd_loc;
    d->d_type   = S_ISDIR(filestat.st_mode) ? DT_DIR : DT_REG;
    d->d_namlen = strlen(d->d_name);
    
    return d;
}

static void NtfsRewinddir(DIR *dirp)
{
    DIR_ITER* find = (DIR_ITER*) dirp->dd_buf;
    ntfs_dirreset_r(__getreent(), find);
    dirp->dd_loc = -1;
}

static long NtfsFtell(int file)
{
    return openFiles[file].pos;
}

static int	NtfsFflush(int file)
{
    return ntfs_fsync_r(__getreent(), (int)&openFiles[file]);
}

static int	NtfsFeof(int file)
{
    return openFiles[file].pos == openFiles[file].len ? EOF : 0;
}

static const char* ntfsDiskTable[ITH_DISK_MAX] =
{
    "sd0",
    "sd1", 
    "cf", 
    "ms", 
    "xd", 
    "nand",
    "nor",
    "msc00",
    "msc01",
    "msc02",
    "msc03",
    "msc04",
    "msc05",
    "msc06",
    "msc07",
    "msc10",
    "msc11",
    "msc12",
    "msc13",    
    "msc14",
    "msc15",
    "msc16",
    "msc17",
    "ram",
};

static int NtfsMount(ITPDisk disk)
{
    int i, j;
    ITPDriveStatus* driveStatus = NULL;
    sec_t* partitions = NULL;
    ITPDriveStatus* driveStatusTable;
    const INTERFACE_ID *discs = ntfsGetDiscInterfaces();
    const INTERFACE_ID *disc = NULL;
    int partition_count = 0;

    switch (disk)
    {
    case ITP_DISK_SD0:
    case ITP_DISK_SD1:
    case ITP_DISK_MSC00:
    case ITP_DISK_MSC01:
    case ITP_DISK_MSC02:
    case ITP_DISK_MSC03:
    case ITP_DISK_MSC04:
    case ITP_DISK_MSC05:
    case ITP_DISK_MSC06:
    case ITP_DISK_MSC07:
    case ITP_DISK_MSC10:
    case ITP_DISK_MSC11:
    case ITP_DISK_MSC12:
    case ITP_DISK_MSC13:
    case ITP_DISK_MSC14:
    case ITP_DISK_MSC15:
    case ITP_DISK_MSC16:
    case ITP_DISK_MSC17:
        for (i = 0; discs[i].name != NULL && discs[i].interface != NULL; i++)
        {
            if (strcmp(discs[i].name, ntfsDiskTable[disk]) == 0)
            {
                disc = &discs[i];
                break;
            }
        }
        break;

    default:
        LOG_INFO "unsupport disk: %d\n", disk LOG_END
        return -1;
    }

    if (disc == NULL)
    {
        LOG_ERR "cannot find disc interface: %d\n", disk LOG_END
        return -1;
    }

    partition_count = ntfsFindPartitions(disc->interface, &partitions);
    LOG_DBG "%d partitions found.\n", partition_count LOG_END

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    j = 0;
    for (i = 0; i < partition_count; i++)
    {
        // find an empty drive
        for (; j < ITP_MAX_DRIVE; j++)
        {
            driveStatus = &driveStatusTable[j];
            
            if (!driveStatus->avail)
                break;
        }

        driveStatus->name[0]    = 'A' + j;

        if (ntfsMount(driveStatus->name, disc->interface, partitions[i], CACHE_DEFAULT_PAGE_SIZE, CACHE_DEFAULT_PAGE_COUNT, NTFS_DEFAULT | NTFS_RECOVER))
        {
            driveStatus->disk       = disk;
            driveStatus->device     = ITP_DEVICE_NTFS;
            driveStatus->avail      = true;
            driveStatus->name[1]    = ':';
            driveStatus->name[2]    = '/';

            write(ITP_DEVICE_DRIVE, driveStatus, sizeof (ITPDriveStatus));
        }
    }
    ntfs_free(partitions);

    return 0;
}

static int NtfsUnmount(ITPDisk disk)
{
    int i;
    ITPDriveStatus* driveStatusTable = NULL;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    // deinit all drives of this disk
    for (i = 0; i < ITP_MAX_DRIVE; i++)
    {
        ITPDriveStatus* driveStatus = &driveStatusTable[i];
        
        if (driveStatus->avail && driveStatus->disk == disk && driveStatus->device == ITP_DEVICE_NTFS)
        {
            ntfsUnmount(driveStatus->name, true);
            driveStatus->avail  = false;
            write(ITP_DEVICE_DRIVE, driveStatus, sizeof (ITPDriveStatus));
        }
    }
    return 0;
}

extern int mkntfs_main(int argc, char *argv[]);

const char* ntfs_disc_names[] = {
    "sd0",
    "sd1",
    "cf",
    "ms",
    "xd",
    "nand",
    "nor",
    "msc00",
    "msc01",
    "msc02",
    "msc03",
    "msc04",
    "msc05",
    "msc06",
    "msc07",
    "msc10",
    "msc11",
    "msc12",
    "msc13",
    "msc14",
    "msc15",
    "msc16",
    "msc17",
};

static int NtfsCreatePartition(ITPPartition* par)
{
    int ret, i;
    char sizebuf[16], startbuf[16];
    char **devptr, **sizeptr;
    static char* args[] = 
    { 
        "mkntfs",
        "-f",
        "-s",
        "512",
        "-p",
        NULL,
    #ifndef NDEBUG
        "--debug",
    #endif
        NULL,
        NULL,
    };
    args[5] = startbuf;

#ifdef NDEBUG
    devptr  = &args[6];
    sizeptr = &args[7];
#else
    devptr  = &args[7];
    sizeptr = &args[8];
#endif

    *devptr     = (char*) ntfs_disc_names[par->disk];
    *sizeptr    = sizebuf;

    for (i = 0; i < par->count; i++)
    {
        sprintf(sizebuf, "%lu", par->size[i]);
        sprintf(startbuf, "%lu", par->start[i]);

        ret = mkntfs_main(sizeof(args) / sizeof(args[0]), args);
        if (ret)
        {
            LOG_ERR "create partition %d fail: %d, start=0x%X, size=%lu\n", i, ret, par->start[i], par->size[i] LOG_END
            return ret;
        }
    }

    return 0;
}

static int NtfsIoctl(int file, unsigned long request, void* ptr, void* info)
{
    int ret;
    
    switch (request)
    {
    case ITP_IOCTL_MOUNT:
        ret = NtfsMount((ITPDisk)ptr);
        if (ret)
        {
            errno = (ITP_DEVICE_NTFS << ITP_DEVICE_ERRNO_BIT) | ret;
            return -1;
        }
        break;

    case ITP_IOCTL_UNMOUNT:
        ret = NtfsUnmount((ITPDisk)ptr);
        if (ret)
        {
            errno = (ITP_DEVICE_NTFS << ITP_DEVICE_ERRNO_BIT) | ret;
            return -1;
        }
        break;

    case ITP_IOCTL_ENABLE:
        break;

    case ITP_IOCTL_DISABLE:
        break;

    case ITP_IOCTL_CREATE_PARTITION:
        ret = NtfsCreatePartition((ITPPartition*)ptr);
        if (ret)
        {
            errno = (ITP_DEVICE_NTFS << ITP_DEVICE_ERRNO_BIT) | ret;
            return -1;
        }
        break;

    case ITP_IOCTL_INIT:
        ntfsInit();
        break;

    default:
        errno = (ITP_DEVICE_NTFS << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        return -1;
    }
    return 0;
}

const ITPFSDevice itpFSDeviceNtfs =
{
    {
        ":ntfs",
        NtfsOpen,
        NtfsClose,
        NtfsRead,
        NtfsWrite,
        NtfsLseek,
        NtfsIoctl,
        NULL
    },
    NtfsRemove,
    NtfsRename,
    NtfsChdir,
    NtfsChmod,
    NtfsMkdir,
    NtfsStat,
    NtfsStatvfs,
    NtfsFstat,
    NtfsGetcwd,
    NtfsRmdir,
    NtfsClosedir,
    NtfsOpendir,
    NtfsReaddir,
    NtfsRewinddir,
    NtfsFtell,
    NtfsFflush,
    NtfsFeof
};
