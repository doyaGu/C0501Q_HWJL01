#include "ith_risc.h"

static int uartDelayTicks;
static int uart_tx_port;
static int uart_tx_dport;
static int gpio_sel_port;
static int uart_tx_sel_msk;
static int uart_tx_sel;
static int uart_tx_pin;
static int uart_tx_ddr;
static int dport_active_high;

static int uartDelayDbgTicks;
static int uart_tx_dbg_port;
static int uart_tx_dbg_dport;
static int gpio_sel_dbg_port;
static int uart_tx_dbg_sel_msk;
static int uart_tx_dbg_sel;
static int uart_tx_dbg_pin;
static int uart_tx_dbg_ddr;
static int dport_active_high_dbg;

enum UartParity
{
	NONE,
	ODD,
	EVEN,
};

static int gUartParity = NONE;

// GPIO Mapping Table
#define UART_TX_PORT            uart_tx_port
#define UART_TX_DPORT           uart_tx_dport
#define GPIO_SEL_PORT           gpio_sel_port
#define UART_TX_SEL_MSK         uart_tx_sel_msk
#define UART_TX_SEL             uart_tx_sel
#define UART_TX_PIN             uart_tx_pin
#define UART_TX_DDR             uart_tx_ddr
#define DPORT_ACTIVE_HIGH       dport_active_high

#define GPIO_INIT()             ithWriteRegMaskA(GPIO_SEL_PORT, UART_TX_SEL, UART_TX_SEL_MSK)
#define SET_TX_PIN()            ithWriteRegA(UART_TX_PORT, UART_TX_PIN)
#define CLEAR_TX_PIN()          ithWriteRegA((UART_TX_PORT + 0x4), UART_TX_PIN)
								
#define ENABLE_TX_PIN()         if (DPORT_ACTIVE_HIGH == 1) { ithWriteRegMaskA(UART_TX_DPORT, UART_TX_DDR, UART_TX_DDR); } \
                                else { ithWriteRegMaskA(UART_TX_DPORT, 0, UART_TX_DDR); }

                                
static int gUartDbgParity = NONE;

// GPIO Mapping Table
#define UART_TX_DBG_PORT            uart_tx_dbg_port
#define UART_TX_DBG_DPORT           uart_tx_dbg_dport
#define GPIO_SEL_DBG_PORT           gpio_sel_dbg_port
#define UART_TX_DBG_SEL_MSK         uart_tx_dbg_sel_msk
#define UART_TX_DBG_SEL             uart_tx_dbg_sel
#define UART_TX_DBG_PIN             uart_tx_dbg_pin
#define UART_TX_DBG_DDR             uart_tx_dbg_ddr
#define DPORT_ACTIVE_HIGH_DBG       dport_active_high_dbg

#define GPIO_DBG_INIT()             ithWriteRegMaskA(GPIO_SEL_DBG_PORT, UART_TX_DBG_SEL, UART_TX_DBG_SEL_MSK)
#define SET_TX_DBG_PIN()            ithWriteRegA(UART_TX_DBG_PORT, UART_TX_DBG_PIN)
#define CLEAR_TX_DBG_PIN()          ithWriteRegA((UART_TX_DBG_PORT + 0x4), UART_TX_DBG_PIN)

#define ENABLE_TX_DBG_PIN()         if (DPORT_ACTIVE_HIGH_DBG == 1) { ithWriteRegMaskA(UART_TX_DBG_DPORT, UART_TX_DBG_DDR, UART_TX_DBG_DDR); } \
                                else { ithWriteRegMaskA(UART_TX_DBG_DPORT, 0, UART_TX_DBG_DDR); }                                    

static int uart_rx_port;
static int uart_rx_dport;
static int gpiorx_sel_port;
static int uart_rx_sel_msk;
static int uart_rx_sel;
static int uart_rx_pin;
static int uart_rx_ddr;
static int dportrx_active_high;

static int rx_gpio_pin;
//static int uartDelayTicks;
static int rxsamplerate;

#define UART_BUFFER_SIZE 256
static unsigned char gpTxBuffer[UART_BUFFER_SIZE] = { 0 };
static unsigned char gpRxBuffer[UART_BUFFER_SIZE] = { 0 };

static int gTxReadIndex = 0;
static int gTxWriteIndex = 0;
static int gRxReadIndex = 0;
static int gRxWriteIndex = 0;

static int gTxState = 0;
static int gTriggerTick = 0;

#define DBG_UART_BUFFER_SIZE 256
static unsigned char gpDbgTxBuffer[DBG_UART_BUFFER_SIZE] = { 0 };
static unsigned char gpDbgRxBuffer[DBG_UART_BUFFER_SIZE] = { 0 };

static int gDbgTxReadIndex = 0;
static int gDbgTxWriteIndex = 0;
static int gDbgRxReadIndex = 0;
static int gDbgRxWriteIndex = 0;

static int gDbgTxState = 0;
static int gDbgTriggerTick = 0;

#define UART_RX_PORT            uart_rx_port
#define UART_RX_DPORT           uart_rx_dport
#define GPIO_RX_SEL_PORT        gpiorx_sel_port
#define UART_RX_SEL_MSK         uart_rx_sel_msk
#define UART_RX_SEL             uart_rx_sel
#define UART_RX_PIN             uart_rx_pin
#define UART_RX_DDR             uart_rx_ddr
#define DPORTRX_ACTIVE_HIGH     dportrx_active_high


#define GPIO_RX_INIT()          ithWriteRegMaskA(GPIO_RX_SEL_PORT, UART_RX_SEL, UART_RX_SEL_MSK)
#define SET_RX_PIN()            ithWriteRegMaskA(UART_RX_PORT, UART_RX_PIN, UART_RX_PIN)
#define CLEAR_RX_PIN()          ithWriteRegMaskA(UART_RX_PORT, 0, UART_RX_PIN)
#define ENABLE_RX_PIN()         if (DPORTRX_ACTIVE_HIGH == 1) { ithWriteRegMaskA(UART_RX_DPORT, UART_RX_DDR, UART_RX_DDR); } \
                                else { ithWriteRegMaskA(UART_RX_DPORT, 0, UART_RX_DDR); }  

void SwUartSetParity(enum UartParity parity)
{
	gUartParity = parity;
}

int SwUartSetWrite(char *ptr, int len)
{
    int i =0;
    if (len < UART_BUFFER_SIZE)
    {
		for (i = 0; i < len; i++)
		{			
			gpTxBuffer[gTxWriteIndex] = ptr[i];
			if (++gTxWriteIndex >= UART_BUFFER_SIZE)
			{
				gTxWriteIndex = 0;
			}
		}
#if 0
    	int remainSize = len;
        //memcpy(gpTxBuffer, ptr, len);
        while (remainSize > 0)
        {
        	gpTxBuffer[gTxWriteIndex++] = ptr[i++];
			//gpTxBuffer[gTxWriteIndex] = ptr[i];
			//gTxWriteIndex = 1;
			//gTxReadIndex = 0;
			if (gTxWriteIndex >= UART_BUFFER_SIZE)
			{
				gTxWriteIndex = 0;
			}
			remainSize--;
        }
#endif
        //gTxState = 0;
    }
}

int SwUartSetRead(char *ptr, int len)
{
    int i =0;
    if (len < UART_BUFFER_SIZE)
    {
    	int remainSize = len;
        //memcpy(gpTxBuffer, ptr, len);
        while (remainSize > 0)
        {
        	gpRxBuffer[gRxWriteIndex++] = ptr[i++];
			if (gRxWriteIndex >= UART_BUFFER_SIZE)
			{
				gRxWriteIndex = 0;
			}
			remainSize--;
        }
    }
}

int SwUartBufferCopy(uint8_t* pbuf, int bufsize)
{
	int len = 0;
	int remainsize = 0;
	int cpySize = 0;
	int tailSize = 0;
	if (gRxWriteIndex == gRxReadIndex)
		return 0;
	else
	{
		if (gRxWriteIndex > gRxReadIndex)
			remainsize = gRxWriteIndex - gRxReadIndex;
		else
			remainsize = UART_BUFFER_SIZE - gRxReadIndex + gRxWriteIndex;  

		if (bufsize <= remainsize)
		{
			cpySize = bufsize;
		}
		else
		{
			cpySize = remainsize;
		}

		tailSize = UART_BUFFER_SIZE - gRxReadIndex;
		if (cpySize <= tailSize)
		{
			memcpy(pbuf, &gpRxBuffer[gRxReadIndex], cpySize);
		}
		else
		{
			memcpy(pbuf, &gpRxBuffer[gRxReadIndex], tailSize);
			memcpy(&pbuf[tailSize], &gpRxBuffer[0], cpySize - tailSize);
		}

		gRxReadIndex += cpySize;
		if (gRxReadIndex >= UART_BUFFER_SIZE)
		{
			gRxReadIndex -= UART_BUFFER_SIZE;
		}
		return cpySize;
	}	
}	

void SwUartSend(void)
{
    unsigned char data = 0;
    int curTime = 0;
    register TickDuration = uartDelayTicks;
    static int sendCount = 0;
    static int num = 0;
    if (gTxReadIndex == gTxWriteIndex)
        return;

    switch (gTxState)
    {
        case 0:
    	    CLEAR_TX_PIN();        
            reset_wiegand_timer(1);
            start_wiegand_timer(1);
            gTriggerTick = TickDuration;
            gTxState++;
            break;
        case 10:
            if ((curTime = get_wiegand_timer(1)) >= gTriggerTick)
            {
                SET_TX_PIN();
                gTriggerTick += TickDuration;
                gTxState++;
            }        
            break;
        case 9:
        	if ((curTime = get_wiegand_timer(1)) >= gTriggerTick)
        	{
				switch (gUartParity)
				{			
				case EVEN:
				if (num%2)
					SET_TX_PIN();
				else
					CLEAR_TX_PIN();
				break;			
				case ODD:
				if (num%2)
					CLEAR_TX_PIN();
				else
					SET_TX_PIN();
				break;		
				case NONE:
				default:
				SET_TX_PIN();
				gTxState++;			
				break;
				}

				gTriggerTick += TickDuration;
                gTxState++;	
										
        	}
        	break;
        case 11:
            if (get_wiegand_timer(1) >= gTriggerTick)
            {
                gTxReadIndex++;
				if (gTxReadIndex >= UART_BUFFER_SIZE)
				{
					gTxReadIndex = 0;
				}
                gTxState = 0;
                num = 0;
            }			
            break;
        default:
            if ((curTime = get_wiegand_timer(1)) >= gTriggerTick)
            {
                data = gpTxBuffer[gTxReadIndex];				
                if (data & (0x1 << (gTxState - 1)))
                {
                    SET_TX_PIN();
                    num++;
                }
                else
                {
                    CLEAR_TX_PIN();				
                }
                gTriggerTick += TickDuration;
                gTxState++;
            }
            break;
    }
}

void SwUartDbgSetParity(enum UartParity parity)
{
	gUartDbgParity = parity;
}

int SwUartDbgSetWrite(char *ptr, int len)
{
    int i =0;
    if (len < DBG_UART_BUFFER_SIZE)
    {
		for (i = 0; i < len; i++)
		{
			gpDbgTxBuffer[gDbgTxWriteIndex] = ptr[i];
			if (++gDbgTxWriteIndex >= DBG_UART_BUFFER_SIZE)
			{
				gDbgTxWriteIndex = 0;
			}
		}
    }
}

void SwUartDbgSend(void)
{
    unsigned char data = 0;
    int curTime = 0;
    register TickDuration = uartDelayDbgTicks;
    static int sendCount = 0;
    static int num = 0;
    if (gDbgTxReadIndex == gDbgTxWriteIndex)
        return;

    switch (gDbgTxState)
    {
        case 0:
    	    CLEAR_TX_DBG_PIN();        
            reset_wiegand_timer(2);
            start_wiegand_timer(2);
            gDbgTriggerTick = TickDuration;
            gDbgTxState++;
            break;            
        case 10:
            if ((curTime = get_wiegand_timer(2)) >= gDbgTriggerTick)
            {
                SET_TX_DBG_PIN();
                gDbgTriggerTick += TickDuration;
                gDbgTxState++;
            }        
            break;
        case 9:
    	if ((curTime = get_wiegand_timer(2)) >= gDbgTriggerTick)
    	{
    		switch (gUartDbgParity)
    		{			
			case EVEN:
			if (num%2)
				SET_TX_DBG_PIN();
			else
				CLEAR_TX_DBG_PIN();
			break;			
			case ODD:
			if (num%2)
				CLEAR_TX_DBG_PIN();
			else
				SET_TX_DBG_PIN();
			break;		
			case NONE:
			default:
			SET_TX_DBG_PIN();
			gDbgTxState++;			
			break;
    		}
			
			gDbgTriggerTick += TickDuration;
            gDbgTxState++;	
									
    	}
    	break;    
        case 11:
            if (get_wiegand_timer(2) >= gDbgTriggerTick)
            {                            
                gDbgTxReadIndex++;
				if (gDbgTxReadIndex >= DBG_UART_BUFFER_SIZE)
				{
					gDbgTxReadIndex = 0;
				}
                gDbgTxState = 0;
                num = 0;
            }
            break;
        default:
            if ((curTime = get_wiegand_timer(2)) >= gDbgTriggerTick)
            {
                data = gpDbgTxBuffer[gDbgTxReadIndex];
                if (data & (0x1 << (gDbgTxState - 1)))
                {
                    SET_TX_DBG_PIN();
                    num++;
                }
                else
                {
                    CLEAR_TX_DBG_PIN();
                }
                gDbgTriggerTick += TickDuration;
                gDbgTxState++;
            }
            break;
    }
}

int SwUartWrite(int file, char *ptr, int len, void* info)
{
    int i,j;

    for(j=0; j<len; j++)
    {
	    int k = 0;
	    char uartTxData;
	    uartTxData = ptr[j];
	     // timer start
	    UART_TIMER_START();

	    // start bit
	    CLEAR_TX_PIN();
	    UART_DELAY(k+=uartDelayTicks);

	    // 8-bits data
	    for(i=0; i<8; i++) 
	    {
	        if ((uartTxData & (0x1 << i)))
	        {
	            SET_TX_PIN();
	        }
	        else
	        {
	            CLEAR_TX_PIN();
	        }
	        //uartTxData = uartTxData >> 1;
	        UART_DELAY(k+=uartDelayTicks);
	    }

	    // stop bit
	    SET_TX_PIN();
	    UART_DELAY(k+=uartDelayTicks);

	    // timer end
	    UART_TIMER_END();
    }

    return len;
}

void SwUartInit(unsigned int baud, unsigned int gpiopin)
{
    int hz;
    // calculate DelayTicks
    hz = PalGetSysClock(); // Get system clock
    uartDelayTicks = (hz/baud) - 1;

    uart_tx_port      = (gpiopin < 32 ? 0xDE00000C : 0xDE00004C);               // GPIO data output register    
    uart_tx_dport     = (gpiopin < 32 ? 0xDE000008 : 0xDE000048);               // GPIO direction register
    gpio_sel_port     = (0xDE000090+(gpiopin/16)*4);               // GPIO select register
    uart_tx_sel_msk   = (3 << ((gpiopin%16) * 2 )); // GPIO(n) Select Pins
    uart_tx_sel       = (0 << ((gpiopin%16) * 2 )); // GPIO(n) Select Pins
    uart_tx_pin       = (1 << (gpiopin % 32));        // GPIO(n) Output Pins
    uart_tx_ddr       = (1 << (gpiopin % 32));        // GPIO(n) Output enable Pins of I/O direction
    dport_active_high =  1;                        // Set 0 as output port

    //ithWriteRegMaskA(0xDE000000+0xAC, (0x3 << 4), (0x3 << 4));
    
    GPIO_INIT();
    ENABLE_TX_PIN();
    SET_TX_PIN();
}

void SwUartDbgInit(unsigned int baud, unsigned int gpiopin)
{
    int hz;
    // calculate DelayTicks
    hz = PalGetSysClock(); // Get system clock
    uartDelayDbgTicks = (hz/baud) - 1;

    uart_tx_dbg_port      = (gpiopin < 32 ? 0xDE00000C : 0xDE00004C);               // GPIO data output register    
    uart_tx_dbg_dport     = (gpiopin < 32 ? 0xDE000008 : 0xDE000048);               // GPIO direction register
    gpio_sel_dbg_port     = (0xDE000090+(gpiopin/16)*4);               // GPIO select register
    uart_tx_dbg_sel_msk   = (3 << ((gpiopin%16) * 2 )); // GPIO(n) Select Pins
    uart_tx_dbg_sel       = (0 << ((gpiopin%16) * 2 )); // GPIO(n) Select Pins
    uart_tx_dbg_pin       = (1 << (gpiopin % 32));        // GPIO(n) Output Pins
    uart_tx_dbg_ddr       = (1 << (gpiopin % 32));        // GPIO(n) Output enable Pins of I/O direction
    dport_active_high_dbg =  1;                        // Set 0 as output port

    //ithWriteRegMaskA(0xDE000000+0xAC, (0x3 << 4), (0x3 << 4));    

    GPIO_DBG_INIT();
    ENABLE_TX_DBG_PIN();
    SET_TX_DBG_PIN();   
}
int SwUartRead(unsigned char* pData) 
{    
    static unsigned char uartRxData=0;    
    static int t1;
    static int start_timer = 0;    
    static int i=0, j=0;
    static int current, prev = 0;
    static int k[9] = {0};
    static int index = 1, index1 = 1;
    register TickDuration = uartDelayTicks;
#if 0
    int i,k=0;
    int j[8],p;
    MMIO_Write(0x16A0, 0x1234);
    if (ithGpioGet(rx_gpio_pin) == 0)
    {         
        UART_TIMER_START();
        for (i=0; i<8; i++)
        { 
            UART_DELAY(k+=uartDelayTicks);
            p = ithGpioGet(rx_gpio_pin) >> rx_gpio_pin;
            uartRxData = (uartRxData|(p << i));
            //uartRxData |= (p << i);
            //j[i] = p;            
        }
        UART_TIMER_END();
        //printf("uartRxData = %d\n", uartRxData);
        //for (i=0; i<8; i++)
        //    printf("j[%d] = %d\n", i, j[i]);
        return uartRxData;
    }
    return uartRxData;

#else
    static int processdata = 0;
    int p;
    current = ithGpioGet(rx_gpio_pin);
    if (current == 0 && prev != current && processdata == 0)
    {
        //UART_TIMER_START();
        reset_wiegand_timer(0);
        start_wiegand_timer(0);        
        t1 = TickDuration*1.5;        
        uartRxData = 0;
        processdata = 1;
        //MMIO_Write(0x16A0, index++);
    }
    else
    {  
        int curTime = get_wiegand_timer(0);
        char* string;
        if (curTime >= t1 && processdata)
        {            
            p = current >> rx_gpio_pin;
            if (i<8)
                uartRxData |= (p << i);
            k[i] = p;
            ++i;            
            t1 += (TickDuration - (curTime - t1));
            
            processdata = 1;
  
            //MMIO_Write(0x16A4, index1++);
            if (i == 8)
            {                
                i=0;
                //MMIO_Write(0x16A2, uartRxData);
                //MMIO_Write(0x16A2, k[1]);
                //MMIO_Write(0x16A4, k[2]);
                //MMIO_Write(0x16A6, k[3]);
                //MMIO_Write(0x16A8, k[4]);
                //MMIO_Write(0x16AA, k[5]);
                //MMIO_Write(0x16AC, k[6]);
                //MMIO_Write(0x16AE, k[7]);           
                processdata = 0;                 
                prev = current;                
                *pData = uartRxData;
                return 1;
            }          
        }        
    }
    prev = current;    
    return 0;
#endif
}

void SwUartRxInit(unsigned int baud, unsigned int gpiopin)
{
    int hz;
    hz = PalGetSysClock();    
    uartDelayTicks = (hz/baud) -1;

    rx_gpio_pin         = gpiopin;

    uart_rx_port        = (rx_gpio_pin < 32 ? 0xDE000004 : 0xDE000044); //GPIO data input resister
    uart_rx_dport       = (rx_gpio_pin < 32 ? 0xDE000008 : 0xDE000048); //GPIO direction resister
    gpiorx_sel_port     = (0xDE000090+(rx_gpio_pin/16)*4);              // GPIO select register    
    uart_rx_sel_msk     = (3 << ((rx_gpio_pin%16) * 2 )); // GPIO(n) Select Pins
    uart_rx_sel         = (0 << ((rx_gpio_pin%16) * 2 )); // GPIO(n) Select Pins
    uart_rx_pin         = (1 << (rx_gpio_pin % 32));        // GPIO(n) Output Pins
    uart_rx_ddr         = (1 << (rx_gpio_pin % 32));        // GPIO(n) Output enable Pins of I/O direction
    dportrx_active_high = 0;                             // Set 0 as output port

    GPIO_RX_INIT();
    ENABLE_RX_PIN();

    
}

