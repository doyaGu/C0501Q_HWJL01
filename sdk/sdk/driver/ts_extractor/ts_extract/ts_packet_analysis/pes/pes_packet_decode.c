


#include "register_template.h"
#include "pes_stream_cfg.h"
#include "ts_packet_analysis.h"

//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
DEFINE_REGISTER_TEMPLATE(PES_STREAM_OPR, PES_STREAM_ID);
//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
static PES_INFO*
_pes_pkt_create_pes_info(
    PES_STREAM_DECODER  *pPes_stream_decoder,
    uint32_t            pid)
{
    PES_INFO    *pPesInfo = 0;

    pPesInfo = &pPes_stream_decoder->tBuildingPes;

    if( pPesInfo )
    {
        memset(pPesInfo, 0x0, sizeof(PES_INFO));
        pPesInfo->elementary_PID = pid;
    }

    return pPesInfo;
}

static uint32_t
_pes_pkt_destroy_pes(
    PES_INFO    *pPes_info,
    void        *extraData)
{
    uint32_t     result = TSPA_ERR_OK;

    do{
#if (ENABLE_PES_COPY_PARSE)
        if( pPes_info->pData )
        {
            free(pPes_info->pData);
            pPes_info->pData = 0;
        }
#endif
    }while(0);

    if( result != TSPA_ERR_OK )
    {
        tspa_msg_ex(TSPA_MSG_ERR, " %s() err 0x%x !", __FUNCTION__, result);
    }
    return result;
}

static uint32_t
_pes_pkt_attach_decoder(
    PES_PKT_DECODER     *pPesPktDecoder,
    PES_STREAM_MBOX     *pPesStreamMbox,
    void                *extraData)
{
    uint32_t     result = TSPA_ERR_OK;

    do{
        switch( pPesStreamMbox->pes_stream_mbox_arg.pes_stream_id )
        {
            case PES_STREAM_VIDEO:
                pPesPktDecoder->pCur_Pes_Stream_Desc = FIND_DESC_ITEM(PES_STREAM_OPR, PES_STREAM_VIDEO);
                break;

            case PES_STREAM_AUDIO:
                pPesPktDecoder->pCur_Pes_Stream_Desc = FIND_DESC_ITEM(PES_STREAM_OPR, PES_STREAM_AUDIO);
                break;

            case PES_STREAM_TELETEXT:
                pPesPktDecoder->pCur_Pes_Stream_Desc = FIND_DESC_ITEM(PES_STREAM_OPR, PES_STREAM_TELETEXT);
                break;

            case PES_STREAM_SUBTITLE:
                pPesPktDecoder->pCur_Pes_Stream_Desc = FIND_DESC_ITEM(PES_STREAM_OPR, PES_STREAM_SUBTITLE);
                break;

            default:
                pPesPktDecoder->pCur_Pes_Stream_Desc = 0;
                break;
        }

        if( pPesPktDecoder->pCur_Pes_Stream_Desc )
        {
            PES_STREAM_DECODER      *pPes_stream_decoder = &pPesPktDecoder->pes_stream_decoder;
            uint32_t                elementary_PID = 0;

            switch( pPesStreamMbox->pes_stream_mbox_arg.pes_stream_id )
            {
                case PES_STREAM_VIDEO:
                    pPesStreamMbox->pes_stream_mbox_arg.arg.v.pPes_stream_decoder = pPes_stream_decoder;
                    pPes_stream_decoder->elementary_PID = pPesStreamMbox->pes_stream_mbox_arg.arg.v.esPID;
                    break;

                case PES_STREAM_AUDIO:
                    pPesStreamMbox->pes_stream_mbox_arg.arg.a.pPes_stream_decoder = pPes_stream_decoder;
                    pPes_stream_decoder->elementary_PID = pPesStreamMbox->pes_stream_mbox_arg.arg.a.esPID;
                    break;

                case PES_STREAM_TELETEXT:
                    pPesStreamMbox->pes_stream_mbox_arg.arg.t.pPes_stream_decoder = pPes_stream_decoder;
                    pPes_stream_decoder->elementary_PID = pPesStreamMbox->pes_stream_mbox_arg.arg.t.esPID;
                    break;

                case PES_STREAM_SUBTITLE:
                    pPesStreamMbox->pes_stream_mbox_arg.arg.s.pPes_stream_decoder = pPes_stream_decoder;
                    pPes_stream_decoder->elementary_PID = pPesStreamMbox->pes_stream_mbox_arg.arg.s.esPID;
                    break;
            }

            pPes_stream_decoder->pes_pkt_dec_mbox = (*pPesStreamMbox);

            if( pPesPktDecoder->pCur_Pes_Stream_Desc->init )
                pPesPktDecoder->pCur_Pes_Stream_Desc->init(pPesStreamMbox, extraData);

            pPes_stream_decoder->bFirstPacket = true;
        }
    }while(0);

    if( result != TSPA_ERR_OK )
    {
        tspa_msg_ex(TSPA_MSG_ERR, " %s() err 0x%x !", __FUNCTION__, result);
    }
    return result;
}

static uint32_t
_pes_pkt_detach_decoder(
    PES_PKT_DECODER     *pPesPktDecoder,
    PES_STREAM_MBOX     *pPesStreamMbox,
    void                *extraData)
{
    uint32_t     result = TSPA_ERR_OK;

    do{
        switch( pPesStreamMbox->pes_stream_mbox_arg.pes_stream_id )
        {
            case PES_STREAM_VIDEO:
                pPesPktDecoder->pCur_Pes_Stream_Desc = FIND_DESC_ITEM(PES_STREAM_OPR, PES_STREAM_VIDEO);
                break;

            case PES_STREAM_AUDIO:
                pPesPktDecoder->pCur_Pes_Stream_Desc = FIND_DESC_ITEM(PES_STREAM_OPR, PES_STREAM_AUDIO);
                break;

            case PES_STREAM_TELETEXT:
                pPesPktDecoder->pCur_Pes_Stream_Desc = FIND_DESC_ITEM(PES_STREAM_OPR, PES_STREAM_TELETEXT);
                break;

            case PES_STREAM_SUBTITLE:
                pPesPktDecoder->pCur_Pes_Stream_Desc = FIND_DESC_ITEM(PES_STREAM_OPR, PES_STREAM_SUBTITLE);
                break;

            default:
                pPesPktDecoder->pCur_Pes_Stream_Desc = 0;
                break;
        }

        if( pPesPktDecoder->pCur_Pes_Stream_Desc )
        {
            PES_STREAM_DECODER      *pPes_stream_decoder = &pPesPktDecoder->pes_stream_decoder;
            /**
             * In detach function, why need to assign value ????
             **/

            switch( pPesStreamMbox->pes_stream_mbox_arg.pes_stream_id )
            {
                case PES_STREAM_VIDEO:
                    pPesStreamMbox->pes_stream_mbox_arg.arg.v.pPes_stream_decoder = pPes_stream_decoder;
                    break;

                case PES_STREAM_AUDIO:
                    pPesStreamMbox->pes_stream_mbox_arg.arg.a.pPes_stream_decoder = pPes_stream_decoder;
                    break;

                case PES_STREAM_TELETEXT:
                    pPesStreamMbox->pes_stream_mbox_arg.arg.t.pPes_stream_decoder = pPes_stream_decoder;
                    break;

                case PES_STREAM_SUBTITLE:
                    pPesStreamMbox->pes_stream_mbox_arg.arg.s.pPes_stream_decoder = pPes_stream_decoder;
                    break;
            }

            // memset(&pPes_stream_decoder->pes_pkt_dec_mbox, 0x0, sizeof(PES_STREAM_MBOX));

            if( pPesPktDecoder->pCur_Pes_Stream_Desc->deinit )
                pPesPktDecoder->pCur_Pes_Stream_Desc->deinit(pPesStreamMbox, extraData);

            if( pPes_stream_decoder->ptBuildingPes )
            {
                _pes_pkt_destroy_pes(pPes_stream_decoder->ptBuildingPes, extraData);
                pPes_stream_decoder->ptBuildingPes = 0;
            }
        }
    }while(0);

    if( result != TSPA_ERR_OK )
    {
        tspa_msg_ex(TSPA_MSG_ERR, " %s() err 0x%x !", __FUNCTION__, result);
    }
    return result;
}

static void
_pes_pkt_parse_pes_prior_hdr(
    PES_STREAM_DECODER  *pPes_stream_decoder)
{
    if( pPes_stream_decoder->ptBuildingPes )
    {
        pPes_stream_decoder->ptBuildingPes->packet_start_code_prefix =
            pPes_stream_decoder->pPriorHeader[0] << 16 |
            pPes_stream_decoder->pPriorHeader[1] << 8 |
            pPes_stream_decoder->pPriorHeader[2];

        pPes_stream_decoder->ptBuildingPes->stream_id =
            pPes_stream_decoder->pPriorHeader[3];

        pPes_stream_decoder->ptBuildingPes->PES_packet_length =
            pPes_stream_decoder->pPriorHeader[4] << 8 |
            pPes_stream_decoder->pPriorHeader[5];
    }

    return;
}

static void
_pes_pkt_parse_pes(
    PES_STREAM_DECODER  *pPes_stream_decoder)
{
    do{
        bool      bSkip = false;
        uint32_t  PTS_DTS_flags = 0;
        uint32_t  PES_header_data_length = 0;

        uint8_t   *pData = 0;
        uint8_t   *pPayloadStartAddr = 0;
        uint8_t   *pPtsStartAddr = 0;
        uint8_t   *pDtsStartAddr = 0;

        if( !pPes_stream_decoder || !pPes_stream_decoder->ptBuildingPes )
            break;

        // Invalid start_code_prefix.
        if( pPes_stream_decoder->ptBuildingPes->packet_start_code_prefix != PES_START_CODE_PREFIX )
        {
            _pes_pkt_destroy_pes(pPes_stream_decoder->ptBuildingPes, 0);
            pPes_stream_decoder->ptBuildingPes = 0;
            break;
        }

        // The operation of segment chain and buffer copy is failed.
        pData = pPes_stream_decoder->ptBuildingPes->pData;

        // The information of partial header section is retrieved already.
        // (packet_start_code_prefix, stream_id, and PES_packet_length).
        // Therefore, we jump the address behide the field PES_packet_length.
        pData += PES_PRIOR_HEADER_SIZE;

        switch( pPes_stream_decoder->ptBuildingPes->stream_id )
        {
            case PROGRAM_STREAM_MAP_ID:
            case PADDING_STREAM_ID:
            case PRIVATE_STREAM_2_ID:
            case ECM_ID:
            case EMM_ID:
            case PROGRAM_STREAM_DIRECTORY_ID:
            case DSMCC_STREAM_ID:
            case ITU_H222_1_TYPE_E_STREAM_ID:
                _pes_pkt_destroy_pes(pPes_stream_decoder->ptBuildingPes, 0); // release pes header info and data
                pPes_stream_decoder->ptBuildingPes = 0;
                bSkip = true;
                break;

            default:
                // Currently we only interests the PTS_DTS_flags and
                // PES_header_data_length

                // '10'                         2 bits
                // PES_scrambling_control       2 bits
                // PES_priority                 1 bit
                // data_alignment_indicator     1 bit
                // Copyright                    1 bit
                // original_or_copy             1 bit
                // PTS_DTS_flags                2 bits
                // ESCR_flag                    1 bit
                // ES_rate_flag                 1 bit
                // DSM_trick_mode_flag          1 bit
                // Additional_copy_info_flag    1 bit
                // PES_CRC_flag                 1 bit
                // PES_extension_flag           1 bit
                // BitStreamKit_SkipBits(&tBitStream, 16);

                PES_header_data_length = pData[2];
                pPayloadStartAddr      = &pData[3] + PES_header_data_length;

                pPes_stream_decoder->ptBuildingPes->PES_header_data_length = PES_header_data_length;
                break;
        }

        if( bSkip == true )     break;

        if( pPes_stream_decoder->bDiscontinuity )
            pPes_stream_decoder->ptBuildingPes->errorFlag |= PES_DISCONTINUITY;

        if( pPes_stream_decoder->bErrorIndicator )
            pPes_stream_decoder->ptBuildingPes->errorFlag |= PES_TRANSPORT_ERROR_INDICATOR;

        pPes_stream_decoder->ptBuildingPes->pPayloadStartAddress = pPayloadStartAddr;

        // report to user layer
        if( pPes_stream_decoder->pes_pkt_dec_mbox.func )
        {
            PES_STREAM_MBOX_ARG  *pPes_arg = 0;

            pPes_arg = &pPes_stream_decoder->pes_pkt_dec_mbox.pes_stream_mbox_arg;
            switch( pPes_arg->pes_stream_id )
            {
                case PES_STREAM_VIDEO:
                    pPes_arg->arg.v.pPes_info = pPes_stream_decoder->ptBuildingPes;
                    break;

                case PES_STREAM_AUDIO:
                    pPes_arg->arg.a.pPes_info = pPes_stream_decoder->ptBuildingPes;
                    break;

                case PES_STREAM_TELETEXT:
                    pPes_arg->arg.t.pPes_info = pPes_stream_decoder->ptBuildingPes;
                    break;

                case PES_STREAM_SUBTITLE:
                    pPes_arg->arg.s.pPes_info = pPes_stream_decoder->ptBuildingPes;
                    break;
            }

            pPes_stream_decoder->pes_pkt_dec_mbox.func(pPes_arg, 0);
        }

        // do destroy pes header info
        if( pPes_stream_decoder->ptBuildingPes )
        {
            _pes_pkt_destroy_pes(pPes_stream_decoder->ptBuildingPes, 0);
            pPes_stream_decoder->ptBuildingPes = 0;
        }

    }while(0);

    return;
}

static uint32_t
_pes_pkt_collect_pes_data_bytes(
    PES_PKT_DECODER     *pPesPktDecoder,
    uint32_t            size,
    uint8_t             *pDataSrc)
{
    PES_STREAM_DECODER      *pPes_stream_decoder = &pPesPktDecoder->pes_stream_decoder;
    uint32_t                remainSize = 0;
    PES_STREAM_MBOX_ARG     *pPes_arg = 0;

    // parsing pes payload
    if( pPesPktDecoder->pCur_Pes_Stream_Desc &&
        pPesPktDecoder->pCur_Pes_Stream_Desc->proc )
    {
        PES_STREAM_MBOX     pesMbox = {0};

        pPes_arg = &pesMbox.pes_stream_mbox_arg;

        switch( pPes_stream_decoder->pes_pkt_dec_mbox.pes_stream_mbox_arg.pes_stream_id )
        {
            case PES_STREAM_VIDEO:
                pPes_arg->pes_stream_id = PES_STREAM_VIDEO;
                break;

            case PES_STREAM_AUDIO:
                pPes_arg->pes_stream_id = PES_STREAM_AUDIO;
                break;

            case PES_STREAM_TELETEXT:
                pPes_arg->pes_stream_id = PES_STREAM_TELETEXT;
                break;

            case PES_STREAM_SUBTITLE:
                pPes_arg->pes_stream_id = PES_STREAM_SUBTITLE;
                break;
        }

        pPes_arg->arg.v.pPes_stream_decoder = pPes_stream_decoder;
        pPes_arg->arg.v.pData               = pDataSrc;
        pPes_arg->arg.v.data_size           = size;

        pPesPktDecoder->pCur_Pes_Stream_Desc->proc(&pesMbox, 0);
        switch( pPes_stream_decoder->pes_pkt_dec_mbox.pes_stream_mbox_arg.pes_stream_id )
        {
            case PES_STREAM_VIDEO:
                pPes_stream_decoder->pes_pkt_dec_mbox.pes_stream_mbox_arg.arg.v.bGetResolution 
                        = pPes_arg->arg.v.bGetResolution;
                break;

            case PES_STREAM_AUDIO:
            case PES_STREAM_TELETEXT:
            case PES_STREAM_SUBTITLE:
                break;
        }
    }

    pPes_stream_decoder->ptBuildingPes->gatherSize += size;

    return 0;
}

static void
_pes_pkt_gather_pes_seg(
    PES_PKT_DECODER     *pPesPktDecoder,
    int                 availableByte,
    uint8_t             *pPayloadAddr)
{
    uint32_t            remainingByte = 0;
    PES_STREAM_DECODER  *pPes_stream_decoder = &pPesPktDecoder->pes_stream_decoder;

    // The rest of TS packet is greater than 0
    while( availableByte > 0 )
    {
        // We have to collect the partial PES header section to help us
        // to evaluate the integrity of a PES packet. Therefore, we have
        // to get the first 6 bytes data (till PES_packet_length).
        if( pPes_stream_decoder->priorHeaderReadByte < PES_PRIOR_HEADER_SIZE )
        {
            if( availableByte < (int)(PES_PRIOR_HEADER_SIZE - pPes_stream_decoder->priorHeaderReadByte) )
            {
                // Copy the data to the special array which keeps the first 6 bytes
                // of PES header only.
                memcpy(&(pPes_stream_decoder->pPriorHeader[pPes_stream_decoder->priorHeaderReadByte]),
                       pPayloadAddr, availableByte);

                // Create a new segment which keeps the data of part of PES packet.
                _pes_pkt_collect_pes_data_bytes(pPesPktDecoder, availableByte, pPayloadAddr);

                pPes_stream_decoder->priorHeaderReadByte += availableByte;
                break;
            }
            else
            {
                // First, handle prior header
                memcpy(&(pPes_stream_decoder->pPriorHeader[pPes_stream_decoder->priorHeaderReadByte]),
                       pPayloadAddr, (PES_PRIOR_HEADER_SIZE - pPes_stream_decoder->priorHeaderReadByte));

                // Parse the first 6 bytes to get the PES_packet_length for us to
                // evaluate the end of PES packet.
                _pes_pkt_parse_pes_prior_hdr(pPes_stream_decoder);

                pPes_stream_decoder->priorHeaderReadByte = PES_PRIOR_HEADER_SIZE;
                continue;
            }
        }
        else // ptDecoder->priorHeaderReadByte == PES_PRIOR_HEADER_SIZE.
        {
            // Full PES packet can be completed in this TS packet.
            if( pPes_stream_decoder->ptBuildingPes->PES_packet_length &&
                ((pPes_stream_decoder->ptBuildingPes->gatherSize + availableByte)
                 >= (pPes_stream_decoder->ptBuildingPes->PES_packet_length + PES_PRIOR_HEADER_SIZE)) )
            {
                remainingByte = pPes_stream_decoder->ptBuildingPes->PES_packet_length
                                    - (pPes_stream_decoder->ptBuildingPes->gatherSize - PES_PRIOR_HEADER_SIZE);

                _pes_pkt_collect_pes_data_bytes(pPesPktDecoder, availableByte, pPayloadAddr);

                _pes_pkt_parse_pes(pPes_stream_decoder);

                // Reinit the some parameters of the decoder to handle
                // the next PES packet.
                pPes_stream_decoder->ptBuildingPes       = 0;
                pPes_stream_decoder->priorHeaderReadByte = 0;
                pPes_stream_decoder->bDiscontinuity      = false;
                pPes_stream_decoder->bErrorIndicator     = false;

                memset(pPes_stream_decoder->pPriorHeader, 0x0, PES_PRIOR_HEADER_SIZE);
                break;
            }
            else
            {
                // video
                _pes_pkt_collect_pes_data_bytes(pPesPktDecoder, availableByte, pPayloadAddr);

                availableByte = 0;
            }
        }
    }

    return;
}

//=============================================================================
//                Public Function Definition
//=============================================================================
void
pes_pkt_register_all_decoder(
    void)
{
    static int bInitialized = 0;

    if( bInitialized )
        return;
    bInitialized = 1;

    REGISTER_ITEM(PES_STREAM_OPR, VIDEO, video);
    // REGISTER_ITEM(PES_STREAM_OPR, AUDIO, audio);
    // REGISTER_ITEM(PES_STREAM_OPR, TELETEXT, teletext);
    // REGISTER_ITEM(PES_STREAM_OPR, SUBTITLE, subtitle);
}


uint32_t
pes_pkt_create_decoder(
    PES_PKT_DECODER     **ppPesPktDecoder,
    PES_STREAM_MBOX     *pPesStreamMbox,
    void                *extraData)
{
    uint32_t            result = TSPA_ERR_OK;
    PES_PKT_DECODER     *pPesPktDecoder = 0;

    do{
        if( !ppPesPktDecoder )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "Null pointer !!");
            result = TSPA_ERR_NULL_POINTER;
            break;
        }

        pPesPktDecoder = tspa_malloc(sizeof(PES_PKT_DECODER));
        if( !pPesStreamMbox )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "err, allocate fail !!");
            result = TSPA_ERR_ALLOCATE_FAIL;
            break;
        }

        memset(pPesPktDecoder, 0x0, sizeof(PES_PKT_DECODER));

        result = _pes_pkt_attach_decoder(pPesPktDecoder, pPesStreamMbox, extraData);

        (*ppPesPktDecoder) = (result == TSPA_ERR_OK) ? pPesPktDecoder : 0;

    }while(0);

    if( result != TSPA_ERR_OK )
    {
        if( pPesPktDecoder )        free(pPesPktDecoder);
        tspa_msg_ex(TSPA_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    return result;
}


uint32_t
pes_pkt_destroy_decoder(
    PES_PKT_DECODER     **ppPesPktDecoder,
    PES_STREAM_MBOX     *pPesStreamMbox,
    void                *extraData)
{
    uint32_t    result = TSPA_ERR_OK;

    do{
        PES_PKT_DECODER     *pPesPktDecoder = (*ppPesPktDecoder);

        if( !ppPesPktDecoder || !pPesPktDecoder )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "Null pointer !!");
            result = TSPA_ERR_NULL_POINTER;
            break;
        }

        _pes_pkt_detach_decoder(pPesPktDecoder, pPesStreamMbox, extraData);

        free(pPesPktDecoder);

        (*ppPesPktDecoder) = 0;
    }while(0);

    if( result != TSPA_ERR_OK )
    {
        tspa_msg_ex(TSPA_MSG_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    return result;
}

void
pes_pkt_decode(
    PES_PKT_DECODER     *pPesPktDecoder,
    uint8_t             *pPktData)
{
    do{
        uint32_t      sync_byte = 0;
        uint32_t      payload_unit_start_indicator = 0;
        uint32_t      adaptation_field_control = 0;
        uint32_t      continuity_counter = 0;
        uint32_t      pid = 0;
        uint32_t      adaptation_field_length = 0;

        int           availableByte = 0;
        uint8_t       *pPayloadAddr = 0;
        uint8_t       *pAdapationFieldAddr = 0;
        uint8_t       *pDataAddr = 0;
        int           counterGap = 0;
        uint32_t      errorCount = 0;

        PES_STREAM_DECODER      *pPes_stream_decoder = 0;

        if( !pPesPktDecoder || !pPktData )      break;

        pPes_stream_decoder = &pPesPktDecoder->pes_stream_decoder;

        sync_byte = pPktData[0];
        // transport_error_indicator
        if( false == pPes_stream_decoder->bErrorIndicator )
        {
            pPes_stream_decoder->bErrorIndicator = ((pPktData[1] & 0x80) ? true : false);
            if( pPes_stream_decoder->bErrorIndicator )
                errorCount++;
        }

        payload_unit_start_indicator = (pPktData[1] & 0x40) >> 6;

        pid = (((uint32_t)pPktData[1] & 0x1F) << 8) | (uint32_t)pPktData[2];

        adaptation_field_control = (pPktData[3] & 0x30) >> 4;
        continuity_counter       = (pPktData[3] & 0x0F);

        pDataAddr = &pPktData[4];

        //----------------------------------------
        // 1. check sync_byte
        if( sync_byte != VALID_SYNC_BYTE )       break;

        if( !pPes_stream_decoder->bFirstPacket )
        {
            //------------------------------------
            // 2. check continuity_counter
            if( continuity_counter == pPes_stream_decoder->continuity_counter )
                break;

            if( continuity_counter != ((pPes_stream_decoder->continuity_counter + 1) & CONTINUITY_COUNTER_MASK) )
            {
                // discontinuity
                pPes_stream_decoder->bDiscontinuity = true;

                counterGap = continuity_counter - pPes_stream_decoder->continuity_counter;

                if( counterGap <= 0 )   counterGap += 15;

                errorCount += counterGap;
            }
        }
        else
            pPes_stream_decoder->bFirstPacket = false;

        pPes_stream_decoder->continuity_counter = continuity_counter;

        //------------------------------------
        // 3. check adaptation_field_control
            // payload doesn't exist. that is all stuffing bytes behind,
            // therefore, simply return.
        if( (adaptation_field_control & PAYLOAD_EXIST) != PAYLOAD_EXIST )
            break;

        if( (adaptation_field_control & ADAPTATION_FIELD_EXIST) == ADAPTATION_FIELD_EXIST )
        {
            pAdapationFieldAddr = pDataAddr;

            adaptation_field_length = pAdapationFieldAddr[0];

            // add 1 for "adaptation_field_length" occupied 1 byte
            pPayloadAddr = (pAdapationFieldAddr + 1 + adaptation_field_length);
        }
        else
            pPayloadAddr = pDataAddr;

        availableByte = (pPktData + TS_PACKET_BYTE_NUMBER) - pPayloadAddr;

        //------------------------------------
        // 4. check payload_unit_start_indicator
        if( payload_unit_start_indicator )
        {
            // parse the previous completed PES packet.

            //------------------------------------
            // two possible cases are described below:
            // 1. when the stream is video stream because the PES_packet_length is
            // possible to be 0. therefore, it's impossible for us to identify the
            // end of the PES packet unless we get the start of next PES packet.

            // 2. some TS packets lost. this mechanism helps us to process the
            // uncompleted PES packet as well.
            if( pPes_stream_decoder->ptBuildingPes )
            {
                // exist data
                _pes_pkt_parse_pes(pPes_stream_decoder);

                // Reinit the some parameters of the decoder to handle
                // the next PES packet.
                pPes_stream_decoder->ptBuildingPes       = 0;
                pPes_stream_decoder->priorHeaderReadByte = 0;
                pPes_stream_decoder->bDiscontinuity      = 0;
                pPes_stream_decoder->bErrorIndicator     = 0;
                memset(pPes_stream_decoder->pPriorHeader, 0x0, PES_PRIOR_HEADER_SIZE);
            }

            // create a new PES_INFO to store the PES
            pPes_stream_decoder->ptBuildingPes = _pes_pkt_create_pes_info(pPes_stream_decoder, pid);
            if( !pPes_stream_decoder->ptBuildingPes )
                break;

            pPes_stream_decoder->ptBuildingPes->errorCount += errorCount;

#if (ENABLE_PES_COPY_PARSE)
            // get free cacsh buffer
            // to do:
            //if (MMP_SUCCESS != queueMgr_GetFree(pPes_stream_decoder->ptQueue,
            //                            (void**)&pPes_stream_decoder->ptBuildingPes->pData))
            //{
            //    pPes_stream_decoder->ptBuildingPes = 0;
            //    break;
            //}
#else
            pPes_stream_decoder->ptBuildingPes->pData = pPayloadAddr;
#endif

            _pes_pkt_gather_pes_seg(pPesPktDecoder, availableByte, pPayloadAddr);
        }
        else
        {
            // payload_unit_start_indicator == 0
            //---------------------------------------
            // a broken packet, we don't need to take care of it
            if( !pPes_stream_decoder->ptBuildingPes )
                break;

            pPes_stream_decoder->ptBuildingPes->errorCount += errorCount;
            _pes_pkt_gather_pes_seg(pPesPktDecoder, availableByte, pPayloadAddr);
        }

    }while(0);

    return;
}



