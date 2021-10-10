

struct ftgmac030_reg_info {
	u32 ofs;
	char *name;
};

static const struct ftgmac030_reg_info ftgmac030_reg_info_tbl[] = {
	/* Interrupt Registers */
	{FTGMAC030_REG_ISR, "ISR"},
	{FTGMAC030_REG_IER, "IER"},
	/* MAC address Registers */
	{FTGMAC030_REG_MAC_MADR, "MAC_MADR"},
	{FTGMAC030_REG_MAC_LADR, "MAC_LADR"},

	/* Multicast Address Hash table */
	{FTGMAC030_REG_MAHT0, "MAHT0"},
	{FTGMAC030_REG_MAHT1, "MAHT1"},

	/* Tx/Rx Descriptor base address */
	{FTGMAC030_REG_NPTXR_BADR, "NPTXR_BADR"},
	{FTGMAC030_REG_RXR_BADR, "RXR_BADR"},
	{FTGMAC030_REG_HPTXR_BADR, "HPTXR_BADR"},

	/* Interrupt Time Controll */
	{FTGMAC030_REG_TXITC, "TXITC"},
	{FTGMAC030_REG_RXITC, "RXITC"},

	/* DMA and FIFO related Registers */	
	{FTGMAC030_REG_APTC, "APTC"},
	{FTGMAC030_REG_DBLAC, "DBLAC"},
	{FTGMAC030_REG_DMAFIFOS, "DMAFIFOS"},
	{FTGMAC030_REG_TPAFCR, "TPAFCR"},

	/* Receive Buffer Size */
	{FTGMAC030_REG_RBSR, "RXBUF_SIZE"},

	/* MAC control and status */
	{FTGMAC030_REG_MACCR, "MACCR"},
	{FTGMAC030_REG_MACSR, "MACSR"},

	/* Phy Access Registers */
	{FTGMAC030_REG_PHYCR, "PHYCR"},
	{FTGMAC030_REG_PHYDATA, "PHYDATA"},

	/* Flow Control */
	{FTGMAC030_REG_FCR, "FCR"},

	{FTGMAC030_REG_BPR, "BPR"},

	/* Wake On LAN Registers */
	{FTGMAC030_REG_WOLCR, "WOLCR"},
	{FTGMAC030_REG_WOLSR, "WOLSR"},
	{FTGMAC030_REG_WFCRC, "WFCRC"},
	{FTGMAC030_REG_WFBM1, "WFBM"},

	/* Tx/Rx ring pointer Registers */
	{FTGMAC030_REG_NPTXR_PTR, "NPTXR_PTR"},
	{FTGMAC030_REG_HPTXR_PTR, "HPTXR_PTR"},
	{FTGMAC030_REG_RXR_PTR, "RXR_PTR"},

	/* Statistics value Registers */
	{FTGMAC030_REG_TX_CNT, "TX_CNT"},
	{FTGMAC030_REG_TX_MCOL_SCOL, "TX_MCOL_SCOL"},
	{FTGMAC030_REG_TX_ECOL_FAIL, "TX_ECOL_FAIL"},
	{FTGMAC030_REG_TX_LCOL_UND, "TX_LCOL_UND"},
	{FTGMAC030_REG_RX_CNT, "RX_CNT"},
	{FTGMAC030_REG_RX_BC, "RX_BC"},
	{FTGMAC030_REG_RX_MC, "RX_MC"},
	{FTGMAC030_REG_RX_PF_AEP, "RX_PF_AEP"},
	{FTGMAC030_REG_RX_RUNT, "RX_RUNT"},
	{FTGMAC030_REG_RX_CRCER_FTL, "RX_CRCER_FTL"},
	{FTGMAC030_REG_RX_COL_LOST, "RX_COL_LOST"},

	/* Broadcast and Multicast received control */
	{FTGMAC030_REG_BMRCR, "BMRCR"},

	/* Error and Debug Registers */
	{FTGMAC030_REG_ERCR, "ERCR"},
	{FTGMAC030_REG_ACIR, "ACIR"},

	/* Phy Interface */
	{FTGMAC030_REG_GISR, "GISR"},

	{FTGMAC030_REG_EEE, "EEE"},

	/* Revision and Feature */
	{FTGMAC030_REG_REVR, "REVR"},
	{FTGMAC030_REG_FEAR, "FEAR"},

	/* PTP Registers */
	{FTGMAC030_REG_PTP_TX_SEC, "PTP_TX_SEC"},
	{FTGMAC030_REG_PTP_TX_NSEC, "PTP_TX_NSEC"},
	{FTGMAC030_REG_PTP_RX_SEC, "PTP_RX_SEC"},
	{FTGMAC030_REG_PTP_RX_NSEC, "PTP_RX_NSEC"},
	{FTGMAC030_REG_PTP_TX_P_SEC, "PTP_TX_P_SEC"},
	{FTGMAC030_REG_PTP_TX_P_NSEC, "PTP_TX_P_NSEC"},
	{FTGMAC030_REG_PTP_RX_P_SEC, "PTP_RX_P_SEC"},
	{FTGMAC030_REG_PTP_RX_P_NSEC, "PTP_RX_P_NSEC"},
	{FTGMAC030_REG_PTP_TM_NNSEC, "PTP_TM_NNSEC"},
	{FTGMAC030_REG_PTP_TM_NSEC, "PTP_TM_NSEC"},
	{FTGMAC030_REG_PTP_TM_SEC, "PTP_TM_SEC"},
	{FTGMAC030_REG_PTP_NS_PERIOD, "PTP_NS_PERIOD"},
	{FTGMAC030_REG_PTP_NNS_PERIOD, "PTP_NNS_PERIOD"},
	{FTGMAC030_REG_PTP_PERIOD_INCR, "PTP_PERIOD_INCR"},
	{FTGMAC030_REG_PTP_ADJ_VAL, "PTP_ADJ_VAL"},

	/* Clock Delay Registers */
	{FTGMAC030_REG_CLKDLY, "CLOCK_DELAY"},

	/* Multicast Address Hash table extension */
	{FTGMAC030_REG_MAHT2, "MAHT2"},
	{FTGMAC030_REG_MAHT3, "MAHT3"},

	/* List Terminator */
	{0, NULL}
};

/**
 * ftgmac030_dump - Print registers, Tx-ring and Rx-ring
 * @ctrl: board private structure
 **/
void ftgmac030_dump(struct ftgmac030_ctrl *ctrl)
{
	struct eth_device *netdev = ctrl->netdev;
	struct ftgmac030_reg_info *reginfo;
	struct ftgmac030_ring *tx_ring = ctrl->tx_ring;
	struct ftgmac030_txdes *tx_desc;
	struct ftgmac030_buffer *buffer_info;
	struct ftgmac030_ring *rx_ring = ctrl->rx_ring;
	struct ftgmac030_rxdes *rx_desc;
	struct my_u0 {
		__le32 a;
		__le32 b;
		__le32 c;
		__le32 d;
	} *u0;
	int i = 0, j;

	/* Print netdevice Info */
	/* ....nothing..... */

	/* Print Registers */
	printf("Register Dump\n");
	printf(" Register Name   Value\n");
	for (reginfo = (struct ftgmac030_reg_info *)ftgmac030_reg_info_tbl;
	     reginfo->name; reginfo++) {
		char regs1[32], regs2[32];

		snprintf(regs1, 32, "%-15s[%3x] %08x", reginfo->name,
			 reginfo->ofs, ior32(reginfo->ofs));

		reginfo++;
		if (reginfo->name)
			snprintf(regs2, 32, "%-15s[%3x] %08x", reginfo->name,
				 reginfo->ofs, ior32(reginfo->ofs));

		printf("%s,  %s\n", regs1, (reginfo->name) ? regs2 : "");

		if (!reginfo->name)
			break;
	}

	/* Print Tx Ring Summary */
	if (!netdev || !netif_running(netdev))
		return;

	printf("Tx FIFO limit %d\n", ctrl->tx_fifo_limit);

	printf("Tx Ring Summary\n");
	printf("Queue [NTU] [NTC] [ntc->dma] [ head ] leng ntw timestamp\n");
	buffer_info = &tx_ring->buffer_info[tx_ring->next_to_clean];
	printf("%5d %5d %5d  0x%08X  0x%08X 0x%04X %3d 0x%016llX\n",
		0, tx_ring->next_to_use, tx_ring->next_to_clean,
		buffer_info->dma, readl(tx_ring->head),
		buffer_info->length, buffer_info->next_to_watch,
		(unsigned long long)buffer_info->time_stamp);

	printf("Tx Ring Dump\n");

	/* Transmit Descriptor Formats 
	 *
	 *     31   30  29  28    27   26-20    19  18-16  15    14  13       0
	 *   +-----------------------------------------------------------------+
	 * 0 |OWN| Rsvd|FTS|LTS|BUS_ERR|Rsvd|CRC_ERR|Rsvd|EDOTR|Rsvd|TXBUF_SIZE|
	 *   +-----------------------------------------------------------------+
	 *     31    30  29-21  22   21-20   19  18   17   16   15           0
	 *   +----------------------------------------------------------------+
	 * 1 |TXIC|TX2FIC|Rsvd|LLC|PKT_TYPE|IPC|UDPC|TCPC|VLAN|   VLAN Tag    |
	 *   +----------------------------------------------------------------+
	 * 2 |                     Reserved(SW used) [31:0]                   |
	 *   +----------------------------------------------------------------+
	 * 3 |                     Buffer Address [31:0]                      |
	 *   +----------------------------------------------------------------+
	 */
	printf("[desc]   [txdes0] [txdes1] [txdes2] [txdes3] " \
		"[bi->dma] leng  ntw timestamp        bi->skb  next\n");

	i = (readl(tx_ring->head) - ior32(FTGMAC030_REG_NPTXR_BADR)) / 0x10;

	printf("Hardware descriptor index %d\n", i);

	/* Due to HW problem not updated NPTXR_PTR correctly
	 * before transmit a packet
	 */
	if (i > tx_ring->count)
		goto skip_hw_desc;

	j = i + 2;
	for (;tx_ring->desc && (i < j); i++) {

		if (i ==  tx_ring->count)
			i = 0;

		tx_desc = FTGMAC030_TX_DESC(*tx_ring, i);
		buffer_info = &tx_ring->buffer_info[i];
		u0 = (struct my_u0 *)tx_desc;

		printf("[%04d]   %08X %08X %08X %08X  %08X " \
			"%04X  %3X %016llX %p\n", i,
			le32_to_cpu(u0->a), le32_to_cpu(u0->b),
			le32_to_cpu(u0->c), le32_to_cpu(u0->d),
			buffer_info->dma, buffer_info->length,
			buffer_info->next_to_watch,
			(unsigned long long)buffer_info->time_stamp,
			buffer_info->skb);

		#if 0
		if (netif_msg_pktdata(ctrl) && buffer_info->skb)
			print_hex_dump(KERN_INFO, "", DUMP_PREFIX_ADDRESS,
			       16, 1, buffer_info->skb->data,
			       (buffer_info->skb->len < 128) ?
			       buffer_info->skb->len : 128, true);
		#endif
	}

skip_hw_desc:
	/* Print Tx Ring */
	printf("Software descriptors content\n");
	for (i = 0; tx_ring->desc && (i < tx_ring->count); i++) {
		const char *next_desc;
		tx_desc = FTGMAC030_TX_DESC(*tx_ring, i);
		buffer_info = &tx_ring->buffer_info[i];
		u0 = (struct my_u0 *)tx_desc;

		if ((i != tx_ring->next_to_use) &&
		    (i != tx_ring->next_to_clean) &&
		    (!u0->a && !u0->b && !u0->c && !u0->d))
			continue;

		if (i == tx_ring->next_to_use && i == tx_ring->next_to_clean)
			next_desc = " NTC/U";
		else if (i == tx_ring->next_to_use)
			next_desc = " NTU";
		else if (i == tx_ring->next_to_clean)
			next_desc = " NTC";
		else
			next_desc = "";
		printf("[%04d]   %08X %08X %08X %08X  %08X " \
			"%04X  %3d %016llX %p %s\n", i,
			le32_to_cpu(u0->a), le32_to_cpu(u0->b),
			le32_to_cpu(u0->c), le32_to_cpu(u0->d),
			buffer_info->dma, buffer_info->length,
			buffer_info->next_to_watch,
			(unsigned long long)buffer_info->time_stamp,
			buffer_info->skb, next_desc);
		#if 0
		if (netif_msg_pktdata(ctrl) && buffer_info->skb)
			print_hex_dump(KERN_INFO, "", DUMP_PREFIX_ADDRESS,
			       16, 1, buffer_info->skb->data,
			       (buffer_info->skb->len < 128) ?
			       buffer_info->skb->len : 128, true);
		#endif
	}

	/* Print Rx Ring Summary */
rx_ring_summary:
	printf("Rx Ring Summary\n");
	printf("Queue [NTU] [NTC] [ head ] [ tail ]\n");
	printf(" %3d   %3d   %3d  %08X  %08X\n",
		0, rx_ring->next_to_use, rx_ring->next_to_clean,
		readl(rx_ring->head), readl(rx_ring->tail));

	/* Print Rx Ring */
	printf("Rx Ring Dump, mtu %d bytes\n",
			     ctrl->netdev->mtu);
	/* Receive Descriptor Format
	 *    31   30  29  28  27   26  25 24 23 22  21   20  19    18   17  16   15    14  13-0
	 *   +-----------------------------------------------------------------------------------+
	 *   |RDY|Rsvd|FRS|LRS|ERR|Rsvd|PF|PC|FF|ODB|RUNT|FTL|CRC|RX_ERR| B | M |EDORR|Rsvd|BYTES|
	 *   +-----------------------------------------------------------------------------------+
	 *
	 *   31-30 29-28 27   26   25   24   23  22  21-20  19 18-16  15      0
	 *   +-----------------------------------------------------------------+
	 * 1 |Rsvd| PTP |IPC|UDPC|TCPC|VLAN|FRAG|LLC|PROTO|IPv6|Rsvd| VLAN Tag | 
	 *   +-----------------------------------------------------------------+
	 * 2 |                      Reserved                       |
	 *   +-----------------------------------------------------+
	 * 3 |                Buffer Address [31:0]                |
	 *   +-----------------------------------------------------+
	 */
	printf("[desc] [rxdes0] [rxdes1] [rxdes2] "\
		"[rxdes3] [bi->dma] [bi->skb]   first/last/Ownn\n");

	for (i = 0; i < rx_ring->count; i++) {
		const char *next_desc;
		u32 rxdes0;

		buffer_info = &rx_ring->buffer_info[i];
		rx_desc = FTGMAC030_RX_DESC(*rx_ring, i);
		u0 = (struct my_u0 *)rx_desc;

		if (!u0->a && !u0->b && !u0->c && !u0->d)
			continue;

		if (i == rx_ring->next_to_use)
			next_desc = " NTU";
		else if (i == rx_ring->next_to_clean)
			next_desc = " NTC";
		else
			next_desc = "";

		if (i > (rx_ring->next_to_use - 3) &&
		    i < (rx_ring->next_to_use + 3)) {
			rxdes0 = le32_to_cpu(rx_desc->rxdes0);
			printf("[%04d] %08X %08X %08X %08X  %08X %p%s    %c/%c/%s\n",
				 i,
				le32_to_cpu(u0->a), le32_to_cpu(u0->b),
				le32_to_cpu(u0->c), le32_to_cpu(u0->d),
				buffer_info->dma, buffer_info->skb, next_desc,
				(rxdes0 & FTGMAC030_RXDES0_FRS) ? 'f' : ' ',
				(rxdes0 & FTGMAC030_RXDES0_LRS) ? 'l' : ' ',
				(rxdes0 & FTGMAC030_RXDES0_RXPKT_RDY) ? "sw" : "hw");
		}
	}
}

/**
* ftgmac030_desc_unused - calculate if we have unused descriptors
**/
static int ftgmac030_desc_unused(struct ftgmac030_ring *ring)
{
    if (ring->next_to_clean > ring->next_to_use)
        return ring->next_to_clean - ring->next_to_use;

    return ring->count + ring->next_to_clean - ring->next_to_use;
}

static void ftgmac030_print_hw_hang(struct ftgmac030_ctrl *ctrl)
{
	struct eth_device *netdev = ctrl->netdev;
	struct ftgmac030_ring *tx_ring = ctrl->tx_ring;
	unsigned int i = tx_ring->next_to_clean;
	unsigned int lts = tx_ring->buffer_info[i].next_to_watch;
	struct ftgmac030_txdes *lts_desc = FTGMAC030_TX_DESC(*tx_ring, lts);

	if (test_bit(__FTGMAC030_DOWN, &ctrl->state))
		return;

	netif_stop_queue(netdev);

	/* detected Hardware unit hang */
	e_err("hw:", "Detected Hardware Unit Hang:\n"
	      "  NPTXR_PTR            <0x%x>\n"
	      "  next_to_use          <%d>\n"
	      "  next_to_clean        <%d>\n"
	      "buffer_info[next_to_clean]:\n"
	      "  time_stamp           <0x%lx>\n"
	      "  next_to_watch        <%d>\n"
	      "  jiffies              <0x%lx>\n"
	      "  txdes0               <0x%x>\n"
	      "MAC Status             <0x%x>\n",
	      readl(tx_ring->head), tx_ring->next_to_use, tx_ring->next_to_clean,
	      tx_ring->buffer_info[lts].time_stamp, lts, jiffies, lts_desc->txdes0,
	      ior32(FTGMAC030_REG_MACCR));

	ftgmac030_dump(ctrl);
	while(1);
}

static void ftgmac030_put_txbuf(struct ftgmac030_ring *tx_ring,
				struct ftgmac030_buffer *buffer_info)
{
	struct ftgmac030_ctrl *ctrl = tx_ring->hw;

	if (buffer_info->dma)
		buffer_info->dma = 0;

	if (buffer_info->skb) {
		dev_kfree_skb_any(buffer_info->skb);
		buffer_info->skb = NULL;
	}
	buffer_info->time_stamp = 0;
}

/**
 * ftgmac030_clean_tx_irq - Reclaim resources after transmit completes
 * @tx_ring: Tx descriptor ring
 * @status: interrupt status for checkig ptp
 * the return value indicates whether actual cleaning was done, there
 * is no guarantee that everything was cleaned
 **/
static bool ftgmac030_clean_tx_irq(struct ftgmac030_ring *tx_ring, u32 status)
{
	struct ftgmac030_ctrl *ctrl = tx_ring->hw;
	struct eth_device *netdev = ctrl->netdev;
	struct ftgmac030_txdes *tx_desc;
	struct ftgmac030_buffer *buffer_info;
	unsigned int i, maccr;
	unsigned int count = 0;
	unsigned int total_tx_bytes = 0, total_tx_packets = 0;
	//unsigned int bytes_compl = 0, pkts_compl = 0;

	spin_lock(&tx_ring->ntu_lock);

	i = tx_ring->next_to_clean;

	while ((i != tx_ring->next_to_use) && (count < tx_ring->count)) {

		tx_desc = FTGMAC030_TX_DESC(*tx_ring, i);
		buffer_info = &tx_ring->buffer_info[i];

		/* Make hw descriptor updates visible to CPU */
		ithInvalidateDCacheRange(tx_desc, sizeof(struct ftgmac030_txdes));

		if (((le32_to_cpu(tx_desc->txdes0) & 0x3fff) == 0)||
		    !tx_desc->txdes1 || !tx_desc->txdes2 ||
		    (tx_desc->txdes0 & cpu_to_le32(FTGMAC030_TXDES0_TXDMA_OWN)))
			break;

		if (tx_desc->txdes0 & cpu_to_le32(FTGMAC030_TXDES0_LTS)) {

			total_tx_packets += buffer_info->segs;
			total_tx_bytes += buffer_info->bytecount;

			if (buffer_info->skb) {
				//bytes_compl += buffer_info->skb->len;
				//pkts_compl++;

				//ftgmac030_tx_hwtstamp(ctrl, buffer_info->skb,
				//		      status);
			}
		}

		ftgmac030_put_txbuf(tx_ring, buffer_info);
		/* clear all except end of ring bit */
		tx_desc->txdes0 &= cpu_to_le32(FTGMAC030_TXDES0_EDOTR);
		tx_desc->txdes1 = 0;
		tx_desc->txdes2 = 0;
		tx_desc->txdes3 = 0;

		count++;

		i++;
		if (i == tx_ring->count)
			i = 0;
	}

	tx_ring->next_to_clean = i;

	spin_unlock(&tx_ring->ntu_lock);

#define TX_WAKE_THRESHOLD 32
	if (count && netif_carrier_ok(netdev) &&
	    ftgmac030_desc_unused(tx_ring) >= TX_WAKE_THRESHOLD) {
		/* Make sure that anybody stopping the queue after this
		 * sees the new next_to_clean.
		 */
		//smp_mb();

		if (netif_queue_stopped(netdev) &&
		    !(test_bit(__FTGMAC030_DOWN, &ctrl->state))) {
			netif_wake_queue(netdev);
			++ctrl->restart_queue;
		}
	}

	/* Detect a transmit hang in hardware, this serializes the
	 * check with the clearing of time_stamp and movement of i
	 */
	maccr = ior32(FTGMAC030_REG_MACCR);
	if (tx_ring->buffer_info[i].time_stamp &&
	    time_after(jiffies, tx_ring->buffer_info[i].time_stamp
		       + (ctrl->tx_timeout_factor * HZ)) &&
	    (maccr & FTGMAC030_MACCR_TXMAC_EN))
		ftgmac030_print_hw_hang(ctrl);

	ctrl->total_tx_bytes += total_tx_bytes;
	ctrl->total_tx_packets += total_tx_packets;

	netdev->stats.tx_bytes += total_tx_bytes;
	netdev->stats.tx_packets += total_tx_packets;
	//printf("tx_cleaned: %d \n", count);
	return count < tx_ring->count;
}

static int ftgmac030_maybe_stop_tx(struct ftgmac030_ring *tx_ring, int size)
{
	struct ftgmac030_ctrl *ctrl;

	BUG_ON(size > tx_ring->count);

	if (ftgmac030_desc_unused(tx_ring) >= size)
		return 0;

	ctrl = tx_ring->hw;
	netif_stop_queue(ctrl->netdev);

wait:
	ithPrintf(" # \n");
	usleep(1000);

	/* We need to check again in case hw finish some tx packets and
	 * made room available.
	 */
	if (ftgmac030_desc_unused(tx_ring) < size)
		goto wait;

	/* A reprieve! */
	netif_start_queue(ctrl->netdev);
	++ctrl->restart_queue;

	return 0;
}

static int ftgmac030_tx_map(struct ftgmac030_ring *tx_ring, struct sk_buff *skb,
			    unsigned int max_per_txd, int first)
{
	struct ftgmac030_ctrl *ctrl = tx_ring->hw;
	struct ftgmac030_buffer *buffer_info;
	unsigned int len = skb_headlen(skb);
	unsigned int offset = 0, size, count = 0, i;
	unsigned int f, bytecount, segs;
	struct ftgmac030_txdes *first_desc = NULL, *tx_desc = NULL;
	u32 txdes1 = FTGMAC030_TXDES1_TXIC;
	u16 protocol;

	if (skb->ip_summed == CHECKSUM_PARTIAL) {
		#if 1
		skb->protocol = eth_type_trans(skb, ctrl->netdev);
		protocol = be16_to_cpu(skb->protocol);
		printf(" protocol = 0x%04X \n", protocol);
		#else
		if (skb->protocol == cpu_to_be16(ETH_P_8021Q))
			protocol = vlan_eth_hdr(skb)->h_vlan_encapsulated_proto;
		else
			protocol = skb->protocol;
		#endif

		switch (protocol) {
		case ETH_P_IP:
			txdes1 |= FTGMAC030_TXDES1_IP_CHKSUM;
			if (ip_hdr(skb)->protocol == IPPROTO_TCP)
				txdes1 |= FTGMAC030_TXDES1_TCP_CHKSUM;
			else if  (ip_hdr(skb)->protocol == IPPROTO_UDP)
				txdes1 |= FTGMAC030_TXDES1_UDP_CHKSUM;
			break;
		case ETH_P_IPV6:
			e_warn(tx_queued, "ETH_P_IPV6 proto=%x!\n", (protocol));
			#if 0
			e_warn(tx_queued, "ETH_P_IPV6 proto=%x!\n", be16_to_cpu(protocol));
			txdes1 |= FTGMAC030_TXDES1_IPV6_PKT;
			if (ipv6_hdr(skb)->nexthdr == IPPROTO_TCP)
				txdes1 |= FTGMAC030_TXDES1_TCP_CHKSUM;
			else if (ipv6_hdr(skb)->nexthdr == IPPROTO_UDP)
				txdes1 |= FTGMAC030_TXDES1_UDP_CHKSUM;
			#endif
			break;
		default:
    		//e_warn(tx_queued, "checksum_partial proto=%x!\n", be16_to_cpu(protocol));
    		e_warn(tx_queued, "checksum_partial proto=%x!\n", (protocol));
			break;
		}
	}

	#if 0
	if (vlan_tx_tag_present(skb)) {
		txdes1 |= FTGMAC030_TXDES1_INS_VLANTAG;
		txdes1 |= FTGMAC030_TXDES1_VLANTAG_CI(vlan_tx_tag_get(skb));
	}
	#endif

	#if defined(CFG_CPU_WB)
	ithFlushDCacheRange((void*)skb->data, len);
	#endif
	
	i = first;

	while (len) {
		buffer_info = &tx_ring->buffer_info[i];
		size = min(len, max_per_txd);

		buffer_info->length = size;
		buffer_info->time_stamp = jiffies;
		buffer_info->next_to_watch = i;
		buffer_info->dma = (unsigned int)skb->data + offset;
		buffer_info->mapped_as_page = false;

		if (i != first) {
			tx_desc = FTGMAC030_TX_DESC(*tx_ring, i);
			tx_desc->txdes0 |= cpu_to_le32(FTGMAC030_TXDES0_TXDMA_OWN|
						       FTGMAC030_TXDES0_TXBUF_SIZE
						       (buffer_info->length));
			tx_desc->txdes1 = cpu_to_le32(txdes1);
			tx_desc->txdes2 = cpu_to_le32(skb);
			tx_desc->txdes3 = cpu_to_le32(buffer_info->dma);
		}

		len -= size;
		offset += size;
		count++;

		if (len) {
			i++;
			if (i == tx_ring->count)
				i = 0;
		}
	}

	segs = 1;
	/* multiply data chunks by size of headers */
	bytecount = skb->len;

    /* i is the lastest desc */
	tx_ring->buffer_info[i].skb = skb;
	tx_ring->buffer_info[i].segs = segs;
	tx_ring->buffer_info[i].bytecount = bytecount;
	tx_ring->buffer_info[first].next_to_watch = i; 


	first_desc = FTGMAC030_TX_DESC(*tx_ring, first);
	buffer_info = &tx_ring->buffer_info[first];

	first_desc->txdes1 = cpu_to_le32(txdes1);
	first_desc->txdes2 = cpu_to_le32(skb);
	first_desc->txdes3 = cpu_to_le32(buffer_info->dma);

	if (tx_desc) { /* more than one desc, and tx_desc is the latedst one */
		tx_desc->txdes0 |= cpu_to_le32(FTGMAC030_TXDES0_LTS);

		first_desc->txdes0 |= cpu_to_le32(FTGMAC030_TXDES0_TXDMA_OWN|
						  FTGMAC030_TXDES0_FTS|
						  FTGMAC030_TXDES0_TXBUF_SIZE
						  (buffer_info->length));
	} else { /* only one desc */
		first_desc->txdes0 |= cpu_to_le32(FTGMAC030_TXDES0_TXDMA_OWN|
						  FTGMAC030_TXDES0_FTS|
						  FTGMAC030_TXDES0_LTS|
						  FTGMAC030_TXDES0_TXBUF_SIZE
						  (buffer_info->length));
	}

	/* Force memory writes to complete before letting h/w
	 * know there are new descriptors to fetch.
	 */
	ithFlushMemBuffer();
	
	//printf("tx_ring->tail = %X \n", tx_ring->tail); // test
	//ftgmac030_dump(ctrl);
	
	//printf("Fire tx\n");
	//writel(1, ctrl->io_base + FTGMAC030_REG_RXPD);  // test fire rx by mmio

	/* Any value written to NPTXPD is accpeted */
	writel(1, tx_ring->tail);
	//printf("Fire tx 1\n");
	//usleep(50);
	//ftgmac030_dump(ctrl);
	//while (1);
	return count;
}

/**
* ftgmac030_alloc_rx_buffers - Replace used receive buffers
* @rx_ring: Rx descriptor ring
**/
static void ftgmac030_alloc_rx_buffers(struct ftgmac030_ring *rx_ring,
    int cleaned_count, gfp_t gfp)
{
    struct ftgmac030_ctrl *ctrl = rx_ring->hw;
    struct eth_device *netdev = ctrl->netdev;
    struct ftgmac030_rxdes *rx_desc;
    struct ftgmac030_buffer *buffer_info;
    struct sk_buff *skb;
    unsigned int i;
    unsigned int bufsz = ALIGN(ctrl->rx_buffer_len, 8);

    i = rx_ring->next_to_use;
    buffer_info = &rx_ring->buffer_info[i];

    while (cleaned_count--) {
        skb = buffer_info->skb;
        if (skb) {
            skb_trim(skb, 0);
            goto map_skb;
        }

        /* FTGMAC030 can not support 2-bytes alignment DMA  */
        skb = __netdev_alloc_skb(netdev, bufsz, gfp);
        if (!skb) {
            /* Better luck next round */
            ctrl->alloc_rx_buff_failed++;
            break;
        }

        /* HW requires at least 8-bytes alignement */
        skb_align(skb, 8);
        buffer_info->skb = skb;
    map_skb:
        buffer_info->dma = (unsigned int)skb->data;

        rx_desc = FTGMAC030_RX_DESC(*rx_ring, i);
        /* Bit 31 RXPKT_RDY is 0, pass owner to hw. */
        rx_desc->rxdes0 = 0;
        rx_desc->rxdes1 = 0;
        /* rxdes2 is not used by hardware. We use it to keep track of skb */
        rx_desc->rxdes2 = (u32)skb->data;
        rx_desc->rxdes3 = cpu_to_le32(buffer_info->dma);

        i++;
        if (i == rx_ring->count) {
            i = 0;
            /* Set bit 15 EDORR to indicate last descriptor. */
            rx_desc->rxdes0 |= cpu_to_le32(FTGMAC030_RXDES0_EDORR);
        }
        buffer_info = &rx_ring->buffer_info[i];
    }

    rx_ring->next_to_use = i;
}
#if 0
/**
 * ftgmac030_alloc_jumbo_rx_buffers - Replace used jumbo receive buffers
 * @rx_ring: Rx descriptor ring
 * @cleaned_count: number of buffers to allocate this pass
 **/
static void ftgmac030_alloc_jumbo_rx_buffers(struct ftgmac030_ring *rx_ring,
					     int cleaned_count, gfp_t gfp)
{
	struct ftgmac030_ctrl *ctrl = rx_ring->hw;
	struct eth_device *netdev = ctrl->netdev;
	struct ftgmac030_rxdes *rx_desc;
	struct ftgmac030_buffer *buffer_info;
	struct sk_buff *skb;
	unsigned int i;
	unsigned int bufsz = 256 - 16;	/* for skb_reserve */

	i = rx_ring->next_to_use;
	buffer_info = &rx_ring->buffer_info[i];

	while (cleaned_count--) {
		skb = buffer_info->skb;
		if (skb) {
			skb_trim(skb, 0);
			goto check_page;
		}

		skb = __netdev_alloc_skb(netdev, bufsz, gfp);
		if (unlikely(!skb)) {
			/* Better luck next round */
			ctrl->alloc_rx_buff_failed++;
			break;
		}

		buffer_info->skb = skb;
check_page:
	    buffer_info->dma = (unsigned int)skb->data;

		rx_desc = FTGMAC030_RX_DESC(*rx_ring, i);
		/* Bit 31 RXPKT_RDY is 0, pass owner to hw. */
		rx_desc->rxdes0 = 0;
		rx_desc->rxdes1 = 0;
		/* rxdes2 is not used by hardware. We use it to keep track of skb */
		rx_desc->rxdes2 = (u32) skb->data;
		rx_desc->rxdes3 = cpu_to_le32(buffer_info->dma);


		if (unlikely(++i == rx_ring->count)) {
			i = 0;
			/* Set bit 15 EDORR to indicate last descriptor. */
			rx_desc->rxdes0 |= cpu_to_le32(FTGMAC030_RXDES0_EDORR);
		}

		buffer_info = &rx_ring->buffer_info[i];
	}

	if (likely(rx_ring->next_to_use != i)) {
		rx_ring->next_to_use = i;

		if (unlikely(i-- == 0)) {
			i = (rx_ring->count - 1);

			rx_desc = FTGMAC030_RX_DESC(*rx_ring, i);
			/* Set bit 15 EDORR to indicate last descriptor. */
			rx_desc->rxdes0 |= cpu_to_le32(FTGMAC030_RXDES0_EDORR);
		}
	}
}

/**
 * ftgmac030_clean_jumbo_rx_irq - Send received data up the network stack; legacy
 * @ctrl: board private structure
 *
 * the return value indicates whether actual cleaning was done, there
 * is no guarantee that everything was cleaned
 **/
static bool ftgmac030_clean_jumbo_rx_irq(struct ftgmac030_ring *rx_ring,
					 int *work_done, int work_to_do)
{
	struct ftgmac030_ctrl *ctrl = rx_ring->hw;
	struct eth_device *netdev = ctrl->netdev;
	struct ftgmac030_rxdes *rx_desc, *next_rxd;
	struct ftgmac030_buffer *buffer_info, *next_buffer;
	u32 length, staterr;
	unsigned int i;
	int cleaned_count = 0;
	bool cleaned = false;
	unsigned int total_rx_bytes = 0, total_rx_packets = 0;
	struct skb_shared_info *shinfo;

	i = rx_ring->next_to_clean;
	rx_desc = FTGMAC030_RX_DESC(*rx_ring, i);
	ithInvalidateDCacheRange(rx_desc, sizeof(struct ftgmac030_rxdes));
	staterr = le32_to_cpu(rx_desc->rxdes0);
	buffer_info = &rx_ring->buffer_info[i];

	while (staterr & FTGMAC030_RXDES0_RXPKT_RDY) {
		struct sk_buff *skb;

		if (*work_done >= work_to_do)
			break;
		(*work_done)++;

		skb = buffer_info->skb;
		//buffer_info->skb = NULL;    /* we willn't re-allocte rx buffer */

		++i;
		if (i == rx_ring->count)
			i = 0;
		next_rxd = FTGMAC030_RX_DESC(*rx_ring, i);
		ithInvalidateDCacheRange(next_rxd, sizeof(struct ftgmac030_rxdes));

		next_buffer = &rx_ring->buffer_info[i];

		cleaned = true;
		cleaned_count++;
		dma_unmap_page(&pdev->dev, buffer_info->dma, PAGE_SIZE,
			       DMA_FROM_DEVICE);
		buffer_info->dma = 0;

		length = staterr & FTGMAC030_RXDES0_VDBC;

#if defined(RECEIVE_BAD_RX_TO_UPLAYER)
		/* errors is only valid for FRS descriptors */
		if (unlikely((staterr & FTGMAC030_RXDES0_FRS) &&
			     (ftgmac030_rx_packet_error(ctrl, rx_desc) &&
			      !(netdev->features & NETIF_F_RXALL)))) {
			/* recycle both page and skb */
			buffer_info->skb = skb;
			/* an error means any chain goes out the window too */
			if (rx_ring->rx_skb_top)
				dev_kfree_skb_irq(rx_ring->rx_skb_top);
			rx_ring->rx_skb_top = NULL;
			goto next_desc;
		}
#else
		/* errors is only valid for FRS descriptors */
		if (unlikely((staterr & FTGMAC030_RXDES0_FRS) &&
				 (ftgmac030_rx_packet_error(ctrl, rx_desc)))) {
			/* recycle both page and skb */
			buffer_info->skb = skb;
			/* an error means any chain goes out the window too */
			if (rx_ring->rx_skb_top)
				dev_kfree_skb_irq(rx_ring->rx_skb_top);
			rx_ring->rx_skb_top = NULL;
			goto next_desc;
		}
#endif

		/* Receive Checksum Offload only valid for FRS descriptor */
		if (staterr & FTGMAC030_RXDES0_FRS)
			ftgmac030_rx_checksum(ctrl, le32_to_cpu(rx_desc->rxdes1),
					      skb);
#define rxtop (rx_ring->rx_skb_top)
		if (!(staterr & FTGMAC030_RXDES0_LRS)) {
			/* this descriptor is only the beginning (or middle) */
			if (!rxtop) {
				/* FRS must be one for beginning of a chain */
				BUG_ON(!(staterr & FTGMAC030_RXDES0_FRS));

				/* this is the beginning of a chain */
				rxtop = skb;
				skb_fill_page_desc(rxtop, 0, buffer_info->page,
						   0, length);
			} else {
				/* this is the middle of a chain */
				shinfo = skb_shinfo(rxtop);
				skb_fill_page_desc(rxtop, shinfo->nr_frags,
						   buffer_info->page, 0,
						   length);
				/* re-use the skb, only consumed the page */
				buffer_info->skb = skb;
			}
			ftgmac030_consume_page(buffer_info, rxtop, length);
			goto next_desc;
		} else {
			if (rxtop) {
				/* end of the chain */
				shinfo = skb_shinfo(rxtop);
				skb_fill_page_desc(rxtop, shinfo->nr_frags,
						   buffer_info->page, 0,
						   length);
				/* re-use the current skb, we only consumed the
				 * page
				 */
				buffer_info->skb = skb;
				skb = rxtop;
				rxtop = NULL;
				ftgmac030_consume_page(buffer_info, skb, length);
			} else {
				/* no chain,  FRS and LRS is one, this buf is the
				 * packet copybreak to save the put_page/alloc_page
				 */
				if (length <= copybreak &&
				    skb_tailroom(skb) >= length) {
					u8 *vaddr;
					vaddr = kmap_atomic(buffer_info->page);
					memcpy(skb_tail_pointer(skb), vaddr,
					       length);
					kunmap_atomic(vaddr);
					/* re-use the page, so don't erase
					 * buffer_info->page
					 */
					skb_put(skb, length);
				} else {
					skb_fill_page_desc(skb, 0,
							   buffer_info->page, 0,
							   length);
					ftgmac030_consume_page(buffer_info, skb,
							   length);
				}
			}
		}

		/* probably a little skewed due to removing CRC */
		total_rx_bytes += skb->len;
		total_rx_packets++;

		/* eth type trans needs skb->data to point to something */
		if (!pskb_may_pull(skb, ETH_HLEN)) {
			e_err(tx_err, "pskb_may_pull failed.\n");
			dev_kfree_skb_irq(skb);
			goto next_desc;
		}

		ftgmac030_receive_skb(ctrl, netdev, skb,
				      le32_to_cpu(rx_desc->rxdes1));
next_desc:
		/* return some buffers to hardware, one at a time is too slow */
		if (unlikely(cleaned_count >= FTGMAC030_RX_BUFFER_WRITE)) {
			ctrl->alloc_rx_buf(rx_ring, cleaned_count, GFP_ATOMIC);
			cleaned_count = 0;
		}

		/* use prefetched values */
		rx_desc = next_rxd;
		buffer_info = next_buffer;

		staterr = le32_to_cpu(rx_desc->rxdes0);
	}
	rx_ring->next_to_clean = i;

	if (cleaned_count)
		ctrl->alloc_rx_buf(rx_ring, cleaned_count, GFP_ATOMIC);

	netdev->stats.rx_bytes += total_rx_bytes;
	netdev->stats.rx_packets += total_rx_packets;

	ctrl->total_rx_bytes += total_rx_bytes;
	ctrl->total_rx_packets += total_rx_packets;
	return cleaned;
}
#endif

/**
 * ftgmac030_update_itr - update the dynamic ITR value based on statistics
 * @itr_setting: current ctrl->itr
 * @packets: the number of packets during this measurement interval
 * @bytes: the number of bytes during this measurement interval
 *
 *      Stores a new ITR value based on packets and byte
 *      counts during the last interrupt.  The advantage of per interrupt
 *      computation is faster updates and more accurate ITR for the current
 *      traffic pattern.  Constants in this function were computed
 *      based on theoretical maximum wire speed and thresholds were set based
 *      on testing data as well as attempting to minimize response time
 *      while increasing bulk throughput.  This functionality is controlled
 *      by the InterruptThrottleRate module parameter.
 **/
static unsigned int ftgmac030_update_itr(u16 itr_setting, int packets, int bytes)
{
	unsigned int retval = itr_setting;

	if (packets == 0)
		return itr_setting;

	switch (itr_setting) {
	case lowest_latency:
		/* handle jumbo frames */
		if (bytes / packets > 8000)
			retval = bulk_latency;
		else if ((packets < 5) && (bytes > 512))
			retval = low_latency;
		break;
	case low_latency:	/* 50 usec aka 20000 ints/s */
		if (bytes > 10000) {
			if (bytes / packets > 8000)
				retval = bulk_latency;
			else if ((packets < 10) || ((bytes / packets) > 1200))
				retval = bulk_latency;
			else if ((packets > 35))
				retval = lowest_latency;
		} else if (bytes / packets > 2000) {
			retval = bulk_latency;
		} else if (packets <= 2 && bytes < 512) {
			retval = lowest_latency;
		}
		break;
	case bulk_latency:	/* 250 usec aka 4000 ints/s */
		if (bytes > 25000) {
			if (packets > 35)
				retval = low_latency;
		} else if (bytes < 6000) {
			retval = low_latency;
		}
		break;
	}

	return retval;
}

static void ftgmac030_set_itr(struct ftgmac030_ctrl *ctrl)
{
	u16 current_itr;
	u32 new_itr = ctrl->itr;

	/* for non-gigabit speeds, just fix the interrupt rate at 4000 */
	if (ctrl->phy_speed != SPEED_1000) {
		current_itr = 0;
		new_itr = 4000;
		goto set_itr_now;
	}

	if (ctrl->flags & FLAG_DISABLE_AIM) {
		new_itr = 0;
		goto set_itr_now;
	}

	ctrl->tx_itr = ftgmac030_update_itr(ctrl->tx_itr,
					  ctrl->total_tx_packets,
					  ctrl->total_tx_bytes);
	/* conservative mode (itr 3) eliminates the lowest_latency setting */
	if (ctrl->itr_setting == 3 && ctrl->tx_itr == lowest_latency)
		ctrl->tx_itr = low_latency;

	ctrl->rx_itr = ftgmac030_update_itr(ctrl->rx_itr,
					  ctrl->total_rx_packets,
					  ctrl->total_rx_bytes);
	/* conservative mode (itr 3) eliminates the lowest_latency setting */
	if (ctrl->itr_setting == 3 && ctrl->rx_itr == lowest_latency)
		ctrl->rx_itr = low_latency;

	current_itr = max(ctrl->rx_itr, ctrl->tx_itr);

	/* counts and packets in update_itr are dependent on these numbers */
	switch (current_itr) {
	case lowest_latency:
		new_itr = 70000;
		break;
	case low_latency:	/* 50 usec aka 20000 ints/s */
		new_itr = 20000;
		break;
	case bulk_latency:	/* 250 usec aka 4000 ints/s */
		new_itr = 4000;
		break;
	default:
		break;
	}

set_itr_now:
   // printf(" ctrl->itr = %d, new_itr = %d \n", ctrl->itr, new_itr); // test
	if (new_itr != ctrl->itr) {
		/* this attempts to bias the interrupt rate towards Bulk
		 * by adding intermediate steps when interrupt rate is
		 * increasing
		 */
		new_itr = new_itr > ctrl->itr ?
		    min(ctrl->itr + (new_itr >> 2), new_itr) : new_itr;
		ctrl->itr = new_itr;
		ctrl->rx_ring->itr_val = new_itr;
		ftgmac030_write_itr(ctrl, new_itr);
	}
}

/**
* ftgmac030_write_itr - write the ITR value to the appropriate registers
* @ctrl: address of board private structure
* @itr: new ITR value to program
*
* ftgmac030_write_itr writes the ITR value into the RX Interrupt Timer
* Control Register(RXITC)
**/
void ftgmac030_write_itr(struct ftgmac030_ctrl *ctrl, u32 itr)
{
    u32 rxitc;

    /*
    * itr is expected number of interrupt happens per second.
    *
    * FTGMAC030 requires to set RX cycle time.
    *
    * When RXITC.RXINT_TIME_SEL set, the RX cycle times are:
    * 1000 Mbps mode => 16.384 繕s
    * 100 Mbps mode => 81.92 繕s
    * 10 Mbps mode => 819.2 繕s
    *
    * See FTGMAC030 datasheet register offset 0x34.
    */
    if (itr) {
        u32 rx_cycle, new_itr, rxitc;

        /* convert it to nanoseconds to avoid decimal point calculation */
        switch (ctrl->phy_speed) {
        default:
        case SPEED_10:
            rx_cycle = 819200;
            break;

        case SPEED_100:
            rx_cycle = 81920;
            break;

        case SPEED_1000:
            rx_cycle = 16384;
            break;
        }

        new_itr = 1000000000 / (itr * rx_cycle);

        rxitc = FTGMAC030_ITC_TIME_SEL;
        rxitc |= FTGMAC030_ITC_CYCL_CNT(new_itr);
		
		e_dbg(drv,"ftgmac030_write_itr(): itr=%d, new_itr=%d\n",
			   itr, new_itr);
    }
    else {
        rxitc = 0;
		
		e_dbg(drv,"ftgmac030_write_itr(): itr=0\n");
    }

    iow32(FTGMAC030_REG_RXITC, rxitc);
}

/**
 * ftgmac030_receive_skb - helper function to handle Rx indications
 * @ctrl: board private structure
 * @rxdes1: descriptor contains status field as written by hardware
 * @skb: pointer to sk_buff to be indicated to stack
 **/
static void ftgmac030_receive_skb(struct ftgmac030_ctrl *ctrl,
			      struct eth_device *netdev, struct sk_buff *skb,
			      u32 rxdes1)
{
#if 0
	u16 tag = rxdes1 & FTGMAC030_RXDES1_VLANTAG_CI;
	u8 ptp_type = FTGMAC030_RXDES1_PTP_TYPE(rxdes1);

	ftgmac030_rx_hwtstamp(ctrl, ptp_type, skb);

	skb->protocol = eth_type_trans(skb, netdev);

	if (rxdes1 & FTGMAC030_RXDES1_VLANTAG_AVAIL)
		__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), tag);
#endif

	/* push packet to protocol stack */
	ctrl->netif_rx(ctrl->netif_arg, (void*)(skb->data), skb->len);

}

/**
 * ftgmac030_rx_checksum - Receive Checksum Offload
 * @ctrl: board private structure
 * @status_err: receive descriptor status and error fields
 * @csum: receive descriptor csum field
 * @sk_buff: socket buffer with received data
 **/
static void ftgmac030_rx_checksum(struct ftgmac030_ctrl *ctrl, u32 status_err,
				  struct sk_buff *skb)
{
	u32 proto = (status_err & FTGMAC030_RXDES1_PROT_MASK);

	//skb_checksum_none_assert(skb);

	/* Rx checksum disabled */
	if (!(ctrl->netdev->features & NETIF_F_RXCSUM))
		return;

	if (((proto == FTGMAC030_RXDES1_PROT_TCPIP) &&
	     !(status_err & FTGMAC030_RXDES1_TCP_CHKSUM_ERR)) ||
	     ((proto == FTGMAC030_RXDES1_PROT_UDPIP) &&
	     !(status_err & FTGMAC030_RXDES1_UDP_CHKSUM_ERR))) {
		/* TCP or UDP packet with a valid checksum */
		skb->ip_summed = CHECKSUM_UNNECESSARY;
		ctrl->hw_csum_good++;
	} else {
		/* let the stack verify checksum errors */
		ctrl->hw_csum_err++;
		//printf("proto = %X, status_err=%X \n", proto, status_err);
		//e_info(rx_err, "rx hw checksum err\n");
	}
}

static bool ftgmac030_rx_packet_error(struct ftgmac030_ctrl *ctrl,
				      struct ftgmac030_rxdes *rxdes)
{
	struct eth_device *netdev = ctrl->netdev;
	bool error = false;

	if (unlikely(rxdes->rxdes0 &
		     cpu_to_le32(FTGMAC030_RXDES0_RX_ERR))) {
		e_info(rx_err, "rx err\n");

		netdev->stats.rx_errors++;
		error = true;
	}

	if (unlikely(rxdes->rxdes0 &
		     cpu_to_le32(FTGMAC030_RXDES0_CRC_ERR))) {
		e_info(rx_err, "rx crc err\n");

		netdev->stats.rx_crc_errors++;
		error = true;
	} else if (unlikely(rxdes->rxdes1 &
		   cpu_to_le32(FTGMAC030_RXDES1_IP_CHKSUM_ERR))) {
		e_info(rx_err, "rx IP checksum err\n");

		error = true;
	}

	if (unlikely(rxdes->rxdes0 & cpu_to_le32(FTGMAC030_RXDES0_FTL))) {
		e_info(rx_err, "rx frame too long\n");

		netdev->stats.rx_length_errors++;
		error = true;
	} else if (unlikely(rxdes->rxdes0 &
		   cpu_to_le32(FTGMAC030_RXDES0_RUNT))) {
		e_info(rx_err, "rx runt\n");

		netdev->stats.rx_length_errors++;
		error = true;
	} else if (unlikely(rxdes->rxdes0 &
		   cpu_to_le32(FTGMAC030_RXDES0_RX_ODD_NB))) {
		e_info(rx_err, "rx odd nibble\n");

		netdev->stats.rx_length_errors++;
		error = true;
	}

	return error;
}

/**
 * ftgmac030_clean_rx_irq - Send received data up the network stack
 * @rx_ring: Rx descriptor ring
 *
 * the return value indicates whether actual cleaning was done, there
 * is no guarantee that everything was cleaned
 **/
static bool ftgmac030_clean_rx_irq(struct ftgmac030_ring *rx_ring, int *work_done,
					   int work_to_do)
{
	struct ftgmac030_ctrl *ctrl = rx_ring->hw;
	struct eth_device *netdev = ctrl->netdev;
	struct ftgmac030_buffer *buffer_info, *next_buffer;
	struct ftgmac030_rxdes *rx_desc, *next_rxd;
	u32 length, staterr;
	unsigned int i;
	int cleaned_count = 0;
	bool cleaned = false;
	unsigned int total_rx_bytes = 0, total_rx_packets = 0;

	i = rx_ring->next_to_clean;
	rx_desc = FTGMAC030_RX_DESC(*rx_ring, i);
	ithInvalidateDCacheRange(rx_desc, sizeof(struct ftgmac030_rxdes));
	staterr = le32_to_cpu(rx_desc->rxdes0);
	buffer_info = &rx_ring->buffer_info[i];

	while (staterr & FTGMAC030_RXDES0_RXPKT_RDY) {
		struct sk_buff *skb;

		if (*work_done >= work_to_do)
			break;
		(*work_done)++;

		/* read descriptor and rx_buffer_info after
		 * status RXDES0_RXPKT_RDY
		 */

		skb = buffer_info->skb;
		//buffer_info->skb = NULL;   /* we willn't re-allocte rx buffer */

		i++;
		if (i == rx_ring->count)
			i = 0;
		next_rxd = FTGMAC030_RX_DESC(*rx_ring, i);
		ithInvalidateDCacheRange(next_rxd, sizeof(struct ftgmac030_rxdes));
		next_buffer = &rx_ring->buffer_info[i];

		cleaned = true;
		cleaned_count++;
		buffer_info->dma = 0;

		length = staterr & FTGMAC030_RXDES0_VDBC;

		/* !FRS and !LRS means multiple descriptors were used to
		 * store a single packet, if that's the case we need to
		 * toss it. Go to next frame until we find FRS and LRS
		 * bit set, as it is by definition one packet in one
		 * descriptor entry.
		 */
		if (unlikely((staterr & FTGMAC030_RXDES0_EOP)
			     != FTGMAC030_RXDES0_EOP)) {
			/* All receives must fit into a single buffer */
			e_dbg(rx_err, "Receive packet consumed multiple buffers\n");
			/* recycle */
			buffer_info->skb = skb;
			goto next_desc;
		}

#if defined(RECEIVE_BAD_RX_TO_UPLAYER)
		if (unlikely(ftgmac030_rx_packet_error(ctrl, rx_desc) &&
			     !(netdev->features & NETIF_F_RXALL))) {
				/* recycle */
				buffer_info->skb = skb;
				goto next_desc;
			}
#else
		if (unlikely(ftgmac030_rx_packet_error(ctrl, rx_desc)) {
			/* recycle */
			buffer_info->skb = skb;
			goto next_desc;
		}
#endif
		/* FTGMAC030 always include 4 bytes CRC in length.
		 * Adjust length to remove Ethernet CRC.
		 *
		 * If configured to store CRC, don't subtract FCS,
		 * but keep the FCS bytes out of the total_rx_bytes
		 * counter
		 */
		if (netdev->features & NETIF_F_RXFCS)
			total_rx_bytes -= 4;
		else
			length -= 4;

		//ithInvalidateDCacheRange(skb->data, length);
		ithInvalidateDCacheRange(skb->data, length+4);

		total_rx_bytes += length;
		total_rx_packets++;
		skb->len = length;

		/* Receive Checksum Offload */
		ftgmac030_rx_checksum(ctrl, le32_to_cpu(rx_desc->rxdes1), skb);

		ftgmac030_receive_skb(ctrl, netdev, skb,
				      le32_to_cpu(rx_desc->rxdes1));
next_desc:
		/* return some buffers to hardware, one at a time is too slow */
		if (cleaned_count >= FTGMAC030_RX_BUFFER_WRITE) {
			ctrl->alloc_rx_buf(rx_ring, cleaned_count, GFP_ATOMIC);
			cleaned_count = 0;
		}

		/* use prefetched values */
		rx_desc = next_rxd;
		buffer_info = next_buffer;

		staterr = le32_to_cpu(rx_desc->rxdes0);
	}
	rx_ring->next_to_clean = i;

	if (cleaned_count)
		ctrl->alloc_rx_buf(rx_ring, cleaned_count, GFP_ATOMIC);

	netdev->stats.rx_bytes += total_rx_bytes;
	netdev->stats.rx_packets += total_rx_packets;

	ctrl->total_rx_bytes += total_rx_bytes;
	ctrl->total_rx_packets += total_rx_packets;
	return cleaned;
}

/**
* ftgmac030_irq_disable - Mask off interrupt generation on the NIC
**/
static void ftgmac030_irq_disable(struct ftgmac030_ctrl *ctrl)
{
    iow32(FTGMAC030_REG_IER, 0);
}

/**
* ftgmac030_irq_enable - Enable default interrupt generation settings
**/
static void ftgmac030_irq_enable(struct ftgmac030_ctrl *ctrl)
{
    iow32(FTGMAC030_REG_IER, INT_MASK_ALL_ENABLED);
}

/**
 * ftgmac030_clean_rx_ring - Free Rx Buffers per Queue
 * @rx_ring: Rx descriptor ring
 **/
static void ftgmac030_clean_rx_ring(struct ftgmac030_ring *rx_ring)
{
	struct ftgmac030_ctrl *ctrl = rx_ring->hw;
	struct ftgmac030_buffer *buffer_info;
	unsigned int i;

	/* Free all the Rx ring sk_buffs */
	for (i = 0; i < rx_ring->count; i++) {
		buffer_info = &rx_ring->buffer_info[i];
		if (buffer_info->dma)
			buffer_info->dma = 0;

		if (buffer_info->skb) {
			dev_kfree_skb(buffer_info->skb);
			buffer_info->skb = NULL;
		}
	}

	/* there also may be some cached data from a chained receive */
	if (rx_ring->rx_skb_top) {
		dev_kfree_skb(rx_ring->rx_skb_top);
		rx_ring->rx_skb_top = NULL;
	}

	/* Zero out the descriptor ring */
	memset(rx_ring->desc, 0, rx_ring->size);

	rx_ring->next_to_clean = 0;
	rx_ring->next_to_use = 0;
}

/**
* ftgmac030_alloc_ring_dma - allocate memory for a ring structure
**/
static int ftgmac030_alloc_ring_dma(struct ftgmac030_ctrl *ctrl,
struct ftgmac030_ring *ring)
{
    ring->dma = itpWTAlloc(ring->size);  /* already 64 bytes align */
	if (!ring->dma)
        return -ENOMEM;

    ring->desc = (void*)ring->dma;
	memset(ring->desc, 0, ring->size);


    return 0;
}

/**
* ftgmac030_setup_tx_resources - allocate Tx resources (Descriptors)
* @tx_ring: Tx descriptor ring
*
* Return 0 on success, negative on failure
**/
int ftgmac030_setup_tx_resources(struct ftgmac030_ring *tx_ring)
{
    struct ftgmac030_ctrl *ctrl = tx_ring->hw;
    int err = -ENOMEM, size;

    size = sizeof(struct ftgmac030_buffer) * tx_ring->count;
    tx_ring->buffer_info = vzalloc(size);
    if (!tx_ring->buffer_info)
        goto err;

    /* round up to nearest 4K */
    tx_ring->size = tx_ring->count * sizeof(struct ftgmac030_txdes);
    //tx_ring->size = ALIGN(tx_ring->size, 4096);

    err = ftgmac030_alloc_ring_dma(ctrl, tx_ring);
    if (err)
        goto err;

    tx_ring->next_to_use = 0;
    tx_ring->next_to_clean = 0;

    spin_lock_init(&tx_ring->ntu_lock);

    return 0;
err:
    vfree(tx_ring->buffer_info);
    e_err(drv, "Unable to allocate memory for the transmit descriptor ring\n");
    return err;
}

/**
* ftgmac030_setup_rx_resources - allocate Rx resources (Descriptors)
* @rx_ring: Rx descriptor ring
*
* Returns 0 on success, negative on failure
**/
int ftgmac030_setup_rx_resources(struct ftgmac030_ring *rx_ring)
{
    struct ftgmac030_ctrl *ctrl = rx_ring->hw;
    int size, err = -ENOMEM;

    size = sizeof(struct ftgmac030_buffer) * rx_ring->count;
    rx_ring->buffer_info = vzalloc(size);
    if (!rx_ring->buffer_info)
        goto err;

    /* Round up to nearest 4K */
    rx_ring->size = rx_ring->count * sizeof(struct ftgmac030_rxdes);
    //rx_ring->size = ALIGN(rx_ring->size, 4096);

    err = ftgmac030_alloc_ring_dma(ctrl, rx_ring);
    if (err)
        goto err;

    rx_ring->next_to_clean = 0;
    rx_ring->next_to_use = 0;
    rx_ring->rx_skb_top = NULL;

    return 0;

err:
    vfree(rx_ring->buffer_info);
    e_err(drv, "Unable to allocate memory for the receive descriptor ring\n");
    return err;
}

/**
* ftgmac030_clean_tx_ring - Free Tx Buffers
* @tx_ring: Tx descriptor ring
**/
static void ftgmac030_clean_tx_ring(struct ftgmac030_ring *tx_ring)
{
    struct ftgmac030_ctrl *ctrl = tx_ring->hw;
    struct ftgmac030_buffer *buffer_info;
    unsigned long size;
    unsigned int i;

    for (i = 0; i < tx_ring->count; i++) {
        buffer_info = &tx_ring->buffer_info[i];
        ftgmac030_put_txbuf(tx_ring, buffer_info);
    }

    size = sizeof(struct ftgmac030_buffer) * tx_ring->count;
    memset(tx_ring->buffer_info, 0, size);

    memset(tx_ring->desc, 0, tx_ring->size);

    tx_ring->next_to_use = 0;
    tx_ring->next_to_clean = 0;
}

/**
* ftgmac030_free_tx_resources - Free Tx Resources per Queue
* @tx_ring: Tx descriptor ring
*
* Free all transmit software resources
**/
void ftgmac030_free_tx_resources(struct ftgmac030_ring *tx_ring)
{
    struct ftgmac030_ctrl *ctrl = tx_ring->hw;

    ftgmac030_clean_tx_ring(tx_ring);

    vfree(tx_ring->buffer_info);
    tx_ring->buffer_info = NULL;

    itpWTFree((u32)tx_ring->desc);
    tx_ring->desc = NULL;
}

/**
* ftgmac030_free_rx_resources - Free Rx Resources
* @rx_ring: Rx descriptor ring
*
* Free all receive software resources
**/
void ftgmac030_free_rx_resources(struct ftgmac030_ring *rx_ring)
{
    struct ftgmac030_ctrl *ctrl = rx_ring->hw;

    ftgmac030_clean_rx_ring(rx_ring);

    vfree(rx_ring->buffer_info);
    rx_ring->buffer_info = NULL;

    itpWTFree((u32)rx_ring->desc);
    rx_ring->desc = NULL;
}

/**
* ftgmac030_alloc_queues - Allocate memory for all rings
* @ctrl: board private structure to initialize
**/
static int ftgmac030_alloc_queues(struct ftgmac030_ctrl *ctrl)
{
    int size = sizeof(struct ftgmac030_ring);

    ctrl->tx_ring = kzalloc(size, GFP_KERNEL);
    if (!ctrl->tx_ring)
        goto err;
    ctrl->tx_ring->count = ctrl->tx_ring_count;
    ctrl->tx_ring->hw = ctrl;

    ctrl->rx_ring = kzalloc(size, GFP_KERNEL);
    if (!ctrl->rx_ring)
        goto err;
    ctrl->rx_ring->count = ctrl->rx_ring_count;
    ctrl->rx_ring->hw = ctrl;

    return 0;
err:
    e_err(drv, "Unable to allocate memory for queues\n");
    kfree(ctrl->rx_ring);
    kfree(ctrl->tx_ring);
    return -ENOMEM;
}

/**
 * ftgmac030_configure_tx - Configure Transmit Unit after Reset
 * @ctrl: board private structure
 *
 * Configure the Tx unit of the MAC after a reset.
 **/
static void ftgmac030_configure_tx(struct ftgmac030_ctrl *ctrl)
{
	struct ftgmac030_ring *tx_ring = ctrl->tx_ring;
	u32 tdba, maccr;
	struct ftgmac030_txdes *tdben;

	/* Setup the HW Tx Head */
	tdba = tx_ring->dma;
	iow32(FTGMAC030_REG_NPTXR_BADR, tdba);

	/* Setup end of Tx descriptor */
	tdben = FTGMAC030_TX_DESC(*tx_ring, (tx_ring->count - 1));
	tdben->txdes0 = cpu_to_le32(FTGMAC030_TXDES0_EDOTR);

	tx_ring->head =  (ctrl->io_base +
					  FTGMAC030_REG_NPTXR_PTR);
	/* FTGMAC030 does not have register to read end of
	 * tx descriptor ring.
	 * tail is used to let h/w know there are new descriptors
	 * to fetch.
	 */
	tx_ring->tail = (ctrl->io_base +
					 FTGMAC030_REG_NPTXPD);

	e_dbg(drv,"tx desc dma %x, vaddr %x, nptxpd %08x\n",
		   tx_ring->dma, (u32)tx_ring->desc, readl(tx_ring->head));

	/* FTGMAC030 can not disable append CRC per packet
	 * See _xmit_frame
	 */
	maccr = ior32(FTGMAC030_REG_MACCR);
	maccr |= FTGMAC030_MACCR_CRC_APD;	
	iow32(FTGMAC030_REG_MACCR, maccr);

	/* Set the Tx Interrupt Delay register */
	iow32(FTGMAC030_REG_TXITC, FTGMAC030_ITC_CYCL_CNT(ctrl->tx_int_delay));
}

/**
 * ftgmac030_configure_rx - Configure Receive Unit after Reset
 * @ctrl: board private structure
 *
 * Configure the Rx unit of the MAC after a reset.
 **/
static void ftgmac030_configure_rx(struct ftgmac030_ctrl *ctrl)
{
	struct ftgmac030_ring *rx_ring = ctrl->rx_ring;
	struct ftgmac030_rxdes *rdben;

	/*if (ctrl->netdev->mtu > ETH_FRAME_LEN + ETH_FCS_LEN) {
		ctrl->clean_rx = ftgmac030_clean_jumbo_rx_irq;
		ctrl->alloc_rx_buf = ftgmac030_alloc_jumbo_rx_buffers;
	} else */{
		ctrl->clean_rx = ftgmac030_clean_rx_irq;
		ctrl->alloc_rx_buf = ftgmac030_alloc_rx_buffers;
	}

	/* set the Receive Delay Timer Register */
	if ((ctrl->itr_setting != 0) && (ctrl->itr != 0))
		ftgmac030_write_itr(ctrl, ctrl->itr);

	/* Setup the HW Rx Head and Tail Descriptor Pointers and
	 * the Base and Length of the Rx Descriptor Ring
	 */
	iow32(FTGMAC030_REG_RXR_BADR, rx_ring->dma);

	e_dbg(drv,"rx desc dma %x, vaddr %x\n",
		   rx_ring->dma, (u32)rx_ring->desc);

	rx_ring->head = (ctrl->io_base +
					 FTGMAC030_REG_RXR_BADR);
	/* FTGMAC030 does not have register to read end of
	 * rx descriptor ring
	 */
	rx_ring->tail = (ctrl->io_base +
					 FTGMAC030_REG_RXR_PTR);

	/* Setup end of Rx descriptor */
	rdben = FTGMAC030_RX_DESC(*rx_ring, (rx_ring->count - 1));
	rdben->rxdes0 = cpu_to_le32(FTGMAC030_RXDES0_EDORR);

	iow32(FTGMAC030_REG_APTC, FTGMAC030_APTC_RXPOLL_CNT(1));
}

/**
* ftgmac030_sw_init - Initialize general software structures
(struct ftgmac030_ctrl)
* @ctrl: board private structure to initialize
*
* ftgmac030_sw_init initializes the Adapter private data structure.
* Fields are initialized based on PCI device information and
* OS network device settings (MTU size).
**/
static int ftgmac030_sw_init(struct ftgmac030_ctrl *ctrl)
{
    struct eth_device *netdev = ctrl->netdev;
    u32 fea;

    ctrl->rx_buffer_len = ETH_FRAME_LEN + VLAN_HLEN + ETH_FCS_LEN;
    ctrl->max_frame_size = netdev->mtu + ETH_HLEN + ETH_FCS_LEN;
    ctrl->min_frame_size = ETH_ZLEN + ETH_FCS_LEN;
    ctrl->tx_ring_count = FTGMAC030_DEFAULT_TXD;
    ctrl->rx_ring_count = FTGMAC030_DEFAULT_RXD;

    if (ftgmac030_alloc_queues(ctrl))
        return -ENOMEM;

    fea = ior32(FTGMAC030_REG_FEAR);
    ctrl->max_rx_fifo = fea & 0x7;
    ctrl->max_tx_fifo = (fea >> 4) & 0x7;

    /* Setup hardware time stamping cyclecounter */
    if (ctrl->flags & FLAG_HAS_HW_TIMESTAMP) {
        ctrl->incval = INCVALUE_50MHz;
        ctrl->incval_nns = INCVALUE_50MHz_NNS;

        spin_lock_init(&ctrl->systim_lock);
    }

    /* Explicitly disable IRQ since the NIC can be in any state. */
    ftgmac030_irq_disable(ctrl);

    set_bit(__FTGMAC030_DOWN, &ctrl->state);
    return 0;
}

static int _crc32(uint8_t* s, int length)
{
    int perByte;
    int perBit;
    /* crc polynomial for Ethernet */
    const unsigned long poly = 0xedb88320;
    /* crc value - preinitialized to all 1's */
    unsigned long crc_value = 0xffffffff;
    unsigned long crc_bit_reverse = 0x0, i;

    for(perByte=0; perByte<length; perByte++)
    {
        uint8_t c;

        c = *(s++);
        for(perBit=0; perBit<8; perBit++)
        {
            crc_value = (crc_value >> 1)^(((crc_value^c)&0x01)?poly:0);
            c >>= 1;
        }
    }

    for(i=0; i<32; i++)
        crc_bit_reverse |= ((crc_value>>i) & 0x1) << (31-i);

    //return crc_value;
    return crc_bit_reverse;
}

static inline u8 bitrev8(u8 val)
{
    int i;
	u8 bit_reverse = 0;
    for(i=0; i<8; i++)
        bit_reverse |= ((val>>i) & 0x1) << (7-i);

	return bit_reverse;
}

/**
 * Internal helper function to calculate hash value of
 * Multicast Address Hash Table.
 */
static u32 ftgmac030_hash_mc_addr(unsigned char *mac_addr)
{
	unsigned int crc32 = _crc32(mac_addr, ETH_ALEN);
	crc32 = ~crc32;
	crc32 = bitrev8(crc32 & 0xff) |
		(bitrev8((crc32 >> 8) & 0xff) << 8) |
		(bitrev8((crc32 >> 16) & 0xff) << 16) |
		(bitrev8((crc32 >> 24) & 0xff) << 24);

	/* return MSB 7 bits */
	return ((unsigned char)(crc32 >> 25));
}

//static uint32_t g_mc[4];  // mc test
/**
 * ftgmac030_write_mc_addr_list - write multicast addresses to MAHT
 * @netdev: network interface device structure
 *
 * Writes multicast address list to the MTA hash table.
 * Returns: -ENOMEM on failure
 *                0 on no addresses written
 *                X on writing X addresses to MTA
 */
static int ftgmac030_write_mc_addr_list(struct eth_device *netdev)
{
	struct ftgmac030_ctrl *ctrl = netdev_priv(netdev);
	u32 hash_index, hash_bit, hash_reg;
	u8 *cur_addr;
	int i;

	/* clear mta_shadow */
	memset(&ctrl->mta_shadow, 0, sizeof(ctrl->mta_shadow));

	/* update_mc_addr_list expects a packed array of only addresses. */
    for (i = 0; i < netdev->mc_count; i++) {
		cur_addr = netdev->mc_list + (i*8);
		hash_index = ftgmac030_hash_mc_addr(cur_addr);

		e_dbg(drv, "Multicat MAC address %02X:%02X:%02X:%02X:%02X:%02X, hash_val %d\n",
		cur_addr[0],cur_addr[1],cur_addr[2],cur_addr[3],cur_addr[4],cur_addr[5], hash_index);

		hash_reg = (hash_index >> 5) & (ctrl->mta_reg_count - 1);
		hash_bit = hash_index & 0x1F;

		ctrl->mta_shadow[hash_reg] |= (1 << hash_bit);
	}
#if 0 // mc test
	for (i = 0; i < 4; i++)
		g_mc[i] |= ctrl->mta_shadow[i];
	printf("mc filter: 0x%08X 0x%08X 0x%08X 0x%08X \n", g_mc[0], g_mc[1], g_mc[2], g_mc[3]);
#endif
	/* replace the entire Multicast Address Hash table */
	for (i = 0; i < 2/*ctrl->mta_reg_count*/; i++) {
		FTGMAC030_WRITE_REG_ARRAY(FTGMAC030_REG_MAHT0, i,
					  ctrl->mta_shadow[i]);
		FTGMAC030_WRITE_REG_ARRAY(FTGMAC030_REG_MAHT2, i,
					  ctrl->mta_shadow[i+2]);
    }
	return netdev_mc_count(netdev);
}

/**
 * ftgmac030_set_rx_mode - secondary unicast, Multicast and Promiscuous mode set
 * @netdev: network interface device structure
 *
 * The ndo_set_rx_mode entry point is called whenever the unicast or multicast
 * address list or the network interface flags are updated.  This routine is
 * responsible for configuring the hardware for proper unicast, multicast,
 * promiscuous mode, and all-multi behavior.
 **/
static void ftgmac030_set_rx_mode(struct eth_device *netdev)
{
	struct ftgmac030_ctrl *ctrl = netdev_priv(netdev);
	u32 rctl;

	/* Check for Promiscuous and All Multicast modes */
	rctl = ior32(FTGMAC030_REG_MACCR);

	/* clear the affected bits */
	rctl &= ~(FTGMAC030_MACCR_RX_ALL | FTGMAC030_MACCR_RX_BROADPKT
		  | FTGMAC030_MACCR_HT_MULTI_EN | FTGMAC030_MACCR_RX_MULTIPKT
		  | FTGMAC030_MACCR_JUMBO_LF | FTGMAC030_MACCR_DISCARD_CRCERR);


	if (netdev->flags & IFF_BROADCAST)
		rctl |= FTGMAC030_MACCR_RX_BROADPKT;

	if (netdev->flags & IFF_PROMISC) {
		rctl |= FTGMAC030_MACCR_RX_ALL;
	} else {
		int count;
		if (netdev->flags & IFF_ALLMULTI) {
			rctl |= FTGMAC030_MACCR_RX_MULTIPKT;
		} else if (netdev->flags & IFF_MULTICAST) {
			/*
			 * Try to write MAHT0-1, if the attempt fails
			 * then just set to receive all multicast.
			 */
			count = ftgmac030_write_mc_addr_list(netdev);
			if (count)
				rctl |= FTGMAC030_MACCR_HT_MULTI_EN;
			else
				rctl |= FTGMAC030_MACCR_RX_MULTIPKT;
		}
	}

	/* Rx VLAN tag is stripped
	 */
	if (netdev->features & NETIF_F_HW_VLAN_CTAG_RX)
		rctl |= FTGMAC030_MACCR_REMOVE_VLAN;
	else
		rctl &= ~FTGMAC030_MACCR_REMOVE_VLAN;

	/* Enable Long Packet receive */
	if (ctrl->netdev->mtu > ETH_DATA_LEN)
		rctl |= FTGMAC030_MACCR_JUMBO_LF;

	/* This is useful for sniffing bad packets. */
	if (ctrl->netdev->features & NETIF_F_RXALL) {
		rctl |= (FTGMAC030_MACCR_RX_ALL |
			 FTGMAC030_MACCR_RX_RUNT);
		rctl &= ~FTGMAC030_MACCR_DISCARD_CRCERR;
	} else
		/* Do not Store bad packets */
		rctl |= FTGMAC030_MACCR_DISCARD_CRCERR;

	/* Below are hardware capabilty specfic */
	if (ctrl->flags & FLAG_DISABLE_IPV6_RECOG)
		rctl |= FTGMAC030_MACCR_DIS_IPV6_PKTREC;

	iow32(FTGMAC030_REG_MACCR, rctl);

	iow32(FTGMAC030_REG_RBSR, ctrl->rx_buffer_len);

	/* if receive broadcast larger than BMRCR[4:0] within
	 * BMRCR[23:16] * BMRCR[24] ms/us depends on current
	 * link speed, h/w discards the broadcast packets for
	 * this period time and enable receive again.
	 *
	 * Actually, I don't have any idea or experience what
	 * number should be configured. Below code just a guideline
	 * how to program this register. Please adjust by yourself
	 * according to your real application situations.
	 *
	 * I am assumed to be 0.5 s and receive 1024 broadcast
	 * packets and  BMRCR[24] is one.
	 */
	if (ctrl->flags & FLAG_DISABLE_RX_BROADCAST_PERIOD) {
		u32 bmrcr, time_thrld;

		bmrcr = (1 << 24);

		switch (ctrl->phy_speed) {
		default:
		case SPEED_10:
			time_thrld = 500000000 / 409600;
			break;

		case SPEED_100:
			time_thrld = 500000000 / 40960;
			break;

		case SPEED_1000:
			time_thrld = 500000000 / 8190;
			break;
		}

		bmrcr |= ((time_thrld & 0xff) << 16);

		/* unit is 256 packets */
		bmrcr |= 4;

		iow32(FTGMAC030_REG_BMRCR, bmrcr);
	}
}

static void ftgmac030_set_mac_hw(struct ftgmac030_ctrl *ctrl)
{
    u8 *mac;
    unsigned int maddr, laddr;

    mac = ctrl->netdev->netaddr;
    maddr = mac[0] << 8 | mac[1];
    laddr = mac[2] << 24 | mac[3] << 16 | mac[4] << 8 | mac[5];

    iow32(FTGMAC030_REG_MAC_MADR, maddr);
    iow32(FTGMAC030_REG_MAC_LADR, laddr);
}

/**
* ftgmac030_configure - configure the hardware for Rx and Tx
* @ctrl: private board structure
**/
static void ftgmac030_configure(struct ftgmac030_ctrl *ctrl)
{
    struct ftgmac030_ring *rx_ring = ctrl->rx_ring;
    u32 maccr;

    ftgmac030_set_mac_hw(ctrl);

    ftgmac030_configure_tx(ctrl);

    ftgmac030_set_rx_mode(ctrl->netdev);
    ftgmac030_configure_rx(ctrl);

    ctrl->alloc_rx_buf(rx_ring, ftgmac030_desc_unused(rx_ring), GFP_KERNEL);

    maccr = ior32(FTGMAC030_REG_MACCR);
    maccr |= (FTGMAC030_MACCR_TXDMA_EN | FTGMAC030_MACCR_RXDMA_EN
        | FTGMAC030_MACCR_TXMAC_EN | FTGMAC030_MACCR_RXMAC_EN);

    if (1)
    {
        if (ctrl->mode == ITE_ETH_MAC_LB) {
            maccr |= FTGMAC030_MACCR_LOOP_EN;
            ctrl->phy_speed = SPEED_100;
            ctrl->phy_duplex = DUPLEX_FULL;
        }
        else if (ctrl->mode == ITE_ETH_MAC_LB_1000) {
            maccr |= FTGMAC030_MACCR_LOOP_EN;
            ctrl->phy_speed = SPEED_1000;
            ctrl->phy_duplex = DUPLEX_FULL;
        }
        else
            phy_read_mode(ctrl->netdev, &ctrl->phy_speed, &ctrl->phy_duplex);

        #if 0
        if (ctrl->mode != ITE_ETH_REAL) {
            maccr |= FTGMAC030_MACCR_RX_ALL;
			#if defined(MAC_CRC_DIS)
            maccr |= FTGMAC030_MACCR_DISCARD_CRCERR;
			#endif
        }
        #endif
    }

    /* adjust timeout factor according to speed/duplex */
    maccr &= ~(FTGMAC030_MACCR_MODE_100 | FTGMAC030_MACCR_MODE_1000);
    switch (ctrl->phy_speed) {
    default:
    case SPEED_10:
        ctrl->tx_timeout_factor = 16;
        break;

    case SPEED_100:
        ctrl->tx_timeout_factor = 10;
        maccr |= FTGMAC030_MACCR_MODE_100;
        break;

    case SPEED_1000:
        ctrl->tx_timeout_factor = 1;
        maccr |= FTGMAC030_MACCR_MODE_1000;
        break;
    }

    if (ctrl->phy_duplex)
        maccr |= FTGMAC030_MACCR_FULLDUP;
    else
        maccr &= ~FTGMAC030_MACCR_FULLDUP;

    iow32(FTGMAC030_REG_MACCR, maccr);
}

/**
*  ftgmac030_set_flow_control - Set flow control high/low watermarks
*  @ctrl: pointer to the HW structure
*
*  Sets the flow control high/low threshold (watermark) registers.
*
*  FTGMAC030 does not support disable transmision of XON frame.
*  A pause frame is sent with pause time = 0 when the RX FIFO free
*  space is larger than the high threshold.
**/
s32 ftgmac030_set_flow_control(struct ftgmac030_ctrl *ctrl)
{
    return 0;
}

/**
*  ftgmac030_init_hw - Initialize hardware
*  @ctrl: pointer to the HW structure
*
*  This inits the hardware readying it for operation.
**/
static s32 ftgmac030_init_hw(struct ftgmac030_ctrl *ctrl)
{
    s32 ret_val;
    u16 i;

    /* Zero out the Multicast HASH table */
    e_dbg("hw:", "Zeroing the Multicast HASH table\n");
    for (i = 0; i < 2/*ctrl->mta_reg_count*/; i++) {
        FTGMAC030_WRITE_REG_ARRAY(FTGMAC030_REG_MAHT0, i, 0);
        FTGMAC030_WRITE_REG_ARRAY(FTGMAC030_REG_MAHT2, i, 0);
    }

    /* Setup link and flow control */
    ret_val = ftgmac030_set_flow_control(ctrl);

    return ret_val;
}

/**
*  ftgmac030_reset_hw - Reset hardware
*  @ctrl: pointer to the controller structure
*
*  This resets the hardware into a known state.
**/
static s32 ftgmac030_reset_hw(struct ftgmac030_ctrl *ctrl)
{
    int i;

    /* NOTE: reset clears all registers */
    iow32(FTGMAC030_REG_MACCR, FTGMAC030_MACCR_SW_RST);
    for (i = 0; i < 5; i++) {
        unsigned int maccr;

        maccr = ior32(FTGMAC030_REG_MACCR);
        if (!(maccr & FTGMAC030_MACCR_SW_RST))
            return 0;

        udelay(1000);
    }

    e_err("hw:", "Software reset failed\n");
    return -EIO;
}

/**
* ftgmac030_reset - bring the hardware into a known good state
*
* This function boots the hardware and enables some settings that
* require a configuration cycle of the hardware - those cannot be
* set/changed during runtime. After reset the device needs to be
* properly configured for Rx, Tx etc.
*/
void ftgmac030_reset(struct ftgmac030_ctrl *ctrl)
{
    u32 tx_space, min_tx_space, min_rx_space;
    u32 fifo, fifo_val;

    /* Allow time for pending master requests to run */
    ftgmac030_reset_hw(ctrl);

    fifo = ior32(FTGMAC030_REG_TPAFCR);
    fifo &= ~(FTGMAC030_TPAFCR_TFIFO_SIZE(7) |
        FTGMAC030_TPAFCR_RFIFO_SIZE(7));

    /* To maintain wire speed transmits, the Tx FIFO should be
    * large enough to accommodate two full transmit packets,
    * rounded up to the next 1KB and expressed in KB.
    */
    min_tx_space = (ctrl->max_frame_size +
        sizeof(struct ftgmac030_txdes) -
        ETH_FCS_LEN) * 2;
    min_tx_space = ALIGN(min_tx_space, 1024);

    if (min_tx_space <= (2 << 10))
        fifo_val = 0;
    else if (min_tx_space <= (4 << 10))
        fifo_val = 1;
    else if (min_tx_space <= (8 << 10))
        fifo_val = 2;
    else if (min_tx_space <= (16 << 10))
        fifo_val = 3;
    else if (min_tx_space <= (32 << 10))
        fifo_val = 4;
    else if (min_tx_space <= (64 << 10))
        fifo_val = 5;
    else
        fifo_val = 6;

    if (fifo_val > 5 || fifo_val > ctrl->max_tx_fifo) {
        LOG_WARNING "tx fifo sel(%d) larger than hw max(%d)\n",
            fifo_val, ctrl->max_tx_fifo LOG_END
            fifo_val = ctrl->max_tx_fifo;
    }

    fifo |= FTGMAC030_TPAFCR_TFIFO_SIZE(fifo_val);

    /* software strips receive CRC, so leave room for it */
    min_rx_space = ctrl->max_frame_size;
    min_rx_space = ALIGN(min_rx_space, 1024);

    if (min_rx_space <= (2 << 10))
        fifo_val = 0;
    else if (min_rx_space <= (4 << 10))
        fifo_val = 1;
    else if (min_rx_space <= (8 << 10))
        fifo_val = 2;
    else if (min_rx_space <= (16 << 10))
        fifo_val = 3;
    else if (min_rx_space <= (32 << 10))
        fifo_val = 4;
    else if (min_rx_space <= (64 << 10))
        fifo_val = 5;
    else
        fifo_val = 6;

    if (fifo_val > 5 || fifo_val > ctrl->max_rx_fifo) {
        LOG_WARNING "rx fifo sel(%d) larger than hw max(%d)\n",
            fifo_val, ctrl->max_rx_fifo LOG_END
            fifo_val = ctrl->max_rx_fifo;
    }

    fifo |= FTGMAC030_TPAFCR_RFIFO_SIZE(fifo_val);

    iow32(FTGMAC030_REG_TPAFCR, fifo);

    /* flow control settings
    *
    * The high water mark must be low enough to fit one full frame
    * (or the size used for early receive) above it in the Rx FIFO.
    * Set it to the lower of 10% of:
    * - the Rx FIFO free size
    * - the full Rx FIFO size minus one full frame
    */
    if (ctrl->flags & FLAG_DISABLE_FC_PAUSE_TIME)
        ctrl->pause_time = 0;
    else
        ctrl->pause_time = FTGMAC030_FC_PAUSE_TIME;

    fifo = FTGMAC030_TPAFCR_RFIFO_VAL(ior32(FTGMAC030_REG_TPAFCR));
    fifo = ((1 << fifo) * 2048);

    ctrl->low_water = min((fifo / 10), (fifo - ctrl->max_frame_size));
    ctrl->high_water = (fifo * 9) / 10;

    /* Maximum size per Tx descriptor limited to 14 bits.
    * Align it with 1024 bytes, so total 15360 bytes.
    */
    fifo = FTGMAC030_TPAFCR_TFIFO_VAL(ior32(FTGMAC030_REG_TPAFCR));
    tx_space = ((1 << fifo) * 2048);

    ctrl->tx_fifo_limit = min_t(u32, tx_space, 15360);

    /* Disable Adaptive Interrupt Moderation if 2 full packets cannot
    * fit in receive buffer.
    */
    fifo = FTGMAC030_TPAFCR_RFIFO_VAL(ior32(FTGMAC030_REG_TPAFCR));
    fifo = ((1 << fifo) * 2048);

    if (ctrl->itr_setting & 0x3) {
        if ((ctrl->max_frame_size * 2) > fifo) {
            if (!(ctrl->flags & FLAG_DISABLE_AIM)) {
                e_info("hw:", "Interrupt Throttle Rate off\n");
                ctrl->flags |= FLAG_DISABLE_AIM;
                ctrl->itr = 0;
                ftgmac030_write_itr(ctrl, 0);
            }
        }
        else if (ctrl->flags & FLAG_DISABLE_AIM) {
            e_info("hw:", "Interrupt Throttle Rate on\n");
            ctrl->flags &= ~FLAG_DISABLE_AIM;
            ctrl->itr = 20000;
            ftgmac030_write_itr(ctrl, ctrl->itr);
        }
    }

    e_info("hw:", "tx_fifo %d rx_fifo %d fc low %d fc high %d itr %d\n",
        ctrl->tx_fifo_limit, min_rx_space, ctrl->low_water,
        ctrl->high_water, ctrl->itr);

    if (ftgmac030_init_hw(ctrl))
        e_err("hw:", "Hardware Error\n");

#if defined(PTP_ENABLE) // Irene Lin
    /* initialize systim and reset the ns time counter */
    ctrl->hwtstamp_config.flags = 0;
    ctrl->hwtstamp_config.tx_type = HWTSTAMP_TX_OFF;
    ctrl->hwtstamp_config.rx_filter = HWTSTAMP_FILTER_NONE;
    ftgmac030_config_hwtstamp(ctrl, &ctrl->hwtstamp_config);
#endif

    /* Set EEE advertisement as appropriate */
    if (ctrl->flags & FLAG_HAS_EEE) {
        /* Implement for PHY that has EEE */
    }
}

int ftgmac030_up(struct ftgmac030_ctrl *ctrl)
{
	/* hardware has been reset, we need to reload some things */
	ftgmac030_configure(ctrl);

	clear_bit(__FTGMAC030_DOWN, &ctrl->state);

	ftgmac030_irq_enable(ctrl);

	netif_start_queue(ctrl->netdev);

	return 0;
}

/**
 * ftgmac030_down - quiesce the device and optionally reset the hardware
 * @ctrl: board private structure
 * @reset: boolean flag to reset the hardware or not
 */
void ftgmac030_down(struct ftgmac030_ctrl *ctrl, bool reset)
{
	struct eth_device *netdev = ctrl->netdev;
	u32 maccr;

	ftgmac030_irq_disable(ctrl);

	/* signal that we're down so the interrupt handler does not
	 */
	set_bit(__FTGMAC030_DOWN, &ctrl->state);

	maccr = ior32(FTGMAC030_REG_MACCR);

	/* disable receives in the hardware */
	maccr &= ~(FTGMAC030_MACCR_RXDMA_EN | FTGMAC030_MACCR_RXMAC_EN);
	iow32(FTGMAC030_REG_MACCR, maccr);
	/* flush and sleep below */

	netif_stop_queue(netdev);

	/* disable transmits in the hardware */
	maccr &= ~(FTGMAC030_MACCR_TXDMA_EN | FTGMAC030_MACCR_TXMAC_EN);
	iow32(FTGMAC030_REG_MACCR, maccr);

	//usleep_range(10000, 20000);

	ftgmac030_clean_tx_ring(ctrl->tx_ring);
	ftgmac030_clean_rx_ring(ctrl->rx_ring);

	if (reset)
		ftgmac030_reset(ctrl);
}

static void ftgmac030_show_feature(struct ftgmac030_ctrl *ctrl)
{
#if SHOW_HW_INFO
    u8 *fifo_size[8] = { "2K", "4K", "8K", "16K", "32K", "Invalid", "Invalid", "Invalid" };
    u32 val = ior32(FTGMAC030_REG_FEAR);

    printf(" Interface support: ");
    if (val & (0x1 << 27))
        printf("RGMII ");
    if (val & (0x1 << 26))
        printf("RMII ");
    if (val & (0x1 << 25))
        printf("GMII ");
    if (val & (0x1 << 24))
        printf("MII ");
    printf("\n");

    printf("PTP support: %s \n", (val & (0x1 << 18)) ? "Yes" : "No");
    printf("TCP/IP checksum offload: %s \n", (val & (0x1 << 17)) ? "Yes" : "No");
    printf("Host interface: %s \n", (val & (0x1 << 16)) ? "AXI 64-bit" : "AHB 32-bit");
    if (val & (0x1 << 16))
        printf("AXI ID width: %d \n", ((val >> 8) & 0x1F));
    printf("Tx FIFO Size: %s \n", fifo_size[(val >> 4) & 0x7]);
    printf("Rx FIFO Size: %s \n", fifo_size[(val >> 0) & 0x7]);
#endif
}
