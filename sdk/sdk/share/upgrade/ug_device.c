#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "ite/ug.h"
#include "ug_cfg.h"

typedef enum
{
    DEVICE_NAND = 0,
    DEVICE_NOR  = 1,
    DEVICE_SD0  = 2,
    DEVICE_SD1  = 3,

    DEVICE_COUNT
} DeviceType;

#pragma pack(1)
typedef struct
{
    uint32_t device_type;
    uint32_t unformatted_size;
    uint32_t partition_count;
} device_t;

static const char* device_table[] =
{
    "NAND",
    "NOR",
    "SD0",
    "SD1"
};

int ugUpgradeDevice(ITCStream *f, ITPDisk* disk, bool partition, bool nopartition)
{
    int i, ret = 0;
    device_t dev;
    uint64_t size[ITP_MAX_PARTITION], inc;
    ITPPartition par;
    ITPDriveStatus* driveStatusTable;
    uint32_t readsize;

    // read device header
    readsize = itcStreamRead(f, &dev, sizeof(device_t));
    if (readsize != sizeof(device_t))
    {
        ret = __LINE__;
        LOG_ERR "Cannot read file: %ld != %ld\n", readsize, sizeof(device_t) LOG_END
        goto end;
    }

    dev.device_type         = itpLetoh32(dev.device_type);
    dev.unformatted_size    = itpLetoh32(dev.unformatted_size);
    dev.partition_count     = itpLetoh32(dev.partition_count);

    LOG_INFO "Upgrade %s. Non-partition size is %ld bytes, and having %ld partition(s)\n",
        device_table[dev.device_type],
        dev.unformatted_size,
        dev.partition_count
    LOG_END

    readsize = itcStreamRead(f, &size, sizeof(uint64_t) * dev.partition_count);
    if (readsize != sizeof(uint64_t) * dev.partition_count)
    {
        ret = __LINE__;
        LOG_ERR "Cannot read file: %ld != %ld\n", readsize, sizeof(uint64_t) * dev.partition_count LOG_END
        goto end;
    }

    for (i = 0; i < (int)dev.partition_count; i++)
    {
        size[i] = itpLetoh64(size[i]);

        if (size[i] > 0)
            LOG_INFO "Partition[%d] size is %lld\n", i, size[i] LOG_END
        else
            LOG_INFO "Partition[%d] size is maximum availiable\n", i LOG_END
    }

    if (dev.device_type == DEVICE_NAND)
    {
        par.disk = ITP_DISK_NAND;
    }
    else if (dev.device_type == DEVICE_NOR)
    {
        par.disk = ITP_DISK_NOR;
    }
    else if (dev.device_type == DEVICE_SD0)
    {
        par.disk = ITP_DISK_SD0;
    }
    else if (dev.device_type == DEVICE_SD1)
    {
        par.disk = ITP_DISK_SD1;
    }
    else
    {
        ret = __LINE__;
        LOG_ERR "Unknown device type: %d\n", dev.device_type LOG_END
        goto end;
    }

    *disk = par.disk;

    if (dev.partition_count == 0)
        goto end;

    if (partition)
    {
        LOG_INFO "Force to partition\n" LOG_END
        goto repartition;
    }

#ifdef _WIN32
    goto end;
#endif

    if (!nopartition)
    {
        ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_GET_PARTITION, &par);
        if (ret)
        {
            LOG_INFO "Origial partition not exist\n" LOG_END
            goto repartition;
        }

        if (par.count != dev.partition_count)
        {
            LOG_INFO "Origial partition count %d not equal %d\n", par.count, dev.partition_count LOG_END
            goto repartition;
        }

        for (i = 0; i < (int)dev.partition_count; i++)
        {
            LOG_INFO "Origial partition %d size is %lld bytes\n", i, par.size[i] LOG_END
            if (size[i] && par.size[i] != size[i])
                goto repartition;
        }
    }
    goto end;

repartition:
    LOG_INFO "Partitioning...\n" LOG_END

    par.count = dev.partition_count;
    inc = dev.unformatted_size;
    for (i = 0; i < (int)dev.partition_count; i++)
    {
        par.start[i] = inc;
        par.size[i] = size[i];
        inc += size[i];
    }

    LOG_INFO "Unmount %s disk(s)...\n", device_table[dev.device_type] LOG_END
    ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)par.disk);
    if (ret)
    {
        LOG_ERR "Unmount %s disk(s) fail: 0x%X\n", device_table[dev.device_type], ret LOG_END
    }

    LOG_INFO "Create partition(s)...\n" LOG_END
    ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_CREATE_PARTITION, &par);
    if (ret)
    {
        LOG_ERR "Create partition(s) fail: 0x%X\n", ret LOG_END
        goto end;
    }

    LOG_INFO "Mount %s disk(s)...\n", device_table[dev.device_type] LOG_END
    ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)par.disk);
    if (ret)
    {
        LOG_ERR "Mount %s disk(s) fail: 0x%X\n", device_table[dev.device_type], ret LOG_END
        goto end;
    }

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    for (i = 0; i < ITP_MAX_DRIVE; i++)
    {
        ITPDriveStatus* driveStatus = &driveStatusTable[i];

        LOG_DBG "drive[%d] disk=%d avail=%d\n", i, driveStatus->disk, driveStatus->avail LOG_END

        if (driveStatus->disk == par.disk && driveStatus->avail)
        {
            LOG_INFO "Format drive %c: ...\n", 'A' + i LOG_END

            ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*) i);
            if (ret)
            {
                LOG_ERR "Format fail: 0x%X\n", ret LOG_END
                goto end;
            }
        }
    }
    LOG_INFO "Partition finished.\n" LOG_END

end:
#ifdef _WIN32
    ret = 0;
#endif
    return ret;
}
