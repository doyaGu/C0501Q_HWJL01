/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file psi_table_tot.h
 * Use to decode the TOT table of TS packets
 * @author I-Chun Lai
 * @version 0.1
 */
#ifndef PSI_TABLE_TOT_H
#define PSI_TABLE_TOT_H

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
typedef struct PSI_TOT_INFO_TAG PSI_TOT_INFO;

// The callback function will be called after the decode of TOT table is
// is done.
typedef void (*PSI_TOT_CALLBACK)(void *pCallbackData, PSI_TOT_INFO *ptTotInfo);

//=============================================================================
//                              Structure Definition
//=============================================================================
struct PSI_TOT_INFO_TAG
{
    PSI_MJDBCD_TIME UTC_time;
    PSI_DESCRIPTOR  *ptFirstDescriptor;
};

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Attach the TOT decoder on the TS parser system
 *
 * @param ptDecoder             Subtable demultiplexor to which the decoder is
 *                              attached.
 * @param table_id              Table Id, 0x73
 * @param table_id_extension    Table Id Extension.
 * @param pfCallback            A callback function is called after the table
 *                              decoding complete.
 * @param pCallbackData         private data given in argument to the callback.
 * @return                      0 if everything went ok.
 */
//=============================================================================
int
psiTableTOT_AttachDecoder(
    PSI_DECODER      *ptDecoder,
    MMP_UINT32       table_id,
    MMP_UINT32       table_id_extension,
    PSI_TOT_CALLBACK pfCallback,
    void             *pCallbackData);

//=============================================================================
/**
 * Destroy the TOT table and its allocated memory.
 *
 * @param ptPmtInfo A TOT table keeps the sections decoded information.
 * @return none
 */
//=============================================================================
void
psiTableTOT_DestroyTable(
    PSI_TOT_INFO *ptTotInfo);

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef PSI_TABLE_TOT_H