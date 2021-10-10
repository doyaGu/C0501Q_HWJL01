
#include "globle.h"
#include "test_items.h"
#include "ite_jpg.h"

#if (CFG_CHIP_FAMILY == 9920)
#include "it9920/jpg_reg.h"
#elif (CFG_CHIP_FAMILY == 9850)
#include "it9850/jpg_reg.h"
#else
#include "it9070/jpg_reg.h"
#endif


//=============================================================================
//				  Constant Definition
//=============================================================================

//=============================================================================
//				  Macro Definition
//=============================================================================

//=============================================================================
//				  Structure Definition
//=============================================================================

//=============================================================================
//				  Global Data Definition
//=============================================================================

//=============================================================================
//				  Private Function Definition
//=============================================================================
static int
_reset_stream_info(
    JPG_STREAM_HANDLE   *pHJStream,
    void		        *extraData)
{
    return 0;
}

//=============================================================================
//				  Public Function Definition
//=============================================================================
void
test_jpeg_dec_withIsp(
    unsigned char   *jpegStream,
    unsigned int    streamLength,
    char            *filename)
{
    HJPG                *pHJpeg = 0;
    JPG_INIT_PARAM      initParam = {0};
    JPG_STREAM_INFO     inStreamInfo = {0};
    JPG_STREAM_INFO     outStreamInfo = {0};
    JPG_BUF_INFO        entropyBufInfo = {0};
    JPG_USER_INFO       jpgUserInfo = {0};
    uint8_t             *pJStrmBuf = 0;
    uint32_t            JStrmSize = 0;
    JPG_RECT            destRect = {0};

    struct timeval currT = {0};
    gettimeofday(&currT, NULL);
    srand(currT.tv_usec);

    initParam.codecType     = JPG_CODEC_DEC_JPG;
    initParam.decType       = JPG_DEC_PRIMARY; //JPG_DEC_SMALL_THUMB; //JPG_DEC_PRIMARY;
    initParam.outColorSpace = JPG_COLOR_SPACE_RGB565;
    initParam.width         = _Get_Lcd_Width();
    initParam.height        = _Get_Lcd_Height();
    initParam.dispMode      = JPG_DISP_CENTER;

    iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

    if( filename )
    {
        inStreamInfo.streamType   = JPG_STREAM_FILE;
        inStreamInfo.jstream.path = (void*)filename;
    }
    else
    {
        inStreamInfo.streamType = JPG_STREAM_MEM;
        inStreamInfo.jstream.mem[0].pAddr  = jpegStream;
        inStreamInfo.jstream.mem[0].length = streamLength;
        inStreamInfo.validCompCnt = 1;
    }

#if 0//!(_MSC_VER)
    {
        FN_FILE     *fp = 0;

        if( (fp = f_open("./Canon_S2IS_007_LB.jpg", "r")) == 0 )
        {
            _err("open dump.jpg fail !!");
        }
        f_seek(fp, 0, FN_SEEK_END);
        JStrmSize = f_tell(fp);
        f_seek(fp, 0, FN_SEEK_SET);

        pJStrmBuf = malloc(JStrmSize);
        memset(pJStrmBuf, 0xaa, JStrmSize);
        f_read(pJStrmBuf, 1, JStrmSize, fp);
        f_close(fp);
        if( inStreamInfo.streamType == JPG_STREAM_MEM )
        {
            inStreamInfo.src.mem[0].pAddr  = pJStrmBuf;
            inStreamInfo.src.mem[0].length = JStrmSize;
        }

    }
#endif

    inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
    inStreamInfo.jpg_reset_stream_info =  _reset_stream_info;

    outStreamInfo.streamIOType       = JPG_STREAM_IO_WRITE;
    outStreamInfo.streamType         = JPG_STREAM_MEM;
    outStreamInfo.jstream.mem[0].pAddr  = (uint8_t*)_Get_Lcd_Addr_A();  // get output buf;
    outStreamInfo.jstream.mem[0].pitch  = _Get_Lcd_Pitch();
    outStreamInfo.jstream.mem[0].length = _Get_Lcd_Pitch() * _Get_Lcd_Height();
    outStreamInfo.validCompCnt = 1;

    iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, &outStreamInfo, 0);

    iteJpg_Parsing(pHJpeg, &entropyBufInfo, (void*)&destRect); //0);

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    printf("  (%d, %d) %dx%d, dispMode=%d\r\n",
                jpgUserInfo.jpgRect.x, jpgUserInfo.jpgRect.y,
                jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
                initParam.dispMode);
    printf("  LCD A=0x%x, LCD B=0x%x\n", _Get_Lcd_Addr_A(), _Get_Lcd_Addr_B());

    iteJpg_Setup(pHJpeg, 0);

    iteJpg_Process(pHJpeg, &entropyBufInfo, 0, 0);

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    printf("\n\tresult = %d\n", jpgUserInfo.status);

    iteJpg_DestroyHandle(&pHJpeg, 0);

    if( pJStrmBuf ) free(pJStrmBuf);
    return;
}

void
test_jpeg_enc(
    uint32_t        in_w,
    uint32_t        in_h,
    uint8_t         *pAddr_y,
    uint8_t         *pAddr_u,
    uint8_t         *pAddr_v,
    bool            bYuv422,
    char            *encName)
{
    HJPG                *pHJpeg = 0;
    JPG_INIT_PARAM      initParam = {0};
    JPG_STREAM_INFO     inStreamInfo = {0};
    JPG_STREAM_INFO     outStreamInfo = {0};
    JPG_BUF_INFO        entropyBufInfo = {0};
    JPG_USER_INFO       jpgUserInfo = {0};
    uint32_t            jpgEncSize = 0;

    struct timeval currT = {0};
    gettimeofday(&currT, NULL);
    srand(currT.tv_usec);

    // ------------------------------------------------------
    // encode
    initParam.codecType     = JPG_CODEC_ENC_JPG;
    initParam.outColorSpace = (bYuv422) ? JPG_COLOR_SPACE_YUV422 : JPG_COLOR_SPACE_YUV420;
    initParam.width         = in_w;
    initParam.height        = in_h;
    initParam.encQuality    = 85;

    iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

    inStreamInfo.streamIOType       = JPG_STREAM_IO_READ;
    inStreamInfo.streamType         = JPG_STREAM_MEM;
    // Y
    inStreamInfo.jstream.mem[0].pAddr  = (uint8_t*)pAddr_y;  // get output buf;
    inStreamInfo.jstream.mem[0].pitch  = in_w;
    //inStreamInfo.src.mem[0].length = _Get_Lcd_Width() * _Get_Lcd_Height();
    // U
    inStreamInfo.jstream.mem[1].pAddr  = (uint8_t*)pAddr_u;
    inStreamInfo.jstream.mem[1].pitch  = in_w/2;
    //inStreamInfo.src.mem[1].length = (_Get_Lcd_Width()/2) * _Get_Lcd_Height();
    // V
    inStreamInfo.jstream.mem[2].pAddr  = (uint8_t*)pAddr_v;
    inStreamInfo.jstream.mem[2].pitch  = in_w/2;
    //inStreamInfo.src.mem[2].length = (_Get_Lcd_Width()/2) * _Get_Lcd_Height();

    inStreamInfo.validCompCnt = 3;

    if( encName )
    {
        outStreamInfo.streamType = JPG_STREAM_FILE;
        outStreamInfo.jstream.path   = (void*)encName;
    }

    outStreamInfo.streamIOType          = JPG_STREAM_IO_WRITE;
    outStreamInfo.jpg_reset_stream_info =  _reset_stream_info;


    printf("\n\n\tencode input: Y=0x%x, u=0x%x, v=0x%x\n",
                inStreamInfo.jstream.mem[0].pAddr,
                inStreamInfo.jstream.mem[1].pAddr,
                inStreamInfo.jstream.mem[2].pAddr);

    iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, &outStreamInfo, 0);

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    //printf("  (%d, %d) %dx%d, dispMode=%d\r\n",
    //            jpgUserInfo.jpgRect.x, jpgUserInfo.jpgRect.y,
    //            jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
    //            initParam.dispMode);
    //printf("  LCD A=0x%x, LCD B=0x%x\n", _Get_Lcd_Addr_A(), _Get_Lcd_Addr_B());

    iteJpg_Setup(pHJpeg, 0);

    iteJpg_Process(pHJpeg, &entropyBufInfo, &jpgEncSize, 0);

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    printf("\n\tresult = %d, encode size = %f KB\n", jpgUserInfo.status, (float)jpgEncSize/1024);

    iteJpg_DestroyHandle(&pHJpeg, 0);
    return;
}


void
test_jpeg_enc_clip(
	uint32_t		start_x,
	uint32_t		start_y,
	uint32_t		in_w,
	uint32_t		in_h,
	uint32_t		pitch_Y,
	uint8_t 		*pAddr_y,
	uint8_t 		*pAddr_u,
	uint8_t 		*pAddr_v,
	char			*encName)

{
    HJPG                *pHJpeg = 0;
    JPG_INIT_PARAM      initParam = {0};
    JPG_STREAM_INFO     inStreamInfo = {0};
    JPG_STREAM_INFO     outStreamInfo = {0};
    JPG_BUF_INFO        entropyBufInfo = {0};
    JPG_USER_INFO       jpgUserInfo = {0};
    uint32_t            jpgEncSize = 0;
	uint32_t			OffsetAddrY = 0, OffsetAddrUV = 0;

    // ------------------------------------------------------
    // encode
    initParam.codecType     = JPG_CODEC_ENC_JPG;
    initParam.outColorSpace = JPG_COLOR_SPACE_YUV422;
    initParam.width         = in_w;
    initParam.height        = in_h;
    initParam.encQuality    = 85;

    iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

    inStreamInfo.streamIOType       = JPG_STREAM_IO_READ;
    inStreamInfo.streamType         = JPG_STREAM_MEM;

    // Y
    inStreamInfo.jstream.mem[0].pitch  = pitch_Y;
	
	OffsetAddrY = start_y * inStreamInfo.jstream.mem[0].pitch + start_x;
	inStreamInfo.jstream.mem[0].pAddr  = (uint8_t*)pAddr_y + OffsetAddrY;  // get output buf;

    // U
    if (ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 && ithIsTilingModeOn())
		inStreamInfo.jstream.mem[1].pitch  =  pitch_Y;
	else
	    inStreamInfo.jstream.mem[1].pitch  =  pitch_Y >> 1;
	
	OffsetAddrUV =  start_y * inStreamInfo.jstream.mem[1].pitch + start_x/2;
	inStreamInfo.jstream.mem[1].pAddr  = (uint8_t*)pAddr_u + OffsetAddrUV;

    // V
	if (ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 && ithIsTilingModeOn())
		inStreamInfo.jstream.mem[2].pitch  =  pitch_Y;
	else
	    inStreamInfo.jstream.mem[2].pitch  =  pitch_Y >> 1;

	OffsetAddrUV =	start_y * inStreamInfo.jstream.mem[2].pitch + start_x/2;
	inStreamInfo.jstream.mem[2].pAddr  = (uint8_t*)pAddr_v + OffsetAddrUV;

    inStreamInfo.validCompCnt = 3;

    if( encName )
    {
        outStreamInfo.streamType = JPG_STREAM_FILE;
        outStreamInfo.jstream.path   = (void*)encName;
    }

    outStreamInfo.streamIOType          = JPG_STREAM_IO_WRITE;
    outStreamInfo.jpg_reset_stream_info =  _reset_stream_info;


    printf("\n\n\tencode input: Y=0x%x, u=0x%x, v=0x%x\n",
                inStreamInfo.jstream.mem[0].pAddr,
                inStreamInfo.jstream.mem[1].pAddr,
                inStreamInfo.jstream.mem[2].pAddr);

    iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, &outStreamInfo, 0);

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    //printf("  (%d, %d) %dx%d, dispMode=%d\r\n",
    //            jpgUserInfo.jpgRect.x, jpgUserInfo.jpgRect.y,
    //            jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
    //            initParam.dispMode);
    //printf("  LCD A=0x%x, LCD B=0x%x\n", _Get_Lcd_Addr_A(), _Get_Lcd_Addr_B());

    iteJpg_Setup(pHJpeg, 0);

    iteJpg_Process(pHJpeg, &entropyBufInfo, &jpgEncSize, 0);

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    printf("\n\tresult = %d, encode size = %f KB\n", jpgUserInfo.status, (float)jpgEncSize/1024);

    iteJpg_DestroyHandle(&pHJpeg, 0);
    return;
}


void
test_jpeg_dec_to_yuv(
    unsigned char   *jpegStream,
    unsigned int    streamLength,
    char            *filename)
{
    JPG_ERR             jRst = JPG_ERR_OK;
    HJPG                *pHJpeg = 0;
    JPG_INIT_PARAM      initParam = {0};
    JPG_STREAM_INFO     inStreamInfo = {0};
    JPG_STREAM_INFO     outStreamInfo = {0};
    JPG_BUF_INFO        entropyBufInfo = {0};
    JPG_USER_INFO       jpgUserInfo = {0};
    JPG_RECT            destRect = {0}; //Benson add.

    uint8_t         *pY = 0, *pU = 0, *pV = 0;
	uint32_t        real_width     = 0, real_height = 0, real_height_ForTile = 0;
    uint8_t     	*WriteBuf = NULL; 
    uint8_t         *mappedSysRam = NULL;


    struct timeval currT;
    printf("test_jpeg_dec_to_yuv,jpegStream=0x%x , streamlen= %d ,filename=%s\n",jpegStream,streamLength,filename);


    // decode
    initParam.codecType     = JPG_CODEC_DEC_JPG_CMD;//JPG_CODEC_DEC_JPG;
    initParam.decType       = JPG_DEC_PRIMARY; //JPG_DEC_SMALL_THUMB; //JPG_DEC_PRIMARY;

    initParam.outColorSpace = JPG_COLOR_SPACE_YUV420;//JPG_COLOR_SPACE_ARGB4444;//JPG_COLOR_SPACE_ARGB8888;//JPG_COLOR_SPACE_RGB565;//JPG_COLOR_SPACE_YUV420;  //JPG_COLOR_SPACE_YUV422;
    initParam.width         = _Get_Lcd_Width();
    initParam.height        = _Get_Lcd_Height();
    initParam.dispMode      = JPG_DISP_CENTER;

    printf("initParam.width= %d ,initParam.height=%d ,ithLcdGetWidth()=%d\n",initParam.width,initParam.height,ithLcdGetWidth());
    iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

    inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
    inStreamInfo.streamType            = JPG_STREAM_MEM;
    inStreamInfo.jstream.mem[0].pAddr  = jpegStream;
    inStreamInfo.jstream.mem[0].length = streamLength;
    inStreamInfo.validCompCnt = 1;

    if( (jRst = iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, 0, 0)) )
    {
        printf(" err 0x%x !! %s[%u]\n", __FUNCTION__, __LINE__);
        while(1);
    }

    //if( (jRst = iteJpg_Parsing(pHJpeg, &entropyBufInfo, 0)) )
    if( (jRst = iteJpg_Parsing(pHJpeg, &entropyBufInfo,  (void*)&destRect)) )
    {
        printf(" err 0x%x !! %s[%u]\n", __FUNCTION__, __LINE__);
        while(1);
    }

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    printf("  disp(%ux%u), dispMode=%d, real(%ux%u), img(%ux%u), slice=%u, pitch(%u, %u)\r\n",
                jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
                initParam.dispMode,
                jpgUserInfo.real_width, jpgUserInfo.real_height,
                jpgUserInfo.imgWidth, jpgUserInfo.imgHeight,
                jpgUserInfo.slice_num,
                jpgUserInfo.comp1Pitch, jpgUserInfo.comp23Pitch);

    real_width  = jpgUserInfo.real_width;
    real_height = jpgUserInfo.real_height;

    outStreamInfo.streamIOType = JPG_STREAM_IO_WRITE;
    outStreamInfo.streamType   = JPG_STREAM_MEM;
    // Y
    #if (CFG_CHIP_FAMILY != 9920)
		   if (ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 && ithIsTilingModeOn())
	   {
 		   real_width = jpgUserInfo.comp1Pitch;
		   real_height_ForTile	= (real_height + 0x7f)/ 0x80;
		   pY = malloc(jpgUserInfo.comp1Pitch * 128 * real_height_ForTile * 3);
		   printf("pY=0x%x\n",pY);
		   if(!pY) 
		   {
			   printf("JPG Allocate Line buf fail !!\n");
			   while(1);  
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
			   while(1);  
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
	#else
	
    pY = malloc(real_width * real_height * 4);  //forARGB8888 format. 
    memset(pY, 0x0, real_width * real_height * 4);

	// Y
	switch(initParam.outColorSpace)
	{
		case JPG_COLOR_SPACE_RGB565:
			outStreamInfo.jstream.mem[0].pAddr  = _Get_Lcd_Addr_A();
    		outStreamInfo.jstream.mem[0].pitch  = (uint16_t)_Get_Lcd_Pitch();
    		outStreamInfo.jstream.mem[0].length =  _Get_Lcd_Width()* _Get_Lcd_Height()*2;
		break;

		case JPG_COLOR_SPACE_ARGB4444:
			outStreamInfo.jstream.mem[0].pAddr  = (uint8_t*)pY; // get output buf;
    		outStreamInfo.jstream.mem[0].pitch  = jpgUserInfo.comp1Pitch * 2;
    		outStreamInfo.jstream.mem[0].length = real_width * real_height * 2;
		break;

		case JPG_COLOR_SPACE_ARGB8888:
			outStreamInfo.jstream.mem[0].pAddr  = (uint8_t*)pY;
    		outStreamInfo.jstream.mem[0].pitch  = jpgUserInfo.comp1Pitch * 4;
    		outStreamInfo.jstream.mem[0].length = real_width * real_height * 4;
		break;

		default:
			outStreamInfo.jstream.mem[0].pAddr  = (uint8_t*)pY;
    		outStreamInfo.jstream.mem[0].pitch  = jpgUserInfo.comp1Pitch;
    		outStreamInfo.jstream.mem[0].length = real_width * real_height;
		break;
	}

    // U
    outStreamInfo.jstream.mem[1].pAddr  = (uint8_t*)(outStreamInfo.jstream.mem[0].pAddr + outStreamInfo.jstream.mem[0].length);
    outStreamInfo.jstream.mem[1].pitch  = jpgUserInfo.comp23Pitch;
    outStreamInfo.jstream.mem[1].length = outStreamInfo.jstream.mem[1].pitch * real_height;
    // V
    outStreamInfo.jstream.mem[2].pAddr  = (uint8_t*)(outStreamInfo.jstream.mem[1].pAddr + outStreamInfo.jstream.mem[1].length);
    outStreamInfo.jstream.mem[2].pitch  = jpgUserInfo.comp23Pitch;
    outStreamInfo.jstream.mem[2].length = outStreamInfo.jstream.mem[2].pitch * real_height;
    outStreamInfo.validCompCnt = 3;
	#endif

    printf("\n\tY=0x%x, u=0x%x, v=0x%x\n",
                outStreamInfo.jstream.mem[0].pAddr,
                outStreamInfo.jstream.mem[1].pAddr,
                outStreamInfo.jstream.mem[2].pAddr);

    if( (jRst = iteJpg_SetStreamInfo(pHJpeg, 0, &outStreamInfo, 0)) )
    {
        printf(" err 0x%x !! %s[%u]\n", __FUNCTION__, __LINE__);
        while(1);
    }
    printf("  LCD A=0x%x, LCD B=0x%x\n", _Get_Lcd_Addr_A(), _Get_Lcd_Addr_B());

    if( (jRst = iteJpg_Setup(pHJpeg, 0)) )
    {
        printf(" err 0x%x !! %s[%u]\n", __FUNCTION__, __LINE__);
        while(1);
    }

    WriteBuf = (uint8_t *)itpVmemAlloc((real_width * real_height * 3 ) );
    mappedSysRam = ithMapVram((uint32_t)WriteBuf, (real_width * real_height * 3) , ITH_VRAM_WRITE);
	memcpy(mappedSysRam, pY, (real_width * real_height * 3) );
	ithUnmapVram(mappedSysRam, (real_width * real_height * 3 ) );
	ithFlushDCacheRange(mappedSysRam, (real_width * real_height * 3 ) );
	ithFlushMemBuffer();

    //ithFlushDCacheRange((void*)pY, real_width * real_height * 3);
    //ithFlushMemBuffer();

    if( (jRst = iteJpg_Process(pHJpeg, &entropyBufInfo, 0, 0)) )
    {
        printf(" err 0x%x !! %s[%u]\n", __FUNCTION__, __LINE__);
        while(1);
    }

    if (WriteBuf)
    {
        itpVmemFree((uint32_t)WriteBuf);
    }

    //iteJpg_WaitIdle(pHJpeg, 0);
    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    printf("\n\tresult = %d\n", jpgUserInfo.status);

    iteJpg_DestroyHandle(&pHJpeg, 0);

    {
        CLIP_WND_INFO   clipInfo = {0};
        BASE_RECT       srcRect = {0};
        DATA_COLOR_TYPE     colorType = 0;

        clipInfo.bClipEnable  = 0;

        srcRect.w = real_width;
        srcRect.h = real_height;

        //printf("  ** jpgUserInfo.colorFormate=0x%x\n", jpgUserInfo.colorFormate);
        switch( jpgUserInfo.colorFormate )
        {
			case JPG_COLOR_SPACE_YUV444:    colorType = DATA_COLOR_YUV444;break;
            case JPG_COLOR_SPACE_YUV422:    colorType = DATA_COLOR_YUV422;break;
            case JPG_COLOR_SPACE_YUV420:    colorType = DATA_COLOR_YUV420;break;
            case JPG_COLOR_SPACE_YUV422R:   colorType = DATA_COLOR_YUV422R;break;
			case JPG_COLOR_SPACE_RGB565:    colorType = DATA_COLOR_RGB565;break;
			case JPG_COLOR_SPACE_ARGB8888:  colorType = DATA_COLOR_ARGB8888;break;
			case JPG_COLOR_SPACE_ARGB4444:  colorType = DATA_COLOR_ARGB4444;break;
        }

        test_isp_colorTrans(
            outStreamInfo.jstream.mem[0].pAddr,
            outStreamInfo.jstream.mem[1].pAddr,
            outStreamInfo.jstream.mem[2].pAddr,
            colorType,
            &clipInfo,
            &srcRect);
    }

    if( pY )    free(pY);
    return;

}

void
test_jpeg_dec_enc(
    char            *decName,
    char            *encName)
{
    HJPG                *pHJpeg = 0;
    JPG_INIT_PARAM      initParam = {0};
    JPG_STREAM_INFO     inStreamInfo = {0};
    JPG_STREAM_INFO     outStreamInfo = {0};
    JPG_BUF_INFO        entropyBufInfo = {0};
    JPG_USER_INFO       jpgUserInfo = {0};

    struct timeval currT = {0};
    gettimeofday(&currT, NULL);
    srand(currT.tv_usec);

    // isp output yuv420/yuv422 fail
    return;



    // decode
    initParam.codecType     = JPG_CODEC_DEC_JPG;
    initParam.decType       = JPG_DEC_PRIMARY; //JPG_DEC_SMALL_THUMB; //JPG_DEC_PRIMARY;
    initParam.outColorSpace = JPG_COLOR_SPACE_YUV420; //JPG_COLOR_SPACE_YUV420; //JPG_COLOR_SPACE_YUV422;
    initParam.width         = _Get_Lcd_Width();
    initParam.height        = _Get_Lcd_Height();
    initParam.dispMode      = JPG_DISP_CENTER;

    iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

    if( decName )
    {
        inStreamInfo.streamType = JPG_STREAM_FILE;
        inStreamInfo.jstream.path   = (void*)decName;
    }

    inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
    inStreamInfo.jpg_reset_stream_info =  _reset_stream_info;

    outStreamInfo.streamIOType       = JPG_STREAM_IO_WRITE;
    outStreamInfo.streamType         = JPG_STREAM_MEM;
    // Y
    outStreamInfo.jstream.mem[0].pAddr  = (uint8_t*)_Get_Lcd_Addr_A();  // get output buf;
    //outStreamInfo.dest.mem[0].pitch  = _Get_Lcd_Pitch();
    //outStreamInfo.dest.mem[0].length = _Get_Lcd_Pitch() * _Get_Lcd_Height();
    outStreamInfo.jstream.mem[0].pitch  = _Get_Lcd_Width();
    outStreamInfo.jstream.mem[0].length = _Get_Lcd_Width() * _Get_Lcd_Height();
    // U
    outStreamInfo.jstream.mem[1].pAddr  = (uint8_t*)(outStreamInfo.jstream.mem[0].pAddr + outStreamInfo.jstream.mem[0].length);
    outStreamInfo.jstream.mem[1].pitch  = _Get_Lcd_Width()/2;
    outStreamInfo.jstream.mem[1].length = outStreamInfo.jstream.mem[1].pitch * _Get_Lcd_Height();
    // V
    outStreamInfo.jstream.mem[2].pAddr  = (uint8_t*)(outStreamInfo.jstream.mem[1].pAddr + outStreamInfo.jstream.mem[1].length);
    outStreamInfo.jstream.mem[2].pitch  = _Get_Lcd_Width()/2;
    outStreamInfo.jstream.mem[2].length = outStreamInfo.jstream.mem[2].pitch * _Get_Lcd_Height();
    outStreamInfo.validCompCnt = 3;

    printf("\n\n\tY=0x%x, u=0x%x, v=0x%x\n",
                outStreamInfo.jstream.mem[0].pAddr,
                outStreamInfo.jstream.mem[1].pAddr,
                outStreamInfo.jstream.mem[2].pAddr);

    iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, &outStreamInfo, 0);

    iteJpg_Parsing(pHJpeg, &entropyBufInfo, 0);

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    //printf("  (%d, %d) %dx%d, dispMode=%d\r\n",
    //            jpgUserInfo.jpgRect.x, jpgUserInfo.jpgRect.y,
    //            jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
    //            initParam.dispMode);
    //printf("  LCD A=0x%x, LCD B=0x%x\n", _Get_Lcd_Addr_A(), _Get_Lcd_Addr_B());

    iteJpg_Setup(pHJpeg, 0);

    iteJpg_Process(pHJpeg, &entropyBufInfo, 0, 0);

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    printf("\n\tresult = %d\n", jpgUserInfo.status);

    iteJpg_DestroyHandle(&pHJpeg, 0);

    return;
    // ------------------------------------------------------
    // encode
    initParam.codecType     = JPG_CODEC_ENC_JPG;
    initParam.outColorSpace = JPG_COLOR_SPACE_YUV420; //JPG_COLOR_SPACE_YUV422;
    initParam.width         = _Get_Lcd_Width();
    initParam.height        = _Get_Lcd_Height();
    initParam.encQuality    = 85;

    iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

    inStreamInfo.streamIOType       = JPG_STREAM_IO_READ;
    inStreamInfo.streamType         = JPG_STREAM_MEM;
    // Y
    inStreamInfo.jstream.mem[0].pAddr  = (uint8_t*)_Get_Lcd_Addr_A();  // get output buf;
    inStreamInfo.jstream.mem[0].length = _Get_Lcd_Width() * _Get_Lcd_Height();
    // U
    inStreamInfo.jstream.mem[1].pAddr  = (uint8_t*)(inStreamInfo.jstream.mem[0].pAddr + inStreamInfo.jstream.mem[0].length);
    inStreamInfo.jstream.mem[1].length = (_Get_Lcd_Width()/2) * _Get_Lcd_Height();
    // V
    inStreamInfo.jstream.mem[2].pAddr  = (uint8_t*)(inStreamInfo.jstream.mem[1].pAddr + inStreamInfo.jstream.mem[1].length);
    inStreamInfo.jstream.mem[2].length = (_Get_Lcd_Width()/2) * _Get_Lcd_Height();

    inStreamInfo.validCompCnt = 3;

    if( encName )
    {
        outStreamInfo.streamType = JPG_STREAM_FILE;
        outStreamInfo.jstream.path   = (void*)encName;
    }

    outStreamInfo.streamIOType          = JPG_STREAM_IO_WRITE;
    outStreamInfo.jpg_reset_stream_info =  _reset_stream_info;


    printf("\n\n\tencode input: Y=0x%x, u=0x%x, v=0x%x\n",
                inStreamInfo.jstream.mem[0].pAddr,
                inStreamInfo.jstream.mem[1].pAddr,
                inStreamInfo.jstream.mem[2].pAddr);

    iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, &outStreamInfo, 0);

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    //printf("  (%d, %d) %dx%d, dispMode=%d\r\n",
    //            jpgUserInfo.jpgRect.x, jpgUserInfo.jpgRect.y,
    //            jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
    //            initParam.dispMode);
    //printf("  LCD A=0x%x, LCD B=0x%x\n", _Get_Lcd_Addr_A(), _Get_Lcd_Addr_B());

    iteJpg_Setup(pHJpeg, 0);

    iteJpg_Process(pHJpeg, &entropyBufInfo, 0, 0);

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    printf("\n\tresult = %d\n", jpgUserInfo.status);

    iteJpg_DestroyHandle(&pHJpeg, 0);
    return;
}


