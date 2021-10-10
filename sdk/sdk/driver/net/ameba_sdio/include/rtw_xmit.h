#ifndef __RTW_XMIT_H__
#define __RTW_XMIT_H__

#define NULL_FRAMETAG		(0x0)
#define DATA_FRAMETAG		(0x01)
#define CMD_FRAMETAG		(0x02)

struct  submit_ctx{
	u32 submit_time; /* */
	u32 timeout_ms; /* <0: not synchronous, 0: wait forever, >0: up to ms waiting */
	int status; /* status for operation */
#ifdef PLATFORM_LINUX
	struct completion done;
#endif
};

enum {
	RTW_SCTX_SUBMITTED = -1,
	RTW_SCTX_DONE_SUCCESS = 0,
	RTW_SCTX_DONE_UNKNOWN,
	RTW_SCTX_DONE_TIMEOUT,
	RTW_SCTX_DONE_BUF_ALLOC,
	RTW_SCTX_DONE_BUF_FREE,
	RTW_SCTX_DONE_WRITE_PORT_ERR,
	RTW_SCTX_DONE_TX_DESC_NA,
	RTW_SCTX_DONE_TX_DENY,
	RTW_SCTX_DONE_CCX_PKT_FAIL,
	RTW_SCTX_DONE_DRV_STOP,
	RTW_SCTX_DONE_DEV_REMOVE,
	RTW_SCTX_DONE_CMD_ERROR,
};

void rtw_sctx_init(struct submit_ctx *sctx, int timeout_ms);
int rtw_sctx_wait(struct submit_ctx *sctx, const char *msg);
void rtw_sctx_done_err(struct submit_ctx **sctx, int status);
void rtw_sctx_done(struct submit_ctx **sctx);


struct tx_desc
{
	unsigned int txdw0;
	unsigned int txdw1;
	unsigned int txdw2;
	unsigned int txdw3;
	unsigned int txdw4;
	unsigned int txdw5;
};



struct xmit_buf
{
	_list list;
	PADAPTER padapter;
	_pkt *pkt;
	u16 pkt_len;
	u8 *pallocated_buf;
	u8 *pbuf;
#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
	u8 *phead;
	u8 *pdata;
	u8 *ptail;
	u8 *pend;
	u8	agg_num;
	void *priv_data;
#endif

struct submit_ctx *sctx;

#ifdef CONFIG_USB_HCI
#define NR_URB_PERXMITBUF	1
#if defined(PLATFORM_OS_XP)||defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD)
	PURB	pxmit_urb[NR_URB_PERXMITBUF];
	dma_addr_t dma_transfer_addr;	/* (in) dma addr for transfer_buffer */
#endif

#ifdef PLATFORM_OS_XP
	PIRP		pxmit_irp[NR_URB_PERXMITBUF];
#endif

#ifdef PLATFORM_OS_CE
	USB_TRANSFER	usb_transfer_write_port;
#endif

#endif
#if defined(DBG_XMIT_BUF )|| defined(DBG_XMIT_BUF_EXT)
	u8 no;
#endif
};
struct xmit_priv
{
	_lock lock;
	PADAPTER padapter;
	_queue free_xmit_queue;
	_queue xmitbuf_pending_queue;
	
	u8 *pallocated_freebuf;
	u8 *xmit_freebuf;
	uint free_xmitbuf_cnt;
	uint min_free_xmitbuf_cnt;
	
#ifdef CONFIG_TX_AGGREGATION
	u8 *pallocated_frame_buf;
	u8 *pxmit_frame_buf;
	uint free_xmitframe_cnt;
	uint min_free_xmitframe_cnt;
	uint max_agg_num;
	uint max_agg_pkt_len;
	_queue	free_xmitframe_queue;
	_queue	xmitframe_pending_queue;

	_thread_hdl_	SdioXmitThread;

	_sema		SdioXmitSema;	
	_sema		SdioXmitTerminateSema;
#endif

#ifdef CONFIG_USB_TX_AGGREGATION
	u8 *pallocated_frame_buf;
	u8 *pxmit_frame_buf;
	uint free_xmitframe_cnt;
	_queue	free_xmitframe_queue;
	_queue	xmitframe_pending_queue;

	_thread_hdl_	UsbXmitThread;
	_sema		UsbXmitSema;	
	_sema		UsbXmitTerminateSema;
#endif

	u64	tx_bytes;
	u64	tx_pkts;
	u64	tx_drop;

	_sema xmit_sema;
	_sema XmitTerminateSema;
#if defined(PLATFORM_FREERTOS)
	struct task_struct	xmitThread;
#else
	_thread_hdl_ xmitThread;
#endif

#ifdef PLATFORM_ECOS
	//int xmit_tasklet;
	//thread_func_t xmit_tasklet;
	void (*xmit_tasklet)(void* priv);
#endif

#ifdef CONFIG_USB_HCI
		_sema	tx_retevt;//all tx return event;
		u8		txirp_cnt;//
	
	#ifdef PLATFORM_OS_CE
		USB_TRANSFER	usb_transfer_write_port;
	//	USB_TRANSFER	usb_transfer_write_mem;
	#endif
	#ifdef PLATFORM_LINUX
		struct tasklet_struct xmit_tasklet;
	#endif
	#ifdef PLATFORM_FREEBSD
		struct task xmit_tasklet;
	#endif
#endif

	_lock lock_sctx;

};

#define XMITBUF_ALIGN_SZ		512

#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
#ifdef CONFIG_TX_AGGREGATION
#ifdef PLATFORM_ECOS
	#define SDIO_TX_AGG_MAX	(8)
#else
	#define SDIO_TX_AGG_MAX	(8)
#endif
	#define MAX_XMITBUF_SZ	(SDIO_TX_AGG_MAX*1540)	// FW source limit
#else
	#define MAX_XMITBUF_SZ (1664)
	#define SDIO_TX_AGG_MAX	1
#endif

#if defined CONFIG_SDIO_HCI
#ifdef CONFIG_TX_AGGREGATION
#ifdef PLATFORM_ECOS
	#define NR_XMITBUFF	(40)
#else
	#define NR_XMITBUFF	(32)
#endif
#else
#ifdef PLATFORM_ECOS
	#define NR_XMITBUFF	(256)
#else
	#define NR_XMITBUFF	(128)
#endif
#endif
#endif
#if defined(CONFIG_GSPI_HCI)
#ifdef PLATFORM_FREERTOS
	#define NR_XMITBUFF	(20)
#else
	#define NR_XMITBUFF	(128)
#endif
#endif

#elif defined (CONFIG_USB_HCI)

#ifdef CONFIG_USB_TX_AGGREGATION
	#define USB_TX_AGG_MAX	3
	#define MAX_XMITBUF_SZ	(USB_TX_AGG_MAX*2048)	// USB_TX_AGG_MAX*2k
#else
	#define MAX_XMITBUF_SZ	(2048)
#endif

#ifdef CONFIG_SINGLE_XMIT_BUF
	#define NR_XMITBUFF	(1)
#else
	#define NR_XMITBUFF	(128)
#endif //CONFIG_SINGLE_XMIT_BUF
#elif defined (CONFIG_PCI_HCI)
	#define MAX_XMITBUF_SZ	(1664)
	#define NR_XMITBUFF	(128)
#endif

#if defined(CONFIG_TX_AGGREGATION) || defined(CONFIG_USB_TX_AGGREGATION)
struct xmit_frame
{
	_list	list;

//	struct pkt_attrib attrib;

	_pkt *pkt;
	u16 pkt_len;
	
	int	frame_tag;

	_adapter *padapter;

	u8	*buf_addr;

	struct xmit_buf *pxmitbuf;

#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
//	u8	pg_num;
	u8	agg_num;
#endif

#ifdef CONFIG_USB_HCI
#ifdef CONFIG_USB_TX_AGGREGATION
	u8	agg_num;
#endif
	s8	pkt_offset;
#ifdef CONFIG_RTL8192D
	u8	EMPktNum;
	u16	EMPktLen[5];//The max value by HW
#endif
#endif

#ifdef CONFIG_XMIT_ACK
	u8 ack_report;
#endif

	u8 *alloc_addr; /* the actual address this xmitframe allocated */
	u8 ext_tag; /* 0:data, 1:mgmt */

};
void rtw_count_tx_stats(PADAPTER padapter, struct xmit_frame *pxmitframe, int sz);
s32 rtw_xmitframe_enqueue(_adapter *padapter, struct xmit_frame *pxmitframe);
s32 check_pending_xmitframe(PADAPTER padapter);
s32 rtw_free_xmitframe(struct xmit_priv *pxmitpriv, struct xmit_frame *pxmitframe);
void rtw_free_xmitframe_queue(struct xmit_priv *pxmitpriv, _queue *pframequeue);
s32 rtw_xmit(_adapter *padapter, _pkt *ppkt);
#endif
s32 rtw_init_xmit_priv(PADAPTER padapter);
void rtw_free_xmit_priv(PADAPTER padapter);
struct xmit_buf *rtw_alloc_xmitbuf(PADAPTER padapter);
s32 rtw_free_xmitbuf(PADAPTER padapter, struct xmit_buf *pxmitbuf);
struct xmit_buf* rtw_dequeue_xmitbuf(PADAPTER padapter);
void enqueue_pending_xmitbuf(struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf);
void enqueue_pending_xmitbuf_to_head(struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf);
s32 check_pending_xmitbuf(PADAPTER padapter);
void rtw_free_xmitbuf_queue(PADAPTER padapter, _queue *pframequeue);
thread_return rtw_xmit_thread(thread_context context);
#endif
