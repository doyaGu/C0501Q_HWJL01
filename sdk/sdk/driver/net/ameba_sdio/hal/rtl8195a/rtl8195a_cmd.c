/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
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
#define _RTL8195A_CMD_C_

#include <drv_types.h>
#include <rtw_debug.h>
#include <rtl8195a_cmd.h>
#include <rtw_io.h>
#ifdef PLATFORM_FREERTOS
#include <freertos/generic.h>
#endif

/**************************************************************************
  *****Response Formats****************
  ****
 ------------------------------------------------------------------------
 |Delimiter(2bytes) | CMD(4 bytes) |Delimiter(2bytes) | Return(1 bytes) | Delimeter(2bytes) | payload
 ------------------------------------------------------------------------
 |\r\n                      | ATCMD           |\r\n                      | 0(OK)                | \r\n                       |Data
 |\r\n                      | ATCMD           |\r\n                      | ERRNO              | \r\n                       |
 ------------------------------------------------------------------------
  ****
***************************************************************************/
//----------------------------------------------------------------------------//
#ifndef WLAN0_NAME
  #define WLAN0_NAME		"wlan0"
#endif
#ifndef WLAN1_NAME
  #define WLAN1_NAME		"wlan1"
#endif

#if 1//todo
static void rtw_wep_key_convert(u8 *password, u8 password_len){
	int i = 0;

	if(password_len == 5){
		u8 p[11];
		for(i=0; i<password_len; i++){
			p[2*i] = *(password+i)/0x10;
			p[2*i+1] = *(password+i)%0x10;
		}
		for(i=0;i<10;i++){
			if(p[i]<=0x9)
				p[i] += 0x30;
			else if((p[i]>=0xa)&&(p[i]<=0xf))
				p[i] += 0x57;
		}
		p[10] = '\0';
		printk("  PASSWORD => %s\n", p);
	}
	else if(password_len == 13){
		u8 p[27];
		for(i=0; i<password_len; i++){
			p[2*i] = *(password+i)/0x10;
			p[2*i+1] = *(password+i)%0x10;
		}
		for(i=0;i<26;i++){
			if(p[i]<=0x9)
				p[i] += 0x30;
			else if((p[i]>=0xa)&&(p[i]<=0xf))
				p[i] += 0x57;
		}
		p[26] = '\0';
		printk("  PASSWORD => %s\n", p);
	}	
	else
		DBG_871X("Invalid password length for WEP: %d\n", password_len);
}
#endif
static int rtw_show_wifi_info(const char *ifname, u8 *info)
{
	int ret = 0;
	rtw_wifi_setting_t *pSetting = (rtw_wifi_setting_t *)info;
	
	printk("WIFI  %s Setting:\n\r",ifname);
	printk("==============================\n\r");

	switch(pSetting->mode) {
		case RTW_MODE_AP:
			printk("      MODE => AP\n\r");
			break;
		case RTW_MODE_STA:
			printk("      MODE => STATION\n\r");
			break;
		default:
			printk("      MODE => UNKNOWN\n\r");
	}

	printk("      SSID => %s\n\r", pSetting->ssid);
	printk("   CHANNEL => %d\n\r", pSetting->channel);

	switch(pSetting->security_type) {
		case RTW_SECURITY_OPEN:
			printk("  SECURITY => OPEN\n\r");
			break;
		case RTW_SECURITY_WEP_PSK:
			printk("  SECURITY => WEP\n\r");
			printk("  KEY INDEX => %d\n\r", pSetting->key_idx);
			break;
		case RTW_SECURITY_WPA_TKIP_PSK:
			printk("  SECURITY => TKIP\n\r");
			break;
		case RTW_SECURITY_WPA2_AES_PSK:
			printk("  SECURITY => AES\n\r");
			break;
		default:
			printk("  SECURITY => UNKNOWN\n\r");
	}
	//without the conversion, password can't be print properly	
	if(pSetting->security_type == RTW_SECURITY_WEP_PSK)
		rtw_wep_key_convert(pSetting->password, strlen((char *)(pSetting->password)));
	else
		printk("  PASSWORD => %s\n\r", pSetting->password);

#if defined(CONFIG_LWIP_LAYER) && (CONFIG_LWIP_LAYER == 1) && defined(PLATFORM_FREERTOS)
	rtw_os_show_ip_and_mac();
#endif

	printk("\r\n");

	return ret;
}

/*function to print scan result*/
__inline static void rtw_print_scan_result(char *result)
{
	rtw_scan_result_t* record =  (rtw_scan_result_t*) result;
//	DumpForOneBytes(result, sizeof(rtw_scan_result_t));
	record->SSID.val[record->SSID.len] = 0; /* Ensure the SSID is null terminated */
	printk("%s\t ", (record->bss_type == RTW_BSS_TYPE_ADHOC ) ? "Adhoc" : "Infra" );
	printk(MAC_FMT, MAC_ARG(record->BSSID.octet));
	printk(" %d\t ", record->signal_strength);
	printk(" %d\t  ", (u8)record->channel);
	printk(" %d\t  ", record->wps_type);
	printk("%s\t\t ", ( record->security == RTW_SECURITY_OPEN ) ? "Open" :
					( record->security == RTW_SECURITY_WEP_PSK ) ? "WEP" :
					( record->security == RTW_SECURITY_WPA_TKIP_PSK ) ? "WPA TKIP" :
					( record->security == RTW_SECURITY_WPA_AES_PSK ) ? "WPA AES" :
					( record->security == RTW_SECURITY_WPA2_AES_PSK ) ? "WPA2 AES" :
					( record->security == RTW_SECURITY_WPA2_TKIP_PSK ) ? "WPA2 TKIP" :
					( record->security == RTW_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed" :
					( record->security == RTW_SECURITY_WPA_WPA2_MIXED ) ? "WPA/WPA2 AES" :
					"Unknown" );

	printk(" %s ", record->SSID.val);
	printk("\r\n");
}

static s32 rtw_ATWA_handler(PADAPTER padapter, char status, u8 *rsp_msg, u16 sz){
	s32 ret = _SUCCESS;
	_irqL irql;
	
	rtw_wifi_setting_t *pSetting = (rtw_wifi_setting_t *)rsp_msg;
	if(status == RTW_SUCCESS)
	{
		if(sz >= sizeof(rtw_wifi_setting_t)){
			rtw_show_wifi_info((const char *)WLAN0_NAME, rsp_msg);
			DBG_871X("[%s] started!\n", pSetting->ssid);
			
			if(check_fwstate(padapter, WIFI_AP_STATE) == _FALSE){
				rtw_enter_critical_bh(&padapter->lock, &irql);
				set_fwstate(padapter, WIFI_AP_STATE);
				rtw_exit_critical_bh(&padapter->lock, &irql);
				rtw_os_indicate_connect(padapter->pnetdev);
			}
		}
		else if(rtw_memcmp(IW_EVT_STR_STA_ASSOC, rsp_msg, strlen(IW_EVT_STR_STA_ASSOC)) == _TRUE){
			u8 mac[ETH_ALEN] = {0};
			rtw_memcpy(mac, rsp_msg+strlen(IW_EVT_STR_STA_ASSOC), ETH_ALEN);
			DBG_871X("STA associated: "MAC_FMT"\n", MAC_ARG(mac));
			rtw_indicate_sta_assoc_event(padapter->pnetdev, mac);
		}
		else if(rtw_memcmp(IW_EVT_STR_STA_DISASSOC, rsp_msg, strlen(IW_EVT_STR_STA_DISASSOC)) == _TRUE){
			u8 mac[ETH_ALEN] = {0};
			rtw_memcpy(mac, rsp_msg+strlen(IW_EVT_STR_STA_DISASSOC), ETH_ALEN);
			DBG_871X("STA disassociated: "MAC_FMT"\n", MAC_ARG(mac));
			rtw_indicate_sta_disassoc_event(padapter->pnetdev, mac);
		}
	}
	else
		DBG_871X("ATWA: FAILED![errno:%d]\n", status);
	
	return ret;
}

//not supported yet
static s32 rtw_ATWB_handler(PADAPTER padapter, char status, u8 *rsp_msg, u16 sz){
	s32 ret = _SUCCESS;
	rtw_wifi_setting_t wifi_setting[2];
	char *ifname[2] = {WLAN0_NAME,WLAN1_NAME};
	int i;
	
	if(status == RTW_SUCCESS)
	{
		for(i=0;i<2;i++){
			rtw_memcpy(&wifi_setting[i], rsp_msg+(i*sizeof(rtw_wifi_setting_t)), sizeof(rtw_wifi_setting_t));
			rtw_show_wifi_info((const char *)ifname[i], (u8 *)&wifi_setting[i]);
		}
		if(wifi_setting[0].ssid[0] != 0)//STA
		{
			padapter->fw_status = 1;
			rtw_os_indicate_connect(padapter->pnetdev);	
		}
		if(wifi_setting[1].ssid[0] != 0)//AP
		{
			DBG_871X("[%s] started!\n", wifi_setting[1].ssid);
		}
	}
	else
		DBG_871X("ATWB: FAILED![errno:%d]\n", status);
	
	return ret;
}

static s32 rtw_ATWC_handler(PADAPTER padapter, char status, u8 *rsp_msg, u16 sz){
	s32 ret = _SUCCESS;
	_irqL irql;
	
	rtw_wifi_setting_t *pSetting = (rtw_wifi_setting_t *)rsp_msg;
	if((status == RTW_SUCCESS)&&(pSetting->ssid[0] != 0))
	{
		if(sz >= sizeof(rtw_wifi_setting_t))
			rtw_show_wifi_info((const char *)WLAN0_NAME, rsp_msg);		
		if(check_fwstate(padapter, WIFI_ASOC_STATE) == _FALSE){
			rtw_enter_critical_bh(&padapter->lock, &irql);
			set_fwstate(padapter, WIFI_ASOC_STATE | WIFI_STATION_STATE);
			rtw_exit_critical_bh(&padapter->lock, &irql);
			rtw_os_indicate_connect(padapter->pnetdev);
		}	
	}
	else
		DBG_871X("ATWC: FAILED![errno:%d]\n", status);
	
	return ret;
}

static s32 rtw_ATWD_handler(PADAPTER padapter, char status, u8 *rsp_msg, u16 sz){
	s32 ret = _SUCCESS;
	if(status == RTW_SUCCESS)
	{
		if(check_fwstate(padapter, WIFI_ASOC_STATE|WIFI_AP_STATE) == _TRUE)
		{
			reset_fwstate(padapter);
			DBG_871X("network becomes disconnected!\n");
			rtw_os_indicate_disconnect(padapter->pnetdev);
		}
#ifdef CONFIG_POWER_SAVING
		padapter->LinkDetectInfo.bBusyTraffic = _FALSE;
#endif
	}
	else
		DBG_871X("ATWD: FAILED![errno:%d]\n", status);
	
	return ret;
}

static s32 rtw_ATWM_handler(PADAPTER padapter, char status, u8 *rsp_msg, u16 sz){
	s32 ret = _SUCCESS;
	rtw_eth_frame_t *frame = NULL;
	int cnt = sz/ (sizeof(rtw_eth_frame_t));
	static int channel = 1;
	int i, j;
	if(status == RTW_SUCCESS)
	{
		DBG_871X("Switch to channel(%d), packet received(%d)\n", channel, cnt);
		channel++;
		if(channel > 13)
			channel = 1;
		if(cnt){
			frame = (rtw_eth_frame_t *)rsp_msg;
			for(i = 0; i<cnt; i++ ){
				printk("TYPE: 0x%x, ", frame->type);
				printk("DA:");
				for(j = 0; j < 6; j ++)
					printk(" %02x", frame->da[j]);
				printk(", SA:");
				for(j = 0; j < 6; j ++)
					printk(" %02x", frame->sa[j]);
				printk(", len=%d\n", frame->len);
				frame++;
			}
		}
	}
	else
		DBG_871X("ATWM: FAILED![errno:%d]\n", status);
	
	return ret;
}

u32 ApNumAll;
static s32 rtw_ATWS_handler(PADAPTER padapter, char status, u8 *rsp_msg, u16 sz){
	s32 ret = _SUCCESS;
	int i, ApNum = 0;
	
	if(status == RTW_SUCCESS)
	{
		ApNum = sz/sizeof(rtw_scan_result_t);
		for(i=0; i<ApNum; i++){
#ifdef PLATFORM_ECOS
			rtw_os_indicate_scan_result(padapter->pnetdev, _SUCCESS, (char *)(rsp_msg+i*sizeof(rtw_scan_result_t)));
#else
			DBG_871X("%d\t ", ApNumAll + i+1);
			rtw_print_scan_result((char *)(rsp_msg+i*sizeof(rtw_scan_result_t)));
#endif
		}
		ApNumAll = ApNum + ApNumAll;
		if(ApNum == 0) {
#ifdef PLATFORM_ECOS
			rtw_os_indicate_scan_result(padapter->pnetdev, _SUCCESS, NULL);
#else
			ApNumAll = 0;
			DBG_871X("====<Scan done!>====\n");
#endif
		}
	}
	else{
#ifdef PLATFORM_ECOS
		rtw_os_indicate_scan_result(padapter->pnetdev, _FAIL, NULL);
#else
		DBG_871X("ATWS: FAILED![errno:%d]\n", status);
#endif
	}
	return ret;
}

static s32 rtw_ATWQ_handler(PADAPTER padapter, char status, u8 *rsp_msg, u16 sz){
	_irqL irql;
	s32 ret = _SUCCESS;
	unsigned char wsc_ip[4] = {0};
	int wsc_des_ip_addr = 0;
	rtw_wifi_setting_t *pSetting = (rtw_wifi_setting_t *)rsp_msg;
	
	if((status == RTW_SUCCESS)&&(pSetting->ssid[0] != 0))
	{

		if(sz >= sizeof(rtw_wifi_setting_t))
			rtw_show_wifi_info((const char *)WLAN0_NAME, rsp_msg);

		rtw_memcpy(wsc_ip, (char *)(rsp_msg+sizeof(rtw_wifi_setting_t)), sizeof(wsc_ip));
		DBG_871X("Simple config: please send ack back to %d.%d.%d.%d\n", wsc_ip[0], wsc_ip[1], wsc_ip[2], wsc_ip[3]);

		if(check_fwstate(padapter, WIFI_ASOC_STATE) == _FALSE){
			rtw_enter_critical_bh(&padapter->lock, &irql);
			set_fwstate(padapter, WIFI_ASOC_STATE | WIFI_WSC_STATE);
			rtw_exit_critical_bh(&padapter->lock, &irql);
			rtw_os_indicate_connect(padapter->pnetdev);
			wsc_des_ip_addr = wsc_ip[0] | (wsc_ip[1] << 8) | (wsc_ip[2] << 16) 
				| (wsc_ip[3] << 24);
			rtw_os_send_simple_config_ack(padapter->pnetdev, cpu_to_le32(wsc_des_ip_addr));
		}
	}
	else{
		DBG_871X("ATWQ: FAILED![errno:%d]\n", status);
		rtw_os_send_simple_config_ack(padapter->pnetdev, 0);
	}
	
	return ret;
}

static s32 rtw_ATWZ_handler(PADAPTER padapter, char status, u8 *rsp_msg, u16 sz){
	s32 ret = _SUCCESS;
#ifdef CONFIG_USB_HCI
	PUSB_DATA pusb = &padapter->dvobj->intf_data;
	
	pusb->read_mac_done = 1;
	rtw_memcpy(pusb->mac, rsp_msg, 20);
#endif
	DBG_871X("Private Message: %s\n", (char *)rsp_msg);

	return ret;
}

static s32 rtw_atw_info_handler(PADAPTER padapter, char status, u8 *rsp_msg, u16 sz){
	s32 ret = _SUCCESS;
	int num_device = 0;
	int i, num_client = 0;
	char *ifname[2] = {WLAN0_NAME,WLAN1_NAME};
	u8 *pSetting = rsp_msg;
	u16 info_sz = sz;
	rtw_wifi_setting_t* wifi_setting;
	rtw_mac_t *mac;
	_irqL irql;
	
	if(status == RTW_SUCCESS)
	{
		while(info_sz >= sizeof(rtw_wifi_setting_t)){
			wifi_setting = (rtw_wifi_setting_t *)pSetting;
			rtw_show_wifi_info((char *)ifname[num_device], pSetting);
			if((wifi_setting->mode == RTW_MODE_STA) && (wifi_setting->ssid[0]!=0)){
				if(check_fwstate(padapter, WIFI_ASOC_STATE) == _FALSE){
					rtw_enter_critical_bh(&padapter->lock, &irql);
					set_fwstate(padapter, WIFI_ASOC_STATE | WIFI_STATION_STATE);
					rtw_exit_critical_bh(&padapter->lock, &irql);
				}
			}
			else if((wifi_setting->mode == RTW_MODE_AP) && (wifi_setting->ssid[0]!=0)){
				if(check_fwstate(padapter, WIFI_AP_STATE) == _FALSE){
					rtw_enter_critical_bh(&padapter->lock, &irql);
					set_fwstate(padapter, WIFI_AP_STATE);
					rtw_exit_critical_bh(&padapter->lock, &irql);
				}
			}
			rtw_os_indicate_wifi_info(padapter->pnetdev, (char *)wifi_setting);
			pSetting += sizeof(rtw_wifi_setting_t);		
			info_sz -= sizeof(rtw_wifi_setting_t);
			num_device ++;
			if(wifi_setting->mode == RTW_MODE_AP){
				DBG_871X("Associated Client List:");
				DBG_871X("==============================");
				num_client = info_sz/sizeof(rtw_mac_t);
				if(num_client == 0){
					DBG_871X("Client Num: 0");
					break;
				}
				DBG_871X("Client Num: %d", num_client);
				for(i=0;i<num_client;i++){
					pSetting += i*sizeof(rtw_mac_t);
					DBG_871X("Client %d:", i + 1);
					mac = (rtw_mac_t *)(pSetting);
					DBG_871X("\tMAC => "MAC_FMT, MAC_ARG(mac->octet));
				}
			}
		}		
	}
	else{
		DBG_871X("ATW?: FAILED![errno:%d]\n", status);
		rtw_os_indicate_wifi_info(padapter->pnetdev, NULL);
	}
	return ret;
}

static s32 rtw_ATWX_handler(PADAPTER padapter, char status, u8 *rsp_msg, u16 sz){
	s32 ret = _SUCCESS;
	rtw_wifi_setting_t *pSetting = (rtw_wifi_setting_t *)rsp_msg;
	if((status == RTW_SUCCESS)&&(pSetting->ssid[0] != 0))
	{

		if(sz >= sizeof(rtw_wifi_setting_t))
			rtw_show_wifi_info((const char *)WLAN0_NAME, rsp_msg);
		if(padapter->fw_status == 0){
			padapter->fw_status = 1;
			DBG_871X("network becomes connected!\n");
			rtw_os_indicate_connect(padapter->pnetdev);
		}
	}
	else
		DBG_871X("ATWX: FAILED![errno:%d]\n", status);
	
	return ret;
}

static s32 rtw_ATWV_handler(PADAPTER padapter, char status, u8 *rsp_msg, u16 sz){
	s32 ret = _SUCCESS;
	u8 cmd_id = *rsp_msg;
	if(status == RTW_SUCCESS){

		DBG_871X("ATWV=[%d] SUCCESSED\n", cmd_id);
#if 0
		if(cmd_id == RTW_WOWLAN_CMD_GET_STATUS)
		{
			struct rtw_wowlan_status *status = (struct rtw_wowlan_status *)(rsp_msg+1);
			//DBG_871X("wake up reason: 0x%x\n", status->wakeup_reasons);
			rtw_os_indicate_wowlan_status(padapter->pnetdev, cmd_id, (char *)status);
		}
		else if(cmd_id == RTW_WOWLAN_CMD_ENABLE)
			rtw_os_indicate_wowlan_status(padapter->pnetdev, cmd_id, (char *)status);
#else
		rtw_os_indicate_wowlan_status(padapter->pnetdev, cmd_id, (char *)(rsp_msg+1));
#endif
	}
	else
		DBG_871X("ATWV=[%d] FAILED![errno: %d]\n", cmd_id, status);
	return ret;
}

static s32 rtw_ATWW_handler(PADAPTER padapter, char status, u8 *rsp_msg, u16 sz){
	s32 ret = _SUCCESS;
	_irqL irql;
	
	rtw_wifi_setting_t *pSetting = (rtw_wifi_setting_t *)rsp_msg;
	if((status == RTW_SUCCESS)&&(pSetting->ssid[0] != 0))
	{
		if(sz >= sizeof(rtw_wifi_setting_t))
			rtw_show_wifi_info((const char *)WLAN0_NAME, rsp_msg);		
		if(check_fwstate(padapter, WIFI_ASOC_STATE) == _FALSE){
			rtw_enter_critical_bh(&padapter->lock, &irql);
			set_fwstate(padapter, WIFI_ASOC_STATE | WIFI_STATION_STATE);
			rtw_exit_critical_bh(&padapter->lock, &irql);
			rtw_os_indicate_connect(padapter->pnetdev);
		}	
	}
	else
		DBG_871X("ATWW: FAILED![errno:%d]\n", status);
	
	return ret;
}

s32 rtl8195a_c2h_cmd_handler(PADAPTER padapter, u8 *cmd_data, u16 sz){
	s32 ret = _SUCCESS;
	u8 *str = cmd_data;
	char code[5] = {0};//for 4 bytes at cmd code
	rtw_result_t status = RTW_SUCCESS;
	u8 *cmd_payload = NULL;
	//char delims[] = "\r\n";

	if((cmd_data == NULL)||(sz==0))
		return ret;

	//todo: use strsep, much safer
	rtw_memcpy(code, str+2, 4);
	status = *(str+8);
	cmd_payload = str+11;
	DBG_871X("A C2H message received! Cmd: %s sz:%d bytes\n", code, sz);
	if(strcmp(code, _AT_WLAN_JOIN_NET_)==0)
		rtw_ATWC_handler(padapter, status, cmd_payload, sz-11);
	else if(strcmp(code, _AT_WLAN_DISC_NET_)==0)
		rtw_ATWD_handler(padapter, status, cmd_payload, sz-11);
	else if(strcmp(code, _AT_WLAN_SCAN_)==0)
		rtw_ATWS_handler(padapter, status, cmd_payload, sz-11);
	else if(strcmp(code, _AT_WLAN_AP_ACTIVATE_)==0)
		rtw_ATWA_handler(padapter, status, cmd_payload, sz-11);
	else if(strcmp(code, _AT_WLAN_AP_STA_ACTIVATE_)==0)
		rtw_ATWB_handler(padapter, status, cmd_payload, sz-11);
	else if(strcmp(code, _AT_WLAN_SIMPLE_CONFIG_)==0)
		rtw_ATWQ_handler(padapter, status, cmd_payload, sz-11);
	else if(strcmp(code, _AT_WLAN_IWPRIV_)==0)
		rtw_ATWZ_handler(padapter, status, cmd_payload, sz-11);
	else if(strcmp(code, _AT_WLAN_INFO_)==0)
		rtw_atw_info_handler(padapter, status, cmd_payload, sz-11);
	else if(strcmp(code, _AT_WLAN_AIRKISS_)==0)
		rtw_ATWX_handler(padapter, status, cmd_payload, sz-11);
	else if(strcmp(code, _AT_WLAN_PROMISC_)==0)
		rtw_ATWM_handler(padapter, status, cmd_payload, sz-11);
	else if(strcmp(code, _AT_WLAN_WOWLAN_)==0)
		rtw_ATWV_handler(padapter, status, cmd_payload, sz-11);
	else if(strcmp(code, _AT_WLAN_WPS_)==0)
		rtw_ATWW_handler(padapter, status, cmd_payload, sz-11);
	else
	{
		DBG_871X("AT CMD: [%s] status: [%d]\n", code, status);
	}

	return ret;
}
