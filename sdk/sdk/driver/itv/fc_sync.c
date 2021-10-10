#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> /* O_RDWR */

#include "fc_sync.h"
#include "itp.h"

int fc_init_sync(FC_STRC_SYNC *sync)
{
    int                    iret;
    const signed long long stime = 0LL;

    memset((void *)sync, 0, sizeof(FC_STRC_SYNC));

    sync->fd_stc      = open(":stc", 0, O_RDWR);
    if (-1 == sync->fd_stc)
        SERR();

    sync->sync_state  = FC_SYNC_STATE_OOS;
    sync->cover_audio = 0;

    iret              = write(sync->fd_stc, (char *)&stime, sizeof(signed long long));
    if (iret < 0)
        SERR();

    return 0;
}

int fc_deinit_sync(FC_STRC_SYNC *sync)
{
    sync->sync_state  = FC_SYNC_STATE_OOS;
    sync->cover_audio = 0;

    close(sync->fd_stc);
    memset((void *)sync, 0, sizeof(FC_STRC_SYNC));
    return 0;
}

void fc_sync_reset(FC_STRC_SYNC *sync)
{
    sync->sync_state  = FC_SYNC_STATE_OOS;
    sync->cover_audio = 0;
}

void fc_sync_settime(FC_STRC_SYNC *sync, signed long long stcval)
{
    int iret = write(sync->fd_stc, (char *)&stcval, sizeof(signed long long));
    if (iret < 0)
        SERR();

    sync->sync_state = FC_SYNC_STATE_SYNC;
}

void fc_sync_gettime(FC_STRC_SYNC *sync, signed long long *stcval)
{
    int iret;

    if (sync->sync_state == FC_SYNC_STATE_OOS)
        *stcval = -1LL;

    iret = read(sync->fd_stc, (char *)stcval, sizeof(signed long long));
    if (iret < 0)
        SERR();
}

void fc_sync_pause(FC_STRC_SYNC *sync)
{
    int iret = ioctl(sync->fd_stc, ITP_IOCTL_PAUSE, NULL);
    if (iret < 0)
        SERR();
}

int fc_sync_is_oos(FC_STRC_SYNC *sync)
{
    return (sync->sync_state == FC_SYNC_STATE_OOS);
}

void fc_sync_set_oos(FC_STRC_SYNC *sync)
{
    sync->sync_state = FC_SYNC_STATE_OOS;
}

void fc_sync_set_cover_audio(FC_STRC_SYNC *sync)
{
    sync->cover_audio = 1;
}

int fc_sync_is_cover_audio(FC_STRC_SYNC *sync)
{
    return sync->cover_audio;
}