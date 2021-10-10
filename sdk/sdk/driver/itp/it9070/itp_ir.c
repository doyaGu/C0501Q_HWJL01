/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Remote IR functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <unistd.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#include "itp_cfg.h"

#define QUEUE_LEN 256

static const uint32_t irTypeTable[] =
{
#include "ir_type.inc"
};

#define STATE_NUM               (5)
#define MAX_VAL                 ((1<<10)-1)

#define PRECISION               16
#define SAMP_DUR                100    // sample duration in microseconds

#define REPEAT_THRESHOLD_BEGIN  (5)     // accumulate number of repeat-key will start dispatch key
#define REPEAT_THRESHOLD_SPDUP  (2)     // accumulate number of key will change to high speed mode
#define REPEAT_THRESHOLD_HOLD1  (5)     // dispatch a key foreach number of repeat-key at low speed mode
#define REPEAT_THRESHOLD_HOLD2  (1)     // dispatch a key foreach number of repeat-key at high speed mode

enum RCState {
    WAIT_START,
    WAIT_START_REPEAT_KEY,      // hiden state for repeat key
    WAIT_BIT,
    WAIT_BIT_ONE,               // hiden state for a "1" bit
    WAIT_END
};

static int irThresholds[STATE_NUM][2];     // min & max for a signal stste
static int irRecvBitCount;
static enum RCState irCurrState;
static unsigned long irRecvCode;      // LSB received code
static unsigned long irRecvCodeH;     // MSB received code if irTypeTable[5] > 32
static int irRepeatKeyPress, irRepeatKeyHold, irRepeatKeyFast, irRepeatKeyCnt;

static QueueHandle_t irQueue;
static ITPKeypadEvent irLastEvent;

static inline int MULSHIFT(int x, int y, int shift)
{
    int64_t xext, yext;
    xext = (int64_t)x;
    yext = (int64_t)y;
    xext = ((xext * yext) >> shift);
    return (int)xext;
}

static int IrProbe(void)
{
    int completeACode = 0, signal, code;
    unsigned long getBit;

    signal = ithIrProbe();

    switch (irCurrState)
    {
    case WAIT_START:
        if ((signal >= irThresholds[WAIT_START][0]) && (signal <= irThresholds[WAIT_START][1]))
        {
            irCurrState = WAIT_BIT;
        }
        else if ((signal >= irThresholds[WAIT_START_REPEAT_KEY][0]) && (signal <= irThresholds[WAIT_START_REPEAT_KEY][1]))
        {
            code = irRecvCode;

        #ifdef CFG_IR_REPEAT
            if (irRepeatKeyPress >= REPEAT_THRESHOLD_BEGIN)
            {
                //ithPrintf("Hold\n");

                if ((irRepeatKeyFast == 0 && irRepeatKeyHold >= REPEAT_THRESHOLD_HOLD1) ||
                    (irRepeatKeyFast == 1 && irRepeatKeyHold >= REPEAT_THRESHOLD_HOLD2))
                {
                    //ithPrintf("Send repeat key (%08x)\n", irRecvCode);

                    completeACode = 1;
                    irRepeatKeyHold = 0;
                    if (irRepeatKeyCnt >= REPEAT_THRESHOLD_SPDUP)
                        irRepeatKeyFast = 1;
                    else
                        irRepeatKeyCnt++;
                }
                else
                    irRepeatKeyHold++;
            }
            else
                irRepeatKeyPress++;

        #endif // CFG_IR_REPEAT

            irCurrState = WAIT_END;
        }
        else
            irCurrState = WAIT_START; // error

        break;

    case WAIT_BIT:
        irRepeatKeyPress = irRepeatKeyHold = 0;
        irRepeatKeyFast = irRepeatKeyCnt = 0;
        if ((signal >= irThresholds[WAIT_BIT][0]) && (signal <= irThresholds[WAIT_BIT][1]))
        {
            // bit "0"
            getBit = 0;
        }
        else if ((signal >= irThresholds[WAIT_BIT_ONE][0]) && (signal <= irThresholds[WAIT_BIT_ONE][1]))
        {
            // bit "1"
            if (irTypeTable[6])
                getBit = 0x80000000L;
            else
                getBit = 0x00000001L;
        }
        else
        {
            // error
            irRecvCode = irRecvCodeH = irRecvBitCount = 0;
            irCurrState = WAIT_START;
            break;
        }

        if (irTypeTable[6])
        {
            irRecvCodeH = (irRecvCodeH >> 1) | ((irRecvCode & 1) << 31);
            irRecvCode  = (irRecvCode  >> 1) | getBit;
        }
        else
        {
            irRecvCodeH = (irRecvCodeH << 1) | (irRecvCode & 0x80000000L);
            irRecvCode  = (irRecvCode  << 1) | getBit;
        }

        irRecvBitCount++;
        if (irRecvBitCount < irTypeTable[5])
            irCurrState = WAIT_BIT;   // not yet complet a code
        else
        {
            irCurrState = WAIT_END;
        }
        break;

    case WAIT_END:
        if ((signal >= irThresholds[WAIT_END][0]) && (signal <= irThresholds[WAIT_END][1]))
        {
            if (irRecvBitCount >= irTypeTable[5])
            {
                completeACode = 1;
                code = irRecvCode;
            }
            else
            {
                completeACode  = 0;
            }

            irRecvBitCount = 0;
            irCurrState = WAIT_START;
        }
        else
        {
            // error
            irRecvBitCount = 0;
            irRepeatKeyPress = irRepeatKeyHold = 0;
            irRepeatKeyFast = irRepeatKeyCnt = 0;
            completeACode = 0;
            irCurrState = WAIT_START;
        }
        break;

    default:
        break;
    }

    if (completeACode)
        return code;

    return -1;
}

static void IrIntrHandler(void* arg)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    int code = IrProbe();

    //ithPrintf("IR code: 0x%X\n", code);

    if (code == -1)
        return;

    if (((code & 0xFFFF) == CFG_IR_VENDOR_CODE) && (((code >> 16) & 0xFF) == (~((code >> 24) & 0xFF) & 0xFF)))
        code = (code >> 16) & 0xFF;

    xQueueSendFromISR(irQueue, &code, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void IrCalcThresholds(void)
{
    int i, f1, f2;
    int durations[STATE_NUM];

    durations[0] = irTypeTable[0] * (1 << PRECISION) / 1000;
    durations[1] = irTypeTable[1] * (1 << PRECISION) / 1000;
    durations[2] = irTypeTable[2] * (1 << PRECISION) / 1000;
    durations[3] = irTypeTable[3] * (1 << PRECISION) / 1000;
    durations[4] = irTypeTable[4] * (1 << PRECISION) / 1000;

    // know the sampling duration
    // set the min & max for each state
    for (i = 0; i < STATE_NUM; i++)
    {
        // 0.92 & 1.08 is experienc value, duration range N is between 0.92*N and 1.08*N.
        f1 = ((durations[i]) << 1) / (SAMP_DUR * (1 << PRECISION) / 1000);
        f2 = MULSHIFT(f1, 0.92f * (1 << PRECISION), PRECISION) >> 1;
        if (f2 >= MAX_VAL)
        {
            LOG_ERR "Out of counter resolution!\n" LOG_END
        }

        irThresholds[i][0] = (f2 < 1) ? 1 : f2; // f1 * 0.92
        irThresholds[i][1] = (i == STATE_NUM - 1) ?
            MAX_VAL :
            ((MULSHIFT(f1, 1.08f * (1 << PRECISION), PRECISION) + 1) >> 1); // f1 * 1.08 + 0.5

        LOG_DBG "irThresholds[%d][0] = %d, irThresholds[%d][1] = %d\n", i, irThresholds[i][0], i, irThresholds[i][1] LOG_END
    }
}

static void IrInit(void)
{
    irCurrState = WAIT_START;
    irRecvCode = irRecvCodeH = irRecvBitCount = 0;
    irRepeatKeyPress = irRepeatKeyHold = 0;
    irRepeatKeyFast = irRepeatKeyCnt = 0;

    ithIrInit(CFG_GPIO_IR, 0, SAMP_DUR, PRECISION);
    IrCalcThresholds();

    // Prepare q and intr
    irLastEvent.code = -1;
    irQueue = xQueueCreate(QUEUE_LEN, (unsigned portBASE_TYPE) sizeof(int));

    portENTER_CRITICAL();

    // Init IR(remote control) interrupt
    ithIntrDisableIrq(ITH_INTR_RC);
    ithIntrClearIrq(ITH_INTR_RC);
    ithIntrRegisterHandlerIrq(ITH_INTR_RC, IrIntrHandler, NULL);
    ithIntrEnableIrq(ITH_INTR_RC);

    ithIrCtrlEnable(ITH_IR_INT);
    ithIrIntrCtrlEnable(ITH_IR_DATA);

    // Enable IR Capture
    ithIrCtrlEnable(ITH_IR_EN);

    portEXIT_CRITICAL();
}

static int IrRead(int file, char *ptr, int len, void* info)
{
    ITPKeypadEvent* ev = (ITPKeypadEvent*)ptr;

    if (xQueueReceive(irQueue, &ev->code, 0))
    {
        gettimeofday(&ev->time, NULL);
        ev->flags = ITP_KEYPAD_DOWN;

        if (irLastEvent.code == ev->code)
            ev->flags |= ITP_KEYPAD_REPEAT;

        irLastEvent.code = ev->code;
        irLastEvent.time.tv_sec = ev->time.tv_sec;
        irLastEvent.time.tv_usec = ev->time.tv_usec;

        return sizeof (ITPKeypadEvent);
    }
    else if (irLastEvent.code != -1)
    {
        struct timeval now;

        gettimeofday(&now, NULL);
        if (itpTimevalDiff(&irLastEvent.time, &now) >= CFG_IR_PRESS_INTERVAL)
        {
            ev->code            = irLastEvent.code;
            ev->time.tv_sec     = now.tv_sec;
            ev->time.tv_usec    = now.tv_usec;
            ev->flags           = ITP_KEYPAD_UP;
            irLastEvent.code    = -1;
            return sizeof (ITPKeypadEvent);
        }
    }

    return 0;
}

static int IrIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_INIT:
        IrInit();
        break;

    default:
        errno = (ITP_DEVICE_IR << ITP_DEVICE_ERRNO_BIT) | __LINE__;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceIr =
{
    ":ir",
    itpOpenDefault,
    itpCloseDefault,
    IrRead,
    itpWriteDefault,
    itpLseekDefault,
    IrIoctl,
    NULL
};
