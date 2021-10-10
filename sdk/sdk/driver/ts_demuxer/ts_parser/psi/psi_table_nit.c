/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file psi_table_nit.c
 * Use to decode the NIT table of TS packets
 * @author I-Chun Lai
 * @version 0.1
 */

#ifdef USE_BITSTREAM_KIT
#include "bitstream_kit.h"
#endif
#include "psi_table_nit.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define CRC32_FIELD_SIZE            (4)
#define DESCRIPTOR_PRIOR_HEADER_SIZE (2)

//=============================================================================
//                              Structure Definition
//=============================================================================

// A private decoder to deal with the decoding issue of NIT table.
typedef struct PSI_NIT_DECODER_TAG
{
    //PSI_NIT_INFO        tCurrentNit; // [20100503] Vincent marked.
    PSI_NIT_INFO*           ptBuildingNit;
    //MMP_BOOL            bCurrentNitValid; // [20100503] Vincent marked.
    MMP_UINT32              last_section_number;

    MMP_UINT32              totalSectionCount;
    PSI_SECTION*            ptFirstSection;
    PSI_NIT_CALLBACK        pfCallback;
    void*                   pCallbackData;

    MMP_INT                 allocId;
    SECTION_PAYLOAD_ALLOC   pfAlloc;
    SECTION_PAYLOAD_FREE    pfFree;
} PSI_NIT_DECODER;

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static void
_NIT_DetachDecoder(
    PSI_DEMUX*  ptDemux,
    MMP_UINT32  table_id,
    MMP_UINT32  table_id_extension);

static void
_NIT_GatherSection(
    PSI_DECODER* ptDecoder,
    void*        ptPrivateDecoder,
    PSI_SECTION* ptSection);

static void
_NIT_DecodeSection(
    PSI_NIT_INFO*   ptNitInfo,
    PSI_SECTION*    ptSection);

static void
_NIT_InsertSection(
    PSI_NIT_DECODER*    ptNitDecoder,
    PSI_SECTION*        ptInsertSection);

static PSI_NIT_TRANSPORT_STREAM*
_NIT_AddTransportStream(
    PSI_NIT_INFO*   ptNitInfo,
    MMP_UINT32      transport_stream_id,
    MMP_UINT32      original_network_id);

static void
_NIT_DestroyTransportStream(
    PSI_NIT_INFO* ptNitInfo);

static PSI_DESCRIPTOR*
_NIT_TransportStreamAddDescriptor(
    PSI_NIT_TRANSPORT_STREAM* ptTransportStream,
    MMP_UINT32 tag,
    MMP_UINT32 length,
    MMP_UINT8* pData);

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Attach the NIT decoder on the TS parser system
 *
 * @param ptDecoder             Subtable demultiplexor to which the decoder is
 *                              attached.
 * @param table_id              Table Id, 0x40 or 0x41
 * @param table_id_extension    Table Id Extension.
 * @param pfCallback            A callback function is called after the table
 *                              decoding complete.
 * @param pCallbackData         private data given in argument to the callback.
 * @return                      0 if everything went ok.
 */
//=============================================================================
int
psiTableNIT_AttachDecoder(
    PSI_DECODER*        ptDecoder,
    MMP_UINT32          table_id,
    MMP_UINT32          table_id_extension,
    PSI_NIT_CALLBACK    pfCallback,
    void*               pCallbackData)
{
    PSI_DEMUX*              ptDemux = MMP_NULL;
    PSI_DEMUX_SUBDECODER*   ptSubdecoder = MMP_NULL;
    PSI_NIT_DECODER*        ptNitDecoder = MMP_NULL;

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

    ptNitDecoder = (PSI_NIT_DECODER*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                  sizeof(PSI_NIT_DECODER));
    if (MMP_NULL == ptNitDecoder)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptSubdecoder);
        return ATTACH_SUBDECODER_FAIL;
    }

    // Subtable decoder initialization
    ptSubdecoder->pfGatherSection   = &_NIT_GatherSection;
    ptSubdecoder->ptPrivateDecoder  = ptNitDecoder;
    ptSubdecoder->id = (table_id << 16) | table_id_extension;
    ptSubdecoder->pfDetachSubdecoder = _NIT_DetachDecoder;

    // Attach the subtable decoder to the demux
    ptSubdecoder->ptNextSubdecoder = ptDemux->ptFirstSubdecoder;
    ptDemux->ptFirstSubdecoder = ptSubdecoder;

    // NIT decoder initialization
    PalMemset(ptNitDecoder, 0x0, sizeof(PSI_NIT_DECODER));
    ptNitDecoder->pfCallback    = pfCallback;
    ptNitDecoder->pCallbackData = pCallbackData;
    ptNitDecoder->allocId       = ptDecoder->allocId;
    ptNitDecoder->pfAlloc       = ptDecoder->pfAlloc;
    ptNitDecoder->pfFree        = ptDecoder->pfFree;

    return ATTACH_SUBDECODER_SUCCESS;
}

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
    PSI_NIT_INFO* ptNitInfo)
{
    if (ptNitInfo)
    {
#if defined (SUPPORT_OTA) && defined (ENABLE_DSM_CC)
        DescriptorKit_DestroyDescriptor(ptNitInfo->ptFirstDescriptor);
#endif
        _NIT_DestroyTransportStream(ptNitInfo);
        PalHeapFree(PAL_HEAP_DEFAULT, ptNitInfo);
    }
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Detach/Remove the NIT decoder from the TS parser system and also free all
 * allocated memory
 *
 * @param ptDemux               Subtable demultiplexor to which the decoder is
 *                              attached.
 * @param table_id              Table Id, 0x40 or 0x41
 * @param table_id_extension    Table Id extension, Here is TS Id.
 * @return none
 */
//=============================================================================
void
_NIT_DetachDecoder(
    PSI_DEMUX*  ptDemux,
    MMP_UINT32  table_id,
    MMP_UINT32  table_id_extension)
{
    PSI_DEMUX_SUBDECODER*   ptSubdecoder = MMP_NULL;
    PSI_DEMUX_SUBDECODER**  pptPrevSubdecoder = MMP_NULL;
    PSI_NIT_DECODER*        ptNitDecoder = MMP_NULL;

    if (MMP_NULL != ptDemux)
    {
        ptSubdecoder = psiTableDemux_GetSubdecoder(ptDemux,
                                                   table_id,
                                                   table_id_extension);
        if (MMP_NULL == ptSubdecoder)
        {
            // No such NIT decoder
            return;
        }
    }
    else
        return;

    ptNitDecoder = (PSI_NIT_DECODER*)ptSubdecoder->ptPrivateDecoder;
    if (ptNitDecoder)
    {
        psiTableNIT_DestroyTable(ptNitDecoder->ptBuildingNit);
        ptNitDecoder->ptBuildingNit = MMP_NULL;

        SectionKit_DestroySection(ptNitDecoder->allocId, ptNitDecoder->pfFree, ptNitDecoder->ptFirstSection);
        PalHeapFree(PAL_HEAP_DEFAULT, ptNitDecoder);
    }

    pptPrevSubdecoder = &ptDemux->ptFirstSubdecoder;
    while (*pptPrevSubdecoder != ptSubdecoder)
        pptPrevSubdecoder = &(*pptPrevSubdecoder)->ptNextSubdecoder;

    *pptPrevSubdecoder = ptSubdecoder->ptNextSubdecoder;
    PalHeapFree(PAL_HEAP_DEFAULT, ptSubdecoder);
}

//=============================================================================
/**
 * Gather section data of the NIT. This is a callback function for the
 * subtable demultiplexor.
 *
 * @param ptDecoder         A PSI decoder to handle the TS packet decoding
 *                          issue
 * @param ptPrivateDecoder  Pointer to the NIT decoder structure
 * @param ptSection         The datagram of the current section
 * @return none
 */
//=============================================================================
static void
_NIT_GatherSection(
    PSI_DECODER* ptDecoder,
    void*        ptPrivateDecoder,
    PSI_SECTION* ptSection)
{
    MMP_BOOL            bSectionAppend  = MMP_TRUE;
    MMP_BOOL            bTableReInit    = MMP_FALSE;
    MMP_BOOL            bTableComplete  = MMP_FALSE;

    PSI_NIT_DECODER*    ptNitDecoder    = MMP_NULL;
    PSI_SECTION*        ptCurrentSection = MMP_NULL;

    if (ptDecoder && ptPrivateDecoder && ptSection)
    {
        ptNitDecoder = (PSI_NIT_DECODER*)ptPrivateDecoder;
        ptCurrentSection = ptSection;
    }
    else
    {
        // vincent noted on 9 april 2010:
        // added this to avoid memory leak but it means nothing
        // because we never reach here.
        if (ptSection)
            SectionKit_DestroySection(ptNitDecoder->allocId, ptNitDecoder->pfFree, ptSection);
        return;
    }

    // If the section_syntax_indicator != 1, then this section
    // is not a private section.
    // On the other hand, it's not part of NIT
    if (ptCurrentSection->section_syntax_indicator != 1)
          bSectionAppend = MMP_FALSE;

    // If bSectionAppend is true then we have a valid NIT section
    if (bSectionAppend)
    {
        // Discontinuity
        if (ptDecoder->bDiscontinuity)
        {
            bTableReInit = MMP_TRUE;
            ptDecoder->bDiscontinuity = MMP_FALSE;
        }
        else
        {
            // The building NIT is already existed. check the consistence of
            // the building table and incoming section.
            if (ptNitDecoder->ptBuildingNit)
            {
                // Any of the parameter comparison is failed.
                // We need to re-init the table structure.
                if ((ptNitDecoder->ptBuildingNit->network_id
                    != ptCurrentSection->table_id_extension)
                 || (ptNitDecoder->ptBuildingNit->version_number
                    != ptCurrentSection->version_number)
                 || (ptNitDecoder->last_section_number
                    != ptCurrentSection->last_section_number))
                {
                    bTableReInit = MMP_TRUE;
                }
            }
            /* [20100503] Vincent marked.
            else
            {
                // The last saved NIT table should be activated when the
                // current_next_indicator is 0 but the value of incoming
                // section is 1. Also, the version_number of the saved
                // NIT is equal to the version_number of the incoming
                // section. Usually, the version_number will be the number
                // that the previous version_number + 1. The mechanism is
                // used to provide the buffer time for decode side. The
                // benefit is obviously, that is, reduce the table re-build
                // time.
                if ((ptNitDecoder->bCurrentNitValid)
                 && (ptNitDecoder->tCurrentNit.version_number
                    == ptCurrentSection->version_number))
                {
                    // Notify the AP layer about that the new NIT should be
                    // activated.
                    if ((0 == ptNitDecoder->tCurrentNit.current_next_indicator)
                     && (1 == ptCurrentSection->current_next_indicator))
                    {
                        PSI_NIT_INFO* ptNitInfo =
                            (PSI_NIT_INFO*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                        sizeof(PSI_NIT_INFO));

                        if (ptNitInfo)
                        {
                            ptNitDecoder->tCurrentNit.current_next_indicator = 1;

                            PalMemcpy(ptNitInfo,
                                      &ptNitDecoder->tCurrentNit,
                                      sizeof(PSI_NIT_INFO));

                            ptNitDecoder->pfCallback(ptNitDecoder->pCallbackData,
                                                     ptNitInfo);
                        }
                        bSectionAppend = MMP_FALSE;
                    }
                    else
                        bTableReInit = MMP_TRUE;
                }
            }
            */
        }
    }

    // Check whether the table should be re-inited.
    if (bTableReInit)
    {
        if (ptNitDecoder->ptBuildingNit)
        {
            psiTableNIT_DestroyTable(ptNitDecoder->ptBuildingNit);
            ptNitDecoder->ptBuildingNit = MMP_NULL;
        }

        // Delete all chained sections
        SectionKit_DestroySection(ptNitDecoder->allocId, ptNitDecoder->pfFree, ptNitDecoder->ptFirstSection);
        ptNitDecoder->ptFirstSection = MMP_NULL;
        ptNitDecoder->totalSectionCount = 0;

        // Record the current pat to invalid
        //ptNitDecoder->bCurrentNitValid = MMP_FALSE; // [20100503] Vincent marked.
    }

    // Append the section into the Table. If sections can form a complete
    // table, then process decoding.
    if (bSectionAppend)
    {
        if (MMP_NULL == ptNitDecoder->ptBuildingNit)
        {
            ptNitDecoder->ptBuildingNit =
                (PSI_NIT_INFO*)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(PSI_NIT_INFO));

            if (ptNitDecoder->ptBuildingNit)
            {
                PalMemset(ptNitDecoder->ptBuildingNit, 0x0, sizeof(PSI_NIT_INFO));

                ptNitDecoder->ptBuildingNit->table_id =
                    ptCurrentSection->table_id;
                ptNitDecoder->ptBuildingNit->network_id =
                    ptCurrentSection->table_id_extension;
                ptNitDecoder->ptBuildingNit->version_number =
                    ptCurrentSection->version_number;
                ptNitDecoder->ptBuildingNit->current_next_indicator =
                    ptCurrentSection->current_next_indicator;
                ptNitDecoder->last_section_number =
                    ptCurrentSection->last_section_number;
            }
        }

        // Insert the section into the section list (ptNitDecoder->ptFirstSection).
        _NIT_InsertSection(ptNitDecoder, ptCurrentSection);

        // Check if we have all the sections
        bTableComplete = MMP_FALSE;
        if ((ptNitDecoder->totalSectionCount)
         == (ptNitDecoder->last_section_number + 1))
        {
            bTableComplete = MMP_TRUE;
        }

        // error handle fot allocation fail
        if (MMP_NULL == ptNitDecoder->ptBuildingNit)
            return;

        // Time for NIT table decode
        if (MMP_TRUE == bTableComplete)
        {
            /* [20100503] Vincent marked.
            // Save the building NIT inform to current NIT including
            // version_number, current_next_indicator, and etc.
            PalMemcpy(&ptNitDecoder->tCurrentNit,
                      ptNitDecoder->ptBuildingNit,
                      sizeof(PSI_NIT_INFO));
            ptNitDecoder->bCurrentNitValid = MMP_TRUE;
            */
            
            // Decode the table
            _NIT_DecodeSection(ptNitDecoder->ptBuildingNit,
                               ptNitDecoder->ptFirstSection);

            // Section information is stored in the building NIT table,
            // therefore, delete these sections.
            SectionKit_DestroySection(ptNitDecoder->allocId, ptNitDecoder->pfFree, ptNitDecoder->ptFirstSection);

            // Callback to notify AP layer about the new Table constructed
            if (ptNitDecoder->pfCallback)
            {
                ptNitDecoder->pfCallback(ptNitDecoder->pCallbackData,
                                         ptNitDecoder->ptBuildingNit);
            }

            // The AP will free the ptBuildingNit. We just need to re-init
            // the decoder parameter.
            ptNitDecoder->ptBuildingNit = MMP_NULL;
            ptNitDecoder->ptFirstSection = MMP_NULL;
            ptNitDecoder->totalSectionCount = 0;
        }
    }
    else // Ignore the incoming section.
        SectionKit_DestroySection(ptNitDecoder->allocId, ptNitDecoder->pfFree, ptCurrentSection);
}

//=============================================================================
/**
 * Decode the sections and then construct a transport stream list of NIT table.
 *
 * @param ptNitInfo A NIT table keeps the sections decoded information.
 * @param ptSection A section list to form a complete NIT table.
 * @return none
 */
//=============================================================================
static void
_NIT_DecodeSection(
    PSI_NIT_INFO*   ptNitInfo,
    PSI_SECTION*    ptSection)
{
#ifdef USE_BITSTREAM_KIT
    BITSTREAM tBitStream = { 0 };
#else
    MMP_UINT8* pCurrentAddr = MMP_NULL;
#endif
    MMP_UINT8* pEnd = MMP_NULL;

    MMP_UINT32 transport_stream_id              = 0;
    MMP_UINT32 original_network_id              = 0;
    MMP_UINT32 network_descriptors_length       = 0;
    MMP_UINT32 transport_descriptors_length     = 0;
    PSI_NIT_TRANSPORT_STREAM* ptTransportStream = MMP_NULL;

    while (ptSection)
    {
#ifdef USE_BITSTREAM_KIT
        BitStreamKit_Init(&tBitStream,
            ptSection->pPayloadStartAddress);

        BitStreamKit_SkipBits(&tBitStream, 4);
        network_descriptors_length = BitStreamKit_GetBits(&tBitStream, 12);
        BitStreamKit_SkipBits(&tBitStream, network_descriptors_length * 8);

        BitStreamKit_SkipBits(&tBitStream, 16);

        // The loop is used to build up the information of NIT.
        while ((tBitStream.pStartAddress + CRC32_FIELD_SIZE)
                <= ptSection->pPayloadEndAddress)
        {
            transport_stream_id = BitStreamKit_GetBits(&tBitStream, 16);
            original_network_id = BitStreamKit_GetBits(&tBitStream, 16);
            BitStreamKit_SkipBits(&tBitStream, 4);
            transport_descriptors_length = BitStreamKit_GetBits(&tBitStream, 12);

            ptTransportStream = _NIT_AddTransportStream(
                ptNitInfo,
                transport_stream_id,
                original_network_id);
            // error handle for allocation fail
            if (MMP_NULL == ptTransportStream)
                return;

            pEnd = tBitStream.pStartAddress + transport_descriptors_length;
            if (pEnd > ptSection->pPayloadEndAddress)
                break;

            while (tBitStream.pStartAddress + DESCRIPTOR_PRIOR_HEADER_SIZE <= pEnd)
            {
                MMP_UINT32 descriptor_tag       = BitStreamKit_GetBits(&tBitStream, 8);
                MMP_UINT32 descriptor_length    = BitStreamKit_GetBits(&tBitStream, 8);
                if (tBitStream.pStartAddress + descriptor_length <= pEnd)
                    _NIT_TransportStreamAddDescriptor(
                        ptTransportStream,
                        descriptor_tag,
                        descriptor_length,
                        tBitStream.pStartAddress);
                tBitStream.pStartAddress += descriptor_length;
            }
        }
#else
        pCurrentAddr = ptSection->pPayloadStartAddress;

        network_descriptors_length = (MMP_UINT32) (pCurrentAddr[0] << 8 | pCurrentAddr[1]) & 0xFFF;
#if defined (SUPPORT_OTA) && defined (ENABLE_DSM_CC)
        // First loop of NIT table.
        pCurrentAddr += 2;
        pEnd = pCurrentAddr + network_descriptors_length;
        if (pEnd > ptSection->pPayloadEndAddress)
            break;

        while (pCurrentAddr + DESCRIPTOR_PRIOR_HEADER_SIZE <= pEnd)
        {
            MMP_UINT32 descriptor_tag       = pCurrentAddr[0];
            MMP_UINT32 descriptor_length    = pCurrentAddr[1];
            pCurrentAddr += 2;
            if (pCurrentAddr + descriptor_length <= pEnd)
            {
                PSI_DESCRIPTOR* ptDescriptor =
                    DescriptorKit_CreateDescriptor(descriptor_tag, descriptor_length, pCurrentAddr);

                if (MMP_NULL != ptDescriptor)
                {
                    if (MMP_NULL == ptNitInfo->ptFirstDescriptor)
                    {
                        ptNitInfo->ptFirstDescriptor = ptDescriptor;
                    }
                    else
                    {
                        PSI_DESCRIPTOR* ptLastDescriptor =
                            ptNitInfo->ptFirstDescriptor;
                        while (MMP_NULL != ptLastDescriptor->ptNextDescriptor)
                        {
                            ptLastDescriptor = ptLastDescriptor->ptNextDescriptor;
                        }
                        ptLastDescriptor->ptNextDescriptor = ptDescriptor;
                    }
                }
            }
            pCurrentAddr += descriptor_length;
        }
        // reversed_future_use + transport_stream_loop_length (4 + 12 = 16 bits = 2 bytes)
        pCurrentAddr += 2;
#else
        pCurrentAddr += (4 + network_descriptors_length);
#endif

        // The loop is used to build up the information of SDT.
        while ((pCurrentAddr + CRC32_FIELD_SIZE)
                <= ptSection->pPayloadEndAddress)
        {
            transport_stream_id = (MMP_UINT32) (pCurrentAddr[0] << 8 | pCurrentAddr[1]);
            original_network_id = (MMP_UINT32) (pCurrentAddr[2] << 8 | pCurrentAddr[3]);
            transport_descriptors_length =
                (MMP_UINT32) (pCurrentAddr[4] << 8 | pCurrentAddr[5]) & 0xFFF;
            pCurrentAddr += 6;
            ptTransportStream = _NIT_AddTransportStream(
                ptNitInfo,
                transport_stream_id,
                original_network_id);
            // error handle for allocation fail
            if (MMP_NULL == ptTransportStream)
                return;

            pEnd = pCurrentAddr + transport_descriptors_length;
            if (pEnd > ptSection->pPayloadEndAddress)
                break;

            while (pCurrentAddr + DESCRIPTOR_PRIOR_HEADER_SIZE <= pEnd)
            {
                MMP_UINT32 descriptor_tag       = pCurrentAddr[0];
                MMP_UINT32 descriptor_length    = pCurrentAddr[1];
                pCurrentAddr += 2;
                if (pCurrentAddr + descriptor_length <= pEnd)
                    _NIT_TransportStreamAddDescriptor(
                        ptTransportStream,
                        descriptor_tag,
                        descriptor_length,
                        pCurrentAddr);
                pCurrentAddr += descriptor_length;
            }
        }
#endif
        ptSection = ptSection->ptNextSection;
    }
}

//=============================================================================
/**
 * Destroy the transport stream list of NIT table
 *
 * @param ptNitInfo A NIT table keeps the sections decoded information.
 * @return none
 */
//=============================================================================
static void
_NIT_DestroyTransportStream(
    PSI_NIT_INFO* ptNitInfo)
{
    if (ptNitInfo)
    {
        PSI_NIT_TRANSPORT_STREAM* ptTransportStream =
            ptNitInfo->ptFirstTransportStream;
        while (MMP_NULL != ptTransportStream)
        {
            PSI_NIT_TRANSPORT_STREAM* ptNextTransportStream =
                ptTransportStream->ptNextTransportStream;
            DescriptorKit_DestroyDescriptor(ptTransportStream->ptFirstDescriptor);
            PalHeapFree(PAL_HEAP_DEFAULT, ptTransportStream);
            ptTransportStream = ptNextTransportStream;
        }
        ptNitInfo->ptFirstTransportStream = MMP_NULL;
    }
}

//=============================================================================
/**
 * Insert a section into the section list of NIT decoder
 *
 * @param ptNitDecoder      The private Decoder to handle the NIT decode.
 * @param ptInsertSection   A section that we want to insert into section list.
 * @return none
 */
//=============================================================================
void
_NIT_InsertSection(
    PSI_NIT_DECODER*    ptNitDecoder,
    PSI_SECTION*        ptInsertSection)
{
    PSI_SECTION* ptCurrentSection = MMP_NULL;
    PSI_SECTION* ptPreviousSection = MMP_NULL;

    if (MMP_NULL == ptInsertSection || MMP_NULL == ptNitDecoder)
        return;

    if (ptNitDecoder->ptFirstSection)
    {
        ptCurrentSection = ptNitDecoder->ptFirstSection;

        do
        {
            if (ptCurrentSection->section_number
              < ptInsertSection->section_number)
            {
                if (MMP_NULL != ptCurrentSection->ptNextSection)
                {
                    ptPreviousSection = ptCurrentSection;
                    ptCurrentSection = ptCurrentSection->ptNextSection;
                }
                else
                {
                    // Append
                    ptCurrentSection->ptNextSection = ptInsertSection;
                    ptNitDecoder->totalSectionCount++;
                    break;
                }
            }
            else if (ptCurrentSection->section_number
                   > ptInsertSection->section_number)
            {
                if (MMP_NULL != ptPreviousSection)
                {
                    ptPreviousSection->ptNextSection = ptInsertSection;
                }
                else
                {
                    ptNitDecoder->ptFirstSection = ptInsertSection;
                }

                ptInsertSection->ptNextSection = ptCurrentSection;
                ptNitDecoder->totalSectionCount++;
                break;
            }
            else // if (ptCurrentSection->section_number
                 //  == ptInsertSection->section_number)
            {
                // Section duplication. Replace and free the old section.
                if (MMP_NULL != ptPreviousSection)
                {
                    ptPreviousSection->ptNextSection = ptInsertSection;
                }
                else
                {
                    ptNitDecoder->ptFirstSection = ptInsertSection;
                }
                ptInsertSection->ptNextSection = ptCurrentSection->ptNextSection;

                ptCurrentSection->ptNextSection = MMP_NULL;
                SectionKit_DestroySection(ptNitDecoder->allocId, ptNitDecoder->pfFree, ptCurrentSection);
                break;
            }
        } while (ptCurrentSection);
    }
    else // The section is the first incoming section
    {
        ptNitDecoder->ptFirstSection = ptInsertSection;
        ptNitDecoder->totalSectionCount = 1;
    }
}

//=============================================================================
/**
 * Add a transport stream description at the end of the NIT
 *
 * @param ptNitInfo                     A NIT table keeps the sections decoded
 *                                      information.
 * @param transport_stream_id           Transport stream ID.
 * @param original_network_id           Original network ID
 * @return Pointer to the latest inserted transport stream description
 */
//=============================================================================
static PSI_NIT_TRANSPORT_STREAM*
_NIT_AddTransportStream(
    PSI_NIT_INFO*   ptNitInfo,
    MMP_UINT32      transport_stream_id,
    MMP_UINT32      original_network_id)
{
    PSI_NIT_TRANSPORT_STREAM* ptTransportStream =
        PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(PSI_NIT_TRANSPORT_STREAM));
    if (MMP_NULL != ptTransportStream)
    {
        ptTransportStream->transport_stream_id      = transport_stream_id;
        ptTransportStream->original_network_id      = original_network_id;
        ptTransportStream->ptNextTransportStream    = MMP_NULL;
        ptTransportStream->ptFirstDescriptor        = MMP_NULL;

        if (MMP_NULL == ptNitInfo->ptFirstTransportStream)
        {
            ptNitInfo->ptFirstTransportStream = ptTransportStream;
        }
        else
        {
            PSI_NIT_TRANSPORT_STREAM* ptLastTransportStream =
                ptNitInfo->ptFirstTransportStream;
            while (MMP_NULL != ptLastTransportStream->ptNextTransportStream)
            {
                ptLastTransportStream =
                    ptLastTransportStream->ptNextTransportStream;
            }
            ptLastTransportStream->ptNextTransportStream = ptTransportStream;
        }
    }

    return ptTransportStream;
}

//=============================================================================
/**
 * Add a descriptor in the NIT transport stream description
 *
 * @param ptTransportStream Pointer to the NIT transport stream description
 * @param tag               descriptor tag.
 * @param length            descriptor length
 * @param pData             descriptor data
 * @return Pointer to the lastest inserted descriptor
 */
//=============================================================================
static PSI_DESCRIPTOR*
_NIT_TransportStreamAddDescriptor(
    PSI_NIT_TRANSPORT_STREAM* ptTransportStream,
    MMP_UINT32 tag,
    MMP_UINT32 length,
    MMP_UINT8* pData)
{
    PSI_DESCRIPTOR* ptDescriptor =
        DescriptorKit_CreateDescriptor(tag, length, pData);

    if (MMP_NULL != ptDescriptor)
    {
        if (MMP_NULL == ptTransportStream->ptFirstDescriptor)
        {
            ptTransportStream->ptFirstDescriptor = ptDescriptor;
        }
        else
        {
            PSI_DESCRIPTOR* ptLastDescriptor =
                ptTransportStream->ptFirstDescriptor;
            while (MMP_NULL != ptLastDescriptor->ptNextDescriptor)
            {
                ptLastDescriptor = ptLastDescriptor->ptNextDescriptor;
            }
            ptLastDescriptor->ptNextDescriptor = ptDescriptor;
        }
    }

    return ptDescriptor;
}
