/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 *  Ethernet Controller extern API implementation.
 *
 * @author Irene Lin
 * @version 1.0
 */
#include "ndis.h"
#include "mac100/config.h"
#include "mac100/mac100.h"
#include "mac100/mac100_reg.h"

//=============================================================================
//                              Global Data Definition
//=============================================================================
static struct eth_device eth_dev;
static struct eth_device *dev = MAC_NULL;


//=============================================================================
//                              Private Function Definition
//=============================================================================

#define MAC_IO_COUNT    10

#if (CFG_CHIP_FAMILY == 9070)
#define MAC_IO_INDEX_OFFSET     15
static const uint8_t gpioMode[] = 
{
    3,  // gpio 15
    3,  // gpio 16
    3,  // gpio 17
    3,  // gpio 18
    0,0,0,0,0,0,  // gpio 19 ~ 24
    3,  // gpio 25
    3,  // gpio 26
    0,0,0, // gpio 27 ~ 29
    3,  // gpio 30
    3,  // gpio 31
    3,  // gpio 32
    3,  // gpio 33
    2,  // gpio 34
    2,  // gpio 35
    2,  // gpio 36
    2,  // gpio 37
    2,  // gpio 38
    2,  // gpio 39
    2,  // gpio 40
    2,  // gpio 41
    2,  // gpio 42
    2,  // gpio 43
};
#endif

static void MacConfig(struct ftmac100 *priv, const MAC_UINT8* ioConfig)
{
    int i;
    for(i=0; i<MAC_IO_COUNT; i++)
    {
#if (CFG_CHIP_FAMILY == 9910) || (CFG_CHIP_FAMILY == 9850)
        ithGpioSetMode(ioConfig[i], ITH_GPIO_MODE3);
        ithGpioSetDriving(ioConfig[i], 0);
#elif (CFG_CHIP_FAMILY == 9070)
        ithGpioSetMode(ioConfig[i], gpioMode[ioConfig[i]-MAC_IO_INDEX_OFFSET]);
#else
        LOG_ERROR " no io setting!!! \n" LOG_END
        while(1);
#endif
    }

    /* for link gpio */
    if(priv->linkGpio>0)
        ithGpioSetMode(priv->linkGpio, ITH_GPIO_MODE0);

    #if defined(LINK_INTR)
    if(priv->linkGpio<0)
        ithGpioSetMode(44, ITH_GPIO_MODE2);
    #endif
    //printf(" 0x94 = 0x%08X \n", ithReadRegA(GPIO_BASE+0x94));
    //printf(" 0x98 = 0x%08X \n", ithReadRegA(GPIO_BASE+0x98));

    // 9910: reg 0x60 = 0x3903, from initial script
    #if (CFG_CHIP_FAMILY == 9070)
    MAC_WriteRegH(0x7901, 0x60);  // reg 0x60 = 0x7901
    #elif (CFG_CHIP_FAMILY == 9850)
    MAC_WriteRegH(0x3903, 0x60);  // reg 0x60 = 0x3903
    #endif
}

static void phy_read_mode(struct eth_device *netdev, MAC_INT* speed, MAC_INT* duplex)
{
    struct ftmac100 *priv = netdev->priv;
    struct mii_if_info *mii_if = &priv->mii;
    MAC_UINT32 status;
    MAC_INT i;

    if (priv->cfg->phy_read_mode)
    {
        status = priv->cfg->phy_read_mode(speed, duplex);
        if (status) /* not link */
            return;
    }
    else
    {
        for (i = 0; i < 2; i++)
            status = mii_if->mdio_read(netdev, mii_if->phy_id, MII_BMSR);

        if (!(status & BMSR_LSTATUS))
            return;

        *speed = SPEED_10;
        *duplex = DUPLEX_HALF;

        /* AutoNegotiate completed */
#if 0
        {
            MAC_UINT16 autoadv, autorec;
            autoadv = mii_if->mdio_read(netdev, mii_if->phy_id, MII_ADVERTISE);
            autorec = mii_if->mdio_read(netdev, mii_if->phy_id, MII_LPA);
            status = autoadv & autorec;

            if (status & (ADVERTISE_100HALF | ADVERTISE_100FULL))
                *speed = SPEED_100;
            if (status & (ADVERTISE_100FULL | ADVERTISE_10FULL))
                *duplex = DUPLEX_FULL;
        }
#else
        {
            MAC_UINT32 ctrl;
            ctrl = mii_if->mdio_read(netdev, mii_if->phy_id, MII_BMCR);
            if (ctrl & BMCR_SPEED100)
                *speed = SPEED_100;
            if (ctrl & BMCR_FULLDPLX)
                *duplex = DUPLEX_FULL;
        }
#endif
    }

    priv->autong_complete = 1;

    ithPrintf(" Link On %s %s \n",
                    *speed == SPEED_100 ? "100Mbps" : "10Mbps",
                    *duplex == DUPLEX_FULL ? "full" : "half");
}


#include "mac100_hw.c"
#include "mac100_desc.c"
#include "mac100_rx.c"
#include "mac100_tx.c"
#include "mac100_buf.c"
#include "mac100_mdio.c"
#include "mac100_intr.c"
#include "mac100_netdev_ops.c"

//=============================================================================
//                              Public Function Definition
//=============================================================================
MAC_INT iteMacInitialize(ITE_MAC_CFG_T* cfg)
{
    MAC_INT rc=0;
    struct ftmac100 *priv;
    LOG_ENTER "[%s]  \n", __FUNCTION__ LOG_END

    if(dev)
        return rc;

    dev = &eth_dev;
    memset(dev, 0, sizeof(struct eth_device));
    priv = (struct ftmac100*)malloc(sizeof(struct ftmac100));
    if(!priv)
    {
        rc = ERROR_MAC_PRIV_MEM_ALLOC_FAIL;
        goto end;
    }
    memset((void*)priv, 0, sizeof(struct ftmac100));
    priv->base = MAC_BASE;
    priv->netdev = dev;
    MAC_CreateSem(priv->tx_lock);

    /* initialize struct mii_if_info */
    priv->mii.phy_id	= cfg->phyAddr;
    priv->mii.phy_id_mask	= 0x1f;
    priv->mii.reg_num_mask	= 0x1f;
    priv->mii.dev		= &eth_dev;
    priv->mii.mdio_read	= ftmac100_mdio_read;
    priv->mii.mdio_write	= ftmac100_mdio_write;
    priv->linkGpio = cfg->linkGpio;
    priv->cfg = cfg;
    dev->priv = priv;
    NETIF_CARRIER_OFF(dev);

    MacConfig(priv, cfg->ioConfig);

#if defined(CLK_FROM_PHY)
{
	MAC_UINT32 maccr=0;

	MAC_ReadReg(priv->base + FTMAC100_OFFSET_MACCR, &maccr);
    maccr |= MAC_REF_CLK_FROM_PHY;
	MAC_WriteReg(maccr, priv->base + FTMAC100_OFFSET_MACCR);
}
#endif

end:
    if(rc)
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END

    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

static int inited = 0;

MAC_INT iteMacOpen(MAC_UINT8* mac_addr, void (*func)(void *arg, void *packet, int len), void* arg, MAC_INT mode)
{
    MAC_INT rc=0;
    struct ftmac100 *priv;
    LOG_ENTER "[%s]  \n", __FUNCTION__ LOG_END

    if(!dev)
        return rc;

    priv = dev->priv;
    priv->netif_rx = func;
    priv->netif_arg = arg;
    priv->mode = mode;
    memset((void*)&dev->stats, 0x0, sizeof(struct net_device_stats));
    printf(" eth mode: %d \n", mode);
    if(mac_addr)
        memcpy(mac_addr, dev->netaddr, 6);

    if (inited)
        return rc;

    if(rc=ftmac100_open(dev, 1))
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END

    inited = 1;

    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

MAC_INT iteMacStop(void)
{
    MAC_INT rc=0;
    LOG_ENTER "[%s]  \n", __FUNCTION__ LOG_END

    if(!dev)
        return rc;

    if(rc=ftmac100_stop(dev, 1))
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END

    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

MAC_INT iteMacSend(void* packet, MAC_UINT32 len)
{
    MAC_INT rc=0;
    LOG_ENTER "[%s] packet 0x%08X, len %d \n", __FUNCTION__, packet, len LOG_END

    if(!dev)
        return rc;

    if(!NETIF_RUNNING(dev))
        return -1;

    if(!NETIF_CARRIER_OK(dev))
    {
        LOG_INFO " tx: no carrier \n" LOG_END
        return -1;
    }

    while(!NETIF_TX_QUEUE_OK(dev)) /* wait for tx queue available */
    {
        printf("&\n");
  	    udelay(50);
    }

    if(rc=ftmac100_hard_start_xmit(packet, len, dev))
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END

    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

MAC_INT iteMacIoctl(struct mii_ioctl_data* data, int cmd)
{
    MAC_INT rc=0;

    if(!dev)
        return rc;

    if(rc=ftmac100_do_ioctl(dev, data, cmd))
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END
    return rc;
}

MAC_INT iteMacSetMacAddr(MAC_UINT8* mac_addr)
{
    MAC_INT rc=0;
    LOG_ENTER "[%s]  \n", __FUNCTION__ LOG_END

    if(!dev)
    {
        rc = ERROR_MAC_NO_DEV;
        goto end;
    }
    if(NETIF_RUNNING(dev))
    {
        rc = ERROR_MAC_DEV_BUSY;
        goto end;
    }

    memcpy(dev->netaddr, mac_addr, 6);

end:
    if(rc)
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END
    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

MAC_INT iteMacSetRxMode(int flag, MAC_UINT8* mc_list, int count)
{
    MAC_INT rc=0;
    LOG_ENTER "[%s]  \n", __FUNCTION__ LOG_END

    if(!dev)
    {
        rc = ERROR_MAC_NO_DEV;
        goto end;
    }

    ithEnterCritical();
    rc = ftmac100_set_rx_mode(dev, flag, mc_list, count);
    ithExitCritical();

end:
    if(rc)
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END
    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

MAC_INT iteMacGetStats(MAC_UINT32* tx_packets, MAC_UINT32* tx_bytes, MAC_UINT32* rx_packets, MAC_UINT32* rx_bytes)
{
    MAC_INT rc=0;

    if(!dev)
    {
        rc = ERROR_MAC_NO_DEV;
        goto end;
    }
    if(tx_packets) (*tx_packets) = dev->stats.tx_packets;
    if(tx_bytes)   (*tx_bytes)   = dev->stats.tx_bytes;
    if(rx_packets) (*rx_packets) = dev->stats.rx_packets;
    if(rx_bytes)   (*rx_bytes)   = dev->stats.rx_bytes;

end:
    if(rc)
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END
    return rc;
}

MAC_INT iteMacSuspend(void)
{
    MAC_INT rc=0;
    LOG_ENTER "[%s]  \n", __FUNCTION__ LOG_END

    if(!dev)
        return rc;
    
    if(rc=ftmac100_stop(dev, 0))
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END

    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

MAC_INT iteMacResume(void)
{
    MAC_INT rc=0;
    LOG_ENTER "[%s]  \n", __FUNCTION__ LOG_END

    if(!dev)
        return rc;

    if(rc=ftmac100_open(dev, 0))
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END

end:
    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

void iteMacSetClock(int clk_inv, int delay)
{
    struct ftmac100 *priv = dev->priv;

    MAC_UINT32 mask = MAC_REF_CLK_INVERT | (0xFF << 20);
    MAC_UINT32 val = clk_inv ? MAC_REF_CLK_INVERT : 0;

    val |= MAC_REF_CLK_DELAY(delay);

    MAC_WriteRegMask(val, priv->base + FTMAC100_OFFSET_MACCR, mask);
}

//=============================================================================
//  APIs for alter and report network device settings.
//=============================================================================
/**
 * is link status up/ok
 *
 * Returns 1 if the device reports link status up/ok, 0 otherwise.
 */
MAC_INT iteEthGetLink(void)
{
    struct ftmac100 *priv;

    if(!dev)
        return 0;

    if(!NETIF_RUNNING(dev))
        return 0;

    priv = (struct ftmac100 *)dev->priv;
    
    if(priv->mode == ITE_ETH_MAC_LB)
        return 1;

#if !defined(LINK_INTR)
    {
        MAC_INT cur_link;
        MAC_INT prev_link = NETIF_CARRIER_OK(dev);

        if (priv->cfg->phy_link_status)
            cur_link = priv->cfg->phy_link_status();
        else
            cur_link = mii_link_ok(&priv->mii);

        if (cur_link && !prev_link)
        {
            mac_set_ctrl_reg(priv);
            NETIF_CARRIER_ON(dev);
        }
        else if (prev_link && !cur_link)
        {
            printf("xxxxx\n");
            NETIF_CARRIER_OFF(dev);
        }
    }
#else
    if (priv->cfg->phy_link_status)
    {
        MAC_INT cur_link;
        MAC_INT prev_link = NETIF_CARRIER_OK(dev);

        cur_link = priv->cfg->phy_link_status();
        if (cur_link && !prev_link)
        {
            mac_set_ctrl_reg(priv);
            NETIF_CARRIER_ON(dev);
        }
        else if (prev_link && !cur_link)
        {
            printf("xxxxx\n");
            NETIF_CARRIER_OFF(dev);
        }
    }
#endif

    return NETIF_CARRIER_OK(dev) ? 1 : 0;
}

MAC_INT iteEthGetLink2(void)
{
    struct ftmac100 *priv;

    if(!dev)
        return 0;
	
    priv = (struct ftmac100 *)dev->priv;

    if (priv->cfg->phy_link_status)
        return priv->cfg->phy_link_status();

	return mii_link_ok(&priv->mii);
}

/**
 * get settings that are specified in @ecmd
 *
 * @ecmd requested ethtool_cmd
 *
 * Returns 0 for success, non-zero on error.
 */
MAC_INT iteEthGetSetting(struct ethtool_cmd *ecmd)
{
    MAC_INT rc=0;
    struct ftmac100 *priv;

    if(!dev)
        return 0;

    priv = (struct ftmac100 *)dev->priv;
    if(rc=mii_ethtool_gset(&priv->mii, ecmd))
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END
    return rc;
}

/**
 * set settings that are specified in @ecmd
 *
 * @ecmd requested ethtool_cmd
 *
 * Returns 0 for success, non-zero on error.
 */
MAC_INT iteEthSetSetting(struct ethtool_cmd *ecmd)
{
    MAC_INT rc=0;
    struct ftmac100 *priv;

    if(!dev)
        return 0;

    priv = (struct ftmac100 *)dev->priv;
    if(rc=mii_ethtool_sset(&priv->mii, ecmd))
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END
    return rc;
}

/**
 * restart NWay (autonegotiation) for this interface
 *
 * Returns 0 for success, non-zero on error.
 */
MAC_INT iteEthNWayRestart(void)
{
    MAC_INT rc=0;
    struct ftmac100 *priv;

    if(!dev)
        return 0;

    priv = (struct ftmac100 *)dev->priv;
    if(rc=mii_nway_restart(&priv->mii))
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END
    return rc;
}


