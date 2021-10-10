
/******************************************************************************
 * internal functions (buffer)
 *****************************************************************************/
static int ftmac100_alloc_rx_page(struct ftmac100 *priv,
                  struct ftmac100_rxdes *rxdes)
{
    MAC_UINT32 buf;
    buf = (MAC_UINT32)MAC_VmemAlloc(4, RX_BUF_SIZE); /* api is 64-bytes alignment */
    if(!buf)
        return ERROR_MAC_ALLOC_RX_BUF_FAIL;

    ftmac100_rxdes_set_page(rxdes, buf);
    ftmac100_rxdes_set_dma_addr(rxdes, buf);
    ftmac100_rxdes_set_buffer_size(rxdes, RX_BUF_SIZE);
    ftmac100_rxdes_set_dma_own(rxdes);
    return 0;
}

static void ftmac100_free_buffers(struct ftmac100 *priv)
{
    int i;

    for (i = 0; i < RX_QUEUE_ENTRIES; i++) {
        struct ftmac100_rxdes *rxdes = &priv->descs->rxdes[i];
        MAC_UINT32 buf = ftmac100_rxdes_get_page(rxdes);
        if (!buf)
            continue;

        MAC_VmemFree(buf);
        ftmac100_rxdes_set_page(rxdes, 0);
    }

    /* no need to free tx buffer */

    itpWTFree((uint32_t)priv->descs);
    priv->descs = MAC_NULL;
}

static int ftmac100_alloc_buffers(struct ftmac100 *priv)
{
    int i, rc=0;

    priv->descs = (struct ftmac100_descs*)itpWTAlloc(sizeof(struct ftmac100_descs));
    if (!priv->descs)
    {
        rc = ERROR_MAC_ALLOC_DESC_FAIL;
        goto end;
    }
    if((uint32_t)priv->descs & 0xF)
	    printf(" mac desc not align... 0x%08X \t\n", priv->descs);

    memset((void*)priv->descs, 0, sizeof(struct ftmac100_descs));

    /* initialize RX ring */
    ftmac100_rxdes_set_end_of_ring(&priv->descs->rxdes[RX_QUEUE_ENTRIES - 1]);

    for (i = 0; i < RX_QUEUE_ENTRIES; i++) {
        struct ftmac100_rxdes *rxdes = &priv->descs->rxdes[i];

        if (rc=ftmac100_alloc_rx_page(priv, rxdes))
            goto err;
    }

    /* initialize TX ring */
    ftmac100_txdes_set_end_of_ring(&priv->descs->txdes[TX_QUEUE_ENTRIES - 1]);
    return rc;

err:
    ftmac100_free_buffers(priv);
end:
    if(rc)
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, rc LOG_END
    return rc;
}
