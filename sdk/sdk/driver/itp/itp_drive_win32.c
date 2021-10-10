/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL drive functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <sys/ioctl.h>
#include "itp_cfg.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static ITPDriveStatus driveStatusTable[ITP_MAX_DRIVE];

static void DriveMountDisk(ITPDisk disk)
{
    int writable, removable;
    int ret, i;

    (void)ret;

#ifdef CFG_FS_NTFS
    ret = ioctl(ITP_DEVICE_NTFS, ITP_IOCTL_MOUNT, (void*) disk);
#endif
#ifdef CFG_FS_FAT
    ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_MOUNT, (void*) disk);
#endif

    switch (disk)
    {
#ifdef CFG_SD0_ENABLE
    case ITP_DISK_SD0:
    #ifdef CFG_SD0_STATIC
        writable    = true;
        removable   = false;
    #else
        writable    = !ithCardLocked(ITH_CARDPIN_SD0);
        removable   = true;
    #endif // CFG_SD0_STATIC
        break;
#endif // CFG_SD0_ENABLE

#ifdef CFG_SD1_ENABLE
    case ITP_DISK_SD1:
    #ifdef CFG_SD1_STATIC
        writable    = true;
        removable   = false;
    #else
        writable    = !ithCardLocked(ITH_CARDPIN_SD1);
        removable   = true;
    #endif // CFG_SD0_STATIC
        break;
#endif // CFG_SD0_ENABLE

#ifdef CFG_CF_ENABLE
    case ITP_DISK_CF:
        // TODO: IMPLEMENT
        writable = true;
        removable = true;
        break;
#endif // CFG_CF_ENABLE

#ifdef CFG_MS_ENABLE
    case ITP_DISK_MS:
        writable = iteMsproIsLock();
        removable = true;
        break;
#endif // CFG_MS_ENABLE

#ifdef CFG_XD_ENABLE
    case ITP_DISK_XD:
        writable = true;
        removable = true;
        break;
#endif // CFG_XD_ENABLE

#ifdef CFG_MSC_ENABLE
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
            writable = true;
            removable = true;
            break;
#endif // CFG_MSC_ENABLE

    default:
        writable = true;
        removable = false;
    }

    for (i = 0; i < ITP_MAX_DRIVE; i++)
    {
        ITPDriveStatus* driveStatus = &driveStatusTable[i];
        
        if (driveStatus->avail && driveStatus->disk == disk)
        {
            driveStatus->writable   = writable;
            driveStatus->removable  = removable;
        }
    }
}

static void DriveUnmountDisk(ITPDisk disk)
{
    int ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*) disk);
    if (ret)
        ret = ioctl(ITP_DEVICE_NTFS, ITP_IOCTL_UNMOUNT, (void*) disk);
}

static const char* disk_table[] =
{
    "SD0",
    "SD1", 
    "CF",
    "MS",
    "XD",
    "NAND",
    "NOR",
    "MSC00",
    "MSC01",
    "MSC02",
    "MSC03",
    "MSC04",
    "MSC05",
    "MSC06",
    "MSC07",
    "MSC10",
    "MSC11",
    "MSC12",
    "MSC13",
    "MSC14",
    "MSC15",
    "MSC16",
    "MSC17",
    "RAM",
};

static void DriveMount(void)
{
#ifdef CFG_WIN32_FS_HW
    int i;

#ifdef CFG_NAND_ENABLE
    DriveMountDisk(ITP_DISK_NAND);
#endif

#ifdef CFG_NOR_ENABLE
    DriveMountDisk(ITP_DISK_NOR);
#endif

#ifdef CFG_SD0_STATIC
    DriveMountDisk(ITP_DISK_SD0);
#endif

#ifdef CFG_SD1_STATIC
    DriveMountDisk(ITP_DISK_SD1);
#endif

#ifdef CFG_RAMDISK_ENABLE
    DriveMountDisk(ITP_DISK_RAM);
#endif

    for (i = 0; i < ITP_MAX_DRIVE; i++)
    {
        ITPDriveStatus* driveStatus = &driveStatusTable[i];
        if (driveStatus->avail)
        {
            LOG_INFO
                "Drive %c: disk=%s dev=%d name=%s\n", 'A' + i, disk_table[driveStatus->disk], driveStatus->device >> ITP_DEVICE_BIT, driveStatus->name 
            LOG_END
        }
    }
#else
    // simulate private for A:/ , public for B:/ , SD #0 for C:/ and USB #0 for D:/
    int i;

    for (i = 0; i < ITP_MAX_DRIVE; i++)
    {
        ITPDriveStatus* driveStatus = &driveStatusTable[i];

        if (i == 0 || i == 1)
        {
            driveStatus->disk = ITP_DISK_NOR;
            driveStatus->avail = true;
        }
#if 0
        else if (i == 2)
        {
            driveStatus->disk = ITP_DISK_SD0;
            driveStatus->avail = true;
        }
        else if (i == 3)
        {
            driveStatus->disk = ITP_DISK_MSC00;
            driveStatus->avail = true;
        }
        else
        {
            driveStatus->avail = false;
        }
#endif // 1
    #ifdef CFG_FS_NTFS
        driveStatus->device    = ITP_DEVICE_NTFS;
    #else
        driveStatus->device    = ITP_DEVICE_FAT;
    #endif
        driveStatus->name[0]   = 'A' + i;
        driveStatus->name[1]   = ':';
        driveStatus->name[2]   = '/';
        driveStatus->name[3]   = '\0';
        driveStatus->writable  = true;
        driveStatus->removable = true;
    }
#endif // CFG_WIN32_FS_HW
}

static void DriveUnmount(void)
{
#ifdef CFG_WIN32_FS_HW
    int i;
    
    for (i = 0; i < ITP_MAX_DRIVE; i++)
    {
        ITPDriveStatus* driveStatus = &driveStatusTable[i];

        if (driveStatus->avail)
            DriveUnmountDisk(driveStatus->disk);
    }
#endif // CFG_WIN32_FS_HW
}

static void DriveInit(void)
{
    int i;
    
    for (i = 0; i < ITP_MAX_DRIVE; i++)
    {
        ITPDriveStatus* driveStatus = &driveStatusTable[i];
        
        driveStatus->disk      = -1;
        driveStatus->device    = -1;
        driveStatus->avail     = false;
    }
}

static int DriveIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        DriveInit();
        break;

    case ITP_IOCTL_ENABLE:
        break;
        
    case ITP_IOCTL_DISABLE:
        break;

    case ITP_IOCTL_MOUNT:
        DriveMount();
        break;
        
    case ITP_IOCTL_UNMOUNT:
        DriveUnmount();
        break;

    case ITP_IOCTL_PROBE:
        break;

    case ITP_IOCTL_GET_TABLE:
        *(ITPDriveStatus**)ptr = driveStatusTable;
        break;

    default:
        errno = (ITP_DEVICE_DRIVE << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceDrive =
{
    ":drive",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    DriveIoctl,
    NULL
};
