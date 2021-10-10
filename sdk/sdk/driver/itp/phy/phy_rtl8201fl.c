#include "ite/ite_mac.h"


#define CLK_FROM_PHY
#define PHY_INTR_EN

#if 0 // for reference
ITE_ETH_REAL = 1,
ITE_ETH_MAC_LB = 2,
ITE_ETH_PCS_LB_10 = 3,
ITE_ETH_PCS_LB_100 = 4,
ITE_ETH_MDI_LB_10 = 5,
ITE_ETH_MDI_LB_100 = 6
#endif



static struct ethtool_cmd ecmd = {0};
static struct mii_ioctl_data phy_reg = {0, 0, 0, 0};

#define phy_set_page(page)      do { phy_reg.reg_num=31; phy_reg.val_write=page; iteMacIoctl(&phy_reg, IOCSMIIREG); } while(0)
#define phy_reg_read(reg)       do { phy_reg.reg_num=(reg); phy_reg.val_read=0; iteMacIoctl(&phy_reg, IOCGMIIREG); } while(0)
#define phy_reg_write(reg,val)  do { phy_reg.reg_num=(reg); phy_reg.val_write=(val); iteMacIoctl(&phy_reg, IOCSMIIREG); } while(0)
#define phy_reset()             do { phy_reg.reg_num=0; phy_reg.val_write=0x9200; iteMacIoctl(&phy_reg, IOCSMIIREG); } while(0)


void
PhyInit(int ethMode)
{
    int res;
    uint16_t tmp;
    uint32_t timeout;

    if (ethMode == ITE_ETH_MAC_LB)
        return;

    {
        iteEthGetSetting(&ecmd);
        #if 0
        printf(" ecmd.supported = 0x%08X \n", ecmd.supported);
        printf(" ecmd.advertising = 0x%08X \n", ecmd.advertising);
        printf(" ecmd.speed = 0x%08X \n", ecmd.speed);
        printf(" ecmd.duplex = 0x%08X \n", ecmd.duplex);
        printf(" ecmd.port = 0x%08X \n", ecmd.port);
        printf(" ecmd.phy_address = 0x%08X \n", ecmd.phy_address);
        printf(" ecmd.transceiver = 0x%08X \n", ecmd.transceiver);
        printf(" ecmd.autoneg = 0x%08X \n", ecmd.autoneg);
        printf(" ecmd.lp_advertising = 0x%08X \n\n", ecmd.lp_advertising);
        #endif
        //phy_reg.phy_id = ecmd.phy_address;
    }
	phy_reg.phy_id = CFG_NET_ETHERNET_PHY_ADDR;
	
    /* phy reset */
    phy_reset();
    timeout = 2000; // ms
    do {
        phy_reg_read(0);
        usleep(1000);
    } while((phy_reg.val_read & 0x8000) && timeout--);
    printf(" after reset: phy reg %d = 0x%04X \n", phy_reg.reg_num, phy_reg.val_read);
	
    /** CRS can't toggle for RMII */
    phy_set_page(7);
    phy_reg_read(16);
    #if defined(CLK_FROM_PHY)
    phy_reg_write(16, (phy_reg.val_read|(0x1<<2)));
    #else
    phy_reg_write(16, (phy_reg.val_read|((0x1<<12)|(0x1<<2))));
    #endif
    phy_reg_read(16);
    //printf(" phy P7 reg %d = 0x%04X \n", phy_reg.reg_num, phy_reg.val_read);
    phy_set_page(0);

#if defined(PHY_INTR_EN)
    //phy_reg_read(30); /** clear interrupt */
    phy_set_page(7);
    phy_reg_read(19);
    //printf(" P7 reg %d = 0x%04X \n", phy_reg.reg_num, phy_reg.val_read);
	phy_reg.val_read &= ~0x30;  /** LED selection D[5:4]=00 */
    phy_reg_write(19, (phy_reg.val_read|(0x2000)));
    phy_reg_read(19);
    //printf(" P7 reg %d = 0x%04X \n", phy_reg.reg_num, phy_reg.val_read);
    phy_set_page(0);
#endif

    if (ethMode == ITE_ETH_PCS_LB_10) {
        printf(" PHY PCS 10 loopback! \n\n");
        phy_set_page(0);
        phy_reg_write(0, 0x4100);
        phy_set_page(7);
        tmp = 0x1FF8;
        #if defined(CLK_FROM_PHY)
        tmp &= ~(0x1 << 12);
        #endif
        phy_reg_write(16, tmp);
        phy_set_page(0);
        usleep(20 * 1000);
    } 
    else if(ethMode == ITE_ETH_PCS_LB_100) {
        printf(" PHY PCS 100 loopback! \n\n");
        phy_set_page(0);
        phy_reg_write(0, 0x6100);
        phy_set_page(7);
        tmp = 0x1FF8;
        #if defined(CLK_FROM_PHY)
        tmp &= ~(0x1 << 12);
        #endif
        phy_reg_write(16, tmp);
        phy_set_page(0);
        usleep(20 * 1000);
    }
    else if (ethMode == ITE_ETH_MDI_LB_10) {
        printf(" PHY MDI 10 loopback! \n\n");
        phy_set_page(0);
        phy_reg_write(0, 0x0100);
        usleep(100 * 1000);
    }
    else if (ethMode == ITE_ETH_MDI_LB_100) {
        printf(" PHY MDI 100 loopback! \n\n");
        phy_set_page(0);
        phy_reg_write(0, 0x2100);
        usleep(100 * 1000);
    }
}

/* phy interrupt status definition */
#define MII_INTRSTS         0x1e        /* Interrupt Indicators   */
#define INTR_LINKCHG		0x0800
#define INTR_DUPLEXCHG      0x2000
#define INTR_SPEEDCHG		0x4000
#define INTR_ANERR			0x8000
#define INTR_MSK			0xE800

/* in interrupt context */
int rtl8201_link_change(void)
{
    uint32_t intr;

    phy_set_page(0);
    phy_reg_read(0x1e);
    intr = phy_reg.val_read & INTR_MSK;

#if 0
    if (intr & INTR_DUPLEXCHG)
        ithPrintf("phy: duplex change! \n");
    if (intr & INTR_SPEEDCHG)
        ithPrintf("phy: speed change! \n");
#endif
    if (intr & INTR_ANERR)
        ithPrintf("phy: ANERR! \n");
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
int(*itpPhyLinkChange)(void) = rtl8201_link_change;
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



static uint8_t _macAddr[] = {0x66,0x55,0x44,0x33,0x22,0x11};

static void
PhyWolEnter(void)
{
    uint16_t tmp;

    /* 1: initial setting */
    /* 1.1 set MAC address */
    phy_set_page(18);
#if 0
    tmp = macAddr[1]<<8 | macAddr[0];
    phy_reg_write(16, tmp);
    tmp = macAddr[3]<<8 | macAddr[2];
    phy_reg_write(17, tmp);
    tmp = macAddr[5]<<8 | macAddr[4];
    phy_reg_write(18, tmp);
#else
    tmp = _macAddr[1]<<8 | _macAddr[0];
    phy_reg_write(16, tmp);
    tmp = _macAddr[3]<<8 | _macAddr[2];
    phy_reg_write(17, tmp);
    tmp = _macAddr[5]<<8 | _macAddr[4];
    phy_reg_write(18, tmp);
#endif
    /* 1.2 set max packet length */
    phy_set_page(17);
    phy_reg_write(17, 0x9FFF);  // D[15]: WOL reset

    /* 2: enter WOL */
    /* 2.1 WOL event select and enable */
    phy_set_page(17);
    phy_reg_write(16, 0x1000);  // enable all WOL events (0x3FFF)
    /* 2.2 wake up frame select and enable */
    // mask of wake-up frame #?
    // 16 bits CRC of wake-up frame #?
    /* 2.3 speed selection */
    phy_set_page(0);
    phy_reg_read(0);
    phy_reg_write(0, (phy_reg.val_read & ~0x2000)); // D[13]:0 => 10Mbps
    /* 2.4 TX isolate */
    phy_set_page(7);
    phy_reg_write(20, 0x90D3);  // D[15]=1
    /* 2.5 RX isolate */
    phy_set_page(17);
    phy_reg_write(19, 0x8002);  // D[15]=1

    printf("\n\n enter WOL ==> \n");
}

static void
PhyWolExit(void)
{
    /* 1: restore to the original link speed */
    phy_set_page(0);
    phy_reg_read(0);
    phy_reg_write(0, (phy_reg.val_read|0x2000)); // D[13]:1 => 100Mbps
    /* 2: TX/RX isolate disable */
    phy_set_page(7);
    phy_reg_write(20, 0x10D3);  // D[15]=0
    phy_set_page(17);
    phy_reg_write(19, 0x0002);  // D[15]=0
    /* 3: disable all WOL events */
    phy_reg_write(16, 0x0);
    /* 4: WOL reset */
    phy_reg_write(17, 0x8000);  // D[15]: WOL reset
    ithPrintf(" <== leave WOL\n\n");
}
