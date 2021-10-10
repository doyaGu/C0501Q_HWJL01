#ifndef __IO_API_H_JCDAFPH7_WXE3_Q44Q_EDT9_XU2W1XZ5H9K5__
#define __IO_API_H_JCDAFPH7_WXE3_Q44Q_EDT9_XU2W1XZ5H9K5__

#ifdef __cplusplus
extern "C" {
#endif


#include <string.h>
#include <stdlib.h>
//=============================================================================
//				  Constant Definition
//=============================================================================
#define HAS_FILE_SYS    1

typedef enum _IO_ERR_TAG
{
    IO_ERR_OK               = 0,
    IO_ERR_NULL_POINTER,
    IO_ERR_OPEN_FAIL,
    
    IO_ERR_UNKNOW,
}IO_ERR;

typedef enum _IO_SEEK_TYPE_TAG
{
    IO_SEEK_SET = 0, /**< Beginning of file */
    IO_SEEK_CUR = 1, /**< Current position of file pointer */
    IO_SEEK_END = 2, /**< End of file */
}IO_SEEK_TYPE;

typedef enum _IO_API_CMD_TAG
{
    IO_CMD_NO_IMPL       = 0,
    IO_CMD_GET_TOTAL_LENGTH,
    IO_CMD_GET_CUR_POS,
    
}IO_API_CMD;

typedef enum _IO_TYPE_TAG
{
    IO_UNKNOW      = 0,
    IO_FILE_RB,
    IO_FILE_WB,
    IO_MEM_RB,
    IO_MEM_WB,

}IO_TYPE;
//=============================================================================
//				  Macro Definition
//=============================================================================


//=============================================================================
//				  Structure Definition
//=============================================================================
struct _IO_DESC_TAG;

typedef struct _IO_MGR_TAG
{
    // share 
    IO_TYPE             ioType;

    unsigned int        length;    // stream total length or file size
    unsigned int        pos;       // current position in streamLength

    void                *privData;
    
    struct _IO_DESC_TAG    *ioDesc;

    // file 
    char                *filePath;
    void                *HFile;

}IO_MGR;

typedef struct _IO_DESC_TAG
{
    const char     *typeName;
    IO_ERR         (*open_stream)(IO_MGR *ioMgr, void *extraData);
    IO_ERR         (*close_stream)(IO_MGR *ioMgr);
    IO_ERR         (*seek_data)(IO_MGR *ioMgr, int offset, IO_SEEK_TYPE type);
    int            (*fill_buf)(IO_MGR *ioMgr, void *destBuf, unsigned int size, unsigned int count);
    int            (*output_buf)(IO_MGR *ioMgr, void *srcBuf, unsigned int size, unsigned int count);
    unsigned int   (*control)(IO_MGR *ioMgr, IO_API_CMD cmd);
   
}IO_DESC;

typedef struct _BASE_STRM_INFO_TAG
{
    union {
        struct {
            unsigned char   *startAddr;
            unsigned int    length;
        }strm;

        unsigned char   *filename;
    }u;
}BASE_STRM_INFO;
//=============================================================================
//				  Global Data Definition
//=============================================================================


//=============================================================================
//				  Private Function Definition
//=============================================================================


//=============================================================================
//				  Public Function Definition
//=============================================================================
IO_MGR*
ioA_Create_Stream(
    IO_TYPE     srcType,
    void        *extraData);
    

void
ioA_Destroy_Stream(
    IO_MGR    *pIoMgr);
    

IO_ERR
ioA_Stream_Open(
    IO_MGR    *pIoMgr,
    void      *extraData);
    

void
ioA_Stream_Close(
    IO_MGR    *pIoMgr);
    

int
ioA_Stream_FillBuf(
    IO_MGR          *pIoMgr,
    void            *destBuf, 
    unsigned int    size,
    unsigned int    count);
    

int
ioA_Stream_OutputBuf(
    IO_MGR          *pIoMgr,
    void            *srcBuf, 
    unsigned int    size,
    unsigned int    count);
    

int
ioA_Stream_SeekData(
    IO_MGR          *pIoMgr,
    int             offset, 
    IO_SEEK_TYPE    type);

    
unsigned int
ioA_Stream_Ctrl(
    IO_MGR      *pIoMgr,
    IO_API_CMD  cmd);
    
    

#ifdef __cplusplus
}
#endif

#endif
