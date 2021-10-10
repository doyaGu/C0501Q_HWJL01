#include <sys/ioctl.h>
#include <assert.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include "ite/itp.h"
#include "ctrlboard.h"
#include "scene.h"

#define EXT_MAX_QUEUE_SIZE          8
#define EXT_MAX_DATA_LEN            1024
#define EXT_MIN_DATA_LEN            3
#define EXT_DATA_HEADER             0xEE

static mqd_t extInQueue = -1;
static mqd_t extOutQueue = -1;
static pthread_t extInTask;
static pthread_t extOutTask;
static sem_t extUartSem;
static bool extQuit;

static void ExternalUartCallback(void* arg1, uint32_t arg2)
{
	sem_post(&extUartSem);
}

static int ExternalGetDataSize(uint8_t* data)
{
    uint16_t cmd;
    int size = 9; // head + cmd + crc16 + tail
    
    if (data[0] != EXT_DATA_HEADER)
    {
        printf("unknown header: 0x%2X\n", (unsigned int)data[0]);
        return 0;
    }
    memcpy(&cmd, &data[1], 2);

    cmd = itpBetoh16(cmd);

    switch (cmd)
    {
    case EXTERNAL_TAP_POWER:
        size += 1;
        break;

    case EXTERNAL_TAP_AUTO:
    case EXTERNAL_TAP_COOL:
    case EXTERNAL_TAP_DRY:
    case EXTERNAL_TAP_FAN:
    case EXTERNAL_TAP_HEAT:
    case EXTERNAL_DRAG_TEMP_UP:
    case EXTERNAL_DRAG_TEMP_DOWN:
    case EXTERNAL_TAP_WIND_HIGH:
    case EXTERNAL_TAP_WIND_MID:
    case EXTERNAL_TAP_WIND_LOW:
    case EXTERNAL_TAP_WIND_AUTO:
    case EXTERNAL_TAP_HOME_PAGE_UP:
    case EXTERNAL_TAP_HOME_PAGE_DOWN:
    case EXTERNAL_TAP_AIRCONDITIONER:
    case EXTERNAL_TAP_MULTIMEDIA:
        size += 0;
        break;

    default:
        printf("unknown cmd: 0x%X\n", (unsigned int)cmd);
        return 0;
    }
    return size;
}

static bool ExternalProcessData(uint8_t* data, int size)
{
    ExternalEvent ev;
    uint16_t dataCrc16, cmd;
    uint16_t crc16 = itcCrc16(&data[1], size - 7); // not include header & tail & crc16

    memcpy(&dataCrc16, data + size - 6, 2);
    dataCrc16 = itpBetoh16(dataCrc16);

    if (crc16 != dataCrc16)
    {
        printf("crc incorrect: 0x%X != 0x%X\n", (unsigned int)crc16, (unsigned int)dataCrc16);
        return false;
    }

    memcpy(&cmd, &data[1], 2);
    cmd = itpBetoh16(cmd);

    ev.type = cmd;
    ev.arg1 = ev.arg2 = ev.arg3 = 0;

    switch (cmd)
    {
    case EXTERNAL_TAP_POWER:
        ev.arg1 = data[3];
        break;

    case EXTERNAL_TAP_AUTO:
    case EXTERNAL_TAP_COOL:
    case EXTERNAL_TAP_DRY:
    case EXTERNAL_TAP_FAN:
    case EXTERNAL_TAP_HEAT:
    case EXTERNAL_DRAG_TEMP_UP:
    case EXTERNAL_DRAG_TEMP_DOWN:
    case EXTERNAL_TAP_WIND_HIGH:
    case EXTERNAL_TAP_WIND_MID:
    case EXTERNAL_TAP_WIND_LOW:
    case EXTERNAL_TAP_WIND_AUTO:
    case EXTERNAL_TAP_HOME_PAGE_UP:
    case EXTERNAL_TAP_HOME_PAGE_DOWN:
    case EXTERNAL_TAP_AIRCONDITIONER:
    case EXTERNAL_TAP_MULTIMEDIA:
        break;

    default:
        printf("unknown cmd: 0x%X\n", (unsigned int)cmd);
        return 0;
    }

    mq_send(extInQueue, (const char*)&ev, sizeof (ExternalEvent), 0);

    return true;
}

static int ExternalPackData(ExternalEvent* ev, uint8_t* data)
{
    uint16_t crc16, cmd;
    uint8_t param = 0;
    int size = 9;

    assert(ev);
    assert(data);

    switch (ev->type)
    {
    case EXTERNAL_TAP_POWER:
        printf("EXTERNAL_TAP_POWER\n");
        cmd = EXTERNAL_TAP_POWER;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    case EXTERNAL_TAP_AUTO:
        printf("EXTERNAL_TAP_AUTO\n");
        cmd = EXTERNAL_TAP_AUTO;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    case EXTERNAL_TAP_COOL:
        printf("EXTERNAL_TAP_COOL\n");
        cmd = EXTERNAL_TAP_COOL;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    case EXTERNAL_TAP_DRY:
        printf("EXTERNAL_TAP_DRY\n");
        cmd = EXTERNAL_TAP_DRY;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    case EXTERNAL_TAP_FAN:
        printf("EXTERNAL_TAP_FAN\n");
        cmd = EXTERNAL_TAP_FAN;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    case EXTERNAL_TAP_HEAT:
        printf("EXTERNAL_TAP_HEAT\n");
        cmd = EXTERNAL_TAP_HEAT;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    case EXTERNAL_DRAG_TEMP_UP:
        printf("EXTERNAL_DRAG_TEMP_UP\n");
        cmd = EXTERNAL_DRAG_TEMP_UP;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    case EXTERNAL_DRAG_TEMP_DOWN:
        printf("EXTERNAL_DRAG_TEMP_DOWN\n");
        cmd = EXTERNAL_DRAG_TEMP_DOWN;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    case EXTERNAL_TAP_WIND_HIGH:
        printf("EXTERNAL_TAP_WIND_HIGH\n");
        cmd = EXTERNAL_TAP_WIND_HIGH;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    case EXTERNAL_TAP_WIND_MID:
        printf("EXTERNAL_TAP_WIND_MID\n");
        cmd = EXTERNAL_TAP_WIND_MID;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    case EXTERNAL_TAP_WIND_LOW:
        printf("EXTERNAL_TAP_WIND_LOW\n");
        cmd = EXTERNAL_TAP_WIND_LOW;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    case EXTERNAL_TAP_WIND_AUTO:
        printf("EXTERNAL_TAP_WIND_AUTO\n");
        cmd = EXTERNAL_TAP_WIND_AUTO;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    case EXTERNAL_TAP_HOME_PAGE_UP:
        printf("EXTERNAL_TAP_HOME_PAGE_UP\n");
        cmd = EXTERNAL_TAP_HOME_PAGE_UP;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    case EXTERNAL_TAP_HOME_PAGE_DOWN:
        printf("EXTERNAL_TAP_HOME_PAGE_DOWN\n");
        cmd = EXTERNAL_TAP_HOME_PAGE_DOWN;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    case EXTERNAL_TAP_AIRCONDITIONER:
        printf("EXTERNAL_TAP_AIRCONDITIONER\n");
        cmd = EXTERNAL_TAP_AIRCONDITIONER;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    case EXTERNAL_TAP_MULTIMEDIA:
        printf("EXTERNAL_TAP_MULTIMEDIA\n");
        cmd = EXTERNAL_TAP_MULTIMEDIA;
        param = (uint8_t)ev->arg1;
        size += 1;
        break;

    default:
        printf("unknown event type: 0x%X\n", ev->type);
        return 0;
    }

    memset(data, 0, size);

    data[0] = EXT_DATA_HEADER;

    cmd = cpu_to_be16(cmd);
    memcpy(&data[1], &cmd, 2);

    data[3] = param;

    crc16 = itcCrc16(&data[1], size - 7); // not include header & tail & crc16
    crc16 = cpu_to_be16(crc16);
    memcpy(data + size - 6, &crc16, 2);

    data[size - 4] = 0xFF;
    data[size - 3] = 0xFC;
    data[size - 2] = 0xFF;
    data[size - 1] = 0xFF;

    return size;
}

static void* ExternalInTask(void* arg)
{
    int availSize, readSize, dataSize;
    uint8_t data[EXT_MAX_DATA_LEN];

    availSize = EXT_MAX_DATA_LEN;
    readSize = dataSize = 0;

    while (!extQuit)
    {
        int size;

        sem_wait(&extUartSem);

        if (extQuit)
            break;

        size = read(ITP_DEVICE_UART1, data + readSize, availSize);
        readSize += size;
        availSize -= size;
        
        if (readSize < EXT_MIN_DATA_LEN)
            continue;

        if (dataSize == 0)
        {
            dataSize = ExternalGetDataSize(data);
            if (dataSize == 0)
            {
                availSize = EXT_MAX_DATA_LEN;
                readSize = dataSize = 0;
                continue;
            }
            else
            {
                if (readSize < dataSize)
                {
                    availSize = dataSize - readSize;
                }
                else
                {
                    availSize = 0;
                }
            }
        }

        if (availSize > 0)
            continue;

        if (!ExternalProcessData(data, dataSize))
        {
            availSize = EXT_MAX_DATA_LEN;
            readSize = dataSize = 0;
            continue;
        }

        availSize = EXT_MAX_DATA_LEN;
        readSize = dataSize = 0;

        usleep(1000);
    }
    sem_destroy(&extUartSem);
    mq_close(extInQueue);
    extInQueue = -1;
    return NULL;
}

static void* ExternalOutTask(void* arg)
{
    while (!extQuit)
    {
        ExternalEvent ev;

        if (mq_receive(extOutQueue, (char*)&ev, sizeof(ExternalEvent), 0) > 0)
        {
            uint8_t data[EXT_MAX_DATA_LEN];
            int dataSize;

            if (extQuit)
                break;

            dataSize = ExternalPackData(&ev, data);
            if (dataSize > 0)
            {
                int size, writtenSize = 0;
                int availSize = dataSize;

                for (;;)
                {
                    size = write(ITP_DEVICE_UART1, data + writtenSize, availSize);
                    writtenSize += size;
                    availSize -= size;

                    if (availSize == 0)
                        break;

                    usleep(1000);
                }
            }
        }
        usleep(10000);
    }
    mq_close(extOutQueue);
    extOutQueue = -1;
    return NULL;
}

void ExternalInit(void)
{
    struct mq_attr qattr;
    pthread_attr_t attr;

    qattr.mq_flags = 0;
    qattr.mq_maxmsg = EXT_MAX_QUEUE_SIZE;
    qattr.mq_msgsize = sizeof(ExternalEvent);

    extInQueue = mq_open("external_in", O_CREAT | O_NONBLOCK, 0644, &qattr);
    assert(extInQueue != -1);
    extOutQueue = mq_open("external_out", O_CREAT | O_NONBLOCK, 0644, &qattr);
    assert(extOutQueue != -1);

    sem_init(&extUartSem, 0, 0);

    extQuit = false;

    ioctl(ITP_DEVICE_UART1, ITP_IOCTL_REG_UART_CB, (void*)ExternalUartCallback);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&extInTask, &attr, ExternalInTask, NULL);
    pthread_create(&extOutTask, &attr, ExternalOutTask, NULL);
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
