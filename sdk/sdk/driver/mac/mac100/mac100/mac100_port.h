
 
#define MAC_TRUE        true
#define MAC_FALSE       false

#define MAC_NULL        NULL
#define MAC_INT         int
#define MAC_UINT32      uint32_t
#define MAC_UINT16      uint16_t
#define MAC_UINT8       uint8_t
#define MAC_BOOL        bool
#define MAC_INLINE      inline
        
/*
 * for read/write register
 */
#define MAC_WriteRegH(val,reg)       ithWriteRegH(reg,val)
#define MAC_WriteRegMaskH(val,reg,mask)       ithWriteRegMaskH(reg,val,mask)
#define MAC_ReadRegH(reg,pval)       (*pval)=ithReadRegH(reg)
#define MAC_WriteReg(val,reg)       ithWriteRegA(reg,val)
#define MAC_WriteRegMask(val,reg,mask)       ithWriteRegMaskA(reg,val,mask)
#define MAC_ReadReg(reg,pval)       (*pval)=ithReadRegA(reg)
#define MAC_VmemAlloc(align,size)   itpVmemAlloc(size)   /** already 64-bytes alignment */
#define MAC_VmemFree(x)             itpVmemFree((uint32_t)x)

/*
 * for sempahore
 */
#define MAC_SEM             sem_t*
#define MAC_CreateSem(x)    { x=malloc(sizeof(sem_t)); sem_init(x, 0, 1); }   
#define MAC_CreateSemLock(x)    { x=malloc(sizeof(sem_t)); sem_init(x, 0, 0); }   
#define MAC_WaitSem         sem_wait
#define MAC_ReleaseSem      sem_post
        
#define sleep(x)            usleep(x*1000)
#define udelay              ithDelay

#define GPIO_BASE			ITH_GPIO_BASE




#define unlikely
#define BUG()       do { LOG_ERROR "impossible path!!! %s:%s:%s \n", __FILE__, __FUNCTION__,__LINE__ LOG_END  while(1);   } while(0)

