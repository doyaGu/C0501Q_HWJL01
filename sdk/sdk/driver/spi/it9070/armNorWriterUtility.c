#include "../../include/ssp/armNorWriterUtility.h"

void ArmNorWriterCleanState()
{
	ithWriteRegH(ARM_NOR_WRITER_STATUS,         ANW_STATUS_NOT_READY);
	ithWriteRegH(ARM_NOR_WRITER_ROM_BUF_ADDR_L, 0);
	ithWriteRegH(ARM_NOR_WRITER_ROM_BUF_ADDR_H, 0);
	ithWriteRegH(ARM_NOR_WRITER_RB_BUF_ADDR_L,  0);
	ithWriteRegH(ARM_NOR_WRITER_RB_BUF_ADDR_H,  0);
	ithWriteRegH(ARM_NOR_WRITER_ROM_LENGTH_L,   0);
	ithWriteRegH(ARM_NOR_WRITER_ROM_LENGTH_H,   0);
	ithWriteRegH(ARM_NOR_WRITER_APP_STATUS,     ANW_APP_STATUS_OK);
}

void SetArmNorWriterStatus(
	ANW_STATUS writerStatus)
{
	ithWriteRegH(ARM_NOR_WRITER_STATUS, writerStatus);
}

ANW_STATUS GetArmNorWriterStatus()
{
	uint16_t value = 0;
	
	value = ithReadRegH(ARM_NOR_WRITER_STATUS);

	return (ANW_STATUS)value;
}

void SetArmNorWriterAppStatus(
	ANW_APP_STATUS appStatus)
{
	ithWriteRegH(ARM_NOR_WRITER_APP_STATUS, appStatus);
}

ANW_APP_STATUS GetArmNorWriterAppStatus()
{
	uint16_t value = 0;
	
	value = ithReadRegH(ARM_NOR_WRITER_APP_STATUS);

	return (ANW_APP_STATUS)value;
}

void SetArmNorWriterBufLength(uint32_t romBufLength)
{
	ithWriteRegH(ARM_NOR_WRITER_ROM_LENGTH_L, (uint16_t)(romBufLength & 0xFFFF));
	ithWriteRegH(ARM_NOR_WRITER_ROM_LENGTH_H, (uint16_t)(romBufLength >> 16));
}

uint32_t GetArmNorWriterBufLength()
{
	uint16_t value1 = 0;
	uint16_t value2 = 0;

	value1 = ithReadRegH(ARM_NOR_WRITER_ROM_LENGTH_L);
	value2 = ithReadRegH(ARM_NOR_WRITER_ROM_LENGTH_H);

	return (((uint32_t)value2) << 16 | value1);
}

void SetArmNorWriterRomBufAddr(uint32_t romBufAddr)
{
	ithWriteRegH(ARM_NOR_WRITER_ROM_BUF_ADDR_L, (uint16_t)(romBufAddr & 0xFFFF));
	ithWriteRegH(ARM_NOR_WRITER_ROM_BUF_ADDR_H, (uint16_t)(romBufAddr >> 16));
}

uint32_t GetArmNorWriterRomBufAddr()
{
	uint16_t value1 = 0;
	uint16_t value2 = 0;

	value1 = ithReadRegH(ARM_NOR_WRITER_ROM_BUF_ADDR_L);
	value2 = ithReadRegH(ARM_NOR_WRITER_ROM_BUF_ADDR_H);

	return (((uint32_t)value2) << 16 | value1);
}

void SetArmNorWriterRbBufAddr(uint32_t rbBufAddr)
{
	ithWriteRegH(ARM_NOR_WRITER_RB_BUF_ADDR_L, (uint16_t)(rbBufAddr & 0xFFFF));
	ithWriteRegH(ARM_NOR_WRITER_RB_BUF_ADDR_H, (uint16_t)(rbBufAddr >> 16));
}

void SetArmNorWriterAppProgressMin(uint32_t barMin)
{
	ithWriteRegH(ARM_NOR_WRITER_APP_BAR_MIN_L, (uint16_t)(barMin & 0xFFFF));
	ithWriteRegH(ARM_NOR_WRITER_APP_BAR_MIN_H, (uint16_t)(barMin >> 16));
}

void SetArmNorWriterAppProgressMax(uint32_t barMax)
{
	ithWriteRegH(ARM_NOR_WRITER_APP_BAR_MAX_L, (uint16_t)(barMax & 0xFFFF));
	ithWriteRegH(ARM_NOR_WRITER_APP_BAR_MAX_H, (uint16_t)(barMax >> 16));
}

void SetArmNorWriterAppProgressCurr(uint32_t barCurr)
{
	ithWriteRegH(ARM_NOR_WRITER_APP_BAR_CURR_L, (uint16_t)(barCurr & 0xFFFF));
	ithWriteRegH(ARM_NOR_WRITER_APP_BAR_CURR_H, (uint16_t)(barCurr >> 16));
}

uint32_t GetArmNorWriterAppProgressMin()
{
	uint16_t value1 = 0;
	uint16_t value2 = 0;

	value1 = ithReadRegH(ARM_NOR_WRITER_APP_BAR_MIN_L);
	value2 = ithReadRegH(ARM_NOR_WRITER_APP_BAR_MIN_H);

	return (((uint32_t)value2) << 16 | value1);
}

uint32_t GetArmNorWriterAppProgressMax()
{
	uint16_t value1 = 0;
	uint16_t value2 = 0;

	value1 = ithReadRegH(ARM_NOR_WRITER_APP_BAR_MAX_L);
	value2 = ithReadRegH(ARM_NOR_WRITER_APP_BAR_MAX_H);

	return (((uint32_t)value2) << 16 | value1);
}

uint32_t GetArmNorWriterAppProgressCurr()
{
	uint16_t value1 = 0;
	uint16_t value2 = 0;

	value1 = ithReadRegH(ARM_NOR_WRITER_APP_BAR_CURR_L);
	value2 = ithReadRegH(ARM_NOR_WRITER_APP_BAR_CURR_H);

	return (((uint32_t)value2) << 16 | value1);
}
