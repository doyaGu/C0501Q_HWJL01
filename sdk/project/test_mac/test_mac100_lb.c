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

void RxCallback(void* ctx, void* data, int len)
{
	int i;
	uint8_t* rx_data = (uint8_t*)data;
	static uint32_t pktCnt = 0;
	uint8_t success=1;

	printf("\r\n[%04d, %d] \n", len, ++pktCnt);
	if(len != packet_len)
	{
		printf("Rx length %d != Tx length %d \n\n", len, packet_len);
		len = (len > packet_len) ? len : packet_len;
		success = 0;
	}

    usleep(5*1000); // delay for print buffer
	printf("Compare packet data: ");
	{
		for(i=0; i<len; i++)
		{
			if(rx_data[i] != tx_packet[i])
			{
				success=0;
				break;
			}
		}
		if(success)
		{
			success_cnt++;
			printf("Sucess!! %d / %d \n\n", success_cnt, tx_cnt);
		}
		else
		{
			fail_cnt++;
			printf("Fail!! %d / %d \n\n", fail_cnt, tx_cnt);
		}
	}

	if(!success)
	{
		for(i=0; i<len; i++)
		{
			if(!(i%0xF))
				printf("\n");
			printf("%02x ", rx_data[i]);
            if(!(i % 0x80)) usleep(5*1000); // delay for print buffer
		}
		printf(" Rx \r\n");

        usleep(5*1000); // delay for print buffer

		for(i=0; i<len; i++)
		{
			if(!(i%0xF))
				printf("\n");
			printf("%02x ", tx_packet[i]);
            if(!(i % 0x80)) usleep(5*1000); // delay for print buffer
		}
		printf("Tx \r\n");
        while(1); // stop for print buffer
	}
	printf("\n\n");
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
    mac_cfg.clk_inv 	  = CFG_NET_MAC_CLOCK_INVERSE;
    mac_cfg.clk_delay	  = CFG_NET_MAC_CLOCK_DELAY;
    mac_cfg.phyAddr = CFG_NET_ETHERNET_PHY_ADDR;
    mac_cfg.ioConfig = ioConfig;
    mac_cfg.linkGpio = CFG_GPIO_ETHERNET_LINK;
    mac_cfg.phy_link_change = itpPhyLinkChange;
    mac_cfg.linkGpio_isr  = itpPhylinkIsr;
    mac_cfg.phy_link_status = itpPhyLinkStatus;
    mac_cfg.phy_read_mode = itpPhyReadMode;

#if defined(CFG_NET_ETHERNET_LINK_INTR)
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
    if(1) {
        int i;
		for(i=0; i<PACKET_SIZE; i++)
			packet[i] = (uint8_t)((i+1) % 0x100);
    }
	if (0) {
		int i;
		for (i = 0; i < PACKET_SIZE / 2; i++) {
			packet[i*2] = (uint8_t)(i & 0xFF);
			packet[i*2+1] = (uint8_t)((i>>8) & 0xFF);
		}
	}

    return res;
}


static int
MainLoop(
    void)
{
    int res = 0;
    uint8_t macaddr[6];
    int linkup = 0;

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
	
    linkup = 1;
    printf("\n link up! \n\n");
#endif

	tx_packet = packet;
	packet_len = PACKET_SIZE;
    for(;;)
    {
		tx_cnt++;
		res = iteMacSend(tx_packet, packet_len);
		if (res) {
			printf("iteMacSend() rc = %d \n", res);
			while (1);
		}
		//while (1);
        sem_wait(sem);
if(tx_cnt >= 10) while(1);
		if(packet_len<=64)
		{
			tx_packet = packet;
			packet_len = PACKET_SIZE;
		}
		else
		{
			packet_len-=1;
			tx_packet+=1;
		}
    }

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
