/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file psi_descriptor_kit.h
 * Used to create and destroy descriptor.
 * @author Steven Hsiao
 * @version 0.1
 */

#ifndef __psi_descriptor_kit_H_ECkyayCZ_UbM9_nHVN_5HD1_YpyF1Tdjd5WQ__
#define __psi_descriptor_kit_H_ECkyayCZ_UbM9_nHVN_5HD1_YpyF1Tdjd5WQ__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct PSI_DESCR_T
{
    uint32_t           descriptor_tag;
    uint32_t           descriptor_length;
    uint8_t            *pPayload;
    void               *ptDecodedContent;
    struct PSI_DESCR_T *ptNextDescriptor;
} PSI_DESCR;
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
 * @return        PSI_DESCRIPTOR* to store the information of the
 *                new descriptor.
 */
//=============================================================================
PSI_DESCR *
psi_descr_create(
    uint32_t tag,
    uint32_t length,
    uint8_t  *pData);

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
    PSI_DESCR *ptDescriptor);

#ifdef __cplusplus
}
#endif

#endif