/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * HAL UART functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ith_cfg.h"

static uint32_t gFCRvalue[4] = { 0 };

static uint32_t *getFCRvalue(ITHUartPort port)
{
    if (port == ITH_UART0)
    	return &gFCRvalue[0];    
    else if (port == ITH_UART1)    
        return &gFCRvalue[1];
	else if (port == ITH_UART2)
		return &gFCRvalue[2];
	else
		return &gFCRvalue[3];
    
}

/**
 * Enables specified FIFO controls.
 *
 * @param ctrl the controls to enable.
 */
void ithUartFifoCtrlEnable(ITHUartPort port, ITHUartFifoCtrl ctrl)
{
    uint32_t* pValue = getFCRvalue(port);

    *pValue |= (1 << ITH_UART_FCR_FIFO_EN_BIT);    
    ithWriteRegA(port + ITH_UART_FCR_REG, *pValue);
    //ithWriteRegA(0xde600108, 1);	
	//ithWriteRegH(0x16B0 ,port + ITH_UART_FCR_REG);
}

/**
 * Disables specified FIFO controls.
 *
 * @param ctrl the controls to disable.
 */
void ithUartFifoCtrlDisable(ITHUartPort port, ITHUartFifoCtrl ctrl)
{
    uint32_t* pValue = getFCRvalue(port);

    *pValue &= ~(0x1 << ITH_UART_FCR_FIFO_EN_BIT);
    ithWriteRegA(port + ITH_UART_FCR_REG, *pValue);
}

/**
 * Sets UART TX interrupt trigger level.
 *
 * @param port The UART port
 * @param level The UART TX trigger level
 */
void ithUartSetTxTriggerLevel(
    ITHUartPort port,
    ITHUartTriggerLevel level)
{
    uint32_t* pValue = getFCRvalue(port);

    *pValue &= ~ITH_UART_FCR_TXFIFO_TRGL_MASK;
    *pValue |= (level << ITH_UART_FCR_TXFIFO_TRGL_BIT);
    ithWriteRegA(port + ITH_UART_FCR_REG, (*pValue | (0x1 << ITH_UART_FCR_TXFIFO_RESET_BIT)));
}

/**
 * Sets UART RX interrupt trigger level.
 *
 * @param port The UART port
 * @param level The UART RX trigger level
 */
void ithUartSetRxTriggerLevel(
    ITHUartPort port,
    ITHUartTriggerLevel level)
{
    uint32_t* pValue = getFCRvalue(port);

    *pValue &= ~ITH_UART_FCR_RXFIFO_TRGL_MASK;
    *pValue |= (level << ITH_UART_FCR_RXFIFO_TRGL_BIT);
    ithWriteRegA(port + ITH_UART_FCR_REG, (*pValue | (0x1 << ITH_UART_FCR_RXFIFO_RESET_BIT)));
}


void ithUartSetMode(
    ITHUartPort port,
    ITHUartMode mode,
    unsigned int txPin,
    unsigned int rxPin)
{
    int txgpiomode = -1, rxgpiomode=-1;

    switch(port)
    {
    case ITH_UART0:
#if defined(CFG_CHIP_PKG_IT9910) || defined(CFG_CHIP_PKG_IT9070)
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_HOSTSEL_REG, ITH_GPIO_HOSTSEL_GPIO << ITH_GPIO_HOSTSEL_BIT, ITH_GPIO_HOSTSEL_MASK);

#ifdef CFG_CHIP_PKG_IT9910
        if (mode != ITH_UART_TX)
            ithGpioSetMode(0, ITH_GPIO_MODE1);

        ithGpioSetMode(1, ITH_GPIO_MODE1);
#else
        if (mode != ITH_UART_TX)
            ithGpioSetMode(2, ITH_GPIO_MODE1);

        ithGpioSetMode(3, ITH_GPIO_MODE1);

        // IrDA
        if (mode != ITH_UART_DEFAULT)
        {
            ithGpioSetMode(6, ITH_GPIO_MODE1);

            if (mode == ITH_UART_FIR)
                ithSetRegBitH(ITH_UART_CLK_REG, ITH_UART_CLK_SRC_BIT);
        }
#endif
#endif
        txgpiomode = ITH_GPIO_MODE_TX0;
        rxgpiomode = ITH_GPIO_MODE_RX0;
        break;
    case ITH_UART1:
        txgpiomode = ITH_GPIO_MODE_TX1;
        rxgpiomode = ITH_GPIO_MODE_RX1;
        break;
    case ITH_UART2:
        txgpiomode = ITH_GPIO_MODE_TX2;
        rxgpiomode = ITH_GPIO_MODE_RX2;
        break;
    case ITH_UART3:
        txgpiomode = ITH_GPIO_MODE_TX3;
        rxgpiomode = ITH_GPIO_MODE_RX3;
        break;
    default:
        txgpiomode = ITH_GPIO_MODE_TX1;
        rxgpiomode = ITH_GPIO_MODE_RX1;
        break;
    }
    // cannot be IrDA mode
    if (mode != ITH_UART_TX && rxPin != -1)
    {
        ithGpioSetMode(rxPin, rxgpiomode);
        ithGpioSetIn(rxPin);
    }
    ithGpioSetMode(txPin, txgpiomode);
    ithGpioSetOut(txPin);

    ithWriteRegMaskA(port + ITH_UART_MDR_REG, mode, ITH_UART_MDR_MODE_SEL_MASK);
}

void ithUartSetParity(
    ITHUartPort port,
    ITHUartParity parity,
    unsigned int stop,
    unsigned int len)
{
    uint32_t lcr;
    lcr = ithReadRegA(port + ITH_UART_LCR_REG) & ~ITH_UART_LCR_DLAB;

    // Clear orignal parity setting
    lcr &= 0xC0;

    switch (parity)
    {
    case ITH_UART_ODD:
        lcr |= ITH_UART_LCR_ODD;
        break;

    case ITH_UART_EVEN:
        lcr |= ITH_UART_LCR_EVEN;
        break;

    case ITH_UART_MARK:
        lcr |= ITH_UART_LCR_STICKPARITY | ITH_UART_LCR_ODD;
        break;

    case ITH_UART_SPACE:
        lcr |= ITH_UART_LCR_STICKPARITY | ITH_UART_LCR_EVEN;
        break;

    default:
        break;
    }

    if (stop == 2)
        lcr |= ITH_UART_LCR_STOP;

    lcr |= len - 5;
    ithWriteRegA(port + ITH_UART_LCR_REG, lcr);
}


void ithUartSetBaudRate(  ITHUartPort port,
    unsigned int baud)
{
	unsigned int totalDiv, intDiv, fDiv;
	uint32_t lcr;
	lcr = ithReadRegA(port + ITH_UART_LCR_REG) & ~ITH_UART_LCR_DLAB;
	
	 // Set DLAB = 1
    ithWriteRegA(port + ITH_UART_LCR_REG, ITH_UART_LCR_DLAB);

	totalDiv = ithGetBusClock() / baud;
    intDiv = totalDiv >> 4;
    fDiv = totalDiv & 0xF;

	 // Set baud rate
    ithWriteRegA(port + ITH_UART_DLM_REG, (intDiv & 0xF00) >> 8);
    ithWriteRegA(port + ITH_UART_DLL_REG, intDiv & 0xFF);

    // Set fraction rate
    ithWriteRegA(port + ITH_UART_DLH_REG, fDiv & 0xF);

	ithWriteRegA(port + ITH_UART_LCR_REG, lcr);
}



void ithUartReset(
    ITHUartPort port,
    unsigned int baud,
    ITHUartParity parity,
    unsigned int stop,
    unsigned int len)
{
    unsigned int totalDiv, intDiv, fDiv;
    uint32_t lcr;

    // Power on clock
    ithSetRegBitH(ITH_UART_CLK_REG, ITH_UART_CLK_BIT);

    // Temporarily setting?
    #if (CFG_CHIP_FAMILY != 9850 && CFG_CHIP_FAMILY != 9920)
    if (port == ITH_UART0)
        ithWriteRegMaskA(ITH_GPIO_BASE + ITH_GPIO_HOSTSEL_REG, ITH_GPIO_HOSTSEL_GPIO << ITH_GPIO_HOSTSEL_BIT, ITH_GPIO_HOSTSEL_MASK);
    #endif

    totalDiv = ithGetBusClock() / baud;
    intDiv = totalDiv >> 4;
    fDiv = totalDiv & 0xF;

    lcr = ithReadRegA(port + ITH_UART_LCR_REG) & ~ITH_UART_LCR_DLAB;

    // Set DLAB = 1
    ithWriteRegA(port + ITH_UART_LCR_REG, ITH_UART_LCR_DLAB);

    // Set baud rate
    ithWriteRegA(port + ITH_UART_DLM_REG, (intDiv & 0xFF00) >> 8);
    ithWriteRegA(port + ITH_UART_DLL_REG, intDiv & 0xFF);

    // Set fraction rate
    ithWriteRegA(port + ITH_UART_DLH_REG, fDiv & 0xF);

    // Clear orignal parity setting
    lcr &= 0xC0;

    switch (parity)
    {
    case ITH_UART_ODD:
        lcr |= ITH_UART_LCR_ODD;
        break;

    case ITH_UART_EVEN:
        lcr |= ITH_UART_LCR_EVEN;
        break;

    case ITH_UART_MARK:
        lcr |= ITH_UART_LCR_STICKPARITY | ITH_UART_LCR_ODD;
        break;

    case ITH_UART_SPACE:
        lcr |= ITH_UART_LCR_STICKPARITY | ITH_UART_LCR_EVEN;
        break;

    default:
        break;
    }

    if (stop == 2)
        lcr |= ITH_UART_LCR_STOP;

    lcr |= len - 5;

    ithWriteRegA(port + ITH_UART_LCR_REG, lcr);

    ithUartFifoCtrlEnable(port, ITH_UART_FIFO_EN);  // enable fifo as default
    ithUartSetTxTriggerLevel(port, ITH_UART_TRGL2); // default to maximum level
    ithUartSetRxTriggerLevel(port, ITH_UART_TRGL0); // default to maximum level

}
