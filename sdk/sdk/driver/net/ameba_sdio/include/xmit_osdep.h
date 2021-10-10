/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
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
#ifndef __XMIT_OSDEP_H_
#define __XMIT_OSDEP_H_
#include "drv_types.h"

#ifdef PLATFORM_LINUX

struct xmit_priv;
struct xmit_buf;
#if defined(CONFIG_TX_AGGREGATION)||defined(CONFIG_USB_TX_AGGREGATION)
#define NR_XMITFRAME	256
struct xmit_frame;
#endif

extern int _rtw_xmit_entry(_pkt *pkt, _nic_hdl pnetdev);
extern int rtw_xmit_entry(_pkt *pkt, _nic_hdl pnetdev);
#endif //#ifdef PLATFORM_LINUX

#ifdef PLATFORM_FREERTOS
//Decrease xmit frame due to memory limitation - Alex Fang
#if USE_XMIT_EXTBUFF
#define NR_XMITFRAME	16	//NR_XMITBUFF + NR_XMIT_EXTBUFF
#else
//#define NR_XMITFRAME	8
#define NR_XMITFRAME	6 //Decrease recv frame due to memory limitation - YangJue 
#endif

int rtw_xmit_entry(_pkt *pkt, _nic_hdl pnetdev);
#endif /*PLATFORM_FREERTOS*/

#ifdef PLATFORM_ECOS
#define NR_XMITFRAME		(1024)
int rtw_xmit_entry(_pkt *pkt, _nic_hdl pnetdev);
#endif

#if defined(CONFIG_TX_AGGREGATION)||defined(CONFIG_USB_TX_AGGREGATION)
void rtw_os_xmit_complete(_adapter *padapter, struct xmit_frame *pxframe);
#else
void rtw_os_xmit_complete(_adapter *padapter, struct xmit_buf *pxbuf);
#endif
extern void rtw_os_pkt_complete(_adapter *padapter, _pkt *pkt);
int rtw_os_xmit_resource_alloc(_adapter *padapter, struct xmit_buf *pxmitbuf, u32 alloc_sz, u8 flag);
void rtw_os_xmit_resource_free(_adapter *padapter, struct xmit_buf *pxmitbuf, u32 free_sz, u8 flag);

#endif //__XMIT_OSDEP_H_

