
//*****************************************************************************
// Name: sxa_dmx_3gpp_stbl_entry.c
//
// Description:
//     STBL Entry Functions for 3GPP Decoder
//
// Copyright (c) 2008 SMedia Technology Corp. All Rights Reserved.
//*****************************************************************************

//=============================================================================
//                              Compile Option
//=============================================================================

//=============================================================================
//                              Include Files
//=============================================================================
#include "sxa_dmx_3gpp_stbl_entry.h"


//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
extern unsigned long gppFilePos;

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Private Function declare
//=============================================================================
static GPP_RESULT loadEntry(
                      GPP_DECODE_TABLE_STREAM* ts,
                      unsigned long entryIndex);

//=============================================================================
//                              Public Function definition
//=============================================================================
// Middle
// Completed
GPP_RESULT sxaDmx3GPP_TableStreamInitial(
                     PAL_FILE       *filePtr,
                     unsigned long  fileOffset,
                     unsigned long  itemPerEntry,
                     unsigned long  entryCount,
                     unsigned long  streamBufferSize,
                     GPP_DECODE_TABLE_STREAM *ts)
{
    GPP_RESULT ret;

    ts->pFile           = filePtr;
    ts->fileOffset      = fileOffset;
    ts->itemPerEntry    = itemPerEntry;
    ts->entryCount      = entryCount;
    ts->bufferSize      = streamBufferSize * 2;     // [dvyu] 2008.1.23 Modified, double the size for backward seeking
    ts->tableStream     = (unsigned long*)PalHeapAlloc(PAL_HEAP_DEFAULT, ts->bufferSize);
    if (ts->tableStream == NULL)
    {
        //PalPrintf("[SMedia] allocate system memory fail@ TableStreamInitial, size = %d\n\r", streamBufferSize);
        return (GPP_ERROR_STREAM_MALLOC);
    }
    ts->bufEntryCnt = ts->bufferSize / itemPerEntry / 4;

    if (GPP_SUCCESS != (ret = loadEntry(ts, 0)))
    {
        return ret;
    }

    return (GPP_SUCCESS);
}



GPP_RESULT sxaDmx3GPP_TableStreamDestory(GPP_DECODE_TABLE_STREAM* ts)
{
    if (ts->tableStream)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ts->tableStream);
    }

    return (GPP_SUCCESS);
}



GPP_RESULT sxaDmx3GPP_GetValueFromTableStream(
                      GPP_DECODE_TABLE_STREAM* ts,
                      unsigned long entryIndex,              // zero base (start from 0)
                      unsigned long itemIndex,               // zero base (start from 0), item index in an entry
                      unsigned long *value)
{
    GPP_RESULT ret;

    if (entryIndex < ts->dwStartIdx || entryIndex >= (ts->dwStartIdx + ts->bufEntryCnt))
    {
        // reload buffer
        if (GPP_SUCCESS != (ret = loadEntry(ts, entryIndex)))
        {
            return ret;
        }
    }
    *value = ts->tableStream[(entryIndex - ts->dwStartIdx) * ts->itemPerEntry + (itemIndex)];

    return (GPP_SUCCESS);
}



static GPP_RESULT loadEntry(
                      GPP_DECODE_TABLE_STREAM* ts,
                      unsigned long entryIndex)
{
    unsigned long dwFileERRCode;
    unsigned int  i;
    unsigned long readCount = ts->bufEntryCnt * ts->itemPerEntry;
    unsigned int  nPos;
    // { [dvyu] 2008.1.23 Add, shift the start index position for backward seeking
    if (entryIndex > (ts->bufEntryCnt / 2))
    {
        entryIndex -= (ts->bufEntryCnt / 2);
    }
    else
    {
        entryIndex = 0;
    }
    // } [dvyu] 2008.1.11
/*
    nPos = PalTFileTell(0,NULL);
    if (nPos == ts->fileOffset) {
        dwFileERRCode = PalTFileSeek(ts->pFile, ts->fileOffset + (entryIndex * 4 * ts->itemPerEntry), PAL_SEEK_CUR, MMP_NULL);
    } else {
        dwFileERRCode = PalTFileSeek(ts->pFile, ts->fileOffset + (entryIndex * 4 * ts->itemPerEntry), PAL_SEEK_SET, MMP_NULL);
    }*/
    dwFileERRCode = PalTFileSeek(ts->pFile, ts->fileOffset + (entryIndex * 4 * ts->itemPerEntry), PAL_SEEK_SET, MMP_NULL);    
    if (dwFileERRCode < 0)
    {
        return (GPP_ERROR_DECODE_FILE_SEEK);
    }
    dwFileERRCode = PalTFileRead(ts->tableStream, 4, readCount, ts->pFile, MMP_NULL);
    if (dwFileERRCode < 0)
    {
        return (GPP_ERROR_DECODE_FILE_READ);
    }
    gppFilePos = ts->fileOffset + (entryIndex * 4 * ts->itemPerEntry) + readCount * 4;

    for (i = 0; i < readCount; i++)
    {
        BS_WAP(ts->tableStream[i]);
    }

    ts->dwStartIdx = entryIndex;

    return (GPP_SUCCESS);
}

