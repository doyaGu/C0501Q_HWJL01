#ifndef __AXISPI_HW_H__
#define __AXISPI_HW_H__

#include <stdbool.h>
#include <stdint.h>
#include "axispi_reg.h"

typedef enum
{
	SPI_HW_PORT_0 = REG_SPI_BASE,	
}SPI_HW_PORT;

uint32_t AxiSPIReadAddr(void);
void AxiSpiWriteAddr(uint32_t data);
void AxiSpiWriteInstLength(uint32_t data);
void AxiSpiWriteAddrLength(uint32_t data);
void AxiSpiWriteDataCounter(uint32_t data);
void AxiSpiWriteData(uint32_t data);
uint32_t AxiSpiReadData(void);
uint32_t AxiSpiReadDataCounter(void);
uint32_t AxiSpiReadTXFIFOStatus(void);
uint32_t AxiSpiReadTXFIFOValidSpace(void);
uint32_t AxiSpiReadRXFIFODataCount(void);
uint32_t AxiSpiReadIntrStatus(void);
void AxiSpiSetIntrStatus(void);
void AxiSpiWriteInstCodeRead(uint32_t data);
void AxiSpiWriteInstCode4bitRead(uint32_t data);
void AxiSpiWriteInstCodeWriteEnable(uint32_t data);
void AxiSpiWriteInstCodeWriteDisable(uint32_t data);
void AxiSpiWriteInstCode4bitWriteEnable(uint32_t data);
void AxiSpiWriteInstCode4bitWriteDisable(uint32_t data);
void AxiSpiSelectChipSelect(uint32_t data);
void AxiSpiSetOperateMode(uint32_t data);
void AxiSpiDTRModeEnable(void);
void AxiSpiSetClkDiv(uint32_t data);
void AxiSpiSetBusyBit(uint32_t data);
void AxiSpiWriteEnable(void);
void AxiSpiWriteDisable(void);
void AxiSpiReadStatusEnable(void);
void AxiSpiDMAEnable(void);
void AxiSpiDMADisable(void);
void AxiSpiCmdCompletIntrEnable(void);

#endif  // __AXISPI_HW_H__