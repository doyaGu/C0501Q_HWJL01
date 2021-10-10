/*
 * Copyright (c) 2007 SMedia technology Corp. All Rights Reserved.
 */
/** @file psi_sectioin_kit.c
 * Used to create, destroy, and verify section.
 *
 * @author Steven Hsiao
 * @version 0.1
 */

#include "psi_section_kit.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define CRC_INIT_VALUE (0xFFFFFFFF)
#define CRC_TOTAL_BYTES (4)

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

extern  MMP_UINT32 crcValidationArray[256];

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Allocate memory to keep the data of a new section and init the section.
 *
 * @param allocId   allocation identify id.
 * @param pfAlloc   section payload alloc function.
 * @param size      The requested section size of the section. The size is the
 *                  maximum section size defined in spec
 * @return          PSI_SECTION* to point to the new section. If the return
 *                  pointer is MMP_NULL, it means the new section creation is
 *                  failed.
 */
//=============================================================================
PSI_SECTION*
SectionKit_CreateSection(
    MMP_INT                     allocId,
    SECTION_PAYLOAD_ALLOC       pfAlloc,
    MMP_UINT32                  size)
{
    PSI_SECTION* ptNewSection;

    ptNewSection = (PSI_SECTION*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                              sizeof(PSI_SECTION));

    if (ptNewSection)
    {
        PalMemset(ptNewSection, 0x0, sizeof(PSI_SECTION));
        if (pfAlloc)
        {
            ptNewSection->pData = (MMP_UINT8*) (*pfAlloc) (allocId, size);
        }
        if (ptNewSection->pData)
        {
            ptNewSection->pPayloadEndAddress = ptNewSection->pPayloadStartAddress
                                             = ptNewSection->pData;
            return ptNewSection;
        }
        else // Free if failed
            PalHeapFree(PAL_HEAP_DEFAULT, ptNewSection);
    }

    return MMP_NULL;
}


//=============================================================================
/**
 * Delete all the following sections including itself.
 *
 * @param allocId       allocation identify id.
 * @param pfFree        section payload free function.
 * @param ptSection     The pointer is pointed to the first section that we
 *                      want to delete.
 * @return none
 */
//=============================================================================
void
SectionKit_DestroySection(
    MMP_INT                     allocId,
    SECTION_PAYLOAD_FREE        pfFree,
    PSI_SECTION*                ptSection)

{
    PSI_SECTION* ptNextSection;

    while (ptSection)
    {
        ptNextSection = ptSection->ptNextSection;

        if (ptSection->pData && pfFree)
            (*pfFree) (allocId, ptSection->pData);

        PalHeapFree(PAL_HEAP_DEFAULT, ptSection);
        ptSection =  ptNextSection;
    }
}


//=============================================================================
/**
 * Check whether the section is valid or invalid.
 *
 * @param ptSection     The pointer is pointed to the section that we want to
 *                      do CRC check.
 * @return              MMP_TRUE means the CRC check is ok, CRC check is failed
 *                      otherwise.
 */
//=============================================================================
MMP_BOOL
SectionKit_VerifySection(
    PSI_SECTION* ptSection)
{
    MMP_UINT32 i;
    MMP_UINT32 crc = CRC_INIT_VALUE;
    MMP_UINT32 checkByteCount;

    MMP_UINT8* pStartAddress = ptSection->pData;

    checkByteCount = (ptSection->pPayloadEndAddress + CRC_TOTAL_BYTES)
                    - ptSection->pData;

    // The section is a generic table, therefore, a CRC_32 field is existed
    // in the end of section
    if (ptSection->section_syntax_indicator)
    {
        for (i = 0; i < checkByteCount; i++)
        {
            crc = (crc << 8)
                  ^ crcValidationArray[(crc >> 24) ^ (*(pStartAddress+i))];
        }
        if (crc == 0)
            return MMP_TRUE;
        else
            return MMP_FALSE;
    }
    else // All payload are private datagram. Hence, simply return MMP_TRUE
        return MMP_TRUE;
}
