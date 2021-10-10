#ifndef __RTW_RECV_H__
#define __RTW_RECV_H__

#include <linux/tasklet.h>  // Irene Lin
#include "drv_types.h"

#ifdef PLATFORM_LINUX//PLATFORM_LINUX /PLATFORM_BSD

	#ifdef CONFIG_SINGLE_RECV_BUF
		#define NR_RECVBUFF (1)
	#else
		#if defined(CONFIG_GSPI_HCI)
			#define NR_RECVBUFF (32)
		#elif defined(CONFIG_SDIO_HCI)
			#define NR_RECVBUFF (8)	
		#else
			#define NR_RECVBUFF (16) // 
		#endif	
	#endif //CONFIG_SINGLE_RECV_BUF

	#define NR_PREALLOC_RECV_SKB (8)	
#elif defined(PLATFORM_FREERTOS)
	//	#define NR_RECVBUFF (8) //Decrease recv buffer due to memory limitation - Alex Fang
	#define NR_RECVBUFF (1)	//Decrease recv buffer due to memory limitation - YangJue
#elif defined(PLATFORM_ECOS)
#define NR_RECVBUFF (2)
#endif

#define RECVBUFF_ALIGN_SZ 8

struct recv_priv
{
	_lock lock;
	PADAPTER adapter;
	u8 *pallocated_recv_buf;
	u8 *precv_buf;
#if defined(CONFIG_SDIO_RX_COPY) || defined(CONFIG_USB_RX_AGGREGATION)
	_queue	free_recv_queue;
	_queue	recv_pending_queue;
//	_queue	uc_swdec_pending_queue;


	u8 *pallocated_frame_buf;
	u8 *precv_frame_buf;

	uint free_recvframe_cnt;
	uint min_free_recvframe_cnt; //monitor recvframe usage
#endif

#ifdef CONFIG_USB_HCI
		//u8 *pallocated_urb_buf;
		_sema allrxreturnevt;
//		uint	ff_hwaddr;
		u8	rx_pending_cnt;
	
#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
#ifdef PLATFORM_LINUX
		PURB	int_in_urb;
#endif
	
		u8	*int_in_buf;
#endif //CONFIG_USB_INTERRUPT_IN_PIPE
	
#endif

#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD)
#ifdef PLATFORM_FREEBSD
	struct task recv_tasklet;
#else
	struct tasklet_struct recv_tasklet;
#endif
#elif defined(PLATFORM_FREERTOS)
#ifdef CONFIG_RECV_TASKLET_THREAD
	struct task_struct	recvtasklet_thread;
#endif
#elif defined(PLATFORM_ECOS)
	//int recv_tasklet;
	//thread_func_t recv_tasklet;
	void (*recv_tasklet)(void* priv);
#endif

	struct sk_buff_head free_recv_skb_queue;
	struct sk_buff_head rx_skb_queue;

	u32 free_recv_buf_queue_cnt;
	u32 min_free_recv_buf_queue_cnt; //monitor recvbuf usage
	u32 max_agg_num; //monitor rx agg number for each rx packet
	u32 max_agg_pkt_len;
	_queue free_recv_buf_queue;
	_queue recv_buf_pending_queue;

	u64	rx_bytes;
	u64	rx_pkts;
	u64	rx_drop;
};
struct recv_buf
{
	_list list;
	PADAPTER adapter;
	u32	len;
	u8	*phead;
	u8	*pdata;
	u8	*ptail;
	u8	*pend;
	
	u8	*pbuf;
	u8	*pallocated_buf;
	u32 ref_cnt;

	
#ifdef CONFIG_USB_HCI
	
	#if defined(PLATFORM_OS_XP)||defined(PLATFORM_LINUX)||defined(PLATFORM_FREEBSD)
		PURB	purb;
		dma_addr_t dma_transfer_addr;	/* (in) dma addr for transfer_buffer */
		u32 alloc_sz;
	#endif
	
	#ifdef PLATFORM_OS_XP
		PIRP		pirp;
	#endif
	
	#ifdef PLATFORM_OS_CE
		USB_TRANSFER	usb_transfer_read_port;
	#endif
	
//		u8	irp_pending;
		int  transfer_len;
	
#endif
#if defined(PLATFORM_LINUX) ||defined(PLATFORM_FREERTOS) ||defined(PLATFORM_ECOS)
	_pkt *pskb;
#endif
};

#if defined (CONFIG_SDIO_RX_COPY) || defined(CONFIG_USB_RX_AGGREGATION)

struct recv_stat
{
	unsigned int rxdw0;

	unsigned int rxdw1;

	unsigned int rxdw2;

	unsigned int rxdw3;

	unsigned int rxdw4;

	unsigned int rxdw5;

};

struct rx_pkt_attrib	{
	u16	pkt_len;
	u8	shift_sz;
	u8	crc_err;
	u8	icv_err;
	u8 	pkt_rpt_type;	
};

/*
	head  ----->

		data  ----->

			payload

		tail  ----->


	end   ----->

	len = (unsigned int )(tail - data);

*/
#define RECVFRAME_HDR_ALIGN 128
#define NR_RECVFRAME 256

#define RXFRAME_ALIGN	8
#define RXFRAME_ALIGN_SZ	(1<<RXFRAME_ALIGN)
struct recv_frame_hdr
{
	_list	list;

	_pkt	*pkt;

	_adapter  *adapter;

//	u8 fragcnt;

//	int frame_tag;

	struct rx_pkt_attrib attrib;

	uint  len;
	u8 *rx_head;
	u8 *rx_data;
	u8 *rx_tail;
	u8 *rx_end;

	void *precvbuf;

};


union recv_frame{

	union{
		_list list;
		struct recv_frame_hdr hdr;
		uint mem[RECVFRAME_HDR_ALIGN>>2];
	}u;

	//uint mem[MAX_RXSZ>>2];

};
#endif
__inline static u8 *recvbuf_push(struct recv_buf *precvbuf, sint sz)
{
	// append data before rx_data

	/* add data to the start of recv_frame
 *
 *      This function extends the used data area of the recv_frame at the buffer
 *      start. rx_data must be still larger than rx_head, after pushing.
 */

	if(precvbuf==NULL)
		return NULL;


	precvbuf->pdata -= sz ;
	if( precvbuf->pdata < precvbuf->phead)
	{
		precvbuf->pdata += sz ;
		return NULL;
	}

	precvbuf->len +=sz;

	return precvbuf->pdata;

}


__inline static u8 *recvbuf_pull(struct recv_buf *precvbuf, sint sz)
{
	// rx_data += sz; move rx_data sz bytes  hereafter

	//used for extract sz bytes from rx_data, update rx_data and return the updated rx_data to the caller


	if(precvbuf==NULL)
		return NULL;


	precvbuf->pdata += sz;

	if(precvbuf->pdata > precvbuf->ptail)
	{
		precvbuf->pdata -= sz;
		return NULL;
	}

	precvbuf->len -=sz;

	return precvbuf->pdata;

}

__inline static u8 *recvbuf_put(struct recv_buf *precvbuf, sint sz)
{
	// rx_tai += sz; move rx_tail sz bytes  hereafter

	//used for append sz bytes from ptr to rx_tail, update rx_tail and return the updated rx_tail to the caller
	//after putting, rx_tail must be still larger than rx_end.

	if(precvbuf==NULL)
		return NULL;

	precvbuf->ptail += sz;

	if(precvbuf->ptail > precvbuf->pend)
	{
		precvbuf->ptail -= sz;
		return NULL;
	}

	precvbuf->len +=sz;

	return precvbuf->ptail;

}



__inline static u8 *recvbuf_pull_tail(struct recv_buf *precvbuf, sint sz)
{
	// rmv data from rx_tail (by yitsen)

	//used for extract sz bytes from rx_end, update rx_end and return the updated rx_end to the caller
	//after pulling, rx_end must be still larger than rx_data.

	if(precvbuf==NULL)
		return NULL;

	precvbuf->ptail -= sz;

	if(precvbuf->ptail < precvbuf->pdata)
	{
		precvbuf->ptail += sz;
		return NULL;
	}

	precvbuf->len -=sz;

	return precvbuf->ptail;

}

#if defined (CONFIG_SDIO_RX_COPY) || defined(CONFIG_USB_RX_AGGREGATION)
__inline static u8 *recvframe_push(union recv_frame *precvframe, sint sz)
{
	// append data before rx_data

	/* add data to the start of recv_frame
 *
 *      This function extends the used data area of the recv_frame at the buffer
 *      start. rx_data must be still larger than rx_head, after pushing.
 */

	if(precvframe==NULL)
		return NULL;


	precvframe->u.hdr.rx_data -= sz ;
	if( precvframe->u.hdr.rx_data < precvframe->u.hdr.rx_head )
	{
		precvframe->u.hdr.rx_data += sz ;
		return NULL;
	}

	precvframe->u.hdr.len +=sz;

	return precvframe->u.hdr.rx_data;

}


__inline static u8 *recvframe_pull(union recv_frame *precvframe, sint sz)
{
	// rx_data += sz; move rx_data sz bytes  hereafter

	//used for extract sz bytes from rx_data, update rx_data and return the updated rx_data to the caller


	if(precvframe==NULL)
		return NULL;


	precvframe->u.hdr.rx_data += sz;

	if(precvframe->u.hdr.rx_data > precvframe->u.hdr.rx_tail)
	{
		precvframe->u.hdr.rx_data -= sz;
		return NULL;
	}

	precvframe->u.hdr.len -=sz;

	return precvframe->u.hdr.rx_data;

}

__inline static u8 *recvframe_put(union recv_frame *precvframe, sint sz)
{
	// rx_tai += sz; move rx_tail sz bytes  hereafter

	//used for append sz bytes from ptr to rx_tail, update rx_tail and return the updated rx_tail to the caller
	//after putting, rx_tail must be still larger than rx_end.
 	unsigned char * prev_rx_tail;

	if(precvframe==NULL)
		return NULL;

	prev_rx_tail = precvframe->u.hdr.rx_tail;

	precvframe->u.hdr.rx_tail += sz;

	if(precvframe->u.hdr.rx_tail > precvframe->u.hdr.rx_end)
	{
		precvframe->u.hdr.rx_tail -= sz;
		return NULL;
	}

	precvframe->u.hdr.len +=sz;

	return precvframe->u.hdr.rx_tail;

}

__inline static u8 *recvframe_pull_tail(union recv_frame *precvframe, sint sz)
{
	// rmv data from rx_tail (by yitsen)

	//used for extract sz bytes from rx_end, update rx_end and return the updated rx_end to the caller
	//after pulling, rx_end must be still larger than rx_data.

	if(precvframe==NULL)
		return NULL;

	precvframe->u.hdr.rx_tail -= sz;

	if(precvframe->u.hdr.rx_tail < precvframe->u.hdr.rx_data)
	{
		precvframe->u.hdr.rx_tail += sz;
		return NULL;
	}

	precvframe->u.hdr.len -=sz;

	return precvframe->u.hdr.rx_tail;

}
union recv_frame *rtw_alloc_recvframe (_queue *pfree_recv_queue);
void rtw_init_recvframe(union recv_frame *precvframe, struct recv_priv *precvpriv);
int rtw_free_recvframe(union recv_frame *precvframe, _queue *pfree_recv_queue);
sint rtw_enqueue_recvframe(union recv_frame *precvframe, _queue *queue);
void rtw_free_recvframe_queue(_queue *pframequeue,  _queue *pfree_recv_queue);
sint rtw_enqueue_recvbuf_to_head(struct recv_buf *precvbuf, _queue *queue);
#endif
s32 rtw_init_recv_priv(_adapter *padapter);
void rtw_free_recv_priv(_adapter *padapter);
s32 rtw_enqueue_recvbuf(struct recv_buf *precvbuf, _queue *queue);
struct recv_buf *rtw_dequeue_recvbuf (_queue *queue);

#endif
