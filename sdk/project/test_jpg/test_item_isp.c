#include "test_item_isp.h"
#include "ite/itv.h"

#if (CFG_CHIP_FAMILY == 9920)
	#include "isp/mmp_isp_9920.h"
#else
	#include "isp/mmp_isp.h"
#endif

//=============================================================================
//				  Constant Definition
//=============================================================================
#if (CFG_CHIP_FAMILY == 9920)
typedef struct tagFrameFuncInfo {   
    void             *vramAddr;
    MMP_UINT         startX;
    MMP_UINT         startY;
    MMP_UINT         width;
    MMP_UINT         height;
    MMP_UINT         pitch;
    MMP_UINT32       linebytes;
    MMP_UINT32       bitstreambytes;
    MMP_UINT         colorKeyR;
    MMP_UINT         colorKeyG;
    MMP_UINT         colorKeyB;
    MMP_BOOL         EnAlphaBlend;
    MMP_UINT         constantAlpha;
    MMP_PIXEL_FORMAT format;
    MMP_UINT         uiBufferIndex;
} FrameFuncInfo; //For Jpeg With FrameFunction Use.
#endif
//=============================================================================
//				  Macro Definition
//=============================================================================

//=============================================================================
//				  Structure Definition
//=============================================================================

//=============================================================================
//				  Global Data Definition
//=============================================================================
unsigned int g_bOffOtherFF = 1;      // ctrl use 2 frame function or not
ISP_DEVICE   gIspDev;

//=============================================================================
//				  Private Function Definition
//=============================================================================

//=============================================================================
//				  Public Function Definition
//=============================================================================
void
test_isp_colorTrans(
    uint8_t         *srcAddr_rgby,
    uint8_t         *srcAddr_u,
    uint8_t         *srcAddr_v,
    DATA_COLOR_TYPE colorType,
    CLIP_WND_INFO   *clipInfo,
    BASE_RECT       *srcRect)
{
    int                 result   = 0, OutputWidth = 0, OutputHeight = 0;
    MMP_ISP_OUTPUT_INFO outInfo  = {0};
    MMP_ISP_SHARE       ispInput = {0};
	FILE  *f = 0;
	uint32_t gvideo_vram_addr = 0;
	uint8_t  *gvideo_sys_addr  = NULL;
	int size = 0;
#if (CFG_CHIP_FAMILY == 9920)
    MMP_ISP_CORE_INFO   ISPCOREINFO = { 0 };
#endif

    ispInput.width        = srcRect->w;
    ispInput.height       = srcRect->h;
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
            ispInput.pitchUv = (srcRect->w >> 1);
            break;

        case DATA_COLOR_YUV422R:
            ispInput.format  = MMP_ISP_IN_YUV422R;
            ispInput.pitchY  = srcRect->w;
            ispInput.pitchUv = srcRect->w;
            break;

        case DATA_COLOR_YUV420:
            ispInput.format  = MMP_ISP_IN_YUV420;
            ispInput.pitchY  = srcRect->w;
            ispInput.pitchUv = (srcRect->w >> 1);
            break;
        }
        break;

    case DATA_COLOR_ARGB8888:
    case DATA_COLOR_ARGB4444:
        size = _Get_Lcd_Width() * _Get_Lcd_Height() * 2;
		gvideo_vram_addr = itpVmemAlloc(size);
		gvideo_sys_addr = ithMapVram((uint32_t)gvideo_vram_addr, size , ITH_VRAM_WRITE);
		if(!gvideo_vram_addr) printf("open fail!\n");
		f = fopen("d:/800x480_565.raw", "rb"); //Loading a Pic(800x480 RGB565) to be the background picture.
		if(f)
		{
			fread(gvideo_sys_addr, 1 , size ,f);
			fclose(f);
		}
		else
			printf("open Y file fail!\n");
		ithUnmapVram(gvideo_sys_addr ,size);
		
        ispInput.addrY = (uint32_t)gvideo_vram_addr;
		ispInput.format = MMP_ISP_IN_RGB565;
        ispInput.pitchY = _Get_Lcd_Pitch();
		ispInput.width  = _Get_Lcd_Width();
    	ispInput.height = _Get_Lcd_Height();
    	ispInput.isAdobe_CMYK = 0;
        break;

    case DATA_COLOR_NV12:
    case DATA_COLOR_NV21:
        ispInput.addrY   = (uint32_t)srcAddr_rgby;
        ispInput.addrU   = (uint32_t)srcAddr_u;
        ispInput.pitchY  = srcRect->w;
        ispInput.pitchUv = srcRect->w;
        ispInput.format  = (colorType == DATA_COLOR_NV12) ? MMP_ISP_IN_NV12 : MMP_ISP_IN_NV21;
        break;

	case DATA_COLOR_RGB565:
        //goto end; //9920  Command trigger mode don`t need to use ISP when you set to output RGB565 format.
        return;
		
    }

    // ---------------------------------------
    // initial isp
#if (CFG_CHIP_FAMILY == 9920)
	mmpIspTerminate(&gIspDev);
    result = mmpIspInitialize(&gIspDev, MMP_ISP_CORE_0);
    if (result)
        printf("mmpIspInitialize() error (0x%x) !!\n", result);

	// for VP1
	ISPCOREINFO.EnPreview = false;
	ISPCOREINFO.PreScaleSel = MMP_ISP_PRESCALE_NORMAL;
	//end of for VP1.

    result = mmpIspSetCore(gIspDev, &ISPCOREINFO);
    if (result)
        printf("mmpIspSetCore() error (0x%x) !!\n", result);

    result = mmpIspSetMode(gIspDev, MMP_ISP_MODE_PLAY_VIDEO);
    if (result)
        printf("mmpIspSetMode() error (0x%x) !! \n", result);

 	outInfo.startX = 0;
    outInfo.startY = 0;
    outInfo.width   = _Get_Lcd_Width();
    outInfo.height  = _Get_Lcd_Height();
    outInfo.addrRGB = _Get_Lcd_Addr_A();
    outInfo.format =  MMP_ISP_OUT_DITHER565A;
	
    outInfo.pitchRGB = (uint16_t)_Get_Lcd_Pitch();
	mmpIspSetOutputWindow(gIspDev, &outInfo);
    mmpIspSetVideoWindow(gIspDev, 0, 0, outInfo.width, outInfo.height);

    if ((colorType == DATA_COLOR_ARGB8888) || (colorType == DATA_COLOR_ARGB4444)) 
    {
		int IFrm0Mode =0;
		if(colorType == DATA_COLOR_ARGB8888)
			IFrm0Mode = 2;
		else if(colorType == DATA_COLOR_ARGB4444)
			IFrm0Mode = 1;
		
        mmpIspEnable(gIspDev, MMP_ISP_FRAME_FUNCTION_0);
        FrameFuncInfo FF0 = {0};
        FF0.vramAddr = srcAddr_rgby;
        FF0.startX = 0;
        FF0.startY = 0;
        FF0.width = srcRect->w;
        FF0.height = srcRect->h;
        FF0.colorKeyR = 100;
        FF0.colorKeyG = 100;
        FF0.colorKeyB = 100;
        FF0.EnAlphaBlend = 1;
        FF0.constantAlpha = 0;
        if(IFrm0Mode == 0)
        {
            FF0.format = MMP_PIXEL_FORMAT_RGB565;
			FF0.pitch = srcRect->w;
        }
        else if(IFrm0Mode == 1)
        {
            FF0.format = MMP_PIXEL_FORMAT_ARGB4444;
			FF0.pitch = srcRect->w*2;
        }
        else if(IFrm0Mode == 2)
        {
            FF0.format = MMP_PIXEL_FORMAT_ARGB8888;
			FF0.pitch = srcRect->w*4;
        }

         result = mmpIspSetFrameFunction(
                    gIspDev,
                    MMP_ISP_FRAME_FUNCTION_0,
                    FF0.vramAddr,
                    FF0.startX,
                    FF0.startY,
                    FF0.width,
                    FF0.height,
                    FF0.pitch,
                    FF0.colorKeyR,
                    FF0.colorKeyG,
                    FF0.colorKeyB,
                    FF0.EnAlphaBlend,
                    FF0.constantAlpha,
                    FF0.format,
                    FF0.uiBufferIndex);
            if (result)
                printf("mmpIspSetFrameFunction() error (0x%x) !!\n", result);
    }

    result = mmpIspPlayImageProcess(gIspDev, &ispInput);
    if (result)
        printf("mmpIspPlayImageProcess() error (0x%x) !!\n", result);

    result = mmpIspWaitEngineIdle(gIspDev);
    if (result)
        printf("mmpIspWaitEngineIdle() error (0x%x) !!\n", result);
#else

 	mmpIspTerminate(&gIspDev);
    result = mmpIspInitialize(&gIspDev);
	if (result)
    	_err("mmpIspInitialize() error (0x%x) !!\n", result);
	#if (CFG_CHIP_FAMILY == 9850)
   		result = mmpIspSetMode(gIspDev, MMP_ISP_MODE_JPEG_TRANSFORM);
	#else
   		result = mmpIspSetMode(gIspDev, MMP_ISP_MODE_TRANSFORM);
	#endif
	
	 if (result)
        _err("mmpIspSetMode() error (0x%x) !! \n", result);
	 
    OutputWidth  = _Get_Lcd_Width();
    OutputHeight = _Get_Lcd_Height();
    mmpIspSetDisplayWindow(gIspDev, 0, 0, OutputWidth, OutputHeight);
    mmpIspSetVideoWindow(gIspDev, 0, 0, OutputWidth, OutputHeight);
    mmpIspSetOutputFormat(gIspDev, MMP_ISP_OUT_DITHER565);

	 // set isp output to LCD Buffer
	#if 0
    outInfo.startX = 0;
    outInfo.startY = 0;
    outInfo.width  = _Get_Lcd_Width();
    outInfo.height = _Get_Lcd_Height();
    result         = mmpIspSetDisplayWindow(
        outInfo.startX, outInfo.startY,
        outInfo.width, outInfo.height);
	#else
    outInfo.startX   = 0;
    outInfo.startY   = 0;
    outInfo.width    = _Get_Lcd_Width();
    outInfo.height   = _Get_Lcd_Height();
    outInfo.addrRGB  = _Get_Lcd_Addr_A();
	outInfo.pitchRGB = (uint16_t)_Get_Lcd_Pitch();
    outInfo.format   = MMP_ISP_OUT_DITHER565;
    mmpIspSetOutputWindow(gIspDev, &outInfo);
	//printf("outInfo.width = %d ,outInfo.height=%d ,outInfo.addrRGB=0x%x\n",outInfo.width,outInfo.height ,outInfo.addrRGB);
	#endif
    if (result)
        _err("mmpIspSetDisplayWindow() error (0x%x) !!\n", result);

    // set clip window info
    if (clipInfo && clipInfo->bClipEnable)
    {
        BASE_RECT rect = {0, 50, 100, 150};
        if (clipInfo->bClipOutside == 0)
            Draw_Rect((uint8_t *)_Get_Lcd_Addr_A(), _Get_Lcd_Pitch(), &rect, ITH_RGB565(0xff, 0, 0xff));

        switch (clipInfo->clipWndId)
        {
        case 0:  mmpIspEnable(gIspDev, MMP_ISP_CLIPPING0);   break;
        case 1:  mmpIspEnable(gIspDev, MMP_ISP_CLIPPING1);   break;
        case 2:  mmpIspEnable(gIspDev, MMP_ISP_CLIPPING2);   break;
        }

        mmpIspSetClipWindow(
            gIspDev,
            clipInfo->clipWndId, clipInfo->bClipOutside,
            clipInfo->clipRect.x, clipInfo->clipRect.y,
            clipInfo->clipRect.w, clipInfo->clipRect.h);
    }

    // set isp source info and fire
    result = mmpIspPlayImageProcess(gIspDev, &ispInput);
    if (result)
        _err("mmpIspPlayImageProcess() error (0x%x) !!\n", result);
	result = mmpIspWaitEngineIdle(); //mmpIspTerminate();
	if (result)
        _err("mmpIspTerminate() error (0x%x) !!\n", result);
#endif
}