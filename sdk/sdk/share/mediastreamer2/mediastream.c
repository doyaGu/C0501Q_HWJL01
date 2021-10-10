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


#include "mediastreamer2/mediastream.h"
#include "private.h"

#if defined(_WIN32_WCE)
time_t
ms_time (time_t *t) {
    DWORD timemillis = GetTickCount();
    if (timemillis>0) {
        if (t!=NULL) *t = timemillis/1000;
    }
    return timemillis/1000;
}
#endif


static void disable_checksums(ortp_socket_t sock){
#if defined(DISABLE_CHECKSUMS) && defined(SO_NO_CHECK)
    int option=1;
    if (setsockopt(sock,SOL_SOCKET,SO_NO_CHECK,&option,sizeof(option))==-1){
        ms_warning("Could not disable udp checksum: %s",strerror(errno));
    }
#endif
}

MSTickerPrio __ms_get_default_prio(bool_t is_video){
    const char *penv;

    if (is_video){
#ifdef __ios
        return MS_TICKER_PRIO_HIGH;
#else
        return MS_TICKER_PRIO_NORMAL;
#endif
    }

    penv=getenv("MS_AUDIO_PRIO");
    if (penv){
        if (strcasecmp(penv,"NORMAL")==0) return MS_TICKER_PRIO_NORMAL;
        if (strcasecmp(penv,"HIGH")==0) return MS_TICKER_PRIO_HIGH;
        if (strcasecmp(penv,"REALTIME")==0) return MS_TICKER_PRIO_REALTIME;
        ms_error("Undefined priority %s", penv);
    }
#ifdef __linux
    return MS_TICKER_PRIO_REALTIME;
#else
    return MS_TICKER_PRIO_HIGH;
#endif
}

RtpSession * create_duplex_rtpsession(int loc_rtp_port, int loc_rtcp_port, bool_t ipv6, bool_t use_multicast) {
    RtpSession *rtpr;

    rtpr=rtp_session_new(RTP_SESSION_SENDRECV);
    rtp_session_set_recv_buf_size(rtpr,MAX_RTP_SIZE);
    rtp_session_set_scheduling_mode(rtpr,0);
    rtp_session_set_blocking_mode(rtpr,0);
    rtp_session_enable_adaptive_jitter_compensation(rtpr,FALSE);
    rtp_session_set_symmetric_rtp(rtpr,TRUE);
    rtp_session_set_local_addr(rtpr,ipv6 ? "::" : "0.0.0.0",loc_rtp_port, loc_rtcp_port);
    rtp_session_signal_connect(rtpr,"timestamp_jump",(RtpCallback)rtp_session_resync,(long)NULL);
    rtp_session_signal_connect(rtpr,"ssrc_changed",(RtpCallback)rtp_session_resync,(long)NULL);
    rtp_session_set_ssrc_changed_threshold(rtpr,0);
    rtp_session_set_rtcp_report_interval(rtpr,2500); /*At the beginning of the session send more reports*/
	if(use_multicast)
		rtp_session_set_multicast_rtp(rtpr,TRUE);
	else
		rtp_session_set_multicast_rtp(rtpr,FALSE);
	disable_checksums(rtp_session_get_rtp_socket(rtpr));

    return rtpr;
}

void media_stream_free(MediaStream *stream) {
    if (stream->sessions.rtp_session!=NULL) {
        rtp_session_unregister_event_queue(stream->sessions.rtp_session, stream->evq);
        rtp_session_destroy(stream->sessions.rtp_session);
    }
    if (stream->sessions.zrtp_context != NULL) {
        ortp_zrtp_context_destroy(stream->sessions.zrtp_context);
        stream->sessions.zrtp_context=NULL;
    }
    if (stream->evq) ortp_ev_queue_destroy(stream->evq);
    if (stream->rtpsend!=NULL) ms_filter_destroy(stream->rtpsend);
    if (stream->rtprecv!=NULL) ms_filter_destroy(stream->rtprecv);
    if (stream->encoder!=NULL) ms_filter_destroy(stream->encoder);
    if (stream->decoder!=NULL) ms_filter_destroy(stream->decoder);
    if (stream->qi) ms_quality_indicator_destroy(stream->qi);

}

void media_stream_set_rtcp_information(MediaStream *stream, const char *cname, const char *tool){
    if (stream->sessions.rtp_session!=NULL){
        rtp_session_set_source_description(stream->sessions.rtp_session,cname,NULL,NULL,NULL,NULL,tool, NULL);
    }
}

void media_stream_enable_adaptive_bitrate_control(MediaStream *stream, bool_t enabled) {
    stream->use_rc = enabled;
}

bool_t ms_is_ipv6(const char *remote){
    bool_t ret=FALSE;
#ifdef INET6
    struct addrinfo hints, *res0;
    int err;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    err = getaddrinfo(remote,"8000", &hints, &res0);
    if (err!=0) {
        ms_warning ("get_local_addr_for: %s", gai_strerror(err));
        return FALSE;
    }
    ret=(res0->ai_addr->sa_family==AF_INET6);
    freeaddrinfo(res0);
#endif
    return ret;
}

float media_stream_get_quality_rating(MediaStream *stream){
    if (stream->qi){
        return ms_quality_indicator_get_rating(stream->qi);
    }
    return -1;
}

float media_stream_get_average_quality_rating(MediaStream *stream){
    if (stream->qi){
        return ms_quality_indicator_get_average_rating(stream->qi);
    }
    return -1;
}


