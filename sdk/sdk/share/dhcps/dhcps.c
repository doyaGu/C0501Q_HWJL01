/////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010, Valhalla Wireless
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Valhalla Wireless nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL Valhalla Wireless BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// History
// creation t.elliott 2010
//

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Acts as a very simple DHCP server.   It disables MS Netbios on the client.
//  It sets the IP address, Netmask, Broadcast, MTU for the client interface.
//
//  NEED TO FIX :
//	It will respond to any requests and always hands out the same IP address for every request
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <string.h>

//#include "utils/lwiplib.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
//#include "common.h"
#include "dhcps.h"
//#include "config.h"

#include "ite/itp.h"

#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
PACK_STRUCT_BEGIN
struct dhcp_msg
{
    uint8_t  op, htype, hlen, hops;
    uint8_t  xid[4];
    uint16_t secs, flags;
    uint8_t  ciaddr[4];
    uint8_t  yiaddr[4];
    uint8_t  siaddr[4];
    uint8_t  giaddr[4];
    uint8_t  chaddr[16];
    uint8_t  sname[64];
    uint8_t  file[128];
    uint8_t  options[312];
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif

static struct dhcps_state s;

//#define USE_CLASS_B_NET 1

#define BOOTP_BROADCAST                         0x8000

#define DHCP_REQUEST                            1
#define DHCP_REPLY                              2
#define DHCP_HTYPE_ETHERNET                     1
#define DHCP_HLEN_ETHERNET                      6
#define DHCP_MSG_LEN                            236

#define DHCPS_SERVER_PORT                       67
#define DHCPS_CLIENT_PORT                       68

#define DHCPDISCOVER                            1
#define DHCPOFFER                               2
#define DHCPREQUEST                             3
#define DHCPDECLINE                             4
#define DHCPACK                                 5
#define DHCPNAK                                 6
#define DHCPRELEASE                             7

#define DHCP_OPTION_SUBNET_MASK                 1
#define DHCP_OPTION_ROUTER                      3
#define DHCP_OPTION_DNS_SERVER                  6
#define DHCP_OPTION_REQ_IPADDR                  50
#define DHCP_OPTION_LEASE_TIME                  51
#define DHCP_OPTION_MSG_TYPE                    53
#define DHCP_OPTION_SERVER_ID                   54
#define DHCP_OPTION_INTERFACE_MTU               26
#define DHCP_OPTION_PERFORM_ROUTER_DISCOVERY    31
#define DHCP_OPTION_BROADCAST_ADDRESS           28
#define DHCP_OPTION_REQ_LIST                    55
#define DHCP_OPTION_END                         255

static uint8_t   xid[4] = { 0xad, 0xde, 0x12, 0x23 };
static const uint8_t   magic_cookie[4] = { 99, 130, 83, 99 };
static struct udp_pcb  *pcb_dhcps;
static struct ip_addr  broadcast_dhcps;
static struct ip_addr  server_address;
static struct ip_addr  client_addresses[CFG_DHCPS_ADDR_COUNT];
static uint8_t chaddres[CFG_DHCPS_ADDR_COUNT][16];
static struct dhcp_msg *m;
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
static uint8_t * add_msg_type(uint8_t *optptr, uint8_t type)
{
    *optptr++ = DHCP_OPTION_MSG_TYPE;
    *optptr++ = 1;
    *optptr++ = type;
    return optptr;
}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
static uint8_t * add_offer_options(uint8_t *optptr)
{
    struct ip_addr ipadd;

    ipadd.addr = *((uint32_t *) &server_address);

#ifdef USE_CLASS_B_NET
    *optptr++ = DHCP_OPTION_SUBNET_MASK;
    *optptr++ = 4;      //len
    *optptr++ = 255;
    *optptr++ = 240;    //note this is different from interface netmask and broadcast address
    *optptr++ = 0;
    *optptr++ = 0;
#else
    *optptr++ = DHCP_OPTION_SUBNET_MASK;
    *optptr++ = 4;      //len
    *optptr++ = 255;
    *optptr++ = 255;    //note this is different from interface netmask and broadcast address
    *optptr++ = 255;
    *optptr++ = 0;
#endif

    *optptr++ = DHCP_OPTION_LEASE_TIME;
    *optptr++ = 4;      //len
#if 0
    *optptr++ = 0x00;
    *optptr++ = 0x01;
    *optptr++ = 0x51;
    *optptr++ = 0x80;           //1 day
#else
	*optptr++ = 0x00;
	*optptr++ = 0xED;
	*optptr++ = 0x4E;
	*optptr++ = 0x00; 
#endif

    *optptr++ = DHCP_OPTION_SERVER_ID;
    *optptr++ = 4;      //len
    *optptr++ = ip4_addr1(&ipadd);
    *optptr++ = ip4_addr2(&ipadd);
    *optptr++ = ip4_addr3(&ipadd);
    *optptr++ = ip4_addr4(&ipadd);

#ifdef CLASS_B_NET
    *optptr++ = DHCP_OPTION_BROADCAST_ADDRESS;
    *optptr++ = 4;      //len
    *optptr++ = ip4_addr1(&ipadd);
    *optptr++ = 255;
    *optptr++ = 255;
    *optptr++ = 255;
#else
    *optptr++ = DHCP_OPTION_BROADCAST_ADDRESS;
    *optptr++ = 4;      //len
    *optptr++ = ip4_addr1(&ipadd);
    *optptr++ = ip4_addr2(&ipadd);
    *optptr++ = ip4_addr3(&ipadd);
    *optptr++ = 255;
#endif

    *optptr++ = DHCP_OPTION_INTERFACE_MTU;
    *optptr++ = 2;      //len
//#ifdef CLASS_B_NET
#if 1
    *optptr++ = 0x05;   //mtu of 1500
    *optptr++ = 0xdc;
#else
    *optptr++ = 0x02;           //mtu of 576
    *optptr++ = 0x40;
#endif

    //*optptr++ = DHCP_OPTION_PERFORM_ROUTER_DISCOVERY;
    //*optptr++ = 1;      //len
    //*optptr++ = 0x00;   //dont do router discovery

    *optptr++ = DHCP_OPTION_DNS_SERVER;
    *optptr++ = 4;      //len
    *optptr++ = ip4_addr1(&ipadd);
    *optptr++ = ip4_addr2(&ipadd);
    *optptr++ = ip4_addr3(&ipadd);
    *optptr++ = ip4_addr4(&ipadd);
    
    *optptr++ = DHCP_OPTION_ROUTER;
    *optptr++ = 4;      //len
    *optptr++ = ip4_addr1(&ipadd);
    *optptr++ = ip4_addr2(&ipadd);
    *optptr++ = ip4_addr3(&ipadd);
    *optptr++ = ip4_addr4(&ipadd);

    //disable microsoft netbios over tcp
    *optptr++ = 43;     //vendor specific
    *optptr++ = 6;      //length of embedded option

    *optptr++ = 0x01;   //vendor specific (microsoft disable netbios over tcp)
    *optptr++ = 4;      //len
    *optptr++ = 0x00;
    *optptr++ = 0x00;
    *optptr++ = 0x00;
    *optptr++ = 0x02;           //disable=0x02,  enable = 0x00

    return optptr;
}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
static uint8_t * add_end(uint8_t *optptr)
{
    *optptr++ = DHCP_OPTION_END;
    return optptr;
}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
static void create_msg(struct dhcp_msg *m)
{
    struct ip_addr client;
    int i;
    static uint8_t null_chaddr[16];
    
    for (i = 0; i < CFG_DHCPS_ADDR_COUNT; i++)
    {
        if (memcmp((char *) &chaddres[i][0], (char *) m->chaddr, 16) == 0)
        {
            client.addr = *((uint32_t *) &client_addresses[i]);
            printf("exist addr[%d] 0x%X, hw %02X-%02X-%02X-%02X-%02X-%02X\n", i, client.addr, m->chaddr[0], m->chaddr[1], m->chaddr[2], m->chaddr[3], m->chaddr[4], m->chaddr[5]);            
            break;
        }
    }

    if (i == CFG_DHCPS_ADDR_COUNT)
    {
        for (i = 0; i < CFG_DHCPS_ADDR_COUNT; i++)
        {
            if (memcmp((char *) &chaddres[i][0], (char *) null_chaddr, 16) == 0)
            {
                client.addr = *((uint32_t *) &client_addresses[i]);
                memcpy((char *) &chaddres[i][0], m->chaddr, 16);
                printf("new addr[%d] 0x%X, hw %02X-%02X-%02X-%02X-%02X-%02X\n", i, client.addr, m->chaddr[0], m->chaddr[1], m->chaddr[2], m->chaddr[3], m->chaddr[4], m->chaddr[5]);
                break;
            }
        }
    }

    if (i == CFG_DHCPS_ADDR_COUNT)
    {
        client.addr = *((uint32_t *) &client_addresses[0]);
        memcpy((char *) &chaddres[0][0], m->chaddr, 16);
        printf("out of addr 0x%X, hw %02X-%02X-%02X-%02X-%02X-%02X\n", client.addr, m->chaddr[0], m->chaddr[1], m->chaddr[2], m->chaddr[3], m->chaddr[4], m->chaddr[5]);
    }

    m->op    = DHCP_REPLY;
    m->htype = DHCP_HTYPE_ETHERNET;
    m->hlen  = 6;     //mac id length
    m->hops  = 0;
    memcpy((char *) xid, (char *) m->xid, sizeof(m->xid));
    m->secs  = 0;
    m->flags = htons(BOOTP_BROADCAST);     /*  Broadcast bit. */

    memcpy((char *) m->yiaddr, (char *) &client.addr, sizeof(m->yiaddr));

    memset((char *) m->ciaddr, 0, sizeof(m->ciaddr));
    memset((char *) m->siaddr, 0, sizeof(m->siaddr));
    memset((char *) m->giaddr, 0, sizeof(m->giaddr));
    memset((char *) m->sname, 0, sizeof(m->sname));
    memset((char *) m->file, 0, sizeof(m->file));

    memset((char *) m->options, 0, sizeof(m->options));
    memcpy((char *) m->options, (char *) magic_cookie, sizeof(magic_cookie));
}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
static void send_offer(struct pbuf *p)
{
    uint8_t *end;

    create_msg(m);

    end = add_msg_type(&m->options[4], DHCPOFFER);
    end = add_offer_options(end);
    end = add_end(end);

    udp_sendto(pcb_dhcps, p, &broadcast_dhcps, DHCPS_CLIENT_PORT);
}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
static void send_nak(struct pbuf *p)
{
    uint8_t *end;

    create_msg(m);

    end = add_msg_type(&m->options[4], DHCPNAK);
    end = add_end(end);

    udp_sendto(pcb_dhcps, p, &broadcast_dhcps, DHCPS_CLIENT_PORT);
}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
static void send_ack(struct pbuf *p)
{
    uint8_t *end;

    create_msg(m);

    end = add_msg_type(&m->options[4], DHCPACK);
    end = add_offer_options(end);
    end = add_end(end);

    udp_sendto(pcb_dhcps, p, &broadcast_dhcps, DHCPS_CLIENT_PORT);
    
#if defined (CFG_NET_ETHERNET_WIFI)
    usleep(30000);
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET_DEFAULT, NULL);
#endif
    
}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
static uint8_t parse_options(uint8_t *optptr, int16_t len)
{
    uint8_t        *end = optptr + len;
    int16_t        type = 0;
    int i;

    s.state = DHCPS_STATE_IDLE;

    while (optptr < end)
    {
        switch ((int16_t) *optptr)
        {
        case DHCP_OPTION_MSG_TYPE:
            type = *(optptr + 2);
            break;

        case DHCP_OPTION_REQ_IPADDR:
            for (i = 0; i < CFG_DHCPS_ADDR_COUNT; i++)
            {
                if (memcmp((char *) &client_addresses[i], (char *) optptr + 2, 4) == 0)
                {
                    s.state = DHCPS_STATE_ACK;
                    break;
                }
            }
            if (i == CFG_DHCPS_ADDR_COUNT)
            {

                printf("[DHCP]nak i  %d ,#line %d \n",i,__LINE__);            
                s.state = DHCPS_STATE_NAK;
            }
            break;
        case DHCP_OPTION_END:
            break;
        }


        optptr += optptr[1] + 2;
    }

    switch (type)
    {
    case    DHCPDECLINE:
		printf("DHCPDECLINE\n");
        //s.state = DHCPS_STATE_IDLE;
        s.state = DHCPS_STATE_OFFER;
        break;

    case    DHCPDISCOVER:
        s.state = DHCPS_STATE_OFFER;
        break;

    case    DHCPREQUEST:
        if (!(s.state == DHCPS_STATE_ACK || s.state == DHCPS_STATE_NAK))
        {
            printf("[DHCP]s.state  %d ,#line %d \n",s.state,__LINE__);
            s.state = DHCPS_STATE_NAK;
        }
        break;

    case    DHCPRELEASE:
		printf("DHCPRELEASE\n");
        //s.state = DHCPS_STATE_IDLE;
        s.state = DHCPS_STATE_OFFER;
        break;
    }
    return s.state;
}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
static int16_t parse_msg(struct pbuf *p)
{
    m = (struct dhcp_msg *) p->payload;

    if (memcmp((char *) m->options, (char *) magic_cookie, sizeof(magic_cookie)) == 0)
    {
        return parse_options(&m->options[4], p->len);
    }
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
static void handle_dhcp(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, uint16_t port)
{
    struct pbuf *q;
    int16_t     tlen;

    printf("handle_dhcp\n");
#if defined (CFG_NET_ETHERNET_WIFI)
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_RESET_DEFAULT, NULL);
#endif

    if (p == NULL)
        return;

    tlen = p->tot_len;
    if (p->next != NULL)
    {
        q = pbuf_coalesce(p, PBUF_TRANSPORT);
        if (q->tot_len != tlen)
        {
            pbuf_free(p);
            return;
        }
    }
    else
    {
        q = p;
    }

    switch (parse_msg(p))
    {
    case    DHCPS_STATE_OFFER:
        printf("send_offer\n");
        send_offer(q);
        break;
    case    DHCPS_STATE_ACK:
        printf("send_ack\n");
        send_ack(q);
        break;
    case    DHCPS_STATE_NAK:
        printf("send_nak\n");
        send_nak(q);
        break;
    }

    pbuf_free(q);
}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void dhcps_init(void)
{
    int i, j, addr4;

    pcb_dhcps = udp_new();

    IP4_ADDR(&broadcast_dhcps, 255, 255, 255, 255);

#if defined(CFG_NET_WIFI_HOSTAPD) || defined(CFG_NET_WIFI_IPADDR)
	ipaddr_aton(CFG_NET_WIFI_IPADDR, &server_address);
#elif defined(CFG_NET_ETHERNET_IPADDR)
    ipaddr_aton(CFG_NET_ETHERNET_IPADDR, &server_address);
#else
    IP4_ADDR(&server_address, 192, 168, 1, 1);
    #endif
    
    addr4 = ip4_addr4(&server_address);
    
    j = 0;
    for (i = 0; i < CFG_DHCPS_ADDR_COUNT; i++)
    {
        if (addr4 == 1 + i)
            continue;
    
        IP4_ADDR(&client_addresses[j++], ip4_addr1(&server_address), ip4_addr2(&server_address), ip4_addr3(&server_address), 1 + i);
    }

    udp_bind(pcb_dhcps, IP_ADDR_ANY, DHCPS_SERVER_PORT);

    udp_recv(pcb_dhcps, handle_dhcp, NULL);
}

void dhcps_deinit(void)
{
    udp_remove(pcb_dhcps);
}

