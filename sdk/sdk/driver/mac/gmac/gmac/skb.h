#ifndef	GMAC_SKB_H
#define	GMAC_SKB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/os.h>
#include "ndis.h"
#include "ite/itp.h"


/* Don't change this without changing skb_csum_unnecessary! */
#define CHECKSUM_NONE		0
#define CHECKSUM_UNNECESSARY	1
#define CHECKSUM_COMPLETE	2
#define CHECKSUM_PARTIAL	3

struct sk_buff {
    u8      *data;
    unsigned int    len;
    unsigned int    data_len;

    __be16     protocol;  /* big-endian */
    u8      ip_summed : 2;
    u8      alloc_data_buf : 1;
};

#define skb_trim(a,b)
#define skb_align(a,b)  /* already align */

struct eth_device;

struct sk_buff *__netdev_alloc_skb(struct eth_device *netdev,
    unsigned int bufsz, gfp_t gfp);

#define dev_kfree_skb	dev_kfree_skb_any

static inline void dev_kfree_skb_any(struct sk_buff *skb)
{
    if (skb->alloc_data_buf)
        itpVmemFree((u32)skb->data);

    free(skb);
}
static inline unsigned int skb_headlen(const struct sk_buff *skb)
{
    return skb->len - skb->data_len;
}



#ifdef __cplusplus
}
#endif

#endif
