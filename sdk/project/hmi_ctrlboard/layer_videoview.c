#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"
#include "scene.h"
#include "ctrlboard.h"

static ITUVideo* videoViewVideo;

bool VideoViewSDRemovedOnCustom(ITUWidget* widget, char* param)
{
    return true;
}

bool VideoViewUsbRemovedOnCustom(ITUWidget* widget, char* param)
{
    return true;
}

bool VideoViewOnEnter(ITUWidget* widget, char* param)
{
    int vol;

    if (!videoViewVideo)
    {
        videoViewVideo = ituSceneFindWidget(&theScene, "videoViewVideo");
        assert(videoViewVideo);
    }
    vol = AudioGetVolume();
    videoViewVideo->volume = vol;

    return true;
}

bool VideoViewOnLeave(ITUWidget* widget, char* param)
{
    return true;
}

void VideoViewReset(void)
{
    videoViewVideo = NULL;
}
