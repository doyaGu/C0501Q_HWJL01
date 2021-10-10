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

#if defined (CFG_RS485_0_DMA) || defined (CFG_RS485_1_DMA)
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
static uint8_t *tempTxBuf = NULL;
static uint8_t *gpDMABuffer = 0;
static uint32_t gWriteIndex = 0;
static uint32_t gReadIndex = 0;
static uint32_t RS485_DMA_BUFFER_SIZE = 0;

static LLP_CONTEXT *g_LLPCtxt = NULL;
#endif

static unsigned int GPIO_RS485_TX_ENABLE[5];
static unsigned int GPIO_RS485_RX[5];
static unsigned int GPIO_RS485_TX[5];
static int RS485_SetBaudrate[5];

static ITHUartParity RS485_Parity[4],RS485_SoftUart_Parity;
static ITHUartPort RS485PutcharPort;
static pthread_mutex_t RS485InternalMutex  = PTHREAD_MUTEX_INITIALIZER;
static ITPPendFunction  itpRS485DeferIntrHandler[4];
static ITHUartPort  gUartPort;
static ITHRS485Port gRS485_Port;
static uint8_t RS485DeferIntrOn[4];


#if defined(CFG_RS485_0_INTR) || defined(CFG_RS485_1_INTR) || defined(CFG_RS485_2_INTR) || defined(CFG_RS485_3_INTR)
static unsigned int RS485_BUF_SIZE;

/* Queues used to hold received characters, and characters waiting to be
transmitted. */
static QueueHandle_t xRxedChars[5]= {NULL};
static QueueHandle_t xCharsForTx[5]= {NULL};
static int RxQueueFull[5] = {0};

static void RS485IntrHandler(void* arg)
{
    ITHUartPort port = (ITHUartPort) arg;
    uint32_t status = ithUartClearIntr(port);

	signed char cChar;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	QueueHandle_t txQueue, rxQueue;
	int *pRxqueueFull = NULL;
	long result = 0;

	if (port == ITH_UART0)
	{
	   txQueue = xCharsForTx[0];
	   rxQueue = xRxedChars[0];
	   pRxqueueFull = &RxQueueFull[0];
	}
	else if (port == ITH_UART1)
	{
	   txQueue = xCharsForTx[1];
	   rxQueue = xRxedChars[1];
	   pRxqueueFull = &RxQueueFull[1];
	}
	else if (port == ITH_UART2)
	{
	   txQueue = xCharsForTx[2];
	   rxQueue = xRxedChars[2];
	   pRxqueueFull = &RxQueueFull[2];
	}
	else
	{
	   txQueue = xCharsForTx[3];
	   rxQueue = xRxedChars[3];
	   pRxqueueFull = &RxQueueFull[3];
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
			   
				//wait Tx and Tx Fifo empty and disable TX_ENABLE gpio.
				while(!ithUartIsTxWIthFifoEmpty(port) && (ithGpioGet(GPIO_RS485_TX_ENABLE[0]) || ithGpioGet(GPIO_RS485_TX_ENABLE[1]) || ithGpioGet(GPIO_RS485_TX_ENABLE[2]) || ithGpioGet(GPIO_RS485_TX_ENABLE[3])))
				{
					//printf("TX not ready!\n");
				}
				switch(port)
				{
					case ITH_UART0:
					if(GPIO_RS485_TX_ENABLE[0])
					{
						ithGpioClear(GPIO_RS485_TX_ENABLE[0]);
						ithGpioSetMode(GPIO_RS485_TX_ENABLE[0], ITH_GPIO_MODE0);
						ithGpioSetIn(GPIO_RS485_TX_ENABLE[0]);
					}
					break;
					case ITH_UART1:
					if(GPIO_RS485_TX_ENABLE[1])
					{
						ithGpioClear(GPIO_RS485_TX_ENABLE[1]);
						ithGpioSetMode(GPIO_RS485_TX_ENABLE[1], ITH_GPIO_MODE0);
						ithGpioSetIn(GPIO_RS485_TX_ENABLE[1]);
					}
					break;
					case ITH_UART2:
					if(GPIO_RS485_TX_ENABLE[2])
					{
						ithGpioClear(GPIO_RS485_TX_ENABLE[2]);
						ithGpioSetMode(GPIO_RS485_TX_ENABLE[2], ITH_GPIO_MODE0);
						ithGpioSetIn(GPIO_RS485_TX_ENABLE[2]);
					}
					break;
					case ITH_UART3:
					if(GPIO_RS485_TX_ENABLE[3])
					{
						ithGpioClear(GPIO_RS485_TX_ENABLE[3]);
						ithGpioSetMode(GPIO_RS485_TX_ENABLE[3], ITH_GPIO_MODE0);
						ithGpioSetIn(GPIO_RS485_TX_ENABLE[3]);
					}
					break;
				}	
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
			result = xQueueSendFromISR( rxQueue, &cChar, &xHigherPriorityTaskWoken );		   
			if(result == 0)
				*pRxqueueFull = 1;
		   
			switch(port)
			{
			case ITH_UART0:
				if(RS485DeferIntrOn[0])
					itpPendFunctionCallFromISR(itpRS485DeferIntrHandler[0],NULL,NULL);
				else
					itpRS485DeferIntrHandler[0](NULL,NULL);
			break;
			case ITH_UART1:
				if(RS485DeferIntrOn[1])
					itpPendFunctionCallFromISR(itpRS485DeferIntrHandler[1],NULL,NULL);
				else
					itpRS485DeferIntrHandler[1](NULL,NULL);
			break;
			case ITH_UART2:
				if(RS485DeferIntrOn[2])
					itpPendFunctionCallFromISR(itpRS485DeferIntrHandler[2],NULL,NULL);
				else
					itpRS485DeferIntrHandler[2](NULL,NULL);
			break;
			case ITH_UART3:
				if(RS485DeferIntrOn[3])
					itpPendFunctionCallFromISR(itpRS485DeferIntrHandler[3],NULL,NULL);
				else
					itpRS485DeferIntrHandler[3](NULL,NULL);
			break;
			default:
				itpRS485DeferIntrHandler[0](NULL,NULL);
			}
	   }
	}

	/* If an event caused a task to unblock then we call "Yield from ISR" to
	   ensure that the unblocked task is the task that executes when the interrupt
	   completes if the unblocked task has a priority higher than the interrupted
	   task. */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

#elif (defined (CFG_RS485_0_DMA) || defined (CFG_RS485_1_DMA))
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
#endif // CFG_RS485_0_INTR |CFG_RS485_1_INTR| CFG_RS485_2_INTR | CFG_RS485_3_INTR 

static int RS485Putchar(int c)
{
#if defined(CFG_RS485_0_INTR) || defined(CFG_RS485_1_INTR)  || defined(CFG_RS485_2_INTR) || defined(CFG_RS485_3_INTR)
    QueueHandle_t txQueue;

	switch(RS485PutcharPort)
	{
	case ITH_UART0:
		txQueue = xCharsForTx[0];
		break;
	case ITH_UART1:
		txQueue = xCharsForTx[1];
		break;
	case ITH_UART2:
		txQueue = xCharsForTx[2];
		break;
	case ITH_UART3:
		txQueue = xCharsForTx[3];
		break;
	default:
		txQueue = xCharsForTx[0];
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

#endif // CFG_RS485_0_INTR || CFG_RS485_1_INTR  || CFG_RS485_2_INTR  || CFG_RS485_3_INTR
    return c;
}

static void RS485DefaultIntrHandler(void)
{
    // DO NOTHING
}


static void RS485Open(ITHRS485Port RS485_Port,ITHUartPort UartPort, unsigned int baud)
{
    pthread_mutex_lock(&RS485InternalMutex);

#if defined(CFG_RS485_0_ENABLE) || defined(CFG_RS485_1_ENABLE) || defined(CFG_RS485_2_ENABLE) || defined(CFG_RS485_3_ENABLE)
    if(RS485_Port != ITH_RS485_4)
    {
        /* Set the required protocol. */
        if(RS485_Parity[RS485_Port])
        {
          ithUartReset(UartPort, baud, RS485_Parity[RS485_Port], 1, 8);
        }else
        {
          ithUartReset(UartPort, baud, ITH_UART_NONE, 1, 8);
        }

        /* Enable Rx and Tx. */
        ithUartSetMode(UartPort, ITH_UART_DEFAULT, GPIO_RS485_TX[RS485_Port], GPIO_RS485_RX[RS485_Port]);
    }
#endif
    pthread_mutex_unlock(&RS485InternalMutex);
}

static void RS485Close(void)
{
    return ;
}

static void RS485Reset(ITHUartPort UartPort)
{
    unsigned int levelTx, levelRx , i;
    ITHUartFifoDepth fifoDepth;
	static int RS485_DMA_init = 0;

    pthread_mutex_lock(&RS485InternalMutex);
#if defined(CFG_RS485_0_INTR) || defined(CFG_RS485_1_INTR) || defined(CFG_RS485_2_INTR) || defined(CFG_RS485_3_INTR)
    {
        ITHIntr intr;
        if (UartPort == ITH_UART0)
        {
            intr = ITH_INTR_UART0;

			if (!xRxedChars[0] && !xCharsForTx[0])
			{
	            /* Create the queues used to hold Rx and Tx characters. */
	            xRxedChars[0] = xQueueCreate( RS485_BUF_SIZE, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
	            xCharsForTx[0] = xQueueCreate( RS485_BUF_SIZE + 1, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
			}
        }
        else if (UartPort == ITH_UART1)
        {
            intr = ITH_INTR_UART1;

			if (!xRxedChars[1] && !xCharsForTx[1])
			{
	            /* Create the queues used to hold Rx and Tx characters. */
	            xRxedChars[1] = xQueueCreate( RS485_BUF_SIZE, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
	            xCharsForTx[1] = xQueueCreate( RS485_BUF_SIZE + 1, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
			}
        }
		else if (UartPort == ITH_UART2)
		{
			intr = ITH_INTR_UART2;

			if (!xRxedChars[2] && !xCharsForTx[2])
			{
				/* Create the queues used to hold Rx and Tx characters. */
				xRxedChars[2] = xQueueCreate( RS485_BUF_SIZE, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
				xCharsForTx[2] = xQueueCreate( RS485_BUF_SIZE + 1, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
			}
		}
		else
		{
			intr = ITH_INTR_UART3;

			if (!xRxedChars[3] && !xCharsForTx[3])
			{   
				/* Create the queues used to hold Rx and Tx characters. */
				xRxedChars[3] = xQueueCreate( RS485_BUF_SIZE, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
				xCharsForTx[3] = xQueueCreate( RS485_BUF_SIZE + 1, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
			}
		}

        ithEnterCritical();

		switch(UartPort)
		{
		case ITH_UART0:
			itpRS485DeferIntrHandler[0] = RS485DefaultIntrHandler;
			RS485DeferIntrOn[0] = 0;
		break;
		case ITH_UART1:
			itpRS485DeferIntrHandler[1] = RS485DefaultIntrHandler;
			RS485DeferIntrOn[1] = 0;
		break;
		case ITH_UART2:
			itpRS485DeferIntrHandler[2] = RS485DefaultIntrHandler;
			RS485DeferIntrOn[2] = 0;
		break;
		case ITH_UART3:
			itpRS485DeferIntrHandler[3] = RS485DefaultIntrHandler;
			RS485DeferIntrOn[3] = 0;
		break;
		default:
			itpRS485DeferIntrHandler[0] = RS485DefaultIntrHandler;
			RS485DeferIntrOn[0] = 0;
		}
		
        /* Enable the Rx interrupts.  The Tx interrupts are not enabled
        		til there are characters to be transmitted. */
        ithIntrDisableIrq(intr);
        ithUartClearIntr(UartPort);
        ithIntrClearIrq(intr);
 
        ithIntrSetTriggerModeIrq(intr, ITH_INTR_LEVEL);
        ithIntrRegisterHandlerIrq(intr, RS485IntrHandler, (void*)UartPort);
        ithUartEnableIntr(UartPort, ITH_UART_RX_READY);

        /* Enable the interrupts. */
        ithIntrEnableIrq(intr);

        ithExitCritical();
    }

#elif (defined (CFG_RS485_0_DMA) || defined (CFG_RS485_1_DMA))
	if(!RS485_DMA_init)
	{
    if ((UartPort == ITH_UART0) || (UartPort == ITH_UART1))
    {
        LLP_CONTEXT *llpaddr = NULL;

	    switch (UartPort)
        {
        case ITH_UART0:
            	readDmaChannel = ithDmaRequestCh("dma_rs485_0_read", ITH_DMA_CH_PRIO_HIGH_3, NULL, NULL);
        		ithDmaReset(readDmaChannel);
				writeDmaChannel = ithDmaRequestCh("dma_rs485_0_write", ITH_DMA_CH_PRIO_HIGHEST, NULL, NULL);
		        ithDmaReset(writeDmaChannel);
            break;
        case ITH_UART1:
                readDmaChannel = ithDmaRequestCh("dma_rs485_1_read", ITH_DMA_CH_PRIO_HIGH_3, NULL, NULL);
        		ithDmaReset(readDmaChannel);
		        writeDmaChannel = ithDmaRequestCh("dma_rs485_1_write", ITH_DMA_CH_PRIO_HIGHEST, NULL, NULL);
        		ithDmaReset(writeDmaChannel);
            break;
        }

        tempTxBuf = (uint8_t *)itpVmemAlloc(RS485_DMA_BUFFER_SIZE);
        gpDMABuffer = (uint8_t *)itpVmemAlloc(RS485_DMA_BUFFER_SIZE);
        if (gpDMABuffer == NULL)
        {
            printf("Alloc DMA buffer fail\n");
        }
        else
        {
            g_LLPCtxt = (LLP_CONTEXT *)itpVmemAlloc(sizeof(LLP_CONTEXT) + 32);
            //printf("--- g_LLPCtxt addr = 0x%x , sizeof(LLP_CONTEXT) = 0x%x---\n", g_LLPCtxt, sizeof(LLP_CONTEXT));
            llpaddr = (LLP_CONTEXT *)(((uint32_t)g_LLPCtxt + 0x1F) & ~(0x1F));
            //printf("new g_LLPCtxt addr = 0x%x\n", llpaddr);
            llpaddr->SrcAddr =  le32_to_cpu(UartPort);
            llpaddr->DstAddr = le32_to_cpu(gpDMABuffer);
            llpaddr->LLP = le32_to_cpu(llpaddr);
            llpaddr->TotalSize = le32_to_cpu(RS485_DMA_BUFFER_SIZE);
            llpaddr->Control = le32_to_cpu(0x00220000);

            ithDmaSetSrcAddr(readDmaChannel, UartPort);
            ithDmaSetDstAddr(readDmaChannel, (uint32_t)gpDMABuffer);
            switch (UartPort)
            {
            case ITH_UART0:
                ithDmaSetRequest(readDmaChannel, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_UART0_RX, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
                break;
            case ITH_UART1:
                ithDmaSetRequest(readDmaChannel, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_UART1_RX, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
                break;
            }
            ithDmaSetSrcParams(readDmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
            ithDmaSetDstParams(readDmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
            ithDmaSetTxSize(readDmaChannel, RS485_DMA_BUFFER_SIZE);
            ithDmaSetBurst(readDmaChannel, ITH_DMA_BURST_1);
            ithDmaSetLLPAddr(readDmaChannel, (uint32_t)llpaddr);
            //printf("llpaddr:0x%x\n", (uint32_t)llpaddr);

            ithUartEnableDmaMode2(UartPort);
            ithDmaStart(readDmaChannel);
        }
    }
	RS485_DMA_init = 1;
	}
#endif // CFG_RS485_0_INTR || CFG_RS485_1_INTR || CFG_RS485_2_INTR || CFG_RS485_3_INTR

    if (RS485PutcharPort == 0)
        RS485PutcharPort = UartPort;
	
	usleep(1000*100); // if run roo fast, it will become sothing wrong, Benson

	if(!ithPutcharFunc)	ithPutcharFunc = RS485Putchar;
    pthread_mutex_unlock(&RS485InternalMutex);
}

static int RS485Read(int file, char *ptr, int len, void* info)
{
    ITHUartPort port = (ITHUartPort) info;
    int count = 0;
	int i;

    pthread_mutex_lock(&RS485InternalMutex);

	#if 0
    //Disable TX gpio
	switch(port)
    {
    	case ITH_UART0:
		if(GPIO_RS485_TX_ENABLE[0])
		{
	        ithGpioClear(GPIO_RS485_TX_ENABLE[0]);
	        ithGpioSetMode(GPIO_RS485_TX_ENABLE[0], ITH_GPIO_MODE0);
	        ithGpioSetIn(GPIO_RS485_TX_ENABLE[0]);
		}
		break;
		case ITH_UART1:
		if(GPIO_RS485_TX_ENABLE[1])
		{
			ithGpioClear(GPIO_RS485_TX_ENABLE[1]);
			ithGpioSetMode(GPIO_RS485_TX_ENABLE[1], ITH_GPIO_MODE0);
			ithGpioSetIn(GPIO_RS485_TX_ENABLE[1]);

		}
		break;
		case ITH_UART2:
		if(GPIO_RS485_TX_ENABLE[2])
		{
			ithGpioClear(GPIO_RS485_TX_ENABLE[2]);
	  		ithGpioSetMode(GPIO_RS485_TX_ENABLE[2], ITH_GPIO_MODE0);
	  		ithGpioSetIn(GPIO_RS485_TX_ENABLE[2]);

		}
		break;
		case ITH_UART3:
		if(GPIO_RS485_TX_ENABLE[3])
		{
			ithGpioClear(GPIO_RS485_TX_ENABLE[3]);
			ithGpioSetMode(GPIO_RS485_TX_ENABLE[3], ITH_GPIO_MODE0);
			ithGpioSetIn(GPIO_RS485_TX_ENABLE[3]);

		}
		break;
    }
	#endif

#if defined(CFG_RS485_0_INTR) || defined(CFG_RS485_1_INTR) || defined(CFG_RS485_2_INTR) || defined(CFG_RS485_3_INTR)
    QueueHandle_t rxQueue; //  = (port == ITH_UART0) ? xRxedChars[0] : xRxedChars[1];
    int* pRxqueueFull = NULL;

	if (port == ITH_UART0)
    {
		rxQueue = xRxedChars[0];
		pRxqueueFull = &RxQueueFull[0]; 
    }
	else if (port == ITH_UART1)
	{
		rxQueue = xRxedChars[1];
		pRxqueueFull = &RxQueueFull[1];
	}
	else if (port == ITH_UART2)
	{
		rxQueue = xRxedChars[2];
		pRxqueueFull = &RxQueueFull[2];
	}
	else
	{
		rxQueue = xRxedChars[3];
		pRxqueueFull = &RxQueueFull[3];
	}
	
	if (xQueueReceive(rxQueue, ptr++, 0))
	{
		count++;		
		while (count < len)
		{			
			if (xQueueReceive(rxQueue, ptr++, 0))
				count++;							
			else
				break;
		}
	}
	if (*pRxqueueFull)
		printf("rxqueue is Full\n");

pthread_mutex_unlock(&RS485InternalMutex);
return count;
	
#elif (defined (CFG_RS485_0_DMA) || defined (CFG_RS485_1_DMA))
    if (((ITHUartPort)info == ITH_UART0) || ((ITHUartPort)info == ITH_UART1))
    {
        uint32_t transferSize = 0;
        uint32_t dataSize = 0;

        transferSize = ithReadRegA(ITH_DMA_BASE + ITH_DMA_C0_TX_SIZE_REG + readDmaChannel * ITH_DMA_CH_OFFSET);
        gWriteIndex = (RS485_DMA_BUFFER_SIZE - transferSize);
		
		if (gWriteIndex > RS485_DMA_BUFFER_SIZE)
		{
	    	ithUartDisableDmaMode2(port);
			pthread_mutex_unlock(&RS485InternalMutex);
	    	return dataSize;
		}
		
        if (gWriteIndex != gReadIndex)
        {
            //printf("[transferSize]:0x%x [gWriteIndex]:0x%x [gReadIndex]:0x%x\n", transferSize, gWriteIndex, gReadIndex);
            if (gWriteIndex < gReadIndex)
            {
                dataSize = (RS485_DMA_BUFFER_SIZE - gReadIndex) + gWriteIndex;
                ithInvalidateDCacheRange(gpDMABuffer, RS485_DMA_BUFFER_SIZE);
                memcpy(ptr, gpDMABuffer + gReadIndex, RS485_DMA_BUFFER_SIZE - gReadIndex);
                memcpy(ptr + (RS485_DMA_BUFFER_SIZE - gReadIndex), gpDMABuffer, gWriteIndex);
            }
            else
            {
                dataSize = gWriteIndex - gReadIndex;
                ithInvalidateDCacheRange(gpDMABuffer, RS485_DMA_BUFFER_SIZE);
                memcpy(ptr, gpDMABuffer + gReadIndex, dataSize);
            }

            gReadIndex = gWriteIndex;
            //printf("dataSize:%d\n", dataSize);

        }

        ithUartDisableDmaMode2(port);
        pthread_mutex_unlock(&RS485InternalMutex);
        return dataSize;
    }
    else
    {
        // Is a character waiting?
        while (ithUartIsRxReady(port))
        {
            *ptr++ = ithUartGetChar(port);       // Read character from RS485
            count++;
        }
        pthread_mutex_unlock(&RS485InternalMutex);
        if (count > 0)
            return count;

        return 0;
    }
#else

    count = 0;

    // Is a character waiting?
    while (ithUartIsRxReady(port))
    {
        *ptr++ = ithUartGetChar(port);       // Read character from RS485
        count++;
    }
    pthread_mutex_unlock(&RS485InternalMutex);
    if (count > 0)
        return count;

    return 0;

#endif // CFG_RS485_0_INTR || CFG_RS485_1_INTR || CFG_RS485_2_INTR || CFG_RS485_3_INTR
}

static int RS485Write(int file, char *ptr, int len, void* info)
{
    ITHUartPort port = (ITHUartPort) info;
    int i;
	
    pthread_mutex_lock(&RS485InternalMutex);

    //Enable TX gpio
    //usleep(1);
	switch(port)
    {
    	case ITH_UART0:
		if(GPIO_RS485_TX_ENABLE[0])
		{
	        ithGpioSet(GPIO_RS485_TX_ENABLE[0]);
	        ithGpioSetMode(GPIO_RS485_TX_ENABLE[0], ITH_GPIO_MODE0);
	        ithGpioSetOut(GPIO_RS485_TX_ENABLE[0]);
		}
		break;
		case ITH_UART1:
		if(GPIO_RS485_TX_ENABLE[1])
		{
			ithGpioSet(GPIO_RS485_TX_ENABLE[1]);
	        ithGpioSetMode(GPIO_RS485_TX_ENABLE[1], ITH_GPIO_MODE0);
	        ithGpioSetOut(GPIO_RS485_TX_ENABLE[1]);
		}
		break;
		case ITH_UART2:
		if(GPIO_RS485_TX_ENABLE[2])
		{
			ithGpioSet(GPIO_RS485_TX_ENABLE[2]);
	        ithGpioSetMode(GPIO_RS485_TX_ENABLE[2], ITH_GPIO_MODE0);
	        ithGpioSetOut(GPIO_RS485_TX_ENABLE[2]);
		}
		break;
		case ITH_UART3:
		if(GPIO_RS485_TX_ENABLE[3])
		{
			ithGpioSet(GPIO_RS485_TX_ENABLE[3]);
	        ithGpioSetMode(GPIO_RS485_TX_ENABLE[3], ITH_GPIO_MODE0);
	        ithGpioSetOut(GPIO_RS485_TX_ENABLE[3]);
		}
		break;
    }

#if defined(CFG_RS485_0_INTR) || defined(CFG_RS485_1_INTR)  || defined(CFG_RS485_2_INTR) || defined(CFG_RS485_3_INTR)
    QueueHandle_t txQueue; // = (port == ITH_UART0) ? xCharsForTx[0] : xCharsForTx[1];

	switch(port)
	{
		case ITH_UART0:
			txQueue  = xCharsForTx[0];
            break;
        case ITH_UART1:
			txQueue  = xCharsForTx[1];
            break;
		case ITH_UART2:
			txQueue  = xCharsForTx[2];
            break;
        case ITH_UART3:
			txQueue  = xCharsForTx[3];
            break;
	}

    for (i = 0; i < len; i++)
    {
        /* Place the character in the queue of characters to be transmitted. */
		if (xQueueSend(txQueue, &ptr[i], portMAX_DELAY) != pdPASS)
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

	//Note. Disable TX_ENABLE Pin in ISR routine.
    pthread_mutex_unlock(&RS485InternalMutex);
    return i;

#elif (defined (CFG_RS485_0_DMA) || defined (CFG_RS485_1_DMA))
    if (((ITHUartPort)info == ITH_UART0)||((ITHUartPort)info == ITH_UART1))
    {
        uint32_t timeout_ms = 3000;
        uint32_t dstWidth = ITH_DMA_WIDTH_16;
        uint32_t srcWidth = ITH_DMA_WIDTH_16;
        ITHDmaBurst burstSize    = ITH_DMA_BURST_1;

        /* Lock */
        //ithLockMutex(ithStorMutex);

        if (tempTxBuf == NULL)
        {
        	printf("tempTxBuf =NULL!\n");
            return 0;
        }

        memcpy(tempTxBuf, ptr, len);
        ithDmaSetSrcAddr(writeDmaChannel, (uint32_t)tempTxBuf);
        ithDmaSetDstAddr(writeDmaChannel, (ITHUartPort)info);
        switch ((ITHUartPort)info)
        {
        case ITH_UART0:
            ithDmaSetRequest(writeDmaChannel, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_UART0_TX);
            break;
        case ITH_UART1:
            ithDmaSetRequest(writeDmaChannel, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_UART1_TX);
            break;
        }
        ithDmaSetSrcParams(writeDmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
        ithDmaSetDstParams(writeDmaChannel, ITH_DMA_WIDTH_8, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
        ithDmaSetTxSize(writeDmaChannel, len);
        ithDmaSetBurst(writeDmaChannel, ITH_DMA_BURST_1);
        ithUartEnableDmaMode2((ITHUartPort)info);

        ithDmaStart(writeDmaChannel);

        while (ithDmaIsBusy(writeDmaChannel) /*&& --timeout_ms*/)
        {
            DummySleep();
        }

        ithUartDisableDmaMode2((ITHUartPort)info);

        /* Unlock */
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
        pthread_mutex_unlock(&RS485InternalMutex);
        return len;
    }
#else

#ifndef _MSC_VER
#ifndef NDEBUG
    portSAVEDISABLE_INTERRUPTS();
#endif
#endif
    for (i = 0; i < len; i++)
    {
        // Is transmitter ready?
        while (ithUartIsTxFull(port));
        ithUartPutChar(port, *ptr++);
    }
#ifndef _MSC_VER	
#ifndef NDEBUG
    portRESTORE_INTERRUPTS();
#endif
#endif
    pthread_mutex_unlock(&RS485InternalMutex);

    return len;

#endif // CFG_RS485_0_INTR || CFG_RS485_1_INTR || CFG_RS485_2_INTR || CFG_RS485_3_INTR
}

static void RS485SoftUartReset(void)
{
    int swuart_parity;

    //swuart_parity = ITP_SWUART_NONE;
    swuart_parity  = RS485_SoftUart_Parity;
    ioctl(ITP_DEVICE_SWUART, ITP_IOCTL_SET_PARITY, &swuart_parity);
}

static int RS485SoftUartRead(int file, char *ptr, int len, void* info)
{
    ITHUartPort port = (ITHUartPort) info;

    //pthread_mutex_lock(&RS485InternalMutex);
    //Disable TX gpio
    if(GPIO_RS485_TX_ENABLE[4])
    {
        ithGpioClear(GPIO_RS485_TX_ENABLE[4]);
        ithGpioSetMode(GPIO_RS485_TX_ENABLE[4], ITH_GPIO_MODE0);
        ithGpioSetIn(GPIO_RS485_TX_ENABLE[4]);
    }

#if defined(CFG_DBG_SWUART_CODEC) || defined(CFG_SWUART_CODEC_ENABLE)
    return ithCodecUartRead(ptr, len);
#else
	return 0;
#endif
}

static int RS485SoftUartWrite(int file, char *ptr, int len, void* info)
{
    ITHUartPort port = (ITHUartPort) info;
    int i;

    //pthread_mutex_lock(&RS485InternalMutex);
    //Enable TX gpio
    if(GPIO_RS485_TX_ENABLE[4])
    {
        ithGpioSet(GPIO_RS485_TX_ENABLE[4]);
        ithGpioSetMode(GPIO_RS485_TX_ENABLE[4], ITH_GPIO_MODE0);
        ithGpioSetOut(GPIO_RS485_TX_ENABLE[4]);
    }

#ifndef _MSC_VER
#ifndef NDEBUG
    portSAVEDISABLE_INTERRUPTS();
#endif
#endif

#if defined(CFG_DBG_SWUART_CODEC) || defined(CFG_SWUART_CODEC_ENABLE)
    //for RS485_1
    ithCodecUartWrite(ptr, len);
#endif

#ifndef _MSC_VER
#ifndef NDEBUG
    portRESTORE_INTERRUPTS();
#endif
#endif
    //pthread_mutex_unlock(&RS485InternalMutex);

    return len;

}

static int RS485Ioctl(int file, unsigned long request, void* ptr, void* info)
{
    int i;
	ITHRS485Port RS485_Port;
    gUartPort = (ITHUartPort) info;

    switch (request)
    {
    case FIONREAD:
        // Is a character waiting?
        break;

	case ITP_IOCTL_REG_RS485_DEFER_CB:
		switch(gRS485_Port)
		{
		case ITH_RS485_0:
			RS485DeferIntrOn[0] = 1;
			break;
		case ITH_RS485_1:
			RS485DeferIntrOn[1] = 1;
			break;
		case ITH_RS485_2:
			RS485DeferIntrOn[2] = 1;
			break;
		case ITH_RS485_3:
			RS485DeferIntrOn[3] = 1;
			break;
		default:
			RS485DeferIntrOn[0] = 1;
		}
    case ITP_IOCTL_REG_RS485_CB:
		switch(gRS485_Port)
		{
		case ITH_RS485_0:
			itpRS485DeferIntrHandler[0] = (ITPPendFunction)ptr;
			break;
		case ITH_RS485_1:
			itpRS485DeferIntrHandler[1] = (ITPPendFunction)ptr;
			break;
		case ITH_RS485_2:
			itpRS485DeferIntrHandler[2] = (ITPPendFunction)ptr;
			break;
		case ITH_RS485_3:
			itpRS485DeferIntrHandler[3] = (ITPPendFunction)ptr;
			break;
		default:
			itpRS485DeferIntrHandler[0] = (ITPPendFunction)ptr;
		}
		break;
	
    case ITP_IOCTL_INIT:
        //ithPutcharFunc = RS485Putchar;
        switch(gUartPort)
		{
			case ITH_UART0:
				RS485_Parity[0] = *(ITHUartParity*)ptr;
			break;
			case ITH_UART1:
				RS485_Parity[1] = *(ITHUartParity*)ptr;
			break;
			case ITH_UART2:
				RS485_Parity[2] = *(ITHUartParity*)ptr;
			break;
			case ITH_UART3:
				RS485_Parity[3] = *(ITHUartParity*)ptr;
			break;
		}
#if defined(CFG_RS485_4_ENABLE)
		RS485_SoftUart_Parity = *(ITHUartParity*)ptr;
#endif
        break;

    case ITP_IOCTL_ON:
        gRS485_Port = RS485_Port = *(ITHRS485Port*)ptr;
        switch(RS485_Port)
        {
	        case ITH_RS485_0:
	        {
				
	            if(GPIO_RS485_TX_ENABLE[0])
	            {
	                ithGpioClear(GPIO_RS485_TX_ENABLE[0]);
	                ithGpioSetMode(GPIO_RS485_TX_ENABLE[0], ITH_GPIO_MODE0);
	                ithGpioSetIn(GPIO_RS485_TX_ENABLE[0]);
	            }
#if defined(CFG_RS485_0_ENABLE)
	            GPIO_RS485_TX_ENABLE[0] = CFG_GPIO_RS485_0_TX_ENABLE;
	            GPIO_RS485_RX[0] = CFG_GPIO_RS485_0_RX;
	            GPIO_RS485_TX[0] = CFG_GPIO_RS485_0_TX;
				
	            ithGpioClear(GPIO_RS485_RX[0]);
	            ithGpioSetMode(GPIO_RS485_RX[0], ITH_GPIO_MODE0);
	            ithGpioSetIn(GPIO_RS485_RX[0]);
	            ithGpioClear(GPIO_RS485_TX[0]);
	            ithGpioSetMode(GPIO_RS485_TX[0], ITH_GPIO_MODE0);
	            ithGpioSetIn(GPIO_RS485_TX[0]);

#if defined(CFG_RS485_0_INTR)
	            RS485_BUF_SIZE = CFG_RS485_0_BUF_SIZE;
#elif defined(CFG_RS485_0_DMA)
				RS485_DMA_BUFFER_SIZE  = CFG_RS485_0_DMA_BUF_SIZE;
#endif
	            RS485Open(RS485_Port ,gUartPort, CFG_RS485_0_BAUDRATE);
#endif
	            break;
	        }
			case ITH_RS485_1:
	        {
				
	            if(GPIO_RS485_TX_ENABLE[1])
	            {
	                ithGpioClear(GPIO_RS485_TX_ENABLE[1]);
	                ithGpioSetMode(GPIO_RS485_TX_ENABLE[1], ITH_GPIO_MODE0);
	                ithGpioSetIn(GPIO_RS485_TX_ENABLE[1]);
	            }
#if defined(CFG_RS485_1_ENABLE)
	            GPIO_RS485_TX_ENABLE[1] = CFG_GPIO_RS485_1_TX_ENABLE;
	            GPIO_RS485_RX[1] = CFG_GPIO_RS485_1_RX;
	            GPIO_RS485_TX[1] = CFG_GPIO_RS485_1_TX;

	            ithGpioClear(GPIO_RS485_RX[1]);
	            ithGpioSetMode(GPIO_RS485_RX[1], ITH_GPIO_MODE0);
	            ithGpioSetIn(GPIO_RS485_RX[1]);
	            ithGpioClear(GPIO_RS485_TX[1]);
	            ithGpioSetMode(GPIO_RS485_TX[1], ITH_GPIO_MODE0);
	            ithGpioSetIn(GPIO_RS485_TX[1]);

#if defined(CFG_RS485_1_INTR)
	            RS485_BUF_SIZE = CFG_RS485_1_BUF_SIZE;
#elif defined(CFG_RS485_1_DMA)
				RS485_DMA_BUFFER_SIZE  = CFG_RS485_1_DMA_BUF_SIZE;
#endif
	            RS485Open(RS485_Port ,gUartPort, CFG_RS485_1_BAUDRATE);
#endif
	            break;
	        }
			case ITH_RS485_2:
	        {
				
	            if(GPIO_RS485_TX_ENABLE[2])
	            {
	                ithGpioClear(GPIO_RS485_TX_ENABLE[2]);
	                ithGpioSetMode(GPIO_RS485_TX_ENABLE[2], ITH_GPIO_MODE0);
	                ithGpioSetIn(GPIO_RS485_TX_ENABLE[2]);
	            }
#if defined(CFG_RS485_2_ENABLE)
	            GPIO_RS485_TX_ENABLE[2] = CFG_GPIO_RS485_2_TX_ENABLE;
	            GPIO_RS485_RX[2] = CFG_GPIO_RS485_2_RX;
	            GPIO_RS485_TX[2] = CFG_GPIO_RS485_2_TX;

	            ithGpioClear(GPIO_RS485_RX[2]);
	            ithGpioSetMode(GPIO_RS485_RX[2], ITH_GPIO_MODE0);
	            ithGpioSetIn(GPIO_RS485_RX[2]);
	            ithGpioClear(GPIO_RS485_TX[2]);
	            ithGpioSetMode(GPIO_RS485_TX[2], ITH_GPIO_MODE0);
	            ithGpioSetIn(GPIO_RS485_TX[2]);

#if defined(CFG_RS485_2_INTR)
	            RS485_BUF_SIZE = CFG_RS485_2_BUF_SIZE;
#endif
	            RS485Open(RS485_Port,gUartPort, CFG_RS485_2_BAUDRATE);
#endif
	            break;
	        }
			case ITH_RS485_3:
	        {
				
	            if(GPIO_RS485_TX_ENABLE[3])
	            {
	                ithGpioClear(GPIO_RS485_TX_ENABLE[3]);
	                ithGpioSetMode(GPIO_RS485_TX_ENABLE[3], ITH_GPIO_MODE0);
	                ithGpioSetIn(GPIO_RS485_TX_ENABLE[3]);
	            }
#if defined(CFG_RS485_3_ENABLE)
	            GPIO_RS485_TX_ENABLE[3] = CFG_GPIO_RS485_3_TX_ENABLE;
	            GPIO_RS485_RX[3] = CFG_GPIO_RS485_3_RX;
	            GPIO_RS485_TX[3] = CFG_GPIO_RS485_3_TX;

	            ithGpioClear(GPIO_RS485_RX[3]);
	            ithGpioSetMode(GPIO_RS485_RX[3], ITH_GPIO_MODE0);
	            ithGpioSetIn(GPIO_RS485_RX[3]);
	            ithGpioClear(GPIO_RS485_TX[3]);
	            ithGpioSetMode(GPIO_RS485_TX[3], ITH_GPIO_MODE0);
	            ithGpioSetIn(GPIO_RS485_TX[3]);

#if defined(CFG_RS485_3_INTR)
	            RS485_BUF_SIZE		   = CFG_RS485_3_BUF_SIZE;
#endif
	            RS485Open(RS485_Port ,UartPort, CFG_RS485_3_BAUDRATE);
#endif
	            break;
	        }
	        case ITH_RS485_4:
	        {
	            if(GPIO_RS485_TX_ENABLE[4])
	            {
	                ithGpioClear(GPIO_RS485_TX_ENABLE[4]);
	                ithGpioSetMode(GPIO_RS485_TX_ENABLE[4], ITH_GPIO_MODE0);
	                ithGpioSetIn(GPIO_RS485_TX_ENABLE[4]);
	            }
#if defined(CFG_RS485_4_ENABLE)
	            GPIO_RS485_TX_ENABLE[4] = CFG_GPIO_RS485_4_TX_ENABLE;
	            //GPIO_RS485_RX[4] = CFG_GPIO_RS485_4_RX;
	            //GPIO_RS485_TX[4] = CFG_GPIO_RS485_4_TX;
	            RS485Open(RS485_Port ,gUartPort, CFG_RS485_4_BAUDRATE);
#endif
	            break;
	        }

        }
        break;

    case ITP_IOCTL_OFF:
        RS485Close();
        break;
	case ITP_IOCTL_SET_BAUDRATE:
		switch(gUartPort)
		{
			case ITH_UART0:
				RS485_SetBaudrate[0] = *(int*)ptr;
			break;
			case ITH_UART1:
				RS485_SetBaudrate[1] = *(int*)ptr;
			break;
			case ITH_UART2:
				RS485_SetBaudrate[2] = *(int*)ptr;
			break;
			case ITH_UART3:
				RS485_SetBaudrate[3] = *(int*)ptr;
			break;
		}
		ithUartSetBaudRate(gUartPort, RS485_SetBaudrate[gRS485_Port]);
		break;

	case ITP_IOCTL_SET_PARITY:
		switch(gUartPort)
		{
			case ITH_UART0:
				RS485_Parity[0] = *(ITHUartParity*)ptr;
			break;
			case ITH_UART1:
				RS485_Parity[1] = *(ITHUartParity*)ptr;
			break;
			case ITH_UART2:
				RS485_Parity[2] = *(ITHUartParity*)ptr;
			break;
			case ITH_UART3:
				RS485_Parity[3] = *(ITHUartParity*)ptr;
			break;
		}
#if defined(CFG_RS485_4_ENABLE)
		RS485_SoftUart_Parity = *(ITHUartParity*)ptr;
#endif
		ithUartSetParity(gUartPort, RS485_Parity[gRS485_Port], 1, 8);
		break;

    case ITP_IOCTL_RESET:
#if defined(CFG_RS485_0_ENABLE) || defined(CFG_RS485_1_ENABLE) || defined(CFG_RS485_2_ENABLE) || defined(CFG_RS485_3_ENABLE)
        RS485Reset(gUartPort);
#endif
#if defined(CFG_RS485_4_ENABLE)
        RS485SoftUartReset();
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
    (void*)ITH_UART0
};

const ITPDevice itpDeviceRS485_1 =
{
    ":rs485_1",
    itpOpenDefault,
    itpCloseDefault,
    RS485Read,
    RS485Write,
    itpLseekDefault,
    RS485Ioctl,
    (void*)ITH_UART1
};

const ITPDevice itpDeviceRS485_2 =
{
    ":rs485_2",
    itpOpenDefault,
    itpCloseDefault,
    RS485Read,
    RS485Write,
    itpLseekDefault,
    RS485Ioctl,
    (void*)ITH_UART2
};

const ITPDevice itpDeviceRS485_3 =
{
    ":rs485_3",
    itpOpenDefault,
    itpCloseDefault,
    RS485Read,
    RS485Write,
    itpLseekDefault,
    RS485Ioctl,
    (void*)ITH_UART3
};


const ITPDevice itpDeviceRS485_4 =
{
    ":rs485_4",
    itpOpenDefault,
    itpCloseDefault,
    RS485SoftUartRead,
    RS485SoftUartWrite,
    itpLseekDefault,
    RS485Ioctl,
    (void*)ITH_UART1
};



