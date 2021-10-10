#ifndef __ISP_REG_H_I7U1OSBH_HFH5_KK3Q_XKST_5P4DY8OJKVRN__
#define __ISP_REG_H_I7U1OSBH_HFH5_KK3Q_XKST_5P4DY8OJKVRN__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#define ISP_REG_BASE                            0x0500

//================================================================
// 0x0500
//================================================================
#define ISP_REG_SET500                          (ISP_REG_BASE + 0x0000)

#define ISP_BIT_ISP_INTERRUPT_CLEAR             0x0001 //0000 0001 0000 0000
#define ISP_BIT_DRIVER_FIRE_EN                  0x0001 //0000 0000 0000 0001

#define ISP_SHT_ISP_INTERRUPT_CLEAR             8
#define ISP_SHT_DRIVER_FIRE_EN                  0

//================================================================
// 0x0502
//================================================================
#define ISP_REG_SET502                          (ISP_REG_BASE + 0x0002)

#define ISP_BIT_REMAP_CHROMAADDR_EN             0x0001 //0000 0000 0100 0000
#define ISP_BIT_REMAP_LUMAADDR_EN               0x0001 //0000 0000 0010 0000
#define ISP_BIT_IN_YUV255RANGE_EN               0x0001 //0000 0000 0001 0000
#define ISP_BIT_UVREPEAT_MODE                   0x0001 //0000 0000 0000 1000
#define ISP_BIT_COLOR_CORRECT_EN                0x0001 //0000 0000 0000 0100
#define ISP_BIT_COLOR_SPECE_EN                  0x0001 //0000 0000 0000 0010
#define ISP_BIT_DOWN_SAMPLE_EN                  0x0001 //0000 0000 0000 0001

#define ISP_SHT_REMAP_CHROMAADDR_EN             6
#define ISP_SHT_REMAP_LUMAADDR_EN               5
#define ISP_SHT_IN_YUV255RANGE_EN               4
#define ISP_SHT_UVREPEAT_MODE                   3
#define ISP_SHT_COLOR_CORRECT_EN                2
#define ISP_SHT_COLOR_SPECE_EN                  1
#define ISP_SHT_DOWN_SAMPLE_EN                  0

//================================================================
// 0x0504
// Deinterlace function
//================================================================
#define ISP_REG_SET_DEINTERLACE                 (ISP_REG_BASE + 0x0004)

#define ISP_BIT_UV2D_METHOD_EN                  0x0001 //1000 0000 0000 0000
#define ISP_BIT_LOWLEVELBYPASSBLEND             0x0001 //0010 0000 0000 0000
#define ISP_BIT_30LOWLEVELEDGE_DISABLE          0x0001 //0001 0000 0000 0000
#define ISP_BIT_LOWLEVELOUTSIDE_EN              0x0001 //0000 1000 0000 0000
#define ISP_BIT_LOWLEVELMODE                    0x0001 //0000 0100 0000 0000
#define ISP_BIT_LOWLEVELEDGE_EN                 0x0001 //0000 0010 0000 0000
#define ISP_BIT_CHROMA_EDGEDET_EN               0x0001 //0000 0000 1000 0000
#define ISP_BIT_LUMA_EDGEDET_EN                 0x0001 //0000 0000 0100 0000
#define ISP_BIT_DEINTERLACE_EN                  0x0001 //0000 0000 0010 0000
#define ISP_BIT_2D_DEINTER_MODE_EN              0x0001 //0000 0000 0001 0000
#define ISP_BIT_SRC_BOTTOM_FIELD_FIRST          0x0001 //0000 0000 0000 1000
#define ISP_BIT_DEINTER_BOTTOM_EN               0x0001 //0000 0000 0000 0100
#define ISP_BIT_SRC_LPFITR_EN                   0x0001 //0000 0000 0000 0010

#define ISP_SHT_UV2D_METHOD_EN                  15
#define ISP_SHT_LOWLEVELBYPASSBLEND             13
#define ISP_SHT_30LOWLEVELEDGE_DISABLE          12
#define ISP_SHT_LOWLEVELOUTSIDE_EN              11
#define ISP_SHT_LOWLEVELMODE                    10
#define ISP_SHT_LOWLEVELEDGE_EN                 9
#define ISP_SHT_CHROMA_EDGEDET_EN               7
#define ISP_SHT_LUMA_EDGEDET_EN                 6
#define ISP_SHT_DEINTERLACE_EN                  5
#define ISP_SHT_2D_DEINTER_MODE_EN              4
#define ISP_SHT_SRC_BOTTOM_FIELD_FIRST          3
#define ISP_SHT_DEINTER_BOTTOM_EN               2
#define ISP_SHT_SRC_LPFITR_EN                   1

//================================================================
// 0x0506
// Input format
//================================================================
#define ISP_REG_INPUT_FORMAT                    (ISP_REG_BASE + 0x0006)

#define ISP_BIT_IN_PLANE_FORMAT                 0x0003//0000 0011 0000 0000

#define ISP_SHT_IN_PLANE_FORMAT                 8

//================================================================
// 0x0508
//================================================================
#define ISP_REG_RUNLENENC_PARM                  (ISP_REG_BASE + 0x0008)

#define ISP_BIT_RUNLENENC_RUNSIZE               0x0007//0000 0111 0000 0000
#define ISP_BIT_RUNLENENC_REJECT_EN             0x0001//0000 0000 0000 0100
#define ISP_BIT_RUNLENENC_UNITSIZE              0x0001//0000 0000 0000 0010
#define ISP_BIT_RUNLENENC_EN                    0x0001//0000 0000 0000 0001
#define ISP_BIT_JPEGDECODE_MODE                 0x0001//0000 0000 1000 0000

#define ISP_SHT_RUNLENENC_RUNSIZE               8
#define ISP_SHT_JPEGDECODE_MODE                 7
#define ISP_SHT_RUNLENENC_REJECT_EN             2
#define ISP_SHT_RUNLENENC_UNITSIZE              1
#define ISP_SHT_RUNLENENC_EN                    0

//================================================================
// 0x050A
//================================================================
#define ISP_REG_OUTPUT_FORMAT                   (ISP_REG_BASE + 0x000A)

#define ISP_BIT_OUT_RGB_FORMAT                  0x0003//1100 0000 0000 0000
#define ISP_BIT_OUT_DITHER_MODE                 0x0001//0000 0000 1000 0000

#define ISP_SHT_OUT_RGB_FORMAT                  14
#define ISP_SHT_OUT_DITHER_MODE                 7

//================================================================
// 0x050C
//================================================================
#define ISP_REG_SET50C                          (ISP_REG_BASE + 0x000C)

#define ISP_BIT_TOTALSLICENUM                   0x00FF//1111 1111 0000 0000

#define ISP_SHT_TOTALSLICENUM                   8

//================================================================
// 0x050E
//================================================================
#define ISP_REG_SET50E                          (ISP_REG_BASE + 0x000E)

#define ISP_BIT_ISP_INTERRUPT_MODE              0x0007//0000 0000 0111 0000
#define ISP_BIT_ISP_INTERRUPT_EN                0x0001//0000 0000 0000 0010

#define ISP_SHT_ISP_INTERRUPT_MODE              4
#define ISP_SHT_ISP_INTERRUPT_EN                1

//================================================================
// 0x0510 ~ 0x052E
// Input buffer base, width, height and pitch
//================================================================
// Decode input buffer
#define ISP_REG_INPUT_ADDR_YL                   (ISP_REG_BASE + 0x0010)
#define ISP_REG_INPUT_ADDR_YH                   (ISP_REG_BASE + 0x0012)
#define ISP_REG_INPUT_ADDR_UL                   (ISP_REG_BASE + 0x0014)
#define ISP_REG_INPUT_ADDR_UH                   (ISP_REG_BASE + 0x0016)
#define ISP_REG_INPUT_ADDR_VL                   (ISP_REG_BASE + 0x0018)
#define ISP_REG_INPUT_ADDR_VH                   (ISP_REG_BASE + 0x001A)

#define ISP_REG_INPUT_ADDR_YPL                  (ISP_REG_BASE + 0x001C)
#define ISP_REG_INPUT_ADDR_YPH                  (ISP_REG_BASE + 0x001E)

#define ISP_REG_INPUT_PITCH_Y                   (ISP_REG_BASE + 0x0028)
#define ISP_REG_INPUT_PITCH_UV                  (ISP_REG_BASE + 0x002A)
#define ISP_REG_INPUT_HEIGHT                    (ISP_REG_BASE + 0x002C)
#define ISP_REG_INPUT_WIDTH                     (ISP_REG_BASE + 0x002E)

#define ISP_BIT_INPUT_ADDR_L                    0xFFFF //1111 1111 1111 1111 15:0
#define ISP_BIT_INPUT_ADDR_H                    0xFFFF //1111 1111 1111 1111 15:0
#define ISP_BIT_INPUT_PITCH_Y                   0x7FFF //0111 1111 1111 1111 14:0
#define ISP_BIT_INPUT_PITCH_UV                  0x7FFF //0111 1111 1111 1111 14:0
#define ISP_BIT_INPUT_HEIGHT                    0x1FFF //0001 1111 1111 1111 12:0
#define ISP_BIT_INPUT_WIDTH                     0x1FFF //0001 1111 1111 1111 12:0

//================================================================
// 0x0538 ~ 0x053E
// Scaling factor
//================================================================
#define ISP_REG_SCALE_HCI_L                     (ISP_REG_BASE + 0x0038)// HCI
#define ISP_REG_SCALE_HCI_H                     (ISP_REG_BASE + 0x003A)// HCI
#define ISP_REG_SCALE_VCI_L                     (ISP_REG_BASE + 0x003C)// VCI
#define ISP_REG_SCALE_VCI_H                     (ISP_REG_BASE + 0x003E)// VCI

#define ISP_BIT_SCALE_L                         0xFFFF //1111 1111 1111 1111 15:0
#define ISP_BIT_SCALE_H                         0x000F //0000 0000 0000 1111 3:0

//================================================================
// 0x0542 ~ 0x0558
// Transfer matrix for YUV to RGB
//================================================================
#define ISP_REG_YUV_TO_RGB_11                   (ISP_REG_BASE + 0x0042)
#define ISP_REG_YUV_TO_RGB_12                   (ISP_REG_BASE + 0x0044)
#define ISP_REG_YUV_TO_RGB_13                   (ISP_REG_BASE + 0x0046)
#define ISP_REG_YUV_TO_RGB_21                   (ISP_REG_BASE + 0x0048)
#define ISP_REG_YUV_TO_RGB_22                   (ISP_REG_BASE + 0x004A)
#define ISP_REG_YUV_TO_RGB_23                   (ISP_REG_BASE + 0x004C)
#define ISP_REG_YUV_TO_RGB_31                   (ISP_REG_BASE + 0x004E)
#define ISP_REG_YUV_TO_RGB_32                   (ISP_REG_BASE + 0x0050)
#define ISP_REG_YUV_TO_RGB_33                   (ISP_REG_BASE + 0x0052)
#define ISP_REG_YUV_TO_RGB_CONST_R              (ISP_REG_BASE + 0x0054)
#define ISP_REG_YUV_TO_RGB_CONST_G              (ISP_REG_BASE + 0x0056)
#define ISP_REG_YUV_TO_RGB_CONST_B              (ISP_REG_BASE + 0x0058)

#define ISP_BIT_YUV_TO_RGB                      0x07FF //0000 0111 1111 1111 10:0
#define ISP_BIT_YUV_TO_RGB_CONST                0x03FF //0000 0011 1111 1111 9:0

//================================================================
// 0x055A ~ 0x0570
// Color correction matrix & delta R/G/B
//================================================================
#define ISP_REG_COL_COR_11                      (ISP_REG_BASE + 0x005A)
#define ISP_REG_COL_COR_12                      (ISP_REG_BASE + 0x005C)
#define ISP_REG_COL_COR_13                      (ISP_REG_BASE + 0x005E)
#define ISP_REG_COL_COR_21                      (ISP_REG_BASE + 0x0060)
#define ISP_REG_COL_COR_22                      (ISP_REG_BASE + 0x0062)
#define ISP_REG_COL_COR_23                      (ISP_REG_BASE + 0x0064)
#define ISP_REG_COL_COR_31                      (ISP_REG_BASE + 0x0066)
#define ISP_REG_COL_COR_32                      (ISP_REG_BASE + 0x0068)
#define ISP_REG_COL_COR_33                      (ISP_REG_BASE + 0x006A)

#define ISP_REG_COL_COR_DELTA_R                 (ISP_REG_BASE + 0x006C)
#define ISP_REG_COL_COR_DELTA_G                 (ISP_REG_BASE + 0x006E)
#define ISP_REG_COL_COR_DELTA_B                 (ISP_REG_BASE + 0x0070)

#define ISP_BIT_COL_COR                         0x1FFF //0001 1111 1111 1111 12:0
#define ISP_BIT_COL_CORR_DELTA                  0x01FF //0000 0001 1111 1111 8:0

//================================================================
// 0x05A2 ~ 0x05B2
// Output Address, Width, Height and Pitch
//================================================================
#define ISP_REG_OUT_ADDR_0L                     (ISP_REG_BASE + 0x00A2)
#define ISP_REG_OUT_ADDR_0H                     (ISP_REG_BASE + 0x00A4)

#define ISP_REG_OUT_WIDTH                       (ISP_REG_BASE + 0x00AE)
#define ISP_REG_OUT_HEIGHT                      (ISP_REG_BASE + 0x00B0)
#define ISP_REG_OUT_PITCH                       (ISP_REG_BASE + 0x00B2)

#define ISP_BIT_OUT_ADDR_L                      0xFFFF //1111 1111 1111 1111 15:0
#define ISP_BIT_OUT_ADDR_H                      0xFFFF //1111 1111 1111 1111 15:0
#define ISP_BIT_OUT_WIDTH                       0x3FFF //0011 1111 1111 1111 13:0
#define ISP_BIT_OUT_HEIGHT                      0x3FFF //0011 1111 1111 1111 13:0
#define ISP_BIT_OUT_PITCH                       0x7FFF //0111 1111 1111 1111 14:0


//================================================================
// 0x05B6 ~ 0x05BA
// 3D Deinterlace Parm
//================================================================
#define ISP_REG_3D_DEINTER_PARM_1               (ISP_REG_BASE + 0x00B6)
#define ISP_REG_3D_DEINTER_PARM_2               (ISP_REG_BASE + 0x00B8)
#define ISP_REG_3D_DEINTER_PARM_3               (ISP_REG_BASE + 0x00BA)

#define ISP_BIT_3D_MDTHRED_HIGH                 0x00FF //1111 1111 0000 0000
#define ISP_BIT_3D_MDTHRED_LOW                  0x00FF //0000 0000 1111 1111

#define ISP_BIT_DISABLE_MOTIONVALUE_A           0x0001 //1000 0000 0000 0000
#define ISP_BIT_DISABLE_MOTIONVALUE_B           0x0001 //0100 0000 0000 0000
#define ISP_BIT_DISABLE_MOTIONVALUE_C           0x0001 //0010 0000 0000 0000
#define ISP_BIT_DISABLE_MOTIONVALUE_D           0x0001 //0001 0000 0000 0000
#define ISP_BIT_DISABLE_MOTIONVALUE_E           0x0001 //0000 1000 0000 0000
#define ISP_BIT_DISABLE_MOTIONVALUE_F           0x0001 //0000 0100 0000 0000
#define ISP_BIT_DISABLE_MOTIONVALUE_G           0x0001 //0000 0010 0000 0000
#define ISP_BIT_LPF_WEIGHT_EN                   0x0001 //0000 0001 0000 0000
#define ISP_BIT_LPF_BLEND_EN                    0x0001 //0000 0000 1000 0000
#define ISP_BIT_3D_MDTHRED_STEP                 0x007F //0000 0000 0111 1111

#define ISP_BIT_LPF_STATICPIXEL_EN              0x0001 //0000 0000 0000 0001

#define ISP_SHT_3D_MDTHRED_HIGH                 8
#define ISP_SHT_3D_MDTHRED_LOW                  0

#define ISP_SHT_DISABLE_MOTIONVALUE_A           15
#define ISP_SHT_DISABLE_MOTIONVALUE_B           14
#define ISP_SHT_DISABLE_MOTIONVALUE_C           13
#define ISP_SHT_DISABLE_MOTIONVALUE_D           12
#define ISP_SHT_DISABLE_MOTIONVALUE_E           11
#define ISP_SHT_DISABLE_MOTIONVALUE_F           10
#define ISP_SHT_DISABLE_MOTIONVALUE_G           9
#define ISP_SHT_LPF_WEIGHT_EN                   8
#define ISP_SHT_LPF_BLEND_EN                    7
#define ISP_SHT_3D_MDTHRED_STEP                 0

#define ISP_SHT_LPF_STATICPIXEL_EN              0

//================================================================
// 0x05BC
// 2D Deinterlace Parm
//================================================================
#define ISP_REG_2D_DEINTER_PARM_1               (ISP_REG_BASE + 0x00BC)

#define ISP_BIT_2D_EDGE_WEIGHT                  0x003F //0011 1111 1000 0000
#define ISP_BIT_2D_ORG_WEIGHT                   0x007F //0000 0000 0111 1111

#define ISP_SHT_2D_EDGE_WEIGHT                  7
#define ISP_SHT_2D_ORG_WEIGHT                   0

//================================================================
// 0x05C0 ~ 0x05CA
// Scale Background function
//================================================================
#define ISP_REG_BGCOLOR_RG                      (ISP_REG_BASE + 0x00C0)
#define ISP_REG_BGCOLOR_B                       (ISP_REG_BASE + 0x00C2)

#define ISP_REG_SCALE_DSTPOS_X                  (ISP_REG_BASE + 0x00C4)
#define ISP_REG_SCALE_DSTPOS_Y                  (ISP_REG_BASE + 0x00C6)
#define ISP_REG_SCALE_DSTWIDTH                  (ISP_REG_BASE + 0x00C8)
#define ISP_REG_SCALE_DSTHEIGHT                 (ISP_REG_BASE + 0x00CA)

#define ISP_BIT_BGCOLOR                         0x00FF //0000 0000 1111 1111
#define ISP_BIT_SCALE_DSTPOS_X                  0x3FFF //0011 1111 1111 1111 13:0
#define ISP_BIT_SCALE_DSTPOS_Y                  0x3FFF //0011 1111 1111 1111 13:0
#define ISP_BIT_SCALE_DSTWIDTH                  0x3FFF //0011 1111 1111 1111 13:0
#define ISP_BIT_SCALE_DSTHEIGHT                 0x3FFF //0011 1111 1111 1111 13:0

#define ISP_SHT_BGCOLOR_R                       0
#define ISP_SHT_BGCOLOR_G                       8

//================================================================
// 0x05D0 ~ 0x05E4
// Frame function
//================================================================
#define ISP_REG_SET_FRMFUN_0                    (ISP_REG_BASE + 0x00D0)
#define ISP_REG_CONST_ALPHA_0                   (ISP_REG_BASE + 0x00D2)
#define ISP_REG_FRMFUN_0_KEY_RG                 (ISP_REG_BASE + 0x00D4)
#define ISP_REG_FRMFUN_0_KEY_B                  (ISP_REG_BASE + 0x00D6)

#define ISP_REG_FRMFUN_0_ADDR_L                 (ISP_REG_BASE + 0x00D8)
#define ISP_REG_FRMFUN_0_ADDR_H                 (ISP_REG_BASE + 0x00DA)
#define ISP_REG_FRMFUN_0_START_X                (ISP_REG_BASE + 0x00DC)
#define ISP_REG_FRMFUN_0_START_Y                (ISP_REG_BASE + 0x00DE)
#define ISP_REG_FRMFUN_0_WIDTH                  (ISP_REG_BASE + 0x00E0)
#define ISP_REG_FRMFUN_0_HEIGHT                 (ISP_REG_BASE + 0x00E2)
#define ISP_REG_FRMFUN_0_PITCH                  (ISP_REG_BASE + 0x00E4)

#define ISP_BIT_FRMFUN_MODE                     0x0003 //0000 0000 0000 1100
#define ISP_BIT_FRMFUN_ALPHA_BLEND_EN           0x0001 //0000 0000 0000 0010
#define ISP_BIT_FRMFUN_EN                       0x0001 //0000 0000 0000 0001
#define ISP_BIT_CONST_ALPHA                     0x00FF //0000 0000 1111 1111
#define ISP_BIT_FRMFUN_KEY                      0x00FF //0000 0000 1111 1111

#define ISP_BIT_FRMFUN_ADDR_L                   0xFFFF //1111 1111 1111 1111 15:0
#define ISP_BIT_FRMFUN_ADDR_H                   0x03FF //0000 0011 1111 1111 9:0
#define ISP_BIT_FRMFUN_START_X                  0x07FF //0000 0111 1111 1111 10:0
#define ISP_BIT_FRMFUN_START_Y                  0x07FF //0000 0111 1111 1111 10:0
#define ISP_BIT_FRMFUN_WIDTH                    0x07FF //0000 0111 1111 1111 10:0
#define ISP_BIT_FRMFUN_HEIGHT                   0x07FF //0000 0111 1111 1111 10:0
#define ISP_BIT_FRMFUN_PITCH                    0x7FFF //0111 1111 1111 1111 14:0

#define ISP_SHT_FRMFUN_MODE                     2
#define ISP_SHT_FRMFUN_ALPHA_BLEND_EN           1
#define ISP_SHT_FRMFUN_EN                       0

#define ISP_SHT_FRMFUN_KEY_R                    0
#define ISP_SHT_FRMFUN_KEY_G                    8

//================================================================
// 0x063C ~ 0x0654
// PreScale function
//================================================================
#define ISP_REG_PRESCALE_HCI_L                  (ISP_REG_BASE + 0x013C)
#define ISP_REG_PRESCALE_HCI_H                  (ISP_REG_BASE + 0x013E)
#define ISP_REG_PRESCALE_WIDTH                  (ISP_REG_BASE + 0x0140)

#define ISP_REG_PRESCALE_WX0100                 (ISP_REG_BASE + 0x0142)
#define ISP_REG_PRESCALE_WX0302                 (ISP_REG_BASE + 0x0144)
#define ISP_REG_PRESCALE_WX1110                 (ISP_REG_BASE + 0x0146)
#define ISP_REG_PRESCALE_WX1312                 (ISP_REG_BASE + 0x0148)
#define ISP_REG_PRESCALE_WX2120                 (ISP_REG_BASE + 0x014A)
#define ISP_REG_PRESCALE_WX2322                 (ISP_REG_BASE + 0x014C)
#define ISP_REG_PRESCALE_WX3130                 (ISP_REG_BASE + 0x014E)
#define ISP_REG_PRESCALE_WX3332                 (ISP_REG_BASE + 0x0150)
#define ISP_REG_PRESCALE_WX4140                 (ISP_REG_BASE + 0x0152)
#define ISP_REG_PRESCALE_WX4342                 (ISP_REG_BASE + 0x0154)

#define ISP_BIT_PRESCALE_HCI_L                  0xFFFF //1111 1111 1111 1111
#define ISP_BIT_PRESCALE_HCI_H                  0x000F //0000 0000 0000 1111
#define ISP_BIT_PRESCALE_WIDTH                  0x1FFF //0001 1111 1111 1111 12:0
#define ISP_BIT_PRESCALEWEIGHT                  0x00FF //0000 0000 1111 1111

#define ISP_SHT_PRESCALEWEIGHT_H                8
#define ISP_SHT_PRESCALEWEIGHT_L                0

//================================================================
// 0x0656 ~ 0x065A
// LowLevelEdge
//================================================================
#define ISP_REG_CC_IN_OFFSET_R                  (ISP_REG_BASE + 0x0156)
#define ISP_REG_CC_IN_OFFSET_G                  (ISP_REG_BASE + 0x0158)
#define ISP_REG_CC_IN_OFFSET_B                  (ISP_REG_BASE + 0x015A)

#define ISP_BIT_IN_OFFSET                       0x01FF //0000 0001 1111 1111

//================================================================
// 0x065C ~ 0x0662
// LowLevelEdge
//================================================================
#define ISP_REG_LOWLEVELEDGE_START_X            (ISP_REG_BASE + 0x015C)
#define ISP_REG_LOWLEVELEDGE_START_Y            (ISP_REG_BASE + 0x015E)
#define ISP_REG_LOWLEVELEDGE_WIDTH              (ISP_REG_BASE + 0x0160)
#define ISP_REG_LOWLEVELEDGE_HEIGHT             (ISP_REG_BASE + 0x0162)

//================================================================
// 0x0664 ~ 0x066E
// Run-Length Encoder
//================================================================
#define ISP_REG_RUNLENENC_ADDR_L                (ISP_REG_BASE + 0x0164)
#define ISP_REG_RUNLENENC_ADDR_H                (ISP_REG_BASE + 0x0166)
#define ISP_REG_RUNLENENC_LINEBYTE              (ISP_REG_BASE + 0x0168)
#define ISP_REG_RUNLENENC_PITCH                 (ISP_REG_BASE + 0x016A)
#define ISP_REG_RUNLENENC_MAXBIT_L              (ISP_REG_BASE + 0x016C)
#define ISP_REG_RUNLENENC_MAXBIT_H              (ISP_REG_BASE + 0x016E)

#define ISP_BIT_RUNLENENC_ADDR_L                0xFFFF //1111 1111 1111 1111 15:0
#define ISP_BIT_RUNLENENC_ADDR_H                0xFFFF //1111 1111 1111 1111 15:0
#define ISP_BIT_RUNLENENC_MAXBIT_L              0xFFFF //1111 1111 1111 1111 15:0
#define ISP_BIT_RUNLENENC_MAXBIT_H              0xFFFF //1111 1111 1111 1111 15:0

//================================================================
// 0x06A0 ~ 0x06BC
// Remap luma address
//================================================================
#define ISP_REG_MAPADR_Y_0403                   (ISP_REG_BASE + 0x01A0)
#define ISP_REG_MAPADR_Y_0605                   (ISP_REG_BASE + 0x01A2)
#define ISP_REG_MAPADR_Y_0807                   (ISP_REG_BASE + 0x01A4)
#define ISP_REG_MAPADR_Y_1009                   (ISP_REG_BASE + 0x01A6)
#define ISP_REG_MAPADR_Y_1211                   (ISP_REG_BASE + 0x01A8)
#define ISP_REG_MAPADR_Y_1413                   (ISP_REG_BASE + 0x01AA)
#define ISP_REG_MAPADR_Y_1615                   (ISP_REG_BASE + 0x01AC)
#define ISP_REG_MAPADR_Y_1817                   (ISP_REG_BASE + 0x01AE)
#define ISP_REG_MAPADR_Y_2019                   (ISP_REG_BASE + 0x01B0)
#define ISP_REG_MAPADR_Y_2221                   (ISP_REG_BASE + 0x01B2)
#define ISP_REG_MAPADR_Y_2423                   (ISP_REG_BASE + 0x01B4)
#define ISP_REG_MAPADR_Y_2625                   (ISP_REG_BASE + 0x01B6)
#define ISP_REG_MAPADR_Y_2827                   (ISP_REG_BASE + 0x01B8)
#define ISP_REG_MAPADR_Y_3029                   (ISP_REG_BASE + 0x01BA)
#define ISP_REG_MAPADR_Y_XX31                   (ISP_REG_BASE + 0x01BC)

#define ISP_BIT_MAPADDR_Y                       0x3F3F //0011 1111 0011 1111

//================================================================
// 0x06C0 ~ 0x06DC
// Remap chroma address
//================================================================
#define ISP_REG_MAPADR_UV_0403                  (ISP_REG_BASE + 0x01C0)
#define ISP_REG_MAPADR_UV_0605                  (ISP_REG_BASE + 0x01C2)
#define ISP_REG_MAPADR_UV_0807                  (ISP_REG_BASE + 0x01C4)
#define ISP_REG_MAPADR_UV_1009                  (ISP_REG_BASE + 0x01C6)
#define ISP_REG_MAPADR_UV_1211                  (ISP_REG_BASE + 0x01C8)
#define ISP_REG_MAPADR_UV_1413                  (ISP_REG_BASE + 0x01CA)
#define ISP_REG_MAPADR_UV_1615                  (ISP_REG_BASE + 0x01CC)
#define ISP_REG_MAPADR_UV_1817                  (ISP_REG_BASE + 0x01CE)
#define ISP_REG_MAPADR_UV_2019                  (ISP_REG_BASE + 0x01D0)
#define ISP_REG_MAPADR_UV_2221                  (ISP_REG_BASE + 0x01D2)
#define ISP_REG_MAPADR_UV_2423                  (ISP_REG_BASE + 0x01D4)
#define ISP_REG_MAPADR_UV_2625                  (ISP_REG_BASE + 0x01D6)
#define ISP_REG_MAPADR_UV_2827                  (ISP_REG_BASE + 0x01D8)
#define ISP_REG_MAPADR_UV_3029                  (ISP_REG_BASE + 0x01DA)
#define ISP_REG_MAPADR_UV_XX31                  (ISP_REG_BASE + 0x01DC)

#define ISP_BIT_MAPADDR_UV                      0x3F3F //0011 1111 0011 1111

//================================================================
// 0x06FC
// ISP engine status
//================================================================
#define ISP_REG_ISP_ENGINE_STATUS               (ISP_REG_BASE + 0x01FC)

//================================================================
// 0x06FE
// ISP engine status 2
//================================================================
#define ISP_REG_ISP_ENGINE_STATUS_2             (ISP_REG_BASE + 0x01FE)

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