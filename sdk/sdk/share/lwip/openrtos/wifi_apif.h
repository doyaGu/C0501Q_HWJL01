#ifndef __PKTIF_H__
#define __PKTIF_H__

#ifdef CFG_NET_WIFI_HOSTAPD
#include "lwip/err.h"
#include "lwip/netif.h"
#include "ite/ite_wifi.h"

void  wifi_apif_ctrl(struct netif *netif, struct net_device_config* cfg);
void  wifi_apif_info(struct netif *netif, struct net_device_info* info);
int  wifi_apif_ioctrl(struct netif *netif, void* ptr,int cmd);


err_t wifi_apif_init    (struct netif *netif);
void  wifi_apif_shutdown(struct netif *netif);
//void  wifi_apif_poll    (struct netif *netif);


#endif
#endif

