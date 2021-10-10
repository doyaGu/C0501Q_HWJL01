/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file psi_table_pmt.c
 * Use to decode the PMT table of TS packets
 * @author Steven Hsiao
 * @version 0.1
 */

#ifdef USE_BITSTREAM_KIT
#include "bitstream_kit.h"
#endif
#include "psi_table_pmt.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define MAX_SECTION_SIZE_OF_PMT     (1024)

#define PMT_TABLE_ID                (0x02)

#define INVALID_DESCRIPTOR_TAG      (0xFF)
#define INVALID_DESCRIPTOR_LENGTH   (255)
#define INVALID_ELEMENTARY_PID      (0x1FFF)

// The offset is from the PMT section start.
#define PCR_PID_OFFSET              (67)
#define PROGRAM_INFO_LENGTH_OFFSET  (84)

// The byte offset of the first program_info descriptor from the PMT section
// start.
#define PROGRAM_INFO_BYTE_OFFSET    (12)

// The header bytes of descriptor is descriptor_tag(8) + descriptor_length(8)
// = 2 bytes
#define DESCRIPTOR_HEADER_SIZE      (2)

// The header bytes of ES is stream_type(8) + reserved(3) + elementary_PID(13)
// + reserved(4) + ES_info_length(12) = 5 bytes
#define ES_INFO_HEADER_SIZE         (5)

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

// A private decoder to deal with the decoding issue of PMT table.
typedef struct PSI_PMT_DECODER_TAG
{
    MMP_UINT32              program_number;

    // Save the latest PSI_PMT_INFO, turn in it if no update.
    //PSI_PMT_INFO        tCurrentPmt; // [20100503] Vincent marked.

    // Processing PSI_PMT_INFO, parse a PMT (completed sections) to fill with
    // it.
    PSI_PMT_INFO*           ptBuildingPmt;
    //MMP_BOOL            bCurrentPmtValid; // [20100503] Vincent marked.
    MMP_UINT32              last_section_number;

    MMP_UINT32              totalSectionCount;
    PSI_SECTION*            ptFirstSection;
    PSI_PMT_CALLBACK        pfCallback;
    void*                   pCallbackData;

    MMP_INT                 allocId;
    SECTION_PAYLOAD_ALLOC   pfAlloc;
    SECTION_PAYLOAD_FREE    pfFree;
} PSI_PMT_DECODER;

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
_PMT_AddDescriptor(
    PSI_PMT_INFO* ptPmtInfo,
    MMP_UINT32 tag,
    MMP_UINT32 length,
    MMP_UINT8* pData);

static void
_PMT_EsAddDescriptor(
    PSI_PMT_ES_INFO* ptEsInfo,
    MMP_UINT32 tag,
    MMP_UINT32 length,
    MMP_UINT8* pData);

static PSI_PMT_ES_INFO*
_PMT_AddEsInfo(
    PSI_PMT_INFO*   ptPmtInfo,
    MMP_UINT32      stream_type,
    MMP_UINT32      elementary_PID);

static void
_PMT_GatherSection(
    PSI_DECODER* ptDecoder,
    PSI_SECTION* ptSection);

static void
_PMT_DecodeSection(
    PSI_PMT_INFO*   ptPmtInfo,
    PSI_SECTION*    ptSection);

static void
_PMT_InsertSection(
    PSI_PMT_DECODER*    ptPmtDecoder,
    PSI_SECTION*        ptInsertSection);

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Attach the PMT decoder on the TS parser system.
 *
 * @param program_number    The program_number is used to identify different PMT
 * @param pfCallback        A callback function is called after the table
 *                          decoding complete.
 * @param pCallbackData     The datagram of the callback function
 * @return                  PSI_DECODER* to handle psi section parsing and
 *                          data collecion of PMT of incoming TS packets.
 */
//=============================================================================
PSI_DECODER*
psiTablePMT_AttachDecoder(
    MMP_UINT32          program_number,
    PSI_PMT_CALLBACK    pfCallback,
    void*               pCallbackData)
{
    PSI_DECODER*        ptDecoder = MMP_NULL;
    PSI_PMT_DECODER*    ptPmtDecoder = MMP_NULL;

    ptDecoder = (PSI_DECODER*) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                            sizeof(PSI_DECODER));
    if (MMP_NULL == ptDecoder)
        return MMP_NULL;

    ptPmtDecoder = (PSI_PMT_DECODER*) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                   sizeof(PSI_PMT_DECODER));
    if (MMP_NULL == ptPmtDecoder)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptDecoder);
        return MMP_NULL;
    }

    // PSI decoder initilization
    PalMemset(ptDecoder, 0x0, sizeof(PSI_DECODER));
    ptDecoder->pfCallback       = (PSI_DECODER_CALLBACK) _PMT_GatherSection;
    ptDecoder->ptPrivateDecoder = (void*) ptPmtDecoder;
    ptDecoder->sectionMaxSize   = MAX_SECTION_SIZE_OF_PMT;
    ptDecoder->bDiscontinuity   = MMP_TRUE;
    ptDecoder->allocId          = PAL_HEAP_DEFAULT;
    ptDecoder->pfAlloc          = _HeapAlloc; // PalHeapAlloc;
    ptDecoder->pfFree           = _HeapFree;  // PalHeapFree;

    // PMT decoder initilization
    PalMemset(ptPmtDecoder, 0x0, sizeof(PSI_PMT_DECODER));
    ptPmtDecoder->program_number    = program_number;
    ptPmtDecoder->pfCallback        = pfCallback;
    ptPmtDecoder->pCallbackData     = pCallbackData;
    ptPmtDecoder->allocId           = ptDecoder->allocId;
    ptPmtDecoder->pfAlloc           = ptDecoder->pfAlloc;
    ptPmtDecoder->pfFree            = ptDecoder->pfFree;

    return ptDecoder;
}

//=============================================================================
/**
 * Detach the PMT decoder from the TS parser system and also free all
 * allocated memory.
 *
 * @param ptDecoder The existed decoder to handle PMT decode.
 * @return none
 */
//=============================================================================
void
psiTablePMT_DetachDecoder(
    PSI_DECODER* ptDecoder)
{
    PSI_PMT_DECODER*    ptPmtDecoder = MMP_NULL;
    PSI_PMT_ES_INFO*    ptTempEsInfo = MMP_NULL;
    PSI_PMT_ES_INFO*    ptTempNextEsInfo = MMP_NULL;

    if (ptDecoder)
        ptPmtDecoder = ptDecoder->ptPrivateDecoder;
    else
        return;

    if (ptPmtDecoder)
    {
        psiTablePMT_DestroyTable(ptPmtDecoder->ptBuildingPmt);
        ptPmtDecoder->ptBuildingPmt = MMP_NULL;

        SectionKit_DestroySection(ptPmtDecoder->allocId, ptPmtDecoder->pfFree, ptPmtDecoder->ptFirstSection);
        PalHeapFree(PAL_HEAP_DEFAULT, ptPmtDecoder);
    }

    if (ptDecoder->ptCurrentSection)
    {
        SectionKit_DestroySection(ptDecoder->allocId, ptDecoder->pfFree, ptDecoder->ptCurrentSection);
    }

    PalHeapFree(PAL_HEAP_DEFAULT, ptDecoder);
    ptDecoder = MMP_NULL;
}

//=============================================================================
/**
 * Destroy the PMT table and its allocated memory.
 *
 * @param ptPmtInfo A PMT table keeps the sections decoded information.
 * @return none
 */
//=============================================================================
void
psiTablePMT_DestroyTable(
    PSI_PMT_INFO* ptPmtInfo)
{
    PSI_PMT_ES_INFO*    ptCurrentEsInfo = MMP_NULL;
    PSI_PMT_ES_INFO*    ptTmepEsInfo = MMP_NULL;

    if (ptPmtInfo)
    {
        if (ptPmtInfo->ptFirstDescriptor)
            DescriptorKit_DestroyDescriptor(ptPmtInfo->ptFirstDescriptor);

        ptCurrentEsInfo = ptPmtInfo->ptFirstEsInfo;
        while (ptCurrentEsInfo)
        {
            ptTmepEsInfo = ptCurrentEsInfo->ptNexEsInfo;
            if (ptCurrentEsInfo->ptFirstDescriptor)
            {
                DescriptorKit_DestroyDescriptor(
                    ptCurrentEsInfo->ptFirstDescriptor);
            }
            PalHeapFree(PAL_HEAP_DEFAULT, ptCurrentEsInfo);
            ptCurrentEsInfo = ptTmepEsInfo;
        }

        PalHeapFree(PAL_HEAP_DEFAULT, ptPmtInfo);
    }
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Add a new descriptor in the first descriptor list (program descriptor list).
 * @param ptPmtInfo A PMT info structure to keep the PMT table information.
 * @param tag       A unique descriptor tag to identify the descriptor.
 * @param length    The descriptor content data size behind the field.
 *                  descriptor_length.
 * @param pData     The descriptor content data.
 * @return none
 */
//=============================================================================
static void
_PMT_AddDescriptor(
    PSI_PMT_INFO*   ptPmtInfo,
    MMP_UINT32      tag,
    MMP_UINT32      length,
    MMP_UINT8*      pData)
{
    PSI_DESCRIPTOR*     ptDescriptor = MMP_NULL;
    PSI_DESCRIPTOR*     ptLastDescriptor = MMP_NULL;

    // Invalid input
    if ((MMP_NULL == ptPmtInfo)
     || (MMP_NULL == pData)
     || (tag >= INVALID_DESCRIPTOR_TAG)
     || (length >= INVALID_DESCRIPTOR_LENGTH))
    {
        return;
    }

    ptDescriptor = DescriptorKit_CreateDescriptor(tag, length, pData);
    if (ptDescriptor)
    {
        if (MMP_NULL == ptPmtInfo->ptFirstDescriptor)
            ptPmtInfo->ptFirstDescriptor = ptDescriptor;
        else
        {
            ptLastDescriptor = ptPmtInfo->ptFirstDescriptor;
            while (ptLastDescriptor->ptNextDescriptor)
                ptLastDescriptor = ptLastDescriptor->ptNextDescriptor;
            ptLastDescriptor->ptNextDescriptor = ptDescriptor;
            ptLastDescriptor = ptDescriptor;
        }
    }
    else
        return;
}

//=============================================================================
/**
 * Add a new descriptor in the second descriptor list (ES_Info descriptor list).
 * @param ptEsInfo  The structure to keep the information of a elementary stream
 * @param tag       A unique descriptor tag to identify the descriptor.
 * @param length    The descriptor content data size behind the field.
 *                  descriptor_length.
 * @param pData     The descriptor content data.
 * @no return value.
 */
//=============================================================================
static void
_PMT_EsAddDescriptor(
    PSI_PMT_ES_INFO*    ptEsInfo,
    MMP_UINT32          tag,
    MMP_UINT32          length,
    MMP_UINT8*          pData)
{
    PSI_DESCRIPTOR*     ptDescriptor = MMP_NULL;
    PSI_DESCRIPTOR*     ptLastDescriptor = MMP_NULL;

    // Invalid input
    if ((MMP_NULL == ptEsInfo)
     || (MMP_NULL == pData)
     || (tag >= INVALID_DESCRIPTOR_TAG)
     || (length >= INVALID_DESCRIPTOR_LENGTH))
    {
        return;
    }

    ptDescriptor = DescriptorKit_CreateDescriptor(tag, length, pData);
    if (ptDescriptor)
    {
        if (MMP_NULL == ptEsInfo->ptFirstDescriptor)
            ptEsInfo->ptFirstDescriptor = ptDescriptor;
        else
        {
            ptLastDescriptor = ptEsInfo->ptFirstDescriptor;
            while (ptLastDescriptor->ptNextDescriptor)
                ptLastDescriptor = ptLastDescriptor->ptNextDescriptor;
            ptLastDescriptor->ptNextDescriptor = ptDescriptor;
            ptLastDescriptor = ptDescriptor;
        }
    }
    else
        return;
}

//=============================================================================
/**
 * Add a new ES_Info into the PMT table
 *
 * @param ptPmtInfo         A PMT info structure to keep the PMT table
 *                          information.
 * @param stream_type       A stream type to announce the type of the stream.
 *                          Please see H222.0 p49 Table 2-29 to get further
 *                          details.
 * @param elementary_PID    The unique PID of the elementary stream.
 * @return                  PSI_PMT_ES_INFO* is the new added ES_Info structure.
 */
//=============================================================================
static PSI_PMT_ES_INFO*
_PMT_AddEsInfo(
    PSI_PMT_INFO*   ptPmtInfo,
    MMP_UINT32      stream_type,
    MMP_UINT32      elementary_PID)
{
    PSI_PMT_ES_INFO*    ptEsInfo = MMP_NULL;
    PSI_PMT_ES_INFO*    ptLastEsInfo = MMP_NULL;

    // Invalid input
    if (MMP_NULL  == ptPmtInfo || elementary_PID >= INVALID_ELEMENTARY_PID)
        return MMP_NULL;

    ptEsInfo = (PSI_PMT_ES_INFO*) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                               sizeof(PSI_PMT_ES_INFO));

    if (ptEsInfo)
    {
        PalMemset(ptEsInfo, 0x0, sizeof(PSI_PMT_ES_INFO));
        ptEsInfo->elementary_PID = elementary_PID;
        ptEsInfo->stream_type = stream_type;

        if (MMP_NULL == ptPmtInfo->ptFirstEsInfo)
        {
            ptPmtInfo->ptFirstEsInfo = ptEsInfo;
            ptPmtInfo->totalEsCount  = 1;
        }
        else
        {
            ptLastEsInfo = ptPmtInfo->ptFirstEsInfo;
            while (ptLastEsInfo->ptNexEsInfo)
                ptLastEsInfo = ptLastEsInfo->ptNexEsInfo;
            ptLastEsInfo->ptNexEsInfo = ptEsInfo;
            ptLastEsInfo = ptEsInfo;
            ptPmtInfo->totalEsCount++;
        }        
    }

    return ptEsInfo;
}

//=============================================================================
/**
 * Gather section data of the PMT. Once the whole sections data are completed,
 * call decoder to parse the PMT table
 *
 * @param ptDecoder A PSI decoder to handle the TS packet decoding issue
 * @param ptSection The datagram of the current section
 * @return none
 */
//=============================================================================
static void
_PMT_GatherSection(
    PSI_DECODER* ptDecoder,
    PSI_SECTION* ptSection)
{
#ifdef USE_BITSTREAM_KIT
    BITSTREAM           tBitStream = { 0 };
#endif
    MMP_BOOL            bSectionAppend  = MMP_TRUE;
    MMP_BOOL            bTableReInit    = MMP_FALSE;
    MMP_BOOL            bTableComplete  = MMP_FALSE;

    PSI_SECTION*        ptCurrentSection = MMP_NULL;
    PSI_PMT_DECODER*    ptPmtDecoder = MMP_NULL;
    PSI_PMT_INFO*       ptPmtInfo = MMP_NULL;

    // 0. Ensure the pointer ptDecoder and ptSection are valid
    if (ptDecoder && ptDecoder->ptPrivateDecoder && ptSection)
    {
        ptPmtDecoder = (PSI_PMT_DECODER*) ptDecoder->ptPrivateDecoder;
        ptCurrentSection = ptSection;
    }
    else
    {
        // Vincent noted on 9 april 2010:
        // added this to avoid memory leak but it means nothing
        // because we never reach here.
        if (ptSection)
            SectionKit_DestroySection(ptPmtDecoder->allocId, ptPmtDecoder->pfFree, ptSection);
        return;
    }

    // 1. Table Id validation. Not PID == 0x02, ignore the section.
    if (ptCurrentSection->table_id != PMT_TABLE_ID)
        bSectionAppend = MMP_FALSE;

    // 2. If the section_syntax_indicator != 1, then this section
    //    is not a generic table.
    //    On the other hand, it's not part of PAT
    if (ptCurrentSection->section_syntax_indicator != 1)
          bSectionAppend = MMP_FALSE;

    // 3. Do some check if the section should be appened to other
    //    sections to form a complete table.
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
            // The building pat is already existed. check the consistence of
            // the building table and incoming section.
            if (ptPmtDecoder->ptBuildingPmt)
            {
                // Any of the parameter comparsion is failed.
                // We need to inited the table structure later.
                if ((ptPmtDecoder->ptBuildingPmt->version_number
                    != ptCurrentSection->version_number)
                 || (ptPmtDecoder->last_section_number
                    != ptCurrentSection->last_section_number))
                {
                    bTableReInit = MMP_TRUE;
                }
            }
            /* [20100503] Vincent marked.
            else
            {
                // The last saved PMT table should be activated when the
                // current_next_indicator is 0 but the value of incoming
                // section is 1. Also, the version_number of the saved
                // PAT is equal to the version_number of the incoming
                // section. Usually, the version_number will be the number
                // that the previous version_number + 1. The mechanism is
                // used to provide the buffer time for decode side. The
                // benefit is obviously, that is, reduce the table re-build
                // time.
                if ((ptPmtDecoder->bCurrentPmtValid)
                 && (ptPmtDecoder->tCurrentPmt.version_number
                    == ptCurrentSection->version_number))
                {
                    // Notify the AP layer about that the new PMT should be
                    // activated.
                    if ((0 == ptPmtDecoder->tCurrentPmt.current_next_indicator)
                     && (1 == ptCurrentSection->current_next_indicator))
                    {

                        ptPmtInfo = (PSI_PMT_INFO*) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                                 sizeof(PSI_PMT_INFO));

                        if (ptPmtInfo)
                        {
                            ptPmtDecoder->tCurrentPmt.current_next_indicator = 1;

                            PalMemcpy(ptPmtInfo,
                                      &ptPmtDecoder->tCurrentPmt,
                                      sizeof(PSI_PMT_INFO));

                            ptPmtDecoder->pfCallback(ptPmtDecoder->pCallbackData,
                                                     ptPmtInfo);
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

    // 4. Check whether the table should be re-inited.
    if (bTableReInit)
    {
        if (ptPmtDecoder->ptBuildingPmt)
        {
            psiTablePMT_DestroyTable(ptPmtDecoder->ptBuildingPmt);
            ptPmtDecoder->ptBuildingPmt = MMP_NULL;

            // Delete all chained sections
            SectionKit_DestroySection(ptPmtDecoder->allocId, ptPmtDecoder->pfFree, ptPmtDecoder->ptFirstSection);
            ptPmtDecoder->ptFirstSection = MMP_NULL;
            ptPmtDecoder->totalSectionCount = 0;
        }
        // Record the current pat to invalid
        //ptPmtDecoder->bCurrentPmtValid = MMP_FALSE; // [20100503] Vincent marked.
    }

    // 5. Append the section into the Table. If sections can form a complete
    //    table, then process decoding.
    if (bSectionAppend)
    {
        if (MMP_NULL == ptPmtDecoder->ptBuildingPmt)
        {
            ptPmtDecoder->ptBuildingPmt =
                (PSI_PMT_INFO*) PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(PSI_PMT_INFO));

            if (ptPmtDecoder->ptBuildingPmt)
            {
                PalMemset(ptPmtDecoder->ptBuildingPmt, 0x0, sizeof(PSI_PMT_INFO));
                
                // take over the parsed information of pmt
                ptPmtDecoder->ptBuildingPmt->program_number =
                    ptCurrentSection->table_id_extension;
                ptPmtDecoder->ptBuildingPmt->current_next_indicator =
                    ptCurrentSection->current_next_indicator;
                ptPmtDecoder->ptBuildingPmt->version_number =
                    ptCurrentSection->version_number;
                ptPmtDecoder->last_section_number =
                    ptCurrentSection->last_section_number;

#ifdef USE_BIT_STREAM_KIT
                BitStreamKit_Init(&tBitStream, ptCurrentSection->pData);

                // Get the PCR_PID from the current section.
                ptPmtDecoder->ptBuildingPmt->PCR_PID =
                    BitStreamKit_ShowBits(&tBitStream, PCR_PID_OFFSET, 13);
#else
                ptPmtDecoder->ptBuildingPmt->PCR_PID =
                    (MMP_UINT32) ((ptCurrentSection->pData[8] & 0x1F) << 8 |
                                   ptCurrentSection->pData[9]);
#endif
            }
        }

        // Insert the section into the section list
        _PMT_InsertSection(ptPmtDecoder, ptCurrentSection);

        bTableComplete = MMP_FALSE;
        if ((ptPmtDecoder->totalSectionCount)
         == (ptPmtDecoder->last_section_number + 1))
        {
            bTableComplete = MMP_TRUE;
        }

        // error handle fot allocation fail
        if (MMP_NULL == ptPmtDecoder->ptBuildingPmt)
            return;

        // Time for PMT table decode
        if (MMP_TRUE == bTableComplete)
        {
            /* [20100503] Vincent marked.
            // Save the build PMT inform to currnet PMT including
            // version_number, current_next_indicator, and etc.
            PalMemcpy(&ptPmtDecoder->tCurrentPmt,
                      ptPmtDecoder->ptBuildingPmt,
                      sizeof(PSI_PMT_INFO));
            ptPmtDecoder->bCurrentPmtValid = MMP_TRUE;
            */

            // Decode the table
            _PMT_DecodeSection(ptPmtDecoder->ptBuildingPmt,
                               ptPmtDecoder->ptFirstSection);

            // Section information is stored in the building PAT table,
            // therefore, delete these sections.
            SectionKit_DestroySection(ptPmtDecoder->allocId, ptPmtDecoder->pfFree, ptPmtDecoder->ptFirstSection);

            // Callback to notify AP layer about the new Table constructed
            if (ptPmtDecoder->pfCallback)
            {
                ptPmtDecoder->pfCallback(ptPmtDecoder->pCallbackData,
                                         ptPmtDecoder->ptBuildingPmt);
            }

            // The AP will free the ptBuildingPmt. We just need to re-init
            // the decoder parameter.
            ptPmtDecoder->ptBuildingPmt = MMP_NULL;
            ptPmtDecoder->ptFirstSection = MMP_NULL;
            ptPmtDecoder->totalSectionCount = 0;
        }
    }
    else // Ignore the incoming section.
        SectionKit_DestroySection(ptPmtDecoder->allocId, ptPmtDecoder->pfFree, ptCurrentSection);
}

//=============================================================================
/**
 * Decode the sections and then construct PMT table.
 *
 * @param ptPmtInfo A PMT table keeps the sections decoded information.
 * @param ptSection A section list to form a complete PMT table.
 * @return none
 */
//=============================================================================
static void
_PMT_DecodeSection(
    PSI_PMT_INFO*   ptPmtInfo,
    PSI_SECTION*    ptSection)
{
#ifdef USE_BITSTREAM_KIT
    BITSTREAM           tBitStream = { 0 };
    BITSTREAM           tDescriptorBitStream = { 0 };
    BITSTREAM           tEsBitStream = { 0 };
#else
    MMP_UINT8*          pBitCurrentAddr = MMP_NULL;
    MMP_UINT8*          pBitDescriptorAddr = MMP_NULL;
    MMP_UINT8*          pBitEsAddr = MMP_NULL;
#endif
    PSI_PMT_ES_INFO*    ptEsInfo = MMP_NULL;

    MMP_UINT8*          pDescriptorStartAddress;
    MMP_UINT8*          pEsStartAddress;

    MMP_UINT32          descriptor_tag;
    MMP_UINT32          descriptor_length;
    MMP_UINT32          stream_type;
    MMP_UINT32          elementary_PID;
    MMP_UINT32          program_info_length;
    MMP_UINT32          ES_info_length;

    MMP_INT32           loopLength;
    MMP_INT32           restSectionSize;

    while (ptSection)
    {
        restSectionSize = ptSection->pPayloadEndAddress - ptSection->pData;
#ifdef USE_BITSTREAM_KIT
        BitStreamKit_Init(&tBitStream,
                        ptSection->pData);

        // Due to other information such as table_id, program_number,
        // PCR_PID, and etc. are extracted already. Therefore, we don't
        // need to retrieve those data again. So, we just need to read
        // the value of program_info_length, and then jump the address
        // pointer to the start of the first program descriptor.
        program_info_length =
            BitStreamKit_ShowBits(&tBitStream, PROGRAM_INFO_LENGTH_OFFSET, 12);
#else
        pBitCurrentAddr = ptSection->pData;
        program_info_length = (MMP_UINT32) ((pBitCurrentAddr[10] & 0x0F) << 8 |
                                             pBitCurrentAddr[11]);
#endif
        // Get the start address of the first program descriptor
        pDescriptorStartAddress = ptSection->pData + PROGRAM_INFO_BYTE_OFFSET;

        // The length of the first loop is determined by program_info_length
        loopLength = program_info_length;

        // The first loop of PMT section. See H222.0 p48 to get further details.
        while (loopLength > 0)
        {
#ifdef USE_BITSTREAM_KIT
            BitStreamKit_Init(&tDescriptorBitStream, pDescriptorStartAddress);

            descriptor_tag =
                BitStreamKit_GetBits(&tDescriptorBitStream, 8);

            descriptor_length =
                BitStreamKit_GetBits(&tDescriptorBitStream, 8);

            loopLength -= (DESCRIPTOR_HEADER_SIZE + descriptor_length);

            if (loopLength >= 0)
            {
                _PMT_AddDescriptor(ptPmtInfo,
                                   descriptor_tag,
                                   descriptor_length,
                                   tDescriptorBitStream.pStartAddress);

                // Address jump to the start address of next descriptor
                pDescriptorStartAddress += (DESCRIPTOR_HEADER_SIZE
                                          + descriptor_length);
            }
#else
            pBitDescriptorAddr = pDescriptorStartAddress;

            descriptor_tag = pBitDescriptorAddr[0];

            descriptor_length = pBitDescriptorAddr[1];

            loopLength -= (DESCRIPTOR_HEADER_SIZE + descriptor_length);

            if (loopLength >= 0)
            {
                _PMT_AddDescriptor(ptPmtInfo,
                                   descriptor_tag,
                                   descriptor_length,
                                   &pBitDescriptorAddr[2]);

                // Address jump to the start address of next descriptor
                pDescriptorStartAddress += (DESCRIPTOR_HEADER_SIZE
                                          + descriptor_length);
            }
#endif
        }

        restSectionSize -= (PROGRAM_INFO_BYTE_OFFSET + program_info_length);

        // Address jump to the start address of the second loop of PMT section.
        // See H222.0 p48 to get further details.
        pEsStartAddress = (ptSection->pData
                         + PROGRAM_INFO_BYTE_OFFSET
                         + program_info_length);

        // The second loop of PMT section. See H222.0 p48 to get further details.
        while (restSectionSize > 0)
        {
#ifdef USE_BITSTREAM_KIT
            BitStreamKit_Init(&tEsBitStream, pEsStartAddress);

            stream_type =
                BitStreamKit_GetBits(&tEsBitStream, 8);

            // reserved bits
                BitStreamKit_SkipBits(&tEsBitStream, 3);

            elementary_PID =
                BitStreamKit_GetBits(&tEsBitStream, 13);

            // reserved bits
                BitStreamKit_SkipBits(&tEsBitStream, 4);

            ES_info_length =
                BitStreamKit_GetBits(&tEsBitStream, 12);
#else
            pBitEsAddr = pEsStartAddress;
            stream_type = pBitEsAddr[0];

            elementary_PID = (MMP_UINT32) ((pBitEsAddr[1] & 0x1F) << 8 |
                                            pBitEsAddr[2]);

            ES_info_length = (MMP_UINT32) ((pBitEsAddr[3] & 0x0F) << 8 |
                                            pBitEsAddr[4]);
#endif

            ptEsInfo = _PMT_AddEsInfo(ptPmtInfo,
                                      stream_type,
                                      elementary_PID);
            // error handle for allocation fail
            if (MMP_NULL == ptEsInfo)
                return;
                
            loopLength = ES_info_length;

            // Address jump to the start address of the first descriptor
            // of the specific ES_Info.
            pDescriptorStartAddress = pEsStartAddress + ES_INFO_HEADER_SIZE;

            // The last loop of PMT section. See H222.0 p48 to get further
            // details.
            while (loopLength > 0)
            {
#ifdef USE_BITSTREAM_KIT
                BitStreamKit_Init(&tDescriptorBitStream,
                                  pDescriptorStartAddress);

                descriptor_tag =
                    BitStreamKit_GetBits(&tDescriptorBitStream, 8);

                descriptor_length =
                    BitStreamKit_GetBits(&tDescriptorBitStream, 8);

                loopLength -= (DESCRIPTOR_HEADER_SIZE + descriptor_length);

                if (loopLength >= 0)
                {
                    _PMT_EsAddDescriptor(ptEsInfo,
                                         descriptor_tag,
                                         descriptor_length,
                                         tDescriptorBitStream.pStartAddress);

                    // Address jump to the start address of next descriptor
                    pDescriptorStartAddress += (DESCRIPTOR_HEADER_SIZE
                                             + descriptor_length);
                }
#else
                pBitDescriptorAddr = pDescriptorStartAddress;
                descriptor_tag = pBitDescriptorAddr[0];
                descriptor_length = pBitDescriptorAddr[1];

                loopLength -= (DESCRIPTOR_HEADER_SIZE + descriptor_length);

                if (loopLength >= 0)
                {
                    _PMT_EsAddDescriptor(ptEsInfo,
                                         descriptor_tag,
                                         descriptor_length,
                                         &pBitDescriptorAddr[2]);

                    // Address jump to the start address of next descriptor
                    pDescriptorStartAddress += (DESCRIPTOR_HEADER_SIZE
                                             + descriptor_length);
                }
#endif
            }

            restSectionSize -= (ES_INFO_HEADER_SIZE + ES_info_length);
            pEsStartAddress += (ES_INFO_HEADER_SIZE + ES_info_length);
        }
        ptSection = ptSection->ptNextSection;
    }
}

//=============================================================================
/**
 * Insert a section into the section list of PMT decoder
 *
 * @param ptPmtDecoder      The private Decoder to handle the PMT decode.
 * @param ptInsertSection   A section that we want to insert into section list.
 * @return none
 */
//=============================================================================
static void
_PMT_InsertSection(
    PSI_PMT_DECODER* ptPmtDecoder,
    PSI_SECTION* ptInsertSection)
{
    PSI_SECTION* ptCurrentSection = MMP_NULL;
    PSI_SECTION* ptPreviousSection = MMP_NULL;

    if (MMP_NULL == ptInsertSection || MMP_NULL == ptPmtDecoder)
        return;

    if (ptPmtDecoder->ptFirstSection)
    {
        ptCurrentSection = ptPmtDecoder->ptFirstSection;

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
                    // Append
                    ptCurrentSection->ptNextSection = ptInsertSection;
                    ptPmtDecoder->totalSectionCount++;
                    break;
                }
            }
            else if (ptCurrentSection->section_number > ptInsertSection->section_number)
            {
                if (MMP_NULL != ptPreviousSection)
                {
                    ptPreviousSection->ptNextSection = ptInsertSection;
                }
                else
                {
                    ptPmtDecoder->ptFirstSection = ptInsertSection;
                }

                ptInsertSection->ptNextSection = ptCurrentSection;
                ptPmtDecoder->totalSectionCount++;
                break;
            }
            else // ptCurrentSection->section_number == ptInsertSection->section_number
            {
                // Section duplication. Replace and free the old section.
                if (MMP_NULL != ptPreviousSection)
                {
                    ptPreviousSection->ptNextSection = ptInsertSection;
                }
                else
                {
                    ptPmtDecoder->ptFirstSection = ptInsertSection;
                }
                ptInsertSection->ptNextSection = ptCurrentSection->ptNextSection;

                if (ptCurrentSection->pData)
                    PalHeapFree(PAL_HEAP_DEFAULT, ptCurrentSection->pData);
                PalHeapFree(PAL_HEAP_DEFAULT, ptCurrentSection);

                break;
            }
        } while(ptCurrentSection);
    }
    else // The section is the first incoming section
    {
        ptPmtDecoder->ptFirstSection = ptInsertSection;
        ptPmtDecoder->totalSectionCount = 1;
    }
}
