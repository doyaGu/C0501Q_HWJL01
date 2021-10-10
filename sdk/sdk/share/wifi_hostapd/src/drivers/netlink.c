/*
 * Netlink helper functions for driver wrappers
 * Copyright (c) 2002-2009, Jouni Malinen <j@w1.fi>
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

#include "common.h"
#include "eloop.h"
#include "priv_netlink.h"
#include "netlink.h"

#include "ite/ite_wifi.h"


struct netlink_data {
	struct netlink_config *cfg;
	int sock;
};

struct netlink_data *netlink;


#if 0
static void netlink_receive_link(struct netlink_data *netlink,
				 void (*cb)(void *ctx, struct ifinfomsg *ifi,
					    u8 *buf, size_t len),
				 struct nlmsghdr *h)
{
	if (cb == NULL || NLMSG_PAYLOAD(h, 0) < sizeof(struct ifinfomsg))
		return;
	cb(netlink->cfg->ctx, NLMSG_DATA(h),
	   NLMSG_DATA(h) + NLMSG_ALIGN(sizeof(struct ifinfomsg)),
	   NLMSG_PAYLOAD(h, sizeof(struct ifinfomsg)));
}
#else
static void netlink_receive_link(struct netlink_data *netlink,
				 void (*cb)(void *ctx, struct ifinfomsg *ifi,
					    u8 *buf, size_t len),
				 char* buffer,int dataLen)
{
	if (cb == NULL)
		return;
	cb(netlink->cfg->ctx, NULL,
	   buffer,
	   dataLen);
}


#endif


int netlink_receive(void)
{
	char buf[8192];
	int left;
	struct sockaddr_nl from;
	socklen_t fromlen;


	portENTER_CRITICAL();
	fromlen = sizeof(from);
    left = mmpRtlWifiDriverNetlinkrecvfrom(buf, sizeof(buf));
	
	if (left < 0) {
		portEXIT_CRITICAL();
		return 1;
	}

	netlink_receive_link(netlink, netlink->cfg->newlink_cb,buf,left);
    

	if (left > 0) {
		printf("netlink: %d extra bytes in the end of netlink message\n",left);	   
	}


    portEXIT_CRITICAL();
	return 1;
}


struct netlink_data * netlink_init(struct netlink_config *cfg)
{
	struct sockaddr_nl local;

	netlinkInitial();

	netlink = os_zalloc(sizeof(*netlink));
	if (netlink == NULL)
		return NULL;

	netlink->cfg = cfg;

    #if 0
	netlink->sock = socket(PF_INET, SOCK_DGRAM, 0 );
	if (netlink->sock < 0) {
		wpa_printf(MSG_ERROR, "netlink: Failed to open netlink "
			   "socket: %s", strerror(errno));
		netlink_deinit(netlink);
		return NULL;
	}

	os_memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = RTMGRP_LINK;
	if (bind(netlink->sock, (struct sockaddr *) &local, sizeof(local)) < 0)
	{
		wpa_printf(MSG_ERROR, "netlink: Failed to bind netlink "
			   "socket: %s", strerror(errno));
		netlink_deinit(netlink);
		return NULL;
	}

	eloop_register_read_sock(netlink->sock, netlink_receive, netlink,
				 NULL);

	#endif

	return netlink;
}


void netlink_deinit(struct netlink_data *netlink)
{
	if (netlink == NULL)
		return;
	if (netlink->sock >= 0) {
		eloop_unregister_read_sock(netlink->sock);
		close(netlink->sock);
	}
	os_free(netlink->cfg);
	os_free(netlink);
}

int netlink_send_oper_ifla(struct netlink_data *netlink, int ifindex,
			   int linkmode, int operstate)
{
	struct {
		struct nlmsghdr hdr;
		struct ifinfomsg ifinfo;
		char opts[16];
	} req;
	struct rtattr *rta;
	static int nl_seq;
	ssize_t ret;

	os_memset(&req, 0, sizeof(req));

	req.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req.hdr.nlmsg_type = RTM_SETLINK;
	req.hdr.nlmsg_flags = NLM_F_REQUEST;
	req.hdr.nlmsg_seq = ++nl_seq;
	req.hdr.nlmsg_pid = 0;

	req.ifinfo.ifi_family = AF_UNSPEC;
	req.ifinfo.ifi_type = 0;
	req.ifinfo.ifi_index = ifindex;
	req.ifinfo.ifi_flags = 0;
	req.ifinfo.ifi_change = 0;

	if (linkmode != -1) {
		rta = aliasing_hide_typecast(
			((char *) &req + NLMSG_ALIGN(req.hdr.nlmsg_len)),
			struct rtattr);
		rta->rta_type = IFLA_LINKMODE;
		rta->rta_len = RTA_LENGTH(sizeof(char));
		*((char *) RTA_DATA(rta)) = linkmode;
		req.hdr.nlmsg_len = NLMSG_ALIGN(req.hdr.nlmsg_len) +
			RTA_LENGTH(sizeof(char));
	}
	if (operstate != -1) {
		rta = aliasing_hide_typecast(
			((char *) &req + NLMSG_ALIGN(req.hdr.nlmsg_len)),
			struct rtattr);
		rta->rta_type = IFLA_OPERSTATE;
		rta->rta_len = RTA_LENGTH(sizeof(char));
		*((char *) RTA_DATA(rta)) = operstate;
		req.hdr.nlmsg_len = NLMSG_ALIGN(req.hdr.nlmsg_len) +
			RTA_LENGTH(sizeof(char));
	}

	wpa_printf(MSG_DEBUG, "netlink: Operstate: linkmode=%d, operstate=%d",
		   linkmode, operstate);

	/*ret = send(netlink->sock, &req, req.hdr.nlmsg_len, 0);
	if (ret < 0) {
		wpa_printf(MSG_DEBUG, "netlink: Sending operstate IFLA "
			   "failed: %s (assume operstate is not supported)",
			   strerror(errno));
	}*/

	return 0;
}
