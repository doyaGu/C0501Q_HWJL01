/** @file
 * PAL UART functions.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2011-2012
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#include <errno.h>
#include <sys/socket.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#include "itp_cfg.h"

#ifdef CFG_UART_DMA
#define DMA_BUFFER_SIZE CFG_UART_DMA_BUF_SIZE
#define DMA_TIMEOUT     10000
static bool boot_mode = false;
#endif

typedef struct LLP_CONTEXT_TAG
{
    uint32_t  SrcAddr;
    uint32_t  DstAddr;
    uint32_t  LLP;
    uint32_t  Control;
    uint32_t  TotalSize;
}LLP_CONTEXT;

typedef struct _UART_OBJ
{
    ITHUartPort   port;
    ITHIntr       Intr;
    int           txgpio;
    int           rxgpio;
    QueueHandle_t xRxedChars;
    QueueHandle_t xCharsForTx;
    int           RxQueueFull;
	ITPPendFunction  itpUartDeferIntrHandler;
	uint8_t       UartDeferIntrOn;
	uint32_t      timeout;	
#ifdef CFG_UART_DMA
    char*         rdch_name;
    char*         wtch_name;
    int           dma_tx_req;
    int           dma_rx_req;
    int           readDmaChannel;
    int           writeDmaChannel;
    uint8_t *     tempTxBuf;
    uint8_t *     gpDMABuffer;
    uint32_t      gWriteIndex;
    uint32_t      gReadIndex;
    LLP_CONTEXT * g_LLPCtxt;
#endif
}UART_OBJ;

static UART_OBJ Uartobj[4] =
{
    {
        ITH_UART0,
        ITH_INTR_UART0,
#ifdef CFG_GPIO_UART0_TX
        CFG_GPIO_UART0_TX,
        CFG_GPIO_UART0_RX,
#else
        -1,
        -1,
#endif
        NULL,                           // xRxedChars
        NULL,                           // xCharsForTx
        0,                              //RxQueueFull
        NULL,
        0,
        0,								//timeout
#ifdef CFG_UART_DMA
        NULL,
        NULL,
        ITH_DMA_HW_UART0_TX,
        ITH_DMA_HW_UART0_RX,
#endif
    },
    {
        ITH_UART1,
        ITH_INTR_UART1,
#ifdef CFG_GPIO_UART1_TX
        CFG_GPIO_UART1_TX,
        CFG_GPIO_UART1_RX,
#else
        -1,
        -1,
#endif
        NULL,                           // xRxedChars
        NULL,                           // xCharsForTx
        0,                              //RxQueueFull
		NULL,
		0,
		0,								//timeout		
#ifdef CFG_UART_DMA
        NULL,
        NULL,
        ITH_DMA_HW_UART1_TX,
        ITH_DMA_HW_UART1_RX,
#endif
    },
        {
        ITH_UART2,
#if (CFG_CHIP_FAMILY == 9850)
        ITH_INTR_UART2,
#else
        -1,
#endif
#ifdef CFG_GPIO_UART2_TX
        CFG_GPIO_UART2_TX,
        CFG_GPIO_UART2_RX,
#else
        -1,
        -1,
#endif
        NULL,                           // xRxedChars
        NULL,                           // xCharsForTx
        0,                              //RxQueueFull
		NULL,
		0,
		0,								//timeout        
#ifdef CFG_UART_DMA
        0,
        0,
        0,
        0,
#endif
    },
        {
        ITH_UART3,
#if (CFG_CHIP_FAMILY == 9850)
        ITH_INTR_UART3,
#else
        -1,
#endif
#ifdef CFG_GPIO_UART3_TX
        CFG_GPIO_UART3_TX,
        CFG_GPIO_UART3_RX,
#else
        -1,
        -1,
#endif
        NULL,                           // xRxedChars
        NULL,                           // xCharsForTx
        0,                              //RxQueueFull
		NULL,
		0,
		0,								//timeout        		
#ifdef CFG_UART_DMA
        0,
        0,
        0,
        0,
#endif
    },

};

static void UartDefaultIntrHandler(void)
{
    // DO NOTHING
}


#ifdef CFG_UART_INTR
static void UartIntrHandler(void *arg)
{
    ITHUartPort   port                     = (ITHUartPort) arg;
    uint32_t      status                   = ithUartClearIntr(port);
    signed char   cChar;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    QueueHandle_t txQueue, rxQueue;
    int           *pRxqueueFull            = NULL;
    long          result                   = 0;

    UART_OBJ *uartobj = NULL;

    if (port == ITH_UART0)
        uartobj = &Uartobj[0];
    else if (port == ITH_UART1)
        uartobj = &Uartobj[1];
    else if (port == ITH_UART2)
        uartobj = &Uartobj[2];
    else
        uartobj = &Uartobj[3];

    txQueue = uartobj->xCharsForTx;
    rxQueue = uartobj->xRxedChars;
    pRxqueueFull = &uartobj->RxQueueFull;

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
            cChar  = ithUartGetChar(port);
            result = xQueueSendFromISR(rxQueue, &cChar, &xHigherPriorityTaskWoken);
            if (result == 0)
                *pRxqueueFull = 1;

			if(uartobj->UartDeferIntrOn)
				itpPendFunctionCallFromISR(uartobj->itpUartDeferIntrHandler,NULL,NULL);
			else
				uartobj->itpUartDeferIntrHandler(NULL,NULL);
        }
    }

    /* If an event caused a task to unblock then we call "Yield from ISR" to
        ensure that the unblocked task is the task that executes when the interrupt
        completes if the unblocked task has a priority higher than the interrupted
        task. */
    //portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
#elif defined(CFG_UART_DMA)
static void
DummySleep(void)
{
    unsigned int idle    = 100;
    unsigned int i       = 0;
    unsigned int nothing = 0;

    for (i = 0; i < idle; i++)
    {
        nothing++;
    }
}

#endif // CFG_UART_INTR

static ITHUartPort  uartPutcharPort;
static unsigned int Uartbaudrate;
//static int flag = 0;
static int UartPutchar(int c)
{
#ifdef CFG_UART_INTR
    int i = 0;
    if (ithGetCpuMode() == ITH_CPU_SYS)
    {
        QueueHandle_t txQueue;
        UART_OBJ *uartobj = NULL;

        if (uartPutcharPort == ITH_UART0)
            uartobj = &Uartobj[0];
        else if (uartPutcharPort == ITH_UART1)
            uartobj = &Uartobj[1];
        else if (uartPutcharPort == ITH_UART2)
            uartobj = &Uartobj[2];
        else
            uartobj = &Uartobj[3];

    	if (!uartobj->xCharsForTx && !uartobj->xRxedChars)
        {
            /* Create the queues used to hold Rx and Tx characters. */
            uartobj->xRxedChars  = xQueueCreate(CFG_UART_BUF_SIZE, ( unsigned portBASE_TYPE ) sizeof( signed char ));
            uartobj->xCharsForTx = xQueueCreate(CFG_UART_BUF_SIZE + 1, ( unsigned portBASE_TYPE ) sizeof( signed char ));
        }

        txQueue = uartobj->xCharsForTx;

        do
        {
            /* Place the character in the queue of characters to be transmitted. */
            if (xQueueSend(txQueue, &c, 0) != pdPASS)
            {
                if (ithUartIsTxEmpty(uartPutcharPort))
                {
                    signed char cChar;
                    if (xQueueReceive(txQueue, &cChar, 0) == pdTRUE)
                        ithUartPutChar(uartPutcharPort, cChar);
                }
                ithUartEnableIntr(uartPutcharPort, ITH_UART_TX_READY);
            }
            else
            {
                i++;
            }
        } while (i < 1);

        if (ithUartIsTxEmpty(uartPutcharPort))
        {
            signed char cChar;

            if (xQueueReceive(txQueue, &cChar, 0) == pdTRUE)
            {
                ithUartPutChar(uartPutcharPort, cChar);
            }
        }
        ithUartEnableIntr(uartPutcharPort, ITH_UART_TX_READY);
    }
    else
    {
        while (ithUartIsTxFull(uartPutcharPort));
        ithUartPutChar(uartPutcharPort, c);
    }
#else
    // Is transmitter ready?
    while (ithUartIsTxFull(uartPutcharPort));
    ithUartPutChar(uartPutcharPort, c);
#endif // CFG_UART_INTR
    return c;
}

static int UartWriteDefault(ITHUartPort port, char *ptr, int len)
{
	int i = 0;
#ifdef CFG_UART_FORCE_FLUSH
	ithEnterCritical();
#endif
	for (i = 0; i < len; i++)
	{
	// Is transmitter ready?
	while (ithUartIsTxFull(port));
	   ithUartPutChar(port, *ptr++);
	}
#ifdef CFG_UART_FORCE_FLUSH
	ithExitCritical();
#endif
	return len;
}

static int UartReadDefault(ITHUartPort port, char *ptr, int len)
{	
	#define TIMEOUT 1000
    int count = 0;
	uint32_t  lasttime;
    for (;;)
    {
        if (ithUartIsRxReady(port))
        {
            int timeout = TIMEOUT;
            *ptr++ = ithUartGetChar(port);       // Read character from uart
            count++;
            while (count < len)
            {
                // Is a character waiting?
                if (ithUartIsRxReady(port))
                {
                    *ptr++  = ithUartGetChar(port);      // Read character from uart
                    count++;
                    timeout = TIMEOUT;
                }
                else if (timeout-- <= 0)
                {
                    //ithPrintf("count=%d len=%d\n", count, len);
                    return count;
                }
            }
        }
        else
        {
            /* if sleep a lot of time, uart will lost data */
            usleep(10);
        }
    }
    return count;
}

static int itpUartOpen(const char *name, int flags, int mode, void *info)
{
    return 0;
}

static void UartOpen(ITHUartPort port, ITHUartParity Parity)
{
    if (Parity)
    {
        ithUartReset(port, Uartbaudrate, Parity, 1, 8);
    }
    else
    {
        ithUartReset(port, Uartbaudrate, ITH_UART_NONE, 1, 8);
    }

    /* Enable Rx and Tx. */
#ifdef CFG_GPIO_UART1_TX
    ithUartSetMode(port, ITH_UART_DEFAULT, CFG_GPIO_UART1_TX, CFG_GPIO_UART1_RX);
#else
    ithUartSetMode(port, ITH_UART_DEFAULT, 0,                 0);
#endif
}

static void UartReset(ITHUartPort port, unsigned int baud)
{
    unsigned int     levelTx, levelRx;
    ITHUartFifoDepth fifoDepth;
	uint32_t value = 0;

    UART_OBJ *uartobj = NULL;

    if (port == ITH_UART0)
    {
        uartobj = &Uartobj[0];
#ifdef CFG_UART_DMA
		uartobj->rdch_name = "dma_uart0_read";
		uartobj->wtch_name = "dma_uart0_write";
#endif
    }
    else if (port == ITH_UART1)
	{
        uartobj = &Uartobj[1];
#ifdef CFG_UART_DMA
		uartobj->rdch_name = "dma_uart1_read";
		uartobj->wtch_name = "dma_uart1_write";
#endif		
	}
    else if (port == ITH_UART2)
        uartobj = &Uartobj[2];
    else
        uartobj = &Uartobj[3];

    Uartbaudrate = baud;

	/* Set the required protocol. */
    ithUartReset(uartobj->port, baud, ITH_UART_NONE, 1, 8);

    ithUartSetMode(uartobj->port, ITH_UART_DEFAULT, uartobj->txgpio, uartobj->rxgpio);

#ifdef CFG_UART_INTR

    if (!uartobj->xCharsForTx && !uartobj->xRxedChars)
    {
        /* Create the queues used to hold Rx and Tx characters. */
        uartobj->xRxedChars  = xQueueCreate(CFG_UART_BUF_SIZE, ( unsigned portBASE_TYPE ) sizeof( signed char ));
        uartobj->xCharsForTx = xQueueCreate(CFG_UART_BUF_SIZE + 1, ( unsigned portBASE_TYPE ) sizeof( signed char ));
    }

    ithEnterCritical();

	uartobj->itpUartDeferIntrHandler = UartDefaultIntrHandler;
	uartobj->UartDeferIntrOn = 0;
	
    /* Enable the Rx interrupts.  The Tx interrupts are not enabled
        until there are characters to be transmitted. */
    ithIntrDisableIrq(uartobj->Intr);
    ithUartClearIntr(uartobj->port);
    ithIntrClearIrq(uartobj->Intr);

    ithIntrSetTriggerModeIrq(uartobj->Intr, ITH_INTR_LEVEL);
    ithIntrRegisterHandlerIrq(uartobj->Intr, UartIntrHandler, (void *)uartobj->port);
    ithUartEnableIntr(uartobj->port, ITH_UART_RX_READY);

    /* Enable the interrupts. */
    ithIntrEnableIrq(uartobj->Intr);
    ithExitCritical();

#elif defined(CFG_UART_DMA)
if((port == ITH_UART0) || (port == ITH_UART1))
{
	if (!boot_mode)
	{
    LLP_CONTEXT *llpaddr = NULL;

    uartobj->readDmaChannel = ithDmaRequestCh(uartobj->rdch_name, ITH_DMA_CH_PRIO_HIGH_3, NULL, NULL);
    ithDmaReset(uartobj->readDmaChannel);
    uartobj->writeDmaChannel = ithDmaRequestCh(uartobj->wtch_name, ITH_DMA_CH_PRIO_HIGHEST, NULL, NULL);
    ithDmaReset(uartobj->writeDmaChannel);
		
    uartobj->tempTxBuf = (uint8_t *)itpVmemAlloc(DMA_BUFFER_SIZE);
    uartobj->gpDMABuffer = (uint8_t *)itpVmemAlloc(DMA_BUFFER_SIZE);

	//ithPrintf("uartobj->gpDMABuffer=0x%x\n", uartobj->gpDMABuffer);
    if (uartobj->gpDMABuffer == NULL || uartobj->tempTxBuf == NULL)
    {
        ithPrintf("Alloc DMA buffer fail\n");
    }
    else
    {
	    
        uartobj->g_LLPCtxt= (LLP_CONTEXT *)itpVmemAlloc(sizeof(LLP_CONTEXT) + 32);
        //ithPrintf("--- g_LLPCtxt addr = 0x%x , sizeof(LLP_CONTEXT) = 0x%x---\n", uartobj->g_LLPCtxt, sizeof(LLP_CONTEXT));
        llpaddr = (LLP_CONTEXT *)(((uint32_t)uartobj->g_LLPCtxt + 0x1F) & ~(0x1F));
        //ithPrintf("new g_LLPCtxt addr = 0x%x\n", llpaddr);
        llpaddr->SrcAddr =  le32_to_cpu(port);
        llpaddr->DstAddr = le32_to_cpu(uartobj->gpDMABuffer);
        llpaddr->LLP = le32_to_cpu(llpaddr);
        llpaddr->TotalSize = le32_to_cpu(DMA_BUFFER_SIZE);
        llpaddr->Control = le32_to_cpu(0x00220000);

		
        ithDmaSetSrcAddr(uartobj->readDmaChannel, port);
        ithDmaSetDstAddr(uartobj->readDmaChannel, (uint32_t)uartobj->gpDMABuffer);
        ithDmaSetRequest(uartobj->readDmaChannel, ITH_DMA_HW_HANDSHAKE_MODE, uartobj->dma_rx_req, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);

        ithDmaSetSrcParams(uartobj->readDmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
        ithDmaSetDstParams(uartobj->readDmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
        ithDmaSetTxSize(uartobj->readDmaChannel, DMA_BUFFER_SIZE);
        ithDmaSetBurst(uartobj->readDmaChannel, ITH_DMA_BURST_1);
			
        ithDmaSetLLPAddr(uartobj->readDmaChannel, (uint32_t)llpaddr);
		//ithPrintf("llpaddr:0x%x\n", (uint32_t)llpaddr);

        ithUartEnableDmaMode2(port);
        ithDmaStart(uartobj->readDmaChannel);
    }
    }
}
#endif
    if (uartPutcharPort == 0)
        uartPutcharPort = port;
}

static int UartRead(int file, char *ptr, int len, void *info)
{
    ITHUartPort   port = (ITHUartPort) info;
    UART_OBJ *uartobj = NULL;
	uint32_t  lasttime;

    if (port == ITH_UART0)
        uartobj = &Uartobj[0];
    else if (port == ITH_UART1)
        uartobj = &Uartobj[1];
    else if (port == ITH_UART2)
        uartobj = &Uartobj[2];
    else
        uartobj = &Uartobj[3];

#ifdef CFG_UART_INTR
    QueueHandle_t rxQueue;
    int           *pRxqueueFull = NULL;

    rxQueue = uartobj->xRxedChars;
    pRxqueueFull = &uartobj->RxQueueFull;

    int count = 0;

	if(uartobj->timeout)
	{
		if (xQueueReceive(rxQueue, ptr, 0))
    {
        count++;
			ptr++;
			lasttime = itpGetTickCount();
			while(count < len)
			{
				if (xQueueReceive(rxQueue, ptr, 0))
				{					
                	count++;
					ptr++;
				}
				else if (itpGetTickDuration(lasttime) < uartobj->timeout)
				{
					usleep(50);		
				}
				else
                	break;	
			}
		}
	}
	else
	{		
	    if (xQueueReceive(rxQueue, ptr, 0))
	    {
	        count++;
			ptr++;
        while (count < len)
        {
	            if (xQueueReceive(rxQueue, ptr, 0))
	            {
                count++;
					ptr++;
	            }
            else
                break;
        }
    }
	}	
    if (*pRxqueueFull)
        printf("rxqueue is Full\n");

    return count;

#elif defined(CFG_UART_DMA)
if (port == ITH_UART0 || port == ITH_UART1)
{
	if (!boot_mode)
	{
    uint32_t transferSize = 0;
    uint32_t dataSize = 0;

		
    transferSize = ithReadRegA(ITH_DMA_BASE + ITH_DMA_C0_TX_SIZE_REG + uartobj->readDmaChannel* ITH_DMA_CH_OFFSET);
    uartobj->gWriteIndex = (DMA_BUFFER_SIZE - transferSize);
    if (uartobj->gWriteIndex > DMA_BUFFER_SIZE)
    {
        //ithUartDisableDmaMode2(uartobj->port);
        return dataSize;
    }
    if (uartobj->gWriteIndex != uartobj->gReadIndex)
    {
        //ithPrintf("[transferSize]:0x%x [gWriteIndex]:0x%x [gReadIndex]:0x%x\n", transferSize, uartobj->gWriteIndex, uartobj->gReadIndex);
        if (uartobj->gWriteIndex < uartobj->gReadIndex)
        {
            dataSize = (DMA_BUFFER_SIZE - uartobj->gReadIndex) + uartobj->gWriteIndex;
            ithInvalidateDCacheRange(uartobj->gpDMABuffer, DMA_BUFFER_SIZE);
            memcpy(ptr, uartobj->gpDMABuffer + uartobj->gReadIndex, DMA_BUFFER_SIZE - uartobj->gReadIndex);
            memcpy(ptr + (DMA_BUFFER_SIZE - uartobj->gReadIndex), uartobj->gpDMABuffer, uartobj->gWriteIndex);
        }
        else
        {
            dataSize = uartobj->gWriteIndex - uartobj->gReadIndex;
            ithInvalidateDCacheRange(uartobj->gpDMABuffer, DMA_BUFFER_SIZE);
            memcpy(ptr, uartobj->gpDMABuffer + uartobj->gReadIndex, dataSize);
        }

        uartobj->gReadIndex = uartobj->gWriteIndex;
        //ithPrintf("dataSize:%d\n", dataSize);

    }

    //ithUartDisableDmaMode2(port);
    return dataSize;
	}
	else
		return UartReadDefault(port, ptr, len);		
}
else
{
	return UartReadDefault(port, ptr, len);
}
#else
	return UartReadDefault(port, ptr, len);
#endif // CFG_UART_INTR
}

static int UartWrite(int file, char *ptr, int len, void *info)
{
    ITHUartPort port = (ITHUartPort) info;
    int         i = 0;
    UART_OBJ *uartobj = NULL;

    if (port == ITH_UART0)
        uartobj = &Uartobj[0];
    else if (port == ITH_UART1)
        uartobj = &Uartobj[1];
    else if (port == ITH_UART2)
        uartobj = &Uartobj[2];
    else
        uartobj = &Uartobj[3];

#ifdef CFG_UART_INTR
    QueueHandle_t txQueue;
    txQueue = uartobj->xCharsForTx;

    do
    {
        /* Place the character in the queue of characters to be transmitted. */
        if (xQueueSend(txQueue, &ptr[i], 0) != pdPASS)
        {
            if (ithUartIsTxEmpty(port))
            {
                signed char cChar;
                if (xQueueReceive(txQueue, &cChar, 0) == pdTRUE)
                    ithUartPutChar(port, cChar);
            }
            ithUartEnableIntr(port, ITH_UART_TX_READY);
        }
        else
        {
            i++;
        }
    } while (i < len);

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

    return len;
#elif defined(CFG_UART_DMA)
    if (port == ITH_UART0 || port == ITH_UART1)
    {
    	if (!boot_mode)
    	{
        if(uartobj->tempTxBuf == NULL)
        {
            return 0;
        }
    
	    memcpy(uartobj->tempTxBuf, ptr,len);
        ithDmaSetSrcAddr(uartobj->writeDmaChannel, (uint32_t)uartobj->tempTxBuf);
        ithDmaSetDstAddr(uartobj->writeDmaChannel, (ITHUartPort)info);
    
        ithDmaSetRequest(uartobj->writeDmaChannel, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, uartobj->dma_tx_req);
    
        ithDmaSetSrcParams(uartobj->writeDmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
        ithDmaSetDstParams(uartobj->writeDmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
        ithDmaSetTxSize(uartobj->writeDmaChannel, len);
        ithDmaSetBurst(uartobj->writeDmaChannel, ITH_DMA_BURST_1);
        //ithUartEnableDmaMode2((ITHUartPort)info);
    
        ithDmaStart(uartobj->writeDmaChannel);
    
        while(ithDmaIsBusy(uartobj->writeDmaChannel) /*&& --timeout_ms*/)
        {
            DummySleep();
        }
    
        //ithUartDisableDmaMode2((ITHUartPort)info);
    
        return len;
    }
    else
			return UartWriteDefault(port, ptr, len);

    }
    else    
    	return UartWriteDefault(port, ptr, len);
#else
	return UartWriteDefault(port, ptr, len);

#endif // CFG_UART_INTR
}

#if defined (CFG_UART_DMA)
void UartSetboot(bool bootmode)
{
	boot_mode = bootmode;		
}
#endif
static int UartIoctl(int file, unsigned long request, void *ptr, void *info)
{
    ITHUartPort   port = (ITHUartPort) info;
    ITHUartParity Uart_Parity;
	UART_OBJ *uartobj = NULL;

    if (port == ITH_UART0)
        uartobj = &Uartobj[0];
    else if (port == ITH_UART1)
        uartobj = &Uartobj[1];
    else if (port == ITH_UART2)
        uartobj = &Uartobj[2];
    else
        uartobj = &Uartobj[3];

    switch (request)
    {
    case FIONREAD:
        // Is a character waiting?
/*
#ifdef CFG_UART_INTR
        {
            QueueHandle_t rxQueue = (port == ITH_UART0) ? xRxedChars[0] : xRxedChars[1];
            *(int *)ptr = uxQueueMessagesWaiting(rxQueue);
        }
#else*/
      //  *(int *)ptr = ithUartIsRxReady(port) ? 1 : 0;
//#endif     // CFG_UART_INTR
        break;
	case ITP_IOCTL_REG_UART_CB:
		uartobj->itpUartDeferIntrHandler = (ITPPendFunction)ptr;
		break;
	case ITP_IOCTL_REG_UART_DEFER_CB:		
		uartobj->UartDeferIntrOn = 1;
		uartobj->itpUartDeferIntrHandler = (ITPPendFunction)ptr;
		break;
	case ITP_IOCTL_UART_TIMEOUT:
		uartobj->timeout = (uint32_t)ptr;
		break;
    case ITP_IOCTL_ON: //set odd , even parity check.
        Uart_Parity = *(ITHUartParity *)ptr;
        UartOpen(port, Uart_Parity);
        break;

    case ITP_IOCTL_INIT: //set baud rate
        ithPutcharFunc = UartPutchar;
        break;

    case ITP_IOCTL_RESET:
        UartReset(port, (unsigned int)ptr);
        break;

	case ITP_IOCTL_UART_SET_BOOT:
#if defined (CFG_UART_DMA)		
		UartSetboot((bool)ptr);
#endif
		break;
		
    default:
        errno = -1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceUart0 =
{
    ":uart0",
    itpUartOpen,
    itpCloseDefault,
    UartRead,
    UartWrite,
    itpLseekDefault,
    UartIoctl,
    (void *)ITH_UART0
};

const ITPDevice itpDeviceUart1 =
{
    ":uart1",
    itpUartOpen,
    itpCloseDefault,
    UartRead,
    UartWrite,
    itpLseekDefault,
    UartIoctl,
    (void *)ITH_UART1
};

const ITPDevice itpDeviceUart2 =
{
    ":uart2",
    itpUartOpen,
    itpCloseDefault,
    UartRead,
    UartWrite,
    itpLseekDefault,
    UartIoctl,
    (void *)ITH_UART2
};

const ITPDevice itpDeviceUart3 =
{
    ":uart3",
    itpUartOpen,
    itpCloseDefault,
    UartRead,
    UartWrite,
    itpLseekDefault,
    UartIoctl,
    (void *)ITH_UART3
};
