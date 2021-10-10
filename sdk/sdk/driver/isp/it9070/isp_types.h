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
#define ISP_SCALE_MAX_BLOCK_WIDTH   512
#define ISP_SCALE_MAX_BLOCK_HEIGHT  60
#define ISP_SCALE_MAX_LINE_BUFF_LEN 1920

#define FULLHD_WIDTH                1920
#define FULLHD_HEIGHT               1080
#define HD_WIDTH                    1280
#define HD_HEIGHT                   720
#define PAL_WIDTH                   720
#define PAL_HEIGHT                  576

#define VIDEO_BLANK_WIDTH           16
#define VIDEO_BLANK_HEIGHT          16
#define BLANK_BUFFER_INDEX          (4)

//=============================================================================
//                Macro Definition
//=============================================================================
/**
 *  Debug message
 */
extern MMP_UINT32 ispMsgOnFlag;

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

    #define isp_msg(type, string, ...)         ((void)((type & ispMsgOnFlag) ? printf(string, __VA_ARGS__) : MMP_NULL))
    #define isp_msg_ex(type, string, ...)      do { if (type & ispMsgOnFlag) { \
                                                        printf(string,         __VA_ARGS__); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__); } \
                                               } while (0)

#else
    #ifndef trac
        #define trac(string, args ...)         do { printf(string, ## args); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                               } while (0)
    #endif

    #define isp_msg(type, string, args ...)    ((void)((type & ispMsgOnFlag) ? printf(string, ## args) : MMP_NULL))
    #define isp_msg_ex(type, string, args ...) do { if (type & ispMsgOnFlag) { \
                                                        printf(string,         ## args); \
                                                        printf("  %s [#%d]\n", __FILE__, __LINE__); } \
                                               } while (0)
#endif

#define isp_enable_msg(type)                   (ispMsgOnFlag |= type)
#define isp_disable_msg(type)                  (ispMsgOnFlag &= ~(type))

//=============================================================================
//                Structure Definition
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
    ISP_FLAGS_UPDATE_DeInterlaceParam = (0x00000001 << 4),
    ISP_FLAGS_UPDATE_SubTitle0        = (0x00000001 << 5),
    ISP_FLAGS_UPDATE_SubTitle1        = (0x00000001 << 6),
    ISP_FLAGS_UPDATE_YUVtoRGBMatrix   = (0x00000001 << 7),
    ISP_FLAGS_UPDATE_CCMatrix         = (0x00000001 << 8),
    ISP_FLAGS_UPDATE_PreScaleParam    = (0x00000001 << 9),
    ISP_FLAGS_UPDATE_PreScaleMatrix   = (0x00000001 << 10),
    ISP_FLAGS_UPDATE_ScaleParam       = (0x00000001 << 11),
    ISP_FLAGS_UPDATE_ScaleMatrixH     = (0x00000001 << 12),
    ISP_FLAGS_UPDATE_ScaleMatrixV     = (0x00000001 << 13),
    ISP_FLAGS_UPDATE_FrameFun0        = (0x00000001 << 14),
    ISP_FLAGS_UPDATE_FrameFun1        = (0x00000001 << 15),
    ISP_FLAGS_UPDATE_RGBtoYUVMatrix   = (0x00000001 << 16),
    ISP_FLAGS_UPDATE_Clip0Fun         = (0x00000001 << 17),
    ISP_FLAGS_UPDATE_Clip1Fun         = (0x00000001 << 18),
    ISP_FLAGS_UPDATE_Clip2Fun         = (0x00000001 << 19),
    ISP_FLAGS_UPDATE_OutParameter     = (0x00000001 << 20),
    ISP_FLAGS_UPDATE_OutBufInfo       = (0x00000001 << 21),
    ISP_FLAGS_UPDATE_OutAddress       = (0x00000001 << 22),
    ISP_FLAGS_UPDATE_Mpeg2BufferIdx   = (0x00000001 << 23),
    ISP_FLAGS_UPDATE_FrmMatrix        = (0x00000001 << 24),
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
typedef enum ISP_PLL_SEL_TAG
{
    ISP_PLL_SEL_1 = 0,
    ISP_PLL_SEL_2 = 1,
    ISP_PLL_SEL_3 = 2
} ISP_PLL_SEL;

typedef enum ISP_DEINTER_MODE_TAG
{
    DEINTER3D = 0,
    DEINTER2D = 1
} ISP_DEINTER_MODE;

typedef enum ISP_SUBTITLE_FORMAT_TAG
{
    YUVT6442 = 0,
    YUVT4444 = 1,
    YUVT6334 = 2,
    YUVT5443 = 3
} ISP_SUBTITLE_FORAMT;

typedef enum ISP_FRAMEFUN_FORMAT_TAG
{
    CARGB565 = 0,
    ARGB4444 = 1,
    ARGB8888 = 2
} ISP_FRAMEFUN_FORMAT;

typedef enum ISP_OUT_FORMAT_TAG
{
    RGBPacket = 0,
    YUVPacket = 1,
    YUVPlane  = 2
} ISP_OUT_FORMAT;

typedef enum ISP_YUV_PACKET_FORMAT_TAG
{
    YUYV = 0,
    YVYU = 1,
    UYVY = 2,
    VYUY = 3
} ISP_YUV_PACKET_FORMAT;

typedef enum ISP_YUV_PLANE_FORMAT_TAG
{
    YUV422  = 0,
    YUV420  = 1,
    YUV444  = 2,
    YUV422R = 3
} ISP_YUV_PLANE_FORMAT;

typedef enum ISP_RGBPACKET_FORMAT_TAG
{
    RGB565 = 0,
    RGB444 = 1,
    RGB888 = 2
} ISP_RGB_PACKET_FORMAT;

typedef enum ISP_NV_FORMAT_TAG
{
    NV12 = 0,
    NV21 = 1
} ISP_NV_FORMAT;

typedef enum ISP_OUT_RGB_FORMAT_TAG
{
    Dither565   = 0,
    Dither444   = 1,
    NoDither565 = 2,
    NoDither888 = 3
} ISP_OUT_RGB_FORMAT;

typedef enum ISP_ROTATE_FORMAT_TAG
{
    Deg0   = 0,
    Deg90  = 1,
    Deg270 = 2,
    Deg180 = 3,
    Mirror = 4,
    Flip   = 5
} ISP_ROTATE_FORMAT;

typedef enum ISP_CLIPE_FORMAT_TAG
{
    ClipInside  = 0,
    ClipOutside = 1
} ISP_CLIPE_FORMAT;

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
    MMP_BOOL   EnableBlockMode;
    MMP_BOOL   EnableJPEGDECODE;
    MMP_BOOL   EnableRawDataMode;
    MMP_UINT16 TotalSliceNum;
    MMP_UINT16 WriteRawSliceNum;
} ISP_ENGINE_MODE_CTRL;

//Input Buffer Struct
typedef struct ISP_INPUT_INFO_TAG
{
    MMP_BOOL EnableInYUV255Range;
    /// Planar mode means : You have 3 separate images, one for each component,
    /// each image 8 bits / pixel.To get the real colored pixel, you have to
    /// mix the components from all planes.The resolution of planes may differ!
    MMP_BOOL              EnableYUVPlaneMode;
    /// Packed mode means : you have all components mixed / interleaved together,
    /// so you have small "packs" of components in a single, big image.
    MMP_BOOL              EnableYUVPackMode;
    MMP_BOOL              EnableNVMode;
    MMP_BOOL              EnableRGB888;
    MMP_BOOL              EnableRGB565;
    ISP_YUV_PACKET_FORMAT PacketFormat;
    ISP_YUV_PLANE_FORMAT  PlaneFormat;
    ISP_NV_FORMAT         NVFormat;
    MMP_UINT8             *AddrY;
    MMP_UINT8             *AddrU;
    MMP_UINT8             *AddrV;
    MMP_UINT8             *AddrYp;
    MMP_UINT8             *AddrUp;
    MMP_UINT8             *AddrVp;
    MMP_UINT16            PitchY;
    MMP_UINT16            PitchUV;
    MMP_UINT16            SrcWidth;
    MMP_UINT16            SrcHeight;
    MMP_UINT16            SrcPosX;
    MMP_UINT16            SrcPosY;
    MMP_UINT16            PanelWidth;
    MMP_UINT16            PanelHeight;
    MMP_UINT16            PanelColorY;
    MMP_UINT16            PanelColorU;
    MMP_UINT16            PanelColorV;
    MMP_UINT16            SrcExtedTop;
    MMP_UINT16            SrcExtedDown;
    MMP_UINT16            SrcExtedLeft;
    MMP_UINT16            SrcExtedRight;
    MMP_BOOL              EnableRdRqDoubleLine;
    MMP_UINT16            DstExtedTop;
    MMP_UINT16            DstExtedDown;
    MMP_UINT16            DstExtedLeft;
    MMP_UINT16            DstExtedRight;
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
    MMP_BOOL         UVRepeatMode;

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

//Subtitle Function
typedef struct ISP_SUBTITLE_CTRL_TAG
{
    MMP_BOOL            Enable;
    MMP_UINT8           *Addr;
    MMP_UINT16          SrcWidth;
    MMP_UINT16          SrcHeight;
    MMP_FLOAT           HCI;
    MMP_FLOAT           VCI;
    MMP_UINT16          DstWidth;
    MMP_UINT16          DstHeight;
    MMP_UINT16          Pitch;
    MMP_UINT16          StartX;
    MMP_UINT16          StartY;
    ISP_SUBTITLE_FORAMT Format;

    // ui decopress
    MMP_BOOL            EnableUiDec;
    MMP_UINT32          UiDecLineBytes;
    MMP_UINT32          UiDecTotalBytes;
} ISP_SUBTITLE_CTRL;

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
    MMP_FLOAT  HCIP;
    MMP_UINT16 VCIP;
    MMP_FLOAT  HCI;
    MMP_FLOAT  VCI;
    MMP_UINT16 DstWidth;    // set by mmpIspSetVideoWindow()
    MMP_UINT16 DstHeight;   // set by mmpIspSetVideoWindow()
    MMP_UINT16 DstPosX;     // set by mmpIspSetVideoWindow()
    MMP_UINT16 DstPosY;     // set by mmpIspSetVideoWindow()
    MMP_UINT16 BGColorR;
    MMP_UINT16 BGColorG;
    MMP_UINT16 BGColorB;
    MMP_FLOAT  WeightMatX[ISP_SCALE_TAP_SIZE][ISP_SCALE_TAP];
    MMP_FLOAT  WeightMatY[ISP_SCALE_TAP_SIZE][ISP_SCALE_TAP];
} ISP_SCALE_CTRL;

//Frame Function
typedef struct ISP_FRMFUN_CTRL_TAG
{
    MMP_BOOL            Enable;
    MMP_BOOL            EnableRGB2YUV;
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

    // ui decopress
    MMP_BOOL            EnableUiDec;
    MMP_UINT32          UiDecLineBytes;
    MMP_UINT32          UiDecTotalBytes;

    // constant color
    MMP_BOOL            EnableBlendConst;
    MMP_BOOL            EnableGridConst;
    MMP_UINT16          GridDataMode;
    MMP_UINT16          ConstColorR0;
    MMP_UINT16          ConstColorG0;
    MMP_UINT16          ConstColorB0;
    MMP_UINT16          ConstColorR1;
    MMP_UINT16          ConstColorG1;
    MMP_UINT16          ConstColorB1;
    MMP_UINT16          UIBufferIndex;
} ISP_FRMFUN_CTRL;

//Transfer Matrix RGB to YUV
typedef struct ISP_RGB_TO_YUV_TAG
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
    MMP_UINT16 ConstY;
    MMP_UINT16 ConstU;
    MMP_UINT16 ConstV;
    MMP_UINT16 Reserved;
} ISP_RGB_TO_YUV;

//Clipping Function
typedef struct ISP_CLIP_FN_CTRL_TAG
{
    MMP_BOOL         Enable;
    ISP_CLIPE_FORMAT Format;
    MMP_UINT16       ClipLeft;
    MMP_UINT16       ClipRight;
    MMP_UINT16       ClipTop;
    MMP_UINT16       ClipBottom;
    MMP_UINT16       StartX;
    MMP_UINT16       StartY;
    MMP_UINT16       Width;
    MMP_UINT16       Height;

    MMP_UINT16       Reserved;
} ISP_CLIP_FN_CTRL;

//Port 1 output
typedef struct ISP_OUTPUT_INFO_TAG
{
    MMP_BOOL              EnableOutYUV235Range;
    MMP_UINT16            SWWrFlipNum;
    MMP_BOOL              EnableSWFlipMode;
    ISP_ROTATE_FORMAT     RotateType;
    MMP_BOOL              EnableTripleBuf;
    MMP_BOOL              EnableDoubleBuf;
    MMP_BOOL              DisableToLCDFlip;
    MMP_BOOL              EnableProgFieldMode;
    MMP_BOOL              EnableNegative;
    MMP_BOOL              EnableFieldScale;
    MMP_BOOL              BottomFieldScaleFirst;
    MMP_BOOL              EnableCCFun;  // enable color correction function
    MMP_BOOL              EnableCSFun;
    MMP_BOOL              DisbleVideoOut;
    MMP_BOOL              EnableQueueFire;
    MMP_BOOL              EnableLcdOnFly;
    MMP_BOOL              EnableKeepLastField;
    MMP_BOOL              EnableVideoPreview;
    MMP_BOOL              EnableOnFlyWriteMem;
    MMP_BOOL              EnableDoubleFrameRate;
    MMP_BOOL              DitherMode;
    ISP_OUT_FORMAT        OutFormat;
    ISP_OUT_RGB_FORMAT    RGBFormat;
    ISP_YUV_PACKET_FORMAT PacketFormat;
    ISP_YUV_PLANE_FORMAT  PlaneFormat;
    MMP_UINT8             *Addr0;
    MMP_UINT8             *Addr1;
    MMP_UINT8             *Addr2;
    MMP_UINT16            Width;    // set by mmpIspSetDisplayWindow()
    MMP_UINT16            Height;   // set by mmpIspSetDisplayWindow()
    MMP_UINT16            PitchYRGB;
    MMP_UINT16            PitchUV;
    MMP_UINT16            Reserved;
    //ISP_PIXEL_FORMAT        Format;
} ISP_OUTPUT_INFO;

/**
 * ISP Data
 */
typedef struct ISP_CONTEXT_TAG
{
    //ISP YUV Process
    MMP_BOOL             EnableYUVProcess;

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

    //Subtitle Function
    ISP_SUBTITLE_CTRL    SubTitle0;
    ISP_SUBTITLE_CTRL    SubTitle1;

    //YUV2RGB Function
    const ISP_YUV_TO_RGB *YUV2RGBFun;

    //Color Correction Function
    ISP_COLOR_CORRECTION CCFun;

    //PreScale Function
    ISP_PRESCALE_CTRL    PreScaleFun;

    //Scale Function
    ISP_SCALE_CTRL       ScaleFun;

    //Frame Function
    ISP_FRMFUN_CTRL      FrameFun0;
    ISP_FRMFUN_CTRL      FrameFun1;
    ISP_RGB_TO_YUV       FrmMatrix;

    //RGB2YUV Function
    const ISP_RGB_TO_YUV *RGB2YUVFun;

    //Clip Function
    ISP_CLIP_FN_CTRL     ClipFun0;
    ISP_CLIP_FN_CTRL     ClipFun1;
    ISP_CLIP_FN_CTRL     ClipFun2;

    //Output Information
    ISP_OUTPUT_INFO      OutInfo;

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
    //ISP_ISP_SHARE       ispShare;
    MMP_INT16  sharp;

    MMP_UINT16 Mpeg2TopBufferIndex;
    MMP_UINT16 Mpeg2BotBufferIndex;
    MMP_UINT32 top_field_first;
    MMP_UINT16 Blank_Buffer_Index;
} ISP_CONTEXT;

//=============================================================================
//                Global Data Definition
//=============================================================================
extern ISP_CONTEXT *ISPctxt;

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