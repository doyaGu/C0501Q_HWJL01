/*
 * Driver interface list
 * Copyright (c) 2004-2005, Jouni Malinen <j@w1.fi>
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


#ifdef CONFIG_DRIVER_RTW
extern struct wpa_driver_ops wpa_driver_rtw_ops; /* driver_rtw.c */
#endif /* CONFIG_DRIVER_RTW */


struct wpa_driver_ops *hostapd_drivers[] =
{
#ifdef CONFIG_DRIVER_RTW
	&wpa_driver_rtw_ops,
#endif /* CONFIG_DRIVER_RTW */
	NULL
};
