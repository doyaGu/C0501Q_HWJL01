/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Initialize functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include "ite/ith.h"
#include "ite/itp.h"
#include <sys/ioctl.h>
#include <locale.h>
#include <stdio.h>
#include <pthread.h>

void BootInit(void)
{
	pthread_win32_process_attach_np();

    // init hal module
    ithInit();

    // init memleak tool
#ifdef CFG_DBG_MEMLEAK
    dbg_init(CFG_DBG_MEMLEAK_LEN);
#endif

#if defined(CFG_DBG_PRINTBUF)
    // init print buffer device
    itpRegisterDevice(ITP_DEVICE_STD, &itpDevicePrintBuf);
    itpRegisterDevice(ITP_DEVICE_PRINTBUF, &itpDevicePrintBuf);
    ioctl(ITP_DEVICE_PRINTBUF, ITP_IOCTL_INIT, NULL);
#endif // defined(CFG_DBG_PRINTBUF)

    puts(CFG_SYSTEM_NAME "/" CFG_PROJECT_NAME " ver " CFG_VERSION_MAJOR_STR "." CFG_VERSION_MINOR_STR "." CFG_VERSION_PATCH_STR "." CFG_VERSION_CUSTOM_STR "." CFG_VERSION_TWEAK_STR);
}
