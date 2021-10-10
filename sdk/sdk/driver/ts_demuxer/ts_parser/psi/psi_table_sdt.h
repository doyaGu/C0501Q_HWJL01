/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file psi_table_sdt.h
 * Use to decode the SDT table of TS packets
 * @author I-Chun Lai
 * @version 0.1
 */
#ifndef PSI_TABLE_SDT_H
#define PSI_TABLE_SDT_H

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

typedef struct PSI_SDT_INFO_TAG PSI_SDT_INFO;

// The callback function will be called after the decode of SDT table is
// is done.
typedef void (*PSI_SDT_CALLBACK)(void *pCallbackData, PSI_SDT_INFO *ptSdtInfo);

//=============================================================================
//                              Structure Definition
//=============================================================================

//
// This structure is used to store a decoded SDT service description.
// (ETSI EN 300 468 V1.7.1 section 5.2.3).
//
typedef struct PSI_SDT_SERVICE_TAG
{
    MMP_UINT32                 service_id;
    MMP_UINT32                 EIT_schedule_flag;
    MMP_UINT32                 EIT_present_following_flag;
    MMP_UINT32                 running_status;
    MMP_UINT32                 free_CA_mode;
    PSI_DESCRIPTOR             *ptFirstDescriptor;
    struct PSI_SDT_SERVICE_TAG *ptNextService;
} PSI_SDT_SERVICE;

//
// This structure is used to store a decoded SDT.
// (ETSI EN 300 468 V1.7.1 section 5.2.3).
//
struct PSI_SDT_INFO_TAG
{
    MMP_UINT32      transport_stream_id;
    MMP_UINT32      version_number;
    MMP_UINT32      current_next_indicator;
    MMP_UINT32      original_network_id;
    PSI_SDT_SERVICE *ptFirstService;
};

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Attach the SDT decoder on the TS parser system
 *
 * @param ptDecoder             Subtable demultiplexor to which the decoder is
 *                              attached.
 * @param table_id              Table Id, 0x42 or 0x46.
 * @param table_id_extension    Table Id Extension. Here is TS Id.
 * @param pfCallback            A callback function is called after the table
 *                              decoding complete.
 * @param pCallbackData         private data given in argument to the callback.
 * @return                      0 if everything went ok.
 */
//=============================================================================
int
psiTableSDT_AttachDecoder(
    PSI_DECODER      *ptDecoder,
    MMP_UINT32       table_id,
    MMP_UINT32       table_id_extension,
    PSI_SDT_CALLBACK pfCallback,
    void             *pCallbackData);

//=============================================================================
/**
 * Destroy the SDT table and its allocated memory
 *
 * @param ptSdtInfo A SDT table keeps the sections decoded information.
 * @return none
 */
//=============================================================================
void
psiTableSDT_DestroyTable(
    PSI_SDT_INFO *ptSdtInfo);

#ifdef __cplusplus
}
#endif

#endif