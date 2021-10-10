
#include "jpg_defs.h"
#include "jpg_types.h"
#include "jpg_codec.h"
#include "jpg_common.h"
#include "register_template.h"
#include "jpg_extern_link.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define JPG_DC_ONLY_FACTOR      8

// Q table
const uint8_t
const Def_QTable_Y[JPG_Q_TABLE_SIZE] =
{
    // Luminance Quantization Table
    0x10,
    0x0b, 0x0c,
    0x0e, 0x0c, 0x0a,
    0x10, 0x0e, 0x0d, 0x0e,
    0x12, 0x11, 0x10, 0x13, 0x18,
    0x28, 0x1a, 0x18, 0x16, 0x16, 0x18,
    0x31, 0x23, 0x25, 0x1d, 0x28, 0x3a, 0x33,
    0x3d, 0x3c, 0x39, 0x33, 0x38, 0x37, 0x40, 0x48,
    0x5c, 0x4e, 0x40, 0x44, 0x57, 0x45, 0x37,
    0x38, 0x50, 0x6d, 0x51, 0x57, 0x5f,
    0x62, 0x67, 0x68, 0x67, 0x3e,
    0x4d, 0x71, 0x79, 0x70,
    0x64, 0x78, 0x5c,
    0x65, 0x67,
    0x63,
};

const uint8_t
const Def_QTable_UV[JPG_Q_TABLE_SIZE] =
{
    // Chrominance Quantization Table
    0x11,
    0x12, 0x12,
    0x18, 0x15, 0x18,
    0x2f, 0x1a, 0x1a, 0x2f,
    0x63, 0x42, 0x38, 0x42, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63, 0x63,
    0x63, 0x63, 0x63,
    0x63, 0x63,
    0x63,
};

// Huffman table
const uint8_t
const Def_DCHuffTable[2][28] =
{
    {   // Luminance Quantization Table
        // the list of code lengths (16 bytes)
        0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // the set of value following this list (12 bytes)
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b,
    },
    {   // Chrominance Quantization Table
        // the list of code lengths (16 bytes)
        0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        // the set of value following this list (12 bytes)
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b,
    },
};

const uint8_t
const Def_ACHuffTable[2][178] =
{
    {   // Luminance Quantization Table
        // the list of code lengths (16 bytes)
        0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03,
        0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d,
        // the set of value following this list (162 bytes)
        0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
        0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
        0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
        0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
        0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
        0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
        0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
        0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
        0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
        0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
        0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
        0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
        0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
        0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
        0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
        0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
        0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
        0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
        0xf9, 0xfa,
    },
    {   // Chrominance Quantization Table
        // the list of code lengths (16 bytes)
        0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04,
        0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
        // the set of value following this list (162 bytes)
        0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
        0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
        0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
        0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
        0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
        0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
        0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
        0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
        0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
        0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
        0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
        0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
        0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
        0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
        0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
        0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
        0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
        0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
        0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
        0xf9, 0xfa,
    },
};

//=============================================================================
//                  Macro Definition
//=============================================================================
DEFINE_REGISTER_TEMPLATE(JPG_CODEC_DESC, JPG_CODEC_TYPE);

#define SET_BYTE_VALUE(ptr, value)          do{(*(ptr))=(value); (ptr++);}while(0)

#define SET_WORD_VALUE(ptr, wValue)         do{ (*(ptr)) = ((wValue) >> 8); (ptr++);  \
                                                (*(ptr)) = (uint8_t)(wValue); (ptr++); \
                                            }while(0)

#define SET_LOW_BYTE_VALUE(ptr, value)      (*(ptr)) |= ((value) & 0x0f)
#define SET_HIGH_BYTE_VALUE(ptr, value)     (*(ptr)) |= (((value) << 4) & 0xf0)

//=============================================================================
//                  Structure Definition
//=============================================================================

struct JPG_COMM_DEV_TAG;

typedef struct JPG_COMM_DEV_TAG
{
    JCOMM_HANDLE        hJComm;

    JCOMM_INIT_PARAM    jCommInitParam;

    JPG_CODEC_DESC      jCodecDesc;

    JPG_CODEC_HANDLE    hJCodec;

    JPG_ERR (*pre_setting)(struct JPG_COMM_DEV_TAG *pJCommDev);

}JPG_COMM_DEV;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_jpg_register_all(
    void)
{
    static int bInitialized = 0;

    if( bInitialized )
        return;
    bInitialized = 1;

    REGISTER_ITEM(JPG_CODEC_DESC, DECODER, decoder);
    REGISTER_ITEM(JPG_CODEC_DESC, ENCODER, encoder);
    REGISTER_ITEM(JPG_CODEC_DESC, DEC_MJPG, dec_mjpg);
    REGISTER_ITEM(JPG_CODEC_DESC, ENC_MJPG, enc_mjpg);
    REGISTER_ITEM(JPG_CODEC_DESC, DEC_JPG_CMD, dec_jpg_cmd);
}

static JPG_ERR
_jpg_allocate_line_buf(
    JPG_COMM_DEV    *pJCommDev)
{
    JPG_ERR             result = JPG_ERR_OK;
    JPG_STREAM_DESC     *pJStreamDesc = &pJCommDev->hJCodec.pHInJStream->jStreamDesc;
    JPG_LINE_BUF_INFO   *pJLineBufInfo = &pJCommDev->hJCodec.jLineBufInfo;
    uint32_t            lineBufSize = 0;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x\n", pJCommDev);

    do{
        JPG_CODEC_HANDLE    *pHJCodec = &pJCommDev->hJCodec;

        // allocate line buffer
        if( pJStreamDesc->jControl )
            pJStreamDesc->jControl(pJCommDev->hJCodec.pHInJStream,
                                   (uint32_t)JPG_STREAM_CMD_GET_LINE_BUF_SIZE,
                                   &lineBufSize, 0);
        if( pJStreamDesc->jHeap_mem )
        {
            pJLineBufInfo->addrAlloc = pJStreamDesc->jHeap_mem(
                                            pHJCodec->pHInJStream,
                                            JPG_HEAP_LINE_BUF,
                                            lineBufSize,
                                            &pJLineBufInfo->size);
            if( pJLineBufInfo->addrAlloc == 0 )
            {
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " Allocate Line buf fail !! ", __FUNCTION__);
                result = JPG_ERR_ALLOCATE_FAIL;
                break;
            }
        }
    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;
}

#if _MSC_VER
static JPG_ERR
_jpg_allocate_disp_buf(
    JPG_COMM_DEV    *pJCommDev)
{
    // this function for win32
    JPG_ERR             result = JPG_ERR_OK;
    JPG_STREAM_DESC     *pJStreamDesc = &pJCommDev->hJCodec.pHInJStream->jStreamDesc;
    JPG_BUF_INFO        *pJOutBufInfo_0 = &pJCommDev->hJCodec.pHOutJStream->jStreamInfo.jstream.mem[0];//&pJCommDev->hJComm.jOutBufInfo[0];
    JPG_BUF_INFO        *pJOutBufInfo_1 = &pJCommDev->hJCodec.pHOutJStream->jStreamInfo.jstream.mem[1];//&pJCommDev->hJComm.jOutBufInfo[1];
    JPG_BUF_INFO        *pJOutBufInfo_2 = &pJCommDev->hJCodec.pHOutJStream->jStreamInfo.jstream.mem[2];//&pJCommDev->hJComm.jOutBufInfo[2];
    JPG_DISP_INFO       *pJDispInfo = pJCommDev->hJComm.pJDispInfo;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x\n", pJCommDev);

    {
        uint32_t addr1 = ithLcdGetBaseAddrA();
        uint32_t addr2 = ithLcdGetBaseAddrB();
        uint32_t addr3 = ithLcdGetBaseAddrC();

        uint32_t flip_id = ithLcdGetFlip();

        printf("lcd(%p)flip_id(%d)\n", addr1, flip_id);
    }

    do{
        JPG_CODEC_HANDLE    *pHJCodec = &pJCommDev->hJCodec;

        // allocate display buffer
        switch( pJDispInfo->outColorSpace )
        {
            case JPG_COLOR_SPACE_ARGB4444:
            case JPG_COLOR_SPACE_ARGB8888:
            case JPG_COLOR_SPACE_RGB565:
                if( 0 && pJStreamDesc->jHeap_mem )  //Here should do nothing!! Benson 2015/11/20
                {
                    JPG_BUF_INFO        *pJSysOutBufInfo_0 = &pJCommDev->hJComm.jSysOutBufInfo[0];

                    (*pJSysOutBufInfo_0) = (*pJOutBufInfo_0);
                    pJOutBufInfo_0->pBufAddr = pJStreamDesc->jHeap_mem(
                                                    pHJCodec->pHInJStream,
                                                    JPG_HEAP_DISP_BUF_0,
                                                    pJOutBufInfo_0->bufLength,
                                                    &pJSysOutBufInfo_0->bufLength);
                    if( pJOutBufInfo_0->pBufAddr == 0 )
                    {
                        jpg_msg_ex(JPG_MSG_TYPE_ERR, " Allocate Line buf fail !! ", __FUNCTION__);
                        result = JPG_ERR_ALLOCATE_FAIL;
                        break;
                    }
                }
                break;

            case JPG_COLOR_SPACE_YUV422:
            case JPG_COLOR_SPACE_YUV420:
                if( pJStreamDesc->jHeap_mem )
                {
                    JPG_BUF_INFO        *pJSysOutBufInfo_0 = &pJCommDev->hJComm.jSysOutBufInfo[0];
                    JPG_BUF_INFO        *pJSysOutBufInfo_1 = &pJCommDev->hJComm.jSysOutBufInfo[1];
                    JPG_BUF_INFO        *pJSysOutBufInfo_2 = &pJCommDev->hJComm.jSysOutBufInfo[2];

                    (*pJSysOutBufInfo_0) = (*pJOutBufInfo_0);
                    (*pJSysOutBufInfo_1) = (*pJOutBufInfo_1);
                    (*pJSysOutBufInfo_2) = (*pJOutBufInfo_2);

                    pJOutBufInfo_0->pBufAddr = pJStreamDesc->jHeap_mem(
                                                    pHJCodec->pHInJStream,
                                                    JPG_HEAP_DISP_BUF_0,
                                                    pJOutBufInfo_0->bufLength,
                                                    &pJSysOutBufInfo_0->bufLength);
                    if( pJOutBufInfo_0->pBufAddr == 0 )
                    {
                        jpg_msg_ex(JPG_MSG_TYPE_ERR, " Allocate Line buf fail !! ", __FUNCTION__);
                        result = JPG_ERR_ALLOCATE_FAIL;
                        break;
                    }

                    pJOutBufInfo_1->pBufAddr = pJStreamDesc->jHeap_mem(
                                                    pHJCodec->pHInJStream,
                                                    JPG_HEAP_DISP_BUF_1,
                                                    pJOutBufInfo_1->bufLength,
                                                    &pJSysOutBufInfo_1->bufLength);
                    if( pJOutBufInfo_1->pBufAddr == 0 )
                    {
                        jpg_msg_ex(JPG_MSG_TYPE_ERR, " Allocate Line buf fail !! ", __FUNCTION__);
                        result = JPG_ERR_ALLOCATE_FAIL;
                        break;
                    }

                    pJOutBufInfo_2->pBufAddr = pJStreamDesc->jHeap_mem(
                                                    pHJCodec->pHInJStream,
                                                    JPG_HEAP_DISP_BUF_2,
                                                    pJOutBufInfo_2->bufLength,
                                                    &pJSysOutBufInfo_2->bufLength);
                    if( pJOutBufInfo_2->pBufAddr == 0 )
                    {
                        jpg_msg_ex(JPG_MSG_TYPE_ERR, " Allocate Line buf fail !! ", __FUNCTION__);
                        result = JPG_ERR_ALLOCATE_FAIL;
                        break;
                    }
                }
                break;
        }
    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;

}

static JPG_ERR
_jpg_free_disp_buf(
    JPG_COMM_DEV    *pJCommDev)
{
    // this function for win32
    JPG_ERR             result = JPG_ERR_OK;
    JPG_STREAM_DESC     *pJStreamDesc = &pJCommDev->hJCodec.pHInJStream->jStreamDesc;
    JPG_BUF_INFO        *pJOutBufInfo_0 = &pJCommDev->hJComm.jOutBufInfo[0];
    JPG_BUF_INFO        *pJOutBufInfo_1 = &pJCommDev->hJComm.jOutBufInfo[1];
    JPG_BUF_INFO        *pJOutBufInfo_2 = &pJCommDev->hJComm.jOutBufInfo[2];
    JPG_DISP_INFO       *pJDispInfo = pJCommDev->hJComm.pJDispInfo;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x\n", pJCommDev);

    do{
        JPG_CODEC_HANDLE    *pHJCodec = &pJCommDev->hJCodec;
        JPG_BUF_INFO        *pJSysOutBufInfo_0 = &pJCommDev->hJComm.jSysOutBufInfo[0];
        JPG_BUF_INFO        *pJSysOutBufInfo_1 = &pJCommDev->hJComm.jSysOutBufInfo[1];
        JPG_BUF_INFO        *pJSysOutBufInfo_2 = &pJCommDev->hJComm.jSysOutBufInfo[2];

        // allocate display buffer
        switch( pJDispInfo->outColorSpace )
        {
            case JPG_COLOR_SPACE_ARGB4444:
            case JPG_COLOR_SPACE_ARGB8888:
            case JPG_COLOR_SPACE_RGB565:
                _jpg_dump_vram(pHJCodec->pHInJStream,
                               JPG_STREAM_CMD_GET_VRAM_DISP_BUF_0,
                               pJSysOutBufInfo_0->pBufAddr, pJSysOutBufInfo_0->bufLength);

                if( pJStreamDesc->jFree_mem )
                {
                    if( pJOutBufInfo_0->pBufAddr )
                        pJStreamDesc->jFree_mem(pHJCodec->pHInJStream, JPG_HEAP_DISP_BUF_0, pJOutBufInfo_0->pBufAddr);

                    (*pJOutBufInfo_0) = (*pJSysOutBufInfo_0);
                }
                break;

            case JPG_COLOR_SPACE_YUV422:
            case JPG_COLOR_SPACE_YUV420:
                _jpg_dump_vram(pHJCodec->pHInJStream,
                               JPG_STREAM_CMD_GET_VRAM_DISP_BUF_0,
                               pJSysOutBufInfo_0->pBufAddr, pJSysOutBufInfo_0->bufLength);

                _jpg_dump_vram(pHJCodec->pHInJStream,
                               JPG_STREAM_CMD_GET_VRAM_DISP_BUF_1,
                               pJSysOutBufInfo_1->pBufAddr, pJSysOutBufInfo_1->bufLength);

                _jpg_dump_vram(pHJCodec->pHInJStream,
                               JPG_STREAM_CMD_GET_VRAM_DISP_BUF_2,
                               pJSysOutBufInfo_2->pBufAddr, pJSysOutBufInfo_2->bufLength);

                if( pJStreamDesc->jFree_mem )
                {
                    if( pJOutBufInfo_0->pBufAddr )
                        pJStreamDesc->jFree_mem(pHJCodec->pHInJStream, JPG_HEAP_DISP_BUF_0, pJOutBufInfo_0->pBufAddr);

                    (*pJOutBufInfo_0) = (*pJSysOutBufInfo_0);

                    if( pJOutBufInfo_1->pBufAddr )
                        pJStreamDesc->jFree_mem(pHJCodec->pHInJStream, JPG_HEAP_DISP_BUF_1, pJOutBufInfo_1->pBufAddr);

                    (*pJOutBufInfo_1) = (*pJSysOutBufInfo_1);

                    if( pJOutBufInfo_2->pBufAddr )
                        pJStreamDesc->jFree_mem(pHJCodec->pHInJStream, JPG_HEAP_DISP_BUF_2, pJOutBufInfo_2->pBufAddr);

                    (*pJOutBufInfo_2) = (*pJSysOutBufInfo_2);
                }
                break;
        }
    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;
}

#else
    #define _jpg_allocate_disp_buf(pJCommDev)       JPG_ERR_OK
    #define _jpg_free_disp_buf(pJCommDev)           JPG_ERR_OK
#endif

static JPG_ERR
_jpg_allocate_sys_bs_buf(
    JPG_COMM_DEV    *pJCommDev)
{
    JPG_ERR         result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x\n", pJCommDev);

    do{
        JPG_CODEC_HANDLE    *pHJCodec = &pJCommDev->hJCodec;
        JPG_STREAM_DESC     *pJStreamDesc = &pJCommDev->hJCodec.pHInJStream->jStreamDesc;
        JPG_HW_BS_CTRL      *pJHwBsCtrl = &pJCommDev->hJCodec.jHwBsCtrl;

        if( pJStreamDesc->jHeap_mem )
        {
            uint32_t    sysBsBufSize = 0;

            // allocate jpg system bs buffer
            if( pJStreamDesc->jControl )
                pJStreamDesc->jControl(pHJCodec->pHInJStream,
                                       (uint32_t)JPG_STREAM_CMD_GET_ENC_SYS_BS_BUF_SIZE,  //256KB
                                       &sysBsBufSize, 0);

            pHJCodec->pSysBsBuf = pJStreamDesc->jHeap_mem(pJCommDev->hJCodec.pHInJStream,
                                                          JPG_HEAP_ENC_SYS_BS_BUF, sysBsBufSize,
                                                          &pHJCodec->sysBsBufSize);
            if( pHJCodec->pSysBsBuf == 0 )
            {
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, allocate fail !!");
                result = JPG_ERR_ALLOCATE_FAIL;
                break;
            }
        }
    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;
}

static JPG_ERR
_jpg_allocate_jheader_buf(
    JPG_COMM_DEV    *pJCommDev)
{
    JPG_ERR         result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x\n", pJCommDev);

    do{
        JPG_CODEC_HANDLE    *pHJCodec = &pJCommDev->hJCodec;
        JPG_STREAM_DESC     *pJStreamDesc = &pJCommDev->hJCodec.pHInJStream->jStreamDesc;
        JPG_HW_BS_CTRL      *pJHwBsCtrl = &pJCommDev->hJCodec.jHwBsCtrl;

        if( pJStreamDesc->jHeap_mem )
        {
            uint32_t    hdrSize = 0;

            // allocate jpg header buffer
            // 6 marker types => total marker = 6*2
            hdrSize = (6*2 + JPG_ENC_DQT_LENGTH + JPG_ENC_DRI_LENGTH + JPG_ENC_SOF_LENGTH + JPG_ENC_DHT_LENGTH + JPG_ENC_SOS_LENGTH);
            pJHwBsCtrl->pJHdrInfo = pJStreamDesc->jHeap_mem(pJCommDev->hJCodec.pHInJStream,
                                                            JPG_HEAP_DEF, hdrSize, &pJHwBsCtrl->jHdrInfoSize);
            if( pJHwBsCtrl->pJHdrInfo == 0 ||
                hdrSize != pJHwBsCtrl->jHdrInfoSize )
            {
                jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, allocate fail !!");
                result = JPG_ERR_ALLOCATE_FAIL;
                break;
            }
        }
    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;
}

static void
_jpg_dec_color_fmt_transform(
    JPG_COMM_DEV    *pJCommDev)
{
    JPG_CODEC_HANDLE    *pHJCodec = &pJCommDev->hJCodec;
    JPG_FRM_COMP        *pJFrmComp = &pJCommDev->hJCodec.jFrmCompInfo;
    JPG_DISP_INFO       *pJDispInfo = pJCommDev->hJComm.pJDispInfo;
    JPG_LINE_BUF_INFO   *pJLineBufInfo = &pJCommDev->hJCodec.jLineBufInfo;
    uint32_t            minSliceNum = 0, tmpNum = 0;
    uint32_t            sliceNum_444 = 0, sliceNum_422 = 0, sliceNum_420 = 0;

    // 1. This function is used for down sample output (To reduce line buffer size)
    // 2. Those magic numbers are the result of test (Base on line buffer = 974000 bytes).

	printf("%s\n",__FUNCTION__);
    if( pJDispInfo->rotType == JPG_ROT_TYPE_90 || pJDispInfo->rotType == JPG_ROT_TYPE_270 )
    {
        minSliceNum = 28;
    }
    else
    {
        if( pJFrmComp->imgHeight > 5000 )
            minSliceNum = 26;
        else if( pJFrmComp->imgHeight > 4400 )
            minSliceNum = 24;
        else if( pJFrmComp->imgHeight > 3600 )
            minSliceNum = 22;
        else if( pJFrmComp->imgHeight > 3400 )
            minSliceNum = 20;
        else
            minSliceNum = 18;
    }

    if( pJFrmComp->decColorSpace == JPG_COLOR_SPACE_YUV411 )
        pHJCodec->ctrlFlag |= JPG_FLAGS_DEC_UV_HOR_DUPLICATE;

    if( ((pJDispInfo->rotType == JPG_ROT_TYPE_90 || pJDispInfo->rotType == JPG_ROT_TYPE_270) &&
         ((pJDispInfo->srcW >= (JPG_DC_ONLY_FACTOR * pJDispInfo->dstH)) ||
          (pJDispInfo->srcH >= (JPG_DC_ONLY_FACTOR * pJDispInfo->dstW))))
     || ((pJDispInfo->rotType == JPG_ROT_TYPE_0 || pJDispInfo->rotType == JPG_ROT_TYPE_180) &&
         ((pJDispInfo->srcW >= (JPG_DC_ONLY_FACTOR * pJDispInfo->dstW)) ||
          (pJDispInfo->srcH >= (JPG_DC_ONLY_FACTOR * pJDispInfo->dstH)))) )
    {
        pHJCodec->ctrlFlag &= 0x0000FFFF;
        pHJCodec->ctrlFlag |= JPG_FLAGS_DEC_DC_ONLY;
    }
    else
    {
        tmpNum = pJLineBufInfo->size / (((pJFrmComp->imgWidth+0x7) & ~0x7) << 2);

        sliceNum_444 = tmpNum / 6;
        sliceNum_422 = tmpNum / 4;
        sliceNum_420 = tmpNum / 3;

        if( sliceNum_422 >= minSliceNum && sliceNum_444 < minSliceNum )
        {
            if( pJFrmComp->decColorSpace == JPG_COLOR_SPACE_YUV444 )
            {
                pHJCodec->ctrlFlag |= JPG_FLAGS_DEC_UV_HOR_DOWNSAMPLE;
                pJFrmComp->decColorSpace = JPG_COLOR_SPACE_YUV422;
            }
        }
        else if ( sliceNum_420 >= minSliceNum && sliceNum_422 < minSliceNum )
        {
            switch( pJFrmComp->decColorSpace )
            {
                case JPG_COLOR_SPACE_YUV444:
                    pHJCodec->ctrlFlag |= (JPG_FLAGS_DEC_UV_HOR_DOWNSAMPLE | JPG_FLAGS_DEC_UV_VER_DOWNSAMPLE);
                    break;

                case JPG_COLOR_SPACE_YUV411:
                case JPG_COLOR_SPACE_YUV422:
                    pHJCodec->ctrlFlag |= JPG_FLAGS_DEC_UV_VER_DOWNSAMPLE;
                    break;

                case JPG_COLOR_SPACE_YUV422R:
                    pHJCodec->ctrlFlag |= JPG_FLAGS_DEC_UV_HOR_DOWNSAMPLE;
                    break;
            }
            pJFrmComp->decColorSpace = JPG_COLOR_SPACE_YUV420;

        }
        else if( sliceNum_420 < minSliceNum )
        {
            //org => YUV H drop => UV V drop
            //444 => 444 => 422R
            //422 => 422 => 420
            //422R => 422R => (x)
            //411 => (x) => (x)
            //420 => 420 => (x)
            pHJCodec->ctrlFlag |= (JPG_FLAGS_DEC_Y_HOR_DOWNSAMPLE | JPG_FLAGS_DEC_UV_HOR_DOWNSAMPLE);
            tmpNum = pJLineBufInfo->size / (((((pJFrmComp->imgWidth >> 1)) + 0x7) & ~0x7) << 2);

            sliceNum_444 = tmpNum / 6;
            sliceNum_422 = tmpNum / 4;
            sliceNum_420 = tmpNum / 3;

            if( sliceNum_422 >= minSliceNum && sliceNum_444 < minSliceNum )
            {
                if( pJFrmComp->decColorSpace == JPG_COLOR_SPACE_YUV444 )
                {
                    pHJCodec->ctrlFlag |= JPG_FLAGS_DEC_UV_VER_DOWNSAMPLE;
                    pJFrmComp->decColorSpace = JPG_COLOR_SPACE_YUV422R;
                }
            }
            else if( sliceNum_420 >= minSliceNum && sliceNum_422 < minSliceNum )
            {
                if( pJFrmComp->decColorSpace == JPG_COLOR_SPACE_YUV422 )
                {
                    pHJCodec->ctrlFlag |= JPG_FLAGS_DEC_UV_VER_DOWNSAMPLE;
                    pJFrmComp->decColorSpace = JPG_COLOR_SPACE_YUV420;
                }
                else if( pJFrmComp->decColorSpace != JPG_COLOR_SPACE_YUV420 )
                {
                    pHJCodec->ctrlFlag &= 0x0000FFFF;
                    pHJCodec->ctrlFlag |= JPG_FLAGS_DEC_DC_ONLY;
                }
            }
            else if( sliceNum_420 < minSliceNum )
            {
                pHJCodec->ctrlFlag &= 0x0000FFFF;
                pHJCodec->ctrlFlag |= JPG_FLAGS_DEC_DC_ONLY;
            }
        }

        if( (pHJCodec->ctrlFlag & JPG_FLAGS_DEC_DC_ONLY) == 0 &&
            pJFrmComp->decColorSpace == JPG_COLOR_SPACE_YUV444 &&
            (sliceNum_444 < (minSliceNum + 2)) )
        {
            pHJCodec->ctrlFlag |= JPG_FLAGS_DEC_UV_HOR_DOWNSAMPLE;
            pJFrmComp->decColorSpace = JPG_COLOR_SPACE_YUV422;
        }
    }

    return;
}


static JPG_ERR
_jpg_dec_set_color_format(
    JPG_COMM_DEV    *pJCommDev)
{
    JPG_ERR             result = JPG_ERR_OK;
    JPG_FRM_COMP        *pJFrmComp = &pJCommDev->hJCodec.jFrmCompInfo;
    JCOMM_INIT_PARAM    *pJCommInitParam = &pJCommDev->jCommInitParam;

    /**
     * Support format
     *         Y         U         V
     *       H   V     H   V     H   V
     * 444   1   1     1   1     1   1
     * 422   2   1     1   1     1   1
     * 422R  1   2     1   1     1   1
     * 420   2   2     1   1     1   1
     * 411   4   1     1   1     1   1
     */
    pJFrmComp->widthUnit  = (pJFrmComp->jFrmInfo[0].horizonSamp << 3);
    pJFrmComp->heightUnit = (pJFrmComp->jFrmInfo[0].verticalSamp << 3);

    jpg_msg(0, "Image Sample Frequency = (%d %d %d %d %d %d)  \n",
            pJFrmComp->jFrmInfo[0].horizonSamp,
            pJFrmComp->jFrmInfo[0].verticalSamp,
            pJFrmComp->jFrmInfo[1].horizonSamp,
            pJFrmComp->jFrmInfo[1].verticalSamp,
            pJFrmComp->jFrmInfo[2].horizonSamp,
            pJFrmComp->jFrmInfo[2].verticalSamp,
            pJFrmComp->jFrmInfo[3].horizonSamp,
            pJFrmComp->jFrmInfo[3].verticalSamp);

    pJFrmComp->realWidth  = (pJFrmComp->imgWidth  + (pJFrmComp->widthUnit - 1)) & ~(pJFrmComp->widthUnit - 1);
    pJFrmComp->realHeight = (pJFrmComp->imgHeight + (pJFrmComp->heightUnit - 1)) & ~(pJFrmComp->heightUnit - 1);

    do{
        if( pJFrmComp->bSingleChannel )
        {
            pJFrmComp->decColorSpace = JPG_COLOR_SPACE_YUV444;
            break;
        }

        if( (pJFrmComp->jFrmInfo[0].horizonSamp == 1) &&
            (pJFrmComp->jFrmInfo[0].verticalSamp == 1) )
        {
#if (CFG_CHIP_FAMILY == 9850)
            pJCommDev->hJCodec.ctrlFlag |= JPG_FLAGS_DEC_UV_HOR_DOWNSAMPLE;
            pJFrmComp->decColorSpace = JPG_COLOR_SPACE_YUV422;
#else
            pJFrmComp->decColorSpace = JPG_COLOR_SPACE_YUV444;
#endif            
        }
        else if( (pJFrmComp->jFrmInfo[0].horizonSamp == 2) &&
                 (pJFrmComp->jFrmInfo[0].verticalSamp == 1) )
        {
            pJFrmComp->decColorSpace = JPG_COLOR_SPACE_YUV422;
        }
        else if( (pJFrmComp->jFrmInfo[0].horizonSamp == 1) &&
                 (pJFrmComp->jFrmInfo[0].verticalSamp == 2) )
        {
            pJFrmComp->decColorSpace = JPG_COLOR_SPACE_YUV422R;
        }
        else if( (pJFrmComp->jFrmInfo[0].horizonSamp == 2) &&
                 (pJFrmComp->jFrmInfo[0].verticalSamp == 2) )
        {
            if( (pJFrmComp->jFrmInfo[1].horizonSamp == 1) &&
                (pJFrmComp->jFrmInfo[1].verticalSamp == 2) )
                pJFrmComp->decColorSpace = JPG_COLOR_SPACE_YUV422;
            else
            {
#if (CFG_CHIP_FAMILY == 9850)
				pJCommDev->hJCodec.ctrlFlag |= JPG_FLAGS_DEC_UV_VER_DUPLICATE;
				pJFrmComp->decColorSpace = JPG_COLOR_SPACE_YUV422;
#else
				pJFrmComp->decColorSpace = JPG_COLOR_SPACE_YUV420; 
#endif            
            }
        }
        else if( (pJFrmComp->jFrmInfo[0].horizonSamp == 4) &&
                 (pJFrmComp->jFrmInfo[0].verticalSamp == 1) )
        {
            //pJFrmComp->decColorSpace = JPG_COLOR_SPACE_YUV411; 
            //#if (CFG_CHIP_FAMILY == 9850) ,now I just test 411->422 for 9850 project, but I think 9070 is also too.
                   pJCommDev->hJCodec.ctrlFlag |= JPG_FLAGS_DEC_UV_HOR_DUPLICATE;
                   pJFrmComp->decColorSpace = JPG_COLOR_SPACE_YUV422;
			//#endif          
        }
        else
        {
            result = JPG_ERR_WRONG_COLOR_FORMAT;
        }
    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
    }
    return result;
}

static JPG_ERR
_jpg_dec_set_frm_size(
    JPG_COMM_DEV    *pJCommDev)
{
    JPG_ERR             result = JPG_ERR_OK;
    JPG_FRM_SIZE_INFO   *pJFrmSizeInfo = &pJCommDev->hJCodec.jFrmSizeInfo;
    JPG_FRM_COMP        *pJFrmCompInfo = &pJCommDev->hJCodec.jFrmCompInfo;
    JPG_DISP_INFO       *pJDispInfo = pJCommDev->hJComm.pJDispInfo;

    do{
        pJFrmSizeInfo->realWidth  = pJFrmCompInfo->realWidth;
        pJFrmSizeInfo->realHeight = pJFrmCompInfo->realHeight;

        pJFrmSizeInfo->mcuRealWidth  = pJFrmSizeInfo->realWidth / pJFrmCompInfo->widthUnit;
        pJFrmSizeInfo->mcuRealHeight = pJFrmSizeInfo->realHeight / pJFrmCompInfo->heightUnit;

        pJFrmSizeInfo->startX     = pJDispInfo->srcX;
        pJFrmSizeInfo->startY     = pJDispInfo->srcY;
        pJFrmSizeInfo->dispWidth  = pJDispInfo->srcW;
        pJFrmSizeInfo->dispHeight = pJDispInfo->srcH;

        // verify range
        if( (pJFrmSizeInfo->startX + pJFrmSizeInfo->dispWidth) > pJFrmSizeInfo->realWidth )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "X = %d, dispWidth = %d, realWidth = %d ! ",
                pJFrmSizeInfo->startX, pJFrmSizeInfo->dispWidth, pJFrmSizeInfo->realWidth);
            result = JPG_ERR_INVALID_PARAMETER;
            break;
        }

        if( (pJFrmSizeInfo->startY + pJFrmSizeInfo->dispHeight) > pJFrmSizeInfo->realHeight )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "X = %d, dispWidth = %d, realWidth = %d ! ",
                pJFrmSizeInfo->startY, pJFrmSizeInfo->dispHeight, pJFrmSizeInfo->realHeight);
            result = JPG_ERR_INVALID_PARAMETER;
            break;
        }

        // calculate MCU boundary
        pJFrmSizeInfo->mcuDispLeft = pJFrmSizeInfo->startX / pJFrmCompInfo->widthUnit;
        pJFrmSizeInfo->mcuDispUp   = pJFrmSizeInfo->startY / pJFrmCompInfo->heightUnit;

        if( pJFrmSizeInfo->dispWidth && pJFrmSizeInfo->dispHeight )
        {
            pJFrmSizeInfo->mcuDispWidth = pJFrmSizeInfo->dispWidth / pJFrmCompInfo->widthUnit;
            if( pJFrmSizeInfo->dispWidth % pJFrmCompInfo->widthUnit )
                pJFrmSizeInfo->mcuDispWidth++;

            pJFrmSizeInfo->mcuDispHeight = pJFrmSizeInfo->dispHeight / pJFrmCompInfo->heightUnit;
            if( pJFrmSizeInfo->dispHeight % pJFrmCompInfo->heightUnit )
                pJFrmSizeInfo->mcuDispHeight++;
        }
        else
        {
            pJFrmSizeInfo->mcuDispWidth  = pJFrmSizeInfo->mcuRealWidth;
            pJFrmSizeInfo->mcuDispHeight = pJFrmSizeInfo->mcuRealHeight;

            pJFrmSizeInfo->dispWidth  = pJFrmCompInfo->imgWidth;
            pJFrmSizeInfo->dispHeight = pJFrmCompInfo->imgHeight;
        }

        pJFrmSizeInfo->mcuDispRight = pJFrmSizeInfo->mcuDispLeft + pJFrmSizeInfo->mcuDispWidth;
        pJFrmSizeInfo->mcuDispDown  = pJFrmSizeInfo->mcuDispUp + pJFrmSizeInfo->mcuDispHeight;

        if( pJFrmSizeInfo->mcuDispRight > pJFrmSizeInfo->mcuRealWidth ||
            pJFrmSizeInfo->mcuDispDown > pJFrmSizeInfo->mcuRealHeight )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "err, mcuDispRight=%d > mcuRealWidth=%d or mcuDispDown=%d > mcuRealHeight=%d !\n",
                pJFrmSizeInfo->mcuDispRight, pJFrmSizeInfo->mcuRealWidth,
                pJFrmSizeInfo->mcuDispDown, pJFrmSizeInfo->mcuRealHeight);
            result = JPG_ERR_INVALID_PARAMETER;
            break;
        }
    }while(0);

    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
    }
    return result;
}

static JPG_ERR
_jpg_dec_pre_setting(
    JPG_COMM_DEV    *pJCommDev)
{
    JPG_ERR             result = JPG_ERR_OK;
    JPG_CODEC_HANDLE    *pHJCodec = &pJCommDev->hJCodec;
    JPG_DISP_INFO       *pJDispInfo = pJCommDev->hJComm.pJDispInfo;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x\n", pJCommDev);

    do{
        JPG_FRM_COMP        *pJFrmComp = &pJCommDev->hJCodec.jFrmCompInfo;
        JPG_HW_CTRL         *pJHwCtrl = &pJCommDev->hJCodec.jHwCtrl;

        // check decoded jpg color format
        result = _jpg_dec_set_color_format(pJCommDev);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
            break;
        }

#if 0
        /**
         * transform color format. Long time ago, this be used to workaround Isp and Jpg H/W handshaking.
         * Now, we use this for performance (down sample color format).
         **/
        _jpg_dec_color_fmt_transform(pJCommDev);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
            break;
        }

//#else //Benson mark JPG_DEC_DC_ONLY mode , because IT9970 not support it.

        if( ((pJDispInfo->rotType == JPG_ROT_TYPE_90 || pJDispInfo->rotType == JPG_ROT_TYPE_270) &&
             ((pJDispInfo->srcW >= (JPG_DC_ONLY_FACTOR * pJDispInfo->dstH)) ||
              (pJDispInfo->srcH >= (JPG_DC_ONLY_FACTOR * pJDispInfo->dstW))))
          ||
            ((pJDispInfo->rotType == JPG_ROT_TYPE_0 || pJDispInfo->rotType == JPG_ROT_TYPE_180) &&
             ((pJDispInfo->srcW >= (JPG_DC_ONLY_FACTOR * pJDispInfo->dstW)) ||
              (pJDispInfo->srcH >= (JPG_DC_ONLY_FACTOR * pJDispInfo->dstH)))) )
        {
            pHJCodec->ctrlFlag &= 0x0000FFFF;
            pHJCodec->ctrlFlag |= JPG_FLAGS_DEC_DC_ONLY;
            jpg_msg_ex(1, "\n\tEnable DC only mode !!");
        }
#endif
        // check mcu info in a frame
        result = _jpg_dec_set_frm_size(pJCommDev);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
            break;
        }

        // Y: huffman table
        pJHwCtrl->dcHuffTable[0] = pJFrmComp->huff_DC[0].pHuffmanTable;
        pJHwCtrl->acHuffTable[0] = pJFrmComp->huff_AC[0].pHuffmanTable;
        pJHwCtrl->dcHuffW2talCodeLenCnt[0] = pJFrmComp->huff_DC[0].totalCodeLenCnt;
        pJHwCtrl->acHuffW2talCodeLenCnt[0] = pJFrmComp->huff_AC[0].totalCodeLenCnt;

        // UV: huffman table
        pJHwCtrl->dcHuffTable[1] = pJFrmComp->huff_DC[1].pHuffmanTable;
        pJHwCtrl->acHuffTable[1] = pJFrmComp->huff_AC[1].pHuffmanTable;
        pJHwCtrl->dcHuffW2talCodeLenCnt[1] = pJFrmComp->huff_DC[1].totalCodeLenCnt;
        pJHwCtrl->acHuffW2talCodeLenCnt[1] = pJFrmComp->huff_AC[1].totalCodeLenCnt;

        // Q table
        pJHwCtrl->qTableY = pJFrmComp->qTable.table[pJFrmComp->jFrmInfo[0].qTableSel];
        if( pJFrmComp->qTable.tableCnt == 1 )
        {
            // only one Q table case
            pJHwCtrl->qTableUv = pJFrmComp->qTable.table[pJFrmComp->jFrmInfo[0].qTableSel];
            pJFrmComp->qTable.tableCnt = 2;
        }
        else
            pJHwCtrl->qTableUv = pJFrmComp->qTable.table[pJFrmComp->jFrmInfo[1].qTableSel];

    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;
}

#if (CONFIG_JPG_CODEC_DESC_ENC_MJPG_DESC) || (CONFIG_JPG_CODEC_DESC_ENCODER_DESC)
static uint8_t*
_jpg_enc_gen_DQT(
    uint8_t         *ptCur,
    JPG_FRM_COMP    *pJFrmComp)
{
    uint16_t    qTabCnt = 2;
    uint16_t    segLeng = JPG_ENC_DQT_LENGTH;

    SET_WORD_VALUE(ptCur, 0xFFDB); // FF DB => marker
    SET_WORD_VALUE(ptCur, segLeng);
    *ptCur = 0x00;
    SET_HIGH_BYTE_VALUE(ptCur, 0);
    SET_LOW_BYTE_VALUE(ptCur, 0);
    // skip the precision & destination field identifier of Q table
    ptCur++;

    memcpy(ptCur, &pJFrmComp->qTable.table[0][0], 64); // 64 = JPG_Q_TABLE_ELEMENT_NUM
    ptCur += 64;

    *ptCur = 0x00;
    SET_HIGH_BYTE_VALUE(ptCur, 0);
    SET_LOW_BYTE_VALUE(ptCur, 1);
    // skip the precision & destination field identifier of Q table
    ptCur++;
    memcpy(ptCur, &pJFrmComp->qTable.table[1][0], 64); // 64 = JPG_Q_TABLE_ELEMENT_NUM
    ptCur += 64;

    return ptCur;
}

static uint8_t*
_jpg_enc_gen_DRI(
    uint8_t   *ptCur,
    uint16_t  restartInterval)
{
    uint16_t    segLeng = JPG_ENC_DRI_LENGTH;

    // to do :
    SET_WORD_VALUE(ptCur, 0xFFDD); // FF DD => marker
    SET_WORD_VALUE(ptCur, segLeng);
    SET_WORD_VALUE(ptCur, restartInterval);

    return ptCur;
}

static uint8_t*
_jpg_enc_gen_SOF00(
    uint8_t         *ptCur,
    JPG_FRM_COMP    *pJFrmComp)
{
    uint8_t     compCnt = 3;
    uint16_t    segLeng = JPG_ENC_SOF_LENGTH;
    int         i;

    SET_WORD_VALUE(ptCur, 0xFFC0); // FF C0 => marker
    SET_WORD_VALUE(ptCur, segLeng);

    // set "8" for the sample precision field of scan segment
    SET_BYTE_VALUE(ptCur, 8);

    SET_WORD_VALUE(ptCur, pJFrmComp->imgHeight);
    SET_WORD_VALUE(ptCur, pJFrmComp->imgWidth);

    SET_BYTE_VALUE(ptCur, compCnt);

    for(i = 0; i < compCnt; i++)
    {
        SET_BYTE_VALUE(ptCur, (i + 1));

        *ptCur = 0x00;
        SET_HIGH_BYTE_VALUE(ptCur, pJFrmComp->jFrmInfo[i].horizonSamp);
        SET_LOW_BYTE_VALUE(ptCur, pJFrmComp->jFrmInfo[i].verticalSamp);
        ptCur++;

        SET_BYTE_VALUE(ptCur, (uint8_t)pJFrmComp->jFrmInfo[i].qTableSel);
    }

    return ptCur;
}

static uint8_t*
_jpg_enc_gen_DHT(
    uint8_t         *ptCur,
    JPG_FRM_COMP    *pJFrmComp)
{
    uint16_t    HTabSel = 0;
    uint16_t    segLeng = JPG_ENC_DHT_LENGTH;

    SET_WORD_VALUE(ptCur, 0xFFC4); // FF C4 => marker
    SET_WORD_VALUE(ptCur, segLeng);

    for(HTabSel = 0; HTabSel < 2; HTabSel++)
    {
        // DC Huffman table
        *ptCur = 0x00;
        SET_HIGH_BYTE_VALUE(ptCur, 0);
        SET_LOW_BYTE_VALUE(ptCur, HTabSel);

        ptCur++;

        // copy "Number of Huffman codes of length i"
        memcpy(ptCur, &Def_DCHuffTable[HTabSel][0], 28);

        pJFrmComp->huff_DC[HTabSel].pHuffmanTable = (uint8_t*)(&Def_DCHuffTable[HTabSel][0]);
        pJFrmComp->huff_DC[HTabSel].totalCodeLenCnt = 12;    // 12 = 28 - 16
        ptCur += 28;

        // AC Huffman table
        *ptCur = 0x00;
        SET_HIGH_BYTE_VALUE(ptCur, 1);
        SET_LOW_BYTE_VALUE(ptCur, HTabSel);

        ptCur++;

        // copy "Number of Huffman codes of length i"
        memcpy(ptCur, &Def_ACHuffTable[HTabSel][0], 178);

        pJFrmComp->huff_AC[HTabSel].pHuffmanTable = (uint8_t*)(&Def_ACHuffTable[HTabSel][0]);
        pJFrmComp->huff_AC[HTabSel].totalCodeLenCnt = 162;   // 162 = 178 - 16
        ptCur += 178;
    }

    return ptCur;

}

static uint8_t*
_jpg_enc_gen_SOS(
    uint8_t         *ptCur,
    JPG_FRM_COMP    *pJFrmComp)
{
    uint8_t     compCnt = 3;
    uint16_t    segLeng = JPG_ENC_SOS_LENGTH;
    int         i;

    SET_WORD_VALUE(ptCur, 0xFFDA); // FF DA => marker
    SET_WORD_VALUE(ptCur, segLeng);

    SET_BYTE_VALUE(ptCur, compCnt);

    for(i = 0; i < compCnt; i++)
    {
        SET_BYTE_VALUE(ptCur, (uint8_t)(i + 1));

        *ptCur = 0x00;

        SET_HIGH_BYTE_VALUE(ptCur, pJFrmComp->jFrmInfo[i].dcHuffTableSel);
        SET_LOW_BYTE_VALUE(ptCur, pJFrmComp->jFrmInfo[i].acHuffTableSel);
        ptCur++;
    }

    // skip Ss
    *ptCur = 0x00;
    ptCur++;

    // set Se to 63
    SET_BYTE_VALUE(ptCur, 63);

    // skip Ah, Al
    *ptCur = 0x00;
    ptCur++;

    return ptCur;
}

static JPG_ERR
_jpg_enc_gen_header(
    JPG_COMM_DEV    *pJCommDev)
{
    JPG_ERR             result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x\n", pJCommDev);

    do{
        JPG_FRM_COMP        *pJFrmComp = &pJCommDev->hJCodec.jFrmCompInfo;
        JPG_HW_BS_CTRL      *pJHwBsCtrl = &pJCommDev->hJCodec.jHwBsCtrl;
        uint8_t             *pCur = 0, *pEnd = 0;

        if( !pJHwBsCtrl->pJHdrInfo )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, No jpg header buf !! ");
            result = JPG_ERR_NULL_POINTER;
            break;
        }

        pCur = pJHwBsCtrl->pJHdrInfo;
        pEnd = pJHwBsCtrl->pJHdrInfo + pJHwBsCtrl->jHdrInfoSize;

        SET_WORD_VALUE(pCur, 0xFFD8); // FF D8 => marker

        pCur = _jpg_enc_gen_DQT(pCur, pJFrmComp);
        if( pCur > pEnd )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Out buffer !");
            result = JPG_ERR_INVALID_PARAMETER;
            break;
        }

        pCur = _jpg_enc_gen_DRI(pCur, pJFrmComp->restartInterval);
        if( pCur > pEnd )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Out buffer !");
            result = JPG_ERR_INVALID_PARAMETER;
            break;
        }

        pCur = _jpg_enc_gen_SOF00(pCur, pJFrmComp);
        if( pCur > pEnd )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Out buffer !");
            result = JPG_ERR_INVALID_PARAMETER;
            break;
        }

        pCur = _jpg_enc_gen_DHT(pCur, pJFrmComp);
        if( pCur > pEnd )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Out buffer !");
            result = JPG_ERR_INVALID_PARAMETER;
            break;
        }

        pCur = _jpg_enc_gen_SOS(pCur, pJFrmComp);
        if( pCur > pEnd )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " Out buffer !");
            result = JPG_ERR_INVALID_PARAMETER;
            break;
        }
    }while(0);


    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;
}

static JPG_ERR
_jpg_enc_set_comp_select(
    JPG_FRM_COMP    *pJFrmComp)
{
    JPG_ERR     result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x\n", pJFrmComp);

    do{
        if( pJFrmComp->encColorSpace != JPG_COLOR_SPACE_YUV422 &&
            pJFrmComp->encColorSpace != JPG_COLOR_SPACE_YUV420 )
        {
            result = JPG_ERR_HW_NOT_SUPPORT;
            break;
        }

        pJFrmComp->jFrmInfo[1].horizonSamp  = 1;
        pJFrmComp->jFrmInfo[1].verticalSamp = 1;

        pJFrmComp->jFrmInfo[2].horizonSamp  = 1;
        pJFrmComp->jFrmInfo[2].verticalSamp = 1;

        switch( pJFrmComp->encColorSpace )
        {
            case JPG_COLOR_SPACE_YUV422:
                pJFrmComp->jFrmInfo[0].horizonSamp  = 2;
                pJFrmComp->jFrmInfo[0].verticalSamp = 1;
                break;

            case JPG_COLOR_SPACE_YUV420:
                pJFrmComp->jFrmInfo[0].horizonSamp  = 2;
                pJFrmComp->jFrmInfo[0].verticalSamp = 2;
                break;
        }

        pJFrmComp->jFrmInfo[0].qTableSel      = 0;
        pJFrmComp->jFrmInfo[0].dcHuffTableSel = 0;
        pJFrmComp->jFrmInfo[0].acHuffTableSel = 0;

        pJFrmComp->jFrmInfo[1].qTableSel      = 1;
        pJFrmComp->jFrmInfo[1].dcHuffTableSel = 1;
        pJFrmComp->jFrmInfo[1].acHuffTableSel = 1;

        pJFrmComp->jFrmInfo[2].qTableSel      = 1;
        pJFrmComp->jFrmInfo[2].dcHuffTableSel = 1;
        pJFrmComp->jFrmInfo[2].acHuffTableSel = 1;

    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;
}

static JPG_ERR
_jpg_enc_set_q_table(
    JPG_FRM_COMP    *pJFrmComp,
    uint32_t        quality)
{
    JPG_ERR         result = JPG_ERR_OK;
    uint8_t         *pQTable_Y = 0;
    uint8_t         *pQTable_UV = 0;
    int             i;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x, %d\n", pJFrmComp, quality);

    pQTable_Y  = &pJFrmComp->qTable.table[0][0];
    pQTable_UV = &pJFrmComp->qTable.table[1][0];

    if( quality == 1 )
    {
        for(i = 0; i < JPG_Q_TABLE_SIZE; i++)
        {
            pQTable_Y[i]  = Def_QTable_Y[i];
            pQTable_UV[i] = Def_QTable_UV[i];
        }
    }
    else
    {
        quality = (quality < 50) ? (5000 / quality) : (200 - (quality << 1));

        // adjust Q table
        for(i = 0; i < JPG_Q_TABLE_SIZE; i++)
        {
            // luminance
            pQTable_Y[i] = (Def_QTable_Y[i] * quality + 50) / 100;
            pQTable_Y[i] = (pQTable_Y[i]) ? pQTable_Y[i] : 1;

            // chrominance
            pQTable_UV[i] = (Def_QTable_UV[i] * quality + 50) / 100;
            pQTable_UV[i] = (pQTable_UV[i]) ? pQTable_UV[i] : 1;
        }
    }

    pJFrmComp->qTable.tableCnt = 2;

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return JPG_ERR_OK;
}

static JPG_ERR
_jpg_enc_pre_setting(
    JPG_COMM_DEV    *pJCommDev)
{
    JPG_ERR             result = JPG_ERR_OK;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x\n", pJCommDev);

    do{
        JPG_STREAM_DESC     *pJStreamDesc = &pJCommDev->hJCodec.pHInJStream->jStreamDesc;
        JPG_HW_BS_CTRL      *pJHwBsCtrl = &pJCommDev->hJCodec.jHwBsCtrl;
        JPG_CODEC_HANDLE    *pHJCodec = &pJCommDev->hJCodec;
        JPG_FRM_SIZE_INFO   *pJFrmSizeInfo = &pJCommDev->hJCodec.jFrmSizeInfo;
        JPG_FRM_COMP        *pJFrmCompInfo = &pJCommDev->hJCodec.jFrmCompInfo;
        JPG_HW_CTRL         *pJHwCtrl = &pJCommDev->hJCodec.jHwCtrl;

        //---------------------------------
        // generate jpg header info
        pJFrmCompInfo->encColorSpace = pJCommDev->jCommInitParam.encColorSpace;
        pJFrmCompInfo->imgWidth      = pJCommDev->jCommInitParam.encWidth;
        pJFrmCompInfo->imgHeight     = pJCommDev->jCommInitParam.encHeight;

        // set encode image component info
        _jpg_enc_set_comp_select(pJFrmCompInfo);
        pJFrmCompInfo->widthUnit  = (pJFrmCompInfo->jFrmInfo[0].horizonSamp << 3);
        pJFrmCompInfo->heightUnit = (pJFrmCompInfo->jFrmInfo[0].verticalSamp << 3);
        pJFrmCompInfo->realWidth  = (pJFrmCompInfo->imgWidth  + (pJFrmCompInfo->widthUnit - 1)) & ~(pJFrmCompInfo->widthUnit - 1);
        pJFrmCompInfo->realHeight = (pJFrmCompInfo->imgHeight + (pJFrmCompInfo->heightUnit - 1)) & ~(pJFrmCompInfo->heightUnit - 1);
        pJFrmCompInfo->compNum = 3;
        pJFrmCompInfo->restartInterval = 0;

        // set valid frame size
#if 1
        pJFrmSizeInfo->realWidth     = pJFrmCompInfo->realWidth;
        pJFrmSizeInfo->realHeight    = pJFrmCompInfo->realHeight;

        pJFrmSizeInfo->mcuRealWidth  = pJFrmSizeInfo->realWidth / pJFrmCompInfo->widthUnit;
        pJFrmSizeInfo->mcuRealHeight = pJFrmSizeInfo->realHeight / pJFrmCompInfo->heightUnit;

        pJFrmSizeInfo->mcuDispWidth  = pJFrmSizeInfo->mcuRealWidth;
        pJFrmSizeInfo->mcuDispHeight = pJFrmSizeInfo->mcuRealHeight;
#else
        JPG_DISP_INFO       *pJDispInfo = pJCommDev->hJComm.pJDispInfo;
        // check mcu info in a frame
        pJDispInfo->srcX = 0;
        pJDispInfo->srcY = 0;
        pJDispInfo->srcW = pJShare2Isp->width;
        pJDispInfo->srcH = pJShare2Isp->height;
        result = _jpg_dec_set_frm_size(pJCommDev);
        if( result != JPG_ERR_OK )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
            break;
        }
#endif
        // set huffman table
        pJHwCtrl->dcHuffTable[0] = (uint8_t*)&Def_DCHuffTable[0][0];
        pJHwCtrl->acHuffTable[0] = (uint8_t*)&Def_ACHuffTable[0][0];
        pJHwCtrl->dcHuffTable[1] = (uint8_t*)&Def_DCHuffTable[1][0];
        pJHwCtrl->acHuffTable[1] = (uint8_t*)&Def_ACHuffTable[1][0];

        pJHwCtrl->dcHuffW2talCodeLenCnt[0] = 12;
        pJHwCtrl->acHuffW2talCodeLenCnt[0] = 162;
        pJHwCtrl->dcHuffW2talCodeLenCnt[1] = 12;
        pJHwCtrl->acHuffW2talCodeLenCnt[1] = 162;

        // set Q table
        _jpg_enc_set_q_table(pJFrmCompInfo, pJCommDev->jCommInitParam.encQuality);

        _jpg_enc_gen_header(pJCommDev);
    }while(0);

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;
}
#else
    #define _jpg_enc_pre_setting        0
#endif
//=============================================================================
//                  Public Function Definition
//=============================================================================
JPG_ERR
jComm_CreateHandle(
    JCOMM_HANDLE        **pHJComm,
    void                *extraData)
{
    JPG_ERR         result = JPG_ERR_OK;
    JPG_COMM_DEV    *pJCommDev = 0;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x, 0x%x\n", pHJComm, extraData);

    do{
        if( *pHJComm != 0 )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, Exist handle !!");
            result = JPG_ERR_INVALID_PARAMETER;
            break;
        }

        // ------------------------
        // craete dev info
        pJCommDev = jpg_malloc(sizeof(JPG_COMM_DEV));
        if( !pJCommDev )
        {
            jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, allocate fail !!");
            result = JPG_ERR_ALLOCATE_FAIL;
            break;
        }
        if(pJCommDev)
            memset(pJCommDev, 0x0, sizeof(JPG_COMM_DEV));

        pJCommDev->hJCodec.bInitialed = false;

        //------------------------------
        // register codec descriptor
        _jpg_register_all();

        (*pHJComm) = &pJCommDev->hJComm;
    }while(0);

    if( result != JPG_ERR_OK )
    {
        if( pJCommDev )
        {
            JCOMM_HANDLE   *pHTmp = &pJCommDev->hJComm;
            jComm_DestroyHandle(&pHTmp, 0);
        }
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;
}

JPG_ERR
jComm_DestroyHandle(
    JCOMM_HANDLE    **pHJComm,
    void            *extraData)
{
    JPG_ERR         result = JPG_ERR_OK;
    JPG_COMM_DEV    *pJCommDev = 0;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x, 0x%x\n", pHJComm, extraData);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_JCOMM, pHJComm, 0, result);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_JCOMM, (*pHJComm), 0, result);

    pJCommDev = DOWN_CAST(JPG_COMM_DEV, (*pHJComm), hJComm);
    if( pJCommDev )
    {
        free(pJCommDev);
        (*pHJComm) = 0;
    }

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;
}

JPG_ERR
jComm_Init(
    JCOMM_HANDLE        *pHJComm,
    JCOMM_INIT_PARAM    *pJCommInitParam,
    void                *extraData)
{
    JPG_ERR         result = JPG_ERR_OK;
    JPG_COMM_DEV    *pJCommDev = 0;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x, 0x%x, 0x%x\n", pHJComm, pJCommInitParam, extraData);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_JCOMM, pHJComm, 0, result);

    pJCommDev = DOWN_CAST(JPG_COMM_DEV, pHJComm, hJComm);
    if( pJCommDev && pJCommInitParam )
    {
        JPG_CODEC_DESC      *pJCodecDesc = 0;

        do{
            JPG_CODEC_HANDLE    *pHJCodec = &pJCommDev->hJCodec;

            memcpy(&pJCommDev->jCommInitParam, pJCommInitParam, sizeof(JCOMM_INIT_PARAM));

            pJCommDev->hJCodec.pHInJStream  = pJCommInitParam->pHInJStream;
            pJCommDev->hJCodec.pHOutJStream = pJCommInitParam->pHOutJStream;
            pJCommDev->hJComm.pJDispInfo    = pJCommInitParam->pJDispInfo;

            if( pJCommDev->hJCodec.bInitialed == false )
            {
                //------------------------------
                // get codec descriptor
                pJCodecDesc = FIND_DESC_ITEM(JPG_CODEC_DESC, pJCommInitParam->codecType);
                if( !pJCodecDesc )
                {
                    jpg_msg_ex(JPG_MSG_TYPE_ERR, " err, can't find descriptor(id=0x%x) !!", pJCommInitParam->codecType);
                    result = JPG_ERR_INVALID_PARAMETER;
                    break;
                }

                switch( pJCommInitParam->codecType )
                {
                    //case JPG_CODEC_DEC_JPROG:
                    case JPG_CODEC_DEC_JPG:
                        result = _jpg_allocate_line_buf(pJCommDev);
                        if( result != JPG_ERR_OK )
                        {
                            jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
                            break;
                        }

                        result = _jpg_allocate_disp_buf(pJCommDev);
                        if( result != JPG_ERR_OK )
                        {
                            jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
                            break;
                        }
                    case JPG_CODEC_DEC_JPG_CMD:
                    case JPG_CODEC_DEC_MJPG:
						if(pJCommInitParam->pJDispInfo->outColorSpace == JPG_COLOR_SPACE_RGB565  && 
							pJCommInitParam->codecType == JPG_CODEC_DEC_MJPG)
						{
							result = _jpg_allocate_line_buf(pJCommDev);
						}
                        pJCommDev->pre_setting = _jpg_dec_pre_setting;
                        break;

                    case JPG_CODEC_ENC_JPG:
                        result = _jpg_allocate_sys_bs_buf(pJCommDev);
                        if( result != JPG_ERR_OK )
                        {
                            jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
                            break;
                        }
                        pJCommDev->hJComm.pSysBsBufAddr = pHJCodec->pSysBsBuf;
                        pJCommDev->hJComm.sysBsBufSize  = pHJCodec->sysBsBufSize;
                    case JPG_CODEC_ENC_MJPG:
                        result = _jpg_allocate_jheader_buf(pJCommDev);
                        if( result != JPG_ERR_OK )
                        {
                            jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
                            break;
                        }
                        pJCommDev->hJComm.pJHdrData     = pHJCodec->jHwBsCtrl.pJHdrInfo;
                        pJCommDev->hJComm.jHdrDataSize  = pHJCodec->jHwBsCtrl.jHdrInfoSize;
                        pJCommDev->pre_setting = _jpg_enc_pre_setting;
                        break;
                }

                pJCommDev->jCodecDesc = *(pJCodecDesc);

                //----------------------------
                // init func
                if( pJCommDev->jCodecDesc.init )
                {
                	if(pJCommInitParam->codecType != JPG_CODEC_DEC_MJPG)
                    	result = pJCommDev->jCodecDesc.init(&pJCommDev->hJCodec, extraData);
					else
						result = pJCommDev->jCodecDesc.init(&pJCommDev->hJCodec, (void*)pHJComm);
                }

                pJCommDev->hJCodec.bInitialed = true;
            }
        }while(0);
    }

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;
}

JPG_ERR
jComm_deInit(
    JCOMM_HANDLE        *pHJComm,
    void                *extraData)
{
    JPG_ERR         result = JPG_ERR_OK;
    JPG_COMM_DEV    *pJCommDev = 0;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x, 0x%x\n", pHJComm, extraData);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_JCOMM, pHJComm, 0, result);

    pJCommDev = DOWN_CAST(JPG_COMM_DEV, pHJComm, hJComm);
    if( pJCommDev && pJCommDev->hJCodec.bInitialed == true )
    {
        JPG_CODEC_HANDLE    *pHJCodec = &pJCommDev->hJCodec;
        JPG_STREAM_DESC     *pJStreamDesc = &pJCommDev->hJCodec.pHInJStream->jStreamDesc;
        JPG_HW_BS_CTRL      *pJHwBsCtrl = &pJCommDev->hJCodec.jHwBsCtrl;
        JPG_LINE_BUF_INFO   *pJLineBufInfo = &pJCommDev->hJCodec.jLineBufInfo;

        if( pJCommDev->jCodecDesc.deInit )
            result = pJCommDev->jCodecDesc.deInit(&pJCommDev->hJCodec, extraData);

        if( pJStreamDesc->jFree_mem )
        {
            switch( pJCommDev->jCommInitParam.codecType )
            {
                //case JPG_CODEC_DEC_JPROG:
                case JPG_CODEC_DEC_JPG:
				case JPG_CODEC_DEC_MJPG:
                    // free line buffer
                    if( pJLineBufInfo->addrAlloc )
                    {
                        pJStreamDesc->jFree_mem(pHJCodec->pHInJStream, JPG_HEAP_LINE_BUF, pJLineBufInfo->addrAlloc);
                        pJLineBufInfo->addrAlloc = 0;
                    }

                    // free display buffer
                    _jpg_free_disp_buf(pJCommDev);
                    break;

                case JPG_CODEC_ENC_JPG:
                    // free jpg header buffer
                    if( pJHwBsCtrl->pJHdrInfo )
                    {
                        pJStreamDesc->jFree_mem(pJCommDev->hJCodec.pHInJStream, JPG_HEAP_DEF, pJHwBsCtrl->pJHdrInfo);
                        pJHwBsCtrl->pJHdrInfo = 0;
                    }

                    // free sys bs buffer
                    if( pHJCodec->pSysBsBuf )
                    {
                        pJStreamDesc->jFree_mem(pJCommDev->hJCodec.pHInJStream, JPG_HEAP_ENC_SYS_BS_BUF, pHJCodec->pSysBsBuf);
                        pHJCodec->pSysBsBuf = 0;
                    }
                    break;
            }
        }

        pJCommDev->hJCodec.bInitialed = false;

        // just time delay
        jpg_sleep(1); //10
    }

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;
}

JPG_ERR
jComm_Setup(
    JCOMM_HANDLE        *pHJComm,
    void                *extraData)
{
    JPG_ERR         result = JPG_ERR_OK;
    JPG_COMM_DEV    *pJCommDev = 0;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x, 0x%x\n", pHJComm, extraData);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_JCOMM, pHJComm, 0, result);

    pJCommDev = DOWN_CAST(JPG_COMM_DEV, pHJComm, hJComm);
    if( pJCommDev && pJCommDev->hJComm.pJFrmComp )
    {
        do{
            JPG_MULTI_SECT_INFO *pJMultiSectInfo = &pJCommDev->hJCodec.jMultiSectInfo;

            memcpy(&pJCommDev->hJCodec.jFrmCompInfo, pJCommDev->hJComm.pJFrmComp, sizeof(JPG_FRM_COMP));

            if( pJCommDev->pre_setting && pJCommDev->hJCodec.bSkipPreSetting == false )
            {
                result = pJCommDev->pre_setting(pJCommDev);
                if( result != JPG_ERR_OK )
                {
                    jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
                    break;
                }
            }

            if( pJCommDev->jCodecDesc.setup )
            {
                JPG_CODEC_HANDLE    *pHJCodec = &pJCommDev->hJCodec;
                JPG_HW_BS_CTRL      *pJHwBsCtrl = &pJCommDev->hJCodec.jHwBsCtrl;
                JPG_BUF_INFO        *pJInBufInfo = &pJCommDev->hJComm.jInBufInfo[0];
                uint8_t             *pTmpBsAddr = 0;

                pJCommDev->hJCodec.pHJComm = (void*)pHJComm;

                switch( pJCommDev->jCommInitParam.codecType )
                {
                    //case JPG_CODEC_DEC_JPROG:
                    case JPG_CODEC_DEC_JPG_CMD:
                    case JPG_CODEC_DEC_MJPG:
                    case JPG_CODEC_DEC_JPG:
                        pJCommDev->hJCodec.sysBsBufSize = pJInBufInfo[pHJComm->actIOBuf_idx].bufLength;
                        pJCommDev->hJCodec.pSysBsBuf    = pJInBufInfo[pHJComm->actIOBuf_idx].pBufAddr;
                        break;

                    case JPG_CODEC_ENC_JPG:
                    case JPG_CODEC_ENC_MJPG:
                        {
                            JPG_SHARE_DATA      *pJShare2Isp = &pJCommDev->hJCodec.jShare2Isp;
                            JPG_MULTI_SECT_INFO *pJMultiSectInfo = &pJCommDev->hJCodec.jMultiSectInfo;

                            pJMultiSectInfo->section_hight = pJCommDev->hJComm.encSectHight;

                            /**
                             * old code, pJShare2Isp may be used to isp/jpg on-fly encode.
                             * Now, we use to recode line buffer info.
                             **/
                            pJShare2Isp->width      = pJCommDev->jCommInitParam.encWidth;
                            pJShare2Isp->height     = pJCommDev->jCommInitParam.encHeight;
                            pJShare2Isp->addrY      = (uint32_t)pJInBufInfo[0].pBufAddr;
                            pJShare2Isp->addrU      = (uint32_t)pJInBufInfo[1].pBufAddr;
                            pJShare2Isp->addrV      = (uint32_t)pJInBufInfo[2].pBufAddr;
                            pJShare2Isp->pitchY     = pJInBufInfo[0].pitch;
                            pJShare2Isp->pitchUv    = pJInBufInfo[1].pitch;
                            pJShare2Isp->colorSpace = pJCommDev->jCommInitParam.encColorSpace;
                            pJShare2Isp->sliceCount = (pJMultiSectInfo->section_hight >> 3);
                        }
                        break;
                }

                result = pJCommDev->jCodecDesc.setup(&pJCommDev->hJCodec, extraData);
                if( result != JPG_ERR_OK )
                {
                    jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);
                    break;
                }
            }

            pJMultiSectInfo->bFirst    = true;
            pJMultiSectInfo->bFinished = false;
        }while(0);
    }

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;
}

JPG_ERR
jComm_Fire(
    JCOMM_HANDLE        *pHJComm,
    bool                bLastSection,
    void                *extraData)
{
    JPG_ERR         result = JPG_ERR_OK;
    JPG_COMM_DEV    *pJCommDev = 0;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x, %d, 0x%x\n", pHJComm, bLastSection, extraData);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_JCOMM, pHJComm, 0, result);

    pJCommDev = DOWN_CAST(JPG_COMM_DEV, pHJComm, hJComm);
    if( pJCommDev )
    {
    
		JCOMM_INIT_PARAM	*pJCommInit = &pJCommDev->jCommInitParam;
        JPG_CODEC_HANDLE    *pHJCodec = &pJCommDev->hJCodec;
        JPG_MULTI_SECT_INFO *pJMultiSectInfo = &pJCommDev->hJCodec.jMultiSectInfo;
        JPG_HW_BS_CTRL      *pJHwBsCtrl = &pJCommDev->hJCodec.jHwBsCtrl;
        JPG_BUF_INFO        *pJInBufInfo = &pJCommDev->hJComm.jInBufInfo[0];
        JPG_BUF_INFO        *pJOutBufInfo = &pJCommDev->hJComm.jOutBufInfo[0];
        JPG_LINE_BUF_INFO   *pJLineBufInfo = &pJCommDev->hJCodec.jLineBufInfo;

        do{
            bool        bSkip = false;

            switch( pJCommDev->jCommInitParam.codecType )
            {
                case JPG_CODEC_DEC_JPG:
                    if( pJMultiSectInfo->bFinished == true )
                    {
                        bSkip = true;
                        break;
                    }

                    if( pJMultiSectInfo->bFirst == true )
                    {
                        uint8_t     *pTmpBsAddr = pJInBufInfo[pHJComm->actIOBuf_idx].pBufAddr;
                        uint16_t    shiftByteNum = 0;

                        //  win32
                        _jpg_reflash_vram(pHJCodec->pHInJStream,
                                          JPG_STREAM_CMD_GET_VRAM_BS_BUF_A,
                                          0, 0, &pTmpBsAddr);

                        // 32-bit alignment....   //Actually , it should not do 32-bit align here ,  maybe has some issue ,Benson 2016/04/13.
                        shiftByteNum = (uint16_t)((uint32_t)pTmpBsAddr & 0x3);  
                        pJCommDev->hJCodec.sysBsBufSize = (((pJInBufInfo[pHJComm->actIOBuf_idx].bufLength + shiftByteNum) + 0x3) & ~0x3);
                        pJCommDev->hJCodec.pSysBsBuf    = pJInBufInfo[pHJComm->actIOBuf_idx].pBufAddr;
                    }
                    else
                    {
                        // ??? keep old code flow, but I think it should check 32-bit alignment address every setction.
                        // pipe line decode
                        pJCommDev->hJCodec.sysBsBufSize = pJInBufInfo[pHJComm->actIOBuf_idx].bufLength;
                        pJCommDev->hJCodec.pSysBsBuf    = pJInBufInfo[pHJComm->actIOBuf_idx].pBufAddr;
                    }
                    break;

                case JPG_CODEC_DEC_JPG_CMD:
                case JPG_CODEC_DEC_MJPG:
                    {
                        // set bit stream info to H/W, must be 32-bit alignment.
                        uint8_t     *pTmpBsAddr = 0;

                        // pHJComm->actIOBuf_idx = 0; // actIOBuf_idx must be "0"
                        pTmpBsAddr = pJInBufInfo[pHJComm->actIOBuf_idx].pBufAddr;
                        //  win32 case
                        _jpg_reflash_vram(pHJCodec->pHInJStream,
                                          JPG_STREAM_CMD_GET_VRAM_BS_BUF_A,
                                          0, 0, &pTmpBsAddr);

                        pJHwBsCtrl->shiftByteNum = ((uint32_t)pTmpBsAddr & 0x3);
                        pJCommDev->hJCodec.sysBsBufSize = (((pJInBufInfo[pHJComm->actIOBuf_idx].bufLength + pJHwBsCtrl->shiftByteNum) + 0x3) & ~0x3);
                        pJCommDev->hJCodec.pSysBsBuf    = pJInBufInfo[pHJComm->actIOBuf_idx].pBufAddr - pJHwBsCtrl->shiftByteNum;

						if(pJCommInit->pJDispInfo->outColorSpace != JPG_COLOR_SPACE_RGB565 )//  mjpeg handshake mode will pass it here  , Benson
						{
	                        // set line buffer info to H/W
	                        pJLineBufInfo->comp1Addr   = pJOutBufInfo[0].pBufAddr;
	                        pJLineBufInfo->comp1Pitch  = pJOutBufInfo[0].pitch;
	                        pJLineBufInfo->comp2Addr   = pJOutBufInfo[1].pBufAddr;
	                        pJLineBufInfo->comp23Pitch = pJOutBufInfo[1].pitch;
	                        pJLineBufInfo->comp3Addr   = pJOutBufInfo[2].pBufAddr;
						}
                    }
                    break;

                case JPG_CODEC_ENC_MJPG:
                    pHJCodec->pSysBsBuf    = pJCommDev->hJComm.pSysBsBufAddr;
                    pHJCodec->sysBsBufSize = pJCommDev->hJComm.sysBsBufSize;
                    break;

                case JPG_CODEC_ENC_JPG:
                    if( pJMultiSectInfo->bFirst == false )
                    {
                        // set line buffer info to H/W
                        pJLineBufInfo->comp1Addr   = pJInBufInfo[0].pBufAddr;
                        pJLineBufInfo->comp1Pitch  = pJInBufInfo[0].pitch;
                        pJLineBufInfo->comp2Addr   = pJInBufInfo[1].pBufAddr;
                        pJLineBufInfo->comp23Pitch = pJInBufInfo[1].pitch;
                        pJLineBufInfo->comp3Addr   = pJInBufInfo[2].pBufAddr;
                        pJLineBufInfo->sliceNum    = (pJMultiSectInfo->section_hight >> 3);
                    }
                    break;
            }

            if( bSkip == true )     break;

            pJMultiSectInfo->bFinished      = bLastSection;
            pJCommDev->hJCodec.bLastSection = bLastSection;

            if( pJCommDev->jCodecDesc.fire )
                result = pJCommDev->jCodecDesc.fire(&pJCommDev->hJCodec, (void*)pHJComm);  //Benson add for WriteBack mode. 2015/11/18 
                //result = pJCommDev->jCodecDesc.fire(&pJCommDev->hJCodec, extraData);

            // need to upddate sysBsBuf position and size for ap layer
            pJCommDev->hJComm.sysValidBsBufSize = pJCommDev->hJCodec.sysValidBsBufSize;
        }while(0);
    }

    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;
}

JPG_ERR
jComm_Control(
    JCOMM_HANDLE    *pHJComm,
    uint32_t        cmd,
    uint32_t        *value,
    void            *extraData)
{
    JPG_ERR         result = JPG_ERR_OK;
    JPG_COMM_DEV    *pJCommDev = 0;

    _jpg_trace_enter(JPG_MSG_TYPE_TRACE_JCOMM, "0x%x, 0x%x, 0x%x, 0x%x\n", pHJComm, cmd, value, extraData);
    _jpg_verify_handle(JPG_MSG_TYPE_TRACE_JCOMM, pHJComm, 0, result);

    pJCommDev = DOWN_CAST(JPG_COMM_DEV, pHJComm, hJComm);
    if( pJCommDev )
    {
        if( pJCommDev->jCodecDesc.control )
            result = pJCommDev->jCodecDesc.control(&pJCommDev->hJCodec, cmd, value, extraData);
    }

    if( result != JPG_ERR_OK &&
        result != JPG_ERR_NO_IMPLEMENT )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " %s() err 0x%x !! ", __FUNCTION__, result);

    _jpg_trace_leave(JPG_MSG_TYPE_TRACE_JCOMM);
    return result;
}


