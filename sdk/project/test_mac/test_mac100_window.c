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

#define TOTAL_NUM   (62-8)

static uint8_t param[TOTAL_NUM][2];
static int sucess_cnt;


int DoTest(int clock, int delay)
{
#define TEST_CNT    10000
    int res = 0;
    int fail_cnt = 0;

    iteMacSetClock(clock, delay);

    printf(" \r\n", delay);
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
            //printf("clock: %s, delay: 0x%02X => Fail! %d \r\n", clock ? "Inverse" : "Normal", delay, tx_cnt);
            //return -1;
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
	/*
    if (tx_cnt == TEST_CNT) {
        printf("clock: %s, delay: 0x%02X => Success! \r\n", clock ? "Inverse" : "Normal", delay);
        return 0;
    }
    */
    if(fail_cnt==0)
    {
        printf("clock: %s, delay: 0x%02X => Success! \r\n", clock ? "Inverse" : "Normal", delay);
        return 0;
    }
    else
    {
        printf("clock: %s, delay: 0x%02X => Fail! (%d/%d pass) \r\n", clock ? "Inverse" : "Normal", delay, (TEST_CNT-fail_cnt), TEST_CNT);
        return -1;
    }
}

void DoTestClock(int clock)
{
    uint32_t delay;
    
    for (delay = 0; delay <= 0x0F; delay++)
    {
        if (DoTest(clock, delay) == 0) {
            param[success_cnt][0] = clock;
            param[success_cnt][1] = delay;
            success_cnt++;
        }
    }
    
    for (delay = 0x1F; delay <= 0xBF; delay += 16)
    {
        if (DoTest(clock, delay) == 0) {
            param[success_cnt][0] = clock;
            param[success_cnt][1] = delay;
            success_cnt++;
        }
    }
}

static uint8_t param2[TOTAL_NUM][2];

static int idx = 0;
static int first_fail_idx;
static int second_success_idx;

#define MAC_SUCCESS     1
#define MAC_FAIL        0
static int status = MAC_SUCCESS;

/* success..... fail..... success */
void DoScan2(int clock)
{
    uint32_t delay;

    for (delay = 0; delay <= 0x0F; delay++)
    {
        if (DoTest(clock, delay) == 0) {
            if (status == MAC_FAIL)
                second_success_idx = idx;

            status = MAC_SUCCESS;
        }
        else {
            if (status == MAC_SUCCESS)
                first_fail_idx = idx;

            status = MAC_FAIL;
        }

        param2[idx][0] = clock;
        param2[idx][1] = delay;
        idx++;
    }

    for (delay = 0x1F; delay <= 0xBF; delay += 16)
    {
        if (DoTest(clock, delay) == 0) {
            if (status == MAC_FAIL)
                second_success_idx = idx;

            status = MAC_SUCCESS;
        }
        else {
            if (status == MAC_SUCCESS)
                first_fail_idx = idx;

            status = MAC_FAIL;
        }

        param2[idx][0] = clock;
        param2[idx][1] = delay;
        idx++;
    }
}


static int
MainLoop(
    void)
{
    int res;
    uint8_t macaddr[6];
    int linkup = 0;
    int clock;
    int test0, test1;
    int center;

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

    
    printf(" Try clock... \r\n");
    test0 = DoTest(0, 0x00);
    test1 = DoTest(1, 0x00);

    printf("\r\n\r\n Start Scan...... \r\n");

    /* success..... fail..... success */
    if ((test0 == 0) && (test1 == 0))
        goto case2;

case1:
    /* fail.... success.... fail */
    if (test0 < 0)
        clock = 0;  
    else
        clock = 1;

    success_cnt = 0;
    DoTestClock(clock);
    DoTestClock(!clock);
    printf("\n\n Scan DONE! \n\n");
    printf("\r\n Safe setting => clock:%d, delay:0x%02X \r\n", param[success_cnt/2][0], param[success_cnt/2][1]);
    while (1);

case2:
    DoScan2(0);
    DoScan2(1);
    printf("\n\n Scan DONE! \n\n");
    center = (first_fail_idx + (TOTAL_NUM - second_success_idx)) / 2;
    center = (second_success_idx + center) % TOTAL_NUM;
    printf("\r\n Safe setting => clock:%d, delay:0x%02X \r\n", param2[center][0], param2[center][1]);

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
    printf(" Not support! \r\n");
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

