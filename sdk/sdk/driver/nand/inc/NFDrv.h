/*
 * Copyright (c) 2004 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * Header file of anti-aliasing font functions.
 *
 */

#ifndef NFDRV_H
#define NFDRV_H

#include "configs.h"
#include "ite/mmp_types.h"
#include "NF_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

DLLAPI MMP_UINT8
mmpNFInitialize(
    unsigned long *pNumOfBlocks,
    unsigned char *pPagePerBlock,
    unsigned long *PageSize);

DLLAPI void
mmpNFGetwear(
    MMP_UINT32 blk,
    MMP_UINT8  *dest);

#ifdef  NF_PITCH_RESERVED_BLOCK_ISSUE
DLLAPI MMP_UINT8
mmpNFSetreservedblocks(
    unsigned int blk);
#endif

DLLAPI MMP_UINT8
mmpNFErase(
    MMP_UINT32 pba);

DLLAPI MMP_UINT8
mmpNFWrite(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8  *pDataBuffer,
    MMP_UINT8  *pSpareBuffer);

DLLAPI MMP_UINT8
mmpNFLbaWrite(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8  *pDataBuffer,
    MMP_UINT8  *pSpareBuffer);

DLLAPI MMP_UINT8
mmpNFWriteDouble(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8  *buffer0,
    MMP_UINT8  *buffer1);

DLLAPI MMP_UINT8
mmpNFRead(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8  *buffer);

DLLAPI MMP_UINT8
mmpNFLbaRead(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8  *buffer);

DLLAPI MMP_UINT8
mmpNFReadPart(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8  *buffer,
    MMP_UINT8  index);

DLLAPI MMP_UINT8
mmpNFIsBadBlock(
    MMP_UINT32 pba);

DLLAPI MMP_UINT8
mmpNFIsBadBlockForBoot(
    MMP_UINT32 pba);

DLLAPI MMP_UINT8
mmpNFReadOneByte(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8  sparepos,
    MMP_UINT8  *ch);

DLLAPI void
mmpNFGetAttribute(
    MMP_BOOL *NfType,
    MMP_BOOL *EccStatus);

DLLAPI void
mmpEnableSpiCsCtrl(
    MMP_UINT32 EnCtrl);

#ifdef __cplusplus
}
#endif

#endif