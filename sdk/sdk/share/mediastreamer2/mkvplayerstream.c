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

static void _mkvplayer_stream_notify_callback(void *player, MSFilter *f, unsigned int id, void *data) {
    MKVPlayerStream *obj = (MKVPlayerStream *)player;
    switch(id) {
        case MS_PLAYER_EOF:
            obj->eof = TRUE;
            break;

        case MS_VIDEO_DECODER_FIRST_IMAGE_DECODED:
            obj->firstVideoImage = TRUE;
            break;
    }
}

MKVPlayerStream * mkvplayer_stream_init(const char *filename) {
    MSSndCardManager    *sndCardManager;
    MSSndCard           *sndCard;
    const char          *displayName;
    MKVPlayerStream     *obj;

    obj = ms_new0(MKVPlayerStream, 1);
    if (NULL == obj)
        return obj;

    obj->player = ms_filter_new(MS_MKV_PLAYER_ID);
    ms_filter_set_notify_callback(obj->player, _mkvplayer_stream_notify_callback, obj);

    sndCardManager = ms_snd_card_manager_get();
    sndCard = ms_snd_card_manager_get_default_playback_card(sndCardManager);
    obj->audioSink = ms_snd_card_create_writer(sndCard);

    displayName = video_stream_get_default_video_renderer();
    obj->videoSink = ms_filter_new_from_name(displayName);

    obj->ticker = ms_ticker_new();
    obj->filename = strdup(filename);
    obj->eof = FALSE;
    obj->origTimeIsSet = FALSE;
    obj->firstVideoImage = FALSE;

    return obj;
}

void mkvplayer_stream_uninit(MKVPlayerStream *obj) {
    ms_filter_destroy(obj->player);
    ms_filter_destroy(obj->audioSink);
    ms_filter_destroy(obj->videoSink);
    if(obj->audioDecoder != NULL) ms_filter_destroy(obj->audioDecoder);
    if(obj->videoDecoder != NULL) ms_filter_destroy(obj->videoDecoder);
    ms_ticker_destroy(obj->ticker);
    free(obj->filename);
}

void mkvplayer_stream_start(MKVPlayerStream *obj) {
    MSPinFormat pinFmt;
    int samplerate;

    ms_filter_call_method(obj->player, MS_PLAYER_OPEN, obj->filename);
    pinFmt.pin = 0;
    ms_filter_call_method(obj->player, MS_FILTER_GET_OUTPUT_FMT, &pinFmt);
    if (pinFmt.fmt)
    {
        obj->videoDecoder = ms_factory_create_decoder(ms_factory_get_fallback(), pinFmt.fmt->encoding);
        ms_filter_set_notify_callback(obj->videoDecoder, _mkvplayer_stream_notify_callback, obj);
    }
    pinFmt.pin = 1;
    ms_filter_call_method(obj->player, MS_FILTER_GET_OUTPUT_FMT, &pinFmt);
    obj->audioDecoder = ms_factory_create_decoder(ms_factory_get_fallback(), pinFmt.fmt->encoding);
    samplerate = pinFmt.fmt->rate;
    // ms_filter_call_method(obj->audioDecoder, MS_FILTER_GET_SAMPLE_RATE, &samplerate);
    ms_filter_call_method(obj->audioSink, MS_FILTER_SET_SAMPLE_RATE, &samplerate);

    if (obj->videoDecoder)
    {
        ms_filter_link(obj->player, 0, obj->videoDecoder, 0);
        ms_filter_link(obj->videoDecoder, 0, obj->videoSink, 0);
    }
    ms_filter_link(obj->player, 1, obj->audioDecoder, 0);
    ms_filter_link(obj->audioDecoder, 0, obj->audioSink, 0);
    ms_ticker_attach(obj->ticker, obj->player);
    ms_filter_call_method_noarg(obj->player, MS_PLAYER_START);

    obj->origTime = obj->ticker->time;
    obj->origTimeIsSet = TRUE;
}

void mkvplayer_stream_stop(MKVPlayerStream *obj) {
    ms_filter_call_method_noarg(obj->player, MS_PLAYER_CLOSE);
    ms_ticker_detach(obj->ticker, obj->player);
    if (obj->videoDecoder)
    {
        ms_filter_unlink(obj->player, 0, obj->videoDecoder, 0);
        ms_filter_unlink(obj->videoDecoder, 0, obj->videoSink, 0);
    }
    ms_filter_unlink(obj->player, 1, obj->audioDecoder, 0);
    ms_filter_unlink(obj->audioDecoder, 0, obj->audioSink, 0);
    obj->origTimeIsSet = FALSE;
}

uint64_t mkvplayer_stream_get_time(const MKVPlayerStream *obj) {
    if(obj->origTimeIsSet) {
        return obj->ticker->time - obj->origTime;
    } else {
        return 0;
    }
}

void mkvplayer_stream_set_native_window_id(const MKVPlayerStream *obj, unsigned long id){
    if (obj && obj->videoSink){
        ms_filter_call_method(obj->videoSink,MS_VIDEO_DISPLAY_SET_NATIVE_WINDOW_ID,&id);
    }
}

