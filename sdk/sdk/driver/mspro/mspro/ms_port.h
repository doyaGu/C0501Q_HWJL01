#ifndef MS_PORT_H
#define MS_PORT_H

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
#define AHB_WriteRegister(reg,val)              ithWriteRegA(reg,val)
#define AHB_WriteRegisterMask(reg,val,mask)     ithWriteRegMaskA(reg,val,mask)
#define AHB_ReadRegister(reg,pval)              (*pval)=ithReadRegA(reg)

#define SYS_Malloc                      malloc     
#define SYS_Free						free
#define SYS_MemorySet					memset

/*
 * for event
 */
sem_t* ms_create_sem(int cnt);
#define MMP_EVENT                       sem_t*
#define SYS_CreateEvent()               ms_create_sem(0) 
#define SYS_WaitEvent                   itpSemWaitTimeout
#define SYS_SetEvent                    sem_post
#define SYS_DelEvent(a)                 do { sem_destroy(a); free(a); } while(0)
#define SYS_SetEventFromIsr				itpSemPostFromISR

        
#define MMP_Sleep(x)                    usleep(x*1000)


#ifdef __cplusplus
}
#endif

#endif


