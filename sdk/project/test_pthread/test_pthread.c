#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define MAX_THREAD 8
//=============================================================================
//				  Macro Definition
//=============================================================================
#define _mutex_init(mutex)              do{ if(!mutex){\
                                            pthread_mutex_init(&mutex, NULL);\
                                            printf(" mutex_init: %s, 0x%x\n", #mutex, mutex);}\
                                         }while(0)
#define _mutex_deinit(mutex)            do{ if(mutex){\
                                            pthread_mutex_destroy(&mutex);\
                                            mutex=0;printf(" mutex_de-init: %s, 0x%x\n", #mutex, mutex);}\
                                        }while(0)
#define _mutex_lock(mutex)              do{ if(mutex){\
                                            printf("  lock %s\n", __FUNCTION__);\
                                            pthread_mutex_lock(&mutex);}\
                                        }while(0)
#define _mutex_unlock(mutex)            do{ if(mutex){\
                                            pthread_mutex_unlock(&mutex);\
                                            printf("  unlock %s\n", __FUNCTION__);}\
                                        }while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef void* (*START_ROUTINE)(void*);

typedef struct
{
    int     id;
    int     nproc;
} parm;


//=============================================================================
//                  Global Data Definition
//=============================================================================
static char            message[100];	/* storage for message  */
static pthread_mutex_t msg_mutex = 0;//PTHREAD_MUTEX_INITIALIZER;
static int             token = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void *hello(void *arg)
{
    parm *p=(parm *)arg;
    printf("Hello from node %d\n", p->id);
    return (NULL);
}

static void* greeting(void *arg)
{
    parm    *p = (parm *) arg;
    int     id = p->id;
    int     i;

    if (id != 0)
    {
        /* Create message */
        while (1)
        {
            pthread_mutex_lock(&msg_mutex);
            if (token  == 0)
            {
                sprintf(message, "Greetings from process %d!", id);
                token++;
                pthread_mutex_unlock(&msg_mutex);
                break;
            }
            pthread_mutex_unlock(&msg_mutex);
            usleep(1000);
        }
        /* Use strlen+1 so that '\0' gets transmitted */
    } 
    else
    {                /* my_rank == 0 */
        for (i = 1; i < p->nproc; i++)
        {
            while (1)
            {
                pthread_mutex_lock(&msg_mutex);
                if (token == 1)
                {
                    printf("%s\n", message);
                    token--;
                    pthread_mutex_unlock(&msg_mutex);
                    break;
                }
                pthread_mutex_unlock(&msg_mutex);
                usleep(1000);
            }
        }
    }

    return NULL;
}


static void
Test_1(void)
{
    int             n,i;
    pthread_t       *threads;
    pthread_attr_t  pthread_custom_attr;
    parm            *p;
    struct  timeval currT;
    START_ROUTINE   act_routine;

    act_routine = greeting; // hello;

    itpInit();

    gettimeofday(&currT, NULL);
    srand((uint32_t)currT.tv_usec);

    n = (rand() % MAX_THREAD);

    n = (n) ? n : 1;
    printf(" set total %d threads \n", n);

    threads = (pthread_t *)malloc(n*sizeof(*threads));
    pthread_attr_init(&pthread_custom_attr);

    p = (parm *)malloc(sizeof(parm)*n);
    /* Start up thread */

    for (i=0; i < n; i++)
    {
        p[i].id=i;
        p[i].nproc = n;
        pthread_create(&threads[i], &pthread_custom_attr, act_routine, (void *)(p+i));
    }

    /* Synchronize the completion of each thread. */

    for (i=0; i<n; i++)
    {
        pthread_join(threads[i],NULL);
    }
    free(p);
}



static void *func1(void *dummy)
{
    uint32_t    x = 0;
    uint32_t    sleep_time = 0;
    uint32_t    time_out_cnt = 0; 
    static struct  timeval startT;
    struct  timeval currT;
    float   elapsedTime = 0; // ms  
    
    gettimeofday(&startT, NULL);
    time_out_cnt = ((rand() >> 4) & 0xfff);

    while ( elapsedTime < time_out_cnt )
    {
        gettimeofday(&currT, NULL);
        elapsedTime = (currT.tv_sec - startT.tv_sec) * 1000.0;      // sec to ms
        elapsedTime += ((currT.tv_usec - startT.tv_usec) / 1000.0);   // us to ms    
        
        _mutex_lock(msg_mutex);
        sleep_time = ((rand() >> 5) & 0x3ff);
        printf("  func_1: sleeping %d usec\n", sleep_time);
        usleep(sleep_time);

        printf("  func_1: x = %d -> %d\n", x, x+10);
        x += 10;
        _mutex_unlock(msg_mutex);
    }
    
    printf(" exit %s()\n", __func__);
    pthread_exit(NULL);
}

static void *func2(void *dummy)
{
    uint32_t    y = 0;
    uint32_t    sleep_time = 0;
    uint32_t    time_out_cnt = 0; 
    static struct  timeval startT;
    struct  timeval currT;
    float   elapsedTime = 0; // ms
    
    gettimeofday(&startT, NULL);
    time_out_cnt = ((rand() >> 4) & 0x1fff);

    while ( elapsedTime < time_out_cnt )
    {
        gettimeofday(&currT, NULL);
        elapsedTime = (currT.tv_sec - startT.tv_sec) * 1000.0;      // sec to ms
        elapsedTime += ((currT.tv_usec - startT.tv_usec) / 1000.0);   // us to ms    
        
        _mutex_lock(msg_mutex);
        sleep_time = ((rand() >> 2) & 0x3fff);
        printf("- func_2: sleeping %d usec\n", sleep_time);
        usleep(sleep_time);
        
        printf("- func_2: y = %d -> %d\n", y, y+1000);
        y += 1000;
        _mutex_unlock(msg_mutex);
    }
    
    printf(" exit %s()\n", __func__);
    pthread_exit(NULL);
}


static void 
Test_2()
{
    pthread_cond_t  termination_cond;
    pthread_mutex_t termination_mutex;
    pthread_t       threads_func1;
    pthread_t       threads_func2;

    struct  timeval currT;
    
    itpInit();

    usleep(1000000);
    gettimeofday(&currT, NULL);
    srand((uint32_t)(currT.tv_usec/1000));
    
    // _mutex_init(termination_mutex);
    // pthread_cond_init(&termination_cond, NULL);
    
    _mutex_init(msg_mutex);
    
    pthread_create(&threads_func1, 0, func1, 0);
    pthread_create(&threads_func2, 0, func2, 0);

    pthread_join(threads_func1, NULL);
    pthread_join(threads_func2, NULL);
    
    _mutex_deinit(msg_mutex);
    
	// _mutex_lock(termination_mutex);
	// pthread_cond_wait(&termination_cond, &termination_mutex);
	// _mutex_unlock(termination_mutex);
    while(1);
    
    exit(0);
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
void* TestFunc(void* arg)
{
    #ifdef CFG_DBG_TRACE
    uiTraceStart();
    #endif

    //Test_1();
    Test_2();
    
    #ifdef CFG_DBG_TRACE
    vTraceStop();
    #endif
    return 0;
}


