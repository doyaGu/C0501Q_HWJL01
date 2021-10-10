#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include "ite/ith.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct {
	volatile unsigned int lock;
} spinlock_t;


#define spin_lock_init(x)	
#define spin_lock(x)						  ithEnterCritical()
#define spin_unlock(x)						  ithExitCritical()
#define local_irq_enable(x)
#define local_irq_disable(x)
#define spin_lock_irq(x)                      do { local_irq_disable(x); spin_lock(x); } while (0)
#define spin_unlock_irq(x)                    do { spin_unlock(x); local_irq_enable(x); } while (0)
#define spin_lock_bh(x)                       spin_lock(x)
#define spin_unlock_bh(x)                     spin_unlock(x)
#define local_irq_save(x)					
#define local_irq_restore(x)				
#define spin_lock_irqsave(lock, flags)		do { local_irq_save(flags);       spin_lock(lock); } while (0)
#define spin_unlock_irqrestore(lock, flags)	do { spin_unlock(lock);  local_irq_restore(flags); } while (0)

static inline int spin_trylock_irqsave(void* lock, int flags)
{
    local_irq_save(flags);
    spin_lock(lock);
	return 1;
}



#ifdef __cplusplus
}
#endif

#endif // _SPINLOCK_H_
