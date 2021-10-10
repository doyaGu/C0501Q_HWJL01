


#if defined(MAC_TASK)
MAC_SEM   isr_sem;
#endif


static void checkLinkStatus(struct eth_device *netdev)
{
    struct ftmac100 *priv = netdev->priv;
    struct mii_if_info *mii_if = &priv->mii;
    MAC_UINT32 status;

    if (!priv->cfg->phy_link_change) {
		ithPrintf ("phy_link_change callback is NULL! Please implement it! \n");
		return;
    }

    if(NETIF_RUNNING(netdev) && priv->cfg->phy_link_change()) 
    {
        int cur_link = mii_link_ok(&priv->mii);
        int prev_link = NETIF_CARRIER_OK(netdev);
        
        if (cur_link && !prev_link)
        {
            mac_set_ctrl_reg(priv);
            NETIF_CARRIER_ON(netdev);
        }
        else if (prev_link && !cur_link)
        {
            NETIF_CARRIER_OFF(netdev);
        }
        //ithPrintf("\n\n PHY status change! \n\n");
    }
}

volatile static MAC_UINT32 intr_status;
/******************************************************************************
 * interrupt handler
 *****************************************************************************/
 
#if defined(MAC_DEFER_ISR)
static void ftmac100_poll(void *arg1, uint32_t arg2);
#endif

static void ftmac100_interrupt(void *arg)
{
    struct eth_device *netdev = arg;
    struct ftmac100 *priv = netdev->priv;

    //ithPrintf(" mac isr \n");
    
    if (NETIF_RUNNING(netdev)) {
        /* Disable interrupts for polling */
        ftmac100_disable_all_int(priv);
        MAC_ReadReg(priv->base + FTMAC100_OFFSET_ISR, &intr_status);

        ithFlushAhbWrap();
		#if defined(MAC_TASK)
        itpSemPostFromISR(isr_sem);
		#endif
		#if defined(MAC_DEFER_ISR)
		itpPendFunctionCallFromISR(ftmac100_poll, netdev, 0);
		#endif
    }

    return;
}


static void linkIntrHandler(unsigned int pin, void *arg)
{
    struct eth_device *netdev = (struct eth_device *)arg;
    struct ftmac100 *priv = netdev->priv;

    //ithPrintf(" gpio isr \n");

    if(pin != priv->linkGpio)
    {
        ithPrintf(" MAC: phy link gpio error! %d \n", pin);
        return;
    }
   
    checkLinkStatus(netdev);

    return;
}

#if defined(MAC_DEFER_ISR)
#define BUDGET  (RX_QUEUE_ENTRIES*3/4)
#else
#define BUDGET  (RX_QUEUE_ENTRIES/2)
#endif

#if defined(MAC_DEFER_ISR)
static void ftmac100_poll(void *arg1, uint32_t arg2)
{
    struct eth_device *netdev = (struct eth_device *)arg1;
#else // MAC_TASK
static void ftmac100_poll(struct eth_device *netdev)
{
#endif
    struct ftmac100 *priv = netdev->priv;
    MAC_UINT32 status;
    MAC_BOOL completed = MAC_TRUE;
    int rx = 0;

    //MAC_ReadReg(priv->base + FTMAC100_OFFSET_ISR, &intr_status);
    //printf(" intr task: 0x%08X \n", intr_status);
    if (intr_status & (FTMAC100_INT_RPKT_FINISH | FTMAC100_INT_NORXBUF)) {
        /*
         * FTMAC100_INT_RPKT_FINISH:
         *	RX DMA has received packets into RX buffer successfully
         *
         * FTMAC100_INT_NORXBUF:
         *	RX buffer unavailable
         */
        MAC_BOOL retry;

        do {
            retry = ftmac100_rx_packet(priv, &rx);
        } while (retry && rx < BUDGET);

        if (retry && rx == BUDGET)
            completed = MAC_FALSE;
    }

    if (intr_status & (FTMAC100_INT_XPKT_OK | FTMAC100_INT_XPKT_LOST)) {
        /*
         * FTMAC100_INT_XPKT_OK:
         *	packet transmitted to ethernet successfully
         *
         * FTMAC100_INT_XPKT_LOST:
         *	packet transmitted to ethernet lost due to late
         *	collision or excessive collision
         */
        ftmac100_tx_complete(priv);
    }

    if (intr_status & (FTMAC100_INT_NORXBUF | FTMAC100_INT_RPKT_LOST |
              FTMAC100_INT_AHB_ERR | FTMAC100_INT_PHYSTS_CHG)) {
        LOG_WARNING "[ISR] = 0x%x: %s%s%s%s\n", intr_status,
                intr_status & FTMAC100_INT_NORXBUF ? "NORXBUF " : "",
                intr_status & FTMAC100_INT_RPKT_LOST ? "RPKT_LOST " : "",
                intr_status & FTMAC100_INT_AHB_ERR ? "AHB_ERR " : "",
                intr_status & FTMAC100_INT_PHYSTS_CHG ? "PHYSTS_CHG" : "" LOG_END

        if (intr_status & FTMAC100_INT_NORXBUF) {
            /* RX buffer unavailable */
            netdev->stats.rx_over_errors++;
        }

        if (intr_status & FTMAC100_INT_RPKT_LOST) {
            /* received packet lost due to RX FIFO full */
            netdev->stats.rx_fifo_errors++;
        }

        if (intr_status & FTMAC100_INT_PHYSTS_CHG) {
            /* PHY link status change */
            #if 0
            mii_check_link(&priv->mii);
            LOG_INFO "\n\nPHY Status Chang!!!!!!!!!!!!! \n\n" LOG_END
            #else
            printf("mac link \n");
            checkLinkStatus(netdev);
            #endif
        }
    }

    if (completed) {
        /* stop polling */
        ftmac100_enable_all_int(priv);
    }
#if defined(MAC_TASK)
    else {
        LOG_WARNING "rx second pass!!!!!! ################## \n" LOG_END
        MAC_ReleaseSem(isr_sem);
    }
#endif

    LOG_INFO "rx cnt %d \n", rx LOG_END
}

#if defined(MAC_TASK)
static volatile int task_done=0;
void* iteMacThreadFunc(void* data)
{
    MAC_CreateSemLock(isr_sem);
    task_done = 1;

wait:
    MAC_WaitSem(isr_sem);
    if(dev)
        ftmac100_poll(dev);
    goto wait;
}
#endif // #if defined(MAC_TASK)
#if defined(WIN32) || defined(MAC_DEFER_ISR)
void* iteMacThreadFunc(void* data)
{
    for( ; ; ) usleep(0xFFFFFFFF);
}
#endif


#if defined(MAC_IRQ_ENABLE)

#include "ite/ith.h"

static void intrEnable(struct eth_device *netdev)
{
    struct ftmac100 *priv = netdev->priv;

    /** register interrupt handler to interrupt mgr */
    ithIntrRegisterHandlerIrq(ITH_INTR_MAC, ftmac100_interrupt, (void*)netdev);
    ithIntrSetTriggerModeIrq(ITH_INTR_MAC, ITH_INTR_LEVEL);
    ithIntrSetTriggerLevelIrq(ITH_INTR_MAC, ITH_INTR_HIGH_RISING);
    ithIntrEnableIrq(ITH_INTR_MAC);

    #if defined(LINK_INTR)
    ithPrintf(" link gpio %d \n", priv->linkGpio);
    if(priv->linkGpio>0)
    {
        /** register phy link up/down interrupt handler to gpio mgr, GPIO */
        ithGpioEnable(priv->linkGpio);
        ithGpioSetIn(priv->linkGpio);
        ithGpioClearIntr(priv->linkGpio);
        if (priv->cfg->linkGpio_isr)
            ithGpioRegisterIntrHandler(priv->linkGpio, priv->cfg->linkGpio_isr, (void*)netdev);
        else
            ithGpioRegisterIntrHandler(priv->linkGpio, linkIntrHandler, (void*)netdev);
        ithGpioCtrlEnable(priv->linkGpio, ITH_GPIO_INTR_LEVELTRIGGER);  /* use level trigger mode */
        ithGpioCtrlEnable(priv->linkGpio, ITH_GPIO_INTR_TRIGGERFALLING); /* low active */
        ithGpioEnableIntr(priv->linkGpio);
    }
    #endif

#if defined(MAC_TASK)
    if(!task_done)
        LOG_ERROR " Task not create!!! \n\n" LOG_END
#endif

}

static void intrDisable(struct ftmac100 *priv)
{
    ithIntrDisableIrq(ITH_INTR_MAC);
}

#else
#define intrEnable(a)
#define intrDisable(a)
#endif
