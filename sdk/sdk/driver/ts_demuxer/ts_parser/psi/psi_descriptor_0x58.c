/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file psi_descriptor_0x58.c
 * Application interface for the DVB "local time offset" descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.19.
 *
 * @author I-Chun Lai
 * @version 0.1
 */

#include "bitstream_kit.h"
#include "psi_descriptor_0x58.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define BYTES_OF_LOCAL_TIME_OFFSET  (13)

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================
//=============================================================================
/**
 * "local time offset" descriptor decoder.
 *
 * @param ptDescriptor  pointer to the descriptor structure
 * @return a pointer to a new "local time offset" descriptor structure
 *         which contains the decoded data.
 */
//=============================================================================
PSI_LOCAL_TIME_OFFSET_DESCRIPTOR*
psiDescriptor_DecodeLocalTimeOffsetDescriptor(
    PSI_DESCRIPTOR * ptDescriptor)
{
    PSI_LOCAL_TIME_OFFSET_DESCRIPTOR* ptDecoded         = MMP_NULL;
    MMP_UINT8*                        pEnd              = MMP_NULL;
    BITSTREAM                         tBitStream        = { 0 };

    // Check the tag
    if (LOCAL_TIME_OFFSET_DESCRIPTOR_TAG != ptDescriptor->descriptor_tag)
    {
        // bad tag
        return MMP_NULL;
    }

    // Decode again
    if (ptDescriptor->ptDecodedContent)
        PalHeapFree(PAL_HEAP_DEFAULT, ptDescriptor->ptDecodedContent);

    // Allocate memory
    ptDecoded = (PSI_LOCAL_TIME_OFFSET_DESCRIPTOR*)PalHeapAlloc(
        PAL_HEAP_DEFAULT,
        sizeof(PSI_LOCAL_TIME_OFFSET_DESCRIPTOR));
    if (!ptDecoded)
    {
        // out of memory
        return MMP_NULL;
    }
    PalMemset(ptDecoded, 0, sizeof(PSI_LOCAL_TIME_OFFSET_DESCRIPTOR));
    ptDescriptor->ptDecodedContent = (void*)ptDecoded;

    BitStreamKit_Init(&tBitStream, ptDescriptor->pPayload);
    pEnd = ptDescriptor->pPayload + ptDescriptor->descriptor_length;

    while (tBitStream.pStartAddress + BYTES_OF_LOCAL_TIME_OFFSET <= pEnd)
    {
        if (ptDecoded->totalLocalTimeOffsetCount < MAX_LOCAL_TIME_OFFSET_COUNT)
        {
            PSI_LOCAL_TIME_OFFSET* ptLocalTimeOffset =
                &ptDecoded->tLocalTimeOffset[ptDecoded->totalLocalTimeOffsetCount++];

            ptLocalTimeOffset->country_code[0]      = (MMP_UINT8)BitStreamKit_GetBits(&tBitStream, 8);
            ptLocalTimeOffset->country_code[1]      = (MMP_UINT8)BitStreamKit_GetBits(&tBitStream, 8);
            ptLocalTimeOffset->country_code[2]      = (MMP_UINT8)BitStreamKit_GetBits(&tBitStream, 8);
            ptLocalTimeOffset->country_code[3]      = 0;
            //ptLocalTimeOffset->country_region_id    = BitStreamKit_GetBits(&tBitStream, 6);
            BitStreamKit_SkipBits(&tBitStream, 7);
            ptLocalTimeOffset->local_time_offset_polarity   = BitStreamKit_GetBits(&tBitStream, 1);
            ptLocalTimeOffset->local_time_offset            = BitStreamKit_GetBits(&tBitStream, 16);
            //ptLocalTimeOffset->time_of_change.high16        = BitStreamKit_GetBits(&tBitStream, 16);
            //ptLocalTimeOffset->time_of_change.low24         = BitStreamKit_GetBits(&tBitStream, 24);
            //ptLocalTimeOffset->next_time_offset             = BitStreamKit_GetBits(&tBitStream, 16);
        }
        else
        {
            // give an error message
            break;
        }
    }

    return ptDecoded;
}
