#define __OSDEP_SERVICE_C__
#include <linux/os.h> // Irene Lin
#include <linux/timer.h> // Irene Lin
#include <linux/mmc/errorno.h> // Irene Lin
#include <freertos/ite_skbuf.h> // Irene Lin
#include "autoconf.h"
#include "basic_types.h"
#include "osdep_service.h"
#include "rtw_debug.h"

#ifdef DBG_MEMORY_LEAK
#ifdef PLATFORM_LINUX
atomic_t _malloc_cnt = ATOMIC_INIT(0);
atomic_t _malloc_size = ATOMIC_INIT(0);
#endif
#endif /* DBG_MEMORY_LEAK */

inline u8* _linux_vmalloc(u32 sz)
{
	u8 	*pbuf;
#ifdef PLATFORM_LINUX	
	pbuf = vmalloc(sz);
#endif	
	
#ifdef DBG_MEMORY_LEAK
#ifdef PLATFORM_LINUX
	if ( pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif
#endif /* DBG_MEMORY_LEAK */

	return pbuf;	
}

inline u8* _linux_zvmalloc(u32 sz)
{
	u8 	*pbuf;

#ifdef PLATFORM_LINUX
	pbuf = _linux_vmalloc(sz);
	if (pbuf != NULL)
		memset(pbuf, 0, sz);
#endif	

	return pbuf;	
}

inline void _linux_vmfree(u8 *pbuf, u32 sz)
{
#ifdef	PLATFORM_LINUX
	vfree(pbuf);
#endif	

#ifdef DBG_MEMORY_LEAK
#ifdef PLATFORM_LINUX
	atomic_dec(&_malloc_cnt);
	atomic_sub(sz, &_malloc_size);
#endif
#endif /* DBG_MEMORY_LEAK */
}

u8* _linux_malloc(u32 sz)
{

	u8 	*pbuf=NULL;

#ifdef PLATFORM_LINUX
#ifdef RTK_DMP_PLATFORM
	if(sz > 0x4000)
		pbuf = (u8 *)dvr_malloc(sz);
	else
#endif		
		pbuf = kmalloc(sz,in_interrupt() ? GFP_ATOMIC : GFP_KERNEL); 		

#endif	

#ifdef DBG_MEMORY_LEAK
#ifdef PLATFORM_LINUX
	if ( pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif
#endif /* DBG_MEMORY_LEAK */

	return pbuf;	
	
}


u8* _linux_zmalloc(u32 sz)
{

	u8 	*pbuf = _rtw_malloc(sz);

	if (pbuf != NULL) {

#ifdef PLATFORM_LINUX
		memset(pbuf, 0, sz);
#endif	

	}

	return pbuf;	

}

void	_linux_mfree(u8 *pbuf, u32 sz)
{

#ifdef	PLATFORM_LINUX
#ifdef RTK_DMP_PLATFORM
	if(sz > 0x4000)
		dvr_free(pbuf);
	else
#endif
		kfree(pbuf);

#endif	
	
#ifdef DBG_MEMORY_LEAK
#ifdef PLATFORM_LINUX
	atomic_dec(&_malloc_cnt);
	atomic_sub(sz, &_malloc_size);
#endif
#endif /* DBG_MEMORY_LEAK */
	
}

inline struct sk_buff *_linux_skb_alloc(u32 sz)
{
#ifdef PLATFORM_LINUX
	return __dev_alloc_skb(sz, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
#endif /* PLATFORM_LINUX */
}

inline void _linux_skb_free(struct sk_buff *skb)
{
	dev_kfree_skb_any(skb);
}

inline struct sk_buff *_rtw_skb_copy(const struct sk_buff *skb)
{
#ifdef PLATFORM_LINUX
	return skb_copy(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
#endif /* PLATFORM_LINUX */
}

inline struct sk_buff *_rtw_skb_clone(struct sk_buff *skb)
{
#ifdef PLATFORM_LINUX
	return skb_clone(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
#endif /* PLATFORM_LINUX */
}

inline int _rtw_netif_rx(_nic_hdl ndev, struct sk_buff *skb)
{
#ifdef PLATFORM_LINUX
	skb->dev = ndev;
	return netif_rx(skb);
#endif /* PLATFORM_LINUX */
}

void _rtw_skb_queue_purge(struct sk_buff_head *list)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(list)) != NULL)
		rtw_skb_free(skb);
}

#ifdef CONFIG_USB_HCI
inline void *_rtw_usb_buffer_alloc(struct usb_device *dev, size_t size, dma_addr_t *dma)
{
#ifdef PLATFORM_LINUX
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	return usb_alloc_coherent(dev, size, (in_interrupt() ? GFP_ATOMIC : GFP_KERNEL), dma);
#else
	return usb_buffer_alloc(dev, size, (in_interrupt() ? GFP_ATOMIC : GFP_KERNEL), dma);
#endif
#endif /* PLATFORM_LINUX */
}

inline void _rtw_usb_buffer_free(struct usb_device *dev, size_t size, void *addr, dma_addr_t dma)
{
#ifdef PLATFORM_LINUX
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	usb_free_coherent(dev, size, addr, dma); 
#else
	usb_buffer_free(dev, size, addr, dma);
#endif
#endif /* PLATFORM_LINUX */
}
#endif /* CONFIG_USB_HCI */


void _linux_memcpy(void* dst, void* src, u32 sz)
{

#if defined (PLATFORM_LINUX)

	memcpy(dst, src, sz);

#endif	

}

int	_linux_memcmp(void *dst, void *src, u32 sz)
{

#if defined (PLATFORM_LINUX)
//under Linux/GNU/GLibc, the return value of memcmp for two same mem. chunk is 0

	if (!(memcmp(dst, src, sz)))
		return _TRUE;
	else
		return _FALSE;
#endif
	
}

void _linux_memset(void *pbuf, int c, u32 sz)
{

#if defined (PLATFORM_LINUX)|| defined (PLATFORM_FREEBSD)

        memset(pbuf, c, sz);

#endif

}

void _linux_init_listhead(_list *list)
{
	INIT_LIST_HEAD(list);
}

/*
For the following list_xxx operations, 
caller must guarantee the atomic context.
Otherwise, there will be racing condition.
*/
u32 _linux_is_list_empty(_list *phead)
{
	if(list_empty(phead))
		return _TRUE;

	return _FALSE;
}

void _linux_list_insert_head(_list *plist, _list *phead)
{
	list_add(plist, phead);
}

void _linux_list_insert_tail(_list *plist, _list *phead)
{
	list_add_tail(plist, phead);
}

/*

Caller must check if the list is empty before calling rtw_list_delete

*/
void _linux_list_delete(_list *plist)
{
	list_del_init(plist);
}

void _linux_init_sema(_sema	*sema, int init_val)
{

#ifdef PLATFORM_LINUX

	//sema_init(sema, init_val);
	sem_init(sema, 0, init_val); // Irene Lin

#endif
	
}

void _linux_free_sema(_sema	*sema)
{
	sem_destroy(sema); // Irene Lin
}

void _linux_up_sema(_sema	*sema)
{

#ifdef PLATFORM_LINUX

	//up(sema);
	sem_post(sema); // Irene Lin

#endif	

}

u32 _linux_down_sema(_sema *sema)
{
	
#if 0//def PLATFORM_LINUX // Irene Lin
	
	if (down_interruptible(sema))
		return _FAIL;
	else
		return _SUCCESS;
#else
	sem_wait(sema);
	return _SUCCESS;
#endif    	

}

void	_linux_mutex_init(_mutex *pmutex)
{

#ifdef PLATFORM_LINUX

#if 1//(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))  // Irene Lin
	mutex_init(pmutex);
#else
	init_MUTEX(pmutex);
#endif

#endif

}

void	_linux_mutex_free(_mutex *pmutex);
void	_linux_mutex_free(_mutex *pmutex)
{
#ifdef PLATFORM_LINUX

#if 1//(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))  // Irene Lin
	mutex_destroy(pmutex);
#else	
#endif

#endif

}

__inline static void _linux_enter_critical(_lock *plock, _irqL *pirqL)
{
	spin_lock_irqsave(plock, *pirqL);
}

__inline static void _linux_exit_critical(_lock *plock, _irqL *pirqL)
{
	spin_unlock_irqrestore(plock, *pirqL);
}

__inline static void _linux_enter_critical_ex(_lock *plock, _irqL *pirqL)
{
	spin_lock_irqsave(plock, *pirqL);
}

__inline static void _linux_exit_critical_ex(_lock *plock, _irqL *pirqL)
{
	spin_unlock_irqrestore(plock, *pirqL);
}

__inline static void _linux_enter_critical_bh(_lock *plock, _irqL *pirqL)
{
	spin_lock_bh(plock);
}

__inline static void _linux_exit_critical_bh(_lock *plock, _irqL *pirqL)
{
	spin_unlock_bh(plock);
}

__inline static int _linux_enter_critical_mutex(_mutex *pmutex, _irqL *pirqL)
{
	int ret = 0;
#if 1 // Irene Lin
	mutex_lock(pmutex);
    return ret;
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	//mutex_lock(pmutex);
	ret = mutex_lock_interruptible(pmutex);
#else
	ret = down_interruptible(pmutex);
#endif
	return ret;
#endif
}


__inline static void _linux_exit_critical_mutex(_mutex *pmutex, _irqL *pirqL)
{
#if 1//(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))   // Irene Lin
		mutex_unlock(pmutex);
#else
		up(pmutex);
#endif
}

void	_linux_spinlock_init(_lock *plock)
{

#ifdef PLATFORM_LINUX

	spin_lock_init(plock);

#endif	
	
}

void	_linux_spinlock_free(_lock *plock)
{
		
}


void	_linux_spin_lock(_lock	*plock)
{

#ifdef PLATFORM_LINUX

	spin_lock(plock);

#endif
	
}

void	_linux_spin_unlock(_lock *plock)
{

#ifdef PLATFORM_LINUX

	spin_unlock(plock);

#endif

}

__inline static void _linux_spinlock_irqsave(_lock *plock, _irqL *pirqL)
{
	spin_lock_irqsave(plock, *pirqL);
}

__inline static void _linux_spinunlock_irqsave(_lock *plock, _irqL *pirqL)
{
	spin_unlock_irqrestore(plock, *pirqL);
}

u32	_linux_get_current_time(void)
{
	
#ifdef PLATFORM_LINUX
	return jiffies;
#endif	

}
inline u32 _linux_systime_to_ms(u32 systime)
{

#ifdef PLATFORM_LINUX
	return systime * 1000 / HZ;
#endif	

}

inline u32 _linux_ms_to_systime(u32 ms)
{

#ifdef PLATFORM_LINUX
	return ms * HZ / 1000;
#endif	

}

void _linux_msleep_os(int ms)
{

#ifdef PLATFORM_LINUX

  	msleep((unsigned int)ms);

#endif	

}
void _linux_usleep_os(int us)
{

#ifdef PLATFORM_LINUX
  	
      // msleep((unsigned int)us);
      if ( 1 < (us/1000) )
                msleep(1);
      else
		msleep( (us/1000) + 1);

#endif	

}

void _linux_mdelay_os(int ms){
	//mdelay(ms);
    msleep(ms); // Irene Lin
}

void _linux_udelay_os(int ms){
	udelay(ms);
}

void _linux_yield_os(void)
{

#ifdef PLATFORM_LINUX
#if !defined(WIN32) // Irene Lin
	//yield();
	sched_yield(); // Irene Lin
#endif
#endif

}

__inline static void _linux_init_timer(_timer *ptimer, void *adapter, TIMER_FUN pfunc,void *cntx, const char *name)
{
	//setup_timer(ptimer, pfunc,(u32)cntx);	
	ptimer->function = (void *)pfunc;
	ptimer->data = (unsigned long)cntx;
	init_timer(ptimer);
}

__inline static void _linux_set_timer(_timer *ptimer,u32 delay_time)
{	
	mod_timer(ptimer , (jiffies+(delay_time*HZ/1000)));	
}

__inline static unsigned char _linux_cancel_timer_ex(_timer *ptimer)
{
	return del_timer_sync(ptimer);
}

__inline static void _linux_del_timer(_timer *ptimer)
{
	del_timer_sync(ptimer); 	
}

#if 0 // Irene Lin
__inline static void _init_workitem(_workitem *pwork, void *pfunc, PVOID cntx)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
	INIT_WORK(pwork, pfunc);
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(2,5,41))
	INIT_WORK(pwork, pfunc,pwork);
#else
	INIT_TQUEUE(pwork, pfunc,pwork);
#endif
}

__inline static void _set_workitem(_workitem *pwork)
{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,5,41))
	schedule_work(pwork);
#else
	schedule_task(pwork);
#endif
}

__inline static void _cancel_workitem_sync(_workitem *pwork)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22))
	cancel_work_sync(pwork);
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(2,5,41))
	flush_scheduled_work();
#else
	flush_scheduled_tasks();
#endif
}
#endif

inline void _linux_ATOMIC_SET(ATOMIC_T *v, int i)
{
	#ifdef PLATFORM_LINUX
	atomic_set(v,i);
	#endif
}

inline int _linux_ATOMIC_READ(ATOMIC_T *v)
{
	#ifdef PLATFORM_LINUX
	return atomic_read(v);
	#endif
}

inline void _linux_ATOMIC_ADD(ATOMIC_T *v, int i)
{
	#ifdef PLATFORM_LINUX
	atomic_add(i,v);
	#endif
}
inline void _linux_ATOMIC_SUB(ATOMIC_T *v, int i)
{
	#ifdef PLATFORM_LINUX
	atomic_sub(i,v);
	#endif
}

inline void _linux_ATOMIC_INC(ATOMIC_T *v)
{
	#ifdef PLATFORM_LINUX
	atomic_inc(v);
	#endif
}

inline void _linux_ATOMIC_DEC(ATOMIC_T *v)
{
	#ifdef PLATFORM_LINUX
	atomic_dec(v);
	#endif
}

inline int _linux_ATOMIC_ADD_RETURN(ATOMIC_T *v, int i)
{
	#ifdef PLATFORM_LINUX
	return atomic_add_return(i,v);
	#endif
}

inline int _linux_ATOMIC_SUB_RETURN(ATOMIC_T *v, int i)
{
	#ifdef PLATFORM_LINUX
	return atomic_sub_return(i,v);
	#endif
}

inline int _linux_ATOMIC_INC_RETURN(ATOMIC_T *v)
{
	#ifdef PLATFORM_LINUX
	return atomic_inc_return(v);
	#endif
}

inline int _linux_ATOMIC_DEC_RETURN(ATOMIC_T *v)
{
	#ifdef PLATFORM_LINUX
	return atomic_dec_return(v);
	#endif
}
#if 0
static u64 _linux_modular64(u64 n, u64 base)
{
	unsigned int __base = (base);
	unsigned int __rem;

	if (((n) >> 32) == 0) {
		__rem = (unsigned int)(n) % __base;
		(n) = (unsigned int)(n) / __base;
	}
	else
		__rem = __div64_32(&(n), __base);
	
	return __rem;
}
#endif
static __inline void _linux_thread_enter(char *name)
{

#if 0//def PLATFORM_LINUX  // Irene Lin
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0))
	daemonize("%s", name);
	#endif
	allow_signal(SIGTERM);
#endif

}

#if 0
static int _linux_create_task(struct task_struct *ptask, const char *name,
	u32  stack_size, u32 priority, thread_func_t func, void *thctx){

	ptask = kthread_run(func, thctx, name);
	if(IS_ERR(ptask))
	{	
		DBG_871X("%s() thread start Failed!\n", __FUNCTION__);
		return _FAIL;
	}

	return _SUCCESS;
}
#endif

void _linux_delete_task(struct task_struct *task)
{
	kthread_stop(task);
}

#if 0//def PLATFORM_LINUX   // Irene Lin
/*
* Open a file with the specific @param path, @param flag, @param mode
* @param fpp the pointer of struct file pointer to get struct file pointer while file opening is success
* @param path the path of the file to open
* @param flag file operation flags, please refer to linux document
* @param mode please refer to linux document
* @return Linux specific error code
*/
static int openFile(struct file **fpp, char *path, int flag, int mode) 
{ 
	struct file *fp; 
 
	fp=filp_open(path, flag, mode); 
	if(IS_ERR(fp)) {
		*fpp=NULL;
		return PTR_ERR(fp);
	}
	else {
		*fpp=fp; 
		return 0;
	}	
}

/*
* Close the file with the specific @param fp
* @param fp the pointer of struct file to close
* @return always 0
*/
static int closeFile(struct file *fp) 
{ 
	filp_close(fp,NULL);
	return 0; 
}

static int readFile(struct file *fp,char *buf,int len) 
{ 
	int rlen=0, sum=0;
	
	if (!fp->f_op || !fp->f_op->read) 
		return -EPERM;

	while(sum<len) {
		rlen=fp->f_op->read(fp,buf+sum,len-sum, &fp->f_pos);
		if(rlen>0)
			sum+=rlen;
		else if(0 != rlen)
			return rlen;
		else
			break;
	}
	
	return  sum;

}

static int writeFile(struct file *fp,char *buf,int len) 
{ 
	int wlen=0, sum=0;
	
	if (!fp->f_op || !fp->f_op->write) 
		return -EPERM; 

	while(sum<len) {
		wlen=fp->f_op->write(fp,buf+sum,len-sum, &fp->f_pos);
		if(wlen>0)
			sum+=wlen;
		else if(0 != wlen)
			return wlen;
		else
			break;
	}

	return sum;

}

/*
* Test if the specifi @param path is a file and readable
* @param path the path of the file to test
* @return Linux specific error code
*/
static int isFileReadable(char *path)
{ 
	struct file *fp;
	int ret = 0;
	mm_segment_t oldfs;
	char buf;
 
	fp=filp_open(path, O_RDONLY, 0); 
	if(IS_ERR(fp)) {
		ret = PTR_ERR(fp);
	}
	else {
		oldfs = get_fs(); set_fs(get_ds());
		
		if(1!=readFile(fp, &buf, 1))
			ret = PTR_ERR(fp);
		
		set_fs(oldfs);
		filp_close(fp,NULL);
	}	
	return ret;
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read, or Linux specific error code
*/
static int retriveFromFile(char *path, u8* buf, u32 sz)
{
	int ret =-1;
	mm_segment_t oldfs;
	struct file *fp;

	if(path && buf) {
		if( 0 == (ret=openFile(&fp,path, O_RDONLY, 0)) ){
			DBG_871X("%s openFile path:%s fp=%p\n",__FUNCTION__, path ,fp);

			oldfs = get_fs(); set_fs(get_ds());
			ret=readFile(fp, buf, sz);
			set_fs(oldfs);
			closeFile(fp);
			
			DBG_871X("%s readFile, ret:%d\n",__FUNCTION__, ret);
			
		} else {
			DBG_871X("%s openFile path:%s Fail, ret:%d\n",__FUNCTION__, path, ret);
		}
	} else {
		DBG_871X("%s NULL pointer\n",__FUNCTION__);
		ret =  -EINVAL;
	}
	return ret;
}

/*
* Open the file with @param path and wirte @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written, or Linux specific error code
*/
static int storeToFile(char *path, u8* buf, u32 sz)
{
	int ret =0;
	mm_segment_t oldfs;
	struct file *fp;
	
	if(path && buf) {
		if( 0 == (ret=openFile(&fp, path, O_CREAT|O_WRONLY, 0666)) ) {
			DBG_871X("%s openFile path:%s fp=%p\n",__FUNCTION__, path ,fp);

			oldfs = get_fs(); set_fs(get_ds());
			ret=writeFile(fp, buf, sz);
			set_fs(oldfs);
			closeFile(fp);

			DBG_871X("%s writeFile, ret:%d\n",__FUNCTION__, ret);
			
		} else {
			DBG_871X("%s openFile path:%s Fail, ret:%d\n",__FUNCTION__, path, ret);
		}	
	} else {
		DBG_871X("%s NULL pointer\n",__FUNCTION__);
		ret =  -EINVAL;
	}
	return ret;
}
#endif //PLATFORM_LINUX


/*
* Test if the specifi @param path is a file and readable
* @param path the path of the file to test
* @return _TRUE or _FALSE
*/
int rtw_is_file_readable(char *path)
{
#if 0//def PLATFORM_LINUX  // Irene Lin
	if(isFileReadable(path) == 0)
		return _TRUE;
	else
		return _FALSE;
#else
	//Todo...
	return _FALSE;
#endif
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read
*/
int rtw_retrive_from_file(char *path, u8* buf, u32 sz)
{
#if 0//def PLATFORM_LINUX  // Irene Lin
	int ret =retriveFromFile(path, buf, sz);
	return ret>=0?ret:0;
#else
	//Todo...
	return 0;
#endif
}

/*
* Open the file with @param path and wirte @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written
*/
int rtw_store_to_file(char *path, u8* buf, u32 sz)
{
#if 0//def PLATFORM_LINUX // Irene Lin
	int ret =storeToFile(path, buf, sz);
	return ret>=0?ret:0;
#else
	//Todo...
	return 0;
#endif
}

void _rtw_os_indicate_wowlan_status(struct net_device *pnetdev, unsigned char ID, char *status){
	//DBG_871X("ATWV=[%d] SUCCESSED\n", ID);
	//if(ID == RTW_WOWLAN_CMD_GET_STATUS)
	//{
		//struct rtw_wowlan_status *status = (struct rtw_wowlan_status *)(status);
		//DBG_871X("wake up reason: 0x%x\n", status->wakeup_reasons);
	//}
}

void _rtw_os_indicate_wifi_info(struct net_device *pnetdev, char *info){
//todo
}

void _rtw_os_indicate_scan_result(struct net_device *pnetdev, int status, char *result)
{
//todo
}

void _rtw_indicate_sta_assoc_event(struct net_device *pnetdev, u8 *mac)
{
//todo
}

void _rtw_indicate_sta_disassoc_event(struct net_device *pnetdev, u8 *mac)
{
//todo
}

void _rtw_os_send_simple_config_ack(struct net_device *pnetdev, u32 dest_ip){
//todo
}

void _rtw_os_indicate_connect(struct net_device *pnetdev)
{
	netif_carrier_on(pnetdev);
	if(!rtw_netif_queue_stopped(pnetdev))
		rtw_netif_start_queue(pnetdev);
	else
		rtw_netif_wake_queue(pnetdev);
}

void _rtw_os_indicate_disconnect(struct net_device *pnetdev)
{
	if(pnetdev)
	{
		if (!rtw_netif_queue_stopped(pnetdev))
			rtw_netif_stop_queue(pnetdev);
		netif_carrier_off(pnetdev);
	}
}

void rtw_show_stack(unsigned long *sp){
#ifdef DBG_STACK_TRACE
	dump_stack();
#endif
}

const struct osdep_service_ops osdep_service = {
// 0
//memory ops
	_linux_vmalloc,		//rtw_vmalloc
	_linux_zvmalloc,		//rtw_zvmalloc
	_linux_vmfree,			//rtw_vmfree
	_linux_malloc, //rtw_malloc
	_linux_zmalloc, //rtw_zmalloc
	_linux_mfree, //rtw_mfree
	_linux_memcpy, //rtw_memcpy
	_linux_memcmp, //rtw_memcmp
	_linux_memset, //rtw_memset
// 9	
//list ops
	_linux_init_listhead, //rtw_init_listhead
	_linux_is_list_empty, //rtw_is_list_empty
	_linux_list_insert_head, //rtw_list_insert_head
	_linux_list_insert_tail, //rtw_list_insert_tail
	_linux_list_delete, //rtw_list_delete
// 14	
//sema ops
	_linux_init_sema, //rtw_init_sema
	_linux_free_sema, //rtw_free_sema
	_linux_up_sema, //rtw_up_sema
	_linux_down_sema, //rtw_down_sema
	NULL, //rtw_down_sema_timeout
//19	
//mutex ops
	_linux_mutex_init, //rtw_mutex_init
	_linux_mutex_free, //rtw_mutex_free
	NULL, //rtw_mutex_get
	NULL, //rtw_mutex_put
//23	
//spinlock ops
	_linux_enter_critical,		//rtw_enter_critical
	_linux_exit_critical,		//rtw_exit_critical
	_linux_enter_critical_bh,		//rtw_enter_critical_bh
	_linux_exit_critical_bh,		//rtw_exit_critical_bh
	_linux_enter_critical_mutex,		//rtw_enter_critical_mutex
	_linux_exit_critical_mutex,		//rtw_exit_critical_mutex
	_linux_spinlock_init, //rtw_spinlock_init
	_linux_spinlock_free, //rtw_spinlock_free
	_linux_spin_lock, //rtw_spin_lock
	_linux_spin_unlock, //rtw_spin_unlock
	_linux_spinlock_irqsave,	//rtw_spinlock_irqsave
	_linux_spinunlock_irqsave,//rtw_spinunlock_irqsave
//35	
	NULL,//rtw_init_xqueue
	NULL,//rtw_push_to_xqueue
	NULL,//rtw_pop_from_xqueue
	NULL,//rtw_deinit_xqueue
//39	
//sys time
	_linux_get_current_time, //rtw_get_current_time
	_linux_systime_to_ms, //rtw_systime_to_ms
	NULL, //rtw_systime_to_sec
	_linux_ms_to_systime, //rtw_ms_to_systime
	NULL, //rtw_sec_to_systime
	_linux_msleep_os, //rtw_msleep_os
	_linux_usleep_os, //rtw_usleep_os
	_linux_mdelay_os, //rtw_mdelay_os
	_linux_udelay_os, //rtw_udelay_os
	_linux_yield_os, //rtw_yield_os
// 49	
//timer ops	
	_linux_init_timer, //rtw_init_timer
	_linux_set_timer, //rtw_set_timer
	_linux_cancel_timer_ex, //rtw_cancel_timer
	_linux_del_timer, //rtw_del_timer
// 53	
//ATOMIC ops	
	_linux_ATOMIC_SET, //ATOMIC_SET
	_linux_ATOMIC_READ, //ATOMIC_READ
	_linux_ATOMIC_ADD, //ATOMIC_ADD
	_linux_ATOMIC_SUB, //ATOMIC_SUB
	_linux_ATOMIC_INC, //ATOMIC_INC
	_linux_ATOMIC_DEC, //ATOMIC_DEC
	_linux_ATOMIC_ADD_RETURN, //ATOMIC_ADD_RETURN
	_linux_ATOMIC_SUB_RETURN, //ATOMIC_SUB_RETURN
	_linux_ATOMIC_INC_RETURN, //ATOMIC_INC_RETURN
	_linux_ATOMIC_DEC_RETURN, //ATOMIC_DEC_RETURN
// 63	
	NULL, //rtw_modular64
	NULL,			//rtw_get_random_bytes
	NULL,		//rtw_getFreeHeapSize
// 66
//task ops
	NULL,		//rtw_create_task
	_linux_delete_task,		//rtw_delete_task
	NULL,		//rtw_wakeup_task
//69
	_linux_thread_enter,		//rtw_thread_enter
	NULL,		//rtw_thread_exit
	_linux_flush_signals_thread, //rtw_flush_signals_thread
// 72
	NULL,           //rtw_timerCreate,       
	NULL,           //rtw_timerDelete,       
	NULL,    //rtw_timerIsTimerActive,
	NULL,             //rtw_timerStop,         
	NULL,      //rtw_timerChangePeriod  
	_linux_skb_alloc,
	_linux_skb_free
};


