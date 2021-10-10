/* Faraday FTGMAC030 Linux driver
 * Copyright(c) 2016 - 2018 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

/* Linux Faraday FTGMAC030 Ethernet Driver main header file */

#ifndef _FTGMAC030_H_
#define _FTGMAC030_H_

#include <linux/os.h>
#include <linux/spinlock.h>
#include <linux/compiler.h>

#include "ite/ith.h"
#include "gmac/config.h"
#include "gmac/skb.h"
#include "ndis.h"
#include "mii.h"

#include "regs.h"

struct ftgmac030_ctrl;

#define drv         "drv:"
//#define hw          "hw:"
#define link        "link:"
#define intr        "intr:"
#define ifdown      "ifdown:"
#define tx_queued   "tx_queued:"
#define rx_err      "rx_err:"

#if E_DBG
#define e_dbg(type, format, arg...)     do { ithPrintf("[MAC][DBG]"type); ithPrintf(format, ## arg); } while (0)
#else
#define e_dbg(type, format, arg...)
#endif

#if E_ERR
#define e_err(type, format, arg...)     do { ithPrintf("[MAC][ERR]"type); ithPrintf(format, ## arg); } while (0)
#else
#define e_err(type, format, arg...)
#endif

#if E_INFO
#define e_info(type, format, arg...)    do { ithPrintf("[MAC][INFO]"type); ithPrintf(format, ## arg); } while (0)
#else
#define e_info(type, format, arg...)
#endif

#if E_WARN
#define e_warn(type, format, arg...)    do { ithPrintf("[MAC][WARN]"type); ithPrintf(format, ## arg); } while (0)
#else
#define e_warn(type, format, arg...)
#endif

#if E_NOTICE
#define e_notice(type, format, arg...)  do { ithPrintf("[MAC][NOTICE]"type); ithPrintf(format, ## arg); } while (0)
#else
#define e_notice(type, format, arg...)
#endif




/* Number of Transmit and Receive Descriptors must be a multiple of 8 */
#define REQ_TX_DESCRIPTOR_MULTIPLE  8
#define REQ_RX_DESCRIPTOR_MULTIPLE  8

/* Tx/Rx descriptor defines */
#define FTGMAC030_DEFAULT_TXD		TX_QUEUE_ENTRIES
#define FTGMAC030_MAX_TXD		4096
#define FTGMAC030_MIN_TXD		64

#define FTGMAC030_DEFAULT_RXD		RX_QUEUE_ENTRIES
#define FTGMAC030_MAX_RXD		4096
#define FTGMAC030_MIN_RXD		64

#define FTGMAC030_MIN_ITR_USECS 	10 /* 100000 irq/sec */
#define FTGMAC030_MAX_ITR_USECS 	10000 /* 100    irq/sec */

#define FTGMAC030_FC_PAUSE_TIME		0x0680 /* 858 usec */

/* How many Tx Descriptors do we need to call netif_wake_queue ? */
/* How many Rx Buffers do we bundle into one write to the hardware ? */
#define FTGMAC030_RX_BUFFER_WRITE	16 /* Must be power of 2 */

#define DEFAULT_JUMBO			9234

#define LINK_TIMEOUT			100

/* Count for polling __E1000_RESET condition every 10-20msec.
 * Experimentation has shown the reset can take approximately 210msec.
 */
#define FTGMAC030_CHECK_RESET_COUNT	25

/* wrappers around a pointer to a socket buffer,
 * so a DMA handle can be stored along with the buffer
 */
struct ftgmac030_buffer {
	dma_addr_t dma;
	struct sk_buff *skb;
	union {
		/* Tx */
		struct {
			unsigned long time_stamp;
			u16 length;
			u16 next_to_watch;
			unsigned int segs;
			unsigned int bytecount;
			u16 mapped_as_page;
		};
		/* Rx */
		struct {
			struct page *page;
		};
	};
};

struct ftgmac030_ring {
	struct ftgmac030_ctrl *hw;	/* back pointer to ctrl */
	void *desc;			/* pointer to ring memory  */
	dma_addr_t dma;			/* phys address of ring    */
	unsigned int size;		/* length of ring in bytes */
	unsigned int count;		/* number of desc. in ring */

	u16 next_to_use;
	u16 next_to_clean;
	spinlock_t ntu_lock;	/* protects next to use pointer */
	spinlock_t ntc_lock;	/* protects clean up */

	//void __iomem *head;
	//void __iomem *tail;
	uint32_t head;
	uint32_t tail;

	/* array of buffer information structs */
	struct ftgmac030_buffer *buffer_info;

	char name[IFNAMSIZ + 5];
	u32 ims_val;
	u32 itr_val;
	int set_itr;

	struct sk_buff *rx_skb_top;
};

/* board specific private data structure */
struct ftgmac030_ctrl {
	u32 rx_buffer_len;

	/* track device up/down/testing state */
	unsigned long state;

	/* Interrupt Throttle Rate */
	u32 itr;
	u32 itr_setting;
	u16 tx_itr;
	u16 rx_itr;

	/* Tx - one ring per active queue */
	struct ftgmac030_ring *tx_ring ____cacheline_aligned_in_smp;
	u32 tx_fifo_limit;

	//struct napi_struct napi;

	unsigned int uncorr_errors;	/* uncorrectable ECC errors */
	unsigned int corr_errors;	/* correctable ECC errors */
	unsigned int restart_queue;

	u8 tx_timeout_factor;

	u32 tx_int_delay;

	unsigned int total_tx_bytes;
	unsigned int total_tx_packets;
	unsigned int total_rx_bytes;
	unsigned int total_rx_packets;

	/* Tx stats */
	u32 tx_timeout_count;
	u32 tx_dma_failed;
	u32 tx_hwtstamp_timeouts;

	/* Rx */
	bool (*clean_rx)(struct ftgmac030_ring *ring, int *work_done,
			 int work_to_do) ____cacheline_aligned_in_smp;
	void (*alloc_rx_buf)(struct ftgmac030_ring *ring, int cleaned_count,
			     gfp_t gfp);
	struct ftgmac030_ring *rx_ring;

	/* Rx stats */
	u64 hw_csum_err;
	u64 hw_csum_good;
	u64 rx_hdr_split;
	u32 alloc_rx_buff_failed;
	u32 rx_dma_failed;
	u32 rx_hwtstamp_cleared;

	u32 max_frame_size;
	u32 min_frame_size;
	u16 max_tx_fifo;
	u16 max_rx_fifo;

	/* OS defined structs */
	struct eth_device *netdev;

	/* structs defined in e1000_hw.h */
	uint32_t io_base;
	//s32 irq;

	struct mii_if_info    mii;
	int phy_speed;
	int phy_duplex;
	int phy_link;

	u8 mdc_cycthr;

	u32 high_water;          /* Flow control high-water mark */
	u32 low_water;           /* Flow control low-water mark */
	u16 pause_time;          /* Flow control pause timer */

	u16 mta_reg_count;

	/* Maximum size of the MTA register table in all supported hws */
#define MAX_MTA_REG 4
	u32 mta_shadow[MAX_MTA_REG];

	u32 max_hw_frame_size;

	unsigned int flags;

//	int phy_hang_count;

	u16 tx_ring_count;
	u16 rx_ring_count;

#if defined(PTP_ENABLE) // Irene Lin
	struct hwtstamp_config hwtstamp_config;
#endif
	struct sk_buff *tx_hwtstamp_skb;
	unsigned long tx_hwtstamp_start;
	//struct work_struct tx_hwtstamp_work;
	spinlock_t systim_lock;	/* protects SYSTIML/H regsters */
	u32 incval; /* increment value ns for one period */ 
	u32 incval_nns; /* fraction part of ns incval in decimal */

#if defined(PTP_ENABLE) // Irene Lin
	struct ptp_clock *ptp_clock;
	struct ptp_clock_info ptp_clock_info;
#endif
	//u16 eee_advert;

    //======
    unsigned int autong_complete : 1;	/* Auto-Negotiate completed */
    unsigned int mode : 4;            /* real, mac lb, phy pcs lb, phy mdi lb */
    unsigned int is_grmii : 1;         /* is giga ethernet or fast ethernet */
    unsigned int use_gpio36 : 1;       /* 0: GPIO32-35, 1: GPIO36-39 */
    ITE_MAC_CFG_T* cfg;

    void(*netif_rx)(void *arg, void *packet, int len);
    void*           netif_arg;
};

/* hardware capability, feature, and workaround flags */
#define FLAG_DISABLE_AIM                  (1 << 0)
#define FLAG_HAS_HW_TIMESTAMP             (1 << 1)
#define FLAG_HAS_WOL                      (1 << 2)
#define FLAG_HAS_JUMBO_FRAMES             (1 << 3)
#define FLAG_HAS_EEE                      (1 << 4)
#define FLAG_DISABLE_FC_PAUSE_TIME        (1 << 5)
#define FLAG_HAS_HW_VLAN_FILTER           (1 << 6)
#define FLAG_NO_WAKE_UCAST                (1 << 7)
#define FLAG_DISABLE_IPV6_RECOG           (1 << 8)
#define FLAG_DISABLE_RX_BROADCAST_PERIOD  (1 << 9)

#define FTGMAC030_GET_DESC(R, i, type)	(&(((struct type *)((R).desc))[i]))
#define FTGMAC030_TX_DESC(R, i)		FTGMAC030_GET_DESC(R, i, ftgmac030_txdes)
#define FTGMAC030_RX_DESC(R, i)	    	FTGMAC030_GET_DESC(R, i, ftgmac030_rxdes)

enum ftgmac030_state_t {
	__FTGMAC030_TESTING,
	__FTGMAC030_RESETTING,
	__FTGMAC030_ACCESS_SHARED_RESOURCE,
	__FTGMAC030_DOWN
};

enum latency_range {
	lowest_latency = 0,
	low_latency = 1,
	bulk_latency = 2,
	latency_invalid = 255
};

extern char ftgmac030_driver_name[];
extern const char ftgmac030_driver_version[];

void ftgmac030_check_options(struct ftgmac030_ctrl *ctrl);
void ftgmac030_set_ethtool_ops(struct eth_device *netdev);

int ftgmac030_up(struct ftgmac030_ctrl *ctrl);
void ftgmac030_down(struct ftgmac030_ctrl *ctrl, bool reset);
void ftgmac030_reinit_locked(struct ftgmac030_ctrl *ctrl);
void ftgmac030_reset(struct ftgmac030_ctrl *ctrl);
void ftgmac030_power_up_phy(struct ftgmac030_ctrl *ctrl);
int ftgmac030_setup_rx_resources(struct ftgmac030_ring *ring);
int ftgmac030_setup_tx_resources(struct ftgmac030_ring *ring);
void ftgmac030_free_rx_resources(struct ftgmac030_ring *ring);
void ftgmac030_free_tx_resources(struct ftgmac030_ring *ring);
void ftgmac030_reset_interrupt_capability(struct ftgmac030_ctrl *ctrl);
void ftgmac030_get_hw_control(struct ftgmac030_ctrl *ctrl);
void ftgmac030_release_hw_control(struct ftgmac030_ctrl *ctrl);
void ftgmac030_write_itr(struct ftgmac030_ctrl *ctrl, u32 itr);

extern unsigned int copybreak;
#if defined(PTP_ENABLE) // Irene Lin
void ftgmac030_ptp_init(struct ftgmac030_ctrl *ctrl);
void ftgmac030_ptp_remove(struct ftgmac030_ctrl *ctrl);
#endif

static inline u32 __ior32(struct ftgmac030_ctrl *ctrl, unsigned long reg)
{
	return ithReadRegA(ctrl->io_base + reg);
}

#define ior32(reg)	__ior32(ctrl, reg)

static void __iow32(struct ftgmac030_ctrl *ctrl, unsigned long reg, u32 val)
{
    ithWriteRegA(ctrl->io_base + reg, val);
}

#define iow32(reg, val)	__iow32(ctrl, reg, val)
#define writel(val, reg)  ithWriteRegA(reg, val)
#define readl(reg)        ithReadRegA(reg)

#define FTGMAC030_WRITE_REG_ARRAY(reg, offset, value) \
	(__iow32(ctrl, (reg + ((offset) << 2)), (value)))

#define FTGMAC030_READ_REG_ARRAY(a, reg, offset) \
	(__ior32((a)->io_base + reg + ((offset) << 2)))

#endif /* _FTGMAC030_H_ */
