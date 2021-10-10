#ifndef __CAP_REG_H__
#define __CAP_REG_H__

#include "ite/ith_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#define REG_CAP_BASE                    (0x2000) /* Base Register Address */

//====================================================================
/*
 * 1. 0x2000
 *    General Setting Register
 */
//====================================================================
#define CAP_REG_SET01                   (REG_CAP_BASE + 0x0000)

#define CAP_SHT_TOPFIELDPOL             13
#define CAP_SHT_INTERLEAVEMODE          12
#define CAP_SHT_YUVFORMAT               8
#define CAP_SHT_TRIBUF                  7
#define CAP_SHT_VSYNCPOL                6
#define CAP_SHT_HSYNCPOL                5
#define CAP_SHT_DEMODE                  4
#define CAP_SHT_CCIRMODE                1

#define CAP_MSK_TOPFIELDPOL             (N01_BITS_MSK << CAP_SHT_TOPFIELDPOL)           // 0010 0000 0000 0000
#define CAP_MSK_INTERLEAVEMODE          (N01_BITS_MSK << CAP_SHT_INTERLEAVEMODE)        // 0001 0000 0000 0000
#define CAP_MSK_YUVFORMAT               (N02_BITS_MSK << CAP_SHT_YUVFORMAT)             // 0000 0011 0000 0000
#define CAP_MSK_TRIBUF                  (N01_BITS_MSK << CAP_SHT_TRIBUF)                // 0000 0000 1000 0000
#define CAP_MSK_VSYNCPOL                (N01_BITS_MSK << CAP_SHT_VSYNCPOL)              // 0000 0000 0100 0000
#define CAP_MSK_HSYNCPOL                (N01_BITS_MSK << CAP_SHT_HSYNCPOL)              // 0000 0000 0010 0000
#define CAP_MSK_DEMODE                  (N01_BITS_MSK << CAP_SHT_DEMODE)                // 0000 0000 0001 0000
#define CAP_MSK_CCIRMODE                (N01_BITS_MSK << CAP_SHT_CCIRMODE)              // 0000 0000 0000 0010

//====================================================================
/*
 * 0x2002 ~ 0x2008
 *    General Setting Register
 */
//====================================================================
#define CAP_REG_WIDTH                   (REG_CAP_BASE + 0x0002)
#define CAP_REG_HEIGHT                  (REG_CAP_BASE + 0x0004)
#define CAP_REG_PITCHY                  (REG_CAP_BASE + 0x0006)
#define CAP_REG_PITCHUV                 (REG_CAP_BASE + 0x0008)

#define CAP_MSK_WIDTH                   N11_BITS_MSK
#define CAP_MSK_HEIGHT                  N11_BITS_MSK
#define CAP_MSK_PITCHY                  N13_BITS_MSK
#define CAP_MSK_PITCHUV                 N13_BITS_MSK

//====================================================================
/*
 * 0x200A ~ 0x202C
 *    Output Bass Address Register
 */
//====================================================================
#define CAP_REG_Y0_BUF_ADDR_L           (REG_CAP_BASE + 0x000A)
#define CAP_REG_Y0_BUF_ADDR_H           (REG_CAP_BASE + 0x000C)
#define CAP_REG_Y1_BUF_ADDR_L           (REG_CAP_BASE + 0x000E)
#define CAP_REG_Y1_BUF_ADDR_H           (REG_CAP_BASE + 0x0010)
#define CAP_REG_Y2_BUF_ADDR_L           (REG_CAP_BASE + 0x0012)
#define CAP_REG_Y2_BUF_ADDR_H           (REG_CAP_BASE + 0x0014)

#define CAP_REG_U0_BUF_ADDR_L           (REG_CAP_BASE + 0x0016)
#define CAP_REG_U0_BUF_ADDR_H           (REG_CAP_BASE + 0x0018)
#define CAP_REG_U1_BUF_ADDR_L           (REG_CAP_BASE + 0x001A)
#define CAP_REG_U1_BUF_ADDR_H           (REG_CAP_BASE + 0x001C)
#define CAP_REG_U2_BUF_ADDR_L           (REG_CAP_BASE + 0x001E)
#define CAP_REG_U2_BUF_ADDR_H           (REG_CAP_BASE + 0x0020)

#define CAP_REG_V0_BUF_ADDR_L           (REG_CAP_BASE + 0x0022)
#define CAP_REG_V0_BUF_ADDR_H           (REG_CAP_BASE + 0x0024)
#define CAP_REG_V1_BUF_ADDR_L           (REG_CAP_BASE + 0x0026)
#define CAP_REG_V1_BUF_ADDR_H           (REG_CAP_BASE + 0x0028)
#define CAP_REG_V2_BUF_ADDR_L           (REG_CAP_BASE + 0x002A)
#define CAP_REG_V2_BUF_ADDR_H           (REG_CAP_BASE + 0x002C)

#define CAP_MSK_BUF_ADDR_L				N16_BITS_MSK
#define CAP_MSK_BUF_ADDR_H				N16_BITS_MSK

//====================================================================
/*
 * 0x202E ~ 0x2030
 *    HSync active area Register
 */
//====================================================================
#define CAP_REG_HSYNC_ACT_START_IDX     (REG_CAP_BASE + 0x002E)
#define CAP_REG_HSYNC_ACT_END_IDX       (REG_CAP_BASE + 0x0030)

#define CAP_MSK_HSYNC_ACT_IDX           N13_BITS_MSK

//====================================================================
/*
 * 0x2032 ~ 0x2038
 *    VSync active area Register
 */
//====================================================================
#define CAP_REG_VSYNC_ACT_TOP_START_IDX (REG_CAP_BASE + 0x0032)
#define CAP_REG_VSYNC_ACT_TOP_END_IDX   (REG_CAP_BASE + 0x0034)
#define CAP_REG_VSYNC_ACT_BTM_START_IDX (REG_CAP_BASE + 0x0036)
#define CAP_REG_VSYNC_ACT_BTM_END_IDX   (REG_CAP_BASE + 0x0038)

#define CAP_MSK_VSYNC_ACT_IDX           N12_BITS_MSK

//====================================================================
/*
 * 0x203A
 *    H/W Test Register 
 */
//====================================================================
#define CAP_REG_TESTHW					(REG_CAP_BASE + 0x003A)

#define CAP_SHT_WRTESTEN                15
#define CAP_SHT_WRTESTNUM               13
#define CAP_SHT_YBUFFERTHD              8
#define CAP_SHT_UBUFFERTHD              4
#define CAP_SHT_VBUFFERTHD              0

#define CAP_MSK_WRTESTEN                (N01_BITS_MSK << CAP_SHT_WRTESTEN)      // 1000 0000 0000 0000
#define CAP_MSK_WRTESTNUM               (N02_BITS_MSK << CAP_SHT_WRTESTNUM)     // 0110 0000 0000 0000
#define CAP_MSK_YBUFFERTHD              (N05_BITS_MSK << CAP_SHT_YBUFFERTHD)    // 0001 1111 0000 0000
#define CAP_MSK_UBUFFERTHD              (N04_BITS_MSK << CAP_SHT_UBUFFERTHD)    // 0000 0000 1111 0000
#define CAP_MSK_VBUFFERTHD              (N04_BITS_MSK << CAP_SHT_VBUFFERTHD)    // 0000 0000 0000 1111

//====================================================================
/*
 * 0x203E
 *    Capture Start Register
 */
//====================================================================
#define CAP_REG_START_CAP               (REG_CAP_BASE + 0x003E)

#define CAP_BIT_START_CAP               1
#define CAP_SHT_START_CAP               0
#define CAP_MSK_START_CAP               (N01_BITS_MSK << CAP_SHT_START_CAP)		// 0000 0000 0000 0001

//////////////////////////////////////////////////
// Only Used to Get H/W Status
//////////////////////////////////////////////////

//====================================================================
/*
 * 0x2040 ~ 0x204C
 *    H/W status Register
 */
//====================================================================
#define CAP_REG_DETECTED_HLINE_NUMS  (REG_CAP_BASE + 0x0040)
#define CAP_REG_DETECTED_VLINE_NUMS  (REG_CAP_BASE + 0x0042)
#define CAP_REG_DETECTED_DISP_WIDTH  (REG_CAP_BASE + 0x0044)
#define CAP_REG_DETECTED_DISP_HEIGHT (REG_CAP_BASE + 0x0046)

#define CAP_MSK_DETECTED_HLINE_NUMS  (N13_BITS_MSK)
#define CAP_MSK_DETECTED_VLINE_NUMS  (N12_BITS_MSK)
#define CAP_MSK_DETECTED_DISP_WIDTH  (N12_BITS_MSK)
#define CAP_MSK_DETECTED_DISP_HEIGHT (N12_BITS_MSK)

#define CAP_REG_BUF_WRITEIDX         (REG_CAP_BASE + 0x004C)
#define CAP_REG_INTERRUPT_FLAG       (REG_CAP_BASE + 0x004C)
#define CAP_SHT_INTERRUPT_FLAG       15
#define CAP_SHT_BUF_WRITEIDX         0
#define CAP_MSK_INTERRUPT_FLAG       (N01_BITS_MSK << CAP_SHT_INTERRUPT_FLAG)	// 1000 0000 0000 0000
#define CAP_MSK_BUF_WRITEIDX         (N02_BITS_MSK << CAP_SHT_BUF_WRITEIDX)		// 0000 0000 0000 0011

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
