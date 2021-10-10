/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file psi_table_sdt.c
 * Use to decode the SDT table of TS packets
 * @author I-Chun Lai
 * @version 0.1
 */

#ifdef USE_BITSTREAM_KIT
#include "bitstream_kit.h"
#endif
#include "psi_table_sdt.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define CRC32_FIELD_SIZE (4)
#define EXTRA_SECTION_HEADER_SIZE (3)
#define DESCRIPTOR_PRIOR_HEADER_SIZE (2)

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

// A private decoder to deal with the decoding issue of SDT table.
typedef struct PSI_SDT_DECODER_TAG
{
    //PSI_SDT_INFO        tCurrentSdt; // [20100503] Vincent marked.
    PSI_SDT_INFO*           ptBuildingSdt;
    //MMP_BOOL            bCurrentSdtValid; // [20100503] Vincent marked.
    MMP_UINT32              last_section_number;

    MMP_UINT32              totalSectionCount;
    PSI_SECTION*            ptFirstSection;
    PSI_SDT_CALLBACK        pfCallback;
    void*                   pCallbackData;

    MMP_INT                 allocId;
    SECTION_PAYLOAD_ALLOC   pfAlloc;
    SECTION_PAYLOAD_FREE    pfFree;
} PSI_SDT_DECODER;

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
_SDT_DetachDecoder(
    PSI_DEMUX*  ptDemux,
    MMP_UINT32  table_id,
    MMP_UINT32  table_id_extension);

static void
_SDT_GatherSection(
    PSI_DECODER* ptDecoder,
    void*        ptPrivateDecoder,
    PSI_SECTION* ptSection);

static void
_SDT_DecodeSection(
    PSI_SDT_INFO*   ptSdtInfo,
    PSI_SECTION*    ptSection);

static void
_SDT_InsertSection(
    PSI_SDT_DECODER*    ptSdtDecoder,
    PSI_SECTION*        ptInsertSection);

static PSI_SDT_SERVICE*
_SDT_AddService(
    PSI_SDT_INFO*   ptSdtInfo,
    MMP_UINT32      service_id,
    MMP_UINT32      EIT_schedule_flag,
    MMP_UINT32      EIT_present_following_flag,
    MMP_UINT32      running_status,
    MMP_UINT32      free_CA_mode);

static void
_SDT_DestroyService(
    PSI_SDT_INFO* ptSdtInfo);

static PSI_DESCRIPTOR*
_SDT_ServiceAddDescriptor(
    PSI_SDT_SERVICE* ptService,
    MMP_UINT32 tag,
    MMP_UINT32 length,
    MMP_UINT8* pData);

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Attach the SDT decoder on the TS parser system
 *
 * @param ptDecoder             Subtable demultiplexor to which the decoder is
 *                              attached.
 * @param table_id              Table Id, 0x42 or 0x46.
 * @param table_id_extension    Table Id Extension. Here is TS Id.
 * @param pfCallback            A callback function is called after the table
 *                              decoding complete.
 * @param pCallbackData         private data given in argument to the callback.
 * @return                      0 if everything went ok.
 */
//=============================================================================
int
psiTableSDT_AttachDecoder(
    PSI_DECODER*        ptDecoder,
    MMP_UINT32          table_id,
    MMP_UINT32          table_id_extension,
    PSI_SDT_CALLBACK    pfCallback,
    void*               pCallbackData)
{
    PSI_DEMUX*              ptDemux = MMP_NULL;
    PSI_DEMUX_SUBDECODER*   ptSubdecoder = MMP_NULL;
    PSI_SDT_DECODER*        ptSdtDecoder = MMP_NULL;

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

    ptSdtDecoder = (PSI_SDT_DECODER*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                  sizeof(PSI_SDT_DECODER));
    if (MMP_NULL == ptSdtDecoder)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptSubdecoder);
        return ATTACH_SUBDECODER_FAIL;
    }

    // Subtable decoder initialization
    ptSubdecoder->pfGatherSection   = &_SDT_GatherSection;
    ptSubdecoder->ptPrivateDecoder  = ptSdtDecoder;
    ptSubdecoder->id = (table_id << 16) | table_id_extension;
    ptSubdecoder->pfDetachSubdecoder = _SDT_DetachDecoder;

    // Attach the subtable decoder to the demux
    ptSubdecoder->ptNextSubdecoder = ptDemux->ptFirstSubdecoder;
    ptDemux->ptFirstSubdecoder = ptSubdecoder;

    // SDT decoder initialization
    PalMemset(ptSdtDecoder, 0x0, sizeof(PSI_SDT_DECODER));
    ptSdtDecoder->pfCallback    = pfCallback;
    ptSdtDecoder->pCallbackData = pCallbackData;
    ptSdtDecoder->allocId       = ptDecoder->allocId;
    ptSdtDecoder->pfAlloc       = ptDecoder->pfAlloc;
    ptSdtDecoder->pfFree        = ptDecoder->pfFree;

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
psiTableSDT_DestroyTable(
    PSI_SDT_INFO* ptSdtInfo)
{
    if (ptSdtInfo)
    {
        _SDT_DestroyService(ptSdtInfo);
        PalHeapFree(PAL_HEAP_DEFAULT, ptSdtInfo);
    }
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Detach/Remove the SDT decoder from the TS parser system and also free all
 * allocated memory
 *
 * @param ptDemux               Subtable demultiplexor to which the decoder is
 *                              attached.
 * @param table_id              Table Id, 0x42 or 0x46.
 * @param table_id_extension    Table Id extension, Here is TS Id.
 * @return none
 */
//=============================================================================
void
_SDT_DetachDecoder(
    PSI_DEMUX*  ptDemux,
    MMP_UINT32  table_id,
    MMP_UINT32  table_id_extension)
{
    PSI_DEMUX_SUBDECODER*   ptSubdecoder = MMP_NULL;
    PSI_DEMUX_SUBDECODER**  pptPrevSubdecoder = MMP_NULL;
    PSI_SDT_DECODER*        ptSdtDecoder = MMP_NULL;

    if (MMP_NULL != ptDemux)
    {
        ptSubdecoder = psiTableDemux_GetSubdecoder(ptDemux,
                                                   table_id,
                                                   table_id_extension);
        if (MMP_NULL == ptSubdecoder)
        {
            // No such SDT decoder
            return;
        }
    }
    else
        return;

    ptSdtDecoder = (PSI_SDT_DECODER*)ptSubdecoder->ptPrivateDecoder;
    if (ptSdtDecoder)
    {
        psiTableSDT_DestroyTable(ptSdtDecoder->ptBuildingSdt);
        ptSdtDecoder->ptBuildingSdt = MMP_NULL;

        SectionKit_DestroySection(ptSdtDecoder->allocId, ptSdtDecoder->pfFree, ptSdtDecoder->ptFirstSection);
        PalHeapFree(PAL_HEAP_DEFAULT, ptSdtDecoder);
    }

    pptPrevSubdecoder = &ptDemux->ptFirstSubdecoder;
    while (*pptPrevSubdecoder != ptSubdecoder)
        pptPrevSubdecoder = &(*pptPrevSubdecoder)->ptNextSubdecoder;

    *pptPrevSubdecoder = ptSubdecoder->ptNextSubdecoder;
    PalHeapFree(PAL_HEAP_DEFAULT, ptSubdecoder);
}

//=============================================================================
/**
 * Gather section data of the SDT. This is a callback function for the
 * subtable demultiplexor.
 *
 * @param ptDecoder         A PSI decoder to handle the TS packet decoding
 *                          issue
 * @param ptPrivateDecoder  Pointer to the SDT decoder structure
 * @param ptSection         The datagram of the current section
 * @return none
 */
//=============================================================================
static void
_SDT_GatherSection(
    PSI_DECODER* ptDecoder,
    void*        ptPrivateDecoder,
    PSI_SECTION* ptSection)
{
#ifdef USE_BITSTREAM_KIT
    BITSTREAM           tBitStream = { 0 };
#endif

    MMP_BOOL            bSectionAppend  = MMP_TRUE;
    MMP_BOOL            bTableReInit    = MMP_FALSE;
    MMP_BOOL            bTableComplete  = MMP_FALSE;

    PSI_SDT_DECODER*    ptSdtDecoder    = MMP_NULL;
    PSI_SECTION*        ptCurrentSection = MMP_NULL;

    if (ptDecoder && ptPrivateDecoder && ptSection)
    {
        ptSdtDecoder = (PSI_SDT_DECODER*)ptPrivateDecoder;
        ptCurrentSection = ptSection;
    }
    else
    {
        // Vincent noted on 9 april 2010:
        // added this to avoid memory leak but it means nothing
        // because we never reach here.
        if (ptSection)
            SectionKit_DestroySection(ptSdtDecoder->allocId, ptSdtDecoder->pfFree, ptSection);
        return;
    }

    // If the section_syntax_indicator != 1, then this section
    // is not a generic table.
    // On the other hand, it's not part of SDT
    if (ptCurrentSection->section_syntax_indicator != 1)
          bSectionAppend = MMP_FALSE;

    // If bSectionAppend is true then we have a valid SDT section
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
            // The building SDT is already existed. check the consistence of
            // the building table and incoming section.
            if (ptSdtDecoder->ptBuildingSdt)
            {
                // Any of the parameter comparsion is failed.
                // We need to reinited the table structure.
                if ((ptSdtDecoder->ptBuildingSdt->transport_stream_id
                    != ptCurrentSection->table_id_extension)
                 || (ptSdtDecoder->ptBuildingSdt->version_number
                    != ptCurrentSection->version_number)
                 || (ptSdtDecoder->last_section_number
                    != ptCurrentSection->last_section_number))
                {
                    bTableReInit = MMP_TRUE;
                }
            }
            /* [20100503] Vincent marked.
            else
            {
                // The last saved SDT table should be activated when the
                // current_next_indicator is 0 but the value of incoming
                // section is 1. Also, the version_number of the saved
                // SDT is equal to the version_number of the incoming
                // section. Usually, the version_number will be the number
                // that the previous version_number + 1. The mechanism is
                // used to provide the buffer time for decode side. The
                // benefit is obviously, that is, reduce the table re-build
                // time.
                if ((ptSdtDecoder->bCurrentSdtValid)
                 && (ptSdtDecoder->tCurrentSdt.version_number
                    == ptCurrentSection->version_number))
                {
                    // Notify the AP layer about that the new SDT should be
                    // activated.
                    if ((0 == ptSdtDecoder->tCurrentSdt.current_next_indicator)
                     && (1 == ptCurrentSection->current_next_indicator))
                    {
                        PSI_SDT_INFO* ptSdtInfo =
                            (PSI_SDT_INFO*)PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                        sizeof(PSI_SDT_INFO));

                        if (ptSdtInfo)
                        {
                            ptSdtDecoder->tCurrentSdt.current_next_indicator = 1;

                            PalMemcpy(ptSdtInfo,
                                      &ptSdtDecoder->tCurrentSdt,
                                      sizeof(PSI_SDT_INFO));

                            ptSdtDecoder->pfCallback(ptSdtDecoder->pCallbackData,
                                                     ptSdtInfo);
                        }
                    }
                    bSectionAppend = MMP_FALSE;
                }
            }
            */
        }
    }

    // Check whether the table should be re-inited.
    if (bTableReInit)
    {
        if (ptSdtDecoder->ptBuildingSdt)
        {
            psiTableSDT_DestroyTable(ptSdtDecoder->ptBuildingSdt);
            ptSdtDecoder->ptBuildingSdt = MMP_NULL;
        }

        // Delete all chained sections
        SectionKit_DestroySection(ptSdtDecoder->allocId, ptSdtDecoder->pfFree, ptSdtDecoder->ptFirstSection);
        ptSdtDecoder->ptFirstSection = MMP_NULL;
        ptSdtDecoder->totalSectionCount = 0;

        // Record the current pat to invalid
        //ptSdtDecoder->bCurrentSdtValid = MMP_FALSE; // [20100503] Vincent marked.
    }

    // Append the section into the Table. If sections can form a complete
    // table, then process decoding.
    if (bSectionAppend)
    {
        if (MMP_NULL == ptSdtDecoder->ptBuildingSdt)
        {
            ptSdtDecoder->ptBuildingSdt =
                (PSI_SDT_INFO*)PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(PSI_SDT_INFO));

            if (ptSdtDecoder->ptBuildingSdt)
            {
                PalMemset(ptSdtDecoder->ptBuildingSdt, 0x0, sizeof(PSI_SDT_INFO));

                // take over the parsed information of sdt
                ptSdtDecoder->ptBuildingSdt->transport_stream_id =
                    ptCurrentSection->table_id_extension;
                ptSdtDecoder->ptBuildingSdt->version_number =
                    ptCurrentSection->version_number;
                ptSdtDecoder->ptBuildingSdt->current_next_indicator =
                    ptCurrentSection->current_next_indicator;
                ptSdtDecoder->last_section_number =
                    ptCurrentSection->last_section_number;

#ifdef USE_BITSTREAM_KIT
                BitStreamKit_Init(&tBitStream, ptCurrentSection->pPayloadStartAddress);
                ptSdtDecoder->ptBuildingSdt->original_network_id =
                    BitStreamKit_GetBits(&tBitStream, 16);
#else
                ptSdtDecoder->ptBuildingSdt->original_network_id =
                    (MMP_UINT32) (ptCurrentSection->pPayloadStartAddress[0] << 8 |
                                  ptCurrentSection->pPayloadStartAddress[1]);
#endif
            }
        }

        // Insert the section into the section list (ptSdtDecoder->ptFirstSection).
        _SDT_InsertSection(ptSdtDecoder, ptCurrentSection);

        // Check if we have all the sections
        bTableComplete = MMP_FALSE;
        if ((ptSdtDecoder->totalSectionCount)
         == (ptSdtDecoder->last_section_number + 1))
        {
            bTableComplete = MMP_TRUE;
        }

        // error handle fot allocation fail
        if (MMP_NULL == ptSdtDecoder->ptBuildingSdt)
            return;

        // Time for SDT table decode
        if (MMP_TRUE == bTableComplete)
        {
            /* [20100503] Vincent marked.
            // Save the building SDT inform to currnet SDT including
            // version_number, current_next_indicator, and etc.
            PalMemcpy(&ptSdtDecoder->tCurrentSdt,
                      ptSdtDecoder->ptBuildingSdt,
                      sizeof(PSI_SDT_INFO));
            ptSdtDecoder->bCurrentSdtValid = MMP_TRUE;
            */

            // Decode the table
            _SDT_DecodeSection(ptSdtDecoder->ptBuildingSdt,
                               ptSdtDecoder->ptFirstSection);

            // Section information is stored in the building SDT table,
            // therefore, delete these sections.
            SectionKit_DestroySection(ptSdtDecoder->allocId, ptSdtDecoder->pfFree, ptSdtDecoder->ptFirstSection);

            // Callback to notify AP layer about the new Table constructed
            if (ptSdtDecoder->pfCallback)
            {
                ptSdtDecoder->pfCallback(ptSdtDecoder->pCallbackData,
                                         ptSdtDecoder->ptBuildingSdt);
            }

            // The AP will free the ptBuildingSdt. We just need to re-init
            // the decoder parameter.
            ptSdtDecoder->ptBuildingSdt = MMP_NULL;
            ptSdtDecoder->ptFirstSection = MMP_NULL;
            ptSdtDecoder->totalSectionCount = 0;
        }
    }
    else // Ignore the incoming section.
        SectionKit_DestroySection(ptSdtDecoder->allocId, ptSdtDecoder->pfFree, ptCurrentSection);
}

//=============================================================================
/**
 * Decode the sections and then construct a program list of SDT table.
 *
 * @param ptSdtInfo A SDT table keeps the sections decoded information.
 * @param ptSection A section list to form a complete SDT table.
 * @return none
 */
//=============================================================================
static void
_SDT_DecodeSection(
    PSI_SDT_INFO*   ptSdtInfo,
    PSI_SECTION*    ptSection)
{
#ifdef USE_BITSTREAM_KIT
    BITSTREAM tBitStream = { 0 };
#else
    MMP_UINT8* pCurrentAddr = MMP_NULL;
#endif
    MMP_UINT8* pEnd = MMP_NULL;

    MMP_UINT32 service_id                   = 0;
    MMP_UINT32 EIT_schedule_flag            = 0;
    MMP_UINT32 EIT_present_following_flag   = 0;
    MMP_UINT32 running_status               = 0;
    MMP_UINT32 free_CA_mode                 = 0;
    MMP_UINT32 descriptors_loop_length      = 0;
    PSI_SDT_SERVICE* ptService              = MMP_NULL;

    while (ptSection)
    {
#ifdef USE_BITSTREAM_KIT
        BitStreamKit_Init(&tBitStream,
            ptSection->pPayloadStartAddress
            + EXTRA_SECTION_HEADER_SIZE);

        // The loop is used to build up the information of SDT.
        while ((tBitStream.pStartAddress + CRC32_FIELD_SIZE) <= ptSection->pPayloadEndAddress)
        {
            service_id                   = BitStreamKit_GetBits(&tBitStream, 16);
            BitStreamKit_SkipBits(&tBitStream, 6);
            EIT_schedule_flag            = BitStreamKit_GetBits(&tBitStream, 1);
            EIT_present_following_flag   = BitStreamKit_GetBits(&tBitStream, 1);
            running_status               = BitStreamKit_GetBits(&tBitStream, 3);
            free_CA_mode                 = BitStreamKit_GetBits(&tBitStream, 1);
            descriptors_loop_length      = BitStreamKit_GetBits(&tBitStream, 12);

            ptService = _SDT_AddService(ptSdtInfo,
                                        service_id,
                                        EIT_schedule_flag,
                                        EIT_present_following_flag,
                                        running_status,
                                        free_CA_mode);
            // error handle for allocation fail
            if (MMP_NULL == ptService)
                return;

            pEnd = tBitStream.pStartAddress + descriptors_loop_length;
            if (pEnd > ptSection->pPayloadEndAddress)
                break;

            while (tBitStream.pStartAddress + DESCRIPTOR_PRIOR_HEADER_SIZE <= pEnd)
            {
                MMP_UINT32 descriptor_tag       = BitStreamKit_GetBits(&tBitStream, 8);
                MMP_UINT32 descriptor_length    = BitStreamKit_GetBits(&tBitStream, 8);
                if (tBitStream.pStartAddress + descriptor_length <= pEnd)
                    _SDT_ServiceAddDescriptor(ptService,
                                              descriptor_tag,
                                              descriptor_length,
                                              tBitStream.pStartAddress);
                tBitStream.pStartAddress += descriptor_length;
            }
        }
#else
        pCurrentAddr = ptSection->pPayloadStartAddress + EXTRA_SECTION_HEADER_SIZE;

        // The loop is used to build up the information of SDT.
        while ((pCurrentAddr + CRC32_FIELD_SIZE) <= ptSection->pPayloadEndAddress)
        {
            service_id                   = (MMP_UINT32) (pCurrentAddr[0] << 8 | pCurrentAddr[1]);
            EIT_schedule_flag            = (pCurrentAddr[2] & 0x02) >> 1;
            EIT_present_following_flag   = (pCurrentAddr[2] & 0x01);
            running_status               = (pCurrentAddr[3] & 0xE0) >> 5;
            free_CA_mode                 = (pCurrentAddr[3] & 0x10) >> 4;
            descriptors_loop_length      = (MMP_UINT32) ((pCurrentAddr[3] & 0x0F) << 8 |
                                                          pCurrentAddr[4]);
            pCurrentAddr += 5;
            ptService = _SDT_AddService(ptSdtInfo,
                                        service_id,
                                        EIT_schedule_flag,
                                        EIT_present_following_flag,
                                        running_status,
                                        free_CA_mode);
            // error handle for allocation fail
            if (MMP_NULL == ptService)
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
                    _SDT_ServiceAddDescriptor(ptService,
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
 * Destroy the service list of SDT table
 *
 * @param ptSdtInfo A SDT table keeps the sections decoded information.
 * @return none
 */
//=============================================================================
static void
_SDT_DestroyService(
    PSI_SDT_INFO* ptSdtInfo)
{
    if (ptSdtInfo)
    {
        PSI_SDT_SERVICE* ptService = ptSdtInfo->ptFirstService;
        while (MMP_NULL != ptService)
        {
            PSI_SDT_SERVICE* ptNextService = ptService->ptNextService;
            DescriptorKit_DestroyDescriptor(ptService->ptFirstDescriptor);
            PalHeapFree(PAL_HEAP_DEFAULT, ptService);
            ptService = ptNextService;
        }
        ptSdtInfo->ptFirstService = MMP_NULL;
    }
}

//=============================================================================
/**
 * Insert a section into the section list of SDT decoder
 *
 * @param ptSdtDecoder      The private Decoder to handle the SDT decode.
 * @param ptInsertSection   A section that we want to insert into section list.
 * @return none
 */
//=============================================================================
static void
_SDT_InsertSection(
    PSI_SDT_DECODER*    ptSdtDecoder,
    PSI_SECTION*        ptInsertSection)
{
    PSI_SECTION* ptCurrentSection = MMP_NULL;
    PSI_SECTION* ptPreviousSection = MMP_NULL;

    if (MMP_NULL == ptInsertSection || MMP_NULL == ptSdtDecoder)
        return;

    if (ptSdtDecoder->ptFirstSection)
    {
        ptCurrentSection = ptSdtDecoder->ptFirstSection;

        // search all current section link list to insert at the appropriated position
        do
        {
            if (ptCurrentSection->section_number < ptInsertSection->section_number)
            {
                if (MMP_NULL != ptCurrentSection->ptNextSection)
                {
                    ptPreviousSection = ptCurrentSection;
                    ptCurrentSection = ptCurrentSection->ptNextSection;
                }
                else
                {
                    // append to the end of section link list
                    ptCurrentSection->ptNextSection = ptInsertSection;
                    ptSdtDecoder->totalSectionCount++;
                    break;
                }
            }
            else if (ptCurrentSection->section_number > ptInsertSection->section_number)
            {
                // insert the new coming section into section link list
                if (MMP_NULL != ptPreviousSection)
                {
                    ptPreviousSection->ptNextSection = ptInsertSection;
                }
                else
                {
                    ptSdtDecoder->ptFirstSection = ptInsertSection;
                }

                ptInsertSection->ptNextSection = ptCurrentSection;
                ptSdtDecoder->totalSectionCount++;
                break;
            }
            else // find the same section (number), replace with the new coming one
            {
                // Section duplication. Replace and free the old section.
                if (MMP_NULL != ptPreviousSection)
                {
                    ptPreviousSection->ptNextSection = ptInsertSection;
                }
                else
                {
                    ptSdtDecoder->ptFirstSection = ptInsertSection;
                }
                ptInsertSection->ptNextSection = ptCurrentSection->ptNextSection;

                ptCurrentSection->ptNextSection = MMP_NULL;
                SectionKit_DestroySection(ptSdtDecoder->allocId, ptSdtDecoder->pfFree, ptCurrentSection);

                break;
            }
        } while (ptCurrentSection);
    }
    else // The section is the first incoming section
    {
        ptSdtDecoder->ptFirstSection = ptInsertSection;
        ptSdtDecoder->totalSectionCount = 1;
    }
}

//=============================================================================
/**
 * Add a service description at the end of the SDT
 *
 * @param ptSdtInfo                     A SDT table keeps the sections decoded
 *                                      information.
 * @param service_id                    Service ID.
 * @param EIT_schedule_flag             EIT schedule flag
 * @param EIT_present_following_flag    EIT present following flag
 * @param running_status                running_status
 * @param free_CA_mode                  free_CA_mode
 * @return Pointer to the latest inserted service description
 */
//=============================================================================
static PSI_SDT_SERVICE*
_SDT_AddService(
    PSI_SDT_INFO*   ptSdtInfo,
    MMP_UINT32      service_id,
    MMP_UINT32      EIT_schedule_flag,
    MMP_UINT32      EIT_present_following_flag,
    MMP_UINT32      running_status,
    MMP_UINT32      free_CA_mode)
{
    PSI_SDT_SERVICE* ptService = PalHeapAlloc(PAL_HEAP_DEFAULT,
                                              sizeof(PSI_SDT_SERVICE));
    if (MMP_NULL != ptService)
    {
        ptService->service_id                   = service_id;
        ptService->EIT_schedule_flag            = EIT_schedule_flag;
        ptService->EIT_present_following_flag   = EIT_present_following_flag;
        ptService->running_status               = running_status;
        ptService->free_CA_mode                 = free_CA_mode;
        ptService->ptNextService                = MMP_NULL;
        ptService->ptFirstDescriptor            = MMP_NULL;

        if (MMP_NULL == ptSdtInfo->ptFirstService)
        {
            ptSdtInfo->ptFirstService = ptService;
        }
        else
        {
            PSI_SDT_SERVICE* ptLastService = ptSdtInfo->ptFirstService;
            while (MMP_NULL != ptLastService->ptNextService)
            {
                ptLastService = ptLastService->ptNextService;
            }
            ptLastService->ptNextService = ptService;
        }
    }

    return ptService;
}

//=============================================================================
/**
 * Add a descriptor in the SDT service description
 *
 * @param ptService Pointer to the SDT service description
 * @param tag       descriptor tag.
 * @param length    descriptor length
 * @param pData     descriptor data
 * @return Pointer to the lastest inserted descriptor
 */
//=============================================================================

static PSI_DESCRIPTOR*
_SDT_ServiceAddDescriptor(
    PSI_SDT_SERVICE* ptService,
    MMP_UINT32 tag,
    MMP_UINT32 length,
    MMP_UINT8* pData)
{
    PSI_DESCRIPTOR* ptDescriptor =
        DescriptorKit_CreateDescriptor(tag, length, pData);

    if (MMP_NULL != ptDescriptor)
    {
        if (MMP_NULL == ptService->ptFirstDescriptor)
        {
            ptService->ptFirstDescriptor = ptDescriptor;
        }
        else
        {
            PSI_DESCRIPTOR* ptLastDescriptor = ptService->ptFirstDescriptor;
            while (MMP_NULL != ptLastDescriptor->ptNextDescriptor)
            {
                ptLastDescriptor = ptLastDescriptor->ptNextDescriptor;
            }
            ptLastDescriptor->ptNextDescriptor = ptDescriptor;
        }
    }

    return ptDescriptor;
}
