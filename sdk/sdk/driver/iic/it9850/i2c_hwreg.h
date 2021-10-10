/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * I2C Registers
 *
 * @author Sammy Chen
 * @version 0.1
 */
#ifndef I2C_HWREG_H
#define I2C_HWREG_H
//#include "host\ahb.h"

#include "ite/ith_defs.h"

/*Memory Base is only for CPU direct mmapping*/
#define REG_I2C_CONTROL                    (0x00)
#define REG_I2C_STATUS                     (0x04)
#define REG_I2C_CLOCK_DIV                  (0x08)
#define REG_I2C_DATA                       (0x0C)
#define REG_I2C_SLAVE_ADDR                 (0x10)
#define REG_I2C_ICR                        (0x1C)
#define REG_I2C_GLITCH                     (0x14)
#define REG_I2C_BUS_MONITOR                (0x18)
#define REG_I2C_REV_NUMBER                 (0x30)
#define REG_I2C_FEATURE                    (0x34)

/*ICR Register*/
#define REG_BIT_INTR_TRIG_START            (1u << 14)
#define REG_BIT_INTR_TRIG_ARBITRATION_LOSS (1u << 13)
#define REG_BIT_INTR_TRIG_SLAVE_SELECT     (1u << 12)
#define REG_BIT_INTR_TRIG_STOP             (1u << 11)
#define REG_BIT_INTR_TRIG_BUS_ERROR        (1u << 10)
#define REG_BIT_INTR_TRIG_DATA_RECEIVE     (1u << 9)
#define REG_BIT_INTR_TRIG_DATA_TRANSFER    (1u << 8)

/*Control Register*/
#define REG_BIT_CONTL_FIFO_EN              (1u << 8)
#define REG_BIT_CONTL_TRANSFER_BYTE        (1u << 7)
#define REG_BIT_CONTL_NACK                 (1u << 6)
#define REG_BIT_CONTL_STOP                 (1u << 5)
#define REG_BIT_CONTL_START                (1u << 4)
#define REG_BIT_CONTL_GC                   (1u << 3)
#define REG_BIT_CONTL_CLK_ENABLE           (1u << 2)
#define REG_BIT_CONTL_I2C_ENABLE           (1u << 1)
#define REG_BIT_CONTL_I2C_RESET            (1u << 0)

#define REG_BIT_INTR_TRIG                  REG_BIT_INTR_TRIG_ARBITRATION_LOSS | REG_BIT_INTR_TRIG_SLAVE_SELECT | \
                                           REG_BIT_INTR_TRIG_BUS_ERROR | REG_BIT_INTR_TRIG_DATA_RECEIVE | \
                                           REG_BIT_INTR_TRIG_DATA_TRANSFER

#define REG_BIT_INTR_ALL                   REG_BIT_INTR_TRIG_START | \
                                           REG_BIT_INTR_TRIG_ARBITRATION_LOSS | \
                                           REG_BIT_INTR_TRIG_SLAVE_SELECT | \
                                           REG_BIT_INTR_TRIG_STOP | \
                                           REG_BIT_INTR_TRIG_BUS_ERROR | \
                                           REG_BIT_INTR_TRIG_DATA_RECEIVE | \
                                           REG_BIT_INTR_TRIG_DATA_TRANSFER

/*Status Register*/
#define REG_BIT_STATUS_START               (1u << 11)
#define REG_BIT_STATUS_ARBITRATION_LOSS    (1u << 10)
#define REG_BIT_STATUS_SLAVE_SELECT        (1u << 8)
#define REG_BIT_STATUS_STOP                (1u << 7)
#define REG_BIT_STATUS_BUS_ERROR           (1u << 6)
#define REG_BIT_STATUS_DATA_RECEIVE_DONE   (1u << 5)
#define REG_BIT_STATUS_DATA_TRANSFER_DONE  (1u << 4)
#define REG_BIT_STATUS_NON_ACK             (1u << 1)
#define REG_BIT_STATUS_RECEIVE_MODE        (1u << 0)

/*Data Register*/
#define REG_BIT_READ_ENABLE                (1u << 0)

/*Clcok Division Register*/
#define REG_MASK_CLK_DIV_COUNT             (0x3FFFF)

/*Hold Time & Glitch Suppression Setting Register*/
#define REG_MASK_GSR                       (0x1C00)
#define REG_SHIFT_GSR                      (10)
#define REG_MASK_TSR                       (0x3FF)

#endif