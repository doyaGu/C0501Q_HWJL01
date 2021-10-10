/*
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
#ifndef __LWIP_PACKET_H__
#define __LWIP_PACKET_H__

#include "lwip/arch.h"

#include "lwip/pbuf.h"
///#include "lwip/inet.h"
#include "lwip/ip.h"
#include "lwip/raw.h"
//struct stack;

//#include <netpacket/packet.h>
//#include <linux/if_packet.h>
#include "lwip/sockets.h"

#define ETH_P_ALL  0x0003    /* Every packet (be careful!!!) */

#define packet_pcb raw_pcb
#define MEMP_PACKET_PCB MEMP_NUM_RAW_PCB

/* The following functions is the application layer interface to the
   PACKET code. */
struct packet_pcb * packet_new     (u16_t proto,u16_t dgramflag);
void             packet_remove     (struct packet_pcb *pcb);
err_t            packet_bind       (struct packet_pcb *pcb, struct ip_addr *ipaddr,
		u16_t protocol);
err_t            packet_connect    (struct packet_pcb *pcb, struct ip_addr *ipaddr,
		u16_t protocol);

void             packet_recv       (struct packet_pcb *pcb,
                                    void (* recv)(void *arg, struct packet_pcb *pcb,
                                                  struct pbuf *p,
                                                  struct ip_addr *addr,
                                                  u16_t proto),
                                    void *recv_arg);
err_t            packet_sendto     (struct packet_pcb *pcb, struct pbuf *p, struct ip_addr *ipaddr, u16_t protocol);
err_t            packet_send       (struct packet_pcb *pcb, struct pbuf *p);

/* The following functions are the lower layer interface to PACKET. */
u8_t             packet_input      (struct pbuf *p,struct sockaddr_ll *sll,u16_t link_header_size);
void             packet_init       (void);


#endif /* __LWIP_PACKET_H__ */
