/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2006  Simon MORLAT (simon.morlat@linphone.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#include "mediastreamer2/msfilter.h"
#include "adpcm.h"

typedef struct _ADPCMEncData{
    MSBufferizer *bz;
    int ptime;
    uint32_t ts;
} ADPCMEncData;

typedef struct _ADPCMDecData{
    MSBufferizer *bz_t;
} ADPCMDecData;

static ADPCMDecData * adpcm_dec_data_new(){
    ADPCMDecData *obj=(ADPCMDecData *)ms_new(ADPCMDecData,1);
    obj->bz_t=ms_bufferizer_new();
    return obj;
}

static void adpcm_dec_data_destroy(ADPCMDecData *obj){
    ms_bufferizer_destroy(obj->bz_t);
    ms_free(obj);
}

static void adpcm_dec_init(MSFilter *obj){
    obj->data=adpcm_dec_data_new();
    adpcm_decode_init();
}

static void adpcm_dec_uninit(MSFilter *obj){
    adpcm_dec_data_destroy((ADPCMDecData*)obj->data);
}

static ADPCMEncData * adpcm_enc_data_new(){
    ADPCMEncData *obj=(ADPCMEncData *)ms_new(ADPCMEncData,1);
    obj->bz=ms_bufferizer_new();
    obj->ptime=0;
    obj->ts=0;
    return obj;
}

static void adpcm_enc_data_destroy(ADPCMEncData *obj){
    ms_bufferizer_destroy(obj->bz);
    ms_free(obj);
}

static void adpcm_enc_init(MSFilter *obj){
    obj->data=adpcm_enc_data_new();
    adpcm_encode_init();
}

static void adpcm_enc_uninit(MSFilter *obj){
    adpcm_enc_data_destroy((ADPCMEncData*)obj->data);
}

static void adpcm_enc_process(MSFilter *obj){
    ADPCMEncData *dt=(ADPCMEncData*)obj->data;
    MSBufferizer *bz=dt->bz;
    uint8_t buffer[2240];
    int frame_per_packet=2;
    int size_of_pcm=320;

    mblk_t *m;

    if (dt->ptime>=10)
    {
        frame_per_packet = dt->ptime/10;
    }

    if (frame_per_packet<=0)
        frame_per_packet=1;
    if (frame_per_packet>14) /* 7*20 == 140 ms max */
        frame_per_packet=14;
#if (CFG_AUDIO_SAMPLING_RATE == 16000)
    size_of_pcm = 256*frame_per_packet;
#else
    size_of_pcm = 128*frame_per_packet; /* ex: for 20ms -> 160*2==320 */
#endif
    while((m=ms_queue_get(obj->inputs[0]))!=NULL){
        ms_bufferizer_put(bz,m);
    }

    while (ms_bufferizer_read(bz,buffer,size_of_pcm)==size_of_pcm){
        mblk_t *o=allocb(size_of_pcm/4,0);
        int len;
        len = adpcm_encoder(buffer,o->b_wptr,size_of_pcm);
        o->b_wptr+=len;
        
        mblk_set_timestamp_info(o,dt->ts);
        dt->ts+=len;
        ms_queue_put(obj->outputs[0],o);
    }
}

static int enc_add_fmtp(MSFilter *f, void *arg){
    const char *fmtp=(const char *)arg;
    ADPCMEncData *s=(ADPCMEncData*)f->data;
    char val[30];
    if (fmtp_get_value(fmtp,"ptime",val,sizeof(val))){
        s->ptime=atoi(val);
        ms_message("MSADPCMEnc: got ptime=%i",s->ptime);
    }
    return 0;
}


static int enc_add_attr(MSFilter *f, void *arg){
    const char *fmtp=(const char *)arg;
    ADPCMEncData *s=(ADPCMEncData*)f->data;
    if (strstr(fmtp,"ptime:10")!=NULL){
        s->ptime=10;
    }else if (strstr(fmtp,"ptime:20")!=NULL){
        s->ptime=20;
    }else if (strstr(fmtp,"ptime:30")!=NULL){
        s->ptime=30;
    }else if (strstr(fmtp,"ptime:40")!=NULL){
        s->ptime=40;
    }else if (strstr(fmtp,"ptime:50")!=NULL){
        s->ptime=50;
    }else if (strstr(fmtp,"ptime:60")!=NULL){
        s->ptime=60;
    }else if (strstr(fmtp,"ptime:70")!=NULL){
        s->ptime=70;
    }else if (strstr(fmtp,"ptime:80")!=NULL){
        s->ptime=80;
    }else if (strstr(fmtp,"ptime:90")!=NULL){
        s->ptime=90;
    }else if (strstr(fmtp,"ptime:100")!=NULL){
        s->ptime=100;
    }else if (strstr(fmtp,"ptime:110")!=NULL){
        s->ptime=110;
    }else if (strstr(fmtp,"ptime:120")!=NULL){
        s->ptime=120;
    }else if (strstr(fmtp,"ptime:130")!=NULL){
        s->ptime=130;
    }else if (strstr(fmtp,"ptime:140")!=NULL){
        s->ptime=140;
    }
    return 0;
}

static MSFilterMethod enc_methods[]={
    {   MS_FILTER_ADD_ATTR      ,   enc_add_attr},
    {   MS_FILTER_ADD_FMTP      ,   enc_add_fmtp},
    {   0               ,   NULL        }
};

#ifdef _MSC_VER

MSFilterDesc ms_adpcm_enc_desc={
    MS_ADPCM_ENC_ID,
    "MSADPCMEnc",
    N_("adpcm encoder"),
    MS_FILTER_ENCODER,
    "adpcm",
    1,
    1,
    adpcm_enc_init,
    NULL,
    adpcm_enc_process,
    NULL,
    adpcm_enc_uninit,
    enc_methods
};

#else

MSFilterDesc ms_adpcm_enc_desc={
    .id=MS_ADPCM_ENC_ID,
    .name="MSADPCMEnc",
    .text=N_("adpcm encoder"),
    .category=MS_FILTER_ENCODER,
    .enc_fmt="adpcm",
    .ninputs=1,
    .noutputs=1,
    .init=adpcm_enc_init,
    .process=adpcm_enc_process,
    .uninit=adpcm_enc_uninit,
    .methods=enc_methods
};

#endif

static void adpcm_dec_process(MSFilter *obj){
    ADPCMDecData *dt=(ADPCMDecData*)obj->data;
    MSBufferizer *bz_t=dt->bz_t;
    uint8_t buffer[128];
    int size_of_pcm=80;
    mblk_t *m;
    while((m=ms_queue_get(obj->inputs[0]))!=NULL){
        ms_bufferizer_put(bz_t,m);
    }    

    while(ms_bufferizer_read(bz_t,buffer,size_of_pcm)==size_of_pcm){
        mblk_t *o=allocb(size_of_pcm*4,0);
        int len;
        len = adpcm_decoder(buffer,o->b_wptr,size_of_pcm);
        o->b_wptr += len;
        ms_queue_put(obj->outputs[0],o);        
    }
}

#ifdef _MSC_VER

MSFilterDesc ms_adpcm_dec_desc={
    MS_ADPCM_DEC_ID,
    "MSADPCMDec",
    N_("adpcm decoder"),
    MS_FILTER_DECODER,
    "adpcm",
    1,
    1,
    adpcm_dec_init,
    NULL,
    adpcm_dec_process,
    NULL,
    adpcm_dec_uninit,
    NULL
};

#else

MSFilterDesc ms_adpcm_dec_desc={
    .id=MS_ADPCM_DEC_ID,
    .name="MSADPCMDec",
    .text=N_("adpcm decoder"),
    .category=MS_FILTER_DECODER,
    .enc_fmt="adpcm",
    .ninputs=1,
    .noutputs=1,
    .init=adpcm_dec_init,
    .process=adpcm_dec_process,
    .uninit=adpcm_dec_uninit,
};

#endif

MS_FILTER_DESC_EXPORT(ms_adpcm_dec_desc)
MS_FILTER_DESC_EXPORT(ms_adpcm_enc_desc)

