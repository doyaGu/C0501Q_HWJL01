#include <sys/ioctl.h>
#include <assert.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"
#include "sports_equipment.h"
#include "scene.h"

#define EXT_MAX_QUEUE_SIZE      8

static mqd_t extQueue = -1;
static pthread_t extTask;
static bool extQuit;

static void* ExternalCheckTask(void* arg)
{
    while (!extQuit)
    {
        // TODO: IMPLEMENT
        if (0)
        {
            ExternalEvent ev;

            ev.type = EXTERNAL_WARN;
            ev.arg1 = EXTERNAL_WARN_SAFETY;

            mq_send(extQueue, (const char*)&ev, sizeof (ExternalEvent), 0);
        }
        usleep(10000);
    }
    return NULL;
}

void ExternalInit(void)
{
    struct mq_attr qattr;

    qattr.mq_flags = 0;
    qattr.mq_maxmsg = EXT_MAX_QUEUE_SIZE;
    qattr.mq_msgsize = sizeof(ExternalEvent);

    extQueue = mq_open("external", O_CREAT | O_NONBLOCK, 0644, &qattr);
    assert(extQueue != -1);

    extQuit = false;

    pthread_create(&extTask, NULL, ExternalCheckTask, NULL);
}

void ExternalExit(void)
{
    extQuit = true;

    pthread_join(extTask, NULL);

    mq_close(extQueue);
    extQueue = -1;

    // TODO: IMPLEMENT
}

int ExternalCheck(ExternalEvent* ev)
{
    assert(ev);

    if (mq_receive(extQueue, (char*)ev, sizeof(ExternalEvent), 0) > 0)
        return 1;

    return 0;
}
