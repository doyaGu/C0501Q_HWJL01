
#if defined(CFG_DEMOD_ENABLE)

#include <fcntl.h>
#include <sys/ioctl.h>
#include "ite/itp.h"
#include "../../../driver/itp/itp_demod_thread.h"
#include "IteAirTsStreamSource.h"

//#include "ite/itp_dbg.h"
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
#ifdef __cplusplus
    extern "C" 
    {
        TSAF_HANDLE  *g_pHTsaf[MAX_DEMOD_SUPPORT_CNT] = {0};
    }
#endif
//=============================================================================
//				  Private Function Definition
//=============================================================================


//=============================================================================
//				  Public Function Definition
//=============================================================================
int
IteAirTsStreamSource
::createNew_Probe(
    UsageEnvironment &env,
    int              demod_idx)
{
    int         result = -1;
    uint32_t    value = 0;
    char        serviceName[32] = {0};
    int         demodFd = -1;

    do{
        if( demod_idx < MAX_DEMOD_SUPPORT_CNT && !g_pHTsaf[demod_idx] )
        {
            printf(" demode init fail !! %s()[#%d]\n", __FUNCTION__, __LINE__);
            break;
        }

        result = 0;
    }while(0);

    return result;
}

IteAirTsStreamSource*
IteAirTsStreamSource
::createNew(
    UsageEnvironment &env,
    int              demod_idx,
    int              service_idx,
    uint32_t         preferredSampleSize)
{
    IteAirTsStreamSource *iteAirTsSource 
        = new IteAirTsStreamSource(env, demod_idx, service_idx, preferredSampleSize);

    return iteAirTsSource;
}

IteAirTsStreamSource
::IteAirTsStreamSource(
    UsageEnvironment &env,
    int              demod_idx,
    int              service_idx,
    uint32_t         preferredSampleSize)
: FramedSource(env), _pHTsaf(g_pHTsaf[demod_idx]), 
  _demod_idex(demod_idx), _service_idex(service_idx),
  _preferredSampleSize(preferredSampleSize), _lastPlayTime(0),
  _bLimitNumBytesToStream(False), _numBytesToStream(0)
{
    tsaf_Open(&_pHTsaf, service_idx, 0);
    return;
}

IteAirTsStreamSource
::~IteAirTsStreamSource()
{
    do{
        tsaf_Close(&_pHTsaf, 0);
    }while(0);
    
    return;
}

void IteAirTsStreamSource
::doGetNextFrame() 
{
#define GET_TS_TIME_OUT     1000 // ms

    do{
        struct timeval startT;
        uint64_t duration_time = 0;
        uint32_t realSize = 0;
        
        if( _preferredSampleSize > 0 && _preferredSampleSize < fMaxSize )
        {
            fMaxSize = _preferredSampleSize;
        }
        
        tsaf_Read(&_pHTsaf, fTo, fMaxSize, &realSize, 0);
        fFrameSize = (unsigned)realSize;

        gettimeofday(&startT, NULL);
#if 0
        while( fFrameSize <= 0 && duration_time < GET_TS_TIME_OUT )
        {
            struct timeval currT;
            
            gettimeofday(&currT, NULL);
            duration_time = (currT.tv_sec - startT.tv_sec) * 1000;      // sec to ms
            duration_time += ((currT.tv_usec - startT.tv_usec) / 1000); // us to ms
            tsaf_Read(&_pHTsaf, fTo, fMaxSize, &realSize, 0);
            fFrameSize = (unsigned)realSize;
        }
        
        if( duration_time >= GET_TS_TIME_OUT )
        {
            printf(" read ts stream fail (size=%d)!! %s[#%d]\n", fFrameSize, __FUNCTION__, __LINE__);
            handleClosure(this);
            break;
        }
#endif
        // Set the 'presentation time':
        if( _playTimePerFrame > 0 && _preferredSampleSize > 0 )
        {
            fPresentationTime.tv_sec = 0;
            fPresentationTime.tv_usec = 0;
            //if( fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0 )
            //{
            //    // This is the first frame, so use the current time:
            //    gettimeofday(&fPresentationTime, NULL);
            //}
            //else
            //{
            //    // Increment by the play time of the previous data:
            //    unsigned uSeconds = fPresentationTime.tv_usec + _lastPlayTime;
            //    fPresentationTime.tv_sec += uSeconds / 1000000;
            //    fPresentationTime.tv_usec = uSeconds % 1000000;
            //}
            //
            //// Remember the play time of this data:
            //_lastPlayTime = (_playTimePerFrame * fFrameSize) / _preferredSampleSize;
            //fDurationInMicroseconds = _lastPlayTime;
        }
        else
        {
            // We don't know a specific play time duration for this data,
            // so just record the current time as being the 'presentation time':
            fPresentationTime.tv_sec = 0;
            fPresentationTime.tv_usec = 0;            
            //gettimeofday(&fPresentationTime, NULL);
        }
        fDurationInMicroseconds = 0;
        // Inform the downstream object that it has data:
        FramedSource::afterGetting(this);
    }while(0);
    
    return;
}
#endif
