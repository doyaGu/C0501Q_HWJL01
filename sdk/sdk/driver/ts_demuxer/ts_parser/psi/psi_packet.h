/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file psi_packet.h
 * Used to process TS header decoding and then dispatch the decoded payload
 * section to privateDecoder for further decode.
 * @author Steven Hsiao
 * @version 0.1
 */

#ifndef PSI_PACKET_H
#define PSI_PACKET_H

#include "ite/mmp_types.h"
#include "psi_section_kit.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

typedef struct PSI_DECODER_TAG PSI_DECODER;

typedef void (*PSI_DECODER_CALLBACK)(PSI_DECODER* ptDecoder,
                                     PSI_SECTION* ptSection);

//=============================================================================
//                              Structure Definition
//=============================================================================

struct PSI_DECODER_TAG
{
    // Hang callback function to process PSI section. (process gathering
    // sections of a table to parse)
    PSI_DECODER_CALLBACK    pfCallback;

    // Point to individual PSI decoder structure. (PSI_XXX_DECODER)
    void*                   ptPrivateDecoder;

    // Temporal store length.
    MMP_UINT32              sectionMaxSize;

    MMP_UINT32              continuity_counter;
    MMP_BOOL                bDiscontinuity;

    // Temporal store for a completed PSI section.
    PSI_SECTION*            ptCurrentSection;

    // Indicate byte number in a section should be handled later.
    MMP_UINT32              needByte;

    // Indicate whether header of section is processed. (3 bytes at header of
    // section to be parsed, to obtain section_length)
    MMP_BOOL                bCompleteHeader;

    MMP_INT                 allocId;
    SECTION_PAYLOAD_ALLOC   pfAlloc;
    SECTION_PAYLOAD_FREE    pfFree;
};

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Inject a TS packet into a PSI decoder for content decoding.
 *
 * @param ptDecoder         The decoder to deocde a incoming TS packet.
 * @param pPacketAddress    The pointer is pointing to the start of a 188 bytes
 *                          TS packet.
 * @return none
 */
//=============================================================================
void
psiPacket_DecodePacket(
    PSI_DECODER*    ptDecoder,
    MMP_UINT8*      pPacketAddress);

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef PSI_PACKET_H
