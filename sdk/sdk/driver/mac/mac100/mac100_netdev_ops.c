
/******************************************************************************
 * struct net_device_ops functions
 *****************************************************************************/
static int ftmac100_open(struct eth_device *netdev, int alloc_buf)
{
    struct ftmac100 *priv = netdev->priv;
    int err;

    if(alloc_buf)
    {
        err = ftmac100_alloc_buffers(priv);
        if (err) 
            goto err_end;
    }


    priv->rx_pointer = 0;
    priv->tx_clean_pointer = 0;
    priv->tx_pointer = 0;
    priv->tx_pending = 0;

    err = ftmac100_start_hw(priv);
    if (err)
        goto err_end;

    NETIF_START(netdev);
    ftmac100_enable_all_int(priv);
    intrEnable(netdev);

    return 0;

err_end:
    if(err)
        LOG_ERROR "[%s] rc = 0x%08X \n", __FUNCTION__, err LOG_END
    return err;
}

static int ftmac100_stop(struct eth_device *netdev, int free_buf)
{
    struct ftmac100 *priv = netdev->priv;

    ftmac100_disable_all_int(priv);
    intrDisable(priv);
    NETIF_STOP(netdev);
    ftmac100_stop_hw(priv);
    if(free_buf)
        ftmac100_free_buffers(priv);

    return 0;
}

static int ftmac100_hard_start_xmit(void* packet, MAC_UINT32 len, struct eth_device *netdev)
{
    struct ftmac100 *priv = netdev->priv;

    if (len > MAX_PKT_SIZE) {
        LOG_ERROR "tx packet too big\n" LOG_END

        netdev->stats.tx_dropped++;
        return 0;
    }

    return ftmac100_xmit(priv, packet, len);
}

/* optional */
static int ftmac100_do_ioctl(struct eth_device *netdev, struct mii_ioctl_data* data, int cmd)
{
    struct ftmac100 *priv = netdev->priv;

    return generic_mii_ioctl(&priv->mii, data, cmd, MAC_NULL);
}

/**********************************************/
static int crc32(uint8_t* s, int length)
{
    int perByte;
    int perBit;
    /* crc polynomial for Ethernet */
    const unsigned long poly = 0xedb88320;
    /* crc value - preinitialized to all 1's */
    unsigned long crc_value = 0xffffffff;
    unsigned long crc_bit_reverse = 0x0, i;

    for(perByte=0; perByte<length; perByte++)
    {
        uint8_t c;

        c = *(s++);
        for(perBit=0; perBit<8; perBit++)
        {
            crc_value = (crc_value >> 1)^(((crc_value^c)&0x01)?poly:0);
            c >>= 1;
        }
    }

    for(i=0; i<32; i++)
        crc_bit_reverse |= ((crc_value>>i) & 0x1) << (31-i);

    //return crc_value;
    return crc_bit_reverse;
}

static void ftmac100_setmulticast(struct ftmac100 *priv, MAC_UINT8* mc_list, int count)
{
    MAC_UINT8* cur_addr;
    int i, crc_val;
    MAC_UINT32 ht0=0, ht1=0;

    for(i=0; i<count; i++)
    {
        cur_addr = mc_list + (i*8);
        if(*cur_addr & 1)
        {
            crc_val = crc32(cur_addr, 6);
            crc_val = (crc_val >> 26) & 0x3F;  // MSB 6 bits

            if(crc_val >= 32)
                ht1 |= (0x1 << (crc_val-32));
            else
                ht0 |= (0x1 << crc_val);
        }
    }

    priv->mc_ht0 = ht0;
    priv->mc_ht1 = ht1;
}

static int ftmac100_set_rx_mode(struct eth_device *netdev, int flag, MAC_UINT8* mc_list, int count)
{
    struct ftmac100 *priv = netdev->priv;

    priv->rx_flag = flag; /* see <if.h> IFF_PROMISC, IFF_ALLMULTI, IFF_MULTICAST */
    priv->mc_count = count;

    if((flag & IFF_MULTICAST) && (count > 0) && mc_list)
        ftmac100_setmulticast(priv, mc_list, count);
    else
    {
        priv->mc_ht0 = 0x0;
        priv->mc_ht1 = 0x0;
    }

    if(NETIF_RUNNING(dev))
        mac_set_ctrl_reg(priv);

    return 0;
}

