/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file psi_table_demux.h
 * Subtable demutiplexor.
 * @author I-Chun Lai
 * @version 0.1
 */

#ifndef PSI_TABLE_DEMUX_H
#define PSI_TABLE_DEMUX_H

#include "ite/mmp_types.h"
#include "psi_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

#define ATTACH_SUBDECODER_SUCCESS   (0)
#define ATTACH_SUBDECODER_FAIL      (1)

//=============================================================================
//                              Macro Definition
//=============================================================================

// Callback used in case of a new subtable detected.
typedef void (*PSI_DEMUX_CALLBACK)(void*        pCallbackData,
                                   PSI_DECODER* ptDecoder,
                                   MMP_UINT32   table_id,
                                   MMP_UINT32   table_id_extension);

// The callback function will be called while a section needs to be decoded by
// the subtable decoder.
typedef void (*PSI_DEMUX_SUBDECODER_CALLBACK)(PSI_DECODER* ptDecoder,
                                              void*        ptPrivateDecoder,
                                              PSI_SECTION* ptSection);

typedef struct PSI_DEMUX_TAG PSI_DEMUX;
typedef struct PSI_DEMUX_SUBDECODER_TAG PSI_DEMUX_SUBDECODER;

//=============================================================================
//                              Structure Definition
//=============================================================================

// NOTE: In SI, it's not allowed to tell tables by PID, because some have the
//       same one. So using "(table_id << 16) | table_id_extension" to be the
//       only one identification.

// This structure contains the data specific to the decoding of one subtable.
struct PSI_DEMUX_SUBDECODER_TAG
{
    MMP_UINT32                          id; // (table_id << 16) | table_id_extension
    PSI_DEMUX_SUBDECODER_CALLBACK       pfGatherSection;

    // A private decoder that be used and keep data at local for specific
    // subtable decoding.
    void*                               ptPrivateDecoder;

    PSI_DEMUX_SUBDECODER*               ptNextSubdecoder;

    // Provide function callback in detach process.
    void (*pfDetachSubdecoder)(PSI_DEMUX* ptDemux,
                               MMP_UINT32 table_id,
                               MMP_UINT32 table_id_extension);
};

// This structure contains the subtables demultiplexor data, such as the
// decoders and new subtable callback.
struct PSI_DEMUX_TAG
{
    PSI_DECODER*            ptDecoder;
    PSI_DEMUX_SUBDECODER*   ptFirstSubdecoder;

    // This function will be called when find a new subtable,
    // callback to upper layer to do something like attaching subtable decoder.
    PSI_DEMUX_CALLBACK      pfCallback;

    void*                   pCallbackData;
};

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Creates a new demux structure.
 *
 * @param pfCallback    A callback function called when a new type of subtable
 *                      is found.
 * @param pCallbackData private data given in argument to the callback.
 * @return              a handle to the new demux structure.
 */
//=============================================================================
PSI_DECODER*
psiTableDemux_AttachDemux(
    PSI_DEMUX_CALLBACK  pfCallback,
    void*               pCallbackData);


//=============================================================================
/**
 * Destroys a demux structure.
 *
 * @param ptDecoder     The handle of the demux to be destroyed.
 * @return none
 */
//=============================================================================
void
psiTableDemux_DetachDemux(
    PSI_DECODER* ptDecoder);


//=============================================================================
/**
 * Finds a subtable decoder given the table id and table id extension.
 *
 * @param ptDecoder             Pointer to the demux structure.
 * @param table_id              Table ID of the wanted subtable.
 * @param table_id_extension    Table ID extension of the wanted subtable.
 * @return                      a pointer to the found subdecoder, or NULL.
 */
//=============================================================================
PSI_DEMUX_SUBDECODER*
psiTableDemux_GetSubdecoder(PSI_DEMUX* ptDemux,
                            MMP_UINT32 table_id,
                            MMP_UINT32 table_id_extension);

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef PSI_TABLE_DEMUX_H
