//#include "rt_config.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


#include "ite_port.h"

struct net_device *smNetDevice;
struct net_device *smEthDevice;



/*Simple Linux NDIS implementation*/
struct net_device *smNetAlloc(int sizeof_priv)
{
    void *priv = NULL;
    struct net_device *dev = NULL;

    dev = kmalloc(sizeof(struct net_device), GFP_KERNEL);
    memset(dev, 0, sizeof(struct net_device));
    priv = kmalloc(sizeof_priv, GFP_KERNEL); 
    memset(priv, 0, sizeof_priv);

    dev->priv = priv;
    sprintf(dev->name, "wlan0");
    dev->flags = IFF_BROADCAST|IFF_MULTICAST;

    printk("@@@@Alloc PROV_DEIVE size [%s] : %d \n", dev->name, sizeof_priv);

    return dev;
}

struct net_device *smEthAlloc(int sizeof_priv)
{
    void *priv = NULL;
    struct net_device *dev = NULL;

    dev = kmalloc(sizeof(struct net_device), GFP_KERNEL);
    memset(dev, 0, sizeof(struct net_device));
    priv = kmalloc(sizeof_priv, GFP_KERNEL); 
    memset(priv, 0, sizeof_priv);

    dev->priv = priv;
    sprintf(dev->name, "mgnt.wlan0");
    dev->flags = IFF_BROADCAST|IFF_MULTICAST;

    printk("@@@@Alloc PROV_DEIVE size [%s] : %d \n", dev->name, sizeof_priv);

    return dev;
}



void* smNetdev_priv(struct net_device *dev)
{
    return (char *)dev->priv;
}


void smNetFree(struct net_device* dev)
{
    if(dev && dev->priv)
        kfree(dev->priv);

    if(dev)
        kfree(dev);
}

int smNetOpen(struct net_device *dev, input_fn input, void *arg)
{
    int ret = 0;

    /* Is it already up?	*/
    if (dev->flags & IFF_UP)
        return 0;

    dev->state |= __LINK_STATE_START;
    if (dev->open) 
        ret = dev->open(dev);

    if (!ret) 
        dev->flags |= IFF_UP;

#ifdef LWIP_INTERFACE
    dev->rxNetif = arg;
    dev->rxCallBack = input;
#endif

    return ret;
}

int smNetClose(struct net_device *dev)
{
    /** rt73 dongle is removed and disconnected */
    if(!smNetDevice) 
        return 0;

    if (!(dev->flags & IFF_UP))
        return 0;

    dev->state &= ~__LINK_STATE_START;
    if (dev->stop)
        dev->stop(dev);

    dev->flags &= ~IFF_UP;

#ifdef LWIP_INTERFACE
    dev->rxNetif = NULL;
    dev->rxCallBack = NULL;
#endif

    return 0;
}
    
void smNetTransmit(struct net_device *dev, struct sk_buff *skb)
{
    if (dev->flags & IFF_UP)
    {
        if(!netif_queue_stopped(dev))
        {
            if (dev->hard_start_xmit) 
                (dev->hard_start_xmit)(skb, dev);
            else
            {

				dev_kfree_skb_any(skb);
            }	
        }
		else
		{ 
                dev_kfree_skb_any(skb);    
        }
    }
    else
    {
        dev_kfree_skb_any(skb);    
    }

    
    return;
}

void smNetReceive(struct sk_buff *skb)
{
#ifdef LWIP_INTERFACE
    struct net_device* dev = skb->dev;
    if(dev && dev->rxNetif && dev->rxCallBack)
    {
        dev->rxCallBack(dev->rxNetif, skb->data, skb->len);
    }
    else
#endif
    {
        int i;
        
        printk("@@@@Get a packet from WiFi, size = %d \n\n", skb->len);
        for (i = 0; i < (int)skb->len; i++)
        {
            if ((i % 16) == 0) printk("\n");
            printk("%02x ", skb->data[i]);
        }
        printk("\n");
    }

    dev_kfree_skb_any(skb);
    
    return;
}

static void dumpConfig(struct net_device_config *netDeviceConfig)
{
    char modeString[4][8] = {"OFF", "STA", "ADHOC"};
    char secString[5][8] = {"NOSEC", "WEP", "WPAPSK", "WPA2PSK", "WPS"};
    char authString[3][8] = {"OPEN", "SHARED", "AUTO"};
    char feaString[3][8] = {"ANYSSID", "PS", "WMM"};
    int i, j , keyLen;

    printk("\n==========================================\n");
    printk(" Operation Mode : %s \n", modeString[netDeviceConfig->operationMode]);
    if (netDeviceConfig->operationMode == WLAN_MODE_ADHOC)
        printk(" Channel ID     : %d \n", netDeviceConfig->channelId);
    printk(" SSID           : %s \n", netDeviceConfig->ssidName);
    printk(" Security Mode  : %s \n", secString[netDeviceConfig->securitySuit.securityMode]);
    if (netDeviceConfig->securitySuit.securityMode == WLAN_SEC_WEP)
    {
        printk(" Auth. Mode     : %s \n", authString[netDeviceConfig->securitySuit.authMode]);
        printk(" Default Key ID : %d \n", netDeviceConfig->securitySuit.defaultKeyId);
        for (i = 0; i < 4; i++)
        {
            keyLen = strlen(netDeviceConfig->securitySuit.wepKeys[i]);
            printk(" WEP KEY %d     : ", i);
            for (j = 0; j < keyLen; j++)
                printk("%02x ", netDeviceConfig->securitySuit.wepKeys[i][j]);
            printk("\n");
        }
    }
    else if ((netDeviceConfig->securitySuit.securityMode == WLAN_SEC_WPAPSK) ||
        (netDeviceConfig->securitySuit.securityMode == WLAN_SEC_WPA2PSK))       
        printk(" Pre-Shared Key : %s \n", netDeviceConfig->securitySuit.preShareKey);
    printk(" Features       : ");
    for (i = 0; i < 3; i++)
    {
        if (netDeviceConfig->feaFlags & (1<<i))
            printk("%s ", feaString[i]);
    }
    printk("\n==========================================\n");
    
    return;
}

static void dumpInfo(struct net_device_info *netDeviceInfo)
{
    char modeString[3][6] = {"INFRA", "ADHOC", "UNKNW"};
    char secString[2][8] = {"SEC_OFF", "SEC_ON "};
    int i;

    printk("\n==========================================\n");
    if (netDeviceInfo->infoType == WLAN_INFO_LINK)
    {
        printk("AP MAC ADDR : %02x:%02x:%02x:%02x:%02x:%02x\n",
            netDeviceInfo->apMacAddr[0],
            netDeviceInfo->apMacAddr[1],
            netDeviceInfo->apMacAddr[2],
            netDeviceInfo->apMacAddr[3],
            netDeviceInfo->apMacAddr[4],
            netDeviceInfo->apMacAddr[5]);
        printk("Channel ID  : %02d\n", netDeviceInfo->channelId);
        printk("SSID Name   : %s \n", netDeviceInfo->ssidName);
    }
    else if (netDeviceInfo->infoType == WLAN_INFO_SCAN)
    {
        for (i = 0; i < netDeviceInfo->apCnt; i++)
        {
            printk("[%02d] [%02d] <%02x:%02x:%02x:%02x:%02x:%02x> [%32s][%s][%s] \n", i,
                        netDeviceInfo->apList[i].channelId,
                        netDeviceInfo->apList[i].apMacAddr[0],
                        netDeviceInfo->apList[i].apMacAddr[1],
                        netDeviceInfo->apList[i].apMacAddr[2],
                        netDeviceInfo->apList[i].apMacAddr[3],
                        netDeviceInfo->apList[i].apMacAddr[4],
                        netDeviceInfo->apList[i].apMacAddr[5],
                        netDeviceInfo->apList[i].ssidName,
                        modeString[netDeviceInfo->apList[i].operationMode],
                        secString[netDeviceInfo->apList[i].securityOn]);
        }
    }
    printk("\n==========================================\n");
    
    return;
}

int smNetCtrl(struct net_device *dev, struct net_device_config *netDeviceConfig)
{
    int ret = -1;

    if (dev->wireless_control)
    {
        dumpConfig(netDeviceConfig);
        ret = (*dev->wireless_control)(dev, netDeviceConfig);
    }
    
    return ret;
}

int smNetIOCtrl(struct net_device *dev, void* ptr, int cmd)
{
    int ret = -1;

    if (dev->do_ioctl)
    {
        ret = (*dev->do_ioctl)(dev, (struct ifreq *)ptr, cmd);
    }
    
    return ret;
}


int smNetInfo(struct net_device *dev, struct net_device_info *netDeviceInfo)
{
    int ret = -1;

    if (dev->wireless_information)
    {
        ret = (*dev->wireless_information)(dev, netDeviceInfo);
        //dumpInfo(netDeviceInfo);
    }
    
    return ret;
}



struct net_device* smNetGetDevice(void)
{
    return smNetDevice;
}

struct net_device* smEthGetDevice(void)
{
    return smEthDevice;
}

