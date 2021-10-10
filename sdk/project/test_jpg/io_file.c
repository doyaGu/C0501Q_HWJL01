
#include "globle.h"
#include <stdio.h>
#include "io_api.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
#define OUT_JPEG_BS_SIZE_MAX    (300<<10)

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
File_open(
    IO_MGR  *ioMgr, 
    void    *extraData)
{
    IO_ERR          result = IO_ERR_OK;
    BASE_STRM_INFO  *pStrmInfo = (BASE_STRM_INFO*)extraData;

    if( ioMgr )
    {
        ioMgr->filePath = pStrmInfo->u.filename;
        switch( ioMgr->ioType )
        {
            case IO_FILE_RB:
            case IO_MEM_RB:
                ioMgr->HFile = fopen(pStrmInfo->u.filename, "rb");
                if( ioMgr->HFile )
                {
                    fseek(ioMgr->HFile, 0, SEEK_END);
                    ioMgr->length = ftell(ioMgr->HFile);
                    fseek(ioMgr->HFile, 0, SEEK_SET);                
                }
                else
                {
                    _err("open %s fail !", pStrmInfo->u.filename);
                    result = IO_ERR_OPEN_FAIL;
                }
                break;

            case IO_MEM_WB:
                ioMgr->HFile = fopen(pStrmInfo->u.filename, "wb");
                if( !ioMgr->HFile )
                {
                    _err("open %s fail !", pStrmInfo->u.filename);
                    result = IO_ERR_OPEN_FAIL;
                } 
                ioMgr->length = OUT_JPEG_BS_SIZE_MAX;
                break;                
        }

    }
    else
        result = IO_ERR_NULL_POINTER;
        
    return result;
}

IO_ERR
File_close(
    IO_MGR  *ioMgr)
{
    IO_ERR     result = IO_ERR_OK;
    
    if( ioMgr )
    {
        if( ioMgr->HFile )
        {
            fclose(ioMgr->HFile);
            ioMgr->HFile = 0;
        }
    }
    else
        result = IO_ERR_NULL_POINTER;

    return result;
}

IO_ERR
File_seek(
    IO_MGR          *ioMgr,
    int             offset, 
    IO_SEEK_TYPE    type)
{
    return 0;
}

int
File_read(
    IO_MGR          *ioMgr,
    void            *destBuf, 
    unsigned int    size, 
    unsigned int    count)
{
    int     readLeng = 0;
    
    if( ioMgr )
    {
        readLeng = fread(destBuf, size, count, ioMgr->HFile);
    }
    
    return readLeng;
}

int
File_write(
    IO_MGR          *ioMgr,
    void            *srcBuf, 
    unsigned int    size, 
    unsigned int    count)
{
    int     writeLeng = 0;
    
    if( ioMgr )
    {
        writeLeng = fwrite(srcBuf, size, count, ioMgr->HFile);
    }
    
    return writeLeng;
}

unsigned int
File_ctl(
    IO_MGR          *ioMgr,
    IO_API_CMD      cmd)
{
    return 0;
}
//=============================================================================
//				  Public Function Definition
//=============================================================================

const IO_DESC file_io = 
{
    "file io",
    File_open,
    File_close,
    File_seek,
    File_read,
    File_write,
    File_ctl,
};
    
    
    