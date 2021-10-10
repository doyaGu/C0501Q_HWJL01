/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file psi_table_demux.c
 * Subtable demutiplexor.
 * @author I-Chun Lai
 * @version 0.1
 */

#include "psi_table_demux.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

// though the max section length of NIT, SDT, TDT and TOT is 1024,
// the max section length of EIT is 4096
#define MAX_SIZE_OF_PRIVATE_SECTION (4096)

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static void
_Demux_GatherSection(
    PSI_DECODER* ptDecoder,
    PSI_SECTION* ptSection);

//=============================================================================
//                              Public Function Definition
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
    void*               pCallbackData)
{
    PSI_DECODER*    ptDecoder   = MMP_NULL;
    PSI_DEMUX*      ptDemux     = MMP_NULL;

    ptDecoder = (PSI_DECODER*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                           sizeof(PSI_DECODER));
    if (MMP_NULL == ptDecoder)
        return MMP_NULL;

    ptDemux = (PSI_DEMUX*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                       sizeof(PSI_DEMUX));
    if (MMP_NULL == ptDemux)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptDecoder);
        return MMP_NULL;
    }

    // PSI decoder initilization
    PalMemset(ptDecoder, 0x0, sizeof(PSI_DECODER));
    ptDecoder->pfCallback       = (PSI_DECODER_CALLBACK)_Demux_GatherSection;
    ptDecoder->ptPrivateDecoder = (void*)ptDemux;
    ptDecoder->sectionMaxSize   = MAX_SIZE_OF_PRIVATE_SECTION;
    ptDecoder->bDiscontinuity   = MMP_TRUE;
    ptDecoder->allocId          = PAL_HEAP_DEFAULT;
    ptDecoder->pfAlloc          = _HeapAlloc; // PalHeapAlloc;
    ptDecoder->pfFree           = _HeapFree;  // PalHeapFree;

    // Subtable demux initilization
    PalMemset(ptDemux, 0x0, sizeof(PSI_DEMUX));
    ptDemux->ptDecoder      = ptDecoder;
    ptDemux->pfCallback     = pfCallback;
    ptDemux->pCallbackData  = pCallbackData;

    return ptDecoder;
}


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
    PSI_DECODER* ptDecoder)
{
    PSI_DEMUX* ptDemux = MMP_NULL;

    if (ptDecoder)
        ptDemux = (PSI_DEMUX*)ptDecoder->ptPrivateDecoder;
    else
        return;

    if (ptDemux)
    {
        PSI_DEMUX_SUBDECODER* ptSubdecoder;
        PSI_DEMUX_SUBDECODER* ptSubdecoderTemp;

        ptSubdecoder = ptDemux->ptFirstSubdecoder;
        while (ptSubdecoder)
        {
            ptSubdecoderTemp = ptSubdecoder;
            ptSubdecoder     = ptSubdecoder->ptNextSubdecoder;
            if (ptSubdecoderTemp->pfDetachSubdecoder)
            {
                ptSubdecoderTemp->pfDetachSubdecoder(
                    ptDemux,
                    (ptSubdecoderTemp->id >> 16) & 0xFFFF,
                    ptSubdecoderTemp->id & 0xFFFF);
            }
            else
            {
                PalHeapFree(PAL_HEAP_DEFAULT, ptSubdecoderTemp);
            }
        }
        PalHeapFree(PAL_HEAP_DEFAULT, ptDemux);
    }

    if (ptDecoder->ptCurrentSection)
    {
        SectionKit_DestroySection(ptDecoder->allocId, ptDecoder->pfFree, ptDecoder->ptCurrentSection);
    }

    PalHeapFree(PAL_HEAP_DEFAULT, ptDecoder);
}

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
                            MMP_UINT32 table_id_extension)
{
    MMP_UINT32 id;
    PSI_DEMUX_SUBDECODER* ptSubdecoder;

    if (MMP_NULL != ptDemux)
        ptSubdecoder = ptDemux->ptFirstSubdecoder;
    else
        return MMP_NULL;

    id = table_id << 16 | table_id_extension;
    while (ptSubdecoder)
    {
        if (ptSubdecoder->id == id)
            break;

        ptSubdecoder = ptSubdecoder->ptNextSubdecoder;
    }

    return ptSubdecoder;
}


//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Sends a PSI section to the right subtable decoder
 *
 * @param ptDecoder A PSI decoder to handle the TS packet decoding issue
 * @param ptSection The datagram of the current section
 * @return none
 */
//=============================================================================
static void
_Demux_GatherSection(
    PSI_DECODER* ptDecoder,
    PSI_SECTION* ptSection)
{
    PSI_DEMUX*            ptDemux      = MMP_NULL;
    PSI_DEMUX_SUBDECODER* ptSubdecoder = MMP_NULL;

    if (ptDecoder && ptDecoder->ptPrivateDecoder && ptSection)
    {
        ptDemux = (PSI_DEMUX*)ptDecoder->ptPrivateDecoder;
        // Check if a subtable decoder is available
        ptSubdecoder = psiTableDemux_GetSubdecoder(ptDemux,
                                                   ptSection->table_id,
                                                   ptSection->table_id_extension);
    }
    else
        return;

    if (MMP_NULL == ptSubdecoder)
    {
        // Tell the application we found a new subtable, so that it may attach a
        // subtable decoder
        ptDemux->pfCallback(ptDemux->pCallbackData,
                            ptDecoder,
                            ptSection->table_id,
                            ptSection->table_id_extension);
        // Check if a new subtable decoder is available
        ptSubdecoder = psiTableDemux_GetSubdecoder(ptDemux,
                                                   ptSection->table_id,
                                                   ptSection->table_id_extension);
    }

    if (MMP_NULL != ptSubdecoder)
    {
        ptSubdecoder->pfGatherSection(ptDemux->ptDecoder,
                                      ptSubdecoder->ptPrivateDecoder,
                                      ptSection);
    }
    else
    {
        SectionKit_DestroySection(ptDecoder->allocId, ptDecoder->pfFree, ptSection);
    }
}
