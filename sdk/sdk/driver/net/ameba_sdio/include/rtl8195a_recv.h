#ifndef __RTL8195A_RECV_H__
#define __RTL8195A_RECV_H__
#include <drv_types.h>
#include <8195_desc.h>
#if defined(CONFIG_USB_HCI)

/*
#ifdef PLATFORM_OS_CE
#define MAX_RECVBUF_SZ (8192+1024) // 8K+1k
#else
	#ifdef CONFIG_MINIMAL_MEMORY_USAGE
		#define MAX_RECVBUF_SZ (4000) // about 4K
	#else
		#ifdef CONFIG_PLATFORM_MSTAR
			#define MAX_RECVBUF_SZ (8192) // 8K
		#else
		#define MAX_RECVBUF_SZ (32768) // 32k
		#endif
		//#define MAX_RECVBUF_SZ (20480) //20K
		//#define MAX_RECVBUF_SZ (10240) //10K 
		//#define MAX_RECVBUF_SZ (16384) //  16k - 92E RX BUF :16K
		//#define MAX_RECVBUF_SZ (8192+1024) // 8K+1k		
	#endif
#endif
*/
#ifdef CONFIG_USB_RX_AGGREGATION
#define MAX_RX_AGG_NUM	10
#define MAX_RECVBUF_SZ (MAX_RX_AGG_NUM * 2048)
#else
#define MAX_RECVBUF_SZ (2048)
#endif

#elif defined(CONFIG_PCI_HCI)
//#ifndef CONFIG_MINIMAL_MEMORY_USAGE
//	#define MAX_RECVBUF_SZ (9100)
//#else
	#define MAX_RECVBUF_SZ (4000) // about 4K
//#endif


#elif defined(CONFIG_SDIO_HCI)
#ifdef PLATFORM_ECOS
#define MAX_RX_AGG_NUM	12
#else
#define MAX_RX_AGG_NUM	6
#endif
#define MAX_RECVBUF_SZ (MAX_RX_AGG_NUM * 2048)
#endif


s32 rtl8195as_init_recv_priv(PADAPTER padapter);
void rtl8195as_free_recv_priv(PADAPTER padapter);


s32 rtl8195au_init_recv_priv(PADAPTER padapter);
void rtl8195au_free_recv_priv(PADAPTER padapter);
void rtl8195au_init_recvbuf(PADAPTER padapter, struct recv_buf *precvbuf);

//void update_recvframe_attrib_95a(	union recv_frame *precvframe,	struct recv_stat *prxstat);


#ifdef CONFIG_GSPI_HCI
void rtl8195a_gspi_recv_tasklet(void *priv);
s32 rtl8195a_gspi_init_recv_priv(PADAPTER padapter);
void rtl8195a_gspi_free_recv_priv(PADAPTER padapter);
#endif /*CONFIG_GSPI_HCI*/

#endif
