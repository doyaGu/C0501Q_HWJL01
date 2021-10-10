#include <assert.h>
#include <stdlib.h>
#include "scene.h"
#include "sports_equipment.h"

static ITURadioBox* programType1IconRadioBox;
static ITURadioBox* programType2IconRadioBox;
static ITURadioBox* programType3IconRadioBox;
static ITURadioBox* programType4IconRadioBox;
static ITURadioBox* programType5IconRadioBox;
static ITURadioBox* programType6IconRadioBox;
static ITURadioBox* programType7IconRadioBox;
static ITURadioBox* programType8IconRadioBox;
static ITURadioBox* programType9IconRadioBox;
static ITURadioBox* programType10IconRadioBox;
static ITURadioBox* programType11IconRadioBox;
static ITURadioBox* programType12IconRadioBox;
static ITURadioBox* programType1TextRadioBox;
static ITURadioBox* programType2TextRadioBox;
static ITURadioBox* programType3TextRadioBox;
static ITURadioBox* programType4TextRadioBox;
static ITURadioBox* programType5TextRadioBox;
static ITURadioBox* programType6TextRadioBox;
static ITURadioBox* programType7TextRadioBox;
static ITURadioBox* programType8TextRadioBox;
static ITURadioBox* programType9TextRadioBox;
static ITURadioBox* programType10TextRadioBox;
static ITURadioBox* programType11TextRadioBox;
static ITURadioBox* programType12TextRadioBox;
static ITUSprite* programDialogSprite;
static ITUWheel* programTimeWheel;
static ITUWheel* programDistanceWheel;
static ITUWheel* programCalorieWheel;
static ITUWheel* programPulseWheel;
static ITUContainer* programTimeContainer;
static ITUContainer* programDistanceContainer;
static ITUContainer* programCalorieContainer;
static ITUContainer* programPulseContainer;

bool ProgramTypeRadioBoxOnPress(ITUWidget* widget, char* param)
{
    if (ituRadioBoxIsChecked(programType1IconRadioBox))
    {
        ProgramSetType(PROG_AEROBIC);
    }
    else if (ituRadioBoxIsChecked(programType2IconRadioBox))
    {
        ProgramSetType(PROG_INTERMITTENT);
    }
    else if (ituRadioBoxIsChecked(programType3IconRadioBox))
    {
        ProgramSetType(PROG_TIME);
		ituWidgetSetVisible(programTimeContainer, true);
		ituWidgetSetVisible(programDistanceContainer, false);
		ituWidgetSetVisible(programCalorieContainer, false);
		ituWidgetSetVisible(programPulseContainer, false);
    }
    else if (ituRadioBoxIsChecked(programType4IconRadioBox))
    {
        ProgramSetType(PROG_DISTANCE);
		ituWidgetSetVisible(programTimeContainer, false);
		ituWidgetSetVisible(programDistanceContainer, true);
		ituWidgetSetVisible(programCalorieContainer, false);
		ituWidgetSetVisible(programPulseContainer, false);
    }
    else if (ituRadioBoxIsChecked(programType5IconRadioBox))
    {
        ProgramSetType(PROG_MARATHON);
    }
    else if (ituRadioBoxIsChecked(programType6IconRadioBox))
    {
        ProgramSetType(PROG_HALF_MARATHON);
    }
    else if (ituRadioBoxIsChecked(programType7IconRadioBox))
    {
        ProgramSetType(PROG_FAT_BURNING);
    }
    else if (ituRadioBoxIsChecked(programType8IconRadioBox))
    {
        ProgramSetType(PROG_BUFFER);
    }
    else if (ituRadioBoxIsChecked(programType9IconRadioBox))
    {
        ProgramSetType(PROG_CALORIE);
		ituWidgetSetVisible(programTimeContainer, false);
		ituWidgetSetVisible(programDistanceContainer, false);
		ituWidgetSetVisible(programCalorieContainer, true);
		ituWidgetSetVisible(programPulseContainer, false);
    }
    else if (ituRadioBoxIsChecked(programType10IconRadioBox))
    {
        ProgramSetType(PROG_PULSE);
		ituWidgetSetVisible(programTimeContainer, false);
		ituWidgetSetVisible(programDistanceContainer, false);
		ituWidgetSetVisible(programCalorieContainer, false);
		ituWidgetSetVisible(programPulseContainer, true);
    }
    else if (ituRadioBoxIsChecked(programType11IconRadioBox))
    {
        ProgramSetType(PROG_QUARTER_MARATHON);
    }
    else if (ituRadioBoxIsChecked(programType12IconRadioBox))
    {
        ProgramSetType(PROG_MINI_MARATHON);
    }
    return true;
}

bool ProgramDialogExitButtonOnPress(ITUWidget* widget, char* param)
{
    if (programDialogSprite->frame == 0)
    {
        theProgram.time = (programTimeWheel->focusIndex + PROG_MIN_TIME) * 60;
    }
    else if (programDialogSprite->frame == 1)
    {
        theProgram.distance = (programDistanceWheel->focusIndex + PROG_MIN_DISTANCE) * 1000;
    }
    else if (programDialogSprite->frame == 2)
    {
        theProgram.calorie = programCalorieWheel->focusIndex * 10 + PROG_MIN_CALORIE;
    }
    else if (programDialogSprite->frame == 3)
    {
        theProgram.pulse = programPulseWheel->focusIndex + PROG_MIN_PULSE;
    }
    return true;
}

bool ProgramOnEnter(ITUWidget* widget, char* param)
{
    if (!programType1IconRadioBox)
    {
        programType1IconRadioBox = ituSceneFindWidget(&theScene, "programType1IconRadioBox");
        assert(programType1IconRadioBox);

        programType2IconRadioBox = ituSceneFindWidget(&theScene, "programType2IconRadioBox");
        assert(programType2IconRadioBox);

        programType3IconRadioBox = ituSceneFindWidget(&theScene, "programType3IconRadioBox");
        assert(programType3IconRadioBox);

        programType4IconRadioBox = ituSceneFindWidget(&theScene, "programType4IconRadioBox");
        assert(programType4IconRadioBox);

        programType5IconRadioBox = ituSceneFindWidget(&theScene, "programType5IconRadioBox");
        assert(programType5IconRadioBox);

        programType6IconRadioBox = ituSceneFindWidget(&theScene, "programType6IconRadioBox");
        assert(programType6IconRadioBox);

        programType7IconRadioBox = ituSceneFindWidget(&theScene, "programType7IconRadioBox");
        assert(programType7IconRadioBox);

        programType8IconRadioBox = ituSceneFindWidget(&theScene, "programType8IconRadioBox");
        assert(programType8IconRadioBox);

        programType9IconRadioBox = ituSceneFindWidget(&theScene, "programType9IconRadioBox");
        assert(programType9IconRadioBox);

        programType10IconRadioBox = ituSceneFindWidget(&theScene, "programType10IconRadioBox");
        assert(programType10IconRadioBox);

        programType11IconRadioBox = ituSceneFindWidget(&theScene, "programType11IconRadioBox");
        assert(programType11IconRadioBox);

        programType12IconRadioBox = ituSceneFindWidget(&theScene, "programType12IconRadioBox");
        assert(programType12IconRadioBox);

        programType1TextRadioBox = ituSceneFindWidget(&theScene, "programType1TextRadioBox");
        assert(programType1TextRadioBox);

        programType2TextRadioBox = ituSceneFindWidget(&theScene, "programType2TextRadioBox");
        assert(programType2TextRadioBox);

        programType3TextRadioBox = ituSceneFindWidget(&theScene, "programType3TextRadioBox");
        assert(programType3TextRadioBox);

        programType4TextRadioBox = ituSceneFindWidget(&theScene, "programType4TextRadioBox");
        assert(programType4TextRadioBox);

        programType5TextRadioBox = ituSceneFindWidget(&theScene, "programType5TextRadioBox");
        assert(programType5TextRadioBox);

        programType6TextRadioBox = ituSceneFindWidget(&theScene, "programType6TextRadioBox");
        assert(programType6TextRadioBox);

        programType7TextRadioBox = ituSceneFindWidget(&theScene, "programType7TextRadioBox");
        assert(programType7TextRadioBox);

        programType8TextRadioBox = ituSceneFindWidget(&theScene, "programType8TextRadioBox");
        assert(programType8TextRadioBox);

        programType9TextRadioBox = ituSceneFindWidget(&theScene, "programType9TextRadioBox");
        assert(programType9TextRadioBox);

        programType10TextRadioBox = ituSceneFindWidget(&theScene, "programType10TextRadioBox");
        assert(programType10TextRadioBox);

        programType11TextRadioBox = ituSceneFindWidget(&theScene, "programType11TextRadioBox");
        assert(programType11TextRadioBox);

        programType12TextRadioBox = ituSceneFindWidget(&theScene, "programType12TextRadioBox");
        assert(programType12TextRadioBox);

        programDialogSprite = ituSceneFindWidget(&theScene, "programDialogSprite");
        assert(programDialogSprite);

        programTimeWheel = ituSceneFindWidget(&theScene, "programTimeWheel");
        assert(programTimeWheel);

        programDistanceWheel = ituSceneFindWidget(&theScene, "programDistanceWheel");
        assert(programDistanceWheel);

        programCalorieWheel = ituSceneFindWidget(&theScene, "programCalorieWheel");
        assert(programCalorieWheel);

        programPulseWheel = ituSceneFindWidget(&theScene, "programPulseWheel");
        assert(programPulseWheel);

		programTimeContainer = ituSceneFindWidget(&theScene, "programTimeContainer");
		assert(programTimeContainer);

		programDistanceContainer = ituSceneFindWidget(&theScene, "programDistanceContainer");
		assert(programDistanceContainer);

		programCalorieContainer = ituSceneFindWidget(&theScene, "programCalorieContainer");
		assert(programCalorieContainer);

		programPulseContainer = ituSceneFindWidget(&theScene, "programPulseContainer");
		assert(programPulseContainer);
    }

    switch (theProgram.prog)
    {
    case PROG_AEROBIC:
        ituRadioBoxSetChecked(programType1IconRadioBox, true);
        ituRadioBoxSetChecked(programType1TextRadioBox, true);
        break;

    case PROG_INTERMITTENT:
        ituRadioBoxSetChecked(programType2IconRadioBox, true);
        ituRadioBoxSetChecked(programType2TextRadioBox, true);
        break;

    case PROG_TIME:
        ituRadioBoxSetChecked(programType3IconRadioBox, true);
        ituRadioBoxSetChecked(programType3TextRadioBox, true);
        break;

    case PROG_DISTANCE:
        ituRadioBoxSetChecked(programType4IconRadioBox, true);
        ituRadioBoxSetChecked(programType4TextRadioBox, true);
        break;

    case PROG_MARATHON:
        ituRadioBoxSetChecked(programType5IconRadioBox, true);
        ituRadioBoxSetChecked(programType5TextRadioBox, true);
        break;

    case PROG_HALF_MARATHON:
        ituRadioBoxSetChecked(programType6IconRadioBox, true);
        ituRadioBoxSetChecked(programType6TextRadioBox, true);
        break;

    case PROG_FAT_BURNING:
        ituRadioBoxSetChecked(programType7IconRadioBox, true);
        ituRadioBoxSetChecked(programType7TextRadioBox, true);
        break;

    case PROG_BUFFER:
        ituRadioBoxSetChecked(programType8IconRadioBox, true);
        ituRadioBoxSetChecked(programType8TextRadioBox, true);
        break;

    case PROG_CALORIE:
        ituRadioBoxSetChecked(programType9IconRadioBox, true);
        ituRadioBoxSetChecked(programType9TextRadioBox, true);
        break;

    case PROG_PULSE:
        ituRadioBoxSetChecked(programType10IconRadioBox, true);
        ituRadioBoxSetChecked(programType10TextRadioBox, true);
        break;

    case PROG_QUARTER_MARATHON:
        ituRadioBoxSetChecked(programType11IconRadioBox, true);
        ituRadioBoxSetChecked(programType11TextRadioBox, true);
        break;

    case PROG_MINI_MARATHON:
        ituRadioBoxSetChecked(programType12IconRadioBox, true);
        ituRadioBoxSetChecked(programType12TextRadioBox, true);
        break;
    }

    ituWheelGoto(programTimeWheel, theProgram.time / 60 - PROG_MIN_TIME);
    ituWheelGoto(programDistanceWheel, theProgram.distance / 1000 - PROG_MIN_DISTANCE);
    ituWheelGoto(programCalorieWheel, (theProgram.calorie - PROG_MIN_CALORIE) / 10);
    ituWheelGoto(programPulseWheel, theProgram.pulse - PROG_MIN_PULSE);

    return true;
}

bool ProgramOnLeave(ITUWidget* widget, char* param)
{
    return true;
}

void ProgramReset(void)
{
    programType1IconRadioBox = NULL;
}
