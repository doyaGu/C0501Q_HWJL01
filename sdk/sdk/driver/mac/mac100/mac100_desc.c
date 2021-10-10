
/******************************************************************************
 * internal functions (receive descriptor)
 *****************************************************************************/
static MAC_INLINE MAC_BOOL ftmac100_rxdes_first_segment(struct ftmac100_rxdes *rxdes)
{
    return rxdes->rxdes0 & cpu_to_le32(FTMAC100_RXDES0_FRS);
}

static MAC_INLINE MAC_BOOL ftmac100_rxdes_last_segment(struct ftmac100_rxdes *rxdes)
{
    return rxdes->rxdes0 & cpu_to_le32(FTMAC100_RXDES0_LRS);
}

static MAC_INLINE MAC_BOOL ftmac100_rxdes_owned_by_dma(struct ftmac100_rxdes *rxdes)
{
    return rxdes->rxdes0 & cpu_to_le32(FTMAC100_RXDES0_RXDMA_OWN);
}

static MAC_INLINE void ftmac100_rxdes_set_dma_own(struct ftmac100_rxdes *rxdes)
{
#if defined(CFG_CPU_WB)
    /* avoid cpu flush dirty cache to memory */
	ithInvalidateDCacheRange((void*)rxdes->rxdes2, RX_BUF_SIZE);
#endif
    /* clear status bits */
    rxdes->rxdes0 = cpu_to_le32(FTMAC100_RXDES0_RXDMA_OWN);
}

static MAC_INLINE MAC_BOOL ftmac100_rxdes_rx_error(struct ftmac100_rxdes *rxdes)
{
    return rxdes->rxdes0 & cpu_to_le32(FTMAC100_RXDES0_RX_ERR);
}

static MAC_INLINE MAC_BOOL ftmac100_rxdes_crc_error(struct ftmac100_rxdes *rxdes)
{
    return rxdes->rxdes0 & cpu_to_le32(FTMAC100_RXDES0_CRC_ERR);
}

static MAC_INLINE MAC_BOOL ftmac100_rxdes_frame_too_long(struct ftmac100_rxdes *rxdes)
{
    return rxdes->rxdes0 & cpu_to_le32(FTMAC100_RXDES0_FTL);
}

static MAC_INLINE MAC_BOOL ftmac100_rxdes_runt(struct ftmac100_rxdes *rxdes)
{
    return rxdes->rxdes0 & cpu_to_le32(FTMAC100_RXDES0_RUNT);
}

static MAC_INLINE MAC_BOOL ftmac100_rxdes_odd_nibble(struct ftmac100_rxdes *rxdes)
{
    return rxdes->rxdes0 & cpu_to_le32(FTMAC100_RXDES0_RX_ODD_NB);
}

static MAC_INLINE unsigned int ftmac100_rxdes_frame_length(struct ftmac100_rxdes *rxdes)
{
#if defined(MAC_CRC_DIS)
    unsigned int len =  le32_to_cpu(rxdes->rxdes0) & FTMAC100_RXDES0_RFL;
    return (len-4);
#else
    return le32_to_cpu(rxdes->rxdes0) & FTMAC100_RXDES0_RFL;
#endif
}

static MAC_INLINE MAC_BOOL ftmac100_rxdes_multicast(struct ftmac100_rxdes *rxdes)
{
    return rxdes->rxdes0 & cpu_to_le32(FTMAC100_RXDES0_MULTICAST);
}

static MAC_INLINE void ftmac100_rxdes_set_buffer_size(struct ftmac100_rxdes *rxdes,
                       unsigned int size)
{
    rxdes->rxdes1 &= cpu_to_le32(FTMAC100_RXDES1_EDORR);
    rxdes->rxdes1 |= cpu_to_le32(FTMAC100_RXDES1_RXBUF_SIZE(size));
}

static MAC_INLINE void ftmac100_rxdes_set_end_of_ring(struct ftmac100_rxdes *rxdes)
{
    rxdes->rxdes1 |= cpu_to_le32(FTMAC100_RXDES1_EDORR);
}

static MAC_INLINE void ftmac100_rxdes_set_dma_addr(struct ftmac100_rxdes *rxdes,
                    MAC_UINT32 addr)
{
    rxdes->rxdes2 = cpu_to_le32(addr);
}

static MAC_INLINE MAC_UINT32 ftmac100_rxdes_get_dma_addr(struct ftmac100_rxdes *rxdes)
{
    return le32_to_cpu(rxdes->rxdes2);
}

/*
 * rxdes3 is not used by hardware. We use it to keep buffer address.
 * Since hardware does not touch it, we can skip cpu_to_le32()/le32_to_cpu().
 */
static MAC_INLINE void ftmac100_rxdes_set_page(struct ftmac100_rxdes *rxdes, MAC_UINT32 buf)
{
    rxdes->rxdes3 = (unsigned int)buf;
}

static MAC_INLINE MAC_UINT32 ftmac100_rxdes_get_page(struct ftmac100_rxdes *rxdes)
{
    return (MAC_UINT32)rxdes->rxdes3;
}


/******************************************************************************
 * internal functions (transmit descriptor)
 *****************************************************************************/
static MAC_INLINE void ftmac100_txdes_reset(struct ftmac100_txdes *txdes)
{
    /* clear all except end of ring bit */
    txdes->txdes0 = 0;
    txdes->txdes1 &= cpu_to_le32(FTMAC100_TXDES1_EDOTR);
    txdes->txdes2 = 0;
    txdes->txdes3 = 0;
}

static MAC_INLINE MAC_BOOL ftmac100_txdes_owned_by_dma(struct ftmac100_txdes *txdes)
{
    return txdes->txdes0 & cpu_to_le32(FTMAC100_TXDES0_TXDMA_OWN);
}

static MAC_INLINE void ftmac100_txdes_set_dma_own(struct ftmac100_txdes *txdes)
{
    #if defined(CFG_CPU_WB)
	ithFlushDCacheRange((void*)txdes->txdes2, txdes->txdes3);
    #endif

    /*
     * Make sure dma own bit will not be set before any other
     * descriptor fields.
     */
    txdes->txdes0 |= cpu_to_le32(FTMAC100_TXDES0_TXDMA_OWN);
	ithFlushMemBuffer();
}

static MAC_INLINE MAC_BOOL ftmac100_txdes_excessive_collision(struct ftmac100_txdes *txdes)
{
    return txdes->txdes0 & cpu_to_le32(FTMAC100_TXDES0_TXPKT_EXSCOL);
}

static MAC_INLINE MAC_BOOL ftmac100_txdes_late_collision(struct ftmac100_txdes *txdes)
{
    return txdes->txdes0 & cpu_to_le32(FTMAC100_TXDES0_TXPKT_LATECOL);
}

static MAC_INLINE void ftmac100_txdes_set_end_of_ring(struct ftmac100_txdes *txdes)
{
    txdes->txdes1 |= cpu_to_le32(FTMAC100_TXDES1_EDOTR);
}

static MAC_INLINE void ftmac100_txdes_set_first_segment(struct ftmac100_txdes *txdes)
{
    txdes->txdes1 |= cpu_to_le32(FTMAC100_TXDES1_FTS);
}

static MAC_INLINE void ftmac100_txdes_set_last_segment(struct ftmac100_txdes *txdes)
{
    txdes->txdes1 |= cpu_to_le32(FTMAC100_TXDES1_LTS);
}

static MAC_INLINE void ftmac100_txdes_set_txint(struct ftmac100_txdes *txdes)
{
    txdes->txdes1 |= cpu_to_le32(FTMAC100_TXDES1_TXIC);
}

static MAC_INLINE void ftmac100_txdes_set_buffer_size(struct ftmac100_txdes *txdes,
                       unsigned int len)
{
    txdes->txdes1 |= cpu_to_le32(FTMAC100_TXDES1_TXBUF_SIZE(len));
}

static MAC_INLINE void ftmac100_txdes_set_dma_addr(struct ftmac100_txdes *txdes,
                    void* addr)
{
    txdes->txdes2 = cpu_to_le32((MAC_UINT32)addr);
}

static MAC_INLINE MAC_UINT32 ftmac100_txdes_get_dma_addr(struct ftmac100_txdes *txdes)
{
    return le32_to_cpu(txdes->txdes2);
}

/*
 * txdes3 is not used by hardware. We use it to keep track of packet length.
 * Since hardware does not touch it, we can skip cpu_to_le32()/le32_to_cpu().
 */
static MAC_INLINE void ftmac100_txdes_set_len(struct ftmac100_txdes *txdes, MAC_UINT32 len)
{
    txdes->txdes3 = (unsigned int)len;
}

static MAC_INLINE MAC_UINT32 ftmac100_txdes_get_len(struct ftmac100_txdes *txdes)
{
    return (MAC_UINT32)txdes->txdes3;
}

