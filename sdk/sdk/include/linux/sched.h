#ifndef SCHED_H
#define SCHED_H

#include <pthread.h>

#define sched_setscheduler(a,b,c)   pthread_setschedparam(a,b,c)

#endif // SCHED_H
