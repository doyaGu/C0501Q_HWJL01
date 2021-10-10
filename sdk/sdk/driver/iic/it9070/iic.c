/*
 * Copyright (c) 2005 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  I2C API functoin file.
 *      Date: 2005/10/7
 *
 * @author Alex.C Hsieh
 * @version 0.95
 */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include "mmp_iic.h"
#include "i2c_hwreg.h"
#include "ite/ith.h"

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================
#ifdef DEBUG
    #define IIC_DbgPrint printf
#else
    #define IIC_DbgPrint(x) printf(x)
#endif

#define IIC_REG_BIT(var, n) (((var & (1 << n)) > 0) ? 1 : 0)

#define PRECONDITION           assert
#define TIMEOUT                20000                            //0X10000
#define CLK_1M                 1024 * 1024
#define CLK_400K               (400 * 1024)
#define CLK_100K               (100 * 1024)
#define IIC_MAX_SIZE           256
#define SEM_ENABLE             0
#define MMP_MAX_IIC_DEVICE_NUM 0x10
#define CLK_32K                32768
#define REF_CLOCK              40000000                         // 36M
//=============================================================================
//                              Structure Definition
//=============================================================================
typedef enum IIC_STATE_FLAG_TAG
{
    TRANSMIT_DATA    = 0x1,
    RECEIVE_DATA     = 0x2,
    TRANSMIT_SERVICE = 0x4,
    RECRIVE_SERVICE  = 0x8,
    NACK_EN          = 0x10
} IIC_STATE_FLAG;

typedef enum DATA_MODE_TAG
{
    WO_STOP,                        //without stop
    W_STOP                          //with stop
} IIC_DATA_MODE;

//=============================================================================
//                              Global Data Definition
//=============================================================================
static IIC_DEVICE      g_IIC_Device[MMP_MAX_IIC_DEVICE_NUM];
static uint32_t        g_IIC_DeviceCount = 0;
static uint32_t        gCurrentClock     = 0;

static uint32_t        IicReferenceCount = 0;
static pthread_mutex_t IicExternalMutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t IicInternalMutex  = PTHREAD_MUTEX_INITIALIZER;
static uint32_t        IIC_delay         = 0;

#if (defined CFG_IIC0_GPIO_CONFIG_1)
static uint32_t        IIC_GPIO_SCL      = 2;
static uint32_t        IIC_GPIO_SDA      = 3;
#elif (defined CFG_IIC0_GPIO_CONFIG_2)
static uint32_t        IIC_GPIO_SCL      = 11;
static uint32_t        IIC_GPIO_SDA      = 12;
#endif
//=============================================================================
//                              Function Definition
//=============================================================================
void
_IIC_Delay(
    uint32_t loop)
{
    while (loop)
        loop--;
    return;
}

uint32_t
IIC_WaitDeviceAck(
    IIC_DEVICE *iic_dev)
{
    uint32_t ack = 0;

    // Set to input mode
    // [20100125] Remove by Vincent
    //mmpSetGpioMode(MMP_GPIO_INPUT_MODE, iic_dev->gpio_group, iic_dev->ioSDA);

#if defined (MMP_AUDIO_CODEC_WM8728) || defined(MMP_AUDIO_CODEC_WM8778) || defined(MMP_AUDIO_CODEC_WM8750BL)
    _IIC_Delay(iic_dev->delay); //liang

    // [20100125] Remove by Vincent
    //mmpGetGpioState(iic_dev->gpio_group, iic_dev->ioSDA, &ack); //liang
#else
    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSDA, GPIO_PULL_HI(iic_dev->ioSDA));
#endif // defined(MMP_AUDIO_CODEC_WM8728) || defined(MMP_AUDIO_CODEC_WM8778) || defined(MMP_AUDIO_CODEC_WM8750BL)

    _IIC_Delay(iic_dev->delay);

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSCLK, GPIO_PULL_LO(iic_dev->ioSCLK));
    _IIC_Delay(iic_dev->delay);

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSCLK, GPIO_PULL_HI(iic_dev->ioSCLK));
    _IIC_Delay(iic_dev->delay);

    // [20100125] Remove by Vincent
    //mmpGetGpioState(iic_dev->gpio_group, iic_dev->ioSDA, &ack);
    _IIC_Delay(iic_dev->delay);

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSCLK, GPIO_PULL_LO(iic_dev->ioSCLK));

    // Set to input mode
    // [20100125] Remove by Vincent
    //mmpSetGpioMode(MMP_GPIO_INPUT_MODE, iic_dev->gpio_group, iic_dev->ioSDA);

    if ((ack & iic_dev->ioSDA) == 0)
    {
        return 0;
    }

    return 1;
}

uint32_t
IIC_SendDeviceAck(
    IIC_DEVICE *iic_dev)
{
    // Set to output mode
    // [20100125] Remove by Vincent
    //mmpSetGpioMode(MMP_GPIO_OUTPUT_MODE, iic_dev->gpio_group, iic_dev->ioSDA);

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSCLK, GPIO_PULL_LO(iic_dev->ioSCLK));
    _IIC_Delay(iic_dev->delay);

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSDA, GPIO_PULL_LO(iic_dev->ioSDA));
    _IIC_Delay(iic_dev->delay);

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSCLK, GPIO_PULL_HI(iic_dev->ioSCLK));
    _IIC_Delay(iic_dev->delay);

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSCLK, GPIO_PULL_LO(iic_dev->ioSCLK));

    // Set to input mode
    // [20100125] Remove by Vincent
    //mmpSetGpioMode(MMP_GPIO_INPUT_MODE, iic_dev->gpio_group, iic_dev->ioSDA);

    return 0;
}

void
IIC_StartCondition(
    IIC_DEVICE *iic_dev)
{
    // SCLK/DATA pull high
    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSCLK|iic_dev->ioSDA, GPIO_PULL_HI(iic_dev->ioSCLK)|GPIO_PULL_HI(iic_dev->ioSDA));
    _IIC_Delay(iic_dev->delay);

    // DATA pull low
    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSDA, GPIO_PULL_LO(iic_dev->ioSDA));
    _IIC_Delay(iic_dev->delay);

    // SCLK pull low
    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSCLK, GPIO_PULL_LO(iic_dev->ioSCLK));
}

void
IIC_StopCondition(
    IIC_DEVICE *iic_dev)
{
    // SCLK/DATA pull low
    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSCLK|iic_dev->ioSDA, GPIO_PULL_LO(iic_dev->ioSCLK)|GPIO_PULL_LO(iic_dev->ioSDA));
    _IIC_Delay(iic_dev->delay);
    // SCLK pull high

    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSCLK, GPIO_PULL_HI(iic_dev->ioSCLK));
    _IIC_Delay(iic_dev->delay);

    // DATA pull high
    // [20100125] Remove by Vincent
    //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSDA, GPIO_PULL_HI(iic_dev->ioSDA));
    _IIC_Delay(iic_dev->delay);
}

uint32_t
IIC_SendDeviceWrite(
    IIC_DEVICE *iic_dev,
    uint16_t   data)
{
    uint32_t i = 0;
    // send address
    for (i = 0; i < 8; i++)
    {
        if (data & 0x80)
        {
            // [20100125] Remove by Vincent
            //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSDA,GPIO_PULL_HI(iic_dev->ioSDA));
        }
        else
        {
            // [20100125] Remove by Vincent
            //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSDA,GPIO_PULL_LO(iic_dev->ioSDA));
        }
        _IIC_Delay(iic_dev->delay);

        // CLK pull low, DATA prepare
        // [20100125] Remove by Vincent
        //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSCLK,GPIO_PULL_LO(iic_dev->ioSCLK));
        _IIC_Delay(iic_dev->delay);

        // CLK pull high, DATA transfer
        // [20100125] Remove by Vincent
        //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSCLK,GPIO_PULL_HI(iic_dev->ioSCLK));
        _IIC_Delay(iic_dev->delay);

        // CLK pull low, DATA ready
        // [20100125] Remove by Vincent
        //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSCLK,GPIO_PULL_LO(iic_dev->ioSCLK));
        _IIC_Delay(iic_dev->delay);
        data <<= 1;
    }

    if (IIC_WaitDeviceAck(iic_dev) != 0)
    {
        IIC_DbgPrint(("Write to device error (NO ACK)"));
        return 1;
    }
    return 0;
}

uint32_t
IIC_SendDeviceRead(
    IIC_DEVICE *iic_dev,
    uint16_t   *data)
{
    uint32_t i        = 0;
    uint32_t readBack = 0x0000;
    for (i = 0; i < 8; i++)
    {
        // CLK pull high
        // [20100125] Remove by Vincent
        //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSCLK,GPIO_PULL_HI(iic_dev->ioSCLK));
        _IIC_Delay(iic_dev->delay);
        // [20100125] Remove by Vincent
        //mmpGetGpioState(iic_dev->gpio_group, iic_dev->ioSDA,&readBack);
        _IIC_Delay(iic_dev->delay);
        // CLK pull low
        // [20100125] Remove by Vincent
        //mmpSetGpioState(iic_dev->gpio_group, iic_dev->ioSCLK,GPIO_PULL_LO(iic_dev->ioSCLK));
        _IIC_Delay(iic_dev->delay);

        if (readBack & iic_dev->ioSDA)
        {
            (*data) |= 1;
        }

        (*data) <<= 1;
    }
    (*data) >>= 1;

    if (IIC_SendDeviceAck(iic_dev) != 0)
    {
        IIC_DbgPrint(("Write to device error (NO ACK)"));
        return 1;
    }
    return 0;
}

uint32_t
IIC_GetFreeDevice(
    MMP_IIC_HANDLE *hDevice)
{
    uint32_t i      = 0;
    uint32_t result = 1;

    (*hDevice) = 0;
    for (i = 0; i < MMP_MAX_IIC_DEVICE_NUM; i++)
    {
        if (g_IIC_Device[i].inuse == false)
        {
            *(hDevice) = i;
            result     = 0;
            break;
        }
    }

    return result;
}

uint32_t
IIC_SearchDevice()
{
    uint32_t i      = 0;
    uint32_t result = 0;

    /*
       for( i=0; i<MMP_MAX_IIC_DEVICE_NUM; i++ )
       {
       if(g_IIC_Device[i].inuse == false)
        continue;

       if(g_IIC_Device[i].ioSDA == data_pin)
       {
        result = 1;
        break;
       }

       if(g_IIC_Device[i].ioSCLK == sclk_pin)
       {
        result = 1;
        break;
       }
       }
     */

    return result;
}

uint32_t
IIC_AddDevice(
    MMP_IIC_HANDLE *hDevice,
    uint32_t       delay)
{
    uint32_t result;

#if SEM_ENABLE
    SYS_WaitSemaphore(g_IIC_Semaphore);
#endif

    if ((result = IIC_SearchDevice(/*gpio_group, sclk_pin, data_pin*/)) != 0)
    {
        goto END;
    }

    if ((result = IIC_GetFreeDevice(hDevice)) == 0)
    {
        g_IIC_Device[(*hDevice)].inuse = true;
        g_IIC_Device[(*hDevice)].delay = delay;

        g_IIC_DeviceCount++;
    }

END:

#if SEM_ENABLE
    SYS_ReleaseSemaphore(g_IIC_Semaphore);
#endif

    return result;
}

uint32_t
IIC_RemoveDevice(
    MMP_IIC_HANDLE hDevice)
{
    uint32_t result = 0;

#if SEM_ENABLE
    SYS_WaitSemaphore(g_IIC_Semaphore);
#endif

    if (g_IIC_Device[hDevice].inuse != true)
    {
        result = 1;
        goto END;
    }

    // [20100125] Remove by Vincent
    //mmpSetGpioMode(MMP_GPIO_INPUT_MODE,
    //               g_IIC_Device[hDevice].gpio_group,
    //               g_IIC_Device[hDevice].ioSCLK|g_IIC_Device[hDevice].ioSDA);

    g_IIC_Device[hDevice].inuse  = false;
    g_IIC_Device[hDevice].ioSCLK = 0;
    g_IIC_Device[hDevice].ioSDA  = 0;
    g_IIC_Device[hDevice].delay  = 0;

    g_IIC_DeviceCount--;

END:

#if SEM_ENABLE
    SYS_ReleaseSemaphore(g_IIC_Semaphore);
#endif

    return result;
}

//=============================================================================
/**
 *  FARADAY FTIIC010
 *  Formula : SCLout = PCLK/(2*COUNT - GSR + 1)
 *
 *  PCLK is formed by REG_WCLK
 *  COUNT is REG_I2C_CLOCK_DIV[9:0]
 *  GSR is REG_I2C_GLITCH[12:10]
 */
//=============================================================================
static uint32_t
IIC_SetClockRate(
    uint32_t clock)
{
    uint32_t pclk;
    uint32_t div;
    uint32_t glitch;
    uint32_t count;
    uint32_t gsr;

#if defined(__OPENRTOS__) || defined(WIN32)
    pclk          = ithGetBusClock();
#elif defined(__FREERTOS__)
    pclk          = or32_getBusCLK();
#else
    pclk          = REF_CLOCK;//fpga extern clk
#endif

    glitch        = ithReadRegA(REG_I2C_GLITCH);
    div           = (pclk / clock);
    gsr           = (glitch & REG_MASK_GSR) >> REG_SHIFT_GSR;
    count         = (((div - gsr - 4) / 2) & REG_MASK_CLK_DIV_COUNT); //count = (((div + gsr -1)/2) & REG_MASK_CLK_DIV_COUNT) ;
    gCurrentClock = (uint32_t)(pclk / (2 * count + gsr + 4));         //gCurrentClock = (uint32_t)(pclk/(2*count - gsr +1));

    //set i2c div
    ithWriteRegA(REG_I2C_CLOCK_DIV, count);
    //printf("IIC current clock %f KHz\n", ((double)gCurrentClock) / 1024);
    return gCurrentClock;
}

static uint32_t
IIC_CheckAck(
    IIC_STATE_FLAG state)
{
    uint32_t i      = 0;
    uint32_t data   = 0;
    uint32_t result = 0;

    while (++i)
    {
        data = ithReadRegA(REG_I2C_STATUS);

        if (state & TRANSMIT_DATA)
        {
            if (data & REG_BIT_STATUS_DATA_TRANSFER_DONE)
            {
                if (data & REG_BIT_STATUS_NON_ACK)
                {
                    result = I2C_NON_ACK;
                    goto end;
                }
                if (data & REG_BIT_STATUS_ARBITRATION_LOSS)
                {
                    result = I2C_ARBITRATION_LOSS;
                    goto end;
                }

                if (state & RECRIVE_SERVICE)
                {
                    if (!(data & REG_BIT_STATUS_RECEIVE_MODE))
                    {
                        result = I2C_MODE_TRANSMIT_ERROR;
                        goto end;
                    }
                }
                else
                {
                    if (data & REG_BIT_STATUS_RECEIVE_MODE)
                    {
                        result = I2C_MODE_TRANSMIT_ERROR;
                        goto end;
                    }
                }

                //no error and leave loop
                result = 0;
                goto end;
            }
        }
        else if (state & RECEIVE_DATA)
        {
            if (data & REG_BIT_STATUS_DATA_RECEIVE_DONE)
            {
                if (state & NACK_EN)
                {
                    if (!(data & REG_BIT_STATUS_NON_ACK))
                    {
                        result = I2C_INVALID_ACK;
                        goto end;
                    }
                }
                else
                {
                    if (data & REG_BIT_STATUS_NON_ACK)
                    {
                        result = I2C_NON_ACK;
                        goto end;
                    }
                }

                if (!(data & REG_BIT_STATUS_RECEIVE_MODE))
                {
                    result = I2C_MODE_RECEIVE_ERROR;
                    goto end;
                }
                //no error and leave loop
                result = 0;
                goto end;
            }
        }

        if (i > TIMEOUT)
        {
            result = I2C_WAIT_TRANSMIT_TIME_OUT;
            goto end;
        }
    }

end:
    return result;
}

static uint32_t
IIC_SlaveCheckAck(
    IIC_STATE_FLAG state)
{
    uint32_t i      = 0;
    uint32_t data   = 0;
    uint32_t result = 0;

    while (++i)
    {
        data = ithReadRegA(REG_I2C_STATUS);

        if (state & TRANSMIT_DATA)
        {
            if (data & REG_BIT_STATUS_DATA_TRANSFER_DONE)
            {
                if (data & REG_BIT_STATUS_NON_ACK)
                {
                    result = I2C_NON_ACK;
                    goto end;
                }
                if (data & REG_BIT_STATUS_ARBITRATION_LOSS)
                {
                    result = I2C_ARBITRATION_LOSS;
                    goto end;
                }

                if (state & RECRIVE_SERVICE)
                {
                    if (!(data & REG_BIT_STATUS_RECEIVE_MODE))
                    {
                        printf("[%d] ERROR~~~~~\n", __LINE__);
                        result = I2C_MODE_TRANSMIT_ERROR;
                        goto end;
                    }
                }

                //no error and leave loop
                result = 0;
                goto end;
            }
        }
        else if (state & RECEIVE_DATA)
        {
            if (data & REG_BIT_STATUS_DATA_RECEIVE_DONE)
            {
                if (state & NACK_EN)
                {
                    if (!(data & REG_BIT_STATUS_NON_ACK))
                    {
                        result = I2C_INVALID_ACK;
                        goto end;
                    }
                }
                else
                {
                    if (data & REG_BIT_STATUS_NON_ACK)
                    {
                        result = I2C_NON_ACK;
                        goto end;
                    }
                }

                if (!(data & REG_BIT_STATUS_RECEIVE_MODE))
                {
                    result = I2C_MODE_RECEIVE_ERROR;
                    goto end;
                }
                //no error and leave loop
                result = 0;
                goto end;
            }
        }

        if (i > TIMEOUT)
        {
            result = I2C_WAIT_TRANSMIT_TIME_OUT;
            goto end;
        }
    }

end:
    return result;
}

static uint32_t
IIC_Initialize(
    uint32_t delay)
{
    uint32_t result = 0;
    uint32_t data;
#if 0 // add by Steven for 4 wire case
    HOST_ReadRegister(0x784A, &data);
    data |= 0x1;
    HOST_WriteRegister(0x784A, data);
#endif
    //Reset HW
    ithWriteRegA(REG_I2C_CONTROL, REG_BIT_CONTL_I2C_RESET);
    usleep(5);
    ithWriteRegA(REG_I2C_CONTROL, 0u);

    IIC_SetClockRate(CLK_400K);
    if (delay)
    {
        data  = ithReadRegA(REG_I2C_DATA);
        data &= ~(REG_MASK_TSR);
        data |= delay;
        // Setup GSR
        data |= (0x7 << 10);
        ithWriteRegA(REG_I2C_GLITCH, data);
    }

#if SEM_ENABLE
    //create semaphore
    if (g_IIC_Semaphore == NULL)
    {
        g_IIC_Semaphore = SYS_CreateSemaphore(1, "MMP_IIC_INIT");
    }
#endif

    return result;
}

static uint32_t
I2C_Terminate(
    void)
{
    uint32_t result = 0;

    //disable HW
    ithWriteRegMaskA(REG_I2C_CONTROL, 0u, REG_BIT_CONTL_CLK_ENABLE | REG_BIT_CONTL_I2C_ENABLE);

#if SEM_ENABLE
    //release semaphore
    if (g_IIC_Semaphore)
    {
        SYS_DeleteSemaphore(g_IIC_Semaphore);
        g_IIC_Semaphore = NULL;
    }
#endif

    return result;
}

uint32_t
IIC_SendData(
    uint8_t       slaveAddr,
    uint8_t       *pbuffer,
    uint16_t      size,
    IIC_DATA_MODE wStop)
{
    uint16_t i;
    uint32_t result;
    uint16_t data;

    ithEnterCritical();
    //set slave addr and sent start command
    //slave address bit[7:1]
    slaveAddr <<= 1;
    //r/w bit[0]
    slaveAddr  &= ~REG_BIT_READ_ENABLE;
    ithWriteRegA(REG_I2C_DATA, (uint16_t)slaveAddr);
    ithWriteRegA(
        REG_I2C_CONTROL,
        REG_BIT_INTR_ALL |
        REG_BIT_CONTL_TRANSFER_BYTE |
        REG_BIT_CONTL_CLK_ENABLE |
        REG_BIT_CONTL_I2C_ENABLE |
        REG_BIT_CONTL_START);

    //check Ack
    result = IIC_CheckAck(TRANSMIT_DATA);
    if (result != 0)
    {
        goto end;
    }

    for (i = 0; i < size; i++)
    {
        data = (uint16_t)pbuffer[i];
        ithWriteRegA(REG_I2C_DATA, data);
        //last byte
        if (wStop && i == (size - 1))
        {
            //enable STOP Flag
            ithWriteRegA(
                REG_I2C_CONTROL,
                REG_BIT_INTR_TRIG |
                REG_BIT_CONTL_TRANSFER_BYTE |
                REG_BIT_CONTL_CLK_ENABLE |
                REG_BIT_CONTL_I2C_ENABLE |
                REG_BIT_CONTL_STOP);
        }
        else
        {
            ithWriteRegA(
                REG_I2C_CONTROL,
                REG_BIT_INTR_TRIG |
                REG_BIT_CONTL_TRANSFER_BYTE |
                REG_BIT_CONTL_CLK_ENABLE |
                REG_BIT_CONTL_I2C_ENABLE);
        }

        //check Ack
        result = IIC_CheckAck(TRANSMIT_DATA);
        if (result != 0)
        {
            goto end;
        }
    }
    ithExitCritical();

end:
    ithExitCritical();
    return result;
}

uint32_t
IIC_ReceiveData(
    uint8_t  slaveAddr,
    uint8_t  *pbuffer,
    uint16_t size)
{
    uint16_t i;
    uint32_t result;
    uint32_t data;

    ithEnterCritical();
    //set slave addr and sent start command
    //slave address bit[7:1]
    slaveAddr <<= 1;
    //r/w bit[0]
    slaveAddr  |= REG_BIT_READ_ENABLE;
    ithWriteRegA(REG_I2C_DATA, slaveAddr);
    ithWriteRegA(
        REG_I2C_CONTROL,
        REG_BIT_INTR_TRIG |
        REG_BIT_CONTL_TRANSFER_BYTE |
        REG_BIT_CONTL_CLK_ENABLE |
        REG_BIT_CONTL_I2C_ENABLE |
        REG_BIT_CONTL_START);

    //check Ack
    result = IIC_CheckAck(TRANSMIT_DATA | RECRIVE_SERVICE);
    if (result != 0)
    {
        goto end;
    }

    for (i = 0; i < size; i++)
    {
        //last byte
        if (i == (size - 1))
        {
            //enable STOP / NACk Flag
            ithWriteRegA(
                REG_I2C_CONTROL,
                REG_BIT_INTR_TRIG |
                REG_BIT_CONTL_TRANSFER_BYTE |
                REG_BIT_CONTL_CLK_ENABLE |
                REG_BIT_CONTL_I2C_ENABLE |
                REG_BIT_CONTL_STOP |
                REG_BIT_CONTL_NACK);

            //check Ack
            result = IIC_CheckAck(RECEIVE_DATA | NACK_EN);
            if (result == I2C_NON_ACK)
            {
                result = 0;
            }

            data       = ithReadRegA(REG_I2C_DATA);
            pbuffer[i] = (uint8_t)data;
        }
        else
        {
            ithWriteRegA(
                REG_I2C_CONTROL,
                REG_BIT_INTR_TRIG |
                REG_BIT_CONTL_TRANSFER_BYTE |
                REG_BIT_CONTL_CLK_ENABLE |
                REG_BIT_CONTL_I2C_ENABLE);

            //check Ack
            result = IIC_CheckAck(RECEIVE_DATA);
            if (result != 0)
            {
                goto end;
            }

            data       = ithReadRegA(REG_I2C_DATA);
            pbuffer[i] = (uint8_t)data;
        }
    }
    ithExitCritical();

end:
    ithExitCritical();
    return result;
}

uint32_t
mmpIicInitialize(
    IIC_PORT    port,
    IIC_OP_MODE opMode,
    uint32_t    sclk_pin,
    uint32_t    data_pin,
    uint32_t    initClock,    // in Hz
    uint32_t    delay)
{
    uint32_t result = 0;

    pthread_mutex_lock(&IicInternalMutex);

    IicReferenceCount++;
    if (IicReferenceCount == 1)
    {
#ifdef CFG_TVOUT_HDMI
        ithWriteRegMaskA(ITH_GPIO_BASE + 0xD0, 0, (1 << 29) | (1 << 28));
#else
        ithWriteRegMaskA(ITH_GPIO_BASE + 0xD0, (1 << 28), (1 << 29) | (1 << 28));
#endif

#if (defined CFG_IIC0_GPIO_CONFIG_1)
        ithWriteRegMaskA(ITH_GPIO_BASE + 0xD0, (1 << 6), (1 << 6) | (1 << 5) | (1 << 4));
        ithWriteRegMaskA(ITH_GPIO_BASE + 0x90, (0x3 << 4 | 0x3 << 6), (0x3 << 4 | 0x3 << 6));
#elif (defined CFG_IIC0_GPIO_CONFIG_2)
        ithWriteRegMaskA(ITH_GPIO_BASE + 0x90, (0x3 << 22 | 0x3 << 24), (0x3 << 22 | 0x3 << 24));
#else
    #error "ERROR: Only support IIC0_GPIO_CONFIG_1 or IIC0_GPIO_CONFIG_2"
#endif
        result    = IIC_Initialize(delay);
        IIC_delay = delay;
    }
    else
    {
        printf("ERROR! IIC multiple initialization!!!\n");
    }

    pthread_mutex_unlock(&IicInternalMutex);

    return result;
}

uint32_t
mmpIicTerminate(
    IIC_PORT port)
{
    uint32_t result = 0;

    pthread_mutex_lock(&IicInternalMutex);

    IicReferenceCount--;
    assert(IicReferenceCount >= 0);
    if (IicReferenceCount == 0)
    {
        result = I2C_Terminate();
    }

    pthread_mutex_unlock(&IicInternalMutex);

    if (IicReferenceCount == 0)
    {
        //pthread_mutex_destroy(&IicInternalMutex);
    }

    return result;
}

uint32_t
mmpIicStart(
	IIC_PORT port)
{
    uint32_t result = 0;

    pthread_mutex_lock(&IicInternalMutex);

    ithWriteRegA(
        REG_I2C_CONTROL,
        REG_BIT_INTR_TRIG |
        REG_BIT_CONTL_TRANSFER_BYTE |
        REG_BIT_CONTL_CLK_ENABLE |
        REG_BIT_CONTL_I2C_ENABLE |
        REG_BIT_CONTL_START);

    pthread_mutex_unlock(&IicInternalMutex);

    return result;
}

uint32_t
mmpIicStop(
	IIC_PORT port)
{
    uint32_t result = 0;

    pthread_mutex_lock(&IicInternalMutex);

    ithWriteRegA(
        REG_I2C_CONTROL,
        REG_BIT_INTR_TRIG |
        REG_BIT_CONTL_TRANSFER_BYTE |
        REG_BIT_CONTL_CLK_ENABLE |
        REG_BIT_CONTL_I2C_ENABLE |
        REG_BIT_CONTL_STOP);

    pthread_mutex_unlock(&IicInternalMutex);

    return result;
}

uint32_t
mmpIicRecieve(
	IIC_PORT port,
    uint32_t *data)
{
    uint32_t result = 0;
    uint32_t count  = 0;
    pthread_mutex_lock(&IicInternalMutex);

    while (ithReadRegA(REG_I2C_BUS_MONITOR) != 0x3)
    {
        count++;
        usleep(1000);
        if (count >= 1500)
        {
            printf("i2c is busy ...(Recieve)(scl=%x, sda=%x)\n", ithGpioGet(IIC_GPIO_SCL), ithGpioGet(IIC_GPIO_SDA));
            IIC_Initialize(IIC_delay);
            break;
        }
    }

    ithWriteRegA(REG_I2C_DATA, (uint32_t)data);
    result = IIC_CheckAck(RECEIVE_DATA);

    pthread_mutex_unlock(&IicInternalMutex);

    return result;
}

uint32_t
mmpIicSend(
	IIC_PORT port,	
    uint32_t data)
{
    uint32_t result = 0;
    uint32_t count  = 0;
    pthread_mutex_lock(&IicInternalMutex);

    while (ithReadRegA(REG_I2C_BUS_MONITOR) != 0x3)
    {
        count++;
        usleep(1000);
        if (count >= 1500)
        {
            printf("i2c is busy ...(Send)(scl=%x, sda=%x)\n", ithGpioGet(IIC_GPIO_SCL), ithGpioGet(IIC_GPIO_SDA));
            IIC_Initialize(IIC_delay);
            break;
        }
    }

    ithWriteRegA(REG_I2C_DATA, data);
    result = IIC_CheckAck(TRANSMIT_DATA);

    pthread_mutex_unlock(&IicInternalMutex);

    return result;
}

#ifdef CFG_DEV_TEST
DEFINE_COULD_BE_MOCKED_NOT_VOID_FUNC6(
    uint32_t,
    mmpIicSendData,
    IIC_PORT,    port,
    IIC_OP_MODE, mode,
    uint8_t,     slaveAddr,
    uint8_t,     regAddr,
    uint8_t *,   outData,
    uint32_t,    outDataSize)
#else
__attribute__((used)) uint32_t
mmpIicSendData(
    IIC_PORT    port,
    IIC_OP_MODE mode,
    uint8_t     slaveAddr,
    uint8_t     regAddr,
    uint8_t     *outData,
    uint32_t    outDataSize)
#endif
{
    uint32_t result;
    uint8_t  dbuf[256];
    uint8_t  *pdbuf = dbuf;
    uint16_t size2;
    uint32_t count  = 0;
    PRECONDITION(outData);
    PRECONDITION(outDataSize < IIC_MAX_SIZE);

    pthread_mutex_lock(&IicInternalMutex);

    while (ithReadRegA(REG_I2C_BUS_MONITOR) != 0x3)
    {
        count++;
        usleep(1000);
        if (count >= 1500)
        {
            printf("i2c is busy ...(SendData)(%x,%x)(scl=%x, sda=%x)\n", slaveAddr, regAddr, ithGpioGet(IIC_GPIO_SCL), ithGpioGet(IIC_GPIO_SDA));
            IIC_Initialize(IIC_delay);
            break;
        }
    }

    *pdbuf++ = regAddr;
    memcpy(pdbuf, outData, outDataSize);
    size2    = outDataSize + 1;
    result   = IIC_SendData(slaveAddr, dbuf, size2, W_STOP);

    pthread_mutex_unlock(&IicInternalMutex);

    return result;
}

uint32_t
mmpIicSendDataEx(
    IIC_PORT    port,
    IIC_OP_MODE mode,
    uint8_t     slaveAddr,
    uint8_t     *outData,
    uint32_t    outDataSize)
{
    uint32_t result;
    uint32_t count = 0;
    PRECONDITION(outDataSize);
    PRECONDITION(outDataSize < IIC_MAX_SIZE);

    pthread_mutex_lock(&IicInternalMutex);

    while (ithReadRegA(REG_I2C_BUS_MONITOR) != 0x3)
    {
        count++;
        usleep(1000);
        if (count >= 1500)
        {
            printf("i2c is busy ...(SendDataEx)(%x)(scl=%x, sda=%x)\n", slaveAddr, ithGpioGet(IIC_GPIO_SCL), ithGpioGet(IIC_GPIO_SDA));
            IIC_Initialize(IIC_delay);
            break;
        }
    }

    result = IIC_SendData(slaveAddr, outData, outDataSize, W_STOP);
    pthread_mutex_unlock(&IicInternalMutex);

    return result;
}

#ifdef CFG_DEV_TEST
DEFINE_COULD_BE_MOCKED_NOT_VOID_FUNC7(
    uint32_t,
    mmpIicReceiveData,
    IIC_PORT,    port,
    IIC_OP_MODE, mode,
    uint8_t,     slaveAddr,
    uint8_t *,   outData,
    uint32_t,    outDataSize,
    uint8_t *,   inData,
    uint32_t,    inDataSize)
#else
uint32_t
mmpIicReceiveData(
    IIC_PORT    port,
    IIC_OP_MODE mode,
    uint8_t     slaveAddr,
    uint8_t     *outData,
    uint32_t    outDataSize,
    uint8_t     *inData,
    uint32_t    inDataSize)
#endif
{
    uint32_t result;
    uint32_t count = 0;
    PRECONDITION(inData);
    PRECONDITION(inDataSize < IIC_MAX_SIZE);

    pthread_mutex_lock(&IicInternalMutex);

    while (ithReadRegA(REG_I2C_BUS_MONITOR) != 0x3)
    {
        count++;
        usleep(1000);
        if (count >= 1500)
        {
            printf("i2c is busy ...(ReceiveData)(%x,%x)(scl=%x, sda=%x)\n", slaveAddr, *outData, ithGpioGet(IIC_GPIO_SCL), ithGpioGet(IIC_GPIO_SDA));
            IIC_Initialize(IIC_delay);
            break;
        }
    }

    result = IIC_SendData(slaveAddr, outData, outDataSize, WO_STOP);

    if (result == 0)
    {
        result = IIC_ReceiveData(slaveAddr, inData, inDataSize);
    }

    pthread_mutex_unlock(&IicInternalMutex);

    return result;
}

uint32_t
mmpIicReceiveDataEx(
    IIC_PORT    port,
    IIC_OP_MODE mode,
    uint8_t     slaveAddr,
    uint8_t     *outData,
    uint32_t    outDataSize,
    uint8_t     *inData,
    uint32_t    inDataSize)
{
    uint32_t result = 0;
    uint32_t count  = 0;

    PRECONDITION(inData);
    PRECONDITION(outDataSize < IIC_MAX_SIZE);
    PRECONDITION(inDataSize < IIC_MAX_SIZE);

    pthread_mutex_lock(&IicInternalMutex);

    while (ithReadRegA(REG_I2C_BUS_MONITOR) != 0x3)
    {
        count++;
        usleep(1000);
        if (count >= 1500)
        {
            printf("i2c is busy ...(ReceiveDataEX)(%x)(scl=%x, sda=%x)\n", slaveAddr, ithGpioGet(IIC_GPIO_SCL), ithGpioGet(IIC_GPIO_SDA));
            IIC_Initialize(IIC_delay);
            break;
        }
    }

    if (outData && outDataSize)
    {
        result = IIC_SendData(slaveAddr, outData, outDataSize, WO_STOP);
    }

    if (result == 0)
    {
        result = IIC_ReceiveData(slaveAddr, inData, inDataSize);
    }

    pthread_mutex_unlock(&IicInternalMutex);

    return result;
}

uint32_t
mmpIicSetClockRate(
    IIC_PORT port,
    uint32_t clock)
{
    uint32_t result = 0;
    uint32_t count  = 0;

    pthread_mutex_lock(&IicInternalMutex);

    while (ithReadRegA(REG_I2C_BUS_MONITOR) != 0x3)
    {
        count++;
        usleep(1000);
        if (count >= 1500)
        {
            printf("i2c is busy ...(SetClockRate)(%x)(scl=%x, sda=%x)\n", clock, ithGpioGet(IIC_GPIO_SCL), ithGpioGet(IIC_GPIO_SDA));
            IIC_Initialize(IIC_delay);
            break;
        }
    }
    result = IIC_SetClockRate(clock);
    pthread_mutex_unlock(&IicInternalMutex);

    return result;
}

uint32_t
mmpIicGetClockRate(
    IIC_PORT port)
{
    uint32_t result = 0;

    pthread_mutex_lock(&IicInternalMutex);
    result = gCurrentClock;
    pthread_mutex_unlock(&IicInternalMutex);

    return result;
}

uint32_t
mmpIicGenStop(
	IIC_PORT port)
{
    pthread_mutex_lock(&IicInternalMutex);
    ithWriteRegA(
        REG_I2C_CONTROL,
        REG_BIT_INTR_TRIG |
        REG_BIT_CONTL_TRANSFER_BYTE |
        REG_BIT_CONTL_CLK_ENABLE |
        REG_BIT_CONTL_I2C_ENABLE |
        REG_BIT_CONTL_STOP |
        REG_BIT_CONTL_NACK);
    pthread_mutex_unlock(&IicInternalMutex);

    return 0;
}

void
mmpIicLockModule(
    IIC_PORT port)
{
    pthread_mutex_lock(&IicExternalMutex);
}

void
mmpIicReleaseModule(
    IIC_PORT port)
{
    pthread_mutex_unlock(&IicExternalMutex);
}

void
mmpIicSetSlaveMode(
	IIC_PORT port,
    uint32_t slaveAddr)
{
    /* Set slave address */
    ithWriteRegA(REG_I2C_SLAVE_ADDR, slaveAddr);

    /* Write CR: Enable all interrupts and I2C enable, and disable SCL enable */
    ithWriteRegA(
        REG_I2C_CONTROL,
        REG_BIT_INTR_ALL |
        REG_BIT_CONTL_TRANSFER_BYTE |
        //REG_BIT_CONTL_GC |
        REG_BIT_CONTL_I2C_ENABLE);
}

/**
 * mmpIicSlaveRead()
 *
 * @param outputBuffer: To receive data from IIC master.
 * @param outputBufferLength: The buffer length of outputBuffer.
 *
 * @return MMP_TRUE if data received. MMP_FALSE if no data received.
 */
uint32_t
mmpIicSlaveRead(
	IIC_PORT port,
    uint32_t slaveAddr,
    uint8_t  *inutBuffer,
    uint32_t inputBufferLength)
{
    uint32_t outputBufferWriteIndex = 0;
    uint32_t regData                = 0;
    uint32_t ccc                    = 0;

    /* Set slave address */
    ithWriteRegA(REG_I2C_SLAVE_ADDR, slaveAddr);

    /* Write CR: Enable all interrupts and I2C enable, and disable SCL enable */
    ithWriteRegA(
        REG_I2C_CONTROL,
        REG_BIT_INTR_ALL |
        REG_BIT_CONTL_TRANSFER_BYTE |
        REG_BIT_CONTL_GC |
        REG_BIT_CONTL_I2C_ENABLE);

    /* Read status, check if recevice matched slave address */
    regData = ithReadRegA(REG_I2C_STATUS);
    if (IIC_REG_BIT(regData, 8) == 1 &&         /* SAM */
        IIC_REG_BIT(regData, 5) == 1 &&         /* DR */
        IIC_REG_BIT(regData, 2) == 1 &&         /* I2CB */
        IIC_REG_BIT(regData, 1) == 0 &&         /* ACK */
        IIC_REG_BIT(regData, 0) == 0)           /* RW */
    {
        while (1)
        {
            /* Enable transfer bit */
            ithWriteRegA(
                REG_I2C_CONTROL,
                REG_BIT_INTR_ALL |
                REG_BIT_CONTL_TRANSFER_BYTE |
                REG_BIT_CONTL_GC |
                REG_BIT_CONTL_I2C_ENABLE);

            regData = ithReadRegA(REG_I2C_STATUS);
            if (IIC_REG_BIT(regData, 7) == 1)                   /* Detect STOP condition */
            {
                /* Receive STOP */
                goto end;
            }

            if (IIC_REG_BIT(regData, 5) == 1)                           /* Receive ack */
            {
                /* Read data */
                uint32_t iicInputData = 0;

                iicInputData                       = ithReadRegA(REG_I2C_DATA);
                inutBuffer[outputBufferWriteIndex] = iicInputData & 0x000000FF;
                outputBufferWriteIndex++;
                if (outputBufferWriteIndex >= inputBufferLength)
                {
                    goto end;
                }
            }
        }
    }

end:
    return outputBufferWriteIndex;
}

bool
mmpIicSetSlaveModeCallback(
    IIC_PORT           port,
    uint32_t           slaveAddr,
    IicReceiveCallback recvCallback,
    IicWriteCallback   writeCallback)
{
    uint32_t outputBufferWriteIndex = 0;
    uint32_t regData                = 0;

    /* Set slave address */
    ithWriteRegA(REG_I2C_SLAVE_ADDR, slaveAddr);

    /* Write CR: Enable all interrupts and I2C enable, and disable SCL enable */
    ithWriteRegA(
        REG_I2C_CONTROL,
        REG_BIT_INTR_ALL |
        REG_BIT_CONTL_TRANSFER_BYTE |
        REG_BIT_CONTL_I2C_ENABLE);

    usleep(1000);
    /* Read status, check if recevice matched slave address */
    regData = ithReadRegA(REG_I2C_STATUS);
    if (IIC_REG_BIT(regData, 8) == 1 &&         /* SAM */
        IIC_REG_BIT(regData, 5) == 1 &&         /* DR */
        IIC_REG_BIT(regData, 2) == 1 &&         /* I2CB */
        IIC_REG_BIT(regData, 1) == 0)
    {
        while (1)
        {
            if (IIC_REG_BIT(regData, 0) == 0)                   /* RW */
            {
                // Slave read
                /* Enable transfer bit */
                ithWriteRegMaskA(
                    REG_I2C_CONTROL,
                    REG_BIT_CONTL_TRANSFER_BYTE,
                    REG_BIT_CONTL_TRANSFER_BYTE);

                regData = ithReadRegA(REG_I2C_STATUS);
                if (IIC_REG_BIT(regData, 7) == 1)                       /* Detect STOP condition */
                {
                    /* Receive STOP */
                    goto end;
                }

                if (IIC_REG_BIT(regData, 5) == 1)
                {
                    /* Read data */
                    uint32_t iicInputData = 0;

                    iicInputData = ithReadRegA(REG_I2C_DATA);
                    if (recvCallback)
                    {
                        recvCallback(NULL, iicInputData);
                        outputBufferWriteIndex++;
                    }
                }
            }
            else if (IIC_REG_BIT(regData, 0) == 1)                      /* RW */
            {
                regData = ithReadRegA(REG_I2C_STATUS);
                if (IIC_REG_BIT(regData, 7) == 1)                       /* Detect STOP condition */
                {
                    /* Receive STOP */
                    goto end;
                }

                // Slave write
                if (writeCallback)
                {
                    if (writeCallback(g_IIC_Device[0]))
                    {
                        goto end;
                    }
                }
            }
        }
    }

end:
    return true;
}

uint32_t
mmpIicSlaveWrite(
    IIC_PORT port,
    uint32_t slaveAddr,
    uint8_t  *outputBuffer,
    uint32_t outputBufferLength)
{
    uint32_t iicResult  = 0;
    uint32_t regData    = 0;
    uint32_t writeIndex = 0;

    /* Set slave address */
    ithWriteRegA(REG_I2C_SLAVE_ADDR, slaveAddr);

    for (writeIndex = 0; writeIndex < outputBufferLength; writeIndex++)
    {
        ithWriteRegA(REG_I2C_DATA, outputBuffer[writeIndex]);
        /* Enable transfer bit */
        /* Write CR: Enable all interrupts and I2C enable, and disable SCL enable */
        ithWriteRegA(
            REG_I2C_CONTROL,
            REG_BIT_INTR_ALL |
            REG_BIT_CONTL_TRANSFER_BYTE |
            REG_BIT_CONTL_I2C_ENABLE);

        /* Check ACK */
        iicResult = IIC_SlaveCheckAck(TRANSMIT_DATA);
        if (iicResult)
        {
            // Check ACK fail
            goto end;
        }
    }

end:
    return iicResult;
}

uint32_t
mmpIicSlaveDmaWrite(
    IIC_PORT port,
    uint8_t  *outputBuffer,
    uint32_t outputBufferLength)
{
    printf("IT9070 not support this function!!!\n");
    return;
}
