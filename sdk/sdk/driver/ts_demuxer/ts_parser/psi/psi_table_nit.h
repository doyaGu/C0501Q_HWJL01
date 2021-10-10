/*
 * Copyright (c) 2008 ITE Technology Corp. All Rights Reserved.
 */
/** @file psi_table_nit.h
 * Use to decode the NIT table of TS packets
 * @author I-Chun Lai
 * @version 0.1
 */
#ifndef PSI_TABLE_NIT_H
#define PSI_TABLE_NIT_H

#include "ite/mmp_types.h"
#include "psi_packet.h"
#include "psi_table_demux.h"
#include "psi_descriptor_kit.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================
typedef struct PSI_NIT_INFO_TAG PSI_NIT_INFO;

// The callback function will be called after the decode of NIT table is
// is done.
typedef void (*PSI_NIT_CALLBACK)(void *pCallbackData, PSI_NIT_INFO *ptNitInfo);

//=============================================================================
//                              Structure Definition
//=============================================================================

//
// This structure is used to store a decoded NIT transport stream description.
// (ETSI EN 300 468 V1.7.1 section 5.2.1).
//
typedef struct PSI_NIT_TRANSPORT_STREAM_TAG
{
    MMP_UINT32                          transport_stream_id;
    MMP_UINT32                          original_network_id;
    PSI_DESCRIPTOR                      *ptFirstDescriptor;
    struct PSI_NIT_TRANSPORT_STREAM_TAG *ptNextTransportStream;
} PSI_NIT_TRANSPORT_STREAM;

//
// This structure is used to store a decoded NIT.
// (ETSI EN 300 468 V1.7.1 section 5.2.1).
//
struct PSI_NIT_INFO_TAG
{
    MMP_UINT32               table_id;
    MMP_UINT32               network_id;
    MMP_UINT32               version_number;
    MMP_UINT32               current_next_indicator;
#if defined (SUPPORT_OTA) && defined (ENABLE_DSM_CC)
    PSI_DESCRIPTOR           *ptFirstDescriptor;
#endif
    PSI_NIT_TRANSPORT_STREAM *ptFirstTransportStream;
};

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Attach the NIT decoder on the TS parser system
 *
 * @param ptDecoder             Subtable demultiplexor to which the decoder is
 *                              attached.
 * @param table_id              Table Id, 0x40, 0x41
 * @param table_id_extension    Table Id Extension.  Here is network id.
 * @param pfCallback            A callback function is called after the table
 *                              decoding complete.
 * @param pCallbackData         private data given in argument to the callback.
 * @return                      0 if everything went ok.
 */
//=============================================================================
int
psiTableNIT_AttachDecoder(
    PSI_DECODER      *ptDecoder,
    MMP_UINT32       table_id,
    MMP_UINT32       table_id_extension,
    PSI_NIT_CALLBACK pfCallback,
    void             *pCallbackData);

//=============================================================================
/**
 * Destroy the NIT table and its allocated memory.
 *
 * @param ptPmtInfo A NIT table keeps the sections decoded information.
 * @return none
 */
//=============================================================================
void
psiTableNIT_DestroyTable(
    PSI_NIT_INFO *ptNitInfo);

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef PSI_TABLE_NIT_H