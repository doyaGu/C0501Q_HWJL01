
#include "ts_packet_analysis_defs.h"

#include "psi_table_cfg.h"

#if (CONFIG_PSI_TABLE_OPR_PAT_DECODER_DESC)

#include "psi_table_operator.h"
#include "psi_table_pat.h"
//=============================================================================
//                Constant Definition
//=============================================================================

#define MAX_SECTION_SIZE_OF_PAT     (1024)

// The size is program_number (16) + reserved (3) +
// network_PID/program_map_PID (13) = 4 Bytes
#define PROGRAM_INFO_SIZE           (4)

#define PAT_TABLE_ID                (0x0)
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
// A private decoder to deal with the decoding issue of PAT table.
typedef struct PSI_PAT_DECODER_TAG
{
    // Processing PSI_PAT_INFO, parse a PAT (completed sections) to fill with it.
    PSI_PAT_INFO*           ptBuildingPat;

    uint32_t                last_section_number;

    uint32_t                totalSectionCount;
    PSI_SECT*               ptFirstSection;

    PSI_TABLE_MBOX          psi_pkt_dec_mbox;   // from psi_packet_decoder mbox

    int                     allocId;
    SECT_PAYLOAD_ALLOC      pf_pat_sect_alloc;
    SECT_PAYLOAD_FREE       pf_pat_sect_free;
} PSI_PAT_DECODER;
//=============================================================================
//                Global Data Definition
//=============================================================================
static uint32_t pat_clear_table(PSI_TABLE_MBOX *pPsiTableMbox, void *extraData);

//=============================================================================
//                Private Function Definition
//=============================================================================
static void*
_pat_heap_func(
    int         allocId,
    uint32_t    size)
{
    return tspa_malloc(size);
}

static void
_pat_free_func(
    int     allocId,
    void    *pFreeAddr)
{
    free(pFreeAddr);
}

static void
_pat_insert_section(
    PSI_PAT_DECODER     *pPatDecoder,
    PSI_SECT            *pInsertSect)
{
    PSI_SECT    *ptCurSect = 0;
    PSI_SECT    *ptPrevSect = 0;

    do{
        if( !pPatDecoder || !pInsertSect )      break;

        if( pPatDecoder->ptFirstSection )
        {
            ptCurSect = pPatDecoder->ptFirstSection;

            // search all current section link list to insert at the appropriated position
            do
            {
                if( ptCurSect->section_number < pInsertSect->section_number )
                {
                    if( ptCurSect->ptNextSection )
                    {
                        ptPrevSect = ptCurSect;
                        ptCurSect  = ptCurSect->ptNextSection;
                    }
                    else
                    {
                        // append to the end of section link list
                        ptCurSect->ptNextSection = pInsertSect;
                        pPatDecoder->totalSectionCount++;
                        break;
                    }
                }
                else if ( ptCurSect->section_number > pInsertSect->section_number )
                {
                    // insert the new coming section into section link list
                    if( ptPrevSect )    ptPrevSect->ptNextSection   = pInsertSect;
                    else                pPatDecoder->ptFirstSection = pInsertSect;

                    pInsertSect->ptNextSection = ptCurSect;
                    pPatDecoder->totalSectionCount++;
                    break;
                }
                else // find the same section (number), replace with the new coming one
                {
                    // Section duplication. Replace and free the old section.
                    if( ptPrevSect )    ptPrevSect->ptNextSection   = pInsertSect;
                    else                pPatDecoder->ptFirstSection = pInsertSect;

                    pInsertSect->ptNextSection = ptCurSect->ptNextSection;

                    ptCurSect->ptNextSection = 0;
                    psi_section_destroy(pPatDecoder->allocId, pPatDecoder->pf_pat_sect_free, ptCurSect);
                    break;
                }
            }while (ptCurSect);
        }
        else // The section is the first incoming section
        {
            pPatDecoder->ptFirstSection = pInsertSect;
            pPatDecoder->totalSectionCount = 1;
        }
    }while(0);

    return;
}

static void
_pat_decode_section(
    PSI_PAT_INFO    *ptPatInfo,
    PSI_SECT        *ptSect)
{

    uint8_t*            pCurAddr = 0;
    PSI_PAT_PROGRAM*    ptProgram = 0;
    PSI_PAT_PROGRAM*    ptLastProgram = 0;

    if( ptPatInfo )     ptLastProgram = ptPatInfo->pFirstProgram;

    while( ptSect )
    {
        bool    bBreak = false;

        pCurAddr = ptSect->pPayloadStartAddress;

        // The loop is used to build up the program list of PAT table
        while( (pCurAddr + PROGRAM_INFO_SIZE) <= ptSect->pPayloadEndAddress )
        {
            ptProgram = tspa_malloc(sizeof(PSI_PAT_PROGRAM));
            if( !ptProgram )
            {
                bBreak = true;
                break;
            }

            // program_number
            ptProgram->program_number  = (uint32_t)(pCurAddr[0] << 8 | pCurAddr[1]);
            ptProgram->program_map_PID = (uint32_t)((pCurAddr[2] & 0x1F) << 8 | pCurAddr[3]);

            pCurAddr += 4;
            ptProgram->pNextProgram = 0;

            // No program in the program list yet
            if( ptLastProgram )
            {
                // Find the last program of the program list
                while( ptLastProgram->pNextProgram )
                    ptLastProgram = ptLastProgram->pNextProgram;

                ptLastProgram->pNextProgram = ptProgram;
                ptPatInfo->totalProgramCount++;
            }
            else
            {
                ptPatInfo->pFirstProgram = ptLastProgram = ptProgram;
                ptPatInfo->totalProgramCount = 1;
            }
        }

        if( bBreak == true )        break;

        ptSect = ptSect->ptNextSection;
    }

    return;
}

static void
_pat_gather_section(
    PSI_TABLE_DECODER    *pDecoder,
    PSI_SECT             *pSect)
{
    bool    bSectionAppend  = true;
    bool    bTableReInit    = false;
    bool    bTableComplete  = false;

    do{
        PSI_PAT_DECODER     *pPatDecoder = 0;
        PSI_SECT            *pCurSect = 0;

        //------------------------------------
        // 0. Ensure the pointer pDecoder and ptSect are valid
        if( !pDecoder || !pDecoder->pPrivDecoder || !pSect )
        {
            if( pSect )
                // if some decoder is Null, why can use the member in the decoder ????
                psi_section_destroy(pDecoder->allocId, pDecoder->pf_sect_free, pSect);
            break;
        }

        pPatDecoder = (PSI_PAT_DECODER*)pDecoder->pPrivDecoder;
        pCurSect    = pSect;

        //------------------------------------
        // 1. Table Id validation. Not PID == 0, ignore the section.
        // 2. If the section_syntax_indicator != 1, then this section
        //    is not a generic table.
        //    On the other hand, it's not part of PAT
        if( (pCurSect->table_id != PAT_TABLE_ID) ||
            (pCurSect->section_syntax_indicator != 1) )
            bSectionAppend = false;

        //------------------------------------
        // 3. Do some check if the section should be appened to other
        //    sections to form a complete table.
        if( bSectionAppend )
        {
            // Discontinuity
            if( pDecoder->bDiscontinuity )
            {
                bTableReInit = true;
                pDecoder->bDiscontinuity = false;
            }
            else
            {
                // The building pat is already existed. check the consistence of
                // the building table and incoming section.
                if( pPatDecoder->ptBuildingPat )
                {
                    // Any of the parameter comparsion is failed.
                    // We need to inited the table structure later.
                    if( (pPatDecoder->ptBuildingPat->transport_stream_id != pCurSect->table_id_extension)
                        || (pPatDecoder->ptBuildingPat->version_number != pCurSect->version_number)
                        || (pPatDecoder->last_section_number != pCurSect->last_section_number) )
                    {
                        bTableReInit = true;
                    }
                }
            }
        }

        //------------------------------------
        // 4. Check whether the table should be re-inited.
        if( bTableReInit && pPatDecoder->ptBuildingPat )
        {
            PSI_TABLE_MBOX  patMbox = {0};

            patMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PAT;
            patMbox.psi_table_mbox_arg.argv.pat.pPsi_Pat_Info = (void*)pPatDecoder->ptBuildingPat;
            pat_clear_table(&patMbox, 0);

            pPatDecoder->ptBuildingPat = 0;

            // Delete all chained sections
            psi_section_destroy(pPatDecoder->allocId, pPatDecoder->pf_pat_sect_free, pPatDecoder->ptFirstSection);

            pPatDecoder->ptFirstSection = 0;
            pPatDecoder->totalSectionCount = 0;
        }

        //------------------------------------
        // 5. Append the section into the Table. If sections can form a complete
        //    table, then process decoding.
        if( bSectionAppend )
        {
            if( pPatDecoder->ptBuildingPat == 0 )
            {
                pPatDecoder->ptBuildingPat = tspa_malloc(sizeof(PSI_PAT_INFO));
                if( pPatDecoder->ptBuildingPat )
                {
                    memset(pPatDecoder->ptBuildingPat, 0x0, sizeof(PSI_PAT_INFO));

                    // take over the parsed information of pat
                    pPatDecoder->ptBuildingPat->current_next_indicator = pCurSect->current_next_indicator;
                    pPatDecoder->ptBuildingPat->version_number         = pCurSect->version_number;
                    pPatDecoder->ptBuildingPat->transport_stream_id    = pCurSect->table_id_extension;
                    pPatDecoder->last_section_number                   = pCurSect->last_section_number;
                }
            }

            // Insert the section into the section list
            _pat_insert_section(pPatDecoder, pCurSect);

            bTableComplete = false;
            if( pPatDecoder->totalSectionCount == (pPatDecoder->last_section_number + 1) )
            {
                bTableComplete = true;
            }

            // error handle fot allocation fail
            if( !pPatDecoder->ptBuildingPat )
                break;

            // Time for PAT table decode
            if( bTableComplete )
            {
                // Decode the table
                _pat_decode_section(pPatDecoder->ptBuildingPat, pPatDecoder->ptFirstSection);

                // Section information is stored in the building PAT table,
                // therefore, delete these sections.
                psi_section_destroy(pPatDecoder->allocId, pPatDecoder->pf_pat_sect_free, pPatDecoder->ptFirstSection);

                // Callback to notify AP layer about the new Table constructed
                if( pPatDecoder->psi_pkt_dec_mbox.func )
                {
                    pPatDecoder->psi_pkt_dec_mbox.psi_table_mbox_arg.argv.pat.pPsi_Pat_Info = pPatDecoder->ptBuildingPat;
                    pPatDecoder->psi_pkt_dec_mbox.func(&pPatDecoder->psi_pkt_dec_mbox.psi_table_mbox_arg, 0);
                }

                // The AP will free the ptBuildingPat. We just need to re-init
                // the decoder parameter.
                pPatDecoder->ptBuildingPat     = 0;
                pPatDecoder->ptFirstSection    = 0;
                pPatDecoder->totalSectionCount = 0;
            }
        }
        else // Ignore the incoming section.
            psi_section_destroy(pPatDecoder->allocId, pPatDecoder->pf_pat_sect_free, pCurSect);

    }while(0);

    return;
}

static uint32_t
pat_clear_table(
    PSI_TABLE_MBOX  *pPsiTableMbox,
    void            *extraData)
{
    uint32_t        result = 0;
    PSI_PAT_INFO    *pPat_Info = (PSI_PAT_INFO*)pPsiTableMbox->psi_table_mbox_arg.argv.pat.pPsi_Pat_Info;

    if( pPat_Info )
    {
        PSI_PAT_PROGRAM     *pNextProgram = 0;

        // pat destroy program
        while( pPat_Info->pFirstProgram )
        {
            pNextProgram = pPat_Info->pFirstProgram->pNextProgram;
            free(pPat_Info->pFirstProgram);
            pPat_Info->pFirstProgram = pNextProgram;
        }
        pPat_Info->pFirstProgram = 0;

        free(pPat_Info);
    }
    return result;
}

static uint32_t
pat_attach_decoder(
    PSI_TABLE_MBOX  *pPsiTableMbox,
    void            *extraData)
{
    uint32_t            result = 0;
    PSI_TABLE_DECODER   *pPsi_table_decoder = 0;
    PSI_PAT_DECODER     *pPatDecoder = 0;

    do{
        if( !pPsiTableMbox )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "Null pointer !!");
            break;
        }

        pPsi_table_decoder = pPsiTableMbox->psi_table_mbox_arg.argv.pat.pPsi_Table_Decoder;

        pPatDecoder = tspa_malloc(sizeof(PSI_PAT_DECODER));
        if( !pPatDecoder )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "allcate faile !!");
            break;
        }

        // PSI decoder initialization
        pPsi_table_decoder->pf_psi_sect_handler = _pat_gather_section;
        pPsi_table_decoder->pPrivDecoder        = (void*) pPatDecoder;
        pPsi_table_decoder->sectMaxSize         = MAX_SECTION_SIZE_OF_PAT;
        pPsi_table_decoder->bDiscontinuity      = true;
        pPsi_table_decoder->allocId             = 0;
        pPsi_table_decoder->pf_sect_alloc       = _pat_heap_func;
        pPsi_table_decoder->pf_sect_free        = _pat_free_func;

        // PAT decoder initialization
        memset(pPatDecoder, 0x0, sizeof(PSI_PAT_DECODER));
        pPatDecoder->psi_pkt_dec_mbox  = (*pPsiTableMbox);
        pPatDecoder->allocId           = pPsi_table_decoder->allocId;
        pPatDecoder->pf_pat_sect_alloc = pPsi_table_decoder->pf_sect_alloc;
        pPatDecoder->pf_pat_sect_free  = pPsi_table_decoder->pf_sect_free;
    }while(0);

    return result;
}

static uint32_t
pat_detach_decoder(
    PSI_TABLE_MBOX  *pPsiTableMbox,
    void            *extraData)
{
    uint32_t            result = 0;
    PSI_TABLE_DECODER   *pPsi_table_decoder = 0;
    PSI_PAT_DECODER     *pPatDecoder = 0;

    do{
        PSI_TABLE_MBOX  patMbox = {0};

        if( !pPsiTableMbox )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "Null pointer !!");
            break;
        }

        pPsi_table_decoder = pPsiTableMbox->psi_table_mbox_arg.argv.pat.pPsi_Table_Decoder;
        if( !pPsi_table_decoder || !pPsi_table_decoder->pPrivDecoder )
            break;

        pPatDecoder = (PSI_PAT_DECODER*)pPsi_table_decoder->pPrivDecoder;

        patMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PAT;
        patMbox.psi_table_mbox_arg.argv.pat.pPsi_Pat_Info = (void*)pPatDecoder->ptBuildingPat;
        pat_clear_table(&patMbox, extraData);

        pPatDecoder->ptBuildingPat = 0;

        psi_section_destroy(pPatDecoder->allocId, pPatDecoder->pf_pat_sect_free, pPatDecoder->ptFirstSection);
        free(pPatDecoder);

        if( pPsi_table_decoder->ptCurSect )
            psi_section_destroy(pPsi_table_decoder->allocId,
                                pPsi_table_decoder->pf_sect_free, pPsi_table_decoder->ptCurSect);
    }while(0);

    return result;
}


//=============================================================================
//                Public Function Definition
//=============================================================================
PSI_TABLE_OPR PSI_TABLE_OPR_pat_decoder_desc =
{
    "pat decoder",      // char        *name;
    0,                  // struct PSI_TABLE_OPR_T     *next;
    PSI_TABLE_PAT,      // PSI_TABLE_ID                psi_table_id;
    0,                  // void        *privInfo;
    pat_attach_decoder, // uint32_t    (*attach_decoder)(PSI_TABLE_ARG *pPsiTableArg, void *extraData);
    pat_detach_decoder, // uint32_t    (*detach_decoder)(PSI_TABLE_ARG *pPsiTableArg, void *extraData);
    pat_clear_table,    // uint32_t    (*clear_table)(PSI_TABLE_ARG *pPsiTableArg, void *extraData);
};

#else   /* #if (CONFIG_PSI_TABLE_OPR_PAT_DECODER_DESC) */
PSI_TABLE_OPR PSI_TABLE_OPR_pat_decoder_desc =
{
    "pat decoder",
    0,
    PSI_TABLE_PAT,
    0,
};
#endif
