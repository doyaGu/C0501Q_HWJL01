#ifndef ITP_PTHREAD_H
#define ITP_PTHREAD_H

#include_next<pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern int itpPthreadCreate(pthread_t* pthread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg, const char* name);

#ifndef pthread_create
#define pthread_create(pthread, attr, start_routine, arg) itpPthreadCreate((pthread), (attr), (start_routine), (arg), #start_routine)
#endif

#ifdef __cplusplus
}
#endif

#endif // ITP_PTHREAD_H
