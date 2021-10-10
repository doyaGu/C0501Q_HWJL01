#ifndef __jpg_stream_H_eug73jgL_fkiT_yBmx_wXP6_iSvbgkK1z7fk__
#define __jpg_stream_H_eug73jgL_fkiT_yBmx_wXP6_iSvbgkK1z7fk__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "jpg_err.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
typedef enum JPG_STREAM_TYPE_TAG
{
    JPG_STREAM_UNKNOW   = 0,
    JPG_STREAM_FILE,
    JPG_STREAM_MEM,
    JPG_STREAM_CUSTOMER,

}JPG_STREAM_TYPE;

/**
 * stream I/O read/write type
 **/
typedef enum JPG_STREAM_IO_TYPE_TAG
{
    JPG_STREAM_IO_UNKNOW     = 0,
    JPG_STREAM_IO_READ,
    JPG_STREAM_IO_WRITE,

}JPG_STREAM_IO_TYPE;

/**
 * seek type
 **/
typedef enum JPG_SEEK_TYPE_TAG
{
    JPG_SEEK_SET = 0, /**< Beginning of file */
    JPG_SEEK_CUR = 1, /**< Current position of file pointer */
    JPG_SEEK_END = 2, /**< End of file */

}JPG_SEEK_TYPE;

/**
 * jpg buffer heap type
 **/
typedef enum JPG_HEAP_TYPE_TAG
{
    JPG_HEAP_DEF    =   0,
    JPG_HEAP_STATIC,
    JPG_HEAP_LINE_BUF,
    JPG_HEAP_BS_BUF,
    JPG_HEAP_ENC_YUV_BUF,
    JPG_HEAP_ENC_BS_RING_BUF,
    JPG_HEAP_ENC_SYS_BS_BUF,

    // for win32
    JPG_HEAP_DISP_BUF_0,
    JPG_HEAP_DISP_BUF_1,
    JPG_HEAP_DISP_BUF_2,

}JPG_HEAP_TYPE;

typedef enum JPG_STREAM_CMD_TAG
{
    JPG_STREAM_CMD_NO_IMPL      = 0,
    JPG_STREAM_CMD_GET_BS_BUF_SIZE = 0xB0000000,
    JPG_STREAM_CMD_GET_LINE_BUF_SIZE,
    JPG_STREAM_CMD_GET_BS_RING_BUF_SIZE,
    JPG_STREAM_CMD_GET_ENC_SYS_BS_BUF_SIZE,
    //JPG_STREAM_CMD_GET_TOTAL_LENGTH,
    //JPG_STREAM_CMD_GET_CUR_POS,
    JPG_STREAM_CMD_SET_STREAM_INFO,

    // for win32
    JPG_STREAM_CMD_GET_VRAM_LINE_BUF,
    JPG_STREAM_CMD_GET_VRAM_BS_BUF_A,
    JPG_STREAM_CMD_GET_VRAM_BS_BUF_B,
    JPG_STREAM_CMD_GET_VRAM_DISP_BUF,
    JPG_STREAM_CMD_GET_VRAM_ENC_RING_BUF,
    JPG_STREAM_CMD_GET_VRAM_ENC_YUV_BUF,
    JPG_STREAM_CMD_GET_VRAM_DISP_BUF_0,
    JPG_STREAM_CMD_GET_VRAM_DISP_BUF_1,
    JPG_STREAM_CMD_GET_VRAM_DISP_BUF_2,

}JPG_STREAM_CMD;

//=============================================================================
//                  Macro Definition
//=============================================================================
#if (_MSC_VER) // WIN32
    #define jpg_byte_align4                        __attribute__ ((aligned(4)))
#elif (__GNUC__)
    #define jpg_byte_align4                        __attribute__ ((aligned(4)))
#else
    #define jpg_byte_align4
#endif
//=============================================================================
//                  Structure Definition
//=============================================================================
struct JPG_STREAM_HANDLE_TAG;

/**
 * Jpg stream I/O description
 **/
typedef struct JPG_STREAM_DESC_TAG
{
    char              typeName[64]; // for alignment
    JPG_STREAM_TYPE   streamType;

    JPG_ERR (*jOpen_stream)(struct JPG_STREAM_HANDLE_TAG *pHJStream, void *extraData);
    JPG_ERR (*jClose_stream)(struct JPG_STREAM_HANDLE_TAG *pHJStream, void *extraData);
    JPG_ERR (*jSeek_stream)(struct JPG_STREAM_HANDLE_TAG *pHJStream, int offset, JPG_SEEK_TYPE seekType, void *extraData);
    JPG_ERR (*jTell_stream)(struct JPG_STREAM_HANDLE_TAG *pHJStream, uint32_t *pCurPos, void *extraData);
    JPG_ERR (*jFull_buf)(struct JPG_STREAM_HANDLE_TAG *pHJStream, void *srcBuf, uint32_t requestSize, uint32_t *realSize, void *extraData);
    JPG_ERR (*jOut_buf)(struct JPG_STREAM_HANDLE_TAG *pHJStream, void *destBuf, uint32_t length, void *extraData);
    JPG_ERR (*jControl)(struct JPG_STREAM_HANDLE_TAG *pHJStream, uint32_t cmd, uint32_t *value, void *extraData);

    // heap API
    void*   (*jHeap_mem)(struct JPG_STREAM_HANDLE_TAG *pHJStream, JPG_HEAP_TYPE heapType, uint32_t requestSize, uint32_t *realSize);
    void    (*jFree_mem)(struct JPG_STREAM_HANDLE_TAG *pHJStream, JPG_HEAP_TYPE heapType, void *ptr);

}JPG_STREAM_DESC;

typedef struct JPG_STREAM_INFO_TAG
{
    JPG_STREAM_TYPE     streamType;
    JPG_STREAM_IO_TYPE  streamIOType;

    // for customer, if they want to define by self
    int (*jpg_reset_stream_info)(struct JPG_STREAM_HANDLE_TAG *pHJStream, void *extraData);

    uint32_t            validCompCnt; // number of Valid components for memory source
    union{
        // memory source
        struct{
            uint8_t     *pAddr;
            uint32_t    length;
            uint32_t    pitch;
        }mem[4]; // 0: R/Y/rgb565, 1: G/U, 2: B/V, 3: reserve

        void        *hFile;     // file handle
        void        *path;      // file name
        void        *extraData; // reserve for other case
    }jstream;

}JPG_STREAM_INFO;

/**
 * Jpg I/O handle
 **/
typedef struct JPG_STREAM_HANDLE_TAG
{
    JPG_STREAM_DESC     jStreamDesc;

    JPG_STREAM_INFO     jStreamInfo;

    bool                bOpened;
    uint32_t            compCnt;    // number of components in the jpg stream
    uint32_t            streamSize; // total stream size
    uint32_t            curBsPos;

    void                *privData[4];
}JPG_STREAM_HANDLE;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
