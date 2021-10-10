/*==========================================================================*/
/*                                                                          */
/* MODULE:      wt8574.c                                                   */
/*                                                                          */
/* USAGE:       This module contains routines for Weltrend WT6854           */
/*              8 Bit I/O expander.                                        */
/*                                                                          */
/****************************************************************************/

//=============================================================================
//                              Include Files
//=============================================================================
#include <stdio.h>
#include <pthread.h>
#include "ite/itp.h"
#include "iic/mmp_iic.h"
#include "gpio_ex/wt6854.h"
#include "ite/ith.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define IO_EXPAND_DEV           0x20

// [read only] Input data. It represents the actual value of each pin.
// 0: low level input
// 1: high level input
#define REG_INTPUT_DATA         0x00

// [read/write] Output data. The value read back form this register can not
// represent actual pin value.
// 0: output low
// 1: output high
#define REG_OUTPUT_DATA         0x01

// [read/write] Enable polarity inversion or not.
// 0: no change. High level input will be read as "1" in REG_INTPUT_DATA.
// 1: enable polarity inversion. High level input will be read as "0" in REG_INTPUT_DATA.
#define REG_POLARITY_INVERSION  0x02

// [read/write] Control the I/O pin direction.
// 0: output
// 1: input
#define REG_PIN_DIRECTION       0x03

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
static uint8_t  SaveOutputPins = 0;
static uint8_t  SaveInputPins  = 0;
static uint8_t  OldStatus      = 0;
static uint8_t  NewStatus      = 0;

/****************************************************************************/
/*  C O D E                                                                 */
/****************************************************************************/

/*  Write all the 8 pins to IO expander. */
void IOExpander_WritePort(uint8_t Reg, uint8_t W_Data)
{
    uint32_t iicResult      = 0;
    uint32_t i2c_clock_rate = 0;

    W_Port    = W_Data;
    iicResult = mmpIicSendData(IIC_PORT_0, IIC_MASTER_MODE, IO_EXPAND_DEV, Reg, &W_Data, 1);   //WT6854
}

uint8_t IOExpander_ReadPort(void)
{
    uint8_t  U_Data;
    uint32_t iicResult      = 0;
    uint32_t i2c_clock_rate = 0;
    uint8_t  Reg            = 0x00;

    iicResult = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, IO_EXPAND_DEV, &Reg, 1, &U_Data, 1);
    return U_Data;   // Return all 8 pin
}

/* Read the port pin number in IO expander.  */
uint8_t IOExpander_ReadPortPin(uint8_t W_PinNum)
{
    uint8_t  U_Data;
    uint32_t iicResult      = 0;
    uint32_t i2c_clock_rate = 0;
    uint8_t  Reg            = 0x00;

    iicResult = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, IO_EXPAND_DEV, &Reg, 1, &U_Data, 1);
    OldStatus = U_Data;
    if (U_Data & (0x1 << W_PinNum))  // Read/mask the request pin
        return (1);                  // Pin high
    else
        return (0);
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

/* Set the port pin number to Input mode in IO expander.  */
void IOExpander_SetInPortPin(uint8_t W_PinNum)
{
    uint8_t  Reg       = 0x03;
    uint32_t iicResult = 0;

    SaveInputPins |= (0x1 << W_PinNum);
    iicResult      = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, IO_EXPAND_DEV, &Reg, 1, &W_Port, 1);

    W_Port        &= (~(0x1 << W_PinNum));
    W_Port        |= (0x1 << W_PinNum);

    iicResult      = mmpIicSendData(IIC_PORT_0, IIC_MASTER_MODE, IO_EXPAND_DEV, Reg, &W_Port, 1);
}

/* Set the port pin number to Output mode in IO expander.  */
void IOExpander_SetOutPortPin(uint8_t W_PinNum)
{
    uint8_t  Reg       = 0x03;
    uint32_t iicResult = 0;
    SaveOutputPins |= (0x1 << W_PinNum);

    iicResult       = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, IO_EXPAND_DEV, &Reg, 1, &W_Port, 1);

    W_Port         &= (~(0x1 << W_PinNum));
    //W_Port  |= (0x1 << W_PinNum);

    iicResult       = mmpIicSendData(IIC_PORT_0, IIC_MASTER_MODE, IO_EXPAND_DEV, Reg, &W_Port, 1);
}

/* Set the port pin number in IO expander.  */
void IOExpander_SetPortPin(uint8_t W_PinNum)
{
    uint32_t iicResult      = 0;
    uint32_t i2c_clock_rate = 0;
    uint8_t  Reg            = 0x01;

    iicResult = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, IO_EXPAND_DEV, &Reg, 1, &W_Port, 1);

    W_Port   |= (0x1 << W_PinNum);

    iicResult = mmpIicSendData(IIC_PORT_0, IIC_MASTER_MODE, IO_EXPAND_DEV, Reg, &W_Port, 1);
}

/* Clear the port pin number in IO expander.  */
void IOExpander_ClrPortPin(uint8_t W_PinNum)
{
    uint32_t iicResult      = 0;
    uint32_t i2c_clock_rate = 0;
    uint8_t  Reg            = 0x01;

    iicResult = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, IO_EXPAND_DEV, &Reg, 1, &W_Port, 1);

    W_Port   &= (~(0x1 << W_PinNum));
    //W_Port   |= SaveInputPins;

    iicResult = mmpIicSendData(IIC_PORT_0, IIC_MASTER_MODE, IO_EXPAND_DEV, Reg, &W_Port, 1);
}

void IOExpanderDriver_initial(void)
{
    char    pin = 4;
    uint8_t R_data;
    R_data    = 0x00;
    IOExpander_WritePort(0x02, R_data); //input data no inversion . high level input will be as "1".
    R_data    = 0xFF;
    IOExpander_WritePort(0x03, R_data); //default all setting to input mode , 0 : output  , 1:input
    OldStatus = IOExpander_ReadPort();  //save the first GPIO realize status (input).
    IOExpander_SetOutPortPin(pin);      //default set  pins 4= output.
    IOExpander_ClrPortPin(pin);
}

/********************************  END  *************************************/
