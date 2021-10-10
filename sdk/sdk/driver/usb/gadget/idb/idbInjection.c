/*
 *
 *  Copyright (C) 2012 ITE TECH. INC.   All Rights Reserved.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "../gadget.h"
#include "idbInjection.h"
#include "idbProperty.h"
#include "idb.h"
#include "../udc_reg.h"
#ifdef __cplusplus
extern "C" {
#endif

static struct idbCtx* gidbCtx=0;
static IDBCALLBACK gidbCallback=0;
struct usb_interface_descriptor idbInterfaceDescriptor =  {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = 0xff,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = IDB_INTERFACE_STRING_INDEX,
};

struct usb_endpoint_descriptor idbFsEpInDescriptor = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = USB_DIR_IN,
    .bmAttributes = USB_ENDPOINT_XFER_BULK,
};

struct usb_endpoint_descriptor idbFsEpOutDescriptor = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = USB_DIR_OUT,
    .bmAttributes = USB_ENDPOINT_XFER_BULK,
};

struct usb_endpoint_descriptor idbHsEpInDescriptor = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bmAttributes = USB_ENDPOINT_XFER_BULK,
};

struct usb_endpoint_descriptor idbHsEpOutDescriptor = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bmAttributes = USB_ENDPOINT_XFER_BULK,
};

void idbLock(struct idbCtx* idbCtx) {
    if (idbCtx) {
        _spin_lock(&idbCtx->lock);
    }
    else if (gidbCtx) {
        _spin_lock(&gidbCtx->lock);
    }
    else {
        printf("[X] idbLock\n");
    }
}

void idbUnlock(struct idbCtx* idbCtx) {
    if (idbCtx) {
        _spin_unlock(&idbCtx->lock);
    }
    else if (gidbCtx) {
        _spin_unlock(&gidbCtx->lock);
    }
    else {
        printf("[X] idbUnlock\n");
    }
}

void idbReqComplete(struct usb_ep *ep, struct usb_request *req) {
    struct idbCtx* idbCtx=ep->driver_data;
    if(req->status /*|| (req->actual != req->length)*/)
        printf("[X] %s => status:0x%X(%d) (%d/%d) %08x:%08x\n", __FUNCTION__, -req->status, -req->status, req->actual, req->length, mUsbIntSrc1Rd(), mUsbIntSrc1MaskRd());
    if(req->status == -ECONNRESET) 
        usb_ep_fifo_flush(ep);
    //idbLock(idbCtx);
	if (req==idbCtx->reqIn) {
		idbCtx->reqInStatus=-1;
	} 
    else if (req==idbCtx->reqInZero) {
		idbCtx->reqInZeroStatus=-1;
	}
    else if (req==idbCtx->reqInUsr) {
		idbCtx->reqInUsrStatus=-1;
	}
    else if (req==idbCtx->reqInUsrZero) {
		idbCtx->reqInUsrZeroStatus=-1;
	}
	else {
		idbCtx->reqOutStatus=1;
	}
    //idbUnlock(idbCtx);
}

int idbBindInjection(struct usb_gadget *gadget, struct idbCtx* idbCtx, struct usb_config_descriptor* configDesc) {
    struct usb_ep *ep;
    _spin_lock_init(&idbCtx->lock);

printf("[!] +++%s\n", __FUNCTION__);

	idbCtx->gadget=gadget;
	ep=usb_ep_autoconfig(gadget, &idbFsEpInDescriptor);
    ep->driver_data=idbCtx;
    idbCtx->epIn=ep;
    ep=usb_ep_autoconfig(gadget, &idbFsEpOutDescriptor);
    ep->driver_data=idbCtx;
    idbCtx->epOut=ep;
    if(gadget_is_dualspeed(gadget)) {
        idbHsEpInDescriptor.bEndpointAddress=idbFsEpInDescriptor.bEndpointAddress;
        idbHsEpOutDescriptor.bEndpointAddress=idbFsEpOutDescriptor.bEndpointAddress;
	}
    idbHsEpInDescriptor.wMaxPacketSize=cpu_to_le16(512);
    idbHsEpOutDescriptor.wMaxPacketSize=cpu_to_le16(512);
    configDesc->bNumInterfaces=2;
    idbCtx->exception=0;
    idbCtx->running=0;
    idbCtx->connected=0;

printf("[!] ---%s\n", __FUNCTION__);
}

int idbDoSetInterfaceInjection(struct idbCtx* idbCtx, int altsetting) {
printf("[!] +++%s\n", __FUNCTION__);
	if (idbCtx->reqIn) {
        usb_ep_dequeue(idbCtx->epIn, idbCtx->reqIn);
		usb_ep_free_request(idbCtx->epIn, idbCtx->reqIn);
		idbCtx->reqIn=0;
	}
	if (idbCtx->reqInZero) {
        usb_ep_dequeue(idbCtx->epIn, idbCtx->reqInZero);
		usb_ep_free_request(idbCtx->epIn, idbCtx->reqInZero);
		idbCtx->reqInZero=0;
	}
	if (idbCtx->reqInUsr) {
        usb_ep_dequeue(idbCtx->epIn, idbCtx->reqInUsr);
		usb_ep_free_request(idbCtx->epIn, idbCtx->reqInUsr);
		idbCtx->reqInUsr=0;
	}
	if (idbCtx->reqInUsrZero) {
        usb_ep_dequeue(idbCtx->epIn, idbCtx->reqInUsrZero);
		usb_ep_free_request(idbCtx->epIn, idbCtx->reqInUsrZero);
		idbCtx->reqInUsrZero=0;
	}
	if (idbCtx->reqOut) {
        usb_ep_dequeue(idbCtx->epOut, idbCtx->reqOut);
		usb_ep_free_request(idbCtx->epOut, idbCtx->reqOut);
		idbCtx->reqOut=0;
	}
	usb_ep_disable(idbCtx->epIn);
	usb_ep_disable(idbCtx->epOut);
    if (altsetting < 0) {
        idbCtx = 0;
printf("[!] ---%s\n@0", __FUNCTION__);
        return 0;
    }
    gidbCtx=idbCtx;
	usb_ep_enable(idbCtx->epIn, (gadget_is_dualspeed(idbCtx->gadget) && idbCtx->gadget->speed == USB_SPEED_HIGH)?&idbHsEpInDescriptor:&idbFsEpInDescriptor);
	usb_ep_enable(idbCtx->epOut, (gadget_is_dualspeed(idbCtx->gadget) && idbCtx->gadget->speed == USB_SPEED_HIGH)?&idbHsEpOutDescriptor:&idbFsEpOutDescriptor);
	idbCtx->reqIn=usb_ep_alloc_request(idbCtx->epIn);
	idbCtx->reqIn->buf=idbCtx->bufIn;
	idbCtx->reqIn->context=idbCtx;
	idbCtx->reqIn->complete=idbReqComplete;
	idbCtx->reqInStatus=-1;
	idbCtx->reqInZero=usb_ep_alloc_request(idbCtx->epIn);
	idbCtx->reqInZero->buf=idbCtx->bufIn;
	idbCtx->reqInZero->context=idbCtx;
	idbCtx->reqInZero->complete=idbReqComplete;
    idbCtx->reqInZero->length=0;
	idbCtx->reqInZeroStatus=-1;
	idbCtx->reqInUsr=usb_ep_alloc_request(idbCtx->epIn);
	idbCtx->reqInUsr->buf=idbCtx->bufInUsr;
	idbCtx->reqInUsr->context=idbCtx;
	idbCtx->reqInUsr->complete=idbReqComplete;
	idbCtx->reqInUsrStatus=-1;
	idbCtx->reqInUsrZero=usb_ep_alloc_request(idbCtx->epIn);
	idbCtx->reqInUsrZero->buf=idbCtx->bufInUsr;
	idbCtx->reqInUsrZero->context=idbCtx;
	idbCtx->reqInUsrZero->complete=idbReqComplete;
    idbCtx->reqInUsrZero->length=0;
	idbCtx->reqInUsrZeroStatus=-1;
	idbCtx->reqOut=usb_ep_alloc_request(idbCtx->epOut);
	idbCtx->reqOut->buf=idbCtx->bufOut;
	idbCtx->reqOut->context=idbCtx;
	idbCtx->reqOut->complete=idbReqComplete;
	idbCtx->reqOutStatus=-1;

printf("[!] ---%s\n", __FUNCTION__);
    return 0;
}

int idbThreadInjection(struct idbCtx* idbCtx) {
printf("[!] +++%s\n", __FUNCTION__);
	if (!idbCtx->wakeupIdbSem) {
		pthread_t task;
		pthread_attr_t attr;
		struct sched_param param;
		idbCtx->wakeupIdbSem=(sem_t *)malloc(sizeof(sem_t)); 
		sem_init(idbCtx->wakeupIdbSem, 0, 0); 
		pthread_attr_init(&attr);
		pthread_attr_setstacksize(&attr, 8192);
		//param.sched_priority=1+2;
		//pthread_attr_setschedparam(&attr, &param);
		if(pthread_create(&task, &attr, idbThread, idbCtx))
			printf("[X] create idbThread fail! \n");
    }
	sem_post(idbCtx->wakeupIdbSem);
printf("[!] ---%s\n", __FUNCTION__);
}

int idbIsRunning() {
    return gidbCtx ? gidbCtx->running : 0;
}

int idbIsConnected() {    
    return idbIsRunning() ? gidbCtx->connected : 0;
}

void idbAsycnDataSwitch(int asyncdataSwitch) {
    gidbCtx->connected=!!asyncdataSwitch;
}

void* idbThread(void* arg) {
	struct idbCtx* idbCtx=(struct idbCtx*)arg;
	int count;
    int howManyMore;
again:
	sem_wait(idbCtx->wakeupIdbSem);
    howManyMore=-1;
    printf("+++%s\n", __FUNCTION__);
	{
		while (idbCtx->running)
		{
            usleep(1000);
			if (idbCtx->exception)
			{
				printf("!!!%s@e\n", __FUNCTION__);
                howManyMore=-1;
				continue;
			}
			if (idbCtx->reqOutStatus < 0) {
                idbCtx->reqOut->buf=idbCtx->bufOut;
				idbCtx->reqOut->length=0;
				idbCtx->reqOutStatus=0;
				usb_ep_queue(idbCtx->epOut, idbCtx->reqOut);
			} 
			else if (idbCtx->reqOutStatus > 0) {
                if (howManyMore<0) {
                    howManyMore = idbHowManyMore(idbCtx->bufOut, idbCtx->reqOut->length);
//if (howManyMore>0)
//printf("[i] howManyMore=%d@0\n", howManyMore);
                    if (howManyMore > 0) {
                        idbCtx->reqOut->buf=idbCtx->bufOut+idbCtx->reqOut->length;
                        idbCtx->reqOut->length=howManyMore;
                        idbCtx->reqOutStatus=0;
                        usb_ep_queue(idbCtx->epOut, idbCtx->reqOut);
                        continue;
                    }
                }
                if (idbCtx->reqInStatus < 0 && idbCtx->reqInZeroStatus < 0) {
//if (howManyMore>0)
//printf("[i] howManyMore=%d@1\n", howManyMore);
                    howManyMore=-1;
                    idbCtx->reqOutStatus=-1;
                    if (idbIsCustomerId(idbCtx->bufOut)) {
                        if (gidbCallback) {
                            gidbCallback(idbCtx->bufOut);
                        }
                        else {
                            printf("[X] no customer id handler!\n");
                        }
                    }
                    else {
                        idbCtx->reqIn->length=idbDoProperty(idbCtx->bufOut, idbCtx->bufIn);
                        //idbLock(idbCtx);
                        if (idbCtx->reqIn->length > 0) {
                            idbCtx->reqInStatus=0;
                            usb_ep_queue(idbCtx->epIn, idbCtx->reqIn);
                            if ((idbCtx->reqIn->length % 512)==0) {
                                idbCtx->reqInZeroStatus=0;
                                usb_ep_queue(idbCtx->epIn, idbCtx->reqInZero);
                            }
                        }
                        //idbUnlock(idbCtx);
                    }
                }
			}
		}
	}
    gidbCtx->connected=0;
    gidbCallback=0;
    printf("---%s\n", __FUNCTION__);
    goto again;
}

int idbSend2Host(void* data, int size, int wait) {
    int r=-1;
    int static _ref=0;
    int ref;

    if (idbIsRunning()) {
        do {
            idbLock(gidbCtx);
            if (gidbCtx->reqInUsrStatus < 0 && gidbCtx->reqInUsrZeroStatus < 0) {
                gidbCtx->reqInUsrStatus=0;
                idbUnlock(gidbCtx);
                memcpy(gidbCtx->reqInUsr->buf, data, size);
                gidbCtx->reqInUsr->length=size;
                usb_ep_queue(gidbCtx->epIn, gidbCtx->reqInUsr);
                if ((size % 512) == 0) {
                    gidbCtx->reqInUsrZeroStatus = 0;
                    usb_ep_queue(gidbCtx->epIn, gidbCtx->reqInUsrZero);
                }
                r=0;
                break;
            }
            idbUnlock(gidbCtx);
            usleep(1000);
        } while (wait && idbIsRunning() && !gidbCtx->exception);
    }
    return r;
}

int idbSend2HostAsync(void* data, int size, int wait) {
   if (idbIsConnected()) {
       return idbSend2Host(data, size, wait);
   }
   return -1;
 }
 
int idbSetCallback(IDBCALLBACK idbCallback) {
    gidbCallback=idbCallback;
        return 0;
}

//portENTER_CRITICAL();
//portEXIT_CRITICAL();

#ifdef __cplusplus
}
#endif