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

#include "mediastreamer2/mscommon.h"
#include "mediastreamer2/msvideo.h"
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
#include "mediastreamer2/mswebcam.h"


typedef struct _Castor3CamData{
	MSVideoSize vsize;
	MSPicture pict;
	int index;
	uint64_t starttime;
	float fps;
	mblk_t *pic;
}Castor3CamData;

static void castor3cam_init(MSFilter *f){
	Castor3CamData *d=(Castor3CamData*)ms_new(Castor3CamData,1);
	d->vsize.width=MS_VIDEO_SIZE_CIF_W;
	d->vsize.height=MS_VIDEO_SIZE_CIF_H;
	d->fps=15;
	d->index=0;
	d->starttime=0;
	d->pic=NULL;
	f->data=d;
}

static void castor3cam_uninit(MSFilter *f){
	ms_free(f->data);
}

static void castor3cam_preprocess(MSFilter *f){
	Castor3CamData *d=(Castor3CamData*)f->data;
	d->pic=ms_yuv_buf_alloc(&d->pict,d->vsize.width,d->vsize.height);
	memset(d->pic->b_rptr,0,d->pic->b_wptr-d->pic->b_rptr);
	d->starttime=f->ticker->time;
}

static void plane_draw(uint8_t *p, int w, int h, int lsz, int index){
	int i,j;
	for(i=0;i<h;++i){
		for(j=0;j<w;++j){
			p[j]= (( (i/50)*50 + (j/50)*50 + index ) & 0x1)*200;
		}
		p+=lsz;
	}
}

static void castor3cam_draw(Castor3CamData *d){
	plane_draw(d->pict.planes[0],d->pict.w,d->pict.h,d->pict.strides[0],d->index*2);
	plane_draw(d->pict.planes[1],d->pict.w/2,d->pict.h/2,d->pict.strides[1],d->index);
	plane_draw(d->pict.planes[2],d->pict.w/2,d->pict.h/2,d->pict.strides[2],d->index);
}

static void castor3cam_process(MSFilter *f){
	Castor3CamData *d=(Castor3CamData*)f->data;
	float elapsed=(float)(f->ticker->time-d->starttime);
	if ((elapsed*d->fps/1000.0)>d->index){
		castor3cam_draw(d);
		ms_queue_put(f->outputs[0],dupb(d->pic));
		d->index++;
	}
}

static void castor3cam_postprocess(MSFilter *f){
	Castor3CamData *d=(Castor3CamData*)f->data;
	if (d->pic) {
		freemsg(d->pic);
		d->pic=NULL;
	}
}

static int castor3cam_set_vsize(MSFilter *f, void* data){
	Castor3CamData *d=(Castor3CamData*)f->data;
	d->vsize=*(MSVideoSize*)data;
	return 0;
}

static int castor3cam_get_vsize(MSFilter *f, void* data){
	Castor3CamData *d=(Castor3CamData*)f->data;
	*(MSVideoSize*)data = d->vsize;
	return 0;
}

static int castor3cam_set_fps(MSFilter *f, void* data){
	Castor3CamData *d=(Castor3CamData*)f->data;
	d->fps=*(float*)data;
	return 0;
}

static int castor3cam_get_fmt(MSFilter *f, void* data){
	*(MSPixFmt*)data=MS_YUV420P;
	return 0;
}

MSFilterMethod castor3cam_methods[]={
	{	MS_FILTER_SET_VIDEO_SIZE, castor3cam_set_vsize },
    {	MS_FILTER_GET_VIDEO_SIZE, castor3cam_get_vsize },
	{	MS_FILTER_SET_FPS	, castor3cam_set_fps	},
	{	MS_FILTER_GET_PIX_FMT	, castor3cam_get_fmt	},
	{	0,0 }
};

MSFilterDesc ms_castor3cam_desc={
	MS_CASTOR3_CAM_ID,
	"MSCastor3Cam",
	"A filter that outputs camera streaming",
	MS_FILTER_OTHER,
	NULL,
	0,
	1,
	castor3cam_init,
	castor3cam_preprocess,
	castor3cam_process,
	castor3cam_postprocess,
	castor3cam_uninit,
	castor3cam_methods
};

MS_FILTER_DESC_EXPORT(ms_castor3cam_desc)

static void castor3cam_detect(MSWebCamManager *obj);

static void castor3cam_cam_init(MSWebCam *cam){
	cam->name=ms_strdup("Castor3Cam");
}


static MSFilter *castor3cam_create_reader(MSWebCam *obj){
	return ms_filter_new_from_desc(&ms_castor3cam_desc);
}

MSWebCamDesc castor3cam_desc={
	"Castor3Cam",
	&castor3cam_detect,
	&castor3cam_cam_init,
	&castor3cam_create_reader,
	NULL
};

static void castor3cam_detect(MSWebCamManager *obj){
	MSWebCam *cam=ms_web_cam_new(&castor3cam_desc);
	ms_web_cam_manager_add_cam(obj,cam);
}

