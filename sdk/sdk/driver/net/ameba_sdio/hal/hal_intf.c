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

#define _HAL_INTF_C_
#include <drv_types.h>
#include <hal_intf.h>

s32	rtw_hal_init_xmit_priv(_adapter *padapter)
{	
	if(padapter->HalFunc.init_xmit_priv)
		return padapter->HalFunc.init_xmit_priv(padapter);

	return _FAIL;
}
void	rtw_hal_free_xmit_priv(_adapter *padapter)
{
	
	if(padapter->HalFunc.free_xmit_priv)
		padapter->HalFunc.free_xmit_priv(padapter);
}
s32	rtw_hal_init_recv_priv(_adapter *padapter)
{	
	if(padapter->HalFunc.init_recv_priv)
		return padapter->HalFunc.init_recv_priv(padapter);

	return _FAIL;
}
void	rtw_hal_free_recv_priv(_adapter *padapter)
{
	
	if(padapter->HalFunc.free_recv_priv)
		padapter->HalFunc.free_recv_priv(padapter);
}
uint	 rtw_hal_init(_adapter *padapter) 
{
	u32 ret = _FAIL;
	if(padapter->HalFunc.hal_init)
		ret = padapter->HalFunc.hal_init(padapter);

	if(ret == _SUCCESS)
		padapter->hw_init_completed = _TRUE;
	else
		padapter->hw_init_completed = _FALSE;
	
	return ret;	
}
uint	 rtw_hal_deinit(_adapter *padapter) 
{
	padapter->hw_init_completed = _FALSE;
	if(padapter->HalFunc.hal_deinit)
		return padapter->HalFunc.hal_deinit(padapter);

	return _FAIL;	
}

u32	rtw_hal_inirp_init(_adapter *padapter)
{
	if(padapter->HalFunc.inirp_init)	
		return padapter->HalFunc.inirp_init(padapter);	
	return _FAIL;
}
	
u32	rtw_hal_inirp_deinit(_adapter *padapter)
{
	
	if(padapter->HalFunc.inirp_deinit)
		return padapter->HalFunc.inirp_deinit(padapter);

	return _FAIL;	
}

#if defined (CONFIG_TX_AGGREGATION) || defined(CONFIG_USB_TX_AGGREGATION) 
s32	rtw_hal_xmit(_adapter *padapter, struct xmit_frame *pxmitframe)
{
	if(padapter->HalFunc.hal_xmit)
		return padapter->HalFunc.hal_xmit(padapter, pxmitframe);

	return _FALSE;	
}
#else
s32	rtw_hal_xmit(_adapter *padapter, struct xmit_buf *pxmitbuf)
{
	if(padapter->HalFunc.hal_xmit)
		return padapter->HalFunc.hal_xmit(padapter, pxmitbuf);

	return _FAIL;	
}
#endif
s32	rtw_hal_mgnt_xmit(_adapter *padapter, struct xmit_buf *pxmitbuf)
{
	if(padapter->HalFunc.hal_mgnt_xmit)
		return padapter->HalFunc.hal_mgnt_xmit(padapter, pxmitbuf);

	return _FAIL;	
}
s32 rtw_hal_xmit_thread_handler(_adapter *padapter)
{
	if(padapter->HalFunc.xmit_thread_handler)
		return padapter->HalFunc.xmit_thread_handler(padapter);
	return _FAIL;
}
void rtw_hal_enable_interrupt(_adapter *padapter)
{
	if (padapter->HalFunc.enable_interrupt)
		padapter->HalFunc.enable_interrupt(padapter);
	else 
		DBG_871X("%s: HalFunc.enable_interrupt is NULL!\n", __FUNCTION__);
	
}
void rtw_hal_disable_interrupt(_adapter *padapter)
{
	if (padapter->HalFunc.disable_interrupt)
		padapter->HalFunc.disable_interrupt(padapter);
	else
		DBG_871X("%s: HalFunc.disable_interrupt is NULL!\n", __FUNCTION__);	
}

#if defined(CONFIG_TX_AGGREGATION)||defined(CONFIG_USB_TX_AGGREGATION)
/*	Start specifical interface thread		*/
s32	rtw_hal_start_thread(_adapter *padapter)
{
	if(padapter->HalFunc.run_thread)
		return padapter->HalFunc.run_thread(padapter);
	return _FAIL;
}
/*	Start specifical interface thread		*/
void	rtw_hal_stop_thread(_adapter *padapter)
{
	if(padapter->HalFunc.cancel_thread)
		padapter->HalFunc.cancel_thread(padapter);
}
#endif

#ifdef PLATFORM_FREERTOS
s32	rtw_hal_recv_tasklet(_adapter *padapter)
{
	if(padapter->HalFunc.recv_tasklet) {
		padapter->HalFunc.recv_tasklet(padapter);
		return _TRUE;
	}
	return _FAIL;
}
#endif //PLATFORM_FREERTOS

#ifdef CONFIG_POWER_SAVING
u32 rtw_hal_rpwm_notify(_adapter *padapter, u8 event)
{
	if(padapter->HalFunc.hal_rpwm_notify)
		return padapter->HalFunc.hal_rpwm_notify(padapter, event);
	return _FAIL;
}

#ifdef CONFIG_WOWLAN
u32	rtw_hal_send_wowlan_cmd(_adapter *padapter, u8 id, u8 *data, u16 len){
	if(padapter->HalFunc.hal_send_wowlan_cmd)
		return padapter->HalFunc.hal_send_wowlan_cmd(padapter, id, data, len);
	return _FAIL;
}
#endif
#endif

u32 rtw_hal_read_mac_addr(_adapter *padapter, u8 *mac){
	if(padapter->HalFunc.hal_read_mac_addr)
		return padapter->HalFunc.hal_read_mac_addr(padapter, mac);
	return _FAIL;
}