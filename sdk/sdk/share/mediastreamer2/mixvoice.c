
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/dtmfgen.h"

#define max_file 10

typedef struct _Mixdata{
    unsigned char* pcm_buf;
    unsigned int mix_size;
    unsigned int end;
    bool_t flag;
}Mixdata;

static inline uint32_t swap32(uint32_t a)
{
	return ((a & 0xFF) << 24) | ((a & 0xFF00) << 8) | 
		((a & 0xFF0000) >> 8) | ((a & 0xFF000000) >> 24);
}

static int wav_file_open(MSFilter *f, void *arg){
    Mixdata *d=(Mixdata*)f->data;
    char *filepath=(const char *)arg;  
    FILE *fp[max_file];
    char *file_name;
    unsigned int wpidx=0;    
    int data_size[max_file];    
    int file_num=0;
    int i,j;
    
    if(d->flag) return 0;

    d->mix_size = 0;
    file_name = strtok(filepath, " ");
    while (file_name != NULL){
        char* buf;
        fp[file_num] = fopen(file_name,"rb");
        buf = malloc(50);
        fread(buf, 1, 44, fp[file_num]);
        data_size[file_num] = *((int*)&buf[40]);
#if (CFG_CHIP_FAMILY == 9910)
        data_size[file_num] = swap32(data_size[file_num]);
    //    data_size[file_num] = (((data_size[file_num]) & 0x000000FF) << 24) | \
    //                          (((data_size[file_num]) & 0x0000FF00) <<  8) | \
    //                          (((data_size[file_num]) & 0x00FF0000) >>  8) | \
    //                          (((data_size[file_num]) & 0xFF000000) >> 24);    
#endif
        d->mix_size += data_size[file_num];
        file_name = strtok(NULL, " ");
        free(buf);
        file_num++;
    }
    
    d->pcm_buf = (unsigned char*)malloc(d->mix_size);
    for(j=0;j<file_num;j++){
        unsigned char* file_pcm;
        file_pcm = (unsigned char*)malloc(data_size[j]);
        fread(file_pcm, 1, data_size[j], fp[j]);
        memmove(d->pcm_buf+wpidx,file_pcm,data_size[j]);
        wpidx += data_size[j];
        free(file_pcm);
        fclose(fp[j]);
    }
  
    if(!d->pcm_buf){
#if (CFG_CHIP_FAMILY == 9910)    
        for (i=0;i<d->mix_size;i+=2)
        {
            d->pcm_buf[i]  = d->pcm_buf[i]^d->pcm_buf[i+1];
            d->pcm_buf[i+1]= d->pcm_buf[i]^d->pcm_buf[i+1];
            d->pcm_buf[i]  = d->pcm_buf[i]^d->pcm_buf[i+1];
        }//big endian
#endif
    }
    
    d->flag = TRUE;
    return 0;
} 

static void mix_voice_init(MSFilter *f){
    Mixdata *d=(Mixdata*)ms_new(Mixdata,1);
    d->flag =FALSE;
    d->end = 0;
    d->mix_size = 0;
    d->pcm_buf = NULL;
    f->data=d;
}

static void mix_voice_process(MSFilter *f){
    Mixdata *d=(Mixdata*)f->data;
    mblk_t *m;
    if(d->flag){
        while((m=ms_queue_get(f->inputs[0]))!=NULL){
            mblk_t *o;
            //msgpullup(m,-1);
            o=allocb(m->b_wptr-m->b_rptr,0);
            //mblk_meta_copy(m, o);
            if(d->flag){
                for(;m->b_rptr<m->b_wptr;m->b_rptr+=2,o->b_wptr+=2,d->end+=2){
                    if(d->end < d->mix_size){//mixsound
                    *((int16_t*)(o->b_wptr))=(*((int16_t*)(d->pcm_buf+d->end)))*(0.7)+(0.2)*((int)*(int16_t*)m->b_rptr);
                    }else{//mixsound over
                        //for(;m->b_rptr<m->b_wptr;m->b_rptr+=2,o->b_wptr+=2){
                            *((int16_t*)(o->b_wptr))=(int)*(int16_t*)m->b_rptr;
                        //}
                    }
                }
            }else{//no mixsound
                for(;m->b_rptr<m->b_wptr;m->b_rptr+=2,o->b_wptr+=2){
                    *((int16_t*)(o->b_wptr))=(int)*(int16_t*)m->b_rptr;
                }              
            }
            if(d->end > d->mix_size){
                d->end=0;
                d->flag=FALSE;
                free(d->pcm_buf);
                d->pcm_buf=NULL;
            };
            freemsg(m);
            ms_queue_put(f->outputs[0],o);
        }
    }else{
        while((m=ms_queue_get(f->inputs[0]))!=NULL){
        ms_queue_put(f->outputs[0],m);
         }
    }
    
}

static void mix_voice_postprocess(MSFilter *f){
    Mixdata *d=(Mixdata*)f->data;
    d->flag =FALSE;
    d->end = 0;
    d->mix_size = 0;
    free(d->pcm_buf);
    d->pcm_buf = NULL;
}

static void mix_voice_uninit(MSFilter *f){
    Mixdata *d=(Mixdata*)f->data;
    ms_free(d);
}

 static MSFilterMethod mixvoice_methods[]={
    {   MS_WAV_FILE_OPEN        ,   wav_file_open},
    {   0               ,   NULL        }
}; 

#ifdef _MSC_VER

MSFilterDesc ms_mix_voice_desc={
    MS_MIXVOICE_ID,
    "MSMixVoice",
    N_("Mix the wave"),
    MS_FILTER_OTHER,
    NULL,
    1,
    1,
    mix_voice_init,
    NULL,
    mix_voice_process,
    mix_voice_postprocess,
    mix_voice_uninit,
    mixvoice_methods
};

#else

MSFilterDesc ms_mix_voice_desc={
    .id=MS_MIXVOICE_ID,
    .name="MSMixVoice",
    .text=N_("Mix the wave"),
    .category=MS_FILTER_OTHER,
    .ninputs=1,
    .noutputs=1,
    .init = mix_voice_init,
    .process = mix_voice_process,
    .uninit = mix_voice_uninit,
    .postprocess=mix_voice_postprocess,
    .methods=mixvoice_methods
};

#endif

MS_FILTER_DESC_EXPORT(ms_mix_voice_desc)