#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ite/ug.h"
#include "ug_cfg.h"

#define MAX_PARTITION_COUNT 4

#pragma pack(1)
typedef struct
{
    uint64_t partition_size;
} partition_t;

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

int ugUpgradePartition(ITCStream *f, ITPDisk disk, int partition, char* drive, bool format)
{
    ITPDriveStatus* driveStatusTable;
    int ret, i, j;
    partition_t par;
    uint32_t readsize;

    // read partition header
    readsize = itcStreamRead(f, &par, sizeof(partition_t));
    if (readsize != sizeof(partition_t))
    {
        ret = __LINE__;
        LOG_ERR "Cannot read file: %ld != %ld\n", readsize, sizeof(partition_t) LOG_END
        return ret;
    }

    par.partition_size  = itpLetoh64(par.partition_size);

    LOG_DBG "partition_size=%ld\n", par.partition_size LOG_END

#ifdef _WIN32
    goto end;
#endif

    // try to find the drive
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    j = 0;
    for (i = 0; i < ITP_MAX_DRIVE; i++)
    {
        ITPDriveStatus* driveStatus = &driveStatusTable[i];

        if (driveStatus->avail)
        {
            LOG_DBG 
                "drive %c: disk=%s name=%s\n", 'A' + i, disk_table[driveStatus->disk], driveStatus->name 
            LOG_END
        }

        if (driveStatus->disk == disk && driveStatus->avail)
        {
            if (j++ == partition)
            {
                if (format)
                {
                    ret = ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*) i);
                LOG_DBG 
                    "format drive %c: disk=%s name=%s ret=%d\n", 'A' + i, disk_table[driveStatus->disk], driveStatus->name, ret
                LOG_END
                }
                strcpy(drive, driveStatus->name);
                ret = 0;
                goto end;
            }
        }
    }

    LOG_ERR "Cannot find the partition %d in %s disk\n", partition, disk_table[disk] LOG_END
    ret = __LINE__;

end:
#ifdef _WIN32
    ret = 0;
    strcpy(drive, "A:/");
#endif
    return ret;
}
