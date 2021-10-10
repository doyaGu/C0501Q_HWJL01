#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "capture_s/ite_capture.h"
#include "isp/mmp_isp.h"   //for ISP
#include "ite/itv.h"       //for VideoInit()
#include "ite/ith_video.h" //for VideoInit()
#include "capture_s/capture_types.h"
static pthread_t tid;

bool stopTest;

#define CaptureDevName CFG_CAPTURE_MODULE_NAME
#define SecondCaptureDevName CFG_SECOND_CAPTURE_MODULE_NAME

typedef enum SEND_STATE_TAG
{
    SEND_BEGIN,
    SEND_START,
    SEND_STOP,
} SEND_STATE;

static void
VideoInit(
    void)
{
    ithVideoInit(NULL);
    itv_init();
}

static void
VideoExit(
    void)
{
    /* release dbuf & itv */
    itv_flush_dbuf();
    itv_deinit();

    /* release decoder stuff */
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    ithVideoExit();
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
}

void SettingISPAnd_FilpLCD(
    void)
{
    static int         New_state   = 0;
    static int         Old_state   = 0;
    static int         state_count = 0;
    static int         cap_idx     = 0;
    uint8_t            *dbuf       = NULL;
    ITV_DBUF_PROPERTY  dbufprop    = {0};
    ITE_CAP_VIDEO_INFO outdata     = {0};

    ithCaptureGetNewFrame(&outdata);

	dbuf = itv_get_dbuf_anchor();
    if (dbuf == NULL)
        return;

	if(outdata.IsInterlaced)
		itv_enable_isp_feature(MMP_ISP_DEINTERLACE); 

    dbufprop.src_w    = outdata.OutWidth;
    dbufprop.src_h    = outdata.OutHeight;
    dbufprop.pitch_y  = outdata.PitchY;
    dbufprop.pitch_uv = outdata.PitchUV;
    dbufprop.format   = MMP_ISP_IN_YUV422;
    dbufprop.ya       = outdata.DisplayAddrY;
    dbufprop.ua       = outdata.DisplayAddrU;
    dbufprop.va       = outdata.DisplayAddrV;
    //printf("dbufprop.ya=0x%x,dbufprop.ua=0x%x,dbufprop.va=0x%x,dbufprop.src_w=%d,dbufprop.src_h=%d,dbufprop.pitch_y=%d,dbufprop.pitch_uv=%d\n",dbufprop.ya,dbufprop.ua,dbufprop.va,dbufprop.src_w,dbufprop.src_h,dbufprop.pitch_y,dbufprop.pitch_uv);
    itv_update_dbuf_anchor(&dbufprop);

    return;
}

#ifndef WIN32
static void* DrawVideoSurface(void* arg)
{
	ITUSurface* lcdSurf;
    lcdSurf = ituGetDisplaySurface();

	while(!stopTest)
    {
        ituDrawVideoSurface(lcdSurf, 0, 0, ithLcdGetWidth(), ithLcdGetHeight());
        ituFlip(lcdSurf);
        usleep(20000);
    }
    pthread_exit(NULL);
}
#endif

void *TestFunc(void *arg)
{
    itpInit();
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);

    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);

    while (!stopTest)
    {
        struct  timeval     start,end;
        unsigned long       diff;
		uint16_t            bSignalStable = 0;
		CaptureModuleDriver IrSensor;
        SEND_STATE          gState = SEND_BEGIN;
        sleep(3);


#ifndef WIN32

		// init itu
		ituLcdInit();

#ifdef CFG_M2D_ENABLE
		ituM2dInit();
#ifdef CFG_VIDEO_ENABLE
		ituFrameFuncInit();
#endif // CFG_VIDEO_ENABLE
#else
		ituSWInit();
#endif // CFG_M2D_ENABLE
		
#if defined(CFG_BUILD_ITV) && !defined(CFG_TEST_VIDEO) 
		itv_set_pb_mode(1);
#endif
		pthread_create(&tid, NULL, DrawVideoSurface, NULL);
#else
		VideoInit();
#endif

        do
        {
            switch (gState)
            {
            case SEND_BEGIN:
                ithCapInitialize();
 
 				IrSensor = (CaptureModuleDriver)CaptureModuleDriver_GetDevice(CaptureDevName);
				ithCaptureSetModule(IrSensor);

				bSignalStable = ithCapDeviceIsSignalStable();
				if (!bSignalStable) printf("Capture device not stable!!\n");
 

                printf("ith9850CaptureRun\n");
                ithCapFire();
                gettimeofday(&start, NULL);
                gState = SEND_START;
                break;

            case SEND_START:
                SettingISPAnd_FilpLCD();
               // mmpDumpReg();
                gettimeofday(&end, NULL);
                diff = (end.tv_sec) - (start.tv_sec);
                if (diff >= 30)
                {
                    gState = SEND_STOP;
                }
                break;

            case SEND_STOP:
                printf("ith9850CaptureTerminate\n");
                ithCapTerminate();
                usleep(1000 * 1000 * 3);
                gState = SEND_BEGIN;
                break;
            }
        } while (!stopTest);
    }
    return NULL;
}
