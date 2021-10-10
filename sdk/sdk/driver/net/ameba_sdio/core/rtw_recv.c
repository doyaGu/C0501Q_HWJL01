#define _RTW_RECV_C_

#include "autoconf.h"
#include "rtw_debug.h"
#include "osdep_service.h"
#include "drv_types.h"
#include "rtw_recv.h"
#include "recv_osdep.h"
#include "hal_intf.h"
#include "8195_desc.h"
#include "rtl8195a_cmd.h"
#ifdef PLATFORM_FREERTOS
#include "freertos/generic.h"
#endif //PLATFORM_FREERTOS

struct recv_buf *rtw_dequeue_recvbuf (_queue *queue)
{
	_irqL irqL;
	struct recv_buf *precvbuf;
	_list	*plist, *phead;	
	_adapter *padapter;
	struct recv_priv *precvpriv;

#if defined (CONFIG_SDIO_HCI) || defined (CONFIG_GSPI_HCI)
	rtw_enter_critical_bh(&queue->lock, &irqL);
#else
	rtw_enter_critical(&queue->lock, &irqL);
#endif/*#ifdef  CONFIG_SDIO_HCI*/
	
	if(rtw_queue_empty(queue) == _TRUE)
	{
		precvbuf = NULL;
	}
	else
	{
		phead = get_list_head(queue);

		plist = get_next(phead);

		precvbuf = LIST_CONTAINOR(plist, struct recv_buf, list);

		rtw_list_delete(&precvbuf->list);

		padapter=precvbuf->adapter;
		if(padapter !=NULL){
			precvpriv=&padapter->recvpriv;
			if(queue == &precvpriv->free_recv_buf_queue){
				precvpriv->free_recv_buf_queue_cnt--;
				precvpriv->min_free_recv_buf_queue_cnt = MIN(precvpriv->min_free_recv_buf_queue_cnt, precvpriv->free_recv_buf_queue_cnt);
			}
		}		
	}

#if defined (CONFIG_SDIO_HCI) || defined (CONFIG_GSPI_HCI)
	rtw_exit_critical_bh(&queue->lock, &irqL);
#else
	rtw_exit_critical(&queue->lock, &irqL);
#endif/*#ifdef  CONFIG_SDIO_HCI*/

	return precvbuf;

}
s32 rtw_enqueue_recvbuf(struct recv_buf *precvbuf, _queue *queue)
{
	_irqL irqL;	
	_adapter *padapter = precvbuf->adapter;
	struct recv_priv *precvpriv = &padapter->recvpriv;
	
#if defined (CONFIG_SDIO_HCI) || defined (CONFIG_GSPI_HCI)
	rtw_enter_critical_bh(&queue->lock, &irqL);
#else
	rtw_enter_critical(&queue->lock, &irqL);
#endif/*#ifdef  CONFIG_SDIO_HCI*/

	rtw_list_delete(&precvbuf->list);

	rtw_list_insert_tail(&precvbuf->list, get_list_head(queue));

	if (padapter != NULL) {
		if (queue == &precvpriv->free_recv_buf_queue)
			precvpriv->free_recv_buf_queue_cnt++;
	}
	
#if defined (CONFIG_SDIO_HCI) || defined (CONFIG_GSPI_HCI)
	rtw_exit_critical_bh(&queue->lock, &irqL);
#else
	rtw_exit_critical(&queue->lock, &irqL);
#endif/*#ifdef  CONFIG_SDIO_HCI*/
	return _SUCCESS;
	
}

s32 rtw_init_recv_priv(PADAPTER padapter)
{
	s32			res;
#if defined (CONFIG_SDIO_RX_COPY) || defined(CONFIG_USB_RX_AGGREGATION)
	u32 i;
	union recv_frame *precvframe;
	struct recv_priv *precvpriv = &padapter->recvpriv;
	rtw_spinlock_init(&precvpriv->lock);
	rtw_init_queue(&precvpriv->free_recv_queue);
	rtw_init_queue(&precvpriv->recv_pending_queue);
//	rtw_init_queue(&precvpriv->uc_swdec_pending_queue);

	precvpriv->adapter = padapter;

	precvpriv->free_recvframe_cnt = NR_RECVFRAME;
	precvpriv->min_free_recvframe_cnt = precvpriv->free_recvframe_cnt;

//	rtw_os_recv_resource_init(precvpriv, padapter);

	precvpriv->pallocated_frame_buf = rtw_zvmalloc(NR_RECVFRAME * sizeof(union recv_frame) + RXFRAME_ALIGN_SZ);
	
	if(precvpriv->pallocated_frame_buf==NULL){
		res= _FAIL;
		goto exit;
	}

	precvpriv->precv_frame_buf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(precvpriv->pallocated_frame_buf), RXFRAME_ALIGN_SZ);
	
	precvframe = (union recv_frame*) precvpriv->precv_frame_buf;

	for(i=0; i < NR_RECVFRAME ; i++)
	{
		rtw_init_listhead(&(precvframe->u.list));

		rtw_list_insert_tail(&(precvframe->u.list), &(precvpriv->free_recv_queue.queue));

//		res = rtw_os_recv_resource_alloc(padapter, precvframe);
		precvframe->u.hdr.pkt = NULL;

		precvframe->u.hdr.len = 0;

		precvframe->u.hdr.adapter =padapter;
		precvframe++;

	}

#endif
	res = rtw_hal_init_recv_priv(padapter);
	goto exit;
exit:
_func_exit_;
	return res;
}

void rtw_free_recv_priv(PADAPTER padapter)
{
#if defined (CONFIG_SDIO_RX_COPY) || defined(CONFIG_USB_RX_AGGREGATION)

	struct recv_priv *precvpriv = &padapter->recvpriv;
//	rtw_mfree_recv_priv_lock(precvpriv);
	rtw_spinlock_free(&precvpriv->lock);
	rtw_os_recv_resource_free(precvpriv);

	if(precvpriv->pallocated_frame_buf) {
		rtw_vmfree(precvpriv->pallocated_frame_buf, NR_RECVFRAME * sizeof(union recv_frame) + RXFRAME_ALIGN_SZ);
	}
#endif
	rtw_hal_free_recv_priv(padapter);
}

#if defined(CONFIG_SDIO_RX_COPY) || defined(CONFIG_USB_RX_AGGREGATION)
union recv_frame *_rtw_alloc_recvframe (_queue *pfree_recv_queue)
{

	union recv_frame  *precvframe;
	_list	*plist, *phead;
	_adapter *padapter;
	struct recv_priv *precvpriv;
_func_enter_;

	if(rtw_queue_empty(pfree_recv_queue) == _TRUE)
	{
		precvframe = NULL;
	}
	else
	{
		phead = get_list_head(pfree_recv_queue);

		plist = get_next(phead);

		precvframe = LIST_CONTAINOR(plist, union recv_frame, u);

		rtw_list_delete(&precvframe->u.hdr.list);
		padapter=precvframe->u.hdr.adapter;
		if(padapter !=NULL){
			precvpriv=&padapter->recvpriv;
			if(pfree_recv_queue == &precvpriv->free_recv_queue){
				precvpriv->free_recvframe_cnt--;
				precvpriv->min_free_recvframe_cnt = MIN(precvpriv->min_free_recvframe_cnt, precvpriv->free_recvframe_cnt);
			}
		}
	}

_func_exit_;

	return precvframe;

}

union recv_frame *rtw_alloc_recvframe (_queue *pfree_recv_queue)
{
	_irqL irqL;
	union recv_frame  *precvframe;
	
	rtw_enter_critical_bh(&pfree_recv_queue->lock, &irqL);

	precvframe = _rtw_alloc_recvframe(pfree_recv_queue);

	rtw_exit_critical_bh(&pfree_recv_queue->lock, &irqL);

	return precvframe;
}

void rtw_init_recvframe(union recv_frame *precvframe, struct recv_priv *precvpriv)
{
	/* Perry: This can be removed */
	rtw_init_listhead(&precvframe->u.hdr.list);

	precvframe->u.hdr.len=0;
}

int rtw_free_recvframe(union recv_frame *precvframe, _queue *pfree_recv_queue)
{
	_irqL irqL;
	_adapter *padapter=precvframe->u.hdr.adapter;
	struct recv_priv *precvpriv = &padapter->recvpriv;

_func_enter_;

	rtw_os_free_recvframe(precvframe);

	rtw_enter_critical_bh(&pfree_recv_queue->lock, &irqL);

	rtw_list_delete(&(precvframe->u.hdr.list));

	precvframe->u.hdr.len = 0;

	rtw_list_insert_tail(&(precvframe->u.hdr.list), get_list_head(pfree_recv_queue));

	if(padapter !=NULL){
		if(pfree_recv_queue == &precvpriv->free_recv_queue){
				precvpriv->free_recvframe_cnt++;
		}
	}

      rtw_exit_critical_bh(&pfree_recv_queue->lock, &irqL);

_func_exit_;

	return _SUCCESS;

}

sint _rtw_enqueue_recvframe(union recv_frame *precvframe, _queue *queue)
{

	_adapter *padapter=precvframe->u.hdr.adapter;
	struct recv_priv *precvpriv = &padapter->recvpriv;

_func_enter_;

	//rtw_init_listhead(&(precvframe->u.hdr.list));
	rtw_list_delete(&(precvframe->u.hdr.list));

	rtw_list_insert_tail(&(precvframe->u.hdr.list), get_list_head(queue));

	if (padapter != NULL) {
		if (queue == &precvpriv->free_recv_queue)
			precvpriv->free_recvframe_cnt++;
	}

_func_exit_;

	return _SUCCESS;
}

sint rtw_enqueue_recvframe(union recv_frame *precvframe, _queue *queue)
{
	sint ret;
	_irqL irqL;
	
	//_spinlock(&pfree_recv_queue->lock);
	rtw_enter_critical_bh(&queue->lock, &irqL);
	ret = _rtw_enqueue_recvframe(precvframe, queue);
	//rtw_spin_unlock(&pfree_recv_queue->lock);
	rtw_exit_critical_bh(&queue->lock, &irqL);

	return ret;
}

/*
sint	rtw_enqueue_recvframe(union recv_frame *precvframe, _queue *queue)
{
	return rtw_free_recvframe(precvframe, queue);
}
*/




/*
caller : defrag ; recvframe_chk_defrag in recv_thread  (passive)
pframequeue: defrag_queue : will be accessed in recv_thread  (passive)

using spinlock to protect

*/

void rtw_free_recvframe_queue(_queue *pframequeue,  _queue *pfree_recv_queue)
{
	union	recv_frame 	*precvframe;
	_list	*plist, *phead;

_func_enter_;
	rtw_spin_lock(&pframequeue->lock);

	phead = get_list_head(pframequeue);
	plist = get_next(phead);

	while(rtw_end_of_queue_search(phead, plist) == _FALSE)
	{
		precvframe = LIST_CONTAINOR(plist, union recv_frame, u);

		plist = get_next(plist);

		//rtw_list_delete(&precvframe->u.hdr.list); // will do this in rtw_free_recvframe()

		rtw_free_recvframe(precvframe, pfree_recv_queue);
	}

	rtw_spin_unlock(&pframequeue->lock);

_func_exit_;

}

sint rtw_enqueue_recvbuf_to_head(struct recv_buf *precvbuf, _queue *queue)
{
	_irqL irqL;

	rtw_enter_critical_bh(&queue->lock, &irqL);

	rtw_list_delete(&precvbuf->list);
	rtw_list_insert_head(&precvbuf->list, get_list_head(queue));

	rtw_exit_critical_bh(&queue->lock, &irqL);

	return _SUCCESS;
}

s32 rtw_recv_entry(union recv_frame *precvframe)
{
	_adapter *padapter;
	struct recv_priv *precvpriv;
	s32 ret=_SUCCESS;

_func_enter_;

//	RT_TRACE(_module_rtl871x_recv_c_,_drv_info_,("+rtw_recv_entry\n"));

	padapter = precvframe->u.hdr.adapter;

	precvpriv = &padapter->recvpriv;

	if ((padapter->bDriverStopped == _FALSE) && (padapter->bSurpriseRemoved == _FALSE))
	{
		RT_TRACE(_module_rtl871x_recv_c_, _drv_alert_, ("@@@@ recv_func: recv_func rtw_recv_indicatepkt\n" ));
		//indicate this recv_frame
		ret = rtw_recv_indicatepkt(padapter, precvframe);
		if (ret != _SUCCESS)
		{	
			#ifdef DBG_RX_DROP_FRAME
			DBG_871X("DBG_RX_DROP_FRAME %s rtw_recv_indicatepkt fail!\n", __FUNCTION__);
			#endif
			goto _recv_entry_drop;
		}
	}
	else
	{
		RT_TRACE(_module_rtl871x_recv_c_, _drv_alert_, ("@@@@  recv_func: rtw_free_recvframe\n" ));
		RT_TRACE(_module_rtl871x_recv_c_, _drv_debug_, ("recv_func:bDriverStopped(%d) OR bSurpriseRemoved(%d)", padapter->bDriverStopped, padapter->bSurpriseRemoved));
		#ifdef DBG_RX_DROP_FRAME
		DBG_871X("DBG_RX_DROP_FRAME %s ecv_func:bDriverStopped(%d) OR bSurpriseRemoved(%d)\n", __FUNCTION__,
			padapter->bDriverStopped, padapter->bSurpriseRemoved);
		#endif
		ret = _FAIL;
		goto _recv_entry_drop;
	}


_func_exit_;

	return ret;

_recv_entry_drop:
	//RT_TRACE(_module_rtl871x_recv_c_,_drv_err_,("_recv_entry_drop\n"));

_func_exit_;

	return ret;
}

#else
int rtw_recv_entry(PADAPTER padapter, struct recv_buf *precvbuf)
{
	int ret = _SUCCESS;
	struct recv_priv *precvpriv = &padapter->recvpriv;
	RXDESC_8195A rxdesc;
	
	rtw_memcpy(&rxdesc, precvbuf->pdata, SIZE_RX_DESC_8195a);
	//remove the rx header
	recvbuf_pull(precvbuf, rxdesc.offset);

	if(rxdesc.type == RX_PACKET_802_3)//data pkt 
	{	
		if ((padapter->bDriverStopped == _FALSE) && (padapter->bSurpriseRemoved == _FALSE))
		{
			//RT_TRACE(_module_rtl871x_recv_c_, _drv_alert_, ("@@@@ recv_func: recv_func rtw_recv_indicatepkt\n" ));
			//indicate this recv_frame
			ret = rtw_recv_indicatepkt(padapter, precvbuf);
			if (ret != _SUCCESS)
			{	
				#ifdef DBG_RX_DROP_FRAME
				DBG_871X("DBG_RX_DROP_FRAME %s rtw_recv_indicatepkt fail!\n", __FUNCTION__);
				#endif
				goto _recv_data_drop;
			}
		}
		else
		{
			RT_TRACE(_module_rtl871x_recv_c_, _drv_alert_, ("@@@@  recv_func: rtw_free_recvframe\n" ));
			RT_TRACE(_module_rtl871x_recv_c_, _drv_debug_, ("recv_func:bDriverStopped(%d) OR bSurpriseRemoved(%d)", padapter->bDriverStopped, padapter->bSurpriseRemoved));
			#ifdef DBG_RX_DROP_FRAME
			DBG_871X("DBG_RX_DROP_FRAME %s ecv_func:bDriverStopped(%d) OR bSurpriseRemoved(%d)\n", __FUNCTION__,
				padapter->bDriverStopped, padapter->bSurpriseRemoved);
			#endif
			ret = _FAIL;
			goto _recv_data_drop;
		}
	}
	else if(rxdesc.type == RX_C2H_CMD)//cmd pkt
	{

		rtl8195a_c2h_cmd_handler(padapter, precvbuf->pdata, rxdesc.pkt_len);

		rtw_skb_free(precvbuf->pskb);
		goto _free_recv_buf;
	}

_recv_data_drop:
	if(ret == _FAIL){
		precvpriv->rx_drop++;
	}
_free_recv_buf:
	precvbuf->pskb = NULL;
	rtw_enqueue_recvbuf(precvbuf, &precvpriv->free_recv_buf_queue);
	return ret;
}
#endif
