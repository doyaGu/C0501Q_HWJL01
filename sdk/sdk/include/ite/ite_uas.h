/** @file
 * ITE UAS Driver API header file.
 *
 * @author Irene Lin
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#ifndef ITE_UAS_H
#define ITE_UAS_H

#include "ite/ith.h"



#ifdef __cplusplus
extern "C" {
#endif


/**
 * Register UAS driver to USB.
 */
int iteUasDriverRegister(void);

/**
 * File system must call this API first when initializing a volume.
 *
 * @param uas the context.
 * @param lun the lun index.
 */
int iteUasInitialize(void* uas, uint8_t lun);

/**
 * Release any resources associated with a drive when it is removed.
 *
 * @param uas the context.
 * @param lun the lun index.
 */
int iteUasTerminate(void* uas, uint8_t lun);

/**
 * Read a series of sectors from the targe device.
 *
 * @param uas the context.
 * @param lun the lun index.
 * @param sector the sector to be read from.
 * @param count the total sector number to be read.
 * @param data the buffer address
 */
int iteUasReadMultipleSector(void* uas, uint8_t lun, uint32_t sector, int count, void* data);

/**
 * Write a series of sectors to the targe device.
 *
 * @param uas the context.
 * @param lun the lun index.
 * @param sector the starting sector.
 * @param count the total sector number.
 * @param data the buffer address
 */
int iteUasWriteMultipleSector(void* uas, uint8_t lun, uint32_t sector, int count, void* data);

/**
 * Get the number of sectors and block size.
 *
 * @param uas the context.
 * @param lun the lun index.
 * @param sectorNum the total sector number.
 * @param blockLength the block size.
 */
int iteUasGetCapacity(void* uas, uint8_t lun, uint32_t* sectorNum, uint32_t* blockLength);
    
/**
 * Used to get the device status.
 *
 * @param uas the context.
 * @param lun the lun index.
 */
int iteUasGetStatus(void* uas, uint8_t lun);   

/**
 * This routine is used to check if the card(lun) is in write-protected status.
 *
 * @param uas the context.
 * @param lun the lun index.
 *
 * @return true if this lun is in write-protected status, otherwise return false.
 */
bool iteUasIsLock(void* uas, uint8_t lun);

int iteUasGetLunNum(void* uas);


#ifdef __cplusplus
}
#endif

#endif
