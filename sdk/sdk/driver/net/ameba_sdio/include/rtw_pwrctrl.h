/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef __RTW_PWRCTRL_H_
#define __RTW_PWRCTRL_H_
#include <basic_types.h>

#ifdef CONFIG_WOWLAN
enum rtw_proto_offloads {
	RTW_WOWLAN_PROTO_OFFLOAD_ARP = BIT(0),
	RTW_WOWLAN_PROTO_OFFLOAD_NS = BIT(1),
};

/**
 * struct rtw_proto_offload_cmd_common - ARP/NS offload common part
 * @enabled: enable flags
 * @remote_ipv4_addr: remote address to answer to (or zero if all)
 * @host_ipv4_addr: our IPv4 address to respond to queries for
 * @arp_mac_addr: our MAC address for ARP responses
 * @reserved: unused
 */
 #ifdef RTW_PACK_STRUCT_USE_INCLUDES
#include "pack_begin.h"
#endif
RTW_PACK_STRUCT_BEGIN
struct rtw_proto_offload_cmd_common {
	u32 enabled;
	u32 remote_ipv4_addr;
	u32 host_ipv4_addr;
	u8 arp_mac_addr[ETH_ALEN];
	u32 reserved;
} RTW_PACK_STRUCT_STRUCT;
#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#include "pack_end.h"
#endif
/*
 * REPLY_WOWLAN_PATTERNS
 */
#define RTW_WOWLAN_MIN_PATTERN_LEN	16
#define RTW_WOWLAN_MAX_PATTERN_LEN	128

#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#include "pack_begin.h"
#endif
RTW_PACK_STRUCT_BEGIN
struct rtw_wowlan_pattern {
	u8 mask[RTW_WOWLAN_MAX_PATTERN_LEN / 8];
	u8 pattern[RTW_WOWLAN_MAX_PATTERN_LEN];
	u8 mask_size;
	u8 pattern_size;
	u16 reserved;
} RTW_PACK_STRUCT_STRUCT;
#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#include "pack_end.h"
#endif

#define rtw_WOWLAN_MAX_PATTERNS	20

#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#include "pack_begin.h"
#endif
RTW_PACK_STRUCT_BEGIN
struct rtw_wowlan_patterns_cmd {
	u32 n_patterns;
	struct rtw_wowlan_pattern patterns[];
} RTW_PACK_STRUCT_STRUCT;
#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#include "pack_end.h"
#endif

/*
 * REPLY_WOWLAN_WAKEUP_FILTER
 */
enum rtw_wowlan_wakeup_filters {
	RTW_WOWLAN_WAKEUP_MAGIC_PACKET				= BIT(0),
	RTW_WOWLAN_WAKEUP_PATTERN_MATCH			= BIT(1),
	RTW_WOWLAN_WAKEUP_BEACON_MISS				= BIT(2),
	RTW_WOWLAN_WAKEUP_LINK_CHANGE				= BIT(3),
	RTW_WOWLAN_WAKEUP_GTK_REKEY_FAIL			= BIT(4),
	RTW_WOWLAN_WAKEUP_EAP_IDENT_REQ				= BIT(5),
	RTW_WOWLAN_WAKEUP_4WAY_HANDSHAKE			= BIT(6),
	RTW_WOWLAN_WAKEUP_ALWAYS					= BIT(7),
	RTW_WOWLAN_WAKEUP_ENABLE_NET_DETECT		= BIT(8),
	RTW_WOWLAN_WAKEUP_RF_KILL_DEASSERT			= BIT(9),
	RTW_WOWLAN_WAKEUP_REMOTE_LINK_LOSS			= BIT(10),
	RTW_WOWLAN_WAKEUP_REMOTE_SIGNATURE_TABLE	= BIT(11),
	RTW_WOWLAN_WAKEUP_REMOTE_WAKEUP_PACKET	= BIT(12),
	RTW_WOWLAN_WAKEUP_MAX = 0x7FFFFFFF
};

#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#include "pack_begin.h"
#endif
RTW_PACK_STRUCT_BEGIN
struct rtw_wowlan_config_cmd {
	u32 wakeup_filter;
	u16 non_qos_seq;
	u16 qos_seq[8];
	u8 wowlan_ba_teardown_tids;
	u8 is_11n_connection;
} RTW_PACK_STRUCT_STRUCT;
#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#include "pack_end.h"
#endif

enum rtw_wowlan_cmd_id{
	RTW_WOWLAN_CMD_GET_STATUS = 0x00, // get rtw_wowlan_status
	RTW_WOWLAN_CMD_PATTERNS = 0x01, // wowlan pattern setting
	RTW_WOWLAN_CMD_CFG = 0x02, // set wake up event
	RTW_WOWLAN_CMD_PROT_OFFLOAD_CONFIG = 0x03, //ARP offload
	RTW_WOWLAN_CMD_CLEAR_ALL = 0x04, //clear all wowlan setting
	RTW_WOWLAN_CMD_MAX = 0xff
};
/*
 * REPLY_WOWLAN_GET_STATUS = 0xe5
 */
#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#include "pack_begin.h"
#endif
RTW_PACK_STRUCT_BEGIN
struct rtw_wowlan_status {
	u64 replay_ctr;
	u16 pattern_number;
	u16 non_qos_seq_ctr;
	u16 qos_seq_ctr[8];
	u32 wakeup_reasons;
	u32 rekey_status;
	u32 num_of_gtk_rekeys;
	u32 transmitted_ndps;
	u32 received_beacons;
	u32 wake_packet_length;
	u32 wake_packet_bufsize;
	u8 wake_packet[]; /* can be truncated from _length to _bufsize */
} RTW_PACK_STRUCT_STRUCT;
#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#include "pack_end.h"
#endif
#endif

#if 0
#define FW_PWR0	0	
#define FW_PWR1 	1
#define FW_PWR2 	2
#define FW_PWR3 	3


#define HW_PWR0	7	
#define HW_PWR1 	6
#define HW_PWR2 	2
#define HW_PWR3	0
#define HW_PWR4	8

#define FW_PWRMSK	0x7


#define XMIT_ALIVE	BIT(0)
#define RECV_ALIVE	BIT(1)
#define CMD_ALIVE	BIT(2)
#define EVT_ALIVE	BIT(3)
#ifdef CONFIG_BT_COEXIST
#define BTCOEX_ALIVE	BIT(4)
#endif // CONFIG_BT_COEXIST




#ifdef CONFIG_PNO_SUPPORT
#define MAX_PNO_LIST_COUNT 16
#define MAX_SCAN_LIST_COUNT 14 //2.4G only
#endif

/*
	BIT[2:0] = HW state
	BIT[3] = Protocol PS state,   0: register active state , 1: register sleep state
	BIT[4] = sub-state
*/

#define PS_DPS				BIT(0)
#define PS_LCLK				(PS_DPS)
#define PS_RF_OFF			BIT(1)
#define PS_ALL_ON			BIT(2)
#define PS_ST_ACTIVE		BIT(3)

#define PS_ISR_ENABLE		BIT(4)
#define PS_IMR_ENABLE		BIT(5)
#define PS_ACK				BIT(6)
#define PS_TOGGLE			BIT(7)

#define PS_STATE_MASK		(0x0F)
#define PS_STATE_HW_MASK	(0x07)
#define PS_SEQ_MASK			(0xc0)

#define PS_STATE(x)		(PS_STATE_MASK & (x))
#define PS_STATE_HW(x)	(PS_STATE_HW_MASK & (x))
#define PS_SEQ(x)		(PS_SEQ_MASK & (x))

#define PS_STATE_S0		(PS_DPS)
#define PS_STATE_S1		(PS_LCLK)
#define PS_STATE_S2		(PS_RF_OFF)
#define PS_STATE_S3		(PS_ALL_ON)
#define PS_STATE_S4		((PS_ST_ACTIVE) | (PS_ALL_ON))


#define PS_IS_RF_ON(x)	((x) & (PS_ALL_ON))
#define PS_IS_ACTIVE(x)	((x) & (PS_ST_ACTIVE))
#define CLR_PS_STATE(x)	((x) = ((x) & (0xF0)))


struct reportpwrstate_parm {
	unsigned char mode;
	unsigned char state; //the CPWM value
	unsigned short rsvd;
}; 

#define LPS_DELAY_TIME	1*HZ // 1 sec

#define EXE_PWR_NONE	0x01
#define EXE_PWR_IPS		0x02
#define EXE_PWR_LPS		0x04

// RF state.
typedef enum _rt_rf_power_state
{
	rf_on,		// RF is on after RFSleep or RFOff
	rf_sleep,	// 802.11 Power Save mode
	rf_off,		// HW/SW Radio OFF or Inactive Power Save
	//=====Add the new RF state above this line=====//
	rf_max
}rt_rf_power_state;

// RF Off Level for IPS or HW/SW radio off
#define	RT_RF_OFF_LEVL_ASPM			BIT(0)	// PCI ASPM
#define	RT_RF_OFF_LEVL_CLK_REQ		BIT(1)	// PCI clock request
#define	RT_RF_OFF_LEVL_PCI_D3			BIT(2)	// PCI D3 mode
#define	RT_RF_OFF_LEVL_HALT_NIC		BIT(3)	// NIC halt, re-initialize hw parameters
#define	RT_RF_OFF_LEVL_FREE_FW		BIT(4)	// FW free, re-download the FW
#define	RT_RF_OFF_LEVL_FW_32K		BIT(5)	// FW in 32k
#define	RT_RF_PS_LEVEL_ALWAYS_ASPM	BIT(6)	// Always enable ASPM and Clock Req in initialization.
#define	RT_RF_LPS_DISALBE_2R			BIT(30)	// When LPS is on, disable 2R if no packet is received or transmittd.
#define	RT_RF_LPS_LEVEL_ASPM			BIT(31)	// LPS with ASPM

#define	RT_IN_PS_LEVEL(ppsc, _PS_FLAG)		((ppsc->cur_ps_level & _PS_FLAG) ? _TRUE : _FALSE)
#define	RT_CLEAR_PS_LEVEL(ppsc, _PS_FLAG)	(ppsc->cur_ps_level &= (~(_PS_FLAG)))
#define	RT_SET_PS_LEVEL(ppsc, _PS_FLAG)		(ppsc->cur_ps_level |= _PS_FLAG)

// ASPM OSC Control bit, added by Roger, 2013.03.29.
#define	RT_PCI_ASPM_OSC_IGNORE		0	 // PCI ASPM ignore OSC control in default
#define	RT_PCI_ASPM_OSC_ENABLE		BIT0 // PCI ASPM controlled by OS according to ACPI Spec 5.0
#define	RT_PCI_ASPM_OSC_DISABLE		BIT1 // PCI ASPM controlled by driver or BIOS, i.e., force enable ASPM


enum { // for ips_mode
	IPS_NONE=0,
	IPS_NORMAL,
	IPS_LEVEL_2,	
	IPS_NUM
};
#endif

// Design for pwrctrl_priv.ips_deny, 32 bits for 32 reasons at most
typedef enum _PS_DENY_REASON
{
	PS_DENY_DRV_INITIAL = 0,
	PS_DENY_DRV_TXDATA = 1,
	PS_DENY_DRV_RXDATA = 2,
	PS_DENY_DRV_REMOVE = 30,
	PS_DENY_OTHERS = 31
} PS_DENY_REASON;

enum Power_Mgnt
{
	PS_MODE_ACTIVE	= 0	,
	PS_MODE_SLEEP			,
	PS_MODE_NUM,
};

typedef _sema _pwrlock;


__inline static void _init_pwrlock(_pwrlock *plock)
{
	rtw_init_sema(plock, 1);
}

__inline static void _free_pwrlock(_pwrlock *plock)
{
	rtw_free_sema(plock);
}


__inline static void _enter_pwrlock(_pwrlock *plock)
{
	rtw_down_sema(plock);
}


__inline static void _exit_pwrlock(_pwrlock *plock)
{
	rtw_up_sema(plock);
}

struct pwrctrl_priv
{
	_pwrlock	lock;
	volatile u16 rpwm2; // requested power state for fw
	volatile u16 cpwm2; // fw current power state. updated when 1. read from HCPWM 2. driver lowers power level
//	volatile u8 tog; // toggling
//	volatile u8 cpwm_tog; // toggling

	u8	ps_enable;
	u8	pwr_mode;
	u8 	dtim;
	u8	ps_processing; /* temporarily used to mark whether in rpwm processing */

	u8	bpower_saving; //for LPS/IPS
	u8	bFwCurrentInPSMode;
	u8	bLeisurePs;
	u8	LpsIdleCount;
	// ps_deny: if 0, power save is free to go; otherwise deny all kinds of power save.
	// Use PS_DENY_REASON to decide reason.
	// Don't access this variable directly without control function,
	// and this variable should be protected by lock.
	u32 ps_deny;
};

#define rtw_get_ips_mode_req(pwrctl) \
	(pwrctl)->ips_mode_req

#define rtw_ips_mode_req(pwrctl, ips_mode) \
	(pwrctl)->ips_mode_req = (ips_mode)

#define RTW_PWR_STATE_CHK_INTERVAL 2000

#define _rtw_set_pwr_state_check_timer(pwrctl, ms) \
	do { \
		/*DBG_871X("%s _rtw_set_pwr_state_check_timer(%p, %d)\n", __FUNCTION__, (pwrctl), (ms));*/ \
		rtw_set_timer(&(pwrctl)->pwr_state_check_timer, (ms)); \
	} while(0)
	
#define rtw_set_pwr_state_check_timer(pwrctl) \
	_rtw_set_pwr_state_check_timer((pwrctl), (pwrctl)->pwr_state_check_interval)

extern void rtw_init_pwrctrl_priv(_adapter *adapter);
extern void rtw_free_pwrctrl_priv(_adapter * adapter);

extern void rtw_set_ps_mode(PADAPTER padapter, u8 ps_mode);
extern void rtw_set_rpwm(_adapter * padapter, u8 val8);
extern void LeaveAllPowerSaveMode(PADAPTER Adapter);
extern void LeaveAllPowerSaveModeDirect(PADAPTER Adapter);

void rtw_ps_processor(_adapter*padapter);




void LPS_Enter(PADAPTER padapter);
void LPS_Leave(PADAPTER padapter);
void	traffic_check_for_leave_lps(PADAPTER padapter, u8 tx, u32 tx_packets);
void _dynamic_check_timer_handlder (void *FunctionContext);
u8 rtw_dynamic_chk_wk_cmd(_adapter*padapter);

//u8 rtw_interface_ps_func(_adapter *padapter,HAL_INTF_PS_FUNC efunc_id,u8* val);
void rtw_set_ips_deny(_adapter *padapter, u32 ms);
int _rtw_pwr_wakeup(_adapter *padapter, u32 ips_deffer_ms, const char *caller);
#define rtw_pwr_wakeup(adapter) _rtw_pwr_wakeup(adapter, RTW_PWR_STATE_CHK_INTERVAL, __FUNCTION__)
#define rtw_pwr_wakeup_ex(adapter, ips_deffer_ms) _rtw_pwr_wakeup(adapter, ips_deffer_ms, __FUNCTION__)
int rtw_pm_set_ips(_adapter *padapter, u8 mode);
int rtw_pm_set_lps(_adapter *padapter, u8 mode);

void rtw_ps_deny(PADAPTER padapter, PS_DENY_REASON reason);
void rtw_ps_deny_cancel(PADAPTER padapter, PS_DENY_REASON reason);
u32 rtw_ps_deny_get(PADAPTER padapter);
u8 PS_RDY_CHECK(_adapter * padapter);
int rtw_ps_setting(PADAPTER padapter, int enable);
#ifdef CONFIG_WOWLAN
s32 rtw_configure_wowlan(_adapter *padapter, struct cfg80211_wowlan *wowlan, u32 IPAddr);
#endif
#endif  //__RTL871X_PWRCTRL_H_


