#ifndef __CAP_REG_H__
#define __CAP_REG_H__

#include "ite/ith_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#define REG_CAP_BASE                    (ITH_CAP_BASE) /* Base Register Address */

//====================================================================
/*
 * 1. 0x0000
 *    General Setting Register
 */
//====================================================================
#define CAP_REG_SET01                   (REG_CAP_BASE + 0x0000)

#define CAP_SHT_DEEN					3
#define CAP_SHT_HSEN                 	2
#define CAP_SHT_INTERLEAVE              1
#define CAP_SHT_EMBEDDED                0

#define CAP_MSK_DEEN	                (N01_BITS_MSK << CAP_SHT_DEEN)	 	            // 0000 0000 0000 1000
#define CAP_MSK_HSEN                	(N01_BITS_MSK << CAP_SHT_HSEN)              	// 0000 0000 0000 0100
#define CAP_MSK_INTERLEAVE              (N01_BITS_MSK << CAP_SHT_INTERLEAVE)            // 0000 0000 0000 0010
#define CAP_MSK_EMBEDDED                (N01_BITS_MSK << CAP_SHT_EMBEDDED)              // 0000 0000 0000 0000

//====================================================================
/*
 * 0x0004 
 *    General Setting Register
 */
//====================================================================
#define CAP_REG_SET02                   (REG_CAP_BASE + 0x0004)

#define CAP_SHT_HEIGHT					16
#define CAP_SHT_WIDTH                 	0

#define CAP_MSK_HEIGHT	                (N13_BITS_MSK << CAP_SHT_HEIGHT)	 	            
#define CAP_MSK_WIDTH                	(N12_BITS_MSK << CAP_SHT_WIDTH)  

//====================================================================
/*
 * 0x0008 
 *    Active Region Setting Register 0
 */
//====================================================================
#define CAP_REG_HNUM1                   (REG_CAP_BASE + 0x0008)

#define CAP_MSK_HNUM1                	N13_BITS_MSK

//====================================================================
/*
 * 0x000C 	
 *    Active Region Setting Register 1
 */
//====================================================================
#define CAP_REG_VNUM1				    (REG_CAP_BASE + 0x000C)

#define CAP_SHT_VNUM2					16
#define CAP_SHT_VNUM1                 	0
#define CAP_MSK_VNUM2                	(N12_BITS_MSK << CAP_SHT_VNUM2)	 
#define CAP_MSK_VNUM1 			        (N12_BITS_MSK << CAP_SHT_VNUM1)

//====================================================================
/*
 * 0x0010
 *    Active Region Setting Register 2
 */
//====================================================================
#define CAP_REG_VNUM2				    (REG_CAP_BASE + 0x0010)

#define CAP_SHT_VNUM4					16
#define CAP_SHT_VNUM3                 	0
#define CAP_MSK_VNUM4                	(N12_BITS_MSK << CAP_SHT_VNUM4)	 
#define CAP_MSK_VNUM3 			        (N12_BITS_MSK << CAP_SHT_VNUM3)

//====================================================================
/*
 * 0x0014
 *    General Setting Register 
 */
//====================================================================
#define CAP_REG_SET_COLORFORMAT			(REG_CAP_BASE + 0x0014)

#define CAP_SHT_INPUTWIDTH              6
#define CAP_SHT_COLOR_DEPTH             4
#define CAP_SHT_COLOR_ORDER             2
#define CAP_SHT_COLORFORMAT             0

#define CAP_MSK_INPUTWIDTH              (N02_BITS_MSK << CAP_SHT_INPUTWIDTH)
#define CAP_MSK_COLOR_DEPTH             (N02_BITS_MSK << CAP_SHT_COLOR_DEPTH) 
#define CAP_MSK_COLOR_ORDER             (N02_BITS_MSK << CAP_SHT_COLOR_ORDER) 
#define CAP_MSK_COLORFORMAT             (N02_BITS_MSK << CAP_SHT_COLORFORMAT)   

//====================================================================
/*
 * 0x001C
 *    Clock Setting Register
 */
//====================================================================
#define CAP_REG_SET_CLKSRC              (REG_CAP_BASE + 0x001C)


#define CAP_SHT_UCLK_AUTO_DLYEN         31
#define CAP_SHT_UCLK_PINNUM             16
#define CAP_SHT_UCLK_INV                12
#define CAP_SHT_UCLK_DLY                8
#define CAP_SHT_UCLKEN                  3
#define CAP_SHT_UCLKSRC                 0

#define CAP_MSK_UCLK_AUTO_DLYEN         (N01_BITS_MSK << CAP_SHT_UCLK_AUTO_DLYEN)
#define CAP_MSK_UCLK_PINNUM             (N06_BITS_MSK << CAP_SHT_UCLK_PINNUM)
#define CAP_MSK_UCLK_INV                (N01_BITS_MSK << CAP_SHT_UCLK_INV)
#define CAP_MSK_UCLK_DLY                (N04_BITS_MSK << CAP_SHT_UCLK_DLY)
#define CAP_MSK_UCLKEN                 	(N01_BITS_MSK << CAP_SHT_UCLKEN)
#define CAP_MSK_UCLKSRC                 (N03_BITS_MSK << CAP_SHT_UCLKSRC)

//====================================================================
/*
 * 0x0020 ~ 0x0024
 *    General Setting Register
 */
//====================================================================
#define CAP_REG_CAPIOEn0               (REG_CAP_BASE + 0x0020)
#define CAP_REG_CAPIOEn1               (REG_CAP_BASE + 0x0024)

//====================================================================
/*
 * 0x0028
 *   Data Pin Mux 0
 */
//====================================================================
#define CAP_REG_DATA_PINNUM            (REG_CAP_BASE + 0x0028)

#define CAP_SHT_DEPINNUM   		        16
#define CAP_SHT_VSPINNUM                8
#define CAP_SHT_HSPINNUM                0

#define CAP_MSK_DEPINNUM                (N06_BITS_MSK << CAP_SHT_DEPINNUM)
#define CAP_MSK_VSPINNUM                (N06_BITS_MSK << CAP_SHT_VSPINNUM)
#define CAP_MSK_HSPINNUM                (N06_BITS_MSK << CAP_SHT_HSPINNUM)

//====================================================================
/*
 * 0x002C ~0x0034
 *    Y Data Pin Mux 0~2
 */
//====================================================================
#define CAP_REG_Y0_PINNUM               (REG_CAP_BASE + 0x002C)
#define CAP_REG_Y1_PINNUM               (REG_CAP_BASE + 0x0030)
#define CAP_REG_Y2_PINNUM               (REG_CAP_BASE + 0x0034)

#define CAP_SHT_Y11_PINNUM   		      24
#define CAP_SHT_Y10_PINNUM   		      16
#define CAP_SHT_Y09_PINNUM                8
#define CAP_SHT_Y08_PINNUM                0

#define CAP_SHT_Y07_PINNUM   		      24
#define CAP_SHT_Y06_PINNUM   		      16
#define CAP_SHT_Y05_PINNUM                8
#define CAP_SHT_Y04_PINNUM                0

#define CAP_SHT_Y03_PINNUM   		      24
#define CAP_SHT_Y02_PINNUM   		      16
#define CAP_SHT_Y01_PINNUM                8
#define CAP_SHT_Y00_PINNUM                0

#define CAP_MSK_Y11_PINNUM                (N06_BITS_MSK << CAP_SHT_Y11_PINNUM)
#define CAP_MSK_Y10_PINNUM                (N06_BITS_MSK << CAP_SHT_Y10_PINNUM)
#define CAP_MSK_Y09_PINNUM                (N06_BITS_MSK << CAP_SHT_Y09_PINNUM)
#define CAP_MSK_Y08_PINNUM                (N06_BITS_MSK << CAP_SHT_Y08_PINNUM)

#define CAP_MSK_Y07_PINNUM                (N06_BITS_MSK << CAP_SHT_Y07_PINNUM)
#define CAP_MSK_Y06_PINNUM                (N06_BITS_MSK << CAP_SHT_Y06_PINNUM)
#define CAP_MSK_Y05_PINNUM                (N06_BITS_MSK << CAP_SHT_Y05_PINNUM)
#define CAP_MSK_Y04_PINNUM                (N06_BITS_MSK << CAP_SHT_Y04_PINNUM)

#define CAP_MSK_Y03_PINNUM                (N06_BITS_MSK << CAP_SHT_Y03_PINNUM)
#define CAP_MSK_Y02_PINNUM                (N06_BITS_MSK << CAP_SHT_Y02_PINNUM)
#define CAP_MSK_Y01_PINNUM                (N06_BITS_MSK << CAP_SHT_Y01_PINNUM)
#define CAP_MSK_Y00_PINNUM                (N06_BITS_MSK << CAP_SHT_Y00_PINNUM)

//====================================================================
/*
 * 0x0038 ~0x0040
 *    U Data Pin Mux 0~2
 */
//====================================================================
#define CAP_REG_U0_PINNUM               (REG_CAP_BASE + 0x0038)
#define CAP_REG_U1_PINNUM               (REG_CAP_BASE + 0x003C)
#define CAP_REG_U2_PINNUM               (REG_CAP_BASE + 0x0040)

#define CAP_SHT_U11_PINNUM   		      24
#define CAP_SHT_U10_PINNUM   		      16
#define CAP_SHT_U09_PINNUM                8
#define CAP_SHT_U08_PINNUM                0

#define CAP_SHT_U07_PINNUM   		      24
#define CAP_SHT_U06_PINNUM   		      16
#define CAP_SHT_U05_PINNUM                8
#define CAP_SHT_U04_PINNUM                0

#define CAP_SHT_U03_PINNUM   		      24
#define CAP_SHT_U02_PINNUM   		      16
#define CAP_SHT_U01_PINNUM                8
#define CAP_SHT_U00_PINNUM                0

#define CAP_MSK_U11_PINNUM                (N06_BITS_MSK << CAP_SHT_U11_PINNUM)
#define CAP_MSK_U10_PINNUM                (N06_BITS_MSK << CAP_SHT_U10_PINNUM)
#define CAP_MSK_U09_PINNUM                (N06_BITS_MSK << CAP_SHT_U09_PINNUM)
#define CAP_MSK_U08_PINNUM                (N06_BITS_MSK << CAP_SHT_U08_PINNUM)

#define CAP_MSK_U07_PINNUM                (N06_BITS_MSK << CAP_SHT_U07_PINNUM)
#define CAP_MSK_U06_PINNUM                (N06_BITS_MSK << CAP_SHT_U06_PINNUM)
#define CAP_MSK_U05_PINNUM                (N06_BITS_MSK << CAP_SHT_U05_PINNUM)
#define CAP_MSK_U04_PINNUM                (N06_BITS_MSK << CAP_SHT_U04_PINNUM)

#define CAP_MSK_U03_PINNUM                (N06_BITS_MSK << CAP_SHT_U03_PINNUM)
#define CAP_MSK_U02_PINNUM                (N06_BITS_MSK << CAP_SHT_U02_PINNUM)
#define CAP_MSK_U01_PINNUM                (N06_BITS_MSK << CAP_SHT_U01_PINNUM)
#define CAP_MSK_U00_PINNUM                (N06_BITS_MSK << CAP_SHT_U00_PINNUM)

//====================================================================
/*
 * 0x0044 ~0x004C
 *    V Data Pin Mux 0~2
 */
//====================================================================
#define CAP_REG_V0_PINNUM               (REG_CAP_BASE + 0x0044)
#define CAP_REG_V1_PINNUM               (REG_CAP_BASE + 0x0048)
#define CAP_REG_V2_PINNUM               (REG_CAP_BASE + 0x004C)

#define CAP_SHT_V11_PINNUM   		      24
#define CAP_SHT_V10_PINNUM   		      16
#define CAP_SHT_V09_PINNUM                8
#define CAP_SHT_V08_PINNUM                0

#define CAP_SHT_V07_PINNUM   		      24
#define CAP_SHT_V06_PINNUM   		      16
#define CAP_SHT_V05_PINNUM                8
#define CAP_SHT_V04_PINNUM                0

#define CAP_SHT_V03_PINNUM   		      24
#define CAP_SHT_V02_PINNUM   		      16
#define CAP_SHT_V01_PINNUM                8
#define CAP_SHT_V00_PINNUM                0

#define CAP_MSK_V11_PINNUM                (N06_BITS_MSK << CAP_SHT_V11_PINNUM)
#define CAP_MSK_V10_PINNUM                (N06_BITS_MSK << CAP_SHT_V10_PINNUM)
#define CAP_MSK_V09_PINNUM                (N06_BITS_MSK << CAP_SHT_V09_PINNUM)
#define CAP_MSK_V08_PINNUM                (N06_BITS_MSK << CAP_SHT_V08_PINNUM)

#define CAP_MSK_V07_PINNUM                (N06_BITS_MSK << CAP_SHT_V07_PINNUM)
#define CAP_MSK_V06_PINNUM                (N06_BITS_MSK << CAP_SHT_V06_PINNUM)
#define CAP_MSK_V05_PINNUM                (N06_BITS_MSK << CAP_SHT_V05_PINNUM)
#define CAP_MSK_V04_PINNUM                (N06_BITS_MSK << CAP_SHT_V04_PINNUM)

#define CAP_MSK_V03_PINNUM                (N06_BITS_MSK << CAP_SHT_V03_PINNUM)
#define CAP_MSK_V02_PINNUM                (N06_BITS_MSK << CAP_SHT_V02_PINNUM)
#define CAP_MSK_V01_PINNUM                (N06_BITS_MSK << CAP_SHT_V01_PINNUM)
#define CAP_MSK_V00_PINNUM                (N06_BITS_MSK << CAP_SHT_V00_PINNUM)

//====================================================================
/*
 * 0x0050
 *    Capture Internal Setting Register
 */
//====================================================================
#define CAP_REG_INTERNAL_SETTING        (REG_CAP_BASE + 0x0050)

#define CAP_SHT_DUMPMODE         		31
#define CAP_SHT_CHECK_DE         		30
#define CAP_SHT_CHECK_VS         		29
#define CAP_SHT_CHECK_HS         		28
#define CAP_SHT_SAMPLE_VS         		27
#define CAP_SHT_HSYNC_SKIP         		12
#define CAP_SHT_VSYNC_SKIP              8
#define CAP_SHT_VSPOL_DET               7
#define CAP_SHT_HSPOL_DET               6
#define CAP_SHT_VHS      	            1
#define CAP_SHT_PHS		                0

#define CAP_MSK_DUMPMODE 		        (N01_BITS_MSK << CAP_SHT_DUMPMODE)
#define CAP_MSK_CHECK_DE 		        (N01_BITS_MSK << CAP_SHT_CHECK_DE)
#define CAP_MSK_CHECK_VS 		        (N01_BITS_MSK << CAP_SHT_CHECK_VS)
#define CAP_MSK_CHECK_HS 		        (N01_BITS_MSK << CAP_SHT_CHECK_HS)
#define CAP_MSK_SAMPLE_VS             	(N01_BITS_MSK << CAP_SHT_SAMPLE_VS)
#define CAP_MSK_HSYNC_SKIP              (N06_BITS_MSK << CAP_SHT_HSYNC_SKIP)
#define CAP_MSK_VSYNC_SKIP              (N03_BITS_MSK << CAP_SHT_VSYNC_SKIP)
#define CAP_MSK_VSPOL_DET               (N01_BITS_MSK << CAP_SHT_VSPOL_DET)
#define CAP_MSK_HSPOL_DET               (N01_BITS_MSK << CAP_SHT_HSPOL_DET)
#define CAP_MSK_VHS                 	(N01_BITS_MSK << CAP_SHT_VHS)
#define CAP_MSK_PHS                 	(N01_BITS_MSK << CAP_SHT_PHS)

//====================================================================
/*
 * 0x0054
 *    Capture Internal Setting Register
 */
//====================================================================
#define CAP_REG_CAPTURE_OUTPUT        (REG_CAP_BASE + 0x0054)
#define CAP_REG_FRAMERATE_SETTING     (REG_CAP_BASE + 0x0058)
#define CAP_REG_ERROR_DETECT          (REG_CAP_BASE + 0x005C)
#define CAP_REG_DITHER_SET        	  (REG_CAP_BASE + 0x0064)
#define CAP_REG_ROI_SET_0      	  	  (REG_CAP_BASE + 0x0068)
#define CAP_REG_ROI_SET_1      	  	  (REG_CAP_BASE + 0x006C)

#define CAP_REG_COLOR_SPACE_CON_0     (REG_CAP_BASE + 0x0070)
#define CAP_REG_COLOR_SPACE_CON_1     (REG_CAP_BASE + 0x0074)
#define CAP_REG_COLOR_SPACE_CON_2     (REG_CAP_BASE + 0x0078)
#define CAP_REG_COLOR_SPACE_CON_3     (REG_CAP_BASE + 0x007C)
#define CAP_REG_COLOR_SPACE_CON_4     (REG_CAP_BASE + 0x0080)
#define CAP_REG_COLOR_SPACE_CON_5     (REG_CAP_BASE + 0x0084)
#define CAP_REG_COLOR_SPACE_CON_6     (REG_CAP_BASE + 0x0088)
#define CAP_REG_SCALING_SET_0 		  (REG_CAP_BASE + 0x008C)
#define CAP_REG_SCALING_SET_1 		  (REG_CAP_BASE + 0x0090)
#define CAP_REG_SCALING_SET_2 		  (REG_CAP_BASE + 0x0094)
#define CAP_REG_SCALING_SET_3 		  (REG_CAP_BASE + 0x0098)
#define CAP_REG_SCALING_SET_4 		  (REG_CAP_BASE + 0x009C)
#define CAP_REG_SCALING_SET_5 		  (REG_CAP_BASE + 0x00A0)
#define CAP_REG_SCALING_SET_6 		  (REG_CAP_BASE + 0x00A4)

#define CAP_REG_GEN_SET_0	 		  (REG_CAP_BASE + 0x00A8)
#define CAP_REG_GEN_SET_1	 		  (REG_CAP_BASE + 0x00AC)
#define CAP_REG_GEN_SET_2	 		  (REG_CAP_BASE + 0x00B0)
#define CAP_REG_GEN_SET_3	 		  (REG_CAP_BASE + 0x00B4)
#define CAP_REG_GEN_SET_4	 		  (REG_CAP_BASE + 0x00B8)
#define CAP_REG_GEN_SET_5	 		  (REG_CAP_BASE + 0x00BC)
#define CAP_REG_GEN_SET_6	 		  (REG_CAP_BASE + 0x00C0)
#define CAP_REG_GEN_SET_7	 		  (REG_CAP_BASE + 0x00C4)
#define CAP_REG_GEN_SET_8	 		  (REG_CAP_BASE + 0x00C8)
#define CAP_REG_GEN_SET_9	 		  (REG_CAP_BASE + 0x00CC)
#define CAP_REG_GEN_SET_10	 		  (REG_CAP_BASE + 0x00D0)
#define CAP_REG_GEN_SET_11	 		  (REG_CAP_BASE + 0x00D4)
#define CAP_REG_GEN_SET_12	 		  (REG_CAP_BASE + 0x00D8)
#define CAP_REG_GEN_SET_13	 		  (REG_CAP_BASE + 0x00DC)
#define CAP_REG_INTR_SET	 		  (REG_CAP_BASE + 0x00E0)
#define CAP_REG_GEN_SET_14	 		  (REG_CAP_BASE + 0x00E4)
#define CAP_REG_GEN_SET_15	 		  (REG_CAP_BASE + 0x00E8)
#define CAP_REG_GEN_SET_16	 		  (REG_CAP_BASE + 0x00EC)
#define CAP_REG_GEN_SET_17	 		  (REG_CAP_BASE + 0x00F0)
#define CAP_REG_GEN_SET_18	 		  (REG_CAP_BASE + 0x00F4)
#define CAP_REG_GEN_SET_19	 		  (REG_CAP_BASE + 0x00F8)
#define CAP_REG_GEN_SET_20	 		  (REG_CAP_BASE + 0x00FC)
#define CAP_REG_GEN_SET_21	 		  (REG_CAP_BASE + 0x0100)
#define CAP_REG_GEN_SET_22	 		  (REG_CAP_BASE + 0x0104)
#define CAP_REG_GEN_SET_23	 		  (REG_CAP_BASE + 0x0108)
#define CAP_REG_GEN_SET_24	 		  (REG_CAP_BASE + 0x010C)
#define CAP_REG_GEN_SET_25	 		  (REG_CAP_BASE + 0x0110)
#define CAP_REG_GEN_SET_26	 		  (REG_CAP_BASE + 0x0114)
#define CAP_REG_GEN_SET_27	 		  (REG_CAP_BASE + 0x0118)
#define CAP_REG_GEN_SET_28	 		  (REG_CAP_BASE + 0x011C)
#define CAP_REG_GEN_SET_29	 		  (REG_CAP_BASE + 0x0120)
#define CAP_REG_GEN_SET_30	 		  (REG_CAP_BASE + 0x0124)
#define CAP_REG_GEN_SET_31	 		  (REG_CAP_BASE + 0x0128)
#define CAP_REG_AVSYNC_SET_0	 	  (REG_CAP_BASE + 0x012C)
#define CAP_REG_AVSYNC_SET_1	 	  (REG_CAP_BASE + 0x0130)
#define CAP_REG_GEN_SET_32	 		  (REG_CAP_BASE + 0x0134)
#define CAP_REG_ENGINE_STATUS_0 	  (REG_CAP_BASE + 0x0200)
#define CAP_REG_ENGINE_STATUS_1 	  (REG_CAP_BASE + 0x0204)
#define CAP_REG_ENGINE_STATUS_2 	  (REG_CAP_BASE + 0x0208)
#define CAP_REG_ENGINE_STATUS_3 	  (REG_CAP_BASE + 0x020C)
#define CAP_REG_AVSYNC_STATUS_0		  (REG_CAP_BASE + 0x0210)
#define CAP_REG_AVSYNC_STATUS_1		  (REG_CAP_BASE + 0x0214)
#define CAP_REG_AVSYNC_STATUS_2		  (REG_CAP_BASE + 0x0218)
#define CAP_REG_AVSYNC_STATUS_3		  (REG_CAP_BASE + 0x021c)
#define CAP_REG_SRAM_BIST_STATUS	  (REG_CAP_BASE + 0x0220)
#define CAP_REG_DEBUG            	  (REG_CAP_BASE + 0x0224)


//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef enum CAP_REG_TABLE_TAG
{
    CAP_REG_000			= 0,
    CAP_REG_004			= 1,
    CAP_REG_008			= 2,
    CAP_REG_00C			= 3,
    CAP_REG_010			= 4,
    CAP_REG_014			= 5,
    CAP_REG_018			= 6,
    CAP_REG_01C			= 7,
    CAP_REG_020			= 8,
    CAP_REG_024			= 9,
    CAP_REG_028			= 10,
    CAP_REG_02C			= 11,
    CAP_REG_030			= 12,
    CAP_REG_034			= 13,
    CAP_REG_038			= 14,
    CAP_REG_03C			= 15,
    CAP_REG_040			= 16,
    CAP_REG_044			= 17,
    CAP_REG_048			= 18,
    CAP_REG_04C			= 19,
    CAP_REG_050			= 20,
    CAP_REG_054			= 21,
    CAP_REG_058			= 22,
    CAP_REG_05C			= 23,
    CAP_REG_060			= 24,
    CAP_REG_064			= 25,
    CAP_REG_068			= 26,
    CAP_REG_06C			= 27,
    CAP_REG_070			= 28,
    CAP_REG_074			= 29,
    CAP_REG_078			= 30,
    CAP_REG_07C			= 31,
    CAP_REG_080			= 32,
    CAP_REG_084			= 33,
    CAP_REG_088			= 34,
    CAP_REG_08C			= 35,
    CAP_REG_090			= 36,
    CAP_REG_094			= 37,
    CAP_REG_098			= 38,
    CAP_REG_09C			= 39,
    CAP_REG_0A0			= 40,
    CAP_REG_0A4			= 41,
    CAP_REG_0A8			= 42,
    CAP_REG_0AC			= 43,
    CAP_REG_0B0			= 44,
    CAP_REG_0B4			= 45,
    CAP_REG_0B8			= 46,
    CAP_REG_0BC			= 47,
    CAP_REG_0C0			= 48,
    CAP_REG_0C4			= 49,
    CAP_REG_0C8			= 50,
    CAP_REG_0CC			= 51,
    CAP_REG_0D0			= 52,
    CAP_REG_0D4			= 53,
    CAP_REG_0D8			= 54,
    CAP_REG_0DC			= 55,
    CAP_REG_0E0			= 56,
    CAP_REG_0E4			= 57,
    CAP_REG_0E8			= 58,
    CAP_REG_0EC			= 59,
    CAP_REG_0F0			= 60,
    CAP_REG_0F4			= 61,
    CAP_REG_0F8			= 62,
    CAP_REG_0FC			= 63,
    CAP_REG_100			= 64,
    CAP_REG_104			= 65,
    CAP_REG_108			= 66,
    CAP_REG_10C			= 67,
    CAP_REG_110			= 68,
    CAP_REG_114			= 69,
    CAP_REG_118			= 70,
    CAP_REG_11C			= 71,
    CAP_REG_120			= 72,
    CAP_REG_124			= 73,
    CAP_REG_128			= 74,
    CAP_REG_12C			= 75,
    CAP_REG_130			= 76,
    CAP_REG_134			= 77,
    CAP_REG_138			= 78,
    CAP_REG_13C 		= 79,
    CAP_REG_200			= 80,
    CAP_REG_204			= 81,
    CAP_REG_208			= 82,
    CAP_REG_20C			= 83,
    CAP_REG_210			= 84,
    CAP_REG_214			= 85,
    CAP_REG_218			= 86,
    CAP_REG_21C			= 87,
    CAP_REG_220			= 88,
} CAP_REG_TABLE;

//=============================================================================
//                Global Data Definition
//=============================================================================
static MMP_UINT32
CapRegTable[2][89] = {
  {
    0xD0700000,  0xD0700004, 0xD0700008, 0xD070000C, 0xD0700010, 0xD0700014, 0xD0700018, 0xD070001C,
    0xD0700020,  0xD0700024, 0xD0700028, 0xD070002C, 0xD0700030, 0xD0700034, 0xD0700038, 0xD070003C,
    0xD0700040,  0xD0700044, 0xD0700048, 0xD070004C, 0xD0700050, 0xD0700054, 0xD0700058, 0xD070005C,
    0xD0700060,  0xD0700064, 0xD0700068, 0xD070006C, 0xD0700070, 0xD0700074, 0xD0700078, 0xD070007C,
    0xD0700080,  0xD0700084, 0xD0700088, 0xD070008C, 0xD0700090, 0xD0700094, 0xD0700098, 0xD070009C,
    0xD07000A0,  0xD07000A4, 0xD07000A8, 0xD07000AC, 0xD07000B0, 0xD07000B4, 0xD07000B8, 0xD07000BC,
    0xD07000C0,  0xD07000C4, 0xD07000C8, 0xD07000CC, 0xD07000D0, 0xD07000D4, 0xD07000D8, 0xD07000DC,
    0xD07000E0,  0xD07000E4, 0xD07000E8, 0xD07000EC, 0xD07000F0, 0xD07000F4, 0xD07000F8, 0xD07000FC,
    0xD0700100,  0xD0700104, 0xD0700108, 0xD070010C, 0xD0700110, 0xD0700114, 0xD0700118, 0xD070011C,
    0xD0700120,  0xD0700124, 0xD0700128, 0xD070012C, 0xD0700130, 0xD0700134, 0xD0700138, 0xD070013C,
    0xD0700200,  0xD0700204, 0xD0700208, 0xD070020C, 0xD0700210, 0xD0700214, 0xD0700218, 0xD070021C,
    0xD0700220
  },
  {
	0xD0701000,  0xD0701004, 0xD0701008, 0xD070100C, 0xD0701010, 0xD0701014, 0xD0701018, 0xD070101C,
	0xD0701020,  0xD0701024, 0xD0701028, 0xD070102C, 0xD0701030, 0xD0701034, 0xD0701038, 0xD070103C,
	0xD0701040,  0xD0701044, 0xD0701048, 0xD070104C, 0xD0701050, 0xD0701054, 0xD0701058, 0xD070105C,
	0xD0701060,  0xD0701064, 0xD0701068, 0xD070106C, 0xD0701070, 0xD0701074, 0xD0701078, 0xD070107C,
	0xD0701080,  0xD0701084, 0xD0701088, 0xD070108C, 0xD0701090, 0xD0701094, 0xD0701098, 0xD070109C,
	0xD07010A0,  0xD07010A4, 0xD07010A8, 0xD07010AC, 0xD07010B0, 0xD07010B4, 0xD07010B8, 0xD07010BC,
	0xD07010C0,  0xD07010C4, 0xD07010C8, 0xD07010CC, 0xD07010D0, 0xD07010D4, 0xD07010D8, 0xD07010DC,
	0xD07010E0,  0xD07010E4, 0xD07010E8, 0xD07010EC, 0xD07010F0, 0xD07010F4, 0xD07010F8, 0xD07010FC,
	0xD0701100,  0xD0701104, 0xD0701108, 0xD070110C, 0xD0701110, 0xD0701114, 0xD0701118, 0xD070111C,
	0xD0701120,  0xD0701124, 0xD0701128, 0xD070112C, 0xD0701130, 0xD0701134, 0xD0701138, 0xD070113C,
	0xD0701200,  0xD0701204, 0xD0701208, 0xD070120C, 0xD0701210, 0xD0701214, 0xD0701218, 0xD070121C,
	0xD0701220
  }
};

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
