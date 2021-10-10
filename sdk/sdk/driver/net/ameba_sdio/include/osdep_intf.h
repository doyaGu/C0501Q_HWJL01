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

#ifndef __OSDEP_INTF_H_
#define __OSDEP_INTF_H_
#include "autoconf.h"
#include "drv_types.h"
struct dvobj_priv *devobj_init(void);
void devobj_deinit(struct dvobj_priv *pdvobj);
u8 rtw_init_drv_sw(_adapter *padapter);
u8 rtw_free_drv_sw(_adapter *padapter);
void rtw_dev_unload(PADAPTER padapter);
u32 rtw_start_drv_threads(_adapter *padapter);
void rtw_stop_drv_threads (_adapter *padapter);
#ifdef PLATFORM_LINUX
int rtw_ioctl(struct net_device *dev, struct iwreq *rq, int cmd);

int rtw_init_netdev_name(struct net_device *pnetdev, const char *ifname);
struct net_device *rtw_init_netdev(_adapter *padapter);
void rtw_unregister_netdevs(struct net_device *pnetdev);

int rtw_ndev_notifier_register(void);
void rtw_ndev_notifier_unregister(void);
#endif //PLATFORM_LINUX

#ifdef PLATFORM_FREEBSD
extern int rtw_ioctl(struct ifnet * ifp, u_long cmd, caddr_t data);
#endif

#ifdef PLATFORM_FREERTOS
struct net_device *rtw_init_netdev(_adapter *padapter);
int rtw_ioctl(struct net_device *pnetdev, struct iwreq *rq, int cmd);
#endif /*PLATFORM_FREERTOS*/

#ifdef PLATFORM_ECOS
struct net_device *rtw_init_netdev(_adapter *padapter);
extern int rtw_ioctl(struct net_device *pnetdev, struct ifreq *rq, int cmd);
#endif /*PLATFORM_ECOS*/

int rtw_drv_register_netdev(_adapter *padapter);
void rtw_drv_unregister_netdev(_adapter *padapter);
void rtw_cancel_all_timer(_adapter *padapter);
#endif
