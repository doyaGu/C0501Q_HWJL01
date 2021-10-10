
#if defined(CFG_DEMOD_ENABLE)
#include "MPEG2TransportStreamFramer.hh"
#include "IteAirTsServerMediaSubsession.h"
#include "SimpleRTPSink.hh"
#include "ite/itp.h"
#include "ts_airfile/ite_ts_airfile.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
typedef enum _DEMOD_ID_TAG
{
    DEMOD_ID_00 = 0xd0,
    DEMOD_ID_01,
    DEMOD_ID_02,
    DEMOD_ID_03,

} DEMOD_ID;

//=============================================================================
//				  Macro Definition
//=============================================================================


//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct _DEMOD_PRIV_INFO_TAG
{
    const char          *name;
    const DEMOD_ID      id;
    bool                bUsed;

    const uint32_t      startFreq;
    const uint32_t      endFreq;
    const uint32_t      bandwidth;
    const uint32_t      channlTotalCnt; 
    
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
	{"channel_0", DEMOD_ID_00, false, 533000, 539000, 6000, 1},
#endif
#if (MAX_DEMOD_SUPPORT_CNT > 1)
	{"channel_1", DEMOD_ID_01, false, 545000, 551000, 6000, 1},
#endif
#if (MAX_DEMOD_SUPPORT_CNT > 2)
	{"channel_2", DEMOD_ID_02, false, 533000, 539000, 6000, 1},
#endif
#if (MAX_DEMOD_SUPPORT_CNT > 3)
	{"channel_3", DEMOD_ID_03, false, 533000, 539000, 6000, 1},
#endif
};


//=============================================================================
//				  Private Function Definition
//=============================================================================


//=============================================================================
//				  Public Function Definition
//=============================================================================
IteAirTsServerMediaSubsession *
IteAirTsServerMediaSubsession
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

    for(i = 0; i < MAX_DEMOD_SUPPORT_CNT; i++)
    {
        if( strncmp(dataFileName, demodPrivDef[i].name, strlen(demodPrivDef[i].name)) == 0 )
        {
            if( demodPrivDef[i].bUsed == false )
            {
                demodPrivDef[i].bUsed = true;
                break;
            }
            else    return NULL;
        }
    }

    demod_idx = i;

    strncpy(service_name, pSubname, (size_t)(pSubnameL2 - pSubname));
    service_idx = atoi(service_name);

    IteAirTsServerMediaSubsession   *iteAirTsSMS 
        = new IteAirTsServerMediaSubsession(env, dataFileName, demod_idx, service_idx);
           
    if( iteAirTsSMS->_bDemod_ready == false ) 
    {
        printf(" fail !!!! %s()[#%d]\n", __FUNCTION__, __LINE__);
        delete iteAirTsSMS;
        iteAirTsSMS = NULL;
    }
    return iteAirTsSMS;
}

#define TRANSPORT_PACKET_SIZE 188
#define TRANSPORT_PACKETS_PER_NETWORK_PACKET 7

IteAirTsServerMediaSubsession
::IteAirTsServerMediaSubsession(
    UsageEnvironment &env,
    char const  *fileName,
    int         demod_idx,
    int         service_idx)
: FileServerMediaSubsession(env, fileName, false),
  _demod_idx(demod_idx), _service_idx(service_idx), _iteAirSource(NULL)
{
    _inputDataSize = TRANSPORT_PACKETS_PER_NETWORK_PACKET*TRANSPORT_PACKET_SIZE;
    
    int rst = IteAirTsStreamSource::createNew_Probe(envir(), _demod_idx);

    if( rst < 0 )   _bDemod_ready = false;
    else            _bDemod_ready = true;

    return;
}

IteAirTsServerMediaSubsession
::~IteAirTsServerMediaSubsession()
{
    // tsi_disable
    // itp_demod close file
    demodPrivDef[_demod_idx].bUsed = false;
}

void
IteAirTsServerMediaSubsession
::searchChannel(uint32_t freq, uint32_t bandwidth)
{

}

FramedSource *IteAirTsServerMediaSubsession
::createNewStreamSource(
    unsigned clientSessionId,
    unsigned &estBitrate)
{
    estBitrate = 5000; // kbps, estimate
    IteAirTsStreamSource* iteAirSource
        = IteAirTsStreamSource::createNew(envir(), _demod_idx, _service_idx, _inputDataSize);
    
    if( iteAirSource == NULL ) printf("\tCreate IteAirTsStreamSource fail !!\n");

    _iteAirSource = iteAirSource;

    // Create a framer for the Transport Stream:
    MPEG2TransportStreamFramer *framer
        = MPEG2TransportStreamFramer::createNew(envir(), _iteAirSource);

    if( framer == NULL )    printf(" fail !!!! %s()[#%d]\n", __FUNCTION__, __LINE__);
    return framer;
}

RTPSink *IteAirTsServerMediaSubsession
::createNewRTPSink(
    Groupsock *rtpGroupsock,
    unsigned char /*rtpPayloadTypeIfDynamic*/,
    FramedSource * /*inputSource*/)
{
    return SimpleRTPSink::createNew(envir(), rtpGroupsock,
                                    33, 90000, "video", "MP2T",
                                    1, True, False /*no 'M' bit*/);
}

void IteAirTsServerMediaSubsession
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


void IteAirTsServerMediaSubsession
::testScaleFactor(float &scale)
{
    scale = 1.0f;
}

void IteAirTsServerMediaSubsession
::seekStream(unsigned clientSessionId,
             void *streamToken,
             double &seekNPT,
             double streamDuration,
             u_int64_t &numBytes)
{
    // Begin by calling the original, default version of this routine:
    //OnDemandServerMediaSubsession::seekStream(clientSessionId, streamToken, seekNPT, streamDuration, numBytes);
}

#endif

