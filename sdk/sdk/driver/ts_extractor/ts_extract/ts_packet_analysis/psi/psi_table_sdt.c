
#include "ts_packet_analysis_defs.h"

#include "psi_table_cfg.h"

#if (CONFIG_PSI_TABLE_OPR_SDT_DECODER_DESC)

#include "psi_table_operator.h"
#include "psi_priv_sect_demux.h"
#include "psi_table_sdt.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define SDT_EXTRA_SECTION_HEADER_SIZE       (3)
#define CRC32_FIELD_SIZE                    (4)
#define SDT_DESCRIPTOR_PRIOR_HEADER_SIZE    (2)

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
// A private decoder to deal with the decoding issue of SDT table.
typedef struct PSI_SDT_DECODER_TAG
{
    PSI_SDT_INFO            *ptBuildingSdt;
    uint32_t                last_section_number;

    uint32_t                totalSectionCount;
    PSI_SECT                *ptFirstSect;

    PSI_TABLE_MBOX          psi_pkt_dec_mbox;   // from psi_packet_decoder mbox

    int                     allocId;
    SECT_PAYLOAD_ALLOC      pf_sdt_sect_alloc;
    SECT_PAYLOAD_FREE       pf_sdt_sect_free;
}PSI_SDT_DECODER;
//=============================================================================
//                Global Data Definition
//=============================================================================
static uint32_t sdt_clear_table(PSI_TABLE_MBOX *pPsiTableMbox, void *extraData);

//=============================================================================
//                Private Function Definition
//=============================================================================
static void*
_sdt_heap_func(
    int         allocId,
    uint32_t    size)
{
    return tspa_malloc(size);
}

static void
_sdt_free_func(
    int     allocId,
    void    *pFreeAddr)
{
    free(pFreeAddr);
}

static PSI_DESCR*
_sdt_service_add_descriptor(
    PSI_SDT_SERVICE     *ptService,
    uint32_t            tag,
    uint32_t            length,
    uint8_t             *pData)
{
    PSI_DESCR   *ptDescriptor = psi_descr_create(tag, length, pData);

    if( ptDescriptor )
    {
        if( !ptService->ptFirstDescriptor )
        {
            ptService->ptFirstDescriptor = ptDescriptor;
        }
        else
        {
            PSI_DESCR   *ptLastDescriptor = ptService->ptFirstDescriptor;
            while( ptLastDescriptor->ptNextDescriptor )
                ptLastDescriptor = ptLastDescriptor->ptNextDescriptor;

            ptLastDescriptor->ptNextDescriptor = ptDescriptor;
        }
    }

    return ptDescriptor;
}

static PSI_SDT_SERVICE*
_sdt_add_service(
    PSI_SDT_INFO*   ptSdtInfo,
    uint32_t        service_id,
    uint32_t        EIT_schedule_flag,
    uint32_t        EIT_present_following_flag,
    uint32_t        running_status,
    uint32_t         free_CA_mode)
{
    PSI_SDT_SERVICE     *ptService = tspa_malloc(sizeof(PSI_SDT_SERVICE));

    if( ptService )
    {
        ptService->service_id                   = service_id;
        ptService->EIT_schedule_flag            = EIT_schedule_flag;
        ptService->EIT_present_following_flag   = EIT_present_following_flag;
        ptService->running_status               = running_status;
        ptService->free_CA_mode                 = free_CA_mode;
        ptService->ptNextService                = 0;
        ptService->ptFirstDescriptor            = 0;

        if( ptSdtInfo->ptFirstService == 0 )
        {
            ptSdtInfo->ptFirstService = ptService;
        }
        else
        {
            PSI_SDT_SERVICE     *ptLastService = ptSdtInfo->ptFirstService;

            while( ptLastService->ptNextService )
                ptLastService = ptLastService->ptNextService;

            ptLastService->ptNextService = ptService;
        }
    }

    return ptService;
}

static void
_sdt_decode_section(
    PSI_SDT_INFO    *ptSdtInfo,
    PSI_SECT        *ptSect)
{

    uint8_t     *pCurAddr = 0;
    uint8_t     *pEnd = 0;
    uint32_t    service_id                   = 0;
    uint32_t    EIT_schedule_flag            = 0;
    uint32_t    EIT_present_following_flag   = 0;
    uint32_t    running_status               = 0;
    uint32_t    free_CA_mode                 = 0;
    uint32_t    descriptors_loop_length      = 0;

    PSI_SDT_SERVICE  *ptService              = 0;

    while( ptSect )
    {
        pCurAddr = ptSect->pPayloadStartAddress + SDT_EXTRA_SECTION_HEADER_SIZE;

        // The loop is used to build up the information of SDT.
        while( (pCurAddr + CRC32_FIELD_SIZE) <= ptSect->pPayloadEndAddress )
        {
            service_id                   = (uint32_t)(pCurAddr[0] << 8 | pCurAddr[1]);
            EIT_schedule_flag            = (pCurAddr[2] & 0x02) >> 1;
            EIT_present_following_flag   = (pCurAddr[2] & 0x01);
            running_status               = (pCurAddr[3] & 0xE0) >> 5;
            free_CA_mode                 = (pCurAddr[3] & 0x10) >> 4;
            descriptors_loop_length      = (uint32_t)((pCurAddr[3] & 0x0F) << 8 | pCurAddr[4]);

            pCurAddr += 5;
            ptService = _sdt_add_service(ptSdtInfo,
                                         service_id,
                                         EIT_schedule_flag,
                                         EIT_present_following_flag,
                                         running_status,
                                         free_CA_mode);
            // error handle for allocation fail
            if( !ptService )
                return;

            pEnd = pCurAddr + descriptors_loop_length;
            if( pEnd > ptSect->pPayloadEndAddress )
                break;

            while( pCurAddr + SDT_DESCRIPTOR_PRIOR_HEADER_SIZE <= pEnd )
            {
                uint32_t    descriptor_tag    = pCurAddr[0];
                uint32_t    descriptor_length = pCurAddr[1];

                pCurAddr += 2;
                if( pCurAddr + descriptor_length <= pEnd )
                    _sdt_service_add_descriptor(ptService,
                                                descriptor_tag,
                                                descriptor_length,
                                                pCurAddr);
                pCurAddr += descriptor_length;
            }
        }

        ptSect = ptSect->ptNextSection;
    }
    return;
}

static void
_sdt_insert_section(
    PSI_SDT_DECODER     *ptSdtDecoder,
    PSI_SECT            *ptInsertSect)
{
    PSI_SECT     *ptCurSect = 0;
    PSI_SECT     *ptPrevSect = 0;

    if ( !ptInsertSect || !ptSdtDecoder)
        return;

    if (ptSdtDecoder->ptFirstSect)
    {
        ptCurSect = ptSdtDecoder->ptFirstSect;

        // search all current section link list to insert at the appropriated position
        do
        {
            if( ptCurSect->section_number < ptInsertSect->section_number )
            {
                if( ptCurSect->ptNextSection )
                {
                    ptPrevSect = ptCurSect;
                    ptCurSect  = ptCurSect->ptNextSection;
                }
                else
                {
                    // append to the end of section link list
                    ptCurSect->ptNextSection = ptInsertSect;
                    ptSdtDecoder->totalSectionCount++;
                    break;
                }
            }
            else if ( ptCurSect->section_number > ptInsertSect->section_number )
            {
                // insert the new coming section into section link list
                if( ptPrevSect )    ptPrevSect->ptNextSection = ptInsertSect;
                else                ptSdtDecoder->ptFirstSect = ptInsertSect;

                ptInsertSect->ptNextSection = ptCurSect;
                ptSdtDecoder->totalSectionCount++;
                break;
            }
            else // find the same section (number), replace with the new coming one
            {
                // Section duplication. Replace and free the old section.
                if( ptPrevSect )    ptPrevSect->ptNextSection = ptInsertSect;
                else                ptSdtDecoder->ptFirstSect = ptInsertSect;

                ptInsertSect->ptNextSection = ptCurSect->ptNextSection;

                ptCurSect->ptNextSection = 0;
                psi_section_destroy(ptSdtDecoder->allocId,
                                    ptSdtDecoder->pf_sdt_sect_free, ptCurSect);
                break;
            }
        } while( ptCurSect );
    }
    else // The section is the first incoming section
    {
        ptSdtDecoder->ptFirstSect = ptInsertSect;
        ptSdtDecoder->totalSectionCount = 1;
    }

    return;
}

static void
_sdt_gather_section(
    PSI_TABLE_DECODER    *pDecoder,
    void                 *pSubdecoder_priv_decoder,
    PSI_SECT             *pSect)
{
    do{
        bool               bSectionAppend = true;
        bool               bTableReInit   = false;
        bool               bTableComplete = false;
        PSI_SDT_DECODER    *ptSdtDecoder = 0;
        PSI_SECT           *ptCurSect = 0;

        if( !pDecoder || !pSubdecoder_priv_decoder || !pSect )
        {
            // Vincent noted on 9 april 2010:
            // added this to avoid memory leak but it means nothing
            // because we never reach here.
            if( pSect )
                // if some decoder is Null, why can use the member in the decoder ????
                psi_section_destroy(pDecoder->allocId, pDecoder->pf_sect_free, pSect);
            break;
        }

        ptSdtDecoder = (PSI_SDT_DECODER*)pSubdecoder_priv_decoder;
        ptCurSect    = pSect;

        //----------------------------------
        // If the section_syntax_indicator != 1, then this section
        // is not a generic table.
        // On the other hand, it's not part of SDT
        if( ptCurSect->section_syntax_indicator != 1 )
              bSectionAppend = false;

        //---------------------------------------
        // If bSectionAppend is true then we have a valid SDT section
        if( bSectionAppend )
        {
            // Discontinuity
            if( pDecoder->bDiscontinuity )
            {
                bTableReInit = true;
                pDecoder->bDiscontinuity = false;
            }
            else if( ptSdtDecoder->ptBuildingSdt )
            {
                // The building SDT is already existed. check the consistence of
                // the building table and incoming section.

                if( (ptSdtDecoder->ptBuildingSdt->transport_stream_id != ptCurSect->table_id_extension) ||
                    (ptSdtDecoder->ptBuildingSdt->version_number != ptCurSect->version_number) ||
                    (ptSdtDecoder->last_section_number != ptCurSect->last_section_number) )
                {
                    // Any of the parameter comparsion is failed.
                    // We need to reinited the table structure.
                    bTableReInit = true;
                }
            }
        }

        //---------------------------------------
        // Check whether the table should be re-inited.
        if( bTableReInit )
        {
            if( ptSdtDecoder->ptBuildingSdt )
            {
                PSI_TABLE_MBOX  patMbox = {0};

                patMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_SDT;
                patMbox.psi_table_mbox_arg.argv.sdt.pPsi_Sdt_Info = (void*)ptSdtDecoder->ptBuildingSdt;
                sdt_clear_table(&patMbox, 0);

                ptSdtDecoder->ptBuildingSdt = 0;
            }

            // Delete all chained sections
            psi_section_destroy(ptSdtDecoder->allocId, ptSdtDecoder->pf_sdt_sect_free, ptSdtDecoder->ptFirstSect);
            ptSdtDecoder->ptFirstSect = 0;
            ptSdtDecoder->totalSectionCount = 0;
        }
        //---------------------------------------
        // Append the section into the Table. If sections can form a complete
        // table, then process decoding.
        if( bSectionAppend == false )
        {
            // Ignore the incoming section.
            psi_section_destroy(ptSdtDecoder->allocId, ptSdtDecoder->pf_sdt_sect_free, ptCurSect);
            break;
        }

        if( !ptSdtDecoder->ptBuildingSdt )
        {
            ptSdtDecoder->ptBuildingSdt = tspa_malloc(sizeof(PSI_SDT_INFO));

            if( ptSdtDecoder->ptBuildingSdt )
            {
                memset(ptSdtDecoder->ptBuildingSdt, 0x0, sizeof(PSI_SDT_INFO));

                // take over the parsed information of sdt
                ptSdtDecoder->ptBuildingSdt->transport_stream_id    = ptCurSect->table_id_extension;
                ptSdtDecoder->ptBuildingSdt->version_number         = ptCurSect->version_number;
                ptSdtDecoder->ptBuildingSdt->current_next_indicator = ptCurSect->current_next_indicator;
                ptSdtDecoder->last_section_number                   = ptCurSect->last_section_number;

                ptSdtDecoder->ptBuildingSdt->original_network_id =
                    (uint32_t)(ptCurSect->pPayloadStartAddress[0] << 8 | ptCurSect->pPayloadStartAddress[1]);
            }

            //-----------------------------------------
            // Insert the section into the section list (ptSdtDecoder->ptFirstSection).
            _sdt_insert_section(ptSdtDecoder, ptCurSect);

            //-----------------------------------------
            // Check if we have all the sections
            bTableComplete = false;
            if( ptSdtDecoder->totalSectionCount == (ptSdtDecoder->last_section_number + 1) )
                bTableComplete = true;

            //-----------------------------------------
            // error handle fot allocation fail
            if( !ptSdtDecoder->ptBuildingSdt )      break;

            //-----------------------------------------
            // Time for SDT table decode
            if( bTableComplete == true )
            {
                // Decode the table
                _sdt_decode_section(ptSdtDecoder->ptBuildingSdt,
                                    ptSdtDecoder->ptFirstSect);

                // Section information is stored in the building SDT table,
                // therefore, delete these sections.
                psi_section_destroy(ptSdtDecoder->allocId, ptSdtDecoder->pf_sdt_sect_free, ptSdtDecoder->ptFirstSect);

                // Callback to notify AP layer about the new Table constructed
                if( ptSdtDecoder->psi_pkt_dec_mbox.func )
                {
                    ptSdtDecoder->psi_pkt_dec_mbox.psi_table_mbox_arg.argv.sdt.pPsi_Sdt_Info = ptSdtDecoder->ptBuildingSdt;
                    ptSdtDecoder->psi_pkt_dec_mbox.func(&ptSdtDecoder->psi_pkt_dec_mbox.psi_table_mbox_arg, 0);
                }

                // The AP will free the ptBuildingSdt. We just need to re-init
                // the decoder parameter.
                ptSdtDecoder->ptBuildingSdt     = 0;
                ptSdtDecoder->ptFirstSect       = 0;
                ptSdtDecoder->totalSectionCount = 0;
            }
        }
    }while(0);

    return;
}

static void
_sdt_detach_subdecoder(
    PSI_PRIV_SECT_DEMUX   *pPriv_sect_demux,
    PSI_TABLE_MBOX_ARG    *pPsi_table_mbox_arg,
    void                  *extraData)
{
    uint32_t    result = 0;

    do{
        PSI_DEMUX_SUBDECODER    *ptSubdecoder = 0;
        PSI_DEMUX_SUBDECODER    **pptPrevSubdecoder = 0;
        PSI_SDT_DECODER         *ptSdtDecoder = 0;
        uint32_t                table_id, table_id_extension;

        if( !pPriv_sect_demux || !pPsi_table_mbox_arg )      break;

        table_id           = pPsi_table_mbox_arg->argv.sdt.table_id;
        table_id_extension = pPsi_table_mbox_arg->argv.sdt.table_id_extension;

        if( pPriv_sect_demux->get_subtable_decoder )
        {
            ptSubdecoder =
                pPriv_sect_demux->get_subtable_decoder(pPriv_sect_demux, pPsi_table_mbox_arg, extraData);
        }

        // No such SDT decoder
        if( !ptSubdecoder )     break;

        ptSdtDecoder = (PSI_SDT_DECODER*)ptSubdecoder->ptPrivDecoder;
        if( ptSdtDecoder )
        {
            PSI_TABLE_MBOX  patMbox = {0};

            patMbox.psi_table_mbox_arg.psi_table_id           = PSI_TABLE_SDT;
            patMbox.psi_table_mbox_arg.argv.sdt.pPsi_Sdt_Info = (void*)ptSdtDecoder->ptBuildingSdt;
            sdt_clear_table(&patMbox, extraData);

            ptSdtDecoder->ptBuildingSdt = 0;

            psi_section_destroy(ptSdtDecoder->allocId,
                                ptSdtDecoder->pf_sdt_sect_free, ptSdtDecoder->ptFirstSect);

            free(ptSdtDecoder);
        }

        pptPrevSubdecoder = &pPriv_sect_demux->ptFirstSubdecoder;
        while( *pptPrevSubdecoder != ptSubdecoder )
            pptPrevSubdecoder = &(*pptPrevSubdecoder)->ptNextSubdecoder;

        *pptPrevSubdecoder = ptSubdecoder->ptNextSubdecoder;
        free(ptSubdecoder);
    }while(0);

    return;
}

static void
_sdt_destroy_service(
    PSI_SDT_INFO    *ptSdtInfo)
{
    if( ptSdtInfo )
    {
        PSI_SDT_SERVICE     *ptService = ptSdtInfo->ptFirstService;
        while( ptService )
        {
            PSI_SDT_SERVICE     *ptNextService = ptService->ptNextService;

            psi_descr_destroy(ptService->ptFirstDescriptor);

            free(ptService);
            ptService = ptNextService;
        }
        ptSdtInfo->ptFirstService = 0;
    }
    return;
}

static uint32_t
sdt_attach_decoder(
    PSI_TABLE_MBOX  *pPsiTableMbox,
    void            *extraData)
{
    uint32_t                result = 0;
    PSI_TABLE_DECODER       *pPsi_table_decoder = 0;
    PSI_DEMUX_SUBDECODER    *ptSubdecoder = 0;
    PSI_SDT_DECODER         *pSdtDecoder = 0;

    do{
        PSI_PRIV_SECT_DEMUX     *pPriv_sect_demux = 0;
        PSI_TABLE_MBOX_ARG      *pPsi_table_mbox_arg = 0;
        uint32_t                table_id, table_id_extension;

        if( !pPsiTableMbox )
        {
            tspa_msg_ex(1, "Null pointer !!");
            result = -1;
            break;
        }

        pPsi_table_mbox_arg = &pPsiTableMbox->psi_table_mbox_arg;
        pPsi_table_decoder = (PSI_TABLE_DECODER*)pPsi_table_mbox_arg->argv.sdt.pPsi_Table_Decoder;
        pPriv_sect_demux   = (PSI_PRIV_SECT_DEMUX*)pPsi_table_mbox_arg->argv.sdt.pPriv_sect_demux;
        if( !pPsi_table_decoder || !pPriv_sect_demux )
        {
            tspa_msg_ex(1, "Null pointer !!");
            result = -1;
            break;
        }

        if( pPriv_sect_demux->get_subtable_decoder &&
            pPriv_sect_demux->get_subtable_decoder(pPriv_sect_demux, pPsi_table_mbox_arg, extraData) )
        {
            // A subtable decoder for the same table_id and table_id_extension
            // is already attached.
            break;
        }

        table_id           = pPsi_table_mbox_arg->argv.sdt.table_id;
        table_id_extension = pPsi_table_mbox_arg->argv.sdt.table_id_extension;

        if( !(ptSubdecoder = tspa_malloc(sizeof(PSI_DEMUX_SUBDECODER))) ||
            !(pSdtDecoder = tspa_malloc(sizeof(PSI_SDT_DECODER))) )
        {
            tspa_msg_ex(1, "err, allocate fail !!");
            result = -1;
            break;
        }

        //----------------------------------
        // Subtable decoder initialization
        ptSubdecoder->gather_sect       = _sdt_gather_section;
        ptSubdecoder->ptPrivDecoder     = pSdtDecoder;
        ptSubdecoder->id                = (table_id << 16) | table_id_extension;
        ptSubdecoder->detach_subdecoder = _sdt_detach_subdecoder;

        //----------------------------------
        // Attach the subtable decoder to the demux
        ptSubdecoder->ptNextSubdecoder = pPriv_sect_demux->ptFirstSubdecoder;
        pPriv_sect_demux->ptFirstSubdecoder = ptSubdecoder;

        //----------------------------------
        // SDT decoder initialization
        memset(pSdtDecoder, 0x0, sizeof(PSI_SDT_DECODER));
        pSdtDecoder->psi_pkt_dec_mbox  = *(pPsiTableMbox);
        pSdtDecoder->allocId           = pPsi_table_decoder->allocId;
        pSdtDecoder->pf_sdt_sect_alloc = pPsi_table_decoder->pf_sect_alloc;
        pSdtDecoder->pf_sdt_sect_free  = pPsi_table_decoder->pf_sect_free;
    }while(0);

    if( result != 0 )
    {
        if( ptSubdecoder )      free(ptSubdecoder);
        if( pSdtDecoder )       free(pSdtDecoder);
    }
    return result;
}

static uint32_t
sdt_clear_table(
    PSI_TABLE_MBOX  *pPsiTableMbox,
    void            *extraData)
{
    uint32_t        result = 0;
    PSI_SDT_INFO    *pSdt_Info = (PSI_SDT_INFO*)pPsiTableMbox->psi_table_mbox_arg.argv.sdt.pPsi_Sdt_Info;

    if( pSdt_Info )
    {
        _sdt_destroy_service(pSdt_Info);
        free(pSdt_Info);
    }
    return result;
}
//=============================================================================
//                Public Function Definition
//=============================================================================
PSI_TABLE_OPR PSI_TABLE_OPR_sdt_decoder_desc =
{
    "sdt decoder",      // char        *name;
    0,                  // struct PSI_TABLE_OPR_T     *next;
    PSI_TABLE_SDT,      // PSI_TABLE_ID                psi_table_id;
    0,                  // void        *privInfo;
    sdt_attach_decoder, // uint32_t    (*attach_decoder)(PSI_TABLE_ARG *pPsiTableArg, void *extraData);
    0,                  // uint32_t    (*detach_decoder)(PSI_TABLE_ARG *pPsiTableArg, void *extraData);
    sdt_clear_table,    // uint32_t    (*clear_table)(PSI_TABLE_ARG *pPsiTableArg, void *extraData);
};

#else   /* #if (CONFIG_PSI_TABLE_OPR_SDT_DECODER_DESC) */
PSI_TABLE_OPR PSI_TABLE_OPR_sdt_decoder_desc =
{
    "sdt decoder",
    0,
    PSI_TABLE_SDT,
    0,
};
#endif

