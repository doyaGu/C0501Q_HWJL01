#ifndef __OSDEP_SERVICE_H__
#define __OSDEP_SERVICE_H__
#include <autoconf.h>
#include <basic_types.h>

#define _FAIL		0
#define _SUCCESS	1

#undef _TRUE
#define _TRUE		1

#undef _FALSE
#define _FALSE		0

#ifdef PLATFORM_FREEBSD
#include "osdep_service_bsd.h"
#endif

#ifdef PLATFORM_LINUX
#include "osdep_service_linux.h"
#endif

#ifdef PLATFORM_OS_XP
#include "osdep_service_xp.h"
#endif

#ifdef PLATFORM_OS_CE
#include "osdep_service_ce.h"
#endif

#if 1//def PLATFORM_FREERTOS // Irene Lin
#if 0 // Irene Lin
#include "freertos/osdep_service_freertos.h"
#endif
#include <linux/timer.h> // Irene Lin
#include <freertos/wireless.h>  // Irene Lin
#include <freertos/ite_skbuf.h>
#include <freertos/ite_ndis_ameba.h>
//#include <freertos/netdevice_freertos.h>
//#include <freertos/task_struct_freertos.h>
#include <linux/kthread.h>
#endif


#ifdef PLATFORM_ECOS
#include <ecos/osdep_service_ecos.h>
#include <include/skbuff.h>
#include <include/timer.h>
#include <include/wireless.h>
#include <include/wrapper.h>
#endif

#define RTW_MAX_DELAY			0xFFFFFFFF
#define RTW_WAIT_FOREVER		0xFFFFFFFF

#ifdef PLATFORM_FREERTOS
struct timer_list {
	_timerHandle 	timer_hdl;
	unsigned long	data;
	void (*function)(void *);
};
#endif

#ifndef BIT
	#define BIT(x)	( 1 << (x))
#endif

#ifndef BIT0
#define BIT0		0x0001
#define BIT1		0x0002
#define BIT2		0x0004
#define BIT3		0x0008
#define BIT4		0x0010
#define BIT5		0x0020
#define BIT6		0x0040
#define BIT7		0x0080
#define BIT8		0x0100
#define BIT9		0x0200
#define BIT10	0x0400
#define BIT11	0x0800
#define BIT12	0x1000
#define BIT13	0x2000
#define BIT14	0x4000
#define BIT15	0x8000
#define BIT16	0x00010000
#define BIT17	0x00020000
#define BIT18	0x00040000
#define BIT19	0x00080000
#define BIT20	0x00100000
#define BIT21	0x00200000
#define BIT22	0x00400000
#define BIT23	0x00800000
#define BIT24	0x01000000
#define BIT25	0x02000000
#define BIT26	0x04000000
#define BIT27	0x08000000
#define BIT28	0x10000000
#define BIT29	0x20000000
#define BIT30	0x40000000
#define BIT31	0x80000000
#endif

typedef thread_return (*thread_func_t)(thread_context context);
typedef void (*TIMER_FUN)(void *context);
extern int RTW_STATUS_CODE(int error_code);

#ifdef CONFIG_MEM_MONITOR
//----- ------------------------------------------------------------------
// Memory Monitor
//----- ------------------------------------------------------------------
#define MEM_MONITOR_SIMPLE		0x1
#define MEM_MONITOR_LEAK		0x2

#define MEM_MONITOR_FLAG_WIFI_DRV	0x1
#define MEM_MONITOR_FLAG_WPAS		0x2
#if CONFIG_MEM_MONITOR & MEM_MONITOR_LEAK
struct mem_entry {
	struct list_head	list;
	int			size;
	void			*ptr;
};
#endif

void init_mem_monitor(_list *pmem_table, int *used_num);
void deinit_mem_monitor(_list *pmem_table, int *used_num);
void add_mem_usage(_list *pmem_table, void *ptr, int size, int *used_num, int flag);
void del_mem_usage(_list *pmem_table, void *ptr, int *used_num, int flag);
int get_mem_usage(_list *pmem_table);
#endif

/*********************************** OSDEP API *****************************************/
u8*	_rtw_vmalloc(u32 sz);
u8*	_rtw_zvmalloc(u32 sz);
void	_rtw_vmfree(u8 *pbuf, u32 sz);
u8*	_rtw_zmalloc(u32 sz);
u8*	_rtw_malloc(u32 sz);
void	_rtw_mfree(u8 *pbuf, u32 sz);

#ifdef CONFIG_MEM_MONITOR
u8*	rtw_vmalloc(u32 sz);
u8*	rtw_zvmalloc(u32 sz);
void	rtw_vmfree(u8 *pbuf, u32 sz);
u8*	rtw_zmalloc(u32 sz);
u8*	rtw_malloc(u32 sz);
void	rtw_mfree(u8 *pbuf, u32 sz);
#else
#define rtw_malloc			_rtw_malloc
#define rtw_mfree		_rtw_mfree
#define rtw_zmalloc			_rtw_zmalloc
#ifdef CONFIG_USE_VMALLOC
#define rtw_vmalloc			_rtw_vmalloc
#define rtw_zvmalloc			_rtw_zvmalloc
#define rtw_vmfree	_rtw_vmfree
#else /* CONFIG_USE_VMALLOC */
#define rtw_vmalloc		_rtw_malloc
#define rtw_zvmalloc			_rtw_zmalloc
#define rtw_vmfree		_rtw_mfree
#endif /* CONFIG_USE_VMALLOC */
#endif /*CONFIG_MEM_MONITOR*/

extern void	rtw_memcpy(void* dec, void* sour, u32 sz);
extern int	rtw_memcmp(void *dst, void *src, u32 sz);
extern void	rtw_memset(void *pbuf, int c, u32 sz);
extern void	rtw_init_listhead(_list *list);
extern u32	rtw_is_list_empty(_list *phead);
extern void	rtw_list_insert_head(_list *plist, _list *phead);
extern void	rtw_list_insert_tail(_list *plist, _list *phead);
extern void	rtw_list_delete(_list *plist);
extern void	rtw_init_sema(_sema *sema, int init_val);
extern void	rtw_free_sema(_sema	*sema);
extern void	rtw_up_sema(_sema	*sema);
extern u32	rtw_down_sema(_sema *sema);
extern void	rtw_mutex_init(_mutex *pmutex);
extern void	rtw_mutex_free(_mutex *pmutex);
extern void rtw_enter_critical(_lock *plock, _irqL *pirqL);
extern void rtw_exit_critical(_lock *plock, _irqL *pirqL);
extern void rtw_enter_critical_bh(_lock *plock, _irqL *pirqL);
extern void rtw_exit_critical_bh(_lock *plock, _irqL *pirqL);
extern int rtw_enter_critical_mutex(_mutex *pmutex, _irqL *pirqL);
extern void rtw_exit_critical_mutex(_mutex *pmutex, _irqL *pirqL);
extern void	rtw_spinlock_init(_lock *plock);
extern void	rtw_spinlock_free(_lock *plock);
extern void	rtw_spin_lock(_lock	*plock);
extern void	rtw_spin_unlock(_lock	*plock);
extern void rtw_spinlock_irqsave(_lock *plock, _irqL *irqL);
extern void rtw_spinunlock_irqsave(_lock *plock, _irqL *irqL);
extern void	rtw_init_queue(_queue	*pqueue);
extern u32	rtw_queue_empty(_queue	*pqueue);
extern void	rtw_deinit_queue(_queue *pqueue);
extern u32	rtw_end_of_queue_search(_list *queue, _list *pelement);
extern u32	rtw_get_current_time(void);
extern u32	rtw_systime_to_ms(u32 systime);
extern u32	rtw_ms_to_systime(u32 ms);
extern s32	rtw_get_passing_time_ms(u32 start);
extern s32	rtw_get_time_interval_ms(u32 start, u32 end);
extern void	rtw_msleep_os(int ms);
extern void	rtw_usleep_os(int us);
extern void rtw_mdelay_os(int ms);
extern void rtw_udelay_os(int us);
extern void rtw_yield_os(void);
extern void rtw_init_timer(_timer *ptimer, void *adapter, TIMER_FUN pfunc,void* cntx, const char *name);
extern void rtw_set_timer(_timer *ptimer,u32 delay_time);
extern u8 rtw_cancel_timer(_timer * ptimer);

extern void ATOMIC_SET(ATOMIC_T *v, int i);
extern int ATOMIC_READ(ATOMIC_T *v);
extern void ATOMIC_ADD(ATOMIC_T *v, int i);
extern void ATOMIC_SUB(ATOMIC_T *v, int i);
extern void ATOMIC_INC(ATOMIC_T *v);
extern void ATOMIC_DEC(ATOMIC_T *v);
extern int ATOMIC_ADD_RETURN(ATOMIC_T *v, int i);
extern int ATOMIC_SUB_RETURN(ATOMIC_T *v, int i);
extern int ATOMIC_INC_RETURN(ATOMIC_T *v);
extern int ATOMIC_DEC_RETURN(ATOMIC_T *v);

extern int rtw_netif_queue_stopped(struct net_device *pnetdev);
extern void rtw_netif_wake_queue(struct net_device *pnetdev);
extern void rtw_netif_start_queue(struct net_device *pnetdev);
extern void rtw_netif_stop_queue(struct net_device *pnetdev);
extern void rtw_flush_signals_thread(void);
extern void rtw_thread_enter(char *name);
#ifndef PLATFORM_LINUX
extern void rtw_thread_exit(void);
#endif
/*********************************** Thread related *****************************************/
int	rtw_create_task(struct task_struct *task, const char *name, u32  stack_size, u32 priority, thread_func_t func, void *thctx);
void rtw_delete_task(struct task_struct * task);
void	rtw_wakeup_task(struct task_struct *task);

/*********************************** SKB related *****************************************/
struct sk_buff *_rtw_skb_alloc(u32 sz);
void _rtw_skb_free(struct sk_buff *skb);
struct sk_buff *_rtw_skb_copy(const struct sk_buff *skb);
#if 1 // Irene Lin
extern struct sk_buff *_rtw_skb_clone(struct sk_buff *skb);
extern int _rtw_netif_rx(_nic_hdl ndev, struct sk_buff *skb);
#else
struct sk_buff *_rtw_skb_clone(struct sk_buff *skb);
int _rtw_netif_rx(_nic_hdl ndev, struct sk_buff *skb);
#endif
void _rtw_skb_queue_purge(struct sk_buff_head *list);

_timerHandle rtw_timerCreate( const signed char *pcTimerName, 
							  osdepTickType xTimerPeriodInTicks, 
							  u32 uxAutoReload, 
							  void * pvTimerID, 
							  TIMER_FUN pxCallbackFunction );

struct sk_buff *rtw_skb_alloc(u32 sz);
void rtw_skb_free(struct sk_buff *skb);

#define rtw_skb_copy(skb)	_rtw_skb_copy((skb))
#define rtw_skb_clone(skb)	_rtw_skb_clone((skb))
#define rtw_netif_rx(ndev, skb) _rtw_netif_rx(ndev, skb)
#define rtw_skb_queue_purge(sk_buff_head) _rtw_skb_queue_purge(sk_buff_head)
#ifdef CONFIG_USB_HCI
void *_rtw_usb_buffer_alloc(struct usb_device *dev, size_t size, dma_addr_t *dma);
void _rtw_usb_buffer_free(struct usb_device *dev, size_t size, void *addr, dma_addr_t dma);
#define rtw_usb_buffer_alloc(dev, size, dma) _rtw_usb_buffer_alloc((dev), (size), (dma))
#define rtw_usb_buffer_free(dev, size, addr, dma) _rtw_usb_buffer_free((dev), (size), (addr), (dma))
#endif /* CONFIG_USB_HCI */

void _rtw_os_indicate_connect(struct net_device *pnetdev);
void _rtw_os_indicate_disconnect(struct net_device *pnetdev);
void _rtw_indicate_sta_assoc_event(struct net_device *pnetdev, u8 *mac);
void _rtw_indicate_sta_disassoc_event(struct net_device *pnetdev, u8 *mac);
void _rtw_os_indicate_scan_result(struct net_device *pnetdev, int status, char *result);
void _rtw_os_send_simple_config_ack(struct net_device *pnetdev, u32 dest_ip);
void _rtw_os_indicate_wifi_info(struct net_device *pnetdev, char *info);
void _rtw_os_indicate_wowlan_status(struct net_device *pnetdev, unsigned char ID, char *status);
#define rtw_os_indicate_connect(ndev) _rtw_os_indicate_connect(ndev)
#define rtw_os_indicate_disconnect(ndev) _rtw_os_indicate_disconnect(ndev)
#define rtw_indicate_sta_disassoc_event(ndev, mac)  _rtw_indicate_sta_disassoc_event((ndev), (mac))
#define rtw_indicate_sta_assoc_event(ndev, mac)  _rtw_indicate_sta_assoc_event((ndev), (mac))
#define rtw_os_indicate_scan_result(ndev, status, result)  _rtw_os_indicate_scan_result((ndev), (status), (result))
#define rtw_os_send_simple_config_ack(ndev, target_ip) _rtw_os_send_simple_config_ack((ndev), (target_ip)) 
#define rtw_os_indicate_wifi_info(ndev, info)			_rtw_os_indicate_wifi_info(ndev, info)
#define rtw_os_indicate_wowlan_status(ndev, id, status)	_rtw_os_indicate_wowlan_status(ndev, id, status)

//File operation APIs, just for linux now
extern int rtw_is_file_readable(char *path);
extern int rtw_retrive_from_file(char *path, u8* buf, u32 sz);
extern int rtw_store_to_file(char *path, u8* buf, u32 sz);
extern void rtw_show_stack(unsigned long *sp);

struct net_device *rtw_alloc_etherdev_with_old_priv(int sizeof_priv, void *old_priv);
struct net_device * rtw_alloc_etherdev(int sizeof_priv);
void rtw_free_netdev(struct net_device * netdev);

u32 rtw_down_timeout_sema(_sema *sema, u32 timeout);
u32 rtw_timerDelete( _timerHandle xTimer, osdepTickType xBlockTime );
u32 rtw_timerIsTimerActive( _timerHandle xTimer );
u32 rtw_timerStop( _timerHandle xTimer, osdepTickType xBlockTime );
u32 rtw_timerChangePeriod( _timerHandle xTimer,   osdepTickType xNewPeriod, osdepTickType xBlockTime );
int rtw_dev_remove(struct net_device *pnetdev);

u32 rtw_getFreeHeapSize(void);

struct osdep_service_ops {
// 0
//memory ops
	u8* (*rtw_vmalloc)(u32 sz);
	u8* (*rtw_zvmalloc)(u32 sz);
	void (*rtw_vmfree)(u8 *pbuf, u32 sz);
	u8* (*rtw_malloc)(u32 sz);
	u8* (*rtw_zmalloc)(u32 sz);
	void (*rtw_mfree)(u8 *pbuf, u32 sz);
	void (*rtw_memcpy)(void* dst, void* src, u32 sz);
	int (*rtw_memcmp)(void *dst, void *src, u32 sz);
	void (*rtw_memset)(void *pbuf, int c, u32 sz);
// 9	
//list ops
	void (*rtw_init_listhead)(_list *list);
	u32 (*rtw_is_list_empty)(_list *phead);
	void (*rtw_list_insert_head)(_list *plist, _list *phead);
	void (*rtw_list_insert_tail)(_list *plist, _list *phead);
	void (*rtw_list_delete)(_list *plist);
// 14	
//sema ops
	void (*rtw_init_sema)(_sema *sema, int init_val);
	void (*rtw_free_sema)(_sema *sema);
	void (*rtw_up_sema)(_sema *sema);
	u32 (*rtw_down_sema)(_sema *sema);
	u32 (*rtw_down_timeout_sema)(_sema *sema, u32 timeout);
//19	
//mutex ops
	void (*rtw_mutex_init)(_mutex *pmutex);
	void (*rtw_mutex_free)(_mutex *pmutex);
	void (*rtw_mutex_get)(_mutex *pmutex);
	void (*rtw_mutex_put)(_mutex *pmutex);
//23	
//spinlock ops
	void (*rtw_enter_critical)(_lock *plock, _irqL *pirqL);
	void (*rtw_exit_critical)(_lock *plock, _irqL *pirqL);
	void (*rtw_enter_critical_bh)(_lock *plock, _irqL *pirqL);
	void (*rtw_exit_critical_bh)(_lock *plock, _irqL *pirqL);
	int (*rtw_enter_critical_mutex)(_mutex *pmutex, _irqL *pirqL);
	void (*rtw_exit_critical_mutex)(_mutex *pmutex, _irqL *pirqL);
	void (*rtw_spinlock_init)(_lock *plock);
	void (*rtw_spinlock_free)(_lock *plock);
	void (*rtw_spin_lock)(_lock *plock);
	void (*rtw_spin_unlock)(_lock *plock);
	void (*rtw_spinlock_irqsave)(_lock *plock, _irqL *irqL);
	void (*rtw_spinunlock_irqsave)(_lock *plock, _irqL *irqL);
//35
	int (*rtw_init_xqueue)( _xqueue* queue, const char* name, u32 message_size, u32 number_of_messages );
	int (*rtw_push_to_xqueue)( _xqueue* queue, void* message, u32 timeout_ms );
	int (*rtw_pop_from_xqueue)( _xqueue* queue, void* message, u32 timeout_ms );
	int (*rtw_deinit_xqueue)( _xqueue* queue );
//39	
//sys time
	u32	(*rtw_get_current_time)(void);
	u32 (*rtw_systime_to_ms)(u32 systime);
	u32 (*rtw_systime_to_sec)(u32 systime);
	u32 (*rtw_ms_to_systime)(u32 ms);
	u32	(*rtw_sec_to_systime)(u32 sec);
	void (*rtw_msleep_os)(int ms);
	void (*rtw_usleep_os)(int us);
	void (*rtw_mdelay_os)(int ms);
	void (*rtw_udelay_os)(int us);
	void (*rtw_yield_os)(void);
// 49	
//timer ops
	void (*rtw_init_timer)(_timer *ptimer, void *adapter, TIMER_FUN pfunc,void* cntx, const char *name);
	void (*rtw_set_timer)(_timer *ptimer,u32 delay_time);
	u8 (*rtw_cancel_timer)(_timer *ptimer);
	void (*rtw_del_timer)(_timer *ptimer);
// 53	
//ATOMIC ops
	void (*ATOMIC_SET)(ATOMIC_T *v, int i);
	int (*ATOMIC_READ)(ATOMIC_T *v);
	void (*ATOMIC_ADD)(ATOMIC_T *v, int i);
	void (*ATOMIC_SUB)(ATOMIC_T *v, int i);
	void (*ATOMIC_INC)(ATOMIC_T *v);
	void (*ATOMIC_DEC)(ATOMIC_T *v);
	int (*ATOMIC_ADD_RETURN)(ATOMIC_T *v, int i);
	int (*ATOMIC_SUB_RETURN)(ATOMIC_T *v, int i);
	int (*ATOMIC_INC_RETURN)(ATOMIC_T *v);
	int (*ATOMIC_DEC_RETURN)(ATOMIC_T *v);
// 63
	u64 (*rtw_modular64)(u64 x, u64 y);
	int (*rtw_get_random_bytes)(void* dst, u32 size);
	u32 (*rtw_getFreeHeapSize)(void);
// 66
//task ops
	int (*rtw_create_task)(struct task_struct *task, const char *name, u32 stack_size, u32 priority, thread_func_t func, void *thctx);
	void (*rtw_delete_task)(struct task_struct *task);
	void (*rtw_wakeup_task)(struct task_struct *task);
	
#if 0	//TODO
	void (*rtw_init_delayed_work)(struct delayed_work *dwork, work_func_t func, const char *name);
	void (*rtw_deinit_delayed_work)(struct delayed_work *dwork);
	int (*rtw_queue_delayed_work)(struct workqueue_struct *wq, struct delayed_work *dwork, unsigned long delay, void* context);
	BOOLEAN (*rtw_cancel_delayed_work)(struct delayed_work *dwork);
#endif	
//69
	void (*rtw_thread_enter)(char *name);
	void (*rtw_thread_exit)(void);
	void (*rtw_flush_signals_thread)(void); 
// 72
	_timerHandle (*rtw_timerCreate)( const signed char *pcTimerName, 
							  osdepTickType xTimerPeriodInTicks, 
							  u32 uxAutoReload, 
							  void * pvTimerID, 
							  TIMER_FUN pxCallbackFunction );
	u32 (*rtw_timerDelete)( _timerHandle xTimer, 
							   osdepTickType xBlockTime );
	u32 (*rtw_timerIsTimerActive)( _timerHandle xTimer );
	u32 (*rtw_timerStop)( _timerHandle xTimer, 
							   osdepTickType xBlockTime );
	u32 (*rtw_timerChangePeriod)( _timerHandle xTimer, 
							   osdepTickType xNewPeriod, 
							   osdepTickType xBlockTime );
	struct sk_buff *(*rtw_skb_alloc)(u32 sz);
	void (*rtw_skb_free)(struct sk_buff *skb);
};

#endif
