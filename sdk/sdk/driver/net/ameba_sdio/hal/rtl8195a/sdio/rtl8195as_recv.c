#define _RTL8195AS_RECV_C_
#include "autoconf.h"
#include "drv_types.h"
#include "8195_desc.h"
#include "8195_sdio_reg.h"
#include "recv_osdep.h"
#include "rtl8195a_recv.h"
#include "rtl8195a_cmd.h"

#ifdef PLATFORM_ECOS
extern void rtw_triggered_wlan_rx_tasklet(_adapter *priv);
#endif


#ifdef CONFIG_SDIO_RX_COPY
static void update_recvframe_attrib_95a(
	union recv_frame *precvframe,
	struct recv_stat *prxstat)
{
	struct rx_pkt_attrib	*pattrib;
	struct recv_stat	report;
	PRXDESC_8195A prxreport;
	u8 offset, shift_sz;

	report.rxdw0 = le32_to_cpu(prxstat->rxdw0);
	report.rxdw1 = le32_to_cpu(prxstat->rxdw1);
	report.rxdw2 = le32_to_cpu(prxstat->rxdw2);
	report.rxdw3 = le32_to_cpu(prxstat->rxdw3);
	report.rxdw4 = le32_to_cpu(prxstat->rxdw4);
	report.rxdw5 = le32_to_cpu(prxstat->rxdw5);

	prxreport = (PRXDESC_8195A)&report;

	pattrib = &precvframe->u.hdr.attrib;

#ifndef PLATFORM_ECOS //with memset, cpu usage will be increased when traffic is busy
	rtw_memset(pattrib, 0, sizeof(struct rx_pkt_attrib));
#endif

	pattrib->crc_err = (u8)((report.rxdw0 >> 31) & 0x1);;//(u8)prxreport->crc;	
	pattrib->icv_err = (u8)((report.rxdw0 >> 30) & 0x1);;//(u8)prxreport->icv;

	pattrib->pkt_rpt_type = (u8)((report.rxdw1) & 0xff);//prxreport->type;

	offset = (u8)((report.rxdw0>>16)&0xff);
	shift_sz = (offset>SIZE_RX_DESC_8195a)?(offset - SIZE_RX_DESC_8195a):0;//prxreport->offset, due to data alignment of FW
	pattrib->shift_sz = shift_sz;

	pattrib->pkt_len = (u16)(report.rxdw0 &0x0000ffff) - shift_sz;//(u16)prxreport->pktlen, due to FW data alignment
	
}

//#ifndef PLATFORM_ECOS
static
//#endif
void rtl8195as_recv_tasklet(void* priv)
{
	PADAPTER			padapter;
	struct recv_priv		*precvpriv;
	struct recv_buf		*precvbuf;
	union recv_frame		*precvframe;
	//struct recv_frame_hdr	*phdr;
	struct rx_pkt_attrib	*pattrib;
	//_irqL	irql;
	u8		*ptr;

	u32 debug_agg_num = 0;
#ifdef DBG_RX_AGG
	u32 debug_packet_offset = 0;
#endif
	u32		pkt_offset, skb_len, alloc_sz;
	s32		transfer_len;
	_pkt		*pkt_copy = NULL;
	u8		rx_report_sz = 0;

	padapter = (PADAPTER)priv;
	precvpriv = &padapter->recvpriv;
	
	do {
		if ((padapter->bDriverStopped == _TRUE)||(padapter->bSurpriseRemoved== _TRUE))
		{
			DBG_871X("recv_tasklet => bDriverStopped or bSurpriseRemoved \n");
			break;
		}
		
		precvbuf = rtw_dequeue_recvbuf(&precvpriv->recv_buf_pending_queue);
		if (NULL == precvbuf) break;

		transfer_len = (s32)precvbuf->len;
		ptr = precvbuf->pdata;

#ifdef DBG_RX_AGG
		DBG_871X("%s: transfer_len = %d\n", __FUNCTION__, transfer_len);
		//DumpForOneBytes(ptr, precvbuf->len);
#endif
		debug_agg_num = 0;
		do {
			precvframe = rtw_alloc_recvframe(&precvpriv->free_recv_queue);
			if (precvframe == NULL) {
				RT_TRACE(_module_rtl871x_recv_c_, _drv_err_, ("%s: no enough recv frame!\n",__FUNCTION__));
				DBG_871X("%s: no enough recv frame!\n",__FUNCTION__);
				rtw_enqueue_recvbuf_to_head(precvbuf, &precvpriv->recv_buf_pending_queue);

				// The case of can't allocte recvframe should be temporary,
				// schedule again and hope recvframe is available next time.
#ifdef PLATFORM_LINUX
				tasklet_schedule(&precvpriv->recv_tasklet);
#endif
#ifdef PLATFORM_ECOS
				// Considering TP, do every one without checking if recv_buf_pending_queue.qlen == 1
				rtw_triggered_wlan_rx_tasklet(padapter);
#endif
				return;
			}
			
			//rx desc parsing
			update_recvframe_attrib_95a(precvframe, (struct recv_stat*)ptr);

			pattrib = &precvframe->u.hdr.attrib;

			rx_report_sz = SIZE_RX_DESC_8195a;
			pkt_offset = rx_report_sz + pattrib->shift_sz + pattrib->pkt_len;

			if ((pattrib->pkt_len==0) || (pkt_offset>((u32)transfer_len))) 
			{
				DBG_871X("%s()-%d: RX Warning!,pkt_len==0 or pkt_offset(%d)> transfoer_len(%d) \n", __FUNCTION__, __LINE__, pkt_offset, transfer_len);
				DBG_871X("%s()-%d: RX Warning!,rx_report_sz(%d), pattrib->shift_sz(%d), pattrib->pkt_len(%d) \n", __FUNCTION__, __LINE__, rx_report_sz, pattrib->shift_sz, pattrib->pkt_len);
				rtw_free_recvframe(precvframe, &precvpriv->free_recv_queue);
				break;
			}

			if ((pattrib->crc_err) || (pattrib->icv_err))
			{			
				DBG_871X("%s: crc_err=%d icv_err=%d, skip!\n", __FUNCTION__, pattrib->crc_err, pattrib->icv_err);
				rtw_free_recvframe(precvframe, &precvpriv->free_recv_queue);
			}
			else
			{
				skb_len = pattrib->pkt_len;
				alloc_sz = skb_len + 8; // 8 for skb->data 4 bytes alignment
				pkt_copy = rtw_skb_alloc(alloc_sz);

				if(pkt_copy)
				{
					pkt_copy->dev = padapter->pnetdev;
					precvframe->u.hdr.pkt = pkt_copy;
					skb_reserve( pkt_copy, 8 - ((SIZE_PTR)( pkt_copy->data ) & 7 ));//force pkt_copy->data at 8-byte alignment address
					rtw_memcpy(pkt_copy->data, (ptr + rx_report_sz + pattrib->shift_sz), skb_len);
					precvframe->u.hdr.rx_head = pkt_copy->head;
					precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail = pkt_copy->data;
					precvframe->u.hdr.rx_end = skb_end_pointer(pkt_copy);
				}
				else
				{
					if(precvbuf->pskb)
						precvframe->u.hdr.pkt = rtw_skb_clone(precvbuf->pskb);

					if(precvframe->u.hdr.pkt)
					{
						_pkt	*pkt_clone = precvframe->u.hdr.pkt;

						pkt_clone->data = ptr + rx_report_sz + pattrib->shift_sz;
						skb_reset_tail_pointer(pkt_clone);
						precvframe->u.hdr.rx_head = precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail 
							= pkt_clone->data;
						precvframe->u.hdr.rx_end =  pkt_clone->data + skb_len;
					}
					else
					{
						DBG_871X("rtl8195as_recv_tasklet: rtw_skb_clone fail\n");
						rtw_free_recvframe(precvframe, &precvpriv->free_recv_queue);
						break;
					}
				}

				recvframe_put(precvframe, skb_len);

				if(pattrib->pkt_rpt_type == RX_PACKET_802_3)
				{
					if (rtw_recv_entry(precvframe) != _SUCCESS)
					{
						RT_TRACE(_module_rtl871x_recv_c_, _drv_err_, ("%s: rtw_recv_entry(precvframe) != _SUCCESS\n",__FUNCTION__));
					}

				}
				else if(pattrib->pkt_rpt_type == RX_C2H_CMD)
				{
					rtl8195a_c2h_cmd_handler(padapter, ptr + rx_report_sz + pattrib->shift_sz, pattrib->pkt_len);
					rtw_free_recvframe(precvframe, &precvpriv->free_recv_queue);					

				}
				else{
					DBG_871X("%s: packet type [0x%x] unknown! drop!\n", __FUNCTION__, pattrib->pkt_rpt_type);
					rtw_free_recvframe(precvframe, &precvpriv->free_recv_queue);
				}

			}

			pkt_offset = _RND4(pkt_offset);
			transfer_len -= pkt_offset;
			ptr += pkt_offset;	
			precvframe = NULL;
			pkt_copy = NULL;

			debug_agg_num++;
#ifdef DBG_RX_AGG
			debug_packet_offset += pkt_offset;
			DBG_871X("%s: rx packet [%d] received! pkt_len(%d) pkt_offset(%d)\n", __FUNCTION__, debug_agg_num, pattrib->pkt_len, debug_packet_offset);
#endif
		}while(transfer_len>0);
		precvpriv->max_agg_num = MAX(precvpriv->max_agg_num,debug_agg_num);
		precvpriv->max_agg_pkt_len = MAX(precvpriv->max_agg_pkt_len, precvbuf->len);
#ifdef DBG_RX_AGG
		//debug rx agg
		DBG_871X("%s: agg_num : %d\n", __FUNCTION__, debug_agg_num);
		if(debug_packet_offset != precvbuf->len)
			DBG_871X("%s: debug_packet_offset(%d) != precvbuf->len(%d) !!\n", __FUNCTION__, debug_packet_offset, precvbuf->len);
#endif		
		precvbuf->len = 0;

		rtw_enqueue_recvbuf(precvbuf, &precvpriv->free_recv_buf_queue);
	} while (1);

}
#else
//#ifndef PLATFORM_ECOS
static
//#endif
void rtl8195as_recv_tasklet(void* priv)
{
	PADAPTER padapter;
	struct recv_buf *precvbuf;
	struct recv_priv *precvpriv;
	padapter = (PADAPTER)priv;
	precvpriv = &padapter->recvpriv;
	do {
		precvbuf = rtw_dequeue_recvbuf(&precvpriv->recv_buf_pending_queue);
		if (NULL == precvbuf) break;

		if (rtw_recv_entry(padapter, precvbuf) != _SUCCESS)
		{
			RT_TRACE(_module_rtl871x_recv_c_, _drv_err_, ("%s: rtw_recv_entry(padapter, precvbuf) != _SUCCESS\n",__FUNCTION__));
		}

	} while (1);

}
#endif

/*
 * Initialize recv private variable for hardware dependent
 * 1. recv buf
 * 2. recv tasklet
 *
 */
s32 rtl8195as_init_recv_priv(PADAPTER padapter)
{
	s32			res;
	u32			i, n;
	struct recv_buf		*precvbuf;
	struct recv_priv	*precvpriv = &padapter->recvpriv;

	res = _SUCCESS;

	//3 1. init recv buffer
	rtw_init_queue(&precvpriv->free_recv_buf_queue);
	rtw_init_queue(&precvpriv->recv_buf_pending_queue);

	n = NR_RECVBUFF * sizeof(struct recv_buf) + 4;
	precvpriv->pallocated_recv_buf = rtw_zmalloc(n);
	if (precvpriv->pallocated_recv_buf == NULL) {
		res = _FAIL;
		goto exit;
	}

	precvpriv->precv_buf = (u8*)N_BYTE_ALIGMENT((SIZE_PTR)(precvpriv->pallocated_recv_buf), 4);

	// init each recv buffer
	precvbuf = (struct recv_buf*)precvpriv->precv_buf;
	for (i = 0; i < NR_RECVBUFF; i++)
	{
		rtw_init_listhead(&precvbuf->list);

		precvbuf->adapter = padapter;

#ifdef CONFIG_SDIO_RX_COPY
#ifdef PLATFORM_ECOS
		precvbuf->pallocated_buf = rtw_zmalloc(MAX_RECVBUF_SZ + RECVBUFF_ALIGN_SZ);
		if (NULL == precvbuf->pallocated_buf)
		{
			DBG_871X("%s: alloc recvbuf fail!\n", __FUNCTION__);
			return _FAIL; //todo: need to free precvpriv->pallocated_recv_buf
		}
		//phead and pend should be modified here only
		precvbuf->phead = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(precvbuf->pallocated_buf), RECVBUFF_ALIGN_SZ);
		precvbuf->pend = precvbuf->pallocated_buf + (MAX_RECVBUF_SZ + RECVBUFF_ALIGN_SZ);
		precvbuf->pdata = precvbuf->phead;
		precvbuf->ptail = precvbuf->phead;
#else
		if (precvbuf->pskb == NULL) {
			SIZE_PTR tmpaddr=0;
			SIZE_PTR alignment=0;

			precvbuf->pskb = rtw_skb_alloc(MAX_RECVBUF_SZ + RECVBUFF_ALIGN_SZ);

			if(precvbuf->pskb)
			{
				precvbuf->pskb->dev = padapter->pnetdev;

				tmpaddr = (SIZE_PTR)precvbuf->pskb->data;
				alignment = tmpaddr & (RECVBUFF_ALIGN_SZ-1);
				skb_reserve(precvbuf->pskb, (RECVBUFF_ALIGN_SZ - alignment));

				precvbuf->phead = precvbuf->pskb->head;
				precvbuf->pdata = precvbuf->pskb->data;
				precvbuf->ptail = skb_tail_pointer(precvbuf->pskb);
				precvbuf->pend = skb_end_pointer(precvbuf->pskb);
				precvbuf->len = 0;
			}

			if (precvbuf->pskb == NULL) {
				DBG_871X("%s: alloc_skb fail!\n", __FUNCTION__);
			}
		}
#endif
#endif

		rtw_list_insert_tail(&precvbuf->list, &precvpriv->free_recv_buf_queue.queue);

		precvbuf++;
	}
	precvpriv->free_recv_buf_queue_cnt = i;
	precvpriv->min_free_recv_buf_queue_cnt = precvpriv->free_recv_buf_queue_cnt;

	//3 2. init tasklet
#ifdef PLATFORM_LINUX
	tasklet_init(&precvpriv->recv_tasklet,
	     (void(*)(unsigned long))rtl8195as_recv_tasklet,
	     (unsigned long)padapter);
#endif
#ifdef PLATFORM_ECOS
	//precvpriv->recv_tasklet = 1;
	precvpriv->recv_tasklet = rtl8195as_recv_tasklet;
	padapter->has_triggered_rx_tasklet = 0;
#endif
	goto exit;

exit:
	return res;
}


/*
 * Free recv private variable of hardware dependent
 * 1. recv buf
 * 2. recv tasklet
 *
 */
void rtl8195as_free_recv_priv(PADAPTER padapter)
{
	u32 n, i;
	struct recv_buf *precvbuf;
	struct recv_priv *precvpriv = &padapter->recvpriv;

	//3 1. kill tasklet
#ifdef PLATFORM_LINUX
	tasklet_kill(&precvpriv->recv_tasklet);
#endif
#ifdef PLATFORM_ECOS
	//precvpriv->recv_tasklet = 0;
	precvpriv->recv_tasklet = NULL;
	padapter->has_triggered_rx_tasklet = 0;
#endif
	//3 2. free all recv buffers
	precvbuf = (struct recv_buf*)precvpriv->precv_buf;
	if (precvbuf) {
		n = NR_RECVBUFF;
		precvpriv->min_free_recv_buf_queue_cnt = precvpriv->free_recv_buf_queue_cnt = 0;
		for (i = 0; i < n ; i++)
		{
			rtw_list_delete(&precvbuf->list);
			rtw_os_recvbuf_resource_free(padapter, precvbuf);
			precvbuf++;
		}
		precvpriv->precv_buf = NULL;
	}

	if (precvpriv->pallocated_recv_buf) {
		n = NR_RECVBUFF * sizeof(struct recv_buf) + 4;
		rtw_mfree(precvpriv->pallocated_recv_buf, n);
		precvpriv->pallocated_recv_buf = NULL;
		//precvpriv->precv_buf = NULL;
	}
}


