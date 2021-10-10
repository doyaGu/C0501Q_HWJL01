#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"
#include "scene.h"
#include "ctrlboard.h"

#define COUNTDOWN_TICK 5000

static ITUSprite* imagePlayerStorageSprite;
static ITUBackground* imageViewCtrlBackground;
static ITUIcon* imageViewIcon;
static ITUScrollMediaFileListBox* imagePlayerScrollMediaFileListBox;
static ITULayer* imagePlayerLayer;

static uint32_t imageViewLastTick;
extern char imagePlayerPath[PATH_MAX];
extern bool imagePlayerLoading, imagePlayerLoaded;
static uint8_t* imageViewData;
static int imageViewDataSize;

static void imageViewLoadCallback(uint8_t* data, int size)
{
    if (data && size > 0)
    {
        if(imageViewDataSize > 0)
        {
            free(imageViewData);
            imageViewData = NULL;
            imageViewDataSize = 0;
        }
        imageViewData = data;
        imageViewDataSize = size;
        imagePlayerLoaded = true;
    }
}

bool ImageViewSDRemovedOnCustom(ITUWidget* widget, char* param)
{
    if (imagePlayerStorageSprite->frame == STORAGE_SD)
    {
        ituLayerGoto(imagePlayerLayer);
    }
    return true;
}

bool ImageViewUsbRemovedOnCustom(ITUWidget* widget, char* param)
{
    if (imagePlayerStorageSprite->frame == STORAGE_USB)
    {
        ituLayerGoto(imagePlayerLayer);
    }
    return true;
}

bool ImageViewLastButtonOnPress(ITUWidget* widget, char* param)
{
    ITUScrollText* item = ituMediaFileListPrev((ITUMediaFileListBox*)imagePlayerScrollMediaFileListBox);
    if (item)
    {
        char* filepath = (char*)ituWidgetGetCustomData(item);
        
        printf("Try to load %s\n", filepath);

        strncpy(imagePlayerPath, filepath, PATH_MAX);

        imagePlayerLoading = true;
    }
    imageViewLastTick = itpGetTickCount();
    return true;
}

bool ImageViewNextButtonOnPress(ITUWidget* widget, char* param)
{
    ITUScrollText* item = ituMediaFileListNext((ITUMediaFileListBox*)imagePlayerScrollMediaFileListBox);
    if (item)
    {
        char* filepath = (char*)ituWidgetGetCustomData(item);

        printf("Try to load %s\n", filepath);

        strncpy(imagePlayerPath, filepath, PATH_MAX);

        imagePlayerLoading = true;
    }
    imageViewLastTick = itpGetTickCount();
    return true;
}

bool ImageViewOnTimer(ITUWidget* widget, char* param)
{
    if (ituWidgetIsVisible(imageViewCtrlBackground) && itpGetTickDuration(imageViewLastTick) >= COUNTDOWN_TICK)
    {
        ituWidgetHide(imageViewCtrlBackground, ITU_EFFECT_SCROLL_DOWN, 10);
        imageViewLastTick = itpGetTickCount();
    }

    if (imagePlayerLoading && !imagePlayerLoaded)
    {
        int ret = PhotoLoad(imagePlayerPath, imageViewLoadCallback);
        if (ret == 0)
            imagePlayerLoading = false;
    }
    else if (imagePlayerLoaded)
    {
        ituIconLoadJpegData(imageViewIcon, imageViewData, imageViewDataSize);
        free(imageViewData);
        imageViewDataSize = 0;
        imagePlayerLoaded = false;
    }
    return true;
}

bool ImageViewViewButtonOnPress(ITUWidget* widget, char* param)
{
    imageViewLastTick = itpGetTickCount();
    ituWidgetShow(imageViewCtrlBackground, ITU_EFFECT_SCROLL_UP, 10);

    return true;
}

static bool ImageViewBackgroundUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    if (ev == ITU_EVENT_TOUCHPINCH)
    {
        int x, y;

        ituWidgetGetGlobalPosition(widget, &x, &y);
        x += arg2;
        y += arg3;

        // TODO: IMPLEMENT
        // arg1: distance
    }
    return ituIconUpdate(widget, ev, arg1, arg2, arg3);
}

bool ImageViewOnEnter(ITUWidget* widget, char* param)
{
    if (!imagePlayerStorageSprite)
    {
        imagePlayerStorageSprite = ituSceneFindWidget(&theScene, "imagePlayerStorageSprite");
        assert(imagePlayerStorageSprite);

        imageViewCtrlBackground = ituSceneFindWidget(&theScene, "imageViewCtrlBackground");
        assert(imageViewCtrlBackground);

        imageViewIcon = ituSceneFindWidget(&theScene, "imageViewIcon");
        assert(imageViewIcon);
        ituWidgetSetUpdate(imageViewIcon, ImageViewBackgroundUpdate);

        imagePlayerScrollMediaFileListBox = ituSceneFindWidget(&theScene, "imagePlayerScrollMediaFileListBox");
        assert(imagePlayerScrollMediaFileListBox);

        imagePlayerLayer = ituSceneFindWidget(&theScene, "imagePlayerLayer");
        assert(imagePlayerLayer);
    }

    ituWidgetSetVisible(imageViewCtrlBackground, true);
    imageViewLastTick = itpGetTickCount();

    imageViewData = NULL;
    imageViewDataSize = 0;

    if (((ITUListBox*)imagePlayerScrollMediaFileListBox)->focusIndex >= 0 && !imagePlayerLoading)
    {
        ITUScrollText* item = (ITUScrollText*) ituListBoxGetFocusItem((ITUListBox*)imagePlayerScrollMediaFileListBox);
        char* filepath = (char*)ituWidgetGetCustomData(item);

        printf("Try to load %s\n", filepath);

        strncpy(imagePlayerPath, filepath, PATH_MAX);

        imagePlayerLoading = true;
    }
    return true;
}

bool ImageViewOnLeave(ITUWidget* widget, char* param)
{
    return true;
}

void ImageViewReset(void)
{
    imagePlayerStorageSprite = NULL;
}
