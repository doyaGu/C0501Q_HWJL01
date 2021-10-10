/*
 * Copyright (c) 2004 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * SPI bus Driver API header file.
 *
 * @author Sammy Chen
 */
#ifndef MMP_SPI_H
#define MMP_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DLL export API declaration for Win32 and WinCE.
 */
#if 0
    #if defined(WIN32) || defined(_WIN32_WCE)
        #if defined(SPI_EXPORTS)
            #define SPI_API __declspec(dllexport)
        #else
            #define SPI_API __declspec(dllimport)
        #endif
    #else
        #define SPI_API     extern
    #endif /* defined(WIN32) */
#endif
#define SPI_API

//=============================================================================
//                              Compile Option
//=============================================================================
#define SPI_USE_DMA
#define SPI_USE_DMA_INTR

#if defined(SPI_USE_DMA_INTR) && !defined(__OPENRTOS__)
    #undef SPI_USE_DMA_INTR
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "stdint.h"
#include <stdbool.h>
#include <pthread.h>
#ifdef ARM_NOR_WRITER
#include "armNorWriterUtility.h" 
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum SPI_PORT_TAG
{
    SPI_0,
    SPI_1,
    SPI_PORT_MAX
}SPI_PORT;

typedef enum SPI_MODE_TAG
{
    SPI_MODE_0,
    SPI_MODE_1,
    SPI_MODE_2,
    SPI_MODE_3
}SPI_MODE;

typedef enum 
{
	CPO_0_CPH_0,
	CPO_1_CPH_0,
	CPO_0_CPH_1,
	CPO_1_CPH_1
}SPI_FORMAT;

typedef enum
{
	SPI_CLK_5K,
	SPI_CLK_1M,
	SPI_CLK_5M,
	SPI_CLK_10M,
	SPI_CLK_20M,
	SPI_CLK_40M,
}SPI_CLK_LAB;

typedef struct SPI_REF_CLK_TAG
{
	SPI_CLK_LAB spi_clk_lab;
	float   refclock;
}SPI_REF_CLK;

typedef struct SPI_IO_MAPPING_ENTRY_TAG
{
    int gpioPin;
    int gpioMode;
} SPI_IO_MAPPING_ENTRY;

typedef struct SPI_CONFIG_MAPPING_ENTRY_TAG
{
    SPI_IO_MAPPING_ENTRY spiMosi;
    SPI_IO_MAPPING_ENTRY spiMiso;
    SPI_IO_MAPPING_ENTRY spiClock;
    SPI_IO_MAPPING_ENTRY spiChipSel;
} SPI_CONFIG_MAPPING_ENTRY;

typedef struct
{
	int					refCount;
	pthread_mutex_t		mutex;
	SPI_FORMAT 			format;
	SPI_MODE   			mode;
	uint32_t   			divider;
	uint32_t   			dma_ch;
	char*	   			ch_name;
	uint32_t   			dma_slave_ch;
	char*	   			slave_ch_name;
	SPI_CONFIG_MAPPING_ENTRY    tMappingEntry;
}SPI_CONTEXT, *pSPI_CONTEXT;

typedef enum SPI_CONTROL_MODE_TAG
{
    SPI_CONTROL_NOR,
    SPI_CONTROL_SLAVE,
    SPI_CONTROL_NAND,
    SPI_SHARE_GPIO_MAX
}SPI_CONTROL_MODE;

typedef enum SPI_OP_MODE_TAG
{
    SPI_OP_MASTR,
    SPI_OP_SLAVE
}SPI_OP_MODE;

typedef enum SPI_CSN_TAG
{
	SPI_CSN_0,
	SPI_CSN_1
}SPI_CSN;

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================
typedef bool (*SpiSlaveCallbackFunc)(uint32_t inData);

/** @defgroup group20 ITE SPI Driver API
 *  The supported API for SPI.
 *  @{
 */
//=============================================================================
/**
 * File system must call this API first when initializing a spi bus.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpSdTerminate()
 */
//=============================================================================
SPI_API int32_t mmpSpiInitialize(SPI_PORT port, SPI_OP_MODE mode, SPI_FORMAT  format, SPI_CLK_LAB spiclk);

//=============================================================================
/**
 * This routine is used to release any resources associated with a drive when it is terminated.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpSdInitialize()
 */
//=============================================================================
SPI_API int32_t mmpSpiTerminate(SPI_PORT port);

//=============================================================================
/**
 * This routine is used to write data to targe device by dma.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpSpiDmaWrite()
 */
//=============================================================================
SPI_API int32_t
mmpSpiDmaWrite(
    SPI_PORT port,
    uint8_t* inCommand,
    uint32_t  inCommandSize,
    uint8_t* inData,
    uint32_t  inDataSize,
    uint8_t  dataLength);

//=============================================================================
/**
 * This routine is used to write data to targe device by dma.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpSpiDmaWrite()
 */
//=============================================================================
SPI_API int32_t
mmpSpiDmaWriteMulti(
    SPI_PORT port,
    uint8_t* inputData,
    int32_t  inputSize,
    uint8_t* psrc,
    int32_t  size,
    uint8_t  dataLength,
    uint32_t pageSize);

//=============================================================================
/**
 * This routine is used to read data from targe device by dma.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see mmpSpiDmaRead()
 */
//=============================================================================
SPI_API int32_t
mmpSpiDmaRead(
    SPI_PORT port,
    uint8_t* inputData,
    uint32_t inputSize,
    uint8_t* pdes,
    uint32_t size,
    uint8_t  dataLength);
 
SPI_API int32_t
mmpSpiDmaReadNoDropFirstByte(
    SPI_PORT port,
    uint8_t* inputData,
    int32_t  inputSize,
    void*    pdes,
    int32_t  size,
    uint8_t  dataLength);
 
//=============================================================================
/**
 * This routine is used to write data to targe device by pio.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
SPI_API int32_t
mmpSpiPioWrite(
    SPI_PORT port,
    uint8_t *inputData,
    uint32_t   inputSize,
    uint8_t *pbuf,
    uint32_t size,
    uint8_t dataLength);

//=============================================================================
/**
 * This routine is used to write data to targe device by pio but no mutex protect.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
SPI_API int32_t
mmpSpiPioWriteNoLock(
    SPI_PORT port,
    void*    inputData,
    uint32_t inputSize,
    void*    pbuf,
    uint32_t size,
    uint8_t  dataLength);

//=============================================================================
/**
 * This routine is used to read data from targe device by pio.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
SPI_API int32_t
mmpSpiPioRead(
    SPI_PORT port,
    uint8_t*    inputData,
    uint32_t inputSize,
    uint8_t*    outputBuf,
    uint32_t size,
    uint8_t  dataLength);
    
#if 0
SPI_API void
mmpSpiSetMode(
    SPI_PORT port,
    SPI_MODE mode);    
#endif


//=============================================================================
/**
 * This routine is used to set as master mode.
 *
 * @param port	port to master mode.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
SPI_API void
mmpSpiSetMaster(
    SPI_PORT port);

//=============================================================================
/**
 * This routine is used to set as slave mode.
 *
 * @param port	port to slave mode.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
SPI_API void
mmpSpiSetSlave(
    SPI_PORT port);

#if 0
SPI_API int
mmpSpiDmaTriggerRead(
    SPI_PORT	port,
    void*		pdes,
    int			size);
#endif

SPI_API void
mmpSpiSetControlMode(
    SPI_CONTROL_MODE controlMode);

SPI_API void
mmpSpiResetControl(
    void);    

SPI_API uint32_t
mmpSpiDmaTransfer(
    SPI_PORT port,
    void     *tx_buf,
    void     *rx_buf,
    uint32_t buflen);

SPI_API uint32_t
mmpSpiPioTransfer(
    SPI_PORT port,
    void     *tx_buf,
    void     *rx_buf,
    uint32_t buflen);

SPI_API uint32_t
mmpSpiTransfer(
    SPI_PORT port,
    void     *tx_buf,
    void     *rx_buf,
    uint32_t buflen);
void
mmpSpiInitShareGpioPin(
    unsigned int *pins);

//@}


#ifdef __cplusplus
}
#endif

#endif /* MMP_SPI_H */