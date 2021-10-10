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
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msvideo.h"
#include "mediastreamer2/msrtp.h"
#include "mediastreamer2/msvideoout.h"
#include "mediastreamer2/msextdisplay.h"
#include "mediastreamer2/msitc.h"
#include "mediastreamer2/mstee.h"
#include "private.h"

#include <ortp/zrtp.h>

/* this code is not part of the library itself, it is part of the mediastream program */
void video_stream_free (VideoStream * stream) {
    media_stream_free(&stream->ms);

    if (stream->source != NULL)
        ms_filter_destroy (stream->source);
    if (stream->sourceforIPCamStreaming != NULL)
        ms_filter_destroy (stream->sourceforIPCamStreaming);
    if (stream->output != NULL)
        ms_filter_destroy (stream->output);
    if (stream->sizeconv != NULL)
        ms_filter_destroy (stream->sizeconv);
    if (stream->pixconv!=NULL)
        ms_filter_destroy(stream->pixconv);
    if (stream->tee!=NULL)
        ms_filter_destroy(stream->tee);
    if (stream->tee2!=NULL)
        ms_filter_destroy(stream->tee2);
    if (stream->jpegwriter!=NULL)
        ms_filter_destroy(stream->jpegwriter);
    if (stream->filewriter!=NULL)
        ms_filter_destroy(stream->filewriter);
    if (stream->itcsinkforfilewriter!=NULL)
        ms_filter_destroy(stream->itcsinkforfilewriter);
    if (stream->itcsourceforfilewriter!=NULL)
        ms_filter_destroy(stream->itcsourceforfilewriter);
    if (stream->tickerforfilewriter != NULL)
        ms_ticker_destroy(stream->tickerforfilewriter);
    if (stream->output2!=NULL)
        ms_filter_destroy(stream->output2);
    if (stream->ms.sessions.ticker != NULL)
        ms_ticker_destroy (stream->ms.sessions.ticker);
    if (stream->display_name!=NULL)
        ms_free(stream->display_name);
    if (stream->ms.rc!=NULL){
        ms_bitrate_controller_destroy(stream->ms.rc);
    }
    if (stream->av_recorder.recorder!=NULL)
        ms_filter_destroy(stream->av_recorder.recorder);
    if (stream->av_recorder.audio_input!=NULL)
        ms_filter_destroy(stream->av_recorder.audio_input);
    if (stream->av_recorder.video_input!=NULL)
        ms_filter_destroy(stream->av_recorder.video_input);
    if (stream->av_recorder.ticker != NULL)
        ms_ticker_destroy (stream->av_recorder.ticker);
    if (stream->teeforrecord!=NULL)
        ms_filter_destroy(stream->teeforrecord);
    if (stream->itcsink!=NULL)
        ms_filter_destroy(stream->itcsink);

    ms_free (stream);
}

static void event_cb(void *ud, MSFilter* f, unsigned int event, void *eventdata){
    VideoStream *st=(VideoStream*)ud;
    if (st->eventcb!=NULL){
        st->eventcb(st->event_pointer,f,event,eventdata);
    }
}

/*this function must be called from the MSTicker thread:
it replaces one filter by another one.
This is a dirty hack that works anyway.
It would be interesting to have something that does the job
simplier within the MSTicker api
*/
void video_stream_change_decoder(VideoStream *stream, int payload){
    RtpSession *session=stream->ms.sessions.rtp_session;
    RtpProfile *prof=rtp_session_get_profile(session);
    PayloadType *pt=rtp_profile_get_payload(prof,payload);
    if (pt!=NULL){
        MSFilter *dec;

        if (stream->ms.decoder!=NULL && stream->ms.decoder->desc->enc_fmt!=NULL &&
            strcasecmp(pt->mime_type,stream->ms.decoder->desc->enc_fmt)==0){
            /* same formats behind different numbers, nothing to do */
                return;
        }
        dec=ms_filter_create_decoder(pt->mime_type);
        if (dec!=NULL){
            ms_filter_unlink(stream->ms.rtprecv, 0, stream->ms.decoder, 0);
            if (stream->tee2)
                ms_filter_unlink(stream->ms.decoder,0,stream->tee2,0);
            else
                ms_filter_unlink(stream->ms.decoder,0,stream->output,0);
            ms_filter_postprocess(stream->ms.decoder);
            ms_filter_destroy(stream->ms.decoder);
            stream->ms.decoder=dec;
            if (pt->recv_fmtp!=NULL)
                ms_filter_call_method(stream->ms.decoder,MS_FILTER_ADD_FMTP,(void*)pt->recv_fmtp);
            ms_filter_link (stream->ms.rtprecv, 0, stream->ms.decoder, 0);
            if (stream->tee2)
                ms_filter_link(stream->ms.decoder,0,stream->tee2,0);
            else
                ms_filter_link (stream->ms.decoder,0 , stream->output, 0);
            ms_filter_preprocess(stream->ms.decoder,stream->ms.sessions.ticker);
            ms_filter_set_notify_callback(dec, event_cb, stream);
        }else{
            ms_warning("No decoder found for %s",pt->mime_type);
        }
    }else{
        ms_warning("No payload defined with number %i",payload);
    }
}

static void video_stream_process_rtcp(VideoStream *stream, mblk_t *m){
    do{
        if (rtcp_is_SR(m)){
            const report_block_t *rb;
            ms_message("video_stream_process_rtcp: receiving RTCP SR");
            rb=rtcp_SR_get_report_block(m,0);
            if (rb){
                unsigned int ij;
                float rt=rtp_session_get_round_trip_propagation(stream->ms.sessions.rtp_session);
                float flost;
                ij=report_block_get_interarrival_jitter(rb);
                flost=(float)(100.0*report_block_get_fraction_lost(rb)/256.0);
                ms_message("video_stream_process_rtcp: interarrival jitter=%u , lost packets percentage since last report=%f, round trip time=%f seconds",ij,flost,rt);
                if (stream->ms.rc)
                    ms_bitrate_controller_process_rtcp(stream->ms.rc,m);
            }
        }
    }while(rtcp_next_packet(m));
}

static void payload_type_changed(RtpSession *session, unsigned long data){
    VideoStream *stream=(VideoStream*)data;
    int pt=rtp_session_get_recv_payload_type(stream->ms.sessions.rtp_session);
    video_stream_change_decoder(stream,pt);
}

void video_stream_iterate(VideoStream *stream){
    /*
    if (stream->output!=NULL)
        ms_filter_call_method_noarg(stream->output,
            MS_VIDEO_OUT_HANDLE_RESIZING);
    */
    if (stream->ms.evq){
        OrtpEvent *ev;
        while (NULL != (ev=ortp_ev_queue_get(stream->ms.evq))) {
            OrtpEventType evt=ortp_event_get_type(ev);
            if (evt == ORTP_EVENT_RTCP_PACKET_RECEIVED){
                OrtpEventData *evd=ortp_event_get_data(ev);
                video_stream_process_rtcp(stream,evd->packet);
            }
            ortp_event_destroy(ev);
        }
    }
}


void video_stream_link_audio(VideoStream *v_stream, AudioStream *a_stream){
    if ((NULL == v_stream) || (NULL == a_stream))
        return;

    v_stream->audiostream=a_stream;
    if (v_stream->av_recorder.audio_input && a_stream->itcsink){
        ms_message("video_stream_link_audio() connecting itc filters");
        ms_filter_call_method(a_stream->itcsink,MS_ITC_SINK_CONNECT,v_stream->av_recorder.audio_input);
    }
}

void video_stream_unlink_audio(VideoStream *stream, AudioStream *audio){
    if ((NULL == stream) || (NULL == audio))
        return;

    stream->audiostream=NULL;
    if (stream->av_recorder.audio_input && audio->itcsink){
        ms_filter_call_method(audio->itcsink,MS_ITC_SINK_CONNECT,NULL);
    }
}

const char *video_stream_get_default_video_renderer(void){
#if 0 //WIN32
    return "MSDrawDibDisplay";
#elif defined(ANDROID)
    return "MSAndroidDisplay";
#elif __APPLE__ && !defined(__ios)
    return "MSOSXGLDisplay";
#elif defined (HAVE_XV)
    return "MSX11Video";
#elif defined(HAVE_GL)
    return "MSGLXVideo";
#elif defined(__ios)
    return "IOSDisplay";
#else
    return "MSCastor3Display";
#endif
}

static void choose_display_name(VideoStream *stream){
    stream->display_name=ms_strdup(video_stream_get_default_video_renderer());
}

VideoStream *video_stream_new(int loc_rtp_port, int loc_rtcp_port, bool_t use_ipv6, bool_t call_mobile){
    MSMediaStreamSessions sessions={0};
    VideoStream *obj;
	if(call_mobile)
    	sessions.rtp_session=create_duplex_rtpsession(loc_rtp_port,loc_rtcp_port,use_ipv6,FALSE);
	else
		sessions.rtp_session=create_duplex_rtpsession(loc_rtp_port,loc_rtcp_port,use_ipv6,TRUE);
    obj = video_stream_new_with_sessions(&sessions);
    return obj;
}

VideoStream *video_stream_new_with_sessions(const MSMediaStreamSessions *sessions){
    VideoStream *stream = (VideoStream *)ms_new0 (VideoStream, 1);
    stream->ms.type = VideoStreamType;
    stream->ms.sessions=*sessions;
    stream->ms.evq=ortp_ev_queue_new();
    stream->ms.rtpsend=ms_filter_new(MS_RTP_SEND_ID);
    rtp_session_register_event_queue(stream->ms.sessions.rtp_session,stream->ms.evq);
    MS_VIDEO_SIZE_ASSIGN(stream->sent_vsize, CIF);
    stream->dir=VideoStreamSendRecv;
    choose_display_name(stream);
    //stream->ms.process_rtcp=video_stream_process_rtcp;
    return stream;
}

void video_stream_set_sent_video_size(VideoStream *stream, MSVideoSize vsize){
    ms_message("Setting video size %dx%d on stream [%p]", vsize.width, vsize.height,stream);
    stream->sent_vsize=vsize;
}

void video_stream_set_relay_session_id(VideoStream *stream, const char *id){
    ms_filter_call_method(stream->ms.rtpsend, MS_RTP_SEND_SET_RELAY_SESSION_ID,(void*)id);
}

void video_stream_enable_self_view(VideoStream *stream, bool_t val){
    MSFilter *out=stream->output;
    stream->corner=val ? 0 : -1;
    if (out){
        ms_filter_call_method(out,MS_VIDEO_DISPLAY_SET_LOCAL_VIEW_MODE,&stream->corner);
    }
}

void video_stream_set_render_callback (VideoStream *s, VideoStreamRenderCallback cb, void *user_pointer){
    s->rendercb=cb;
    s->render_pointer=user_pointer;
}

void video_stream_set_event_callback (VideoStream *s, VideoStreamEventCallback cb, void *user_pointer){
    s->eventcb=cb;
    s->event_pointer=user_pointer;
}

void video_stream_set_display_filter_name(VideoStream *s, const char *fname){
    if (s->display_name!=NULL){
        ms_free(s->display_name);
        s->display_name=NULL;
    }
    if (fname!=NULL)
        s->display_name=ms_strdup(fname);
}


static void ext_display_cb(void *ud, MSFilter* f, unsigned int event, void *eventdata){
    MSExtDisplayOutput *output=(MSExtDisplayOutput*)eventdata;
    VideoStream *st=(VideoStream*)ud;
    if (st->rendercb!=NULL){
        st->rendercb(st->render_pointer,
                    output->local_view.w!=0 ? &output->local_view : NULL,
                    output->remote_view.w!=0 ? &output->remote_view : NULL);
    }
}

void video_stream_set_direction(VideoStream *vs, VideoStreamDir dir){
    vs->dir=dir;
}

static MSVideoSize get_compatible_size(MSVideoSize maxsize, MSVideoSize wished_size){
    int max_area=maxsize.width*maxsize.height;
    int whished_area=wished_size.width*wished_size.height;
    if (whished_area>max_area){
        return maxsize;
    }
    return wished_size;
}

static MSVideoSize get_with_same_orientation(MSVideoSize size, MSVideoSize refsize){
    if (ms_video_size_get_orientation(refsize)!=ms_video_size_get_orientation(size)){
        int tmp;
        tmp=size.width;
        size.width=size.height;
        size.height=tmp;
    }
    return size;
}

static void configure_video_source(VideoStream *stream){
    MSVideoSize vsize,cam_vsize;
    float fps=15;
    MSPixFmt format;

    /* transmit orientation to source filter */
    ms_filter_call_method(stream->source,MS_VIDEO_CAPTURE_SET_DEVICE_ORIENTATION,&stream->device_orientation);
    /* transmit its preview window id if any to source filter*/
    if (stream->preview_window_id!=0){
        video_stream_set_native_preview_window_id(stream, stream->preview_window_id);
    }

    ms_filter_call_method(stream->ms.encoder,MS_FILTER_GET_VIDEO_SIZE,&vsize);
    vsize=get_compatible_size(vsize,stream->sent_vsize);
    ms_filter_call_method(stream->source,MS_FILTER_SET_VIDEO_SIZE,&vsize);
    /*the camera may not support the target size and suggest a one close to the target */
    ms_filter_call_method(stream->source,MS_FILTER_GET_VIDEO_SIZE,&cam_vsize);
    if (cam_vsize.width*cam_vsize.height<=vsize.width*vsize.height &&
            cam_vsize.width != vsize.width){
        vsize=cam_vsize;
        ms_message("Output video size adjusted to match camera resolution (%ix%i)\n",vsize.width,vsize.height);
    } else if (cam_vsize.width*cam_vsize.height>vsize.width*vsize.height){
#if TARGET_IPHONE_SIMULATOR || defined(__arm__)
        ms_error("Camera is proposing a size bigger than encoder's suggested size (%ix%i > %ix%i) "
                   "Using the camera size as fallback because cropping or resizing is not implemented for arm.",
                   cam_vsize.width,cam_vsize.height,vsize.width,vsize.height);
        vsize=cam_vsize;
#else
        vsize=get_with_same_orientation(vsize,cam_vsize);
        ms_warning("Camera video size greater than encoder one. A scaling filter will be used!\n");
#endif
    }
    ms_filter_call_method(stream->ms.encoder,MS_FILTER_SET_VIDEO_SIZE,&vsize);
    ms_filter_call_method(stream->ms.encoder,MS_FILTER_GET_FPS,&fps);
    ms_message("Setting sent vsize=%ix%i, fps=%f",vsize.width,vsize.height,fps);
    /* configure the filters */
    if (ms_filter_get_id(stream->source)!=MS_STATIC_IMAGE_ID) {
        ms_filter_call_method(stream->source,MS_FILTER_SET_FPS,&fps);
    }
    /* get the output format for webcam reader */
    ms_filter_call_method(stream->source,MS_FILTER_GET_PIX_FMT,&format);

    if (format==MS_MJPEG){
        stream->pixconv=ms_filter_new(MS_MJPEG_DEC_ID);
    }else{
        stream->pixconv = ms_filter_new(MS_PIX_CONV_ID);
        /*set it to the pixconv */
        ms_filter_call_method(stream->pixconv,MS_FILTER_SET_PIX_FMT,&format);
        ms_filter_call_method(stream->pixconv,MS_FILTER_SET_VIDEO_SIZE,&cam_vsize);
    }
    stream->sizeconv=ms_filter_new(MS_SIZE_CONV_ID);
    ms_filter_call_method(stream->sizeconv,MS_FILTER_SET_VIDEO_SIZE,&vsize);
    if (stream->ms.rc){
        ms_bitrate_controller_destroy(stream->ms.rc);
        stream->ms.rc=NULL;
    }
    if (stream->ms.use_rc){
        stream->ms.rc=ms_av_bitrate_controller_new(NULL,NULL,stream->ms.sessions.rtp_session,stream->ms.encoder);
    }
}

static void video_stream_set_recorder_audio_codec(VideoStream *stream) {
    if (stream->av_recorder.audio_input && stream->av_recorder.recorder){
        MSPinFormat pinfmt={0};
        ms_filter_call_method(stream->av_recorder.audio_input,MS_FILTER_GET_OUTPUT_FMT,&pinfmt);
        if (pinfmt.fmt){
            ms_message("Configuring av recorder with video format %s",ms_fmt_descriptor_to_string(pinfmt.fmt));
            pinfmt.pin=1;
            ms_filter_call_method(stream->av_recorder.recorder,MS_FILTER_SET_INPUT_FMT,&pinfmt);
        }
    }
}

static void audio_input_updated(void *stream, MSFilter *f, unsigned int event_id, void *arg){
    if (event_id==MS_FILTER_OUTPUT_FMT_CHANGED){
        ms_message("Audio ITC source updated.");
        video_stream_set_recorder_audio_codec((VideoStream*)stream);
    }
}

static void video_stream_set_recorder_video_codec(VideoStream *stream, const char *mime) {
    MSVideoSize vsize;
    MSPinFormat pinFmt;

    if (!stream || !stream->av_recorder.recorder)
        return;
#ifdef CFG_WIN32_SIMULATOR
    MS_VIDEO_SIZE_ASSIGN(vsize, CIF);
#else
    MS_VIDEO_SIZE_ASSIGN(vsize, 720P);
#endif
    pinFmt.pin = 0;
    pinFmt.fmt = ms_factory_get_video_format(ms_factory_get_fallback(), mime, vsize, 25.00, NULL);
    ms_filter_call_method(stream->av_recorder.recorder, MS_FILTER_SET_INPUT_FMT, &pinFmt);
}

int video_stream_start (VideoStream *stream, RtpProfile *profile, const char *rem_rtp_ip, int rem_rtp_port,
    const char *rem_rtcp_ip, int rem_rtcp_port, int payload, int jitt_comp, bool_t mobile_call, bool_t video_from_ipcam, MSWebCam *cam){
    if (cam==NULL && stream->dir != VideoStreamRecvOnly){
        cam = ms_web_cam_manager_get_default_cam( ms_web_cam_manager_get() );
    }
    //return video_stream_start_with_source(stream, profile, rem_rtp_ip, rem_rtp_port, rem_rtcp_ip, rem_rtcp_port, payload, jitt_comp, cam, ms_web_cam_create_reader(cam));
    if(stream->dir == VideoStreamRecvOnly)
        return video_stream_start_with_source(stream, profile, rem_rtp_ip, rem_rtp_port, rem_rtcp_ip, rem_rtcp_port, payload, jitt_comp, mobile_call, video_from_ipcam, NULL, NULL);
    else
        return video_stream_start_with_source(stream, profile, rem_rtp_ip, rem_rtp_port, rem_rtcp_ip, rem_rtcp_port, payload, jitt_comp, mobile_call, video_from_ipcam, cam, ms_web_cam_create_reader(cam));
}

int video_stream_start_with_source (VideoStream *stream, RtpProfile *profile, const char *rem_rtp_ip, int rem_rtp_port,
    const char *rem_rtcp_ip, int rem_rtcp_port, int payload, int jitt_comp, bool_t mobile_call, bool_t video_from_ipcam, MSWebCam* cam, MSFilter* source){
    PayloadType *pt;
    RtpSession *rtps=stream->ms.sessions.rtp_session;
    MSPixFmt format;
    MSVideoSize disp_size;
    int tmp;
    JBParameters jbp;
    const int socket_buf_size=2000000;
	bool_t    mobile_call_mode = mobile_call;

    if( source == NULL && stream->dir != VideoStreamRecvOnly){
        ms_error("videostream.c: no defined source");
        return -1;
    }

    pt=rtp_profile_get_payload(profile,payload);
    if (pt==NULL){
        ms_error("videostream.c: undefined payload type %d.", payload);
        return -1;
    }

    rtp_session_set_profile(rtps,profile);
    if (rem_rtp_port>0)
        rtp_session_set_remote_addr_full(rtps,rem_rtp_ip,rem_rtp_port,rem_rtcp_ip,rem_rtcp_port);
    rtp_session_set_payload_type(rtps,payload);
    rtp_session_set_jitter_compensation(rtps,jitt_comp);

    rtp_session_signal_connect(stream->ms.sessions.rtp_session,"payload_type_changed",
            (RtpCallback)payload_type_changed,(unsigned long)stream);

    rtp_session_set_recv_buf_size(stream->ms.sessions.rtp_session,MAX_RTP_SIZE);

    rtp_session_get_jitter_buffer_params(stream->ms.sessions.rtp_session,&jbp);
    jbp.max_packets=1000;//needed for high resolution video
    rtp_session_set_jitter_buffer_params(stream->ms.sessions.rtp_session,&jbp);
    rtp_session_set_rtp_socket_recv_buffer_size(stream->ms.sessions.rtp_session,socket_buf_size);
    rtp_session_set_rtp_socket_send_buffer_size(stream->ms.sessions.rtp_session,socket_buf_size);

    if (stream->dir==VideoStreamSendRecv || stream->dir==VideoStreamSendOnly){
#ifndef CFG_FFMPEG_H264_SW
        /* hack for ITE */
        /* filters graph ***************************************
         *
         * castor3cam --> fake h.264 encoder --> RtpSend
         *
         * *****************************************************/
        MSConnectionHelper ch;
        /*plumb the outgoing stream */
        printf("### %s:%d dir=%s\n", __FUNCTION__, __LINE__, stream->dir==VideoStreamSendRecv?"VideoStreamSendRecv":"VideoStreamSendOnly");
        /* to see which codec is really active */
        //printf("### ms_filter_create_encoder: %s\n", pt->mime_type);

        if (rem_rtp_port>0) ms_filter_call_method(stream->ms.rtpsend,MS_RTP_SEND_SET_SESSION,stream->ms.sessions.rtp_session);
        //stream->ms.encoder=ms_filter_create_encoder(pt->mime_type);
        //if (stream->ms.encoder==NULL){
            /* big problem: we don't have a registered codec for this payload...*/
            //ms_error("videostream.c: No encoder available for payload %i:%s.",payload,pt->mime_type);
            //return -1;
        //}
        /* creates the filters */
        stream->cam=cam;
        stream->source = source;
        printf("### %s:%d create cam \"%s\"\n", __FUNCTION__, __LINE__, stream->source->desc->name);
		
		if (mobile_call_mode){
            printf("set mobile call mode\n");
            ms_filter_call_method(stream->source,MS_FILTER_SET_MOBILE,&mobile_call_mode);
        }	
        //if (pt->normal_bitrate>0){
        //    ms_message("Limiting bitrate of video encoder to %i bits/s",pt->normal_bitrate);
        //    ms_filter_call_method(stream->ms.encoder,MS_FILTER_SET_BITRATE,&pt->normal_bitrate);
        //}
        //if (pt->send_fmtp){
        //    printf("### %s:%d need send fmtp=\"%s\"\n", __FUNCTION__, __LINE__, pt->send_fmtp);
        //    ms_filter_call_method(stream->ms.encoder,MS_FILTER_ADD_FMTP,pt->send_fmtp);
        //}

        //configure_video_source (stream);
        /* and then connect all */
        ms_filter_link(stream->source, 0, stream->ms.rtpsend, 0);
#else
        MSConnectionHelper ch;
        /*plumb the outgoing stream */

        if (rem_rtp_port>0) ms_filter_call_method(stream->ms.rtpsend,MS_RTP_SEND_SET_SESSION,stream->ms.sessions.rtp_session);
        stream->ms.encoder=ms_filter_create_encoder(pt->mime_type);
        if (stream->ms.encoder==NULL){
            /* big problem: we don't have a registered codec for this payload...*/
            ms_error("videostream.c: No encoder available for payload %i:%s.",payload,pt->mime_type);
            return -1;
        }
        /* creates the filters */
        stream->cam=cam;
        stream->source = source;
        stream->tee = ms_filter_new(MS_TEE_ID);

        if (pt->normal_bitrate>0){
            ms_message("Limiting bitrate of video encoder to %i bits/s",pt->normal_bitrate);
            ms_filter_call_method(stream->ms.encoder,MS_FILTER_SET_BITRATE,&pt->normal_bitrate);
        }
        if (pt->send_fmtp){
            ms_filter_call_method(stream->ms.encoder,MS_FILTER_ADD_FMTP,pt->send_fmtp);
        }
        if (stream->use_preview_window){
            if (stream->rendercb==NULL){
                stream->output2=ms_filter_new_from_name (stream->display_name);
            }
        }

        configure_video_source (stream);
            /* and then connect all */
        ms_connection_helper_start(&ch);
        ms_connection_helper_link(&ch, stream->source, -1, 0);
        if (stream->pixconv) {
            ms_connection_helper_link(&ch, stream->pixconv, 0, 0);
        }
        ms_connection_helper_link(&ch, stream->tee, 0, 0);
        if (stream->sizeconv) {
            ms_connection_helper_link(&ch, stream->sizeconv, 0, 0);
        }
        ms_connection_helper_link(&ch, stream->ms.encoder, 0, 0);
        ms_connection_helper_link(&ch, stream->ms.rtpsend, 0, -1);
        if (stream->output2){
            if (stream->preview_window_id!=0){
                ms_filter_call_method(stream->output2, MS_VIDEO_DISPLAY_SET_NATIVE_WINDOW_ID,&stream->preview_window_id);
            }
            ms_filter_link(stream->tee,1,stream->output2,0);
        }
#endif
    }
    if (stream->dir==VideoStreamSendRecv || stream->dir==VideoStreamRecvOnly){
#ifndef CFG_FFMPEG_H264_SW
        /* hack for iTE H.264 video stream */
        /* filters graph ***************************************
         *
         * RtpRecv --> H.264 decoder --> castor3display
         *
         * ****************************************************/
        MSConnectionHelper ch;

        printf("### %s:%d dir=%s\n", __FUNCTION__, __LINE__, stream->dir==VideoStreamSendRecv?"VideoStreamSendRecv":"VideoStreamRecvOnly");
        /*to see which codec is really active */
        printf("### ms_filter_create_decoder: %s\n", pt->mime_type);

        /* create decoder first */
        stream->ms.decoder = ms_filter_create_decoder(pt->mime_type);
        if (stream->ms.decoder==NULL){
            /* big problem: we don't have a registered decoderfor this payload...*/
            ms_error("videostream.c: No decoder available for payload %i:%s.",payload,pt->mime_type);
            return -1;
        }
        ms_filter_set_notify_callback(stream->ms.decoder, event_cb, stream);

        if(!video_from_ipcam)
        {
            stream->ms.rtprecv = ms_filter_new (MS_RTP_RECV_ID);
            ms_filter_call_method(stream->ms.rtprecv,MS_RTP_RECV_SET_SESSION,stream->ms.sessions.rtp_session);
            printf("### %s:%d Rtp Recv session %p\n", __FUNCTION__, __LINE__, stream->ms.sessions.rtp_session);
        }
        else
            stream->sourceforIPCamStreaming = ms_filter_new(MS_ITC_IPCAM_ID);

        /* need no JPEG writer, hence we need no tee filter */
        stream->jpegwriter=ms_filter_new(MS_JPEG_WRITER_ID);
        if (stream->jpegwriter){
            stream->tee2=ms_filter_new(MS_TEE_ID);
            stream->filewriter = ms_filter_new(MS_FILE_WRITER_ID);
            stream->itcsinkforfilewriter = ms_filter_new(MS_ITC_SINK_ID);
            stream->itcsourceforfilewriter = ms_filter_new(MS_ITC_SOURCE_ID);
        }    

        if (stream->rendercb!=NULL){
            printf("### %s:%d stream->rendercb exist\n", __FUNCTION__, __LINE__);
            stream->output=ms_filter_new(MS_EXT_DISPLAY_ID);
            ms_filter_set_notify_callback(stream->output,ext_display_cb,stream);
        }else{
            printf("### %s:%d stream->rendercb not exist, create output=%s\n", __FUNCTION__, __LINE__, stream->display_name);
            stream->output=ms_filter_new_from_name (stream->display_name);
        }

        /* set parameters to the decoder*/
        if (pt->send_fmtp){
            printf("### %s:%d send_fmtp\n", __FUNCTION__, __LINE__);
            ms_filter_call_method(stream->ms.decoder,MS_FILTER_ADD_FMTP,pt->send_fmtp);
        }
        if (pt->recv_fmtp!=NULL) {
            printf("### %s:%d recv_fmtp\n", __FUNCTION__, __LINE__);
            ms_filter_call_method(stream->ms.decoder,MS_FILTER_ADD_FMTP,(void*)pt->recv_fmtp);
        }
        stream->av_recorder.recorder = ms_filter_new(MS_MKV_RECORDER_ID);
        if (stream->av_recorder.recorder)
        {
            stream->teeforrecord = ms_filter_new(MS_TEE_ID);
            stream->itcsink = ms_filter_new(MS_ITC_SINK_ID);
            stream->av_recorder.audio_input = ms_filter_new(MS_ITC_SOURCE_ID);
            stream->av_recorder.video_input = ms_filter_new(MS_ITC_SOURCE_ID);
        }
        /* and connect the filters */
        ms_connection_helper_start (&ch);

        // [rtprecv]--pin0--
        if(!video_from_ipcam)
            ms_connection_helper_link (&ch,stream->ms.rtprecv,-1,0);
        else
            ms_connection_helper_link (&ch,stream->sourceforIPCamStreaming,-1,0);
        
        if (stream->teeforrecord) {
            // [rtprecv]--pin0--[teeforrecord]--pin0--
            ms_connection_helper_link (&ch,stream->teeforrecord,0,0);
            // [rtprecv]--pin0--[teeforrecord]--pin0--
            //                                --pin1-- --pin0--[itcsink]
            ms_filter_link(stream->teeforrecord,1,stream->itcsink,0);

            // [itc_source/video]--pin0-- --pin1--[recorder]
            ms_filter_link(stream->av_recorder.video_input,0,stream->av_recorder.recorder,0);
            // [itc_source/audio]--pin0-- --pin1--[recorder]
            ms_filter_link(stream->av_recorder.audio_input,0,stream->av_recorder.recorder,1);
            ms_filter_call_method(stream->itcsink,MS_ITC_SINK_CONNECT,stream->av_recorder.video_input);
        }
        // [rtprecv]--pin0--[decoder]--pin0--
        ms_connection_helper_link (&ch,stream->ms.decoder,0,0);
        if (stream->tee2){
            // [rtprecv]--pin0--[decoder]--pin0--[tee2]--pin0--
            ms_connection_helper_link (&ch,stream->tee2,0,0);
            // [rtprecv]--pin0--[decoder]--pin0--[tee2]--pin0--
            //                                         --pin1--[jpegwriter]
            ms_filter_link(stream->tee2,1,stream->jpegwriter,0);
            ms_filter_link(stream->jpegwriter,0,stream->itcsinkforfilewriter,0);
            ms_filter_link(stream->itcsourceforfilewriter,0,stream->filewriter,0);
            ms_filter_call_method(stream->itcsinkforfilewriter,MS_ITC_SINK_CONNECT,stream->itcsourceforfilewriter);

            //ms_filter_call_method(stream->itcsink,MS_ITC_SINK_CONNECT,stream->av_recorder.video_input);
        }
        // if (stream->tee2)
        //    [rtprecv]--pin0--[decoder]--pin0--[tee2]--pin0--[output]
        //                                            --pin1--[jpegwriter]
        // else
        //    [rtprecv]--pin0--[decoder]--pin0--[output]
        ms_connection_helper_link (&ch,stream->output,0,-1);
        /* the video source must be send for preview , if it exists*/
#else
        MSConnectionHelper ch;

        /* create decoder first */
        stream->ms.decoder=ms_filter_create_decoder(pt->mime_type);
        if (stream->ms.decoder==NULL){
            /* big problem: we don't have a registered decoderfor this payload...*/
            ms_error("videostream.c: No decoder available for payload %i:%s.",payload,pt->mime_type);
            return -1;
        }
        ms_filter_set_notify_callback(stream->ms.decoder, event_cb, stream);

        if(!video_from_ipcam)
        {
            stream->ms.rtprecv = ms_filter_new (MS_RTP_RECV_ID);
            ms_filter_call_method(stream->ms.rtprecv,MS_RTP_RECV_SET_SESSION,stream->ms.sessions.rtp_session);
        }
        else
            stream->sourceforIPCamStreaming = ms_filter_new(MS_ITC_IPCAM_ID);

        stream->jpegwriter=ms_filter_new(MS_JPEG_WRITER_ID);
        if (stream->jpegwriter){
            stream->tee2=ms_filter_new(MS_TEE_ID);
            stream->filewriter = ms_filter_new(MS_FILE_WRITER_ID);
            stream->itcsinkforfilewriter = ms_filter_new(MS_ITC_SINK_ID);
            stream->itcsourceforfilewriter = ms_filter_new(MS_ITC_SOURCE_ID);
        }

        if (stream->rendercb!=NULL){
            stream->output=ms_filter_new(MS_EXT_DISPLAY_ID);
            ms_filter_set_notify_callback(stream->output,ext_display_cb,stream);
        }else{
            stream->output=ms_filter_new_from_name (stream->display_name);
        }

        /* set parameters to the decoder*/
        if (pt->send_fmtp){
            ms_filter_call_method(stream->ms.decoder,MS_FILTER_ADD_FMTP,pt->send_fmtp);
        }
        if (pt->recv_fmtp!=NULL)
            ms_filter_call_method(stream->ms.decoder,MS_FILTER_ADD_FMTP,(void*)pt->recv_fmtp);

        /*force the decoder to output YUV420P */
        format=MS_YUV420P;
        ms_filter_call_method(stream->ms.decoder,MS_FILTER_SET_PIX_FMT,&format);

        /*configure the display window */
        if(stream->output != NULL) {
            disp_size.width=MS_VIDEO_SIZE_CIF_W;
            disp_size.height=MS_VIDEO_SIZE_CIF_H;
            tmp=1;
            ms_filter_call_method(stream->output,MS_FILTER_SET_VIDEO_SIZE,&disp_size);
            ms_filter_call_method(stream->output,MS_VIDEO_DISPLAY_ENABLE_AUTOFIT,&tmp);
            ms_filter_call_method(stream->output,MS_FILTER_SET_PIX_FMT,&format);
            ms_filter_call_method(stream->output,MS_VIDEO_DISPLAY_SET_LOCAL_VIEW_MODE,&stream->corner);
            if (stream->window_id!=0){
                ms_filter_call_method(stream->output, MS_VIDEO_DISPLAY_SET_NATIVE_WINDOW_ID,&stream->window_id);
            }
        }
        stream->av_recorder.recorder = ms_filter_new(MS_MKV_RECORDER_ID);
        if (stream->av_recorder.recorder)
        {
            stream->teeforrecord = ms_filter_new(MS_TEE_ID);
            stream->itcsink = ms_filter_new(MS_ITC_SINK_ID);
            stream->av_recorder.audio_input = ms_filter_new(MS_ITC_SOURCE_ID);
            stream->av_recorder.video_input = ms_filter_new(MS_ITC_SOURCE_ID);
        }
        /* and connect the filters */
        ms_connection_helper_start (&ch);

        // [rtprecv]--pin0--
        if(!video_from_ipcam)
            ms_connection_helper_link (&ch,stream->ms.rtprecv,-1,0);
        else
            ms_connection_helper_link (&ch,stream->sourceforIPCamStreaming,-1,0);
        
        if (stream->teeforrecord) {
            // [rtprecv]--pin0--[teeforrecord]--pin0--
            ms_connection_helper_link (&ch,stream->teeforrecord,0,0);
            // [rtprecv]--pin0--[teeforrecord]--pin0--
            //                                --pin1-- --pin0--[itcsink]
            ms_filter_link(stream->teeforrecord,1,stream->itcsink,0);

            // [itc_source/video]--pin0-- --pin1--[recorder]
            ms_filter_link(stream->av_recorder.video_input,0,stream->av_recorder.recorder,0);
            // [itc_source/audio]--pin0-- --pin1--[recorder]
            ms_filter_link(stream->av_recorder.audio_input,0,stream->av_recorder.recorder,1);
            ms_filter_call_method(stream->itcsink,MS_ITC_SINK_CONNECT,stream->av_recorder.video_input);
        }
        // [rtprecv]--pin0--[decoder]--pin0--
        ms_connection_helper_link (&ch,stream->ms.decoder,0,0);
        if (stream->tee2){
            // [rtprecv]--pin0--[decoder]--pin0--[tee2]--pin0--
            ms_connection_helper_link (&ch,stream->tee2,0,0);
            // [rtprecv]--pin0--[decoder]--pin0--[tee2]--pin0--
            //                                         --pin1--[jpegwriter]
            ms_filter_link(stream->tee2,1,stream->jpegwriter,0);
            ms_filter_link(stream->jpegwriter,0,stream->itcsinkforfilewriter,0);
            ms_filter_link(stream->itcsourceforfilewriter,0,stream->filewriter,0);
            ms_filter_call_method(stream->itcsinkforfilewriter,MS_ITC_SINK_CONNECT,stream->itcsourceforfilewriter);

        }
        // if (stream->tee2)
        //    [rtprecv]--pin0--[decoder]--pin0--[tee2]--pin0--[output]
        //                                            --pin1--[jpegwriter]
        // else
        //    [rtprecv]--pin0--[decoder]--pin0--[output]
        ms_connection_helper_link (&ch,stream->output,0,-1);
        /* the video source must be send for preview , if it exists*/
        if (stream->tee!=NULL && stream->output2==NULL)
            ms_filter_link(stream->tee,1,stream->output,1);
#endif
        video_stream_set_recorder_video_codec(stream, pt->mime_type);
        if (stream->av_recorder.recorder)
            ms_filter_set_notify_callback(stream->av_recorder.audio_input,audio_input_updated,stream);
    }

    /* create the ticker */
    stream->ms.sessions.ticker = ms_ticker_new();
    ms_ticker_set_name(stream->ms.sessions.ticker,"Video MSTicker");
	if (stream->dir==VideoStreamSendRecv || stream->dir==VideoStreamRecvOnly){
    	stream->av_recorder.ticker = ms_ticker_new();
    	ms_ticker_set_name(stream->av_recorder.ticker,"MKVRecoder MSTicker");
    	stream->tickerforfilewriter =  ms_ticker_new();
    	ms_ticker_set_name(stream->tickerforfilewriter,"FileWriter MSTicker");
	}

    /* attach the graphs */
    if (stream->source)
        ms_ticker_attach (stream->ms.sessions.ticker, stream->source);    
    if (stream->ms.rtprecv && !video_from_ipcam)
        ms_ticker_attach (stream->ms.sessions.ticker, stream->ms.rtprecv);
    else if (stream->sourceforIPCamStreaming)
        ms_ticker_attach (stream->ms.sessions.ticker, stream->sourceforIPCamStreaming);
	if (stream->dir==VideoStreamSendRecv || stream->dir==VideoStreamRecvOnly){
    	if (stream->av_recorder.video_input)
        	ms_ticker_attach (stream->av_recorder.ticker, stream->av_recorder.video_input);
    	if (stream->itcsourceforfilewriter)
        	ms_ticker_attach (stream->tickerforfilewriter, stream->itcsourceforfilewriter);
	}
    return 0;
}

void video_stream_update_video_params(VideoStream *stream){
    /*calling video_stream_change_camera() does the job of unplumbing/replumbing and configuring the new graph*/
    video_stream_change_camera(stream,stream->cam);
}

void video_stream_change_camera(VideoStream *stream, MSWebCam *cam){
    bool_t keep_source=(cam==stream->cam);

    if (stream->ms.sessions.ticker && stream->source){
        ms_ticker_detach(stream->ms.sessions.ticker,stream->source);
        /*unlink source filters and subsequent post processing filters */
        ms_filter_unlink (stream->source, 0, stream->pixconv, 0);
        ms_filter_unlink (stream->pixconv, 0, stream->sizeconv, 0);
        ms_filter_unlink (stream->sizeconv, 0, stream->tee, 0);
        /*destroy the filters */
        if (!keep_source) ms_filter_destroy(stream->source);
        ms_filter_destroy(stream->pixconv);
        ms_filter_destroy(stream->sizeconv);

        /*re create new ones and configure them*/
        if (!keep_source) {
            stream->source = ms_web_cam_create_reader(cam);
        }
        stream->cam=cam;
        configure_video_source(stream);

        ms_filter_link (stream->source, 0, stream->pixconv, 0);
        ms_filter_link (stream->pixconv, 0, stream->tee, 0);
        ms_filter_link (stream->tee, 0, stream->sizeconv, 0);

        ms_ticker_attach(stream->ms.sessions.ticker,stream->source);
    }
}

void video_stream_send_vfu(VideoStream *stream){
    if (stream->ms.encoder)
        ms_filter_call_method_noarg(stream->ms.encoder,MS_FILTER_REQ_VFU);
}

void
video_stream_stop (VideoStream * stream)
{
    stream->eventcb = NULL;
    stream->event_pointer = NULL;
    if (stream->ms.sessions.ticker){
        if (stream->source)
            ms_ticker_detach(stream->ms.sessions.ticker,stream->source);
        if (stream->ms.rtprecv)
            ms_ticker_detach(stream->ms.sessions.ticker,stream->ms.rtprecv);
        else if (stream->sourceforIPCamStreaming)
            ms_ticker_detach(stream->ms.sessions.ticker,stream->sourceforIPCamStreaming);
        if (stream->av_recorder.video_input)
            ms_ticker_detach(stream->av_recorder.ticker,stream->av_recorder.video_input);
        if (stream->itcsourceforfilewriter)
            ms_ticker_detach(stream->tickerforfilewriter, stream->itcsourceforfilewriter);

        rtp_stats_display(rtp_session_get_stats(stream->ms.sessions.rtp_session),"Video session's RTP statistics");

        if (stream->source){
#ifndef CFG_FFMPEG_H264_SW
            ms_filter_unlink(stream->source, 0, stream->ms.rtpsend,0);
#else
            MSConnectionHelper ch;
            ms_connection_helper_start(&ch);
            ms_connection_helper_unlink(&ch, stream->source, -1, 0);
            if (stream->pixconv) {
                ms_connection_helper_unlink(&ch, stream->pixconv, 0, 0);
            }
            ms_connection_helper_unlink(&ch, stream->tee, 0, 0);
            if (stream->sizeconv) {
                ms_connection_helper_unlink(&ch, stream->sizeconv, 0, 0);
            }
            ms_connection_helper_unlink(&ch, stream->ms.encoder, 0, 0);
            ms_connection_helper_unlink(&ch, stream->ms.rtpsend, 0, -1);
            if (stream->output2){
                ms_filter_unlink(stream->tee,1,stream->output2,0);
            }
#endif
        }
        if (stream->ms.rtprecv || stream->sourceforIPCamStreaming){
            MSConnectionHelper h;
            ms_connection_helper_start (&h);
            if(stream->ms.rtprecv)
                ms_connection_helper_unlink (&h,stream->ms.rtprecv,-1,0);
            else if(stream->sourceforIPCamStreaming)
                ms_connection_helper_unlink (&h,stream->sourceforIPCamStreaming,-1,0);
            if (stream->teeforrecord) {
                MSRecorderState rstate;
                ms_connection_helper_unlink(&h,stream->teeforrecord,0,0);
                ms_filter_unlink(stream->teeforrecord,1,stream->itcsink,0);

                ms_filter_unlink(stream->av_recorder.video_input,0,stream->av_recorder.recorder,0);
                ms_filter_unlink(stream->av_recorder.audio_input,0,stream->av_recorder.recorder,1);
                if (ms_filter_call_method(stream->av_recorder.recorder,MS_RECORDER_GET_STATE,&rstate)==0){
                    if (rstate!=MSRecorderClosed){
                        ms_filter_call_method_noarg(stream->av_recorder.recorder, MS_RECORDER_CLOSE);
                    }
                }

                ms_filter_call_method(stream->itcsink,MS_ITC_SINK_CONNECT,NULL);
            }
            ms_connection_helper_unlink (&h,stream->ms.decoder,0,0);
            if (stream->tee2){
                ms_connection_helper_unlink (&h,stream->tee2,0,0);
                ms_filter_unlink(stream->tee2,1,stream->jpegwriter,0);
                ms_filter_unlink(stream->jpegwriter,0,stream->itcsinkforfilewriter,0);
                ms_filter_unlink(stream->itcsourceforfilewriter,0,stream->filewriter,0);

                ms_filter_call_method(stream->itcsinkforfilewriter,MS_ITC_SINK_CONNECT, NULL);

            }
            if(stream->output)
                ms_connection_helper_unlink(&h,stream->output,0,-1);
            if (stream->tee && stream->output2==NULL)
                ms_filter_unlink(stream->tee,1,stream->output,1);
        }
    }
    video_stream_free (stream);
}


void video_stream_show_video(VideoStream *stream, bool_t show){
    if (stream->output){
        ms_filter_call_method(stream->output,MS_VIDEO_DISPLAY_SHOW_VIDEO,&show);
    }
}


unsigned long video_stream_get_native_window_id(VideoStream *stream){
    unsigned long id;
    if (stream->output){
        if (ms_filter_call_method(stream->output,MS_VIDEO_DISPLAY_GET_NATIVE_WINDOW_ID,&id)==0)
            return id;
    }
    return stream->window_id;
}

void video_stream_set_native_window_id(VideoStream *stream, unsigned long id){
    stream->window_id=id;
    if (stream->output){
        ms_filter_call_method(stream->output,MS_VIDEO_DISPLAY_SET_NATIVE_WINDOW_ID,&id);
    }
}

void video_stream_set_native_preview_window_id(VideoStream *stream, unsigned long id){
    stream->preview_window_id=id;
#ifndef __ios
    if (stream->output2){
        ms_filter_call_method(stream->output2,MS_VIDEO_DISPLAY_SET_NATIVE_WINDOW_ID,&id);
    }
#endif
    if (stream->source){
        ms_filter_call_method(stream->source,MS_VIDEO_DISPLAY_SET_NATIVE_WINDOW_ID,&id);
    }
}

unsigned long video_stream_get_native_preview_window_id(VideoStream *stream){
    unsigned long id=0;
    if (stream->output2){
        if (ms_filter_call_method(stream->output2,MS_VIDEO_DISPLAY_GET_NATIVE_WINDOW_ID,&id)==0)
            return id;
    }
    if (stream->source){
        if (ms_filter_has_method(stream->source,MS_VIDEO_DISPLAY_GET_NATIVE_WINDOW_ID)
            && ms_filter_call_method(stream->source,MS_VIDEO_DISPLAY_GET_NATIVE_WINDOW_ID,&id)==0)
            return id;
    }
    return stream->preview_window_id;
}

void video_stream_use_preview_video_window(VideoStream *stream, bool_t yesno){
    stream->use_preview_window=yesno;
}

void video_stream_set_device_rotation(VideoStream *stream, int orientation){
    MSFilter* target_filter;

    if (stream == 0)
        return;

    stream->device_orientation = orientation;
    target_filter=stream->source;
    if (target_filter){
        ms_filter_call_method(target_filter,MS_VIDEO_CAPTURE_SET_DEVICE_ORIENTATION,&orientation);
    }
}

VideoPreview * video_preview_new(void){
    VideoPreview *stream = (VideoPreview *)ms_new0 (VideoPreview, 1);
    MS_VIDEO_SIZE_ASSIGN(stream->sent_vsize, CIF);
    choose_display_name(stream);
    return stream;
}


void video_preview_start(VideoPreview *stream, MSWebCam *device){
    MSPixFmt format;
    float fps=(float)29.97;
    int mirroring=1;
    int corner=-1;
    MSVideoSize disp_size=stream->sent_vsize;
    MSVideoSize vsize=disp_size;
    const char *displaytype=stream->display_name;

    stream->source = ms_web_cam_create_reader(device);


    /* configure the filters */
    ms_filter_call_method(stream->source,MS_FILTER_SET_VIDEO_SIZE,&vsize);
    if (ms_filter_get_id(stream->source)!=MS_STATIC_IMAGE_ID)
        ms_filter_call_method(stream->source,MS_FILTER_SET_FPS,&fps);
    ms_filter_call_method(stream->source,MS_FILTER_GET_PIX_FMT,&format);
    ms_filter_call_method(stream->source,MS_FILTER_GET_VIDEO_SIZE,&vsize);
    if (format==MS_MJPEG){
        stream->pixconv=ms_filter_new(MS_MJPEG_DEC_ID);
    }else{
        stream->pixconv=ms_filter_new(MS_PIX_CONV_ID);
        ms_filter_call_method(stream->pixconv,MS_FILTER_SET_PIX_FMT,&format);
        ms_filter_call_method(stream->pixconv,MS_FILTER_SET_VIDEO_SIZE,&vsize);
    }

    format=MS_YUV420P;

    stream->output2=ms_filter_new_from_name (displaytype);
    ms_filter_call_method(stream->output2,MS_FILTER_SET_PIX_FMT,&format);
    ms_filter_call_method(stream->output2,MS_FILTER_SET_VIDEO_SIZE,&disp_size);
    ms_filter_call_method(stream->output2,MS_VIDEO_DISPLAY_ENABLE_MIRRORING,&mirroring);
    ms_filter_call_method(stream->output2,MS_VIDEO_DISPLAY_SET_LOCAL_VIEW_MODE,&corner);
    /* and then connect all */

    ms_filter_link(stream->source,0, stream->pixconv,0);
    ms_filter_link(stream->pixconv, 0, stream->output2, 0);

    if (stream->preview_window_id!=0){
        video_stream_set_native_preview_window_id(stream, stream->preview_window_id);
    }

    /* create the ticker */
    stream->ms.sessions.ticker = ms_ticker_new();
    ms_ticker_set_name(stream->ms.sessions.ticker,"Video MSTicker");
    ms_ticker_attach (stream->ms.sessions.ticker, stream->source);
}

void video_preview_stop(VideoStream *stream){
    ms_ticker_detach(stream->ms.sessions.ticker, stream->source);
    ms_filter_unlink(stream->source,0,stream->pixconv,0);
    ms_filter_unlink(stream->pixconv,0,stream->output2,0);
    video_stream_free(stream);
}


int video_stream_recv_only_start(VideoStream *videostream, RtpProfile *profile, const char *addr, int port, int used_pt, int jitt_comp){
    video_stream_set_direction(videostream,VideoStreamRecvOnly);
    return video_stream_start(videostream,profile,addr,port,addr,port+1,used_pt,jitt_comp,FALSE, FALSE, NULL);
}

int video_stream_send_only_start(VideoStream *videostream,
                RtpProfile *profile, const char *addr, int port, int rtcp_port,
                int used_pt, int  jitt_comp, MSWebCam *device){
    video_stream_set_direction (videostream,VideoStreamSendOnly);
    return video_stream_start(videostream,profile,addr,port,addr,rtcp_port,used_pt,jitt_comp,FALSE, FALSE, device);
}


void video_stream_recv_only_stop(VideoStream *vs){
    video_stream_stop(vs);
}

void video_stream_send_only_stop(VideoStream *vs){
    video_stream_stop(vs);
}

void video_stream_start_record_mkv(VideoStream *st, const char *name){
    if (st 
     && st->av_recorder.recorder
     && ms_filter_get_id(st->av_recorder.recorder)==MS_MKV_RECORDER_ID){
        MSRecorderState state;
        int pin=1;
        ms_filter_call_method(st->av_recorder.recorder, MS_RECORDER_GET_STATE, &state);
        if (state==MSRecorderClosed){
            if ((ms_filter_call_method(st->av_recorder.recorder, MS_RECORDER_OPEN, (void*)name)) == -1)
                return;
        }
        ms_filter_call_method_noarg(st->av_recorder.recorder, MS_RECORDER_START);
        ms_filter_call_method(st->teeforrecord, MS_TEE_UNMUTE, &pin);
    }else{
        ms_error("Cannot record file");
    }
}

void video_stream_stop_record_mkv(VideoStream *st){
    if (st 
     && st->av_recorder.recorder
     && ms_filter_get_id(st->av_recorder.recorder)==MS_MKV_RECORDER_ID){
        ms_filter_call_method_noarg(st->av_recorder.recorder, MS_RECORDER_CLOSE);
    }else{
        ms_error("Cannot stop record file");
    }
}

void video_stream_start_play_mkv(VideoStream *st, const char *name){
    //if (ms_filter_get_id(st->recorder)==MS_MKV_RECORDER_ID){
       // ms_filter_call_method(st->recorder, MS_RECORDER_OPEN, (void*)name);
       // ms_filter_call_method_noarg(st->recorder, MS_RECORDER_START);
    //}else{
    //    ms_error("Cannot record file");
    //}
}

void video_stream_stop_play_mkv(VideoStream *st){
}

/* enable ZRTP on the video stream using information from the audio stream */
void video_stream_enable_zrtp(VideoStream *vstream, AudioStream *astream, OrtpZrtpParams *param){
    if (astream->ms.sessions.zrtp_context != NULL) {
        vstream->ms.sessions.zrtp_context=ortp_zrtp_multistream_new(astream->ms.sessions.zrtp_context, vstream->ms.sessions.rtp_session, param);
    }
}

bool_t video_stream_enable_strp(VideoStream* stream, enum ortp_srtp_crypto_suite_t suite, const char* snd_key, const char* rcv_key) {
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
