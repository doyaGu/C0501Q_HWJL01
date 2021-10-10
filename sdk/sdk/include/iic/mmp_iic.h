/*
 * Copyright (c) 2005 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * ITE IIC Driver API header file.
 *
 * @author Alex.C Hsieh
 * @version 0.9
 */
#ifndef MMP_IIC_H
#define MMP_IIC_H

#include <stdint.h>
#include <pthread.h>
#include "ite/itp.h"
#include "ite/mock_helper.h"

/**
 * DLL export API declaration for Win32.
 */
/*
   #if defined(WIN32)
   #if defined(IIC_EXPORTS)
   #define IIC_API __declspec(dllexport)
   #else
   #define IIC_API __declspec(dllimport)
   #endif
   #else
   #define IIC_API extern
   #endif
 */

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Compile Option
//=============================================================================
#define IIC_USE_DMA

//=============================================================================
//                              Macro Definition
//=============================================================================
/*Error Code*/

#define I2C_ERROR_CODE_BASE        0x012C0000

#define I2C_NON_ACK                (I2C_ERROR_CODE_BASE + 1u)
#define I2C_ARBITRATION_LOSS       (I2C_ERROR_CODE_BASE + 2u)
#define I2C_MODE_TRANSMIT_ERROR    (I2C_ERROR_CODE_BASE + 3u)
#define I2C_MODE_RECEIVE_ERROR     (I2C_ERROR_CODE_BASE + 4u)
#define I2C_WAIT_TRANSMIT_TIME_OUT (I2C_ERROR_CODE_BASE + 5u)
#define I2C_INVALID_ACK            (I2C_ERROR_CODE_BASE + 6u)

//=============================================================================
//                              Structure Definition
//=============================================================================
struct _IIC_DEVICE;

typedef int32_t MMP_IIC_HANDLE;

typedef void (*IicReceiveCallback)(uint8_t *recvBuffer, uint32_t recvBufferSize);
typedef int (*IicWriteCallback)(struct _IIC_DEVICE dev);

typedef enum _IIC_PORT
{
    IIC_PORT_0,
    IIC_PORT_1,
    IIC_PORT_2,
    IIC_PORT_MAX
} IIC_PORT;

typedef enum OP_MODE_TAG
{
    IIC_SLAVE_MODE,
    IIC_MASTER_MODE
} IIC_OP_MODE;

typedef struct _IIC_DEVICE
{
    pthread_mutex_t funcMutex;
    uint32_t        magicID;
    IIC_PORT        port;
    uint32_t        refCount;
    bool            inuse;
    uint32_t        ioSCLK;
    uint32_t        ioSDA;
    uint32_t        delay;
    IIC_OP_MODE     opMode;
    uint32_t        regAddrBase;
    uint32_t        clockRate;

#ifdef IIC_USE_DMA
    // DMA
    int   dmaChannel;
    #ifdef IIC_USE_DMA_INTR
    sem_t dmaMutex;
    #endif
#endif

    // Slave mode
    bool               stopSlaveThread;
    uint8_t            slaveAddr;
    pthread_t          slaveThread;
    IicReceiveCallback recvCallback;
    IicWriteCallback   writeCallback;
} IIC_DEVICE;

//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup group9 ITE IIC Driver API
 *  The supported API for IIC.
 *  @{
 */

/**
 * Initialize IIC
 *
 * @param port      set IIC device port.
 * @param opMode    set IIC master or slave mode.
 * @param sclk_pin  GPIO pin number which connect to IIC sclk pin.
 * @param data_pin  GPIO pin number which connect to IIC data pin.
 * @param initClock set initialize IIC clock.
 * @param delay     specify delay time (in ms) for each GPIO operation
 * @return          uint32_t_SUCCESS if succeed, error codes of uint32_t_ERROR
 *                  otherwise.
 *
 * @remark Application must call this API first when it want to use IIC API.
 * Before mmpIicTerminate() this API will be used once only.
 */
uint32_t
mmpIicInitialize(
    IIC_PORT    port,
    IIC_OP_MODE opMode,
    uint32_t    sclk_pin,
    uint32_t    data_pin,
    uint32_t    initClock,
    uint32_t    delay);

uint32_t
mmpSwIicInitialize(
    IIC_PORT    port,
    IIC_OP_MODE opMode,
    uint32_t    sclk_pin,
    uint32_t    data_pin,
    uint32_t    initClock,
    uint32_t    delay);

/**
 * Terminate IIC
 *
 * @param port  set IIC device port.
 *
 * @return      uint32_t_SUCCESS if succeed, error codes of uint32_t_ERROR
 *              otherwise.
 */
uint32_t
mmpIicTerminate(
    IIC_PORT port);

/**
 * Recieve device data packet throught IIC
 *
 * @param data  Recieved device data
 * @return      uint32_t_SUCCESS if succeed, error codes of uint32_t_ERROR
 *              otherwise.
 */
uint32_t
mmpIicRecieve(
	IIC_PORT port,
    uint32_t *data);

/**
 * Send device data packet throught IIC
 *
 * @param data  Data send to device
 * @return      uint32_t_SUCCESS if succeed, error codes of uint32_t_ERROR
 *              otherwise.
 */
uint32_t
mmpIicSend(
	IIC_PORT port,
    uint32_t data);

/**
 * IIC start condition
 *
 * @return      uint32_t_SUCCESS if succeed, error codes of uint32_t_ERROR
 *              otherwise.
 */
uint32_t
mmpIicStart(
	IIC_PORT port);

/**
 * IIC stop condition
 *
 * @return      uint32_t_SUCCESS if succeed, error codes of uint32_t_ERROR
 *              otherwise.
 */
uint32_t
mmpIicStop(
	IIC_PORT port);

/**
 * Send data to I2C slave device.
 *
 * @param port          set IIC device port.
 * @param mode          master/salve mode.
 * @param slaveAddr     slave address.
 * @param regAddr       starting reigster address.
 * @param outData       output buffer pointer.
 * @param outDataSize   number of bytes to Send.Maximum size is 254.
 * @return uint32_t_SUCCESS if succeed, otherwise error codes
 */
uint32_t
mmpIicSendData(
    IIC_PORT    port,
    IIC_OP_MODE mode,
    uint8_t     slaveAddr,
    uint8_t     regAddr,
    uint8_t     *outData,
    uint32_t    outDataSize);

uint32_t
mmpSwIicSendData(
    IIC_PORT    port,
    IIC_OP_MODE mode,
    uint8_t     slaveAddr,
    uint8_t     regAddr,
    uint8_t     *outData,
    uint32_t    outDataSize);

DECLARE_COULD_BE_MOCKED_FUNC6(
    uint32_t,
    mmpIicSendData,
    IIC_PORT,
    IIC_OP_MODE,
    uint8_t,
    uint8_t,
    uint8_t *,
    uint32_t);

/**
 * Send data to I2C slave device.
 *
 * @param mode          master/salve mode.So far only support master mode.
 * @param addr          slave address.
 * @param outData       output buffer pointer.
 * @param outDataSize   number of bytes to Send.Maximum size is 254.
 * @return uint32_t_SUCCESS if succeed, otherwise error codes
 */
uint32_t
mmpIicSendDataEx(
    IIC_PORT    port,
    IIC_OP_MODE mode,
    uint8_t     slaveAddr,
    uint8_t     *outData,
    uint32_t    outDataSize);

/**
 * Receive data from I2C slave device.
 *
 * @param port          set IIC device port.
 * @param mode          master/salve mode.So far only support master mode.
 * @param slaveAddr     slave address.
 * @param regAddr       starting reigster address.
 * @param outData       output buffer pointer.
 * @param outDataSize   number of bytes to send.Maximum size is 254.
 * @param inData        input buffer pointer.
 * @param inDataSize    number of bytes to receive.Maximum size is 254.
 * @return uint32_t_SUCCESS if succeed, otherwise error codes
 */
uint32_t
mmpIicReceiveData(
    IIC_PORT    port,
    IIC_OP_MODE mode,
    uint8_t     slaveAddr,
    uint8_t     *outData,
    uint32_t    outDataSize,
    uint8_t     *inData,
    uint32_t    inDataSize);

uint32_t
mmpSwIicReceiveData(
    IIC_PORT    port,
    IIC_OP_MODE mode,
    uint8_t     slaveAddr,
    uint8_t     *outData,
    uint32_t    outDataSize,
    uint8_t     *inData,
    uint32_t    inDataSize);

DECLARE_COULD_BE_MOCKED_FUNC7(
    uint32_t,
    mmpIicReceiveData,
    IIC_PORT,
    IIC_OP_MODE,
    uint8_t,
    uint8_t *,
    uint32_t,
    uint8_t *,
    uint32_t);

/**
 * Receive data to I2C slave device.
 *
 * @param mode          master/salve mode.So far only support master mode.
 * @param slaveAddr     slave address.
 * @param outData       output buffer pointer.
 * @param outDataSize   number of bytes to send.Maximum size is 254.
 * @param inData        input buffer pointer.
 * @param inDataSize    number of bytes to receive.Maximum size is 254.
 * @return uint32_t_SUCCESS if succeed, otherwise error codes
 */
uint32_t
mmpIicReceiveDataEx(
    IIC_PORT    port,
    IIC_OP_MODE mode,
    uint8_t     slaveAddr,
    uint8_t     *outData,
    uint32_t    outDataSize,
    uint8_t     *inData,
    uint32_t    inDataSize);

/**
 * Change I2c transferring clock .
 *
 * @param port          set IIC device port.
 * @param clock         desire clock rate.
 * @return actual clock rate.
 */
uint32_t
mmpIicSetClockRate(
    IIC_PORT port,
    uint32_t clock);

/**
 * Get current clock .
 *
 * @param port          set IIC device port.
 * @return current clock rate.
 */
uint32_t
mmpIicGetClockRate(
    IIC_PORT port);

/**
 * Gen stop indication command .
 *
 * @return uint32_t_SUCCESS if succeed, otherwise error codes
 */
uint32_t
mmpIicGenStop(
    IIC_PORT port);

/**
 * Lock whole IIC module to prevent re-entry issue.
 *
 * @param port          set IIC device port.
 * @return none
 */
void
mmpIicLockModule(
    IIC_PORT port);

/**
 * Release the locked IIC module for other task usage.
 *
 * @param port          set IIC device port.
 * @return none
 */
void
mmpIicReleaseModule(
    IIC_PORT port);

/**
 * Set IIC module to slave mode.
 *
 * @param slaveAddr     salve address.
 * @return none
 */
void
mmpIicSetSlaveMode(
	IIC_PORT port,
    uint32_t slaveAddr);

/**
 * Read data from IIC master.
 *
 * @param slaveAddr             salve address.
 * @param inutBuffer            The buffer to receive data from master.
 * @param inputBufferLength     The length of inutBuffer in byte.
 * @return none
 */
uint32_t
mmpIicSlaveRead(
	IIC_PORT port,
    uint32_t slaveAddr,
    uint8_t  *inutBuffer,
    uint32_t inputBufferLength);

/**
 * Read data from IIC master, and call specified function when data input.
 *
 * @param port              set IIC device port.
 * @param slaveAddr         salve address.
 * @param recvCallback      The callback function to handle input data.
 * @param writeCallback     The callback function to write data out.
 * @return success or not.
 */
bool
mmpIicSetSlaveModeCallback(
    IIC_PORT           port,
    uint32_t           slaveAddr,
    IicReceiveCallback recvCallback,
    IicWriteCallback   writeCallback);

/**
 * Write data to IIC master.
 *
 * @param port                  set IIC device port.
 * @param slaveAddr             salve address.
 * @param outputBuffer          Data pointer to write out.
 * @param outputBufferLength    The length of outputBuffer in byte.
 * @return success or not.
 */
uint32_t
mmpIicSlaveWrite(
    IIC_PORT port,
    uint32_t slaveAddr,
    uint8_t  *outputBuffer,
    uint32_t outputBufferLength);

/**
 * Write data to IIC master by DMA.
 *
 * @param port                  set IIC device port.
 * @param outputBuffer          Data pointer to write out.
 * @param outputBufferLength    The length of outputBuffer in byte.
 * @return success or not.
 */

uint32_t
mmpIicSlaveDmaWrite(
    IIC_PORT port,
    uint8_t  *outputBuffer,
    uint32_t outputBufferLength);

///////////////////////////////////////////////////////////////////////////////
//@}

#ifdef __cplusplus
}
#endif

#endif // MMP_IIC_H