#ifndef __jpg_parser_H_iYFN2b1P_uByZ_qox7_Y4v2_lnAK8lnM5VxS__
#define __jpg_parser_H_iYFN2b1P_uByZ_qox7_Y4v2_lnAK8lnM5VxS__

#ifdef __cplusplus
extern "C" {
#endif

#include "jpg_types.h"
#include "ite_jpg.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define JPG_MARKER_START                0xFF

#define JPG_APP00_MARKER                0xE0
#define JPG_APP01_MARKER                0xE1
#define JPG_APP02_MARKER                0xE2
#define JPG_APP03_MARKER                0xE3
#define JPG_APP04_MARKER                0xE4
#define JPG_APP05_MARKER                0xE5
#define JPG_APP06_MARKER                0xE6
#define JPG_APP07_MARKER                0xE7
#define JPG_APP08_MARKER                0xE8
#define JPG_APP09_MARKER                0xE9
#define JPG_APP10_MARKER                0xEA
#define JPG_APP11_MARKER                0xEB
#define JPG_APP12_MARKER                0xEC
#define JPG_APP13_MARKER                0xED
#define JPG_APP14_MARKER                0xEE
#define JPG_APP15_MARKER                0xEF


#define JPG_BASELINE_MARKER             0xC0
#define JPG_EXTENDED_SEQUENTIAL         0xC1
#define JPG_PROGRESSIVE                 0xC2
#define JPG_LOSSLESS_SEQUENTIAL         0xC3
#define JPG_HUFFMAN_TABLE_MARKER        0xC4

#define JPG_DIFF_SEQUENTIAL             0xC5
#define JPG_DIFF_PROGRESSIVE            0xC6
#define JPG_DIFF_LOSSLESS               0xC7

#define JPG_START_OF_IMAGE_MARKER       0xD8
#define JPG_END_OF_IMAGE_MARKER         0xD9
#define JPG_START_OF_SCAN_MARKER        0xDA
#define JPG_Q_TABLE_MARKER              0xDB
#define JPG_DRI_MARKER                  0xDD
#define JPG_NO_MEANING_MARKER			0xFF



#define JPG_APP2_MAX_SEGMENT            20
#define JPG_Q_TABLE_ELEMENT_NUM         64


/**
 * Jpg attrib definition
 */
typedef enum JPG_ATTRIB_STATUS_TAG
{
    JATT_STATUS_GENERATED                    = 0x00000001,
    JATT_STATUS_HAS_SMALL_THUMB              = 0x00000002,
    JATT_STATUS_HAS_LARGE_THUMB              = 0x00000004,
    JATT_STATUS_UNSUPPORT_PRIMARY            = 0x00000008,
    JATT_STATUS_UNSUPPORT_SMALL_THUMB        = 0x00000010,
    JATT_STATUS_UNSUPPORT_LARGE_THUMB        = 0x00000020,
    JATT_STATUS_EVER_DECODE_PRIMARY          = 0x00000040,
    JATT_STATUS_EVER_DECODE_SMALL_THUMB      = 0x00000080,
    JATT_STATUS_EVER_DECODE_LARGE_THUMB      = 0x00000100,
    JATT_STATUS_LARGE_THUMB_MULTI_SEGMENTS   = 0x00000200,
    JATT_STATUS_JFIF                         = 0x00000400

} JPG_ATTRIB_STATUS;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 * Exif information
 */
typedef struct JPG_EXIF_INFO_TAG
{
    uint32_t      primaryWidth;
    uint32_t      primaryHeight;

    char          dateTime[20];
    uint32_t      primaryOrientation;
    uint32_t      thumbOrientation;
    uint32_t      imageWidth;
    uint32_t      imageHeight;
    char          cameraMake[32];
    char          cameraModel[32];

    uint16_t      year;
    uint16_t      month;
    uint16_t      day;
    uint16_t      hour;
    uint16_t      minute;
    uint16_t      second;

} JPG_EXIF_INFO;


typedef struct JPG_BASE_LITE_INFO_TAG
{
    uint32_t      width;
    uint32_t      height;
    uint32_t      offset;
    uint32_t      size;
    bool          bJprog;         // find jpeg progessive case

}JPG_BASE_LITE_INFO;

typedef struct JPG_PRS_BS_CTRL_TAG
{
    bool        b1stSection;

    uint8_t     *bsPrsBuf;
    uint32_t    bsPrsBufMaxLeng;
    uint32_t    curPos;       // current position in bitstream buffer A
    uint32_t    realSize;     // real filled data size

}JPG_PRS_BS_CTRL;

typedef struct JPG_ATTRIB_TAG
{
    JPG_ATTRIB_STATUS   flag;

    JPG_BASE_LITE_INFO  jBaseLiteInfo[JPG_DEC_TYPE_COUNT];
    JPG_DEC_TYPE        decThumbType;   // decode thumbnail. If no thumbnail, decode primary.

    uint32_t            currBufLeng;
    uint32_t            primaryOffset;
    uint32_t            primaryLength;  // primary section length

    JPG_EXIF_INFO       exifInfo;

    bool                bCMYK;    // find Adobe CMYK color format case

    // for App 1
    bool                bApp1Find;

    // for App 2
    uint32_t            app2StreamOffset[JPG_APP2_MAX_SEGMENT];
    uint32_t            app2StreamSize[JPG_APP2_MAX_SEGMENT];
    uint32_t            app2StreamCnt;
    uint32_t            exifAppOffset[JPG_APP2_MAX_SEGMENT+1];
    uint32_t            exifAppLength[JPG_APP2_MAX_SEGMENT+1];
    uint32_t            exifAppCnt;

}JPG_ATTRIB;

typedef struct JPG_PARSER_INFO_TAG
{
    // base
    uint32_t            fileLength;
    uint32_t            remainSize;    // file remain size (un-process)
    bool                bMultiSection; // multi-section case or not
    bool                bLEndian;      // system is little endian or not
    bool                bExifLEndian;  // exif data is little endian or not

    bool                bSkipHDT;      // for mjpeg case

    // app section
    JPG_ATTRIB          jAttrib;

    // base section
    JPG_FRM_COMP        jFrmComp;

    // bit stream control
    JPG_PRS_BS_CTRL     jPrsBsCtrl;

}JPG_PARSER_INFO;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
void
jPrs_Seek2SectionStart(
    JPG_STREAM_HANDLE   *pHJStream,
    JPG_PARSER_INFO     *pJPrsInfo,
    uint32_t            destPos,
    uint32_t            sectLength);


// parsing base jpg info, ex. Q-table, H-thable, ...etc.
JPG_ERR
jPrs_BaseParser(
    JPG_STREAM_HANDLE   *pHJStream,
    JPG_PARSER_INFO     *pJPrsInfo);


// parsing app section info, ex. EXIF, 0xE0 ~ 0xEF, 0xF0 ~ 0xFF
JPG_ERR
jPrs_AppParser(
    JPG_STREAM_HANDLE   *pHJStream,
    JPG_PARSER_INFO     *pJPrsInfo);


#ifdef __cplusplus
}
#endif

#endif
