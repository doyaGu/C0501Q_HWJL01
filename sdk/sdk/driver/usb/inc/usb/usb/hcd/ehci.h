/*
 * Copyright (c) 2008 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Definitions used for the EHCI driver.
 *
 * @author Irene Lin
 */
#ifndef USB_EHCI_HCD_H
#define USB_EHCI_HCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../hcd.h"
#include "../otg/otg.h"

#define __hc32 __le32
#define __hc16 __le16

/* statistics can be kept for for tuning/monitoring */
struct ehci_stats {
    /* irq usage */
    unsigned long normal;
    unsigned long error;
    unsigned long reclaim;
    unsigned long lost_iaa;

    /* termination of urbs from core */
    unsigned long complete;
    unsigned long unlink;
};

//===============================================================================
/**
 * EHCI register interface, corresponds to
 * Faraday FOTG210 Revision 1.3 specification
 */
//===============================================================================

/* EHCI Section 2.2 Host Controller Capability Registers */
struct ehci_caps {
    uint32_t hcc_reg;                               /* HC Capability Register - offset 0x0 */

    uint32_t hcs_params;                            /* HCSPARAMS - offset 0x4 */
#define HCS_DEBUG_PORT(p)       (((p) >> 20) & 0xf) /* bits 23:20, debug port? */
#define HCS_INDICATOR(p)        ((p) & (1 << 16))   /* true: has port indicators */
#define HCS_N_CC(p)             (((p) >> 12) & 0xf) /* bits 15:12, #companion HCs */
#define HCS_N_PCC(p)            (((p) >> 8) & 0xf)  /* bits 11:8, ports per CC */
#define HCS_PORTROUTED(p)       ((p) & (1 << 7))    /* true: port routing */
#define HCS_PPC(p)              ((p) & (1 << 4))    /* true: port power control */
#define HCS_N_PORTS(p)          (((p) >> 0) & 0xf)  /* bits 3:0, ports on HC */

    uint32_t hcc_params;                            /* HCCPARAMS - offset 0x8 */
#define HCC_EXT_CAPS(p)         (((p) >> 8) & 0xff) /* for pci extended caps */
#define HCC_ISOC_CACHE(p)       ((p) & (1 << 7))    /* true: can cache isoc frame */
#define HCC_ISOC_THRES(p)       (((p) >> 4) & 0x7)  /* bits 6:4, uframes cached */
#define HCC_CANPARK(p)          ((p) & (1 << 2))    /* true: can park on async qh */
#define HCC_PGM_FRAMELISTLEN(p) ((p) & (1 << 1))    /* true: periodic_size changes*/
#define HCC_64BIT_ADDR(p)       ((p) & (1))         /* true: can use 64-bit addr */
#if 0
    uint32_t portroute [2];                         /* nibbles for routing - offset 0xC */
#endif
};

/* EHCI Section 2.3 Host Controller Operational Registers */
struct ehci_regs {
    /* USBCMD: offset 0x00 */
    uint32_t command;
/* 23:16 is r/w intr rate, in microframes; default "8" == 1/msec */
#define CMD_PARK   (1 << 11)             /* enable "park" on async qh */
#define CMD_PARK_CNT(c) (((c) >> 8) & 3) /* how many transfers to park for */
#define CMD_LRESET (1 << 7)              /* partial reset (no ports, etc) */
#define CMD_IAAD   (1 << 6)              /* "doorbell" interrupt async advance */
#define CMD_ASE    (1 << 5)              /* async schedule enable */
#define CMD_PSE    (1 << 4)              /* periodic schedule enable */
/* 3:2 is periodic frame list size */
#define CMD_RESET  (1 << 1)              /* reset HC not bus */
#define CMD_RUN    (1 << 0)              /* start/stop HC */

    /* USBSTS: offset 0x04 */
    uint32_t status;
#define STS_ASS    (1 << 15)    /* Async Schedule Status */
#define STS_PSS    (1 << 14)    /* Periodic Schedule Status */
#define STS_RECL   (1 << 13)    /* Reclamation */
#define STS_HALT   (1 << 12)    /* Not running (any reason) */
/* some bits reserved */
    /* these STS_* flags are also intr_enable bits (USBINTR) */
#define STS_IAA    (1 << 5)     /* Interrupted on async advance */
#define STS_FATAL  (1 << 4)     /* such as some PCI access errors */
#define STS_FLR    (1 << 3)     /* frame list rolled over */
#define STS_PCD    (1 << 2)     /* port change detect */
#define STS_ERR    (1 << 1)     /* "error" completion (overflow, ...) */
#define STS_INT    (1 << 0)     /* "normal" completion (short, ...) */

    /* USBINTR: offset 0x08 */
    uint32_t intr_enable;

    /* FRINDEX: offset 0x0C */
    uint32_t frame_index;       /* current microframe number */
    /* CTRLDSSEGMENT: offset 0x10 */
    uint32_t segment;           /* address bits 63:32 if needed */
    /* PERIODICLISTBASE: offset 0x14 */
    uint32_t frame_list;        /* points to periodic list */
    /* ASYNCICLISTADDR: offset 0x18 */
    uint32_t async_next;        /* address of next async queue head */

    /* CONFIGFLAG: offset 0x40 */
    uint32_t configured_flag;
#define FLAG_CF            (1 << 0) /* true: we'll support "high speed" */

    /* PORTSC: offset 0x20 */
    uint32_t port_status[1];         /* up to N_PORTS */
/* 31:23 reserved */
#define PORT_WKOC_E        (1 << 22) /* wake on overcurrent (enable) */
#define PORT_WKDISC_E      (1 << 21) /* wake on disconnect (enable) */
#define PORT_WKCONN_E      (1 << 20) /* wake on connect (enable) */
/* 19:16 for port testing */
#define PORT_TEST_SHT      16
#define PORT_TEST_MSK      (0xF << 16) /* test mode mask */
#define PORT_TEST_J_STATE  (0x1 << 16) /* Test J_STATE */
#define PORT_TEST_K_STATE  (0x2 << 16) /* Test K_STATE */
#define PORT_TEST_SE0_NAK  (0x3 << 16) /* Test SE0_NAK */
#define PORT_TEST_PACKET   (0x4 << 16) /* Test Packet */
#define PORT_TEST_FORCE_EN (0x5 << 16) /* Test FORCE_ENABLE */
/* 9 reserved */
#define PORT_RESET         (1 << 8)    /* reset port */
#define PORT_SUSPEND       (1 << 7)    /* suspend port */
#define PORT_RESUME        (1 << 6)    /* resume it */

#define PORT_PEC           (1 << 3)    /* port enable change */
#define PORT_PE            (1 << 2)    /* port enable */
#define PORT_CSC           (1 << 1)    /* connect status change */
#define PORT_CONNECT       (1 << 0)    /* device connected */

    uint32_t reserved3[3];
    uint32_t hc_misc;
};

//===============================================================================
/** ehci_hcd->lock guards shared data against other CPUs:
 *   ehci_hcd:	async, reclaim, periodic (and shadow), ...
 *   hcd_dev:	ep[]
 *   ehci_qh:	qh_next, qtd_list
 *   ehci_qtd:	qtd_list
 *
 * Also, hold this lock when talking to HC registers or
 * when updating hw_* fields in shared qh/qtd/... structures.
 */
//===============================================================================
#define EHCI_MAX_ROOT_PORTS 0          /* see HCS_N_PORTS */
#define EHCI_QH_NUM         8
#define EHCI_QTD_NUM        300
#define EHCI_ITD_NUM        320 // ISO TODO
#define EHCI_SITD_NUM       320 // ISO TODO

struct ehci_hcd               /* one per controller */
{
    spinlock_t     lock;

    /* async schedule support */
    struct ehci_qh *async;
    struct ehci_qh *reclaim;
    uint32_t       scanning : 1;

    /* periodic schedule support */
#define DEFAULT_I_TDPS 1024             /* some HCs can do less */
    uint32_t           periodic_size;
    uint32_t           *periodic;       /* hw periodic table */
    uint32_t           periodic_addr;
    uint32_t           i_thresh;        /* uframes HC might cache */

    union ehci_shadow  *pshadow;        /* mirror hw periodic table */
    int                next_uframe;     /* scan periodic, start here */
    uint32_t           periodic_sched;  /* periodic activity count */

    /* per root hub port */
    uint32_t           reset_done [EHCI_MAX_ROOT_PORTS];

    /* glue to PCI and HCD framework */
    struct ehci_caps   caps;
    struct ehci_regs   regs;
    struct otg_regs    otg_regs;
    struct global_regs global_regs;
    uint32_t           hc_caps;
    uint32_t           hcs_params;      /* cached register copy */
    uint32_t           hcc_params;

    /* per-HC memory pools (could be per-PCI-bus, but ...) */
    uint8_t            *qh_pool;        /* qh per active urb */
    uint8_t            *qtd_pool;       /* one or more per qh */
    uint8_t            *itd_pool;       /* itd per iso urb */
    uint8_t            *sitd_pool;      /* sitd per split iso urb */

    uint8_t            qh_manage[EHCI_QH_NUM];
    uint8_t            qtd_manage[EHCI_QTD_NUM];
    uint8_t            itd_manage[EHCI_ITD_NUM];
    uint8_t            sitd_manage[EHCI_SITD_NUM];
#define EHCI_MEM_FREE 0
#define EHCI_MEM_USED 1

    uint32_t          command;
    volatile uint32_t tasklet;

    /* irq statistics */
#ifdef EHCI_STATS
    struct ehci_stats stats;
    #define COUNT(x)       do { (x)++; } while (0)
#else
    #define COUNT(x)       do {} while (0)
#endif
};
#define to_ehci_work(ehci) (ehci->tasklet = 2)

/* convert between an HCD pointer and the corresponding EHCI_HCD */
static inline struct ehci_hcd *hcd_to_ehci(struct usb_hcd *hcd)
{
    return (struct ehci_hcd *) (hcd->hcd_priv);
}

static inline struct usb_hcd *ehci_to_hcd(struct ehci_hcd *ehci)
{
    return list_entry((void *) ehci, struct usb_hcd, hcd_priv);
}

#define cpu_to_hc32(ehci, x)  cpu_to_le32(x)
#define hc32_to_cpu(ehci, x)  le32_to_cpu(x)
#define hc32_to_cpup(ehci, x) le32_to_cpu((*x))

/* NOTE:  urb->transfer_flags expected to not use this bit !!! */
#define EHCI_STATE_UNLINK 0x8000        /* urb being unlinked */

//===============================================================================
/*
 * EHCI Specification 1.0 Section 3.5
 * QTD: describe data transfer components (buffer, direction, ...)
 * See Fig 3-6 "Queue Element Transfer Descriptor Block Diagram".
 *
 * These are associated only with "QH" (Queue Head) structures,
 * used with control, bulk, and interrupt transfers.
 */
//===============================================================================
#define QTD_NEXT(ehci, dma) cpu_to_hc32(ehci, (u32)dma)

/* for periodic/async schedules and qtd lists, mark end of list */
#define EHCI_LIST_END(x)    cpu_to_le32(1) /* "null pointer" to hw */

struct ehci_qtd {
    /* first part defined by EHCI spec */
    uint32_t hw_next;               /* see EHCI 3.5.1 */
    uint32_t hw_alt_next;           /* see EHCI 3.5.2 */
    uint32_t hw_token;              /* see EHCI 3.5.3 */
#define QTD_TOGGLE     (1 << 31)    /* data toggle */
#define QTD_LENGTH(tok) (((tok) >> 16) & 0x7fff)
#define QTD_IOC        (1 << 15)    /* interrupt on complete */
#define QTD_CERR(tok)   (((tok) >> 10) & 0x3)
#define QTD_PID(tok)    (((tok) >> 8) & 0x3)
#define QTD_STS_ACTIVE (1 << 7)     /* HC may execute this */
#define QTD_STS_HALT   (1 << 6)     /* halted on error */
#define QTD_STS_DBE    (1 << 5)     /* data buffer error (in HC) */
#define QTD_STS_BABBLE (1 << 4)     /* device was babbling (qtd halted) */
#define QTD_STS_XACT   (1 << 3)     /* device gave illegal response */
#define QTD_STS_MMF    (1 << 2)     /* incomplete split transaction */
#define QTD_STS_STS    (1 << 1)     /* split transaction state */
#define QTD_STS_PING   (1 << 0)     /* issue PING? */

#define ACTIVE_BIT(ehci) cpu_to_le32(QTD_STS_ACTIVE)
#define HALT_BIT(ehci)   cpu_to_le32(QTD_STS_HALT)
#define STATUS_BIT(ehci) cpu_to_le32(QTD_STS_STS)

    uint32_t         hw_buf [5];     /* see EHCI 3.5.4 */

    /* the rest is HCD-private */
    dma_addr_t       qtd_dma;           /* qtd address */
    struct list_head qtd_list;          /* sw qtd list */

    struct urb       *urb;              /* qtd's urb */
    uint8_t          *buf_addr;         /* buffer address */
    uint32_t         length;            /* length of buffer */

    uint32_t         buf_index;
};

#define IS_SHORT_READ(token)   (QTD_LENGTH(token) != 0 && QTD_PID(token) == 1)

/*-------------------------------------------------------------------------*/

/* type tag from {qh,itd,sitd,fstn}->hw_next */
#define Q_NEXT_TYPE(ehci, dma) ((dma) & cpu_to_le32(3 << 1))

/* values for that type tag */
#define Q_TYPE_ITD  (0 << 1)
#define Q_TYPE_QH   (1 << 1)
#define Q_TYPE_SITD (2 << 1)
#define Q_TYPE_FSTN (3 << 1)

/* next async queue entry, or pointer to interrupt/periodic QH */
#define QH_NEXT(ehci, addr) (cpu_to_hc32(ehci, (((uint32_t)addr) & ~0x01f) | Q_TYPE_QH))

/*
 * Entries in periodic shadow table are pointers to one of four kinds
 * of data structure.  That's dictated by the hardware; a type tag is
 * encoded in the low bits of the hardware's periodic schedule.  Use
 * Q_NEXT_TYPE to get the tag.
 *
 * For entries in the async schedule, the type tag always says "qh".
 */
union ehci_shadow {
    struct ehci_qh   *qh;           /* Q_TYPE_QH */
    struct ehci_itd  *itd;          /* Q_TYPE_ITD */
    struct ehci_sitd *sitd;         /* Q_TYPE_SITD */
    struct ehci_fstn *fstn;         /* Q_TYPE_FSTN */
    uint32_t         *hw_next;      /* (all types) */
    void             *ptr;
};

//===============================================================================
/*
 * EHCI Specification 1.0 Section 3.6
 * QH: describes control/bulk/interrupt endpoints
 * See Fig 3-7 "Queue Head Structure Layout".
 *
 * These appear in both the async and (for interrupt) periodic schedules.
 */
//===============================================================================
struct ehci_qh {
    /* first part defined by EHCI spec */
    uint32_t hw_next;                   /* see EHCI 3.6.1 */
    uint32_t hw_info1;                  /* see EHCI 3.6.2 */
#define QH_HEAD    0x00008000
    uint32_t hw_info2;                  /* see EHCI 3.6.2 */
#define QH_SMASK   0x000000ff
#define QH_CMASK   0x0000ff00
#define QH_HUBADDR 0x007f0000
#define QH_HUBPORT 0x3f800000
#define QH_MULT    0xc0000000
    uint32_t          hw_current;       /* qtd list - see EHCI 3.6.4 */

    /* qtd overlay (hardware parts of a struct ehci_qtd) */
    uint32_t          hw_qtd_next;
    uint32_t          hw_alt_next;
    uint32_t          hw_token;
    uint32_t          hw_buf0;
    uint32_t          hw_buf1;
    uint32_t          hw_buf2;
    uint32_t          hw_buf3;
    uint32_t          hw_buf4;

    /* the rest is HCD-private */
    dma_addr_t        qh_dma;       /* address of qh */
    union ehci_shadow qh_next;      /* ptr to qh; or periodic */
    struct list_head  qtd_list;     /* sw qtd list */
    struct ehci_qtd   *dummy;
    struct ehci_qh    *reclaim;     /* next to reclaim */
    struct ehci_hcd   *ehci;

    uint8_t           qh_state;
#define QH_STATE_LINKED      1      /* HC sees this */
#define QH_STATE_UNLINK      2      /* HC may still see this */
#define QH_STATE_IDLE        3      /* HC doesn't see this */
#define QH_STATE_UNLINK_WAIT 4      /* LINKED and on reclaim q */
#define QH_STATE_COMPLETING  5      /* don't touch token.HALT */

    uint8_t xacterrs;               /* XactErr retry counter */
#define QH_XACTERR_MAX       32     /* XactErr retry limit */

    /* periodic schedule info */
    uint8_t           usecs;        /* intr bandwidth */
    uint8_t           gap_uf;       /* uframes split/csplit gap */
    uint8_t           c_usecs;      /* ... split completion bw */

    uint16_t          tt_usecs;     /* tt downstream bandwidth */
    uint16_t          reserved;
    uint16_t          period;       /* polling interval */
    uint16_t          start;        /* where polling starts */
#define NO_FRAME ((uint16_t) ~0)    /* pick new start */

    struct usb_device *dev;

    uint32_t          buf_index;
    uint32_t          refcount;
};

/*-------------------------------------------------------------------------*/

/* description of one iso transaction (up to 3 KB data if highspeed) */
struct ehci_iso_packet {
	/* These will be copied to iTD when scheduling */
	uint64_t			bufp;		/* itd->hw_bufp{,_hi}[pg] |= */
	uint32_t			transaction;	/* itd->hw_transaction[i] |= */
	uint8_t			cross;		/* buf crosses pages */
	/* for full speed OUT splits */
	uint32_t			buf1;
};

/* temporary schedule data for packets from iso urbs (both speeds)
 * each packet is one logical usb transaction to the device (not TT),
 * beginning at stream->next_uframe
 */
struct ehci_iso_sched {
	struct list_head	td_list;
	unsigned		span;
	struct ehci_iso_packet	packet [0];
};

/*
 * ehci_iso_stream - groups all (s)itds for this endpoint.
 * acts like a qh would, if EHCI had them for ISO.
 */
struct ehci_iso_stream {
	/* first two fields match QH, but info1 == 0 */
	uint32_t			hw_next;
	uint32_t			hw_info1;

	uint32_t			refcount;
	uint8_t			bEndpointAddress;
	uint8_t			highspeed;
	uint16_t			depth;		/* depth in uframes */
	struct list_head	td_list;	/* queued itds/sitds */
	struct list_head	free_list;	/* list of unused itds/sitds */
	struct usb_device	*udev;
	struct usb_host_endpoint *ep;

	/* output of (re)scheduling */
	unsigned long		start;		/* jiffies */
	unsigned long		rescheduled;
	int			next_uframe;
	uint32_t			splits;

	/* the rest is derived from the endpoint descriptor,
	 * trusting urb->interval == f(epdesc->bInterval) and
	 * including the extra info for hw_bufp[0..2]
	 */
	uint8_t			usecs, c_usecs;
	uint16_t			interval;
	uint16_t			tt_usecs;
	uint16_t			maxp;
	uint16_t			raw_mask;
	unsigned		bandwidth;

	/* This is used to initialize iTD's hw_bufp fields */
	uint32_t			buf0;
	uint32_t			buf1;
	uint32_t			buf2;

	/* this is used to initialize sITD's tt info */
	uint32_t			address;
};

#define ehci_invalid_qh(qh)   ithInvalidateDCacheRange((void *)(qh), sizeof(uint32_t) * 12)
#define ehci_invalid_qtd(qtd) ithInvalidateDCacheRange((void *)(qtd), sizeof(uint32_t) * 4)
#define ehci_invalid_itd(itd) ithInvalidateDCacheRange((void *)(itd), sizeof(uint32_t) * 12)
#define ehci_invalid_sitd(sitd) ithInvalidateDCacheRange((void *)(sitd), sizeof(uint32_t) * 4)
//===============================================================================
/*
 * EHCI Specification 1.0 Section 3.3
 * Fig 3-4 "Isochronous Transaction Descriptor (iTD)"
 *
 * Schedule records for high speed iso xfers
 */
//===============================================================================
struct ehci_itd {
    /* first part defined by EHCI spec */
    uint32_t hw_next;                   /* see EHCI 3.3.1 */
    uint32_t hw_transaction [8];        /* see EHCI 3.3.2 */
#define EHCI_ISOC_ACTIVE  (1 << 31)     /* activate transfer this slot */
#define EHCI_ISOC_BUF_ERR (1 << 30)     /* Data buffer error */
#define EHCI_ISOC_BABBLE  (1 << 29)     /* babble detected */
#define EHCI_ISOC_XACTERR (1 << 28)     /* XactErr - transaction error */
#define EHCI_ITD_LENGTH(tok) (((tok) >> 16) & 0x7fff)
#define EHCI_ITD_IOC      (1 << 15)     /* interrupt on complete */

#define ITD_ACTIVE(ehci)	cpu_to_hc32(ehci, EHCI_ISOC_ACTIVE)

	uint32_t			hw_bufp [7];	/* see EHCI 3.3.3 */
	uint32_t			hw_bufp_hi [7];	/* Appendix B */

	/* the rest is HCD-private */
	dma_addr_t		itd_dma;	/* for this itd */
	union ehci_shadow	itd_next;	/* ptr to periodic q entry */

	struct urb		*urb;
	struct ehci_iso_stream	*stream;	/* endpoint's queue */
	struct list_head	itd_list;	/* list of stream's itds */

	/* any/all hw_transactions here may be used by that urb */
	unsigned		frame;		/* where scheduled */
	unsigned		pg;
	unsigned		index[8];	/* in urb->iso_frame_desc */

    uint32_t         buf_index;	
};

//===============================================================================
/*
 * EHCI Specification 1.0 Section 3.4
 * siTD, aka split-transaction isochronous Transfer Descriptor
 *       ... describe low/full speed iso xfers through TT in hubs
 * see Figure 3-5 "Split-transaction Isochronous Transaction Descriptor (siTD)
 */
//===============================================================================
struct ehci_sitd {
    /* first part defined by EHCI spec */
    uint32_t          hw_next;
/* uses bit field macros above - see EHCI 0.95 Table 3-8 */
    uint32_t          hw_fullspeed_ep;    /* see EHCI table 3-9 */
    uint32_t          hw_uframe;          /* see EHCI table 3-10 */
	uint32_t		  hw_results;		/* EHCI table 3-11 */
#define	SITD_IOC	(1 << 31)	/* interrupt on completion */
#define	SITD_PAGE	(1 << 30)	/* buffer 0/1 */
#define	SITD_LENGTH(x)	(0x3ff & ((x)>>16))
#define	SITD_STS_ACTIVE	(1 << 7)	/* HC may execute this */
#define	SITD_STS_ERR	(1 << 6)	/* error from TT */
#define	SITD_STS_DBE	(1 << 5)	/* data buffer error (in HC) */
#define	SITD_STS_BABBLE	(1 << 4)	/* device was babbling */
#define	SITD_STS_XACT	(1 << 3)	/* illegal IN response */
#define	SITD_STS_MMF	(1 << 2)	/* incomplete split transaction */
#define	SITD_STS_STS	(1 << 1)	/* split transaction state */

#define SITD_ACTIVE(ehci)	cpu_to_hc32(ehci, SITD_STS_ACTIVE)
	uint32_t			hw_buf [2];		/* EHCI table 3-12 */
	uint32_t			hw_backpointer;		/* EHCI table 3-13 */
	uint32_t			hw_buf_hi [2];		/* Appendix B */

	/* the rest is HCD-private */
	dma_addr_t		sitd_dma;
	union ehci_shadow	sitd_next;	/* ptr to periodic q entry */

	struct urb		*urb;
	struct ehci_iso_stream	*stream;	/* endpoint's queue */
	struct list_head	sitd_list;	/* list of stream's sitds */
	unsigned		frame;
	unsigned		index;
	
    uint32_t         buf_index;	
};

//===============================================================================
/*
 * EHCI Specification 1.0 Section 3.7
 * Periodic Frame Span Traversal Node (FSTN)
 *
 * Manages split interrupt transactions (using TT) that span frame boundaries
 * into uframes 0/1; see 4.12.2.2.  In those uframes, a "save place" FSTN
 * makes the HC jump (back) to a QH to scan for fs/ls QH completions until
 * it hits a "restore" FSTN; then it returns to finish other uframe 0/1 work.
 */
//===============================================================================
struct ehci_fstn {
    uint32_t          hw_next;      /* any periodic q entry */
    uint32_t          hw_prev;      /* qh or EHCI_LIST_END */

    /* the rest is HCD-private */
    uint8_t           *fstn_dma;
    union ehci_shadow fstn_next;    /* ptr to periodic q entry */
};

int ehci_port_reset(struct ehci_hcd *ehci);
int ehci_get_speed(struct ehci_hcd *ehci);
int ehci_start(struct usb_hcd *hcd, struct usb_device **usb_device);
void ehci_work(struct ehci_hcd *ehci);
void ehci_irq(struct usb_hcd *hcd);

static inline int _ehci_dev_exist(struct ehci_hcd *ehci)
{
    uint32_t status = ithReadRegA(ehci->regs.port_status[0]);
    if (status & PORT_CONNECT)
        return 1;
    else
    {
        ehci_to_hcd(ehci)->state = HC_STATE_HALT;
        return 0;
    }
}

/*--------------------------------------------------*/
#define EHCI_WARN 0
#define EHCI_DBG  0

#if EHCI_WARN
    #define ehci_warn(ddev, string, args ...) do { ithPrintf("[EHCI][WARN]"); \
                                                   ithPrintf(string, ## args); \
} while (0)
#else
    #define ehci_warn(ddev, string, args ...)
#endif

#if EHCI_DBG
    #define ehci_vdbg(ddev, string, args ...) do { ithPrintf("[EHCI][DBG]");  \
                                                   ithPrintf(string, ## args); \
} while (0)
    #define ehci_dbg(ddev, string, args ...)  do { ithPrintf("[EHCI][DBG]");  \
                                                   ithPrintf(string, ## args); \
} while (0)
#else
    #define ehci_vdbg(ddev, string, args ...)
    #define ehci_dbg(ddev, string, args ...)
#endif

#define ehci_err(ddev, string, args ...) do { ithPrintf("[EHCI][ERR]"); \
                                              ithPrintf(string, ## args); \
                                              ithPrintf("	%s:%d\n", __FUNCTION__, __LINE__); \
} while (0)

/*--------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif // #ifndef USB_EHCI_HCD_H