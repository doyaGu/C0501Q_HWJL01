#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "bootloader.h"
#include "config.h"

void RestorePackage(void)
{
    uint32_t ret, pkgsize, alignsize, blockcount, blocksize, pos, gapsize;
    uint8_t *pkgbuf;
    ITCArrayStream arrayStream;

#if defined(CFG_UPGRADE_IMAGE_NAND)
    int fd = open(":nand", O_RDONLY, 0);
    LOG_DBG "nand fd: 0x%X\n", fd LOG_END
#elif defined(CFG_UPGRADE_IMAGE_NOR)
    int fd;
    fd = open(":nor", O_RDONLY, 0);
    LOG_DBG "nor fd: 0x%X\n", fd LOG_END
#elif defined(CFG_UPGRADE_IMAGE_SD0)
    int fd = open(":sd0", O_RDONLY, 0);
    LOG_DBG "sd0 fd: 0x%X\n", fd LOG_END
#elif defined(CFG_UPGRADE_IMAGE_SD1)
    int fd = open(":sd1", O_RDONLY, 0);
    LOG_DBG "sd1 fd: 0x%X\n", fd LOG_END
#endif

    if (fd == -1)
    {
        LOG_ERR "open device error: %d\n", fd LOG_END
        return;
    }

    if (ioctl(fd, ITP_IOCTL_GET_BLOCK_SIZE, &blocksize))
    {
        LOG_ERR "get block size error\n" LOG_END
        return;
    }
    
    if (ioctl(fd, ITP_IOCTL_GET_GAP_SIZE, &gapsize))
    {
        LOG_ERR "get gap size error:\n" LOG_END
        return;
    }    
    
    // read package size
    pos = CFG_UPGRADE_BACKUP_PACKAGE_POS / (blocksize + gapsize);
    blockcount = CFG_UPGRADE_BACKUP_PACKAGE_POS % (blocksize + gapsize);
    if (blockcount > 0)
    {
        LOG_WARN "package position 0x%X and block size 0x%X are not aligned; align to 0x%X\n", CFG_UPGRADE_BACKUP_PACKAGE_POS, blocksize, (pos * blocksize) LOG_END
    }

    LOG_DBG "blocksize=%d pos=0x%X blockcount=%d\n", blocksize, pos, blockcount LOG_END

    if (lseek(fd, pos, SEEK_SET) != pos)
    {
        LOG_ERR "seek to package position %d error\n", pos LOG_END
        return;
    }

    pkgbuf = malloc(blocksize);
    if (!pkgbuf)
    {
        LOG_ERR "out of memory %d\n", blocksize LOG_END
        return;
    }

    blockcount = 1;
    ret = read(fd, pkgbuf, blockcount);
    if (ret != blockcount)
    {
        LOG_ERR "read package header error: %d != %d\n", ret, blockcount LOG_END
        return;
    }

    pkgsize = *(uint32_t*)pkgbuf;
    pkgsize = itpLetoh32(pkgsize);
    LOG_DBG "package size: %d\n", pkgsize LOG_END

    // read package
    alignsize = ITH_ALIGN_UP(pkgsize + 4, blocksize);
    pkgbuf = realloc(pkgbuf, alignsize);
    if (!pkgbuf)
    {
        LOG_ERR "out of memory %d\n", alignsize LOG_END
        return;
    }

    blockcount = alignsize / blocksize - blockcount;

    ret = read(fd, pkgbuf + blocksize, blockcount);
    if (ret != blockcount)
    {
        LOG_ERR "read package error: %d != %d\n", ret, blockcount LOG_END
        return;
    }

    itcArrayStreamOpen(&arrayStream, pkgbuf + 4, pkgsize);
    
    if (ugCheckCrc(&arrayStream, NULL))
        LOG_ERR "Upgrade failed.\n" LOG_END
    else
        ret = ugUpgradePackage(&arrayStream);

#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
    LOG_INFO "Flushing NOR cache...\n" LOG_END
    ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif

    if (ret)
        LOG_INFO "Upgrade failed.\n" LOG_END
    else
        LOG_INFO "Upgrade finished.\n" LOG_END

    exit(ret);    
}
