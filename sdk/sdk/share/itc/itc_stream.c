#include <assert.h>
#include "ite/itp.h"
#include "ite/itc.h"

static int StreamClose(struct ITCStreamTag* stream)
{
    assert(stream);
    return 0;
}

static int StreamRead(struct ITCStreamTag* stream, void* buf, int size)
{
    assert(stream);
    return 0;
}

static int StreamWrite(struct ITCStreamTag* stream, const void* buf, int size)
{
    assert(stream);
    return 0;
}

static int StreamSeek(struct ITCStreamTag* stream, long offset, int origin)
{
    assert(stream);
    return 0;
}

static long StreamTell(struct ITCStreamTag* stream)
{
    assert(stream);
    return 0;
}

static int StreamAvailable(struct ITCStreamTag* stream)
{
    assert(stream);
    return 0;
}

void itcStreamOpen(ITCStream* stream)
{
    assert(stream);

    stream->eof         = false;
    stream->size        = 0;
    stream->Close       = StreamClose;
    stream->Read        = StreamRead;
    stream->Write       = StreamWrite;
    stream->Seek        = StreamSeek;
    stream->Tell        = StreamTell;
    stream->Available   = StreamAvailable;
    stream->ReadLock    = NULL;
    stream->ReadUnlock  = NULL;
    stream->WriteLock   = NULL;
    stream->WriteUnlock = NULL;
}
