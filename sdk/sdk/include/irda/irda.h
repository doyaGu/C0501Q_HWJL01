
#ifndef IRDA_H
#define IRDA_H

#include <stdio.h>
#include <unistd.h>
#include "stdint.h"

#define IRDA_ENABLE_CACHE 
//============================ Baund Rate  Mapping =============================================
#define IRDA_CLK                     	    27000000//48000000//22118000//48000000//18432000/2	
#define PSR_CLK				                1843200

#define FIFO128_ENABLE                      0x01     
#define OUT3_USE_BIT6						0x01

//CPE_BAUD_115200    can't divide
#define IRDA_BAUD_115200                    (int)(PSR_CLK / 1843200)//1
#define IRDA_BAUD_57600                     (int)(PSR_CLK / 921600) //2
#define IRDA_BAUD_38400			                (int)(PSR_CLK / 614400) //3
#define IRDA_BAUD_19200                     (int)(PSR_CLK / 307200) //6
#define IRDA_BAUD_14400                     (int)(PSR_CLK / 230400) //8
#define IRDA_BAUD_9600                      (int)(PSR_CLK / 153600) //12
#define IRDA_BAUD_4800                      (int)(PSR_CLK / 76800)  //24
#define IRDA_BAUD_2400                      (int)(PSR_CLK / 38400)  //48
#define IRDA_BAUD_1200                      (int)(PSR_CLK / 19200)  //96

//============================ Register Address Mapping =========================================

//Registre Address Define (IrDA/FIR)
#define SERIAL_TST      			        0x14
#define SERIAL_MRXLENL                0x30
#define SERIAL_MRXLENH                0x34
#define SERIAL_FMIIR_DMA			        0x3C
#define SERIAL_FMIIR  			          0x3C
#define SERIAL_FMIIER				          0x40
#define SERIAL_STFF_STS				        0x44
#define SERIAL_STFF_RXLENL			      0x48
#define SERIAL_STFF_RXLENH			      0x4c
#define SERIAL_FMLSR				          0x50
#define SERIAL_FMLSIER				        0x54
#define SERIAL_RSR				            0x58
#define SERIAL_RXFF_CNTR			        0x5c
#define SERIAL_LSTFMLENL			        0x60
#define SERIAL_LSTFMLENH			        0x64

//=========================== Register Value/Bit Define ==========================================

//Register "PSR=0x08(DLAB=1)" 
#define SERIAL_PSR_FIR_FIX_VALUE		    0x06


//Register "FCR=0x08" Define (IrDA/FIR)
#define SERIAL_FCR_TX_TRIGGER16_LEVEL_1		0x01 
#define SERIAL_FCR_TX_TRIGGER16_LEVEL_3		0x03 
#define SERIAL_FCR_TX_TRIGGER16_LEVEL_9		0x09 
#define SERIAL_FCR_TX_TRIGGER16_LEVEL_13	0x0D 
#define SERIAL_FCR_RX_TRIGGER16_LEVEL_1		0x01 
#define SERIAL_FCR_RX_TRIGGER16_LEVEL_4		0x04 
#define SERIAL_FCR_RX_TRIGGER16_LEVEL_8		0x08 
#define SERIAL_FCR_RX_TRIGGER16_LEVEL_14	0x0E 

#define SERIAL_FCR_TX_TRIGGER32_LEVEL_1		0x01 
#define SERIAL_FCR_TX_TRIGGER32_LEVEL_6		0x06 
#define SERIAL_FCR_TX_TRIGGER32_LEVEL_18	0x12 
#define SERIAL_FCR_TX_TRIGGER32_LEVEL_26	0x1A 
#define SERIAL_FCR_RX_TRIGGER32_LEVEL_1		0x01 
#define SERIAL_FCR_RX_TRIGGER32_LEVEL_8		0x08 
#define SERIAL_FCR_RX_TRIGGER32_LEVEL_16	0x10 
#define SERIAL_FCR_RX_TRIGGER32_LEVEL_28	0x1C 

#define SERIAL_FCR_TX_TRIGGER64_LEVEL_1		1 
#define SERIAL_FCR_TX_TRIGGER64_LEVEL_16	16 
#define SERIAL_FCR_TX_TRIGGER64_LEVEL_32	32 
#define SERIAL_FCR_TX_TRIGGER64_LEVEL_56	56 
#define SERIAL_FCR_RX_TRIGGER64_LEVEL_1		1 
#define SERIAL_FCR_RX_TRIGGER64_LEVEL_16	16 
#define SERIAL_FCR_RX_TRIGGER64_LEVEL_32	32  
#define SERIAL_FCR_RX_TRIGGER64_LEVEL_56	56 

#define SERIAL_FCR_TX_TRIGGER128_LEVEL_1	1 
#define SERIAL_FCR_TX_TRIGGER128_LEVEL_32	32 
#define SERIAL_FCR_TX_TRIGGER128_LEVEL_64	64 
#define SERIAL_FCR_TX_TRIGGER128_LEVEL_120  120 
#define SERIAL_FCR_RX_TRIGGER128_LEVEL_1	1 
#define SERIAL_FCR_RX_TRIGGER128_LEVEL_32	32 
#define SERIAL_FCR_RX_TRIGGER128_LEVEL_64	64  
#define SERIAL_FCR_RX_TRIGGER128_LEVEL_120	120 

#ifdef FIFO32_ENABLE 
    #define FIFO_SIZE			            0x20
    #define SERIAL_FCR_TX_TRI1	            SERIAL_FCR_TX_TRIGGER32_LEVEL_1
    #define SERIAL_FCR_TX_TRI2	            SERIAL_FCR_TX_TRIGGER32_LEVEL_6
    #define SERIAL_FCR_TX_TRI3	            SERIAL_FCR_TX_TRIGGER32_LEVEL_18
    #define SERIAL_FCR_TX_TRI4	            SERIAL_FCR_TX_TRIGGER32_LEVEL_26
    #define SERIAL_FCR_RX_TRI1	            SERIAL_FCR_RX_TRIGGER32_LEVEL_1
    #define SERIAL_FCR_RX_TRI2	            SERIAL_FCR_RX_TRIGGER32_LEVEL_8
    #define SERIAL_FCR_RX_TRI3	            SERIAL_FCR_RX_TRIGGER32_LEVEL_16
    #define SERIAL_FCR_RX_TRI4	            SERIAL_FCR_RX_TRIGGER32_LEVEL_28
#endif

#ifdef FIFO64_ENABLE                             
    #define FIFO_SIZE			            0x40
    #define SERIAL_FCR_TX_TRI1	            SERIAL_FCR_TX_TRIGGER64_LEVEL_1	
    #define SERIAL_FCR_TX_TRI2	            SERIAL_FCR_TX_TRIGGER64_LEVEL_16	
    #define SERIAL_FCR_TX_TRI3	            SERIAL_FCR_TX_TRIGGER64_LEVEL_32	
    #define SERIAL_FCR_TX_TRI4	            SERIAL_FCR_TX_TRIGGER64_LEVEL_56
    #define SERIAL_FCR_RX_TRI1	            SERIAL_FCR_RX_TRIGGER64_LEVEL_1	
    #define SERIAL_FCR_RX_TRI2	            SERIAL_FCR_RX_TRIGGER64_LEVEL_16	
    #define SERIAL_FCR_RX_TRI3	            SERIAL_FCR_RX_TRIGGER64_LEVEL_32	
    #define SERIAL_FCR_RX_TRI4	            SERIAL_FCR_RX_TRIGGER64_LEVEL_56
#endif

#ifdef FIFO128_ENABLE                             
    #define FIFO_SIZE			            0x80
    #define SERIAL_FCR_TX_TRI1	            SERIAL_FCR_TX_TRIGGER128_LEVEL_1	
    #define SERIAL_FCR_TX_TRI2	            SERIAL_FCR_TX_TRIGGER128_LEVEL_32	
    #define SERIAL_FCR_TX_TRI3	            SERIAL_FCR_TX_TRIGGER128_LEVEL_64	
    #define SERIAL_FCR_TX_TRI4	            SERIAL_FCR_TX_TRIGGER128_LEVEL_120
    #define SERIAL_FCR_RX_TRI1	            SERIAL_FCR_RX_TRIGGER128_LEVEL_1	
    #define SERIAL_FCR_RX_TRI2	            SERIAL_FCR_RX_TRIGGER128_LEVEL_32	
    #define SERIAL_FCR_RX_TRI3	            SERIAL_FCR_RX_TRIGGER128_LEVEL_64	
    #define SERIAL_FCR_RX_TRI4	            SERIAL_FCR_RX_TRIGGER128_LEVEL_120
#endif

#define SERIAL_FCR_TX_FIFO_RESET		    0x04 
#define SERIAL_FCR_RX_FIFO_RESET		    0x02 
#define SERIAL_FCR_TXRX_FIFO_ENABLE         0x01 
             
             
//Register "TST=0x10" Define     
#ifdef OUT3_USE_BIT6      
    #define SERIAL_MCR_OUT3 	                0x40
    #define SERIAL_MCR_DMA2 	                0x20    
#else
    #define SERIAL_MCR_OUT3 	                0x20
#endif
                                       
//Register "TST=0x14" Define                
#define SERIAL_TST_TEST_PAR_ERR 	        0x01
#define SERIAL_TST_TEST_FRM_ERR 	        0x02
#define SERIAL_TST_TEST_BAUDGEN 	        0x04
#define SERIAL_TST_TEST_PHY_ERR       	    0x08
#define SERIAL_TST_TEST_CRC_ERR	    	    0x10
                          
                                            
//Register "MDR=0x20" Define                
#define SERIAL_MDR_IITx				        0x40
#define SERIAL_MDR_FIRx				        0x20
#define SERIAL_MDR_MDSEL			        0x1c
#define SERIAL_MDR_DMA_ENABLE          	    0x10
#define SERIAL_MDR_FMEND_ENABLE			    0x08
#define SERIAL_MDR_SIP_BY_CPU			    0x04
#define SERIAL_MDR_UART_MODE			    0x00
#define SERIAL_MDR_SIR_MODE			        0x01
#define SERIAL_MDR_FIR_MODE			        0x02
#define SERIAL_MDR_TX_INV			        0x40
#define SERIAL_MDR_RX_INV			        0x20
//Register "ACR-0x24" Define (IrDA/FIR)
#define SERIAL_ACR_TX_ENABLE			    0x01
#define SERIAL_ACR_RX_ENABLE			    0x02
#define SERIAL_ACR_FIR_SET_EOT			    0x04
#define SERIAL_ACR_FORCE_ABORT			    0x08
#define SERIAL_ACR_SEND_SIP			        0x10
#define SERIAL_ACR_STFF_TRGL_1			    0x01
#define SERIAL_ACR_STFF_TRGL_4			    0x04
#define SERIAL_ACR_STFF_TRGL_7			    0x07
#define SERIAL_ACR_STFF_TRGL_8			    0x08
                                            
#define SERIAL_ACR_STFF_TRG1			    0x00
#define SERIAL_ACR_STFF_TRG2			    0x20
#define SERIAL_ACR_STFF_TRG3			    0x40
#define SERIAL_ACR_STFF_TRG4			    0x60
#define SERIAL_ACR_STFF_CLEAR			    0x9F                                            
                                            
                                            
#define SIR_ACR_SIR_PW_316BRATE			    0x00
#define SIR_ACR_SIR_PW_163US			    0x80

//Register "TXLENL/TXLENH=0x28/0x2C" Define (IrDA/FIR)
#define SERIAL_TXLENL_DEFAULT			    0x00FF

//Register "MRXLENL/MRXLENH=0x30/0x34" Define (IrDA/FIR)
#define SERIAL_MRXLENHL_DEFAULT			    0x1F40


//Register "PLR=0x38" Define (IrDA/FIR)
#define SERIAL_PLR_16    			        0x00
#define SERIAL_PLR_4  			            0x01
#define SERIAL_PLR_8    			        0x02
#define SERIAL_PLR_32  			            0x03

//Register "PLR=0x3c" Define (IrDA/FIR)
#define SERIAL_FMIIER_PIO_EOF               0x10
#define SERIAL_FMIIER_PIO_TX_UNDERRUN       0x8
#define SERIAL_FMIIER_PIO_RX_OVERRUN        0x4
#define SERIAL_FMIIER_PIO_TT                0x2
#define SERIAL_FMIIER_PIO_RT                0x1

//Register "FMIIER/PIO=0x40" Define (IrDA/FIR)
#define SERIAL_FMIIR_PIO_FRM_SENT		    0x20
#define SERIAL_FMIIR_PIO_EOF_DETECTED	    0x10
#define SERIAL_FMIIR_PIO_TXFIFO_URUN	    0x08
#define SERIAL_FMIIR_PIO_RXFIFO_ORUN	    0x04
#define SERIAL_FMIIR_PIO_TXFIFO_TRIG	    0x02
#define SERIAL_FMIIR_PIO_RXFIFO_TRIG	    0x01
#define SERIAL_FMIIR_PIO_RX_EVENT		    0x15


#define SERIAL_FMIIR_DMA_FRM_SENT		    0x20
#define SERIAL_FMIIR_DMA_TXFIFO_URUN		0x08
#define SERIAL_FMIIR_DMA_STFIFO_ORUN		0x04
#define SERIAL_FMIIR_DMA_STFIFO_TIMEOUT	    0x02
#define SERIAL_FMIIR_DMA_STFIFO_TRIG		0x01
#define SERIAL_FMIIR_DMA_RX_EVENT		    0x07


//Register "FMIIER/PIO=0x40" Define (IrDA/FIR)
#define SERIAL_FMIIER_PIO_FRM_SENT		    0x20
#define SERIAL_FMIIER_PIO_EOF_DETECTED		0x10
#define SERIAL_FMIIER_PIO_TXFIFO_URUN		0x08
#define SERIAL_FMIIER_PIO_RXFIFO_ORUN		0x04
#define SERIAL_FMIIER_PIO_TXFIFO_TRIG		0x02
#define SERIAL_FMIIER_PIO_RXFIFO_TRIG		0x01

#define SERIAL_FMIIER_DMA_FRM_SENT		    0x20
#define SERIAL_FMIIER_DMA_TXFIFO_URUN		0x08
#define SERIAL_FMIIER_DMA_STFIFO_ORUN		0x04
#define SERIAL_FMIIER_DMA_STFIFO_TIMEOUT	0x02
#define SERIAL_FMIIER_DMA_STFIFO_TRIG		0x01



//Register "STFF_STS=0x44" Define (IrDA/FIR)
#define SERIAL_STFF_STS_RXFIFO_ORUN		    0x01
#define SERIAL_STFF_STS_CRC_ERR			    0x02
#define SERIAL_STFF_STS_PHY_ERR			    0x04
#define SERIAL_STFF_STS_SIZE_ERR		    0x08
#define SERIAL_STFF_STS_STS_VLD			    0x10


//Register "FMLSR=0x50" Define (IrDA/FIR)
#define SERIAL_FMLSR_FIR_IDLE			    0x80
#define SERIAL_FMLSR_TX_EMPTY			    0x40
#define SERIAL_FMLSR_STFIFO_FULL		    0x20
#define SERIAL_FMLSR_SIZE_ERR			    0x10
#define SERIAL_FMLSR_PHY_ERROR		        0x08
#define SERIAL_FMLSR_CRC_ERROR		        0x04
#define SERIAL_FMLSR_STFIFO_EMPTY		    0x02
#define SERIAL_FMLSR_RXFIFO_EMPTY		    0x01

//Register "FMLSIER=0x54" Define (IrDA/FIR)
#define SERIAL_FMLSIER_FIR_IDLE			    0x80
#define SERIAL_FMLSIER_TX_EMPTY			    0x40
#define SERIAL_FMLSIER_STFIFO_FULL		    0x20
#define SERIAL_FMLSIER_SIZE_ERR			    0x10
#define SERIAL_FMLSIER_PHY_ERROR		    0x08
#define SERIAL_FMLSIER_CRC_ERROR		    0x04
#define SERIAL_FMLSIER_STFIFO_EMPTY		    0x02
#define SERIAL_FMLSIER_RXFIFO_EMPTY		    0x01

//=========================== Send Data Define ==========================================
#define SERIAL_FIR_FRAME_MAX_SIZE		    2000
#define SERIAL_FIR_FRAME_MAX_NUMBER		    7
#define TX_BUFFER_SIZE		                10000 

//=========================== Receive Data Define ==========================================
#define RX_BUFFER_SIZE		                10000 

//=========================== Error Type Definition ==========================================
#define SERIAL_FIR_RX_ERROR_OVERRUN		    0x01
#define SERIAL_FIR_TX_ERROR_UNDERRUN		0x02


//=========================== DMA REG ========================================================
#define DMA_REG_C0_DST_ADDR           0x010C
#define DMA_CHANNEL_OFFSET            0x20

//=========================== Structure Define ==========================================
typedef struct
{
	uint8_t  PSR;
	uint8_t  DLL;
	uint8_t  DLM;
	uint8_t  MDR;
	uint8_t  PLR;
	uint32_t TXLENHL;
	uint32_t LSTFMLENHL;
	uint8_t  LST_FrameNumber;
	uint8_t  TX_FIFO_Trigger_Level;
	uint8_t  TX_FMIIER;
	uint8_t  TX_FMLSIER;
	uint32_t TX_PORT_ADDRESS;	
	uint8_t  IRQNumber_FMIIR;
	uint8_t  IRQNumber_FMLSR;	
	uint16_t SET_EOT_FRAME_LENGTH;
	uint16_t SET_EOT_LAST_FRAME_LENGTH;
	
	uint32_t DMA_RegisterPort;
	uint32_t DMA_Request_Select;
	
}IRDA_FIR_TX_Config_Status;


typedef struct
{
	uint8_t  PSR;
	uint8_t  DLL;
	uint8_t  DLM;
	uint8_t  PLR;	
	uint8_t  MDR; //Select FIR/DMA mode
	uint32_t MRXLENHL; //maximum length
	uint8_t  RX_FIFO_Trigger_Level;
	uint8_t  ST_FIFO_Trigger_Level;
	uint8_t  RX_FMIIER;
	uint8_t  RX_FMLSIER;
	uint32_t RX_PORT_ADDRESS;	
	uint8_t  IRQNumber_FMIIR;	
	uint8_t  IRQNumber_FMLSR;		
	uint32_t DMA_RegisterPort;
	uint32_t DMA_Request_Select;			
}IRDA_FIR_RX_Config_Status;


typedef struct
{
	uint8_t *bpTXDataBuffer;
	uint32_t RemainByte;
	uint32_t TotalByte;
	uint16_t  TX_Expect_Frame_Numbers;
	uint16_t TX_1Frame_Bytes;
	uint16_t  Send_FRM_SENT_OK_Counter;
	uint32_t  SendErrorType;
  uint32_t SendByte;

}IRDA_FIR_TX_Data_Buffer_Status;

typedef struct
{
	volatile int32_t Len;
	volatile uint8_t  STFFStatus;
   	
}IRDA_FIR_RX_DMA_Status;

			
typedef struct
{
	uint8_t  *bpRXDataBuffer;
	uint16_t  RX_Receive_Frame_Counter;
	uint32_t ReceivedBytes;
	uint32_t ExpectBytes;
	uint16_t  RX_Expect_Frame_Numbers;
	uint32_t  ReceiveErrorType_FMIIR;
	uint8_t  ReceiveErrorType_FMLSR;	
	uint8_t  ReceiveStatusFIFO_Number;	
	IRDA_FIR_RX_DMA_Status DMA_Buffer[10]; //Reserve 20 status fifo
	uint8_t  *bpRXDMABuffer;
	uint16_t  RX_Merge_Frame_Counter;
	uint32_t  dmaSize;


}IRDA_FIR_RX_Data_Buffer_Status;
			
typedef struct
{
	uint32_t Slect_PORT_ADDRESS;	
	uint8_t  Select_IRQNumber_FMIIR;
	uint8_t  Select_IRQNumber_FMLSR;	
	uint32_t DMA_Reg_ADD;
	uint32_t DMA_Port;	

	
}FIR_TEST_SELECT_PORT_STRUCTURE;			
			
typedef struct
{
	uint32_t  port;
	volatile uint32_t  Length;
	char    *cptRXDataBuffer;
	uint32_t  IRQ_Num;
   	
}IRDA_SIR_RX_Status;
			
typedef struct
{
	uint32_t  port;
	volatile uint32_t  Length;
	char    *cptTXDataBuffer;
   	
}IRDA_SIR_TX_Status;

typedef struct
{
	uint8_t  bSetEOTMethodEnable;
	uint8_t  bForceAbortEnable;
	uint8_t  SIPSentByCPU;
	uint8_t  PreambleLength;
	uint8_t  InvertedPulseEnable_TX;
	uint8_t  InvertedPulseEnable_RX;	
	uint8_t  FIRDMAEnable;
  uint8_t  temp;
}IRDA_FIR_Mode;

typedef void (*PrHandler)(void);
	

//=============== IrDA DMA part ==========================================
#define IRDA_DMATYPE_AHB	2
#define IRDA_DMATYPE_APB	1
#define IRDA_DMATYPE_NONE	0


#endif
