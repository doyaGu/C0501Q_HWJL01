/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * ITE XD Driver API header file.
 *
 */

#ifndef ITE_XD_H
#define ITE_XD_H

//=============================================================================
//                              Include Files
//=============================================================================
#include "stdint.h"
#include "xd/xd_mlayer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DLL export API declaration for Win32.
 */
/*
   #if defined(WIN32)
   #if defined(XD_EXPORTS)
   #define XD_API __declspec(dllexport)
   #else
   #define XD_API __declspec(dllimport)
   #endif
   #else
   #define XD_API extern
   #endif
 */
#define XD_API

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Enumeration Type Definition
//=============================================================================
typedef struct XD_INFO_TAG
{
    uint32_t Init;
    uint32_t CurrSector;
    uint32_t NumOfBlk;
    uint32_t TotalSecNum;
} ITE_XD_INFO;

//=============================================================================
//                              Constant Definition
//=============================================================================
#define XD_RESULT                uint32_t

/*Error Code*/
#define XD_ERROR_DEVICE_UNKNOW   1
#define XD_ERROR_DEVICE_TIMEOUT  2
#define XD_ERROR_STATUS_BUSY     3
#define XD_ERROR_STATUS_PROTECT  4
#define XD_ERROR_ADDR_UN_ALIGNED 5

//=============================================================================
//                              Function Declaration
//=============================================================================

/** @defgroup group17 ITE XD Driver API
 *  The XD module API.
 *  @{
 */

/**
 * Initialize XD
 *
 * @return XD_RESULT_SUCCESS if succeed, error codes of XD_RESULT_ERROR otherwise.
 * @see iteXdTerminate()
 */
XD_API XD_RESULT iteXdInitial();

/**
 * Initialize XD
 *
 * @return XD_RESULT_SUCCESS if succeed, error codes of XD_RESULT_ERROR otherwise.
 * @see iteXdTerminate()
 */
XD_API XD_RESULT iteXdTerminate(void);

/**
 * read XD
 *
 * @return 1 if succeed, error codes of XD_RESULT_ERROR otherwise.
 */
XD_API XD_RESULT iteXdRead(uint32_t sector, uint32_t count, void *data);

/**
 * write XD
 *
 * @return XD_RESULT_SUCCESS if succeed, error codes of XD_RESULT_ERROR otherwise.
 */
XD_API XD_RESULT iteXdWrite(uint32_t sector, uint32_t count, void *data);

/**
 * To get total sector and page size of XD
 *
 * @return 1 if succeed, error codes of XD_RESULT_ERROR otherwise.
 */
XD_API XD_RESULT iteXdGetCapacity(uint32_t *sectorNum, uint32_t *blockLength);

/**
 * To get XD card status
 *
 * @return XD_RESULT_SUCCESS if succeed, error codes of XD_RESULT_ERROR otherwise.
 */
XD_API XD_RESULT iteXdGetCardState(XD_CARD_STATE index);

//@}
#ifdef __cplusplus
}
#endif

#endif