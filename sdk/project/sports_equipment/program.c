#include <stdio.h>
#include "sports_equipment.h"

Program theProgram;

void ProgramInit(void)
{
    theProgram.prog = PROG_AEROBIC;
    theProgram.time = 10 * 60;
    theProgram.distance = 6000 * theProgram.time / (60 * 60);
	theProgram.calorie = 180 * theProgram.time / (60 * 60);
    theProgram.pulse = theConfig.pulse;
    theProgram.pace = theProgram.distance;

	if (theProgram.time < PROG_MIN_TIME)
		theProgram.time = PROG_MIN_TIME;

	if (theProgram.distance < PROG_MIN_DISTANCE)
		theProgram.distance = PROG_MIN_DISTANCE;

    theProgram.calorie -= theProgram.calorie % 10;

	if (theProgram.calorie < PROG_MIN_CALORIE)
		theProgram.calorie = PROG_MIN_CALORIE;

	if (theProgram.pulse < PROG_MIN_PULSE)
		theProgram.pulse = PROG_MIN_PULSE;

	theProgram.restart = false;
}

void ProgramSetType(ProgramType prog)
{
    theProgram.prog = prog;
	theProgram.time = theConfig.goal_time;
	theProgram.distance = theConfig.distance;
	theProgram.calorie = theConfig.calorie;
	theProgram.pulse = theConfig.pulse;
	theProgram.pace = theConfig.pace;

    switch (prog)
    {
	case PROG_AEROBIC:
		theProgram.time = 10 * 60;
        theProgram.distance = 6000 * theProgram.time / (60 * 60);
	    theProgram.calorie = 180 * theProgram.time / (60 * 60);
	    theProgram.pace = theProgram.distance;
		break;

	case PROG_INTERMITTENT:
		theProgram.distance = 1000;
        theProgram.time = 60 * 60 * theProgram.distance / 6 / 1000;
	    theProgram.calorie = 180 * theProgram.time / (60 * 60);
	    theProgram.pace = theProgram.distance;
		break;

    case PROG_TIME:
        theProgram.distance = 6000 * theProgram.time / (60 * 60);
	    theProgram.calorie = 180 * theProgram.time / (60 * 60);
	    theProgram.pace = theProgram.distance;
        break;

    case PROG_DISTANCE:
        theProgram.time = 60 * 60 * theProgram.distance / 6 / 1000;
	    theProgram.calorie = 180 * theProgram.time / (60 * 60);
	    theProgram.pace = theProgram.distance;
        break;

    case PROG_MARATHON:
        theProgram.distance = 42195;
        theProgram.time = 60 * 60 * theProgram.distance / 6 / 1000;
	    theProgram.calorie = 180 * theProgram.time / (60 * 60);
	    theProgram.pace = theProgram.distance;
        break;

    case PROG_HALF_MARATHON:
        theProgram.distance = 21098;
        theProgram.time = 60 * 60 * theProgram.distance / 6 / 1000;
	    theProgram.calorie = 180 * theProgram.time / (60 * 60);
	    theProgram.pace = theProgram.distance;
        break;

	case PROG_FAT_BURNING:
		theProgram.calorie = 100;
        theProgram.time = 60 * 60 * theProgram.calorie / 180;
        theProgram.distance = 6000 * theProgram.time / (60 * 60);
        theProgram.pace = theProgram.distance;
		break;

	case PROG_BUFFER:
		theProgram.pace = 200;
        theProgram.distance = theProgram.pace;
        theProgram.time = 60 * 60 * theProgram.distance / 6 / 1000;
	    theProgram.calorie = 180 * theProgram.time / (60 * 60);
		break;

	case PROG_CALORIE:
		theProgram.distance = 42195;
        theProgram.time = 60 * 60 * theProgram.distance / 6 / 1000;
	    theProgram.calorie = 180 * theProgram.time / (60 * 60);
	    theProgram.pace = theProgram.distance;
		break;

    case PROG_QUARTER_MARATHON:
        theProgram.distance = 10548;
        theProgram.time = 60 * 60 * theProgram.distance / 6 / 1000;
	    theProgram.calorie = 180 * theProgram.time / (60 * 60);
	    theProgram.pace = theProgram.distance;
        break;

    case PROG_MINI_MARATHON:
        theProgram.distance = 50000;
        theProgram.time = 60 * 60 * theProgram.distance / 6 / 1000;
	    theProgram.calorie = 180 * theProgram.time / (60 * 60);
	    theProgram.pace = theProgram.distance;
        break;
    }
	if (theProgram.time < PROG_MIN_TIME)
		theProgram.time = PROG_MIN_TIME;

	if (theProgram.distance < PROG_MIN_DISTANCE)
		theProgram.distance = PROG_MIN_DISTANCE;

    theProgram.calorie -= theProgram.calorie % 10;

	if (theProgram.calorie < PROG_MIN_CALORIE)
		theProgram.calorie = PROG_MIN_CALORIE;

	if (theProgram.pulse < PROG_MIN_PULSE)
		theProgram.pulse = PROG_MIN_PULSE;

	theProgram.restart = true;
}
