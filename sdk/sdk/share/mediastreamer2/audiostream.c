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


#ifdef HAVE_CONFIG_H
#include "mediastreamer-config.h"
#endif

#include "mediastreamer2/mediastream.h"

#include "mediastreamer2/dtmfgen.h"
#include "mediastreamer2/mssndcard.h"
#include "mediastreamer2/msrtp.h"
#include "mediastreamer2/msfileplayer.h"
#include "mediastreamer2/msfilerec.h"
#include "mediastreamer2/msvolume.h"
#include "mediastreamer2/msequalizer.h"
#include "mediastreamer2/mscodecutils.h"
#include "mediastreamer2/mstee.h"
#include "mediastreamer2/hwengine.h"
#include "mediastreamer2/msitc.h"
#include "private.h"

#ifdef INET6
    #include <sys/types.h>

#ifndef WIN32
    #include <sys/socket.h>
    #include <netdb.h>
#endif
#endif

//#define PURE_WAV_RECORD
static void configure_itc(AudioStream *stream, LinphoneAudioStreamFlow select_flow);
/* this code is not part of the library itself, it is part of the mediastream program */
static void audio_stream_free(AudioStream *stream) {
    media_stream_free(&stream->ms);
    if (stream->soundread!=NULL) ms_filter_destroy(stream->soundread);
    if (stream->soundwrite!=NULL) ms_filter_destroy(stream->soundwrite);
    if (stream->dtmfgen!=NULL) ms_filter_destroy(stream->dtmfgen);
    if (stream->ec!=NULL)   ms_filter_destroy(stream->ec);
    if (stream->volrecv!=NULL) ms_filter_destroy(stream->volrecv);
    if (stream->volsend!=NULL) ms_filter_destroy(stream->volsend);
    if (stream->equalizerSPK!=NULL) ms_filter_destroy(stream->equalizerSPK);
    if (stream->equalizerMIC!=NULL) ms_filter_destroy(stream->equalizerMIC);
    if (stream->ms.sessions.ticker!=NULL) ms_ticker_destroy(stream->ms.sessions.ticker);
    if (stream->read_resampler!=NULL) ms_filter_destroy(stream->read_resampler);
    if (stream->write_resampler!=NULL) ms_filter_destroy(stream->write_resampler);
    if (stream->dtmfgen_rtp!=NULL) ms_filter_destroy(stream->dtmfgen_rtp);
    if (stream->ms.rc) ms_bitrate_controller_destroy(stream->ms.rc);
    if (stream->itcsink!=NULL) ms_filter_destroy(stream->itcsink);
    if (stream->teeforrecord!=NULL) ms_filter_destroy(stream->teeforrecord);
    if (stream->a_recorder.tee!=NULL) ms_filter_destroy(stream->a_recorder.tee);
    if (stream->a_recorder.itcsink!=NULL) ms_filter_destroy(stream->a_recorder.itcsink);
    if (stream->a_recorder.audio_input!=NULL) ms_filter_destroy(stream->a_recorder.audio_input);
    if (stream->a_recorder.recorder!=NULL) ms_filter_destroy(stream->a_recorder.recorder);
    if (stream->a_recorder.ticker!=NULL) ms_ticker_destroy(stream->a_recorder.ticker);    
    ms_free(stream);
}

static int dtmf_tab[16]={'0','1','2','3','4','5','6','7','8','9','*','#','A','B','C','D'};

static void on_dtmf_received(RtpSession *s, int dtmf, void * user_data)
{
    AudioStream *stream=(AudioStream*)user_data;
    if (dtmf>15){
        ms_warning("Unsupported telephone-event type.");
        return;
    }
    ms_message("Receiving dtmf %c.",dtmf_tab[dtmf]);
    if (stream->dtmfgen!=NULL && stream->play_dtmfs){
        ms_filter_call_method(stream->dtmfgen,MS_DTMF_GEN_PUT,&dtmf_tab[dtmf]);
    }
}

/*
 * note: since not all filters implement MS_FILTER_GET_SAMPLE_RATE, fallback_from_rate and fallback_to_rate are expected to provide sample rates
 * obtained by another context, such as the RTP clock rate for example.
 */
static void audio_stream_configure_resampler(MSFilter *resampler,MSFilter *from,MSFilter *to) {
    int from_rate=0, to_rate=0;
    ms_filter_call_method(from,MS_FILTER_GET_SAMPLE_RATE,&from_rate);
    ms_filter_call_method(to,MS_FILTER_GET_SAMPLE_RATE,&to_rate);
    ms_filter_call_method(resampler,MS_FILTER_SET_SAMPLE_RATE,&from_rate);
    ms_filter_call_method(resampler,MS_FILTER_SET_OUTPUT_SAMPLE_RATE,&to_rate);
    ms_message("configuring %s-->%s from rate[%i] to rate [%i]",
               from->desc->name, to->desc->name, from_rate,to_rate);
}

static void audio_stream_process_rtcp(AudioStream *stream, mblk_t *m){
    do{
        const report_block_t *rb=NULL;
        if (rtcp_is_SR(m)){
            rb=rtcp_SR_get_report_block(m,0);
        }else if (rtcp_is_RR(m)){
            rb=rtcp_RR_get_report_block(m,0);
        }
        if (rb){
            unsigned int ij;
            float rt=rtp_session_get_round_trip_propagation(stream->ms.sessions.rtp_session);
            float flost;
            ij=report_block_get_interarrival_jitter(rb);
            flost=(float)(100.0*report_block_get_fraction_lost(rb)/256.0);
            ms_message("audio_stream_process_rtcp: interarrival jitter=%u , "
                       "lost packets percentage since last report=%f, round trip time=%f seconds",ij,flost,rt);
            if (stream->ms.rc) ms_bitrate_controller_process_rtcp(stream->ms.rc,m);
            if (stream->ms.qi) ms_quality_indicator_update_from_feedback(stream->ms.qi,m);
        }
    }while(rtcp_next_packet(m));
}

static void audio_pure_wav_record_graph_link(MSConnectionHelper *h,AudioStream *stream){
    
    if (stream->a_recorder.tee) { // link pure audio record graph
        ms_connection_helper_link(h, stream->a_recorder.tee, 0, 0);
        ms_filter_link(stream->a_recorder.tee, 1, stream->a_recorder.itcsink, 0);
        ms_filter_link(stream->a_recorder.audio_input,0,stream->a_recorder.recorder,0);
        ms_filter_call_method(stream->a_recorder.itcsink,MS_ITC_SINK_CONNECT,stream->a_recorder.audio_input);            
    }    
}

static void audio_pure_wav_record_graph_unlink(MSConnectionHelper *h,AudioStream *stream){
    if(stream->a_recorder.tee!=NULL) {// unlink pure audio record graph
        MSRecorderState rstate;
        ms_connection_helper_unlink(h,stream->a_recorder.tee,0,0);
        ms_filter_unlink(stream->a_recorder.tee,1,stream->a_recorder.itcsink,0);
        ms_filter_unlink(stream->a_recorder.audio_input,0,stream->a_recorder.recorder,0);

        if (ms_filter_call_method(stream->a_recorder.recorder,MS_FILE_REC_GET_STATE,&rstate)==0){
            if (rstate!=MSRecorderClosed){
                ms_filter_call_method_noarg(stream->a_recorder.recorder, MS_FILE_REC_CLOSE);
            }
        }                              
        ms_filter_call_method(stream->a_recorder.itcsink,MS_ITC_SINK_CONNECT,NULL);
    }
}

static void audio_mkv_rec_graph_link(MSConnectionHelper *h,AudioStream *stream,LinphoneAudioStreamFlow select_flow){
    if (stream->teeforrecord) {
        int pin = 1;
        // [udprecv]--pin0--[teeforrecord]--pin0--
        //ms_filter_call_method(stream->teeforrecord,MS_TEE_MUTE,&pin);
        ms_connection_helper_link (h,stream->teeforrecord,0,0);
        // [udprecv]--pin0--[teeforrecord]--pin0--
        //                                --pin1--[itcsink]
        ms_filter_link(stream->teeforrecord,1,stream->itcsink,0);
        configure_itc(stream, select_flow);
    }
}

static void audio_mkv_rec_graph_unlink(MSConnectionHelper *h,AudioStream *stream){
    if (stream->teeforrecord!=NULL) {
        ms_connection_helper_unlink(h,stream->teeforrecord,0,0);
        ms_filter_unlink(stream->teeforrecord,1,stream->itcsink,0);
    }    
}

void audio_stream_iterate(AudioStream *stream){
    if (stream->ms.is_beginning && ms_time(NULL)-stream->ms.start_time>15){
        rtp_session_set_rtcp_report_interval(stream->ms.sessions.rtp_session,5000);
        stream->ms.is_beginning=FALSE;
    }
    if (stream->ms.evq){
        OrtpEvent *ev=ortp_ev_queue_get(stream->ms.evq);
        if (ev!=NULL){
            OrtpEventType evt=ortp_event_get_type(ev);
            if (evt==ORTP_EVENT_RTCP_PACKET_RECEIVED){
                audio_stream_process_rtcp(stream,ortp_event_get_data(ev)->packet);
                stream->last_packet_time=ms_time(NULL);
            }else if (evt==ORTP_EVENT_RTCP_PACKET_EMITTED){
                /*we choose to update the quality indicator when the oRTP stack decides to emit a RTCP report */
                ms_quality_indicator_update_local(stream->ms.qi);
            }
            ortp_event_destroy(ev);
        }
    }
}

bool_t audio_stream_alive(AudioStream * stream, int timeout){
    const rtp_stats_t *stats=rtp_session_get_stats(stream->ms.sessions.rtp_session);
    if (stats->recv!=0){
        if (stats->recv!=stream->last_packet_count){
            stream->last_packet_count=stats->recv;
            stream->last_packet_time=ms_time(NULL);
        }
    }
    if (stats->recv!=0){
        if (ms_time(NULL)-stream->last_packet_time>timeout){
            /* more than timeout seconds of inactivity*/
            return FALSE;
        }
    }
    return TRUE;
}

/*this function must be called from the MSTicker thread:
it replaces one filter by another one.
This is a dirty hack that works anyway.
It would be interesting to have something that does the job
simplier within the MSTicker api
*/
void audio_stream_change_decoder(AudioStream *stream, int payload){
    RtpSession *session=stream->ms.sessions.rtp_session;
    RtpProfile *prof=rtp_session_get_profile(session);
    PayloadType *pt=rtp_profile_get_payload(prof,payload);
    if (pt!=NULL){
        MSFilter *dec=ms_filter_create_decoder(pt->mime_type);
        if (dec!=NULL){
            ms_filter_unlink(stream->ms.rtprecv, 0, stream->ms.decoder, 0);
            ms_filter_unlink(stream->ms.decoder,0,stream->dtmfgen,0);
            ms_filter_postprocess(stream->ms.decoder);
            ms_filter_destroy(stream->ms.decoder);
            stream->ms.decoder=dec;
            if (pt->recv_fmtp!=NULL)
                ms_filter_call_method(stream->ms.decoder,MS_FILTER_ADD_FMTP,(void*)pt->recv_fmtp);
            ms_filter_link (stream->ms.rtprecv, 0, stream->ms.decoder, 0);
            ms_filter_link (stream->ms.decoder,0 , stream->dtmfgen, 0);
            ms_filter_preprocess(stream->ms.decoder,stream->ms.sessions.ticker);

        }else{
            ms_warning("No decoder found for %s",pt->mime_type);
        }
    }else{
        ms_warning("No payload defined with number %i",payload);
    }
}

static void payload_type_changed(RtpSession *session, unsigned long data){
    AudioStream *stream=(AudioStream*)data;
    int pt=rtp_session_get_recv_payload_type(stream->ms.sessions.rtp_session);
    audio_stream_change_decoder(stream,pt);
}
/*invoked from FEC capable filters*/
static  mblk_t* audio_stream_payload_picker(MSRtpPayloadPickerContext* context,unsigned int sequence_number) {
    return rtp_session_pick_with_cseq(((AudioStream*)(context->filter_graph_manager))->ms.sessions.rtp_session, sequence_number);
}

static void configure_itc(AudioStream *stream, LinphoneAudioStreamFlow select_flow){
    if (stream->itcsink){
        MSPinFormat pinfmt={0};
        RtpSession *session=stream->ms.sessions.rtp_session;
        PayloadType *pt=rtp_profile_get_payload(rtp_session_get_profile(session),rtp_session_get_recv_payload_type(session));

        if (pt){
            pinfmt.pin=0;
            if(select_flow == AudioFromRtpRecv)
                pinfmt.fmt=ms_factory_get_audio_format(ms_factory_get_fallback(),"PCMU",pt->clock_rate,pt->channels,NULL);
            if(select_flow == AudioFromSoundRead)
                pinfmt.fmt=ms_factory_get_audio_format(ms_factory_get_fallback(),"A_PCM",pt->clock_rate,pt->channels,NULL);    
            ms_filter_call_method(stream->itcsink,MS_FILTER_SET_INPUT_FMT,&pinfmt);
            ms_message("configure_itc(): format set to %s",ms_fmt_descriptor_to_string(pinfmt.fmt));
        }else ms_warning("configure_itc(): audio decoder doesn't give output format.");
    }
}

static void _rec_codec_type(VoiceMemoRecordStream *stream,int codec_type){
    
    switch (codec_type){
            case ULAW:
                stream->ms.encoder=ms_filter_new(MS_ULAW_ENC_ID); 
                break;
            
            case ALAW:
                stream->ms.encoder=ms_filter_new(MS_ALAW_ENC_ID);    
                break;
            
            case SPEEX:
                stream->ms.encoder=ms_filter_create_encoder("speex");
                break;

             default:
                break;
    }
}

static void _ring_codec_type(VoiceMemoPlayStream *stream,int codec_type,int *param){
    switch (codec_type){
            case ULAW:
                *param = 0;
                stream->ms.decoder=ms_filter_new(MS_ULAW_DEC_ID);
                break;
            
            case ALAW:
                *param = 0;
                stream->ms.decoder=ms_filter_new(MS_ALAW_DEC_ID);
                break;
            
            case SPEEX:
                *param = 1;
                stream->ms.decoder=ms_filter_create_decoder("speex");
                break;

             default:
                *param = 0;
                break;
    }

}

int audio_stream_start_full(AudioStream *stream, RtpProfile *profile, const char *rem_rtp_ip,int rem_rtp_port,
    const char *rem_rtcp_ip, int rem_rtcp_port, int payload,int jitt_comp, const char *infile, const char *outfile,
    MSSndCard *playcard, MSSndCard *captcard, bool_t use_ec, LinphoneAudioStreamFlow select_flow){
    RtpSession *rtps=stream->ms.sessions.rtp_session;
    PayloadType *pt,*tel_ev;
    int tmp;
    MSConnectionHelper h;
    int sample_rate;
    MSRtpPayloadPickerContext picker_context;

    rtp_session_set_profile(rtps,profile);
    if (rem_rtp_port>0) rtp_session_set_remote_addr_full(rtps,rem_rtp_ip,rem_rtp_port,rem_rtcp_ip,rem_rtcp_port);
    if (rem_rtcp_port > 0) {
        rtp_session_enable_rtcp(rtps, TRUE);
    } else {
        rtp_session_enable_rtcp(rtps, FALSE);
    }
    rtp_session_set_payload_type(rtps,payload);
    rtp_session_set_jitter_compensation(rtps,jitt_comp);

    if (rem_rtp_port>0)
        ms_filter_call_method(stream->ms.rtpsend,MS_RTP_SEND_SET_SESSION,rtps);
    if(stream->receive_graph) stream->ms.rtprecv=ms_filter_new(MS_RTP_RECV_ID);
    if(stream->ms.rtprecv) ms_filter_call_method(stream->ms.rtprecv,MS_RTP_RECV_SET_SESSION,rtps);
    stream->ms.sessions.rtp_session=rtps;

    Castor3snd_reinit_for_diff_rate(CFG_AUDIO_SAMPLING_RATE,16);//check if IIS need reinited or not (sampling rate ,bitsize)
    
    //stream->dtmfgen=ms_filter_new(MS_DTMF_GEN_ID);
    //rtp_session_signal_connect(rtps,"telephone-event",(RtpCallback)on_dtmf_received,(unsigned long)stream);
    //rtp_session_signal_connect(rtps,"payload_type_changed",(RtpCallback)payload_type_changed,(unsigned long)stream);
    /* creates the local part */
    if (captcard!=NULL) stream->soundread=ms_snd_card_create_reader(captcard);
    else {
        stream->soundread=ms_filter_new(MS_FILE_PLAYER_ID);
        stream->read_resampler=ms_filter_new(MS_RESAMPLE_ID);
        if (infile!=NULL) audio_stream_play(stream,infile);
    }
    if (playcard!=NULL) {
        if (stream->soundwrite==NULL)
            stream->soundwrite=ms_snd_card_create_writer(playcard);
    } else {
        stream->soundwrite=ms_filter_new(MS_FILE_REC_ID);
        if (outfile!=NULL) audio_stream_record(stream,outfile);
    }

    /* creates the couple of encoder/decoder */
    pt=rtp_profile_get_payload(profile,payload);
    if (pt==NULL){
        ms_error("audiostream.c: undefined payload type.");
        return -1;
    }
    tel_ev=rtp_profile_get_payload_from_mime (profile,"telephone-event");

    if ( (tel_ev==NULL || ( (tel_ev->flags & PAYLOAD_TYPE_FLAG_CAN_RECV) && !(tel_ev->flags & PAYLOAD_TYPE_FLAG_CAN_SEND)))
        && ( strcasecmp(pt->mime_type,"pcmu")==0 || strcasecmp(pt->mime_type,"pcma")==0)){
        /*if no telephone-event payload is usable and pcma or pcmu is used, we will generate
          inband dtmf*/
        //stream->dtmfgen_rtp=ms_filter_new (MS_DTMF_GEN_ID);
    }

    if (ms_filter_call_method(stream->ms.rtpsend,MS_FILTER_GET_SAMPLE_RATE,&sample_rate)!=0){
        ms_error("Sample rate is unknown for RTP side !");
        return -1;
    }

    stream->ms.encoder=ms_filter_create_encoder(pt->mime_type);
    stream->ms.decoder=ms_filter_create_decoder(pt->mime_type);
    if ((stream->ms.encoder==NULL) || (stream->ms.decoder==NULL)){
        /* big problem: we have not a registered codec for this payload...*/
        ms_error("audio_stream_start_full: No decoder or encoder available for payload %s.",pt->mime_type);
        return -1;
    }

    if (ms_filter_has_method(stream->ms.decoder, MS_FILTER_SET_RTP_PAYLOAD_PICKER)) {
        ms_message("Decoder has FEC capabilities");
        picker_context.filter_graph_manager=stream;
        picker_context.picker=&audio_stream_payload_picker;
        ms_filter_call_method(stream->ms.decoder,MS_FILTER_SET_RTP_PAYLOAD_PICKER, &picker_context);
    }
    if(stream->use_mix)
        stream->mixvoice=ms_filter_new(MS_MIXVOICE_ID);
    if(stream->use_volsend)
        stream->volsend=ms_filter_new(MS_VOLUME_ID);
    if(stream->use_volrecv)
        stream->volrecv=ms_filter_new(MS_VOLUME_ID);
    if (captcard!=NULL && playcard!=NULL && use_ec && stream->receive_graph && stream->send_graph){
        printf("set aec\n");
        //ms_filter_call_method(stream->soundread,MS_FILTER_SET_USEAEC,&use_ec);
        stream->ec=ms_filter_new(MS_SBC_AEC_ID); 
    }    

    if (stream->dtmfgen) {
        ms_filter_call_method(stream->dtmfgen,MS_FILTER_SET_SAMPLE_RATE,&sample_rate);
    }
    if (stream->dtmfgen_rtp) {
        ms_filter_call_method(stream->dtmfgen_rtp,MS_FILTER_SET_SAMPLE_RATE,&sample_rate);
    }
    /* give the sound filters some properties */
//    if (ms_filter_call_method(stream->soundread,MS_FILTER_SET_SAMPLE_RATE,&sample_rate) != 0) {
//        /* need to add resampler*/
//        if (stream->read_resampler == NULL) stream->read_resampler=ms_filter_new(MS_RESAMPLE_ID);
//    }

//    if (ms_filter_call_method(stream->soundwrite,MS_FILTER_SET_SAMPLE_RATE,&sample_rate) != 0) {
//        /* need to add resampler*/
//        if (stream->write_resampler == NULL) stream->write_resampler=ms_filter_new(MS_RESAMPLE_ID);
//    }

//    tmp=1;
//    ms_filter_call_method(stream->soundwrite,MS_FILTER_SET_NCHANNELS, &tmp);

    /*configure the echo canceller if required */

    /* give the encoder/decoder some parameters*/
/*     ms_filter_call_method(stream->ms.encoder,MS_FILTER_SET_SAMPLE_RATE,&pt->clock_rate);
    if (pt->normal_bitrate>0){
        ms_message("Setting audio encoder network bitrate to %i",pt->normal_bitrate);
        ms_filter_call_method(stream->ms.encoder,MS_FILTER_SET_BITRATE,&pt->normal_bitrate);
    }
    ms_filter_call_method(stream->ms.decoder,MS_FILTER_SET_SAMPLE_RATE,&pt->clock_rate);
 */ 

    if (pt->send_fmtp!=NULL) ms_filter_call_method(stream->ms.encoder,MS_FILTER_ADD_FMTP, (void*)pt->send_fmtp);
    if (pt->recv_fmtp!=NULL) ms_filter_call_method(stream->ms.decoder,MS_FILTER_ADD_FMTP,(void*)pt->recv_fmtp);

    /*create the equalizer*/
    if (stream->eq_SPK && CFG_AUDIO_SAMPLING_RATE == 8000)
        stream->equalizerSPK=ms_filter_new(MS_EQUALIZER_ID);

    if (stream->eq_MIC && CFG_AUDIO_SAMPLING_RATE == 8000)
        stream->equalizerMIC=ms_filter_new(MS_EQUALIZER_ID);

    /*configure resampler if needed*/
    if (stream->read_resampler){
        audio_stream_configure_resampler(stream->read_resampler,stream->soundread,stream->ms.rtpsend);
    }

    if (stream->write_resampler && stream->ms.rtprecv){
        audio_stream_configure_resampler(stream->write_resampler,stream->ms.rtprecv,stream->soundwrite);
    }

    if (stream->ms.use_rc){
        stream->ms.rc=ms_audio_bitrate_controller_new(stream->ms.sessions.rtp_session,stream->ms.encoder,0);
    }
    stream->ms.qi=ms_quality_indicator_new(stream->ms.sessions.rtp_session);

    /* and then connect all */
    /* tip: draw yourself the picture if you don't understand */

    /*sending graph*/
    if(stream->send_graph){
    
        ms_connection_helper_start(&h);
        // [soundread]--pin0--
        ms_connection_helper_link(&h,stream->soundread,-1,0);
#ifdef TWO_WAY_AUDIORECORD
        if(select_flow == AudioFromSoundRead)
            audio_mkv_rec_graph_link(&h,stream,select_flow);
#endif
#ifdef PURE_WAV_RECORD  
        if(select_flow == AudioFromSoundRead)
            audio_pure_wav_record_graph_link(&h,stream);
#endif 
        if (stream->read_resampler)
            ms_connection_helper_link(&h,stream->read_resampler,0,0);
        // [soundread]--pin0-- --pin1--[ec]--pin1--
        if (stream->ec)
            ms_connection_helper_link(&h,stream->ec,1,1);
        // [soundread]--pin0-- --pin1--[ec]--pin1-- --pin0--[volsend]--pin0--
        if (stream->equalizerMIC)
            ms_connection_helper_link(&h,stream->equalizerMIC,0,0);    
        if (stream->volsend)
            ms_connection_helper_link(&h,stream->volsend,0,0);
        if (stream->dtmfgen_rtp)
            ms_connection_helper_link(&h,stream->dtmfgen_rtp,0,0);
        // [soundread]--pin0-- --pin1--[ec]--pin1-- --pin0--[volsend]--pin0--[encoder]--pin0--
        if(stream->ms.encoder)
            ms_connection_helper_link(&h,stream->ms.encoder,0,0);
        // [soundread]--pin0-- --pin1--[ec]--pin1-- --pin0--[volsend]--pin0--[encoder]--pin0--[rtpsend]
        ms_connection_helper_link(&h,stream->ms.rtpsend,0,-1);
    }
    
    /*receiving graph*/

    if(stream->receive_graph){
    
        ms_connection_helper_start(&h);
        // [rtprecv]--pin0--
        ms_connection_helper_link(&h,stream->ms.rtprecv,-1,0);
#ifdef TWO_WAY_AUDIORECORD
        if(select_flow == AudioFromRtpRecv)
            audio_mkv_rec_graph_link(&h,stream,select_flow);
#else
            audio_mkv_rec_graph_link(&h,stream,select_flow);
#endif
        // [rtprecv]--pin0--[decoder]--pin0--
        if(stream->ms.decoder)
            ms_connection_helper_link(&h,stream->ms.decoder,0,0);
#ifdef PURE_WAV_RECORD
        if(select_flow == AudioFromRtpRecv)
            audio_pure_wav_record_graph_link(&h,stream);
#endif        
        if (stream->dtmfgen)
            ms_connection_helper_link(&h,stream->dtmfgen,0,0);
        if (stream->volrecv)
            ms_connection_helper_link(&h,stream->volrecv,0,0);
        if (stream->mixvoice)
            ms_connection_helper_link(&h,stream->mixvoice,0,0);
        if (stream->equalizerSPK)
            ms_connection_helper_link(&h,stream->equalizerSPK,0,0);
        if (stream->ec)
            ms_connection_helper_link(&h,stream->ec,0,0);
        if (stream->write_resampler)
            ms_connection_helper_link(&h,stream->write_resampler,0,0);
        ms_connection_helper_link(&h,stream->soundwrite,0,-1);
    }
    /* create ticker */
    stream->ms.sessions.ticker=ms_ticker_new();
    ms_ticker_set_name(stream->ms.sessions.ticker,"Audio MSTicker");
#if (CFG_CHIP_FAMILY == 9910)
    ms_ticker_set_priority(stream->ms.sessions.ticker,__ms_get_default_prio(FALSE));
#endif
    if(stream->send_graph) ms_ticker_attach(stream->ms.sessions.ticker,stream->soundread);
    if(stream->receive_graph) ms_ticker_attach(stream->ms.sessions.ticker,stream->ms.rtprecv);

    stream->ms.start_time=ms_time(NULL);
    stream->ms.is_beginning=TRUE;
#ifdef PURE_WAV_RECORD
    stream->a_recorder.ticker = ms_ticker_new();
    ms_ticker_set_name(stream->a_recorder.ticker,"Audiorec MSTicker");
    ms_ticker_attach(stream->a_recorder.ticker,stream->a_recorder.recorder);
#endif    

    return 0;
}

int audio_stream_start_with_files(AudioStream *stream, RtpProfile *prof,const char *rem_rtp_ip, int rem_rtp_port,
    int rem_rtcp_port, int pt,int jitt_comp, const char *infile, const char * outfile)
{
    return audio_stream_start_full(stream,prof,rem_rtp_ip,rem_rtp_port,rem_rtp_ip,rem_rtcp_port,pt,jitt_comp,infile,outfile,NULL,NULL,FALSE,AudioFromSoundRead);
}

AudioStream * audio_stream_start(RtpProfile *prof,int locport,const char *remip,int remport,int profile,int jitt_comp,bool_t use_ec)
{
    MSSndCard *sndcard_playback;
    MSSndCard *sndcard_capture;
    AudioStream *stream;
    sndcard_capture=ms_snd_card_manager_get_default_capture_card(ms_snd_card_manager_get());
    sndcard_playback=ms_snd_card_manager_get_default_playback_card(ms_snd_card_manager_get());
    if (sndcard_capture==NULL || sndcard_playback==NULL)
        return NULL;
    stream=audio_stream_new(locport, locport+1, ms_is_ipv6(remip));
    if (audio_stream_start_full(stream,prof,remip,remport,remip,remport+1,profile,jitt_comp,NULL,NULL,sndcard_playback,sndcard_capture,use_ec,AudioFromSoundRead)==0) return stream;
    audio_stream_free(stream);
    return NULL;
}

AudioStream *audio_stream_start_with_sndcards(RtpProfile *prof,int locport,const char *remip,int remport,int profile,int jitt_comp,MSSndCard *playcard, MSSndCard *captcard, bool_t use_ec)
{
    AudioStream *stream;
    if (playcard==NULL) {
        ms_error("No playback card.");
        return NULL;
    }
    if (captcard==NULL) {
        ms_error("No capture card.");
        return NULL;
    }
    stream=audio_stream_new(locport, locport+1, ms_is_ipv6(remip));
    if (audio_stream_start_full(stream,prof,remip,remport,remip,remport+1,profile,jitt_comp,NULL,NULL,playcard,captcard,use_ec,AudioFromSoundRead)==0) return stream;
    audio_stream_free(stream);
    return NULL;
}

// Pass NULL to stop playing
void audio_stream_play(AudioStream *st, const char *name){
    if (st->soundread == NULL) {
        ms_warning("Cannot play file: the stream hasn't been started");
        return;
    }
    if (ms_filter_get_id(st->soundread)==MS_FILE_PLAYER_ID){
        ms_filter_call_method_noarg(st->soundread,MS_FILE_PLAYER_CLOSE);
        if (name != NULL) {
            ms_filter_call_method(st->soundread,MS_FILE_PLAYER_OPEN,(void*)name);
            if (st->read_resampler){
                audio_stream_configure_resampler(st->read_resampler,st->soundread,st->ms.rtpsend);
            }
            ms_filter_call_method_noarg(st->soundread,MS_FILE_PLAYER_START);
        }
    }else{
        ms_error("Cannot play file: the stream hasn't been started with"
        " audio_stream_start_with_files");
    }
}

void audio_stream_record(AudioStream *st, const char *name){
    if (ms_filter_get_id(st->soundwrite)==MS_FILE_REC_ID){
        ms_filter_call_method_noarg(st->soundwrite,MS_FILE_REC_CLOSE);
        ms_filter_call_method(st->soundwrite,MS_FILE_REC_OPEN,(void*)name);
        ms_filter_call_method_noarg(st->soundwrite,MS_FILE_REC_START);
    }else{
        ms_error("Cannot record file: the stream hasn't been started with"
        " audio_stream_start_with_files");
    }
}

void audio_stream_start_record_wav(AudioStream *st, const char *name){
    
    if (st&& st->a_recorder.recorder && ms_filter_get_id(st->a_recorder.recorder)==MS_FILE_REC_ID){
        MSRecorderState state;
        int pin=1;
        ms_filter_call_method(st->a_recorder.recorder, MS_FILE_REC_GET_STATE, &state);
        if (state==MSRecorderClosed){
            if ((ms_filter_call_method(st->a_recorder.recorder, MS_FILE_REC_OPEN, (void*)name)) == -1)
                return;
        }        
        ms_filter_call_method_noarg(st->a_recorder.recorder, MS_FILE_REC_START);
        ms_filter_call_method(st->a_recorder.tee, MS_TEE_UNMUTE, &pin);
    }else{
        ms_error("Cannot record file");
    }
}

void audio_stream_stop_record_wav(AudioStream *st){
    if (st && st->a_recorder.recorder && ms_filter_get_id(st->a_recorder.recorder)==MS_FILE_REC_ID){
        ms_filter_call_method_noarg(st->a_recorder.recorder, MS_FILE_REC_CLOSE);
    }else{
        ms_error("Cannot stop record file");
    }
}


AudioStream *audio_stream_new_with_sessions(const MSMediaStreamSessions *sessions){
    AudioStream *stream=(AudioStream *)ms_new0(AudioStream,1);
//    MSFilterDesc *ec_desc=ms_filter_lookup_by_name("MSOslec");

    ms_filter_enable_statistics(TRUE);
    ms_filter_reset_statistics();

    stream->ms.type = AudioStreamType;
    stream->ms.sessions=*sessions;
    /*some filters are created right now to allow configuration by the application before start() */
    stream->ms.rtpsend=ms_filter_new(MS_RTP_SEND_ID);

//    if (ec_desc!=NULL){
//        stream->ec=ms_filter_new_from_desc(ec_desc);
//    }else{
//#if defined(BUILD_WEBRTC_AECM)
//        stream->ec=ms_filter_new(MS_WEBRTC_AEC_ID);
//#else
//        stream->ec=ms_filter_new(MS_SBC_AEC_ID);
//#endif
//    }

#ifdef VIDEO_ENABLED
    /*
     * In practice, these filters are needed only for audio+video recording.
     */
    if (ms_factory_lookup_filter_by_id(ms_factory_get_fallback(), MS_MKV_RECORDER_ID)){
        stream->itcsink=ms_filter_new(MS_ITC_SINK_ID);
        if (stream->itcsink)
            stream->teeforrecord=ms_filter_new(MS_TEE_ID);     
    }
#endif

#ifdef PURE_WAV_RECORD
    stream->a_recorder.itcsink = ms_filter_new(MS_ITC_SINK_ID);
    if(stream->a_recorder.itcsink){
        int encode_type = 1;//HX : PCM wav file  
        stream->a_recorder.tee = ms_filter_new(MS_TEE_ID);
        stream->a_recorder.audio_input = ms_filter_new(MS_ITC_SOURCE_ID);
        stream->a_recorder.recorder = ms_filter_new(MS_FILE_REC_ID);
        ms_filter_call_method_noarg(stream->a_recorder.recorder,MS_FILE_REC_CLOSE);
        ms_filter_call_method(stream->a_recorder.recorder,MS_FILE_REC_SET_SPECIAL_CASE,&encode_type);
    }
#endif

    stream->ms.evq=ortp_ev_queue_new();
    rtp_session_register_event_queue(stream->ms.sessions.rtp_session,stream->ms.evq);
    stream->play_dtmfs=TRUE;
    stream->use_gc=FALSE;
    stream->use_agc=FALSE;
    stream->use_ng=FALSE;
    stream->use_mix=FALSE;
    stream->use_volsend=FALSE;
    stream->use_volrecv=FALSE;
    stream->receive_graph=TRUE;
    stream->send_graph=TRUE;

    return stream;
}

AudioStream *audio_stream_new(int loc_rtp_port, int loc_rtcp_port, bool_t ipv6){
    AudioStream *obj;
    MSMediaStreamSessions sessions={0};
    sessions.rtp_session=create_duplex_rtpsession(loc_rtp_port,loc_rtcp_port,ipv6,FALSE);
    obj=audio_stream_new_with_sessions(&sessions);
    return obj;
}

void audio_stream_play_received_dtmfs(AudioStream *st, bool_t yesno){
    st->play_dtmfs=yesno;
}

int audio_stream_start_now(AudioStream *stream, RtpProfile * prof,  const char *remip, int remport, int rem_rtcp_port, int payload_type, int jitt_comp, MSSndCard *playcard, MSSndCard *captcard, bool_t use_ec){
    return audio_stream_start_full(stream,prof,remip,remport,remip,rem_rtcp_port,
        payload_type,jitt_comp,NULL,NULL,playcard,captcard,use_ec,AudioFromSoundRead);
}

void audio_stream_set_relay_session_id(AudioStream *stream, const char *id){
    ms_filter_call_method(stream->ms.rtpsend, MS_RTP_SEND_SET_RELAY_SESSION_ID,(void*)id);
}

void audio_stream_set_echo_canceller_params(AudioStream *stream, int tail_len_ms, int delay_ms, int framesize){
    if (stream->ec){
        if (tail_len_ms!=0)
            ms_filter_call_method(stream->ec,MS_ECHO_CANCELLER_SET_TAIL_LENGTH,&tail_len_ms);
        if (delay_ms!=0){
            ms_filter_call_method(stream->ec,MS_ECHO_CANCELLER_SET_DELAY,&delay_ms);
        }
        if (framesize!=0)
            ms_filter_call_method(stream->ec,MS_ECHO_CANCELLER_SET_FRAMESIZE,&framesize);
    }
}

void audio_stream_enable_echo_limiter(AudioStream *stream, EchoLimiterType type){
    stream->el_type=type;
    if (stream->volsend){
        bool_t enable_noise_gate = stream->el_type==ELControlFull;
        ms_filter_call_method(stream->volrecv,MS_VOLUME_ENABLE_NOISE_GATE,&enable_noise_gate);
        ms_filter_call_method(stream->volsend,MS_VOLUME_SET_PEER,type!=ELInactive?stream->volrecv:NULL);
    } else {
        ms_warning("cannot set echo limiter to mode [%i] because no volume send",type);
    }
}

void audio_stream_enable_gain_control(AudioStream *stream, bool_t val){
    stream->use_gc=val;
}

void audio_stream_enable_automatic_gain_control(AudioStream *stream, bool_t val){
    stream->use_agc=val;
}

void audio_stream_enable_noise_gate(AudioStream *stream, bool_t val){
    stream->use_ng=val;
    if (stream->volsend){
        ms_filter_call_method(stream->volsend,MS_VOLUME_ENABLE_NOISE_GATE,&val);
    } else {
        ms_warning("cannot set noise gate mode to [%i] because no volume send",val);
    }
}

void audio_stream_set_mic_gain(AudioStream *stream, float gain){
    if (stream->volsend){//gain = -1 mute the mic
        ms_filter_call_method(stream->volsend,MS_VOLUME_SET_GAIN,&gain);
    }else ms_warning("Could not apply gain: gain control wasn't activated. "
            "Use audio_stream_enable_gain_control() before starting the stream.");
}

void audio_stream_enable_equalizer(AudioStream *stream, bool_t enabled){
    stream->eq_SPK=enabled;
    if (stream->equalizerSPK){
        int tmp=enabled;
        ms_filter_call_method(stream->equalizerSPK,MS_EQUALIZER_SET_ACTIVE,&tmp);
    }
}

void audio_stream_equalizer_set_gain(AudioStream *stream, int frequency, float gain, int freq_width){
    if (stream->equalizerSPK){
        MSEqualizerGain d;
        d.frequency=frequency;
        d.gain=gain;
        d.width=freq_width;
        ms_filter_call_method(stream->equalizerSPK,MS_EQUALIZER_SET_GAIN,&d);
    }
}

void audio_stream_stop(AudioStream * stream, LinphoneAudioStreamFlow select_flow){
    if (stream->ms.sessions.ticker){
        MSConnectionHelper h;
        if(stream->send_graph) ms_ticker_detach(stream->ms.sessions.ticker,stream->soundread);
        if(stream->ms.rtprecv) ms_ticker_detach(stream->ms.sessions.ticker,stream->ms.rtprecv);
#ifdef PURE_WAV_RECORD
        ms_ticker_detach(stream->a_recorder.ticker, stream->a_recorder.recorder);        
#endif
        rtp_stats_display(rtp_session_get_stats(stream->ms.sessions.rtp_session),"Audio session's RTP statistics");

        /*dismantle the outgoing graph*/
        if(stream->send_graph){
        
            ms_connection_helper_start(&h);
            ms_connection_helper_unlink(&h,stream->soundread,-1,0);
#ifdef TWO_WAY_AUDIORECORD
            if(select_flow == AudioFromSoundRead)
                audio_mkv_rec_graph_unlink(&h,stream);
#endif
#ifdef PURE_WAV_RECORD
            if(select_flow == AudioFromSoundRead)
                audio_pure_wav_record_graph_unlink(&h,stream);
#endif      
            if (stream->read_resampler!=NULL)
                ms_connection_helper_unlink(&h,stream->read_resampler,0,0);
            if (stream->ec!=NULL)
                ms_connection_helper_unlink(&h,stream->ec,1,1);
            if (stream->equalizerMIC!=NULL)
                ms_connection_helper_unlink(&h,stream->equalizerMIC,0,0);
            if (stream->volsend!=NULL)
                ms_connection_helper_unlink(&h,stream->volsend,0,0);
            if (stream->dtmfgen_rtp)
                ms_connection_helper_unlink(&h,stream->dtmfgen_rtp,0,0);
            if(stream->ms.encoder)
                ms_connection_helper_unlink(&h,stream->ms.encoder,0,0);
            ms_connection_helper_unlink(&h,stream->ms.rtpsend,0,-1);
        }
            
        /*dismantle the receiving graph*/
        if(stream->receive_graph){
            ms_connection_helper_start(&h);
            ms_connection_helper_unlink(&h,stream->ms.rtprecv,-1,0);
#ifdef TWO_WAY_AUDIORECORD
            if(select_flow == AudioFromRtpRecv)
                audio_mkv_rec_graph_unlink(&h,stream);
#else
                audio_mkv_rec_graph_unlink(&h,stream);
#endif  
            if(stream->ms.decoder)
                ms_connection_helper_unlink(&h,stream->ms.decoder,0,0);
#ifdef PURE_WAV_RECORD
            if(select_flow == AudioFromRtpRecv)
                audio_pure_wav_record_graph_unlink(&h,stream);
#endif              
            if (stream->dtmfgen!=NULL)
                ms_connection_helper_unlink(&h,stream->dtmfgen,0,0);
            if (stream->volrecv!=NULL)
                ms_connection_helper_unlink(&h,stream->volrecv,0,0);
            if (stream->mixvoice!=NULL)
                ms_connection_helper_unlink(&h,stream->mixvoice,0,0);        
            if (stream->equalizerSPK!=NULL)
                ms_connection_helper_unlink(&h,stream->equalizerSPK,0,0);
            if (stream->ec!=NULL)
                ms_connection_helper_unlink(&h,stream->ec,0,0);
            if (stream->write_resampler!=NULL)
                ms_connection_helper_unlink(&h,stream->write_resampler,0,0);
            ms_connection_helper_unlink(&h,stream->soundwrite,0,-1);
        }
    }
    audio_stream_free(stream);
    ms_filter_log_statistics();
}

int audio_stream_send_dtmf(AudioStream *stream, char dtmf)
{
    if (stream->dtmfgen_rtp)
        ms_filter_call_method(stream->dtmfgen_rtp,MS_DTMF_GEN_PLAY,&dtmf);
    else if (stream->ms.rtpsend)
        ms_filter_call_method(stream->ms.rtpsend,MS_RTP_SEND_SEND_DTMF,&dtmf);
    return 0;
}

void audio_stream_get_local_rtp_stats(AudioStream *stream, rtp_stats_t *lstats){
    if (stream->ms.sessions.rtp_session){
        const rtp_stats_t *stats=rtp_session_get_stats(stream->ms.sessions.rtp_session);
        memcpy(lstats,stats,sizeof(*stats));
    }else memset(lstats,0,sizeof(rtp_stats_t));
}


void audio_stream_mute_rtp(AudioStream *stream, bool_t val)
{
    if (stream->ms.rtpsend){
        if (val)
            ms_filter_call_method(stream->ms.rtpsend,MS_RTP_SEND_MUTE_MIC,&val);
        else
            ms_filter_call_method(stream->ms.rtpsend,MS_RTP_SEND_UNMUTE_MIC,&val);
    }
}

float audio_stream_get_quality_rating(AudioStream *stream){
    return media_stream_get_quality_rating(&stream->ms);
}

float audio_stream_get_average_quality_rating(AudioStream *stream){
    return media_stream_get_average_quality_rating(&stream->ms);
}

void audio_stream_enable_zrtp(AudioStream *stream, OrtpZrtpParams *params){
    stream->ms.sessions.zrtp_context=ortp_zrtp_context_new(stream->ms.sessions.rtp_session, params);
}

bool_t audio_stream_enable_strp(AudioStream* stream, enum ortp_srtp_crypto_suite_t suite, const char* snd_key, const char* rcv_key) {
    // assign new srtp transport to stream->ms.sessions.rtp_session
    // with 2 Master Keys
    RtpTransport *rtp_tpt, *rtcp_tpt;

    if (!ortp_srtp_supported()) {
        ms_error("ortp srtp support not enabled");
        return FALSE;
    }

    ms_message("%s: stream=%p key='%s' key='%s'", __FUNCTION__,
        stream, snd_key, rcv_key);

    stream->ms.sessions.srtp_session = ortp_srtp_create_configure_session(suite,
        rtp_session_get_send_ssrc(stream->ms.sessions.rtp_session),
        snd_key,
        rcv_key);

    if (!stream->ms.sessions.srtp_session) {
        return FALSE;
    }

    // TODO: check who will free rtp_tpt ?
    srtp_transport_new(stream->ms.sessions.srtp_session, &rtp_tpt, &rtcp_tpt);

    rtp_session_set_transports(stream->ms.sessions.rtp_session, rtp_tpt, rtcp_tpt);

    return TRUE;
}

MS2_PUBLIC VoiceMemoRecordStream *voice_memo_start_record (const char * file, MSSndCard *sndcard)
{
    VoiceMemoRecordStream *stream;
    MSConnectionHelper h;
    char* ext;
    int encode_type = 0;//0:unknow(speex) 1:PCM 6:a-law 7:u-law 
    
    stream=(VoiceMemoRecordStream *)ms_new0(VoiceMemoRecordStream,1);

    stream->sndread=ms_snd_card_create_reader(sndcard);
    
    _rec_codec_type(stream,encode_type);

    stream->filewrite=ms_filter_new(MS_FILE_REC_ID);

    ms_filter_call_method_noarg(stream->filewrite,MS_FILE_REC_CLOSE);
    ms_filter_call_method(stream->filewrite,MS_FILE_REC_SET_SPECIAL_CASE,&encode_type);
    ms_filter_call_method(stream->filewrite,MS_FILE_REC_OPEN,(void*)file);
    ms_filter_call_method_noarg(stream->filewrite,MS_FILE_REC_START);

    stream->ms.sessions.ticker=ms_ticker_new();
    ms_ticker_set_name(stream->ms.sessions.ticker,"Voice Memo Record MSTicker");

    ms_connection_helper_start(&h);
    ms_connection_helper_link(&h,stream->sndread,-1,0);
    if (stream->ms.encoder)
        ms_connection_helper_link(&h,stream->ms.encoder,0,0);
    ms_connection_helper_link(&h,stream->filewrite,0,-1);
    ms_ticker_attach(stream->ms.sessions.ticker,stream->sndread);

    return stream;
}

MS2_PUBLIC void voice_memo_stop_record (VoiceMemoRecordStream * stream)
{
    MSConnectionHelper h;
    ms_ticker_detach(stream->ms.sessions.ticker,stream->sndread);

    ms_connection_helper_start(&h);
    ms_connection_helper_unlink(&h,stream->sndread,-1,0);
    if(stream->ms.encoder)
        ms_connection_helper_unlink(&h,stream->ms.encoder,0,0);
    ms_connection_helper_unlink(&h,stream->filewrite,0,-1);

    ms_ticker_destroy(stream->ms.sessions.ticker);
    ms_filter_destroy(stream->sndread);
    if(stream->ms.encoder)
        ms_filter_destroy(stream->ms.encoder);
    ms_filter_destroy(stream->filewrite);
    ms_free(stream);
}

MS2_PUBLIC VoiceMemoPlayStream *voice_memo_start_play (const char * file, MSSndCard *sndcard)
{
    VoiceMemoPlayStream *stream;
    MSConnectionHelper h;
    char* ext;
    int special_case;
    int codec_type;

    stream=(VoiceMemoPlayStream *)ms_new0(VoiceMemoPlayStream,1);
    
    stream->fileread=ms_filter_new(MS_FILE_PLAYER_ID);
    ms_filter_call_method_noarg(stream->fileread,MS_FILE_PLAYER_CLOSE);
    ms_filter_call_method(stream->fileread,MS_FILE_PLAYER_OPEN,(void*)file);
    ms_filter_call_method(stream->fileread,MS_FILTER_GET_CODEC_TYPE,&codec_type);
    _ring_codec_type(stream,codec_type,&special_case);
    ms_filter_call_method(stream->fileread,MS_FILE_PLAYER_SET_SPECIAL_CASE,&special_case);    
    ms_filter_call_method_noarg(stream->fileread,MS_FILE_PLAYER_START);
    
    stream->sndwrite=ms_snd_card_create_writer(sndcard);

    stream->ms.sessions.ticker=ms_ticker_new();
    ms_ticker_set_name(stream->ms.sessions.ticker,"Voice Memo Play MSTicker");
    stream->ms.sessions.ticker->interval = 20; // to meet the duration of one audio frame

    ms_connection_helper_start(&h);
    ms_connection_helper_link(&h,stream->fileread,-1,0);
    if(stream->ms.decoder)
        ms_connection_helper_link(&h,stream->ms.decoder,0,0);
    ms_connection_helper_link(&h,stream->sndwrite,0,-1);
    ms_ticker_attach(stream->ms.sessions.ticker,stream->fileread);

    return stream;
}

MS2_PUBLIC void voice_memo_stop_play (VoiceMemoPlayStream * stream)
{
    MSConnectionHelper h;
    ms_ticker_detach(stream->ms.sessions.ticker,stream->fileread);

    ms_connection_helper_start(&h);
    ms_connection_helper_unlink(&h,stream->fileread,-1,0);
    if(stream->ms.decoder)
        ms_connection_helper_unlink(&h,stream->ms.decoder,0,0);
    ms_connection_helper_unlink(&h,stream->sndwrite,0,-1);

    ms_ticker_destroy(stream->ms.sessions.ticker);
    ms_filter_destroy(stream->fileread);
    if(stream->ms.decoder)
        ms_filter_destroy(stream->ms.decoder);
    ms_filter_destroy(stream->sndwrite);
    ms_free(stream);
}
#if ENABLE_AUDIO_ENGENEER_MODEL
MS2_PUBLIC Playrecstream *voice_playrec_start(MSSndCard *sndcard, MSSndCard *captcard)
{
    Playrecstream *stream;
    MSConnectionHelper h;
    char* ext;
    int special_case = 2;//wav
    //int samplerate = 16000;
    const char *infile;
    const char *outfile;
    infile = "d:/infile.wav";
    outfile = "d:/outfile.wav";
    stream=(Playrecstream *)ms_new0(Playrecstream,1);
    
    stream->ec=ms_filter_new(MS_SBC_AEC_ID);
    if(stream->ec){
        int delay = CFG_AEC_DELAY_MS + 115;
        ms_filter_call_method(stream->ec,MS_ECHO_CANCELLER_SET_DELAY,&delay);
    }
  
//mic,send
    stream->sndread=ms_snd_card_create_reader(sndcard);
    stream->filewrite=ms_filter_new(MS_FILE_REC_ID);

    ms_filter_call_method_noarg(stream->filewrite,MS_FILE_REC_CLOSE);
    ms_filter_call_method(stream->filewrite,MS_FILE_REC_SET_SPECIAL_CASE,&special_case);
    ms_filter_call_method(stream->filewrite,MS_FILE_REC_OPEN,(void*)outfile);
    //ms_filter_call_method(stream->filewrite,MS_FILTER_SET_SAMPLE_RATE,&samplerate);
    ms_filter_call_method_noarg(stream->filewrite,MS_FILE_REC_START);
    ms_connection_helper_start(&h);
    ms_connection_helper_link(&h,stream->sndread,-1,0);
    if(stream->ec){
        ms_connection_helper_link(&h,stream->ec,1,1);
    }
    ms_connection_helper_link(&h,stream->filewrite,0,-1);    
    
// spk,recv
    stream->fileread=ms_filter_new(MS_FILE_PLAYER_ID);
    ms_filter_call_method_noarg(stream->fileread,MS_FILE_PLAYER_CLOSE);
    ms_filter_call_method(stream->fileread,MS_FILE_PLAYER_SET_SPECIAL_CASE,&special_case);
    ms_filter_call_method(stream->fileread,MS_FILE_PLAYER_OPEN,(void*)infile);
    ms_filter_call_method_noarg(stream->fileread,MS_FILE_PLAYER_START);
    stream->sndwrite=ms_snd_card_create_writer(sndcard);

    
    ms_connection_helper_start(&h);
    ms_connection_helper_link(&h,stream->fileread,-1,0);
    if(stream->ec){
        ms_connection_helper_link(&h,stream->ec,0,0);
    }
    ms_connection_helper_link(&h,stream->sndwrite,0,-1);
    usleep(100000);
    stream->ms.sessions.ticker=ms_ticker_new();
    ms_ticker_set_name(stream->ms.sessions.ticker,"play rec test");   
    ms_ticker_attach(stream->ms.sessions.ticker,stream->fileread);
    ms_ticker_attach(stream->ms.sessions.ticker,stream->sndread);    

    return stream;
}

MS2_PUBLIC void voice_playrec_stop(Playrecstream * stream)
{
    MSConnectionHelper h;
    ms_ticker_detach(stream->ms.sessions.ticker,stream->fileread);
    ms_ticker_detach(stream->ms.sessions.ticker,stream->sndread);
    
//rec destroy      
    ms_connection_helper_start(&h);
    ms_connection_helper_unlink(&h,stream->sndread,-1,0);
    if(stream->ec)
        ms_connection_helper_unlink(&h,stream->ec,1,1);
    ms_connection_helper_unlink(&h,stream->filewrite,0,-1);
    
//playback destroy
    ms_connection_helper_start(&h);
    ms_connection_helper_unlink(&h,stream->fileread,-1,0);
    if(stream->ec)
        ms_connection_helper_unlink(&h,stream->ec,0,0);
    ms_connection_helper_unlink(&h,stream->sndwrite,0,-1);

    ms_filter_destroy(stream->fileread);
    ms_filter_destroy(stream->sndwrite);

    ms_ticker_destroy(stream->ms.sessions.ticker);
    ms_filter_destroy(stream->sndread);
    ms_filter_destroy(stream->filewrite);
    if(stream->ec)
        ms_filter_destroy(stream->ec);
    ms_free(stream);
}
#endif

MS2_PUBLIC void voice_mix_flag (AudioStream* stream,const char * file)
{
    if(stream->mixvoice)
        ms_filter_call_method(stream->mixvoice,MS_WAV_FILE_OPEN,(void*)file); 
}