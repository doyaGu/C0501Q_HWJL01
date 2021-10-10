#include <sys/ioctl.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "ite/ug.h"
#include "ug_cfg.h"

#ifndef CFG_UPGRADE_MARK_POS
    #define CFG_UPGRADE_MARK_POS 0
#endif

#if CFG_UPGRADE_MARK_POS

static const char ugMarkMagic[] = { 'U', 'P', 'G', 'R', 'A', 'D', 'I', 'N', 'G' };

static int ugMarkFD = -1;
static uint32_t ugMarkBlockSize, ugMarkPos;
static uint8_t* ugMarkBuf = NULL;

static void ugMarkExit(void)
{
    free(ugMarkBuf);
    ugMarkBuf = NULL;

    if (ugMarkFD != -1)
    {
        close(ugMarkFD);
        ugMarkFD = -1;
    }
}

static int ugMarkInit(void)
{
    int ret = 0;

    assert(ugMarkFD == -1);

#if defined(CFG_UPGRADE_IMAGE_NAND)
    ugMarkFD = open(":nand", O_RDONLY, 0);
    LOG_DBG "nand fd: 0x%X\n", ugMarkFD LOG_END
#elif defined(CFG_UPGRADE_IMAGE_NOR)
    ugMarkFD = open(":nor", O_RDONLY, 0);
    LOG_DBG "nor fd: 0x%X\n", ugMarkFD LOG_END
#elif defined(CFG_UPGRADE_IMAGE_SD0)
    ugMarkFD = open(":sd0", O_RDONLY, 0);
    LOG_DBG "sd0 fd: 0x%X\n", ugMarkFD LOG_END
#elif defined(CFG_UPGRADE_IMAGE_SD1)
    ugMarkFD = open(":sd1", O_RDONLY, 0);
    LOG_DBG "sd1 fd: 0x%X\n", ugMarkFD LOG_END
#else
    #error "unknown device."
#endif

    if (ugMarkFD == -1)
    {
        LOG_ERR "open device error\n" LOG_END
        ret = __LINE__;
        goto end;
    }

    if (ioctl(ugMarkFD, ITP_IOCTL_GET_BLOCK_SIZE, &ugMarkBlockSize))
    {
        LOG_ERR "get block size error\n" LOG_END
        ret = __LINE__;
        goto end;
    }
    ugMarkPos = CFG_UPGRADE_MARK_POS / ugMarkBlockSize;

    LOG_DBG "ugMarkFD=0x%X ugMarkBlockSize=%d ugMarkPos=0x%X\n", ugMarkFD, ugMarkBlockSize, ugMarkPos LOG_END

    if (lseek(ugMarkFD, ugMarkPos, SEEK_SET) != ugMarkPos)
    {
        LOG_ERR "seek to mark position %d(%d) error\n", CFG_UPGRADE_MARK_POS, ugMarkPos LOG_END
        ret = __LINE__;
        goto end;
    }

    assert(ugMarkBlockSize >= 8);
    ugMarkBuf = (uint8_t *)malloc(ugMarkBlockSize);
    if (!ugMarkBuf)
    {
        LOG_ERR "out of memory %d\n", ugMarkBlockSize LOG_END
        ret = __LINE__;
        goto end;
    }

    if (read(ugMarkFD, ugMarkBuf, 1) != 1)
    {
        LOG_ERR "read mark error: %d != 1\n", ret LOG_END
        ret = __LINE__;
        goto end;
    }

end:
    if (ret)
        ugMarkExit();

    return ret;
}
#endif // CFG_UPGRADE_MARK_POS

int ugUpgradeCheck(void)
{
    int ret = 0;
#if CFG_UPGRADE_MARK_POS

    ret = ugMarkInit();
    if (ret)
        goto end;

    // check
    if (memcmp(ugMarkBuf, ugMarkMagic, sizeof(ugMarkMagic)) == 0)
    {
        LOG_ERR "last upgrading is not finished.\n" LOG_END
        ret = __LINE__;
        goto end;
    }

end:
    ugMarkExit();

#endif // CFG_UPGRADE_MARK_POS
    return ret;
}

int ugUpgradeStart(void)
{
    int ret = 0;
#if CFG_UPGRADE_MARK_POS

    ret = ugMarkInit();
    if (ret)
        goto end;

    memcpy(ugMarkBuf, ugMarkMagic, sizeof(ugMarkMagic));

    if (lseek(ugMarkFD, ugMarkPos, SEEK_SET) != ugMarkPos)
    {
        LOG_ERR "seek to mark position %d(%d) error\n", CFG_UPGRADE_MARK_POS, ugMarkPos LOG_END
        ret = __LINE__;
        goto end;
    }

    if (write(ugMarkFD, ugMarkBuf, 1) != 1)
    {
        LOG_ERR "write mark fail\n" LOG_END
        goto end;
    }

end:
    ugMarkExit();

#endif // CFG_UPGRADE_MARK_POS
    return ret;
}


int ugUpgradeFinish(void)
{
    int ret = 0;
#if CFG_UPGRADE_MARK_POS

    ret = ugMarkInit();
    if (ret)
        goto end;

    memset(ugMarkBuf, -1, sizeof(ugMarkMagic));

    if (lseek(ugMarkFD, ugMarkPos, SEEK_SET) != ugMarkPos)
    {
        LOG_ERR "seek to mark position %d(%d) error\n", CFG_UPGRADE_MARK_POS, ugMarkPos LOG_END
        ret = __LINE__;
        goto end;
    }

#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
    ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif

    if (write(ugMarkFD, ugMarkBuf, 1) != 1)
    {
        LOG_ERR "clear mark fail\n" LOG_END
        goto end;
    }

end:
    ugMarkExit();

#endif // CFG_UPGRADE_MARK_POS

    return ret;
}
