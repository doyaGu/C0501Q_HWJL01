#include <sdio_drvio.h>
#include <sdio_ops.h>
#include <8195_sdio_reg.h>
#include <rtl8195a_recv.h>
#include <rtw_debug.h>
//
// Description:
//	The following mapping is for SDIO host local register space.
//
// Creadted by Roger, 2011.01.31.
//
static void HalSdioGetCmdAddr8195ASdio(
	IN	PADAPTER	padapter,
	IN 	u8				DeviceID,
	IN	u32				Addr,
	OUT	u32*			pCmdAddr
	)
{
	switch (DeviceID)
	{
		case SDIO_LOCAL_DEVICE_ID:
			*pCmdAddr = ((SDIO_LOCAL_DEVICE_ID << 13) | (Addr & SDIO_LOCAL_MSK));
			break;

		case WLAN_TX_FIFO_DEVICE_ID:
			*pCmdAddr = ((WLAN_TX_FIFO_DEVICE_ID << 13) | (Addr & WLAN_TX_FIFO_MSK));
			break;

		case WLAN_RX_FIFO_DEVICE_ID:
			*pCmdAddr = ((WLAN_RX_FIFO_DEVICE_ID << 13) | (Addr & WLAN_RX_FIFO_MSK));
			break;

		default:
			break;
	}
}
static u8 get_deviceid(u32 addr)
{
	u8 deviceId;
	u16 pseudoId;


	pseudoId = (u16)(addr >> 16);
	switch (pseudoId)
	{
		case 0x1025:
			deviceId = SDIO_LOCAL_DEVICE_ID;
			break;

		case 0x1031:
			deviceId = WLAN_TX_FIFO_DEVICE_ID;
			break;

		case 0x1034:
			deviceId = WLAN_RX_FIFO_DEVICE_ID;
			break;

		default:
			deviceId = SDIO_LOCAL_DEVICE_ID;
			break;
	}

	return deviceId;
}


/*
 * Ref:
 *	HalSdioGetCmdAddr8195ASdio()
 */
static u32 _cvrt2ftaddr(const u32 addr, u8 *pdeviceId, u16 *poffset)
{
	u8 deviceId;
	u16 offset;
	u32 ftaddr;


	deviceId = get_deviceid(addr);
	offset = 0;

	switch (deviceId)
	{
		case SDIO_LOCAL_DEVICE_ID:
			offset = addr & SDIO_LOCAL_MSK;
			break;

		case WLAN_TX_FIFO_DEVICE_ID:
			offset = addr & WLAN_TX_FIFO_MSK;
			break;

		case WLAN_RX_FIFO_DEVICE_ID:
			offset = addr & WLAN_RX_FIFO_MSK;
			break;

		default:
			deviceId = SDIO_LOCAL_DEVICE_ID;
			offset = addr & SDIO_LOCAL_MSK;
			break;
	}
	ftaddr = (deviceId << 13) | offset;

	if (pdeviceId) *pdeviceId = deviceId;
	if (poffset) *poffset = offset;

	return ftaddr;
}

u8 sdio_read8(PADAPTER padapter, u32 addr)
{
	u32 ftaddr;
	u8 val;

_func_enter_;

	ftaddr = _cvrt2ftaddr(addr, NULL, NULL);
	sd_cmd52_read(padapter, ftaddr, 1, (u8*)&val);	

_func_exit_;

	return val;
}

u8 sdio_f0_read8(PADAPTER padapter, u32 addr)
{
	u8 val;

_func_enter_;
	val = sd_f0_read8(padapter, addr, NULL);

_func_exit_;

	return val;
}

u16 sdio_read16(PADAPTER padapter, u32 addr)
{
	u32 ftaddr;
	u16 val;

_func_enter_;

	ftaddr = _cvrt2ftaddr(addr, NULL, NULL);
	sd_cmd52_read(padapter, ftaddr, 2, (u8*)&val);	

	val = le16_to_cpu(val);

_func_exit_;

	return val;
}

u32 sdio_read32(PADAPTER padapter, u32 addr)
{
	u32 ftaddr;
	u32 val;
	u8 shift;
//	s32 err;
	
_func_enter_;
	ftaddr = _cvrt2ftaddr(addr, NULL, NULL);

#if 0//def CONFIG_POWER_SAVING
	if (_TRUE == adapter_to_pwrctl(padapter)->bFwCurrentInPSMode)
        {
		sd_cmd52_read(padapter, ftaddr, 4, (u8*)&val);
		val = le32_to_cpu(val);
		return val;
	}
#endif

	// 4 bytes alignment
	shift = ftaddr & 0x3;
	if (shift == 0) {
		val = sd_read32(padapter, ftaddr, NULL);
	} else {
		u8 *ptmpbuf;
				
		ptmpbuf = (u8*)rtw_malloc(8);
		if (NULL == ptmpbuf) {
			DBG_871X("%s: Allocate memory FAIL!(size=8) addr=0x%x\n", __func__, addr);
			return _FAIL;
		}

		ftaddr &= ~(u16)0x3;
		sd_read(padapter, ftaddr, 8, ptmpbuf);
		rtw_memcpy(&val, ptmpbuf+shift, 4);
		val = le32_to_cpu(val);

		rtw_mfree(ptmpbuf, 8);
	}

_func_exit_;

	return val;
}

s32 sdio_readN(PADAPTER padapter, u32 addr, u32 cnt, u8* pbuf)
{

	u8 deviceId;
	u16 offset;
	u32 ftaddr;
	u8 shift;
	s32 err;

_func_enter_;

	err = 0;

	ftaddr = _cvrt2ftaddr(addr, &deviceId, &offset);

#if 0//def CONFIG_POWER_SAVING
	if ((_TRUE == adapter_to_pwrctl(padapter)->bFwCurrentInPSMode))
	{
		err = sd_cmd52_read(padapter, ftaddr, cnt, pbuf);
		return err;
	}
#endif

	// 4 bytes alignment
	shift = ftaddr & 0x3;
	if (shift == 0) {
		err = sd_read(padapter, ftaddr, cnt, pbuf);
	} else {
		u8 *ptmpbuf;
		u32 n;

		ftaddr &= ~(u16)0x3;
		n = cnt + shift;
		ptmpbuf = rtw_malloc(n);
		if (NULL == ptmpbuf) return -1;
		err = sd_read(padapter, ftaddr, n, ptmpbuf);
		if (!err)
			rtw_memcpy(pbuf, ptmpbuf+shift, cnt);
		rtw_mfree(ptmpbuf, n);
	}

_func_exit_;

	return err;
}

void sdio_read_mem(PADAPTER padapter, u32 addr, u32 cnt, u8 *rmem)
{
	s32 err;

_func_enter_;

	err = sdio_readN(padapter, addr, cnt, rmem);

_func_exit_;
}

/*
 * Description:
 *	Read from RX FIFO
 *	Round read size to block size,
 *	and make sure data transfer will be done in one command.
 *
 * Parameters:
 *	func		a pointer of sdio func
 *	addr		port ID
 *	cnt			size to read
 *	rmem		address to put data
 *
 * Return:
 *	_SUCCESS(1)		Success
 *	_FAIL(0)		Fail
 */

u32 sdio_read_port(
//	struct sdio_func *func,
	PADAPTER padapter,
	u32 addr,
	u32 cnt,
	u8 *mem)
{
	s32 err;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
//	DBG_871X("%s(): addr is %d\n", __func__, addr);
//	DBG_871X("%s(): SDIORxFIFOCnt is %d\n", __func__, padapter->SdioRxFIFOCnt);
_func_enter_;

	HalSdioGetCmdAddr8195ASdio(padapter, addr, psdio->SdioRxFIFOCnt++, &addr);

//	DBG_871X("%s(): Get Cmd Addr is 0x%x\n", __func__, addr);


	cnt = _RND4(cnt);

#if 1//def PLATFORM_ECOS
	if (cnt >  psdio->block_transfer_len)
		cnt = _RND(cnt,  psdio->block_transfer_len);
#else
	if (cnt > psdio->func->cur_blksize)
		cnt = _RND(cnt, psdio->func->cur_blksize);
#endif
	
//	cnt = sdio_align_size(cnt);
 	//DBG_871X("%s(): cnt is %d\n", __func__, cnt);
//#ifdef PLATFORM_ECOS
	err = sd_read(padapter, addr, cnt, mem);
//#else
//	err = _sd_read(padapter, addr, cnt, mem);
//#endif
_func_exit_;	
	if (err) return _FAIL;
	return _SUCCESS;
}


s32 sdio_write8(PADAPTER padapter, u32 addr, u8 val)
{
	u32 ftaddr;
	s32 err;

_func_enter_;
	ftaddr = _cvrt2ftaddr(addr, NULL, NULL);
	sd_write8(padapter, ftaddr, val, &err);

_func_exit_;

	return err;
}

s32 sdio_write16(PADAPTER padapter, u32 addr, u16 val)
{
	u32 ftaddr;
//	u8 shift;
	s32 err;

_func_enter_;

	ftaddr = _cvrt2ftaddr(addr, NULL, NULL);
	val = cpu_to_le16(val);
	err = sd_cmd52_write(padapter, ftaddr, 2, (u8*)&val);

_func_exit_;

	return err;
}

s32 sdio_write32(PADAPTER padapter, u32 addr, u32 val)
{
	u8 deviceId;
	u16 offset;
	u32 ftaddr;
	u8 shift;
	s32 err;

_func_enter_;

	err = 0;

	ftaddr = _cvrt2ftaddr(addr, &deviceId, &offset);

	// 4 bytes alignment
	shift = ftaddr & 0x3;
#if 1
	if (shift == 0)
	{
		sd_write32(padapter, ftaddr, val, &err);
	}
	else
	{
		val = cpu_to_le32(val);
		err = sd_cmd52_write(padapter, ftaddr, 4, (u8*)&val);
	}
#else
	if (shift == 0) {	
		sd_write32(pintfhdl, ftaddr, val, &err);
	} else {
		u8 *ptmpbuf;

		ptmpbuf = (u8*)rtw_malloc(8);
		if (NULL == ptmpbuf) return (-1);

		ftaddr &= ~(u16)0x3;
		err = sd_read(pintfhdl, ftaddr, 8, ptmpbuf);
		if (err) {
			rtw_mfree(ptmpbuf, 8);
			return err;
		}
		val = cpu_to_le32(val);
		rtw_memcpy(ptmpbuf+shift, &val, 4);
		err = sd_write(pintfhdl, ftaddr, 8, ptmpbuf);
		
		rtw_mfree(ptmpbuf, 8);
	}
#endif	

_func_exit_;

	return err;
}

s32 sdio_writeN(PADAPTER padapter, u32 addr, u32 cnt, u8* pbuf)
{

	u8 deviceId;
	u16 offset;
	u32 ftaddr;
	u8 shift;
	s32 err;
_func_enter_;

	err = 0;

	ftaddr = _cvrt2ftaddr(addr, &deviceId, &offset);
	
#if 0//def CONFIG_POWER_SAVING
	if ((_TRUE == adapter_to_pwrctl(padapter)->bFwCurrentInPSMode))
	{
		err = sd_cmd52_write(padapter, ftaddr, cnt, pbuf);
		return err;
	}
#endif

	shift = ftaddr & 0x3;
	if (shift == 0) {
		err = sd_write(padapter, ftaddr, cnt, pbuf);
	} else {
		u8 *ptmpbuf;
		u32 n;

		ftaddr &= ~(u16)0x3;
		n = cnt + shift;
		ptmpbuf = rtw_malloc(n);
		if (NULL == ptmpbuf) return -1;
		err = sd_read(padapter, ftaddr, 4, ptmpbuf);
		if (err) {
			rtw_mfree(ptmpbuf, n);
			return err;
		}
		rtw_memcpy(ptmpbuf+shift, pbuf, cnt);
		err = sd_write(padapter, ftaddr, n, ptmpbuf);
		rtw_mfree(ptmpbuf, n);
	}

_func_exit_;

	return err;
}

void sdio_write_mem(PADAPTER padapter, u32 addr, u32 cnt, u8 *wmem)
{
_func_enter_;

	sdio_writeN(padapter, addr, cnt, wmem);

_func_exit_;
}

/*
 * Description:
 *	Write to TX FIFO
 *	Align write size block size,
 *	and make sure data could be written in one command.
 *
 * Parameters:
 *	pintfhdl	a pointer of intf_hdl
 *	addr		port ID
 *	cnt			size to write
 *	wmem		data pointer to write
 *
 * Return:
 *	_SUCCESS(1)		Success
 *	_FAIL(0)		Fail
 */
u32 sdio_write_port(
	PADAPTER padapter,
	u32 addr,
	u32 cnt,
	u8 *mem)
{

	s32 err;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
	cnt = _RND4(cnt);
//DBG_871X("%s: SDIO_REG_FREE_TXBD_NUM @ %d\n", __FUNCTION__, rtw_read16(padapter, SDIO_REG_FREE_TXBD_NUM));
_func_enter_;
	if(padapter->bStopTrx){
		DBG_871X(" %s (padapter->bStopTrx!!!)\n",__FUNCTION__);
		return _FAIL;
	}

	HalSdioGetCmdAddr8195ASdio(padapter, addr, cnt >> 2, &addr);
	
#if 1//def PLATFORM_ECOS
	if (cnt >  psdio->block_transfer_len)
		cnt = _RND(cnt,  psdio->block_transfer_len);
#else
	if (cnt > psdio->func->cur_blksize)
		cnt = _RND(cnt, psdio->func->cur_blksize);
#endif

	err = sd_write(padapter, addr, cnt, mem);	
_func_exit_;
	if (err)
	{
		DBG_871X("%s, error=%d\n", __func__, err);
		return _FAIL;
	}
	return _SUCCESS;
}

/*
 * Todo: align address to 4 bytes.
 */
s32 _sdio_local_read(
	PADAPTER padapter,
	u32		addr,
	u32		cnt,
	u8		*pbuf)
{
	s32 err;
	u8 *ptmpbuf;
	u32 n;

	HalSdioGetCmdAddr8195ASdio(padapter, SDIO_LOCAL_DEVICE_ID, addr, &addr);

#if 0//def CONFIG_POWER_SAVING
	if ((_TRUE == adapter_to_pwrctl(padapter)->bFwCurrentInPSMode))
	{
		err = _sd_cmd52_read(padapter, addr, cnt, pbuf);
		return err;
	}
#endif

       n = RND4(cnt);
	ptmpbuf = (u8 *)rtw_malloc(n);
	if(!ptmpbuf)
		return (-1);
	
	err = _sd_read(padapter, addr, n, ptmpbuf);
	
	if (!err)
		rtw_memcpy(pbuf, ptmpbuf, cnt);

	if(ptmpbuf)
		rtw_mfree(ptmpbuf, n);	

	return err;
}


/*
 * Todo: align address to 4 bytes.
 */
s32 sdio_local_read(
	PADAPTER	padapter,
	u32		addr,
	u32		cnt,
	u8		*pbuf)
{

	s32 err;
	u8 *ptmpbuf;
	u32 n;
	HalSdioGetCmdAddr8195ASdio(padapter, SDIO_LOCAL_DEVICE_ID, addr, &addr);

#if 0//def CONFIG_POWER_SAVING
	if ((_TRUE == adapter_to_pwrctl(padapter)->bFwCurrentInPSMode))
	{
		err = sd_cmd52_read(padapter, addr, cnt, pbuf);
		return err;
	}
#endif

        n = RND4(cnt);
	ptmpbuf = (u8*)rtw_malloc(n);
	if(!ptmpbuf)
		return (-1);

	err = sd_read(padapter, addr, n, ptmpbuf);
	if (!err)
		rtw_memcpy(pbuf, ptmpbuf, cnt);

	if(ptmpbuf)
		rtw_mfree(ptmpbuf, n);	

	return err;
}


/*
 * Todo: align address to 4 bytes.
 */
s32 _sdio_local_write(
	PADAPTER	padapter,
	u32		addr,
	u32		cnt,
	u8		*pbuf)
{

	s32 err;
	u8 *ptmpbuf;

	if(addr & 0x3)
		DBG_871X("%s, address must be 4 bytes alignment\n", __FUNCTION__);

	if(cnt  & 0x3)
		DBG_871X("%s, size must be the multiple of 4 \n", __FUNCTION__);
	
	HalSdioGetCmdAddr8195ASdio(padapter, SDIO_LOCAL_DEVICE_ID, addr, &addr);

#if 0//def CONFIG_POWER_SAVING
	if (_TRUE == adapter_to_pwrctl(padapter)->bFwCurrentInPSMode)
	{
		err = _sd_cmd52_write(padapter, addr, cnt, pbuf);
		return err;
	}
#endif

        ptmpbuf = (u8*)rtw_malloc(cnt);
	if(!ptmpbuf)
		return (-1);

	rtw_memcpy(ptmpbuf, pbuf, cnt);

	err = _sd_write(padapter, addr, cnt, ptmpbuf);
	
	if (ptmpbuf)
		rtw_mfree(ptmpbuf, cnt);

	return err;
}


/*
 * Todo: align address to 4 bytes.
 */
s32 sdio_local_write(
	PADAPTER padapter,
	u32		addr,
	u32		cnt,
	u8		*pbuf)
{
	s32 err;
	u8 *ptmpbuf;
	if(addr & 0x3)
		DBG_871X("%s, address must be 4 bytes alignment\n", __FUNCTION__);

	if(cnt  & 0x3)
		DBG_871X("%s, size must be the multiple of 4 \n", __FUNCTION__);
	
	HalSdioGetCmdAddr8195ASdio(padapter, SDIO_LOCAL_DEVICE_ID, addr, &addr);

#if 0//def CONFIG_POWER_SAVING
	if (_TRUE == adapter_to_pwrctl(padapter)->bFwCurrentInPSMode)
	{
		err = sd_cmd52_write(padapter, addr, cnt, pbuf);
		return err;
	}
#endif

        ptmpbuf = (u8*)rtw_malloc(cnt);
	if(!ptmpbuf)
		return (-1);

	rtw_memcpy(ptmpbuf, pbuf, cnt);

	err = sd_write(padapter, addr, cnt, ptmpbuf);
	
	if (ptmpbuf)
		rtw_mfree(ptmpbuf, cnt);

	return err;
}


u8 SdioLocalCmd52Read1Byte(PADAPTER padapter, u32 addr)
{

	u8 val = 0;
	
	HalSdioGetCmdAddr8195ASdio(padapter, SDIO_LOCAL_DEVICE_ID, addr, &addr);
	sd_cmd52_read(padapter, addr, 1, &val);

	return val;
}

u16 SdioLocalCmd52Read2Byte(PADAPTER padapter, u32 addr)
{

	u16 val = 0;

	HalSdioGetCmdAddr8195ASdio(padapter, SDIO_LOCAL_DEVICE_ID, addr, &addr);
	sd_cmd52_read(padapter, addr, 2, (u8*)&val);

	val = le16_to_cpu(val);

	return val;
}

u32 SdioLocalCmd52Read4Byte(PADAPTER padapter, u32 addr)
{
	u32 val = 0;

	HalSdioGetCmdAddr8195ASdio(padapter, SDIO_LOCAL_DEVICE_ID, addr, &addr);
	sd_cmd52_read(padapter, addr, 4, (u8*)&val);

	val = le32_to_cpu(val);

	return val;
}

u32 SdioLocalCmd53Read4Byte(PADAPTER padapter, u32 addr)
{
//	struct intf_hdl * pintfhdl;
//	u8 bMacPwrCtrlOn;
	u32 val=0;

//	pintfhdl=&padapter->iopriv.intf;
	HalSdioGetCmdAddr8195ASdio(padapter, SDIO_LOCAL_DEVICE_ID, addr, &addr);
#if 0//def CONFIG_POWER_SAVING
	if (_TRUE == adapter_to_pwrctl(padapter)->bFwCurrentInPSMode)
	{
		sd_cmd52_read(padapter, addr, 4, (u8*)&val);
		val = le32_to_cpu(val);
	}
	else
#endif
	val = sd_read32(padapter, addr, NULL);

	return val;
}

void SdioLocalCmd52Write1Byte(PADAPTER padapter, u32 addr, u8 v)
{
//	struct intf_hdl * pintfhdl;
	
//	pintfhdl=&padapter->iopriv.intf;
	HalSdioGetCmdAddr8195ASdio(padapter, SDIO_LOCAL_DEVICE_ID, addr, &addr);
	sd_cmd52_write(padapter, addr, 1, &v);
}

void SdioLocalCmd52Write2Byte(PADAPTER padapter, u32 addr, u16 v)
{
//	struct intf_hdl * pintfhdl;

//	pintfhdl=&padapter->iopriv.intf;
	HalSdioGetCmdAddr8195ASdio(padapter, SDIO_LOCAL_DEVICE_ID, addr, &addr);
	v = cpu_to_le16(v);
	sd_cmd52_write(padapter, addr, 2, (u8*)&v);
}

void SdioLocalCmd52Write4Byte(PADAPTER padapter, u32 addr, u32 v)
{
//	struct intf_hdl * pintfhdl;

//	pintfhdl=&padapter->iopriv.intf;
	HalSdioGetCmdAddr8195ASdio(padapter, SDIO_LOCAL_DEVICE_ID, addr, &addr);
	v = cpu_to_le32(v);
	sd_cmd52_write(padapter, addr, 4, (u8*)&v);
}

#ifdef PLATFORM_ECOS
extern void rtw_triggered_wlan_rx_tasklet(_adapter *priv);
#endif
static void sd_rxhandler(PADAPTER padapter, struct recv_buf *precvbuf)
{
	struct recv_priv *precvpriv = &padapter->recvpriv;
	_queue *ppending_queue;

	ppending_queue = &precvpriv->recv_buf_pending_queue;

	//3 1. enqueue recvbuf
	rtw_enqueue_recvbuf(precvbuf, ppending_queue);	
	//3 2. schedule tasklet
#ifdef PLATFORM_LINUX
	tasklet_schedule(&precvpriv->recv_tasklet);
#endif
#ifdef PLATFORM_ECOS
	// Considering TP, do every one without checking if recv_buf_pending_queue.qlen == 1
	rtw_triggered_wlan_rx_tasklet(padapter);
#endif
}

#ifdef CONFIG_SDIO_RX_COPY
static struct recv_buf* sd_recv_rxfifo(PADAPTER padapter, u32 size)
{
	u32 readsize, ret;
	u8 *preadbuf;
	struct recv_priv *precvpriv;
	struct recv_buf	*precvbuf;

_func_enter_;
	readsize = size;

	//3 1. alloc recvbuf
	precvpriv = &padapter->recvpriv;
	precvbuf = rtw_dequeue_recvbuf(&precvpriv->free_recv_buf_queue);
	if (precvbuf == NULL) {
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("%s: alloc recvbuf FAIL!\n", __FUNCTION__));
		DBG_871X("%s: alloc recvbuf FAIL!\n", __FUNCTION__);
		return NULL;
	}

	//3 2. alloc skb
#ifdef PLATFORM_ECOS
	// can't allocate skb with size larger than 2K, use malloc for precvbuf->pbuf instead
#else
	if (precvbuf->pskb == NULL) {
		SIZE_PTR tmpaddr=0;
		SIZE_PTR alignment=0;

		DBG_871X("%s: alloc_skb for rx buffer\n", __FUNCTION__);

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
			DBG_871X("%s: alloc_skb fail! read=%d\n", __FUNCTION__, readsize);
			return NULL;
		}
	}
#endif
	//3 3. read data from rxfifo
	preadbuf = precvbuf->pdata;

#if 0//def CONFIG_POWER_SAVING
#ifdef CONFIG_PS_DYNAMIC_CHK
	rtw_ps_deny(padapter, PS_DENY_DRV_RXDATA);
	rtw_set_ps_mode(padapter, PS_MODE_ACTIVE);
#endif
#endif

	//DBG_871X("%s: read packet @ %d bytes\n", __FUNCTION__, readsize);
	ret = sdio_read_port(padapter, WLAN_RX_FIFO_DEVICE_ID, readsize, preadbuf);

#if 0//def CONFIG_POWER_SAVING
#ifdef CONFIG_PS_DYNAMIC_CHK
	rtw_ps_deny_cancel(padapter, PS_DENY_DRV_RXDATA);
	rtw_set_ps_mode(padapter, PS_MODE_SLEEP);
#endif
#endif
	if (ret == _FAIL) {
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("%s: read port FAIL!\n", __FUNCTION__));
		return NULL;
	}
	

	//3 4. init recvbuf
	precvbuf->len = readsize;
_func_exit_;
	return precvbuf;
}
#else
static struct recv_buf* sd_recv_rxfifo(PADAPTER padapter, u32 size)
{
	u32 readsize, allocsize, ret;
	u8 *preadbuf;
	_pkt *ppkt;
	struct recv_priv *precvpriv;
	struct recv_buf	*precvbuf;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
_func_enter_;
	readsize = size;

	//3 1. alloc skb
	// align to block size
#ifdef PLATFORM_ECOS
	allocsize = _RND(readsize, psdio->block_transfer_len);
#else
	allocsize = _RND(readsize, psdio->func->cur_blksize);
#endif
	ppkt = rtw_skb_alloc(allocsize);

	if (ppkt == NULL) {
		return NULL;
	}

	//3 2. read data from rxfifo
	preadbuf = skb_put(ppkt, readsize);
#if 0//def CONFIG_POWER_SAVING
#ifdef CONFIG_PS_DYNAMIC_CHK
	rtw_ps_deny(padapter, PS_DENY_DRV_RXDATA);
#endif
	rtw_set_ps_mode(padapter, PS_MODE_ACTIVE);
#endif

	//DBG_871X("%s: read packet @ %d bytes\n", __FUNCTION__, readsize);
	ret = sdio_read_port(padapter, WLAN_RX_FIFO_DEVICE_ID, readsize, preadbuf);

#if 0//def CONFIG_POWER_SAVING
#ifdef CONFIG_PS_DYNAMIC_CHK
	rtw_ps_deny_cancel(padapter, PS_DENY_DRV_RXDATA);
#endif
	rtw_set_ps_mode(padapter, PS_MODE_SLEEP);
#endif
	if (ret == _FAIL) {
		rtw_skb_free(ppkt);
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("%s: read port FAIL!\n", __FUNCTION__));
		return NULL;
	}

	//3 3. alloc recvbuf
	precvpriv = &padapter->recvpriv;
	precvbuf = rtw_dequeue_recvbuf(&precvpriv->free_recv_buf_queue);
	if (precvbuf == NULL) {
		rtw_skb_free(ppkt);
		RT_TRACE(_module_hci_ops_os_c_, _drv_err_, ("%s: alloc recvbuf FAIL!\n", __FUNCTION__));
		return NULL;
	}

	//3 4. init recvbuf
	precvbuf->pskb = ppkt;
	precvbuf->len = ppkt->len;
	precvbuf->phead = ppkt->head;
	precvbuf->pdata = ppkt->data;
	precvbuf->ptail = skb_tail_pointer(precvbuf->pskb);
	precvbuf->pend = skb_end_pointer(precvbuf->pskb);
_func_exit_;	
	return precvbuf;
}
#endif

#ifdef CONFIG_LOOPBACK_TEST
static void rtl8195as_loopback_recv(PADAPTER padapter, u32 size);
#endif
void sd_int_dpc(PADAPTER padapter)
{
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
	struct recv_buf *precvbuf;
	
#ifdef CONFIG_SDIO_TX_ENABLE_AVAL_INT
	if (psdio->sdio_hisr & SDIO_HISR_AVAL_INT)
	{
		
		u32	freepage;

		//psdio->sdio_hisr ^= SDIO_HISR_AVAL_INT;

		//read SDIO_REG_FREE_TXBD_NUM to clear interrupt SDIO_HISR_AVAL_INT
//#ifdef PLATFORM_ECOS //need sdio claim host
		sdio_local_read(padapter, SDIO_REG_FREE_TXBD_NUM, 4, (u8 *)&freepage);
//#else
//		_sdio_local_read(padapter, SDIO_REG_FREE_TXBD_NUM, 4, (u8 *)&freepage);
//#endif
		//DBG_871X("TXBD available!SDIO_REG_FREE_TXBD_NUM@0x%08x\n", freepage);

		rtw_up_sema(&(padapter->xmitpriv.xmit_sema));
	}
#endif
	if (psdio->sdio_hisr & SDIO_HISR_RX_REQUEST)
	{

		psdio->sdio_hisr ^= SDIO_HISR_RX_REQUEST;

		do {
			//Sometimes rx length will be zero. driver need to use cmd53 read again.
			if(psdio->SdioRxFIFOSize == 0)
			{
				u8 data[4];
				u8 rx_len_rdy;			

				//validate RX_LEN_RDY before reading RX0_REQ_LEN
				rx_len_rdy = sdio_read8(padapter, SDIO_REG_RX0_REQ_LEN+3);
				if(rx_len_rdy & BIT7){
//#ifdef PLATFORM_ECOS //need sdio claim host
					sdio_local_read(padapter, SDIO_REG_RX0_REQ_LEN, 4, data);
//#else
//					_sdio_local_read(padapter, SDIO_REG_RX0_REQ_LEN, 4, data);
//#endif
					psdio->SdioRxFIFOSize = le16_to_cpu(*(u16*)data);
				}
			}
			if(psdio->SdioRxFIFOSize)
			{
#ifdef CONFIG_LOOPBACK_TEST
				rtl8195as_loopback_recv(padapter, psdio->SdioRxFIFOSize);
				precvbuf = NULL;
#else
				precvbuf = sd_recv_rxfifo(padapter, psdio->SdioRxFIFOSize);
#endif
				psdio->SdioRxFIFOSize = 0;
				if (precvbuf)
				{
					sd_rxhandler(padapter, precvbuf);
				}
				else
					break;
			}
			else
				break;

		} while (1);

	}
#if 0
	if (psdio->sdio_hisr & SDIO_HISR_C2H_MSG_INT)
	{

	}
	if (psdio->sdio_hisr & SDIO_HISR_CPU_NOT_RDY) 
	{
		//when host send CMD53 to SDIO OFF, and CPU_RDY_IND=0,
		//this interrupt will be triggered, and host should stop sending
		u8 ind;
		ind = sdio_read8(padapter, SDIO_REG_CPU_IND);
		DBG_871X("Stop packet transmittion!SDIO_REG_CPU_IND@0x%02x\n", ind);
		padapter->bStopTrx = _TRUE;
	}
#endif
	if (psdio->sdio_hisr & SDIO_HISR_CPWM1)
	{
		//Register HCPWM
		//BIT1: WLAN_TRX (0: WLAN OFF, 1: WLAN ON)
		//when WLAN OFF, host should stop sending packet to FW
		u8 cpwm;
		cpwm = sdio_read8(padapter, SDIO_REG_HCPWM);
		if(cpwm&SDIO_HCPWM_WLAN_TRX){
			DBG_871X("WIFI_ON!HCPWM@(0x%02x)\n", cpwm);
			//need to clean tx queue after wifi on
#ifdef CONFIG_TX_AGGREGATION
			rtw_free_xmitframe_queue(&padapter->xmitpriv, &padapter->xmitpriv.xmitframe_pending_queue);
#endif
			rtw_free_xmitbuf_queue(padapter, &padapter->xmitpriv.xmitbuf_pending_queue);
			padapter->bStopTrx = _FALSE;
		}
		else{
			DBG_871X("WIFI_OFF!HCPWM@(0x%02x)\n", cpwm);
			padapter->bStopTrx = _TRUE;
			if(padapter->fw_status){
				padapter->fw_status = 0;
				rtw_os_indicate_disconnect(padapter->pnetdev);
			}
		}	
	}
}

void sd_int_hdl(PADAPTER padapter)
{
	u8 data[4];
	u8 rx_len_rdy;
	u8 rx_len[4];

	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
	
	if ((padapter->bDriverStopped == _TRUE) || (padapter->bSurpriseRemoved == _TRUE))
		return;

#ifdef DBG_RX_AGG
	DBG_871X("%s: IRQ arrived!\n", __FUNCTION__);
#endif

	//read SDIO_HISR(32bits)
//#ifdef PLATFORM_ECOS //need sdio claim host
	sdio_local_read(padapter, SDIO_REG_HISR, 4, data); 
//#else
//	_sdio_local_read(padapter, SDIO_REG_HISR, 4, data); 
//#endif
	psdio->sdio_hisr = le32_to_cpu(*(u32*)data);

#if 1
	//rx length is valide only when RX_REQ_LEN_RDY(BIT31) is 1
	rx_len_rdy = sdio_read8(padapter, SDIO_REG_RX0_REQ_LEN+3);
	if(rx_len_rdy & BIT7){
//#ifdef PLATFORM_ECOS //need sdio claim host
		sdio_local_read(padapter, SDIO_REG_RX0_REQ_LEN, 4, rx_len); 
//#else
//		_sdio_local_read(padapter, SDIO_REG_RX0_REQ_LEN, 4, rx_len); 
//#endif
		psdio->SdioRxFIFOSize = le16_to_cpu(*(u16*)rx_len);
	}
#endif

	if (psdio->sdio_hisr & psdio->sdio_himr)
	{
		u32 v32;

		psdio->sdio_hisr &= psdio->sdio_himr;
#ifdef PLATFORM_ECOS
		// Reduce the frequency of RX Request Interrupt during RX handling
		DisableInterrupt8195ASdio(padapter);//ecos
#endif
		// clear HISR
		v32 = psdio->sdio_hisr & MASK_SDIO_HISR_CLEAR;
		if (v32) {
			v32 = cpu_to_le32(v32);
//#ifdef PLATFORM_ECOS //need sdio claim host
			sdio_local_write(padapter, SDIO_REG_HISR, 4, (u8*)&v32);
//#else
//			_sdio_local_write(padapter, SDIO_REG_HISR, 4, (u8*)&v32);
//#endif
		}

		sd_int_dpc(padapter);
		
#ifdef PLATFORM_ECOS
		EnableInterrupt8195ASdio(padapter);//ecos
#endif
	} 
	else 
	{
		//some irq arrive during the int_hdl
		RT_TRACE(_module_hci_ops_c_, _drv_err_,
				("%s: HISR(0x%08x) and HIMR(0x%08x) not match!\n",
				__FUNCTION__, psdio->sdio_hisr, psdio->sdio_himr));
	}	
}

u8 HalGetTxBufUnitSize8195ASdio(PADAPTER padapter)
{
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
	u8 TxUnitCnt = 0;
	TxUnitCnt = SdioLocalCmd52Read1Byte(padapter, SDIO_REG_TXBUF_UNIT_SZ);
	if(!TxUnitCnt)
		return _FAIL;
	psdio->SdioTxMaxSZ = TxUnitCnt * 64 - 32; //num * unit_sz(64 bytes) -32(reserved for safety)
	DBG_871X("%s: TX_UNIT_BUF_MAX_SIZE @ %d bytes\n", __FUNCTION__, psdio->SdioTxMaxSZ);
	return _SUCCESS;
}
//
//	Description:
//		Query SDIO Local register to query current the number of Free TxPacketBuffer page.
//
u8 HalQueryTxBufferStatus8195ASdio(PADAPTER padapter)
{
	u32 i;
	u8 data[2];
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;

	//read offset 0x21->0x20 consequently, suggested by designer
	//because LSB changes faster
	for (i=0; i<2; i++){
		data[1-i] = SdioLocalCmd52Read1Byte(padapter, SDIO_REG_FREE_TXBD_NUM+1-i);
	}

	psdio->SdioTxBDFreeNum = le16_to_cpu(*(u16*)data);
#ifdef DBG_TX_BD_FREENUM
	DBG_871X("%s: Free page for TXBD(0x%x)\n", __FUNCTION__, psdio->SdioTxBDFreeNum);
#endif
	return _TRUE;
}

//
//	Description:
//		Initialize SDIO Host Interrupt Mask configuration variables for future use.
//
//	Assumption:
//		Using SDIO Local register ONLY for configuration.
//
//	Created by Roger, 2011.02.11.
//
void InitInterrupt8195ASdio(PADAPTER padapter)
{
	struct dvobj_priv *psdpriv = padapter->dvobj;
	PSDIO_DATA psdio = &psdpriv->intf_data;
	psdio->sdio_himr = (u32)(	\
								SDIO_HIMR_RX_REQUEST_MSK | 
#ifdef CONFIG_SDIO_TX_ENABLE_AVAL_INT
								SDIO_HIMR_AVAL_MSK	|
#endif
//								SDIO_HIMR_CPU_NOT_RDY_MSK |
								SDIO_HIMR_CPWM1_MSK |
								0);
}

//
//	Description:
//		Enalbe SDIO Host Interrupt Mask configuration on SDIO local domain.
//
//	Assumption:
//		1. Using SDIO Local register ONLY for configuration.
//		2. PASSIVE LEVEL
//
//	Created by Roger, 2011.02.11.
//
void EnableInterrupt8195ASdio(PADAPTER padapter)
{
	PSDIO_DATA psdio = &padapter->dvobj->intf_data;
	u32 himr;

	himr = cpu_to_le32(psdio->sdio_himr);
	sdio_local_write(padapter, SDIO_REG_HIMR, 4, (u8*)&himr);

	//
	// <Roger_Notes> There are some C2H CMDs have been sent before system interrupt is enabled, e.g., C2H, CPWM.
	// So we need to clear all C2H events that FW has notified, otherwise FW won't schedule any commands anymore.
	// 2011.10.19.
	//
	//rtw_write8(padapter, REG_C2HEVT_CLEAR, C2H_EVT_HOST_CLOSE);
}

//
//	Description:
//		Disable SDIO Host IMR configuration to mask unnecessary interrupt service.
//
//	Assumption:
//		Using SDIO Local register ONLY for configuration.
//
//	Created by Roger, 2011.02.11.
//
void DisableInterrupt8195ASdio(PADAPTER padapter)
{
	u32 himr;

	himr = cpu_to_le32(SDIO_HIMR_DISABLED);
	sdio_local_write(padapter, SDIO_REG_HIMR, 4, (u8*)&himr);

}

//for SDIO IO debug
static u8 sd_recv_one_pkt(PADAPTER padapter, u32 size)
{
	struct recv_buf *precvbuf;
	struct dvobj_priv *psddev;
	PSDIO_DATA psdio_data;
	struct sdio_func *func;

	u8 res = -1;

	//DBG_871X("+%s: size: %d+\n", __func__, size);
_func_enter_;
	if (padapter == NULL) {
		DBG_871X(KERN_ERR "%s: padapter is NULL!\n", __func__);
		return -1;
	}
	
	psddev = padapter->dvobj;
	psdio_data = &psddev->intf_data;
	func = psdio_data->func;
	if(psdio_data->SdioRxFIFOSize == 0)
	{
		u8 data[4];
		u8 rx_len_rdy;			

		//validate RX_LEN_RDY before reading RX0_REQ_LEN
		rx_len_rdy = sdio_read8(padapter, SDIO_REG_RX0_REQ_LEN+3);
		if(rx_len_rdy & BIT7){
			sdio_local_read(padapter, SDIO_REG_RX0_REQ_LEN, 4, data);
			psdio_data->SdioRxFIFOSize = le16_to_cpu(*(u16*)data);
		}
		DBG_871X("+%s: psdio_data->SdioRxFIFOSize: %d+\n", __func__, psdio_data->SdioRxFIFOSize);
	}
	if(psdio_data->SdioRxFIFOSize){
		precvbuf = sd_recv_rxfifo(padapter, psdio_data->SdioRxFIFOSize);
		psdio_data->SdioRxFIFOSize = 0;
		if (precvbuf) {
			#if 1
			printk("Completed Recv One Pkt.\n");
			#else
			sd_rxhandler(padapter, precvbuf);
			#endif
			res = 0;
		}else{
			res = -1;
			printk("Failed Recv One Pkt.\n");
		}
	}
	DBG_871X("-%s-\n", __func__);
_func_exit_;
	return res;
}
int rtw_io_rw_direct_host(PADAPTER padapter, u8 *in, u8 sz, u8 *out){
	int ret = 0;
	const char delim[] = " ";
	int write = 0; //0 for read, 1 for write
	u32 count = 0;
	int buffer[5] = {0};
	char *str, *ptr = NULL;
	int temp;
	u32 fn, addr, cnt, val = 0;
	u8 val8 = 0;
	DBG_871X("%s: input command: %s\n", __FUNCTION__, in);

	ptr = (char *)(in + 5);
	/* Fetch args */
	count = 0;
	do {
		str = (char*)strsep(&ptr, delim);
		if (NULL == str) break;
		sscanf(str, "%i", &temp);
		buffer[count++] = temp;
	} while (1);
	if((count != 5)&&(count != 4))
	{
		DBG_871X("%s: input arg wrong\n", __FUNCTION__);
		return -1;
	}
	fn = (u32)buffer[0];
	write = (u32)buffer[1];
	addr = (u32)buffer[2];
	cnt = (u32)buffer[3];
	if((cnt > 4)||((fn == 0)&&(cnt > 1)))
	{
		DBG_871X("%s: read/write 4 bytes maximum\n", __FUNCTION__);
		return -1;
	}
	if(addr == 0xe000){
		ret = sd_recv_one_pkt(padapter, 0);
		goto exit;
	}
	if(write){
		val = (u32)buffer[4];
		val8 = (u8)buffer[4];
		DBG_871X("%s: fn: %d write 0x%04x (%d bytes) to register @ 0x%04x\n", __FUNCTION__, fn, val, cnt, addr);
		if(fn != 0)
			ret = sd_cmd52_write(padapter, addr, cnt, (u8 *)&val);
		else
			sd_f0_write8(padapter, addr, val8, &ret);
	}
	else{
		DBG_871X("%s: fn: %d read %d bytes from register @ 0x%04x\n", __FUNCTION__, fn, cnt, addr);
		if(fn != 0)
			ret = sd_cmd52_read(padapter, addr, cnt, (u8 *)&val);
		else{
			val8 = sd_f0_read8(padapter, addr, &ret);
			val = (u32)val8;
		}
		DBG_871X("%s: fn: %d register(offset: 0x%04x): 0x%08x\n", __FUNCTION__, fn, addr, val);
	}
	if(out)
		rtw_memcpy(out, (u8 *)&val, 4);
exit:
	return ret;
}

#ifdef CONFIG_POWER_SAVING
u32 rtl8195a_rpwm_notify(_adapter *padapter, u8 rpwm_event)
{
	u16 rpwm2;
	u16 cpwm2_old, cpwm2;
	u32 start_time;
	u32 ret = _SUCCESS;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);

#define RPWM_ACK_TIMEOUT_MS 200

_func_enter_;

	pwrpriv->ps_processing = _TRUE;
	
	rpwm2 = rtw_read16(padapter, SDIO_REG_HRPWM2);
	
#ifdef DBG_POWER_SAVING
	DBG_871X("%s==> rpwm_event=%d rpwm2=0x%x\n", __FUNCTION__, rpwm_event, rpwm2);
#endif

	rpwm2 &= RPWM2_TOGGLE_BIT;  // get the toggle bit
	switch (rpwm_event) {
		case RPWM2_PWR_ACT:
			rpwm2 |= (RPWM2_ACT_BIT|RPWM2_ACK_BIT);
			break;
		    
		case RPWM2_PWR_DSTANDBY:
			rpwm2 |= (RPWM2_SLEEP_BIT|RPWM2_DSTANDBY_BIT);
			// TODO: setup the wake up event
			break;
		    
		case RPWM2_PWR_PG:
			rpwm2 |= (RPWM2_PG_BIT|RPWM2_NBOOT_BIT);
			break;

		case RPWM2_PWR_PG_FB:
			rpwm2 |= (RPWM2_PG_BIT|RPWM2_FBOOT_BIT);
			break;

		case RPWM2_PWR_CG:
			rpwm2 |= (RPWM2_CG_BIT|RPWM2_ACK_BIT);
			break;
			
		case RPWM2_PWR_WAKEUP:
			rpwm2 |= RPWM2_ACT_BIT;
			break;

		default:
			// unknown RPWM event
			pwrpriv->ps_processing = _FALSE;
			return _FAIL;
	}

	rpwm2 = rpwm2^RPWM2_TOGGLE_BIT;   // inverse toggle bit

#ifdef DBG_POWER_SAVING
	DBG_871X("write rpwm2=0x%x\n", rpwm2);
#endif

 	cpwm2_old = rtw_read16(padapter, SDIO_REG_HCPWM2);
	pwrpriv->rpwm2 = rpwm2;
	pwrpriv->cpwm2 = cpwm2_old;
	
	rtw_write16(padapter, SDIO_REG_HRPWM2, rpwm2);
	
	start_time = rtw_get_current_time();
	if (rpwm2 & RPWM2_ACK_BIT) {
		while(1){
			if(padapter->bSurpriseRemoved || padapter->bDriverStopped){
				break;
			}	
			cpwm2 = rtw_read16(padapter, SDIO_REG_HCPWM2);
			if ((cpwm2 & CPWM2_TOGGLE_BIT) != (cpwm2_old & CPWM2_TOGGLE_BIT)) {
				pwrpriv->cpwm2 = cpwm2;
				break;
			}
			if (rtw_get_passing_time_ms(start_time) > RPWM_ACK_TIMEOUT_MS)
			{
//#ifdef DBG_POWER_SAVING
				DBG_871X("Wait Ack from CPWM2 Timeout\n");
//#endif
				ret = _FAIL;
				break;
			}
			rtw_msleep_os(1);
		}
	}
	pwrpriv->ps_processing = _FALSE;
_func_exit_;
    return ret;
}

#ifdef CONFIG_WOWLAN
static char *rtw_wowlan_cmd_hdr = "ATWV=[?]"; //? should be put cmd ID
static int rtw_wowlan_cmd_index = 6; //where the '?' locates
static int rtw_wowlan_cmd_hdr_sz = 8; // should be strlen(wowlan_cmd_hdr)
static void rtw_fill_wowlan_cmd_hdr(char *data, char cmd_id){
	rtw_memcpy(data, rtw_wowlan_cmd_hdr, strlen(rtw_wowlan_cmd_hdr));
	//sprintf(&data[wowlan_cmd_index], "%d", cmd_id);
	data[rtw_wowlan_cmd_index] = cmd_id;
}
u32	rtl8195a_send_wowlan_cmd(_adapter *padapter, u8 id, u8 *data, u16 len){

	PSDIO_DATA psdio = &padapter->dvobj->intf_data;
	s32 txbd_needed = 0;
	u8 *tx_buf, *tx_buf_alloc = NULL;
	s32 tx_buf_len = SIZE_TX_DESC_8195a + rtw_wowlan_cmd_hdr_sz + len;
	PTXDESC_8195A ptxdesc;

#ifdef CONFIG_SDIO_TX_OVF_CTRL	
	u32	polling_num = 0;
#endif //CONFIG_SDIO_TX_OVF_CTRL

	txbd_needed = tx_buf_len/(psdio->SdioTxMaxSZ) + 1;

#ifdef CONFIG_SDIO_TX_OVF_CTRL	
query_free_page:
	// check if hardware tx fifo page is enough
	if( _FALSE == rtw_sdio_query_tx_free_bd(padapter, txbd_needed))
	{
		polling_num++;
		if ((polling_num % 60) == 0) {//or 80
			//DBG_871X("%s: FIFO starvation!(%d) len=%d agg=%d page=(R)%d(A)%d\n",
			//	__func__, n, pxmitbuf->len, pxmitbuf->agg_num, pframe->pg_num, freePage[PageIdx] + freePage[PUBLIC_QUEUE_IDX]);
			rtw_msleep_os(1);
		}

		// Total number of TXBD is NOT available, so update current TXBD status
		HalQueryTxBufferStatus8195ASdio(padapter);
		goto query_free_page;
	}
#endif

	if ((padapter->bSurpriseRemoved == _TRUE) 
		|| (padapter->bDriverStopped == _TRUE)
	){
		RT_TRACE(_module_hal_xmit_c_, _drv_notice_,
			 ("%s: bSurpriseRemoved(wirte port)\n", __FUNCTION__));
		return _FALSE;
	}

	tx_buf_alloc= rtw_zmalloc(tx_buf_len + 4);
	if(tx_buf_alloc == NULL){
		DBG_871X("%s: malloc failed for sending pattern\n", __FUNCTION__);
		return _FALSE;
	}
	tx_buf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(tx_buf_alloc), 4);
	ptxdesc = (PTXDESC_8195A)tx_buf;
	ptxdesc->offset = SIZE_TX_DESC_8195a;
	ptxdesc->bus_agg_num = 1;
	ptxdesc->txpktsize = len + rtw_wowlan_cmd_hdr_sz;
	ptxdesc->type = TX_H2C_CMD;

	rtw_fill_wowlan_cmd_hdr(tx_buf + SIZE_TX_DESC_8195a, id);
	rtw_memcpy(tx_buf + SIZE_TX_DESC_8195a + rtw_wowlan_cmd_hdr_sz, data, len);
	
	if((ptxdesc->offset>SIZE_TX_DESC_8195a)||((ptxdesc->offset+ptxdesc->txpktsize)>psdio->SdioTxMaxSZ)||((u32)(tx_buf)%4))
		DBG_871X("%s: PKT SIZE ERROR OFFSET(%d) PKTSIZE(%d)\n", __FUNCTION__, ptxdesc->offset, ptxdesc->txpktsize);

#ifdef CONFIG_POWER_SAVING
	rtw_set_ps_mode(padapter, PS_MODE_ACTIVE);
#endif

	rtw_write_port(padapter, WLAN_TX_FIFO_DEVICE_ID, tx_buf_len, tx_buf);

#ifdef CONFIG_POWER_SAVING
	rtw_set_ps_mode(padapter, PS_MODE_SLEEP);
#endif
	
#ifdef CONFIG_SDIO_TX_OVF_CTRL
	rtw_sdio_update_tx_free_bd(padapter, txbd_needed);
#endif

	if(tx_buf_alloc)
		rtw_mfree(tx_buf_alloc, tx_buf_len + 4);

	return _TRUE;

}
#endif //#ifdef CONFIG_WOWLAN
#endif //CONFIG_POWER_SAVING

#ifdef CONFIG_LOOPBACK_TEST
static void rtl8195as_loopback_recv(PADAPTER padapter, u32 size){
	u32 readsize, allocsize, ret;
	static u32 loopback_recv_cnt = 0;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
	u8 *ppkt;
	readsize = size;
	allocsize = _RND(readsize, psdio->block_transfer_len);
	ppkt = rtw_malloc(allocsize);
	if (ppkt == NULL) {
		goto exit;
	}
	ret = sdio_read_port(padapter, WLAN_RX_FIFO_DEVICE_ID, readsize, ppkt);
	if (ret == _FAIL) {
		DBG_871X("%s: read port FAIL!\n", __FUNCTION__);
		goto exit;
	}
	DBG_871X("Loopback recv successed [%d]!\n", ++loopback_recv_cnt);
exit:
	if(ppkt)
		rtw_mfree(ppkt, allocsize);
}

s32 rtl8195as_loopback_send(PADAPTER padapter, u8 *pbuf, u16 sz){
	u32	polling_num = 0;
	PTXDESC_8195A ptxdesc = (PTXDESC_8195A)pbuf;
	PSDIO_DATA psdio;
	static u32 loopback_send_cnt = 0;
#ifdef CONFIG_SDIO_TX_OVF_CTRL	
	//Query SDIO TX FIFO to check if it's possible to send this packet
query_free_page:
	// check if hardware tx fifo page is enough
	if( _FALSE == rtw_sdio_query_tx_free_bd(padapter, ptxdesc->bus_agg_num))
	{
		polling_num++;
		if ((polling_num % 60) == 0) {//or 80
			//DBG_871X("%s: FIFO starvation!(%d) len=%d agg=%d\n",
			//	__func__, n, sz, ptxdesc->bus_agg_num);
			rtw_msleep_os(1);
		}

		// Total number of TXBD is NOT available, so update current TXBD status
		HalQueryTxBufferStatus8195ASdio(padapter);
		goto query_free_page;
	}
#endif
	if ((padapter->bSurpriseRemoved == _TRUE) 
		|| (padapter->bDriverStopped == _TRUE)
	){
		RT_TRACE(_module_hal_xmit_c_, _drv_notice_,
			 ("%s: bSurpriseRemoved(wirte port)\n", __FUNCTION__));
		return _FAIL;
	}
	

	psdio= &padapter->dvobj->intf_data;

	//Check TX Descriptor information, fault TX descriptor can driver device crash
	if((ptxdesc->offset>SIZE_TX_DESC_8195a)||((ptxdesc->offset+ptxdesc->txpktsize)>psdio->SdioTxMaxSZ)||((u32)(pbuf)%4))
		DBG_871X("%s: PKT SIZE ERROR OFFSET(%d) PKTSIZE(%d)\n", __FUNCTION__, ptxdesc->offset, ptxdesc->txpktsize);

	rtw_write_port(padapter, WLAN_TX_FIFO_DEVICE_ID, sz, pbuf);
	DBG_871X("Loopback send successed [%d]!\n", ++loopback_send_cnt);

#ifdef CONFIG_SDIO_TX_OVF_CTRL
	rtw_sdio_update_tx_free_bd(padapter, ptxdesc->bus_agg_num);
#endif

	if(pbuf)
		rtw_mfree(pbuf, sz);
	
	return _SUCCESS;
}
#endif //#ifdef CONFIG_LOOPBACK_TEST