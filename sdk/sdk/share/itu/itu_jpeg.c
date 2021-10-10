#include <sys/stat.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "jpeglib.h"
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

#include "ite/itv.h" //test


#if defined(CFG_JPEG_HW_ENABLE)

    #include "jpg/ite_jpg.h"
    #include "isp/mmp_isp.h"
    #define MAX_JPEG_DECODE_SIZE 36000000
    #define JPEG_SOF_MARKER      0xFFC0
    #define JPEG_SOS_MARKER      0xFFDA
    #define JPEG_DHT_MARKER      0xFFC4
    #define JPEG_DRI_MARKER      0xFFDD
    #define JPEG_DQT_MARKER      0xFFDB
    #define JPEG_APP00_MARKER    0xFFE0
    #define JPEG_APP01_MARKER    0xFFE1
    #define JPEG_APP02_MARKER    0xFFE2
    #define JPEG_APP03_MARKER    0xFFE3
    #define JPEG_APP04_MARKER    0xFFE4
    #define JPEG_APP05_MARKER    0xFFE5
    #define JPEG_APP06_MARKER    0xFFE6
    #define JPEG_APP07_MARKER    0xFFE7
    #define JPEG_APP08_MARKER    0xFFE8
    #define JPEG_APP09_MARKER    0xFFE9
    #define JPEG_APP10_MARKER    0xFFEA
    #define JPEG_APP11_MARKER    0xFFEB
    #define JPEG_APP12_MARKER    0xFFEC
    #define JPEG_APP13_MARKER    0xFFED
    #define JPEG_APP14_MARKER    0xFFEE
    #define JPEG_APP15_MARKER    0xFFEF
    #define JPEG_COM_MARKER      0xFFFE

//=============================================================================
//				  Constant Definition
//=============================================================================
typedef enum DATA_COLOR_TYPE_TAG
{
    DATA_COLOR_YUV444,
    DATA_COLOR_YUV422,
    DATA_COLOR_YUV422R,
    DATA_COLOR_YUV420,
    DATA_COLOR_ARGB8888,
    DATA_COLOR_ARGB4444,
    DATA_COLOR_RGB565,
    DATA_COLOR_NV12,
    DATA_COLOR_NV21,

    DATA_COLOR_CNT,
} DATA_COLOR_TYPE;

typedef enum _ISP_ACT_CMD_TAG
{
    ISP_ACT_CMD_IDLE = 0,
    ISP_ACT_CMD_INIT,
    ISP_ACT_CMD_TERMINATE,
    ISP_ACT_CMD_PROC,
} ISP_ACT_CMD;

//=============================================================================
//				  Macro Definition
//=============================================================================

//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct _BASE_RECT_TAG
{
    int x;
    int y;
    int w;
    int h;
} BASE_RECT;

typedef struct _CLIP_WND_INFO_TAG
{
    int       bClipEnable;
    int       bClipOutside;
    int       clipWndId;
    BASE_RECT clipRect;
} CLIP_WND_INFO;

//=============================================================================
//				  Global Data Definition
//=============================================================================
static ISP_DEVICE gIspDev;
extern ITUSurface *VideoSurf[2];

//=============================================================================
//				  Private Function Definition
//=============================================================================

//=============================================================================
//				  Public Function Definition
//=============================================================================
void
set_isp_colorTrans(
    uint8_t         *srcAddr_rgby,
    uint8_t         *srcAddr_u,
    uint8_t         *srcAddr_v,
    DATA_COLOR_TYPE colorType,
    CLIP_WND_INFO   *clipInfo,
    BASE_RECT       *srcRect,
    BASE_RECT       *destRect,
    int             imgWidth,
    int             imgHeight,
    int             M2dPitch,
    uint16_t        *dest)
{
    int                 result   = 0;
    int                 width    = 0, height = 0;
    MMP_ISP_OUTPUT_INFO outInfo  = {0};
    MMP_ISP_SHARE       ispInput = {0};

    ispInput.width        = (imgWidth >> 3) << 3;  //srcRect->w;
    ispInput.height       = (imgHeight >> 3) << 3; //srcRect->h;
    ispInput.isAdobe_CMYK = 0;

    switch (colorType)
    {
    case DATA_COLOR_YUV444:
    case DATA_COLOR_YUV422:
    case DATA_COLOR_YUV422R:
    case DATA_COLOR_YUV420:
        ispInput.addrY = (uint32_t)srcAddr_rgby;
        ispInput.addrU = (uint32_t)srcAddr_u;
        ispInput.addrV = (uint32_t)srcAddr_v;
        switch (colorType)
        {
        case DATA_COLOR_YUV444:
            ispInput.format  = MMP_ISP_IN_YUV444;
            ispInput.pitchY  = srcRect->w;
            ispInput.pitchUv = srcRect->w;
            break;
        case DATA_COLOR_YUV422:
            ispInput.format  = MMP_ISP_IN_YUV422;
            ispInput.pitchY  = srcRect->w;
			ispInput.pitchUv = (ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 && ithIsTilingModeOn()) ? srcRect->w : (srcRect->w >> 1);
            break;
        case DATA_COLOR_YUV422R:
            ispInput.format  = MMP_ISP_IN_YUV422R;
            ispInput.pitchY  = srcRect->w;
            ispInput.pitchUv = srcRect->w;
            break;
        case DATA_COLOR_YUV420:
            ispInput.format  = MMP_ISP_IN_YUV420;
            ispInput.pitchY  = srcRect->w;
			ispInput.pitchUv = (ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 && ithIsTilingModeOn()) ? srcRect->w : (srcRect->w >> 1); 
            break;
        }
        break;

    case DATA_COLOR_ARGB8888:
    case DATA_COLOR_ARGB4444:
    case DATA_COLOR_RGB565:
        ispInput.addrY = (uint32_t)srcAddr_rgby;
        switch (colorType)
        {
        case DATA_COLOR_ARGB8888:
            ispInput.format = MMP_ISP_IN_RGB888;
            ispInput.pitchY = (srcRect->w << 2);
            break;

        case DATA_COLOR_RGB565:
            ispInput.format = MMP_ISP_IN_RGB565;
            ispInput.pitchY = (srcRect->w << 1);
            break;

        case DATA_COLOR_ARGB4444:
            printf(" not support ARGB4444 !");
            return;
        }
        break;

    case DATA_COLOR_NV12:
    case DATA_COLOR_NV21:
        ispInput.addrY   = (uint32_t)srcAddr_rgby;
        ispInput.addrU   = (uint32_t)srcAddr_u;
        ispInput.pitchY  = srcRect->w;
        ispInput.pitchUv = srcRect->w;
        ispInput.format  = (colorType == DATA_COLOR_NV12) ? MMP_ISP_IN_NV12 : MMP_ISP_IN_NV21;
        break;
    }

    // ---------------------------------------
    // initial isp
    mmpIspTerminate(&gIspDev);
    width  = destRect->w; //dispWidth;
    height = destRect->h; //dispHeight;

	//delete rotate codes.
    mmpIspInitialize(&gIspDev);
#if (CFG_CHIP_FAMILY == 9850)
	mmpIspSetMode(gIspDev, MMP_ISP_MODE_JPEG_TRANSFORM);
#else
	mmpIspSetMode(gIspDev, MMP_ISP_MODE_TRANSFORM);
#endif
    mmpIspSetDisplayWindow(gIspDev, 0, 0, width, height);
    mmpIspSetVideoWindow(gIspDev, 0, 0, width, height);
    mmpIspSetOutputFormat(gIspDev, MMP_ISP_OUT_DITHER565);
    //ithLcdDisableHwFlip();

    // set isp output to M2D Buffer
    outInfo.startX   = destRect->x; //0;
    outInfo.startY   = destRect->y; //0;
    outInfo.width    = width;  //dispWidth;
    outInfo.height   = height; //dispHeight;
    outInfo.addrRGB  = (uint32_t)dest;
    outInfo.pitchRGB = (uint16_t)M2dPitch;
    outInfo.format   = MMP_ISP_OUT_DITHER565;
    mmpIspSetOutputWindow(gIspDev, &outInfo);

    // set isp source info and fire
    result = mmpIspPlayImageProcess(gIspDev, &ispInput);
    if (result)
        printf("mmpIspPlayImageProcess() error (0x%x) !!\n", result);

    result = mmpIspWaitEngineIdle();
    if (result)
        printf("mmpIspWaitEngineIdle() error (0x%x) !!\n", result);
}

int *ituJpegLoadEx(int width, int height, uint8_t *data, int size)
{
    ITUSurface      *surf          = NULL;
    uint16_t        *dest          = NULL;
    HJPG            *pHJpeg        = 0;
    JPG_INIT_PARAM  initParam      = {0};
    JPG_STREAM_INFO inStreamInfo   = {0};
    JPG_STREAM_INFO outStreamInfo  = {0};
    JPG_BUF_INFO    entropyBufInfo = {0};
    JPG_USER_INFO   jpgUserInfo    = {0};
    JPG_ERR         result         = JPG_ERR_OK;
    ITUColor        black          = { 0, 0, 0, 0 };        
    JPG_RECT        destRect       = {0};
    CLIP_WND_INFO   clipInfo       = {0};
    BASE_RECT       srcRect        = {0};
    DATA_COLOR_TYPE colorType      = 0;
    uint32_t        real_width     = 0, real_height = 0, real_height_ForTile = 0;
    uint32_t        imgWidth       = 0, imgHeight = 0;
    uint32_t        SmallPicWidth  = 800, SmallPicHeight = 480;
    uint8_t         *pY			   = 0 ,*pStart        = 0, *pCur = 0, *pEnd = 0;
    uint32_t        CurCount       = 0, MarkerType = 0, GetMarkerLength = 0;
	uint8_t         *dbuf	       = NULL; 
    ITV_DBUF_PROPERTY  dbufprop    = {0};
	int new_index = 0;

    //malloc_stats();
    
    if (data[0] != 0xFF || data[1] != 0xD8)
    {
        printf("jpeg read stream fail,data[0]=0x%x,data[1]=0x%x\n", data[0], data[1]);
        return 0;
    }
    else
    {
        pStart = pCur = data;
        pEnd   = pCur + size;

        while ( (pCur < pEnd) )
        {
            MarkerType = (*(pCur) << 8 | *(pCur + 1));
            switch (MarkerType)
            {
            case JPEG_SOF_MARKER:
                pCur           += 2;
                GetMarkerLength = (*(pCur) << 8 | *(pCur + 1));
                CurCount        = pCur - pStart;
                CurCount       += 3; //pass 3 byte
                imgHeight       = pStart[CurCount] << 8 | pStart[CurCount + 1];
                CurCount       += 2;
                imgWidth        = pStart[CurCount] << 8 | pStart[CurCount + 1];
                pCur           += GetMarkerLength;
                break;
            case JPEG_SOS_MARKER:
            case JPEG_DHT_MARKER:
            case JPEG_DRI_MARKER:
            case JPEG_DQT_MARKER:
            case JPEG_APP00_MARKER:
            case JPEG_APP01_MARKER:
            case JPEG_APP02_MARKER:
            case JPEG_APP03_MARKER:
            case JPEG_APP04_MARKER:
            case JPEG_APP05_MARKER:
            case JPEG_APP06_MARKER:
            case JPEG_APP07_MARKER:
            case JPEG_APP08_MARKER:
            case JPEG_APP09_MARKER:
            case JPEG_APP10_MARKER:
            case JPEG_APP11_MARKER:
            case JPEG_APP12_MARKER:
            case JPEG_APP13_MARKER:
            case JPEG_APP14_MARKER:
            case JPEG_APP15_MARKER:
            case JPEG_COM_MARKER:
                pCur           += 2;
                GetMarkerLength = (*(pCur) << 8 | *(pCur + 1));
                pCur           += GetMarkerLength;
                break;
            default:
                pCur++;
                break;
            }
        }
        pCur = pStart;
    }

    if (width == 0 || height == 0)
    {
        width = (int)imgWidth;
        height = (int)imgHeight;
    }

	new_index = itv_get_vidSurf_index();
	while( new_index == -1)
	{
		//printf("wait to get new_index!\n");
		usleep(1000);
        new_index = itv_get_vidSurf_index();
	}

    switch(new_index)
    {
       case  0:
            new_index =1;
       break;
       
       case  1:
       case -2:
            new_index =0;
       break;
    }

	surf = VideoSurf[new_index];
	dest = (uint8_t *)ituLockSurface(surf, 0, 55, width, height);
	ituColorFill(surf, 0, 55, width, height, &black);

    if ((imgWidth * imgHeight >= MAX_JPEG_DECODE_SIZE) || imgWidth >=4096 || imgHeight >= 4096)
    {
        printf("JPG not support this format\n");
        ituUnlockSurface(surf);
        return 0;
    }
    else if ( (imgWidth * imgHeight) <= (SmallPicWidth * SmallPicHeight))
    {
        initParam.codecType                = JPG_CODEC_DEC_JPG_CMD;
        initParam.decType                  = JPG_DEC_PRIMARY;
        initParam.dispMode                 = JPG_DISP_CENTER;        //JPG_DISP_FIT;
        initParam.outColorSpace            = JPG_COLOR_SPACE_YUV420; //just a initial value. not really the picture`s  format.JPG CMD mode back will modify it.
        initParam.width                    = width;
        initParam.height                   = height;
        iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

        inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
        inStreamInfo.streamType            = JPG_STREAM_MEM;
        inStreamInfo.jpg_reset_stream_info = 0;

        inStreamInfo.validCompCnt          = 1;
        inStreamInfo.jstream.mem[0].pAddr  = data;
        inStreamInfo.jstream.mem[0].length = size;
        iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, 0, 0);
        result                             = iteJpg_Parsing(pHJpeg, &entropyBufInfo, (void *)&destRect);
        if (result == JPG_ERR_JPROG_STREAM)
        {
            printf("JPG not support this format\n");
            goto end;
        }

        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
        printf("memory mode  (%d, %d) %dx%d, dispMode=%d\r\n",
               jpgUserInfo.jpgRect.x, jpgUserInfo.jpgRect.y,
               jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
               initParam.dispMode);

        real_width                          = jpgUserInfo.real_width;
        real_height                         = jpgUserInfo.real_height;

	   if (ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 && ithIsTilingModeOn())
	   {
 		   real_width = jpgUserInfo.comp1Pitch;
		   real_height_ForTile	= (real_height + 0x7f)/ 0x80;
		   pY = memalign(128*1024,(jpgUserInfo.comp1Pitch * 128 * real_height_ForTile * 3));
		   if(!pY) 
		   {
			   printf("JPG Allocate Line buf fail !!\n");
			   goto end;	  
		   }
		   memset(pY, 0x0, jpgUserInfo.comp1Pitch * 128 * real_height_ForTile * 3);
	   
		   outStreamInfo.jstream.mem[0].pAddr  = (uint8_t*)pY;	// get output buf;
		   outStreamInfo.jstream.mem[0].pitch  = jpgUserInfo.comp1Pitch;
		   outStreamInfo.jstream.mem[0].length = jpgUserInfo.comp1Pitch  * 128* real_height_ForTile;
		   // U
		   outStreamInfo.jstream.mem[1].pAddr  = (uint8_t*)(outStreamInfo.jstream.mem[0].pAddr + outStreamInfo.jstream.mem[0].length);
		   outStreamInfo.jstream.mem[1].pitch  = jpgUserInfo.comp23Pitch;
		   outStreamInfo.jstream.mem[1].length = outStreamInfo.jstream.mem[1].pitch *  real_height_ForTile *128 ;
		   // V
		   outStreamInfo.jstream.mem[2].pAddr  = (uint8_t*)(outStreamInfo.jstream.mem[1].pAddr + outStreamInfo.jstream.mem[1].length);
		   outStreamInfo.jstream.mem[2].pitch  = jpgUserInfo.comp23Pitch;
		   outStreamInfo.jstream.mem[2].length = outStreamInfo.jstream.mem[2].pitch * real_height_ForTile *128 ;
		   outStreamInfo.validCompCnt = 3;
	   }
	   else
	   {
		   real_width						   = 2048;
		   pY	  = malloc(real_width * real_height * 3);
		   if(!pY) 
		   {
			   printf("JPG Allocate Line buf fail !!\n");
			   goto end;	  
		   }
		   memset(pY, 0x0, real_width * real_height * 3);
	   
		   outStreamInfo.jstream.mem[0].pAddr  = (uint8_t *)pY; // get output buf;
		   outStreamInfo.jstream.mem[0].pitch  = jpgUserInfo.comp1Pitch;
		   outStreamInfo.jstream.mem[0].length = real_width * real_height;
		   // U
		   outStreamInfo.jstream.mem[1].pAddr  = (uint8_t *)(outStreamInfo.jstream.mem[0].pAddr + outStreamInfo.jstream.mem[0].length);
		   outStreamInfo.jstream.mem[1].pitch  = jpgUserInfo.comp23Pitch;
		   outStreamInfo.jstream.mem[1].length = outStreamInfo.jstream.mem[1].pitch * real_height;
		   // V
		   outStreamInfo.jstream.mem[2].pAddr  = (uint8_t *)(outStreamInfo.jstream.mem[1].pAddr + outStreamInfo.jstream.mem[1].length);
		   outStreamInfo.jstream.mem[2].pitch  = jpgUserInfo.comp23Pitch;
		   outStreamInfo.jstream.mem[2].length = outStreamInfo.jstream.mem[2].pitch * real_height;
		   outStreamInfo.validCompCnt		   = 3;

	   }     
    }
    else
    {
        initParam.codecType     = JPG_CODEC_DEC_JPG;
        initParam.decType       = JPG_DEC_PRIMARY;
        initParam.outColorSpace = JPG_COLOR_SPACE_RGB565;
        initParam.width         = width;
        initParam.height        = height;
        initParam.dispMode      = JPG_DISP_CENTER;//JPG_DISP_FIT;
        iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

        inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
        inStreamInfo.streamType            = JPG_STREAM_MEM;
        inStreamInfo.jpg_reset_stream_info = 0;

        inStreamInfo.validCompCnt          = 1;
        inStreamInfo.jstream.mem[0].pAddr  = data;
        inStreamInfo.jstream.mem[0].length = size;

        result = iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, 0, 0);
        if (result != JPG_ERR_OK)
        {
            printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
            goto end;
        }
        result = iteJpg_Parsing(pHJpeg, &entropyBufInfo, (void *)&destRect);
        if (result == JPG_ERR_JPROG_STREAM)
        {
            printf("JPG not support this format\n");
            goto end;
        }
        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
        /*
        printf("handshake mode  (%d, %d) %dx%d, dispMode=%d\r\n",
               jpgUserInfo.jpgRect.x, jpgUserInfo.jpgRect.y,
               jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
               initParam.dispMode);
        	*/
        outStreamInfo.jstream.mem[0].pAddr  = (uint8_t *)dest;            // get output buf;
        outStreamInfo.jstream.mem[0].pitch  = surf->pitch;
        outStreamInfo.jstream.mem[0].length = surf->pitch * surf->height; //outStreamInfo.jstream.mem[0].pitch * jpgUserInfo.jpgRect.h;
        outStreamInfo.validCompCnt          = 1;
    }
    outStreamInfo.streamIOType = JPG_STREAM_IO_WRITE;
    outStreamInfo.streamType   = JPG_STREAM_MEM;

    result = iteJpg_SetStreamInfo(pHJpeg, 0, &outStreamInfo, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
        goto end;
    }

    result = iteJpg_Setup(pHJpeg, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
        goto end;
    }

    if (pY)
    {	
    	if (ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 && ithIsTilingModeOn()) 
    		ithFlushDCacheRange((void *)pY, (jpgUserInfo.comp1Pitch * 128 * real_height_ForTile * 5 ));
    	else
        	ithFlushDCacheRange((void *)pY, real_width * real_height * 3);
        ithFlushMemBuffer();
    }

    result = iteJpg_Process(pHJpeg, &entropyBufInfo, 0, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
        goto end;
    }

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    //printf("\n\tresult = %d\n", jpgUserInfo.status);

    iteJpg_DestroyHandle(&pHJpeg, 0);

    if ((imgWidth * imgHeight) <= (SmallPicWidth * SmallPicHeight))
    {
        clipInfo.bClipEnable = 0;

        srcRect.w            = real_width;
        srcRect.h            = real_height;

        switch (jpgUserInfo.colorFormate)
        {
        case JPG_COLOR_SPACE_YUV411:    colorType  = DATA_COLOR_YUV422; break;
        case JPG_COLOR_SPACE_YUV444:    colorType  = DATA_COLOR_YUV444; break;
        case JPG_COLOR_SPACE_YUV422:    colorType  = DATA_COLOR_YUV422; break;
        case JPG_COLOR_SPACE_YUV420:    colorType  = DATA_COLOR_YUV420; break;
        case JPG_COLOR_SPACE_YUV422R:   colorType  = DATA_COLOR_YUV422R; break;
        case JPG_COLOR_SPACE_RGB565:     colorType = DATA_COLOR_RGB565; break;
        }

        set_isp_colorTrans(
            outStreamInfo.jstream.mem[0].pAddr,
            outStreamInfo.jstream.mem[1].pAddr,
            outStreamInfo.jstream.mem[2].pAddr,
            colorType,
            &clipInfo,
            &srcRect,
            &destRect,
            imgWidth,
            imgHeight,
            surf->pitch,
            dest);
    }

	while( (dbuf = itv_get_dbuf_anchor()) == NULL)
	{
		//printf("wait to get dbuf!\n");
		usleep(1000);
	}
	
	{
		// ------------------------------------
		// Just through itv driver to Flip LCD ,both handshake mode or command trigger mode run this setting (when command trigger ,MMP_ISP_IN_RGB565 is not really format.).
		dbufprop.src_w    = 0;
		dbufprop.src_h    = 0;
		dbufprop.pitch_y  = 0;
		dbufprop.pitch_uv = 0;
		dbufprop.format   = MMP_ISP_IN_RGB565;
		dbufprop.ya       = 0;
		dbufprop.ua       = 0;
		dbufprop.va       = 0;
		dbufprop.bidx     = 0;
		//printf("dbufprop.ya=0x%x,dbufprop.ua=0x%x,dbufprop.va=0x%x,dbufprop.src_w=%d,dbufprop.src_h=%d,dbufprop.pitch_y=%d,dbufprop.pitch_uv=%d,dbufprop.format=%d\n",dbufprop.ya,dbufprop.ua,dbufprop.va,dbufprop.src_w,dbufprop.src_h,dbufprop.pitch_y,dbufprop.pitch_uv,dbufprop.format);
		itv_update_dbuf_anchor(&dbufprop);
	}
	
    if (pY)        free(pY);

    //printf("jpeg decode end\n");
	ituUnlockSurface(surf);
    return 1;

end:
    iteJpg_DestroyHandle(&pHJpeg, 0);
	ituUnlockSurface(surf);
    return 1;
}

int *ituJpegLoadFileEx(int width, int height, char* filepath)
{
	int result;
    FILE* f = NULL;
    int size = 0;
    uint8_t* data = NULL;
    struct stat sb;

    assert(filepath);

    f = fopen(filepath, "rb");
    if (!f)
        goto end;

    if (fstat(fileno(f), &sb) == -1)
        goto end;

    size = sb.st_size;

    data = malloc(size);
    if (!data)
        goto end;

    size = fread(data, 1, size, f);
    result = ituJpegLoadEx(width, height, data, size);
end:
    free(data);

    if (f)
        fclose(f);

    return result;
}



ITUSurface *ituJpegLoad(int width, int height, uint8_t *data, int size, unsigned int flags)
{
    ITUSurface      *surf          = NULL;
    uint16_t        *dest          = NULL;
    HJPG            *pHJpeg        = 0;
    JPG_INIT_PARAM  initParam      = {0};
    JPG_STREAM_INFO inStreamInfo   = {0};
    JPG_STREAM_INFO outStreamInfo  = {0};
    JPG_BUF_INFO    entropyBufInfo = {0};
    JPG_USER_INFO   jpgUserInfo    = {0};
    JPG_ERR         result         = JPG_ERR_OK;
    ITUColor        black          = { 0, 0, 0, 0 };
    uint8_t         *pY            = 0;
    JPG_RECT        destRect       = {0};
    CLIP_WND_INFO   clipInfo       = {0};
    BASE_RECT       srcRect        = {0};
    DATA_COLOR_TYPE colorType      = 0;
    uint32_t        real_width     = 0, real_height = 0, real_height_ForTile = 0;
    uint32_t        imgWidth       = 0, imgHeight = 0;
    uint32_t        SmallPicWidth  = 800, SmallPicHeight = 480;
    uint8_t         *pStart        = 0, *pCur = 0, *pEnd = 0;
    uint32_t        CurCount       = 0, MarkerType = 0, GetMarkerLength = 0;

    //malloc_stats();
    if (data[0] != 0xFF || data[1] != 0xD8)
    {
        printf("jpeg read stream fail,data[0]=0x%x,data[1]=0x%x\n", data[0], data[1]);
        return NULL; //return NULL to tell AP, if JPEG reading fail!.
    }
    else
    {
        pStart = pCur = data;
        pEnd   = pCur + size;

        while ( (pCur < pEnd) )
        {
            MarkerType = (*(pCur) << 8 | *(pCur + 1));
            switch (MarkerType)
            {
            case JPEG_SOF_MARKER:
                pCur           += 2;
                GetMarkerLength = (*(pCur) << 8 | *(pCur + 1));
                CurCount        = pCur - pStart;
                CurCount       += 3; //pass 3 byte
                imgHeight       = pStart[CurCount] << 8 | pStart[CurCount + 1];
                CurCount       += 2;
                imgWidth        = pStart[CurCount] << 8 | pStart[CurCount + 1];
                pCur           += GetMarkerLength;
                break;
            case JPEG_SOS_MARKER:
            case JPEG_DHT_MARKER:
            case JPEG_DRI_MARKER:
            case JPEG_DQT_MARKER:
            case JPEG_APP00_MARKER:
            case JPEG_APP01_MARKER:
            case JPEG_APP02_MARKER:
            case JPEG_APP03_MARKER:
            case JPEG_APP04_MARKER:
            case JPEG_APP05_MARKER:
            case JPEG_APP06_MARKER:
            case JPEG_APP07_MARKER:
            case JPEG_APP08_MARKER:
            case JPEG_APP09_MARKER:
            case JPEG_APP10_MARKER:
            case JPEG_APP11_MARKER:
            case JPEG_APP12_MARKER:
            case JPEG_APP13_MARKER:
            case JPEG_APP14_MARKER:
            case JPEG_APP15_MARKER:
            case JPEG_COM_MARKER:
                pCur           += 2;
                GetMarkerLength = (*(pCur) << 8 | *(pCur + 1));
                pCur           += GetMarkerLength;
                break;
            default:
                pCur++;
                break;
            }
        }
        pCur = pStart;
    }

    if (width == 0 || height == 0)
    {
        width = (int)imgWidth;
        height = (int)imgHeight;
    }
#if 0
    else
    {
        if (width > (int)imgWidth)
            width = (int)imgWidth;

        if (height > (int)imgHeight)
            height = (int)imgHeight;
    }
#endif

    surf = ituCreateSurface(width, height, 0, ITU_RGB565, NULL, 0);
    if (!surf)
    {   
        printf("Jpeg Create Surface fail !!\n");
        return NULL;
    }
    dest = (uint16_t *)ituLockSurface(surf, 0, 0, width, height);
    ituColorFill(surf, 0, 0, width, height, &black);

    if ((imgWidth * imgHeight >= MAX_JPEG_DECODE_SIZE) || imgWidth >=4096 || imgHeight >= 4096)
    {
        printf("JPG not support this format\n");
        ituUnlockSurface(surf);
		ituDestroySurface(surf);
        return NULL;
    }
    else if ( (imgWidth * imgHeight) <= (SmallPicWidth * SmallPicHeight))
    {
        initParam.codecType                = JPG_CODEC_DEC_JPG_CMD;
        initParam.decType                  = JPG_DEC_PRIMARY;
		if(flags == ITU_FIT_TO_RECT)
		initParam.dispMode				   = JPG_DISP_FIT;
		else
		initParam.dispMode				   = JPG_DISP_CENTER; 
		
        initParam.outColorSpace            = JPG_COLOR_SPACE_YUV420; //just a initial value. not really the picture`s  format.JPG CMD mode back will modify it.
        initParam.width                    = width;
        initParam.height                   = height;
        iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

        inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
        inStreamInfo.streamType            = JPG_STREAM_MEM;
        inStreamInfo.jpg_reset_stream_info = 0;

        inStreamInfo.validCompCnt          = 1;
        inStreamInfo.jstream.mem[0].pAddr  = data;
        inStreamInfo.jstream.mem[0].length = size;
        iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, 0, 0);
        result                             = iteJpg_Parsing(pHJpeg, &entropyBufInfo, (void *)&destRect);
        if (result == JPG_ERR_JPROG_STREAM)
        {
            printf("JPG not support this format\n");
            goto end;
        }

        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
        printf("memory mode  (%d, %d) %dx%d, dispMode=%d\r\n",
               jpgUserInfo.jpgRect.x, jpgUserInfo.jpgRect.y,
               jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
               initParam.dispMode);

        real_width                          = jpgUserInfo.real_width;
        real_height                         = jpgUserInfo.real_height;

	   if (ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 && ithIsTilingModeOn())
	   {
 		   real_width = jpgUserInfo.comp1Pitch;
		   real_height_ForTile	= (real_height + 0x7f)/ 0x80;
		   pY = memalign(128*1024,(jpgUserInfo.comp1Pitch * 128 * real_height_ForTile * 3 ));
		   if(!pY) 
		   {
			   printf("JPG Allocate Line buf fail !!\n");
			   goto end;	  
		   }
		   memset(pY, 0x0, jpgUserInfo.comp1Pitch * 128 * real_height_ForTile * 3);
	   
		   outStreamInfo.jstream.mem[0].pAddr  = (uint8_t*)pY;	// get output buf;
		   outStreamInfo.jstream.mem[0].pitch  = jpgUserInfo.comp1Pitch;
		   outStreamInfo.jstream.mem[0].length = jpgUserInfo.comp1Pitch  * 128* real_height_ForTile;
		   // U
		   outStreamInfo.jstream.mem[1].pAddr  = (uint8_t*)(outStreamInfo.jstream.mem[0].pAddr + outStreamInfo.jstream.mem[0].length);
		   outStreamInfo.jstream.mem[1].pitch  = jpgUserInfo.comp23Pitch;
		   outStreamInfo.jstream.mem[1].length = outStreamInfo.jstream.mem[1].pitch *  real_height_ForTile *128 ;
		   // V
		   outStreamInfo.jstream.mem[2].pAddr  = (uint8_t*)(outStreamInfo.jstream.mem[1].pAddr + outStreamInfo.jstream.mem[1].length);
		   outStreamInfo.jstream.mem[2].pitch  = jpgUserInfo.comp23Pitch;
		   outStreamInfo.jstream.mem[2].length = outStreamInfo.jstream.mem[2].pitch * real_height_ForTile *128 ;
		   outStreamInfo.validCompCnt = 3;
	   }
	   else
	   {
		   real_width						   = 2048;
		   pY	  = malloc(real_width * real_height * 3);
		   if(!pY) 
		   {
			   printf("JPG Allocate Line buf fail !!\n");
			   goto end;	  
		   }
		   memset(pY, 0x0, real_width * real_height * 3);
	   
		   outStreamInfo.jstream.mem[0].pAddr  = (uint8_t *)pY; // get output buf;
		   outStreamInfo.jstream.mem[0].pitch  = jpgUserInfo.comp1Pitch;
		   outStreamInfo.jstream.mem[0].length = real_width * real_height;
		   // U
		   outStreamInfo.jstream.mem[1].pAddr  = (uint8_t *)(outStreamInfo.jstream.mem[0].pAddr + outStreamInfo.jstream.mem[0].length);
		   outStreamInfo.jstream.mem[1].pitch  = jpgUserInfo.comp23Pitch;
		   outStreamInfo.jstream.mem[1].length = outStreamInfo.jstream.mem[1].pitch * real_height;
		   // V
		   outStreamInfo.jstream.mem[2].pAddr  = (uint8_t *)(outStreamInfo.jstream.mem[1].pAddr + outStreamInfo.jstream.mem[1].length);
		   outStreamInfo.jstream.mem[2].pitch  = jpgUserInfo.comp23Pitch;
		   outStreamInfo.jstream.mem[2].length = outStreamInfo.jstream.mem[2].pitch * real_height;
		   outStreamInfo.validCompCnt		   = 3;

	   }     
    }
    else
    {
        initParam.codecType     = JPG_CODEC_DEC_JPG;
        initParam.decType       = JPG_DEC_PRIMARY;
        initParam.outColorSpace = JPG_COLOR_SPACE_RGB565;
        initParam.width         = width;
        initParam.height        = height;
		if(flags == ITU_FIT_TO_RECT)
		initParam.dispMode		 = JPG_DISP_FIT;
		else
		initParam.dispMode		= JPG_DISP_CENTER; 
		
        iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

        inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
        inStreamInfo.streamType            = JPG_STREAM_MEM;
        inStreamInfo.jpg_reset_stream_info = 0;

        inStreamInfo.validCompCnt          = 1;
        inStreamInfo.jstream.mem[0].pAddr  = data;
        inStreamInfo.jstream.mem[0].length = size;

        result = iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, 0, 0);
        if (result != JPG_ERR_OK)
        {
            printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
            goto end;
        }
        result = iteJpg_Parsing(pHJpeg, &entropyBufInfo, (void *)&destRect);
        if (result == JPG_ERR_JPROG_STREAM)
        {
            printf("JPG not support this format\n");
            goto end;
        }
        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
        /*
        printf("handshake mode  (%d, %d) %dx%d, dispMode=%d\r\n",
               jpgUserInfo.jpgRect.x, jpgUserInfo.jpgRect.y,
               jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
               initParam.dispMode);
        */
        outStreamInfo.jstream.mem[0].pAddr  = (uint8_t *)dest;            // get output buf;
        outStreamInfo.jstream.mem[0].pitch  = surf->pitch;
        outStreamInfo.jstream.mem[0].length = surf->pitch * surf->height; //outStreamInfo.jstream.mem[0].pitch * jpgUserInfo.jpgRect.h;
        outStreamInfo.validCompCnt          = 1;
    }
    outStreamInfo.streamIOType = JPG_STREAM_IO_WRITE;
    outStreamInfo.streamType   = JPG_STREAM_MEM;

    result = iteJpg_SetStreamInfo(pHJpeg, 0, &outStreamInfo, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
        goto end;
    }

    result = iteJpg_Setup(pHJpeg, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
        goto end;
    }

    if (pY)
    {	
    	if (ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 && ithIsTilingModeOn()) 
    		ithFlushDCacheRange((void *)pY, (jpgUserInfo.comp1Pitch * 128 * real_height_ForTile * 5 ));
    	else
        	ithFlushDCacheRange((void *)pY, real_width * real_height * 3);
        ithFlushMemBuffer();
    }

    result = iteJpg_Process(pHJpeg, &entropyBufInfo, 0, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
        goto end;
    }

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    //printf("\n\tresult = %d\n", jpgUserInfo.status);

    iteJpg_DestroyHandle(&pHJpeg, 0);

    if ((imgWidth * imgHeight) <= (SmallPicWidth * SmallPicHeight))
    {
        clipInfo.bClipEnable = 0;

        srcRect.w            = real_width;
        srcRect.h            = real_height;

        switch (jpgUserInfo.colorFormate)
        {
        case JPG_COLOR_SPACE_YUV411:    colorType  = DATA_COLOR_YUV422; break;
        case JPG_COLOR_SPACE_YUV444:    colorType  = DATA_COLOR_YUV444; break;
        case JPG_COLOR_SPACE_YUV422:    colorType  = DATA_COLOR_YUV422; break;
        case JPG_COLOR_SPACE_YUV420:    colorType  = DATA_COLOR_YUV420; break;
        case JPG_COLOR_SPACE_YUV422R:   colorType  = DATA_COLOR_YUV422R; break;
        case JPG_COLOR_SPACE_RGB565:     colorType = DATA_COLOR_RGB565; break;
        }

        set_isp_colorTrans(
            outStreamInfo.jstream.mem[0].pAddr,
            outStreamInfo.jstream.mem[1].pAddr,
            outStreamInfo.jstream.mem[2].pAddr,
            colorType,
            &clipInfo,
            &srcRect,
            &destRect,
            imgWidth,
            imgHeight,
            surf->pitch,
            dest);
    }
    if (pY)        free(pY);

    //printf("jpeg decode end\n");
    ituUnlockSurface(surf);
    return surf;

end:
    iteJpg_DestroyHandle(&pHJpeg, 0);
    ituUnlockSurface(surf);
	ituDestroySurface(surf);
    return NULL;
}

ITUSurface* ituJpegAlphaLoad(int width, int height, uint8_t* alpha, uint8_t* data, int size)
{
    // TODO: IMPLEMENT
    return NULL;
}

ITUSurface *ituJpegLoadFile(int width, int height, char* filepath, unsigned int flags)
{
    ITUSurface* surf = NULL;
    FILE* f = NULL;
    int size = 0;
    uint8_t* data = NULL;
    struct stat sb;

    assert(filepath);

    f = fopen(filepath, "rb");
    if (!f)
        goto end;

    if (fstat(fileno(f), &sb) == -1)
        goto end;

    size = sb.st_size;

    data = malloc(size);
    if (!data)
        goto end;

    size = fread(data, 1, size, f);
printf("+ituJpegLoad(%d,%d,0x%X,%d)\n", width, height, data, size);
    surf = ituJpegLoad(width, height, data, size, flags);
printf("-ituJpegLoad\n");
end:
    free(data);

    if (f)
        fclose(f);

    return surf;
}

#else
ITUSurface *ituJpegLoad(int width, int height, uint8_t *data, int size, unsigned int flags)
{
    uint8_t                       *src  = NULL;
    uint16_t                      *dest = NULL;
    ITUSurface                    *surf = NULL;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr         jerr;
    int                           w, h, x, y;

    assert(data);
    assert(size > 0);

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data, size);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    if (cinfo.output_components != 3)
        goto end;

    src = malloc(cinfo.output_width * cinfo.output_components);
    if (!src)
        goto end;

    y    = 0;

    if (width == 0 || height == 0)
    {
        w = (int)cinfo.output_width;
        h = (int)cinfo.output_height;
    }
    else
    {
        w = width < (int)cinfo.output_width ? width : cinfo.output_width;
        h = height < (int)cinfo.output_height ? height : cinfo.output_height;
    }

    surf = ituCreateSurface(w, h, 0, ITU_RGB565, NULL, 0);
    if (!surf)
        goto end;

    dest = (uint16_t *)ituLockSurface(surf, 0, 0, w, h);
    assert(dest);

    while ((int)cinfo.output_scanline < h)
    {
        jpeg_read_scanlines(&cinfo, &src, 1);

        for (x = 0; x < w; x++)
        {
            dest[x + y * w] = ITH_RGB565(src[x * 3], src[x * 3 + 1], src[x * 3 + 2]);
        }
        y++;
    }

    jpeg_destroy_decompress(&cinfo);
    ituUnlockSurface(surf);

end:
    if (src)
        free(src);

    return surf;
}

ITUSurface* ituJpegAlphaLoad(int width, int height, uint8_t* alpha, uint8_t* data, int size)
{
    uint8_t                       *src = NULL;
    uint32_t                      *dest = NULL;
    ITUSurface                    *surf = NULL;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr         jerr;
    int                           w, h, x, y;

    assert(data);
    assert(size > 0);

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data, size);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    if (cinfo.output_components != 3)
        goto end;

    src = malloc(cinfo.output_width * cinfo.output_components);
    if (!src)
        goto end;

    y = 0;
    w = width;
    h = height;

    surf = ituCreateSurface(w, h, 0, ITU_ARGB8888, NULL, 0);
    if (!surf)
        goto end;

    dest = (uint32_t *)ituLockSurface(surf, 0, 0, w, h);
    assert(dest);

    while ((int)cinfo.output_scanline < h)
    {
        jpeg_read_scanlines(&cinfo, &src, 1);

        for (x = 0; x < w; x++)
        {
            dest[x + y * w] = ITH_ARGB8888(*alpha++, src[x * 3], src[x * 3 + 1], src[x * 3 + 2]);
        }
        y++;
    }

    jpeg_destroy_decompress(&cinfo);
    ituUnlockSurface(surf);

end:
    if (src)
        free(src);

    return surf;
}

ITUSurface *ituJpegLoadFile(int width, int height, char* filepath, unsigned int flags)
{
    uint8_t                       *src  = NULL;
    uint16_t                      *dest = NULL;
    ITUSurface                    *surf = NULL;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr         jerr;
    int                           w, h, x, y;
    FILE*                         f = NULL;

    assert(filepath);

    f = fopen(filepath, "rb");
    if (!f)
        goto end;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, f);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    if (cinfo.output_components != 3)
        goto end;

    src = malloc(cinfo.output_width * cinfo.output_components * cinfo.output_height);
    if (!src)
        goto end;

    while (cinfo.output_scanline < cinfo.output_height)
    {
        uint8_t* rowp[1];
        rowp[0] = src + cinfo.output_scanline * cinfo.output_width * cinfo.output_components;
        jpeg_read_scanlines(&cinfo, rowp, 1);
    }

    if (width == 0 || height == 0)
    {
        w = (int)cinfo.output_width;
        h = (int)cinfo.output_height;
    }
    else
    {
        w = width < (int)cinfo.output_width ? width : cinfo.output_width;
        h = height < (int)cinfo.output_height ? height : cinfo.output_height;
    }

    surf = ituCreateSurface(w, h, 0, ITU_RGB565, NULL, 0);
    if (!surf)
        goto end;

    dest = (uint16_t *)ituLockSurface(surf, 0, 0, w, h);
    assert(dest);

    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            int xx = x * cinfo.output_width / w;
            int yy = y * cinfo.output_height / h;
            int index = cinfo.output_width * yy + xx;
            dest[w * y + x] = ITH_RGB565(src[index * 3], src[index * 3 + 1], src[index * 3 + 2]);
        }
    }

    jpeg_destroy_decompress(&cinfo);
    ituUnlockSurface(surf);

end:
    if (src)
        free(src);

    if (f)
        fclose(f);

    return surf;
}
#endif // !defined(_WIN32) && defined(CFG_JPEG_HW_ENABLE)
