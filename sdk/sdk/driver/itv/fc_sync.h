#ifndef FC_SYNC_H
#define FC_SYNC_H

#define FC_SYNC_STATE_OOS   0
#define FC_SYNC_STATE_SYNC  1


#define SERR() do { printf("ERROR# %s:%d, %s\n", __FILE__, __LINE__, __func__); while(1); } while(0)
#define S()    do { printf("=> %s:%d, %s\n",     __FILE__, __LINE__, __func__);  


typedef struct
{
    int fd_stc;

    int sync_state;
    int cover_audio;
} FC_STRC_SYNC;

int fc_init_sync(FC_STRC_SYNC *sync);
int fc_deinit_sync(FC_STRC_SYNC *sync);
void fc_sync_reset(FC_STRC_SYNC *sync);
void fc_sync_settime(FC_STRC_SYNC *sync, signed long long stcval);
void fc_sync_gettime(FC_STRC_SYNC *sync, signed long long *stcval);
void fc_sync_pause(FC_STRC_SYNC *sync);


int fc_sync_is_oos(FC_STRC_SYNC *sync);
void fc_sync_set_oos(FC_STRC_SYNC *sync);

void fc_sync_set_cover_audio(FC_STRC_SYNC *sync);
int fc_sync_is_cover_audio(FC_STRC_SYNC *sync);

#endif //FC_SYNC_H

