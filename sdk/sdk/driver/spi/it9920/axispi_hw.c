#include <stdint.h>
#include "axispi_hw.h"
//#include "axispi_msg.h"
#include "ite/ith.h"


void SPIRunRDX1()
{
	//ithWriteRegMaskA(AXISPI_BASE+0x00,address);//SPI Flash Address.		 
	//ithWriteRegMaskA(AXISPI_BASE+0x04,0x01000003);//Instruction length, address length(3 byte).		  
	//ithWriteRegMaskA(AXISPI_BASE+0x08,datasize);//Data Counter. (1 byte = 8bit) 												//		  
	//ithWriteRegMaskA(AXISPI_BASE+0x0c,0x00000000+(cmd << 24) );//SPI command(02h, X1 PP)
}

uint32_t AxiSPIReadAddr(void)
{
	return ithReadRegA(SPI_REG_CMD_W0);
}

void AxiSpiWriteAddr(uint32_t data)
{
	//printf("WriteAddr 0x%02X\n", data);
	ithWriteRegA(SPI_REG_CMD_W0, data);	
}

void AxiSpiWriteInstLength(uint32_t data)
{
	//printf("InstLength 0x%02X\n", data);
	ithWriteRegMaskA(SPI_REG_CMD_W1, (data << SPI_SHT_INST_LENGTH) , SPI_MSK_INST_LENGTH);
}

void AxiSpiWriteAddrLength(uint32_t data)
{
	//printf("AddrLength 0x%02X\n", data);
	ithWriteRegMaskA(SPI_REG_CMD_W1, (data << SPI_SHT_ADDR_LENGTH) , SPI_MSK_ADDR_LENGTH);
}

void AxiSpiWriteDummy(uint32_t data)
{
	ithWriteRegMaskA(SPI_REG_CMD_W1, (data << SPI_SHT_DUMMY_CYCLE) , SPI_MSK_DUMMY_CYCLE);
}

void AxiSpiWriteDataCounter(uint32_t data)
{
	//printf("DataCounter 0x%02X\n", data);
	ithWriteRegA(SPI_REG_CMD_W2, data);
}

void AxiSpiWriteData(uint32_t data)
{
	//printf("WData 0x%02X\n", data);
	ithWriteRegA(SPI_REG_DATA_PORT, data);
}

uint32_t AxiSpiReadData(void)
{
	return ithReadRegA(SPI_REG_DATA_PORT);
}


uint32_t AxiSpiReadDataCounter(void)
{
	return ithReadRegA(SPI_REG_CMD_W2);
}

uint32_t AxiSpiReadTXFIFOStatus(void)
{
	uint32_t value;
	value = ithReadRegA(SPI_REG_STATUS) & SPI_MSK_TXFIFO_READY;
	if(!value)
		printf("TX FIFO FULL!\n");
	
	return value;

}

uint32_t AxiSpiReadRXFIFOStatus(void)
{
	uint32_t value;
	value = ithReadRegA(SPI_REG_STATUS) & SPI_MSK_RXFIFO_READY;
	if(!value)
		printf("RX FIFO FULL!\n");
	
	return value;

}

uint32_t AxiSpiReadTXFIFOValidSpace(void)
{
	uint32_t value;
	value = (ithReadRegA(SPI_REG_STATUS) & SPI_MSK_TXFIFO_VALID_SPACE) >> SPI_SHT_TXFIFO_VALID_SPACE;
	if(!value)
		printf("TX FIFO is full!!\n");
	return value;

}

uint32_t AxiSpiReadRXFIFODataCount(void)
{
	uint32_t value;
	value = (ithReadRegA(SPI_REG_STATUS) & SPI_MSK_RXFIFO_DATA_COUNT) >> SPI_SHT_RXFIFO_DATA_COUNT;
	
	return value;

}


uint32_t AxiSpiReadIntrStatus(void)
{
	uint32_t value=0;
	value = (ithReadRegA(SPI_REG_INTR_STATUS) & SPI_MSK_CMD_CMPLT_STATUS);
	//printf("ReadIntr=0x%x\n",value);
	return value;
}

void AxiSpiSetIntrStatus(void)
{
	ithWriteRegMaskA(SPI_REG_INTR_STATUS, 0x1,SPI_MSK_CMD_CMPLT_STATUS);
}

void AxiSpiWriteInstCodeRead(uint32_t data)
{
	uint32_t value=0;

	value |= (data <<SPI_SHT_INST_CODE);
	//printf("WriteInstCode data=0x%02X,value=0x%x\n", data,value);
	ithWriteRegA(SPI_REG_CMD_W3, value);
}

void AxiSpiWriteInstCode4bitRead(uint32_t data)
{
	uint32_t value=0;

	value |= (data <<SPI_SHT_INST_CODE);
	value |= (0x4 << SPI_SHT_OPERATE_MODE);
	//printf("WriteInstCode data=0x%02X,value=0x%x\n", data,value);
	ithWriteRegA(SPI_REG_CMD_W3, value);
}


void AxiSpiWriteInstCodeReadStatusEnable(uint32_t data)
{
	uint32_t value=0;

	value |=  (data << SPI_SHT_INST_CODE);
	value |=  (0x1 << SPI_SHT_EN_READ_STATUS);
	//printf("AxiSpiWriteInstCodeReadStatusEnable data=0x%02X,value=0x%x\n", data,value);
	ithWriteRegA(SPI_REG_CMD_W3, value);
}

void AxiSpiWriteInstCodeReadStatus(uint32_t data)
{
	uint32_t value=0;

	value |=  (data << SPI_SHT_INST_CODE);
	value |=  (0x1 << SPI_SHT_READ_STATUS);
	//printf("AxiSpiWriteInstCodeReadStatus data=0x%02X,value=0x%x\n", data,value);
	ithWriteRegA(SPI_REG_CMD_W3, value);
}

void AxiSpiWriteInstCodeWriteEnable(uint32_t data)
{
	uint32_t value=0;

	value |= (data << SPI_SHT_INST_CODE);
	value |= (0x1 << SPI_SHT_EN_WRITE);
	//printf("AxiSpiWriteInstCodeWriteEnable data=0x%02X,value=0x%x\n", data,value);
	ithWriteRegA(SPI_REG_CMD_W3, value);
}

void AxiSpiWriteInstCodeWriteDisable(uint32_t data)
{
	uint32_t value;

	value |= (data << SPI_SHT_INST_CODE);
	value |= (0x0 << SPI_SHT_EN_WRITE);
	//printf("AxiSpiWriteInstCodeWriteDisable data=0x%02X,value=0x%x\n", data,value);
	ithWriteRegA(SPI_REG_CMD_W3, value);
}

void AxiSpiWriteInstCode4bitWriteEnable(uint32_t data)
{
	uint32_t value=0;

	value |= (data << SPI_SHT_INST_CODE);
	value |= (0x1 << SPI_SHT_EN_WRITE);
	value |= (0x4 << SPI_SHT_OPERATE_MODE);
	//printf("AxiSpiWriteInstCode4bitWriteEnable data=0x%02X,value=0x%x\n", data,value);
	ithWriteRegA(SPI_REG_CMD_W3, value);
}

void AxiSpiWriteInstCode4bitWriteDisable(uint32_t data)
{
	uint32_t value;

	value |= (data << SPI_SHT_INST_CODE);
	value |= (0x0 << SPI_SHT_EN_WRITE);
	value |= (0x4 << SPI_SHT_OPERATE_MODE);
	printf("AxiSpiWriteInstCodeWriteDisable data=0x%02X,value=0x%x\n", data,value);
	ithWriteRegA(SPI_REG_CMD_W3, value);
}




void AxiSpiSelectChipSelect(uint32_t data)
{
	printf("AxiSpiSelectChipSelect 0x%02X\n", data);
	ithWriteRegMaskA(SPI_REG_CMD_W3, (data << SPI_SHT_START_CS) ,SPI_MSK_START_CS);
}

void AxiSpiSetOperateMode(uint32_t data)
{
	printf("AxiSpiSetOperateMode 0x%02X\n", data);
	ithWriteRegMaskA(SPI_REG_CMD_W3, (data << SPI_SHT_OPERATE_MODE) ,SPI_MSK_OPERATE_MODE);
}

void AxiSpiDTRModeEnable(void)
{
	printf("AxiSpiDTRModeEnable\n");
	ithWriteRegMaskA(SPI_REG_CMD_W3, (0x1 << SPI_SHT_EN_DTR_MODE) ,SPI_MSK_EN_DTR_MODE);
}

void AxiSpiSetClkDiv(uint32_t data)
{
	printf("AxiSpiSetClkDivider 0x%02X\n", data);
	ithWriteRegMaskA(SPI_REG_CTL, (data << SPI_SHT_CLK_DIV) ,SPI_MSK_CLK_DIV);
}

void AxiSpiSetBusyBit(uint32_t data)
{
	printf("AxiSpiSetBusyBit 0x%02X\n", data);
	ithWriteRegMaskA(SPI_REG_CTL, (data << SPI_SHT_RDY_LOC) ,SPI_MSK_RDY_LOC);
}

#if 0
void AxiSpiWriteEnable(void)
{
	printf("AxiSpiWriteEnable\n");
	ithWriteRegMaskA(SPI_REG_CMD_W3, (0x1 << SPI_SHT_EN_WRITE) ,SPI_MSK_EN_WRITE);
}

void AxiSpiWriteDisable(void)
{
	printf("AxiSpiWriteDisable\n");
	ithWriteRegMaskA(SPI_REG_CMD_W3, (0x0 << SPI_SHT_EN_WRITE) ,SPI_MSK_EN_WRITE);
}

void AxiSpiReadStatusEnable(void)
{
	printf("AxiSpiReadStatusEnable\n");
	ithWriteRegMaskA(SPI_REG_CMD_W3, (0x1 << SPI_SHT_EN_READ_STATUS) ,SPI_MSK_EN_READ_STATUS);
}
#endif


void AxiSpiDMAEnable(void)
{
	//printf("AxiSpiDMAEnable\n");
	ithWriteRegMaskA(SPI_REG_INTR_CTL, 0x1 ,N01_BITS_MSK);
}

void AxiSpiDMADisable(void)
{
	//printf("AxiSpiDMADisable\n");
	ithWriteRegMaskA(SPI_REG_INTR_CTL, 0x0 ,N01_BITS_MSK);
}


void AxiSpiCmdCompletIntrEnable(void)
{
	printf("AxiSpiCmdCompletIntrEnable\n");
	ithWriteRegMaskA(SPI_REG_INTR_CTL, (0x1 << SPI_SHT_EN_CMD_CMPLT_INTR)  ,SPI_MSK_EN_CMD_CMPLT_INTR);
}

