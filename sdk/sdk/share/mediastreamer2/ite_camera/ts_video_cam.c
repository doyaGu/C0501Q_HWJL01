

#include "mediastreamer2/mscommon.h"
#include "mediastreamer2/msvideo.h"
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
#include "mediastreamer2/mswebcam.h"
#include "mediastreamer2/rfc3984.h"

#if defined(TS_CAM_ENABLE)

#include "ite/itp.h"
#include "pthread.h"
#include "ring_buf_opt.h"

#if 1 // (_MSC_VER)
    #include "ts_recv_file.h"
#else
    #include "ts_recv_camera.h"
#endif
//=============================================================================
//                Constant Definition
//=============================================================================
#define TS_L2_BUF_MAX_SIZE          (2<<20)
#define CMD_PKTS_BUF_MAX_SIZE       512112
#define TS_PACKET_SIZE              (188)
#define TS_VALID_SYNC_BYTE          (0x47)
#define TS_CMD_PKT_PID              0x201

/**
 *
 **/
typedef enum TS_STREAM_STATE_T
{
    TS_STREAM_STATE_SEARCH_PKT_START  = 0x00,
    TS_STREAM_STATE_LESS_PKT_SIZE     = 0x22,
} TS_STREAM_STATE;
//=============================================================================
//                Macro Definition
//=============================================================================

#define SLICE_NUM			512
#define ENABLE_3_BYTES_START_CODE	0
#if ENABLE_3_BYTES_START_CODE
#define IS_NALU_START_CODE(p)		((p[0] == 0x00) && \
					 (p[1] == 0x00) && \
					 (p[2] == 0x01))
#else
#define IS_NALU_START_CODE(p)		((p[0] == 0x00) && \
					 (p[1] == 0x00) && \
					 (p[2] == 0x00) && \
					 (p[3] == 0x01))
#endif

//=============================================================================
//                Structure Definition
//=============================================================================

typedef struct _SLICE_DATA {
	uint8_t* pcSliceBuffer[SLICE_NUM];
	int	 pnSliceSize[SLICE_NUM];
	int	 nSliceCount;
} SLICE_DATA;

/**
 * ts camera decoder info
 **/
typedef struct TS_CAM_DEC_INFO_T
{
    uint32_t       reserved;
} TS_CAM_DEC_INFO;

/**
 * ts camera encoder info
 **/
typedef struct TS_CAM_ENC_INFO_T
{
    pthread_t           tsi_L2_thread;
    pthread_t           cmd_pkts_parse_thread;
    ms_mutex_t          lock_key;
    bool                bDel_thread;

    uint32_t            tsi_idx;

    // L2 buffer info
    mblk_t              *comp_buf;
    bool                bStart_ts_L2_cache;
    uint8_t             *pTs_L2_buf;
    uint32_t            ts_L2_buf_size;
    RB_OPT              ts_L2_buf_opr;

    // cmd packet buffer info
    bool                bStart_cmd_pkts_cache;
    uint8_t             *pCmd_pkts_buf;
    uint32_t            cmd_pkts_buf_size;
    RB_OPT              cmd_pkt_buf_opr;

    // packet parsing
    uint32_t            act_pkt_size;   // action packet size
    TS_STREAM_STATE     stream_state;
    uint32_t            collectedByte;
    uint8_t             *pIncompletePktCache;

    // ts input operator
    TS_RECV_OPR         ts_recv_opr;

    // network maximum transmission unit in bytes
    int                 mtu;

    /* Evan, added for basic Camera info */
    MSVideoSize		vsize;
    float		fps;
    SLICE_DATA		slice;
    Rfc3984Context	*packer;
    int			mode;
} TS_CAM_ENC_INFO;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

///////////////////////////////////
// H.264 Nalu Process relate
///////////////////////////////////

/*
 * Function:
 * 	slice_nalu_process
 * Params:
 * 	SLICE_DATA slice: struct to save data
 * 	uint8_t *pencb:	Encoded H.264 buffer pointer
 * 	int     szencb:	Encoded H.264 buffer size
 * Description:
 * 	Split input buffer to Nalus
 * Return:
 * 	If Nalus exist in encoded buffer, split pointers to array, and record
 * 	relate size. It should return Nalus numbers in array, this functions 
 * 	clear arrays when calling it.
 *
*/
static int slice_nalu_process(SLICE_DATA *slice, uint8_t *pencb, int szencb)
{
	int i, nSliceCount = -1;

#if ENABLE_3_BYTES_START_CODE
	for (i = 0; i < szencb-3; i++)
#else
	for (i = 0; i < szencb-4; i++)
#endif
	{
		uint8_t *src = pencb + i;
		bool sc1 = IS_NALU_START_CODE(src);
		bool sc2 = (src[0]==0x00 && src[1]==0x00 && src[2]==0x01);

		if (sc1 || sc2)
		{
			nSliceCount++;
			if (sc2)
			{
				// TODO: special hook
				slice->pcSliceBuffer[nSliceCount] = src - 1;
				slice->pnSliceSize[nSliceCount] = 2;
			}
			else
			{
			slice->pcSliceBuffer[nSliceCount] = src;
			//slice->pnSliceSize[nSliceCount] = 1;
			slice->pnSliceSize[nSliceCount] = 2;
			i++;
			}
		}
		else
		{
			if (nSliceCount >= 0)
				slice->pnSliceSize[nSliceCount]++;
		}
	}
	if (nSliceCount >= 0) // only check (szencb-4), so +4
	{
#if ENABLE_3_BYTES_START_CODE
		slice->pnSliceSize[nSliceCount] += 3;
#else
		slice->pnSliceSize[nSliceCount] += 4;
#endif
	}
	return nSliceCount;
}

///////////////////////////////////
// Decode API
///////////////////////////////////
static void
ts_dec_init(
    MSFilter    *f)
{
    TS_CAM_DEC_INFO *s = (TS_CAM_DEC_INFO *)ms_new0(TS_CAM_DEC_INFO, 1);
    f->data = s;
    return;
}

static void
ts_dec_preprocess(
    MSFilter    *f)
{
    TS_CAM_DEC_INFO *s = (TS_CAM_DEC_INFO *)f->data;
    int error = 0;
    return;
}

static void
ts_dec_process(
    MSFilter    *f)
{
    return;
}

static void
ts_dec_postprocess(
    MSFilter    *f)
{
    return;
}

static void
ts_dec_uninit(
    MSFilter    *f)
{
    TS_CAM_DEC_INFO *s = (TS_CAM_DEC_INFO *)f->data;
    ms_free(s);
    return;
}

///////////////////////////////////
// Encode API
///////////////////////////////////
static uint32_t
_cmd_pkts_split(
    TS_CAM_ENC_INFO     *pEnc_info,
    uint8_t             *pBuf,
    uint32_t            size)
{
#define _GET_PID(pData)     ((uint32_t)(((pData[1] & 0x1f) << 8) | pData[2]))

    do{
        uint8_t     *pData = pBuf;
        uint32_t    remainSize = size;

        if( !pBuf || !size )    break;

        while( remainSize > 0 )
        {
            switch( pEnc_info->stream_state )
            {
                case TS_STREAM_STATE_SEARCH_PKT_START:
                    if( (*pData) == TS_VALID_SYNC_BYTE )
                    {
                        if( remainSize < pEnc_info->act_pkt_size )
                        {
                            pEnc_info->stream_state = TS_STREAM_STATE_LESS_PKT_SIZE;
                        }
                        else
                        {
                            if( _GET_PID(pData) == TS_CMD_PKT_PID )
                                rb_opt_update_w(&pEnc_info->cmd_pkt_buf_opr, (uint32_t)pData, pEnc_info->act_pkt_size);

                            pData       += pEnc_info->act_pkt_size;
                            remainSize  -= pEnc_info->act_pkt_size;
                        }
                    }
                    else
                    {
                        ++pData;
                        --remainSize;
                    }
                    break;

                case TS_STREAM_STATE_LESS_PKT_SIZE:
                    if( pEnc_info->collectedByte > 0 &&
                            remainSize >= (int)(pEnc_info->act_pkt_size - pEnc_info->collectedByte) )
                    {
                        memcpy(&pEnc_info->pIncompletePktCache[pEnc_info->collectedByte],
                               pData, (pEnc_info->act_pkt_size - pEnc_info->collectedByte));

                        if( _GET_PID(pEnc_info->pIncompletePktCache) == TS_CMD_PKT_PID )
                            rb_opt_update_w(&pEnc_info->cmd_pkt_buf_opr, (uint32_t)pEnc_info->pIncompletePktCache, pEnc_info->act_pkt_size);

                        pData       += (pEnc_info->act_pkt_size - pEnc_info->collectedByte);
                        remainSize  -= (pEnc_info->act_pkt_size - pEnc_info->collectedByte);

                        pEnc_info->collectedByte = 0;
                        pEnc_info->stream_state  = TS_STREAM_STATE_SEARCH_PKT_START;
                    }
                    else
                    {
                        memcpy(&pEnc_info->pIncompletePktCache[pEnc_info->collectedByte],
                               pData, remainSize);

                        pEnc_info->collectedByte += remainSize;
                        remainSize = 0;
                    }
                    break;
            }
        }
    }while(0);

    return 0;
}

static void *
_cmd_pkts_parse_thread(
    void *args)
{
    TS_CAM_ENC_INFO     *pEnc_info = (TS_CAM_ENC_INFO *)args;

    while( pEnc_info->bDel_thread == false )
    {
        if( pEnc_info->bStart_cmd_pkts_cache == true )
        {
            // return channel
            // To Do:
        }

        usleep(300000);
    }

    ms_message(" exit %s()\n", __FUNCTION__);
    pthread_exit(NULL);
    return 0;
}

static void *
_ts_L2_buf_thread(
    void *args)
{
    TS_CAM_ENC_INFO     *pEnc_info = (TS_CAM_ENC_INFO *)args;

    while( pEnc_info->bDel_thread == false )
    {
        uint8_t     *pSample_addr = 0;
        uint32_t    sample_size = 0;

        if( pEnc_info->bStart_ts_L2_cache == true )
        {
            if( pEnc_info->ts_recv_opr.ts_recv_get_data )
                pEnc_info->ts_recv_opr.ts_recv_get_data(&pEnc_info->ts_recv_opr, &pSample_addr, &sample_size, 0);

            //-----------------------------------
            // copy to L2 buffer
            rb_opt_update_w(&pEnc_info->ts_L2_buf_opr, (uint32_t)pSample_addr, sample_size);

        #if 0 // Now, only 264 stream input
            //-----------------------------------
            // ts cmd packet parser and copy to pCmd_pkts_buf
            _cmd_pkts_split(pEnc_info, pSample_addr, sample_size);
        #endif
        }

        usleep(10000);
    }

    ms_message(" exit %s()\n", __FUNCTION__);
    pthread_exit(NULL);
    return 0;
}

static void
ts_enc_init(
    MSFilter    *f)
{
    TS_CAM_ENC_INFO     *pEnc_info = (TS_CAM_ENC_INFO *)ms_new(TS_CAM_ENC_INFO, 1);
printf("[MSTsEnc] init\n");
    do{
        if( !f || !pEnc_info )
        {
            ms_error("ts cam: Null pointer (0x%x,0x%x) ! %s[%d]\n",
                     f, pEnc_info, __FUNCTION__, __LINE__);
            break;
        }
        //-------------------------------
        // initial
        memset(pEnc_info, 0x0, sizeof(TS_CAM_ENC_INFO));
        pEnc_info->mtu          = 1316;//ms_get_payload_max_size();
        pEnc_info->tsi_idx      = 0;
        pEnc_info->bDel_thread  = false;
        pEnc_info->stream_state = TS_STREAM_STATE_SEARCH_PKT_START;
        pEnc_info->act_pkt_size = TS_PACKET_SIZE;
        pEnc_info->pIncompletePktCache = ms_malloc(pEnc_info->act_pkt_size);
        if( !pEnc_info->pIncompletePktCache )
        {
            ms_error("ts cam: alloc fail ! %s[%d]\n", __FUNCTION__, __LINE__);
            break;
        }

        // set ts input
        pEnc_info->ts_recv_opr           = ts_recv_opr;
        pEnc_info->ts_recv_opr.tsi_index = pEnc_info->tsi_idx;

        // L2 buffer
        pEnc_info->ts_L2_buf_size = TS_L2_BUF_MAX_SIZE;
        pEnc_info->comp_buf   = allocb(pEnc_info->ts_L2_buf_size, 0);
        pEnc_info->pTs_L2_buf = pEnc_info->comp_buf->b_datap->db_base;
        if( !pEnc_info->pTs_L2_buf )
        {
            ms_error("ts cam: alloc fail ! %s[%d]\n", __FUNCTION__, __LINE__);
            break;
        }
        // L2 buffer operator init
        rb_opt_init(&pEnc_info->ts_L2_buf_opr, (uint32_t)pEnc_info->pTs_L2_buf, pEnc_info->ts_L2_buf_size);

        // cmd packet buffer
        pEnc_info->cmd_pkts_buf_size = CMD_PKTS_BUF_MAX_SIZE;
        pEnc_info->pCmd_pkts_buf = ms_malloc(pEnc_info->cmd_pkts_buf_size);
        if( !pEnc_info->pCmd_pkts_buf )
        {
            ms_error("ts cam: alloc fail ! %s[%d]\n", __FUNCTION__, __LINE__);
            break;
        }
        // cmd packet buffer operator init
        rb_opt_init(&pEnc_info->cmd_pkt_buf_opr, (uint32_t)pEnc_info->pCmd_pkts_buf, pEnc_info->cmd_pkts_buf_size);

        //--------------------------------------
        // init tsi
        if( pEnc_info->ts_recv_opr.ts_recv_init )
            pEnc_info->ts_recv_opr.ts_recv_init(&pEnc_info->ts_recv_opr, 0);

        if( pEnc_info->ts_recv_opr.ts_recv_turn_on )
            pEnc_info->ts_recv_opr.ts_recv_turn_on(&pEnc_info->ts_recv_opr, 0);

	/* Evan TODO: add for configure_video_source */
    pEnc_info->vsize.width = 852;
    pEnc_info->vsize.height = 480;
	pEnc_info->fps = 25.0;
	memset(&pEnc_info->slice, NULL, sizeof(SLICE_DATA));
	pEnc_info->packer = NULL;
	pEnc_info->mode = 1; // make sure RFC3984 packetization mode is Fregmentation Unit (FU-A)

        //--------------------------------------
        // create thread
        pthread_create(&pEnc_info->tsi_L2_thread, NULL, _ts_L2_buf_thread, (void *)pEnc_info);
        // pthread_create(&pEnc_info->cmd_pkts_parse_thread, NULL, _cmd_pkts_parse_thread, (void*)pEnc_info);

        //--------------------------------------
        ms_mutex_init(&pEnc_info->lock_key, NULL);
        f->data = pEnc_info;
    }while(0);

    return;
}

static void
ts_enc_uninit(
    MSFilter    *f)
{
    TS_CAM_ENC_INFO     *pEnc_info = (TS_CAM_ENC_INFO *)f->data;
printf("[MSTsEnc] uninit\n");
    do{
        if( !f || !pEnc_info )    break;

        ms_mutex_lock(&pEnc_info->lock_key);

        pEnc_info->bDel_thread = true;

        pthread_join(pEnc_info->tsi_L2_thread, NULL);
        // pthread_join(pEnc_info->cmd_pkts_parse_thread, NULL);

        if( pEnc_info->comp_buf )               freemsg(pEnc_info->comp_buf);
        pEnc_info->pTs_L2_buf = 0;

        if( pEnc_info->pCmd_pkts_buf )          ms_free(pEnc_info->pCmd_pkts_buf);
        if( pEnc_info->pIncompletePktCache )    ms_free(pEnc_info->pIncompletePktCache);

        //----------------------------------
        // terminate tsi
        if( pEnc_info->ts_recv_opr.ts_recv_turn_off )
            pEnc_info->ts_recv_opr.ts_recv_turn_off(&pEnc_info->ts_recv_opr, 0);

        if( pEnc_info->ts_recv_opr.ts_recv_deinit )
            pEnc_info->ts_recv_opr.ts_recv_deinit(&pEnc_info->ts_recv_opr, 0);

        ms_mutex_unlock(&pEnc_info->lock_key);
        ms_mutex_destroy(&pEnc_info->lock_key);
        ms_free(pEnc_info);
    }while(0);

    return;
}

static void
ts_enc_preprocess(
    MSFilter    *f)
{
    TS_CAM_ENC_INFO     *pEnc_info = (TS_CAM_ENC_INFO *)f->data;
    int                 error = 0;
printf("[MSTsEnc] preprocess\n");
    do{
        if( !f || !pEnc_info )    break;

	/* Evan, add for rfc3984 */
	pEnc_info->packer = rfc3984_new();
	rfc3984_set_mode(pEnc_info->packer, pEnc_info->mode);
	rfc3984_enable_stap_a(pEnc_info->packer, FALSE);

        if( pEnc_info->bStart_ts_L2_cache == false )
        {
            uint8_t     *pSample_addr = 0;
            uint32_t    sample_size = 0;

            // align write pointer
            if( pEnc_info->ts_recv_opr.ts_recv_get_data )
                pEnc_info->ts_recv_opr.ts_recv_get_data(&pEnc_info->ts_recv_opr, &pSample_addr, &sample_size, 0);

            pEnc_info->bStart_ts_L2_cache = true;
        }

        pEnc_info->bStart_cmd_pkts_cache = true;

        // -----------------------------
        // wait for getting camera info by return_channel ???

    }while(0);

    return;
}

#define TMP_BUF_SIZE	(5 * 1024 * 1024)
static uint8_t tmpb[TMP_BUF_SIZE];
static int tmp_len = 0;
static uint8_t *last_ptr = &tmpb[0];

static void
ts_enc_process(
    MSFilter    *f)
{
    TS_CAM_ENC_INFO     *pEnc_info = (TS_CAM_ENC_INFO *)f->data;
    int                 error = 0;
    uint32_t		ts = f->ticker->time*90LL;

    do{
        mblk_t  *inm = 0;

        if( !f || !pEnc_info )    break;

        ms_mutex_lock(&pEnc_info->lock_key);

        //while( (inm = ms_queue_get(f->inputs[0])) )
        {
            uint32_t    sample_addr = 0, sample_size = 0;
#if 0
            rb_opt_update_r(&pEnc_info->ts_L2_buf_opr, 0, &sample_addr, &sample_size);
            if( sample_addr && sample_size )
            {
                uint8_t     *r_ptr = 0;
                mblk_t      *comp_buf = pEnc_info->comp_buf;
                mblk_t      *packet = 0;
                int         len = 0;

                //printf(" sample_addr=0x%x, sample_size=%d \n", sample_addr, sample_size);
                comp_buf->b_rptr = (uint8_t*)sample_addr;
                comp_buf->b_wptr = (uint8_t*)(sample_addr + sample_size);

                for(r_ptr = comp_buf->b_rptr; r_ptr < comp_buf->b_wptr; )
                {
                    len = MIN(pEnc_info->mtu, (comp_buf->b_wptr - r_ptr));
                    packet = dupb(comp_buf);
                    packet->b_rptr = r_ptr;
                    packet->b_wptr = r_ptr + len;
                    if( !f->outputs[0] )    break;
                    ms_queue_put(f->outputs[0], packet);
                    r_ptr += len;
                }
            }
            //freemsg(inm);
#endif
		/* TODO: Evan, prefetch NALu in L2 buffer */
		rb_opt_update_r(&pEnc_info->ts_L2_buf_opr, 0, &sample_addr, &sample_size);
		if( sample_addr && sample_size )
		{
			uint8_t *r_ptr = &tmpb[0];
			uint8_t *w_ptr = r_ptr + tmp_len;
			uint8_t *pt = r_ptr;
			bool bfirst = false, blast = false;

			mblk_t *m;
			MSQueue nalus;
			int i, nSliceCount = 0;
	
			if (((w_ptr-r_ptr)+sample_size) > TMP_BUF_SIZE)
				printf("### %s:%d error size %d+%d > %d\n", __func__, __LINE__, w_ptr-r_ptr, sample_size, TMP_BUF_SIZE);
			memcpy(w_ptr, sample_addr, sample_size);
			tmp_len += sample_size;
			w_ptr = r_ptr + tmp_len;

			// first round fetech, to align start code
			// search from dead
#if ENABLE_3_BYTES_START_CODE
			for (;pt <= w_ptr-3; pt++)
#else
			for (;pt <= w_ptr-4; pt++)
#endif
			{
				if (IS_NALU_START_CODE(pt))
				{
					uint8_t *pStart = pt;
					bfirst = true;

					// second round fetch, to grab actual data
					// search from tail
#if ENABLE_3_BYTES_START_CODE
					for (pt = w_ptr-3; pt>last_ptr; pt--)
#else
					for (pt = w_ptr-4; pt>last_ptr; pt--)
#endif
					{
						if (IS_NALU_START_CODE(pt))
						{
							/* Evan, add for RFC3984 */
							ms_queue_init(&nalus);
							blast = true;

							// split to Nalus
							nSliceCount = slice_nalu_process(&pEnc_info->slice, pStart, pt-pStart) + 1;
//printf("### send %d slice\n", nSliceCount);

							for (i = 0; i < nSliceCount; i++)
							{
								uint8_t *pbuf = pEnc_info->slice.pcSliceBuffer[i];
								int szbuf = pEnc_info->slice.pnSliceSize[i];
#if 0
printf("Nalus[%d] %p len %d 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n", i, pbuf, szbuf,
		pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5]);
#endif
								m = allocb(szbuf + 10, 0);
#if ENABLE_3_BYTES_START_CODE
								memcpy(m->b_wptr, pbuf + 3, (szbuf - 3));
								m->b_wptr += szbuf - 3;
#else
								memcpy(m->b_wptr, pbuf + 4, (szbuf - 4));
								m->b_wptr += szbuf - 4;
#endif
								ms_queue_put(&nalus, m);

								// send to back end
								if(!ms_queue_empty(&nalus))
									rfc3984_pack(pEnc_info->packer, &nalus, f->outputs[0], ts);

								ts += 270; // (9' t/ms), 9 * 30
							}
#if 0
							// send to back end
							if(!ms_queue_empty(&nalus))
								rfc3984_pack(pEnc_info->packer, &nalus, f->outputs[0], ts);
#endif
							// move remain data to front
							tmp_len = w_ptr - pt;
							memcpy(r_ptr, pt, tmp_len);
							last_ptr = r_ptr;

							goto process_end;
						}
					}
					if (!blast) // copy remain data to front
					{
						tmp_len = w_ptr - pStart;
						if (r_ptr != pStart)
							memcpy(r_ptr, pStart, tmp_len);
#if ENABLE_3_BYTES_START_CODE
						last_ptr = r_ptr + tmp_len - 3;
#else
						last_ptr = r_ptr + tmp_len - 4;
#endif
					}
				}
			}
			if (!bfirst)
				tmp_len = 0;
		}
        }
process_end:
        ms_mutex_unlock(&pEnc_info->lock_key);
    }while(0);

    return;
}

static void
ts_enc_postprocess(
    MSFilter    *f)
{
    TS_CAM_ENC_INFO     *pEnc_info = (TS_CAM_ENC_INFO *)f->data;
    int                 error = 0;
printf("[MSTsEnc] postprocess\n");
    do{
        if( !f || !pEnc_info )    break;

	/* Evan, add for rfc3984 */
	rfc3984_destroy(pEnc_info->packer);
	pEnc_info->packer = NULL;

        //ms_mutex_lock(&pEnc_info->lock_key);
        //ms_mutex_unlock(&pEnc_info->lock_key);
    }while(0);

    return;
}

#if 1 /* Evan, add for configure_video_source */
static int ts_enc_set_vsize(MSFilter *f, void* data)
{
	TS_CAM_ENC_INFO *pEnc_info = (TS_CAM_ENC_INFO *)f->data;
	pEnc_info->vsize = *(MSVideoSize*)data;
	/* TODO: switch output res SIF, D1, 720P */
	return 0;
}

static int ts_enc_get_vsize(MSFilter *f, void* data)
{
	TS_CAM_ENC_INFO *pEnc_info = (TS_CAM_ENC_INFO *)f->data;
	*(MSVideoSize*)data = pEnc_info->vsize;
	return 0;
}

static int ts_enc_set_fps(MSFilter *f, void* data)
{
	TS_CAM_ENC_INFO *pEnc_info = (TS_CAM_ENC_INFO *)f->data;
	pEnc_info->fps = *(float*)data;
	return 0;
}

static int ts_enc_get_fps(MSFilter *f, void* data)
{
	TS_CAM_ENC_INFO *pEnc_info = (TS_CAM_ENC_INFO *)f->data;
	*(float*)data = pEnc_info->fps;
	return 0;
}
#endif

static int
ts_enc_set_mtu(
    MSFilter    *f,
    void        *arg)
{
    TS_CAM_ENC_INFO     *pEnc_info = (TS_CAM_ENC_INFO *)f->data;

    do{
        if( !f || !pEnc_info )    break;

        ms_mutex_lock(&pEnc_info->lock_key);

        pEnc_info->mtu = *(int *)arg;

        ms_mutex_unlock(&pEnc_info->lock_key);
    }while(0);

    return 0;
}

///////////////////////////////////
// ts camera API
///////////////////////////////////
static void
ts_cam_detect(
    MSWebCamManager     *obj)
{
    extern MSWebCamDesc ts_cam_desc;
    MSWebCam *cam = ms_web_cam_new(&ts_cam_desc);
    ms_web_cam_manager_add_cam(obj, cam);
}

static void
ts_cam_init(
    MSWebCam    *cam)
{
    cam->name = ms_strdup("Ts camera");
}

static MSFilter*
ts_cam_create_reader(
    MSWebCam    *obj)
{
    extern MSFilterDesc ms_ts_enc_desc;
    return ms_filter_new_from_desc(&ms_ts_enc_desc);
}

//=============================================================================
//                Public Function Definition
//=============================================================================
static MSFilterMethod dec_methods[] =
{
    // {MS_FILTER_ADD_FMTP, dec_add_fmtp},
    {0, NULL}
};


MSFilterDesc ms_ts_dec_desc =
{
    MS_TS_DEC_ID,
    "MSTsDec",
    N_("A Ts decoder"),
    MS_FILTER_DECODER,
    "ts",
    1,
    1,
    ts_dec_init,
    ts_dec_preprocess,
    ts_dec_process,
    ts_dec_postprocess,
    ts_dec_uninit,
    dec_methods
};


static MSFilterMethod enc_methods[] =
{
    {MS_FILTER_SET_FPS,         ts_enc_set_fps},
    {MS_FILTER_GET_FPS,         ts_enc_get_fps},
    {MS_FILTER_SET_VIDEO_SIZE,  ts_enc_set_vsize},
    {MS_FILTER_GET_VIDEO_SIZE,  ts_enc_get_vsize},
    // {MS_FILTER_ADD_FMTP,        ts_enc_add_fmtp},
    // {MS_FILTER_SET_BITRATE,     ts_enc_set_br},
    // {MS_FILTER_GET_BITRATE,     ts_enc_get_br},
    {MS_FILTER_SET_MTU,         ts_enc_set_mtu},
    // {MS_FILTER_REQ_VFU,         ts_enc_req_vfu},
    {0, NULL}
};


MSFilterDesc ms_ts_enc_desc =
{
    MS_TS_ENC_ID,
    "MSTsEnc",
    N_("TS stream by pass."),
    MS_FILTER_OTHER, 
    "ts",
    0, /*MS_YUV420P is assumed on this input */
    1,
    ts_enc_init,
    ts_enc_preprocess,
    ts_enc_process,
    ts_enc_postprocess,
    ts_enc_uninit,
    enc_methods
};


MSWebCamDesc ts_cam_desc =
{
    "Ts camera",
    &ts_cam_detect,
    &ts_cam_init,
    &ts_cam_create_reader,
    NULL
};

#endif
