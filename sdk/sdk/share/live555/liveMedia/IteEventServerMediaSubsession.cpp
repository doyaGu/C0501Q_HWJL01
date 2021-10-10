#include <time.h>
#include "GroupsockHelper.hh"
#include "RTSPCommon.hh"
#include "IteEventServerMediaSubsession.h"
#include "ts_airfile/ite_ts_airfile.h"
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
#define MAX_SERVICE_PER_DEMOD   4

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct _DEMOD_PRIV_INFO_TAG
{
    const char          *name;
    bool  bServiceUsed[MAX_SERVICE_PER_DEMOD];
} DEMOD_PRIV_INFO;

//=============================================================================
//                Global Data Definition
//=============================================================================
/**
 * Define URL
 * rtsp://xxx.xxx.xxx.xxx/channel_0.meta
 **/

static DEMOD_PRIV_INFO demodPrivDef[] =
{
#if (MAX_DEMOD_SUPPORT_CNT > 0)
	{"channel_0", false, false, false, false},
#endif
#if (MAX_DEMOD_SUPPORT_CNT > 1)
	{"channel_1", false, false, false, false},
#endif
#if (MAX_DEMOD_SUPPORT_CNT > 2)
	{"channel_2", false, false, false, false},
#endif
#if (MAX_DEMOD_SUPPORT_CNT > 3)
	{"channel_3", false, false, false, false},
#endif
};

#ifdef __cplusplus
    extern "C"
    {
        bool g_MotionEvent[MAX_DEMOD_SUPPORT_CNT] = {true, true, true, true};
    }
#endif
//=============================================================================
//                Private Function Definition
//=============================================================================


//=============================================================================
//                Public Function Definition
//=============================================================================
////////// IteEventServerMediaSubsession //////////

IteEventServerMediaSubsession *
IteEventServerMediaSubsession
::createNew(
    UsageEnvironment &env,
    char const *dataFileName)
{
    unsigned    i, demod_idx;
    char        service_name[16] = {0};
    unsigned    service_idx = (-1);
    char        *pInName = (char *)dataFileName;
    char        *pSubname = 0, *pSubnameL2 = 0;

    // tsi_enable
    // itp_demode open file => change service
    pSubname = (strchr(pInName, '.') + 1);
    pSubnameL2 = strchr(pSubname + 1, '.');
    strncpy(service_name, pSubname, (size_t)(pSubnameL2 - pSubname));
    service_idx = atoi(service_name);
    
    for(i = 0; i < MAX_DEMOD_SUPPORT_CNT; i++)
    {
        if( strncmp(dataFileName, demodPrivDef[i].name, strlen(demodPrivDef[i].name)) == 0 )
        {
            if( demodPrivDef[i].bServiceUsed[service_idx] == false )
            {
                demodPrivDef[i].bServiceUsed[service_idx] = true;
                break;
            }
            else
            {
                printf("channel: %u, service: %u, instance is locked\n", i, service_idx);
                return NULL;
            }
        }
    }

    demod_idx = i;
    return new IteEventServerMediaSubsession(env, demod_idx, service_idx);
}

IteEventServerMediaSubsession
::IteEventServerMediaSubsession(
    UsageEnvironment &env, unsigned demod_idx, unsigned service_idx)
    :  OnDemandServerMediaSubsession(env, false),
       _demod_idx(demod_idx), _service_idx(service_idx)
{
    return;
}

IteEventServerMediaSubsession
::~IteEventServerMediaSubsession()
{
    demodPrivDef[_demod_idx].bServiceUsed[_service_idx] = false;
}

FramedSource *IteEventServerMediaSubsession
::createNewStreamSource(
    unsigned clientSessionId,
    unsigned &estBitrate)
{
    estBitrate = 1; // kbps, estimate
    return IteEventSource::createNew(envir(), _demod_idx, 100); // 100ms period check
}

RTPSink *IteEventServerMediaSubsession
::createNewRTPSink(
    Groupsock *rtpGroupsock,
    unsigned char rtpPayloadTypeIfDynamic,
    FramedSource * /*inputSource*/)
{
    return ApplicationRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}

////////// IteEventSource //////////
IteEventSource *
IteEventSource::createNew(UsageEnvironment &env, unsigned demodId, unsigned duration)
{
    return new IteEventSource(env, demodId, duration);
}


IteEventSource::IteEventSource(UsageEnvironment &env, unsigned demodId, unsigned int duration)
    : FramedSource(env), fDemodId(demodId), fDuration(duration)
{
}

IteEventSource::~IteEventSource()
{
}

void IteEventSource::doGetNextFrame()
{
    struct timeval curTime = {0};
    unsigned long long time;
    char utcTime[200];
    unsigned int      y, m, d;
    unsigned char     h, mm, s;

    if( g_MotionEvent[fDemodId] == false )
    {
        nextTask() = envir().taskScheduler().scheduleDelayedTask(fDuration, (TaskFunc *)retryLater, this);
        return;
    }
    g_MotionEvent[fDemodId] = false;

#if defined(CFG_ENABLE_EXTERNAL_RTC)
    {
        // RTC module return UTC
        RtcExt_GetDate(&y, &m, &d);
        RtcExt_GetTime(&h, &mm, &s);
    }
#else
    {
        time_t      rawtime;
        struct tm   *timeinfo;
        timeinfo = localtime(&rawtime);
        h   = timeinfo->tm_hour;
        mm = timeinfo->tm_min;
        s = timeinfo->tm_sec;
        y  = timeinfo->tm_year + 1900;
        m = timeinfo->tm_mon + 1;
        d   = timeinfo->tm_mday;
    }
#endif
    snprintf(utcTime, 200, "%d-%d-%dT%d:%d:%d", y, m, d, h, mm, s);

    gettimeofday(&curTime, NULL);
    time = (curTime.tv_sec * 1000) + (curTime.tv_usec / 1000);
    fPresentationTime.tv_sec = time / 1000;
    fPresentationTime.tv_usec = (time % 1000) * 1000;
    fDurationInMicroseconds = fDuration * 1000;
    snprintf((char *)fTo, 1024,
             "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
             "<tt:MetaDataStream xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
             "<tt:Event>"
             "<wsnt:NotificationMessage>"
             "<wsnt:Topic Dialect=\"...Concrete\">"
             "tns1:RuleEngine/DeclarativeMotionDetector/MotionMatched"
             "</wsnt:Topic>"
             "<wsnt:Message>"
             "<tt:Message UtcTime=\"%s\">"
             "<tt:Source>"
             "<tt:SimpleItem Name=\"VideoSourceConfigurationToken\" Value=\"%d\">"
             "<tt:SimpleItem Name=\"VideoAnalyticsConfigurationToken\" Value=\"1\">"
             "</tt:Source>"
             "</tt:Message>"
             "</wsnt:Message>"
             "</wsnt:NotificationMessage>"
             "</tt:Event>"
             "/tt:MetaDataStream>",
             utcTime, fDemodId);

    FramedSource::afterGetting(this);
    return;
}

void IteEventSource::retryLater(void *firstArg)
{
    IteEventSource *source = (IteEventSource *)firstArg;
    //printf("%s(%d)\n", __FILE__, __LINE__);
    source->doGetNextFrame();
}


////////// ApplicationRTPSink //////////

ApplicationRTPSink *
ApplicationRTPSink::createNew(UsageEnvironment &env, Groupsock *RTPgs,
                              unsigned char rtpPayloadType)
{
    return new ApplicationRTPSink(env, RTPgs, rtpPayloadType);
}

ApplicationRTPSink::ApplicationRTPSink(UsageEnvironment &env,
                                       Groupsock *rtpgs, unsigned char rtpPayloadType)
    : MultiFramedRTPSink(env, rtpgs, rtpPayloadType, 90000, "T140")
{
}

ApplicationRTPSink::~ApplicationRTPSink()
{
}

char const *ApplicationRTPSink::sdpMediaType() const
{
    return "application";
}

void ApplicationRTPSink
::doSpecialFrameHandling(unsigned /*fragmentationOffset*/,
                         unsigned char * /*frameStart*/,
                         unsigned /*numBytesInFrame*/,
                         struct timeval framePresentationTime,
                         unsigned /*numRemainingBytes*/)
{
    setMarkerBit();
    setTimestamp(framePresentationTime);
}
