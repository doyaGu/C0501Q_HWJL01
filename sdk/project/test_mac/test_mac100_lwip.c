#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "ite/ith.h"
#include "ite/itp.h"
#include "ite/ite_mac.h"

//#define WOL_TEST

#define TEST_STACK_SIZE 102400

#if defined(WOL_TEST)

static timer_t timer;
static struct itimerspec timerspec;
static sem_t sem;
static bool wait = false;


static void WolIntrHandler(unsigned int pin, void *arg)
{
    ithPrintf("\n\n gpio %d intr.. \n\n", pin);
   
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_OFF, (void*)ITP_PHY_WOL);
    ithGpioClearIntr(pin);
    ithGpioDisableIntr(pin);
    ithPrintf(" leave isr.. \n");
    itpSemPostFromISR(&sem);
}

static void* TimerStartWol(void* arg)
{
    printf("\n prepare enter WOL state... \n");
    iteMacSuspend();
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_ON, (void*)ITP_PHY_WOL);
    ithGpioRegisterIntrHandler(CFG_GPIO_ETHERNET_WAKE_ON_LAN, WolIntrHandler, NULL);
    wait = true;
    return NULL;
}
#endif


void* TestFunc(void* arg)
{
    pthread_t task;
    pthread_attr_t attr;
    ITPEthernetSetting setting = {0};
    
    // init pal
    itpInit();

#if defined(WOL_TEST)
    sem_init(&sem, 0, 0);
    /* start timer for wol setting */
    timer_create(CLOCK_REALTIME, NULL, &timer);
    timer_connect(timer, (VOIDFUNCPTR)TimerStartWol, 0);
    timerspec.it_value.tv_sec = 10;  /* 10 second */
    timerspec.it_value.tv_nsec = 0; 
#endif

    setting.dhcp = 0;
    // ipaddr
    setting.ipaddr[0] = 192;
    setting.ipaddr[1] = 168;
    setting.ipaddr[2] = 1;
    setting.ipaddr[3] = 1;
    // netmask
    setting.netmask[0] = 255;
    setting.netmask[1] = 255;
    setting.netmask[2] = 255;
    setting.netmask[3] = 0;
    // gateway
    setting.gw[0] = 192;
    setting.gw[1] = 168;;
    setting.gw[2] = 1;
    setting.gw[3] = 254;
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting);
    
    // do the test
    for (;;)
    {
#if defined(WOL_TEST)
        wait = false;
        printf(" normal state..... 10 second! \n");
        /* start timer for wol setting */
        timer_settime(timer, 0, &timerspec, NULL);
        while(wait==false) usleep(100*1000);
        /* wail wol event */
        printf(" wait WOL event... \n");
        sem_wait(&sem);
        printf(" receive WOL event... \n");
        iteMacResume();
#else
        sleep(1);
#endif
    }
    
    return NULL;
}

