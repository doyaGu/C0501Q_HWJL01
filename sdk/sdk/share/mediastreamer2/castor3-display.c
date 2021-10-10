#ifdef HAVE_CONFIG_H
    #include "mediastreamer-config.h"
#endif

#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msvideo.h"

#include "layouts.h"
#include "ite/itu.h"

#ifndef CFG_FFMPEG_H264_SW
/* hack for RMI display */
    #include "ite/itp.h"
    #include "ite/itv.h"
    #include "libavformat/avformat.h"
    #include "isp/mmp_isp.h"
#endif // !CFG_FFMPEG_H264_SW

typedef struct Yuv2RgbCtx {
    uint8_t         *rgb;
    size_t          rgblen;
    MSVideoSize     dsize;
    MSVideoSize     ssize;
    MSScalerContext *sws;
} Yuv2RgbCtx;

static void yuv2rgb_init(Yuv2RgbCtx *ctx)
{
    ctx->rgb          = NULL;
    ctx->rgblen       = 0;
    ctx->dsize.width  = 0;
    ctx->dsize.height = 0;
    ctx->ssize.width  = 0;
    ctx->ssize.height = 0;
    ctx->sws          = NULL;
}

static void yuv2rgb_uninit(Yuv2RgbCtx *ctx)
{
    if (ctx->rgb)
    {
        ms_free(ctx->rgb);
        ctx->rgb    = NULL;
        ctx->rgblen = 0;
    }
    if (ctx->sws)
    {
        ms_scaler_context_free(ctx->sws);
        ctx->sws = NULL;
    }
    ctx->dsize.width  = 0;
    ctx->dsize.height = 0;
    ctx->ssize.width  = 0;
    ctx->ssize.height = 0;
}

static void yuv2rgb_prepare(Yuv2RgbCtx *ctx, MSVideoSize src, MSVideoSize dst)
{
    if (ctx->sws != NULL)
        yuv2rgb_uninit(ctx);
    ctx->sws    = ms_scaler_create_context(src.width, src.height, MS_YUV420P,
                                           dst.width, dst.height, MS_RGB565,
                                           MS_SCALER_METHOD_NEIGHBOUR);
    ctx->dsize  = dst;
    ctx->ssize  = src;
    ctx->rgblen = dst.width * dst.height * 2;
    ctx->rgb    = (uint8_t *)ms_malloc0(ctx->rgblen + dst.width);
}

/*
   this function resizes the original pictures to the destination size and converts to rgb.
   It takes care of reallocating a new SwsContext and rgb buffer if the source/destination sizes have
   changed.
 */
static void yuv2rgb_process(Yuv2RgbCtx *ctx, MSPicture *src, MSVideoSize dstsize)
{
    MSVideoSize srcsize;

    srcsize.width  = src->w;
    srcsize.height = src->h;
    if (!ms_video_size_equal(dstsize, ctx->dsize) || !ms_video_size_equal(srcsize, ctx->ssize))
    {
        yuv2rgb_prepare(ctx, srcsize, dstsize);
    }
    {
        int     rgb_stride = dstsize.width * 2;
        uint8_t *p;

        p = ctx->rgb;
        if (ms_scaler_process(ctx->sws, src->planes, src->strides, &p, &rgb_stride) < 0)
        {
            ms_error("Error in 420->rgb ms_scaler_process().");
        }
    }
}

static void yuv2rgb_draw(Yuv2RgbCtx *ctx, int dstx, int dsty)
{
    if (ctx->rgb)
    {
        ITUSurface *screenSurf = ituGetDisplaySurface();
        ITUSurface *srcSurf    = ituCreateSurface(ctx->dsize.width, ctx->dsize.height, ctx->dsize.width * 2, ITU_RGB565, ctx->rgb, ITU_STATIC);
        if (srcSurf)
            ituBitBlt(screenSurf, dstx, dsty, ctx->dsize.width, ctx->dsize.height, srcSurf, 0, 0);
        //ituFlip(screenSurf); // for test only
        ituDestroySurface(srcSurf);
    }
}

typedef struct _Castor3Display {
    ITUWidget   *window;
    MSVideoSize wsize; /*the initial requested window size*/
    MSVideoSize vsize; /*the video size received for main input*/
    Yuv2RgbCtx  mainview;
    bool_t      need_repaint;
} Castor3Display;

static void castor3_display_init(MSFilter  *f)
{
    Castor3Display *obj = (Castor3Display *)ms_new0(Castor3Display, 1);
    obj->window       = NULL;
    obj->wsize.width  = MS_VIDEO_SIZE_CIF_W;
    obj->wsize.height = MS_VIDEO_SIZE_CIF_H;
    obj->vsize.width  = MS_VIDEO_SIZE_CIF_W;
    obj->vsize.height = MS_VIDEO_SIZE_CIF_H;
    yuv2rgb_init(&obj->mainview);
    obj->need_repaint = FALSE;
    f->data           = obj;
}

static void castor3_display_prepare(MSFilter *f)
{
    Castor3Display *cd = (Castor3Display *)f->data;
}

static void castor3_display_unprepare(MSFilter *f)
{
    Castor3Display *cd = (Castor3Display *)f->data;
}

static void castor3_display_uninit(MSFilter *f)
{
    Castor3Display *obj = (Castor3Display *)f->data;
    castor3_display_unprepare(f);
    yuv2rgb_uninit(&obj->mainview);
    ms_free(obj);
}

static void castor3_display_preprocess(MSFilter *f)
{
    castor3_display_prepare(f);
}

static void castor3_display_process(MSFilter *f)
{
#ifdef CFG_FFMPEG_H264_SW
    Castor3Display *obj = (Castor3Display *)f->data;
    MSVideoSize    wsize; /* the window size*/
    MSVideoSize    vsize;
    MSRect         mainrect;
    MSPicture      mainpic;
    mblk_t         *main_im  = NULL;
    bool_t         repainted = FALSE;

    if (obj->window == NULL)
    {
        goto end;
    }

    wsize.width  = ituWidgetGetWidth(obj->window);
    wsize.height = ituWidgetGetHeight(obj->window);
    obj->wsize   = wsize;
    /*get most recent message and draw it*/
    if (f->inputs[0] != NULL && (main_im = ms_queue_peek_last(f->inputs[0])) != NULL)
    {
        if (ms_yuv_buf_init_from_mblk(&mainpic, main_im) == 0)
        {
            if ((obj->vsize.width != mainpic.w || obj->vsize.height != mainpic.h)
                && (mainpic.w > wsize.width || mainpic.h > wsize.height))
            {
                ms_message("Detected video resolution changed, resizing window");
                wsize.width       = mainpic.w;
                wsize.height      = mainpic.h;
                obj->need_repaint = TRUE;
            }
            obj->vsize.width  = mainpic.w;
            obj->vsize.height = mainpic.h;
        }
    }

    if (main_im != NULL || obj->need_repaint)
    {
        ms_layout_compute(wsize, obj->vsize, obj->vsize, -1, 0.0f, &mainrect, NULL);
        ituWidgetGetGlobalPosition(obj->window, &mainrect.x, &mainrect.y);
        vsize.width  = mainrect.w;
        vsize.height = mainrect.h;

        if (main_im != NULL)
            yuv2rgb_process(&obj->mainview, &mainpic, vsize);

        if (obj->need_repaint)
        {
            repainted         = TRUE;
            obj->need_repaint = FALSE;
        }
        if (main_im != NULL)
        {
            yuv2rgb_draw(&obj->mainview, mainrect.x, mainrect.y);
        }
    }
#else
    mblk_t            *im      = NULL;
    AVFrame           *picture = NULL;
    ITV_DBUF_PROPERTY dispProp = {0};

    while ((im = ms_queue_get(f->inputs[0])) != NULL)
    {
        uint8_t *dbuf = NULL;
        picture = im->b_rptr;
        dbuf    = itv_get_dbuf_anchor();
        if (dbuf != NULL)
        {
            uint32_t col, row, bytespp, pitch;
            int      bidx = im->reserved1;

            dispProp.src_w    = picture->width;
            dispProp.src_h    = picture->height;
            dispProp.ya       = picture->data[0];
            dispProp.ua       = picture->data[1];
            dispProp.va       = picture->data[2];
            dispProp.pitch_y  = picture->linesize[0];
            dispProp.pitch_uv = picture->linesize[1];
            dispProp.bidx     = bidx;
            dispProp.format   = MMP_ISP_IN_YUV420;

            itv_update_dbuf_anchor(&dispProp);
            freemsg(im);
            break;
        }
        else
        {
            freemsg(im);
            usleep(1000);
        }
    }
#endif // CFG_FFMPEG_H264_SW

end:

    if (f->inputs[0] != NULL)
        ms_queue_flush(f->inputs[0]);
    if (f->inputs[1] != NULL)
        ms_queue_flush(f->inputs[1]);
}

static int get_native_window_id(MSFilter *f, void *data)
{
    Castor3Display *obj = (Castor3Display *)f->data;
    *(long *)data = (long)obj->window;
    return 0;
}

static int set_native_window_id(MSFilter *f, void *data)
{
    Castor3Display *obj = (Castor3Display *)f->data;
    obj->window = (ITUWidget *)(*(long *)data);
    return 0;
}

static int get_vsize(MSFilter *f, void *data)
{
    Castor3Display *obj = (Castor3Display *)f->data;
    *(MSVideoSize *)data = obj->wsize;
    return 0;
}

static int set_vsize(MSFilter *f, void *data)
{
    Castor3Display *obj = (Castor3Display *)f->data;
    obj->wsize = *(MSVideoSize *)data;
    return 0;
}

static MSFilterMethod methods[] = {
    {   MS_FILTER_GET_VIDEO_SIZE, get_vsize },
    {   MS_FILTER_SET_VIDEO_SIZE, set_vsize },
    {   MS_VIDEO_DISPLAY_GET_NATIVE_WINDOW_ID, get_native_window_id },
    {   MS_VIDEO_DISPLAY_SET_NATIVE_WINDOW_ID, set_native_window_id },
    {   0, NULL}
};

#ifdef _MSC_VER

MSFilterDesc ms_castor3_display_desc = {
    MS_CASTOR3_DISPLAY_ID,
    "MSCastor3Display",
    N_("A video display based on Castor3 platform"),
    MS_FILTER_OTHER,
    NULL,
    2,
    0,
    castor3_display_init,
    castor3_display_preprocess,
    castor3_display_process,
    NULL,
    castor3_display_uninit,
    methods
};

#else

MSFilterDesc ms_castor3_display_desc = {
    .id         = MS_CASTOR3_DISPLAY_ID,
    .name       = "MSCastor3Display",
    .text       = N_("A video display based on Castor3 api"),
    .category   = MS_FILTER_OTHER,
    .ninputs    = 2,
    .noutputs   = 0,
    .init       = castor3_display_init,
    .preprocess = castor3_display_preprocess,
    .process    = castor3_display_process,
    .uninit     = castor3_display_uninit,
    .methods    = methods
};

#endif

MS_FILTER_DESC_EXPORT(ms_castor3_display_desc)