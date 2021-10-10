/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL USB Mass Storage Class related code.
 *
 * @author Irene Lin
 * @version 1.0
 */
#include <sys/ioctl.h>
#include <string.h>
#include "ite/ite_msc.h"

#define MSC_MAX_LUN     4 //8

static uint8_t lunStatus[2] = {0};

#define MSC_LUN_IS_ACTIVE(status, lun)      (status & (1<<lun))
#define MSC_LUN_ACTIVE(status, lun)         (status |= (1<<lun))
#define MSC_LUN_DEACTIVE(status, lun)       (status &= ~(1<<lun))


static pthread_t msc_task[2];
static bool msc_task_stop[2];
static sem_t msc_sem[2];


static void* MscPollHandler(void* arg)
{
    ITPUsb* itpusb = (ITPUsb*)arg;
    ITPCardStatus cardStatus;
    int lun, res, retry;

    while (1)
    {
        for(lun=0; lun<MSC_MAX_LUN; lun++)
        {
			if(msc_task_stop[itpusb->index] == true)
				goto end;

            retry = 1;
            do {
                res = iteMscGetStatus(itpusb->ctxt, lun);
            } while(res && --retry);

            /* original is active */
            if(MSC_LUN_IS_ACTIVE(lunStatus[itpusb->index], lun))
            {
                if(res!=0)
                {
                    cardStatus.card = (itpusb->index==0) ? (ITP_CARD_MSC00 << lun) : (ITP_CARD_MSC10 << lun);
                    cardStatus.inserted = 0;
                    write(ITP_DEVICE_CARD, &cardStatus, sizeof(ITPCardStatus));
                    printf(" %d: de-active \n", (itpusb->index==0) ? lun : 8+lun);

                    MSC_LUN_DEACTIVE(lunStatus[itpusb->index], lun);
                }
            }
            else /* original is not active */
            {
                if(res==0)
                {
                    cardStatus.card = (itpusb->index==0) ? (ITP_CARD_MSC00 << lun) : (ITP_CARD_MSC10 << lun);
                    cardStatus.inserted = 1;
                    write(ITP_DEVICE_CARD, &cardStatus, sizeof(ITPCardStatus));
                    printf(" %d: active \n", (itpusb->index==0) ? lun : 8+lun);

                    MSC_LUN_ACTIVE(lunStatus[itpusb->index], lun);
                }
            }
            itpSemWaitTimeout(&msc_sem[itpusb->index], 100);
        }
		if(msc_task_stop[itpusb->index] == true)
			goto end;
        itpSemWaitTimeout(&msc_sem[itpusb->index], 600);
    }
end:
	return 0;		
}

static void MscStatusReset(int usbIndex)
{
    int lun;
    ITPCardStatus cardStatus;

    for(lun=0; lun<MSC_MAX_LUN; lun++)
    {
        if(MSC_LUN_IS_ACTIVE(lunStatus[usbIndex], lun))
        {
            cardStatus.card = (usbIndex==0) ? (ITP_CARD_MSC00 << lun) : (ITP_CARD_MSC10 << lun);
            cardStatus.inserted = 0;
            write(ITP_DEVICE_CARD, &cardStatus, sizeof(ITPCardStatus));
            printf(" %d: de-active by reset \n", (usbIndex==0) ? lun : 8+lun);

            MSC_LUN_DEACTIVE(lunStatus[usbIndex], lun);
        }
    }
    lunStatus[usbIndex] = 0;
}


static void MscConnect(ITPUsb* itpusb)
{
    if(USB_DEVICE_MSC(itpusb->type))
    {
        pthread_attr_t attr;
        struct sched_param param;

        LOG_INFO " USB%d: MSC is inserted!! \n", itpusb->index LOG_END

        sem_init(&msc_sem[itpusb->index], 0, 0);

		msc_task_stop[itpusb->index] = false;
		pthread_attr_init(&attr);
        param.sched_priority = 4;
        pthread_attr_setschedparam(&attr, &param);
		pthread_create(&msc_task[itpusb->index], &attr, MscPollHandler, itpusb);
    }
}

static void MscDisconnect(ITPUsb* itpusb)
{
    if(USB_DEVICE_MSC(itpusb->type))
    {
        LOG_INFO " USB%d: MSC device is disconnected!\n", itpusb->index LOG_END
	    msc_task_stop[itpusb->index] = true;
        sem_post(&msc_sem[itpusb->index]);

	    pthread_join(msc_task[itpusb->index], NULL);

        MscStatusReset(itpusb->index);
        sem_destroy(&msc_sem[itpusb->index]);
    }
}


#if defined(CFG_UAS_ENABLE)

#include "ite/ite_uas.h"

static uint8_t uas_lun_num[2];

static void* UasPollHandler(void* arg)
{
    ITPUsb* itpusb = (ITPUsb*)arg;
    ITPCardStatus cardStatus;
    int lun, res, retry;

    while (1)
    {
         for(lun=0; lun<uas_lun_num[itpusb->index]; lun++)
        {
            if (msc_task_stop[itpusb->index] == true)
                goto end;

            retry = 1;
            do {
                res = iteUasGetStatus(itpusb->ctxt, lun);
            } while(res && --retry);

            /* original is active */
            if(MSC_LUN_IS_ACTIVE(lunStatus[itpusb->index], lun))
            {
                if(res!=0)
                {
                    cardStatus.card = (itpusb->index==0) ? (ITP_CARD_MSC00 << lun) : (ITP_CARD_MSC10 << lun);
                    cardStatus.inserted = 0;
                    write(ITP_DEVICE_CARD, &cardStatus, sizeof(ITPCardStatus));
                    printf(" %d: de-active \n", (itpusb->index==0) ? lun : 8+lun);

                    MSC_LUN_DEACTIVE(lunStatus[itpusb->index], lun);
                }
            }
            else /* original is not active */
            {
                if(res==0)
                {
                    cardStatus.card = (itpusb->index==0) ? (ITP_CARD_MSC00 << lun) : (ITP_CARD_MSC10 << lun);
                    cardStatus.inserted = 1;
                    write(ITP_DEVICE_CARD, &cardStatus, sizeof(ITPCardStatus));
					
                    printf(" %d: active \n", (itpusb->index==0) ? lun : 8+lun);

                    MSC_LUN_ACTIVE(lunStatus[itpusb->index], lun);
                }
            }
            itpSemWaitTimeout(&msc_sem[itpusb->index], 100);
        }
        if (msc_task_stop[itpusb->index] == true)
            goto end;
        itpSemWaitTimeout(&msc_sem[itpusb->index], (10 - uas_lun_num[itpusb->index]) * 100);
    }
end:
    return 0;
}


static void UasConnect(ITPUsb* itpusb)
{
    if(USB_DEVICE_UAS(itpusb->type))
    {
        pthread_attr_t attr;
        struct sched_param param;
        LOG_INFO " USB%d: UAS is interted!! \n", itpusb->index LOG_END

        ithPrintf(" USB%d: UAS is interted!! \n", itpusb->index);
        uas_lun_num[itpusb->index] = iteUasGetLunNum(itpusb->ctxt);
        ithPrintf(" uas_lun_num: %d \n", uas_lun_num[itpusb->index]);
        sem_init(&msc_sem[itpusb->index], 0, 0);

        msc_task_stop[itpusb->index] = false;
        pthread_attr_init(&attr);
        param.sched_priority = 4;
        pthread_attr_setschedparam(&attr, &param);
        pthread_create(&msc_task[itpusb->index], &attr, UasPollHandler, itpusb);
    }
}

static void UasDisconnect(ITPUsb* itpusb)
{
    if(USB_DEVICE_UAS(itpusb->type))
    {
        LOG_INFO " USB%d: UAS device is disconnected!\n", itpusb->index LOG_END

        uas_lun_num[itpusb->index] = 0;

        msc_task_stop[itpusb->index] = true;

        sem_post(&msc_sem[itpusb->index]);

        pthread_join(msc_task[itpusb->index], NULL);
        MscStatusReset(itpusb->index);
        sem_destroy(&msc_sem[itpusb->index]);
    }
}


#endif // #if defined(CFG_UAS_ENABLE)
