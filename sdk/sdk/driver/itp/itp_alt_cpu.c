/** @file
 * PAL ALT_CPU functions.
 *
 * @author Steven Hsiao
 * @version 1.0
 * @date 2017/10/16
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#include <errno.h>
#include <sys/socket.h>
#include "openrtos/FreeRTOS.h"
#include "alt_cpu/alt_cpu_device.h"


#ifdef CFG_RSL_MASTER
extern ITPDevice itpDeviceRslMaster;
#endif
#ifdef CFG_RSL_SLAVE
extern ITPDevice itpDeviceRslSlave;
#endif
#ifdef CFG_SW_PWM
extern ITPDevice itpDeviceSwPwm;
#endif

typedef struct
{
    int         engineMode;
    ITPDevice*  ptDevice;
} ALT_CPU_ENGINE;


static pthread_mutex_t gAltCpuMutex  = PTHREAD_MUTEX_INITIALIZER;
static ALT_CPU_ENGINE gtCurDevice = { 0 };

static ALT_CPU_ENGINE gptAltCpuEngineArray[] =
{
#ifdef CFG_RSL_MASTER
    {ALT_CPU_RSL_MASTER, &itpDeviceRslMaster},
#endif
#ifdef CFG_RSL_SLAVE
    {ALT_CPU_RSL_SLAVE, &itpDeviceRslSlave},
#endif
#ifdef CFG_SW_PWM
    {ALT_CPU_SW_PWM, &itpDeviceSwPwm},
#endif
};

static int altCpuIoctl(int file, unsigned long request, void *ptr, void *info)
{
    int i = 0;
    int newEngineMode = 0;
    ITPDevice *pNewDevice = NULL;
    int ret = 0;
    
    pthread_mutex_lock(&gAltCpuMutex);
    switch (request)
    {
    	case ITP_IOCTL_ALT_CPU_SWITCH_ENG:
    	    //Engine is running, therefore, need to reload new engine image.
    	    newEngineMode = *(int*)ptr;
            if (gtCurDevice.engineMode == ALT_CPU_UNKNOWN_DEVICE || gtCurDevice.engineMode != newEngineMode)
            {
                iteRiscResetCpu(ALT_CPU);
                for (i = 0; i < sizeof(gptAltCpuEngineArray) / sizeof(ALT_CPU_ENGINE); i++)
                {
                    if (newEngineMode == gptAltCpuEngineArray[i].engineMode)
                    {
                        pNewDevice = gptAltCpuEngineArray[i].ptDevice;
                        gtCurDevice.ptDevice = pNewDevice;
                        break;
                    }
                }

                if (i == sizeof(gptAltCpuEngineArray) / sizeof(ALT_CPU_ENGINE))
                {
                    printf("itp_alt_cpu.c(%d), requested ALT CPU engine is not exited\n", __LINE__);
                }
            }
    		break;
        default:
            ret = gtCurDevice.ptDevice->ioctl(file, request, ptr, info);
            break;
    }    
    pthread_mutex_unlock(&gAltCpuMutex);
    return ret;
}

static int altCpuRead(int file, char *ptr, int len, void* info)
{
    int ret = 0;
    if (gtCurDevice.ptDevice)
    {
        pthread_mutex_lock(&gAltCpuMutex);
        ret =  gtCurDevice.ptDevice->read(file, ptr, len, info);
        pthread_mutex_unlock(&gAltCpuMutex);
    }
    return ret;
}

static int altCpuWrite(int file, char *ptr, int len, void* info)
{
    int ret = 0;
    if (gtCurDevice.ptDevice)
    {
        pthread_mutex_lock(&gAltCpuMutex);
        ret =  gtCurDevice.ptDevice->write(file, ptr, len, info);
        pthread_mutex_unlock(&gAltCpuMutex);
    }
    return ret;
}

static int altCpuSeek(int file, int ptr, int dir, void *info)
{
    int ret = 0;

    if (gtCurDevice.ptDevice)
    {
        pthread_mutex_lock(&gAltCpuMutex);
        ret =  gtCurDevice.ptDevice->lseek(file, ptr, dir, info);
        pthread_mutex_unlock(&gAltCpuMutex);
    }
    return ret;
}

const ITPDevice itpDeviceAltCpu =
{
    ":altcpu",
    itpOpenDefault,
    itpCloseDefault,
    altCpuRead,
    altCpuWrite,
    altCpuSeek,
    altCpuIoctl,
    NULL
};
