/*
 * Copyright (c) 2007 SMedia technology Corp. All Rights Reserved.
 */
/** @file psi_sectioin_kit.c
 * Used to create, destroy, and verify section.
 *
 * @author Steven Hsiao
 * @version 0.1
 */

#include <stdlib.h>
#include <string.h>
#include "crc.h"
#include "psi_section_kit.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define CRC_TOTAL_BYTES     (4)
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Allocate memory to keep the data of a new section and init the section.
 *
 * @param allocId   allocation identify id.
 * @param pfAlloc   section payload alloc function.
 * @param size      The requested section size of the section. The size is the
 *                  maximum section size defined in spec
 * @return          PSI_SECT* to point to the new section. If the return
 *                  pointer is MMP_NULL, it means the new section creation is
 *                  failed.
 */
//=============================================================================
PSI_SECT*
psi_section_create(
    int                     allocId,
    SECT_PAYLOAD_ALLOC      pfAlloc,
    uint32_t                size)
{
    PSI_SECT    *ptNewSection = 0;

    do{
        ptNewSection = (PSI_SECT*)malloc(sizeof(PSI_SECT));
        if( !ptNewSection )     break;

        memset(ptNewSection, 0x0, sizeof(PSI_SECT));

        if( pfAlloc )
            ptNewSection->pData = (uint8_t*) (*pfAlloc) (allocId, size);

        if( !ptNewSection->pData )
        {
            // Free if failed
            free(ptNewSection);
            ptNewSection = 0;
        }

        ptNewSection->pPayloadEndAddress = ptNewSection->pPayloadStartAddress
                                         = ptNewSection->pData;
    }while(0);

    return ptNewSection;
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
psi_section_destroy(
    int                     allocId,
    SECT_PAYLOAD_FREE       pfFree,
    PSI_SECT*               ptSection)
{
    PSI_SECT* ptNextSection = 0;

    while (ptSection)
    {
        ptNextSection = ptSection->ptNextSection;

        if (ptSection->pData && pfFree)
            (*pfFree) (allocId, ptSection->pData);

        free(ptSection);
        ptSection = ptNextSection;
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
bool
psi_section_verify(
    PSI_SECT    *ptSection)
{
    bool        result = true;
    uint32_t    i, checkByteCount = 0;
    uint32_t    crc = (uint32_t)(-1);

    uint8_t     *pStartAddress = ptSection->pData;

    checkByteCount = (ptSection->pPayloadEndAddress + CRC_TOTAL_BYTES)
                    - ptSection->pData;

    // The section is a generic table, therefore, a CRC_32 field is existed
    // in the end of section
    do{
        // All payload are private datagram. Hence, simply return MMP_TRUE
        if( !ptSection->section_syntax_indicator )      break;

        for (i = 0; i < checkByteCount; i++)
        {
            crc = (crc << 8)
                  ^ crc_valid_table[(crc >> 24) ^ (*(pStartAddress+i))];
        }

        if( crc != 0 )      result = false;
    }while(0);

    return result;
}
