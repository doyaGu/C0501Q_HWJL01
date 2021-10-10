#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "ite/itu.h"
#include "itu_private.h"
#include "castor3player.h"

#include "ite/audio.h"

#define BOOT_VIDEO "media/boot.mp4"

typedef struct _VideoWindow {
    ITUSurface* lcdSurf;
    int x_pos;
    int y_pos;
    int width;
    int height;
    int lcdBgColor;
} VideoWindow;

static VideoWindow *vwindow = NULL; 
static pthread_t tid;
static MTAL_SPEC mtal_spec = {0};
static bool videoPlayerIsFileEOF = false;

extern bool bPlayingBootVideo;
extern bool isOtherVideoPlaying;

static void FillBgColor(ITUSurface* lcdSurf, int bgColor)
{
    ITUColor fillColor;
	ITURotation rot = itv_get_rotation();
    
    fillColor.alpha = 0;
    fillColor.red = (uint8_t)((bgColor&0xff0000) >> 16);
    fillColor.green = (uint8_t)((bgColor&0x00ff00) >> 8);
    fillColor.blue = (uint8_t)(bgColor&0x0000ff);

#ifndef CFG_ENABLE_ROTATE
    ituColorFill(lcdSurf, 0, 0, ithLcdGetWidth(), ithLcdGetHeight(), &fillColor);
#else
    if (rot == ITU_ROT_90 || rot == ITU_ROT_270)
        ituColorFill(lcdSurf, 0, 0, ithLcdGetHeight(), ithLcdGetWidth(), &fillColor);
    else
    	ituColorFill(lcdSurf, 0, 0, ithLcdGetWidth(), ithLcdGetHeight(), &fillColor);
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
    int ret;
    VideoWindow *vwindow = (VideoWindow *)arg;

    while(!videoPlayerIsFileEOF)
    {
#ifdef CFG_BOOT_VIDEO_ENABLE_WINDOW_MODE
        FillBgColor(vwindow->lcdSurf, vwindow->lcdBgColor);
#endif
        ret = ituDrawVideoSurface_ex(vwindow->lcdSurf, vwindow->x_pos, vwindow->y_pos, vwindow->width, vwindow->height);
        if (ret != -1)
            ituFlip(vwindow->lcdSurf);
        usleep(20000);
    }
    
    pthread_exit(NULL);
}

void PlayVideo(int x, int y, int width, int height, int bgColor, int volume)
{
    FILE *bootFile = NULL;
    int fileSize = 0;
    unsigned char* fileBuffer = NULL;

    vwindow = (VideoWindow *)malloc(sizeof(VideoWindow));
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

    bootFile = fopen(CFG_PRIVATE_DRIVE ":/" BOOT_VIDEO, "rb");
    fseek(bootFile, 0, SEEK_END);
    fileSize = ftell(bootFile);
    fseek(bootFile, 0, SEEK_SET);
    if (fileSize)
    {
        fileBuffer = (unsigned char*)malloc(fileSize);
        if (fileBuffer)
        {
            fread(fileBuffer, fileSize, 1, bootFile);
            fclose(bootFile);
            free(fileBuffer);
        }
    }

    audioReadCodec(ITE_MP3_DECODE);

    strcpy(mtal_spec.srcname, CFG_PRIVATE_DRIVE ":/" BOOT_VIDEO);
    mtal_spec.vol_level = volume;
    mtal_pb_select_file(&mtal_spec);
    mtal_pb_play();

    pthread_create(&tid, NULL, DrawVideoSurface, (void *)vwindow);
    usleep(300 * 1000);
}

void WaitPlayVideoFinish(void)
{
    if(tid)
	{
		pthread_join(tid, NULL);
        
        mtal_pb_stop();
        mtal_pb_exit();

		if(vwindow)
			free(vwindow);
        videoPlayerIsFileEOF = false;
        bPlayingBootVideo = false;
        isOtherVideoPlaying = false;
		tid = 0;
	}    
}

