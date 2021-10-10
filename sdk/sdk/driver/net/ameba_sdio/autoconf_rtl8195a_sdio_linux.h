/******************************************************************************
 *
 * Copyright(c) 2010 - 2012 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
/*
 * Automatically generated C config: don't edit
 */




//***** temporarily flag *******

#define AUTOCONF_INCLUDED
#define RTL871X_MODULE_NAME "RTL8195A"
#define DRV_NAME "rtl8195a"

#define CONFIG_RTL8195A

#define CONFIG_SDIO_HCI

#define PLATFORM_LINUX

#define CONFIG_USE_VMALLOC
#define CONFIG_USE_TCM_HEAP 0

#define USE_RECV_TASKLET
#define ETH_ALEN					6 //ethernet address length
#define SUPPORT_SCAN_BUF 0

#define CONFIG_SDIO_TX_ENABLE_AVAL_INT
#define CONFIG_SDIO_TX_OVF_CTRL
#define CONFIG_SDIO_RX_COPY //for Rx Aggregation
#define CONFIG_TX_AGGREGATION

#define CONFIG_POWER_SAVING     // Irene Lin: TODO
//#define CONFIG_PS_DYNAMIC_CHK   // Irene Lin: TODO

//#define CONFIG_LOOPBACK_TEST //to enable loopback test, must disable CONFIG_SDIO_RX_COPY
/*******debug relative config*********/
#define DBG 1
#define CONFIG_DEBUG
//#define CONFIG_DEBUG_RTL871X //enable debug trace, _func_enter_/_func_exit_   // Irene Lin: test

//#define DBG_XMIT_BUF	//monitor free_xmitbuf_queue
//#define DBG_TX_DROP_FRAME
//#define DBG_TX_BD_FREENUM

//#define DBG_RX_AGG

//#define DBG_POWER_SAVING  // Irene Lin: test

//#define DBG_STACK_TRACE  //enable rtw_show_stack, print callstack when error happends

//#define DBG_TP //for throughput debug, monitor xmit/recv resource usage status

/*******Firmware download relative config*********/
//#define CONFIG_FWDL
#ifdef CONFIG_FWDL
//#define CONFIG_EMBEDDED_FWIMG	
#define CONFIG_FILE_FWIMG
#endif

