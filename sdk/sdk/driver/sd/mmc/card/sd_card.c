#include <linux/mmc/errorno.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/card.h>
#include "ite/ite_sd.h"


int iteSdInitializeEx(int index)
{
    struct mmc_host *mmc = mmc_get_host(index);
    struct mmc_card *card;
    int rc;

    if (!mmc || !mmc->card)
        return -2;

    card = mmc->card;

    if (card->type >= SD_TYPE_SDIO)
        return -3;

    mmc_claim_host(mmc->card->host);
    rc = mmc_set_blocklen(mmc->card, 512);

    if ((mmc->ops->get_ro(mmc) == 1) ||
        !(card->csd.cmdclass & CCC_BLOCK_WRITE))
        mmc_card_set_readonly(card);

    mmc_release_host(mmc->card->host);

    return rc;
}

int iteSdTerminateEx(int index)
{
    struct mmc_host *mmc = mmc_get_host(index);

    //printf(" iteSdTerminateEx() ==> TODO \n");

    return 0;
}

static int mmc_test_busy(struct mmc_command *cmd)
{
    return !(cmd->resp[0] & R1_READY_FOR_DATA) ||
        (R1_CURRENT_STATE(cmd->resp[0]) == R1_STATE_PRG);
}

/*
* Checks that a normal transfer didn't have any errors
*/
static int sd_check_result(struct mmc_request *mrq)
{
    int ret;

    BUG_ON(!mrq || !mrq->cmd || !mrq->data);

    ret = 0;

    if (!ret && mrq->cmd->error)
        ret = mrq->cmd->error;
    if (!ret && mrq->data->error)
        ret = mrq->data->error;
    if (!ret && mrq->stop && mrq->stop->error)
        ret = mrq->stop->error;
    if (!ret && mrq->data->bytes_xfered !=
        mrq->data->blocks * mrq->data->blksz)
        ret = -11;

    if (ret == -EINVAL)
        ret = -2;

    return ret;
}

/*
* Wait for the card to finish the busy state
*/
static int sd_wait_busy(struct mmc_card *card)
{
    int ret, busy;
    struct mmc_command cmd = { 0 };
    
    busy = 0;
    do {
        memset(&cmd, 0, sizeof(struct mmc_command));

        cmd.opcode = MMC_SEND_STATUS;
        cmd.arg = card->rca << 16;
        cmd.flags = MMC_RSP_R1 | MMC_CMD_AC;

        ret = mmc_wait_for_cmd(card->host, &cmd, 0);
        if (ret)
            break;

        if (!busy && mmc_test_busy(&cmd)) {
            busy = 1;
            if (card->host->caps & MMC_CAP_WAIT_WHILE_BUSY)
                printf("%s: Warning: Host did not "
                "wait for busy state to end.\n",
                mmc_hostname(card->host));
        }
    } while (mmc_test_busy(&cmd));

    return ret;
}

static void sd_prepare_mrq(struct mmc_card *card, struct mmc_request *mrq, 
    uint32_t sector, uint32_t count, void *buf, int write)
{
    if (count > 1) {
    //if (1) {
        mrq->cmd->opcode = write ?
        MMC_WRITE_MULTIPLE_BLOCK : MMC_READ_MULTIPLE_BLOCK;
    }
    else {
        mrq->cmd->opcode = write ?
        MMC_WRITE_BLOCK : MMC_READ_SINGLE_BLOCK;
    }

    mrq->cmd->arg = sector;
    if (!mmc_card_blockaddr(card))
        mrq->cmd->arg <<= 9;

    mrq->cmd->flags = MMC_RSP_R1 | MMC_CMD_ADTC;

    if (count == 1)
        mrq->stop = NULL;
    else {
        mrq->stop->opcode = MMC_STOP_TRANSMISSION;
        mrq->stop->arg = 0;
        mrq->stop->flags = MMC_RSP_R1B | MMC_CMD_AC;
    }

    mrq->data->blksz = 512;
    mrq->data->blocks = count;
    mrq->data->flags = write ? MMC_DATA_WRITE : MMC_DATA_READ;
    mrq->data->buf = buf;

    mmc_set_data_timeout(mrq->data, card);
}

int iteSdReadMultipleSectorEx(int index, uint32_t sector, int count, void* buf)
{
    int rc = 0;
    struct mmc_host *mmc = mmc_get_host(index);
    struct mmc_card *card;
    struct mmc_request mrq = { 0 };
    struct mmc_command cmd = { 0 };
    struct mmc_command stop = { 0 };
    struct mmc_data data = { 0 };
    void *tmp_buf = NULL;

    if (!mmc || !mmc->card)
        return -2;

    card = mmc->card;

    mrq.cmd = &cmd;
    mrq.data = &data;
    mrq.stop = &stop;

    sd_prepare_mrq(card, &mrq, sector, count, buf, 0);

    mmc_claim_host(mmc->card->host);

    mmc_wait_for_req(card->host, &mrq);

#if 0
    sd_wait_busy(card);

    rc = sd_check_result(&mrq);
#else
    rc = sd_check_result(&mrq);
    if (rc == 0)
        sd_wait_busy(card);
#endif

    mmc_release_host(mmc->card->host);

    return rc;
}

int iteSdWriteMultipleSectorEx(int index, uint32_t sector, int count, void *buf)
{
    int rc = 0;
    struct mmc_host *mmc = mmc_get_host(index);
    struct mmc_card *card;
    struct mmc_request mrq = { 0 };
    struct mmc_command cmd = { 0 };
    struct mmc_command stop = { 0 };
    struct mmc_data data = { 0 };

    if (!mmc || !mmc->card)
        return -2;

    card = mmc->card;

    if (mmc_card_readonly(card)) {
        printf("[SD][ERR] Invalid write! This card is read-only! \n");
        return -3;
    }

    mmc_claim_host(mmc->card->host);

    mrq.cmd = &cmd;
    mrq.data = &data;
    mrq.stop = &stop;

    sd_prepare_mrq(card, &mrq, sector, count, buf, 1);

    mmc_wait_for_req(card->host, &mrq);

#if 0
    sd_wait_busy(card);

    rc = sd_check_result(&mrq);
#else
    rc = sd_check_result(&mrq);
    if (rc == 0)
        sd_wait_busy(card);
#endif

    mmc_release_host(mmc->card->host);

    return rc;
}

int iteSdGetCardStateEx(int index, int state)
{
    struct mmc_host *mmc = mmc_get_host(index);

    if (mmc == NULL)
        return 0;

    switch (state){
    case SD_INIT_OK:
        if (mmc->card)
            return 1;
        else
            return 0;

    case SD_INSERTED:
        return mmc->ops->get_cd(mmc);
        
    default:
        return 0;
    }
}

int iteSdGetCapacityEx(int index, uint32_t* sectorNum, uint32_t* blockLength)
{
    struct mmc_host *mmc = mmc_get_host(index);
    struct mmc_card *card;

    (*sectorNum) = 0;
    (*blockLength) = 0;

    if (mmc == NULL || mmc->card == NULL)
        return -1;

    card = mmc->card;

    if (!mmc_card_sd(card) && mmc_card_blockaddr(card))
        (*sectorNum) = card->ext_csd.sectors;
    else
        (*sectorNum) = card->csd.capacity << (card->csd.read_blkbits - 9);
    
    (*blockLength) = 512;

    return 0;
}

bool iteSdIsLockEx(int index)
{
    struct mmc_host *mmc = mmc_get_host(index);
    struct mmc_card *card;

    if (!mmc || !mmc->card)
        return false;

    card = mmc->card;

    return mmc_card_readonly(card) ? true : false;
}

