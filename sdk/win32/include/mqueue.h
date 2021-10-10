#ifndef ITP_MQUEUE_H
#define ITP_MQUEUE_H

#include <sys/types.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* message queue types */
typedef int mqd_t;

struct mq_attr {
  long mq_flags;    /* message queue flags */
  long mq_maxmsg;   /* maximum number of messages */
  long mq_msgsize;  /* maximum message size */
  long mq_curmsgs;  /* number of messages currently queued */
};

#define MQ_PRIO_MAX 16

/* prototypes */
mqd_t mq_open (const char *__name, int __oflag, ...);
int mq_close (mqd_t __msgid);
int mq_send (mqd_t __msgid, const char *__msg, size_t __msg_len, unsigned int __msg_prio);
int mq_timedsend(mqd_t msgid, const char *msg, size_t msg_len, unsigned msg_prio, const struct timespec *abs_timeout);
ssize_t mq_receive (mqd_t __msgid, char *__msg, size_t __msg_len, unsigned int *__msg_prio);
ssize_t mq_timedreceive(mqd_t msgid, char *msg, size_t msg_len, unsigned int *msg_prio, const struct timespec *abs_timeout);

#ifdef __cplusplus
}
#endif

#endif /* ITP_MQUEUE_H */
