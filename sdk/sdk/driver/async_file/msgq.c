#include "async_file/config.h"
#include "async_file/pal.h"
#include "async_file/heap.h"
#include <sys/time.h>

#define LOOP_SLEEP      10
#define WAIT_TIMEOUT    2
#define MAX_MSGQ_COUNT  1

typedef struct MSGQ_TAG
{
    MMP_UINT8*  buf;
    MMP_ULONG   entrySize;
    MMP_ULONG   entryCount;
    MMP_UINT8*  readPtr;
    MMP_UINT8*  writePtr;
    MMP_UINT8*  endPtr;
} MSGQ;

static MSGQ palQueues[MAX_MSGQ_COUNT];     // All pre-created queues


unsigned int GetClock(void)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0)
        printf("gettimeofday failed!\n");
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

unsigned long GetDuration(unsigned int clock)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0)
        printf("gettimeofday failed!\n");
    return (unsigned int)(tv.tv_sec*1000+tv.tv_usec/1000) - clock;
}

MMP_INT
PalMsgQInitialize(
    void)
{
    MSGQ* queue;
    MMP_UINT i = 0;
    LOG_ENTER "PalMsgQInitialize()\r\n" LOG_END

    // Clear variables
    PalMemset(palQueues, 0, sizeof (palQueues));

    LOG_LEAVE "PalMsgQInitialize()=0\r\n" LOG_END
    return 0;
}

MMP_INT
PalMsgQTerminate(
    void)
{
    LOG_ENTER "PalMsgQTerminate()\r\n" LOG_END
    LOG_LEAVE "PalMsgQTerminate()=0\r\n" LOG_END
    return 0;
}

PAL_MSGQ
PalCreateMsgQ(
    MMP_INT     name,
    MMP_ULONG   msgSize,
    MMP_ULONG   msgCount)
{
    MSGQ* queue;
    MMP_ULONG size;
    LOG_ENTER "PalCreateMsgQ(name=%d)\r\n", name LOG_END

    PalAssert(name < MAX_MSGQ_COUNT);

    size = msgSize * msgCount;
    queue = &palQueues[name];
    queue->buf = (MMP_UINT8*)PalHeapAlloc(PAL_HEAP_DEFAULT, size);
    if (queue->buf)
    {
        queue->readPtr      = queue->buf;
        queue->writePtr     = queue->buf;
        queue->endPtr       = queue->buf + size;
        queue->entrySize    = msgSize;
        queue->entryCount   = msgCount;
    }
    else
        queue = MMP_NULL;

    LOG_LEAVE "PalCreateMsgQ()=0x%X\r\n", queue LOG_END
    return queue;
}

MMP_INT
PalDestroyMsgQ(
    PAL_MSGQ queue)
{
    MSGQ* q;
    LOG_ENTER "PalDestroyMsgQ(queue=0x%X)\r\n", queue LOG_END

    PalAssert(queue);

    q = (MSGQ*)queue;
    if (q->buf)
    {
        PalHeapFree(PAL_HEAP_DEFAULT, q->buf);
    }
    PalMemset(q, 0, sizeof(MSGQ));

    LOG_LEAVE "PalDestroyMsgQ()=0\r\n" LOG_END
    return 0;
}

MMP_INT
PalReadMsgQ(
    PAL_MSGQ queue,
    void* msg,
    MMP_ULONG timeout)
{
    MMP_INT result = 0;
    MSGQ* msgQ;
    PAL_CLOCK_T clk;
    LOG_ENTER "PalReadMsgQ(queue=0x%X,msg=0x%X,imeout=%lu)\r\n",
        queue, msg, timeout LOG_END

    PalAssert(queue);
    PalAssert(msg);

    msgQ = (MSGQ*) queue;

    clk = GetClock();

    while (msgQ->readPtr == msgQ->writePtr)
    {
        PalSleep(LOOP_SLEEP);

        if (timeout != PAL_MSGQ_INFINITE && GetDuration(clk) >= timeout)
        {
            result = WAIT_TIMEOUT;
            goto end;
        }
    }

    PalMemcpy(msg, msgQ->readPtr, msgQ->entrySize);
    msgQ->readPtr += msgQ->entrySize;
    if (msgQ->readPtr >= msgQ->endPtr)
        msgQ->readPtr = msgQ->buf;
end:
    LOG_LEAVE "PalReadMsgQ()=%d\r\n", result LOG_END
    return result;
}

MMP_INT
PalWriteMsgQ(
    PAL_MSGQ    queue,
    void*       msg,
    MMP_ULONG   timeout)
{
    MMP_INT result = 0;
    MSGQ* msgQ;
    PAL_CLOCK_T clk;
    MMP_UINT8* nextWritePtr;
    LOG_ENTER "PalWriteMsgQ(queue=0x%X,msg=0x%X,timeout=%lu)\r\n",
        queue, msg, timeout LOG_END

    PalAssert(queue);
    PalAssert(msg);

    msgQ = (MSGQ*) queue;

    clk = GetClock();

    nextWritePtr = msgQ->writePtr + msgQ->entrySize;
    if (nextWritePtr >= msgQ->endPtr)
        nextWritePtr = msgQ->buf;
    while (nextWritePtr == msgQ->readPtr)
    {
        PalSleep(LOOP_SLEEP);

        if (timeout != PAL_MSGQ_INFINITE && GetDuration(clk) >= timeout)
        {
            result = WAIT_TIMEOUT;
            goto end;
        }
    }

    PalMemcpy(msgQ->writePtr, msg, msgQ->entrySize);
    msgQ->writePtr = nextWritePtr;

end:
    LOG_LEAVE "PalWriteMsgQ()=%d\r\n", result LOG_END
    return result;
}
