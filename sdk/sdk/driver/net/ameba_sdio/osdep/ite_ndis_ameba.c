#include <string.h>
#include <malloc.h>
#include <net/if.h>
#include <freertos/wireless.h>
#include <freertos/ite_ndis_ameba.h>

static struct net_device *amebaNetDevice = NULL;

void iteAmebaSetNetdev(struct net_device *dev)
{
    amebaNetDevice = dev;
}

struct net_device *iteAmebaGetNetdev(void)
{
    return amebaNetDevice;
}

struct net_device *iteAmebaNetdevAlloc(int sizeof_priv)
{
    void *priv = NULL;
    struct net_device *dev = NULL;

    dev = malloc(sizeof(struct net_device));
    memset(dev, 0, sizeof(struct net_device));

    if (sizeof_priv) {
        priv = malloc(sizeof_priv);
        memset(priv, 0, sizeof_priv);

        dev->priv = priv;
    }
    else
        dev->priv = NULL;

    sprintf(dev->name, "amebaSdio"); 					//INF_MAIN_DEV_NAME + '0'

    printf("\r\n=== net_device = %p, name = %s ===\r\n\r\n", dev, dev->name);

    return dev;
}

void iteAmebaNetdevFree(struct net_device* dev)
{
    if (dev && dev->priv)
        free(dev->priv);

    if (dev)
        free(dev);
}


int iteAmebaOpen(struct net_device *dev, input_fn input, void *arg)
{
    int ret = 0;

    if (!dev)
        return -1;

    /* Is it already up?	*/
    if (dev->flags & IFF_UP)
        return 0;

    dev->state |= __LINK_STATE_START;
    if (dev->open)
        ret = dev->open(dev);

    if (!ret)
    {
        dev->flags |= IFF_UP;

        dev->rx_cb = input;
        dev->rx_netif = arg;
    }

    return ret;
}

int iteAmebaClose(struct net_device *dev)
{
    /** removed and disconnected */
    if (!dev)
        return 0;

    if (!(dev->flags & IFF_UP))
        return 0;

    dev->state &= ~__LINK_STATE_START;
    if (dev->stop)
        dev->stop(dev);

    dev->flags &= ~IFF_UP;

    dev->rx_cb = NULL;
    dev->rx_netif = NULL;

    return 0;
}

int iteAmebaTransmit(struct net_device *dev, struct sk_buff *skb)
{
    int rc = -1;

    if (dev->flags & IFF_UP) 
    {
        if (!netif_queue_stopped(dev)) 
        {
            skb->dev = dev;
            if (dev->hard_start_xmit)
                rc = (dev->hard_start_xmit)(skb, dev);
            else
                dev_kfree_skb_any(skb);
        }
        else
            dev_kfree_skb_any(skb);
    }
    else
        dev_kfree_skb_any(skb);

    return rc;
}

int iteAmebaReceive(struct sk_buff *skb)
{
#if 1
    struct net_device* dev = skb->dev;
    if (dev && dev->rx_cb)
    {
        dev->rx_cb(dev->rx_netif, skb->data, skb->len);
    }
#endif

#if 0 
    {
        int i;

        printf("\r\n[%04d]", skb->len);
        for (i = 0; i < 14; i++)
        {
            printf("%02x ", skb->data[i]);
        }
        printf("\r\n");
    }
#endif		

    dev_kfree_skb_any(skb);

    return 0;
}

int iteAmebaGetLink(struct net_device *dev)
{
    if (netif_carrier_ok(dev))
        return 1;
    else
        return 0;
}

#define RTL_IOCTL_ATCMD				(SIOCDEVPRIVATE+1)

int iteAmebaIoctl(struct net_device *dev, u8 *cmd)
{
    struct iwreq iw = { 0 };
    u8 *if_name = "wlan0";
    int ret;

    if (!dev)
        return -1;

    memcpy(iw.ifr_name, if_name, strlen(if_name));
    iw.u.data.pointer = cmd;
    iw.u.data.length = strlen(cmd);

    ret = dev->do_ioctl(dev, &iw, RTL_IOCTL_ATCMD);
    if (ret)
        printf("%s cmd fail! \n", cmd);

    return ret;
}

