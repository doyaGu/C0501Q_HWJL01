#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "ite/ite_mac.h"



static uint8_t macaddr[] = {0x02, 0x22, 0x33, 0x44, 0x55, 0x66};
static const uint8_t ioConfig[] = { CFG_GPIO_ETHERNET };
static ITE_MAC_CFG_T mac_cfg;

#define TX_MB_SIZE          50
#define TEST_SIZE_CNT		8
static uint32_t packet_size[TEST_SIZE_CNT] = {64, 128, 256, 384, 512, 768, 1024, 1500};
static uint8_t packet[1500];


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
		for(i=0; i<1500; i++)
			packet[i] = (uint8_t)((i+1) % 0x100);
    }
    return res;
}

void RxCallback(void* ctx, void* data, int len)
{
	int i;
	uint8_t* rx_data = (uint8_t*)data;
	static uint32_t pktCnt = 0;

	printf("\r\n[%04d, %d] \n", len, ++pktCnt);

#if defined(DUMP_RX_DATA)
	for(i=0; i<len; i++)
	{
		if(!(i%0x10))
			printf("\n");
		printf("%02x ", rx_data[i]);
	}
	printf("\n\n");
#endif
}


static int
MainLoop(
    void)
{
    int res = 0;
    uint8_t macaddr[6];
    int linkup = 0;
    uint32_t total_packets, total_bytes, i, size_idx;
    uint32_t tx_cnt=0, tx_bytes=0, rx_cnt=0, rx_bytes=0;
    uint32_t time;

wait_linkup:
    printf("\n wait link up...... \n\n");
    while(!iteEthGetLink()) usleep(10*1000);

    linkup = 1;
    printf("\n link up! \n\n");

	ithTimerCtrlEnable(ITH_TIMER3, ITH_TIMER_UPCOUNT);

	for(size_idx=0; size_idx < TEST_SIZE_CNT ; size_idx++)
	{
		res = iteMacOpen(macaddr, RxCallback, NULL, ITE_ETH_REAL);
		if(res)
		{
			printf(" iteMacOpen() fail 0x%08X \n", res);
			while(1);
		}

		printf(" \n=============== packet: %d bytes ===================\n", packet_size[size_idx]);
		total_packets = (TX_MB_SIZE*1024*1024)/packet_size[size_idx];
		total_bytes = total_packets * packet_size[size_idx];
		printf(" total cnt: %d, total bytes: %d \n\n", total_packets, total_bytes);

		iteMacGetStats(&tx_cnt, &tx_bytes, &rx_cnt, &rx_bytes);
		printf(" START: %d, %d, %d, %d \n\n", tx_cnt, tx_bytes, rx_cnt, rx_bytes);

	ithTimerSetCounter(ITH_TIMER3, 0);
	ithTimerEnable(ITH_TIMER3);

		for(i=0; i<total_packets; i++)
		{
			res = iteMacSend(packet, packet_size[size_idx]);
		}

		iteMacGetStats(&tx_cnt, &tx_bytes, &rx_cnt, &rx_bytes);
		while(tx_cnt != total_packets)
			iteMacGetStats(&tx_cnt, &tx_bytes, &rx_cnt, &rx_bytes);
		printf(" %d, %d, %d, %d \n\n", tx_cnt, tx_bytes, rx_cnt, rx_bytes);

	time = ithTimerGetTime(ITH_TIMER3);
	ithTimerDisable(ITH_TIMER3);
		printf(" END  : %d, %d, %d, %d \n\n", tx_cnt, tx_bytes, rx_cnt, rx_bytes);
		printf(" total time: %d us ==> %f Mbps \n\n", time, ((double)((tx_bytes*8)/(1024*1024))/time)*1000000 );

		iteMacStop();
	}
printf("\n\n stop!!!!! \n");
while(1);

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
