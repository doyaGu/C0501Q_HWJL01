#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "ite/itu.h"
#include "itu_private.h"
#include "castor3player.h"

#define BOOT_VIDEO "media/boot.mp4"

typedef struct _VideoWindow {
    ITUSurface* lcdSurf;
    int x_pos;
    int y_pos;
    int width;
    int height;
    int lcdBgColor;
} VideoWindow;

static pthread_t tid;
static MTAL_SPEC mtal_spec = {0};
static bool videoPlayerIsFileEOF = false;

extern bool bPlayingBootVideo;
extern bool isOtherVideoPlaying;

static void FillBgColor(ITUSurface* lcdSurf, int bgColor)
{
    ITUColor fillColor;
    
    fillColor.alpha = 0;
    fillColor.red = (uint8_t)((bgColor&0xff0000) >> 16);
    fillColor.green = (uint8_t)((bgColor&0x00ff00) >> 8);
    fillColor.blue = (uint8_t)(bgColor&0x0000ff);

#ifndef CFG_ENABLE_ROTATE
    ituColorFill(lcdSurf, 0, 0, ithLcdGetWidth(), ithLcdGetHeight(), &fillColor);
#else
    ituColorFill(lcdSurf, 0, 0, ithLcdGetHeight(), ithLcdGetWidth(), &fillColor);
#endif
}

static void EventHandler(PLAYER_EVENT nEventID, void *arg)
{
    switch(nEventID)
    {
        case PLAYER_EVENT_EOF:
            printf("File EOF\n");
            videoPlayerIsFileEOF = true;
            break;
        case PLAYER_EVENT_OPEN_FILE_FAIL:
            printf("Open file fail\n");
            videoPlayerIsFileEOF = true;
            break;
        case PLAYER_EVENT_UNSUPPORT_FILE:
            printf("File not support\n");
            videoPlayerIsFileEOF = true;
            break;
        default:
            break;
    }
}

static void *DrawVideoSurface(void *arg)
{
    VideoWindow *vwindow = (VideoWindow *)arg;

    while(!videoPlayerIsFileEOF)
    {
        FillBgColor(vwindow->lcdSurf, vwindow->lcdBgColor);
        ituDrawVideoSurface(vwindow->lcdSurf, vwindow->x_pos, vwindow->y_pos, vwindow->width, vwindow->height);
        ituFlip(vwindow->lcdSurf);
        usleep(20000);
    }
    
    pthread_exit(NULL);
}

void PlayVideo(int x, int y, int width, int height, int bgColor, int volume)
{
    VideoWindow *vwindow = (VideoWindow *)malloc(sizeof(VideoWindow));
    vwindow->lcdSurf = ituGetDisplaySurface();
    vwindow->x_pos = x;
    vwindow->y_pos = y;
    vwindow->width = width;
    vwindow->height = height;
    vwindow->lcdBgColor = bgColor;
    
    itv_set_video_window(x, y, width, height);
    bPlayingBootVideo = true;
    isOtherVideoPlaying = true;
    mtal_pb_init(EventHandler);
    strcpy(mtal_spec.srcname, CFG_PUBLIC_DRIVE ":/" BOOT_VIDEO);
    mtal_spec.vol_level = volume;
    mtal_pb_select_file(&mtal_spec);
    mtal_pb_play();

    pthread_create(&tid, NULL, DrawVideoSurface, (void *)vwindow);
}

void WaitPlayVideoFinish(void)
{
    if(tid)
	{
		pthread_join(tid, NULL);
        
        mtal_pb_stop();
        mtal_pb_exit();
        videoPlayerIsFileEOF = false;
        bPlayingBootVideo = false;
        isOtherVideoPlaying = false;
		tid = 0;
	}    
}

