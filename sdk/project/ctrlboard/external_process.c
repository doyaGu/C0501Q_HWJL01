#include <assert.h>
#include <stdlib.h>
#include "SDL/SDL.h"
#include "ctrlboard.h"
#include "scene.h"

static ITULayer* buttonLayer;
static ITUCoverFlow* buttonCoverFlow;

static void ExternalProcessUart(ExternalEvent* ev)
{
    if (!ituWidgetIsVisible(buttonLayer))
        ituLayerGoto(buttonLayer);
    
    ituCoverFlowGoto(buttonCoverFlow, 2);
    ituSceneSendEvent(&theScene, EVENT_CUSTOM_UART, ev->buf1);
}

void ExternalProcessEvent(ExternalEvent* ev)
{
    char buf[64];
    assert(ev);

    switch (ev->type)
    {
    case EXTERNAL_SHOW_MSG:
        printf("EXTERNAL_SHOW_MSG\n");
        ExternalProcessUart(ev);
        break;

    case EXTERNAL_TEST0:
        printf("EXTERNAL_TEST0\n");
        ituSceneSendEvent(&theScene, EVENT_CUSTOM_KEY0, NULL);
        break;

    case EXTERNAL_TEST1:
        printf("EXTERNAL_TEST1: %d\n", ev->arg1);
        ituSceneSendEvent(&theScene, EVENT_CUSTOM_KEY1, NULL);
        break;

    case EXTERNAL_TEST2:
        printf("EXTERNAL_TEST2: %d %d\n", ev->arg1, ev->arg2);
        sprintf(buf, "%d %d", ev->arg1, ev->arg2);
        ituSceneSendEvent(&theScene, EVENT_CUSTOM_KEY2, buf);
        break;

    case EXTERNAL_TEST3:
        printf("EXTERNAL_TEST3: %d %d %d\n", ev->arg1, ev->arg2, ev->arg3);
        sprintf(buf, "%d %d %d", ev->arg1, ev->arg2, ev->arg3);
        ituSceneSendEvent(&theScene, EVENT_CUSTOM_KEY1, buf);
        break;

    case EXTERNAL_TEST4:
        printf("EXTERNAL_TEST4: %d\n", ev->arg1);
        break;

    case EXTERNAL_TEST5:
        printf("EXTERNAL_TEST5: %d\n", ev->arg1);
        break;
    }
}

void ExternalProcessInit(void)
{
    buttonLayer = ituSceneFindWidget(&theScene, "buttonLayer");
    assert(buttonLayer);

    buttonCoverFlow = ituSceneFindWidget(&theScene, "buttonCoverFlow");
    assert(buttonCoverFlow);
}
