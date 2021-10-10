/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Posix OpenRTOS functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/uio.h>
#include <sys/utime.h>
#include <sys/utsname.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <execinfo.h>
#include <fnmatch.h>
#include <ftw.h>
#include <iconv.h>
#include <limits.h>
#include <malloc.h>
#include <mqueue.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/semphr.h"
#include "openrtos/task.h"
#include "openrtos/timers.h"
#include "itp_cfg.h"

/**
 * Pthread mutex handle implementation definition.
 */
typedef struct
{
    void* xMutex;   ///< Native mutex.
    int type;       ///< Pthread mutex type.
} itpMutex;

/**
 * Pthread key handle implementation definition.
 */
typedef struct
{
    bool used;              ///< Indicates used or not.
    void (*dtor)(void*);    ///< Destructor callback function.
    void* value;            ///< Key value.
} itpThreadKey;

/**
 * PThread handle implementation definition.
 */
typedef struct
{
    void* xHandle;                          ///< Native thread handle.
    void* xSemaphore;                       ///< Used for joinable thread.
    void* (*start_routine)(void*);          ///< Thread function.
    void* arg;                              ///< Thread function argument.
    void* value_ptr;                        ///< Result value of thread function.
    int detachstate;                        ///< Pthread detach state.
    itpThreadKey keys[PTHREAD_KEYS_MAX];    ///< Thread keys.
    int volume;                             ///< Current disk volume.
} itpThread;

/**
 * PThread condition handle implementation definition.
 */
typedef struct
{
    void* xSemaphore;   ///< Native semaphore.
    int count;          ///< condition count.
} itpCondition;

/**
 * POSIX message queue handle implementation definition.
 */
typedef struct
{
    void* xQueue;               ///< Native message queue.
    unsigned long uxItemSize;   ///< Queue item size.
    int oflag;                  ///< Open flags.
} itpQueue;

int _kill(int pid, int sig)
{
    pid = pid; sig = sig; /* avoid warnings */
    errno = EINVAL;
    return -1;
}

void __libc_fini_array(void);
void  Rmalloc_stat(const char *file);

void _exit(int status)
{
    static bool exiting = false;
    if (exiting)
        while (1);

    exiting = true;

    #if (CFG_CHIP_FAMILY == 9850) && defined(CFG_CHIP_REV_A0) && defined(CFG_NAND_ENABLE)
    //for workaround the 9850's SPI engine issue if SPI NAND rebooting
    ithWriteRegA(ITH_SSP0_BASE + 0x74, 0);
    #endif

    if (status != 0)
    {
        // backtrace
        itpPrintBacktrace();

        // memory debug
        #ifdef CFG_MEMDBG_ENABLE
        {
            int32_t dbgFlag0 = ithMemDbgGetFlag(ITH_MEMDBG0);
            int32_t dbgFlag1 = ithMemDbgGetFlag(ITH_MEMDBG1);

            if (dbgFlag0 >= 0)
                ithPrintf("[ERROR] Memory access detect on region 0 by reguest# %d\n", dbgFlag0);

            if (dbgFlag1 >= 0)
                ithPrintf("[ERROR] Memory access detect on region 1 by request# %d\n", dbgFlag1);
        }
        #endif // CFG_MEMDBG_ENABLE

        // clock
        ithClockStats();

        // cmdq
    #ifdef CFG_CMDQ_ENABLE
        ithCmdQStats();
    #endif

        // memory usage
        malloc_stats();

        // task list
    #if (configUSE_TRACE_FACILITY == 1)
        {
            static signed char buf[2048];
            vTaskList(buf);
            ithPrintf(buf);
        }
    #endif

    #ifdef CFG_DBG_RMALLOC
        Rmalloc_stat(__FILE__);
    #endif
    }
    else
    {
        __libc_fini_array();
    }

#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
    ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif

    ithPrintf("exit(%d), reboot...\n", status);
#ifdef CFG_WATCHDOG_INTR
    ithWatchDogCtrlDisable(ITH_WD_INTR);
    ithIntrDisableIrq(ITH_INTR_WD);
#endif // CFG_WATCHDOG_INTR
    ithWatchDogCtrlEnable(ITH_WD_RESET);
    ithWatchDogEnable();
    ithWatchDogSetReload(0);
    ithWatchDogRestart();

    vTaskEndScheduler();
    while (1);
}

int _getpid(void)
{
    return (int) xTaskGetCurrentTaskHandle();
}

extern caddr_t __heap_start__;
extern caddr_t __heap_end__;
static void* heap_ptr = NULL;

caddr_t _sbrk(int incr)
{
    void* base;

    if (heap_ptr == NULL) {
        heap_ptr = (void*) & __heap_start__;
    }

    if ((void*)(heap_ptr + incr) <= (void*)(&__heap_end__)) {
        base = heap_ptr;
        heap_ptr += incr;
        return (void*)(base);

    } else {
    #if defined(CFG_BUILD_DEBUG) || defined(CFG_BUILD_DEBUGREL)
        #define BACKTRACE_SIZE 100
        static void *btbuf[BACKTRACE_SIZE];
        int i, btcount = backtrace(btbuf, BACKTRACE_SIZE);

    #endif // defined(CFG_BUILD_DEBUG) || defined(CFG_BUILD_DEBUGREL)

        ithPrintf("Out of memory: start=0x%X end=0x%X incr=0x%X ptr=0x%X\n", &__heap_start__, &__heap_end__, incr, heap_ptr);
        malloc_stats();

    #ifdef CFG_DBG_MEMLEAK
        dbg_heap_dump("");
    #endif

    #ifdef CFG_DBG_RMALLOC
        Rmalloc_stat(__FILE__);
    #endif

    #if defined(CFG_BUILD_DEBUG) || defined(CFG_BUILD_DEBUGREL)
        // backtrace
        for (i = 0; i < btcount; i++)
            ithPrintf("0x%X ", btbuf[i]);

        ithPrintf("\n");
    #endif // defined(CFG_BUILD_DEBUG) || defined(CFG_BUILD_DEBUGREL)

        errno = ENOMEM;
        return (void*)(-1);
    }
}

int _open(const char* name, int flags, int mode)
{
    int i, subfile;

    if (!name)
    {
        errno = EFAULT;
        return -1;
    }

    if (name[0] == ':')
    {
        char* subname = strchr(&name[1], ':');
        int len = subname ? subname - name : strlen(name);

        // try to open custom device
        for (i = ITP_DEVICE_CUSTOM >> ITP_DEVICE_BIT; i < ITP_DEVICE_MAX; i++)
        {
            const ITPDevice* dev = itpDeviceTable[i];

            if (dev && strncmp(name, dev->name, len) == 0)
            {
                subfile = dev->open(subname + 1, flags, mode, dev->info);
                if (subfile == -1)
                    return -1;

                return (i << ITP_DEVICE_BIT) | subfile;
            }
        }
    }
#if defined(CFG_FS_FAT) || defined(CFG_FS_NTFS)
    else
    {
        // try to open file system
        itpThread* t = (itpThread*) xTaskGetApplicationTaskTag(xTaskGetCurrentTaskHandle());
        ITPDriveStatus* driveStatusTable;
        ITPDriveStatus* driveStatus;

        ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

        if (name[1] == ':')
            driveStatus = &driveStatusTable[toupper(name[0]) - 'A'];
        else
            driveStatus = &driveStatusTable[t->volume];

        if (driveStatus->avail)
        {
            const ITPDevice* dev = (ITPDevice*) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
            if (dev)
            {
                subfile = dev->open(name, flags, mode, dev->info);
                if (subfile != -1)
                    return driveStatus->device | subfile;
                else
                    return -1;
            }
        }
    }
#endif // defined(CFG_FS_FAT) || defined(CFG_FS_NTFS)
    errno = ENOENT;
    return -1;
}

int _close(int file)
{
    const ITPDevice* dev;

    if (file < 0)
    {
        LOG_ERR "_close(0x%X) invalid handle\n", file LOG_END
        errno = ENOENT;
        return -1;
    }

    if (file <= STDERR_FILENO)
        return 0;

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

int _read(int file, char *ptr, int len)
{
    const ITPDevice* dev;

    if (file < 0)
    {
        LOG_ERR "_read(0x%X) invalid handle\n", file LOG_END
        errno = ENOENT;
        return -1;
    }

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

int itpRead(int file, char *ptr, int len)
{
    const ITPDevice* dev;

    if (file < 0)
    {
        LOG_ERR "itpRead(0x%X) invalid handle\n", file LOG_END
        errno = ENOENT;
        return -1;
    }

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

int _write(int file, char *ptr, int len)
{
    const ITPDevice* dev;

    if (file < 0)
    {
        LOG_ERR "_write(0x%X) invalid handle\n", file LOG_END
        errno = ENOENT;
        return -1;
    }

    dev = itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (dev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return dev->write(file & ITP_FILE_MASK, (char*)ptr, len, dev->info);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

int itpWrite(int file, char *ptr, int len)
{
    const ITPDevice* dev;

    if (file < 0)
    {
        LOG_ERR "itpWrite(0x%X) invalid handle\n", file LOG_END
        errno = ENOENT;
        return -1;
    }

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

int _lseek(int file, int ptr, int dir)
{
    const ITPDevice* dev;

    if (file < 0)
    {
        LOG_ERR "_lseek(0x%X) invalid handle\n", file LOG_END
        errno = ENOENT;
        return -1;
    }

    dev = itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

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

int ioctl(int file, unsigned long request, void* ptr)
{
    const ITPDevice* dev;

    if (file < 0)
    {
        LOG_ERR "ioctl(0x%X) invalid handle\n", file LOG_END
        errno = ENOENT;
        return -1;
    }

    if ((file & ITP_DEVICE_MASK) == 0)
        dev = itpDeviceTable[ITP_DEVICE_SOCKET >> ITP_DEVICE_BIT];
    else
        dev = itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (dev)
    {
        /*  Pass call on to device driver.  Return result.      */
        return dev->ioctl(file & ITP_FILE_MASK, request, ptr, dev->info);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

int _unlink(const char *path)
{
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus;
    int ret = -1;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    if (path[1] == ':')
        driveStatus = &driveStatusTable[toupper(path[0]) - 'A'];
    else
    {
        itpThread* t = (itpThread*) xTaskGetApplicationTaskTag(xTaskGetCurrentTaskHandle());
        driveStatus = &driveStatusTable[t->volume];
    }

    if (driveStatus->avail)
    {
        const ITPFSDevice* fsdev = (ITPFSDevice*) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
        if (fsdev)
        {
            ret = fsdev->remove(path);
        }
        else
        {
            errno = ENOENT;
            return -1;
        }
    }
    return ret;
}

int _rename(const char *oldname, const char *newname)
{
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus;
    int ret = -1;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    if (oldname[1] == ':')
        driveStatus = &driveStatusTable[toupper(oldname[0]) - 'A'];
    else
    {
        itpThread* t = (itpThread*) xTaskGetApplicationTaskTag(xTaskGetCurrentTaskHandle());
        driveStatus = &driveStatusTable[t->volume];
    }

    if (driveStatus->avail)
    {
        const ITPFSDevice* dev = (ITPFSDevice*) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
        if (dev)
            ret = dev->rename(oldname, newname);
        else
        {
            errno = ENOENT;
            return -1;
        }
    }
    return ret;
}

int chdir(const char *path)
{
    itpThread* t = (itpThread*) xTaskGetApplicationTaskTag(xTaskGetCurrentTaskHandle());
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus;
    int ret = -1;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    if (path[1] == ':')
        driveStatus = &driveStatusTable[toupper(path[0]) - 'A'];
    else
        driveStatus = &driveStatusTable[t->volume];

    if (driveStatus->avail)
    {
        const ITPFSDevice* dev = (ITPFSDevice*) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
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
            t->volume = toupper(path[0]) - 'A';
    }
    return ret;
}

int chmod(const char *path, mode_t mode)
{
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus;
    int ret = -1;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    if (path[1] == ':')
        driveStatus = &driveStatusTable[toupper(path[0]) - 'A'];
    else
    {
        itpThread* t = (itpThread*) xTaskGetApplicationTaskTag(xTaskGetCurrentTaskHandle());
        driveStatus = &driveStatusTable[t->volume];
    }

    if (driveStatus->avail)
    {
        const ITPFSDevice* dev = (ITPFSDevice*) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
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
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus;
    int ret = -1;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    if (path[1] == ':')
        driveStatus = &driveStatusTable[toupper(path[0]) - 'A'];
    else
    {
        itpThread* t = (itpThread*) xTaskGetApplicationTaskTag(xTaskGetCurrentTaskHandle());
        driveStatus = &driveStatusTable[t->volume];
    }

    if (driveStatus->avail)
    {
        const ITPFSDevice* dev = (ITPFSDevice*) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
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

int _stat(const char *path, struct stat *buf)
{
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus;
    int ret = -1;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    if (path[1] == ':')
        driveStatus = &driveStatusTable[toupper(path[0]) - 'A'];
    else
    {
        itpThread* t = (itpThread*) xTaskGetApplicationTaskTag(xTaskGetCurrentTaskHandle());
        driveStatus = &driveStatusTable[t->volume];
    }

    if (driveStatus->avail)
    {
        const ITPFSDevice* dev = (ITPFSDevice*) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
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
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus;
    int ret = -1;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    if (path[1] == ':')
        driveStatus = &driveStatusTable[toupper(path[0]) - 'A'];
    else
    {
        itpThread* t = (itpThread*) xTaskGetApplicationTaskTag(xTaskGetCurrentTaskHandle());
        driveStatus = &driveStatusTable[t->volume];
    }

    if (driveStatus->avail)
    {
        const ITPFSDevice* dev = (ITPFSDevice*) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
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

    return ret;
}

int _fstat(int file, struct stat *st)
{
    if ((STDOUT_FILENO == file) || (STDERR_FILENO == file) || (STDIN_FILENO == file))
    {
      st->st_mode = S_IFCHR;
      return  0;
    }
    else
    {
        const ITPFSDevice* dev = (ITPFSDevice*) itpDeviceTable[(file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

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

char* getcwd(char *buf, size_t size)
{
    itpThread* t = (itpThread*) xTaskGetApplicationTaskTag(xTaskGetCurrentTaskHandle());
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus;
    char* dir = NULL;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);
    driveStatus = &driveStatusTable[t->volume];

    if (driveStatus->avail)
    {
        const ITPFSDevice* dev = (ITPFSDevice*) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
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
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus;
    int ret = -1;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    if (path[1] == ':')
        driveStatus = &driveStatusTable[toupper(path[0]) - 'A'];
    else
    {
        itpThread* t = (itpThread*) xTaskGetApplicationTaskTag(xTaskGetCurrentTaskHandle());
        driveStatus = &driveStatusTable[t->volume];
    }

    if (driveStatus->avail)
    {
        const ITPFSDevice* dev = (ITPFSDevice*) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
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

int closedir(DIR *dirp)
{
    const ITPFSDevice* dev = (ITPFSDevice*)itpDeviceTable[(dirp->dd_fd & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];
    if (dev)
        return dev->closedir(dirp);
    else
    {
        errno = ENOENT;
        return -1;
    }
}

DIR* opendir(const char *dirname)
{
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus;
    DIR* dir = NULL;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    if (dirname[1] == ':')
        driveStatus = &driveStatusTable[toupper(dirname[0]) - 'A'];
    else
    {
        itpThread* t = (itpThread*) xTaskGetApplicationTaskTag(xTaskGetCurrentTaskHandle());
        driveStatus = &driveStatusTable[t->volume];
    }

    if (driveStatus->avail)
    {
        const ITPFSDevice* dev = (ITPFSDevice*) itpDeviceTable[driveStatus->device >> ITP_DEVICE_BIT];
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

struct dirent* readdir(DIR *dirp)
{
    const ITPFSDevice* dev = (ITPFSDevice*)itpDeviceTable[(dirp->dd_fd & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];
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
    const ITPFSDevice* dev = (ITPFSDevice*)itpDeviceTable[(dirp->dd_fd & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];
    if (dev)
        dev->rewinddir(dirp);
}

int scandir(const char *dirname,
   struct dirent *** namelist,
   int (*select)(struct dirent *),
   int (*dcomp)(const struct dirent **, const struct dirent **))
{
    register struct dirent *d, *p, **names;
    register size_t nitems;
    long arraysz;
    DIR *dirp;
    int successful = 0;
    int rc = 0;

    dirp = NULL;
    names = NULL;

    if ((dirp = opendir(dirname)) == NULL)
        return(-1);

    arraysz = 1;
    names = (struct dirent **)malloc(arraysz * sizeof(struct dirent *));
    if (names == NULL)
        goto cleanup;

    nitems = 0;
    while ((d = readdir(dirp)) != NULL) {
        if (select != NULL && !(*select)(d))
            continue;   /* just selected names */
        /*
         * Make a minimum size copy of the data
         */
        p = (struct dirent *)malloc(sizeof (struct dirent));
        if (p == NULL)
            goto cleanup;
        p->d_ino = d->d_ino;
        p->d_type = d->d_type;
        p->d_namlen = d->d_namlen;
        strcpy(p->d_name, d->d_name);

        /*
         * Check to make sure the array has space left and
         * realloc the maximum size.
         */
        if (++nitems >= (size_t)arraysz) {
            arraysz = nitems;
            names = (struct dirent **)realloc((char *)names,
                arraysz * sizeof(struct dirent *));
            if (names == NULL)
                goto cleanup;
        }
        names[nitems-1] = p;
    }
    successful = 1;
cleanup:
    closedir(dirp);
    if (successful) {
        if (nitems && dcomp != NULL)
            qsort(names, nitems, sizeof(struct dirent *), (void *)dcomp);
        *namelist = names;
        rc = nitems;
    } else {  /* We were unsuccessful, clean up storage and return -1.  */
        if ( names ) {
            size_t i;
            for (i=0; i < nitems; i++ )
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

mqd_t mq_open(const char *name, int oflag, ...)
{
    if (oflag & O_CREAT)
    {
        mode_t mode;
        struct mq_attr* attr;
        va_list ap;
        itpQueue* q = pvPortMalloc(sizeof (itpQueue));
        if (!q)
        {
            errno = ENOMEM;
            return -1;
        }

        va_start(ap, oflag);
        mode = va_arg(ap, mode_t);
        attr = va_arg(ap, struct mq_attr*);
        va_end(ap);

        q->xQueue = xQueueCreate(attr->mq_maxmsg, attr->mq_msgsize);
        if (!q->xQueue)
        {
            vPortFree(q);
            errno = ENOMEM;
            return -1;
        }

        q->uxItemSize   = attr->mq_msgsize;
        q->oflag        = oflag | attr->mq_flags;

        return (mqd_t)q;
    }
    else
    {
        errno = EINVAL;
        return -1;
    }
}

int _times(struct tms *tp)
{
    /* Return a clock that ticks at 100Hz.  */
    clock_t timeval = (clock_t) xTaskGetTickCount() / configTICK_RATE_HZ * CLOCKS_PER_SEC;

    if (tp)
    {
        tp->tms_utime  = timeval;   /* user time */
        tp->tms_stime  = 0;         /* system time */
        tp->tms_cutime = 0;         /* user time, children */
        tp->tms_cstime = 0;         /* system time, children */
    }
    return timeval;
}

int _gettimeofday(struct timeval *tp, void *tzvp)
{
    struct timezone *tzp = tzvp;
    if (tp)
    {
    #ifdef CFG_RTC_ENABLE
        ioctl(ITP_DEVICE_RTC, ITP_IOCTL_GET_TIME, (void*)tp);
    #else
        unsigned long ms;

        if (ithGetCpuMode() == ITH_CPU_SYS)
            ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
        else
            ms = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;

        tp->tv_usec = (ms % 1000) * 1000;
        tp->tv_sec  = CFG_RTC_DEFAULT_TIMESTAMP + ms / 1000;
    #endif // CFG_RTC_ENABLE
    }

    if (tzp)
    {
        tzp->tz_minuteswest = _timezone / 60;
        tzp->tz_dsttime     = _daylight;        
    }
    return 0;
}

int settimeofday(const struct timeval *tv , const struct timezone *tz)
{
    struct timeval t;
        
    if (tv == NULL)
    {
        errno = EINVAL;
        return -1;
    }
#ifdef CFG_RTC_ENABLE
    memcpy(&t, tv, sizeof (struct timeval));
    if (tz)
    {
        t.tv_sec += tz->tz_minuteswest * 60;
    }
    else
    {
        t.tv_sec += _timezone;
    }
    ioctl(ITP_DEVICE_RTC, ITP_IOCTL_SET_TIME, (void*)&t);    
#endif
    return 0;
}

int usleep(useconds_t useconds)
{
    TickType_t t = (TickType_t)((uint64_t) useconds / (1000 * portTICK_PERIOD_MS));
    useconds_t r = useconds - ((useconds_t)t * (1000 * portTICK_PERIOD_MS));

    if (t > 0)
        vTaskDelay(t);

    if (r > 0)
        ithDelay(r);

    return 0;
}

unsigned sleep(unsigned seconds)
{
    if (seconds > 0)
        usleep((uint64_t) seconds * 1000000);
    else
        taskYIELD();

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

int pthread_attr_init(pthread_attr_t* attr)
{
    attr->detachstate               = PTHREAD_CREATE_JOINABLE;
    attr->schedpolicy               = SCHED_OTHER;
    attr->schedparam.sched_priority = tskIDLE_PRIORITY + 1;
    attr->inheritsched              = PTHREAD_EXPLICIT_SCHED;
    attr->contentionscope           = PTHREAD_SCOPE_SYSTEM;
    attr->stackaddr                 = NULL;
    attr->stacksize                 = configMINIMAL_STACK_SIZE * sizeof(portSTACK_TYPE);
    attr->is_initialized            = 1;
    return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
    attr->is_initialized = 0;
    return 0;
}

int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param)
{
    memcpy(param, &attr->schedparam, sizeof (struct sched_param));
    return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t* attr, int detachstate)
{
    attr->detachstate = detachstate;
    return 0;
}

int pthread_getschedparam(pthread_t pthread, int* policy, struct sched_param* param)
{
    param->sched_priority = uxTaskPriorityGet((TaskHandle_t) pthread);
    return 0;
}

int pthread_attr_setschedparam(pthread_attr_t *attr,
                               const struct sched_param *param)
{
    attr->schedparam.sched_priority = param->sched_priority;
    return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
    if (stacksize < configMINIMAL_STACK_SIZE * sizeof(portSTACK_TYPE))
    {
        LOG_DBG "pthread_attr_setstacksize(0x%X,%d) invalid\n", attr, stacksize LOG_END
        return EINVAL;
    }

    if (stacksize > USHRT_MAX * sizeof(portSTACK_TYPE))
    {
        LOG_ERR "pthread_attr_setstacksize(0x%X,%d) invalid\n", attr, stacksize LOG_END
        return EINVAL;
    }

    attr->stacksize = stacksize;
    return 0;
}

int pthread_attr_setscope(pthread_attr_t *attr, int contentionscope)
{
    attr->contentionscope = contentionscope;
    return 0;
}

static void vTaskCode(void* pvParameters)
{
    itpThread* t = (itpThread*) pvParameters;
    TaskHandle_t xHandle = xTaskGetCurrentTaskHandle();

    t->xHandle = xHandle;
    vTaskSetApplicationTaskTag(xHandle, (TaskHookFunction_t) t);

#ifdef CFG_FS_FAT
    // init fat on this task
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_ENABLE, NULL);
#endif

    t->value_ptr = t->start_routine(t->arg);

#ifdef CFG_FS_FAT
    // exit fat on this task
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_DISABLE, NULL);
#endif

    if (t->detachstate == PTHREAD_CREATE_JOINABLE)
    {
        LOG_DBG "pthread 0x%X exit, wait to be joined\n", t LOG_END
        xSemaphoreGive(t->xSemaphore);
    }
    else
    {
        int i;

        for (i = 0; i < PTHREAD_KEYS_MAX; i++)
        {
            itpThreadKey* k = &t->keys[i];
            if (k->dtor && k->value)
                k->dtor(k->value);
        }

        vTaskPrioritySet(xTaskGetIdleTaskHandle(), uxTaskPriorityGet(xHandle));
        vPortFree(t);
        vTaskDelete(xHandle);
    }
    while (1)
        vTaskDelay(portMAX_DELAY);
}

int itpPthreadCreate(pthread_t* pthread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg, const char* name)
{
    portBASE_TYPE ret;
    TaskHandle_t handle;
    pthread_attr_t attr_default;
    unsigned short depth;
    itpThread* t = pvPortMalloc(sizeof(itpThread));
    if (t == NULL)
    {
        LOG_ERR "itpPthreadCreate(%s) nomem\n", name LOG_END
        return ENOMEM;
    }

    memset(t, 0, sizeof (itpThread));

    if (!attr)
    {
        pthread_attr_init(&attr_default);
        attr = &attr_default;
    }

    t->start_routine    = start_routine;
    t->arg              = arg;
    t->detachstate      = attr->detachstate;

    if (t->detachstate == PTHREAD_CREATE_JOINABLE)
    {
        t->xSemaphore = xSemaphoreCreateCounting(1, 0);
        if (t->xSemaphore == NULL)
        {
            vPortFree(t);
            LOG_ERR "itpPthreadCreate(%s) fail\n", name LOG_END
            return ENOMEM;
        }
    }
    else
        t->xSemaphore = NULL;

    depth = (attr->stacksize / sizeof(portSTACK_TYPE) < USHRT_MAX) ? attr->stacksize / sizeof(portSTACK_TYPE) : USHRT_MAX;
    ret = xTaskCreate(vTaskCode, name, depth, t, attr->schedparam.sched_priority, &handle);
    if (ret != pdPASS)
    {
        if (t->xSemaphore)
            vSemaphoreDelete(t->xSemaphore);

        vPortFree(t);
        LOG_ERR "itpPthreadCreate(%s) fail\n", name LOG_END
        return EAGAIN;
    }

    portENTER_CRITICAL();
    t->xHandle = handle;
    vTaskSetApplicationTaskTag(handle, (TaskHookFunction_t) t);
    *pthread = (pthread_t) handle;
    portEXIT_CRITICAL();

    return 0;
}

int pthread_cancel(pthread_t pthread)
{
    itpThread* t = (itpThread*) xTaskGetApplicationTaskTag((TaskHandle_t) pthread);
    int i;
    TaskHandle_t xHandle;

    for (i = 0; i < PTHREAD_KEYS_MAX; i++)
    {
        itpThreadKey* k = &t->keys[i];
        if (k->dtor && k->value)
            k->dtor(k->value);
    }

    xHandle = xTaskGetIdleTaskHandle();
    vTaskPrioritySet(xHandle, tskIDLE_PRIORITY + 1);

    xHandle = t->xHandle;
    vPortFree(t);
    vTaskDelete(xHandle);

    return 0;
}

void pthread_exit(void* value_ptr)
{
    itpThread* t = (itpThread*) xTaskGetApplicationTaskTag(xTaskGetCurrentTaskHandle());
    t->value_ptr = value_ptr;

#ifdef CFG_FS_FAT
    // exit fat on this task
    ioctl(ITP_DEVICE_FAT, ITP_IOCTL_DISABLE, NULL);
#endif

    if (t->detachstate == PTHREAD_CREATE_JOINABLE)
    {
        LOG_DBG "pthread 0x%X exit, wait to be joined\n", t LOG_END
        xSemaphoreGive(t->xSemaphore);
    }
    else
    {
        int i;
        TaskHandle_t xHandle;

        for (i = 0; i < PTHREAD_KEYS_MAX; i++)
        {
            itpThreadKey* k = &t->keys[i];
            if (k->dtor && k->value)
                k->dtor(k->value);
        }

        xHandle = xTaskGetIdleTaskHandle();
        vTaskPrioritySet(xHandle, tskIDLE_PRIORITY + 1);

        xHandle = t->xHandle;
        vPortFree(t);
        vTaskDelete(xHandle);
    }
    while (1)
        vTaskDelay(portMAX_DELAY);
}

int pthread_join(pthread_t pthread, void **value_ptr)
{
    itpThread* t = (itpThread*) xTaskGetApplicationTaskTag((TaskHandle_t) pthread);
    int i;
    TaskHandle_t xHandle;

    LOG_DBG "pthread join 0x%X\n", t LOG_END

    if (xSemaphoreTake(t->xSemaphore, portMAX_DELAY) == pdFALSE)
    {
        LOG_ERR "pthread_exit(0x%X) fail\n", value_ptr LOG_END
        return EINVAL;
    }

    for (i = 0; i < PTHREAD_KEYS_MAX; i++)
    {
        itpThreadKey* k = &t->keys[i];
        if (k->dtor && k->value)
            k->dtor(k->value);
    }

    if (value_ptr)
        *value_ptr = t->value_ptr;

    vSemaphoreDelete(t->xSemaphore);

    xHandle = xTaskGetIdleTaskHandle();
    vTaskPrioritySet(xHandle, tskIDLE_PRIORITY + 1);

    xHandle = t->xHandle;
    vPortFree(t);
    vTaskDelete(xHandle);

    return 0;
}

int pthread_detach(pthread_t pthread)
{
    itpThread* t = (itpThread*) xTaskGetApplicationTaskTag((TaskHandle_t) pthread);
    t->detachstate = PTHREAD_CREATE_DETACHED;
    if (t->xSemaphore)
    {
        vSemaphoreDelete(t->xSemaphore);
        t->xSemaphore = NULL;
    }
    return 0;
}

int pthread_setschedparam(pthread_t pthread, int policy, struct sched_param* param)
{
    itpThread* t = (itpThread*) xTaskGetApplicationTaskTag((TaskHandle_t) pthread);
    vTaskPrioritySet(t->xHandle, param->sched_priority);
    return 0;
}

pthread_t pthread_self(void)
{
    return (pthread_t) xTaskGetCurrentTaskHandle();
}

int pthread_equal(pthread_t t1, pthread_t t2)
{
    return t1 == t2;
}

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void))
{
    if (once_control->init_executed == 0)
    {
        once_control->init_executed = 1;
        init_routine();
    }
    return 0;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    attr->type = PTHREAD_MUTEX_RECURSIVE;
    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int kind)
{
    attr->type = kind;
    return 0;
}

int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t *attr)
{
    pthread_mutexattr_t a;
    itpMutex* m;

    if (mutex == NULL)
    {
        LOG_ERR "pthread_mutex_init(0x%X, 0x%X) invalid\n", mutex, attr LOG_END
        return EINVAL;
    }

    if (attr == NULL)
    {
        pthread_mutexattr_init(&a);
        attr = &a;
    }

    m = (itpMutex*) pvPortMalloc(sizeof(itpMutex));
    if (m == NULL)
    {
        LOG_ERR "pthread_mutex_init(0x%X, 0x%X) nomem\n", mutex, attr LOG_END
        return ENOMEM;
    }

    if (attr->type == PTHREAD_MUTEX_RECURSIVE)
        m->xMutex = xSemaphoreCreateRecursiveMutex();
    else
        m->xMutex = xSemaphoreCreateMutex();

    if (m->xMutex == NULL)
    {
        vPortFree(m);
        LOG_ERR "pthread_mutex_init(0x%X, 0x%X) fail\n", mutex, attr LOG_END
        return ENOMEM;
    }

    m->type = attr->type;
    *mutex = (pthread_mutex_t) m;
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t* mutex)
{
    if (mutex == NULL || *mutex == (pthread_mutex_t)NULL)
      return EINVAL;

    if (*mutex == PTHREAD_MUTEX_INITIALIZER)
    {
        *mutex = 0;
    }
    else
    {
        itpMutex* m = (itpMutex*)*mutex;

        vSemaphoreDelete(m->xMutex);
        vPortFree(m);
    }
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t* mutex)
{
    itpMutex* m;
    portBASE_TYPE ret;

    if (mutex == NULL || *mutex == (pthread_mutex_t)NULL)
    {
        LOG_ERR "pthread_mutex_lock(0x%X) invalid\n", mutex LOG_END
        return EINVAL;
    }

    portENTER_CRITICAL();

    if (*mutex == PTHREAD_MUTEX_INITIALIZER)
    {
        int ret2 = pthread_mutex_init(mutex, NULL);
        if (ret2)
        {
            portEXIT_CRITICAL();
            return ret2;
        }
    }
    m = (itpMutex*)*mutex;

    portEXIT_CRITICAL();

    if (m->type == PTHREAD_MUTEX_RECURSIVE)
        ret = xSemaphoreTakeRecursive(m->xMutex, portMAX_DELAY);
    else
        ret = xSemaphoreTake(m->xMutex, portMAX_DELAY);

    if (pdTRUE == ret)
        return 0;
    else
    {
        LOG_ERR "pthread_mutex_lock(0x%X) timeout\n", mutex LOG_END
        return EBUSY;
    }
}

int pthread_mutex_trylock(pthread_mutex_t * mutex)
{
    itpMutex* m;
    portBASE_TYPE ret;

    if (mutex == NULL || *mutex == (pthread_mutex_t)NULL)
    {
        LOG_ERR "pthread_mutex_trylock(0x%X) invalid\n", mutex LOG_END
        return EINVAL;
    }

    portENTER_CRITICAL();

    if (*mutex == PTHREAD_MUTEX_INITIALIZER)
    {
        int ret2;
        pthread_mutexattr_t attr;
        ret2 = pthread_mutexattr_init(&attr);
        if (ret2)
        {
            portEXIT_CRITICAL();
            return ret2;
        }

        ret2 = pthread_mutex_init(mutex, &attr);
        if (ret2)
        {
            portEXIT_CRITICAL();
            return ret2;
        }
    }
    m = (itpMutex*)*mutex;

    portEXIT_CRITICAL();

    if (m->type == PTHREAD_MUTEX_RECURSIVE)
        ret = xSemaphoreTakeRecursive(m->xMutex, 0);
    else
        ret = xSemaphoreTake(m->xMutex, 0);

    if (pdTRUE == ret)
        return 0;
    else
    {
        LOG_ERR "pthread_mutex_trylock(0x%X) timeout\n", mutex LOG_END
        return EBUSY;
    }
}

int pthread_mutex_unlock(pthread_mutex_t* mutex)
{
    if (mutex == NULL || *mutex == (pthread_mutex_t)NULL)
    {
        LOG_ERR "pthread_mutex_unlock(0x%X) invalid\n", mutex LOG_END
        return EINVAL;
    }

    if (*mutex != PTHREAD_MUTEX_INITIALIZER)
    {
        itpMutex* m = (itpMutex*)*mutex;
        if (m->type == PTHREAD_MUTEX_RECURSIVE)
            xSemaphoreGiveRecursive(m->xMutex);
        else
            xSemaphoreGive(m->xMutex);
    }
    return 0;
}

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
    itpCondition* c;
    if (cond == NULL)
    {
        LOG_ERR "pthread_cond_init(0x%X, 0x%X) invalid\n", cond, attr LOG_END
        return EINVAL;
    }

    c = (itpCondition*) pvPortMalloc(sizeof(itpCondition));
    if (c == NULL)
    {
        LOG_ERR "pthread_cond_init(0x%X, 0x%X) nomem\n", cond, attr LOG_END
        return ENOMEM;
    }

    c->xSemaphore = xSemaphoreCreateCounting(32, 0);
    if (c->xSemaphore == NULL)
    {
        vPortFree(c);
        LOG_ERR "pthread_cond_init(0x%X, 0x%X) fail\n", cond, attr LOG_END
        return ENOMEM;
    }
    c->count = 0;

    *cond = (pthread_cond_t) c;
    return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
    itpCondition* c;
    if (cond == NULL || *cond == (pthread_cond_t)NULL)
    {
        LOG_ERR "pthread_cond_destroy(0x%X) invalid\n", cond LOG_END
        return EINVAL;
    }

    if (*cond == PTHREAD_COND_INITIALIZER)
    {
        *cond = (pthread_cond_t)NULL;
    }
    else
    {
        itpCondition* c = (itpCondition*) *cond;
        vSemaphoreDelete(c->xSemaphore);
        vPortFree(c);
    }
    return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    itpCondition* c;

    if (cond == NULL || *cond == (pthread_cond_t)NULL)
    {
        LOG_ERR "pthread_cond_wait(0x%X, 0x%X) invalid\n", cond, mutex LOG_END
        return EINVAL;
    }

    portENTER_CRITICAL();

    if (*cond == PTHREAD_COND_INITIALIZER)
    {
        int ret = pthread_cond_init(cond, NULL);
        if (ret)
        {
            portEXIT_CRITICAL();
            return ret;
        }
    }

    c = (itpCondition*) *cond;
    c->count++;

    portEXIT_CRITICAL();

    if (pthread_mutex_unlock(mutex))
        return EINVAL;

    if (xSemaphoreTake(c->xSemaphore, portMAX_DELAY) == pdFALSE)
    {
        portENTER_CRITICAL();
        c->count--;
        portEXIT_CRITICAL();

        LOG_ERR "pthread_cond_wait(0x%X, 0x%X) fail\n", cond, mutex LOG_END
        return EINVAL;
    }

    if (pthread_mutex_lock(mutex))
        return EINVAL;

    return 0;
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    itpCondition* c;
    unsigned long ms;
    struct timeval tv;
    uint64_t start, finish;

    if (cond == NULL || *cond == (pthread_cond_t)NULL || abstime == NULL)
    {
        LOG_ERR "pthread_cond_timedwait(0x%X, 0x%X, 0x%X) invalid\n", cond, mutex, abstime LOG_END
        return EINVAL;
    }

#ifdef CFG_RTC_ENABLE
    ioctl(ITP_DEVICE_RTC, ITP_IOCTL_GET_TIME, (void*)&tv);
    start = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;

#else
    ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    tv.tv_usec = (ms % 1000) * 1000;
    tv.tv_sec  = CFG_RTC_DEFAULT_TIMESTAMP + ms / 1000;
    start = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;

#endif // CFG_RTC_ENABLE

    finish = (uint64_t)abstime->tv_sec * 1000 + abstime->tv_nsec / 1000000;
    ms = (long)(finish - start);

    portENTER_CRITICAL();

    if (*cond == PTHREAD_COND_INITIALIZER)
    {
        int ret = pthread_cond_init(cond, NULL);
        if (ret)
        {
            portEXIT_CRITICAL();
            return ret;
        }
    }

    c = (itpCondition*) *cond;
    c->count++;

    portEXIT_CRITICAL();

    if (pthread_mutex_unlock(mutex))
        return EINVAL;

    if (xSemaphoreTake(c->xSemaphore, ms * portTICK_PERIOD_MS) == pdFALSE)
    {
        portENTER_CRITICAL();
        c->count--;
        portEXIT_CRITICAL();

        LOG_DBG "xSemaphoreTake(0x%X, %d) timeout\n", c->xSemaphore, ms * portTICK_PERIOD_MS LOG_END
        return ETIMEDOUT;
    }

    if (pthread_mutex_lock(mutex))
        return EINVAL;

    return 0;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
    itpCondition* c;

    if (cond == NULL || *cond == (pthread_cond_t)NULL)
    {
        LOG_ERR "pthread_cond_signal(0x%X) invalid\n", cond LOG_END
        return EINVAL;
    }

    portENTER_CRITICAL();

    if (*cond == PTHREAD_COND_INITIALIZER)
    {
        int ret = pthread_cond_init(cond, NULL);
        if (ret)
        {
            portEXIT_CRITICAL();
            return ret;
        }
    }
    c = (itpCondition*) *cond;

    if (c->count == 0)
    {
        portEXIT_CRITICAL();
        return 0;
    }
    c->count--;

    portEXIT_CRITICAL();

    if (xSemaphoreGive(c->xSemaphore) == pdFALSE)
    {
        portENTER_CRITICAL();
        c->count++;
        portEXIT_CRITICAL();

        LOG_ERR "pthread_cond_signal(0x%X) timeout\n", cond LOG_END
        return EINVAL;
    }
    return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
    itpCondition* c;

    if (cond == NULL || *cond == (pthread_cond_t)NULL)
    {
        LOG_ERR "pthread_cond_broadcast(0x%X) invalid\n", cond LOG_END
        return EINVAL;
    }

    portENTER_CRITICAL();

    if (*cond == PTHREAD_COND_INITIALIZER)
    {
        int ret = pthread_cond_init(cond, NULL);
        if (ret)
        {
            portEXIT_CRITICAL();
            return ret;
        }
    }
    c = (itpCondition*) *cond;

    portEXIT_CRITICAL();

    for (;;)
    {
        portENTER_CRITICAL();

        if (c->count == 0)
        {
            portEXIT_CRITICAL();
            return 0;
        }
        c->count--;

        portEXIT_CRITICAL();

        if (xSemaphoreGive(c->xSemaphore) == pdFALSE)
        {
            portENTER_CRITICAL();
            c->count++;
            portEXIT_CRITICAL();

            LOG_ERR "pthread_cond_broadcast(0x%X) fail\n", cond LOG_END
            return EINVAL;
        }
    }
    return 0;
}

int pthread_key_create(pthread_key_t *key, void (*destructor)( void * ))
{
    itpThread* t = (itpThread*) xTaskGetApplicationTaskTag(xTaskGetCurrentTaskHandle());
    int i;

    if (!key)
    {
        LOG_ERR "pthread_key_create(0x%X, 0x%X) invalid\n", key, destructor LOG_END
        return EINVAL;
    }

    for (i = 0; i < PTHREAD_KEYS_MAX; i++)
    {
        itpThreadKey* k = &t->keys[i];

        if (!k->used)
        {
            k->used     = true;
            k->dtor     = destructor;
            k->value    = NULL;
            *key        = (pthread_key_t) k;
            return 0;
        }
    }
    LOG_ERR "pthread_key_create(0x%X, 0x%X) fail: %d\n", key, destructor, i LOG_END
    return EAGAIN;
}

int pthread_key_delete(pthread_key_t key)
{
    itpThreadKey* k = (itpThreadKey*)key;

    if (!k || !k->used)
    {
        LOG_ERR "pthread_key_delete(0x%X) invalid\n", key LOG_END
        return EINVAL;
    }

    k->used     = false;
    k->value    = NULL;
    return 0;
}

int pthread_setspecific(pthread_key_t key, const void *value)
{
    itpThreadKey* k = (itpThreadKey*)key;

    if (!k || !k->used)
    {
        LOG_ERR "pthread_setspecific(0x%X, 0x%X) invalid\n", key, value LOG_END
        return EINVAL;
    }

    k->value = (void*) value;
    return 0;
}

void *pthread_getspecific(pthread_key_t key)
{
    itpThreadKey* k = (itpThreadKey*)key;
    if (!k || !k->used)
    {
        LOG_ERR "pthread_getspecific(0x%X) invalid\n", key LOG_END
        return NULL;
    }

    return k->value;
}

int sched_get_priority_max(int policy)
{
    return configTIMER_TASK_PRIORITY - 1;
}

int sched_get_priority_min(int  policy)
{
    return tskIDLE_PRIORITY + 1;
}

int sched_yield(void)
{
    taskYIELD();
    return 0;
}

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
    if (!sem)
    {
        LOG_ERR "sem_init(0x%X, %d, %d) invalid\n", sem, pshared, value LOG_END
        return -1;
    }

    sem->__sem_lock = (void*) xSemaphoreCreateCounting(SEM_VALUE_MAX, value);
    if (sem->__sem_lock)
        return 0;
    else
    {
        LOG_ERR "sem_init(0x%X, %d, %d) fail\n", sem, pshared, value LOG_END
        return -1;
    }
}

int sem_destroy(sem_t *sem)
{
    if (!sem)
    {
        LOG_ERR "sem_destroy(0x%X) invalid\n", sem LOG_END
        return -1;
    }

    vSemaphoreDelete(sem->__sem_lock);
    return 0;
}

int sem_wait(sem_t *sem)
{
    if (!sem)
    {
        LOG_ERR "sem_wait(0x%X) invalid\n", sem LOG_END
        return -1;
    }

    if (pdTRUE == xSemaphoreTake(sem->__sem_lock, portMAX_DELAY))
        return 0;
    else
    {
        LOG_ERR "sem_wait(0x%X) fail\n", sem LOG_END
        return -1;
    }
}

int sem_trywait(sem_t *sem)
{
    if (!sem)
    {
        LOG_ERR "sem_trywait(0x%X) invalid\n", sem LOG_END
        return -1;
    }

    if (pdTRUE == xSemaphoreTake(sem->__sem_lock, 0))
        return 0;
    else
    {
        //LOG_DBG "sem_trywait(0x%X) fail\n", sem LOG_END
        return -1;
    }
}

int sem_post(sem_t *sem)
{
    if (!sem)
    {
        LOG_ERR "sem_post(0x%X) invalid\n", sem LOG_END
        return -1;
    }

    if(ithGetCpuMode() == ITH_CPU_SYS)
    {
        if (pdTRUE == xSemaphoreGive(sem->__sem_lock))
            return 0;
        else
        {
            LOG_DBG "sem_post(0x%X) fail\n", sem LOG_END
            return -1;
        }
    }
    else
    {
        itpSemPostFromISR(sem);
        return 0;
    }
}

int sem_getvalue(sem_t *sem, int *sval)
{
    if (!sem)
    {
        LOG_ERR "sem_getvalue(0x%X, 0x%X) invalid\n", sem, sval LOG_END
        return -1;
    }

    if(ithGetCpuMode() == ITH_CPU_SYS)
        *sval = uxQueueMessagesWaiting(sem->__sem_lock);
    else
        *sval = uxQueueMessagesWaitingFromISR(sem->__sem_lock);

    return 0;
}

int timer_create(clockid_t clock_id, struct sigevent *evp, timer_t *timerid)
{
    itpTimer* t = (itpTimer*) pvPortMalloc(sizeof (itpTimer));
    if (t == NULL)
    {
        LOG_ERR "timer_create(%d, 0x%X, 0x%X) nomem\n", clock_id, evp, timerid LOG_END
        return -1;
    }

    t->pxTimer  = NULL;
    t->routine  = NULL;
    t->arg      = 0;
    *timerid    = (timer_t) t;
    return 0;
}

#if configUSE_TIMERS == 1

int timer_delete(timer_t timerid)
{
    itpTimer* t = (itpTimer*)timerid;

    if (t->pxTimer)
    {
        if (xTimerDelete(t->pxTimer, 0) == pdFAIL)
        {
            LOG_ERR "timer_delete(%d) fail\n", timerid LOG_END
            return -1;
        }
    }

    vPortFree(t);
    return 0;
}

int timer_connect(timer_t timerid, VOIDFUNCPTR routine, int arg)
{
    ((itpTimer*)timerid)->routine  = routine;
    ((itpTimer*)timerid)->arg      = arg;
    return 0;
}

static void vTimerCallback(TimerHandle_t pxTimer)
{
    itpTimer* t = (itpTimer*) pvTimerGetTimerID(pxTimer);
    t->routine((timer_t)t, t->arg);
}

int timer_settime(timer_t timerid, int flags, const struct itimerspec *value, struct itimerspec *ovalue)
{
    itpTimer* t = (itpTimer*)timerid;
    TickType_t tick;
    unsigned portBASE_TYPE uxAutoReload;

    if (value->it_value.tv_sec == 0 && value->it_value.tv_nsec == 0)
    {
        if (t->pxTimer)
            xTimerStop(t->pxTimer, 0);

        return 0;
    }
    else if (value->it_interval.tv_sec > 0 || value->it_interval.tv_nsec > 0)
    {
        tick = value->it_interval.tv_sec * configTICK_RATE_HZ;
        tick += (uint64_t)value->it_interval.tv_nsec * configTICK_RATE_HZ / 1000000000;
        uxAutoReload = pdTRUE;
    }
    else
    {
        tick = value->it_value.tv_sec * configTICK_RATE_HZ;
        tick += (uint64_t)value->it_value.tv_nsec * configTICK_RATE_HZ / 1000000000;
        uxAutoReload = pdFALSE;
    }

    if (t->pxTimer && (uxAutoReload == pdFALSE))
	{
		if (xTimerChangePeriod(t->pxTimer, tick, 0) == pdFAIL)
		{
			LOG_ERR "timer_settime(%d, %d, 0x%X, 0x%X) fail\n", timerid, flags, value, ovalue LOG_END
			return -1;
		}
		return 0;
	}
	else if(t->pxTimer)
        xTimerDelete(t->pxTimer, 0);

    t->pxTimer = xTimerCreate("Timer", tick, uxAutoReload, (void*)t, vTimerCallback);
    if (!t->pxTimer)
    {
        LOG_ERR "timer_settime(%d, %d, 0x%X, 0x%X) fail\n", timerid, flags, value, ovalue LOG_END
        return -1;
    }

    if (xTimerStart(t->pxTimer, 0) == pdFAIL)
    {
        LOG_ERR "timer_settime(%d, %d, 0x%X, 0x%X) fail\n", timerid, flags, value, ovalue LOG_END
        return -1;
    }

    return 0;
}

#endif // configUSE_TIMERS == 1

int mq_close(mqd_t msgid)
{
    itpQueue* q = (itpQueue*) msgid;
    vQueueDelete(q->xQueue);
    vPortFree(q);
    return 0;
}

int mq_send(mqd_t msgid, const char *msg, size_t msg_len, unsigned int msg_prio)
{
    itpQueue* q = (itpQueue*) msgid;
    TickType_t xTicksToWait = (q->oflag & O_NONBLOCK) ? 0 : portMAX_DELAY;

    if (xQueueSend(q->xQueue, (const void*) msg, xTicksToWait) != pdTRUE)
    {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int mq_timedsend(mqd_t msgid, const char *msg, size_t msg_len, unsigned msg_prio, const struct timespec *abs_timeout)
{
    itpQueue* q = (itpQueue*) msgid;
    TickType_t xTicksToWait;
    
    if (q->oflag & O_NONBLOCK)
    {
        xTicksToWait = 0;
    }
    else
    {
        unsigned long ms;
        struct timeval tv;
        uint64_t start, finish;

    #ifdef CFG_RTC_ENABLE
        ioctl(ITP_DEVICE_RTC, ITP_IOCTL_GET_TIME, (void*)&tv);
        start = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;

    #else
        ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
        tv.tv_usec = (ms % 1000) * 1000;
        tv.tv_sec  = CFG_RTC_DEFAULT_TIMESTAMP + ms / 1000;
        start = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;

    #endif // CFG_RTC_ENABLE

        finish = (uint64_t)abs_timeout->tv_sec * 1000 + abs_timeout->tv_nsec / 1000000;
        ms = (long)(finish - start);
        xTicksToWait = ms * portTICK_PERIOD_MS;
    }

    if (xQueueSend(q->xQueue, (const void*) msg, xTicksToWait) != pdTRUE)
    {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

ssize_t mq_receive(mqd_t msgid, char *msg, size_t msg_len, unsigned int *msg_prio)
{
    itpQueue* q = (itpQueue*) msgid;
    TickType_t xTicksToWait = (q->oflag & O_NONBLOCK) ? 0 : portMAX_DELAY;

    if (xQueueReceive(q->xQueue, msg, xTicksToWait) != pdTRUE)
    {
        errno = EINVAL;
        return -1;
    }
    return q->uxItemSize;
}

ssize_t mq_timedreceive(mqd_t msgid, char *msg, size_t msg_len, unsigned int *msg_prio, const struct timespec *abs_timeout)
{
    itpQueue* q = (itpQueue*) msgid;
    TickType_t xTicksToWait;

    if (q->oflag & O_NONBLOCK)
    {
        xTicksToWait = 0;
    }
    else
    {
        unsigned long ms;
        struct timeval tv;
        uint64_t start, finish;

    #ifdef CFG_RTC_ENABLE
        ioctl(ITP_DEVICE_RTC, ITP_IOCTL_GET_TIME, (void*)&tv);
        start = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;

    #else
        ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
        tv.tv_usec = (ms % 1000) * 1000;
        tv.tv_sec  = CFG_RTC_DEFAULT_TIMESTAMP + ms / 1000;
        start = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;

    #endif // CFG_RTC_ENABLE

        finish = (uint64_t)abs_timeout->tv_sec * 1000 + abs_timeout->tv_nsec / 1000000;
        ms = (long)(finish - start);
        xTicksToWait = ms * portTICK_PERIOD_MS;
    }

    if (xQueueReceive(q->xQueue, msg, xTicksToWait) != pdTRUE)
    {
        errno = EINVAL;
        return -1;
    }
    return q->uxItemSize;
}

int _isatty(int file)
{
    return (file == STDIN_FILENO  ||
            file == STDERR_FILENO ||
            file == STDOUT_FILENO) ? 1 : 0;
}

int lstat(const char *path, struct stat *buf)
{
    return _stat(path, buf);
}

int dup(int fildes)
{
    return fildes;
}

int dup2(int fildes, int fildes2)
{
    return fildes2;
}

int _link(char *old, char *new)
{
    errno = ENOENT;
    return -1;
}

mode_t umask(mode_t mask)
{
    return 0;
}

long sysconf(int __name)
{
    switch (__name)
    {
    case _SC_PAGE_SIZE:
        return 4096U;

    default:
        return -1;
    }
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

char* basename(char *path)
{
    char *p;
    if( path == NULL || *path == '\0' )
        return ".";
    p = path + strlen(path) - 1;
    while( *p == '/' ) {
        if( p == path )
            return path;
        *p-- = '\0';
    }
    while( p >= path && *p != '/' )
        p--;
    return p + 1;
}

char *dirname(char *path)
{
    char *p;
    if( path == NULL || *path == '\0' )
        return ".";
    p = path + strlen(path) - 1;
    while( *p == '/' ) {
        if( p == path )
            return path;
        *p-- = '\0';
    }
    while( p >= path && *p != '/' )
        p--;
    return
        p < path ? "." :
        p == path ? "/" :
        (*p = '\0', path);
}

int fsync(int fd)
{
    if ((STDOUT_FILENO == fd) || (STDERR_FILENO == fd) || (STDIN_FILENO == fd))
    {
        errno = EINVAL;
        return  -1;
    }
    else
    {
        const ITPFSDevice* dev = (ITPFSDevice*) itpDeviceTable[(fd & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

        if (dev)
        {
            struct stat st;
            int ret = dev->fstat(fd & ITP_FILE_MASK, &st);
            if (ret == 0)
            {
                ITPDriveStatus* driveStatusTable;
                ITPDriveStatus* driveStatus;

                ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);
                driveStatus = &driveStatusTable[st.st_ino];

                if (driveStatus->avail && driveStatus->disk == ITP_DISK_NOR)
                {
                    ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
                }
                return 0;
            }
        }
        errno = EINVAL;
        return  -1;
    }
}

int fdatasync(int fd)
{
    return fsync(fd);
}

int waitpid(pid_t pid, int *status, int options)
{
    assert(0);
    return -1;
}

int pipe(int __fildes[2])
{
    return 0;
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

int sigaction(int sig, const struct sigaction * act, struct sigaction * oact)
{
    return 0;
}

int creat(const char *path, mode_t mode)
{
    assert(0);
    return -1;
}

int execv(const char *__path, char * const __argv[])
{
    assert(0);
    return -1;
}

int utime(const char *__file, const struct utimbuf *__times)
{
    assert(0);
    return -1;
}

pid_t getppid(void)
{
    assert(0);
    return -1;
}

int _execve(const char *path, char *const argv[], char *const envp[])
{
    assert(0);
    return -1;
}

pid_t _fork(void)
{
    return 0;
}

pid_t _wait(int *status)
{
    assert(0);
    return -1;
}

int execlp(const char *path, const char *argv0, ...)
{
    assert(0);
    return -1;
}

iconv_t iconv_open(const char *__tocode, const char *__fromcode)
{
    assert(0);
    return NULL;
}

size_t iconv(iconv_t __cd, char **__inbuf, size_t *__inbytesleft, char **__outbuf, size_t *__outbytesleft)
{
    assert(0);
    return 0;
}

int iconv_close(iconv_t __cd)
{
    assert(0);
    return -1;
}

int uname(struct utsname *name)
{
    memset(name, 0, sizeof (struct utsname));
    strcpy(name->sysname, CFG_SYSTEM_NAME);
    strcpy(name->version, CFG_VERSION_MAJOR_STR "." CFG_VERSION_MINOR_STR "." CFG_VERSION_PATCH_STR "." CFG_VERSION_CUSTOM_STR);
    strcpy(name->release, CFG_VERSION_TWEAK_STR);
    return 0;
}

int daemon(int nochdir, int noclose)
{
    return 0;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
    return -1;
}

void openlog(char *ident, int option ,int facility)
{
}

void syslog(int priority, const char *format, ...)
{
}

void vsyslog(int pri, const char *format, va_list ap)
{
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

void itpSemPostFromISR(sem_t* sem)
{
    signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(sem->__sem_lock, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

int itpSemGetValueFromISR(sem_t *sem, int *sval)
{
    if (!sem)
        return -1;

    *sval = uxQueueMessagesWaitingFromISR(sem->__sem_lock);
    return 0;
}

int itpSemWaitTimeout(sem_t *sem, unsigned long ms)
{
    return (pdTRUE == xSemaphoreTake(sem->__sem_lock, ms * portTICK_PERIOD_MS)) ? 0 : -1;
}

void itpTaskNotifyGive(pthread_t pthread)
{
    itpThread* t = (itpThread*) xTaskGetApplicationTaskTag((TaskHandle_t) pthread);
    xTaskNotifyGive(t->xHandle);
}

void itpTaskNotifyGiveFromISR(pthread_t pthread)
{
    itpThread* t = (itpThread*) xTaskGetApplicationTaskTag((TaskHandle_t) pthread);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    vTaskNotifyGiveFromISR(t->xHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

int itpTaskNotifyTake(bool clearCountOnExit, long msToWait)
{
    uint32_t ret = ulTaskNotifyTake(clearCountOnExit ? pdTRUE : pdFALSE, msToWait == -1 ? portMAX_DELAY : msToWait * portTICK_PERIOD_MS);
	return (ret > 0) ? 0 : -1;
}

long itpTimevalDiff(struct timeval *starttime, struct timeval *finishtime)
{
    uint64_t start = (uint64_t)starttime->tv_sec * 1000 + starttime->tv_usec / 1000;
    uint64_t finish = (uint64_t)finishtime->tv_sec * 1000 + finishtime->tv_usec / 1000;
    return (long)(finish - start);
}

uint32_t itpGetTickCount(void)
{
    if(ithGetCpuMode() == ITH_CPU_SYS)
        return xTaskGetTickCount() * portTICK_PERIOD_MS;
    else
        return xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
}

uint32_t itpGetTickDuration(uint32_t tick)
{
    uint32_t diff, t = itpGetTickCount();

    if (t >= tick)
    {
        diff = t - tick;
    }
    else
    {
        diff = 0xFFFFFFFF - tick + t;
    }
    return diff;
}

size_t itpFread(void* buf, size_t size, size_t count, FILE* fp)
{
    const ITPDevice* dev = itpDeviceTable[(fp->_file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (dev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return dev->read(fp->_file & ITP_FILE_MASK, (char*)buf, size * count, dev->info);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

size_t itpFwrite(const void* buf, size_t size, size_t count, FILE* fp)
{
    const ITPDevice* dev = itpDeviceTable[(fp->_file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (dev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return dev->write(fp->_file & ITP_FILE_MASK, (char*)buf, size * count, dev->info);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

int itpFseek(FILE* fp, long offset, int whence)
{
    const ITPDevice* dev = itpDeviceTable[(fp->_file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (dev)
    {
        /*  Pass call on to device driver.  Return result.        */
        if (dev->lseek(fp->_file & ITP_FILE_MASK, offset, whence, dev->info) == -1)
            return -1;

        return 0;
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

long itpFtell(FILE* fp)
{
    const ITPFSDevice* fsdev = (ITPFSDevice*)itpDeviceTable[(fp->_file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (fsdev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return fsdev->ftell(fp->_file & ITP_FILE_MASK);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

int itpFflush(FILE* fp)
{
    const ITPFSDevice* fsdev;

    if (fp == NULL)
        return _fwalk_reent(_GLOBAL_REENT, _fflush_r);

    if (fp == stdin || fp == stdout || fp == stderr)
        return _fflush_r(_REENT, fp);

    fsdev = (ITPFSDevice*)itpDeviceTable[(fp->_file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];
    if (fsdev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return fsdev->fflush(fp->_file & ITP_FILE_MASK);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

int itpFeof(FILE* fp)
{
    const ITPFSDevice* fsdev = (ITPFSDevice*)itpDeviceTable[(fp->_file & ITP_DEVICE_MASK) >> ITP_DEVICE_BIT];

    if (fsdev)
    {
        /*  Pass call on to device driver.  Return result.        */
        return fsdev->feof(fp->_file & ITP_FILE_MASK);
    }
    else
    {
        errno = ENOENT;
        return -1;
    }
}

ssize_t getline(char **bufptr, size_t *n, FILE *fp)
{
#define MIN_LINE_SIZE 4
#define DEFAULT_LINE_SIZE 128

    char *buf;
    char *ptr;
    size_t newsize, numbytes;
    int pos;
    int ch;
    int cont;

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
        *n = DEFAULT_LINE_SIZE;
    }

    numbytes = *n;
    ptr = buf;

    cont = 1;

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
            pos = ptr - buf;
            newsize = (*n << 1);
            buf = realloc (buf, newsize);
            if (buf == NULL)
            {
                cont = 0;
                break;
            }

            /* After reallocating, continue in new buffer */
            *bufptr = buf;
            *n = newsize;
            ptr = buf + pos;
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

int gethostname(char *name, size_t len)
{
    strncpy(name, "lwip", len);
    return 0;
}

int getnameinfo(const struct sockaddr *sa, socklen_t addrlen, char *host,
         socklen_t hostlen, char *serv, socklen_t servlen, unsigned int flags)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return EAI_FAIL;
}

const char *gai_strerror(int ecode)
{
    switch(ecode) {
    case EAI_FAIL   : return "A non-recoverable error occurred";
//    case EAI_FAMILY : return "The address family was not recognized or the address length was invalid for the specified family";
    case EAI_NONAME : return "The name does not resolve for the supplied parameters";
    case 201 : return "EAI_SERVICE";
    case 203 : return "EAI_MEMORY";
    case 210 : return "HOST_NOT_FOUND";
    case 211 : return "NO_DATA";
    case 212 : return "NO_RECOVERY";
    case 213 : return "TRY_AGAIN";
    }

    return "Unknown error";
}

struct servent *getservbyport(int port, const char *proto)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return NULL;
}

struct hostent *gethostbyaddr(const void *addr,
                              socklen_t len, int type)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return NULL;
}

int fnmatch (const char *pattern, const char *string, int flags)
{
  register const char *p = pattern, *n = string;
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

            c = *p++;
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

struct passwd* getpwuid(uid_t uid)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return NULL;
}

struct group* getgrgid(gid_t gid)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return NULL;
}

struct passwd* getpwnam(const char *name)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return NULL;
}

struct group* getgrnam(const char *name)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return NULL;
}

extern caddr_t __mmap_start__;
extern caddr_t __mmap_end__;
static unsigned int __mmap_top__ = 0;

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
    if (__mmap_top__ > (unsigned int)&__mmap_end__)
        return MAP_FAILED;

    //Let's only allocate in increments of a cache line.
    if (__mmap_top__ == 0)
        __mmap_top__ = (unsigned int)&__mmap_start__;

    unsigned int ret = __mmap_top__;
    __mmap_top__ += ((len + 31) & (~(0x1F)));
    if (__mmap_top__ > (unsigned int)&__mmap_end__ || __mmap_top__ < (unsigned int)&__mmap_start__)
        return MAP_FAILED;
    else
        return ((void *) ret);
}

//FIXME munmap() is called by malloc when releasing
//memory back to the "OS" or when destroying an mspace,
//like at the end of a program or when exit() is called.
//Our current dummy implementation always returns 0 (success),
//but this is misleading because we're not keeping track of memory,
//so that memory just "disappears" at that point.
//FIXME We need some code that keeps track of which chunks are allocated
//and which aren't; maybe we could use it for a SASOS type thing,
//and multiple apps could each have their own allocators
int munmap(void *addr, size_t len)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return 0;
}

int mprotect(void *addr, size_t len, int prot)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return 0;
}

int msync(void *addr, size_t len, int flags)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return 0;
}

int mlock(const void *addr, size_t len)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return 0;
}

int munlock(const void *addr, size_t len)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return 0;
}

int setpriority(int which, id_t who, int value)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return 0;
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return -1;
}

int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return -1;
}

char *realpath(const char *path,char *resolved_path)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return NULL;
}

int nftw(const char *dirpath,
        int (*fn) (const char *fpath, const struct stat *sb,
                   int typeflag, struct FTW *ftwbuf),
        int nopenfd, int flags)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return -1;
}

int ftruncate(int file, off_t length)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return -1;
}

FILE *popen(const char *command, const char *type)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return NULL;
}

int pclose(FILE *stream)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return -1;
}

int setuid(uid_t uid)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return -1;
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

ssize_t readlink(const char *path, char *buf, size_t bufsiz)
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return -1;
}

int utimes(const char *filename, const struct timeval times[2])
{
    LOG_DBG "%s NO IMPL\n", __FUNCTION__ LOG_END
    return -1;
}

uint32_t itpGetFreeHeapSize(void)
{
    struct mallinfo mi = mallinfo();
    return (unsigned int)&__heap_end__ - (unsigned int)&__heap_start__ - mi.uordblks;
}

unsigned alarm(unsigned __secs)
{
}
