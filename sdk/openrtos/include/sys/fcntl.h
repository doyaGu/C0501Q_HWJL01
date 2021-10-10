#ifndef ITP_SYS_FCNTL_H
#define ITP_SYS_FCNTL_H

#include_next <sys/fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int lwip_fcntl(int s, int cmd, int val);

static inline int itpFcntl(int s, int cmd, int val)
{
#ifdef CFG_NET_ENABLE
    if ((s & (0xFF << 8)) == 0)
    {
        return lwip_fcntl(s, cmd, val);
    }
    else
#endif // CFG_NET_ENABLE    
    {
        return 0;
    }
}

#define fcntl itpFcntl

#ifdef __cplusplus
}
#endif

#endif // ITP_SYS_FCNTL_H
