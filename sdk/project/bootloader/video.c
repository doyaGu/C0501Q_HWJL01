#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "ite/itu.h"
#include "itu_private.h"
#include "castor3player.h"

static MTAL_SPEC mtal_spec = {0};
static bool videoPlayerIsFileEOF = false;

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

void PlayVideo(void)
{
    mtal_pb_init(EventHandler);
    strcpy(mtal_spec.srcname, "B:/media/boot.mp4");
    mtal_pb_select_file(&mtal_spec);
    mtal_pb_play();
}


void WaitPlayVideoFinish(void)
{
    ITUSurface* lcdSurf;
    lcdSurf = ituGetDisplaySurface();
    while(!videoPlayerIsFileEOF)
    {
        ituDrawVideoSurface(lcdSurf, 0, 0, ithLcdGetWidth(), ithLcdGetHeight());
        ituFlip(lcdSurf);
        usleep(20000);
    }
    videoPlayerIsFileEOF = false;
 
    mtal_pb_stop();
    mtal_pb_exit();    
}

