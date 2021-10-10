
/******************************************************************************
 * internal functions (receive)
 *****************************************************************************/
static MAC_INLINE int ftmac100_next_rx_pointer(int pointer)
{
    return (pointer + 1) & (RX_QUEUE_ENTRIES - 1);
}

static MAC_INLINE void ftmac100_rx_pointer_advance(struct ftmac100 *priv)
{
    priv->rx_pointer = ftmac100_next_rx_pointer(priv->rx_pointer);
}

static MAC_INLINE struct ftmac100_rxdes *ftmac100_current_rxdes(struct ftmac100 *priv)
{
    struct ftmac100_rxdes *rxdes = &priv->descs->rxdes[priv->rx_pointer];

    #if defined(__FREERTOS__) || defined(__OPENRTOS__)
    ithInvalidateDCacheRange(rxdes, sizeof(struct ftmac100_rxdes));
    #endif

    return rxdes;
}

static struct ftmac100_rxdes *
ftmac100_rx_locate_first_segment(struct ftmac100 *priv)
{
    struct ftmac100_rxdes *rxdes = ftmac100_current_rxdes(priv);

    while (!ftmac100_rxdes_owned_by_dma(rxdes)) {
        if (ftmac100_rxdes_first_segment(rxdes))
            return rxdes;

        ftmac100_rxdes_set_dma_own(rxdes);
        ftmac100_rx_pointer_advance(priv);
        rxdes = ftmac100_current_rxdes(priv);
    }

    return MAC_NULL;
}

static MAC_BOOL ftmac100_rx_packet_error(struct ftmac100 *priv,
                     struct ftmac100_rxdes *rxdes)
{
    struct eth_device *netdev = priv->netdev;
    MAC_BOOL error = MAC_FALSE;

    if (unlikely(ftmac100_rxdes_rx_error(rxdes))) {
        LOG_INFO "rx err\n" LOG_END

        netdev->stats.rx_errors++;
        error = MAC_TRUE;
    }

    if (unlikely(ftmac100_rxdes_crc_error(rxdes))) {
        LOG_INFO "rx crc err\n" LOG_END

        netdev->stats.rx_crc_errors++;
        error = MAC_TRUE;
    }

    if (unlikely(ftmac100_rxdes_frame_too_long(rxdes))) {
        LOG_INFO "rx frame too long\n" LOG_END

        netdev->stats.rx_length_errors++;
        error = MAC_TRUE;
    } else if (unlikely(ftmac100_rxdes_runt(rxdes))) {
        LOG_INFO "rx runt\n" LOG_END

        netdev->stats.rx_length_errors++;
        error = MAC_TRUE;
    } else if (unlikely(ftmac100_rxdes_odd_nibble(rxdes))) {
        LOG_INFO "rx odd nibble\n" LOG_END

        netdev->stats.rx_length_errors++;
        error = MAC_TRUE;
    }

    return error;
}

static void ftmac100_rx_drop_packet(struct ftmac100 *priv)
{
    struct eth_device *netdev = priv->netdev;
    struct ftmac100_rxdes *rxdes = ftmac100_current_rxdes(priv);
    MAC_BOOL done = MAC_FALSE;

    LOG_DEBUG "drop packet %p\n", rxdes LOG_END

    do {
        if (ftmac100_rxdes_last_segment(rxdes))
            done = MAC_TRUE;

        ftmac100_rxdes_set_dma_own(rxdes);
        ftmac100_rx_pointer_advance(priv);
        rxdes = ftmac100_current_rxdes(priv);
    } while (!done && !ftmac100_rxdes_owned_by_dma(rxdes));

    netdev->stats.rx_dropped++;
}

static MAC_BOOL ftmac100_rx_packet(struct ftmac100 *priv, int *processed)
{
    struct eth_device *netdev = priv->netdev;
    struct ftmac100_rxdes *rxdes;
    int length;
    MAC_UINT32 rx_buf;

    rxdes = ftmac100_rx_locate_first_segment(priv);
    if (!rxdes)
        return MAC_FALSE;

    if (unlikely(ftmac100_rx_packet_error(priv, rxdes))) {
        ftmac100_rx_drop_packet(priv);
        return MAC_TRUE;
    }

    /*
     * It is impossible to get multi-segment packets
     * because we always provide big enough receive buffers.
     */
    if (unlikely(!ftmac100_rxdes_last_segment(rxdes)))
        BUG();

    /* start processing */
    if (unlikely(ftmac100_rxdes_multicast(rxdes)))
        netdev->stats.multicast++;

    length = ftmac100_rxdes_frame_length(rxdes);
    //rx_buf = ftmac100_rxdes_get_page(rxdes);
    rx_buf = ftmac100_rxdes_get_dma_addr(rxdes); // !!!!!!!!
    #if !defined(CFG_CPU_WB) /* already do this before change owner to dma */
    #if defined (__FREERTOS__) || defined(__OPENRTOS__)
    ithInvalidateDCacheRange((void*)(rx_buf+2), length);
    #endif
    #endif

    //ftmac100_alloc_rx_page(priv, rxdes);

    ftmac100_rx_pointer_advance(priv);

    netdev->stats.rx_packets++;
    netdev->stats.rx_bytes += length;

    /* push packet to protocol stack */
    priv->netif_rx(priv->netif_arg, (void*)(rx_buf+2), length);

    ftmac100_rxdes_set_dma_own(rxdes);

    (*processed)++;
    return MAC_TRUE;
}
