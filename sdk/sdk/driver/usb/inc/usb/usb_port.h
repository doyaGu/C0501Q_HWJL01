#ifndef USB_PORT_H
#define USB_PORT_H

#include "openrtos/portmacro.h"


#ifdef __cplusplus
extern "C" {
#endif


#define MMP_TRUE        true
#define MMP_FALSE       false

#define MMP_NULL        NULL
#define MMP_INT         int
#define MMP_INT32       int
#define MMP_UINT32      uint32_t
#define MMP_UINT        uint32_t
#define MMP_UINT16      uint16_t
#define MMP_UINT8       uint8_t
#define MMP_ULONG       uint32_t
#define MMP_BOOL        bool
#define MMP_INLINE      inline


/*
 * for read/write register
 */
#define HOST_WriteRegister(reg,val)             ithWriteRegH(reg,val)
#define HOST_WriteRegisterMask(reg,val,mask)    ithWriteRegMaskH(reg,val,mask)
#define HOST_ReadRegister(reg,pval)             (*pval)=ithReadRegH(reg)
#define AHB_WriteRegister(reg,val)              ithWriteRegA(reg,val)
#define AHB_WriteRegisterMask(reg,val,mask)     ithWriteRegMaskA(reg,val,mask)
#define AHB_ReadRegister(reg,pval)              (*pval)=ithReadRegA(reg)

/*
 *  for block memory operation
 */
#define HOST_SetBlockMemory						ithSetVram
#define HOST_WriteBlockMemory(dst,src,size)		ithWriteVram(dst,(const void*)src,size)
#define HOST_ReadBlockMemory(dst,src,size)		ithReadVram((void*)dst,src,size)


/*
 * vmem api
 */
#define MEM_USAGE_DMA           0
#define MEM_USAGE_QH_EX         0
#define MEM_USAGE_QTD_EX        0
#define MEM_USAGE_PERIODIC_EX   0
#define MEM_USAGE_4KBUF_EX      0
#define MEM_Allocate(size,a,b)  itpVmemAlloc(size)
#define MEM_Release(x)          itpVmemFree((uint32_t)x)

#define SYS_Malloc                      malloc     
#define SYS_Free						free
#define SYS_MemorySet					memset

/*
 * for sempahore
 */
sem_t* usb_create_sem(int cnt);
#define MMP_MUTEX                       sem_t*
#define SYS_CreateSemaphore(cnt,c)      usb_create_sem(cnt)  
#define SYS_WaitSemaphore               sem_wait
#define SYS_ReleaseSemaphore            sem_post
#define SYS_DeleteSemaphore(a)			do { sem_destroy(a); free(a); } while(0)

/*
 * for spin lock
 */
typedef struct {
	volatile unsigned int lock;
} _spinlock_t;

#define _spin_lock_init(x)	
#define _spin_lock(x)					ithEnterCritical()
#define _spin_unlock(x)					ithExitCritical()
#define _local_irq_save()					
#define _local_irq_restore()				
#define _spin_lock_irqsave(lock)		do { _local_irq_save();  _spin_lock(lock); } while (0)
#define _spin_unlock_irqrestore(lock)	do { _spin_unlock(lock);  _local_irq_restore(); } while (0)
#define _spin_lock_irq(x)               _spin_lock(x)
#define _spin_unlock_irq(x)             _spin_unlock(x)

/*
 * for event
 */
#define MMP_EVENT                       sem_t*
#define SYS_CreateEvent()               usb_create_sem(0) 
#define SYS_WaitEvent                   itpSemWaitTimeout
#define SYS_WaitForEventForever			sem_wait
#define SYS_SetEvent                    sem_post
#define SYS_DelEvent(a)                 do { sem_destroy(a); free(a); } while(0)
#define SYS_SetEventFromIsr				itpSemPostFromISR

        
#define MMP_Sleep(x)                    usleep(x*1000)
#define MMP_USleep                      ithDelay

#define HOST_GetVramBaseAddress()       0


#ifdef __cplusplus
}
#endif

#endif


