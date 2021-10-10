#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"

#define CALLS_PER_SEC (1000 / MS_PER_FRAME)

static ITUBackground* meterGraphBackground;
static ITUMeter* meterValueMeter;
static ITUTrackBar* meterSpeedTrackBar;
static ITUProgressBar* meterSpeedProgressBar;

static int meterValue;
static int meterSpeed;
static int meterPosition;
static int meterGraphBackgroundWidth;

bool MeterValueMeterOnChanged(ITUWidget* widget, char* param)
{
    meterValue = atoi(param);
    return true;
}

bool MeterSpeedTrackBarOnChanged(ITUWidget* widget, char* param)
{
    meterSpeed = atoi(param);

    if (meterSpeed < 10)
        meterSpeed = 10;

    return true;
}

bool MeterOnTimer(ITUWidget* widget, char* param)
{
    int unit = meterGraphBackgroundWidth / CALLS_PER_SEC;

    meterPosition += unit * meterSpeed / 10;

    if (meterPosition >= meterGraphBackgroundWidth)
        meterPosition = 0;

    return true;
}

static void MeterGraphBackgroundDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    int destx, desty, i, ir0;
    ITURectangle prevClip;
    ITUBackground* bg = (ITUBackground*) widget;
    ITURectangle* rect = (ITURectangle*) &widget->rect;
    ITUColor color = { 255, 255, 255, 0 };
    float v0, r0;
    assert(bg);
    assert(dest);

    ituBackgroundDraw(widget, dest, x, y, alpha);

    destx = rect->x + x;
    desty = rect->y + y;

    ituWidgetSetClipping(widget, dest, x, y, &prevClip);

    v0 = 2 * (float)M_PI * (0 + meterPosition) / meterGraphBackgroundWidth;
    r0 = sinf(v0) * meterValue;
    r0 *= rect->height / 2.0f;
    r0 /= 100.0f;
    ir0 = (int)roundf(r0);

    for (i = 0; i < rect->width; ++i)
    {
        int ir;
        float v = 2 * (float)M_PI * (i + meterPosition + 1) / meterGraphBackgroundWidth;
        float r = sinf(v) * meterValue;
        r *= rect->height / 2.0f;
        r /= 100.0f;

        ir = (int)roundf(r);

        if (ir0 < ir)
        {
            ituColorFill(dest, destx + i, desty + rect->height / 2 - ir, 1, ir - ir0, &color);
        }
        else if (ir0 > ir)
        {
            ituColorFill(dest, destx + i, desty + rect->height / 2 - ir0, 1, ir0 - ir, &color);
        }
        else
        {
            ituColorFill(dest, destx + i, desty + rect->height / 2 - ir0, 1, 1, &color);
        }
        ir0 = ir;
    }

    ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
}

bool MeterOnEnter(ITUWidget* widget, char* param)
{
    if (!meterGraphBackground)
    {
        meterGraphBackground = ituSceneFindWidget(&theScene, "meterGraphBackground");
        assert(meterGraphBackground);
        ituWidgetSetDraw(meterGraphBackground, MeterGraphBackgroundDraw);

        meterValueMeter = ituSceneFindWidget(&theScene, "meterValueMeter");
        assert(meterValueMeter);

        meterSpeedTrackBar = ituSceneFindWidget(&theScene, "meterSpeedTrackBar");
        assert(meterSpeedTrackBar);

        meterSpeedProgressBar = ituSceneFindWidget(&theScene, "meterSpeedProgressBar");
        assert(meterSpeedProgressBar);
    }
    ituMeterSetValue(meterValueMeter, 0);
    ituTrackBarSetValue(meterSpeedTrackBar, 0);
    ituProgressBarSetValue(meterSpeedProgressBar, 0);
    meterSpeed = 10;
    meterValue = meterPosition = 0;
    meterGraphBackgroundWidth = ituWidgetGetWidth(meterGraphBackground);

    return true;
}

void MeterReset(void)
{
    meterGraphBackground = NULL;
}
