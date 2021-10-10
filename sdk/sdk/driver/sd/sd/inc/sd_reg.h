/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as SD Register Definition.
 *
 * @author Irene Lin
 */
#ifndef SD_REG_H
#define SD_REG_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "config.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define SD_BASE                        ITH_SD_BASE /* APB Map: 0xDED00000,  HOST Map: 0xB000  */

#define SD_SECTOR_SIZE                 512
#define SD_FIFO_WIDTH                  4
#define SD_FIFO_DEPTH                  128

//=============================================================================
//                       SD Controller Register Definition
//=============================================================================
#define SD_AHB_DATA_PORT               (ITH_SD_BASE + 0x80)

//=============================================================================
/** 0x0000h
 *  MMC Interface Setting & Status Register0
 */
//=============================================================================
#define SD_REG_STS0                    (SD_BASE + 0x0000)

#define SD_MSK_IF_RESET                0x00000001
#define SD_MSK_CRC_BYPASS              0x00000002
#define SD_MSK_CRC_NON_FIX             0x00000004
#define SD_MSK_ASYN_CLK_SEL            0x00000008
#define SD_MSK_CLK_SRC                 0x00000010
#define SD_MSK_HIGH_SPEED              0x00000020
#define SD_MSK_BUS_WIDTH               0x000000C0

#define SD_BUS_WIDTH_1BIT              0x00000000
#define SD_BUS_WIDTH_4BIT              0x00000040
#define SD_BUS_WIDTH_8BIT              0x00000080

//=============================================================================
/** 0x0004h
 *  MMC Interface Setting & Status Register1
 */
//=============================================================================
#define SD_REG_STS1                    (SD_BASE + 0x0004)

#if defined(SD_NEW_HW)
    #define SD_MSK_RESP_TIMEOUT_BYPASS 0x00001000
    #define SD_MSK_RESP_CRC_BYPASS     0x00000800
    #define SD_MSK_CRC_READ_BYPASS     0x00000400
    #define SD_MSK_CRC_WRITE_BYPASS    0x00000200
    #define SD_MSK_D0_STATUS_HIGH      0x00000020
    #define SD_MSK_RESP_TIMEOUT        0x00000010
    #define SD_MSK_RESP_CRC            0x00000008
    #define SD_MSK_CRC_READ            0x00000004
    #define SD_MSK_CRC_WRITE           0x00000002
    #define SD_MSK_CMD_END             0x00000001
    #define SD_ERROR                   0x1E
#else // #if defined(SD_NEW_HW)
    #define SD_MSK_CRC_ERROR           0x00000001
    #define SD_MSK_CRC_READ            0x00000002
    #define SD_MSK_CRC_WRITE           0x00000004
    #define SD_MSK_CMD_RESP_TIMEOUT    0x00000008
    #define SD_MSK_CRC_CMD             0x00000010
    #define SD_MSK_D0_STATUS_HIGH      0x00000020
    #define SD_MSK_INTR_CLR            0x00000100
    #define SD_MSK_INTR_MSK            0x00008000
    #define SD_ERROR                   0x07
#endif // #if defined(SD_NEW_HW)

//=============================================================================
/** 0x0008h
 */
//=============================================================================
#define SD_REG_CTL         (SD_BASE + 0x0008)

#define SD_MSK_CMD_TRIGGER 0x00000001
#define SD_MSK_AUTO_SWAP   0x00000002
#define SD_MSK_RESP_TYPE   0x0000000C
#define SD_MSK_CMD_TYPE    0x00000030
#define SD_MSK_CLK_CTRL    0x00000040

enum
{
    SD_RESP_TYPE_NON     = 0x00,
    SD_RESP_TYPE_48      = 0x04,
    SD_RESP_TYPE_48_BUSY = 0x08,
    SD_RESP_TYPE_136     = 0x0C
};

enum
{
    SD_CMD_NO_DATA  = 0x00,
    SD_CMD_RESERVED = 0x10,
    SD_CMD_DATA_OUT = 0x20,
    SD_CMD_DATA_IN  = 0x30
};

//=============================================================================
/** 0x000Ch ~ 0x0018h
 */
//=============================================================================
#define SD_REG_SECTOR_COUNT_L (SD_BASE + 0x000C)
#define SD_REG_SECTOR_COUNT_H (SD_BASE + 0x0014)
#define SD_REG_CLK_DIV        (SD_BASE + 0x0010)
#define SD_REG_COMMAND        (SD_BASE + 0x0018)

#if 0 /** only USBest will use it */
enum
{
    SD_OP_NORMAL      = 0x00000000,
    SD_OP_STANDARD_SD = 0x00000040,
    SD_OP_SDHC        = 0x00000080
};
#endif

#define SD_MSK_OP   0x000000C0
#define SD_MSK_CMD  0x0000003F

//=============================================================================
/** 0x001Ch ~ 0x0028h
 *  Command Argument
 */
//=============================================================================
#define SD_REG_ARG0 (SD_BASE + 0x001C)
#define SD_REG_ARG1 (SD_BASE + 0x0020)
#define SD_REG_ARG2 (SD_BASE + 0x0024)
#define SD_REG_ARG3 (SD_BASE + 0x0028)

#if defined(SD_NEW_HW)
//=============================================================================
/** 0x002Ch
 *  Interrupt
 */
//=============================================================================
    #define SD_REG_INTR     (SD_BASE + 0x002C)
    #define SD_SHT_INTR_MSK 8
enum {
    SD_INTR_CMD_END      = 0x01,
    SD_INTR_WRITE_CRC    = 0x02,
    SD_INTR_READ_CRC     = 0x04,
    SD_INTR_RESP_CRC     = 0x08,
    SD_INTR_RESP_TIMEOUT = 0x10
};
    #define SD_INTR_ALL 0x1F
    #define SD_INTR_ERR 0x1E
#endif

//=============================================================================
/** 0x0030h ~ 0x0070h
 *  Command Response
 */
//=============================================================================
#define SD_REG_RESP_7_0     (SD_BASE + 0x0030)
#define SD_REG_RESP_15_8    (SD_BASE + 0x0034)
#define SD_REG_RESP_23_16   (SD_BASE + 0x0038)
#define SD_REG_RESP_31_24   (SD_BASE + 0x003C)
#define SD_REG_RESP_39_32   (SD_BASE + 0x0040)
#define SD_REG_RESP_47_40   (SD_BASE + 0x0044)
#define SD_REG_RESP_55_48   (SD_BASE + 0x0048)
#define SD_REG_RESP_63_56   (SD_BASE + 0x004C)
#define SD_REG_RESP_71_64   (SD_BASE + 0x0050)
#define SD_REG_RESP_79_72   (SD_BASE + 0x0054)
#define SD_REG_RESP_87_80   (SD_BASE + 0x0058)
#define SD_REG_RESP_95_88   (SD_BASE + 0x005C)
#define SD_REG_RESP_103_96  (SD_BASE + 0x0060)
#define SD_REG_RESP_111_104 (SD_BASE + 0x0064)
#define SD_REG_RESP_119_112 (SD_BASE + 0x0068)
#define SD_REG_RESP_127_120 (SD_BASE + 0x006C)
#define SD_REG_RESP_135_128 (SD_BASE + 0x0070)

//=============================================================================
/** 0x0074h ~ 0x0078h
 *  Data Sector Length
 */
//=============================================================================
#if defined(SD_NEW_HW)
    #define SD_REG_LENGTH       (SD_BASE + 0x0074)
    #define SD_REG_RESP_TIMEOUT (SD_BASE + 0x0078)
#else
    #define SD_REG_LENGTH_H     (SD_BASE + 0x0074)
    #define SD_REG_LENGTH_L     (SD_BASE + 0x0078)
#endif

//========================================
// Smedia Wrap Register
//========================================
#define SDW_REG_DATA_LEN       (SD_BASE + 0x007C)
#define SDW_REG_DATA_PORT      (SD_BASE + 0x0080)

#define SDW_REG_WRAP_STATUS    (SD_BASE + 0x0084)
#if defined(SD_NEW_HW)
    #define SDW_MSK_INTR_MASK  0x80000000
    #define SDW_INTR_WRAP_END  0x01000000
#endif
#define SDW_MSK_FIFO_CNT       0x00000FF0

#define SDW_MSK_FIFO_EMPTY     0x00000002
#define SDW_MSK_FIFO_FULL      0x00000001

#define SDW_REG_WRAP_CTRL      (SD_BASE + 0x0088)
#define SDW_MSK_DATA_IN        0x00000040
#define SDW_MSK_ENDIAN_EN      0x00000020
#define SDW_MSK_HW_HANDSHAKING 0x00000010
#define SDW_MSK_SW_RESET       0x00000004
#define SDW_MSK_CLEAR_FIFO     0x00000002
#define SDW_MSK_WRAP_FIRE      0x00000001

enum
{
    SDW_DATA_IN,
    SDW_DATA_OUT
};
enum
{
    SDW_NON_DMA,
    SDW_DMA
};

//=============================================================================
//                         SD Card(SPEC) Register Definition
//=============================================================================
#define RESP_CRC                       0x80
#define RESP_NON                       0x40
/**
 * Command
 */
#define COM_BC_GO_IDLE_STATE           (0 | RESP_NON)
#define MMC_BCR_SEND_OP_COND           1
#define COM_BCR_ALL_SEND_CID           2
#define MMC_BCR_SET_RELATIVE_ADDR      (3 | RESP_CRC)
#define SD_BCR_SEND_RELATIVE_ADDR      (3 | RESP_CRC)
#define COM_BC_SET_DSR                 4
#define SD_ADTC_SWITCH_FUNCTION        (6 | RESP_CRC)
#define MMC_AC_SWITCH                  (6 | RESP_CRC)
#define COM_AC_SELECT_DESELECT_CARD    (7 | RESP_CRC)
#define SD_BCR_SEND_IF_COND            (8 | RESP_CRC)
#define MMC_ADTC_SEND_EXT_CSD          (8 | RESP_CRC)

#define COM_AC_SEND_CSD                9
#define COM_AC_SEND_CID                10
#define MMC_AC_READ_DATA_UNTIL_STOP    11
#define COM_AC_STOP_TRANSMISSION       (12 | RESP_CRC)
#define COM_AC_SEND_STATUS             (13 | RESP_CRC)
#define MMC_ADTC_BUSTEST_R             (14 | RESP_CRC)
#define COM_AC_GO_INACTIVE_STATE       15
#define COM_AC_SET_BLOCK_LEN           (16 | RESP_CRC)
#define COM_ADTC_READ_SINGLE_BLOCK     (17 | RESP_CRC)
#define COM_ADTC_READ_MULTIPLE_BLOCK   (18 | RESP_CRC)
#define MMC_ADTC_BUSTEST_W             (19 | RESP_CRC)
#define MMC_ADTC_WRITE_DATA_UNTIL_STOP 20
#define MMC_AC_SET_BLOCK_COUNT         (23 | RESP_CRC)
#define COM_ADTC_WRITE_SINGLE_BLOCK    (24 | RESP_CRC)
#define COM_ADTC_WRITE_MULTIPLE_BLOCK  (25 | RESP_CRC)
#define MMC_ADTC_PROGRAM_CID           (26 | RESP_CRC)
#define COM_ADTC_PROGRAM_CSD           (27 | RESP_CRC)
#define COM_AC_SET_WRITE_PROT          (28 | RESP_CRC)
#define COM_AC_CLR_WRITE_PROT          (29 | RESP_CRC)
#define MMC_ADTC_SEND_WRITE_PROT       (30 | RESP_CRC)
#define SD_ACMD_AC_SET_BUS_WIDTH       (6 | RESP_CRC)
#define SD_ACMD_BCR_SEND_OP_COND       41
#define SD_ACMD_ADTC_SEND_SCR          (51 | RESP_CRC)
#define SD_AC_APP_CMD                  (55 | RESP_CRC)

#define SD_CMD(x)                      (x & 0x3F)

/** Check Pattern */
#define STUFF_BITS_55                  0x55
#define STUFF_BITS_AA                  0xAA

/** Some definition */
#define SD_HOST_SDHC                   0x40
#define SD_BUS_4BITS                   0x02
#define MMC_HOST_HC                    0x40 // Irene: ??

/** SD CMD6: switch function */
#define SD_FUN_CHECK_MODE              0x00
#define SD_FUN_SWITCH_MODE             0x80
#define SD_FUN_HIGH_SPEED              0x01

/** MMC CMD6: switch */
#define ACCESS_WRITE_BYTE              0x3
#define HS_TIMING_INDEX                185
#define MMC_SPEED                      1

#define BUS_WIDTH_INDEX                183
#define MMC_BUS_WIDTH_8                2
#define MMC_BUS_WIDTH_4                1
#define MMC_BUS_WIDTH_1                0

#ifdef __cplusplus
}
#endif

#endif