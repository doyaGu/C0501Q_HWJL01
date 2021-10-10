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
#define _XMIT_OSDEP_C_
#include "drv_types.h"
#include "xmit_osdep.h"
#include "8195_desc.h"

void rtw_os_pkt_complete(_adapter *padapter, _pkt *pkt)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	u16	queue;

	queue = skb_get_queue_mapping(pkt);

	if(__netif_subqueue_stopped(padapter->pnetdev, queue))
		netif_wake_subqueue(padapter->pnetdev, queue);

#else
	if (netif_queue_stopped(padapter->pnetdev))
		netif_wake_queue(padapter->pnetdev);
#endif

	rtw_skb_free(pkt);
}

#if defined(CONFIG_TX_AGGREGATION)||defined(CONFIG_USB_TX_AGGREGATION)
void rtw_os_xmit_complete(_adapter *padapter, struct xmit_frame *pxframe)
{
	if(pxframe->pkt)
		rtw_os_pkt_complete(padapter, pxframe->pkt);

	pxframe->pkt = NULL;
}
#else
void rtw_os_xmit_complete(_adapter *padapter, struct xmit_buf *pxbuf)
{
	if(pxbuf->pkt)
		rtw_os_pkt_complete(padapter, pxbuf->pkt);

	pxbuf->pkt = NULL;
}
#endif

int rtw_os_xmit_resource_alloc(_adapter *padapter, struct xmit_buf *pxmitbuf, u32 alloc_sz, u8 flag)
{
	if (alloc_sz > 0) {
#ifdef CONFIG_USE_USB_BUFFER_ALLOC_TX
		struct dvobj_priv	*pdvobjpriv = padapter->dvobj;
		PUSB_DATA pusb = &pdvobjpriv->intf_data;
		struct usb_device	*pusbd = pusb->dev;

		pxmitbuf->pallocated_buf = rtw_usb_buffer_alloc(pusbd, (size_t)alloc_sz, &pxmitbuf->dma_transfer_addr);
		pxmitbuf->pbuf = pxmitbuf->pallocated_buf;
		if(pxmitbuf->pallocated_buf == NULL)
			return _FAIL;
#else // CONFIG_USE_USB_BUFFER_ALLOC_TX
		
		pxmitbuf->pallocated_buf = rtw_zmalloc(alloc_sz);
		if (pxmitbuf->pallocated_buf == NULL)
		{
			return _FAIL;
		}

		pxmitbuf->pbuf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitbuf->pallocated_buf), XMITBUF_ALIGN_SZ);

#endif // CONFIG_USE_USB_BUFFER_ALLOC_TX
	}

	if (flag) {
#ifdef CONFIG_USB_HCI
		int i;
		for(i=0; i<NR_URB_PERXMITBUF; i++)
	      	{
	      		pxmitbuf->pxmit_urb[i] = usb_alloc_urb(0, GFP_KERNEL);
	             	if(pxmitbuf->pxmit_urb[i] == NULL) 
	             	{
	             		DBG_871X("pxmitbuf->pxmit_urb[i]==NULL");
		        	return _FAIL;	 
	             	}
	      	}
#endif
	}

	return _SUCCESS;	
}

void rtw_os_xmit_resource_free(_adapter *padapter, struct xmit_buf *pxmitbuf,u32 free_sz, u8 flag)
{
	if (flag) {
#ifdef CONFIG_USB_HCI
		int i;

		for(i=0; i<NR_URB_PERXMITBUF; i++)
		{
			if(pxmitbuf->pxmit_urb[i])
			{
				//usb_kill_urb(pxmitbuf->pxmit_urb[i]);
				usb_free_urb(pxmitbuf->pxmit_urb[i]);
			}
		}
#endif
	}

	if (free_sz > 0 ) {
#ifdef CONFIG_USE_USB_BUFFER_ALLOC_TX
		struct dvobj_priv	*pdvobjpriv = padapter->dvobj;
		PUSB_DATA pusb = &pdvobjpriv->intf_data;
		struct usb_device	*pusbd = pusb->dev;

		rtw_usb_buffer_free(pusbd, (size_t)free_sz, pxmitbuf->pallocated_buf, pxmitbuf->dma_transfer_addr);
		pxmitbuf->pallocated_buf =  NULL;
		pxmitbuf->dma_transfer_addr = 0;
#else	// CONFIG_USE_USB_BUFFER_ALLOC_TX
		if(pxmitbuf->pallocated_buf)
			rtw_mfree(pxmitbuf->pallocated_buf, free_sz);
#endif	// CONFIG_USE_USB_BUFFER_ALLOC_TX
	}
}
#if defined(CONFIG_TX_AGGREGATION)||defined(CONFIG_USB_TX_AGGREGATION)
sint rtw_if_up(_adapter *padapter)	{

	sint res;
_func_enter_;		

	if( padapter->bDriverStopped || padapter->bSurpriseRemoved ||
		(padapter->fw_status == 0)){		
		RT_TRACE(_module_rtl871x_mlme_c_, _drv_info_, ("rtw_if_up:bDriverStopped(%d) OR bSurpriseRemoved(%d)", padapter->bDriverStopped, padapter->bSurpriseRemoved));	
		res=_FALSE;
	}
	else
		res=  _TRUE;
	
_func_exit_;
	return res;
}

static void rtw_check_xmit_resource(_adapter *padapter, _pkt *pkt)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
	u16	queue;

	queue = skb_get_queue_mapping(pkt);

	if(pxmitpriv->free_xmitframe_cnt<=4) {
		if (!netif_tx_queue_stopped(netdev_get_tx_queue(padapter->pnetdev, queue)))
			netif_stop_subqueue(padapter->pnetdev, queue);
	}
	
#else
	if(pxmitpriv->free_xmitframe_cnt<=4)
	{
		if (!rtw_netif_queue_stopped(padapter->pnetdev))
			rtw_netif_stop_queue(padapter->pnetdev);
	}
#endif
}

int _rtw_xmit_entry(_pkt *pkt, _nic_hdl pnetdev)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	s32 res = 0;

_func_enter_;

	RT_TRACE(_module_rtl871x_mlme_c_, _drv_info_, ("+xmit_enry\n"));
#if 1
	if (rtw_if_up(padapter) == _FALSE) {
		RT_TRACE(_module_xmit_osdep_c_, _drv_err_, ("rtw_xmit_entry: rtw_if_up fail\n"));
		#ifdef DBG_TX_DROP_FRAME
		DBG_871X("DBG_TX_DROP_FRAME %s if_up fail\n", __FUNCTION__);
		#endif
		goto drop_packet;
	}

	rtw_check_xmit_resource(padapter, pkt);
#endif
	res = rtw_xmit(padapter, pkt);
	if (res < 0) {
		#ifdef DBG_TX_DROP_FRAME
		DBG_871X("DBG_TX_DROP_FRAME %s rtw_xmit fail\n", __FUNCTION__);
		#endif
		goto drop_packet;
	}

	RT_TRACE(_module_xmit_osdep_c_, _drv_info_, ("rtw_xmit_entry: tx_pkts=%d\n", (u32)pxmitpriv->tx_pkts));
	goto exit;

drop_packet:
	pxmitpriv->tx_drop++;
	rtw_skb_free(pkt);
	RT_TRACE(_module_xmit_osdep_c_, _drv_notice_, ("rtw_xmit_entry: drop, tx_drop=%d\n", (u32)pxmitpriv->tx_drop));

exit:

_func_exit_;

	return 0;
}

#else
int _rtw_xmit_entry(_pkt *pkt, _nic_hdl pnetdev)
{
	//int ret = 0;
	PADAPTER padapter;
	struct xmit_buf *pxmitbuf;
	struct xmit_priv *pxmitpriv;
	//_irqL irqL;
	PTXDESC_8195A ptxdesc;

_func_enter_;
	padapter = (PADAPTER)rtw_netdev_priv(pnetdev);
	pxmitpriv = &padapter->xmitpriv;

	//dequeue free xmitbuf
	pxmitbuf = rtw_alloc_xmitbuf(padapter);
	if(!pxmitbuf)
	{
		DBG_871X("%s(): pxmitbuf allocated failed!\n", __FUNCTION__);
		goto drop_packet;
	}
	
	pxmitbuf->pkt_len = pkt->len + SIZE_TX_DESC_8195a;
	ptxdesc = (PTXDESC_8195A)pxmitbuf->pbuf;
	ptxdesc->txpktsize = pkt->len;
	ptxdesc->offset = SIZE_TX_DESC_8195a;
	ptxdesc->type = TX_PACKET_802_3;//indicate transmittion of 802.3 packet
	ptxdesc->bus_agg_num = 0;//to do
	rtw_memcpy(pxmitbuf->pbuf+SIZE_TX_DESC_8195a, pkt->data, pkt->len);	
	pxmitpriv->tx_pkts++;
	pxmitpriv->tx_bytes+=pkt->len;
#ifdef CONFIG_POWER_SAVING
	padapter->LinkDetectInfo.NumTxOkInPeriod ++;
#endif
	rtw_skb_free(pkt);
	if (rtw_hal_xmit(padapter, pxmitbuf) == _FALSE)
		goto drop_packet;
	goto exit;

drop_packet:
	pxmitpriv->tx_drop++;
	rtw_skb_free(pkt);

exit:

_func_exit_;

	return 0;
}
#endif
int rtw_xmit_entry(_pkt *pkt, _nic_hdl pnetdev)
{
	int ret = 0;

	if (pkt) {
		ret =  _rtw_xmit_entry(pkt, pnetdev);
	}
	return ret;
}
