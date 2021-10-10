
#include "IteAirTsSplitServerMediaSubsession.h"
#include "H264VideoRTPSink.hh"
#include "ite/itp.h"
#include "ite/ite_risc_ts_demux.h"
#include "ts_airfile/ite_ts_airfile.h"
//=============================================================================
//				  Constant Definition
//=============================================================================

#define MAX_SERVICE_PER_DEMOD   4

//=============================================================================
//				  Macro Definition
//=============================================================================


//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct _DEMOD_PRIV_INFO_TAG
{
    const char          *name;
    bool  bServiceUsed[MAX_SERVICE_PER_DEMOD];
} DEMOD_PRIV_INFO;

//=============================================================================
//				  Global Data Definition
//=============================================================================
/**
 * Define URL
 * rtsp://xxx.xxx.xxx.xxx/channel_0.01.airts
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
        TSAF_HANDLE  *g_pHTsaf[MAX_DEMOD_SUPPORT_CNT] = {0};
    }
#endif

//=============================================================================
//				  Private Function Definition
//=============================================================================


//=============================================================================
//				  Public Function Definition
//=============================================================================
IteAirTsSplitServerMediaSubsession *
IteAirTsSplitServerMediaSubsession
::createNew(
    UsageEnvironment &env,
    char const *dataFileName)
{
    int         i, demod_idx;
    char        service_name[16] = {0};
    int         service_idx = -1;
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

    IteAirTsSplitServerMediaSubsession   *iteAirTsSMS
        = new IteAirTsSplitServerMediaSubsession(env, dataFileName, demod_idx, service_idx);
    if( iteAirTsSMS->_bDemod_ready == false )
    {
        printf(" fail !!!! %s()[#%d]\n", __FUNCTION__, __LINE__);
        delete iteAirTsSMS;
        iteAirTsSMS = NULL;
    }
    return iteAirTsSMS;
}

IteAirTsSplitServerMediaSubsession
::IteAirTsSplitServerMediaSubsession(
    UsageEnvironment &env,
    char const  *fileName,
    int         demod_idx,
    int         service_idx)
: FileServerMediaSubsession(env, fileName, false),
  _demod_idx(demod_idx), _service_idx(service_idx), _iteAirSource(NULL)
{
    _bDemod_ready = true;

    return;
}

IteAirTsSplitServerMediaSubsession
::~IteAirTsSplitServerMediaSubsession()
{
    demodPrivDef[_demod_idx].bServiceUsed[_service_idx] = false;
    _bDemod_ready = false;
}

void
IteAirTsSplitServerMediaSubsession
::searchChannel(uint32_t freq, uint32_t bandwidth)
{

}

FramedSource *IteAirTsSplitServerMediaSubsession
::createNewStreamSource(
    unsigned clientSessionId,
    unsigned &estBitrate)
{
    estBitrate = 5000; // kbps, estimate
    ItePesSource* iteAirSource
        = ItePesSource::createNew(envir(), (uint32_t)_demod_idx, (uint32_t) _service_idx);
    return iteAirSource;
}

RTPSink *IteAirTsSplitServerMediaSubsession
::createNewRTPSink(
    Groupsock *rtpGroupsock,
    unsigned char rtpPayloadTypeIfDynamic,
    FramedSource * /*inputSource*/)
{
#if 1
    uint8* pSps = 0;
    uint32 spsSize = 0;
    uint8* pPps = 0;
    uint32 ppsSize = 0;
    uint32 i = 0;

    //printf("demod state : %d\n", g_pHTsaf[_demod_idx]->state);
    if (g_pHTsaf[_demod_idx]->state != TSAF_STATE_READY)
        return NULL;
        
    printf("demod: %u, index: %u\n", _demod_idx, _service_idx);
    iteTsDemux_GetSpsPpsInfo(_demod_idx, _service_idx, &pSps, &spsSize, &pPps, &ppsSize);
    printf("spsSize: %u, ppsSize: %u\n", spsSize, ppsSize);
//    printf("SPS\n");
//    for (i = 0; i < spsSize; i++)
//    {
//        printf("0x%02X ", pSps[i]);
//    }
//    printf("\n");
//    printf("PPS\n");
//    for (i = 0; i < ppsSize; i++)
//    {
//        printf("0x%02X ", pPps[i]);
//    }
//    printf("\n");
    return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, pSps, spsSize, pPps, ppsSize);

#else
    uint8     *pSps = 0, *pPps = 0;
    uint32    spsSize = 0, ppsSize = 0;

    printf("demod: %u, index: %u\n", _demod_idx, _service_idx);

#if !(_MSC_VER)
    if( g_pHTsaf[_demod_idx]->state != TSAF_STATE_READY )   
        return NULL;

    iteTsDemux_GetSpsPpsInfo(_demod_idx, _service_idx, &pSps, &spsSize, &pPps, &ppsSize);
    if( !pSps || !spsSize || !pPps || !ppsSize )
    {
        printf("No H.264 data ! (pSps=0x%x, spsSize=%d, pPps=0x%x, ppsSize=%d)\n",
                pSps, spsSize, pPps, ppsSize);
        return NULL;
    }
#endif
    
    printf("spsSize: %u, ppsSize: %u\n", spsSize, ppsSize);

    // uint32_t    i;
    // printf("SPS\n");
    // for(i = 0; i < spsSize; i++)    printf("0x%02X ", pSps[i]);
    // printf("\n");

    // printf("PPS\n");
    // for(i = 0; i < ppsSize; i++)    printf("0x%02X ", pPps[i]);
    // printf("\n");

    return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                       pSps, spsSize, pPps, ppsSize);
#endif
}

void IteAirTsSplitServerMediaSubsession
::startStream(unsigned clientSessionId,
              void *streamToken,
              TaskFunc *rtcpRRHandler,
              void *rtcpRRHandlerClientData, unsigned short &rtpSeqNum,
              unsigned &rtpTimestamp,
              ServerRequestAlternativeByteHandler *serverRequestAlternativeByteHandler,
              void *serverRequestAlternativeByteHandlerClientData)
{
    // Call the original, default version of this routine:
    OnDemandServerMediaSubsession::startStream(clientSessionId, streamToken,
            rtcpRRHandler, rtcpRRHandlerClientData,
            rtpSeqNum, rtpTimestamp,
            serverRequestAlternativeByteHandler,
            serverRequestAlternativeByteHandlerClientData);
}


void IteAirTsSplitServerMediaSubsession
::testScaleFactor(float &scale)
{
    scale = 1.0f;
}

void IteAirTsSplitServerMediaSubsession
::seekStream(unsigned clientSessionId,
             void *streamToken,
             double &seekNPT,
             double streamDuration,
             u_int64_t &numBytes)
{
}



