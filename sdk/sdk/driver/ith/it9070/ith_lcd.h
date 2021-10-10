#ifndef ITE_ITH_LCD_H
#define ITE_ITH_LCD_H

#include "ite/ith.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup ith_lcd LCD
 *  @{
 */
#define ITH_LCD_SET1_REG                0x1100
#define ITH_LCD_FIRE_CTRL_BIT           14
#define ITH_LCD_LAYER_SYNC_RQ_BIT       13
#define ITH_LCD_ON_FLY_EN_BIT           11
#define ITH_LCD_HW_FLIP_BIT             10
#define ITH_LCD_VIDEO_FLIP_EN_BIT       9
#define ITH_LCD_VIDEO_FLIP_MODE_BIT     8
#define ITH_LCD_FLIP_BUF_CTRL_BIT       7
#define ITH_LCD_SW_FLIP_MODE_BIT        5
#define ITH_LCD_HSYNC_FLIP_BIT          4
#define ITH_LCD_CRC_EN_BIT              0

#define ITH_LCD_SWFLIPNUM_REG           0x1106
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

#define ITH_LCD_DHWC_EN_REG             0x1120
#define ITH_LCD_DHWC_EN_BIT             15

#define ITH_LCD_DHWC_WIDTH_REG          0x1120
#define ITH_LCD_DHWC_WIDTH_BIT          0
#define ITH_LCD_DHWC_WIDTH_MASK         (N12_BITS_MSK << ITH_LCD_DHWC_WIDTH_BIT)

#define ITH_LCD_DHWC_HEIGHT_REG         0x1122
#define ITH_LCD_DHWC_HEIGHT_BIT         0
#define ITH_LCD_DHWC_HEIGHT_MASK        (N12_BITS_MSK << ITH_LCD_DHWC_HEIGHT_BIT)

#define ITH_LCD_DHWC_PITCH_REG          0x1124
#define ITH_LCD_DHWC_PITCH_BIT          0
#define ITH_LCD_DHWC_PITCH_MASK         (N14_BITS_MSK << ITH_LCD_DHWC_PITCH_BIT)

#define ITH_LCD_DHWC_POSX_REG           0x1126
#define ITH_LCD_DHWC_POSX_BIT           0
#define ITH_LCD_DHWC_POSX_MASK          (N12_BITS_MSK << ITH_LCD_DHWC_POSX_BIT)

#define ITH_LCD_DHWC_POSY_REG           0x1128
#define ITH_LCD_DHWC_POSY_BIT           0
#define ITH_LCD_DHWC_POSY_MASK          (N12_BITS_MSK << ITH_LCD_DHWC_POSY_BIT)

#define ITH_LCD_DHWC_BASE_LO_REG        0x112A
#define ITH_LCD_DHWC_BASE_HI_REG        0x112C

#define ITH_LCD_DHWC_CR_REG             0x112E

#define ITH_LCD_DHWC_ABLDEN_BIT         15
#define ITH_LCD_DHWC_DEFDSTEN_BIT       14
#define ITH_LCD_DHWC_DEFINVDST_BIT      13

#define ITH_LCD_DHWC_INVCOLORWEI_REG    0x112E
#define ITH_LCD_DHWC_INVCOLORWEI_BIT    0
#define ITH_LCD_DHWC_INVCOLORWEI_MASK   (N08_BITS_MSK << ITH_LCD_DHWC_INVCOLORWEI_BIT)

#define ITH_LCD_DHWC_DEFCOLOR_REG       0x1130
#define ITH_LCD_DHWC_FORECOLOR_REG      0x1132
#define ITH_LCD_DHWC_BACKCOLOR_REG      0x1134

#define ITH_LCD_DHWC_FORECOLORWEI_REG   0x1136
#define ITH_LCD_DHWC_FORECOLORWEI_BIT   8
#define ITH_LCD_DHWC_FORECOLORWEI_MASK  (N08_BITS_MSK << ITH_LCD_DHWC_FORECOLORWEI_BIT)

#define ITH_LCD_DHWC_BACKCOLORWEI_REG   0x1136
#define ITH_LCD_DHWC_BACKCOLORWEI_BIT   0
#define ITH_LCD_DHWC_BACKCOLORWEI_MASK  (N08_BITS_MSK << ITH_LCD_DHWC_BACKCOLORWEI_BIT)

#define ITH_LCD_DHWC_UPDATE_REG         0x113E
#define ITH_LCD_DHWC_UPDATE_BIT         15

//=============================================================================
//                              lcd color conversion
//=============================================================================
#define ITH_LCD_COLORCON_OUT1_1         0x1170
#define ITH_LCD_COLORCON_OUT1_2         0x1172
#define ITH_LCD_COLORCON_OUT1_3         0x1174
#define ITH_LCD_COLORCON_OUT1_4         0x1176
#define ITH_LCD_COLORCON_OUT1_5         0x1178
#define ITH_LCD_COLORCON_OUT1_6         0x117A
#define ITH_LCD_COLORCON_OUT1_7         0x117C
#define ITH_LCD_COLORCON_OUT1_8         0x117E
#define ITH_LCD_COLORCON_OUT1_9         0x1180
#define ITH_LCD_COLORCON_OUT1_10        0x1182
#define ITH_LCD_COLORCON_OUT1_11        0x1184
#define ITH_LCD_COLORCON_OUT1_12        0x1186
#define ITH_LCD_COLORCON_OUT1_13        0x1188
#define ITH_LCD_COLORCON_OUT1_14        0x118A
#define ITH_LCD_COLORCON_OUT1_15        0x118C
#define ITH_LCD_COLORCON_OUT1_16        0x118E
#define ITH_LCD_COLORCON_OUT2_1         0x1190
#define ITH_LCD_COLORCON_OUT2_2         0x1192
#define ITH_LCD_COLORCON_OUT2_3         0x1194
#define ITH_LCD_COLORCON_OUT2_4         0x1196
#define ITH_LCD_COLORCON_OUT2_5         0x1198
#define ITH_LCD_COLORCON_OUT2_6         0x119A
#define ITH_LCD_COLORCON_OUT2_7         0x119C
#define ITH_LCD_COLORCON_OUT2_8         0x119E
#define ITH_LCD_COLORCON_OUT2_9         0x11A0
#define ITH_LCD_COLORCON_OUT2_10        0x11A2
#define ITH_LCD_COLORCON_OUT2_11        0x11A4
#define ITH_LCD_COLORCON_OUT2_12        0x11A6
#define ITH_LCD_COLORCON_OUT2_13        0x11A8

#define ITH_LCD_CTGH_CNT_REG            0x1280
#define ITH_LCD_CTGH_CNT_BIT            0
#define ITH_LCD_CTGH_CNT_MASK           (N13_BITS_MSK << ITH_LCD_CTGH_CNT_BIT)

#define ITH_LCD_READ_STATUS2_REG        0x1282

#define ITH_LCD_FLIP_NUM_BIT            12
#define ITH_LCD_FLIP_NUM_MASK           (N02_BITS_MSK << ITH_LCD_FLIP_NUM_BIT)

#define ITH_LCD_CTGV_CNT_BIT            0
#define ITH_LCD_CTGV_CNT_MASK           (N12_BITS_MSK << ITH_LCD_CTGV_CNT_BIT)

#define ITH_LCD_X_CNT_REG               0x1284
#define ITH_LCD_X_CNT_BIT               0
#define ITH_LCD_X_CNT_MASK              (N12_BITS_MSK << ITH_LCD_X_CNT_BIT)

#define ITH_LCD_INT_CLR_REG             0x129A
#define ITH_LCD_INT_CLR_BIT             15
#define ITH_LCD_INT_CLR_MASK            (N01_BITS_MSK << ITH_LCD_INT_CLR_BIT)

#define ITH_LCD_VDO_EBD_REG9            0x12F0
#define ITH_LCD_VDO_EBD_REG10           0x12F2
#define ITH_LCD_TST_SET_REG2            0x12F8
#define ITH_LCD_TST_PTN_EN_BIT              4
#define ITH_LCD_ONFLY_SYNC_ERR_BIT          1

#define ITH_LCD_INT_CTRL_REG            0x12FA
#define ITH_LCD_INT_EN_BIT              15
#define ITH_LCD_INT_OUTPUT2_BIT         3
#define ITH_LCD_INT_FIELDMODE2_BIT      2
#define ITH_LCD_INT_OUTPUT1_BIT         1
#define ITH_LCD_INT_FIELDMODE1_BIT      0

#define ITH_LCD_INT_LINE1_REG           0x12FC
#define ITH_LCD_INT_LINE1_BIT           0
#define ITH_LCD_INT_LINE1_MASK          (N12_BITS_MSK << ITH_LCD_INT_LINE1_BIT)

#define ITH_LCD_INT_LINE2_REG           0x12FE
#define ITH_LCD_INT_LINE2_BIT           0
#define ITH_LCD_INT_LINE2_MASK          (N12_BITS_MSK << ITH_LCD_INT_LINE2_BIT)

#define ITH_LCD_IO_EN_REG               0x1308
#define ITH_LCD_IO_EN_BIT               12

///

/** @} */ // end of ith_lcd

#ifdef __cplusplus
}
#endif

#endif
