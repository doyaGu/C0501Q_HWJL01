#if defined(HAVE_CONFIG_H)
#include "mediastreamer-config.h"
#endif

#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
#include "ortp/b64.h"

#ifdef HAVE_CONFIG_H
#include "mediastreamer-config.h"
#endif

#ifdef WIN32
#include <malloc.h> /* for alloca */
#endif

#include "ite/audio.h"

#if (CFG_CHIP_FAMILY != 9910)
#ifndef CFG_NET_ETHERNET_WIFI
    #define AEC_RUN_IN_ARM
#endif
    #include "type_def.h"
    #include "aecm_core.h"
    #include "basic_op.h"
    #include "hd_aec.h"
    #include "rfft_256.h"
#endif


#ifdef ENABLE_DUMP_AEC_DATA
#define EC_DUMP_ITE
#endif

typedef struct _AudioFlowController_{
    int target_samples;
    int total_samples;
    int current_pos;
    int current_dropped;
}AudioFlowController_;
 
void audio_flow_controller_init_(AudioFlowController_ *ctl){
    ctl->target_samples=0;
    ctl->total_samples=0;
    ctl->current_pos=0;
    ctl->current_dropped=0;
}
 
void audio_flow_controller_set_target_(AudioFlowController_ *ctl, int samples_to_drop, int total_samples){
    ctl->target_samples=samples_to_drop;
    ctl->total_samples=total_samples;
    ctl->current_pos=0;
    ctl->current_dropped=0;
}
 
static void discard_well_choosed_samples_(mblk_t *m, int nsamples, int todrop){
    int i;
    int16_t *samples=(int16_t*)m->b_rptr;
    int min_diff=32768;
    int pos=0;
 
 
#ifdef TWO_SAMPLES_CRITERIA
    for(i=0;i<nsamples-1;++i){
        int tmp=abs((int)samples[i]- (int)samples[i+1]);
#else
    for(i=0;i<nsamples-2;++i){
        int tmp=abs((int)samples[i]- (int)samples[i+1])+abs((int)samples[i+1]- (int)samples[i+2]);
#endif
        if (tmp<=min_diff){
            pos=i;
            min_diff=tmp;
        }
    }
    /*ms_message("min_diff=%i at pos %i",min_diff, pos);*/
#ifdef TWO_SAMPLES_CRITERIA
    memmove(samples+pos,samples+pos+1,(nsamples-pos-1)*2);
#else
    memmove(samples+pos+1,samples+pos+2,(nsamples-pos-2)*2);
#endif
 
    todrop--;
    m->b_wptr-=2;
    nsamples--;
    if (todrop>0){
        /*repeat the same process again*/
        discard_well_choosed_samples_(m,nsamples,todrop);
    }
}
 
mblk_t * audio_flow_controller_process_(AudioFlowController_ *ctl, mblk_t *m){
    if (ctl->total_samples>0 && ctl->target_samples>0){
        int nsamples=(m->b_wptr-m->b_rptr)/2;
        if (ctl->target_samples*16>ctl->total_samples){
            ms_warning("Too many samples to drop, dropping entire frames");
            m->b_wptr=m->b_rptr;
            ctl->current_pos+=nsamples;
        }else{
            int th_dropped;
            int todrop;
 
            ctl->current_pos+=nsamples;
            th_dropped=(ctl->target_samples*ctl->current_pos)/ctl->total_samples;
            todrop=th_dropped-ctl->current_dropped;
            if (todrop>0){
                if (todrop>nsamples) todrop=nsamples;
                discard_well_choosed_samples_(m,nsamples,todrop);
                /*ms_message("th_dropped=%i, current_dropped=%i, %i samples dropped.",th_dropped,ctl->current_dropped,todrop);*/
                ctl->current_dropped+=todrop;
            }
        }
        if (ctl->current_pos>=ctl->total_samples) ctl->target_samples=0;/*stop discarding*/
    }
    return m;
}
 
static const int flow_control_interval_ms=5000;
 
typedef struct SbcAECState{
    MSFilter *msF;
    MSBufferizer delayed_ref;
    MSBufferizer ref;
    MSBufferizer echo;
    MSBufferizer hw_ref;
    MSBufferizer hw_echo;
    //MSBufferizer hw_clean;

    
        bool_t hw_start;
        ms_thread_t hw_thread;	
        //ms_mutex_t hw_mutex;
        
    int framesize;
    int samplerate;
    int delay_ms;
    int AECframesize;
    int nominal_ref_samples;
    int min_ref_samples;
    AudioFlowController_ afc;
    bool_t echostarted;
    bool_t bypass_mode;
    bool_t using_zeroes;
    //queue_t echo_q;
    //queue_t ref_q;
#ifdef EC_DUMP_ITE
    queue_t echo_copy_q;
    queue_t ref_copy_q;
    queue_t clean_copy_q;
#endif
    ms_mutex_t mutex;
}SbcAECState;

#ifdef AEC_RUN_IN_ARM
void AEC_IN_ARM_INIT(int *AECframesize,int *AECdelayms){
    AEC_Init(&aec_config[0]);
    NR_Create(&anr_config[0], 1);
    AEC_Create(&aec_config[0]);
    TimeStretch_Create(&sndstrh_config[0]);    
    
    #if(CFG_AUDIO_SAMPLING_RATE == 16000)
    *AECframesize = 256;
    *AECdelayms   = CFG_AEC_DELAY_MS;
    #else
    *AECframesize = 128;
    *AECdelayms   = CFG_AEC_DELAY_MS;
    #endif
}

void AEC_IN_ARM_UNINIT(){
    AEC_Destroy(&aec_config[0]);
    TimeStretch_Destroy(&sndstrh_config[0]);    
}
#endif

static void sbc_aec_init(MSFilter *f){
    SbcAECState *s=(SbcAECState *)ms_new(SbcAECState,1);
 
    s->samplerate=CFG_AUDIO_SAMPLING_RATE;
    ms_bufferizer_init(&s->delayed_ref);
    ms_bufferizer_init(&s->echo);
    ms_bufferizer_init(&s->ref);
    ms_bufferizer_init(&s->hw_ref);
    ms_bufferizer_init(&s->hw_echo);
    //ms_bufferizer_init(&s->hw_clean);    
    
    
    s->using_zeroes=FALSE;
    s->echostarted=FALSE;
    s->bypass_mode=FALSE;
    s->hw_start=FALSE;
    
    //s->framesize=320;
    s->delay_ms=CFG_AEC_DELAY_MS;

    //qinit(&s->echo_q);
    //qinit(&s->ref_q);
#ifdef EC_DUMP_ITE
    qinit(&s->echo_copy_q);
    qinit(&s->ref_copy_q);
    qinit(&s->clean_copy_q);
#endif
    ms_mutex_init(&s->mutex,NULL);
 
    f->data=s;
#ifdef AEC_RUN_IN_ARM
    AEC_IN_ARM_INIT(&s->AECframesize,&s->delay_ms);
#else //9910
    iteAecCommand(AEC_CMD_INIT, 0, 0, 0, 0, &s->AECframesize);//AECframesize=144
#endif
    s->framesize = s->AECframesize; //s->framesize can be any size, equal to AECframesize more efficient  

}
 
static void sbc_aec_uninit(MSFilter *f){
    SbcAECState *s=(SbcAECState*)f->data;
 
    ms_bufferizer_uninit(&s->delayed_ref);
    ms_mutex_destroy(&s->mutex);
    ms_free(s);
}

static void *hw_engine_thread(void *arg) {
    
    SbcAECState *s=(SbcAECState*)arg;

    int hw_nbytes = s->AECframesize*2;
    int nbytes    = s->framesize*2;

    while(s->hw_start){
        mblk_t *ref,*echo;
        int err1,err2;
        
        ref=allocb(hw_nbytes, 0);
        echo=allocb(hw_nbytes, 0);
        ms_mutex_lock(&s->mutex);
        err1 = ms_bufferizer_read(&s->hw_echo,echo->b_wptr,hw_nbytes);
        err2 = ms_bufferizer_read(&s->hw_ref,ref->b_wptr,hw_nbytes);
        ms_mutex_unlock(&s->mutex);

        if (err1 && err2) {
            mblk_t *oecho = allocb(hw_nbytes, 0);
            memset(oecho->b_wptr, 0, hw_nbytes);
            echo->b_wptr+=hw_nbytes;
            ref->b_wptr+=hw_nbytes;
#ifdef AEC_RUN_IN_ARM
        #if(CFG_AUDIO_SAMPLING_RATE == 16000)
            //PAES_Process_Block(&aecm,echo->b_rptr, ref->b_rptr, oecho->b_wptr); //16K
            FreqWrapping(echo->b_rptr, ref->b_rptr, oecho->b_wptr, &aec_config[0]);
        #else
            //aecm_core(echo->b_rptr, ref->b_rptr, oecho->b_wptr, &aec_config[0]);
            EchoPlayBack(&aec_config[0], ref->b_rptr);
            EchoCapture(&aec_config[0], echo->b_rptr, oecho->b_wptr, FRAME_LEN);
            //TimeStretch(echo->b_rptr, oecho->b_wptr, &sndstrh_config[0]);
        #endif
#else
            iteAecCommand(AEC_CMD_PROCESS,(unsigned int) echo->b_rptr,(unsigned int) ref->b_rptr, (unsigned int) oecho->b_wptr,hw_nbytes, 0);
#endif
            oecho->b_wptr += hw_nbytes;
            ms_queue_put(s->msF->outputs[1],oecho);
#ifdef EC_DUMP_ITE
            putq(&s->clean_copy_q, dupmsg(oecho));
#endif
        }else{
           usleep(20000);
        }
        if (echo) freemsg(echo);
        if (ref) freemsg(ref);
        
        if(s->hw_start==FALSE){
            break;
        }
    }

    return NULL;
}

static void sbc_aec_start_hwthread(SbcAECState *e){
    //ms_mutex_init(&e->hw_mutex,NULL);
    if (e->hw_start == FALSE){
        pthread_attr_t attr;
        struct sched_param param;
        e->hw_start=TRUE;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 25*1024);       
        param.sched_priority = sched_get_priority_min(0) + 1;
        pthread_attr_setschedparam(&attr, &param);
        ms_thread_create(&e->hw_thread, &attr, hw_engine_thread, e);
    }
}

static void sbc_aec_stop_hwthread(SbcAECState *e){
    e->hw_start=FALSE;
    //ms_mutex_destroy(e->hw_mutex);
    ms_thread_join(e->hw_thread,NULL);      
}
 
static void sbc_aec_preprocess(MSFilter *f){
    SbcAECState *s=(SbcAECState*)f->data;
    mblk_t *m;
    int delay_samples=0;
    
    s->echostarted=FALSE;
    delay_samples=s->delay_ms*s->samplerate/1000;
 
    /* fill with zeroes for the time of the delay*/
    m=allocb(delay_samples*2,0);
    memset(m->b_wptr,0,delay_samples*2);
    m->b_wptr+=delay_samples*2;
    ms_bufferizer_put(&s->delayed_ref,m);
    s->min_ref_samples=-1;
    s->nominal_ref_samples=delay_samples;
    audio_flow_controller_init_(&s->afc);
    s->msF = f;
    sbc_aec_start_hwthread(s);
    
    //hw_engine_init();
    //hw_engine_link_filter(HW_EC_ID, f);
}
 
/*  inputs[0]= reference signal from far end (sent to soundcard)
 *  inputs[1]= near speech & echo signal    (read from soundcard)
 *  outputs[0]=  is a copy of inputs[0] to be sent to soundcard
 *  outputs[1]=  near end speech, echo removed - towards far end
*/
static void sbc_aec_process(MSFilter *f){
    SbcAECState *s=(SbcAECState*)f->data;
    int nbytes=s->framesize*2;
    mblk_t *refm;
    uint8_t *ref,*echo;
    if (s->bypass_mode) {
        while((refm=ms_queue_get(f->inputs[0]))!=NULL){
            ms_queue_put(f->outputs[0],refm);
        }
        while((refm=ms_queue_get(f->inputs[1]))!=NULL){
            ms_queue_put(f->outputs[1],refm);
        }
        return;
    }
    
      
    if (f->inputs[0]!=NULL){
        if (s->echostarted){
            while((refm=ms_queue_get(f->inputs[0]))!=NULL){
                mblk_t *cp=dupmsg(audio_flow_controller_process_(&s->afc,refm));
                ms_mutex_lock(&s->mutex);
                ms_bufferizer_put(&s->delayed_ref,cp);
                ms_bufferizer_put(&s->ref,refm);
                ms_mutex_unlock(&s->mutex);
            }
        }else{
            ms_warning("Getting reference signal but no echo to synchronize on.");
            ms_queue_flush(f->inputs[0]);
        }
    }
 
    ms_bufferizer_put_from_queue(&s->echo,f->inputs[1]);
 
    ref=(uint8_t*)alloca(nbytes);
    echo=(uint8_t*)alloca(nbytes);
    while (ms_bufferizer_read(&s->echo,echo,nbytes)>=nbytes){
        int avail;
        int avail_samples;
        mblk_t *clean;
        mblk_t *echo_to_ec;
        mblk_t *ref_to_ec;
 
        if (!s->echostarted) s->echostarted=TRUE;
        if ((avail=ms_bufferizer_get_avail(&s->delayed_ref))<((s->nominal_ref_samples*2)+nbytes)){
            /*we don't have enough to read in a reference signal buffer, inject silence instead*/
            refm=allocb(nbytes,0);
            memset(refm->b_wptr,0,nbytes);
            refm->b_wptr+=nbytes;
            ms_mutex_lock(&s->mutex);
            ms_bufferizer_put(&s->delayed_ref,refm);
            ms_queue_put(f->outputs[0],dupmsg(refm));
            ms_mutex_unlock(&s->mutex);
            if (!s->using_zeroes){
                ms_warning("Not enough ref samples, using zeroes");
                s->using_zeroes=TRUE;
            }
        }else{
            if (s->using_zeroes){
                ms_message("Samples are back.");
                s->using_zeroes=FALSE;
            }
            /* read from our no-delay buffer and output */
            refm=allocb(nbytes,0);
            if (ms_bufferizer_read(&s->ref,refm->b_wptr,nbytes)==0){
                ms_fatal("Should never happen");
            }
            refm->b_wptr+=nbytes;
            ms_queue_put(f->outputs[0],refm);
        }
 
        /*now read a valid buffer of delayed ref samples*/
        if (ms_bufferizer_read(&s->delayed_ref,ref,nbytes)==0){
            ms_fatal("Should never happen");
        }
        avail-=nbytes;
        avail_samples=avail/2;
        if (avail_samples<s->min_ref_samples || s->min_ref_samples==-1){
            s->min_ref_samples=avail_samples;
        }
 
        // put near-end and far-end data to queue
        echo_to_ec = allocb(nbytes, 0);        
        memcpy(echo_to_ec->b_wptr, echo, nbytes);
        echo_to_ec->b_wptr += nbytes;
        ref_to_ec = allocb(nbytes, 0);
        memcpy(ref_to_ec->b_wptr, ref, nbytes);
        ref_to_ec->b_wptr += nbytes;
        
        ms_mutex_lock(&s->mutex);
        //putq(&s->echo_q, echo_to_ec);
        //putq(&s->ref_q, ref_to_ec);
        ms_bufferizer_put(&s->hw_echo,echo_to_ec);//put to buffer
        ms_bufferizer_put(&s->hw_ref,ref_to_ec);  //put to buffer    
#ifdef EC_DUMP_ITE
        putq(&s->echo_copy_q, dupmsg(echo_to_ec));
        putq(&s->ref_copy_q, dupmsg(ref_to_ec));
#endif
        ms_mutex_unlock(&s->mutex);

        // clean=allocb(nbytes,0);
        // memset(clean->b_wptr,0,nbytes);
        // if(ms_bufferizer_read(&s->hw_clean,clean->b_wptr,nbytes)){
            // clean->b_wptr += nbytes;
            // ms_queue_put(f->outputs[1],clean);
        // }else{
            // if(clean) freemsg(clean);
        // }
        
    }
 
    /*verify our ref buffer does not become too big, meaning that we are receiving more samples than we are sending*/
    if (f->ticker->time % flow_control_interval_ms == 0 && s->min_ref_samples!=-1){
        int diff=s->min_ref_samples-s->nominal_ref_samples;
        if (diff>(nbytes/1)){
            int purge=diff-(nbytes/1);
            ms_warning("echo canceller: we are accumulating too much reference signal, need to throw out %i samples",purge);
            audio_flow_controller_set_target_(&s->afc,purge,(flow_control_interval_ms*s->samplerate)/1000);
        }
        s->min_ref_samples=-1;
    }
}
 
static void sbc_aec_postprocess(MSFilter *f){
    SbcAECState *s=(SbcAECState*)f->data;
    
    sbc_aec_stop_hwthread(s);
#ifdef AEC_RUN_IN_ARM 
    AEC_IN_ARM_UNINIT();
#endif 
    ms_bufferizer_flush (&s->delayed_ref);
    ms_bufferizer_flush (&s->echo);
    ms_bufferizer_flush (&s->ref);
    ms_bufferizer_flush (&s->hw_ref);
    ms_bufferizer_flush (&s->hw_echo);
    //ms_bufferizer_flush (&s->hw_clean);    
 
    //hw_engine_uninit();
    
    //flushq(&s->echo_q,0);
    //flushq(&s->ref_q,0);

#ifdef EC_DUMP_ITE
    FILE *echofile;
    FILE *reffile;
    FILE *cleanfile;
    mblk_t *echo;
    mblk_t *ref;
    mblk_t *clean;
    int nbytes=s->framesize*2;
    static int index = 0;
    char *fname;
    char USBPATH = 'D';
#if CFG_DOORBELL_INDOOR
    USBPATH = 'E';
#endif
    printf("save audio data in USB %c:/ \n",USBPATH);
    fname=ms_strdup_printf("%c:/echo%03d.raw",USBPATH,index);
    echofile=fopen(fname,"w");
    ms_free(fname);
    fname=ms_strdup_printf("%c:/ref%03d.raw",USBPATH,index);
    reffile=fopen(fname,"w");
    ms_free(fname);
    fname=ms_strdup_printf("%c:/clean%03d.raw",USBPATH,index);
    cleanfile=fopen(fname,"w");
    ms_free(fname);
    index++;
    while (1)
    {
        echo=ref=NULL;
        echo=getq(&s->echo_copy_q);
        ref=getq(&s->ref_copy_q);
        clean=getq(&s->clean_copy_q);
        if (echo && ref && clean)
        {
            fwrite(echo->b_rptr,nbytes,1,echofile);
            freemsg(echo);            
            fwrite(ref->b_rptr,nbytes,1,reffile);
            freemsg(ref);
            fwrite(clean->b_rptr,nbytes,1,cleanfile);
            freemsg(clean);
        }
        else
        {
            flushq(&s->echo_copy_q,0);
            flushq(&s->ref_copy_q,0);
            flushq(&s->clean_copy_q,0);
            fclose(echofile);
            fclose(reffile);
            fclose(cleanfile);
            break;
        }
    }
#endif
}

static int sbc_aec_method_1(MSFilter *f, void *arg){
    printf("do nothing for method 1, fix it later\n");
    return 0;
}
 
static int sbc_aec_method_2(MSFilter *f, void *arg){
    printf("do nothing for method 2, fix it later\n");
    return 0;
}
 
static int echo_canceller_set_delay(MSFilter *f, void *arg){
	SbcAECState *s=(SbcAECState*)f->data;
	s->delay_ms= *(int*)arg;
    return 0;
}
 
static int sbc_aec_method_4(MSFilter *f, void *arg){
    printf("do nothing for method 4, fix it later\n");
    return 0;
}
 
static int sbc_aec_method_5(MSFilter *f, void *arg){
    printf("do nothing for method 5, fix it later\n");
    return 0;
}
 
static int sbc_aec_method_6(MSFilter *f, void *arg){
    printf("do nothing for method 6, fix it later\n");
    return 0;
}
 
static int sbc_aec_method_7(MSFilter *f, void *arg){
    printf("do nothing for method 7, fix it later\n");
    return 0;
}
 
static int sbc_aec_method_8(MSFilter *f, void *arg){
    printf("do nothing for method 8, fix it later\n");
    return 0;
}
 
static MSFilterMethod sbc_aec_methods[]={
    {MS_FILTER_SET_SAMPLE_RATE          , sbc_aec_method_1},
    {MS_ECHO_CANCELLER_SET_TAIL_LENGTH    , sbc_aec_method_2},
    {MS_ECHO_CANCELLER_SET_DELAY        , echo_canceller_set_delay},
    {MS_ECHO_CANCELLER_SET_FRAMESIZE    , sbc_aec_method_4},
    {MS_ECHO_CANCELLER_SET_BYPASS_MODE    , sbc_aec_method_5},
    {MS_ECHO_CANCELLER_GET_BYPASS_MODE    , sbc_aec_method_6},
    {MS_ECHO_CANCELLER_GET_STATE_STRING    , sbc_aec_method_7},
    {MS_ECHO_CANCELLER_SET_STATE_STRING    , sbc_aec_method_8}
};
 
#ifdef _MSC_VER
 
MSFilterDesc ms_sbc_aec_desc={
    MS_SBC_AEC_ID,
    "MSSbcAEC",
    N_("Echo canceller using sbc aec"),
    MS_FILTER_OTHER,
    NULL,
    2,
    2,
    sbc_aec_init,
    sbc_aec_preprocess,
    sbc_aec_process,
    sbc_aec_postprocess,
    sbc_aec_uninit,
    sbc_aec_methods
};
 
#else
 
MSFilterDesc ms_sbc_aec_desc={
    .id=MS_SBC_AEC_ID,
    .name="MSSbcAEC",
    .text=N_("Echo canceller using sbc aec"),
    .category=MS_FILTER_OTHER,
    .ninputs=2,
    .noutputs=2,
    .init=sbc_aec_init,
    .preprocess=sbc_aec_preprocess,
    .process=sbc_aec_process,
    .postprocess=sbc_aec_postprocess,
    .uninit=sbc_aec_uninit,
    .methods=sbc_aec_methods
};
 
#endif
 
MS_FILTER_DESC_EXPORT(ms_sbc_aec_desc)
