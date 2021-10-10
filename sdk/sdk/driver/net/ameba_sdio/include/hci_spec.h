/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef __HCI_SPEC_H__
#define __HCI_SPEC_H__


#if defined(CONFIG_GSPI_HCI)
//#include "8195_gspi_reg.h"
#include <basic_types.h>
#include <freertos/osdep_service_freertos.h>

// SPI Header Files
#ifdef PLATFORM_LINUX
#include <linux/spi/spi.h>
#endif

#define GSPI_CMD_LEN		4
#define HAL_INTERFACE_CMD_LEN			GSPI_CMD_LEN
#define GSPI_STATUS_LEN		8
#define HAL_INTERFACE_CMD_STATUS_LEN   	GSPI_STATUS_LEN
#define HAL_INTERFACE_OVERHEAD			(HAL_INTERFACE_CMD_LEN+HAL_INTERFACE_OVERHEAD)
//reserve tx headroom in case of softap forwarding unicase packet 
#define RX_RESERV_HEADROOM		(SKB_WLAN_TX_EXTRA_LEN>RX_DRIVER_INFO+RXDESC_SIZE)?(SKB_WLAN_TX_EXTRA_LEN-RX_DRIVER_INFO-RXDESC_SIZE):0


//extern void spi_set_intf_ops(struct _io_ops *pops);
extern void spi_int_hdl(PADAPTER padapter);
#define rtw_hci_interrupt_handler(__adapter) spi_int_hdl(__adapter)
#endif
#endif //__HCI_SPEC_H__

