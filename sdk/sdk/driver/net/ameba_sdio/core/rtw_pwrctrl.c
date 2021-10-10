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
#define _RTW_PWRCTRL_C_

#include <drv_types.h>

#ifdef CONFIG_POWER_SAVING
void pwr_state_check_handler(RTW_TIMER_HDL_ARGS)
{
//	_adapter *padapter = (_adapter *)FunctionContext;
//	rtw_ps_cmd(padapter);
}

void rtw_init_pwrctrl_priv(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);

_func_enter_;

	_init_pwrlock(&pwrctrlpriv->lock);
//	_init_pwrlock(&pwrctrlpriv->check_32k_lock);
	pwrctrlpriv->LpsIdleCount = 0;
	pwrctrlpriv->bLeisurePs = _TRUE;

	pwrctrlpriv->bFwCurrentInPSMode = _FALSE;

	pwrctrlpriv->rpwm2 = 0;
	pwrctrlpriv->cpwm2 = 0;

	pwrctrlpriv->ps_enable = 1;

	pwrctrlpriv->pwr_mode = PS_MODE_ACTIVE;

//	pwrctrlpriv->smart_ps = padapter->registrypriv.smart_ps;
//	pwrctrlpriv->bcn_ant_mode = 0;
	pwrctrlpriv->dtim = 0;
	pwrctrlpriv->ps_processing = _FALSE;
//	pwrctrlpriv->tog = 0x80;

//	rtw_init_timer(&pwrctrlpriv->pwr_state_check_timer, padapter, pwr_state_check_handler, padapter, "pwr_rpwm_timer");

_func_exit_;

}


void rtw_free_pwrctrl_priv(PADAPTER adapter)
{
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(adapter);

_func_enter_;

//	rtw_cancel_timer(&pwrctrlpriv->pwr_state_check_timer);
	_free_pwrlock(&pwrctrlpriv->lock);
//	_free_pwrlock(&pwrctrlpriv->check_32k_lock);

_func_exit_;
}

#ifdef CONFIG_PS_DYNAMIC_CHK
void rtw_dynamic_check_timer_handlder(_adapter *adapter)
{
	if(!adapter)
		return;	

	if(adapter->hw_init_completed == _FALSE)
		return;

	if ((adapter->bDriverStopped == _TRUE)||(adapter->bSurpriseRemoved== _TRUE))
		return;

	if(adapter->net_closed == _TRUE)
	{
		return;
	}	
	rtw_dynamic_chk_wk_cmd(adapter);			
}

void _dynamic_check_timer_handlder (void *FunctionContext)
{
	_adapter *adapter = (_adapter *)FunctionContext;

#ifdef DBG_POWER_SAVING
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(adapter);
	DBG_871X("%s: ps mode = %s busy = %s\n", __func__, pwrpriv->pwr_mode?"SLEEP":"ACTIVE", 
		adapter->LinkDetectInfo.bBusyTraffic?"YES":"NO");
#endif

	rtw_dynamic_check_timer_handlder(adapter);
	
	rtw_set_timer(&adapter->dynamic_chk_timer, DYNAMIC_CHK_TMR_INTERVAL);
}
#endif

/*
 * This function should be called for data transfer
 * Active FW before send/receive data
 */
void rtw_set_ps_mode(PADAPTER padapter, u8 ps_mode)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);

_func_enter_;

	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("%s: PowerMode=%d\n",
			  __FUNCTION__, ps_mode));

	_enter_pwrlock(&pwrpriv->lock);

	if(pwrpriv->ps_enable == 0)
		goto exit;	
	
	if (pwrpriv->pwr_mode == ps_mode)
	{
		//if (PS_MODE_ACTIVE == ps_mode)
			goto exit;
	}

	if(ps_mode == PS_MODE_ACTIVE)
	{
#ifdef DBG_POWER_SAVING
			DBG_871X("rtw_set_ps_mode: Leave 802.11 power save\n");
#endif
			//_enter_pwrlock(&pwrpriv->lock);
			rtw_hal_rpwm_notify(padapter, RPWM2_PWR_ACT);			
			pwrpriv->bFwCurrentInPSMode = _FALSE;
			pwrpriv->pwr_mode = ps_mode;
			//_exit_pwrlock(&pwrpriv->lock);
	}
	else
	{
#ifdef CONFIG_PS_DYNAMIC_CHK
		if(rtw_get_bBusyTraffic(padapter))
			goto exit;
#endif	
		if(PS_RDY_CHECK(padapter))
		{
#ifdef DBG_POWER_SAVING
			DBG_871X("rtw_set_ps_mode: Enter 802.11 power save\n");
#endif
			//_enter_pwrlock(&pwrpriv->lock);
			rtw_hal_rpwm_notify(padapter, RPWM2_PWR_CG);
			pwrpriv->bFwCurrentInPSMode = _TRUE;
			pwrpriv->pwr_mode = ps_mode;
			//_exit_pwrlock(&pwrpriv->lock);
		}
	}
_func_exit_;
exit:
	_exit_pwrlock(&pwrpriv->lock);
	return;
}

u8 PS_RDY_CHECK(_adapter * padapter)
{

	struct pwrctrl_priv	*pwrpriv = adapter_to_pwrctl(padapter);
	u32 ps_deny = 0;
	u8 ps_ready = _FALSE;

	do{
		if ((check_fwstate(padapter, WIFI_AP_STATE) == _TRUE))
			break;

		//_enter_pwrlock(&adapter_to_pwrctl(padapter)->lock);
		ps_deny = rtw_ps_deny_get(padapter);
		//_exit_pwrlock(&adapter_to_pwrctl(padapter)->lock);
		if (ps_deny != 0)
		{
#ifdef DBG_POWER_SAVING
			DBG_871X(FUNC_ADPT_FMT ": ps_deny=0x%08X, skip power save!\n",
				FUNC_ADPT_ARG(padapter), ps_deny);
#endif
			break;
		}
		
		if(pwrpriv->ps_processing == _TRUE)
			break;

		//all condition matched, ready to enter sleep mode
		ps_ready = _TRUE;
	}while(0);

	return ps_ready;
}


//
//	Description:
//		Enter the leisure power save mode.
//
void LPS_Enter(PADAPTER padapter)
{
	struct pwrctrl_priv	*pwrpriv = adapter_to_pwrctl(padapter);

_func_enter_;

	if(pwrpriv->ps_enable == 0)
		return;

	_enter_pwrlock(&pwrpriv->lock);
	if (PS_RDY_CHECK(padapter) == _FALSE){
		_exit_pwrlock(&pwrpriv->lock);
		return;
	}
	_exit_pwrlock(&pwrpriv->lock);
	
#ifdef DBG_POWER_SAVING
	DBG_871X("+LeisurePSEnter\n");
#endif

	if (pwrpriv->bLeisurePs)
	{
		// Idle for a while if we connect to AP a while ago.
		if(pwrpriv->LpsIdleCount >= 2) //  4 Sec 
		{

			if(pwrpriv->pwr_mode == PS_MODE_ACTIVE)
			{
			#if 0
				rtw_hal_rpwm_notify(padapter, RPWM2_PWR_CG);
				_enter_pwrlock(&pwrpriv->lock);
				pwrpriv->bFwCurrentInPSMode = _TRUE;
				pwrpriv->pwr_mode = PS_MODE_SLEEP;
				pwrpriv->bpower_saving = _TRUE;
				_exit_pwrlock(&pwrpriv->lock);
			#else
				rtw_set_ps_mode(padapter, PS_MODE_SLEEP);
				pwrpriv->bpower_saving = _TRUE;
				pwrpriv->LpsIdleCount = 0;
			#endif
			}
		}
		else
			pwrpriv->LpsIdleCount++;
	}

#ifdef DBG_POWER_SAVING
	DBG_871X("-LeisurePSEnter\n");
#endif

_func_exit_;
}


//
//	Description:
//		Leave the leisure power save mode.
//
void LPS_Leave(PADAPTER padapter)
{

	struct pwrctrl_priv	*pwrpriv = adapter_to_pwrctl(padapter);

_func_enter_;

#ifdef DBG_POWER_SAVING
	DBG_871X("+LeisurePSLeave\n");
#endif
	
	if (pwrpriv->bLeisurePs)
	{
		if(pwrpriv->pwr_mode != PS_MODE_ACTIVE)
		{
		#if 0
			rtw_hal_rpwm_notify(padapter, RPWM2_PWR_ACT);
			_enter_pwrlock(&pwrpriv->lock);
			pwrpriv->bFwCurrentInPSMode = _FALSE;
			pwrpriv->pwr_mode = PS_MODE_ACTIVE;
			_exit_pwrlock(&pwrpriv->lock);
		#else
			rtw_set_ps_mode(padapter, PS_MODE_ACTIVE);
			pwrpriv->bpower_saving = _FALSE;
			pwrpriv->LpsIdleCount = 0;
		#endif
		}
	}

	pwrpriv->bpower_saving = _FALSE;

#ifdef DBG_POWER_SAVING
	DBG_871X("-LeisurePSLeave\n");
#endif

_func_exit_;
	
}

//
// Description: Leave all power save mode: LPS, FwLPS, IPS if needed.
// Move code to function by tynli. 2010.03.26. 
//
void LeaveAllPowerSaveMode(PADAPTER Adapter)
{
	struct pwrctrl_priv	*pwrpriv = adapter_to_pwrctl(Adapter);

_func_enter_;
	rtw_hal_rpwm_notify(Adapter, RPWM2_PWR_ACT);
	_enter_pwrlock(&pwrpriv->lock);
	pwrpriv->bFwCurrentInPSMode = _FALSE;
	pwrpriv->pwr_mode = PS_MODE_ACTIVE;
	pwrpriv->bpower_saving = _FALSE;
	_exit_pwrlock(&pwrpriv->lock);
_func_exit_;
}

/*
 * ATTENTION:
 *	This function will request pwrctrl LOCK!
 */
void rtw_ps_deny(PADAPTER padapter, PS_DENY_REASON reason)
{
	struct pwrctrl_priv *pwrpriv;

//	DBG_871X("+" FUNC_ADPT_FMT ": Request PS deny for %d (0x%08X)\n",
//		FUNC_ADPT_ARG(padapter), reason, BIT(reason));

	pwrpriv = adapter_to_pwrctl(padapter);

	_enter_pwrlock(&pwrpriv->lock);
	if (pwrpriv->ps_deny & BIT(reason))
	{
		DBG_871X(FUNC_ADPT_FMT ": [WARNING] Reason %d had been set before!!\n",
			FUNC_ADPT_ARG(padapter), reason);
	}
	pwrpriv->ps_deny |= BIT(reason);
	_exit_pwrlock(&pwrpriv->lock);

//	DBG_871X("-" FUNC_ADPT_FMT ": Now PS deny for 0x%08X\n",
//		FUNC_ADPT_ARG(padapter), pwrpriv->ps_deny);
}

/*
 * ATTENTION:
 *	This function will request pwrctrl LOCK!
 */
void rtw_ps_deny_cancel(PADAPTER padapter, PS_DENY_REASON reason)
{
	struct pwrctrl_priv *pwrpriv;


//	DBG_871X("+" FUNC_ADPT_FMT ": Cancel PS deny for %d(0x%08X)\n",
//		FUNC_ADPT_ARG(padapter), reason, BIT(reason));

	pwrpriv = adapter_to_pwrctl(padapter);

	_enter_pwrlock(&pwrpriv->lock);
	if ((pwrpriv->ps_deny & BIT(reason)) == 0)
	{
		DBG_871X(FUNC_ADPT_FMT ": [ERROR] Reason %d had been canceled before!!\n",
			FUNC_ADPT_ARG(padapter), reason);
	}
	pwrpriv->ps_deny &= ~BIT(reason);
	_exit_pwrlock(&pwrpriv->lock);

//	DBG_871X("-" FUNC_ADPT_FMT ": Now PS deny for 0x%08X\n",
//		FUNC_ADPT_ARG(padapter), pwrpriv->ps_deny);
}

/*
 * ATTENTION:
 *	Before calling this function pwrctrl lock should be occupied already,
 *	otherwise it may return incorrect value.
 */
u32 rtw_ps_deny_get(PADAPTER padapter)
{
	u32 deny;


	deny = adapter_to_pwrctl(padapter)->ps_deny;

	return deny;
}

int rtw_ps_setting(PADAPTER padapter, int enable)
{	
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);

	if(enable){
		
		if(pwrctrlpriv->ps_enable)
			goto exit;
		
		_enter_pwrlock(&pwrctrlpriv->lock);
		pwrctrlpriv->ps_enable = 1;
		_exit_pwrlock(&pwrctrlpriv->lock);
#ifdef CONFIG_PS_DYNAMIC_CHK		
		rtw_set_timer(&padapter->dynamic_chk_timer, DYNAMIC_CHK_TMR_INTERVAL);
#endif
	}else{

		if(pwrctrlpriv->ps_enable == 0)
			goto exit;
		
		_enter_pwrlock(&pwrctrlpriv->lock);
		pwrctrlpriv->ps_enable = 0;
		_exit_pwrlock(&pwrctrlpriv->lock);
#ifdef CONFIG_PS_DYNAMIC_CHK		
		rtw_cancel_timer(&padapter->dynamic_chk_timer);
#endif
		LeaveAllPowerSaveMode(padapter);
	}
	
exit:
	return 0;
}

#ifdef CONFIG_WOWLAN
static s32 rtw_wowlan_send_patterns(_adapter *padapter, struct cfg80211_wowlan *wowlan)
{
	struct rtw_wowlan_patterns_cmd *pattern_cmd;
	s32 i, len, n_patterns;
	u8 *cmd_data;

	if (!wowlan->n_patterns)
		return _SUCCESS;
	
	n_patterns = wowlan->n_patterns;
	len = sizeof(*pattern_cmd) + n_patterns * sizeof(struct rtw_wowlan_pattern);
	cmd_data = rtw_zmalloc(len);
	if (!cmd_data)
		return _FAIL;
	
	pattern_cmd = (struct rtw_wowlan_patterns_cmd *)cmd_data;

	pattern_cmd->n_patterns = cpu_to_le32(n_patterns);

	for (i = 0; i < n_patterns; i++) {
		int mask_len = DIV_ROUND_UP(wowlan->patterns[i].pattern_len, 8);

		rtw_memcpy(&pattern_cmd->patterns[i].mask,
		       wowlan->patterns[i].mask, mask_len);
		rtw_memcpy(&pattern_cmd->patterns[i].pattern,
		       wowlan->patterns[i].pattern,
		       wowlan->patterns[i].pattern_len);
		pattern_cmd->patterns[i].mask_size = mask_len;
		pattern_cmd->patterns[i].pattern_size =
			wowlan->patterns[i].pattern_len;
	}
	rtw_hal_send_wowlan_cmd(padapter, RTW_WOWLAN_CMD_PATTERNS, cmd_data, len);
	rtw_mfree(cmd_data, len);
	return _SUCCESS;
}

static s32 rtw_wowlan_send_proto_offload(_adapter *padapter, u32 IPAddr)
{
	struct rtw_proto_offload_cmd_common *common;
	u32 enabled = 0, len;
	u8 *cmd_data;

	if(!IPAddr)
		return _SUCCESS;
	
	len = sizeof(*common);
	cmd_data = rtw_zmalloc(len);
	if (!cmd_data){
		DBG_871X("malloc failed for proto offload!\n");
		return _FAIL;
	}

	common = (struct rtw_proto_offload_cmd_common *)cmd_data;

	enabled = RTW_WOWLAN_PROTO_OFFLOAD_ARP;

	common->host_ipv4_addr = IPAddr;

	rtw_memcpy(common->arp_mac_addr, padapter->mac_addr, ETH_ALEN);

	common->enabled = cpu_to_le32(enabled);

	rtw_hal_send_wowlan_cmd(padapter, RTW_WOWLAN_CMD_PROT_OFFLOAD_CONFIG, cmd_data, len);
	rtw_mfree(cmd_data, len);
	return _SUCCESS;
}

static int rtw_wowlan_send_cfg(_adapter *padapter, struct cfg80211_wowlan *wowlan){
	struct rtw_wowlan_config_cmd *wowlan_config_cmd = NULL;
	u32 len;
	u8 *cmd_data;
	
	len = sizeof(*wowlan_config_cmd);
	cmd_data = rtw_zmalloc(len);
	if (!cmd_data)
		return _FAIL;

	wowlan_config_cmd = (struct rtw_wowlan_config_cmd *)cmd_data;

	if (wowlan->disconnect)
		wowlan_config_cmd->wakeup_filter |=
			cpu_to_le32(RTW_WOWLAN_WAKEUP_BEACON_MISS |
				    RTW_WOWLAN_WAKEUP_LINK_CHANGE);
	if (wowlan->magic_pkt)
		wowlan_config_cmd->wakeup_filter |=
			cpu_to_le32(RTW_WOWLAN_WAKEUP_MAGIC_PACKET);
	if (wowlan->gtk_rekey_failure)
		wowlan_config_cmd->wakeup_filter |=
			cpu_to_le32(RTW_WOWLAN_WAKEUP_GTK_REKEY_FAIL);
	if (wowlan->eap_identity_req)
		wowlan_config_cmd->wakeup_filter |=
			cpu_to_le32(RTW_WOWLAN_WAKEUP_EAP_IDENT_REQ);
	if (wowlan->four_way_handshake)
		wowlan_config_cmd->wakeup_filter |=
			cpu_to_le32(RTW_WOWLAN_WAKEUP_4WAY_HANDSHAKE);
	if (wowlan->n_patterns)
		wowlan_config_cmd->wakeup_filter |=
			cpu_to_le32(RTW_WOWLAN_WAKEUP_PATTERN_MATCH);

	if (wowlan->rfkill_release)
		wowlan_config_cmd->wakeup_filter |=
			cpu_to_le32(RTW_WOWLAN_WAKEUP_RF_KILL_DEASSERT);

	if (wowlan->tcp) {
		/*
		 * Set the "link change" (really "link lost") flag as well
		 * since that implies losing the TCP connection.
		 */
		wowlan_config_cmd->wakeup_filter |=
			cpu_to_le32(RTW_WOWLAN_WAKEUP_REMOTE_LINK_LOSS |
				    RTW_WOWLAN_WAKEUP_REMOTE_SIGNATURE_TABLE |
				    RTW_WOWLAN_WAKEUP_REMOTE_WAKEUP_PACKET |
				    RTW_WOWLAN_WAKEUP_LINK_CHANGE);
	}
	rtw_hal_send_wowlan_cmd(padapter, RTW_WOWLAN_CMD_CFG, cmd_data, len);
	rtw_mfree(cmd_data, len);
	return _SUCCESS;
}

s32 rtw_configure_wowlan(_adapter *padapter, struct cfg80211_wowlan *wowlan, u32 IPAddr){
	int ret;
	
	ret = rtw_wowlan_send_patterns(padapter, wowlan);
	if (ret != _SUCCESS)
		goto out;
	
	ret = rtw_wowlan_send_proto_offload(padapter, IPAddr);
	if (ret != _SUCCESS)
		goto out;

	ret = rtw_wowlan_send_cfg(padapter, wowlan);
	if (ret != _SUCCESS)
		goto out;
	
out:
	return ret;
}
#endif
#endif
