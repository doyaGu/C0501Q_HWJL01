/*
 * Copyright(c) 2015 ITE Tech.Inc.All Rights Reserved.
 */
/** @file
 * HAL GPIO functions.
 *
 * @author Jim Tan
 * @author Benson Lin
 * @author I-Chun Lai
 * @version 1.0
 */
#include "../ith_cfg.h"

static uint32_t gpioRegs[(ITH_GPIO_REV_REG - ITH_GPIO1_PINDIR_REG + 4) / 4];

#define DEFINE_REG_ADDR_TABLE4(FUNC) static const uint32_t FUNC ## _REG_ADDR[4] =   \
{                                                                                   \
    ITH_GPIO_BASE + ITH_GPIO1_ ## FUNC ## _REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO2_ ## FUNC ## _REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO3_ ## FUNC ## _REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO4_ ## FUNC ## _REG,                                     \
};

#define DEFINE_REG_ADDR_TABLE7(FUNC) static const uint32_t FUNC ## _REG_ADDR[7] =   \
{                                                                                   \
    ITH_GPIO_BASE + ITH_GPIO1_ ## FUNC ##_L_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO1_ ## FUNC ##_H_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO2_ ## FUNC ##_L_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO2_ ## FUNC ##_H_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO3_ ## FUNC ##_L_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO3_ ## FUNC ##_H_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO4_ ## FUNC ##_REG,                                     \
};

#define DEFINE_REG_ADDR_TABLE16(FUNC) static const uint32_t FUNC ## _REG_ADDR[16] =   \
{                                                                                     \
    ITH_GPIO_BASE + ITH_GPIO1_ ## FUNC ##_G1_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO1_ ## FUNC ##_G2_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO1_ ## FUNC ##_G3_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO1_ ## FUNC ##_G4_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO2_ ## FUNC ##_G1_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO2_ ## FUNC ##_G2_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO2_ ## FUNC ##_G3_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO2_ ## FUNC ##_G4_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO3_ ## FUNC ##_G1_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO3_ ## FUNC ##_G2_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO3_ ## FUNC ##_G3_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO3_ ## FUNC ##_G4_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO4_ ## FUNC ##_G1_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO4_ ## FUNC ##_G2_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO4_ ## FUNC ##_G3_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO4_ ## FUNC ##_G4_REG,                                     \
};



DEFINE_REG_ADDR_TABLE4(PINDIR)
DEFINE_REG_ADDR_TABLE4(DATASET)
DEFINE_REG_ADDR_TABLE4(DATACLR)
DEFINE_REG_ADDR_TABLE4(DATAIN)
DEFINE_REG_ADDR_TABLE4(INTREN)
DEFINE_REG_ADDR_TABLE4(INTRCLR)
DEFINE_REG_ADDR_TABLE4(BOUNCEEN)
DEFINE_REG_ADDR_TABLE4(PULLEN)
DEFINE_REG_ADDR_TABLE4(PULLTYPE)
DEFINE_REG_ADDR_TABLE4(INTRTRIG)
DEFINE_REG_ADDR_TABLE4(INTRBOTH)
DEFINE_REG_ADDR_TABLE4(INTRRISENEG)
DEFINE_REG_ADDR_TABLE16(MODESEL)
DEFINE_REG_ADDR_TABLE7(DRIVING_SET)


void ithGpioSetMode(unsigned int pin, ITHGpioMode mode)
{
    uint32_t value, mask;

    ithEnterCritical();

    switch (mode)
    {
    // for UART0 output
    case ITH_GPIO_MODE_TX0:
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_URSEL1_REG, ITH_GPIO_URTX_EN_BIT);
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_URSEL1_REG, pin << ITH_GPIO_URTX_BIT, ITH_GPIO_URTX_MASK);
        mode = 0;
        break;

    case ITH_GPIO_MODE_RX0:
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_URSEL1_REG, ITH_GPIO_URRX_EN_BIT);
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_URSEL1_REG, pin << ITH_GPIO_URRX_BIT, ITH_GPIO_URRX_MASK);
        mode = 0;
        break;

    // for UART1 output
    case ITH_GPIO_MODE_TX1:
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_URSEL2_REG, ITH_GPIO_URTX_EN_BIT);
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_URSEL2_REG, pin << ITH_GPIO_URTX_BIT, ITH_GPIO_URTX_MASK);
        mode = 0;
        break;

    case ITH_GPIO_MODE_RX1:
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_URSEL2_REG, ITH_GPIO_URRX_EN_BIT);
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_URSEL2_REG, pin << ITH_GPIO_URRX_BIT, ITH_GPIO_URRX_MASK);
        mode = 0;
        break;

    // for UART2 output
    case ITH_GPIO_MODE_TX2:
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_URSEL3_REG, ITH_GPIO_URTX_EN_BIT);
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_URSEL3_REG, pin << ITH_GPIO_URTX_BIT, ITH_GPIO_URTX_MASK);
        mode = 0;
        break;

    case ITH_GPIO_MODE_RX2:
        ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO_MISC0_REG, ITH_GPIO_UR2_RXSRC_BIT);
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_URSEL3_REG, ITH_GPIO_URRX_EN_BIT);
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_URSEL3_REG, pin << ITH_GPIO_URRX_BIT, ITH_GPIO_URRX_MASK);
        mode = 0;
        break;

    case ITH_GPIO_MODE_RX2WGAND:
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_MISC0_REG, ITH_GPIO_UR2_RXSRC_BIT);
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_URSEL3_REG, ITH_GPIO_URRX_EN_BIT);
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_URSEL3_REG, pin << ITH_GPIO_URRX_BIT, ITH_GPIO_URRX_MASK);
        mode = 0;
        break;

    // for UART3 output
    case ITH_GPIO_MODE_TX3:
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_URSEL4_REG, ITH_GPIO_URTX_EN_BIT);
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_URSEL4_REG, pin << ITH_GPIO_URTX_BIT, ITH_GPIO_URTX_MASK);
        mode = 0;
        break;

    case ITH_GPIO_MODE_RX3:
        ithClearRegBitA(ITH_GPIO_BASE + ITH_GPIO_MISC0_REG, ITH_GPIO_UR3_RXSRC_BIT);
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_URSEL4_REG, ITH_GPIO_URRX_EN_BIT);
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_URSEL4_REG, pin << ITH_GPIO_URRX_BIT, ITH_GPIO_URRX_MASK);
        mode = 0;
        break;

    case ITH_GPIO_MODE_RX3WGAND:
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_MISC0_REG, ITH_GPIO_UR3_RXSRC_BIT);
        ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_URSEL4_REG, ITH_GPIO_URRX_EN_BIT);
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_URSEL4_REG, pin << ITH_GPIO_URRX_BIT, ITH_GPIO_URRX_MASK);
        mode = 0;
        break;
    }

    value = mode << ((pin & 0x7) * 4);
    mask  = 0x0F << ((pin & 0x7) * 4);
    if (pin < 96)
    {
        ithWriteRegMaskA(MODESEL_REG_ADDR[pin >> 3], value, mask);
    }
    //else
    //{
        // workaround: Register ITH_GPIO7_MODE_REG is write only (IC defact).
        // So we need a temp variable to store the GPIO7 mode setting.
    //    static uint16_t gpio7ModeRegData = 0;
    //    gpio7ModeRegData = ((gpio7ModeRegData & ~mask) | (value & mask));
    //    ithWriteRegA(ITH_GPIO_BASE + ITH_GPIO7_MODE_REG, gpio7ModeRegData);
    //}

    ithExitCritical();
}

void ithGpioCtrlEnable(unsigned int pin, ITHGpioCtrl ctrl)
{
     int group = pin >> 5;

    ithEnterCritical();

    switch (ctrl)
    {
    case ITH_GPIO_PULL_ENABLE:
        ithSetRegBitA(PULLEN_REG_ADDR[group], (pin & 0x1F));
        break;

    case ITH_GPIO_PULL_UP:
        ithSetRegBitA(PULLTYPE_REG_ADDR[group], (pin & 0x1F));
        break;

    case ITH_GPIO_INTR_LEVELTRIGGER:
        ithSetRegBitA(INTRTRIG_REG_ADDR[group], (pin & 0x1F));
        break;

    case ITH_GPIO_INTR_BOTHEDGE:
        ithSetRegBitA(INTRBOTH_REG_ADDR[group], (pin & 0x1F));
        break;

    case ITH_GPIO_INTR_TRIGGERFALLING:
        ithSetRegBitA(INTRRISENEG_REG_ADDR[group], (pin & 0x1F));
        break;
    }

    ithExitCritical();
}

void ithGpioCtrlDisable(unsigned int pin, ITHGpioCtrl ctrl)
{
    int group = pin >> 5;

    ithEnterCritical();

    switch (ctrl)
    {
    case ITH_GPIO_PULL_ENABLE:
        ithClearRegBitA(PULLEN_REG_ADDR[group], (pin & 0x1F));
        break;

    case ITH_GPIO_PULL_UP:
        ithClearRegBitA(PULLTYPE_REG_ADDR[group], (pin & 0x1F));
        break;

    case ITH_GPIO_INTR_LEVELTRIGGER:
        ithClearRegBitA(INTRTRIG_REG_ADDR[group], (pin & 0x1F));
        break;

    case ITH_GPIO_INTR_BOTHEDGE:
        ithClearRegBitA(INTRBOTH_REG_ADDR[group], (pin & 0x1F));
        break;

    case ITH_GPIO_INTR_TRIGGERFALLING:
        ithClearRegBitA(INTRRISENEG_REG_ADDR[group], (pin & 0x1F));
        break;
    }

    ithExitCritical();
}

void ithGpioSuspend(void)
{
    int i;

    for (i = 0; i < ITH_COUNT_OF(gpioRegs); i++)
    {
        switch (i)
        {
        case ITH_GPIO1_DATASET_REG:
        case ITH_GPIO1_DATACLR_REG:
        case ITH_GPIO1_INTRRAWSTATE_REG:
        case ITH_GPIO1_INTRMASKSTATE_REG:
        case ITH_GPIO1_INTRCLR_REG:
        case ITH_GPIO2_DATAOUT_REG:
        case ITH_GPIO2_DATAIN_REG:
        case ITH_GPIO2_DATASET_REG:
        case ITH_GPIO2_DATACLR_REG:
        case ITH_GPIO2_INTRRAWSTATE_REG:
        case ITH_GPIO2_INTRMASKSTATE_REG:
        case ITH_GPIO2_INTRCLR_REG:
        case ITH_GPIO_FEATURE_REG:
        case ITH_GPIO_REV_REG:
            // don't need to backup
            break;

        default:
            gpioRegs[i] = ithReadRegA(ITH_GPIO_BASE + ITH_GPIO1_PINDIR_REG + i * 4);
        }
    }
}

void ithGpioResume(void)
{
    int i;

    for (i = 0; i < ITH_COUNT_OF(gpioRegs); i++)
    {
        switch (i)
        {
        case ITH_GPIO1_DATASET_REG:
        case ITH_GPIO1_DATACLR_REG:
        case ITH_GPIO1_INTRRAWSTATE_REG:
        case ITH_GPIO1_INTRMASKSTATE_REG:
        case ITH_GPIO1_INTRCLR_REG:
        case ITH_GPIO2_DATAOUT_REG:
        case ITH_GPIO2_DATAIN_REG:
        case ITH_GPIO2_DATASET_REG:
        case ITH_GPIO2_DATACLR_REG:
        case ITH_GPIO2_INTRRAWSTATE_REG:
        case ITH_GPIO2_INTRMASKSTATE_REG:
        case ITH_GPIO2_INTRCLR_REG:
        case ITH_GPIO_FEATURE_REG:
        case ITH_GPIO_REV_REG:
            // don't need to restore
            break;

        default:
            ithWriteRegA(ITH_GPIO_BASE + ITH_GPIO1_PINDIR_REG + i * 4, gpioRegs[i]);
        }
    }
}

void ithGpioEnableClock(void)
{
    // enable clock
    ithSetRegBitA(ITH_APB_CLK2_REG, ITH_EN_W1CLK_BIT);
}

void ithGpioDisableClock(void)
{
    // disable clock
    ithClearRegBitA(ITH_APB_CLK2_REG, ITH_EN_W1CLK_BIT);
}

void ithGpioStats(void)
{
    PRINTF("GPIO:\r\n");
    ithPrintRegA(ITH_GPIO_BASE + ITH_GPIO1_DATAOUT_REG, ITH_GPIO_REV_REG - ITH_GPIO1_DATAOUT_REG + sizeof(uint32_t));   //for 9850
}

void ithGpioSetDriving(unsigned int pin, ITHGpioDriving level)
{
	ithEnterCritical();
    ithWriteRegMaskA(DRIVING_SET_REG_ADDR[pin >> 4], level << ((pin & 0xF) * 2),  (0x3) <<((pin & 0xF) * 2));
    ithExitCritical();
}

inline void ithGpioSetIn(unsigned int pin)
{
    int group = pin >> 5;

    ithEnterCritical();
    ithClearRegBitA(PINDIR_REG_ADDR[group], pin & 0x1F);
    ithExitCritical();
}

inline void ithGpioSetOut(unsigned int pin)
{
    int group = pin >> 5;

    ithEnterCritical();	
	ithSetRegBitA(PINDIR_REG_ADDR[group], pin & 0x1F);
    ithExitCritical();
}

inline void ithGpioSet(unsigned int pin)
{
    int group = pin >> 5;

    ithEnterCritical();
	ithWriteRegA(DATASET_REG_ADDR[group], 0x1 << (pin & 0x1F));
    ithExitCritical();
}

inline void ithGpioClear(unsigned int pin)
{
    int group = pin >> 5;

    ithEnterCritical();
    ithWriteRegA(DATACLR_REG_ADDR[group], 0x1 << (pin & 0x1F));	
    ithExitCritical();
}

inline uint32_t ithGpioGet(unsigned int pin)
{
    int group = pin >> 5;
    return ithReadRegA(DATAIN_REG_ADDR[group]) & (0x1 << (pin & 0x1F));
}

inline void ithGpioEnableIntr(unsigned int pin)
{
	int group = pin >> 5;	
	ithSetRegBitA(INTREN_REG_ADDR[group], (pin & 0x1F));
}

inline void ithGpioDisableIntr(unsigned int pin)
{
    int group = pin >> 5;
    ithClearRegBitA(INTREN_REG_ADDR[group], (pin & 0x1F));
}

inline void ithGpioClearIntr(unsigned int pin)
{
    int group = pin >> 5;    
    ithSetRegBitA(INTRCLR_REG_ADDR[group], (pin & 0x1F));
}

inline void ithGpioEnableBounce(unsigned int pin)
{
    int group = pin >> 5;
    ithSetRegBitA(BOUNCEEN_REG_ADDR[group], (pin & 0x1F));

}

inline void ithGpioDisableBounce(unsigned int pin)
{
    int group = pin >> 5;
    ithClearRegBitA(BOUNCEEN_REG_ADDR[group], (pin & 0x1F));
}

inline void ithGpioSetDebounceClock(unsigned int clk)
{
    ithWriteRegA(ITH_GPIO_BASE + ITH_GPIO_BOUNCEPRESCALE_REG, ithGetBusClock() / clk - 1);
}
