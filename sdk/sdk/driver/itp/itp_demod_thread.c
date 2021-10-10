#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include "itp_demod_thread.h"
#include "ite/itp.h"
#include "ts_demuxer/ite_ts_demuxer.h"

#if defined(CFG_ISDB_ENABLE)
    #include "tsi/mmp_tsi.h"
#endif


#define TSI_L2_BUFFER_SIZE          (8 * 1024 * 1024)
#define TSI_RECEIVE_TIME_DELAY      300000

#define TSI_TIMER_GAP           30

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct _TSI_L2_BUF_MGR_TAG
{
    bool                bSkip;

    pthread_mutex_t     tsiThreadStopMutex;
    bool                bStartTsiThread;
    pthread_mutex_t     tsiCacheStopMutex;
    bool                bStartTsiCache;

    int                 tsi_L2_idx;

    pthread_mutex_t     tsiBufMutex;
    uint32_t            tsiBuf_RPtr;
    uint32_t            tsiBuf_WPtr;
    uint8_t             *pTsiL2Buf;

    struct timeval      lastT;

}TSI_L2_BUF_MGR;

//=============================================================================
//                Global Data Definition
//=============================================================================
static TSI_L2_BUF_MGR   g_tsiBufMgr[ITP_MAX_DEMOD_SUPPORT_CNT] = {0};

//=============================================================================
//                Macro Definition
//=============================================================================
#if 0
    #define _trace_enter()            printf("enter: %s()[#%d]\n", __FUNCTION__, __LINE__)
    #define _trace_leave()            printf("leave: %s()[#%d]\n", __FUNCTION__, __LINE__)
#else
    #define _trace_enter()
    #define _trace_leave()
#endif
//=============================================================================
//                Private Function Definition
//=============================================================================

// Private Funtion
uint32_t
_TsiBufferGetValidSize(
    uint32_t   index)
{
    uint32_t    validSize = 0;

    _mutex_lock(g_tsiBufMgr[index].tsiBufMutex);

    if( g_tsiBufMgr[index].tsiBuf_WPtr >= g_tsiBufMgr[index].tsiBuf_RPtr )
    {
        validSize = g_tsiBufMgr[index].tsiBuf_WPtr - g_tsiBufMgr[index].tsiBuf_RPtr;
    }
    else
    {
        validSize = (TSI_L2_BUFFER_SIZE - g_tsiBufMgr[index].tsiBuf_RPtr) + g_tsiBufMgr[index].tsiBuf_WPtr;
    }

    _mutex_unlock(g_tsiBufMgr[index].tsiBufMutex);

    return validSize;
}


static int
_TsiThreadGetCacheStatus(
    uint32_t    index)
{
    int status = 0;

    _mutex_lock(g_tsiBufMgr[index].tsiCacheStopMutex);
    status = g_tsiBufMgr[index].bStartTsiCache;
    _mutex_unlock(g_tsiBufMgr[index].tsiCacheStopMutex);

    return status;
}

static int
_TsiThreadGetThreadStatus(
    uint32_t    index)
{
    int status = 0;

    _mutex_lock(g_tsiBufMgr[index].tsiThreadStopMutex);
    status = g_tsiBufMgr[index].bStartTsiThread;
    _mutex_unlock(g_tsiBufMgr[index].tsiThreadStopMutex);

    return status;
}
//=============================================================================
//                Public Function Definition
//=============================================================================

// Public Funtion
void *
TsiBufferThread(
    void *args)
{
    TSI_THREAD_PARAM    *pTsiThreadParam = (TSI_THREAD_PARAM*)args;
    const int           tsi_L2_idx = pTsiThreadParam->idx;
    TSD_HANDLE          *pHTsd = 0;
    int                 i;

    _trace_enter();

    printf("\n\ttsi_L2_idx = %d\n", tsi_L2_idx);

    _mutex_init(g_tsiBufMgr[tsi_L2_idx].tsiThreadStopMutex);
    _mutex_init(g_tsiBufMgr[tsi_L2_idx].tsiCacheStopMutex);
    _mutex_init(g_tsiBufMgr[tsi_L2_idx].tsiBufMutex);

    g_tsiBufMgr[tsi_L2_idx].bStartTsiThread = true;

    pHTsd = (TSD_HANDLE*)pTsiThreadParam->handle;
    printf("\tpHTsd[%d]=0x%x, handle=0x%x\n", tsi_L2_idx, pHTsd, pTsiThreadParam->handle);

    if( g_tsiBufMgr[tsi_L2_idx].pTsiL2Buf )
    {
        free(g_tsiBufMgr[tsi_L2_idx].pTsiL2Buf);
        g_tsiBufMgr[tsi_L2_idx].pTsiL2Buf = NULL;
    }

    if ( !(g_tsiBufMgr[tsi_L2_idx].pTsiL2Buf = (uint8_t *)malloc(TSI_L2_BUFFER_SIZE)) )
        printf(" tsi(%d-th) L2 buf alloc fail !! %s", tsi_L2_idx, __FUNCTION__);

    while( _TsiThreadGetThreadStatus(tsi_L2_idx) )
    {
        //static struct timeval lastT = {0};
        struct timeval currT;
        double elapsedTime;
        int    timeOut = {0};

        if( g_tsiBufMgr[tsi_L2_idx].lastT.tv_sec == 0 )
            gettimeofday(&g_tsiBufMgr[tsi_L2_idx].lastT, NULL);

        gettimeofday(&currT, NULL);

        elapsedTime = (currT.tv_sec - g_tsiBufMgr[tsi_L2_idx].lastT.tv_sec) * 1000.0;      // sec to ms
        elapsedTime += ((currT.tv_usec - g_tsiBufMgr[tsi_L2_idx].lastT.tv_usec) / 1000.0);   // us to ms

        if( elapsedTime > TSI_TIMER_GAP )
        {
            memcpy(&g_tsiBufMgr[tsi_L2_idx].lastT, &currT, sizeof(struct timeval));

            while( _TsiThreadGetCacheStatus(tsi_L2_idx) )
            {
                TSD_SAMPLE_INFO sampleInfo = {0};

                if( timeOut > 50 )
                {
                    printf(" %d-th tsi get sample timeout at %s()\n", tsi_L2_idx, __FUNCTION__);
                    continue;
                }

                _mutex_lock(g_tsiBufMgr[tsi_L2_idx].tsiBufMutex);

                if( g_tsiBufMgr[tsi_L2_idx].bSkip == false )
                {

                #if defined(CFG_ISDB_ENABLE)
                    // Read from TSI
                    mmpTsiReceive(tsi_L2_idx, &sampleInfo.sampleAddr, &sampleInfo.sampleSize);
                #else
                    // Get sample
                    sampleInfo.bShareBuf = true;

                    tsd_Get_Sample(pHTsd, TSD_SAMPLE_TS, &sampleInfo, 0);
                #endif

                    if( sampleInfo.sampleSize > 0 )
                    {
                        unsigned int remainderSize = 0;

                        timeOut = 0;

                        printf("sampleSize[%d]=%d\n", tsi_L2_idx, sampleInfo.sampleSize);
                        remainderSize = (TSI_L2_BUFFER_SIZE - g_tsiBufMgr[tsi_L2_idx].tsiBuf_WPtr);

                        //----------------------------------
                        // Copy to L2 buffer
                        if ( sampleInfo.sampleSize <= remainderSize )
                        {
                            memcpy(g_tsiBufMgr[tsi_L2_idx].pTsiL2Buf + g_tsiBufMgr[tsi_L2_idx].tsiBuf_WPtr, sampleInfo.sampleAddr, sampleInfo.sampleSize);
                            g_tsiBufMgr[tsi_L2_idx].tsiBuf_WPtr += sampleInfo.sampleSize;
                        }
                        else
                        {
                            memcpy(g_tsiBufMgr[tsi_L2_idx].pTsiL2Buf + g_tsiBufMgr[tsi_L2_idx].tsiBuf_WPtr, sampleInfo.sampleAddr, remainderSize);
                            g_tsiBufMgr[tsi_L2_idx].tsiBuf_WPtr = 0;
                            //printf("\n\t......g_TsiBufferWritePtr[%d] = 0\n", i);
                            memcpy(g_tsiBufMgr[tsi_L2_idx].pTsiL2Buf + g_tsiBufMgr[tsi_L2_idx].tsiBuf_WPtr, sampleInfo.sampleAddr + remainderSize, sampleInfo.sampleSize - remainderSize);
                            g_tsiBufMgr[tsi_L2_idx].tsiBuf_WPtr += (sampleInfo.sampleSize - remainderSize);
                        }
                    }
                    else
                    {
                        timeOut++;
                    }
                }
                else
                    timeOut = 0;

                _mutex_unlock(g_tsiBufMgr[tsi_L2_idx].tsiBufMutex);

                usleep(TSI_RECEIVE_TIME_DELAY);
            }
        }
        else
        {
            usleep(20000);
        }
    }

    if( g_tsiBufMgr[tsi_L2_idx].pTsiL2Buf )
    {
        free(g_tsiBufMgr[tsi_L2_idx].pTsiL2Buf);
        g_tsiBufMgr[tsi_L2_idx].pTsiL2Buf = NULL;
    }

    _trace_leave();
    _mutex_deinit(g_tsiBufMgr[tsi_L2_idx].tsiThreadStopMutex);
    _mutex_deinit(g_tsiBufMgr[tsi_L2_idx].tsiCacheStopMutex);
    _mutex_deinit(g_tsiBufMgr[tsi_L2_idx].tsiBufMutex);
    pthread_exit(NULL);
    return 0;
}

int TsiStopThread(uint32_t  index)
{
    _trace_enter();

    _mutex_lock(g_tsiBufMgr[index].tsiThreadStopMutex);
    g_tsiBufMgr[index].bStartTsiThread = false;
    _mutex_unlock(g_tsiBufMgr[index].tsiThreadStopMutex);

    _trace_leave();
    return 0;
}

int TsiStartCache(uint32_t  index)
{
    _trace_enter();

    _mutex_lock(g_tsiBufMgr[index].tsiCacheStopMutex);
    g_tsiBufMgr[index].bStartTsiCache = true;
    _mutex_unlock(g_tsiBufMgr[index].tsiCacheStopMutex);

    _trace_leave();
    return 0;
}

int TsiStopCache(uint32_t  index)
{
    _trace_enter();

    _mutex_lock(g_tsiBufMgr[index].tsiCacheStopMutex);
    g_tsiBufMgr[index].bStartTsiCache = false;
    _mutex_unlock(g_tsiBufMgr[index].tsiCacheStopMutex);

    _mutex_lock(g_tsiBufMgr[index].tsiBufMutex);
    g_tsiBufMgr[index].tsiBuf_RPtr = g_tsiBufMgr[index].tsiBuf_WPtr = 0;
    _mutex_unlock(g_tsiBufMgr[index].tsiBufMutex);

    _trace_leave();
    return 0;
}

int
TsiReadBuffer(
    uint32_t     index,
    uint8_t      *buffer,
    unsigned int bufferLength)
{
#define TSI_READ_BUF_TIMEOUT    15000
    int     ret = 0;
    struct timeval lastT = {0};
    struct timeval currT;
    double elapsedTime;

    _trace_enter();

    gettimeofday(&lastT, NULL);

    while(1)
    {
        unsigned int validSize = 0;

        if ( _TsiThreadGetCacheStatus(index) == 0 )
        {
            ret = 1;
            break;
        }

        if( g_tsiBufMgr[index].bSkip == true )
            break;

        validSize = _TsiBufferGetValidSize(index);

        if ( validSize >= bufferLength )
        {
            unsigned int ReadRemainderSize = 0;

            _mutex_lock(g_tsiBufMgr[index].tsiBufMutex);

            ReadRemainderSize = (TSI_L2_BUFFER_SIZE - g_tsiBufMgr[index].tsiBuf_RPtr);

            if ( ReadRemainderSize < bufferLength )
            {
                memcpy(buffer, g_tsiBufMgr[index].pTsiL2Buf + g_tsiBufMgr[index].tsiBuf_RPtr, ReadRemainderSize);
                g_tsiBufMgr[index].tsiBuf_RPtr = 0;

                memcpy(buffer + ReadRemainderSize, g_tsiBufMgr[index].pTsiL2Buf + g_tsiBufMgr[index].tsiBuf_RPtr, bufferLength - ReadRemainderSize);
                g_tsiBufMgr[index].tsiBuf_RPtr += (bufferLength - ReadRemainderSize);
            }
            else
            {
                memcpy(buffer, g_tsiBufMgr[index].pTsiL2Buf + g_tsiBufMgr[index].tsiBuf_RPtr, bufferLength);
                g_tsiBufMgr[index].tsiBuf_RPtr += bufferLength;
            }

            _mutex_unlock(g_tsiBufMgr[index].tsiBufMutex);

            ret = 0;
            break;
        }
        else
        {
            gettimeofday(&currT, NULL);

            elapsedTime = (currT.tv_sec - lastT.tv_sec) * 1000.0;      // sec to ms
            elapsedTime += ((currT.tv_usec - lastT.tv_usec) / 1000.0);   // us to ms
            if( elapsedTime > TSI_READ_BUF_TIMEOUT )
            {
                memcpy(&lastT, &currT, sizeof(struct timeval));
                printf("\t\tvalidSize = %d, L2 read time out (%f sec)\n", validSize, TSI_READ_BUF_TIMEOUT/1000.0);
                ret = 1;
                break;
            }
            //usleep(1000);
        }
    }

    _trace_leave();
    return ret;
}

int
TsiSkipBuffer(
    uint32_t    tsi_id,
    bool        bSkip)
{
    g_tsiBufMgr[tsi_id].bSkip = bSkip;
    printf("\n\tset %d-th tsi skip(%s) in L2 !!!\n", tsi_id, (g_tsiBufMgr[tsi_id].bSkip? "true" : "false"));
    return 0;
}


