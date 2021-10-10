/*
* Copyright (c) 2016 ITE Tech. Inc. All Rights Reserved.
*/
/** @file
*
* @author Irene Lin
*/

#include <linux/mmc/errorno.h>
#include "dbg.h"
#include "sdhci.h"

struct sdc {
    struct sdhci_host   *sdc[2];
    volatile unsigned int	active_sdc;
};

static struct sdc   ctxt;
static void *sd0_mutex; /* sd0 pin share with AXISPI */
static pthread_mutex_t sd1_mutex = PTHREAD_MUTEX_INITIALIZER;

struct sdhci_pltfm_data {
    const struct sdhci_ops *ops;
    unsigned int quirks;
    unsigned int quirks2;
};

#define Transfer_Mode_SDMA   1

#if defined(CFG_ITH_FPGA)
#define FTSDC021_BASE_CLOCK  13
#else
#define FTSDC021_BASE_CLOCK  200
#endif

static unsigned int ite_get_max_clk(struct sdhci_host *host)
{
    /* Controller does not specify the base clock frequency.
    * Current design base clock  = SD ODC frequency x 2.
    */
    return FTSDC021_BASE_CLOCK * 1000000;
}

static unsigned int ite_get_timeout_clk(struct sdhci_host *host)
{
    return 33;
}

static unsigned int ite_get_ro(struct sdhci_host *host)
{
    bool locked = ithCardLocked((host->index == SD_0) ? ITH_CARDPIN_SD0 : ITH_CARDPIN_SD1);

    return (locked == true) ? 1 : 0;
}

static unsigned int ite_get_cd(struct sdhci_host *host)
{
    bool cd = ithCardInserted((host->index == SD_0) ? ITH_CARDPIN_SD0 : ITH_CARDPIN_SD1);

    return (cd == true) ? 1 : 0;
}

static void ite_mutex_lock(struct sdhci_host *host)
{
    if (host->index == 0) {
        if (sd0_mutex == NULL)
            sd0_mutex = ithStorMutex;
        ithLockMutex(sd0_mutex);
#if defined(CFG_ITH_FPGA)
		ithWriteRegA(0xB0900000, 0x76543210);
		ithWriteRegA(0xB0900008, 0x00000010);
#endif
	}
	else {
		pthread_mutex_lock(&sd1_mutex);
#if defined(CFG_ITH_FPGA)
		ithWriteRegA(0xB0900000, 0xfedcba98);
		ithWriteRegA(0xB0900008, 0x00000001);
#endif
	}

#if defined(CFG_ITH_FPGA)
	ithWriteRegA(0xD1000000 + 0x60, 0x22220000);
	ithWriteRegA(0xD1000000 + 0x64, 0x00022222);
#else
	//printf("+\n");
	ithStorageSelect((host->index == SD_0) ? ITH_STOR_SD : ITH_STOR_SD1);
#endif
}

static void ite_mutex_unlock(struct sdhci_host *host)
{
    if (host->index == 0)
        ithUnlockMutex(sd0_mutex);
    else
        pthread_mutex_unlock(&sd1_mutex);
	//printf("-\n");
}

static struct sdhci_ops ite_ops = {
    /** for sdhci core use */
    .set_clock = sdhci_set_clock,
    .set_bus_width = sdhci_set_bus_width,
    .reset = sdhci_reset,
    .set_uhs_signaling = sdhci_set_uhs_signaling,
    /** for ite use */
    .get_max_clock = ite_get_max_clk,
    .get_timeout_clock = ite_get_timeout_clk,
    .get_ro = ite_get_ro,
    .get_cd = ite_get_cd,
    .lock = ite_mutex_lock,
    .unlock = ite_mutex_unlock,
};

static struct sdhci_pltfm_data ite_pdata = {
    .ops = &ite_ops,
    .quirks = SDHCI_QUIRK_CAP_CLOCK_BASE_BROKEN |
    SDHCI_QUIRK_BROKEN_TIMEOUT_VAL
#if defined(Transfer_Mode_PIO)
    | SDHCI_QUIRK_BROKEN_DMA |
    SDHCI_QUIRK_BROKEN_ADMA,
#elif defined(Transfer_Mode_SDMA)
    | SDHCI_QUIRK_FORCE_DMA |
    SDHCI_QUIRK_BROKEN_ADMA,
#elif defined(Transfer_Mode_ADMA)
    ,
#endif
    .quirks2 = SDHCI_QUIRK2_NO_1_8_V,
};


int iteSdcInitialize(int index, SD_CARD_INFO *card_info)
{
    int ret = 0;
    struct mmc_host *mmc;
    struct sdhci_host *host = ctxt.sdc[index];

    memset((void*)card_info, 0x0, sizeof(card_info));

    if (host) {
        mmc = host->mmc;
        goto get_info;
    }

    host = sdhci_alloc_host(NULL/*nothing*/, 0);
    if (!host) {
        printf(" sdc%d: sdhci_alloc_host() fail! \n", index);
        ret = -ENOMEM;
        goto out;
    }
    host->ioaddr = (index == 0) ? ITH_SD0_BASE : ITH_SD1_BASE;
    host->hw_name = (index == 0) ? "sdc0" : "sdc1";
    host->ops = ite_pdata.ops;
    host->quirks = ite_pdata.quirks;
    host->quirks2 = ite_pdata.quirks2;
    host->index = index;
    ctxt.sdc[index] = host;
    mmc = host->mmc;
    mmc->name = (index == 0) ? "mmc0" : "mmc1";

    ret = sdhci_add_host(host);
    if (ret) {
        sdhci_free_host(host);
        goto out;
    }

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
    ctxt.sdc[index] = NULL;
    check_result(ret);

    return ret;
}

int iteSdcTerminate(int index)
{
    struct sdhci_host *host = ctxt.sdc[index];
    int dead;

    if (host == NULL)
        return 0;

    dead = (sdhci_readl(host, SDHCI_INT_STATUS) == 0xffffffff);

    sdhci_remove_host(host, dead);
    sdhci_free_host(host);
    ctxt.sdc[index] = NULL;
    return 0;
}

struct mmc_host* mmc_get_host(int index)
{
    if (ctxt.sdc[index] == NULL)
        return NULL;
    else
        return ctxt.sdc[index]->mmc;
}

