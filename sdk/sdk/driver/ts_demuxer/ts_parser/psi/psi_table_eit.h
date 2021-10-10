/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file psi_table_eit.h
 * Use to decode the SDT table of TS packets
 * @author I-Chun Lai
 * @version 0.1
 */
#ifndef PSI_TABLE_EIT_H
#define PSI_TABLE_EIT_H

#include "ite/mmp_types.h"
#include "psi_packet.h"
#include "psi_table_demux.h"
#include "psi_descriptor_kit.h"
#include "psi_time.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

typedef struct PSI_EIT_INFO_TAG PSI_EIT_INFO;

// The callback function will be called after the decode of EIT table is
// is done.
typedef void (*PSI_EIT_CALLBACK)(void *pCallbackData, PSI_EIT_INFO *ptEitInfo);
typedef MMP_BOOL (*PSI_SECTION_FILTER_CALLBACK)(void *pCallbackData, PSI_SECTION *ptSection);

//=============================================================================
//                              Structure Definition
//=============================================================================

//
// This structure is used to store a decoded EIT event description.
// (ETSI EN 300 468 V1.7.1 section 5.2.4).
//
typedef struct PSI_EIT_EVENT_TAG
{
    MMP_UINT32               event_id;
    PSI_MJDBCD_TIME          start_time;
    MMP_UINT32               duration;
    MMP_UINT32               running_status;
    MMP_UINT32               free_CA_mode;
    PSI_DESCRIPTOR           *ptFirstDescriptor;
    struct PSI_EIT_EVENT_TAG *ptNextEvent;
} PSI_EIT_EVENT;

//
// This structure is used to store a decoded EIT.
// (ETSI EN 300 468 V1.7.1 section 5.2.4).
//
struct PSI_EIT_INFO_TAG
{
    MMP_UINT32    table_id;
    MMP_UINT32    service_id;
    MMP_UINT32    version_number;
    MMP_UINT32    current_next_indicator;
    MMP_UINT32    section_number;
    MMP_UINT32    transport_stream_id;
    MMP_UINT32    original_network_id;
    MMP_UINT32    last_table_id;
    MMP_UINT32    totalEventCount;
    PSI_EIT_EVENT *ptFirstEvent;
};

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Attach the EIT decoder on the TS parser system
 *
 * @param ptDecoder             Subtable demultiplexor to which the decoder is
 *                              attached.
 * @param table_id              Table Id, 0x4E, 0x4F, 0x50~0x5F, or 0x60~0x6F.
 * @param table_id_extension    Table Id Extension. Here is TS Id.
 * @param pfCallback            A callback function is called after the table
 *                              decoding complete.
 * @param pCallbackData         private data given in argument to the callback.
 * @return                      0 if everything went ok.
 */
//=============================================================================
int
psiTableEIT_AttachDecoder(
    PSI_DECODER                 *ptDecoder,
    MMP_UINT32                  table_id,
    MMP_UINT32                  table_id_extension,
    PSI_EIT_CALLBACK            pfCallback,
    void                        *pCallbackData,
    PSI_SECTION_FILTER_CALLBACK pfSectionFilter);

//=============================================================================
/**
 * Destroy the EIT table and its allocated memory
 *
 * @param ptEitInfo A EIT table keeps the sections decoded information.
 * @return none
 */
//=============================================================================
void
psiTableEIT_DestroyTable(
    PSI_EIT_INFO *ptEitInfo);

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef PSI_TABLE_EIT_H