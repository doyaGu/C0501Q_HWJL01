#ifndef __RTL8195A_XMIT_H__
#define __RTL8195A_XMIT_H__
#include "drv_types.h"
s32 rtl8195as_init_xmit_priv(PADAPTER padapter);
void rtl8195as_free_xmit_priv(PADAPTER padapter);
s32 rtl8195as_hal_xmit_handler(PADAPTER padapter);
#ifdef CONFIG_TX_AGGREGATION
s32 rtl8195as_hal_xmit(PADAPTER padapter, struct xmit_frame *pxmitframe);
#else
s32 rtl8195as_hal_xmit(PADAPTER padapter, struct xmit_buf *pxmitbuf);
#endif
s32 rtl8195as_hal_mgnt_xmit(PADAPTER padapter, struct xmit_buf *pxmitbuf);

#ifdef CONFIG_USB_HCI
s32 rtl8195au_init_xmit_priv(PADAPTER padapter);
void rtl8195au_free_xmit_priv(PADAPTER padapter);
s32 rtl8195au_hal_xmit_handler(PADAPTER padapter);
#ifdef CONFIG_USB_TX_AGGREGATION
s32 rtl8195au_hal_xmit(PADAPTER padapter, struct xmit_frame *pxmitframe);
#else
s32 rtl8195au_hal_xmit(PADAPTER padapter, struct xmit_buf *pxmitbuf);
#endif
s32 rtl8195au_hal_mgnt_xmit(PADAPTER padapter, struct xmit_buf *pxmitbuf);
void rtl8195au_xmit_tasklet(void *priv);
s32 rtl8195au_xmitframe_complete(PADAPTER padapter, struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf);
//s32	rtl8195au_hal_xmitframe_enqueue(PADAPTER padapter, struct xmit_frame *pxmitframe);
//s32 rtl8195au_xmit_buf_handler(PADAPTER padapter);
#endif
#ifdef CONFIG_GSPI_HCI
s32 rtl8195a_gspi_init_xmit_priv(PADAPTER padapter);
void rtl8195a_gspi_free_xmit_priv(PADAPTER padapter);
s32 rtl8195as_spi_dequeue_writeport(PADAPTER padapter);
s32 rtl8195a_gspi_hal_xmit_handler(PADAPTER padapter);
s32 rtl8195a_gspi_hal_xmit(PADAPTER padapter, struct xmit_buf *pxmitbuf);
s32 rtl8195a_gspi_hal_mgnt_xmit(PADAPTER padapter, struct xmit_buf *pxmitbuf);
#endif /*CONFIG_GSPI_HCI*/
void rtl8195as_xmit_tasklet(void* data);
#endif