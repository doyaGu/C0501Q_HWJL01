#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include "ite/itc.h"
#include "itc_cfg.h"

int itcFileStreamClose(ITCStream* stream)
{
    int result = 0;
    ITCFileStream* fs = (ITCFileStream*) stream;
    assert(fs);

    if (fs->file)
    {
        result = fclose(fs->file);
        fs->file = NULL;
    }
    return result;
}

static int FileStreamRead(ITCStream* stream, void* buf, int size)
{
    ITCFileStream* fs = (ITCFileStream*) stream;
    assert(fs);

    return fread(buf, 1, size, fs->file);
}

static int FileStreamWrite(ITCStream* stream, const void* buf, int size)
{
    ITCFileStream* fs = (ITCFileStream*) stream;
    assert(fs);

    return fwrite(buf, 1, size, fs->file);
}

static int FileStreamSeek(ITCStream* stream, long offset, int origin)
{
    ITCFileStream* fs = (ITCFileStream*) stream;
    assert(fs);

    return fseek(fs->file, offset, origin);
}

static long FileStreamTell(ITCStream* stream)
{
    ITCFileStream* fs = (ITCFileStream*) stream;
    assert(fs);

    return ftell(fs->file);
}

static int FileStreamAvailable(ITCStream* stream)
{
    ITCFileStream* fs = (ITCFileStream*) stream;
    assert(fs);

    return stream->size - ftell(fs->file);
}

int itcFileStreamOpen(ITCFileStream* fstream, const char* filename, bool write)
{
    struct stat sb;

    assert(fstream);
    assert(filename);

    itcStreamOpen((ITCStream*) fstream);

    itcStreamSetClose(fstream, itcFileStreamClose);
    itcStreamSetRead(fstream, FileStreamRead);
    itcStreamSetWrite(fstream, FileStreamWrite);
    itcStreamSetSeek(fstream, FileStreamSeek);
    itcStreamSetTell(fstream, FileStreamTell);
    itcStreamSetAvailable(fstream, FileStreamAvailable);

    fstream->file = fopen(filename, write ? "wb" : "rb");
    if (!fstream->file)
    {
        LOG_ERR "itcFileStreamOpen %s failed\n", filename LOG_END
        return __LINE__;
    }

    if (fstat(fileno(fstream->file), &sb) == -1)
    {
        LOG_ERR "fstat %s failed\n", filename LOG_END
        return __LINE__;
    }

    fstream->stream.size = (int)sb.st_size;

    return 0;
}
