/*
 * Copyright (c) 2017 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 *  Gigabit Ethernet Controller extern API implementation.
 *
 * @author Irene Lin
 * @version 1.0
 */
#include <malloc.h>
#include <linux/os.h>
#include <linux/bitops.h>
#include <linux/util.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "ite/ite_mac.h"

#include "mii.h"
#include "ndis.h"
#include "if_ether.h"
#include "gmac/config.h"
#include "gmac/skb.h"
#include "gmac/ip.h"
#include "gmac/ftgmac030.h"
#include "gmac/gmac_err.h"

//=============================================================================
//                              Global Data Definition
//=============================================================================
static struct eth_device eth_dev;
static struct eth_device *dev = NULL;


//=============================================================================
//                              sate Function Definition
//=============================================================================
static void MacConfig(struct ftgmac030_ctrl *ctrl, const uint8_t* ioConfig)
{
    int i, count=0;
    int io_cnt = ctrl->is_grmii ? ITE_MAC_GRMII_PIN_CNT : ITE_MAC_RMII_PIN_CNT;

    for (i = 0; i<io_cnt; i++) {
        ithGpioSetMode(ioConfig[i], ITH_GPIO_MODE1);

        if (ioConfig[i] == 36)
            ctrl->use_gpio36 = true;
    }
	if(1)
	{ // test
		ithWriteRegA(0xD100006C, 0x11111110);  // GPIO[31:25] mode setting
		//ithWriteRegA(0xD10000E0, 0x00000111);  // GPIO[39:32] mode setting
		ithWriteRegA(0xD10000E0, 0x11111111);  // GPIO[39:32] mode setting
	}
	if (1) {
		ithWriteRegMaskA((uint32_t)ctrl->io_base + FTGMAC030_REG_CLKDLY, 0x400, 0x400);
		printf("0x%08X = 0x%08X \n", (ctrl->io_base + FTGMAC030_REG_CLKDLY), ithReadRegA(ctrl->io_base + FTGMAC030_REG_CLKDLY));
	}
    if (ctrl->is_grmii)
        iow32(FTGMAC030_REG_GISR, 0x2);  // D[1:0] RGMII = 10
    else {
        iow32(FTGMAC030_REG_GISR, 0x1);  // D[1:0] RMII = 01

        // fast ethernet choose GPIO32-35 or GPIO36-39
        if (ctrl->use_gpio36 == true)
            ithWriteRegMaskA((uint32_t)ctrl->io_base + FTGMAC030_REG_CLKDLY, FTGMAC030_GPIO36_39, FTGMAC030_GPIO36_39);
    }
	printf("0x%08X = 0x%08X \n", (ctrl->io_base + FTGMAC030_REG_CLKDLY), ithReadRegA(ctrl->io_base + FTGMAC030_REG_CLKDLY));


#if defined(CFG_NET_ETHERNET_LINK_INTR)
    /* for link gpio */
	printf("link gpio \n");
    if(ctrl->cfg->linkGpio>0)
        ithGpioSetMode(ctrl->cfg->linkGpio, ITH_GPIO_MODE0);
#endif

    /* enable gmac controller's clock setting */
    // TODO
}

static void phy_read_mode(struct eth_device *netdev, int* speed, int* duplex)
{
    struct ftgmac030_ctrl *ctrl = netdev->priv;
    struct mii_if_info *mii_if = &ctrl->mii;
    uint32_t status;
    int i;
    char *speed_string[3] = { "1000Mbps", "100Mbps", "10Mbps" };
    char *str;

    if (ctrl->cfg->phy_read_mode) {
        status = ctrl->cfg->phy_read_mode(speed, duplex);
        if (status) /* not link */
            return;
    } else {
        for (i = 0; i < 2; i++)
            status = mii_if->mdio_read(netdev, mii_if->phy_id, MII_BMSR);

        if (!(status & BMSR_LSTATUS))
            return;

        *speed = SPEED_10;
        *duplex = DUPLEX_HALF;
        str = speed_string[2];

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
            uint32_t ctrl;
            ctrl = mii_if->mdio_read(netdev, mii_if->phy_id, MII_BMCR);
            if (ctrl & BMCR_SPEED1000) {
                *speed = SPEED_1000;
                str = speed_string[0];
            }
            else if (ctrl & BMCR_SPEED100) {
                *speed = SPEED_100;
                str = speed_string[1];
            }

            if (ctrl & BMCR_FULLDPLX)
                *duplex = DUPLEX_FULL;
        }
#endif
    }

    ctrl->autong_complete = 1;

    ithPrintf(" Link On %s %s \n", str, *duplex == DUPLEX_FULL ? "full" : "half");
}


#include "gmac.c"
#include "gmac_intr.c"
#include "gmac_netdev_ops.c"
#include "gmac_mdio.c"


//=============================================================================
//                              Public Function Definition
//=============================================================================
int iteMacInitialize(ITE_MAC_CFG_T* cfg)
{
    int rc=0;
    struct ftgmac030_ctrl *ctrl = NULL;
    unsigned long osclk;

    LOG_ENTER "[%s]  \n", __FUNCTION__ LOG_END

    if (dev)
        return rc;

    dev = &eth_dev;
    memset(dev, 0, sizeof(struct eth_device));
    ctrl = (struct ftgmac030_ctrl*)malloc(sizeof(struct ftgmac030_ctrl));
    if (!ctrl) {
        rc = ERROR_MAC_PRIV_MEM_ALLOC_FAIL;
        goto end;
    }
    memset((void*)ctrl, 0, sizeof(struct ftgmac030_ctrl));
    dev->priv = ctrl;
    ctrl->netdev = dev;
    ctrl->max_hw_frame_size = DEFAULT_JUMBO;
    ctrl->io_base = ITH_GMAC_BASE;
	ctrl->cfg = cfg;
	ctrl->is_grmii = cfg->flags & ITE_MAC_GRMII;
	printf("\n #### %s Ethernet #### \n\n", ctrl->is_grmii ? "Giga" : "Fast");

    ftgmac030_show_feature(ctrl);

    /* See ftgmac030.h for more flags */
    ctrl->flags = FLAG_HAS_JUMBO_FRAMES;

#if defined(PTP_ENABLE)
    if (ior32(FTGMAC030_REG_FEAR) & FTGMAC030_FEAR_PTP)
        ctrl->flags |= FLAG_HAS_HW_TIMESTAMP;
#endif

    ctrl->itr_setting = 3;
    ctrl->mta_reg_count = 4;
    /* Transmit Interrupt Delay in cycle units:
    *  1000 Mbps mode -> 16.384 繕s
    *  100 Mbps mode -> 81.92 繕s
    *  10 Mbps mode -> 819.2 繕s
    */
    ctrl->tx_int_delay = 8;


    /* Set initial default active device features */
    dev->features = (NETIF_F_RXCSUM | NETIF_F_HW_CSUM);
    //dev->features = 0;

    dev->flags = IFF_BROADCAST/* | IFF_MULTICAST*/;
    dev->mtu = 1500;

    /* Set user-changeable features (subset of all device features) */
    dev->hw_features = dev->features;
    //dev->hw_features |= NETIF_F_RXFCS;
    //dev->hw_features |= NETIF_F_RXALL;

	osclk = 40;// 400;  // AXI bus: 400MHz TODO
    /* The MDC period = MDC_CYCTHR x system clock period.
    * Users must set the correct value before using MDC.
    * Note: IEEE 802.3 specifies minimum cycle is 400ns of MDC
    */
    ctrl->mdc_cycthr = (400 * osclk) / 1000;

	/* setup ftgmac030_ctrl struct */
	rc = ftgmac030_sw_init(ctrl);
	if (rc)
		goto err_sw_init;

    /* reset the hardware with the new settings */
    ftgmac030_reset_hw(ctrl);

    rc = ftgmac030_mii_init(ctrl);
    if (rc)
        goto err_init_miibus;

    /* carrier off reporting is important to ethtool even BEFORE open */
    netif_carrier_off(dev);

    MacConfig(ctrl, cfg->ioConfig);

    return 0;

err_init_miibus:
err_sw_init:
end:
    if (ctrl)
        free(ctrl);

    check_result(rc);
    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

static int inited = 0;

int iteMacOpen(uint8_t* mac_addr, void (*func)(void *arg, void *packet, int len), void* arg, int mode)
{
    int rc=0;
    struct ftgmac030_ctrl *ctrl;
    LOG_ENTER "[%s]  \n", __FUNCTION__ LOG_END

    if (!dev)
        return rc;

    ctrl = dev->priv;
    ctrl->netif_rx = func;
    ctrl->netif_arg = arg;
    ctrl->mode = mode;
    memset((void*)&dev->stats, 0x0, sizeof(struct net_device_stats));
    printf(" eth mode: %d \n", mode);
    if(mac_addr)
        memcpy(mac_addr, dev->netaddr, 6);

    if (inited)
        return rc;

	if (ctrl->mode != ITE_ETH_REAL) {
		dev->features &= ~(NETIF_F_RXCSUM | NETIF_F_HW_CSUM);
		dev->features |= NETIF_F_RXALL;
	}
	
    if (rc = ftgmac030_open(dev))
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END

	netif_start(dev);
    inited = 1;

    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
	check_result(rc);
	

    return rc;
}

int iteMacStop(void)
{
    int rc=0;
    LOG_ENTER "[%s]  \n", __FUNCTION__ LOG_END

    if(!dev)
        return rc;

    if(rc=ftgmac030_close(dev))
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END

	check_result(rc);
	LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

int iteMacSend(void* packet, uint32_t len)
{
    int rc=0;
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

    if (rc = ftgmac030_xmit_frame(packet, len, dev))
        LOG_ERROR "[%s] rc = 0x%08X(%d) \n", __FUNCTION__, rc, rc LOG_END

	check_result(rc);
    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

int iteMacIoctl(struct mii_ioctl_data* data, int cmd)
{
    int rc=0;

    if(!dev)
        return rc;

    if(rc=ftgmac030_ioctl(dev, data, cmd))
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END

	check_result(rc);

    return rc;
}

int iteMacSetMacAddr(uint8_t* mac_addr)
{
    int rc=0;
    LOG_ENTER "[%s]  \n", __FUNCTION__ LOG_END

    if(!dev)
    {
        rc = ERROR_MAC_NO_DEV;
        goto end;
    }
    if (netif_running(dev))
    {
        rc = ERROR_MAC_DEV_BUSY;
        goto end;
    }

    memcpy(dev->netaddr, mac_addr, 6);

end:
    check_result(rc);

    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

int iteMacSetRxMode(int flag, uint8_t* mc_list, int count)
{
    int rc=0;
	struct ftgmac030_ctrl *ctrl;
    LOG_ENTER "[%s]  \n", __FUNCTION__ LOG_END

    if(!dev)
    {
        rc = ERROR_MAC_NO_DEV;
        goto end;
    }

	ctrl = netdev_priv(dev);

    dev->mc_list = mc_list;
	dev->mc_count = count;
 
    if ((flag & (IFF_MULTICAST|IFF_ALLMULTI)) && (ctrl->mode != ITE_ETH_REAL))
		dev->features &= ~NETIF_F_RXALL;

	dev->flags = flag;
    ithEnterCritical();
    ftgmac030_set_rx_mode(dev);
    ithExitCritical();

end:
    check_result(rc);

    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

int iteMacChangeMtu(int mtu)
{
    int rc=0;
    LOG_ENTER "[%s]  \n", __FUNCTION__ LOG_END

    if(!dev)
    {
        rc = ERROR_MAC_NO_DEV;
        goto end;
    }

    ftgmac030_change_mtu(dev, mtu);

end:
    check_result(rc);

    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

int iteMacGetStats(uint32_t* tx_packets, uint32_t* tx_bytes, uint32_t* rx_packets, uint32_t* rx_bytes)
{
    int rc=0;

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
    check_result(rc);
    return rc;
}

int iteMacSuspend(void)
{
    int rc=0;
	struct ftgmac030_ctrl *ctrl;
    LOG_ENTER "[%s]  \n", __FUNCTION__ LOG_END

    if(!dev)
        return rc;
    
    ctrl = netdev_priv(dev);

	if (netif_running(dev)) {
		int count = FTGMAC030_CHECK_RESET_COUNT;

		while (test_bit(__FTGMAC030_RESETTING, &ctrl->state) && count--)
			usleep(15000);

		WARN_ON(test_bit(__FTGMAC030_RESETTING, &ctrl->state));

		/* Quiesce the device without resetting the hardware */
		ftgmac030_down(ctrl, false);
		ftgmac030_free_irq(ctrl);
	}

    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

int iteMacResume(void)
{
    int rc=0;
	struct ftgmac030_ctrl *ctrl;
    LOG_ENTER "[%s]  \n", __FUNCTION__ LOG_END

    if(!dev)
        return rc;

    ctrl = netdev_priv(dev);

	ftgmac030_reset(ctrl);

	if (netif_running(dev)) {
		u32 err = ftgmac030_request_irq(ctrl);
		if (err)
			return err;

		ftgmac030_up(ctrl);
	}

end:
    LOG_LEAVE "[%s]  \n", __FUNCTION__ LOG_END
    return rc;
}

/* this setting only for fast ethernet (RMII) */
void iteMacSetClock(int clk_inv, int refclk_delay, int rxd_delay)
{
    struct ftgmac030_ctrl *ctrl = netdev_priv(dev);

    uint32_t val = clk_inv ? FTGMAC030_REFCLK_INV : 0;

    val |= FTGMAC030_REFCLK_DELAY(refclk_delay);
    val |= FTGMAC030_RXD_DELAY(rxd_delay);

    ithWriteRegMaskA(ctrl->io_base + FTGMAC030_REG_CLKDLY, val, FTGMAC030_CLKDLY_MSK);
}

//=============================================================================
//  APIs for alter and report network device settings.
//=============================================================================
/**
 * is link status up/ok
 *
 * Returns 1 if the device reports link status up/ok, 0 otherwise.
 */
int iteEthGetLink(void)
{
    struct ftgmac030_ctrl *ctrl;

    if(!dev)
        return 0;

    if(!NETIF_RUNNING(dev))
        return 0;

    ctrl = netdev_priv(dev);

	if ((ctrl->mode == ITE_ETH_MAC_LB) || (ctrl->mode == ITE_ETH_MAC_LB_1000))
        return 1;

#if defined(CFG_NET_ETHERNET_LINK_INTR)
    if (ctrl->cfg->phy_link_status) /* for 8304mb switch, down/up hw here not in isr */
    {
        int new_link;
        int prev_link = NETIF_CARRIER_OK(dev);
		int speed, duplex;

        new_link = ctrl->cfg->phy_link_status();
        phy_read_mode(dev, &speed, &duplex);

        if ((prev_link == new_link) &&
            (ctrl->phy_duplex == duplex) &&
            (ctrl->phy_speed == speed))
            return;

        ithPrintf("new speed %d, old speed %d, link %s\n",
            speed, ctrl->phy_speed, new_link ? "up" : "down");

        if (!test_bit(__FTGMAC030_DOWN, &ctrl->state))
            ftgmac030_down(ctrl, true);

        ctrl->phy_speed = speed;
        ctrl->phy_duplex = duplex;
        ctrl->phy_link = new_link;

        if (new_link) {
            ftgmac030_up(ctrl);
            NETIF_CARRIER_ON(dev);
        }
        else
            NETIF_CARRIER_OFF(dev);
    }
#else
	{	// polling from application layer
		int new_link;
		int prev_link = NETIF_CARRIER_OK(dev);
		int speed, duplex;

		if (ctrl->cfg->phy_link_status)
			new_link = ctrl->cfg->phy_link_status();
		else
			new_link = mii_link_ok(&ctrl->mii);

#if 1 // for not print message // TODO
		if (prev_link == new_link)
			return;

		phy_read_mode(dev, &speed, &duplex);
#else

        phy_read_mode(dev, &speed, &duplex);

        if ((prev_link == new_link) &&
            (ctrl->phy_duplex == duplex) &&
            (ctrl->phy_speed == speed))
            return;
#endif
        ithPrintf("new speed %d, old speed %d, link %s\n",
            speed, ctrl->phy_speed, new_link ? "up" : "down");

        if (!test_bit(__FTGMAC030_DOWN, &ctrl->state))
            ftgmac030_down(ctrl, true);

        ctrl->phy_speed = speed;
        ctrl->phy_duplex = duplex;
        ctrl->phy_link = new_link;

        if (new_link) {
            ftgmac030_up(ctrl);
            NETIF_CARRIER_ON(dev);
        }
        else
            NETIF_CARRIER_OFF(dev);
	}
#endif

    return NETIF_CARRIER_OK(dev) ? 1 : 0;
}

int iteEthGetLink2(void)
{
    struct ftgmac030_ctrl *ctrl;

    if(!dev)
        return 0;
	
    ctrl = (struct ftgmac030_ctrl *)dev->priv;

    if (ctrl->cfg->phy_link_status)
        return ctrl->cfg->phy_link_status();

	return mii_link_ok(&ctrl->mii);
}

/**
 * get settings that are specified in @ecmd
 *
 * @ecmd requested ethtool_cmd
 *
 * Returns 0 for success, non-zero on error.
 */
int iteEthGetSetting(struct ethtool_cmd *ecmd)
{
    int rc=0;
    struct ftgmac030_ctrl *ctrl;

    if(!dev)
        return 0;

    ctrl = (struct ftgmac030_ctrl *)dev->priv;
    if(rc=mii_ethtool_gset(&ctrl->mii, ecmd))
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
int iteEthSetSetting(struct ethtool_cmd *ecmd)
{
    int rc=0;
    struct ftgmac030_ctrl *ctrl;

    if(!dev)
        return 0;

    ctrl = (struct ftgmac030_ctrl *)dev->priv;
    if(rc=mii_ethtool_sset(&ctrl->mii, ecmd))
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END
    return rc;
}

/**
 * restart NWay (autonegotiation) for this interface
 *
 * Returns 0 for success, non-zero on error.
 */
int iteEthNWayRestart(void)
{
    int rc=0;
    struct ftgmac030_ctrl *ctrl;

    if(!dev)
        return 0;

    ctrl = (struct ftgmac030_ctrl *)dev->priv;
    if(rc=mii_nway_restart(&ctrl->mii))
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END
    return rc;
}


