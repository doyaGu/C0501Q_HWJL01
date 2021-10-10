//*****************************************************************************
// Name: sxa_dmx_3gpp_stbl_entry.h
//
// Description:
//     Header File for sxa_dmx_3gpp_stbl_entry.c
//
// Copyright (c) 2008 ITE Technology Corp. All Rights Reserved.
//*****************************************************************************

#ifndef _SXA_DMX_3GPP_STBL_ENTRY_H_
#define _SXA_DMX_3GPP_STBL_ENTRY_H_

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "sxa_dmx_common.h"
//#include "pal/pal.h"

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct GPP_DECODE_TABLE_STREAM_TAG
{
    // From initial
    PAL_FILE      *pFile;
    unsigned long fileOffset;          /**<  file offset */
    unsigned long itemPerEntry;        /**<  number of item in an entry */
    unsigned long entryCount;          /**<  number of entry in the file */
    unsigned long bufEntryCnt;         /**<  number of entry in the buffer */
    unsigned long bufferSize;          /**<  size of raw data buffer */
    // From Refresh
    unsigned long dwStartIdx;          /**<  first entry index of the stream in the buffer */
    unsigned long *tableStream;        /**<  raw data from file */
} GPP_DECODE_TABLE_STREAM;

//=============================================================================
//                              Function Declaration
//=============================================================================

GPP_RESULT sxaDmx3GPP_TableStreamInitial(
    PAL_FILE                *filePtr,
    unsigned long           fileOffset,
    unsigned long           itemPerEntry,
    unsigned long           entryCount,
    unsigned long           streamBufferSize,
    GPP_DECODE_TABLE_STREAM *ts);

GPP_RESULT sxaDmx3GPP_TableStreamDestory(GPP_DECODE_TABLE_STREAM *ts);

GPP_RESULT sxaDmx3GPP_GetValueFromTableStream(
    GPP_DECODE_TABLE_STREAM *ts,
    unsigned long           entryIndex,                      // zero base (start from 0)
    unsigned long           itemIndex,                       // zero base (start from 0), item index in an entry
    unsigned long           *value);

#ifdef __cplusplus
}
#endif

#endif // _SXA_DMX_3GPP_STBL_ENTRY_H_