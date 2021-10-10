/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#define __OSDEP_SERVICE_C__
#include "autoconf.h"
#include "basic_types.h"
#include "osdep_service.h"
#include "rtw_debug.h"
#ifdef PLATFORM_FREERTOS
#include "freertos/wrapper.h"

#ifdef DBG_MEMORY_LEAK
#ifdef PLATFORM_LINUX
atomic_t _malloc_cnt = ATOMIC_INIT(0);
atomic_t _malloc_size = ATOMIC_INIT(0);
#endif
#endif /* DBG_MEMORY_LEAK */
#endif /*PLATFORM_FREERTOS*/

extern struct osdep_service_ops osdep_service;

#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREERTOS)
/*
* Translate the OS dependent @param error_code to OS independent RTW_STATUS_CODE
* @return: one of RTW_STATUS_CODE
*/
#ifndef PLATFORM_FREERTOS
inline 
#endif
int RTW_STATUS_CODE(int error_code){
	if(error_code >=0)
		return _SUCCESS;

	switch(error_code) {
		//case -ETIMEDOUT:
		//	return RTW_STATUS_TIMEDOUT;
		default:
			return _FAIL;
	}
}
#else//PLATFORM_LINUX
#ifndef PLATFORM_FREERTOS
inline 
#endif
int RTW_STATUS_CODE(int error_code){
	return error_code;
}
#endif//PLATFORM_LINUX

u32 rtw_atoi(u8* s)
{
	int num=0,flag=0;
	unsigned int i;

	for(i=0;i<=strlen((char *)s);i++)
	{
		if(s[i] >= '0' && s[i] <= '9')
			num = num * 10 + s[i] -'0';
		else if(s[0] == '-' && i==0) 
			flag =1;
		else 
			break;
	}

	if(flag == 1)
		num = num * -1;

	return(num); 
}

u8* _rtw_vmalloc(u32 sz)
{
	u8 *pbuf = NULL;	
#if CONFIG_USE_TCM_HEAP
	pbuf = tcm_heap_malloc(sz);
#endif
	if(pbuf==NULL){
		if(osdep_service.rtw_vmalloc) {
			pbuf = osdep_service.rtw_vmalloc(sz);
		} else
			DBG_871X("Not implement osdep service: rtw_vmalloc");	
	}
	return pbuf;
}

u8* _rtw_zvmalloc(u32 sz)
{
	u8 *pbuf = NULL;	
#if CONFIG_USE_TCM_HEAP
	pbuf = tcm_heap_calloc(sz);
#endif
	if(pbuf==NULL){
		if(osdep_service.rtw_zvmalloc) {
			pbuf = osdep_service.rtw_zvmalloc(sz);
		} else
			DBG_871X("Not implement osdep service: rtw_zvmalloc");	
	}
	return pbuf;
}

void _rtw_vmfree(u8 *pbuf, u32 sz)
{
	
#if CONFIG_USE_TCM_HEAP
	if( (u32)pbuf > 0x1FFF0000 && (u32)pbuf < 0x20000000 )
		tcm_heap_free(pbuf);
	else
#endif
	{
	if(osdep_service.rtw_vmfree) {
		osdep_service.rtw_vmfree(pbuf, sz);
	} else
		DBG_871X("Not implement osdep service: rtw_vmfree");	
	}
}

u8* _rtw_malloc(u32 sz)
{
	if(osdep_service.rtw_malloc) {
		u8 *pbuf = osdep_service.rtw_malloc(sz);
		return pbuf;
	} else
		DBG_871X("Not implement osdep service: rtw_malloc");	

	return NULL;
}

u8* _rtw_zmalloc(u32 sz)
{
	if(osdep_service.rtw_zmalloc) {
		u8 *pbuf = osdep_service.rtw_zmalloc(sz);
		return pbuf;
	} else
		DBG_871X("Not implement osdep service: rtw_zmalloc");	

	return NULL;
}

void _rtw_mfree(u8 *pbuf, u32 sz)
{
	if(osdep_service.rtw_mfree) {
		osdep_service.rtw_mfree(pbuf, sz);
	} else
		DBG_871X("Not implement osdep service: rtw_mfree");
}

#ifdef CONFIG_MEM_MONITOR
#if CONFIG_MEM_MONITOR & MEM_MONITOR_LEAK
_list mem_table;
int mem_used_num;
#endif
int min_free_heap_size;

void init_mem_monitor(_list *pmem_table, int *used_num)
{
#if CONFIG_MEM_MONITOR & MEM_MONITOR_LEAK
	rtw_init_listhead(pmem_table);
	*used_num = 0;
#endif
	min_free_heap_size = rtw_getFreeHeapSize();
}

void deinit_mem_monitor(_list *pmem_table, int *used_num)
{
#if CONFIG_MEM_MONITOR & MEM_MONITOR_LEAK
	_list *plist;
	struct mem_entry *mem_entry;
	
	if(*used_num > 0)
		DBG_ERR("Have %d mem_entry kept in monitor", *used_num);
	else
		DBG_INFO("No mem_entry kept in monitor");
	
	save_and_cli();
	
	while (rtw_end_of_queue_search(pmem_table, get_next(pmem_table)) == _FALSE)	{		
		plist = get_next(pmem_table);
		mem_entry = LIST_CONTAINOR(plist, struct mem_entry, list);
		
		DBG_ERR("Not release memory at %p with size of %d", mem_entry->ptr, mem_entry->size);
		
		rtw_list_delete(plist);
		_rtw_mfree((u8 *) mem_entry, sizeof(struct mem_entry));
	}
	
	restore_flags();
#endif
}

void add_mem_usage(_list *pmem_table, void *ptr, int size, int *used_num, int flag)
{
	int free_heap_size = rtw_getFreeHeapSize();
#if CONFIG_MEM_MONITOR & MEM_MONITOR_LEAK
	struct mem_entry *mem_entry;
#endif
	if(ptr == NULL) {
		DBG_ERR("Catch a mem alloc fail with size of %d, current heap free size = %d", size, free_heap_size);
		return;
	}
	else{
		if(flag == MEM_MONITOR_FLAG_WPAS)
			DBG_INFO("Alloc memory at %p with size of %d", ptr, size);
		else
			DBG_INFO("Alloc memory at %p with size of %d", ptr, size);
	}
#if CONFIG_MEM_MONITOR & MEM_MONITOR_LEAK
	mem_entry = (struct mem_entry *) _rtw_malloc(sizeof(struct mem_entry));

	if(mem_entry == NULL) {
		DBG_ERR("Fail to alloc mem_entry");
		return;
	}

	memset(mem_entry, 0, sizeof(struct mem_entry));
	mem_entry->ptr = ptr;
	mem_entry->size = size;
	
	save_and_cli();
	rtw_list_insert_head(&mem_entry->list, pmem_table);
	restore_flags();

	*used_num ++;
#endif
	if(min_free_heap_size > free_heap_size)
		min_free_heap_size = free_heap_size;
}

void del_mem_usage(_list *pmem_table, void *ptr, int *used_num, int flag)
{
#if CONFIG_MEM_MONITOR & MEM_MONITOR_LEAK
	_list *plist;
	struct mem_entry *mem_entry = NULL;

	if(ptr == NULL)
		return;
	
	if(flag == MEM_MONITOR_FLAG_WPAS)
		DBG_INFO("Free memory at %p", ptr);
	else
		DBG_INFO("Free memory at %p", ptr);	

	save_and_cli();

	plist = get_next(pmem_table);
	while ((rtw_end_of_queue_search(pmem_table, plist)) == _FALSE)	
	{		
		mem_entry = LIST_CONTAINOR(plist, struct mem_entry, list);
		if(mem_entry->ptr == ptr) {
			rtw_list_delete(plist);
			break;
		}
		plist = get_next(plist);
	}

	restore_flags();

	if(plist == pmem_table)
		DBG_ERR("Fail to find the mem_entry in mem table");
	else {
		*used_num --;
		_rtw_mfree((u8 *) mem_entry, sizeof(struct mem_entry));
	}
#endif
}

#if CONFIG_MEM_MONITOR & MEM_MONITOR_LEAK
int get_mem_usage(_list *pmem_table)
{
	_list *plist;
	struct mem_entry *mem_entry;
	int mem_usage = 0;
	int entry_num = 0;

	save_and_cli();

	if((plist = get_next(pmem_table)) == NULL) {
		DBG_ERR("No mem table available\n");
		restore_flags();
		return 0;
	}

	while (rtw_end_of_queue_search(pmem_table, plist) == _FALSE) {
		entry_num ++;
		mem_entry = LIST_CONTAINOR(plist, struct mem_entry, list);
		mem_usage += mem_entry->size;

		DBG_INFO("size of mem_entry(%d)=%d\n", entry_num, mem_entry->size);
		plist = get_next(plist);
	}

	restore_flags();

	DBG_INFO("Get %d mem_entry\n", entry_num);

	return mem_usage;
}
#endif


u8* rtw_vmalloc(u32 sz)
{
	u8 *pbuf = _rtw_vmalloc(sz);
#if CONFIG_MEM_MONITOR & MEM_MONITOR_LEAK
	add_mem_usage(&mem_table, pbuf, sz, &mem_used_num, MEM_MONITOR_FLAG_WIFI_DRV);
#else
	add_mem_usage(NULL, pbuf, sz, NULL, MEM_MONITOR_FLAG_WIFI_DRV);
#endif
	return pbuf;
}

u8* rtw_zvmalloc(u32 sz)
{
	u8 *pbuf = _rtw_zvmalloc(sz);
#if CONFIG_MEM_MONITOR & MEM_MONITOR_LEAK
	add_mem_usage(&mem_table, pbuf, sz, &mem_used_num, MEM_MONITOR_FLAG_WIFI_DRV);
#else
	add_mem_usage(NULL, pbuf, sz, NULL, MEM_MONITOR_FLAG_WIFI_DRV);
#endif
	return pbuf;
}

void rtw_vmfree(u8 *pbuf, u32 sz)
{
	_rtw_vmfree(pbuf, sz);
#if CONFIG_MEM_MONITOR & MEM_MONITOR_LEAK
	del_mem_usage(&mem_table, pbuf, &mem_used_num, MEM_MONITOR_FLAG_WIFI_DRV);	
#else
	del_mem_usage(NULL, pbuf, NULL, MEM_MONITOR_FLAG_WIFI_DRV);
#endif
}

u8* rtw_malloc(u32 sz)
{
	u8 *pbuf = _rtw_malloc(sz);
#if CONFIG_MEM_MONITOR & MEM_MONITOR_LEAK
	add_mem_usage(&mem_table, pbuf, sz, &mem_used_num, MEM_MONITOR_FLAG_WIFI_DRV);
#else
	add_mem_usage(NULL, pbuf, sz, NULL, MEM_MONITOR_FLAG_WIFI_DRV);
#endif
	return pbuf;
}

u8* rtw_zmalloc(u32 sz)
{
	u8 *pbuf = _rtw_zmalloc(sz);
#if CONFIG_MEM_MONITOR & MEM_MONITOR_LEAK
	add_mem_usage(&mem_table, pbuf, sz, &mem_used_num, MEM_MONITOR_FLAG_WIFI_DRV);
#else
	add_mem_usage(NULL, pbuf, sz, NULL, MEM_MONITOR_FLAG_WIFI_DRV);
#endif
	return pbuf;
}

void rtw_mfree(u8 *pbuf, u32 sz)
{
	_rtw_mfree(pbuf, sz);
#if CONFIG_MEM_MONITOR & MEM_MONITOR_LEAK
	del_mem_usage(&mem_table, pbuf, &mem_used_num, MEM_MONITOR_FLAG_WIFI_DRV);	
#else
	del_mem_usage(NULL, pbuf, NULL, MEM_MONITOR_FLAG_WIFI_DRV);
#endif
}
#endif

void* rtw_malloc2d(int h, int w, int size)
{
	int j;

	void **a = (void **) rtw_zmalloc( h*sizeof(void *) + h*w*size );
	if(a == NULL)
	{
		DBG_871X("%s: alloc memory fail!\n", __FUNCTION__);
		return NULL;
	}

	for( j=0; j<h; j++ )
		a[j] = ((char *)(a+h)) + j*w*size;

	return a;
}

void rtw_mfree2d(void *pbuf, int h, int w, int size)
{
	rtw_mfree((u8 *)pbuf, h*sizeof(void*) + w*h*size);
}

void rtw_memcpy(void* dst, void* src, u32 sz)
{
	if(osdep_service.rtw_memcpy)
		osdep_service.rtw_memcpy(dst, src, sz);
	else
		DBG_871X("Not implement osdep service: rtw_memcpy");
}

int rtw_memcmp(void *dst, void *src, u32 sz)
{
	if(osdep_service.rtw_memcmp)
		return osdep_service.rtw_memcmp(dst, src, sz);
	else
		DBG_871X("Not implement osdep service: rtw_memcmp");

	return _FALSE;
}

void rtw_memset(void *pbuf, int c, u32 sz)
{
	if(osdep_service.rtw_memset)
		osdep_service.rtw_memset(pbuf, c, sz);
	else
		DBG_871X("Not implement osdep service: rtw_memset");
}

void rtw_init_listhead(_list *list)
{
	if(osdep_service.rtw_init_listhead)
		osdep_service.rtw_init_listhead(list);
	else
		DBG_871X("Not implement osdep service: rtw_init_listhead");
}

/*
For the following list_xxx operations, 
caller must guarantee the atomic context.
Otherwise, there will be racing condition.
*/
u32 rtw_is_list_empty(_list *phead)
{
	if(osdep_service.rtw_is_list_empty)
		return osdep_service.rtw_is_list_empty(phead);
	else
		DBG_871X("Not implement osdep service: rtw_is_list_empty");
	return _FALSE;
}

void rtw_list_insert_head(_list *plist, _list *phead)
{
	if(osdep_service.rtw_list_insert_head)
		osdep_service.rtw_list_insert_head(plist, phead);
	else
		DBG_871X("Not implement osdep service: rtw_list_insert_head");
}

void rtw_list_insert_tail(_list *plist, _list *phead)
{
	if(osdep_service.rtw_list_insert_tail)
		osdep_service.rtw_list_insert_tail(plist, phead);
	else
		DBG_871X("Not implement osdep service: rtw_list_insert_tail");
}

/*

Caller must check if the list is empty before calling rtw_list_delete

*/
void rtw_list_delete(_list *plist)
{
	if(osdep_service.rtw_list_delete)
		osdep_service.rtw_list_delete(plist);
	else
		DBG_871X("Not implement osdep service: rtw_list_delete");
}

void rtw_init_sema(_sema *sema, int init_val)
{
	if(osdep_service.rtw_init_sema)
		osdep_service.rtw_init_sema(sema, init_val);
	else
		DBG_871X("Not implement osdep service: rtw_init_sema");
}

void rtw_free_sema(_sema *sema)
{
	if(osdep_service.rtw_free_sema)
		osdep_service.rtw_free_sema(sema);
	else
		DBG_871X("Not implement osdep service: rtw_free_sema");
}

void rtw_up_sema(_sema *sema)
{
	if(osdep_service.rtw_up_sema)
		osdep_service.rtw_up_sema(sema);
	else
		DBG_871X("Not implement osdep service: rtw_up_sema");
}

u32 rtw_down_sema(_sema *sema)
{
	if(osdep_service.rtw_down_sema)
		return osdep_service.rtw_down_sema(sema);
	else
		DBG_871X("Not implement osdep service: rtw_down_sema");

	return _FAIL;
}

u32	rtw_down_timeout_sema(_sema *sema, u32 timeout)
{
	if(osdep_service.rtw_down_timeout_sema)
		return osdep_service.rtw_down_timeout_sema(sema, timeout);
	else
		DBG_871X("Not implement osdep service: rtw_down_timeout_sema");

	return _FAIL;
}

void rtw_mutex_init(_mutex *pmutex)
{
	if(osdep_service.rtw_mutex_init)
		osdep_service.rtw_mutex_init(pmutex);
	else
		DBG_871X("Not implement osdep service: rtw_mutex_init");
}

void rtw_mutex_free(_mutex *pmutex)
{
	if(osdep_service.rtw_mutex_free)
		osdep_service.rtw_mutex_free(pmutex);
	else
		DBG_871X("Not implement osdep service: rtw_mutex_free");
}

void rtw_mutex_put(_mutex *pmutex)
{
	if(osdep_service.rtw_mutex_put)
		osdep_service.rtw_mutex_put(pmutex);
	else
		DBG_871X("Not implement osdep service: rtw_mutex_put");
}

void rtw_mutex_get(_mutex *pmutex)
{
	if(osdep_service.rtw_mutex_get)
		osdep_service.rtw_mutex_get(pmutex);
	else
		DBG_871X("Not implement osdep service: rtw_mutex_get");
}

void rtw_enter_critical(_lock *plock, _irqL *pirqL)
{
	if(osdep_service.rtw_enter_critical)
		osdep_service.rtw_enter_critical(plock, pirqL);
	else
		DBG_871X("Not implement osdep service: rtw_enter_critical");
}

void rtw_exit_critical(_lock *plock, _irqL *pirqL)
{
	if(osdep_service.rtw_exit_critical)
		osdep_service.rtw_exit_critical(plock, pirqL);
	else
		DBG_871X("Not implement osdep service: rtw_exit_critical");
}

void rtw_enter_critical_bh(_lock *plock, _irqL *pirqL)
{
	if(osdep_service.rtw_enter_critical_bh)
		osdep_service.rtw_enter_critical_bh(plock, pirqL);
	else
		DBG_871X("Not implement osdep service: rtw_enter_critical_bh");
}

void rtw_exit_critical_bh(_lock *plock, _irqL *pirqL)
{
	if(osdep_service.rtw_exit_critical_bh)
		osdep_service.rtw_exit_critical_bh(plock, pirqL);
	else
		DBG_871X("Not implement osdep service: rtw_enter_critical_bh");
}

int rtw_enter_critical_mutex(_mutex *pmutex, _irqL *pirqL)
{
	if(osdep_service.rtw_enter_critical_mutex)
		return osdep_service.rtw_enter_critical_mutex(pmutex, pirqL);
	else
		DBG_871X("Not implement osdep service: rtw_enter_critical_mutex");

	return 0;
}

void rtw_exit_critical_mutex(_mutex *pmutex, _irqL *pirqL)
{
	if(osdep_service.rtw_exit_critical_mutex)
		osdep_service.rtw_exit_critical_mutex(pmutex, pirqL);
	else
		DBG_871X("Not implement osdep service: rtw_exit_critical_mutex");
}

void	rtw_init_queue(_queue	*pqueue)
{
	rtw_init_listhead(&(pqueue->queue));
	rtw_spinlock_init(&(pqueue->lock));
}

u32	  rtw_queue_empty(_queue	*pqueue)
{
	return (rtw_is_list_empty(&(pqueue->queue)));
}

void	rtw_deinit_queue(_queue *pqueue)
{
	rtw_spinlock_free(&(pqueue->lock));
}

u32 rtw_end_of_queue_search(_list *head, _list *plist)
{
	if (head == plist)
		return _TRUE;
	else
		return _FALSE;
}

_list	*rtw_get_queue_head(_queue	*queue)
{
	return (&(queue->queue));
}

#if 1
void rtw_spinlock_init(_lock *plock)
{
	if(osdep_service.rtw_spinlock_init)
		osdep_service.rtw_spinlock_init(plock);
	else
		DBG_871X("Not implement osdep service: rtw_spinlock_init");
}

void rtw_spinlock_free(_lock *plock)
{
	if(osdep_service.rtw_spinlock_free)
		osdep_service.rtw_spinlock_free(plock);
	else
		DBG_871X("Not implement osdep service: rtw_spinlock_free");
}

void rtw_spin_lock(_lock *plock)
{
	if(osdep_service.rtw_spin_lock)
		osdep_service.rtw_spin_lock(plock);
	else
		DBG_871X("Not implement osdep service: rtw_spin_lock");
}

void rtw_spin_unlock(_lock *plock)
{
	if(osdep_service.rtw_spin_unlock)
		osdep_service.rtw_spin_unlock(plock);
	else
		DBG_871X("Not implement osdep service: rtw_spin_unlock");
}

void rtw_spinlock_irqsave(_lock *plock, _irqL *irqL)
{
	if(osdep_service.rtw_spinlock_irqsave)
		osdep_service.rtw_spinlock_irqsave(plock, irqL);
	else
		DBG_871X("Not implement osdep service: rtw_spinlock_irqsave");
}

void rtw_spinunlock_irqsave(_lock *plock, _irqL *irqL)
{
	if(osdep_service.rtw_spinunlock_irqsave)
		osdep_service.rtw_spinunlock_irqsave(plock, irqL);
	else
		DBG_871X("Not implement osdep service: rtw_spinunlock_irqsave");
}
#endif

#if 0 //to do for freertos
rtw_result_t rtw_init_xqueue( _xqueue* queue, const char* name, u32 message_size, u32 number_of_messages )
{
	if(osdep_service.rtw_init_xqueue)
		return (rtw_result_t)osdep_service.rtw_init_xqueue(queue, name, message_size, number_of_messages);
	else
		DBG_871X("Not implement osdep service: rtw_init_xqueue");

	return RTW_NOTFOUND;
}

rtw_result_t rtw_push_to_xqueue( _xqueue* queue, void* message, u32 timeout_ms )
{
	if(osdep_service.rtw_push_to_xqueue)
		return (rtw_result_t)osdep_service.rtw_push_to_xqueue(queue, message, timeout_ms);
	else
		DBG_871X("Not implement osdep service: rtw_push_to_xqueue");

	return RTW_NOTFOUND;
}

rtw_result_t rtw_pop_from_xqueue( _xqueue* queue, void* message, u32 timeout_ms )
{
	if(osdep_service.rtw_pop_from_xqueue)
		return (rtw_result_t)osdep_service.rtw_pop_from_xqueue(queue, message, timeout_ms);
	else
		DBG_871X("Not implement osdep service: rtw_pop_from_xqueue");

	return RTW_NOTFOUND;
}

rtw_result_t rtw_deinit_xqueue( _xqueue* queue )
{
	if(osdep_service.rtw_deinit_xqueue)
		return (rtw_result_t)osdep_service.rtw_deinit_xqueue(queue);
	else
		DBG_871X("Not implement osdep service: rtw_deinit_xqueue");

	return RTW_NOTFOUND;
}
#endif

u32 rtw_get_current_time(void)
{
	if(osdep_service.rtw_get_current_time)
		return osdep_service.rtw_get_current_time();
	else
		DBG_871X("Not implement osdep service: rtw_get_current_time");

	return 0;
}

u32 rtw_systime_to_ms(u32 systime)
{
	if(osdep_service.rtw_systime_to_ms)
		return osdep_service.rtw_systime_to_ms(systime);
	else
		DBG_871X("Not implement osdep service: rtw_systime_to_ms");

	return 0;
}

u32 rtw_systime_to_sec(u32 systime)
{
	if(osdep_service.rtw_systime_to_sec)
		return osdep_service.rtw_systime_to_sec(systime);
	else
		DBG_871X("Not implement osdep service: rtw_systime_to_sec");

	return 0;
}

u32 rtw_ms_to_systime(u32 ms)
{
	if(osdep_service.rtw_ms_to_systime)
		return osdep_service.rtw_ms_to_systime(ms);
	else
		DBG_871X("Not implement osdep service: rtw_ms_to_systime");

	return 0;
}

u32 rtw_sec_to_systime(u32 sec)
{
	if(osdep_service.rtw_sec_to_systime)
		return osdep_service.rtw_sec_to_systime(sec);
	else
		DBG_871X("Not implement osdep service: rtw_sec_to_systime");

	return 0;
}

// the input parameter start use the same unit as returned by rtw_get_current_time
s32 rtw_get_passing_time_ms(u32 start)
{
	return rtw_systime_to_ms(rtw_get_current_time() - start);
}

s32 rtw_get_time_interval_ms(u32 start, u32 end)
{
	return rtw_systime_to_ms(end - start);
}
	
void rtw_msleep_os(int ms)
{
	if(osdep_service.rtw_msleep_os)
		osdep_service.rtw_msleep_os(ms);
	else
		DBG_871X("Not implement osdep service: rtw_msleep_os");
}

void rtw_usleep_os(int us)
{
	if(osdep_service.rtw_usleep_os)
		osdep_service.rtw_usleep_os(us);
	else
		DBG_871X("Not implement osdep service: rtw_usleep_os");	
}

void rtw_mdelay_os(int ms)
{
	if(osdep_service.rtw_mdelay_os)
		osdep_service.rtw_mdelay_os(ms);
	else
		DBG_871X("Not implement osdep service: rtw_mdelay_os");
}

void rtw_udelay_os(int us)
{
	if(osdep_service.rtw_udelay_os)
		osdep_service.rtw_udelay_os(us);
	else
		DBG_871X("Not implement osdep service: rtw_udelay_os");
}

void rtw_yield_os(void)
{
	if(osdep_service.rtw_yield_os)
		osdep_service.rtw_yield_os();
	else
		DBG_871X("Not implement osdep service: rtw_yield_os");
}

void rtw_init_timer(_timer *ptimer, void *adapter, TIMER_FUN pfunc,void* cntx, const char *name)
{
	if(osdep_service.rtw_init_timer)
		osdep_service.rtw_init_timer(ptimer, adapter, pfunc, cntx, name);
	else
		DBG_871X("Not implement osdep service: rtw_init_timer");
}

void rtw_set_timer(_timer *ptimer,u32 delay_time)
{
	if(osdep_service.rtw_set_timer)
		osdep_service.rtw_set_timer(ptimer, delay_time);
	else
		DBG_871X("Not implement osdep service: rtw_set_timer");
}

u8 rtw_cancel_timer(_timer *ptimer)
{
	if(osdep_service.rtw_cancel_timer)
		osdep_service.rtw_cancel_timer(ptimer);
	else
		DBG_871X("Not implement osdep service: rtw_cancel_timer");
	
	return 0;
}

void rtw_del_timer(_timer *ptimer)
{
	if(osdep_service.rtw_del_timer)
		osdep_service.rtw_del_timer(ptimer);
	else
		DBG_871X("Not implement osdep service: rtw_del_timer");
}

void ATOMIC_SET(ATOMIC_T *v, int i)
{
	if(osdep_service.ATOMIC_SET)
		osdep_service.ATOMIC_SET(v, i);
	else
		DBG_871X("Not implement osdep service: ATOMIC_SET");
}

int ATOMIC_READ(ATOMIC_T *v)
{
	if(osdep_service.ATOMIC_READ)
		return osdep_service.ATOMIC_READ(v);
	else
		DBG_871X("Not implement osdep service: ATOMIC_READ");

	return 0;
}

void ATOMIC_ADD(ATOMIC_T *v, int i)
{
	if(osdep_service.ATOMIC_ADD)
		osdep_service.ATOMIC_ADD(v, i);
	else
		DBG_871X("Not implement osdep service: ATOMIC_ADD");
}

void ATOMIC_SUB(ATOMIC_T *v, int i)
{
	if(osdep_service.ATOMIC_SUB)
		osdep_service.ATOMIC_SUB(v, i);
	else
		DBG_871X("Not implement osdep service: ATOMIC_SUB");
}

void ATOMIC_INC(ATOMIC_T *v)
{
	if(osdep_service.ATOMIC_INC)
		osdep_service.ATOMIC_INC(v);
	else
		DBG_871X("Not implement osdep service: ATOMIC_INC");
}

void ATOMIC_DEC(ATOMIC_T *v)
{
	if(osdep_service.ATOMIC_DEC)
		osdep_service.ATOMIC_DEC(v);
	else
		DBG_871X("Not implement osdep service: ATOMIC_DEC");
}

int ATOMIC_ADD_RETURN(ATOMIC_T *v, int i)
{
	if(osdep_service.ATOMIC_ADD_RETURN)
		return osdep_service.ATOMIC_ADD_RETURN(v, i);
	else
		DBG_871X("Not implement osdep service: ATOMIC_ADD_RETURN");

	return 0;
}

int ATOMIC_SUB_RETURN(ATOMIC_T *v, int i)
{
	if(osdep_service.ATOMIC_SUB_RETURN)
		return osdep_service.ATOMIC_SUB_RETURN(v, i);
	else
		DBG_871X("Not implement osdep service: ATOMIC_SUB_RETURN");

	return 0;
}

int ATOMIC_INC_RETURN(ATOMIC_T *v)
{
	if(osdep_service.ATOMIC_INC_RETURN)
		return osdep_service.ATOMIC_INC_RETURN(v);
	else
		DBG_871X("Not implement osdep service: ATOMIC_INC_RETURN");

	return 0;
}

int ATOMIC_DEC_RETURN(ATOMIC_T *v)
{
	if(osdep_service.ATOMIC_DEC_RETURN)
		return osdep_service.ATOMIC_DEC_RETURN(v);
	else
		DBG_871X("Not implement osdep service: ATOMIC_DEC_RETURN");

	return 0;
}

int ATOMIC_DEC_AND_TEST(ATOMIC_T *v)
{
	return ATOMIC_DEC_RETURN(v) == 0;
}

u64 rtw_modular64(u64 x, u64 y)
{
	if(osdep_service.rtw_modular64)
		return osdep_service.rtw_modular64(x, y);
	else
		DBG_871X("Not implement osdep service: rtw_modular64");

	return 0;
}

int rtw_get_random_bytes(void* dst, u32 size)
{
	if(osdep_service.rtw_get_random_bytes)
		return osdep_service.rtw_get_random_bytes(dst, size);
	else
		DBG_871X("Not implement osdep service: rtw_get_random_bytes");

	return 0;
}

u32 rtw_getFreeHeapSize(void)
{
	if(osdep_service.rtw_getFreeHeapSize)
		return osdep_service.rtw_getFreeHeapSize();
	else
		DBG_871X("Not implement osdep service: rtw_getFreeHeapSize");

	return 0;
}

int rtw_netif_queue_stopped(struct net_device *pnetdev)
{
#ifdef PLATFORM_LINUX
	return _linux_netif_queue_stopped(pnetdev);
#else
	return _TRUE;
#endif
}

void rtw_netif_wake_queue(struct net_device *pnetdev)
{
#ifdef PLATFORM_LINUX
	_linux_netif_wake_queue(pnetdev);
#endif
}

void rtw_netif_start_queue(struct net_device *pnetdev)
{
#ifdef PLATFORM_LINUX
	_linux_netif_start_queue(pnetdev);
#endif
}

void rtw_netif_stop_queue(struct net_device *pnetdev)
{
#ifdef PLATFORM_LINUX
	_linux_netif_stop_queue(pnetdev);
#endif
}

#if 0//to do for freertos
int rtw_create_task(struct task_struct *task, const char *name,
	u32 stack_size, u32 priority, thread_func_t func, void *thctx)
{
	if(osdep_service.rtw_create_task)
		return osdep_service.rtw_create_task(task, name, stack_size, priority, func, thctx);
	else
		DBG_871X("Not implement osdep service: rtw_create_task");
	return 1;
}
void rtw_delete_task(struct task_struct *task)
{
	if(osdep_service.rtw_delete_task)
		osdep_service.rtw_delete_task(task);
	else
		DBG_871X("Not implement osdep service: rtw_delete_task");

	return;	
}
void rtw_wakeup_task(struct task_struct *task)
{
	if(osdep_service.rtw_wakeup_task)
		osdep_service.rtw_wakeup_task(task);
	else
		DBG_871X("Not implement osdep service: rtw_wakeup_task");

	return;	
}

static void worker_thread_main( void *arg )
{
	rtw_worker_thread_t* worker_thread = (rtw_worker_thread_t*) arg;

	while ( 1 )
	{
		rtw_event_message_t message;

		if ( rtw_pop_from_xqueue( &worker_thread->event_queue, &message, RTW_WAIT_FOREVER ) == RTW_SUCCESS )
		{
			message.function(message.buf, message.buf_len, message.flags, message.user_data);
		}
	}
}

rtw_result_t rtw_create_worker_thread( rtw_worker_thread_t* worker_thread, u8 priority, u32 stack_size, u32 event_queue_size )
{
	if(NULL == worker_thread)
		return RTW_ERROR;

	memset( worker_thread, 0, sizeof( *worker_thread ) );

	if ( rtw_init_xqueue( &worker_thread->event_queue, "worker queue", sizeof(rtw_event_message_t), event_queue_size ) != RTW_SUCCESS )
	{
		return RTW_ERROR;
	}

	if ( !rtw_create_task( &worker_thread->thread, "worker thread", stack_size, priority, worker_thread_main, (void*) worker_thread ) )
	{
		rtw_deinit_xqueue( &worker_thread->event_queue );
		return RTW_ERROR;
	}

	return RTW_SUCCESS;
}

rtw_result_t rtw_delete_worker_thread( rtw_worker_thread_t* worker_thread )
{
	if(NULL == worker_thread)
		return RTW_ERROR;

	rtw_deinit_xqueue( &worker_thread->event_queue );

	rtw_delete_task(&worker_thread->thread);

	return RTW_SUCCESS;
}
#endif
_timerHandle rtw_timerCreate( const signed char *pcTimerName, 
							  osdepTickType xTimerPeriodInTicks, 
							  u32 uxAutoReload, 
							  void * pvTimerID, 
							  TIMER_FUN pxCallbackFunction )
{
	if(osdep_service.rtw_timerCreate)
		return osdep_service.rtw_timerCreate(pcTimerName, xTimerPeriodInTicks, uxAutoReload, 
											 pvTimerID, pxCallbackFunction);
	else
		DBG_871X("Not implement osdep service: rtw_timerCreate");

	return 0;	
}

u32 rtw_timerDelete( _timerHandle xTimer, 
							   osdepTickType xBlockTime )
{
	if(osdep_service.rtw_timerDelete)
		return osdep_service.rtw_timerDelete( xTimer, xBlockTime );
	else
		DBG_871X("Not implement osdep service: rtw_timerDelete");

	return 0;	
}

u32 rtw_timerIsTimerActive( _timerHandle xTimer )
{
	if(osdep_service.rtw_timerIsTimerActive)
		return osdep_service.rtw_timerIsTimerActive(xTimer);
	else
		DBG_871X("Not implement osdep service: rtw_timerIsTimerActive");

	return 0;	
}

u32  rtw_timerStop( _timerHandle xTimer, 
							   osdepTickType xBlockTime )
{
	if(osdep_service.rtw_timerStop)
		return osdep_service.rtw_timerStop(xTimer, xBlockTime);
	else
		DBG_871X("Not implement osdep service: rtw_timerStop");

	return 0;	
}

u32  rtw_timerChangePeriod( _timerHandle xTimer, 
							   osdepTickType xNewPeriod, 
							   osdepTickType xBlockTime )
{
	if(osdep_service.rtw_timerChangePeriod)
		return osdep_service.rtw_timerChangePeriod(xTimer, xNewPeriod, xBlockTime);
	else
		DBG_871X("Not implement osdep service: rtw_timerChangePeriod");

	return 0;	
}

#if 0 //TODO
void rtw_init_delayed_work(struct delayed_work *dwork, work_func_t func, const char *name)
{
	if(osdep_service.rtw_init_delayed_work)
		osdep_service.rtw_init_delayed_work(dwork, func, name);
	else
		DBG_871X("Not implement osdep service: rtw_init_delayed_work");

	return;	
}

void rtw_deinit_delayed_work(struct delayed_work *dwork)
{
	if(osdep_service.rtw_deinit_delayed_work)
		osdep_service.rtw_deinit_delayed_work(dwork);
	else
		DBG_871X("Not implement osdep service: rtw_deinit_delayed_work");

	return;	
}

int rtw_queue_delayed_work(struct workqueue_struct *wq,
			struct delayed_work *dwork, u32 delay, void* context)
{
	if(osdep_service.rtw_queue_delayed_work)
		osdep_service.rtw_queue_delayed_work(wq, dwork, delay, context);
	else
		DBG_871X("Not implement osdep service: rtw_queue_delayed_work");

	return;	
}

BOOLEAN rtw_cancel_delayed_work(struct delayed_work *dwork)
{
	if(osdep_service.rtw_cancel_delayed_work)
		osdep_service.rtw_cancel_delayed_work(dwork);
	else
		DBG_871X("Not implement osdep service: rtw_cancel_delayed_work");

	return;	
}
#endif

void rtw_thread_enter(char *name)
{
	if(osdep_service.rtw_thread_enter)
		osdep_service.rtw_thread_enter(name);
	else
		DBG_871X("Not implement osdep service: rtw_thread_enter");
}
#ifndef PLATFORM_LINUX//need return under Linux, use define
void rtw_thread_exit(void)
{
	if(osdep_service.rtw_thread_exit)
		osdep_service.rtw_thread_exit();
	else
		DBG_871X("Not implement osdep service: rtw_thread_exit");
}
#endif
void rtw_flush_signals_thread(void) 
{
	if(osdep_service.rtw_flush_signals_thread)
		osdep_service.rtw_flush_signals_thread();
	else
		DBG_871X("Not implement osdep service: rtw_flush_signals_thread");
}

int rtw_create_task(struct task_struct *task, const char *name,
	u32 stack_size, u32 priority, thread_func_t func, void *thctx)
{
	if(osdep_service.rtw_create_task)
		return osdep_service.rtw_create_task(task, name, stack_size, priority, func, thctx);
	else
		DBG_871X("Not implement osdep service: rtw_create_task");
	return _FAIL;
}

void rtw_delete_task(struct task_struct *task)
{
	if(osdep_service.rtw_delete_task)
		osdep_service.rtw_delete_task(task);
	else
		DBG_871X("Not implement osdep service: rtw_delete_task");

	return;	
}
void rtw_wakeup_task(struct task_struct *task)
{
	if(osdep_service.rtw_wakeup_task)
		osdep_service.rtw_wakeup_task(task);
	else
		DBG_871X("Not implement osdep service: rtw_wakeup_task");

	return;	
}

struct net_device *rtw_alloc_etherdev_with_old_priv(int sizeof_priv, void *old_priv)
{
	struct net_device *pnetdev;
	struct rtw_netdev_priv_indicator *pnpi;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	pnetdev = alloc_etherdev_mq(sizeof(struct rtw_netdev_priv_indicator), 4);
#else
	pnetdev = alloc_etherdev(sizeof(struct rtw_netdev_priv_indicator));
#endif
	if (!pnetdev)
		goto RETURN;
	
	pnpi = netdev_priv(pnetdev);
	pnpi->priv=old_priv;
	pnpi->sizeof_priv=sizeof_priv;

RETURN:
	return pnetdev;
}

struct net_device * rtw_alloc_etherdev(int sizeof_priv)
{
	struct net_device *pnetdev;
	struct rtw_netdev_priv_indicator *pnpi;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	pnetdev = alloc_etherdev_mq(sizeof(struct rtw_netdev_priv_indicator), 4);
#else
	pnetdev = alloc_etherdev(sizeof(struct rtw_netdev_priv_indicator));
#endif
	if (!pnetdev)
		goto RETURN;
	
	pnpi = netdev_priv(pnetdev);
	
	pnpi->priv = rtw_zvmalloc(sizeof_priv);
	if (!pnpi->priv) {
		free_netdev(pnetdev);
		pnetdev = NULL;
		goto RETURN;
	}
	
	pnpi->sizeof_priv=sizeof_priv;
RETURN:
	return pnetdev;
}

void rtw_free_netdev(struct net_device * netdev)
{
	struct rtw_netdev_priv_indicator *pnpi;
	
	if(!netdev)
		goto RETURN;
	
	pnpi = netdev_priv(netdev);

	if(!pnpi->priv)
		goto RETURN;

	rtw_vmfree(pnpi->priv, pnpi->sizeof_priv);
	free_netdev(netdev);

RETURN:
	return;
}

struct sk_buff *rtw_skb_alloc(u32 sz)
{
	if(osdep_service.rtw_skb_alloc)
		return osdep_service.rtw_skb_alloc(sz);
	else
		DBG_871X("Not implement osdep service: _freertos_skb_alloc");
	return _FAIL;
}

void rtw_skb_free(struct sk_buff *skb)
{
	if(osdep_service.rtw_skb_free)
		osdep_service.rtw_skb_free(skb);
	else
		DBG_871X("Not implement osdep service: rtw_skb_free");
}

