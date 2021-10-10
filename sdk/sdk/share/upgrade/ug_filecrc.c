#include <malloc.h>
#include <string.h>
#include "ite/ug.h"
#include "ug_cfg.h"

#define MAX_CRC_ITEM (100)

static uint32_t ugGetCrcValue(uint8_t* ptr)
{
    uint32_t val = ((uint32_t)ptr[3] << 24) | ((uint32_t)ptr[2] << 16) | ((uint32_t)ptr[1] << 8) | ptr[0];
    return val;
}

static void ugSetCrcValue(uint8_t* ptr, uint32_t crc)
{
    ptr[3] = (uint8_t)((crc >> 24) & 0xFF);
    ptr[2] = (uint8_t)((crc >> 16) & 0xFF);
    ptr[1] = (uint8_t)((crc >> 8) & 0xFF);
    ptr[0] = (uint8_t)(crc & 0xFF);
}

int ugCheckFilesCrc(char* path, char* crcpath)
{
    int ret;
    ITCFileStream fileStream;
    uint8_t*ptr, *buf = NULL;
    int size;

    ret = itcFileStreamOpen(&fileStream, crcpath, false);
    if (ret)
    {
        LOG_ERR "Cannot open file %s\n", crcpath LOG_END
        return ret;
    }

    size = fileStream.stream.size;
    buf = malloc(size);
    if (!buf)
    {
        ret = UG_OUT_OF_MEM;
        LOG_ERR "Out of memory: %d\n", size LOG_END
        goto end;
    }

    // check crc file itself
    ret = ugCheckCrc(&fileStream.stream, buf);
    if (ret)
    {
        LOG_ERR "Check file %s CRC failed.\n", crcpath LOG_END
        goto end;
    }

    itcStreamClose(&fileStream);

    // check each file's crc in crc file
    ptr = buf;
    size -= 4;
    while (ptr != &buf[size])
    {
        char pathbuf[PATH_MAX];
        int n;
        uint32_t crcval, crc = 0;

        strcpy(pathbuf, path);
        strcat(pathbuf, ptr);

        n = strlen(ptr) + 1;
        crcval = ugGetCrcValue(&ptr[n]);

        ret = ugCalcFileCrc(pathbuf, &crc);
        if (ret)
            goto end;

        if (crcval != crc)
        {
            ret = __LINE__;
            LOG_ERR "%s CRC incorrect: 0x%X != 0x%X\n", pathbuf, crcval, crc LOG_END
            goto end;
        }
        ptr += n + 4;
    }

end:
    free(buf);

    if (ret == 0)
    {
        LOG_INFO "CRC file check success.\n" LOG_END
    }
    return ret;
}

int ugSetFileCrc(char* filepath, char* path, char* crcpath)
{
    int ret, size, bufsize, filesize;
    ITCFileStream fileStream;
    uint8_t *ptr, *buf = NULL;
    uint32_t crc = 0;
    char pathbuf[PATH_MAX];

    // read all crc values to buffer
    ret = itcFileStreamOpen(&fileStream, crcpath, false);
    if (ret)
    {
        LOG_ERR "Cannot open file %s\n", crcpath LOG_END
        return ret;
    }

    size = fileStream.stream.size;
    bufsize = size + strlen(filepath) + 1 + 4;
    buf = malloc(bufsize);
    if (!buf)
    {
        ret = UG_OUT_OF_MEM;
        LOG_ERR "Out of memory: %d\n", size LOG_END
        goto end;
    }

    // check crc file itself
    ret = ugCheckCrc(&fileStream.stream, buf);
    if (ret)
    {
        LOG_ERR "Check file %s CRC failed.\n", crcpath LOG_END
        goto end;
    }

    itcStreamClose(&fileStream);

    // calc file crc
    strcpy(pathbuf, path);
    strcat(pathbuf, filepath);
    ret = ugCalcFileCrc(pathbuf, &crc);
    if (ret)
        goto end;

    // looking for existing crc value
    ptr = buf;
    size -= 4;
    while (ptr != &buf[size])
    {
        int n = strlen(ptr) + 1;

        if (strcmp(filepath, ptr) == 0)
        {
            ugSetCrcValue(&ptr[n], crc);
            filesize = size + 4;
            break;
        }
        ptr += n + 4;
    }

    // create new one if not exist
    if (ptr == &buf[size])
    {
        int n = strlen(filepath) + 1;
        strcpy(ptr, filepath);
        ugSetCrcValue(&ptr[n], crc);
        filesize = bufsize;
    }

    // update file's crc itself
    crc = ugCalcBufferCrc(buf, filesize - 4);
    ptr = &buf[filesize - 4];
    ugSetCrcValue(ptr, crc);

    // write updated crc to file
    ret = itcFileStreamOpen(&fileStream, crcpath, true);
    if (ret)
    {
        free(buf);
        LOG_ERR "Cannot open file %s\n", crcpath LOG_END
        return ret;
    }

    size = itcStreamWrite(&fileStream, buf, filesize);
    if (size != filesize)
    {
        ret = __LINE__;
        LOG_ERR "Cannot write file: %ld != %ld\n", size, bufsize LOG_END
        goto end;
    }

end:
    free(buf);
    itcStreamClose(&fileStream);
    return ret;
}

int ugUpgradeFilesCrc(char* path, char* crcpath)
{
    int ret;
    ITCFileStream fileStream;
    uint8_t*ptr, *buf = NULL;
    int size;

    ret = itcFileStreamOpen(&fileStream, crcpath, false);
    if (ret)
    {
        LOG_ERR "Cannot open file %s\n", crcpath LOG_END
        return ret;
    }

    size = fileStream.stream.size;
    buf = malloc(size);
    if (!buf)
    {
        ret = UG_OUT_OF_MEM;
        LOG_ERR "Out of memory: %d\n", size LOG_END
        goto end;
    }

    // check crc file itself
    ret = ugCheckCrc(&fileStream.stream, buf);
    if (ret)
    {
        LOG_ERR "Check file %s CRC failed.\n", crcpath LOG_END
        goto end;
    }

    itcStreamClose(&fileStream);

    // check each file's crc in crc file
    ptr = buf;
    size -= 4;
    while (ptr != &buf[size])
    {
        char pathbuf[PATH_MAX];
        uint32_t crc = 0;
        int n = strlen(ptr) + 1;

        strcpy(pathbuf, path);
        strcat(pathbuf, ptr);

        ret = ugCalcFileCrc(pathbuf, &crc);
        if (ret)
            goto end;

        ugSetCrcValue(&ptr[n], crc);

        ptr += n + 4;
    }

end:
    free(buf);

    if (ret == 0)
    {
        LOG_INFO "CRC file check success.\n" LOG_END
    }
    return ret;
}
