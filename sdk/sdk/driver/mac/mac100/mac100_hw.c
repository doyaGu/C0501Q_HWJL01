
/******************************************************************************
 * internal functions (hardware register access)
 *****************************************************************************/
#define INT_MASK_ALL_ENABLED	(FTMAC100_INT_RPKT_FINISH	| \
                 FTMAC100_INT_NORXBUF		| \
                 FTMAC100_INT_XPKT_OK		| \
                 FTMAC100_INT_XPKT_LOST		| \
                 FTMAC100_INT_RPKT_LOST		| \
                 FTMAC100_INT_AHB_ERR)

#define INT_MASK_ALL_DISABLED	0

static void ftmac100_enable_all_int(struct ftmac100 *priv)
{
    MAC_UINT32 intr = INT_MASK_ALL_ENABLED;

    #if defined(LINK_INTR)
    if(priv->linkGpio<0) /* use mac's interrupt */
        intr |= FTMAC100_INT_PHYSTS_CHG;
    #endif

    MAC_WriteReg(intr, priv->base + FTMAC100_OFFSET_IMR);
}

static void ftmac100_disable_all_int(struct ftmac100 *priv)
{
    MAC_WriteReg(INT_MASK_ALL_DISABLED, priv->base + FTMAC100_OFFSET_IMR);
}

static void ftmac100_set_rx_ring_base(struct ftmac100 *priv, MAC_UINT32 addr)
{
    MAC_WriteReg(addr, priv->base + FTMAC100_OFFSET_RXR_BADR);
}

static void ftmac100_set_tx_ring_base(struct ftmac100 *priv, MAC_UINT32 addr)
{
    MAC_WriteReg(addr, priv->base + FTMAC100_OFFSET_TXR_BADR);
}

static void ftmac100_txdma_start_polling(struct ftmac100 *priv)
{
    MAC_WriteReg(1, priv->base + FTMAC100_OFFSET_TXPD);
}

static int ftmac100_reset(struct ftmac100 *priv)
{
    struct eth_device *netdev = priv->netdev;
    int i;

    /* NOTE: reset clears all registers */
    MAC_WriteReg(FTMAC100_MACCR_SW_RST, priv->base + FTMAC100_OFFSET_MACCR);

    for (i = 0; i < 5; i++) {
        MAC_UINT32 maccr;

        MAC_ReadReg(priv->base + FTMAC100_OFFSET_MACCR, &maccr);
        if (!(maccr & FTMAC100_MACCR_SW_RST)) {
            /*
             * FTMAC100_MACCR_SW_RST cleared does not indicate
             * that hardware reset completed (what the f*ck).
             * We still need to wait for a while.
             */
            udelay(500);
            return 0;
        }

        udelay(1000);
    }

    LOG_ERROR "software reset failed\n" LOG_END
    return ERROR_MAC_RESET_TIMEOUT;
}

static void ftmac100_set_mac(struct ftmac100 *priv, const unsigned char *mac)
{
    MAC_UINT32 maddr = mac[0] << 8 | mac[1];
    MAC_UINT32 laddr = mac[2] << 24 | mac[3] << 16 | mac[4] << 8 | mac[5];

    MAC_WriteReg(maddr, priv->base + FTMAC100_OFFSET_MAC_MADR);
    MAC_WriteReg(laddr, priv->base + FTMAC100_OFFSET_MAC_LADR);
}

#define MACCR_ENABLE_ALL	(FTMAC100_MACCR_XMT_EN	| \
                 FTMAC100_MACCR_RCV_EN	| \
                 FTMAC100_MACCR_XDMA_EN	| \
                 FTMAC100_MACCR_RDMA_EN	| \
                 FTMAC100_MACCR_CRC_APD	| \
                 FTMAC100_MACCR_RX_RUNT	| \
                 FTMAC100_MACCR_RX_BROADPKT | \
                 MAC_ENDIAN )

static MAC_INLINE void mac_set_ctrl_reg(struct ftmac100 *priv)
{
    MAC_INT speed, duplex;
    MAC_UINT32 maccr = MACCR_ENABLE_ALL;

    if(priv->mode == ITE_ETH_MAC_LB)
    {
        maccr |= FTMAC100_MACCR_LOOP_EN;
        maccr |= FTMAC100_MACCR_SPEED_100;
        maccr |= FTMAC100_MACCR_FULLDUP;
        NETIF_CARRIER_ON(priv->netdev);
    }
    else
    {
        phy_read_mode(priv->netdev, &speed, &duplex);

        if(speed == SPEED_100)
            maccr |= FTMAC100_MACCR_SPEED_100;

        if(duplex == DUPLEX_FULL)
            maccr |= FTMAC100_MACCR_FULLDUP;
        else
            maccr |= FTMAC100_MACCR_ENRX_IN_HALFTX;
    }

    if(priv->mode != ITE_ETH_REAL)
    {
        maccr |= FTMAC100_MACCR_RCV_ALL;
        #if defined(MAC_CRC_DIS)
        maccr |= FTMAC100_MACCR_CRC_DIS;
        #endif
    }

#if defined(CLK_FROM_PHY)
    maccr |= MAC_REF_CLK_FROM_PHY;
#endif
    if (priv->cfg->clk_inv)
    maccr |= MAC_REF_CLK_INVERT;

    maccr |= MAC_REF_CLK_DELAY(priv->cfg->clk_delay);
    
#if defined(MULTICAST)
    /* see <if.h> IFF_PROMISC, IFF_ALLMULTI, IFF_MULTICAST */
    if(priv->rx_flag & IFF_PROMISC)
        maccr |= FTMAC100_MACCR_RCV_ALL;

    if(priv->rx_flag & IFF_ALLMULTI)
        maccr |= FTMAC100_MACCR_RX_MULTIPKT;

    if(priv->rx_flag & IFF_MULTICAST)
        maccr |= FTMAC100_MACCR_HT_MULTI_EN;

    MAC_WriteReg(priv->mc_ht0, priv->base + FTMAC100_OFFSET_MAHT0);
    MAC_WriteReg(priv->mc_ht1, priv->base + FTMAC100_OFFSET_MAHT1);
#endif

    MAC_WriteReg(maccr, priv->base + FTMAC100_OFFSET_MACCR);
}

static int ftmac100_start_hw(struct ftmac100 *priv)
{
    struct eth_device *netdev = priv->netdev;
    int rc=0;

    if (rc=ftmac100_reset(priv))
        goto err;

    /* setup ring buffer base registers */
    ftmac100_set_rx_ring_base(priv,
                  (MAC_UINT32)priv->descs->rxdes);
    ftmac100_set_tx_ring_base(priv,
                  (MAC_UINT32)priv->descs->txdes);

    MAC_WriteReg(FTMAC100_APTC_RXPOLL_CNT(1), priv->base + FTMAC100_OFFSET_APTC);

    ftmac100_set_mac(priv, netdev->netaddr);

    /* setup control register */
    mac_set_ctrl_reg(priv);
    return 0;

err:
    if(rc)
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END
    return rc;
}

static void ftmac100_stop_hw(struct ftmac100 *priv)
{
    MAC_UINT32 maccr = MAC_ENDIAN;

#if defined(CLK_FROM_PHY)
    maccr |= MAC_REF_CLK_FROM_PHY;
#endif
    MAC_WriteReg(maccr, priv->base + FTMAC100_OFFSET_MACCR);
}

