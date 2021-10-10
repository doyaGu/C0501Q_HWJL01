#define _RTW_XMIT_C_

#include "autoconf.h"
#include "rtw_debug.h"
#include "drv_types.h"
#include "rtw_xmit.h"
#include "rtw_ioctl.h"
#include "hal_intf.h"
#include "xmit_osdep.h"
#include "osdep_service.h"
#ifdef PLATFORM_FREERTOS
#include "freertos/freertos_skbuff.h"
#endif /*PLATFORM_FREERTOS*/


void rtw_sctx_init(struct submit_ctx *sctx, int timeout_ms)
{
	sctx->timeout_ms = timeout_ms;
	sctx->submit_time= rtw_get_current_time();
#ifdef PLATFORM_LINUX /* TODO: add condition wating interface for other os */
	init_completion(&sctx->done);
#endif
	sctx->status = RTW_SCTX_SUBMITTED;
}

int rtw_sctx_wait(struct submit_ctx *sctx, const char *msg)
{
	int ret = _FAIL;
	int status = 0;

#ifdef PLATFORM_LINUX
	unsigned long expire = 0; 
	expire= sctx->timeout_ms ? msecs_to_jiffies(sctx->timeout_ms) : MAX_SCHEDULE_TIMEOUT;
	if (!wait_for_completion_timeout(&sctx->done, expire)) {
		/* timeout, do something?? */
		status = RTW_SCTX_DONE_TIMEOUT;
		DBG_871X("%s timeout: %s\n", __func__, msg);
	} else {
		status = sctx->status;
	}
#endif

	if (status == RTW_SCTX_DONE_SUCCESS) {
		ret = _SUCCESS;
	}

	return ret;
}

bool rtw_sctx_chk_waring_status(int status)
{
	switch(status) {
	case RTW_SCTX_DONE_UNKNOWN:
	case RTW_SCTX_DONE_BUF_ALLOC:
	case RTW_SCTX_DONE_BUF_FREE:

	case RTW_SCTX_DONE_DRV_STOP:
	case RTW_SCTX_DONE_DEV_REMOVE:
		return _TRUE;
	default:
		return _FALSE;
	}
}

void rtw_sctx_done_err(struct submit_ctx **sctx, int status)
{
	if (*sctx) {
		if (rtw_sctx_chk_waring_status(status))
			DBG_871X("%s status:%d\n", __func__, status);
		(*sctx)->status = status;
		#ifdef PLATFORM_LINUX
		complete(&((*sctx)->done));
		#endif
		*sctx = NULL;
	}
}

void rtw_sctx_done(struct submit_ctx **sctx)
{
	rtw_sctx_done_err(sctx, RTW_SCTX_DONE_SUCCESS);
}

s32 rtw_init_xmit_priv(PADAPTER padapter)
{
	struct xmit_buf *pxmitbuf;
#if defined(CONFIG_TX_AGGREGATION)||defined(CONFIG_USB_TX_AGGREGATION)
	struct xmit_frame *pxframe;
#endif
	sint	res=_SUCCESS;
	int i , j=0;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	
	rtw_init_queue(&pxmitpriv->free_xmit_queue);
	rtw_init_queue(&pxmitpriv->xmitbuf_pending_queue);
	rtw_spinlock_init(&pxmitpriv->lock_sctx);
	rtw_spinlock_init(&pxmitpriv->lock);
	rtw_init_sema(&pxmitpriv->xmit_sema, 0);
	rtw_init_sema(&pxmitpriv->XmitTerminateSema, 0);
	
	pxmitpriv->padapter = padapter;

#if defined(CONFIG_TX_AGGREGATION)||defined(CONFIG_USB_TX_AGGREGATION)
	rtw_init_queue(&pxmitpriv->free_xmitframe_queue);
	rtw_init_queue(&pxmitpriv->xmitframe_pending_queue);

	/*	
	Please allocate memory with the sz = (struct xmit_frame) * NR_XMITFRAME, 
	and initialize free_xmit_frame below.
	Please also apply  free_txobj to link_up all the xmit_frames...
	*/

	pxmitpriv->pallocated_frame_buf = rtw_zvmalloc(NR_XMITFRAME * sizeof(struct xmit_frame) + 4);
	
	if (pxmitpriv->pallocated_frame_buf  == NULL){
		pxmitpriv->pxmit_frame_buf =NULL;
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("alloc xmit_frame fail!\n"));	
		res= _FAIL;
		goto exit;
	}
	pxmitpriv->pxmit_frame_buf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitpriv->pallocated_frame_buf), 4);
	//pxmitpriv->pxmit_frame_buf = pxmitpriv->pallocated_frame_buf + 4 -
	//						((SIZE_PTR) (pxmitpriv->pallocated_frame_buf) &3);

	pxframe = (struct xmit_frame*) pxmitpriv->pxmit_frame_buf;

	for (i = 0; i < NR_XMITFRAME; i++)
	{
		rtw_init_listhead(&(pxframe->list));

		pxframe->padapter = padapter;
		pxframe->frame_tag = NULL_FRAMETAG;

		pxframe->pkt = NULL;		

		pxframe->buf_addr = NULL;
		pxframe->pxmitbuf = NULL;
 
		rtw_list_insert_tail(&(pxframe->list), &(pxmitpriv->free_xmitframe_queue.queue));

		pxframe++;
	}

	pxmitpriv->min_free_xmitframe_cnt = pxmitpriv->free_xmitframe_cnt = NR_XMITFRAME;

//	pxmitpriv->frag_len = MAX_FRAG_THRESHOLD;

#endif


	
	pxmitpriv->pallocated_freebuf = rtw_zvmalloc(NR_XMITBUFF*sizeof(struct xmit_buf)+4);
	if(pxmitpriv->pallocated_freebuf==NULL)
	{
		DBG_871X("%s: pallocated_freebuf failed!\n", __FUNCTION__);
		res = _FAIL;
		goto exit;
	}
	pxmitpriv->xmit_freebuf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitpriv->pallocated_freebuf), 4);

	pxmitbuf = (struct xmit_buf *)pxmitpriv->xmit_freebuf;
	for (i = 0; i < NR_XMITBUFF; i++)
	{
		rtw_init_listhead(&(pxmitbuf->list));

		pxmitbuf->padapter = padapter;

		/* Tx buf allocation may fail sometimes, so sleep and retry. */
		if((res=rtw_os_xmit_resource_alloc(padapter, pxmitbuf,(MAX_XMITBUF_SZ + XMITBUF_ALIGN_SZ), _TRUE)) == _FAIL) {
			rtw_msleep_os(10);
			res = rtw_os_xmit_resource_alloc(padapter, pxmitbuf,(MAX_XMITBUF_SZ + XMITBUF_ALIGN_SZ), _TRUE);
			if (res == _FAIL) {
				goto free_os_resource;
			}
		}
#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
		pxmitbuf->phead = pxmitbuf->pbuf;
		pxmitbuf->pend = pxmitbuf->pbuf + MAX_XMITBUF_SZ;
		pxmitbuf->pkt_len = 0;
		pxmitbuf->pdata = pxmitbuf->ptail = pxmitbuf->phead;
		pxmitbuf->agg_num = 1;//will check this num when dequeue_writeport
#endif

		rtw_list_insert_tail(&(pxmitbuf->list), &(pxmitpriv->free_xmit_queue.queue));
		#ifdef DBG_XMIT_BUF
		pxmitbuf->no=i;
		#endif
		pxmitbuf++;
	}
	pxmitpriv->min_free_xmitbuf_cnt = pxmitpriv->free_xmitbuf_cnt = NR_XMITBUFF;

#ifdef CONFIG_USB_HCI
/*
		pxmitpriv->txirp_cnt=1;
	
		rtw_init_sema(&(pxmitpriv->tx_retevt), 0);
	
*/
#endif

	if((res = rtw_hal_init_xmit_priv(padapter)) == _FAIL)
		goto free_os_resource;
	
free_os_resource:
	if(res == _FAIL){
		pxmitbuf = (struct xmit_buf *)pxmitpriv->xmit_freebuf;
		for(j=1;j<i;j++)
		{
			rtw_os_xmit_resource_free(padapter, pxmitbuf,(MAX_XMITBUF_SZ + XMITBUF_ALIGN_SZ), _TRUE);			
			pxmitbuf++;
		}
	}		
	if((res == _FAIL)&&(pxmitpriv->pallocated_freebuf))
		rtw_vmfree(pxmitpriv->pallocated_freebuf, NR_XMITBUFF*sizeof(struct xmit_buf)+4);

exit:

_func_exit_;	

	return res;
}
void rtw_free_xmit_priv(PADAPTER padapter)
{

	int i = 0;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)pxmitpriv->xmit_freebuf;
#if defined(CONFIG_TX_AGGREGATION)||defined(CONFIG_USB_TX_AGGREGATION)
	struct xmit_frame	*pxmitframe = (struct xmit_frame*) pxmitpriv->pxmit_frame_buf;
#endif
 _func_enter_;   

	rtw_hal_free_xmit_priv(padapter);

	rtw_spinlock_free(&pxmitpriv->lock);
	rtw_free_sema(&pxmitpriv->xmit_sema);
	rtw_free_sema(&pxmitpriv->XmitTerminateSema);

	rtw_spinlock_free(&pxmitpriv->free_xmit_queue.lock);
	rtw_spinlock_free(&pxmitpriv->xmitbuf_pending_queue.lock);
	
#if defined(CONFIG_TX_AGGREGATION)||defined(CONFIG_USB_TX_AGGREGATION)
	rtw_spinlock_free(&pxmitpriv->free_xmitframe_queue.lock);
	rtw_spinlock_free(&pxmitpriv->xmitframe_pending_queue.lock);
 	if(pxmitpriv->pxmit_frame_buf==NULL)
		return;
	
	for(i=0; i<NR_XMITFRAME; i++)
	{	
		rtw_os_xmit_complete(padapter, pxmitframe);

		pxmitframe++;
	}
#endif
	for(i=0; i<NR_XMITBUFF; i++)
	{
#if (!defined CONFIG_TX_AGGREGATION) && (!defined CONFIG_USB_TX_AGGREGATION)
		rtw_os_xmit_complete(padapter, pxmitbuf);
#endif
		rtw_os_xmit_resource_free(padapter, pxmitbuf,(MAX_XMITBUF_SZ + XMITBUF_ALIGN_SZ), _TRUE);
		
		pxmitbuf++;
	}
	
#if defined(CONFIG_TX_AGGREGATION)||defined(CONFIG_USB_TX_AGGREGATION)
	if(pxmitpriv->pallocated_frame_buf) {
		rtw_vmfree(pxmitpriv->pallocated_frame_buf, NR_XMITFRAME * sizeof(struct xmit_frame) + 4);
	}
	pxmitpriv->min_free_xmitframe_cnt = pxmitpriv->free_xmitframe_cnt = 0;
#endif

	if(pxmitpriv->pallocated_freebuf)
		rtw_vmfree(pxmitpriv->pallocated_freebuf, NR_XMITBUFF*sizeof(struct xmit_buf)+4);
	pxmitpriv->min_free_xmitbuf_cnt = pxmitpriv->free_xmitbuf_cnt = 0;
	
_func_exit_;
}

#if defined(CONFIG_TX_AGGREGATION)||defined(CONFIG_USB_TX_AGGREGATION)
void rtw_count_tx_stats(PADAPTER padapter, struct xmit_frame *pxmitframe, int sz)
{

	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;

	u8	pkt_num = 1;

	if ((pxmitframe->frame_tag&0x0f) == DATA_FRAMETAG)
	{
#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
		pkt_num = pxmitframe->agg_num;
#endif

#ifdef CONFIG_POWER_SAVING
	padapter->LinkDetectInfo.NumTxOkInPeriod += pkt_num;
#endif

		pxmitpriv->tx_pkts += pkt_num;

		pxmitpriv->tx_bytes += sz;		
	}
}

void rtw_init_xmitframe(struct xmit_frame *pxframe)
{
	if (pxframe !=  NULL)//default value setting
	{
		pxframe->buf_addr = NULL;
		pxframe->pxmitbuf = NULL;
		pxframe->frame_tag = DATA_FRAMETAG;
#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
//		pxframe->pg_num = 1;
		pxframe->agg_num = 1;
#endif

#ifdef CONFIG_XMIT_ACK
		pxframe->ack_report = 0;
#endif

	}
}

/*
Calling context:
1. OS_TXENTRY
2. RXENTRY (rx_thread or RX_ISR/RX_CallBack)

If we turn on USE_RXTHREAD, then, no need for critical section.
Otherwise, we must use _enter/_exit critical to protect free_xmit_queue...

Must be very very cautious...

*/
struct xmit_frame *rtw_alloc_xmitframe(struct xmit_priv *pxmitpriv)//(_queue *pfree_xmit_queue)
{
	/*
		Please remember to use all the osdep_service api,
		and lock/unlock or _enter/_exit critical to protect 
		pfree_xmit_queue
	*/

	_irqL irqL;
	struct xmit_frame *pxframe = NULL;
	_list *plist, *phead;
	_queue *pfree_xmit_queue = &pxmitpriv->free_xmitframe_queue;

_func_enter_;

	rtw_enter_critical_bh(&pfree_xmit_queue->lock, &irqL);

	if (rtw_queue_empty(pfree_xmit_queue) == _TRUE) {
		RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("rtw_alloc_xmitframe:%d\n", pxmitpriv->free_xmitframe_cnt));
		pxframe =  NULL;
	} else {
		phead = get_list_head(pfree_xmit_queue);

		plist = get_next(phead);

		pxframe = LIST_CONTAINOR(plist, struct xmit_frame, list);

		rtw_list_delete(&(pxframe->list));
		pxmitpriv->free_xmitframe_cnt--;
		pxmitpriv->min_free_xmitframe_cnt = MIN(pxmitpriv->min_free_xmitframe_cnt,pxmitpriv->free_xmitframe_cnt);

		RT_TRACE(_module_rtl871x_xmit_c_, _drv_info_, ("rtw_alloc_xmitframe():free_xmitframe_cnt=%d\n", pxmitpriv->free_xmitframe_cnt));
	}

	rtw_exit_critical_bh(&pfree_xmit_queue->lock, &irqL);

	rtw_init_xmitframe(pxframe);

_func_exit_;

	return pxframe;
}

struct xmit_frame *rtw_alloc_xmitframe_once(struct xmit_priv *pxmitpriv)
{
	struct xmit_frame *pxframe = NULL;
	u8 *alloc_addr;

	alloc_addr = rtw_zmalloc(sizeof(struct xmit_frame) + 4);
	
	if (alloc_addr == NULL)
		goto exit;
		
	pxframe = (struct xmit_frame *)N_BYTE_ALIGMENT((SIZE_PTR)(alloc_addr), 4);
	pxframe->alloc_addr = alloc_addr;

	pxframe->padapter = pxmitpriv->padapter;
	//pxframe->frame_tag = NULL_FRAMETAG;

	pxframe->pkt = NULL;

	pxframe->buf_addr = NULL;
	pxframe->pxmitbuf = NULL;

	rtw_init_xmitframe(pxframe);

	DBG_871X("################## %s ##################\n", __func__);

exit:
	return pxframe;
}

s32 rtw_free_xmitframe(struct xmit_priv *pxmitpriv, struct xmit_frame *pxmitframe)
{	
	_irqL irqL;
	_queue *queue = NULL;
	_adapter *padapter = pxmitpriv->padapter;
	_pkt *pndis_pkt = NULL;

_func_enter_;	

	if (pxmitframe == NULL) {
		RT_TRACE(_module_rtl871x_xmit_c_, _drv_err_, ("======rtw_free_xmitframe():pxmitframe==NULL!!!!!!!!!!\n"));
		goto exit;
	}

	if (pxmitframe->pkt){
		pndis_pkt = pxmitframe->pkt;
		pxmitframe->pkt = NULL;
	}

	if (pxmitframe->alloc_addr) {
		DBG_871X("################## %s with alloc_addr ##################\n", __func__);
		rtw_mfree(pxmitframe->alloc_addr, sizeof(struct xmit_frame) + 4);
		goto check_pkt_complete;
	}

	queue = &pxmitpriv->free_xmitframe_queue;

	rtw_enter_critical_bh(&queue->lock, &irqL);

	rtw_list_delete(&pxmitframe->list);	
	rtw_list_insert_tail(&pxmitframe->list, get_list_head(queue));

	pxmitpriv->free_xmitframe_cnt++;
	RT_TRACE(_module_rtl871x_xmit_c_, _drv_debug_, ("rtw_free_xmitframe():free_xmitframe_cnt=%d\n", pxmitpriv->free_xmitframe_cnt));
	rtw_exit_critical_bh(&queue->lock, &irqL);

check_pkt_complete:

	if(pndis_pkt)
		rtw_os_pkt_complete(padapter, pndis_pkt);

exit:

_func_exit_;

	return _SUCCESS;
}

void rtw_free_xmitframe_queue(struct xmit_priv *pxmitpriv, _queue *pframequeue)
{
	_irqL irqL;
	_list	*plist, *phead;
	struct	xmit_frame 	*pxmitframe;

_func_enter_;	

	rtw_enter_critical(&(pframequeue->lock), &irqL);

	phead = get_list_head(pframequeue);
	plist = get_next(phead);
	
	while (rtw_end_of_queue_search(phead, plist) == _FALSE)
	{
			
		pxmitframe = LIST_CONTAINOR(plist, struct xmit_frame, list);

		plist = get_next(plist); 
		
		rtw_free_xmitframe(pxmitpriv,pxmitframe);
			
	}
	rtw_exit_critical(&(pframequeue->lock), &irqL);

_func_exit_;
}

s32 rtw_xmitframe_enqueue(_adapter *padapter, struct xmit_frame *pxmitframe)
{
	struct xmit_priv 	*pxmitpriv = &padapter->xmitpriv;
	_queue *pqueue = &pxmitpriv->xmitframe_pending_queue;
	_irqL irqL;
_func_enter_;
	//enqueue xmitbuf
rtw_enter_critical_bh(&(pqueue->lock), &irqL);
	rtw_list_insert_tail(&pxmitframe->list, get_list_head(pqueue));
rtw_exit_critical_bh(&(pqueue->lock), &irqL);
	return _SUCCESS;
}

s32 check_pending_xmitframe(PADAPTER padapter)
{
	_irqL irql;
	_queue *pqueue;
	s32	ret = _FALSE;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
_func_enter_;
	pqueue = &pxmitpriv->xmitframe_pending_queue;

	rtw_enter_critical_bh(&pqueue->lock, &irql);

	if(rtw_queue_empty(pqueue) == _FALSE)
		ret = _TRUE;

	rtw_exit_critical_bh(&pqueue->lock, &irql);
_func_exit_;
	return ret;
}

struct xmit_frame* rtw_dequeue_pending_xframe(struct xmit_priv *pxmitpriv)
{
	_irqL irqL;
	_list *plist, *phead;
	struct xmit_frame *pxmitframe = NULL;
	_queue *pframe_queue = NULL;
	pframe_queue = &pxmitpriv->xmitframe_pending_queue;
_func_enter_;
rtw_enter_critical_bh(&pframe_queue->lock, &irqL);
	phead = get_list_head(pframe_queue);
	plist = get_next(phead);
	if(plist != phead)
	{
		pxmitframe = LIST_CONTAINOR(plist, struct xmit_frame, list);
		rtw_list_delete(&pxmitframe->list);
	}
rtw_exit_critical_bh(&pframe_queue->lock, &irqL);
_func_exit_;

	return pxmitframe;
}

/*
 * The main transmit(tx) entry
 *
 * Return
 *	1	enqueue
 *	0	success, hardware will handle this xmit frame(packet)
 *	<0	fail
 */
s32 rtw_xmit(_adapter *padapter, _pkt *ppkt)
{
	static u32 start = 0;
	static u32 drop_cnt = 0;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct xmit_frame *pxmitframe = NULL;

//	s32 res;

	if (start == 0)
		start = rtw_get_current_time();

	pxmitframe = rtw_alloc_xmitframe(pxmitpriv);

	if (rtw_get_passing_time_ms(start) > 2000) {
		if (drop_cnt)
			DBG_871X("DBG_TX_DROP_FRAME %s no more pxmitframe, drop_cnt:%u\n", __FUNCTION__, drop_cnt);
		start = rtw_get_current_time();
		drop_cnt = 0;
	}

	if (pxmitframe == NULL) {
		drop_cnt ++;
		RT_TRACE(_module_xmit_osdep_c_, _drv_err_, ("rtw_xmit: no more pxmitframe\n"));
		return -1;
	}

	pxmitframe->pkt = ppkt;
	pxmitframe->pkt_len = ppkt->len;
	pxmitframe->frame_tag = DATA_FRAMETAG;

	if (rtw_hal_xmit(padapter, pxmitframe) == _TRUE)
		return 1;

	return 0;
}
#endif

struct xmit_buf *rtw_alloc_xmitbuf(PADAPTER padapter)//(_queue *pfree_xmit_queue)
{
	/*
		Please remember to use all the osdep_service api,
		and lock/unlock or _enter/_exit critical to protect 
		pfree_xmit_queue
	*/

	_irqL irqL;
	struct xmit_buf *pxmitbuf = NULL;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	_list *plist, *phead;
	_queue *pfree_xmit_queue = &pxmitpriv->free_xmit_queue;

_func_enter_;

	rtw_enter_critical(&pfree_xmit_queue->lock, &irqL);

	if (rtw_queue_empty(pfree_xmit_queue) == _TRUE) {
		//DBG_871X("rtw_alloc_xmitbuf failed!\n");
		pxmitbuf=  NULL;
	} else {
		phead = get_list_head(pfree_xmit_queue);

		plist = get_next(phead);

		pxmitbuf = LIST_CONTAINOR(plist, struct xmit_buf, list);

		rtw_list_delete(&(pxmitbuf->list));
	}
	if(pxmitbuf){
		pxmitpriv->free_xmitbuf_cnt--;
		pxmitpriv->min_free_xmitbuf_cnt = MIN(pxmitpriv->min_free_xmitbuf_cnt,pxmitpriv->free_xmitbuf_cnt);
		#ifdef DBG_XMIT_BUF
		DBG_871X("DBG_XMIT_BUF ALLOC no=%d,  free_xmitbuf_cnt=%d\n",pxmitbuf->no, pxmitpriv->free_xmitbuf_cnt);
		#endif
#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
		pxmitbuf->pkt_len = 0;
		pxmitbuf->pdata = pxmitbuf->ptail = pxmitbuf->phead;
#endif
#ifdef CONFIG_USB_HCI
		pxmitbuf->pkt_len = 0;
#endif
	}

	rtw_exit_critical(&pfree_xmit_queue->lock, &irqL);

_func_exit_;

	return pxmitbuf;
}
s32 rtw_free_xmitbuf(PADAPTER padapter, struct xmit_buf *pxmitbuf)
{	
	_irqL irqL;
	_queue *queue = NULL;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
_func_enter_;	
	queue = &pxmitpriv->free_xmit_queue;
	if (pxmitbuf == NULL) {
		DBG_871X("======rtw_free_xmitbuf():pxmitbuf==NULL!!!!!!!!!!\n");
		goto exit;
	}

	rtw_enter_critical(&queue->lock, &irqL);

	rtw_list_delete(&pxmitbuf->list);	
	rtw_list_insert_tail(&pxmitbuf->list, get_list_head(queue));
	pxmitpriv->free_xmitbuf_cnt++;
	#ifdef DBG_XMIT_BUF
	DBG_871X("DBG_XMIT_BUF FREE no=%d, free_xmitbuf_cnt=%d\n",pxmitbuf->no ,pxmitpriv->free_xmitbuf_cnt);
	#endif
	rtw_exit_critical(&queue->lock, &irqL);

exit:

_func_exit_;

	return _SUCCESS;
}

s32 check_pending_xmitbuf(PADAPTER padapter)
{
	_irqL irql;
	_queue *pqueue;
	s32	ret = _FALSE;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
_func_enter_;
	pqueue = &pxmitpriv->xmitbuf_pending_queue;

	rtw_enter_critical(&pqueue->lock, &irql);

	if(rtw_queue_empty(pqueue) == _FALSE)
		ret = _TRUE;

	rtw_exit_critical(&pqueue->lock, &irql);
_func_exit_;
	return ret;
}

void enqueue_pending_xmitbuf(
	struct xmit_priv *pxmitpriv,
	struct xmit_buf *pxmitbuf)
{
	_irqL irql;
	_queue *pqueue;
	_adapter *pri_adapter = pxmitpriv->padapter;

	pqueue = &pxmitpriv->xmitbuf_pending_queue;

	rtw_enter_critical(&pqueue->lock, &irql);
	rtw_list_delete(&pxmitbuf->list);
	rtw_list_insert_tail(&pxmitbuf->list, get_list_head(pqueue));
	rtw_exit_critical(&pqueue->lock, &irql);

	rtw_up_sema(&(pri_adapter->xmitpriv.xmit_sema));
}


void enqueue_pending_xmitbuf_to_head(
	struct xmit_priv *pxmitpriv,
	struct xmit_buf *pxmitbuf)
{
	_irqL irql;
	_queue *pqueue;
//	_adapter *pri_adapter = pxmitpriv->padapter;

	pqueue = &pxmitpriv->xmitbuf_pending_queue;

	rtw_enter_critical(&pqueue->lock, &irql);
	rtw_list_delete(&pxmitbuf->list);
	rtw_list_insert_head(&pxmitbuf->list, get_list_head(pqueue));
	rtw_exit_critical(&pqueue->lock, &irql);
}

void rtw_free_xmitbuf_queue(PADAPTER padapter, _queue *pframequeue)
{
	_irqL irqL;
	_list	*plist, *phead;
	struct	xmit_buf 	*pxmitbuf;

_func_enter_;	

	rtw_enter_critical(&(pframequeue->lock), &irqL);

	phead = get_list_head(pframequeue);
	plist = get_next(phead);
	
	while (rtw_end_of_queue_search(phead, plist) == _FALSE)
	{
			
		pxmitbuf = LIST_CONTAINOR(plist, struct xmit_buf, list);

		plist = get_next(plist); 
		
		rtw_free_xmitbuf(padapter,pxmitbuf);
			
	}
	rtw_exit_critical(&(pframequeue->lock), &irqL);

_func_exit_;
}

struct xmit_buf* rtw_dequeue_xmitbuf(PADAPTER padapter)
{
	_irqL irqL;
	_list *plist, *phead;
	struct xmit_buf *pxmitbuf = NULL;
	_queue *pframe_queue = NULL;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	pframe_queue = &pxmitpriv->xmitbuf_pending_queue;
_func_enter_;
rtw_enter_critical(&pframe_queue->lock, &irqL);
	phead = get_list_head(pframe_queue);
	plist = get_next(phead);
	if(plist != phead)
	{
		pxmitbuf = LIST_CONTAINOR(plist, struct xmit_buf, list);
		rtw_list_delete(&pxmitbuf->list);
	}
rtw_exit_critical(&pframe_queue->lock, &irqL);
_func_exit_;
	return pxmitbuf;
}
thread_return rtw_xmit_thread(thread_context context)
{
	s32 err;
	PADAPTER padapter;
	struct xmit_priv *pxmitpriv;
	padapter = (PADAPTER)context;
	pxmitpriv = &padapter->xmitpriv;
	
	rtw_thread_enter("RTW_XMIT_THREAD");
	DBG_871X("start %s\n", __FUNCTION__);
	
	do{
#ifdef PLATFORM_LINUX
		//if (kthread_should_stop())
    	if (kthread_should_stop(pxmitpriv->xmitThread))  // Irene Lin
			break;
#endif
		err = rtw_hal_xmit_thread_handler(padapter);
		//rtw_flush_signals_thread();
	}while(_SUCCESS == err);
	
	rtw_up_sema(&(pxmitpriv->XmitTerminateSema));

	RT_TRACE(_module_hal_xmit_c_, _drv_notice_, ("-%s\n", __FUNCTION__));
	DBG_871X("exit %s\n", __FUNCTION__);
	
	rtw_thread_exit();
}
