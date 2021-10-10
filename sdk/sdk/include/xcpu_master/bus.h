/*
 * Copyright (c) 2013 ITE Technology Corp. All Rights Reserved.
 */
/** @file bus.h
 *
 * @version 0.1
 */

#ifndef BUS_H
#define BUS_H

#include "pal.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

///////////////////////////////////////////////////////////////////////////////
// Bus driver for SPI or I2C
///////////////////////////////////////////////////////////////////////////////

#if (ITX_BUS_TYPE == ITX_BUS_I2C)
//=============================================================================
/**
 * Used to open bus.
 *
 * @param pParam    Pointer to parameters. Upper layer can use this argument
 *                  to pass any parameters into this layer if needed.
 * @return          MMP_RESULT_SUCCESS If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
busI2COpen(
    void);
    
//=============================================================================
/**
 * Used to close bus.
 *
 * @return          MMP_RESULT_SUCCESS If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
busI2CClose(
    void);
    
//=============================================================================
/**
 * Used to lock mutex.
 *
 * @return          MMP_RESULT_SUCCESS If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
busI2CEnter(
    void);
        
//=============================================================================
/**
 * Used to unlock mutex.
 *
 * @return          MMP_RESULT_SUCCESS If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
busI2CLeave(
    void);

//=============================================================================
/**
 * Used to read a block of bytes from the I2C port.
 *
 * @param pData     Address to store the data.
 * @param size      Number of bytes to read.
 * @return          MMP_RESULT_SUCCESS If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
busI2CRead(
    MMP_UINT8   slaveAddr,
    MMP_UINT16  startAddr,
    MMP_UINT8*  pdata,
    MMP_UINT32  size);

//=============================================================================
/**
 * Used to write a block of bytes from the I2C port.
 *

 * @param pData     Address of data to write.
 * @param size      Number of bytes to write.
 * @return          MMP_RESULT_SUCCESS If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
busI2CWrite(   
    MMP_UINT8   slaveAddr,
    MMP_UINT16  startAddr,
    MMP_UINT8*  pdata,
    MMP_UINT32  size);

#endif

#if (ITX_BUS_TYPE == ITX_BUS_SPI)
//=============================================================================
/**
 * Used to open bus.
 *
 * @return          MMP_RESULT_SUCCESS If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
busSPIOpen(
    void);
    
//=============================================================================
/**
 * Used to close bus.
 *

 * @return          MMP_RESULT_SUCCESS If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
busSPIClose(
    void);

//=============================================================================
/**
 * Used to lock mutex.
 *
 * @return          MMP_RESULT_SUCCESS If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
busSPIEnter(
    void);

//=============================================================================
/**
 * Used to unlock mutex.
 *
 * @return          MMP_RESULT_SUCCESS If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
busSPILeave(
    void);

//=============================================================================
/**
 * Used to read a block of bytes from the SPI port.
 *
 * @param pCtrl     Address to store control bytes.
 * @param ctrlLen   Number of control bytes to send.
 * @param pData		Address to store the data.
 * @param dataLen   Number of data bytes to read.
 * @return          MMP_RESULT_SUCCESS If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
busSPIRead(
    MMP_UINT8*  pCtrl,
    MMP_UINT32  ctrlLen,
    MMP_UINT8*  pData,
    MMP_UINT32  dataLen);

//=============================================================================
/**
 * Used to write a block of bytes from the I2C port.
 *
 * @param pCtrl     Address to store control bytes.
 * @param ctrlLen   Number of control bytes to send.
 * @param pData     Address of data bytes to write
 * @param dataLen   Number of data bytes to write
 * @return          MMP_RESULT_SUCCESS If successful. Otherwise, return a nonzero value.
 */
//============================================================================= 
MMP_RESULT 
busSPIWrite(
    MMP_UINT8*  pCtrl,
    MMP_UINT32  ctrlLen,
    MMP_UINT8*  pData,
    MMP_UINT32  dataLen);

#endif

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef FILE_MGR_H