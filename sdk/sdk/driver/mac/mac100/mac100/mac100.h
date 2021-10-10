/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 *  Ethernet Controller private data header file.
 *
 * @author Irene Lin
 * @version 1.0
 */

#ifndef MAC100_H
#define MAC100_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include "mii.h"
#include "mac100/config.h"
#include "mac100/mac100_reg.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
/* private data */
struct ftmac100_descs {
    struct ftmac100_rxdes rxdes[RX_QUEUE_ENTRIES];
    struct ftmac100_txdes txdes[TX_QUEUE_ENTRIES];
};


struct ftmac100
{
    MAC_UINT32      base;
    MAC_SEM         tx_lock;
    int             linkGpio;

    void (*netif_rx)(void *arg, void *packet, int len);
    void*           netif_arg;

    MAC_UINT32      rx_pointer;
    MAC_UINT32      tx_clean_pointer;
    MAC_UINT32      tx_pointer;
    MAC_UINT32      tx_pending;

    MAC_UINT32 autong_complete : 1;	/* Auto-Negotiate completed */
    MAC_UINT32 mode : 3;            /* real, mac lb, phy pcs lb, phy mdi lb */

    struct eth_device     *netdev;
    struct ftmac100_descs *descs;
    struct mii_if_info    mii;
	ITE_MAC_CFG_T* cfg;

    MAC_UINT32      rx_flag;   /* see <if.h> IFF_PROMISC, IFF_ALLMULTI, IFF_MULTICAST */
    int             mc_count;  /* Multicast mac addresses count */
    MAC_UINT32      mc_ht0;    /* Multicast hash table 0 */
    MAC_UINT32      mc_ht1;    /* Multicast hash table 1 */
};

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//							Funtion Declaration
//=============================================================================


#ifdef __cplusplus
}
#endif

#endif //MAC100_H
