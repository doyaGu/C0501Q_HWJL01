/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Posix WIN32 functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "itp_cfg.h"
#include <sys/reent.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <locale.h>
#include <malloc.h>
#include <math.h>
#include <mqueue.h>
#include <netdb.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>
#include <mmsystem.h>
#include <io.h>

static char *GetErrorString(int errnum)
{
    static TCHAR buf[256];

    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errnum,
        0,
        buf,
        256,
        NULL);

    return buf;
}

int open(const char *name, int flags, ...)
{
    va_list ap;
    int     mode, i, subfile;

    va_start(ap, flags);
    mode = va_arg(ap, int);
    va_end(ap);

    if (name[0] == ':')
    {
        char *subname = strchr(&name[1], ':');
        int  len      = subname ? subname - name : strlen(name);

        // try to open custom device
        for (i = ITP_DEVICE_CUSTOM >> ITP_DEVICE_BIT; i < ITP_DEVICE_MAX; i++)
        {
            const ITPDevice *dev = itpDeviceTable[i];

            if (dev && strncmp(name, dev->name, len) == 0)
            {
                subfile = dev->open(subname + 1, flags, mode, dev->info);
                if (subfile == -1)
                    return -1;

                return (i << ITP_DEVICE_BIT) | subfile;
            }
        }
    }
    else if (name[1] == ':')
    {
        const ITPDevice *dev = itpDeviceTable[ITP_DEVICE_FILE >> ITP_DEVICE_BIT];
        subfile = dev->open(name, flags, mode, dev->info);
        if (subfile == -1)
            return -1;

        return ITP_DEVICE_FILE | subfile;
    }
    errno = ENOENT;
    return -1;
}

int close(int file)
{
    const ITPDevice *dev;

    if ((file & ITP_DEVICE_MASK) == 0)
        dev = itpDeviceTable[ITP_DEVICE_SOCKET >> ITP_DEVICE_BIT];
    else
        dev = itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (dev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return dev->close(file & ITP_FILE_MASK, dev->info);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

int read(int file, char *ptr, size_t len)
{
    const ITPDevice *dev;

    if ((file & ITP_DEVICE_MASK) == 0)
        dev = itpDeviceTable[ITP_DEVICE_SOCKET >> ITP_DEVICE_BIT];
    else
        dev = itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (dev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return dev->read(file & ITP_FILE_MASK, ptr, len, dev->info);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

int write(int file, char *ptr, size_t len)
{
    const ITPDevice *dev;

    if ((file & ITP_DEVICE_MASK) == 0)
        dev = itpDeviceTable[ITP_DEVICE_SOCKET >> ITP_DEVICE_BIT];
    else
        dev = itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (dev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return dev->write(file & ITP_FILE_MASK, ptr, len, dev->info);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

int lseek(int file, int ptr, int dir)
{
    const ITPDevice *dev = itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];
    if (dev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return dev->lseek(file & ITP_FILE_MASK, ptr, dir, dev->info);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

int ioctl(int file, unsigned long request, void *ptr)
{
    const ITPDevice *dev;

    if ((file & ITP_DEVICE_MASK) == 0)
        dev = itpDeviceTable[ITP_DEVICE_SOCKET >> ITP_DEVICE_BIT];
    else
        dev = itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (dev)
    {
        /*  Pass call on to device driver.  Return result.		*/
        return dev->ioctl(file & ITP_FILE_MASK, request, ptr, dev->info);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

static int volume;

#ifdef CFG_WIN32_FS_HW

void *fopen(const char *name, const char *mode)
{
    ITPDriveStatus *driveStatusTable;
    ITPDriveStatus *driveStatus;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    if (name[1] == ':')
        driveStatus = &driveStatusTable[toupper(name[0]) - 'A'];
    else
        driveStatus = &driveStatusTable[volume];

    if (driveStatus->avail)
    {
        const ITPDevice *dev = (ITPDevice *) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
        if (dev)
        {
            int subfile, flags, m, o;

            switch (mode[0])
            {
            case 'r':                   /* open for reading */
                m = O_RDONLY;
                o = 0;
                break;

            case 'w':                   /* open for writing */
                m = O_WRONLY;
                o = O_CREAT | O_TRUNC;
                break;

            case 'a':                   /* open for appending */
                m = O_WRONLY;
                o = O_CREAT | O_APPEND;
                break;

            default:                    /* illegal mode */
                return NULL;
            }
            if (mode[1] && (mode[1] == '+' || mode[2] == '+'))
                m = O_RDWR;

            if (mode[1] && (mode[1] == 'b' || mode[2] == 'b'))
                m |= O_BINARY;

            flags   = m | o;

            subfile = dev->open(name, flags, 0666, dev->info);
            if (subfile != -1)
            {
                return (void *)(driveStatus->device | subfile);
            }
        }
    }
    errno = ENOENT;
    return NULL;
}

int fclose(void *fp)
{
    int             file = (int)fp;
    const ITPDevice *dev = itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (dev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return dev->close(file & ITP_FILE_MASK, dev->info);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

size_t fread(void *ptr, size_t size, size_t count, void *fp)
{
    int             file = (int)fp;
    const ITPDevice *dev = itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (dev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return dev->read(file & ITP_FILE_MASK, ptr, size * count, dev->info);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

size_t fwrite(const void *ptr, size_t size, size_t count, void *fp)
{
    int             file = (int)fp;
    const ITPDevice *dev = itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (dev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return dev->write(file & ITP_FILE_MASK, (char *)ptr, size * count, dev->info);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

int fseek(void *fp, long offset, int whence)
{
    int             file = (int)fp;
    const ITPDevice *dev = itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (dev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return dev->lseek(file & ITP_FILE_MASK, offset, whence, dev->info);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

long ftell(void *fp)
{
    int               file = (int)fp;
    const ITPFSDevice *dev = (ITPFSDevice *)itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (dev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return dev->ftell(file & ITP_FILE_MASK);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

int fflush(void *fp)
{
    int               file = (int)fp;
    const ITPFSDevice *dev = (ITPFSDevice *)itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (dev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return dev->fflush(file & ITP_FILE_MASK);
    }
    else
    {
        errno = ENOENT;
        return EOF;
    }
}

int closedir(DIR *dirp)
{
    const ITPFSDevice *dev = (ITPFSDevice *)itpDeviceTable[(dirp->dd_fd & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];
    if (dev)
        return dev->closedir(dirp);
    else
    {
        errno = ENOENT;
        return -1;
    }
}

DIR *opendir(const char *dirname)
{
    ITPDriveStatus *driveStatusTable;
    ITPDriveStatus *driveStatus;
    DIR            *dir = NULL;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);
    driveStatus = &driveStatusTable[volume];

    if (driveStatus->avail)
    {
        const ITPFSDevice *dev = (ITPFSDevice *) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
        if (dev)
            dir = dev->opendir(dirname);
        else
        {
            errno = ENOENT;
            return NULL;
        }
    }
    if (dir == NULL)
        errno = EINVAL;

    return dir;
}

struct dirent *readdir(DIR *dirp)
{
    const ITPFSDevice *dev = (ITPFSDevice *)itpDeviceTable[(dirp->dd_fd & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];
    if (dev)
        return dev->readdir(dirp);
    else
    {
        errno = ENOENT;
        return NULL;
    }
}

void rewinddir(DIR *dirp)
{
    const ITPFSDevice *dev = (ITPFSDevice *)itpDeviceTable[(dirp->dd_fd & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];
    if (dev)
        dev->rewinddir(dirp);
}

int remove(const char *path)
{
    ITPDriveStatus *driveStatusTable;
    ITPDriveStatus *driveStatus;
    int            ret = -1;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);
    driveStatus = &driveStatusTable[volume];

    if (driveStatus->avail)
    {
        const ITPFSDevice *fsdev = (ITPFSDevice *) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
        if (fsdev)
            ret = fsdev->remove(path);
        else
        {
            errno = ENOENT;
            return -1;
        }
    }
    if (ret)
        errno = EINVAL;

    return ret;
}

int rename(const char *oldname, const char *newname)
{
    ITPDriveStatus *driveStatusTable;
    ITPDriveStatus *driveStatus;
    int            ret = -1;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);
    driveStatus = &driveStatusTable[volume];

    if (driveStatus->avail)
    {
        const ITPFSDevice *dev = (ITPFSDevice *) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
        if (dev)
            ret = dev->rename(oldname, newname);
        else
        {
            errno = ENOENT;
            return -1;
        }
    }
    if (ret)
        errno = EINVAL;

    return ret;
}

int chdir(const char *path)
{
    ITPDriveStatus *driveStatusTable;
    ITPDriveStatus *driveStatus;
    int            ret = -1;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);
    driveStatus = &driveStatusTable[volume];

    if (driveStatus->avail)
    {
        const ITPFSDevice *dev = (ITPFSDevice *) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
        if (dev)
            ret = dev->chdir(path);
        else
        {
            errno = ENOENT;
            return -1;
        }
    }

    if (ret)
        errno = EINVAL;
    else
    {
        if (strlen(path) > 2 && path[1] == ':')
            volume = toupper(path[0]) - 'A';
    }
    return ret;
}

int chmod(const char *path, mode_t mode)
{
    ITPDriveStatus *driveStatusTable;
    ITPDriveStatus *driveStatus;
    int            ret = -1;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);
    driveStatus = &driveStatusTable[volume];

    if (driveStatus->avail)
    {
        const ITPFSDevice *dev = (ITPFSDevice *) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
        if (dev)
            ret = dev->chmod(path, mode);
        else
        {
            errno = ENOENT;
            return -1;
        }
    }

    if (ret)
        errno = EINVAL;

    return ret;
}

int mkdir(const char *path, mode_t mode)
{
    ITPDriveStatus *driveStatusTable;
    ITPDriveStatus *driveStatus;
    int            ret = -1;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);
    driveStatus = &driveStatusTable[volume];

    if (driveStatus->avail)
    {
        const ITPFSDevice *dev = (ITPFSDevice *) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
        if (dev)
            ret = dev->mkdir(path, mode);
        else
        {
            errno = ENOENT;
            return -1;
        }
    }

    if (ret)
        errno = EINVAL;

    return ret;
}

int stat(const char *path, struct stat *buf)
{
    ITPDriveStatus *driveStatusTable;
    ITPDriveStatus *driveStatus;
    int            ret = -1;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);
    driveStatus = &driveStatusTable[volume];

    if (driveStatus->avail)
    {
        const ITPFSDevice *dev = (ITPFSDevice *) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
        if (dev)
            ret = dev->stat(path, buf);
        else
        {
            errno = ENOENT;
            return -1;
        }
    }

    if (ret)
        errno = EINVAL;

    return ret;
}

int statvfs(const char *path, struct statvfs *buf)
{
    int ret = -1;

    if (strlen(path) > 2 && path[1] == ':')
    {
        int            volume = toupper(path[0]) - 'A';
        ITPDriveStatus *driveStatusTable;
        ITPDriveStatus *driveStatus;

        ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);
        driveStatus = &driveStatusTable[volume];

        if (driveStatus->avail)
        {
            const ITPFSDevice *dev = (ITPFSDevice *) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
            if (dev)
                ret = dev->statvfs(path, buf);
            else
            {
                errno = ENOENT;
                return -1;
            }
        }
        if (ret)
            errno = EINVAL;
    }
    else
        errno = EINVAL;

    return ret;
}

int fstat(int file, struct stat *st)
{
    if ((STDOUT_FILENO == file) || (STDERR_FILENO == file))
    {
        st->st_mode = S_IFCHR;
        return 0;
    }
    else
    {
        const ITPFSDevice *dev = (ITPFSDevice *) itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

        if (dev)
        {
            /*  Pass call on to device driver.  Return result.        */
            return dev->fstat(file & ITP_FILE_MASK, st);
        }
        else
        {
            errno = ENOENT;
            return -1;
        }
    }
}

char *getcwd(char *buf, size_t size)
{
    ITPDriveStatus *driveStatusTable;
    ITPDriveStatus *driveStatus;
    char           *dir = NULL;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);
    driveStatus = &driveStatusTable[volume];

    if (driveStatus->avail)
    {
        const ITPFSDevice *dev = (ITPFSDevice *) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
        if (dev)
            dir = dev->getcwd(buf, size);
        else
        {
            errno = ENOENT;
            return NULL;
        }
    }

    if (dir == NULL)
        errno = EINVAL;

    return dir;
}

int rmdir(const char *path)
{
    ITPDriveStatus *driveStatusTable;
    ITPDriveStatus *driveStatus;
    int            ret = -1;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);
    driveStatus = &driveStatusTable[volume];

    if (driveStatus->avail)
    {
        const ITPFSDevice *dev = (ITPFSDevice *) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
        if (dev)
            ret = dev->rmdir(path);
        else
        {
            errno = ENOENT;
            return -1;
        }
    }

    if (ret)
        errno = EINVAL;

    return ret;
}

#else // CFG_WIN32_FS_HW

int stat(const char *path, struct stat *buf)
{
    const ITPFSDevice *dev = &itpFSDeviceFile;
    if (dev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return dev->stat(path, buf);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

_CRTIMP int __cdecl _fstat64i32(_In_ int _FileDes, _Out_ struct _stat64i32 * _Stat);

int fstat(int file, struct stat *st)
{
    if ((file & ITP_DEVICE_MASK) == 0)
    {
        struct _stat buff;
        int ret = _fstat(file, &buff);
        if (ret == 0)
        {
            st->st_dev = buff.st_dev;
            st->st_ino = buff.st_ino;
            st->st_mode = buff.st_mode;
            st->st_nlink = buff.st_nlink;
            st->st_uid = buff.st_uid;
            st->st_gid = buff.st_gid;
            st->st_rdev = buff.st_rdev;
            st->st_size = buff.st_size;
            st->st_atime = buff.st_atime;
            st->st_mtime = buff.st_mtime;
            st->st_ctime = buff.st_ctime;
            st->st_blksize = 2048;
            st->st_blocks = 0;
        }
        return ret;
    }
    else
    {
        const ITPFSDevice *dev = &itpFSDeviceFile;
        if (dev)
        {
            /*  Pass call on to device driver.  Return result.        */
            return dev->fstat(file & ITP_FILE_MASK, st);
        }
        else
        {
            errno = ENOENT;
            return -1;
        }
    }
}

int closedir(DIR *dirp)
{
    _findclose(dirp->dd_fd);
    free(dirp->dd_buf);
    free(dirp);
    return 0;
}

DIR *opendir(const char *dirname)
{
    intptr_t ret;
    DIR      *dirp = (DIR *)malloc(sizeof(DIR));
    if (!dirp)
    {
        LOG_ERR "%s\r\n", GetErrorString(GetLastError()) LOG_END
        return NULL;
    }

    #ifdef ITP_FAT_UTF8
    {
        wchar_t             buf[PATH_MAX + 1];
        struct _wfinddata_t *find = (struct _wfinddata_t *)malloc(sizeof(struct _wfinddata_t) + sizeof(struct dirent) + strlen(dirname) + 1);
        if (!find)
        {
            LOG_ERR "%s\r\n", GetErrorString(GetLastError()) LOG_END
            free(dirp);
            return NULL;
        }

        MultiByteToWideChar(CP_UTF8, 0, dirname, -1, buf, PATH_MAX + 1);

        if (buf[wcslen(buf) - 1] == L'/')
            wcscat(buf, L"*.*");
        else
            wcscat(buf, L"/*.*");

        ret = _wfindfirst(buf, find);
        if (ret == -1)
        {
            LOG_ERR "%s\r\n", GetErrorString(GetLastError()) LOG_END
            free(find);
            free(dirp);
            return NULL;
        }

        dirp->dd_fd   = ret;
        dirp->dd_size = sizeof(struct _wfinddata_t);
        dirp->dd_buf  = (char *) find;
        dirp->dd_loc  = -1;

        strcpy(&dirp->dd_buf[sizeof(struct _wfinddata_t) + sizeof(struct dirent)], dirname);
    }

    #else
    {
        char               buf[PATH_MAX + 1];
        struct _finddata_t *find = (struct _finddata_t *)malloc(sizeof(struct _finddata_t) + sizeof(struct dirent) + strlen(dirname) + 1);
        if (!find)
        {
            LOG_ERR "%s\r\n", GetErrorString(GetLastError()) LOG_END
            free(dirp);
            return NULL;
        }

        strcpy(buf, dirname);

        if (buf[strlen(buf) - 1] == '/' || buf[strlen(buf) - 1] == '\\')
            strcat(buf, "*.*");
        else
            strcat(buf, "/*.*");

        ret = _findfirst(buf, find);
        if (-1 == ret)
        {
            LOG_ERR "%s\r\n", GetErrorString(GetLastError()) LOG_END
            free(find);
            free(dirp);
            return NULL;
        }

        dirp->dd_fd   = ret;
        dirp->dd_size = sizeof(struct _finddata_t);
        dirp->dd_buf  = (char *) find;
        dirp->dd_loc  = -1;

        strcpy(&dirp->dd_buf[sizeof(struct _finddata_t) + sizeof(struct dirent)], dirname);
    }
    #endif // ITP_FAT_UTF8

    return dirp;
}

struct dirent *readdir(DIR *dirp)
{
    struct dirent *d;

    if (dirp->dd_size == sizeof(struct _wfinddata_t))
    {
        struct _wfinddata_t *find = (struct _wfinddata_t *) dirp->dd_buf;

        d = (struct dirent *) (dirp->dd_buf + sizeof(struct _wfinddata_t));

        if (dirp->dd_loc >= 0)
        {
            int ret = _wfindnext(dirp->dd_fd, find);
            if (ret)
            {
                //LOG_ERR "%s\r\n", GetErrorString(GetLastError()) LOG_END
                return NULL;
            }
        }
        d->d_ino    = ++dirp->dd_loc;
        d->d_type   = (find->attrib & _A_SUBDIR) ? DT_DIR : DT_REG;
        d->d_namlen = WideCharToMultiByte(CP_UTF8, 0, find->name, -1, d->d_name, NAME_MAX + 1, 0, 0);
    }
    else
    {
        struct _finddata_t *find = (struct _finddata_t *) dirp->dd_buf;

        d = (struct dirent *) (dirp->dd_buf + sizeof(struct _finddata_t));

        if (dirp->dd_loc >= 0)
        {
            int ret = _findnext(dirp->dd_fd, find);
            if (ret)
            {
                LOG_ERR "%s\r\n", GetErrorString(GetLastError()) LOG_END
                return NULL;
            }
        }
        d->d_ino    = ++dirp->dd_loc;
        d->d_type   = (find->attrib & _A_SUBDIR) ? DT_DIR : DT_REG;
        d->d_namlen = strlen(find->name);
        strncpy(d->d_name, find->name, d->d_namlen + 1);
    }

    return d;
}

void rewinddir(DIR *dirp)
{
    intptr_t ret;

    if (dirp->dd_size == sizeof(struct _wfinddata_t))
    {
        struct _wfinddata_t *find = (struct _wfinddata_t *) dirp->dd_buf;
        wchar_t             buf[PATH_MAX + 1];

        MultiByteToWideChar(CP_UTF8, 0, &dirp->dd_buf[sizeof(struct _wfinddata_t) + sizeof(struct dirent)], -1, buf, PATH_MAX + 1);
        if (buf[wcslen(buf) - 1] == L'/')
            wcscat(buf, L"*.*");
        else
            wcscat(buf, L"/*.*");

        ret = _wfindfirst(buf, find);
        if (ret == -1)
        {
            LOG_ERR "%s\r\n", GetErrorString(GetLastError()) LOG_END
            return;
        }
    }
    else
    {
        struct _finddata_t *find = (struct _finddata_t *) dirp->dd_buf;
        char               buf[PATH_MAX + 1];

        strcpy(buf, &dirp->dd_buf[sizeof(struct _finddata_t) + sizeof(struct dirent)]);
        if (buf[strlen(buf) - 1] == '/')
            strcat(buf, "*.*");
        else
            strcat(buf, "/*.*");

        ret = _findfirst(buf, find);
        if (ret == -1)
        {
            LOG_ERR "%s\r\n", GetErrorString(GetLastError()) LOG_END
            return;
        }
    }

    _findclose(dirp->dd_fd);
    dirp->dd_fd  = ret;
    dirp->dd_loc = -1;
}

int scandir(const char *dirname,
            struct dirent ***namelist,
            int (*select)(const struct dirent *),
            int (*dcomp)(const struct dirent **, const struct dirent **))
{
    register struct dirent *d, *p, **names;
    register size_t        nitems;
    long                   arraysz;
    DIR                    *dirp;
    int                    successful = 0;
    int                    rc         = 0;

    dirp  = NULL;
    names = NULL;

    if ((dirp = opendir(dirname)) == NULL)
        return(-1);

    arraysz = 1;
    names   = (struct dirent **)malloc(arraysz * sizeof(struct dirent *));
    if (names == NULL)
        goto cleanup;

    nitems = 0;
    while ((d = readdir(dirp)) != NULL)
    {
        if (select != NULL && !(*select)(d))
            continue;                   /* just selected names */
        /*
         * Make a minimum size copy of the data
         */
        p           = (struct dirent *)malloc(sizeof(struct dirent));
        if (p == NULL)
            goto cleanup;
        p->d_ino    = d->d_ino;
        p->d_type   = d->d_type;
        p->d_namlen = d->d_namlen;
        strcpy(p->d_name, d->d_name);

        /*
         * Check to make sure the array has space left and
         * realloc the maximum size.
         */
        if (++nitems >= (size_t)arraysz)
        {
            arraysz = nitems;
            names   = (struct dirent **)realloc((char *)names,
                                                arraysz * sizeof(struct dirent *));
            if (names == NULL)
                goto cleanup;
        }
        names[nitems - 1] = p;
    }
    successful = 1;
cleanup:
    closedir(dirp);
    if (successful)
    {
        if (nitems && dcomp != NULL)
            qsort(names, nitems, sizeof(struct dirent *), (int (*)(const void *, const void *))dcomp);
        *namelist = names;
        rc        = nitems;
    }
    else          /* We were unsuccessful, clean up storage and return -1.  */
    {
        if (names)
        {
            size_t i;
            for (i = 0; i < nitems; i++)
                free( names[i] );
            free( names );
        }
        rc = -1;
    }

    return(rc);
}

int alphasort(const struct dirent **d1, const struct dirent **d2)
{
    return(strcmp((*d1)->d_name, (*d2)->d_name));
}

int statvfs(const char *path, struct statvfs *sbuf)
{
    ULONGLONG free_bytes_available;       /* for user - similar to bavail */
    ULONGLONG total_number_of_bytes;
    ULONGLONG total_number_of_free_bytes; /* for everyone - bfree */

    if (!GetDiskFreeSpaceEx (path,
                             (PULARGE_INTEGER) &free_bytes_available,
                             (PULARGE_INTEGER) &total_number_of_bytes,
                             (PULARGE_INTEGER) &total_number_of_free_bytes))
    {
        LOG_ERR "%s\r\n", GetErrorString(GetLastError()) LOG_END
        return -1;
    }
    if (total_number_of_bytes < 16ULL * 1024 * 1024 * 1024 * 1024)
        sbuf->f_bsize = 4096;
    else if (total_number_of_bytes < 32ULL * 1024 * 1024 * 1024 * 1024)
        sbuf->f_bsize = 8192;
    else if (total_number_of_bytes < 64ULL * 1024 * 1024 * 1024 * 1024)
        sbuf->f_bsize = 16384;
    else if (total_number_of_bytes < 128ULL * 1024 * 1024 * 1024 * 1024)
        sbuf->f_bsize = 32768;
    else
        sbuf->f_bsize = 65536;

    /* As with stat, -1 indicates a field is not known. */
    sbuf->f_frsize  = sbuf->f_bsize;
    sbuf->f_blocks  = (fsblkcnt_t)(total_number_of_bytes / sbuf->f_bsize);
    sbuf->f_bfree   = (fsblkcnt_t)(total_number_of_free_bytes / sbuf->f_bsize);
    sbuf->f_bavail  = (fsblkcnt_t)(free_bytes_available / sbuf->f_bsize);
    sbuf->f_files   = -1;
    sbuf->f_ffree   = -1;
    sbuf->f_favail  = -1;
    sbuf->f_fsid    = -1;
    sbuf->f_flag    = -1;
    sbuf->f_namemax = FILENAME_MAX;

    return 0;
}

#endif // CFG_WIN32_FS_HW

int fcntl(int s, int cmd, int val)
{
#ifdef CFG_NET_ENABLE
    if ((s & ITP_DEVICE_MASK) == 0)
    {
        return lwip_fcntl(s, cmd, val);
    }
    else
#endif // CFG_NET_ENABLE    
    {
        LOG_DBG "%s NO IMPL: %d\n", __FUNCTION__, cmd LOG_END
        return 0;
    }
}

int usleep(useconds_t useconds)
{
    LARGE_INTEGER freq, start, curr, setting;

    if (useconds >= 1000)
    {
        Sleep(useconds / 1000);
        useconds /= 1000;
    }

    QueryPerformanceFrequency(&freq);
    setting.QuadPart = (LONGLONG) (freq.QuadPart * useconds * 0.000001f);
    QueryPerformanceCounter(&start);

    do
    {
        QueryPerformanceCounter(&curr);
    } while ((curr.QuadPart - start.QuadPart) < setting.QuadPart);

    return 0;
}

unsigned sleep(unsigned seconds)
{
    Sleep(seconds * 1000);
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    if (req->tv_sec > 0)
        sleep(req->tv_sec);

    if (req->tv_nsec > 0)
        usleep(req->tv_nsec / 1000);

    if (rem)
        rem->tv_sec = rem->tv_sec = 0;

    return 0;
}

int timer_create(clockid_t clock_id, struct sigevent *evp, timer_t *timerid)
{
    itpTimer *t = (itpTimer *) malloc(sizeof(itpTimer));
    if (t == NULL)
        return -1;

    t->pxTimer = NULL;
    t->routine = NULL;
    t->arg     = 0;

    *timerid   = (timer_t)t;

    return 0;
}

int timer_delete(timer_t timerid)
{
    itpTimer *timer = (itpTimer *) timerid;

    if (timer->pxTimer)
    {
        if (timeKillEvent((UINT) timer->pxTimer) != TIMERR_NOERROR)
            return -1;
    }

    free(timer);
    return 0;
}

int timer_connect(timer_t timerid, VOIDFUNCPTR routine, int arg)
{
    ((itpTimer *) timerid)->routine = routine;
    ((itpTimer *) timerid)->arg     = arg;
    return 0;
}

static void CALLBACK TimerProc(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
    ((itpTimer *) dwUser)->routine(dwUser, ((itpTimer *) dwUser)->arg);
}

int timer_settime(timer_t timerid, int flags, const struct itimerspec *value, struct itimerspec *ovalue)
{
    itpTimer *timer = (itpTimer *) timerid;
    UINT     uDelay, fuEvent;

    if (value->it_value.tv_sec == 0 && value->it_value.tv_nsec == 0)
    {
        if (timer->pxTimer)
        {
            timeKillEvent((UINT) timer->pxTimer);
            timer->pxTimer = NULL;
        }
        return 0;
    }
    else if (value->it_interval.tv_sec > 0 || value->it_interval.tv_nsec > 0)
    {
        uDelay  = (UINT) value->it_interval.tv_sec * 1000;
        uDelay += (UINT) value->it_interval.tv_nsec / 1000000;
        fuEvent = TIME_PERIODIC;
    }
    else
    {
        uDelay  = (UINT) value->it_value.tv_sec * 1000;
        uDelay += (UINT) value->it_value.tv_nsec / 1000000;
        fuEvent = TIME_ONESHOT;
    }

    if (timer->pxTimer)
        timeKillEvent((UINT) timer->pxTimer);

    timer->pxTimer = (void *) timeSetEvent(uDelay, 1, TimerProc, (DWORD) timerid, fuEvent);
    if (!timer->pxTimer)
        return -1;

    return 0;
}

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    struct timeval now;
    int ret = gettimeofday(&now, NULL);

    if (ret != 0)
        return ret;

    tp->tv_sec = now.tv_sec;
    tp->tv_nsec = now.tv_usec * 1000;
    return 0;
}

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME         ft;
    unsigned __int64 tmpres = 0;
    static int       tzflag;

    if (NULL != tv)
    {
        GetSystemTimeAsFileTime(&ft);

        tmpres     |= ft.dwHighDateTime;
        tmpres    <<= 32;
        tmpres     |= ft.dwLowDateTime;

        /*converting file time to unix epoch*/
        tmpres     -= 11644473600000000Ui64;
        tmpres     /= 10; /*convert into microseconds*/
        //tv->tv_sec  = tmpres / 1000000UL;
        time(&tv->tv_sec);
        tv->tv_usec = tmpres % 1000000UL;
    }

    if (NULL != tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime     = _daylight;
    }

    return 0;
}

int settimeofday(const struct timeval *tv, const struct timezone *tz)
{
#define FILETIME_1970        116444736000000000ull /* seconds between 1/1/1601 and 1/1/1970 */
#define HECTONANOSEC_PER_SEC 10000000ull
#define TIMEVAL_TO_TIMESPEC(tv, ts) { \
        (ts)->tv_sec  = (tv)->tv_sec; \
        (ts)->tv_nsec = (tv)->tv_usec * 1000; \
}

    struct timespec tp;
    SYSTEMTIME      st;
    ULARGE_INTEGER  fti;

    if (tv == NULL)
        return -1;

    TIMEVAL_TO_TIMESPEC (tv, &tp);

    fti.QuadPart = UInt32x32To64(tp.tv_sec, HECTONANOSEC_PER_SEC) + tp.tv_nsec % 100 + FILETIME_1970;
    FileTimeToSystemTime ((LPFILETIME) &fti, &st);
    if (!SetSystemTime (&st))
        return -1;

    return 0;
}

struct tm *localtime_r(const time_t *timer, struct tm *result)
{
    struct tm *local_result;
    local_result = localtime(timer);

    if (local_result == NULL || result == NULL)
        return NULL;

    memcpy(result, local_result, sizeof(result));
    return result;
}

long sysconf(int __name)
{
    return -1;
}

int vasprintf(char **sptr, char *fmt, va_list argv)
{
    int wanted = vsnprintf(*sptr = NULL, 0, fmt, argv);
    if ((wanted > 0) && ((*sptr = malloc(1 + wanted)) != NULL))
        return vsprintf(*sptr, fmt, argv);

    return wanted;
}

int waitpid(pid_t pid, int *status, int options)
{
    return -1;
}

pid_t fork(void)
{
    return 0;
}

int pipe(int __fildes[2])
{
    return 0;
}

pid_t getpid(void)
{
    return (int) GetCurrentThreadId();
}

uid_t getuid(void)
{
    return 0;
}

gid_t getgid(void)
{
    return 0;
}

uid_t geteuid(void)
{
    return 0;
}

gid_t getegid(void)
{
    return 0;
}

int asprintf(char **sptr, char *fmt, ...)
{
    int     retval;
    va_list argv;
    va_start(argv, fmt);
    retval = vasprintf(sptr, fmt, argv);
    va_end(argv);
    return retval;
}

size_t strlcpy(char *dst, const char *src, size_t size)
{
    strncpy(dst, src, size);

    if (size > 0)
        dst[size - 1] = '\0';

    return strlen(src);
}

size_t strlcat(char *dest, const char *src, size_t maxlen)
{
    size_t src_len, dst_len;
    size_t len, needed;
    src_len = strlen(src);

    {
        char *end = (char *)memchr(dest, '\0', maxlen);
        if (!end)
            return maxlen + src_len;

        dst_len = end - dest;
    }

    len = needed = dst_len + src_len + 1;

    if (len >= maxlen)
        len = maxlen - 1;

    memcpy(dest + dst_len, src, len - dst_len);
    dest[len] = '\0';

    return needed - 1;
}

float cbrtf(float xx)
{
    static float CBRT2 = 1.25992104989487316477f;
    static float CBRT4 = 1.58740105196819947475f;
    int          e, rem, sign;
    float        x, z;

    x = xx;
    if (x == 0)
        return(0.0);
    if (x > 0)
        sign = 1;
    else
    {
        sign = -1;
        x    = -x;
    }

    z = x;
    /* extract power of 2, leaving
     * mantissa between 0.5 and 1
     */
    x = frexpf(x, &e);

    /* Approximate cube root of number between .5 and 1,
     * peak relative error = 9.2e-6
     */
    x = (((-0.13466110473359520655053f * x
           + 0.54664601366395524503440f) * x
          - 0.95438224771509446525043f) * x
         + 1.1399983354717293273738f) * x
        + 0.40238979564544752126924f;

    /* exponent divided by 3 */
    if (e >= 0)
    {
        rem  = e;
        e   /= 3;
        rem -= 3 * e;
        if (rem == 1)
            x *= CBRT2;
        else if (rem == 2)
            x *= CBRT4;
    }
    /* argument less than 1 */
    else
    {
        e    = -e;
        rem  = e;
        e   /= 3;
        rem -= 3 * e;
        if (rem == 1)
            x /= CBRT2;
        else if (rem == 2)
            x /= CBRT4;
        e = -e;
    }

    /* multiply by power of 2 */
    x  = ldexpf(x, e);

    /* Newton iteration */
    x -= (x - (z / (x * x))) * 0.333333333333f;

    if (sign < 0)
        x = -x;

    return(x);
}

void itpSemPostFromISR(sem_t *sem)
{}

int itpSemWaitTimeout(sem_t *sem, unsigned long ms)
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);

    ts.tv_sec  += ms / 1000;
    ts.tv_nsec += (ms % 1000) * 1000000;

    return sem_timedwait(sem, &ts);
}

void *itpReturnAddress(unsigned int level)
{
    return NULL;
}

void *__builtin_frame_address(unsigned int level)
{
    return NULL;
}

typedef struct queue
{
    uint8_t         *buffer;
    int             capacity;
    int             size;
    int             in;
    int             out;
    pthread_mutex_t mutex;
    pthread_cond_t  cond_full;
    pthread_cond_t  cond_empty;
    unsigned long   uxItemSize;
    int             oflag;
} queue_t;

mqd_t mq_open(const char *name, int oflag, ...)
{
    if (oflag & O_CREAT)
    {
        mode_t         mode;
        struct mq_attr *attr;
        va_list        ap;
        queue_t        *q = (queue_t *)malloc(sizeof(queue_t));
        if (!q)
        {
            return -1;
        }

        va_start(ap, oflag);
        mode      = va_arg(ap, mode_t);
        attr      = va_arg(ap, struct mq_attr *);
        va_end(ap);

        q->buffer = malloc(attr->mq_maxmsg * attr->mq_msgsize);
        if (!q->buffer)
        {
            free(q);
            return -1;
        }

        q->capacity   = attr->mq_maxmsg;
        q->size       = 0;
        q->in         = 0;
        q->out        = 0;
        q->mutex      = PTHREAD_MUTEX_INITIALIZER;
        q->cond_full  = PTHREAD_COND_INITIALIZER;
        q->cond_empty = PTHREAD_COND_INITIALIZER;
        q->uxItemSize = attr->mq_msgsize;
        q->oflag      = oflag | attr->mq_flags;

        return (mqd_t)q;
    }
    else
    {
        return -1;
    }
}

int mq_close(mqd_t msgid)
{
    queue_t *q = (queue_t *) msgid;
    free(q->buffer);
    free(q);
    return 0;
}

int mq_send(mqd_t msgid, const char *msg, size_t msg_len, unsigned int msg_prio)
{
    queue_t *queue = (queue_t *) msgid;

    if ((queue->oflag & O_NONBLOCK) && queue->size == queue->capacity)
        return 0;

    pthread_mutex_lock(&(queue->mutex));
    while (queue->size == queue->capacity)
        pthread_cond_wait(&(queue->cond_full), &(queue->mutex));

    memcpy(queue->buffer + queue->in * queue->uxItemSize, msg, queue->uxItemSize);
    ++queue->size;
    ++queue->in;
    queue->in %= queue->capacity;
    pthread_mutex_unlock(&(queue->mutex));
    pthread_cond_broadcast(&(queue->cond_empty));

    return 0;
}

int mq_timedsend(mqd_t msgid, const char *msg, size_t msg_len, unsigned msg_prio, const struct timespec *abs_timeout)
{
    queue_t *queue = (queue_t *)msgid;

    if ((queue->oflag & O_NONBLOCK) && queue->size == queue->capacity)
        return 0;

    pthread_mutex_lock(&(queue->mutex));
    while (queue->size == queue->capacity)
        pthread_cond_timedwait(&(queue->cond_full), &(queue->mutex), abs_timeout);

    memcpy(queue->buffer + queue->in * queue->uxItemSize, msg, queue->uxItemSize);
    ++queue->size;
    ++queue->in;
    queue->in %= queue->capacity;
    pthread_mutex_unlock(&(queue->mutex));
    pthread_cond_broadcast(&(queue->cond_empty));

    return 0;
}

ssize_t mq_receive(mqd_t msgid, char *msg, size_t msg_len, unsigned int *msg_prio)
{
    queue_t *queue = (queue_t *) msgid;

    if ((queue->oflag & O_NONBLOCK) && queue->size == 0)
        return 0;

    pthread_mutex_lock(&(queue->mutex));
    while (queue->size == 0)
        pthread_cond_wait(&(queue->cond_empty), &(queue->mutex));

    memcpy(msg, queue->buffer + queue->out * queue->uxItemSize, queue->uxItemSize);
    --queue->size;
    ++queue->out;
    queue->out %= queue->capacity;
    pthread_mutex_unlock(&(queue->mutex));
    pthread_cond_broadcast(&(queue->cond_full));

    return queue->uxItemSize;
}

ssize_t mq_timedreceive(mqd_t msgid, char *msg, size_t msg_len, unsigned int *msg_prio, const struct timespec *abs_timeout)
{
    queue_t *queue = (queue_t *) msgid;

    if ((queue->oflag & O_NONBLOCK) && queue->size == 0)
        return 0;

    pthread_mutex_lock(&(queue->mutex));
    while (queue->size == 0)
        pthread_cond_timedwait(&(queue->cond_empty), &(queue->mutex), abs_timeout);

    memcpy(msg, queue->buffer + queue->out * queue->uxItemSize, queue->uxItemSize);
    --queue->size;
    ++queue->out;
    queue->out %= queue->capacity;
    pthread_mutex_unlock(&(queue->mutex));
    pthread_cond_broadcast(&(queue->cond_full));

    return queue->uxItemSize;
}

#if !defined(CFG_DBG_MEMLEAK) && !defined(CFG_DBG_RMALLOC)

char *strndup(const char *str, size_t n)
{
    const char *ptr = str;
    size_t     len;
    char       *copy;

    while (n-- > 0 && *ptr)
        ptr++;

    len  = ptr - str;

    copy = (char *)malloc(len + 1);
    if (copy)
    {
        memcpy(copy, str, len);
        copy[len] = '\0';
    }
    return copy;
}

#endif // !CFG_DBG_MEMLEAK

char *strsep(char **stringp, const char *delim)
{
    char *begin, *end;

    begin = *stringp;
    if (begin == NULL)
        return NULL;

    /* A frequent case is when the delimiter string contains only one
       character.  Here we don't need to call the expensive `strpbrk'
       function and instead work using `strchr'.  */
    if (delim[0] == '\0' || delim[1] == '\0')
    {
        char ch = delim[0];

        if (ch == '\0')
            end = NULL;
        else
        {
            if (*begin == ch)
                end = begin;
            else if (*begin == '\0')
                end = NULL;
            else
                end = strchr (begin + 1, ch);
        }
    }
    else
        /* Find the end of the token.  */
        end = strpbrk (begin, delim);

    if (end)
    {
        /* Terminate the token and set *STRINGP past NUL character.  */
        *end++   = '\0';
        *stringp = end;
    }
    else
        /* No more delimiters; this is the last token.  */
        *stringp = NULL;

    return begin;
}

char *strerror_r(int errnum, char *buffer, size_t n)
{
    char *error;
    error = strerror (errnum);

    return strncpy (buffer, (const char *)error, n);
}

void bzero(char *s, int n)
{
    memset(s, 0, n);
}

long itpTimevalDiff(struct timeval *starttime, struct timeval *finishtime)
{
    long msec;
    msec  = (long)(finishtime->tv_sec - starttime->tv_sec) * 1000;
    msec += (long)(finishtime->tv_usec - starttime->tv_usec) / 1000;
    return msec;
}

uint32_t itpGetTickCount(void)
{
    return GetTickCount();
}

uint32_t itpGetTickDuration(uint32_t tick)
{
    return GetTickCount() - tick;
}

struct mallinfo mallinfo(void)
{
    struct mallinfo mi;
    mi.arena    = ithVmem->totalSize;
    mi.uordblks = ithVmem->totalSize - ithVmem->freeSize;
    return mi;
}

void malloc_stats(void)
{
    ithVmemStats();
}

uint32_t itpGetFreeHeapSize(void)
{
    return ithVmem->freeSize;
}

char *basename(char *path)
{
    char *p;
    if (path == NULL || *path == '\0')
        return ".";
    p = path + strlen(path) - 1;
    while (*p == '/')
    {
        if (p == path)
            return path;
        *p-- = '\0';
    }
    while (p >= path && *p != '/')
        p--;
    return p + 1;
}

char *dirname(char *path)
{
    char *p;
    if (path == NULL || *path == '\0')
        return ".";
    p = path + strlen(path) - 1;
    while (*p == '/')
    {
        if (p == path)
            return path;
        *p-- = '\0';
    }
    while (p >= path && *p != '/')
        p--;
    return
        p < path ? "." :
        p == path ? "/" :
        (*p = '\0', path);
}

int uname(struct utsname *name)
{
    memset(name, 0, sizeof(struct utsname));
    strcpy(name->sysname, CFG_SYSTEM_NAME);
    strcpy(name->version, CFG_VERSION_MAJOR_STR "." CFG_VERSION_MINOR_STR "." CFG_VERSION_PATCH_STR "." CFG_VERSION_CUSTOM_STR);
    strcpy(name->release, CFG_VERSION_TWEAK_STR);
    return 0;
}

int daemon(int nochdir, int noclose)
{
    return 0;
}

void openlog(char *ident, int option, int facility)
{}

void vsyslog(int pri, const char *format, va_list ap)
{}

ssize_t getline(char **bufptr, size_t *n, FILE *fp)
{
#define MIN_LINE_SIZE     4
#define DEFAULT_LINE_SIZE 128

    char   *buf;
    char   *ptr;
    size_t newsize, numbytes;
    int    pos;
    int    ch;
    int    cont;

    if (fp == NULL || bufptr == NULL || n == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    buf = *bufptr;
    if (buf == NULL || *n < MIN_LINE_SIZE)
    {
        buf = (char *)realloc (*bufptr, DEFAULT_LINE_SIZE);
        if (buf == NULL)
        {
            return -1;
        }
        *bufptr = buf;
        *n      = DEFAULT_LINE_SIZE;
    }

    numbytes = *n;
    ptr      = buf;

    cont     = 1;

    while (cont)
    {
        /* fill buffer - leaving room for nul-terminator */
        while (--numbytes > 0)
        {
            if ((ch = getc (fp)) == EOF)
            {
                cont = 0;
                break;
            }
            else
            {
                *ptr++ = ch;
                if (ch == '\n')
                {
                    cont = 0;
                    break;
                }
            }
        }

        if (cont)
        {
            /* Buffer is too small so reallocate a larger buffer.  */
            pos     = ptr - buf;
            newsize = (*n << 1);
            buf     = realloc (buf, newsize);
            if (buf == NULL)
            {
                cont = 0;
                break;
            }

            /* After reallocating, continue in new buffer */
            *bufptr  = buf;
            *n       = newsize;
            ptr      = buf + pos;
            numbytes = newsize - pos;
        }
    }

    /* if no input data, return failure */
    if (ptr == buf)
        return -1;

    /* otherwise, nul-terminate and return number of bytes read */
    *ptr = '\0';
    return (ssize_t)(ptr - buf);
}

#ifndef _DIAGASSERT
    #define _DIAGASSERT(e)
#endif

int  opterr = 1;                /* if error message should be printed */
int  optind = 1;                /* index into parent argv vector */
int  optopt = '?';              /* character checked for validity */
int  optreset;                  /* reset getopt */
char *optarg;                   /* argument associated with option */

#define IGNORE_FIRST       (*options == '-' || *options == '+')
#define PRINT_ERROR        ((opterr) && ((*options != ':') \
                                         || (IGNORE_FIRST && options[1] != ':')))
#define IS_POSIXLY_CORRECT (getenv("POSIXLY_CORRECT") != NULL)
#define PERMUTE            (!IS_POSIXLY_CORRECT && !IGNORE_FIRST)
/* XXX: GNU ignores PC if *options == '-' */
#define IN_ORDER           (!IS_POSIXLY_CORRECT && *options == '-')

/* return values */
#define BADCH              (int)'?'
#define BADARG             (int)':'
#define INORDER            (int)1

#define EMSG               ""

static int getopt_internal(int, char *const *, const char *);
static int gcd(int, int);
static void permute_args(int, int, int, char *const *);
static void xwarnx(const char *, ...);

static char       *place         = EMSG; /* option letter processing */

/* XXX: set optreset to 1 rather than these two */
static int        nonopt_start   = -1; /* first non option argument (for permute) */
static int        nonopt_end     = -1; /* first option after non options (for permute) */

/* Error messages */
static const char recargchar[]   = "option requires an argument -- %c";
static const char recargstring[] = "option requires an argument -- %s";
static const char ambig[]        = "ambiguous option -- %.*s";
static const char noarg[]        = "option doesn't take an argument -- %.*s";
static const char illoptchar[]   = "illegal option -- %c";
static const char illoptstring[] = "illegal option -- %s";

static const char *progname;

/* Replacement for warnx(3) for systems without it. */
static void xwarnx(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if (progname)
        (void) fprintf(stderr, "%s: ", progname);
    if (fmt)
        (void) vfprintf(stderr, fmt, ap);
    (void) fprintf(stderr, "\n");
    va_end(ap);
}

/*
 * Compute the greatest common divisor of a and b.
 */
static int
gcd(int a, int b)
{
    int c;

    c = a % b;
    while (c != 0)
    {
        a = b;
        b = c;
        c = a % b;
    }

    return b;
}

/*
 * Exchange the block from nonopt_start to nonopt_end with the block
 * from nonopt_end to opt_end (keeping the same order of arguments
 * in each block).
 */
static void
permute_args(int nonopt_start, int nonopt_end, int opt_end, char *const *nargv)
{
    int  cstart, cyclelen, i, j, ncycle, nnonopts, nopts, pos;
    char *swap;

    /*
     * compute lengths of blocks and number and size of cycles
     */
    nnonopts = nonopt_end - nonopt_start;
    nopts    = opt_end - nonopt_end;
    ncycle   = gcd(nnonopts, nopts);
    cyclelen = (opt_end - nonopt_start) / ncycle;

    for (i = 0; i < ncycle; i++)
    {
        cstart = nonopt_end + i;
        pos    = cstart;
        for (j = 0; j < cyclelen; j++)
        {
            if (pos >= nonopt_end)
                pos -= nnonopts;
            else
                pos += nopts;
            swap                     = nargv[pos];
            /* LINTED const cast */
            ((char **) nargv)[pos]   = nargv[cstart];
            /* LINTED const cast */
            ((char **)nargv)[cstart] = swap;
        }
    }
}

/*
 * getopt_internal --
 *      Parse argc/argv argument vector.  Called by user level routines.
 *  Returns -2 if -- is found (can be long option or end of options marker).
 */
static int
getopt_internal(int nargc, char *const *nargv, const char *options)
{
    const char *oli;            /* option letter list index */
    int        optchar;

    _DIAGASSERT(nargv != NULL);
    _DIAGASSERT(options != NULL);

    optarg = NULL;

    /*
     * XXX Some programs (like rsyncd) expect to be able to
     * XXX re-initialize optind to 0 and have getopt_long(3)
     * XXX properly function again.  Work around this braindamage.
     */
    if (optind == 0)
        optind = 1;

    if (optreset)
        nonopt_start = nonopt_end = -1;
start:
    if (optreset || !*place)                    /* update scanning pointer */
    {
        optreset = 0;
        if (optind >= nargc)                    /* end of argument vector */
        {
            place = EMSG;
            if (nonopt_end != -1)
            {
                /* do permutation, if we have to */
                permute_args(nonopt_start, nonopt_end,
                             optind, nargv);
                optind -= nonopt_end - nonopt_start;
            }
            else if (nonopt_start != -1)
            {
                /*
                 * If we skipped non-options, set optind
                 * to the first of them.
                 */
                optind = nonopt_start;
            }
            nonopt_start = nonopt_end = -1;
            return -1;
        }
        if (*(place = nargv[optind]) != '-')            /* found non-option */
        {
            place = EMSG;
            if (IN_ORDER)
            {
                /*
                 * GNU extension:
                 * return non-option as argument to option 1
                 */
                optarg = nargv[optind++];
                return INORDER;
            }
            if (!PERMUTE)
            {
                /*
                 * if no permutation wanted, stop parsing
                 * at first non-option
                 */
                return -1;
            }
            /* do permutation */
            if (nonopt_start == -1)
                nonopt_start = optind;
            else if (nonopt_end != -1)
            {
                permute_args(nonopt_start, nonopt_end,
                             optind, nargv);
                nonopt_start = optind -
                               (nonopt_end - nonopt_start);
                nonopt_end   = -1;
            }
            optind++;
            /* process next argument */
            goto start;
        }
        if (nonopt_start != -1 && nonopt_end == -1)
            nonopt_end = optind;
        if (place[1] && *++place == '-')                /* found "--" */
        {
            place++;
            return -2;
        }
    }
    if ((optchar = (int)*place++) == (int)':' ||
        (oli = strchr(options + (IGNORE_FIRST ? 1 : 0), optchar)) == NULL)
    {
        /* option letter unknown or ':' */
        if (!*place)
            ++optind;
        if (PRINT_ERROR)
            xwarnx(illoptchar, optchar);
        optopt = optchar;
        return BADCH;
    }
    if (optchar == 'W' && oli[1] == ';')                /* -W long-option */
    {           /* XXX: what if no long options provided (called by getopt)? */
        if (*place)
            return -2;

        if (++optind >= nargc)                  /* no arg */
        {
            place  = EMSG;
            if (PRINT_ERROR)
                xwarnx(recargchar, optchar);
            optopt = optchar;
            /* XXX: GNU returns '?' if options[0] != ':' */
            return BADARG;
        }
        else                                    /* white space */
            place = nargv[optind];
        /*
         * Handle -W arg the same as --arg (which causes getopt to
         * stop parsing).
         */
        return -2;
    }
    if (*++oli != ':')                          /* doesn't take argument */
    {
        if (!*place)
            ++optind;
    }
    else                                        /* takes (optional) argument */
    {
        optarg = NULL;
        if (*place)                             /* no white space */
            optarg = place;
        /* XXX: disable test for :: if PC? (GNU doesn't) */
        else if (oli[1] != ':')                 /* arg not optional */
        {
            if (++optind >= nargc)              /* no arg */
            {
                place  = EMSG;
                if (PRINT_ERROR)
                    xwarnx(recargchar, optchar);
                optopt = optchar;
                /* XXX: GNU returns '?' if options[0] != ':' */
                return BADARG;
            }
            else
                optarg = nargv[optind];
        }
        place = EMSG;
        ++optind;
    }
    /* dump back option letter */
    return optchar;
}

/*
 * getopt --
 *      Parse argc/argv argument vector.
 *
 * [eventually this will replace the real getopt]
 */
int getopt(int nargc, char *const nargv[], const char *options)
{
    int retval;

    progname = nargv[0];

    if ((retval = getopt_internal(nargc, nargv, options)) == -2)
    {
        ++optind;
        /*
         * We found an option (--), so if we skipped non-options,
         * we have to permute.
         */
        if (nonopt_end != -1)
        {
            permute_args(nonopt_start, nonopt_end, optind,
                         nargv);
            optind -= nonopt_end - nonopt_start;
        }
        nonopt_start = nonopt_end = -1;
        retval       = -1;
    }
    return retval;
}

/*
 * getopt_long --
 *      Parse argc/argv argument vector.
 */
int
getopt_long(int nargc,
            char *const *nargv,
            const char *options,
            const struct option *long_options,
            int *idx)
{
    int retval;

    _DIAGASSERT(nargv != NULL);
    _DIAGASSERT(options != NULL);
    _DIAGASSERT(long_options != NULL);
    /* idx may be NULL */

    progname = nargv[0];

    if ((retval = getopt_internal(nargc, nargv, options)) == -2)
    {
        char   *current_argv, *has_equal;
        size_t current_argv_len;
        int    i, match;

        current_argv = place;
        match        = -1;

        optind++;
        place        = EMSG;

        if (*current_argv == '\0')                      /* found "--" */
        {               /*
                         * We found an option (--), so if we skipped
                         * non-options, we have to permute.
                         */
            if (nonopt_end != -1)
            {
                permute_args(nonopt_start, nonopt_end,
                             optind, nargv);
                optind -= nonopt_end - nonopt_start;
            }
            nonopt_start = nonopt_end = -1;
            return -1;
        }
        if ((has_equal = strchr(current_argv, '=')) != NULL)
        {
            /* argument found (--option=arg) */
            current_argv_len = has_equal - current_argv;
            has_equal++;
        }
        else
            current_argv_len = strlen(current_argv);

        for (i = 0; long_options[i].name; i++)
        {
            /* find matching long option */
            if (strncmp(current_argv, long_options[i].name,
                        current_argv_len))
                continue;

            if (strlen(long_options[i].name) ==
                (unsigned)current_argv_len)
            {
                /* exact match */
                match = i;
                break;
            }
            if (match == -1)                            /* partial match */
                match = i;
            else
            {
                /* ambiguous abbreviation */
                if (PRINT_ERROR)
                    xwarnx(ambig, (int)current_argv_len,
                           current_argv);
                optopt = 0;
                return BADCH;
            }
        }
        if (match != -1)                                /* option found */
        {
            if (long_options[match].has_arg == no_argument
                && has_equal)
            {
                if (PRINT_ERROR)
                    xwarnx(noarg, (int)current_argv_len,
                           current_argv);
                /*
                 * XXX: GNU sets optopt to val regardless of
                 * flag
                 */
                if (long_options[match].flag == NULL)
                    optopt = long_options[match].val;
                else
                    optopt = 0;
                /* XXX: GNU returns '?' if options[0] != ':' */
                return BADARG;
            }
            if (long_options[match].has_arg == required_argument ||
                long_options[match].has_arg == optional_argument)
            {
                if (has_equal)
                    optarg = has_equal;
                else if (long_options[match].has_arg ==
                         required_argument)
                {
                    /*
                     * optional argument doesn't use
                     * next nargv
                     */
                    optarg = nargv[optind++];
                }
            }
            if ((long_options[match].has_arg == required_argument)
                && (optarg == NULL))
            {
                /*
                 * Missing argument; leading ':'
                 * indicates no error should be generated
                 */
                if (PRINT_ERROR)
                    xwarnx(recargstring, current_argv);
                /*
                 * XXX: GNU sets optopt to val regardless
                 * of flag
                 */
                if (long_options[match].flag == NULL)
                    optopt = long_options[match].val;
                else
                    optopt = 0;
                /* XXX: GNU returns '?' if options[0] != ':' */
                --optind;
                return BADARG;
            }
        }
        else                                    /* unknown option */
        {
            if (PRINT_ERROR)
                xwarnx(illoptstring, current_argv);
            optopt = 0;
            return BADCH;
        }
        if (long_options[match].flag)
        {
            *long_options[match].flag = long_options[match].val;
            retval                    = 0;
        }
        else
            retval = long_options[match].val;
        if (idx)
            *idx = match;
    }
    return retval;
}

int sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
    return 0;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
    return -1;
}

void syslog(int priority, const char *format, ...)
{}

void *memmem(const void *haystack_start, size_t haystack_len, const void *needle_start, size_t needle_len)
{
    const unsigned char *haystack = (const unsigned char *) haystack_start;
    const unsigned char *needle   = (const unsigned char *) needle_start;

    if (needle_len == 0)
        return (void *) haystack;

    while (needle_len <= haystack_len)
    {
        if (!memcmp (haystack, needle, needle_len))
            return (void *) haystack;

        haystack++;
        haystack_len--;
    }
    return NULL;
}

int getdtablesize(void)
{
    return 0;
}

pid_t setsid(void)
{
    return (pid_t)-1;
}

int mknod(const char *path, mode_t mode, dev_t dev)
{
    return -1;
}

int gethostname(char *name, size_t len)
{
    strncpy(name, "lwip", len);
    return 0;
}

int getnameinfo(const struct sockaddr *sa, socklen_t addrlen, char *host,
                socklen_t hostlen, char *serv, socklen_t servlen, unsigned int flags)
{
    return EAI_FAIL;
}

const char *gai_strerror(int ecode)
{
    switch (ecode)
    {
    case EAI_FAIL: return "A non-recoverable error occurred";

    //    case EAI_FAMILY : return "The address family was not recognized or the address length was invalid for the specified family";
    case EAI_NONAME: return "The name does not resolve for the supplied parameters";

    case 201: return "EAI_SERVICE";

    case 203: return "EAI_MEMORY";

    case 210: return "HOST_NOT_FOUND";

    case 211: return "NO_DATA";

    case 212: return "NO_RECOVERY";

    case 213: return "TRY_AGAIN";
    }

    return "Unknown error";
}

struct servent *getservbyport(int port, const char *proto)
{
    return NULL;
}

struct hostent *gethostbyaddr(const void *addr,
                              socklen_t len, int type)
{
    return NULL;
}

int fnmatch(const char *pattern, const char *string, int flags)
{
    register const char    *p = pattern, *n = string;
    register unsigned char c;

#define FOLD(c) ((flags & FNM_CASEFOLD) ? tolower (c) : (c))

    while ((c = *p++) != '\0')
    {
        c = FOLD (c);

        switch (c)
        {
        case '?':
            if (*n == '\0')
                return FNM_NOMATCH;
            else if ((flags & FNM_FILE_NAME) && *n == '/')
                return FNM_NOMATCH;
            else if ((flags & FNM_PERIOD) && *n == '.' &&
                     (n == string || ((flags & FNM_FILE_NAME) && n[-1] == '/')))
                return FNM_NOMATCH;
            break;

        case '\\':
            if (!(flags & FNM_NOESCAPE))
            {
                c = *p++;
                c = FOLD (c);
            }
            if (FOLD ((unsigned char)*n) != c)
                return FNM_NOMATCH;
            break;

        case '*':
            if ((flags & FNM_PERIOD) && *n == '.' &&
                (n == string || ((flags & FNM_FILE_NAME) && n[-1] == '/')))
                return FNM_NOMATCH;

            for (c = *p++; c == '?' || c == '*'; c = *p++, ++n)
                if (((flags & FNM_FILE_NAME) && *n == '/') ||
                    (c == '?' && *n == '\0'))
                    return FNM_NOMATCH;

            if (c == '\0')
                return 0;

            {
                unsigned char c1 = (!(flags & FNM_NOESCAPE) && c == '\\') ? *p : c;
                c1 = FOLD (c1);
                for (--p; *n != '\0'; ++n)
                    if ((c == '[' || FOLD ((unsigned char)*n) == c1) &&
                        fnmatch (p, n, flags & ~FNM_PERIOD) == 0)
                        return 0;
                return FNM_NOMATCH;
            }

        case '[':
            {
                /* Nonzero if the sense of the character class is inverted.  */
                register int not;

                if (*n == '\0')
                    return FNM_NOMATCH;

                if ((flags & FNM_PERIOD) && *n == '.' &&
                    (n == string || ((flags & FNM_FILE_NAME) && n[-1] == '/')))
                    return FNM_NOMATCH;

                not = (*p == '!' || *p == '^');
                if (not)
                    ++p;

                c = *p++;
                for (;;)
                {
                    register unsigned char cstart = c, cend = c;

                    if (!(flags & FNM_NOESCAPE) && c == '\\')
                        cstart = cend = *p++;

                    cstart = cend = FOLD (cstart);

                    if (c == '\0')
                        /* [ (unterminated) loses.  */
                        return FNM_NOMATCH;

                    c = *p++;
                    c = FOLD (c);

                    if ((flags & FNM_FILE_NAME) && c == '/')
                        /* [/] can never match.  */
                        return FNM_NOMATCH;

                    if (c == '-' && *p != ']')
                    {
                        cend = *p++;
                        if (!(flags & FNM_NOESCAPE) && cend == '\\')
                            cend = *p++;
                        if (cend == '\0')
                            return FNM_NOMATCH;
                        cend = FOLD (cend);

                        c    = *p++;
                    }

                    if (FOLD ((unsigned char)*n) >= cstart
                        && FOLD ((unsigned char)*n) <= cend)
                        goto matched;

                    if (c == ']')
                        break;
                }
                if (!not)
                    return FNM_NOMATCH;
                break;

matched:;
                /* Skip the rest of the [...] that already matched.  */
                while (c != ']')
                {
                    if (c == '\0')
                        /* [... (unterminated) loses.  */
                        return FNM_NOMATCH;

                    c = *p++;
                    if (!(flags & FNM_NOESCAPE) && c == '\\')
                        /* XXX 1003.2d11 is unclear if this is right.  */
                        ++p;
                }
                if (not)
                    return FNM_NOMATCH;
            }
            break;

        default:
            if (c != FOLD ((unsigned char)*n))
                return FNM_NOMATCH;
        }

        ++n;
    }

    if (*n == '\0')
        return 0;

    if ((flags & FNM_LEADING_DIR) && *n == '/')
        /* The FNM_LEADING_DIR flag says that "foo*" matches "foobar/frobozz".  */
        return 0;

    return FNM_NOMATCH;
}

int ffs(int word)
{
    int i;

    if (!word)
        return 0;

    i = 0;
    for (;;)
    {
        if (((1 << i++) & word) != 0)
            return i;
    }
}

struct passwd *getpwuid(uid_t uid)
{
    return NULL;
}

struct group *getgrgid(gid_t gid)
{
    return NULL;
}

struct passwd *getpwnam(const char *name)
{
    return NULL;
}

struct group *getgrnam(const char *name)
{
    return NULL;
}

struct _reent *__getreent(void)
{
    static struct _reent reent;
    return &reent;
}

int rand_r(unsigned int *seed)
{
    long k;
    long s = (long)(*seed);
    if (s == 0)
        s = 0x12345987;
    k       = s / 127773;
    s       = 16807 * (s - k * 127773) - 2836 * k;
    if (s < 0)
        s += 2147483647;
    (*seed) = (unsigned int)s;
    return (int)(s & RAND_MAX);
}

int setpriority(int which, id_t who, int value)
{
    return 0;
}

size_t itpMbstowcs(wchar_t *dest, const char *src, size_t max)
{
    int len = strlen(src) + 1;
    return MultiByteToWideChar(CP_UTF8, 0, src, len, dest, len) - 1;
}

int itpWctomb(char *dest, wchar_t src)
{
    return WideCharToMultiByte(CP_UTF8, 0, &src, 1, dest, 4, NULL, NULL);
}

int itpMbtowc(wchar_t *dest, const char *src, size_t max)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, src, max, dest, 1);
    return WideCharToMultiByte(CP_UTF8, 0, dest, 1, NULL, 0, NULL, NULL);
}

int setenv(const char *name, const char *value, int overwrite)
{
    int  len  = strlen(name) + 1 + strlen(value) + 1;
    char *str = malloc(len);
    if (str)
    {
        sprintf(str, "%s=%s", name, value);
        putenv(str);
        free(str);
        return 0;
    }
    else
        return -1;
}

int unsetenv(const char *name)
{
    int  len  = strlen(name) + 2;
    char *str = malloc(len);
    if (str)
    {
        sprintf(str, "%s=", name);
        putenv(str);
        free(str);
        return 0;
    }
    else
        return -1;
}

off_t ftello(FILE *fp)
{
    return (off_t)ftell(fp);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
    return -1;
}

int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result)
{
    return -1;
}

char *realpath(const char *path, char *resolved_path)
{
    return NULL;
}

int nftw(const char *dirpath,
         int (*fn)(const char *fpath, const struct stat *sb,
                   int typeflag, struct FTW *ftwbuf),
         int nopenfd, int flags)
{
    return -1;
}

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
    return NULL;
}

int munmap(void *addr, size_t len)
{
    return -1;
}

int ftruncate(int file, off_t length)
{
    return -1;
}

int mkstemp(char *path)
{
    return -1;
}

FILE *popen(const char *command, const char *type)
{
    return NULL;
}

int pclose(FILE *stream)
{
    return -1;
}

int setuid(uid_t uid)
{
    return -1;
}

int seteuid(uid_t uid)
{
    return 0;
}

int chroot(const char *path)
{
    return 0;
}

int setgid(gid_t gid)
{
    return 0;
}

int setegid(gid_t egid)
{
    return 0;
}

/*
 * We do not implement alternate representations. However, we always
 * check whether a given modifier is allowed for a certain conversion.
 */
#define ALT_E        0x01
#define ALT_O        0x02
//#define LEGAL_ALT(x)       { if (alt_format & ~(x)) return (0); }
#define LEGAL_ALT(x) {; }
#define TM_YEAR_BASE (1900)

static int conv_num(const char **, int *, int, int);

static const char *day[7] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
    "Friday", "Saturday"
};
static const char *abday[7] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
static const char *mon[12] = {
    "January", "February", "March", "April", "May", "June", "July",
    "August", "September", "October", "November", "December"
};
static const char *abmon[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static const char *am_pm[2] = {
    "AM", "PM"
};

char *strptime(const char *buf, const char *fmt, struct tm *tm)
{
    char       c;
    const char *bp;
    size_t     len = 0;
    int        alt_format, i, split_year = 0;

    bp = buf;

    while ((c = *fmt) != '\0')
    {
        /* Clear `alternate' modifier prior to new conversion. */
        alt_format = 0;

        /* Eat up white-space. */
        if (isspace(c))
        {
            while (isspace(*bp))
                bp++;

            fmt++;
            continue;
        }

        if ((c = *fmt++) != '%')
            goto literal;

again:        switch (c = *fmt++)
        {
        case '%':     /* "%%" is converted to "%". */
literal:
            if (c != *bp++)
                return (0);
            break;

        /*
         * "Alternative" modifiers. Just set the appropriate flag
         * and start over again.
         */
        case 'E':     /* "%E?" alternative conversion modifier. */
            LEGAL_ALT(0);
            alt_format |= ALT_E;
            goto again;

        case 'O':     /* "%O?" alternative conversion modifier. */
            LEGAL_ALT(0);
            alt_format |= ALT_O;
            goto again;

        /*
         * "Complex" conversion rules, implemented through recursion.
         */
        case 'c':     /* Date and time, using the locale's format. */
            LEGAL_ALT(ALT_E);
            if (!(bp = strptime(bp, "%x %X", tm)))
                return (0);
            break;

        case 'D':     /* The date as "%m/%d/%y". */
            LEGAL_ALT(0);
            if (!(bp = strptime(bp, "%m/%d/%y", tm)))
                return (0);
            break;

        case 'R':     /* The time as "%H:%M". */
            LEGAL_ALT(0);
            if (!(bp = strptime(bp, "%H:%M", tm)))
                return (0);
            break;

        case 'r':     /* The time in 12-hour clock representation. */
            LEGAL_ALT(0);
            if (!(bp = strptime(bp, "%I:%M:%S %p", tm)))
                return (0);
            break;

        case 'T':     /* The time as "%H:%M:%S". */
            LEGAL_ALT(0);
            if (!(bp = strptime(bp, "%H:%M:%S", tm)))
                return (0);
            break;

        case 'X':     /* The time, using the locale's format. */
            LEGAL_ALT(ALT_E);
            if (!(bp = strptime(bp, "%H:%M:%S", tm)))
                return (0);
            break;

        case 'x':     /* The date, using the locale's format. */
            LEGAL_ALT(ALT_E);
            if (!(bp = strptime(bp, "%m/%d/%y", tm)))
                return (0);
            break;

        /*
         * "Elementary" conversion rules.
         */
        case 'A':     /* The day of week, using the locale's form. */
        case 'a':
            LEGAL_ALT(0);
            for (i = 0; i < 7; i++)
            {
                /* Full name. */
                len = strlen(day[i]);
                if (strncasecmp((char *)(day[i]), (char *)bp, len) == 0)
                    break;

                /* Abbreviated name. */
                len = strlen(abday[i]);
                if (strncasecmp((char *)(abday[i]), (char *)bp, len) == 0)
                    break;
            }

            /* Nothing matched. */
            if (i == 7)
                return (0);

            tm->tm_wday = i;
            bp         += len;
            break;

        case 'B':     /* The month, using the locale's form. */
        case 'b':
        case 'h':
            LEGAL_ALT(0);
            for (i = 0; i < 12; i++)
            {
                /* Full name. */

                len = strlen(mon[i]);
                if (strncasecmp((char *)(mon[i]), (char *)bp, len) == 0)
                    break;

                /* Abbreviated name. */
                len = strlen(abmon[i]);
                if (strncasecmp((char *)(abmon[i]), (char *) bp, len) == 0)
                    break;
            }

            /* Nothing matched. */
            if (i == 12)
                return (0);

            tm->tm_mon = i;
            bp        += len;
            break;

        case 'C':     /* The century number. */
            LEGAL_ALT(ALT_E);
            if (!(conv_num(&bp, &i, 0, 99)))
                return (0);

            if (split_year)
            {
                tm->tm_year = (tm->tm_year % 100) + (i * 100);
            }
            else
            {
                tm->tm_year = i * 100;
                split_year  = 1;
            }
            break;

        case 'd':     /* The day of month. */
        case 'e':
            LEGAL_ALT(ALT_O);
            if (!(conv_num(&bp, &tm->tm_mday, 1, 31)))
                return (0);
            break;

        case 'k':     /* The hour (24-hour clock representation). */
            LEGAL_ALT(0);

        /* FALLTHROUGH */
        case 'H':
            LEGAL_ALT(ALT_O);
            if (!(conv_num(&bp, &tm->tm_hour, 0, 23)))
                return (0);
            break;

        case 'l':     /* The hour (12-hour clock representation). */
            LEGAL_ALT(0);

        /* FALLTHROUGH */
        case 'I':
            LEGAL_ALT(ALT_O);
            if (!(conv_num(&bp, &tm->tm_hour, 1, 12)))
                return (0);
            if (tm->tm_hour == 12)
                tm->tm_hour = 0;
            break;

        case 'j':     /* The day of year. */
            LEGAL_ALT(0);
            if (!(conv_num(&bp, &i, 1, 366)))
                return (0);
            tm->tm_yday = i - 1;
            break;

        case 'M':     /* The minute. */
            LEGAL_ALT(ALT_O);
            if (!(conv_num(&bp, &tm->tm_min, 0, 59)))
                return (0);
            break;

        case 'm':     /* The month. */
            LEGAL_ALT(ALT_O);
            if (!(conv_num(&bp, &i, 1, 12)))
                return (0);
            tm->tm_mon = i - 1;
            break;

        //            case 'p': /* The locale's equivalent of AM/PM. */
        //                LEGAL_ALT(0);
        //                /* AM? */
        //                if (strcasecmp(am_pm[0], bp) == 0)
        //                {
        //                    if (tm->tm_hour > 11)
        //                        return (0);
        //
        //                    bp += strlen(am_pm[0]);
        //                    break;
        //                }
        //                /* PM? */
        //                else if (strcasecmp(am_pm[1], bp) == 0)
        //                {
        //                    if (tm->tm_hour > 11)
        //                        return (0);
        //
        //                    tm->tm_hour += 12;
        //                    bp += strlen(am_pm[1]);
        //                    break;
        //                }
        //
        //                /* Nothing matched. */
        //                return (0);

        case 'S':     /* The seconds. */
            LEGAL_ALT(ALT_O);
            if (!(conv_num(&bp, &tm->tm_sec, 0, 61)))
                return (0);
            break;

        case 'U':     /* The week of year, beginning on sunday. */
        case 'W':     /* The week of year, beginning on monday. */
            LEGAL_ALT(ALT_O);
            /*
             * XXX This is bogus, as we can not assume any valid
             * information present in the tm structure at this
             * point to calculate a real value, so just check the
             * range for now.
             */
            if (!(conv_num(&bp, &i, 0, 53)))
                return (0);
            break;

        case 'w':     /* The day of week, beginning on sunday. */
            LEGAL_ALT(ALT_O);
            if (!(conv_num(&bp, &tm->tm_wday, 0, 6)))
                return (0);
            break;

        case 'Y':     /* The year. */
            LEGAL_ALT(ALT_E);
            if (!(conv_num(&bp, &i, 0, 9999)))
                return (0);

            tm->tm_year = i - TM_YEAR_BASE;
            break;

        case 'y':     /* The year within 100 years of the epoch. */
            LEGAL_ALT(ALT_E | ALT_O);
            if (!(conv_num(&bp, &i, 0, 99)))
                return (0);

            if (split_year)
            {
                tm->tm_year = ((tm->tm_year / 100) * 100) + i;
                break;
            }
            split_year = 1;
            if (i <= 68)
                tm->tm_year = i + 2000 - TM_YEAR_BASE;
            else
                tm->tm_year = i + 1900 - TM_YEAR_BASE;
            break;

        /*
         * Miscellaneous conversions.
         */
        case 'n':     /* Any kind of white-space. */
        case 't':
            LEGAL_ALT(0);
            while (isspace(*bp))
                bp++;
            break;

        default:     /* Unknown/unsupported conversion. */
            return (0);
        }
    }

    /* LINTED functional specification */
    return ((char *)bp);
}

static int conv_num(const char **buf, int *dest, int llim, int ulim)
{
    int result = 0;

    /* The limit also determines the number of valid digits. */
    int rulim  = ulim;

    if (**buf < '0' || **buf > '9')
        return (0);

    do
    {
        result *= 10;
        result += *(*buf)++ - '0';
        rulim  /= 10;
    } while ((result * 10 <= ulim) && rulim && **buf >= '0' && **buf <= '9');

    if (result < llim || result > ulim)
        return (0);

    *dest = result;
    return (1);
}

int fsync(int fd)
{
    return 0;
}

int fdatasync(int fd)
{
    return 0;
}

int fchmod(int fildes, mode_t mode)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return -1;
}

int fchown(int fd, uid_t owner, gid_t group)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return -1;    
}

int utimes(const char *filename, const struct timeval times[2])
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return -1;    
}
