#ifndef NF_LB_HWECC_H
#define NF_LB_HWECC_H

#include "configs.h"
#include "ite/mmp_types.h"
#include "NF_Reg.h"


#ifdef NF_LARGEBLOCK_2KB
#ifdef NF_HW_ECC

#ifdef __cplusplusxx
extern "C" {
#endif

MMP_UINT8
LB_HWECC_Initialize(
    unsigned long* pNumOfBlocks,
    unsigned char* pPagePerBlock,
    unsigned long* PageSize);

void
LB_HWECC_Getwear(
    MMP_UINT32 blk,
    MMP_UINT8 *dest);

#ifdef	PITCH_RESERVED_BLOCK_ISSUE_X
void
LB_HWECC_Setreservedblocks(
    unsigned long  blk);
#endif
    
MMP_UINT8
LB_HWECC_Erase(
    MMP_UINT32 pba);

MMP_UINT8
LB_HWECC_Write(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pDataBuffer,
    MMP_UINT8* pSpareBuffer);

MMP_UINT8
LB_HWECC_LBA_Write(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8* pDataBuffer,
    MMP_UINT8* pSpareBuffer);
    
MMP_UINT8
LB_HWECC_WriteDouble(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8 *buffer0,
    MMP_UINT8 *buffer1);
 	
MMP_UINT8
LB_HWECC_Read(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8 *buffer);

MMP_UINT8
LB_HWECC_LBA_Read(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8 *buffer);
    
MMP_UINT8
LB_HWECC_ReadPart(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8 *buffer,
    MMP_UINT8 index);


MMP_UINT8
LB_HWECC_IsBadBlock(
    MMP_UINT32 pba);

MMP_UINT8
LB_HWECC_IsBadBlockForBoot(
    MMP_UINT32 pba);

MMP_UINT8
LB_HWECC_ReadOneByte(
    MMP_UINT32 pba,
    MMP_UINT32 ppo,
    MMP_UINT8 sparepos,
    MMP_UINT8 *ch);

void
LB_HWECC_GetAttribute(
	MMP_BOOL* NfType,
	MMP_BOOL* EccStatus);

unsigned char*
LB_HWECC_GetBlockErrorRecord();

#ifdef __cplusplus
}
#endif

#endif  // End #ifdef NF_HW_ECC
#endif  // End #ifdef LARGEBLOCK

#endif
