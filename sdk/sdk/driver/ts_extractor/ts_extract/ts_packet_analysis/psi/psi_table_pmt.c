

#include "ts_packet_analysis_defs.h"

#include "psi_table_cfg.h"

#if (CONFIG_PSI_TABLE_OPR_PMT_DECODER_DESC)

#include "psi_table_operator.h"
#include "psi_table_pmt.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define PMT_MAX_SECTION_SIZE            (1024)
#define PMT_TABLE_ID                    (0x02)

/**
 * The byte offset of the first program_info descriptor from the PMT section start.
 **/
#define PROGRAM_INFO_BYTE_OFFSET        (12)

/**
 * The header bytes of descriptor is descriptor_tag(8) + descriptor_length(8) = 2 bytes
 **/
#define DESCRIPTOR_HEADER_SIZE          (2)

#define PMT_INVALID_DESCRIPTOR_TAG      (0xFF)
#define PMT_INVALID_DESCRIPTOR_LENGTH   (255)
#define PMT_INVALID_ELEMENTARY_PID      (0x1FFF)

/**
 * The header bytes of ES is stream_type(8) + reserved(3) + elementary_PID(13)
 * + reserved(4) + ES_info_length(12) = 5 bytes
 **/
#define ES_INFO_HEADER_SIZE             (5)
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
// A private decoder to deal with the decoding issue of PMT table.
typedef struct PSI_PMT_DECODER_TAG
{
    uint32_t                program_number;

    // Processing PSI_PMT_INFO, parse a PMT (completed sections) to fill with
    // it.
    PSI_PMT_INFO            *ptBuildingPmt;
    uint32_t                last_section_number;

    uint32_t                totalSectionCount;
    PSI_SECT                *ptFirstSect;
    PSI_TABLE_MBOX          psi_pkt_dec_mbox;   // from psi_packet_decoder mbox
    // PSI_PMT_CALLBACK        pfCallback;
    // void*                   pCallbackData;

    int                     allocId;
    SECT_PAYLOAD_ALLOC      pf_pmt_sect_alloc;
    SECT_PAYLOAD_FREE       pf_pmt_sect_free;
} PSI_PMT_DECODER;
//=============================================================================
//                Global Data Definition
//=============================================================================
static uint32_t pmt_clear_table(PSI_TABLE_MBOX *pPsiTableMbox, void *extraData);
//=============================================================================
//                Private Function Definition
//=============================================================================
static void*
_pmt_heap_func(
    int         allocId,
    uint32_t    size)
{
    return tspa_malloc(size);
}

static void
_pmt_free_func(
    int     allocId,
    void    *pFreeAddr)
{
    free(pFreeAddr);
}

static void
_pmt_es_add_descriptor(
    PSI_PMT_ES_INFO     *ptEsInfo,
    uint32_t            tag,
    uint32_t            length,
    uint8_t             *pData)
{
    PSI_DESCR   *ptDescriptor = 0;
    PSI_DESCR   *ptLastDescriptor = 0;

    do{
        // Invalid input
        if( !ptEsInfo || !pData ||
            (tag >= PMT_INVALID_DESCRIPTOR_TAG) ||
            (length >= PMT_INVALID_DESCRIPTOR_LENGTH) )
            break;

        ptDescriptor = psi_descr_create(tag, length, pData);
        if( !ptDescriptor )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "create descr faile !!");
            break;
        }

        if( !ptEsInfo->ptFirstDescriptor )
            ptEsInfo->ptFirstDescriptor = ptDescriptor;
        else
        {
            ptLastDescriptor = ptEsInfo->ptFirstDescriptor;
            while( ptLastDescriptor->ptNextDescriptor )
                ptLastDescriptor = ptLastDescriptor->ptNextDescriptor;

            ptLastDescriptor->ptNextDescriptor = ptDescriptor;
            ptLastDescriptor = ptDescriptor;
        }
    }while(0);

    return;
}

static PSI_PMT_ES_INFO*
_pmt_add_es_info(
    PSI_PMT_INFO    *ptPmtInfo,
    uint32_t        stream_type,
    uint32_t        elementary_PID)
{
    PSI_PMT_ES_INFO     *ptEsInfo = 0;
    PSI_PMT_ES_INFO     *ptLastEsInfo = 0;

    do{
        if( !ptPmtInfo || elementary_PID >= PMT_INVALID_ELEMENTARY_PID )
            break;

        ptEsInfo = tspa_malloc(sizeof(PSI_PMT_ES_INFO));
        if( ptEsInfo )
        {
            memset(ptEsInfo, 0x0, sizeof(PSI_PMT_ES_INFO));

            ptEsInfo->elementary_PID = elementary_PID;
            ptEsInfo->stream_type    = stream_type;

            if( !ptPmtInfo->ptFirstEsInfo )
            {
                ptPmtInfo->ptFirstEsInfo = ptEsInfo;
                ptPmtInfo->totalEsCount  = 1;
            }
            else
            {
                ptLastEsInfo = ptPmtInfo->ptFirstEsInfo;
                while( ptLastEsInfo->ptNexEsInfo )
                    ptLastEsInfo = ptLastEsInfo->ptNexEsInfo;

                ptLastEsInfo->ptNexEsInfo = ptEsInfo;
                ptLastEsInfo = ptEsInfo;
                ptPmtInfo->totalEsCount++;
            }
        }
    }while(0);

    return ptEsInfo;
}

static void
_pmt_add_descriptor(
    PSI_PMT_INFO    *ptPmtInfo,
    uint32_t        tag,
    uint32_t        length,
    uint8_t         *pData)
{
    PSI_DESCR   *ptDescriptor = 0;
    PSI_DESCR   *ptLastDescriptor = 0;

    do{
        // Invalid input
        if( !ptPmtInfo || !pData ||
            (tag >= PMT_INVALID_DESCRIPTOR_TAG) ||
            (length >= PMT_INVALID_DESCRIPTOR_LENGTH) )
            break;

        ptDescriptor = psi_descr_create(tag, length, pData);
        if( !ptDescriptor )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "create descr faile !!");
            break;
        }

        if( !ptPmtInfo->ptFirstDescriptor )
            ptPmtInfo->ptFirstDescriptor = ptDescriptor;
        else
        {
            ptLastDescriptor = ptPmtInfo->ptFirstDescriptor;
            while( ptLastDescriptor->ptNextDescriptor )
                ptLastDescriptor = ptLastDescriptor->ptNextDescriptor;

            ptLastDescriptor->ptNextDescriptor = ptDescriptor;
            ptLastDescriptor = ptDescriptor;
        }

    }while(0);

    return;
}

static void
_pmt_decode_section(
    PSI_PMT_INFO    *ptPmtInfo,
    PSI_SECT        *ptSect)
{
    uint32_t    descriptor_tag;
    uint32_t    descriptor_length;
    uint32_t    stream_type;
    uint32_t    elementary_PID;
    uint32_t    program_info_length;
    uint32_t    ES_info_length;
    int         loopLength;
    int         restSectSize;

    uint8_t     *pBitCurAddr = 0;
    uint8_t     *pBitDescrAddr = 0;
    uint8_t     *pBitEsAddr = 0;
    uint8_t     *pDescrStartAddr;
    uint8_t     *pEsStartAddr;

    PSI_PMT_ES_INFO*    ptEsInfo = 0;

    while( ptSect )
    {
        bool    bBreak = false;

        restSectSize = ptSect->pPayloadEndAddress - ptSect->pData;
        pBitCurAddr  = ptSect->pData;
        program_info_length = (uint32_t)((pBitCurAddr[10] & 0x0F) << 8 | pBitCurAddr[11]);

        // Get the start address of the first program descriptor
        pDescrStartAddr = ptSect->pData + PROGRAM_INFO_BYTE_OFFSET;

        // The length of the first loop is determined by program_info_length
        loopLength = program_info_length;

        // The first loop of PMT section. See H222.0 p48 to get further details.
        while( loopLength > 0 )
        {
            pBitDescrAddr = pDescrStartAddr;

            descriptor_tag = pBitDescrAddr[0];

            descriptor_length = pBitDescrAddr[1];

            loopLength -= (DESCRIPTOR_HEADER_SIZE + descriptor_length);

            if( loopLength >= 0 )
            {
                _pmt_add_descriptor(ptPmtInfo,
                                   descriptor_tag,
                                   descriptor_length,
                                   &pBitDescrAddr[2]);

                // Address jump to the start address of next descriptor
                pDescrStartAddr += (DESCRIPTOR_HEADER_SIZE + descriptor_length);
            }
        }

        restSectSize -= (PROGRAM_INFO_BYTE_OFFSET + program_info_length);

        // Address jump to the start address of the second loop of PMT section.
        // See H222.0 p48 to get further details.
        pEsStartAddr = (ptSect->pData
                            + PROGRAM_INFO_BYTE_OFFSET
                            + program_info_length);

        // The second loop of PMT section. See H222.0 p48 to get further details.
        while( restSectSize > 0 )
        {
            pBitEsAddr  = pEsStartAddr;
            stream_type = pBitEsAddr[0];

            elementary_PID = (uint32_t)((pBitEsAddr[1] & 0x1F) << 8 | pBitEsAddr[2]);
            ES_info_length = (uint32_t)((pBitEsAddr[3] & 0x0F) << 8 | pBitEsAddr[4]);

            ptEsInfo = _pmt_add_es_info(ptPmtInfo, stream_type, elementary_PID);
            // error handle for allocation fail
            if( !ptEsInfo )     return;

            loopLength = ES_info_length;

            // Address jump to the start address of the first descriptor
            // of the specific ES_Info.
            pDescrStartAddr = pEsStartAddr + ES_INFO_HEADER_SIZE;

            // The last loop of PMT section. See H222.0 p48 to get further
            // details.
            while( loopLength > 0 )
            {
                pBitDescrAddr     = pDescrStartAddr;
                descriptor_tag    = pBitDescrAddr[0];
                descriptor_length = pBitDescrAddr[1];

                loopLength -= (DESCRIPTOR_HEADER_SIZE + descriptor_length);

                if( loopLength >= 0 )
                {
                    _pmt_es_add_descriptor(ptEsInfo,
                                           descriptor_tag,
                                           descriptor_length,
                                           &pBitDescrAddr[2]);

                    // Address jump to the start address of next descriptor
                    pDescrStartAddr += (DESCRIPTOR_HEADER_SIZE + descriptor_length);
                }
            }

            restSectSize -= (ES_INFO_HEADER_SIZE + ES_info_length);
            pEsStartAddr += (ES_INFO_HEADER_SIZE + ES_info_length);
        }

        ptSect = ptSect->ptNextSection;
    }
    return;
}

static void
_pmt_insert_section(
    PSI_PMT_DECODER     *ptPmtDecoder,
    PSI_SECT            *ptInsertSect)
{
    PSI_SECT    *ptCurSect = 0;
    PSI_SECT    *ptPrevSect = 0;

    do{
        if( !ptPmtDecoder || !ptInsertSect )        break;

        if( ptPmtDecoder->ptFirstSect )
        {
            ptCurSect = ptPmtDecoder->ptFirstSect;

            do
            {
                if( ptCurSect->section_number < ptInsertSect->section_number )
                {
                    if ( ptCurSect->ptNextSection )
                    {
                        ptPrevSect = ptCurSect;
                        ptCurSect = ptCurSect->ptNextSection;
                    }
                    else
                    {
                        // Append
                        ptCurSect->ptNextSection = ptInsertSect;
                        ptPmtDecoder->totalSectionCount++;
                        break;
                    }
                }
                else if ( ptCurSect->section_number > ptInsertSect->section_number )
                {
                    if( ptPrevSect )    ptPrevSect->ptNextSection = ptInsertSect;
                    else                ptPmtDecoder->ptFirstSect = ptInsertSect;

                    ptInsertSect->ptNextSection = ptCurSect;
                    ptPmtDecoder->totalSectionCount++;
                    break;
                }
                else // ptCurSect->section_number == ptInsertSect->section_number
                {
                    // Section duplication. Replace and free the old section.
                    if( ptPrevSect )    ptPrevSect->ptNextSection = ptInsertSect;
                    else                ptPmtDecoder->ptFirstSect = ptInsertSect;

                    ptInsertSect->ptNextSection = ptCurSect->ptNextSection;

                    if( ptCurSect->pData )  free(ptCurSect->pData);

                    free(ptCurSect);
                    break;
                }
            }while( ptCurSect );
        }
        else // The section is the first incoming section
        {
            ptPmtDecoder->ptFirstSect = ptInsertSect;
            ptPmtDecoder->totalSectionCount = 1;
        }
    }while(0);

    return;
}

static void
_pmt_gather_section(
    PSI_TABLE_DECODER    *pDecoder,
    PSI_SECT             *pSect)
{
    PSI_SECT            *ptCurSect = 0;
    PSI_PMT_DECODER     *ptPmtDecoder = 0;

    do{
        bool                bSectionAppend  = true;
        bool                bTableReInit    = false;
        bool                bTableComplete  = false;

        //------------------------------------
        // 0. Ensure the pointer pDecoder and ptSect are valid
        if( !pDecoder || !pDecoder->pPrivDecoder || !pSect )
        {
            // Vincent noted on 9 april 2010:
            // added this to avoid memory leak but it means nothing
            // because we never reach here.
            if( pSect )
                // if some decoder is Null, why can use the member in the decoder ????
                psi_section_destroy(pDecoder->allocId, pDecoder->pf_sect_free, pSect);
            break;
        }

        ptPmtDecoder = (PSI_PMT_DECODER*)pDecoder->pPrivDecoder;
        ptCurSect    = pSect;
        //------------------------------------
        // 1. Table Id validation. Not PID == 0x02, ignore the section.
        if( ptCurSect->table_id != PMT_TABLE_ID )
            bSectionAppend = false;

        //------------------------------------
        // 2. If the section_syntax_indicator != 1, then this section
        //    is not a generic table.
        //    On the other hand, it's not part of PAT
        if( ptCurSect->section_syntax_indicator != 1 )
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
            else if( ptPmtDecoder->ptBuildingPmt )
            {
                // The building pat is already existed. check the consistence of
                // the building table and incoming section.

                if( (ptPmtDecoder->ptBuildingPmt->version_number != ptCurSect->version_number) ||
                    (ptPmtDecoder->last_section_number != ptCurSect->last_section_number) )
                {
                    // Any of the parameter comparsion is failed.
                    // We need to inited the table structure later.
                    bTableReInit = true;
                }
            }
        }

        //------------------------------------
        // 4. Check whether the table should be re-inited.
        if( bTableReInit && ptPmtDecoder->ptBuildingPmt )
        {
            PSI_TABLE_MBOX  patMbox = {0};

            patMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PMT;
            patMbox.psi_table_mbox_arg.argv.pmt.pPsi_Pmt_Info = (void*)ptPmtDecoder->ptBuildingPmt;
            pmt_clear_table(&patMbox, 0);

            ptPmtDecoder->ptBuildingPmt = 0;

            // Delete all chained sections
            psi_section_destroy(ptPmtDecoder->allocId,
                                ptPmtDecoder->pf_pmt_sect_free, ptPmtDecoder->ptFirstSect);
            ptPmtDecoder->ptFirstSect = 0;
            ptPmtDecoder->totalSectionCount = 0;
        }
        //------------------------------------
        // 5. Append the section into the Table. If sections can form a complete
        //    table, then process decoding.
        if( !bSectionAppend )
        {
            // Ignore the incoming section.
            psi_section_destroy(ptPmtDecoder->allocId,
                                ptPmtDecoder->pf_pmt_sect_free, ptCurSect);
            break;
        }

        if( !ptPmtDecoder->ptBuildingPmt )
        {
            ptPmtDecoder->ptBuildingPmt = tspa_malloc(sizeof(PSI_PMT_INFO));
            if( ptPmtDecoder->ptBuildingPmt )
            {
                memset(ptPmtDecoder->ptBuildingPmt, 0x0, sizeof(PSI_PMT_INFO));

                // take over the parsed information of pmt
                ptPmtDecoder->ptBuildingPmt->program_number         = ptCurSect->table_id_extension;
                ptPmtDecoder->ptBuildingPmt->current_next_indicator = ptCurSect->current_next_indicator;
                ptPmtDecoder->ptBuildingPmt->version_number         = ptCurSect->version_number;
                ptPmtDecoder->last_section_number                   = ptCurSect->last_section_number;

                ptPmtDecoder->ptBuildingPmt->pcr_pid =
                    (uint32_t)((ptCurSect->pData[8] & 0x1F) << 8 | ptCurSect->pData[9]);
            }
        }

        //------------------------------------
        // Insert the section into the section list
        _pmt_insert_section(ptPmtDecoder, ptCurSect);

        bTableComplete = false;
        if( ptPmtDecoder->totalSectionCount == (ptPmtDecoder->last_section_number + 1) )
            bTableComplete = true;

        //------------------------------------
        // error handle fot allocation fail
        if( !ptPmtDecoder->ptBuildingPmt )      break;

        //------------------------------------
        // Time for PMT table decode
        if( bTableComplete )
        {
            // Decode the table
            _pmt_decode_section(ptPmtDecoder->ptBuildingPmt,
                                ptPmtDecoder->ptFirstSect);

            // Section information is stored in the building PAT table,
            // therefore, delete these sections.
            psi_section_destroy(ptPmtDecoder->allocId,
                                ptPmtDecoder->pf_pmt_sect_free, ptPmtDecoder->ptFirstSect);

            // Callback to notify AP layer about the new Table constructed
            if( ptPmtDecoder->psi_pkt_dec_mbox.func )
            {
                ptPmtDecoder->psi_pkt_dec_mbox.psi_table_mbox_arg.argv.pmt.pPsi_Pmt_Info = ptPmtDecoder->ptBuildingPmt;
                ptPmtDecoder->psi_pkt_dec_mbox.func(&ptPmtDecoder->psi_pkt_dec_mbox.psi_table_mbox_arg, 0);
            }

            // The AP will free the ptBuildingPmt. We just need to re-init
            // the decoder parameter.
            ptPmtDecoder->ptBuildingPmt = 0;
            ptPmtDecoder->ptFirstSect = 0;
            ptPmtDecoder->totalSectionCount = 0;
        }
    }while(0);

    return;
}

static uint32_t
pmt_attach_decoder(
    PSI_TABLE_MBOX  *pPsiTableMbox,
    void            *extraData)
{
    uint32_t            result = 0;
    PSI_TABLE_DECODER   *pPsi_table_decoder = 0;
    PSI_PMT_DECODER     *pPmtDecoder = 0;

    do{
        uint32_t        program_number = 0;

        if( !pPsiTableMbox )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "Null pointer !!");
            break;
        }

        pPsi_table_decoder = pPsiTableMbox->psi_table_mbox_arg.argv.pmt.pPsi_Table_Decoder;
        program_number     = pPsiTableMbox->psi_table_mbox_arg.argv.pmt.pmtProgramNum;

        pPmtDecoder = tspa_malloc(sizeof(PSI_PMT_DECODER));
        if( !pPmtDecoder )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "allcate faile !!");
            break;
        }

        //-----------------------------
        // PSI decoder initilization
        pPsi_table_decoder->pf_psi_sect_handler = _pmt_gather_section;
        pPsi_table_decoder->pPrivDecoder        = (void*) pPmtDecoder;
        pPsi_table_decoder->sectMaxSize         = PMT_MAX_SECTION_SIZE;
        pPsi_table_decoder->bDiscontinuity      = true;
        pPsi_table_decoder->allocId             = 0;
        pPsi_table_decoder->pf_sect_alloc       = _pmt_heap_func;
        pPsi_table_decoder->pf_sect_free        = _pmt_free_func;

        //----------------------------------
        // PMT decoder initilization
        memset(pPmtDecoder, 0x0, sizeof(PSI_PMT_DECODER));
        pPmtDecoder->program_number    = program_number;
        pPmtDecoder->psi_pkt_dec_mbox  = (*pPsiTableMbox);
        // pPmtDecoder->pfCallback        = pfCallback;
        // pPmtDecoder->pCallbackData     = pCallbackData;
        pPmtDecoder->allocId           = pPsi_table_decoder->allocId;
        pPmtDecoder->pf_pmt_sect_alloc = pPsi_table_decoder->pf_sect_alloc;
        pPmtDecoder->pf_pmt_sect_free  = pPsi_table_decoder->pf_sect_free;

    }while(0);

    return result;
}

static uint32_t
pmt_detach_decoder(
    PSI_TABLE_MBOX  *pPsiTableMbox,
    void            *extraData)
{
    uint32_t            result = 0;
    PSI_TABLE_DECODER   *pPsi_table_decoder = 0;
    PSI_PMT_DECODER     *pPmtDecoder = 0;

    do{
        PSI_TABLE_MBOX  pmtMbox = {0};

        if( !pPsiTableMbox )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "Null pointer !!");
            break;
        }

        pPsi_table_decoder = pPsiTableMbox->psi_table_mbox_arg.argv.pmt.pPsi_Table_Decoder;
        if( !pPsi_table_decoder || !pPsi_table_decoder->pPrivDecoder )
            break;

        pPmtDecoder = (PSI_PMT_DECODER*)pPsi_table_decoder->pPrivDecoder;

        pmtMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_PMT;
        pmtMbox.psi_table_mbox_arg.argv.pmt.pPsi_Pmt_Info = (void*)pPmtDecoder->ptBuildingPmt;
        pmt_clear_table(&pmtMbox, extraData);

        psi_section_destroy(pPmtDecoder->allocId,
                            pPmtDecoder->pf_pmt_sect_free, pPmtDecoder->ptFirstSect);
        free(pPmtDecoder);

        if( pPsi_table_decoder->ptCurSect )
            psi_section_destroy(pPsi_table_decoder->allocId,
                                pPsi_table_decoder->pf_sect_free, pPsi_table_decoder->ptCurSect);
    }while(0);

    return result;
}

static uint32_t
pmt_clear_table(
    PSI_TABLE_MBOX  *pPsiTableMbox,
    void            *extraData)
{
    uint32_t        result = 0;
    PSI_PMT_INFO    *pPmt_Info = (PSI_PMT_INFO*)pPsiTableMbox->psi_table_mbox_arg.argv.pmt.pPsi_Pmt_Info;

    if( pPmt_Info )
    {
        PSI_PMT_ES_INFO     *ptCurEsInfo = 0;
        PSI_PMT_ES_INFO     *ptTmepEsInfo = 0;

        if( pPmt_Info->ptFirstDescriptor )
            psi_descr_destroy(pPmt_Info->ptFirstDescriptor);

        ptCurEsInfo = pPmt_Info->ptFirstEsInfo;
        while( ptCurEsInfo )
        {
            ptTmepEsInfo = ptCurEsInfo->ptNexEsInfo;
            if( ptCurEsInfo->ptFirstDescriptor )
                psi_descr_destroy(ptCurEsInfo->ptFirstDescriptor);

            free(ptCurEsInfo);
            ptCurEsInfo = ptTmepEsInfo;
        }

        free(pPmt_Info);
    }
    return result;
}
//=============================================================================
//                Public Function Definition
//=============================================================================
PSI_TABLE_OPR PSI_TABLE_OPR_pmt_decoder_desc =
{
    "pmt decoder",      // char        *name;
    0,                  // struct PSI_TABLE_OPR_T     *next;
    PSI_TABLE_PMT,      // PSI_TABLE_ID                psi_table_id;
    0,                  // void        *privInfo;
    pmt_attach_decoder, // uint32_t    (*attach_decoder)(PSI_TABLE_ARG *pPsiTableArg, void *extraData);
    pmt_detach_decoder, // uint32_t    (*detach_decoder)(PSI_TABLE_ARG *pPsiTableArg, void *extraData);
    pmt_clear_table,    // uint32_t    (*clear_table)(PSI_TABLE_ARG *pPsiTableArg, void *extraData);
};

#else   /* #if (CONFIG_PSI_TABLE_OPR_PMT_DECODER_DESC) */
PSI_TABLE_OPR PSI_TABLE_OPR_pmt_decoder_desc =
{
    "pmt decoder",
    0,
    PSI_TABLE_PMT,
    0,
};
#endif