#include <pthread.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include "ite/itp.h"
#include "filelist.h"
#include "castor3player.h"
#include "ite/itv.h"
#include "ite/ith_video.h"
#include "ite/itu.h" //Benson
#include "SDL/SDL.h"

#include <math.h>
#include <mqueue.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <crtdbg.h>

#endif // _WIN32


/*
 * Global variables
 */
FILELIST  files;
MTAL_SPEC mtal_spec = {0};
bool      stopTest;
char Tfile[] = "a:/logo2.mp4";
static pthread_t tid;

typedef enum {
    PLAY_MODE_PLAY_ONCE,
    PLAY_MODE_REPEAT_ONE,
    PLAY_MODE_REPEAT_ALL
} PLAY_MODE;


typedef struct _VideoWindow {
    ITUSurface* lcdSurf;
    int x_pos;
    int y_pos;
    int width;
    int height; 
} VideoWindow;  //Benson


/*
 * Functions
 */
bool is_file_eof = false;

void EventHandler(PLAYER_EVENT nEventID, void *arg)
{
    switch (nEventID)
    {
    case PLAYER_EVENT_EOF:
        printf("YC: File EOF\n");
        is_file_eof = true;
        break;

    default:
        is_file_eof = true;
        break;
    }
}


static void* DrawVideoSurface(void* arg)
{
	ITUSurface* lcdSurf;
    lcdSurf = ituGetDisplaySurface();
	printf("run DrawVideoSurface!\n");

	while(!stopTest)
    {
        ituDrawVideoSurface(lcdSurf, 0, 0, ithLcdGetWidth(), ithLcdGetHeight());
        ituFlip(lcdSurf);
        usleep(20000);
    }
    pthread_exit(NULL);
}

void PlayVideo()
{
    mtal_pb_init(EventHandler);
    //strcpy(mtal_spec.srcname, CFG_PUBLIC_DRIVE ":/" BOOT_VIDEO);

	strcpy(mtal_spec.srcname, Tfile);
    printf("mtal_pb_select_file(%s)\n",Tfile);
    mtal_pb_select_file(&mtal_spec);
    mtal_pb_play();

    pthread_create(&tid, NULL, DrawVideoSurface, NULL);
}


static int play_nextfile(
    MTAL_SPEC *spec,
    PLAY_MODE mode)
{
    static int i      = -1;
    int        result = -1;

    for (++i; (mode != PLAY_MODE_PLAY_ONCE) || (i < MAX_FILE_INDEX); ++i)
    {
        switch (mode)
        {
        case PLAY_MODE_REPEAT_ONE:
            if ((i - 1) >= 0)
                --i;
            break;

        case PLAY_MODE_REPEAT_ALL:
            if (i >= MAX_FILE_INDEX)
                i = 0;
            break;
        }
        if (!files[i])
            continue;

        strcpy(mtal_spec.srcname, files[i]);
        printf("mtal_pb_select_file(%s)\n", files[i]);
        mtal_pb_select_file(&mtal_spec);
        mtal_pb_play();
        result = 0;
        break;
    }

    return result;
}

static void event_loop()
{
    int current_time = 0;
	pthread_t task;

    printf("*********************************\n");
    printf("*  iTE MediaPlayer demonstrate  *\n");
    printf("*********************************\n\n");
    printf("Please hit [MENU] to start, or select files\n\n");

	PlayVideo();
	while(1);
}

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

void *TestFunc(void *arg)
{
    // target board drivers init
    itpInit();
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);

    memset(&mtal_spec, 0, sizeof(MTAL_SPEC));

    sleep(8);

    files = filelist_init();
    if (NULL == files)
        goto fail;
#ifdef WIN32
    filelist_add_files_from_all_drives(files, "mkv:mkv;avi:avi;mp4:mp4;mov:mov;3gp:3gp");
#else
    filelist_add(files, "a:/release.mkv");
#endif

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

    event_loop();

    /* never return */
    filelist_deinit(files);

    itpExit();
fail:
    malloc_stats();
    return 0;
}
