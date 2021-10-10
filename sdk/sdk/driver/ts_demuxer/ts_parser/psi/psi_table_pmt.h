/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file psi_table_pmt.h
 * Use to decode the PMT table of TS packets
 * @author Steven Hsiao
 * @version 0.1
 */
#ifndef PSI_TABLE_PMT_H
#define PSI_TABLE_PMT_H

#include "ite/mmp_types.h"
#include "psi_descriptor_kit.h"
#include "psi_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

typedef struct PSI_PMT_INFO_TAG PSI_PMT_INFO;

// The callback function will be called after the decode of PMT table is
// is done.
typedef void (*PSI_PMT_CALLBACK) (void *pCallbackData, PSI_PMT_INFO *ptPmtInfo);

//=============================================================================
//                              Structure Definition
//=============================================================================

// The structure is used to store each Elementary stream information, and also
// wrapped the descriptor list for AP usage.
// Note: The ptFirstDescriptor is pointing to the first descriptor of the
//       second descriptor list (ES info descriptor list) which is located
//       behide the field ES_info_length of TS_program_map_sections()
//       See H222.0 p48 to get further details.
typedef struct PSI_PMT_ES_INFO_TAG
{
    MMP_UINT32                 stream_type;
    MMP_UINT32                 elementary_PID;
    PSI_DESCRIPTOR             *ptFirstDescriptor;
    struct PSI_PMT_ES_INFO_TAG *ptNexEsInfo;
} PSI_PMT_ES_INFO;

// The structure is used to store whole PMT table information
// Note1: The ptFirstDescriptor is pointing to the first descriptor of the
//        first descriptor list (program info descriptor list) which is
//        located behide the field program_info_length of
//        TS_program_map_sections() See H222.0 p48 to get further details.
// Note2: There exists one or more elementray streams of a PMT table. The
//        ptFirstEsInfo is pointing to the first elementary stream info of
//        the ES_Info list. See H222.0 p48 to get further details.
struct PSI_PMT_INFO_TAG
{
    MMP_UINT32      PID;
    MMP_UINT32      program_number;
    MMP_UINT32      version_number;
    MMP_UINT32      current_next_indicator;
    MMP_UINT32      PCR_PID;
    PSI_DESCRIPTOR  *ptFirstDescriptor;     // in practice, check if subtitle,
                                            // teletext and ac3 exist
    MMP_UINT32      totalEsCount;
    PSI_PMT_ES_INFO *ptFirstEsInfo;
};

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Attach the PMT decoder on the TS parser system.
 *
 * @param program_number    The program_number is used to identify different PMT
 * @param pfCallback        A callback function is called after the table
 *                          decoding complete.
 * @param pCallbackData     The datagram of the callback function
 * @return                  PSI_DECODER* to handle psi section parsing and
 *                          data collecion of PMT of incoming TS packets.
 */
//=============================================================================
PSI_DECODER *
psiTablePMT_AttachDecoder(
    MMP_UINT32       program_number,
    PSI_PMT_CALLBACK pfCallback,
    void             *pCallbackData);

//=============================================================================
/**
 * Detach the PMT decoder from the TS parser system and also free all
 * allocated memory.
 *
 * @param ptDecoder The existed decoder to handle PMT decode.
 * @return none
 */
//=============================================================================
void
psiTablePMT_DetachDecoder(
    PSI_DECODER *ptDecoder);

//=============================================================================
/**
 * Destroy the PMT table and its allocated memory.
 *
 * @param ptPmtInfo A PMT table keeps the sections decoded information.
 * @return none
 */
//=============================================================================
void
psiTablePMT_DestroyTable(
    PSI_PMT_INFO *ptPmtInfo);

//=============================================================================
/**
 * Add a new descriptor in the first descriptor list (program descriptor list).
 * @param ptPmtInfo A PMT info structure to keep the PMT table information.
 * @param tag       A unique descriptor tag to identify the descriptor.
 * @param length    The descriptor content data size behind the field.
 *                  descriptor_length.
 * @param pData     The descriptor content data.
 * @return none
 */
//=============================================================================
void
psiTablePMT_AddDescriptor(
    PSI_PMT_INFO *ptPmtInfo,
    MMP_UINT32   tag,
    MMP_UINT32   length,
    MMP_UINT8    *pData);

//=============================================================================
/**
 * Add a new descriptor in the second descriptor list (ES_Info descriptor list).
 * @param ptEsInfo  The structure to keep the information of a elementary stream
 * @param tag       A unique descriptor tag to identify the descriptor.
 * @param length    The descriptor content data size behind the field.
 *                  descriptor_length.
 * @param pData     The descriptor content data.
 * @no return value.
 */
//=============================================================================
void
psiTablePMT_EsAddDescriptor(
    PSI_PMT_ES_INFO *ptEsInfo,
    MMP_UINT32      tag,
    MMP_UINT32      length,
    MMP_UINT8       *pData);

#ifdef __cplusplus
}
#endif

#endif