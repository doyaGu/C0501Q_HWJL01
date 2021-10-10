/*
 * Copyright (c) 2012 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * ITE NAND Driver API header file.
 *
 * @author Joseph Chang
 */
#ifndef ITE_NAND_H
#define ITE_NAND_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DLL export API declaration for Win32 and WinCE.
 */
#if 0//defined(WIN32) || defined(_WIN32_WCE)
    #if defined(SD_EXPORTS)
        #define NAND_API __declspec(dllexport)
    #else
        #define NAND_API __declspec(dllimport)
    #endif
#else
//#define NAND_API extern
    #define NAND_API
#endif /* defined(WIN32) */

//=============================================================================
//                              Include Files
//=============================================================================
#include "ite/ith.h"

//#include "../driver/nand/inc/configs.h"
#include "../driver/nand/inc/NF_Reg.h"
//#include "../driver/nand/inc/NFDrv.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct NF_INFO_TAG
{
    // --- nandflash object attribute
    uint32_t Init;
    uint32_t CurBlk;
    uint32_t BootRomSize;
    uint32_t enSpiCsCtrl;
    uint32_t addrHasDummyByte;
    uint32_t blEndBlk;
    uint32_t bootImgStartBlk;
    uint32_t bootImgEndBlk;

    // --- Physic nandflash attribute
    uint32_t NumOfBlk;
    uint32_t PageInBlk;
    uint32_t PageSize;
    uint32_t SprSize;
} ITE_NF_INFO;

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * File system must call this API first when initializing a volume.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
NAND_API int iteNfInitialize(ITE_NF_INFO *info);

//=============================================================================
/**
 * This routine is used to release any resources associated with a drive when it is removed.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
NAND_API int iteNfTerminate(void);

//=============================================================================
/**
 * This routine is used to read a series of sectors from the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
NAND_API int iteNfRead(uint32_t pba, uint32_t ppo, uint8_t *buf);

//=============================================================================
/**
 * This routine is used to read a series of sectors from the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
NAND_API int iteNfReadOneByte(uint32_t pba, uint32_t ppo, uint8_t pos, uint8_t *ch);

//=============================================================================
/**
 * This routine is used to read a series of sectors from the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
NAND_API int iteNfReadPart(uint32_t pba, uint32_t ppo, uint8_t *buf, uint8_t index);

//=============================================================================
/**
 * This routine is used to write a series of sectors to the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
NAND_API int iteNfWrite(uint32_t pba, uint32_t ppo, uint8_t *buf, uint8_t *spr);

//=============================================================================
/**
 * This routine is used to write a series of sectors to the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
NAND_API int iteNfWriteDouble(uint32_t pba, uint32_t ppo, uint8_t *buf0, uint8_t *buf1);

//=============================================================================
/**
 * This routine is used to write a series of sectors to the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
NAND_API int iteNfErase(uint32_t pba);

//=============================================================================
/**
 * This routine is used to write a series of sectors to the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
NAND_API int iteNfIsBadBlock(uint32_t pba);

//=============================================================================
/**
 * This routine is used to write a series of sectors to the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
NAND_API int iteNfIsBadBlockForBoot(uint32_t pba);

//=============================================================================
/**
 * This routine is used to write a series of sectors to the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
NAND_API void iteNfGetAttribute(uint8_t *NfType, uint8_t *EccStatus);

//=============================================================================
/**
 * This routine is used to write a series of sectors to the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
NAND_API void iteNfGetwear(uint32_t blk, uint8_t *dest);

/*
   #define iteNfInitialize(a)              iteNfInitializeT(a)
   #define iteNfTerminate()                iteNfTerminateT()
   #define iteNfRead(a,b,c)				iteNfReadT(a,b,c)
   #define iteNfReadOneByte(a,b,c,d)       iteNfReadOneByteT(a,b,c,d)
   #define iteNfReadPart(a,b,c,d)			iteNfReadPartT(a,b,c,d)
   #define iteNfWrite(a,b,c,d)				iteNfWriteT(a,b,c,d)
   #define iteNfWriteDouble(a,b,c,d)		iteNfWriteDoubleT(a, b, c, d)

   #define iteNfErase(a)				    iteNfEraseT(a)
   #define iteNfIsBadBlock(a)			    iteNfIsBadBlockT(a)
   #define iteNfIsBadBlockForBoot(a)       iteNfIsBadBlockForBootT(a)
   #define iteNfGetAttribute(a,b)          iteNfGetAttributeT(a,b)
   #define iteNfGetwear(a)                 iteNfGetwearT(SD_0, a)
 */

#ifdef __cplusplus
}
#endif

#endif /* ITE_NAND_H */