#include <strings.h>
#include <linux/err.h>

#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/errorno.h>

#include <linux/completion.h>
#include <linux/fls.h>
#include <linux/mmc/core.h>
#include "core.h"
#include "host.h"

#include "mmc_ops.h"
#include "sd_ops.h"
#include "sdio_ops.h"



static const unsigned freqs[] = { 400000, 300000, 200000, 100000 };

/*
 * Enabling software CRCs on the data blocks can be a significant (30%)
 * performance cost, and for other reasons may not always be desired.
 * So we allow it it to be disabled.
 */
bool use_spi_crc = 1;


#ifdef CONFIG_FAIL_MMC_REQUEST

#else /* CONFIG_FAIL_MMC_REQUEST */

static inline void mmc_should_fail_request(struct mmc_host *host,
					   struct mmc_request *mrq)
{
}

#endif /* CONFIG_FAIL_MMC_REQUEST */

/**
 *	mmc_request_done - finish processing an MMC request
 *	@host: MMC host which completed request
 *	@mrq: MMC request which request
 *
 *	MMC drivers should call this function when they have completed
 *	their processing of a request.
 */
void mmc_request_done(struct mmc_host *host, struct mmc_request *mrq)
{
	struct mmc_command *cmd = mrq->cmd;
	int err = cmd->error;

	if (err && cmd->retries && mmc_host_is_spi(host)) {
		if (cmd->resp[0] & R1_SPI_ILLEGAL_COMMAND)
			cmd->retries = 0;
	}

	if (err && cmd->retries && !mmc_card_removed(host->card)) {
		/*
		 * Request starter must handle retries - see
		 * mmc_wait_for_req_done().
		 */
		if (mrq->done)
			mrq->done(mrq);
	} else {
		mmc_should_fail_request(host, mrq);

		if (mrq->sbc) {
			pr_debug("%s: req done <CMD%u>: %d: %08x %08x %08x %08x\n",
				mmc_hostname(host), mrq->sbc->opcode,
				mrq->sbc->error,
				mrq->sbc->resp[0], mrq->sbc->resp[1],
				mrq->sbc->resp[2], mrq->sbc->resp[3]);
		}

		pr_debug("%s: req done (CMD%u): %d: %08x %08x %08x %08x\n",
			mmc_hostname(host), cmd->opcode, err,
			cmd->resp[0], cmd->resp[1],
			cmd->resp[2], cmd->resp[3]);

		if (mrq->data) {
			pr_debug("%s:     %d bytes transferred: %d\n",
				mmc_hostname(host),
				mrq->data->bytes_xfered, mrq->data->error);
		}

		if (mrq->stop) {
			pr_debug("%s:     (CMD%u): %d: %08x %08x %08x %08x\n",
				mmc_hostname(host), mrq->stop->opcode,
				mrq->stop->error,
				mrq->stop->resp[0], mrq->stop->resp[1],
				mrq->stop->resp[2], mrq->stop->resp[3]);
		}

		if (mrq->done)
			mrq->done(mrq);

		mmc_host_clk_release(host);
	}
}

static void
mmc_start_request(struct mmc_host *host, struct mmc_request *mrq)
{
	if (mrq->sbc) {
		pr_debug("<%s: starting CMD%u arg %08x flags %08x>\n",
			 mmc_hostname(host), mrq->sbc->opcode,
			 mrq->sbc->arg, mrq->sbc->flags);
	}

	pr_debug("%s: starting CMD%u arg %08x flags %08x\n",
		 mmc_hostname(host), mrq->cmd->opcode,
		 mrq->cmd->arg, mrq->cmd->flags);

	if (mrq->data) {
		pr_debug("%s:     blksz %d blocks %d flags %08x "
			"tsac %d ms nsac %d\n",
			mmc_hostname(host), mrq->data->blksz,
			mrq->data->blocks, mrq->data->flags,
			mrq->data->timeout_ns / 1000000,
			mrq->data->timeout_clks);
	}

	if (mrq->stop) {
		pr_debug("%s:     CMD%u arg %08x flags %08x\n",
			 mmc_hostname(host), mrq->stop->opcode,
			 mrq->stop->arg, mrq->stop->flags);
	}

	WARN_ON(!host->claimed);

	mrq->cmd->error = 0;
	mrq->cmd->mrq = mrq;
	if (mrq->sbc) {
		mrq->sbc->error = 0;
		mrq->sbc->mrq = mrq;
	}
	if (mrq->data) {
		BUG_ON(mrq->data->blksz > host->max_blk_size);
		BUG_ON(mrq->data->blocks > host->max_blk_count);
		BUG_ON(mrq->data->blocks * mrq->data->blksz >
			host->max_req_size);

		mrq->cmd->data = mrq->data;
		mrq->data->error = 0;
		mrq->data->mrq = mrq;
		if (mrq->stop) {
			mrq->data->stop = mrq->stop;
			mrq->stop->error = 0;
			mrq->stop->mrq = mrq;
		}
	}
	mmc_host_clk_hold(host);
	host->ops->request(host, mrq);
}

static void mmc_wait_done(struct mmc_request *mrq)
{
    complete(&mrq->completion);
}

static int __mmc_start_req(struct mmc_host *host, struct mmc_request *mrq)
{
    init_completion(&mrq->completion);
    mrq->done = mmc_wait_done;
    if (mmc_card_removed(host->card)) {
        mrq->cmd->error = -ENOMEDIUM;
        complete(&mrq->completion);
        return -ENOMEDIUM;
    }
    mmc_start_request(host, mrq);
    return 0;
}

static void mmc_wait_for_req_done(struct mmc_host *host,
        struct mmc_request *mrq)
{
    struct mmc_command *cmd;

    while (1) {
#if 1
        if (wait_for_completion_timeout(&mrq->completion, 4 * 1000) == 0) {
            printf("%s: timeout!! cmd:%d \n", __func__, mrq->cmd->opcode);
            printf(" (%d, %d) \n", (4 * 1000), (mrq->data->timeout_ns / 1000000));
        }
		
#else
        wait_for_completion(&mrq->completion);
#endif

        cmd = mrq->cmd;

        /*
        * If host has timed out waiting for the sanitize
        * to complete, card might be still in programming state
        * so let's try to bring the card out of programming
        * state.
        */
#if 0
        if (cmd->sanitize_busy && cmd->error == -ETIMEDOUT) {
            if (!mmc_interrupt_hpi(host->card)) {
                pr_warn("%s: %s: Interrupted sanitize\n",
                    mmc_hostname(host), __func__);
                cmd->error = 0;
                break;
			} else {
                pr_err("%s: %s: Failed to interrupt sanitize\n",
                    mmc_hostname(host), __func__);
            }
        }
#endif
        if (!cmd->error || !cmd->retries ||
            mmc_card_removed(host->card))
            break;

        pr_debug("%s: req failed (CMD%u): %d, retrying...\n",
            mmc_hostname(host), cmd->opcode, cmd->error);
        cmd->retries--;
        cmd->error = 0;
        host->ops->request(host, mrq);
    }
    destroy_completion(&mrq->completion);
}

/**
 *	mmc_wait_for_req - start a request and wait for completion
 *	@host: MMC host to start command
 *	@mrq: MMC request to start
 *
 *	Start a new MMC custom command request for a host, and wait
 *	for the command to complete. Does not attempt to parse the
 *	response.
 */
void mmc_wait_for_req(struct mmc_host *host, struct mmc_request *mrq)
{
    __mmc_start_req(host, mrq);
    mmc_wait_for_req_done(host, mrq);
}

/**
 *	mmc_wait_for_cmd - start a command and wait for completion
 *	@host: MMC host to start command
 *	@cmd: MMC command to start
 *	@retries: maximum number of retries
 *
 *	Start a new MMC command for a host, and wait for the command
 *	to complete.  Return any error that occurred while the command
 *	was executing.  Do not attempt to parse the response.
 */
int mmc_wait_for_cmd(struct mmc_host *host, struct mmc_command *cmd, int retries)
{
	struct mmc_request mrq = {NULL};

    WARN_ON(!host->claimed);

    memset(cmd->resp, 0, sizeof(cmd->resp));
    cmd->retries = retries;

    mrq.cmd = cmd;
    cmd->data = NULL;

    mmc_wait_for_req(host, &mrq);

    return cmd->error;
}

/**
 *	mmc_set_data_timeout - set the timeout for a data command
 *	@data: data phase for command
 *	@card: the MMC card associated with the data transfer
 *
 *	Computes the data timeout parameters according to the
 *	correct algorithm given the card type.
 */
void mmc_set_data_timeout(struct mmc_data *data, const struct mmc_card *card)
{
    unsigned int mult;

    /*
    * SDIO cards only define an upper 1 s limit on access.
    */
    if (mmc_card_sdio(card)) {
        data->timeout_ns = 1000000000;
        data->timeout_clks = 0;
        return;
    }

    /*
    * SD cards use a 100 multiplier rather than 10
    */
    mult = mmc_card_sd(card) ? 100 : 10;

    /*
    * Scale up the multiplier (and therefore the timeout) by
    * the r2w factor for writes.
    */
    if (data->flags & MMC_DATA_WRITE)
        mult <<= card->csd.r2w_factor;

    data->timeout_ns = card->csd.tacc_ns * mult;
    data->timeout_clks = card->csd.tacc_clks * mult;

    /*
    * SD cards also have an upper limit on the timeout.
    */
    if (mmc_card_sd(card)) {
        unsigned int timeout_us, limit_us;

        timeout_us = data->timeout_ns / 1000;
        if (mmc_host_clk_rate(card->host))
            timeout_us += data->timeout_clks * 1000 /
            (mmc_host_clk_rate(card->host) / 1000);

        if (data->flags & MMC_DATA_WRITE)
            /*
            * The MMC spec "It is strongly recommended
            * for hosts to implement more than 500ms
            * timeout value even if the card indicates
            * the 250ms maximum busy length."  Even the
            * previous value of 300ms is known to be
            * insufficient for some cards.
            */
            limit_us = 3000000;
        else
            limit_us = 100000;

        /*
        * SDHC cards always use these fixed values.
        */
        if (timeout_us > limit_us || mmc_card_blockaddr(card)) {
            data->timeout_ns = limit_us * 1000;
            data->timeout_clks = 0;
        }

        /* assign limit value if invalid */
        if (timeout_us == 0)
            data->timeout_ns = limit_us * 1000;
    }

    /*
    * Some cards require longer data read timeout than indicated in CSD.
    * Address this by setting the read timeout to a "reasonably high"
    * value. For the cards tested, 300ms has proven enough. If necessary,
    * this value can be increased if other problematic cards require this.
    */
    if (mmc_card_long_read_time(card) && data->flags & MMC_DATA_READ) {
        #if 1 // Irene Lin
        data->timeout_ns = 2300000000;
        #else
        data->timeout_ns = 300000000;
        #endif
        data->timeout_clks = 0;
    }

    /*
    * Some cards need very high timeouts if driven in SPI mode.
    * The worst observed timeout was 900ms after writing a
    * continuous stream of data until the internal logic
    * overflowed.
    */
    if (mmc_host_is_spi(card->host)) {
        if (data->flags & MMC_DATA_WRITE) {
            if (data->timeout_ns < 1000000000)
                data->timeout_ns = 1000000000;	/* 1s */
		} else {
            if (data->timeout_ns < 100000000)
                data->timeout_ns = 100000000;	/* 100ms */
        }
    }
}

/**
*	mmc_align_data_size - pads a transfer size to a more optimal value
*	@card: the MMC card associated with the data transfer
*	@sz: original transfer size
*
*	Pads the original data size with a number of extra bytes in
*	order to avoid controller bugs and/or performance hits
*	(e.g. some controllers revert to PIO for certain sizes).
*
*	Returns the improved size, which might be unmodified.
*
*	Note that this function is only relevant when issuing a
*	single scatter gather entry.
*/
unsigned int mmc_align_data_size(struct mmc_card *card, unsigned int sz)
{
    /*
    * FIXME: We don't have a system for the controller to tell
    * the core about its problems yet, so for now we just 32-bit
    * align the size.
    */
    sz = ((sz + 3) / 4) * 4;

    return sz;
}

/**
 *	__mmc_claim_host - exclusively claim a host
 *	@host: mmc host to claim
 *	@abort: whether or not the operation should be aborted
 *
 *	Claim a host for a set of operations.  If @abort is non null and
 *	dereference a non-zero value then this will return prematurely with
 *	that non-zero value without acquiring the lock.  Returns zero
 *	with the lock held otherwise.
 */
int __mmc_claim_host(struct mmc_host *host, atomic_t *abort)
{
	host->ops->lock(host);
    host->claimed = 1;
    return 0;
}

/**
 *	mmc_release_host - release a host
 *	@host: mmc host to release
 *
 *	Release a MMC host, allowing others to claim the host
 *	for their operations.
 */
void mmc_release_host(struct mmc_host *host)
{
    host->claimed = 0;
	host->ops->unlock(host);
}

/*
 * This is a helper function, which fetches a runtime pm reference for the
 * card device and also claims the host.
 */
void mmc_get_card(struct mmc_card *card)
{
    mmc_claim_host(card->host);
}

/*
 * This is a helper function, which releases the host and drops the runtime
 * pm reference for the card device.
 */
void mmc_put_card(struct mmc_card *card)
{
    mmc_release_host(card->host);
}


/*
 * Internal function that does the actual ios call to the host driver,
 * optionally printing some debug output.
 */
static inline void mmc_set_ios(struct mmc_host *host)
{
	struct mmc_ios *ios = &host->ios;

	pr_debug("%s: clock %uHz busmode %u powermode %u cs %u Vdd %u "
		"width %u timing %u\n",
		 mmc_hostname(host), ios->clock, ios->bus_mode,
		 ios->power_mode, ios->chip_select, ios->vdd,
		 ios->bus_width, ios->timing);

	if (ios->clock > 0)
		mmc_set_ungated(host);
	host->ops->set_ios(host, ios);
}

/*
 * Control chip select pin on a host.
 */
void mmc_set_chip_select(struct mmc_host *host, int mode)
{
    mmc_host_clk_hold(host);
    host->ios.chip_select = mode;
    mmc_set_ios(host);
    mmc_host_clk_release(host);
}

/*
 * Sets the host clock to the highest possible frequency that
 * is below "hz".
 */
static void __mmc_set_clock(struct mmc_host *host, unsigned int hz)
{
    WARN_ON(hz && hz < host->f_min);

    if (hz > host->f_max)
        hz = host->f_max;

    host->ios.clock = hz;
    mmc_set_ios(host);
}

void mmc_set_clock(struct mmc_host *host, unsigned int hz)
{
    mmc_host_clk_hold(host);
    __mmc_set_clock(host, hz);
    mmc_host_clk_release(host);
}

#ifdef CONFIG_MMC_CLKGATE

#else
void mmc_set_ungated(struct mmc_host *host)
{
}
#endif

/*
 * Change the bus mode (open drain/push-pull) of a host.
 */
void mmc_set_bus_mode(struct mmc_host *host, unsigned int mode)
{
    mmc_host_clk_hold(host);
    host->ios.bus_mode = mode;
    mmc_set_ios(host);
    mmc_host_clk_release(host);
}

/*
 * Change data bus width of a host.
 */
void mmc_set_bus_width(struct mmc_host *host, unsigned int width)
{
    mmc_host_clk_hold(host);
    host->ios.bus_width = width;
    mmc_set_ios(host);
    mmc_host_clk_release(host);
}

/*
 * Set initial state after a power cycle or a hw_reset.
 */
void mmc_set_initial_state(struct mmc_host *host)
{
    if (mmc_host_is_spi(host))
        host->ios.chip_select = MMC_CS_HIGH;
    else
        host->ios.chip_select = MMC_CS_DONTCARE;
    host->ios.bus_mode = MMC_BUSMODE_PUSHPULL;
    host->ios.bus_width = MMC_BUS_WIDTH_1;
    host->ios.timing = MMC_TIMING_LEGACY;

    mmc_set_ios(host);
}

int mmc_regulator_get_supply(struct mmc_host *mmc)
{
	struct device *dev = mmc_dev(mmc);
	int ret;

	mmc->supply.vmmc = ERR_PTR(-ENODEV);
	mmc->supply.vqmmc = ERR_PTR(-ENODEV);

	if (IS_ERR(mmc->supply.vmmc)) {
		if (PTR_ERR(mmc->supply.vmmc) == -EPROBE_DEFER)
			return -EPROBE_DEFER;
		dev_dbg(dev, "No vmmc regulator found\n");
	} else {
		ret = mmc_regulator_get_ocrmask(mmc->supply.vmmc);
		if (ret > 0)
			mmc->ocr_avail = ret;
		else
			dev_warn(dev, "Failed getting OCR mask: %d\n", ret);
	}

	if (IS_ERR(mmc->supply.vqmmc)) {
		if (PTR_ERR(mmc->supply.vqmmc) == -EPROBE_DEFER)
			return -EPROBE_DEFER;
		dev_dbg(dev, "No vqmmc regulator found\n");
	}

	return 0;
}
EXPORT_SYMBOL_GPL(mmc_regulator_get_supply);

/*
 * Mask off any voltages we don't support and select
 * the lowest voltage
 */
u32 mmc_select_voltage(struct mmc_host *host, u32 ocr)
{
	int bit;

	/*
	 * Sanity check the voltages that the card claims to
	 * support.
	 */
	if (ocr & 0x7F) {
		dev_warn(mmc_dev(host),
		"card claims to support voltages below defined range\n");
		ocr &= ~0x7F;
	}

	ocr &= host->ocr_avail;
	if (!ocr) {
		dev_warn(mmc_dev(host), "no support for card's volts\n");
		return 0;
	}

	if (host->caps2 & MMC_CAP2_FULL_PWR_CYCLE) {
		bit = ffs(ocr) - 1;
		ocr &= 3 << bit;
		mmc_power_cycle(host, ocr);
	} else {
		bit = fls(ocr) - 1;
		ocr &= 3 << bit;
		if (bit != host->ios.vdd)
			dev_warn(mmc_dev(host), "exceeding card's volts\n");
	}

	return ocr;
}

int __mmc_set_signal_voltage(struct mmc_host *host, int signal_voltage)
{
    int err = 0;
    int old_signal_voltage = host->ios.signal_voltage;

    host->ios.signal_voltage = signal_voltage;
    if (host->ops->start_signal_voltage_switch) {
        mmc_host_clk_hold(host);
        err = host->ops->start_signal_voltage_switch(host, &host->ios);
        mmc_host_clk_release(host);
    }

    if (err)
        host->ios.signal_voltage = old_signal_voltage;

    return err;

}

int mmc_set_signal_voltage(struct mmc_host *host, int signal_voltage, u32 ocr)
{
	struct mmc_command cmd = {0};
    int err = 0;
    u32 clock;

    BUG_ON(!host);

    /*
    * Send CMD11 only if the request is to switch the card to
    * 1.8V signalling.
    */
    if (signal_voltage == MMC_SIGNAL_VOLTAGE_330)
        return __mmc_set_signal_voltage(host, signal_voltage);

    /*
    * If we cannot switch voltages, return failure so the caller
    * can continue without UHS mode
    */
    if (!host->ops->start_signal_voltage_switch)
        return -EPERM;
    if (!host->ops->card_busy)
        pr_warn("%s: cannot verify signal voltage switch\n",
        mmc_hostname(host));

    mmc_host_clk_hold(host);

    cmd.opcode = SD_SWITCH_VOLTAGE;
    cmd.arg = 0;
    cmd.flags = MMC_RSP_R1 | MMC_CMD_AC;

    err = mmc_wait_for_cmd(host, &cmd, 0);
    if (err)
        goto err_command;

    if (!mmc_host_is_spi(host) && (cmd.resp[0] & R1_ERROR)) {
        err = -EIO;
        goto err_command;
    }
    /*
    * The card should drive cmd and dat[0:3] low immediately
    * after the response of cmd11, but wait 1 ms to be sure
    */
    mmc_delay(1);
    if (host->ops->card_busy && !host->ops->card_busy(host)) {
        err = -EAGAIN;
        goto power_cycle;
    }
    /*
    * During a signal voltage level switch, the clock must be gated
    * for 5 ms according to the SD spec
    */
    clock = host->ios.clock;
    host->ios.clock = 0;
    mmc_set_ios(host);

    if (__mmc_set_signal_voltage(host, signal_voltage)) {
        /*
        * Voltages may not have been switched, but we've already
        * sent CMD11, so a power cycle is required anyway
        */
        err = -EAGAIN;
        goto power_cycle;
    }

    /* Keep clock gated for at least 5 ms */
    mmc_delay(5);
    host->ios.clock = clock;
    mmc_set_ios(host);

    /* Wait for at least 1 ms according to spec */
    mmc_delay(1);

    /*
    * Failure to switch is indicated by the card holding
    * dat[0:3] low
    */
    if (host->ops->card_busy && host->ops->card_busy(host))
        err = -EAGAIN;

power_cycle:
    if (err) {
        pr_debug("%s: Signal voltage switch failed, "
            "power cycling card\n", mmc_hostname(host));
        mmc_power_cycle(host, ocr);
    }

err_command:
    mmc_host_clk_release(host);

    return err;
}

/*
 * Select timing parameters for host.
 */
void mmc_set_timing(struct mmc_host *host, unsigned int timing)
{
	mmc_host_clk_hold(host);
	host->ios.timing = timing;
	mmc_set_ios(host);
	mmc_host_clk_release(host);
}

/*
 * Select appropriate driver type for host.
 */
void mmc_set_driver_type(struct mmc_host *host, unsigned int drv_type)
{
	mmc_host_clk_hold(host);
	host->ios.drv_type = drv_type;
	mmc_set_ios(host);
	mmc_host_clk_release(host);
}

/*
 * Apply power to the MMC stack.  This is a two-stage process.
 * First, we enable power to the card without the clock running.
 * We then wait a bit for the power to stabilise.  Finally,
 * enable the bus drivers and clock to the card.
 *
 * We must _NOT_ enable the clock prior to power stablising.
 *
 * If a host does all the power sequencing itself, ignore the
 * initial MMC_POWER_UP stage.
 */
void mmc_power_up(struct mmc_host *host, u32 ocr)
{
	if (host->ios.power_mode == MMC_POWER_ON)
		return;

	mmc_host_clk_hold(host);

	host->ios.vdd = fls(ocr) - 1;
	host->ios.power_mode = MMC_POWER_UP;
	/* Set initial state and call mmc_set_ios */
	mmc_set_initial_state(host);

	/* Try to set signal voltage to 3.3V but fall back to 1.8v or 1.2v */
	if (__mmc_set_signal_voltage(host, MMC_SIGNAL_VOLTAGE_330) == 0)
		dev_dbg(mmc_dev(host), "Initial signal voltage of 3.3v\n");
	else if (__mmc_set_signal_voltage(host, MMC_SIGNAL_VOLTAGE_180) == 0)
		dev_dbg(mmc_dev(host), "Initial signal voltage of 1.8v\n");
	else if (__mmc_set_signal_voltage(host, MMC_SIGNAL_VOLTAGE_120) == 0)
		dev_dbg(mmc_dev(host), "Initial signal voltage of 1.2v\n");

	/*
	 * This delay should be sufficient to allow the power supply
	 * to reach the minimum voltage.
	 */
	mmc_delay(10);

	host->ios.clock = host->f_init;

	host->ios.power_mode = MMC_POWER_ON;
	mmc_set_ios(host);

	/*
	 * This delay must be at least 74 clock sizes, or 1 ms, or the
	 * time required to reach a stable voltage.
	 */
	mmc_delay(10);

	mmc_host_clk_release(host);
}

void mmc_power_off(struct mmc_host *host)
{
    if (host->ios.power_mode == MMC_POWER_OFF)
        return;

    mmc_host_clk_hold(host);

    host->ios.clock = 0;
    host->ios.vdd = 0;

    host->ios.power_mode = MMC_POWER_OFF;
    /* Set initial state and call mmc_set_ios */
    mmc_set_initial_state(host);

    /*
    * Some configurations, such as the 802.11 SDIO card in the OLPC
    * XO-1.5, require a short delay after poweroff before the card
    * can be successfully turned on again.
    */
    mmc_delay(1);

    mmc_host_clk_release(host);
}

void mmc_power_cycle(struct mmc_host *host, u32 ocr)
{
	mmc_power_off(host);
	/* Wait at least 1 ms according to SD spec */
	mmc_delay(1);
	mmc_power_up(host, ocr);
}

/*
 * Cleanup when the last reference to the bus operator is dropped.
 */
static void __mmc_release_bus(struct mmc_host *host)
{
    BUG_ON(!host);
    BUG_ON(host->bus_refs);
    BUG_ON(!host->bus_dead);

    host->bus_ops = NULL;
}

/*
 * Increase reference count of bus operator
 */
static inline void mmc_bus_get(struct mmc_host *host)
{
    unsigned long flags;

    spin_lock_irqsave(&host->lock, flags);
    host->bus_refs++;
    spin_unlock_irqrestore(&host->lock, flags);
}

/*
 * Decrease reference count of bus operator and free it if
 * it is the last reference.
 */
static inline void mmc_bus_put(struct mmc_host *host)
{
    unsigned long flags;

    spin_lock_irqsave(&host->lock, flags);
    host->bus_refs--;
    if ((host->bus_refs == 0) && host->bus_ops)
        __mmc_release_bus(host);
    spin_unlock_irqrestore(&host->lock, flags);
}

/*
 * Assign a mmc bus handler to a host. Only one bus handler may control a
 * host at any given time.
 */
void mmc_attach_bus(struct mmc_host *host, const struct mmc_bus_ops *ops)
{
    unsigned long flags;

    BUG_ON(!host);
    BUG_ON(!ops);

    WARN_ON(!host->claimed);

    spin_lock_irqsave(&host->lock, flags);

    BUG_ON(host->bus_ops);
    BUG_ON(host->bus_refs);

    host->bus_ops = ops;
    host->bus_refs = 1;
    host->bus_dead = 0;

    spin_unlock_irqrestore(&host->lock, flags);
}

/*
 * Remove the current bus handler from a host.
 */
void mmc_detach_bus(struct mmc_host *host)
{
    unsigned long flags;

    BUG_ON(!host);

    WARN_ON(!host->claimed);
    WARN_ON(!host->bus_ops);

    spin_lock_irqsave(&host->lock, flags);

    host->bus_dead = 1;

    spin_unlock_irqrestore(&host->lock, flags);

    mmc_bus_put(host);
}

void mmc_rescan(struct mmc_host *host);

static void _mmc_detect_change(struct mmc_host *host, unsigned long delay,
    bool cd_irq)
{
    host->detect_change = 1;
    if (delay > 0)
        usleep(delay * 1000);
    mmc_rescan(host);
}

/**
 *	mmc_detect_change - process change of state on a MMC socket
 *	@host: host which changed state.
 *	@delay: optional delay to wait before detection (jiffies)
 *
 *	MMC drivers should call this when they detect a card has been
 *	inserted or removed. The MMC layer will confirm that any
 *	present card is still functional, and initialize any newly
 *	inserted.
 */
void mmc_detect_change(struct mmc_host *host, unsigned long delay)
{
    _mmc_detect_change(host, delay, true);
}

void mmc_init_erase(struct mmc_card *card)
{
	unsigned int sz;

	if (is_power_of_2(card->erase_size))
		card->erase_shift = ffs(card->erase_size) - 1;
	else
		card->erase_shift = 0;

	/*
	 * It is possible to erase an arbitrarily large area of an SD or MMC
	 * card.  That is not desirable because it can take a long time
	 * (minutes) potentially delaying more important I/O, and also the
	 * timeout calculations become increasingly hugely over-estimated.
	 * Consequently, 'pref_erase' is defined as a guide to limit erases
	 * to that size and alignment.
	 *
	 * For SD cards that define Allocation Unit size, limit erases to one
	 * Allocation Unit at a time.  For MMC cards that define High Capacity
	 * Erase Size, whether it is switched on or not, limit to that size.
	 * Otherwise just have a stab at a good value.  For modern cards it
	 * will end up being 4MiB.  Note that if the value is too small, it
	 * can end up taking longer to erase.
	 */
	if (mmc_card_sd(card) && card->ssr.au) {
		card->pref_erase = card->ssr.au;
		card->erase_shift = ffs(card->ssr.au) - 1;
	} else if (card->ext_csd.hc_erase_size) {
		card->pref_erase = card->ext_csd.hc_erase_size;
	} else if (card->erase_size) {
		sz = (card->csd.capacity << (card->csd.read_blkbits - 9)) >> 11;
		if (sz < 128)
			card->pref_erase = 512 * 1024 / 512;
		else if (sz < 512)
			card->pref_erase = 1024 * 1024 / 512;
		else if (sz < 1024)
			card->pref_erase = 2 * 1024 * 1024 / 512;
		else
			card->pref_erase = 4 * 1024 * 1024 / 512;
		if (card->pref_erase < card->erase_size)
			card->pref_erase = card->erase_size;
		else {
			sz = card->pref_erase % card->erase_size;
			if (sz)
				card->pref_erase += card->erase_size - sz;
		}
	} else
		card->pref_erase = 0;
}

int mmc_can_erase(struct mmc_card *card)
{
	if ((card->host->caps & MMC_CAP_ERASE) &&
	    (card->csd.cmdclass & CCC_ERASE) && card->erase_size)
		return 1;
	return 0;
}

int mmc_can_trim(struct mmc_card *card)
{
	if (card->ext_csd.sec_feature_support & EXT_CSD_SEC_GB_CL_EN)
		return 1;
	return 0;
}

int mmc_can_discard(struct mmc_card *card)
{
	/*
	 * As there's no way to detect the discard support bit at v4.5
	 * use the s/w feature support filed.
	 */
	if (card->ext_csd.feature_support & MMC_DISCARD_FEATURE)
		return 1;
	return 0;
}

int mmc_can_sanitize(struct mmc_card *card)
{
	if (!mmc_can_trim(card) && !mmc_can_erase(card))
		return 0;
	if (card->ext_csd.sec_feature_support & EXT_CSD_SEC_SANITIZE)
		return 1;
	return 0;
}

int mmc_can_secure_erase_trim(struct mmc_card *card)
{
	if ((card->ext_csd.sec_feature_support & EXT_CSD_SEC_ER_EN) &&
	    !(card->quirks & MMC_QUIRK_SEC_ERASE_TRIM_BROKEN))
		return 1;
	return 0;
}

int mmc_erase_group_aligned(struct mmc_card *card, unsigned int from,
			    unsigned int nr)
{
	if (!card->erase_size)
		return 0;
	if (from % card->erase_size || nr % card->erase_size)
		return 0;
	return 1;
}

int mmc_set_blocklen(struct mmc_card *card, unsigned int blocklen)
{
	struct mmc_command cmd = {0};

	if (mmc_card_blockaddr(card) || mmc_card_ddr52(card))
		return 0;

	cmd.opcode = MMC_SET_BLOCKLEN;
	cmd.arg = blocklen;
	cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_AC;
	return mmc_wait_for_cmd(card->host, &cmd, 5);
}

int mmc_set_blockcount(struct mmc_card *card, unsigned int blockcount,
			bool is_rel_write)
{
	struct mmc_command cmd = {0};

	cmd.opcode = MMC_SET_BLOCK_COUNT;
	cmd.arg = blockcount & 0x0000FFFF;
	if (is_rel_write)
		cmd.arg |= 1 << 31;
	cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_AC;
	return mmc_wait_for_cmd(card->host, &cmd, 5);
}

static void mmc_hw_reset_for_init(struct mmc_host *host)
{
	if (!(host->caps & MMC_CAP_HW_RESET) || !host->ops->hw_reset)
		return;
	mmc_host_clk_hold(host);
	host->ops->hw_reset(host);
	mmc_host_clk_release(host);
}

int mmc_can_reset(struct mmc_card *card)
{
	u8 rst_n_function;

	if (!mmc_card_mmc(card))
		return 0;
	rst_n_function = card->ext_csd.rst_n_function;
	if ((rst_n_function & EXT_CSD_RST_N_EN_MASK) != EXT_CSD_RST_N_ENABLED)
		return 0;
	return 1;
}

static int mmc_rescan_try_freq(struct mmc_host *host, unsigned freq)
{
	host->f_init = freq;

#ifdef CONFIG_MMC_DEBUG
	pr_info("%s: %s: trying to init card at %u Hz\n",
		mmc_hostname(host), __func__, host->f_init);
#endif
	mmc_power_up(host, host->ocr_avail);

	/*
	 * Some eMMCs (with VCCQ always on) may not be reset after power up, so
	 * do a hardware reset if possible.
	 */
	mmc_hw_reset_for_init(host);

#if defined(CFG_SDIO_ENABLE)
	/*
	 * sdio_reset sends CMD52 to reset card.  Since we do not know
	 * if the card is being re-initialized, just send it.  CMD52
	 * should be ignored by SD/eMMC cards.
	 */
	sdio_reset(host);
#endif

	mmc_go_idle(host);

	mmc_send_if_cond(host, host->ocr_avail);

	/* Order's important: probe SDIO, then SD, then MMC */
#if defined(CFG_SDIO_ENABLE)
	if (!mmc_attach_sdio(host))
		return 0;
#endif
	if (!mmc_attach_sd(host))
		return 0;
	if (!mmc_attach_mmc(host))
		return 0;

	mmc_power_off(host);
	return -EIO;
}

int _mmc_detect_card_removed(struct mmc_host *host)
{
	int ret;

	if (host->caps & MMC_CAP_NONREMOVABLE)
		return 0;

	if (!host->card || mmc_card_removed(host->card))
		return 1;

	ret = host->bus_ops->alive(host);

	/*
	 * Card detect status and alive check may be out of sync if card is
	 * removed slowly, when card detect switch changes while card/slot
	 * pads are still contacted in hardware (refer to "SD Card Mechanical
	 * Addendum, Appendix C: Card Detection Switch"). So reschedule a
	 * detect work 200ms later for this case.
	 */
	if (!ret && host->ops->get_cd && !host->ops->get_cd(host)) {
		mmc_detect_change(host, msecs_to_jiffies(200));
		pr_debug("%s: card removed too slowly\n", mmc_hostname(host));
	}

	if (ret) {
		mmc_card_set_removed(host->card);
		pr_debug("%s: card remove detected\n", mmc_hostname(host));
	}

	return ret;
}

int mmc_detect_card_removed(struct mmc_host *host)
{
	struct mmc_card *card = host->card;
	int ret;

	WARN_ON(!host->claimed);

	if (!card)
		return 1;

	ret = mmc_card_removed(card);
	/*
	 * The card will be considered unchanged unless we have been asked to
	 * detect a change or host requires polling to provide card detection.
	 */
	if (!host->detect_change && !(host->caps & MMC_CAP_NEEDS_POLL))
		return ret;

	host->detect_change = 0;
	if (!ret) {
		ret = _mmc_detect_card_removed(host);
		if (ret && (host->caps & MMC_CAP_NEEDS_POLL)) {
			/*
			 * Schedule a detect work as soon as possible to let a
			 * rescan handle the card removal.
			 */
			//cancel_delayed_work(&host->detect);
			_mmc_detect_change(host, 0, false);
		}
	}

	return ret;
}

void mmc_rescan(struct mmc_host *host)
{
    int i;

    if (host->trigger_card_event && host->ops->card_event) {
        host->ops->card_event(host);
        host->trigger_card_event = false;
    }

    if (host->rescan_disable)
        return;

    /* If there is a non-removable card registered, only scan once */
    if ((host->caps & MMC_CAP_NONREMOVABLE) && host->rescan_entered)
        return;
    host->rescan_entered = 1;

    mmc_bus_get(host);

    /*
    * if there is a _removable_ card registered, check whether it is
    * still present
    */
    if (host->bus_ops && !host->bus_dead
        && !(host->caps & MMC_CAP_NONREMOVABLE))
        host->bus_ops->detect(host);

    host->detect_change = 0;

    /*
    * Let mmc_bus_put() free the bus/bus_ops if we've found that
    * the card is no longer present.
    */
    mmc_bus_put(host);
    mmc_bus_get(host);

    /* if there still is a card present, stop here */
    if (host->bus_ops != NULL) {
        mmc_bus_put(host);
        goto out;
    }

    /*
    * Only we can add a new handler, so it's safe to
    * release the lock here.
    */
    mmc_bus_put(host);

    if (!(host->caps & MMC_CAP_NONREMOVABLE) && host->ops->get_cd &&
        host->ops->get_cd(host) == 0) {
        mmc_claim_host(host);
        mmc_power_off(host);
        mmc_release_host(host);
        goto out;
    }

    mmc_claim_host(host);
    for (i = 0; i < ARRAY_SIZE(freqs); i++) {
        if (!mmc_rescan_try_freq(host, max(freqs[i], host->f_min)))
            break;
        if (freqs[i] <= host->f_min)
            break;
    }
    mmc_release_host(host);

out:
#if 1
    return;
#else
	if (host->caps & MMC_CAP_NEEDS_POLL)
		mmc_schedule_delayed_work(&host->detect, HZ);
#endif
}

#define mmc_gpiod_request_cd_irq(x)

void mmc_start_host(struct mmc_host *host)
{
    if (host->ios.power_mode == MMC_POWER_ON) {
        /* do nothing */
        host->rescan_disable = 0;
    }
    else {
        host->f_init = max(freqs[0], host->f_min);
        host->rescan_disable = 0;
        host->ios.power_mode = MMC_POWER_UNDEFINED;
        if (host->caps2 & MMC_CAP2_NO_PRESCAN_POWERUP)
            mmc_power_off(host);
        else
            mmc_power_up(host, host->ocr_avail);
    }
	mmc_gpiod_request_cd_irq(host);
	_mmc_detect_change(host, 0, false);
}

void mmc_stop_host(struct mmc_host *host)
{
	host->rescan_disable = 1;

	mmc_bus_get(host);
	if (host->bus_ops && !host->bus_dead) {
		/* Calling bus_ops->remove() with a claimed host can deadlock */
		host->bus_ops->remove(host);
		mmc_claim_host(host);
		mmc_detach_bus(host);
		mmc_power_off(host);
		mmc_release_host(host);
		mmc_bus_put(host);
		return;
	}
	mmc_bus_put(host);

	BUG_ON(host->card);

	mmc_power_off(host);
}
