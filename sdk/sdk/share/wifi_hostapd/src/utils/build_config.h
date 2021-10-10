/*
 * wpa_supplicant/hostapd - Build time configuration defines
 * Copyright (c) 2005-2006, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 *
 * This header file can be used to define configuration defines that were
 * originally defined in Makefile. This is mainly meant for IDE use or for
 * systems that do not have suitable 'make' tool. In these cases, it may be
 * easier to have a single place for defining all the needed C pre-processor
 * defines.
 */

#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H

/* Insert configuration defines, e.g., #define EAP_MD5, here, if needed. */

#define HOSTAPD
#define CONFIG_CTRL_IFACE 
#define CONFIG_CTRL_IFACE_UDP
#ifdef CFG_NET_WIFI_ENABLE_80211N
#define CONFIG_IEEE80211N
#endif
#define CONFIG_DRIVER_RTW
#define CONFIG_DRIVER_RTL_DFS
//#define CONFIG_WPS 
//#define EAP_SERVER_WSC
#define EAP_SERVER
#define NEED_AP_MLME
#define CONFIG_NO_DUMP_STATE
#define CONFIG_NO_RADIUS
#define CONFIG_NO_ACCOUNTING
#define CONFIG_NO_VLAN
//#define IEEE8021X_EAPOL
#define CONFIG_L2_PACKET
#define NEED_NETLINK
#define NEED_LINUX_IOCTL
//#define CONFIG_IAPP
//#define CONFIG_RSN_PREAUTH
#define CONFIG_PEERKEY

#endif /* BUILD_CONFIG_H */
