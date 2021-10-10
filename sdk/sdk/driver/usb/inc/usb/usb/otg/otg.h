/*
 * Copyright (c) 2008 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * OTG related.
 *
 * @author Irene Lin
 */

#ifndef USB_EX_OTG_H
#define USB_EX_OTG_H

#ifdef __cplusplus
extern "C" {
#endif

struct otg_regs {
    /** 0x80 */
    uint32_t ctrl_status;
#define HOST_SPEED(p)        (((p) >> 22) & 0x3)   /** host speed type */
#define FULL_SPEED           0
#define LOW_SPEED            1
#define HIGH_SPEED           2
#define CURR_ID(p)           ((p) >> 21)           /** for otg current ID */
#define ID_A                 0
#define ID_B                 1
#define CURR_ROLE(p)         ((p) >> 20)           /** for otg current role */
#define ROLE_HOST            0
#define ROLE_DEVICE          1
#define A_SET_B_HNP_EN       (1 << 6)
#define A_DEVICE_BUS_DROP    (1 << 5)
#define A_DEVICE_BUS_REQUEST (1 << 4)
#define B_DSCHRG_VBUS        (1 << 2)
#define B_HNP_EN             (1 << 1)
#define B_BUS_REQUEST        (1 << 0)
    /** 0x84 */
    uint32_t intr_status;
    uint32_t intr_enable;
#define OTGC_INT_BSRPDN      (1 << 0)
#define OTGC_INT_ASRPDET     (1 << 4)
#define OTGC_INT_AVBUSERR    (1 << 5)
#define OTGC_INT_RLCHG       (1 << 8)
#define OTGC_INT_IDCHG       (1 << 9)
#define OTGC_INT_OVC         (1 << 10)
#define OTGC_INT_BPLGRMV     (1 << 11)
#define OTGC_INT_APLGRMV     (1 << 12)

    uint32_t curr_role;
};

#define OTGC_INT_A_TYPE      (OTGC_INT_ASRPDET |     \
                              OTGC_INT_AVBUSERR |    \
                              OTGC_INT_RLCHG |       \
                              OTGC_INT_IDCHG |       \
                              OTGC_INT_OVC |         \
                              OTGC_INT_BPLGRMV |     \
                              OTGC_INT_APLGRMV)

#define OTGC_INT_B_TYPE      (OTGC_INT_BSRPDN |      \
                              OTGC_INT_AVBUSERR |    \
                              OTGC_INT_RLCHG |       \
                              OTGC_INT_IDCHG |       \
                              OTGC_INT_OVC)

struct global_regs {
    uint32_t intr_status;
#define G_HC_INTR            (1 << 2)
#define G_OTG_INTR           (1 << 1)
#define G_DEVICE_INTR        (1 << 0)
    uint32_t intr_mask;
};

void USBEX_OTGSetup(struct ehci_hcd *ehci);

#ifdef __cplusplus
}
#endif

#endif