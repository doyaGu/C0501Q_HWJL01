#include "ite/ite_mac.h"



static struct mii_ioctl_data phy_reg = { CFG_NET_ETHERNET_PHY_ADDR, 0, 0, 0 };

#define phy_set_page(page)      do { phy_reg.reg_num=20; phy_reg.val_write=page; iteMacIoctl(&phy_reg, IOCSMIIREG); } while(0)
#define phy_reg_write(reg,val)  do { phy_reg.reg_num=(reg); phy_reg.val_write=(val); iteMacIoctl(&phy_reg, IOCSMIIREG); } while(0)

static uint16_t phy_reg_read(uint16_t reg)
{
    phy_reg.reg_num = reg; 
    phy_reg.val_read = 0; 
    iteMacIoctl(&phy_reg, IOCGMIIREG);
    return phy_reg.val_read;
}

static int ip101g_reset(void)
{
    int bmcr;

    // Software Reset PHY
    bmcr = phy_reg_read(MII_BMCR);
    if (bmcr < 0)
        return bmcr;
    bmcr |= BMCR_RESET;
    phy_reg_write(MII_BMCR, bmcr);

    do {
        bmcr = phy_reg_read(MII_BMCR);
        if (bmcr < 0)
            return bmcr;
    } while (bmcr & BMCR_RESET);

    return 0;
}

static void ip101g_set_aps(int en)
{
    int aps;

    phy_set_page(16);
    aps = phy_reg_read(16);
    if (en)
        phy_reg_write(16, (aps | 0x0002));
    else
        phy_reg_write(16, (aps & ~0x0002));
}

static void ip101g_config_intr(void)
{
#if defined(CFG_NET_ETHERNET_LINK_INTR)
    phy_set_page(16);
    /* D[2]: selsec INTR32. The multiplex function is for 32pin package at pin21 RXEN/INTR_32. */
    /* D[7]: disable TXER/RXER function in RMII mode */
    phy_reg_write(29, (phy_reg_read(29) & ~0x0080) | 0x0004);
    /* interrupt pin used. */
    phy_reg_write(17, 0x8000);
#endif
}

void
PhyInit(int ethMode)
{
    int res;
    uint16_t tmp;
    uint32_t timeout;

    if (ethMode == ITE_ETH_MAC_LB)
        return;

    /* phy reset */
    if (ip101g_reset())
        printf("ip101g reset fail! \n");
    printf(" after reset: phy reg %d = 0x%04X \n", phy_reg.reg_num, phy_reg.val_read);

    // Alter driving current
#if 1
    phy_set_page(4);
    phy_reg_write(22, (phy_reg_read(22) & 0x1FFF) | 0x8000);
    phy_set_page(16);
    phy_reg_write(26, (phy_reg_read(26) & 0x8000) | 0x4924);
#endif

    // Enable Auto Power Saving mode
    ip101g_set_aps(1);

    // config interrupt
    ip101g_config_intr();

    if (ethMode == ITE_ETH_PCS_LB_10) {
        printf(" PHY PCS 10 loopback! \n\n");
        phy_reg_write(0, 0x4100);
        usleep(20 * 1000);
    } 
    else if(ethMode == ITE_ETH_PCS_LB_100) {
        printf(" PHY PCS 100 loopback! \n\n");
        phy_reg_write(0, 0x6100);
        usleep(20 * 1000);
    }
    else if (ethMode == ITE_ETH_MDI_LB_10) {
        printf(" PHY MDI 10 loopback! \n\n");
        phy_reg_write(0, 0x0100);
        usleep(100 * 1000);
    }
    else if (ethMode == ITE_ETH_MDI_LB_100) {
        printf(" PHY MDI 100 loopback! \n\n");
        phy_reg_write(0, 0x2100);
        usleep(100 * 1000);
    }
}

/* phy interrupt status definition */
#define INTR_SPEEDCHG		0x0004
#define INTR_DUPLEXCHG      0x0002
#define INTR_LINKCHG		0x0001

/* in interrupt context */
int ip101g_link_change(void)
{
    uint32_t intr;

    phy_set_page(16);
    intr = phy_reg_read(17);

#if 0
    if (intr & INTR_DUPLEXCHG)
        ithPrintf("phy: duplex change! \n");
    if (intr & INTR_SPEEDCHG)
        ithPrintf("phy: speed change! \n");
#endif
    if (intr & INTR_LINKCHG) {
        ithPrintf("phy: link change! \n");
        return 1;
    }

    return 0;
}

/**
* Check interrupt status for link change. 
* Call from mac driver's internal ISR for phy's interrupt.
*/
int(*itpPhyLinkChange)(void) = ip101g_link_change;
/**
* Replace mac driver's ISR for phy's interrupt. 
*/
ITHGpioIntrHandler itpPhylinkIsr = NULL;
/**
* Returns 0 if the device reports link status up/ok 
*/
int(*itpPhyReadMode)(int* speed, int* duplex) = NULL;
/**
* Get link status.
*/
uint32_t(*itpPhyLinkStatus)(void) = NULL;


