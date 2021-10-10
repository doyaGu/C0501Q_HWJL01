/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Ethernet functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <sys/ioctl.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "netif/etharp.h"
#include "itp_cfg.h"
#include "ite/ite_mac.h"
#include "ite/itp.h"

#ifdef __OPENRTOS__
#include "openrtos/FreeRTOSConfig.h"
#define MAC_TASK_PRIORITY 5
#if !defined(CFG_NET_AMEBA_SDIO)
#define ITE_MAC
#endif
#endif // __OPENRTOS__

#define MAC_STACK_SIZE        20480

#if defined(CFG_NET_AMEBA_SDIO)
err_t amebaif_init(struct netif *netif);
#endif
err_t ethernetif_init(struct netif *netif);
void  ethernetif_shutdown(struct netif *netif);
void  ethernetif_poll(struct netif *netif);

#ifdef CFG_NET_ETHERNET_MAC_ADDR_DEFAULT
static uint8_t macAddr[6] = { CFG_NET_ETHERNET_MAC_ADDR_DEFAULT };
#else
static uint8_t macAddr[6];
#endif

#ifdef CFG_GPIO_ETHERNET
static const uint8_t  ioConfig[] = { CFG_GPIO_ETHERNET };
#else
static const uint8_t  ioConfig[10];
#endif

static struct netif   ethNetifs[CFG_NET_ETHERNET_COUNT];
static struct dhcp    ethNetifDhcp;

static bool           ethInited, ethDhcp;
static struct timeval ethLastTime;
static int            ethMscnt;
static timer_t        ethTimer;
static bool           ethInPollingFunc;

#ifdef CFG_NET_ETHERNET_AUTOIP
static struct autoip  ethNetifAutoip;
static bool           ethAutoip;
#endif

#ifdef CFG_NET_ETHERNET_MULTI_INTERFACE
static uint8_t eth_netif[CFG_NET_ETHERNET_COUNT];
#endif

#ifdef ITE_MAC

#ifdef CFG_NET_ETHERNET_MAC_ADDR_STORAGE

static const char *ethMacVerifyCode = CFG_NET_ETHERNET_MAC_ADDR_VERIFY_CODE;

int MacRead(uint8_t *macaddr)
{
    uint8_t  *buf   = NULL;
    uint32_t pos, blocksize = 0;
    int      ret, i;
    uint32_t offset = 0;

    LOG_INFO "Read MAC address from storage\n" LOG_END

#if defined(CFG_NET_ETHERNET_MAC_ADDR_NAND)
    int fd = open(":nand", O_RDWR, 0);
    LOG_DBG "nand fd: 0x%X\n", fd LOG_END
#elif defined(CFG_NET_ETHERNET_MAC_ADDR_NOR)
    int fd = open(":nor", O_RDWR, 0);
    LOG_DBG "nor fd: 0x%X\n", fd LOG_END
#elif defined(CFG_NET_ETHERNET_MAC_ADDR_SD0)
    int fd = open(":sd0", O_RDWR, 0);
    LOG_DBG "sd0 fd: 0x%X\n", fd LOG_END
#elif defined(CFG_NET_ETHERNET_MAC_ADDR_SD1)
    int fd = open(":sd1", O_RDWR, 0);
    LOG_DBG "sd1 fd: 0x%X\n", fd LOG_END
#endif
    if (fd == -1)
    {
        LOG_ERR "open device error: %d\n", fd LOG_END
        ret = __LINE__;
        goto end;
    }
    if (ioctl(fd, ITP_IOCTL_GET_BLOCK_SIZE, &blocksize))
    {
        LOG_ERR "get block size error\n" LOG_END
        ret = __LINE__;
        goto end;
    }
    LOG_DBG "blocksize=%d\n", blocksize LOG_END

    pos    = CFG_NET_ETHERNET_MAC_ADDR_POS / blocksize;

        #if defined(CFG_NET_ETHERNET_MAC_ADDR_NAND)
    offset = CFG_NET_ETHERNET_MAC_ADDR_POS % blocksize;
        #endif

    if (lseek(fd, pos, SEEK_SET) != pos)
    {
        LOG_ERR "seek to mac addr position %d(%d) error\n", CFG_NET_ETHERNET_MAC_ADDR_POS, pos LOG_END
        ret = __LINE__;
        goto end;
    }

    assert(blocksize >= 8);
    buf = (uint8_t *)malloc(blocksize);
    if (!buf)
    {
        ret = __LINE__;
        goto end;
    }

    ret = read(fd, buf, 1);
    if (ret != 1)
    {
        LOG_ERR "read mac address error: %d != 1\n", ret LOG_END
        ret = __LINE__;
        goto end;
    }

    // verify number
    if (buf[0 + offset] == ethMacVerifyCode[0] && buf[1 + offset] == ethMacVerifyCode[1])
    {
        memcpy(macaddr, &buf[2 + offset], 6);
        ret = 0;
    }
    else
    {
        LOG_WARN "mac address %02X-%02X-%02X-%02X-%02X-%02X verify code %02X%02X incorrect; expect %02X%02X\n",
        buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[0], buf[1], ethMacVerifyCode[0], ethMacVerifyCode[1]
        LOG_END
    }

end:
    if (fd != -1)
        close(fd);

    if (buf)
        free(buf);

    return ret;
}

void MacWrite(uint8_t *macaddr)
{
    uint8_t  *buf   = NULL;
    uint32_t pos, blocksize = 0;
    int      ret, i;
    uint32_t offset = 0;

    LOG_INFO "Write MAC address to storage...\n" LOG_END

#if defined(CFG_NET_ETHERNET_MAC_ADDR_NAND)
    int fd = open(":nand", O_RDWR, 0);
    LOG_DBG "nand fd: 0x%X\n", fd LOG_END
#elif defined(CFG_NET_ETHERNET_MAC_ADDR_NOR)
    int fd = open(":nor", O_RDWR, 0);
    LOG_DBG "nor fd: 0x%X\n", fd LOG_END
#elif defined(CFG_NET_ETHERNET_MAC_ADDR_SD0)
    int fd = open(":sd0", O_RDWR, 0);
    LOG_DBG "sd0 fd: 0x%X\n", fd LOG_END
#elif defined(CFG_NET_ETHERNET_MAC_ADDR_SD1)
    int fd = open(":sd1", O_RDWR, 0);
    LOG_DBG "sd1 fd: 0x%X\n", fd LOG_END
#endif
    if (fd == -1)
    {
        LOG_ERR "open device error: %d\n", fd LOG_END
        ret = __LINE__;
        goto end;
    }
    if (ioctl(fd, ITP_IOCTL_GET_BLOCK_SIZE, &blocksize))
    {
        LOG_ERR "get block size error\n" LOG_END
        ret = __LINE__;
        goto end;
    }
    LOG_DBG "blocksize=%d\n", blocksize LOG_END

    pos    = CFG_NET_ETHERNET_MAC_ADDR_POS / blocksize;

        #if defined(CFG_NET_ETHERNET_MAC_ADDR_NAND)
    offset = CFG_NET_ETHERNET_MAC_ADDR_POS % blocksize;
        #endif

    if (lseek(fd, pos, SEEK_SET) != pos)
    {
        LOG_ERR "seek to mac addr position %d(%d) error\n", CFG_NET_ETHERNET_MAC_ADDR_POS, pos LOG_END
        ret = __LINE__;
        goto end;
    }

    assert(blocksize >= 8);
    buf = (uint8_t *)malloc(blocksize);
    if (!buf)
    {
        ret = __LINE__;
        goto end;
    }

        #if defined(CFG_NET_ETHERNET_MAC_ADDR_NAND)
    if (read(fd, buf, 1) != 1)
    {
        LOG_ERR "read storage error: %d != 1\n", ret LOG_END
        ret = __LINE__;
        goto end;
    }

    if (lseek(fd, pos, SEEK_SET) != pos)
    {
        LOG_ERR "seek to mac addr position %d(%d) error\n", CFG_NET_ETHERNET_MAC_ADDR_POS, pos LOG_END
        ret = __LINE__;
        goto end;
    }
        #endif

    buf[0 + offset] = ethMacVerifyCode[0];
    buf[1 + offset] = ethMacVerifyCode[1];
    for (i = 0; i < 6; i++)
    {
        buf[i + 2 + offset] = macaddr[i];
    }

    if (write(fd, buf, 1) != 1)
    {
        LOG_ERR "write mac addr fail\n" LOG_END
        goto end;
    }

end:
    if (fd != -1)
        close(fd);

    if (buf)
        free(buf);
}

#endif // CFG_NET_ETHERNET_MAC_ADDR_STORAGE



static ITE_MAC_CFG_T mac_cfg;

int MacInit(bool initPhy)
{
    int ret, i;

    mac_cfg.flags |= (ITH_COUNT_OF(ioConfig) == ITE_MAC_GRMII_PIN_CNT) ? ITE_MAC_GRMII : 0;
    mac_cfg.clk_inv       = CFG_NET_MAC_CLOCK_INVERSE;
    mac_cfg.clk_delay     = CFG_NET_MAC_CLOCK_DELAY;
    mac_cfg.phyAddr       = CFG_NET_ETHERNET_PHY_ADDR;
    mac_cfg.ioConfig      = ioConfig;
    mac_cfg.linkGpio      = CFG_GPIO_ETHERNET_LINK;
    mac_cfg.phy_link_change = itpPhyLinkChange;
    mac_cfg.linkGpio_isr  = itpPhylinkIsr;
    mac_cfg.phy_link_status = itpPhyLinkStatus;
    mac_cfg.phy_read_mode = itpPhyReadMode;

    ret                   = iteMacInitialize(&mac_cfg);
    if (ret)
        return ret;

    #ifdef CFG_NET_ETHERNET_MAC_ADDR_RANDOM
    LOG_DBG "Random generate default MAC address\n" LOG_END
    srand(ithTimerGetCounter(portTIMER));
    macAddr[0] = 0;
    for (i = 1; i < 6; i++)
        macAddr[i] = (rand() % 256);

    #endif // CFG_NET_ETHERNET_MAC_ADDR_RANDOM

    #ifdef CFG_NET_ETHERNET_MAC_ADDR_STORAGE
    // read mac address from storage
    if (MacRead(macAddr))
    {
        #ifdef CFG_NET_ETHERNET_MAC_ADDR_UPGRADE
        MacWrite(macAddr);
        #endif
    }

    #endif // CFG_NET_ETHERNET_MAC_ADDR_STORAGE

    LOG_INFO "MAC address: %02X-%02X-%02X-%02X-%02X-%02X\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5] LOG_END

    ret = iteMacSetMacAddr((uint8_t *)macAddr);
    if (ret)
        return ret;

    if (initPhy)
        PhyInit(ITE_ETH_REAL);
        
    return 0;
}

#endif // ITE_MAC

// This function initializes all network interfaces
void itpEthernetLwipInit(void)
{
#ifdef CFG_NET_ETHERNET_MULTI_INTERFACE
//Mult-Interface Initail, eason 03/20/2017
    struct netif *xnetif_main, *xnetif_subnet_0;
    ip_addr_t    ipaddr, netmask, gw;

    ip_addr_set_zero(&ipaddr);
    ip_addr_set_zero(&netmask);
    ip_addr_set_zero(&gw);

    xnetif_subnet_0 = &ethNetifs[0];
	xnetif_main     = &ethNetifs[1];

	#ifdef CFG_NET_ETHERNET_IPADDR
    ipaddr_aton(CFG_NET_ETHERNET_IPADDR,  &ipaddr);
    ipaddr_aton(CFG_NET_ETHERNET_NETMASK, &netmask);
    ipaddr_aton(CFG_NET_ETHERNET_GATEWAY, &gw);
    #endif // CFG_NET_ETHERNET_IPADDR
	
/* Only set one interface for default */
/* Interface main */
    netif_add(xnetif_main, &ipaddr, &netmask, &gw, NULL, ethernetif_init, tcpip_input);
    netif_set_up(xnetif_main);

/* interface 0 */
    netif_set_default(netif_add(xnetif_subnet_0, &ipaddr, &netmask, &gw, NULL, ethernetif_init, tcpip_input));
    netif_set_up(xnetif_subnet_0);


    #if defined(CFG_NET_ETHERNET_DHCP) || defined(CFG_NET_ETHERNET_AUTOIP)

//DHCP just for netif_0(connect to differnet subnet AP) at Mult-Interface.
	#ifdef CFG_NET_ETHERNET_MULTI_INTERFACE_DHCP
    dhcp_set_struct(xnetif_subnet_0, &ethNetifDhcp);
    dhcp_start(xnetif_subnet_0);

    ethDhcp = true;
	#endif

    #ifdef CFG_NET_ETHERNET_AUTOIP
    autoip_set_struct(xnetif_main, &ethNetifAutoip);
    autoip_start(xnetif_main);
    ethAutoip = true;
    #endif

    gettimeofday(&ethLastTime, NULL);
    ethMscnt = 0;
    #endif

    ethInited = true;
#else
	//Normal Netif Initial
    struct netif *xnetif;
    ip_addr_t    ipaddr, netmask, gw;

    ip_addr_set_zero(&ipaddr);
    ip_addr_set_zero(&netmask);
    ip_addr_set_zero(&gw);

    xnetif = &ethNetifs[0];

    #ifdef CFG_NET_ETHERNET_IPADDR
    ipaddr_aton(CFG_NET_ETHERNET_IPADDR,  &ipaddr);
    ipaddr_aton(CFG_NET_ETHERNET_NETMASK, &netmask);
    ipaddr_aton(CFG_NET_ETHERNET_GATEWAY, &gw);
    #endif // CFG_NET_ETHERNET_IPADDR
#if defined(CFG_NET_AMEBA_SDIO) && defined(__OPENRTOS__)
    netif_set_default(netif_add(xnetif, &ipaddr, &netmask, &gw, NULL, amebaif_init, tcpip_input));
#else
    netif_set_default(netif_add(xnetif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, tcpip_input));
#endif
#if LWIP_NETIF_REMOVE_CALLBACK
    netif_set_remove_callback(xnetif, ethernetif_shutdown);
#endif
    netif_set_up(xnetif);

    #if defined(CFG_NET_ETHERNET_DHCP) || defined(CFG_NET_ETHERNET_AUTOIP)

    #ifdef CFG_NET_ETHERNET_DHCP
    dhcp_set_struct(xnetif, &ethNetifDhcp);
    dhcp_start(xnetif);
    ethDhcp = true;

    #endif // CFG_NET_ETHERNET_DHCP

    #ifdef CFG_NET_ETHERNET_AUTOIP
    autoip_set_struct(xnetif, &ethNetifAutoip);
    autoip_start(xnetif);
    ethAutoip = true;

    #endif // CFG_NET_ETHERNET_AUTOIP

    gettimeofday(&ethLastTime, NULL);
    ethMscnt = 0;

    #endif // defined(CFG_NET_ETHERNET_DHCP) || defined(CFG_NET_ETHERNET_AUTOIP)

    ethInited = true;
#endif
}

static void EthernetGetInfo(ITPEthernetInfo *info)
{
    struct netif *xnetif;

    if (ethInited == false)
    {
        LOG_ERR "ethernet is not init yet\n" LOG_END
        return;
    }

    if (info->index >= CFG_NET_ETHERNET_COUNT)
    {
        LOG_ERR "Out of ethernet: %d\n", info->index LOG_END
        return;
    }

    xnetif = &ethNetifs[info->index];

    if (xnetif->flags & NETIF_FLAG_LINK_UP)
        info->flags |= ITP_ETH_LINKUP;

    if (xnetif->ip_addr.addr)
        info->flags |= ITP_ETH_ACTIVE;

    info->address = xnetif->ip_addr.addr;
    info->netmask = xnetif->netmask.addr;
    sprintf(info->displayName, "Ethernet%d", info->index);
    memcpy((void *)info->hardwareAddress, (void *)macAddr, 6);
    sprintf(info->name, "eth%d", info->index);
}

static void EthernetPoll(void)
{
    struct netif *xnetif = &ethNetifs[CFG_NET_ETHERNET_COUNT - 1];

    if (!ethInited)
        return;

    if ((xnetif->ip_addr.addr == 0)
        && (ethDhcp
#ifdef CFG_NET_ETHERNET_AUTOIP
            || ethAutoip
#endif
            ))
    {
        struct timeval currTime;

        gettimeofday(&currTime, NULL);
        if (itpTimevalDiff(&ethLastTime, &currTime) >= DHCP_FINE_TIMER_MSECS)
        {
            if (ethDhcp)
                dhcp_fine_tmr();

#ifdef CFG_NET_ETHERNET_AUTOIP
            if (ethAutoip)
                autoip_tmr();
#endif

            ethLastTime = currTime;
            ethMscnt   += DHCP_FINE_TIMER_MSECS;
        }
        else if (ethMscnt >= DHCP_COARSE_TIMER_MSECS)
        {
            if (ethDhcp)
                dhcp_coarse_tmr();

            ethMscnt = 0;
        }
    }

    // check for packets and link status
#if defined(CFG_NET_AMEBA_SDIO)
    amebaif_poll(xnetif);
#else
    ethernetif_poll(xnetif);
#endif
}

#if CFG_NET_ETHERNET_POLL_INTERVAL > 0

static void EthernetPollHandler(timer_t timerid, int arg)
{
    ethInPollingFunc = true;
    EthernetPoll();
    ethInPollingFunc = false;
}

#endif // CFG_NET_ETHERNET_POLL_INTERVAL > 0

static int EthernetInit(void)
{
    // create mac poll task
#ifdef ITE_MAC
    {
        pthread_t          task;
        pthread_attr_t     attr;
        struct sched_param param;
        int                res;

    #ifdef CFG_NET_MAC_INIT_ON_BOOTLOADER
        res = MacInit(false);
    #else
        res = MacInit(true);
    #endif
        if (res)
        {
            errno = (ITP_DEVICE_ETHERNET << ITP_DEVICE_ERRNO_BIT) | res;
            return -1;
        }

        pthread_attr_init(&attr);
        //pthread_attr_setstacksize(&attr, MAC_STACK_SIZE);
        param.sched_priority = MAC_TASK_PRIORITY;
        pthread_attr_setschedparam(&attr, &param);
        pthread_create(&task, &attr, iteMacThreadFunc, NULL);
    }
#endif // ITE_MAC

#if CFG_NET_ETHERNET_POLL_INTERVAL > 0
    {
        struct itimerspec value;
        timer_create(CLOCK_REALTIME, NULL, &ethTimer);
        timer_connect(ethTimer, (VOIDFUNCPTR)EthernetPollHandler, 0);
        value.it_value.tv_sec = value.it_interval.tv_sec  = 0;
        value.it_value.tv_nsec = value.it_interval.tv_nsec = CFG_NET_ETHERNET_POLL_INTERVAL * 1000000;
        timer_settime(ethTimer, 0, &value, NULL);
    }
#endif // CFG_NET_ETHERNET_POLL_INTERVAL > 0

    return 0;
}

static void EthernetExit(void)
{
    int i;

    timer_delete(ethTimer);
    while (ethInPollingFunc)
        usleep(1000);
    for (i = 0; i < CFG_NET_ETHERNET_COUNT; ++i)
    {
        struct netif *xnetif = &ethNetifs[i];
        netif_set_down(xnetif);
        netif_remove(xnetif);
    }
}

static void EthernetReset(ITPEthernetSetting *setting)
{
    struct netif *xnetif;
    ip_addr_t    ipaddr, netmask, gw;

    if (setting->index >= CFG_NET_ETHERNET_COUNT)
    {
        LOG_ERR "Out of ethernet: %d\n", setting->index LOG_END
        return;
    }

    xnetif = &ethNetifs[setting->index]; //index_0 is interface_0, index_1 is interface_main

    if ((setting->index == CFG_NET_ETHERNET_COUNT - 1)
    #ifdef CFG_NET_ETHERNET_MULTI_INTERFACE_DHCP
	    || (setting->index == 0)
    #endif
	)
    {
        xnetif->ip_addr.addr = 0;

        if (setting->dhcp || setting->autoip)
        {
            if (!setting->dhcp && ethDhcp)
            {
                dhcp_stop(xnetif);
                ethDhcp = false;
            }

            if (setting->dhcp)
            {
                dhcp_set_struct(xnetif, &ethNetifDhcp);
                dhcp_start(xnetif);
                ethDhcp = true;
            }
#ifdef CFG_NET_ETHERNET_AUTOIP
            if (!setting->autoip && ethAutoip)
            {
                autoip_stop(xnetif);
                ethAutoip = false;
            }

            if (setting->autoip && !ethAutoip)
            {
                autoip_set_struct(xnetif, &ethNetifAutoip);
                autoip_start(xnetif);
                ethAutoip = true;
            }
#endif         // CFG_NET_ETHERNET_AUTOIP
            gettimeofday(&ethLastTime, NULL);
            ethMscnt = 0;
            return;
        }
        else
        {
            if (ethDhcp)
                dhcp_stop(xnetif);

            ethDhcp = false;

#ifdef CFG_NET_ETHERNET_AUTOIP
            if (ethAutoip)
                autoip_stop(xnetif);

            ethAutoip = false;

#endif         // CFG_NET_ETHERNET_AUTOIP
        }
    }
    //netif_set_down(xnetif);
    IP4_ADDR(&ipaddr,  setting->ipaddr[0],  setting->ipaddr[1],  setting->ipaddr[2],  setting->ipaddr[3]);
    IP4_ADDR(&netmask, setting->netmask[0], setting->netmask[1], setting->netmask[2], setting->netmask[3]);
    IP4_ADDR(&gw,      setting->gw[0],      setting->gw[1],      setting->gw[2],      setting->gw[3]);

    netif_set_addr(xnetif, &ipaddr, &netmask, &gw);
    netif_set_up(xnetif);
}

static void EthernetDetect(ITPEthernetSetting *setting)
{
    struct netif *xnetif;
    ip_addr_t    ipaddr;

	xnetif = &ethNetifs[setting->index];
    IP4_ADDR(&ipaddr,  setting->ipaddr[0],  setting->ipaddr[1],  setting->ipaddr[2],  setting->ipaddr[3]);

	if (setting->index == (CFG_NET_ETHERNET_COUNT - 1)) //avoid multi -interface
	etharp_request(xnetif, &ipaddr);
}

static int EthernetIoctl(int file, unsigned long request, void *ptr, void *info)
{
    switch (request)
    {
    case ITP_IOCTL_POLL:
        EthernetPoll();
        break;

	case ITP_IOCTL_ARP_REPLY:
		EthernetDetect((ITPEthernetSetting *)ptr);
		break;

	case ITP_IOCTL_IP_DUPLICATE:
		return etharp_ip_conflict();

    case ITP_IOCTL_IS_AVAIL:
#ifdef CFG_NET_ETHERNET_MULTI_INTERFACE
        eth_netif[0] = ethNetifs[0].ip_addr.addr ? 1 : 0;
		eth_netif[1] = ethNetifs[1].ip_addr.addr ? 1 : 0;

        return eth_netif[0] && eth_netif[1];
#else
        return ethNetifs[0].ip_addr.addr ? 1 : 0;
#endif

    case ITP_IOCTL_IS_CONNECTED:
        return netif_is_link_up(&ethNetifs[CFG_NET_ETHERNET_COUNT - 1]);


    case ITP_IOCTL_GET_INFO:
        EthernetGetInfo((ITPEthernetInfo *)ptr);
        break;

    case ITP_IOCTL_INIT:
        return EthernetInit();
        break;

    case ITP_IOCTL_EXIT:
        EthernetExit();
        break;

    case ITP_IOCTL_RESET:
        EthernetReset((ITPEthernetSetting *)ptr);
        break;

    case ITP_IOCTL_RESET_DEFAULT:
        printf("itp ethernet set netif 0 default \n");
        netif_set_default(&ethNetifs[0]);
        break;

    case ITP_IOCTL_ENABLE:
#ifdef CFG_NET_ETHERNET_MULTI_INTERFACE
        netif_set_link_up(&ethNetifs[0]);
        netif_set_link_up(&ethNetifs[1]);
#else
        netif_set_link_up(&ethNetifs[0]);
#endif

        break;

    case ITP_IOCTL_DISABLE:
#ifdef CFG_NET_ETHERNET_MULTI_INTERFACE
        netif_set_link_down(&ethNetifs[0]);
        netif_set_link_down(&ethNetifs[1]);
#else
        netif_set_link_down(&ethNetifs[0]);
#endif

        break;

#if defined(NET_RTL8201FL)
    case ITP_IOCTL_ON:
    #if defined(ITE_MAC)
        if (((int)ptr) == ITP_PHY_WOL)
            PhyWolEnter();
    #endif
        break;

    case ITP_IOCTL_OFF:
    #if defined(ITE_MAC)
        if (((int)ptr) == ITP_PHY_WOL)
            PhyWolExit();
    #endif
        break;
#endif

    case ITP_IOCTL_WIRTE_MAC:
#ifdef CFG_NET_ETHERNET_MAC_ADDR_STORAGE
        MacWrite((uint8_t *)ptr);
#endif
        break;

    default:
        errno = (ITP_DEVICE_ETHERNET << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceEthernet =
{
    ":ethernet",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    EthernetIoctl,
    NULL
};