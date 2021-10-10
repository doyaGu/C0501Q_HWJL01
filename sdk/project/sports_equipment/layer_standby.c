#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "sports_equipment.h"

ITUBackground* standbyBackground;

bool StandbyStartPopupButtonOnPress(ITUWidget* widget, char* param)
{
    theProgram.restart = true;
    return true;
}

bool StandbyOnEnter(ITUWidget* widget, char* param)
{
    if (!standbyBackground)
    {
        standbyBackground = ituSceneFindWidget(&theScene, "standbyBackground");
        assert(standbyBackground);

        theProgram.pulse = theConfig.pulse;

        SceneSetReady(true);
    }
    return true;
}

void StandbyReset(void)
{
    standbyBackground = NULL;
}
