#ifndef __CAP_TYPES_H__
#define __CAP_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "encoder/encoder_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define CAP_SCALE_TAP               4
#define CAP_SCALE_TAP_SIZE          8

#if (CFG_CHIP_FAMILY == 9920)
#define CAP_IN_YUYV 0
#define CAP_IN_YVYU 1
#define CAP_IN_UYVY 2
#define CAP_IN_VYUY 3

#else

/* 422 input data order for 1 channel mode */
#define CAP_IN_YUV422_YUYV 0
#define CAP_IN_YUV422_YVYU 1
#define CAP_IN_YUV422_UYVY 2
#define CAP_IN_YUV422_VYUY 3

/*422 input data order for 2 channel mode*/
#define CAP_IN_YUV422_YY_UV 0
#define CAP_IN_YUV422_YY_VU 1
#define CAP_IN_YUV422_UV_YY 2
#define CAP_IN_YUV422_VU_YY 3
#endif

//=============================================================================
//                Macro Definition
//=============================================================================
/**
 *  Debug message
 */
typedef enum _CAP_MSG_TYPE
{
    CAP_MSG_TYPE_ERR          = (0x1 << 0),
} CAP_MSG_TYPE;


#ifdef _MSC_VER // WIN32
    #ifndef trac
    #define trac(string, ...)               do{ printf(string, __VA_ARGS__); \
                                                 printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                            }while(0)
    #endif

    #define cap_msg(type, string, ...)      ((void)((type) ? printf(string, __VA_ARGS__) : MMP_NULL))
    #define cap_msg_ex(type, string, ...)   do{ if(type){ \
                                                   printf(string, __VA_ARGS__); \
                                                   printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                            }while(0)

#else
    #ifndef trac
    #define trac(string, args...)               do{ printf(string, ## args); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                }while(0)
    #endif

    #define cap_msg(type, string, args...)      ((void)((type) ? printf(string, ## args) : MMP_NULL))
    #define cap_msg_ex(type, string, args...)   do{ if(type){ \
                                                       printf(string, ## args); \
                                                       printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                }while(0)
#endif

//=============================================================================
//                Type Definition
//=============================================================================
/**
 *  CAP Update Flags
 */
typedef enum CAP_FLAGS_UPDATE_TAG
{
    CAP_FLAGS_UPDATE_CCMatrix               = (0x00000001 << 0),

} CAP_FLAGS_UPDATE;

typedef enum CAP_INPUT_VIDEO_FORMAT_TAG
{
    Progressive,
    Interleaving

}CAP_INPUT_VIDEO_FORMAT;

typedef enum CAP_INPUT_COLORDEPTH_TAG
{
    COLOR_DEPTH_8_BITS = 0,
    COLOR_DEPTH_10_BITS = 1,
    COLOR_DEPTH_12_BITS = 2

}CAP_INPUT_COLORDEPTH;


typedef enum CAP_INPUT_YUV_WIDTH_MODE_TAG
{
    CPP_1P2T = 0,
    CPP_1P1T = 1

}CAP_INPUT_YUV_WIDTH_MODE;

typedef enum CAP_INPUT_YUV_DATA_FORMAT_TAG
{
#if (CFG_CHIP_FAMILY == 9920)

    YUV422 = 0,
    YUV444 = 1,
	RGB888 = 2,
#else
	YUV444 = 0,
	YUV422 = 1,
#endif

}CAP_INPUT_YUV_DATA_FORMAT;

typedef enum CAP_INPUT_VIDEO_SYNC_MODE_TAG
{
    BT_601 = 0,
    BT_656 = 1

}CAP_INPUT_VIDEO_SYNC_MODE;

typedef enum CAP_ISP_HANDSHAKING_MODE_TAG
{
    MEMORY_MODE,
    ONFLY_MODE

}CAP_ISP_HANDSHAKING_MODE;

typedef enum CAP_MEM_BUF_TAG
{
    CAP_MEM_Y0 = 0,
    CAP_MEM_UV0 = 1,
    CAP_MEM_Y1 = 2,
    CAP_MEM_UV1 = 3,
    CAP_MEM_Y2 = 4,
    CAP_MEM_UV2 = 5,
    CAPTURE_MEM_BUF_COUNT = 6

}CAP_MEM_BUF;

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

}CAP_ERROR_CODE;

typedef enum CAP_VHPOL_STATE_TAG
{
    VHPOL_00,
    VHPOL_01,
    VHPOL_10,
    VHPOL_11

}CAP_VHPOL_STATE;

typedef enum CAP_LANE_STATUS_TAG
{
    CAP_LANE0_STATUS,
    CAP_LANE1_STATUS,
    CAP_LANE2_STATUS,
    CAP_LANE3_STATUS

}CAP_LANE_STATUS;

typedef enum CAP_SKIP_MODE_TAG
{
    CAPTURE_NO_DROP = 0,
    CAPTURE_SKIP_BY_TWO,
    CAPTURE_SKIP_BY_THREE,
    CAPTURE_SKIP_30FPS_TO_25FPS,
    CAPTURE_SKIP_60FPS_TO_25FPS,
} CAP_SKIP_MODE;

#if (CFG_CHIP_FAMILY == 9920)
typedef enum CAP_INPUT_NV12FORMAT_TAG
{
    UV = 0,
   	VU = 1

}CAP_INPUT_NV12FORMAT;

typedef enum CAP_INPUT_DITHER_MODE_TAG
{
    DITHER_OL = 0,
    DITHER_1L = 1,
    DITHER_2L = 2,
    DITHER_3L = 3
    
}CAP_INPUT_DITHER_MODE;


typedef enum CAP_INPUT_DATA_WIDTH_TAG
{
	PIN_24_30_36BITS = 0,	
	PIN_16_20_24BITS = 1,
	PIN_8_10_12BITS  = 2
	
}CAP_INPUT_DATA_WIDTH;

typedef enum CAP_OUTPUT_MEMORY_FOTMAT_TAG
{
	SEMI_PLANAR_420 = 0,	
	SEMI_PLANAR_422 = 1,
	PACKET_MODE_422 = 2
	
}CAP_OUTPUT_MEMORY_FOTMAT;
#endif
//=============================================================================
//                              Structure Definition
//=============================================================================

//Color correction matrix
typedef struct CAP_COLOR_CORRECTION_TAG
{
    MMP_INT16               OffsetR;
    MMP_INT16               OffsetG;
    MMP_INT16               OffsetB;
    MMP_INT16               _11;
    MMP_INT16               _12;
    MMP_INT16               _13;
    MMP_INT16               _21;
    MMP_INT16               _22;
    MMP_INT16               _23;
    MMP_INT16               _31;
    MMP_INT16               _32;
    MMP_INT16               _33;
    MMP_INT16               DeltaR;
    MMP_INT16               DeltaG;
    MMP_INT16               DeltaB;

} CAP_COLOR_CORRECTION;

typedef struct CAP_INPUT_CLK_INFO_TAG
{
    MMP_BOOL                EnUCLK;
    MMP_UINT8               UCLKRatio;
    MMP_UINT8               UCLKDly;
    MMP_BOOL                UCLKInv;
    MMP_UINT8               UCLKSrc;
    MMP_UINT8               UCLKVDSel;

}CAP_INPUT_CLK_INFO;

typedef struct CAP_INPUT_PIN_SELECT_TAG
{
    MMP_UINT8               Y_Pin_Num[12];
    MMP_UINT8               U_Pin_Num[12];
    MMP_UINT8               V_Pin_Num[12];

}CAP_INPUT_PIN_SELECT;

#if (CFG_CHIP_FAMILY == 9920)
typedef struct CAP_INPUT_DITHER_INFO_TAG
{
   MMP_BOOL                EnDither;
   CAP_INPUT_DITHER_MODE   DitherMode;

}CAP_INPUT_DITHER_INFO;
#endif

/* Input Format Info */
typedef struct CAP_INPUT_INFO_TAG
{
    //MMP_BOOL EnDDRMode;
    MMP_BOOL                EnRepeatPixel;
    MMP_BOOL                EnBT656LOI;
    MMP_BOOL                TopFieldPol; //0: HSYNC low, 1: Hsync hight
    MMP_BOOL                SetFieldPol; // 1: Enable register 'TopFieldPol' in BT.601
    MMP_BOOL                EnExtField; // 0: Disable , 1: Enable
    MMP_BOOL                VSyncPol; //'0' 0 stands for blanking '1' 1 stands for blanking
    MMP_BOOL                HSyncPol;
#if (CFG_CHIP_FAMILY == 9920)
	MMP_UINT16				VSyncSkip;
	MMP_UINT16				HSyncSkip;
	MMP_BOOL				HSnapV;		
	MMP_BOOL				CheckHS;
	MMP_BOOL				CheckVS;
	MMP_BOOL				CheckDE;
	MMP_UINT16				WrMergeThresld;
	CAP_INPUT_NV12FORMAT 	NV12Format;
	MMP_UINT32				MemUpBound;
	MMP_UINT32				MemLoBound;
#endif

    CAP_INPUT_VIDEO_SYNC_MODE   EmbeddedSync;
    CAP_INPUT_VIDEO_FORMAT      Interleave; // 0: Progressive , 1 :interleaving
    CAP_INPUT_COLORDEPTH        ColorDepth;

    MMP_UINT16              PitchY;
    MMP_UINT16              PitchUV;
    MMP_UINT16              framerate;

    /* Active Region  Info */
    MMP_UINT16              capwidth;
    MMP_UINT16              capheight;
    MMP_UINT16              HNum1;   /* Input HSync active area start numner [12:0] */
    MMP_UINT16              HNum2;   /* Input HSync active area end numner [12:0] */
    MMP_UINT16              LineNum1;/* Input active area start line number[11:0] of top field */
    MMP_UINT16              LineNum2;/* Input active area end line number[11:0] of top field */
    MMP_UINT16              LineNum3;/* Input active area start line number[11:0] of bottom field */
    MMP_UINT16              LineNum4;/* Input active area end line number[11:0] of bottom field */

    /* ROI Info */
    MMP_UINT16              ROIPosX;
    MMP_UINT16              ROIPosY;
    MMP_UINT16              ROIWidth;
    MMP_UINT16              ROIHeight;

}CAP_INPUT_INFO;

typedef struct CAP_ENFUN_INFO_TAG
{
    MMP_BOOL                EnDDRMode;
    MMP_BOOL                EnDEMode;
    MMP_BOOL                EnCSFun;
    MMP_BOOL                EnCCFun;
    MMP_BOOL                EnInBT656;
#if (CFG_CHIP_FAMILY == 9920)
	MMP_BOOL                EnHSync;
	MMP_BOOL                EnAutoDetHSPol;
	MMP_BOOL                EnAutoDetVSPol;
	MMP_BOOL                EnDumpMode;
	MMP_BOOL                EnMemContinousDump;
	MMP_BOOL                EnSramNap;
	MMP_BOOL				EnMemLimit;
#else
    MMP_BOOL                EnUseExtDE;
    MMP_BOOL                EnUseExtVRst;
    MMP_BOOL                EnUseExtHRst;
    MMP_BOOL                EnNoHSyncForSensor;
#endif
    MMP_BOOL                EnProgressiveToField;
    MMP_BOOL                EnCrossLineDE;
    MMP_BOOL                EnYPbPrTopVSMode;
    MMP_BOOL                EnDlyVS;
    MMP_BOOL                EnHSPosEdge;
    MMP_BOOL                EnPort1UV2LineDS;

}CAP_ENFUN_INFO;

typedef struct CAP_YUV_INFO_TAG
{
#if (CFG_CHIP_FAMILY == 9920)
	MMP_UINT8 					ColorOrder;
	CAP_INPUT_COLORDEPTH		ColorDepth;
	CAP_INPUT_DATA_WIDTH		InputWidth;
#else
    CAP_INPUT_YUV_WIDTH_MODE    ClockPerPixel;
#endif
	MMP_UINT8					YUV422Format;
    CAP_INPUT_YUV_DATA_FORMAT   InputMode;

}CAP_YUV_INFO;

/* Pin I/O  Related Define */
typedef struct CAP_INPUT_MUX_INFO_TAG
{
    MMP_UINT8               Y_Pin_Num[12];
    MMP_UINT8               U_Pin_Num[12];
    MMP_UINT8               V_Pin_Num[12];

    //CAP_INPUT_CLK_INFO
    MMP_BOOL                EnUCLK;
    MMP_UINT8               UCLKRatio;
    MMP_UINT8               UCLKDly;
    MMP_BOOL                UCLKInv;
    MMP_UINT8               UCLKSrc;
    MMP_UINT8               UCLKVDSel;
#if (CFG_CHIP_FAMILY == 9920)
	MMP_BOOL				UCLKAutoDlyEn;
	MMP_UINT8				HS_Pin_Num;
	MMP_UINT8				VS_Pin_Num;
	MMP_UINT8				DE_Pin_Num;

#endif

}CAP_INPUT_MUX_INFO;

typedef struct CAP_OUTPUT_PIN_SELECT_TAG
{
    MMP_UINT8               CAPIOVDOUTSEL[36];
    MMP_UINT8               CAPIOVDOUTSEL_X;

}CAP_OUTPUT_PIN_SELECT;

typedef struct CAP_IO_MODE_INFO_TAG
{
    MMP_UINT16              CAPIOEN_VD_15_0; //CAPIOEN_VD[15:0]
    MMP_UINT16              CAPIOEN_VD_31_16;//CAPIOEN_VD[31:16]
    MMP_UINT16              CAPIOEN_VD_32_35;//CAPIOEN_VD[32:35]
    MMP_UINT8               CAPIOEN_X;
    MMP_UINT8               CAPIOEN_HS;
    MMP_UINT8               CAPIOEN_VS;
    MMP_UINT8               CAPIOEN_DE;
    MMP_UINT8               CAPIOEN_Field;
    MMP_UINT8               CAPIOEN_PMCLK;

    MMP_BOOL                EnInternalRxHDMI;
    MMP_BOOL                CAPIOFFEn_Field;
    MMP_BOOL                CAPIOFFEn_DE;
    MMP_BOOL                CAPIOFFEn_VS;
    MMP_BOOL                CAPIOFFEn_HS;
#if (CFG_CHIP_FAMILY == 9920)
	MMP_UINT32              CAPIOFFEn_VD_00_31;
    MMP_UINT32              CAPIOFFEn_VD_35_32;
#else
    MMP_UINT16              CAPIOFFEn_VD_00_15;
    MMP_UINT16              CAPIOFFEn_VD_16_31;
    MMP_UINT16              CAPIOFFEn_VD_32_47;
    MMP_UINT16              CAPIOFFEn_VD_48_54;
#endif

    MMP_BOOL                HDMICLKInv;
    MMP_UINT8               HDMICLKDly;

}CAP_IO_MODE_INFO;

typedef struct CAP_OUTPUT_INFO_TAG
{
    MMP_UINT16              OutWidth;
    MMP_UINT16              OutHeight;
    MMP_UINT32              OutAddrOffset;
#if (CFG_CHIP_FAMILY == 9920)
	CAP_OUTPUT_MEMORY_FOTMAT OutMemFormat;
#endif

}CAP_OUTPUT_INFO;

typedef struct CAP_COLOR_BAR_CONFIG_TAG
{
    MMP_BOOL                Enable_colorbar;
    MMP_BOOL                Vsync_pol;
    MMP_BOOL                Hsync_pol;
    MMP_UINT16              VS_act_start_line;
    MMP_UINT16              VS_act_line;
    MMP_UINT16              act_line;
    MMP_UINT16              blank_line1;
    MMP_UINT16              blank_line2;
    MMP_UINT16              Hs_act;
    MMP_UINT16              blank_pix1;
    MMP_UINT16              blank_pix2;
    MMP_UINT16              act_pix;

}CAP_COLOR_BAR_CONFIG;

typedef struct CAP_HORSCALE_INFO_TAG
{
    MMP_UINT32              HorScaleHCI;
    MMP_UINT16              HorScaleWidth;

    MMP_UINT8               HorScaleWX_00;
    MMP_UINT8               HorScaleWX_01;
    MMP_UINT8               HorScaleWX_02;
    MMP_UINT8               HorScaleWX_03;

    MMP_UINT8               HorScaleWX_10;
    MMP_UINT8               HorScaleWX_11;
    MMP_UINT8               HorScaleWX_12;
    MMP_UINT8               HorScaleWX_13;

    MMP_UINT8               HorScaleWX_20;
    MMP_UINT8               HorScaleWX_21;
    MMP_UINT8               HorScaleWX_22;
    MMP_UINT8               HorScaleWX_23;

    MMP_UINT8               HorScaleWX_30;
    MMP_UINT8               HorScaleWX_31;
    MMP_UINT8               HorScaleWX_32;
    MMP_UINT8               HorScaleWX_33;

    MMP_UINT8               HorScaleWX_40;
    MMP_UINT8               HorScaleWX_41;
    MMP_UINT8               HorScaleWX_42;
    MMP_UINT8               HorScaleWX_43;

}CAP_HORSCALE_INFO;

//Transfer Matrix RGB to YUV
typedef struct CAP_RGB_TO_YUV_TAG
{
    MMP_UINT16              _11;
    MMP_UINT16              _12;
    MMP_UINT16              _13;
    MMP_UINT16              _21;
    MMP_UINT16              _22;
    MMP_UINT16              _23;
    MMP_UINT16              _31;
    MMP_UINT16              _32;
    MMP_UINT16              _33;
    MMP_UINT16              ConstY;
    MMP_UINT16              ConstU;
    MMP_UINT16              ConstV;

} CAP_RGB_TO_YUV;

/* Capture Scale Function */
typedef struct CAP_SCALE_CTRL_TAG
{
    MMP_FLOAT               HCI;
    MMP_FLOAT               WeightMatX[CAP_SCALE_TAP_SIZE][CAP_SCALE_TAP];

} CAP_SCALE_CTRL;

//color contrl
typedef struct CAP_COLOR_CTRL_TAG
{
    MMP_INT32           brightness;
    MMP_FLOAT           contrast;
    MMP_INT32           hue;
    MMP_FLOAT           saturation;
    MMP_INT32           colorEffect[2];

} CAP_COLOR_CTRL;

typedef struct CAP_CONTEXT_TAG
{
    MMP_BOOL                EnableOnflyMode;
    MMP_BOOL                EnableInterrupt;
    CAP_SKIP_MODE           skip_mode;
    MMP_UINT16              Skippattern;
    MMP_UINT16              SkipPeriod;
#if (CFG_CHIP_FAMILY == 9920)
	MMP_BOOL				bMatchResolution;
	MMP_UINT8				Cap_ID;
	MMP_UINT8				*video_sys_addr; //Benson
	MMP_UINT32				video_vram_addr; //Benson	
	MMP_UINT32				OutAddrY[3];
	MMP_UINT32				OutAddrUV[3];
#endif

    // Input format Info
    CAP_INPUT_INFO          ininfo;

    // Input data format Info
    CAP_YUV_INFO            YUVinfo;

#if (CFG_CHIP_FAMILY == 9920)
	// Input data format Info
	CAP_INPUT_DITHER_INFO   Ditherinfo;
#endif

    // I/O pin Info
    CAP_INPUT_MUX_INFO      inmux_info;

    CAP_OUTPUT_PIN_SELECT   outpin_info;

    CAP_IO_MODE_INFO        iomode_info;

    CAP_OUTPUT_INFO         outinfo;

    // capture enable function
    CAP_ENFUN_INFO          funen;

    // Scale Fun.
    CAP_SCALE_CTRL          ScaleFun;

    // RGB to YUV Fun.
    CAP_RGB_TO_YUV          RGBtoYUVFun;

    // Interrupt
    MMP_BOOL                EnCapIntr;
    MMP_UINT16              IntrMode;

    //Color Correction Function
    CAP_COLOR_CORRECTION    CCFun;

    //color contrl
    CAP_COLOR_CTRL          ColorCtrl;

    //Update Flag
    CAP_FLAGS_UPDATE        UpdateFlags;
}CAP_CONTEXT;

extern CAP_CONTEXT* Capctxt;

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

