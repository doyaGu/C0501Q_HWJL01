/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file psi_descriptor_kit.h
 * Used to create and destroy descriptor.
 * @author Steven Hsiao
 * @version 0.1
 */

#ifndef PSI_DESCRIPTOR_KIT_H
#define PSI_DESCRIPTOR_KIT_H

#include "ite/mmp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct PSI_DESCRIPTOR_TAG
{
    MMP_UINT32                  descriptor_tag;
    MMP_UINT32                  descriptor_length;
    MMP_UINT8*                  pPayload;
    void*                       ptDecodedContent;
    struct PSI_DESCRIPTOR_TAG*  ptNextDescriptor;
}PSI_DESCRIPTOR;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
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
    MMP_UINT    tag,
    MMP_UINT    length,
    MMP_UINT8*  pData);


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
    PSI_DESCRIPTOR* ptDescriptor);


#ifdef __cplusplus
}
#endif

#endif // End of #ifndef PSI_DESCRIPTOR_KIT_H

