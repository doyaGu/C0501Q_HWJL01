/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file psi_table_tot.c
 * Use to decode the TOT table of TS packets
 * @author I-Chun Lai
 * @version 0.1
 */

#ifdef USE_BITSTREAM_KIT
#include "bitstream_kit.h"
#endif
#include "psi_table_tot.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define UTC_TIME_FIELD_SIZE         (5)
#define CRC32_FIELD_SIZE            (4)

#define INVALID_DESCRIPTOR_TAG      (0xFF)
#define INVALID_DESCRIPTOR_LENGTH   (255)

//=============================================================================
//                              Structure Definition
//=============================================================================

// A private decoder to deal with the decoding issue of TOT table.
typedef struct PSI_TOT_DECODER_TAG
{
    PSI_TOT_INFO*           ptBuildingTot;

    PSI_TOT_CALLBACK        pfCallback;
    void*                   pCallbackData;

    MMP_INT                 allocId;
    SECTION_PAYLOAD_ALLOC   pfAlloc;
    SECTION_PAYLOAD_FREE    pfFree;
} PSI_TOT_DECODER;

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static void
_TOT_AddDescriptor(
    PSI_TOT_INFO* ptTotInfo,
    MMP_UINT32 tag,
    MMP_UINT32 length,
    MMP_UINT8* pData);

static void
_TOT_DetachDecoder(
    PSI_DEMUX*  ptDemux,
    MMP_UINT32  table_id,
    MMP_UINT32  table_id_extension);

static void
_TOT_GatherSection(
    PSI_DECODER* ptDecoder,
    void*        ptPrivateDecoder,
    PSI_SECTION* ptSection);

//=============================================================================
//                              Public Function Definition
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
    PSI_DECODER*        ptDecoder,
    MMP_UINT32          table_id,
    MMP_UINT32          table_id_extension,
    PSI_TOT_CALLBACK    pfCallback,
    void*               pCallbackData)
{
    PSI_DEMUX*              ptDemux = MMP_NULL;
    PSI_DEMUX_SUBDECODER*   ptSubdecoder = MMP_NULL;
    PSI_TOT_DECODER*        ptTotDecoder = MMP_NULL;

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

    ptTotDecoder = (PSI_TOT_DECODER*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                  sizeof(PSI_TOT_DECODER));
    if (MMP_NULL == ptTotDecoder)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptSubdecoder);
        return ATTACH_SUBDECODER_FAIL;
    }

    // Subtable decoder initialization
    ptSubdecoder->pfGatherSection   = &_TOT_GatherSection;
    ptSubdecoder->ptPrivateDecoder  = ptTotDecoder;
    ptSubdecoder->id = (table_id << 16) | table_id_extension;
    ptSubdecoder->pfDetachSubdecoder = _TOT_DetachDecoder;

    // Attach the subtable decoder to the demux
    ptSubdecoder->ptNextSubdecoder = ptDemux->ptFirstSubdecoder;
    ptDemux->ptFirstSubdecoder = ptSubdecoder;

    // TOT decoder initialization
    PalMemset(ptTotDecoder, 0x0, sizeof(PSI_TOT_DECODER));
    ptTotDecoder->pfCallback    = pfCallback;
    ptTotDecoder->pCallbackData = pCallbackData;
    ptTotDecoder->allocId       = ptDecoder->allocId;
    ptTotDecoder->pfAlloc       = ptDecoder->pfAlloc;
    ptTotDecoder->pfFree        = ptDecoder->pfFree;

    return ATTACH_SUBDECODER_SUCCESS;
}

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
    PSI_TOT_INFO* ptTotInfo)
{
    if (ptTotInfo)
    {
        if (ptTotInfo->ptFirstDescriptor)
            DescriptorKit_DestroyDescriptor(ptTotInfo->ptFirstDescriptor);

        PalHeapFree(PAL_HEAP_DEFAULT, ptTotInfo);
    }
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Detach/Remove the TOT decoder from the TS parser system and also free all
 * allocated memory
 *
 * @param ptDemux               Subtable demultiplexor to which the decoder is
 *                              attached.
 * @param table_id              Table Id, 0x73
 * @param table_id_extension    Table Id extension, Here is TS Id.
 * @return none
 */
//=============================================================================
void
_TOT_DetachDecoder(
    PSI_DEMUX*  ptDemux,
    MMP_UINT32  table_id,
    MMP_UINT32  table_id_extension)
{
    PSI_DEMUX_SUBDECODER*   ptSubdecoder = MMP_NULL;
    PSI_DEMUX_SUBDECODER**  pptPrevSubdecoder = MMP_NULL;
    PSI_TOT_DECODER*        ptTotDecoder = MMP_NULL;

    if (MMP_NULL != ptDemux)
    {
        ptSubdecoder = psiTableDemux_GetSubdecoder(ptDemux,
                                                   table_id,
                                                   table_id_extension);
        if (MMP_NULL == ptSubdecoder)
        {
            // No such TOT decoder
            return;
        }
    }
    else
        return;

    ptTotDecoder = (PSI_TOT_DECODER*)ptSubdecoder->ptPrivateDecoder;
    if (ptTotDecoder)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptTotDecoder);
    }

    pptPrevSubdecoder = &ptDemux->ptFirstSubdecoder;
    while (*pptPrevSubdecoder != ptSubdecoder)
        pptPrevSubdecoder = &(*pptPrevSubdecoder)->ptNextSubdecoder;

    *pptPrevSubdecoder = ptSubdecoder->ptNextSubdecoder;
    PalHeapFree(PAL_HEAP_DEFAULT, ptSubdecoder);
}

//=============================================================================
/**
 * Gather section data of the TOT. This is a callback function for the
 * subtable demultiplexor.
 *
 * @param ptDecoder         A PSI decoder to handle the TS packet decoding
 *                          issue
 * @param ptPrivateDecoder  Pointer to the TOT decoder structure
 * @param ptSection         The datagram of the current section
 * @return none
 */
//=============================================================================
static void
_TOT_GatherSection(
    PSI_DECODER* ptDecoder,
    void*        ptPrivateDecoder,
    PSI_SECTION* ptSection)
{
#ifdef USE_BITSTREAM_KIT
    BITSTREAM           tBitStream = { 0 };
#else
    MMP_UINT8*          pCurrentAddr = MMP_NULL;
#endif
    PSI_TOT_DECODER*    ptTotDecoder    = MMP_NULL;
    PSI_SECTION*        ptCurrentSection = MMP_NULL;

    if (ptDecoder && ptPrivateDecoder && ptSection)
    {
        ptTotDecoder = (PSI_TOT_DECODER*)ptPrivateDecoder;
        ptCurrentSection = ptSection;
    }
    else
    {
        // vincent noted on 9 april 2010:
        // added this to avoid memory leak but it means nothing
        // because we never reach here.
        if (ptSection)
            SectionKit_DestroySection(ptTotDecoder->allocId, ptTotDecoder->pfFree, ptSection);
        return;
    }

    // If the section_syntax_indicator != 0, then this section
    // is not a generic table.
    // On the other hand, it's not part of TOT
    if (0 == ptCurrentSection->section_syntax_indicator)
    {
        ptTotDecoder->ptBuildingTot =
            (PSI_TOT_INFO*)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(PSI_TOT_INFO));
        if (ptTotDecoder->ptBuildingTot)
        {
            MMP_UINT32 descriptors_loop_length = 0;
            MMP_UINT8* pEnd = MMP_NULL;

            PalMemset(ptTotDecoder->ptBuildingTot, 0x0, sizeof(PSI_TOT_INFO));
#ifdef USE_BITSTREAM_KIT
            BitStreamKit_Init(&tBitStream, ptCurrentSection->pPayloadStartAddress);
            ptTotDecoder->ptBuildingTot->UTC_time.high16 = BitStreamKit_GetBits(&tBitStream, 16);
            ptTotDecoder->ptBuildingTot->UTC_time.low24  = BitStreamKit_GetBits(&tBitStream, 24);
            BitStreamKit_SkipBits(&tBitStream, 4);
            descriptors_loop_length                      = BitStreamKit_GetBits(&tBitStream, 12);

            pEnd = ptSection->pPayloadEndAddress - CRC32_FIELD_SIZE;
            while (tBitStream.pStartAddress < pEnd)
            {
                MMP_UINT32 descriptor_tag       = BitStreamKit_GetBits(&tBitStream, 8);
                MMP_UINT32 descriptor_length    = BitStreamKit_GetBits(&tBitStream, 8);
                if (tBitStream.pStartAddress + descriptor_length <= pEnd)
                    _TOT_AddDescriptor(ptTotDecoder->ptBuildingTot,
                    descriptor_tag,
                    descriptor_length,
                    tBitStream.pStartAddress);
                tBitStream.pStartAddress += descriptor_length;
            }
#else
            pCurrentAddr = ptCurrentSection->pPayloadStartAddress;
            ptTotDecoder->ptBuildingTot->UTC_time.high16 =
                (MMP_UINT32) (pCurrentAddr[0] << 8 |
                              pCurrentAddr[1]);
            ptTotDecoder->ptBuildingTot->UTC_time.low24  =
                (MMP_UINT32) (pCurrentAddr[2] << 16 |
                              pCurrentAddr[3] << 8 |
                              pCurrentAddr[4]);
            descriptors_loop_length =
                (MMP_UINT32) ((pCurrentAddr[5] & 0x0F) << 8 |
                               pCurrentAddr[6]);
            pCurrentAddr += 7;
            pEnd = ptSection->pPayloadEndAddress - CRC32_FIELD_SIZE;
            while (pCurrentAddr < pEnd)
            {
                MMP_UINT32 descriptor_tag       = pCurrentAddr[0];
                MMP_UINT32 descriptor_length    = pCurrentAddr[1];
                pCurrentAddr += 2;
                if (pCurrentAddr + descriptor_length <= pEnd)
                    _TOT_AddDescriptor(ptTotDecoder->ptBuildingTot,
                    descriptor_tag,
                    descriptor_length,
                    pCurrentAddr);
                pCurrentAddr += descriptor_length;
            }

#endif
            // Callback to notify AP layer about the latest UTC Time is received
            if (ptTotDecoder->pfCallback)
            {
                ptTotDecoder->pfCallback(ptTotDecoder->pCallbackData,
                    ptTotDecoder->ptBuildingTot);
            }
        }
    }

    SectionKit_DestroySection(ptTotDecoder->allocId, ptTotDecoder->pfFree, ptCurrentSection);
}

//=============================================================================
/**
 * Add a new descriptor in the descriptor list.
 * @param ptTotInfo A TOT info structure to keep the TOT table information.
 * @param tag       A unique descriptor tag to identify the descriptor.
 * @param length    The descriptor content data size behind the field.
 *                  descriptor_length.
 * @param pData     The descriptor content data.
 * @return none
 */
//=============================================================================
static void
_TOT_AddDescriptor(
    PSI_TOT_INFO* ptTotInfo,
    MMP_UINT32 tag,
    MMP_UINT32 length,
    MMP_UINT8* pData)
{
    PSI_DESCRIPTOR*     ptDescriptor = MMP_NULL;
    PSI_DESCRIPTOR*     ptLastDescriptor = MMP_NULL;

    // Invalid input
    if ((MMP_NULL == ptTotInfo)
     || (MMP_NULL == pData)
     || (tag >= INVALID_DESCRIPTOR_TAG)
     || (length >= INVALID_DESCRIPTOR_LENGTH))
    {
        return;
    }

    ptDescriptor = DescriptorKit_CreateDescriptor(tag, length, pData);
    if (ptDescriptor)
    {
        if (MMP_NULL == ptTotInfo->ptFirstDescriptor)
            ptTotInfo->ptFirstDescriptor = ptDescriptor;
        else
        {
            ptLastDescriptor = ptTotInfo->ptFirstDescriptor;
            while (ptLastDescriptor->ptNextDescriptor)
                ptLastDescriptor = ptLastDescriptor->ptNextDescriptor;
            ptLastDescriptor->ptNextDescriptor = ptDescriptor;
            ptLastDescriptor = ptDescriptor;
        }
    }
    else
        return;
}
