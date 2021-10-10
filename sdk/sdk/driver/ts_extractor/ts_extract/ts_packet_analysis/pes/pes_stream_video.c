

#include "ts_packet_analysis_defs.h"

#include "pes_stream_cfg.h"
#include "pes_stream_video.h"
#include "pes_packet_decode.h"

#if (CONFIG_PES_STREAM_OPR_VIDEO_DESC)
//=============================================================================
//                Constant Definition
//=============================================================================
#define START_CODE_SEARCH_STATE     0xea
#define START_CODE_VERIFY_STATE     0xeb
#define START_CODE_READY_STATE      0xec


//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct PES_VIDEO_DEV_T
{
    uint32_t            stream_type;

    PES_VIDEO_INFO      pes_video_info;

    //
    uint32_t            prev_bytes;
    uint32_t            prev_value;
    uint32_t            pes_start_code_state;
    uint32_t            vid_start_code_state;
    uint8_t             *pSps_sect;
    uint32_t            sps_sect_size;

}PES_VIDEO_DEV;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

static uint8_t*
_find_start_code_sub_handler(
    PES_VIDEO_DEV   *pPesVDev,
    uint8_t         *pStream,
    uint32_t        stream_size)
{
    uint8_t     *pStart_code = 0;
    uint32_t    *pCur = 0, *pEnd = 0;
    uint32_t    length = 0, remain_bytes = 0;
    uint32_t    dummy_bytes = 0;

    uint32_t    cur_value[2] = {0};
    uint32_t    act_idx    = 0;
    uint32_t    aiding_idx = 1;

    dummy_bytes  = ((uint32_t)pStream & 0x3);
    pCur         = (uint32_t*)((uint32_t)pStream & ~0x3);
    length       = (stream_size >> 2);
    pEnd         = (pCur + length);
    remain_bytes = (stream_size & 0x3);

    if( pPesVDev->prev_bytes )
    {
    }
    else
    {
    }

    return pStart_code;
}

static uint8_t*
_find_start_code(
    PES_VIDEO_DEV   *pPesVDev,
    uint8_t         *pStream,
    uint32_t        stream_size)
{
    bool        result = false;
    uint8_t     *pStart_code = 0;
    uint32_t    *pCur = 0, *pEnd = 0;
    uint32_t    length = 0, remain_bytes = 0;
    uint32_t    dummy_bytes = 0;

    // big endian case

    do{
        uint32_t    i = 0;
        uint32_t    cur_value[2] = {0};
        uint32_t    act_idx    = 0;
        uint32_t    aiding_idx = 1;

        if( !pStream || !stream_size )      break;

        dummy_bytes  = ((uint32_t)pStream & 0x3);
        pCur         = (uint32_t*)((uint32_t)pStream & ~0x3);
        length       = ((stream_size + dummy_bytes) >> 2);
        pEnd         = (pCur + length - 1);
        remain_bytes = ((stream_size + dummy_bytes) & 0x3);

        if( stream_size < 9 )
        {
#if 0
            pStart_code = _find_start_code_sub_handler(pPesVDev, pStream, stream_size);
#else
            printf(" Algorithm limitation: stream size MUST over 8 !!\n");
            pPesVDev->prev_bytes = 0;
            pPesVDev->prev_value = (-1);
#endif
            break;
        }

        // merge with previous data
        if( pPesVDev->prev_bytes )
        {
            switch( dummy_bytes )
            {
                case 0:
                    cur_value[act_idx]    = pPesVDev->prev_value;
                    cur_value[aiding_idx] = (*pCur);
                    dummy_bytes = 2;
                    break;

                case 1:
                    cur_value[act_idx]    = ((pPesVDev->prev_value & 0x000000FF) << 24);
                    cur_value[aiding_idx] = ((pPesVDev->prev_value & 0x0000FF00) >> 8) | ((*pCur) & 0xFFFFFF00);
                    dummy_bytes = 3;
                    break;

                case 2:
                    cur_value[act_idx] = ((pPesVDev->prev_value & 0x0000FFFF) | ((*pCur) & 0xFFFF0000));
                    dummy_bytes = 0;
                    cur_value[aiding_idx] = (*(++pCur));
                    break;

                case 3:
                    cur_value[act_idx] = (((pPesVDev->prev_value & 0x0000FFFF) << 8) | ((*pCur) & 0xFF000000));
                    dummy_bytes = 1;
                    cur_value[aiding_idx] = (*(++pCur));
                    break;
            }
        }
        else
        {
            // the first enter when start to search
            /* *
             * If the start code at the first section, it may be:
             *      the start code only "0x01", "00 00" be dropped OR
             *      the start code only "00 01", "00" be dropped
             **/
            cur_value[act_idx]    = (*pCur);
            cur_value[aiding_idx] = (*(++pCur));
            switch( dummy_bytes )
            {
                case 1: cur_value[act_idx] |= 0x000000FF; break;
                case 2: cur_value[act_idx] |= 0x0000FFFF; break;
                case 3: cur_value[act_idx] |= 0x00FFFFFF; break;
            }
        }

        //-----------------------------------------
        // handle the first 4 bytes with dummy data
        switch( dummy_bytes )
        {
            case 0:
                if( (cur_value[act_idx] & 0x0000FF00) == 0x00 )
                {
                    // (big endian) case 0: "xx 01 00 00" or "01 00 00 xx"
                    if( (cur_value[act_idx] & 0x00FFFFFF) == 0x00010000 ||
                        (cur_value[act_idx] & 0xFFFFFF00) == 0x01000000 )
                    {
                        // find start code, and change to uint8_t pointer
                        /**
                         * the start code only "0x01", "00 00" in the previous section OR
                         * the start code only "00 01", "00" in the previous section
                         **/
                        pStart_code = ((uint8_t*)(pCur - 1) + 2);
                        goto find_start_code;
                    }
                }

                if( (cur_value[act_idx] & 0xFF000000) == 0x00 )
                {
                    // (big endian) case 1: "00 00 xx xx, xx xx xx 01" or
                    //                      "00 xx xx xx, xx xx 01 00"
                    if( (cur_value[act_idx] & 0xFFFF0000) == 0x00000000 &&
                        (cur_value[aiding_idx] & 0x000000FF) == 0x00000001 )
                    {
                        // find start code, and change to uint8_t pointer
                        pStart_code = ((uint8_t*)(pCur - 1) + 2);
                        goto find_start_code;
                    }
                    else if( (cur_value[act_idx] & 0xFF000000) == 0x00000000 &&
                             (cur_value[aiding_idx] & 0x0000FFFF) == 0x00000100 )
                    {
                        // find start code, and change to uint8_t pointer
                         pStart_code = ((uint8_t*)(pCur - 1) + 3);
                         goto find_start_code;
                    }
                }
                break;

            case 1:
                if( (cur_value[act_idx] & 0xFFFFFF00) == 0x01000000 )
                {
                    // find start code, and change to uint8_t pointer
                    // the start code only "0x01", "00 00" in the previous section
                    pStart_code = ((uint8_t*)(pCur - 1) + 3);
                    goto find_start_code;
                }

                if( (cur_value[act_idx] & 0xFF000000) == 0x00 )
                {
                    // (big endian) case 1: "00 00 xx xx, xx xx xx 01" or
                    //                      "00 xx xx xx, xx xx 01 00"
                    if( (cur_value[act_idx] & 0xFFFF0000) == 0x00000000 &&
                        (cur_value[aiding_idx] & 0x000000FF) == 0x00000001 )
                    {
                        // find start code, and change to uint8_t pointer
                        // the start code only "00 01", "00" in the previous section
                        pStart_code = ((uint8_t*)(pCur - 1) + 3);
                        goto find_start_code;
                    }
                    else if( (cur_value[act_idx] & 0xFF000000) == 0x00000000 &&
                             (cur_value[aiding_idx] & 0x0000FFFF) == 0x00000100 )
                    {
                        // find start code, and change to uint8_t pointer
                         pStart_code = ((uint8_t*)(pCur - 1) + 3);
                         goto find_start_code;
                    }
                }
                break;

            case 2:
                if( (cur_value[act_idx] & 0xFF000000) == 0x00 )
                {
                    if( (cur_value[act_idx] & 0xFFFF0000) == 0x00000000 &&
                        (cur_value[aiding_idx] & 0x000000FF) == 0x00000001 )
                    {
                        // find start code, and change to uint8_t pointer
                        // the start code only "0x01", "00 00" in the previous section
                        pStart_code = (uint8_t*)pCur;
                        goto find_start_code;
                    }
                    else if( (cur_value[act_idx] & 0xFF000000) == 0x00000000 &&
                             (cur_value[aiding_idx] & 0x0000FFFF) == 0x00000100 )
                    {
                        // find start code, and change to uint8_t pointer
                        // the start code only "00 01", "00" in the previous section
                        pStart_code = (uint8_t*)pCur;
                        goto find_start_code;
                    }
                }
                break;

            case 3:
                if( (cur_value[act_idx] & 0xFF000000) == 0x00 &&
                    (cur_value[aiding_idx] & 0x0000FFFF) == 0x00000100 )
                {
                    // find start code, and change to uint8_t pointer
                    // the start code only "00 01", "00" in the previous section
                    pStart_code = ((uint8_t*)pCur + 1);
                    goto find_start_code;
                }
                break;
        }

        act_idx    = aiding_idx;
        aiding_idx = !aiding_idx;

        //-----------------------------------------
        // start with 4 bytes alignment
        while( pCur < pEnd )
        {
            cur_value[aiding_idx] = (*(++pCur));
            if( (cur_value[act_idx] & 0x0000FF00) == 0x00 )
            {
                // (big endian) case 0: "xx 01 00 00" or "01 00 00 xx"
                if( (cur_value[act_idx] & 0x00FFFFFF) == 0x00010000 )
                {
                    // find start code, and change to uint8_t pointer
                    pStart_code = (uint8_t*)(pCur - 1);
                    goto find_start_code;
                }
                else if( (cur_value[act_idx] & 0xFFFFFF00) == 0x01000000 )
                {
                    // find start code, and change to uint8_t pointer
                    pStart_code = ((uint8_t*)(pCur - 1) + 1);
                    goto find_start_code;
                }
            }

            if( (cur_value[act_idx] & 0xFF000000) == 0x00 )
            {
                // (big endian) case 1: "00 00 xx xx, xx xx xx 01" or
                //                      "00 xx xx xx, xx xx 01 00"
                if( (cur_value[act_idx] & 0xFFFF0000) == 0x00000000 &&
                    (cur_value[aiding_idx] & 0x000000FF) == 0x00000001 )
                {
                    // find start code, and change to uint8_t pointer
                    pStart_code = ((uint8_t*)(pCur - 1) + 2);
                    goto find_start_code;
                }
                else if( (cur_value[act_idx] & 0xFF000000) == 0x00000000 &&
                         (cur_value[aiding_idx] & 0x0000FFFF) == 0x00000100 )
                {
                    // find start code, and change to uint8_t pointer
                     pStart_code = ((uint8_t*)(pCur - 1) + 3);
                     goto find_start_code;
                }
            }

            act_idx    = aiding_idx;
            aiding_idx = !aiding_idx;
        }

        //-----------------------------------------
        // handle the last 4 bytes with dummy data, and keep the remain bytes
        cur_value[aiding_idx] = (*(++pCur));
        switch( remain_bytes )
        {
            case 0:
                cur_value[aiding_idx] |= 0xFFFFFFFF; // just for debug
                if( (cur_value[act_idx] & 0x00FFFFFF) == 0x00010000 )
                {
                    // find start code, and change to uint8_t pointer
                     pStart_code = (uint8_t*)(pCur - 1);
                     goto find_start_code;
                }

                if( (cur_value[act_idx] & 0xFFFFFF00) == 0x01000000 )
                {
                    // find start code, and change to uint8_t pointer
                     pStart_code = ((uint8_t*)(pCur - 1) + 1);
                     goto find_start_code;
                }
                pPesVDev->prev_bytes = 2;
                pPesVDev->prev_value = ((cur_value[act_idx] & 0xFFFF0000) >> 16);
                break;

            case 1:
                cur_value[aiding_idx] |= 0x000000FF; // just for debug
                if( (cur_value[act_idx] & 0x0000FF00) == 0x00 )
                {
                    if( (cur_value[act_idx] & 0x00FFFFFF) == 0x00010000 )
                    {
                        // find start code, and change to uint8_t pointer
                        pStart_code = (uint8_t*)(pCur - 1);
                        goto find_start_code;
                    }
                    else if( (cur_value[act_idx] & 0xFFFFFF00) == 0x01000000 )
                    {
                        // find start code, and change to uint8_t pointer
                        pStart_code = ((uint8_t*)(pCur - 1) + 1);
                        goto find_start_code;
                    }
                }

                if( (cur_value[act_idx] & 0xFFFF0000) == 0x00 &&
                    (cur_value[aiding_idx] & 0x000000FF) == 0x00000001 )
                {
                    // find start code, and change to uint8_t pointer
                    pStart_code = ((uint8_t*)(pCur - 1) + 2);
                    goto find_start_code;
                }
                pPesVDev->prev_bytes = 2;
                pPesVDev->prev_value = ((cur_value[act_idx] & 0xFF000000) >> 24) | ((cur_value[aiding_idx] & 0x000000FF) << 8);
                break;

            default:
                if( remain_bytes == 2 )
                    cur_value[aiding_idx] |= 0x0000FFFF; // just for debug
                else
                    cur_value[aiding_idx] |= 0x00FFFFFF; // just for debug

                if( (cur_value[act_idx] & 0x0000FF00) == 0x00 )
                {
                    // (big endian) case 0: "xx 01 00 00" or "01 00 00 xx"
                    if( (cur_value[act_idx] & 0x00FFFFFF) == 0x00010000 )
                    {
                        // find start code, and change to uint8_t pointer
                        pStart_code = (uint8_t*)(pCur - 1);
                        goto find_start_code;
                    }
                    else if( (cur_value[act_idx] & 0xFFFFFF00) == 0x01000000 )
                    {
                        // find start code, and change to uint8_t pointer
                        pStart_code = ((uint8_t*)(pCur - 1) + 1);
                        goto find_start_code;
                    }
                }

                if( (cur_value[act_idx] & 0xFF000000) == 0x00 )
                {
                    // (big endian) case 1: "00 00 xx xx, xx xx xx 01" or
                    //                      "00 xx xx xx, xx xx 01 00"
                    if( (cur_value[act_idx] & 0xFFFF0000) == 0x00000000 &&
                        (cur_value[aiding_idx] & 0x000000FF) == 0x00000001 )
                    {
                        // find start code, and change to uint8_t pointer
                        pStart_code = ((uint8_t*)(pCur - 1) + 2);
                        goto find_start_code;
                    }
                    else if( (cur_value[act_idx] & 0xFF000000) == 0x00000000 &&
                             (cur_value[aiding_idx] & 0x0000FFFF) == 0x00000100 )
                    {
                        // find start code, and change to uint8_t pointer
                         pStart_code = ((uint8_t*)(pCur - 1) + 3);
                         goto find_start_code;
                    }
                }

                if( remain_bytes == 2 )
                    pPesVDev->prev_value = (cur_value[aiding_idx] & 0x0000FFFF);
                else
                {
                    // remain_bytes == 3
                    if( (cur_value[aiding_idx] & 0x00FFFFFF) == 0x00010000 )
                    {
                        // find start code, and change to uint8_t pointer
                         pStart_code = (uint8_t*)pCur;
                         goto find_start_code;
                    }
                    pPesVDev->prev_value = ((cur_value[aiding_idx] & 0x00FFFF00) >> 8);
                }

                pPesVDev->prev_bytes = 2;
                break;
        }

    }while(0);

find_start_code:

    if( pStart_code )
    {
        if( pStart_code < pStream )
        {
            /**
             * the start code only "0x01", "00 00" in the previous section OR
             * the start code only "00 01", "00" in the previous section
             **/
            pStart_code = pStream;
        }
        pPesVDev->prev_bytes = 0;
        pPesVDev->prev_value = (-1);
    }

    return pStart_code;
}

static void
_pes_v_avc_get_sps_sect(
    PES_VIDEO_DEV   *pPesVDev,
    uint8_t         *pSample_data,
    uint32_t        sample_size)
{
    do{
        uint32_t    nal_type = 0;
        uint32_t    pesHdrLen = 0;
        uint8_t     *pCur = 0, *pEnd = 0;
        uint8_t     *pVideoStart = 0;

        if( !pSample_data || !sample_size )     break;

        pCur = pSample_data;
        pEnd = pSample_data + sample_size;

    }while(0);

    return;
}

static void
_pes_v_mp2_get_seq_hdr(
    PES_VIDEO_DEV   *pPesVDev,
    uint8_t         *pSample_data,
    uint32_t        sample_size)
{
    do{
        uint32_t    nal_type = 0;
        uint32_t    pesHdrLen = 0;
        uint8_t     *pCur = 0, *pEnd = 0;
        uint8_t     *pVideoStart = 0;

        if( !pSample_data || !sample_size )     break;

        pCur = pSample_data;
        pEnd = pSample_data + sample_size;

    }while(0);

    return;
}

static uint32_t
pes_video_init(
    PES_STREAM_MBOX     *pPesStreamMbox,
    void                *extraData)
{
    uint32_t        result = 0;

    do{
        PES_VIDEO_DEV       *pPesVDev = 0;
        PES_STREAM_DECODER  *pPes_stream_decoder = 0;

        if( !pPesStreamMbox )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "Null pointer !\n");
            break;
        }

        pPes_stream_decoder = (PES_STREAM_DECODER*)pPesStreamMbox->pes_stream_mbox_arg.arg.v.pPes_stream_decoder;

        pPesVDev = tspa_malloc(sizeof(PES_VIDEO_DEV));
        if( !pPesVDev )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "allcate fail !\n");
            break;
        }

        memset(pPesVDev, 0x0, sizeof(PES_VIDEO_DEV));

        // ------------------------------
        // init parameters
        pPesVDev->pes_start_code_state = START_CODE_SEARCH_STATE;
        pPesVDev->vid_start_code_state = START_CODE_SEARCH_STATE;
        pPesVDev->stream_type          = pPesStreamMbox->pes_stream_mbox_arg.arg.v.video_stream_type;
        pPesVDev->prev_bytes           = 0;
        pPesVDev->prev_value           = (-1);

        pPes_stream_decoder->privData = (void*)&pPesVDev->pes_video_info;

    }while(0);

    return result;
}

static uint32_t
pes_video_deinit(
    PES_STREAM_MBOX     *pPesStreamMbox,
    void                *extraData)
{
    uint32_t        result = 0;

    do{
        PES_VIDEO_DEV       *pPesVDev = 0;
        PES_STREAM_DECODER  *pPes_stream_decoder = 0;

        if( !pPesStreamMbox )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "Null pointer !\n");
            break;
        }

        pPes_stream_decoder = (PES_STREAM_DECODER*)pPesStreamMbox->pes_stream_mbox_arg.arg.v.pPes_stream_decoder;

        if( !pPes_stream_decoder->privData )        break;

        pPesVDev = DOWN_CAST(PES_VIDEO_DEV, pPes_stream_decoder->privData, pes_video_info);

        if( pPesVDev->pSps_sect )
        {
            free(pPesVDev->pSps_sect);
            pPesVDev->pSps_sect = 0;
        }

        pPes_stream_decoder->privData = 0;

        free(pPesVDev);
    }while(0);

    return result;
}

static uint32_t
pes_video_analysis(
    PES_STREAM_MBOX     *pPesStreamMbox,
    void                *extraData)
{
    uint32_t    result = 0;

    do{
        PES_VIDEO_DEV       *pPesVDev = 0;
        PES_STREAM_DECODER  *pPes_stream_decoder = 0;
        uint8_t             *pSample_data = 0;
        uint32_t            sample_size = 0, remain_size = 0;
        uint8_t             *pStart_code = 0, *pCur = 0;

        if( !pPesStreamMbox )
        {
            tspa_msg_ex(TSPA_MSG_ERR, "Null pointer !\n");
            break;
        }

        {// just debug
            pPesStreamMbox->pes_stream_mbox_arg.arg.v.bGetResolution = true;
            break; 
        }

        pPes_stream_decoder = (PES_STREAM_DECODER*)pPesStreamMbox->pes_stream_mbox_arg.arg.v.pPes_stream_decoder;
        pSample_data        = pPesStreamMbox->pes_stream_mbox_arg.arg.v.pData;
        sample_size         = pPesStreamMbox->pes_stream_mbox_arg.arg.v.data_size;

        if( !pPes_stream_decoder->privData )        break;

        pPesVDev = DOWN_CAST(PES_VIDEO_DEV, pPes_stream_decoder->privData, pes_video_info);

        // if get resolution => leave
        if( pPesVDev->stream_type == ISO_IEC_RESERVED || 
            pPesVDev->vid_start_code_state == START_CODE_READY_STATE )
            break;

        do{
            //---------------------------------------
            // analysis packet
            pStart_code = _find_start_code(pPesVDev, pSample_data, sample_size);
            if( !pStart_code )      break;

            // get start code
            switch( pStart_code[0] )
            {
                case 0x00:
                    if( pStart_code[1] == 0x00 && pStart_code[2] == 0x01 )
                    {
                        // copy section to cacsh buffer for parsing header
                        pCur        = pStart_code + 3;
                        remain_size = sample_size - (pCur - pSample_data);
                    }
                    else if ( pStart_code[1] == 0x01 )
                    {
                        // copy section to cacsh buffer for parsing header
                        pCur        = pStart_code + 2;
                        remain_size = sample_size - (pCur - pSample_data);
                    }
                    break;

                case 0x01:
                    // copy section to cacsh buffer for parsing header
                    pCur        = pStart_code + 1;
                    remain_size = sample_size - (pCur - pSample_data);
                    break;
            }

            sample_size          = remain_size;
            pSample_data         = pCur;
            pPesVDev->prev_bytes = 0;
            pPesVDev->prev_value = (-1);

            if( !sample_size )
            {
                tspa_msg_ex(TSPA_MSG_ERR, " PES remain size == 0 !\n");
                break;
            }
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // if remain size < 8, then the state mechine will fail.
            switch( pPesVDev->pes_start_code_state )
            {
                case START_CODE_SEARCH_STATE:
                    // check video PES
                    if( (pCur[0] & 0xF0) == 0xE0 )
                        pPesVDev->pes_start_code_state = START_CODE_READY_STATE;
                    break;

                case START_CODE_READY_STATE:
                    switch( pPesVDev->stream_type )
                    {
                        case ISO_IEC_11172_2_VIDEO:   // mpeg1 video
                        case ISO_IEC_13818_2_VIDEO:   // mpeg2 video
                            if( pCur[0] == 0xB3 ) // Sequence header
                            {
                                //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                // if width/height bit field in different PES, it wil fail.
                                _pes_v_mp2_get_seq_hdr(pPesVDev, pSample_data, sample_size);
                                pPesStreamMbox->pes_stream_mbox_arg.arg.v.bGetResolution = true;
                            }
                            else
                                pPesVDev->pes_start_code_state = START_CODE_SEARCH_STATE;                                
                            break;

                        case ISO_IEC_14496_10_VIDEO:  // avc (h.264) video
                            if( pCur[0] == 0x07 ) // sps
                            {
                                //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                // if width/height bit field in different PES, it wil fail.
                                _pes_v_avc_get_sps_sect(pPesVDev, pSample_data, sample_size);
                                pPesStreamMbox->pes_stream_mbox_arg.arg.v.bGetResolution = true;
                            }
                            else
                                pPesVDev->pes_start_code_state = START_CODE_SEARCH_STATE;
                            break;
                    }

                    break;
            }
        }while( sample_size );
        
        //---------------------------------------
        // copy packet to tmp buf for parsing

    }while(0);

    return result;
}

static uint32_t
pes_video_get_info(
    PES_STREAM_MBOX     *pPesStreamMbox,
    void                *extraData)
{
    uint32_t        result = 0;

    return result;
}
//=============================================================================
//                Public Function Definition
//=============================================================================
PES_STREAM_OPR PES_STREAM_OPR_video_desc =
{
    "pes video",        // char        *name;
    0,                  // struct PES_STREAM_OPR_T     *next;
    PES_STREAM_VIDEO,   // PES_STREAM_ID               id;
    0,                  // void        *privInfo;
    pes_video_init,     // uint32_t    (*init)(PES_STREAM_MBOX *pPesStreamMbox, void *extraData);
    pes_video_deinit,   // uint32_t    (*deinit)(PES_STREAM_MBOX *pPesStreamMbox, void *extraData);
    pes_video_analysis, // uint32_t    (*proc)(PES_STREAM_MBOX *pPesStreamMbox, void *extraData);
    pes_video_get_info, // uint32_t    (*get_info)(PES_STREAM_MBOX *pPesStreamMbox, void *extraData);
};
#else /* #if (CONFIG_PES_STREAM_OPR_VIDEO_DESC) */
PES_STREAM_OPR PES_STREAM_OPR_video_desc =
{
    "pes video",        // char        *name;
    0,                  // struct PES_STREAM_OPR_T     *next;
    PES_STREAM_VIDEO,   // PES_STREAM_ID               id;
    0,                  // void        *privInfo;
};
#endif
