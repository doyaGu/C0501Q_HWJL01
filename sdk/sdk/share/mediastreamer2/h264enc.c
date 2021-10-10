#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/rfc3984.h"
#include "mediastreamer2/msvideo.h"
#include "mediastreamer2/msticker.h"

#include "ffmpeg-priv.h"

#include "ortp/b64.h"

/* iTE related */
#include "ite/itp.h"
#include "ite/ith_video.h"
#include "ite/itv.h"
#include "isp/mmp_isp.h"
#include "fc_external.h"

/* ffmpeg */
#include "libavutil/avstring.h"
#include "libavutil/colorspace.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "libavutil/parseutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/avassert.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libavcodec/audioconvert.h"
#include "libavutil/opt.h"
#include "libavcodec/avfft.h"
#include "libswresample/swresample.h"

typedef struct _H264EncData {
	MSVideoSize vsize;
	float fps;
} H264EncData;

static void h264_enc_init(MSFilter *f)
{
	H264EncData *d = (H264EncData*)ms_new(H264EncData, 1);
	f->data = d;

	/* TODO: add for configure_video_source */
    d->vsize.width  = 852;
    d->vsize.height = 480;
	d->fps		    = 25.0;
}

static void h264_enc_process(MSFilter *f)
{
}

static void h264_enc_uninit(MSFilter *f)
{
	H264EncData *d=(H264EncData*)f->data;
	ms_free(d);
}

static int h264_enc_set_vsize(MSFilter *f, void* data)
{
	H264EncData *d = (H264EncData*)f->data;
	d->vsize = *(MSVideoSize*)data;
	return 0;
}

static int h264_enc_get_vsize(MSFilter *f, void* data)
{
	H264EncData *d = (H264EncData*)f->data;
	*(MSVideoSize*)data = d->vsize;
	return 0;
}

static int h264_enc_set_fps(MSFilter *f, void* data)
{
	H264EncData *d = (H264EncData*)f->data;
	d->fps = *(float*)data;
	return 0;
}

static int h264_enc_get_fps(MSFilter *f, void* data)
{
	H264EncData *d = (H264EncData*)f->data;
	*(float*)data = d->fps;
	return 0;
}

static MSFilterMethod h264_enc_methods[]={
	{	MS_FILTER_SET_FPS	,	h264_enc_set_fps	},
	{	MS_FILTER_GET_FPS	,	h264_enc_get_fps	},
	{	MS_FILTER_SET_VIDEO_SIZE ,	h264_enc_set_vsize },
	{	MS_FILTER_GET_VIDEO_SIZE ,	h264_enc_get_vsize },
//	{	MS_FILTER_ADD_FMTP	,	enc_add_fmtp },
//	{	MS_FILTER_SET_BITRATE	,	enc_set_br	},
//	{	MS_FILTER_GET_BITRATE	,	enc_get_br	},
//	{	MS_FILTER_SET_MTU	,	enc_set_mtu	},
//	{	MS_FILTER_REQ_VFU	,	enc_req_vfu	},
	{	0			,	NULL	}
};

#if defined(_MSC_VER)
MSFilterDesc ms_h264_enc_desc = {
	MS_H264_ENC_ID,
	"MSH264Enc",
	"A Fake H264 Encoder",
	MS_FILTER_ENCODER,
	"H264",
	1,
	1,
	h264_enc_init,
	NULL,
	h264_enc_process,
	NULL,
	h264_enc_uninit,
	h264_enc_methods
};
#else
MSFilterDesc ms_h264_enc_desc = {
	.id		= MS_H264_ENC_ID,
	.name		= "MSH264Enc",
	.text		= "A Fake H264 Encoder",
	.category	= MS_FILTER_ENCODER,
	.enc_fmt	= "H264",
	.ninputs	= 1,
	.noutputs	= 1,
	.init		= h264_enc_init,
	.preprocess	= NULL,
	.process	= h264_enc_process,
	.postprocess	= NULL,
	.uninit		= h264_enc_uninit,
	.methods	= h264_enc_methods
};
#endif
