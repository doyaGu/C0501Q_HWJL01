#ifndef	__SM_PORT_H__
#define	__SM_PORT_H__

#include "ite/ith.h"
#include "ite/itp.h"
#include "linux/list.h"

/*=========================================================================*/
/*Compiler flags*/

/*enhanced feature*/
#define WMM_SUPPORT			1
#define WSC_SUPPORT			1

// Irene 2009/0312 rx_task
#define RX_TASK                             1
// Irene 2009/0318 lwip_interface
#define LWIP_INTERFACE                      1

#ifndef inline
#define inline                      __inline
#endif

/*=========================================================================*/
/*Type defines*/
typedef int                 pid_t;


/*=========================================================================*/
/* multiple definition issue */


/*=========================================================================*/
/* Memory related*/
#define GFP_DMA         0
#define GFP_ATOMIC    	1
#define GFP_KERNEL    	2

#define kmalloc(_size, _flag)	(malloc(_size))
#define kfree(_p)				(free(_p))

#define wmb()
//no need

int copy_from_user(void *_des,void *_src,int _len);

int copy_to_user(void *_des,void *_src,int _len);


//#define copy_from_user(_des, _src, _len)        memcpy((void *)(_des), (void *)(_src), (int)(_len))
//#define copy_to_user(_des, _src, _len)          memcpy((void *)(_des), (void *)(_src), (int)(_len))
/*=========================================================================*/
/* Output related*/
#define printk              printf
/*=========================================================================*/
/* OS related */
#include "ite_os.h"

//event
#if 1
struct eventsockaddr {
  char sa_len;
  char sa_family;
  char sa_data[14];
};

void netlinkInitial(void);

void wireless_send_event(void* dev,unsigned int cmd,unsigned int lens,char* buff,struct eventsockaddr ap_addr,struct eventsockaddr addr);


#else

#define wireless_send_event(x,y,z,w) 
#endif

/*=========================================================================*/
/* Timer related */
#include "ite_timer.h"

#define udelay  	            ithDelay
#define mdelay(x)	            ithDelay(x*1000)
#define timer_list		        ITE_TIMER_S

//#define HZ								    1000            /*1 clock = 1 ms.*/

#define init_timer						iteTimerInit
#define add_timer						  iteTimerAdd
#define del_timer_sync				iteTimerDel
#define timer_pending         iteTimerPending

#define jiffies							  ITE_GET_TIME()
#define jiffies_to_msecs(times)  times*1000
#define msecs_to_jiffies(time_ms)  jiffies+time_ms  
/*=========================================================================*/
/* Packet Buffer related */
#include "ite_skbuf.h"

#define BUG()       //do { LOG_ERROR "impossible path!!! %s:%s:%s \n", __FILE__, __FUNCTION__,__LINE__ LOG_END  while(1);   } while(0)

#if 0
#define dev_alloc_skb(_size)            alloc_skb(_size)
#define __dev_alloc_skb(_size,_type)    dev_alloc_skb((_size))
#define dev_kfree_skb(_skb)             free_skb((_skb))
#define dev_kfree_skb_any               dev_kfree_skb
#define skb_reserve(_skb,_len)	        ((_skb)->data += (_len))
#define skb_put(_skb,_len)              skb_put((_skb),(_len))
//#define skb_clone(_skb,_gfp_mask)
#define kfree_skb(_skb)                 free_skb((_skb))
#else
#define dev_kfree_skb_any               dev_kfree_skb
#endif

/*=========================================================================*/
/* Error Code*/

/*=========================================================================*/
/* USB related */
#include "usb/ite_usb.h"
#if 0
struct usb_anchor {
        struct list_head urb_list;
        wait_queue_head_t wait;
        spinlock_t lock;
         unsigned int poisoned:1;
};

static inline void init_usb_anchor(struct usb_anchor *anchor)
{
        INIT_LIST_HEAD(&anchor->urb_list);
        init_waitqueue_head(&anchor->wait);
        spin_lock_init(&anchor->lock);
}
#endif

/*=========================================================================*/
/* Network related*/
//Irene TBD : can replaced by lwip layer
#define ETH_ALEN        6
#define ARPHRD_ETHER 	  1		/* Ethernet 10Mbps		*/

#define IFNAMSIZ        16
#define MAX_ADDR_LEN    6

//#define sockaddr    socketaddr

typedef unsigned short	sa_family_t;
#if 0
struct sockaddr {
	sa_family_t	sa_family;	/* address family, AF_xxx	*/
	char		sa_data[14];	/* 14 bytes of protocol address	*/
};
#endif
/*=========================================================================*/
/* NetIF/NDIS related */
#include "ite_ndis.h"
extern struct net_device *smNetDevice;
extern struct net_device *smEthDevice;


#define netdev_priv(_dev)              smNetdev_priv((_dev)) //James Add

#define netif_rx(_skb)                  smNetReceive((_skb))

#define alloc_netdevice                 smNetAlloc
#define free_netdevice                  smNetFree
#define eth_type_trans(_skb, _dev)      (0x00) /*No need in lwip*/

#define alloc_etherdev smEthAlloc //need implement eth.c
#define ether_setup(_dev)
#define dev_alloc_name(_dev,x)  sprintf(_dev->name, "mgnt.wlan0")

static inline int register_netdev(struct net_device *_dev)
{
	if(memcmp(_dev->name,"wlan0",5) == 0)
	{
        printf("register_netdev wlan0\n");
		smNetDevice = _dev;
	}
	else
	{
        printf("register_netdev not wlan0\n");
		smEthDevice = _dev;
	}
    return 0;
}

static inline int unregister_netdev(struct net_device *_dev)
{
	if(memcmp(_dev->name,"wlan0",5) == 0)
	{
		smNetDevice = NULL;
	}
	else
	{
	    smEthDevice = NULL;
	}

    return 0;
}


//#define register_netdev(_dev)           (smNetDevice = _dev)  //Irene test
//#define unregister_netdev(_dev)         (smNetDevice = NULL)
/*=========================================================================*/
/* Wireless_Ext/IOCTL related */
#include "ite_ioctl.h"

int rtw_ioctl_smCtrl(struct net_device *dev, struct net_device_config *netDeviceConfig);
int rtw_ioctl_smInfo(struct net_device *dev, struct net_device_info *netDeviceInfo);


/*=========================================================================*/
/* Common API */
#include "ite_util.h"
/*=========================================================================*/
/* OpenSSL-replaced API */


//#define INT_MAX		((int)(~0U>>1))
/*=========================================================================*/
/* SM Port */

//James Add : maybe not used
#include "workqueue.h"

#define work_struct job_t
#define container_of(ptr, type, member) ({                  \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type,member) );})
//#define offsetof(TYPE, MEMBER) ((unsigned int) &((TYPE *)0)->MEMBER)


#endif
