/** @file
 * ITE MSC Driver API header file.
 *
 * @author Irene Lin
 * @version 1.0
 * @date 2011-2012
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#ifndef ITE_MSC_H
#define ITE_MSC_H

#ifdef __cplusplus
extern "C" {
#endif

#define MSC_API extern

//=============================================================================
//                              Include Files
//=============================================================================
#include "ite/ith.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MMP_MSC_MAX_LUN_NUM     8

//=============================================================================
//                              Struct Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================
MSC_API int iteMscDriverRegister(void);

//=============================================================================
/**
 * File system must call this API first when initializing a volume.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see iteMscTerminate()
 */
//=============================================================================
MSC_API int iteMscInitialize(void* us, uint8_t lun);

//=============================================================================
/**
 * This routine is used to release any resources associated with a drive when it is removed.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see iteMscInitialize()
 */
//=============================================================================
MSC_API int iteMscTerminate(void* us, uint8_t lun);

//=============================================================================
/**
 * This routine is used to read a series of sectors from the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see iteMscWriteMultipleSector()
 */
//=============================================================================
MSC_API int iteMscReadMultipleSector(void* us, uint8_t lun, uint32_t sector, int count, void* data);

//=============================================================================
/**
 * This routine is used to write a series of sectors to the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see iteMscWriteMultipleSector()
 */
//=============================================================================
MSC_API int iteMscWriteMultipleSector(void* us, uint8_t lun, uint32_t sector, int count, void* data);

//=============================================================================
/**
 * This routine is used to get the number of sectors and block size.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
MSC_API int iteMscGetCapacity(void* us, uint8_t lun, uint32_t* sectorNum, uint32_t* blockLength);
    
//=============================================================================
/**
 * This routine is used to get the device status.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
MSC_API int iteMscGetStatus(void* us, uint8_t lun);   
MSC_API int iteMscGetStatus2(void* us, uint8_t lun);   

//=============================================================================
/**
 * This routine is used to check if the card(lun) is in write-protected status.
 *
 * @return MMP_TRUE if this lun is in write-protected status, otherwise return MMP_FALSE.
 */
//=============================================================================
MSC_API bool iteMscIsLock(void* us, uint8_t lun);

/** Irene: 2011_0106
 * Is in read/write procedure? Just for application workaround. 
 */
MSC_API bool iteMscInDataAccess(void* us);


// +wlHsu
MSC_API void
iteMscResetErrStatus(
    void);
// -wlHsu

#ifdef __cplusplus
}
#endif

#endif /* ITE_MSC_H */
