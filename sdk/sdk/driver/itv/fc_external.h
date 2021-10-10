#ifndef FC_EXTERNAL_H
#define FC_EXTERNAL_H

#include <stdint.h>

typedef struct
{
    int     regression_idx; /* debug */

    void    *ffmpeg_avout;
    void    *fc_rcs;

    /* RMI */
    uint8_t *pbase[32][3];

    /* additional codec info */
    int     is_sorenson_h263;

    /* Evan, information for avc */
    int     insCnt;
    //int mode;
    //int frame_buf_cnt;
} ITV_OPAQUE_DATA;

#endif //FC_EXTERNAL_H
