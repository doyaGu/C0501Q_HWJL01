/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2006-2013 Belledonne Communications, Grenoble

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


#ifdef HAVE_CONFIG_H
#include "mediastreamer-config.h"
#endif

#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/dtmfgen.h"
#include "mediastreamer2/msfileplayer.h"

static void _ring_codec(RingStream *stream,int codec_type,int *param);
#ifdef CFG_MP3_RING

#include "audio_mgr.h"

static void linphonec_call_mp3_ring(char* ring,int interval);
static void linphonec_stop_mp3_ring();

// return 0 : mp3 not ringing ,1: mp3 ringing
int linphonec_mp3_is_ringing();
// implement call mp3 ring
static void *AL_Local_player_http_handle = NULL;
static char *gRing_name = NULL;

#define LOCAL_PLAYER_BUFFER_LEN  (64 * 1024)

static void linphonec_call_mp3_ring(char* ring,int interval)
{
    char* ext;
    SMTK_AUDIO_PARAM_NETWORK audiomgr_local;
    int nResult = 0;

    ext = strrchr(ring, '.');
    if (!ext)
    {
        printf("Invalid file name: %s\n", ring);
        return ;
    }
    ext++;

    // check mp3 ring
    if (stricmp(ext, "mp3") == 0)
    {
        if (gRing_name)
        {
            free(gRing_name);
            gRing_name = NULL;
        }
        gRing_name = strdup(ring);
    
        // close handler (if any)
        if (AL_Local_player_http_handle)
        {
            fclose(AL_Local_player_http_handle);
            AL_Local_player_http_handle = NULL;
        }
        audiomgr_local.audioMgrCallback = NULL;

        audiomgr_local.nType = SMTK_AUDIO_MP3;
        // stop sbc
        castor3snd_deinit_for_video_memo_play();        

        if (gRing_name)
        {
            AL_Local_player_http_handle = fopen(gRing_name, "rb");
        }
        if (!AL_Local_player_http_handle)
        {
            printf("%s() L#%ld: fopen error \r\n", __FUNCTION__, __LINE__);
            return ;
        }
        //printf("[Main]%s() L#%ld:  %s success \r\n", __FUNCTION__, __LINE__, ring);
        audiomgr_local.pHandle     = AL_Local_player_http_handle;
        audiomgr_local.LocalRead   = fread;
        audiomgr_local.nReadLength = LOCAL_PLAYER_BUFFER_LEN;
        audiomgr_local.bSeek       = 0;
        audiomgr_local.nM4A        = 0;
        audiomgr_local.bLocalPlay  = 1;
        audiomgr_local.pFilename   = gRing_name;
        if(interval == -1)
            smtkAudioMgrSetMode(SMTK_AUDIO_NORMAL);
        else
            smtkAudioMgrSetMode(SMTK_AUDIO_REPEAT);    
        nResult  = smtkAudioMgrPlayNetwork(&audiomgr_local);
        
    }


}

static void linphonec_stop_mp3_ring()
{
    // close handler (if any)
    if (AL_Local_player_http_handle)
    {
    // check mp3 ring
#ifdef __OPENRTOS__
        smtkAudioMgrQuickStop();
#endif
        fclose(AL_Local_player_http_handle);
        AL_Local_player_http_handle = NULL;
        
        castor3snd_reinit_for_video_memo_play();
    }
    
}

// return 0 : mp3 not ringing ,1: mp3 ringing
int linphonec_mp3_is_ringing()
{
    if (AL_Local_player_http_handle){
        return 1;
    } else {
        return 0;
    }
}


#endif

static void _ring_codec_type(RingStream *stream,int codec_type,int *param){
    
    switch (codec_type){
            case ULAW:
                *param = 0;
                stream->decoder=ms_filter_new(MS_ULAW_DEC_ID);
                break;
            
            case ALAW:
                *param = 0;
                stream->decoder=ms_filter_new(MS_ALAW_DEC_ID);
                break;
            
            case SPEEX:
                *param = 1;
                stream->decoder=ms_filter_create_decoder("speex");
                break;
                
            case OFFSET8BIT:
                *param = 0;
                stream->decoder=ms_filter_new(MS_OFFSET8BIT_DEC_ID);
                break;

             default:
                *param = 0;
                break;
    }

}


RingStream * ring_start(const char *file, int interval, MSSndCard *sndcard){   

   return ring_start_with_cb(file,interval,sndcard,NULL,NULL);
}

RingStream * ring_start_with_cb(const char *file,int interval,MSSndCard *sndcard, MSFilterNotifyFunc func,void * user_data)
{
    RingStream *stream;
    int tmp;
    int srcrate,dstrate;
    MSConnectionHelper h;
    int special_case; // for better sound, smooth
    int codec_type;

    stream=(RingStream *)ms_new0(RingStream,1);
    
#ifdef CFG_MP3_RING 
    linphonec_call_mp3_ring(file,interval);
    if(AL_Local_player_http_handle)
        return stream;// mp3 play :need not filter connect 
#endif 

    stream->source=ms_filter_new(MS_FILE_PLAYER_ID);    
    //stream->gendtmf=ms_filter_new(MS_MIXVOICE_ID);
    stream->mixer = ms_filter_new(MS_MIXVOICE_ID);
    stream->sndwrite=ms_snd_card_create_writer(sndcard);

    if (file){
        ms_filter_call_method(stream->source,MS_FILE_PLAYER_OPEN,(void*)file);
        ms_filter_call_method(stream->source,MS_FILTER_GET_CODEC_TYPE,&codec_type);
        _ring_codec_type(stream,codec_type,&special_case);
        ms_filter_call_method(stream->source,MS_FILE_PLAYER_SET_SPECIAL_CASE,&special_case);
        ms_filter_call_method(stream->source,MS_FILE_PLAYER_LOOP,&interval);
        ms_filter_call_method_noarg(stream->source,MS_FILE_PLAYER_START);
    }

    /*configure sound outputfilter*/
    ms_filter_call_method(stream->source,MS_FILTER_GET_SAMPLE_RATE,&srcrate);
    if (stream->gendtmf)
        ms_filter_call_method(stream->gendtmf,MS_FILTER_SET_SAMPLE_RATE,&srcrate);
    ms_filter_call_method(stream->sndwrite,MS_FILTER_SET_SAMPLE_RATE,&srcrate);
    ms_filter_call_method(stream->sndwrite,MS_FILTER_GET_SAMPLE_RATE,&dstrate);
    if (srcrate!=dstrate){
        stream->write_resampler=ms_filter_new(MS_RESAMPLE_ID);
        ms_filter_call_method(stream->write_resampler,MS_FILTER_SET_SAMPLE_RATE,&srcrate);
        ms_filter_call_method(stream->write_resampler,MS_FILTER_SET_OUTPUT_SAMPLE_RATE,&dstrate);
        ms_message("configuring resampler from rate[%i] to rate [%i]", srcrate,dstrate);
    }
    ms_filter_call_method(stream->source,MS_FILTER_GET_NCHANNELS,&tmp);
    if (stream->gendtmf)
        ms_filter_call_method(stream->gendtmf,MS_FILTER_SET_NCHANNELS,&tmp);
    ms_filter_call_method(stream->sndwrite,MS_FILTER_SET_NCHANNELS,&tmp);
    
    if (interval == -1){//set callback func to detect stop
        ms_filter_call_method(stream->source,MS_FILTER_GET_DATA_LENGTH,&tmp);
        ms_filter_call_method(stream->sndwrite,MS_FILTER_SET_DATALENGTH,&tmp);
    }
    
    if(func!=NULL)
        ms_filter_set_notify_callback(stream->sndwrite,func,user_data);

    
    stream->ticker=ms_ticker_new();

    ms_ticker_set_name(stream->ticker,"Audio (ring) MSTicker");

    ms_connection_helper_start(&h);
    ms_connection_helper_link(&h,stream->source,-1,0);
    if(stream->decoder)
        ms_connection_helper_link(&h,stream->decoder,0,0);    
    if (stream->gendtmf)
        ms_connection_helper_link(&h,stream->gendtmf,0,0);
    if (stream->write_resampler)
        ms_connection_helper_link(&h,stream->write_resampler,0,0);
    if (stream->mixer)
        ms_connection_helper_link(&h,stream->mixer,0,0);
    ms_connection_helper_link(&h,stream->sndwrite,0,-1);
    ms_ticker_attach(stream->ticker,stream->source);

    return stream;
}

int get_wav_time(RingStream *stream){
    int wavetime=0;
    if(stream->source)
        ms_filter_call_method(stream->source,MS_FILTER_GET_PLAY_TIME,&wavetime);
    return wavetime;
}

void ring_pause_play(RingStream *stream,int pause){
    if(stream->source){
        if(pause)
            ms_filter_call_method_noarg(stream->source,MS_PLAYER_PAUSE);
        else
            ms_filter_call_method_noarg(stream->source,MS_PLAYER_START);
    }
}

void ring_play_dtmf(RingStream *stream, char dtmf, int duration_ms){
    if (duration_ms>0)
        ms_filter_call_method(stream->gendtmf, MS_DTMF_GEN_PLAY, &dtmf);
    else ms_filter_call_method(stream->gendtmf, MS_DTMF_GEN_START, &dtmf);
}

void ring_stop_dtmf(RingStream *stream){
    ms_filter_call_method_noarg(stream->gendtmf, MS_DTMF_GEN_STOP);
}

void ring_stop(RingStream *stream){
    MSConnectionHelper h;

#ifdef CFG_MP3_RING
    if(AL_Local_player_http_handle){
        linphonec_stop_mp3_ring();
        ms_free(stream);        
        return;
    }
#endif    

    ms_ticker_detach(stream->ticker,stream->source);

    ms_connection_helper_start(&h);
    ms_connection_helper_unlink(&h,stream->source,-1,0);
    if(stream->decoder)
        ms_connection_helper_unlink(&h,stream->decoder,0,0);        
    if (stream->gendtmf)
        ms_connection_helper_unlink(&h,stream->gendtmf,0,0);
    if (stream->write_resampler)
        ms_connection_helper_unlink(&h,stream->write_resampler,0,0);
    if (stream->mixer)
        ms_connection_helper_unlink(&h,stream->mixer,0,0);
    ms_connection_helper_unlink(&h,stream->sndwrite,0,-1);

    ms_ticker_destroy(stream->ticker);
    ms_filter_destroy(stream->source);
    if (stream->gendtmf)
        ms_filter_destroy(stream->gendtmf);
    if (stream->write_resampler)
        ms_filter_destroy(stream->write_resampler);
    if (stream->mixer)
        ms_filter_destroy(stream->mixer);
    ms_filter_destroy(stream->sndwrite);
    ms_free(stream);
#ifdef _WIN32_WCE
    ms_warning("Sleeping a bit after closing the audio device...");
    ms_sleep(1);
#endif

}

void ring_mix_pcm(RingStream* stream,char *file)
{
    printf("mix file : %s\n",file);
    if(stream->mixer)
        ms_filter_call_method(stream->mixer,MS_WAV_FILE_OPEN,(void*)file); 
}
