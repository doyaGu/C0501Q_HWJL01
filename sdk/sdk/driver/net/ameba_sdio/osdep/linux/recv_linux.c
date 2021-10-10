#define _RECV_OSDEP_C_
#include "autoconf.h"
#include "rtw_debug.h"
#include "osdep_service.h"
#include "drv_types.h"
#include "rtw_recv.h"
#include "hal_intf.h"

void rtw_os_recv_indicate_pkt(_adapter *padapter, _pkt *pkt)
{

	/* Indicat the packets to upper layer */
	if (pkt) {
		pkt->protocol = eth_type_trans(pkt, padapter->pnetdev);
		pkt->dev = padapter->pnetdev;
		pkt->ip_summed = CHECKSUM_NONE;
		rtw_netif_rx(padapter->pnetdev, pkt);
	}
}
#if defined(CONFIG_SDIO_RX_COPY) || defined(CONFIG_USB_RX_AGGREGATION)
int rtw_recv_indicatepkt(_adapter *padapter, union recv_frame *precv_frame)
{
	struct recv_priv *precvpriv;
	_queue	*pfree_recv_queue;
	_pkt *skb;
//	struct mlme_priv*pmlmepriv = &padapter->mlmepriv;
//	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;

_func_enter_;

	precvpriv = &(padapter->recvpriv);
	pfree_recv_queue = &(precvpriv->free_recv_queue);

	skb = precv_frame->u.hdr.pkt;
	if(skb == NULL)
	{
		RT_TRACE(_module_recv_osdep_c_,_drv_err_,("rtw_recv_indicatepkt():skb==NULL something wrong!!!!\n"));
		goto _recv_indicatepkt_drop;
	}

	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("rtw_recv_indicatepkt():skb != NULL !!!\n"));
	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("rtw_recv_indicatepkt():precv_frame->u.hdr.rx_head=%p  precv_frame->hdr.rx_data=%p\n", precv_frame->u.hdr.rx_head, precv_frame->u.hdr.rx_data));
	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("precv_frame->hdr.rx_tail=%p precv_frame->u.hdr.rx_end=%p precv_frame->hdr.len=%d \n", precv_frame->u.hdr.rx_tail, precv_frame->u.hdr.rx_end, precv_frame->u.hdr.len));

	skb->data = precv_frame->u.hdr.rx_data;

	skb_set_tail_pointer(skb, precv_frame->u.hdr.len);

	skb->len = precv_frame->u.hdr.len;

	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("\n skb->head=%p skb->data=%p skb->tail=%p skb->end=%p skb->len=%d\n", skb->head, skb->data, skb_tail_pointer(skb), skb_end_pointer(skb), skb->len));

#ifdef CONFIG_POWER_SAVING
	padapter->LinkDetectInfo.NumRxOkInPeriod++;
	if( (!MacAddr_isBcst(skb->data)) && (!IS_MCAST(skb->data))){
		//DBG_871X("Not a broadcast or multicase\n");
		padapter->LinkDetectInfo.NumRxUnicastOkInPeriod++;
	}
#endif

	rtw_os_recv_indicate_pkt(padapter, skb);
	precvpriv->rx_bytes += skb->len;
	precvpriv->rx_pkts++;

//_recv_indicatepkt_end:
	precv_frame->u.hdr.pkt = NULL; // pointers to NULL before rtw_free_recvframe()

	rtw_free_recvframe(precv_frame, pfree_recv_queue);

	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("\n rtw_recv_indicatepkt :after rtw_os_recv_indicate_pkt!!!!\n"));

_func_exit_;

        return _SUCCESS;

_recv_indicatepkt_drop:

	 //enqueue back to free_recv_queue
	 if(precv_frame)
		 rtw_free_recvframe(precv_frame, pfree_recv_queue);

	precvpriv->rx_drop++;

	 return _FAIL;

_func_exit_;

}

//free os related resource in union recv_frame
void rtw_os_recv_resource_free(struct recv_priv *precvpriv)
{
	sint i;
	union recv_frame *precvframe;
	precvframe = (union recv_frame*) precvpriv->precv_frame_buf;

	for(i=0; i < NR_RECVFRAME; i++)
	{
		if(precvframe->u.hdr.pkt)
		{
			rtw_skb_free(precvframe->u.hdr.pkt);//free skb by driver
			precvframe->u.hdr.pkt = NULL;
		}
		precvframe++;
	}
}

void rtw_os_free_recvframe(union recv_frame *precvframe)
{
	if(precvframe->u.hdr.pkt)
	{
		rtw_skb_free(precvframe->u.hdr.pkt);//free skb by driver

		precvframe->u.hdr.pkt = NULL;
	}
}

#else
int rtw_recv_indicatepkt(_adapter *padapter, struct recv_buf *precvbuf)
{
	struct recv_priv *precvpriv;
	_pkt *skb;

_func_enter_;

	precvpriv = &(padapter->recvpriv);

	skb = precvbuf->pskb;
	if(skb == NULL)
	{
		RT_TRACE(_module_recv_osdep_c_,_drv_err_,("rtw_recv_indicatepkt():skb==NULL something wrong!!!!\n"));
		goto _recv_indicatepkt_drop;
	}

	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("rtw_recv_indicatepkt():skb != NULL !!!\n"));		

	skb->data = precvbuf->pdata;

	skb_set_tail_pointer(skb, precvbuf->len);

	skb->len = precvbuf->len;

	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("\n skb->head=%p skb->data=%p skb->tail=%p skb->end=%p skb->len=%d\n", skb->head, skb->data, skb_tail_pointer(skb), skb_end_pointer(skb), skb->len));

#ifdef CONFIG_POWER_SAVING
	padapter->LinkDetectInfo.NumRxOkInPeriod++;
	if( (!MacAddr_isBcst(skb->data)) && (!IS_MCAST(skb->data))){
		//DBG_871X("Not a broadcast or multicase\n");
		padapter->LinkDetectInfo.NumRxUnicastOkInPeriod++;
	}
#endif

	rtw_os_recv_indicate_pkt(padapter, skb);
	precvpriv->rx_bytes += skb->len;
	precvpriv->rx_pkts++;

	RT_TRACE(_module_recv_osdep_c_,_drv_info_,("\n rtw_recv_indicatepkt :after rtw_os_recv_indicate_pkt!!!!\n"));

_func_exit_;
        return _SUCCESS;

_recv_indicatepkt_drop:
_func_exit_;
	 return _FAIL;
}
#endif

//alloc os related resource in struct recv_buf
int rtw_os_recvbuf_resource_alloc(_adapter *padapter, struct recv_buf *precvbuf)
{
	int res=_SUCCESS;

#ifdef CONFIG_USB_HCI
	struct dvobj_priv	*pdvobjpriv = padapter->dvobj;
	PUSB_DATA pusb = &pdvobjpriv->intf_data;
	struct usb_device	*pusbd = pusb->dev;

//	precvbuf->irp_pending = _FALSE;
	precvbuf->purb = usb_alloc_urb(0, GFP_KERNEL);
	if(precvbuf->purb == NULL){
		res = _FAIL;
	}

	precvbuf->pskb = NULL;

//	precvbuf->reuse = _FALSE;

	precvbuf->pallocated_buf  = precvbuf->pbuf = NULL;

	precvbuf->pdata = precvbuf->phead = precvbuf->ptail = precvbuf->pend = NULL;

	precvbuf->transfer_len = 0;

	precvbuf->len = 0;
	
#ifdef CONFIG_USE_USB_BUFFER_ALLOC_RX
	precvbuf->pallocated_buf = rtw_usb_buffer_alloc(pusbd, (size_t)precvbuf->alloc_sz, &precvbuf->dma_transfer_addr);
	precvbuf->pbuf = precvbuf->pallocated_buf;
	if(precvbuf->pallocated_buf == NULL)
		return _FAIL;
#endif //CONFIG_USE_USB_BUFFER_ALLOC_RX

	
#endif //CONFIG_USB_HCI

	return res;
}

//free os related resource in struct recv_buf
int rtw_os_recvbuf_resource_free(_adapter *padapter, struct recv_buf *precvbuf)
{
	int ret = _SUCCESS;

#ifdef CONFIG_USB_HCI

#ifdef CONFIG_USE_USB_BUFFER_ALLOC_RX

	struct dvobj_priv	*pdvobjpriv = padapter->dvobj;
	PUSB_DATA pusb = &pdvobjpriv->intf_data;
	struct usb_device	*pusbd = pusb->dev;

	rtw_usb_buffer_free(pusbd, (size_t)precvbuf->alloc_sz, precvbuf->pallocated_buf, precvbuf->dma_transfer_addr);
	precvbuf->pallocated_buf =  NULL;
	precvbuf->dma_transfer_addr = 0;

#endif //CONFIG_USE_USB_BUFFER_ALLOC_RX

	if(precvbuf->purb)
	{
		//usb_kill_urb(precvbuf->purb);
		usb_free_urb(precvbuf->purb);
	}

#endif //CONFIG_USB_HCI


	if(precvbuf->pskb)
		rtw_skb_free(precvbuf->pskb);


	return ret;

}

#ifdef CONFIG_USB_RX_AGGREGATION
int rtw_os_alloc_recvframe(_adapter *padapter, union recv_frame *precvframe, u8 *pdata, _pkt *pskb)
{
	int res = _SUCCESS;
	u32	skb_len, alloc_sz = 0;
	_pkt	 *pkt_copy = NULL;	
	struct rx_pkt_attrib *pattrib = &precvframe->u.hdr.attrib;


	if(pdata == NULL)
	{		
		precvframe->u.hdr.pkt = NULL;
		res = _FAIL;
		return res;
	}	

	skb_len = pattrib->pkt_len;
	
	//	8 is for skb->data 4 bytes alignment.
	alloc_sz += skb_len + 8;

	pkt_copy = rtw_skb_alloc(alloc_sz);

	if(pkt_copy)
	{
		pkt_copy->dev = padapter->pnetdev;
		precvframe->u.hdr.pkt = pkt_copy;
		skb_reserve(pkt_copy, 8 - ((SIZE_PTR)( pkt_copy->data) & 7 ));//force pkt_copy->data at 8-byte alignment address
		rtw_memcpy(pkt_copy->data, pdata, skb_len);
		precvframe->u.hdr.rx_head = pkt_copy->data;
		precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail = pkt_copy->data;
		precvframe->u.hdr.rx_end = skb_end_pointer(pkt_copy);
	}
	else
	{
#ifdef CONFIG_USE_USB_BUFFER_ALLOC_RX
		DBG_871X("%s:can not allocate memory for skb copy\n", __FUNCTION__);

		precvframe->u.hdr.pkt = NULL;

		//rtw_free_recvframe(precvframe, pfree_recv_queue);
		//goto _exit_recvbuf2recvframe;

		res = _FAIL;	
#else
		if((pattrib->mfrag == 1)&&(pattrib->frag_num == 0))
		{				
			DBG_871X("%s: alloc_skb fail , drop frag frame \n", __FUNCTION__);
			//rtw_free_recvframe(precvframe, pfree_recv_queue);
			return _FAIL;
		}

		if(pskb == NULL)
		{
			return _FAIL;
		}
			
		precvframe->u.hdr.pkt = rtw_skb_clone(pskb);
		if(precvframe->u.hdr.pkt)
		{
			precvframe->u.hdr.rx_head = precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail = pdata;
			precvframe->u.hdr.rx_end =  pdata + alloc_sz;
		}
		else
		{
			DBG_871X("%s: rtw_skb_clone fail\n", __FUNCTION__);
			//rtw_free_recvframe(precvframe, pfree_recv_queue);
			//goto _exit_recvbuf2recvframe;
			res = _FAIL;
		}
#endif			
	}		
return res;
}
#endif
