#include "ite/ite_mac.h"
#include "rtk_api.h"
#include "rtk_api_ext.h"
#include "rtl8309n_asictypes.h"


static rtl8304mb_port[4] = { RTL8304MB_PORT0, RTL8304MB_PORT1, RTL8304MB_PORT2, RTL8304MB_PORT3 };
static uint32_t rtl8304mb_link_status = 0;  /* D[0]:link, D[28]:port 0, D[29]:port 1, D[30]:port 2 */



static uint32_t rtl8304mb_link(void)
{
    uint32_t i, link = 0;
    rtk_api_ret_t ret;
    rtk_port_linkStatus_t linkStatus;
    rtk_port_speed_t speed;
    rtk_port_duplex_t duplex;

    for (i = 0; i < 3; i++)
    {
        linkStatus = speed = duplex = 0;
        ret = rtk_port_phyStatus_get(rtl8304mb_port[i], &linkStatus, &speed, &duplex);
        if (ret)
            ithPrintf("[ERR] rtk_port_phyStatus_get(%d) ret=0x%X \n", rtl8304mb_port[i], ret);
        if (linkStatus) {
            link |= ((1 << (28 + i)) | 0x1);
			ithPrintf("rtl8304mb: port %d(real port:%d), speed = %s, duplex = %s \n", i,rtl8304mb_port[i], speed==PORT_SPEED_100M?"100M":"10M", duplex==PORT_FULL_DUPLEX?"FULL":"HALF");
    	}
    }
    if (link) ithPrintf("rtl8304mb_link: 0x%08X\n", link);

    return link;
}

void PhyInit(int ethMode)
{
    rtk_port_phy_ability_t ability;
    rtk_api_ret_t ret;

    if (ret = rtk_switch_init())
    {
        printf("[ERR] rtk_switch_init() ret=0x%X \n", ret);
        goto end;
    }
    if (ret = rtk_qos_init(1))
    {
        printf("[ERR] rtk_qos_init() ret=0x%X \n", ret);
        goto end;
    }
    if (ret = rtk_cpu_enable_set(ENABLED))
    {
        printf("[ERR] rtk_cpu_enable_set() ret=0x%X \n", ret);
        goto end;
    }
    if (ret = rtk_cpu_tagPort_set(RTL8304MB_PORT3, DISABLED))
    {
        printf("[ERR] rtk_cpu_tagPort_set() ret=0x%X \n", ret);
        goto end;
    }

#if FORCE_10M_FULL
	{
	    int i;
		for (i=0; i<3; i++) {
			memset((void*)&ability, 0x0, sizeof(ability));
			ability.Full_10 = 1;
			ability.FC = 1;
			rtk_port_phyAutoNegoAbility_set(rtl8304mb_port[i], &ability);
		}
	}
#endif
	
#if 0
    if (ret = rtk_vlan_init())
    {
        printf("[ERR] rtk_vlan_init() ret=0x%X \n", ret);
        goto end;
    }
    if (ret = rtk_filter_igrAcl_init())
    {
        printf("[ERR] rtk_filter_igrAcl_init() ret=0x%X \n", ret);
        goto end;
    }
#endif

#if 0
    ret = rtk_port_phyAutoNegoAbility_get(RTL8304MB_PORT1, &ability);
    if (ret)
    {
        printf("[ERR] rtk_port_phyAutoNegoAbility_get() ret=0x%X \n", ret);
        goto end;
    }
    printf("rtk_port_phyAutoNegoAbility_get(): \n");
    printf(" ability.AutoNegotiation = %d \n", ability.AutoNegotiation);
    printf(" ability.Half_10 = %d \n", ability.Half_10);
    printf(" ability.Full_10 = %d \n", ability.Full_10);
    printf(" ability.Half_100 = %d \n", ability.Half_100);
    printf(" ability.Full_100 = %d \n", ability.Full_100);
    printf(" ability.FC = %d \n", ability.FC);
    printf(" ability.AsyFC = %d \n", ability.AsyFC);

    rtk_port_linkStatus_t linkStatus;
    rtk_port_speed_t speed;
    rtk_port_duplex_t duplex;

    linkStatus = speed = duplex = 0;
    ret = rtk_port_phyStatus_get(RTL8304MB_PORT0, &linkStatus, &speed, &duplex);
    if (ret)
    {
        printf("[ERR] rtk_port_phyStatus_get(%d) ret=0x%X \n", RTL8304MB_PORT0, ret);
        //goto end;
    }
    printf(" port %d: linkStatus=%d, speed=%d, duplex=%d \n", RTL8304MB_PORT0, linkStatus, speed, duplex);

    linkStatus = speed = duplex = 0;
    ret = rtk_port_phyStatus_get(RTL8304MB_PORT1, &linkStatus, &speed, &duplex);
    if (ret)
    {
        printf("[ERR] rtk_port_phyStatus_get(%d) ret=0x%X \n", RTL8304MB_PORT1, ret);
        //goto end;
    }
    printf(" port %d: linkStatus=%d, speed=%d, duplex=%d \n", RTL8304MB_PORT1, linkStatus, speed, duplex);

    linkStatus = speed = duplex = 0;
    ret = rtk_port_phyStatus_get(RTL8304MB_PORT2, &linkStatus, &speed, &duplex);
    if (ret)
    {
        printf("[ERR] rtk_port_phyStatus_get(%d) ret=0x%X \n", RTL8304MB_PORT2, ret);
        //goto end;
    }
    printf(" port %d: linkStatus=%d, speed=%d, duplex=%d \n", RTL8304MB_PORT2, linkStatus, speed, duplex);

    linkStatus = speed = duplex = 0;
    ret = rtk_port_phyStatus_get(RTL8304MB_PORT3, &linkStatus, &speed, &duplex);
    if (ret)
    {
        printf("[ERR] rtk_port_phyStatus_get(%d) ret=0x%X \n", RTL8304MB_PORT3, ret);
        //goto end;
    }
    printf(" port %d: linkStatus=%d, speed=%d, duplex=%d \n", RTL8304MB_PORT3, linkStatus, speed, duplex);
#endif

#if 0
    rtk_port_t port = 0;
    rtk_enable_t enTag = 0;
    if (ret = rtk_cpu_tagPort_get(&port, &enTag));
    {
        printf("[ERR] rtk_cpu_tagPort_get() ret=0x%X \n", ret);
        //goto end;
    }
    printf("rtk_cpu_tagPort_get() port:%d, enTag=%d \n", port, enTag);
#endif

#if 0 /* fail */
    {
        rtl8309n_mode_ext_t pMode = RTL8309N_MODE_EXT_RMII;
        rtl8309n_port_mac_ability_t pPortAbility = { 0 };
        
        pPortAbility.speed = RTL8309N_PORT_SPEED_100M;
        pPortAbility.duplex = RTL8309N_PORT_FULL_DUPLEX;
        pPortAbility.link = RTL8309N_PORT_LINK_UP;
        pPortAbility.nway = RTL8309N_DISABLED;
        pPortAbility.txpause = RTL8309N_DISABLED;
        pPortAbility.rxpause = RTL8309N_DISABLED;
        ret = rtl8309n_port_macForceLinkExt0_set(pMode, &pPortAbility); /* will disable reference clock */
        if (ret)
        {
            printf("[ERR] rtl8309n_port_macForceLinkExt0_set() ret=0x%X \n", ret);
            //goto end;
        }
        printf("rtl8309n_port_macForceLinkExt0_set()\n");
        usleep(1000 * 1000);
        
        ret = rtl8309n_port_macForceLinkExt0_get(&pMode, &pPortAbility);
        if (ret)
        {
            printf("[ERR] rtl8309n_port_macForceLinkExt0_get() ret=0x%X \n", ret);
            //goto end;
        }
        printf("rtl8309n_port_macForceLinkExt0_get(): \n");
        printf("mode = %d \n", pMode);
        printf("speed = %d \n", pPortAbility.speed);
        printf("duplex = %d \n", pPortAbility.duplex);
        printf("link = %d \n", pPortAbility.link);
        printf("nway = %d \n", pPortAbility.nway);
        printf("txpause = %d \n", pPortAbility.txpause);
        printf("rxpause = %d \n", pPortAbility.rxpause);
        printf("media = %d \n", pPortAbility.media);

    }
#endif

#if 0
    {
        rtl8309n_mode_ext_t pMode = 0;
        rtl8309n_port_mac_ability_t pPortAbility = { 0 };

        ret = rtl8309n_port_macAbilityExt0_get(&pMode, &pPortAbility);
        if (ret)
        {
            printf("[ERR] rtl8309n_port_macAbilityExt0_get() ret=0x%X \n", ret);
            //goto end;
        }
        printf("rtl8309n_port_macAbilityExt0_get(): \n");
        printf("mode = %d \n", pMode);
        printf("speed = %d \n", pPortAbility.speed);
        printf("duplex = %d \n", pPortAbility.duplex);
        printf("link = %d \n", pPortAbility.link);
        printf("nway = %d \n", pPortAbility.nway);
        printf("txpause = %d \n", pPortAbility.txpause);
        printf("rxpause = %d \n", pPortAbility.rxpause);
        printf("media = %d \n", pPortAbility.media);
    }
#endif

    rtl8304mb_link_status = rtl8304mb_link();

    /* enable link interrupt */
    ret = rtl8309n_intr_enable_set(RTL8309N_ENABLED, 0x119);
    if (ret)
    {
        printf("[ERR] rtl8309n_intr_enable_set() ret=0x%X \n", ret);
        //goto end;
    }

end:
    if (ret)
    {
        printf(" stop! \n");
        while (1);
    }

    return;
}

int rtl8304mb_link_port3(void)
{
    rtk_api_ret_t ret;
    rtl8309n_mode_ext_t pMode = 0;
    rtl8309n_port_mac_ability_t pPortAbility = { 0 };

    ret = rtl8309n_port_macAbilityExt0_get(&pMode, &pPortAbility);
    if (ret)
    {
        printf("[ERR] rtl8309n_port_macAbilityExt0_get() ret=0x%X \n", ret);
        //goto end;
    }
    printf("rtl8309n_port_macAbilityExt0_get(): \n");
    printf("mode = %d \n", pMode);
    printf("speed = %d \n", pPortAbility.speed);
    printf("duplex = %d \n", pPortAbility.duplex);
    printf("link = %d \n", pPortAbility.link);
    printf("nway = %d \n", pPortAbility.nway);
    printf("txpause = %d \n", pPortAbility.txpause);
    printf("rxpause = %d \n", pPortAbility.rxpause);
    printf("media = %d \n", pPortAbility.media);

    return (int)pPortAbility.link;
}

int rtl8304mb_read_mode(int* speed, int* duplex)
{
    rtk_api_ret_t ret;
    rtl8309n_mode_ext_t pMode = 0;
    rtl8309n_port_mac_ability_t pPortAbility = { 0 };

    ret = rtl8309n_port_macAbilityExt0_get(&pMode, &pPortAbility);
    if (ret)
    {
        printf("[ERR] rtl8309n_port_macAbilityExt0_get() ret=0x%X \n", ret);
        //goto end;
    }

    if (pPortAbility.speed == RTL8309N_PORT_SPEED_10M)
        (*speed) = SPEED_10;
    else if (pPortAbility.speed == RTL8309N_PORT_SPEED_100M)
        (*speed) = SPEED_100;
    else
        printf("MAC8 speed: %d \n", pPortAbility.speed);

    if (pPortAbility.duplex == RTL8309N_PORT_FULL_DUPLEX)
        (*duplex) = DUPLEX_FULL;
    else if (pPortAbility.duplex == RTL8309N_PORT_HALF_DUPLEX)
        (*duplex) = DUPLEX_HALF;
    else
        printf("MAC8 duplex: %d \n", pPortAbility.duplex);

    return (pPortAbility.link ? 0 : -1); // 0 means link up
}

void rtl8304mb_linkIntrHandler(unsigned int pin, void *arg)
{
    uint32 status = 0;

    if (pin != CFG_GPIO_ETHERNET_LINK)
    {
        ithPrintf("rtl8304mb link gpio error! %d \n", pin);
        return;
    }

    rtl8309n_intr_status_get(&status);
    if (status)
    {
        //ithPrintf("rtl8304mb_intr_status: 0x%X \n", status);
        rtl8304mb_link_status = rtl8304mb_link();
        rtl8309n_intr_status_set(status);
    }

    return;
}

uint32_t rtl8304mb_get_link_status(void)
{
    return rtl8304mb_link_status;
}

int rtl8304mb_mib_dump(int port)
{
    rtk_api_ret_t ret;
    rtk_mib_cntValue_t cntValue[2];
    printf("rtl8304mb_mib_dump() - enter \n");

    rtl8309n_mib_enable_set(RTL8309N_ENABLED);

    printf("port %d(%d): \n", port, rtl8304mb_port[port]);
    if (ret = rtk_mib_get(rtl8304mb_port[port], MIB_TXBYTECNT, cntValue)) goto end;
    printf("MIB_TXBYTECNT %X %X \n", cntValue[1], cntValue[0]);
    if (ret = rtk_mib_get(rtl8304mb_port[port], MIB_RXBYTECNT, cntValue)) goto end;
    printf("MIB_RXBYTECNT %X %X \n", cntValue[1], cntValue[0]);
    if (ret = rtk_mib_get(rtl8304mb_port[port], MIB_TXPKTCNT, cntValue)) goto end;
    printf("MIB_TXPKTCNT %X %X \n", cntValue[1], cntValue[0]);
    if (ret = rtk_mib_get(rtl8304mb_port[port], MIB_RXPKTCNT, cntValue)) goto end;
    printf("MIB_RXPKTCNT %X %X \n", cntValue[1], cntValue[0]);
    if (ret = rtk_mib_get(rtl8304mb_port[port], MIB_RXDPCNT, cntValue)) goto end;
    printf("MIB_RXDPCNT %X %X \n", cntValue[1], cntValue[0]);
    if (ret = rtk_mib_get(rtl8304mb_port[port], MIB_RXCRCCNT, cntValue)) goto end;
    printf("MIB_RXCRCCNT %X %X \n", cntValue[1], cntValue[0]);
    if (ret = rtk_mib_get(rtl8304mb_port[port], MIB_RXFRAGCNT, cntValue)) goto end;
    printf("MIB_RXFRAGCNT %X %X \n", cntValue[1], cntValue[0]);
    if (ret = rtk_mib_get(rtl8304mb_port[port], MIB_TXBRDCNT, cntValue)) goto end;
    printf("MIB_TXBRDCNT %X %X \n", cntValue[1], cntValue[0]);
    if (ret = rtk_mib_get(rtl8304mb_port[port], MIB_RXBRDCNT, cntValue)) goto end;
    printf("MIB_RXBRDCNT %X %X \n", cntValue[1], cntValue[0]);
    if (ret = rtk_mib_get(rtl8304mb_port[port], MIB_TXMULTCNT, cntValue)) goto end;
    printf("MIB_TXMULTCNT %X %X \n", cntValue[1], cntValue[0]);
    if (ret = rtk_mib_get(rtl8304mb_port[port], MIB_RXMULTCNT, cntValue)) goto end;
    printf("MIB_RXMULTCNT %X %X \n", cntValue[1], cntValue[0]);
    if (ret = rtk_mib_get(rtl8304mb_port[port], MIB_TXUNICNT, cntValue)) goto end;
    printf("MIB_TXUNICNT %X %X \n", cntValue[1], cntValue[0]);
    if (ret = rtk_mib_get(rtl8304mb_port[port], MIB_RXUNICNT, cntValue)) goto end;
    printf("MIB_RXUNICNT %X %X \n", cntValue[1], cntValue[0]);
    if (ret = rtk_mib_get(rtl8304mb_port[port], MIB_RXSYMBLCNT, cntValue)) goto end;
    printf("MIB_RXSYMBLCNT %X %X \n", cntValue[1], cntValue[0]);
    
end:
    if (ret)
        printf("ret = %d \n", ret);
    return ret;
}


/**
* Check interrupt status for link change.
* Call from mac driver's internal ISR for phy's interrupt.
*/
int(*itpPhyLinkChange)(void) = NULL;
/**
* Replace mac driver's ISR for phy's interrupt.
*/
ITHGpioIntrHandler itpPhylinkIsr = rtl8304mb_linkIntrHandler;
/**
* Returns 0 if the device reports link status up/ok
*/
int(*itpPhyReadMode)(int* speed, int* duplex) = rtl8304mb_read_mode;
/**
* Get link status.
*/
uint32_t(*itpPhyLinkStatus)(void) = rtl8304mb_get_link_status;
