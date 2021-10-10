#ifndef __SPI_HW_H__
#define __SPI_HW_H__

#include <stdbool.h>
#include <stdint.h>
#include "spi_reg.h"

typedef enum
{
	SPI_HW_PORT_0 = SPI0_BASE,
	SPI_HW_PORT_1 = SPI1_BASE
}SPI_HW_PORT;

typedef enum
{
	SPI_CSN_NORMAL,
	SPI_CSN_ALWAYS_ZERO
}SPI_CSN_STATE;

typedef enum
{
	SPI_RXLOCK_SEND_UNTIL_FREE_COUNT_ZERO,
	SPI_RXLOCK_STOP_UNTIL_RXFIF_THAN_TXFIFO,
}SPI_RXLOCK_STATE;

typedef enum
{
	SPI_ENDIAN_LITTLE,
	SPI_ENDIAN_BIG,
}SPI_ENDIAN_TYPE;

typedef enum
{
	SPI_FIFO_CLEAR,
	SPI_FIFO_NORMAL,
}SPI_FIFO_STATE;

typedef enum
{
	SPI_FS_ACTIVE_HIGH,
	SPI_FS_ACTIVE_LOW
}SPI_FS_POLARITY;

typedef enum
{
	SPI_SCLK_REMAIN_LOW,
	SPI_SCLK_REMAIN_HIGH
}SPI_SCLK_POLARITY;

typedef enum
{
	SPI_SCLK_PHASE_0,
	SPI_SCLK_PHASE_1
}SPI_SCLK_PHASE;

typedef enum
{
	SPI_OUT_CLK_NORMAL,
	SPI_OUT_CLK_INVERSE	
}SPI_OUT_CLK_INV;

typedef enum
{
	SPI_OUT_CLK_BYPASS_DISABLE,
	SPI_OUT_CLK_BYPASS_ENABLE	
}SPI_OUT_CLK_BYPASS;

typedef enum
{
	AMCLK,
	W8CLK
}SPI_CLK_SRC;

void spiSetTxEndian(SPI_HW_PORT port, SPI_ENDIAN_TYPE type);
void spiSetRxEndian(SPI_HW_PORT port, SPI_ENDIAN_TYPE type);
void spiSetFrameFormat(SPI_HW_PORT port);	// Always set to SPI mode
void spiSetFrameSyncPolarity(SPI_HW_PORT port, SPI_FS_POLARITY polarity);
void spiSetMasterMode(SPI_HW_PORT port);
void spiSetSlaveMode(SPI_HW_PORT port);
void spiSetSCLKPolarity(SPI_HW_PORT port, SPI_SCLK_POLARITY polarity);
void spiSetSCLKPhase(SPI_HW_PORT port, SPI_SCLK_PHASE phase);
void spiSetDataLength(SPI_HW_PORT port, uint32_t length);
void spiSetClockDivider(SPI_HW_PORT port, uint32_t divider);
void spiClearTxFifo(SPI_HW_PORT port, SPI_FIFO_STATE state);
void spiClearRxFifo(SPI_HW_PORT port, SPI_FIFO_STATE state);
void spiTransmitDataOutput(SPI_HW_PORT port, bool enable);
void spiEngineStart();
void spiEngineStop();
int spiGetTxFifoValidCount(SPI_HW_PORT port);
int spiGetRxFifoValidCount(SPI_HW_PORT port);
bool spiIsBusy(SPI_HW_PORT port);
bool spiIsTxFifoFull(SPI_HW_PORT port);
bool spiIsRxFifoFull(SPI_HW_PORT port);
void spiSetTxFifoThreshold(SPI_HW_PORT port, uint32_t threshold);
void spiSetRxFifoThreshold(SPI_HW_PORT port, uint32_t threshold);
void spiTxDmaRequestEnable(SPI_HW_PORT port);
void spiTxDmaRequestDisable(SPI_HW_PORT port);
void spiRxDmaRequestEnable(SPI_HW_PORT port);
void spiRxDmaRequestDisable(SPI_HW_PORT port);
void spiTxThresholdInterruptEnable(SPI_HW_PORT port);
void spiTxThresholdInterruptDisable(SPI_HW_PORT port);
void spiRxThresholdInterruptEnable(SPI_HW_PORT port);
void spiRxThresholdInterruptDisable(SPI_HW_PORT port);
void spiTxUnderrunInteruptEnable(SPI_HW_PORT port);
void spiTxUnderrunInteruptDisable(SPI_HW_PORT port);
void spiRxOverrunInteruptEnable(SPI_HW_PORT port);
void spiRxOverrunInteruptDisable(SPI_HW_PORT port);
void spiWriteData(SPI_HW_PORT port, uint32_t data);
uint32_t spiReadData(SPI_HW_PORT port);
void spiSetFreeRunCount(SPI_HW_PORT port, uint32_t freeRunCount);
void spiGarbageFilterEnable(SPI_HW_PORT port);
void spiGarbageFilterDisable(SPI_HW_PORT port);
void spiSetGarbageFilterLength(SPI_HW_PORT port, uint32_t filterLength);
void spiSetCSN(SPI_HW_PORT port, SPI_CSN_STATE state);
void spiSetFreeRunRxLock(SPI_HW_PORT port, SPI_RXLOCK_STATE state);
void spiFreeRunEnable(SPI_HW_PORT port);
void spiFreeRunDisable(SPI_HW_PORT port);
uint32_t spiGetMajorVersion(SPI_HW_PORT port);
uint32_t spiGetMinorVersion(SPI_HW_PORT port);
uint32_t spiGetReleaseVersion(SPI_HW_PORT port);
void spiSetClockOutInv(SPI_HW_PORT port, SPI_OUT_CLK_INV inv);
void spiSetClockOutBypass(SPI_HW_PORT port, SPI_OUT_CLK_BYPASS bypass);
void spiSetClockDelay(SPI_HW_PORT port, int delay);


#endif  // __SPI_HW_H__
