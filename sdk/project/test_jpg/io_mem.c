
#include "globle.h"
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
IO_ERR
Mem_open(
    IO_MGR  *ioMgr, 
    void    *extraData)
{
    IO_ERR     result = IO_ERR_OK;
    BASE_STRM_INFO  *pStrmInfo = (BASE_STRM_INFO*)extraData;

    if( ioMgr )
    {
        switch( ioMgr->ioType )
        {
            case IO_MEM_RB:
                ioMgr->pos   = 0;
                ioMgr->HFile = pStrmInfo->u.strm.startAddr;
                ioMgr->length = pStrmInfo->u.strm.length;
                break;

            case IO_MEM_WB:
                ioMgr->pos   = 0;
                ioMgr->HFile = pStrmInfo->u.strm.startAddr;
                ioMgr->length = pStrmInfo->u.strm.length;
                break;                
        }

    }
    else
        result = IO_ERR_NULL_POINTER;

    return result;
}

IO_ERR
Mem_close(
    IO_MGR  *ioMgr)
{
    IO_ERR     result = IO_ERR_OK;
    
    if( ioMgr )
    {
        switch( ioMgr->ioType )
        {
            case IO_MEM_RB:
                ioMgr->pos   = 0;
                ioMgr->HFile = 0;            
                break;
                
            case IO_MEM_WB:
                ioMgr->pos   = 0;
                ioMgr->HFile = 0;
                break;                
        }

    }
    else
        result = IO_ERR_NULL_POINTER;

    return result;
}

IO_ERR
Mem_seek(
    IO_MGR          *ioMgr,
    int             offset, 
    IO_SEEK_TYPE    type)
{
    switch( type )
    {
        case IO_SEEK_SET:
            ioMgr->pos = 0;
            break;
        case IO_SEEK_CUR:
            ioMgr->pos += offset;
            break;
        case IO_SEEK_END:
            ioMgr->pos += ioMgr->length;
            break;
    }
    return 0;
}

int
Mem_read(
    IO_MGR          *ioMgr,
    void            *destBuf, 
    unsigned int    size, 
    unsigned int    count)
{
    int     readLeng = 0;
    
    if( ioMgr )
    {
        readLeng = size*count;
        memcpy(destBuf, (void*)((unsigned int)ioMgr->HFile + ioMgr->pos), readLeng);
        ioMgr->pos += readLeng;
    }
    
    return readLeng;
}

int
Mem_write(
    IO_MGR          *ioMgr,
    void            *srcBuf, 
    unsigned int    size, 
    unsigned int    count)
{
    int     writeLeng = 0;
    
    if( ioMgr )
    {
        writeLeng = size*count;
        if( ioMgr->pos + writeLeng > ioMgr->length )
        {
            _err(" err! out buffer size !!");
        }
        else
        {        
            memcpy((void*)((unsigned int)ioMgr->HFile + ioMgr->pos), srcBuf, writeLeng);
            ioMgr->pos += writeLeng;
        }
    }
    
    return writeLeng;
}

unsigned int
Mem_ctl(
    IO_MGR          *ioMgr,
    IO_API_CMD      cmd)
{
    return 0;
}
//=============================================================================
//				  Public Function Definition
//=============================================================================

const IO_DESC mem_io = 
{
    "Mem io",
    Mem_open,
    Mem_close,
    Mem_seek,
    Mem_read,
    Mem_write,
    Mem_ctl,
};
    
    
    