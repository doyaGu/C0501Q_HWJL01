/*
 * Copyright (c) 2006 SMedia Technology Corp. All Rights Reserved.
 */
/* @file
 * Include file for GPIO Control
 *
 * @author Steven Hsiao
 * @date 2017.09.11.
 * @version 1.0
 *
 */
#define GPIO_BASE               0xDE000000
#define GPIO_PINDIR_OFFSET      0x08
#define GPIO_MODE_OFFSET        0x100
#define GPIO_DATASET_OFFSET     0xC
#define GPIO_DATACLR_OFFSET     0x10
#define GPIO_DATAIN_OFFSET      0x4
#define GPIO_DATAOUT_OFFSET     0x0

#define MMIO_BASE               0xC0200000

#define GPIO_DIR_BASE       (GPIO_BASE + GPIO_PINDIR_OFFSET)
#define GPIO_MODE_BASE      (GPIO_BASE + GPIO_MODE_OFFSET)
#define GPIO_SET_BASE       (GPIO_BASE + GPIO_DATASET_OFFSET)
#define GPIO_CLEAR_BASE     (GPIO_BASE + GPIO_DATACLR_OFFSET)
#define GPIO_IN_DATA_BASE   (GPIO_BASE + GPIO_DATAIN_OFFSET)
#define GPIO_OUT_DATA_BASE  (GPIO_BASE + GPIO_DATAOUT_OFFSET)

static __inline void RegWrite(unsigned long addr, unsigned long data) {
    *(volatile unsigned long *) (addr) = data;
}

static __inline unsigned long RegRead(unsigned long addr) {
    return *(volatile unsigned long *) (addr);
}

static __inline void RegWriteMask(unsigned long addr, unsigned long data, unsigned long mask) {
    RegWrite(addr, ((RegRead(addr) & ~mask) | (data & mask)));
}

static __inline void RegWriteH(unsigned short addr, unsigned short data) {
    *(volatile unsigned short *) (addr + MMIO_BASE) = data;
}



void setGpioDir(unsigned long gpioPin, unsigned long bIn)
{
    unsigned long regAddr = 0;
    unsigned long bitOffset = 0;
    regAddr = GPIO_DIR_BASE + ((gpioPin >> 5) * 0x40);
    bitOffset = (gpioPin & 0x1F);
    if (bIn)
    {
        RegWriteMask(regAddr, (0x0 << bitOffset), (0x1 << bitOffset));
    }
    else
    {
        RegWriteMask(regAddr, (0x1 << bitOffset), (0x1 << bitOffset));
    }
}

void setGpioMode(unsigned long gpioPin, unsigned long mode)
{
    unsigned long regAddr = 0;
    unsigned long bitOffset = 0;
    regAddr = GPIO_MODE_BASE + ((gpioPin >> 4) * 0x4);
    bitOffset = ((gpioPin & 0xF) << 1);
    RegWriteMask(regAddr, (mode << bitOffset), (0x3 << bitOffset));
}

unsigned long getGpioValue(unsigned long gpioPin, unsigned long bIn) 
{
    unsigned long regAddr = 0;
    unsigned long bitOffset = 0;
    unsigned long gpioValue = 0;
    if (bIn)
    {
        regAddr = GPIO_IN_DATA_BASE + ((gpioPin >> 5) * 0x40);
    }
    else
    {
        regAddr = GPIO_OUT_DATA_BASE + ((gpioPin >> 5) * 0x40);
    }
    bitOffset = (gpioPin & 0x1F);

    gpioValue = RegRead(regAddr);
    if ((gpioValue & (0x1 << bitOffset)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void setGpioValue(unsigned long gpioPin, unsigned long bHigh)
{
    unsigned long regAddr = 0;
    unsigned long bitOffset = 0;
    if (bHigh)
    {
        regAddr = GPIO_SET_BASE + ((gpioPin >> 5) * 0x40);
    }
    else
    {
        regAddr = GPIO_CLEAR_BASE + ((gpioPin >> 5) * 0x40);
    }
    bitOffset = (gpioPin & 0x1F);
    RegWrite(regAddr, (0x1 << bitOffset));
}
