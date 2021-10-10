#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "itu_cfg.h"
#include "ite/ith.h"
#include "ite/itu.h"
#include "ite/itp.h"

#if defined(CFG_M2D_ENABLE)
    #include "gfx/gfx.h"
#endif

#if defined(CFG_VIDEO_ENABLE) && !defined(CFG_FFMPEG_H264_SW)
    #include "ite/itv.h"
    #include "ite/ith_video.h"
#endif
#include "itu_private.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MAX_UI_BUFFER_COUNT    2
//#if defined(CFG_VIDEO_ENABLE) && !defined(CFG_FFMPEG_H264_SW)
//    #define FRAME_BUFFER_COUNT 4
//#endif
#define VIDEO_SURFACE_COUNT    2
//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Static Data Definition
//=============================================================================
static bool     g_inited;
static uint32_t g_vid_buff_addr[VIDEO_SURFACE_COUNT];
ITUSurface      *VideoSurf[VIDEO_SURFACE_COUNT];

//=============================================================================
//                              Private Function Declaration
//=============================================================================
#if defined(CFG_VIDEO_ENABLE) && !defined(CFG_FFMPEG_H264_SW)
static void VideoInit(void);
static void VideoExit(void);
#endif // CFG_VIDEO_ENABLE

//=============================================================================
//                              Public Function Definition
//=============================================================================
void ituFrameFuncInit(void)
{
#ifndef CFG_FFMPEG_H264_SW
    int        width;
    int        height;
    int        pitch;
    int        i;
	ITURotation     rot;
#endif
    
    if (g_inited)
        return;
        
#ifndef CFG_FFMPEG_H264_SW

    rot = itv_get_rotation();
    
    switch (rot)
    {
    case ITU_ROT_90:
    case ITU_ROT_270:
        width  = ithLcdGetHeight();
        height = ithLcdGetWidth();
        pitch  = ithLcdGetPitch() * ithLcdGetHeight() / ithLcdGetWidth();
        break;
        
    default:
        width  = ithLcdGetWidth();
        height = ithLcdGetHeight();
        pitch  = ithLcdGetPitch();
        break;
    }

    if (!g_vid_buff_addr[0])
    {        
        int size = pitch * height * VIDEO_SURFACE_COUNT;
        g_vid_buff_addr[0] = itpVmemAlloc(size);

        assert(g_vid_buff_addr[0]);

        for (i = 1; i < VIDEO_SURFACE_COUNT; ++i)
        {
            g_vid_buff_addr[i] = g_vid_buff_addr[i - 1] + pitch * height;
        }
    #ifdef _DEBUG
        for (i = 0; i < VIDEO_SURFACE_COUNT; ++i)
            printf("video buffer(%d, %X)\n", i, g_vid_buff_addr[i]);
    #endif
        //memset(g_vid_buff_addr[0], 0, size);
    }
    
#endif
  
#ifndef CFG_FFMPEG_H264_SW

    VideoInit();

    for (i = 0; i < VIDEO_SURFACE_COUNT; i++)
    {
        VideoSurf[i] = ituCreateSurface(width, height, pitch, ITU_RGB565, g_vid_buff_addr[i], ITU_STATIC);
        itv_set_vidSurf_buf(g_vid_buff_addr[i] , i);
    }

#endif

    g_inited = true;  
}

void ituFrameFuncExit(void)
{
    uint8_t i;
    
    if (!g_inited)
        return;

#ifndef CFG_FFMPEG_H264_SW
    
    VideoExit();

    for (i = 0; i < VIDEO_SURFACE_COUNT; i++)
    {
        ituDestroySurface(VideoSurf[i]);        
    }

    if (g_vid_buff_addr[0])
    {
        itpVmemFree(g_vid_buff_addr[0]);
        memset(g_vid_buff_addr, 0, sizeof(g_vid_buff_addr));
    }
#endif
    g_inited = false;
}

void ituDrawVideoSurface(ITUSurface* dest, int startX, int startY, int width, int height)
{
#ifndef CFG_FFMPEG_H264_SW
    int index;
    ITUColor color;
    
    color.alpha = 0;
    color.red = 0;
    color.green = 0;
    color.blue = 0;
#endif    
    if (!g_inited)
        return;

#ifndef CFG_FFMPEG_H264_SW
    index = itv_get_vidSurf_index();
    
    if (index == -1 || index == -2)
    	ituColorFill(dest, startX, startY, width, height, &color);
    else
        ituBitBlt(dest, startX, startY, width, height, VideoSurf[index], startX, startY);    
#endif
}

int ituDrawVideoSurface_ex(ITUSurface* dest, int startX, int startY, int width, int height)
{
#ifndef CFG_FFMPEG_H264_SW
    int index;
    ITUColor color;

    color.alpha = 0;
    color.red = 0;
    color.green = 0;
    color.blue = 0;
#endif    
    if (!g_inited)
        return;

#ifndef CFG_FFMPEG_H264_SW
    index = itv_get_vidSurf_index();

    if (index != -1 && index != -2)
        ituBitBlt(dest, startX, startY, width, height, VideoSurf[index], startX, startY);
    else
        return -1;
#endif
    return 0;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
#if defined(CFG_VIDEO_ENABLE) && !defined(CFG_FFMPEG_H264_SW)
static void
VideoInit(
    void)
{
    ithVideoInit(NULL);
    itv_init();
}

static void
VideoExit(
    void)
{
    /* release dbuf & itv */
    itv_stop_vidSurf_anchor();
    itv_flush_dbuf();
    itv_deinit();

    /* release decoder stuff */
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    ithVideoExit();
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
}
#endif // CFG_VIDEO_ENABLE
