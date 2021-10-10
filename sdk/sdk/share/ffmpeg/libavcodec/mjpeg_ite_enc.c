
#include <stdlib.h>
#include "libavutil/log.h"
#include "avcodec.h"

#include "mjpeg_ite_enc.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
typedef struct _MJPEG_ITE_ENC_CTXT_TAG
{
    AVCodecContext      *avctx;
    AVFrame             picture; /* picture structure */
    AVFrame             *picture_ptr; /* pointer to picture structure */
    int                 width;
    int                 height;

    // ite H/W jpg
    HJPG                *pHJpeg;
    JPG_COLOR_SPACE     encColorFmt;
    int                 quality;

}MJPEG_ITE_ENC_CTXT;

//=============================================================================
//				  Global Data Definition
//=============================================================================
#define ITE_MJPG_BPP_STEP 19
static double jBPP[ITE_MJPG_BPP_STEP] = 
{
    0.231679159, 0.338165601, 0.431156489, 0.511465269, 0.586883405,
    0.657421783, 0.725403807, 0.782155396, 0.845730682, 0.901791915,
    0.960211365, 1.032247529, 1.122407683, 1.233239363, 1.361397386, 
    1.561601677, 1.842349216, 2.33261124, 3.386962311,
};

//=============================================================================
//				  Private Function Declaration
//=============================================================================


//=============================================================================
//				  Private Function Definition
//=============================================================================
static av_cold int 
ite_mjpeg_encode_init(
    AVCodecContext  *avctx)
{
    int                 result = 0;
    JPG_ERR             jpgRst = JPG_ERR_OK;
    MJPEG_ITE_ENC_CTXT  *mjEncCtxt = avctx->priv_data;

    do{
        JPG_INIT_PARAM      initParam = {0}; 
        JPG_USER_INFO       jpgUserInfo = {0};
        int                 i, byte_rate = 0;

        if( avctx == NULL )
        {
            printf(" AVCodecContext NULL pointer! %s [%d]\n", __FILE__, __LINE__);
            result = -1;
            break;
        }

        if(!mjEncCtxt->picture_ptr)
            mjEncCtxt->picture_ptr = &mjEncCtxt->picture;

        avcodec_get_frame_defaults(&mjEncCtxt->picture);

        mjEncCtxt->width  = avctx->width;
        mjEncCtxt->height = avctx->height;
        mjEncCtxt->avctx  = avctx;

        switch( avctx->pix_fmt )
        {
            case PIX_FMT_YUVJ420P:  mjEncCtxt->encColorFmt = JPG_COLOR_SPACE_YUV420;     break;
            case PIX_FMT_YUVJ422P:  mjEncCtxt->encColorFmt = JPG_COLOR_SPACE_YUV422;     break;
        }
        
        initParam.codecType     = JPG_CODEC_ENC_MJPG;
        initParam.outColorSpace = mjEncCtxt->encColorFmt;
        initParam.width         = mjEncCtxt->width;
        initParam.height        = mjEncCtxt->height;

        // To Do: bitrate transfor to quality
        if( avctx->time_base.den == 0 )
        {
            printf(" Need to set frame rate !! %s [%d]\n", __FILE__, __LINE__);
            result = -1;
            break;
        }
        
        avctx->max_b_frames = 0;
        byte_rate = (avctx->bit_rate * avctx->time_base.num) / avctx->time_base.den;
        
        for(i = 0; i < ITE_MJPG_BPP_STEP; i++)
        {
            if( byte_rate < (jBPP[i] * mjEncCtxt->width * mjEncCtxt->height) )
                break;
        }
        mjEncCtxt->quality = 5 * (i - 1);
        mjEncCtxt->quality = (mjEncCtxt->quality < 5) ? 5 : mjEncCtxt->quality;
        mjEncCtxt->quality = (mjEncCtxt->quality > 95) ? 95 : mjEncCtxt->quality;

        initParam.encQuality = mjEncCtxt->quality;        

        iteJpg_CreateHandle(&mjEncCtxt->pHJpeg, &initParam, 0);

        // set DHT and DQT and generate jHeader
        jpgRst = iteJpg_Setup(mjEncCtxt->pHJpeg, 0);
        if( jpgRst != JPG_ERR_OK )
        {
            printf(" ite_mjpeg setup err (0x%x) ! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            result = -1;
        }

    }while(0);

    return result;
}

static int 
ite_mjpeg_encode_picture(
    AVCodecContext  *avctx,
    uint8_t         *buf, 
    int             buf_size,
    void            *data)
{
    int                 realBsSize = 0;
    JPG_ERR             jpgRst = JPG_ERR_OK;
    AVFrame             *pic = data;

    do{
        MJPEG_ITE_ENC_CTXT  *mjEncCtxt = avctx->priv_data;
        JPG_BUF_INFO        entropyBufInfo = {0};        
        JPG_STREAM_INFO     inStreamInfo = {0};
        JPG_STREAM_INFO     outStreamInfo = {0};

        if( mjEncCtxt == 0 ||
            pic->data[0] == 0 || pic->data[1] == 0 || pic->data[2] == 0 || 
            pic->linesize[0] == 0 || pic->linesize[1] == 0 )
        {
            printf(" wrong parameters 0x%x, %d, %d, %d, %d, %d in %s [%d]\n", mjEncCtxt, pic->data[0], pic->data[1], pic->data[2],
                    pic->linesize[0], pic->linesize[1], __FILE__, __LINE__);
            jpgRst = JPG_ERR_INVALID_PARAMETER;
            break;
        }

        // set input and output info 
        inStreamInfo.streamIOType         = JPG_STREAM_IO_READ;
        inStreamInfo.streamType           = JPG_STREAM_MEM;
        // Y
        inStreamInfo.jstream.mem[0].pAddr = (uint8_t*)pic->data[0];
        inStreamInfo.jstream.mem[0].pitch = pic->linesize[0];
        //inStreamInfo.src.mem[0].length = _Get_Lcd_Width() * _Get_Lcd_Height();
        // U
        inStreamInfo.jstream.mem[1].pAddr = (uint8_t*)pic->data[1];
        inStreamInfo.jstream.mem[1].pitch = pic->linesize[1];
        //inStreamInfo.src.mem[1].length = (_Get_Lcd_Width()/2) * _Get_Lcd_Height();
        // V
        inStreamInfo.jstream.mem[2].pAddr = (uint8_t*)pic->data[2];
        inStreamInfo.jstream.mem[2].pitch = pic->linesize[2];
        //inStreamInfo.src.mem[2].length = (_Get_Lcd_Width()/2) * _Get_Lcd_Height();

        inStreamInfo.validCompCnt = 3;

        outStreamInfo.streamIOType          = JPG_STREAM_IO_WRITE;
        outStreamInfo.streamType            = JPG_STREAM_MEM;
        outStreamInfo.jstream.mem[0].pAddr  = (uint8_t*)buf; 
        outStreamInfo.jstream.mem[0].pitch  = buf_size;
        outStreamInfo.jstream.mem[0].length = buf_size;
        outStreamInfo.validCompCnt = 1;

        jpgRst = iteJpg_SetStreamInfo(mjEncCtxt->pHJpeg, &inStreamInfo, 0, 0);
        if( jpgRst != JPG_ERR_OK )
        {
            printf(" err (0x%x) ! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            break;
        }

        // setup 
        jpgRst = iteJpg_Setup(mjEncCtxt->pHJpeg, 0);
        if( jpgRst != JPG_ERR_OK )
        {
            printf(" err (0x%x) ! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            break;
        }
        
        // fire
        jpgRst = iteJpg_Process(mjEncCtxt->pHJpeg, &entropyBufInfo, (uint32_t*)&realBsSize, 0);
        if( jpgRst != JPG_ERR_OK )
        {
            printf(" err (0x%x) ! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            break;
        }        
    }while(0);
    
    if( jpgRst != JPG_ERR_OK )      realBsSize = 0;

    return realBsSize;
}

static av_cold int 
ite_mjpeg_encode_end(
    AVCodecContext  *avctx)
{
    MJPEG_ITE_ENC_CTXT  *mjEncCtxt = avctx->priv_data;

    // need to release buf ??

    iteJpg_DestroyHandle(&mjEncCtxt->pHJpeg, 0);

    return 0;
}
//=============================================================================
//				  Public Function Definition
//=============================================================================
static enum PixelFormat inSrc_fmts[3] = {PIX_FMT_YUVJ420P, PIX_FMT_YUVJ422P, PIX_FMT_NONE};

AVCodec ff_ite_mjpeg_encoder = {
    "ite mjpeg",
    AVMEDIA_TYPE_VIDEO,
    CODEC_ID_MJPEG, // CODEC_ID_ITE_MJPEG
    sizeof(MJPEG_ITE_ENC_CTXT),
    ite_mjpeg_encode_init,
    ite_mjpeg_encode_picture,
    ite_mjpeg_encode_end,
    NULL,
    0,
    NULL,
    NULL,
    NULL,
    inSrc_fmts,
    "MJPEG (Motion JPEG)",
};
