
#include "io_api.h"
//=============================================================================
//				  Constant Definition
//=============================================================================


//=============================================================================
//				  Macro Definition
//=============================================================================


//=============================================================================
//				  Structure Definition
//=============================================================================


//=============================================================================
//				  Global Data Definition
//=============================================================================


//=============================================================================
//				  Private Function Definition
//=============================================================================

extern IO_DESC  file_io;
extern IO_DESC  mem_io;

//=============================================================================
//				  Public Function Definition
//=============================================================================
IO_MGR*
ioA_Create_Stream(
    IO_TYPE     srcType,
    void        *extraData)
{
    IO_MGR    *pIoMgr = (IO_MGR*)malloc(sizeof(IO_MGR));

    if( pIoMgr )
    {
        memset((void*)pIoMgr, 0, sizeof(IO_MGR));

        pIoMgr->ioType = srcType;

        switch( srcType )
        {
            case IO_FILE_RB:
                pIoMgr->ioDesc = &file_io;
                break;
                
            case IO_FILE_WB:
                pIoMgr->ioDesc = &file_io;
                break;

            case IO_MEM_RB:
                // data = input buf addr
                pIoMgr->ioDesc = &mem_io;
                break;
                
            case IO_MEM_WB:
                // data = input buf addr
                pIoMgr->ioDesc = &mem_io;
                break;
        }
    }

    return pIoMgr; 
}

void
ioA_Destroy_Stream(
    IO_MGR    *pIoMgr)
{
    if( pIoMgr )
    {
        if( pIoMgr->privData )
        {
            free(pIoMgr->privData);
            pIoMgr->privData = 0;
        }
        
        free(pIoMgr);
    }
}

IO_ERR
ioA_Stream_Open(
    IO_MGR    *pIoMgr,
    void      *extraData)
{
    IO_ERR         result = IO_ERR_OK;

    if( pIoMgr && pIoMgr->ioDesc->open_stream )
    {
        result = pIoMgr->ioDesc->open_stream(pIoMgr, extraData);
    }

    return result;
}

void
ioA_Stream_Close(
    IO_MGR    *pIoMgr)
{
    if( pIoMgr && pIoMgr->ioDesc->close_stream )
    {
        pIoMgr->ioDesc->close_stream(pIoMgr);
    }
}

int
ioA_Stream_FillBuf(
    IO_MGR          *pIoMgr,
    void            *destBuf, 
    unsigned int    size,
    unsigned int    count)
{
    int           length = 0;

    if( pIoMgr && pIoMgr->ioDesc->fill_buf )
    {
        length = pIoMgr->ioDesc->fill_buf(pIoMgr, destBuf, size, count);
    }

    return length;
}

int
ioA_Stream_OutputBuf(
    IO_MGR          *pIoMgr,
    void            *srcBuf, 
    unsigned int    size,
    unsigned int    count)
{
    int           length = 0;

    if( pIoMgr && pIoMgr->ioDesc->output_buf )
    {
        length = pIoMgr->ioDesc->output_buf(pIoMgr, srcBuf, size, count);
    }
    
    return length;
}

int
ioA_Stream_SeekData(
    IO_MGR          *pIoMgr,
    int             offset, 
    IO_SEEK_TYPE    type)
{
    int           length = 0;

    if( pIoMgr && pIoMgr->ioDesc->seek_data )
    {
        length = pIoMgr->ioDesc->seek_data(pIoMgr, offset, type);
    }
    
    return length;
}

unsigned int
ioA_Stream_Ctrl(
    IO_MGR      *pIoMgr,
    IO_API_CMD  cmd)
{
    unsigned int           result = 0;

    if( pIoMgr && pIoMgr->ioDesc->control )
    {
        result = pIoMgr->ioDesc->control(pIoMgr, cmd);
    }

    return result;
}
