/*
 * Copyright (c) 2011 ITE Technology Corp. All Rights Reserved.
 *
 * File : video_register.h
 * Author : Evan Chang
 */

#ifndef _VIDEO_REGISTER_H_
#define _VIDEO_REGISTER_H_

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Constant Definition
 */

#define REG_VIDEO_BASE                          (0x0E00)
#define REG_VIDEO_BASE_EXT                      (0x0F00)

/*
 * 01.  0x0E00 ~ 0x0E03
 * Base Address of Buffer 0 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_0_Y_ADDR_L_00H   (REG_VIDEO_BASE + 0x0000)
#define REG_VIDEO_DECODE_FRAME_0_Y_ADDR_H_02H   (REG_VIDEO_BASE + 0x0002)

#define VIDEO_MASK_DECODE_FRAME_ADDR            (0x0FFFFFFF)

/*
 * 02.  0x0E04 ~ 0x0E07
 * Base Address of Buffer 0 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_0_U_ADDR_L_04H   (REG_VIDEO_BASE + 0x0004)
#define REG_VIDEO_DECODE_FRAME_0_U_ADDR_H_06H   (REG_VIDEO_BASE + 0x0006)

/*
 * 03.  0x0E08 ~ 0x0E0b
 * Base Address of Buffer 0 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_0_V_ADDR_L_08H   (REG_VIDEO_BASE + 0x0008)
#define REG_VIDEO_DECODE_FRAME_0_V_ADDR_H_0AH   (REG_VIDEO_BASE + 0x000A)

/*
 * 04.  0x0E0C ~ 0x0E0F
 * Base Address of Buffer 1 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_1_Y_ADDR_L_0CH   (REG_VIDEO_BASE + 0x000C)
#define REG_VIDEO_DECODE_FRAME_1_Y_ADDR_H_0EH   (REG_VIDEO_BASE + 0x000E)

/*
 * 05.  0x0E10 ~ 0x0E13
 * Base Address of Buffer 1 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_1_U_ADDR_L_10H   (REG_VIDEO_BASE + 0x0010)
#define REG_VIDEO_DECODE_FRAME_1_U_ADDR_H_12H   (REG_VIDEO_BASE + 0x0012)

/*
 * 06.  0x0E14 ~ 0x0E17
 * Base Address of Buffer 1 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_1_V_ADDR_L_14H   (REG_VIDEO_BASE + 0x0014)
#define REG_VIDEO_DECODE_FRAME_1_V_ADDR_H_16H   (REG_VIDEO_BASE + 0x0016)

/*
 * 07.  0x0E18 ~ 0x0E1B
 * Base Address of Buffer 2 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_2_Y_ADDR_L_18H   (REG_VIDEO_BASE + 0x0018)
#define REG_VIDEO_DECODE_FRAME_2_Y_ADDR_H_1AH   (REG_VIDEO_BASE + 0x001A)

/*
 * 08.  0x0E1C ~ 0x0E1F
 * Base Address of Buffer 2 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_2_U_ADDR_L_1CH   (REG_VIDEO_BASE + 0x001C)
#define REG_VIDEO_DECODE_FRAME_2_U_ADDR_H_1EH   (REG_VIDEO_BASE + 0x001E)

/*
 * 09.  0x0E20 ~ 0x0E23
 * Base Address of Buffer 2 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_2_V_ADDR_L_20H   (REG_VIDEO_BASE + 0x0020)
#define REG_VIDEO_DECODE_FRAME_2_V_ADDR_H_22H   (REG_VIDEO_BASE + 0x0022)

/*
 * 10.  0x0E24 ~ 0x0E27
 * Base Address of Buffer 3 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_3_Y_ADDR_L_24H   (REG_VIDEO_BASE + 0x0024)
#define REG_VIDEO_DECODE_FRAME_3_Y_ADDR_H_26H   (REG_VIDEO_BASE + 0x0026)

/*
 * 11.  0x0E28 ~ 0x0E2B
 * Base Address of Buffer 3 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_3_U_ADDR_L_28H   (REG_VIDEO_BASE + 0x0028)
#define REG_VIDEO_DECODE_FRAME_3_U_ADDR_H_2AH   (REG_VIDEO_BASE + 0x002A)

/*
 * 12.  0x0E2C ~ 0x0E2F
 * Base Address of Buffer 3 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_3_V_ADDR_L_2CH   (REG_VIDEO_BASE + 0x002C)
#define REG_VIDEO_DECODE_FRAME_3_V_ADDR_H_2EH   (REG_VIDEO_BASE + 0x002E)

/*
 * 13.  0x0E30 ~ 0x0E33
 * Base Address of Buffer 4 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_4_Y_ADDR_L_30H   (REG_VIDEO_BASE + 0x0030)
#define REG_VIDEO_DECODE_FRAME_4_Y_ADDR_H_32H   (REG_VIDEO_BASE + 0x0032)

/*
 * 14.  0x0E34 ~ 0x0E37
 * Base Address of Buffer 4 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_4_U_ADDR_L_34H   (REG_VIDEO_BASE + 0x0034)
#define REG_VIDEO_DECODE_FRAME_4_U_ADDR_H_36H   (REG_VIDEO_BASE + 0x0036)

/*
 * 15.  0x0E38 ~ 0x0E3B
 * Base Address of Buffer 4 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_4_V_ADDR_L_38H   (REG_VIDEO_BASE + 0x0038)
#define REG_VIDEO_DECODE_FRAME_4_V_ADDR_H_3AH   (REG_VIDEO_BASE + 0x003A)

/*
 * 16.  0x0E3C ~ 0x0E3F
 * Base Address of Buffer 5 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_5_Y_ADDR_L_3CH   (REG_VIDEO_BASE + 0x003C)
#define REG_VIDEO_DECODE_FRAME_5_Y_ADDR_H_3EH   (REG_VIDEO_BASE + 0x003E)

/*
 * 17.  0x0E40 ~ 0x0E43
 * Base Address of Buffer 5 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_5_U_ADDR_L_40H   (REG_VIDEO_BASE + 0x0040)
#define REG_VIDEO_DECODE_FRAME_5_U_ADDR_H_42H   (REG_VIDEO_BASE + 0x0042)

/*
 * 18.  0x0E44 ~ 0x0E47
 * Base Address of Buffer 5 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_5_V_ADDR_L_44H   (REG_VIDEO_BASE + 0x0044)
#define REG_VIDEO_DECODE_FRAME_5_V_ADDR_H_46H   (REG_VIDEO_BASE + 0x0046)

/*
 * 19.  0x0E48 ~ 0x0E4B
 * Base Address of Buffer 6 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_6_Y_ADDR_L_48H   (REG_VIDEO_BASE + 0x0048)
#define REG_VIDEO_DECODE_FRAME_6_Y_ADDR_H_4AH   (REG_VIDEO_BASE + 0x004A)

/*
 * 20.  0x0E4C ~ 0x0E4F
 * Base Address of Buffer 6 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_6_U_ADDR_L_4CH   (REG_VIDEO_BASE + 0x004C)
#define REG_VIDEO_DECODE_FRAME_6_U_ADDR_H_4EH   (REG_VIDEO_BASE + 0x004E)

/*
 * 21.  0x0E50 ~ 0x0E53
 * Base Address of Buffer 6 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_6_V_ADDR_L_50H   (REG_VIDEO_BASE + 0x0050)
#define REG_VIDEO_DECODE_FRAME_6_V_ADDR_H_52H   (REG_VIDEO_BASE + 0x0052)

/*
 * 22.  0x0E54 ~ 0x0E57
 * Base Address of Buffer 7 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_7_Y_ADDR_L_54H   (REG_VIDEO_BASE + 0x0054)
#define REG_VIDEO_DECODE_FRAME_7_Y_ADDR_H_56H   (REG_VIDEO_BASE + 0x0056)

/*
 * 23.  0x0E58 ~ 0x0E5B
 * Base Address of Buffer 7 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_7_U_ADDR_L_58H   (REG_VIDEO_BASE + 0x0058)
#define REG_VIDEO_DECODE_FRAME_7_U_ADDR_H_5AH   (REG_VIDEO_BASE + 0x005A)

/*
 * 24.  0x0E5C ~ 0x0E5F
 * Base Address of Buffer 7 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_7_V_ADDR_L_5CH   (REG_VIDEO_BASE + 0x005C)
#define REG_VIDEO_DECODE_FRAME_7_V_ADDR_H_5EH   (REG_VIDEO_BASE + 0x005E)

/*
 * 25.  0x0E60 ~ 0x0E63
 * MV List0 Buffer Base Address
 */
#define REG_VIDEO_MV_BUF_0_ADDR_L_60H           (REG_VIDEO_BASE + 0x0060)
#define REG_VIDEO_MV_BUF_0_ADDR_H_62H           (REG_VIDEO_BASE + 0x0062)

/*
 * 26.  0x0E64~0x0E67
 * MV List1 Buffer Base Address
 */
#define REG_VIDEO_MV_BUF_1_ADDR_L_64H           (REG_VIDEO_BASE + 0x0064)
#define REG_VIDEO_MV_BUF_1_ADDR_H_66H           (REG_VIDEO_BASE + 0x0066)

/*
 * 27.  0x0E68 ~ 0x0E6B
 * TC Buffer Base Address
 */
#define REG_VIDEO_TC_BUF_ADDR_L_68H             (REG_VIDEO_BASE + 0x0068)
#define REG_VIDEO_TC_BUF_ADDR_H_6AH             (REG_VIDEO_BASE + 0x006A)

/*
 * 28.  0x0E6C ~ 0x0E6F
 * VLD Buffer Base Address
 */
#define REG_VIDEO_VLD_BUF_ADDR_L_6CH            (REG_VIDEO_BASE + 0x006C)
#define REG_VIDEO_VLD_BUF_ADDR_H_6EH            (REG_VIDEO_BASE + 0x006E)

/*
 * 29.  0x0E70 ~ 0x0E73
 * Base Address of Buffer 8 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_8_Y_ADDR_L_70H   (REG_VIDEO_BASE + 0x0070)
#define REG_VIDEO_DECODE_FRAME_8_Y_ADDR_H_72H   (REG_VIDEO_BASE + 0x0072)

/*
 * 30.  0x0E74 ~ 0x0E77
 * Base Address of Buffer 8 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_8_U_ADDR_L_74H   (REG_VIDEO_BASE + 0x0074)
#define REG_VIDEO_DECODE_FRAME_8_U_ADDR_H_76H   (REG_VIDEO_BASE + 0x0076)

/*
 * 31.  0x0E78 ~ 0x0E7B
 * Base Address of Buffer 8 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_8_V_ADDR_L_78H   (REG_VIDEO_BASE + 0x0078)
#define REG_VIDEO_DECODE_FRAME_8_V_ADDR_H_7AH   (REG_VIDEO_BASE + 0x007A)

/*
 * 32.  0x0E7C ~ 0x0E7F
 * Base Address of Buffer 9 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_9_Y_ADDR_L_7CH   (REG_VIDEO_BASE + 0x007C)
#define REG_VIDEO_DECODE_FRAME_9_Y_ADDR_H_7EH   (REG_VIDEO_BASE + 0x007E)

/*
 * 33.  0x0E80 ~ 0x0E83
 * Base Address of Buffer 9 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_9_U_ADDR_L_80H   (REG_VIDEO_BASE + 0x0080)
#define REG_VIDEO_DECODE_FRAME_9_U_ADDR_H_83H   (REG_VIDEO_BASE + 0x0082)

/*
 * 34.  0x0E84 ~ 0x0E87
 * Base Address of Buffer 9 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_9_V_ADDR_L_84H   (REG_VIDEO_BASE + 0x0084)
#define REG_VIDEO_DECODE_FRAME_9_V_ADDR_H_86H   (REG_VIDEO_BASE + 0x0086)

/*
 * 35.  0x0E88 ~ 0x0E8B
 * Base Address of Buffer 10 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_10_Y_ADDR_L_88H  (REG_VIDEO_BASE + 0x0088)
#define REG_VIDEO_DECODE_FRAME_10_Y_ADDR_H_8AH  (REG_VIDEO_BASE + 0x008A)

/*
 * 36.  0x0E8C ~ 0x0E8F
 * Base Address of Buffer 10 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_10_U_ADDR_L_8CH  (REG_VIDEO_BASE + 0x008C)
#define REG_VIDEO_DECODE_FRAME_10_U_ADDR_H_8EH  (REG_VIDEO_BASE + 0x008E)

/*
 * 37.  0x0E90 ~ 0x0E93
 * Base Address of Buffer 10 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_10_V_ADDR_L_90H  (REG_VIDEO_BASE + 0x0090)
#define REG_VIDEO_DECODE_FRAME_10_V_ADDR_H_92H  (REG_VIDEO_BASE + 0x0092)

/*
 * 38.  0x0E94 ~ 0x0E97
 * Base Address of Buffer 11 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_11_Y_ADDR_L_94H  (REG_VIDEO_BASE + 0x0094)
#define REG_VIDEO_DECODE_FRAME_11_Y_ADDR_H_96H  (REG_VIDEO_BASE + 0x0096)

/*
 * 39.  0x0E98 ~ 0x0E9B
 * Base Address of Buffer 11 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_11_U_ADDR_L_98H  (REG_VIDEO_BASE + 0x0098)
#define REG_VIDEO_DECODE_FRAME_11_U_ADDR_H_9AH  (REG_VIDEO_BASE + 0x009A)

/*
 * 40. 0x0E9C ~ 0x0E9F
 * Base Address of Buffer 11 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_11_V_ADDR_L_9CH  (REG_VIDEO_BASE + 0x009C)
#define REG_VIDEO_DECODE_FRAME_11_V_ADDR_H_9EH  (REG_VIDEO_BASE + 0x009E)

/*
 * 41.  0x0EA0 ~ 0x0EA3
 * Base Address of Buffer 12 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_12_Y_ADDR_L_A0H  (REG_VIDEO_BASE + 0x00A0)
#define REG_VIDEO_DECODE_FRAME_12_Y_ADDR_H_A2H  (REG_VIDEO_BASE + 0x00A2)

/*
 * 42.  0x0EA4 ~ 0x0EA7
 * Base Address of Buffer 12 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_12_U_ADDR_L_A4H  (REG_VIDEO_BASE + 0x00A4)
#define REG_VIDEO_DECODE_FRAME_12_U_ADDR_H_A6H  (REG_VIDEO_BASE + 0x00A6)

/*
 * 43.  0x0EA8 ~ 0x0EAB
 * Base Address of Buffer 12 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_12_V_ADDR_L_A8H  (REG_VIDEO_BASE + 0x00A8)
#define REG_VIDEO_DECODE_FRAME_12_V_ADDR_H_AAH  (REG_VIDEO_BASE + 0x00AA)

/*
 * 44.  0x0EAC ~ 0x0EAF
 * Base Address of Buffer 13 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_13_Y_ADDR_L_ACH  (REG_VIDEO_BASE + 0x00AC)
#define REG_VIDEO_DECODE_FRAME_13_Y_ADDR_H_AEH  (REG_VIDEO_BASE + 0x00AE)

/*
 * 45.  0x0EB0 ~ 0x0EB3
 * Base Address of Buffer 13 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_13_U_ADDR_L_B0H  (REG_VIDEO_BASE + 0x00B0)
#define REG_VIDEO_DECODE_FRAME_13_U_ADDR_H_B2H  (REG_VIDEO_BASE + 0x00B2)

/*
 * 46.  0x0EB4 ~ 0x0EB7
 * Base Address of Buffer 13 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_13_V_ADDR_L_B4H  (REG_VIDEO_BASE + 0x00B4)
#define REG_VIDEO_DECODE_FRAME_13_V_ADDR_H_B6H  (REG_VIDEO_BASE + 0x00B6)

/*
 * 47.  0x0EB8 ~ 0x0EBB
 * Base Address of Buffer 14 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_14_Y_ADDR_L_B8H  (REG_VIDEO_BASE + 0x00B8)
#define REG_VIDEO_DECODE_FRAME_14_Y_ADDR_H_BAH  (REG_VIDEO_BASE + 0x00BA)

/*
 * 48.  0x0EBC ~ 0x0EBF
 * Base Address of Buffer 14 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_14_U_ADDR_L_BCH  (REG_VIDEO_BASE + 0x00BC)
#define REG_VIDEO_DECODE_FRAME_14_U_ADDR_H_BEH  (REG_VIDEO_BASE + 0x00BE)

/*
 * 49.  0x0EC0 ~ 0x0EC3
 * Base Address of Buffer 14 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_14_V_ADDR_L_C0H  (REG_VIDEO_BASE + 0x00C0)
#define REG_VIDEO_DECODE_FRAME_14_V_ADDR_H_C2H  (REG_VIDEO_BASE + 0x00C2)

/*
 * 50.  0x0EC4 ~ 0x0EC7
 * Base Address of Buffer 15 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_15_Y_ADDR_L_C4H  (REG_VIDEO_BASE + 0x00C4)
#define REG_VIDEO_DECODE_FRAME_15_Y_ADDR_H_C6H  (REG_VIDEO_BASE + 0x00C6)

/*
 * 51.  0x0EC8 ~ 0x0ECB
 * Base Address of Buffer 15 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_15_U_ADDR_L_C8H  (REG_VIDEO_BASE + 0x00C8)
#define REG_VIDEO_DECODE_FRAME_15_U_ADDR_H_CAH  (REG_VIDEO_BASE + 0x00CA)

/*
 * 52.  0x0ECC ~ 0x0ECF
 * Base Address of Buffer 15 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_15_V_ADDR_L_CCH  (REG_VIDEO_BASE + 0x00CC)
#define REG_VIDEO_DECODE_FRAME_15_V_ADDR_H_CEH  (REG_VIDEO_BASE + 0x00CE)

/*
 * 53.  0x0ED0 ~ 0x0ED3
 * Base Address of Buffer 16 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_16_Y_ADDR_L_D0H  (REG_VIDEO_BASE + 0x00D0)
#define REG_VIDEO_DECODE_FRAME_16_Y_ADDR_H_D2H  (REG_VIDEO_BASE + 0x00D2)

/*
 * 54.  0x0ED4 ~ 0x0ED7
 * Base Address of Buffer 16 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_16_U_ADDR_L_D4H  (REG_VIDEO_BASE + 0x00D4)
#define REG_VIDEO_DECODE_FRAME_16_U_ADDR_H_D6H  (REG_VIDEO_BASE + 0x00D6)

/*
 * 55.  0x0ED8 ~ 0x0EDB
 * Base Address of Buffer 16 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_16_V_ADDR_L_D8H  (REG_VIDEO_BASE + 0x00D8)
#define REG_VIDEO_DECODE_FRAME_16_V_ADDR_H_DAH  (REG_VIDEO_BASE + 0x00DA)

/*
 * 56.  0x0EDC ~ 0x0EDF
 * Base Address of Buffer 17 for Y Frame
 */
#define REG_VIDEO_DECODE_FRAME_17_Y_ADDR_L_DCH  (REG_VIDEO_BASE + 0x00DC)
#define REG_VIDEO_DECODE_FRAME_17_Y_ADDR_H_DEH  (REG_VIDEO_BASE + 0x00DE)

/*
 * 57.  0x0EE0 ~ 0x0EE3
 * Base Address of Buffer 17 for U Frame
 */
#define REG_VIDEO_DECODE_FRAME_17_U_ADDR_L_E0H  (REG_VIDEO_BASE + 0x00E0)
#define REG_VIDEO_DECODE_FRAME_17_U_ADDR_H_E2H  (REG_VIDEO_BASE + 0x00E2)

/*
 * 58.  0x0EE4 ~ 0x0EE7
 * Base Address of Buffer 17 for V Frame
 */
#define REG_VIDEO_DECODE_FRAME_17_V_ADDR_L_E4H  (REG_VIDEO_BASE + 0x00E4)
#define REG_VIDEO_DECODE_FRAME_17_V_ADDR_H_E6H  (REG_VIDEO_BASE + 0x00E6)

/*
 * 59.  0x0EE8
 * Engine Parameter Reserve Register 0
 */
#define REG_VIDEO_DB_BUF_ADDR_L_E8H             (REG_VIDEO_BASE + 0x00E8)
#define REG_VIDEO_DB_BUF_ADDR_H_EAH             (REG_VIDEO_BASE + 0x00EA)

/*
 * 61.  0x0EEC
 * Col Data Bottom field offset Register
 */
#define REG_VIDEO_COL_BOT_OFFSET_ECH            (REG_VIDEO_BASE + 0x00EC)

/*
 * 62.  0x0EEE
 * H264 interrupt Setting Register
 */
#define REG_VIDEO_INTERRUPT_SETTING_3_EEH       (REG_VIDEO_BASE + 0x00EE)

/*
 * 63.  0x0EF0
 * Special Function Setting Register
 */
#define REG_VIDEO_SPECIAL_FUNC_SETTING_F0H      (REG_VIDEO_BASE + 0x00F0)

/*
 * 64.  0x0EF2
 * Reserve Register 0
 */
#define REG_VIDEO_RESERVE_0_F2H                 (REG_VIDEO_BASE + 0x00F2)

/*
 * 65.  0x0EF4 ~ 0x0EF7
 * Command and Data Buffer Starting Address Register
 */
#define REG_VIDEO_CMD_DATA_BUF_ADDR_L_F4H       (REG_VIDEO_BASE + 0x00F4)
#define REG_VIDEO_CMD_DATA_BUF_ADDR_H_F6H       (REG_VIDEO_BASE + 0x00F6)

#define VIDEO_MASK_CMD_DATA_BUF_ADDR            (0x03FFFFFF)

/*
 * 66.  0x0EF8
 * Decoder Engine Status Register 0
 */
#define REG_VIDEO_DECODE_STATUS_0_F8H           (REG_VIDEO_BASE + 0x00F8)

/*
 * 67.  0x0EFA
 * Decoder Engine Status Register 1
 */
#define REG_VIDEO_DECODE_STATUS_1_FAH           (REG_VIDEO_BASE + 0x00FA)

/*
 * 68.  0x0EFC
 * Decoder Engine Status Register 2
 */
#define REG_VIDEO_DECODE_STATUS_2_FCH           (REG_VIDEO_BASE + 0x00FC)

#define VIDEO_MASK_QUEUE_STATUS                 (0x0100)        /* ---- ---1 ---- ---- */
#define VIDEO_MASK_DECODE_STATUS                (0x0200)        /* ---- --1- ---- ---- */

#define VIDEO_DECODE_STATUS_IDLE                (0x0000)        /* ---- --00 ---- ---- */

// 0x0F00 ~ 0x0F03
#define REG_VIDEO_WRITABLE_MEMORY_RANGE_END_ADDR_L_00H      (REG_VIDEO_BASE_EXT + 0x0000)
#define REG_VIDEO_WRITABLE_MEMORY_RANGE_END_ADDR_H_02H      (REG_VIDEO_BASE_EXT + 0x0002)

// 0x0F04 ~ 0x0F07
#define REG_VIDEO_WRITABLE_MEMORY_RANGE_START_ADDR_L_04H    (REG_VIDEO_BASE_EXT + 0x0004)
#define REG_VIDEO_WRITABLE_MEMORY_RANGE_START_ADDR_H_06H    (REG_VIDEO_BASE_EXT + 0x0006)

/*
 * 69.  0x0F60
 * Decoder Engine Status Register 3
 */
#define REG_VIDEO_DECODE_STATUS_3_60H           (REG_VIDEO_BASE_EXT + 0x0060)

/*
 * 70.  0x0F62
 * Decoder Engine Status Register 4
 */
#define REG_VIDEO_DECODE_STATUS_4_62H           (REG_VIDEO_BASE_EXT + 0x0062)

/*
 * 71.  0x0F64
 * Decoder Engine Status Register 5
 */
#define REG_VIDEO_DECODE_STATUS_5_64H           (REG_VIDEO_BASE_EXT + 0x0064)

/*
 * 72.  0x0F66
 * Decoder Engine Status Register 6
 */
#define REG_VIDEO_DECODE_STATUS_6_66H           (REG_VIDEO_BASE_EXT + 0x0066)

/*
 * 73.  0x0F68
 * VLD Debug Info 0
 */
#define REG_VIDEO_VLD_DEBUG_INFO_0_68H          (REG_VIDEO_BASE_EXT + 0x0068)

#define VIDEO_MASK_MACROBLOCK_X_COORDINATE      (0x3F80)        /* --11 1111 1000 0000 */
#define VIDEO_MASK_MACROBLOCK_Y_COORDINATE      (0x007F)        /* 0000 0000 0111 1111 */

/*
 * 74.  0x0F6A
 * VLD Debug Info 1
 */
#define REG_VIDEO_VLD_DEBUG_INFO_1_6AH          (REG_VIDEO_BASE_EXT + 0x006A)

/*
 * 75.  0x0F6C
 * VLD Debug Info 2
 */
#define REG_VIDEO_VLD_DEBUG_INFO_2_6CH          (REG_VIDEO_BASE_EXT + 0x006C)

/*
 * 76.  0x0F6E
 * VLD Debug Info 3
 */
#define REG_VIDEO_VLD_DEBUG_INFO_3_6EH          (REG_VIDEO_BASE_EXT + 0x006E)

/*
 * 77.  0x0F70
 * TC Debug Info 0
 */
#define REG_VIDEO_TC_DEBUG_INFO_0_70H           (REG_VIDEO_BASE_EXT + 0x0070)

/*
 * 78.  0x0F72
 * TC Debug Info 1
 */
#define REG_VIDEO_TC_DEBUG_INFO_1_72H           (REG_VIDEO_BASE_EXT + 0x0072)

/*
 * 79.  0x0F74
 * MC Debug Info
 */
#define REG_VIDEO_MC_DEBUG_INFO_74H             (REG_VIDEO_BASE_EXT + 0x0074)

/*
 * 80.  0x0F76
 * DB Debug Info
 */
#define REG_VIDEO_DB_DEBUG_INFO_76H             (REG_VIDEO_BASE_EXT + 0x0076)

/*
 * 81.  0x0F78
 * MVP Debug Info
 */
#define REG_VIDEO_MVP_DEBUG_INFO_78H            (REG_VIDEO_BASE_EXT + 0x0078)

/*
 * 82.  0x0F7A
 * Arbiter and Engine Status Debug Info
 */
#define REG_VIDEO_ARBITER_DEBUG_INFO_7AH        (REG_VIDEO_BASE_EXT + 0x007A)

/*
 * 83.  0x0F7C
 * Setup Debug Info
 */
#define REG_VIDEO_SETUP_DEBUG_INFO_7CH          (REG_VIDEO_BASE_EXT + 0x007C)

#define VIDEO_MASK_MACROBLOCK_Y_COORDINATE_S    (0x00FF)        /* 0000 0000 1111 1111 */
#define VIDEO_MASK_MACROBLOCK_X_COORDINATE_S    (0xFF00)        /* 1111 1111 0000 0000 */

/*
 * 84.  0x0F7E
 * TC Debug Info 3
 */
#define REG_VIDEO_TC_DEBUG_INFO_3_7EH           (REG_VIDEO_BASE_EXT + 0x007E)

/*
 * 85.  0x0F80
 * TC Debug Info 4
 */
#define REG_VIDEO_TC_DEBUG_INFO_4_80H           (REG_VIDEO_BASE_EXT + 0x0080)

#define VIDEO_MASK_MACROBLOCK_X_COORDINATE_T    (0x7F00)        /* 0111 1111 0000 0000 */
#define VIDEO_MASK_MACROBLOCK_Y_COORDINATE_T    (0x007F)        /* 0000 0000 0111 1111 */

/*
 * 86.  0x0F82
 * REC Debug Info
 */
#define REG_VIDEO_REC_DEBUG_INFO_82H            (REG_VIDEO_BASE_EXT + 0x0082)

/*
 * 87.  0x0F84
 * BIST Debug Info 0
 */
#define REG_VIDEO_BIST_DEBUG_INFO_0_84H         (REG_VIDEO_BASE_EXT + 0x0084)

/*
 * 88.  0x0F86
 * BIST Debug Info 1
 */
#define REG_VIDEO_BIST_DEBUG_INFO_1_86H         (REG_VIDEO_BASE_EXT + 0x0086)

/*
 * 89.  0x0F88
 * CABAC Debug Info 0
 */
#define REG_VIDEO_CABAC_DEBUG_INFO_0_88H        (REG_VIDEO_BASE_EXT + 0x0088)

/*
 * 90.  0x0F8A
 * CABAC Debug Info 1
 */
#define REG_VIDEO_CABAC_DEBUG_INFO_1_8AH        (REG_VIDEO_BASE_EXT + 0x008A)

/*
 * 91.  0x0F8C
 * CABAC Debug Info 2
 */
#define REG_VIDEO_CABAC_DEBUG_INFO_2_8CH        (REG_VIDEO_BASE_EXT + 0x008C)

/*
 * 92.  0x0F8E
 * CABAC Debug Info 3
 */
#define REG_VIDEO_CABAC_DEBUG_INFO_3_8EH        (REG_VIDEO_BASE_EXT + 0x008E)

/*
 * 0x0F08~0F42
 * Remap Address Table
 */
#define REG_VIDEO_REMAP_ADR_LUM_3_08H           (REG_VIDEO_BASE_EXT + 0x0008)
#define REG_VIDEO_REMAP_ADR_LUM_5_0AH           (REG_VIDEO_BASE_EXT + 0x000A)
#define REG_VIDEO_REMAP_ADR_LUM_7_0CH           (REG_VIDEO_BASE_EXT + 0x000C)
#define REG_VIDEO_REMAP_ADR_LUM_9_0EH           (REG_VIDEO_BASE_EXT + 0x000E)
#define REG_VIDEO_REMAP_ADR_LUM_11_10H          (REG_VIDEO_BASE_EXT + 0x0010)
#define REG_VIDEO_REMAP_ADR_LUM_13_12H          (REG_VIDEO_BASE_EXT + 0x0012)
#define REG_VIDEO_REMAP_ADR_LUM_15_14H          (REG_VIDEO_BASE_EXT + 0x0014)
#define REG_VIDEO_REMAP_ADR_LUM_17_16H          (REG_VIDEO_BASE_EXT + 0x0016)
#define REG_VIDEO_REMAP_ADR_LUM_19_18H          (REG_VIDEO_BASE_EXT + 0x0018)
#define REG_VIDEO_REMAP_ADR_LUM_21_1AH          (REG_VIDEO_BASE_EXT + 0x001A)
#define REG_VIDEO_REMAP_ADR_LUM_23_1CH          (REG_VIDEO_BASE_EXT + 0x001C)
#define REG_VIDEO_REMAP_ADR_LUM_25_1EH          (REG_VIDEO_BASE_EXT + 0x001E)
#define REG_VIDEO_REMAP_ADR_LUM_27_20H          (REG_VIDEO_BASE_EXT + 0x0020)
#define REG_VIDEO_REMAP_ADR_LUM_29_22H          (REG_VIDEO_BASE_EXT + 0x0022)
#define REG_VIDEO_REMAP_ADR_LUM_31_24H          (REG_VIDEO_BASE_EXT + 0x0024)
#define REG_VIDEO_REMAP_ADR_CHR_3_26H           (REG_VIDEO_BASE_EXT + 0x0026)
#define REG_VIDEO_REMAP_ADR_CHR_5_28H           (REG_VIDEO_BASE_EXT + 0x0028)
#define REG_VIDEO_REMAP_ADR_CHR_7_2AH           (REG_VIDEO_BASE_EXT + 0x002A)
#define REG_VIDEO_REMAP_ADR_CHR_9_2CH           (REG_VIDEO_BASE_EXT + 0x002C)
#define REG_VIDEO_REMAP_ADR_CHR_11_2EH          (REG_VIDEO_BASE_EXT + 0x002E)
#define REG_VIDEO_REMAP_ADR_CHR_13_30H          (REG_VIDEO_BASE_EXT + 0x0030)
#define REG_VIDEO_REMAP_ADR_CHR_15_32H          (REG_VIDEO_BASE_EXT + 0x0032)
#define REG_VIDEO_REMAP_ADR_CHR_17_34H          (REG_VIDEO_BASE_EXT + 0x0034)
#define REG_VIDEO_REMAP_ADR_CHR_19_36H          (REG_VIDEO_BASE_EXT + 0x0036)
#define REG_VIDEO_REMAP_ADR_CHR_21_38H          (REG_VIDEO_BASE_EXT + 0x0038)
#define REG_VIDEO_REMAP_ADR_CHR_23_3AH          (REG_VIDEO_BASE_EXT + 0x003A)
#define REG_VIDEO_REMAP_ADR_CHR_25_3CH          (REG_VIDEO_BASE_EXT + 0x003C)
#define REG_VIDEO_REMAP_ADR_CHR_27_3EH          (REG_VIDEO_BASE_EXT + 0x003E)
#define REG_VIDEO_REMAP_ADR_CHR_29_40H          (REG_VIDEO_BASE_EXT + 0x0040)
#define REG_VIDEO_REMAP_ADR_CHR_31_42H          (REG_VIDEO_BASE_EXT + 0x0042)      

#define REG_VIDEO_REMAP_ADR_ENABLE_44H          (REG_VIDEO_BASE_EXT + 0x0044)

#ifdef __cplusplus
extern }
#endif

#endif //_VIDEO_REGISTER_H_
