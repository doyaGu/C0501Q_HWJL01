/**
 * @file
 * 
 * Implementation of packet protocol PCBs for low-level handling of
 * different types of LEVEL 2 protocols besides (or overriding) those
 * already available in lwIP.
 *
 * 2005.06.04
 * Send is not complete.
 * RAW-DGRAM mgmt still missing.
 *
 */

/*   This is part of LWIPv6
 *   
 *   Copyright 2005 Renzo Davoli University of Bologna - Italy
 *   
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */   
/*
 * Some of the code is still inhrited from the origina LWIP code:
 *
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include <string.h>

#include "lwip/opt.h"

#include "lwip/def.h"
#include "lwip/memp.h"
#include "lwip/inet.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/packet.h"

#include "netif/etharp.h"

#include "lwip/stats.h"

#include "arch/perf.h"
#include "lwip/snmp.h"

#if LWIP_PACKET

#define TOS_DGRAM 1
#define TOS_RAW 0

/** The list of PACKET PCBs */
static struct packet_pcb *packet_pcbs = NULL;


void
packet_init(void)
{
   //do somthing
}

/**
 * Determine if in incoming IP packet is covered by a PACKET PCB
 * and if so, pass it to a user-provided receive callback function.
 *
 * Given an incoming IP datagram (as a chain of pbufs) this function
 * finds a corresponding PACKET PCB and calls the corresponding receive
 * callback function.
 *
 * @param pbuf pbuf to be demultiplexed to a PACKET PCB.
 * @param netif network interface on which the datagram was received.
 * @Return - 1 if the packet has been eaten by a PACKET PCB receive
 *           callback function. The caller MAY NOT not reference the
 *           packet any longer, and MAY NOT call pbuf_free().
 * @return - 0 if packet is not eaten (pbuf is still referenced by the
 *           caller).
 *
 */
u8_t
packet_input(struct pbuf *p,struct sockaddr_ll *sll,u16_t link_header_size)
{
  struct packet_pcb *pcb;
  u8_t eaten = 0;
  struct ip_addr ipsll;

  LWIP_DEBUGF(RAW_DEBUG, ("packet_input\n"));

  pcb = packet_pcbs;
  /* loop through all packet pcbs until the packet is eaten by one */
  /* this allows multiple pcbs to match against the packet by design */
  while (pcb != NULL) {
    if (
				(IPSADDR_IFINDEX(pcb->local_ip) == 0 ||
				 IPSADDR_IFINDEX(pcb->local_ip) == sll->sll_ifindex) &&
				(pcb->protocol == ETH_P_ALL || pcb->protocol == sll->sll_protocol)
			 ) {
      /* receive callback function available? */
      if (pcb->recv != NULL) {
				struct pbuf *r, *q;
				char *ptr;

				r = pbuf_alloc(PBUF_RAW, p->tot_len, PBUF_RAM);

				if (r != NULL) {
					ptr = r->payload;

					for(q = p; q != NULL; q = q->next) {
						memcpy(ptr, q->payload, q->len);
						ptr += q->len;
					}

					SALL2IPADDR(*sll,ipsll);
					#if 1 
					ipsll.proto = sll->sll_protocol;
					#endif
					if  (pcb->tos == TOS_DGRAM)
						pbuf_header(r, -link_header_size);
					pcb->recv(pcb->recv_arg, pcb, r, &ipsll);
				}
      }
    }
    pcb = pcb->next;
  }
  LWIP_DEBUGF(RAW_DEBUG, ("packet_input leave\n"));
  return eaten;
}

/**
 * Bind a PACKET PCB.
 *
 * @param pcb PACKET PCB to be bound with a local address ipaddr.
 * @param ipaddr local IP address to bind with. Use IP_ADDR_ANY to
 * bind to all local interfaces.
 *
 * @return lwIP error code.
 * - ERR_OK. Successful. No error occured.
 * - ERR_USE. The specified IP address is already bound to by
 * another PACKET PCB.
 *
 * @see packet_disconnect()
 */
err_t
packet_bind(struct packet_pcb *pcb, struct ip_addr *ipaddr,u16_t protocol)
{
  //ip_addr_set(&pcb->local_ip, ipaddr);
	  pcb->local_ip.addr = ipaddr->addr;
	  pcb->local_ip.addr1 = ipaddr->addr1;
	  pcb->local_ip.addr2 = ipaddr->addr2;
	  pcb->local_ip.addr3 = ipaddr->addr3;
	  pcb->local_ip.proto = ipaddr->proto;
	  pcb->protocol=protocol;

  return ERR_OK;
}

/**
 * Connect an PACKET PCB. This function is required by upper layers
 * of lwip. Using the packet api you could use packet_sendto() instead
 *
 * This will associate the PACKET PCB with the remote address.
 *
 * @param pcb PACKET PCB to be connected with remote address ipaddr and port.
 * @param ipaddr remote IP address to connect with.
 *
 * @return lwIP error code
 *
 * @see packet_disconnect() and packet_sendto()
 */
err_t
packet_connect(struct packet_pcb *pcb, struct ip_addr *ipaddr, u16_t protocol)
{
	//ip_addr_set(&pcb->remote_ip, ipaddr);
	pcb->remote_ip.addr = ipaddr->addr;
    pcb->remote_ip.addr1 = ipaddr->addr1;
    pcb->remote_ip.addr2 = ipaddr->addr2;
    pcb->remote_ip.addr3 = ipaddr->addr3;
    pcb->remote_ip.proto = ipaddr->proto;
	pcb->out_protocol=protocol;
 
	return ERR_OK;
}

/**
 * Set the callback function for received packets that match the
 * packet PCB's protocol and binding. 
 * 
 * The callback function MUST either
 * - eat the packet by calling pbuf_free() and returning non-zero. The
 *   packet will not be passed to other packet PCBs or other protocol layers.
 * - not free the packet, and return zero. The packet will be matched
 *   against further PCBs and/or forwarded to another protocol layers.
 * 
 * @return non-zero if the packet was free()d, zero if the packet remains
 * available for others.
 */
void
packet_recv(struct packet_pcb *pcb,
         void (* recv)(void *arg, struct packet_pcb *upcb, struct pbuf *p,
                      struct ip_addr *addr, u16_t proto),
         void *recv_arg)
{
  /* remember recv() callback and user data */
  pcb->recv = (void *) recv;
  pcb->recv_arg = recv_arg;
}

/**
 * Send the packet IP packet to the given address. Note that actually you cannot
 * modify the IP headers (this is inconsistent with the receive callback where
 * you actually get the IP headers), you can only specify the IP payload here.
 * It requires some more changes in lwIP. (there will be a packet_send() function
 * then.)
 *
 * @param pcb the packet pcb which to send
 * @param p the IP payload to send
 * @param ipaddr the destination address of the IP packet
 *
 */
err_t
packet_sendto(struct packet_pcb *pcb, struct pbuf *p, struct ip_addr *ipaddr, u16_t protocol)
{
	
	struct sockaddr_ll sll;

	IPADDR2SALL(*ipaddr, sll);
    struct netif *netif;
  
	/*netif search*/
	if ((netif = netif_find_id(IPSADDR_IFINDEX(*ipaddr))) != NULL)
		return eth_packet_out (netif, p, &sll, protocol, pcb->tos);
	else
		return ERR_IF;
}

/**
 * Send the raw IP packet to the address given by raw_connect()
 *
 * @param pcb the raw pcb which to send
 * @param p the IP payload to send
 * @param ipaddr the destination address of the IP packet
 *
 */
err_t
packet_send(struct packet_pcb *pcb, struct pbuf *p)
{
	  return packet_sendto(pcb, p, &pcb->remote_ip, pcb->out_protocol);
}

/**
 * Remove an PACKET PCB.
 *
 * @param pcb PACKET PCB to be removed. The PCB is removed from the list of
 * PACKET PCB's and the data structure is freed from memory.
 *
 * @see packet_new()
 */
void
packet_remove(struct packet_pcb *pcb)
{
  struct packet_pcb *pcb2;
  /* pcb to be removed is first in list? */
  if (packet_pcbs == pcb) {
    /* make list start at 2nd pcb */
    packet_pcbs = packet_pcbs->next;
    /* pcb not 1st in list */
  } else for(pcb2 = packet_pcbs; pcb2 != NULL; pcb2 = pcb2->next) {
    /* find pcb in packet_pcbs list */
    if (pcb2->next != NULL && pcb2->next == pcb) {
      /* remove pcb from list */
      pcb2->next = pcb->next;
    }
  }
  memp_free(MEMP_PACKET_PCB, pcb);	
}

/**
 * Create a PACKET PCB.
 *
 * @return The PACKET PCB which was created. NULL if the PCB data structure
 * could not be allocated.
 *
 * @param proto the protocol number of the IPs payload (e.g. IP_PROTO_ICMP)
 *
 * @see packet_remove()
 */
struct packet_pcb *
packet_new(u16_t proto,u16_t dgramflag) {
  struct packet_pcb *pcb;

  LWIP_DEBUGF(RAW_DEBUG , ("packet_new\n"));

  pcb = memp_malloc(MEMP_PACKET_PCB);
  /* could allocate PACKET PCB? */
  if (pcb != NULL) {
    /* initialize PCB to all zeroes */
    memset(pcb, 0, sizeof(struct packet_pcb));
#ifdef LWSLIRP
		pcb->slirp_fddata = NULL;
#endif
    pcb->protocol = proto;
    pcb->next = packet_pcbs;
	pcb->tos = dgramflag; /* override type of service dgram (1) or raw (0) */
    packet_pcbs = pcb;
  }
  return pcb;
}

#endif /* LWIP_PACKET */
