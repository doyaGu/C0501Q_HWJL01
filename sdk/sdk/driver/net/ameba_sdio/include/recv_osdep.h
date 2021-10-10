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
#ifndef __RECV_OSDEP_H_
#define __RECV_OSDEP_H_
#include "drv_types.h"
#if defined(CONFIG_SDIO_RX_COPY) || defined(CONFIG_USB_RX_AGGREGATION)
extern s32 rtw_recv_entry(union recv_frame *precvframe);
int rtw_os_alloc_recvframe(_adapter *padapter, union recv_frame *precvframe, u8 *pdata, _pkt *pskb);
void rtw_os_free_recvframe(union recv_frame *precvframe);
void rtw_os_recv_resource_free(struct recv_priv *precvpriv);
void rtw_os_recv_indicate_pkt(_adapter *padapter, _pkt *pkt);
int rtw_recv_indicatepkt(_adapter *padapter, union recv_frame *precv_frame);
#else
extern int rtw_recv_entry(PADAPTER padapter, struct recv_buf *precvbuf);
extern int rtw_recv_indicatepkt(_adapter *padapter, struct recv_buf *precvbuf);
#endif
int rtw_os_recvbuf_resource_alloc(_adapter *padapter, struct recv_buf *precvbuf);
int rtw_os_recvbuf_resource_free(_adapter *padapter, struct recv_buf *precvbuf);

#endif