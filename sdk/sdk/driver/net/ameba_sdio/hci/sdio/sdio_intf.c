#include <linux/current.h> // Irene Lin
#include <autoconf.h>
#include <rtw_debug.h>
#include <sdio_ops.h>
#include <sdio_drvio.h>
#include <hci_intf.h>
#include <osdep_intf.h>

static u32 sdio_init(struct dvobj_priv *dvobj)
{
	PSDIO_DATA psdio_data;
	struct sdio_func *func;
	int err;

_func_enter_;

	psdio_data = &dvobj->intf_data;
	func = psdio_data->func;

	// 1. init SDIO bus
	rtw_sdio_bus_ops.claim_host(func);

	err = rtw_sdio_bus_ops.enable_func(func);
	if (err) {
		DBG_871X(KERN_CRIT "%s: sdio_enable_func FAIL(%d)!\n", __func__, err);
		goto release;
	}

	err = rtw_sdio_set_block_size(func, 512);
	if (err) {
		DBG_871X(KERN_CRIT "%s: sdio_set_block_size FAIL(%d)!\n", __func__, err);
		goto release;
	}
	psdio_data->block_transfer_len = 512;
	psdio_data->tx_block_mode = 1;
	psdio_data->rx_block_mode = 1;

release:
	rtw_sdio_bus_ops.release_host(func);

_func_exit_;
	if (err) return _FAIL;
	return _SUCCESS;
}
static void sdio_deinit(struct dvobj_priv *dvobj)
{
	struct sdio_func *func;
	int err;

	func = dvobj->intf_data.func;

	if (func) {
		rtw_sdio_bus_ops.claim_host(func);
		err = rtw_sdio_bus_ops.disable_func(func);
		if (err)
		{
			DBG_871X(KERN_ERR "%s: sdio_disable_func(%d)\n", __func__, err);
		}

		if (dvobj->irq_alloc) {
			err = rtw_sdio_bus_ops.release_irq(func);
			if (err)
			{
				DBG_871X(KERN_ERR "%s: sdio_release_irq(%d)\n", __func__, err);
			}
		}

		rtw_sdio_bus_ops.release_host(func);
	}
}

static struct dvobj_priv *sdio_dvobj_init(void *data)
{
	struct sdio_func *func = (struct sdio_func *)data;
	int status = _FAIL;
	struct dvobj_priv *dvobj = NULL;
	PSDIO_DATA psdio;
_func_enter_;

	if((dvobj = devobj_init()) == NULL) {
		goto exit;
	}

	rtw_sdio_set_drvdata(func, (void *)dvobj);
	psdio = &dvobj->intf_data;
	psdio->func = func;
	psdio->SdioRxFIFOCnt = 0;	
	if (sdio_init(dvobj) != _SUCCESS) {
		RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("%s: initialize SDIO Failed!\n", __FUNCTION__));
		goto free_dvobj;
	}
	rtw_reset_continual_io_error(dvobj);
	status = _SUCCESS;

free_dvobj:
	if (status != _SUCCESS && dvobj) {
		rtw_sdio_set_drvdata(func, NULL);
		
		devobj_deinit(dvobj);
		
		dvobj = NULL;
	}
exit:
_func_exit_;
	return dvobj;
}

static void sdio_dvobj_deinit(struct dvobj_priv *dvobj)
{
	PSDIO_DATA psdio_data;
	struct sdio_func *func;
_func_enter_;

	psdio_data = &dvobj->intf_data;
	func = psdio_data->func;
	rtw_sdio_set_drvdata(func, NULL);
	if (dvobj) {
		sdio_deinit(dvobj);
		devobj_deinit(dvobj);
	}

_func_exit_;
	return;
}

#ifndef PLATFORM_ECOS
static
#endif
void sd_sync_int_hdl(struct sdio_func *func)
{
	struct dvobj_priv *psdpriv;

	if(func == NULL)
	{
		DBG_871X("%s: func is NULL!\n", __FUNCTION__);
		return;
	}
	psdpriv = (struct dvobj_priv *) rtw_sdio_get_drvdata(func);
		
#ifdef PLATFORM_LINUX
	rtw_sdio_set_irq_thd(psdpriv, current);
#endif

	sd_int_hdl(psdpriv->if1);

#ifdef PLATFORM_LINUX
	rtw_sdio_set_irq_thd(psdpriv, NULL);
#endif
}

static int sdio_alloc_irq(struct dvobj_priv *dvobj)
{
	PSDIO_DATA psdio_data;
	struct sdio_func *func;
	int err = 0;

	psdio_data = &dvobj->intf_data;
	func = psdio_data->func;

	rtw_sdio_bus_ops.claim_host(func);

#ifdef PLATFORM_ECOS
	err = rtw_sdio_bus_ops.claim_irq(func, &sdio_int_hdl);
#else
	err = rtw_sdio_bus_ops.claim_irq(func, &sd_sync_int_hdl);
#endif
	if (err)
	{
		DBG_871X(KERN_CRIT "%s: sdio_claim_irq FAIL(%d)!\n", __func__, err);
	}
	else
	{
		dvobj->irq_alloc = 1;
	}

	rtw_sdio_bus_ops.release_host(func);

	return err?_FAIL:_SUCCESS;
}

static void sdio_free_irq(struct dvobj_priv *dvobj)
{
    PSDIO_DATA psdio_data;
    struct sdio_func *func;
    int err;

	if (dvobj->irq_alloc) {
		psdio_data = &dvobj->intf_data;
		func = psdio_data->func;

		if (func) {
			rtw_sdio_bus_ops.claim_host(func);
			err = rtw_sdio_bus_ops.release_irq(func);
			if (err)
			{
				DBG_871X("%s: sdio_release_irq FAIL(%d)!\n", __func__, err);
			}
			rtw_sdio_bus_ops.release_host(func);
		}
		dvobj->irq_alloc = 0;
	}
}

const struct host_ctrl_intf_ops hci_ops = {
	sdio_dvobj_init,
	sdio_dvobj_deinit,
	sdio_alloc_irq,
	sdio_free_irq
};
