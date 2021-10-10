#include "ite/ite_mac.h"



static struct mii_ioctl_data phy_reg = { CFG_NET_ETHERNET_PHY_ADDR, 0, 0, 0 };

#define phy_set_page(page)      do { phy_reg.reg_num=31; phy_reg.val_write=page; iteMacIoctl(&phy_reg, IOCSMIIREG); } while(0)
#define phy_reg_write(reg,val)  do { phy_reg.reg_num=(reg); phy_reg.val_write=(val); iteMacIoctl(&phy_reg, IOCSMIIREG); } while(0)

static uint16_t phy_reg_read(uint16_t reg)
{
    phy_reg.reg_num = reg; 
    phy_reg.val_read = 0; 
    iteMacIoctl(&phy_reg, IOCGMIIREG);
    return phy_reg.val_read;
}

static int rtl8211f_reset(void)
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

/* phy interrupt definition */
#define INTR_JABBER			 	0x0400
#define INTR_ALDPS_STATE_CHANGE	0x0200
#define INTR_PME			 	0x0080
#define INTR_PHY_REG_ACCESSIBLE	0x0020
#define INTR_LINK_CHG    	    0x0010
#define INTR_AN_COMPLETE	    0x0008
#define INTR_AN_ERROR		    0x0001

#define INTR_ENABLE_ALL	(INTR_JABBER |\
	INTR_ALDPS_STATE_CHANGE |\
	INTR_PME |\
	INTR_PHY_REG_ACCESSIBLE |\
	INTR_LINK_CHG |\
	INTR_AN_COMPLETE |\
	INTR_AN_ERROR)
	
static void rtl8211f_config_intr(void)
{
    phy_set_page(0xA42);
	
#if defined(CFG_NET_ETHERNET_LINK_INTR)
    phy_reg_write(0x12, INTR_ENABLE_ALL);
#endif

	phy_set_page(0);
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
    if (rtl8211f_reset())
        printf("rtl8211e reset fail! \n");
    printf(" after reset: phy reg %d = 0x%04X \n", phy_reg.reg_num, phy_reg.val_read);

    /* config interrupt */
    rtl8211f_config_intr();

    if (ethMode == ITE_ETH_PCS_LB_10) {
        printf(" PHY PCS 10 loopback! \n\n");
        //phy_reg_write(0, 0x4100);
		phy_reg_write(0, 0x4000);
        usleep(20 * 1000);
    } 
    else if(ethMode == ITE_ETH_PCS_LB_100) {
        printf(" PHY PCS 100 loopback! \n\n");
        //phy_reg_write(0, 0x6100);
		phy_reg_write(0, 0x6000);
		phy_reg_read(0);
		printf(" phy reg %d = 0x%04X \n", phy_reg.reg_num, phy_reg.val_read);
        usleep(20 * 1000);
    }
    else if(ethMode == ITE_ETH_PCS_LB_1000) {
        printf(" PHY PCS 1000 loopback! \n\n");
        phy_reg_write(0, 0x4140);
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
    else if (ethMode == ITE_ETH_MDI_LB_1000) {
        printf(" PHY MDI 1000 loopback! \n\n");
        phy_reg_write(0, 0x0140);
        usleep(100 * 1000);
    }
}

#define INTR_JABBER			 	0x0400
#define INTR_ALDPS_STATE_CHANGE	0x0200
#define INTR_PME			 	0x0080
#define INTR_PHY_REG_ACCESSIBLE	0x0020
#define INTR_LINK_CHG    	    0x0010
#define INTR_AN_COMPLETE	    0x0008
#define INTR_AN_ERROR		    0x0001

/* in interrupt context */
int rtl8211f_link_change(void)
{
    uint16_t intr;

	phy_set_page(0xA43);

    intr = phy_reg_read(0x1D);
	
	phy_set_page(0);

#if 1
    if (intr & INTR_JABBER)
        ithPrintf("phy: jabber detected! \n");
    if (intr & INTR_ALDPS_STATE_CHANGE)
        ithPrintf("phy: ALDPS state changed! \n");
    if (intr & INTR_PME)
        ithPrintf("phy: WOL event occurred! \n");
    if (intr & INTR_PHY_REG_ACCESSIBLE)
        ithPrintf("phy: Can access PHY register through MDC/MDIO! \n");
    if (intr & INTR_AN_COMPLETE)
        ithPrintf("phy: Auto-Negotiation completed! \n");
    if (intr & INTR_AN_ERROR)
        ithPrintf("phy: auto-negotiation error! \n");
#endif
    if (intr & INTR_LINK_CHG) {
        ithPrintf("phy: link change! \n");
        return 1;
    }

    return 0;
}

#define PHYSR_FULL_DUPLEX               0x0008
#define PHYSR_LINK_OK                   0x0004

/* return 0 means link up */
int rtl8211f_read_mode(int* speed, int* duplex)
{
    uint16_t status = phy_reg_read(0x1A);
    uint16_t _speed = (status & 0x0030) >> 4;

    if (!(status & PHYSR_LINK_OK)) {
        ithPrintf("phy status reg0x1A = 0x%04X - link not ok! \n", status);
        return -1;
    }

    if (status & PHYSR_FULL_DUPLEX)
        (*duplex) = DUPLEX_FULL;
    else
        (*duplex) = DUPLEX_HALF;

    switch (_speed) {
    default:
    case 0:
        (*speed) = SPEED_10;
        break;
    case 1:
        (*speed) = SPEED_100;
        break;
    case 2:
        (*speed) = SPEED_1000;
        break;
    }

    return 0; // 0 means link up
}


/**
* Check interrupt status for link change. 
* Call from mac driver's internal ISR for phy's interrupt.
*/
int(*itpPhyLinkChange)(void) = rtl8211f_link_change;
/**
* Replace mac driver's ISR for phy's interrupt. 
*/
ITHGpioIntrHandler itpPhylinkIsr = NULL;
/**
* Returns 0 if the device reports link status up/ok 
*/
int(*itpPhyReadMode)(int* speed, int* duplex) = rtl8211f_read_mode;
/**
* Get link status.
*/
uint32_t(*itpPhyLinkStatus)(void) = NULL;


