
#include <linux/os.h>
#include <linux/err.h>
#include <linux/util.h>

#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/errorno.h>

#include "../core/debug.h"
#include "ite/itp.h"
#include "sdhci.h"

#define REMOVE_UNUSED_CODE
#define SD_IRQ_ENABLE

#if defined(WIN32)
#undef SD_IRQ_ENABLE
#define ithPrintf   printf
#endif


#define DRIVER_NAME "sdhci"


static void sdhci_irq(void* arg);
static void sdhci_thread_irq(void *arg1, uint32_t arg2);
static void sdhci_enable_preset_value(struct sdhci_host *host, bool enable);
static int sdhci_get_cd(struct mmc_host *mmc);
#if !defined(SD_IRQ_ENABLE)
static void sdhci_timeout_timer(unsigned long data);
#endif

static void sdhci_dumpregs(struct sdhci_host *host)
{
	pr_debug(DRIVER_NAME ": =========== REGISTER DUMP (%s)===========\n",
		mmc_hostname(host->mmc));

	pr_debug(DRIVER_NAME ": Sys addr: 0x%08x | Version:  0x%08x\n",
		sdhci_readl(host, SDHCI_DMA_ADDRESS),
		sdhci_readw(host, SDHCI_HOST_VERSION));
	pr_debug(DRIVER_NAME ": Blk size: 0x%08x | Blk cnt:  0x%08x\n",
		sdhci_readw(host, SDHCI_BLOCK_SIZE),
		sdhci_readw(host, SDHCI_BLOCK_COUNT));
	pr_debug(DRIVER_NAME ": Argument: 0x%08x | Trn mode: 0x%08x\n",
		sdhci_readl(host, SDHCI_ARGUMENT),
		sdhci_readw(host, SDHCI_TRANSFER_MODE));
	pr_debug(DRIVER_NAME ": Present:  0x%08x | Host ctl: 0x%08x\n",
		sdhci_readl(host, SDHCI_PRESENT_STATE),
		sdhci_readb(host, SDHCI_HOST_CONTROL));
	pr_debug(DRIVER_NAME ": Power:    0x%08x | Blk gap:  0x%08x\n",
		sdhci_readb(host, SDHCI_POWER_CONTROL),
		sdhci_readb(host, SDHCI_BLOCK_GAP_CONTROL));
	pr_debug(DRIVER_NAME ": Wake-up:  0x%08x | Clock:    0x%08x\n",
		sdhci_readb(host, SDHCI_WAKE_UP_CONTROL),
		sdhci_readw(host, SDHCI_CLOCK_CONTROL));
	pr_debug(DRIVER_NAME ": Timeout:  0x%08x | Int stat: 0x%08x\n",
		sdhci_readb(host, SDHCI_TIMEOUT_CONTROL),
		sdhci_readl(host, SDHCI_INT_STATUS));
	pr_debug(DRIVER_NAME ": Int enab: 0x%08x | Sig enab: 0x%08x\n",
		sdhci_readl(host, SDHCI_INT_ENABLE),
		sdhci_readl(host, SDHCI_SIGNAL_ENABLE));
	pr_debug(DRIVER_NAME ": AC12 err: 0x%08x | Slot int: 0x%08x\n",
		sdhci_readw(host, SDHCI_ACMD12_ERR),
		sdhci_readw(host, SDHCI_SLOT_INT_STATUS));
	pr_debug(DRIVER_NAME ": Caps:     0x%08x | Caps_1:   0x%08x\n",
		sdhci_readl(host, SDHCI_CAPABILITIES),
		sdhci_readl(host, SDHCI_CAPABILITIES_1));
	pr_debug(DRIVER_NAME ": Cmd:      0x%08x | Max curr: 0x%08x\n",
		sdhci_readw(host, SDHCI_COMMAND),
		sdhci_readl(host, SDHCI_MAX_CURRENT));
	pr_debug(DRIVER_NAME ": Host ctl2: 0x%08x\n",
		sdhci_readw(host, SDHCI_HOST_CONTROL2));
#if 0
	if (host->flags & SDHCI_USE_ADMA) {
		if (host->flags & SDHCI_USE_64_BIT_DMA)
			pr_debug(DRIVER_NAME ": ADMA Err: 0x%08x | ADMA Ptr: 0x%08x%08x\n",
				 readl(host->ioaddr + SDHCI_ADMA_ERROR),
				 readl(host->ioaddr + SDHCI_ADMA_ADDRESS_HI),
				 readl(host->ioaddr + SDHCI_ADMA_ADDRESS));
		else
			pr_debug(DRIVER_NAME ": ADMA Err: 0x%08x | ADMA Ptr: 0x%08x\n",
				 readl(host->ioaddr + SDHCI_ADMA_ERROR),
				 readl(host->ioaddr + SDHCI_ADMA_ADDRESS));
	}
#endif
	pr_debug(DRIVER_NAME ": ===========================================\n");
	pr_debug(DRIVER_NAME ": 0x%02x: 0x%08x | 0x%02x: 0x%08x\n", SDHCI_DMA_ADDRESS,
		sdhci_readl(host, SDHCI_DMA_ADDRESS),
		SDHCI_HOST_VERSION,
		sdhci_readw(host, SDHCI_HOST_VERSION));
	pr_debug(DRIVER_NAME ": 0x%02x: 0x%08x | 0x%02x: 0x%08x\n", SDHCI_BLOCK_SIZE, 
		sdhci_readw(host, SDHCI_BLOCK_SIZE),
		SDHCI_BLOCK_COUNT,
		sdhci_readw(host, SDHCI_BLOCK_COUNT));
	pr_debug(DRIVER_NAME ": 0x%02x: 0x%08x | 0x%02x: 0x%08x\n", SDHCI_ARGUMENT, 
		sdhci_readl(host, SDHCI_ARGUMENT),
		SDHCI_TRANSFER_MODE,
		sdhci_readw(host, SDHCI_TRANSFER_MODE));
	pr_debug(DRIVER_NAME ": 0x%02x: 0x%08x | 0x%02x: 0x%08x\n", SDHCI_PRESENT_STATE, 
		sdhci_readl(host, SDHCI_PRESENT_STATE),
		SDHCI_HOST_CONTROL,
		sdhci_readb(host, SDHCI_HOST_CONTROL));
	pr_debug(DRIVER_NAME ": 0x%02x: 0x%08x | 0x%02x: 0x%08x\n", SDHCI_POWER_CONTROL, 
		sdhci_readb(host, SDHCI_POWER_CONTROL),
		SDHCI_BLOCK_GAP_CONTROL,
		sdhci_readb(host, SDHCI_BLOCK_GAP_CONTROL));
	pr_debug(DRIVER_NAME ": 0x%02x: 0x%08x | 0x%02x: 0x%08x\n", SDHCI_WAKE_UP_CONTROL, 
		sdhci_readb(host, SDHCI_WAKE_UP_CONTROL),
		SDHCI_CLOCK_CONTROL,
		sdhci_readw(host, SDHCI_CLOCK_CONTROL));
	pr_debug(DRIVER_NAME ": 0x%02x: 0x%08x | 0x%02x: 0x%08x\n", SDHCI_TIMEOUT_CONTROL, 
		sdhci_readb(host, SDHCI_TIMEOUT_CONTROL),
		SDHCI_INT_STATUS,
		sdhci_readl(host, SDHCI_INT_STATUS));
	pr_debug(DRIVER_NAME ": 0x%02x: 0x%08x | 0x%02x: 0x%08x\n", SDHCI_INT_ENABLE, 
		sdhci_readl(host, SDHCI_INT_ENABLE),
		SDHCI_SIGNAL_ENABLE,
		sdhci_readl(host, SDHCI_SIGNAL_ENABLE));
	pr_debug(DRIVER_NAME ": 0x%02x: 0x%08x | 0x%02x: 0x%08x\n", SDHCI_ACMD12_ERR, 
		sdhci_readw(host, SDHCI_ACMD12_ERR),
		SDHCI_SLOT_INT_STATUS,
		sdhci_readw(host, SDHCI_SLOT_INT_STATUS));
	pr_debug(DRIVER_NAME ": 0x%02x: 0x%08x | 0x%02x: 0x%08x\n", SDHCI_CAPABILITIES, 
		sdhci_readl(host, SDHCI_CAPABILITIES),
		SDHCI_CAPABILITIES_1,
		sdhci_readl(host, SDHCI_CAPABILITIES_1));
	pr_debug(DRIVER_NAME ": 0x%02x: 0x%08x | 0x%02x: 0x%08x\n", SDHCI_COMMAND, 
		sdhci_readw(host, SDHCI_COMMAND),
		SDHCI_MAX_CURRENT,
		sdhci_readl(host, SDHCI_MAX_CURRENT));
	pr_debug(DRIVER_NAME ": 0x%02x: 0x%08x\n", SDHCI_HOST_CONTROL2,
		sdhci_readw(host, SDHCI_HOST_CONTROL2));
	pr_debug(DRIVER_NAME ": ===========================================\n");
	while (1);
}

/*****************************************************************************\
 *                                                                           *
 * Low level functions                                                       *
 *                                                                           *
\*****************************************************************************/

static void sdhci_runtime_pm_bus_on(struct sdhci_host *host)
{
	if (host->bus_on)
		return;
	host->bus_on = true;
}

static void sdhci_runtime_pm_bus_off(struct sdhci_host *host)
{
	if (!host->bus_on)
		return;
	host->bus_on = false;
}

void sdhci_reset(struct sdhci_host *host, u8 mask)
{
	unsigned long timeout;

	sdhci_writeb(host, mask, SDHCI_SOFTWARE_RESET);

	if (mask & SDHCI_RESET_ALL) {
		host->clock = 0;
		/* Reset-all turns off SD Bus Power */
		#if !defined(REMOVE_UNUSED_CODE)
		if (host->quirks2 & SDHCI_QUIRK2_CARD_ON_NEEDS_BUS_ON)
			sdhci_runtime_pm_bus_off(host);
		#endif
	}

	/* Wait max 100 ms */
	timeout = 100;

	/* hw clears the bit when it's done */
	while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & mask) {
		if (timeout == 0) {
			pr_err("%s: Reset 0x%x never completed.\n",
				mmc_hostname(host->mmc), (int)mask);
			sdhci_dumpregs(host);
			return;
		}
		timeout--;
		mdelay(1);
	}
}
EXPORT_SYMBOL_GPL(sdhci_reset);

static void sdhci_do_reset(struct sdhci_host *host, u8 mask)
{
	if (host->quirks & SDHCI_QUIRK_NO_CARD_NO_RESET) {
		if (!sdhci_get_cd(host->mmc))
			return;
	}

	host->ops->reset(host, mask);

	if (mask & SDHCI_RESET_ALL) {
		if (host->flags & (SDHCI_USE_SDMA | SDHCI_USE_ADMA)) {
			if (host->ops->enable_dma)
				host->ops->enable_dma(host);
		}

		/* Resetting the controller clears many */
		host->preset_enabled = false;
	}
}

static void sdhci_set_ios(struct mmc_host *mmc, struct mmc_ios *ios);

static void sdhci_init(struct sdhci_host *host, int soft)
{
	if (soft)
		sdhci_do_reset(host, SDHCI_RESET_CMD|SDHCI_RESET_DATA);
	else
		sdhci_do_reset(host, SDHCI_RESET_ALL);

	host->ier = SDHCI_INT_BUS_POWER | SDHCI_INT_DATA_END_BIT |
		    SDHCI_INT_DATA_CRC | SDHCI_INT_DATA_TIMEOUT |
		    SDHCI_INT_INDEX | SDHCI_INT_END_BIT | SDHCI_INT_CRC |
		    SDHCI_INT_TIMEOUT | SDHCI_INT_DATA_END |
		    SDHCI_INT_RESPONSE;

	sdhci_writel(host, host->ier, SDHCI_INT_ENABLE);
	sdhci_writel(host, host->ier, SDHCI_SIGNAL_ENABLE);

	if (soft) {
		/* force clock reconfiguration */
		host->clock = 0;
		sdhci_set_ios(host->mmc, &host->mmc->ios);
	}
}

static void sdhci_reinit(struct sdhci_host *host)
{
	sdhci_init(host, 0);
}

/*****************************************************************************\
 *                                                                           *
 * Core functions                                                            *
 *                                                                           *
\*****************************************************************************/

static void sdhci_read_block_pio(struct sdhci_host *host)
{
	unsigned long flags;
	size_t blksize, len, chunk;
	u32 scratch;
	u8 *buf;

	DBG("PIO reading\n");

	blksize = host->data->blksz;
	chunk = 0;

	local_irq_save(flags);

	while (blksize) {
		len = blksize;

		blksize -= len;

		buf = host->data->buf;

		while (len) {
			if (chunk == 0) {
				scratch = sdhci_readl(host, SDHCI_BUFFER);
				chunk = 4;
			}

			*buf = scratch & 0xFF;

			buf++;
			scratch >>= 8;
			chunk--;
			len--;
		}
	}

	local_irq_restore(flags);
}

static void sdhci_write_block_pio(struct sdhci_host *host)
{
	unsigned long flags;
	size_t blksize, len, chunk;
	u32 scratch;
	u8 *buf;

	DBG("PIO writing\n");

	blksize = host->data->blksz;
	chunk = 0;
	scratch = 0;

	local_irq_save(flags);

	while (blksize) {
		len = blksize;
		blksize -= len;

		buf = host->data->buf;

		while (len) {
			scratch |= (u32)*buf << (chunk * 8);

			buf++;
			chunk++;
			len--;

			if ((chunk == 4) || ((len == 0) && (blksize == 0))) {
				sdhci_writel(host, scratch, SDHCI_BUFFER);
				chunk = 0;
				scratch = 0;
			}
		}
	}

	local_irq_restore(flags);
}

static void sdhci_transfer_pio(struct sdhci_host *host)
{
	u32 mask;

	BUG_ON(!host->data);

	if (host->blocks == 0)
		return;

	if (host->data->flags & MMC_DATA_READ)
		mask = SDHCI_DATA_AVAILABLE;
	else
		mask = SDHCI_SPACE_AVAILABLE;

	while (sdhci_readl(host, SDHCI_PRESENT_STATE) & mask) {
		if (host->data->flags & MMC_DATA_READ)
			sdhci_read_block_pio(host);
		else
			sdhci_write_block_pio(host);

		host->blocks--;
		if (host->blocks == 0)
			break;
	}

	DBG("PIO transfer complete.\n");
}

static int sdhci_pre_dma_transfer(struct sdhci_host *host,
				  struct mmc_data *data, int cookie)
{
	int sg_count = 1;
    u32 total_size = data->blksz * data->blocks;
    u32 buf_addr = (u32)data->buf;

    #if defined(WIN32)
    data->tmp_buf = (void*)itpVmemAlloc(total_size);
    if (data->flags & MMC_DATA_WRITE)
        ithWriteVram((uint32_t)data->tmp_buf, data->buf, total_size);
    #endif

    #if defined(CFG_CPU_WB)
    ithFlushDCacheRange((void*)data->buf, total_size);
    #endif
	ithFlushMemBuffer();

	/*
	 * If the data buffers are already mapped, return the previous
	 * dma_map_sg() result.
	 */
	//if (data->host_cookie == COOKIE_PRE_MAPPED)
	//	return data->sg_count;

	//data->sg_count = sg_count;
	data->host_cookie = cookie;

	return sg_count;
}

static void sdhci_adma_table_post(struct sdhci_host *host,
	struct mmc_data *data)
{
    return;
}

static u8 sdhci_calc_timeout(struct sdhci_host *host, struct mmc_command *cmd)
{
#if !defined(REMOVE_UNUSED_CODE)
    u8 count;
#endif
	struct mmc_data *data = cmd->data;
#if !defined(REMOVE_UNUSED_CODE)
    unsigned target_timeout, current_timeout;
#endif

	/*
	 * If the host controller provides us with an incorrect timeout
	 * value, just skip the check and use 0xE.  The hardware may take
	 * longer to time out, but that's much better than having a too-short
	 * timeout value.
	 */
	if (host->quirks & SDHCI_QUIRK_BROKEN_TIMEOUT_VAL)
		return 0xE;
	
#if !defined(REMOVE_UNUSED_CODE)
	/* Unspecified timeout, assume max */
	if (!data && !cmd->busy_timeout)
		return 0xE;

	/* timeout in us */
	if (!data)
		target_timeout = cmd->busy_timeout * 1000;
	else {
		target_timeout = DIV_ROUND_UP(data->timeout_ns, 1000);
		if (host->clock && data->timeout_clks) {
			unsigned long long val;

			/*
			 * data->timeout_clks is in units of clock cycles.
			 * host->clock is in Hz.  target_timeout is in us.
			 * Hence, us = 1000000 * cycles / Hz.  Round up.
			 */
			val = 1000000 * data->timeout_clks;
			if (do_div(val, host->clock))
				target_timeout++;
			target_timeout += val;
		}
	}

	/*
	 * Figure out needed cycles.
	 * We do this in steps in order to fit inside a 32 bit int.
	 * The first step is the minimum timeout, which will have a
	 * minimum resolution of 6 bits:
	 * (1) 2^13*1000 > 2^22,
	 * (2) host->timeout_clk < 2^16
	 *     =>
	 *     (1) / (2) > 2^6
	 */
	count = 0;
	current_timeout = (1 << 13) * 1000 / host->timeout_clk;
	while (current_timeout < target_timeout) {
		count++;
		current_timeout <<= 1;
		if (count >= 0xF)
			break;
	}

	if (count >= 0xF) {
		DBG("%s: Too large timeout 0x%x requested for CMD%d!\n",
		    mmc_hostname(host->mmc), count, cmd->opcode);
		count = 0xE;
	}

	return count;
#endif	
    return 0xE;
}

static void sdhci_set_transfer_irqs(struct sdhci_host *host)
{
	u32 pio_irqs = SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL;
	u32 dma_irqs = SDHCI_INT_DMA_END | SDHCI_INT_ADMA_ERROR;

	if (host->flags & SDHCI_REQ_USE_DMA)
		host->ier = (host->ier & ~pio_irqs) | dma_irqs;
	else
		host->ier = (host->ier & ~dma_irqs) | pio_irqs;

	sdhci_writel(host, host->ier, SDHCI_INT_ENABLE);
	sdhci_writel(host, host->ier, SDHCI_SIGNAL_ENABLE);
}

static void sdhci_set_timeout(struct sdhci_host *host, struct mmc_command *cmd)
{
	u8 count;

	if (host->ops->set_timeout) {
		host->ops->set_timeout(host, cmd);
	} else {
		count = sdhci_calc_timeout(host, cmd);
		sdhci_writeb(host, count, SDHCI_TIMEOUT_CONTROL);
	}
}

#if defined(WIN32)
#define dma_address(data)   (u32)(data->tmp_buf)
#else
#define dma_address(data)   (u32)(data->buf)
#endif

static void sdhci_prepare_data(struct sdhci_host *host, struct mmc_command *cmd)
{
	u8 ctrl;
	struct mmc_data *data = cmd->data;

	WARN_ON(host->data);

	if (data || (cmd->flags & MMC_RSP_BUSY))
		sdhci_set_timeout(host, cmd);

	if (!data)
		return;

	/* Sanity checks */
	BUG_ON(data->blksz * data->blocks > 524288);
	BUG_ON(data->blksz > host->mmc->max_blk_size);
	BUG_ON(data->blocks > 65535);

	host->data = data;
	host->data_early = 0;
	host->data->bytes_xfered = 0;

	if (host->flags & (SDHCI_USE_SDMA | SDHCI_USE_ADMA))
		host->flags |= SDHCI_REQ_USE_DMA;

	if (host->flags & SDHCI_REQ_USE_DMA) {
		int sg_cnt = sdhci_pre_dma_transfer(host, data, COOKIE_MAPPED);

		if (sg_cnt <= 0) {
			/*
			 * This only happens when someone fed
			 * us an invalid request.
			 */
			WARN_ON(1);
			host->flags &= ~SDHCI_REQ_USE_DMA;
#if !defined(REMOVE_UNUSED_CODE)
		} else if (host->flags & SDHCI_USE_ADMA) {
			sdhci_adma_table_pre(host, data, sg_cnt);

			sdhci_writel(host, host->adma_addr, SDHCI_ADMA_ADDRESS);
			if (host->flags & SDHCI_USE_64_BIT_DMA)
				sdhci_writel(host,
					     (u64)host->adma_addr >> 32,
					     SDHCI_ADMA_ADDRESS_HI);
#endif
		} else {
			WARN_ON(sg_cnt != 1);
			sdhci_writel(host, dma_address(data),
				SDHCI_DMA_ADDRESS);
		}
	}

	/*
	 * Always adjust the DMA selection as some controllers
	 * (e.g. JMicron) can't do PIO properly when the selection
	 * is ADMA.
	 */
	if (host->version >= SDHCI_SPEC_200) {
		ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
		ctrl &= ~SDHCI_CTRL_DMA_MASK;
		if ((host->flags & SDHCI_REQ_USE_DMA) &&
			(host->flags & SDHCI_USE_ADMA)) {
			if (host->flags & SDHCI_USE_64_BIT_DMA)
				ctrl |= SDHCI_CTRL_ADMA64;
			else
				ctrl |= SDHCI_CTRL_ADMA32;
		} else {
			ctrl |= SDHCI_CTRL_SDMA;
		}
		sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
	}

	if (!(host->flags & SDHCI_REQ_USE_DMA)) {
		host->blocks = data->blocks;
	}

	sdhci_set_transfer_irqs(host);

	/* Set the DMA boundary value and block size */
	sdhci_writew(host, SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
		data->blksz), SDHCI_BLOCK_SIZE);
	sdhci_writew(host, data->blocks, SDHCI_BLOCK_COUNT);
}

static void sdhci_set_transfer_mode(struct sdhci_host *host,
	struct mmc_command *cmd)
{
	u16 mode = 0;
	struct mmc_data *data = cmd->data;

	if (data == NULL) {
		if (host->quirks2 &
			SDHCI_QUIRK2_CLEAR_TRANSFERMODE_REG_BEFORE_CMD) {
			sdhci_writew(host, 0x0, SDHCI_TRANSFER_MODE);
		} else {
		/* clear Auto CMD settings for no data CMDs */
			mode = sdhci_readw(host, SDHCI_TRANSFER_MODE);
			sdhci_writew(host, mode & ~(SDHCI_TRNS_AUTO_CMD12 |
				SDHCI_TRNS_AUTO_CMD23), SDHCI_TRANSFER_MODE);
		}
		return;
	}

	WARN_ON(!host->data);

	if (!(host->quirks2 & SDHCI_QUIRK2_SUPPORT_SINGLE))
		mode = SDHCI_TRNS_BLK_CNT_EN;

	if (mmc_op_multi(cmd->opcode) || data->blocks > 1) {
		mode = SDHCI_TRNS_BLK_CNT_EN | SDHCI_TRNS_MULTI;
		/*
		 * If we are sending CMD23, CMD12 never gets sent
		 * on successful completion (so no Auto-CMD12).
		 */
		if (!host->mrq->sbc && (host->flags & SDHCI_AUTO_CMD12) &&
		    (cmd->opcode != SD_IO_RW_EXTENDED))
			mode |= SDHCI_TRNS_AUTO_CMD12;
		else if (host->mrq->sbc && (host->flags & SDHCI_AUTO_CMD23)) {
			mode |= SDHCI_TRNS_AUTO_CMD23;
			sdhci_writel(host, host->mrq->sbc->arg, SDHCI_ARGUMENT2);
		}
	}

	if (data->flags & MMC_DATA_READ)
		mode |= SDHCI_TRNS_READ;
	if (host->flags & SDHCI_REQ_USE_DMA)
		mode |= SDHCI_TRNS_DMA;

	sdhci_writew(host, mode, SDHCI_TRANSFER_MODE);
}

static void sdhci_stop_cmd(void *arg1, uint32_t arg2)
{
	struct mmc_request *mrq = (struct mmc_request *)arg1;

	sdhci_send_command(mmc_priv(mrq->host), mrq->stop);
}

static void sdhci_finish_data(struct sdhci_host *host)
{
	struct mmc_data *data;

	BUG_ON(!host->data);

	data = host->data;
	host->data = NULL;

	if ((host->flags & (SDHCI_REQ_USE_DMA | SDHCI_USE_ADMA)) ==
	    (SDHCI_REQ_USE_DMA | SDHCI_USE_ADMA))
		sdhci_adma_table_post(host, data);

	/*
	 * The specification states that the block count register must
	 * be updated, but it does not specify at what point in the
	 * data flow. That makes the register entirely useless to read
	 * back so we have to assume that nothing made it to the card
	 * in the event of an error.
	 */
	if (data->error)
		data->bytes_xfered = 0;
	else
		data->bytes_xfered = data->blksz * data->blocks;

	/*
	 * Need to send CMD12 if -
	 * a) open-ended multiblock transfer (no CMD23)
	 * b) error in multiblock transfer
	 */
	if (data->stop &&
	    (data->error ||
	     !host->mrq->sbc)) {

		/*
		 * The controller needs a reset of internal state machines
		 * upon error conditions.
		 */
		if (data->error) {
			sdhci_do_reset(host, SDHCI_RESET_CMD);
			sdhci_do_reset(host, SDHCI_RESET_DATA);
		}

		if (in_interrupt())
			itpPendFunctionCallFromISR(sdhci_stop_cmd, host->mrq, 0);
	    else
			sdhci_send_command(host, data->stop);
	} else
		tasklet_schedule(&host->finish_tasklet);
}

void sdhci_send_command(struct sdhci_host *host, struct mmc_command *cmd)
{
	int flags;
	u32 mask;
	unsigned long timeout;

	WARN_ON(host->cmd);

	/* Initially, a command has no error */
	cmd->error = 0;

	/* Wait max 10 ms */
	timeout = 10;

	mask = SDHCI_CMD_INHIBIT;
	if ((cmd->data != NULL) || (cmd->flags & MMC_RSP_BUSY))
		mask |= SDHCI_DATA_INHIBIT;

	/* We shouldn't wait for data inihibit for stop commands, even
	   though they might use busy signaling */
	if (host->mrq->data && (cmd == host->mrq->data->stop))
		mask &= ~SDHCI_DATA_INHIBIT;

	while (sdhci_readl(host, SDHCI_PRESENT_STATE) & mask) {
		if (timeout == 0) {
			pr_err("%s: Controller never released inhibit bit(s).\n",
			       mmc_hostname(host->mmc));
			sdhci_dumpregs(host);
			cmd->error = -EIO;
			tasklet_schedule(&host->finish_tasklet);
			return;
		}
		timeout--;
		mdelay(1);
	}

	timeout = jiffies;
	if (!cmd->data && cmd->busy_timeout > 9000)
		timeout += DIV_ROUND_UP(cmd->busy_timeout, 1000) * HZ + HZ;
	else
		timeout += 10 * HZ;
#if defined(SD_IRQ_ENABLE)
	mod_timer(&host->timer, timeout);
#endif

	host->cmd = cmd;
	host->busy_handle = 0;

	sdhci_prepare_data(host, cmd);

	sdhci_writel(host, cmd->arg, SDHCI_ARGUMENT);

	sdhci_set_transfer_mode(host, cmd);

	if ((cmd->flags & MMC_RSP_136) && (cmd->flags & MMC_RSP_BUSY)) {
		pr_err("%s: Unsupported response type!\n",
			mmc_hostname(host->mmc));
		cmd->error = -EINVAL;
		tasklet_schedule(&host->finish_tasklet);
		return;
	}

	if (!(cmd->flags & MMC_RSP_PRESENT))
		flags = SDHCI_CMD_RESP_NONE;
	else if (cmd->flags & MMC_RSP_136)
		flags = SDHCI_CMD_RESP_LONG;
	else if (cmd->flags & MMC_RSP_BUSY)
		flags = SDHCI_CMD_RESP_SHORT_BUSY;
	else
		flags = SDHCI_CMD_RESP_SHORT;

	if (cmd->flags & MMC_RSP_CRC)
		flags |= SDHCI_CMD_CRC;
	if (cmd->flags & MMC_RSP_OPCODE)
		flags |= SDHCI_CMD_INDEX;

	/* CMD19 is special in that the Data Present Select should be set */
	if (cmd->data || cmd->opcode == MMC_SEND_TUNING_BLOCK ||
	    cmd->opcode == MMC_SEND_TUNING_BLOCK_HS200)
		flags |= SDHCI_CMD_DATA;

	sdhci_writew(host, SDHCI_MAKE_CMD(cmd->opcode, flags), SDHCI_COMMAND);
}
EXPORT_SYMBOL_GPL(sdhci_send_command);

static void sdhci_finish_command(struct sdhci_host *host)
{
	int i;

	BUG_ON(host->cmd == NULL);

	if (host->cmd->flags & MMC_RSP_PRESENT) {
		if (host->cmd->flags & MMC_RSP_136) {
			/* CRC is stripped so we need to do some shifting. */
			for (i = 0;i < 4;i++) {
				host->cmd->resp[i] = sdhci_readl(host,
					SDHCI_RESPONSE + (3-i)*4) << 8;
				if (i != 3)
					host->cmd->resp[i] |=
						sdhci_readb(host,
						SDHCI_RESPONSE + (3-i)*4-1);
			}
		} else {
			host->cmd->resp[0] = sdhci_readl(host, SDHCI_RESPONSE);
		}
	}

	/* Finished CMD23, now send actual command. */
	if (host->cmd == host->mrq->sbc) {
		host->cmd = NULL;
		sdhci_send_command(host, host->mrq->cmd);
	} else {

		/* Processed actual command. */
		if (host->data && host->data_early)
			sdhci_finish_data(host);

		if (!host->cmd->data)
			tasklet_schedule(&host->finish_tasklet);

		host->cmd = NULL;
	}
}

static u16 sdhci_get_preset_value(struct sdhci_host *host)
{
	u16 preset = 0;

	switch (host->timing) {
	case MMC_TIMING_UHS_SDR12:
		preset = sdhci_readw(host, SDHCI_PRESET_FOR_SDR12);
		break;
	case MMC_TIMING_UHS_SDR25:
		preset = sdhci_readw(host, SDHCI_PRESET_FOR_SDR25);
		break;
	case MMC_TIMING_UHS_SDR50:
		preset = sdhci_readw(host, SDHCI_PRESET_FOR_SDR50);
		break;
	case MMC_TIMING_UHS_SDR104:
	case MMC_TIMING_MMC_HS200:
		preset = sdhci_readw(host, SDHCI_PRESET_FOR_SDR104);
		break;
	case MMC_TIMING_UHS_DDR50:
	case MMC_TIMING_MMC_DDR52:
		preset = sdhci_readw(host, SDHCI_PRESET_FOR_DDR50);
		break;
	case MMC_TIMING_MMC_HS400:
		preset = sdhci_readw(host, SDHCI_PRESET_FOR_HS400);
		break;
	default:
		pr_warn("%s: Invalid UHS-I mode selected\n",
			mmc_hostname(host->mmc));
		preset = sdhci_readw(host, SDHCI_PRESET_FOR_SDR12);
		break;
	}
	return preset;
}

u16 sdhci_calc_clk(struct sdhci_host *host, unsigned int clock,
		   unsigned int *actual_clock)
{
	int div = 0; /* Initialized for compiler warning */
	int real_div = div, clk_mul = 1;
	u16 clk = 0;
	bool switch_base_clk = false;

#if 0
	if (clock == 400000)  // Irene test
		clock = 260000;
#endif

	if (host->version >= SDHCI_SPEC_300) {
		if (host->preset_enabled) {
			u16 pre_val;

			clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
			pre_val = sdhci_get_preset_value(host);
			div = (pre_val & SDHCI_PRESET_SDCLK_FREQ_MASK)
				>> SDHCI_PRESET_SDCLK_FREQ_SHIFT;
			if (host->clk_mul &&
				(pre_val & SDHCI_PRESET_CLKGEN_SEL_MASK)) {
				clk = SDHCI_PROG_CLOCK_MODE;
				real_div = div + 1;
				clk_mul = host->clk_mul;
			} else {
				real_div = max_t(int, 1, div << 1);
			}
			goto clock_set;
		}

		/*
		 * Check if the Host Controller supports Programmable Clock
		 * Mode.
		 */
		if (host->clk_mul) {
			for (div = 1; div <= 1024; div++) {
				if ((host->max_clk * host->clk_mul / div)
					<= clock)
					break;
			}
			if ((host->max_clk * host->clk_mul / div) <= clock) {
				/*
				 * Set Programmable Clock Mode in the Clock
				 * Control register.
				 */
				clk = SDHCI_PROG_CLOCK_MODE;
				real_div = div;
				clk_mul = host->clk_mul;
				div--;
			} else {
				/*
				 * Divisor can be too small to reach clock
				 * speed requirement. Then use the base clock.
				 */
				switch_base_clk = true;
			}
		}

		if (!host->clk_mul || switch_base_clk) {
			/* Version 3.00 divisors must be a multiple of 2. */
			if (host->max_clk <= clock)
				div = 1;
			else {
				for (div = 2; div < SDHCI_MAX_DIV_SPEC_300;
				     div += 2) {
					if ((host->max_clk / div) <= clock)
						break;
				}
			}
			real_div = div;
			div >>= 1;
			if ((host->quirks2 & SDHCI_QUIRK2_CLOCK_DIV_ZERO_BROKEN)
				&& !div && host->max_clk <= 25000000)
				div = 1;
		}
	} else {
		/* Version 2.00 divisors must be a power of 2. */
		for (div = 1; div < SDHCI_MAX_DIV_SPEC_200; div *= 2) {
			if ((host->max_clk / div) <= clock)
				break;
		}
		real_div = div;
		div >>= 1;
	}

	//printf("div=%d \n", div);
clock_set:
	if (real_div)
		*actual_clock = (host->max_clk * clk_mul) / real_div;
	clk |= (div & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT;
	clk |= ((div & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN)
		<< SDHCI_DIVIDER_HI_SHIFT;

	return clk;
}
EXPORT_SYMBOL_GPL(sdhci_calc_clk);

void sdhci_set_clock(struct sdhci_host *host, unsigned int clock)
{
	u16 clk;
	unsigned long timeout;

	host->mmc->actual_clock = 0;

	sdhci_writew(host, 0, SDHCI_CLOCK_CONTROL);

	if (clock == 0)
		return;

	clk = sdhci_calc_clk(host, clock, &host->mmc->actual_clock);

	clk |= SDHCI_CLOCK_INT_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

	/* Wait max 20 ms */
	timeout = 20;
	while (!((clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL))
		& SDHCI_CLOCK_INT_STABLE)) {
		if (timeout == 0) {
			pr_err("%s: Internal clock never stabilised.\n",
			       mmc_hostname(host->mmc));
			sdhci_dumpregs(host);
			return;
		}
		timeout--;
		mdelay(1);
	}

	clk |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);
}
EXPORT_SYMBOL_GPL(sdhci_set_clock);

static void sdhci_set_power_reg(struct sdhci_host *host, unsigned char mode,
				unsigned short vdd)
{
	struct mmc_host *mmc = host->mmc;

	spin_unlock_irq(&host->lock);
	mmc_regulator_set_ocr(mmc, mmc->supply.vmmc, vdd);
	spin_lock_irq(&host->lock);

	if (mode != MMC_POWER_OFF)
		sdhci_writeb(host, SDHCI_POWER_ON, SDHCI_POWER_CONTROL);
	else
		sdhci_writeb(host, 0, SDHCI_POWER_CONTROL);
}

void sdhci_set_power(struct sdhci_host *host, unsigned char mode,
		     unsigned short vdd)
{
	u8 pwr = 0;

	if (mode != MMC_POWER_OFF) {
		switch (1 << vdd) {
		case MMC_VDD_165_195:
			pwr = SDHCI_POWER_180;
			break;
		case MMC_VDD_29_30:
		case MMC_VDD_30_31:
			pwr = SDHCI_POWER_300;
			break;
		case MMC_VDD_32_33:
		case MMC_VDD_33_34:
			pwr = SDHCI_POWER_330;
			break;
		default:
            pr_warn("%s: Invalid vdd %#x\n",
			     mmc_hostname(host->mmc), vdd);
			break;
		}
	}

	if (host->pwr == pwr)
		return;

	host->pwr = pwr;

	if (pwr == 0) {
		sdhci_writeb(host, 0, SDHCI_POWER_CONTROL);
		if (host->quirks2 & SDHCI_QUIRK2_CARD_ON_NEEDS_BUS_ON)
			sdhci_runtime_pm_bus_off(host);
	} else {
		/*
		 * Spec says that we should clear the power reg before setting
		 * a new value. Some controllers don't seem to like this though.
		 */
		if (!(host->quirks & SDHCI_QUIRK_SINGLE_POWER_WRITE))
			sdhci_writeb(host, 0, SDHCI_POWER_CONTROL);

		/*
		 * At least the Marvell CaFe chip gets confused if we set the
		 * voltage and set turn on power at the same time, so set the
		 * voltage first.
		 */
		if (host->quirks & SDHCI_QUIRK_NO_SIMULT_VDD_AND_POWER)
			sdhci_writeb(host, pwr, SDHCI_POWER_CONTROL);

		pwr |= SDHCI_POWER_ON;

		sdhci_writeb(host, pwr, SDHCI_POWER_CONTROL);

		if (host->quirks2 & SDHCI_QUIRK2_CARD_ON_NEEDS_BUS_ON)
			sdhci_runtime_pm_bus_on(host);

		/*
		 * Some controllers need an extra 10ms delay of 10ms before
		 * they can apply clock after applying power
		 */
		if (host->quirks & SDHCI_QUIRK_DELAY_AFTER_POWER)
			mdelay(10);
	}
}
EXPORT_SYMBOL_GPL(sdhci_set_power);

static void __sdhci_set_power(struct sdhci_host *host, unsigned char mode,
			      unsigned short vdd)
{
	struct mmc_host *mmc = host->mmc;

	if (host->ops->set_power)
		host->ops->set_power(host, mode, vdd);
	else if (!IS_ERR(mmc->supply.vmmc))
		sdhci_set_power_reg(host, mode, vdd);
	else
		sdhci_set_power(host, mode, vdd);
}

/*****************************************************************************\
 *                                                                           *
 * MMC callbacks                                                             *
 *                                                                           *
\*****************************************************************************/

static void sdhci_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct sdhci_host *host;
	int present;
	unsigned long flags;

	host = mmc_priv(mmc);

	/* Firstly check card presence */
	present = mmc->ops->get_cd(mmc);

	spin_lock_irqsave(&host->lock, flags);

	WARN_ON(host->mrq != NULL);

	/*
	 * Ensure we don't send the STOP for non-SET_BLOCK_COUNTED
	 * requests if Auto-CMD12 is enabled.
	 */
	if (!mrq->sbc && (host->flags & SDHCI_AUTO_CMD12)) {
		if (mrq->stop) {
			mrq->data->stop = NULL;
			mrq->stop = NULL;
		}
	}

	host->mrq = mrq;
	mrq->host = mmc;

	if (!present || host->flags & SDHCI_DEVICE_DEAD) {
		host->mrq->cmd->error = -ENOMEDIUM;
		tasklet_schedule(&host->finish_tasklet);
	} else {
		if (mrq->sbc && !(host->flags & SDHCI_AUTO_CMD23))
			sdhci_send_command(host, mrq->sbc);
		else
			sdhci_send_command(host, mrq->cmd);
	}

	mmiowb();
	spin_unlock_irqrestore(&host->lock, flags);

	#if !defined(SD_IRQ_ENABLE) // TODO
    {
	    u32 timeout = 5 * 1000;

	    do {
#if 0//defined(WIN32)
			if (mrq->cmd->opcode == 25)
				usleep(200 * 1000);
#endif
            sdhci_irq(host);
            if ((host->mrq == NULL) && (host->cmd == NULL) && (host->data == NULL))
                break;
            usleep(1000);
		} while(timeout--);

		if (timeout == 0) {
			printf(" %s: No IRQ timeout! \n", mmc_hostname(host->mmc));
			while (1);
			sdhci_timeout_timer(host);
		}
    }
	#endif
}

void sdhci_set_bus_width(struct sdhci_host *host, int width)
{
	u8 ctrl;

	ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
	if (width == MMC_BUS_WIDTH_8) {
		ctrl &= ~SDHCI_CTRL_4BITBUS;
		if (host->version >= SDHCI_SPEC_300)
			ctrl |= SDHCI_CTRL_8BITBUS;
	} else {
		if (host->version >= SDHCI_SPEC_300)
			ctrl &= ~SDHCI_CTRL_8BITBUS;
		if (width == MMC_BUS_WIDTH_4)
			ctrl |= SDHCI_CTRL_4BITBUS;
		else
			ctrl &= ~SDHCI_CTRL_4BITBUS;
	}
	sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
}
EXPORT_SYMBOL_GPL(sdhci_set_bus_width);

void sdhci_set_uhs_signaling(struct sdhci_host *host, unsigned timing)
{
	u16 ctrl_2;

	ctrl_2 = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	/* Select Bus Speed Mode for host */
	ctrl_2 &= ~SDHCI_CTRL_UHS_MASK;
	if ((timing == MMC_TIMING_MMC_HS200) ||
	    (timing == MMC_TIMING_UHS_SDR104))
		ctrl_2 |= SDHCI_CTRL_UHS_SDR104;
	else if (timing == MMC_TIMING_UHS_SDR12)
		ctrl_2 |= SDHCI_CTRL_UHS_SDR12;
	else if (timing == MMC_TIMING_UHS_SDR25)
		ctrl_2 |= SDHCI_CTRL_UHS_SDR25;
	else if (timing == MMC_TIMING_UHS_SDR50)
		ctrl_2 |= SDHCI_CTRL_UHS_SDR50;
	else if ((timing == MMC_TIMING_UHS_DDR50) ||
		 (timing == MMC_TIMING_MMC_DDR52))
		ctrl_2 |= SDHCI_CTRL_UHS_DDR50;
	else if (timing == MMC_TIMING_MMC_HS400)
		ctrl_2 |= SDHCI_CTRL_HS400; /* Non-standard */
	sdhci_writew(host, ctrl_2, SDHCI_HOST_CONTROL2);
}
EXPORT_SYMBOL_GPL(sdhci_set_uhs_signaling);

static void sdhci_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct sdhci_host *host = mmc_priv(mmc);
	unsigned long flags;
	u8 ctrl;

	spin_lock_irqsave(&host->lock, flags);

	if (host->flags & SDHCI_DEVICE_DEAD) {
		spin_unlock_irqrestore(&host->lock, flags);
		if (!IS_ERR(mmc->supply.vmmc) &&
		    ios->power_mode == MMC_POWER_OFF)
			mmc_regulator_set_ocr(mmc, mmc->supply.vmmc, 0);
		return;
	}

	/*
	 * Reset the chip on each power off.
	 * Should clear out any weird states.
	 */
	if (ios->power_mode == MMC_POWER_OFF) {
		sdhci_writel(host, 0, SDHCI_SIGNAL_ENABLE);
		sdhci_reinit(host);
	}

	if (host->version >= SDHCI_SPEC_300 &&
		(ios->power_mode == MMC_POWER_UP) &&
		!(host->quirks2 & SDHCI_QUIRK2_PRESET_VALUE_BROKEN))
		sdhci_enable_preset_value(host, false);

	if (!ios->clock || ios->clock != host->clock) {
		host->ops->set_clock(host, ios->clock);
		host->clock = ios->clock;

		if (host->quirks & SDHCI_QUIRK_DATA_TIMEOUT_USES_SDCLK &&
		    host->clock) {
			host->timeout_clk = host->mmc->actual_clock ?
						host->mmc->actual_clock / 1000 :
						host->clock / 1000;
			host->mmc->max_busy_timeout =
				host->ops->get_max_timeout_count ?
				host->ops->get_max_timeout_count(host) :
				1 << 27;
			host->mmc->max_busy_timeout /= host->timeout_clk;
		}
	}

	__sdhci_set_power(host, ios->power_mode, ios->vdd);

	if (host->ops->platform_send_init_74_clocks)
		host->ops->platform_send_init_74_clocks(host, ios->power_mode);

	host->ops->set_bus_width(host, ios->bus_width);

	ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);

	if ((ios->timing == MMC_TIMING_SD_HS ||
	     ios->timing == MMC_TIMING_MMC_HS)
	    && !(host->quirks & SDHCI_QUIRK_NO_HISPD_BIT))
		ctrl |= SDHCI_CTRL_HISPD;
	else
		ctrl &= ~SDHCI_CTRL_HISPD;

	if (host->version >= SDHCI_SPEC_300) {
		u16 clk, ctrl_2;

		/* In case of UHS-I modes, set High Speed Enable */
		if ((ios->timing == MMC_TIMING_MMC_HS400) ||
		    (ios->timing == MMC_TIMING_MMC_HS200) ||
		    (ios->timing == MMC_TIMING_MMC_DDR52) ||
		    (ios->timing == MMC_TIMING_UHS_SDR50) ||
		    (ios->timing == MMC_TIMING_UHS_SDR104) ||
		    (ios->timing == MMC_TIMING_UHS_DDR50) ||
		    (ios->timing == MMC_TIMING_UHS_SDR25))
			ctrl |= SDHCI_CTRL_HISPD;

		if (!host->preset_enabled) {
			sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
			/*
			 * We only need to set Driver Strength if the
			 * preset value enable is not set.
			 */
			ctrl_2 = sdhci_readw(host, SDHCI_HOST_CONTROL2);
			ctrl_2 &= ~SDHCI_CTRL_DRV_TYPE_MASK;
			if (ios->drv_type == MMC_SET_DRIVER_TYPE_A)
				ctrl_2 |= SDHCI_CTRL_DRV_TYPE_A;
			else if (ios->drv_type == MMC_SET_DRIVER_TYPE_B)
				ctrl_2 |= SDHCI_CTRL_DRV_TYPE_B;
			else if (ios->drv_type == MMC_SET_DRIVER_TYPE_C)
				ctrl_2 |= SDHCI_CTRL_DRV_TYPE_C;
			else if (ios->drv_type == MMC_SET_DRIVER_TYPE_D)
				ctrl_2 |= SDHCI_CTRL_DRV_TYPE_D;
			else {
				pr_warn("%s: invalid driver type, default to driver type B\n",
					mmc_hostname(mmc));
				ctrl_2 |= SDHCI_CTRL_DRV_TYPE_B;
			}

			sdhci_writew(host, ctrl_2, SDHCI_HOST_CONTROL2);
		} else {
			/*
			 * According to SDHC Spec v3.00, if the Preset Value
			 * Enable in the Host Control 2 register is set, we
			 * need to reset SD Clock Enable before changing High
			 * Speed Enable to avoid generating clock gliches.
			 */

			/* Reset SD Clock Enable */
			clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
			clk &= ~SDHCI_CLOCK_CARD_EN;
			sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

			sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);

			/* Re-enable SD Clock */
			host->ops->set_clock(host, host->clock);
		}

		/* Reset SD Clock Enable */
		clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
		clk &= ~SDHCI_CLOCK_CARD_EN;
		sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

		host->ops->set_uhs_signaling(host, ios->timing);
		host->timing = ios->timing;

		if (!(host->quirks2 & SDHCI_QUIRK2_PRESET_VALUE_BROKEN) &&
				((ios->timing == MMC_TIMING_UHS_SDR12) ||
				 (ios->timing == MMC_TIMING_UHS_SDR25) ||
				 (ios->timing == MMC_TIMING_UHS_SDR50) ||
				 (ios->timing == MMC_TIMING_UHS_SDR104) ||
				 (ios->timing == MMC_TIMING_UHS_DDR50) ||
				 (ios->timing == MMC_TIMING_MMC_DDR52))) {
			u16 preset;

			sdhci_enable_preset_value(host, true);
			preset = sdhci_get_preset_value(host);
			ios->drv_type = (preset & SDHCI_PRESET_DRV_MASK)
				>> SDHCI_PRESET_DRV_SHIFT;
		}

		/* Re-enable SD Clock */
		host->ops->set_clock(host, host->clock);
	} else
		sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);

	/*
	 * Some (ENE) controllers go apeshit on some ios operation,
	 * signalling timeout and CRC errors even on CMD0. Resetting
	 * it on each ios seems to solve the problem.
	 */
	if (host->quirks & SDHCI_QUIRK_RESET_CMD_DATA_ON_IOS)
		sdhci_do_reset(host, SDHCI_RESET_CMD | SDHCI_RESET_DATA);

	mmiowb();
	spin_unlock_irqrestore(&host->lock, flags);
}

static int sdhci_get_cd(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);

	if (host->ops->get_cd)
		return host->ops->get_cd(host);

	/* Host native card detect */
	return !!(sdhci_readl(host, SDHCI_PRESENT_STATE) & SDHCI_CARD_PRESENT);
}

static int sdhci_check_ro(struct sdhci_host *host)
{
	unsigned long flags;
	int is_readonly;

	spin_lock_irqsave(&host->lock, flags);

	if (host->flags & SDHCI_DEVICE_DEAD)
		is_readonly = 0;
	else if (host->ops->get_ro)
		is_readonly = host->ops->get_ro(host);
	else
		is_readonly = !(sdhci_readl(host, SDHCI_PRESENT_STATE)
				& SDHCI_WRITE_PROTECT);

	spin_unlock_irqrestore(&host->lock, flags);

	return is_readonly;
}

#define SAMPLE_COUNT	5

static int sdhci_get_ro(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);
	int i, ro_count;

	if (!(host->quirks & SDHCI_QUIRK_UNSTABLE_RO_DETECT))
		return sdhci_check_ro(host);

	ro_count = 0;
	for (i = 0; i < SAMPLE_COUNT; i++) {
		if (sdhci_check_ro(host)) {
			if (++ro_count > SAMPLE_COUNT / 2)
				return 1;
		}
		msleep(30);
	}
	return 0;
}

static void sdhci_hw_reset(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);

	if (host->ops && host->ops->hw_reset)
		host->ops->hw_reset(host);
}

static void sdhci_enable_sdio_irq_nolock(struct sdhci_host *host, int enable)
{
	if (!(host->flags & SDHCI_DEVICE_DEAD)) {
		if (enable)
			host->ier |= SDHCI_INT_CARD_INT;
		else
			host->ier &= ~SDHCI_INT_CARD_INT;

		sdhci_writel(host, host->ier, SDHCI_INT_ENABLE);
		sdhci_writel(host, host->ier, SDHCI_SIGNAL_ENABLE);
		mmiowb();
	}
}

static void sdhci_enable_sdio_irq(struct mmc_host *mmc, int enable)
{
	struct sdhci_host *host = mmc_priv(mmc);
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	if (enable)
		host->flags |= SDHCI_SDIO_IRQ_ENABLED;
	else
		host->flags &= ~SDHCI_SDIO_IRQ_ENABLED;

	sdhci_enable_sdio_irq_nolock(host, enable);
	spin_unlock_irqrestore(&host->lock, flags);
}

static int sdhci_start_signal_voltage_switch(struct mmc_host *mmc,
					     struct mmc_ios *ios)
{
	struct sdhci_host *host = mmc_priv(mmc);
	u16 ctrl;
	//int ret;

	/*
	 * Signal Voltage Switching is only applicable for Host Controllers
	 * v3.00 and above.
	 */
	if (host->version < SDHCI_SPEC_300)
		return 0;

	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);

	switch (ios->signal_voltage) {
	case MMC_SIGNAL_VOLTAGE_330:
		/* Set 1.8V Signal Enable in the Host Control2 register to 0 */
		ctrl &= ~SDHCI_CTRL_VDD_180;
		sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
        #if !defined(REMOVE_UNUSED_CODE)
		if (!IS_ERR(mmc->supply.vqmmc)) {
			ret = regulator_set_voltage(mmc->supply.vqmmc, 2700000,
						    3600000);
			if (ret) {
				pr_warn("%s: Switching to 3.3V signalling voltage failed\n",
					mmc_hostname(mmc));
				return -EIO;
			}
		}
		#endif
		/* Wait for 5ms */
		usleep(5000);

		/* 3.3V regulator output should be stable within 5 ms */
		ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		if (!(ctrl & SDHCI_CTRL_VDD_180))
			return 0;

		pr_warn("%s: 3.3V regulator output did not became stable\n",
			mmc_hostname(mmc));

		return -EAGAIN;
	case MMC_SIGNAL_VOLTAGE_180:
		#if !defined(REMOVE_UNUSED_CODE)
		if (!IS_ERR(mmc->supply.vqmmc)) {
			ret = regulator_set_voltage(mmc->supply.vqmmc,
					1700000, 1950000);
			if (ret) {
				pr_warn("%s: Switching to 1.8V signalling voltage failed\n",
					mmc_hostname(mmc));
				return -EIO;
			}
		}
		#endif

		/*
		 * Enable 1.8V Signal Enable in the Host Control2
		 * register
		 */
		ctrl |= SDHCI_CTRL_VDD_180;
		sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);

		/* Some controller need to do more when switching */
		if (host->ops->voltage_switch)
			host->ops->voltage_switch(host);

		/* 1.8V regulator output should be stable within 5 ms */
		ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		if (ctrl & SDHCI_CTRL_VDD_180)
			return 0;

		pr_warn("%s: 1.8V regulator output did not became stable\n",
			mmc_hostname(mmc));

		return -EAGAIN;
	case MMC_SIGNAL_VOLTAGE_120:
		#if !defined(REMOVE_UNUSED_CODE)
		if (!IS_ERR(mmc->supply.vqmmc)) {
			ret = regulator_set_voltage(mmc->supply.vqmmc, 1100000,
						    1300000);
			if (ret) {
				pr_warn("%s: Switching to 1.2V signalling voltage failed\n",
					mmc_hostname(mmc));
				return -EIO;
			}
		}
		#endif
		return 0;
	default:
		/* No signal voltage switch required */
		return 0;
	}
}

static int sdhci_card_busy(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);
	u32 present_state;
	ithPrintf("enter sdhci_card_busy() ~~~~~~~~~~~~~~~~~ \n");

	/* Check whether DAT[3:0] is 0000 */
	present_state = sdhci_readl(host, SDHCI_PRESENT_STATE);

	return !(present_state & SDHCI_DATA_LVL_MASK);
}

static void sdhci_enable_preset_value(struct sdhci_host *host, bool enable)
{
	/* Host Controller v3.00 defines preset value registers */
	if (host->version < SDHCI_SPEC_300)
		return;

	/*
	 * We only enable or disable Preset Value if they are not already
	 * enabled or disabled respectively. Otherwise, we bail out.
	 */
	if (host->preset_enabled != enable) {
		u16 ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);

		if (enable)
			ctrl |= SDHCI_CTRL_PRESET_VAL_ENABLE;
		else
			ctrl &= ~SDHCI_CTRL_PRESET_VAL_ENABLE;

		sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);

		if (enable)
			host->flags |= SDHCI_PV_ENABLED;
		else
			host->flags &= ~SDHCI_PV_ENABLED;

		host->preset_enabled = enable;
	}
}

static void sdhci_card_event(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);
	unsigned long flags;
	int present;
ithPrintf("enter sdhci_card_event() ~~~~~~~~~~~~~~~~~ \n");
	/* First check if client has provided their own card event */
	if (host->ops->card_event)
		host->ops->card_event(host);

	present = sdhci_get_cd(host->mmc);

	spin_lock_irqsave(&host->lock, flags);

	/* Check host->mrq first in case we are runtime suspended */
	if (host->mrq && !present) {
		pr_err("%s: Card removed during transfer!\n",
			mmc_hostname(host->mmc));
		pr_err("%s: Resetting controller.\n",
			mmc_hostname(host->mmc));

		sdhci_do_reset(host, SDHCI_RESET_CMD);
		sdhci_do_reset(host, SDHCI_RESET_DATA);

		host->mrq->cmd->error = -ENOMEDIUM;
		tasklet_schedule(&host->finish_tasklet);
	}

	spin_unlock_irqrestore(&host->lock, flags);
}

static void sdhci_lock(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);
	
	if (host->ops->lock)
		host->ops->lock(host);
}

static void sdhci_unlock(struct mmc_host *mmc)
{
	struct sdhci_host *host = mmc_priv(mmc);
	
	if (host->ops->unlock)
		host->ops->unlock(host);
}

static const struct mmc_host_ops sdhci_ops = {
	.request	= sdhci_request,
	//.post_req	= sdhci_post_req,
	//.pre_req	= sdhci_pre_req,
	.set_ios	= sdhci_set_ios,
	.get_cd		= sdhci_get_cd,
	.get_ro		= sdhci_get_ro,
	.hw_reset	= sdhci_hw_reset,
	.enable_sdio_irq = sdhci_enable_sdio_irq,
	.start_signal_voltage_switch	= sdhci_start_signal_voltage_switch,
	#if 0
	.prepare_hs400_tuning		= sdhci_prepare_hs400_tuning,
	.execute_tuning			= sdhci_execute_tuning,
	.select_drive_strength		= sdhci_select_drive_strength,
	#endif
	.card_event			= sdhci_card_event,
	.card_busy	= sdhci_card_busy,
	.lock = sdhci_lock,
	.unlock = sdhci_unlock,
};

/*****************************************************************************\
 *                                                                           *
 * Tasklets                                                                  *
 *                                                                           *
\*****************************************************************************/

static void sdhci_tasklet_finish(unsigned long param)
{
	struct sdhci_host *host;
	unsigned long flags;
	struct mmc_request *mrq;

	host = (struct sdhci_host*)param;

	spin_lock_irqsave(&host->lock, flags);

        /*
         * If this tasklet gets rescheduled while running, it will
         * be run again afterwards but without any active request.
         */
	if (!host->mrq) {
		spin_unlock_irqrestore(&host->lock, flags);
		return;
	}

#if defined(SD_IRQ_ENABLE)
	del_timer(&host->timer);
#endif
	mrq = host->mrq;

	/*
	 * Always unmap the data buffers if they were mapped by
	 * sdhci_prepare_data() whenever we finish with a request.
	 * This avoids leaking DMA mappings on error.
	 */
	if (host->flags & SDHCI_REQ_USE_DMA) {
		struct mmc_data *data = mrq->data;

		if (data && data->host_cookie == COOKIE_MAPPED) {
			if (data->flags & MMC_DATA_READ)
				ithInvalidateDCacheRange((void*)data->buf, data->blksz * data->blocks);
			data->host_cookie = COOKIE_UNMAPPED;
		}
		
		#if defined(WIN32)
		if (data && (data->flags & MMC_DATA_READ)) {
			ithReadVram(data->buf, (uint32_t)data->tmp_buf, data->blksz * data->blocks);
			itpVmemFree((uint32_t)data->tmp_buf);
		}
		#endif
	}

	/*
	 * The controller needs a reset of internal state machines
	 * upon error conditions.
	 */
	if (!(host->flags & SDHCI_DEVICE_DEAD) &&
	    ((mrq->cmd && mrq->cmd->error) ||
	     (mrq->sbc && mrq->sbc->error) ||
	     (mrq->data && ((mrq->data->error && !mrq->data->stop) ||
			    (mrq->data->stop && mrq->data->stop->error))) ||
	     (host->quirks & SDHCI_QUIRK_RESET_AFTER_REQUEST))) {
		//sdhci_dumpregs(host); // test
		/* Some controllers need this kick or reset won't work here */
		if (host->quirks & SDHCI_QUIRK_CLOCK_BEFORE_RESET)
			/* This is to force an update */
			host->ops->set_clock(host, host->clock);

		/* Spec says we should do both at the same time, but Ricoh
		   controllers do not like that. */
		sdhci_do_reset(host, SDHCI_RESET_CMD);
		sdhci_do_reset(host, SDHCI_RESET_DATA);
	}

	host->mrq = NULL;
	host->cmd = NULL;
	host->data = NULL;

	mmiowb();
	spin_unlock_irqrestore(&host->lock, flags);

	mmc_request_done(host->mmc, mrq);
}

static void sdhci_timeout_timer(unsigned long data)
{
	struct sdhci_host *host;
	unsigned long flags;

	host = (struct sdhci_host*)data;

	spin_lock_irqsave(&host->lock, flags);

	if (host->mrq) {
		pr_err("%s: Timeout waiting for hardware interrupt.\n",
		       mmc_hostname(host->mmc));
		sdhci_dumpregs(host);

		if (host->data) {
			host->data->error = -ETIMEDOUT;
			sdhci_finish_data(host);
		} else {
			if (host->cmd)
				host->cmd->error = -ETIMEDOUT;
			else
				host->mrq->cmd->error = -ETIMEDOUT;

			tasklet_schedule(&host->finish_tasklet);
		}
	}

	mmiowb();
	spin_unlock_irqrestore(&host->lock, flags);
}


/*****************************************************************************\
 *                                                                           *
 * Interrupt handling                                                        *
 *                                                                           *
\*****************************************************************************/

static void sdhci_cmd_irq(struct sdhci_host *host, u32 intmask, u32 *mask)
{
	BUG_ON(intmask == 0);

	if (!host->cmd) {
		pr_err("%s: Got command interrupt 0x%08x even though no command operation was in progress.\n",
		       mmc_hostname(host->mmc), (unsigned)intmask);
		sdhci_dumpregs(host);
		return;
	}

	if (intmask & (SDHCI_INT_TIMEOUT | SDHCI_INT_CRC |
		       SDHCI_INT_END_BIT | SDHCI_INT_INDEX)) {
		if (intmask & SDHCI_INT_TIMEOUT)
			host->cmd->error = -ETIMEDOUT;
		else
			host->cmd->error = -EILSEQ;

		/*
		 * If this command initiates a data phase and a response
		 * CRC error is signalled, the card can start transferring
		 * data - the card may have received the command without
		 * error.  We must not terminate the mmc_request early.
		 *
		 * If the card did not receive the command or returned an
		 * error which prevented it sending data, the data phase
		 * will time out.
		 */
		if (host->cmd->data &&
		    (intmask & (SDHCI_INT_CRC | SDHCI_INT_TIMEOUT)) ==
		     SDHCI_INT_CRC) {
			host->cmd = NULL;
			return;
		}

		tasklet_schedule(&host->finish_tasklet);
		return;
	}

	/*
	 * The host can send and interrupt when the busy state has
	 * ended, allowing us to wait without wasting CPU cycles.
	 * Unfortunately this is overloaded on the "data complete"
	 * interrupt, so we need to take some care when handling
	 * it.
	 *
	 * Note: The 1.0 specification is a bit ambiguous about this
	 *       feature so there might be some problems with older
	 *       controllers.
	 */
	if (host->cmd->flags & MMC_RSP_BUSY) {
		if (host->cmd->data)
            pr_debug("Cannot wait for busy signal when also doing a data transfer");
		else if (!(host->quirks & SDHCI_QUIRK_NO_BUSY_IRQ)
				&& !host->busy_handle) {
			/* Mark that command complete before busy is ended */
			host->busy_handle = 1;
			return;
		}

		/* The controller does not support the end-of-busy IRQ,
		 * fall through and take the SDHCI_INT_RESPONSE */
	} else if ((host->quirks2 & SDHCI_QUIRK2_STOP_WITH_TC) &&
		   host->cmd->opcode == MMC_STOP_TRANSMISSION && !host->data) {
		*mask &= ~SDHCI_INT_DATA_END;
	}

	if (intmask & SDHCI_INT_RESPONSE)
		sdhci_finish_command(host);
}

static void sdhci_adma_show_error(struct sdhci_host *host) { }

static void sdhci_data_irq(struct sdhci_host *host, u32 intmask)
{
#if !defined(REMOVE_UNUSED_CODE)
	u32 command;
#endif
	BUG_ON(intmask == 0);

    #if !defined(REMOVE_UNUSED_CODE)
	/* CMD19 generates _only_ Buffer Read Ready interrupt */
	if (intmask & SDHCI_INT_DATA_AVAIL) {
		command = SDHCI_GET_CMD(sdhci_readw(host, SDHCI_COMMAND));
		if (command == MMC_SEND_TUNING_BLOCK ||
		    command == MMC_SEND_TUNING_BLOCK_HS200) {
			host->tuning_done = 1;
			wake_up(&host->buf_ready_int);
			return;
		}
	}
	#endif

	if (!host->data) {
		/*
		 * The "data complete" interrupt is also used to
		 * indicate that a busy state has ended. See comment
		 * above in sdhci_cmd_irq().
		 */
		if (host->cmd && (host->cmd->flags & MMC_RSP_BUSY)) {
			if (intmask & SDHCI_INT_DATA_TIMEOUT) {
				host->cmd->error = -ETIMEDOUT;
				tasklet_schedule(&host->finish_tasklet);
				return;
			}
			if (intmask & SDHCI_INT_DATA_END) {
				/*
				 * Some cards handle busy-end interrupt
				 * before the command completed, so make
				 * sure we do things in the proper order.
				 */
				if (host->busy_handle)
					sdhci_finish_command(host);
				else
					host->busy_handle = 1;
				return;
			}
		}

		pr_err("%s: Got data interrupt 0x%08x even though no data operation was in progress.\n",
		       mmc_hostname(host->mmc), (unsigned)intmask);
		sdhci_dumpregs(host);

		return;
	}

	if (intmask & SDHCI_INT_DATA_TIMEOUT)
		host->data->error = -ETIMEDOUT;
	else if (intmask & SDHCI_INT_DATA_END_BIT)
		host->data->error = -EILSEQ;
	else if ((intmask & SDHCI_INT_DATA_CRC) &&
		SDHCI_GET_CMD(sdhci_readw(host, SDHCI_COMMAND))
			!= MMC_BUS_TEST_R)
		host->data->error = -EILSEQ;
	else if (intmask & SDHCI_INT_ADMA_ERROR) {
		pr_err("%s: ADMA error\n", mmc_hostname(host->mmc));
		sdhci_adma_show_error(host);
		host->data->error = -EIO;
		if (host->ops->adma_workaround)
			host->ops->adma_workaround(host, intmask);
	}

	if (host->data->error)
		sdhci_finish_data(host);
	else {
		if (intmask & (SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL))
			sdhci_transfer_pio(host);

		/*
		 * We currently don't do anything fancy with DMA
		 * boundaries, but as we can't disable the feature
		 * we need to at least restart the transfer.
		 *
		 * According to the spec sdhci_readl(host, SDHCI_DMA_ADDRESS)
		 * should return a valid address to continue from, but as
		 * some controllers are faulty, don't trust them.
		 */
		if (intmask & SDHCI_INT_DMA_END) {
			u32 dmastart, dmanow;
			dmastart = dma_address(host->data);
			dmanow = dmastart + host->data->bytes_xfered;
			/*
			 * Force update to the next DMA block boundary.
			 */
			dmanow = (dmanow &
				~(SDHCI_DEFAULT_BOUNDARY_SIZE - 1)) +
				SDHCI_DEFAULT_BOUNDARY_SIZE;
			host->data->bytes_xfered = dmanow - dmastart;
            pr_debug("%s: DMA base 0x%08x, transferred 0x%06x bytes,"
				" next 0x%08x\n",
				mmc_hostname(host->mmc), dmastart,
				host->data->bytes_xfered, dmanow);
			sdhci_writel(host, dmanow, SDHCI_DMA_ADDRESS);
		}

		if (intmask & SDHCI_INT_DATA_END) {
			if (host->cmd) {
				/*
				 * Data managed to finish before the
				 * command completed. Make sure we do
				 * things in the proper order.
				 */
				host->data_early = 1;
			} else {
				sdhci_finish_data(host);
			}
		}
	}
}

static void sdhci_irq(void* arg)
{
	struct sdhci_host *host = (struct sdhci_host *)arg;
	u32 intmask, mask, unexpected = 0;
	int max_loops = 16;

	spin_lock(&host->lock);

	if (host->runtime_suspended && !sdhci_sdio_irq_enabled(host)) {
		spin_unlock(&host->lock);
		return;
	}

	intmask = sdhci_readl(host, SDHCI_INT_STATUS);
#if defined(SD_IRQ_ENABLE)
	if (!intmask || intmask == 0xffffffff) {
		ithPrintf(" %s: invalid reg value 0x%X \n", mmc_hostname(host->mmc), intmask);
		goto out;
	}
#else
	if (/*!intmask ||*/ intmask == 0xffffffff) {
		ithPrintf(" %s: invalid reg value 0x%X \n", mmc_hostname(host->mmc), intmask);
		goto out;
	}
#endif

	do {
		/* Clear selected interrupts. */
		mask = intmask & (SDHCI_INT_CMD_MASK | SDHCI_INT_DATA_MASK |
				  SDHCI_INT_BUS_POWER);
		sdhci_writel(host, mask, SDHCI_INT_STATUS);

#if !defined(SD_IRQ_ENABLE)
		if (intmask)
#endif
		pr_debug("*** %s got interrupt: 0x%08x\n",
			mmc_hostname(host->mmc), intmask);

		if (intmask & SDHCI_INT_CMD_MASK)
			sdhci_cmd_irq(host, intmask & SDHCI_INT_CMD_MASK,
				      &intmask);

		if (intmask & SDHCI_INT_DATA_MASK)
			sdhci_data_irq(host, intmask & SDHCI_INT_DATA_MASK);

		if (intmask & SDHCI_INT_BUS_POWER)
			pr_err("%s: Card is consuming too much power!\n",
				mmc_hostname(host->mmc));

		if (intmask & SDHCI_INT_CARD_INT) {
#if defined(SD_IRQ_ENABLE)
			sdhci_enable_sdio_irq_nolock(host, false);
			host->thread_isr |= SDHCI_INT_CARD_INT;
			if (in_interrupt())
				itpPendFunctionCallFromISR(sdhci_thread_irq, host, 0);
#else
			mmc_signal_sdio_irq(host->mmc);
#endif
		}

		intmask &= ~(SDHCI_INT_CARD_INSERT | SDHCI_INT_CARD_REMOVE |
			     SDHCI_INT_CMD_MASK | SDHCI_INT_DATA_MASK |
			     SDHCI_INT_ERROR | SDHCI_INT_BUS_POWER |
			     SDHCI_INT_CARD_INT);

		if (intmask) {
			unexpected |= intmask;
			sdhci_writel(host, intmask, SDHCI_INT_STATUS);
		}

		intmask = sdhci_readl(host, SDHCI_INT_STATUS);
	} while (intmask && --max_loops);
out:
	spin_unlock(&host->lock);

	if (unexpected) {
		pr_err("%s: Unexpected interrupt 0x%08x.\n",
			   mmc_hostname(host->mmc), unexpected);
		sdhci_dumpregs(host);
	}

	return;
}

static void sdhci_thread_irq(void *arg1, uint32_t arg2)
{
	struct sdhci_host *host = (struct sdhci_host *)arg1;
	unsigned long flags;
	u32 isr;

	spin_lock_irqsave(&host->lock, flags);
	isr = host->thread_isr;
	host->thread_isr = 0;
	spin_unlock_irqrestore(&host->lock, flags);

	if (isr & SDHCI_INT_CARD_INT) {
		sdio_run_irqs(host->mmc);

		spin_lock_irqsave(&host->lock, flags);
		if (host->flags & SDHCI_SDIO_IRQ_ENABLED)
			sdhci_enable_sdio_irq_nolock(host, true);
		spin_unlock_irqrestore(&host->lock, flags);
	}

	return;
}

/*****************************************************************************\
 *                                                                           *
 * Device allocation/registration                                            *
 *                                                                           *
\*****************************************************************************/

struct sdhci_host *sdhci_alloc_host(struct device *dev,
    size_t priv_size)
{
    struct mmc_host *mmc;
    struct sdhci_host *host;

    //WARN_ON(dev == NULL);

    mmc = mmc_alloc_host(sizeof(struct sdhci_host) + priv_size, dev);
    if (!mmc)
        return ERR_PTR(-ENOMEM);

    host = mmc_priv(mmc);
    host->mmc = mmc;
    host->mmc_host_ops = sdhci_ops;
    mmc->ops = &host->mmc_host_ops;

    return host;
}

EXPORT_SYMBOL_GPL(sdhci_alloc_host);

static int sdhci_set_dma_mask(struct sdhci_host *host)
{
    return 0;
}

#if defined(SD_IRQ_ENABLE)
static void sdc_interrupt_enable(struct sdhci_host *host)
{
    int intr_num = (host->index == 0) ? ITH_INTR_SD0 : ITH_INTR_SD1;
	
    ithIntrRegisterHandlerIrq(intr_num, sdhci_irq, (void*)host);
    ithIntrSetTriggerModeIrq(intr_num, ITH_INTR_LEVEL);
    ithIntrSetTriggerLevelIrq(intr_num, ITH_INTR_HIGH_RISING);
    ithIntrEnableIrq(intr_num);
}

static void sdc_interrupt_disable(struct sdhci_host *host)
{
	int intr_num = (host->index == 0) ? ITH_INTR_SD0 : ITH_INTR_SD1;
    ithIntrDisableIrq(intr_num);
}
#else
#define sdc_interrupt_enable(a)
#define sdc_interrupt_disable(a)
#endif

int sdhci_add_host(struct sdhci_host *host)
{
    struct mmc_host *mmc;
	u32 caps[2] = {0, 0};
    u32 max_current_caps;
    unsigned int ocr_avail;
    unsigned int override_timeout_clk;
    u32 max_clk;
    int ret;

    WARN_ON(host == NULL);
    if (host == NULL)
        return -EINVAL;

    mmc = host->mmc;

    override_timeout_clk = host->timeout_clk;

    sdhci_do_reset(host, SDHCI_RESET_ALL);

    host->version = sdhci_readw(host, SDHCI_HOST_VERSION);
    host->version = (host->version & SDHCI_SPEC_VER_MASK)
        >> SDHCI_SPEC_VER_SHIFT;
    if (host->version > SDHCI_SPEC_300) {
        pr_err("%s: Unknown controller version (%d). You may experience problems.\n",
            mmc_hostname(mmc), host->version);
    }

    caps[0] = (host->quirks & SDHCI_QUIRK_MISSING_CAPS) ? host->caps :
        sdhci_readl(host, SDHCI_CAPABILITIES);

    if (host->version >= SDHCI_SPEC_300)
        caps[1] = (host->quirks & SDHCI_QUIRK_MISSING_CAPS) ?
        host->caps1 :
        sdhci_readl(host, SDHCI_CAPABILITIES_1);

    if (host->quirks & SDHCI_QUIRK_FORCE_DMA)
        host->flags |= SDHCI_USE_SDMA;
    else if (!(caps[0] & SDHCI_CAN_DO_SDMA))
        DBG("Controller doesn't have SDMA capability\n");
    else
        host->flags |= SDHCI_USE_SDMA;

    if ((host->quirks & SDHCI_QUIRK_BROKEN_DMA) &&
        (host->flags & SDHCI_USE_SDMA)) {
        DBG("Disabling DMA as it is marked broken\n");
        host->flags &= ~SDHCI_USE_SDMA;
    }

    if ((host->version >= SDHCI_SPEC_200) &&
        (caps[0] & SDHCI_CAN_DO_ADMA2))
        host->flags |= SDHCI_USE_ADMA;

    if ((host->quirks & SDHCI_QUIRK_BROKEN_ADMA) &&
        (host->flags & SDHCI_USE_ADMA)) {
        DBG("Disabling ADMA as it is marked broken\n");
        host->flags &= ~SDHCI_USE_ADMA;
    }

    /*
    * It is assumed that a 64-bit capable device has set a 64-bit DMA mask
    * and *must* do 64-bit DMA.  A driver has the opportunity to change
    * that during the first call to ->enable_dma().  Similarly
    * SDHCI_QUIRK2_BROKEN_64_BIT_DMA must be left to the drivers to
    * implement.
    */
    if (caps[0] & SDHCI_CAN_64BIT)
        host->flags |= SDHCI_USE_64_BIT_DMA;

    if (host->flags & (SDHCI_USE_SDMA | SDHCI_USE_ADMA)) {
        ret = sdhci_set_dma_mask(host);

        if (!ret && host->ops->enable_dma)
            ret = host->ops->enable_dma(host);

        if (ret) {
            pr_warn("%s: No suitable DMA available - falling back to PIO\n",
                mmc_hostname(mmc));
            host->flags &= ~(SDHCI_USE_SDMA | SDHCI_USE_ADMA);

            ret = 0;
        }
    }

#if !defined(REMOVE_UNUSED_CODE)
    /* SDMA does not support 64-bit DMA */
    if (host->flags & SDHCI_USE_64_BIT_DMA)
        host->flags &= ~SDHCI_USE_SDMA;

    if (host->flags & SDHCI_USE_ADMA) {
        dma_addr_t dma;
        void *buf;

        /*
        * The DMA descriptor table size is calculated as the maximum
        * number of segments times 2, to allow for an alignment
        * descriptor for each segment, plus 1 for a nop end descriptor,
        * all multipled by the descriptor size.
        */
        if (host->flags & SDHCI_USE_64_BIT_DMA) {
            host->adma_table_sz = (SDHCI_MAX_SEGS * 2 + 1) *
                SDHCI_ADMA2_64_DESC_SZ;
            host->desc_sz = SDHCI_ADMA2_64_DESC_SZ;
		} else {
            host->adma_table_sz = (SDHCI_MAX_SEGS * 2 + 1) *
                SDHCI_ADMA2_32_DESC_SZ;
            host->desc_sz = SDHCI_ADMA2_32_DESC_SZ;
        }

        host->align_buffer_sz = SDHCI_MAX_SEGS * SDHCI_ADMA2_ALIGN;
        buf = dma_alloc_coherent(mmc_dev(mmc), host->align_buffer_sz +
            host->adma_table_sz, &dma, GFP_KERNEL);
        if (!buf) {
            pr_warn("%s: Unable to allocate ADMA buffers - falling back to standard DMA\n",
                mmc_hostname(mmc));
            host->flags &= ~SDHCI_USE_ADMA;
		} else if ((dma + host->align_buffer_sz) &
            (SDHCI_ADMA2_DESC_ALIGN - 1)) {
            pr_warn("%s: unable to allocate aligned ADMA descriptor\n",
                mmc_hostname(mmc));
            host->flags &= ~SDHCI_USE_ADMA;
            dma_free_coherent(mmc_dev(mmc), host->align_buffer_sz +
                host->adma_table_sz, buf, dma);
		} else {
            host->align_buffer = buf;
            host->align_addr = dma;

            host->adma_table = buf + host->align_buffer_sz;
            host->adma_addr = dma + host->align_buffer_sz;
        }
    }

    /*
    * If we use DMA, then it's up to the caller to set the DMA
    * mask, but PIO does not need the hw shim so we set a new
    * mask here in that case.
    */
    if (!(host->flags & (SDHCI_USE_SDMA | SDHCI_USE_ADMA))) {
        host->dma_mask = DMA_BIT_MASK(64);
        mmc_dev(mmc)->dma_mask = &host->dma_mask;
    }
#endif

    if (host->version >= SDHCI_SPEC_300)
        host->max_clk = (caps[0] & SDHCI_CLOCK_V3_BASE_MASK)
        >> SDHCI_CLOCK_BASE_SHIFT;
    else
        host->max_clk = (caps[0] & SDHCI_CLOCK_BASE_MASK)
        >> SDHCI_CLOCK_BASE_SHIFT;

    host->max_clk *= 1000000;
    if (host->max_clk == 0 || host->quirks &
        SDHCI_QUIRK_CAP_CLOCK_BASE_BROKEN) {
        if (!host->ops->get_max_clock) {
            pr_err("%s: Hardware doesn't specify base clock frequency.\n",
                mmc_hostname(mmc));
            ret = -ENODEV;
            goto undma;
        }
        host->max_clk = host->ops->get_max_clock(host);
    }

    /*
    * In case of Host Controller v3.00, find out whether clock
    * multiplier is supported.
    */
    host->clk_mul = (caps[1] & SDHCI_CLOCK_MUL_MASK) >>
        SDHCI_CLOCK_MUL_SHIFT;

    /*
    * In case the value in Clock Multiplier is 0, then programmable
    * clock mode is not supported, otherwise the actual clock
    * multiplier is one more than the value of Clock Multiplier
    * in the Capabilities Register.
    */
    if (host->clk_mul)
        host->clk_mul += 1;

    /*
    * Set host parameters.
    */
    max_clk = host->max_clk;

    if (host->ops->get_min_clock)
        mmc->f_min = host->ops->get_min_clock(host);
    else if (host->version >= SDHCI_SPEC_300) {
        if (host->clk_mul) {
            mmc->f_min = (host->max_clk * host->clk_mul) / 1024;
            max_clk = host->max_clk * host->clk_mul;
		} else
            mmc->f_min = host->max_clk / SDHCI_MAX_DIV_SPEC_300;
	} else
        mmc->f_min = host->max_clk / SDHCI_MAX_DIV_SPEC_200;

    if (!mmc->f_max || mmc->f_max > max_clk)
        mmc->f_max = max_clk;

    if (!(host->quirks & SDHCI_QUIRK_DATA_TIMEOUT_USES_SDCLK)) {
        host->timeout_clk = (caps[0] & SDHCI_TIMEOUT_CLK_MASK) >>
            SDHCI_TIMEOUT_CLK_SHIFT;
        if (host->timeout_clk == 0) {
            if (host->ops->get_timeout_clock) {
                host->timeout_clk =
                    host->ops->get_timeout_clock(host);
			} else {
                pr_err("%s: Hardware doesn't specify timeout clock frequency.\n",
                    mmc_hostname(mmc));
                ret = -ENODEV;
                goto undma;
            }
        }

        if (caps[0] & SDHCI_TIMEOUT_CLK_UNIT)
            host->timeout_clk *= 1000;

        if (override_timeout_clk)
            host->timeout_clk = override_timeout_clk;

        mmc->max_busy_timeout = host->ops->get_max_timeout_count ?
            host->ops->get_max_timeout_count(host) : 1 << 27;
        mmc->max_busy_timeout /= host->timeout_clk;
    }

    mmc->caps |= MMC_CAP_SDIO_IRQ | MMC_CAP_ERASE | MMC_CAP_CMD23;
#if defined(SD_IRQ_ENABLE)
    mmc->caps2 |= MMC_CAP2_SDIO_IRQ_NOTHREAD;
#endif
    if (host->quirks & SDHCI_QUIRK_MULTIBLOCK_READ_ACMD12)
        host->flags |= SDHCI_AUTO_CMD12;

    /* Auto-CMD23 stuff only works in ADMA or PIO. */
    if ((host->version >= SDHCI_SPEC_300) &&
        ((host->flags & SDHCI_USE_ADMA) ||
        !(host->flags & SDHCI_USE_SDMA)) &&
        !(host->quirks2 & SDHCI_QUIRK2_ACMD23_BROKEN)) {
        host->flags |= SDHCI_AUTO_CMD23;
        DBG("%s: Auto-CMD23 available\n", mmc_hostname(mmc));
	} else {
        DBG("%s: Auto-CMD23 unavailable\n", mmc_hostname(mmc));
    }

    /*
    * A controller may support 8-bit width, but the board itself
    * might not have the pins brought out.  Boards that support
    * 8-bit width must set "mmc->caps |= MMC_CAP_8_BIT_DATA;" in
    * their platform code before calling sdhci_add_host(), and we
    * won't assume 8-bit width for hosts without that CAP.
    */
    if (!(host->quirks & SDHCI_QUIRK_FORCE_1_BIT_DATA))
        mmc->caps |= MMC_CAP_4_BIT_DATA;

    if (host->quirks2 & SDHCI_QUIRK2_HOST_NO_CMD23)
        mmc->caps &= ~MMC_CAP_CMD23;

    if (caps[0] & SDHCI_CAN_DO_HISPD)
        mmc->caps |= MMC_CAP_SD_HIGHSPEED | MMC_CAP_MMC_HIGHSPEED;

    /* If there are external regulators, get them */
    ret = mmc_regulator_get_supply(mmc);
    if (ret == -EPROBE_DEFER)
        goto undma;

    #if !defined(REMOVE_UNUSED_CODE)
    /* If vqmmc regulator and no 1.8V signalling, then there's no UHS */
    if (!IS_ERR(mmc->supply.vqmmc)) {
        ret = regulator_enable(mmc->supply.vqmmc);
        if (!regulator_is_supported_voltage(mmc->supply.vqmmc, 1700000,
            1950000))
            caps[1] &= ~(SDHCI_SUPPORT_SDR104 |
            SDHCI_SUPPORT_SDR50 |
            SDHCI_SUPPORT_DDR50);
        if (ret) {
            pr_warn("%s: Failed to enable vqmmc regulator: %d\n",
                mmc_hostname(mmc), ret);
            mmc->supply.vqmmc = ERR_PTR(-EINVAL);
        }
    }
	#endif

    if (host->quirks2 & SDHCI_QUIRK2_NO_1_8_V)
        caps[1] &= ~(SDHCI_SUPPORT_SDR104 | SDHCI_SUPPORT_SDR50 |
        SDHCI_SUPPORT_DDR50);

    /* Any UHS-I mode in caps implies SDR12 and SDR25 support. */
    if (caps[1] & (SDHCI_SUPPORT_SDR104 | SDHCI_SUPPORT_SDR50 |
        SDHCI_SUPPORT_DDR50))
        mmc->caps |= MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25;

    /* SDR104 supports also implies SDR50 support */
    if (caps[1] & SDHCI_SUPPORT_SDR104) {
        mmc->caps |= MMC_CAP_UHS_SDR104 | MMC_CAP_UHS_SDR50;
        /* SD3.0: SDR104 is supported so (for eMMC) the caps2
        * field can be promoted to support HS200.
        */
        if (!(host->quirks2 & SDHCI_QUIRK2_BROKEN_HS200))
            mmc->caps2 |= MMC_CAP2_HS200;
	} else if (caps[1] & SDHCI_SUPPORT_SDR50)
        mmc->caps |= MMC_CAP_UHS_SDR50;

    if (host->quirks2 & SDHCI_QUIRK2_CAPS_BIT63_FOR_HS400 &&
        (caps[1] & SDHCI_SUPPORT_HS400))
        mmc->caps2 |= MMC_CAP2_HS400;

    #if !defined(REMOVE_UNUSED_CODE)
    if ((mmc->caps2 & MMC_CAP2_HSX00_1_2V) &&
        (IS_ERR(mmc->supply.vqmmc) ||
        !regulator_is_supported_voltage(mmc->supply.vqmmc, 1100000,
        1300000)))
        mmc->caps2 &= ~MMC_CAP2_HSX00_1_2V;
	#endif

    if ((caps[1] & SDHCI_SUPPORT_DDR50) &&
        !(host->quirks2 & SDHCI_QUIRK2_BROKEN_DDR50))
        mmc->caps |= MMC_CAP_UHS_DDR50;

    /* Does the host need tuning for SDR50? */
    if (caps[1] & SDHCI_USE_SDR50_TUNING)
        host->flags |= SDHCI_SDR50_NEEDS_TUNING;

    /* Driver Type(s) (A, C, D) supported by the host */
    if (caps[1] & SDHCI_DRIVER_TYPE_A)
        mmc->caps |= MMC_CAP_DRIVER_TYPE_A;
    if (caps[1] & SDHCI_DRIVER_TYPE_C)
        mmc->caps |= MMC_CAP_DRIVER_TYPE_C;
    if (caps[1] & SDHCI_DRIVER_TYPE_D)
        mmc->caps |= MMC_CAP_DRIVER_TYPE_D;

    /* Initial value for re-tuning timer count */
    host->tuning_count = (caps[1] & SDHCI_RETUNING_TIMER_COUNT_MASK) >>
        SDHCI_RETUNING_TIMER_COUNT_SHIFT;

    /*
    * In case Re-tuning Timer is not disabled, the actual value of
    * re-tuning timer will be 2 ^ (n - 1).
    */
    if (host->tuning_count)
        host->tuning_count = 1 << (host->tuning_count - 1);

    /* Re-tuning mode supported by the Host Controller */
    host->tuning_mode = (caps[1] & SDHCI_RETUNING_MODE_MASK) >>
        SDHCI_RETUNING_MODE_SHIFT;

    ocr_avail = 0;

    /*
    * According to SD Host Controller spec v3.00, if the Host System
    * can afford more than 150mA, Host Driver should set XPC to 1. Also
    * the value is meaningful only if Voltage Support in the Capabilities
    * register is set. The actual current value is 4 times the register
    * value.
    */
    max_current_caps = sdhci_readl(host, SDHCI_MAX_CURRENT);
	#if !defined(REMOVE_UNUSED_CODE)
    if (!max_current_caps && !IS_ERR(mmc->supply.vmmc)) {
        int curr = regulator_get_current_limit(mmc->supply.vmmc);
        if (curr > 0) {

            /* convert to SDHCI_MAX_CURRENT format */
			curr = curr/1000;  /* convert to mA */
			curr = curr/SDHCI_MAX_CURRENT_MULTIPLIER;

            curr = min_t(u32, curr, SDHCI_MAX_CURRENT_LIMIT);
            max_current_caps =
                (curr << SDHCI_MAX_CURRENT_330_SHIFT) |
                (curr << SDHCI_MAX_CURRENT_300_SHIFT) |
                (curr << SDHCI_MAX_CURRENT_180_SHIFT);
        }
    }
	#endif

    if (caps[0] & SDHCI_CAN_VDD_330) {
        ocr_avail |= MMC_VDD_32_33 | MMC_VDD_33_34;

        mmc->max_current_330 = ((max_current_caps &
            SDHCI_MAX_CURRENT_330_MASK) >>
            SDHCI_MAX_CURRENT_330_SHIFT) *
            SDHCI_MAX_CURRENT_MULTIPLIER;
    }
    if (caps[0] & SDHCI_CAN_VDD_300) {
        ocr_avail |= MMC_VDD_29_30 | MMC_VDD_30_31;

        mmc->max_current_300 = ((max_current_caps &
            SDHCI_MAX_CURRENT_300_MASK) >>
            SDHCI_MAX_CURRENT_300_SHIFT) *
            SDHCI_MAX_CURRENT_MULTIPLIER;
    }
    if (caps[0] & SDHCI_CAN_VDD_180) {
        ocr_avail |= MMC_VDD_165_195;

        mmc->max_current_180 = ((max_current_caps &
            SDHCI_MAX_CURRENT_180_MASK) >>
            SDHCI_MAX_CURRENT_180_SHIFT) *
            SDHCI_MAX_CURRENT_MULTIPLIER;
    }

    /* If OCR set by host, use it instead. */
    if (host->ocr_mask)
        ocr_avail = host->ocr_mask;

    /* If OCR set by external regulators, give it highest prio. */
    if (mmc->ocr_avail)
        ocr_avail = mmc->ocr_avail;

    mmc->ocr_avail = ocr_avail;
    mmc->ocr_avail_sdio = ocr_avail;
    if (host->ocr_avail_sdio)
        mmc->ocr_avail_sdio &= host->ocr_avail_sdio;
    mmc->ocr_avail_sd = ocr_avail;
    if (host->ocr_avail_sd)
        mmc->ocr_avail_sd &= host->ocr_avail_sd;
    else /* normal SD controllers don't support 1.8V */
        mmc->ocr_avail_sd &= ~MMC_VDD_165_195;
    mmc->ocr_avail_mmc = ocr_avail;
    if (host->ocr_avail_mmc)
        mmc->ocr_avail_mmc &= host->ocr_avail_mmc;

    if (mmc->ocr_avail == 0) {
        pr_err("%s: Hardware doesn't report any support voltages.\n",
            mmc_hostname(mmc));
        ret = -ENODEV;
        goto unreg;
    }

    spin_lock_init(&host->lock);

    /*
    * Maximum number of segments. Depends on if the hardware
    * can do scatter/gather or not.
    */
    if (host->flags & SDHCI_USE_ADMA)
        mmc->max_segs = SDHCI_MAX_SEGS;
    else if (host->flags & SDHCI_USE_SDMA)
        mmc->max_segs = 1;
    else /* PIO */
        mmc->max_segs = SDHCI_MAX_SEGS;

    /*
    * Maximum number of sectors in one transfer. Limited by SDMA boundary
    * size (512KiB). Note some tuning modes impose a 4MiB limit, but this
    * is less anyway.
    */
    mmc->max_req_size = 524288;

    /*
    * Maximum segment size. Could be one segment with the maximum number
    * of bytes. When doing hardware scatter/gather, each entry cannot
    * be larger than 64 KiB though.
    */
    if (host->flags & SDHCI_USE_ADMA) {
        if (host->quirks & SDHCI_QUIRK_BROKEN_ADMA_ZEROLEN_DESC)
            mmc->max_seg_size = 65535;
        else
            mmc->max_seg_size = 65536;
	} else {
        mmc->max_seg_size = mmc->max_req_size;
    }

    /*
    * Maximum block size. This varies from controller to controller and
    * is specified in the capabilities register.
    */
    if (host->quirks & SDHCI_QUIRK_FORCE_BLK_SZ_2048) {
        mmc->max_blk_size = 2;
	} else {
        mmc->max_blk_size = (caps[0] & SDHCI_MAX_BLOCK_MASK) >>
            SDHCI_MAX_BLOCK_SHIFT;
        if (mmc->max_blk_size >= 3) {
            pr_warn("%s: Invalid maximum block size, assuming 512 bytes\n",
                mmc_hostname(mmc));
            mmc->max_blk_size = 0;
        }
    }

    mmc->max_blk_size = 512 << mmc->max_blk_size;

    /*
    * Maximum block count.
    */
    mmc->max_blk_count = (host->quirks & SDHCI_QUIRK_NO_MULTIBLOCK) ? 1 : 65535;

    /*
    * Init tasklets.
    */
    tasklet_init(&host->finish_tasklet,
        sdhci_tasklet_finish, (unsigned long)host);

#if defined(SD_IRQ_ENABLE)
    setup_timer(&host->timer, sdhci_timeout_timer, (unsigned long)host);
#endif

    //init_waitqueue_head(&host->buf_ready_int); // for tuning

    sdhci_init(host, 0);

    sdc_interrupt_enable(host);

#ifdef CONFIG_MMC_DEBUG
    sdhci_dumpregs(host);
#endif

    ret = mmc_add_host(mmc);
    if (ret)
        goto unled;

    pr_info("%s: SDHCI controller on %s using %s\n",
        mmc_hostname(mmc), host->hw_name,
        (host->flags & SDHCI_USE_ADMA) ?
        (host->flags & SDHCI_USE_64_BIT_DMA) ? "ADMA 64-bit" : "ADMA" :
        (host->flags & SDHCI_USE_SDMA) ? "DMA" : "PIO");

    return 0;

unled:
    sdhci_do_reset(host, SDHCI_RESET_ALL);
    sdhci_writel(host, 0, SDHCI_INT_ENABLE);
    sdhci_writel(host, 0, SDHCI_SIGNAL_ENABLE);
    tasklet_kill(&host->finish_tasklet);
unreg:
undma:
	#if !defined(REMOVE_UNUSED_CODE)
    if (host->align_buffer)
        dma_free_coherent(mmc_dev(mmc), host->align_buffer_sz +
        host->adma_table_sz, host->align_buffer,
        host->align_addr);
    host->adma_table = NULL;
    host->align_buffer = NULL;
	#endif

    return ret;
}

EXPORT_SYMBOL_GPL(sdhci_add_host);

void sdhci_remove_host(struct sdhci_host *host, int dead)
{
	struct mmc_host *mmc = host->mmc;
	unsigned long flags;

	if (dead) {
		spin_lock_irqsave(&host->lock, flags);

		host->flags |= SDHCI_DEVICE_DEAD;

		if (host->mrq) {
			pr_err("%s: Controller removed during "
				" transfer!\n", mmc_hostname(mmc));

			host->mrq->cmd->error = -ENOMEDIUM;
			tasklet_schedule(&host->finish_tasklet);
		}

		spin_unlock_irqrestore(&host->lock, flags);
	}

	mmc_remove_host(mmc);

	if (!dead)
		sdhci_do_reset(host, SDHCI_RESET_ALL);

	sdhci_writel(host, 0, SDHCI_INT_ENABLE);
	sdhci_writel(host, 0, SDHCI_SIGNAL_ENABLE);
	sdc_interrupt_disable(host); // for ite

#if defined(SD_IRQ_ENABLE)
	del_timer_sync(&host->timer);
	destroy_timer(&host->timer); // for ite
#endif

	tasklet_kill(&host->finish_tasklet);

    #if !defined(REMOVE_UNUSED_CODE)
	if (!IS_ERR(mmc->supply.vqmmc))
		regulator_disable(mmc->supply.vqmmc);

	if (host->align_buffer)
		dma_free_coherent(mmc_dev(mmc), host->align_buffer_sz +
				  host->adma_table_sz, host->align_buffer,
				  host->align_addr);

	host->adma_table = NULL;
	host->align_buffer = NULL;
	#endif
}

EXPORT_SYMBOL_GPL(sdhci_remove_host);

void sdhci_free_host(struct sdhci_host *host)
{
    mmc_free_host(host->mmc);
}

EXPORT_SYMBOL_GPL(sdhci_free_host);

