#ifndef ITE_ITH_LCD_H
#define ITE_ITH_LCD_H

#include "ite/ith.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup ith_lcd LCD
 *  @{
 */
#define REG_LCD_BASE                   (0x1100) /* Base Register Address */
#define REG_LCD(offset)                (REG_LCD_BASE + (offset))

#define ITH_LCD_SET1_REG                0x1100
#define ITH_LCD_LAYER_SYNC_RQ_BIT       13
#define ITH_LCD_ON_FLY_EN_BIT           11
#define ITH_LCD_HW_FLIP_BIT             10
#define ITH_LCD_VIDEO_FLIP_EN_BIT       9
#define ITH_LCD_VIDEO_FLIP_MODE_BIT     8
#define ITH_LCD_FLIP_BUF_CTRL_BIT       7
#define ITH_LCD_SW_FLIP_MODE_BIT        5
#define ITH_LCD_HSYNC_FLIP_BIT          4

#define ITH_LCD_SET_MODE_REG            0x1102
#define ITH_LCD_ROT_MODE_BIT            14
#define ITH_LCD_ROT_MODE_MASK           (N02_BITS_MSK << ITH_LCD_ROT_MODE_BIT)
#define ITH_LCD_CCIR_MODE_BIT           13
#define ITH_LCD_DISPLAY_MODE_BIT        0
#define ITH_LCD_DISPLAY_MODE_MASK       (N12_BITS_MSK << ITH_LCD_DISPLAY_MODE_BIT)

#define ITH_LCD_SWFLIPNUM_REG           0x1106
#define ITH_LCD_UI_DEC_FLIP_BIT         4
#define ITH_LCD_UI_DEC_FLIP_MASK        (N02_BITS_MSK << ITH_LCD_UI_DEC_FLIP_BIT)
#define ITH_LCD_SWFLIPNUM_BIT           0
#define ITH_LCD_SWFLIPNUM_MASK          (N02_BITS_MSK << ITH_LCD_SWFLIPNUM_BIT)

#define ITH_LCD_SRCFMT_REG              0x110C
#define ITH_LCD_SRCFMT_BIT              12
#define ITH_LCD_SRCFMT_MASK             (N03_BITS_MSK << ITH_LCD_SRCFMT_BIT)

#define ITH_LCD_BASEB_LO_REG            0x1116
#define ITH_LCD_BASEB_LO_BIT            0
#define ITH_LCD_BASEB_LO_MASK           (0xFFF8 << ITH_LCD_BASEB_LO_BIT)

#define ITH_LCD_BASEB_HI_REG            0x1118
#define ITH_LCD_BASEB_HI_BIT            0
#define ITH_LCD_BASEB_HI_MASK           (0xFFFF << ITH_LCD_BASEB_HI_BIT)

#define ITH_LCD_BASEC_LO_REG            0x111A
#define ITH_LCD_BASEC_LO_BIT            0
#define ITH_LCD_BASEC_LO_MASK           (0xFFF8 << ITH_LCD_BASEC_LO_BIT)

#define ITH_LCD_BASEC_HI_REG            0x111C
#define ITH_LCD_BASEC_HI_BIT            0
#define ITH_LCD_BASEC_HI_MASK           (0xFFFF << ITH_LCD_BASEC_HI_BIT)

#define ITH_LCD_UPDATE_REG              0x111E
#define ITH_LCD_LAYER1UPDATE_BIT        15
#define ITH_LCD_LAYER1UPDATE_MASK       (N01_BITS_MSK << ITH_LCD_LAYER1UPDATE_BIT)
#define ITH_LCD_DISPEN_BIT              1
#define ITH_LCD_DISPEN_MASK             (N01_BITS_MSK << ITH_LCD_DISPEN_BIT)
#define ITH_LCD_SYNCFIRE_BIT            0
#define ITH_LCD_SYNCFIRE_MASK           (N01_BITS_MSK << ITH_LCD_SYNCFIRE_BIT)

#define ITH_LCD_HWC_EN_REG             0x1120
#define ITH_LCD_HWC_EN_BIT             15

#define ITH_LCD_HWC_WIDTH_REG          0x1120
#define ITH_LCD_HWC_WIDTH_BIT          0
#define ITH_LCD_HWC_WIDTH_MASK         (N11_BITS_MSK << ITH_LCD_HWC_WIDTH_BIT)

#define ITH_LCD_HWC_HEIGHT_REG         0x1122
#define ITH_LCD_HWC_HEIGHT_BIT         0
#define ITH_LCD_HWC_HEIGHT_MASK        (N11_BITS_MSK << ITH_LCD_HWC_HEIGHT_BIT)

#define ITH_LCD_HWC_PITCH_REG          0x1124
#define ITH_LCD_HWC_PITCH_BIT          0
#define ITH_LCD_HWC_PITCH_MASK         (N13_BITS_MSK << ITH_LCD_HWC_PITCH_BIT)

#define ITH_LCD_HWC_POSX_REG           0x1126
#define ITH_LCD_HWC_POSX_BIT           0
#define ITH_LCD_HWC_POSX_MASK          (N11_BITS_MSK << ITH_LCD_HWC_POSX_BIT)

#define ITH_LCD_HWC_POSY_REG           0x1128
#define ITH_LCD_HWC_POSY_BIT           0
#define ITH_LCD_HWC_POSY_MASK          (N11_BITS_MSK << ITH_LCD_HWC_POSY_BIT)

#define ITH_LCD_HWC_BASE_LO_REG        0x112A
#define ITH_LCD_HWC_BASE_LO_BIT        0
#define ITH_LCD_HWC_BASE_LO_MASK       (0xFFF8 << ITH_LCD_HWC_BASE_LO_BIT)
#define ITH_LCD_HWC_BASE_HI_REG        0x112C
#define ITH_LCD_HWC_BASE_HI_BIT        0
#define ITH_LCD_HWC_BASE_HI_MASK       (0xFFFF << ITH_LCD_HWC_BASE_HI_BIT)

#define ITH_LCD_HWC_CR_REG             0x112E
#define ITH_LCD_HWC_ABLDEN_BIT         15
#define ITH_LCD_HWC_DEFDSTEN_BIT       14
#define ITH_LCD_HWC_DEFINVDST_BIT      13
#define ITH_LCD_HWC_INVCOLORWEI_REG    0x112E
#define ITH_LCD_HWC_INVCOLORWEI_BIT    0
#define ITH_LCD_HWC_INVCOLORWEI_MASK   (N07_BITS_MSK << ITH_LCD_HWC_INVCOLORWEI_BIT)

#define ITH_LCD_HWC_DEFCOLOR_REG       0x1130
#define ITH_LCD_HWC_FORECOLOR_REG      0x1132
#define ITH_LCD_HWC_BACKCOLOR_REG      0x1134

#define ITH_LCD_HWC_FORECOLORWEI_REG   0x1136
#define ITH_LCD_HWC_FORECOLORWEI_BIT   8
#define ITH_LCD_HWC_FORECOLORWEI_MASK  (N08_BITS_MSK << ITH_LCD_HWC_FORECOLORWEI_BIT)

#define ITH_LCD_HWC_BACKCOLORWEI_REG   0x1136
#define ITH_LCD_HWC_BACKCOLORWEI_BIT   0
#define ITH_LCD_HWC_BACKCOLORWEI_MASK  (N08_BITS_MSK << ITH_LCD_HWC_BACKCOLORWEI_BIT)

#define ITH_LCD_HWC_FIFO_RQ_THD_REG    0x1138
#define ITH_LCD_HWC_FIFO_RQ_THD_BIT    12
#define ITH_LCD_HWC_FIFO_RQ_THD_MASK   (N03_BITS_MSK << ITH_LCD_HWC_FIFO_RQ_THD_BIT)

#define ITH_LCD_HWC_UPDATE_REG         0x113E
#define ITH_LCD_HWC_UPDATE_BIT         15

#define ITH_LCD_UI_DECPRESS_EN_REG     0x1140
#define ITH_LCD_UI_DECPRESS_EN_BIT     15
#define ITH_LCD_UI_DEC_LINEBYTE_REG    0x1140
#define ITH_LCD_UI_DEC_LINEBYTE_BIT    0
#define ITH_LCD_UI_DEC_LINEBYTE_MASK  (N15_BITS_MSK << ITH_LCD_UI_DEC_LINEBYTE_BIT)

#define ITH_LCD_UI_DEC_PITCH_REG       0x1142
#define ITH_LCD_UI_DEC_PITCH_BIT       0
#define ITH_LCD_UI_DEC_PITCH_MASK      (N15_BITS_MSK << ITH_LCD_UI_DEC_PITCH_BIT)

#define ITH_LCD_UI_DEC_TBYTE_LO_REG    0x1144
#define ITH_LCD_UI_DEC_TBYTE_HI_REG    0x1146

#define ITH_LCD_UI_DEC_UPDATE_REG      0x1148
#define ITH_LCD_UI_DEC_UPDATE_BIT      15

#define ITH_LCD_GAMMA_FUN_EN_REG       0x1160
#define ITH_LCD_GAMMA_FUN_EN_BIT       15
#define ITH_LCD_GAMMA_R_PTR_REG        0x1160
#define ITH_LCD_GAMMA_R_PTR_BIT        0
#define ITH_LCD_GAMMA_R_PTR_MASK       (N07_BITS_MSK << ITH_LCD_GAMMA_R_PTR_BIT)

#define ITH_LCD_GAMMA_R_VALUE_REG      0x1162
#define ITH_LCD_GAMMA_R_VALUE_BIT      0
#define ITH_LCD_GAMMA_R_VALUE_MASK     (N10_BITS_MSK << ITH_LCD_GAMMA_R_VALUE_BIT)

#define ITH_LCD_GAMMA_G_PTR_REG        0x1164
#define ITH_LCD_GAMMA_G_PTR_BIT        0
#define ITH_LCD_GAMMA_G_PTR_MASK       (N07_BITS_MSK << ITH_LCD_GAMMA_G_PTR_BIT)

#define ITH_LCD_GAMMA_G_VALUE_REG      0x1166
#define ITH_LCD_GAMMA_G_VALUE_BIT      0
#define ITH_LCD_GAMMA_G_VALUE_MASK     (N10_BITS_MSK << ITH_LCD_GAMMA_G_VALUE_BIT)

#define ITH_LCD_GAMMA_B_PTR_REG        0x1168
#define ITH_LCD_GAMMA_B_PTR_BIT        0
#define ITH_LCD_GAMMA_B_PTR_MASK       (N07_BITS_MSK << ITH_LCD_GAMMA_B_PTR_BIT)

#define ITH_LCD_GAMMA_B_VALUE_REG      0x116A
#define ITH_LCD_GAMMA_B_VALUE_BIT      0
#define ITH_LCD_GAMMA_B_VALUE_MASK     (N10_BITS_MSK << ITH_LCD_GAMMA_B_VALUE_BIT)

#define ITH_LCD_RGB2YUV_REG            0x1170
#define ITH_LCD_RGB2YUV_EN_BIT         15
#define ITH_LCD_YUV_PACKET_BIT         14
#define ITH_LCD_YUV_FORMAT_BIT         12
#define ITH_LCD_YUV_FORMAT_MASK        (N02_BITS_MSK << ITH_LCD_YUV_FORMAT_BIT)

#define ITH_LCD_RGB2YUV11_REG          0x1172
#define ITH_LCD_RGB2YUV11_BIT          0
#define ITH_LCD_RGB2YUV11_MASK         (N10_BITS_MSK << ITH_LCD_RGB2YUV11_BIT)

#define ITH_LCD_RGB2YUV12_REG          0x1174
#define ITH_LCD_RGB2YUV12_BIT          0
#define ITH_LCD_RGB2YUV12_MASK         (N10_BITS_MSK << ITH_LCD_RGB2YUV12_BIT)

#define ITH_LCD_RGB2YUV13_REG          0x1176
#define ITH_LCD_RGB2YUV13_BIT          0
#define ITH_LCD_RGB2YUV13_MASK         (N10_BITS_MSK << ITH_LCD_RGB2YUV13_BIT)

#define ITH_LCD_RGB2YUVC1_REG          0x1178
#define ITH_LCD_RGB2YUVC1_BIT          0
#define ITH_LCD_RGB2YUVC1_MASK         (N09_BITS_MSK << ITH_LCD_RGB2YUVC1_BIT)

#define ITH_LCD_RGB2YUV21_REG          0x117A
#define ITH_LCD_RGB2YUV21_BIT          0
#define ITH_LCD_RGB2YUV21_MASK         (N10_BITS_MSK << ITH_LCD_RGB2YUV21_BIT)

#define ITH_LCD_RGB2YUV22_REG          0x117C
#define ITH_LCD_RGB2YUV22_BIT          0
#define ITH_LCD_RGB2YUV22_MASK         (N10_BITS_MSK << ITH_LCD_RGB2YUV22_BIT)

#define ITH_LCD_RGB2YUV23_REG          0x117E
#define ITH_LCD_RGB2YUV23_BIT          0
#define ITH_LCD_RGB2YUV23_MASK         (N10_BITS_MSK << ITH_LCD_RGB2YUV23_BIT)

#define ITH_LCD_RGB2YUVC2_REG          0x1180
#define ITH_LCD_RGB2YUVC2_BIT          0
#define ITH_LCD_RGB2YUVC2_MASK         (N09_BITS_MSK << ITH_LCD_RGB2YUVC2_BIT)

#define ITH_LCD_RGB2YUV31_REG          0x1182
#define ITH_LCD_RGB2YUV31_BIT          0
#define ITH_LCD_RGB2YUV31_MASK         (N10_BITS_MSK << ITH_LCD_RGB2YUV31_BIT)

#define ITH_LCD_RGB2YUV32_REG          0x1184
#define ITH_LCD_RGB2YUV32_BIT          0
#define ITH_LCD_RGB2YUV32_MASK         (N10_BITS_MSK << ITH_LCD_RGB2YUV32_BIT)

#define ITH_LCD_RGB2YUV33_REG          0x1186
#define ITH_LCD_RGB2YUV33_BIT          0
#define ITH_LCD_RGB2YUV33_MASK         (N10_BITS_MSK << ITH_LCD_RGB2YUV33_BIT)

#define ITH_LCD_RGB2YUVC3_REG          0x1188
#define ITH_LCD_RGB2YUVC3_BIT          0
#define ITH_LCD_RGB2YUVC3_MASK         (N09_BITS_MSK << ITH_LCD_RGB2YUVC3_BIT)

#define ITH_LCD_TCON_CTG_REG           0x1190
#define ITH_LCD_CTG0_EN_BIT            0
#define ITH_LCD_CTG1_EN_BIT            1
#define ITH_LCD_CTG2_EN_BIT            2
#define ITH_LCD_CTG3_EN_BIT            3
#define ITH_LCD_CTG4_EN_BIT            4
#define ITH_LCD_CTG5_EN_BIT            5
#define ITH_LCD_CTG6_EN_BIT            6
#define ITH_LCD_CTG7_EN_BIT            7
#define ITH_LCD_CTG_RST_BIT            8

#define ITH_LCD_CTG_HTOTAL_REG         0x1192
#define ITH_LCD_CTG_HTOTAL_BIT         0
#define ITH_LCD_CTG_HTOTAL_MASK        (N12_BITS_MSK << ITH_LCD_CTG_HTOTAL_BIT)

#define ITH_LCD_CTG_VTOTAL_REG         0x1194
#define ITH_LCD_CTG_VTOTAL_BIT         0
#define ITH_LCD_CTG_VTOTAL_MASK        (N12_BITS_MSK << ITH_LCD_CTG_VTOTAL_BIT)
                                      
#define ITH_LCD_CTG0_SET_REG           0x11A0
#define ITH_LCD_CTG1_SET_REG           0x11B0  

#define ITH_LCD_CTG_P1_Y_REG           0x11A2
#define ITH_LCD_CTG0_P2_ACT_REG        0x11A4

#define ITH_LCD_CTG0_P2_Y_REG          0x11A6
#define ITH_LCD_CTG0_P3_ACT_REG        0x11A8
#define ITH_LCD_CTG0_P3_Y_REG          0x11AA

#define ITH_LCD_CTG0_P4_ACT_REG        0x11AC
#define ITH_LCD_CTG0_P4_Y_REG          0x11AE
#define ITH_LCD_CTG1_SET_REG           0x11B0

#define ITH_LCD_CTG1_P1_Y_REG          0x11B2
#define ITH_LCD_CTG1_P2_ACT_REG        0x11B4

#define ITH_LCD_CTG1_P2_Y_REG          0x11B6
#define ITH_LCD_CTG1_P3_ACT_REG        0x11B8
#define ITH_LCD_CTG1_P3_Y_REG          0x11BA
#define ITH_LCD_CTG1_P4_ACT_REG        0x11BC
#define ITH_LCD_CTG1_P4_Y_REG          0x11BE
#define ITH_LCD_CTG2_SET_REG           0x11C0
#define ITH_LCD_CTG2_P1_Y_REG          0x11C2
#define ITH_LCD_CTG2_P2_ACT_REG        0x11C4
#define ITH_LCD_CTG2_P2_Y_REG          0x11C6
#define ITH_LCD_CTG2_P3_ACT_REG        0x11C8
#define ITH_LCD_CTG2_P3_Y_REG          0x11CA
#define ITH_LCD_CTG2_P4_ACT_REG        0x11CC
#define ITH_LCD_CTG2_P4_Y_REG          0x11CE
#define ITH_LCD_CTG3_SET_REG           0x11D0
#define ITH_LCD_CTG3_P1_Y_REG          0x11D2
#define ITH_LCD_CTG3_P2_ACT_REG        0x11D4
#define ITH_LCD_CTG3_P2_Y_REG          0x11D6
#define ITH_LCD_CTG3_P3_ACT_REG        0x11D8
#define ITH_LCD_CTG3_P3_Y_REG          0x11DA
#define ITH_LCD_CTG3_P4_ACT_REG        0x11DC
#define ITH_LCD_CTG3_P4_Y_REG          0x11DE


#define ITH_LCD_CTGH_CNT_REG            0x1280
#define ITH_LCD_CTGH_CNT_BIT            0
#define ITH_LCD_CTGH_CNT_MASK           (N12_BITS_MSK << ITH_LCD_CTGH_CNT_BIT)

#define ITH_LCD_READ_STATUS1_REG        0x1280

#define ITH_LCD_FLIP_NUM_BIT            12
#define ITH_LCD_FLIP_NUM_MASK           (N02_BITS_MSK << ITH_LCD_FLIP_NUM_BIT)

#define ITH_LCD_CTGV_CNT_BIT            0
#define ITH_LCD_CTGV_CNT_MASK           (N12_BITS_MSK << ITH_LCD_CTGV_CNT_BIT)

#define ITH_LCD_X_CNT_REG               0x1284
#define ITH_LCD_X_CNT_BIT               0
#define ITH_LCD_X_CNT_MASK              (N12_BITS_MSK << ITH_LCD_X_CNT_BIT)

#define ITH_LCD_INT_CLR_REG             0x1262
#define ITH_LCD_INT_CLR_BIT             15
#define ITH_LCD_INT_CLR_MASK            (N01_BITS_MSK << ITH_LCD_INT_CLR_BIT)

#define ITH_LCD_INT_CTRL_REG            0x1262//0x12FA
#define ITH_LCD_INT_EN_BIT              13
#define ITH_LCD_INT_FIELDMODE1_BIT      12

#define ITH_LCD_INT_LINE1_REG           0x1262//0x12FC
#define ITH_LCD_INT_LINE1_BIT           0
#define ITH_LCD_INT_LINE1_MASK          (N12_BITS_MSK << ITH_LCD_INT_LINE1_BIT)

#define ITH_LCD_IO_EN_REG               0x1258
#define ITH_LCD_IO_EN_BIT               12


/** @} */ // end of ith_lcd

#ifdef __cplusplus
}
#endif

#endif
