#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ite/itp.h"
#include "ite/ite_mac.h"




#define PACKET_SIZE		1500//1514
static uint8_t packet[PACKET_SIZE];
static int packet_len;
static uint8_t* tx_packet;
static uint32_t success_cnt;
static uint32_t fail_cnt;
static uint32_t tx_cnt;
static sem_t* sem;

static uint8_t success;


void RxCallback(void* ctx, void* data, int len)
{
    sem_post(sem);
}



static uint8_t macaddr[] = {0x02, 0x22, 0x33, 0x44, 0x55, 0x66};
static const uint8_t ioConfig[] = { CFG_GPIO_ETHERNET };
static ITE_MAC_CFG_T mac_cfg;

static int
Initialize(void)
{
    int res;

    mac_cfg.flags |= (ITH_COUNT_OF(ioConfig) == ITE_MAC_GRMII_PIN_CNT) ? ITE_MAC_GRMII : 0;
    mac_cfg.clk_inv = CFG_NET_MAC_CLOCK_INVERSE;
    mac_cfg.clk_delay = CFG_NET_MAC_CLOCK_DELAY;
    mac_cfg.phyAddr = CFG_NET_ETHERNET_PHY_ADDR;
    mac_cfg.ioConfig = ioConfig;
    mac_cfg.linkGpio = CFG_GPIO_ETHERNET_LINK;
    mac_cfg.phy_link_change = itpPhyLinkChange;
    mac_cfg.linkGpio_isr  = itpPhylinkIsr;
    mac_cfg.phy_link_status = itpPhyLinkStatus;
    mac_cfg.phy_read_mode = itpPhyReadMode;

    if (mac_cfg.flags & ITE_MAC_GRMII) {
        printf(" This tool NOT support giga ethernet (RGMII) !!!!! \n");
        while (1);
    }

#if defined(CFG_GPIO_ETHERNET_LINK)
    // enable gpio interrupt
    ithIntrEnableIrq(ITH_INTR_GPIO);
#endif
    res = iteMacInitialize(&mac_cfg);
    if(res)
    {
        printf(" iteMacInitialize() fail! \n");
        while(1);
    }
    res = iteMacSetMacAddr(macaddr);
    if(res)
    {
        printf(" iteMacSetMacAddr() fail! \n");
        while(1);
    }
    PhyInit(CFG_ETH_MODE);
	
    /** fill test pattern */
    {
        int i;
		for(i=0; i<PACKET_SIZE; i++)
			//packet[i] = (uint8_t)((i+1) % 0x100);
			packet[i] = (uint8_t)(rand() % 0x100);
    }

    return res;
}

#define TEST_CNT    10000

#define RXD_DELAY_CNT       (16)
#define REFCLK_DELAY_CNT    (16)

static uint32_t g_fail_cnt[REFCLK_DELAY_CNT][2][RXD_DELAY_CNT];


int DoTest(int clk_inv, int refclk_delay, int rxd_delay)
{
    int res = 0;
    int fail_cnt = 0;

    iteMacSetClock(clk_inv, refclk_delay, rxd_delay);

    tx_packet = packet;
    packet_len = PACKET_SIZE;
    for (tx_cnt = 0; tx_cnt<TEST_CNT; tx_cnt++)
    {
        res = iteMacSend(tx_packet, packet_len);
        if (res) {
            printf(" iteMacSend() fail! res = 0x%X \n", res);
            return -1;
        }

        res = itpSemWaitTimeout(sem, 2);
        if (res) {
            fail_cnt++;
        }

        if (packet_len <= 64)
        {
            tx_packet = packet;
            packet_len = PACKET_SIZE;
        }
        else
        {
            packet_len -= 1;
            tx_packet += 1;
        }
    }

    g_fail_cnt[refclk_delay][clk_inv][rxd_delay] = fail_cnt;
    if(fail_cnt==0)
    {
        printf("clock: %s, rxd_delay: 0x%02X => Success! \r\n", clk_inv ? "Inverse" : "Normal", rxd_delay);
        return 0;
    }
    else
    {
        printf("clock: %s, rxd_delay: 0x%02X => Fail! (%d/%d pass) \r\n", clk_inv ? "Inverse" : "Normal", rxd_delay, (TEST_CNT - fail_cnt), TEST_CNT);
        return -1;
    }
}

void Scan(void)
{
    uint32_t clk_inv, refclk_delay, rxd_delay;
    
    for (refclk_delay = 0; refclk_delay < REFCLK_DELAY_CNT; refclk_delay++) {
        printf(" \n\n refclk delay = %d \n", refclk_delay);

        for (clk_inv = 0; clk_inv < 2; clk_inv++) {
            for (rxd_delay = 0; rxd_delay < RXD_DELAY_CNT; rxd_delay++) {
                DoTest(clk_inv, refclk_delay, rxd_delay);
            }
        }
    }

}

void ShowResult(void)
{
    int refclk_delay, clk_inv, rxd_delay;

    printf("\n\n");
    printf("The number is success count!!\n\n");
    printf("refclk delay:  ");
    for (refclk_delay = 0; refclk_delay < REFCLK_DELAY_CNT; refclk_delay++)
        printf("%6d", refclk_delay);
    printf("\n");

    for (clk_inv = 0; clk_inv < 2; clk_inv++) {
        for (rxd_delay = 0; rxd_delay < RXD_DELAY_CNT; rxd_delay++) {
            printf("%d rxd_delay:%2d", clk_inv, rxd_delay);
            for (refclk_delay = 0; refclk_delay < REFCLK_DELAY_CNT; refclk_delay++)
                printf("%6d", (TEST_CNT - g_fail_cnt[refclk_delay][clk_inv][rxd_delay]));
            printf("\n");
        }
    }
}

static int
MainLoop(
    void)
{
    int res;
    uint8_t macaddr[6];

    res = iteMacOpen(macaddr, RxCallback, NULL, CFG_ETH_MODE);
    if(res)
    {
        printf(" iteMacOpen() fail 0x%08X \n", res);
        while(1);
    }

#if (CFG_ETH_MODE != ITE_ETH_MAC_LB)
wait_linkup:
    printf("\n wait link up...... \n\n");
    while(!iteEthGetLink()) usleep(10*1000);
	
    printf("\n link up! \n\n");
#endif

    printf("\r\n\r\n Start Scan...... \r\n");

    Scan();
    printf("\n\n Scan DONE! \n\n");

    ShowResult();

    while (1);

    return res;
}


static int
Terminate(void)
{
    return 0;
}


void* TestFunc(void* arg)
{
	int res;
	
#define TEST_STACK_SIZE 102400
	pthread_t task;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, TEST_STACK_SIZE);
	param.sched_priority = sched_get_priority_max(1);
	pthread_attr_setschedparam(&attr, &param);
	pthread_create(&task, &attr, iteMacThreadFunc, NULL);

	sem = malloc(sizeof(sem_t)); 
    sem_init(sem, 0, 0);

#if defined(CFG_NET_RTL8304MB)
    printf(" Not support RTL8304MB! \r\n");
    while (1);
#endif

	res = Initialize();
	if (res)
		goto end;

	res = MainLoop();
	if (res)
		goto end;

	res = Terminate();
	if (res)
		goto end;
	
end:
    return NULL;
}

