#include <sys/stat.h>
#include <sys/statvfs.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include "ite/ug.h"
#include "ug_cfg.h"

#pragma pack(1)
typedef struct
{
    uint32_t filename_size;
} file_t;

static const uint32_t enckey = CFG_UPGRADE_ENC_KEY;

static size_t enc_fread(void *buffer, size_t size, size_t count, ITCStream *stream)
{
    if (enckey == 0)
    {
        return itcStreamRead(stream, buffer, size * count);
    }
    else
    {
        uint8_t* ptr = (uint8_t*)buffer;
        size_t ret, i, bufsize, alignsize;
        uint32_t key32;
        uint8_t* buf;

        bufsize = size * count;

        buf = malloc(bufsize);
        if (buf == NULL)
        {
            LOG_ERR "Out of memory: %ld\n", bufsize LOG_END
            return 0;
        }

        alignsize = bufsize & ~(sizeof(uint32_t) - 1);

        key32 = (((uint32_t) enckey) << 24) |
            (((uint32_t) enckey) << 16) |
            (((uint32_t) enckey) << 8) |
            ((uint32_t) enckey);

        ret = itcStreamRead(stream, buf, size * count);

        for (i = 0; i < alignsize; i += sizeof(uint32_t))
            *(uint32_t*)&ptr[i] = *(uint32_t*)&buf[i] ^ key32;

        for (; i < bufsize; i++)
            ptr[i] = buf[i] ^ enckey;

        free(buf);

        return ret;
    }
}

int ugUpgradeFile(ITCStream *f, char* drive)
{
    int ret = 0;
    file_t file;
    char filename[PATH_MAX];
    char* buf = NULL;
    char* buf2 = NULL;
    uint32_t filesize, bufsize, readsize, size2, remainsize;
    FILE* outfile = NULL;
    struct statvfs info;

    // read file
    readsize = itcStreamRead(f, &file, sizeof(file_t));
    if (readsize != sizeof(file_t))
    {
        ret = __LINE__;
        LOG_ERR "Cannot read file: %ld != %ld\n", readsize, sizeof(file_t) LOG_END
        goto end;
    }

    file.filename_size = itpLetoh32(file.filename_size);
    LOG_DBG "filename_size=%ld\n", file.filename_size LOG_END

    strcpy(filename, drive);

    readsize = itcStreamRead(f, &filename[2], file.filename_size);
    if (readsize != file.filename_size)
    {
        ret = __LINE__;
        LOG_ERR "Cannot read file: %ld != %ld\n", readsize, file.filename_size LOG_END
        goto end;
    }

    readsize = itcStreamRead(f, &filesize, sizeof (uint32_t));
    if (readsize != sizeof (uint32_t))
    {
        ret = __LINE__;
        LOG_ERR "Cannot read file: %ld != %ld\n", readsize, sizeof (uint32_t) LOG_END
        goto end;
    }

    filesize = itpLetoh32(filesize);

    bufsize = CFG_UG_BUF_SIZE < filesize ? CFG_UG_BUF_SIZE : filesize;
    buf = malloc(bufsize);
    if (!buf)
    {
        ret = __LINE__;
        LOG_ERR "Out of memory: %ld\n", bufsize LOG_END
        goto end;
    }

    // check capacity
    if (statvfs(drive, &info) == 0)
    {
        uint64_t avail = info.f_bfree * info.f_bsize;
        struct stat buffer;

        if (stat(filename, &buffer) == 0)
            avail += buffer.st_size;

        if (filesize > avail)
        {
            ret = __LINE__;
            LOG_ERR "Out of space: %d > %d\n", filesize, avail LOG_END
            goto end;
        }
    }

    // upgrade
    outfile = fopen(filename, "wb");
    if (!outfile)
    {
        ret = __LINE__;
        LOG_ERR "Cannot open file %s: %d\n", filename, errno LOG_END
        goto end;
    }

    LOG_INFO "[%d%%] Writing %s, %ld bytes", ugGetProrgessPercentage(), filename, filesize LOG_END

    remainsize = filesize;
    do
    {
        readsize = (remainsize < bufsize) ? remainsize : bufsize;
        size2 = enc_fread(buf, 1, readsize, f);
        if (size2 != readsize)
        {
            ret = __LINE__;
            LOG_ERR "Cannot read file: %ld != %ld\n", size2, readsize LOG_END
            goto end;
        }

        size2 = fwrite(buf, 1, readsize, outfile);
        if (size2 != readsize)
        {
            ret = __LINE__;
            LOG_ERR "Cannot write file: %ld != %ld\n", size2, readsize LOG_END
            goto end;
        }

        remainsize -= readsize;

        putchar('.');
        fflush(stdout);
    }
    while (remainsize > 0);

    putchar('\n');

    // verify
    fclose(outfile);
    outfile = NULL;

    buf2 = malloc(bufsize);
    if (!buf2)
    {
        ret = __LINE__;
        LOG_ERR "Out of memory: %ld\n", bufsize LOG_END
        goto end;
    }

    outfile = fopen(filename, "rb");
    if (!outfile)
    {
        ret = __LINE__;
        LOG_ERR "Cannot open file %s: %d\n", filename, errno LOG_END
        goto end;
    }

    if (itcStreamSeek(f, -(long)filesize, SEEK_CUR))
    {
        ret = __LINE__;
        LOG_ERR "Cannot seek file %d\n", errno LOG_END
        goto end;
    }

    LOG_INFO "[%d%%] Verifying", ugGetProrgessPercentage() LOG_END

    remainsize = filesize;
    do
    {
        readsize = (remainsize < bufsize) ? remainsize : bufsize;
        size2 = enc_fread(buf, 1, readsize, f);
        if (size2 != readsize)
        {
            ret = __LINE__;
            LOG_ERR "Cannot read file: %ld != %ld\n", size2, readsize LOG_END
            goto end;
        }

        size2 = fread(buf2, 1, readsize, outfile);
        if (size2 != readsize)
        {
            ret = __LINE__;
            LOG_ERR "Cannot read file: %ld != %ld\n", size2, readsize LOG_END
            goto end;
        }

        if (memcmp(buf, buf2, readsize))
        {
            ret = __LINE__;
            LOG_ERR "\nVerify failed.\n" LOG_END
            goto end;
        }

        remainsize -= readsize;

        putchar('.');
        fflush(stdout);
    }
    while (remainsize > 0);
    putchar('\n');
    
end:
    if (outfile)
        fclose(outfile);

    if (buf2)
        free(buf2);

    if (buf)
        free(buf);

    return ret;
}
