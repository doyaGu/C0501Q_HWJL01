#include "autoconf.h"
#include "rtw_debug.h"
#include "rtw_ioctl.h"
#include "rtw_xmit.h"
#include "rtw_cmd.h"
#include "hal_intf.h"
#include "8195_desc.h"
#include <linux/mmc/errorno.h> // Irene Lin
#include <linux/util.h> // Irene Lin
#define _IOCTL_LINUX_C_
//define some private IOCTL options which are not in wireless.h
#define RTL_IOCTL_ATCMD				(SIOCDEVPRIVATE+1)
#define RTL_IOCTL_SDIOPRIV				(SIOCDEVPRIVATE+2)
#define RTL_IOCTL_DBG					(SIOCDEVPRIVATE+3)
#define RTL_IOCTL_LOOPBACK			(SIOCDEVPRIVATE+4)
static int rtw_wx_atcmd(PADAPTER padapter, struct net_device *dev, struct iw_point *p)
{
	int ret = 0;
	struct xmit_buf *pxmitbuf;
	struct xmit_priv *pxmitpriv;
	PTXDESC_8195A ptxdesc;

_func_enter_;
	pxmitpriv = &padapter->xmitpriv;
	if (p->length <= 0 || !p->pointer){
		ret = -EINVAL;
		return ret;
	}

	pxmitbuf = rtw_alloc_xmitbuf(padapter);
	if(!pxmitbuf)
	{
		DBG_871X("%s(): pxmitbuf allocated failed!\n", __FUNCTION__);
		ret = -ENOMEM;
		goto exit;
	}
	
	pxmitbuf->pkt_len = p->length + SIZE_TX_DESC_8195a;
	ptxdesc = (PTXDESC_8195A)pxmitbuf->pbuf;
	ptxdesc->txpktsize = p->length;
	ptxdesc->offset = SIZE_TX_DESC_8195a;
	ptxdesc->type = TX_H2C_CMD;//indicate transmittion of H2C packet
	ptxdesc->bus_agg_num = 0;//to do
	if (copy_from_user((pxmitbuf->pbuf + SIZE_TX_DESC_8195a), p->pointer, p->length))
	{
		rtw_free_xmitbuf(padapter, pxmitbuf);
		ret = -EFAULT;
		goto exit;
	}
	
	if(rtw_hal_mgnt_xmit(padapter, pxmitbuf) == _FALSE){
		ret = -ENOMEM;
		goto exit;
	}

exit:
_func_exit_;
	return ret;
}
#ifdef CONFIG_SDIO_HCI
//this is for sdio debug
extern int rtw_io_rw_direct_host(PADAPTER padapter, u8 *in, u8 sz, u8 *out);
static int rtw_wx_sdiopriv(PADAPTER padapter, struct net_device *dev, struct iw_point *p)
{
	int ret = 0;
	u8 *pbuf = NULL;
	int sz = p->length + 1;//last byte for '\0'
_func_enter_;

	if (p->length <= 0 || !p->pointer){
		ret = -EINVAL;
		goto exit;
	}

	pbuf = rtw_zmalloc(sz);
	if(pbuf == NULL)
	{
		DBG_871X("%s: malloc failed (%d)\n", __FUNCTION__, sz);
		ret = -ENOMEM;
		goto exit;
	}
	
	if (copy_from_user(pbuf, p->pointer, p->length))
	{
		ret = -EFAULT;
		goto exit;
	}
	pbuf[sz - 1] = '\0';
	ret = rtw_io_rw_direct_host(padapter, pbuf, sz, p->pointer);
	
exit:
_func_exit_;
	if(pbuf)
		rtw_mfree(pbuf, sz);
	return ret;
}
#endif

#ifdef CONFIG_SDIO_RX_COPY
	#include <rtl8195a_recv.h>
#endif
static int rtw_wx_dbg(PADAPTER padapter, struct net_device *dev, struct iw_point *p)
{
	int ret = 0;
	u8 *pbuf = NULL;
	int sz = p->length + 1;//last byte for '\0'
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct recv_priv *precvpriv = &padapter->recvpriv;
_func_enter_;

	if (p->length <= 0 || !p->pointer){
		ret = -EINVAL;
		goto exit;
	}

	pbuf = rtw_zmalloc(sz);
	if(pbuf == NULL)
	{
		DBG_871X("%s: malloc failed (%d)\n", __FUNCTION__, sz);
		ret = -ENOMEM;
		goto exit;
	}
	
	if (copy_from_user(pbuf, p->pointer, p->length))
	{
		ret = -EFAULT;
		goto exit;
	}
	pbuf[sz - 1] = '\0';
	DBG_871X("XMITBUF: %d (%d)\n", pxmitpriv->min_free_xmitbuf_cnt, pxmitpriv->free_xmitbuf_cnt);
	pxmitpriv->min_free_xmitbuf_cnt = pxmitpriv->free_xmitbuf_cnt;
#ifdef CONFIG_TX_AGGREGATION
	DBG_871X("XMITFRAME: %d (%d)\n", pxmitpriv->min_free_xmitframe_cnt, pxmitpriv->free_xmitframe_cnt);
	pxmitpriv->min_free_xmitframe_cnt = pxmitpriv->free_xmitframe_cnt;
	DBG_871X("XMITMAXAGG: %d(%d), %d(%d)\n", pxmitpriv->max_agg_num, SDIO_TX_AGG_MAX, pxmitpriv->max_agg_pkt_len, MAX_XMITBUF_SZ);
	pxmitpriv->max_agg_num = 0;
	pxmitpriv->max_agg_pkt_len = 0;
#endif
	DBG_871X("RECVBUF: %d (%d)\n", precvpriv->min_free_recv_buf_queue_cnt, precvpriv->free_recv_buf_queue_cnt);
	precvpriv->min_free_recv_buf_queue_cnt = precvpriv->free_recv_buf_queue_cnt;
#ifdef CONFIG_SDIO_RX_COPY
	DBG_871X("RECVFRAME: %d (%d)\n", precvpriv->min_free_recvframe_cnt, precvpriv->free_recvframe_cnt);
	precvpriv->min_free_recvframe_cnt = precvpriv->free_recvframe_cnt;
	DBG_871X("RECVMAXAGG: %d(%d), %d(%d)\n", precvpriv->max_agg_num, MAX_RX_AGG_NUM, precvpriv->max_agg_pkt_len, MAX_RECVBUF_SZ);
	precvpriv->max_agg_num = 0;
	precvpriv->max_agg_pkt_len = 0;
#endif
	
exit:
_func_exit_;
	if(pbuf)
		rtw_mfree(pbuf, sz);
	return ret;
}

#ifdef CONFIG_LOOPBACK_TEST
extern void rtw_hal_start_loopback_test(_adapter *padapter);
extern void rtw_hal_stop_loopback_test(_adapter *padapter);
static int rtw_wx_loopback(PADAPTER padapter, struct net_device *dev, struct iw_point *p)
{
	int ret = 0;
	u8 *pbuf = NULL;
	int sz = p->length + 1;//last byte for '\0'

_func_enter_;

	if (p->length <= 0 || !p->pointer){
		ret = -EINVAL;
		goto exit;
	}

	pbuf = rtw_zmalloc(sz);
	if(pbuf == NULL)
	{
		DBG_871X("%s: malloc failed (%d)\n", __FUNCTION__, sz);
		ret = -ENOMEM;
		goto exit;
	}
	
	if (copy_from_user(pbuf, p->pointer, p->length))
	{
		ret = -EFAULT;
		goto exit;
	}
	pbuf[sz - 1] = '\0';
	
	if(rtw_memcmp(pbuf+9, "start", 5) == _TRUE)
		rtw_hal_start_loopback_test(padapter);
	else
		rtw_hal_stop_loopback_test(padapter);
	
exit:
_func_exit_;
	if(pbuf)
		rtw_mfree(pbuf, sz);
	return ret;
}
#endif

int rtw_ioctl(struct net_device *pnetdev, struct iwreq *rq, int cmd)
{
	PADAPTER padapter = ((struct rtw_netdev_priv_indicator *)netdev_priv(pnetdev))->priv;
	struct iwreq *wrq = (struct iwreq *)rq;
	int ret = 0;
_func_enter_;
	DBG_871X("%s=======> cmd: 0x%04x\n", __FUNCTION__, cmd);
	switch (cmd)
	{
		case RTL_IOCTL_ATCMD:
			ret = rtw_wx_atcmd(padapter, pnetdev, &wrq->u.data);
			break;
#ifdef CONFIG_SDIO_HCI
		case RTL_IOCTL_SDIOPRIV:
			ret = rtw_wx_sdiopriv(padapter, pnetdev, &wrq->u.data);
			break;
#endif
		case RTL_IOCTL_DBG:
			ret = rtw_wx_dbg(padapter, pnetdev, &wrq->u.data);
			break;
#ifdef CONFIG_LOOPBACK_TEST
		case RTL_IOCTL_LOOPBACK:
			ret = rtw_wx_loopback(padapter, pnetdev, &wrq->u.data);
			break;
#endif
		default:
			ret = -EOPNOTSUPP;
			break;
	}
_func_exit_;
	return ret;
}
