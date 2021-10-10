#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"

static ITUSprite* imagePlayerStorageSprite;
static ITUBackground* imagePlayerStorageTypeBackground;
static ITURadioBox* imagePlayerSDRadioBox;
static ITURadioBox* imagePlayerUsbRadioBox;
static ITURadioBox* imagePlayerInternalRadioBox;
static ITUScrollMediaFileListBox* imagePlayerScrollMediaFileListBox;
static ITUIcon* imagePlayerViewIcon;
static ITUButton* imagePlayerViewButton;

char imagePlayerPath[PATH_MAX];
bool imagePlayerLoading, imagePlayerLoaded;
static uint8_t* imagePlayerData;
static int imagePlayerDataSize;

static void imagePlayerLoadCallback(uint8_t* data, int size)
{
    if (data && size > 0)
    {
        if (imagePlayerDataSize > 0)
        {
            free(imagePlayerData);
            imagePlayerData = NULL;
            imagePlayerDataSize = 0;
        }
        imagePlayerData = data;
        imagePlayerDataSize = size;
        imagePlayerLoaded = true;
    }
}

bool ImagePlayerSDInsertedOnCustom(ITUWidget* widget, char* param)
{
    if (!ituWidgetIsVisible(imagePlayerStorageSprite))
    {
        StorageType storageType = StorageGetCurrType();

        ituWidgetSetVisible(imagePlayerStorageSprite, true);
        ituWidgetSetVisible(imagePlayerScrollMediaFileListBox, true);
        ituSpriteGoto(imagePlayerStorageSprite, storageType);

        ituMediaFileListSetPath(&imagePlayerScrollMediaFileListBox->mflistbox, StorageGetDrivePath(storageType));
    }
    ituWidgetEnable(imagePlayerSDRadioBox);
    return true;
}

bool ImagePlayerSDRemovedOnCustom(ITUWidget* widget, char* param)
{
    if (imagePlayerStorageSprite->frame == STORAGE_SD)
    {
        StorageType storageType = StorageGetCurrType();

        // TODO: IMPLEMENT

        if (storageType == STORAGE_NONE)
        {
            ituWidgetSetVisible(imagePlayerStorageSprite, false);
            ituWidgetSetVisible(imagePlayerScrollMediaFileListBox, false);
        }
        else
        {
            ituWidgetSetVisible(imagePlayerStorageSprite, true);
            ituWidgetSetVisible(imagePlayerScrollMediaFileListBox, true);
            ituSpriteGoto(imagePlayerStorageSprite, storageType);

            ituMediaFileListSetPath(&imagePlayerScrollMediaFileListBox->mflistbox, StorageGetDrivePath(storageType));
        }
    }
    ituWidgetDisable(imagePlayerSDRadioBox);
    return true;
}

bool ImagePlayerUsbInsertedOnCustom(ITUWidget* widget, char* param)
{
    if (!ituWidgetIsVisible(imagePlayerStorageSprite))
    {
        StorageType storageType = StorageGetCurrType();

        ituWidgetSetVisible(imagePlayerStorageSprite, true);
        ituWidgetSetVisible(imagePlayerScrollMediaFileListBox, true);
        ituSpriteGoto(imagePlayerStorageSprite, storageType);

        if (storageType == STORAGE_SD)
            ituRadioBoxSetChecked(imagePlayerSDRadioBox, true);
        else if (storageType == STORAGE_USB)
            ituRadioBoxSetChecked(imagePlayerUsbRadioBox, true);
        else if (storageType == STORAGE_INTERNAL)
            ituRadioBoxSetChecked(imagePlayerInternalRadioBox, true);

        ituMediaFileListSetPath(&imagePlayerScrollMediaFileListBox->mflistbox, StorageGetDrivePath(storageType));
    }
    ituWidgetEnable(imagePlayerUsbRadioBox);
    return true;
}

bool ImagePlayerUsbRemovedOnCustom(ITUWidget* widget, char* param)
{
    if (imagePlayerStorageSprite->frame == STORAGE_USB)
    {
        StorageType storageType = StorageGetCurrType();

        // TODO: IMPLEMENT

        if (storageType == STORAGE_NONE)
        {
            ituWidgetSetVisible(imagePlayerStorageSprite, false);
            ituWidgetSetVisible(imagePlayerScrollMediaFileListBox, false);
        }
        else
        {
            ituWidgetSetVisible(imagePlayerStorageSprite, true);
            ituWidgetSetVisible(imagePlayerScrollMediaFileListBox, true);
            ituSpriteGoto(imagePlayerStorageSprite, storageType);

            if (storageType == STORAGE_SD)
                ituRadioBoxSetChecked(imagePlayerSDRadioBox, true);
            else if (storageType == STORAGE_USB)
                ituRadioBoxSetChecked(imagePlayerUsbRadioBox, true);
            else if (storageType == STORAGE_INTERNAL)
                ituRadioBoxSetChecked(imagePlayerInternalRadioBox, true);

            ituMediaFileListSetPath(&imagePlayerScrollMediaFileListBox->mflistbox, StorageGetDrivePath(storageType));
        }
    }
    ituWidgetDisable(imagePlayerUsbRadioBox);
    return true;
}

bool ImagePlayerStorageRadioBoxOnPress(ITUWidget* widget, char* param)
{
    StorageType storageType = StorageGetCurrType();

    if ((storageType == STORAGE_SD && widget == (ITUWidget*)imagePlayerSDRadioBox) ||
        (storageType == STORAGE_USB && widget == (ITUWidget*)imagePlayerUsbRadioBox) ||
        (storageType == STORAGE_INTERNAL && widget == (ITUWidget*)imagePlayerInternalRadioBox))
        return false;

    // TODO: IMPLEMENT

    if (widget == (ITUWidget*)imagePlayerSDRadioBox)
    {
        StorageSetCurrType(STORAGE_SD);
        ituSpriteGoto(imagePlayerStorageSprite, STORAGE_SD);
        ituMediaFileListSetPath(&imagePlayerScrollMediaFileListBox->mflistbox, StorageGetDrivePath(STORAGE_SD));
    }
    else if (widget == (ITUWidget*)imagePlayerUsbRadioBox)
    {
        StorageSetCurrType(STORAGE_USB);
        ituSpriteGoto(imagePlayerStorageSprite, STORAGE_USB);
        ituMediaFileListSetPath(&imagePlayerScrollMediaFileListBox->mflistbox, StorageGetDrivePath(STORAGE_USB));
    }
    else if (widget == (ITUWidget*)imagePlayerInternalRadioBox)
    {
        StorageSetCurrType(STORAGE_INTERNAL);
        ituSpriteGoto(imagePlayerStorageSprite, STORAGE_INTERNAL);
        ituMediaFileListSetPath(&imagePlayerScrollMediaFileListBox->mflistbox, StorageGetDrivePath(STORAGE_INTERNAL));
    }
    return true;
}

bool ImagePlayerStorageTypeCheckBoxOnPress(ITUWidget* widget, char* param)
{
    if (ituWidgetIsVisible(imagePlayerStorageTypeBackground))
    {
        ituWidgetSetVisible(imagePlayerStorageTypeBackground, false);
        ituWidgetEnable(imagePlayerScrollMediaFileListBox);
    }
    else
    {
        ituWidgetSetVisible(imagePlayerStorageTypeBackground, true);
        ituWidgetDisable(imagePlayerScrollMediaFileListBox);
    }
    return true;
}

bool ImagePlayerScrollMediaFileListBoxOnSelection(ITUWidget* widget, char* param)
{
    if (((ITUListBox*)imagePlayerScrollMediaFileListBox)->focusIndex >= 0 && !imagePlayerLoaded  &&  !imagePlayerDataSize && !imagePlayerLoading)
    {
        ITUScrollText* item = ituMediaFileListPlay((ITUMediaFileListBox*)imagePlayerScrollMediaFileListBox);
        if (item)
        {
            char* filepath = (char*)ituWidgetGetCustomData(item);

            printf("Try to load %s\n", filepath);

            strncpy(imagePlayerPath, filepath, PATH_MAX);

            imagePlayerLoading = true;
        }
    }
    return true;
}

bool ImagePlayerLastButtonOnPress(ITUWidget* widget, char* param)
{
    ITUScrollText* item = ituMediaFileListPrev((ITUMediaFileListBox*)imagePlayerScrollMediaFileListBox);
    if (item)
    {
        char* filepath = (char*)ituWidgetGetCustomData(item);
        
        printf("Try to load %s\n", filepath);

        strncpy(imagePlayerPath, filepath, PATH_MAX);

        imagePlayerLoading = true;
    }
    return true;
}

bool ImagePlayerNextButtonOnPress(ITUWidget* widget, char* param)
{
    ITUScrollText* item = ituMediaFileListNext((ITUMediaFileListBox*)imagePlayerScrollMediaFileListBox);
    if (item)
    {
        char* filepath = (char*)ituWidgetGetCustomData(item);

        printf("Try to load %s\n", filepath);

        strncpy(imagePlayerPath, filepath, PATH_MAX);

        imagePlayerLoading = true;
    }
    return true;
}

bool ImagePlayerOnTimer(ITUWidget* widget, char* param)
{
    if (imagePlayerLoading && !imagePlayerLoaded)
    {
        int ret = PhotoLoad(imagePlayerPath, imagePlayerLoadCallback);
        if (ret == 0)
            imagePlayerLoading = false;
    }
    else if (imagePlayerLoaded)
    {
        ituIconLoadJpegData(imagePlayerViewIcon, imagePlayerData, imagePlayerDataSize);
        ituWidgetSetVisible(imagePlayerViewIcon, true);
        ituWidgetSetVisible(imagePlayerViewButton, true);
        free(imagePlayerData);
        imagePlayerData = NULL;
        imagePlayerDataSize = 0;
        imagePlayerLoaded = false;
    }
    return true;
}

bool ImagePlayerOnEnter(ITUWidget* widget, char* param)
{
    StorageType storageType;

    SceneLeaveVideoState();
    if (!imagePlayerStorageSprite)
    {
        imagePlayerStorageSprite = ituSceneFindWidget(&theScene, "imagePlayerStorageSprite");
        assert(imagePlayerStorageSprite);

        imagePlayerStorageTypeBackground = ituSceneFindWidget(&theScene, "imagePlayerStorageTypeBackground");
        assert(imagePlayerStorageTypeBackground);

        imagePlayerSDRadioBox = ituSceneFindWidget(&theScene, "imagePlayerSDRadioBox");
        assert(imagePlayerSDRadioBox);

        imagePlayerUsbRadioBox = ituSceneFindWidget(&theScene, "imagePlayerUsbRadioBox");
        assert(imagePlayerUsbRadioBox);

        imagePlayerInternalRadioBox = ituSceneFindWidget(&theScene, "imagePlayerInternalRadioBox");
        assert(imagePlayerInternalRadioBox);

        imagePlayerScrollMediaFileListBox = ituSceneFindWidget(&theScene, "imagePlayerScrollMediaFileListBox");
        assert(imagePlayerScrollMediaFileListBox);

        imagePlayerViewIcon = ituSceneFindWidget(&theScene, "imagePlayerViewIcon");
        assert(imagePlayerViewIcon);

        imagePlayerViewButton = ituSceneFindWidget(&theScene, "imagePlayerViewButton");
        assert(imagePlayerViewButton);
    }

    if (strcmp(param, "imageViewLayer") == 0)
    {
        if (((ITUListBox*)imagePlayerScrollMediaFileListBox)->focusIndex >= 0 && !imagePlayerLoading)
        {
            ITUScrollText* item = (ITUScrollText*) ituListBoxGetFocusItem((ITUListBox*)imagePlayerScrollMediaFileListBox);
            char* filepath = (char*)ituWidgetGetCustomData(item);

            strncpy(imagePlayerPath, filepath, PATH_MAX);

            printf("Try to load %s\n", imagePlayerPath);

            imagePlayerLoading = true;
        }
    }
    else
    {
        storageType = StorageGetCurrType();

        if (storageType == STORAGE_NONE)
        {
            ituWidgetSetVisible(imagePlayerStorageSprite, false);
            ituWidgetSetVisible(imagePlayerScrollMediaFileListBox, false);
        }
        else
        {
            ituWidgetSetVisible(imagePlayerStorageSprite, true);
            ituWidgetSetVisible(imagePlayerScrollMediaFileListBox, true);
            ituSpriteGoto(imagePlayerStorageSprite, storageType);

            if (storageType == STORAGE_SD)
                ituRadioBoxSetChecked(imagePlayerSDRadioBox, true);
            else if (storageType == STORAGE_USB)
                ituRadioBoxSetChecked(imagePlayerUsbRadioBox, true);
            else if (storageType == STORAGE_INTERNAL)
                ituRadioBoxSetChecked(imagePlayerInternalRadioBox, true);

            if (StorageGetDrivePath(STORAGE_SD))
                ituWidgetEnable(imagePlayerSDRadioBox);
            else
                ituWidgetDisable(imagePlayerSDRadioBox);

            if (StorageGetDrivePath(STORAGE_USB))
                ituWidgetEnable(imagePlayerUsbRadioBox);
            else
                ituWidgetDisable(imagePlayerUsbRadioBox);

            if (StorageGetDrivePath(STORAGE_INTERNAL))
                ituWidgetEnable(imagePlayerInternalRadioBox);
            else
                ituWidgetDisable(imagePlayerInternalRadioBox);

            ituMediaFileListSetPath(&imagePlayerScrollMediaFileListBox->mflistbox, StorageGetDrivePath(storageType));
        }

        imagePlayerLoading = imagePlayerLoaded = false;
        if (imagePlayerDataSize > 0)
        {
            free(imagePlayerData);
            imagePlayerData = NULL;
            imagePlayerDataSize = 0;
        }
    }

    ituWidgetSetVisible(imagePlayerViewIcon, false);
    ituWidgetSetVisible(imagePlayerViewButton, false);

    return true;
}

bool ImagePlayerOnLeave(ITUWidget* widget, char* param)
{
    return true;
}

void ImagePlayerReset(void)
{
    imagePlayerStorageSprite = NULL;
}
