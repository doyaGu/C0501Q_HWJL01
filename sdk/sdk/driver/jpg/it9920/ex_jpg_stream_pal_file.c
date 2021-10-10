

#include "ite_jpg.h"
#include "jpg_stream.h"
#include "pal/pal.h"
#include "mem/mem.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define DEF_BS_BUF_LENGTH           (256 << 10)
#define DEF_BS_RING_BUF_LENGTH      (64 << 10)
#define DEF_ENC_SYS_BS_BUF_LENGTH   (256 << 10)
#define DEF_LINE_BUF_LENGTH         (952 << 10)

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_jpgPrintString(
    MMP_WCHAR   *string)
{
    MMP_INT i;
    for (i = 0; i < PalWcslen(string); i++)
        printf("%c", (char)string[i]);
    printf("\n");
}

static JPG_ERR
PalFile_open(
    JPG_STREAM_HANDLE   *pHJStream,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    if( pHJStream )
    {
        pHJStream->privData[0] = 0;

        // fopen
        switch( pHJStream->jStreamInfo.streamIOType )
        {
            case JPG_STREAM_IO_READ:
                pHJStream->privData[0] =
                    (void*)PalTFileOpen((MMP_WCHAR*)pHJStream->jStreamInfo.jstream.path, PAL_FILE_RB, 0);
                break;

            case JPG_STREAM_IO_WRITE:
                pHJStream->privData[0] =
                    (void*)PalTFileOpen((MMP_WCHAR*)pHJStream->jStreamInfo.jstream.path, PAL_FILE_WB, 0);
                break;
        }

        do{
            if( pHJStream->privData[0] == 0 )
            {
                printf(" open jpg fail !! %s[%d]\n ", __FILE__, __LINE__);
                _jpgPrintString((MMP_WCHAR*)pHJStream->jStreamInfo.jstream.path);
                break;
            }

            if( pHJStream->jStreamInfo.streamIOType == JPG_STREAM_IO_READ )
            {
                // get total file size
                pHJStream->curBsPos = 0;
                PalTFileSeek(pHJStream->privData[0], 0, PAL_SEEK_END, 0);
                pHJStream->streamSize = PalTFileTell(pHJStream->privData[0], 0);
                PalTFileSeek(pHJStream->privData[0], 0, PAL_SEEK_SET, 0);
            }

        }while(0);
    }

    return result;
}

static JPG_ERR
PalFile_close(
    JPG_STREAM_HANDLE   *pHJStream,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    if( pHJStream )
    {
        if( pHJStream->privData[0] )
        {
            PalTFileClose(pHJStream->privData[0], 0);
            pHJStream->privData[0] = 0;;
        }
    }

    return result;
}

static JPG_ERR
PalFile_seek(
    JPG_STREAM_HANDLE   *pHJStream,
    int                 offset,
    JPG_SEEK_TYPE       seekType,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    if( pHJStream )
    {
        if( pHJStream->privData[0] )
        {
            int         rst = 0;
            uint32_t    type = 0;

            switch( seekType )
            {
                case JPG_SEEK_SET:
                    type = PAL_SEEK_SET;
                    pHJStream->curBsPos = offset;
                    break;
                case JPG_SEEK_CUR:
                    type = PAL_SEEK_CUR;
                    pHJStream->curBsPos += offset;
                    break;
                case JPG_SEEK_END:
                    type = PAL_SEEK_END;
                    pHJStream->curBsPos = (pHJStream->streamSize - pHJStream->curBsPos);
                    break;
            }
            PalTFileSeek(pHJStream->privData[0], 0, type, 0);
            // if( rst )   printf(" seek fail !!");

            if( pHJStream->curBsPos > pHJStream->streamSize )
                pHJStream->curBsPos = pHJStream->streamSize;

            if( (int)pHJStream->curBsPos < 0 )
                pHJStream->curBsPos = 0;
        }
    }

    return result;
}

static JPG_ERR
PalFile_tell(
    JPG_STREAM_HANDLE   *pHJStream,
    uint32_t            *pCurPos,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    if( pHJStream )
    {
        if( pHJStream->privData[0] )
        {
            uint32_t    curr = 0;
            curr = PalTFileTell(pHJStream->privData[0], 0);
            if( pCurPos )       *pCurPos = curr;
        }
    }

    return result;
}

static JPG_ERR
PalFile_read(
    JPG_STREAM_HANDLE   *pHJStream,
    void                *srcBuf,
    uint32_t            requestSize,
    uint32_t            *realSize,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    if( pHJStream )
    {
        if( pHJStream->privData[0] )
        {
            uint32_t    size = 0;
            size = PalTFileRead(srcBuf, 1, requestSize, pHJStream->privData[0], 0);
            if( realSize )      *realSize = size;
            pHJStream->curBsPos += size;
        }
    }

    return result;
}

static JPG_ERR
PalFile_write(
    JPG_STREAM_HANDLE   *pHJStream,
    void                *destBuf,
    uint32_t            length,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    if( pHJStream )
    {
        if( pHJStream->privData[0] )
        {
            uint32_t    size = 0;
            size = PalTFileWrite(destBuf, 1, length, pHJStream->privData[0], 0);
            pHJStream->curBsPos += size;
        }
    }

    return result;
}

static JPG_ERR
PalFile_control(
    JPG_STREAM_HANDLE   *pHJStream,
    uint32_t            cmd,
    uint32_t            *value,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;

    if( pHJStream )
    {
        switch( cmd )
        {
            case JPG_STREAM_CMD_GET_BS_BUF_SIZE:
                if( value )     *value = DEF_BS_BUF_LENGTH;
                break;

            case JPG_STREAM_CMD_GET_LINE_BUF_SIZE:
                if( value )     *value = DEF_LINE_BUF_LENGTH;
                break;

            case JPG_STREAM_CMD_GET_BS_RING_BUF_SIZE:
                if( value )     *value = DEF_BS_RING_BUF_LENGTH;
                break;

            case JPG_STREAM_CMD_GET_ENC_SYS_BS_BUF_SIZE:
                if( value )     *value = DEF_ENC_SYS_BS_BUF_LENGTH;
                break;
        }
    }

    return result;
}

static void*
PalFile_HeapBuf(
    JPG_STREAM_HANDLE   *pHJStream,
    JPG_HEAP_TYPE       heapType,
    uint32_t            requestSize,
    uint32_t            *realSize)
{
    void    *ptr = 0;

    if( pHJStream )
    {
        switch( heapType )
        {
            case JPG_HEAP_BS_BUF:
                ptr = (void*)((((uint32_t)MEM_Deploy(MEM_DEPLOY_JPEG) + 3) >> 2) << 2);
                if( realSize )   *realSize = (ptr) ? requestSize : 0;
                break;

            case JPG_HEAP_LINE_BUF:
                ptr = (void*)((((uint32_t)MEM_Deploy(MEM_DEPLOY_JPEG) + 3) >> 2) << 2);
                ptr = (void*)((uint32_t)ptr + DEF_BS_BUF_LENGTH);
                if( realSize )   *realSize = (ptr) ? requestSize : 0;
                break;
            case JPG_HEAP_ENC_YUV_BUF:
            case JPG_HEAP_ENC_BS_RING_BUF:
            case JPG_HEAP_ENC_SYS_BS_BUF:
            case JPG_HEAP_DEF:
                ptr = PalHeapAlloc(PAL_HEAP_DEFAULT, requestSize);
                if( ptr && realSize )    *realSize = requestSize;
                break;
        }
    }

    return (void*)ptr;
}

static void
PalFile_FreeBuf(
    JPG_STREAM_HANDLE   *pHJStream,
    JPG_HEAP_TYPE       heapType,
    void                *ptr)
{
    if( pHJStream )
    {
        switch( heapType )
        {
            case JPG_HEAP_BS_BUF:        
            case JPG_HEAP_LINE_BUF:
                break;

            case JPG_HEAP_ENC_YUV_BUF:
            case JPG_HEAP_ENC_BS_RING_BUF:
            case JPG_HEAP_ENC_SYS_BS_BUF:
            case JPG_HEAP_DEF:
                if( ptr )   PalHeapFree(PAL_HEAP_DEFAULT, ptr);
                break;
        }
    }

    return;
}


static void*
PalMem_HeapBuf(
    JPG_STREAM_HANDLE   *pHJStream,
    JPG_HEAP_TYPE       heapType,
    uint32_t            requestSize,
    uint32_t            *realSize)
{
    void    *ptr = 0;

    if( pHJStream )
    {
        switch( heapType )
        {
            case JPG_HEAP_BS_BUF:
                ptr = pHJStream->privData[0];
                if( ptr && realSize )    *realSize = pHJStream->streamSize;
                break;

            case JPG_HEAP_LINE_BUF:
                ptr = (void*)((((uint32_t)MEM_Deploy(MEM_DEPLOY_JPEG) + 3) >> 2) << 2);
                if( realSize )  *realSize = (ptr) ? requestSize : 0;
                break;

            case JPG_HEAP_ENC_BS_RING_BUF:
            case JPG_HEAP_ENC_SYS_BS_BUF:
            case JPG_HEAP_DEF:
                ptr = PalHeapAlloc(PAL_HEAP_DEFAULT, requestSize);
                if( ptr && realSize )    *realSize = requestSize;
                break;
        }
    }

    return (void*)ptr;
}

static void
PalMem_FreeBuf(
    JPG_STREAM_HANDLE   *pHJStream,
    JPG_HEAP_TYPE       heapType,
    void                *ptr)
{
    if( pHJStream )
    {
        switch( heapType )
        {
            case JPG_HEAP_BS_BUF:
                pHJStream->privData[0] = 0;
                break;

            case JPG_HEAP_LINE_BUF:
                break;

            case JPG_HEAP_ENC_BS_RING_BUF:
            case JPG_HEAP_ENC_SYS_BS_BUF:
            case JPG_HEAP_DEF:
                if( ptr )   PalHeapFree(PAL_HEAP_DEFAULT, ptr);
                break;
        }
    }

    return;
}

static JPG_ERR
PalMem_control(
    JPG_STREAM_HANDLE   *pHJStream,
    uint32_t            cmd,
    uint32_t            *value,
    void                *extraData)
{
    JPG_ERR     result = JPG_ERR_OK;
    if( pHJStream )
    {
        switch( cmd )
        {
            case JPG_STREAM_CMD_GET_BS_BUF_SIZE:
                if( value )     *value = DEF_BS_BUF_LENGTH;
                break;

            case JPG_STREAM_CMD_GET_LINE_BUF_SIZE:
                if( value )     *value = DEF_LINE_BUF_LENGTH;
                break;

            case JPG_STREAM_CMD_GET_BS_RING_BUF_SIZE:
                if( value )     *value = DEF_BS_RING_BUF_LENGTH;
                break;

            case JPG_STREAM_CMD_GET_ENC_SYS_BS_BUF_SIZE:
                if( value )     *value = DEF_ENC_SYS_BS_BUF_LENGTH;
                break;
        }
    }

    return result;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================

JPG_STREAM_DESC jpg_stream_pal_file_desc =
{
    "jpg pal file stream",
    JPG_STREAM_CUSTOMER,

    PalFile_open,
    PalFile_close,
    PalFile_seek,
    PalFile_tell,
    PalFile_read,
    PalFile_write,
    PalFile_control,
    PalFile_HeapBuf,
    PalFile_FreeBuf,
};

JPG_STREAM_DESC jpg_stream_pal_mem_desc =
{
    "jpg pal mem stream",
    JPG_STREAM_CUSTOMER,

    0,
    0,
    0,
    0,
    0,
    0,
    PalMem_control,
    PalMem_HeapBuf,
    PalMem_FreeBuf,
};
