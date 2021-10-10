/*==========================================================================*/
/*                                                                          */
/* MODULE:      pcf8574.c                                                   */
/*                                                                          */
/* USAGE:       This module contains routines for Philips PCF8574           */
/*              16 Bit I/O expander.                                        */
/*                                                                          */
/****************************************************************************/

//=============================================================================
//                              Include Files
//=============================================================================
#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "iic/mmp_iic.h"
#include "gpio_ex/pcf8574.h"
#include "ite/ith.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define IO_EXPAND_DEV       0x40 >> 1
#define QSIZE        10
#define IIC_CLK_100K (100 * 1024)

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
static uint8_t  W_Port         = 0;
static uint8_t  tempQueue[QSIZE];
static uint32_t count          = 0;
static uint8_t  tempRIndex     = 0;
static uint8_t  tempWIndex     = 0;
static uint8_t  SaveOutputPins = 0;
static uint8_t  SaveInputPins  = 0;
static uint8_t  OldStatus      = 0;
static uint8_t  NewStatus      = 0;

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================

/* Read all the 8 pins from IO expander.  */
uint8_t IOExpander_ReadPort(void)
{
    uint8_t  U_Data;
    uint32_t iicResult      = 0;
    uint32_t i2c_clock_rate = 0;

    iicResult = mmpIicReceiveDataEx(
		IIC_PORT_0,
        IIC_MASTER_MODE,
        IO_EXPAND_DEV,
        0,
        0,
        &U_Data,
        1);

    return U_Data;   // Return all 8 pin
}

/*  Write all the 8 pins to IO expander. */
void IOExpander_WritePort(uint8_t W_Data)
{
    uint32_t iicResult      = 0;
    uint32_t i2c_clock_rate = 0;

    W_Port    = W_Data;

    iicResult = mmpIicSendDataEx(IIC_PORT_0, IIC_MASTER_MODE, IO_EXPAND_DEV, &W_Data, 1);
}

/* Set the port pin number to Input mode in IO expander.  */
void IOExpander_SetInPortPin(uint8_t W_PinNum)
{
    SaveInputPins |= (0x1 << W_PinNum);
}

/* Set the port pin number to Output mode in IO expander.  */
void IOExpander_SetOutPortPin(uint8_t W_PinNum)
{
    SaveOutputPins |= (0x1 << W_PinNum);
}

/* Read the port pin number in IO expander.  */
uint8_t IOExpander_ReadPortPin(uint8_t W_PinNum)
{
    uint8_t  U_Data;
    uint32_t iicResult      = 0;
    uint32_t i2c_clock_rate = 0;

    iicResult = mmpIicReceiveDataEx(
		IIC_PORT_0,
        IIC_MASTER_MODE,
        IO_EXPAND_DEV,
        0,
        0,
        &U_Data,
        1);

    OldStatus = U_Data;
    if (U_Data & (0x1 << W_PinNum))  // Read/mask the request pin
        return (1);                  // Pin high
    else
        return (0);
}

/* Set the port pin number in IO expander.  */
void IOExpander_SetPortPin(uint8_t W_PinNum)
{
    uint32_t iicResult      = 0;
    uint32_t i2c_clock_rate = 0;

    iicResult = mmpIicReceiveDataEx(
		IIC_PORT_0,
        IIC_MASTER_MODE,
        IO_EXPAND_DEV,
        0,
        0,
        &W_Port,
        1);

    W_Port   |= SaveInputPins;
    W_Port   |= (0x1 << W_PinNum);

    iicResult = mmpIicSendDataEx(IIC_PORT_0, IIC_MASTER_MODE, IO_EXPAND_DEV, &W_Port, 1);
}

/* Clear the port pin number in IO expander.  */
void IOExpander_ClrPortPin(uint8_t W_PinNum)
{
    uint32_t iicResult      = 0;
    uint32_t i2c_clock_rate = 0;

    iicResult = mmpIicReceiveDataEx(
		IIC_PORT_0,
        IIC_MASTER_MODE,
        IO_EXPAND_DEV,
        0,
        0,
        &W_Port,
        1);

    W_Port   &= (~(0x1 << W_PinNum));
    W_Port   |= SaveInputPins;

    iicResult = mmpIicSendDataEx(IIC_PORT_0, IIC_MASTER_MODE, IO_EXPAND_DEV, &W_Port, 1);
}

uint8_t IOExpander_GetInputStatus(void)
{
    uint8_t r_data       = 0;
    uint8_t i;
    uint8_t SaveDiffPins = 0;
    uint8_t temp_old     = 0;
    uint8_t temp_new     = 0;
    r_data    = IOExpander_ReadPort();
    NewStatus = r_data;

    temp_old  = OldStatus;
    temp_new  = NewStatus;

    if (OldStatus != NewStatus)
    {
        for (i = 0; i < 8; i++)
        {
            if ( (temp_old & 0x1) != (temp_new & 0x1))
            {
                SaveDiffPins |= (0x1 << i);
            }

            temp_old >>= 1;
            temp_new >>= 1;

            if (temp_old == temp_new)
            {
                OldStatus = NewStatus;
                break;
            }
        }
        OldStatus = NewStatus;
    }
    else
    {
        //avoid outputpins`s interrupt to influence AP.
        return 0;
    }

    return SaveDiffPins;
}

void IOExpanderDriver_initial(void)
{
    char pin = 4;
    //uint32_t iicResult  = mmpIicInitialize(0, 0);  //if IIC device not initial before that can using it.
    IOExpander_WritePort(0xFF);        //default all setting to input mode.
    OldStatus = IOExpander_ReadPort(); //save the first GPIO realize status.
#ifndef CFG_SECOND_CAPTURE_MODULE
    IOExpander_SetOutPortPin(pin);     //default set  pins 4= output  for Opendoor case.
	IOExpander_ClrPortPin(pin);
#endif
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
