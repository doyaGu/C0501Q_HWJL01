/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file psi_descriptor_kit.c
 * Used to create and destroy descriptor.
 * @author Steven Hsiao
 * @version 0.1
 */

#include "psi_descriptor_kit.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

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
 * Init a new descriptor structure to store the information of the descriptor.
 *
 * @param tag     Descriptor tag number is used to be the identifier
 *                of the descriptor.
 * @param length  The content data length of the descriptor.
 * @param pData   The data payload of the descriptor.
 * @return        PSI_DESCRIPTOR* to store the information of the
 *                new descriptor.
 */
//=============================================================================
PSI_DESCRIPTOR*
DescriptorKit_CreateDescriptor(
    MMP_UINT tag,
    MMP_UINT length,
    MMP_UINT8* pData)
{
    PSI_DESCRIPTOR* ptDescriptor = MMP_NULL;
    ptDescriptor = (PSI_DESCRIPTOR*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                 sizeof(PSI_DESCRIPTOR));

    if (ptDescriptor)
    {
        PalMemset(ptDescriptor, 0x0, sizeof(PSI_DESCRIPTOR));
        ptDescriptor->descriptor_tag    = tag;
        ptDescriptor->descriptor_length = length;

        if (length > 0)
        {
            ptDescriptor->pPayload = (MMP_UINT8*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                              length);

            if (ptDescriptor->pPayload)
            {
                PalMemcpy(ptDescriptor->pPayload, pData, length);
            }
            else
            {
                PalHeapFree(PAL_HEAP_DEFAULT, ptDescriptor);
                return MMP_NULL;
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
DescriptorKit_DestroyDescriptor(
    PSI_DESCRIPTOR* ptDescriptor)
{
    PSI_DESCRIPTOR* ptNextDescriptor;

    while (ptDescriptor)
    {
        ptNextDescriptor = ptDescriptor->ptNextDescriptor;

        if (ptDescriptor->pPayload)
            PalHeapFree(PAL_HEAP_DEFAULT, ptDescriptor->pPayload);

        if (ptDescriptor->ptDecodedContent)
            PalHeapFree(PAL_HEAP_DEFAULT, ptDescriptor->ptDecodedContent);

        PalHeapFree(PAL_HEAP_DEFAULT, ptDescriptor);
        ptDescriptor = ptNextDescriptor;
    }
}
