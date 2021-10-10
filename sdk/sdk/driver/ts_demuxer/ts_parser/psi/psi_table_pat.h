/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file psi_table_pat.h
 * Use to decode the PAT table of TS packets
 * @author Steven Hsiao
 * @version 0.1
 */
#ifndef PSI_TABLE_PAT_H
#define PSI_TABLE_PAT_H

#include "ite/mmp_types.h"
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

typedef struct PSI_PAT_INFO_TAG PSI_PAT_INFO;

// The callback function will be called after the decode of PAT table is done.
typedef void (*PSI_PAT_CALLBACK) (void *pCallbackData, PSI_PAT_INFO *ptPatInfo);

//=============================================================================
//                              Structure Definition
//=============================================================================

// A strucut to store the program mapping information,
// that is, pogram_number -> program_map_PID.
typedef struct PSI_PAT_PROGRAM_TAG
{
    MMP_UINT32                 program_number;
    MMP_UINT32                 program_map_PID;
    struct PSI_PAT_PROGRAM_TAG *ptNextProgram;
} PSI_PAT_PROGRAM;

// A structure to keep the information of the PAT table.
// The main key to identify different PAT table is by
// transport_stream_id.
struct PSI_PAT_INFO_TAG
{
    MMP_UINT32      transport_stream_id;
    MMP_UINT32      version_number;
    MMP_UINT32      current_next_indicator;
    MMP_UINT32      totalProgramCount;
    PSI_PAT_PROGRAM *ptFirstProgram;
};

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Attach the PAT decoder on the TS parser system
 *
 * @param pfCallback    A callback function is called after the table
 *                      decoding complete.
 * @param pCallbackData The datagram of the callback function
 * @return              PSI_DECODER* to handle psi section parsing and
 *                      data collecion of PAT (PID: 0) of incoming TS packets.
 */
//=============================================================================
PSI_DECODER *
psiTablePAT_AttachDecoder(
    PSI_PAT_CALLBACK pfCallback,
    void             *pCallbackData);

//=============================================================================
/**
 * Detach/Remove the PAT decoder from the TS parser system and also free all
 * allocated memory
 *
 * @param ptDecoder The existed decoder to handle PAT decode.
 * @return none
 */
//=============================================================================
void
psiTablePAT_DetachDecoder(
    PSI_DECODER *ptDecoder);

//=============================================================================
/**
 * Destroy the PAT table and its allocated memory
 *
 * @param pPatInfo A PAT table keeps the sections decoded information.
 * @return none
 */
//=============================================================================
void
psiTablePAT_DestroyTable(
    PSI_PAT_INFO *ptPatInfo);

#ifdef __cplusplus
}
#endif

#endif