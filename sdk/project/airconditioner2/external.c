#include <sys/ioctl.h>
#include <assert.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"
#include "airconditioner.h"
#include "scene.h"

#define EXT_MAX_QUEUE_SIZE          8
#define EXT_CMD_LEN                 256
#define EXT_RESULT_LEN              22
#define EXT_COUNTDOWN_INIT          8
#define EXT_COUNTDOWN               120

ExternalMachine extMachine;

static mqd_t extInQueue = -1;
static mqd_t extOutQueue = -1;
static pthread_t extInTask;
static pthread_t extOutTask;
static bool extQuit;
static uint32_t extTick;
static int extCountDown;

static void ExternalUpdateCmdCheckSum(uint8_t cmd[EXT_CMD_LEN])
{
    cmd[6] = 0xFF - (cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5]) + 1;
}

static bool ExternalCheckResultCheckSum(uint8_t result[EXT_CMD_LEN])
{
    int i, sum, checksum;

    sum = 0;
    for (i = 0; i < 20; i++)
        sum += result[i];

    checksum = 0xFF - sum + 1;

    return checksum == result[20] ? true : false;
}

static void ExternalHandleCommError(bool commError)
{
    ExternalEvent ev;

    ev.type = EXTERNAL_WARN;

    if (commError)
    {
        extMachine.warnings[EXTERNAL_WARN_NONE] = 0;
        extMachine.warnings[EXTERNAL_WARN_COMM] = 1;
        ev.arg1 = EXTERNAL_WARN_COMM;
    }
    else
    {
        extMachine.warnings[EXTERNAL_WARN_NONE] = 1;
        extMachine.warnings[EXTERNAL_WARN_COMM] = 0;
        ev.arg1 = EXTERNAL_WARN_NONE;
    }
    ev.arg2 = ev.arg3 = EXTERNAL_WARN_NONE;
    mq_send(extInQueue, (const char*)&ev, sizeof (ExternalEvent), 0);
}

static void ExternalHandleNormalMessages(uint8_t result[EXT_CMD_LEN])
{
    // TODO: IMPLEMENT
}

static void ExternalHandleWarningMessages(uint8_t result[EXT_CMD_LEN])
{
    ExternalWarnType warn = EXTERNAL_WARN_NONE;
    uint32_t bits = 0;
    int i;
    ExternalEvent ev;

    if (result[13])
    {
        bits = result[13];
        warn = EXTERNAL_WARN_1;

        for (i = 0; i < 8; i++)
            extMachine.warnings[warn++] = ((bits >> i) & 0x1) ? 1 : 0;
    }

    if (result[14])
    {
        bits = result[14];
        warn = EXTERNAL_WARN_9;

        for (i = 0; i < 8; i++)
            extMachine.warnings[warn++] = ((bits >> i) & 0x1) ? 1 : 0;
    }

    if (result[15])
    {
        bits = result[15];
        warn = EXTERNAL_WARN_17;

        for (i = 0; i < 8; i++)
            extMachine.warnings[warn++] = ((bits >> i) & 0x1) ? 1 : 0;
    }
    
    if (result[16])
    {
        bits = result[16];
        warn = EXTERNAL_WARN_25;

        for (i = 0; i < 8; i++)
            extMachine.warnings[warn++] = ((bits >> i) & 0x1) ? 1 : 0;
    }
    
    if (result[17])
    {
        bits = result[17];
        warn = EXTERNAL_WARN_33;

        for (i = 0; i < 8; i++)
            extMachine.warnings[warn++] = ((bits >> i) & 0x1) ? 1 : 0;
    }

    ev.type = EXTERNAL_WARN;
    ev.arg1 = ev.arg2 = ev.arg3 = EXTERNAL_WARN_NONE;

    for (i = EXTERNAL_WARN_1; i < EXTERNAL_WARN_COMM; i++)
    {
        if (extMachine.warnings[i])
        {
            if (ev.arg1 == EXTERNAL_WARN_NONE)
            {
                ev.arg1 = extMachine.warnings[i];
            }
            else if (ev.arg2 == EXTERNAL_WARN_NONE)
            {
                ev.arg2 = extMachine.warnings[i];
            }
            else if (ev.arg3 == EXTERNAL_WARN_NONE)
            {
                ev.arg3 = extMachine.warnings[i];
                break;
            }
        }
    }

    extMachine.warnings[EXTERNAL_WARN_NONE] = (ev.arg1 == EXTERNAL_WARN_NONE) ? 1 : 0;

    mq_send(extInQueue, (const char*)&ev, sizeof (ExternalEvent), 0);
}

static void* ExternalTask(void* arg)
{
    extTick = itpGetTickCount();
    extCountDown = EXT_COUNTDOWN_INIT;

    while (!extQuit)
    {
        ExternalEvent ev;
        uint8_t cmd[EXT_CMD_LEN] = { 0xAA, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55 };
        uint8_t result[EXT_RESULT_LEN];

        if (mq_receive(extOutQueue, (char*)&ev, sizeof(ExternalEvent), 0) > 0)
        {
            switch (ev.type)
            {
            case EXTERNAL_PM25:
                printf("EXTERNAL_PM25: %d\n", ev.arg1);
                cmd[2] = 0x0;
                cmd[3] = ev.arg1 ? 0x52 : 0x53;
                cmd[4] = 0x0;
                cmd[5] = 0x0;
                break;

            case EXTERNAL_MODE:
                printf("EXTERNAL_MODE: %d\n", ev.arg1);
                // TODO: IMPLEMENT
                break;

            case EXTERNAL_TEMPERATURE:
                printf("EXTERNAL_TEMPERATURE: %d\n", ev.arg1);
                // TODO: IMPLEMENT
                break;

            case EXTERNAL_POWER:
                printf("EXTERNAL_POWER: %d\n", ev.arg1);
                // TODO: IMPLEMENT
                break;

            case EXTERNAL_WIND:
                printf("EXTERNAL_WIND: %d\n", ev.arg1);
                // TODO: IMPLEMENT
                break;

            case EXTERNAL_LIGHT:
                printf("EXTERNAL_LIGHT: %d\n", ev.arg1);
                // TODO: IMPLEMENT
                break;
            }
        }
        else
        {
            cmd[2] = 0x0;
            cmd[3] = 0x0;
            cmd[4] = 0x0;
            cmd[5] = 0x0;
        }
        ExternalUpdateCmdCheckSum(cmd);

    #if (defined(CFG_UART0_ENABLE) && !defined(CFG_DBG_UART0)) || defined(_WIN32)
        for (;;)
        {
            int ret;
            
            ret = write(ITP_DEVICE_UART0, cmd, EXT_CMD_LEN);
            if (ret == EXT_CMD_LEN)
            {
                memset(result, 0, sizeof (result));

                ret = read(ITP_DEVICE_UART0, result, EXT_CMD_LEN);
                if (ret == EXT_CMD_LEN)
                {
                    if (result[0] == 0xAA && result[21] == 0x55 && ExternalCheckResultCheckSum(result))
                    {
                        if (extMachine.warnings[EXTERNAL_WARN_COMM])
                        {
                            extTick = itpGetTickCount();
                            extCountDown = EXT_COUNTDOWN;

                            if (result[1] != 0xA3)
                            {
                                ExternalHandleCommError(false);
                            }
                        }

                        if (result[1] == 0xA1)
                        {
                            ExternalHandleNormalMessages(result);
                        }
                        else if (result[1] == 0xA3)
                        {
                            ExternalHandleWarningMessages(result);
                        }
                        break;
                    }
                }
            }

            if (!extMachine.warnings[EXTERNAL_WARN_COMM])
            {
                if (itpGetTickDuration(extTick) >= 1000)
                {
                    extTick = itpGetTickCount();

                    if (--extCountDown <= 0)
                    {
                        extCountDown = EXT_COUNTDOWN;
                        ExternalHandleCommError(true);
                    }
                }
            }
            usleep(400000);
            cmd[2] |= 0x2;
        };
    #endif // (defined(CFG_UART0_ENABLE) && !defined(CFG_DBG_UART0)) || defined(_WIN32)

        usleep(100000);
    }
    mq_close(extOutQueue);
    extOutQueue = -1;
    return NULL;
}

void ExternalInit(void)
{
    struct mq_attr qattr;
    pthread_attr_t attr;

    memset(&extMachine, 0, sizeof(extMachine));

    extMachine.warnings[EXTERNAL_WARN_NONE] = 1;

    qattr.mq_flags = 0;
    qattr.mq_maxmsg = EXT_MAX_QUEUE_SIZE;
    qattr.mq_msgsize = sizeof(ExternalEvent);

    extInQueue = mq_open("external_in", O_CREAT | O_NONBLOCK, 0644, &qattr);
    assert(extInQueue != -1);

    extOutQueue = mq_open("external_out", O_CREAT | O_NONBLOCK, 0644, &qattr);
    assert(extOutQueue != -1);

    extQuit = false;

#if (defined(CFG_UART0_ENABLE) && !defined(CFG_DBG_UART0)) || defined(_WIN32)
    ioctl(ITP_DEVICE_UART0, ITP_IOCTL_UART_TIMEOUT, (void*)100);
#endif

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&extInTask, &attr, ExternalTask, NULL);
}

void ExternalExit(void)
{
    extQuit = true;
}

int ExternalReceive(ExternalEvent* ev)
{
    assert(ev);

    if (extQuit)
        return 0;

    if (mq_receive(extInQueue, (char*)ev, sizeof(ExternalEvent), 0) > 0)
        return 1;

    return 0;
}

int ExternalSend(ExternalEvent* ev)
{
    assert(ev);

    if (extQuit)
        return -1;

    return mq_send(extOutQueue, (char*)ev, sizeof(ExternalEvent), 0);
}
