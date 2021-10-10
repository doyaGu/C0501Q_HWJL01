/*
* Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
*/
/** @file
*
* @author Irene Lin
*/
#include <unistd.h>
#include <linux/spinlock.h>
#include <linux/fls.h>
#include <linux/mmc/errorno.h>
#include <linux/mmc/host.h>
#include "ite/ith.h"
#include "ite/ite_sd.h"
#include "sdc_error.h"
#include "sdc_reg.h"
#include "dbg.h"

#define USE_DMA         1
#define SD_IRQ_ENABLE
#define SD_DEFAULT_SWITCH_TO_SDIO

#if defined(WIN32)
#undef SD_IRQ_ENABLE
#define ithPrintf   printf
#endif

#if defined(SD_IRQ_ENABLE)
#include "openrtos/FreeRTOS.h"
#include "openrtos/timers.h"
#define SD_IRQ_TIMEOUT
#if defined(CFG_SDIO_ENABLE)
#define SDIO_INTR_EN
#endif

#if defined(SDIO_INTR_EN) && defined(CFG_SDIO_ENABLE) && (CFG_CHIP_FAMILY == 9850)
#define SD_CLOCK_ALWAYS_ON  /* for SDIO interrupt */
#endif
#endif

struct sdc_host {
    struct mmc_host		*mmc;
    spinlock_t		    lock;

    unsigned long		clock;
    unsigned int		clk_div;
    unsigned int		bus_width;
    unsigned int        bus_clk;
    unsigned int        ctrl;

    int			        dma;
    int                 enable_sdio_intr;

    struct mmc_request	*mrq;
    struct mmc_command	*cmd;
    struct mmc_data		*data;

    unsigned int		timeout_ms;
    unsigned int		dma_len;
    unsigned int		dma_dir;
    unsigned int		index;

    unsigned int		flags;
    unsigned int		flags_isr;
#define SDC_FLAG_INTR_CMD    0x01
#define SDC_FLAG_INTR_WRAP   0x02
#define SDC_FLAG_INTR_DMA    0x04
#define SDC_FLAG_INTR_ERR    0x08

#define SDC_FLAG_INTR_MSK    0x07

#if defined(SD_IRQ_TIMEOUT)
    TimerHandle_t       timer;
#endif
};

struct sdc {
    struct sdc_host     *sdc[2];

    int		            dma_rx;
    int		            dma_tx;
    volatile unsigned int	active_sdc;
};

static struct sdc   ctxt;

#if (CFG_CHIP_FAMILY == 9910)
pthread_mutex_t sd_mutex = PTHREAD_MUTEX_INITIALIZER;
#else
static void *sd_mutex;
#endif

#include "sdc.c"

static void ite_enable_sdio_irq(struct mmc_host *mmc, int enable);

static void ite_mutex_lock(struct mmc_host *mmc)
{
#if (CFG_CHIP_FAMILY == 9910)
    pthread_mutex_lock(&sd_mutex);
#else
    struct sdc_host *host = mmc_priv(mmc);

    if(sd_mutex == NULL)
		sd_mutex = ithStorMutex;
    ithLockMutex(sd_mutex);
    //ithPrintf(" < \n");

    #if defined(SD_DEFAULT_SWITCH_TO_SDIO)
    /* sd card cmd18 will trigger sdio interrupt, so..... we should disable sdio interrupt when SD/MMC card */
    if (mmc->card->type < SD_TYPE_SDIO)
        ite_enable_sdio_irq(mmc, host->enable_sdio_intr);
    #endif
#endif
}

static void ite_mutex_unlock(struct mmc_host *mmc)
{
#if (CFG_CHIP_FAMILY == 9910)
    pthread_mutex_unlock(&sd_mutex);
#else
    #if defined(SD_DEFAULT_SWITCH_TO_SDIO)
    struct sdc_host *host = NULL;

    ithEnterCritical();
    host = ctxt.sdc[!ctxt.active_sdc]; /* another host */

    if (host) {
        if (host->mmc->card->type >= SD_TYPE_SDIO) {
            SD_SetClockDivReg(host->clk_div);
            SD_SetBusWidth(host->bus_width);
            sdc_io_select(host->index);
            ithWriteRegA(SD_REG_CTL, host->ctrl);  /* set switch pin1 and pin4 of sd data */
            ctxt.active_sdc = host->index;
			//ithPrintf("sd%d - u\n", ctxt.active_sdc);
            /* sdio interrupt maybe disable by SD/MMC card, so... we should restore it back */
            ite_enable_sdio_irq(host->mmc, host->enable_sdio_intr);
        }
    }
    ithExitCritical();
    #endif

    //ithPrintf(" > \n");
    ithUnlockMutex(sd_mutex);
#endif

}

static void ite_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
    struct sdc_host *host = mmc_priv(mmc);

    host->clk_div = sdc_set_clock(host, ios->clock);
    host->bus_width = sdc_set_bus_width(ios->bus_width);
}

static int ite_get_ro(struct mmc_host *mmc)
{
    struct sdc_host *host = mmc_priv(mmc);
    bool locked = ithCardLocked((host->index == SD_0) ? ITH_CARDPIN_SD0 : ITH_CARDPIN_SD1);

    return (locked == true) ? 1 : 0;
}

static int ite_get_cd(struct mmc_host *mmc)
{
    struct sdc_host *host = mmc_priv(mmc);

    return (sdc_is_card_inserted(host->index) == true) ? 1 : 0;
}

static void ite_enable_sdio_irq(struct mmc_host *mmc, int enable)
{
    struct sdc_host *host = mmc_priv(mmc);

    host->enable_sdio_intr = enable;

#if defined(SD_IRQ_ENABLE)
    if (enable) {
        if (host->index == ctxt.active_sdc) {  /* for sd + sdio, it will enable when sd card unlock */
            //ithPrintf(" + \n");
            SD_EnableSdioInterrupt();
        }
    }
    else {
        SD_DisableSdioInterrupt();
        //ithPrintf(" - \n");
    }
#endif
}

static void ite_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
    int rc = 0;
    struct sdc_host *host = mmc_priv(mmc);

    WARN_ON(host->mrq != NULL);

    /* for dual sd interface */
    if (host->index != ctxt.active_sdc) {
        SD_SetClockDivReg(host->clk_div);
        SD_SetBusWidth(host->bus_width);
        ctxt.active_sdc = host->index;
    }
    sdc_io_select(host->index);

    if (sdc_is_card_inserted(host->index) == false) {
        mrq->cmd->error = -ENOMEDIUM;
        mmc_request_done(mmc, mrq);
        return;
    }

    host->mrq = mrq;
    host->data = mrq->data;
    host->timeout_ms = 50;

    if (mrq->data) {
        host->timeout_ms += mrq->data->timeout_ns / 1000000;
        sdc_set_data_size(mrq->data->blocks, mrq->data->blksz);
        if (host->dma)
            sdc_setup_data_dma(&ctxt, host, mrq->data);
    }
    LOG_INFO "timeout: %d\n", host->timeout_ms LOG_END

    sdc_start_cmd(host, mrq->cmd);

#if !defined(SD_IRQ_ENABLE)
    if (mrq->data) {
        if (host->dma)
            rc = sdc_rw_data_dma(host, &ctxt); /* check ready for read/write data by DMA mode */
        else
            rc = sdc_rw_data_pio(host, mrq->data); /* read/write data for PIO mode */
    }
    if (!mrq->data)
    {
        rc = sdc_wait_sdc_ready(host);
        sdc_error_check(host);
        sdc_finish_request(host, host->mrq);
    }
#endif

    check_result(rc);
    return;
}

static const struct mmc_host_ops sdc_ops = {
    .request = ite_request,
    .get_ro = ite_get_ro,
    .get_cd = ite_get_cd,
    .set_ios = ite_set_ios,
    .enable_sdio_irq = ite_enable_sdio_irq,
    .lock = ite_mutex_lock,
    .unlock = ite_mutex_unlock,
};

struct mmc_host* mmc_get_host(int index)
{
	if(ctxt.sdc[index] == NULL)
		return NULL;
	else
	    return ctxt.sdc[index]->mmc;
}

int iteSdcInitialize(int index, SD_CARD_INFO *card_info)
{
    int ret = 0;
    struct mmc_host *mmc;
    struct sdc_host *host = ctxt.sdc[index];
    
    memset((void*)card_info, 0x0, sizeof(card_info));

    if (host) {
        mmc = host->mmc;
        goto get_info;
    }

    mmc = mmc_alloc_host(sizeof(struct sdc_host), NULL);
    if (!mmc) {
        ret = -ENOMEM;
        goto out;
    }

    mmc->ops = &sdc_ops;
    mmc->max_blk_size = 512;
    mmc->max_blk_count = 65535;
    mmc->max_req_size = mmc->max_blk_size * mmc->max_blk_count;

    host = mmc_priv(mmc);
    host->mmc = mmc;
    host->dma = USE_DMA;
    host->bus_clk = ithGetBusClock();
    host->clock = 300000;
    host->ctrl = (ithSdSwitchPin1(index) == true) ? SD_MSK_SWITCH_PIN1_4 : 0x0;
#if defined(SD_CLOCK_ALWAYS_ON)
    /* hardware issue: sdio need clock always on. Sdio interrutp will fail if clock gating.  */
    host->ctrl |= SD_MSK_CLK_ALWAYS_ON;
#endif
    mmc->f_min = 100000;
    mmc->f_max = 40000000;
    mmc->name = (index == 0) ? "sdc0" : "sdc1";

    mmc->ocr_avail = MMC_VDD_32_33 | MMC_VDD_33_34;
    mmc->caps = MMC_CAP_4_BIT_DATA | 
#if defined(SDIO_INTR_EN)
        MMC_CAP_SDIO_IRQ | 
#endif
        MMC_CAP_MMC_HIGHSPEED | 
        MMC_CAP_SD_HIGHSPEED;
    if (1)
        mmc->caps |= (MMC_CAP_8_BIT_DATA|MMC_CAP_BUS_WIDTH_TEST);

    spin_lock_init(&host->lock);

    if (host->dma && !ctxt.dma_rx && !ctxt.dma_tx)
    {
        ctxt.dma_rx = ctxt.dma_tx = -1;
#if defined(SD_IRQ_ENABLE)
        ctxt.dma_rx = ithDmaRequestCh("sd_rx", ITH_DMA_CH_PRIO_HIGH_3, dma_isr, (void*)&ctxt);
        ctxt.dma_tx = ithDmaRequestCh("sd_tx", ITH_DMA_CH_PRIO_HIGH_3, dma_isr, (void*)&ctxt);
#else
        ctxt.dma_rx = ithDmaRequestCh("sd_rx", ITH_DMA_CH_PRIO_HIGH_3, NULL, NULL);
        ctxt.dma_tx = ithDmaRequestCh("sd_tx", ITH_DMA_CH_PRIO_HIGH_3, NULL, NULL);
#endif
        if (ctxt.dma_rx < 0 || ctxt.dma_tx < 0)
        {
            ret = ERR_SD_REQUEST_DMA_FAIL;
            goto out;
        }

        ithDmaReset(ctxt.dma_rx);
        ithDmaSetBurst(ctxt.dma_rx, ITH_DMA_BURST_128);
        ithDmaSetSrcAddr(ctxt.dma_rx, SDW_REG_DATA_PORT);
        ithDmaSetRequest(ctxt.dma_rx, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SD, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM);
        ithDmaSetSrcParams(ctxt.dma_rx, ITH_DMA_WIDTH_32, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);

        ithDmaReset(ctxt.dma_tx);
        ithDmaSetBurst(ctxt.dma_tx, ITH_DMA_BURST_128);
        ithDmaSetDstAddr(ctxt.dma_tx, SDW_REG_DATA_PORT);
        ithDmaSetRequest(ctxt.dma_tx, ITH_DMA_NORMAL_MODE, ITH_DMA_MEM, ITH_DMA_HW_HANDSHAKE_MODE, ITH_DMA_HW_SD);
        ithDmaSetDstParams(ctxt.dma_tx, ITH_DMA_WIDTH_32, ITH_DMA_CTRL_FIX, ITH_DMA_MASTER_1);
    }

#if defined(SD_IRQ_TIMEOUT)
    host->timer = xTimerCreate("sd_timeout", 
        50, 
        pdFALSE, // one-shot timer
        host,
        sdc_timeout_handler);
    if (host->timer == NULL)
        LOG_ERROR "%s: create timer fail! \n", __func__ LOG_END
#endif

    ctxt.active_sdc = index;
    ctxt.sdc[ctxt.active_sdc] = host;
	mmc->index = host->index = index;

    mmc_claim_host(mmc);
    ret = sdc_init(host);
    mmc_release_host(mmc);
    if (ret)
        goto out;
    
    //if (ctxt.sdc[!index] == NULL)  /* it will be reset */
        sdc_interrupt_enable(&ctxt);

    mmc_add_host(mmc);
    
get_info:
    if (mmc->card) {
        card_info->type = mmc->card->type;
        card_info->ctxt = mmc->card;
		card_info->sdc_idx = index;

        if (card_info->type >= SD_TYPE_SDIO) {
            card_info->sdio_drv_num = mmc->card->sdio_drv_num;
            memcpy((void*)card_info->sdio_info, 
                    (void*)mmc->card->sdio_info, 
                    sizeof(struct sdio_func_info)*mmc->card->sdio_drv_num);
        }
    }

    return 0;

out:
    if (host) {
        if (ctxt.dma_rx >= 0)
            ithDmaFreeCh(ctxt.dma_rx);
        if (ctxt.dma_tx >= 0)
            ithDmaFreeCh(ctxt.dma_tx);
        ctxt.dma_rx = ctxt.dma_tx = 0;
    }
    if (mmc)
        mmc_free_host(mmc);

    ctxt.sdc[index] = NULL;

    check_result(ret);

    return ret;
}

int iteSdcTerminate(int index)
{
    struct sdc_host *host = ctxt.sdc[index];
    struct mmc_host *mmc = NULL;

    if(host == NULL)
		return 0;

	mmc = host->mmc;
    mmc_remove_host(mmc);
    //sdc_stop_clock(host);

    /* disable all interrupt */
    // TODO: disable all interrupt

#if defined(SD_IRQ_TIMEOUT)
    if( xTimerIsTimerActive( host->timer ) != pdFALSE )
        printf(" timer is ACTIVE! \n");
    if (xTimerStop(host->timer, 2) == pdFAIL)
        LOG_ERROR "%s: stop timer fail! \n", __func__ LOG_END
    if (xTimerDelete(host->timer, 2) == pdFAIL)
        LOG_ERROR "%s: delete timer fail! \n", __func__ LOG_END
#endif

    if (host->dma && (ctxt.sdc[!index] == NULL))
    {
        if (ctxt.dma_rx >= 0)
            ithDmaFreeCh(ctxt.dma_rx);
        if (ctxt.dma_tx >= 0)
            ithDmaFreeCh(ctxt.dma_tx);
        ctxt.dma_rx = ctxt.dma_tx = 0;

        sdc_interrupt_disable();
    }

    mmc_free_host(mmc);
    ctxt.sdc[index] = NULL;

    return 0;
}

int iteSdcGetType(int index)
{
    struct sdc_host *host = ctxt.sdc[index];
    struct mmc_host *mmc;

    if (!host)
        return -1;

    mmc = host->mmc;

    if (!mmc->card)
        return -1;

    return mmc->card->type;
}
