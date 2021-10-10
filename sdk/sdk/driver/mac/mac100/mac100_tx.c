
/******************************************************************************
 * internal functions (transmit)
 *****************************************************************************/

static MAC_INLINE int ftmac100_next_tx_pointer(int pointer)
{
    return (pointer + 1) & (TX_QUEUE_ENTRIES - 1);
}

static MAC_INLINE void ftmac100_tx_pointer_advance(struct ftmac100 *priv)
{
    priv->tx_pointer = ftmac100_next_tx_pointer(priv->tx_pointer);
}

static MAC_INLINE void ftmac100_tx_clean_pointer_advance(struct ftmac100 *priv)
{
    priv->tx_clean_pointer = ftmac100_next_tx_pointer(priv->tx_clean_pointer);
}

static MAC_INLINE struct ftmac100_txdes *ftmac100_current_txdes(struct ftmac100 *priv)
{
    return &priv->descs->txdes[priv->tx_pointer];
}

static MAC_INLINE struct ftmac100_txdes *ftmac100_current_clean_txdes(struct ftmac100 *priv)
{
    struct ftmac100_txdes *txdes = &priv->descs->txdes[priv->tx_clean_pointer];

    #if defined (__FREERTOS__) || defined(__OPENRTOS__)
    ithInvalidateDCacheRange(txdes, sizeof(struct ftmac100_txdes));
    #endif

    return txdes;
}

static MAC_BOOL ftmac100_tx_complete_packet(struct ftmac100 *priv)
{
    struct eth_device *netdev = priv->netdev;
    struct ftmac100_txdes *txdes;

    if (priv->tx_pending == 0)
        return MAC_FALSE;

    txdes = ftmac100_current_clean_txdes(priv);

    if (ftmac100_txdes_owned_by_dma(txdes))
        return MAC_FALSE;

    if (unlikely(ftmac100_txdes_excessive_collision(txdes) ||
             ftmac100_txdes_late_collision(txdes))) {
        /*
         * packet transmitted to ethernet lost due to late collision
         * or excessive collision
         */
        netdev->stats.tx_aborted_errors++;
    } else {
        netdev->stats.tx_packets++;
        netdev->stats.tx_bytes += ftmac100_txdes_get_len(txdes);
    }

    ftmac100_txdes_reset(txdes);

    ftmac100_tx_clean_pointer_advance(priv);

    MAC_WaitSem(priv->tx_lock);
    priv->tx_pending--;
    //ithPrintf(" tx_e %d \n", priv->tx_pending);
    NETIF_TX_QUEUE_ON(netdev);
    MAC_ReleaseSem(priv->tx_lock);

    return MAC_TRUE;
}

static void ftmac100_tx_complete(struct ftmac100 *priv)
{
    while (ftmac100_tx_complete_packet(priv))
        ;
}

static int ftmac100_xmit(struct ftmac100 *priv, void* packet,
             MAC_UINT32 length)
{
    struct eth_device *netdev = priv->netdev;
    struct ftmac100_txdes *txdes;
    unsigned int len = (length < ETH_ZLEN) ? ETH_ZLEN : length;

    txdes = ftmac100_current_txdes(priv);
    ftmac100_tx_pointer_advance(priv);

    /* setup TX descriptor */
    ftmac100_txdes_set_len(txdes, len);
    ftmac100_txdes_set_dma_addr(txdes, packet);

    ftmac100_txdes_set_first_segment(txdes);
    ftmac100_txdes_set_last_segment(txdes);
    ftmac100_txdes_set_txint(txdes);
    ftmac100_txdes_set_buffer_size(txdes, len);

    MAC_WaitSem(priv->tx_lock);
    priv->tx_pending++;
    //ithPrintf(" tx_s %d \n", priv->tx_pending);
    if (priv->tx_pending >= (TX_QUEUE_ENTRIES-4))
        NETIF_TX_QUEUE_OFF(netdev);

    /* start transmit */
    ftmac100_txdes_set_dma_own(txdes);
    MAC_ReleaseSem(priv->tx_lock);

    ftmac100_txdma_start_polling(priv);

    return 0;
}
