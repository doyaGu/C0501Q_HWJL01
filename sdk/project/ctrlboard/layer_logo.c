#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"

static ITULayer* touchCalibrationLayer;

bool LogoOnEnter(ITUWidget* widget, char* param)
{
    if (!touchCalibrationLayer)
    {
        touchCalibrationLayer = ituSceneFindWidget(&theScene, "touchCalibrationLayer");
        assert(touchCalibrationLayer);
    }
    if(theConfig.touch_calibration)
    {
		ituLayerGoto(touchCalibrationLayer);
        return false;
    }
    return true;
}

void LogoReset(void)
{
    touchCalibrationLayer = NULL;
}
