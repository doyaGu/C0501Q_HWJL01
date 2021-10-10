/*
 * Copyright (c) 2015 ITE technology Corp. All Rights Reserved.
 */
/** @file itv.c
 * Used to do H/W video overlay
 *
 * @author I-Chun Lai
 * @version 0.1
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>

#include "ite/ith.h"
#include "ite/itp.h"
#include "ite/itv.h"
#include "isp/mmp_isp.h"
#include "fc_sync.h"
#include "ite/ith_video.h"
#if (CFG_CHIP_FAMILY != 9850)
    #include "ite/ite_m2d.h"
    #include "m2d/m2d_graphics.h"
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
#if (defined(WIN32) && defined(_DEBUG)) || 0
    #define DEBUG_FLIP
#endif

/* FRAME FUNCTION */
#define ITV_FF_NSET             2
#define ITV_FF_NBUF             2

#define MAX_ROTATE_BUFFER_COUNT 2

//=============================================================================
//                              Macro Definition
//=============================================================================
#define WAIT_UNTIL(expr)          while (!(expr)) { usleep(20000); }
#define SHORT_WAIT_UNTIL(expr)    while (!(expr)) { usleep(1000);  }
#define QUEUE_IS_FULL(w, r, size) ((((w) + 1) % (size)) == (r))

#define IS_VERTICAL_DISPLAY()     ((gRotation) == ITU_ROT_90 || (gRotation) == ITU_ROT_270)
#define IS_ROTATE_DISPLAY()       ((gRotation) != ITU_ROT_0)

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct
{
    uint8_t         *uibuf[ITV_FF_NBUF];
    ITV_UI_PROPERTY uiprop[ITV_FF_NBUF];
    volatile int    ui_ridx;
    volatile int    ui_widx;
} ITV_STRC_FF;

typedef struct
{
    // dbuf: display buffer or decoded video buffer
    //       currently we use 3 decoded video buffers
    uint8_t           *dbuf[ITV_MAX_NDBUF];     // decoded video buffer address
    ITV_DBUF_PROPERTY dbufprop[ITV_MAX_NDBUF];  // property of the decoded video buffer
    ITV_DBUF_PROPERTY blank_yuv_frame;
    volatile int      disp_buf_ridx;
    volatile int      disp_buf_widx;

    //FC_STRC_SYNC      itv_sync;

    /* postponed command */
    volatile int      pcmd_flush_dbuf;
    volatile int      pcmd_flush_uibuf[ITV_FF_NSET];      /* FRAME FUNCTION */

    /* MTAL */
    int               mtal_pb_mode_isr;      // indicate whether the player currently is in playback mode or not.
    volatile int      mtal_pb_mode_u;

    /* FRAME FUNCTION */
    bool              ui_update;
    bool              video_update;
    ITV_DBUF_PROPERTY *curr_video;
    int               ff_mode_isr[ITV_FF_NSET];
    volatile int      ff_mode_u[ITV_FF_NSET];
    ITV_STRC_FF       ff[ITV_FF_NSET];
    volatile int      ff_setbase;
    volatile int      ff_setbase_id;
    volatile int      ff_setbase_bid;
    volatile uint8_t  *ff_setbase_base;
} ITV_STRC_RESOURCE;

typedef struct LCD_FLIP_TAG
{
    sem_t     tScanlineEvent;
    uint32_t  bInited;
    pthread_t hw_overlay_tid;
    pthread_t lcd_isr_tid;
} LCD_FLIP;

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
// when there are no video frames to output, a blank 16 x 16 frame will be output
static ITV_STRC_RESOURCE itv_rcs;
static LCD_FLIP          gtFlipCtrl;

#if (CFG_CHIP_FAMILY != 9850)
// for rotation
static uint32_t        gRotateBuffer[MAX_ROTATE_BUFFER_COUNT];
static uint8_t         gRotateBufIdx = 0;
static MMP_M2D_SURFACE gRoateSurf[MAX_ROTATE_BUFFER_COUNT];
static MMP_M2D_SURFACE *_gRoateSurf;
static MMP_M2D_SURFACE _gLcdSurf[3];
#endif
static ITURotation     gRotation = ITU_ROT_0;
static ISP_DEVICE      gIspDev;

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static void itv_lcdISR(void *arg);
static void itv_change_display_format();

static void update_ui()
{
    /* FRAME FUNCTION */
    int         i;
    ITV_STRC_FF *ff;
    int         widx, ridx;

    //itv_rcs.ui_update = false;
    for (i = 0; i < ITV_FF_NSET; i++)
    {
        ff   = &(itv_rcs.ff[i]);
        widx = ff->ui_widx;
        ridx = ff->ui_ridx;

        if (widx != ridx)
        {                                    // UI change, do flip
            if (itv_rcs.ff_mode_isr[i] == 1) // if frame function is enabled
            {
                itv_rcs.ui_update = true;
                mmpIspSetFrameFunction(
                    gIspDev,
                    ff->uibuf[ridx],
                    ff->uiprop[ridx].startX,
                    ff->uiprop[ridx].startY,
                    ff->uiprop[ridx].width,
                    ff->uiprop[ridx].height,
                    ff->uiprop[ridx].pitch,
                    ff->uiprop[ridx].colorKeyR,
                    ff->uiprop[ridx].colorKeyG,
                    ff->uiprop[ridx].colorKeyB,
                    ff->uiprop[ridx].constAlpha,
                    MMP_PIXEL_FORMAT_RGB565,
                    ff->ui_ridx);  // todo: check if the ui buffer index as a real value

                switch (i)
                {
                case 0: mmpIspEnable(gIspDev, MMP_ISP_FRAME_FUNCTION_0); break;
                case 1:
                default: mmpIspEnable(gIspDev, MMP_ISP_FRAME_FUNCTION_1); break;
                }
            }
            //-
            ff->ui_ridx = (ridx + 1) % ITV_FF_NBUF;
        }
        else
        {
            /* postponed command */
            itv_rcs.pcmd_flush_uibuf[i] = 0;

            if (itv_rcs.ff_mode_isr[i] == 0)
            {
                switch (i)
                {
                case 0: mmpIspDisable(gIspDev, MMP_ISP_FRAME_FUNCTION_0); break;
                case 1:
                default: mmpIspDisable(gIspDev, MMP_ISP_FRAME_FUNCTION_1); break;
                }
            }
        }
    }
}

static int g_lcd_index;
static int lcd_index;
static uint32_t next_lcd_index()
{
#if (CFG_CHIP_PKG_IT9852)    
    lcd_index = (lcd_index + 1) % 2;
#else
    lcd_index = (lcd_index + 1) % 3;
#endif   
    return lcd_index;
}

static void flip_lcd(
    int lcd_index)
{
    MMP_ISP_OUTPUT_INFO out_info  = {0};
    uint32_t            out_buffer;
    MMP_ISP_SHARE       isp_share = { 0 };

    do
    {
        if (itv_rcs.curr_video != &itv_rcs.blank_yuv_frame)
        {
            if ((NULL == itv_rcs.curr_video)
                || (NULL == itv_rcs.curr_video->ya)
                || (NULL == itv_rcs.curr_video->ua)
                || (NULL == itv_rcs.curr_video->va)
                || (0 == itv_rcs.curr_video->src_w)
                || (0 == itv_rcs.curr_video->src_h)
                || (0 == itv_rcs.curr_video->pitch_y)
                || (0 == itv_rcs.curr_video->pitch_uv))
            {
                itv_rcs.curr_video = &itv_rcs.blank_yuv_frame;
                break;
            }

            if (itv_rcs.curr_video->format == 0)
                itv_rcs.curr_video->format = MMP_ISP_IN_YUV420;
        }
    } while (0);

    isp_share.addrY   = (uint32_t)itv_rcs.curr_video->ya;
    isp_share.addrU   = (uint32_t)itv_rcs.curr_video->ua;
    isp_share.addrV   = (uint32_t)itv_rcs.curr_video->va;
    isp_share.width   = itv_rcs.curr_video->src_w;
    isp_share.height  = itv_rcs.curr_video->src_h;
    isp_share.pitchY  = itv_rcs.curr_video->pitch_y;
    isp_share.pitchUv = itv_rcs.curr_video->pitch_uv;
    isp_share.format  = itv_rcs.curr_video->format;

#if (CFG_CHIP_FAMILY != 9850)
    if (IS_ROTATE_DISPLAY())
    {
        _gRoateSurf      = (_gRoateSurf == &gRoateSurf[0]) ? &gRoateSurf[1] : &gRoateSurf[0];
        out_info.addrRGB = (MMP_UINT32)((M2D_SURFACE *)(*_gRoateSurf))->baseScanPtr;
        lcd_index        = (_gRoateSurf == &gRoateSurf[0]) ? 0 : 1;
    }
    else
#endif
    {
        lcd_index        = next_lcd_index();
        out_buffer       = (uint32_t)itv_rcs.dbuf[lcd_index];
        out_info.addrRGB = out_buffer;
        g_lcd_index      = lcd_index;
    }

#if (CFG_CHIP_FAMILY != 9850)
    if (IS_VERTICAL_DISPLAY())
    {
        out_info.width    = ithLcdGetHeight();
        out_info.height   = ithLcdGetWidth();
        out_info.pitchRGB = out_info.width * (ithLcdGetPitch() / ithLcdGetWidth());
    }
    else
#endif
    {
        out_info.width    = ithLcdGetWidth();
        out_info.height   = ithLcdGetHeight();
        out_info.pitchRGB = ithLcdGetPitch();
    }
    out_info.format = MMP_ISP_OUT_DITHER565;
#if (CFG_CHIP_FAMILY == 9850)
#ifndef CFG_CAPTURE_MODULE_ENABLE
    mmpIspSetMode(gIspDev, MMP_ISP_MODE_PLAY_VIDEO);
#endif
#endif
    mmpIspSetOutputWindow(gIspDev, &out_info);
    mmpIspPlayImageProcess(gIspDev, &isp_share);
    SHORT_WAIT_UNTIL(mmpIspIsEngineIdle());

#if (CFG_CHIP_FAMILY != 9850)
    if (IS_ROTATE_DISPLAY())
    {
        MMP_M2D_SURFACE *m2dtempSurf;
        float           angle;
        MMP_INT         dX;
        MMP_INT         dY;
        MMP_INT         cX;
        MMP_INT         cY;
        int             lcd_index = next_lcd_index();

        switch (gRotation)
        {
        default:
            angle = 0;
            break;

        case ITU_ROT_90:
            angle = 90.0f;
            dX    = ithLcdGetWidth() - 1;
            dY    = 0;
            cX    = 0;
            cY    = 0;
            break;

        case ITU_ROT_180:
            angle = 180.0f;
            dX    = 0; // [TODO]: the value is incorrect. It needs to be fixed.
            dY    = 0;
            cX    = 0;
            cY    = 0;
            break;

        case ITU_ROT_270:
            angle = 270.0f;
            dX    = 0;
            dY    = 0;
            cX    = ithLcdGetHeight();
            cY    = 0;
            break;
        }

        m2dtempSurf = &_gLcdSurf[lcd_index];
        //printf("%s(%d)\n", __FUNCTION__, __LINE__);
    #ifdef CFG_M2D_ENABLE
        //printf("%s(%d)\n", __FUNCTION__, __LINE__);
        mmpM2dTransformations(
            *m2dtempSurf,
            dX,
            dY,
            *_gRoateSurf,
            cX,
            cY,
            angle,
            1.0f);

        //flip
        mmpM2dWaitIdle();
        //printf("%s(%d)\n", __FUNCTION__, __LINE__);
    #endif
        g_lcd_index = lcd_index;
    }
#endif
    itv_rcs.ui_update    = false;
    itv_rcs.video_update = false;
}

static void itv_lcdISR(void *arg __attribute__((__unused__)))
{
    if (gtFlipCtrl.bInited)
        sem_post(&gtFlipCtrl.tScanlineEvent);

    ithLcdIntrClear();
}

static void _itv_hw_overlay_task(void)
{
    /* postponed command */
    if (itv_rcs.mtal_pb_mode_u != -1)
    {
        itv_rcs.mtal_pb_mode_isr = itv_rcs.mtal_pb_mode_u;
        itv_rcs.mtal_pb_mode_u   = -1;
    }
#ifdef CFG_LCD_ENABLE
    if (g_lcd_index >= 0)
    {
        ithLcdSwFlip(g_lcd_index);

        if (gtFlipCtrl.bInited)
            sem_wait(&gtFlipCtrl.tScanlineEvent);
        g_lcd_index = -1;
    }
#endif

    /* FRAME FUNCTION */
    {
        int i;

        for (i = 0; i < ITV_FF_NSET; i++)    // two sets of frame functions
        {
            if (itv_rcs.ff_mode_u[i] != -1)
            {
                itv_rcs.ff_mode_isr[i] = itv_rcs.ff_mode_u[i];
                itv_rcs.ff_mode_u[i]   = -1;
            }
        }

        if (itv_rcs.ff_setbase == 1)
        {
            const int   id  = itv_rcs.ff_setbase_id;
            const int   bid = itv_rcs.ff_setbase_bid;
            ITV_STRC_FF *ff = &(itv_rcs.ff[id]);

            ff->uibuf[bid]     = itv_rcs.ff_setbase_base;
            itv_rcs.ff_setbase = 0;
        }
    }

    itv_rcs.video_update = false;
    update_ui();

    if (itv_rcs.mtal_pb_mode_isr == 0)
    {
        // flush display buffer?, TODO: state change (play -> stop, stop -> play)
        // itv_rcs.disp_buf_ridx = itv_rcs.disp_buf_widx;
        itv_rcs.curr_video = &itv_rcs.blank_yuv_frame;
    }
    if (itv_rcs.disp_buf_ridx != itv_rcs.disp_buf_widx)
    {
        itv_rcs.curr_video    = &itv_rcs.dbufprop[itv_rcs.disp_buf_ridx];
        itv_rcs.video_update  = true;
        itv_rcs.disp_buf_ridx = (itv_rcs.disp_buf_ridx + 1) % ITV_MAX_DISP_BUF;
    }
    else
    {
        /* postponed command */
        itv_rcs.pcmd_flush_dbuf = 0;    // set this flag to 0 means all queued display buffers have been flushed.
    }

    if (itv_rcs.video_update || itv_rcs.ui_update)
        flip_lcd(itv_rcs.disp_buf_ridx);
}

#ifdef _WIN32
static g_stop_lcd_isr_task;
static void *lcd_isr_task(void *arg)
{
    #ifdef DEBUG_FLIP
    printf("[LCD ISR] Enter\n");
    #endif

    g_stop_lcd_isr_task = false;
    while (!g_stop_lcd_isr_task)
    {
        itv_lcdISR(arg);
        usleep(16000);
    }

    #ifdef DEBUG_FLIP
    printf("[LCD ISR] Leave\n");
    #endif
    pthread_exit(NULL);
    return (void *)0;
}
#endif // _WIN32

static g_stop_itv_hw_overlay_task;
static void *itv_hw_overlay_task(void *arg)
{
    g_stop_itv_hw_overlay_task = false;
    while (!g_stop_itv_hw_overlay_task)
    {
        _itv_hw_overlay_task();
        usleep(10000);
    }
    pthread_exit(NULL);
    return (void *)0;
}

static void itv_change_display_format()
{
#ifdef CFG_LCD_ENABLE
    {
        int width  = ithLcdGetWidth();
        int height = ithLcdGetHeight();
        if (IS_VERTICAL_DISPLAY())
        {
            width  = ithLcdGetHeight();
            height = ithLcdGetWidth();
        }

        mmpIspInitialize(&gIspDev);
        mmpIspSetMode(gIspDev, MMP_ISP_MODE_TRANSFORM);
        mmpIspSetDisplayWindow(gIspDev, 0, 0, width, height);
        mmpIspSetVideoWindow(gIspDev, 0, 0, width, height);
        mmpIspSetOutputFormat(gIspDev, MMP_ISP_OUT_DITHER565);
        ithLcdSwFlip(ithLcdGetFlip());
        ithLcdDisableHwFlip();
    }
#endif

    if (!gtFlipCtrl.bInited)
        gtFlipCtrl.bInited = !sem_init(&gtFlipCtrl.tScanlineEvent, 0, 1);

    if (gtFlipCtrl.bInited)
    {
        g_lcd_index = -1;
        pthread_create(&gtFlipCtrl.hw_overlay_tid, NULL, itv_hw_overlay_task, NULL);
#ifndef WIN32
        /* enable lcd ISR */
        {
            uint32_t lcd_height = ithLcdGetHeight();
            ithIntrDisableIrq(ITH_INTR_LCD);
            ithLcdIntrClear();
            ithIntrClearIrq(ITH_INTR_LCD);

            ithIntrRegisterHandlerIrq(ITH_INTR_LCD, itv_lcdISR, NULL);
            ithLcdIntrSetScanLine1(lcd_height);
            ithLcdIntrSetScanLine2(lcd_height);
            ithLcdIntrCtrlDisable(ITH_LCD_INTR_OUTPUT2);
            ithLcdIntrCtrlDisable(ITH_LCD_INTR_FIELDMODE2);
            ithLcdIntrCtrlDisable(ITH_LCD_INTR_OUTPUT1);
            ithLcdIntrCtrlDisable(ITH_LCD_INTR_FIELDMODE1);

            ithLcdIntrCtrlEnable(ITH_LCD_INTR_OUTPUT2);

            ithIntrEnableIrq(ITH_INTR_LCD);
            ithLcdIntrEnable();
        }
#else
        pthread_create(&gtFlipCtrl.lcd_isr_tid, NULL, lcd_isr_task, NULL);
#endif
    }
}

int itv_init(void)
{
    int i;

    printf("ITV# init !\n");

    /* sanity check */
    if (ITV_MAX_NDBUF > 3)
    {
        printf("ERROR# invalid ITV_MAX_NDBUF (%d), %s:%d !\n", ITV_MAX_NDBUF, __FILE__, __LINE__);
        return -1;
    }

    // initialize the blank YUV frame's value
    memset((void *)&itv_rcs, 0, sizeof(ITV_STRC_RESOURCE));
    itv_rcs.blank_yuv_frame.ya       = (uint8_t *)ithVideoQuery(VIDEO_ADDRESS_BLANK_BUF_Y, 0);
    itv_rcs.blank_yuv_frame.ua       = (uint8_t *)ithVideoQuery(VIDEO_ADDRESS_BLANK_BUF_U, 0);
    itv_rcs.blank_yuv_frame.va       = (uint8_t *)ithVideoQuery(VIDEO_ADDRESS_BLANK_BUF_V, 0);
    itv_rcs.blank_yuv_frame.src_w    = 16;
    itv_rcs.blank_yuv_frame.src_h    = 16;
    itv_rcs.blank_yuv_frame.pitch_y  = ithVideoQuery(VIDEO_FRAME_BUFFER_Y_PITCH, 0);
    itv_rcs.blank_yuv_frame.pitch_uv = ithVideoQuery(VIDEO_FRAME_BUFFER_UV_PITCH, 0);
    itv_rcs.blank_yuv_frame.format   = MMP_ISP_IN_YUV420;
    itv_rcs.curr_video               = &itv_rcs.blank_yuv_frame;
    itv_rcs.dbuf[0]                  = (uint8_t *)ithLcdGetBaseAddrA();
    itv_rcs.dbuf[1]                  = (uint8_t *)ithLcdGetBaseAddrB();
    itv_rcs.dbuf[2]                  = (uint8_t *)ithLcdGetBaseAddrC();
    lcd_index                        = ithLcdGetFlip();
    /* MTAL */
    itv_rcs.mtal_pb_mode_u           = -1;
    //-

    /* FRAME FUNCTION */
    for (i = 0; i < ITV_FF_NSET; i++)
        itv_rcs.ff_mode_u[i] = -1;

    itv_change_display_format();

    /* sync module */
    //fc_init_sync(&itv_rcs.itv_sync);    // [ToDO] inin sync module should be done in the player, not here

    return 0;
}

int itv_deinit(void)
{
    printf("ITV# de-init !\n");

    if (gtFlipCtrl.bInited)
    {
        g_stop_itv_hw_overlay_task = true;
        pthread_join(gtFlipCtrl.hw_overlay_tid, NULL);

#ifndef WIN32
        ithLcdIntrDisable();
#else
        g_stop_lcd_isr_task = true;
        pthread_join(gtFlipCtrl.lcd_isr_tid, NULL);
#endif
        sem_destroy(&(gtFlipCtrl.tScanlineEvent));
        memset(&gtFlipCtrl, 0, sizeof(gtFlipCtrl));
    }

    /* sync module */
    //fc_deinit_sync(&itv_rcs.itv_sync);

    memset((void *)&itv_rcs, 0, sizeof(ITV_STRC_RESOURCE));
    return 0;
}

uint8_t *itv_get_dbuf_anchor(void)
{
    if (QUEUE_IS_FULL(itv_rcs.disp_buf_widx, itv_rcs.disp_buf_ridx, ITV_MAX_DISP_BUF))
        return NULL;

    return itv_rcs.dbuf[itv_rcs.disp_buf_widx];
}

int itv_update_dbuf_anchor(ITV_DBUF_PROPERTY *prop)
{
    if (QUEUE_IS_FULL(itv_rcs.disp_buf_widx, itv_rcs.disp_buf_ridx, ITV_MAX_DISP_BUF))
        return -1;

    memcpy((void *)&itv_rcs.dbufprop[itv_rcs.disp_buf_widx], (void *)prop, sizeof(ITV_DBUF_PROPERTY));
    itv_rcs.disp_buf_widx = (itv_rcs.disp_buf_widx + 1) % ITV_MAX_DISP_BUF;
    return 0;
}

void itv_flush_dbuf(void)
{
    itv_rcs.pcmd_flush_dbuf = 1;

    WAIT_UNTIL(itv_rcs.pcmd_flush_dbuf == 0);
}

/* FRAME FUNCTION */
int itv_ff_setup_base(int id, int bid, uint8_t *base)
{
    WAIT_UNTIL(itv_rcs.ff_setbase == 0);

    itv_rcs.ff_setbase_id   = id;
    itv_rcs.ff_setbase_bid  = bid;
    itv_rcs.ff_setbase_base = base;
    itv_rcs.ff_setbase      = 1;

    WAIT_UNTIL(itv_rcs.ff_setbase == 0);

    return 0;
}

int itv_ff_enable(int id, int enable)
{
    printf("ITV# %s, id(%d):%d +\n", __func__, id, enable);

    itv_flush_uibuf(id);

    WAIT_UNTIL(itv_rcs.ff_mode_u[id] == -1);
    itv_rcs.ff_mode_u[id] = enable;
    WAIT_UNTIL(itv_rcs.ff_mode_u[id] == -1);

    printf("ITV# %s, id(%d):%d -\n", __func__, id, enable);
    return 0;
}

uint8_t *itv_get_uibuf_anchor(int id)
{
    ITV_STRC_FF *ff  = &(itv_rcs.ff[id]);
    const int   widx = ff->ui_widx;
    const int   ridx = ff->ui_ridx;

    if (QUEUE_IS_FULL(widx, ridx, ITV_FF_NBUF))
        return NULL;

    return ff->uibuf[widx];
}

int itv_update_uibuf_anchor(int id, ITV_UI_PROPERTY *uiprop)
{
    ITV_STRC_FF *ff  = &(itv_rcs.ff[id]);
    const int   widx = ff->ui_widx;
    const int   ridx = ff->ui_ridx;

    if (QUEUE_IS_FULL(widx, ridx, ITV_FF_NBUF))
        return -1;

    memcpy((void *)&(ff->uiprop[widx]), (void *)uiprop, sizeof(ITV_UI_PROPERTY));
    ff->ui_widx = (widx + 1) % ITV_FF_NBUF;
    return 0;
}

void itv_flush_uibuf(int id)
{
    itv_rcs.pcmd_flush_uibuf[id] = 1;

    WAIT_UNTIL(itv_rcs.pcmd_flush_uibuf[id] == 0);
}

// pb_mode: a boolean value,
//          1 means in playback mode, 0 means not in playback mode
int itv_set_pb_mode(int pb_mode)
{
    printf("%s(%d) pb_mode(%d)\n", __FUNCTION__, __LINE__, pb_mode);

    WAIT_UNTIL(itv_rcs.mtal_pb_mode_u == -1);
    itv_rcs.mtal_pb_mode_u = pb_mode;
    WAIT_UNTIL(itv_rcs.mtal_pb_mode_u == -1);

    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    return itv_rcs.mtal_pb_mode_u;
}

void itv_set_rotation(ITURotation rot)
{
    gRotation = rot;

#if (CFG_CHIP_FAMILY != 9850)
    // create rotation buffer and sruface
    if (IS_ROTATE_DISPLAY() && (!gRotateBuffer[0]))
    {
        int i;
        int size = ithLcdGetPitch() * ithLcdGetHeight() * 2;
        gRotateBuffer[0] = itpVmemAlloc(size);

        //assert(gRotateBuffer[0]);

        for (i = 1; i < MAX_ROTATE_BUFFER_COUNT; ++i)
        {
            gRotateBuffer[i] = gRotateBuffer[i - 1] + ithLcdGetPitch() * ithLcdGetHeight();
        }

        printf("Rotate tmpbut %x %x\n", gRotateBuffer[0], gRotateBuffer[1]);
    #ifdef CFG_M2D_ENABLE
        for (i = 0; i < MAX_ROTATE_BUFFER_COUNT; ++i)
        {
            mmpM2dCreateVirtualSurface(
                ithLcdGetHeight(),
                ithLcdGetWidth(),
                MMP_M2D_IMAGE_FORMAT_RGB565,
                (MMP_UINT8 *)gRotateBuffer[i],
                &gRoateSurf[i]);
            printf("%X\n", (void *)((M2D_SURFACE *)gRoateSurf[i])->baseScanPtr);
        }
        for (i = 0; i < 3; ++i)
        {
            mmpM2dCreateVirtualSurface(
                ithLcdGetWidth(),
                ithLcdGetHeight(),
                MMP_M2D_IMAGE_FORMAT_RGB565,
                itv_rcs.dbuf[i],
                &_gLcdSurf[i]);
        }
    #endif
    }
#endif
}

void
itv_set_video_window(
    uint32_t startX,
    uint32_t startY,
    uint32_t width,
    uint32_t height)
{
    mmpIspSetVideoWindow(gIspDev, startX, startY, width, height);
}

ISP_RESULT
itv_enable_isp_feature(
    MMP_ISP_CAPS cap)
{
    return mmpIspEnable(gIspDev, cap);
}