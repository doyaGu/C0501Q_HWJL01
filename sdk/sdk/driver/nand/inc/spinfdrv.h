/*
 * Copyright (c) 2015 iTE Corp. All Rights Reserved.
 */
/** @file
 * ITE SPI NAND Driver API header file.
 *
 */

#ifndef SPINF_DRV_H
#define SPINF_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#define USE_MMP_DRIVER
//=============================================================================
//                              Include Files
//=============================================================================
#ifdef USE_MMP_DRIVER
    #include <sys/ioctl.h>
    #include <assert.h>
    #include <stdio.h>
    #include <pthread.h>
    #include <unistd.h>

    #include "ite/itp.h"
#else
    #include "stdint.h"
    #include "tooldriver/xcpu_io.h"
#endif

/***************************
 *
 **************************/
#define MAX_SPI_NAND_NAME_SIZE           (32)

#define SPINF_ERROR_READ_ID_ERR          (0x01)
#define SPINF_ERROR_GET_FEATURE_ERR      (0x02)
#define SPINF_ERROR_SET_FEATURE_ERR      (0x03)

#define SPINF_ERROR_SEND_RD_CMD1_ERR     (0x04)
#define SPINF_ERROR_SEND_CMD_0X13_ERR    (0x05)
#define SPINF_ERROR_READ_PAGE_ERR        (0x06)

#define SPINF_ERROR_CMD_WT_EN_ERR        (0x07)
#define SPINF_ERROR_CMD_ERS_ERR          (0x08)

#define SPINF_ERROR_RESET_CMD_FAIL       (0x09)
#define SPINF_ERROR_GET_FEATURE_CMD_FAIL (0x0a)
#define SPINF_ERROR_ID_CMD_FAIL          (0x0b)
#define SPINF_ERROR_ID_ERROR             (0x0c)

//feature C0 register
#define    NF_STATUS_OIP                 (0x01)
#define    NF_STATUS_WEL                 (0x02)
#define    NF_STATUS_E_FAIL              (0x04)
#define    NF_STATUS_P_FAIL              (0x08)
#define    NF_STATUS_ECCS0               (0x10)
#define    NF_STATUS_ECCS1               (0x20)
#define    NF_STATUS_ECCS2               (0x40)
/***************************
 *
 **************************/
typedef struct SPINF_CFG_TAG
{
    char     cfgDevName[MAX_SPI_NAND_NAME_SIZE];
    uint8_t  cfgMID;
    uint8_t  cfgDID0;
    uint8_t  cfgDID1;
    uint8_t  cfgBBM_type;
    uint32_t cfgPageSize;
    uint32_t cfgPageInBlk;
    uint32_t cfgTotalBlk;
    uint32_t cfgSprSize;
} SPINF_CFG;

typedef struct NF_INFO_TAG
{
    uint32_t  Init;
    uint32_t  fromCfg;
    uint32_t  CurBlk;
    uint32_t  BootRomSize;

    uint8_t   MID;
    uint8_t   DID0;
    uint8_t   DID1;
    uint8_t   BBM_type;

    uint32_t  PageInBlk;
    uint32_t  PageSize;
    uint32_t  TotalBlk;
    uint32_t  SprSize;
    uint32_t  FtlSprSize;
    SPINF_CFG *CfgArray;
    uint32_t  CfgCnt;
    uint8_t   name[MAX_SPI_NAND_NAME_SIZE];
} SPI_NF_INFO;

typedef struct NF_SPARE_TAG
{
    uint32_t wear;
    uint8_t  dummy1[8];
    uint32_t ecc;
    uint32_t goodCode;
    uint8_t  dummy2[104];
} NF_SPARE;

/***************************
 * public function
 **************************/
#ifdef  USE_MMP_DRIVER
void showData(uint8_t *buf, uint32_t len);
#endif

uint8_t spiNf_ReadId(unsigned char *id);
uint8_t spiNf_Initial(SPI_NF_INFO *info);
uint8_t spiNf_getFeature(uint8_t addr, uint8_t *buf);
uint8_t spiNf_setFeature(uint8_t addr, uint8_t Reg);
uint8_t spiNf_ByteRead(uint32_t blk, uint32_t ppo, uint8_t *dBuf, uint32_t offset, uint32_t rLen);
uint8_t spiNf_PageRead(uint32_t blk, uint32_t ppo, uint8_t *dBuf);
uint8_t spiNf_PageProgram(uint32_t blk, uint32_t ppo, uint8_t *dBuf);
uint8_t spiNf_BlockErase(uint32_t blk);
void spiNf_SetSpiCsCtrl(uint32_t csCtrl);

#ifdef __cplusplus
}
#endif

#endif