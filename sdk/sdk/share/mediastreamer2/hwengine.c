/*
by powei
*/

#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/hwengine.h"

//#define EC_MEASUREMENTS

typedef struct _HWEngine{
	bool_t started;
	uint64_t diff[HW_ENGINE_COUNT];
	uint32_t counter[HW_ENGINE_COUNT];
	void *filters[HW_ENGINE_COUNT];
	ms_thread_t hw_thread;	
	ms_mutex_t mutex;
}HWEngine;

extern HWEngineDesc hw_ec_engine_desc;
//extern HWEngineDesc hw_enc_engine_desc;
//extern HWEngineDesc hw_dec_engine_desc;

static HWEngineDesc * hw_engines[]={
    &hw_ec_engine_desc,
	//&hw_enc_engine_desc,
	//&hw_dec_engine_desc,
    NULL
};

static HWEngine* e=NULL;

static void *hw_engine_thread(void *arg)
{
    int used[HW_ENGINE_COUNT];    
    while(e->started) {
		used[HW_EC_ID] = 0;
		used[HW_ENC_ID] = 0;
		used[HW_DEC_ID] = 0;
		
#ifdef EC_MEASUREMENTS	
		MSTimeSpec begin,end;
		ms_get_cur_time(&begin);
#endif			
		
		if(e->filters[HW_EC_ID])
            used[HW_EC_ID] = hw_engines[HW_EC_ID]->process(e->filters[HW_EC_ID]);

#ifdef EC_MEASUREMENTS
		ms_get_cur_time(&end);
        if(used[HW_EC_ID]) {
			e->diff[HW_EC_ID]+=(end.tv_sec-begin.tv_sec)*1000 + (end.tv_nsec-begin.tv_nsec)/1000000;
			e->counter[HW_EC_ID]++;
        }	
		if(used[HW_EC_ID] && (e->counter[HW_EC_ID]%100==0)) {
			printf("*******  EC risc time = %d ***********\n", (int)e->diff[HW_EC_ID]);
			e->diff[HW_EC_ID] = 0;
		}		
		ms_get_cur_time(&begin);
#endif	  

		if(e->filters[HW_ENC_ID])
            used[HW_ENC_ID] = hw_engines[HW_ENC_ID]->process(e->filters[HW_ENC_ID]);

#ifdef EC_MEASUREMENTS
		ms_get_cur_time(&end);
        if(used[HW_ENC_ID]) {
			e->diff[HW_ENC_ID]+=(end.tv_sec-begin.tv_sec)*1000 + (end.tv_nsec-begin.tv_nsec)/1000000;
			e->counter[HW_ENC_ID]++;
        }	
		if(used[HW_ENC_ID] && (e->counter[HW_ENC_ID]%100==0)) {
			printf("******* ENC risc time = %d ***********\n", (int)e->diff[HW_ENC_ID]);
			e->diff[HW_ENC_ID] = 0;
		}		
		ms_get_cur_time(&begin);
#endif	  
		
		if(e->filters[HW_DEC_ID])
            used[HW_DEC_ID] = hw_engines[HW_DEC_ID]->process(e->filters[HW_DEC_ID]);
		
#ifdef EC_MEASUREMENTS
		ms_get_cur_time(&end);
        if(used[HW_DEC_ID]) {
			e->diff[HW_DEC_ID]+=(end.tv_sec-begin.tv_sec)*1000 + (end.tv_nsec-begin.tv_nsec)/1000000;
			e->counter[HW_DEC_ID]++;
        }	
		if(used[HW_DEC_ID] && (e->counter[HW_DEC_ID]%100==0)) {
			printf("******* DEC risc time = %d ***********\n", (int)e->diff[HW_DEC_ID]);
			e->diff[HW_DEC_ID] = 0;
		}		    
#endif

		if (used[HW_EC_ID]==0 && used[HW_ENC_ID]==0 && used[HW_DEC_ID]==0)
			usleep(1000);
    }
}

void hw_engine_init(void){
	int i;
	if (e) return;
	e =(HWEngine *)ms_new(HWEngine,1);
	e->started = FALSE;
    ms_mutex_init(&e->mutex,NULL);
	
	for(i=0;i<HW_ENGINE_COUNT;i++) {
		e->filters[i] = NULL;
		e->diff[i] = 0;
		e->counter[i] = 1;
	}

    if (e->started==FALSE)
    {
        pthread_attr_t attr;
        struct sched_param param;
        e->started=TRUE;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 25*1024);       
        param.sched_priority = sched_get_priority_min(0) + 1;
        pthread_attr_setschedparam(&attr, &param);
        ms_thread_create(&e->hw_thread, &attr, hw_engine_thread, NULL);
    }
}

void hw_engine_uninit(void){
    if (e==NULL) return;
	e->started=FALSE;
	ms_mutex_destroy(&e->mutex);
	ms_thread_join(e->hw_thread, NULL);
	ms_free(e);
	e=NULL;
}


void hw_engine_link_filter(HWEngineId id, void* filter){
	//printf("******** link %d engine = %x\n", (int)id, (unsigned int)filter);
	e->filters[id] = filter;
}

void hw_engine_lock(void) {
    ms_mutex_lock(&e->mutex);	
}

void hw_engine_unlock(void) {
    ms_mutex_unlock(&e->mutex);	
}
