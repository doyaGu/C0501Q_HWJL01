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

/*
 * Global variables
 */
FILELIST  files;
MTAL_SPEC mtal_spec = {0};
bool      stopTest;

typedef enum {
    PLAY_MODE_PLAY_ONCE,
    PLAY_MODE_REPEAT_ONE,
    PLAY_MODE_REPEAT_ALL
} PLAY_MODE;

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

    printf("*********************************\n");
    printf("*  iTE MediaPlayer demonstrate  *\n");
    printf("*********************************\n\n");
    printf("Please hit [MENU] to start, or select files\n\n");

    if (mtal_pb_init(EventHandler) != 0)
        goto error;

    while (!stopTest && 0 == play_nextfile(&mtal_spec, PLAY_MODE_REPEAT_ALL))
    {
        while (!stopTest)
        {
            if (is_file_eof)
            {
                is_file_eof = false;
                mtal_pb_stop();
                if (0 != play_nextfile(&mtal_spec, PLAY_MODE_REPEAT_ALL))
                    stopTest = true;
            }

#if 0
            mtal_pb_get_current_time(&current_time);
            printf("YC0: current time = %d\n", current_time);
#endif            
            usleep(500000);
        }
        mtal_pb_stop();
    }

error:
    mtal_pb_exit();
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

    sleep(1);

    files = filelist_init();
    if (NULL == files)
        goto fail;
#ifdef WIN32
    filelist_add_files_from_all_drives(files, "mkv:mkv;avi:avi;mp4:mp4;mov:mov;3gp:3gp");
#else
    filelist_add(files, "a:/release.mkv");
#endif

    VideoInit();
    event_loop();
    VideoExit();

    /* never return */
    filelist_deinit(files);

    itpExit();
fail:
    malloc_stats();
    return 0;
}