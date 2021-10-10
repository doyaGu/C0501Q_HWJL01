
#define MDIO_TIMEOUT    3000   // ms

/******************************************************************************
 * struct mii_if_info functions
 *****************************************************************************/
static int ftmac100_mdio_read(struct eth_device *netdev, int phy_id, int reg)
{
    struct ftmac100 *priv = netdev->priv;
    MAC_UINT32 phycr;
    int i;

    phycr = FTMAC100_PHYCR_PHYAD(phy_id) |
        FTMAC100_PHYCR_REGAD(reg) |
        FTMAC100_PHYCR_MIIRD;

    MAC_WriteReg(phycr, priv->base + FTMAC100_OFFSET_PHYCR);

    for (i = 0; i < MDIO_TIMEOUT; i++) {
         MAC_ReadReg(priv->base + FTMAC100_OFFSET_PHYCR, &phycr);

        if ((phycr & FTMAC100_PHYCR_MIIRD) == 0)
            return phycr & FTMAC100_PHYCR_MIIRDATA;

        //udelay(100);
    }

    ithPrintf("mdio read timed out! reg=0x%08X, phy_id %d \n", reg, phy_id);

    return 0;
}

static void ftmac100_mdio_write(struct eth_device *netdev, int phy_id, int reg,
                int data)
{
    struct ftmac100 *priv = netdev->priv;
    MAC_UINT32 phycr;
    int i;

    phycr = FTMAC100_PHYCR_PHYAD(phy_id) |
        FTMAC100_PHYCR_REGAD(reg) |
        FTMAC100_PHYCR_MIIWR;

    data = FTMAC100_PHYWDATA_MIIWDATA(data);

    MAC_WriteReg(data, priv->base + FTMAC100_OFFSET_PHYWDATA);
    MAC_WriteReg(phycr, priv->base + FTMAC100_OFFSET_PHYCR);

    for (i = 0; i < MDIO_TIMEOUT; i++) {
         MAC_ReadReg(priv->base + FTMAC100_OFFSET_PHYCR, &phycr);

        if ((phycr & FTMAC100_PHYCR_MIIWR) == 0)
            return;

        //udelay(100);
    }

    ithPrintf("mdio write timed out! 0x%08X=0x%08X, phy_id %d \n", reg, data, phy_id);
}



