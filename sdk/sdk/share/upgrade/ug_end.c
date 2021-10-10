#include <errno.h>
#include <stdio.h>
#include "ite/ug.h"
#include "ug_cfg.h"

#pragma pack(1)
typedef struct
{
    uint32_t crc;
} end_t;

int ugUpgradeEnd(ITCStream *f)
{
    int ret = 0;
    end_t end;
    uint32_t readsize;

    // read end block
    readsize = itcStreamRead(f, &end, sizeof(end_t));
    if (readsize != sizeof(end_t))
    {
        ret = __LINE__;
        LOG_ERR "Cannot read file: %ld != %ld\n", readsize, sizeof(end_t) LOG_END
        goto end;
    }

    end.crc = itpLetoh32(end.crc);
    LOG_DBG "crc=0x%X\n", end.crc LOG_END

end:
    return ret;
}
