#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "itu_cfg.h"
#include "ite/ith.h"
#include "ite/itu.h"
#include "ite/itp.h"

#if defined(CFG_M2D_ENABLE)
    #include "ite/ite_m2d.h"
#endif

#if defined(CFG_VIDEO_ENABLE) && !defined(CFG_FFMPEG_H264_SW)
    #include "ite/itv.h"
    #include "ite/ith_video.h"
#endif
#include "itu_private.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MAX_UI_BUFFER_COUNT 2
//#if defined(CFG_VIDEO_ENABLE) && !defined(CFG_FFMPEG_H264_SW)
//    #define FRAME_BUFFER_COUNT 4
//#endif

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
//typedef struct
//{
//    ITUSurface      surf;
//#if defined(CFG_M2D_ENABLE)
//    MMP_M2D_SURFACE m2dSurf;
//#endif
//} M2dSurface;

//=============================================================================
//                              Static Data Definition
//=============================================================================
static uint32_t    g_ui_buff_addr[MAX_UI_BUFFER_COUNT];
static uint32_t    g_old_surface_addr;
static void        (*g_old_ituFlip)(ITUSurface *surf);
static bool        g_inited;
static ITURotation g_curr_rotation;

//=============================================================================
//                              Private Function Declaration
//=============================================================================
#if defined(CFG_VIDEO_ENABLE) && !defined(CFG_FFMPEG_H264_SW)
static void VideoInit(void);
static void VideoExit(void);
static uint32_t getUiBufBaseAddr(void);
static void FrameFuncSetRotation(ITURotation rot);
static void FrameFuncFlip(ITUSurface *surf);
#endif // CFG_VIDEO_ENABLE

//=============================================================================
//                              Public Function Definition
//=============================================================================
void ituFrameFuncInit(void)
{
#ifndef CFG_FFMPEG_H264_SW
    M2dSurface *screenSurf;
    int        width;
    int        height;
    int        pitch;
#endif

    if (g_inited)
        return;

#ifndef CFG_FFMPEG_H264_SW
    screenSurf    = (M2dSurface *)ituGetDisplaySurface();

    width         = screenSurf->surf.width;  //ithLcdGetWidth();
    height        = screenSurf->surf.height; //ithLcdGetHeight();
    pitch         = screenSurf->surf.pitch;  //ithLcdGetPitch();

    g_old_ituFlip = ituFlip;

    VideoInit();
    // TODO: IMPLEMENT
    if (!g_ui_buff_addr[0])
    {
        int i;
        int size = pitch * height * MAX_UI_BUFFER_COUNT;
        g_ui_buff_addr[0] = itpVmemAlloc(size);

        assert(g_ui_buff_addr[0]);

        itv_ff_setup_base(0, 0, (uint8_t *)g_ui_buff_addr[0]);
        for (i = 1; i < MAX_UI_BUFFER_COUNT; ++i)
        {
            g_ui_buff_addr[i] = g_ui_buff_addr[i - 1] + pitch * height;
            itv_ff_setup_base(0, i, (uint8_t *)g_ui_buff_addr[i]);
        }
    #ifdef _DEBUG
        for (i = 0; i < MAX_UI_BUFFER_COUNT; ++i)
            printf("frame func(%d, %X)\n", i, g_ui_buff_addr[i]);
    #endif
    }

    itv_ff_enable(0, true);
    g_old_surface_addr    = screenSurf->surf.addr;
    screenSurf->surf.addr = getUiBufBaseAddr();

    #if defined(CFG_M2D_ENABLE)
    gfxSurfaceSetSurfaceBaseAddress(
        screenSurf->m2dSurf,
        screenSurf->surf.addr);
    #endif

    ituFlip        = FrameFuncFlip;
    ituSetRotation = FrameFuncSetRotation;
#endif

    g_inited       = true;
}

void ituFrameFuncExit(void)
{
    if (!g_inited)
        return;

#ifndef CFG_FFMPEG_H264_SW
    M2dSurface *screenSurf = (M2dSurface *)ituGetDisplaySurface();

    VideoExit();
    if (g_ui_buff_addr[0])
    {
        itpVmemFree(g_ui_buff_addr[0]);
        memset(g_ui_buff_addr, 0, sizeof(g_ui_buff_addr));
    }
    ituFlip               = g_old_ituFlip;
    #ifdef WIN32
    screenSurf->surf.addr = g_old_surface_addr;
        #if defined(CFG_M2D_ENABLE)
    gfxSurfaceSetSurfaceBaseAddress(
        screenSurf->m2dSurf,
        screenSurf->surf.addr);
        #endif

    ithLcdSwFlip(0);
    #else
    {
        uint32_t lcd_index = ithLcdGetFlip();

        switch (lcd_index)
        {
        default:
        case 0:
            screenSurf->surf.addr = ithLcdGetBaseAddrB();
            break;

        case 1:
            screenSurf->surf.addr = ithLcdGetBaseAddrA();
            break;
        }
        #if defined(CFG_M2D_ENABLE)
        gfxSurfaceSetSurfaceBaseAddress(
            screenSurf->m2dSurf,
            screenSurf->surf.addr);
        #endif
    }
    ithLcdEnableHwFlip();
    #endif
#endif
    g_inited = false;
}

void ituDrawVideoSurface(ITUSurface* dest, int startX, int startY, int width, int height)
{
     return;
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
    itv_flush_dbuf();
    itv_deinit();

    /* release decoder stuff */
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    ithVideoExit();
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
}

static uint32_t
getUiBufBaseAddr(
    void)
{
    uint8_t *ui_buf_base_addr = NULL;
    do
    {
        ui_buf_base_addr = itv_get_uibuf_anchor(0);
        if (ui_buf_base_addr == NULL)
        {
            usleep(15000);
        }
    } while (ui_buf_base_addr == NULL);

    return (uint32_t)ui_buf_base_addr;
}

static void
FrameFuncSetRotation(
    ITURotation rot)
{
    ITUSurface *lcdSurf;
    M2dSurface *screenSurf;

    if (g_curr_rotation == rot)
        return;

    lcdSurf    = ituGetDisplaySurface();
    assert(lcdSurf);
    screenSurf = (M2dSurface *)lcdSurf;

    switch (rot)
    {
    default:
        lcdSurf->width  = ithLcdGetWidth();
        lcdSurf->height = ithLcdGetHeight();
        lcdSurf->pitch  = ithLcdGetPitch();
        break;

    case ITU_ROT_90:
    case ITU_ROT_270:
        lcdSurf->width  = ithLcdGetHeight();
        lcdSurf->height = ithLcdGetWidth();
        lcdSurf->pitch  = ithLcdGetPitch() * ithLcdGetHeight() / ithLcdGetWidth();
        break;
    }

    g_curr_rotation = rot;
    itv_set_rotation(rot);

    #if defined(CFG_M2D_ENABLE)
    gfxSurfaceSetWidth(screenSurf->m2dSurf, lcdSurf->width);
    gfxSurfaceSetHeight(screenSurf->m2dSurf, lcdSurf->height);
    gfxSurfaceSetPitch(screenSurf->m2dSurf, lcdSurf->pitch); 
    #endif
}

static void
FrameFuncFlip(
    ITUSurface *surf)
{
    ITV_UI_PROPERTY ui_prop  = {0};
    M2dSurface      *scrSurf = (M2dSurface *)surf;
    
    #if defined(CFG_M2D_ENABLE)
    gfxwaitEngineIdle();
    #endif

    ithUnmapVram(
        ithMapVram(scrSurf->surf.addr,
                   scrSurf->surf.pitch * scrSurf->surf.width, ITH_VRAM_WRITE),
        scrSurf->surf.pitch * scrSurf->surf.width);
    
    ui_prop.startX     = 0;
    ui_prop.startY     = 0;
    ui_prop.width      = scrSurf->surf.width;
    ui_prop.height     = scrSurf->surf.height;
    ui_prop.pitch      = scrSurf->surf.pitch;
    ui_prop.colorKeyR  = 0xFF;
    ui_prop.colorKeyG  = 0x00;
    ui_prop.colorKeyB  = 0xFF;   
    ui_prop.constAlpha = 0xFF;
    itv_update_uibuf_anchor(0, &ui_prop);
    scrSurf->surf.addr = getUiBufBaseAddr();
    #if defined(CFG_M2D_ENABLE)
    gfxSurfaceSetSurfaceBaseAddress(
        scrSurf->m2dSurf,
        scrSurf->surf.addr);
    #endif
}
#endif // CFG_VIDEO_ENABLE