#include "ite/itc.h"
#include "itc_cfg.h"

uint16_t itcCrc16(const uint8_t* data, uint16_t size)
{
	int i, j;
	uint16_t crc = 0x0000;
	
    for (i = 0; i < size; i++)
	{
		crc ^= (uint16_t)(data[i] << 8);
		for (j = 0; j < 8; j++)
		{
			if ((crc & 0x8000) > 0)
                crc = (uint16_t)((crc << 1) ^ 0x8408);
            else
                crc <<= 1;
		}
	}
	return crc;
}
