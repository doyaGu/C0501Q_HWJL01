#ifndef __ISP_TYPES_H_6LX4PNNH_236F_H9EQ_5J77_Q9PJXPQ2OI5F__
#define __ISP_TYPES_H_6LX4PNNH_236F_H9EQ_5J77_Q9PJXPQ2OI5F__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include <stdio.h>
#include "isp_defs.h"
#include "mmp_isp.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define ISP_SCALE_TAP               4
#define ISP_SCALE_TAP_SIZE          8
#define ISP_SCALE_MAX_LINE_BUFF_LEN 1280

//=============================================================================
//				  Macro Definition
//=============================================================================
/**
 *  Debug message
 */

typedef enum _ISP_MSG_TYPE
{
    ISP_MSG_TYPE_ERR = (0x1 << 0),
} ISP_MSG_TYPE;

#ifdef _MSC_VER // WIN32
    #ifndef trac
        #define trac(string, ...)              do { printf(string, __VA_ARGS__); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
} while (0)
    #endif

    #define isp_msg(type, string, ...)         ((void)((type) ? printf(string, __VA_ARGS__) : MMP_NULL))
    #define isp_msg_ex(type, string, ...)      do { if (type) { \
                                                        printf(string,         __VA_ARGS__); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__); } \
} while (0)

#else
    #ifndef trac
        #define trac(string, args ...)         do { printf(string, ## args); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
} while (0)
    #endif

    #define isp_msg(type, string, args ...)    ((void)((type) ? printf(string, ## args) : MMP_NULL))
    #define isp_msg_ex(type, string, args ...) do { if (type) { \
                                                        printf(string,         ## args); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__); } \
} while (0)
#endif

//=============================================================================
//				  Structure Definition
//=============================================================================
/**
 *  ISP Update Flags
 */
typedef enum ISP_FLAGS_UPDATE_TAG
{
    ISP_FLAGS_UPDATE_EngineMode       = (0x00000001 << 0),
    ISP_FLAGS_UPDATE_InputParameter   = (0x00000001 << 1),
    ISP_FLAGS_UPDATE_InputBuf         = (0x00000001 << 2),
    ISP_FLAGS_UPDATE_InputAddr        = (0x00000001 << 3),
    ISP_FLAGS_UPDATE_RemapAddr        = (0x00000001 << 4),
    ISP_FLAGS_UPDATE_DeInterlaceParam = (0x00000001 << 5),
    ISP_FLAGS_UPDATE_YUVtoRGBMatrix   = (0x00000001 << 6),
    ISP_FLAGS_UPDATE_CCMatrix         = (0x00000001 << 7),
    ISP_FLAGS_UPDATE_ScaleParam       = (0x00000001 << 8),
    ISP_FLAGS_UPDATE_FrameFun0        = (0x00000001 << 9),
    ISP_FLAGS_UPDATE_OutParameter     = (0x00000001 << 10),
    ISP_FLAGS_UPDATE_OutBufInfo       = (0x00000001 << 11),
    ISP_FLAGS_UPDATE_OutAddress       = (0x00000001 << 12),
    ISP_FLAGS_UPDATE_RunLenEnc        = (0x00000001 << 13),
    ISP_FLAGS_UPDATE_Interrupt        = (0x00000001 << 14),
    ISP_FLAGS_UPDATE_PreScaleParam    = (0x00000001 << 15),
    ISP_FLAGS_UPDATE_PreScaleMatrix   = (0x00000001 << 16),
} ISP_FLAGS_UPDATE;

typedef enum ISP_FLAGS_WORK_TAG
{
    ISP_FLAGS_FIRST_VIDEO_FRAME       = (0x00000001 << 0),
    ISP_FLAGS_SECOND_VIDEO_FRAME      = (0x00000001 << 1),
    ISP_FLAGS_LCD_ONFLY_INIT          = (0x00000001 << 2),
    ISP_FLAGS_QUEUE_FIRE_INIT         = (0x00000001 << 3),
    ISP_FLAGS_ENABLE_ONFLY_UPDATEFLAG = (0x00000001 << 4)
} ISP_FLAGS_WORK;

/************************************************************************
 * \brief
 *   type define base on isp specification
 ***********************************************************************/
typedef enum ISP_DEINTER_MODE_TAG
{
    DEINTER3D = 0,
    DEINTER2D = 1
} ISP_DEINTER_MODE;

typedef enum ISP_FRAMEFUN_FORMAT_TAG
{
    CARGB565 = 0,
    ARGB4444 = 1,
    ARGB8888 = 2
} ISP_FRAMEFUN_FORMAT;

typedef enum ISP_YUV_PLANE_FORMAT_TAG
{
    YUV422  = 0,
    YUV420  = 1,
    YUV444  = 2,
    YUV422R = 3
} ISP_YUV_PLANE_FORMAT;

typedef enum ISP_OUT_RGB_FORMAT_TAG
{
    Dither565   = 0,
    Dither444   = 1,
    NoDither565 = 2,
    NoDither888 = 3
} ISP_OUT_RGB_FORMAT;

//=============================================================================
//                              Structure Definition
//=============================================================================
#define MAX_CT_NUM 10
typedef struct MMP_ISP_CT_TableType_TAG
{
    MMP_INT32 CT_Num;
    MMP_INT32 CT[MAX_CT_NUM];
    MMP_FLOAT R_gain[MAX_CT_NUM];
    MMP_FLOAT G_gain[MAX_CT_NUM];
    MMP_FLOAT B_gain[MAX_CT_NUM];
} MMP_ISP_CT_TableType;

typedef struct EV_MATRIX_TAG
{
    MMP_FLOAT M_color_correction[3][3];
    MMP_FLOAT V_color_correction[3][1];
} EV_MATRIX;

//Engine Mode
typedef struct ISP_ENGINE_MODE_CTRL_TAG
{
    MMP_BOOL   EnableJPEGDECODE;
    MMP_UINT16 TotalSliceNum;
} ISP_ENGINE_MODE_CTRL;

//Input Buffer
typedef struct ISP_INPUT_INFO_TAG
{
    MMP_BOOL             EnableInYUV255Range;
    MMP_BOOL             EnableCCFun;
    MMP_BOOL             EnableCSFun;
    MMP_BOOL             EnableDSFun;
    MMP_BOOL             UVRepeatMode;
    MMP_BOOL             EnableRemapYAddr;
    MMP_BOOL             EnableRemapUVAddr;
    ISP_YUV_PLANE_FORMAT PlaneFormat;
    MMP_UINT8            *AddrY;
    MMP_UINT8            *AddrU;
    MMP_UINT8            *AddrV;
    MMP_UINT8            *AddrYp;
    MMP_UINT16           PitchY;
    MMP_UINT16           PitchUV;
    MMP_UINT16           SrcWidth;
    MMP_UINT16           SrcHeight;
} ISP_INPUT_INFO;

//Deinterlace
typedef struct ISP_DEINTERLACE_CTRL_TAG
{
    MMP_BOOL         Enable;
    ISP_DEINTER_MODE DeinterMode;
    MMP_BOOL         EnAutoSwapField;
    MMP_BOOL         EnSrcBottomFieldFirst;
    MMP_BOOL         EnDeinterBottomField;
    MMP_BOOL         EnSrcLPF;
    MMP_BOOL         EnLummaEdgeDetect;
    MMP_BOOL         EnChromaEdgeDetect;
    MMP_BOOL         Disable30MotionDetect;
    MMP_BOOL         EnUV2DMethod;

    //For 3D Parameter
    MMP_UINT16       MDThreshold_High;
    MMP_UINT16       MDThreshold_Low;
    MMP_UINT16       MDThreshold_Step;
    MMP_BOOL         EnLPFBlend;
    MMP_BOOL         EnLPFWeight;
    MMP_BOOL         EnLPFStaticPixel;
    MMP_BOOL         DisableMV_A;
    MMP_BOOL         DisableMV_B;
    MMP_BOOL         DisableMV_C;
    MMP_BOOL         DisableMV_D;
    MMP_BOOL         DisableMV_E;
    MMP_BOOL         DisableMV_F;
    MMP_BOOL         DisableMV_G;

    //For 2D Parameter
    MMP_UINT16       D2EdgeBlendWeight;
    MMP_UINT16       D2OrgBlendWeight;

    //LowLevelEdge
    MMP_BOOL         EnLowLevelEdge;
    MMP_BOOL         EnLowLevelOutside;
    MMP_UINT16       LowLevelMode;
    MMP_UINT16       LowLevelBypassBlend;
    MMP_UINT16       LowLevelPosX;
    MMP_UINT16       LowLevelPosY;
    MMP_UINT16       LowLevelWidth;
    MMP_UINT16       LowLevelHeight;
} ISP_DEINTERLACE_CTRL;

//YUV to RGB transform matrix
typedef struct ISP_YUV_TO_RGB_TAG
{
    MMP_UINT16 _11;
    MMP_UINT16 _12;
    MMP_UINT16 _13;
    MMP_UINT16 _21;
    MMP_UINT16 _22;
    MMP_UINT16 _23;
    MMP_UINT16 _31;
    MMP_UINT16 _32;
    MMP_UINT16 _33;
    MMP_UINT16 ConstR;
    MMP_UINT16 ConstG;
    MMP_UINT16 ConstB;
    MMP_UINT16 Reserved;
} ISP_YUV_TO_RGB;

//Color correction matrix
typedef struct ISP_COLOR_CORRECTION_TAG
{
    MMP_FLOAT  OffsetR;
    MMP_FLOAT  OffsetG;
    MMP_FLOAT  OffsetB;
    MMP_FLOAT  _11;
    MMP_FLOAT  _12;
    MMP_FLOAT  _13;
    MMP_FLOAT  _21;
    MMP_FLOAT  _22;
    MMP_FLOAT  _23;
    MMP_FLOAT  _31;
    MMP_FLOAT  _32;
    MMP_FLOAT  _33;
    MMP_FLOAT  DeltaR;
    MMP_FLOAT  DeltaG;
    MMP_FLOAT  DeltaB;
    MMP_UINT16 Reserved;
} ISP_COLOR_CORRECTION;

//PreScale Function
typedef struct ISP_PRESCALE_CTRL_TAG
{
    MMP_FLOAT  HCI;
    MMP_UINT16 DstWidth;
    MMP_FLOAT  WeightMatX[ISP_SCALE_TAP_SIZE][ISP_SCALE_TAP];
} ISP_PRESCALE_CTRL;

//Scale Function
typedef struct ISP_SCALE_CTRL_TAG
{
    MMP_FLOAT  HCI;
    MMP_FLOAT  VCI;
    MMP_UINT16 DstWidth;
    MMP_UINT16 DstHeight;
    MMP_UINT16 DstPosX;
    MMP_UINT16 DstPosY;
    MMP_UINT16 BGColorR;
    MMP_UINT16 BGColorG;
    MMP_UINT16 BGColorB;
} ISP_SCALE_CTRL;

//Frame Function
typedef struct ISP_FRMFUN_CTRL_TAG
{
    MMP_BOOL            Enable;
    MMP_UINT8           *Addr;
    MMP_UINT16          Width;
    MMP_UINT16          Height;
    MMP_UINT16          Pitch;
    MMP_UINT16          StartX;
    MMP_UINT16          StartY;
    MMP_UINT16          ColorKeyR;
    MMP_UINT16          ColorKeyG;
    MMP_UINT16          ColorKeyB;
    MMP_UINT16          ConstantAlpha;
    ISP_FRAMEFUN_FORMAT Format;
} ISP_FRMFUN_CTRL;

//Output
typedef struct ISP_OUTPUT_INFO_TAG
{
    MMP_BOOL           DitherMode;
    ISP_OUT_RGB_FORMAT RGBFormat;
    MMP_UINT8          *Addr;
    MMP_UINT16         Width;
    MMP_UINT16         Height;
    MMP_UINT16         Pitch;
} ISP_OUTPUT_INFO;

//Run-Length Encoder
typedef struct ISP_RUNLEN_ENC_CTRL_TAG
{
    MMP_BOOL   Enable;
    MMP_BOOL   EnableRejectBit;
    MMP_UINT16 UnitSize;
    MMP_UINT16 RunSize;
    MMP_UINT8  *Addr;
    MMP_UINT16 LineByte;
    MMP_UINT16 Pitch;
    MMP_UINT32 MaxBit;
} ISP_RUNLEN_ENC_CTRL;

//Remap Addr
typedef struct ISP_REMAP_ADDR_TAG
{
    //MMP_UINT16              Addr_03;
    //MMP_UINT16              Addr_04;
    //MMP_UINT16              Addr_05;
    //MMP_UINT16              Addr_06;
    //MMP_UINT16              Addr_07;
    //MMP_UINT16              Addr_08;
    //MMP_UINT16              Addr_09;
    //MMP_UINT16              Addr_10;
    //MMP_UINT16              Addr_11;
    //MMP_UINT16              Addr_12;
    //MMP_UINT16              Addr_13;
    //MMP_UINT16              Addr_14;
    //MMP_UINT16              Addr_15;
    //MMP_UINT16              Addr_16;
    //MMP_UINT16              Addr_17;
    //MMP_UINT16              Addr_18;
    //MMP_UINT16              Addr_19;
    //MMP_UINT16              Addr_20;
    //MMP_UINT16              Addr_21;
    //MMP_UINT16              Addr_22;
    //MMP_UINT16              Addr_23;
    //MMP_UINT16              Addr_24;
    //MMP_UINT16              Addr_25;
    //MMP_UINT16              Addr_26;
    //MMP_UINT16              Addr_27;
    //MMP_UINT16              Addr_28;
    //MMP_UINT16              Addr_29;
    //MMP_UINT16              Addr_30;
    //MMP_UINT16              Addr_31;
    MMP_UINT16 Addr[32];
} ISP_REMAP_ADDR;

/**
 * ISP Data
 */
typedef struct ISP_CONTEXT_TAG
{
    //Update Flag
    ISP_FLAGS_UPDATE     UpdateFlags;

    //Work Flag
    ISP_FLAGS_WORK       WorkFlags;

    //ISP work mode
    MMP_ISP_MODE         ispMode;

    //ISP Engine mode
    ISP_ENGINE_MODE_CTRL EngineMode;

    //Input Information
    ISP_INPUT_INFO       InInfo;

    //Deinterlace
    ISP_DEINTERLACE_CTRL DeInterlace;

    //YUV2RGB Function
    ISP_YUV_TO_RGB       YUV2RGBFun;

    //Color Correction Function
    ISP_COLOR_CORRECTION CCFun;

    //PreScale Function
    ISP_PRESCALE_CTRL    PreScaleFun;

    //Scale Function
    ISP_SCALE_CTRL       ScaleFun;

    //Frame Function
    ISP_FRMFUN_CTRL      FrameFun0;

    //Output Information
    ISP_OUTPUT_INFO      OutInfo;

    //Run-Length Encoder
    ISP_RUNLEN_ENC_CTRL  RunLenEnc;

    //Remap Luma and Chroma Addr
    ISP_REMAP_ADDR       RemapYAddr;
    ISP_REMAP_ADDR       RemapUVAddr;
    MMP_UINT8            RemapTableYIdx;
    MMP_UINT8            RemapTableUVIdx;

    //Interrupt
    MMP_BOOL             EnableInterrupt;
    MMP_UINT16           InterruptMode;

    /**
     * For color correction matrix
     */
    MMP_INT16  hue;
    MMP_INT16  saturation;
    MMP_INT16  contrast;
    MMP_INT16  midPoint;
    MMP_UINT16 colorEffect;
    MMP_INT16  brightness;
    MMP_UINT32 dummy;
    MMP_INT16  sharp;
} ISP_CONTEXT;

#ifdef __cplusplus
}
#endif

#endif