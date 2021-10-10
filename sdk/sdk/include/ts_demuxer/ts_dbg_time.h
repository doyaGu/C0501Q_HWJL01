#ifndef __TS_DBG_TIME_H_UI7RUOLL_GKNY_SVHF_9BFP_1RYSKVA3F6NF__
#define __TS_DBG_TIME_H_UI7RUOLL_GKNY_SVHF_9BFP_1RYSKVA3F6NF__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <time.h>
//=============================================================================
//				  Constant Definition
//=============================================================================


//=============================================================================
//				  Macro Definition
//=============================================================================


//=============================================================================
//				  Structure Definition
//=============================================================================


//=============================================================================
//				  Global Data Definition
//=============================================================================


//=============================================================================
//				  Private Function Definition
//=============================================================================
static void
_measureTime(
    const char  *caller,
          char  *prefix)
{
#define MAX_MEASURE_TIME_CNT 10
    struct dbg_measure_time_info{
        bool        bUsed;
        char        caller[64];
        //uint32_t    startTime;
        struct timeval startTime;
    };
    static struct dbg_measure_time_info dbg_time_info[MAX_MEASURE_TIME_CNT] = {0};
    int           i;
        
    for(i=0; i<MAX_MEASURE_TIME_CNT; i++)
    {
        if( dbg_time_info[i].bUsed == true &&
            memcmp(dbg_time_info[i].caller, caller, strlen(caller)) == 0 )
        {
            struct  timeval currT;
            float   elapsedTime;
            
            // get duration
            if( prefix )    printf("%s", prefix);
            
            gettimeofday(&currT, NULL);
            elapsedTime = (currT.tv_sec - dbg_time_info[i].startTime.tv_sec) * 1000.0;      // sec to ms
            elapsedTime += ((currT.tv_usec - dbg_time_info[i].startTime.tv_usec) / 1000.0);   // us to ms 
            printf(" %f ms\n", elapsedTime);
            //printf(" %d ms\n", (clock() - dbg_time_info[i].startTime));
            memset(&dbg_time_info[i], 0x0, sizeof(struct dbg_measure_time_info));
            break;
        }
    }
    
    if( i == MAX_MEASURE_TIME_CNT )
    {
        // get start time
        for(i=0; i<MAX_MEASURE_TIME_CNT; i++)
        {
            if( dbg_time_info[i].bUsed == false )
            {
                dbg_time_info[i].bUsed = true;
                strncpy(dbg_time_info[i].caller, (char*)caller, 63);
                //dbg_time_info[i].startTime = clock();
                gettimeofday(&dbg_time_info[i].startTime, NULL);
                break;
            }
        }
    }

    return;
}

static void
tsd_dbg_get_clock(
    struct timeval *startT)
{
    gettimeofday(startT, NULL);
}

static uint32_t
tsd_dbg_get_duration(
    struct timeval *startT)
{
    struct timeval currT = {0};
    uint64_t  duration_time = 0;
    
    gettimeofday(&currT, NULL);
    duration_time = (currT.tv_sec - startT->tv_sec) * 1000;      // sec to ms
    duration_time += ((currT.tv_usec - startT->tv_usec) / 1000); // us to ms
    return (uint32_t)duration_time;
}

//=============================================================================
//				  Public Function Definition
//=============================================================================
#ifndef NDEBUG
    #ifdef _MSC_VER // WIN32
        #define ts_dbg_measureTime(caller, prefix, ...)        do{char tmp_txt[256] = {0}; \
                                                                  sprintf(tmp_txt, prefix, __VA_ARGS__); \
                                                                  _measureTime(caller, tmp_txt); \
                                                                 }while(0)
    #else
        #define ts_dbg_measureTime(caller, prefix, args...)    do{char tmp_txt[256] = {0}; \
                                                                  sprintf(tmp_txt, prefix, ## args); \
                                                                  _measureTime(caller, tmp_txt); \
                                                                 }while(0)
    #endif

#else // #ifndef NDEBUG
    #ifdef _MSC_VER // WIN32
        #define ts_dbg_measureTime(caller, prefix, ...)
    #else
        #define ts_dbg_measureTime(caller, prefix, args...) 
    #endif
#endif

#ifdef __cplusplus
}
#endif

#endif
