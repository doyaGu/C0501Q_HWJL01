#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "ite/itc.h"
#include "ite/ith.h"
#include "itc_cfg.h"

static int BufferStreamClose(ITCStream* stream)
{
    int result;
    ITCBufferStream* bs = (ITCBufferStream*) stream;
    assert(bs);


    result = pthread_mutex_lock(&bs->mutex);
    if (result)
    {
        LOG_ERR "pthread_mutex_lock fail: %d\n", result LOG_END
        return result;
    }

    free(bs->buf);
    bs->buf = NULL;


    result = pthread_mutex_destroy(&bs->mutex);
    if (result)
    {
        LOG_ERR "pthread_mutex_destroy fail: %d\n", result LOG_END
    }
    return 0;
}

static int BufferStreamRead(ITCStream* stream, void* buf, int size)
{
    int result, readsize = 0;
    ITCBufferStream* bs = (ITCBufferStream*) stream;
    assert(bs);

    result = pthread_mutex_lock(&bs->mutex);
    if (result)
    {
        LOG_ERR "pthread_mutex_lock fail: %d\n", result LOG_END
        return 0;
    }

    if (stream->eof)
        goto end;

    if (bs->readpos == bs->size)
        bs->readpos = 0;

    if (bs->readpos < bs->writepos)
    {
        readsize = ITH_MIN(size, bs->writepos - bs->readpos); 
        memcpy(buf, &bs->buf[bs->readpos], readsize);

        bs->readpos += readsize;
    }
    else // if (bs->readpos >= bs->writepos)
    {
        if (size > bs->size - bs->readpos)
        {
            int size1, size2;
            
            size1 = bs->size - bs->readpos; 
            memcpy(buf, &bs->buf[bs->readpos], size1);

            size -= size1;
            bs->readpos = 0;

            size2 = ITH_MIN(size, bs->writepos - bs->readpos); 
            memcpy(((unsigned char*) buf) + size1, &bs->buf[bs->readpos], size2);

            bs->readpos += size2;
            readsize = size1 + size2;
        }
        else // if (size <= bs->size - bs->readpos)
        {
            readsize = size; 
            memcpy(&bs->buf[bs->readpos], buf, readsize);

            bs->readpos += readsize;
        }
    }

    if (readsize > 0 && bs->readpos == bs->writepos)
        stream->eof = true;

end:

    result = pthread_mutex_unlock(&bs->mutex);
    if (result)
    {
        LOG_ERR "pthread_mutex_unlock fail: %d\n", result LOG_END
    }
    return readsize;
}

static int BufferStreamWrite(ITCStream* stream, const void* buf, int size)
{
    int result, writesize = 0;
    ITCBufferStream* bs = (ITCBufferStream*) stream;
    assert(bs);

    result = pthread_mutex_lock(&bs->mutex);
    if (result)
    {
        LOG_ERR "pthread_mutex_lock fail: %d\n", result LOG_END
        return 0;
    }

    if (bs->writepos < bs->readpos)
    {
        writesize = ITH_MIN(size, bs->readpos - bs->writepos); 
        memcpy(&bs->buf[bs->writepos], buf, writesize);

        bs->writepos += writesize;
    }
    else // if (bs->writepos >= bs->readpos)
    {
        if (size > bs->size - bs->writepos)
        {
            int size1, size2;
            
            size1 = bs->size - bs->writepos; 
            memcpy(&bs->buf[bs->writepos], buf, size1);

            size -= size1;
            bs->writepos = 0;

            size2 = ITH_MIN(size, bs->readpos - bs->writepos); 
            memcpy(&bs->buf[bs->writepos], ((unsigned char*) buf) + size1, size2);

            bs->writepos += size2;
            writesize = size1 + size2;
        }
        else // if (size <= bs->size - bs->writepos)
        {
            writesize = size; 
            memcpy(&bs->buf[bs->writepos], buf, writesize);

            bs->writepos += writesize;
        }
    }

    if (writesize > 0)
        stream->eof = false;

    result = pthread_mutex_unlock(&bs->mutex);
    if (result)
    {
        LOG_ERR "pthread_mutex_unlock fail: %d\n", result LOG_END
    }
    return writesize;
}

static int BufferStreamSeek(ITCStream* stream, long offset, int origin)
{
    int pos, result = 0;
    ITCBufferStream* bs = (ITCBufferStream*) stream;
    assert(bs);

    result = pthread_mutex_lock(&bs->mutex);
    if (result)
    {
        LOG_ERR "pthread_mutex_lock fail: %d\n", result LOG_END
        return 0;
    }

    switch (origin)
    {
    case SEEK_SET:
        if (bs->readpos < bs->writepos)
        {
            if (offset <= bs->writepos)
            {
                bs->readpos = offset;
                stream->eof = (bs->readpos == bs->writepos) ? true : false;
            }
            else
                result = -1;
        }
        else // if (bs->readpos >= bs->writepos)
        {
            if (offset < bs->size)
            {
                bs->readpos = offset;
            }
            else
                result = -1;
        }
        break;

    case SEEK_CUR:
        pos = bs->readpos + offset;
        if (bs->readpos < bs->writepos)
        {
            if (pos >= 0 && pos < bs->writepos)
            {
                bs->readpos = pos;
                stream->eof = (bs->readpos == bs->writepos) ? true : false;
            }
            else
                result = -1;
        }
        else // if (bs->readpos >= bs->writepos)
        {
            if (pos >= bs->writepos && pos < bs->size)
            {
                bs->readpos = pos;
            }
            else
                result = -1;
        }
        break;

    case SEEK_END:
        pos = bs->size + offset;
        if (bs->readpos < bs->writepos)
        {
            if (pos >= 0 && pos < bs->writepos)
            {
                bs->readpos = pos;
                stream->eof = (bs->readpos == bs->writepos) ? true : false;
            }
            else
                result = -1;
        }
        else // if (bs->readpos >= bs->writepos)
        {
            if (pos >= bs->writepos && pos < bs->size)
            {
                bs->readpos = pos;
            }
            else
                result = -1;
        }
        break;

    default:
        assert(0);
        result = -1;
        break;
    }

    result = pthread_mutex_unlock(&bs->mutex);
    if (result)
    {
        LOG_ERR "pthread_mutex_unlock fail: %d\n", result LOG_END
    }
    return result;
}

static long BufferStreamTell(ITCStream* stream)
{
    ITCBufferStream* bs = (ITCBufferStream*) stream;
    assert(bs);

    return bs->readpos;
}

static int BufferStreamAvailable(ITCStream* stream)
{
    int result, avail = 0;
    ITCBufferStream* bs = (ITCBufferStream*) stream;
    assert(bs);

    result = pthread_mutex_lock(&bs->mutex);
    if (result)
    {
        LOG_ERR "pthread_mutex_lock fail: %d\n", result LOG_END
        return 0;
    }

    if (stream->eof)
        goto end;

    if (bs->readpos < bs->writepos)
    {
        avail = bs->writepos - bs->readpos; 
    }
    else // if (bs->readpos >= bs->writepos)
    {
        avail = bs->size - bs->readpos;
    }

end:
    result = pthread_mutex_unlock(&bs->mutex);
    if (result)
    {
        LOG_ERR "pthread_mutex_unlock fail: %d\n", result LOG_END
    }
    return avail;
}

static int BufferStreamReadLock(ITCStream* stream, void** bufptr, int size)
{
    int result, readsize = 0;
    ITCBufferStream* bs = (ITCBufferStream*) stream;
    assert(bufptr);
    assert(bs);

    result = pthread_mutex_lock(&bs->mutex);
    if (result)
    {
        LOG_ERR "pthread_mutex_lock fail: %d\n", result LOG_END
        return ITC_LOCK_FAIL;
    }

    if (stream->eof)
        goto end;

    if (bs->readpos == bs->size)
        bs->readpos = 0;

    if (bs->readpos < bs->writepos)
    {
        readsize = ITH_MIN(size, bs->writepos - bs->readpos); 
    }
    else // if (bs->readpos >= bs->writepos)
    {
        readsize = ITH_MIN(size, bs->size - bs->readpos); 
    }

    *bufptr = (void*) &bs->buf[bs->readpos];

end:
    return readsize;
}

static void BufferStreamReadUnlock(ITCStream* stream, int size)
{
    int result, readsize = 0;
    ITCBufferStream* bs = (ITCBufferStream*) stream;
    assert(bs);

    if (stream->eof)
        goto end;

    if (bs->readpos == bs->size && bs->writepos > 0)
        bs->readpos = 0;

    if (bs->readpos < bs->writepos)
    {
        readsize = ITH_MIN(size, bs->writepos - bs->readpos); 
    }
    else // if (bs->readpos >= bs->writepos)
    {
        readsize = ITH_MIN(size, bs->size - bs->readpos); 
    }

    bs->readpos += readsize;

    if (bs->readpos == bs->size && bs->writepos > 0)
        bs->readpos = 0;

    if (readsize > 0 && bs->readpos == bs->writepos)
        stream->eof = true;

end:
    result = pthread_mutex_unlock(&bs->mutex);
    if (result)
    {
        LOG_ERR "pthread_mutex_unlock fail: %d\n", result LOG_END
    }
}

static int BufferStreamWriteLock(ITCStream* stream, void** bufptr, int size)
{
    int result, writesize = 0;
    ITCBufferStream* bs = (ITCBufferStream*) stream;
    assert(bufptr);
    assert(bs);

    result = pthread_mutex_lock(&bs->mutex);
    if (result)
    {
        LOG_ERR "pthread_mutex_lock fail: %d\n", result LOG_END
        return ITC_LOCK_FAIL;
    }

    *bufptr = (void*) &bs->buf[bs->writepos];

    if (bs->writepos < bs->readpos)
    {
        writesize = ITH_MIN(size, bs->readpos - bs->writepos); 
    }
    else // if (bs->writepos >= bs->readpos)
    {
        if (size > bs->size - bs->writepos)
        {
            writesize = bs->size - bs->writepos; 
        }
        else // if (size <= bs->size - bs->writepos)
        {
            writesize = size; 
        }
    }

    return writesize;
}

static void BufferStreamWriteUnlock(ITCStream* stream, int size)
{
    int result, writesize = 0;
    ITCBufferStream* bs = (ITCBufferStream*) stream;
    assert(bs);

    if (bs->writepos < bs->readpos)
    {
        writesize = ITH_MIN(size, bs->readpos - bs->writepos); 
        bs->writepos += writesize;
    }
    else // if (bs->writepos >= bs->readpos)
    {
        if (size > bs->size - bs->writepos)
        {
            writesize = bs->size - bs->writepos;
            if (bs->readpos > 0)
                bs->writepos = 0;
        }
        else // if (size <= bs->size - bs->writepos)
        {
            writesize = size; 
            bs->writepos += writesize;
        }
    }

    if (bs->writepos == bs->size && bs->readpos > 0)
        bs->writepos = 0;

    if (writesize > 0)
        stream->eof = false;

    result = pthread_mutex_unlock(&bs->mutex);
    if (result)
    {
        LOG_ERR "pthread_mutex_unlock fail: %d\n", result LOG_END
    }
}

int itcBufferStreamOpen(ITCBufferStream* bstream, int size)
{
    int result = 0;
    assert(bstream);
    assert(size > 0);

    itcStreamOpen((ITCStream*) bstream);

    itcStreamSetClose(bstream, BufferStreamClose);
    itcStreamSetRead(bstream, BufferStreamRead);
    itcStreamSetWrite(bstream, BufferStreamWrite);
    itcStreamSetSeek(bstream, BufferStreamSeek);
    itcStreamSetTell(bstream, BufferStreamTell);
    itcStreamSetAvailable(bstream, BufferStreamAvailable);
    itcStreamSetReadLock(bstream, BufferStreamReadLock);
    itcStreamSetReadUnlock(bstream, BufferStreamReadUnlock);
    itcStreamSetWriteLock(bstream, BufferStreamWriteLock);
    itcStreamSetWriteUnlock(bstream, BufferStreamWriteUnlock);

    bstream->buf = malloc(size);
    if (!bstream->buf)
    {
        LOG_ERR "Out of memory: %d\n", size LOG_END
        result = __LINE__;
        goto end;
    }

    result = pthread_mutex_init(&bstream->mutex, NULL);
    if (result)
    {
        LOG_ERR "pthread_mutex_init fail: %d\n", result LOG_END
        goto end;
    }
    bstream->size = size;
    bstream->readpos = bstream->writepos = 0;
    ((ITCStream*)bstream)->eof = true;

end:
    return result;
}
