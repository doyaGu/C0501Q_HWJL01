/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file psi_table_tdt.c
 * Use to decode the TDT table of TS packets
 * @author I-Chun Lai
 * @version 0.1
 */

#ifdef USE_BITSTREAM_KIT
#include "bitstream_kit.h"
#endif
#include "psi_table_tdt.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define UTC_TIME_FIELD_SIZE (5)

//=============================================================================
//                              Structure Definition
//=============================================================================

// A private decoder to deal with the decoding issue of TDT table.
typedef struct PSI_TDT_DECODER_TAG
{
    PSI_MJDBCD_TIME         tUtcTime;

    PSI_TDT_CALLBACK        pfCallback;
    void*                   pCallbackData;

    MMP_INT                 allocId;
    SECTION_PAYLOAD_ALLOC   pfAlloc;
    SECTION_PAYLOAD_FREE    pfFree;
} PSI_TDT_DECODER;

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static void
_TDT_DetachDecoder(
    PSI_DEMUX*  ptDemux,
    MMP_UINT32  table_id,
    MMP_UINT32  table_id_extension);

static void
_TDT_GatherSection(
    PSI_DECODER* ptDecoder,
    void*        ptPrivateDecoder,
    PSI_SECTION* ptSection);

//=============================================================================
//                              Public Function Definition
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
    PSI_DECODER*        ptDecoder,
    MMP_UINT32          table_id,
    MMP_UINT32          table_id_extension,
    PSI_TDT_CALLBACK    pfCallback,
    void*               pCallbackData)
{
    PSI_DEMUX*              ptDemux = MMP_NULL;
    PSI_DEMUX_SUBDECODER*   ptSubdecoder = MMP_NULL;
    PSI_TDT_DECODER*        ptTdtDecoder = MMP_NULL;

    if (MMP_NULL != ptDecoder && MMP_NULL != ptDecoder->ptPrivateDecoder)
        ptDemux = (PSI_DEMUX*)ptDecoder->ptPrivateDecoder;
    else
        return ATTACH_SUBDECODER_FAIL;

    if (psiTableDemux_GetSubdecoder(ptDemux, table_id, table_id_extension))
    {
        // A subtable decoder for the same table_id and table_id_extension
        // is already attached.
        return ATTACH_SUBDECODER_FAIL;
    }

    ptSubdecoder = (PSI_DEMUX_SUBDECODER*)PalHeapAlloc(
        PAL_HEAP_DEFAULT, sizeof(PSI_DEMUX_SUBDECODER));
    if (MMP_NULL == ptSubdecoder)
        return ATTACH_SUBDECODER_FAIL;

    ptTdtDecoder = (PSI_TDT_DECODER*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                  sizeof(PSI_TDT_DECODER));
    if (MMP_NULL == ptTdtDecoder)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptSubdecoder);
        return ATTACH_SUBDECODER_FAIL;
    }

    // Subtable decoder initialization
    ptSubdecoder->pfGatherSection   = &_TDT_GatherSection;
    ptSubdecoder->ptPrivateDecoder  = ptTdtDecoder;
    ptSubdecoder->id = (table_id << 16) | table_id_extension;
    ptSubdecoder->pfDetachSubdecoder = _TDT_DetachDecoder;

    // Attach the subtable decoder to the demux
    ptSubdecoder->ptNextSubdecoder = ptDemux->ptFirstSubdecoder;
    ptDemux->ptFirstSubdecoder = ptSubdecoder;

    // TDT decoder initialization
    PalMemset(ptTdtDecoder, 0x0, sizeof(PSI_TDT_DECODER));
    ptTdtDecoder->pfCallback    = pfCallback;
    ptTdtDecoder->pCallbackData = pCallbackData;
    ptTdtDecoder->allocId       = ptDecoder->allocId;
    ptTdtDecoder->pfAlloc       = ptDecoder->pfAlloc;
    ptTdtDecoder->pfFree        = ptDecoder->pfFree;

    return ATTACH_SUBDECODER_SUCCESS;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Detach/Remove the TDT decoder from the TS parser system and also free all
 * allocated memory
 *
 * @param ptDemux               Subtable demultiplexor to which the decoder is
 *                              attached.
 * @param table_id              Table Id, 0x70
 * @param table_id_extension    Table Id extension, Here is TS Id.
 * @return none
 */
//=============================================================================
void
_TDT_DetachDecoder(
    PSI_DEMUX*  ptDemux,
    MMP_UINT32  table_id,
    MMP_UINT32  table_id_extension)
{
    PSI_DEMUX_SUBDECODER*   ptSubdecoder = MMP_NULL;
    PSI_DEMUX_SUBDECODER**  pptPrevSubdecoder = MMP_NULL;
    PSI_TDT_DECODER*        ptTdtDecoder = MMP_NULL;

    if (MMP_NULL != ptDemux)
    {
        ptSubdecoder = psiTableDemux_GetSubdecoder(ptDemux,
                                                   table_id,
                                                   table_id_extension);
        if (MMP_NULL == ptSubdecoder)
        {
            // No such TDT decoder
            return;
        }
    }
    else
        return;

    ptTdtDecoder = (PSI_TDT_DECODER*)ptSubdecoder->ptPrivateDecoder;
    if (ptTdtDecoder)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptTdtDecoder);
    }

    pptPrevSubdecoder = &ptDemux->ptFirstSubdecoder;
    while (*pptPrevSubdecoder != ptSubdecoder)
        pptPrevSubdecoder = &(*pptPrevSubdecoder)->ptNextSubdecoder;

    *pptPrevSubdecoder = ptSubdecoder->ptNextSubdecoder;
    PalHeapFree(PAL_HEAP_DEFAULT, ptSubdecoder);
}

//=============================================================================
/**
 * Gather section data of the TDT. This is a callback function for the
 * subtable demultiplexor.
 *
 * @param ptDecoder         A PSI decoder to handle the TS packet decoding
 *                          issue
 * @param ptPrivateDecoder  Pointer to the TDT decoder structure
 * @param ptSection         The datagram of the current section
 * @return none
 */
//=============================================================================
static void
_TDT_GatherSection(
    PSI_DECODER* ptDecoder,
    void*        ptPrivateDecoder,
    PSI_SECTION* ptSection)
{
#ifdef USE_BITSTREAM_KIT
    BITSTREAM           tBitStream = { 0 };
#endif

    PSI_TDT_DECODER*    ptTdtDecoder    = MMP_NULL;
    PSI_SECTION*        ptCurrentSection = MMP_NULL;

    if (ptDecoder && ptPrivateDecoder && ptSection)
    {
        ptTdtDecoder = (PSI_TDT_DECODER*)ptPrivateDecoder;
        ptCurrentSection = ptSection;
    }
    else
    {
        // vincent noted on 9 april 2010:
        // added this to avoid memory leak but it means nothing
        // because we never reach here.
        if (ptSection)
            SectionKit_DestroySection(ptTdtDecoder->allocId, ptTdtDecoder->pfFree, ptSection);
        return;
    }

    // If the section_syntax_indicator != 0, then this section
    // is not a generic table.
    // On the other hand, it's not part of TDT
    if (ptCurrentSection->section_syntax_indicator == 0)
    {
//         if ((UTC_TIME_FIELD_SIZE == ptCurrentSection->section_length)
//          && (UTC_TIME_FIELD_SIZE == ptCurrentSection->pPayloadEndAddress - ptCurrentSection->pPayloadStartAddress))
//         {
#ifdef USE_BITSTREAM_KIT
            BitStreamKit_Init(&tBitStream, ptCurrentSection->pPayloadStartAddress);
            ptTdtDecoder->tUtcTime.high16 = BitStreamKit_GetBits(&tBitStream, 16);
            ptTdtDecoder->tUtcTime.low24  = BitStreamKit_GetBits(&tBitStream, 24);
#else
            ptTdtDecoder->tUtcTime.high16 = 
                (MMP_UINT32) (ptCurrentSection->pPayloadStartAddress[0] << 8 |
                              ptCurrentSection->pPayloadStartAddress[1]);
            ptTdtDecoder->tUtcTime.low24 =
                (MMP_UINT32) (ptCurrentSection->pPayloadStartAddress[2] << 16 |
                              ptCurrentSection->pPayloadStartAddress[3] << 8  |
                              ptCurrentSection->pPayloadStartAddress[4]);
#endif
            // Callback to notify AP layer about the latest UTC Time is received
            if (ptTdtDecoder->pfCallback)
            {
                ptTdtDecoder->pfCallback(ptTdtDecoder->pCallbackData,
                                         ptTdtDecoder->tUtcTime);
            }
//         }
    }

    SectionKit_DestroySection(ptTdtDecoder->allocId, ptTdtDecoder->pfFree, ptCurrentSection);
}
