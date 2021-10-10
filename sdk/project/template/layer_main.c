#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL/SDL.h"
#include "scene.h"
#include "project.h"

static ITUBackgroundButton* mainBackgroundButton;

bool MainOnTimer(ITUWidget* widget, char* param)
{
    /*
    Open the below line to see the action of MainOnTimer.    
    */
        
    //printf("print this message by frame rate for %d ms\n", MS_PER_FRAME);
    return true;
}

bool MainButtonOnPress(ITUWidget* widget, char* param)
{
    /*
    Get the configuration from config.ini.
    */
    
    printf("*** Hello iTE ***\n");
    return true;
}

bool MainOnEnter(ITUWidget* widget, char* param)
{
    /*
    This is the first function when enter UI layer.
    */    
    
    if (!mainBackgroundButton)
    {
        ITUWidget* widget;

        mainBackgroundButton = ituSceneFindWidget(&theScene, "mainBackgroundButton");
        assert(mainBackgroundButton);

        widget = (ITUWidget*)mainBackgroundButton;
    }

    return true;
}

void MainReset(void)
{
    /*
    This function is effected only when change itu, to reset all global values.
    */
    
    mainBackgroundButton = NULL;
}
