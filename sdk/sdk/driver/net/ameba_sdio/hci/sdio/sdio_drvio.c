#include <linux/current.h> // Irene Lin
#include <linux/mmc/errorno.h> // Irene Lin
#include <drv_types.h>
#include <rtw_debug.h>
#include <sdio_ops.h>

u8 rtw_sdio_query_tx_free_bd(_adapter *padapter, u16 RequiredBDNum)
{
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
#ifdef DBG_TX_BD_FREENUM
	DBG_871X("%s: SdioTxBDFreeNum = %d\n", __FUNCTION__, psdio->SdioTxBDFreeNum);
#endif
	if (psdio->SdioTxBDFreeNum >= (RequiredBDNum))
		return _TRUE;
	else
		return _FALSE;
}

void rtw_sdio_update_tx_free_bd(_adapter *padapter, u16 RequiredBDNum)
{
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;

	if(psdio->SdioTxBDFreeNum > 0)
		psdio->SdioTxBDFreeNum -= RequiredBDNum;
#ifdef DBG_TX_BD_FREENUM
	DBG_871X("%s: SdioTxBDFreeNum = %d\n", __FUNCTION__, psdio->SdioTxBDFreeNum);
#endif
}


static bool rtw_sdio_claim_host_needed(struct sdio_func *func)
{
#ifdef PLATFORM_LINUX
	struct dvobj_priv *dvobj = rtw_sdio_get_drvdata(func);
	PSDIO_DATA psdio= &dvobj->intf_data;
	if (psdio->sys_sdio_irq_thd && (psdio->sys_sdio_irq_thd == current))
		return _FALSE;
#endif
	return _TRUE;
}

void rtw_sdio_set_irq_thd(struct dvobj_priv *dvobj, _thread_hdl_ thd_hdl)
{
	PSDIO_DATA sdio_data = &dvobj->intf_data;
	sdio_data->sys_sdio_irq_thd = thd_hdl;
}

/*
 * Return:
 *	0		Success
 *	others	Fail
 */
s32 _sd_cmd52_read(PADAPTER padapter, u32 addr, u32 cnt, u8 *pdata)
{
//	PADAPTER padapter;
//	struct dvobj_priv *psdiodev;
//	PSDIO_DATA psdio;

	int err=0;
	u32 i;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
	struct sdio_func *pfunc;
_func_enter_;
	if(padapter->bSurpriseRemoved){
		//DBG_871X(" %s (padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n",__FUNCTION__);
		return err;
	}
	pfunc = psdio->func;
//	DBG_871X("%s(): block size is %d\n", __func__, pfunc->cur_blksize);
	for (i = 0; i < cnt; i++) {
		pdata[i] = rtw_sdio_bus_ops.readb(pfunc, addr+i, &err);
		if (err) {
			//DBG_871X("%s(): sdio_readb failed!\n", __func__);
			break;
		}
	}

	if (err)
	{
		int j;

		//DBG_871X("%s: (%d) addr=0x%05x\n", __func__, err, addr);

		err = 0;
		for(j=0; j<SD_IO_TRY_CNT; j++)
		{
			for (i = 0; i < cnt; i++) {
				pdata[i] = rtw_sdio_bus_ops.readb(pfunc, addr+i, &err);
				if (err) {
					//DBG_871X("%s(): sdio_readb failed!\n", __func__);
					break;
				}
			}
			
			if (err == 0){
				rtw_reset_continual_io_error(padapter->dvobj);
				break;
			}else{				
				//DBG_871X("%s: (%d) addr=0x%05x, try_cnt=%d\n", __func__, err, addr, j);
				if(( -ESHUTDOWN == err ) || ( -ENODEV == err)){			
					padapter->bSurpriseRemoved = _TRUE;
				}

				if(rtw_inc_and_chk_continual_io_error(padapter->dvobj) == _TRUE ){
					padapter->bSurpriseRemoved = _TRUE;
					rtw_show_stack(NULL);
					break;
				}
			}
		}

		if (j==SD_IO_TRY_CNT)
			DBG_871X("%s: FAIL!(%d) addr=0x%05x, try_cnt=%d\n", __func__, err, addr, j);
		//else
			//DBG_871X("%s: (%d) addr=0x%05x, try_cnt=%d\n", __func__, err, addr, j);
	}

_func_exit_;

	return err;
}

/*
 * Return:
 *	0		Success
 *	others	Fail
 */
s32 sd_cmd52_read(PADAPTER padapter, u32 addr, u32 cnt, u8 *pdata)
{

	int err=0;
	struct sdio_func *func;
	bool claim_needed;	
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
_func_enter_;
	if(padapter->bSurpriseRemoved){
		//DBG_871X(" %s (padapter->bSurpriseRemoved)!!!\n",__FUNCTION__);
		return err;
	}	
	func = psdio->func;
	claim_needed = rtw_sdio_claim_host_needed(func);

	if (claim_needed)
		rtw_sdio_bus_ops.claim_host(func);
	err = _sd_cmd52_read(padapter, addr, cnt, pdata);
	if (err) {
		DBG_871X("%s(): err from _sd_cmd52_read is : %d\n", __func__, err);
	}
	if (claim_needed)
		rtw_sdio_bus_ops.release_host(func);

_func_exit_;

	return err;
}

/*
 * Return:
 *	0		Success
 *	others	Fail
 */
s32 _sd_cmd52_write(PADAPTER padapter, u32 addr, u32 cnt, u8 *pdata)
{
	
	int err=0;
	u32 i;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
	struct sdio_func *func;

_func_enter_;

	if(padapter->bSurpriseRemoved){
		//DBG_871X(" %s (padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n",__FUNCTION__);
		return err;
	}
	
	func = psdio->func;

	for (i = 0; i < cnt; i++) {
		rtw_sdio_bus_ops.writeb(func, pdata[i], addr+i, &err);
		if (err) {
			DBG_871X(KERN_ERR "%s: FAIL!(%d) addr=0x%05x val=0x%02x\n", __func__, err, addr+i, pdata[i]);
			break;
		}
	}

	if (err)
	{
		int j;

		//DBG_871X("%s: (%d) addr=0x%05x\n", __func__, err, addr);

		err = 0;
		for(j=0; j<SD_IO_TRY_CNT; j++)
		{
			for (i = 0; i < cnt; i++) {
				rtw_sdio_bus_ops.writeb(func, pdata[i], addr+i, &err);
				if (err) {
					//DBG_871X("%s(): sdio_writeb failed!\n", __func__);
					break;
				}
			}
			
			if (err == 0){
				rtw_reset_continual_io_error(padapter->dvobj);
				break;
			}else{				
				//DBG_871X("%s: (%d) addr=0x%05x, try_cnt=%d\n", __func__, err, addr, j);
				if(( -ESHUTDOWN == err ) || ( -ENODEV == err)){			
					padapter->bSurpriseRemoved = _TRUE;
				}

				if(rtw_inc_and_chk_continual_io_error(padapter->dvobj) == _TRUE ){
					padapter->bSurpriseRemoved = _TRUE;
					rtw_show_stack(NULL);
					break;
				}
			}
		}

		if (j==SD_IO_TRY_CNT)
			DBG_871X("%s: FAIL!(%d) addr=0x%05x, try_cnt=%d\n", __func__, err, addr, j);
		//else
			//DBG_871X("%s: (%d) addr=0x%05x, try_cnt=%d\n", __func__, err, addr, j);
	}

_func_exit_;

	return err;
}

/*
 * Return:
 *	0		Success
 *	others	Fail
 */
s32 sd_cmd52_write(PADAPTER padapter, u32 addr, u32 cnt, u8 *pdata)
{
	int err=0;
	struct sdio_func *func;
	bool claim_needed;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
_func_enter_;

	if(padapter->bSurpriseRemoved){
		//DBG_871X(" %s (padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n",__FUNCTION__);
		return err;
	}

	func = psdio->func;
	claim_needed = rtw_sdio_claim_host_needed(func);

	if (claim_needed)
		rtw_sdio_bus_ops.claim_host(func);
	err = _sd_cmd52_write(padapter, addr, cnt, pdata);
	if (claim_needed)
		rtw_sdio_bus_ops.release_host(func);
_func_exit_;

	return err;
}

u8 _sd_read8(PADAPTER padapter, u32 addr, s32 *err)
{
	u8 v=0;
	struct sdio_func *func;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
_func_enter_;

	if(padapter->bSurpriseRemoved){
		//DBG_871X(" %s (padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n",__FUNCTION__);
		return v;
	}
	
	func = psdio->func;

	v = rtw_sdio_bus_ops.readb(func, addr, err);

	if (err && *err)
		DBG_871X(KERN_ERR "%s: FAIL!(%d) addr=0x%05x\n", __func__, *err, addr);

_func_exit_;

	return v;
}

u8 sd_read8(PADAPTER padapter, u32 addr, s32 *err)
{
	u8 v=0;
	struct sdio_func *func;
	bool claim_needed;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
_func_enter_;

	if(padapter->bSurpriseRemoved){
		//DBG_871X(" %s (padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n",__FUNCTION__);
		return v;
	}
	
	func = psdio->func;
	claim_needed = rtw_sdio_claim_host_needed(func);

	if (claim_needed)
		rtw_sdio_bus_ops.claim_host(func);
	v = rtw_sdio_bus_ops.readb(func, addr, err);
	if (claim_needed)
		rtw_sdio_bus_ops.release_host(func);
	if (err && *err)
		DBG_871X(KERN_ERR "%s: FAIL!(%d) addr=0x%05x\n", __func__, *err, addr);

_func_exit_;

	return v;
}

u8 sd_f0_read8(PADAPTER padapter,u32 addr, s32 *err)
{

	u8 v=0;
	struct sdio_func *func;
	bool claim_needed;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
_func_enter_;

	if(padapter->bSurpriseRemoved){
		//DBG_871X(" %s (padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n",__FUNCTION__);
		return v;
	}

	func = psdio->func;
	claim_needed = rtw_sdio_claim_host_needed(func);

	if (claim_needed)
		rtw_sdio_bus_ops.claim_host(func);
	v = rtw_sdio_bus_ops.f0_readb(func, addr, err);
	if (claim_needed)
		rtw_sdio_bus_ops.release_host(func);
	if (err && *err)
		DBG_871X(KERN_ERR "%s: FAIL!(%d) addr=0x%05x\n", __func__, *err, addr);

_func_exit_;

	return v;
}

void sd_f0_write8(PADAPTER padapter, u32 addr, u8 v, s32 *err)
{
	struct sdio_func *func;
	bool claim_needed;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
	
_func_enter_;

	if(padapter->bSurpriseRemoved){
		//DBG_871X(" %s (padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n",__FUNCTION__);
		return;
	}	
	
	func = psdio->func;
	claim_needed = rtw_sdio_claim_host_needed(func);

	if (claim_needed)
		rtw_sdio_bus_ops.claim_host(func);
	rtw_sdio_bus_ops.f0_writeb(func, v, addr, err);
	if (claim_needed)
		rtw_sdio_bus_ops.release_host(func);
	if (err && *err)
		DBG_871X(KERN_ERR "%s: FAIL!(%d) addr=0x%05x val=0x%02x\n", __func__, *err, addr, v);

_func_exit_;
}

u16 sd_read16(PADAPTER padapter, u32 addr, s32 *err)
{
	u16 v=0;
	struct sdio_func *func;
	bool claim_needed;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
_func_enter_;

	if(padapter->bSurpriseRemoved){
		//DBG_871X(" %s (padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n",__FUNCTION__);
		return v;
	}
	
	func = psdio->func;
	claim_needed = rtw_sdio_claim_host_needed(func);

	if (claim_needed)
		rtw_sdio_bus_ops.claim_host(func);
	v = rtw_sdio_bus_ops.readw(func, addr, err);
	if (claim_needed)
		rtw_sdio_bus_ops.release_host(func);

	if (err && *err)
	{
		int i;

		DBG_871X(KERN_ERR "%s: (%d) addr=0x%05x, val=0x%x\n", __func__, *err, addr, v);

		*err = 0;
		for(i=0; i<SD_IO_TRY_CNT; i++)
		{
			if (claim_needed) rtw_sdio_bus_ops.claim_host(func);
			v = rtw_sdio_bus_ops.readw(func, addr, err);
			if (claim_needed) rtw_sdio_bus_ops.release_host(func);
			
			if (*err == 0){
				rtw_reset_continual_io_error(padapter->dvobj);
				break;
			}else{				
				DBG_871X(KERN_ERR "%s: (%d) addr=0x%05x, val=0x%x, try_cnt=%d\n", __func__, *err, addr, v, i);
				if(( -ESHUTDOWN == *err ) || ( -ENODEV == *err)){			
					padapter->bSurpriseRemoved = _TRUE;
				}

				if(rtw_inc_and_chk_continual_io_error(padapter->dvobj) == _TRUE ){
					padapter->bSurpriseRemoved = _TRUE;
					break;
				}
			}
		}

		if (i==SD_IO_TRY_CNT)
			DBG_871X(KERN_ERR "%s: FAIL!(%d) addr=0x%05x, val=0x%x, try_cnt=%d\n", __func__, *err, addr, v, i);
		//else
		//	DBG_871X(KERN_ERR "%s: (%d) addr=0x%05x, val=0x%x, try_cnt=%d\n", __func__, *err, addr, v, i);

	}

_func_exit_;

	return  v;
}

u32 sd_read32(PADAPTER padapter, u32 addr, s32 *err)
{	
	u32 v=0;
	struct sdio_func *func;
	bool claim_needed;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
_func_enter_;

	if(padapter->bSurpriseRemoved){
		//DBG_871X(" %s (padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n",__FUNCTION__);
		return v;
	}
	
	func = psdio->func;
	claim_needed = rtw_sdio_claim_host_needed(func);

	if (claim_needed)
		rtw_sdio_bus_ops.claim_host(func);
	v = rtw_sdio_bus_ops.readl(func, addr, err);
	if (claim_needed)
		rtw_sdio_bus_ops.release_host(func);

	if (err && *err)
	{
		int i;

		DBG_871X(KERN_ERR "%s: (%d) addr=0x%05x, val=0x%x\n", __func__, *err, addr, v);

		*err = 0;
		for(i=0; i<SD_IO_TRY_CNT; i++)
		{
			if (claim_needed) rtw_sdio_bus_ops.claim_host(func);
			v = rtw_sdio_bus_ops.readl(func, addr, err);
			if (claim_needed) rtw_sdio_bus_ops.release_host(func);
			
			if (*err == 0){
				rtw_reset_continual_io_error(padapter->dvobj);
				break;
			}else{				
				DBG_871X(KERN_ERR "%s: (%d) addr=0x%05x, val=0x%x, try_cnt=%d\n", __func__, *err, addr, v, i);
				if(( -ESHUTDOWN == *err ) || ( -ENODEV == *err)){			
					padapter->bSurpriseRemoved = _TRUE;
				}

				if(rtw_inc_and_chk_continual_io_error(padapter->dvobj) == _TRUE ){
					padapter->bSurpriseRemoved = _TRUE;
					break;
				}
			}
		}

		if (i==SD_IO_TRY_CNT)
			DBG_871X(KERN_ERR "%s: FAIL!(%d) addr=0x%05x, val=0x%x, try_cnt=%d\n", __func__, *err, addr, v, i);
		//else
		//	DBG_871X(KERN_ERR "%s: (%d) addr=0x%05x, val=0x%x, try_cnt=%d\n", __func__, *err, addr, v, i);

	}

_func_exit_;

	return  v;
}

void sd_write8(PADAPTER padapter, u32 addr, u8 v, s32 *err)
{	
	struct sdio_func *func;
	bool claim_needed;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
_func_enter_;

	if(padapter->bSurpriseRemoved){
		//DBG_871X(" %s (padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n",__FUNCTION__);
		return ;
	}
	
	func = psdio->func;
	claim_needed = rtw_sdio_claim_host_needed(func);

	if (claim_needed)
		rtw_sdio_bus_ops.claim_host(func);
	rtw_sdio_bus_ops.writeb(func, v, addr, err);
	if (claim_needed)
		rtw_sdio_bus_ops.release_host(func);
//	if (err && *err)
//		DBG_871X(KERN_ERR "%s: FAIL!(%d) addr=0x%05x val=0x%02x\n", __func__, *err, addr, v);
	if (err && *err)
	{
		int i;

		DBG_871X(KERN_ERR "%s: (%d) addr=0x%05x, val=0x%x\n", __func__, *err, addr, v);

		*err = 0;
		for(i=0; i<SD_IO_TRY_CNT; i++)
		{
			if (claim_needed) rtw_sdio_bus_ops.claim_host(func);
			rtw_sdio_bus_ops.writeb(func, v, addr, err);
			if (claim_needed) rtw_sdio_bus_ops.release_host(func);
			
			if (*err == 0){
				rtw_reset_continual_io_error(padapter->dvobj);
				break;
			}else{				
				DBG_871X(KERN_ERR "%s: (%d) addr=0x%05x, val=0x%x, try_cnt=%d\n", __func__, *err, addr, v, i);
				if(( -ESHUTDOWN == *err ) || ( -ENODEV == *err)){			
					padapter->bSurpriseRemoved = _TRUE;
				}

				if(rtw_inc_and_chk_continual_io_error(padapter->dvobj) == _TRUE ){
					padapter->bSurpriseRemoved = _TRUE;
					break;
				}
			}
		}

		if (i==SD_IO_TRY_CNT)
			DBG_871X(KERN_ERR "%s: FAIL!(%d) addr=0x%05x, val=0x%x, try_cnt=%d\n", __func__, *err, addr, v, i);
		//else
		//	DBG_871X(KERN_ERR "%s: (%d) addr=0x%05x, val=0x%x, try_cnt=%d\n", __func__, *err, addr, v, i);

	}

_func_exit_;
}

void sd_write16(PADAPTER padapter, u32 addr, u16 v, s32 *err)
{	
	struct sdio_func *func;
	bool claim_needed;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
_func_enter_;

	if(padapter->bSurpriseRemoved){
		//DBG_871X(" %s (padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n",__FUNCTION__);
		return ;
	}
	
	func = psdio->func;
	claim_needed = rtw_sdio_claim_host_needed(func);

	if (claim_needed)
		rtw_sdio_bus_ops.claim_host(func);
	rtw_sdio_bus_ops.writew(func, v, addr, err);
	if (claim_needed)
		rtw_sdio_bus_ops.release_host(func);
//	if (err && *err)
//		DBG_871X(KERN_ERR "%s: FAIL!(%d) addr=0x%05x val=0x%04x\n", __func__, *err, addr, v);
	if (err && *err)
	{
		int i;

		DBG_871X(KERN_ERR "%s: (%d) addr=0x%05x, val=0x%x\n", __func__, *err, addr, v);

		*err = 0;
		for(i=0; i<SD_IO_TRY_CNT; i++)
		{
			if (claim_needed) rtw_sdio_bus_ops.claim_host(func);
			rtw_sdio_bus_ops.writew(func, v, addr, err);
			if (claim_needed) rtw_sdio_bus_ops.release_host(func);
			
			if (*err == 0){
				rtw_reset_continual_io_error(padapter->dvobj);
				break;
			}else{				
				DBG_871X(KERN_ERR "%s: (%d) addr=0x%05x, val=0x%x, try_cnt=%d\n", __func__, *err, addr, v, i);
				if(( -ESHUTDOWN == *err ) || ( -ENODEV == *err)){			
					padapter->bSurpriseRemoved = _TRUE;
				}

				if(rtw_inc_and_chk_continual_io_error(padapter->dvobj) == _TRUE ){
					padapter->bSurpriseRemoved = _TRUE;
					break;
				}
			}
		}

		if (i==SD_IO_TRY_CNT)
			DBG_871X(KERN_ERR "%s: FAIL!(%d) addr=0x%05x, val=0x%x, try_cnt=%d\n", __func__, *err, addr, v, i);
		//else
		//	DBG_871X(KERN_ERR "%s: (%d) addr=0x%05x, val=0x%x, try_cnt=%d\n", __func__, *err, addr, v, i);

	}

_func_exit_;
}

void sd_write32(PADAPTER padapter, u32 addr, u32 v, s32 *err)
{
	struct sdio_func *func;
	bool claim_needed;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
_func_enter_;

	if(padapter->bSurpriseRemoved){
		//DBG_871X(" %s (padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n",__FUNCTION__);
		return ;
	}
	
	func = psdio->func;
	claim_needed = rtw_sdio_claim_host_needed(func);

	if (claim_needed)
		rtw_sdio_bus_ops.claim_host(func);
	rtw_sdio_bus_ops.writel(func, v, addr, err);
	if (claim_needed)
		rtw_sdio_bus_ops.release_host(func);

	if (err && *err)
	{
		int i;

		DBG_871X(KERN_ERR "%s: (%d) addr=0x%05x val=0x%08x\n", __func__, *err, addr, v);

		*err = 0;
		for(i=0; i<SD_IO_TRY_CNT; i++)
		{
			if (claim_needed) rtw_sdio_bus_ops.claim_host(func);
			rtw_sdio_bus_ops.writel(func, v, addr, err);
			if (claim_needed) rtw_sdio_bus_ops.release_host(func);
			if (*err == 0){
				rtw_reset_continual_io_error(padapter->dvobj);
				break;
			}else{				
				DBG_871X(KERN_ERR "%s: (%d) addr=0x%05x, val=0x%x, try_cnt=%d\n", __func__, *err, addr, v, i);
				if(( -ESHUTDOWN == *err ) || ( -ENODEV == *err)){			
					padapter->bSurpriseRemoved = _TRUE;
				}

				if(rtw_inc_and_chk_continual_io_error(padapter->dvobj) == _TRUE ){
					padapter->bSurpriseRemoved = _TRUE;
					break;
				}
			}
		}

		if (i==SD_IO_TRY_CNT)
			DBG_871X(KERN_ERR "%s: FAIL!(%d) addr=0x%05x val=0x%08x, try_cnt=%d\n", __func__, *err, addr, v, i);
		else
			DBG_871X(KERN_ERR "%s: (%d) addr=0x%05x val=0x%08x, try_cnt=%d\n", __func__, *err, addr, v, i);
	}

_func_exit_;
}

/*
 * Use CMD53 to read data from SDIO device.
 * This function MUST be called after sdio_claim_host() or
 * in SDIO ISR(host had been claimed).
 *
 * Parameters:
 *	psdio	pointer of SDIO_DATA
 *	addr	address to read
 *	cnt		amount to read
 *	pdata	pointer to put data, this should be a "DMA:able scratch buffer"!
 *
 * Return:
 *	0		Success
 *	others	Fail
 */
s32 _sd_read(PADAPTER padapter, u32 addr, u32 cnt, void *pdata)
{
	
	int err= -EPERM;
	struct sdio_func *pfunc;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
	
_func_enter_;

	if(padapter->bSurpriseRemoved){
		//DBG_871X(" %s (padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n",__FUNCTION__);
		return err;
	}
	
	pfunc = psdio->func;

	if (unlikely((cnt==1) || (cnt==2)))
	{
		u32 i;
		u8 *pbuf = (u8*)pdata;

		for (i = 0; i < cnt; i++)
		{
			*(pbuf+i) = rtw_sdio_bus_ops.readb(pfunc, addr+i, &err);

			if (err) {
				DBG_871X("%s: FAIL!(%d) addr=0x%05x\n", __func__, err, addr);
				break;
			}
		}
		return err;
	}

	err = rtw_sdio_bus_ops.memcpy_fromio(pfunc, pdata, addr, cnt);
	if (err) {
		DBG_871X("%s: FAIL(%d)! ADDR=0x%x Size=%d\n", __func__, err, addr, cnt);
		rtw_show_stack(NULL);
	}
#ifdef DBG_RX_AGG
	else{
		DBG_871X("%s: SUCCESS(%d)! ADDR=0x%x Size=%d\n", __func__, err, addr, cnt);
	}
#endif
_func_exit_;

	return err;
}

/*
 * Use CMD53 to read data from SDIO device.
 *
 * Parameters:
 *	psdio	pointer of SDIO_DATA
 *	addr	address to read
 *	cnt		amount to read
 *	pdata	pointer to put data, this should be a "DMA:able scratch buffer"!
 *
 * Return:
 *	0		Success
 *	others	Fail
 */
s32 sd_read(PADAPTER padapter, u32 addr, u32 cnt, void *pdata)
{	
	struct sdio_func *func;
	bool claim_needed;
	s32 err= -EPERM;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
_func_enter_;

	if(padapter->bSurpriseRemoved){
		//DBG_871X(" %s (padapter->bSurpriseRemoved ||adapter->pwrctrlpriv.pnp_bstop_trx)!!!\n",__FUNCTION__);
		return err;
	}
	
	func = psdio->func;
	claim_needed = rtw_sdio_claim_host_needed(func);

	if (claim_needed)
		rtw_sdio_bus_ops.claim_host(func);
	err = _sd_read(padapter, addr, cnt, pdata);
	if (claim_needed)
		rtw_sdio_bus_ops.release_host(func);
_func_exit_;
	return err;
}


/*
 * Use CMD53 to write data to SDIO device.
 * This function MUST be called after sdio_claim_host() or
 * in SDIO ISR(host had been claimed).
 *
 * Parameters:
 *	psdio	pointer of SDIO_DATA
 *	addr	address to write
 *	cnt		amount to write
 *	pdata	data pointer, this should be a "DMA:able scratch buffer"!
 *
 * Return:
 *	0		Success
 *	others	Fail
 */
s32 _sd_write(PADAPTER padapter, u32 addr, u32 cnt, void *pdata)
{
	
	struct sdio_func *pfunc;
	u32 size;
	s32 err=-EPERM;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
_func_enter_;

	if(padapter->bSurpriseRemoved){
		DBG_871X(" %s (padapter->bSurpriseRemoved!!!)\n",__FUNCTION__);
		return err;
	}	

	pfunc = psdio->func;
//	size = sdio_align_size(func, cnt);

	if (unlikely((cnt==1) || (cnt==2)))
	{
		u32 i;
		u8 *pbuf = (u8*)pdata;

		for (i = 0; i < cnt; i++)
		{
			rtw_sdio_bus_ops.writeb(pfunc, *(pbuf+i), addr+i, &err);
			if (err) {
				DBG_871X("%s: FAIL!(%d) addr=0x%05x val=0x%02x\n", __func__, err, addr, *(pbuf+i));
				break;
			}
		}

		return err;
	}

	size = cnt;
	//DBG_871X("%s(): write to addr 0x%x\n", __func__, addr);
	//DBG_871X("%s(): write size %d\n", __func__, size);
	err = rtw_sdio_bus_ops.memcpy_toio(pfunc, addr, pdata, size);
	if (err) {
		DBG_871X("%s: FAIL(%d)! ADDR=0x%x Size=%d(%d)\n", __func__, err, addr, cnt, size);
	}

_func_exit_;

	return err;
}

/*
 * Use CMD53 to write data to SDIO device.
 *
 * Parameters:
 *  psdio	pointer of SDIO_DATA
 *  addr	address to write
 *  cnt		amount to write
 *  pdata	data pointer, this should be a "DMA:able scratch buffer"!
 *
 * Return:
 *  0		Success
 *  others	Fail
 */
s32 sd_write(PADAPTER padapter, u32 addr, u32 cnt, void *pdata)
{
	struct sdio_func *func;
	bool claim_needed;
	s32 err=-EPERM;
	PSDIO_DATA psdio= &padapter->dvobj->intf_data;
_func_enter_;

	if(padapter->bSurpriseRemoved){
		DBG_871X(" %s (padapter->bSurpriseRemoved!!!)\n",__FUNCTION__);
		return err;
	}
	
	func = psdio->func;
	claim_needed = rtw_sdio_claim_host_needed(func);

	if (claim_needed)
		rtw_sdio_bus_ops.claim_host(func);
	err = _sd_write(padapter, addr, cnt, pdata);
	if (claim_needed)
		rtw_sdio_bus_ops.release_host(func);
_func_exit_;
	return err;
}

void sdio_set_intf_ops(PADAPTER padapter,struct _io_ops *pops)
{
_func_enter_;

	pops->_read8 = &sdio_read8;
	pops->_read16 = &sdio_read16;
	pops->_read32 = &sdio_read32;
	pops->_read_mem = &sdio_read_mem;
	pops->_read_port = &sdio_read_port;

	pops->_write8 = &sdio_write8;
	pops->_write16 = &sdio_write16;
	pops->_write32 = &sdio_write32;
	pops->_writeN = &sdio_writeN;
	pops->_write_mem = &sdio_write_mem;
	pops->_write_port = &sdio_write_port;

	pops->_sd_f0_read8 = sdio_f0_read8;

_func_exit_;
}

