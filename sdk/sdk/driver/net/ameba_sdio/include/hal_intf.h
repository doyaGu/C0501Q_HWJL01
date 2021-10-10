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
#ifndef __HAL_INTF_H__
#define __HAL_INTF_H__
#include <autoconf.h>
#include <basic_types.h>
#include <drv_types.h>

#ifdef CONFIG_POWER_SAVING
enum RTL871X_RPWM2_EVNT {
	RPWM2_PWR_ACT			= 0,		// Active
	RPWM2_PWR_DSTANDBY		= 1,		// Deep Standby
	RPWM2_PWR_PG			= 2,		// Power Gated and wakeup with normal boot
	RPWM2_PWR_PG_FB			= 3,		// Power Gated and wakeup with fast boot   
	RPWM2_PWR_CG			= 4,		// Clock Gated
	RPWM2_PWR_WAKEUP		= 5,		// Wakeup event
};

enum RTL871X_RPWM_BITS {
	RPWM_WLAN_ON_BIT		= BIT0,		// Active
	RPWM_ACK_BIT			= BIT6,		// Acknowledge
	RPWM_TOGGLE_BIT			= BIT7,		// Toggle bit
};

enum RTL871X_RPWM2_BITS {
	RPWM2_ACT_BIT			= BIT0,		// Active
	RPWM2_SLEEP_BIT			= 0,		// Sleep
	RPWM2_DSTANDBY_BIT		= BIT1,		// Deep Standby
	RPWM2_PG_BIT			= 0,		// Power Gated
	RPWM2_FBOOT_BIT			= BIT2,		// fast reboot
	RPWM2_NBOOT_BIT			= 0,		// normal reboot
	RPWM2_WKPIN_A5_BIT		= BIT3,		// enable GPIO A5 wakeup
	RPWM2_WKPIN_C7_BIT		= BIT4,		// enable GPIO C7 wakeup
	RPWM2_WKPIN_D5_BIT		= BIT5,		// enable GPIO D5 wakeup
	RPWM2_WKPIN_E3_BIT		= BIT6,		// enable GPIO E3 wakeup
	RPWM2_PIN_A5_LV_BIT		= BIT7,		// GPIO A5 wakeup level
	RPWM2_PIN_C7_LV_BIT		= BIT8,		// GPIO C7 wakeup level
	RPWM2_PIN_D5_LV_BIT		= BIT9,		// GPIO D5 wakeup level
	RPWM2_PIN_E3_LV_BIT		= BIT10,	// GPIO E3 wakeup level
	RPWM2_CG_BIT			= BIT11,	// Clock Gated
	RPWM2_ACK_BIT			= BIT14,	// Acknowledge
	RPWM2_TOGGLE_BIT		= BIT15,	// Toggle bit
};

enum RTL871X_CPWM2_BITS {
	CPWM2_ACT_BIT           = BIT0,     // Active
	CPWM2_DSTANDBY_BIT      = BIT1,     // Deep Standby
	CPWM2_FBOOT_BIT         = BIT2,     // fast reboot
	CPWM2_INIC_FW_RDY_BIT   = BIT3,     // is the iNIC FW(1) or Boot FW(0)
	CPWM2_TOGGLE_BIT        = BIT15,    // Toggle bit
};
#endif

struct hal_ops {
	u32	(*hal_init)(_adapter *padapter);
	u32	(*hal_deinit)(_adapter *padapter);

	s32	(*init_xmit_priv)(_adapter *padapter);
	void	(*free_xmit_priv)(_adapter *padapter);
	
	s32	(*init_recv_priv)(_adapter *padapter);
	void	(*free_recv_priv)(_adapter *padapter);
#if defined(CONFIG_TX_AGGREGATION)||defined(CONFIG_USB_TX_AGGREGATION)
	s32	(*hal_xmit)(_adapter *padapter, struct xmit_frame *pxmitframe);
#else
	s32	(*hal_xmit)(_adapter *padapter, struct xmit_buf *pxmitbuf);
#endif
	s32	(*hal_mgnt_xmit)(_adapter *padapter, struct xmit_buf *pxmitbuf);
	s32 (*xmit_thread_handler)(_adapter *padapter);

	s32	(*run_thread)(_adapter *padapter);
	void	(*cancel_thread)(_adapter *padapter);
	
	void	(*enable_interrupt)(_adapter *padapter);
	void	(*disable_interrupt)(_adapter *padapter);
	u32	(*inirp_init)(_adapter *padapter);
	u32	(*inirp_deinit)(_adapter *padapter);
	void 	(*recv_tasklet)(void *priv);
#ifdef CONFIG_POWER_SAVING
	u32	(*hal_rpwm_notify)(_adapter *padapter, u8 rpwm_event);
#ifdef CONFIG_WOWLAN
	u32	(*hal_send_wowlan_cmd)(_adapter *padapter, u8 id, u8 *data, u16 len);
#endif
#endif
	u32 (*hal_read_mac_addr)(_adapter *padapter, u8 *mac);
};
uint rtw_hal_init(_adapter *padapter);
uint rtw_hal_deinit(_adapter *padapter);
s32	rtw_hal_init_xmit_priv(_adapter *padapter);
void	rtw_hal_free_xmit_priv(_adapter *padapter);
s32	rtw_hal_init_recv_priv(_adapter *padapter);
void	rtw_hal_free_recv_priv(_adapter *padapter);
#if defined(CONFIG_TX_AGGREGATION)||defined(CONFIG_USB_TX_AGGREGATION)
s32	rtw_hal_xmit(_adapter *padapter, struct xmit_frame *pxmitframe);
#else
s32	rtw_hal_xmit(_adapter *padapter, struct xmit_buf *pxmitbuf);
#endif
s32	rtw_hal_mgnt_xmit(_adapter *padapter, struct xmit_buf *pxmitbuf);
s32 rtw_hal_xmit_thread_handler(_adapter *padapter);
void rtw_hal_enable_interrupt(_adapter *padapter);
void rtw_hal_disable_interrupt(_adapter *padapter);
u32	rtw_hal_inirp_init(_adapter *padapter);
u32	rtw_hal_inirp_deinit(_adapter *padapter);
#if defined(CONFIG_TX_AGGREGATION)||defined(CONFIG_USB_TX_AGGREGATION)
s32	rtw_hal_start_thread(_adapter *padapter);
void	rtw_hal_stop_thread(_adapter *padapter);
#endif
s32	rtw_hal_recv_tasklet(_adapter *padapter);
#ifdef CONFIG_POWER_SAVING
u32 rtw_hal_rpwm_notify(_adapter *padapter, u8 event);
#ifdef CONFIG_WOWLAN
u32 rtw_hal_send_wowlan_cmd(_adapter *padapter, u8 id, u8 *data, u16 len);
#endif
#endif
u32 rtw_hal_read_mac_addr(_adapter *padapter, u8 *mac);
#endif
