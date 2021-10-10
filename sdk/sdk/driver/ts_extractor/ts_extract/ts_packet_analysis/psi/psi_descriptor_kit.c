/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file psi_descriptor_kit.c
 * Used to create and destroy descriptor.
 * @author Steven Hsiao
 * @version 0.1
 */

#include <stdlib.h>
#include <string.h>
#include "psi_descriptor_kit.h"

//=============================================================================
//                Constant Definition
//=============================================================================

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
 * Init a new descriptor structure to store the information of the descriptor.
 *
 * @param tag     Descriptor tag number is used to be the identifier
 *                of the descriptor.
 * @param length  The content data length of the descriptor.
 * @param pData   The data payload of the descriptor.
 * @return        PSI_DESCR* to store the information of the
 *                new descriptor.
 */
//=============================================================================
PSI_DESCR*
psi_descr_create(
    uint32_t    tag,
    uint32_t    length,
    uint8_t     *pData)
{
    PSI_DESCR   *ptDescriptor = 0;

    ptDescriptor = (PSI_DESCR*)malloc(sizeof(PSI_DESCR));

    if( ptDescriptor )
    {
        memset(ptDescriptor, 0x0, sizeof(PSI_DESCR));
        ptDescriptor->descriptor_tag    = tag;
        ptDescriptor->descriptor_length = length;

        if( length > 0 )
        {
            ptDescriptor->pPayload = (uint8_t*)malloc(length);

            if( ptDescriptor->pPayload )
                memcpy(ptDescriptor->pPayload, pData, length);
            else
            {
                free(ptDescriptor);
                ptDescriptor = 0;
            }
        }
    }

    return ptDescriptor;
}


//=============================================================================
/**
 * Delete descriptors of descriptor list by assigning the desired start
 * descriptor entry of the list.
 *
 * @param ptDescriptor  The first delete point of the descriptor list.
 * @return none
 */
//=============================================================================
void
psi_descr_destroy(
    PSI_DESCR   *ptDescriptor)
{
    PSI_DESCR      *ptNextDescriptor;

    while( ptDescriptor )
    {
        ptNextDescriptor = ptDescriptor->ptNextDescriptor;

        if( ptDescriptor->pPayload )
            free(ptDescriptor->pPayload);

        if( ptDescriptor->ptDecodedContent )
            free(ptDescriptor->ptDecodedContent);

        free(ptDescriptor);
        ptDescriptor = ptNextDescriptor;
    }
    return;
}


