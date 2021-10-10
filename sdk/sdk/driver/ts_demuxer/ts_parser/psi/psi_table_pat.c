/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file psi_table_pat.C
 * Use to decode the PAT table of TS packets
 * @author Steven Hsiao
 * @version 0.1
 */

#include "bitstream_kit.h"
#include "psi_table_pat.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define MAX_SECTION_SIZE_OF_PAT (1024)

// The size is program_number (16) + reserved (3) +
// network_PID/program_map_PID (13) = 4 Bytes
#define PROGRAM_INFO_SIZE (4)

#define PAT_TABLE_ID (0x0)

//typedef enum CURRENT_NEXT_INDICATOR_TAG
//{
//    NOT_APPLICABLE          = 0x00,
//    APPLICABLE              = 0x01
//} CURRENT_NEXT_INDICATOR;

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

// A private decoder to deal with the decoding issue of PAT table.
typedef struct PSI_PAT_DECODER_TAG
{
    // Save the latest PSI_PAT_INFO, turn in it if no update.
    //PSI_PAT_INFO        tCurrentPat; // [20100503] Vincent marked.

    // Processing PSI_PAT_INFO, parse a PAT (completed sections) to fill with
    // it.
    PSI_PAT_INFO*           ptBuildingPat;

    //MMP_BOOL            bCurrentPatValid; // [20100503] Vincent marked.
    MMP_UINT32              last_section_number;

    MMP_UINT32              totalSectionCount;
    PSI_SECTION*            ptFirstSection;
    PSI_PAT_CALLBACK        pfCallback;
    void*                   pCallbackData;

    MMP_INT                 allocId;
    SECTION_PAYLOAD_ALLOC   pfAlloc;
    SECTION_PAYLOAD_FREE    pfFree;
} PSI_PAT_DECODER;

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
_PAT_GatherSection(
    PSI_DECODER* ptDecoder,
    PSI_SECTION* ptSection);

static void
_PAT_DecodeSection(
    PSI_PAT_INFO*   ptPatInfo,
    PSI_SECTION*    ptSection);

static void
_PAT_DestroyProgram(
    PSI_PAT_INFO* ptPatInfo);

static void
_PAT_InsertSection(
    PSI_PAT_DECODER*    ptPatDecoder,
    PSI_SECTION*        ptInsertSection);

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Attach the PAT decoder on the TS parser system
 *
 * @param pfCallback    A callback function is called after the table
 *                      decoding complete.
 * @param pCallbackData The datagram of the callback function
 * @return              PSI_DECODER* to handle psi section parsing and
 *                      data collecion of PAT (PID: 0) of incoming TS packets.
 */
//=============================================================================
PSI_DECODER*
psiTablePAT_AttachDecoder(
    PSI_PAT_CALLBACK    pfCallback,
    void*               pCallbackData)
{
    PSI_DECODER*        ptDecoder = MMP_NULL;
    PSI_PAT_DECODER*    ptPatDecoder = MMP_NULL;

    ptDecoder = (PSI_DECODER*) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                            sizeof(PSI_DECODER));

    if (MMP_NULL == ptDecoder)
        return MMP_NULL;

    ptPatDecoder = (PSI_PAT_DECODER*) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                   sizeof(PSI_PAT_DECODER));

    if (MMP_NULL == ptPatDecoder)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, ptDecoder);
        return MMP_NULL;
    }

    // PSI decoder initialization
    PalMemset(ptDecoder, 0x0, sizeof(PSI_DECODER));
    ptDecoder->pfCallback       = (PSI_DECODER_CALLBACK) _PAT_GatherSection;
    ptDecoder->ptPrivateDecoder = (void*) ptPatDecoder;
    ptDecoder->sectionMaxSize   = MAX_SECTION_SIZE_OF_PAT;
    ptDecoder->bDiscontinuity   = MMP_TRUE;
    ptDecoder->allocId          = PAL_HEAP_DEFAULT;
    ptDecoder->pfAlloc          = _HeapAlloc; // PalHeapAlloc;
    ptDecoder->pfFree           = _HeapFree;  // PalHeapFree;

    // PAT decoder initialization
    PalMemset(ptPatDecoder, 0x0, sizeof(PSI_PAT_DECODER));
    ptPatDecoder->pfCallback    = pfCallback;
    ptPatDecoder->pCallbackData = pCallbackData;
    ptPatDecoder->allocId       = ptDecoder->allocId;
    ptPatDecoder->pfAlloc       = ptDecoder->pfAlloc;
    ptPatDecoder->pfFree        = ptDecoder->pfFree;

    return ptDecoder;
}

//=============================================================================
/**
 * Detach the PAT decoder from the TS parser system and also free all
 * allocated memory
 *
 * @param ptDecoder The existed decoder to handle PAT decode.
 * @return none
 */
//=============================================================================
void
psiTablePAT_DetachDecoder(
    PSI_DECODER* ptDecoder)
{
    PSI_PAT_DECODER* ptPatDecoder = MMP_NULL;

    if (ptDecoder)
        ptPatDecoder = ptDecoder->ptPrivateDecoder;
    else
        return;

    if (ptPatDecoder)
    {
        psiTablePAT_DestroyTable(ptPatDecoder->ptBuildingPat);
        ptPatDecoder->ptBuildingPat = MMP_NULL;                

        SectionKit_DestroySection(ptPatDecoder->allocId, ptPatDecoder->pfFree, ptPatDecoder->ptFirstSection);
        PalHeapFree(PAL_HEAP_DEFAULT, ptPatDecoder);
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
 * Destroy the PAT table and its allocated memory
 *
 * @param ptPatInfo A PAT table keeps the sections decoded information.
 * @return none
 */
//=============================================================================
void
psiTablePAT_DestroyTable(
    PSI_PAT_INFO* ptPatInfo)
{
    if (ptPatInfo)
    {
        _PAT_DestroyProgram(ptPatInfo);
        PalHeapFree(PAL_HEAP_DEFAULT, ptPatInfo);
    }
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Gather section data of the PAT. Once the whole sections data are completed,
 * call decoder to parse the PAT table
 *
 * @param ptDecoder A PSI decoder to handle the TS packet decoding issue
 * @param ptSection The datagram of the current section
 * @return none
 */
//=============================================================================
static void
_PAT_GatherSection(
    PSI_DECODER* ptDecoder,
    PSI_SECTION* ptSection)
{
    MMP_BOOL            bSectionAppend  = MMP_TRUE;
    MMP_BOOL            bTableReInit    = MMP_FALSE;
    MMP_BOOL            bTableComplete  = MMP_FALSE;

    PSI_SECTION*        ptCurrentSection = MMP_NULL;
    PSI_PAT_DECODER*    ptPatDecoder = MMP_NULL;
    PSI_PAT_INFO*       ptPatInfo = MMP_NULL;

    // 0. Ensure the pointer ptDecoder and ptSection are valid
    if (ptDecoder && ptDecoder->ptPrivateDecoder && ptSection)
    {
        ptPatDecoder = (PSI_PAT_DECODER*) ptDecoder->ptPrivateDecoder;
        ptCurrentSection = ptSection;
    }
    else
    {
        // Vincent noted on 9 april 2010:
        // added this to avoid memory leak but it means nothing
        // because we never reach here.
        if (ptSection)
            SectionKit_DestroySection(ptPatDecoder->allocId, ptPatDecoder->pfFree, ptSection);
        return;
    }

    // 1. Table Id validation. Not PID == 0, ignore the section.
    if (ptCurrentSection->table_id != PAT_TABLE_ID)
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
            if (ptPatDecoder->ptBuildingPat)
            {
                // Any of the parameter comparsion is failed.
                // We need to inited the table structure later.
                if ((ptPatDecoder->ptBuildingPat->transport_stream_id != ptCurrentSection->table_id_extension)
                 || (ptPatDecoder->ptBuildingPat->version_number != ptCurrentSection->version_number)
                 || (ptPatDecoder->last_section_number != ptCurrentSection->last_section_number))
                {
                    bTableReInit = MMP_TRUE;
                }
            }
            /* [20100503] Vincent marked.
            else
            {
                // The last saved PAT table should be activated when the
                // current_next_indicator is 0 but the value of incoming
                // section is 1. Also, the version_number of the saved
                // PAT is equal to the version_number of the incoming
                // section. Usually, the version_number will be the number
                // that the previous version_number + 1. The mechanism is
                // used to provide the buffer time for decode side. The
                // benefit is obviously, that is, reduce the table re-build
                // time.
                if ((ptPatDecoder->bCurrentPatValid)
                 && (ptPatDecoder->tCurrentPat.version_number == ptCurrentSection->version_number))
                {
                    // Notify the AP layer about that the new PAT should be
                    // activated.
                    if ((NOT_APPLICABLE == ptPatDecoder->tCurrentPat.current_next_indicator)
                     && (APPLICABLE == ptCurrentSection->current_next_indicator))
                    {
                        ptPatInfo = (PSI_PAT_INFO*) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                                 sizeof(PSI_PAT_INFO));

                        if (ptPatInfo)
                        {
                            ptPatDecoder->tCurrentPat.current_next_indicator = APPLICABLE;

                            PalMemcpy(ptPatInfo,
                                      &ptPatDecoder->tCurrentPat,
                                      sizeof(PSI_PAT_INFO));

                            ptPatDecoder->pfCallback(ptPatDecoder->pCallbackData,
                                                     ptPatInfo);
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
        if (ptPatDecoder->ptBuildingPat)
        {
            psiTablePAT_DestroyTable(ptPatDecoder->ptBuildingPat);
            ptPatDecoder->ptBuildingPat = MMP_NULL;

            // Delete all chained sections
            SectionKit_DestroySection(ptPatDecoder->allocId, ptPatDecoder->pfFree, ptPatDecoder->ptFirstSection);
            ptPatDecoder->ptFirstSection = MMP_NULL;
            ptPatDecoder->totalSectionCount = 0;
        }
        // Record the current pat to invalid
        //ptPatDecoder->bCurrentPatValid = MMP_FALSE; // [20100503] Vincent marked.
    }

    // 5. Append the section into the Table. If sections can form a complete
    //    table, then process decoding.
    if (bSectionAppend)
    {
        if (MMP_NULL == ptPatDecoder->ptBuildingPat)
        {
            ptPatDecoder->ptBuildingPat =
                (PSI_PAT_INFO*) PalHeapAlloc(PAL_HEAP_DEFAULT, sizeof(PSI_PAT_INFO));

            if (ptPatDecoder->ptBuildingPat)
            {
                PalMemset(ptPatDecoder->ptBuildingPat, 0x0, sizeof(PSI_PAT_INFO));
                
                // take over the parsed information of pat
                ptPatDecoder->ptBuildingPat->current_next_indicator =
                    ptCurrentSection->current_next_indicator;
                ptPatDecoder->ptBuildingPat->version_number =
                    ptCurrentSection->version_number;
                ptPatDecoder->ptBuildingPat->transport_stream_id =
                    ptCurrentSection->table_id_extension;
                ptPatDecoder->last_section_number =
                    ptCurrentSection->last_section_number;
            }
        }

        // Insert the section into the section list
        _PAT_InsertSection(ptPatDecoder, ptCurrentSection);

        bTableComplete = MMP_FALSE;
        if ((ptPatDecoder->totalSectionCount)
         == (ptPatDecoder->last_section_number + 1))
        {
            bTableComplete = MMP_TRUE;
        }

        // error handle fot allocation fail
        if (MMP_NULL == ptPatDecoder->ptBuildingPat)
            return;

        // Time for PAT table decode
        if (MMP_TRUE == bTableComplete)
        {
            /* [20100503] Vincent marked.
            // Save the build PAT inform to currnet PAT including
            // version_number, current_next_indicator, and etc.
            PalMemcpy(&ptPatDecoder->tCurrentPat,
                      ptPatDecoder->ptBuildingPat,
                      sizeof(PSI_PAT_INFO));
            ptPatDecoder->bCurrentPatValid = MMP_TRUE;
            */

            // Decode the table
            _PAT_DecodeSection(ptPatDecoder->ptBuildingPat,
                               ptPatDecoder->ptFirstSection);

            // Section information is stored in the building PAT table,
            // therefore, delete these sections.
            SectionKit_DestroySection(ptPatDecoder->allocId, ptPatDecoder->pfFree, ptPatDecoder->ptFirstSection);

            // Callback to notify AP layer about the new Table constructed
            if (ptPatDecoder->pfCallback)
            {
                ptPatDecoder->pfCallback(ptPatDecoder->pCallbackData,
                                         ptPatDecoder->ptBuildingPat);
            }

            // The AP will free the ptBuildingPat. We just need to re-init
            // the decoder parameter.
            ptPatDecoder->ptBuildingPat = MMP_NULL;
            ptPatDecoder->ptFirstSection = MMP_NULL;
            ptPatDecoder->totalSectionCount = 0;
        }
    }
    else // Ignore the incoming section.
        SectionKit_DestroySection(ptPatDecoder->allocId, ptPatDecoder->pfFree, ptCurrentSection);
}

//=============================================================================
/**
 * Decode the sections and then construct a program list of PAT table.
 *
 * @param ptPatInfo A PAT table keeps the sections decoded information.
 * @param ptSection A section list to form a complete PAT table.
 * @return none
 */
//=============================================================================
static void
_PAT_DecodeSection(
    PSI_PAT_INFO*   ptPatInfo,
    PSI_SECTION*    ptSection)
{
#ifdef USE_BITSTREAM_KIT
    BITSTREAM           tBitStream = { 0 };
#else
    MMP_UINT8*          pCurrentAddr = MMP_NULL;
#endif

    PSI_PAT_PROGRAM*    ptProgram = MMP_NULL;
    PSI_PAT_PROGRAM*    ptLastProgram = MMP_NULL;

    if (ptPatInfo)
        ptLastProgram = ptPatInfo->ptFirstProgram;

    while (ptSection)
    {
#ifdef USE_BITSTREAM_KIT
        BitStreamKit_Init(&tBitStream, ptSection->pPayloadStartAddress);
        // The loop is used to build up the program list of PAT table
        while ((tBitStream.pStartAddress + PROGRAM_INFO_SIZE) <= ptSection->pPayloadEndAddress)
        {
            ptProgram = (PSI_PAT_PROGRAM*) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                        sizeof(PSI_PAT_PROGRAM));
            // program_number
            ptProgram->program_number =
                BitStreamKit_GetBits(&tBitStream, 16);

            // reserved
                BitStreamKit_SkipBits(&tBitStream, 3);

            // program_map_PID
            // No handle to network_PID?
            // However, always get this PID value, but check if (program_number == 0)
            // before retrieve program_map_PID in callback function.
            ptProgram->program_map_PID =
                BitStreamKit_GetBits(&tBitStream, 13);

            ptProgram->ptNextProgram = MMP_NULL;

            // No program in the program list yet
            if (MMP_NULL == ptLastProgram)
            {
                ptPatInfo->ptFirstProgram = ptLastProgram = ptProgram;
                ptPatInfo->totalProgramCount = 1;
            }
            else
            {
                // Find the last program of the program list
                while(ptLastProgram->ptNextProgram != MMP_NULL)
                    ptLastProgram = ptLastProgram->ptNextProgram;

                ptLastProgram->ptNextProgram = ptProgram;
                ptPatInfo->totalProgramCount++;
            }
        }
#else
        pCurrentAddr = ptSection->pPayloadStartAddress;
        // The loop is used to build up the program list of PAT table
        while ((pCurrentAddr + PROGRAM_INFO_SIZE) <= ptSection->pPayloadEndAddress)
        {
            ptProgram = (PSI_PAT_PROGRAM*) PalHeapAlloc(PAL_HEAP_DEFAULT,
                                                        sizeof(PSI_PAT_PROGRAM));
            // error handle for allocation fail
            if (MMP_NULL == ptProgram)
                return;

            // program_number
            ptProgram->program_number = (MMP_UINT32) (pCurrentAddr[0] << 8 |
                                                      pCurrentAddr[1]);
            ptProgram->program_map_PID = (MMP_UINT32) ((pCurrentAddr[2] & 0x1F) << 8 |
                                                        pCurrentAddr[3]);
           
            pCurrentAddr += 4;
            ptProgram->ptNextProgram = MMP_NULL;

            // No program in the program list yet
            if (MMP_NULL == ptLastProgram)
            {
                ptPatInfo->ptFirstProgram = ptLastProgram = ptProgram;
                ptPatInfo->totalProgramCount = 1;
            }
            else
            {
                // Find the last program of the program list
                while(ptLastProgram->ptNextProgram != MMP_NULL)
                    ptLastProgram = ptLastProgram->ptNextProgram;

                ptLastProgram->ptNextProgram = ptProgram;
                ptPatInfo->totalProgramCount++;
            }
        }
#endif
        ptSection = ptSection->ptNextSection;
    }
}

//=============================================================================
/**
 * Destroy the program list of PAT table
 *
 * @param ptPatInfo A PAT table keeps the sections decoded information.
 * @return none
 */
//=============================================================================
static void
_PAT_DestroyProgram(
    PSI_PAT_INFO* ptPatInfo)
{
    PSI_PAT_PROGRAM* ptNextProgram = MMP_NULL;
    
    if (ptPatInfo)
    {
        while (ptPatInfo->ptFirstProgram)
        {
            ptNextProgram = ptPatInfo->ptFirstProgram->ptNextProgram;
            PalHeapFree(PAL_HEAP_DEFAULT, ptPatInfo->ptFirstProgram);
            ptPatInfo->ptFirstProgram = ptNextProgram;
        }
        ptPatInfo->ptFirstProgram = MMP_NULL;
    }
}

//=============================================================================
/**
 * Insert a section into the section list of PAT decoder
 *
 * @param ptPatDecoder      The private Decoder to handle the PAT decode.
 * @param ptInsertSection   A section that we want to insert into section list.
 * @return none
 */
//=============================================================================
static void
_PAT_InsertSection(
    PSI_PAT_DECODER*    ptPatDecoder,
    PSI_SECTION*        ptInsertSection)
{
    PSI_SECTION* ptCurrentSection = MMP_NULL;
    PSI_SECTION* ptPreviousSection = MMP_NULL;

    if ((MMP_NULL == ptInsertSection) || (MMP_NULL == ptPatDecoder))
        return;

    if (ptPatDecoder->ptFirstSection)
    {
        ptCurrentSection = ptPatDecoder->ptFirstSection;

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
                    ptPatDecoder->totalSectionCount++;
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
                    ptPatDecoder->ptFirstSection = ptInsertSection;
                }

                ptInsertSection->ptNextSection = ptCurrentSection;
                ptPatDecoder->totalSectionCount++;
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
                    ptPatDecoder->ptFirstSection = ptInsertSection;
                }
                ptInsertSection->ptNextSection = ptCurrentSection->ptNextSection;

                ptCurrentSection->ptNextSection = MMP_NULL;
                SectionKit_DestroySection(ptPatDecoder->allocId, ptPatDecoder->pfFree, ptCurrentSection);
                break;
            }
        } while (ptCurrentSection);
    }
    else // The section is the first incoming section
    {
        ptPatDecoder->ptFirstSection = ptInsertSection;
        ptPatDecoder->totalSectionCount = 1;
    }
}
