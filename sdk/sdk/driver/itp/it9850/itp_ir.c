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
#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#include "itp_cfg.h"


//#define ENABLE_IR_INTR
//#define ENABLE_DBG_COMPARE_CODE


#define QUEUE_LEN 256

static const uint32_t irTypeTable[] =
{
#include "ir_type.inc"
};

#if 1
static const uint32_t irValidTxCodeTable[] =
{
    0x55,0xAA
};
#else
static const uint32_t irValidTxCodeTable[] =
{
#include "ir_TxTable.inc"
};
#endif

#define STATE_NUM               (5)
#define MAX_VAL                 ((1<<10)-1)

#define PRECISION               16
#define SAMP_DUR                10    // sample duration in microseconds

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

uint16_t    g_IrTxBuf[]={0x7c, 0x15, 0x15, 0x0a, 0x0a, 0x0b, 0x0b, 0x0a,
                        0x0b, 0x0a, 0x0a, 0x15, 0x15, 0x14, 0x15, 0x15,
                        0x15, 0x14, 0x0b, 0x0a, 0x0a, 0x15, 0x0a, 0x0b,
                        0x0a, 0x0b, 0x14, 0x15, 0x15, 0x0a, 0x15, 0x14,
                        0x15, 0x174,0x68};

static int irThresholds[STATE_NUM][2];     // min & max for a signal stste
static int irRecvBitCount;
static enum RCState irCurrState;
static unsigned long irRecvCode;      // LSB received code
static unsigned long irRecvCodeH;     // MSB received code if irTypeTable[5] > 32
static int irRepeatKeyPress, irRepeatKeyHold, irRepeatKeyFast, irRepeatKeyCnt;

static QueueHandle_t irQueue;
static QueueHandle_t irTxQueue;
static ITPKeypadEvent irLastEvent;
static int  g_IR_TX_HAS_INIT = 0;
static int  g_RxIndex=0;

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

    if(signal != -1)
    {
        //printf("IrProbe(),signal=0x%x\n",signal);

        #ifdef  ENABLE_DBG_COMPARE_CODE
        {
            uint32_t modFreq = ithReadRegA(ITH_IR_BASE + ITH_IR_MOD_FILTER_REG);

            if(signal==0x3FFF)
            {
                g_RxIndex = 0;
            }
            else
            {
                int highR,lowR;

                if( 20 > g_IrTxBuf[g_RxIndex] )
                {
                    highR = g_IrTxBuf[g_RxIndex] + 1;
                    if(g_IrTxBuf[g_RxIndex]>1)  lowR = g_IrTxBuf[g_RxIndex] - 1;
                    else                        lowR = 0;
                }
                else
                {
                    highR = g_IrTxBuf[g_RxIndex] + (g_IrTxBuf[g_RxIndex]*5)/100;
                    lowR = g_IrTxBuf[g_RxIndex] - (g_IrTxBuf[g_RxIndex]*5)/100;
                }

                printf(" ### check ir-RX code, signal=%x, g_RxIndex=%x,H&L=[%x,%x], modfrq=%x\n",signal,g_RxIndex,highR,lowR,modFreq);
                if( (signal > highR) || (signal < lowR) )
                //if( signal != g_IrTxBuf[g_RxIndex] )
                {
                    printf("error, IR code RX != TX, index=%x,[%x,%x] \n",g_RxIndex,signal,g_IrTxBuf[g_RxIndex]);
                    //while(1);
                }
                g_RxIndex++;
            }
        }
        #endif

        switch (irCurrState)
        {
        case WAIT_START:
            //printf("WStart\n");
            if ((signal >= irThresholds[WAIT_START][0]) && (signal <= irThresholds[WAIT_START][1]))
            {
                //printf("WB.1\n");
                irCurrState = WAIT_BIT;
            }
            else if ((signal >= irThresholds[WAIT_START_REPEAT_KEY][0]) && (signal <= irThresholds[WAIT_START_REPEAT_KEY][1]))
            {
                code = irRecvCode;

                //printf("WE.0,code=%x\n",irRecvCode);

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
            {
                //printf("W_S\n");
                irCurrState = WAIT_START; // error
            }

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
            printf("IR wait end!\n");
            if (signal >= irThresholds[WAIT_END][1]/*(signal >= irThresholds[WAIT_END][0]) && (signal <= irThresholds[WAIT_END][1])*/)
            {
                if (irRecvBitCount >= irTypeTable[5])
                {
                    //printf("WE1:code=%x\n",irRecvCode);
                    completeACode = 1;
                    code = irRecvCode;
                }
                else
                {
                    //printf("WE2\n");
                    completeACode  = 0;
                }

                irRecvBitCount = 0;
                irCurrState = WAIT_START;
            }
            else
            {
                // error
                //printf("WE3\n");
                irRecvBitCount = 0;
                irRepeatKeyPress = irRepeatKeyHold = 0;
                irRepeatKeyFast = irRepeatKeyCnt = 0;
                completeACode = 0;
                irCurrState = WAIT_START;
            }
            break;

        default:
            printf(" probe_default!!\n");
            break;
        }

        if (completeACode)
        {
            //printf(" got IR code=%x\n",code);
            return code;
        }
    }
    //printf("IR NOT complete!!\n");
    return -1;
}

static void IrTxSend(uint16_t *ptr)
{
    int i;
    int IrTxCnt = sizeof(g_IrTxBuf)/sizeof(uint16_t);

    if(ptr) printf("### IrTxSend() ###\n");
    printf("## DO RX clear ##\n");
    ithIrCtrlEnable(ITH_IR_EN);
    for(i=0; i<IrTxCnt; i++)
    {
        printf("irTx send:[%x]=%x\n",i,ptr[i]);
        ithIrTxTransmit(ptr[i]);
    }
}

#ifdef  ENABLE_IR_INTR
static void IrIntrHandler(void* arg)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    int code = IrProbe();

    printf("IR code: 0x%X\n", code);
    //printf("RxModFreq=0x%x\n",ithReadRegA(ITH_IR_BASE + ITH_IR_MOD_FILTER_REG2));

    if (code == -1)
        return;

    if (((code & 0xFFFF) == CFG_IR_VENDOR_CODE) && (((code >> 16) & 0xFF) == (~((code >> 24) & 0xFF) & 0xFF)))
        code = (code >> 16) & 0xFF;

    xQueueSendFromISR(irQueue, &code, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
#else
static void IrIntrHandler(void* arg)
{
    int code;

    while(1)
    {
        //printf("start to IrProbe\n");
        code = IrProbe();

        if (code == -1)     continue;

        printf("code: 0x%X\n", code);

        if (((code & 0xFFFF) == CFG_IR_VENDOR_CODE) && (((code >> 16) & 0xFF) == (~((code >> 24) & 0xFF) & 0xFF)))
            code = (code >> 16) & 0xFF;

        printf("IR code: 0x%X\n", code);
        printf("RxModFreq=0x%x\n",ithReadRegA(ITH_IR_BASE + ITH_IR_MOD_FILTER_REG2));

        //xQueueSendFromISR(irQueue, &code, &xHigherPriorityTaskWoken);
        xQueueSend(irQueue, &code, 0);
    }
}

static void IrTxIntrHandler(void* arg)
{
    int code=0;

    while(1)
    {
        if(g_IR_TX_HAS_INIT)
        {
            code=0;
            if(xQueueReceive(irTxQueue, &code, 0))
            {
                printf("got irTX code=%x\n",code);
                //enable dma to SEND IR CODE
                //1.parse event to get IR TX code(code table)
                IrTxSend(g_IrTxBuf);
            }
        }
        usleep(1000);
    }
}
#endif

#define MAX_CODE_INDEX  2
static int IrCheckCodeMapping(int code)
{
    int i;

    for(i=0; i<MAX_CODE_INDEX; i++)
    {
        if(code == irValidTxCodeTable[i])   return 1;
    }
    return 0;
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
        printf("irThresholds[%d][0] = 0x%x, irThresholds[%d][1] = 0x%x\n", i, irThresholds[i][0], i, irThresholds[i][1]);
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

    ithIrTxInit(CFG_GPIO_IR_TX, 0, SAMP_DUR, PRECISION);

    // Prepare q and intr
    irTxQueue = xQueueCreate(QUEUE_LEN, (unsigned portBASE_TYPE) sizeof(int));

#ifdef  ENABLE_IR_INTR
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

    //Maybe the intr setting of IR-TX is not necessary.
    //So skips the initialization of IR-TX temporally
    //ITH_INTR_RC only just for IR-RX(no IR-TX's intr number)

    portEXIT_CRITICAL();
#else
    {
        int res;
        pthread_t task;
        pthread_attr_t attr;

        printf("Create IR pthread~~\n");

        pthread_attr_init(&attr);
        res = pthread_create(&task, &attr, IrIntrHandler, NULL);

        if(res)
        {
            printf( "[IR]%s() L#%ld: ERROR, create IrIntrHandler() thread fail! res=%ld\n", res );
            return -1;
        }

        //ithIrRxSetModFilter(0x770,0xFA0); //20~42

        // Enable IR Capture
        ithIrCtrlEnable(ITH_IR_EN);
    }


    {
        int res;
        pthread_t task;
        pthread_attr_t attr;

        printf("Create IR-TX pthread~~\n");

        pthread_attr_init(&attr);
        res = pthread_create(&task, &attr, IrTxIntrHandler, NULL);

        if(res)
        {
            printf( "[IR]%s() L#%ld: ERROR, create IrTxIntrHandler() thread fail! res=%ld\n", res );
            return -1;
        }

        // Enable IR Capture
        ithIrTxCtrlEnable(ITH_IR_EN);

        ithIrTxSetModFreq(0x839);//(0x1e);

        g_IR_TX_HAS_INIT = 1;
    }
    ithPrintRegA(ITH_IR_BASE, 0x40);

 #endif //#ifdef    ENABLE_IR_INTR

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

static int IrWrite(int file, char *ptr, int len, void* info)
{
    ITPKeypadEvent* ev = (ITPKeypadEvent*)ptr;
    printf("irTx: fd=%x, ptr=%x, ev->code=%x, len=%d, info=%x\n",file,ptr,ev->code,len,info);

    //get event
    if(ev->code)
    {
        if( IrCheckCodeMapping(ev->code) )
        {
            printf("sendQue2IrTx(TxQue=%x,code=%x)\n", irTxQueue, ev->code);
            xQueueSend(irTxQueue, &ev->code, 0);
        }
        else
        {
            return 0;//TODO: maybe return non "0" value??
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
    IrWrite,
    itpLseekDefault,
    IrIoctl,
    NULL
};
