/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as Memory Stick (pro) Register Definition.
 *
 * @author Irene Lin
 */
#ifndef MS_REG_H
#define MS_REG_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "mspro/config.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MSPRO_BASE              ITH_MS_BASE // ITH_MS_BASE: 0xDE400000, HOST:0x8C00

#define MS_AHB_DATA_PORT        (ITH_MS_BASE + 0x10)

#define MS_TIMEOUT              1000
#define MS_BUSY_TIMEOUT         50000 * 4
//#define MS_BUSY_TIMEOUT 100*10  // ms
//#define MS_BUSY_DELAY   100

#define SMSP_TIMEOUT_READ_DATA  1005
#define SMSP_TIMEOUT_WRITE_DATA 1005
#define SMSP_TIMEOUT_READ_ATRB  1005
#define SMS_TIMEOUT_BLOCK_READ  5
#define SMS_TIMEOUT_BLOCK_WRITE 10
#define SMS_TIMEOUT_BLOCK_END_R 5
#define SMS_TIMEOUT_BLOCK_END_W 10
#define SMS_TIMEOUT_ERASE       100

#define MS_PAGE_SIZE            512
#define MS_SEGMENT_SIZE         512

typedef enum MS_INTERFACE_TAG
{
    MS_INTERFACE_SERIAL,
    MS_INTERFACE_PARALLEL
} MS_INTERFACE;

//=============================================================================
//                              Register Definition
//=============================================================================

//=============================================================================
/** 0x0000h
 * Version ID Register
 */
//=============================================================================
#define MS_REG_VERSION                   (MSPRO_BASE + 0x0000)

//=============================================================================
/** 0x0004h
 * Control Register
 */
//=============================================================================
#define MS_REG_CONTROL                   (MSPRO_BASE + 0x0004)

#define MS_MSK_FIFO_OVERRUN_THRE         0xFF000000
#define MS_MSK_FIFO_UNDERRUN_THRE        0x00FF0000
#define MS_MSK_SW_RESET                  0x00008000
#define MS_MSK_SLEEP_WAKEUP              0x00002000
#define MS_MSK_FIFO_CLR                  0x00001000
#define MS_MSK_RX_DMA_EN                 0x00000800
#define MS_MSK_TX_DMA_EN                 0x00000400
#define MS_MSK_GINT_WAIT_BS0_EN          0x00000200
#define MS_MSK_INT_2STATE_EN             0x00000100
#define MS_MSK_INT_CRC_CHECK_EN          0x00000080
#define MS_MSK_INT_FIFO_THRE_OVERRUN_EN  0x00000040
#define MS_MSK_INT_FIFO_THRE_UNDERRUN_EN 0x00000020
#define MS_MSK_INT_FIFO_OVERRUN_EN       0x00000010
#define MS_MSK_INT_FIFO_UNDERRUN_EN      0x00000008
#define MS_MSK_INT_CMD_COMPLETE_EN       0x00000004
#define MS_MSK_TRANSFER_EN               0x00000002
#define MS_MSK_MEMORY_STICK_EN           0x00000001

#define MS_SHT_FIFO_OVERRUN_THRE         24
#define MS_SHT_FIFO_UNDERRUN_THRE        16

//=============================================================================
/** 0x0008h
 * Interrupt Status Register
 */
//=============================================================================
#define MS_REG_INT_STATUS                (MSPRO_BASE + 0x0008)

#define MS_MSK_INT_CLR_ALL               0x00E70000
#define MS_MSK_INT_CLR_CRC_ERROR         0x00800000
#define MS_MSK_INT_CLR_2STATE_ERROR      0x00400000
#define MS_MSK_INT_CLR_CARD_DETECT       0x00200000
#define MS_MSK_INT_CLR_OVERRUN           0x00040000
#define MS_MSK_INT_CLR_UNDERRUN          0x00020000
#define MS_MSK_INT_CLR_CMD_COMPLETE      0x00010000

#define MS_MSK_INT_CRC_ERROR             0x00000080
#define MS_MSK_INT_2STATE_ERROR          0x00000040
#define MS_MSK_INT_CARD_DETECT           0x00000020
#define MS_MSK_INT_FIFO_THRE_OVERRUN     0x00000010
#define MS_MSK_INT_FIFO_THRE_UNDERRUN    0x00000008
#define MS_MSK_INT_FIFO_OVERRUN          0x00000004
#define MS_MSK_INT_FIFO_UNDERRUN         0x00000002
#define MS_MSK_INT_CMD_COMPLETE          0x00000001

//=============================================================================
/** 0x000Ch
 * Command Register
 */
//=============================================================================
#define MS_REG_COMMAND                   (MSPRO_BASE + 0x000C)

// TPC command
/** for MS and MSpro card */
#define MS_MSK_TPC_CMD                   0xFF000000
#define MS_TPC_READ_PAGE_DATA            0x2D000000         // Transfer data from data buffer 512 byte
#define MS_TPC_READ_REG                  0x4B000000         // Read register
#define MS_TPC_GET_INT                   0x78000000         // Read INT register
#define MS_TPC_WRITE_PAGE_DATA           0xD2000000         // Transfer data to data buffer 512 byte
#define MS_TPC_WRITE_REG                 0xB4000000         // Write register
#define MS_TPC_SET_REG_ADRS              0x87000000         // Address setting of Read_Reg/Write_Reg
#define MS_TPC_SET_CMD                   0xE1000000         // Set CMD
/** only for MSpro card */
#define MSP_TPC_READ_LONG_DATA           0x2D000000         // Transfer data from data buffer 512 byte
#define MSP_TPC_READ_SHORT_DATA          0x3C000000         // Transfer data from data buffer 32 ~ 256 byte
#define MSP_TPC_WRITE_LONG_DATA          0xD2000000         // Transfer data to data buffer 512 byte
#define MSP_TPC_WRITE_SHORT_DATA         0xC3000000         // Transfer data to data buffer 32 ~ 256 byte
#define MSP_TPC_EX_SET_CMD               0x96000000         // Set CMD and parameters

#define MS_MSK_CLK_INVERT                0x00080000
#define MS_MSK_PARALLEL                  0x00040000

// data size
#define MS_MSK_CMD_DATA_SIZE             0x00030000
#define MS_SHT_CMD_DATA_SIZE             16
#define Qualet_1_Byte                    0x0
#define Qualet_2_Byte                    0x1
#define Qualet_4_Byte                    0x3

// data length
#define MS_TPC_LEN_READ_REG(nBytes)      nBytes             //1~31 bytes
#define MS_TPC_LEN_GET_INT               1
#define MS_TPC_LEN_WRITE_REG(nBytes)     nBytes             //1~31 bytes
#define MS_TPC_LEN_SET_REG_ADRS          4
#define MS_TPC_LEN_SET_CMD               1

//=============================================================================
/** 0x0010h
 * Data Register
 */
//=============================================================================
#define MS_REG_DATA                      (MSPRO_BASE + 0x0010)

//=============================================================================
/** 0x0014h
 * Status Register
 */
//=============================================================================
#define MS_REG_STATUS                    (MSPRO_BASE + 0x0014)

#define MS_MSK_FIFO_VALID_ENTRIES        0x00FF0000
#define MS_MSK_GET_INT                   0x00004000
#define MS_MSK_SET_FLASH_CTRL_CMD        0x00002000
#define MS_MSK_SET_RW_ADR                0x00001000
#define MS_MSK_WRITE_REG                 0x00000800
#define MS_MSK_WRITE_PAGE_BUF            0x00000400
#define MS_MSK_READ_REG                  0x00000200
#define MS_MSK_READ_PAGE_BUG             0x00000100
#define MS_MSK_2STATE_ACCESS_MODE        0x00000040
#define MS_MSK_CRC_ERROR                 0x00000020
#define MS_MSK_TPC_CHECK_ERROR           0x00000010
#define MS_MSK_FIFO_FULL                 0x00000008
#define MS_MSK_FIFO_EMPTY                0x00000004
#define MS_MSK_TRANS_BUSY                0x00000002
#define MS_MSK_CARD_DETECT               0x00000001

//=============================================================================
/** 0x0018h
 * Serial Clock Divide Register
 */
//=============================================================================
#define MS_REG_SERIAL_CLK_DIVIDE         (MSPRO_BASE + 0x0018)

//=============================================================================
/** 0x001Ch
 * Bus Status Monitor Register
 */
//=============================================================================
#define MS_REG_BUS_MONITOR_STATUS        (MSPRO_BASE + 0x001C)

#define MS_MSK_FIFO_SIZE                 0xFF000000
#define MS_MSK_FIFO_WIDTH                0x00FF0000
#define MS_MSK_CMD_COMPLETE              0x00002000
#define MS_MSK_MS_CS                     0x00001C00
#define MS_MSK_SCK                       0x00000200
#define MS_MSK_SD_OUT                    0x000001E0
#define MS_MSK_SD_IN                     0x0000001E
#define MS_MSK_BUS_STATE                 0x00000001

/** MS_MSK_SD_IN: paralle INT status */
#if 0
    #define MS_MSK_INT_CED_P             (1 << 16)
    #define MS_MSK_INT_ERR_P             (1 << 17)
    #define MS_MSK_INT_BREQ_P            (1 << 18)
    #define MS_MSK_INT_CMDNK_P           (1 << 19)
#endif
#if 1
    #define MS_MSK_INT_CED_P             (1 << 20)
    #define MS_MSK_INT_ERR_P             (1 << 21)
    #define MS_MSK_INT_BREQ_P            (1 << 22)
    #define MS_MSK_INT_CMDNK_P           (1 << 23)
#endif
#if 0
    #define MS_MSK_INT_CED_P             0x00000001
    #define MS_MSK_INT_ERR_P             0x00000002
    #define MS_MSK_INT_BREQ_P            0x00000004
    #define MS_MSK_INT_CMDNK_P           0x00000008
#endif

#define MS_SHT_FIFO_SIZE                 24
#define MS_SHT_FIFO_WIDTH                16

//============================================
/**
 * Memory Stick Pro Spec.
 * SMSP: S => Spec.
 *       M => Memory
 *       S => Stick
 *       P => Pro
 */
//============================================
/**
 * Memory Access Command
 */
#define SMSP_MEM_READ_DATA   0x20
#define SMSP_MEM_WRITE_DATA  0x21
#define SMSP_MEM_READ_ATTRIB 0x24
#define SMSP_MEM_STOP        0x25
#define SMSP_MEM_ERASE       0x26
#define SMSP_MEM_SET_IBD     0x46
#define SMSP_MEM_GET_IBD     0x47

/**
 * Status Register (same with Memory Stick)
 */
#define SMS_INT_CED_S        0x80
#define SMS_INT_ERR_S        0x40
#define SMS_INT_BREQ_S       0x20
#define SMS_INT_CMDNK_S      0x01

//============================================
/**
 * Memory Stick Spec.
 * SMS: S => Spec.
 *      M => Memory
 *      S => Stick
 */
//============================================
/**
 * Memory Access Command
 */
#define SMS_MEM_BLOCK_READ  0xAA
#define SMS_MEM_BLOCK_WRITE 0x55
#define SMS_MEM_BLOCK_END   0x33
#define SMS_MEM_ERASE       0x99
#define SMS_MEM_FLASH_STOP  0xCC

/**
 * Function Command
 */
#define SMS_FUNC_SLEEP      0x5A
#define SMS_FUNC_CLEAR_BUF  0xC3
#define SMS_FUNC_RESET      0x3C

//===================
// For Register
//===================
/**
 * 02h: Status Register0
 */
#define SMS_MSK_WRITE_PROTECT             0x01
#define SMS_MSK_SLEEP                     0x02

/**
 * 03h: Status Register1
 */
#define SMS_MSK_UNABLE_CORRECT_DATA       0x10
#define SMS_MSK_UNABLE_CORRECT_EXTRA_DATA 0x04
#define SMS_MSK_UNABLE_CORRECT_FLAG       0x01

/**
 * 14h: Command parameter register. setting for command execution
 */
#define SMS_CMD_BLOCK_ACCESS_MODE         0x00
#define SMS_CMD_SINGLE_PAGE_ACCESS_MODE   0x20
#define SMS_CMD_EXTRA_DATA_ACCESS_MODE    0x40
#define SMS_CMD_OVERWRITE_ACCESS_MODE     0x80

/**
 * 16h: Overwrite Register
 */
#define SMS_MSK_BLOCK_STATUS              0x80
#define SMS_MSK_PAGE_STATUS               0x60
#define SMS_MSK_UPDATE_STATUS             0x10

#define SMS_PAGE_STATUS_DATA_ERROR        0
#define SMS_PAGE_STATUS_NG                1
#define SMS_PAGE_STATUS_OK                3

/** For overwrite the overwrite flag */
enum
{
    SMS_OWFLAG_BLOCK_NG,
    SMS_OWFLAG_PAGE_NG,
    SMS_OWFLAG_DATA_ERROR,
    SMS_OWFLAG_USED_OR_UPDATING
};

/**
 * 17h: Management Register
 */
#define SMS_MSK_SYSTEM_BIT           0x04        /** 1: Other than Boot Block, 0: Boot Block */
#define SMS_MSK_CONVERSION_TABLE_BIT 0x08

#ifdef __cplusplus
}
#endif

#endif