/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file psi_table_tdt.h
 * Use to decode the TDT table of TS packets
 * @author I-Chun Lai
 * @version 0.1
 */
#ifndef PSI_TABLE_TDT_H
#define PSI_TABLE_TDT_H

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

// The callback function will be called after the decode of TDT table is
// is done.
typedef void (*PSI_TDT_CALLBACK)(void *pCallbackData, PSI_MJDBCD_TIME tUtcTime);

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Attach the TDT decoder on the TS parser system
 *
 * @param ptDecoder             Subtable demultiplexor to which the decoder is
 *                              attached.
 * @param table_id              Table Id, 0x70
 * @param table_id_extension    Table Id Extension.
 * @param pfCallback            A callback function is called after the table
 *                              decoding complete.
 * @param pCallbackData         private data given in argument to the callback.
 * @return                      0 if everything went ok.
 */
//=============================================================================
int
psiTableTDT_AttachDecoder(
    PSI_DECODER      *ptDecoder,
    MMP_UINT32       table_id,
    MMP_UINT32       table_id_extension,
    PSI_TDT_CALLBACK pfCallback,
    void             *pCallbackData);

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef PSI_TABLE_TDT_H