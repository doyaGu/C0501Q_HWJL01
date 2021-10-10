#ifndef AXISPI_REG_H
#define AXISPI_REG_H

#include "ite/ith_defs.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define REG_SPI_BASE                    (ITH_AXISPI_BASE) /* Base Register Address */

//====================================================================
/*
 * 1. 0x0000
 *    SPI Flash Address Register
 */
//====================================================================

#define SPI_REG_CMD_W0                  (REG_SPI_BASE + 0x0000)

//====================================================================
/*
 * 0x0004 
 *    Command Queue Second Word Register 
 */
//====================================================================

#define SPI_REG_CMD_W1					(REG_SPI_BASE + 0x0004)

#define SPI_BIT_EN_CONT_READMODE        1

#define SPI_SHT_ADDR_LENGTH             0
#define SPI_SHT_DUMMY_CYCLE             16
#define SPI_SHT_INST_LENGTH             24
#define SPI_SHT_EN_CONT_READMODE        28

#define SPI_MSK_ADDR_LENGTH         	(N03_BITS_MSK)
#define SPI_MSK_DUMMY_CYCLE             (N08_BITS_MSK << SPI_SHT_DUMMY_CYCLE)    
#define SPI_MSK_INST_LENGTH             (N02_BITS_MSK << SPI_SHT_INST_LENGTH)   
#define SPI_MSK_EN_CONT_READMODE        (N01_BITS_MSK << SPI_SHT_EN_CONT_READMODE)  

//====================================================================
/*
 * 0x0008 
 *   Data counter Register
 */
//====================================================================

#define SPI_REG_CMD_W2	                (REG_SPI_BASE + 0x0008)

//====================================================================
/*
 * 0x000C
 *    Command Queue Fourth Word Register 
 */
//====================================================================

#define SPI_REG_CMD_W3                  (REG_SPI_BASE + 0x000C)

#define SPI_BIT_EN_WRITE           		1
#define SPI_BIT_EN_READ_STATUS  	    1
#define SPI_BIT_EN_DTR_MODE            	1

#define SPI_SHT_EN_WRITE           		1
#define SPI_SHT_EN_READ_STATUS  	    2
#define SPI_SHT_READ_STATUS             3
#define SPI_SHT_EN_DTR_MODE            	4
#define SPI_SHT_OPERATE_MODE            5
#define SPI_SHT_START_CS            	8
#define SPI_SHT_CONT_READMODE_CODE   	16
#define SPI_SHT_INST_CODE   			24

#define SPI_MSK_EN_WRITE 	        	(N01_BITS_MSK << SPI_SHT_EN_WRITE)
#define SPI_MSK_EN_READ_STATUS          (N01_BITS_MSK << SPI_SHT_EN_READ_STATUS)    
#define SPI_MSK_READ_STATUS             (N01_BITS_MSK << SPI_SHT_READ_STATUS)   
#define SPI_MSK_EN_DTR_MODE        		(N01_BITS_MSK << SPI_SHT_EN_DTR_MODE)  
#define SPI_MSK_OPERATE_MODE       		(N03_BITS_MSK << SPI_SHT_OPERATE_MODE) 
#define SPI_MSK_START_CS         		(N02_BITS_MSK << SPI_SHT_START_CS) 
#define SPI_MSK_CONT_READMODE_CODE   	(N08_BITS_MSK << SPI_SHT_CONT_READMODE_CODE)
#define SPI_MSK_INST_CODE   			(N08_BITS_MSK << SPI_SHT_INST_CODE)

//====================================================================
/*
 * 0x0010
 *    Control Register
 */
//====================================================================

#define SPI_REG_CTL              		(REG_SPI_BASE + 0x0010)

#define SPI_SHT_CLK_DIV           		0
#define SPI_SHT_CLK_MODE  	        	4
#define SPI_SHT_ABORT              		8
#define SPI_SHT_RDY_LOC            		16
#define SPI_SHT_DAMR_PORT_SEL           20

#define SPI_MSK_CLK_DIV   	        	(N02_BITS_MSK << SPI_SHT_CLK_DIV)
#define SPI_MSK_CLK_MODE          		(N01_BITS_MSK << SPI_SHT_CLK_MODE)    
#define SPI_MSK_ABORT              		(N01_BITS_MSK << SPI_SHT_ABORT)  
#define SPI_MSK_RDY_LOC        			(N03_BITS_MSK << SPI_SHT_RDY_LOC)
#define SPI_MSK_DAMR_PORT_SEL       	(N01_BITS_MSK << SPI_SHT_DAMR_PORT_SEL) 

//====================================================================
/*
 * 0x0014
 *    AC Timing Register
 */
//====================================================================

#define SPI_REG_AC_TIMING               (REG_SPI_BASE + 0x0014)
#define SPI_MSK_CS_DELAY   	        	(N04_BITS_MSK)
//====================================================================
/*
 * 0x0018
 *   Status Register
 */
//====================================================================

#define SPI_REG_STATUS                  (REG_SPI_BASE + 0x0018)

#define SPI_SHT_TXFIFO_READY           	0
#define SPI_SHT_RXFIFO_READY 	        1
#define SPI_SHT_TXFIFO_VALID_SPACE      4
#define SPI_SHT_RXFIFO_DATA_COUNT		12

#define SPI_MSK_TXFIFO_READY  	       	(N01_BITS_MSK << SPI_SHT_TXFIFO_READY)
#define SPI_MSK_RXFIFO_READY  	       	(N01_BITS_MSK << SPI_SHT_RXFIFO_READY)
#define SPI_MSK_TXFIFO_VALID_SPACE		(N06_BITS_MSK << SPI_SHT_TXFIFO_VALID_SPACE)
#define SPI_MSK_RXFIFO_DATA_COUNT		(N06_BITS_MSK << SPI_SHT_RXFIFO_DATA_COUNT)


//====================================================================
/*
 * 0x0020
 *    Interrupt Control Register
 */
//====================================================================

#define SPI_REG_INTR_CTL                (REG_SPI_BASE + 0x0020)

#define SPI_BIT_EN_DMA           		1
#define SPI_BIT_EN_CMD_CMPLT_INTR 	    1

#define SPI_SHT_EN_DMA           		0
#define SPI_SHT_EN_CMD_CMPLT_INTR 	    1
#define SPI_SHT_TXFIFO_THOD 	    	8
#define SPI_SHT_RXFIFO_THOD 	    	12

#define SPI_MSK_EN_CMD_CMPLT_INTR  	    (N01_BITS_MSK << SPI_SHT_EN_CMD_CMPLT_INTR)
#define SPI_MSK_TXFIFO_THOD 	    	(N02_BITS_MSK << SPI_SHT_TXFIFO_THOD)
#define SPI_MSK_RXFIFO_THOD 	    	(N02_BITS_MSK << SPI_SHT_RXFIFO_THOD)

//====================================================================
/*
 * 0x0024
 *    Interrupt Status Register
 */
//====================================================================

#define SPI_REG_INTR_STATUS             (REG_SPI_BASE + 0x0024)
#define SPI_MSK_CMD_CMPLT_STATUS        (N01_BITS_MSK)

//====================================================================
/*
 * 0x0028
 *    SPI Read Status Register
 */
//====================================================================

#define SPI_REG_READ_STATUS             (REG_SPI_BASE + 0x0028)
#define SPI_MSK_READ_STATUS 	    	(N08_BITS_MSK)

//====================================================================
/*
 * 0x002C
 *    SPI Flash Size Register
 */
//====================================================================

#define SPI_REG_FLASH_SIZE              (REG_SPI_BASE + 0x002C)

//====================================================================
/*
 * 0x0030
 *    DAMR Command Word
 */
//====================================================================

#define SPI_REG_DAMR_CMD                (REG_SPI_BASE + 0x0030)

#define SPI_BIT_DAMR_EN_IO_MODE 	    1

#define SPI_SHT_DAMR_OPERATE_MODE 	    8
#define SPI_SHT_DAMR_INST_CODE 	    	12
#define SPI_SHT_DAMR_IO_MODE 	    	20
#define SPI_SHT_DAMR_EN_IO_MODE 	    28


#define SPI_MSK_DAMR_DUMMY_2ND_CYCLE 	(N08_BITS_MSK)
#define SPI_MSK_DAMR_OPERATE_MODE 	    (N03_BITS_MSK << SPI_SHT_DAMR_OPERATE_MODE)
#define SPI_MSK_DAMR_INST_CODE 		    (N08_BITS_MSK << SPI_SHT_DAMR_INST_CODE)
#define SPI_MSK_DAMR_IO_MODE 		    (N08_BITS_MSK << SPI_SHT_DAMR_IO_MODE)
#define SPI_MSK_DAMR_EN_IO_MODE 		(N01_BITS_MSK << SPI_SHT_DAMR_EN_IO_MODE)

//====================================================================
/*
 * 0x0050
 *    Revision Register
 */
//====================================================================

#define SPI_REG_REV		                (REG_SPI_BASE + 0x0050)

//====================================================================
/*
 * 0x0054
 *    Feature Register
 */
//====================================================================

#define SPI_REG_FEATURE                 (REG_SPI_BASE + 0x0054)

#define SPI_SHT_TXFIFO_DEPTH 	    	0
#define SPI_SHT_RXFIFO_DEPTH 	    	8
#define SPI_SHT_AXI_ID_DW 	    		19
#define SPI_SHT_DTR_MODE 	    		24
#define SPI_SHT_CLK_MODE 	    		25
#define SPI_SHT_DAMR_PORT 	    		29
#define SPI_SHT_HOST_IF_DW 	    		30
#define SPI_SHT_HOST_IF 	    		31


#define SPI_MSK_TXFIFO_DEPTH			(N08_BITS_MSK)
#define SPI_MSK_RXFIFO_DEPTH			(N08_BITS_MSK << SPI_SHT_RXFIFO_DEPTH)
#define SPI_MSK_AXI_ID_DW 	    		(N05_BITS_MSK << SPI_SHT_AXI_ID_DW)
#define SPI_MSK_DTR_MODE 	 		    (N01_BITS_MSK << SPI_SHT_DTR_MODE)
#define SPI_MSK_CLK_MODE 		    	(N01_BITS_MSK << SPI_SHT_CLK_MODE)
#define SPI_MSK_DAMR_PORT				(N01_BITS_MSK << SPI_SHT_DAMR_PORT)
#define SPI_MSK_HOST_IF_DW				(N01_BITS_MSK << SPI_SHT_HOST_IF_DW)
#define SPI_MSK_HOST_IF					(N01_BITS_MSK << SPI_SHT_HOST_IF)


//====================================================================
/*
 * 0x0100 
 *    Data Port Register
 */
//====================================================================

#define SPI_REG_DATA_PORT               (REG_SPI_BASE + 0x0100)

#endif

