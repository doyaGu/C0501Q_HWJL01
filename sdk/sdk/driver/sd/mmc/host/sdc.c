
#define SD_READ_FLIP_FLOP

/* Hardware default design will halt and stop the controller when DATA CRC occur.
   But we don't want to halt the controller, but just want to know the crc status.
   So we should always enable this bit to avoid controller halt. */
#define SD_CrcBypassReg()           ithWriteRegMaskA(SD_REG_STS0, SD_MSK_CRC_BYPASS, SD_MSK_CRC_BYPASS)
/* Hardware design issue for DATA CRC.
   Original design is receive crc in 2-T clock. But it's wrong.
   Set this bit as 1 to fix this wrong design. We should always enable this bit. */
#define SD_CrcNonFixReg(x)          ithWriteRegMaskA(SD_REG_STS0, (x==true)?SD_MSK_CRC_NON_FIX:0x0, SD_MSK_CRC_NON_FIX)
#define SD_SetClockDivReg(clkDiv)   ithWriteRegA(SD_REG_CLK_DIV, (clkDiv<<2))
#define SD_SetBusWidth(value)       ithWriteRegMaskA(SD_REG_STS0, value, SD_MSK_BUS_WIDTH)
#define SD_SetRespTimeoutReg(tick)  ithWriteRegA(SD_REG_RESP_TIMEOUT, tick)
#define SD_ReadDataReg()            ithReadRegA(SDW_REG_DATA_PORT)
#define SD_WriteDataReg(data)       ithWriteRegA(SDW_REG_DATA_PORT, data)
#define SD_FifoEmpty()              (ithReadRegA(SDW_REG_WRAP_STATUS) & SDW_MSK_FIFO_EMPTY)
#define SD_FifoCount()              ((ithReadRegA(SDW_REG_WRAP_STATUS) >> 4) & 0xFF)
#define SD_WrapIntrDisable()        ithWriteRegMaskA(SDW_REG_WRAP_STATUS, SDW_MSK_INTR_MASK, SDW_MSK_INTR_MASK)
#define SD_WrapIntrEnable()         ithWriteRegMaskA(SDW_REG_WRAP_STATUS, 0x0, SDW_MSK_INTR_MASK)

#if (CFG_CHIP_FAMILY == 9910)
#define SD_EnableFlipFlop()         ithSetRegBitA(ITH_GPIO_BASE + ITH_GPIO_HTRAP_REG, 16)
#else
#define SD_EnableFlipFlop()         ithWriteRegMaskA(SD_REG_STS0, 0x8, 0x8)
#endif

#define SD_EnableSdioInterrupt()    ithWriteRegMaskA(SD_REG_INTR, 0x0, (SD_INTR_SDIO_MSK|SD_INTR_SDIO_CPU_MSK))
#define SD_DisableSdioInterrupt()   ithWriteRegMaskA(SD_REG_INTR, SD_INTR_SDIO_MSK, (SD_INTR_SDIO_MSK|SD_INTR_SDIO_CPU_MSK))


static inline void sdc_async_reset(void)
{
    /* async reset */
    ithSetRegBitH(ITH_APB_CLK4_REG, ITH_RST_SDIP_BIT);     /* 0x22 D[8] */
    usleep(50);
    ithClearRegBitH(ITH_APB_CLK4_REG, ITH_RST_SDIP_BIT);   /* 0x22 D[8] */
    usleep(50);

    /* programming some default setting */
#if defined(SD_READ_FLIP_FLOP)
    /** sd data in from flip-flop out */
    SD_EnableFlipFlop();
#endif

    /* some default setting */
    SD_CrcNonFixReg(true);   /* fix wrong data crc design */
    SD_CrcBypassReg();       /* we will not halt the controller when data crc occur */
    SD_SetRespTimeoutReg(64 * 2);
}

static inline void sdc_wrap_reset(void)
{
    ithWriteRegMaskA(SDW_REG_WRAP_CTRL, SDW_MSK_SW_RESET, SDW_MSK_SW_RESET);
    ithWriteRegMaskA(SDW_REG_WRAP_CTRL, 0x0, SDW_MSK_SW_RESET);
}

static inline uint32_t sdc_set_delay(unsigned int clock, int read)
{
    uint32_t timing;

#if defined(FPGA)
    return;
#endif
    if (clock <= 400000)
        timing = 0x0;
    else if (clock <= 25000000)
        timing = 0x0;
    else
        timing = 0x00000020;

    ithWriteRegMaskA((SD_BASE + 0x008C), timing, 0x010000FF);

    return timing;
}

static inline void sdc_gen_clock(void)
{
    ithWriteRegMaskA(SD_REG_CTL, SD_MSK_CLK_CTRL, SD_MSK_CLK_CTRL);
    /*
    * This delay must be at least 74 clock sizes, or 1 ms, or the
    * time required to reach a stable voltage.
    */
    usleep(10 * 1000);
    ithWriteRegMaskA(SD_REG_CTL, 0, SD_MSK_CLK_CTRL);
}

static inline unsigned int sdc_set_clock(struct sdc_host *host, unsigned int clock)
{
    unsigned int clk_div = 0;
    unsigned int delay_timing;

#if defined(FPGA)
    if (clock <= 400000)
        clk_div = 0x87;    /* 27M/135 = 200k */
    else if (clock < 25000000)
        clk_div = 1;       /* 27M/2 = 13.5M */
    else if (clock < 50000000)
        clk_div = 0;       /* 27M */
#else
    if (clock <= 400000)
        //clk_div = 200;      /* 80M/200 = 400k */
        clk_div = 0x200;    /* 80M/512 = 156k */
    else if (clock <= 25000000)
        clk_div = 3;        /* 80M/4 = 20M */
    else if (clock < 50000000)
        clk_div = 1;        /* 80M/2 = 40M */
#endif

    SD_SetClockDivReg(clk_div);
    delay_timing = sdc_set_delay(clock, 0);
    LOG_INFO "sdc: clock = %d, clk_div = %d (bus_clk %d), delay_timing = 0x%X \n", clock, clk_div, host->bus_clk, delay_timing LOG_END

    return clk_div;
}

static inline unsigned int sdc_set_bus_width(uint8_t busWidth)
{
    unsigned int value = 0;

    LOG_INFO "sdc: bus_width = 0x%x, \n", busWidth LOG_END

    switch (busWidth)
    {
    default:
    case MMC_BUS_WIDTH_1:
        value = SD_BUS_WIDTH_1BIT;
        break;
    case MMC_BUS_WIDTH_4:
        value = SD_BUS_WIDTH_4BIT;
        break;
    case MMC_BUS_WIDTH_8:
        value = SD_BUS_WIDTH_8BIT;
        break;
    }
    SD_SetBusWidth(value);

    return value;
}

static inline bool sdc_is_card_inserted(int index)
{
    return ithCardInserted((index == SD_0) ? ITH_CARDPIN_SD0 : ITH_CARDPIN_SD1);
}

static inline void sdc_power_reset(int index, int delay_ms)
{
    switch (index)
    {
    case SD_0:
        ithStorageUnSelect(ITH_STOR_SD);
        ithCardPowerOff(ITH_CARDPIN_SD0);
        usleep(delay_ms * 1000);
        ithCardPowerOn(ITH_CARDPIN_SD0);
        break;
    case SD_1:
        ithStorageUnSelect(ITH_STOR_SD1);
        ithCardPowerOff(ITH_CARDPIN_SD1);
        usleep(delay_ms * 1000);
        ithCardPowerOn(ITH_CARDPIN_SD1);
        break;
    }
}

static inline void sdc_io_select(int index)
{
#if (CFG_CHIP_FAMILY == 9910)
    /* 9910 - has two step */
    ithWriteRegMaskA(SD_REG_STS0, (index << 4), (1 << 4));
#endif

    ithStorageSelect((index == SD_0) ? ITH_STOR_SD : ITH_STOR_SD1);
}

static int sdc_init(struct sdc_host *host)
{
    if (!sdc_is_card_inserted(host->index))
        return ERR_SD_NO_CARD;

    /* move to top because power down will io unselect */
    sdc_power_reset(host->index, 30);
    //sdc_io_select(host->idx);

    /* reset */
    sdc_async_reset(); /* reset interface controller */
    sdc_wrap_reset();  /* reset ite wrapper */

    /* should select sd index again after reset */
    sdc_io_select(host->index);

    /* wait card power up */
    //usleep(250 * 1000);   // TODO

    /* clock delay */
    sdc_set_delay(host->clock, 0);
    sdc_gen_clock();

    return 0;
}

static void sdc_set_wrap_ctrl(struct sdc_host *host, struct mmc_data *data)
{
    uint32_t value = SDW_MSK_WRAP_FIRE;
    uint32_t mask = SDW_MSK_DATA_IN | SDW_MSK_HW_HANDSHAKING | SDW_MSK_WRAP_FIRE;

    if (data->flags & MMC_DATA_READ)
        value |= SDW_MSK_DATA_IN;

    if (host->dma)
        value |= SDW_MSK_HW_HANDSHAKING;
    else
        SD_WrapIntrDisable();

    ithWriteRegMaskA(SDW_REG_WRAP_CTRL, value, mask);
}

static void sdc_start_cmd(struct sdc_host *host, struct mmc_command *cmd)
{
    uint32_t err_bypass = 0;
    uint32_t ctrl = host->ctrl; 

    host->flags = host->flags_isr = 0;

    host->cmd = cmd;
    host->flags |= SDC_FLAG_INTR_CMD;

    /* bypass something */
    if (!(cmd->flags & MMC_RSP_CRC))
        err_bypass |= SD_MSK_RESP_CRC_BYPASS;
    if (!(cmd->flags & MMC_RSP_PRESENT))
        err_bypass |= SD_MSK_RESP_TIMEOUT_BYPASS;
    ithWriteRegA(SD_REG_STS1, err_bypass);

    /* control setting */
    if (host->data)
    {
        host->flags |= (SDC_FLAG_INTR_WRAP | SDC_FLAG_INTR_DMA);
        sdc_set_wrap_ctrl(host, host->data);  /* fire wrap */

        ctrl |= SD_MSK_AUTO_SWAP;  // ??? TODO
        if (host->data->flags & MMC_DATA_WRITE)
            ctrl |= SD_CMD_DATA_OUT;
        if (host->data->flags & MMC_DATA_READ)
            ctrl |= SD_CMD_DATA_IN;
    }
    ctrl |= SD_MSK_CMD_TRIGGER;

    if (cmd->flags & MMC_RSP_BUSY)
        ctrl |= SD_RESP_TYPE_48_BUSY;
    else if (cmd->flags & MMC_RSP_136)
        ctrl |= SD_RESP_TYPE_136;
    else if (cmd->flags & MMC_RSP_PRESENT)
        ctrl |= SD_RESP_TYPE_48;
    else
        ctrl |= SD_RESP_TYPE_NON;

#if defined(SD_IRQ_TIMEOUT)
    /* setup the timeout timer */
    if(ithGetCpuMode() == ITH_CPU_SYS) {
        if (xTimerChangePeriod(host->timer, host->timeout_ms / portTICK_PERIOD_MS, 10) != pdPASS)
            ithPrintf("[SD][ERR] %s: change timer period fail!\n", __func__);
    }
    else {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (xTimerChangePeriodFromISR(host->timer, host->timeout_ms / portTICK_PERIOD_MS, &xHigherPriorityTaskWoken) != pdPASS)
            ithPrintf("[SD][ERR] %s: change timer period fail!\n", __func__);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
#endif

    ithWriteRegA(SD_REG_COMMAND, cmd->opcode);
    ithWriteRegA(SD_REG_ARG0, cmd->arg & 0xFF);
    ithWriteRegA(SD_REG_ARG1, (cmd->arg >> 8) & 0xFF);
    ithWriteRegA(SD_REG_ARG2, (cmd->arg >> 16) & 0xFF);
    ithWriteRegA(SD_REG_ARG3, (cmd->arg >> 24) & 0xFF);
    if (0)//(cmd->opcode == 18)
    {
        ithPrintRegA(ITH_SD_BASE, 0x80);
        ithPrintRegA(ITH_SD_BASE + 0x84, 8);
        while (1);
    }

    ithWriteRegA(SD_REG_CTL, ctrl);
}

static void sdc_set_data_size(uint32_t blocks, uint32_t blksz)
{
    ithWriteRegA(SD_REG_LENGTH, (blksz-1));
    ithWriteRegA(SD_REG_SECTOR_COUNT_L, (uint32_t)(blocks & 0xFF));
    ithWriteRegA(SD_REG_SECTOR_COUNT_H, (uint32_t)((blocks >> 8) & 0xFF));
    /* for wrap */
    ithWriteRegA(SDW_REG_DATA_LEN, (blocks * blksz));
}

static void sdc_get_resp(struct sdc_host *host)
{
    struct mmc_command *cmd = host->cmd;
    uint32_t v1, v2, v3, v4, i;

    if (cmd->flags & MMC_RSP_PRESENT) {
        if (cmd->flags & MMC_RSP_136) {
            /* response type 2 */
            /* MMC R2 response bits  0 - 31  : MMC_RESP_0 
               MMC R2 response bits 32 - 63  : MMC_RESP_1
               MMC R2 response bits 64 - 95  : MMC_RESP_2
               MMC R2 response bits 96 - 127 : MMC_RESP_3 */
            for (i = 0; i <= 3; i++)
            {
                v1 = ithReadRegA((SD_REG_RESP_7_0 + (i * 4) * 4)) & 0xFF;
                v2 = ithReadRegA((SD_REG_RESP_7_0 + (i * 4 + 1) * 4)) & 0xFF;
                v3 = ithReadRegA((SD_REG_RESP_7_0 + (i * 4 + 2) * 4)) & 0xFF;
                v4 = ithReadRegA((SD_REG_RESP_7_0 + (i * 4 + 3) * 4)) & 0xFF;
                cmd->resp[3-i] = v1 | (v2 << 8) | (v3 << 16) | (v4 << 24);
            }
        }
        else {
            /* response types 1, 1b, 3, 4, 5, 6 */
            /* MMC response : MMC_RESP_0 */
            v1 = ithReadRegA(SD_REG_RESP_15_8) & 0xFF;
            v2 = ithReadRegA(SD_REG_RESP_23_16) & 0xFF;
            v3 = ithReadRegA(SD_REG_RESP_31_24) & 0xFF;
            v4 = ithReadRegA(SD_REG_RESP_39_32) & 0xFF;
            cmd->resp[0] = (v4 << 24) | (v3 << 16) | (v2 << 8) | v1;
        }
    }
}

static void sdc_error_check(struct sdc_host *host)
{
    struct mmc_command *cmd = host->cmd;
    struct mmc_data *data = host->data;
    uint32_t err = ithReadRegA(SD_REG_STS1) & SD_ERROR;

    if (err & SD_MSK_RESP_TIMEOUT) {
        ithPrintf("[SD][ERR] Resp timeout! cmd:%d (%d) \n", cmd->opcode, host->index);
        cmd->error = -ETIMEDOUT;
    }
    if (err & SD_MSK_RESP_CRC) {
        ithPrintf("[SD][ERR] Resp CRC error! cmd:%d (%d) \n", cmd->opcode, host->index);
        cmd->error = -EILSEQ;
    }

    if (!data)
        goto end;

    if (err & (SD_MSK_CRC_WRITE | SD_MSK_CRC_READ)) {
        ithPrintf("[SD][ERR] Data CRC error! cmd:%d (%d) \n", cmd->opcode, host->index);
        data->error = -EILSEQ;
    }
    if (!data->error)
        data->bytes_xfered = data->blocks * data->blksz;
    else
        data->bytes_xfered = 0;

end:
    return;
}

static void sdc_finish_request(struct sdc_host *host, struct mmc_request *mrq)
{
    host->mrq = NULL;
    host->cmd = NULL;
    host->data = NULL;
    mmc_request_done(host->mmc, mrq);
}

#define SD_WAIT_FIFO_TIMEOUT    1000

static int sdc_wait_fifo_full(void)
{
    uint32_t status;
    uint32_t timeout = SD_WAIT_FIFO_TIMEOUT;

    while (!((status = ithReadRegA(SDW_REG_WRAP_STATUS)) & SDW_MSK_FIFO_FULL))
    {
        usleep(1000);
        if (--timeout == 0)
            return ERR_SD_FIFO_FULL_TIMEOUT;
    }
    return 0;
}

static int sdc_wait_fifo_empty(void)
{
    uint32_t status;
    uint32_t timeout = SD_WAIT_FIFO_TIMEOUT;

    while (!((status = ithReadRegA(SDW_REG_WRAP_STATUS)) & SDW_MSK_FIFO_EMPTY))
    {
        usleep(1000);
        if (--timeout == 0)
            return ERR_SD_FIFO_EMPTY_TIMEOUT;
    }
    return 0;
}

static int sdc_wait_dma_ready(struct sdc_host *host, struct sdc *ctxt)
{
    int rc = 0;
    uint32_t retries = 100;
    int dma_ch = (host->data->flags & MMC_DATA_READ) ? ctxt->dma_rx : ctxt->dma_tx;

    while (ithDmaIsBusy(dma_ch) && --retries);

    if (retries)
        goto end;

    retries = 500; // ms
    while (ithDmaIsBusy(dma_ch))
    {
        usleep(1000);
        if (--retries == 0) {
            rc = ERR_SD_WAIT_DMA_TIMEOUT;
            goto end;
        }
        if (sdc_is_card_inserted(host->index) == false) {
            rc = ERR_SD_NO_CARD;
            goto end;
        }
    }

end:
    check_result(rc);
    return rc;
}

static int sdc_wait_wrap_ready(struct sdc_host *host)
{
    int rc = 0;
    uint32_t reg;
    uint32_t retries = 100;

    do {
        reg = ithReadRegA(SDW_REG_WRAP_CTRL);
    } while ((reg & SDW_MSK_WRAP_FIRE) && --retries);

    if (retries)
        goto end;

    retries = 500; // ms
    while ((reg = ithReadRegA(SDW_REG_WRAP_CTRL)) & SDW_MSK_WRAP_FIRE)
    {
        usleep(1000);
        if (--retries == 0) {
            rc = ERR_SD_WAIT_WRAP_TIMEOUT;
            goto end;
        }
        if (sdc_is_card_inserted(host->index) == false) {
            rc = ERR_SD_NO_CARD;
            goto end;
        }
    }

end:
    check_result(rc);
    return rc;
}

static int sdc_wait_sdc_ready(struct sdc_host *host)
{
    int rc = 0;
    uint32_t reg;
    uint32_t retries = 100;

    do {
        reg = ithReadRegA(SD_REG_CTL);
    } while ((reg & SD_MSK_CMD_TRIGGER) && --retries);

    if (retries)
        goto end;

    retries = 300; // ms
    while ((reg = ithReadRegA(SD_REG_CTL)) & SD_MSK_CMD_TRIGGER)
    {
        usleep(1000);
        if (--retries == 0) {
            rc = ERR_SD_WAIT_SDC_TIMEOUT;
            goto end;
        }
        if (sdc_is_card_inserted(host->index) == false) {
            rc = ERR_SD_NO_CARD;
            goto end;
        }
    }

end:
    sdc_get_resp(host);
    check_result(rc);
    return rc;
}

static int sdc_rw_data_pio(struct sdc_host *host, struct mmc_data *data)
{
    int rc = 0;
    uint32_t i, j;
    uint32_t total_size = data->blocks * data->blksz;
    uint32_t *u32_buf = data->buf;
    uint8_t *u8_buf = data->buf;
    uint32_t tmp_data;

    if (data->flags & MMC_DATA_READ) {
        for (i = 0; i < (total_size / SD_FIFO_SIZE); i++) {
            rc = sdc_wait_fifo_full();
            if (rc)
                goto end;

            for (j = 0; j < SD_FIFO_SIZE; j+=4, u32_buf++) {
                tmp_data = SD_ReadDataReg();
                *u32_buf = cpu_to_le32(tmp_data);
            }
        }

        /* for small data size */
        total_size = total_size % SD_FIFO_SIZE;
        if (total_size) {
            rc = sdc_wait_sdc_ready(host); /* need check controller ready first! */
            if (rc)
                goto end;
            for (j = 0; j < total_size; j += 4, u32_buf++) {
                tmp_data = SD_ReadDataReg();
                *u32_buf = cpu_to_le32(tmp_data);
            }
        }

        rc = sdc_wait_sdc_ready(host);
        if (rc)
            goto end;

        rc = sdc_wait_wrap_ready(host);
        if (rc)
            goto end;
    }
    else {
        for (i = 0; i < (total_size / SD_FIFO_SIZE); i++) {
            rc = sdc_wait_fifo_empty();
            if (rc)
                goto end;

            for (j = 0; j < SD_FIFO_SIZE; j += 4, u8_buf += 4) {
                tmp_data = (*(u8_buf + 3) << 24 |
                            *(u8_buf + 2) << 16 |
                            *(u8_buf + 1) << 8 |
                            *(u8_buf));
                SD_WriteDataReg(tmp_data);
            }
        }

        rc = sdc_wait_wrap_ready(host);
        if (rc)
            goto end;

        rc = sdc_wait_sdc_ready(host);
        if (rc)
            goto end;
    }

    sdc_error_check(host);

    if (host->mrq->stop)
    {
        host->data = NULL;
        sdc_start_cmd(host, host->mrq->stop);
        rc = sdc_wait_sdc_ready(host);
        if (rc)
            goto end;
        sdc_error_check(host);
    }
    sdc_finish_request(host, host->mrq);
    return 0;

end:
    sdc_error_check(host);
    check_result(rc);
    return rc;
}

static void sdc_setup_data_dma(struct sdc *ctxt, struct sdc_host *host, struct mmc_data *data)
{
    uint32_t total_size = data->blksz * data->blocks;
    uint32_t buf_addr = (uint32_t)data->buf;

    host->data = data;

#if defined(WIN32)
    data->tmp_buf = (void*)itpVmemAlloc(total_size);
    if (data->flags & MMC_DATA_WRITE)
        ithWriteVram((uint32_t)data->tmp_buf, data->buf, total_size);
    buf_addr = (uint32_t)data->tmp_buf;
#endif

    if (data->flags & MMC_DATA_READ) {
        ithDmaSetDstAddr(ctxt->dma_rx, (uint32_t)buf_addr);
        ithDmaSetDstParams(ctxt->dma_rx, dmaWidthMap[((uint32_t)buf_addr) & 0x3], ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
        ithDmaSetTxSize(ctxt->dma_rx, total_size);
        ithDmaStart(ctxt->dma_rx);
    }
    else {
        ithDmaSetSrcAddr(ctxt->dma_tx, (uint32_t)buf_addr);
        ithDmaSetSrcParams(ctxt->dma_tx, dmaWidthMap[((uint32_t)buf_addr) & 0x3], ITH_DMA_CTRL_INC, ITH_DMA_MASTER_0);
        ithDmaSetTxSize(ctxt->dma_tx, total_size);
        ithDmaStart(ctxt->dma_tx);
    }
}

static int sdc_rw_data_dma(struct sdc_host *host, struct sdc *ctxt)
{
    int rc;
    struct mmc_data *data = host->data;

    if (host->data->flags & MMC_DATA_READ) {
        rc = sdc_wait_sdc_ready(host);
        if (rc)
            goto end;

        rc = sdc_wait_wrap_ready(host);
        if (rc)
            goto end;

        rc = sdc_wait_dma_ready(host, ctxt);
        if (rc)
            goto end;
    }
    else {
        rc = sdc_wait_dma_ready(host, ctxt);
        if (rc)
            goto end;

        rc = sdc_wait_wrap_ready(host);
        if (rc)
            goto end;

        rc = sdc_wait_sdc_ready(host);
        if (rc)
            goto end;
    }

#if defined(WIN32)
    if (data->flags & MMC_DATA_READ)
        ithReadVram(data->buf, (uint32_t)data->tmp_buf, data->blksz*data->blocks);
    itpVmemFree((uint32_t)data->tmp_buf);
#else
    if (data->flags & MMC_DATA_READ)
        ithInvalidateDCacheRange(data->buf, data->blksz*data->blocks);
#endif

    sdc_error_check(host);

    if (host->mrq->stop) {
        host->data = NULL;
        sdc_start_cmd(host, host->mrq->stop);
        rc = sdc_wait_sdc_ready(host);
        if (rc)
            goto end;
        sdc_error_check(host);
    }
    sdc_finish_request(host, host->mrq);
    return 0;

end:
    sdc_error_check(host);
    check_result(rc);
    return rc;
}


#if defined(SD_IRQ_ENABLE)
static void sdc_timeout_handler(TimerHandle_t pxTimer)
{
    struct sdc_host *host;
    struct mmc_command *cmd;

    configASSERT( pxTimer );
    ithPrintf("[SDC][ERR] sdc_timeout_handler() \n");

    host = (struct sdc_host *)pvTimerGetTimerID( pxTimer );
    cmd = host->cmd;
    ithPrintf("[SDC][ERR] timeout:%d ms \n", host->timeout_ms);
    if(1) {
    	struct mmc_request *mrq = host->mrq;
			ithPrintf("[SDC][ERR] CMD%u arg %08x flags %08x\n",
				 mrq->cmd->opcode,
				 mrq->cmd->arg, mrq->cmd->flags);

			if (mrq->data) {
				ithPrintf("[SDC][ERR] blksz %d blocks %d flags %08x "
					"tsac %d ms nsac %d\n",
					mrq->data->blksz,
					mrq->data->blocks, mrq->data->flags,
					mrq->data->timeout_ns / 1000000,
					mrq->data->timeout_clks);
			}
		}

    if(host->flags_isr & SDC_FLAG_INTR_CMD)
        LOG_ERROR " SD end! \n" LOG_END
    if(host->flags_isr & SDC_FLAG_INTR_DMA)
        LOG_ERROR " DMA end! \n" LOG_END
    if(host->flags_isr & SDC_FLAG_INTR_WRAP)
        LOG_ERROR " WRAP end! \n" LOG_END

    LOG_ERROR " need check: " LOG_END
    if(host->flags & SDC_FLAG_INTR_CMD)
        printf("SD ");
    if(host->flags & SDC_FLAG_INTR_DMA)
        printf("DMA ");
    if(host->flags & SDC_FLAG_INTR_WRAP)
        printf("WRAP ");
    printf("\n");
	printf("0x%08X = 0x%08X \n", SD_REG_STS1, ithReadRegA(SD_REG_STS1));
	printf("0x%08X = 0x%08X \n", SD_REG_CTL, ithReadRegA(SD_REG_CTL));

    if(host->flags & SDC_FLAG_INTR_DMA) {
        ithDmaDumpReg((host->data->flags & MMC_DATA_READ) ? ctxt.dma_rx : ctxt.dma_tx);
		if (!(host->flags_isr & SDC_FLAG_INTR_DMA)) {
            ithDmaAbort((host->data->flags & MMC_DATA_READ) ? ctxt.dma_rx : ctxt.dma_tx);
            printf("Abort DMA! \n");
		}
    }

    sdc_error_check(host);
    if (!cmd->error)
        cmd->error = -ETIMEDOUT;

    sdc_finish_request(host, host->mrq);
}

static void sdc_irq_status(struct sdc_host *host)
{
    if ((host->flags & SDC_FLAG_INTR_MSK) == 
        (host->flags_isr & SDC_FLAG_INTR_MSK)) {
#if defined(SD_IRQ_TIMEOUT)
        /* stop the timeout timer */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if( xTimerStopFromISR( host->timer, &xHigherPriorityTaskWoken ) != pdPASS )
            ithPrintf("%s: stop timer fail!\n", __func__);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif
        sdc_error_check(host);

        if (host->mrq->stop && host->data) {
            host->data = NULL;
            sdc_start_cmd(host, host->mrq->stop);
        } else
            sdc_finish_request(host, host->mrq);
    }
}

static void dma_isr(int ch, void* arg, uint32_t status)
{
    struct sdc *ctxt = (struct sdc*)arg;
    struct sdc_host *host = ctxt->sdc[ctxt->active_sdc];
    struct mmc_data *data = host->data;

    if (status & (ITH_DMA_INTS_ERR|ITH_DMA_INTS_ABT))
        ithPrintf(" dma irq error 0x%X \n", status);

    host->flags_isr |= SDC_FLAG_INTR_DMA;

    if (data->flags & MMC_DATA_READ)
        ithInvalidateDCacheRange(data->buf, data->blksz*data->blocks);

    sdc_irq_status(host);
}

static void sdc_isr(void* arg)
{
    struct sdc *ctxt = (struct sdc*)arg;
    struct sdc_host *host = ctxt->sdc[ctxt->active_sdc];
    uint32_t intr;
    uint32_t old_flags_isr = host->flags_isr;

    /** check sd controller interrupt */
    intr = ithReadRegA(SD_REG_INTR);
    ithWriteRegA(SD_REG_INTR, intr);
    if (intr & SD_INTR_ERR) {
        host->flags_isr |= SDC_FLAG_INTR_ERR;
        ithPrintf(" sd irq error reg 0x%08X = 0x%08X \n", SD_REG_INTR, intr);
    }
	/** response timeout will not have command end interrupt */
    if (intr & (SD_INTR_CMD_END|SD_MSK_RESP_TIMEOUT)) {
        host->flags_isr |= SDC_FLAG_INTR_CMD;
        sdc_get_resp(host);
    }
    /** sdio interrupt */
    if (intr & SD_INTR_SDIO)
    {
        //ithPrintf(" $ %d \n", ctxt->active_sdc);
#if 0
        mmc_signal_sdio_irq(host->mmc);
#else
        {   /* only can support one sdio device */
            struct sdc_host *host0 = ctxt->sdc[0];
            struct sdc_host *host1 = ctxt->sdc[1];

            if (host0 && (host0->mmc->card->type >= SD_TYPE_SDIO))
                mmc_signal_sdio_irq(host0->mmc);
            else if (host1 && (host1->mmc->card->type >= SD_TYPE_SDIO))
                mmc_signal_sdio_irq(host1->mmc);
        }
#endif

    }

    /** check wrap interrupt */
    intr = ithReadRegA(SDW_REG_WRAP_STATUS);
    if (intr & SDW_INTR_WRAP_END) {
        host->flags_isr |= SDC_FLAG_INTR_WRAP;;
        ithWriteRegA(SDW_REG_WRAP_STATUS, intr);
    }

    if (old_flags_isr != host->flags_isr)
        sdc_irq_status(host);
}

static void sdc_interrupt_enable(struct sdc *ctxt)
{
    ithIntrRegisterHandlerIrq(ITH_INTR_SD, sdc_isr, (void*)ctxt);
    ithIntrSetTriggerModeIrq(ITH_INTR_SD, ITH_INTR_LEVEL);
    ithIntrSetTriggerLevelIrq(ITH_INTR_SD, ITH_INTR_HIGH_RISING);
    ithIntrEnableIrq(ITH_INTR_SD);

    ithWriteRegA(SD_REG_INTR, ~(SD_INTR_ALL << SD_SHT_INTR_MSK) | SD_INTR_ALL);
    SD_WrapIntrEnable();
}

static void sdc_interrupt_disable(void)
{
    ithIntrDisableIrq(ITH_INTR_SD);
    ithWriteRegA(SD_REG_INTR, (SD_INTR_ALL << SD_SHT_INTR_MSK) | SD_INTR_ALL);
    SD_WrapIntrDisable();
}
#else
#define sdc_interrupt_enable(a)
#define sdc_interrupt_disable()
#endif
