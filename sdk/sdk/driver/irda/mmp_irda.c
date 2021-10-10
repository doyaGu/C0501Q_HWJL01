#include <stdio.h>
#include <unistd.h>
#include "irda/irda.h"
#include "irda/irda_error.h"
#include "irda/mmp_irda.h"
#include "irda/uart.h"
#include "ite/ith.h"
#include "ite/itp.h"
#include "ite/ith_defs.h"

#define SIR_TX_AHBDMA_CHANNEL	0x2
#define SIR_RX_AHBDMA_CHANNEL	0x3
#define UART_BASE			  ITH_UART0_BASE
#define UART_AHB_BASE		ITH_UART0_BASE


uint8_t IRDA_SIP_TimerCounter;
uint32_t IRDA_PORT_ForSIP;
IRDA_FIR_TX_Config_Status                     IrDA_TX_Status;
IRDA_FIR_TX_Data_Buffer_Status                SendData;
IRDA_FIR_RX_Config_Status                     IrDA_RX_Status;
IRDA_FIR_RX_Data_Buffer_Status                ReceiveData;
FIR_TEST_SELECT_PORT_STRUCTURE                SelectPort;
IRDA_SIR_RX_Status                            SIR_RX_Device;
IRDA_SIR_TX_Status                            SIR_TX_Device;
IRDA_FIR_Mode                                 IrDA_Mode_Sel;
uint8_t rx_trigger_chs[] = { 1, 32, 64, 120};
uint8_t rx_trigger_level;
static  int  dmaChannel;

static void SetPadSel(uint32_t port)
{
	uint32_t data;

	ithWriteRegMaskA(ITH_GPIO_BASE + 0xD0,0x4<<4,0x7<<4);//Host_sel	
	data = ithReadRegA(ITH_GPIO_BASE + 0x90);
	data |= (1<<4) | (1<<6) | (1<<12);
	ithWriteRegA(ITH_GPIO_BASE + 0x90,data);
	
}

//=========================================================================================
// Function Name: Ext_IRDA_INT_Init
// Description:  add by silas for A321
//=========================================================================================
int Ext_IRDA_INT_Init(uint32_t IRQ_IrDA,PrHandler function)
{
    //TODO
#if 0    
	//setup INT
	Ext_ClearInt(IRQ_IrDA);
	Ext_CloseInt(IRQ_IrDA);
	
	//connnect IrDA interrupt
	if (!Ext_ConnectInt(IRQ_IrDA, function ))
	{
		return false;
	}		
	
	//enable interrupt
	Ext_SetIntTrig(IRQ_IrDA,LEVEL,H_ACTIVE);
	Ext_EnableInt(IRQ_IrDA);	
#endif	
	return true;
}
	
//=========================================================================================
// Function Name: fLib_IRDA_INT_Init
// Description: 
//=========================================================================================
int fLib_IRDA_INT_Init(uint32_t IRQ_IrDA,PrHandler function)
{
#if 0    
	//setup INT
	fLib_ClearInt(IRQ_IrDA);
	fLib_CloseInt(IRQ_IrDA);
	
	//connnect IrDA interrupt
	if (!fLib_ConnectInt(IRQ_IrDA, function ))
	{
		return false;
	}		
	
	//enable interrupt
	fLib_SetIntTrig(IRQ_IrDA,LEVEL,H_ACTIVE);
	fLib_EnableInt(IRQ_IrDA);	
#endif	
	return true;
}	

//=========================================================================================
// Function Name: Ext_IRDA_INT_Disable
// Description: 
//=========================================================================================
int Ext_IRDA_INT_Disable(uint32_t IRQ_IrDA)
{

#if 0
 if (Ext_CloseInt(IRQ_IrDA)==true)

    return true;
 else return false;   
 #endif

    return true;
}	

//=========================================================================================
// Function Name: fLib_IRDA_INT_Disable
// Description: 
//=========================================================================================
int fLib_IRDA_INT_Disable(uint32_t IRQ_IrDA)
{

#if 0
 if (fLib_CloseInt(IRQ_IrDA)==true)
    return true;
 else return false;   
#endif
    return true;
}	
//=========================================================================================
// Function Name: fLib_SetFIRPrescal =>
// Description: "DLL=0x00" "DLM=0x04" "PSR=0x08"
//=========================================================================================				
void fLib_SetFIRPrescal(uint32_t port,uint32_t psr_Value)
{
	//see IrDA spec P.33
	
	  uint32_t lcr;	
    lcr = ithReadRegA(port + SERIAL_LCR);
    lcr = lcr & ~SERIAL_LCR_DLAB;
	/* Set DLAB=1 */
    ithWriteRegA(port + SERIAL_LCR, SERIAL_LCR_DLAB);	
	
	if(psr_Value>0x1f)
	{
		psr_Value=1;
	}
	ithWriteRegA(port + SERIAL_PSR, psr_Value);
  ithWriteRegA(port + SERIAL_DLM, 0);
  ithWriteRegA(port + SERIAL_DLL, 1);   
  ithWriteRegA(port + SERIAL_LCR,lcr);		
} 

//=========================================================================================
// Function Name: fLib_IRDA_Set_FCR / fLib_IRDA_Enable_FCR / fLib_IRDA_Disable_FCR fLib_IRDA_Set_FCR_Trigl_Level
//
// Description: Register "FCR=0x08"
//=========================================================================================	
void fLib_IRDA_Set_FCR(uint32_t port, uint32_t setmode)
{
    ithWriteRegA(port+SERIAL_FCR,setmode);
}

void fLib_IRDA_Enable_FCR(uint32_t port, uint32_t setmode)
{
    uint8_t data;
    
    data=0x01; //FIFO Enable
    ithWriteRegA(port+SERIAL_FCR,(data|setmode));

}	
						
void fLib_IRDA_Set_FCR_Trigl_Level(uint32_t port, uint32_t Tx_Tri_Level , uint32_t Rx_Tri_Level )
{
    uint8_t data;
    
    data=0x01; //FIFO Enable
    
    switch(Tx_Tri_Level)
          {
           case SERIAL_FCR_TX_TRI1:
                data = data | 0x00;                
           break;
           case SERIAL_FCR_TX_TRI2:
                data = data | 0x10;                
           break;    
           case SERIAL_FCR_TX_TRI3:
                data = data | 0x20;                
           break;
           case SERIAL_FCR_TX_TRI4:
                data = data | 0x30;                
           break;   
           default:
                data = data | 0x00;  
           break;           
            
           }
    
    switch(Rx_Tri_Level)
          {
           case SERIAL_FCR_RX_TRI1:
                data = data | 0x00;                
           break;
           case SERIAL_FCR_RX_TRI2:
                data = data | 0x40;                
           break;    
           case SERIAL_FCR_RX_TRI3:
                data = data | 0x80;                
           break;
           case SERIAL_FCR_RX_TRI4:
                data = data | 0xC0;                
           break;   
           default:
                data = data | 0x00;             
           break;           
            
           }    
    
    
    ithWriteRegA(port+SERIAL_FCR,data);

}	
void fLib_IRDA_Set_FCR_Trigl_Level_2(uint32_t port, uint32_t Tx_Tri_Level , uint32_t Rx_Tri_Level )//By 00/01/02/03
{
    uint8_t data;
    
    data=0x01; //FIFO Enable
    
    switch(Tx_Tri_Level)
          {
           case 0:
                data = data | 0x00;                
           break;
           case 1:
                data = data | 0x10;                
           break;    
           case 2:
                data = data | 0x20;                
           break;
           case 3:
                data = data | 0x30;                
           break;   
           default:
                data = data | 0x00;  
           break;           
            
           }
    
    switch(Rx_Tri_Level)
          {
           case 0:
                data = data | 0x00;                
           break;
           case 1:
                data = data | 0x40;                
           break;    
           case 2:
                data = data | 0x80;                
           break;
           case 3:
                data = data | 0xC0;                
           break;   
           default:
                data = data | 0x00;             
           break;           
            
           }    
    
    
    ithWriteRegA(port+SERIAL_FCR,data);

}	
//=========================================================================================
// Function Name: fLib_IRDA_Set_TST 
// Description: "TST=0x14"
//=========================================================================================				
void fLib_IRDA_Set_TST(uint32_t port,uint8_t setmode)
{
    ithWriteRegA(port + SERIAL_TST, setmode);
}


//=========================================================================================
// Function Name: fLib_IRDA_Set_MDR / fLib_IRDA_Enable_MDR / fLib_IRDA_Disable_MDR  =>ok1009
// Description: "MDR=0x20"
//=========================================================================================				
void fLib_IRDA_Set_MDR(uint32_t port,uint8_t setmode)
{
    ithWriteRegA(port + SERIAL_MDR, setmode);
}
void fLib_IRDA_Enable_MDR(uint32_t port,uint8_t setmode)
{
    uint32_t mdr;
    
    mdr = ithReadRegA(port+SERIAL_MDR);
    mdr &= 0xFF;
    ithWriteRegA(port + SERIAL_MDR, (mdr|setmode));

}
void fLib_IRDA_Disable_MDR(uint32_t port,uint8_t setmode)
{
    uint32_t mdr;
    
    mdr = ithReadRegA(port+SERIAL_MDR);
    mdr &= 0xFF;
    ithWriteRegA(port + SERIAL_MDR, (uint32_t)(mdr&(~setmode)));

}

//=========================================================================================
// Function Name: fLib_IRDA_Disable_ACR / fLib_IRDA_Enable_ACR /fLib_IRDA_Disable_ACR
//                /fLib_IRDA_Set_ACR_STFIFO_Trigl_Level=>ok1009
// Description: Register "ACR=0x24"
//=========================================================================================	
void fLib_IRDA_Set_ACR(uint32_t port,uint32_t mode)
{
	//see IrDA spec P.42

	ithWriteRegA(port + SERIAL_ACR,  mode);
}
void fLib_IRDA_Enable_ACR(uint32_t port,uint32_t mode)
{
	//see IrDA spec P.42
	uint32_t data;

	data = ithReadRegA(port + SERIAL_ACR);
	ithWriteRegA(port + SERIAL_ACR, data | mode);

}
void fLib_IRDA_Disable_ACR(uint32_t port,uint32_t mode)
{
	//see IrDA spec P.40
	uint32_t data;

	data = ithReadRegA(port + SERIAL_ACR);
	ithWriteRegA(port + SERIAL_ACR,(data&(~mode)));

}								
void fLib_IRDA_Set_ACR_STFIFO_Trigl_Level(uint32_t port, uint32_t STFIFO_Tri_Level)
{
    uint32_t data;
    
    data = ithReadRegA(port + SERIAL_ACR);

    data = data & SERIAL_ACR_STFF_CLEAR;

    switch(STFIFO_Tri_Level)
          {
           case SERIAL_ACR_STFF_TRGL_1:
                data = data | SERIAL_ACR_STFF_TRG1;                
           break;
           case SERIAL_ACR_STFF_TRGL_4:
                data = data | SERIAL_ACR_STFF_TRG2;                
           break;    
           case SERIAL_ACR_STFF_TRGL_7:
                data = data | SERIAL_ACR_STFF_TRG3;                
           break;
           case SERIAL_ACR_STFF_TRGL_8:
                data = data | SERIAL_ACR_STFF_TRG4;                
           break;   
           default:
                data = data | 0x00;  
           break;           
            
           }
    
    ithWriteRegA(port+SERIAL_ACR,data);

}	


//=========================================================================================
// Function Name: fLib_IRDA_Set_TXLEN_LSTFMLEN =>ok1009
// Description: "TXLENL=0x28" "TXLENH=0x2C" "LSTFMLENL=0x60" "LSTFMLENL=0x64"
//=========================================================================================				
void fLib_IRDA_Set_TXLEN_LSTFMLEN(uint32_t port,uint32_t Normal_FrmLength,uint32_t Last_FrmLength,uint32_t FrmNum)
{
	//see IrDA spec P.35 & P.44
	if(Normal_FrmLength>0x1fff)
	{
		Normal_FrmLength = 0x1fff;
	}
	else if(Normal_FrmLength == 0)
	{
		Normal_FrmLength = 0x1;	
	}	
  ithWriteRegA(port + SERIAL_TXLENL, (Normal_FrmLength & 0xff));
	ithWriteRegA(port + SERIAL_TXLENH, ((Normal_FrmLength & 0x1f00) >> 8));
   

  ithWriteRegA(port + SERIAL_LSTFMLENL, (Last_FrmLength & 0xff));
	ithWriteRegA(port + SERIAL_LSTFMLENH, ((Last_FrmLength & 0x1f00) >> 8)+((FrmNum) << 5));
   
}
//=========================================================================================
// Function Name: fLib_IRDA_Set_MaxLen =>ok1009
// Description: Register "MRXLENL=0x30" "MRXLENH=0x34"
//=========================================================================================
void fLib_IRDA_Set_MaxLen(uint32_t port,uint32_t Max_RxLength)
{
	//see IrDA spec P.36	( Max_RxLength = 1~27 )
	Max_RxLength = Max_RxLength + 5;
  ithWriteRegA(port + SERIAL_MRXLENH, ((Max_RxLength & 0x1f00) >> 8));
  ithWriteRegA(port + SERIAL_MRXLENL, (Max_RxLength & 0xff));
}
//=========================================================================================
// Function Name: fLib_IRDA_Set_PLR =>ok1009
// Description: "PLR=0x38"
//=========================================================================================	
void fLib_IRDA_Set_PLR(uint32_t port,uint32_t Preamble_Length)
{
	ithWriteRegA(port + SERIAL_PLR,  Preamble_Length);
}

//=========================================================================================
// Function Name: fLib_IRDA_Read_FMIIR =>ok1009
// Description: Register "FMIIR =0x3C"
//=========================================================================================
uint8_t fLib_IRDA_Read_FMIIR(uint32_t port)
{
	uint32_t data;
	data = ithReadRegA(port + SERIAL_FMIIR);
	data &= 0xFF;

  return	(uint8_t)data;
}


//=========================================================================================
// Function Name: fLib_SetFIRInt / fLib_IRDA_Enable_FMIIER / fLib_IRDA_Disable_FMIIER =>ok1009
// Description: Register "FMIIER=0x40"
//=========================================================================================
void fLib_IRDA_Set_FMIIER(uint32_t port,uint32_t mode)
{
	ithWriteRegA(port + SERIAL_FMIIER,  mode);
}
void fLib_IRDA_Enable_FMIIER(uint32_t port,uint32_t mode)
{

	uint32_t data;

	data = ithReadRegA(port + SERIAL_FMIIER);
	ithWriteRegA(port + SERIAL_FMIIER, data | mode);
}									

void fLib_IRDA_Disable_FMIIER(uint32_t port,uint32_t mode)
{

	uint32_t data;
	data = ithReadRegA(port + SERIAL_FMIIER);
	ithWriteRegA(port + SERIAL_FMIIER, (data &(~mode)));
}

//=========================================================================================
// Function Name: fLib_IRDA_Set_FMLSIER / fLib_IRDA_Enable_FMLSIER / fLib_IRDA_Disable_FMLSIER =>ok1009
// Description: Register "FMIIER=0x40"
//=========================================================================================
void fLib_IRDA_Set_FMLSIER(uint32_t port,uint32_t mode)
{
	ithWriteRegA(port + SERIAL_FMLSIER, mode);
}

void fLib_IRDA_Enable_FMLSIER(uint32_t port,uint32_t mode)
{
	//see IrDA spec P.42
	uint32_t data;

	data = ithReadRegA(port + SERIAL_FMLSIER);
	ithWriteRegA(port + SERIAL_FMLSIER, data | mode);

}
void fLib_IRDA_Disable_FMLSIER(uint32_t port,uint32_t mode)
{
	//see IrDA spec P.40
	uint32_t data;
	data = ithReadRegA(port + SERIAL_FMLSIER);
	ithWriteRegA(port + SERIAL_FMLSIER, (data&(~mode)));

}	

//=========================================================================================
// Function Name: fLib_IRDA_Read_STFF_STS =>ok1009
// Description: Register "STS=0x44"
//=========================================================================================
uint8_t fLib_IRDA_Read_STFF_STS(uint32_t port)
{
  uint32_t data;

	data = ithReadRegA(port + SERIAL_STFF_STS);
	data &= 0xFF;
	return (uint8_t)data;
}							
//=========================================================================================
// Function Name: fLib_IRDA_Read_STFF_RXLEN =>ok1009
// Description: Register "STFF_RXLENL=0x48" "STFF_RXLENH=0x4C"
//=========================================================================================
uint32_t fLib_IRDA_Read_STFF_RXLEN(uint32_t port)
{
    uint32_t wRxFrameLength,wTemp1,wTemp2;
    
    wTemp1 = ithReadRegA(port+SERIAL_STFF_RXLENL);
    wTemp2 = ithReadRegA(port+SERIAL_STFF_RXLENH);
    wRxFrameLength=wTemp1|(wTemp2<<8);
    return ((uint32_t)wRxFrameLength);
}
//=========================================================================================
// Function Name: fLib_IRDA_Read_RSR =>ok1009
// Description: Register "RSR=0x58"
//=========================================================================================
uint8_t fLib_IRDA_Read_RSR(uint32_t port)
{
  	uint32_t data;
  	
	  data = ithReadRegA(port + SERIAL_RSR);
	  data &= 0xFF;
	  return (uint8_t)data;
}

//=========================================================================================
// Function Name: fLib_IRDA_Read_RXFF_CNTR =>ok1009
// Description: Register "CNTR=0x5C"
//=========================================================================================
uint8_t fLib_IRDA_Read_RXFF_CNTR(uint32_t port)
{
  	uint32_t data;
  	
	  data = ithReadRegA(port + SERIAL_RXFF_CNTR);
    data &= 0xFF;
	  return (uint8_t)data;
}

//=========================================================================================
// Function Name: fLib_IRDA_Read_FMLSR =>ok1009
// Description: Register "FMLSR=0x50"
//=========================================================================================
uint8_t fLib_IRDA_Read_FMLSR(uint32_t port)
{
  	uint32_t data;
	
	  data = ithReadRegA(port + SERIAL_FMLSR);
	  data &= 0xFF;
	  return (uint8_t)data;
}


//=========================================================================================
// Function Name: fLib_IRDA_FIR_WRITE_CHAR
// Description: Register "SERIAL_THR=0x00"
//=========================================================================================
uint32_t fLib_IRDA_FIR_WRITE_CHAR(uint32_t port, char Ch)
{
  	uint32_t status = 0; 
    printf("[+] fLib_IRDA_FIR_WRITE_CHAR()\r\n"); 

    //do
	//{
	// 	status = ithReadRegA(port+SERIAL_FMIIR_PIO);
	//}//while (((status & SERIAL_FMIIER_PIO_TT)!=SERIAL_FMIIER_PIO_TT));	
	// while TxFifo full...do nothing

	  do
		{
		 	status = ithReadRegA(port+SERIAL_FMLSR);
		}while (((status & SERIAL_FMLSR_TX_EMPTY)!=SERIAL_FMLSR_TX_EMPTY));	

    status =ithReadRegA(port+SERIAL_RSR);

    ithWriteRegA(port + SERIAL_THR,Ch);
  
    printf("[+] fLib_IRDA_FIR_WRITE_CHAR()\r\n"); 

    return status;
}


bool fLib_IRDA_FIR_READ_CHAR(uint32_t port, char *pout)
{   
    uint32_t Ch;    
	  uint32_t status;
	  uint32_t count = 0;
	  bool state = true; 
    bool eof = 0;
	
    printf("[+] fLib_IRDA_FIR_READ_CHAR()\r\n"); 

   	do
  	{
	 	    status = ithReadRegA(port+SERIAL_FMIIR);
        if((status & SERIAL_FMIIER_PIO_EOF) == SERIAL_FMIIER_PIO_EOF)
        {
            eof = 1;
            break;
        }

	}
	while (!((status & SERIAL_FMIIER_PIO_RT)==SERIAL_FMIIER_PIO_RT));	// wait until Rx ready
	
    //overrun
    if((status & SERIAL_FMIIER_PIO_RX_OVERRUN) == SERIAL_FMIIER_PIO_RX_OVERRUN)
    {
        //Reset RX FIFO  
        fLib_IRDA_Enable_FCR(port, SERIAL_FCR_RX_FIFO_RESET);
        //Read resume
        status = ithReadRegA(port+SERIAL_RSR);
    }

	if(state == true)
	{
        for(count = 0; count < rx_trigger_level; count++)
        {
            Ch = ithReadRegA(port + SERIAL_RBR);
            *pout =  (char)(Ch);
        }
    } 

    if(eof)
    {
        count = ithReadRegA(port + SERIAL_RXFF_CNTR);
        for(; count > 0 ; count--)
        {
            Ch = ithReadRegA(port + SERIAL_RBR);
            *pout =  (char)(Ch);
        }
    }
    printf("[+] fLib_IRDA_FIR_READ_CHAR()\r\n"); 
    
    return state;
}				




//########################################################################################################





//=========================================================================================
// Function Name: fLib_IRDA_SIP_TIMER_ISR =>
// Description:
//=========================================================================================
void fLib_IRDA_SIP_TIMER_ISR(void)
{
//TODO    
#if 0   
    //100ms 
    fLib_ClearInt(IRQ_TIMER1);  
 
    if (IRDA_SIP_TimerCounter==5)
    {
        //Send SIP
        fLib_IRDA_Enable_ACR(IRDA_PORT_ForSIP,SERIAL_ACR_SEND_SIP);
        IRDA_SIP_TimerCounter=0;
    }
    else
    {
        //Count to 500 ms
        IRDA_SIP_TimerCounter++;
    }
#endif    
}
//=========================================================================================
// Function Name: fLib_IRDA_SIP_TIMER_INIT =>
// Description:
//=========================================================================================
void fLib_IRDA_SIP_TIMER_INIT(uint32_t InitPort)
{
  //Init the  Timer:Timer1 ,100ms
//TODO    
#if 0   
   fLib_Timer_Init(1,10,fLib_IRDA_SIP_TIMER_ISR);
   IRDA_PORT_ForSIP=InitPort;
#endif    
}


//=========================================================================================
// Function Name: fLib_IRDA_SIP_TIMER_DISABLE =>
// Description:
//=========================================================================================
void fLib_IRDA_SIP_TIMER_DISABLE(void)
{
//TODO    
#if 0   
    fLib_IRDA_Disable_MDR(IRDA_PORT_ForSIP,SERIAL_MDR_SIP_BY_CPU);
    fLib_Timer_Close(1);
#endif    
}

//=======================================================================================
// Function Name: fLib_SetIRTxInv
// Description:
//======================================================================================
void fLib_SetIRTxInv(uint32_t port,uint32_t enable)
{	
	  uint32_t mdr;
	
    mdr = ithReadRegA(port + SERIAL_MDR);
    if(enable == 1)
    {
    	mdr=mdr | SERIAL_MDR_IITx;
    }
    else
    {
     	mdr=mdr & (~SERIAL_MDR_IITx);   
    }	
    ithWriteRegA(port + SERIAL_MDR, mdr);
}

//=======================================================================================
// Function Name: fLib_SetIRRxInv
// Description:
//======================================================================================
void fLib_SetIRRxInv(uint32_t port,uint32_t enable)
{	
	  uint32_t mdr;
	
    mdr = ithReadRegA(port + SERIAL_MDR);
    if(enable == 1)
    {
    	mdr=mdr | SERIAL_MDR_FIRx;
    }
    else
    {
     	mdr=mdr & (~SERIAL_MDR_FIRx);   
    }	
    ithWriteRegA(port + SERIAL_MDR, mdr);


}
//************************************* Auto Mode Select *********************************
//=======================================================================================
// Function Name: fLib_IrDA_DoDelay
// Description:
//======================================================================================
uint8_t fLib_IrDA_DoDelay(void)
{	
    uint8_t  bDumy,i; 
    bDumy=0;
    for (i=1;i<100;i++)
        bDumy++;
        
    return bDumy;
}

//=======================================================================================
// Function Name: fLib_IrDA_AutoMode_SIR_Low
// Description:
//======================================================================================
void fLib_IrDA_AutoMode_SIR_Low(uint32_t port)
{	
    uint32_t mdr;

	  mdr = ithReadRegA(port + SERIAL_MCR);
	  mdr|=(SERIAL_MCR_OUT3);
	  ithWriteRegA(port + SERIAL_MCR,mdr); //Poll low ModeSelect
}

//=======================================================================================
// Function Name: fLib_IrDA_AutoMode_FIR_High
// Description:
//======================================================================================
void fLib_IrDA_AutoMode_FIR_High(uint32_t port)
{	
    uint32_t mdr;

		mdr = ithReadRegA(port + SERIAL_MCR);
		mdr&=(~SERIAL_MCR_OUT3);
		ithWriteRegA(port + SERIAL_MCR,mdr); //Poll Hight ModeSelect
}


//=================== SIR PIO Function Call==================================================
//** Lib function code
// 1.TX Init => fLib_SIR_TX_Init()
// 2.RX Init => fLib_SIR_RX_Init()
// 3.Send Data => fLib_SIR_TX_Data()
// 4.Receive Data => fLib_SIR_RX_ISR();
// 5.SIR TX RX Disable => fLib_SIR_TXRX_Close(); //disable TXRX



//=======================================================================================
// Function Name: fLib_SIR_RX_ISR
// Description:The ISR of receive data
//======================================================================================

void fLib_SIR_RX_ISR(void)
{
#if 0
	uint32_t status;		

  if(SIR_RX_Device.port==UART_BASE)   //extension chip  
  	Ext_ClearInt(SIR_RX_Device.IRQ_Num);
  else  
    fLib_ClearInt(SIR_RX_Device.IRQ_Num);
	
 	status = ithReadRegA(SIR_RX_Device.port+SERIAL_LSR);
	   
	while ((status & SERIAL_LSR_DR)==SERIAL_LSR_DR)	// wait until Rx ready
	{
    	status = ithReadRegA(SIR_RX_Device.port + SERIAL_RBR);
    	*(SIR_RX_Device.cptRXDataBuffer+SIR_RX_Device.Length) = status;
    	SIR_RX_Device.Length++;    
 		  status = ithReadRegA(SIR_RX_Device.port+SERIAL_LSR);
	}
#endif	 
}

void fLib_IRDA_Disable_DMA2(uint32_t port);
//=======================================================================================
// Function Name: fLib_SIR_TX_Init
// Description: 
// 
//======================================================================================
void fLib_SIR_TX_Init(uint32_t port, uint32_t baudrate,uint32_t SIP_PW_Value, uint32_t dwTX_Tri_Value,uint32_t bInv)
{
    uint32_t dwValue,psr_Value;
    uint32_t data;

    //power on clk
    data = ithReadRegA(ITH_APB_CLK4_REG);
    data |= (1<<11) | (1<<15);
    ithWriteRegA(ITH_APB_CLK4_REG, data);

    SIR_TX_Device.port=port;
  //1.Set DLAB=1
    ithWriteRegA(port + SERIAL_LCR, SERIAL_LCR_DLAB);	

  //2.Set PSR
	psr_Value = (uint32_t)(IRDA_CLK/PSR_CLK);
	if(psr_Value>0x1f)
	{
		psr_Value = 0x1f;
	}	
	ithWriteRegA(port + SERIAL_PSR, psr_Value);
	      
      
  //3.Set DLM/DLL  => Set baud rate 
    ithWriteRegA(port + SERIAL_DLM, ((baudrate & 0xf00) >> 8));
    ithWriteRegA(port + SERIAL_DLL, (baudrate & 0xff));
 
  //4.Set DLAB=0
    ithWriteRegA(port + SERIAL_LCR,0x00);	
  
  //5.LCR Set Character=8 / Stop_Bit=1 / Parity=Disable
    ithWriteRegA(port + SERIAL_LCR,0x03);	
  		
  //6.Set MDR->SIR  
    //UART_SetMode(port, SERIAL_MDR_SIR);

  //7.Set TX INV 
    fLib_SetIRTxInv(port,bInv);
 
  //disable DMA :silas
	  fLib_IRDA_Disable_DMA2(port);
	
  //8.Reset TX FIFO & 
    fLib_IRDA_Enable_FCR(port,SERIAL_FCR_TX_FIFO_RESET);      //Reset TX FIFO

   //9.Enable FIFO and Set Tri level of TX
    fLib_IRDA_Set_FCR(port,SERIAL_FCR_TXRX_FIFO_ENABLE);      //Enable FIFO
    fLib_IRDA_Set_FCR_Trigl_Level_2(port,dwTX_Tri_Value,0);//Set RX Trigger Level  
  
 
  //10.Enable IER(...)
   	//UART_SetInt(port,0x00);

  //11.Set ACR 1.6/316 & Enable the TX
    dwValue=0;
    dwValue|=(SIP_PW_Value<<7);        //Set SIP PW 
    dwValue|=SERIAL_ACR_TXENABLE; //Enable TX
    ithWriteRegA(port + SERIAL_ACR, dwValue);

}


//=======================================================================================
// Function Name: fLib_SIR_RX_Init
// Description: 
// 
//======================================================================================
void fLib_SIR_RX_Init(uint32_t port, uint32_t baudrate,uint32_t SIP_PW_Value, uint32_t dwRX_Tri_Value,uint32_t ReceiveDataAddr,uint32_t bInv)
{
    uint32_t  dwValue,psr_Value;

    SIR_RX_Device.port=port;
   
    // 1.Set DLAB=1
    ithWriteRegA(port + SERIAL_LCR, SERIAL_LCR_DLAB);	

    // 2.Set PSR
		psr_Value = (uint32_t)(IRDA_CLK/PSR_CLK);
		if(psr_Value>0x1f)
		{
			psr_Value = 0x1f;
		}	
		ithWriteRegA(port + SERIAL_PSR, psr_Value);
	      
      
    // 3.Set DLM/DLL  => Set baud rate 
    ithWriteRegA(port + SERIAL_DLM, ((baudrate & 0xf00) >> 8));
    ithWriteRegA(port + SERIAL_DLL, (baudrate & 0xff));
 
    // 4.Set DLAB=0
    ithWriteRegA(port + SERIAL_LCR,0x00);	
  
    //5.LCR Set Character=8 / Stop_Bit=1 / Parity=Disable
    ithWriteRegA(port + SERIAL_LCR,0x03);	
  		
    //5.Set MDR->SIR  
    //UART_SetMode(port, SERIAL_MDR_SIR);

    //6.Set TX INV 
    fLib_SetIRRxInv(port,bInv); 

		//disable DMA :silas
		fLib_IRDA_Disable_DMA2(port);

    //7.Reset RX FIFO & 
    fLib_IRDA_Enable_FCR(port,SERIAL_FCR_RX_FIFO_RESET);      //Reset RX FIFO

 
    //8.Enable FIFO and Set Tri level of RX
    fLib_IRDA_Set_FCR(port,SERIAL_FCR_TXRX_FIFO_ENABLE);      //Enable FIFO
    fLib_IRDA_Set_FCR_Trigl_Level_2(port,0,dwRX_Tri_Value);//Set RX Trigger Level  
  
    //9.Clear the RX Data Buffer 
    SIR_RX_Device.Length=0;
    SIR_RX_Device.cptRXDataBuffer=(uint8_t*)ReceiveDataAddr;
  
  
    //10.Enable IER(...)
    //UART_SetInt(port,SERIAL_IER_DR);
  
  	switch(port){
       	case UART_BASE:
       	    #if 0
       		SIR_RX_Device.IRQ_Num=IRQ_IRDA_FMLSR;
       		#endif
         	fLib_IRDA_INT_Init(SIR_RX_Device.IRQ_Num,fLib_SIR_RX_ISR);
       		break;
       	default:
       		break;
    }

    //11.Set ACR 1.6/316 & Enable the RX
    dwValue=0;
    dwValue|=(SIP_PW_Value<<7);        //Set SIP PW 
    dwValue|=SERIAL_ACR_RXENABLE; //Enable RX
    ithWriteRegA(port + SERIAL_ACR, dwValue);
}
//=======================================================================================
// Function Name: fLib_SIR_TX_Data
// Description:Transmit the data
// Input:  1.Data buffer
//         2.Data length
// Output: void 
//======================================================================================
void fLib_SIR_TX_Data(uint32_t dwport,char *bpData, uint32_t dwLength)
{
    uint32_t i;
    uint32_t dwValue=0;
   
   	for(i=0; i<dwLength; i++)
   	{
		//fLib_PutSerialChar(dwport,*(bpData+i));	    			  
    }
}

//=======================================================================================
// Function Name: fLib_SIR_TXRX_Close
// Description:
//======================================================================================
void fLib_SIR_TXRX_Close(void)
{
    //Disable the TX RX
    fLib_IRDA_Disable_ACR(SIR_RX_Device.port,SERIAL_ACR_TX_ENABLE);
    fLib_IRDA_Disable_ACR(SIR_RX_Device.port,SERIAL_ACR_RX_ENABLE);  

    fLib_IRDA_Disable_ACR(SIR_TX_Device.port,SERIAL_ACR_TX_ENABLE); 
    fLib_IRDA_Disable_ACR(SIR_TX_Device.port,SERIAL_ACR_RX_ENABLE);

    // 2.Reset TX/RX FIFO 
    fLib_IRDA_Enable_FCR(SIR_RX_Device.port,SERIAL_FCR_RX_FIFO_RESET);      //Reset RX FIFO
    fLib_IRDA_Enable_FCR(SIR_RX_Device.port,SERIAL_FCR_TX_FIFO_RESET);      //Reset TX FIFO 	

    fLib_IRDA_Enable_FCR(SIR_TX_Device.port,SERIAL_FCR_RX_FIFO_RESET);      //Reset RX FIFO
    fLib_IRDA_Enable_FCR(SIR_TX_Device.port,SERIAL_FCR_TX_FIFO_RESET);      //Reset TX FIFO 		
  
    // 3.Disable the FIFO
    fLib_IRDA_Set_FCR(SIR_RX_Device.port,SERIAL_FCR_FE);     
    fLib_IRDA_Set_FCR(SIR_TX_Device.port,SERIAL_FCR_FE);
}	

//=======================================================================================
// Function Name: fLib_SIR_TXRX_Close
// Description:
//======================================================================================
void fLib_SIR_TX_Close(void)
{
  uint32_t status;
  
  //Waiting data TX finish
         //Makesure the TX FIFO is empty
  do{
	 	status = ithReadRegA(SIR_TX_Device.port+SERIAL_LSR);
	}while (!((status & SERIAL_LSR_THRE)==SERIAL_LSR_THRE));	// wait until Tx ready                  


//Disable the TX RX
  fLib_IRDA_Disable_ACR(SIR_TX_Device.port,SERIAL_ACR_TX_ENABLE); 
  fLib_IRDA_Disable_ACR(SIR_TX_Device.port,SERIAL_ACR_RX_ENABLE);

//2.Reset TX/RX FIFO 
  fLib_IRDA_Enable_FCR(SIR_TX_Device.port,SERIAL_FCR_RX_FIFO_RESET);      //Reset RX FIFO
  fLib_IRDA_Enable_FCR(SIR_TX_Device.port,SERIAL_FCR_TX_FIFO_RESET);      //Reset TX FIFO 		
  
//3.Disable the FIFO
  fLib_IRDA_Set_FCR(SIR_TX_Device.port,SERIAL_FCR_FE);
}	
//=======================================================================================
// Function Name: fLib_SIR_TXRX_Close
// Description:
//======================================================================================
void fLib_SIR_RX_Close(void)
{

//Disable the TX RX
  fLib_IRDA_Disable_ACR(SIR_RX_Device.port,SERIAL_ACR_TX_ENABLE);
  fLib_IRDA_Disable_ACR(SIR_RX_Device.port,SERIAL_ACR_RX_ENABLE);  

//2.Reset TX/RX FIFO 
  fLib_IRDA_Enable_FCR(SIR_RX_Device.port,SERIAL_FCR_RX_FIFO_RESET);      //Reset RX FIFO
  fLib_IRDA_Enable_FCR(SIR_RX_Device.port,SERIAL_FCR_TX_FIFO_RESET);      //Reset TX FIFO 	
  
//3.Disable the FIFO
  fLib_IRDA_Set_FCR(SIR_RX_Device.port,SERIAL_FCR_FE);     
}	


//************************************ SIR DMA Function Call **********************************
//** Lib function code
// 1.TX Init => fLib_SIR_TX_Init_DMA()
// 2.RX Init => fLib_SIR_RX_Init_DMA()
// 3.Send Data => fLib_SIR_TX_Data_DMA()
// 4.SIR TX RX Disable => fLib_SIR_TXRX_Close_DMA(); //disable TXRX

//=======================================================================================
// Function Name: fLib_IRDA_Enable_DMA2
// Description: 
// 
//======================================================================================
void fLib_IRDA_Enable_DMA2(uint32_t port)
{
	//see IrDA spec P.42
	uint32_t data;

	data = ithReadRegA(port + SERIAL_MCR);
	ithWriteRegA(port + SERIAL_MCR, data | SERIAL_MCR_DMA2);

}
//=======================================================================================
// Function Name: fLib_IRDA_Disable_DMA2
// Description: 
// 
//======================================================================================
void fLib_IRDA_Disable_DMA2(uint32_t port)
{
	//see IrDA spec P.42
	uint32_t data;

	data = ithReadRegA(port + SERIAL_MCR);
	ithWriteRegA(port + SERIAL_MCR, data & (~SERIAL_MCR_DMA2));

}

//=======================================================================================
// Function Name: fLib_SIR_TX_Init_DMA
// Description: 
// 
//======================================================================================
void fLib_SIR_TX_Init_DMA(uint32_t port, uint32_t baudrate,uint32_t SIP_PW_Value, uint32_t dwTX_Tri_Value,uint32_t bInv)
{
    uint32_t  dwValue,psr_Value;
    
    SIR_TX_Device.port=port;
    // 1.Set DLAB=1
    ithWriteRegA(port + SERIAL_LCR, SERIAL_LCR_DLAB);	

    // 2.Set PSR
		psr_Value = (uint32_t)(IRDA_CLK/PSR_CLK);
		if(psr_Value>0x1f)
		{
			psr_Value = 0x1f;
		}	
		ithWriteRegA(port + SERIAL_PSR, psr_Value);
	      
      
    // 3.Set DLM/DLL  => Set baud rate 
    ithWriteRegA(port + SERIAL_DLM, ((baudrate & 0xf00) >> 8));
    ithWriteRegA(port + SERIAL_DLL, (baudrate & 0xff));
 
    // 4.Set DLAB=0
    ithWriteRegA(port + SERIAL_LCR,0x00);	
  
    //5.LCR Set Character=8 / Stop_Bit=1 / Parity=Disable
    ithWriteRegA(port + SERIAL_LCR,0x03);	
  		
    //6.Set MDR->SIR  
    //UART_SetMode(port, (SERIAL_MDR_SIR|SERIAL_MDR_SIP_BY_CPU|SERIAL_MDR_DMA_ENABLE));	// modify by jerry
    //UART_SetMode(port, SERIAL_MDR_SIR);

    //7.Set TX INV 
    fLib_SetIRTxInv(port,bInv);
 
    //8.Reset TX FIFO & 
    fLib_IRDA_Enable_FCR(port,SERIAL_FCR_TX_FIFO_RESET);      //Reset TX FIFO

    //9.Enable FIFO and Set Tri level of TX
    // modify by jerry
    //fLib_IRDA_Set_FCR(port,SERIAL_FCR_TXRX_FIFO_ENABLE);      //Enable FIFO          => Enable in the last step
    fLib_IRDA_Set_FCR_Trigl_Level_2(port,dwTX_Tri_Value,0);//Set RX Trigger Level  
  
  
    //10.Enable IER(...)
   	//UART_SetInt(port,0x00);

    //11.Set ACR 1.6/316 & Enable the TX
    dwValue=0;
    dwValue|=(SIP_PW_Value<<7);      //Set SIP PW
    //dwValue |=(dwTX_Tri_Value<<5);//STFF_TRGL	// modify by jerry
    dwValue|=SERIAL_ACR_TXENABLE; //Enable TX
    ithWriteRegA(port + SERIAL_ACR, dwValue);
}


//=======================================================================================
// Function Name: fLib_SIR_RX_Init_DMA
// Description: 
// 
//======================================================================================
void fLib_SIR_RX_Init_DMA(uint32_t port, uint32_t baudrate,uint32_t SIP_PW_Value, uint32_t dwRX_Tri_Value,uint32_t ReceiveDataAddr,uint32_t bInv,uint32_t dwReceiveDataLength, uint32_t dmatype)
{
    uint32_t  psr_Value;

    SIR_RX_Device.port=port;
    
    // 1.Set DLAB=1
    ithWriteRegA(port + SERIAL_LCR, SERIAL_LCR_DLAB);	

    // 2.Set PSR
		psr_Value = (uint32_t)(IRDA_CLK/PSR_CLK);
		if(psr_Value>0x1f)
		{
			psr_Value = 0x1f;
		}	
		ithWriteRegA(port + SERIAL_PSR, psr_Value);
	      
      
    // 3.Set DLM/DLL  => Set baud rate 
    ithWriteRegA(port + SERIAL_DLM, ((baudrate & 0xf00) >> 8));
    ithWriteRegA(port + SERIAL_DLL, (baudrate & 0xff));
 
    // 4.Set DLAB=0
    ithWriteRegA(port + SERIAL_LCR,0x00);	
  
    //5.LCR Set Character=8 / Stop_Bit=1 / Parity=Disable
    ithWriteRegA(port + SERIAL_LCR,0x03);	
  		
    //5.Set MDR->SIR  
    //UART_SetMode(port, (SERIAL_MDR_SIR|SERIAL_MDR_SIP_BY_CPU|SERIAL_MDR_DMA_ENABLE));
    //UART_SetMode(port,SERIAL_MDR_SIR);

    //6.Set TX INV 
    fLib_SetIRRxInv(port,bInv); 

    //7.Reset RX FIFO & 
    fLib_IRDA_Enable_FCR(port,SERIAL_FCR_RX_FIFO_RESET);      //Reset RX FIFO

 
    //8.Enable FIFO and Set Tri level of RX
    fLib_IRDA_Set_FCR(port,SERIAL_FCR_TXRX_FIFO_ENABLE);      //Enable FIFO
    fLib_IRDA_Set_FCR_Trigl_Level_2(port,0,dwRX_Tri_Value);//Set RX Trigger Level  
          
    //9.Clear the RX Data Buffer 
    SIR_RX_Device.Length=0;
    SIR_RX_Device.cptRXDataBuffer=(uint8_t*)ReceiveDataAddr;
  
    //10.Enable IER(...)
    //UART_SetInt(port,SERIAL_IER_DR);
    //UART_SetInt(port,0); // modify by jerry
}
//=======================================================================================
// Function Name: fLib_SIR_TX_Data_DMA
// Description:Transmit the data
// Input:  1.Data buffer
//         2.Data length
// Output: void 
//======================================================================================
void fLib_SIR_TX_Data_DMA(uint32_t dwport,char *bpData, uint32_t dwLength, uint8_t dmatype)
{
   
    fLib_IRDA_Enable_DMA2(dwport); 
    fLib_IRDA_Set_FCR(dwport,SERIAL_FCR_TXRX_FIFO_ENABLE);      //Enable FIFO
    //-------------------------------------------------------------------------------
}

//=======================================================================================
// Function Name: fLib_SIR_TXRX_Close_DMA
// Description:
//======================================================================================
void fLib_SIR_TXRX_Close_DMA(void)
{
#if 0
// 1.Disable the TX RX
  fLib_IRDA_Disable_ACR(SIR_RX_Device.port,SERIAL_ACR_TX_ENABLE);
  fLib_IRDA_Disable_ACR(SIR_RX_Device.port,SERIAL_ACR_RX_ENABLE);  

  fLib_IRDA_Disable_ACR(SIR_TX_Device.port,SERIAL_ACR_TX_ENABLE); 
  fLib_IRDA_Disable_ACR(SIR_TX_Device.port,SERIAL_ACR_RX_ENABLE);

// 2.Disable the FIFO
  fLib_IRDA_Set_FCR(SIR_RX_Device.port,SERIAL_FCR_FE);     
  fLib_IRDA_Set_FCR(SIR_TX_Device.port,SERIAL_FCR_FE);

// 3.Disable DMA
  fLib_IRDA_Disable_DMA2(SIR_RX_Device.port);
  fLib_IRDA_Disable_DMA2(SIR_TX_Device.port);  

// 4.Disable the APB DMA Enable  
  fLib_APBDMA_EnableTrans( AHB2APB_DMA_A, false); 
  fLib_APBDMA_EnableTrans( AHB2APB_DMA_B, false);
#endif   
}	

int32_t mmpSirInitialize(IR_PARAMEMTER *parameter)
{
    int32_t result = 0;
    IR_PARAMEMTER *param = parameter;
    
    uint32_t baudrate = param->baudrate;
    uint32_t sip_pw_value = param->sip_pw_value;
    uint32_t bTxInv = param->bTxInv;
    uint32_t bRxInv = param->bRxInv;
    uint32_t dwTX_Tri_Value = 2;
    uint32_t dwRX_Tri_Value = 2;
    uint32_t dwValue,psr_Value;
    uint32_t data;
    uint32_t port = UART_BASE;
    uint32_t int_div = 0;
    uint32_t total_div = 0;
    uint32_t f_div = 0;
    
    //power on clk
    data = ithReadRegA(ITH_APB_CLK4_REG);
    data |= (0x1<<5); //TODO
    ithWriteRegA(ITH_APB_CLK4_REG, data);
    
    //irda padsel
	  SetPadSel(port);

    SIR_TX_Device.port=port;
    SIR_RX_Device.port=port;
    
    fLib_IRDA_Disable_ACR(port, SERIAL_ACR_RX_ENABLE | SERIAL_ACR_TX_ENABLE);
  //1.Set DLAB=1
    ithWriteRegA(port + SERIAL_LCR, SERIAL_LCR_DLAB);	

  //2.Set PSR
	psr_Value = (uint32_t)(IRDA_CLK/PSR_CLK);
	if(IRDA_CLK%PSR_CLK)
		psr_Value++;
	if(psr_Value>0x1f)
	{
		psr_Value = 0x1f;
	}	
	
	ithWriteRegA(port + SERIAL_PSR, psr_Value);
    
    total_div = PSR_CLK/baudrate;
    int_div = total_div >> 4;
    f_div = total_div & 0xF;

  //3.Set DLM/DLL  => Set baud rate 
    ithWriteRegA(port + SERIAL_DLM, ((int_div & 0xf00) >> 8));
    ithWriteRegA(port + SERIAL_DLL, (int_div & 0xff));
    ithWriteRegA(port + SERIAL_DLH, (f_div & 0xf));

  //4.Set DLAB=0
    ithWriteRegA(port + SERIAL_LCR,0x00);	

  //5.LCR Set Character=8 / Stop_Bit=1 / Parity=Disable
    ithWriteRegA(port + SERIAL_LCR,0x03);	
  		
  //6.Set MDR->SIR  
    fLib_IRDA_Disable_MDR(port, SERIAL_MDR_FIR);
    fLib_IRDA_Enable_MDR(port, SERIAL_MDR_SIR);

  //7.Set TX INV 
    fLib_SetIRTxInv(port,bTxInv);
    //6.Set RX INV 
    fLib_SetIRRxInv(port,bRxInv); 
  //disable DMA :silas
	fLib_IRDA_Disable_DMA2(port);

  //8.Reset TX FIFO & RX FIFO
    fLib_IRDA_Enable_FCR(port,SERIAL_FCR_TX_FIFO_RESET);      //Reset TX FIFO
    fLib_IRDA_Enable_FCR(port,SERIAL_FCR_RX_FIFO_RESET);      //Reset RX FIFO

   //9.Enable FIFO and Set Tri level of TX
    fLib_IRDA_Set_FCR(port,SERIAL_FCR_TXRX_FIFO_ENABLE);      //Enable FIFO
    fLib_IRDA_Set_FCR_Trigl_Level_2(port,dwTX_Tri_Value,dwRX_Tri_Value);//Set RX Trigger Level  

  //10.Enable IER(...)
  	ithWriteRegA(port + SERIAL_IER, SERIAL_IER_DR);

  //11.Set ACR 1.6/316 & Enable the TX
    dwValue=0;
    dwValue|=(sip_pw_value<<7);        //Set SIP PW 
    dwValue|=SERIAL_ACR_RXENABLE; //Enable RX
    ithWriteRegA(port + SERIAL_ACR, dwValue);
    
	return result;
}

int32_t mmpSirTerminate(
    void)
{
    int32_t result = 0;
    uint32_t port = UART_BASE;

    fLib_IRDA_Disable_ACR(port, SERIAL_ACR_RX_ENABLE | SERIAL_ACR_TX_ENABLE);

    return result;
}

int32_t mmpSirSendStart(void)
{
    int32_t result = 0;
    uint32_t dwValue; 
 
    dwValue = ithReadRegA(SIR_RX_Device.port + SERIAL_ACR);
    //dwValue &= ~SERIAL_ACR_RXENABLE; //Disnable RX
    dwValue |= SERIAL_ACR_TXENABLE; //Enable TX
    ithWriteRegA(SIR_RX_Device.port + SERIAL_ACR, dwValue); 
    
	return result;
}

int32_t mmpSirSendData(uint8_t *buffer, uint32_t size)
{
    int32_t result = 0;
    uint32_t i;
    uint32_t status;

   	for(i=0; i<size; i++)
   	{
        do
    	{
    	 	  status = ithReadRegA(SIR_RX_Device.port + SERIAL_LSR);
    	}while (!((status & SERIAL_LSR_THRE)==SERIAL_LSR_THRE));	// wait until Tx ready	
    	   
        ithWriteRegA(SIR_RX_Device.port + SERIAL_THR, *(buffer+i));
    }
	return result;
}

int32_t mmpSirSendByte(uint8_t ch)
{
    int32_t result = 0;
    uint32_t i = 0;
    uint32_t status; 
 
	  do
		{
		 	status = ithReadRegA(SIR_RX_Device.port + SERIAL_LSR);
		}while (!((status & SERIAL_LSR_THRE)==SERIAL_LSR_THRE));	// wait until Tx ready	
	
	  ithWriteRegA(SIR_RX_Device.port + SERIAL_THR, ch);
    
	  return result;
}

int32_t mmpSirSendClose(void)
{
    int32_t result = 0;
    uint32_t status, dwValue; 
    do
		{
		 	status = ithReadRegA(SIR_RX_Device.port + SERIAL_LSR);
		}while (!((status & SERIAL_LSR_TE)==SERIAL_LSR_TE));	// wait until Tx Empty	
	
    dwValue = ithReadRegA(SIR_RX_Device.port + SERIAL_ACR);
    //dwValue &= ~SERIAL_ACR_TXENABLE; //Disnable TX
    dwValue |= SERIAL_ACR_RXENABLE; //Enable RX
    ithWriteRegA(SIR_RX_Device.port + SERIAL_ACR, dwValue); 
    
    return result;
}
	
int32_t mmpSirReceiveData(uint8_t *buffer, uint32_t size)
{
    int32_t result = ERROR_SIR_RX_TIMEPUT;
    uint32_t data;
    uint32_t timeOut;   
    struct timeval t1, t2;
    double elapsedTime;
    
    /* Start timer */
    gettimeofday(&t1, NULL);

    SIR_RX_Device.Length = 0;
    timeOut = 100*size; 
    if(timeOut > 5000)
        timeOut = 5000;
        
  gettimeofday(&t2, NULL);
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms

	while(elapsedTime < timeOut)	
	{
      data = ithReadRegA(SIR_RX_Device.port+SERIAL_LSR); // wait until Rx ready
	    if((data & SERIAL_LSR_DR) == SERIAL_LSR_DR) 
	    {
        	data = ithReadRegA(SIR_RX_Device.port + SERIAL_RBR);
        	*(buffer + SIR_RX_Device.Length) = (uint8_t)data;
        	if(++SIR_RX_Device.Length >= size)
	     		{
	     		    result = 0;
	     		    break;
	     		} 
     	}
     	
     	gettimeofday(&t2, NULL);
		  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
		  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
	}
	return result;
}

int32_t mmpSirReceiveByte(uint8_t *ch)
{
    int32_t result = ERROR_SIR_RX_TIMEPUT;
    uint32_t data;

		data = ithReadRegA(SIR_RX_Device.port+SERIAL_LSR); // wait until Rx ready
		if((data & SERIAL_LSR_DR) == SERIAL_LSR_DR) 
		{
			data = ithReadRegA(SIR_RX_Device.port + SERIAL_RBR);
			*ch = (uint8_t)data;
			result = 0;
		}


	return result;
}
   
int32_t mmpFirInitialize(IR_PARAMEMTER *parameter)
{
    int32_t result = 0;
    uint32_t psr_Value;
    uint32_t port = UART_BASE;
	  uint32_t data = 0;
    
    IrDA_Mode_Sel.bSetEOTMethodEnable = 0;
    IrDA_Mode_Sel.bForceAbortEnable = 0;
    IrDA_Mode_Sel.SIPSentByCPU = 0;
    IrDA_Mode_Sel.PreambleLength = 0;
    IrDA_Mode_Sel.InvertedPulseEnable_TX = parameter->bTxInv;
    IrDA_Mode_Sel.InvertedPulseEnable_RX = parameter->bRxInv;
    IrDA_Mode_Sel.FIRDMAEnable = 0;
	  SIR_TX_Device.port=port;
	
		//power on clk
		data = ithReadRegA(ITH_APB_CLK4_REG);
		data |= (1<< 15); //TODO
		ithWriteRegA(ITH_APB_CLK4_REG, data);
		
		//irda padsel
	  SetPadSel(port);
    
    //1.Set DLAB=1
    ithWriteRegA(port + SERIAL_LCR, SERIAL_LCR_DLAB);	

    //2.Set PSR, set to 8M Hz clock
		psr_Value = 6;
		ithWriteRegA(port + SERIAL_PSR, psr_Value);

    //3.Set DLM/DLL  => Set baud rate 
    ithWriteRegA(port + SERIAL_DLM, 0);
    ithWriteRegA(port + SERIAL_DLL, 0);

    //4.Set DLAB=0
    ithWriteRegA(port + SERIAL_LCR,0x00);	
          
  	dmaChannel = ithDmaRequestCh("dma_irda", ITH_DMA_CH_PRIO_HIGH_3, NULL, NULL);

		ithDmaReset(dmaChannel);    
        
    return result;
}

uint32_t fLib_FIR_ReadData_DMA(uint8_t *data, uint32_t size)
{
    uint32_t result = 0;
   
    ithDmaSetSrcAddr(dmaChannel, (uint32_t)(UART_AHB_BASE + SERIAL_THR));
    ithDmaSetDstAddr(dmaChannel, (uint32_t)data);
    ithDmaSetRequest(dmaChannel, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_UART_FIR, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
    ithDmaSetSrcParams(dmaChannel, 1, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
		ithDmaSetDstParams(dmaChannel, 1, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
	  ithDmaSetTxSize(dmaChannel, size);
	  ithDmaSetBurst(dmaChannel, 4);
	  ithDmaStart(dmaChannel);
        
    return result;

}

uint32_t fLib_FIR_WriteData_DMA(uint8_t *data, uint32_t size)
{
    uint32_t result = 0;
        
    ithDmaSetSrcAddr(dmaChannel, (uint32_t)data);
    ithDmaSetDstAddr(dmaChannel, (uint32_t)(UART_AHB_BASE + SERIAL_THR));
    ithDmaSetRequest(dmaChannel, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_UART_TX);    
    ithDmaSetSrcParams(dmaChannel, 4, ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
		ithDmaSetDstParams(dmaChannel, 1, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
	  ithDmaSetTxSize(dmaChannel, size);
	  ithDmaSetBurst(dmaChannel, 1); 
  	ithDmaStart(dmaChannel); 
        
    return result;
}

uint32_t mmpFirWaitDMAIdle(void)
{
  uint32_t timeout_ms = 3000;
	
	while(ithDmaIsBusy(dmaChannel) && --timeout_ms)
	{
        usleep(1000);
    }

    if( !timeout_ms )
    {
    	/* DMA fail */
    	return 0;
    }

    return 1;
}

static void
FIR_RX_DMA_Close(uint32_t port)
{

// 2.Disable the RX_Enable
    fLib_IRDA_Disable_ACR(port,SERIAL_ACR_RX_ENABLE);
    //TODO
    //mmpDmaAbort(dmaCtxt);

    // 1.Reset the RX FIFO
    //fLib_IRDA_Enable_FCR(port,SERIAL_FCR_RX_FIFO_RESET);      //Reset RX FIFO
    //fLib_IRDA_Enable_FCR(port,SERIAL_FCR_TX_FIFO_RESET);      //Reset TX FIFO
  
    //fLib_IRDA_Set_FMLSIER(port,0);
    //fLib_IRDA_Set_FMIIER(port,0);

    //8.Clear 0x28/0x2C
    //fLib_IRDA_Set_MaxLen(port, 0);
}
int32_t mmpFirTerminate(
    void)
{
    int32_t result = 0;
    
    FIR_RX_DMA_Close(UART_BASE);
        
    if(ReceiveData.bpRXDMABuffer)
    {
        itpVmemFree((uint32_t)ReceiveData.bpRXDMABuffer);
        ReceiveData.bpRXDMABuffer = 0;
    }
    
    ithDmaFreeCh(dmaChannel);
    
    return result;
}

static void
InitialRxParam(uint8_t *buffer, uint32_t size, uint16_t wEqualizeSize)
{
    ReceiveData.bpRXDataBuffer=buffer;
	ReceiveData.RX_Receive_Frame_Counter=0;
	ReceiveData.ReceivedBytes=0;
	ReceiveData.ReceiveErrorType_FMIIR=0;	
	ReceiveData.ReceiveErrorType_FMLSR=0;		
	ReceiveData.ReceiveStatusFIFO_Number=0;
    ReceiveData.RX_Merge_Frame_Counter=0;
    if((size % wEqualizeSize) == 0)
	{
	    ReceiveData.RX_Expect_Frame_Numbers = (uint16_t)(size / wEqualizeSize);	
	}
	else
	{
	    if(size & 0x1)
   	        ReceiveData.RX_Expect_Frame_Numbers = (uint16_t)(size / wEqualizeSize + 2) ;
	    else    
	        ReceiveData.RX_Expect_Frame_Numbers = (uint16_t)(size / wEqualizeSize + 1) ;
	}
		
	ReceiveData.ExpectBytes = size+(ReceiveData.RX_Expect_Frame_Numbers);
	ReceiveData.dmaSize = ReceiveData.ExpectBytes;

    IrDA_RX_Status.PSR=SERIAL_PSR_FIR_FIX_VALUE;     
    IrDA_RX_Status.PLR=0;
    IrDA_RX_Status.ST_FIFO_Trigger_Level=SERIAL_ACR_STFF_TRGL_1;
          
    // 1.Set the MDR to FIR Mode  
    IrDA_RX_Status.MDR=SERIAL_MDR_FIR_MODE | SERIAL_MDR_DMA_ENABLE;
    if(IrDA_Mode_Sel.InvertedPulseEnable_RX)
        IrDA_RX_Status.MDR |= SERIAL_MDR_RX_INV;
 
    //2.Program MRXLENL+MRXLENH
    IrDA_RX_Status.MRXLENHL=SERIAL_MRXLENHL_DEFAULT;  
    // 3.Set the RX FIFO level 
    IrDA_RX_Status.RX_FIFO_Trigger_Level=SERIAL_FCR_RX_TRI2;  
    // 4.FMIIR for the RX
    IrDA_RX_Status.RX_FMIIER=SERIAL_FMIIR_DMA_STFIFO_TRIG|SERIAL_FMIIR_DMA_STFIFO_TIMEOUT|SERIAL_FMIIR_DMA_STFIFO_ORUN;	
 
    //5.FMLSIER for the RX
    IrDA_RX_Status.RX_FMLSIER=SERIAL_FMLSIER_CRC_ERROR|SERIAL_FMLSIER_PHY_ERROR|SERIAL_FMLSIER_SIZE_ERR|SERIAL_FMLSIER_STFIFO_FULL;    		
    IrDA_RX_Status.RX_PORT_ADDRESS = UART_BASE;
}

static void
InitialTxParam(uint8_t *buffer, uint32_t size, uint16_t wEqualizeSize)
{
    uint32_t bCounter;
    uint32_t wLastFrameByteNumber;
    uint32_t wTXLENHL;
    
    SendData.bpTXDataBuffer=buffer;
    SendData.RemainByte=size;
    SendData.TotalByte=size;  
    SendData.TX_Expect_Frame_Numbers = (uint16_t)(size/wEqualizeSize);       
    SendData.Send_FRM_SENT_OK_Counter=0;
    SendData.SendErrorType=0;
    SendData.SendByte = 0;
    
    if(IrDA_Mode_Sel.bSetEOTMethodEnable==0)      
    {
        //====>Frame Length Method
        IrDA_TX_Status.SET_EOT_FRAME_LENGTH=0;
        IrDA_TX_Status.SET_EOT_LAST_FRAME_LENGTH=0;
        bCounter=size/wEqualizeSize;
        if(bCounter==0)
        {
            wLastFrameByteNumber=0;	
            wTXLENHL=size;
            IrDA_TX_Status.LST_FrameNumber=0;
            SendData.TX_Expect_Frame_Numbers=1;
            ReceiveData.RX_Expect_Frame_Numbers=1;           
        }
        else
        {
            wLastFrameByteNumber=size%wEqualizeSize;
            wTXLENHL=wEqualizeSize;
            if(wLastFrameByteNumber==0)
            {//Equal-Size Case
                IrDA_TX_Status.LST_FrameNumber=0;	
                SendData.TX_Expect_Frame_Numbers=(uint16_t)(bCounter);       
				//TODO
                ReceiveData.RX_Expect_Frame_Numbers = (uint16_t)bCounter;    
            }
            else
            {//Non-Equal-Size Case
                IrDA_TX_Status.LST_FrameNumber = (uint8_t)bCounter;
                SendData.TX_Expect_Frame_Numbers=1;                      
                ReceiveData.RX_Expect_Frame_Numbers= (uint16_t)(bCounter+1);                               
            }
        } 	
     }
     else
     {  //====>SET EOT Method
        bCounter=size/wEqualizeSize; 
        //Set Expect Frames Numbers
        SendData.TX_Expect_Frame_Numbers=(uint16_t)bCounter;                      
        ReceiveData.RX_Expect_Frame_Numbers=(uint16_t)bCounter;     	
        
        //Set Frame Length of the SET EOT Method
        IrDA_TX_Status.SET_EOT_FRAME_LENGTH=wEqualizeSize;
        
        //Clear the Frame Length Method register
        wLastFrameByteNumber=0;
        wTXLENHL=0;
        IrDA_TX_Status.LST_FrameNumber=0;	
     }	

    SendData.TX_1Frame_Bytes=wEqualizeSize;          
    //2.Fill the TX configuration status
    IrDA_TX_Status.PSR=SERIAL_PSR_FIR_FIX_VALUE;
    IrDA_TX_Status.DLL=1;
    IrDA_TX_Status.DLM=0;
	
	//Set MDR
    IrDA_TX_Status.MDR=SERIAL_MDR_FIR_MODE|(IrDA_Mode_Sel.SIPSentByCPU)
               |(IrDA_Mode_Sel.InvertedPulseEnable_TX)|(IrDA_Mode_Sel.FIRDMAEnable);	                   
                   
	
    if (IrDA_Mode_Sel.bSetEOTMethodEnable==1)
         IrDA_TX_Status.MDR|=SERIAL_MDR_FMEND_ENABLE;
    
    IrDA_TX_Status.PLR=IrDA_Mode_Sel.PreambleLength;
    IrDA_TX_Status.TXLENHL=wTXLENHL;
    IrDA_TX_Status.LSTFMLENHL=wLastFrameByteNumber;
    
    IrDA_TX_Status.TX_FIFO_Trigger_Level=SERIAL_FCR_TX_TRI3;
    
    if (IrDA_Mode_Sel.FIRDMAEnable==SERIAL_MDR_DMA_ENABLE)	
        IrDA_TX_Status.TX_FMIIER=SERIAL_FMIIR_DMA_FRM_SENT|SERIAL_FMIIR_DMA_TXFIFO_URUN;	
    else
        IrDA_TX_Status.TX_FMIIER=SERIAL_FMIIER_PIO_FRM_SENT|SERIAL_FMIIER_PIO_TXFIFO_URUN;	
        
    IrDA_TX_Status.TX_FMLSIER=SERIAL_FMLSIER_TX_EMPTY|SERIAL_FMLSIER_SIZE_ERR|SERIAL_FMLSIER_PHY_ERROR|SERIAL_FMLSIER_CRC_ERROR;
    IrDA_TX_Status.TX_PORT_ADDRESS=UART_BASE;

}
static uint32_t
IRDA_ISR_TX_FMIIR_DMA(void)
{
    uint8_t ISRstatus = 0;
    uint32_t result = 0;
    // 1.Read the FMIIR
    ISRstatus=fLib_IRDA_Read_FMIIR(IrDA_TX_Status.TX_PORT_ADDRESS);
    // 2.Check FRM_SENT&TXFIFO_URUN
    if (ISRstatus&SERIAL_FMIIR_DMA_FRM_SENT)
    {
        SendData.Send_FRM_SENT_OK_Counter++;
        SendData.SendByte += SendData.TX_1Frame_Bytes;
        //printf("SendByte = %d \r\n", SendData.SendByte);
    }
    if(ISRstatus&SERIAL_FMIIR_DMA_TXFIFO_URUN)
    {
        SendData.SendErrorType=ERROR_FIR_TX_UNDERRUN;
    }
    if(SendData.TX_Expect_Frame_Numbers == SendData.Send_FRM_SENT_OK_Counter)
    {
        result = 1;
    }
            
    return result;
}

static void
IRDA_ISR_TX_FMLSR_DMA(void)
{
    uint8_t ISRstatus;
// 1.Read the FMLSR
    ISRstatus=fLib_IRDA_Read_FMLSR(IrDA_TX_Status.TX_PORT_ADDRESS);
    if(ISRstatus&SERIAL_FMLSR_RXFIFO_EMPTY)
        ;
    if(ISRstatus&SERIAL_FMLSR_STFIFO_EMPTY)
        ;
    if(ISRstatus&SERIAL_FMLSR_CRC_ERROR)
        printf("SERIAL_FMLSR_CRC_ERROR\r\n");
    if(ISRstatus&SERIAL_FMLSR_PHY_ERROR)
        printf("SERIAL_FMLSR_PHY_ERROR\r\n");
    if(ISRstatus&SERIAL_FMLSR_SIZE_ERR)
        printf("SERIAL_FMLSR_SIZE_ERR\r\n");
    if(ISRstatus&SERIAL_FMLSR_STFIFO_FULL)
        printf("SERIAL_FMLSR_STFIFO_FULL\r\n");

}
static uint32_t
FirDMATx(
    void)
{
    uint32_t result = 0;
    uint32_t port;
    
    port=IrDA_TX_Status.TX_PORT_ADDRESS;

    fLib_IRDA_Disable_ACR(port, SERIAL_ACR_RX_ENABLE | SERIAL_ACR_TX_ENABLE);

    // 1.Set PSR/DLL/DLM (When DLAB=1)
    fLib_SetFIRPrescal(port,IrDA_TX_Status.PSR); //Set PSR & DLL & DLM
    
    // 2.Set MDR
    fLib_IRDA_Set_MDR(port,IrDA_TX_Status.MDR);
    
    // 3.Set PLR
    fLib_IRDA_Set_PLR(port,IrDA_TX_Status.PLR);
    
    // 4.Set TXLEN/LSTFMLEN
    fLib_IRDA_Set_TXLEN_LSTFMLEN(port,IrDA_TX_Status.TXLENHL,IrDA_TX_Status.LSTFMLENHL,IrDA_TX_Status.LST_FrameNumber);
    
    //5.Set FCR (Reset FIFO/Select FIFO Trigger level)
    fLib_IRDA_Set_FCR(port,SERIAL_FCR_TXRX_FIFO_ENABLE);      //Enable FIFO
    fLib_IRDA_Enable_FCR(port,SERIAL_FCR_TX_FIFO_RESET);      //Reset TX FIFO
    fLib_IRDA_Set_FCR_Trigl_Level(port,IrDA_TX_Status.TX_FIFO_Trigger_Level,0);//Set RX Trigger Level
    
    fLib_FIR_WriteData_DMA(SendData.bpTXDataBuffer, SendData.TotalByte);
    
    //7.Program the FMIIER
    fLib_IRDA_Enable_FMIIER(port,IrDA_TX_Status.TX_FMIIER);
    
    //8.Write FMLSIER
    fLib_IRDA_Set_FMLSIER(port,IrDA_TX_Status.TX_FMLSIER);
    
    //9.Tx_Enable
    fLib_IRDA_Enable_ACR(port,SERIAL_ACR_TX_ENABLE | SERIAL_ACR_RX_ENABLE); //TODO
	  fLib_IRDA_Enable_ACR(port,SERIAL_ACR_TX_ENABLE);

    return result;
}    

static uint32_t
FirPIOTx(
    void)
{
    uint32_t port;	
    uint16_t bSendContinueByte,i;
    uint8_t  bFMIIRstatus;

    port=IrDA_TX_Status.TX_PORT_ADDRESS;
  
    //0.Tx_Disable
    fLib_IRDA_Disable_ACR(port, SERIAL_ACR_RX_ENABLE | SERIAL_ACR_TX_ENABLE);

    //1.Set PSR/DLL/DLM (When DLAB=1)
    fLib_SetFIRPrescal(port,IrDA_TX_Status.PSR); //Set PSR & DLL & DLM
    //2.Set MDR
    fLib_IRDA_Set_MDR(port,IrDA_TX_Status.MDR);
    //3.Set PLR
    fLib_IRDA_Set_PLR(port,IrDA_TX_Status.PLR);
    //4.Set TXLEN/LSTFMLEN
    fLib_IRDA_Set_TXLEN_LSTFMLEN(port,IrDA_TX_Status.TXLENHL,IrDA_TX_Status.LSTFMLENHL,IrDA_TX_Status.LST_FrameNumber);
    //5.Set FCR (Reset FIFO/Select FIFO Trigger level)
    fLib_IRDA_Set_FCR(port,SERIAL_FCR_TXRX_FIFO_ENABLE);      //Enable FIFO
    fLib_IRDA_Enable_FCR(port,SERIAL_FCR_TX_FIFO_RESET);      //Reset TX FIFO
    fLib_IRDA_Set_FCR_Trigl_Level(port,IrDA_TX_Status.TX_FIFO_Trigger_Level,0);//Set RX Trigger Level
    //6.Write a part of the frame =>Write 16 bytes to the Tx FIFO
    i=0;
    while((SendData.RemainByte>0)&&(i<IrDA_TX_Status.TX_FIFO_Trigger_Level))//silas
    {				
        ithWriteRegA((UART_BASE + SERIAL_THR),*(SendData.bpTXDataBuffer++));
        SendData.RemainByte--;
        i++;
    };

    //7.Program the FMIIER
    fLib_IRDA_Set_FMIIER(port,IrDA_TX_Status.TX_FMIIER);
    //8.Write FMLSIER
    fLib_IRDA_Set_FMLSIER(port,IrDA_TX_Status.TX_FMLSIER);
    
    //9.Tx_Enable
    //fLib_IRDA_Disable_ACR(port, SERIAL_ACR_RX_ENABLE);//TODO
    //fLib_IRDA_Enable_ACR(port,SERIAL_ACR_TX_ENABLE);
	  fLib_IRDA_Enable_ACR(port,SERIAL_ACR_TX_ENABLE | SERIAL_ACR_RX_ENABLE);
    //======> Frame Length Method
    while(SendData.RemainByte>0) 
    {
        bFMIIRstatus=fLib_IRDA_Read_FMIIR(IrDA_TX_Status.TX_PORT_ADDRESS);
        if(bFMIIRstatus&SERIAL_FMIIR_PIO_TXFIFO_TRIG)
        {
            if(SendData.RemainByte>(FIFO_SIZE-IrDA_TX_Status.TX_FIFO_Trigger_Level))	
              bSendContinueByte=(FIFO_SIZE-IrDA_TX_Status.TX_FIFO_Trigger_Level);
            else
                bSendContinueByte= (uint16_t)(SendData.RemainByte);
            
            for (i=0;i<bSendContinueByte;i++)
			      {
            	ithWriteRegA((UART_BASE + SERIAL_THR),*(SendData.bpTXDataBuffer++));
			      }
            SendData.RemainByte=SendData.RemainByte-bSendContinueByte;
        }
        if (bFMIIRstatus&SERIAL_FMIIR_PIO_FRM_SENT)
        {
             SendData.Send_FRM_SENT_OK_Counter++;
             SendData.SendByte += SendData.TX_1Frame_Bytes;
             printf("SendByte = %d \r\n", SendData.SendByte);
             printf("@@@ Frame_Sent count = %d \n" ,SendData.Send_FRM_SENT_OK_Counter);
        }
        if(bFMIIRstatus&SERIAL_FMIIR_PIO_TXFIFO_URUN)
        {
            SendData.SendErrorType=ERROR_FIR_TX_UNDERRUN;       
            break;
        }
    }
#if 0    
    while((SendData.Send_FRM_SENT_OK_Counter<(SendData.TX_Expect_Frame_Numbers))&(SendData.SendErrorType==0))
	{
		bFMIIRstatus=(uint8_t)fLib_IRDA_Read_FMIIR(IrDA_TX_Status.TX_PORT_ADDRESS);

		if (bFMIIRstatus&SERIAL_FMIIR_PIO_FRM_SENT)
		{
            SendData.SendByte += SendData.TX_1Frame_Bytes;
			SendData.Send_FRM_SENT_OK_Counter++;
		}
		if (bFMIIRstatus&SERIAL_FMIIR_PIO_TXFIFO_URUN)
		   	SendData.SendErrorType=ERROR_FIR_TX_UNDERRUN;
	}
	if (SendData.Send_FRM_SENT_OK_Counter==(SendData.TX_Expect_Frame_Numbers))
	{
	 	printf("@@@ %d Frame_Sent detected ok! \n" ,(SendData.TX_Expect_Frame_Numbers) );
	}
#endif	
	return SendData.SendErrorType;
	
} 
   
static uint32_t
IRDA_ISR_RX_FMIIR_DMA(void)
{   
    int16_t result = 0;
	  uint8_t ISRstatus,bSTFF_STS;	
    uint32_t data;
    uint32_t i, k;
    uint32_t frameSize = 0;
    uint32_t offset = 0;
    uint32_t offset2 = 0;
    struct timeval t1, t2;
    double elapsedTime;


	// 1.Read FMIIR Status => 1.Tri level interrupt / 2.Over run / 3.EOF detected
    ISRstatus=fLib_IRDA_Read_FMIIR(IrDA_RX_Status.RX_PORT_ADDRESS);
	// 2.Get the character	
    while ((ISRstatus&SERIAL_FMIIR_DMA_RX_EVENT)>0)	
    {
        //Int-1:Check Over Run
        if (ISRstatus&SERIAL_FMIIR_DMA_STFIFO_ORUN)
        {
            printf("Status FIFO Over Run => Fail \n");
            ReceiveData.ReceiveErrorType_FMIIR=ERROR_FIR_RX_ERROR_OVERRUN;
            break;
            //???? Here Bruce will put the process code.
        }
        
        //Int-2:processing the tri level interrupt
        if (ISRstatus&SERIAL_FMIIR_DMA_STFIFO_TRIG)
        {
            for (i=0;i<IrDA_RX_Status.ST_FIFO_Trigger_Level;i++)
            {
                //<1>.Read the STFF_STS
                bSTFF_STS=fLib_IRDA_Read_STFF_STS(IrDA_RX_Status.RX_PORT_ADDRESS);
                ReceiveData.DMA_Buffer[i].STFFStatus= bSTFF_STS;
                //<2>.Check the Valid
                if ((bSTFF_STS&SERIAL_STFF_STS_STS_VLD)>0)
                { //<3>.Read Length & Store
                    ReceiveData.DMA_Buffer[i].Len
                    =fLib_IRDA_Read_STFF_RXLEN(IrDA_RX_Status.RX_PORT_ADDRESS);
                    
                    printf("FRM_Len = %d \n", ReceiveData.DMA_Buffer[i].Len);
                    ReceiveData.RX_Receive_Frame_Counter++;
                    printf("Receive_Frame_Counter = %d \n", ReceiveData.RX_Receive_Frame_Counter);
                    frameSize = ReceiveData.DMA_Buffer[0].Len - 4;
                }
                if ((bSTFF_STS&SERIAL_STFF_STS_SIZE_ERR))
                    printf("SERIAL_STFF_STS_SIZE_ERR \n");
                if ((bSTFF_STS&SERIAL_STFF_STS_PHY_ERR))
                     printf("SERIAL_STFF_STS_PHY_ERR \n");
                if ((bSTFF_STS&SERIAL_STFF_STS_RXFIFO_ORUN))
                {
                    fLib_IRDA_Read_RSR(IrDA_RX_Status.RX_PORT_ADDRESS);
                    printf("SERIAL_STFF_RXFIFO_ORUN \n");
                }
                if ((bSTFF_STS&SERIAL_STFF_STS_CRC_ERR))
                    printf("SERIAL_STFF_STS_CRC_ERR \n");

            }
        }
        //Int-3:Check StatusFIFO Timeout             
        if (ISRstatus&SERIAL_FMIIR_DMA_STFIFO_TIMEOUT)
        {
            do {
                //<1>.Read the STFF_STS
                bSTFF_STS=fLib_IRDA_Read_STFF_STS(IrDA_RX_Status.RX_PORT_ADDRESS);
                //<2>.Check the Valid
                if ((bSTFF_STS&SERIAL_STFF_STS_STS_VLD)>0)
                { 
                    ReceiveData.DMA_Buffer[0].STFFStatus= bSTFF_STS;
                    //<3>.Read Length & Store
                    ReceiveData.DMA_Buffer[0].Len
                    =fLib_IRDA_Read_STFF_RXLEN(IrDA_RX_Status.RX_PORT_ADDRESS);
                    ReceiveData.RX_Receive_Frame_Counter++; 
                    frameSize = ReceiveData.DMA_Buffer[0].Len - 4;
                }
                
                }while((bSTFF_STS&SERIAL_STFF_STS_STS_VLD)>0);
        };
        //Read the status of the interrupt
        ISRstatus=fLib_IRDA_Read_FMIIR(IrDA_RX_Status.RX_PORT_ADDRESS);
    };
    
    if(ReceiveData.RX_Receive_Frame_Counter > ReceiveData.RX_Merge_Frame_Counter)
    {
        frameSize = ReceiveData.DMA_Buffer[0].Len - 4;
        data = ithReadRegA(ITH_DMA_BASE + DMA_REG_C0_DST_ADDR + (dmaChannel*DMA_CHANNEL_OFFSET));
        if(data >= ((uint32_t)ReceiveData.bpRXDMABuffer + ((ReceiveData.RX_Merge_Frame_Counter+1)*frameSize)))
        {
            /* Start timer */
            gettimeofday(&t1, NULL);
            offset = ReceiveData.ReceivedBytes + ReceiveData.RX_Merge_Frame_Counter;
            offset2 = ReceiveData.ReceivedBytes;

				    ithInvalidateDCacheRange((void*)(offset), (frameSize+4));

            for(i =0, k =0; i < (frameSize); i++, k++)
                ReceiveData.bpRXDataBuffer[offset2+k] = ReceiveData.bpRXDMABuffer[offset+i];
                
            ReceiveData.ReceivedBytes += frameSize;
            ReceiveData.RX_Merge_Frame_Counter++;
            if(ReceiveData.RX_Merge_Frame_Counter==ReceiveData.RX_Expect_Frame_Numbers)
                result = 1;

        }
    }

    return result;
}


int32_t mmpFirReceiveData(uint8_t *buffer, uint32_t size)
{
    int32_t result = 0;
    uint16_t wEqualizeSize = 2050;//TODO
    uint32_t port = UART_BASE;
  
    InitialRxParam(buffer, size, wEqualizeSize);

    ReceiveData.bpRXDMABuffer = (uint8_t *)itpVmemAlloc(ReceiveData.ExpectBytes);

    if(ReceiveData.bpRXDMABuffer == 0)
    {
        result = EEROR_FIR_ALLOC_DMA_BUFFER;
        printf("[ERROR]DMA Buffer Alloc Fail\r\n");
        goto end;
    }
    
    fLib_SetFIRPrescal(port,IrDA_RX_Status.PSR); //Set PSR & DLL & DLM
  
    // 1.Set the MDR to FIR Mode + Non DMA mode
    fLib_IRDA_Set_PLR(port,IrDA_RX_Status.PLR);
    // 2.Program MRXLENL+MRXLENH
    fLib_IRDA_Set_MaxLen(port,IrDA_RX_Status.MRXLENHL);
    
    // 3.Enable FIFO / Reset RX FIFO / Set the RX FIFO level 
    fLib_IRDA_Set_FCR(port,SERIAL_FCR_TXRX_FIFO_ENABLE);      //Enable FIFO
    fLib_IRDA_Enable_FCR(port,SERIAL_FCR_RX_FIFO_RESET);      //Reset RX FIFO
    fLib_IRDA_Set_FCR_Trigl_Level(port,0,IrDA_RX_Status.RX_FIFO_Trigger_Level);//Set RX Trigger Level
    
    //6.Set DMA
    fLib_FIR_ReadData_DMA(ReceiveData.bpRXDMABuffer, ReceiveData.ExpectBytes);

    fLib_IRDA_Set_MDR(port,IrDA_RX_Status.MDR);
  
    // 4.FMIIR for the RX
    fLib_IRDA_Enable_FMIIER(port,IrDA_RX_Status.RX_FMIIER);

    //5.FMLSIER for the RX
    fLib_IRDA_Set_FMLSIER(port,IrDA_RX_Status.RX_FMLSIER);

    //7.Set STFIFO Tri level
    fLib_IRDA_Set_ACR_STFIFO_Trigl_Level(port, IrDA_RX_Status.ST_FIFO_Trigger_Level);
    //8.Enable Data Receive
    fLib_IRDA_Enable_ACR(port,SERIAL_ACR_RX_ENABLE);
    while(1)
    {
        result = IRDA_ISR_RX_FMIIR_DMA();
        if(result)
            break;
    }
    mmpFirWaitDMAIdle();
    FIR_RX_DMA_Close(port);
    itpVmemFree((uint32_t)ReceiveData.bpRXDMABuffer);

end:

    return result;	
}

int32_t mmpFirSendCheck(void)
{
    int32_t result = ERROR_IR_DATA_INVALID;
    uint8_t ISRstatus = 0;
    uint8_t   frameValid = 0;
    struct timeval t1, t2;
    double elapsedTime;
    /* Start timer */
    gettimeofday(&t1, NULL);

    //IIR
    while(1)
    {
        ISRstatus=fLib_IRDA_Read_FMIIR(IrDA_TX_Status.TX_PORT_ADDRESS);
    
        if (ISRstatus&SERIAL_FMIIR_DMA_FRM_SENT)
        {
            SendData.Send_FRM_SENT_OK_Counter++;
            SendData.SendByte += SendData.TX_1Frame_Bytes;
            frameValid = 1;
            //printf("frame send = %d\n", SendData.TX_1Frame_Bytes);
            goto end;
        }
        
        if(ISRstatus&SERIAL_FMIIR_DMA_TXFIFO_URUN)
        {
            SendData.SendErrorType=ERROR_FIR_TX_UNDERRUN;
            fLib_IRDA_Read_RSR(IrDA_TX_Status.TX_PORT_ADDRESS);
            goto end;
        }

       /* End timer */
			gettimeofday(&t2, NULL);
			elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
			elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms

       if(elapsedTime > 5)
            break;
    }

end:
    if(frameValid && SendData.SendErrorType == 0)
        result = 0;
                
    return result;
}

int32_t mmpFirReceiveCheck(int32_t *fsize, uint32_t *fCount)
{
    int32_t result = ERROR_IR_DATA_INVALID;
	  uint8_t ISRstatus,bSTFF_STS;	
    uint32_t data;
    uint32_t i = 0, j = 0, k = 0;
    int32_t frameSize = 0;
    uint32_t offset = 0;
    uint32_t offset2 = 0;
    //DMA_CTXT *dma = (DMA_CTXT*)dmaCtxt;
    uint32_t dmaRdPoint = 0, dmaWtPoint = 0;
    uint32_t mergeBytes = 0, remainBytes = 0;

    // 1.Read FMIIR Status => 1.Tri level interrupt / 2.Over run / 3.EOF detected
    ISRstatus=fLib_IRDA_Read_FMIIR(IrDA_RX_Status.RX_PORT_ADDRESS);

    *fCount = 0;

    while (ISRstatus&SERIAL_FMIIR_DMA_RX_EVENT)
    {	
    	// 2.Get the character	
        //Int-1:Check Over Run
        if(ISRstatus&SERIAL_FMIIR_DMA_STFIFO_ORUN)
        {
            printf("Status FIFO Over Run => Fail \n");
            printf("IIR = 0x%x\n", ISRstatus);
            ReceiveData.ReceiveErrorType_FMIIR=ERROR_FIR_RX_ERROR_OVERRUN;
            //sync r/w pointer
            data = ithReadRegA(ITH_DMA_BASE + DMA_REG_C0_DST_ADDR + (dmaChannel*DMA_CHANNEL_OFFSET));
            dmaWtPoint = data - (uint32_t)(ReceiveData.bpRXDMABuffer);
            ReceiveData.ReceivedBytes = dmaWtPoint;
            ReceiveData.RX_Merge_Frame_Counter = ReceiveData.RX_Receive_Frame_Counter;
            
            do{
                //<1>.Read the STFF_STS
                bSTFF_STS=fLib_IRDA_Read_STFF_STS(IrDA_RX_Status.RX_PORT_ADDRESS);
                //<2>.Check the Valid
                if ((bSTFF_STS&SERIAL_STFF_STS_STS_VLD)>0)
                { 
                    ReceiveData.DMA_Buffer[i].STFFStatus= bSTFF_STS;
                    //<3>.Read Length & Store
                    ReceiveData.DMA_Buffer[i].Len
                    =fLib_IRDA_Read_STFF_RXLEN(IrDA_RX_Status.RX_PORT_ADDRESS);
                    i++;
                }
            }while((bSTFF_STS&SERIAL_STFF_STS_STS_VLD)>0);
            
            fLib_IRDA_Enable_FCR(IrDA_RX_Status.RX_PORT_ADDRESS,SERIAL_FCR_RX_FIFO_RESET);      //Reset RX FIFO

            //resume
            fLib_IRDA_Read_RSR(IrDA_RX_Status.RX_PORT_ADDRESS);
            *fsize = -1;     //error
            *fCount = 1;
            return result;
        }
            
        //Int-2:processing the tri level interrupt
        if(ISRstatus&SERIAL_FMIIR_DMA_STFIFO_TRIG)
        {
            //for (i=0;i<IrDA_RX_Status.ST_FIFO_Trigger_Level;i++)
            {
                //<1>.Read the STFF_STS
                bSTFF_STS=fLib_IRDA_Read_STFF_STS(IrDA_RX_Status.RX_PORT_ADDRESS);
                ReceiveData.DMA_Buffer[i].STFFStatus= bSTFF_STS;
                //<2>.Check the Valid
                if ((bSTFF_STS&SERIAL_STFF_STS_STS_VLD)>0)
                { //<3>.Read Length & Store
                    ReceiveData.DMA_Buffer[i].Len
                    =fLib_IRDA_Read_STFF_RXLEN(IrDA_RX_Status.RX_PORT_ADDRESS);
                    if(ReceiveData.ReceiveErrorType_FMIIR == 0)
                        ReceiveData.RX_Receive_Frame_Counter++;
                   //check error
                    if ((bSTFF_STS&SERIAL_STFF_STS_SIZE_ERR))
                    {
                        ReceiveData.ReceiveErrorType_FMIIR=ERROR_STFF_STS_SIZE;
                        ReceiveData.DMA_Buffer[i].Len = -1; 
                        printf("ERROR_STFF_STS_SIZE\n");
                        i++;
                        break;
                    }
                    if ((bSTFF_STS&SERIAL_STFF_STS_PHY_ERR))
                    {
                        ReceiveData.ReceiveErrorType_FMIIR=ERROR_STFF_STS_PHY;
                        ReceiveData.DMA_Buffer[i].Len = -1; 
                        printf("ERROR_STFF_STS_PHY\n");
                        i++;
                        fLib_IRDA_Disable_ACR(IrDA_RX_Status.RX_PORT_ADDRESS,SERIAL_ACR_RX_ENABLE);
                        break;
                    }
                    if ((bSTFF_STS&SERIAL_STFF_STS_RXFIFO_ORUN))
                    {
                        fLib_IRDA_Read_RSR(IrDA_RX_Status.RX_PORT_ADDRESS);
                        ReceiveData.ReceiveErrorType_FMIIR=ERROR_STFF_STS_ORUN;
                        ReceiveData.DMA_Buffer[i].Len = -1; 
                        printf("ERROR_STFF_STS_ORUN\n");
                        i++;
                        break;

                   }
                   if ((bSTFF_STS&SERIAL_STFF_STS_CRC_ERR))
                   {
                        ReceiveData.ReceiveErrorType_FMIIR=ERROR_STFF_STS_CRC;
                        ReceiveData.DMA_Buffer[i].Len = -1; 
                        printf("ERROR_STFF_STS_CRC\n");
                        i++;
                        break;
                   }
                   i++;
                   if(i >= 7)
                    break;        
                }
            }
        }
        //Int-3:Check StatusFIFO Timeout             
        if (ISRstatus&SERIAL_FMIIR_DMA_STFIFO_TIMEOUT)
        {
            do{
                //<1>.Read the STFF_STS
                bSTFF_STS=fLib_IRDA_Read_STFF_STS(IrDA_RX_Status.RX_PORT_ADDRESS);
                //<2>.Check the Valid
                if ((bSTFF_STS&SERIAL_STFF_STS_STS_VLD)>0)
                { 
                    ReceiveData.DMA_Buffer[i].STFFStatus= bSTFF_STS;
                    //<3>.Read Length & Store
                    ReceiveData.DMA_Buffer[i].Len
                    =fLib_IRDA_Read_STFF_RXLEN(IrDA_RX_Status.RX_PORT_ADDRESS);
                    if(ReceiveData.ReceiveErrorType_FMIIR == 0)
                        ReceiveData.RX_Receive_Frame_Counter++; 
                    i++;
                }
            }while((bSTFF_STS&SERIAL_STFF_STS_STS_VLD)>0);
        }
        
        ISRstatus=fLib_IRDA_Read_FMIIR(IrDA_RX_Status.RX_PORT_ADDRESS);
    }

    i = 0;
    while(ReceiveData.RX_Receive_Frame_Counter > ReceiveData.RX_Merge_Frame_Counter)
    {
        if(ReceiveData.DMA_Buffer[i].Len > 4 && i < 8)
        {
             frameSize = ReceiveData.DMA_Buffer[i].Len - 4;
             fsize[i] = frameSize;
        }
        else if(ReceiveData.DMA_Buffer[i].Len <= 4 && i < 8)
        {
            frameSize = -1;
            fsize[i] = frameSize;     //error
        }
        data = ithReadRegA(ITH_DMA_BASE + DMA_REG_C0_DST_ADDR + (dmaChannel*DMA_CHANNEL_OFFSET));
        dmaWtPoint = data - (uint32_t)(ReceiveData.bpRXDMABuffer);
        
        if(frameSize == -1)          //if error ,sync the pointer
        {
            ReceiveData.ReceivedBytes = dmaWtPoint;
            ReceiveData.RX_Merge_Frame_Counter = ReceiveData.RX_Receive_Frame_Counter;
            if(i < 8)
                i++;
            goto end;
        }
            
        dmaRdPoint = ReceiveData.ReceivedBytes;
        
        if(dmaWtPoint < dmaRdPoint)
            dmaWtPoint += ReceiveData.dmaSize;

        if((dmaWtPoint - dmaRdPoint) >= frameSize)
        {
            offset2 += (i == 0) ? 0 :(ReceiveData.DMA_Buffer[i-1].Len - 4);
            
            if((dmaRdPoint + (frameSize + 4)) > ReceiveData.dmaSize)
            {
                //data at buttom 
                mergeBytes = ReceiveData.dmaSize - dmaRdPoint;
                if(mergeBytes >= frameSize)
                {
                    mergeBytes = frameSize;
                    ReceiveData.ReceivedBytes += (frameSize + 4);
                    if(ReceiveData.ReceivedBytes >= ReceiveData.dmaSize)
                        ReceiveData.ReceivedBytes = ReceiveData.ReceivedBytes - ReceiveData.dmaSize;
                }

				

								ithInvalidateDCacheRange((void*)(ReceiveData.bpRXDMABuffer + dmaRdPoint), mergeBytes);

                for(j =0, k =0; j < mergeBytes; j ++, k++)
                    ReceiveData.bpRXDataBuffer[offset2+k] = ReceiveData.bpRXDMABuffer[dmaRdPoint+j];
                
                //data at top
                remainBytes = frameSize - mergeBytes;
                if(remainBytes > 0)
                {					

										ithInvalidateDCacheRange((void*)(ReceiveData.bpRXDMABuffer), remainBytes);
					
                    for(j =0; j < remainBytes; j ++, k++)
                        ReceiveData.bpRXDataBuffer[offset2+k] = ReceiveData.bpRXDMABuffer[j];
    
                    ReceiveData.ReceivedBytes = remainBytes + 16;   // add crc
                }     
            }    
            else
            {

								ithInvalidateDCacheRange((void*)(ReceiveData.bpRXDMABuffer + dmaRdPoint), (frameSize+4));
					 
                for(j =0, k =0; j < (frameSize); j ++, k++)
                    ReceiveData.bpRXDataBuffer[offset2+k] = ReceiveData.bpRXDMABuffer[dmaRdPoint+j];
    
                ReceiveData.ReceivedBytes += (frameSize + 4);
                if(ReceiveData.ReceivedBytes >= ReceiveData.dmaSize)
                    ReceiveData.ReceivedBytes = 0;
            }
            ReceiveData.RX_Merge_Frame_Counter++;
            i++;
        }
    }
    
end: 
    *fCount = i;

    return result;
}

int32_t mmpFirReceiveFrame(uint8_t *buffer, uint32_t maxSize)
{
    int32_t result = 0;
    uint32_t port = UART_BASE;

    InitialRxParam(buffer, maxSize, 2050);
    
    if(ReceiveData.bpRXDMABuffer == 0)
    {

        ReceiveData.bpRXDMABuffer = (uint8_t *)itpVmemAlloc(ReceiveData.dmaSize);
        if(ReceiveData.bpRXDMABuffer == 0)
        {
            result = EEROR_FIR_ALLOC_DMA_BUFFER;
            goto end;
        }
    }
    fLib_IRDA_Disable_ACR(port, SERIAL_ACR_TX_ENABLE);

    fLib_SetFIRPrescal(port,IrDA_RX_Status.PSR); //Set PSR & DLL & DLM
  
    // 1.Set the MDR to FIR Mode + Non DMA mode
    fLib_IRDA_Set_PLR(port,IrDA_RX_Status.PLR);
    // 2.Program MRXLENL+MRXLENH
    fLib_IRDA_Set_MaxLen(port,IrDA_RX_Status.MRXLENHL);
    
    fLib_IRDA_Set_MDR(port,IrDA_RX_Status.MDR);

    // 3.Enable FIFO / Reset RX FIFO / Set the RX FIFO level 
    fLib_IRDA_Set_FCR(port,SERIAL_FCR_TXRX_FIFO_ENABLE);      //Enable FIFO
    //fLib_IRDA_Enable_FCR(port,SERIAL_FCR_RX_FIFO_RESET);      //Reset RX FIFO TODO
    fLib_IRDA_Set_FCR_Trigl_Level(port,0,IrDA_RX_Status.RX_FIFO_Trigger_Level);//Set RX Trigger Level
    
    //6.Set DMA
    fLib_FIR_ReadData_DMA(ReceiveData.bpRXDMABuffer, ReceiveData.dmaSize);

  
    // 4.FMIIR for the RX
    fLib_IRDA_Enable_FMIIER(port,IrDA_RX_Status.RX_FMIIER);

    //5.FMLSIER for the RX
    fLib_IRDA_Set_FMLSIER(port,IrDA_RX_Status.RX_FMLSIER);

    //7.Set STFIFO Tri level
    fLib_IRDA_Set_ACR_STFIFO_Trigl_Level(port, IrDA_RX_Status.ST_FIFO_Trigger_Level);
    //8.Enable Data Receive
    fLib_IRDA_Enable_ACR(port,SERIAL_ACR_RX_ENABLE);
end:
    return result;	
}

int32_t mmpFirReceiveClose(void)
{
    int32_t result = 0;
    uint32_t port = UART_BASE;
    //mmpFirWaitDMAIdle();
    FIR_RX_DMA_Close(port);
    return result;	
}

int32_t mmpFirSendData(uint8_t *buffer, uint32_t size, void* param)
{
    uint16_t frameSize = *(uint16_t*)param;
    uint16_t frameCount = 0;
    uint16_t tsize = 0;
    int32_t result = 0;
    while(size > 0)
    {
        if(size >= frameSize)
        {
            frameCount = (uint16_t)(size/frameSize);
            
            IrDA_Mode_Sel.FIRDMAEnable = SERIAL_MDR_DMA_ENABLE;

            InitialTxParam(buffer, frameSize*frameCount, frameSize);
            result = FirDMATx();
            size -= frameSize*frameCount;
            buffer += frameSize*frameCount;
            
        }
        else if(size >=  32)
        {
            tsize = (uint16_t)((size >> 2) << 2);
            frameCount = 1;
            
            IrDA_Mode_Sel.FIRDMAEnable = SERIAL_MDR_DMA_ENABLE;

            InitialTxParam(buffer, tsize*frameCount, tsize);
            result = FirDMATx();
            size -= tsize;
            buffer += tsize;
        }
        else
        {
            IrDA_Mode_Sel.FIRDMAEnable = 0;
            InitialTxParam(buffer, size, (uint16_t)size);
            result = FirPIOTx();
            size = 0;
        }
    }
    return result;
}

int32_t mmpFirSendFrame(uint8_t *buffer, uint32_t frameSize)
{
    int32_t result = 0;
    
    if(frameSize >= 32)
    {
        IrDA_Mode_Sel.FIRDMAEnable = SERIAL_MDR_DMA_ENABLE;
        InitialTxParam(buffer, frameSize, (uint16_t)frameSize);
        result = FirDMATx();
    }
    else
    {
        IrDA_Mode_Sel.FIRDMAEnable = 0;
        InitialTxParam(buffer, frameSize, (uint16_t)frameSize);
        result = FirPIOTx();
    }
    
    return result;
}

int32_t mmpFirSendClose(void)
{
    int32_t result = 0;
    uint32_t port = UART_BASE;
    
    fLib_IRDA_Disable_ACR(port,SERIAL_ACR_TX_ENABLE); 
    fLib_IRDA_Enable_FCR(port,SERIAL_FCR_TX_FIFO_RESET);
   
    return result;	
}

