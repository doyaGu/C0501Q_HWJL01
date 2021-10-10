

static int ftgmac030_xmit_frame(void* packet, uint32_t length, struct eth_device *netdev)
{
    struct ftgmac030_ctrl *ctrl = netdev_priv(netdev);
    struct ftgmac030_ring *tx_ring = ctrl->tx_ring;
    unsigned int first, len, nr_frags;
    int count;
    unsigned int f;
    unsigned long flags;
    struct sk_buff *skb;

    if (test_bit(__FTGMAC030_DOWN, &ctrl->state))
        return 0;

    if (length <= 0)
        return 0;

    skb = __netdev_alloc_skb(netdev, 0, 0);
    skb->data = packet;
    skb->len = length;
#if 0 // for checksum
    skb.ip_summed = CHECKSUM_PARTIAL;
#endif

    /* If skb is linear, the length of skb->data is skb->len.
    * If skb is not linear (i.e., skb->data_len != 0), the length
    * of skb->data is (skb->len) - (skb->data_len) for the head ONLY.
    *
    * The minimum packet size is 60 bytes not include CRC so
    * pad skb in order to meet this minimum size requirement.
    */
    if (unlikely(skb->len < ETH_ZLEN))
        skb->len = ETH_ZLEN;

    len = skb_headlen(skb);

    /*
    * Check if have enough descriptors otherwise try next time
    */
    count = DIV_ROUND_UP(len, ctrl->tx_fifo_limit);

    if (!spin_trylock_irqsave(&tx_ring->ntu_lock, flags))
        return -2;

    if (ftgmac030_maybe_stop_tx(tx_ring, count)) {
        spin_unlock_irqrestore(&tx_ring->ntu_lock, flags);
        return -3;
    }

#if 0
    /* FTGMAC030 can not disable append CRC per packet */
    if (unlikely(skb->no_fcs)) {
        u32 maccr;

        maccr = ior32(FTGMAC030_REG_MACCR);
        maccr &= ~FTGMAC030_MACCR_CRC_APD;
        iow32(FTGMAC030_REG_MACCR, maccr);
    }
#endif

    first = tx_ring->next_to_use;

    /* if count is 0 then mapping error has occurred */
    count = ftgmac030_tx_map(tx_ring, skb, ctrl->tx_fifo_limit, first);
    if (count) {
		#if defined(PTP_ENABLE)
        if (unlikely((skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP) &&
            !ctrl->tx_hwtstamp_skb)) {
            skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
            ctrl->tx_hwtstamp_skb = skb_get(skb);
            ctrl->tx_hwtstamp_start = jiffies;
        }
        else {
            skb_tx_timestamp(skb);
        }

        /* If NETIF_F_LLTX is set, trans_start not updated.
        * See: include/linux/netdevice.h, txq_trans_update function.
        */
        txq = netdev_get_tx_queue(netdev, 0);
        txq->trans_start = jiffies;
		#endif  // #if defined(PTP_ENABLE)

        tx_ring->next_to_use += count;
        if (tx_ring->next_to_use >= tx_ring->count)
            tx_ring->next_to_use -= tx_ring->count;

		#if 0 // we check it before a new tx packet */
        /* Make sure there is space in the ring for the next send. */
        ftgmac030_maybe_stop_tx(tx_ring,
            (MAX_SKB_FRAGS *
            DIV_ROUND_UP(PAGE_SIZE,
            ctrl->tx_fifo_limit) + 2));
		#endif
    }
    else {
        dev_kfree_skb_any(skb);
        tx_ring->buffer_info[first].time_stamp = 0;
        tx_ring->next_to_use = first;
    }

    spin_unlock_irqrestore(&tx_ring->ntu_lock, flags);

    return 0;
}

static int ftgmac030_open(struct eth_device *netdev)
{
    struct ftgmac030_ctrl *ctrl = netdev_priv(netdev);
    int err;

    ftgmac030_reset(ctrl);

	MacConfig(ctrl, ctrl->cfg->ioConfig);

    /* allocate transmit descriptors */
    err = ftgmac030_setup_tx_resources(ctrl->tx_ring);
    if (err)
        goto err_setup_tx;

    /* allocate receive descriptors */
    err = ftgmac030_setup_rx_resources(ctrl->rx_ring);
    if (err)
        goto err_setup_rx;

    ftgmac030_configure(ctrl);

    err = ftgmac030_request_irq(ctrl);
    if (err)
        goto err_req_irq;

    /* From here on the code is the same as ftgmac030_up() */
    clear_bit(__FTGMAC030_DOWN, &ctrl->state);

    ftgmac030_irq_enable(ctrl);

    netif_start_queue(netdev);

	if ((ctrl->mode == ITE_ETH_MAC_LB) || (ctrl->mode == ITE_ETH_MAC_LB_1000))
		netif_carrier_on(netdev);

    return 0;


err_req_irq:
    ftgmac030_free_rx_resources(ctrl->rx_ring);
err_setup_rx:
    ftgmac030_free_tx_resources(ctrl->tx_ring);
err_setup_tx:

    return err;
}

/**
* ftgmac030_close - Disables a network interface
* @netdev: network interface device structure
*
* Returns 0, this is not allowed to fail
*
* The close entry point is called when an interface is de-activated
* by the OS.  The hardware is still under the drivers control, but
* needs to be disabled.  A global MAC reset is issued to stop the
* hardware, and all transmit and receive resources are freed.
**/
static int ftgmac030_close(struct eth_device *netdev)
{
    struct ftgmac030_ctrl *ctrl = netdev_priv(netdev);
    int count = FTGMAC030_CHECK_RESET_COUNT;

    //while (test_bit(__FTGMAC030_RESETTING, &ctrl->state) && count--)
    //    usleep_range(10000, 20000);

    WARN_ON(test_bit(__FTGMAC030_RESETTING, &ctrl->state));

    if (!test_bit(__FTGMAC030_DOWN, &ctrl->state)) {
        ftgmac030_down(ctrl, true);
        ftgmac030_free_irq(ctrl);

        /* Link status message must follow this format */
        e_info(ifdown, "%s Interface is Down\n", "ethernet");
    }

    ftgmac030_free_tx_resources(ctrl->tx_ring);
    ftgmac030_free_rx_resources(ctrl->rx_ring);

    return 0;
}

/**
 * ftgmac030_change_mtu - Change the Maximum Transfer Unit
 * @netdev: network interface device structure
 * @new_mtu: new value for maximum frame size
 *
 * Returns 0 on success, negative on failure
 **/
static int ftgmac030_change_mtu(struct eth_device *netdev, int new_mtu)
{
	struct ftgmac030_ctrl *ctrl = netdev_priv(netdev);
	int max_frame = new_mtu + VLAN_HLEN + ETH_HLEN + ETH_FCS_LEN;

	/* Jumbo frame support */
	if ((max_frame > ETH_FRAME_LEN + ETH_FCS_LEN) &&
	    !(ctrl->flags & FLAG_HAS_JUMBO_FRAMES)) {
		e_err(drv, "Jumbo Frames not supported.\n");
		return -88;
	}

	/* Supported frame sizes */
	if ((new_mtu < ETH_ZLEN + ETH_FCS_LEN + VLAN_HLEN) ||
	    (max_frame > ctrl->max_hw_frame_size)) {
		e_err(drv, "Unsupported MTU setting\n");
		return -99;
	}

	while (test_and_set_bit(__FTGMAC030_RESETTING, &ctrl->state))
		usleep(2000);
	/* ftgmac030_down -> ftgmac030_reset dependent on max_frame_size & mtu */
	ctrl->max_frame_size = max_frame;
	e_info(drv, "changing MTU from %d to %d\n", netdev->mtu, new_mtu);
	netdev->mtu = new_mtu;

	if (netif_running(netdev))
		ftgmac030_down(ctrl, true);

	/* NOTE: netdev_alloc_skb reserves 16 bytes, and typically NET_IP_ALIGN
	 * means we reserve 2 more, this pushes us to allocate from the next
	 * larger slab size.
	 * i.e. RXBUFFER_2048 --> size-4096 slab
	 * However with the new *_jumbo_rx* routines, jumbo receives will use
	 * fragmented skbs
	 */

	if (max_frame <= 2048)
		ctrl->rx_buffer_len = 2048;
	else
		ctrl->rx_buffer_len = 4096;

	/* adjust allocation if LPE protects us, and we aren't using SBP */
	if ((max_frame == ETH_FRAME_LEN + ETH_FCS_LEN) ||
	    (max_frame == ETH_FRAME_LEN + VLAN_HLEN + ETH_FCS_LEN))
		ctrl->rx_buffer_len = ETH_FRAME_LEN + VLAN_HLEN + ETH_FCS_LEN;

	if (netif_running(netdev))
		ftgmac030_up(ctrl);
	else
		ftgmac030_reset(ctrl);

	clear_bit(__FTGMAC030_RESETTING, &ctrl->state);

	return 0;
}

static int ftgmac030_ioctl(struct eth_device *netdev, struct mii_ioctl_data* data, int cmd)
{
	struct ftgmac030_ctrl *ctrl = netdev_priv(netdev);

	switch (cmd) {
#if 0
	case SIOCSHWTSTAMP:
		return ftgmac030_hwtstamp_set(netdev, ifr);
	case SIOCGHWTSTAMP:
		return ftgmac030_hwtstamp_get(netdev, ifr);
#endif		
	default:
		return generic_mii_ioctl(&ctrl->mii, data, cmd, NULL);
	}
}

