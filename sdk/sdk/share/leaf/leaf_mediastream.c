/**************************************
Linphone Excluded API and Function
***************************************/
#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/msjpegwriter.h"
#include "mediastreamer2/msfilewriter.h" 
#include "iniparser/iniparser.h"

#include "leaf_mediastream.h"
#include "castor3player.h"

//#define PATH_MAX 256

static bool VideomemoPlayFinished;
static bool VideomemoRecording = false;
static bool AudioSoundPlay = false;
static bool AudiomemoRecording = false;
static bool Ringstream_release = false;
static bool bRtspStreamError = false;
static bool bRtspStreamConnectFail = false;
static bool bRtspStreamGetMediaInfo = false;
static RTSP_MEDIA_INFO *pRtsp_media_info;

static pthread_mutex_t Leaf_mutex = PTHREAD_MUTEX_INITIALIZER;
static MSSndCard *get_sndcard(unsigned int cap);
static SoundPlayCallback soundPlayCallback=NULL;

static void video_memo_playback_event_handler(PLAYER_EVENT nEventID, void *arg)
{
    switch(nEventID)
    {
        case PLAYER_EVENT_EOF:
            VideomemoPlayFinished = true;
            break;
        case PLAYER_EVENT_OPEN_FILE_FAIL:
            VideomemoPlayFinished = true;
            break;
        case PLAYER_EVENT_UNSUPPORT_FILE:
            VideomemoPlayFinished = true;
            break;
        default:
            break;
    }
}

static void RtspHandler_callbak(RTSPCLIENT_EVENT event_id, void *arg)
{
    switch (event_id)
    {
    case STREAM_ERROR:
        bRtspStreamError = true;
        break;
    case STREAM_CONNECT_FAIL:
        bRtspStreamConnectFail = true;
        break;
    case STREAM_GET_INFO:
        pRtsp_media_info = (RTSP_MEDIA_INFO *)arg;
        bRtspStreamGetMediaInfo = true;
        break;
    default:
        break;
    }
}

static void leaf_sound_play_callbackfunc(int state)
{
    if (soundPlayCallback)
    {
        return soundPlayCallback(state);
    }
}

void Leaf_media_state_callback(void *userdata, struct _MSFilter *f, unsigned int id, void *arg)
{
	switch (f->desc->id)
	{
		case MS_CASTOR3SND_WRITE_ID:
            leaf_sound_play_callbackfunc(SOUND_FINISH_PLAY);
            Ringstream_release = true ;
			break;

		default:
			break;
	}		
}

LeafCall* leaf_init(void) {
    
	LeafCall *call=ms_new0(LeafCall,1);
	ms_init();
     
    call->ring_sndcard = get_sndcard(MS_SND_CARD_CAP_PLAYBACK);
  	call->play_sndcard = get_sndcard(MS_SND_CARD_CAP_PLAYBACK);
	call->capt_sndcard = get_sndcard(MS_SND_CARD_CAP_CAPTURE);
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        ms_thread_create(&call->Thread, &attr, leaf_background_iterate, call);    
    }
    
    call->cfgini = iniparser_load(CFG_PRIVATE_DRIVE ":/Esound.ini");
    
	return call;
}

static void *leaf_background_iterate(LeafCall *call){
    while(1){
        _check_auto_stop_sound(call);
        // add more background check ...
        usleep(500000);
    }
}

void leaf_init_video_streams (LeafCall *call, unsigned short port) {
	call->videostream=video_stream_udp_new(port,FALSE,FALSE);
}

void leaf_init_audio_streams (LeafCall *call, unsigned short port) {
	AudioStream *audiostream;
	
	call->audiostream=audiostream=audio_stream_udp_new(port,FALSE);
      
#ifdef ENABLE_AUDIO_NOISE_GATE
    audiostream->use_ng=iniparser_getint(call->cfgini , "Esound:noisegate", 1);
#endif
#ifdef ENABLE_AUDIO_SPK_EQUALIZER
    audiostream->eq_SPK=iniparser_getint(call->cfgini , "Esound:eq_SPK", 1);//filter
#endif
#ifdef ENABLE_AUDIO_MIC_EQUALIZER
    audiostream->eq_MIC=iniparser_getint(call->cfgini , "Esound:eq_MIC", 1);//filter
#endif
#ifdef ENABLE_AEC_ENABLE
    audiostream->use_ec=iniparser_getint(call->cfgini , "Esound:echocancellation", 1);//filter
#endif
    audiostream->use_volsend=iniparser_getint(call->cfgini , "Esound:use_volsend", 1);//filter
    audiostream->use_volrecv=iniparser_getint(call->cfgini , "Esound:use_volrecv", 1);;//filter
    audiostream->use_mix=iniparser_getint(call->cfgini , "Esound:use_mix", 0);//filter;
    audiostream->play_dtmfs=FALSE;
    audiostream->use_gc=FALSE;
    audiostream->use_agc=FALSE;
//    if(audiostream->use_ec)
//        audiostream->ec=ms_filter_new(MS_SBC_AEC_ID);//creat    
}

void leaf_start_video_stream(LeafCall *call, const char *addr, int port) {
	VideoStreamDir dir=VideoStreamSendRecv;

	if(call->dir == CallOutgoing)
		dir = VideoStreamSendOnly;
	else
		dir = VideoStreamRecvOnly;

	video_stream_set_direction (call->videostream, dir);
	video_stream_udp_start(call->videostream,                    	
		addr, port,                    	
		FALSE, FALSE, NULL); 
}

void leaf_start_audio_stream(LeafCall *call, const char *addr, int port) {
	/*to be defined*/

	MSSndCard *playcard=call->play_sndcard;
	MSSndCard *captcard=call->capt_sndcard;
	bool_t use_ec;
    use_ec = call->audiostream->use_ec;
    if (playcard) ms_snd_card_set_level(playcard,MS_SND_CARD_PLAYBACK,call->sound_conf.play_level);
    if (captcard) ms_snd_card_set_level(captcard,MS_SND_CARD_CAPTURE,call->sound_conf.rec_level);    
	audio_stream_udp_start_full(
		call->audiostream,	                                        
		addr,	                    
		port,	                                        
		playcard,	                    
		captcard,	                    
		use_ec,
		AudioFromSoundRead
    );
    audio_stream_post_configure(call->audiostream,call->audio_muted,call->cfgini);        
}

void leaf_stop_media_streams(LeafCall *call) {
    while(VideomemoRecording) usleep(1000);
    pthread_mutex_lock(&Leaf_mutex);
	if (call->audiostream!=NULL) {
		audio_stream_udp_stop(call->audiostream, AudioFromSoundRead);
		call->audiostream=NULL;
	}
	if (call->videostream!=NULL){
		video_stream_udp_stop(call->videostream);
		call->videostream=NULL;
	}
    pthread_mutex_unlock(&Leaf_mutex);
}

int leaf_check_ipcam_stream_status()
{
    if(bRtspStreamError)
    {
        bRtspStreamError = false;
        return RtspStreamParsingError;
    }
    else if(bRtspStreamConnectFail)
    {
        bRtspStreamConnectFail = false;
        return RtspStreamConnectFail;
    }

    return RtspStreamNone;
}

bool leaf_get_ipcam_stream_media_info(RTSP_MEDIA_INFO *info)
{
    if(bRtspStreamGetMediaInfo)
    {
        info->width = pRtsp_media_info->width;
        info->height = pRtsp_media_info->height;
        info->max_framerate = pRtsp_media_info->max_framerate;
        bRtspStreamGetMediaInfo = false;
        return true;
    }

    return false;
}

void leaf_start_ipcam_stream(LeafCall *call, const char *addr, int port) {
    video_stream_set_direction (call->videostream, VideoStreamRecvOnly);
    video_stream_udp_start(call->videostream,                    	
		addr, port,                    	
		FALSE, TRUE, NULL);
#ifdef CFG_RTSP_CLIENT_ENABLE
    SetRTSPClientMode(IPCAM_MODE);
    startRTSPClient(addr, port, NULL, RtspHandler_callbak);
#endif    
} 

void leaf_stop_ipcam_stream(LeafCall *call) {
    while(VideomemoRecording) usleep(1000);
    pthread_mutex_lock(&Leaf_mutex);
#ifdef CFG_RTSP_CLIENT_ENABLE
    stopRTSPClient();
#endif    
    if (call->videostream!=NULL){
		video_stream_udp_stop(call->videostream);
		call->videostream=NULL;
	}
    pthread_mutex_unlock(&Leaf_mutex);
}

int leaf_take_video_snapshot(LeafCall *call, char *file,FileWriterCallback func ){
	if ( file ) file=lpc_strip_blanks(file);
	if ( ! file || ! *file ) return 0;

	FileWriter_callback = func;   
    if (call->videostream!=NULL && call->videostream->jpegwriter!=NULL){
		ms_filter_call_method(call->videostream->jpegwriter,MS_JPEG_WRITER_TAKE_SNAPSHOT,(void*)file);
		return 1;
    }else
    {
		printf("Cannot take snapshot: no currently running video stream on this call.\n");
		return 0;
    }
}

#ifdef CFG_BUILD_ITU
int leaf_show_snapshot(LeafCall *call, int width,int height,char *file){
	if ( file ) file=lpc_strip_blanks(file);
	if ( ! file || ! *file ) return 0;
	
	#if defined(CFG_BUILD_ITV) && !defined(CFG_TEST_VIDEO) 
		itv_set_pb_mode(1);
	#endif	
	
	#ifndef _WIN32
	return ituJpegLoadFileEx(width,height,file);
	#else
	return 0;
	#endif
}
#endif

bool leaf_video_memo_is_recording(LeafCall *call)
{
    return VideomemoRecording;
}

static void *video_memo_start_record_task(void *arg)
{
    LeafCall *call = (LeafCall *) arg;
    if(!VideomemoRecording)
    {
        pthread_mutex_lock(&Leaf_mutex);
        if(call->audiostream)
        {
            video_stream_udp_link_audio(call->videostream, call->audiostream);
            video_stream_set_recorder_audio_codec(call->videostream);
        }
        video_stream_start_record_mkv(call->videostream, call->videomemo_file);
        VideomemoRecording = true;
        pthread_mutex_unlock(&Leaf_mutex);
    }
    pthread_exit(NULL);
}

static void *video_memo_stop_record_task(void *arg)
{
    LeafCall *call = (LeafCall *) arg;
    if(VideomemoRecording)
    {
        pthread_mutex_lock(&Leaf_mutex);
        video_stream_stop_record_mkv(call->videostream);
        video_stream_udp_unlink_audio(call->videostream, call->audiostream);
        VideomemoRecording = false;
        pthread_mutex_unlock(&Leaf_mutex);
    }
    pthread_exit(NULL);
}

int leaf_start_video_memo_record(LeafCall *call, char *file) {
	if ( file ) file=lpc_strip_blanks(file);
    if ( ! file || ! *file ) return 0;
    if (call && call->videostream)
    {
        int rc;
        pthread_t tid;
        if(!VideomemoRecording)
        {
            strcpy(call->videomemo_file, file);
            rc = pthread_create(&tid, NULL, video_memo_start_record_task, (void *)call);
            pthread_detach(tid);
        }      
    }
    return 1;
}

void leaf_stop_video_memo_record(LeafCall *call) {
	if (call && call->videostream)
    {
        int rc;
        pthread_t tid;
        if(VideomemoRecording)
        {  
            rc = pthread_create(&tid, NULL, video_memo_stop_record_task, (void *)call);
            pthread_detach(tid);
        }           
    }
}

bool leaf_video_memo_is_play_finished(void) {
    return VideomemoPlayFinished;
}

int leaf_start_video_memo_playback(LeafCall *call, char *file) {
	MSSndCard *sndcard = call->ring_sndcard;
	int       level = call->sound_conf.ring_level;
	MTAL_SPEC mtal_spec = {0};
	if ( file ) file=lpc_strip_blanks(file);
    if ( ! file || ! *file ) return 0;

    VideomemoPlayFinished = false;
	/*how to get sndcard???*/
    if (sndcard) ms_snd_card_set_level(sndcard,MS_SND_CARD_PLAYBACK,level);
	
	castor3snd_deinit_for_video_memo_play();
#ifdef CFG_VIDEO_ENABLE
    mtal_pb_init(video_memo_playback_event_handler);
    strcpy(mtal_spec.srcname, file);
    mtal_spec.vol_level = level;
    mtal_pb_select_file(&mtal_spec);
    mtal_pb_play();
#endif      
    return 1;
}

void leaf_stop_video_memo_playback(LeafCall *call) {
#ifdef CFG_VIDEO_ENABLE
    mtal_pb_stop();
    mtal_pb_exit();
#endif        
    castor3snd_reinit_for_video_memo_play();
    VideomemoPlayFinished = false;
}

void leaf_pause_video_memo_playback() {
#ifdef CFG_VIDEO_ENABLE
    mtal_pb_pause();
#endif

}
int leaf_get_video_memo_current_time() {
    int ret = 0;
    int currenttime = 0;

#ifdef CFG_VIDEO_ENABLE    
    ret = mtal_pb_get_current_time(&currenttime);
#endif

    if(ret < 0)
        return -1;
    else
        return currenttime;
}

int leaf_get_video_memo_total_time() {
    int ret = 0;
    int totaltime = 0;

#ifdef CFG_VIDEO_ENABLE    
    ret = mtal_pb_get_total_duration(&totaltime);
#endif

    if(ret < 0)
        return -1;
    else
        return totaltime;
}

static void *audio_memo_start_record_task(void *arg)
{
    LeafCall *call = (LeafCall *) arg;
    if(!AudiomemoRecording)
    {
        pthread_mutex_lock(&Leaf_mutex);
        if(call->audiostream)
        {
            audio_stream_udp_link_audio_record(call->audiostream);
        }
        audio_stream_start_record_wav(call->audiostream, call->audiomemo_file);
        AudiomemoRecording = true;
        pthread_mutex_unlock(&Leaf_mutex);
    }
    pthread_exit(NULL);
}

static void *audio_memo_stop_record_task(void *arg)
{
    LeafCall *call = (LeafCall *) arg;
    if(AudiomemoRecording)
    {
        pthread_mutex_lock(&Leaf_mutex);
        audio_stream_stop_record_wav(call->audiostream);
        audio_stream_udp_unlink_audio_record(call->audiostream);
        AudiomemoRecording = false;
        pthread_mutex_unlock(&Leaf_mutex);
    }
    pthread_exit(NULL);
}

int leaf_start_audio_memo_record(LeafCall *call, char *file) {
	if ( file ) file=lpc_strip_blanks(file);
    if ( ! file || ! *file ) return 0;
    if (call && call->audiostream)
    {
        int rc;
        pthread_t tid;
        if(!AudiomemoRecording)
        {
            strcpy(call->audiomemo_file, file);
            rc = pthread_create(&tid, NULL, audio_memo_start_record_task, (void *)call);
            pthread_detach(tid);
        }      
    }
    return 1;
}

void leaf_stop_audio_memo_record(LeafCall *call) {
	if (call && call->audiostream)
    {
        int rc;
        pthread_t tid;
        if(AudiomemoRecording)
        {  
            rc = pthread_create(&tid, NULL, audio_memo_stop_record_task, (void *)call);
            pthread_detach(tid);
        }           
    }
}

bool leaf_audio_memo_is_recording(void) {
    return AudiomemoRecording;
}

void leaf_start_voice_memo_record(LeafCall *call, char *file) {  
    MSSndCard *sndcard=call->capt_sndcard;
    
    if (!file) return ;
    if (call->voice_memo_record_stream!=NULL){
        voice_memo_stop_record(call->voice_memo_record_stream);
        call->voice_memo_record_stream=NULL;
    }
    if (sndcard) {
        ms_snd_card_set_level(sndcard,MS_SND_CARD_CAPTURE,call->sound_conf.rec_level);
        call->voice_memo_record_stream=voice_memo_start_record(file,sndcard);
    }
    return ;
}

void leaf_stop_voice_memo_record(LeafCall *call) {
    if (call->voice_memo_record_stream!=NULL){
        voice_memo_stop_record(call->voice_memo_record_stream);
        call->voice_memo_record_stream=NULL;
    }
}
/*  
note:same as leaf_start_sound_play() API remove it
void leaf_start_voice_memo_playback(LeafCall *call, char *file) {    
    MSSndCard *sndcard=call->ring_sndcard;
    if (!file) return ;
    if (call->ringstream!=NULL){
        ring_stop(call->ringstream);
        call->ringstream=NULL;
        call->media_start_time=0;
    }
    if (sndcard) ms_snd_card_set_level(sndcard,MS_SND_CARD_PLAYBACK,call->sound_conf.ring_level);
    if (call->voice_memo_play_stream!=NULL){
        voice_memo_stop_play(call->voice_memo_play_stream);
        call->voice_memo_play_stream=NULL;
    }
    if (sndcard) {
        call->voice_memo_play_stream=voice_memo_start_play(file,sndcard);
    }
}
*/
/*
note:same as leaf_stop_sound_play() API remove it
void leaf_stop_voice_memo_playback(LeafCall *call) {
    if (call->voice_memo_play_stream!=NULL){
        voice_memo_stop_play(call->voice_memo_play_stream);
        call->voice_memo_play_stream=NULL;
    }
}
 */
 
bool leaf_audio_sound_is_playing(void) {
    return AudioSoundPlay;
}

void leaf_start_sound_play(LeafCall *call, char *file,LeafPlaySoundCase playcase,SoundPlayCallback func) {
    MSSndCard *sndcard=call->ring_sndcard;
	const char *playfile=file;
    int interval;

    if (sndcard) ms_snd_card_set_level(sndcard,MS_SND_CARD_PLAYBACK,call->sound_conf.ring_level);
    if(call->ringstream != NULL) leaf_stop_sound_play(call);

    switch(playcase){
        case Normalplay:
            interval = -1;
            break;
        case RepeatPlay:
            interval = 2000;
            break;
        default:
            interval = -1;
    }

    call->ringstream = ring_start_with_cb(file, interval, sndcard, Leaf_media_state_callback, NULL);
     
    AudioSoundPlay = true;
    soundPlayCallback = func;
}

void leaf_stop_sound_play(LeafCall *call) {

    if(call->ringstream){
        ring_stop(call->ringstream);
        call->ringstream=NULL;
        AudioSoundPlay = false;
        Ringstream_release = false ;
        leaf_sound_play_callbackfunc(SOUND_FINISH_PLAY);
        soundPlayCallback = NULL;
    }
    
}

void leaf_pause_sound_play(LeafCall *call,int pause) {

    if(call->ringstream != NULL){
        ring_pause_play(call->ringstream,pause);
        i2s_pause_DAC(pause);
    }
}

void leaf_set_play_level(LeafCall *call,int level){
     call->sound_conf.play_level = level;
     i2s_set_direct_volperc(level);
}

void leaf_set_ring_level(LeafCall *call,int level){
     call->sound_conf.ring_level = level;
     i2s_set_direct_volperc(level);
}

void leaf_set_rec_level(LeafCall *call,int level){
     call->sound_conf.rec_level  = level;
     i2s_ADC_set_direct_volstep(level);
}

double leaf_get_wav_time(char *filename){
    //wav header must be 44byte
    double time = 0.0;
  
    if (filename != NULL && strstr(filename,".wav"))
    {
        FILE* fp;
        fp = fopen(filename, "rb");
        if (fp != NULL)
        {
            int bit_rate,data_size;
            fseek(fp, 28, SEEK_SET);
            fread(&bit_rate, sizeof(bit_rate), 1, fp);
            fseek(fp, 40, SEEK_SET);
            fread(&data_size, sizeof(data_size), 1, fp);        
            fclose(fp);
            fp = NULL;
            time = (double)data_size/(double)bit_rate;
        }
    }
    return time;    
}

void leaf_start_rtsp_stream(const char *addr, int port, char *file) {
    castor3snd_deinit_for_video_memo_play();
#ifdef CFG_RTSP_CLIENT_ENABLE
    SetRTSPClientMode(ADV_MODE);
    startRTSPClient(addr, port, file, NULL);
#endif
}

void leaf_stop_rtsp_stream(void) {
#ifdef CFG_RTSP_CLIENT_ENABLE
    stopRTSPClient();
#endif
    castor3snd_reinit_for_video_memo_play();
} 
    
static MSSndCard *get_sndcard(unsigned int cap){ 
    MSSndCard *sndcard=NULL;
    if (sndcard==NULL) {
        /* get a card that has read+write capabilities */
        sndcard=ms_snd_card_manager_get_default_card(ms_snd_card_manager_get());
        /* otherwise refine to the first card having the right capability*/
        if (sndcard==NULL){
            const MSList *elem=ms_snd_card_manager_get_list(ms_snd_card_manager_get());
            for(;elem!=NULL;elem=elem->next){
                sndcard=(MSSndCard*)elem->data;
                if (ms_snd_card_get_capabilities(sndcard) & cap) break;
            }
        }
        if (sndcard==NULL){/*looks like a bug! take the first one !*/
            const MSList *elem=ms_snd_card_manager_get_list(ms_snd_card_manager_get());
            if (elem) sndcard=(MSSndCard*)elem->data;
        }
    }
    if (sndcard==NULL) ms_error("Could not find a suitable soundcard !");
    return sndcard;
}

static void _check_auto_stop_sound(LeafCall *call){
    time_t curtime=time(NULL);
    if (call->ringstream && Ringstream_release){
        leaf_stop_sound_play(call);
    }
}
