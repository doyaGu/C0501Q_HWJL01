#ifndef ITP_MQUEUE_H
#define ITP_MQUEUE_H

#include_next <mqueue.h>

#ifdef __cplusplus
extern "C"
{
#endif

int mq_timedsend(mqd_t msgid, const char *msg, size_t msg_len, unsigned msg_prio, const struct timespec *abs_timeout);
ssize_t mq_timedreceive(mqd_t msgid, char *msg, size_t msg_len, unsigned int *msg_prio, const struct timespec *abs_timeout);

#ifdef __cplusplus
}
#endif

#endif // ITP_MQUEUE_H
