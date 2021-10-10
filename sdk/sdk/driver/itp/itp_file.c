/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL File functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <stdio.h>
#include "itp_cfg.h"

static int openFiles[OPEN_MAX];

static int FileOpen(const char* name, int flags, int mode, void* info)
{
    int file, i;
    
    file = _open(name, flags, mode);
    if (file != -1)
    {
        // find empty slot
        for (i = 0; i < OPEN_MAX; i++)
        {
            if (openFiles[i] == 0)
            {
                openFiles[i] = file;
                return i;
            }
        }
    }
    errno = (ITP_DEVICE_FILE << ITP_DEVICE_ERRNO_BIT) | __LINE__;
    return -1;
}

static int FileClose(int file, void* info)
{
    int ret = _close(openFiles[file]);
    openFiles[file] = 0;
    if (ret)
    {
        errno = (ITP_DEVICE_FILE << ITP_DEVICE_ERRNO_BIT) | ret;
        return -1;
    }
    return 0;
}

static int FileRead(int file, char *ptr, int len, void* info)
{
    return _read(openFiles[file], ptr, len);
}

static int FileWrite(int file, char *ptr, int len, void* info)
{
    return _write(openFiles[file], ptr, len);
}

static int FileLseek(int file, int ptr, int dir, void* info)
{
    int ret = _lseek(openFiles[file], ptr, dir);
    if (ret == -1)
    {
        errno = (ITP_DEVICE_FILE << ITP_DEVICE_ERRNO_BIT) | ret;
        return -1;
    }
    return ret;
}


static int FileStat(const char *path, struct stat *sbuf)
{
    struct _stat buff;
    int ret = _stat(path, &buff);

    if (ret == 0)
    {
        sbuf->st_dev = buff.st_dev;
        sbuf->st_ino = buff.st_ino;
        sbuf->st_mode = buff.st_mode;
        sbuf->st_nlink = buff.st_nlink;
        sbuf->st_uid = buff.st_uid;
        sbuf->st_gid = buff.st_gid;
        sbuf->st_rdev = buff.st_rdev;
        sbuf->st_size = buff.st_size;
        sbuf->st_atime = buff.st_atime;
        sbuf->st_mtime = buff.st_mtime;
        sbuf->st_ctime = buff.st_ctime;
        sbuf->st_blksize = 2048;
        sbuf->st_blocks = 0;
    }
    return ret;
}


static int FileFstat(int file, struct stat *st)
{
    struct _stat buff;
    int ret = _fstat(openFiles[file], &buff);
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

const ITPFSDevice itpFSDeviceFile =
{
    {
        ":file",
        FileOpen,
        FileClose,
        FileRead,
        FileWrite,
        FileLseek,
        itpIoctlDefault,
        NULL
    },
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    FileStat,
    NULL,
    FileFstat,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
