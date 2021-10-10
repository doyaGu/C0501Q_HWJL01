#include "autoconf.h"
#include "drv_types.h"
#include "8195_desc.h"
#include "8195_sdio_reg.h"
#include "rtw_xmit.h"
#include "sdio_ops.h"
#include "rtl8195a_xmit.h"
s32 rtl8195as_init_xmit_priv(PADAPTER padapter)
{
	s32 res = _SUCCESS;
#ifdef CONFIG_TX_AGGREGATION
	struct xmit_priv 	*pxmitpriv = &padapter->xmitpriv;
	rtw_init_sema(&pxmitpriv->SdioXmitSema, 0);
	rtw_init_sema(&pxmitpriv->SdioXmitTerminateSema, 0);
#endif
	return res;
}

void rtl8195as_free_xmit_priv(PADAPTER padapter)
{
#ifdef CONFIG_TX_AGGREGATION
	struct xmit_priv 	*pxmitpriv = &padapter->xmitpriv;
	rtw_free_sema(&pxmitpriv->SdioXmitSema);
	rtw_free_sema(&pxmitpriv->SdioXmitTerminateSema);
#endif
}

#ifdef CONFIG_TX_AGGREGATION
void rtl8195as_fill_default_txdesc(
	struct xmit_frame *pxmitframe,
	u8 *pbuf)
{
	PTXDESC_8195A ptxdesc = (PTXDESC_8195A)pbuf;
	ptxdesc->bus_agg_num = pxmitframe->agg_num;
	ptxdesc->txpktsize = pxmitframe->pkt_len;
	if(pxmitframe->frame_tag == DATA_FRAMETAG)
		ptxdesc->type = TX_PACKET_802_3;
	else if(pxmitframe->frame_tag == CMD_FRAMETAG)
		ptxdesc->type = TX_H2C_CMD;
	ptxdesc->offset = SIZE_TX_DESC_8195a;
}

s32 rtl8195as_xmitframe_coalesce(_adapter *padapter, _pkt *pkt, struct xmit_frame *pxmitframe)
{
	u8 *pbuf_start, *mem_start;
	u8 hw_hdr_offset;

	if (pxmitframe->buf_addr == NULL){
		DBG_8192C("==> %s buf_addr==NULL \n",__FUNCTION__);
		return _FAIL;
	}
	hw_hdr_offset = SIZE_TX_DESC_8195a;
	pbuf_start = pxmitframe->buf_addr;
	mem_start = pbuf_start + hw_hdr_offset;
	if(pkt)
		rtw_memcpy(mem_start, pkt->data, pkt->len);
	return _SUCCESS;
}

/*
 *	Description:
 *
 *	Parameters:
 *		pxmitframe	xmitframe
 *		pbuf		where to fill tx desc
 */
void rtl8195as_update_txdesc(struct xmit_frame *pxmitframe, u8 *pbuf)
{
	struct tx_desc *pdesc;


	pdesc = (struct tx_desc*)pbuf;
	
#ifndef PLATFORM_ECOS //with memset, cpu usage will be increased when traffic is busy
	rtw_memset(pdesc, 0, sizeof(struct tx_desc));
#endif

	rtl8195as_fill_default_txdesc(pxmitframe, pbuf);

	pdesc->txdw0 = cpu_to_le32(pdesc->txdw0);
	pdesc->txdw1 = cpu_to_le32(pdesc->txdw1);
	pdesc->txdw2 = cpu_to_le32(pdesc->txdw2);
	pdesc->txdw3 = cpu_to_le32(pdesc->txdw3);
	pdesc->txdw4 = cpu_to_le32(pdesc->txdw4);
	pdesc->txdw5 = cpu_to_le32(pdesc->txdw5);

//	rtl8188e_cal_txdesc_chksum(pdesc);
}

static s32 xmit_xmitframes(PADAPTER padapter, struct xmit_priv *pxmitpriv)
{
	u32 err, agg_num=0;
	u8 pkt_index=0;
	_irqL irql;
	_list *frame_plist, *frame_phead;
	struct xmit_frame *pxmitframe;
	_queue *pframe_queue;
	struct xmit_buf *pxmitbuf;
	u32 txlen, max_xmit_len;
	s32 ret;

	err = 0;
	pxmitframe = NULL;
	pframe_queue = NULL;
	pxmitbuf = NULL;

#if 0
		if(check_pending_xmitbuf(padapter) == _TRUE) {
			if (_FALSE == _sdio_query_tx_free_bd(padapter, 5)) {
				err = -2;
				return err;
			}
		}
#endif

		max_xmit_len = MAX_XMITBUF_SZ;

		pframe_queue = &pxmitpriv->xmitframe_pending_queue;
rtw_enter_critical_bh(&pxmitpriv->lock, &irql);
		frame_phead = get_list_head(pframe_queue);

		while (rtw_is_list_empty(frame_phead) == _FALSE)
		{
			frame_plist = get_next(frame_phead);
			pxmitframe = LIST_CONTAINOR(frame_plist, struct xmit_frame, list);				

			// check xmit_buf size enough or not
			txlen = SIZE_TX_DESC_8195a + pxmitframe->pkt_len;

			if ((NULL == pxmitbuf) ||
				((_RND(pxmitbuf->pkt_len, 4) + txlen) > max_xmit_len)
				|| (_FALSE == rtw_sdio_query_tx_free_bd(padapter, agg_num+1))		
			)
			{
				
				if (pxmitbuf) {
					//pxmitbuf->priv_data will be NULL, and will crash here
					if (pxmitbuf->pkt_len > 0)
					{
						struct xmit_frame *pframe;
						pframe = (struct xmit_frame*)pxmitbuf->priv_data;
						pframe->agg_num = agg_num;
						pxmitbuf->agg_num = agg_num;
						rtl8195as_update_txdesc(pframe, pframe->buf_addr);
						rtw_free_xmitframe(pxmitpriv, pframe);
						pxmitbuf->priv_data = NULL;
						enqueue_pending_xmitbuf(pxmitpriv, pxmitbuf);
						//rtw_yield_os();
					} else {
						rtw_free_xmitbuf(padapter, pxmitbuf);
					}
				}

				//when the first xmitframe
				pxmitbuf = rtw_alloc_xmitbuf(padapter);
				if (pxmitbuf == NULL) {
					RT_TRACE(_module_hal_xmit_c_, _drv_err_, ("%s: xmit_buf is not enough!\n", __FUNCTION__));
#ifdef DBG_TP
					DBG_871X("%s: xmit_buf is not enough!\n", __FUNCTION__);
#endif
					err = -2;
#ifdef CONFIG_SDIO_TX_ENABLE_AVAL_INT
					rtw_up_sema(&(pxmitpriv->xmit_sema));
#endif
					break;
				}
				agg_num = 0;
				pkt_index =0;
			}
			
			// ok to send, remove frame from queue
			rtw_list_delete(&pxmitframe->list);
			if (agg_num == 0) {
				pxmitbuf->priv_data = (u8*)pxmitframe;
			}

			// coalesce the xmitframe to xmitbuf
			pxmitframe->pxmitbuf = pxmitbuf;
			pxmitframe->buf_addr = pxmitbuf->ptail;

			ret = rtl8195as_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe);
			if (ret == _FAIL) {
				DBG_871X("%s: coalesce FAIL!", __FUNCTION__);					
				// Todo: error handler
				//rtw_free_xmitframe(pxmitpriv, pxmitframe);
			} else {
				agg_num++;
				if (agg_num != 1)
					rtl8195as_update_txdesc(pxmitframe, pxmitframe->buf_addr);
				rtw_count_tx_stats(padapter, pxmitframe, pxmitframe->pkt_len);
				txlen = SIZE_TX_DESC_8195a + pxmitframe->pkt_len;

				pkt_index++;
				pxmitbuf->ptail += _RND(txlen, 4); // round to 8 bytes alignment
				pxmitbuf->pkt_len += _RND(txlen, 4);
			}
			if (agg_num != 1)
				rtw_free_xmitframe(pxmitpriv, pxmitframe);
			pxmitframe = NULL;	
		}
rtw_exit_critical_bh(&pxmitpriv->lock, &irql);

		// dump xmit_buf to hw tx fifo
		if (pxmitbuf)
		{
			RT_TRACE(_module_hal_xmit_c_, _drv_notice_, ("pxmitbuf->len=%d enqueue\n",pxmitbuf->len));
			//when a aggregated xmitbuf finished
			if (pxmitbuf->pkt_len > 0) {
				struct xmit_frame *pframe;
				pframe = (struct xmit_frame*)pxmitbuf->priv_data;
				pframe->agg_num = agg_num;
				pxmitbuf->agg_num = agg_num;
				rtl8195as_update_txdesc(pframe, pframe->buf_addr);
				rtw_free_xmitframe(pxmitpriv, pframe);
				pxmitbuf->priv_data = NULL;
				enqueue_pending_xmitbuf(pxmitpriv, pxmitbuf);
				rtw_yield_os();
			}
			else
				rtw_free_xmitbuf(padapter, pxmitbuf);

			pxmitbuf = NULL;
		}

	return err;
	
}

/*
 * Description
 *	Transmit xmitframe from queue
 *
 * Return
 *	_SUCCESS	ok
 *	_FAIL		something error
 */
s32 rtl8195as_xmit_handler(PADAPTER padapter)
{
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv ;
	s32 ret;
	_irqL irql;

	ret = rtw_down_sema(&pxmitpriv->SdioXmitSema);
	if (_FAIL == ret) {
		RT_TRACE(_module_hal_xmit_c_, _drv_emerg_, ("%s: down sema fail!\n", __FUNCTION__));
		return _FAIL;
	}

next:
	if ((padapter->bSurpriseRemoved == _TRUE) || 
		(padapter->bDriverStopped == _TRUE) 
	) {
		RT_TRACE(_module_hal_xmit_c_, _drv_notice_,
				 ("%s: bDriverStopped(%d) bSurpriseRemoved(%d)\n",
				  __FUNCTION__, padapter->bDriverStopped, padapter->bSurpriseRemoved));
		return _FAIL;
	}
rtw_enter_critical_bh(&pxmitpriv->lock, &irql);
	ret = check_pending_xmitframe(padapter);
rtw_exit_critical_bh(&pxmitpriv->lock, &irql);
	if (ret == _FALSE) {
		return _SUCCESS;
	}
	
	// dequeue frame and write to hardware
	ret = xmit_xmitframes(padapter, pxmitpriv);
	if (ret == -2) {
		rtw_yield_os();
		goto next;
	}
rtw_enter_critical_bh(&pxmitpriv->lock, &irql);
	ret = check_pending_xmitframe(padapter);
rtw_exit_critical_bh(&pxmitpriv->lock, &irql);
	if (ret == _TRUE) {
		goto next;
	}

	return _SUCCESS;
}

thread_return rtl8195as_xmit_thread(thread_context context)
{
	s32 ret;
	PADAPTER padapter= (PADAPTER)context;	
	struct xmit_priv *pxmitpriv= &padapter->xmitpriv;
	
	ret = _SUCCESS;

	rtw_thread_enter("RTWHALXT");

	DBG_871X("start %s\n", __FUNCTION__);

	do {
#ifdef PLATFORM_LINUX
		//if (kthread_should_stop())
		if (kthread_should_stop(pxmitpriv->SdioXmitThread)) // Irene Lin
			break;
#endif		
		ret = rtl8195as_xmit_handler(padapter);
		//rtw_flush_signals_thread();
	} while ((_SUCCESS == ret)/*&&(pxmitpriv->SdioXmitThread)*/);

	rtw_up_sema(&pxmitpriv->SdioXmitTerminateSema);

	RT_TRACE(_module_hal_xmit_c_, _drv_notice_, ("-%s\n", __FUNCTION__));
	DBG_871X("exit %s\n", __FUNCTION__);

	rtw_thread_exit();
}
#endif

#ifdef PLATFORM_ECOS
extern void rtw_triggered_wlan_tx_tasklet(_adapter *priv);
#endif

s32 rtl8195as_dequeue_writeport(PADAPTER padapter)
{
	struct xmit_buf *pxmitbuf;
	PTXDESC_8195A ptxdesc;
	PSDIO_DATA psdio;
	
#ifdef CONFIG_SDIO_TX_OVF_CTRL	
#ifdef CONFIG_SDIO_TX_ENABLE_AVAL_INT
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	u8	bUpdateTxBDNum = _FALSE;
#else
	u32	polling_num = 0;
#endif
#endif //CONFIG_SDIO_TX_OVF_CTRL

_func_enter_;

	pxmitbuf = rtw_dequeue_xmitbuf(padapter);
	if(pxmitbuf == NULL)
		return _TRUE;
#ifdef CONFIG_SDIO_TX_OVF_CTRL	
query_free_page:
	// check if hardware tx fifo page is enough
	if( _FALSE == rtw_sdio_query_tx_free_bd(padapter, pxmitbuf->agg_num))
	{
#ifdef CONFIG_SDIO_TX_ENABLE_AVAL_INT
		if (bUpdateTxBDNum == _FALSE) {
			// Total number of TXBD is NOT available, so update current TXBD status
			HalQueryTxBufferStatus8195ASdio(padapter);
			bUpdateTxBDNum = _TRUE;
			goto query_free_page;
		} else {
			bUpdateTxBDNum = _FALSE;
#ifdef DBG_TP
			DBG_871X("%s: FIFO starvation! len=%d agg=%d\n",
				__func__, pxmitbuf->pkt_len, pxmitbuf->agg_num);
#endif
			enqueue_pending_xmitbuf_to_head(pxmitpriv, pxmitbuf);
			return _TRUE;
		}
#else //CONFIG_SDIO_TX_ENABLE_AVAL_INT
		polling_num++;
		if ((polling_num % 60) == 0) {//or 80
			//DBG_871X("%s: FIFO starvation!(%d) len=%d agg=%d page=(R)%d(A)%d\n",
			//	__func__, n, pxmitbuf->len, pxmitbuf->agg_num, pframe->pg_num, freePage[PageIdx] + freePage[PUBLIC_QUEUE_IDX]);
			rtw_msleep_os(1);
		}

		// Total number of TXBD is NOT available, so update current TXBD status
		HalQueryTxBufferStatus8195ASdio(padapter);
		goto query_free_page;
#endif //CONFIG_SDIO_TX_ENABLE_AVAL_INT
	}
#endif

	if ((padapter->bSurpriseRemoved == _TRUE) 
		|| (padapter->bDriverStopped == _TRUE)
	){
		RT_TRACE(_module_hal_xmit_c_, _drv_notice_,
			 ("%s: bSurpriseRemoved(wirte port)\n", __FUNCTION__));
		goto free_xmitbuf;
	}
	
	ptxdesc = (PTXDESC_8195A)pxmitbuf->pdata;
	psdio= &padapter->dvobj->intf_data;
	
	if((ptxdesc->offset>SIZE_TX_DESC_8195a)||((ptxdesc->offset+ptxdesc->txpktsize)>psdio->SdioTxMaxSZ)||((u32)(pxmitbuf->pdata)%4))
		DBG_871X("%s: PKT SIZE ERROR OFFSET(%d) PKTSIZE(%d)\n", __FUNCTION__, ptxdesc->offset, ptxdesc->txpktsize);

#ifdef CONFIG_POWER_SAVING
#ifdef CONFIG_PS_DYNAMIC_CHK
	rtw_ps_deny(padapter, PS_DENY_DRV_TXDATA);
	rtw_set_ps_mode(padapter, PS_MODE_ACTIVE);
#endif
#endif

#ifdef CONFIG_TX_AGGREGATION
	pxmitpriv->max_agg_num = MAX(pxmitpriv->max_agg_num, pxmitbuf->agg_num);
	pxmitpriv->max_agg_pkt_len= MAX(pxmitpriv->max_agg_pkt_len, pxmitbuf->pkt_len);
#endif
	rtw_write_port(padapter, WLAN_TX_FIFO_DEVICE_ID, pxmitbuf->pkt_len, pxmitbuf->pdata);

#ifdef CONFIG_POWER_SAVING
#ifdef CONFIG_PS_DYNAMIC_CHK
	rtw_ps_deny_cancel(padapter, PS_DENY_DRV_TXDATA);
	rtw_set_ps_mode(padapter, PS_MODE_SLEEP);
#endif
#endif
	
#ifdef CONFIG_SDIO_TX_OVF_CTRL
	rtw_sdio_update_tx_free_bd(padapter, pxmitbuf->agg_num);
#endif

free_xmitbuf:		
	rtw_free_xmitbuf(padapter, pxmitbuf);
#ifdef PLATFORM_ECOS
	rtw_triggered_wlan_tx_tasklet(padapter);
#endif	
_func_exit_;
	return _FAIL;
}

s32 rtl8195as_hal_xmit_handler(PADAPTER padapter)
{
	s32 ret;
	u8 is_queue_pending;
	u8 is_queue_empty ;

	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	ret = rtw_down_sema(&pxmitpriv->xmit_sema);

	if(ret == _FAIL)
		return _FAIL;

	if ((padapter->bSurpriseRemoved == _TRUE) || 
		(padapter->bDriverStopped == _TRUE) 
	) {
		RT_TRACE(_module_hal_xmit_c_, _drv_notice_,
				 ("%s: bDriverStopped(%d) bSurpriseRemoved(%d)\n",
				  __FUNCTION__, padapter->bDriverStopped, padapter->bSurpriseRemoved));
		return _FAIL;
	}

	is_queue_pending = check_pending_xmitbuf(padapter);

	if(is_queue_pending == _FALSE)
		return _SUCCESS;

	do{
		is_queue_empty=rtl8195as_dequeue_writeport(padapter);
	}while(!is_queue_empty);

	return _SUCCESS;
}


#ifdef CONFIG_TX_AGGREGATION
/*
 * Description:
 *	Handle xmitframe(packet) come from rtw_xmit()
 *
 * Return:
 *	_TRUE	enqueue
 */
s32 rtl8195as_hal_xmit(PADAPTER padapter, struct xmit_frame *pxmitframe)
{
	struct xmit_priv 	*pxmitpriv = &padapter->xmitpriv;
	_irqL irql;

rtw_enter_critical_bh(&pxmitpriv->lock, &irql);
	rtw_xmitframe_enqueue(padapter, pxmitframe);
rtw_exit_critical_bh(&pxmitpriv->lock, &irql);

	rtw_up_sema(&pxmitpriv->SdioXmitSema);

	return _TRUE;
}

#else
s32 rtl8195as_hal_xmit(PADAPTER padapter, struct xmit_buf *pxmitbuf)
{
	struct xmit_priv 	*pxmitpriv = &padapter->xmitpriv;
	_irqL irqL;
_func_enter_;
	//enqueue xmitbuf
rtw_enter_critical_bh(&pxmitpriv->xmitbuf_pending_queue.lock, &irqL);
	rtw_list_insert_tail(&pxmitbuf->list, get_list_head(&pxmitpriv->xmitbuf_pending_queue));
rtw_exit_critical_bh(&pxmitpriv->xmitbuf_pending_queue.lock, &irqL);
_func_exit_;
	rtw_up_sema(&pxmitpriv->xmit_sema);
	return _TRUE;
}
#endif

s32 rtl8195as_hal_mgnt_xmit(PADAPTER padapter, struct xmit_buf *pxmitbuf)
{
//	rtw_write_port(padapter, WLAN_TX_FIFO_DEVICE_ID, pxmitbuf->pkt_len, pxmitbuf->pdata);
//	rtw_free_xmitbuf(padapter, pxmitbuf);
//	return _TRUE;
	struct xmit_priv 	*pxmitpriv = &padapter->xmitpriv;
	_irqL irqL;
_func_enter_;
	//enqueue xmitbuf
rtw_enter_critical_bh(&pxmitpriv->xmitbuf_pending_queue.lock, &irqL);
	rtw_list_insert_tail(&pxmitbuf->list, get_list_head(&pxmitpriv->xmitbuf_pending_queue));
rtw_exit_critical_bh(&pxmitpriv->xmitbuf_pending_queue.lock, &irqL);
_func_exit_;
	rtw_up_sema(&pxmitpriv->xmit_sema);
	return _TRUE;
}

void rtl8195as_xmit_tasklet(void* data){
#ifdef CONFIG_TX_AGGREGATION
	PADAPTER padapter = (PADAPTER)data;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	_irqL irql;
	s32 ret;
	rtw_enter_critical_bh(&pxmitpriv->lock, &irql);
	ret = check_pending_xmitframe(padapter);
	rtw_exit_critical_bh(&pxmitpriv->lock, &irql);
	if(ret == _TRUE)
		rtw_up_sema(&pxmitpriv->xmit_sema);
#endif
}
