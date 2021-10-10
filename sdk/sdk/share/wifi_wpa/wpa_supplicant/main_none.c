/*
 * WPA Supplicant / Example program entrypoint
 * Copyright (c) 2003-2005, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"

#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "ite/itp.h"

#include "common.h"
#include "wpa_supplicant_i.h"
#include "ctrl_iface.h"
#include "ite/ite_wifi.h"

struct wpa_supplicant *wpa_s_ex;
static int wpaprocStop = 0;
int wpaSupplicantDeinit = 0;

int elooprunning = 0;
int gWpaNetlinkInit = 0;
void* iteGetwpa_s(void)
{
   return (void*)wpa_s_ex;
}

void* wpaProc(void *p)
{
    
	struct wpa_interface iface;
	int exitcode = 0;
	struct wpa_params params;
	struct wpa_global *global;

	elooprunning = 0;
    wpaprocStop = 0;
	
	memset(&params, 0, sizeof(params));
	//params.wpa_debug_level = MSG_INFO;

	global = wpa_supplicant_init(&params);
	if (global == NULL)
		return -1;

	memset(&iface, 0, sizeof(iface));
	/* TODO: set interface parameters */
	iface.driver = "wext";
	iface.ifname = "wlan0";

	
	wpa_s_ex = wpa_supplicant_add_iface(global, &iface);
	if (wpa_s_ex == NULL)
		exitcode = -1;


	if (exitcode == 0)
		exitcode = wpa_supplicant_run(global);


	wpa_supplicant_deinit(global);

	wpaprocStop = 1;

	//return exitcode;
}

sem_t* wpa_create_sem(int cnt)
{
    sem_t* x = malloc(sizeof(sem_t)); 
    sem_init(x, 0, cnt); 
    return x;
}

#define SYS_CreateEvent()               wpa_create_sem(0) 
#define SYS_WaitEvent                   itpSemWaitTimeout
#define SYS_SetEvent                    sem_post
#define SYS_DelEvent(a)                 do { sem_destroy(a); free(a); } while(0)
#define SYS_SetEventFromIsr				itpSemPostFromISR

extern void* netlink_event;


void* wpaNetlinkProc(void *p)
{
   int ret = 1;
   netlink_event = SYS_CreateEvent();
   
   for(;;)
   {
	  SYS_WaitEvent(netlink_event, 30);
      netlink_receive();
   }
   return ret;  
}

extern void* netlink_event;



int iteStartWPACtrl(void)
{   
	pthread_t taskn;
	pthread_attr_t attrn;
	struct sched_param paramn;
	pthread_t task;
	pthread_attr_t attr;
	struct sched_param param;
    if (gWpaNetlinkInit == 0){
    	pthread_attr_init(&attrn);
    	paramn.sched_priority = 4;
    	pthread_attr_setdetachstate(&attrn,PTHREAD_CREATE_DETACHED);
    	pthread_attr_setschedparam(&attrn, &paramn);
    	pthread_create(&taskn, &attrn, wpaNetlinkProc, NULL);
    }
    gWpaNetlinkInit++;

	pthread_attr_init(&attr);
	param.sched_priority = 1;
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedparam(&attr, &param);
	pthread_create(&task, &attr, wpaProc, NULL);
	 
	 return 0;
}

void iteStopWPACtrl(void)
{
	 eloop_terminate();
}

int iteStopWPADone(void)
{
	 return wpaprocStop;
}

int iteWPADeinitDone(void)
{
	 return wpaSupplicantDeinit;
}

int iteWPACtrlIsReady(void)
{
    return elooprunning;
}


void iteWPACtrlDisconnectNetwork(void)
{
    wpaSupplicantDeinit = 0;
    //step 0
    {
        char* result;
        char cmd[256];
        int replen = 0;
        snprintf(cmd, sizeof(cmd), "DISABLE_NETWORK 0");
        result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
        free(result);
    }

    //step 1
    {
        char* result;
        char cmd[256];
        int replen = 0;
        snprintf(cmd, sizeof(cmd), "REMOVE_NETWORK 0");
        result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
        free(result);
    }    
}

void iteWPACtrlWpsCancel(void)
{

   char* result;
   char cmd[256];
   int replen = 0;
   snprintf(cmd, sizeof(cmd), "WPS_CANCEL");
   result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
   free(result);
  
}

int iteWPACtrlWpsIsComplete(void)
{
    if(wpa_s_ex->wpa_state == WPA_COMPLETED)
		return 1;
	else
		return 0;
}

void iteWPACtrlConnectNetwork(struct net_device_config * netDeviceConfig)
{
    if(netDeviceConfig->securitySuit.securityMode == WLAN_SEC_EAP)
    {
           //do notthing
           printf("not support EAP mode!\n");
    }
	else if(netDeviceConfig->securitySuit.securityMode == WLAN_SEC_WPS)
	{
         //step 0
		{
			char* result;
			char cmd[256];
			int replen = 0;
			snprintf(cmd, sizeof(cmd), "DISABLE_NETWORK 0");
			result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
			free(result);
		}

		//step 1
		{
			char* result;
			char cmd[256];
			int replen = 0;
			snprintf(cmd, sizeof(cmd), "REMOVE_NETWORK 0");
			result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
			free(result);
		}
		
		//step 0
		{
			char* result;
			char cmd[256];
			int replen = 0;
			snprintf(cmd, sizeof(cmd), "DISABLE_NETWORK 1");
			result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
			free(result);
		}

		//step 1
		{
			char* result;
			char cmd[256];
			int replen = 0;
			snprintf(cmd, sizeof(cmd), "REMOVE_NETWORK 1");
			result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
			free(result);
		}
		
				//step 0
		{
			char* result;
			char cmd[256];
			int replen = 0;
			snprintf(cmd, sizeof(cmd), "DISABLE_NETWORK 2");
			result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
			free(result);
		}

		//step 1
		{
			char* result;
			char cmd[256];
			int replen = 0;
			snprintf(cmd, sizeof(cmd), "REMOVE_NETWORK 2");
			result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
			free(result);
		}
		
        {
			char* result;
			char cmd[256];
			int replen = 0;
			snprintf(cmd, sizeof(cmd), "WPS_PBC");
			result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
			free(result);
        }	
	}
	else
	{
	    //step 0
		{
			char* result;
			char cmd[256];
			int replen = 0;
			snprintf(cmd, sizeof(cmd), "DISABLE_NETWORK 0");
			result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
			free(result);
		}

		//step 1
		{
			char* result;
			char cmd[256];
			int replen = 0;
			snprintf(cmd, sizeof(cmd), "REMOVE_NETWORK 0");
			result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
			free(result);
		}

		//step 2
		{
			char* result;
			char cmd[256];
			int replen = 0;
			snprintf(cmd, sizeof(cmd), "ADD_NETWORK");
			result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
			printf("add_network = %s\n",result);
			free(result);
		}

		//step 3
		{
			char* result;
			char cmd[256];
			int replen = 0;
			snprintf(cmd, sizeof(cmd), "SET_NETWORK 0 ssid \"%s\"",netDeviceConfig->ssidName);
			result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
			free(result);
		}

		//step 3-2
		{
			char* result;
			char cmd[256];
			int replen = 0;
			snprintf(cmd, sizeof(cmd), "SET_NETWORK 0 scan_ssid 1");
			result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
			free(result);
		}

        switch(netDeviceConfig->securitySuit.securityMode)
        {
            case WLAN_SEC_NOSEC:				
				printf("WLAN_SEC_NOSEC\n");
				//step 4
				{
					char* result;
					char cmd[256];
					int replen = 0;
					snprintf(cmd, sizeof(cmd), "SET_NETWORK 0 key_mgmt NONE");
					result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
					free(result);
				}

                break;

            case WLAN_SEC_WEP:
				printf("WLAN_SEC_WEP\n");
				//step 4
				{
					char* result;
					char cmd[256];
					int replen = 0;
					snprintf(cmd, sizeof(cmd), "SET_NETWORK 0 key_mgmt NONE");
					result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
					free(result);
				}

				
				//step 5
				{
					 char* result;
					 char cmd[256];
					 int replen = 0;
					 if((strlen(netDeviceConfig->securitySuit.wepKeys[0]) == 5) || (strlen(netDeviceConfig->securitySuit.wepKeys[0]) == 13))
					 {
					 	snprintf(cmd, sizeof(cmd), "SET_NETWORK 0 wep_key0 \"%s\"",netDeviceConfig->securitySuit.wepKeys[0]);
					 }
					 else if((strlen(netDeviceConfig->securitySuit.wepKeys[0]) == 10) || (strlen(netDeviceConfig->securitySuit.wepKeys[0]) == 26))
					 {
					 	snprintf(cmd, sizeof(cmd), "SET_NETWORK 0 wep_key0 %s",netDeviceConfig->securitySuit.wepKeys[0]);
					 }

					 result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
					 free(result);
				 }

                break;

            case WLAN_SEC_WPAPSK:
			case WLAN_SEC_WPAPSK_AES:
			case WLAN_SEC_WPA2PSK:
			case WLAN_SEC_WPA2PSK_TKIP:
			case WLAN_SEC_WPAPSK_MIX:
				printf("WLAN_SEC_WPAPSK_MIX\n");				
				//step 5
				{
					 char* result;
					 char cmd[256];
					 int replen = 0;
					 snprintf(cmd, sizeof(cmd), "SET_NETWORK 0 psk \"%s\"",netDeviceConfig->securitySuit.preShareKey);
					 result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
					 free(result);
				 }				
				break;
                
        }

		//step start connect
		{
	        char* result;
			char cmd[256];
			int replen = 0;
			snprintf(cmd, 256, "ENABLE_NETWORK 0");
			result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
			free(result);
		}
	}
}

void iteWPACtrlGetNetwork(struct net_device_config * netDeviceConfig)
{
    //get ssid
	{	char* result;
		char cmd[256];
		int replen = 0;
		snprintf(cmd, sizeof(cmd), "GET_NETWORK 0 ssid");
		result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
		strcpy(netDeviceConfig->ssidName,result);
		free(result);
	}

	//get key_mgmt
	{	char* result;
		char cmd[256];
		int replen = 0;
		snprintf(cmd, sizeof(cmd), "GET_NETWORK 0 key_mgmt");
		result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
		if(strcmp(result,"WPA-PSK") == 0)
          netDeviceConfig->securitySuit.securityMode = WLAN_SEC_WPAPSK_MIX;
		else
		  netDeviceConfig->securitySuit.securityMode = WLAN_SEC_NOSEC;	
		free(result);
	}

	//get psk
	{	char* result;
		char cmd[256];
		int replen = 0;
		snprintf(cmd, sizeof(cmd), "GET_NETWORK 0 psk");
		result = wpa_supplicant_ctrl_iface_process(iteGetwpa_s(),cmd,&replen);
        strcpy(netDeviceConfig->securitySuit.preShareKey,result);
		free(result);
	}
}

extern int connectOperstate;

int iteWPAConnectState(void)
{
    return connectOperstate;
}

