/*
 *
 *  Copyright (C) 2012 ITE TECH. INC.   All Rights Reserved.
 *
 */
#ifndef __IDBINJECTION_H__
#define __IDBINJECTION_H__
#include <semaphore.h>
#include "../gadget.h"
#include "idb.h"
#ifdef __cplusplus
extern "C" {
#endif

#define IDB_COMMA ,
#define IDB_ANONYMOUSVAR_HELPER0(type, name) type _##name
#define IDB_ANONYMOUSVAR_HELPER(type, name) IDB_ANONYMOUSVAR_HELPER0(type, name)
#define IDB_ANONYMOUSVAR(type) IDB_ANONYMOUSVAR_HELPER(type, __LINE__)
#if (CONFIG_USBD_IDB)
#define IDB_DATA_INJECTION(x) x
#define IDB_CODE_INJECTION(x) x
#else
#define IDB_DATA_INJECTION(x)
#define IDB_CODE_INJECTION(x) do {} while(0)
#endif

#define IDB_INTERFACE_STRING "ITE IDB DEVICE"

enum {
    IDB_INTERFACE_STRING_INDEX=10,
};

struct idbCtx {
	struct usb_ep* epIn;
	struct usb_ep* epOut;
	struct usb_gadget *gadget;
	struct usb_request* reqIn;
	struct usb_request* reqInZero;
	struct usb_request* reqInUsr;
	struct usb_request* reqInUsrZero;
	struct usb_request* reqOut;
	union {
		unsigned char bufIn[128*1024];
        IDB_ANONYMOUSVAR(long long);
	};
	union {
		unsigned char bufInUsr[128*1024];
        IDB_ANONYMOUSVAR(long long);
	};
	union {
		unsigned char bufOut[128*1024];
        IDB_ANONYMOUSVAR(long long);
	};
	int reqInStatus;
	int reqInZeroStatus;
	int reqInUsrStatus;
	int reqInUsrZeroStatus;
	int reqOutStatus;
    //IDBCALLBACK idbCallback;
    _spinlock_t	lock;
	sem_t* wakeupIdbSem;
    uint32_t exception:1;
    uint32_t running:1;
    uint32_t connected:1;
};

void idbLock();
void idbUnlock();
void idbAsycnDataSwitch(int siwtch);
void idbReqComplete(struct usb_ep *ep, struct usb_request *req);
int idbBindInjection(struct usb_gadget *gadget, struct idbCtx* idbCtx, struct usb_config_descriptor* configDesc);
int idbDoSetInterfaceInjection(struct idbCtx* idbCtx, int altsetting);
int idbThreadInjection(struct idbCtx* idbCtx);
void* idbThread(void* arg);

struct usb_interface_descriptor idbInterfaceDescriptor;
struct usb_endpoint_descriptor idbFsEpInDescriptor;
struct usb_endpoint_descriptor idbFsEpOutDescriptor;
struct usb_endpoint_descriptor idbHsEpInDescriptor;
struct usb_endpoint_descriptor idbHsEpOutDescriptor;

#ifdef __cplusplus
}
#endif
#endif