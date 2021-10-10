/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Codec functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "itp_cfg.h"

#define CODEC_PATH (CFG_PRIVATE_DRIVE ":/codec/")

static const char* codecTable[] =
{
    "aac",
    "amr",
    "eac3",
    "mp3",
    "wave",
    "wma",
    "flac",    
};

#define HEADER_LEN_OFFSET   12

static int CodecOpen(const char* name, int flags, int mode, void* info)
{
    int i;

    if (name == NULL)
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        return -1;
    }

    for (i = 0; i < ITH_COUNT_OF(codecTable); i++)
    {
        if (strcmp(name, codecTable[i]) == 0)
            return i;
    }

    LOG_ERR "codec not exist: %s\n", name LOG_END
    errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
    return -1;
}

static int CodecClose(int file, void* info)
{
    return 0;
}

static int CodecRead(int file, char *ptr, int len, void* info)
{
    int ret = 0;
    char filepath[32], buf2[4];
    FILE* f;
    uint32_t headersize, imagesize = 0;
    char* buf = NULL;
    int nResult;

    if (file >= ITH_COUNT_OF(codecTable))
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        return 0;
    }

    strcpy(filepath, CODEC_PATH);
    strcat(filepath, &codecTable[file][0]);

    strcat(filepath, ".codecs");
    f = fopen(filepath, "rb");
    if (!f)
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "cannot find codec %s\n", filepath LOG_END
        return 0;
    }

    // get header size
    if (fseek(f, HEADER_LEN_OFFSET, SEEK_SET))
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "cannot seek codec %s\n", filepath LOG_END
        goto end;
    }

    if (fread(&headersize, 1, sizeof (uint32_t), f) != sizeof (uint32_t))
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "cannot read codec %s\n", filepath LOG_END
        goto end;
    }
    headersize = itpBetoh32(headersize);

    // get image size
    if (fseek(f, headersize - 4, SEEK_SET))
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "cannot seek codec %s\n", filepath LOG_END
        goto end;
    }

    if (fread(&imagesize, 1, sizeof (uint32_t), f) != sizeof (uint32_t))
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "cannot read codec %s\n", filepath LOG_END
        goto end;
    }
    imagesize = itpBetoh32(imagesize);

    if (imagesize != len)
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "decompress size mismatch: %d != %d\n", imagesize, len LOG_END
        goto end;
    }

    if (fseek(f, headersize, SEEK_SET))
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "cannot seek codec %s\n", filepath LOG_END
        goto end;
    }

    if (fread(buf2, 1, 4, f) != 4)
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "cannot read codec %s\n", filepath LOG_END
        goto end;
    }

    if (strncmp(buf2, "SMAZ", 4) == 0)
    {
        uint32_t contentsize;

        // get content size
        if (fseek(f, HEADER_LEN_OFFSET + 4, SEEK_SET))
        {
            errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
            LOG_ERR "cannot seek codec %s\n", filepath LOG_END
            goto end;
        }

        if (fread(&contentsize, 1, sizeof (uint32_t), f) != sizeof (uint32_t))
        {
            errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
            LOG_ERR "cannot read codec %s\n", filepath LOG_END
            goto end;
        }
        contentsize = itpBetoh32(contentsize);

        // read data
        buf = memalign(32, ITH_ALIGN_UP(contentsize,32));
        if (!buf)
        {
            errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
            LOG_ERR "out of memory: %d\n", imagesize LOG_END
            goto end;
        }

        if (fseek(f, headersize, SEEK_SET))
        {
            errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
            LOG_ERR "cannot seek codec %s\n", filepath LOG_END
            goto end;
        }
        nResult = fread(buf, 1, contentsize, f);
        if (nResult != contentsize)
        {
            errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
            LOG_ERR "cannot read codec %s %d %d\n", filepath,contentsize,nResult LOG_END
            goto end;
        }

        // hardware decompress
        ioctl(ITP_DEVICE_DECOMPRESS, ITP_IOCTL_INIT, NULL);

        if (write(ITP_DEVICE_DECOMPRESS, buf+8, contentsize) == contentsize)
        {
        //printf("[ITP Codec] ITP_DEVICE_DECOMPRESS 0x%x %ld %ld %d\n",ptr,len,headersize+8,contentsize);
            ret = read(ITP_DEVICE_DECOMPRESS, ptr, len);
        //printf("[ITP Codec] ITP_DEVICE_DECOMPRESS 0x%x %ld %ld %d\n",ptr,len,ret,contentsize);
        }

        ioctl(ITP_DEVICE_DECOMPRESS, ITP_IOCTL_EXIT, NULL);
    }
    else
    {
        if (fread(ptr, 1, len, f) != len)
        {
            errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
            LOG_ERR "cannot read codec %s\n", filepath LOG_END
            goto end;
        }
    }

end:
    free(buf);
    fclose(f);
    return ret;
}

static unsigned long CodecGetSize(int file)
{
    char filepath[32];
    FILE* f;
    uint32_t headersize, imagesize = 0;

    if (file >= ITH_COUNT_OF(codecTable))
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        return 0;
    }

    strcpy(filepath, CODEC_PATH);
    strcat(filepath, &codecTable[file][0]);

    strcat(filepath, ".codecs");
    f = fopen(filepath, "rb");
    if (!f)
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "cannot find codec %s\n", filepath LOG_END
        return 0;
    }

    if (fseek(f, HEADER_LEN_OFFSET, SEEK_SET))
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "cannot seek codec %s\n", filepath LOG_END
        goto end;
    }

    if (fread(&headersize, 1, sizeof (uint32_t), f) != sizeof (uint32_t))
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "cannot read codec %s\n", filepath LOG_END
        goto end;
    }
    headersize = itpBetoh32(headersize);

    if (fseek(f, headersize - 4, SEEK_SET))
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "cannot seek codec %s\n", filepath LOG_END
        goto end;
    }

    if (fread(&imagesize, 1, sizeof (uint32_t), f) != sizeof (uint32_t))
    {
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        LOG_ERR "cannot read codec %s\n", filepath LOG_END
        goto end;
    }
    imagesize = itpBetoh32(imagesize);

end:
    fclose(f);
    return imagesize;
}

static int CodecIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_GET_SIZE:
        *(unsigned long*)ptr = CodecGetSize(file);
        break;

    default:
        errno = (ITP_DEVICE_CODEC << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceCodec =
{
    ":codec",
    CodecOpen,
    CodecClose,
    CodecRead,
    itpWriteDefault,
    itpLseekDefault,
    CodecIoctl,
    NULL
};
