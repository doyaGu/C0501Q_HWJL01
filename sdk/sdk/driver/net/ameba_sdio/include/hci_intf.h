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
#ifndef _HCI_INTFS_H_
#define _HCI_INTFS_H_

#include <autoconf.h>
#include <drv_types.h>
struct host_ctrl_intf_ops 
{
	struct dvobj_priv *(*dvobj_init)(void *data);
	void (*dvobj_deinit)(struct dvobj_priv *dvobj);
	int (*dvobj_request_irq)(struct dvobj_priv *dvobj);
	void (*dvobj_free_irq)(struct dvobj_priv *dvobj);
};
enum RTL871X_HCI_TYPE {
	RTW_USB 	= BIT0,
	RTW_SDIO 	= BIT1,
	RTW_GSPI	= BIT2,
};
extern struct dvobj_priv *hci_dvobj_init(void *data);
extern void hci_dvobj_deinit(struct dvobj_priv *dvobj);
extern int hci_dvobj_request_irq(struct dvobj_priv *dvobj);
extern void hci_dvobj_free_irq(struct dvobj_priv *dvobj);

#if defined(CONFIG_GSPI_HCI)
#define hci_bus_intf_type RTW_GSPI
#define hci_set_intf_ops spi_set_intf_ops
#define hci_intf_start	rtw_hal_enable_interrupt
#define hci_intf_stop	rtw_hal_disable_interrupt
u8 spi_query_tx_buffer_status_info(ADAPTER* padapter);
#endif

#if defined(CONFIG_SDIO_HCI) 
#define hci_bus_intf_type RTW_SDIO
#define hci_set_intf_ops sdio_set_intf_ops
#define hci_intf_start	rtw_hal_enable_interrupt
#define hci_intf_stop	rtw_hal_disable_interrupt
#endif

#if defined(CONFIG_USB_HCI)
#define hci_bus_intf_type RTW_USB
#define hci_set_intf_ops usb_set_intf_ops
#define hci_intf_start	usb_intf_start
#define hci_intf_stop	usb_intf_stop
#endif
#endif //_HCI_INTFS_H_

