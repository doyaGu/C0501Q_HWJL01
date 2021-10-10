/*
 * buffered file I/O
 * Copyright (c) 2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/avstring.h"
#include "../avformat.h"
#include <fcntl.h>
#if HAVE_SETMODE
    #include <io.h>
#endif
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "../os_support.h"
#include "../url.h"
#include <io.h>

/* standard file protocol */

static int file_read(URLContext *h, unsigned char *buf, int size)
{
    int fd = (intptr_t) h->priv_data;
    int r  = _read(fd, buf, size);
    return (-1 == r) ? AVERROR(errno) : r;
}

static int file_write(URLContext *h, const unsigned char *buf, int size)
{
    int fd = (intptr_t) h->priv_data;
    int r  = _write(fd, buf, size);
    return (-1 == r) ? AVERROR(errno) : r;
}

static int file_get_handle(URLContext *h)
{
    return (intptr_t) h->priv_data;
}

static int file_check(URLContext *h, int mask)
{
    struct stat st;
    int         ret = stat(h->filename, &st);
    if (ret < 0)
    {
        return AVERROR(errno);
    }

    ret |= st.st_mode&S_IRUSR ? mask&AVIO_FLAG_READ  : 0;
    ret |= st.st_mode&S_IWUSR ? mask&AVIO_FLAG_WRITE : 0;

    return ret;
}

#if CONFIG_FILE_PROTOCOL

static int file_open(URLContext *h, const char *filename, int flags)
{
    int access;
    int fd;

    av_strstart(filename, "file:", &filename);

    if (flags & AVIO_FLAG_WRITE && flags & AVIO_FLAG_READ)
    {
        access = _O_CREAT | _O_TRUNC | _O_RDWR;
    }
    else if (flags & AVIO_FLAG_WRITE)
    {
        access = _O_CREAT | _O_TRUNC | _O_WRONLY;
    }
    else
    {
        access = _O_RDONLY;
    }

#ifdef O_BINARY
    access |= _O_BINARY;
#endif
    fd = _open(filename, access, 0666);
    if (fd == -1)
    {
        return AVERROR(errno);
    }

    h->priv_data = (void *) (intptr_t) fd;
    return 0;
}

/* XXX: use llseek */
static int64_t file_seek(URLContext *h, int64_t pos, int whence)
{
    #define MAX_AVSEEK_SIZE (0x7FFFFFFFL)
    int fd = (intptr_t) h->priv_data;
    if (whence == AVSEEK_SIZE)
    {
        struct stat st;
        int         ret = fstat(fd, &st);
        return ret < 0 ? AVERROR(errno) : st.st_size;
    }

    if (whence == SEEK_SET && pos >= MAX_AVSEEK_SIZE)
    {
        _lseek(fd, 0, whence);
        while(pos > MAX_AVSEEK_SIZE)
        {
            _lseek(fd, MAX_AVSEEK_SIZE, SEEK_CUR);
            pos -= MAX_AVSEEK_SIZE;
        }
        _lseek(fd, (int)pos, SEEK_CUR);
    }
    else
    {
        return _lseek(fd, pos, whence);
    }
}

static int file_close(URLContext *h)
{
    int fd = (intptr_t) h->priv_data;
    return _close(fd);
}

URLProtocol ff_file_protocol = {
    "file",          // const char *name;
    file_open,       // int (*url_open)(URLContext *h, const char *url, int flags);
    file_read,       // int (*url_read)(URLContext *h, unsigned char *buf, int size);
    file_write,      // int (*url_write)(URLContext *h, const unsigned char *buf, int size);
    file_seek,       // int64_t (*url_seek)(URLContext *h, int64_t pos, int whence);
    file_close,      // int (*url_close)(URLContext *h);
    NULL,            // struct URLProtocol *next;
    NULL,            // int (*url_read_pause)(URLContext *h, int pause);
    NULL,            // int64_t (*url_read_seek)(URLContext *h, int stream_index, int64_t timestamp, int flags);
    file_get_handle, // int (*url_get_file_handle)(URLContext *h);
    0,               // int priv_data_size;
    NULL,            // const AVClass *priv_data_class;
    0,               //AVIO_FLAG_READ_WRITE, // int flags;
    file_check,      // int (*url_check)(URLContext *h, int mask);
};

#endif     /* CONFIG_FILE_PROTOCOL */

#if CONFIG_PIPE_PROTOCOL

static int pipe_open(URLContext *h, const char *filename, int flags)
{
    int   fd;
    char *final;
    av_strstart(filename, "pipe:", &filename);

    fd = strtol(filename, &final, 10);
    if((filename == final) || *final )  /* No digits found, or something like 10ab */
    {
        if (flags & AVIO_FLAG_WRITE)
        {
            fd = 1;
        }
        else
        {
            fd = 0;
        }
    }

    #if HAVE_SETMODE
    setmode(fd, O_BINARY);
    #endif
    h->priv_data   = (void *) (intptr_t) fd;
    h->is_streamed = 1;
    return 0;
}

URLProtocol ff_pipe_protocol = {
    .name                = "pipe",
    .url_open            = pipe_open,
    .url_read            = file_read,
    .url_write           = file_write,
    .url_get_file_handle = file_get_handle,
    .url_check           = file_check,
};

#endif /* CONFIG_PIPE_PROTOCOL */