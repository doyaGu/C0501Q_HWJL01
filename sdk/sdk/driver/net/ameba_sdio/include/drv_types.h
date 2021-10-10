#ifndef __DRV_TYPES_H__
#define __DRV_TYPES_H__
#include <autoconf.h>


typedef struct _ADAPTER _adapter,ADAPTER,*PADAPTER;

#include <rtw_debug.h>
#include <osdep_service.h>

#ifdef CONFIG_SDIO_HCI
#include <drv_types_sdio.h>
#define INTF_DATA SDIO_DATA
#elif	defined(CONFIG_USB_HCI)
#include "drv_types_usb.h"
#define INTF_DATA USB_DATA
#elif	defined(CONFIG_GSPI_HCI)
#include "drv_types_gspi.h"
#define INTF_DATA GSPI_DATA
#endif

#if 1//def PLATFORM_FREERTOS  // Irene Lin
#include <freertos/ite_skbuf.h>
#include <freertos/ite_ndis_ameba.h>
//#include <freertos/netdevice_freertos.h>
//#include <freertos/task_struct_freertos.h>
#include <linux/kthread.h>
#endif //PLATFORM_FREERTOS
#include <linux/completion.h>   // Irene Lin

#ifdef PLATFORM_ECOS
#include <wifi_api.h>
#endif

#ifdef CONFIG_POWER_SAVING
#include <rtw_pwrctrl.h>
#endif
struct rtw_traffic_statistics {
	// tx statistics
	u64	tx_bytes;
	u64	tx_pkts;
	u64	tx_drop;
	u64	cur_tx_bytes;
	u64	last_tx_bytes;
	u32	cur_tx_tp; // Tx throughput in MBps.

	// rx statistics
	u64	rx_bytes;
	u64	rx_pkts;
	u64	rx_drop;
	u64	cur_rx_bytes;
	u64	last_rx_bytes;
	u32	cur_rx_tp; // Rx throughput in MBps.
};
struct dvobj_priv
{
	_lock lock;
	/*-------- below is common data --------*/	
	_adapter *if1; //PRIMARY_ADAPTER
	int processing_dev_remove;
	
	u8	irq_alloc;
	ATOMIC_T continual_io_error;

	/*-------- below is for SDIO INTERFACE --------*/
#ifdef INTF_DATA
	INTF_DATA intf_data;
#endif
#ifdef CONFIG_POWER_SAVING
	struct pwrctrl_priv pwrctl_priv;
	struct rtw_traffic_statistics	traffic_stat;
#endif
};

typedef struct _RT_LINK_DETECT_T{
	u32				NumTxOkInPeriod;
	u32				NumRxOkInPeriod;
	u32				NumRxUnicastOkInPeriod;
	BOOLEAN			bBusyTraffic;
	BOOLEAN			bTxBusyTraffic;
	BOOLEAN			bRxBusyTraffic;
	BOOLEAN			bHigherBusyTraffic; // For interrupt migration purpose.
	BOOLEAN			bHigherBusyRxTraffic; // We may disable Tx interrupt according as Rx traffic.
	BOOLEAN			bHigherBusyTxTraffic; // We may disable Tx interrupt according as Tx traffic.
}RT_LINK_DETECT_T, *PRT_LINK_DETECT_T;

/*
#ifdef PLATFORM_LINUX
static struct device *dvobj_to_dev(struct dvobj_priv *dvobj)
{
#ifdef CONFIG_USB_HCI
	return &dvobj->intf_data->intf->dev;
#endif
#ifdef CONFIG_SDIO_HCI
	return &dvobj->intf_data.func->dev;
#endif
}
#endif
*/
#include "rtw_io.h"
#include "rtw_xmit.h"
#include "rtw_recv.h"
#include "rtw_cmd.h"
#include "hal_intf.h"
struct _ADAPTER
{
	_lock lock;
	s32	bDriverStopped;
	s32	bSurpriseRemoved;
	u8	hw_init_completed;
	s32  bStopTrx; //when wifi off/on, to indicate stop/resume packet transmittion
	s32 fw_status; //indicate the wlan link status in FW, 0: unlinked 1: linked
	int net_closed;	
	u8 netif_up;
#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREERTOS) || defined(PLATFORM_ECOS)
	int bup;
	_nic_hdl pnetdev;
	u8 mac_addr[6];	//PermanentAddress
	struct net_device_stats stats;
#endif
#ifdef PLATFORM_ECOS
	wifi_link_status_cb_func_t	*link_status_cb_func; // for client mode
	wifi_sta_status_cb_func_t	*sta_status_cb_func;  // for AP mode
	wifi_scan_cb_func_t *wifi_scan_cb_func; // for scan result report
	wifi_sc_cb_func_t *wifi_sc_cb_func; //for wifi simple config
	wifi_query_cb_func_t *wifi_query_cb_func; //for wifi status query
	wifi_wowlan_cb_func_t *wifi_wowlan_cb_func; //for wifi wowlan status query
#endif
	u16	interface_type;//USB,SDIO

	u16	FirmwareVersion;
	u16	FirmwareVersionRev;
	u16	FirmwareSubVersion;
	u16	FirmwareSignature;

//	u32	FirmwareStartAddr;
//	u32	FirmwareSize;
	u32	FirmwareEntryFun;

#ifdef PLATFORM_FREERTOS
	u8	RxStop;	//Used to stop rx thread as early as possible

#if defined(CONFIG_ISR_THREAD_MODE_INTERRUPT)
	struct task_struct	isrThread;
#endif
#ifdef CONFIG_WIFI_STATEMACHINE
	struct task_struct	wifiStateThread;
	u8	WifiState;
#endif
#endif //PLATFORM_FREERTOS

#ifdef CONFIG_POWER_SAVING
	RT_LINK_DETECT_T	LinkDetectInfo;
#ifdef CONFIG_PS_DYNAMIC_CHK
#define DYNAMIC_CHK_TMR_INTERVAL	2000 //ms
	_timer	dynamic_chk_timer; //dynamic/periodic check timer
#endif
#endif

#ifdef PLATFORM_ECOS
	int	has_triggered_rx_tasklet;
	int	has_triggered_tx_tasklet;
	int				call_dsr;
#endif
	struct dvobj_priv *dvobj;
	//For xmit priv
	struct xmit_priv xmitpriv;
	//For recv priv
	struct recv_priv recvpriv;
	//For cmd priv
	struct cmd_priv cmdpriv;
	struct _io_ops io_ops;
	struct hal_ops HalFunc;
#if 0	
 	u32 (*intf_init)(struct dvobj_priv *dvobj);
	void (*intf_deinit)(struct dvobj_priv *dvobj);
	int (*intf_alloc_irq)(struct dvobj_priv *dvobj);
	void (*intf_free_irq)(struct dvobj_priv *dvobj);
#endif
	void (*intf_start)(_adapter * adapter);
	void (*intf_stop)(_adapter * adapter);

#ifdef CONFIG_LOOPBACK_TEST
	_thread_hdl_ LPThread;
#endif
};

#define RTW_CANNOT_IO(padapter) \
			((padapter)->bSurpriseRemoved)

#define RTW_CANNOT_RX(padapter) \
			((padapter)->bDriverStopped || \
			 (padapter)->bSurpriseRemoved)

#define RTW_CANNOT_TX(padapter) \
			((padapter)->bDriverStopped || \
			 (padapter)->bSurpriseRemoved)

#define dvobj_to_pwrctl(dvobj) (&(dvobj->pwrctl_priv))
#define adapter_to_dvobj(adapter) (adapter->dvobj)
#define adapter_to_pwrctl(adapter) (dvobj_to_pwrctl(adapter->dvobj))

#ifdef PLATFORM_ECOS
struct _device_info_ {
	int type;
	unsigned long conf_addr;
	unsigned long base_addr;
	int irq;
	_adapter *priv;
};
extern struct _device_info_ wlan_device[];
#endif //PLATFORM_ECOS

#define 	WIFI_NULL_STATE		0x00000000
#define	WIFI_ASOC_STATE		0x00000001		// Under Linked state...
#define	WIFI_STATION_STATE	0x00000008
#define	WIFI_AP_STATE			0x00000010
#define	WIFI_WSC_STATE		0x00000100
#define	_FW_LINKED			WIFI_ASOC_STATE

#define IW_EVT_STR_STA_ASSOC	"STA Assoc"
#define IW_EVT_STR_STA_DISASSOC	"STA Disassoc"

__inline static sint check_fwstate(_adapter *padapter, sint state)
{
	if (padapter->fw_status & state)
		return _TRUE;

	return _FALSE;
}

/*
 * No Limit on the calling context,
 * therefore set it to be the critical section...
 *
 * ### NOTE:#### (!!!!)
 * MUST TAKE CARE THAT BEFORE CALLING THIS FUNC, YOU SHOULD HAVE LOCKED padapter->lock
 */

 __inline static void set_fwstate(_adapter *padapter, sint state)
{
	padapter->fw_status |= state;
}
#if 0
__inline static void set_fwstate(_adapter *padapter, sint state)
{
	_irqL irql;
	rtw_enter_critical_bh(&padapter->lock, &irql);
	_set_fwstate(padapter, state);
	rtw_exit_critical_bh(&padapter->lock, &irql);
}
#endif
/*
 * No Limit on the calling context,
 * therefore set it to be the critical section...
 */
 __inline static void _clr_fwstate(_adapter *padapter, sint state)
{
	if (check_fwstate(padapter, state) == _TRUE)
		padapter->fw_status ^= state;
}
__inline static void clr_fwstate(_adapter *padapter, sint state)
{
	_irqL irql;
	rtw_enter_critical_bh(&padapter->lock, &irql);
	_clr_fwstate(padapter, state);
	rtw_exit_critical_bh(&padapter->lock, &irql);
}
__inline static void _reset_fwstate(_adapter *padapter)
{
	padapter->fw_status = WIFI_NULL_STATE;
}
__inline static void reset_fwstate(_adapter *padapter)
{
	_irqL irql;
	rtw_enter_critical_bh(&padapter->lock, &irql);
	_reset_fwstate(padapter);
	rtw_exit_critical_bh(&padapter->lock, &irql);
}
#endif //__DRV_TYPES_H__

