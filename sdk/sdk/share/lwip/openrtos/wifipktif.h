#ifndef __PKTIF_H__
#define __PKTIF_H__

#ifdef CFG_NET_WIFI
#include "lwip/err.h"
#include "lwip/netif.h"
#include "ite/ite_wifi.h"

void  wifiif_ctrl(struct netif *netif, struct net_device_config* cfg);
void  wifiif_info(struct netif *netif, struct net_device_info* info);
int  wifiif_ioctrl(struct netif *netif, void* ptr,int cmd);


err_t wifiif_init    (struct netif *netif);
void  wifiif_shutdown(struct netif *netif);
void  wifiif_poll    (struct netif *netif);


#endif
#endif

