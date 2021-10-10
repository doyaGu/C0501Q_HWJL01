#include <stdio.h>
#include "ite_jpg.h"
#include "jpg_stream.h"
#include "jpg_defs.h"
#include "async_file/file.h"
#include <sys/time.h>

//=============================================================================
//                  Constant Definition
//=============================================================================
#define DEF_BS_BUF_LENGTH         (256 << 10)
#define DEF_BS_RING_BUF_LENGTH    (64 << 10)
#define DEF_ENC_SYS_BS_BUF_LENGTH (256 << 10)
#define DEF_LINE_BUF_LENGTH       (952 << 10)

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
#if (_MSC_VER)
static uint32_t g_vramBsBuf[2] = {0};
static uint32_t g_vramLineBuf    = 0;
static uint32_t g_vramDispBuf[3] = {0};
static uint32_t g_vramEncRingBuf = 0;
#endif

//=============================================================================
//                  Private Function Definition
//=============================================================================

static JPG_ERR
File_open(
    JPG_STREAM_HANDLE *pHJStream,
    void              *extraData)
{
    JPG_ERR result = JPG_ERR_OK;

    if (pHJStream)
    {
        pHJStream->privData[0] = 0;

        // fopen
        switch (pHJStream->jStreamInfo.streamIOType)
        {
        case JPG_STREAM_IO_READ:
            pHJStream->privData[0] = (void *)fopen((char *)pHJStream->jStreamInfo.jstream.path, "rb");
            if (pHJStream->privData[0] == 0)
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " open jpg fail (%s) !!", (char *)pHJStream->jStreamInfo.jstream.path);
            break;

        case JPG_STREAM_IO_WRITE:
			#if 1  //async mode mark it.
            pHJStream->privData[0] = (void *)fopen((char *)pHJStream->jStreamInfo.jstream.path, "wb");
            if (pHJStream->privData[0] == 0)
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " open jpg fail (%s) !!", (char *)pHJStream->jStreamInfo.jstream.path);
			#endif
            break;
        }

        do
        {
            if( pHJStream->privData[0] == 0 )       break;   //async mode mark it.

            jpg_msg_ex(0, " pHJStream->privData[0] = 0x%x\n", pHJStream->privData[0]);
            // get total file size
            pHJStream->curBsPos = 0;

            if (pHJStream->jStreamInfo.streamIOType == JPG_STREAM_IO_READ)
            {
                fseek((FILE *)pHJStream->privData[0], 0, SEEK_END);
                pHJStream->streamSize = ftell((FILE *)pHJStream->privData[0]);
                fseek((FILE *)pHJStream->privData[0], 0, SEEK_SET);
            }
        } while (0);
    }

    return result;
}

static JPG_ERR
File_close(
    JPG_STREAM_HANDLE *pHJStream,
    void              *extraData)
{
    JPG_ERR result = JPG_ERR_OK;

    if (pHJStream)
    {
 			if (pHJStream->privData[0])
            {
                fclose(pHJStream->privData[0]);
                pHJStream->privData[0] = 0;
            }
			#if 0 
		        switch (pHJStream->jStreamInfo.streamIOType)
		        {
		        case JPG_STREAM_IO_READ:
		       //async mode mark it.
		            if (pHJStream->privData[0])
		            {
		                fclose(pHJStream->privData[0]);
		                pHJStream->privData[0] = 0;
		            }

		            break;

		        case JPG_STREAM_IO_WRITE:
		            break;
		        }
			#endif
    }

    return result;
}

static JPG_ERR
File_seek(
    JPG_STREAM_HANDLE *pHJStream,
    int               offset,
    JPG_SEEK_TYPE     seekType,
    void              *extraData)
{
    JPG_ERR result = JPG_ERR_OK;

    if (pHJStream)
    {
        if (pHJStream->privData[0])
        {
            int      rst  = 0;
            uint32_t type = 0;

            switch (seekType)
            {
            case JPG_SEEK_SET:
                type                 = SEEK_SET;
                pHJStream->curBsPos  = offset;
                break;
            case JPG_SEEK_CUR:
                type                 = SEEK_CUR;
                pHJStream->curBsPos += offset;
                break;
            case JPG_SEEK_END:
                type                 = SEEK_END;
                pHJStream->curBsPos  = (pHJStream->streamSize - pHJStream->curBsPos);
                break;
            }

            rst = fseek(pHJStream->privData[0], offset, type);
            if (rst)
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " seek fail !!");

            if (pHJStream->curBsPos > pHJStream->streamSize)
                pHJStream->curBsPos = pHJStream->streamSize;

            if ( (int)pHJStream->curBsPos < 0)
                pHJStream->curBsPos = 0;
        }
    }

    return result;
}

static JPG_ERR
File_tell(
    JPG_STREAM_HANDLE *pHJStream,
    uint32_t          *pCurPos,
    void              *extraData)
{
    JPG_ERR result = JPG_ERR_OK;

    if (pHJStream)
    {
        if (pHJStream->privData[0])
        {
            uint32_t curr = 0;
            curr = ftell(pHJStream->privData[0]);
            if (pCurPos)
                *pCurPos = curr;
        }
    }

    return result;
}

static JPG_ERR
File_read(
    JPG_STREAM_HANDLE *pHJStream,
    void              *srcBuf,
    uint32_t          requestSize,
    uint32_t          *realSize,
    void              *extraData)
{
    JPG_ERR result = JPG_ERR_OK;

    if (pHJStream)
    {
        if (pHJStream->privData[0])
        {
            uint32_t size = 0;
            size                 = fread((uint8_t *)srcBuf, 1, requestSize, (FILE *)pHJStream->privData[0]);
            if (realSize)
                *realSize = size;
            pHJStream->curBsPos += size;
        }
    }

    return result;
}

static void
_file_write_callback(
    PAL_FILE  *file,
    MMP_ULONG result,
    void      *arg)
{
    //printf("_file_write_callback ,file = 0x%x , result = %lu ,arg = 0x%x\n",file , result,arg);
    if (arg) //free destBuf
    {
        free(arg);
        //malloc_stats();
    }
}

static JPG_ERR
File_write(
    JPG_STREAM_HANDLE *pHJStream,
    void              *destBuf,
    uint32_t          length,
    void              *extraData)
{
    JPG_ERR result = JPG_ERR_OK;

    if (pHJStream)
    {
		#if 1   //async mode mark it.
        if (pHJStream->privData[0])
        {
            uint32_t size = 0;
            size                 = fwrite(destBuf, 1, length, (FILE *)pHJStream->privData[0]);

            printf("size = %d\n", size);
            pHJStream->curBsPos += size;
        }
		#else
        //Async mode
        PalFileOpenWriteClose( (char *)pHJStream->jStreamInfo.jstream.path, destBuf,
                               1, length, _file_write_callback, MMP_NULL);
		#endif
    }

    return result;
}

static JPG_ERR
File_control(
    JPG_STREAM_HANDLE *pHJStream,
    uint32_t          cmd,
    uint32_t          *value,
    void              *extraData)
{
    JPG_ERR result = JPG_ERR_OK;
    if (pHJStream)
    {
        switch (cmd)
        {
        case JPG_STREAM_CMD_GET_BS_BUF_SIZE:
            if (value)
                *value = DEF_BS_BUF_LENGTH;
            break;

        case JPG_STREAM_CMD_GET_LINE_BUF_SIZE:
            if (value)
                *value = DEF_LINE_BUF_LENGTH;
            break;

        case JPG_STREAM_CMD_GET_BS_RING_BUF_SIZE:
            if (value)
                *value = DEF_BS_RING_BUF_LENGTH;
            break;

        case JPG_STREAM_CMD_GET_ENC_SYS_BS_BUF_SIZE:
            if (value)
                *value = DEF_ENC_SYS_BS_BUF_LENGTH;
            break;

        // case JPG_STREAM_CMD_GET_TOTAL_LENGTH:
        //     if( value )     *value = pHJStream->streamSize;
        //     break;
        // case JPG_STREAM_CMD_GET_CUR_POS:
        //     if( value )     *value = pHJStream->curBsPos;
        //     break;

        case JPG_STREAM_CMD_SET_STREAM_INFO:
            break;
#if (_MSC_VER)
        case JPG_STREAM_CMD_GET_VRAM_LINE_BUF:
            if (value)
                *value = g_vramLineBuf;
            break;
        case JPG_STREAM_CMD_GET_VRAM_BS_BUF_A:
            if (value)
                *value = g_vramBsBuf[0];
            break;
        case JPG_STREAM_CMD_GET_VRAM_BS_BUF_B:
            if (value)
                *value = g_vramBsBuf[1];
            break;
        case JPG_STREAM_CMD_GET_VRAM_ENC_RING_BUF:
            if (value)
                *value = g_vramEncRingBuf;
            break;
        case JPG_STREAM_CMD_GET_VRAM_DISP_BUF_0:
            if (value)
                *value = g_vramDispBuf[0];
            break;
        case JPG_STREAM_CMD_GET_VRAM_DISP_BUF_1:
            if (value)
                *value = g_vramDispBuf[1];
            break;
        case JPG_STREAM_CMD_GET_VRAM_DISP_BUF_2:
            if (value)
                *value = g_vramDispBuf[2];
            break;
#endif
        }
    }

    return result;
}

static void *
File_HeapBuf(
    JPG_STREAM_HANDLE *pHJStream,
    JPG_HEAP_TYPE     heapType,
    uint32_t          requestSize,
    uint32_t          *realSize)
{
    void *ptr = 0;

    if (pHJStream)
    {
#if _MSC_VER
        switch (heapType)
        {
        case JPG_HEAP_DISP_BUF_0:
        case JPG_HEAP_DISP_BUF_1:
        case JPG_HEAP_DISP_BUF_2:
    #if !(ENABLE_JPG_SW_SIMULSTION)
            if (g_vramDispBuf[heapType - JPG_HEAP_DISP_BUF_0] == 0)
                g_vramDispBuf[heapType - JPG_HEAP_DISP_BUF_0] = (uint32_t)jpgVmemAlloc(requestSize);
    #endif
            ptr = g_vramDispBuf[heapType - JPG_HEAP_DISP_BUF_0];
            printf(" g_vramDispBuf[%d] = 0x%x\n", heapType - JPG_HEAP_DISP_BUF_0, g_vramDispBuf[heapType - JPG_HEAP_DISP_BUF_0]);
            break;

        case JPG_HEAP_BS_BUF:
            ptr = jpg_malloc(requestSize);
            if (ptr && realSize)
                *realSize = requestSize;

    #if !(ENABLE_JPG_SW_SIMULSTION)
            if (g_vramBsBuf[0] == 0)
                g_vramBsBuf[0] = (uint32_t)jpgVmemAlloc(requestSize);

            printf("g_vramBsBuf[0] = 0x%x, size= %d\n", g_vramBsBuf[0], requestSize);
    #endif
            break;

        case JPG_HEAP_LINE_BUF:
            ptr = jpg_malloc(requestSize);
            if (ptr && realSize)
                *realSize = requestSize;

    #if !(ENABLE_JPG_SW_SIMULSTION)
            if (g_vramLineBuf == 0)
                g_vramLineBuf = (uint32_t)jpgVmemAlloc(requestSize);

            printf("g_vramLineBuf = 0x%x, size= %d\n", g_vramLineBuf, requestSize);
    #endif
            break;

        case JPG_HEAP_ENC_BS_RING_BUF:
            ptr = jpg_malloc(requestSize);
            if (ptr && realSize)
                *realSize = requestSize;

    #if !(ENABLE_JPG_SW_SIMULSTION)
            if (g_vramEncRingBuf == 0)
                g_vramEncRingBuf = (uint32_t)jpgVmemAlloc(requestSize);
            printf("g_vramEncRingBuf = 0x%x, size= %d\n", g_vramEncRingBuf, requestSize);
    #endif
            break;

        case JPG_HEAP_ENC_SYS_BS_BUF:
        case JPG_HEAP_DEF:
            ptr = jpg_malloc(requestSize);
            if (ptr && realSize)
                *realSize = requestSize;
            break;
        }
#else
        switch (heapType)
        {
        case JPG_HEAP_LINE_BUF:
    #if (ENABLE_JDEBUG_MODE)
            ptr = jpg_malloc((10 << 20));
            if (ptr && realSize)
                *realSize = (10 << 20);
            break;
    #endif
        case JPG_HEAP_BS_BUF:
        case JPG_HEAP_ENC_BS_RING_BUF:
        case JPG_HEAP_ENC_SYS_BS_BUF:
        case JPG_HEAP_DEF:
            ptr = jpg_malloc(requestSize);
            if (ptr && realSize)
                *realSize = requestSize;
            break;
        }
#endif
    }

    return (void *)ptr;
}

static void
File_FreeBuf(
    JPG_STREAM_HANDLE *pHJStream,
    JPG_HEAP_TYPE     heapType,
    void              *ptr)
{
    if (pHJStream)
    {
#if _MSC_VER
        switch (heapType)
        {
        case JPG_HEAP_DISP_BUF_0:
        case JPG_HEAP_DISP_BUF_1:
        case JPG_HEAP_DISP_BUF_2:
    #if !(ENABLE_JPG_SW_SIMULSTION)
            if (g_vramDispBuf[heapType - JPG_HEAP_DISP_BUF_0])
                jpgVmemFree(g_vramDispBuf[heapType - JPG_HEAP_DISP_BUF_0]);
            g_vramDispBuf[heapType - JPG_HEAP_DISP_BUF_0] = 0;
    #endif
            break;

        case JPG_HEAP_BS_BUF:
            if (ptr)
                free(ptr);

    #if !(ENABLE_JPG_SW_SIMULSTION)
            if (g_vramBsBuf[0])
                jpgVmemFree(g_vramBsBuf[0]);
            g_vramBsBuf[0] = 0;
    #endif
            break;

        case JPG_HEAP_LINE_BUF:
            if (ptr)
                free(ptr);

    #if !(ENABLE_JPG_SW_SIMULSTION)
            if (g_vramLineBuf)
                jpgVmemFree(g_vramLineBuf);
            g_vramLineBuf = 0;
    #endif
            break;

        case JPG_HEAP_ENC_BS_RING_BUF:
            if (ptr)
                free(ptr);

    #if !(ENABLE_JPG_SW_SIMULSTION)
            if (g_vramEncRingBuf)
                jpgVmemFree(g_vramEncRingBuf);
            g_vramEncRingBuf = 0;
    #endif
            break;

        case JPG_HEAP_ENC_SYS_BS_BUF:
        case JPG_HEAP_DEF:
            if (ptr)
                free(ptr);
            break;
        }

#else
        switch (heapType)
        {
        case JPG_HEAP_BS_BUF:
        case JPG_HEAP_LINE_BUF:
        case JPG_HEAP_ENC_BS_RING_BUF:
        case JPG_HEAP_ENC_SYS_BS_BUF:
        case JPG_HEAP_DEF:
            if (ptr)
                free(ptr);
            break;
        }
#endif
    }

    return;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================

JPG_STREAM_DESC jpg_stream_file_desc =
{
    "jpg file stream",  // const char              *typeName;
    JPG_STREAM_FILE,    // const JPG_STREAM_TYPE   streamType;

    File_open,          // JPG_ERR (*jOpen_stream)(struct JPG_STREAM_HANDLE_TAG *hJStream, void *extraData);
    File_close,         // JPG_ERR (*jClose_stream)(struct JPG_STREAM_HANDLE_TAG *hJStream, void *extraData);
    File_seek,          // JPG_ERR (*jSeek_stream)(struct JPG_STREAM_HANDLE_TAG *hJStream, int offset, JPG_SEEK_TYPE seekType, void *extraData);
    File_tell,          // JPG_ERR (*jTell_stream)(struct JPG_STREAM_HANDLE_TAG *pHJStream, void *extraData);
    File_read,          // JPG_ERR (*jFull_buf)(struct JPG_STREAM_HANDLE_TAG *pHJStream, void *srcBuf, uint32_t requestSize, uint32_t *realSize, void *extraData);
    File_write,         // JPG_ERR (*jOut_buf)(struct JPG_STREAM_HANDLE_TAG *hJStream, void *destBuf, uint32_t length, void *extraData);
    File_control,       // JPG_ERR (*jControl)(struct JPG_STREAM_HANDLE_TAG *hJStream, uint32_t cmd, uint32_t *value, void *extraData);
    File_HeapBuf,       // void*   (*jHeap_mem)(struct JPG_STREAM_HANDLE_TAG *hJStream, JPG_HEAP_TYPE heapType, uint32_t requestSize, uint32_t *realSize);
    File_FreeBuf,       // void*   (*jFree_mem)(struct JPG_STREAM_HANDLE_TAG *hJStream, JPG_HEAP_TYPE heapType, void *ptr);
};