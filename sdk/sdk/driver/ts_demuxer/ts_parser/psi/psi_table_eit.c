/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file psi_table_eit.c
 * Use to decode the EIT table of TS packets
 * @author I-Chun Lai
 * @version 0.1
 */


#include "bitstream_kit.h"
#include "psi_table_eit.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define CRC32_FIELD_SIZE                (4)

// Refer to spec. ETSI EN 300 468 p24.
// The section header has decoded partially in psi_packet module.
// The remaining un-decoded data include transport_stream_id(16),
// original_network_id(16), segment_last_section_number(8) and
// last_table_id(8). The count is 6 bytes.
#define EXTRA_SECTION_HEADER_SIZE       (6)

// 2 bytes mean descriptor_tag(8) and descriptor_length(8).
#define DESCRIPTOR_PRIOR_HEADER_SIZE    (2)

#define EIT_MAX_SECTION_SIZE            (4096)

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

// A private decoder to deal with the decoding issue of EIT table.
typedef struct PSI_EIT_DECODER_TAG
{
    PSI_EIT_INFO*               ptBuildingEit;
    MMP_UINT32                  last_section_number;

    PSI_SECTION*                ptFirstSection;
    PSI_EIT_CALLBACK            pfCallback;
    void*                       pCallbackData;
    PSI_SECTION_FILTER_CALLBACK pfSectionFilter;

    MMP_INT                     allocId;
    SECTION_PAYLOAD_ALLOC       pfAlloc;
    SECTION_PAYLOAD_FREE        pfFree;
} PSI_EIT_DECODER;

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
_EIT_DetachDecoder(
    PSI_DEMUX*  ptDemux,
    MMP_UINT32  table_id,
    MMP_UINT32  table_id_extension);

static void
_EIT_GatherSection(
    PSI_DECODER* ptDecoder,
    void*        ptPrivateDecoder,
    PSI_SECTION* ptSection);

static void
_EIT_DecodeSection(
    PSI_EIT_INFO*   ptEitInfo,
    PSI_SECTION*    ptSection);

static void
_EIT_InsertSection(
    PSI_EIT_DECODER*    ptEitDecoder,
    PSI_SECTION*        ptInsertSection);

static PSI_EIT_EVENT*
_EIT_AddEvent(
    PSI_EIT_INFO*   ptEitInfo,
    MMP_UINT32      event_id,
    PSI_MJDBCD_TIME start_time,
    MMP_UINT32      duration,
    MMP_UINT32      running_status,
    MMP_UINT32      free_CA_mode);

static void
_EIT_DestroyEvent(
    PSI_EIT_INFO* ptEitInfo);

static PSI_DESCRIPTOR*
_EIT_EventAddDescriptor(
    PSI_EIT_EVENT* ptEvent,
    MMP_UINT32 tag,
    MMP_UINT32 length,
    MMP_UINT8* pData);

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Attach the EIT decoder on the TS parser system
 *
 * @param ptDecoder             Subtable demultiplexor to which the decoder is
 *                              attached.
 * @param table_id              Table Id, 0x4E, 0x4F, 0x50~0x5F, or 0x60~0x6F.
 * @param table_id_extension    Table Id Extension. Here is TS Id.
 * @param pfCallback            A callback function is called after the table
 *                              decoding complete.
 * @param pCallbackData         private data given in argument to the callback.
 * @return                      0 if everything went ok.
 */
//=============================================================================
int
psiTableEIT_AttachDecoder(
    PSI_DECODER*                ptDecoder,
    MMP_UINT32                  table_id,
    MMP_UINT32                  table_id_extension,
    PSI_EIT_CALLBACK            pfCallback,
    void*                       pCallbackData,
    PSI_SECTION_FILTER_CALLBACK pfSectionFilter)
{
    PSI_DEMUX*              ptDemux = MMP_NULL;
    PSI_DEMUX_SUBDECODER*   ptSubdecoder = MMP_NULL;
    PSI_EIT_DECODER*        ptEitDecoder = MMP_NULL;

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

    ptEitDecoder = (PSI_EIT_DECODER*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                  sizeof(PSI_EIT_DECODER));
    if (MMP_NULL == ptEitDecoder)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptSubdecoder);
        return ATTACH_SUBDECODER_FAIL;
    }

    // Subtable decoder initialization
    ptSubdecoder->pfGatherSection   = &_EIT_GatherSection;
    ptSubdecoder->ptPrivateDecoder  = ptEitDecoder;
    ptSubdecoder->id = (table_id << 16) | table_id_extension;
    ptSubdecoder->pfDetachSubdecoder = _EIT_DetachDecoder;

    // Attach the subtable decoder to the demux
    ptSubdecoder->ptNextSubdecoder = ptDemux->ptFirstSubdecoder;
    ptDemux->ptFirstSubdecoder = ptSubdecoder;

    // EIT decoder initialization
    PalMemset(ptEitDecoder, 0x0, sizeof(PSI_EIT_DECODER));
    ptEitDecoder->pfCallback        = pfCallback;
    ptEitDecoder->pCallbackData     = pCallbackData;
    ptEitDecoder->pfSectionFilter   = pfSectionFilter;
    ptEitDecoder->allocId           = ptDecoder->allocId;
    ptEitDecoder->pfAlloc           = ptDecoder->pfAlloc;
    ptEitDecoder->pfFree            = ptDecoder->pfFree;

    return ATTACH_SUBDECODER_SUCCESS;
}

//=============================================================================
/**
 * Destroy the SDT table and its allocated memory
 *
 * @param ptSdtInfo A SDT table keeps the sections decoded information.
 * @return none
 */
//=============================================================================
void
psiTableEIT_DestroyTable(
    PSI_EIT_INFO* ptEitInfo)
{
    if (ptEitInfo)
    {
        _EIT_DestroyEvent(ptEitInfo);
        PalHeapFree(PAL_HEAP_DEFAULT, ptEitInfo);
    }
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Detach/Remove the EIT decoder from the TS parser system and also free all
 * allocated memory
 *
 * @param ptDemux               Subtable demultiplexor to which the decoder is
 *                              attached.
 * @param table_id              Table Id, 0x4E, 0x4F, 0x50~0x5F, or 0x60~0x6F.
 * @param table_id_extension    Table Id extension, Here is TS Id.
 * @return none
 */
//=============================================================================
void
_EIT_DetachDecoder(
    PSI_DEMUX*  ptDemux,
    MMP_UINT32  table_id,
    MMP_UINT32  table_id_extension)
{
    PSI_DEMUX_SUBDECODER*   ptSubdecoder = MMP_NULL;
    PSI_DEMUX_SUBDECODER**  pptPrevSubdecoder = MMP_NULL;
    PSI_EIT_DECODER*        ptEitDecoder = MMP_NULL;

    if (MMP_NULL != ptDemux)
    {
        ptSubdecoder = psiTableDemux_GetSubdecoder(ptDemux,
                                                   table_id,
                                                   table_id_extension);
        if (MMP_NULL == ptSubdecoder)
        {
            // No such EIT decoder
            return;
        }
    }
    else
        return;

    ptEitDecoder = (PSI_EIT_DECODER*)ptSubdecoder->ptPrivateDecoder;
    if (ptEitDecoder)
    {
        psiTableEIT_DestroyTable(ptEitDecoder->ptBuildingEit);
        ptEitDecoder->ptBuildingEit = MMP_NULL;

        SectionKit_DestroySection(ptEitDecoder->allocId, ptEitDecoder->pfFree, ptEitDecoder->ptFirstSection);
        PalHeapFree(PAL_HEAP_DEFAULT, ptEitDecoder);
    }

    pptPrevSubdecoder = &ptDemux->ptFirstSubdecoder;
    while (*pptPrevSubdecoder != ptSubdecoder)
        pptPrevSubdecoder = &(*pptPrevSubdecoder)->ptNextSubdecoder;

    *pptPrevSubdecoder = ptSubdecoder->ptNextSubdecoder;
    PalHeapFree(PAL_HEAP_DEFAULT, ptSubdecoder);
}

//=============================================================================
/**
 * Gather section data of the EIT. This is a callback function for the
 * subtable demultiplexor.
 *
 * @param ptDecoder         A PSI decoder to handle the TS packet decoding
 *                          issue
 * @param ptPrivateDecoder  Pointer to the EIT decoder structure
 * @param ptSection         The datagram of the current section
 * @return none
 */
//=============================================================================
static void
_EIT_GatherSection(
    PSI_DECODER* ptDecoder,
    void*        ptPrivateDecoder,
    PSI_SECTION* ptSection)
{
#ifdef USE_BITSTREAM_KIT
    BITSTREAM           tBitStream = { 0 };
#else
    MMP_UINT8*          pCurrentAddr = MMP_NULL;
#endif

    PSI_EIT_DECODER*    ptEitDecoder    = MMP_NULL;
    PSI_SECTION*        ptCurrentSection = MMP_NULL;

    if (ptDecoder && ptPrivateDecoder && ptSection)
    {
        ptEitDecoder = (PSI_EIT_DECODER*)ptPrivateDecoder;
        ptCurrentSection = ptSection;
    }
    else
    {
        // vincent noted on 9 april 2010:
        // added this to avoid memory leak but it means nothing
        // because we never reach here.
        if (ptSection)
            SectionKit_DestroySection(ptEitDecoder->allocId, ptEitDecoder->pfFree, ptSection);
        return;
    }

    // If the section_syntax_indicator != 1, then this section
    // is not a generic table.
    // On the other hand, it's not part of EIT
    if (ptCurrentSection->section_syntax_indicator != 1)
    {
        SectionKit_DestroySection(ptEitDecoder->allocId, ptEitDecoder->pfFree, ptCurrentSection);
        return;
    }

    if (ptEitDecoder->pfSectionFilter
     && ptEitDecoder->pfSectionFilter(ptEitDecoder->pCallbackData, ptCurrentSection))
    {
        SectionKit_DestroySection(ptEitDecoder->allocId, ptEitDecoder->pfFree, ptCurrentSection);
        return;
    }

    // The table should be re-inited.
    {
        psiTableEIT_DestroyTable(ptEitDecoder->ptBuildingEit);
        ptEitDecoder->ptBuildingEit = MMP_NULL;

        // Delete all chained sections
        SectionKit_DestroySection(ptEitDecoder->allocId, ptEitDecoder->pfFree, ptEitDecoder->ptFirstSection);
        ptEitDecoder->ptFirstSection = MMP_NULL;
    }

    // Append the section into the Table. If sections can form a complete
    // table, then process decoding.
    //if (MMP_NULL == ptEitDecoder->ptBuildingEit)
    {
        ptEitDecoder->ptBuildingEit =
            (PSI_EIT_INFO*)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(PSI_EIT_INFO));

        if (ptEitDecoder->ptBuildingEit)
        {
            PalMemset(ptEitDecoder->ptBuildingEit, 0x0, sizeof(PSI_EIT_INFO));

            // Partial section header has been decoded in psi_packet module,
            // just take over.
            ptEitDecoder->ptBuildingEit->table_id =
                ptCurrentSection->table_id;
            ptEitDecoder->ptBuildingEit->service_id =
                ptCurrentSection->table_id_extension;
            ptEitDecoder->ptBuildingEit->version_number =
                ptCurrentSection->version_number;
            ptEitDecoder->ptBuildingEit->current_next_indicator =
                ptCurrentSection->current_next_indicator;
            ptEitDecoder->last_section_number =
                ptCurrentSection->last_section_number;

#ifdef USE_BITSTREAM_KIT
            // Decode the remaining data.
            BitStreamKit_Init(&tBitStream, ptCurrentSection->pPayloadStartAddress);
            ptEitDecoder->ptBuildingEit->transport_stream_id =
                BitStreamKit_GetBits(&tBitStream, 16);
            ptEitDecoder->ptBuildingEit->original_network_id =
                BitStreamKit_GetBits(&tBitStream, 16);
            BitStreamKit_SkipBits(&tBitStream, 8);  // segment_last_section_number
            ptEitDecoder->ptBuildingEit->last_table_id =
                BitStreamKit_GetBits(&tBitStream, 8);
#else
            pCurrentAddr = ptCurrentSection->pPayloadStartAddress;
            ptEitDecoder->ptBuildingEit->transport_stream_id =
                (MMP_UINT32) (pCurrentAddr[0] << 8 |
                pCurrentAddr[1]);
            ptEitDecoder->ptBuildingEit->original_network_id =
                (MMP_UINT32) (pCurrentAddr[2] << 8 |
                pCurrentAddr[3]);
            ptEitDecoder->ptBuildingEit->last_table_id = pCurrentAddr[5];
#endif
        }
    }

    // Because segment_last_section_number is altered during gathering sections process,
    // we must parse this syntax every time when get a new section.

#ifdef USE_BITSTREAM_KIT
    // Skip transport_stream_id(16) and original_network_id(16)
    // to decode segment_last_section_number.
    BitStreamKit_Init(&tBitStream, ptCurrentSection->pPayloadStartAddress + 4);

    // "ptSection->section_number >> 3" implies segment number.
    //ptEitDecoder->segment_last_section_number[ptSection->section_number >> 3] =
    //    BitStreamKit_GetBits(&tBitStream, 8);
    BitStreamKit_SkipBits(&tBitStream, 8);
#else
    pCurrentAddr = ptCurrentSection->pPayloadStartAddress + 4;
    //ptEitDecoder->segment_last_section_number[ptSection->section_number >> 3] =
    //    pCurrentAddr[0];
#endif
    // Insert the section into the section list
    _EIT_InsertSection(ptEitDecoder, ptCurrentSection);

    // error handle fot allocation fail
    if (MMP_NULL == ptEitDecoder->ptBuildingEit)
        return;

    // Time for EIT table decode
    {
        // Decode the table
        _EIT_DecodeSection(ptEitDecoder->ptBuildingEit,
            ptEitDecoder->ptFirstSection);
        ptEitDecoder->ptBuildingEit->section_number = ptCurrentSection->section_number;

        // Callback to notify AP layer about the new Table constructed
        if (ptEitDecoder->pfCallback)
        {
            ptEitDecoder->pfCallback(ptEitDecoder->pCallbackData,
                ptEitDecoder->ptBuildingEit);
        }
        else
            SectionKit_DestroySection(ptEitDecoder->allocId, ptEitDecoder->pfFree, ptCurrentSection);

        // The AP will free the ptBuildingEit. We just need to re-init
        // the decoder parameter.
        ptEitDecoder->ptBuildingEit = MMP_NULL;
    }
}

//=============================================================================
/**
 * Decode the sections and then construct a program list of EIT table.
 *
 * @param ptEitInfo A EIT table keeps the sections decoded information.
 * @param ptSection A section list to form a complete EIT table.
 * @return none
 */
//=============================================================================
static void
_EIT_DecodeSection(
    PSI_EIT_INFO*   ptEitInfo,
    PSI_SECTION*    ptSection)
{
#ifdef USE_BITSTREAM_KIT
    BITSTREAM tBitStream    = { 0 };
#else
    MMP_UINT8* pCurrentAddr = MMP_NULL;
#endif
    MMP_UINT8* pEnd         = MMP_NULL;

    MMP_UINT32      event_id                = 0;
    PSI_MJDBCD_TIME start_time;
    MMP_UINT32      duration                = 0;
    MMP_UINT32      running_status          = 0;
    MMP_UINT32      free_CA_mode            = 0;
    MMP_UINT32      descriptors_loop_length = 0;
    PSI_EIT_EVENT*  ptEvent                 = MMP_NULL;

    // [20101207] Vincent: regular context switch to get better performance
    MMP_UINT32      count_for_context_switch = 0;

    while (ptSection)
    {               
#ifdef USE_BITSTREAM_KIT
        BitStreamKit_Init(&tBitStream,
            ptSection->pPayloadStartAddress
            + EXTRA_SECTION_HEADER_SIZE);

        // The loop is used to build up the information of EIT.
        while ((tBitStream.pStartAddress + CRC32_FIELD_SIZE)
                <= ptSection->pPayloadEndAddress)
        {
            start_time.low24  = 0;
            start_time.high16 = 0;

            event_id                = BitStreamKit_GetBits(&tBitStream, 16);
            start_time.high16       = BitStreamKit_GetBits(&tBitStream, 16);
            start_time.low24        = BitStreamKit_GetBits(&tBitStream, 24);
            duration                = BitStreamKit_GetBits(&tBitStream, 24);
            running_status          = BitStreamKit_GetBits(&tBitStream, 3);
            free_CA_mode            = BitStreamKit_GetBits(&tBitStream, 1);
            descriptors_loop_length = BitStreamKit_GetBits(&tBitStream, 12);

            ptEvent = _EIT_AddEvent(ptEitInfo,
                                    event_id,
                                    start_time,
                                    duration,
                                    running_status,
                                    free_CA_mode);
            // error handle for allocation fail
            if (MMP_NULL == ptEvent)
                return;

            pEnd = tBitStream.pStartAddress + descriptors_loop_length;
            if (pEnd > ptSection->pPayloadEndAddress)
                break;

            while (tBitStream.pStartAddress + DESCRIPTOR_PRIOR_HEADER_SIZE <= pEnd)
            {
                MMP_UINT32 descriptor_tag       = BitStreamKit_GetBits(&tBitStream, 8);
                MMP_UINT32 descriptor_length    = BitStreamKit_GetBits(&tBitStream, 8);
                if (tBitStream.pStartAddress + descriptor_length <= pEnd)
                    _EIT_EventAddDescriptor(ptEvent,
                                            descriptor_tag,
                                            descriptor_length,
                                            tBitStream.pStartAddress);
                tBitStream.pStartAddress += descriptor_length;
            }
        }
#else
        pCurrentAddr = ptSection->pPayloadStartAddress + EXTRA_SECTION_HEADER_SIZE;

        // The loop is used to build up the information of EIT.
        while ((pCurrentAddr + CRC32_FIELD_SIZE)
                <= ptSection->pPayloadEndAddress)
        {
            start_time.low24  = 0;
            start_time.high16 = 0;

            event_id                =
                (MMP_UINT32) (pCurrentAddr[0] << 8 |
                              pCurrentAddr[1]);
            start_time.high16       =
                (MMP_UINT32) (pCurrentAddr[2] << 8 |
                              pCurrentAddr[3]);
            start_time.low24        =
                (MMP_UINT32) (pCurrentAddr[4] << 16 |
                              pCurrentAddr[5] << 8 |
                              pCurrentAddr[6]);
            duration                =
                (MMP_UINT32) (pCurrentAddr[7] << 16 |
                              pCurrentAddr[8] << 8 |
                              pCurrentAddr[9]);
            running_status          =
                             (pCurrentAddr[10] & 0xE0) >> 5;
            free_CA_mode            =
                             (pCurrentAddr[10] & 0x10) >> 4;
            descriptors_loop_length =
                (MMP_UINT32) ((pCurrentAddr[10] & 0x0F) << 8 |
                               pCurrentAddr[11]);
            pCurrentAddr += 12;

            ptEvent = _EIT_AddEvent(ptEitInfo,
                                    event_id,
                                    start_time,
                                    duration,
                                    running_status,
                                    free_CA_mode);
            // error handle for allocation fail
            if (MMP_NULL == ptEvent)
                return;

            pEnd = pCurrentAddr + descriptors_loop_length;
            if (pEnd > ptSection->pPayloadEndAddress)
                break;

            while (pCurrentAddr + DESCRIPTOR_PRIOR_HEADER_SIZE <= pEnd)
            {
                MMP_UINT32 descriptor_tag       = pCurrentAddr[0];
                MMP_UINT32 descriptor_length    = pCurrentAddr[1];
                
                pCurrentAddr += 2;
                if (pCurrentAddr + descriptor_length <= pEnd)
                    _EIT_EventAddDescriptor(ptEvent,
                                            descriptor_tag,
                                            descriptor_length,
                                            pCurrentAddr);
                pCurrentAddr += descriptor_length;
            }
        }
#endif
        ptSection = ptSection->ptNextSection;
        
        // [20101207] Vincent: regular context switch to get better performance
        //if (count_for_context_switch%4 == 0)
        //    PalSleep(1);
        //count_for_context_switch++;
    }
}

//=============================================================================
/**
 * Destroy the event list of EIT table
 *
 * @param ptEitInfo A EIT table keeps the sections decoded information.
 * @return none
 */
//=============================================================================
static void
_EIT_DestroyEvent(
    PSI_EIT_INFO* ptEitInfo)
{
    if (ptEitInfo)
    {
        PSI_EIT_EVENT* ptEvent = ptEitInfo->ptFirstEvent;
        while (MMP_NULL != ptEvent)
        {
            PSI_EIT_EVENT* ptNextEvent = ptEvent->ptNextEvent;
            DescriptorKit_DestroyDescriptor(ptEvent->ptFirstDescriptor);
            PalHeapFree(PAL_HEAP_DEFAULT, ptEvent);
            ptEvent = ptNextEvent;
        }
        ptEitInfo->ptFirstEvent = MMP_NULL;
    }
}

//=============================================================================
/**
 * Insert a section into the section list of EIT decoder
 *
 * @param ptEitDecoder      The private Decoder to handle the EIT decode.
 * @param ptInsertSection   A section that we want to insert into section list.
 * @return none
 */
//=============================================================================
static void
_EIT_InsertSection(
    PSI_EIT_DECODER*    ptEitDecoder,
    PSI_SECTION*        ptInsertSection)
{
    PSI_SECTION* ptCurrentSection = MMP_NULL;
    PSI_SECTION* ptPreviousSection = MMP_NULL;
    MMP_UINT8*   pNewData = MMP_NULL;
    MMP_UINT32   sectionSize = 0;

    if (MMP_NULL == ptInsertSection || MMP_NULL == ptEitDecoder)
        return;

    sectionSize = (ptInsertSection->pPayloadEndAddress + CRC32_FIELD_SIZE - ptInsertSection->pData);
    if (sectionSize > EIT_MAX_SECTION_SIZE)
    {
        if (ptInsertSection->pData)
            PalHeapFree(PAL_HEAP_DEFAULT, ptInsertSection->pData);
        PalHeapFree(PAL_HEAP_DEFAULT, ptInsertSection);
        return;
    }

    // Try to save some memory here. The reason is each section needs 4k size, try to
    // allocate the proper size for different section to reduce the total size of consumption
    // of heap memory.
    pNewData = (MMP_UINT8*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                        sectionSize);
    // error handle for allocation fail
    if (MMP_NULL == pNewData)
    {
        if (ptInsertSection->pData)
            PalHeapFree(PAL_HEAP_DEFAULT, ptInsertSection->pData);
        PalHeapFree(PAL_HEAP_DEFAULT, ptInsertSection);
        return;
    }

    PalMemcpy(pNewData, ptInsertSection->pData, sectionSize);
    ptInsertSection->pPayloadStartAddress =
        pNewData + (ptInsertSection->pPayloadStartAddress - ptInsertSection->pData);
    ptInsertSection->pPayloadEndAddress =
        pNewData + (ptInsertSection->pPayloadEndAddress - ptInsertSection->pData);
    PalHeapFree(PAL_HEAP_DEFAULT, ptInsertSection->pData);
    ptInsertSection->pData = pNewData;

    if (ptEitDecoder->ptFirstSection)
    {
        ptCurrentSection = ptEitDecoder->ptFirstSection;

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
                    //ptEitDecoder->totalSectionCount++; // [20100723] Vincent marked.
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
                    ptEitDecoder->ptFirstSection = ptInsertSection;
                }

                ptInsertSection->ptNextSection = ptCurrentSection;
                //ptEitDecoder->totalSectionCount++; // [20100723] Vincent marked.
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
                    ptEitDecoder->ptFirstSection = ptInsertSection;
                }
                ptInsertSection->ptNextSection = ptCurrentSection->ptNextSection;

                ptCurrentSection->ptNextSection = MMP_NULL;
                SectionKit_DestroySection(ptEitDecoder->allocId, ptEitDecoder->pfFree, ptCurrentSection);

                break;
            }
        } while (ptCurrentSection);
    }
    else // The section is the first incoming section
    {
        ptEitDecoder->ptFirstSection = ptInsertSection;
        //ptEitDecoder->totalSectionCount = 1; // [20100723] Vincent marked.
    }
}

//=============================================================================
/**
 * Add a service description at the end of the SDT
 *
 * @param ptEitInfo                     A EIT table keeps the sections decoded
 *                                      information.
 * @param event_id                      Event ID.
 * @param start_time                    Start time
 * @param duration                      Duration
 * @param running_status                running_status
 * @param free_CA_mode                  free_CA_mode
 * @return Pointer to the latest inserted service description
 */
//=============================================================================
static PSI_EIT_EVENT*
_EIT_AddEvent(
    PSI_EIT_INFO*   ptEitInfo,
    MMP_UINT32      event_id,
    PSI_MJDBCD_TIME start_time,
    MMP_UINT32      duration,
    MMP_UINT32      running_status,
    MMP_UINT32      free_CA_mode)
{
    PSI_EIT_EVENT* ptEvent = PalHeapAlloc(PAL_HEAP_DEFAULT,
                                          sizeof(PSI_EIT_EVENT));
    if (MMP_NULL != ptEvent)
    {
        ptEvent->event_id           = event_id;
        ptEvent->start_time         = start_time;
        ptEvent->duration           = duration;
        ptEvent->running_status     = running_status;
        ptEvent->free_CA_mode       = free_CA_mode;
        ptEvent->ptNextEvent        = MMP_NULL;
        ptEvent->ptFirstDescriptor  = MMP_NULL;

        if (MMP_NULL == ptEitInfo->ptFirstEvent)
        {
            ptEitInfo->ptFirstEvent = ptEvent;
            ptEitInfo->totalEventCount = 1;
        }
        else
        {
            PSI_EIT_EVENT* ptLastEvent = ptEitInfo->ptFirstEvent;
            while (MMP_NULL != ptLastEvent->ptNextEvent)
            {
                ptLastEvent = ptLastEvent->ptNextEvent;
            }
            ptLastEvent->ptNextEvent = ptEvent;
            ptEitInfo->totalEventCount++;
        }
    }

    return ptEvent;
}

//=============================================================================
/**
 * Add a descriptor in the EIT event description
 *
 * @param ptEvent   Pointer to the EIT event description
 * @param tag       descriptor tag.
 * @param length    descriptor length
 * @param pData     descriptor data
 * @return Pointer to the lastest inserted descriptor
 */
//=============================================================================
static PSI_DESCRIPTOR*
_EIT_EventAddDescriptor(
    PSI_EIT_EVENT* ptEvent,
    MMP_UINT32 tag,
    MMP_UINT32 length,
    MMP_UINT8* pData)
{
    PSI_DESCRIPTOR* ptDescriptor =
        DescriptorKit_CreateDescriptor(tag, length, pData);

    if (MMP_NULL != ptDescriptor)
    {
        if (MMP_NULL == ptEvent->ptFirstDescriptor)
        {
            ptEvent->ptFirstDescriptor = ptDescriptor;
        }
        else
        {
            PSI_DESCRIPTOR* ptLastDescriptor = ptEvent->ptFirstDescriptor;
            while (MMP_NULL != ptLastDescriptor->ptNextDescriptor)
            {
                ptLastDescriptor = ptLastDescriptor->ptNextDescriptor;
            }
            ptLastDescriptor->ptNextDescriptor = ptDescriptor;
        }
    }

    return ptDescriptor;
}

