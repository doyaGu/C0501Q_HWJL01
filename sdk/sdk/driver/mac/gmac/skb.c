#include <stdio.h>
#include "if_ether.h"
#include "gmac/skb.h"


struct sk_buff *__netdev_alloc_skb(struct eth_device *netdev,
    unsigned int bufsz, gfp_t gfp)
{
    struct sk_buff *skb;
    u8 *data;

    skb = malloc(sizeof(struct sk_buff));
    if (!skb)
        goto out;

    memset(skb, 0, sizeof(struct sk_buff));
    if (bufsz) {
        data = (u8*)itpVmemAlloc(bufsz);   /** already 64-bytes alignment */
        if (!data)
            goto alloc_fail;

        skb->data = data;
        skb->alloc_data_buf = 1;
    }

out:
    if (!skb)
        printf("[MAC] alloc skb fail!!!\n");
    return skb;
alloc_fail:
    free(skb);
    goto out;
}

/**
* eth_proto_is_802_3 - Determine if a given Ethertype/length is a protocol
* @proto: Ethertype/length value to be tested
*
* Check that the value from the Ethertype/length field is a valid Ethertype.
*
* Return true if the valid is an 802.3 supported Ethertype.
*/
static inline bool eth_proto_is_802_3(__be16 proto)
{
#if BYTE_ORDER == LITTLE_ENDIAN
    /* if CPU is little endian mask off bits representing LSB */
    proto &= htons(0xFF00);
#endif
    printf("eth_proto_is_802_3(%X) %X >= %X ?", (u16)proto, (u16)htons(ETH_P_802_3_MIN));
    /* cast both to u16 and compare since LSB can be ignored */
    return (u16)proto >= (u16)htons(ETH_P_802_3_MIN);
}

__be16 eth_type_trans(struct sk_buff *skb, struct eth_device *dev)
{
    const struct ethhdr *eth;

    eth = (struct ethhdr *)skb->data;

    if (likely(eth_proto_is_802_3(eth->h_proto)))
        return eth->h_proto;

    printf("eth_type_trans() return htons(ETH_P_802_2) \n");
    /*
    *      Real 802.2 LLC
    */
    return htons(ETH_P_802_2);
}
