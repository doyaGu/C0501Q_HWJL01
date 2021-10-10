#ifndef NOR_SPEC_H
#define NOR_SPEC_H

typedef enum
{
    READ,
    FAST_READ,
    READ_ID,
    READ_STATUS,
    WRITE_EN,
    WRITE_DIS,
    WRITE,
	WRITE_MULTI,
	AUTO_WRITE,
	AUTO_WRITE_DATA_ONLY,
    WRITE_STATUS,
	WRITE_STATUS_EN,
    ERASE_SECTOR,
	ERASE_BLOCK,
    ERASE_ALL,
	READ_DEVICE,
    EN4B_MODE,
    EX4B_MODE,
    UNKNOW_CMD_TYPE
}FLASH_CMD_TYPE;

#define READ_CMD            0x03
#define FAST_READ_CMD       0x0B
#define READ_ID_CMD         0x90

#define WRITE_EN_CMD        0x06
#define WRITE_DIS_CMD       0x04

#define ERASE_SECOTR_CMD    0x20
#define ERASE_BLOCK_CMD		0xD8
#define ERASE_BULK_CMD      0xC7

#define BYTE_PROGRAM_CMD    0x02
#define AUTO_PROGRAM_CMD    0xAD
#define PAGE_PROGRAM_CMD    0x02

#define READ_STATUS_CMD     0x05
#define WRITE_STATUS_CMD    0x01
#define WRITE_STATUS_EN_CMD 0x50

#define READ_ELEC_SIGN		0xAB
#define JEDEC_READ_ID		0x9F

#define DEEP_POWER_DOWN_CMD     0xB9
#define REL_DEEP_CMD            0xAB

//4 Byte Mode
#define READ4B_CMD          0x13
#define FAST_READ4B_CMD     0x0C

#define ERASE_SECOTR4B_CMD  0x21
#define ERASE_BLOCK4B_CMD	0xDC

#define PAGE_PROGRAM4B_CMD  0x12

#define EN4B_CMD            0xB7
#define EX4B_CMD            0xE9

#define NOR_DEVICE_READY        0
#define NOR_DEVICE_BUSY         1
#define NOR_AAI_MODE			(1u << 6)
#define NOR_MEM_WRITE_ENABLE    (1u << 1)

#define NOR_WRITE_PROTECT		0x1C


#endif
