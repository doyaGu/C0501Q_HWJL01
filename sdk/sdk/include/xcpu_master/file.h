/*
 * Copyright (c) 2013 ITE Technology Corp. All Rights Reserved.
 */
/** @file file.h
 *
 * @version 0.1
 */

#ifndef FILE_MGR_H
#define FILE_MGR_H

#include "itx.h"

#if ITX_BOOT_TYPE == ITX_HOST_BOOT

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
// FILE
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
/**
 * Used to get the information (including the file size) of a specific file 
 * identified by an unique id.
 *
 * @param id        The corresponding id for a specific file.
 * @param ptInfo    The output file information.
 * @return          0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
UserFileSize(
    MMP_UINT32* pFileSize);

//=============================================================================
/**
 * Used to load the content of the specific file into buffer.
 *
 * @param id        The corresponding id for a specific file.
 * @param pBuffer   The output buffer.
 * @param size      Maximum size in bytes to be loaded.
 * @return          The number of bytes actually loaded.
 */
//=============================================================================
MMP_RESULT
UserFileLoad(
    MMP_UINT32 vRamAddress,
    MMP_UINT32 size);

#ifdef __cplusplus
}
#endif

#endif
#endif // End of #ifndef FILE_MGR_H
