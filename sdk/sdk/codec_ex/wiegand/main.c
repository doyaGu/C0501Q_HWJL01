#include <string.h>
#include "mmio.h"
#include "ith_risc.h"
#include "spr_defs.h"

///////////////////////////////////////////////////////////////////////////
//                              Constant Definition
///////////////////////////////////////////////////////////////////////////
#define RISC_COMMUNICATE_REG      0x16A8

#define ENDIAN_SWAP32(x) \
        (((x) & 0x000000FF) << 24) | \
        (((x) & 0x0000FF00) <<  8) | \
        (((x) & 0x00FF0000) >>  8) | \
        (((x) & 0xFF000000) >> 24)

#define COMMAND_BUFFER_SIZE     16
#define WIEGAND_BUFFER_SIZE     32
#define PRINTF_BUFFER_SIZE      320
#define UART_TX_BUFFER_SIZE		320
#define UART_RX_BUFFER_SIZE     320
#define UART_TX_DBG_BUFFER_SIZE 320
#define CTRLBOARD_BUFFER_SIZE   64
#define HEARTBEAT_BUFFER_SIZE   1640//840//640//256

extern char risc2_start_addr;
#define CODEC_CONTEXT_ADDRESS &risc2_start_addr+0x14


typedef struct CODEC_CONTEXT_TAG
{
    int*  commandBuffer;
    unsigned int    commandBufferLength;
    unsigned char*  wiegandBuffer;
    unsigned int    wiegandBufferLength;
    unsigned char*  printfBuffer; // [n*80] = length, [n*80+1] = string
    unsigned char*  uarttxBuffer;
    unsigned char*  uartrxBuffer;
    unsigned char*  uarttxdbgBuffer;
    unsigned char*  ctrlboardBuffer;
    unsigned char* heartbeatBuffer;
} CODEC_CONTEXT;

enum {
    CMD_WIEGAND_ENABLE = 11 
};

enum {
    CMD_SWUART_CODEC_INIT = 21
};

enum {
    CMD_CTRLBOARD_CODEC_INIT = 31,
    CMD_CTRLBOARD_CODEC_WRITE = 32,
    CMD_CTRLBOARD_CODEC_READ = 33,
    CMD_CTRLBOARD_CODEC_HEARTBEAT_WRITE = 34,
    CMD_CTRLBOARD_CODEC_HEARTBEAT_READ = 35
};

enum {
    CMD_SWUARTTX_CODEC_INIT  = 41,
    CMD_SWUARTRX_CODEC_INIT  = 42,
    CMD_SWUART_READ_CODEC    = 43,
    CMD_SWUART_WRITE_CODEC   = 44,
    CMD_SWUART_SETPARITY     = 45,
};

enum{
    CMD_SWUART_DBG_CODEC_INIT = 51,
    CMD_SWUART_DBG_CODEC_WRITE = 52,
    CMD_SWUART_DBG_CODEC_SETPARITY = 53,
};
///////////////////////////////////////////////////////////////////////////
//                              Globale Variable
///////////////////////////////////////////////////////////////////////////
int commandBuffer[4] __attribute__ ((aligned(16), section (".sbss")));
unsigned char wiegandBuffer[WIEGAND_BUFFER_SIZE]    __attribute__ ((aligned(16), section (".sbss")));
unsigned char printfBuffer[PRINTF_BUFFER_SIZE]      __attribute__ ((aligned(16), section (".sbss")));
unsigned char uarttxBuffer[UART_TX_BUFFER_SIZE]      __attribute__ ((aligned(16), section (".sbss")));
unsigned char uartrxBuffer[UART_RX_BUFFER_SIZE]      __attribute__ ((aligned(16), section (".sbss")));
unsigned char uarttxdbgBuffer[UART_TX_DBG_BUFFER_SIZE] __attribute__ ((aligned(16), section (".sbss")));
unsigned char ctrlboardBuffer[CTRLBOARD_BUFFER_SIZE]      __attribute__ ((aligned(16), section (".sbss")));
unsigned char heartbeatBuffer[HEARTBEAT_BUFFER_SIZE]      __attribute__ ((aligned(16), section (".sbss")));


static unsigned long* card_id = (unsigned long*)wiegandBuffer;
int HeartBeatTimeout = 500000000;

///////////////////////////////////////////////////////////////////////////
//                              Function Decleration
///////////////////////////////////////////////////////////////////////////


static __inline void or32_invalidate_cache(void *ptr, int bytes)
{
    unsigned int line_size;
    unsigned int cache_size;
    unsigned int start;
    unsigned int end;
    int ncs, bs;

    // Number of cache sets
    ncs = ((mfspr(SPR_DCCFGR) >> 3) & 0xf);

    // Number of cache block size
    bs  = ((mfspr(SPR_DCCFGR) >> 7) & 0x1) + 4;

    cache_size = (1 << (ncs+bs));
    line_size  = (1 << bs);

    if (bytes < 0)
        bytes = cache_size;

    start = ((unsigned int)ptr) & ~(line_size-1);
    end   = ((unsigned int)ptr) + bytes - 1;
    end   = ((unsigned int)end) & ~(line_size-1);
    if (end > start + cache_size - line_size) {
        end = start + cache_size - line_size;
    }

    do {
        mtspr(SPR_DCBIR, start);
        start += line_size;
    } while (start <= end);
}


int main(int argc, char **argv)
{
    int length;
    int status =0;
    int count =0;
    unsigned char* string = 0;
    CODEC_CONTEXT* codecCtxt;
    int command, parameter0, parameter1, parameter2;
    int i, index;
    int tmp[2] = {0x33, 0x66};
    uint8_t HeartbeatStatus=0;
    uint8_t outBuffer[64] = { 0 };

    int j =0;
    static int usefullduplexuart = 0;
    static int swuartdbgenable = 0;
    static int wiegandenable = 0;
    
    // internal information for arm
    memset(commandBuffer, 0x0, sizeof(commandBuffer));
    codecCtxt = (CODEC_CONTEXT*)*(int*)(CODEC_CONTEXT_ADDRESS);
    codecCtxt->commandBuffer = commandBuffer;
    codecCtxt->commandBufferLength = COMMAND_BUFFER_SIZE;
    codecCtxt->wiegandBuffer = wiegandBuffer;
    codecCtxt->wiegandBufferLength = WIEGAND_BUFFER_SIZE;
    codecCtxt->printfBuffer = printfBuffer;
    codecCtxt->uarttxBuffer = uarttxBuffer;
    codecCtxt->uartrxBuffer = uartrxBuffer;
    codecCtxt->uarttxdbgBuffer = uarttxdbgBuffer;
    codecCtxt->ctrlboardBuffer = ctrlboardBuffer;
    codecCtxt->heartbeatBuffer = heartbeatBuffer;

    // reset buffers
    memset((void*)commandBuffer, 0, COMMAND_BUFFER_SIZE);
    memset((void*)wiegandBuffer, 0, WIEGAND_BUFFER_SIZE);
    memset((void*)printfBuffer,  0, PRINTF_BUFFER_SIZE);
    memset((void*)ctrlboardBuffer,  0, CTRLBOARD_BUFFER_SIZE);
    memset((void*)heartbeatBuffer,  0, HEARTBEAT_BUFFER_SIZE);
    memset((void*)uarttxBuffer,  0, UART_TX_BUFFER_SIZE);
    memset((void*)uartrxBuffer,  0, UART_RX_BUFFER_SIZE);
    memset((void*)uarttxdbgBuffer,  0, UART_TX_DBG_BUFFER_SIZE);
    

       
    while(1)
    {     
        uint16_t communicateReg = MMIO_Read(RISC_COMMUNICATE_REG);
		
        if (communicateReg == 0x1234)
        {
            break;
        }
		MMIO_Write(RISC_COMMUNICATE_REG, 1);
         //if (j>= 1000)
        {
         if (MMIO_Read(0x16A4) == 0x1111)
         {         	 
        	 // read command and parameters

             //or32_invalidate_cache(commandBuffer, 32);
             dc_invalidate();      

        	 command = ENDIAN_SWAP32(commandBuffer[0]);
               
        if (command)
        {
            parameter0 = ENDIAN_SWAP32(commandBuffer[1]);
            parameter1 = ENDIAN_SWAP32(commandBuffer[2]);
            parameter2 = ENDIAN_SWAP32(commandBuffer[3]);

            switch (command)
            {
                case CMD_WIEGAND_ENABLE:                	
                    if (parameter0)
                    {
                        start_wiegand_timer(2); /* timer 0 used by system, do not use timer 0 */
                        wg1_begin(parameter1, parameter2);
                    }
                    else
                    {
                        start_wiegand_timer(1); /* timer 0 used by system, do not use timer 0 */
                        wg0_begin(parameter1, parameter2);
                    }
                    wiegandenable = 1;
                    break;
                 case CMD_SWUART_CODEC_INIT:
                    SwUartInit(parameter0, parameter1);
                    break;
                 case CMD_SWUARTTX_CODEC_INIT:
                    SwUartInit(parameter0, parameter1);
                    usefullduplexuart = 1;
                    break;                 
                 case CMD_SWUARTRX_CODEC_INIT:
                 	SwUartRxInit(parameter0, parameter1);
                 	break;
                 case CMD_SWUART_READ_CODEC:
                 	{
                 		unsigned char*pbuf = (uint8_t*)codecCtxt->uartrxBuffer;  							
						pbuf[0] = SwUartBufferCopy(&pbuf[1], (int)parameter0);
						
                 	}                     	
                 	break;
                 case CMD_SWUART_WRITE_CODEC:
        		 {
        		 	or32_invalidate_cache(uarttxBuffer, (int)parameter0);
            		SwUartSetWrite(uarttxBuffer, (int)parameter0);        
        		 }            		 
                 break;
                 case CMD_SWUART_SETPARITY:
                 	SwUartSetParity((int)parameter0);
                 break;
                 case CMD_SWUART_DBG_CODEC_INIT:
                 SwUartDbgInit(parameter0, parameter1);
                 swuartdbgenable = 1;
                 break;
                 case CMD_SWUART_DBG_CODEC_WRITE:                     
                 {                  
        		 	or32_invalidate_cache(uarttxdbgBuffer, (int)parameter0);
            		SwUartDbgSetWrite(uarttxdbgBuffer, (int)parameter0);        
        		 }
                 break;
                 case CMD_SWUART_DBG_CODEC_SETPARITY:
                 	SwUartDbgSetParity((int)parameter0);
                 break;
                 case CMD_CTRLBOARD_CODEC_INIT:
                    start_wiegand_timer(0);
                    start_wiegand_timer(1);
                    start_wiegand_timer(2);
                    //HeartBeatInit(&Heartbeatinfo);
                    TreadmillGpioInit(parameter0 ,parameter1,parameter2 );
                    break; 
                 case CMD_CTRLBOARD_CODEC_WRITE:  
                 {
                    //or32_invalidate_cache((void*)codecCtxt->ctrlboardBuffer, 64);
                    status =TMRSendData(((uint8_t*)codecCtxt->ctrlboardBuffer) ,parameter0,HeartBeatTimeout,((HEARTBEAT_INFO*)codecCtxt->heartbeatBuffer));
                    outBuffer[1]  = ReceiveTMRData(outBuffer + 2 , 62 , 10 , &outBuffer[0],HeartBeatTimeout,((HEARTBEAT_INFO*)codecCtxt->heartbeatBuffer));


                    if(status)
                    {
                         MMIO_Write(0x1698, 0x0);
                    }
                    else
                    {
                         MMIO_Write(0x1698, 0x1);
                    }
                    break; 
                 }
                 case CMD_CTRLBOARD_CODEC_READ:   
                 {
                    memcpy(((uint8_t*)codecCtxt->ctrlboardBuffer), outBuffer, 64);
                    if(outBuffer[1])
                    {
                        MMIO_Write(0x1698, 0x0);
                    }
                    else
                    {
                        MMIO_Write(0x1698, 0x1);
                    }
                
                    break;
                }
                case CMD_CTRLBOARD_CODEC_HEARTBEAT_WRITE:  
                    HeartBeatTimeout = parameter0;
                    break;
                case CMD_CTRLBOARD_CODEC_HEARTBEAT_READ: 
                {
                    break;     
                }
                default:
                    break;
            }
            commandBuffer[0] = 0;

        }
                MMIO_Write(0x16A4, 0x0000);
        }
            //j=0;
        }
        //j++;

		if (usefullduplexuart)
		{
			unsigned char val =0;

			if(SwUartRead(&val))
			{
				SwUartSetRead(&val, 1);
				string = &val;
			}
			SwUartSend();
		}

        if (swuartdbgenable)        
        {
            SwUartDbgSend();
        }
        // get card id from wiegand
		if (wiegandenable)
		{			
        	wg0_DoWiegandConversion(card_id);
        	wg1_DoWiegandConversion(card_id);
      	}
 
    }
    
    MMIO_Write(RISC_COMMUNICATE_REG, 0x5678);
    MMIO_Write(RISC_COMMUNICATE_REG, 0x0000);
}
