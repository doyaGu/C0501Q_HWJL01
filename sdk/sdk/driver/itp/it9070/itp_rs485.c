/** @file
 * PAL RS485 functions.
 *
 * @author
 * @version 1.0
 * @date 20141014
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#include <errno.h>
#include <sys/socket.h>
#include <pthread.h>
#include "itp_cfg.h"
#ifndef _MSC_VER
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#endif

static unsigned int GPIO_RS485_0_TX_ENABLE;
static unsigned int GPIO_RS485_0_RX;
static unsigned int GPIO_RS485_0_TX;
static unsigned int GPIO_RS485_4_TX_ENABLE;
static unsigned int GPIO_RS485_4_RX;
static unsigned int GPIO_RS485_4_TX;
static ITHRS485Port RS485_port;
static ITHUartParity RS485_Parity;

static pthread_mutex_t RS485InternalMutex  = PTHREAD_MUTEX_INITIALIZER;

#if defined(CFG_RS485_0_INTR) || defined(CFG_RS485_1_INTR)
static unsigned int RS485_BUF_SIZE;

/* Queues used to hold received characters, and characters waiting to be
transmitted. */
static QueueHandle_t xRxedChars[2];
static QueueHandle_t xCharsForTx[2];

static void RS485IntrHandler(void* arg)
{
    ITHUartPort port = (ITHUartPort) arg;
    uint32_t status = ithUartClearIntr(port);
    signed char cChar;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    QueueHandle_t txQueue, rxQueue;

    if (port == ITH_UART0)
    {
        txQueue = xCharsForTx[0];
        rxQueue = xRxedChars[0];
    }
    else
    {
        txQueue = xCharsForTx[1];
        rxQueue = xRxedChars[1];
    }

    if (status & ITH_UART_THR_EMPTY)
    {
        unsigned int i;
        /* The interrupt was caused by the THR becoming empty.  Are there any
        more characters to transmit? */

        for (;;)
        {
            if (xQueueReceiveFromISR(txQueue, &cChar, &xHigherPriorityTaskWoken) == pdTRUE)
            {
                /* A character was retrieved from the queue so can be sent to the
                THR now. */
                ithUartPutChar(port, cChar);

                if (ithUartIsTxFull(port))
                    break;
            }
            else
            {
                /* Queue empty, nothing to send so turn off the Tx interrupt. */
                ithUartDisableIntr(port, ITH_UART_TX_READY);
                break;
            }
        }
    }

    if (status & ITH_UART_RECV_READY)
    {
        /* The interrupt was caused by a character being received.  Grab the
        character from the RHR and place it in the queue or received
        characters. */
        while (ithUartIsRxReady(port))
        {
            cChar = ithUartGetChar(port);
            xQueueSendFromISR( rxQueue, &cChar, &xHigherPriorityTaskWoken );
        }
    }

    /* If an event caused a task to unblock then we call "Yield from ISR" to
    ensure that the unblocked task is the task that executes when the interrupt
    completes if the unblocked task has a priority higher than the interrupted
    task. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
#elif defined(ITP_RS485_DMA)
#   define DMA_BUFFER_SIZE 8*1024
#   define DMA_TIMEOUT     10000

typedef struct LLP_CONTEXT_TAG
{
    uint32_t  SrcAddr;
    uint32_t  DstAddr;
    uint32_t  LLP;
    uint32_t  Control;
    uint32_t  TotalSize;
}LLP_CONTEXT;

static int readDmaChannel;
static int writeDmaChannel;
//static uint8_t* tempRxBuf = NULL;
static uint8_t *tempTxBuf = NULL;
static uint8_t *gpDMABuffer = 0;
static uint32_t gWriteIndex = 0;
static uint32_t gReadIndex = 0;
static LLP_CONTEXT *g_LLPCtxt = NULL;

static void
DummySleep(void)
{
    unsigned int idle = 100;
    unsigned int i = 0;
    unsigned int nothing = 0;

    for (i = 0; i < idle; i++)
    {
        nothing++;
    }
}

#endif // CFG_RS485_0_INTR |CFG_RS485_1_INTR|

static ITHUartPort RS485PutcharPort;

static int RS485Putchar(int c)
{
#if defined(CFG_RS485_0_INTR) || defined(CFG_RS485_1_INTR)
    QueueHandle_t txQueue;

    if (RS485PutcharPort == ITH_UART0)
    {
        txQueue = xCharsForTx[0];
    }
    else
    {
        txQueue = xCharsForTx[1];
    }

    if (ithGetCpuMode() == ITH_CPU_SYS)
    {
        xQueueSend( txQueue, &c, portMAX_DELAY );

        if (ithUartIsTxEmpty(RS485PutcharPort))
        {
            signed char cChar;

            if (xQueueReceive(txQueue, &cChar, 0) == pdTRUE)
            {
                ithUartPutChar(RS485PutcharPort, cChar);
            }
        }
    }
    else
    {
        xQueueSendFromISR( txQueue, &c, 0 );

        if (ithUartIsTxEmpty(RS485PutcharPort))
        {
            signed char cChar;
            portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

            if (xQueueReceiveFromISR(txQueue, &cChar, &xHigherPriorityTaskWoken) == pdTRUE)
            {
                ithUartPutChar(RS485PutcharPort, cChar);
            }

            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    ithUartEnableIntr(RS485PutcharPort, ITH_UART_TX_READY);

#else

    // Is transmitter ready?
    while (ithUartIsTxFull(RS485PutcharPort));
    ithUartPutChar(RS485PutcharPort, c);

#endif // CFG_RS485_0_INTR || CFG_RS485_1_INTR
    return c;
}

static void RS485Open(ITHUartPort port, unsigned int baud)
{
    pthread_mutex_lock(&RS485InternalMutex);

#if defined(CFG_RS485_0_ENABLE)
    if(RS485_port == ITH_RS485_0)
    {
        /* Set the required protocol. */
        if(RS485_Parity)
        {
          ithUartReset(port, baud, RS485_Parity, 1, 8);
        }else
        {
          ithUartReset(port, baud, ITH_UART_NONE, 1, 8);
        }

        /* Enable Rx and Tx. */
        ithUartSetMode(port, ITH_UART_DEFAULT, GPIO_RS485_0_TX, GPIO_RS485_0_RX);
    }
#endif
    pthread_mutex_unlock(&RS485InternalMutex);
}

static void RS485Close(void)
{
    return ;
}

static void RS485Reset(ITHUartPort port)
{
    unsigned int levelTx, levelRx;
    ITHUartFifoDepth fifoDepth;

    //pthread_mutex_lock(&RS485InternalMutex);
#if defined(CFG_RS485_0_INTR) || defined(CFG_RS485_1_INTR)
    {
        ITHIntr intr;

        if (port == ITH_UART0)
        {
            intr = ITH_INTR_UART0;

            /* Create the queues used to hold Rx and Tx characters. */
            xRxedChars[0] = xQueueCreate( RS485_BUF_SIZE, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
            xCharsForTx[0] = xQueueCreate( RS485_BUF_SIZE + 1, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
        }
        else
        {
            intr = ITH_INTR_UART1;

            /* Create the queues used to hold Rx and Tx characters. */
            xRxedChars[1] = xQueueCreate( RS485_BUF_SIZE, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
            xCharsForTx[1] = xQueueCreate( RS485_BUF_SIZE + 1, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
        }

        ithEnterCritical();

        /* Enable the Rx interrupts.  The Tx interrupts are not enabled
        until there are characters to be transmitted. */
        ithIntrDisableIrq(intr);
        ithUartClearIntr(port);
        ithIntrClearIrq(intr);

        ithIntrSetTriggerModeIrq(intr, ITH_INTR_EDGE);
        ithIntrRegisterHandlerIrq(intr, RS485IntrHandler, (void*)port);
        ithUartEnableIntr(port, ITH_UART_RX_READY);

        /* Enable the interrupts. */
        ithIntrEnableIrq(intr);

        ithExitCritical();
    }

#elif defined(ITP_RS485_DMA)
    if (port == ITH_UART1)
    {
        LLP_CONTEXT *llpaddr = NULL;

        readDmaChannel = ithDmaRequestCh("dma_rs485_read", ITH_DMA_CH_PRIO_HIGH_3, NULL, NULL);
        ithDmaReset(readDmaChannel);

        writeDmaChannel = ithDmaRequestCh("dma_rs485_write", ITH_DMA_CH_PRIO_HIGHEST, NULL, NULL);
        ithDmaReset(writeDmaChannel);

        //tempRxBuf = (uint8_t *)itpVmemAlloc(1024);
        tempTxBuf = (uint8_t *)itpVmemAlloc(DMA_BUFFER_SIZE);

        gpDMABuffer = (uint8_t *)itpVmemAlloc(DMA_BUFFER_SIZE);

        if (gpDMABuffer == NULL)
        {
            printf("Alloc DMA buffer fail\n");
        }
        else
        {
            g_LLPCtxt = (LLP_CONTEXT *)itpVmemAlloc(sizeof(LLP_CONTEXT) + 32);
            printf("--- g_LLPCtxt addr = 0x%x , sizeof(LLP_CONTEXT) = 0x%x---\n", g_LLPCtxt, sizeof(LLP_CONTEXT));
            llpaddr = (LLP_CONTEXT *)(((uint32_t)g_LLPCtxt + 0x1F) & ~(0x1F));
            printf("new g_LLPCtxt addr = 0x%x\n", llpaddr);
            llpaddr->SrcAddr =  le32_to_cpu(port);
            llpaddr->DstAddr = le32_to_cpu(gpDMABuffer);
            llpaddr->LLP = le32_to_cpu(llpaddr);
            llpaddr->TotalSize = le32_to_cpu(DMA_BUFFER_SIZE);
            llpaddr->Control = le32_to_cpu(0x00220000);

            ithDmaSetSrcAddr(readDmaChannel, port);
            ithDmaSetDstAddr(readDmaChannel, (uint32_t)gpDMABuffer);
            switch (port)
            {
            case ITH_UART0:
                ithDmaSetRequest(readDmaChannel, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_UART_RX, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
                break;
            case ITH_UART1:
                ithDmaSetRequest(readDmaChannel, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_UART2_RX, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
                break;
            }
            ithDmaSetSrcParams(readDmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
            ithDmaSetDstParams(readDmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
            ithDmaSetTxSize(readDmaChannel, DMA_BUFFER_SIZE);
            ithDmaSetBurst(readDmaChannel, ITH_DMA_BURST_1);
            ithDmaSetLLPAddr(readDmaChannel, (uint32_t)llpaddr);
            printf("llpaddr:0x%x\n", (uint32_t)llpaddr);

            ithUartEnableDmaMode2(port);
            ithDmaStart(readDmaChannel);
        }
    }
#endif // CFG_RS485_0_INTR || CFG_RS485_1_INTR

    if (RS485PutcharPort == 0)
        RS485PutcharPort = port;

    ithPutcharFunc = RS485Putchar;
    //pthread_mutex_unlock(&RS485InternalMutex);
}

static void RS485Reset1(void)
{
    int swuart_parity;

    //swuart_parity = ITP_SWUART_NONE;
    swuart_parity  = RS485_Parity;
    ioctl(ITP_DEVICE_SWUART, ITP_IOCTL_SET_PARITY, &swuart_parity);
}


static int RS485Read(int file, char *ptr, int len, void* info)
{
    ITHUartPort port = (ITHUartPort) info;

    //pthread_mutex_lock(&RS485InternalMutex);
    //Disable TX gpio
    if(GPIO_RS485_0_TX_ENABLE)
    {
        ithGpioClear(GPIO_RS485_0_TX_ENABLE);
        ithGpioSetMode(GPIO_RS485_0_TX_ENABLE, ITH_GPIO_MODE0);
        ithGpioSetIn(GPIO_RS485_0_TX_ENABLE);
    }

#if defined(CFG_RS485_0_INTR) || defined(CFG_RS485_1_INTR)
    QueueHandle_t rxQueue = (port == ITH_UART0) ? xRxedChars[0] : xRxedChars[1];
    int count = 0;

    while (xQueueReceive(rxQueue, ptr++, 0))
        count++;

    //pthread_mutex_unlock(&RS485InternalMutex);
    if (count > 0)
        return count;

    return -1;

#elif defined(ITP_RS485_DMA)
    if ((ITHUartPort)info == ITH_UART1)
    {
        uint32_t transferSize = 0;
        uint32_t dataSize = 0;

        //ithDmaDumpReg(readDmaChannel); //for debug
        //printf("[0x5C]:0x%x\n",ithReadRegA(port + 0x5C));

        transferSize = ithReadRegA(ITH_DMA_BASE + ITH_DMA_C0_TX_SIZE_REG + readDmaChannel * ITH_DMA_CH_OFFSET);

        gWriteIndex = (DMA_BUFFER_SIZE - transferSize);

        if (gWriteIndex != gReadIndex)
        {
            printf("[transferSize]:0x%x [gWriteIndex]:0x%x [gReadIndex]:0x%x\n", transferSize, gWriteIndex, gReadIndex);
            if (gWriteIndex < gReadIndex)
            {
                dataSize = (DMA_BUFFER_SIZE - gReadIndex) + gWriteIndex;
                ithInvalidateDCacheRange(gpDMABuffer, DMA_BUFFER_SIZE);
                memcpy(ptr, gpDMABuffer + gReadIndex, DMA_BUFFER_SIZE - gReadIndex);
                memcpy(ptr + (DMA_BUFFER_SIZE - gReadIndex), gpDMABuffer, gWriteIndex);
            }
            else
            {
                dataSize = gWriteIndex - gReadIndex;
                ithInvalidateDCacheRange(gpDMABuffer, DMA_BUFFER_SIZE);
                memcpy(ptr, gpDMABuffer + gReadIndex, dataSize);
            }

            gReadIndex = gWriteIndex;
            printf("dataSize:%d\n", dataSize);

        }

        ithUartDisableDmaMode2(port);
        //pthread_mutex_unlock(&RS485InternalMutex);
        return dataSize;
    }
    else
    {
        int count = 0;

        // Is a character waiting?
        while (ithUartIsRxReady(port))
        {
            *ptr++ = ithUartGetChar(port);       // Read character from RS485
            count++;
        }
        //pthread_mutex_unlock(&RS485InternalMutex);
        if (count > 0)
            return count;

        return 0;
    }
#else
    int count = 0;

    // Is a character waiting?
    while (ithUartIsRxReady(port))
    {
        *ptr++ = ithUartGetChar(port);       // Read character from RS485
        count++;
    }
    //pthread_mutex_unlock(&RS485InternalMutex);
    if (count > 0)
        return count;

    return 0;

#endif // CFG_RS485_0_INTR || CFG_RS485_1_INTR
}

static int RS485Read1(int file, char *ptr, int len, void* info)
{
    ITHUartPort port = (ITHUartPort) info;

    //pthread_mutex_lock(&RS485InternalMutex);
    //Disable TX gpio
    if(GPIO_RS485_4_TX_ENABLE)
    {
        ithGpioClear(GPIO_RS485_4_TX_ENABLE);
        ithGpioSetMode(GPIO_RS485_4_TX_ENABLE, ITH_GPIO_MODE0);
        ithGpioSetIn(GPIO_RS485_4_TX_ENABLE);
    }
    
#if defined(CFG_DBG_SWUART_CODEC) || defined(CFG_SWUART_CODEC_ENABLE)
    return ithCodecUartRead(ptr, len);
#else
	return 0;
#endif
}

static int RS485Write1(int file, char *ptr, int len, void* info)
{
    ITHUartPort port = (ITHUartPort) info;
    int i;

    //pthread_mutex_lock(&RS485InternalMutex);
    //Enable TX gpio
    if(GPIO_RS485_4_TX_ENABLE)
    {
        ithGpioSet(GPIO_RS485_4_TX_ENABLE);
        ithGpioSetMode(GPIO_RS485_4_TX_ENABLE, ITH_GPIO_MODE0);
        ithGpioSetOut(GPIO_RS485_4_TX_ENABLE);
    }

#ifndef NDEBUG
    portSAVEDISABLE_INTERRUPTS();
#endif

#if defined(CFG_DBG_SWUART_CODEC) || defined(CFG_SWUART_CODEC_ENABLE)
    //for RS485_1
    ithCodecUartWrite(ptr, len);
#endif

#ifndef NDEBUG
    portRESTORE_INTERRUPTS();
#endif
    //pthread_mutex_unlock(&RS485InternalMutex);

    return len;

}

static int RS485Write(int file, char *ptr, int len, void* info)
{
    ITHUartPort port = (ITHUartPort) info;
    int i;

    //pthread_mutex_lock(&RS485InternalMutex);
    //Enable TX gpio
    if(GPIO_RS485_0_TX_ENABLE)
    {
        ithGpioSet(GPIO_RS485_0_TX_ENABLE);
        ithGpioSetMode(GPIO_RS485_0_TX_ENABLE, ITH_GPIO_MODE0);
        ithGpioSetOut(GPIO_RS485_0_TX_ENABLE);
    }

#if defined(CFG_RS485_0_INTR) || defined(CFG_RS485_1_INTR)
    QueueHandle_t txQueue = (port == ITH_UART0) ? xCharsForTx[0] : xCharsForTx[1];

    for (i = 0; i < len; i++)
    {
        /* Place the character in the queue of characters to be transmitted. */
        if( xQueueSend( txQueue, &ptr[i], portMAX_DELAY ) != pdPASS )
            break;
    }

    if (ithUartIsTxEmpty(port))
    {
        signed char cChar;

        if (xQueueReceive(txQueue, &cChar, 0) == pdTRUE)
        {
            ithUartPutChar(port, cChar);
        }
    }

    /* Turn on the Tx interrupt so the ISR will remove the character from the
    queue and send it.   This does not need to be in a critical section as
    if the interrupt has already removed the character the next interrupt
    will simply turn off the Tx interrupt again. */
    ithUartEnableIntr(port, ITH_UART_TX_READY);

    //pthread_mutex_unlock(&RS485InternalMutex);
    return i;

#elif defined(ITP_RS485_DMA)
    if ((ITHUartPort)info == ITH_UART1)
    {
        uint32_t timeout_ms = 3000;
        uint32_t dstWidth = ITH_DMA_WIDTH_16;
        uint32_t srcWidth = ITH_DMA_WIDTH_16;
        ITHDmaBurst burstSize    = ITH_DMA_BURST_1;

        /* Lock */
        //ithLockMutex(ithStorMutex);

        if (tempTxBuf == NULL)
        {
            return 0;
        }

        memcpy(tempTxBuf, ptr, len);

        if (len & 0x1)  /** size is 8-bits */
        {
            srcWidth = 1;
        }
        else if (len & 0x2) /** size is 16-bits */
        {
            if ((uint8_t)tempTxBuf & 0x1)
                srcWidth = 1;
            else
                srcWidth = 2;
        }
        else  /** size is 32-bits */
        {
            if ((uint8_t)tempTxBuf & 0x1)
                srcWidth = 1;
            else if ((uint8_t)tempTxBuf & 0x2)
                srcWidth = 2;
            else
                srcWidth = 4;
        }

        // 32 bits
        if (srcWidth  == 4)
        {
            srcWidth     = ITH_DMA_WIDTH_32;
            dstWidth     = ITH_DMA_WIDTH_32;
            burstSize    = ITH_DMA_BURST_4; // 4
        }
        else if (srcWidth  == 2) // 16 bits
        {
            srcWidth     = ITH_DMA_WIDTH_16;
            dstWidth     = ITH_DMA_WIDTH_16;
            burstSize    = ITH_DMA_BURST_4;
        }
        else    // 8 bits
        {
            srcWidth     = ITH_DMA_WIDTH_8;
            dstWidth     = ITH_DMA_WIDTH_8;
            burstSize    = ITH_DMA_BURST_8; // 8
        }

        ithDmaSetSrcAddr(writeDmaChannel, (uint32_t)tempTxBuf);
        ithDmaSetDstAddr(writeDmaChannel, (ITHUartPort)info);
        switch ((ITHUartPort)info)
        {
        case ITH_UART0:
            ithDmaSetRequest(writeDmaChannel, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_UART_TX);
            break;
        case ITH_UART1:
            ithDmaSetRequest(writeDmaChannel, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_UART2_TX);
            break;
        }
        ithDmaSetSrcParams(writeDmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
        ithDmaSetDstParams(writeDmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
        ithDmaSetTxSize(writeDmaChannel, len);
        ithDmaSetBurst(writeDmaChannel, ITH_DMA_BURST_1);
        ithUartEnableDmaMode2((ITHUartPort)info);

        ithDmaStart(writeDmaChannel);

        while (ithDmaIsBusy(writeDmaChannel) && --timeout_ms)
        {
            DummySleep();
        }

        ithUartDisableDmaMode2((ITHUartPort)info);

        /* Unlock */
        //ithUnlockMutex(ithStorMutex);
        //pthread_mutex_unlock(&RS485InternalMutex);

        return len;
    }
    else
    {
#   ifndef NDEBUG
        portSAVEDISABLE_INTERRUPTS();
#   endif
        for (i = 0; i < len; i++)
        {
            // Is transmitter ready?
            while (ithUartIsTxFull(port));
            ithUartPutChar(port, *ptr++);
        }
#   ifndef NDEBUG
        portRESTORE_INTERRUPTS();
#   endif
        //pthread_mutex_unlock(&RS485InternalMutex);
        return len;
    }
#else

#ifndef NDEBUG
    portSAVEDISABLE_INTERRUPTS();
#endif
    for (i = 0; i < len; i++)
    {
        // Is transmitter ready?
        while (ithUartIsTxFull(port));
        ithUartPutChar(port, *ptr++);
    }
#ifndef NDEBUG
    portRESTORE_INTERRUPTS();
#endif
    //pthread_mutex_unlock(&RS485InternalMutex);

    return len;

#endif // CFG_RS485_0_INTR || CFG_RS485_1_INTR
}


static int RS485Ioctl(int file, unsigned long request, void* ptr, void* info)
{
    ITHUartPort port = (ITHUartPort) info;

    switch (request)
    {
    case FIONREAD:
        // Is a character waiting?
    #if defined(CFG_RS485_0_INTR) || defined(CFG_RS485_1_INTR)
        {
            QueueHandle_t rxQueue = (port == ITH_UART0) ? xRxedChars[0] : xRxedChars[1];
            *(int*)ptr = uxQueueMessagesWaiting(rxQueue);
        }
    #else
        *(int*)ptr = ithUartIsRxReady(port) ? 1 : 0;
    #endif // CFG_RS485_0_INTR || CFG_RS485_1_INTR
        break;

    case ITP_IOCTL_INIT: //set baud rate
        ithPutcharFunc = RS485Putchar;
        break;

    case ITP_IOCTL_ON:
        RS485_port = *(ITHRS485Port*)ptr;

        switch(RS485_port)
        {
        case ITH_RS485_0:
        {
            if(GPIO_RS485_0_TX_ENABLE)
            {
                ithGpioClear(GPIO_RS485_0_TX_ENABLE);
                ithGpioSetMode(GPIO_RS485_0_TX_ENABLE, ITH_GPIO_MODE0);
                ithGpioSetIn(GPIO_RS485_0_TX_ENABLE);
            }
#if defined(CFG_RS485_0_ENABLE)
            GPIO_RS485_0_TX_ENABLE = CFG_GPIO_RS485_0_TX_ENABLE;
            GPIO_RS485_0_RX = CFG_GPIO_RS485_0_RX;
            GPIO_RS485_0_TX = CFG_GPIO_RS485_0_TX;

            ithGpioClear(GPIO_RS485_0_RX);
            ithGpioSetMode(GPIO_RS485_0_RX, ITH_GPIO_MODE0);
            ithGpioSetIn(GPIO_RS485_0_RX);
            ithGpioClear(GPIO_RS485_0_TX);
            ithGpioSetMode(GPIO_RS485_0_TX, ITH_GPIO_MODE0);
            ithGpioSetIn(GPIO_RS485_0_TX);

#if defined(CFG_RS485_0_INTR) || defined(CFG_RS485_1_INTR)
            RS485_BUF_SIZE = CFG_RS485_0_BUF_SIZE;
#endif
            RS485Open(port, CFG_RS485_0_BAUDRATE);
#endif
            break;
        }
        case ITH_RS485_1:
        {
            if(GPIO_RS485_4_TX_ENABLE)
            {
                ithGpioClear(GPIO_RS485_4_TX_ENABLE);
                ithGpioSetMode(GPIO_RS485_4_TX_ENABLE, ITH_GPIO_MODE0);
                ithGpioSetIn(GPIO_RS485_4_TX_ENABLE);
            }
#if defined(CFG_RS485_4_ENABLE)
            GPIO_RS485_4_TX_ENABLE = CFG_GPIO_RS485_4_TX_ENABLE;
            //GPIO_RS485_4_RX = CFG_GPIO_RS485_4_RX;
            //GPIO_RS485_4_TX = CFG_GPIO_RS485_4_TX;

#if defined(CFG_RS485_0_INTR) || defined(CFG_RS485_4_INTR)
            RS485_BUF_SIZE = CFG_RS485_4_BUF_SIZE;
#endif
            RS485Open(port, CFG_RS485_4_BAUDRATE);
#endif
            break;
        }

        }
        break;

    case ITP_IOCTL_OFF:
        RS485Close();
        break;

    case ITP_IOCTL_RESET:
        RS485_Parity = *(ITHUartParity*)ptr;
#if defined(CFG_RS485_0_ENABLE)
        RS485Reset(port);
#endif
#if defined(CFG_RS485_4_ENABLE)
        RS485Reset1();
#endif
        break;

    default:
        errno = -1;
        return -1;
    }
    return 0;
}


const ITPDevice itpDeviceRS485_0 =
{
    ":rs485_0",
    itpOpenDefault,
    itpCloseDefault,
    RS485Read,
    RS485Write,
    itpLseekDefault,
    RS485Ioctl,
    (void*)ITH_UART1
};

const ITPDevice itpDeviceRS485_4 =
{
    ":rs485_4",
    itpOpenDefault,
    itpCloseDefault,
    RS485Read1,
    RS485Write1,
    itpLseekDefault,
    RS485Ioctl,
    (void*)ITH_UART1
};



