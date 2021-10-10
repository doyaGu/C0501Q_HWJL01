
/* Include the SDL main definition header */
#include "SDL_main.h"
#include <sys/ioctl.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "ite/itp.h"
#include "ite/ite_risc.h"

#define MAIN_STACK_SIZE 100000

static void* MainTask(void* arg)
{
    char *argv[2];

    // init pal
    itpInit();

    argv[0] = "SDL_app";
    argv[1] = NULL;

    SDL_main(1, argv);
}

#ifdef main
#undef main
int
main(int argc, char *argv[])
{
    pthread_t task;
    pthread_attr_t attr;
  
#ifdef CFG_CHIP_PKG_IT9910    
    {
    	// chip warm up
         uint16_t commandReg;
         uint32_t cnt = 10;
         uint32_t cnt1 = 0;
         uint32_t i;           
         
    #ifdef CFG_WATCHDOG_ENABLE
         itpRegisterDevice(ITP_DEVICE_WATCHDOG, &itpDeviceWatchDog);
         ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_INIT, NULL);
         ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
    #endif
    
         for(i=0; i<55000*80; i++) asm("");
         
         while (cnt-- != 0)
         {
             commandReg  = ithReadRegH(0x34a);
             printf("Rd %x\n", commandReg);
             if ((commandReg & 0xF) == 0xE)
                 cnt1++;
         }
         
         // read dram status
         commandReg  = ithReadRegH(0x34a);
                  
         if (((commandReg & 0xF) == 0x0) || ((commandReg & 0xF) == 0x1) || ((commandReg & 0xF) == 0x8))
         {
             commandReg = ithReadRegH(0x340) & 0x10;
             commandReg = commandReg ? 0x2a44 : 0x2a54;             
             
             ithWriteRegH(0x340, commandReg);
             
             for(i=0; i<30000; i++) asm("");
             printf("Update 0x340 = %x\n", commandReg);
         }
         
         commandReg  = ithReadRegH(0x3a6);
         printf("Mem Addr (0x3a6) %x\n", commandReg);
         
         commandReg  = ithReadRegH(0x340);
         printf("Mem Addr (0x340) %x\n", commandReg);
         
         commandReg  = ithReadRegH(0x342);
         printf("Mem Addr (0x342) %x\n", commandReg);
         
         commandReg  = ithReadRegH(0x344);
         printf("Mem Addr (0x344) %x\n", commandReg);
         
         commandReg  = ithReadRegH(0x346);
         printf("Mem Addr (0x346) %x\n", commandReg);
         
         commandReg  = ithReadRegH(0x348);
         printf("Mem Addr (0x348) %x\n", commandReg);
         
         commandReg  = ithReadRegH(0x34a);
         printf("Mem Addr (0x34a) %x\n", commandReg);
         
         commandReg  = ithReadRegH(0x34e);
         printf("Mem Addr (0x34e) %x\n", commandReg);
         
         ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
    }
#endif // CFG_CHIP_PKG_IT9910
    
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, MAIN_STACK_SIZE);
    pthread_create(&task, &attr, MainTask, NULL);

    /* Now all the tasks have been started - start the scheduler. */
    vTaskStartScheduler();

    /* Should never reach here! */
    return 0;

}
#endif

/* vi: set ts=4 sw=4 expandtab: */
