/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file psi_section_kit.h
 * Used to create, destroy, and verify section.
 * @author Steven Hsiao
 * @version 0.1
 */

#ifndef PSI_SECTION_KIT_H
#define PSI_SECTION_KIT_H

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

//
// The design of the structure is based on private_section which is described
// in H222.0 p49.
// Basically the section field sequence should as the following unlike the
// structure definition in real code. The reason to do the field shiftment is
// for the purpose of byte alignment issue.
//
// <--- Spec private section field order --->
// xxx_section() {
//      table_id (8)
//      section_syntax_indicator (1)
//      private_indicator (1) (usually is reserved for future use)
//      reserved (2)
//      private_section_length (section length) (12)
//      if (section_syntax_indicator == '0') {
//          for (i = 0; i < N; i++) {
//              private_data_byte
//          }
//      }
//      else {
//          table_id_extension (16) (Ex. PAT - transport_stream_id...)
//          reserved (2)
//          version_number (5)
//          current_next_indicator (1)
//          section_number (8)
//          last_section_number (8)
//          for (i = 0; i < private_section_length - 9; i ++) {
//              private_data_byte
//          }
//          CRC32 (4)
//      }
// }
//
// Note: See H222.0 page 49. It said that the private_section allows data to
// be transmitted with a "minimum" of structure while enabling a decoder to
// parse a stream.
//
// For a field section_syntax_indicator should be declared here:
// If the indicator equals to 1, then the private section follows the generic
// section syntax behide the private_section_length (section_length) field.
// That is, the following data will be spec defined table syntax such as PAT,
// PMT, SDT, and so on.
// However if the indicator equals to 0, then the private_data_bytes
// immediatedly follow the private_section_length field. Under this cirumstance,
// the CRC field is no longer existed. Hence, the psiSectionCrcChk is always
// return MMP_TRUE.
// User can also reference H222.0 p49, clause 2.4.4.10 and Annex C p135 to
// get more further and detailed information.
//
typedef struct PSI_SECTION_TAG
{
    MMP_UINT32              table_id;
    MMP_UINT32              section_syntax_indicator;
//    MMP_UINT32              private_indicator;
    MMP_UINT32              section_length; // private_section_length
    MMP_UINT32              table_id_extension;
    MMP_UINT32              version_number;
    MMP_UINT32              current_next_indicator;
    MMP_UINT32              section_number;
    MMP_UINT32              last_section_number;

    MMP_UINT8*              pData;
    MMP_UINT8*              pPayloadStartAddress;
    MMP_UINT8*              pPayloadEndAddress;

//    MMP_UINT32              crc;

    struct PSI_SECTION_TAG* ptNextSection;
} PSI_SECTION;

typedef void* (*SECTION_PAYLOAD_ALLOC) (MMP_INT allocId, MMP_SIZE_T size);
typedef void  (*SECTION_PAYLOAD_FREE) (MMP_INT allocId, void* pFreeAddr);

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
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
    MMP_UINT32                  size);

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
    PSI_SECTION*                ptSection);

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
    PSI_SECTION* ptSection);

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef PSI_SECTION_KIT_H
