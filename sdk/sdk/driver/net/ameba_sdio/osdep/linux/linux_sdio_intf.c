#if 1 // Irene Lin
#include <linux/mmc/errorno.h>
#include <linux/mmc/sdio.h>
#include "osdep_service_linux.h"
#include "ite/ite_sd.h"
#else
#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <linux/interrupt.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/rtnetlink.h>
#include <linux/fs.h>
#endif

#if 1//LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)  // Irene Lin
#if defined(WIN32) // Irene Lin
#define dev_printk(A,B,fmt,...) DBG_871X(A fmt,__VA_ARGS__)
#else
#define dev_printk(A,B,fmt,args...) DBG_871X(A fmt,##args)
#endif
#else
#include <linux/dma-mapping.h>
#include <linux/moduleparam.h>
#endif

#if 0 // Irene Lin
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/errno.h>
#endif



#include <autoconf.h>
#include <rtw_debug.h>
#include <sdio_ops.h>
#include <sdio_drvio.h>
#include <hal_intf.h>
#include <hci_intf.h>
#include <8195_sdio_reg.h>
#include <rtl8195a.h>
#include <rtl8195a_hal.h>
#include <osdep_service.h>
#include <osdep_intf.h>
#include <sdio_hal.h>
#include <sdio_ops_linux.h>
#include <rtw_ioctl.h>
#include <rtw_xmit.h>
#include <rtw_recv.h>
#include <rtw_io.h>
#include <rtw_cmd.h>

MODULE_AUTHOR("Realtek");
MODULE_DESCRIPTION("RealTek RTL-8195a iNIC");
MODULE_LICENSE("GPL");
MODULE_VERSION(RTL8195_VERSION);

#if 0 // for host rescan experiment
extern u8 rtw_assoc_cmd(_adapter*padapter);
extern void rtw_ps_cmd(PADAPTER padapter);
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
struct mmc_host *g_host = NULL;
void platform_wifi_power_on(void)
{
#if 0
	u8 *p = NULL;
	p = (u8 *) 0xf4691000;
	if(g_host == NULL)
		g_host = (struct mmc_host *)p;
	if(g_host)
	{
	DBG_871X("Try host rescan!\n");
			g_host->detect_change = 1;
			mmc_detect_change(g_host, 0);
	}
#endif
}
void platform_wifi_power_off(void)
{
#if 0
	if(g_host)
	{
			DBG_871X("g_host @ 0x%08x\n", (u32)g_host);
			g_host->detect_change = 1;
			mmc_detect_change(g_host, 0);
	}
#endif
}
#endif

void rtw_set_hal_ops(PADAPTER padapter)
{
	rtl8195as_set_hal_ops(padapter);
}
#if 0
static void sd_intf_start(PADAPTER padapter)
{
	if (padapter == NULL) {
		DBG_871X(KERN_ERR "%s: padapter is NULL!\n", __func__);
		return;
	}

	// hal dep
	rtw_hal_enable_interrupt(padapter);   
}

static void sd_intf_stop(PADAPTER padapter)
{
	if (padapter == NULL) {
		DBG_871X(KERN_ERR "%s: padapter is NULL!\n", __func__);
		return;
	}

	// hal dep
	rtw_hal_disable_interrupt(padapter);
}
#endif

_adapter *rtw_sdio_if1_init(struct dvobj_priv *dvobj, const struct sdio_device_id  *pdid){
	int status = _FAIL;
	struct net_device *pnetdev;
	PADAPTER padapter = NULL;
	u8 mac_addr[ETH_ALEN];
	u16 fw_ready;
	u32 i;
  
_func_enter_;
	if ((padapter = (_adapter *)rtw_zvmalloc(sizeof(*padapter))) == NULL) {
		DBG_871X("%s: vmalloc for padapter failed!\n", __FUNCTION__);
		goto exit;
	}
	padapter->dvobj = dvobj;
	dvobj->if1 = padapter;
	padapter->interface_type = RTW_SDIO;
	// 1. init network device data
	pnetdev = rtw_init_netdev(padapter);
	if (!pnetdev)
		goto free_adapter;
	SET_NETDEV_DEV(pnetdev, &dvobj->intf_data.func->dev);
	padapter = rtw_netdev_priv(pnetdev);

	// 2. init driver special setting, interface, OS and hardware relative
	rtw_set_hal_ops(padapter);
	
	// 3. initialize Chip version
	padapter->intf_start = &hci_intf_start;
	padapter->intf_stop = &hci_intf_stop;
#if 0
	padapter->intf_init = &sdio_init;
	padapter->intf_deinit = &sdio_deinit;
	padapter->intf_alloc_irq = &hci_dvobj_request_irq;
	padapter->intf_free_irq = &hci_dvobj_free_irq;
#endif	
	hci_set_intf_ops(padapter, &padapter->io_ops);

	// 4. init driver common data
	if (rtw_init_drv_sw(padapter) == _FAIL) {
		goto free_adapter;
	}

#ifdef CONFIG_POWER_SAVING
	// Ameba may in sleeping, wake it up first
	rtw_hal_rpwm_notify(padapter, RPWM2_PWR_WAKEUP);
	rtw_msleep_os(50);
#endif

	//wait until the firmware get ready
#ifdef CONFIG_FWDL
	for (i=0;i<100;i++) {
		fw_ready = rtw_read16(padapter, SDIO_REG_HCPWM2);
		if (fw_ready & SDIO_INIT_DONE) {
			break;
		}
		rtw_msleep_os(10);
	}
	if (i==100) {
		DBG_871X("%s: Wait Device Firmware Ready Timeout!!SDIO_REG_HCPWM2 @ 0x%04x\n", __FUNCTION__, fw_ready);
		goto free_adapter;
	}
#else
	padapter->bStopTrx = _TRUE;
	for (i=0;i<100;i++) {
		fw_ready = rtw_read8(padapter, SDIO_REG_CPU_IND);
		if (fw_ready & SDIO_SYSTEM_TRX_RDY_IND) {
			padapter->bStopTrx = _FALSE;
			break;
		}
		rtw_msleep_os(10);
	}
	if (i==100) {
		DBG_871X("%s: Wait Device Firmware Ready Timeout!!SDIO_REG_CPU_IND @ 0x%04x\n", __FUNCTION__, fw_ready);
		goto free_adapter;
	}
#endif

	// 5. fw init
	if(rtw_hal_init(padapter) == _FAIL)
		goto free_adapter;
	
	// 6. get MAC address after fw get ready
#ifndef CONFIG_LOOPBACK_TEST //if for loopback test, no need to read mac address
	if(rtw_hal_read_mac_addr(padapter, mac_addr) == _FAIL)
#endif
	{
		//use hardcode mac address if read mac from fw failed
		DBG_871X("%s: use default MAC address\n", __FUNCTION__);
		mac_addr[0] = 0x00;
		mac_addr[1] = 0xe0;
		mac_addr[2] = 0x4c;
		mac_addr[3] = 0xB7;
		mac_addr[4] = 0x23;
		mac_addr[5] = 0x00;
	}
	rtw_memcpy(padapter->mac_addr, mac_addr, ETH_ALEN);
	rtw_memcpy(pnetdev->dev_addr, mac_addr, ETH_ALEN);
	rtw_hal_disable_interrupt(padapter);

	DBG_871X("bDriverStopped:%d, bSurpriseRemoved:%d, bup:%d\n"
		,padapter->bDriverStopped
		,padapter->bSurpriseRemoved
		,padapter->bup
	);
	status = _SUCCESS;
free_adapter:
	if (status != _SUCCESS) {
		if (pnetdev)
			rtw_free_netdev(pnetdev);
		else
			rtw_vmfree((u8*)padapter, sizeof(*padapter));
		padapter = NULL;
	}	
exit:
	_func_exit_;
	return padapter;
}

static void rtw_sdio_if1_deinit(_adapter *if1)
{
	struct net_device *pnetdev = if1->pnetdev;
_func_enter_;
	rtw_cancel_all_timer(if1);
	rtw_dev_unload(if1);
	rtw_free_drv_sw(if1);
	if(pnetdev)
		rtw_free_netdev(pnetdev);
_func_exit_;
}

static int __devinit rtl8195a_init_one(struct sdio_func *func, const struct sdio_device_id *id)
{
	int status = _FAIL;
	PADAPTER padapter;
	struct dvobj_priv *dvobj;

	DBG_871X("%s():++\n",__FUNCTION__);
	DBG_871X("+rtw_drv_init: vendor=0x%04x device=0x%04x class=0x%02x\n", func->vendor, func->device, func->class);

#if 0 // record which host this card is attached to
	g_host = func->card->host;
#endif

	// 1. init sdio bus
	if ((dvobj = hci_dvobj_init(func)) == NULL) {
		goto exit;
	}
	
  	// 2. init device interface 
	if ((padapter = rtw_sdio_if1_init(dvobj, id)) == NULL) {
		DBG_871X("rtw_init_adapter Failed!\n");
		goto free_dvobj;
	}
	
	// 3. dev_alloc_name && register_netdev
	if((rtw_drv_register_netdev(padapter)) != _SUCCESS) {
		goto free_adapter;
	}	

	// 4. register for irq hander
	if (hci_dvobj_request_irq(dvobj) != _SUCCESS)
	{
		goto unregister_netdev;
	}

//	rtw_assoc_cmd(padapter);
	status = _SUCCESS;
    func->type = SDIO_AMEBA; // Irene Lin

unregister_netdev:
	if(status != _SUCCESS && padapter->pnetdev){
		rtw_drv_unregister_netdev(padapter);
	}
free_adapter:
	if (status != _SUCCESS && padapter) {
		rtw_sdio_if1_deinit(padapter);
	}	
free_dvobj:
	if (status != _SUCCESS)
		hci_dvobj_deinit(dvobj);
	
exit:
	return status == _SUCCESS?0:-ENODEV;
}

static void __devexit rtl8195a_remove_one(struct sdio_func *func)

{
	PADAPTER padapter;
	struct dvobj_priv *dvobj = rtw_sdio_get_drvdata(func);
	padapter = dvobj->if1;
	DBG_871X("%s():++\n", __FUNCTION__);	
	if(padapter)
	{
#ifdef CONFIG_POWER_SAVING
		rtw_ps_deny(padapter, PS_DENY_DRV_REMOVE);
		LeaveAllPowerSaveMode(padapter);
#endif
		rtw_drv_unregister_netdev(padapter);
		if (padapter->bSurpriseRemoved == _FALSE) {
			int err;

			/* test surprise remove */
			rtw_sdio_bus_ops.claim_host(func);
			rtw_sdio_bus_ops.readb(func, 0, &err);
			rtw_sdio_bus_ops.release_host(func);
			if (err == -ENOMEDIUM) {
				padapter->bSurpriseRemoved = _TRUE;
				DBG_871X(KERN_NOTICE "%s: device had been removed!\n", __func__);
			}
		}
		rtw_sdio_if1_deinit(padapter);
	}
	hci_dvobj_deinit(dvobj);
}


static const struct sdio_device_id sdio_ids[] =
{
	{ SDIO_DEVICE(0x024c, 0x8195),.driver_data = (void*)RTL8195A}, // Irene Lin
    //{ SDIO_DEVICE(0x024c, 0x8195), .driver_data = RTL8195A },
};

static struct sdio_driver rtl8195a_sdio_driver = {
	.probe	= rtl8195a_init_one,
	.remove	= __devexit_p(rtl8195a_remove_one),
	.name	= MODULENAME,
	.id_table	= sdio_ids,
};

static int __init rtl8195a_init_module(void)
{

	int ret = 0;

	DBG_871X("module init start\n");
	DBG_871X("%s %s\n", DRV_NAME, DRIVERVERSION);
	DBG_871X("build time: %s %s\n", __DATE__, __TIME__);

	//platform_wifi_power_on();

	ret = rtw_sdio_bus_ops.reg_driver(&rtl8195a_sdio_driver);
	if(ret != 0)
		DBG_871X("sdio register driver Failed!\n");

	DBG_871X("module init ret=%d\n", ret);
	return ret;
}

#if 1 // Irene Lin
int iteAmebaSdioWifiRegister(void)
{
	return rtl8195a_init_module();
}
#endif

static void __exit rtl8195a_cleanup_module(void)
{
	DBG_871X("module exit start\n");

	rtw_sdio_bus_ops.unreg_driver(&rtl8195a_sdio_driver);

	//platform_wifi_power_off();

	DBG_871X("module exit success\n");
}

module_init(rtl8195a_init_module);
module_exit(rtl8195a_cleanup_module);
