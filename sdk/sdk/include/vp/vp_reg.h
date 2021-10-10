#ifndef __VP_REG_H_I7U1OSBH_HFH5_KK3Q_XKST_5P4DY8OJKVRN__
#define __VP_REG_H_I7U1OSBH_HFH5_KK3Q_XKST_5P4DY8OJKVRN__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                              Constant Definition
//=============================================================================
#define VP_REG_BASE                            0x0500


//================================================================
// 0x0500
//================================================================
#define VP_REG_SET500                          (VP_REG_BASE + 0x0000)

#define VP_BIT_VP_REFRESH_PARM                0x0001 //0001 0000 0000 0000
#define VP_BIT_VP_INTERRUPT_CLEAR             0x0001 //0000 0001 0000 0000
#define VP_BIT_VP_SCENE_CHANG_INIT_UPDATE     0x0001 //0000 0000 0001 0000
#define VP_BIT_VP_UPDATE_PARM_EN              0x0001 //0000 0000 0000 1000
#define VP_BIT_DRIVER_FIRE_EN                  0x0001 //0000 0000 0000 0001

#define VP_SHT_VP_REFRESH_PARM                12
#define VP_SHT_VP_INTERRUPT_CLEAR             8
#define VP_SHT_VP_SCENE_CHANG_INIT_UPDATE     4
#define VP_SHT_VP_UPDATE_PARM_EN              3
#define VP_SHT_DRIVER_FIRE_EN                  0

//================================================================
// 0x0502
//================================================================
#define VP_REG_SET502                          (VP_REG_BASE + 0x0002)

#define VP_BIT_SCENE_CHANGE_EN                 0x0001 //0000 0010 0000 0000
#define VP_BIT_CAPTURE_CRTL_DISABLE            0x0001 //0000 0001 0000 0000
#define VP_BIT_RDBUFFER_NUM                    0x0007 //0000 0000 1110 0000
#define VP_BIT_BYPASS_SCALE_EN                 0x0001 //0000 0000 0001 0000
#define VP_BIT_IN_NV_FORMAT                    0x0001 //0000 0000 0000 1000
#define VP_BIT_COLOR_CORRECT_EN                0x0001 //0000 0000 0000 0100
#define VP_BIT_IN_YUV255RANGE_EN               0x0001 //0000 0000 0000 0010
#define VP_BIT_READ_MEM_MODE_EN                0x0001 //0000 0000 0000 0001


#define VP_SHT_SCENE_CHANGE_EN                 9
#define VP_SHT_CAPTURE_CRTL_DISABLE            8
#define VP_SHT_RDBUFFER_NUM                    5
#define VP_SHT_BYPASS_SCALE_EN                 4
#define VP_SHT_IN_NV_FORMAT                    3
#define VP_SHT_COLOR_CORRECT_EN                2
#define VP_SHT_IN_YUV255RANGE_EN               1
#define VP_SHT_READ_MEM_MODE_EN                0

//================================================================
// 0x0504
// Deinterlace function
//================================================================
#define VP_REG_SET504                         (VP_REG_BASE + 0x0004)

#define VP_BIT_UV2D_METHOD_EN                  0x0001 //1000 0000 0000 0000
#define VP_BIT_OUTMOTIONDETECT_EN              0x0001 //0100 0000 0000 0000
#define VP_BIT_LOWLEVELBYPASSBLEND             0x0001 //0010 0000 0000 0000
#define VP_BIT_30LOWLEVELEDGE_DISABLE          0x0001 //0001 0000 0000 0000
#define VP_BIT_LOWLEVELOUTSIDE_EN              0x0001 //0000 1000 0000 0000
#define VP_BIT_LOWLEVELMODE                    0x0001 //0000 0100 0000 0000
#define VP_BIT_LOWLEVELEDGE_EN                 0x0001 //0000 0010 0000 0000
#define VP_BIT_UVREPEAT_MODE                   0x0001 //0000 0001 0000 0000
#define VP_BIT_CHROMA_EDGEDET_EN               0x0001 //0000 0000 1000 0000
#define VP_BIT_LUMA_EDGEDET_EN                 0x0001 //0000 0000 0100 0000
#define VP_BIT_DEINTERLACE_EN                  0x0001 //0000 0000 0010 0000
#define VP_BIT_2D_DEINTER_MODE_EN              0x0001 //0000 0000 0001 0000
#define VP_BIT_SRC_BOTTOM_FIELD_FIRST          0x0001 //0000 0000 0000 1000
#define VP_BIT_DEINTER_BOTTOM_EN               0x0001 //0000 0000 0000 0100
#define VP_BIT_SRC_LPFITR_EN                   0x0001 //0000 0000 0000 0010

#define VP_SHT_UV2D_METHOD_EN                  15
#define VP_SHT_OUTMOTIONDETECT_EN              14
#define VP_SHT_LOWLEVELBYPASSBLEND             13
#define VP_SHT_30LOWLEVELEDGE_DISABLE          12
#define VP_SHT_LOWLEVELOUTSIDE_EN              11
#define VP_SHT_LOWLEVELMODE                    10
#define VP_SHT_LOWLEVELEDGE_EN                 9
#define VP_SHT_UVREPEAT_MODE                   8
#define VP_SHT_CHROMA_EDGEDET_EN               7
#define VP_SHT_LUMA_EDGEDET_EN                 6
#define VP_SHT_DEINTERLACE_EN                  5
#define VP_SHT_2D_DEINTER_MODE_EN              4
#define VP_SHT_SRC_BOTTOM_FIELD_FIRST          3
#define VP_SHT_DEINTER_BOTTOM_EN               2
#define VP_SHT_SRC_LPFITR_EN                   1
 

//================================================================
// 0x0506
// JPEG Encode Parm
//================================================================
#define VP_REG_SET506                         (VP_REG_BASE + 0x0006)

#define VP_BIT_TOTALSLICENUM                   0x00FF//1111 1111 0000 0000
#define VP_BIT_JPEGDECODE_EN                   0x0001//0000 0000 0000 0001

#define VP_SHT_TOTALSLICENUM                   8
#define VP_SHT_JPEGDECODE_EN                   0


//================================================================
// 0x0508
//================================================================
#define VP_REG_SET508                         (VP_REG_BASE + 0x0008)

#define VP_BIT_SWWRFLIP_NUM                    0x0007//1110 0000 0000 0000
#define VP_BIT_SWWRFLIP_EN                     0x0001//0001 0000 0000 0000
#define VP_BIT_SWCTRL_RDADDR_EN                0x0001//0000 1000 0000 0000
#define VP_BIT_WRBUFFER_NUM                    0x0007//0000 0111 0000 0000
#define VP_BIT_ENGINEDELAY                     0x000F//0000 0000 0000 1111

#define VP_SHT_SWWRFLIP_NUM                    13
#define VP_SHT_SWWRFLIP_EN                     12
#define VP_SHT_SWCTRL_RDADDR_EN                11
#define VP_SHT_WRBUFFER_NUM                    8
#define VP_SHT_ENGINEDELAY                     0
//================================================================
// 0x050A
//================================================================
#define VP_REG_SET50A                         (VP_REG_BASE + 0x000A)

#define VP_BIT_OUT_BILINEAR_DOWNSAMPLE_EN      0x0001//0001 0000 0000 0000
#define VP_BIT_OUT_YUVPLANE_FORMAT             0x0003//0000 1100 0000 0000
#define VP_BIT_OUT_NV_FORMAT                   0x0001//0000 0010 0000 0000
#define VP_BIT_OUT_FORMAT                      0x0001//0000 0001 0000 0000
#define VP_BIT_OUTMATRIX_DISABLE               0x0001//0000 0000 0100 0000
#define VP_BIT_OUTPUT_FIELD_MODE               0x0001//0000 0000 0001 0000
#define VP_BIT_REMAP_CHROMAADDR_EN             0x0001//0000 0000 0000 0010
#define VP_BIT_REMAP_LUMAADDR_EN               0x0001//0000 0000 0000 0001

#define VP_SHT_OUT_BILINEAR_DOWNSAMPLE_EN      12
#define VP_SHT_OUT_YUVPLANE_FORMAT             10
#define VP_SHT_OUT_NV_FORMAT                   9
#define VP_SHT_OUT_FORMAT                      8
#define VP_SHT_OUTMATRIX_DISABLE               6
#define VP_SHT_OUTPUT_FIELD_MODE               4
#define VP_SHT_REMAP_CHROMAADDR_EN             1
#define VP_SHT_REMAP_LUMAADDR_EN               0

//================================================================
// 0x050E
//================================================================
#define VP_REG_SET50E                         (VP_REG_BASE + 0x000E)

#define VP_BIT_VP_INTERRUPT_MODE              0x0007//0000 0000 0111 0000
#define VP_BIT_VP_INTERRUPT_EN                0x0001//0000 0000 0000 0010

#define VP_SHT_VP_INTERRUPT_MODE              4
#define VP_SHT_VP_INTERRUPT_EN                1

//================================================================
// 0x0510 ~ 0x0542
// Input buffer base, width, height and pitch
//================================================================
// Decode input buffer
#define VP_REG_INPUT_ADDR_Y0L                  (VP_REG_BASE + 0x0010)
#define VP_REG_INPUT_ADDR_Y0H                  (VP_REG_BASE + 0x0012)
#define VP_REG_INPUT_ADDR_UV0L                 (VP_REG_BASE + 0x0014)
#define VP_REG_INPUT_ADDR_UV0H                 (VP_REG_BASE + 0x0016)

#define VP_REG_INPUT_ADDR_Y1L                  (VP_REG_BASE + 0x0018)
#define VP_REG_INPUT_ADDR_Y1H                  (VP_REG_BASE + 0x001A)
#define VP_REG_INPUT_ADDR_UV1L                 (VP_REG_BASE + 0x001C)
#define VP_REG_INPUT_ADDR_UV1H                 (VP_REG_BASE + 0x001E)

#define VP_REG_INPUT_ADDR_Y2L                  (VP_REG_BASE + 0x0020)
#define VP_REG_INPUT_ADDR_Y2H                  (VP_REG_BASE + 0x0022)
#define VP_REG_INPUT_ADDR_UV2L                 (VP_REG_BASE + 0x0024)
#define VP_REG_INPUT_ADDR_UV2H                 (VP_REG_BASE + 0x0026)

#define VP_REG_INPUT_ADDR_Y3L                  (VP_REG_BASE + 0x0028)
#define VP_REG_INPUT_ADDR_Y3H                  (VP_REG_BASE + 0x002A)
#define VP_REG_INPUT_ADDR_UV3L                 (VP_REG_BASE + 0x002C)
#define VP_REG_INPUT_ADDR_UV3H                 (VP_REG_BASE + 0x002E)

#define VP_REG_INPUT_ADDR_Y4L                  (VP_REG_BASE + 0x0030)
#define VP_REG_INPUT_ADDR_Y4H                  (VP_REG_BASE + 0x0032)
#define VP_REG_INPUT_ADDR_UV4L                 (VP_REG_BASE + 0x0034)
#define VP_REG_INPUT_ADDR_UV4H                 (VP_REG_BASE + 0x0036)

#define VP_REG_INPUT_ADDR_YPL                  (VP_REG_BASE + 0x0038)
#define VP_REG_INPUT_ADDR_YPH                  (VP_REG_BASE + 0x003A)

#define VP_REG_INPUT_PITCH_Y                   (VP_REG_BASE + 0x003C)
#define VP_REG_INPUT_PITCH_UV                  (VP_REG_BASE + 0x003E)
#define VP_REG_INPUT_HEIGHT                    (VP_REG_BASE + 0x0040)
#define VP_REG_INPUT_WIDTH                     (VP_REG_BASE + 0x0042)

#define VP_BIT_INPUT_ADDR_L                    0xFFFF //1111 1111 1111 1111 15:0
#define VP_BIT_INPUT_ADDR_H                    0xFFFF //1111 1111 1111 1111 15:0
#define VP_BIT_INPUT_PITCH_Y                   0x7FFF //0111 1111 1111 1111 14:0
#define VP_BIT_INPUT_PITCH_UV                  0x7FFF //0111 1111 1111 1111 14:0
#define VP_BIT_INPUT_HEIGHT                    0x1FFF //0001 1111 1111 1111 12:0
#define VP_BIT_INPUT_WIDTH                     0x1FFF //0001 1111 1111 1111 12:0

//================================================================
// 0x0544 ~ 0x054A
// Scaling factor
//================================================================
#define VP_REG_SCALE_HCI_L                     (VP_REG_BASE + 0x0044)// HCI 
#define VP_REG_SCALE_HCI_H                     (VP_REG_BASE + 0x0046)// HCI 
#define VP_REG_SCALE_VCI_L                     (VP_REG_BASE + 0x0048)// VCI 
#define VP_REG_SCALE_VCI_H                     (VP_REG_BASE + 0x004A)// VCI 

#define VP_BIT_SCALE_L                         0xFFFF //1111 1111 1111 1111 15:0
#define VP_BIT_SCALE_H                         0x000F //0000 0000 0000 1111 3:0

//================================================================
// 0x054E ~ 0x0558
// Scene Change Parameter
//================================================================
#define VP_REG_SCENE_HOFFSET                   (VP_REG_BASE + 0x004E)
#define VP_REG_SCENE_VOFFSET                   (VP_REG_BASE + 0x0050)
#define VP_REG_SCENE_HSTEP                     (VP_REG_BASE + 0x0052)
#define VP_REG_SCENE_VSTEP                     (VP_REG_BASE + 0x0054)
#define VP_REG_SCENE_STEPNO                    (VP_REG_BASE + 0x0056)
#define VP_REG_SCENE_INITVALUE                 (VP_REG_BASE + 0x0058)

#define VP_BIT_SCENE_HOFFSET                   0x00FF //0000 0000 1111 1111 7:0
#define VP_BIT_SCENE_VOFFSET                   0x00FF //0000 0000 1111 1111 7:0
#define VP_BIT_SCENE_HSTEP                     0x3FFF //0011 1111 1111 1111 12:0
#define VP_BIT_SCENE_VSTEP                     0x3FFF //0011 1111 1111 1111 12:0
#define VP_BIT_SCENE_STEPNO                    0x01FF //0000 0001 1111 1111 8:0
#define VP_BIT_SCENE_INITVALUE                 0x00FF //0000 0000 1111 1111 7:0

//================================================================
// 0x055A ~ 0x0570
// Color correction matrix & delta R/G/B
//================================================================
#define VP_REG_COL_COR_11                      (VP_REG_BASE + 0x005A)
#define VP_REG_COL_COR_12                      (VP_REG_BASE + 0x005C)
#define VP_REG_COL_COR_13                      (VP_REG_BASE + 0x005E)
#define VP_REG_COL_COR_21                      (VP_REG_BASE + 0x0060)
#define VP_REG_COL_COR_22                      (VP_REG_BASE + 0x0062)
#define VP_REG_COL_COR_23                      (VP_REG_BASE + 0x0064)
#define VP_REG_COL_COR_31                      (VP_REG_BASE + 0x0066)
#define VP_REG_COL_COR_32                      (VP_REG_BASE + 0x0068)
#define VP_REG_COL_COR_33                      (VP_REG_BASE + 0x006A)

#define VP_REG_COL_COR_DELTA_R                 (VP_REG_BASE + 0x006C)
#define VP_REG_COL_COR_DELTA_G                 (VP_REG_BASE + 0x006E)
#define VP_REG_COL_COR_DELTA_B                 (VP_REG_BASE + 0x0070)

#define VP_BIT_COL_COR                         0x1FFF //0001 1111 1111 1111 12:0
#define VP_BIT_COL_CORR_DELTA                  0x01FF //0000 0001 1111 1111 8:0

//================================================================
// 0x0572 ~ 0x05B4
// Output Address, Width, Height and Pitch
//================================================================
#define VP_REG_OUT_ADDR_Y0L                    (VP_REG_BASE + 0x0072)
#define VP_REG_OUT_ADDR_Y0H                    (VP_REG_BASE + 0x0074)
#define VP_REG_OUT_ADDR_U0L                    (VP_REG_BASE + 0x0076)
#define VP_REG_OUT_ADDR_U0H                    (VP_REG_BASE + 0x0078)
#define VP_REG_OUT_ADDR_V0L                    (VP_REG_BASE + 0x007A)
#define VP_REG_OUT_ADDR_V0H                    (VP_REG_BASE + 0x007C)

#define VP_REG_OUT_ADDR_Y1L                    (VP_REG_BASE + 0x007E)
#define VP_REG_OUT_ADDR_Y1H                    (VP_REG_BASE + 0x0080)
#define VP_REG_OUT_ADDR_U1L                    (VP_REG_BASE + 0x0082)
#define VP_REG_OUT_ADDR_U1H                    (VP_REG_BASE + 0x0084)
#define VP_REG_OUT_ADDR_V1L                    (VP_REG_BASE + 0x0086)
#define VP_REG_OUT_ADDR_V1H                    (VP_REG_BASE + 0x0088)

#define VP_REG_OUT_ADDR_Y2L                    (VP_REG_BASE + 0x008A)
#define VP_REG_OUT_ADDR_Y2H                    (VP_REG_BASE + 0x008C)
#define VP_REG_OUT_ADDR_U2L                    (VP_REG_BASE + 0x008E)
#define VP_REG_OUT_ADDR_U2H                    (VP_REG_BASE + 0x0090)
#define VP_REG_OUT_ADDR_V2L                    (VP_REG_BASE + 0x0092)
#define VP_REG_OUT_ADDR_V2H                    (VP_REG_BASE + 0x0094)

#define VP_REG_OUT_ADDR_Y3L                    (VP_REG_BASE + 0x0096)
#define VP_REG_OUT_ADDR_Y3H                    (VP_REG_BASE + 0x0098)
#define VP_REG_OUT_ADDR_U3L                    (VP_REG_BASE + 0x009A)
#define VP_REG_OUT_ADDR_U3H                    (VP_REG_BASE + 0x009C)
#define VP_REG_OUT_ADDR_V3L                    (VP_REG_BASE + 0x009E)
#define VP_REG_OUT_ADDR_V3H                    (VP_REG_BASE + 0x00A0)

#define VP_REG_OUT_ADDR_Y4L                    (VP_REG_BASE + 0x00A2)
#define VP_REG_OUT_ADDR_Y4H                    (VP_REG_BASE + 0x00A4)
#define VP_REG_OUT_ADDR_U4L                    (VP_REG_BASE + 0x00A6)
#define VP_REG_OUT_ADDR_U4H                    (VP_REG_BASE + 0x00A8)
#define VP_REG_OUT_ADDR_V4L                    (VP_REG_BASE + 0x00AA)
#define VP_REG_OUT_ADDR_V4H                    (VP_REG_BASE + 0x00AC)

#define VP_REG_OUT_WIDTH                       (VP_REG_BASE + 0x00AE)
#define VP_REG_OUT_HEIGHT                      (VP_REG_BASE + 0x00B0)
#define VP_REG_OUT_Y_PITCH                     (VP_REG_BASE + 0x00B2)
#define VP_REG_OUT_UV_PITCH                    (VP_REG_BASE + 0x00B4)

#define VP_BIT_OUT_ADDR_L                      0xFFFF //1111 1111 1111 1111 15:0
#define VP_BIT_OUT_ADDR_H                      0xFFFF //1111 1111 1111 1111 15:0
#define VP_BIT_OUT_WIDTH                       0x3FFF //0011 1111 1111 1111 13:0
#define VP_BIT_OUT_HEIGHT                      0x3FFF //0011 1111 1111 1111 13:0
#define VP_BIT_OUT_PITCH                       0x7FFF //0111 1111 1111 1111 14:0

//================================================================
// 0x05B6 ~ 0x05BA
// 3D Deinterlace Parm
//================================================================
#define VP_REG_3D_DEINTER_PARM_1               (VP_REG_BASE + 0x00B6)
#define VP_REG_3D_DEINTER_PARM_2               (VP_REG_BASE + 0x00B8)
#define VP_REG_3D_DEINTER_PARM_3               (VP_REG_BASE + 0x00BA)

#define VP_BIT_3D_MDTHRED_HIGH                 0x00FF //1111 1111 0000 0000
#define VP_BIT_3D_MDTHRED_LOW                  0x00FF //0000 0000 1111 1111

#define VP_BIT_DISABLE_MOTIONVALUE_A           0x0001 //1000 0000 0000 0000
#define VP_BIT_DISABLE_MOTIONVALUE_B           0x0001 //0100 0000 0000 0000
#define VP_BIT_DISABLE_MOTIONVALUE_C           0x0001 //0010 0000 0000 0000
#define VP_BIT_DISABLE_MOTIONVALUE_D           0x0001 //0001 0000 0000 0000
#define VP_BIT_DISABLE_MOTIONVALUE_E           0x0001 //0000 1000 0000 0000
#define VP_BIT_DISABLE_MOTIONVALUE_F           0x0001 //0000 0100 0000 0000
#define VP_BIT_DISABLE_MOTIONVALUE_G           0x0001 //0000 0010 0000 0000
#define VP_BIT_LPF_WEIGHT_EN                   0x0001 //0000 0001 0000 0000
#define VP_BIT_LPF_BLEND_EN                    0x0001 //0000 0000 1000 0000
#define VP_BIT_3D_MDTHRED_STEP                 0x007F //0000 0000 0111 1111

#define VP_BIT_LPF_STATICPIXEL_EN              0x0001 //0000 0000 0000 0001

#define VP_SHT_3D_MDTHRED_HIGH                 8
#define VP_SHT_3D_MDTHRED_LOW                  0

#define VP_SHT_DISABLE_MOTIONVALUE_A           15
#define VP_SHT_DISABLE_MOTIONVALUE_B           14
#define VP_SHT_DISABLE_MOTIONVALUE_C           13
#define VP_SHT_DISABLE_MOTIONVALUE_D           12
#define VP_SHT_DISABLE_MOTIONVALUE_E           11
#define VP_SHT_DISABLE_MOTIONVALUE_F           10
#define VP_SHT_DISABLE_MOTIONVALUE_G           9
#define VP_SHT_LPF_WEIGHT_EN                   8
#define VP_SHT_LPF_BLEND_EN                    7
#define VP_SHT_3D_MDTHRED_STEP                 0

#define VP_SHT_LPF_STATICPIXEL_EN              0

//================================================================
// 0x05BC
// 2D Deinterlace Parm
//================================================================
#define VP_REG_2D_DEINTER_PARM_1               (VP_REG_BASE + 0x00BC)

#define VP_BIT_2D_EDGE_WEIGHT                  0x003F //0011 1111 1000 0000
#define VP_BIT_2D_ORG_WEIGHT                   0x007F //0000 0000 0111 1111

#define VP_SHT_2D_EDGE_WEIGHT                  7
#define VP_SHT_2D_ORG_WEIGHT                   0

//================================================================
// 0x05D0 ~ 0x05FA
// Frame function
//================================================================
#define VP_REG_SET_FRMFUN_0                    (VP_REG_BASE + 0x00D0)
#define VP_REG_CONST_ALPHA_0                   (VP_REG_BASE + 0x00D2)
#define VP_REG_FRMFUN_0_KEY_RG                 (VP_REG_BASE + 0x00D4)
#define VP_REG_FRMFUN_0_KEY_B                  (VP_REG_BASE + 0x00D6)
#define VP_REG_FRMFUN_0_ADDR_L                 (VP_REG_BASE + 0x00D8)
#define VP_REG_FRMFUN_0_ADDR_H                 (VP_REG_BASE + 0x00DA)
#define VP_REG_FRMFUN_0_START_X                (VP_REG_BASE + 0x00DC)
#define VP_REG_FRMFUN_0_START_Y                (VP_REG_BASE + 0x00DE)
#define VP_REG_FRMFUN_0_WIDTH                  (VP_REG_BASE + 0x00E0)
#define VP_REG_FRMFUN_0_HEIGHT                 (VP_REG_BASE + 0x00E2)
#define VP_REG_FRMFUN_0_PITCH                  (VP_REG_BASE + 0x00E4)

#define VP_BIT_FRMFUN_GOBANG_EN                0x0001 //0000 0010 0000 0000
#define VP_BIT_FRMFUN_RGB2YUV_EN               0x0001 //0000 0001 0000 0000
#define VP_BIT_FRMFUN_FIELDMODE_EN             0x0001 //0000 0000 0001 0000
#define VP_BIT_FRMFUN_MODE                     0x0001 //0000 0000 0000 0100
#define VP_BIT_FRMFUN_ALPHA_BLEND_EN           0x0001 //0000 0000 0000 0010
#define VP_BIT_FRMFUN_EN                       0x0001 //0000 0000 0000 0001

#define VP_BIT_CONST_ALPHA                     0x00FF //0000 0000 1111 1111
#define VP_BIT_FRMFUN_KEY                      0x00FF //0000 0000 1111 1111

#define VP_BIT_FRMFUN_ADDR_L                   0xFFFF //1111 1111 1111 1111 15:0
#define VP_BIT_FRMFUN_ADDR_H                   0x03FF //0000 0011 1111 1111 9:0
#define VP_BIT_FRMFUN_START_X                  0x07FF //0000 0111 1111 1111 10:0
#define VP_BIT_FRMFUN_START_Y                  0x07FF //0000 0111 1111 1111 10:0
#define VP_BIT_FRMFUN_WIDTH                    0x07FF //0000 0111 1111 1111 10:0
#define VP_BIT_FRMFUN_HEIGHT                   0x07FF //0000 0111 1111 1111 10:0
#define VP_BIT_FRMFUN_PITCH                    0x7FFF //0111 1111 1111 1111 14:0

#define VP_SHT_FRMFUN_GOBANG_EN                9
#define VP_SHT_FRMFUN_RGB2YUV_EN               8
#define VP_SHT_FRMFUN_FIELDMODE_EN             4
#define VP_SHT_FRMFUN_MODE                     2
#define VP_SHT_FRMFUN_ALPHA_BLEND_EN           1
#define VP_SHT_FRMFUN_EN                       0

#define VP_SHT_FRMFUN_KEY_R                    0  
#define VP_SHT_FRMFUN_KEY_G                    8 

//================================================================
// 0x060A
// Hardware debug register.
//================================================================
#define VP_REG_DEBUF_SETTING                   (VP_REG_BASE + 0x010A)

#define VP_BIT_DEBUG_SEL                       0x000F //1111 0000 0000 0000
#define VP_BIT_TEST_FIELDMODE                  0x0001 //0000 0000 0001 0000
#define VP_BIT_TEST_AUTOFIRE                   0x0001 //0000 0000 0000 0001

#define VP_SHT_DEBUG_SEL                       12
#define VP_SHT_TEST_FIELDMODE                  4
#define VP_SHT_TEST_AUTOFIRE                   0

//================================================================
// 0x0614 ~ 0x063A
// Scale weighting matrix
//================================================================
#define VP_REG_SCALEWX0100                     (VP_REG_BASE + 0x0114)
#define VP_REG_SCALEWX0302                     (VP_REG_BASE + 0x0116)
#define VP_REG_SCALEWX1110                     (VP_REG_BASE + 0x0118)
#define VP_REG_SCALEWX1312                     (VP_REG_BASE + 0x011A)
#define VP_REG_SCALEWX2120                     (VP_REG_BASE + 0x011C)
#define VP_REG_SCALEWX2322                     (VP_REG_BASE + 0x011E)
#define VP_REG_SCALEWX3130                     (VP_REG_BASE + 0x0120)
#define VP_REG_SCALEWX3332                     (VP_REG_BASE + 0x0122)
#define VP_REG_SCALEWX4140                     (VP_REG_BASE + 0x0124)
#define VP_REG_SCALEWX4342                     (VP_REG_BASE + 0x0126)

#define VP_REG_SCALEWY0100                     (VP_REG_BASE + 0x0128)
#define VP_REG_SCALEWY0302                     (VP_REG_BASE + 0x012A)
#define VP_REG_SCALEWY1110                     (VP_REG_BASE + 0x012C)
#define VP_REG_SCALEWY1312                     (VP_REG_BASE + 0x012E)
#define VP_REG_SCALEWY2120                     (VP_REG_BASE + 0x0130)
#define VP_REG_SCALEWY2322                     (VP_REG_BASE + 0x0132)
#define VP_REG_SCALEWY3130                     (VP_REG_BASE + 0x0134)
#define VP_REG_SCALEWY3332                     (VP_REG_BASE + 0x0136)
#define VP_REG_SCALEWY4140                     (VP_REG_BASE + 0x0138)
#define VP_REG_SCALEWY4342                     (VP_REG_BASE + 0x013A)

#define VP_BIT_SCALEWEIGHT                     0x00FF //0000 0000 1111 1111

#define VP_SHT_SCALEWEIGHT_H                   8
#define VP_SHT_SCALEWEIGHT_L                   0

//================================================================
// 0x0656 ~ 0x065A
// CC offset
//================================================================
#define VP_REG_CC_IN_OFFSET_R                  (VP_REG_BASE + 0x0156)
#define VP_REG_CC_IN_OFFSET_G                  (VP_REG_BASE + 0x0158)
#define VP_REG_CC_IN_OFFSET_B                  (VP_REG_BASE + 0x015A)

#define VP_BIT_IN_OFFSET                       0x01FF //0000 0001 1111 1111

//================================================================
// 0x065C ~ 0x0662
// LowLevelEdge
//================================================================
#define VP_REG_LOWLEVELEDGE_START_X            (VP_REG_BASE + 0x015C)
#define VP_REG_LOWLEVELEDGE_START_Y            (VP_REG_BASE + 0x015E)
#define VP_REG_LOWLEVELEDGE_WIDTH              (VP_REG_BASE + 0x0160)
#define VP_REG_LOWLEVELEDGE_HEIGHT             (VP_REG_BASE + 0x0162)


//================================================================
// 0x0664 ~ 0x067A
// Transfer matrix for RGB to YUV
//================================================================
#define VP_REG_RGB_TO_YUV_11                   (VP_REG_BASE + 0x0164)
#define VP_REG_RGB_TO_YUV_12                   (VP_REG_BASE + 0x0166)
#define VP_REG_RGB_TO_YUV_13                   (VP_REG_BASE + 0x0168)
#define VP_REG_RGB_TO_YUV_21                   (VP_REG_BASE + 0x016A)
#define VP_REG_RGB_TO_YUV_22                   (VP_REG_BASE + 0x016C)
#define VP_REG_RGB_TO_YUV_23                   (VP_REG_BASE + 0x016E)
#define VP_REG_RGB_TO_YUV_31                   (VP_REG_BASE + 0x0170)
#define VP_REG_RGB_TO_YUV_32                   (VP_REG_BASE + 0x0172)
#define VP_REG_RGB_TO_YUV_33                   (VP_REG_BASE + 0x0174)

#define VP_REG_RGB_TO_YUV_CONST_Y              (VP_REG_BASE + 0x0176)
#define VP_REG_RGB_TO_YUV_CONST_U              (VP_REG_BASE + 0x0178)
#define VP_REG_RGB_TO_YUV_CONST_V              (VP_REG_BASE + 0x017A)

#define VP_BIT_RGB_TO_YUV                      0x03FF //0000 0011 1111 1111
#define VP_BIT_RGB_TO_YUV_CONST                0x01FF //0000 0001 1111 1111

//================================================================
// 0x0680 ~ 0x0696
// Transfer matrix Frmfun for RGB to YUV
//================================================================
#define VP_REG_FRM_RGB2YUV_11                   (VP_REG_BASE + 0x0180)
#define VP_REG_FRM_RGB2YUV_12                   (VP_REG_BASE + 0x0182)
#define VP_REG_FRM_RGB2YUV_13                   (VP_REG_BASE + 0x0184)
#define VP_REG_FRM_RGB2YUV_21                   (VP_REG_BASE + 0x0186)
#define VP_REG_FRM_RGB2YUV_22                   (VP_REG_BASE + 0x0188)
#define VP_REG_FRM_RGB2YUV_23                   (VP_REG_BASE + 0x018A)
#define VP_REG_FRM_RGB2YUV_31                   (VP_REG_BASE + 0x018C)
#define VP_REG_FRM_RGB2YUV_32                   (VP_REG_BASE + 0x018E)
#define VP_REG_FRM_RGB2YUV_33                   (VP_REG_BASE + 0x0190)

#define VP_REG_FRM_RGB2YUV_CONST_Y              (VP_REG_BASE + 0x0192)
#define VP_REG_FRM_RGB2YUV_CONST_U              (VP_REG_BASE + 0x0194)
#define VP_REG_FRM_RGB2YUV_CONST_V              (VP_REG_BASE + 0x0196)

#define VP_BIT_FRM_RGB2YUV                      0x03FF //0000 0011 1111 1111
#define VP_BIT_FRM_RGB2YUV_CONST                0x01FF //0000 0001 1111 1111

//================================================================
// 0x06A0 ~ 0x06BC
// Remap luma address
//================================================================
#define VP_REG_MAPADR_Y_0403                    (VP_REG_BASE + 0x01A0)
#define VP_REG_MAPADR_Y_0605                    (VP_REG_BASE + 0x01A2)
#define VP_REG_MAPADR_Y_0807                    (VP_REG_BASE + 0x01A4)
#define VP_REG_MAPADR_Y_1009                    (VP_REG_BASE + 0x01A6)
#define VP_REG_MAPADR_Y_1211                    (VP_REG_BASE + 0x01A8)
#define VP_REG_MAPADR_Y_1413                    (VP_REG_BASE + 0x01AA)
#define VP_REG_MAPADR_Y_1615                    (VP_REG_BASE + 0x01AC)
#define VP_REG_MAPADR_Y_1817                    (VP_REG_BASE + 0x01AE)
#define VP_REG_MAPADR_Y_2019                    (VP_REG_BASE + 0x01B0)
#define VP_REG_MAPADR_Y_2221                    (VP_REG_BASE + 0x01B2)
#define VP_REG_MAPADR_Y_2423                    (VP_REG_BASE + 0x01B4)
#define VP_REG_MAPADR_Y_2625                    (VP_REG_BASE + 0x01B6)
#define VP_REG_MAPADR_Y_2827                    (VP_REG_BASE + 0x01B8)
#define VP_REG_MAPADR_Y_3029                    (VP_REG_BASE + 0x01BA)
#define VP_REG_MAPADR_Y_XX31                    (VP_REG_BASE + 0x01BC)

#define VP_BIT_MAPADDR_Y                        0x3F3F //0011 1111 0011 1111

//================================================================
// 0x06C0 ~ 0x06DC
// Remap chroma address
//================================================================
#define VP_REG_MAPADR_UV_0403                   (VP_REG_BASE + 0x01C0)
#define VP_REG_MAPADR_UV_0605                   (VP_REG_BASE + 0x01C2)
#define VP_REG_MAPADR_UV_0807                   (VP_REG_BASE + 0x01C4)
#define VP_REG_MAPADR_UV_1009                   (VP_REG_BASE + 0x01C6)
#define VP_REG_MAPADR_UV_1211                   (VP_REG_BASE + 0x01C8)
#define VP_REG_MAPADR_UV_1413                   (VP_REG_BASE + 0x01CA)
#define VP_REG_MAPADR_UV_1615                   (VP_REG_BASE + 0x01CC)
#define VP_REG_MAPADR_UV_1817                   (VP_REG_BASE + 0x01CE)
#define VP_REG_MAPADR_UV_2019                   (VP_REG_BASE + 0x01D0)
#define VP_REG_MAPADR_UV_2221                   (VP_REG_BASE + 0x01D2)
#define VP_REG_MAPADR_UV_2423                   (VP_REG_BASE + 0x01D4)
#define VP_REG_MAPADR_UV_2625                   (VP_REG_BASE + 0x01D6)
#define VP_REG_MAPADR_UV_2827                   (VP_REG_BASE + 0x01D8)
#define VP_REG_MAPADR_UV_3029                   (VP_REG_BASE + 0x01DA)
#define VP_REG_MAPADR_UV_XX31                   (VP_REG_BASE + 0x01DC)

#define VP_BIT_MAPADDR_UV                       0x3F3F //0011 1111 0011 1111

//================================================================
// 0x06E0 ~ 0x06F2
// Gobang Mask
//================================================================
#define VP_REG_FRMFUN_MASK_00                   (VP_REG_BASE + 0x01E0)
#define VP_REG_FRMFUN_MASK_01                   (VP_REG_BASE + 0x01E2)
#define VP_REG_FRMFUN_MASK_02                   (VP_REG_BASE + 0x01E4)
#define VP_REG_FRMFUN_MASK_03                   (VP_REG_BASE + 0x01E6)
#define VP_REG_FRMFUN_MASK_04                   (VP_REG_BASE + 0x01E8)
#define VP_REG_FRMFUN_MASK_05                   (VP_REG_BASE + 0x01EA)
#define VP_REG_FRMFUN_MASK_06                   (VP_REG_BASE + 0x01EC)
#define VP_REG_FRMFUN_MASK_07                   (VP_REG_BASE + 0x01EE)
#define VP_REG_FRMFUN_MASK_08                   (VP_REG_BASE + 0x01F0)
#define VP_REG_FRMFUN_MASK_09                   (VP_REG_BASE + 0x01F2)

#define VP_BIT_FRMFUN_MASK                      0xFFFF //1111 1111 1111 1111 15:0

//================================================================
// 0x06FC
// ISP engine status
//================================================================
#define VP_REG_VP_ENGINE_STATUS               (VP_REG_BASE + 0x01FC)

//================================================================
// 0x06FE
// ISP engine status 2
//================================================================
#define VP_REG_VP_ENGINE_STATUS_2             (VP_REG_BASE + 0x01FE)

//================================================================
// 0x0700
// ISP engine status 3
//================================================================
#define VP_REG_VP_ENGINE_STATUS_3             (VP_REG_BASE + 0x0200)

//=============================================================================
//                Macro Definition
//=============================================================================


//=============================================================================
//                Structure Definition
//=============================================================================


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
