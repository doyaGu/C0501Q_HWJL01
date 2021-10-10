#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"
#include "scene.h"
#include "ctrlboard.h"

extern int Castor3_CalibrateTouch(float screenX[], float screenY[], float touchX[], float touchY[]);
extern void Castor3_ChangeTouchMode(int raw);
extern void Castor3_ReadTouchRawPosition(float* x, float* y);

static ITUCheckBox* touchCalibration1CheckBox;
static ITUCheckBox* touchCalibration2CheckBox;
static ITUCheckBox* touchCalibration3CheckBox;
static ITUCheckBox* touchCalibration4CheckBox;
static ITUCheckBox* touchCalibration5CheckBox;

static float screenX[5], screenY[5], touchX[5], touchY[5];

bool TouchCalibrationCheckBoxOnPress(ITUWidget* widget, char* param)
{
    float x = 0.0f, y = 0.0f;
    ITUCheckBox* checkbox = (ITUCheckBox*)widget;
    
    ituCheckBoxSetChecked(checkbox, true);
#ifdef CFG_TOUCH_ENABLE
    Castor3_ReadTouchRawPosition(&x, &y);
#endif

    if (checkbox == touchCalibration1CheckBox)
    {
        touchX[0] = x;
        touchY[0] = y;
    }
    else if (checkbox == touchCalibration2CheckBox)
    {
        touchX[1] = x;
        touchY[1] = y;
    }
    else if (checkbox == touchCalibration3CheckBox)
    {
        touchX[2] = x;
        touchY[2] = y;
    }
    else if (checkbox == touchCalibration4CheckBox)
    {
        touchX[3] = x;
        touchY[3] = y;
    }
    else if (checkbox == touchCalibration5CheckBox)
    {
        touchX[4] = x;
        touchY[4] = y;
    }

    if (ituCheckBoxIsChecked(touchCalibration1CheckBox) && 
        ituCheckBoxIsChecked(touchCalibration2CheckBox) && 
        ituCheckBoxIsChecked(touchCalibration3CheckBox) && 
        ituCheckBoxIsChecked(touchCalibration4CheckBox) && 
        ituCheckBoxIsChecked(touchCalibration5CheckBox))
    {
    #ifdef CFG_TOUCH_ENABLE
        if (Castor3_CalibrateTouch(screenX, screenY, touchX, touchY) == 0)
        {
            theConfig.touch_calibration = 0;
            ConfigSave();
            itp_codec_standby();
            sleep(2);
            exit(0);
        }
        else
    #endif // CFG_TOUCH_ENABLE
        {
            ituCheckBoxSetChecked(touchCalibration1CheckBox, false);
            ituCheckBoxSetChecked(touchCalibration2CheckBox, false);
            ituCheckBoxSetChecked(touchCalibration3CheckBox, false);
            ituCheckBoxSetChecked(touchCalibration4CheckBox, false);
            ituCheckBoxSetChecked(touchCalibration5CheckBox, false);
        }
    }
	return true;
}

bool TouchCalibrationOnEnter(ITUWidget* widget, char* param)
{
    int x, y;

    if (!touchCalibration1CheckBox)
    {
        touchCalibration1CheckBox = ituSceneFindWidget(&theScene, "touchCalibration1CheckBox");
        assert(touchCalibration1CheckBox);

        touchCalibration2CheckBox = ituSceneFindWidget(&theScene, "touchCalibration2CheckBox");
        assert(touchCalibration2CheckBox);

        touchCalibration3CheckBox = ituSceneFindWidget(&theScene, "touchCalibration3CheckBox");
        assert(touchCalibration3CheckBox);

        touchCalibration4CheckBox = ituSceneFindWidget(&theScene, "touchCalibration4CheckBox");
        assert(touchCalibration4CheckBox);

        touchCalibration5CheckBox = ituSceneFindWidget(&theScene, "touchCalibration5CheckBox");
        assert(touchCalibration5CheckBox);
    }
    ituCheckBoxSetChecked(touchCalibration1CheckBox, false);
    ituCheckBoxSetChecked(touchCalibration2CheckBox, false);
    ituCheckBoxSetChecked(touchCalibration3CheckBox, false);
    ituCheckBoxSetChecked(touchCalibration4CheckBox, false);
    ituCheckBoxSetChecked(touchCalibration5CheckBox, false);

    ituWidgetGetGlobalPosition(touchCalibration1CheckBox, &x, &y);
    screenX[0] = (float) (x + ituWidgetGetWidth(touchCalibration1CheckBox) / 2);
    screenY[0] = (float) (y + ituWidgetGetHeight(touchCalibration1CheckBox) / 2);

    ituWidgetGetGlobalPosition(touchCalibration2CheckBox, &x, &y);
    screenX[1] = (float) (x + ituWidgetGetWidth(touchCalibration2CheckBox) / 2);
    screenY[1] = (float) (y + ituWidgetGetHeight(touchCalibration2CheckBox) / 2);

    ituWidgetGetGlobalPosition(touchCalibration3CheckBox, &x, &y);
    screenX[2] = (float) (x + ituWidgetGetWidth(touchCalibration3CheckBox) / 2);
    screenY[2] = (float) (y + ituWidgetGetHeight(touchCalibration3CheckBox) / 2);

    ituWidgetGetGlobalPosition(touchCalibration4CheckBox, &x, &y);
    screenX[3] = (float) (x + ituWidgetGetWidth(touchCalibration4CheckBox) / 2);
    screenY[3] = (float) (y + ituWidgetGetHeight(touchCalibration4CheckBox) / 2);

    ituWidgetGetGlobalPosition(touchCalibration5CheckBox, &x, &y);
    screenX[4] = (float) (x + ituWidgetGetWidth(touchCalibration5CheckBox) / 2);
    screenY[4] = (float) (y + ituWidgetGetHeight(touchCalibration5CheckBox) / 2);

#ifdef CFG_TOUCH_ENABLE
    Castor3_ChangeTouchMode(1);
#endif
	return true;
}

bool TouchCalibrationOnLeave(ITUWidget* widget, char* param)
{
#ifdef CFG_TOUCH_ENABLE
    Castor3_ChangeTouchMode(0);
#endif
	return true;
}

void TouchCalibrationReset(void)
{
    touchCalibration1CheckBox = NULL;
}
