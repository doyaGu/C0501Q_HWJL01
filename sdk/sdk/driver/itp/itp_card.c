/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL Card functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/queue.h"
#include "itp_cfg.h"

#define QUEUE_LEN 3

static ITPCardStatus cardStatusTable[ITP_CARD_MAX];

static QueueHandle_t cardQueue;

static void CardIntrHandler(unsigned int pin, void* arg)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    switch (pin)
    {
#if (defined(CFG_SD0_ENABLE) && !defined(CFG_SD0_STATIC) && !defined(CFG_SDIO0_STATIC)) || defined(CFG_MS_ENABLE) || defined(CFG_XD_ENABLE)
    case CFG_GPIO_SD0_CARD_DETECT:
        cardStatusTable[0].inserted  = ithCardInserted(ITH_CARDPIN_SD0);
        xQueueSendFromISR(cardQueue, &cardStatusTable[0], &xHigherPriorityTaskWoken);
        break;

#endif // (defined(CFG_SD0_ENABLE) && !defined(CFG_SD0_STATIC) && !defined(CFG_SDIO0_STATIC)) || defined(CFG_MS_ENABLE) || defined(CFG_XD_ENABLE)

#if defined(CFG_SD1_ENABLE) && !defined(CFG_SD1_STATIC) && !defined(CFG_SDIO1_STATIC)
    case CFG_GPIO_SD1_CARD_DETECT:
        cardStatusTable[1].inserted  = ithCardInserted(ITH_CARDPIN_SD1);
        xQueueSendFromISR(cardQueue, &cardStatusTable[1], &xHigherPriorityTaskWoken);
        break;

#endif // defined(CFG_SD1_ENABLE) && !defined(CFG_SD1_STATIC)

    default:
        break;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void CardInit(void)
{
    int i;

    cardQueue = xQueueCreate(QUEUE_LEN, (unsigned portBASE_TYPE) sizeof(ITPCardStatus));

    ithEnterCritical();

    // detect pin of SD0, MS, xD is shared
#if (defined(CFG_SD0_ENABLE) && !defined(CFG_SD0_STATIC) && !defined(CFG_SDIO0_STATIC)) || defined(CFG_MS_ENABLE) || defined(CFG_XD_ENABLE)
    ithGpioClearIntr(CFG_GPIO_SD0_CARD_DETECT);
    ithGpioSetIn(CFG_GPIO_SD0_CARD_DETECT);
    ithGpioRegisterIntrHandler(CFG_GPIO_SD0_CARD_DETECT, CardIntrHandler, (void*) ITP_CARD_SD0);
    ithGpioCtrlDisable(CFG_GPIO_SD0_CARD_DETECT, ITH_GPIO_INTR_LEVELTRIGGER);   /* use edge trigger mode */
    ithGpioCtrlEnable(CFG_GPIO_SD0_CARD_DETECT, ITH_GPIO_INTR_BOTHEDGE); /* both edge */
    ithGpioEnableBounce(CFG_GPIO_SD0_CARD_DETECT);
    ithGpioEnableIntr(CFG_GPIO_SD0_CARD_DETECT);

    cardStatusTable[0].card      = 0;
    #ifdef CFG_SD0_ENABLE
        cardStatusTable[0].card  |= ITP_CARD_SD0;
    #endif
    #ifdef CFG_MS_ENABLE
        cardStatusTable[0].card  |= ITP_CARD_MS;
    #endif
    #ifdef CFG_XD_ENABLE
        cardStatusTable[0].card  |= ITP_CARD_XD;
    #endif

    cardStatusTable[0].inserted  = ithCardInserted(ITH_CARDPIN_SD0);

#endif // (defined(CFG_SD0_ENABLE) && !defined(CFG_SD0_STATIC) && !defined(CFG_SDIO0_STATIC)) || defined(CFG_MS_ENABLE) || defined(CFG_XD_ENABLE)

#if defined(CFG_SD1_ENABLE) && !defined(CFG_SD1_STATIC) && !defined(CFG_SDIO1_STATIC)
    ithGpioClearIntr(CFG_GPIO_SD1_CARD_DETECT);
    ithGpioSetIn(CFG_GPIO_SD1_CARD_DETECT);
    ithGpioRegisterIntrHandler(CFG_GPIO_SD1_CARD_DETECT, CardIntrHandler, (void*) ITP_CARD_SD1);
    ithGpioCtrlDisable(CFG_GPIO_SD1_CARD_DETECT, ITH_GPIO_INTR_LEVELTRIGGER);   /* use edge trigger mode */
    ithGpioCtrlEnable(CFG_GPIO_SD1_CARD_DETECT, ITH_GPIO_INTR_BOTHEDGE); /* both edge */
    ithGpioEnableBounce(CFG_GPIO_SD1_CARD_DETECT);
    ithGpioEnableIntr(CFG_GPIO_SD1_CARD_DETECT);

    cardStatusTable[1].card      = ITP_CARD_SD1;
    cardStatusTable[1].inserted  = ithCardInserted(ITH_CARDPIN_SD1);
#endif // defined(CFG_SD1_ENABLE) && !defined(CFG_SD1_STATIC)

    ithExitCritical();

    // send insert events on booting
#if 0
    for (i = 0; i < 3; i++)
    {
        if (cardStatusTable[i].inserted)
            xQueueSend(cardQueue, &cardStatusTable[i], portMAX_DELAY);
    }
#endif // 0
}

static int CardOpen(const char* name, int flags, int mode, void* info)
{
    if (flags & O_NONBLOCK)
        return 1;

    return 0;
}

static int CardRead(int file, char *ptr, int len, void* info)
{
    if (len < sizeof (ITPCardStatus))
        return 0;

    if (xQueueReceive(cardQueue, ptr, file ? 0 : portMAX_DELAY))
        return sizeof (ITPCardStatus);

    return 0;
}

static int CardWrite(int file, char *ptr, int len, void* info)
{
    if (len == sizeof (ITPCardStatus))
    {
        int cardIndex;
        ITPCardStatus* cardStatus = (ITPCardStatus*)ptr;

        for(cardIndex=0; cardIndex<ITP_CARD_MAX; cardIndex++)
            if((cardStatus->card >> cardIndex) & 0x1) break;

        if(cardIndex >= ITP_CARD_MAX)
            LOG_ERR " CardWrite index fail \n" LOG_END

        cardStatusTable[cardIndex].card = cardStatus->card;
        cardStatusTable[cardIndex].inserted = cardStatus->inserted;

        LOG_DBG "card 0x%X insert: %d\n", cardStatus->card, cardStatus->inserted LOG_END

        xQueueSend(cardQueue, ptr, portMAX_DELAY);
        return sizeof (ITPCardStatus);
    }
    return 0;
}

static bool CardIsAvail(ITPCard card)
{
    int i;

    LOG_DBG "CardIsAvail(0x%X)\n", card LOG_END

#ifdef CFG_SD0_STATIC
    if (card & ITP_CARD_SD0)
        return true;
#endif

#ifdef CFG_SD1_STATIC
    if (card & ITP_CARD_SD1)
        return true;
#endif

    for (i = 0; i < ITP_CARD_MAX; i++)
    {
        LOG_DBG "cardStatusTable[%d]: card=%d inserted=%d\n", i, cardStatusTable[i].card, cardStatusTable[i].inserted LOG_END
        if (cardStatusTable[i].card & card)
            return cardStatusTable[i].inserted;
    }
    return false;
}

static int CardIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_IS_AVAIL:
        return (int) CardIsAvail((ITPCard)ptr);

    case ITP_IOCTL_INIT:
        CardInit();
        break;

    case ITP_IOCTL_GET_TABLE:
        *(ITPCardStatus**)ptr = cardStatusTable;
        break;

    default:
        errno = (ITP_DEVICE_CARD << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceCard =
{
    ":card",
    CardOpen,
    itpCloseDefault,
    CardRead,
    CardWrite,
    itpLseekDefault,
    CardIoctl,
    NULL
};
