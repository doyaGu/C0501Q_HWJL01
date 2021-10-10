#ifndef SPI_FTSSP_H
#define SPI_FTSSP_H

#include "../ite/ith_defs.h"

#define SSP_BASE							ITH_SSP0_BASE

#ifdef _WIN32
#define REG_SSP_PORT_OFFSET					0x400
#else
#define REG_SSP_PORT_OFFSET					0x100000
#endif

#define REG_SSP_CONTROL_0					(SSP_BASE + 0x0)
#define REG_SSP_CONTROL_1					(SSP_BASE + 0x4)
#define REG_SSP_CONTROL_2					(SSP_BASE + 0x8)
#define REG_SSP_STATUS						(SSP_BASE + 0xC)
#define REG_SSP_INTR_CONTROL				(SSP_BASE + 0x10)
#define REG_SSP_INTR_STATUS					(SSP_BASE + 0x14)
#define REG_SSP_DATA						(SSP_BASE + 0x18)
#define REG_SSP_AC_LINK						(SSP_BASE + 0x20)
#define REG_SSP_REVISION					(SSP_BASE + 0x40)
#define REG_SSP_FEATURE						(SSP_BASE + 0x44)
#ifdef CFG_CHIP_PKG_IT9910
#define REG_SSP_FREERUN						(SSP_BASE + 0x1C)
#else
#define REG_SSP_FREERUN						(SSP_BASE + 0x60)
#endif

/*SSP Control Register 0*/
#define REG_BIT_TXFIFO_BIG_END				(1u << 17)
#define REG_BIT_RXFIFO_BIG_END				(1u << 16)
#define REG_BIT_FFMT_SSP					(0u << 12)
#define REG_BIT_FFMT_SPI					(1u << 12)
#define REG_BIT_FFMT_MWIRE					(2u << 12)
#define REG_BIT_FFMT_I2S					(3u << 12)
#define REG_BIT_FFMT_AC_LINK				(4u << 12)

#define REG_BIT_FSDIST						(1u << 8)
#define REG_BIT_LOOKBACK_EN					(1u << 7)
#define REG_BIT_LSB_EN						(1u << 6)
#define REG_BIT_CS_ACTIVE_LOW				(1u << 5)
#define REG_BIT_FSJSSTFY					(1u << 4)
#define REG_BIT_MASTER_MODE					(1u << 3)

#define REG_BIT_CPOL_HI						(1u << 1)
#define REG_BIT_CPHA_HALF_ACTIVE			(1u << 0)

#define REG_BIT_CPOL_LO						(0u << 1)
#define REG_BIT_CPHA_ACTIVE					(0u << 0)

/*SSP Control Register 1*/
#define REG_MASK_PADDING_DATA_LEN			(0xFF << 24)
#define REG_SHIFT_SERIAL_DATA_LEN			16
#define REG_MASK_SERIAL_DATA_LEN			(0x1F << REG_SHIFT_SERIAL_DATA_LEN)
#define REG_MASK_SCLK_DIV					(0xFFFF)

/*SSP Control Register 2*/
#define REG_BIT_SSP_RESET					(1u << 6)
#define REG_BIT_AC_LINK_COLD_RESET			(1u << 5)
#define REG_BIT_AC_LINK_WARM_RESET			(1u << 4)
#define REG_BIT_TXFCLR_EN					(1u << 3)
#define REG_BIT_RXFCLR_EN					(1u << 2)
#define REG_BIT_TXDO_EN						(1u << 1)
#define REG_BIT_SSP_EN						(1u << 0)
#define REG_BIT_SSP_DIS             		(0u)

/*SSP Status Register */
#ifdef __FREERTOS__
#define REG_SHIFT_TX_FIFO_VALID_ENTRIES		12
#define REG_MASK_TX_FIFO_VALID_ENTRIES		(0x1F << REG_SHIFT_TX_FIFO_VALID_ENTRIES)
#else
#define REG_SHIFT_TX_FIFO_VALID_ENTRIES		12
#define REG_BIT_TX_FIFO_VALID_ENTRIES_HI	1
#define REG_MASK_TX_FIFO_VALID_ENTRIES		(0xF << REG_SHIFT_TX_FIFO_VALID_ENTRIES)
#endif

#define REG_SHIFT_RX_FIFO_VALID_ENTRIES		4
#define REG_MASK_RX_FIFO_VALID_ENTRIES		(0x1F << REG_SHIFT_RX_FIFO_VALID_ENTRIES)
#define REG_BIT_IS_BUSY						(1u << 2)
#define REG_BIT_TX_FIFO_IS_AVAIL			(1u << 1)
#define REG_BIT_RX_FIFO_IS_FULL				(1u << 0)

/*SSP Interrupt Control Register */
#define REG_SHIFT_TX_THRESHOLD				16
#define REG_BIT_TX_FIFO_THRESHOLD			(0xF << REG_SHIFT_TX_THRESHOLD)
#define REG_SHIFT_RX_THRESHOLD				8
#define REG_BIT_RX_FIFO_THRESHOLD			(0xF << REG_SHIFT_RX_THRESHOLD)
#define REG_BIT_AC_FRAME_EN					(1u << 6)
#define REG_BIT_TX_DMA_EN           		(1u << 5)
#define REG_BIT_RX_DMA_EN           		(1u << 4)
#define REG_BIT_TX_INTR_TH_EN       		(1u << 3)       //Tx FIFO Threshold Intr Enable
#define REG_BIT_RX_INTR_TH_EN       		(1u << 2)       //Rx FIFO Threshold Intr Enable
#define REG_BIT_TX_INTR_UR_EN       		(1u << 1)       //Tx FIFO Underrun Intr Enable
#define REG_BIT_RX_INTR_OR_EN       		(1u << 0)       //Rx FIFO Overrun Intr Enable

/*SSP Interrupt Status Register */
#define REG_BIT_AC_FRAME_COMPLETE   		(1u << 4)
#define REG_BIT_TX_INTR_TH          		(1u << 3)       //Tx FIFO Threshold Intr 
#define REG_BIT_RX_INTR_TH          		(1u << 2)       //Rx FIFO Threshold Intr 
#define REG_BIT_TX_INTR_UR          		(1u << 1)       //Tx FIFO Underrun Intr 
#define REG_BIT_RX_INTR_OR          		(1u << 0)       //Rx FIFO Overrun Intr 

/*TX FIFO Free Run Register*/
#define REG_BIT_FR_CSN_CLR					(1u << 2)
#define REG_BIT_FR_RX_LOCK					(1u << 1)
#define REG_BIT_FR_TX_EN					(1u << 0)

/*SSP Revision Register */
#ifdef __FREERTOS__
#define REG_BIT_MAJOR_REV           		(0xFF << 16)
#else
#define REG_BIT_MAJOR_REV           		(0xFF)
#endif
#define REG_BIT_MINOR_REV           		(0xFF << 8)
#define REG_BIT_RELEASE_REV         		(0xFF)

#endif

