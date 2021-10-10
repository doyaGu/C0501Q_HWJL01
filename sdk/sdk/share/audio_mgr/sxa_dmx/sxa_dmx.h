#ifndef __SXA_DMX_H_9CFFEFD1853549CC82B1E9DEEE909899__
#define __SXA_DMX_H_9CFFEFD1853549CC82B1E9DEEE909899__

//*****************************************************************************
// Name: sxa_dmx.h
//
// Description:
//     Header File for SXA Demuxer
//
// Copyright (c) 2008 ITE Technology Corp. All Rights Reserved.
//*****************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

#define SXA_CONFIG_DPF 0x0230

/*==================================================================================================*
* Includes                                                                                         *
*==================================================================================================*
*                                                                                                  *
* ######                 ####               ###                                                    *
*   ##                     ##                ##                                                    *
*   ##   ######    #####   ##   ### ###   ## ##   ####   #####                                     *
*   ##    ##  ##  ##   #   ##    ##  ##  ## ###  ##  ## ##  ##                                     *
*   ##    ##  ##  ##   #   ##    ##  ##  ##  ##  ###### ####                                       *
*   ##    ##  ##  ##       ##    ##  ##  ##  ##  ##       ####                                     *
*   ##    ##  ##  ##   #   ##    ##  ##  ## ###  ##   # ##  ##                                     *
* ###### ###  ###  #### ########  ######  ## ###  ####  #####                                      *
*                                                                                                  *
*==================================================================================================*/
#if 0
    #include    "pal/pal.h"

    #if         (SXA_CONFIG_DPF)
        #include    "mmp_types.h"
    #else   /* (SXA_CONFIG_DPF) */
        #include    "mmp/type.h"
    #endif  /* (SXA_CONFIG_DPF) */
#endif

#define MMP_BOOL         unsigned char
#define MMP_CHAR         char
#define MMP_WCHAR        short
#define MMP_UINT8        unsigned char
#define MMP_UINT16       unsigned short
#define MMP_UINT32       unsigned int
#define MMP_UINT         unsigned int
#define MMP_INT          int
#define MMP_INT16        short
#define MMP_INT32        int
// for build pass
#define MMP_UINT64       unsigned int
#define MMP_INT64        signed long long

#define MMP_ULONG        unsigned long
#define MMP_LONG         long
typedef void PAL_FILE;                  /**< File handle */

//#define PAL_FILE FILE
#define MMP_NULL         0

#define PAL_T(x)     L ## x /**< Unicode string type */
#define PalAssert(e) ((void) 0)

#define PAL_HEAP_DEFAULT 0
#define MMP_FALSE        0
#define MMP_TRUE         1

#define PAL_SEEK_SET     0  /**< Beginning of file */
#define PAL_SEEK_CUR     1  /**< Current position of file pointer */
#define PAL_SEEK_END     2  /**< End of file */

#define PAL_FILE_RB      0  /**< Opens for reading (binary) */
#define PAL_FILE_WB      1  /**< Opens an empty file for writing (binary) */
#define PAL_FILE_AB      2  /**< Opens for writing and appending (binary) */
#define PAL_FILE_RBP     3  /**< Opens for both reading and writing (binary) */
#define PAL_FILE_WBP     4  /**< Opens an empty file for both reading and writing (binary) */
#define PAL_FILE_ABP     5  /**< Opens for reading and appending (binary) */

#define MMP_ERROR_OFFSET 16 /**< Error offset */

#define PalHeapAlloc(a, size) malloc(size)
#define PalHeapFree(a, b)     free(b)
#define PalMemset        memset
#define PalMemcpy        memcpy

typedef enum MMP_MODULE_TAG   /**< Module type */
{
    MMP_MODULE_CORE,
    MMP_MODULE_LCD,
    MMP_MODULE_MMC,
    MMP_MODULE_M2D,
    MMP_MODULE_M3D,
    MMP_MODULE_DSC,
    MMP_MODULE_JPEG,
    MMP_MODULE_MPEG,
    MMP_MODULE_AUDIO,
    MMP_MODULE_3GPP,
    MMP_MODULE_USB,
    MMP_MODULE_VIDEO,
} MMP_MODULE;

/**
 * Result codes.
 */
typedef enum MMP_RESULT_TAG
{
    /** No errors occurred */
    MMP_RESULT_SUCCESS = 0,

    /** Unknown error */
    MMP_RESULT_ERROR   = 1,

    /** Error if invalid display handle */
    MMP_RESULT_ERROR_INVALID_DISP,

    /** Error if invalid object (pen, brush, or font)*/
    MMP_RESULT_ERROR_INVALID_OBJECT,

    /** Error if invalid font)*/
    MMP_RESULT_ERROR_INVALID_FONT,

    /** Error if invalid rop3 value */
    MMP_RESULT_ERROR_INVALID_ROP3,

    /** Error if destination is virtual display */
    MMP_RESULT_ERROR_VIRTUAL_DEST_DISP,

    /** Error if image format of destination display is incorrect */
    MMP_RESULT_ERROR_DEST_DISP_FORMAT,

    /**
     * Error if the image format of source display is unmatched with the image
     * format of destination display.
     */
    MMP_RESULT_ERROR_SRC_DISP_FORMAT,

    /** Error if source display without alpha channel */
    MMP_RESULT_ERROR_SRC_ALPHA,

    MMP_RESULT_ERROR_MAX = 0x7FFFFFFF
} MMP_RESULT;
typedef MMP_CHAR PAL_TCHAR;      /**< 8-bit ANSI character type */

/*==================================================================================================*
* Macros                                                                                           *
*==================================================================================================*
*                                                                                                  *
* ###   ###                                                                                        *
*  ### ###                                                                                         *
*  ### ###   ####    #####  ### ##   ####    #####                                                 *
*  ### ###      ##  ##   #   ### #  ##  ##  ##  ##                                                 *
*  ## # ##   #####  ##   #   ##     ##  ##  ####                                                   *
*  ##   ##  ##  ##  ##       ##     ##  ##    ####                                                 *
*  ##   ##  ##  ##  ##   #   ##     ##  ##  ##  ##                                                 *
* #### ####  ######  ####   #####    ####   #####                                                  *
*                                                                                                  *
*==================================================================================================*/
#define SUPPORT_NEW_PCM_INFO 1

/*==================================================================================================*
* Enumerations                                                                                     *
*==================================================================================================*
*                                                                                                  *
* #######                                                    ##      ##                            *
*  ##   #                                                    ##                                    *
*  ## # # ######   ### ###  ########    ####  ### ##  ####  ###### ####    ####  ######    #####   *
*  ####    ##  ##   ##  ##   ## ## ##  ##  ##  ### #     ##  ##      ##   ##  ##  ##  ##  ##  ##   *
*  ## #    ##  ##   ##  ##   ## ## ##  ######  ##     #####  ##      ##   ##  ##  ##  ##  ####     *
*  ##   #  ##  ##   ##  ##   ## ## ##  ##      ##    ##  ##  ##      ##   ##  ##  ##  ##    ####   *
*  ##   #  ##  ##   ##  ##   ## ## ##  ##   #  ##    ##  ##  ##  #   ##   ##  ##  ##  ##  ##  ##   *
* ####### ###  ###   ###### ### ## ###  ####  #####   ######  ###  ######  ####  ###  ### #####    *
*                                                                                                  *
*==================================================================================================*/
//
// Demuxer File Format Type
//
typedef enum tagSXA_DMXFFMT_E
{
    SXA_DMXFFMT_NULL = 0,                       // not supported
    SXA_DMXFFMT_3GPP = 1,
    SXA_DMXFFMT_AVI  = 2,                       /* audio video interleaving */
    SXA_DMXFFMT_PS   = 3                        /* mpeg program stream      */
} SXA_DMXFFMT_E;

//
// Video Format Type
//
typedef enum tagSXA_DMXVFMT_E
{
    SXA_DMXVFMT_NULL  = 0,                      // not supported
    SXA_DMXVFMT_H263  = 1,
    SXA_DMXVFMT_MPEG4 = 2,
    SXA_DMXVFMT_AVC   = 3,
    SXA_DMXVFMT_JPEG  = 4,
    SXA_DMXVFMT_MPEG1 = 5
} SXA_DMXVFMT_E;

//
// Audio Format Type
//
typedef enum tagSXA_DMXAFMT_E
{
    SXA_DMXAFMT_NULL      = 0,                  // not supported
    SXA_DMXAFMT_AMR       = 1,
    SXA_DMXAFMT_MP4       = 2,
    SXA_DMXAFMT_RAW       = 3,
    SXA_DMXAFMT_ULAW      = 4,
    SXA_DMXAFMT_SOWT      = 5,
    SXA_DMXAFMT_TWOS      = 6,
    SXA_DMXAFMT_ADPCM_DVI = 7,
    SXA_DMXAFMT_MPEG      = 8
} SXA_DMXAFMT_E;

//
// Property
//
typedef enum tagSXA_DMXPROP_E
{
    SXA_DMXPROP_MEDIAIFNO     = 1,
    SXA_DMXPROP_ASAMPLEINFO   = 2,
    SXA_DMXPROP_VSAMPLEINFO   = 3,
    SXA_DMXPROP_SEEKFROMINDEX = 4,
    SXA_DMXPROP_SEEKFROMTIME  = 5,
    SXA_DMXPROP_VOLINFO       = 6,
    SXA_DMXPROP_ADTSHEADER    = 7,
    SXA_DMXPROP_PCMSAMPLEINFO = 8,
    SXA_DMXPROP_QTABLE        = 9,
    SXA_DMXPROP_USERDATA      = 10,
} SXA_DMXPROP_E;

//
// Return Result
//
typedef enum tagSXA_DMXECODE_E
{
    SXA_DMXECODE_SOK                 = 0,
    SXA_DMXECODE_EFAIL               = -1,
    SXA_DMXECODE_EHANDLE             = -2,
    SXA_DMXECODE_EVOP_CODING_TYPE    = -3,
    SXA_DMXECODE_ENULL_FRAME         = -4,
    SXA_DMXECODE_ESAMPLE_OFFSET      = -5,      // sample info offset logical error
    SXA_DMXECODE_ESAMPLE_OFFSET_SIZE = -6,      // sample info offset or size logical error
} SXA_DMXECODE_E;

/*==================================================================================================*
* Typedefs                                                                                         *
*==================================================================================================*
*                                                                                                  *
* ######                                ###            ####                                        *
* # ## #                                ##            ##                                           *
* # ## #  #### ### ### ##    ####    ## ##    ####  #######  #####                                 *
*   ##     ##  ##   ### ##  ##  ##  ## ###   ##  ##   ##    ##  ##                                 *
*   ##     ## ##    ##  ##  ######  ##  ##   ######   ##    ####                                   *
*   ##      # ##    ##  ##  ##      ##  ##   ##       ##      ####                                 *
*   ##      ###     ### ##  ##   #  ## ###   ##   #   ##    ##  ##                                 *
*  ####      ##     ## ##    ####    ## ###   ####  ####### #####                                  *
*          ##       ##                                                                             *
*        #####     #####                                                                           *
*                                                                                                  *
*==================================================================================================*/

typedef void *SXA_HANDLE_T;

//
// Parameter for DmxOpenEx
//
typedef struct tagSXA_DMXOPENEXPARAM_T
{
    PAL_TCHAR    *ptPathName;           // filename with full path
    unsigned int nOffset;               // the offset that video starts in the media package file
    unsigned int nLength;               // the length of the video
    unsigned int nMode;                 // for encrypted file
    unsigned int nKey;                  // for encrypted file
    int          nInType;               // for intput data type, 0:network,1:file
} SXA_DMXOPENEXPARAM_T;

//
// Video/Audio Sample Information
//
typedef struct tagSXA_DMXSAMPLEINFO_T
{
    unsigned int nIndex;                // sample index, start from 1
    unsigned int nOffset;               // offset in the file
    unsigned int nSize;                 // size of this sample in byte
    unsigned int nTimeStamp;            // presentation time stamp in ms
    unsigned int nDelta;                // duration of this sample in timescale unit
} SXA_DMXSAMPLEINFO_T;

//
// PCM Audio Sample Information
//
typedef struct tagSXA_DMXPCMSAMPLEINFO_T
{
    unsigned int nIndex;                // sample index, start from 1
    unsigned int nOffset;               // offset in the file
    unsigned int nSize;                 // size of this sample in byte
    unsigned int nTimeStamp;            // presentation time stamp in ms
    unsigned int nDelta;                // duration of this sample in timescale unit
    unsigned int nNextIndex;
} SXA_DMXPCMSAMPLEINFO_T;

//
// Media Information
//
typedef struct tagSXA_DMXMEDIAINFO_T
{
    SXA_DMXFFMT_E nFFmt;
    unsigned int  nDuration;

    SXA_DMXVFMT_E nVFmt;
    unsigned int  nVTimeScale;
    unsigned int  nVSampleCount;
    unsigned int  nVWidth;
    unsigned int  nVHeight;
    unsigned int  nVKeySampleCount;
    unsigned int  *pnVKeySampleIndex;

    SXA_DMXAFMT_E nAFmt;
    unsigned int  nATimeScale;
    unsigned int  nASampleCount;
    unsigned int  nASampleRate;
    unsigned int  nASampleBits;
    unsigned int  nAChannelCount;

    unsigned int  nDataSize;                // size of additional data
    void          *pvData;                  // pointer to additional data
} SXA_DMXMEDIAINFO_T;

typedef struct tagSXA_DMXVOPCOMPLEXITY_T
{
    unsigned long dwEstimationMethod;
    unsigned long dwShape_complexity_estimation_disable;
    unsigned long dwEstimation_method;
    unsigned long dwOpaque;
    unsigned long dwTransparent;
    unsigned long dwIntra_cae;
    unsigned long dwInter_cae;
    unsigned long dwNo_update;
    unsigned long dwUpSampling;
    unsigned long dwTexture_complexity_estimation_set_1_disable;
    unsigned long dwIntra_blocks;
    unsigned long dwInter_blocks;
    unsigned long dwInter4v_blocks;
    unsigned long dwNo_coded_blocks;
    unsigned long dwTexture_complexity_estimation_set_2_disable;
    unsigned long dwDct_coefs;
    unsigned long dwDct_lines;
    unsigned long dwVlc_symbols;
    unsigned long dwVlc_bits;
    unsigned long dwMotion_compensation_complexity_disable;
    unsigned long dwApm;
    unsigned long dwNpm;
    unsigned long dwInterpolate_mc_q;
    unsigned long dwForw_back_mc_q;
    unsigned long dwHalfpel2;
    unsigned long dwHalfpel4;
    unsigned long dwVersion2_complexity_estimation_disable;
    unsigned long dwSadct;
    unsigned long dwQuarterpel;
} SXA_DMXVOPCOMPLEXITY_T;

//
// VOL data of mpeg4 video
//
typedef struct tagSXA_DMXVOLINFO_T
{
    unsigned long          dwVOLVerID;      /**< video object layer verid          */
    unsigned long          dwShape;         /**< video object layer shape          */
    unsigned long          dwTimeIncRes;    /**< vop time increment resolution     */
    unsigned long          dwTimeCodeBit;   /**< vop time increment resolution     */
    unsigned long          dwFixedTimeInc;  /**< fixed vop time increment          */
    unsigned long          dwFrameWidth;    /**< video object layer width          */
    unsigned long          dwFrameHeight;   /**< video object layer height         */
    unsigned long          dwQuantBits;
    unsigned long          dwQuantType;     /**< quant type                        */
    unsigned long          dwResyncMarker;  /**< resync marker disable             */
    unsigned long          dwDataPartition; /**< data partitioned                  */
    unsigned long          dwRevVLC;        /**< reversible vlc                    */
    unsigned long          dwComplexity_estimation_disable;
    SXA_DMXVOPCOMPLEXITY_T vopComplexity;
    unsigned char          QTable[2][64];
} SXA_DMXVOLINFO_T;

typedef struct tagSXA_DMXSEEKINFO_T
{
    unsigned int        nSeek;              // index to a video sample or time in ms
    SXA_DMXSAMPLEINFO_T tASampleInfo;       // to retrieve audio sample info
    SXA_DMXSAMPLEINFO_T tVSampleInfo;       // to retrieve video sample info
} SXA_DMXSEEKINFO_T;

typedef struct tagSXA_DMXADTSHEADER_T
{
    unsigned int  nAacFrameLength;
    unsigned char *pAdtsHeader;
} SXA_DMXADTSHEADER_T;

// ===== decode =====
//<-- Honda add start
typedef struct tagSXA_DMXVOPDCECS_T
{
    unsigned long dwOpaque;
    unsigned long dwTransparent;
    unsigned long dwIntra_cae;
    unsigned long dwInter_cae;
    unsigned long dwNo_update;
    unsigned long dwUpSampling;
    unsigned long dwIntra_blocks;
    unsigned long dwNo_coded_blocks;
    unsigned long dwDct_coefs;
    unsigned long dwDct_lines;
    unsigned long dwVlc_symbols;
    unsigned long dwVlc_bits;
    unsigned long dwInter_blocks;
    unsigned long dwInter4v_blocks;
    unsigned long dwApm;
    unsigned long dwNpm;
    unsigned long dwForw_back_mc_q;
    unsigned long dwHalfpel2;
    unsigned long dwHalfpel4;
    unsigned long dwInterpolate_mc_q;
    unsigned long dwSadct;
    unsigned long dwQuarterpel;
} SXA_DMXVOPDCECS_T;

//
// VOP data of mpeg4 video
//
typedef struct tagSXA_DMXVOPINFO_T
{
    unsigned long     dwCodingType;        /**< coding type, 0=I, 1=P      */
    unsigned long     dwTimeBase;          /**< module time base           */
    unsigned long     dwVOPTimeInc;        /**< vop time increment         */
    unsigned long     dwVopCoded;
    unsigned long     dwRounding;          /**< vop rounding type          */
    unsigned long     dwIntraDCThr;        /**< intra dc vlc thr           */
    unsigned long     dwVOPQuant;          /**< vop quant                  */
    unsigned long     dwFCode;             /**< vop fcode forward          */
    SXA_DMXVOPDCECS_T vopDcecs;
} SXA_DMXVOPINFO_T;

//
// Short header data of H.263 video
//
typedef struct tagSXA_DMXSHORTHEADERINFO_T
{
    unsigned long    dwVOPTimeInc;  /**< VOP Time increment     */
    unsigned long    dwSrcFmt;      /**< Source format          */
    unsigned long    dwFrameWidth;  /**< pixel of width         */
    unsigned long    dwFrameHeight; /**< pixel of height        */
    unsigned long    dwTimeCodeBit; /**< time code bit          */
    SXA_DMXVOPINFO_T VOPInfo;       /**< VOP information        */
} SXA_DMXSHORTHEADERINFO_T;

//
// bitstream size
//
typedef struct tagSXA_DMXDECODESIZE_T
{
    unsigned char *pDataStart;  /**< Start pointer of bitstream  */
    unsigned char *pDataEnd;    /**< End pointer of bitstream    */
    unsigned long dwStartBits;  /**< start bits of bitstream     */
    unsigned long dwDataSize;   /**< size of bitstream           */
} SXA_DMXDECODESIZE_T;

//
// QTable for MPEG
//
typedef struct tagSXA_DMXQTABLE_T
{
    unsigned char *pnQTableY;
    unsigned char *pnQTableUV;
} SXA_DMXQTABLE_T;

//
// First Video Frame Information
//
typedef struct tagSXA_DMX1STFRAMEINFO_T
{
    SXA_DMXFFMT_E nFFmt;
    SXA_DMXVFMT_E nVFmt;
    unsigned int  nVWidth;
    unsigned int  nVHeight;
    unsigned int  nOffset;                  // offset in the file
    unsigned int  nSize;                    // size of this sample in byte
    union
    {
        SXA_DMXVOLINFO_T volInfo;
        unsigned char    QTable[2][64];
    } u;
} SXA_DMX1STFRAMEINFO_T;

//
// User Data for 3GPP
//
typedef struct tagSXA_DMXUSERDATA_T
{
    unsigned char *pchCnam;
    unsigned char *pchCart;
    unsigned char *pchCalb;
} SXA_DMXUSERDATA_T;

//=============================================================================
//                              Constant Definition
//=============================================================================
/* INTRA */
static unsigned char mpeg_iqmat_def[64] =
{
    8, 17, 18, 19, 21, 23, 25, 27,
    17, 18, 19, 21, 23, 25, 27, 28,
    20, 21, 22, 23, 24, 26, 28, 30,
    21, 22, 23, 24, 26, 28, 30, 32,
    22, 23, 24, 26, 28, 30, 32, 35,
    23, 24, 26, 28, 30, 32, 35, 38,
    25, 26, 28, 30, 32, 35, 38, 41,
    27, 28, 30, 32, 35, 38, 41, 45
};

/* INTER */
static unsigned char mpeg_nqmat_def[64] =
{
    16, 17, 18, 19, 20, 21, 22, 23,
    17, 18, 19, 20, 21, 22, 23, 24,
    18, 19, 20, 21, 22, 23, 24, 25,
    19, 20, 21, 22, 23, 24, 26, 27,
    20, 21, 22, 23, 25, 26, 27, 28,
    21, 22, 23, 24, 26, 27, 28, 30,
    22, 23, 24, 26, 27, 28, 30, 31,
    23, 24, 25, 27, 28, 30, 31, 33
};

/* Inverse normal zigzag */
static unsigned char zigzag_i[64] =
{
    0, 1, 8, 16, 9, 2, 3, 10,
    17, 24, 32, 25, 18, 11, 4, 5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13, 6, 7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

//=============================================================================
//                              Function Declaration
//=============================================================================

// get all sample info
int dump_AudioSampleInfo(PAL_TCHAR *filename, SXA_HANDLE_T hDmx);

//get duration
int sxaDmxGetAudioDuration();

// get audio data
char *getAudioSample(SXA_HANDLE_T hDmx, int nIndex, unsigned int *pSize, unsigned int nSeekSize);

// get index
int getAudioSeekIndex(int nTime, int *pIndex, unsigned int *pOffset);

int setAudioSeekOffset(unsigned int Offset);

// input data type, 0:network ; 1:file
signed int
sxaDmxOpen(
    PAL_TCHAR    *ptPathName,
    SXA_HANDLE_T *phDmx,
    int          nInType);

signed int
sxaDmxOpenEx(
    SXA_DMXOPENEXPARAM_T *pParam,
    SXA_HANDLE_T         *phDmx);

signed int
sxaDmxClose(
    SXA_HANDLE_T hDmx);

signed int
sxaDmxGetProp(
    SXA_HANDLE_T  hDmx,
    SXA_DMXPROP_E eProp,
    void          *pvProp);

signed int
sxaDmxParseShortHeader(
    SXA_DMXSHORTHEADERINFO_T *pSHeaderInfo,
    SXA_DMXDECODESIZE_T      *pDecodeSize,
    unsigned char            *stream,
    unsigned int             size);

signed int
sxaDmxParseVop(
    SXA_DMXVOPINFO_T    *pVOPInfo,
    SXA_DMXDECODESIZE_T *pDecodeSize,
    SXA_DMXVOLINFO_T    *pVOLInfo,
    unsigned char       *stream,
    unsigned int        size);

signed int
sxaDmxGet1stVideoFrameInfo(
    SXA_DMXOPENEXPARAM_T  *pParam,
    SXA_DMX1STFRAMEINFO_T *pFrameInfo);

#ifdef __cplusplus
}
#endif

#endif  /* __SXA_DMX_H_9CFFEFD1853549CC82B1E9DEEE909899__ */