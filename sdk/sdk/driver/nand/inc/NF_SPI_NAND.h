#ifndef NF_SPI_NAND__H
#define NF_SPI_NAND__H

#include "configs.h"
#include "ite/mmp_types.h"
#include "spinfdrv.h"


#ifdef NF_HW_ECC

#ifdef __cplusplusxx
extern "C" {
#endif

/********************************************************************
 * 
 ********************************************************************/
 /*
#define	SPINF_ERROR_READ_ID_ERR			1
#define	SPINF_ERROR_GET_FEATURE_ERR		2
#define	SPINF_ERROR_SET_FEATURE_ERR		3

#define	SPINF_ERROR_SEND_RD_CMD1_ERR	4
#define	SPINF_ERROR_SEND_CMD_0X13_ERR	5
#define	SPINF_ERROR_READ_PAGE_ERR		6

#define	SPINF_ERROR_CMD_WT_EN_ERR		7
#define	SPINF_ERROR_CMD_ERS_ERR			8
*/

/********************************************************************
 * public function
 ********************************************************************/
void SpiNf_GetAttribute(uint8_t *NfType, uint8_t *EccStatus);
void SpiNf_Getwear(uint32_t blk, uint8_t *dest);

uint8_t SpiNf_Initialize(uint32_t* pNumOfBlocks, uint8_t* pPagePerBlock, uint32_t* pPageSize);
uint8_t SpiNf_Read(uint32_t pba, uint32_t ppo, uint8_t* pBuf);
uint8_t SpiNf_ReadOneByte(uint32_t pba, uint32_t ppo, uint8_t offset, uint8_t* pByte);
uint8_t SpiNf_ReadPart(uint32_t pba, uint32_t ppo, uint8_t* pBuf, uint8_t index);
uint8_t SpiNf_Write(uint32_t pba, uint32_t ppo, uint8_t* pDtBuf, uint8_t* pSprBuf);
uint8_t SpiNf_WriteDouble(uint32_t pba, uint32_t ppo, uint8_t* pBuf0, uint8_t* pBuf1);
uint8_t SpiNf_Erase(uint32_t pba);

uint8_t SpiNf_IsBadBlock(uint32_t pba);
uint8_t SpiNf_IsBadBlockForBoot(uint32_t pba);

uint8_t SpiNf_LBA_Read(uint32_t pba, uint32_t ppo, uint8_t* pBuf);
uint8_t SpiNf_LBA_Write(uint32_t pba, uint32_t ppo, uint8_t* pDtBuf, uint8_t* pSprBuf);

void SpiNf_SetSpiCsCtrl(uint32_t csCtrl);
#ifdef __cplusplusxx
}
#endif

#endif  // End #ifdef NF_HW_ECC

#endif
