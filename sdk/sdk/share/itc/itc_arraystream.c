#include <assert.h>
#include <string.h>
#include "ite/itc.h"
#include "itc_cfg.h"

static int ArrayStreamRead(ITCStream* stream, void* buf, int size)
{
    int result;
    unsigned char* outbuf = (unsigned char*) buf;
    ITCArrayStream* as = (ITCArrayStream*) stream;
    assert(outbuf);
    assert(as);

    result = as->stream.size - as->pos > size ? size : as->stream.size - as->pos;
    memcpy(outbuf, &as->array[as->pos], result);
    as->pos += result;

    return result;
}

static int ArrayStreamSeek(ITCStream* stream, long offset, int origin)
{
    int pos, result = 0;
    ITCArrayStream* as = (ITCArrayStream*) stream;
    assert(as);

    switch (origin)
    {
    case SEEK_SET:
        if (offset < as->stream.size)
        {
            as->pos = offset;
        }
        else
            result = -1;

        break;

    case SEEK_CUR:
        pos = as->pos + offset;
        if (pos >= 0 && pos < as->stream.size)
        {
            as->pos = pos;
        }
        else
            result = -1;

        break;

    case SEEK_END:
        pos = as->stream.size + offset;
        if (pos >= 0 && pos < as->stream.size)
        {
            as->pos = pos;
        }
        else
            result = -1;

        break;

    default:
        assert(0);
        result = -1;
        break;
    }

    return result;
}

static long ArrayStreamTell(ITCStream* stream)
{
    ITCArrayStream* as = (ITCArrayStream*) stream;
    assert(as);

    return as->pos;
}

static int ArrayStreamAvailable(ITCStream* stream)
{
    ITCArrayStream* as = (ITCArrayStream*) stream;
    assert(as);

    return as->stream.size - as->pos;
}

static int ArrayStreamReadLock(ITCStream* stream, void** bufptr, int size)
{
    int result;
    ITCArrayStream* as = (ITCArrayStream*) stream;
    assert(bufptr);
    assert(as);

    result = as->stream.size - as->pos > size ? size : as->stream.size - as->pos;
    *bufptr = (void*) &as->array[as->pos];

    return result;
}

static void ArrayStreamReadUnlock(ITCStream* stream, int size)
{
    int result;
    ITCArrayStream* as = (ITCArrayStream*) stream;
    assert(as);

    result = as->stream.size - as->pos > size ? size : as->stream.size - as->pos;
    as->pos += result;
}

int itcArrayStreamOpen(ITCArrayStream* astream, const char* array, int size)
{
    assert(astream);
    assert(array);
    assert(size > 0);

    itcStreamOpen((ITCStream*) astream);

    itcStreamSetRead(astream, ArrayStreamRead);
    itcStreamSetSeek(astream, ArrayStreamSeek);
    itcStreamSetTell(astream, ArrayStreamTell);
    itcStreamSetAvailable(astream, ArrayStreamAvailable);
    itcStreamSetReadLock(astream, ArrayStreamReadLock);
    itcStreamSetReadUnlock(astream, ArrayStreamReadUnlock);

    astream->stream.size        = size;
    astream->array              = array;
    astream->pos                = 0;

    return 0;
}
