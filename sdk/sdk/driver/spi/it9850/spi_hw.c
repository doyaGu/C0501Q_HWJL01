#include <stdint.h>
#include "spi_hw.h"
#include "spi_msg.h"
#include "ite/ith.h"

#ifndef _WIN32
#define CHECK_PORT(port)								\
do{														\
	if ((port != SPI0_BASE) && (port != SPI1_BASE))		\
	{													\
		SPI_ERROR_MSG("Unknown port(0x%08X)\n", port);	\
		while(1);										\
	}													\
}while(0);
#else
#define CHECK_PORT(port)
#endif

void spiSetTxEndian(SPI_HW_PORT port, SPI_ENDIAN_TYPE type)
{
    CHECK_PORT(port)

    switch (type)
    {
    case SPI_ENDIAN_LITTLE:
        ithWriteRegMaskA(port + REG_SPI_CONTROL_0, REG_CR0_TXENDIAN_LITTLE, REG_CR0_TXENDIAN_MASK);
        break;

    case SPI_ENDIAN_BIG:
        ithWriteRegMaskA(port + REG_SPI_CONTROL_0, REG_CR0_TXENDIAN_BIG, REG_CR0_TXENDIAN_MASK);
        break;

    default:
        SPI_ERROR_MSG("Unknown type(%d)!\n", type);
        break;
    }
}

void spiSetRxEndian(SPI_HW_PORT port, SPI_ENDIAN_TYPE type)
{
    CHECK_PORT(port)

    switch (type)
    {
    case SPI_ENDIAN_LITTLE:
        ithWriteRegMaskA(port + REG_SPI_CONTROL_0, REG_CR0_RXENDIAN_LITTLE, REG_CR0_RXENDIAN_MASK);
        break;

    case SPI_ENDIAN_BIG:
        ithWriteRegMaskA(port + REG_SPI_CONTROL_0, REG_CR0_RXENDIAN_BIG, REG_CR0_RXENDIAN_MASK);
        break;

    default:
        SPI_ERROR_MSG("Unknown type(%d)!\n", type);
        break;
    }
}

void spiSetFrameFormat(SPI_HW_PORT port)	// Always set to SPI mode
{
    CHECK_PORT(port)

    ithWriteRegMaskA(port + REG_SPI_CONTROL_0, REG_CR0_FFMT_SPI, REG_CR0_FFMT_MASK);
}

void spiSetFrameSyncPolarity(SPI_HW_PORT port, SPI_FS_POLARITY polarity)
{
	CHECK_PORT(port)

    switch (polarity)
    {
    case SPI_FS_ACTIVE_HIGH:
        ithWriteRegMaskA(port + REG_SPI_CONTROL_0, REG_CR0_FSPO_HIGH, REG_CR0_FSPO_MASK);
        break;

    case SPI_FS_ACTIVE_LOW:
        ithWriteRegMaskA(port + REG_SPI_CONTROL_2, REG_CR0_FSPO_LOW, REG_CR0_FSPO_MASK);
        break;

    default:
        SPI_ERROR_MSG("Unknown polarity(%d)!\n", polarity);
        break;
    }
}

void spiSetMasterMode(SPI_HW_PORT port)
{
	CHECK_PORT(port)

	ithWriteRegMaskA(port + REG_SPI_CONTROL_0, REG_CR0_OPM_MASTER, REG_CR0_OPM_MASK);
}

void spiSetSlaveMode(SPI_HW_PORT port)
{
	CHECK_PORT(port)

	ithWriteRegMaskA(port + REG_SPI_CONTROL_0, REG_CR0_OPM_SLAVE, REG_CR0_OPM_MASK);
}

void spiSetSCLKPolarity(SPI_HW_PORT port, SPI_SCLK_POLARITY polarity)
{
	CHECK_PORT(port)

    switch (polarity)
    {
    case SPI_SCLK_REMAIN_LOW:
        ithWriteRegMaskA(port + REG_SPI_CONTROL_0, REG_CR0_SCLKPO_LOW, REG_CR0_SCLKPO_MASK);
        break;

    case SPI_SCLK_REMAIN_HIGH:
        ithWriteRegMaskA(port + REG_SPI_CONTROL_0, REG_CR0_SCLKPO_HIGH, REG_CR0_SCLKPO_MASK);
        break;

    default:
        SPI_ERROR_MSG("Unknown polarity(%d)!\n", polarity);
        break;
    }
}

void spiSetSCLKPhase(SPI_HW_PORT port, SPI_SCLK_PHASE phase)
{
	CHECK_PORT(port)

    switch (phase)
    {
    case SPI_SCLK_PHASE_0:
        ithWriteRegMaskA(port + REG_SPI_CONTROL_0, REG_CR0_SCLKPH_DISABLE, REG_CR0_SCLKPH_MASK);
        break;

    case SPI_SCLK_REMAIN_HIGH:
        ithWriteRegMaskA(port + REG_SPI_CONTROL_0, REG_CR0_SCLKPH_ENABLE, REG_CR0_SCLKPH_MASK);
        break;

    default:
        SPI_ERROR_MSG("Unknown phase(%d)!\n", phase);
        break;
    }
}

void spiSetDataLength(SPI_HW_PORT port, uint32_t length)
{
    CHECK_PORT(port)

    if (length > REG_CR1_SDL_MAX_VALUE)
    {
        SPI_ERROR_MSG("length over %d!\n", REG_CR1_SDL_MAX_VALUE);
    }
    ithWriteRegMaskA(port + REG_SPI_CONTROL_1, length << REG_CR1_SDL_OFFSET, REG_CR1_SDL_MASK);
}

void spiSetClockDivider(SPI_HW_PORT port, uint32_t divider)
{
    CHECK_PORT(port)

    if (divider > REG_CR1_SCLKDIV_MAX_VALUE)
    {
        SPI_ERROR_MSG("divider over %d!\n", REG_CR1_SCLKDIV_MAX_VALUE);
    }
    ithWriteRegMaskA(port + REG_SPI_CONTROL_1, divider, REG_CR1_SCLKDIV_MASK);
}

void spiSetClockOutInv(SPI_HW_PORT port, SPI_OUT_CLK_INV inv)
{
	CHECK_PORT(port)
		
	switch(inv)
	{
	case SPI_OUT_CLK_NORMAL:
		ithWriteRegMaskA(port + REG_SPI_MISC, REG_CLKOUT_INV_DISABLE << REG_CLKOUT_INV_OFFSET, REG_CLKOUT_INV_MASK);
		break;
	case SPI_OUT_CLK_INVERSE:
		ithWriteRegMaskA(port + REG_SPI_MISC, REG_CLKOUT_INV_ENABLE << REG_CLKOUT_INV_OFFSET, REG_CLKOUT_INV_MASK);
		break;
	default:
		SPI_ERROR_MSG("Unknown inv(%d)!\n", inv);
        break;
	}
}

void spiSetClockOutBypass(SPI_HW_PORT port, SPI_OUT_CLK_BYPASS bypass)
{
	CHECK_PORT(port)
		
	switch(bypass)
	{
	case SPI_OUT_CLK_BYPASS_ENABLE:
		ithWriteRegMaskA(port + REG_SPI_MISC, REG_CLKOUT_BYPASS_ENABLE << REG_CLKOUT_BYPASS_OFFSET, REG_CLKOUT_BYPASS_MASK);
		break;
	case SPI_OUT_CLK_BYPASS_DISABLE:
		ithWriteRegMaskA(port + REG_SPI_MISC, REG_CLKOUT_BYPASS_DISABLE << REG_CLKOUT_BYPASS_OFFSET, REG_CLKOUT_BYPASS_MASK);
		break;
	default:
		SPI_ERROR_MSG("Unknown bypass(%d)!\n", bypass);
	    break;	
	}
}

void spiSetClockDelay(SPI_HW_PORT port, int delay)
{
	CHECK_PORT(port)
	ithWriteRegMaskA(port + REG_SPI_MISC, delay << REG_CLKOUT_DALAY_OFFSET, REG_CLKOUT_DALAY_MASK);
}


void spiSetCLKSRC(SPI_HW_PORT port, SPI_CLK_SRC clk_src)
{
	CHECK_PORT(port)
	ithWriteRegMaskA(port + REG_SPI_MISC, clk_src << REG_CLKSRC_OFFSET, REG_CLKSRC_MASK);	
}

void spiClearTxFifo(SPI_HW_PORT port, SPI_FIFO_STATE state)
{
    CHECK_PORT(port)

    switch (state)
    {
    case SPI_FIFO_CLEAR:
        ithWriteRegMaskA(port + REG_SPI_CONTROL_2, REG_CR2_TXFCLR_ENABLE, REG_CR2_TXFCLR_MASK);
        break;

    case SPI_FIFO_NORMAL:
        ithWriteRegMaskA(port + REG_SPI_CONTROL_2, REG_CR2_TXFCLR_NO_EFFECT, REG_CR2_TXFCLR_MASK);
        break;

    default:
        SPI_ERROR_MSG("Unknown state(%d)!\n", state);
        break;
    }
}

void spiClearRxFifo(SPI_HW_PORT port, SPI_FIFO_STATE state)
{
    CHECK_PORT(port)

    switch (state)
    {
    case SPI_FIFO_CLEAR:
        ithWriteRegMaskA(port + REG_SPI_CONTROL_2, REG_CR2_RXFCLR_ENABLE, REG_CR2_RXFCLR_MASK);
        break;

    case SPI_FIFO_NORMAL:
        ithWriteRegMaskA(port + REG_SPI_CONTROL_2, REG_CR2_RXFCLR_NO_EFFECT, REG_CR2_RXFCLR_MASK);
        break;

    default:
        SPI_ERROR_MSG("Unknown state(%d)!\n", state);
        break;
    }
}

void spiTransmitDataOutput(SPI_HW_PORT port, bool enable)
{
    CHECK_PORT(port)

    ithWriteRegMaskA(
        port + REG_SPI_CONTROL_2,
        (enable == true) ? REG_CR2_TXDOE_ENABLE : REG_CR2_TXDOE_DISABLE,
        REG_CR2_TXDOE_MASK);
}

void spiEngineStart(SPI_HW_PORT port)
{
    CHECK_PORT(port)

    ithWriteRegMaskA(port + REG_SPI_CONTROL_2, REG_CR2_SPI_ENABLE, REG_CR2_SPI_MASK);
}

void spiEngineStop(SPI_HW_PORT port)
{
    CHECK_PORT(port)

    ithWriteRegMaskA(port + REG_SPI_CONTROL_2, REG_CR2_SPI_DISABLE, REG_CR2_SPI_MASK);
}

int spiGetTxFifoValidCount(SPI_HW_PORT port)
{
    CHECK_PORT(port)

    uint32_t val = ithReadRegA(port + REG_SPI_STATUS);
    uint32_t retVal = (val & REG_STR_TFVE_MASK) >> REG_STR_TFVE_OFFSET;

    return retVal;
}

int spiGetRxFifoValidCount(SPI_HW_PORT port)
{
    CHECK_PORT(port)

    uint32_t val = ithReadRegA(port + REG_SPI_STATUS);
    uint32_t retVal = (val & REG_STR_RFVE_MASK) >> REG_STR_RFVE_OFFSET;

    return retVal;
}

bool spiIsBusy(SPI_HW_PORT port)
{
    uint32_t val = ithReadRegA(port + REG_SPI_STATUS);
    uint32_t retVal = (val & REG_STR_BUSY_MASK) >> REG_STR_BUSY_OFFSET;

    return (retVal == 1);
}

bool spiIsTxFifoFull(SPI_HW_PORT port)
{
    CHECK_PORT(port)

    uint32_t val = ithReadRegA(port + REG_SPI_STATUS);
    uint32_t retVal = (val & REG_STR_TFNF_MASK) >> REG_STR_TFNF_OFFSET;

    return (retVal == 0);
}

bool spiIsRxFifoFull(SPI_HW_PORT port)
{
    CHECK_PORT(port)

    uint32_t val = ithReadRegA(port + REG_SPI_STATUS);
    uint32_t retVal = (val & REG_STR_RFF_MASK);

    return (retVal == 1);
}

void spiSetTxFifoThreshold(SPI_HW_PORT port, uint32_t threshold)
{
	CHECK_PORT(port)

	if (threshold > REG_ICR_TFTHOD_MAX)
    {
        SPI_ERROR_MSG("threshold over %d!\n", REG_ICR_TFTHOD_MAX);
    }
    ithWriteRegMaskA(port + REG_SPI_INTR_CONTROL, threshold << REG_ICR_TFTHOD_OFFSET, REG_ICR_TFTHOD_MASK);
}

void spiSetRxFifoThreshold(SPI_HW_PORT port, uint32_t threshold)
{
	CHECK_PORT(port)

	if (threshold > REG_ICR_RFTHOD_MAX)
    {
        SPI_ERROR_MSG("threshold over %d!\n", REG_ICR_RFTHOD_MAX);
    }
    ithWriteRegMaskA(port + REG_SPI_INTR_CONTROL, threshold << REG_ICR_RFTHOD_OFFSET, REG_ICR_RFTHOD_MASK);
}

void spiTxDmaRequestEnable(SPI_HW_PORT port)
{
	CHECK_PORT(port)

    ithWriteRegMaskA(port + REG_SPI_INTR_CONTROL, REG_ICR_TX_DMA_ENABLE, REG_ICR_TX_DMA_MASK);
}

void spiTxDmaRequestDisable(SPI_HW_PORT port)
{
	CHECK_PORT(port)

    ithWriteRegMaskA(port + REG_SPI_INTR_CONTROL, REG_ICR_TX_DMA_DISABLE, REG_ICR_TX_DMA_MASK);
}

void spiRxDmaRequestEnable(SPI_HW_PORT port)
{
	CHECK_PORT(port)

    ithWriteRegMaskA(port + REG_SPI_INTR_CONTROL, REG_ICR_RX_DMA_ENABLE, REG_ICR_RX_DMA_MASK);
}

void spiRxDmaRequestDisable(SPI_HW_PORT port)
{
	CHECK_PORT(port)

    ithWriteRegMaskA(port + REG_SPI_INTR_CONTROL, REG_ICR_RX_DMA_DISABLE, REG_ICR_RX_DMA_MASK);
}

void spiTxThresholdInterruptEnable(SPI_HW_PORT port)
{
	CHECK_PORT(port)

    ithWriteRegMaskA(port + REG_SPI_INTR_CONTROL, REG_ICR_TX_THRESHOLD_INTR_ENABLE, REG_ICR_TX_THRESHOLD_INTR_MASK);
}

void spiTxThresholdInterruptDisable(SPI_HW_PORT port)
{
	CHECK_PORT(port)

	ithWriteRegMaskA(port + REG_SPI_INTR_CONTROL, REG_ICR_TX_THRESHOLD_INTR_DISABLE, REG_ICR_TX_THRESHOLD_INTR_MASK);
}

void spiRxThresholdInterruptEnable(SPI_HW_PORT port)
{
	CHECK_PORT(port)

	ithWriteRegMaskA(port + REG_SPI_INTR_CONTROL, REG_ICR_RX_THRESHOLD_INTR_ENABLE, REG_ICR_RX_THRESHOLD_INTR_MASK);
}

void spiRxThresholdInterruptDisable(SPI_HW_PORT port)
{
	CHECK_PORT(port)

	ithWriteRegMaskA(port + REG_SPI_INTR_CONTROL, REG_ICR_RX_THRESHOLD_INTR_DISABLE, REG_ICR_RX_THRESHOLD_INTR_MASK);
}

void spiTxUnderrunInteruptEnable(SPI_HW_PORT port)
{
	CHECK_PORT(port)

	ithWriteRegMaskA(port + REG_SPI_INTR_CONTROL, REG_ICR_TX_UNDERRUN_INTR_ENABLE, REG_ICR_TX_UNDERRUN_INTR_MASK);
}

void spiTxUnderrunInteruptDisable(SPI_HW_PORT port)
{
	CHECK_PORT(port)

	ithWriteRegMaskA(port + REG_SPI_INTR_CONTROL, REG_ICR_TX_UNDERRUN_INTR_DISABLE, REG_ICR_TX_UNDERRUN_INTR_MASK);
}

void spiRxOverrunInteruptEnable(SPI_HW_PORT port)
{
	CHECK_PORT(port)

	ithWriteRegMaskA(port + REG_SPI_INTR_CONTROL, REG_ICR_RX_OVERRUN_INTR_ENABLE, REG_ICR_RX_OVERRUN_INTR_MASK);
}

void spiRxOverrunInteruptDisable(SPI_HW_PORT port)
{
	CHECK_PORT(port)

	ithWriteRegMaskA(port + REG_SPI_INTR_CONTROL, REG_ICR_RX_OVERRUN_INTR_DISABLE, REG_ICR_RX_OVERRUN_INTR_MASK);
}

void spiWriteData(SPI_HW_PORT port, uint32_t data)
{
	CHECK_PORT(port)

	SPI_VERBOSE_LOG("Write 0x%02X\n", data);

	ithWriteRegA(port + REG_SPI_DATA, data);
}

uint32_t spiReadData(SPI_HW_PORT port)
{
	CHECK_PORT(port)

	return ithReadRegA(port + REG_SPI_DATA);
}

void spiSetFreeRunCount(SPI_HW_PORT port, uint32_t freeRunCount)
{
    CHECK_PORT(port)

    if (freeRunCount > REG_TXFR_FRUN_COUNT_MAX_VALUE)
    {
        SPI_ERROR_MSG("freeRunCount over %d!\n", REG_TXFR_FRUN_COUNT_MAX_VALUE);
    }
    ithWriteRegMaskA(port + REG_SPI_FREERUN, freeRunCount << REG_TXFR_FRUN_COUNT_OFFSET, REG_TXFR_FRUN_COUNT_MASK);
}

void spiGarbageFilterEnable(SPI_HW_PORT port)
{
       CHECK_PORT(port)

       ithWriteRegMaskA(port + REG_SPI_FREERUN, REG_TXFR_FRUN_FILTER_ENABLE, REG_TXFR_FRUN_FILTER_MASK);
}

void spiGarbageFilterDisable(SPI_HW_PORT port)
{
       CHECK_PORT(port)

       ithWriteRegMaskA(port + REG_SPI_FREERUN, REG_TXFR_FRUN_FILTER_DISABLE, REG_TXFR_FRUN_FILTER_MASK);
}

void spiSetGarbageFilterLength(SPI_HW_PORT port, uint32_t filterLength)
{
       CHECK_PORT(port)

    if (filterLength > REG_TXFR_FRUN_FILTER_LENG_MAX_VALUE)
    {
        SPI_ERROR_MSG("filterLength over %d!\n", REG_TXFR_FRUN_FILTER_LENG_MAX_VALUE);
    }
    ithWriteRegMaskA(port + REG_SPI_FREERUN, filterLength << REG_TXFR_FRUN_FILTER_LENG_OFFSET, REG_TXFR_FRUN_FILTER_LENG_MASK);
}

void spiSetCSN(SPI_HW_PORT port, SPI_CSN_STATE state)
{
    CHECK_PORT(port)

    switch (state)
    {
    case SPI_CSN_NORMAL:
        ithWriteRegMaskA(port + REG_SPI_FREERUN, REG_TXFR_CSNCLR_NORMAL, REG_TXFR_CSNCLR_MASK);
        break;

    case SPI_CSN_ALWAYS_ZERO:
        ithWriteRegMaskA(port + REG_SPI_FREERUN, REG_TXFR_CSNCLR_ALWAYS_ZERO, REG_TXFR_CSNCLR_MASK);
        break;

    default:
        SPI_ERROR_MSG("Unknown state(%d)!\n", state);
        break;
    }
}

void spiSetFreeRunRxLock(SPI_HW_PORT port, SPI_RXLOCK_STATE state)
{
    CHECK_PORT(port)

    switch (state)
    {
    case SPI_RXLOCK_SEND_UNTIL_FREE_COUNT_ZERO:
        ithWriteRegMaskA(port + REG_SPI_FREERUN, REG_TXFR_FRUN_RXLOCK_0, REG_TXFR_FRUN_RXLOCK_MASK);
        break;

    case SPI_RXLOCK_STOP_UNTIL_RXFIF_THAN_TXFIFO:
        ithWriteRegMaskA(port + REG_SPI_FREERUN, REG_TXFR_FRUN_RXLOCK_1, REG_TXFR_FRUN_RXLOCK_MASK);
        break;

    default:
        SPI_ERROR_MSG("Unknown state(%d)!\n", state);
        break;
    }
}

void spiFreeRunEnable(SPI_HW_PORT port)
{
	CHECK_PORT(port)

	ithWriteRegMaskA(port + REG_SPI_FREERUN, REG_TXFR_FRUN_FIRE_ENABLE, REG_TXFR_FRUN_FIRE_MASK);
}

void spiFreeRunDisable(SPI_HW_PORT port)
{
	CHECK_PORT(port)

	ithWriteRegMaskA(port + REG_SPI_FREERUN, REG_TXFR_FRUN_FIRE_DISABLE, REG_TXFR_FRUN_FIRE_MASK);
}

uint32_t spiGetMajorVersion(SPI_HW_PORT port)
{
    CHECK_PORT(port)

    uint32_t val = ithReadRegA(port + REG_SPI_REVERSION);

    return ((val & REG_REV_MAJOR_REV_MASK) >> REG_REV_MAJOR_REV_OFFSET);
}

uint32_t spiGetMinorVersion(SPI_HW_PORT port)
{
    CHECK_PORT(port)

    uint32_t val = ithReadRegA(port + REG_SPI_REVERSION);

    return ((val & REG_REV_MINOR_REV_MASK) >> REG_REV_MINOR_REV_OFFSET);
}

uint32_t spiGetReleaseVersion(SPI_HW_PORT port)
{
    CHECK_PORT(port)

    uint32_t val = ithReadRegA(port + REG_SPI_REVERSION);

    return (val & REG_REV_RELEASE_REV_MASK);
}

