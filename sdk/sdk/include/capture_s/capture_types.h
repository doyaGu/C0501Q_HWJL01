#ifndef __CAP_TYPES_H__
#define __CAP_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "encoder/encoder_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define TRIPLE_BUFFER  //Benson

#if 1
/* 422 input data order for 1 channel mode */
#define CAP_IN_YUV422_YUYV 0
#define CAP_IN_YUV422_YVYU 1
#define CAP_IN_YUV422_UYVY 2
#define CAP_IN_YUV422_VYUY 3
#endif

//=============================================================================
//                Macro Definition
//=============================================================================
/**
 *  Debug message
 */
typedef enum _CAP_MSG_TYPE
{
    CAP_MSG_TYPE_ERR = (0x1 << 0),
} CAP_MSG_TYPE;

#ifdef _MSC_VER // WIN32
    #ifndef trac
        #define trac(string, ...)              do { printf(string, __VA_ARGS__); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
} while (0)
    #endif

    #define cap_msg(type, string, ...)         ((void)((type) ? printf(string, __VA_ARGS__) : MMP_NULL))
    #define cap_msg_ex(type, string, ...)      do { if (type) { \
                                                        printf(string, __VA_ARGS__); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__); } \
} while (0)

#else
    #ifndef trac
        #define trac(string, args ...)         do { printf(string, ## args); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
} while (0)
    #endif

    #define cap_msg(type, string, args ...)    ((void)((type) ? printf(string, ## args) : MMP_NULL))
    #define cap_msg_ex(type, string, args ...) do { if (type) { \
                                                        printf(string, ## args); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__); } \
} while (0)
#endif

//=============================================================================
//                Type Definition
//=============================================================================
/**
 *  CAP Update Flags
 */
typedef enum CAP_FLAGS_UPDATE_TAG
{
    CAP_FLAGS_UPDATE_CCMatrix = (0x00000001 << 0),
} CAP_FLAGS_UPDATE;

typedef enum CAP_INPUT_VIDEO_FORMAT_TAG
{
    Progressive,
    Interleaving
} CAP_INPUT_VIDEO_FORMAT;

#ifdef TRIPLE_BUFFER
typedef enum CAP_MEM_BUF_TAG
{
    CAP_MEM_Y0            = 0,
    CAP_MEM_U0            = 1,
    CAP_MEM_V0            = 2,
    CAP_MEM_Y1            = 3,
    CAP_MEM_U1            = 4,
    CAP_MEM_V1            = 5,
    CAP_MEM_Y2            = 6,
    CAP_MEM_U2            = 7,
    CAP_MEM_V2            = 8,
    CAPTURE_MEM_BUF_COUNT = 9
} CAP_MEM_BUF;
#else
typedef enum CAP_MEM_BUF_TAG
{
    CAP_MEM_Y0            = 0,
    CAP_MEM_U0            = 1,
    CAP_MEM_V0            = 2,
    CAP_MEM_Y1            = 3,
    CAP_MEM_U1            = 4,
    CAP_MEM_V1            = 5,
    CAPTURE_MEM_BUF_COUNT = 6
} CAP_MEM_BUF;
#endif

typedef enum CAP_ERROR_CODE_TAG
{
    HSYNC_ERR = 0x1,
    VSYNC_ERR,
    SIZE_ERROR_ERR,
    OVERFLOW_ERR,
    ABNORMAL_TERMINATE_ERR,
    TIMEOUT_ERR,
    SCDTO_ERR,
    BT656_SYNC_ERR,
    BT656_PREAMBLE_ERR,
    MEM_LAST_ERR,
    FRAME_SYNC_POINT_ERR,
    FRAME_SYNC_ACTIVE_ERR,
    FIRE_ISP_ERR,
    INTERNAL_DATAPTH_ERR
} CAP_ERROR_CODE;

//=============================================================================
//                              Structure Definition
//=============================================================================
/* Input Format Info */
typedef struct CAP_INPUT_INFO_TAG
{
    MMP_BOOL   TopFieldPol;                //0: HSYNC low, 1: Hsync hight
    MMP_BOOL   VSyncPol;                   //'0' 0 stands for blanking '1' 1 stands for blanking
    MMP_BOOL   HSyncPol;
    MMP_UINT16 PitchY;
    MMP_UINT16 PitchUV;

    /* Active Region  Info */
    MMP_UINT16 capwidth;
    MMP_UINT16 capheight;
    MMP_UINT16 HNum1;                   /* Input HSync active area start numner [12:0] */
    MMP_UINT16 HNum2;                   /* Input HSync active area end numner [12:0] */
    MMP_UINT16 LineNum1;                /* Input active area start line number[11:0] of top field */
    MMP_UINT16 LineNum2;                /* Input active area end line number[11:0] of top field */
    MMP_UINT16 LineNum3;                /* Input active area start line number[11:0] of bottom field */
    MMP_UINT16 LineNum4;                /* Input active area end line number[11:0] of bottom field */
} CAP_INPUT_INFO;

/* 9850 new function*/
typedef struct CAP_INPUT_VIDEO_SOURCE_INFO_TAG
{
    MMP_BOOL   LCDSRC;                //0: HS/VS/DE/DATA from IO ,1: HS/VS/DE/DATA from LCD
    MMP_BOOL   HSYNCDE;               //0: HSYNC=HSYNC ,DE=DE , 1: HSYNC=DE, DE=HSYNC
    MMP_BOOL   DEPOL;                 //0: not change ,1: invert
} CAP_INPUT_VIDEO_SOURCE_INFO;

typedef enum CAP_INPUT_PROTOCOL_INFO_TAG
{
    BT_601 = 0,
    BT_656 = 1
} CAP_INPUT_PROTOCOL_INFO;

typedef enum CAP_INPUT_DATA_FORMAT_TAG
{
	CAP_IN_YUYV = 0,
	CAP_IN_YVYU = 1,
	CAP_IN_UYVY	= 2,
	CAP_IN_VYUY = 3,
}CAP_INPUT_DATA_FORMAT;

typedef struct CAP_CONTEXT_TAG
{
    // Input format Info
    CAP_INPUT_INFO          ininfo;
    CAP_INPUT_VIDEO_FORMAT  Interleave;    // 0: Progressive , 1 :interleaving
    CAP_INPUT_PROTOCOL_INFO input_protocol;
    CAP_INPUT_VIDEO_SOURCE_INFO input_video_source;

    // Input data format Info
    MMP_UINT8               YUV422Format;

    // capture enable function
    MMP_BOOL                EnDEMode;
    MMP_UINT32              OutAddrY[3];
    MMP_UINT32              OutAddrU[3];
    MMP_UINT32              OutAddrV[3];
    MMP_BOOL                bMatchResolution;
} CAP_CONTEXT;

extern CAP_CONTEXT *Capctxt;

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif