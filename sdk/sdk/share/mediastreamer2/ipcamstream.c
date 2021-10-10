#ifdef HAVE_CONFIG_H
#include "mediastreamer-config.h"
#endif

#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
#include "mediastreamer2/msitc.h"
#include "mediastreamer2/msjpegwriter.h"
#include "private.h"

pthread_mutex_t    ipcam_mutex = PTHREAD_MUTEX_INITIALIZER;

static void ipcam_stream_set_recorder_video_codec(VideoStream *stream, const char *mime) {
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

VideoStream * ipcam_streaming_start()
{
    VideoStream *stream;
    MSConnectionHelper ch;

    pthread_mutex_lock(&ipcam_mutex);
    stream=(VideoStream *)ms_new0(VideoStream,1);
    stream->source = ms_filter_new(MS_ITC_IPCAM_ID);
    stream->ms.decoder = ms_filter_create_decoder("H264");
    stream->output=ms_filter_new_from_name ("MSCastor3Display");

    stream->av_recorder.recorder = ms_filter_new(MS_MKV_RECORDER_ID);
    if (stream->av_recorder.recorder)
    {
        stream->teeforrecord = ms_filter_new(MS_TEE_ID);
        stream->itcsink = ms_filter_new(MS_ITC_SINK_ID);
        stream->av_recorder.video_input = ms_filter_new(MS_ITC_SOURCE_ID);
    }

    stream->jpegwriter=ms_filter_new(MS_JPEG_WRITER_ID);
    if (stream->jpegwriter){
        stream->tee2=ms_filter_new(MS_TEE_ID);
        stream->filewriter = ms_filter_new(MS_FILE_WRITER_ID);
        stream->itcsinkforfilewriter = ms_filter_new(MS_ITC_SINK_ID);
        stream->itcsourceforfilewriter = ms_filter_new(MS_ITC_SOURCE_ID);
    }
    
    ms_connection_helper_start (&ch);
    ms_connection_helper_link (&ch,stream->source,-1,0);
    
    if (stream->teeforrecord) {
        ms_connection_helper_link (&ch,stream->teeforrecord,0,0);
        ms_filter_link(stream->teeforrecord,1,stream->itcsink,0);
        ms_filter_link(stream->av_recorder.video_input,0,stream->av_recorder.recorder,0);
        ms_filter_call_method(stream->itcsink,MS_ITC_SINK_CONNECT,stream->av_recorder.video_input);
    }
    ms_connection_helper_link (&ch,stream->ms.decoder,0,0);
    
    if (stream->tee2){
        ms_connection_helper_link (&ch,stream->tee2,0,0);
        ms_filter_link(stream->tee2,1,stream->jpegwriter,0);
        ms_filter_link(stream->jpegwriter,0,stream->itcsinkforfilewriter,0);
        ms_filter_link(stream->itcsourceforfilewriter,0,stream->filewriter,0);
        ms_filter_call_method(stream->itcsinkforfilewriter,MS_ITC_SINK_CONNECT,stream->itcsourceforfilewriter);
    }
    ms_connection_helper_link (&ch,stream->output,0,-1);
    ipcam_stream_set_recorder_video_codec(stream, "H264");
    
    stream->tickerforIPCamStreaming = ms_ticker_new();
    ms_ticker_set_name(stream->tickerforIPCamStreaming,"IPCam Streaming MSTicker");
    stream->av_recorder.ticker=ms_ticker_new();
    ms_ticker_set_name(stream->av_recorder.ticker,"IPCam Recoder MSTicker");
    stream->tickerforfilewriter =  ms_ticker_new();
    ms_ticker_set_name(stream->tickerforfilewriter,"IPCam FileWriter MSTicker");
    
    ms_ticker_attach (stream->tickerforIPCamStreaming, stream->source);
    ms_ticker_attach (stream->av_recorder.ticker, stream->av_recorder.video_input);
    ms_ticker_attach (stream->tickerforfilewriter, stream->itcsourceforfilewriter);
    pthread_mutex_unlock(&ipcam_mutex);
    return stream;
}

void ipcam_streaming_stop(VideoStream *stream)
{
    MSConnectionHelper h;

    pthread_mutex_lock(&ipcam_mutex);
    ms_ticker_detach(stream->tickerforIPCamStreaming, stream->source);
    ms_ticker_detach(stream->av_recorder.ticker, stream->av_recorder.video_input);
    ms_ticker_detach(stream->tickerforfilewriter, stream->itcsourceforfilewriter);
    ms_connection_helper_start(&h);
    ms_connection_helper_unlink(&h,stream->source,-1,0);

    if (stream->teeforrecord) {
        ms_connection_helper_unlink(&h,stream->teeforrecord,0,0);
        ms_filter_unlink(stream->teeforrecord,1,stream->itcsink,0);
        ms_filter_unlink(stream->av_recorder.video_input,0,stream->av_recorder.recorder,0);
        ms_filter_call_method(stream->itcsink,MS_ITC_SINK_CONNECT,NULL);
    }
    ms_connection_helper_unlink(&h,stream->ms.decoder,0,0);

    if (stream->tee2){
        ms_connection_helper_unlink (&h,stream->tee2,0,0);
        ms_filter_unlink(stream->tee2,1,stream->jpegwriter,0);
        ms_filter_unlink(stream->jpegwriter,0,stream->itcsinkforfilewriter,0);
        ms_filter_unlink(stream->itcsourceforfilewriter,0,stream->filewriter,0);
        ms_filter_call_method(stream->itcsinkforfilewriter,MS_ITC_SINK_CONNECT, NULL);
    }    
    ms_connection_helper_unlink(&h,stream->output,0,-1);

    ms_ticker_destroy(stream->tickerforIPCamStreaming);
    ms_ticker_destroy(stream->av_recorder.ticker);
    ms_ticker_destroy(stream->tickerforfilewriter);
    
    ms_filter_destroy(stream->source);
    ms_filter_destroy(stream->teeforrecord);
    ms_filter_destroy(stream->itcsink);
    ms_filter_destroy(stream->av_recorder.video_input);
    ms_filter_destroy(stream->av_recorder.recorder);
    ms_filter_destroy(stream->ms.decoder);
    ms_filter_destroy(stream->tee2);
    ms_filter_destroy(stream->jpegwriter);
    ms_filter_destroy(stream->itcsinkforfilewriter);
    ms_filter_destroy(stream->itcsourceforfilewriter);
    ms_filter_destroy(stream->filewriter);
    ms_filter_destroy(stream->output);
    ms_free(stream);
    pthread_mutex_unlock(&ipcam_mutex);
}

void ipcam_stream_recorder_start(VideoStream *stream, const char *file)
{
    pthread_mutex_lock(&ipcam_mutex);
    if(file)
    {
        MSRecorderState state;
        ms_filter_call_method(stream->av_recorder.recorder, MS_RECORDER_GET_STATE, &state);
        if (state==MSRecorderClosed){
            if ((ms_filter_call_method(stream->av_recorder.recorder, MS_RECORDER_OPEN, (void*)file)) == -1)
            {
                pthread_mutex_unlock(&ipcam_mutex);
                return;
            }    
        }
        ms_filter_call_method_noarg(stream->av_recorder.recorder, MS_RECORDER_START);
    }
    pthread_mutex_unlock(&ipcam_mutex);
}

void ipcam_stream_recorder_stop(VideoStream *stream)
{
    MSRecorderState rstate;
    pthread_mutex_lock(&ipcam_mutex);
    if (ms_filter_call_method(stream->av_recorder.recorder,MS_RECORDER_GET_STATE,&rstate)==0){
        if (rstate!=MSRecorderClosed){
            ms_filter_call_method_noarg(stream->av_recorder.recorder, MS_RECORDER_CLOSE);
        }
    }
    pthread_mutex_unlock(&ipcam_mutex);
}    

void ipcam_stream_snapshot(VideoStream *stream, const char *file)
{
    pthread_mutex_lock(&ipcam_mutex);
    if(file)
    {
        if (stream->jpegwriter!=NULL){
            ms_filter_call_method(stream->jpegwriter,MS_JPEG_WRITER_TAKE_SNAPSHOT,(void*)file);
        }    
    }
    pthread_mutex_unlock(&ipcam_mutex);
}


