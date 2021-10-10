#ifndef __jpg_types_H_9KaduSEu_QWFR_Z9qA_T5i3_XbPyW51GbXJe__
#define __jpg_types_H_9KaduSEu_QWFR_Z9qA_T5i3_XbPyW51GbXJe__

#ifdef __cplusplus
extern "C" {
#endif


#include "jpg_defs.h"
#include "ite_jpg.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define JPG_Q_TABLE_SIZE            64
#define JPG_MAX_COMPONENT_NUM       4


typedef enum JPG_FLAGS_TAG
{
    JPG_FLAGS_ENC_PARTIAL_OUT           = (0x00000001 << 0),
    JPG_FLAGS_MJPG_FIRST_FRAME          = (0x00000001 << 1),
    JPG_FLAGS_MJPG                      = (0x00000001 << 2),
    JPG_FLAGS_DEC_GET_INFO              = (0x00000001 << 3),
    JPG_FLAGS_OUTPUT_RGB565             = (0x00000001 << 5),
    JPG_FLAGS_EN_RGB565_Dither_KEY      = (0x00000001 << 6),
    JPG_FLAGS_DEC_DC_ONLY               = (0x00008000),
    JPG_FLAGS_DEC_UV_HOR_DOWNSAMPLE     = (0x01100000),
    JPG_FLAGS_DEC_UV_VER_DOWNSAMPLE     = (0x02200000),
    JPG_FLAGS_DEC_UV_HOR_DUPLICATE      = (0x04400000),
    JPG_FLAGS_DEC_UV_VER_DUPLICATE      = (0x08800000),
    JPG_FLAGS_DEC_Y_HOR_DOWNSAMPLE      = (0x00010000)
}JPG_FLAGS;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  Jpg decode multisection infomation.
 */
typedef struct JPG_MULTI_SECT_INFO_TAG
{
    bool        bFirst;
    bool        bFinished;
    uint32_t    section_hight;  // for encode
    uint32_t    remainSize;

}JPG_MULTI_SECT_INFO;

/**
 *  Q table
 */
typedef struct JPG_Q_TABLE_TAG
{
    uint8_t     table[4][JPG_Q_TABLE_SIZE];
    uint16_t    tableCnt;
    uint16_t    reserved;

}JPG_Q_TABLE;

/**
 *  Huffman table
 */
typedef struct _jpg_byte_align4 JPG_H_TABLE_TAG
{
    uint8_t     *pHuffmanTable;
    uint16_t    totalCodeLenCnt;
    uint16_t    reserved;

}JPG_H_TABLE;

/**
 * YUV to RGB transform matrix
 */
typedef struct JPG_YUV_TO_RGB_TAG
{
    uint16_t    _11;
    uint16_t    _13;
    uint16_t    _21;
    uint16_t    _22;
    uint16_t    _23;
    uint16_t    _31;
    uint16_t    _32;
    uint16_t    ConstR;
    uint16_t    ConstG;
    uint16_t    ConstB;
    uint16_t    Reserved;

}JPG_YUV_TO_RGB;

/**
 * Output RGB Dithrer key
 */
typedef struct JPG_DITHER_KEY_TAG
{
    bool        bEnDitherKey;
    uint16_t    ditherKey;
    uint16_t    ditherKeyMask;
    uint16_t    bgColor;

}JPG_DITHER_KEY;

/**
 *  JPG frame information (SOF)
 */
typedef struct JPG_FRM_INFO_TAG
{
    uint16_t    horizonSamp;
    uint16_t    verticalSamp;
    uint16_t    qTableSel;
    uint16_t    dcHuffTableSel;
    uint16_t    acHuffTableSel;
    uint16_t    compId;
    uint16_t    reserved;

}JPG_FRM_INFO;

/**
 *  JPG HW control setting
 */
typedef struct JPG_HW_CTRL_TAG
{
    uint16_t    codecCtrl;
    uint16_t    wLineBufSliceWr;
    uint32_t    BSBufRWDataSize;
    uint8_t     *qTableY;
    uint8_t     *qTableUv;
    uint16_t    codecFireCMD;
    uint16_t    lineBufCtrl;
    uint16_t    validLineBufSlice;
    uint16_t    BSBufCtrl;
    uint16_t    validBSBufSize;
    uint16_t    dcHuffW2talCodeLenCnt[2];      // 0xbe         ; 0->Y, 1->UV
    uint16_t    acHuffW2talCodeLenCnt[2];      // 0xbe         ; 0->Y, 1->UV
    uint8_t     *dcHuffTable[2];               // 0xc0         ; 0->Y, 1->UV
    uint8_t     *acHuffTable[2];               // 0xc2, 0xc4   ; 0->Y, 1->UV

}JPG_HW_CTRL;

/**
 *  Bit-stream buffer info. (compressed data)
 */
typedef struct _jpg_byte_align4 JPG_HW_BS_CTRL_TAG
{
    uint8_t       *preBsBuf;
    //uint8_t       *addrAlloc;
    uint8_t       *addr;
    uint32_t      rwSize;
    uint32_t      size;
    uint32_t      toBSSize;
    uint16_t      shiftByteNum;
    uint8_t       bsBufIdx;

    uint8_t       *pJHdrInfo; // for encode
    uint32_t      jHdrInfoSize;

}JPG_HW_BS_CTRL;

/**
 *  Line buffer info. (decompressed data)
 */
typedef struct JPG_LINE_BUF_INFO_TAG
{
    uint8_t       *addrAlloc;
    uint8_t       *comp1Addr;
    uint8_t       *comp2Addr;
    uint8_t       *comp3Addr;
    uint16_t      comp1Pitch;
    uint16_t      comp23Pitch;
    uint16_t      sliceNum;

    uint32_t      size;
    uint32_t      ySliceByteSize;
    uint32_t      uSliceByteSize;
    uint32_t      vSliceByteSize;

    uint16_t      reserved;

}JPG_LINE_BUF_INFO;


/**
 *  JPG freme component (YUV/CMYK)
 */
typedef struct JPG_FRM_COMP_TAG
{
    JPG_COLOR_SPACE      decColorSpace;
    JPG_COLOR_SPACE      encColorSpace;
    uint16_t             imgHeight;
    uint16_t             imgWidth;
    uint16_t             realHeight;
    uint16_t             realWidth;
    uint16_t             heightUnit;
    uint16_t             widthUnit;
    uint16_t             restartInterval;
    uint16_t             compNum;
    uint16_t             validComp;
    uint16_t             reserved;

    JPG_FRM_INFO         jFrmInfo[JPG_MAX_COMPONENT_NUM];  // base info

    JPG_Q_TABLE          qTable;
    bool                 bFindHDT;
    JPG_H_TABLE          huff_DC[2];  // 0->Y, 1->UV
    JPG_H_TABLE          huff_AC[2];  // 0->Y, 1->UV

    bool                 bNonInterleaved;
    bool                 bSingleChannel;
    bool                 bProgressive;
    bool                 bCMYK;

}JPG_FRM_COMP;

/**
 *  JPG frame size info
 */
typedef struct JPG_FRM_SIZE_INFO_TAG
{
    uint16_t          realWidth;
    uint16_t          realHeight;
    uint16_t          startX;
    uint16_t          startY;
    uint16_t          dispWidth;
    uint16_t          dispHeight;

    uint16_t          mcuRealWidth;
    uint16_t          mcuRealHeight;
    uint16_t          mcuDispWidth;
    uint16_t          mcuDispHeight;
    uint16_t          mcuDispLeft;
    uint16_t          mcuDispRight;
    uint16_t          mcuDispUp;
    uint16_t          mcuDispDown;

}JPG_FRM_SIZE_INFO;

/**
 *  JPG/ISP handshacking info
 */
typedef struct JPG_SHARE_DATA_TAG
{
    uint32_t          addrY;
    uint32_t          addrV;
    uint32_t          addrU;
    uint16_t          width;
    uint16_t          height;
    uint16_t          pitchY;
    uint16_t          pitchUv;
    uint16_t          sliceCount;
    bool              bCMYK;
    JPG_COLOR_SPACE   colorSpace;

}JPG_SHARE_DATA;
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
