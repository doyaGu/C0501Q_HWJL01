#ifndef	ITE_WIFI_NDIS_H
#define	ITE_WIFI_NDIS_H

#include "if.h" // An implementation of the TCP/IP protocol suite.
#include "iw_handler.h"

struct net_device_stats
{
	unsigned long   rx_packets;             /* total packets received       */
	unsigned long   tx_packets;             /* total packets transmitted    */
	unsigned long   rx_bytes;               /* total bytes received         */
	unsigned long   tx_bytes;               /* total bytes transmitted      */
	unsigned long   rx_errors;              /* bad packets received         */
	unsigned long   tx_errors;              /* packet transmit problems     */
	unsigned long   rx_dropped;             /* no space in linux buffers    */
	unsigned long   tx_dropped;             /* no space available in linux  */
	unsigned long   multicast;              /* multicast packets received   */
	unsigned long   collisions;

	/* detailed rx_errors: */
	unsigned long   rx_length_errors;
	unsigned long   rx_over_errors;         /* receiver ring buff overflow  */
	unsigned long   rx_crc_errors;          /* recved pkt with crc error    */
	unsigned long   rx_frame_errors;        /* recv'd frame alignment error */
	unsigned long   rx_fifo_errors;         /* recv'r fifo overrun          */
	unsigned long   rx_missed_errors;       /* receiver missed packet       */

	/* detailed tx_errors */
	unsigned long   tx_aborted_errors;
	unsigned long   tx_carrier_errors;
	unsigned long   tx_fifo_errors;
	unsigned long   tx_heartbeat_errors;
	unsigned long   tx_window_errors;

	/* for cslip etc */
	unsigned long   rx_compressed;
	unsigned long   tx_compressed;
};

#if 1
struct net_device_config {
#define WLAN_MODE_OFF               0
#define WLAN_MODE_STA               1
#define WLAN_MODE_ADHOC             2
	int operationMode;	    /*Operation Type*/
    int channelId;          /*Channel ID*/

    char ssidName[32];      /*SSID name*/

    struct
    {
#define WLAN_SEC_NOSEC              0
#define WLAN_SEC_WEP                1
#define WLAN_SEC_WPAPSK             2  /** default is TKIP */
#define WLAN_SEC_WPA2PSK            3  /** default is AES */
#define WLAN_SEC_WPAPSK_AES         4
#define WLAN_SEC_WPA2PSK_TKIP       5
#define WLAN_SEC_WPAPSK_MIX         6
#define WLAN_SEC_WPS                7
#define WLAN_SEC_EAP                8

    	int securityMode;	    /*Sec. Mode*/		

#define WLAN_AUTH_OPENSYSTEM        0
#define WLAN_AUTH_SHAREDKEY         1
#define WLAN_AUTH_AUTO              2
        int authMode;           /*Auth. Mode*/

        int         defaultKeyId;       /*WEP default KEYID, from 0 to 3*/
    	unsigned char		wepKeys[4][32];     /*WEP*/
        unsigned char       preShareKey[64];    /*WPAPSK/WPA2PSK*/

#define WLAN_WPS_PIN                0 /** not support */
#define WLAN_WPS_PBC                1
#define WLAN_WPS_PIN_START          2 /** not support */
#define WLAN_WPS_STOP               3
        int wpsMode;
    } securitySuit;

    void (*write_config_cb)(void* config);  /*for WPS write config to file*/

#define WLAN_FEA_ANYSSID    		(1<<0)  /*Any SSID*/
	unsigned int feaFlags;		    /*Feature flags*/			

};

struct net_device_info {
#define WLAN_INFO_LINK      0
#define WLAN_INFO_AP        1
#define WLAN_INFO_SCAN      2
#define WLAN_INFO_SCAN_GET  3
#define WLAN_INFO_WIFI_STATE 4

    int infoType;

    /*Link Information*/
#define WLAN_LINK_OFF       0
#define WLAN_LINK_ON        1
    int linkInfo;

    /*Wifi driver current state*/
#define WLAN_NULL_STATE		0x00000000
#define	WLAN_ASOC_STATE		0x00000001		// Under Linked state...
#define WLAN_REASOC_STATE	0x00000002
#define	WLAN_SLEEP_STATE	0x00000004
#define	WLAN_STATION_STATE	0x00000008
#define	WLAN_AP_STATE	    0x00000010
#define	WLAN_ADHOC_STATE	0x00000020
#define WLAN_ADHOC_MASTER_STATE 0x00000040
#define WLAN_UNDER_LINKING		0x00000080
#define WLAN_UNDER_CMD			0x00000200
#define WLAN_SITE_MONITOR		0x00000800		//to indicate the station is under site surveying

    unsigned int driverState;

    unsigned char staMacAddr[6+2];	
	unsigned int staMaxRate;
	unsigned int staCount;
    
	/*AP Information*/
    unsigned char apMacAddr[6+2];
    int channelId;
    unsigned char ssidName[32];
	unsigned int avgQuant;		//Percent : 0~100
	int avgRSSI;		//RSSI

    /*Scan Information*/
    int apCnt;
    struct {
		unsigned char name[16];
        unsigned char apMacAddr[6+2];
        int channelId;
        unsigned char ssidName[32];
        
#define WLAN_SCAN_OPMODE_INFRA      0        
#define WLAN_SCAN_OPMODE_ADHOC      1
#define WLAN_SCAN_OPMODE_UNKNOW     2
        int operationMode;

#define WLAN_SCAN_SECURITY_OFF      0
#define WLAN_SCAN_SECURITY_ON       1
        int securityOn;

		unsigned char rfQualityQuant;	//Percent : 0~100
		signed char  rfQualityRSSI;	//RSSI
        unsigned char reserved[2];
        int   bitrate;
        int   securityMode;	    /*Sec. Mode*/	
    } apList[64 /*MAX_LEN_OF_BSS_TABLE*/];    
};


typedef void (*input_fn)(void *arg, void *packet, int len);
#endif


struct net_device
{
	char				name[IFNAMSIZ];
	unsigned long		state;
	unsigned short	    flags;  /* interface flags (a la BSD)   */
	unsigned short	    type;	/* interface hardware type	*/
	void				*priv;  /* pointer to private data      */
	unsigned char		dev_addr[MAX_ADDR_LEN]; /* hw address   */
	unsigned long		last_rx;	/* Time of last Rx	*/
    
	int				(*open)(struct net_device *dev);
	int				(*stop)(struct net_device *dev);
	int				(*hard_start_xmit) (struct sk_buff *skb, struct net_device *dev);
	int				(*set_mac_address) (struct net_device *dev, void *p);
	int				(*do_ioctl)(struct net_device *dev, struct ifreq *ifr, int cmd);
	struct net_device_stats*	(*get_stats)(struct net_device *dev);

	struct iw_statistics*	(*get_wireless_stats)(struct net_device *dev);    
    int                     (*wireless_control)(struct net_device *dev, struct net_device_config *netDeviceConfig);
    int                     (*wireless_information)(struct net_device *dev, struct net_device_info *netDeviceInfo);

	int watchdog_timeo;
	struct iw_handler_def*  wireless_handlers;  
    
#ifdef LWIP_INTERFACE
    void           *rxNetif;
    input_fn       rxCallBack;
#endif
};

#if 0
/* interface flags (netdevice->flags). */
#define IFF_UP          0x1             /* interface is up              */
#define IFF_BROADCAST   0x2             /* broadcast address valid      */
#define IFF_DEBUG       0x4             /* turn on debugging            */
#define IFF_LOOPBACK    0x8             /* is a loopback net            */
#define IFF_POINTOPOINT 0x10            /* interface is has p-p link    */
#define IFF_NOTRAILERS  0x20            /* avoid use of trailers        */
#define IFF_RUNNING     0x40            /* resources allocated          */
#define IFF_NOARP       0x80            /* no ARP protocol              */
#define IFF_PROMISC     0x100           /* receive all packets          */
#define IFF_ALLMULTI    0x200           /* receive all multicast packets*/
#define IFF_MASTER      0x400           /* master of a load balancer    */
#define IFF_SLAVE       0x800           /* slave of a load balancer     */
#define IFF_MULTICAST   0x1000          /* Supports multicast           */

#define IFF_VOLATILE    (IFF_LOOPBACK|IFF_POINTOPOINT|IFF_BROADCAST|IFF_MASTER|IFF_SLAVE|IFF_RUNNING)
#endif

#define IFF_PORTSEL     0x2000          /* can set media type           */
#define IFF_AUTOMEDIA   0x4000          /* auto media select active     */
#define IFF_DYNAMIC     0x8000          /* dialup device with changing addresses*/

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


struct net_device *smNetAlloc(int sizeof_priv);
void* smNetdev_priv(struct net_device *dev);
void smNetFree(struct net_device*);
int smNetOpen(struct net_device *dev, input_fn input, void *arg);
int smNetClose(struct net_device *dev);
void smNetTransmit(struct net_device *dev, struct sk_buff *skb);
void smNetReceive(struct sk_buff *skb);
int smNetIOCtrl(struct net_device *dev, void* ptr, int cmd);
int smNetCtrl(struct net_device *dev, struct net_device_config *netDeviceConfig);
int smNetInfo(struct net_device *dev, struct net_device_info *netDeviceInfo);
struct net_device* smNetGetDevice(void);
#endif
