

static int ftgmac030_mdiobus_read(struct eth_device *netdev, int phy_addr, int regnum)
{
    struct ftgmac030_ctrl *ctrl = netdev_priv(netdev);
    unsigned int phycr;
    int i;

    phycr = FTGMAC030_PHYCR_ST(1) | FTGMAC030_PHYCR_OP(2) |
        FTGMAC030_PHYCR_PHYAD(phy_addr) |
        FTGMAC030_PHYCR_REGAD(regnum) |
        FTGMAC030_PHYCR_MIIRD | ctrl->mdc_cycthr;

    iow32(FTGMAC030_REG_PHYCR, phycr);

    for (i = 0; i < 100; i++) {
        phycr = ior32(FTGMAC030_REG_PHYCR);

        if ((phycr & FTGMAC030_PHYCR_MIIRD) == 0) {
            int data;

            data = ior32(FTGMAC030_REG_PHYDATA);
            return FTGMAC030_PHYDATA_MIIRDATA(data);
        }

        udelay(100);
    }

    e_err(link, "mdio read timed out\n");
    return -EIO;
}

static void ftgmac030_mdiobus_write(struct eth_device *netdev, int phy_addr,
    int regnum, int value)
{
    struct ftgmac030_ctrl *ctrl = netdev_priv(netdev);
    unsigned int phycr;
    int data;
    int i;

    phycr = FTGMAC030_PHYCR_ST(1) | FTGMAC030_PHYCR_OP(1) |
        FTGMAC030_PHYCR_PHYAD(phy_addr) |
        FTGMAC030_PHYCR_REGAD(regnum) |
        FTGMAC030_PHYCR_MIIWR | ctrl->mdc_cycthr;

    data = FTGMAC030_PHYDATA_MIIWDATA(value);

    iow32(FTGMAC030_REG_PHYDATA, data);
    iow32(FTGMAC030_REG_PHYCR, phycr);

    for (i = 0; i < 100; i++) {
        phycr = ior32(FTGMAC030_REG_PHYCR);

        if ((phycr & FTGMAC030_PHYCR_MIIWR) == 0)
            return;

        udelay(100);
    }

    e_err(link, "mdio write timed out\n");
    //return -EIO;
}

/* initialize struct mii_if_info */
static int ftgmac030_mii_init(struct ftgmac030_ctrl *ctrl)
{
    ctrl->mii.phy_id = ctrl->cfg->phyAddr;
    ctrl->mii.phy_id_mask = 0x1f;
    ctrl->mii.reg_num_mask = 0x1f;
    ctrl->mii.dev = &eth_dev;
    ctrl->mii.mdio_read = ftgmac030_mdiobus_read;
    ctrl->mii.mdio_write = ftgmac030_mdiobus_write;
}