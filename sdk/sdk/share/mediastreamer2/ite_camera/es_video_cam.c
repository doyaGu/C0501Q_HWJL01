#include "mediastreamer2/mscommon.h"
#include "mediastreamer2/msvideo.h"
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
#include "mediastreamer2/mswebcam.h"
#include "mediastreamer2/rfc3984.h"
#include <stdlib.h>
#include "ite/itp.h"
#include "pthread.h"
#include "video_encoder/video_encoder.h"
//#ifdef CFG_XCPU_MSGQ
//#include "tsi/mmp_tsi.h"
//#include "xcpu_master/itx.h"
//#endif
//=============================================================================
//                Constant Definition
//=============================================================================
#define SLICE_NUM 16

//=============================================================================
//                Structure Definition
//=============================================================================

typedef struct _SLICE_DATA {
    uint8_t  *pcSliceBuffer;
    uint32_t pnSliceSize[SLICE_NUM];
    uint32_t pnSliceTS[SLICE_NUM];
    int      nSliceCount;
} SLICE_DATA;

/**
 * ts camera encoder info
 **/
typedef struct ES_CAM_ENC_INFO_T
{
    ms_mutex_t     es_cam_mutex;

    uint32_t       tsi_idx;

    // network maximum transmission unit in bytes
    int            mtu;

    /*added for basic Camera info */
    MSVideoSize    vsize;
    float          fps;
    SLICE_DATA     slice;
    Rfc3984Context *packer;
    int            mode;
    uint8_t        *es_ring_buf;
    uint32_t       es_ring_size;
    bool           ring_ctrl;
	int            mobile_call;
    int            only_ring;
    uint8_t        use_cnt;
} ES_CAM_ENC_INFO;

typedef struct VIDEO_INFO_TAG
{
    bool         bused;  
    bool         bmobile;    
} VIDEO_INFO;
//=============================================================================
//                Global Data Definition
//=============================================================================
static FILE     *fp               = 0;
static bool     remain_frame      = false;
static uint8_t  *remain_frame_ptr = NULL;
static uint32_t remain_frame_size = 0;
static uint32_t time_ticks        = 0;

static VIDEO_INFO video_use_info[MAX_USER_NUM] = {0};
static uint32_t  in_use_cnt[2] = {0};
static bool es_have_startup = false;
static bool video_lock = false;
static VIDEO_SAMPLE    *vidSample[2]     = {NULL, NULL};
//=============================================================================
//                Private Function Definition
//=============================================================================
static void
es_enc_init(
    MSFilter    *f)
{
    ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)ms_new(ES_CAM_ENC_INFO, 1);
    printf("[MSEsEnc] init\n");
    do
    {
        if (!f || !pEnc_info)
        {
            ms_error("es cam: Null pointer (0x%x,0x%x) ! %s[%d]\n",
                     f, pEnc_info, __FUNCTION__, __LINE__);
            break;
        }
        //-------------------------------
        // initial
        memset(pEnc_info, 0x0, sizeof(ES_CAM_ENC_INFO));
        pEnc_info->mtu          = 1440;//ms_get_payload_max_size();
        pEnc_info->tsi_idx      = 0;

        /*add for configure_video_source */
        pEnc_info->vsize.width  = 1280;
        pEnc_info->vsize.height = 720;
        pEnc_info->fps          = 30.0;
        memset(&pEnc_info->slice, NULL, sizeof(SLICE_DATA));
        pEnc_info->packer       = NULL;
        pEnc_info->mode         = 1; // make sure RFC3984 packetization mode is Fregmentation Unit (FU-A)
        pEnc_info->es_ring_buf  = NULL;
        pEnc_info->es_ring_size = 1024000;
        pEnc_info->ring_ctrl    = false;

        ms_mutex_init(&pEnc_info->es_cam_mutex, NULL);
        f->data                 = pEnc_info;
    } while (0);

    return;
}

static void
es_enc_uninit(
    MSFilter    *f)
{
    ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
    printf("[MSEsEnc] uninit\n");
    do
    {
        if (!f || !pEnc_info)
            break;

        ms_mutex_destroy(&pEnc_info->es_cam_mutex);
        ms_free(pEnc_info);
    } while (0);

    return;
}

static void
es_enc_preprocess(
    MSFilter    *f)
{
	uint32_t i = 0;
    ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;

    printf("[MSEsEnc] preprocess\n");
    do
    {
        if (!f || !pEnc_info)
            break;

		if(pEnc_info->mobile_call)
		{
        	VideoEncoder_Open(1);
			VideoEncoder_SetSreamstate(1, true);
			VideoEncoder_SetSreamUserNum(1, true);
		}	
		else
		{
        	if (es_have_startup == false)
        	{
				VideoEncoder_Open(0);	
				es_have_startup = true;
        	}
			VideoEncoder_SetSreamstate(0, true);
			VideoEncoder_SetSreamUserNum(0, true);
		}

        /*add for rfc3984 */
        pEnc_info->packer = rfc3984_new();
        rfc3984_set_mode(pEnc_info->packer, pEnc_info->mode);
        rfc3984_enable_stap_a(pEnc_info->packer, FALSE);
    } while (0);
	
	for(i=0;i<MAX_USER_NUM;i++)
	{
		if(video_use_info[i].bused == false)
		{
			pEnc_info->use_cnt = i;
    		video_use_info[pEnc_info->use_cnt].bused = true;
			if(pEnc_info->mobile_call)
			{
				video_use_info[pEnc_info->use_cnt].bmobile = true;
				in_use_cnt[1]++;
			}
			else
			{
				video_use_info[pEnc_info->use_cnt].bmobile = false;
				in_use_cnt[0]++;
			}
			break;
		}
	}

    return;
}

//#define TMP_BUF_SIZE	(5 * 1024 * 1024)
//static uint8_t tmpb[TMP_BUF_SIZE];
//static int tmp_len = 0;
//static uint8_t *last_ptr = &tmpb[0];
static uint32_t old_time = 0;

static void
es_enc_process(
    MSFilter    *f)
{
    ES_CAM_ENC_INFO *pEnc_info     = (ES_CAM_ENC_INFO *)f->data;
    int             error          = 0, send_status = 1;
    //uint32_t            stream_data_size = 1316;
    uint32_t        valid_size     = 0;
    uint32_t        tmp_valid_size = 0;
    uint8_t         *pTsi_buf      = 0;
    uint8_t         *tmp_buf       = 0;
    uint32_t        i, ts = time_ticks;
    bool            ring_ctrl      = false;
    uint8_t         k = 0;
    //VIDEO_SAMPLE    *vidSample     = NULL;
    //VIDEO_SAMPLE    *curr          = NULL;
    VIDEO_SAMPLE    *prev          = NULL;
    static uint32_t frmCnt         = 0;

#ifdef ENABLE_VIDEO_MULTICAST
	ES_CAM_ENC_INFO     *act_pEnc_info = (ES_CAM_ENC_INFO *)rtp_session_get_call();
	if(act_pEnc_info)
	{
		printf("now idx= %d\n", act_pEnc_info->use_cnt);
		rtp_session_reset_multicast();
#ifdef CFG_DUAL_STREAM		
		if (video_use_info[act_pEnc_info->use_cnt].bmobile)
			VideoEncoder_SetSreamstate(0, false);
		else
			VideoEncoder_SetSreamstate(1, false);
#endif		
		for(i=0;i<MAX_USER_NUM;i++)
		{
	    	if(act_pEnc_info->use_cnt != i)
	        	video_use_info[i].bused = false;
		}
		rtp_session_set_call(NULL);
	}
    if(!f || !pEnc_info)    return;
#else	
    if(!f || !pEnc_info)    return;  
#endif  

#if defined(ENABLE_VIDEO_MULTICAST)
	if(pEnc_info->mobile_call && video_use_info[pEnc_info->use_cnt].bused)
		k = 1;
	else
	{
	    for(i=0;i<MAX_USER_NUM;i++)
		{
		    if(video_use_info[i].bused && !video_use_info[i].bmobile && pEnc_info->use_cnt == i)
		    {
				k = 0;
		        break;
		    }
		    else if(video_use_info[i].bused && !video_use_info[i].bmobile && pEnc_info->use_cnt != i)
			{
				k = 0;
				if (vidSample[k] != NULL && !vidSample[k]->send_flag[pEnc_info->use_cnt])
				{		
					vidSample[k]->reused_cnt++;
					vidSample[k]->send_flag[pEnc_info->use_cnt] = 1;
				}
		        return;
			}
		}
		if(i==MAX_USER_NUM)
			return;
	}
#else
	if(pEnc_info->mobile_call)
		k = 1;
	else
		k = 0;
	
	if(pEnc_info->only_ring && !video_lock)
	{
		if (vidSample[k] != NULL && !vidSample[k]->send_flag[pEnc_info->use_cnt])
		{
			vidSample[k]->reused_cnt++;
			vidSample[k]->send_flag[pEnc_info->use_cnt] = 1;
		}
		return;
	}  
#endif	
	if(video_lock)
		return;
	else
		video_lock = true;
    do
    {
		//printf("++++++index now=%d\n", pEnc_info->use_cnt);
        //mblk_t *inm = 0;

        //if( !(pTsi_buf = malloc(2*1024)) )     printf("malloc fail !!");

        //ms_mutex_lock(&pEnc_info->es_cam_mutex);

		/*get new video*/
		if (vidSample[k] == NULL)
        {
            VideoEncoder_GetSample(&vidSample[k], k);
			if (vidSample[k] != NULL)
			{
	            vidSample[k]->reused_cnt = 0;
				for(i=0;i<MAX_USER_NUM;i++)
					vidSample[k]->send_flag[i] = 0;
				//printf("new video ts(%d)\n", vidSample[k]->timestamp);
			}
        }
		/*send out video*/
        if (vidSample[k] != NULL && !vidSample[k]->send_flag[pEnc_info->use_cnt])
        {
            mblk_t  *m          = NULL;
            MSQueue nalus;
            int     nSliceCount = -1;

            ms_queue_init(&nalus);

            VIDEO_SAMPLE *curr = vidSample[k];
            do
            {
                if (curr->streamId == k)
                {	
                nSliceCount++;
                if (nSliceCount >= 16)
                    printf("slice too bigger=%d\n", nSliceCount);
                //printf("Data %d %d\n", curr->size, curr->timestamp);
                m = allocb(curr->size, 0);
                if (m != NULL)
                {
                    memcpy(m->b_wptr, curr->addr, curr->size);
                    m->b_wptr += curr->size;
                    m->b_rptr += 4;
#if defined(CFG_LEAF_ENABLE)
					ms_queue_put(f->outputs[0], m);
#else
                    ms_queue_put(&nalus, m);
                    if (!ms_queue_empty(&nalus))
                        rfc3984_pack(pEnc_info->packer, &nalus, f->outputs[0], curr->timestamp * 90LL);
#endif					
                }
                }
                curr = curr->next;
            } while (curr != NULL);
			vidSample[k]->reused_cnt++;
			vidSample[k]->send_flag[pEnc_info->use_cnt] = 1;
        }
		//else
		//	printf("index(%d) is send\n", pEnc_info->use_cnt);
		/*check for video free*/
		if (vidSample[k] != NULL && vidSample[k]->reused_cnt >= in_use_cnt[k])
        {
			//printf("free video in\n");	
            // free video sample list
            VIDEO_SAMPLE *release_curr = vidSample[k];
            while (release_curr != NULL)
            {
                prev = release_curr;
                release_curr = release_curr->next;
                free(prev);
                prev = NULL;
            }
            vidSample[k] = NULL;
			//printf("free video out\n");	
        }
        //send_status = 0;
        //ms_mutex_unlock(&pEnc_info->es_cam_mutex);
		//printf("------index=%d\n", pEnc_info->use_cnt);
    } while (0);
//#ifdef ENABLE_VIDEO_MULTICAST
//    rtp_session_set_data_status(send_status);
//#endif
	video_lock = false;
    return;
}

static void
es_enc_postprocess(
    MSFilter    *f)
{
    ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
    uint8_t stop_stream_0 = 1;
	uint32_t i = 0;

    printf("[MSEsEnc] postprocess\n");
    do
    {
        if (!f || !pEnc_info)
            break;
		
		ms_mutex_lock(&pEnc_info->es_cam_mutex);
		
        /*add for rfc3984 */
        rfc3984_destroy(pEnc_info->packer);
        pEnc_info->packer             = NULL;
		
        video_use_info[pEnc_info->use_cnt].bused = false;
		video_use_info[pEnc_info->use_cnt].bmobile = false;
		for (i = 0; i < MAX_USER_NUM; i++)
		{
			if (video_use_info[i].bused && !video_use_info[i].bmobile)
				stop_stream_0 = 0;
		}

		if (stop_stream_0 && !video_use_info[pEnc_info->use_cnt].bmobile)
		{
			VideoEncoder_SetSreamstate(0, false);
			if (vidSample[0] != NULL)
        	{
				VIDEO_SAMPLE *curr = NULL;
    			VIDEO_SAMPLE *prev = NULL;
            	// free video sample list
            	curr = vidSample[0];
            	while (curr != NULL)
            	{
            		prev = curr;
            		curr = curr->next;
            		free(prev);
            		prev = NULL;
            	}
            	vidSample[0] = NULL;
				printf("######close video(0)######\n");	
        	}
		}
		else if (video_use_info[pEnc_info->use_cnt].bmobile)
		{
			VideoEncoder_SetSreamstate(1, false);
			if (vidSample[1] != NULL)
        	{
				VIDEO_SAMPLE *curr = NULL;
    			VIDEO_SAMPLE *prev = NULL;
            	// free video sample list
            	curr = vidSample[1];
            	while (curr != NULL)
            	{
            		prev = curr;
            		curr = curr->next;
            		free(prev);
            		prev = NULL;
            	}
            	vidSample[1] = NULL;
				printf("######close video(1)######\n");	
        	}
		}
		
        for (i = 0; i < MAX_USER_NUM; i++)
        {
            if (video_use_info[i].bused)
                break;
            else if (i == (MAX_USER_NUM - 1) && !video_use_info[i].bused)
            {
				printf("######close video all######\n");	
    #ifdef CFG_XCPU_MSGQ
                VideoEncoder_Close();
    #endif
                es_have_startup = false;
            }
        }
		if(pEnc_info->mobile_call)
		{	
			in_use_cnt[1]--;
			VideoEncoder_SetSreamUserNum(1, false);
		}
		else
		{
			in_use_cnt[0]--;
			VideoEncoder_SetSreamUserNum(0, false);
		}
		ms_mutex_unlock(&pEnc_info->es_cam_mutex);
    } while (0);

    return;
}

#if 1 /*add for configure_video_source */
static int es_enc_set_vsize(MSFilter *f, void *data)
{
    ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
    pEnc_info->vsize = *(MSVideoSize *)data;
    /* TODO: switch output res SIF, D1, 720P */
    return 0;
}

static int es_enc_get_vsize(MSFilter *f, void *data)
{
    ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
    *(MSVideoSize *)data = pEnc_info->vsize;
    return 0;
}

static int es_enc_set_fps(MSFilter *f, void *data)
{
    ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
    pEnc_info->fps = *(float *)data;
    return 0;
}

static int es_enc_get_fps(MSFilter *f, void *data)
{
    ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
    *(float *)data = pEnc_info->fps;
    return 0;
}

static int
es_enc_set_mtu(
    MSFilter    *f,
    void        *arg)
{
    ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;

    do
    {
        if (!f || !pEnc_info)
            break;

        ms_mutex_lock(&pEnc_info->es_cam_mutex);

        pEnc_info->mtu = *(int *)arg;

        ms_mutex_unlock(&pEnc_info->es_cam_mutex);
    } while (0);

    return 0;
}

static int 
es_enc_set_mobile(MSFilter *f, void* data)
{
	ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
	pEnc_info->mobile_call = *(int*)data;
	return 0;
}

static int 
es_enc_set_only_ring(MSFilter *f, void* data)
{
	ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
	pEnc_info->only_ring = *(int*)data;
	return 0;
}
#endif

///////////////////////////////////
// ts camera API
///////////////////////////////////
static void
es_cam_detect(
    MSWebCamManager     *obj)
{
    extern MSWebCamDesc es_cam_desc;
    MSWebCam            *cam = ms_web_cam_new(&es_cam_desc);
    ms_web_cam_manager_add_cam(obj, cam);
}

static void
es_cam_init(
    MSWebCam    *cam)
{
    cam->name = ms_strdup("Es camera");
}

static MSFilter *
es_cam_create_reader(
    MSWebCam    *obj)
{
    extern MSFilterDesc ms_es_enc_desc;
    return ms_filter_new_from_desc(&ms_es_enc_desc);
}

//=============================================================================
//                Public Function Definition
//=============================================================================

#if 1
static MSFilterMethod enc_methods[] =
{
    {MS_FILTER_SET_FPS,         es_enc_set_fps},
    {MS_FILTER_GET_FPS,         es_enc_get_fps},
    {MS_FILTER_SET_VIDEO_SIZE,  es_enc_set_vsize},
    {MS_FILTER_GET_VIDEO_SIZE,  es_enc_get_vsize},
    // {MS_FILTER_ADD_FMTP,        es_enc_add_fmtp},
    // {MS_FILTER_SET_BITRATE,    es_enc_set_br},
    // {MS_FILTER_GET_BITRATE,    es_enc_get_br},
    {MS_FILTER_SET_MTU,         es_enc_set_mtu},
    // {MS_FILTER_REQ_VFU,         ts_enc_req_vfu},
    {MS_FILTER_SET_MOBILE,      es_enc_set_mobile},
    {MS_FILTER_SET_ONLY_RING,   es_enc_set_only_ring},
    {0, NULL}
};
#endif

MSFilterDesc ms_es_enc_desc =
{
    MS_ES_CAM_ID,
    "MSEsEnc",
    N_("ES stream by pass."),
    MS_FILTER_OTHER,
    "es",
    0, /*MS_YUV420P is assumed on this input */
    1,
    es_enc_init,
    es_enc_preprocess,
    es_enc_process,
    es_enc_postprocess,
    es_enc_uninit,
    enc_methods
};

MSWebCamDesc es_cam_desc =
{
    "Es camera",
    &es_cam_detect,
    &es_cam_init,
    &es_cam_create_reader,
    NULL
};