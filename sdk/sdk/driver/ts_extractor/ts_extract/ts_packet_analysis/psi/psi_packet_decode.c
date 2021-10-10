


#include "ts_packet_analysis.h"
#include "register_template.h"

//=============================================================================
//                Constant Definition
//=============================================================================
/**
 * We need table_id (8) + section_syntax_indicator(1) + private_indicator(1)
 * + Reserved (2) + private_section_length(12) = 3 Bytes to complete the
 * header part of a private_section.
 **/
#define PRIVATE_SECTION_HEADER_SIZE     (3)

#define CRC32_BYTE_NUMBER               (4)
#define STUFF_BYTE                      (0xFF)

// though the max section length of NIT, SDT, TDT and TOT is 1024,
// the max section length of EIT is 4096
#define MAX_SIZE_OF_PRIVATE_SECTION     (4096)
//=============================================================================
//                Macro Definition
//=============================================================================
DEFINE_REGISTER_TEMPLATE(PSI_TABLE_OPR, PSI_TABLE_ID);

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
static void*
_psi_pkt_heap_func(
    int         allocId,
    uint32_t    size)
{
    return tspa_malloc(size);
}

static void
_psi_pkt_free_func(
    int     allocId,
    void    *pFreeAddr)
{
    free(pFreeAddr);
}

static void
_psi_pkt_decode_section(
    PSI_TABLE_DECODER   *pPsi_decoder,
    PSI_SECT            **pptSect)
{
    uint8_t     *pCurAddr = 0;
    PSI_SECT    *ptSect   = *pptSect;

    pCurAddr = ptSect->pData;
    ptSect->table_id = pCurAddr[0];
    ptSect->section_syntax_indicator = (pCurAddr[1] & 0x80) >> 7;
    // ptSect->private_indicator = (pCurAddr[1] & 0x40) >> 6;

    pCurAddr += 3;

    if( ptSect->section_syntax_indicator )
    {
        ptSect->pPayloadEndAddress -= CRC32_BYTE_NUMBER;

        if( psi_section_verify(ptSect) )
        {
            ptSect->table_id_extension     = (uint32_t)(pCurAddr[0] << 8 | pCurAddr[1]);
            ptSect->version_number         = (pCurAddr[2] & 0x3E) >> 1;
            ptSect->current_next_indicator = (pCurAddr[2] & 0x1);
            ptSect->section_number         = pCurAddr[3];
            ptSect->last_section_number    = pCurAddr[4];
            ptSect->pPayloadStartAddress   = &pCurAddr[5];
        }
        else
        {
            psi_section_destroy(pPsi_decoder->allocId,
                                pPsi_decoder->pf_sect_free, ptSect);
            *pptSect = 0;
        }
    }
    else
    {
        ptSect->pPayloadStartAddress = pCurAddr;
    }

    return;
}

static uint32_t
_psi_pkt_attach_decoder(
    PSI_PKT_DECODER     *pPsiPktDecoder,
    PSI_TABLE_MBOX      *pPsiTableMbox,
    void                *extraData)
{
    uint32_t     result = TSPA_ERR_OK;

    do{
        switch( pPsiTableMbox->psi_table_mbox_arg.psi_table_id )
        {
            case PSI_TABLE_PAT:
                pPsiPktDecoder->pCur_Psi_Table_Desc = FIND_DESC_ITEM(PSI_TABLE_OPR, PSI_TABLE_PAT);
                break;

            case PSI_TABLE_PMT:
                pPsiPktDecoder->pCur_Psi_Table_Desc = FIND_DESC_ITEM(PSI_TABLE_OPR, PSI_TABLE_PMT);
                break;

            case PSI_TABLE_SDT:
                pPsiPktDecoder->pCur_Psi_Table_Desc = FIND_DESC_ITEM(PSI_TABLE_OPR, PSI_TABLE_SDT);
                break;

            default:
                pPsiPktDecoder->pCur_Psi_Table_Desc = 0;
                break;
        }

        if( pPsiPktDecoder->pCur_Psi_Table_Desc &&
            pPsiPktDecoder->pCur_Psi_Table_Desc->attach_decoder )
        {
            switch( pPsiTableMbox->psi_table_mbox_arg.psi_table_id )
            {
                case PSI_TABLE_PAT:
                    pPsiTableMbox->psi_table_mbox_arg.argv.pat.pPsi_Table_Decoder = &pPsiPktDecoder->psi_table_decoder;
                    break;

                case PSI_TABLE_PMT:
                    pPsiTableMbox->psi_table_mbox_arg.argv.pmt.pPsi_Table_Decoder = &pPsiPktDecoder->psi_table_decoder;
                    break;

                case PSI_TABLE_SDT:
                    pPsiTableMbox->psi_table_mbox_arg.argv.sdt.pPsi_Table_Decoder = &pPsiPktDecoder->psi_table_decoder;
                    break;                    
            }

            result = pPsiPktDecoder->pCur_Psi_Table_Desc->attach_decoder(pPsiTableMbox, extraData);
        }
    }while(0);

    if( result != TSPA_ERR_OK )
    {
        tspa_msg_ex(TSPA_MSG_ERR, " %s() err 0x%x !", __FUNCTION__, result);
    }
    return result;
}

static uint32_t
_psi_pkt_detach_decoder(
    PSI_PKT_DECODER     *pPsiPktDecoder,
    PSI_TABLE_MBOX      *pPsiTableMbox,
    void                *extraData)
{
    uint32_t     result = TSPA_ERR_OK;

    do{
        switch( pPsiTableMbox->psi_table_mbox_arg.psi_table_id )
        {
            case PSI_TABLE_PAT:
                pPsiPktDecoder->pCur_Psi_Table_Desc = FIND_DESC_ITEM(PSI_TABLE_OPR, PSI_TABLE_PAT);
                break;

            case PSI_TABLE_PMT:
                pPsiPktDecoder->pCur_Psi_Table_Desc = FIND_DESC_ITEM(PSI_TABLE_OPR, PSI_TABLE_PMT);
                break;

            case PSI_TABLE_SDT:
                pPsiPktDecoder->pCur_Psi_Table_Desc = FIND_DESC_ITEM(PSI_TABLE_OPR, PSI_TABLE_SDT);
                break;

            default:
                pPsiPktDecoder->pCur_Psi_Table_Desc = 0;
                break;
        }

        if( pPsiPktDecoder->pCur_Psi_Table_Desc &&
            pPsiPktDecoder->pCur_Psi_Table_Desc->detach_decoder )
        {
            switch( pPsiTableMbox->psi_table_mbox_arg.psi_table_id )
            {
                case PSI_TABLE_PAT:
                    pPsiTableMbox->psi_table_mbox_arg.argv.pat.pPsi_Table_Decoder = &pPsiPktDecoder->psi_table_decoder;
                    break;

                case PSI_TABLE_PMT:
                    pPsiTableMbox->psi_table_mbox_arg.argv.pmt.pPsi_Table_Decoder = &pPsiPktDecoder->psi_table_decoder;
                    break;

                case PSI_TABLE_SDT:
                    pPsiTableMbox->psi_table_mbox_arg.argv.sdt.pPsi_Table_Decoder = &pPsiPktDecoder->psi_table_decoder;
                    break;                     
            }

            result = pPsiPktDecoder->pCur_Psi_Table_Desc->detach_decoder(pPsiTableMbox, extraData);
        }

    }while(0);

    if( result != TSPA_ERR_OK )
    {
        tspa_msg_ex(TSPA_MSG_ERR, " %s() err 0x%x !", __FUNCTION__, result);
    }
    return result;
}

PSI_DEMUX_SUBDECODER*
_psi_pkt_get_subtable_decoder(
    PSI_PRIV_SECT_DEMUX   *pPriv_sect_demux,
    PSI_TABLE_MBOX_ARG    *pPsi_table_mbox_arg,
    void                  *extraData)
{
    PSI_DEMUX_SUBDECODER    *pPsi_demux_subdecoder = 0;

    do{
        uint32_t    table_id, id;
        uint32_t    table_id_extension;

        if( !pPriv_sect_demux || !pPsi_table_mbox_arg )     break;

        table_id           = pPsi_table_mbox_arg->argv.sdt.table_id;
        table_id_extension = pPsi_table_mbox_arg->argv.sdt.table_id_extension;

        pPsi_demux_subdecoder = pPriv_sect_demux->ptFirstSubdecoder;

        id = ((table_id << 16) | table_id_extension);

        while( pPsi_demux_subdecoder )
        {
            if( pPsi_demux_subdecoder->id == id )
                break;

            pPsi_demux_subdecoder = pPsi_demux_subdecoder->ptNextSubdecoder;
        }
    }while(0);

    return pPsi_demux_subdecoder;
}

static void
_psi_pkt_demux_gather_section(
    PSI_TABLE_DECODER    *pDecoder,
    PSI_SECT             *pSect)
{
    do{
        PSI_PRIV_SECT_DEMUX     *pPriv_sect_demux = 0;
        PSI_DEMUX_SUBDECODER    *pPsi_demux_subdecoder = 0;

        if( !pDecoder || !pDecoder->pPrivDecoder || !pSect )
            break;

        pPriv_sect_demux = (PSI_PRIV_SECT_DEMUX*)pDecoder->pPrivDecoder;
        // Check if a subtable decoder is available
        if( pPriv_sect_demux->get_subtable_decoder )
        {
            PSI_TABLE_MBOX_ARG    mbox_arg = {0};

            mbox_arg.argv.sdt.table_id           = pSect->table_id;
            mbox_arg.argv.sdt.table_id_extension = pSect->table_id_extension;
            pPsi_demux_subdecoder =
                pPriv_sect_demux->get_subtable_decoder(pPriv_sect_demux, &mbox_arg, 0);
        }

        if( !pPsi_demux_subdecoder )
        {
            // Tell the application we found a new subtable, so that it may attach a
            // subtable decoder
            if( pPriv_sect_demux->attach_psi_table_decoder )
            {
                PSI_TABLE_MBOX_ARG    mbox_arg = {0};

                mbox_arg.argv.sdt.table_id           = pSect->table_id;
                mbox_arg.argv.sdt.table_id_extension = pSect->table_id_extension;
                pPriv_sect_demux->attach_psi_table_decoder(pPriv_sect_demux, &mbox_arg, 0);
            }

            // Check if a new subtable decoder is available
            if( pPriv_sect_demux->get_subtable_decoder )
            {
                PSI_TABLE_MBOX_ARG    mbox_arg = {0};

                mbox_arg.argv.sdt.table_id           = pSect->table_id;
                mbox_arg.argv.sdt.table_id_extension = pSect->table_id_extension;
                pPsi_demux_subdecoder =
                    pPriv_sect_demux->get_subtable_decoder(pPriv_sect_demux, &mbox_arg, 0);
            }
        }

        if( !pPsi_demux_subdecoder )
        {
            psi_section_destroy(pDecoder->allocId, pDecoder->pf_sect_free, pSect);
            break;
        }

        pPsi_demux_subdecoder->gather_sect(pPriv_sect_demux->pPsi_table_decoder,
                                           pPsi_demux_subdecoder->ptPrivDecoder,
                                           pSect);
    }while(0);

    return;
}

static void
_psi_pkt_attach_subtable_deocder(
    PSI_PRIV_SECT_DEMUX   *pPriv_sect_demux,
    PSI_TABLE_MBOX_ARG    *pPsi_table_mbox_arg,
    void                  *extraData)
{
    PSI_PKT_DECODER     *pPsiPktDecoder = 0;
    PSI_TABLE_MBOX_ARG  *pPsi_pkt_dec_mbox_arg = 0;
    uint32_t            table_id = pPsi_table_mbox_arg->argv.sdt.table_id;

    switch( table_id )
    {
        case SDT_ACTUAL_TABLE_ID:
            pPsi_pkt_dec_mbox_arg = &pPriv_sect_demux->psi_pkt_dec_mbox.psi_table_mbox_arg;
            pPsiPktDecoder        = (PSI_PKT_DECODER*)pPsi_pkt_dec_mbox_arg->argv.sdt.pPsiPktDecoder;

            pPsi_pkt_dec_mbox_arg->argv.sdt.table_id           = pPsi_table_mbox_arg->argv.sdt.table_id;
            pPsi_pkt_dec_mbox_arg->argv.sdt.table_id_extension = pPsi_table_mbox_arg->argv.sdt.table_id_extension;
            _psi_pkt_attach_decoder(pPsiPktDecoder, &pPriv_sect_demux->psi_pkt_dec_mbox, extraData);
            break;

        case NIT_ACTUAL_TABLE_ID:                           break;
        case NIT_OTHER_TABLE_ID:                            break;
        case EIT_ACTUAL_PRESENT_FOLLOWING_EVENT_TABLE_ID:   break;
        case TDT_TABLE_ID:                                  break;
        case TOT_TABLE_ID:                                  break;
        default:
            if( EIT_ACTUAL_SCHEDULE_EVENT_MIN_TABLE_ID <= table_id &&
                table_id <= EIT_ACTIAL_SCHEDULE_EVENT_MAX_TABLE_ID )
            {
            }
            break;
    }

    return;
}

static uint32_t
_psi_pkt_attach_priv_sect_demux(
    PSI_PKT_DECODER     *pPsiPktDecoder,
    PSI_TABLE_MBOX      *pPsiTableMbox,
    void                *extraData)
{
    uint32_t     result = TSPA_ERR_OK;

    do{
        PSI_TABLE_DECODER       *pPsi_table_decoder = 0;
        PSI_PRIV_SECT_DEMUX     *pPriv_sect_demux = 0;

        pPsi_table_decoder = &pPsiPktDecoder->psi_table_decoder;

        pPriv_sect_demux = tspa_malloc(sizeof(PSI_PRIV_SECT_DEMUX));
        if( !pPriv_sect_demux )
        {
            tspa_msg_ex(TSPA_ERR_ALLOCATE_FAIL, "err, allocate fail !!");
            result = TSPA_ERR_NULL_POINTER;
            break;
        }

        //-----------------------------------
        // PSI decoder initilization
        pPsi_table_decoder->pf_psi_sect_handler = _psi_pkt_demux_gather_section;
        pPsi_table_decoder->pPrivDecoder        = (void*)pPriv_sect_demux;
        pPsi_table_decoder->sectMaxSize         = MAX_SIZE_OF_PRIVATE_SECTION;
        pPsi_table_decoder->bDiscontinuity      = true;
        pPsi_table_decoder->allocId             = 0;
        pPsi_table_decoder->pf_sect_alloc       = _psi_pkt_heap_func;
        pPsi_table_decoder->pf_sect_free        = _psi_pkt_free_func;

        //-----------------------------------
        // Subtable demux initilization
        memset(pPriv_sect_demux, 0x0, sizeof(PSI_PRIV_SECT_DEMUX));
        pPriv_sect_demux->pPsi_table_decoder       = pPsi_table_decoder;
        pPriv_sect_demux->attach_psi_table_decoder = _psi_pkt_attach_subtable_deocder;
        pPriv_sect_demux->psi_pkt_dec_mbox         = *(pPsiTableMbox);
        pPriv_sect_demux->get_subtable_decoder     = _psi_pkt_get_subtable_decoder;

        pPriv_sect_demux->psi_pkt_dec_mbox.psi_table_mbox_arg.argv.sdt.pPriv_sect_demux = pPriv_sect_demux;
    }while(0);

    if( result != TSPA_ERR_OK )
    {
        tspa_msg_ex(TSPA_MSG_ERR, " %s() err 0x%x !", __FUNCTION__, result);
    }
    return result;
}

static uint32_t
_psi_pkt_detach_priv_sect_demux(
    PSI_PKT_DECODER     *pPsiPktDecoder,
    PSI_TABLE_MBOX      *pPsiTableMbox,
    void                *extraData)
{
    uint32_t     result = TSPA_ERR_OK;

    do{
        PSI_TABLE_DECODER       *pPsi_table_decoder = &pPsiPktDecoder->psi_table_decoder;
        PSI_PRIV_SECT_DEMUX     *pPriv_sect_demux = 0;

        if( !pPsiTableMbox )    break;

        pPriv_sect_demux = (PSI_PRIV_SECT_DEMUX*)pPsi_table_decoder->pPrivDecoder;
        if( pPriv_sect_demux )
        {
            PSI_DEMUX_SUBDECODER    *ptSubdecoder;
            PSI_DEMUX_SUBDECODER    *ptSubdecoderTemp;

            ptSubdecoder = pPriv_sect_demux->ptFirstSubdecoder;
            while( ptSubdecoder )
            {
                ptSubdecoderTemp = ptSubdecoder;
                ptSubdecoder     = ptSubdecoder->ptNextSubdecoder;
                if( ptSubdecoderTemp->detach_subdecoder )
                {
                    PSI_TABLE_MBOX_ARG    mbox_arg = {0};

                    mbox_arg.argv.sdt.table_id           = (ptSubdecoderTemp->id >> 16) & 0xFFFF;
                    mbox_arg.argv.sdt.table_id_extension = ptSubdecoderTemp->id & 0xFFFF;
                    ptSubdecoderTemp->detach_subdecoder(pPriv_sect_demux, &mbox_arg, extraData);
                }
                else
                {
                    free(ptSubdecoderTemp);
                }
            }

            free(pPriv_sect_demux);
        }

        if( pPsi_table_decoder->ptCurSect )
        {
            psi_section_destroy(pPsi_table_decoder->allocId,
                                pPsi_table_decoder->pf_sect_free, pPsi_table_decoder->ptCurSect);
        }
    }while(0);

    if( result != TSPA_ERR_OK )
    {
        tspa_msg_ex(TSPA_MSG_ERR, " %s() err 0x%x !", __FUNCTION__, result);
    }
    return result;
}

//=============================================================================
//                Public Function Definition
//=============================================================================
void
psi_pkt_register_all_decoder(
    void)
{
    static int bInitialized = 0;

    if( bInitialized )
        return;
    bInitialized = 1;

    REGISTER_ITEM(PSI_TABLE_OPR, PAT_DECODER, pat_decoder);
    REGISTER_ITEM(PSI_TABLE_OPR, PMT_DECODER, pmt_decoder);
    REGISTER_ITEM(PSI_TABLE_OPR, SDT_DECODER, sdt_decoder);
}

uint32_t
psi_pkt_create_decoder(
    PSI_PKT_DECODER     **ppPsiPktDecoder,
    PSI_TABLE_MBOX      *pPsiTableMbox,
    void                *extraData)
{
    uint32_t            result = TSPA_ERR_OK;
    PSI_PKT_DECODER     *pPsiPktDecoder = 0;
    do{
        if( !ppPsiPktDecoder )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "Null pointer !!");
            result = TSPA_ERR_NULL_POINTER;
            break;
        }

        pPsiPktDecoder = tspa_malloc(sizeof(PSI_PKT_DECODER));
        if( !pPsiPktDecoder )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "err, allocate fail !!");
            result = TSPA_ERR_ALLOCATE_FAIL;
            break;
        }

        memset(pPsiPktDecoder, 0x0, sizeof(PSI_PKT_DECODER));

        switch( pPsiTableMbox->psi_table_mbox_arg.psi_table_id )
        {
            case PSI_TABLE_PAT:
            case PSI_TABLE_PMT:
                result = _psi_pkt_attach_decoder(pPsiPktDecoder, pPsiTableMbox, extraData);
                break;

            case PSI_TABLE_NIT:
            case PSI_TABLE_EIT:
            case PSI_TABLE_SDT:
                /**
                 * pPsiPktDecoder
                 *     ->psi_table_decoder
                 *         ->psi_priv_sect_demux
                 *             ->psi_demux_subdecoder
                 *                 ->private psi_sdt_deocder
                 **/
                pPsiTableMbox->psi_table_mbox_arg.argv.sdt.pPsiPktDecoder     = pPsiPktDecoder;
                pPsiTableMbox->psi_table_mbox_arg.argv.sdt.pPsi_Table_Decoder = &pPsiPktDecoder->psi_table_decoder;
                result = _psi_pkt_attach_priv_sect_demux(pPsiPktDecoder, pPsiTableMbox, extraData);
                break;

            default:        break;
        }

        (*ppPsiPktDecoder) = (result == TSPA_ERR_OK) ? pPsiPktDecoder : 0;

    }while(0);

    if( result != TSPA_ERR_OK )
    {
        if( pPsiPktDecoder )        free(pPsiPktDecoder);
        tspa_msg_ex(TSPA_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }

    return result;
}

uint32_t
psi_pkt_destroy_decoder(
    PSI_PKT_DECODER     **ppPsiPktDecoder,
    PSI_TABLE_MBOX      *pPsiTableMbox,
    void                *extraData)
{
    uint32_t    result = TSPA_ERR_OK;

    do{
        PSI_PKT_DECODER     *pPsiPktDecoder = (*ppPsiPktDecoder);

        if( !ppPsiPktDecoder || !pPsiPktDecoder )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "Null pointer !!");
            result = TSPA_ERR_NULL_POINTER;
            break;
        }

        switch( pPsiTableMbox->psi_table_mbox_arg.psi_table_id )
        {
            case PSI_TABLE_PAT:
            case PSI_TABLE_PMT:
                _psi_pkt_detach_decoder(pPsiPktDecoder, pPsiTableMbox, extraData);
                break;

            case PSI_TABLE_NIT:
            case PSI_TABLE_EIT:
            case PSI_TABLE_SDT:
                _psi_pkt_detach_priv_sect_demux(pPsiPktDecoder, pPsiTableMbox, extraData);
                break;

            default:        break;
        }

        free(pPsiPktDecoder);

        (*ppPsiPktDecoder) = 0;
    }while(0);

    if( result != TSPA_ERR_OK )
    {
        tspa_msg_ex(TSPA_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    return result;
}

void
psi_pkt_decode(
    PSI_PKT_DECODER     *pPsiPktDecoder,
    uint8_t             *pPktData)
{
    do{
        uint32_t    sync_byte = 0;
        uint32_t    payload_unit_start_indicator = 0;
        uint32_t    adaptation_field_control = 0;
        uint32_t    continuity_counter = 0;
        uint32_t    adaptation_field_length = 0;
        uint32_t    pointer_field = 0;
        int         availableByte = 0;

        uint8_t     *pPayloadAddr = 0;
        uint8_t     *pBeginSectAddr = 0;
        uint8_t     *pAdapationFieldAddr = 0;
        uint8_t     *pDataAddr = 0;

        PSI_SECT            *pSect = 0;
        PSI_TABLE_DECODER   *pPsi_decoder = 0;

        if( !pPsiPktDecoder || !pPktData )      break;

        sync_byte = pPktData[0];

        payload_unit_start_indicator = (pPktData[1] & 0x40) >> 6;
        adaptation_field_control     = (pPktData[3] & 0x30) >> 4;
        continuity_counter           = pPktData[3] & 0x0F;
        pDataAddr                    = &pPktData[4];

        pPsi_decoder = &pPsiPktDecoder->psi_table_decoder;
        //--------------------------------
        // 1. Check sync_byte
        // 2. Check continuity_counter
        if( (sync_byte != VALID_SYNC_BYTE) ||
            ((continuity_counter == pPsi_decoder->continuity_counter) &&
             (pPsi_decoder->bDiscontinuity == false)) )
            break;

        if( continuity_counter != ((pPsi_decoder->continuity_counter + 1) & CONTINUITY_COUNTER_MASK) )
        {
            // Discontinuity
            pPsi_decoder->bDiscontinuity = true;

            if( pPsi_decoder->ptCurSect )
            {
                psi_section_destroy(pPsi_decoder->allocId,
                                    pPsi_decoder->pf_sect_free, pPsi_decoder->ptCurSect);
                pPsi_decoder->ptCurSect = 0;
            }
        }

        pPsi_decoder->continuity_counter = continuity_counter;
        //--------------------------------
        // 3. Check adaptation_field_control
            // Payload doesn't exist. That is All stuffing bytes behide,
            // therefore, simply return.
        if( (adaptation_field_control & PAYLOAD_EXIST) != PAYLOAD_EXIST )   break;

        if( (adaptation_field_control & ADAPTATION_FIELD_EXIST) == ADAPTATION_FIELD_EXIST )
        {
            pAdapationFieldAddr = pDataAddr;

            adaptation_field_length = pAdapationFieldAddr[0];
            pPayloadAddr = (pAdapationFieldAddr + 1 + adaptation_field_length);
        }
        else
            pPayloadAddr = pDataAddr;

        //--------------------------------
        // 4. Check payload_unit_start_indicator
        if( payload_unit_start_indicator )
        {
            // pointer_field exist, get a new begin section address through it
            pointer_field = pPayloadAddr[0];
            pPayloadAddr += 1;
            pBeginSectAddr = (pPayloadAddr + pointer_field);
        }

        pSect = pPsi_decoder->ptCurSect;
        //--------------------------------
        // 5. Check whether current section is under process. If no
        //    Create a new section handle the packet.
        if( !pSect )
        {
            if( !pBeginSectAddr )       break;

            pSect = psi_section_create(pPsi_decoder->allocId,
                                       pPsi_decoder->pf_sect_alloc, pPsi_decoder->sectMaxSize);

            pPsi_decoder->ptCurSect = pSect;
            if( !pSect )        break;

            pPayloadAddr = pBeginSectAddr;

            pBeginSectAddr = 0;

            pPsi_decoder->needByte = PRIVATE_SECTION_HEADER_SIZE;
            pPsi_decoder->bCompleteHeader = false;
        }

        availableByte = (pPktData + TS_PACKET_BYTE_NUMBER) - pPayloadAddr;
        //--------------------------------
        // 6. Use loop to deal with the rest packet decode issue
        while( availableByte > 0 )
        {
            if( availableByte < (int)pPsi_decoder->needByte )
            {
                memcpy(pSect->pPayloadEndAddress, pPayloadAddr, availableByte);
                pSect->pPayloadEndAddress += availableByte;
                pPsi_decoder->needByte    -= availableByte;
                availableByte = 0;
            }
            else
            {
                memcpy(pSect->pPayloadEndAddress, pPayloadAddr, pPsi_decoder->needByte);
                pPayloadAddr              += pPsi_decoder->needByte;
                pSect->pPayloadEndAddress += pPsi_decoder->needByte;
                availableByte             -= pPsi_decoder->needByte;

                if( pPsi_decoder->bCompleteHeader == false )
                {
                    pPsi_decoder->bCompleteHeader = true;

                    pPsi_decoder->needByte = pSect->section_length
                                           = (uint32_t)((pSect->pData[1] & 0xF) << 8 | pSect->pData[2]);

                    if( pPsi_decoder->needByte > (pPsi_decoder->sectMaxSize - PRIVATE_SECTION_HEADER_SIZE) )
                    {
                        // PSI section is too long
                        psi_section_destroy(pPsi_decoder->allocId,
                                            pPsi_decoder->pf_sect_free, pSect);
                        pPsi_decoder->ptCurSect = 0;

                        // If there is a new section not being handled then go forward in the packet
                        if( !pBeginSectAddr )
                        {
                            availableByte = 0;
                            continue;
                        }

                        pSect = psi_section_create(pPsi_decoder->allocId,
                                                   pPsi_decoder->pf_sect_alloc, pPsi_decoder->sectMaxSize);

                        pPsi_decoder->ptCurSect = pSect;
                        if( !pSect )
                        {
                            availableByte = 0;
                            continue;
                        }

                        pPayloadAddr = pBeginSectAddr;

                        pBeginSectAddr = 0;

                        pPsi_decoder->needByte = PRIVATE_SECTION_HEADER_SIZE;
                        pPsi_decoder->bCompleteHeader = false;

                        availableByte = (pPktData + TS_PACKET_BYTE_NUMBER) - pPayloadAddr;
                    }
                }
                else
                {
                    _psi_pkt_decode_section(pPsi_decoder, &pSect);
                    if( pSect )
                    {
                        // Section is valid, callback to next step
                        pPsi_decoder->pf_psi_sect_handler(pPsi_decoder, pSect);
                    }

                    pPsi_decoder->ptCurSect = 0;

                    // A TS packet may contain any number of sections, only the first
                    // new one is flagged by the pointer_field. If the next payload
                    // byte isn't 0xff then a new section starts.
                    if( (pBeginSectAddr == 0) && availableByte && (*pPayloadAddr != STUFF_BYTE) )
                        pBeginSectAddr = pPayloadAddr;

                    if( !pBeginSectAddr )
                    {
                        availableByte = 0;
                        continue;
                    }

                    pSect = psi_section_create(pPsi_decoder->allocId,
                                               pPsi_decoder->pf_sect_alloc, pPsi_decoder->sectMaxSize);
                    pPsi_decoder->ptCurSect = pSect;
                    if( !pSect )
                    {
                        availableByte = 0;
                        continue;
                    }

                    pPayloadAddr = pBeginSectAddr;

                    pBeginSectAddr = 0;

                    pPsi_decoder->needByte = PRIVATE_SECTION_HEADER_SIZE;
                    pPsi_decoder->bCompleteHeader = false;

                    availableByte = (pPktData + TS_PACKET_BYTE_NUMBER) - pPayloadAddr;
                }
            }
        }
    }while(0);

    return;
}


void
psi_pkt_clear_table(
    PSI_TABLE_MBOX      *pPsiTableMbox,
    void                *extraData)
{
    uint32_t     result = TSPA_ERR_OK;

    do{
        PSI_TABLE_OPR          *pCur_Psi_Table_Desc = 0;

        switch( pPsiTableMbox->psi_table_mbox_arg.psi_table_id )
        {
            case PSI_TABLE_PAT:
                pCur_Psi_Table_Desc = FIND_DESC_ITEM(PSI_TABLE_OPR, PSI_TABLE_PAT);
                break;

            case PSI_TABLE_PMT:
                pCur_Psi_Table_Desc = FIND_DESC_ITEM(PSI_TABLE_OPR, PSI_TABLE_PMT);
                break;

            case PSI_TABLE_SDT:
                pCur_Psi_Table_Desc = FIND_DESC_ITEM(PSI_TABLE_OPR, PSI_TABLE_SDT);
                break;
            default:
                pCur_Psi_Table_Desc = 0;
                break;
        }

        if( pCur_Psi_Table_Desc &&
            pCur_Psi_Table_Desc->clear_table )
        {
            result = pCur_Psi_Table_Desc->clear_table(pPsiTableMbox, extraData);
        }

    }while(0);

    if( result != TSPA_ERR_OK )
    {
        tspa_msg_ex(TSPA_MSG_ERR, " %s() err 0x%x !", __FUNCTION__, result);
    }
}


