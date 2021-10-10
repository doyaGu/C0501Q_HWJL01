#ifndef __ITE_NDIS_AMEBA__
#define __ITE_NDIS_AMEBA__

#include "wireless.h"
#include "ite_skbuf.h"

//----- ------------------------------------------------------------------
// Device structure
//----- ------------------------------------------------------------------
struct net_device_stats {
	unsigned long   rx_packets;             /* total packets received       */
	unsigned long   tx_packets;             /* total packets transmitted    */
	unsigned long   rx_dropped;             /* no space in linux buffers    */
	unsigned long   tx_dropped;             /* no space available in linux  */
	unsigned long   rx_bytes;               /* total bytes received         */
	unsigned long   tx_bytes;               /* total bytes transmitted      */
};

typedef void (*input_fn)(void *arg, void *packet, int len);

struct net_device {
	char			    name[16];
	unsigned long		state;
    unsigned short	    flags;          /* interface flags (a la BSD)   */
    void			    *priv;		    /* pointer to private data */
	unsigned char		dev_addr[6];	/* set during bootup */
	int (*init)(void);
	int (*open)(struct net_device *dev);
	int (*stop)(struct net_device *dev);
	int (*hard_start_xmit)(struct sk_buff *skb, struct net_device *dev);
	int	(*set_mac_address)(struct net_device *dev, void *addr);  // Irene Lin
	int (*do_ioctl)(struct net_device *dev, struct iwreq *ifr, int cmd);
	struct net_device_stats* (*get_stats)(struct net_device *dev);

	/* for lwip interface */
	input_fn rx_cb;
	void *rx_netif;
};

#define netdev_priv(dev)		dev->priv

#define	__LINK_STATE_XOFF 				(1 << 0)
#define	__LINK_STATE_START				(1 << 1)
#define	__LINK_STATE_PRESENT			(1 << 2)
#define	__LINK_STATE_SCHED				(1 << 3)
#define	__LINK_STATE_NOCARRIER			(1 << 4)
#define	__LINK_STATE_RX_SCHED			(1 << 5)
#define	__LINK_STATE_LINKWATCH_PENDING	(1 << 6)

#define netif_start_queue(dev)		    (dev->state &= ~__LINK_STATE_XOFF)
#define netif_stop_queue(dev)		    (dev->state |= __LINK_STATE_XOFF)
#define netif_queue_stopped(dev)		(dev->state & __LINK_STATE_XOFF)
#define netif_wake_queue(dev)		    (dev->state &= ~__LINK_STATE_XOFF)
#define netif_carrier_on(dev)			(dev->state &= ~__LINK_STATE_NOCARRIER)
#define netif_carrier_off(dev)			(dev->state |= __LINK_STATE_NOCARRIER)	
#define netif_carrier_ok(dev)			!(dev->state & __LINK_STATE_NOCARRIER)


void iteAmebaSetNetdev(struct net_device *dev);
struct net_device *iteAmebaGetNetdev(void);
struct net_device *iteAmebaNetdevAlloc(int sizeof_priv);
void iteAmebaNetdevFree(struct net_device* dev);
int iteAmebaOpen(struct net_device *dev, input_fn input, void *arg);
int iteAmebaClose(struct net_device *dev);
int iteAmebaTransmit(struct net_device *dev, struct sk_buff *skb);
int iteAmebaReceive(struct sk_buff *skb);
int iteAmebaGetLink(struct net_device *dev);


#define alloc_etherdev              iteAmebaNetdevAlloc
#define free_netdev                 iteAmebaNetdevFree
#define SET_NETDEV_DEV(a,b)
#define SET_MODULE_OWNER(x)
static inline int register_netdev(struct net_device *dev)
{
    iteAmebaSetNetdev(dev); 
    return 0;
}
#define unregister_netdev(_dev)		iteAmebaSetNetdev(NULL)
#define netif_rx(skb)               iteAmebaReceive(skb)
#define eth_type_trans(skb, dev)    (0x00) /*No need in lwip*/

int iteAmebaSdioWifiRegister(void);


#endif //__ITE_NDIS_AMEBA__