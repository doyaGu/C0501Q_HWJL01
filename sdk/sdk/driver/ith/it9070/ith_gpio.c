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

#define DEFINE_REG_ADDR_TABLE2(FUNC) static const uint32_t FUNC ## _REG_ADDR[2] =   \
{                                                                                   \
    ITH_GPIO_BASE + ITH_GPIO1_ ## FUNC ## _REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO2_ ## FUNC ## _REG,                                     \
};

#define DEFINE_REG_ADDR_TABLE4(FUNC) static const uint32_t FUNC ## _REG_ADDR[4] =   \
{                                                                                   \
    ITH_GPIO_BASE + ITH_GPIO1_ ## FUNC ## _REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO2_ ## FUNC ## _REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO3_ ## FUNC ## _REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO4_ ## FUNC ## _REG,                                     \
};

#define DEFINE_REG_ADDR_TABLE2_2(FUNC) static const uint32_t FUNC ## _REG_ADDR[2] = \
{                                                                                   \
    ITH_GPIO_BASE + ITH_GPIO_ ## FUNC ## 1_REG,                                     \
    ITH_GPIO_BASE + ITH_GPIO_ ## FUNC ## 2_REG,                                     \
};

DEFINE_REG_ADDR_TABLE2(PINDIR)
DEFINE_REG_ADDR_TABLE2(DATASET)
DEFINE_REG_ADDR_TABLE2(DATACLR)
DEFINE_REG_ADDR_TABLE2(DATAIN)
DEFINE_REG_ADDR_TABLE2(INTREN)
DEFINE_REG_ADDR_TABLE2(INTRCLR)
DEFINE_REG_ADDR_TABLE2(BOUNCEEN)
DEFINE_REG_ADDR_TABLE2(PULLEN)
DEFINE_REG_ADDR_TABLE2(PULLTYPE)
DEFINE_REG_ADDR_TABLE2(INTRTRIG)
DEFINE_REG_ADDR_TABLE2(INTRBOTH)
DEFINE_REG_ADDR_TABLE2(INTRRISENEG)
DEFINE_REG_ADDR_TABLE4(MODE)
DEFINE_REG_ADDR_TABLE2_2(URTXSEL)
DEFINE_REG_ADDR_TABLE2_2(URRXSEL)
DEFINE_REG_ADDR_TABLE4(DRIVING_SET)

static uint32_t gpioRegs[(ITH_GPIO_URRXSEL2_REG - ITH_GPIO1_PINDIR_REG + 4) / 4];

void ithGpioSetMode(unsigned int pin, ITHGpioMode mode)
{
    uint32_t value, mask;
    int group = pin >> 5;

    ithEnterCritical();
    switch (mode)
    {
    // for UART1 output
    case ITH_GPIO_MODE_TX1:
        ithSetRegBitA(URTXSEL_REG_ADDR[group], (pin & 0x1F));
        mode = 0;
        break;

    case ITH_GPIO_MODE_RX1:
        ithSetRegBitA(URRXSEL_REG_ADDR[group], (pin & 0x1F));
        mode = 0;
        break;
    }

    value = mode << ((pin & 0xF)* 2);
    mask  = 0x3 << ((pin & 0xF) * 2);
    ithWriteRegMaskA(MODE_REG_ADDR[pin >> 4], value, mask);

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
    ithSetRegBitH(ITH_APB_CLK2_REG, ITH_EN_W1CLK_BIT);
}

void ithGpioDisableClock(void)
{
    // disable clock
    ithClearRegBitH(ITH_APB_CLK2_REG, ITH_EN_W1CLK_BIT);
}

void ithGpioStats(void)
{
    PRINTF("GPIO:\r\n");
    ithPrintRegA(ITH_GPIO_BASE + ITH_GPIO1_DATAOUT_REG, ITH_GPIO_URRXSEL2_REG - ITH_GPIO1_DATAOUT_REG + sizeof(uint32_t));
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

void ithGpioSetDriving(unsigned int pin, ITHGpioDriving level)
{
	ithEnterCritical();
    ithWriteRegMaskA(DRIVING_SET_REG_ADDR[pin >> 4], level << ((pin & 0xF) * 2),  (0x3) <<((pin & 0xF) * 2));
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

    //ithEnterCritical();
    ithSetRegBitA(INTREN_REG_ADDR[group], (pin & 0x1F));
    //ithExitCritical();
}

inline void ithGpioDisableIntr(unsigned int pin)
{
    int group = pin >> 5;

    //ithEnterCritical();
    ithClearRegBitA(INTREN_REG_ADDR[group], (pin & 0x1F));
    //ithExitCritical();
}

inline void ithGpioClearIntr(unsigned int pin)
{
    int group = pin >> 5;

    //ithEnterCritical();
    ithSetRegBitA(INTRCLR_REG_ADDR[group], (pin & 0x1F));
    //ithExitCritical();
}

inline void ithGpioEnableBounce(unsigned int pin)
{
    int group = pin >> 5;

    //ithEnterCritical();
    ithSetRegBitA(BOUNCEEN_REG_ADDR[group], (pin & 0x1F));
    //ithExitCritical();
}

inline void ithGpioDisableBounce(unsigned int pin)
{
    int group = pin >> 5;

    //ithEnterCritical();
    ithClearRegBitA(BOUNCEEN_REG_ADDR[group], (pin & 0x1F));
    //ithExitCritical();
}

inline void ithGpioSetDebounceClock(unsigned int clk)
{
    ithWriteRegA(ITH_GPIO_BASE + ITH_GPIO2_BOUNCEPRESCALE_REG, ithGetBusClock() / clk - 1);
}