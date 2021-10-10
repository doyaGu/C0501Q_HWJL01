#ifdef HAVE_CONFIG_H
#include "mediastreamer-config.h"
#endif

#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
#include "private.h"

#if defined(CFG_SENSOR_ENABLE) && defined(CFG_XCPU_MSGQ)
#include "xcpu_master/itx.h"
#include "ssp/mmp_spi.h"
#endif

VideoStream * cam_playback_start()
{
    VideoStream *stream;
    MSConnectionHelper ch;

#if defined(CFG_SENSOR_ENABLE) && defined(CFG_XCPU_MSGQ)
    SENSOR_IMAGE_MIRROR pCtrl ={false, true};
    mmpSpiSetControlMode(SPI_CONTROL_SLAVE);
    itxCamLedon();
    itxSetSensorImageMirror(pCtrl);
    mmpSpiResetControl();
#endif 

    stream=(VideoStream *)ms_new0(VideoStream,1);
    stream->cam = ms_web_cam_manager_get_default_cam( ms_web_cam_manager_get() );
    stream->source = ms_web_cam_create_reader(stream->cam);
    stream->ms.decoder = ms_filter_create_decoder("H264");
    stream->output=ms_filter_new_from_name ("MSCastor3Display");

    ms_connection_helper_start (&ch);
    ms_connection_helper_link (&ch,stream->source,-1,0);
    ms_connection_helper_link (&ch,stream->ms.decoder,0,0);
    ms_connection_helper_link (&ch,stream->output,0,-1);

    stream->tickerforCamPlayback = ms_ticker_new();
    ms_ticker_set_name(stream->tickerforCamPlayback,"Camera Playback MSTicker");

    ms_ticker_attach (stream->tickerforCamPlayback, stream->source);
    
    return stream;
}


void cam_playback_stop(VideoStream *stream)
{
    MSConnectionHelper h;

#if defined(CFG_SENSOR_ENABLE) && defined(CFG_XCPU_MSGQ)
    SENSOR_IMAGE_MIRROR pCtrl ={true, true};
    mmpSpiSetControlMode(SPI_CONTROL_SLAVE);
    itxSetSensorImageMirror(pCtrl);
    mmpSpiResetControl();
#endif

    ms_ticker_detach(stream->tickerforCamPlayback, stream->source);
    ms_connection_helper_start(&h);
    ms_connection_helper_unlink(&h,stream->source,-1,0);
    ms_connection_helper_unlink(&h,stream->ms.decoder,0,0);
    ms_connection_helper_unlink(&h,stream->output,0,-1);

    ms_ticker_destroy(stream->tickerforCamPlayback);

    ms_filter_destroy(stream->source);
    ms_filter_destroy(stream->ms.decoder);
    ms_filter_destroy(stream->output);
    ms_free(stream);
}










