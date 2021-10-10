#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"

static ITUPopupButton* buttonTVPopupButton;
static ITUPopupButton* buttonAirConditionerPopupButton;
static ITUSprite* buttonBootAnimationSprite;
static ITUCoverFlow* buttonCoverFlow;
static ITUTextBox* buttonUartTextBox;

bool ButtonPlugPopupButtonOnPress(ITUWidget* widget, char* param)
{
    // TODO: IMPLEMENT
    printf("%s: %s\n", __FUNCTION__, param);
    return true;
}

bool ButtonLightPopupButtonOnPress(ITUWidget* widget, char* param)
{
    // TODO: IMPLEMENT
    printf("%s: %s\n", __FUNCTION__, param);
    return true;
}

bool ButtonTVPopupButtonOnPress(ITUWidget* widget, char* param)
{
    // TODO: IMPLEMENT
    printf("%s: %s\n", __FUNCTION__, param);
    return true;
}

bool ButtonAirConditionerPopupButtonOnPress(ITUWidget* widget, char* param)
{
    // TODO: IMPLEMENT
    printf("%s: %s\n", __FUNCTION__, param);
    return true;
}

static void ButtonBootAnimationSpriteOnStop(ITUSprite* sprite)
{
    ituWidgetSetVisible(sprite, false);
}

bool ButtonSpiReadButtonOnPress(ITUWidget* widget, char* param)
{
    // TODO: IMPLEMENT
    return true;
}

bool ButtonSpiWriteButtonOnPress(ITUWidget* widget, char* param)
{
    // TODO: IMPLEMENT
    return true;
}

bool ButtonI2cReadButtonOnPress(ITUWidget* widget, char* param)
{
    // TODO: IMPLEMENT
    return true;
}

bool ButtonI2cWriteButtonOnPress(ITUWidget* widget, char* param)
{
    // TODO: IMPLEMENT
    return true;
}

bool ButtonUartSendButtonOnPress(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    ev.type = EXTERNAL_SHOW_MSG;
    strcpy(ev.buf1, "test");

    ExternalSend(&ev);

    return true;
}

bool ButtonOnTimer(ITUWidget* widget, char* param)
{
    // TODO: IMPLEMENT
    return false;
}

bool ButtonRemoteButtonOnPress(ITUWidget* widget, char* param)
{
    // TODO: IMPLEMENT
    return true;
}

bool ButtonUpgradeButtonOnPress(ITUWidget* widget, char* param)
{
#ifdef CFG_NET_ENABLE
    if (NetworkIsReady())
        UpgradeSetUrl(CFG_UPGRADE_FTP_URL);
    else
        UpgradeSetUrl(NULL);
#else
    UpgradeSetUrl(NULL);
#endif
    SceneQuit(QUIT_UPGRADE_FIRMWARE);
    return true;
}

bool ButtonOnEnter(ITUWidget* widget, char* param)
{
    if (!buttonTVPopupButton)
    {
        buttonTVPopupButton = ituSceneFindWidget(&theScene, "buttonTVPopupButton");
        assert(buttonTVPopupButton);

        buttonAirConditionerPopupButton = ituSceneFindWidget(&theScene, "buttonAirConditionerPopupButton");
        assert(buttonAirConditionerPopupButton);

        buttonBootAnimationSprite = ituSceneFindWidget(&theScene, "buttonBootAnimationSprite");
        assert(buttonBootAnimationSprite);
        ituSpriteSetOnStop(buttonBootAnimationSprite, ButtonBootAnimationSpriteOnStop);

        buttonCoverFlow = ituSceneFindWidget(&theScene, "buttonCoverFlow");
        assert(buttonCoverFlow);

        buttonUartTextBox = ituSceneFindWidget(&theScene, "buttonUartTextBox");
        assert(buttonUartTextBox);
    }
    return true;
}

void ButtonReset(void)
{
}
