/*
   mediastreamer2 library - modular sound and video processing and streaming
   Copyright (C) 2010  Belledonne Communications SARL <simon.morlat@linphone.org>

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

#include "mediastreamer2/msjpegwriter.h"
#include "mediastreamer2/msfilewriter.h"
#include "mediastreamer2/msvideo.h"
#include "ffmpeg-priv.h"

//for H264 data //Benson
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/rfc3984.h"
#include "mediastreamer2/msticker.h"
//for H264 data. //Benson

#ifdef WIN32
    #include <malloc.h>
#endif
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include "castor3player.h"
#include "../../include/jpg/ite_jpg.h"

#include "stdio.h"
#include "ite/itp.h"
#include "fat/fat.h"
#include "ite/ite_sd.h"


#define DEF_BitStream_BUF_LENGTH           (256 << 10)

#ifndef FALSE
    #define FALSE 0
#endif
#ifndef TRUE
    #define TRUE  1
#endif

typedef struct {
    ms_mutex_t  mutex;
    FILE        *file;
    AVCodec     *codec;
    int         Runstate;
    char        filepath[PATH_MAX];
} JpegWriter;

static void jpg_init(MSFilter *f)
{
    JpegWriter *s = ms_new0(JpegWriter, 1);
#ifdef ENABLE_GENERAL_PLAYER	
    s->codec      = avcodec_find_encoder(CODEC_ID_MJPEG);
#endif
    printf("jpg_init\n");

    if (s->codec == NULL)
    {
        printf("Could not find CODEC_ID_MJPEG !\n");
    }
    ms_mutex_init(&s->mutex,NULL); //Benson
    f->data = s;
}

static void jpg_uninit(MSFilter *f)
{
    JpegWriter *s = (JpegWriter *)f->data;
    printf("jpg_uninit\n");
    if (s->file != NULL)
    {
        fclose(s->file);
    }
    ms_mutex_destroy(&s->mutex); 
    ms_free(s);
    f->data = NULL; 
}

static void cleanup(JpegWriter *s, AVCodecContext *avctx)
{
    if (s->file)
    {
        fclose(s->file);
        s->file = NULL;
    }
    if (avctx)
    {
        avcodec_close(avctx);
        av_free(avctx);
    }
}

#ifndef _MSC_VER
static void jpg_process(MSFilter *f, void *arg)
{
    JpegWriter      *s             = (JpegWriter *)f->data;
    char            *filename      = s->filepath;//(char *)arg;
    mblk_t          *im            = NULL;
    AVFrame         *picture       = NULL;
    HJPG            *pHJpeg        = 0;
    JPG_INIT_PARAM  initParam      = {0};
    JPG_STREAM_INFO inStreamInfo   = {0};
    JPG_STREAM_INFO outStreamInfo  = {0};
    JPG_BUF_INFO    entropyBufInfo = {0};
    JPG_USER_INFO   jpgUserInfo    = {0};
    uint8_t         *ya_out        = 0, *ua_out = 0, *va_out = 0; // address of YUV decoded video buffer
    uint8_t         *pSaveBuf      = 0; 
    uint32_t        src_w_out      = 0, src_h_out = 0;
    uint32_t        jpgEncSize     = 0;
    uint32_t        H264_pitch_y   = 2048; //because the input format form H264 tilemode, so the pitch is 2048
    uint32_t        H264_pitch_uv  = 2048;
    
    ms_mutex_lock(&s->mutex);
    if ((im = ms_queue_get(f->inputs[0])) != NULL)
    {
        picture       = im->b_rptr;
        src_w_out     = picture->width;
        src_h_out     = picture->height;
        ya_out        = picture->data[0];
        ua_out        = picture->data[1];
        va_out        = picture->data[2];
        H264_pitch_y  = picture->linesize[0];
        H264_pitch_uv = picture->linesize[1];
    }
    else
    {
        ms_mutex_unlock(&s->mutex);
        return;
    }


    if (s->Runstate == TRUE)
    {
        mblk_t *CompressedData = NULL;

        if (!ms_queue_empty(f->outputs[0]))
        {
             if (im) freemsg(im);
             ms_mutex_unlock(&s->mutex);
             return;
        }
        
        // ------------------------------------------------------
        // encode
        initParam.codecType = JPG_CODEC_ENC_JPG;
        initParam.outColorSpace = JPG_COLOR_SPACE_YUV420;
        initParam.width         = src_w_out;
        initParam.height        = src_h_out;
        initParam.encQuality    = 70;//85;

        iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

        inStreamInfo.streamIOType         = JPG_STREAM_IO_READ;
        inStreamInfo.streamType           = JPG_STREAM_MEM;
        // Y
        inStreamInfo.jstream.mem[0].pAddr = (uint8_t *)ya_out; //YUV_Save;
        inStreamInfo.jstream.mem[0].pitch = H264_pitch_y;      // src_w_out;

        // U
        inStreamInfo.jstream.mem[1].pAddr = (uint8_t *)ua_out; //(inStreamInfo.jstream.mem[0].pAddr+H264_pitch_y*src_h_out);
        inStreamInfo.jstream.mem[1].pitch = H264_pitch_uv;     //src_w_out/2;

        // V
        inStreamInfo.jstream.mem[2].pAddr = (uint8_t *)va_out; //(inStreamInfo.jstream.mem[1].pAddr+H264_pitch_y*src_h_out);
        inStreamInfo.jstream.mem[2].pitch = H264_pitch_uv;     //src_w_out/2;

        inStreamInfo.validCompCnt         = 3;

#if 0
        if (filename)
        {
            outStreamInfo.streamType   = JPG_STREAM_FILE;
            outStreamInfo.jstream.path = (void *)filename;
        }
        outStreamInfo.streamIOType          = JPG_STREAM_IO_WRITE;
        outStreamInfo.jpg_reset_stream_info = 0; //  _reset_stream_info;
#else
        CompressedData = allocb(DEF_BitStream_BUF_LENGTH+DEF_FileStream_Name_LENGTH,0);
        if (CompressedData == NULL)
        {
            freemsg(im);
            ms_mutex_unlock(&s->mutex);
            return;
        }
        
        strcpy(CompressedData->b_wptr, s->filepath);
        CompressedData->b_wptr += DEF_FileStream_Name_LENGTH;      
           
        outStreamInfo.streamIOType       = JPG_STREAM_IO_WRITE;
        outStreamInfo.streamType         = JPG_STREAM_MEM;
        outStreamInfo.jpg_reset_stream_info =  0; //  _reset_stream_info;      
        outStreamInfo.jstream.mem[0].pAddr  = CompressedData->b_wptr; 
        outStreamInfo.jstream.mem[0].pitch  = H264_pitch_y;
        outStreamInfo.jstream.mem[0].length = DEF_BitStream_BUF_LENGTH;
        outStreamInfo.validCompCnt = 1;   
        
#endif

        printf("\n\n\tencode input: Y=0x%x, u=0x%x, v=0x%x\n",
               inStreamInfo.jstream.mem[0].pAddr,
               inStreamInfo.jstream.mem[1].pAddr,
               inStreamInfo.jstream.mem[2].pAddr);

        iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, &outStreamInfo, 0);
        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);

        iteJpg_Setup(pHJpeg, 0);

        iteJpg_Process(pHJpeg, &entropyBufInfo, &jpgEncSize, 0);

        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
        printf("\n\tresult = %d, encode size = %f KB\n", jpgUserInfo.status, (float)jpgEncSize / 1024);

        CompressedData->b_wptr += jpgEncSize;
        ms_queue_put(f->outputs[0], CompressedData);
        iteJpg_DestroyHandle(&pHJpeg, 0);

        if (im)
            freemsg(im);
        s->Runstate = FALSE;
        ms_mutex_unlock(&s->mutex);
        return;
    }
    else
    {
        if (im)
            freemsg(im);
        ms_mutex_unlock(&s->mutex);
        return;
    }
}
#else
static void jpg_process(MSFilter *f, void *arg)
{
#if 0
    JpegWriter *s = (JpegWriter *)f->data;
    if (s->file != NULL && s->codec != NULL)
    {
        MSPicture yuvbuf, yuvjpeg;
        mblk_t    *m = ms_queue_peek_last(f->inputs[0]);
        if (ms_yuv_buf_init_from_mblk(&yuvbuf, m) == 0)
        {
            int               error;
            int               comp_buf_sz = msgdsize(m);
            uint8_t           *comp_buf   = (uint8_t *)alloca(comp_buf_sz);
            AVFrame           pict;
            mblk_t            *jpegm;
            struct SwsContext *sws_ctx;

            AVCodecContext    *avctx = avcodec_alloc_context();

            avctx->width         = yuvbuf.w;
            avctx->height        = yuvbuf.h;
            avctx->time_base.num = 1;
            avctx->time_base.den = 1;
            avctx->pix_fmt       = PIX_FMT_YUVJ420P;

            error                = avcodec_open(avctx, s->codec);
            if (error != 0)
            {
                ms_error("avcodec_open() failed: %i", error);
                cleanup(s, NULL);
                av_free(avctx);
                return;
            }
            sws_ctx = sws_getContext(avctx->width, avctx->height, PIX_FMT_YUV420P,
                                     avctx->width, avctx->height, avctx->pix_fmt, SWS_FAST_BILINEAR, NULL, NULL, NULL);
            if (sws_ctx == NULL)
            {
                ms_error(" sws_getContext() failed.");
                cleanup(s, avctx);
                goto end;
            }
            jpegm = ms_yuv_buf_alloc (&yuvjpeg, avctx->width, avctx->height);
            if (sws_scale(sws_ctx, (const uint8_t *const *)yuvbuf.planes, yuvbuf.strides, 0, avctx->height, yuvjpeg.planes, yuvjpeg.strides) < 0)
            {
                ms_error("sws_scale() failed.");
                sws_freeContext(sws_ctx);
                cleanup(s, avctx);
                freemsg(jpegm);
                goto end;
            }
            sws_freeContext(sws_ctx);

            avcodec_get_frame_defaults(&pict);
            avpicture_fill((AVPicture *)&pict, (uint8_t *)jpegm->b_rptr, avctx->pix_fmt, avctx->width, avctx->height);
            error = avcodec_encode_video(avctx, (uint8_t *)comp_buf, comp_buf_sz, &pict);
            if (error < 0)
            {
                ms_error("Could not encode jpeg picture.");
            }
            else
            {
                fwrite(comp_buf, error, 1, s->file);
                ms_message("Snapshot done");
            }
            cleanup(s, avctx);
            freemsg(jpegm);
        }
        goto end;
    }
end:
    ms_queue_flush(f->inputs[0]);
#endif    
}
#endif

static int take_snapshot(MSFilter *f, void *arg)
{
#if 1//def CFG_SD0_ENABLE
    JpegWriter *s = (JpegWriter *)f->data;  // it can using  ms_error to debug  -> ms_error("take_snapshot.");

    ms_mutex_lock(&s->mutex);
    s->Runstate = TRUE;
    strcpy(s->filepath, (char*)arg);
    ms_mutex_unlock(&s->mutex);
#endif
    return 1;
}
static void jpg_preprocess(MSFilter *f)
{
    JpegWriter *s                   = (JpegWriter *)f->data;
}

static MSFilterMethod jpg_methods[] = {
    {       MS_JPEG_WRITER_TAKE_SNAPSHOT, take_snapshot },
    {       0, NULL}
};

#ifndef _MSC_VER

MSFilterDesc ms_jpeg_writer_desc = {
    .id       = MS_JPEG_WRITER_ID,
    .name     = "MSJpegWriter",
    .text     = "Take a video snapshot as jpg file",
    .category = MS_FILTER_ENCODER,
    .enc_fmt  = "Jpeg",
    .ninputs  = 1,
    .noutputs = 1,
    .init     = jpg_init,
    .process  = jpg_process,
    .uninit   = jpg_uninit,
    .methods  = jpg_methods
};
#else

MSFilterDesc ms_jpeg_writer_desc = {
    MS_JPEG_WRITER_ID,
    "MSJpegWriter",
    "Take a video snapshot as jpg file",
    MS_FILTER_ENCODER,
    "Jpeg",
    1,
    1,
    jpg_init,
    NULL,
    jpg_process,
    NULL,
    jpg_uninit,
    jpg_methods
};
#endif

MS_FILTER_DESC_EXPORT(ms_jpeg_writer_desc)
