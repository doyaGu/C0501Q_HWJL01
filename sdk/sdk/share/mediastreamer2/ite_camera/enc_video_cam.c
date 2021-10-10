#include "mediastreamer2/mscommon.h"
#include "mediastreamer2/msvideo.h"
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
#include "mediastreamer2/mswebcam.h"
#include "mediastreamer2/rfc3984.h"

#include <stdlib.h>
#include "ite/itp.h"
//#include "pthread.h"

#include "video_encoder/video_encoder_it9910.h"

#define CORRECT_RFC3984
#define GPIO_BASE               0xDE000000
//#define CFG_DUAL_STREAM
//=============================================================================
//                Constant Definition
//=============================================================================
/*define video encode parameter*/
#if defined(CFG_DUAL_STREAM)
    #define VIDEO_STREAM_NUM            2
    #define VIDEO_SAMPLE_NUM            7
    #define VIDEO_SAMPLE_0_SIZE         200 * 1024
    #define VIDEO_SAMPLE_1_SIZE         100 * 1024

pthread_mutex_t video_mutex[2] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};    
#else
    #define VIDEO_STREAM_NUM            1
    #define VIDEO_SAMPLE_NUM            7
    #define VIDEO_SAMPLE_0_SIZE         500 * 1024
    #define VIDEO_SAMPLE_1_SIZE         100 * 1024

pthread_mutex_t video_mutex[1] = {PTHREAD_MUTEX_INITIALIZER};    
#endif

#define VIDEO_FRAME_RATE            30
//=============================================================================
//                Macro Definition
//=============================================================================


//=============================================================================
//                Structure Definition
//=============================================================================

/**
 * ts camera encoder info
 **/
typedef struct ES_CAM_ENC_INFO_T
{
    ms_mutex_t         es_cam_mutex;

    uint32_t           tsi_idx;

    // network maximum transmission unit in bytes
    int                mtu;
    /*added for basic Camera info */
    MSVideoSize		   vsize;
    float		       fps;
    Rfc3984Context	   *packer;
    int			       mode;
	int                mobile_call;
    int                only_ring;
    uint8_t            use_cnt;
} ES_CAM_ENC_INFO;

typedef struct VIDEO_SAMPLE_TAG
{
    mblk_t*      m_video;
    bool         bGetStream;  
    uint32_t     dataSize;   
    uint32_t     timeStamp; 
    bool         bIFrame;    
} VIDEO_SAMPLE;

typedef struct VIDEO_INFO_TAG
{
    bool         bused;  
    bool         bmobile;    
} VIDEO_INFO;

//=============================================================================
//                Global Data Definition
//=============================================================================
//static FILE      *fp = 0;
static VIDEO_SAMPLE*                gptVideoSample[VIDEO_STREAM_NUM][VIDEO_SAMPLE_NUM];
static uint32_t                     gVideoSendIdx[VIDEO_STREAM_NUM];
static uint32_t                     gVideoinitTSIdx[VIDEO_STREAM_NUM] = {0};

static VIDEO_INFO video_use_info[MAX_USER_NUM] = {0};
static uint8_t    use_cnt = 0;
static bool       es_have_startup = false, encoder_have_init = false;
static bool       I_frame_send = false;
static uint32_t   lastTick;

MSQueue              nalus[VIDEO_STREAM_NUM];
//=============================================================================
//                Private Function Definition
//=============================================================================

static uint32_t ESGetClock(void)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0)
        ithPrintf("gettimeofday failed!\n");
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static uint32_t ESGetDuration(uint32_t clock)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0)
        ithPrintf("gettimeofday failed!\n");
    return (unsigned int)(tv.tv_sec*1000+tv.tv_usec/1000) - clock;
}

static void
getVideoStream(
    VIDEO_STREAM_INFO* streamInfo)
{
    gptVideoSample[streamInfo->streamIndex][streamInfo->bufIndex]->bIFrame     = streamInfo->bIFrame;
    gptVideoSample[streamInfo->streamIndex][streamInfo->bufIndex]->dataSize    = streamInfo->dataSize;
    gptVideoSample[streamInfo->streamIndex][streamInfo->bufIndex]->timeStamp   = streamInfo->timeStamp;
    gptVideoSample[streamInfo->streamIndex][streamInfo->bufIndex]->bGetStream  = true;    
}

static void
es_enc_init(
    MSFilter    *f)
{
    ES_CAM_ENC_INFO     *pEnc_info = (ES_CAM_ENC_INFO *)ms_new(ES_CAM_ENC_INFO, 1);
    VIDEO_ENCODE_PARAMETER    enPara;
    uint32_t          i, j;
    
    ithPrintf("[MSEsEnc] init\n");
    do{
        if( !f || !pEnc_info )
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
        pEnc_info->vsize.width = SENSOR_WIDTH;
        pEnc_info->vsize.height = SENSOR_HEIGHT;
        pEnc_info->fps = VIDEO_FRAME_RATE;
        pEnc_info->packer = NULL;
        pEnc_info->mode = 1; // make sure RFC3984 packetization mode is Fregmentation Unit (FU-A)        
    
        ms_mutex_init(&pEnc_info->es_cam_mutex, NULL);
        f->data = pEnc_info;

		pEnc_info->use_cnt = use_cnt;
		video_use_info[pEnc_info->use_cnt].bused = true;
		video_use_info[pEnc_info->use_cnt].bmobile = false;
		use_cnt++;
		
        VideoEncoder_SetStreamBufCallBack(getVideoStream);
        //VideoEncoder_Open();
    }while(0);
    return;
}

static void
es_enc_uninit(
    MSFilter    *f)
{
    ES_CAM_ENC_INFO     *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
    uint32_t            i, j, k;
    ithPrintf("[MSEsEnc] uninit\n");
    do{
        if( !f || !pEnc_info )    break;               

	    video_use_info[pEnc_info->use_cnt].bused = false;
	    for(j=0;j<use_cnt;j++)
	    {
	        if(video_use_info[j].bused)
	            break;
	        else if(j==(use_cnt-1) && !video_use_info[j].bused)
	        {
				VideoEncoder_Close();
				
				for (k=0; k < VIDEO_STREAM_NUM; k++)
				    for (i=0; i < VIDEO_SAMPLE_NUM; i++)
        		    {
            	    	if (gptVideoSample[k][i]->m_video != NULL)        	    
                    		freeb(gptVideoSample[k][i]->m_video);
				    		gptVideoSample[k][i]->m_video = NULL;
        		    }
				
				for (k=0; k < VIDEO_STREAM_NUM; k++) 
				    for (i=0; i < VIDEO_SAMPLE_NUM; i++)
				    {
				    	if (gptVideoSample[k][i] != NULL) 
				    	{       	    
				    		free(gptVideoSample[k][i]);
				    		gptVideoSample[k][i] = NULL;
				    	}
				    }
        	    use_cnt = 0;
				encoder_have_init = false;
            }
        }
      
        ms_mutex_destroy(&pEnc_info->es_cam_mutex);
        ms_free(pEnc_info);
    }while(0);

    return;
}

static void
es_enc_preprocess(
    MSFilter    *f)
{
    ES_CAM_ENC_INFO     *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
	uint32_t            i = 0;

    ithPrintf("[MSEsEnc] preprocess\n");
    do{
        if( !f || !pEnc_info )    break;

        //if( !(fp = fopen("A:/video.es", "rb")) )  ithPrintf("open file fail !!");       
        //VideoEncoder_Fire();
        /*add for rfc3984 */
        pEnc_info->packer = rfc3984_new();
        rfc3984_set_mode(pEnc_info->packer, pEnc_info->mode);
        rfc3984_enable_stap_a(pEnc_info->packer, FALSE);       
        
        //ms_queue_init(&nalus);
		
        if(pEnc_info->mobile_call)
        {
        	ms_queue_init(&nalus[1]);
        	for (i=0; i < VIDEO_SAMPLE_NUM; i++)
            {
                if (NULL == gptVideoSample[1][i])
                    gptVideoSample[1][i] = (VIDEO_SAMPLE *)malloc(sizeof(VIDEO_SAMPLE));
            
                if (NULL != gptVideoSample[1][i])
                    memset(gptVideoSample[1][i], 0, sizeof(VIDEO_SAMPLE));
            }
			for (i=0; i < VIDEO_SAMPLE_NUM; i++)
            {
                gptVideoSample[1][i]->m_video    = allocb(VIDEO_SAMPLE_1_SIZE, 0);
                gptVideoSample[1][i]->bGetStream = false;
                
                VideoEncoder_SetStreamBuf(1, i, gptVideoSample[1][i]->m_video->b_wptr);
                ithPrintf("Allocate %x\n", gptVideoSample[1][i]->m_video->b_wptr);
            }
        	video_use_info[pEnc_info->use_cnt].bmobile = true;
        	gVideoSendIdx[1] = 0;
			gVideoinitTSIdx[1] = 0;
        	VideoEncoder_Open(1);
        }	
		else
		{
			if(encoder_have_init == true)
				return;
			ms_queue_init(&nalus[0]);
			for (i=0; i < VIDEO_SAMPLE_NUM; i++)
            {
                if (NULL == gptVideoSample[0][i])
                    gptVideoSample[0][i] = (VIDEO_SAMPLE *)malloc(sizeof(VIDEO_SAMPLE));
            
                if (NULL != gptVideoSample[0][i])
                    memset(gptVideoSample[0][i], 0, sizeof(VIDEO_SAMPLE));
            }
			for (i=0; i < VIDEO_SAMPLE_NUM; i++)
            {
                gptVideoSample[0][i]->m_video    = allocb(VIDEO_SAMPLE_0_SIZE, 0);
                gptVideoSample[0][i]->bGetStream = false;
                
                VideoEncoder_SetStreamBuf(0, i, gptVideoSample[0][i]->m_video->b_wptr);
                ithPrintf("Allocate %x\n", gptVideoSample[0][i]->m_video->b_wptr);
            }
			gVideoSendIdx[0] = 0;
			gVideoinitTSIdx[0] = 0;
			VideoEncoder_Open(0);
			lastTick = itpGetTickCount();
			encoder_have_init = true;
		}
    }while(0);

    return;
}

static void
es_enc_process(
    MSFilter    *f)
{
    ES_CAM_ENC_INFO     *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
    mblk_t *m = NULL;
    //MSQueue              nalus;
    uint32_t diff;
    uint32_t cnt = 0;   
    uint8_t  i, k = 0;

    //ithPrintf("[MSEsEnc] process\n");
#if defined(ENABLE_VIDEO_MULTICAST)    
    ES_CAM_ENC_INFO     *act_pEnc_info = (ES_CAM_ENC_INFO *)rtp_session_get_call();
	if(act_pEnc_info && I_frame_send == false)
	{
		printf("now idx= %d\n", act_pEnc_info->use_cnt);
		rtp_session_reset_multicast();
		for(i=0;i<use_cnt;i++)
		{
	    	if(act_pEnc_info->use_cnt != i)
	        	video_use_info[i].bused = false;
		}
		rtp_session_set_call(NULL);
	}
    if(!f || !pEnc_info)    return;
#else	
    if(!f || !pEnc_info || pEnc_info->only_ring)    return;     
#endif       

#if defined(ENABLE_VIDEO_MULTICAST)
	if(pEnc_info->mobile_call && video_use_info[pEnc_info->use_cnt].bused)
		k = 1;
	else
	{
	    for(i=0;i<use_cnt;i++)
		{
		    if(video_use_info[i].bused && !video_use_info[i].bmobile && pEnc_info->use_cnt == i)
		    {
		        break;
		    }
		    else if(video_use_info[i].bused && !video_use_info[i].bmobile && pEnc_info->use_cnt != i)
		        return;
		}
		if(i==use_cnt)
			return;
		else
			k = 0;
	}
#else
	if(pEnc_info->mobile_call)
		k = 1;
	else
		k = 0;
#endif	

    //ms_mutex_lock(&pEnc_info->es_cam_mutex); 
	pthread_mutex_lock(&video_mutex[k]);	
       
    //do {        
    //for (k=0; k < VIDEO_STREAM_NUM; k++)
    	if(itpGetTickDuration(lastTick) > 1000)
    	{
    		printf("reset sensor\n");
			//Set GPIO30 to Low
    		AHB_WriteRegisterMask(GPIO_BASE + 0x10, (1 << 29), (1 << 29));
    		//Set GPIO30 Output Mode
    		AHB_WriteRegisterMask(GPIO_BASE + 0x8, (1 << 29), (1 << 29));
    		//Set GPIO30 Mode0
    		AHB_WriteRegisterMask(GPIO_BASE + 0x94, (0x0 << (13 * 2)), (0x3 << (13 * 2)));
			usleep(10000);
			AHB_WriteRegisterMask(GPIO_BASE + 0xC, (1 << 29), (1 << 29));
			mmpSensorInitialize();
			
    		mmpSensorPowerOn(MMP_FALSE, MMP_FALSE);
			mmpSensorPowerOn(MMP_TRUE, MMP_FALSE);

			ithWriteRegMaskH(0x62, 0x1 << 3, 0x1 << 3);
			usleep(1000);
			ithWriteRegMaskH(0x62, 0x0 << 3, 0x1 << 3);
			ithWriteRegMaskH(0x2018, 0x1 << 15, 0x1 << 15);
			lastTick = itpGetTickCount();
    	}
        if (gptVideoSample[k][gVideoSendIdx[k]]->bGetStream)
        {            
        	lastTick = itpGetTickCount();
            //ithPrintf("BufAddr %x\n", gptVideoSample[k][gVideoSendIdx[k]]->m_video->b_wptr);
            if(gVideoinitTSIdx[k] == 0)
				gVideoinitTSIdx[k] = gptVideoSample[k][gVideoSendIdx[k]]->timeStamp;
#ifdef CORRECT_RFC3984
            if (gptVideoSample[k][gVideoSendIdx[k]]->bIFrame)
            {        	  
                VIDEO_HEADER_INFO headInfo;
#if defined(ENABLE_VIDEO_MULTICAST)				  
                I_frame_send = true;
#endif            	  
                headInfo.streamIndex = k;
                VideoEncoder_GetHdrInfo(&headInfo);
            	              	
                m = allocb(headInfo.SPS_Size, 0);
		    	        
                if (m != NULL)
                {
                    memcpy(m->b_wptr, headInfo.SPSBuf, headInfo.SPS_Size);
                    m->b_wptr += headInfo.SPS_Size;
                    m->b_rptr += 4;
#if defined(CFG_LEAF_ENABLE)					
                    ms_queue_put(f->outputs[0], m);
#else
					ms_queue_put(&nalus[k], m);
                    if (!ms_queue_empty(&nalus[k]))
                    {
                    	  rfc3984_pack(pEnc_info->packer, &nalus[k], f->outputs[0], (gptVideoSample[k][gVideoSendIdx[k]]->timeStamp - gVideoinitTSIdx[k])*90LL);		    		    
                    }
#endif					
                }
			          
                m = allocb(headInfo.PPS_Size, 0);
		    	        
                if (m != NULL)
                {
                    memcpy(m->b_wptr, headInfo.PPSBuf, headInfo.PPS_Size);
                    m->b_wptr += headInfo.PPS_Size;
                    m->b_rptr += 4;
#if defined(CFG_LEAF_ENABLE)					
                    ms_queue_put(f->outputs[0],m);
#else
					ms_queue_put(&nalus[k], m);
                    if (!ms_queue_empty(&nalus[k]))
                    {
                    	  rfc3984_pack(pEnc_info->packer, &nalus[k], f->outputs[0], (gptVideoSample[k][gVideoSendIdx[k]]->timeStamp - gVideoinitTSIdx[k])*90LL);		    		    
                    }
#endif					
                }
            }  
#if defined(ENABLE_VIDEO_MULTICAST)			
			else
                I_frame_send = false;
#endif			
#endif                                    
			if (gptVideoSample[k][gVideoSendIdx[k]]->m_video != NULL)
			{
				gptVideoSample[k][gVideoSendIdx[k]]->m_video->b_wptr += gptVideoSample[k][gVideoSendIdx[k]]->dataSize;
        	  
#ifdef CORRECT_RFC3984        	  
        	    gptVideoSample[k][gVideoSendIdx[k]]->m_video->b_rptr += 4;
#endif      
#if defined(CFG_LEAF_ENABLE)
        	    ms_queue_put(f->outputs[0],gptVideoSample[k][gVideoSendIdx[k]]->m_video);
#else
				ms_queue_put(&nalus[k],gptVideoSample[k][gVideoSendIdx[k]]->m_video);
        	    if (!ms_queue_empty(&nalus[k]))
        	    {
                    rfc3984_pack(pEnc_info->packer, &nalus[k], f->outputs[0], (gptVideoSample[k][gVideoSendIdx[k]]->timeStamp - gVideoinitTSIdx[k])*90LL);
#endif                              
                    if (k==0)
                        gptVideoSample[k][gVideoSendIdx[k]]->m_video      = allocb(VIDEO_SAMPLE_0_SIZE, 0);
                    else
                        gptVideoSample[k][gVideoSendIdx[k]]->m_video      = allocb(VIDEO_SAMPLE_1_SIZE, 0);
                  	
                    gptVideoSample[k][gVideoSendIdx[k]]->bGetStream   = false;
                     
                    VideoEncoder_SetStreamBuf(k, gVideoSendIdx[k], gptVideoSample[k][gVideoSendIdx[k]]->m_video->b_wptr);
                
                    gVideoSendIdx[k] = (gVideoSendIdx[k] + 1) % VIDEO_SAMPLE_NUM;
		    		    
                    //ithPrintf("m_video %x\n", gptVideoSample[k][gVideoSendIdx[k]]->m_video->b_wptr);
		    		    		    		
                    cnt++;
#if !defined(CFG_LEAF_ENABLE)
		        } //else {
#endif
		           // ithPrintf("ms_queue_empty\n");
		        //}
			}
        }
    //} while (0);
	pthread_mutex_unlock(&video_mutex[k]);
    //ms_mutex_unlock(&pEnc_info->es_cam_mutex);
    usleep(2000);
    //printf("es_enc_process %d\n", cnt);
    //ms_queue_destroy(&nalus);			
    return;
}

static void
es_enc_postprocess(
    MSFilter    *f)
{
    ES_CAM_ENC_INFO     *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
    ithPrintf("[MSEsEnc] postprocess\n");
    do{
        if( !f || !pEnc_info )    break;

        /*add for rfc3984 */
        rfc3984_destroy(pEnc_info->packer);
        pEnc_info->packer = NULL;                

    }while(0);

    return;
}

#if 1 /*add for configure_video_source */
static int es_enc_set_vsize(MSFilter *f, void* data)
{
	ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
	pEnc_info->vsize = *(MSVideoSize*)data;
	/* TODO: switch output res SIF, D1, 720P */
	return 0;
}

static int es_enc_get_vsize(MSFilter *f, void* data)
{
	ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
	*(MSVideoSize*)data = pEnc_info->vsize;
	return 0;
}

static int es_enc_set_fps(MSFilter *f, void* data)
{
	ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
	pEnc_info->fps = *(float*)data;
	return 0;
}

static int es_enc_get_fps(MSFilter *f, void* data)
{
	ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
	*(float*)data = pEnc_info->fps;
	return 0;
}

static int
es_enc_set_mtu(
    MSFilter    *f,
    void        *arg)
{
    ES_CAM_ENC_INFO     *pEnc_info = (ES_CAM_ENC_INFO *)f->data;

    do{
        if( !f || !pEnc_info )    break;

        ms_mutex_lock(&pEnc_info->es_cam_mutex);

        pEnc_info->mtu = *(int *)arg;

        ms_mutex_unlock(&pEnc_info->es_cam_mutex);
    }while(0);

    return 0;
}

static int es_enc_set_mobile(MSFilter *f, void* data)
{
	ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
	pEnc_info->mobile_call = *(int*)data;
	return 0;
}

static int 
es_enc_set_only_ring(MSFilter *f, void* data)
{
	uint8_t i = 0;
	ES_CAM_ENC_INFO *pEnc_info = (ES_CAM_ENC_INFO *)f->data;
	for(i=0;i<use_cnt;i++)
	{
		if(i != pEnc_info->use_cnt)
			video_use_info[i].bused = false;
	}
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
    MSWebCam *cam = ms_web_cam_new(&es_cam_desc);
    ms_web_cam_manager_add_cam(obj, cam);
}

static void
es_cam_init(
    MSWebCam    *cam)
{
    cam->name = ms_strdup("Es camera");
}

static MSFilter*
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

