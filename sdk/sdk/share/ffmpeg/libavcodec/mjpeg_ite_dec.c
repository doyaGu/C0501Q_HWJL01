#include <stdlib.h>
#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "avcodec.h"

#include "mjpeg_ite_dec.h"
#include "jpg/ite_jpg.h"
#include "isp/mmp_isp.h"

#if (CFG_CHIP_FAMILY == 9920)
#include "../driver/jpg/it9920/jpg_reg.h"
#elif (CFG_CHIP_FAMILY == 9850)
#include "../driver/jpg/it9850/jpg_reg.h"
#else
#include "../driver/jpg/it9070/jpg_reg.h"
#endif

//=============================================================================
//				  Constant Definition
//=============================================================================
typedef struct _MJPEG_ITE_DEC_CTXT_TAG
{
    AVCodecContext *avctx;
    AVFrame        picture;      /* picture structure */
    AVFrame        *picture_ptr; /* pointer to picture structure */
    int            width;
    int            height;

    // ite H/W jpg
    HJPG           *pHJpeg;
} MJPEG_ITE_DEC_CTXT;

typedef struct JPG_DECODER_TAG
{
    uint32_t	framePitchY;
    uint32_t	framePitchUV;
    uint32_t	frameWidth;
    uint32_t	frameHeight;
    uint32_t	frameBufCount;
	uint32_t    currDisplayFrameBufIndex;
    uint32_t	OutAddrY[2];
    uint32_t	OutAddrU[2];
    uint32_t	OutAddrV[2];
	uint8_t     *DisplayAddrY;
    uint8_t     *DisplayAddrU;
    uint8_t     *DisplayAddrV;
} JPG_DECODER;


//=============================================================================
//				  Global Data Definition
//=============================================================================
JPG_DECODER    	*gptJPG_DECODER     = NULL;
static uint32_t Jbuf_vram_addr  	= 0;
static uint8_t* Jbuf_sys_addr   	= NULL;

//=============================================================================
//				  Private Function Declaration
//=============================================================================

//=============================================================================
//				  Private Function Definition
//=============================================================================

void
ite_mjpeg_decode_display(
	void)
{
	uint32_t	   frame_width, frame_height, frame_PitchY, frame_PitchUV;
	
	frame_width   = gptJPG_DECODER->frameWidth;
	frame_height  = gptJPG_DECODER->frameHeight;
	frame_PitchY  = gptJPG_DECODER->framePitchY;
	frame_PitchUV = gptJPG_DECODER->framePitchUV;
	
	if (!Jbuf_sys_addr)
	{
		Jbuf_vram_addr = itpVmemAlignedAlloc(32,(frame_PitchY * frame_height * 3 ) ); //for YUV420
		if(!Jbuf_vram_addr) printf("Jbuf_sys_addr Alloc Buffer Fail!!\n");
		
		Jbuf_sys_addr = (uint8_t*) ithMapVram(Jbuf_vram_addr,(frame_PitchY * frame_height * 3 ) , ITH_VRAM_WRITE);
		gptJPG_DECODER->frameBufCount = 0;
		gptJPG_DECODER->currDisplayFrameBufIndex = 0;
	}

	if(!gptJPG_DECODER->frameBufCount)
	{
		gptJPG_DECODER->OutAddrY[0] = Jbuf_sys_addr;
		gptJPG_DECODER->OutAddrU[0] = gptJPG_DECODER->OutAddrY[0]  + (frame_PitchY * frame_height); 
		gptJPG_DECODER->OutAddrV[0] = gptJPG_DECODER->OutAddrU[0]  + (frame_PitchY * frame_height >> 2);

		gptJPG_DECODER->OutAddrY[1] = gptJPG_DECODER->OutAddrV[0]  + (frame_PitchY * frame_height >> 2);
		gptJPG_DECODER->OutAddrU[1] = gptJPG_DECODER->OutAddrY[1]  + (frame_PitchY * frame_height);
		gptJPG_DECODER->OutAddrV[1] = gptJPG_DECODER->OutAddrU[1]  + (frame_PitchY * frame_height >> 2);
		gptJPG_DECODER->frameBufCount = 2;
	}

	switch (gptJPG_DECODER->currDisplayFrameBufIndex)
    {
	    case 0:
	        gptJPG_DECODER->DisplayAddrY = gptJPG_DECODER->OutAddrY[0];
	        gptJPG_DECODER->DisplayAddrU = gptJPG_DECODER->OutAddrU[0];
	        gptJPG_DECODER->DisplayAddrV = gptJPG_DECODER->OutAddrV[0];
	        break;

	    case 1:
	        gptJPG_DECODER->DisplayAddrY = gptJPG_DECODER->OutAddrY[1];
	        gptJPG_DECODER->DisplayAddrU = gptJPG_DECODER->OutAddrU[1];
	        gptJPG_DECODER->DisplayAddrV = gptJPG_DECODER->OutAddrV[1];
	        break;
    }

	if(gptJPG_DECODER->currDisplayFrameBufIndex >= 1) 	gptJPG_DECODER->currDisplayFrameBufIndex = 0;
	else gptJPG_DECODER->currDisplayFrameBufIndex++;	
}

static av_cold int
ite_mjpeg_decode_init(
    AVCodecContext *avctx)
{
    int                rst        = 0;
    JPG_ERR            jpgRst     = JPG_ERR_OK;
    MJPEG_ITE_DEC_CTXT *mjDecCtxt = avctx->priv_data;
    JPG_INIT_PARAM     initParam  = {0};

    if (!mjDecCtxt->picture_ptr)
        mjDecCtxt->picture_ptr = &mjDecCtxt->picture;

    avcodec_get_frame_defaults(&mjDecCtxt->picture);

	if (NULL == gptJPG_DECODER)
	gptJPG_DECODER = (JPG_DECODER *)calloc(sizeof(char), sizeof(JPG_DECODER)); // for jpg engine

    mjDecCtxt->avctx        = avctx;

    initParam.codecType     = JPG_CODEC_DEC_MJPG;
    initParam.decType       = JPG_DEC_PRIMARY;
    initParam.outColorSpace = JPG_COLOR_SPACE_YUV420;

    initParam.dispMode      = JPG_DISP_CENTER; 
	initParam.width         = ithLcdGetWidth();
    initParam.height        = ithLcdGetHeight();

    iteJpg_CreateHandle(&mjDecCtxt->pHJpeg, &initParam, 0);

    if (!mjDecCtxt->pHJpeg)
    {
        printf(" err ! create mjpg dec handle fail ! %s [%d]\n", __FILE__, __LINE__);
        rst = -1;
    }

    return rst;
}

static int
ite_mjpeg_decode_frame(
    AVCodecContext *avctx,
    void           *data,
    int            *data_size,
    AVPacket       *avpkt)
{
    int                result     = 0;
    JPG_ERR            jpgRst     = JPG_ERR_OK;
	JPG_RECT	       destRect       = {0};
    MJPEG_ITE_DEC_CTXT *mjDecCtxt = avctx->priv_data;

    do
    {
        JPG_STREAM_INFO inStreamInfo   = {0};
        JPG_STREAM_INFO outStreamInfo  = {0};
        JPG_BUF_INFO    entropyBufInfo = {0};
        JPG_USER_INFO   jpgUserInfo    = {0};
        int             buf_size       = avpkt->size;
        AVFrame         *picture       = data;
        uint8_t   *pStar         = avpkt->data;
        uint8_t   *pEnd          = pStar + buf_size;
		
		JPG_RECT        destRect       = {0};

        // ------------------------------------
        // set src type
        inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
        inStreamInfo.streamType            = JPG_STREAM_MEM;
        inStreamInfo.jstream.mem[0].pAddr  = pStar;
        inStreamInfo.jstream.mem[0].length = buf_size;
        inStreamInfo.validCompCnt          = 1;
        iteJpg_SetStreamInfo(mjDecCtxt->pHJpeg, &inStreamInfo, 0, 0);

        // ------------------------------------
        // parsing Header
        jpgRst = iteJpg_Parsing(mjDecCtxt->pHJpeg, &entropyBufInfo,(void *)&destRect);
        if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            result = -1;
            break;
        }

        // ----------------------------------------
        // get output YUV plan buffer
        iteJpg_GetStatus(mjDecCtxt->pHJpeg, &jpgUserInfo, 0);
        avctx->pix_fmt                    = PIX_FMT_YUV420P; //PIX_FMT_YUVJ420P;// output YUV420

        avcodec_set_dimensions(mjDecCtxt->avctx, jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h);

        if (mjDecCtxt->picture_ptr->data[0])
        {
            avctx->release_buffer(avctx, mjDecCtxt->picture_ptr);
        }

        if (avctx->get_buffer(avctx, mjDecCtxt->picture_ptr) < 0)
        {
            printf("get_buffer() failed ! %s [%d]\n", __FILE__, __LINE__);
            result = -1;
            break;
        }

		gptJPG_DECODER->frameHeight =  jpgUserInfo.real_height;
		gptJPG_DECODER->frameWidth  =  jpgUserInfo.real_width;
		gptJPG_DECODER->framePitchY  = jpgUserInfo.comp1Pitch;
		gptJPG_DECODER->framePitchUV = jpgUserInfo.comp23Pitch;
		
		ite_mjpeg_decode_display();

        outStreamInfo.streamIOType         = JPG_STREAM_IO_WRITE;
        outStreamInfo.streamType           = JPG_STREAM_MEM;

		outStreamInfo.jstream.mem[0].pAddr  = gptJPG_DECODER->DisplayAddrY; // get output buf;
        outStreamInfo.jstream.mem[0].pitch  = gptJPG_DECODER->framePitchY;
        outStreamInfo.jstream.mem[0].length = gptJPG_DECODER->framePitchY * gptJPG_DECODER->frameHeight;
        // U
        outStreamInfo.jstream.mem[1].pAddr  = gptJPG_DECODER->DisplayAddrU;
        outStreamInfo.jstream.mem[1].pitch  = gptJPG_DECODER->framePitchUV;
        outStreamInfo.jstream.mem[1].length = gptJPG_DECODER->framePitchUV * gptJPG_DECODER->frameHeight >> 1;
        // V
        outStreamInfo.jstream.mem[2].pAddr  = gptJPG_DECODER->DisplayAddrV;
        outStreamInfo.jstream.mem[2].pitch  = gptJPG_DECODER->framePitchUV;
        outStreamInfo.jstream.mem[2].length = gptJPG_DECODER->framePitchUV * gptJPG_DECODER->frameHeight >> 1;
		
		printf("\n\tY=0x%x, u=0x%x, v=0x%x\n",
					outStreamInfo.jstream.mem[0].pAddr,
					outStreamInfo.jstream.mem[1].pAddr,
					outStreamInfo.jstream.mem[2].pAddr);


        outStreamInfo.validCompCnt         = 3;
        jpgRst                             = iteJpg_SetStreamInfo(mjDecCtxt->pHJpeg, 0, &outStreamInfo, 0);
        if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            result = -1;
            break;
        }
        // ------------------------------
        // setup jpg
        jpgRst = iteJpg_Setup(mjDecCtxt->pHJpeg, 0);
        if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            result = -1;
            break;
        }

        // ------------------------------
        // fire H/W jpg
        jpgRst = iteJpg_Process(mjDecCtxt->pHJpeg, &entropyBufInfo, 0, 0);
        if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            result = -1;
            break;
        }
		
		iteJpg_GetStatus(mjDecCtxt->pHJpeg, &jpgUserInfo, 0); 
	   	printf("\n\tresult = %d\n", jpgUserInfo.status); 

        jpgRst = iteJpg_WaitIdle(mjDecCtxt->pHJpeg, 0);
        if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            result = -1;
            break;
        }
		
		jpgRst = iteJpg_Reset(mjDecCtxt->pHJpeg,0);
		if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            result = -1;
            break;
        }

        *picture   = *mjDecCtxt->picture_ptr;
        *data_size = 0;
		
		picture->width 							= gptJPG_DECODER->frameWidth;
		picture->height							= gptJPG_DECODER->frameHeight;

		picture->data[0]							= outStreamInfo.jstream.mem[0].pAddr;
		picture->data[1]							= outStreamInfo.jstream.mem[1].pAddr;
		picture->data[2]							= outStreamInfo.jstream.mem[2].pAddr;
		picture->data[3]							=  0;

		picture->linesize[0]						= outStreamInfo.jstream.mem[0].pitch;
		picture->linesize[1]						= outStreamInfo.jstream.mem[1].pitch;

		// tmp, need to modify! make no sense now.
		picture->pts								= 0;
		picture->pict_type = AV_PICTURE_TYPE_I;
        picture->key_frame = 1;
		*data_size								= sizeof(AVFrame);
		
    } while (0);

    if (result < 0)
    {
        // reset H/W jpg
        iteJpg_Reset(mjDecCtxt->pHJpeg, 0);
        *data_size = 0;
    }

    return (avpkt->size);
}

static av_cold int
ite_mjpeg_decode_end(
    AVCodecContext *avctx)
{
    JPG_ERR            result     = JPG_ERR_OK;
    MJPEG_ITE_DEC_CTXT *mjDecCtxt = avctx->priv_data;

    if (mjDecCtxt->picture_ptr && mjDecCtxt->picture_ptr->data[0])
        avctx->release_buffer(avctx, mjDecCtxt->picture_ptr);

    iteJpg_DestroyHandle(&mjDecCtxt->pHJpeg, 0);
	if (Jbuf_sys_addr)
    {
        itpVmemFree(Jbuf_vram_addr);
        Jbuf_sys_addr  = NULL;
        Jbuf_vram_addr = 0;
    }
	if (gptJPG_DECODER)
	{
        free(gptJPG_DECODER);
    	gptJPG_DECODER = NULL;
	}
    return 0;
}

//=============================================================================
//				  Public Function Definition
//=============================================================================
static enum PixelFormat inSrc_fmts[3] = {PIX_FMT_YUVJ420P, PIX_FMT_YUVJ422P, PIX_FMT_NONE};

#if defined(_MSC_VER)
AVCodec                 ff_ite_mjpeg_decoder = {
    "ite mjpeg",
    AVMEDIA_TYPE_VIDEO,
    CODEC_ID_MJPEG, // CODEC_ID_ITE_MJPEG
    sizeof(MJPEG_ITE_DEC_CTXT),
    ite_mjpeg_decode_init,
    NULL,
    ite_mjpeg_decode_end,
    ite_mjpeg_decode_frame,
    CODEC_CAP_DELAY,
    NULL,
    NULL,
    NULL,
    NULL,
    "MJPEG (Motion JPEG)",
	NULL,
	NULL,
	NULL,
	3,
};
#else // !defined (_MSC_VER)
AVCodec ff_ite_mjpeg_decoder = {
    .name           = "ite mjpeg",
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = CODEC_ID_MJPEG,
    .priv_data_size = sizeof(MJPEG_ITE_DEC_CTXT),
    .init           = ite_mjpeg_decode_init,
    .close          = ite_mjpeg_decode_end,
    .decode         = ite_mjpeg_decode_frame,
    .capabilities   = CODEC_CAP_DELAY,
    .flush          = NULL,
    .long_name      = "MJPEG (Motion JPEG)",
    .priv_class     = NULL,
    .max_lowres 	= 3,
};
#endif // _MSC_VER

