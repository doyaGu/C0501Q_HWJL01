#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "airconditioner.h"

extern ExternalMachine engineerMachine;

static ITUText* engineerMachine1Text;
static ITUText* engineerMachine2Text;
static ITUText* engineerMachine3Text;
static ITUText* engineerMachine4Text;
static ITUText* engineerMachine5Text;
static ITUText* engineerMachine6Text;
static ITUText* engineerMachine7Text;
static ITUText* engineerMachine8Text;
static ITUText* engineerMachine9Text;
static ITUText* engineerMachine10Text;
static ITUText* engineerMachine11Text;
static ITUText* engineerMachine12Text;
static ITUText* engineerMachine13Text;
static ITUText* engineerMachine14Text;
static ITUText* engineerMachine15Text;
static ITUText* engineerMachine16Text;
static ITUText* engineerMachine17Text;
static ITUText* engineerMachine18Text;
static ITUText* engineerMachine19Text;
static ITUText* engineerMachine20Text;
static ITUText* engineerMachine21Text;
static ITUText* engineerMachine22Text;

static void EngineerMachineUpdateMachineStatus(void)
{
    char buf[32];

    sprintf(buf, "%d", engineerMachine.machineCodes[0]);
    ituTextSetString(engineerMachine1Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[1]);
    ituTextSetString(engineerMachine2Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[2]);
    ituTextSetString(engineerMachine3Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[3]);
    ituTextSetString(engineerMachine4Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[4]);
    ituTextSetString(engineerMachine5Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[5]);
    ituTextSetString(engineerMachine6Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[6]);
    ituTextSetString(engineerMachine7Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[7]);
    ituTextSetString(engineerMachine8Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[8]);
    ituTextSetString(engineerMachine9Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[9]);
    ituTextSetString(engineerMachine10Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[10]);
    ituTextSetString(engineerMachine11Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[11]);
    ituTextSetString(engineerMachine12Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[12]);
    ituTextSetString(engineerMachine13Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[13]);
    ituTextSetString(engineerMachine14Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[14]);
    ituTextSetString(engineerMachine15Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[15]);
    ituTextSetString(engineerMachine16Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[16]);
    ituTextSetString(engineerMachine17Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[17]);
    ituTextSetString(engineerMachine18Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[18]);
    ituTextSetString(engineerMachine19Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[19]);
    ituTextSetString(engineerMachine20Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[20]);
    ituTextSetString(engineerMachine21Text, buf);

    sprintf(buf, "%d", engineerMachine.machineCodes[21]);
    ituTextSetString(engineerMachine22Text, buf);
}

bool EngineerMachineOnTimer(ITUWidget* widget, char* param)
{
    if (memcmp(&engineerMachine, &extMachine, sizeof(ExternalMachine)))
    {
        memcpy(&engineerMachine, &extMachine, sizeof(ExternalMachine));
        EngineerMachineUpdateMachineStatus();
        return true;
    }
    return false;
}

bool EngineerMachineOnEnter(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    if (!engineerMachine1Text)
    {
        engineerMachine1Text = ituSceneFindWidget(&theScene, "engineerMachine1Text");
        assert(engineerMachine1Text);

        engineerMachine2Text = ituSceneFindWidget(&theScene, "engineerMachine2Text");
        assert(engineerMachine2Text);

        engineerMachine3Text = ituSceneFindWidget(&theScene, "engineerMachine3Text");
        assert(engineerMachine3Text);

        engineerMachine4Text = ituSceneFindWidget(&theScene, "engineerMachine4Text");
        assert(engineerMachine4Text);

        engineerMachine5Text = ituSceneFindWidget(&theScene, "engineerMachine5Text");
        assert(engineerMachine5Text);

        engineerMachine6Text = ituSceneFindWidget(&theScene, "engineerMachine6Text");
        assert(engineerMachine6Text);

        engineerMachine7Text = ituSceneFindWidget(&theScene, "engineerMachine7Text");
        assert(engineerMachine7Text);

        engineerMachine8Text = ituSceneFindWidget(&theScene, "engineerMachine8Text");
        assert(engineerMachine8Text);

        engineerMachine9Text = ituSceneFindWidget(&theScene, "engineerMachine9Text");
        assert(engineerMachine9Text);

        engineerMachine10Text = ituSceneFindWidget(&theScene, "engineerMachine10Text");
        assert(engineerMachine10Text);

        engineerMachine11Text = ituSceneFindWidget(&theScene, "engineerMachine11Text");
        assert(engineerMachine11Text);

        engineerMachine12Text = ituSceneFindWidget(&theScene, "engineerMachine12Text");
        assert(engineerMachine12Text);

        engineerMachine13Text = ituSceneFindWidget(&theScene, "engineerMachine13Text");
        assert(engineerMachine13Text);

        engineerMachine14Text = ituSceneFindWidget(&theScene, "engineerMachine14Text");
        assert(engineerMachine14Text);

        engineerMachine15Text = ituSceneFindWidget(&theScene, "engineerMachine15Text");
        assert(engineerMachine15Text);

        engineerMachine16Text = ituSceneFindWidget(&theScene, "engineerMachine16Text");
        assert(engineerMachine16Text);

        engineerMachine17Text = ituSceneFindWidget(&theScene, "engineerMachine17Text");
        assert(engineerMachine17Text);

        engineerMachine18Text = ituSceneFindWidget(&theScene, "engineerMachine18Text");
        assert(engineerMachine18Text);

        engineerMachine19Text = ituSceneFindWidget(&theScene, "engineerMachine19Text");
        assert(engineerMachine19Text);

        engineerMachine20Text = ituSceneFindWidget(&theScene, "engineerMachine20Text");
        assert(engineerMachine20Text);

        engineerMachine21Text = ituSceneFindWidget(&theScene, "engineerMachine21Text");
        assert(engineerMachine21Text);

        engineerMachine22Text = ituSceneFindWidget(&theScene, "engineerMachine22Text");
        assert(engineerMachine22Text);
    }
    memcpy(&engineerMachine, &extMachine, sizeof(ExternalMachine));
    EngineerMachineUpdateMachineStatus();

    ev.type = EXTERNAL_ENG_POWER_MACHINE_MODE;
    ev.arg1 = 1;
    ExternalSend(&ev);

    return true;
}

bool EngineerMachineOnLeave(ITUWidget* widget, char* param)
{
    ExternalEvent ev;

    ev.type = EXTERNAL_ENG_POWER_MACHINE_MODE;
    ev.arg1 = 0;
    ExternalSend(&ev);

    return true;
}

void EngineerMachineReset(void)
{
    engineerMachine1Text = NULL;
}
