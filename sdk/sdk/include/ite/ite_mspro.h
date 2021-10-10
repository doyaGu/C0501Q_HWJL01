/*
 * Copyright (c) 2004 ITE technology Corp. All Rights Reserved.
 */
/** @file
 * ITE Memory Stick Card Driver API header file.
 *
 * @author Irene Lin
 */
#ifndef ITE_MSPRO_H
#define ITE_MSPRO_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DLL export API declaration for Win32 and WinCE.
 */
#if 0//defined(WIN32) || defined(_WIN32_WCE)
    #if defined(MSPRO_EXPORTS)
        #define MSPRO_API __declspec(dllexport)
    #else
        #define MSPRO_API __declspec(dllimport)
    #endif
#else
    #define MSPRO_API     extern
#endif /* defined(WIN32) */

//=============================================================================
//                              Include Files
//=============================================================================
#include "ite/ith.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum MS_CARD_STATE_TAG
{
    MS_INIT_OK,
    MS_INSERTED
} MS_CARD_STATE;

#define MS_CARD_DISABLE 0x99

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct MS_PRO_CARD_ATTRIB_TAG
{
    uint16_t numCylinders;
    uint16_t numHeads;
    uint16_t numSectorPerTrack;
    uint32_t numSectors;
    uint8_t  mediaDescriptor;
} MS_PRO_CARD_ATTRIB;

//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup group16 ITE MS Pro Driver API
 *  The supported API for MS Pro.
 *  @{
 */
//=============================================================================
/**
 * File system must call this API first when initializing a volume.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see iteMsproTerminate()
 */
//=============================================================================
MSPRO_API int iteMsproInitialize(void);

//=============================================================================
/**
 * This routine is used to release any resources associated with a drive when it is removed.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see iteMsproInitialize()
 */
//=============================================================================
MSPRO_API int iteMsproTerminate(void);

//=============================================================================
/**
 * This routine is used to read a series of sectors from the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see iteMsproWriteMultipleSector()
 */
//=============================================================================
MSPRO_API int iteMsproReadMultipleSector(uint32_t sector, int count, void *data);

//=============================================================================
/**
 * This routine is used to write a series of sectors to the targe device.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 *
 * @see iteMsproWriteMultipleSector()
 */
//=============================================================================
MSPRO_API int iteMsproWriteMultipleSector(uint32_t sector, int count, void *data);

//=============================================================================
/**
 * This routine is called by the file system to find out the physical properties of the
 * device e.g. number of sectors.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
MSPRO_API int iteMsproGetAttrib(MS_PRO_CARD_ATTRIB *attrib);

//=============================================================================
/**
 * This routine is used to get the number of sectors and block size.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
MSPRO_API int iteMsproGetCapacity(uint32_t *sectorNum, uint32_t *blockLength);

//=============================================================================
/**
 * This routine is used to get the device status.
 *
 * @return zero if succeed, otherwise return non-zero error codes.
 */
//=============================================================================
MSPRO_API bool iteMsproGetCardState(MS_CARD_STATE state);

//=============================================================================
/**
 * This routine is used to check if ms (pro) card is in write-protected status.
 *
 * @return MMP_TRUE if ms (pro) card is in write-protected status, otherwise return MMP_FALSE.
 */
//=============================================================================
MSPRO_API bool iteMsproIsLock(void);

//@}

#ifdef __cplusplus
}
#endif

#endif /* ITE_MSPRO_H */