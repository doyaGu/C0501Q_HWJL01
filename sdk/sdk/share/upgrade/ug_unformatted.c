#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ite/ug.h"
#include "ug_cfg.h"

#pragma pack(1)
typedef struct
{
    uint32_t unformatted_size;
} unformatted_t;

int ugUpgradeUnformatted(ITCStream *f, ITPDisk disk)
{
    int ret = 0;
    unformatted_t fmt;
    uint32_t readsize;

    // read unformatted header
    readsize = itcStreamRead(f, &fmt, sizeof(unformatted_t));
    if (readsize != sizeof(unformatted_t))
    {
        ret = __LINE__;
        LOG_ERR "Cannot read file: %ld != %ld\n", readsize, sizeof(unformatted_t) LOG_END
        return ret;
    }

    fmt.unformatted_size  = itpLetoh32(fmt.unformatted_size);

    LOG_DBG "unformatted_size=%ld\n", fmt.unformatted_size LOG_END

    return ret;
}
